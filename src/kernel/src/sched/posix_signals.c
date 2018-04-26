/* Copyright (c) 2018 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_KERNEL_SRC_SCHED_POSIX_SIGNALS_C
#define GUARD_KERNEL_SRC_SCHED_POSIX_SIGNALS_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <sched/posix_signals.h>
#include <hybrid/timespec.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <sched/group.h>
#include <sched/pid.h>
#include <sched/suspend.h>
#include <alloca.h>
#include <string.h>
#include <assert.h>
#include <except.h>
#include <bits/sched.h>

DECL_BEGIN

STATIC_ASSERT((uintptr_t)SIGNAL_ACTION_IGNORE == (uintptr_t)SIG_IGN);
STATIC_ASSERT((uintptr_t)SIGNAL_ACTION_TERM   == (uintptr_t)SIG_TERM);
STATIC_ASSERT((uintptr_t)SIGNAL_ACTION_EXIT   == (uintptr_t)SIG_EXIT);
STATIC_ASSERT((uintptr_t)SIGNAL_ACTION_CONT   == (uintptr_t)SIG_CONT);
STATIC_ASSERT((uintptr_t)SIGNAL_ACTION_STOP   == (uintptr_t)SIG_STOP);
STATIC_ASSERT((uintptr_t)SIGNAL_ACTION_CORE   == (uintptr_t)SIG_CORE);


/* [0..1][lock(PRIVATE(THIS_TASK))][ref(sb_share)]
 * The set of signals being blocked in the current thread.
 * When NULL, assume that no signals are being blocked. */
PUBLIC REF ATTR_PERTASK struct sigblock *_this_sigblock = NULL;

/* [0..1][lock(PRIVATE(THIS_TASK))]
 * User-space signal handlers for the calling thread.
 * NOTE: When NULL, default behavior should be used for handling any signal. */
PUBLIC ATTR_PERTASK REF struct sighand_ptr *_this_sighand_ptr = NULL;

#ifdef CONFIG_SIGPENDING_TRACK_MASK
/* [0..1][owned][lock(WRITE_ONCE)]
 * Pending task chain for the calling thread. */
PUBLIC ATTR_PERTASK struct sigpending *_this_sigpending_task = NULL;
#else
/* Pending task chain for the calling thread. */
PUBLIC ATTR_PERTASK struct sigpending _this_sigpending_task = {
    .sp_lock      = MUTEX_INIT,
    .sp_newsig    = SIG_INIT,
    .sp_queue     = NULL
};
#endif

/* [0..1][lock(WRITE_ONCE)]
 * Pending task chain for the calling process. */
PUBLIC ATTR_PERTASK REF struct sigshare *_this_sigpending_proc = NULL;

PRIVATE void KCALL
sighand_destroy(struct sighand *__restrict self) {
 /* Destroy this signal handler suit. */
 unsigned int i;
 for (i = 0; i < _NSIG; ++i)
     kfree(self->sh_hand[i]);
 kfree(self);
}

INTERN void KCALL
sighand_decref(struct sighand *__restrict self) {
 atomic_rwlock_write(&self->sh_lock);
 if (self->sh_share == 1) {
  atomic_rwlock_endwrite(&self->sh_lock);
  sighand_destroy(self);
 } else {
  --self->sh_share;
  atomic_rwlock_endwrite(&self->sh_lock);
 }
}

INTERN void KCALL
sighand_ptr_decref(struct sighand_ptr *__restrict self) {
 if (ATOMIC_DECFETCH(self->sp_refcnt) == 0) {
  if (self->sp_hand)
      sighand_decref(self->sp_hand);
  kfree(self);
 }
}


DEFINE_PERTASK_CLONE(posix_signals_clone_task);
INTERN void KCALL
posix_signals_clone_task(struct task *__restrict new_thread,
                         u32 flags) {
 /* Copy the signal mask. */
 struct sigblock *block = PERTASK_GET(_this_sigblock);
 if (block) { /* NULL would mean: not blocking anything. */
  ATOMIC_FETCHINC(block->sb_share);
  FORTASK(new_thread,_this_sigblock) = block;
 }

 if (flags & CLONE_SIGHAND) {
  /* Use the same signal handler set for the new thread.
   * For this case, we must ensure that the sighand pointer
   * is allocated for both the calling, as well as the new
   * thread (and is identical), yet we don't need to ensure
   * that it actually points to anything. */
  REF struct sighand_ptr *ptr;
  ptr = PERTASK_GET(_this_sighand_ptr);
  if (!ptr) {
   /* Allocate the pointer the first time a new thread
    * appears in the calling thread's thread group, when
    * no signal handlers have been defined, yet. */
   ptr = (REF struct sighand_ptr *)kmalloc(sizeof(struct sighand_ptr),
                                           GFP_SHARED);
   ptr->sp_refcnt = 2; /* +1 for the calling thread; +1 for `new_thread' */
   ptr->sp_hand = NULL;
   atomic_rwlock_init(&ptr->sp_lock);
   PERTASK_SET(_this_sighand_ptr,ptr);
  } else {
   ATOMIC_FETCHINC(ptr->sp_refcnt);
  }
  /* The given new thread the same signal handlers
   * pointer as is already set in the calling thread. */
  FORTASK(new_thread,_this_sighand_ptr) = ptr; /* Inherit reference */
 } else {
  /* Use a lazily duplicated copy of signal handlers
   * for the new thread (the handler set itself will
   * be duplicated using copy-on-write) */
  struct sighand_ptr *old_ptr = PERTASK_GET(_this_sighand_ptr);
  /* NOTE: When no handlers have been defined, no need to
   *       do anything to get the new thread to use default
   *       behavior for all signals. */
  if (old_ptr != NULL) {
   REF struct sighand *EXCEPT_VAR cow_handlers;
   REF struct sighand_ptr *COMPILER_IGNORE_UNINITIALIZED(new_ptr);
   atomic_rwlock_read(&old_ptr->sp_lock);
   cow_handlers = old_ptr->sp_hand;
   if (!cow_handlers) {
    /* No handlers have been allocated (this can happen
     * if the calling thread was part of a thread group
     * at some point, or still is). */
    atomic_rwlock_endread(&old_ptr->sp_lock);
   } else {
    /* Acquire a share(COW)-reference to the handles
     * that should be copied on write. */
    atomic_rwlock_write(&cow_handlers->sh_lock);
    atomic_rwlock_endread(&old_ptr->sp_lock);
    ++cow_handlers->sh_share;
    atomic_rwlock_endwrite(&cow_handlers->sh_lock);
    TRY {
     /* Allocate the new sighand-share pointer for the target thread. */
     new_ptr = (REF struct sighand_ptr *)kmalloc(sizeof(struct sighand_ptr),
                                                 GFP_SHARED);
     new_ptr->sp_refcnt = 1;
     atomic_rwlock_init(&new_ptr->sp_lock);
     new_ptr->sp_hand = cow_handlers; /* Inherit reference. */
    } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
     /* Drop the shared reference we've created before. */
     sighand_decref(cow_handlers);
     error_rethrow();
    }
    /* Save the sighand pointer in the new thread. */
    FORTASK(new_thread,_this_sighand_ptr) = new_ptr;
   }
  }
 }
 if (flags & CLONE_THREAD) {
  /* Same thread group --> same sigshare context. */
  struct sigshare *share;
  share = COMPILER_CONTAINER_OF(sigpending_getproc(),
                                struct sigshare,
                                ss_pending);
  /* Share the sigshare context with the new thread. */
  ATOMIC_FETCHINC(share->ss_refcnt);
  FORTASK(new_thread,_this_sigpending_proc) = share;
 }
}

PRIVATE void KCALL
sigpending_fini(struct sigpending *__restrict self) {
 struct sigqueue *iter,*next;
 iter = self->sp_queue;
 while (iter) {
  next = iter->sq_chain.le_next;
  kfree(iter);
  iter = next;
 }
}

DEFINE_PERTASK_FINI(posix_signals_fini_task);
INTERN void KCALL
posix_signals_fini_task(struct task *__restrict thread) {
 /* Finalize posix-signal components of the given thread. */
 { REF struct sigblock *temp;
   temp = FORTASK(thread,_this_sigblock);
   if (temp && ATOMIC_DECFETCH(temp->sb_share) == 0)
       kfree(temp);
 }
#ifdef CONFIG_SIGPENDING_TRACK_MASK
 if (FORTASK(thread,_this_sigpending_task)) {
  sigpending_fini(FORTASK(thread,_this_sigpending_task));
  kfree(FORTASK(thread,_this_sigpending_task));
 }
#else
 sigpending_fini(&FORTASK(thread,_this_sigpending_task));
#endif
 { REF struct sighand_ptr *temp;
   temp = FORTASK(thread,_this_sighand_ptr);
   if (temp) sighand_ptr_decref(temp);
 }
 { REF struct sigshare *temp;
   temp = FORTASK(thread,_this_sigpending_proc);
   if (temp && ATOMIC_DECFETCH(temp->ss_refcnt) == 0) {
    sigpending_fini(&temp->ss_pending);
    kfree(temp);
   }
 }
}



/* Allocate a missing `_this_sigblock', or replace it
 * with a copy of itself when `sb_share > 1'.
 * @return: * : Always returns `PERTASK(_this_sigblock)'
 * @throw E_BADALLOC: Not enough available memory. */
PUBLIC ATTR_RETNONNULL struct sigblock *KCALL sigblock_unique(void) {
 struct sigblock *result = PERTASK_GET(_this_sigblock);
 if (!result) {
  result = (struct sigblock *)kmalloc(sizeof(struct sigblock),
                                      GFP_SHARED|GFP_CALLOC);
  result->sb_share = 1;
  PERTASK_SET(_this_sigblock,result);
 } else if (ATOMIC_READ(result->sb_share) > 1) {
  struct sigblock *new_result;
  new_result = (struct sigblock *)kmalloc(sizeof(struct sigblock),
                                          GFP_SHARED);
  new_result->sb_share = 1;
  memcpy(&new_result->sb_sigset,&result->sb_sigset,sizeof(sigset_t));
  /* Drop a share-reference from the old set. */
  if unlikely(ATOMIC_DECFETCH(result->sb_share) == 0)
     kfree(result);
  /* Save the new set. */
  PERTASK_SET(_this_sigblock,new_result);
  result = new_result;
 }
 return result;
}

PUBLIC struct sigblock *KCALL sigblock_unique_fast(void) {
 struct sigblock *result = PERTASK_GET(_this_sigblock);
 if (result && ATOMIC_READ(result->sb_share) > 1)
     result = NULL;
 return result;
}

/* Allocates a missing or unshare a shared signal handler
 * table and acquire a write-lock on (return->sh_lock).
 * The caller may then modify the `sh_hand' vector freely,
 * before calling `atomic_rwlock_endwrite(&return->sh_lock)'
 * to commit any changes that were made.
 * This function automatically deals with all inter-process
 * copy-on-write, as well as inter-thread sighand-share behavior. */
PUBLIC ATTR_RETNONNULL struct sighand *KCALL sighand_lock_write(void) {
 struct sighand *oldhand;
 struct sighand_ptr *ptr;
 ptr = PERTASK_GET(_this_sighand_ptr);
 if (!ptr) {
  /* Allocate a new sighand pointer. */
  ptr = (struct sighand_ptr *)kmalloc(sizeof(struct sighand_ptr),
                                      GFP_SHARED);
  ptr->sp_refcnt = 1;
  ptr->sp_hand   = NULL;
  atomic_rwlock_init(&ptr->sp_lock);
  /* No atomic operation, because this field is thread-private. */
  PERTASK_SET(_this_sighand_ptr,ptr);
 }
again:
 atomic_rwlock_read(&ptr->sp_lock);
 oldhand = ptr->sp_hand;
 if (!oldhand) {
  atomic_rwlock_endread(&ptr->sp_lock);
  /* Allocate a new, empty sighand. */
  oldhand = (struct sighand *)kmalloc(sizeof(struct sighand),
                                      GFP_SHARED|GFP_CALLOC);
  oldhand->sh_share = 1;
  /* Pre-acquire a write-lock to the new sighand. */
  atomic_rwlock_cinit_write(&oldhand->sh_lock);

  atomic_rwlock_write(&ptr->sp_lock);
  if unlikely(ptr->sp_hand) {
   /* Another thread was faster... */
   atomic_rwlock_endwrite(&ptr->sp_lock);
   /* Delete the new sighand and start over. */
   kfree(oldhand);
   goto again;
  }
  ptr->sp_hand = oldhand;
  atomic_rwlock_endwrite(&ptr->sp_lock);
  /* At this point, we've still got the write lock we've created
   * above, which is the exact state that the caller is expecting
   * us to return with. */
 } else {
  /* Interlocked change our locking to the sighand vector descriptor. */
  atomic_rwlock_write(&oldhand->sh_lock);
  atomic_rwlock_endread(&ptr->sp_lock);
  if (oldhand->sh_share > 1) {
   struct sighand *EXCEPT_VAR newhand;
   /* Acquire a reference to the vector that we want to copy. */
   ++oldhand->sh_share;
   atomic_rwlock_endwrite(&oldhand->sh_lock);
   /* Must replace with a duplicate. */
   newhand = (struct sighand *)kmalloc(sizeof(struct sighand),
                                       GFP_SHARED|GFP_CALLOC);
   newhand->sh_share = 1;
   /* Pre-acquire a write-lock to the new sighand. */
   atomic_rwlock_cinit_write(&newhand->sh_lock);
   /* Duplicate sighand entries (copy-on-write-style). */
   TRY {
    unsigned int i;
    atomic_rwlock_read(&oldhand->sh_lock);
    for (i = 0; i < _NSIG; ++i) {
     if (!oldhand->sh_hand[i]) continue;
     atomic_rwlock_endread(&oldhand->sh_lock);
     /* Allocate a new sigaction for this entry. */
     newhand->sh_hand[i] = (struct sigaction *)kmalloc(sizeof(struct sigaction),
                                                       GFP_SHARED);
     /* Re-acquire a read-lock to the old handler vector. */
     atomic_rwlock_read(&oldhand->sh_lock);
     COMPILER_READ_BARRIER();
     /* Check if the old vector changed... */
     if unlikely(!oldhand->sh_hand[i]) {
      atomic_rwlock_endread(&oldhand->sh_lock);
      sighand_destroy(newhand);
      goto again;
     }
     /* Copy data for the action. */
     memcpy(newhand->sh_hand[i],
            oldhand->sh_hand[i],
            sizeof(struct sigaction));
    }
    atomic_rwlock_endread(&oldhand->sh_lock);
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    sighand_destroy(newhand);
    error_rethrow();
   }
   /* With the vector now duplicated, try to
    * replace our pointer with that copy. */
   atomic_rwlock_write(&ptr->sp_lock);

   if unlikely(ptr->sp_hand != oldhand) {
    /* The signal handler table changed in the mean time. */
    atomic_rwlock_endwrite(&ptr->sp_lock);
    /* Destroy our copy (which is no longer up-to-date). */
    sighand_destroy(newhand);
    sighand_decref(oldhand);
    goto again;
   } else {
    unsigned int i;
    /* Drop a reference form the old handler */
    atomic_rwlock_write(&oldhand->sh_lock);
    assertf(oldhand->sh_share >= 2,
            "Since we checked that `ptr->sp_hand == oldhand', we know "
            "that one reference must be held by it. Add that to our "
            "temporary reference acquired above, there must still be "
            "at least 2 reference in total! (%u)",
            oldhand->sh_share);
    if unlikely(oldhand->sh_share == 2) {
     /* The old handler vector got unshared by another thread.
      * Discard our copy and use their work instead (keep caches alive!) */
     --oldhand->sh_share; /* Drop our temporarily reference. */
     COMPILER_WRITE_BARRIER();
     atomic_rwlock_endwrite(&oldhand->sh_lock);
     atomic_rwlock_endwrite(&ptr->sp_lock);
     sighand_destroy(newhand);
     goto again;
    }
    assert(oldhand->sh_share > 2);
    /* Verify that our copy matches the old handler _exactly_
     * Due to race conditions, some vector entries may have changed... */
    for (i = 0; i < _NSIG; ++i) {
     if unlikely(!newhand->sh_hand[i]) {
      if (oldhand->sh_hand[i])
          goto nonmatching_copy;
      continue;
     }
     if (!oldhand->sh_hand[i]) goto nonmatching_copy;
     if (memcmp(newhand->sh_hand[i],
                oldhand->sh_hand[i],
                sizeof(struct sigaction)) == 0)
         continue;
nonmatching_copy:
     /* Our copy no longer matches the original. - Try again. */
     assert(oldhand->sh_share > 2);
     --oldhand->sh_share;
     assert(oldhand->sh_share > 1);
     atomic_rwlock_endwrite(&oldhand->sh_lock);
     atomic_rwlock_endwrite(&ptr->sp_lock);
     sighand_destroy(newhand);
     goto again;
    }
    /* Unshare the old handler vector. */
    assert(oldhand->sh_share > 2);
    --oldhand->sh_share;
    assert(oldhand->sh_share > 1);
    /* Drop the second reference still held by `ptr->sp_hand'.
     * Since we're going to override that field, we must also
     * deal with the reference incrementation of the old value. */
    if unlikely(--oldhand->sh_share == 0) {
     /* Must delete the old handler vector. */
     atomic_rwlock_endwrite(&oldhand->sh_lock);
     /* Save the new handler vector. */
     ptr->sp_hand = newhand;
     atomic_rwlock_endwrite(&ptr->sp_lock);
     sighand_destroy(oldhand);
     return newhand;
    }
    atomic_rwlock_endwrite(&oldhand->sh_lock);
    /* Save the new handler vector. */
    ptr->sp_hand = newhand;
   }
   /* Save the new handler. */
   atomic_rwlock_endwrite(&ptr->sp_lock);
   oldhand = newhand;
  }
  /* At this point, we've still got the write lock we've created
   * above, which is the exact state that the caller is expecting
   * us to return with. */
 }
 return oldhand;
}

/* Similar to `sighand_lock_write()', but don't
 * unshare and acquire a read-lock instead. 
 * If no handlers have been defined, return `NULL'
 * instead (implies DEFAULT behavior for all signals). */
PUBLIC struct sighand *KCALL sighand_lock_read(void) {
 struct sighand *result;
 struct sighand_ptr *ptr;
 ptr = PERTASK_GET(_this_sighand_ptr);
 if (!ptr) return NULL;
 atomic_rwlock_read(&ptr->sp_lock);
 result = ptr->sp_hand;
 if (!result) {
  atomic_rwlock_endread(&ptr->sp_lock);
  goto done;
 }
 /* Interlocked acquire a read-lock to the actual signal-handler vector. */
 atomic_rwlock_read(&result->sh_lock);
 atomic_rwlock_endread(&ptr->sp_lock);
done:
 return result;
}



#ifdef CONFIG_SIGPENDING_TRACK_MASK
/* Lazily allocate if missing, and return the
 * per-task or per-process sigpending controllers. */
PUBLIC ATTR_RETNONNULL struct sigpending *
KCALL sigpending_gettask(void) {
 struct sigpending *result,*new_result;
 result = PERTASK_GET(_this_sigpending_task);
 if (result) return result;
 /* Construct a new thread-private sigpending controller. */
 result = (struct sigpending *)kmalloc(sizeof(struct sigpending),
                                       GFP_SHARED|GFP_CALLOC);
 mutex_cinit(&result->sp_lock);
 sig_cinit(&result->sp_newsig);
 /* Save the generated sigpending controller. */
 new_result = ATOMIC_CMPXCH_VAL(PERTASK(_this_sigpending_task),
                                NULL,result);
 if unlikely(new_result) {
  /* Some other thread was faster, using `sigpending_getfor()' */
  kfree(result);
  result = new_result;
 }
 return result;
}
PUBLIC ATTR_RETNONNULL struct sigpending *
KCALL sigpending_getfor(struct task *__restrict thread) {
 struct sigpending *result,*new_result;
 result = FORTASK(thread,_this_sigpending_task);
 if (result) return result;
 /* Construct a new thread-private sigpending controller. */
 result = (struct sigpending *)kmalloc(sizeof(struct sigpending),
                                       GFP_SHARED|GFP_CALLOC);
 mutex_cinit(&result->sp_lock);
 sig_cinit(&result->sp_newsig);
 /* Save the generated sigpending controller. */
 new_result = ATOMIC_CMPXCH_VAL(FORTASK(thread,_this_sigpending_task),
                                NULL,result);
 if unlikely(new_result) {
  /* Some other thread was faster, using `sigpending_getfor()' */
  kfree(result);
  result = new_result;
 }
 return result;
}
#endif /* CONFIG_SIGPENDING_TRACK_MASK */

PUBLIC ATTR_RETNONNULL struct sigpending *
KCALL sigpending_getproc(void) {
 struct sigshare *result,*new_result;
 result = PERTASK_GET(_this_sigpending_proc);
 if (!result) {
  /* Allocate a new sigshare descriptor. */
  result = (struct sigshare *)kmalloc(sizeof(struct sigshare),
                                      GFP_SHARED|GFP_CALLOC);
  result->ss_refcnt = 1;
  mutex_cinit(&result->ss_pending.sp_lock);
  sig_cinit(&result->ss_pending.sp_newsig);
  new_result = ATOMIC_CMPXCH_VAL(PERTASK(_this_sigpending_proc),NULL,result);
  if unlikely(new_result) kfree(result),result = new_result;
 }
 return &result->ss_pending;
}
PUBLIC ATTR_RETNONNULL struct sigpending *
KCALL sigpending_getprocfor(struct task *__restrict thread) {
 struct sigshare *result,*new_result;
 result = FORTASK(thread,_this_sigpending_proc);
 if (!result) {
  /* Allocate a new sigshare descriptor. */
  result = (struct sigshare *)kmalloc(sizeof(struct sigshare),
                                      GFP_SHARED|GFP_CALLOC);
  result->ss_refcnt = 1;
  mutex_cinit(&result->ss_pending.sp_lock);
  sig_cinit(&result->ss_pending.sp_newsig);
  new_result = ATOMIC_CMPXCH_VAL(FORTASK(thread,_this_sigpending_proc),NULL,result);
  if unlikely(new_result) kfree(result),result = new_result;
 }
 return &result->ss_pending;
}






#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"

PUBLIC u8 const signal_default_actions[_NSIG] = {
    [0 ... _NSIG - 1] = SIGNAL_ACTION_IGNORE,
    [SIGHUP]    = SIGNAL_ACTION_TERM,
    [SIGINT]    = SIGNAL_ACTION_TERM,
    [SIGQUIT]   = SIGNAL_ACTION_CORE,
    [SIGILL]    = SIGNAL_ACTION_CORE,
    [SIGABRT]   = SIGNAL_ACTION_CORE,
    [SIGFPE]    = SIGNAL_ACTION_CORE,
    [SIGKILL]   = SIGNAL_ACTION_TERM,
    [SIGSEGV]   = SIGNAL_ACTION_CORE,
    [SIGPIPE]   = SIGNAL_ACTION_TERM,
    [SIGALRM]   = SIGNAL_ACTION_TERM,
    [SIGTERM]   = SIGNAL_ACTION_TERM,
    [SIGUSR1]   = SIGNAL_ACTION_TERM,
    [SIGUSR2]   = SIGNAL_ACTION_TERM,
    [SIGCHLD]   = SIGNAL_ACTION_IGNORE,
    [SIGCONT]   = SIGNAL_ACTION_CONT,
    [SIGSTOP]   = SIGNAL_ACTION_STOP,
    [SIGTSTP]   = SIGNAL_ACTION_STOP,
    [SIGTTIN]   = SIGNAL_ACTION_STOP,
    [SIGTTOU]   = SIGNAL_ACTION_STOP,
    [SIGBUS]    = SIGNAL_ACTION_CORE,
    [SIGPOLL]   = SIGNAL_ACTION_TERM,
    [SIGPROF]   = SIGNAL_ACTION_TERM,
    [SIGSYS]    = SIGNAL_ACTION_CORE,
    [SIGTRAP]   = SIGNAL_ACTION_CORE,
    [SIGURG]    = SIGNAL_ACTION_IGNORE,
    [SIGVTALRM] = SIGNAL_ACTION_TERM,
    [SIGXCPU]   = SIGNAL_ACTION_CORE,
    [SIGXFSZ]   = SIGNAL_ACTION_CORE,
#if defined(SIGIOT) && SIGIOT != SIGABRT
    [SIGIOT]    = SIGNAL_ACTION_CORE,
#endif
#ifdef SIGEMT
    [SIGEMT]    = SIGNAL_ACTION_TERM,
#endif
    [SIGSTKFLT] = SIGNAL_ACTION_TERM,
#if defined(SIGIO) && SIGIO != SIGPOLL
    [SIGIO]     = SIGNAL_ACTION_TERM,
#endif
#if defined(SIGCLD) && SIGCLD != SIGCHLD
    [SIGCLD]    = SIGNAL_ACTION_IGNORE,
#endif
    [SIGPWR]    = SIGNAL_ACTION_TERM,
#ifdef SIGLOST
    [SIGLOST]   = SIGNAL_ACTION_TERM,
#endif
    [SIGWINCH]  = SIGNAL_ACTION_IGNORE,
#if defined(SIGUNUSED) && SIGUNUSED != SIGSYS
    [SIGUNUSED] = SIGNAL_ACTION_CORE,
#endif
#undef ENTRY
};
#pragma GCC diagnostic pop

PRIVATE void KCALL resume_thread(void *UNUSED(arg)) {
 task_resume(THIS_TASK);
}

PRIVATE void KCALL
suspend_thread(void *UNUSED(arg),
               struct cpu_hostcontext_user *__restrict UNUSED(context),
               unsigned int UNUSED(mode)) {
 task_suspend(JTIME_INFINITE);
}

PRIVATE void KCALL
do_redirect_signal_action(struct cpu_hostcontext_user *__restrict context,
                          unsigned int signo, siginfo_t const *__restrict info,
                          struct sigaction const *__restrict action,
                          unsigned int mode) {
 struct sigblock *block;
 /* Redirect user-space towards the signal
  * handler before we update the blocking-mask,
  * so it can save the old mask to-be restored
  * by the sigreturn system call. */
 arch_posix_signals_redirect_action(context,info,action,mode);

 /* Resume user-space execution (for now) and have the signal
  * handler returning re-throw an E_INTERRUPT exception if
  * necessary. */
 error_info()->e_error.e_code = E_USER_RESUME;

 if ((block = sigblock_unique_fast()) == NULL) {
  /* Optimization: If we're not going to modify the
   *               sigblock, don't force-allocate it. */
  if (!(action->sa_flags & SA_NODEFER) &&
      !memxchr(&action->sa_mask,0,sizeof(sigset_t)))
       return;

  /* Get the currently active sigblock set. */
  block = sigblock_unique();
 }

 {
  byte_t *iter,*end,*dst;
  /* Mask out all additional signals. */
  end = (iter = (byte_t *)&action->sa_mask)+sizeof(sigset_t);
  dst = (byte_t *)&block->sb_sigset;
  for (; iter != end; ++iter) *dst |= *iter;
 }

 /* Also block the signal itself when `SA_NODEFER' isn't set. */
 if (!(action->sa_flags & SA_NODEFER))
      __sigaddset(&block->sb_sigset,signo);

 /* ... Continue serving signals and RPC functions
  *     until the calling thread will eventually
  *     return to user-space. */
}


/* Handle (process) a signal. */
PRIVATE void KCALL
handle_signal(struct cpu_hostcontext_user *__restrict context,
              siginfo_t *__restrict info, bool is_thread_signal,
              unsigned int mode) {
 uintptr_t action_ptr; struct sighand *hand;
 struct sigaction *action,*action_copy;
 unsigned int signo = (unsigned int)info->si_signo-1;
 assert(signo < _NSIG);
 hand = sighand_lock_read();
 if (!hand) {
do_default_action:
  action_ptr = signal_default_actions[signo];
special_action:
  switch (action_ptr) {

  case SIGNAL_ACTION_IGNORE:
   break;

  case SIGNAL_ACTION_CONT:
   /* Unset the suspended-flag. */
   task_signal_event(CLD_CONTINUED,__W_CONTINUED);
   if (task_resume(THIS_TASK)) {
    if (!is_thread_signal) {
     /* Continue all other threads in this process. */
     taskgroup_queue_rpc(&resume_thread,NULL,
                          TASK_RPC_NORMAL|
                          TASK_RPC_SINGLE);
    }
   }
   break;

  {
  case SIGNAL_ACTION_STOP:
do_action_stop:
   task_signal_event(CLD_STOPPED,__W_STOPCODE(signo+1));
   if (!is_thread_signal) {
    /* Suspend all other threads in this process. */
    taskgroup_queue_rpc_user(&suspend_thread,NULL,
                              TASK_RPC_NORMAL|
                              TASK_RPC_USER);
   }
   task_suspend(JTIME_INFINITE);
  } break;

  case SIGNAL_ACTION_CORE:
#if 1
   goto do_action_kill;
#else
   /* TODO: Create a coredump. */
   break;
#endif

  {
   struct exception_info *reason;
  case SIGNAL_ACTION_EXIT:
   reason = error_info();
   reason->e_error.e_code = E_EXIT_THREAD;
   goto do_action_throw;

  default:
do_action_kill:
   /* Default action for unknown action codes exit the process. */
   reason = error_info();
   reason->e_error.e_code = E_EXIT_PROCESS;
do_action_throw:
   reason->e_error.e_flag = ERR_FNORMAL;
   memset(reason->e_error.e_pointers,0,sizeof(reason->e_error.e_pointers));
   reason->e_error.e_exit.e_status = __W_EXITCODE(0,signo+1);
   error_throw_current();
  } break;
  }
  return;
 } else if ((action = hand->sh_hand[signo]) == NULL) {
  atomic_rwlock_endread(&hand->sh_lock);
  goto do_default_action;
 }
 /* Force default behavior for these signals. */
 if (signo == SIGKILL) { atomic_rwlock_endread(&hand->sh_lock); goto do_action_kill; }
 if (signo == SIGSTOP) { atomic_rwlock_endread(&hand->sh_lock); goto do_action_stop; }

 /* Deal with one-time signal handlers. */
 if (action->sa_flags & SA_RESETHAND) {
  atomic_rwlock_endread(&hand->sh_lock);
  /* Temporarily acquire a write lock, so
   * we get a change to delete the handler. */
  hand = sighand_lock_write();
  COMPILER_READ_BARRIER();
  action = hand->sh_hand[signo];
  if unlikely(!action) {
   atomic_rwlock_endwrite(&hand->sh_lock);
   goto do_default_action;
  }
  if likely(action->sa_flags & SA_RESETHAND) {
   hand->sh_hand[signo] = NULL; /* Inherit. */
   atomic_rwlock_endwrite(&hand->sh_lock);
   /* Now just serve the action described by the signal. */
   action_ptr = (uintptr_t)action->sa_handler;
   if (SIGNAL_ISACTION(action_ptr)) {
    kfree(action);
    goto special_action;
   }
   TRY {
    /* Do the user-space signal redirection. */
    do_redirect_signal_action(context,signo,info,
                              action,mode);
   } FINALLY {
    kfree(action);
   }
   return;
  }
  atomic_rwlock_downgrade(&hand->sh_lock);
 }

 /* Load the user-space instruction pointer of the signal handler,
  * which then doubles as identifier for special signal actions. */
 action_ptr = (uintptr_t)action->sa_handler;
 /* If the action pointer is a special action, do that action instead. */
 if (SIGNAL_ISACTION(action_ptr)) { atomic_rwlock_endread(&hand->sh_lock); goto special_action; }

 /* Custom signal handler. -> Redirect the user-space
  * context to execute the user's signal handler. */
 action_copy = (struct sigaction *)alloca(sizeof(struct sigaction));
 memcpy(action_copy,action,sizeof(struct sigaction));
 atomic_rwlock_endread(&hand->sh_lock);
 /* Do the user-space signal redirection. */
 do_redirect_signal_action(context,signo,info,
                           action_copy,mode);
}



INTERN void KCALL
posix_signal_rpc(void *UNUSED(arg),
                 struct cpu_hostcontext_user *__restrict context,
                 unsigned int mode) {
 /* Check for signal handlers that are not being blocked
  * and invoke their associated actions, suspending or resuming
  * execution, terminating the process, or simply executing
  * a user-space signal handler (by redirecting `context'). */
 struct sigblock *block;
 struct sigpending *pending[2]; unsigned int i;
again:
 block = PERTASK_GET(_this_sigblock);
#ifdef CONFIG_SIGPENDING_TRACK_MASK
 pending[0] = PERTASK_GET(_this_sigpending_task);
#else
 pending[0] = &PERTASK(_this_sigpending_task);
#endif
 if (offsetof(struct sigshare,ss_pending) == 0) {
  pending[1] = &PERTASK_GET(_this_sigpending_proc)->ss_pending;
 } else {
  struct sigshare *share = PERTASK_GET(_this_sigpending_proc);
  pending[1] = share ? &share->ss_pending : NULL;
 }
 for (i = 0; i < COMPILER_LENOF(pending); ++i) {
  struct sigqueue **piter;
  struct sigqueue *EXCEPT_VAR iter;
  mutex_t *pending_lock;
  if (!pending[i]) continue; /* Skip empty pending sets. */
  pending_lock = &pending[i]->sp_lock;
  mutex_get(pending_lock);
  piter = &pending[i]->sp_queue;
  while ((iter = *piter) != NULL) {
   assert(iter->sq_info.si_signo != 0);
   assert(iter->sq_info.si_signo < _NSIG+1);
   if (!block ||
      (iter->sq_info.si_signo == SIGKILL+1) ||
      (iter->sq_info.si_signo == SIGSTOP+1) ||
      !__sigismember(&block->sb_sigset,iter->sq_info.si_signo-1)) {
    *piter = iter->sq_chain.le_next;
#ifdef CONFIG_SIGPENDING_TRACK_MASK
    {
     struct sigqueue *next;
     bool still_exists = false;
     next = iter->sq_chain.le_next;
     while (next) {
      if (next->sq_info.si_signo == iter->sq_info.si_signo) {
       still_exists = true;
       break;
      }
      next = next->sq_chain.le_next;
     }
     /* Unset this signal from the pending mask if it was the last of its kind. */
     if (!still_exists)
          __sigdelset(&pending[i]->sp_mask,iter->sq_info.si_signo-1);
    }
#endif
    break; /* Handle this signal. */
   }
   piter = &iter->sq_chain.le_next;
  }
  mutex_put(pending_lock);
  if (!iter) continue;
  /* Handle this signal and destroy its chain descriptor afterwards. */
  TRY {
   handle_signal(context,&iter->sq_info,i == 0,mode);
  } FINALLY {
   kfree(iter);
  }
  /* Search for more signals in this pending-set */
  goto again;
 }
}

PRIVATE void KCALL
signal_rpc_leader(void *arg,
                  struct cpu_hostcontext_user *__restrict context,
                  unsigned int mode) {
 /* Try to handle the RPC in the leader. */
 posix_signal_rpc(arg,context,mode);
 if (get_this_process() == THIS_TASK) {
  /* Send the RPC request to all members of this thread-group. */
  taskgroup_queue_rpc_user(&posix_signal_rpc,
                            NULL,
                            TASK_RPC_NORMAL|
                            TASK_RPC_USER|
                            TASK_RPC_SINGLE);
 }
}



/* Enqueue in `sigshare' and find some thread in `threadgroup'
 * that isn't blocking the signal, then post an RPC functions
 * to that thread which will search for pending signals that
 *       are not being ignored and handle them.
 * NOTE: If `threadgroup' isn't the leader of a group,
 *       the signal is send to its leader thread. */
PUBLIC bool KCALL
signal_raise_group(struct task *__restrict threadgroup,
                   USER CHECKED siginfo_t *__restrict info) {
 bool EXCEPT_VAR result = true;
 struct sigqueue *EXCEPT_VAR slot;
 struct sigpending *EXCEPT_VAR pending;
 pending = sigpending_getprocfor(threadgroup);
 /* Allocate the new slot. */
 slot = (struct sigqueue *)kmalloc(sizeof(struct sigqueue),
                                   GFP_SHARED);
 TRY {
  /* Copy user memory to the slot. */
  memcpy(&slot->sq_info,info,sizeof(siginfo_t));
  /* Assert that the signal number is valid. */
  if unlikely((unsigned int)slot->sq_info.si_signo == 0 ||
              (unsigned int)slot->sq_info.si_signo >= _NSIG+1)
     error_throw(E_INVALID_ARGUMENT);
  mutex_get(&pending->sp_lock);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(slot);
  error_rethrow();
 }
 /* Add the new slot to the pending-queue */
 SLIST_INSERT(pending->sp_queue,slot,sq_chain);

#ifdef CONFIG_SIGPENDING_TRACK_MASK
 /* Add the signal that was thrown to the pending mask. */
 __sigaddset(&pending->sp_mask,slot->sq_info.si_signo-1);
#endif
 TRY {
#if 0 /* Can't be done. - Would introduce a race condition since we can't
      *                  control if the target threadgroup changes its sigblock
      *                  pointer, or even de-allocates it (free-memory access). */
  /* Lazily check if the signal is being blocked (this check is repeated later on)
   * The only reason why we do this now is so we can omit having to schedule
   * an RPC function call which is much slower when we can already check if
   * the signal would just be ignored regardless. */
  struct sigblock *block;
  block = FORTASK(threadgroup,_this_sigblock);
  if (block && __sigismember(&block->sb_sigset,slot->sq_info.si_signo-1)) {
   /* Must still fail if the threadgroup has terminated, or has started to. */
   result = !(ATOMIC_READ(threadgroup->t_state) & TASK_STATE_FTERMINATING);
  } else
#endif
  {
   /* Queue a user-RPC function call for the thread group leader. */
   result = task_queue_rpc_user(threadgroup,
                               &signal_rpc_leader,
                                NULL,
                                TASK_RPC_NORMAL|
                                TASK_RPC_SINGLE|
                                TASK_RPC_USER);
  }
 } FINALLY {
  assert(slot == pending->sp_queue);
  if unlikely(!result) {
   /* Remove the slot if the queue failed. */
   pending->sp_queue = slot->sq_chain.le_next;
   kfree(slot);
  }
  COMPILER_WRITE_BARRIER();
  mutex_put(&pending->sp_lock);
  /* Broadcast that a new signals has been added. */
  sig_broadcast(&pending->sp_newsig);
 }
 return result;
}


/* Using RPC function calls, raise a signal in the given thread.
 * If `thread' is the calling thread, this function will not return.
 * @return: true:  The RPC was scheduled for delivery and will be executed (asynchronously).
 * @return: false: The RPC call failed because `thread' has terminated.
 * @throw: E_SEGFAULT: The user-buffers are faulty. */
PUBLIC bool KCALL
signal_raise_thread(struct task *__restrict thread,
                    USER CHECKED siginfo_t *__restrict info) {
 bool EXCEPT_VAR result = true;
 struct sigqueue *EXCEPT_VAR slot;
 struct sigpending *EXCEPT_VAR pending;
 pending = sigpending_getfor(thread);
 /* Allocate the new slot. */
 slot = (struct sigqueue *)kmalloc(sizeof(struct sigqueue),
                                   GFP_SHARED);
 TRY {
  /* Copy user memory to the slot. */
  memcpy(&slot->sq_info,info,sizeof(siginfo_t));
  /* Assert that the signal number is valid. */
  if unlikely((unsigned int)slot->sq_info.si_signo == 0 ||
              (unsigned int)slot->sq_info.si_signo >= _NSIG+1)
     error_throw(E_INVALID_ARGUMENT);
  mutex_get(&pending->sp_lock);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(slot);
  error_rethrow();
 }
 /* Add the new slot to the pending-queue */
 SLIST_INSERT(pending->sp_queue,slot,sq_chain);

#ifdef CONFIG_SIGPENDING_TRACK_MASK
 /* Add the signal that was thrown to the pending mask. */
 __sigaddset(&pending->sp_mask,slot->sq_info.si_signo-1);
#endif

 TRY {
#if 0 /* Can't be done. - Would introduce a race condition since we can't
      *                  control if the target thread changes its sigblock
      *                  pointer, or even de-allocates it (free-memory access). */
  /* Lazily check if the signal is being blocked (this check is repeated later on)
   * The only reason why we do this now is so we can omit having to schedule
   * an RPC function call which is much slower when we can already check if
   * the signal would just be ignored regardless. */
  struct sigblock *block;
  block = FORTASK(thread,_this_sigblock);
  if (block && __sigismember(&block->sb_sigset,slot->sq_info.si_signo-1)) {
   /* Must still fail if the thread has terminated, or has started to. */
   result = !(ATOMIC_READ(thread->t_state) & TASK_STATE_FTERMINATING);
  } else
#endif
  {
   /* Queue a user-RPC function call for the given thread. */
   result = task_queue_rpc_user(thread,
                               &posix_signal_rpc,
                                NULL,
                                TASK_RPC_NORMAL|
                                TASK_RPC_SINGLE|
                                TASK_RPC_USER);
  }
 } FINALLY {
  assert(slot == pending->sp_queue);
  if unlikely(!result) {
   /* Remove the slot if the queue failed. */
   pending->sp_queue = slot->sq_chain.le_next;
   kfree(slot);
  }
  mutex_put(&pending->sp_lock);
  /* Broadcast that a new signals has been added. */
  sig_broadcast(&pending->sp_newsig);
 }
 return result;
}


/* Test for pending signals in the calling thread. */
PRIVATE void KCALL signal_test(void) {
 /* Check for signal handlers that are not being blocked
  * and invoke their associated actions, suspending or resuming
  * execution, terminating the process, or simply executing
  * a user-space signal handler (by redirecting `context'). */
 struct sigblock *block;
 struct sigpending *pending[2]; unsigned int i;
 block = PERTASK_GET(_this_sigblock);
#ifdef CONFIG_SIGPENDING_TRACK_MASK
 pending[0] = PERTASK_GET(_this_sigpending_task);
#else
 pending[0] = &PERTASK(_this_sigpending_task);
#endif
 if (offsetof(struct sigshare,ss_pending) == 0) {
  pending[1] = &PERTASK_GET(_this_sigpending_proc)->ss_pending;
 } else {
  struct sigshare *share = PERTASK_GET(_this_sigpending_proc);
  pending[1] = share ? &share->ss_pending : NULL;
 }
 for (i = 0; i < COMPILER_LENOF(pending); ++i) {
  struct sigqueue *iter;
  mutex_t *pending_lock;
  if (!pending[i]) continue; /* Skip empty pending sets. */
  pending_lock = &pending[i]->sp_lock;
  mutex_get(pending_lock);
  iter = pending[i]->sp_queue;
  for (; iter; iter = iter->sq_chain.le_next) {
   if (!block ||
      (iter->sq_info.si_signo == SIGKILL+1) ||
      (iter->sq_info.si_signo == SIGSTOP+1) ||
      !__sigismember(&block->sb_sigset,iter->sq_info.si_signo-1)) {
    mutex_put(pending_lock);
    /* Return to userspace immediately to check for signals. */
    task_queue_rpc_user(THIS_TASK,&posix_signal_rpc,NULL,TASK_RPC_USER);
    __builtin_unreachable();
   }
  }
  mutex_put(pending_lock);
 }
}

/* Change the signal mask of the calling thread.
 * Following this, check for signals that have been unblocked
 * and throw an `E_INTERRUPT' exception if any were.
 * When `old_mask' is non-NULL, save the old mask inside
 * before changing anything.
 * @throw: E_INVALID_ARGUMENT: The given `sigsetsize' is too large.
 * @throw: E_SEGFAULT:         The user-buffers are faulty. */
PUBLIC void KCALL
signal_chmask(USER CHECKED sigset_t const *mask,
              USER CHECKED sigset_t *old_mask,
              size_t sigsetsize, int how) {
 struct sigblock *block;
 byte_t *iter,*end,*src;
 if unlikely(sigsetsize > sizeof(sigset_t))
    error_throw(E_INVALID_ARGUMENT);
 if (!mask) {
  /* Only copy the current mask to user-space. */
  if (!old_mask) return;
  block = PERTASK_GET(_this_sigblock);
  if (!block) {
   memset(old_mask,0,sigsetsize);
  } else {
   /* Don't confuse the caller by keeping these bits set.
    * In actuality, they are checked for specifically (and ignored)
    * when signals are being tested. */
   __sigdelset(&block->sb_sigset,SIGKILL);
   __sigdelset(&block->sb_sigset,SIGSTOP);
   memcpy(old_mask,&block->sb_sigset,sigsetsize);
  }
  return;
 }
 block = sigblock_unique();
 if (old_mask) {
  /* Same as above: Don't confuse the user. */
  __sigdelset(&block->sb_sigset,SIGKILL);
  __sigdelset(&block->sb_sigset,SIGSTOP);
  memcpy(old_mask,&block->sb_sigset,sigsetsize);
 }
 src  = (byte_t *)mask;
 iter = (byte_t *)&block->sb_sigset;
 end  = iter + sigsetsize;
 switch (how) {

 case SIGNAL_CHMASK_FBLOCK:
  /* Add bits. */
  for (; iter != end; ++iter,++src)
      *iter |= *src;
  break;

 case SIGNAL_CHMASK_FUNBLOCK:
  /* Mask bits. */
  for (; iter != end; ++iter,++src)
      *iter &= ~*src;
  break;

 case SIGNAL_CHMASK_FSETMASK:
  /* Set bits. */
  memcpy(iter,src,sigsetsize);
  break;

 default:
  error_throw(E_INVALID_ARGUMENT);
  break;
 }
 /* Test for signals that have been unblocked. */
 signal_test();
}




/* Change the action for the given `signo'.
 * @throw: E_SEGFAULT:The user-buffers are faulty. */
PUBLIC void KCALL
signal_chaction(int signo,
                USER CHECKED struct sigaction const *new_action,
                USER CHECKED struct sigaction *old_action) {
 struct sigaction *EXCEPT_VAR kernel_new_action;
 struct sigaction *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(kernel_old_action);
 struct sighand *hand;
 assert(signo < _NSIG);
 if (!new_action) {
  if (!old_action) return;
  /* Only copy the old action into the `old_action' buffer. */
  hand = sighand_lock_read();
  if (!hand) goto old_default_action;
  kernel_old_action = hand->sh_hand[signo];
  if (!kernel_old_action) {
   atomic_rwlock_endread(&hand->sh_lock);
   goto old_default_action;
  }
  /* We can't directly copy the old action to user-space
   * because the atomic lock used here doesn't allow for
   * recursion that may be caused by #PF handlers, or if
   * the given user-buffer is faulty.
   * Instead, create a quick copy on our stack using alloca() */
  kernel_new_action = (struct sigaction *)alloca(sizeof(struct sigaction));
  memcpy(kernel_new_action,kernel_old_action,sizeof(struct sigaction));
  atomic_rwlock_endread(&hand->sh_lock);
  /* Now copy the action to user-space. */
  memcpy(old_action,kernel_new_action,sizeof(struct sigaction));
  return;
old_default_action:
  memset(old_action,0,sizeof(struct sigaction));
  return;
 }
 kernel_new_action = (struct sigaction *)kmalloc(sizeof(struct sigaction),
                                                 GFP_SHARED);
 TRY {
  /* Copy the new action into a heap-allocation sigaction descriptor. */
  memcpy(kernel_new_action,new_action,
         sizeof(struct sigaction));
  COMPILER_WRITE_BARRIER();
  if (new_action->sa_handler == SIG_DFL) {
   /* Special case: Restore default action (by setting a NULL handler) */
   kfree(kernel_new_action);
   kernel_new_action = NULL;
  }
  /* Acquire a write-lock to the effective signal handler descriptor. */
  hand = sighand_lock_write();
  kernel_old_action    = hand->sh_hand[signo];
  hand->sh_hand[signo] = kernel_new_action;
  atomic_rwlock_endwrite(&hand->sh_lock);
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  kfree(kernel_new_action);
  error_rethrow();
 }
 if (!kernel_old_action) {
  if (old_action)
      goto old_default_action;
 } else {
  TRY {
   if (old_action)
       memcpy(old_action,kernel_old_action,sizeof(struct sigaction));
  } FINALLY {
   kfree(kernel_old_action);
  }
 }
}


/* Change the signal disposition of all signal actions using
 * a user-space disposition to `SIG_DFL'. (called during exec()) */
PUBLIC void KCALL signal_resetexec(void) {
 struct sighand *hand;
 bool has_write_lock = false;
 struct sigaction *old_actions[_NSIG];
 unsigned int i;
 unsigned int num_del_actions = 0;
 unsigned int num_keep_actions;
 hand = sighand_lock_read();
 if (!hand) return; /* Nothing to do here... */
again:
 num_keep_actions = 0;
 for (i = 0; i < _NSIG; ++i) {
  struct sigaction *act;
  act = hand->sh_hand[i];
  if (!act) continue;
  if ((uintptr_t)act->sa_handler <=
      (uintptr_t)__SIG_SPECIAL_MAX) {
   /* Keep special signal actions. */
   ++num_keep_actions;
   continue;
  }
  if (!has_write_lock) {
   has_write_lock = true;
   /* Acquire a write-lock and start over (because
    * things may have changed in the mean time) */
   atomic_rwlock_endread(&hand->sh_lock);
   hand = sighand_lock_write();
   assertf(hand,"sighand_lock_write() returns non-NULL");
   goto again;
  }
  /* Delete custom actions. */
  old_actions[num_del_actions] = act;
  hand->sh_hand[i] = NULL;
  ++num_del_actions;
 }
 if (has_write_lock) {
  atomic_rwlock_endwrite(&hand->sh_lock);
#if 1
  if (!num_keep_actions) {
   /* We can delete the handler vector, as it
    * doesn't contain anything of interest anymore. */
   struct sighand_ptr *ptr;
   ptr = PERTASK_GET(_this_sighand_ptr);
   atomic_rwlock_write(&ptr->sp_lock);
   if (ptr->sp_hand == hand && ptr->sp_refcnt == 1) {
    /* Delete it. (We're allowed to ZERO out `sp_hand' because
     * we're the only thread using this pointer (`sp_refcnt == 1')) */
    ptr->sp_hand = NULL;
    atomic_rwlock_endwrite(&ptr->sp_lock);
    /* Drop the reference previously held by `ptr->sp_hand' */
    sighand_decref(hand);
   } else {
    atomic_rwlock_endwrite(&ptr->sp_lock);
   }
  }
#endif
  /* Free all the old actions which we've extracted. */
  while (num_del_actions--)
      kfree(old_actions[num_del_actions]);
 } else {
  assert(!num_del_actions);
  atomic_rwlock_endread(&hand->sh_lock);
 }
}



/* Deque a pending signal from `pending' that is matching `mask'.
 * Otherwise, return `false' after connecting to `pending->sp_newsig' */
PRIVATE bool KCALL
deque_pending_signal(struct sigpending *__restrict EXCEPT_VAR self,
                     sigset_t const *__restrict mask,
                     USER CHECKED siginfo_t *info) {
 bool result = false;
 struct sigqueue **pitem,*item;
 mutex_get(&self->sp_lock);
 TRY {
  /* Check for pending signals. */
  pitem = &self->sp_queue;
  while ((item = *pitem) != NULL) {
   int signo = item->sq_info.si_signo-1;
   if (__sigismember(mask,signo)) {
#ifdef CONFIG_SIGPENDING_TRACK_MASK
    bool still_exists = false;
#endif
    /* Found one! (copy its data; caution: SEGFAULT) */
    memcpy(info,&item->sq_info,sizeof(siginfo_t));

    COMPILER_BARRIER();
    *pitem = item->sq_chain.le_next;
#ifdef CONFIG_SIGPENDING_TRACK_MASK
    {
     struct sigqueue *iter;
     iter = item->sq_chain.le_next;
     while (iter) {
      if (iter->sq_info.si_signo-1 == signo) {
       still_exists = true;
       break;
      }
      iter = iter->sq_chain.le_next;
     }
    }
#endif
    kfree(item);
#ifdef CONFIG_SIGPENDING_TRACK_MASK
    /* Unset the bit of this signal in the pending set. */
    if (!still_exists)
         __sigdelset(&self->sp_mask,signo);
#endif
    result = true;
    goto done;
   }
   pitem = &item->sq_chain.le_next;
  }
  task_connect(&self->sp_newsig);
done:;
 } FINALLY {
  mutex_put(&self->sp_lock);
 }
 return result;
}

/* Wait for one of the signals specified in `wait_mask' to become
 * available within the calling thread's pending list, or the
 * shared pending list of the calling thread.
 * @throw: E_INVALID_ARGUMENT: The given `sigsetsize' is too large.
 * @throw: E_SEGFAULT:         The user-buffers are faulty.
 * @return: true:  A Signal was received and stored in `info'
 * @return: false: The given `abs_timeout' has expired. */
PUBLIC bool KCALL
signal_waitfor(USER CHECKED sigset_t const *wait_mask, size_t sigsetsize,
               USER CHECKED siginfo_t *info, jtime_t abs_timeout) {
 sigset_t user_mask;
 struct sigpending *sigtask,*sigproc;
 assert(task_isconnected());
 if unlikely(sigsetsize > sizeof(sigset_t))
    error_throw(E_INVALID_ARGUMENT);
 /* Copy the wait mask from user-space. */
 memcpy(&user_mask,wait_mask,sigsetsize);
 if unlikely(sigsetsize != sizeof(sigset_t))
    memset((byte_t *)&user_mask+sigsetsize,
            0,sizeof(sigset_t)-sigsetsize);
 COMPILER_READ_BARRIER();
 /* Get the pending sets of signal which we're supposed to listen to. */
 sigtask = sigpending_gettask();
 sigproc = sigpending_getproc();

 for (;;) {
  /* Check for pending signals and connect to their signals. */
  if (deque_pending_signal(sigtask,&user_mask,info)) return true;
  if (deque_pending_signal(sigproc,&user_mask,info)) { task_disconnect(); return true; }
  /* Wait for signals to become available. */
  if (!task_waitfor(abs_timeout)) break;
 }
 return false;
}


/* Similar to `signal_waitfor()', but replace the calling thread's
 * signal mask with `wait_mask' for the duration of this call,
 * then proceeds to wait until one of the signals masked has been
 * queued for execution in the calling thread.
 * NOTE: This function must not be called for kernel-worker threads.
 * @return: void: The given timeout has expired.
 * @throw: * :                 An RPC function threw this error...
 * @throw: E_INTERRUPT:        The calling thread was interrupted (Dealt with before transitioning to user-space)
 * @throw: E_INVALID_ARGUMENT: The given `sigsetsize' is too large. */
PUBLIC void KCALL
signal_suspend(USER CHECKED sigset_t const *wait_mask,
               size_t sigsetsize, jtime_t abs_timeout) {
 sigset_t old_mask;
 struct sigblock *EXCEPT_VAR block;
 assertf(!task_isconnected(),
         "This is for posix-signals only. Don't be "
         "connected to KOS signals (they're different things)");
 assertf(!PERTASK_TEST(this_task.t_nothrow_serve),
         "Without this, RPC functions won't be able to throw errors");
 block = sigblock_unique();
 if unlikely(sigsetsize > sizeof(sigset_t))
    error_throw(E_INVALID_ARGUMENT);
 /* Save the old mask */
 memcpy(&old_mask,&block->sb_sigset,sizeof(sigset_t));
 TRY {
  /* Set the given signal-mask. */
  memcpy(&block->sb_sigset,wait_mask,sigsetsize);
  if unlikely(sigsetsize != sizeof(sigset_t))
     memset((byte_t *)&block->sb_sigset+sigsetsize,
             0,sizeof(sigset_t)-sigsetsize);

  /* That's pretty much it.
   * >> `task_sleep()' will continually serve RPC functions,
   *     meaning that once a signal has been sent, the RPC
   *     function will throw an `E_INTERRUPT' error that's
   *     going to be propagated through our FINALLY before
   *     further moving along the stack and through the caller.
   * Intermitten sporadic wakeups (as caused by random RPC functions)
   * are implicitly handled by the while-loop. */
  for (;;) {
   PREEMPTION_DISABLE();
   if (task_serve()) continue;
   if (!task_sleep(abs_timeout)) break;
  }
 } FINALLY {
  memcpy(&block->sb_sigset,&old_mask,sizeof(sigset_t));
 }
}

PUBLIC void KCALL
signal_pending(USER CHECKED sigset_t *wait_mask,
               size_t sigsetsize) {
#ifdef CONFIG_SIGPENDING_TRACK_MASK
 struct sigpending *pending;
 struct sigshare *share;
 byte_t *iter,*end,*src;
 if unlikely(sigsetsize > sizeof(sigset_t))
    error_throw(E_INVALID_ARGUMENT);
 /* Clear all bits in the user-space mask. */
 memset(wait_mask,0,sigsetsize);
 COMPILER_WRITE_BARRIER();
 end = (iter = (byte_t *)wait_mask)+sigsetsize;
 pending = PERTASK_GET(_this_sigpending_task);
 if (pending) {
  src = (byte_t *)&pending->sp_mask;
  for (; iter != end; ++iter,++src)
      *iter |= *src;
 }
 share = PERTASK_GET(_this_sigpending_proc);
 if (share) {
  iter = (byte_t *)wait_mask;
  src = (byte_t *)&share->ss_pending.sp_mask;
  for (; iter != end; ++iter,++src)
      *iter |= *src;
 }
#else
 struct sigpending *EXCEPT_VAR pending[2];
 struct sigqueue *item; unsigned int i;
 if unlikely(sigsetsize > sizeof(sigset_t))
    error_throw(E_INVALID_ARGUMENT);
 /* Clear all bits in the user-space mask. */
 memset(wait_mask,0,sigsetsize);
 COMPILER_WRITE_BARRIER();
 pending[0] = &PERTASK(_this_sigpending_task);
 if (offsetof(struct sigshare,ss_pending) == 0) {
  pending[1] = &PERTASK_GET(_this_sigpending_proc)->ss_pending;
 } else {
  struct sigshare *share = PERTASK_GET(_this_sigpending_proc);
  pending[1] = share ? &share->ss_pending : NULL;
 }
 for (i = 0; i < COMPILER_LENOF(pending); ++i) {
  if (!pending[i]) continue;
  mutex_get(&pending[i]->sp_lock);
  TRY {
   item = pending[i]->sp_queue;
   for (; item; item = item->sq_chain.le_next) {
    /* Add the signal to the user-space set (WARNING: SEGFAULT) */
    __sigaddset(wait_mask,item->sq_info.si_signo-1);
   }
  } FINALLY { /* Finally required because of segfautls. */
   mutex_put(&pending[i]->sp_lock);
  }
 }
#endif
}





/*
#define __NR_kill         129
#define __NR_tkill        130
#define __NR_tgkill       131
*/


DEFINE_SYSCALL4(sigaction,unsigned int,sig,
                UNCHECKED USER struct sigaction const *,act,
                UNCHECKED USER struct sigaction *,oact,
                size_t,sigsetsize) {
 if (!sig || --sig >= _NSIG)
     error_throw(E_INVALID_ARGUMENT);
 if (sigsetsize != sizeof(sigset_t))
     error_throw(E_INVALID_ARGUMENT);
 /* Validate user-structure pointers. */
 validate_readable_opt(act,sizeof(struct sigaction));
 validate_writable_opt(oact,sizeof(struct sigaction));
 /* Change the signal action. */
 signal_chaction(sig,act,oact);
 return 0;
}

DEFINE_SYSCALL4(sigprocmask,int,how,
                UNCHECKED USER sigset_t const *,set,
                UNCHECKED USER sigset_t *,oldset,
                size_t,sigsetsize) {
 /* Validate user-structure pointers. */
 validate_readable_opt(set,sigsetsize);
 validate_writable_opt(oldset,sigsetsize);
 /* Change the signal mask. */
 signal_chmask(set,oldset,sigsetsize,how);
 pagedir_syncall();
 return 0;
}

DEFINE_SYSCALL4(sigtimedwait,
                UNCHECKED USER sigset_t const *,uthese,
                UNCHECKED USER siginfo_t *,uinfo,
                UNCHECKED USER struct timespec const *,uts,
                size_t,sigsetsize) {
 jtime_t timeout;
 /* Validate user-structure pointers. */
 validate_readable(uthese,sigsetsize);
 validate_writable(uinfo,sizeof(siginfo_t));
 validate_readable_opt(uts,sizeof(struct timespec));
 /* Calculate the timeout in jiffies. */
 timeout = uts ? jiffies+jiffies_from_timespec(*uts) : JTIME_INFINITE;
 /* Wait for signals to arrive. */
 if (!signal_waitfor(uthese,sigsetsize,uinfo,timeout))
      return -EAGAIN; /* Posix says EAGAIN for this. */
 return uinfo->si_signo;
}

DEFINE_SYSCALL2(sigsuspend,
                UNCHECKED USER sigset_t const *,unewset,
                size_t,sigsetsize) {
 /* Validate the user-space signal set pointer. */
 validate_readable(unewset,sigsetsize);
 signal_suspend(unewset,sigsetsize,JTIME_INFINITE);
 /* `signal_suspend()' doesn't return because of `JTIME_INFINITE' */
 __builtin_unreachable();
}
DEFINE_SYSCALL2(sigpending,
                UNCHECKED USER sigset_t *,uset,
                size_t,sigsetsize) {
 /* Validate the user-space signal set pointer. */
 validate_writable(uset,sigsetsize);
 signal_pending(uset,sigsetsize);
 return 0;
}


PRIVATE pid_t KCALL
view_for(struct task *__restrict thread) {
 struct thread_pid *p = THIS_THREAD_PID;
 struct thread_pid *other_p = FORTASK(thread,_this_pid);
 if (!p || !other_p) return 0;
 return thread_pid_view(p,other_p->tp_ns);
}


DEFINE_SYSCALL2(kill,pid_t,pid,int,sig) {
 REF struct task *EXCEPT_VAR target;
 siginfo_t info;
 memset(&info,0,sizeof(siginfo_t));
 /* Make sure we've been given a valid signal number. */
 if (sig <= 0 || sig >= _NSIG+1)
     return -EINVAL;
 info.si_signo = sig;
 info.si_errno = 0;
 info.si_code  = SI_USER;
 //info.si_uid = ...; /* TODO: THIS_RUID */
 if (pid > 0) {
  /* Kill the thread matching `pid'. */
  target = pid_lookup_task(pid);
  TRY {
   info.si_pid = view_for(target);
   if (!signal_raise_thread(target,&info))
        error_throw(E_PROCESS_EXITED);
  } FINALLY {
   task_decref(target);
  }
 } else if (pid == 0) {
  /* Kill all processes in the calling thread's process group. */
  info.si_pid = view_for(get_this_process());
  if (!signal_raise_pgroup(get_this_process(),&info))
       error_throw(E_PROCESS_EXITED);
 } else if (pid == -1) {
  /* TODO: Kill all processes that we're allowed to. */
 } else {
  /* Kill the entirety of a process group. */
  target = pid_lookup_task(-pid);
  TRY {
   info.si_pid = view_for(target);
   if (!signal_raise_pgroup(target,&info))
        error_throw(E_PROCESS_EXITED);
  } FINALLY {
   task_decref(target);
  }
 }
 return 0;
}

PRIVATE void KCALL
do_tkill(pid_t tgid, pid_t pid, int sig) {
 siginfo_t info;
 REF struct task *EXCEPT_VAR target;
 if (sig < 0 || sig >= _NSIG+1)
     error_throw(E_INVALID_ARGUMENT);
 memset(&info,0,sizeof(siginfo_t));
 info.si_signo = sig;
 info.si_errno = 0;
 info.si_code  = SI_TKILL;
 //info.si_uid = ...; /* TODO: THIS_RUID */
 target = pid_lookup_task(pid);
 TRY {
  /* Check if the given TGID matches the group of this thread. */
  if (tgid <= 0 || posix_gettid_view(FORTASK(target,_this_group).tg_leader) == tgid) {
   info.si_pid = view_for(target);
   /* Don't deliver signal `0'. - It's used to test access. */
   if (sig != 0) {
    if (!signal_raise_thread(target,&info))
         error_throw(E_PROCESS_EXITED);
   }
  }
 } FINALLY {
  task_decref(target);
 }
}

DEFINE_SYSCALL3(tgkill,pid_t,tgid,pid_t,pid,int,sig) {
 if (pid <= 0 || tgid <= 0)
     error_throw(E_INVALID_ARGUMENT);
 do_tkill(tgid,pid,sig);
 return 0;
}
DEFINE_SYSCALL2(tkill,pid_t,pid,int,sig) {
 if (pid <= 0)
     error_throw(E_INVALID_ARGUMENT);
 do_tkill(0,pid,sig);
 return 0;
}

DEFINE_SYSCALL3(rt_sigqueueinfo,
                pid_t,tgid,int,sig,
                siginfo_t const *,uinfo) {
 REF struct task *EXCEPT_VAR target;
 siginfo_t info;
 memcpy(&info,uinfo,sizeof(siginfo_t));
 info.si_signo = sig;
 target = pid_lookup_task(tgid);
 /* Don't allow sending arbitrary signals to other processes. */
 if ((info.si_code >= 0 || info.si_code == SI_TKILL) &&
     (FORTASK(target,_this_group).tg_leader != get_this_process())) {
  task_decref(target);
  return -EPERM;
 }
 TRY {
  if (sig && !signal_raise_group(target,&info))
      error_throw(E_PROCESS_EXITED);
 } FINALLY {
  task_decref(target);
 }
 return 0;
}
DEFINE_SYSCALL4(rt_tgsigqueueinfo,
                pid_t,tgid,pid_t,tid,int,sig,
                siginfo_t const *,uinfo) {
 REF struct task *EXCEPT_VAR target;
 struct task *leader;
 struct thread_pid *leader_pid;
 siginfo_t info;
 memcpy(&info,uinfo,sizeof(siginfo_t));
 info.si_signo = sig;
 target = pid_lookup_task(tid);
 /* Don't allow sending arbitrary signals to other processes. */
 leader = FORTASK(target,_this_group).tg_leader;
 if ((info.si_code >= 0 || info.si_code == SI_TKILL) &&
     (leader != get_this_process())) {
  task_decref(target);
  return -EPERM;
 }
 TRY {
  /* Check if the thread-group ID matches that of the leader of the requested thread-group. */
  leader_pid = FORTASK(leader,_this_pid);
  if (leader_pid &&
      leader_pid->tp_pids[THIS_THREAD_PID ? THIS_THREAD_PID->tp_ns->pn_indirection : 0] != tgid)
      error_throw(E_PROCESS_EXITED);
  if (sig && !signal_raise_thread(target,&info))
      error_throw(E_PROCESS_EXITED);
 } FINALLY {
  task_decref(target);
 }
 return 0;
}


#define __NR_sigaltstack  132
#define __NR_sigreturn    139

DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_POSIX_SIGNALS_C */
