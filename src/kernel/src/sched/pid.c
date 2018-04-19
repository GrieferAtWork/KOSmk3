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
#ifndef GUARD_KERNEL_SRC_SCHED_PID_C
#define GUARD_KERNEL_SRC_SCHED_PID_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <kos/types.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/syscall.h>
#include <kernel/debug.h>
#include <kernel/sections.h>
#include <kernel/malloc.h>
#include <sched/task.h>
#include <sched/pid.h>
#include <sched/group.h>
#include <sched/posix_signals.h>
#include <bits/siginfo.h>
#include <bits/sched.h>
#include <string.h>
#include <except.h>
#include <stdlib.h>

DECL_BEGIN

/* [0..1][const] PID Descriptor of this thread (or `NULL' for kernel workers, or
 *               hacky threads that don't have a PID (cannot be created in userspace)) */
PUBLIC ATTR_PERTASK REF struct thread_pid *_this_pid = NULL;


PUBLIC ATTR_NOTHROW REF struct task *KCALL
thread_pid_get(struct thread_pid *__restrict self) {
 REF struct task *result;
 atomic_rwlock_read(&self->tp_task_lock);
 result = self->tp_task;
 if (result && !ATOMIC_INCIFNONZERO(result->t_refcnt))
     result = NULL;
 atomic_rwlock_endread(&self->tp_task_lock);
 if (result) {
  u16 state;
  for (;;) {
   state = ATOMIC_READ(result->t_state);
   if (state & TASK_STATE_FSTARTED) break;
   if (state & TASK_STATE_FTERMINATED) {
    /* Terminated before actually started (s.a. `task_failed()') */
    task_decref(result);
    result = NULL;
    break;
   }
   assert(!(state & TASK_STATE_FSLEEPING));
   /* Let whoever is in charge finish setting up the task. */
   task_yield();
  }
 }
 return result;
}



/* Signal an event in to someone wait()-ing for the calling thread.
 * This function wakes a blocking `wait()' call, as well as sends
 * a signal to parent process of the calling thread group.
 * @param: event_code: One of `CLD_*' from <bits/siginfo.h>
 * @param: status:     Signal status (s.a. `union wait' from <bits/waitstatus.h>) */
PUBLIC void KCALL task_signal_event(int event_code, int status) {
 struct thread_pid *pid = THIS_THREAD_PID;
 REF struct task *parent;
 struct task *leader,*parent_leader;
 struct thread_pid *parent_pid;
 siginfo_t taskevent;
 assert(event_code != 0);

 if (!pid) return; /* Cannot signal parent without PID descriptor. */
 if (!pid->tp_siblings.le_pself) return; /* Not part of the sibling chain (lazy check). */
 parent = task_get_parent();
 if (!parent) return; /* Parent is gone, or there is no parent. */
 assert(parent->t_refcnt != 0);
 memset(&taskevent,0,sizeof(siginfo_t));
 parent_leader       = FORTASK(parent,_this_group).tg_leader;
 leader              = get_this_process();
 taskevent.si_signo  = FORTASK(leader,_this_group).tg_process.h_sigcld;
 taskevent.si_code   = event_code;
 parent_pid          = FORTASK(parent_leader,_this_pid);
 taskevent.si_pid    = pid->tp_pids[(parent_pid ? parent_pid->tp_ns : &pidns_kernel)->pn_indirection];
 taskevent.si_status = status;
//taskevent.si_uid   = ...; /* TODO: THIS_RUID */

 /* Save the new state reason. */
 atomic_rwlock_write(&pid->tp_task_lock);
 assert(pid->tp_task == THIS_TASK);
 pid->tp_event           = event_code;
 pid->tp_status.w_status = status;
 atomic_rwlock_endwrite(&pid->tp_task_lock);

 /* Broadcast that a child process changed state to the parent. */
 sig_broadcast(&FORTASK(parent_leader,_this_group).tg_process.h_cldevent);

 /* Send a signal to the parent process. */
 if (taskevent.si_signo != 0) {
  TRY {
   signal_raise_group(parent_leader,&taskevent);
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   error_printf("Failed to signal parent process about termination\n");
  }
 }
 task_decref(parent);
}


/* The kernel's own PID namespace.
 * This is the root namespace that is used by the kernel itself,
 * as well as /bin/init (and usually most other processes, too)
 * NOTE: This is the _only_ PID namespace with an indirection of ZERO(0)! */
PUBLIC struct pidns pidns_kernel = {
    .pn_refcnt      = 1,
    .pn_indirection = 0,
    .pn_parent      = NULL,
    .pn_lock        = ATOMIC_RWLOCK_INIT,
    .pn_used        = 0,
    .pn_size        = 0,
    .pn_mask        = 0,
    .pn_list        = NULL
};

PUBLIC void KCALL
pidns_destroy(struct pidns *__restrict self) {
 size_t i;
 assert(self != &pidns_kernel);
 assert(self->pn_parent != NULL);
 assert(self->pn_indirection != 0);
 if (self->pn_list) {
  for (i = 0; i <= self->pn_mask; ++i) {
   if (self->pn_list[i] &&
       self->pn_list[i] != PIDNS_DUMMY)
       thread_pid_decref(self->pn_list[i]);
  }
  kfree(self->pn_list);
 }
 pidns_decref(self->pn_parent);
 kfree(self);
}

/* Allocate a new child PID namespace for `parent' */
PUBLIC ATTR_RETNONNULL ATTR_MALLOC REF struct pidns *KCALL
pidns_alloc(struct pidns *__restrict parent) {
 REF struct pidns *result;
 result = (REF struct pidns *)kmalloc(sizeof(struct pidns),
                                      GFP_SHARED|GFP_CALLOC);
 result->pn_refcnt      = 1;
 result->pn_indirection = parent->pn_indirection+1;
 result->pn_parent      = parent;
 pidns_incref(parent);
 atomic_rwlock_cinit(&result->pn_lock);
 return result;
}

PRIVATE void KCALL
pidns_addpid(struct pidns *__restrict self,
             struct thread_pid *__restrict pid,
             pid_t pid_value) {
 unsigned int random_bits;
 size_t i,perturb;
again:
 atomic_rwlock_write(&self->pn_lock);
 assert(self->pn_size <= self->pn_mask);
 if unlikely(self->pn_size >= (self->pn_mask >> 1)) {
  /* Try to rehash the PID namespace. */
  WEAK struct thread_pid **EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(new_list);
  size_t new_mask = (self->pn_mask << 1)|1;
  if unlikely(new_mask < 15) new_mask = 15;
  atomic_rwlock_endwrite(&self->pn_lock);
  TRY {
   new_list = (WEAK struct thread_pid **)kmalloc((new_mask+1)*sizeof(struct thread_pid *),
                                                  GFP_SHARED|GFP_CALLOC);
  } CATCH (E_BADALLOC) {
   new_list = NULL;
  }
  atomic_rwlock_write(&self->pn_lock);
  if likely(new_list) {
   WEAK struct thread_pid **old_list;
   if unlikely(new_mask <= self->pn_mask) {
    /* Race condition: Another thread rehashed the
     * namespace while we were allocating our new vector. */
    atomic_rwlock_endwrite(&self->pn_lock);
    kffree(new_list,GFP_CALLOC);
    goto again;
   }
   /* Rehash the namespace and set the new mask. */
   old_list = self->pn_list;
   if (old_list) {
    size_t j;
    for (j = 0; j <= self->pn_mask; ++j) {
     struct thread_pid *entry;
     entry = old_list[j];
     if (!entry) continue; /* Unused entry. */
     if (entry == PIDNS_DUMMY) continue; /* Dummy entry. */
     perturb = i = (size_t)entry->tp_pids[self->pn_indirection] & new_mask;
     for (;; i = ((i << 2) + i + perturb + 1),perturb >>= 5) {
      if (new_list[i & new_mask]) continue; /* Already in use. */
      new_list[i & new_mask] = entry; /* Use this enry! */
      break;
     }
     /* Continue re-hashing. */
    }
   }
   /* Set the new list as active. */
   self->pn_list = new_list;
   self->pn_mask = new_mask;
   /* With DUMMY entries gone, the effective
    * size now matches the used size. */
   self->pn_size = self->pn_used;
   atomic_rwlock_endwrite(&self->pn_lock);
   /* Free the old list and loop back to try again. */
   kfree(old_list);
   goto again;
  }
  if unlikely(self->pn_size == self->pn_mask) {
   atomic_rwlock_endwrite(&self->pn_lock);
   error_rethrow();
  }
 }
 /* The initial number of entropy bits used for PIDs. */
 random_bits = 3;

 /* If the caller provided a non-ZERO */
 if (pid_value != 0) goto add_pid;
 for (;;) {
  struct thread_pid *entry,**pentry;
  struct thread_pid **first_dummy;
generate_pid:
  /* Generate a random PID and add it to the namespace. */
#if __SIZEOF_PID_T__ > __SIZEOF_INT__
  if (random_bits > sizeof(int)*8) {
   pid_value  = (pid_t)rand();
   pid_value |= ((pid_t)(rand() & ((1 << (random_bits % (sizeof(int)*8)))-1))) << sizeof(int)*8;
  } else {
   pid_value  = (pid_t)(rand() & ((1 << (random_bits % (sizeof(int)*8)))-1));
  }
#else
  pid_value = (pid_t)(rand() & ((1 << (random_bits % (sizeof(pid_t)*8)))-1));
#endif
  /* In case our RNG isn't good enough,
   * add some linear scaling into the mix. */
  pid_value += (random_bits / sizeof(pid_t)*8);
  pid_value &= PID_MASK;

  /* We can't be using ZERO as PID value. */
  if unlikely(pid_value == 0) goto next_pid;
add_pid:
  /* Check if this PID is already in use. If it isn't, use it for our purposes. */
  perturb = i = (size_t)pid_value & self->pn_mask;
  first_dummy = NULL;
  for (;; i = ((i << 2) + i + perturb + 1),perturb >>= 5) {
   pentry = &self->pn_list[i & self->pn_mask];
   entry = *pentry;
   if (!entry) { if (!first_dummy) first_dummy = pentry; break; }
   if (entry == PIDNS_DUMMY) { if (!first_dummy) first_dummy = pentry; continue; }
   if (entry->tp_pids[self->pn_indirection] == pid_value) {
next_pid:
    /* PID is already being used (increase entropy) */
    if (!(random_bits & 0x80)) {
     random_bits <<= 1;
     random_bits  |= 1;
    } else {
     ++random_bits;
    }
    goto generate_pid;
   }
   /* Continue scanning... */
  }
  /* Set the used slot. */
  *first_dummy = pid;
  /* Save the used PID. */
  pid->tp_pids[self->pn_indirection] = pid_value;
  break;
 }
 /* Update hash-vector usage counters. */
 ++self->pn_used; /* Non-NULL & non-DUMMY entries. */
 ++self->pn_size; /* Non-NULL entries. */
 atomic_rwlock_endwrite(&self->pn_lock);
}

PRIVATE ATTR_NOTHROW void KCALL
pidns_delpid(struct pidns *__restrict self,
             struct thread_pid *__restrict pid) {
 struct thread_pid *entry; size_t i,perturb;
 atomic_rwlock_write(&self->pn_lock);
 if likely(self->pn_list) {
  perturb = i = (size_t)pid->tp_pids[self->pn_indirection] & self->pn_mask;
  for (;; i = ((i << 2) + i + perturb + 1),perturb >>= 5) {
   entry = self->pn_list[i & self->pn_mask];
   if (!entry) break; /* Could happen... */
   if (entry != pid) continue;
   /* Found it! */
   self->pn_list[i & self->pn_mask] = PIDNS_DUMMY;
   assert(self->pn_used);
   assert(self->pn_size);
   --self->pn_used; /* Non-NULL & non-DUMMY entries. */
   atomic_rwlock_endwrite(&self->pn_lock);
   return;
  }
 }
 atomic_rwlock_endwrite(&self->pn_lock);
}



/* Allocate a new PID descriptor for the given thread. */
PUBLIC ATTR_RETNONNULL ATTR_MALLOC REF struct thread_pid *
KCALL pidns_newpid(struct pidns *__restrict EXCEPT_VAR self, pid_t fixed_pid) {
 REF struct thread_pid *EXCEPT_VAR result;
 struct pidns *EXCEPT_VAR ns;
 result = (REF struct thread_pid *)kmalloc(offsetof(struct thread_pid,tp_pids)+
                                          (self->pn_indirection+1)*sizeof(pid_t),
                                           GFP_SHARED|GFP_CALLOC);
 /* Initialize basic fields. */
 result->tp_refcnt = 1;
 atomic_rwlock_cinit(&result->tp_task_lock);
 result->tp_ns = self;
 pidns_incref(self);

 ns = self;
 TRY {
  /* Add the PID to all associated namespaces. */
  do pidns_addpid(ns,result,fixed_pid),
     fixed_pid = 0; /* All other namespaces use randomly generated PIDs. */
  while ((ns = ns->pn_parent) != NULL);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Remove the PID descriptor from all namespace it was already added to. */
  error_printf("PIDNS_ADDPID() FAILED\n");
  while (self != ns) {
   pidns_delpid(self,result);
   self = self->pn_parent;
  }
  thread_pid_decref(result);
  error_rethrow();
 }
 return result;
}

PUBLIC void KCALL
thread_pid_destroy(struct thread_pid *__restrict self) {
 struct pidns *iter;
 assert(self->tp_task == NULL);
 assertf(self->tp_siblings.le_pself == NULL,
         "self->tp_siblings.le_pself = %p",
         self->tp_siblings.le_pself);
 /* Remove the thread PID from all associated namespaces. */
 iter = self->tp_ns;
 do pidns_delpid(iter,self);
 while ((iter = iter->pn_parent) != NULL);
 pidns_decref(self->tp_ns);
 kfree(self);
}


/* Lookup the a PID descriptor matching the
 * given PID in the calling thread's PID namespace.
 * @throw: E_PROCESS_EXITED: No process exists that matches `pid' */
PUBLIC ATTR_RETNONNULL
REF struct thread_pid *KCALL pid_lookup(pid_t pid) {
 REF struct thread_pid *result; size_t i,perturb;
 struct pidns *ns = THIS_THREAD_PID ? THIS_THREAD_PID->tp_ns : &pidns_kernel;
again:
 atomic_rwlock_read(&ns->pn_lock);
 if likely(ns->pn_list) {
  perturb = i = (size_t)pid & ns->pn_mask;
  for (;; i = ((i << 2) + i + perturb + 1),perturb >>= 5) {
   result = ns->pn_list[i & ns->pn_mask];
   if (!result) break;
   if (result == PIDNS_DUMMY) continue;
   if (result->tp_pids[ns->pn_indirection] != pid) continue;
   /* Found it! */
   thread_pid_incref(result);
   atomic_rwlock_endread(&ns->pn_lock);
   return result;
  }
 }
 atomic_rwlock_endread(&ns->pn_lock);
 /* Doesn't exist... */
 error_throw_resumable(E_PROCESS_EXITED);
 goto again;
}

PUBLIC ATTR_RETNONNULL
REF struct task *KCALL pid_lookup_task(pid_t pid) {
 REF struct task *result;
 REF struct thread_pid *tpid;
 tpid = pid_lookup(pid);
 result = thread_pid_get(tpid);
 thread_pid_decref(tpid);
 if (!result)
      error_throw(E_PROCESS_EXITED);
 return result;
}






DEFINE_PERTASK_FINI(task_fini_pid);
INTERN void KCALL task_fini_pid(struct task *__restrict thread) {
 REF struct thread_pid *pid;
 /* Cleanup the thread PID. */
 pid = FORTASK(thread,_this_pid);
 if (pid) {
  atomic_rwlock_write(&pid->tp_task_lock);
  assert(pid->tp_task == thread);
  pid->tp_task = NULL;
  atomic_rwlock_endwrite(&pid->tp_task_lock);
  /* Must also remove the PID from associated namespaces. */
  thread_pid_decref(pid);
 }
}

DEFINE_PERTASK_CLONE(task_clone_pid);
INTERN void KCALL task_clone_pid(struct task *__restrict new_thread, u32 flags) {
 if (!(new_thread->t_flags & TASK_FKERNELJOB)) {
  REF struct thread_pid *COMPILER_IGNORE_UNINITIALIZED(pid);
  REF struct pidns *EXCEPT_VAR ns;
  if (flags & CLONE_NEWPID) {
   /* Use a PID sub-namespace for the new thread. */
   ns = pidns_alloc(THIS_PIDNS);
  } else {
   /* Use the same PID namespace. */
   ns = THIS_PIDNS;
   pidns_incref(ns);
  }
  TRY {
   /* Create a PID descriptor for non-kernel jobs. */
   pid = pidns_newpid(ns,flags & CLONE_NEWPID ? 1 : 0);
  } FINALLY {
   pidns_decref(ns);
  }
  FORTASK(new_thread,_this_pid) = pid; /* Inherit reference. */
  COMPILER_WRITE_BARRIER();
  /* Set the associated task for the first time. */
  pid->tp_task = new_thread;
  COMPILER_WRITE_BARRIER();

  if (!(flags & CLONE_THREAD)) {
   REF struct task *parent = NULL;
   struct task *parent_leader;
   parent_leader = get_this_process();
   if (flags & CLONE_PARENT) {
    /* Try to use the parent of the calling thread as parent of the new
     * one, rather use the current thread as parent. */
    parent = task_get_parent();
    if (parent)
        parent_leader = FORTASK(parent,_this_group).tg_leader;
   }
   /* Add the PID descriptor to the parent process's chain of children. */
#define PROCESS (FORTASK(parent_leader,_this_group).tg_process)
   sig_get(&PROCESS.h_cldevent);
   if unlikely(!(parent_leader->t_state &
                (TASK_STATE_FTERMINATING|TASK_STATE_FTERMINATED))) {
    /* Add the PID to the chain of children. */
    LIST_INSERT(PROCESS.h_children,pid,tp_siblings);
    thread_pid_incref(pid); /* The reference stored in the `tp_siblings' chain. */
   }
   sig_put(&PROCESS.h_cldevent);
#undef PROCESS
   if (parent)
       task_decref(parent);
  }
 }
}



PUBLIC ATTR_NOTHROW ATTR_CONST pid_t KCALL posix_gettid(void) {
 struct thread_pid *pid = THIS_THREAD_PID;
 return pid ? pid->tp_pids[pid->tp_ns->pn_indirection] : 0;
}
PUBLIC ATTR_NOTHROW pid_t KCALL posix_getpid(void) {
 struct thread_pid *my_pid = THIS_THREAD_PID;
 struct thread_pid *leader_pid;
 if (!my_pid) return 0; /* Caller has no PID */
 leader_pid = FORTASK(get_this_process(),_this_pid);
 if (!leader_pid) return 0; /* Process leader has no PID */
 if (leader_pid->tp_ns != my_pid->tp_ns) return 0; /* Process leader in different PID namespace. */
 return leader_pid->tp_pids[my_pid->tp_ns->pn_indirection];
}
PUBLIC pid_t KCALL posix_getppid(void) {
 REF struct task *parent; pid_t result;
 struct thread_pid *my_pid = THIS_THREAD_PID;
 struct thread_pid *parent_pid;
 if (!my_pid) return 0; /* Caller has no PID */
 parent = task_get_parent();
 if (!parent) return 1; /* Process has no parent (adopted by POSIX's PID-namespace init process) */
 parent_pid = FORTASK(FORTASK(parent,_this_group).tg_leader,_this_pid);
 if (parent_pid->tp_ns != my_pid->tp_ns)
      result = 0; /* Parent process is in different PID namespace. */
 else result = parent_pid->tp_pids[my_pid->tp_ns->pn_indirection];
 task_decref(parent);
 return result;
}
PUBLIC pid_t KCALL posix_getpgid(void) {
 struct thread_pid *my_pid = THIS_THREAD_PID;
 struct thread_pid *leader_pid;
 if (!my_pid) return 0; /* Caller has no PID */
 leader_pid = FORTASK(FORTASK(get_this_process(),_this_group).
                      tg_process.h_procgroup.pg_leader,_this_pid);
 if (!leader_pid) return 0; /* Process group leader has no PID */
 if (leader_pid->tp_ns != my_pid->tp_ns) return 0; /* Process group leader in different PID namespace. */
 return leader_pid->tp_pids[my_pid->tp_ns->pn_indirection];
}
PUBLIC pid_t KCALL posix_getsid(void) {
 struct thread_pid *my_pid = THIS_THREAD_PID;
 struct thread_pid *leader_pid;
 if (!my_pid) return 0; /* Caller has no PID */
 leader_pid = FORTASK(FORTASK(FORTASK(
                      get_this_process(),_this_group). /* Process */
                      tg_process.h_procgroup.pg_leader,_this_group). /* Process-group */
                      tg_process.h_procgroup.pg_master.m_session,_this_pid); /* session */
 if (!leader_pid) return 0; /* Session leader has no PID */
 if (leader_pid->tp_ns != my_pid->tp_ns) return 0; /* Session leader in different PID namespace. */
 return leader_pid->tp_pids[my_pid->tp_ns->pn_indirection];
}


/* Return the PID for the given `thread', as seen from the PID namespace of the calling thread.
 * If the given `thread' cannot be found in that namespace, return ZERO(0) instead.
 * Additionally, ZERO(0) is also returned if the given `thread' doesn't have a PID. */
PUBLIC ATTR_NOTHROW pid_t KCALL
posix_gettid_viewpid(struct thread_pid *__restrict thread) {
 struct pidns *thatns,*myns = THIS_PIDNS;
 thatns = thread->tp_ns;
 if (myns->pn_indirection > thatns->pn_indirection)
     return 0; /* `thread's PID namespace cannot be viewed because we're a sub-space of it. */
 if (!myns->pn_indirection)
     return thread->tp_pids[0]; /* Optimization: The root PID namespace can see everything. */
 /* Manually check if `thatns' is a sub-space of `myns' */
 while (thatns->pn_indirection > myns->pn_indirection)
     assert(thatns->pn_parent),
     assert(thatns->pn_parent->pn_indirection == thatns->pn_indirection-1),
     thatns = thatns->pn_parent;
 assert(thatns->pn_indirection == myns->pn_indirection);
 if (thatns != myns) return 0; /* No, it isn't a sub-space. */
 /* All right. The other namespace is a sub-space or ours */
 return thread->tp_pids[myns->pn_indirection];
}
PUBLIC ATTR_NOTHROW pid_t KCALL
posix_gettid_view(struct task *__restrict thread) {
 struct thread_pid *tpid;
 tpid = FORTASK(thread,_this_pid);
 if (!tpid) return 0;
 return posix_gettid_viewpid(tpid);
}
PUBLIC ATTR_NOTHROW pid_t KCALL
thread_pid_view(struct thread_pid *__restrict self,
                struct pidns *__restrict ns) {
 struct pidns *thatns;
 thatns = self->tp_ns;
 if (ns->pn_indirection > thatns->pn_indirection)
     return 0; /* The namespace of `self' cannot be viewed. */
 if (!ns->pn_indirection)
     return self->tp_pids[0]; /* Optimization: The root PID namespace can see everything. */
 /* Manually check if `thatns' is a sub-space of `ns' */
 while (thatns->pn_indirection > ns->pn_indirection)
     assert(thatns->pn_parent),
     assert(thatns->pn_parent->pn_indirection == thatns->pn_indirection-1),
     thatns = thatns->pn_parent;
 assert(thatns->pn_indirection == ns->pn_indirection);
 if (thatns != ns) return 0; /* No, it isn't a sub-space. */
 /* All right. The other namespace is a sub-space or `ns' */
 return self->tp_pids[ns->pn_indirection];
}



/* Define PID-query related system calls. */
DEFINE_SYSCALL0(gettid) {
 return posix_gettid();
}
DEFINE_SYSCALL0(getpid) {
 return posix_getpid();
}
DEFINE_SYSCALL0(getppid) {
 return posix_getppid();
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_PID_C */
