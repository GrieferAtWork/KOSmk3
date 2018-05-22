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
#ifndef GUARD_LIBS_LIBC_RWLOCK_C
#define GUARD_LIBS_LIBC_RWLOCK_C 1

#include "libc.h"
#include "rwlock.h"
#include "malloc.h"
#include "futex.h"
#include "i386-kos/rpc.h"
#include <kos/heap.h>
#include <kos/types.h>
#include <kos/sched/rwlock.h>
#include <kos/thread.h>
#include <hybrid/atomic.h>
#include <hybrid/timespec.h>
#include <sys/poll.h>
#include <stdbool.h>

DECL_BEGIN

/* NOTE: The race condition of not having updated `rw_owner' after
 *       switching the R/W-lock to write-mode cannot lead to other
 *       threads thinking that they are the ones holding the write-lock:
 *        - When there were no read-locks before then, `rw_owner'
 *          will still contain a `NULL' pointer, and the comparison
 *         `GET_CURRENT() == NULL' is never true, as
 *         `struct task_segment::ts_self' is [1..1]
 *        - When there were read-locks before then (as is possible during
 *          a read->write upgrade operation), `rw_owner' will still be a
 *          heap-pointer to a `struct readlock' structure, meaning that
 *          this pointer is unique and can't possibly match the pointer
 *          of another object, namely `GET_CURRENT()'
 *       -> Therefor, attempting to acquire a write-lock before another
 *          thread has finished acquiring such a lock will never result
 *          in a false positive.
 */



/* Try to lookup a read lock held by the calling thread on `self'
 * If the calling thread isn't holding a read-lock, return `NULL'
 * NOTE: The returned lock never has an `rl_count' value of ZERO(0). */
CRT_KOS ATTR_NOTHROW struct readlock *LIBCCALL
libc_rwlock_getread(rwlock_t *__restrict self) {
 struct readlocks *me = GET_LOCKS();
 struct readlock *result;
 size_t perturb,i,hash;
 if (!me) goto nope;
 hash = RWLOCK_HASH(self);
 perturb = i = hash & me->rls_msk;
 for (;; i = ((i << 2) + i + perturb + 1),perturb >>= 5) {
  result = &me->rls_vec[i & me->rls_msk];
  if (result->rl_lock == self)
      return result->rl_count ? result : NULL; /* Found it! */
 }
nope:
 return NULL;
}

/* Lookup, or create a new read-lock on `self'.
 * If a new lock is created, the returned lock's `rl_count' field is ZERO(0). */
CRT_KOS ATTR_RETNONNULL struct readlock *LIBCCALL
libc_rwlock_newread(rwlock_t *__restrict self) {
 size_t i,hash,perturb;
 struct readlock *result;
 struct readlock *first_dummy;
 struct readlocks *me = GET_LOCKS();
 hash = RWLOCK_HASH(self);
 if (!me) {
  pid_t mytid = libc_gettid();
  /* Fairly simple case: The first time we don't have to rehash anything! */
  me = (struct readlocks *)libc_Xmalloc(SIZEOF_READLOCKS(READLOCKS_INITIAL_MASK));
  me->rls_use = 0;
  me->rls_cnt = 0;
  me->rls_msk = READLOCKS_INITIAL_MASK;
  /* Initialize the readlock vector. */
  for (i = 0; i <= READLOCKS_INITIAL_MASK; ++i) {
   me->rls_vec[i].rl_thread = mytid;
   me->rls_vec[i].rl_count  = 0;
   me->rls_vec[i].rl_lock   = NULL;
  }
  SET_LOCKS(me);
  result = &me->rls_vec[hash & READLOCKS_INITIAL_MASK];
  result->rl_lock = self;
  goto done;
 }
 /* Either we already have some read-locks, or we must rehash.
  * In either case, search for dummy locks and try to fill the
  * hash-map before actually commencing with a full rehash! */
again_find_free:
 perturb = i = hash & me->rls_msk;
 first_dummy = NULL;
 for (;; i = ((i << 2) + i + perturb + 1),perturb >>= 5) {
  result = &me->rls_vec[i & me->rls_msk];
  if (!result->rl_lock) break; /* End-of-chain */
  if (result->rl_lock == self)
      goto done; /* Already loaded */
  if (result->rl_lock == READLOCK_DUMMY) {
   if (!first_dummy)
        first_dummy = result;
  }
 }
 assert(me->rls_use <= me->rls_cnt);
 if (first_dummy) {
  /* Simple case: Can re-use the dummy of a lock no longer in use. */
  assert(first_dummy->rl_lock == READLOCK_DUMMY);
  result = first_dummy;
  ++me->rls_use;
 } else if (me->rls_cnt < me->rls_msk) {
  /* Another simple case: The hash-vector isn't
   * full yet, so we can extend the chain. */
  assert(result->rl_lock == NULL);
  ++me->rls_cnt;
  ++me->rls_use;
 } else {
  size_t new_mask,j;
  struct readlocks *new_me;
  struct readlocks *EXCEPT_VAR xnew_me;
  pid_t mytid = libc_gettid();
  /* The difficult case: The hash-vector has filled up and we must rehash it... */
  assert(me->rls_cnt == me->rls_use);
  assert(me->rls_use == me->rls_msk);
  assert(me->rls_msk != 0);
  new_mask = (me->rls_msk << 1) | 1;
  new_me = (struct readlocks *)libc_Xmalloc(SIZEOF_READLOCKS(new_mask));
  new_me->rls_use = me->rls_use;
  new_me->rls_cnt = me->rls_use;
  new_me->rls_msk = new_mask;
  /* Setup the new readlocks hash-vector as empty. */
  for (i = 0; i <= new_mask; ++i) {
   new_me->rls_vec[i].rl_thread = mytid;
   new_me->rls_vec[i].rl_count  = 0;
   new_me->rls_vec[i].rl_lock   = NULL;
  }

  /* Note that we must be careful not to block and have synchronous RPCs
   * be served before we finished rehashing the readlocks hash-vector.
   * For that reason, we must acquire _all_ the read-locks now, rather
   * than doing so later! */
  xnew_me = new_me;
rehash_lock_again:
  TRY {
   for (i = 0; i <= me->rls_msk; ++i) {
    struct rwlock *lock = me->rls_vec[i].rl_lock;
    assert(lock != READLOCK_DUMMY);
    if (!lock)
         continue; /* Required, because of the NULL-sentinel, which could be anywhere */
    if (!(ATOMIC_FETCHOR(lock->rw_state,RWLOCK_MODE_IRDLOCKBIT) & RWLOCK_MODE_IRDLOCKBIT))
         continue; /* Got the lock! */
    /* All right. We didn't get the lock, so we must release all that we
     * already had, yield to whoever is actually holding it, then try again. */
    while (i-- != 0) {
     lock = me->rls_vec[i].rl_lock;
     ATOMIC_FETCHAND(lock->rw_state,~RWLOCK_MODE_IRDLOCKBIT);
    }
    /* TODO: If this sched_yield() fails, we should sleep for a short while, because
     *       the lock is likely held by a thread running on a different CPU. */
    libc_sched_yield();
    /* NOTE: Also serve RPC functions here, because sched_yield() didn't already do that. */
    libc_rpc_serve();
    goto rehash_lock_again;
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   libc_free(xnew_me);
   error_rethrow();
  }
  COMPILER_BARRIER(); /* NOTE: START OF NON-BLOCKING CODE */

  /* All right! How that we're holding _all_ the locks, we can
   * relocate our read-locks into the new hash-vector. */
  for (i = 0; i <= me->rls_msk; ++i) {
   struct readlock *lock = &me->rls_vec[i];
   struct readlock *new_lock;
   assert(lock->rl_lock != READLOCK_DUMMY);
   if (!lock->rl_lock) continue; /* Required, because of the NULL-sentinel, which could be anywhere */
   /* Find a suitable new location for this lock. */
   perturb = j = RWLOCK_HASH(lock->rl_lock) & new_mask;
   for (;; j = ((j << 2) + j + perturb + 1),perturb >>= 5) {
    new_lock = &new_me->rls_vec[j &  new_mask];
    if (!new_lock->rl_lock) break; /* End-of-chain */
   }
   assert(*lock->rl_pself == lock);
   assert(lock->rl_count != 0);
   assert(lock->rl_thread == (uintptr_t)mytid);
   new_lock->rl_lock   = lock->rl_lock;
   new_lock->rl_count  = lock->rl_count;
   /* Override the link entry of the old lock with the new one. */
   new_lock->rl_pself  = lock->rl_pself;
   new_lock->rl_next   = lock->rl_next;
   *new_lock->rl_pself = new_lock;
   if (new_lock->rl_next)
       new_lock->rl_next->rl_pself = &new_lock->rl_next;
   /* With the lock now relocated, we can unlock its associated read-lock vector. */
   ATOMIC_FETCHAND(new_lock->rl_lock->rw_state,~RWLOCK_MODE_IRDLOCKBIT);
  }

  /* Setup the new locks and free the old.
   * NOTE: The order here is important, as we must not do anything that
   *       could block (such as `libc_free()') before having installed
   *       the new set of active locks. */
  SET_LOCKS(new_me);

  COMPILER_BARRIER(); /* NOTE: END OF NON-BLOCKING CODE */
  libc_free(me);
  me = new_me;
  goto again_find_free;
 }
 assert(me->rls_use <= me->rls_cnt);
 /* Setup this read-lock for use by our R/W-lock */
 assert(result->rl_count == 0);
 result->rl_lock = self;
done:
 return result;
}

/* Same as `libc_rwlock_newread()', but return NULL instead
 * of throwing E_BADALLOC if allocation of a read lock fails. */
CRT_KOS ATTR_NOTHROW struct readlock *LIBCCALL
libc_rwlock_trynewread(rwlock_t *__restrict self) {
 struct readlock *COMPILER_IGNORE_UNINITIALIZED(result);
 error_pushinfo();
 TRY {
  result = libc_rwlock_newread(self);
 } EXCEPT_HANDLED (libc_except_errno()) {
  result = NULL;
 }
 error_popinfo();
 return result;
}

/* Delete a read-lock that is no longer in use. */
CRT_KOS ATTR_NOTHROW void LIBCCALL
libc_rwlock_delread(struct readlock *__restrict lock) {
 assert(lock->rl_count == 0);
#ifndef NDEBUG
 libc_memset(&lock->rl_pself,0xcc,sizeof(lock->rl_pself));
 libc_memset(&lock->rl_next,0xcc,sizeof(lock->rl_next));
#endif
 lock->rl_lock  = READLOCK_DUMMY;
 /* Mark the lock as unused. */
 assert(GET_LOCKS()->rls_use >= 1);
 --GET_LOCKS()->rls_use;
}


EXPORT(rwlock_reading,libc_rwlock_reading);
CRT_KOS WUNUSED ATTR_NOTHROW bool LIBCCALL
libc_rwlock_reading(rwlock_t *__restrict self) {
 struct readlock *lock;
 futex_t status = ATOMIC_READ(self->rw_state);
 if (RWLOCK_IND(status) == 0)
     return false;
 if (RWLOCK_MODE(status) == RWLOCK_MODE_FWRITING)
     return self->rw_owner == GET_CURRENT();
 lock = libc_rwlock_getread(self);
 assert(!lock || lock->rl_count != 0);
 return lock != NULL;
}

EXPORT(rwlock_writing,libc_rwlock_writing);
CRT_KOS WUNUSED ATTR_NOTHROW bool LIBCCALL
libc_rwlock_writing(rwlock_t *__restrict self) {
 return (RWLOCK_MODE(ATOMIC_READ(self->rw_state)) == RWLOCK_MODE_FWRITING &&
         self->rw_owner == GET_CURRENT());
}

EXPORT(rwlock_poll,libc_rwlock_poll);
CRT_KOS ATTR_NOTHROW unsigned int LIBCCALL
libc_rwlock_poll(rwlock_t *__restrict self,
                 struct pollfutex *__restrict descr,
                 unsigned int how) {
 futex_t state;
 unsigned int result = 0;
 state = ATOMIC_READ(self->rw_state);
 switch (RWLOCK_MODE(state)) {

 case RWLOCK_MODE_FREADING:
  result = POLLIN;
  if (RWLOCK_IND(state) == 0 ||
     (RWLOCK_IND(state) == 1 && libc_rwlock_getread(self))) {
   /* There are no readers, or the calling thread is the only
    * reader, so acquiring a write-lock now wouldn't block. */
   result |= POLLOUT;
   pollfutex_init_nop(descr);
  } else if (how & POLLOUT) {
   /* The caller wants a write-lock, but there are none available.
    * Setup the poll to wait for readers to go away.*/
   futex_t need_state = RWLOCK_MODE_FREADING;
   if (libc_rwlock_getread(self))
       need_state = RWLOCK_INCIND(need_state);
   pollfutex_init_wait_nmask(descr,
                            &self->rw_state,
                             RWLOCK_MODE_FMASK|RWLOCK_MODE_INDMASK,
                             need_state,
                             RWLOCK_MODE_FWWAITERS);
  } else /*if (how & POLLIN) */{
   pollfutex_init_nop(descr);
  }
  break;

 case RWLOCK_MODE_FUPGRADING:
  if (libc_rwlock_getread(self)) {
   /* Technically, you can't acquire a write-lock at this point.
    * But rwlock_write() won't block either, as it'll throw an
    * E_RETRY_RWLOCK exception, meaning it won't block! */
   result = POLLIN|POLLOUT;
   pollfutex_init_nop(descr);
  }
  goto wait_for_endwrite;

 case RWLOCK_MODE_FWRITING:
  if (self->rw_owner == GET_CURRENT()) {
   /* Recursive write lock. */
   result = POLLIN|POLLOUT;
   pollfutex_init_nop(descr);
   break;
  }
wait_for_endwrite:
  assertf(!libc_rwlock_getread(self),
          "You can't be holding read locks when another thread is writing");
  result = 0;
  if (how & POLLOUT) {
   /* Configure to wait until there are no readers. */
   pollfutex_init_wait_nmask(descr,
                            &self->rw_state,
                             RWLOCK_MODE_FMASK|RWLOCK_MODE_INDMASK,
                             RWLOCK_STATE(RWLOCK_MODE_FREADING,0),
                             RWLOCK_MODE_FWWAITERS);
  } else {
   /* Configure to wait until there are only readers. */
   pollfutex_init_wait_nmask(descr,
                            &self->rw_state,
                             RWLOCK_MODE_FMASK,
                             RWLOCK_MODE_FREADING,
                             RWLOCK_MODE_FRWAITERS);
  }
  break;

 default: __builtin_unreachable();
 }
 return result & how;
}


EXPORT(rwlock_tryread,libc_rwlock_tryread);
CRT_KOS WUNUSED bool LIBCCALL
libc_rwlock_tryread(rwlock_t *__restrict self) {
 for (;;) {
  futex_t state,new_state;
  struct readlock *rlock;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  case RWLOCK_MODE_FREADING:
   if (state & RWLOCK_MODE_IRDLOCKBIT) {
    rlock = libc_rwlock_getread(self);
    if (rlock) {
     ++rlock->rl_count;
     return true; /* Recursive read lock */
    }
    /* Wait until regular read-mode is entered again. */
    libc_sched_yield();
    continue;
   }
   rlock = libc_rwlock_newread(self);
   if (rlock->rl_count != 0) {
    ++rlock->rl_count;
    return true; /* Recursive read lock */
   }
   /* First lock (Increase indirection and lock the read-locks chain) */
   new_state = RWLOCK_INCIND(state) | RWLOCK_MODE_IRDLOCKBIT;
   if (!ATOMIC_CMPXCH(self->rw_state,state,new_state))
        continue;
   /* Insert the new read-lock into the read-locks chains. */
   rlock->rl_count = 1;
   rlock->rl_pself = &self->rw_locks;
   if ((rlock->rl_next = self->rw_locks) != NULL)
        assert(rlock->rl_next->rl_lock == self),
        rlock->rl_next->rl_pself = &rlock->rl_next;
   self->rw_locks = rlock;
   ATOMIC_FETCHAND(self->rw_state,~RWLOCK_MODE_IRDLOCKBIT);
   return true;

  case RWLOCK_MODE_FUPGRADING:
   /* When in upgrade mode, existing readers are
    * still allowed to acquire recursive read locks. */
   rlock = libc_rwlock_getread(self);
   if (rlock) {
    assert(rlock->rl_count != 0);
    ++rlock->rl_count;
    return true;
   }
   ATTR_FALLTHROUGH
  case RWLOCK_MODE_FWRITING:
   /* When already in write-mode, additional read-locks
    * are actually write-locks, as they must still fulfill
    * the exclusive access restriction imposed by the
    * previous lock. */
   if (self->rw_owner != GET_CURRENT())
       return false;
   /* All right! we're the owner (increase the write indirection). */
   if (!ATOMIC_CMPXCH(self->rw_state,state,RWLOCK_INCIND(state)))
        continue;
   return true;

  default: __builtin_unreachable();
  }
 }
}

EXPORT(rwlock_tryread_nothrow,libc_rwlock_tryread_nothrow);
CRT_KOS WUNUSED bool LIBCCALL
libc_rwlock_tryread_nothrow(rwlock_t *__restrict self) {
 for (;;) {
  futex_t state,new_state;
  struct readlock *rlock;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  case RWLOCK_MODE_FREADING:
   if (state & RWLOCK_MODE_IRDLOCKBIT) {
    rlock = libc_rwlock_getread(self);
    if (rlock) {
     ++rlock->rl_count;
     return true; /* Recursive read lock */
    }
    /* Wait until regular read-mode is entered again. */
    libc_sched_yield();
    continue;
   }
   /* The only difference to the regular `rwlock_tryread()':
    * If we can't allocate a new read-lock, fail immediately. */
   rlock = libc_rwlock_trynewread(self);
   if (!rlock) return false;
   if (rlock->rl_count != 0) {
    ++rlock->rl_count;
    return true; /* Recursive read lock */
   }
   /* First lock (Increase indirection and lock the read-locks chain) */
   new_state = RWLOCK_INCIND(state) | RWLOCK_MODE_IRDLOCKBIT;
   if (!ATOMIC_CMPXCH(self->rw_state,state,new_state))
        continue;
   /* Insert the new read-lock into the read-locks chains. */
   rlock->rl_count = 1;
   rlock->rl_pself = &self->rw_locks;
   if ((rlock->rl_next = self->rw_locks) != NULL)
        assert(rlock->rl_next->rl_lock == self),
        rlock->rl_next->rl_pself = &rlock->rl_next;
   self->rw_locks = rlock;
   ATOMIC_FETCHAND(self->rw_state,~RWLOCK_MODE_IRDLOCKBIT);
   return true;

  case RWLOCK_MODE_FUPGRADING:
   /* When in upgrade mode, existing readers are
    * still allowed to acquire recursive read locks. */
   rlock = libc_rwlock_getread(self);
   if (rlock) {
    assert(rlock->rl_count != 0);
    ++rlock->rl_count;
    return true;
   }
   ATTR_FALLTHROUGH
  case RWLOCK_MODE_FWRITING:
   /* When already in write-mode, additional read-locks
    * are actually write-locks, as they must still fulfill
    * the exclusive access restriction imposed by the
    * previous lock. */
   if (self->rw_owner != GET_CURRENT())
       return false;
   /* All right! we're the owner (increase the write indirection). */
   RWLOCK_ATOMIC_INCIND(self);
   return true;

  default: __builtin_unreachable();
  }
 }
}


EXPORT(rwlock_trywrite,libc_rwlock_trywrite);
CRT_KOS WUNUSED ATTR_NOTHROW bool LIBCCALL
libc_rwlock_trywrite(rwlock_t *__restrict self) {
 for (;;) {
  futex_t state;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  case RWLOCK_MODE_FREADING:
   /* #1 Without any read locks, we can directly switch to write-mode.
    * #2 There is only a single reader, and that reader appears to be us,
    *    so create a write-lock ontop of that! */
   if (RWLOCK_IND(state) == 0 ||
      (RWLOCK_IND(state) == 1 && libc_rwlock_getread(self))) {
    assertf(RWLOCK_IND(state) != 0 || !libc_rwlock_getread(self),
            "No one should be holding any read locks right now.");
    if (!ATOMIC_CMPXCH(self->rw_state,state,
                      (state & RWLOCK_MODE_FWAITERS) |
                       RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
         continue;
    /* We got the write-lock! (save that it belongs to _US_) */
    self->rw_owner = GET_CURRENT();
    return true;
   }
   return false;

  case RWLOCK_MODE_FWRITING:
   /* The R/W-lock is in write-mode. -
    * Check if we're the ones holding that lock. */
   if (self->rw_owner == GET_CURRENT()) {
    /* We are the owner! */
    RWLOCK_ATOMIC_INCIND(self);
    return true;
   }
   assertf(!libc_rwlock_getread(self),
           "You can't be holding read locks when another thread is writing");
  case RWLOCK_MODE_FUPGRADING:
   return false;

  default: __builtin_unreachable();
  }
 }
}


EXPORT(rwlock_tryupgrade,libc_rwlock_tryupgrade);
CRT_KOS WUNUSED ATTR_NOTHROW bool LIBCCALL
libc_rwlock_tryupgrade(rwlock_t *__restrict self) {
 struct readlock *rlock;
 rlock = libc_rwlock_getread(self);
 if (!rlock) {
  assertf(RWLOCK_MODE(self->rw_state) == RWLOCK_MODE_FWRITING &&
          self->rw_owner == GET_CURRENT(),
          "You probably called rwlock_tryupgrade() without holding a read-lock. "
          "The only way that is allowed if you read-lock is actually a symbolic write-lock "
          "caused by a recursive read-after-write.");
  return true;
 }
 assert(rlock->rl_count >= 1);
 /* The simpler case: the read-lock stays, and
  * its thread-local indirection is lowered. */
 for (;;) {
  futex_t state;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  case RWLOCK_MODE_FWRITING:
   assertf(self->rw_owner == GET_CURRENT(),
           "Upgrading a read-lock when the R/W-lock is in write mode can only happen when the calling "
           "thread did `rwlock_read()', `rwlock_write()', `rwlock_read()' and finally `rwlock_upgrade()'");
   return true;

  case RWLOCK_MODE_FREADING:
   assertf(RWLOCK_IND(state) >= 1,
           "With us holding a read-lock, how can the read-indirection be ZERO(0)?");
   if (RWLOCK_IND(state) == 1) {
    if (!ATOMIC_CMPXCH(self->rw_state,state,
                      (state & RWLOCK_MODE_FWAITERS) |
                       RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
         continue;
    break;
   }
   return false;
  case RWLOCK_MODE_FUPGRADING:
   /* Some other thread is currently upgrading
    * their lock, so we can't do the same. */
   return false;

  default: __builtin_unreachable();
  }
 }
 /* We got the write-lock! (save that it belongs to _US_) */
 self->rw_owner = GET_CURRENT();
 /* Also: drop the recursive read-lock that got replaced by the write-lock. */
 if (!--rlock->rl_count)
      libc_rwlock_delread(rlock);
 return true;
}


EXPORT(rwlock_end,libc_rwlock_end);
CRT_KOS WUNUSED ATTR_NOTHROW bool LIBCCALL
libc_rwlock_end(rwlock_t *__restrict self) {
 struct readlock *rlock;
 for (;;) {
  futex_t state;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  case RWLOCK_MODE_FREADING:
  case RWLOCK_MODE_FUPGRADING:
   assertf(RWLOCK_IND(state) >= 1,"No one is holding any read-locks right now");
   rlock = libc_rwlock_getread(self);
   assertf(rlock,"You're not holding any locks on this R/W-lock");
   assert(rlock->rl_count >= 1);
   if (rlock->rl_count != 1) {
    --rlock->rl_count;
    return false; /* Recursive read-lock. */
   }
   /* This is the last lock, so we must update the R/W-lock! */
   if unlikely(state & RWLOCK_MODE_IRDLOCKBIT) {
    /* Another thread is currently updating the lock.
     * -> Let them finish, then try again. */
    libc_sched_yield();
    continue;
   }
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                      RWLOCK_MODE_IRDLOCKBIT |
                      RWLOCK_DECIND(state)))
        continue;
   /* Remove our read-lock. */
   if ((*rlock->rl_pself = rlock->rl_next) != NULL)
         rlock->rl_next->rl_pself = rlock->rl_pself;
   ATOMIC_FETCHAND(self->rw_state,~RWLOCK_MODE_IRDLOCKBIT);
   /* Delete the read-lock. */
   assert(rlock->rl_count == 1);
   rlock->rl_count = 0;
   libc_rwlock_delread(rlock);
   /* Wake a single writer. */
   if ((state & RWLOCK_MODE_FWWAITERS) && RWLOCK_IND(state) == 1)
       libc_Xfutex_wake_bitset(&self->rw_state,1,RWLOCK_MODE_FWWAITERS);

   /* Check for a RETRY_RWLOCK exception, and see if it's meant for us.
    * If it is, make sure to consume it so we don't run into any race
    * conditions that may cause us to act on this a second time, when
    * the exception had already been dealt with! */
   if (error_code() == __E_RETRY_RWLOCK &&
       error_info()->e_error.__e_retry_rwlock.e_rwlock_ptr == self) {
    error_info()->e_error.__e_retry_rwlock.e_rwlock_ptr = NULL;
    return true;
   }
   break;

  case RWLOCK_MODE_FWRITING:
   assertf(self->rw_owner == GET_CURRENT(),
           "You're not the owner of this R/W-lock");
   assert(RWLOCK_IND(state) >= 1);
   if (RWLOCK_IND(state) == 1) {
    /* This is the last write-lock. */
    rlock = libc_rwlock_getread(self);
    if (rlock) {
     /* Must downgrade as the caller had read-lock(s)
      * before they acquired a write-lock. */
     assert(rlock->rl_count != 0);
     /* Set our pre-existing read-lock as the initial one for the R/W-lock. */
     rlock->rl_pself = &self->rw_locks;
     rlock->rl_next  = NULL;
     self->rw_locks  = rlock;
     /* Switch the R/W-lock to read-mode with a single reader (us). */
     while (!ATOMIC_CMPXCH_WEAK(self->rw_state,state,
                               (state & RWLOCK_MODE_FWWAITERS) |
                                RWLOCK_STATE(RWLOCK_MODE_FREADING,1)))
             state = ATOMIC_READ(self->rw_state);
     /* Check if there are waiting readers that can be woken now! */
     if (state & RWLOCK_MODE_FRWAITERS)
         libc_Xfutex_wake_bitset(&self->rw_state,SIZE_MAX,RWLOCK_MODE_FRWAITERS);
    } else {
     /* No read-lock existed before, so we must switch to
      * ZERO-readers read-mode (a fully unlocked R/W-lock). */
     self->rw_locks = NULL;
     while (!ATOMIC_CMPXCH_WEAK(self->rw_state,state,
                               (state & RWLOCK_MODE_FRWAITERS) |
                                RWLOCK_STATE(RWLOCK_MODE_FREADING,0)))
             state = ATOMIC_READ(self->rw_state);
     /* Wake waiting threads. */
     if ((!(state & RWLOCK_MODE_FWWAITERS) ||
           !libc_Xfutex_wake_bitset(&self->rw_state,1,RWLOCK_MODE_FWWAITERS))) {
      /* No writers were woken. -> Try to wake readers. */
      if ((state & RWLOCK_MODE_FRWAITERS) &&
          (ATOMIC_FETCHAND(self->rw_state,~RWLOCK_MODE_FRWAITERS) & RWLOCK_MODE_FRWAITERS))
           libc_Xfutex_wake_bitset(&self->rw_state,SIZE_MAX,RWLOCK_MODE_FRWAITERS);
     }
    }
   } else {
    /* There are still more write-locks remaining,
     * so only decrement the recursion counter. */
    RWLOCK_ATOMIC_DECIND(self);
   }
   break;

  default: __builtin_unreachable();
  }

 }
 return false;
}

EXPORT(rwlock_endread,libc_rwlock_endread);
DEFINE_INTERN_ALIAS(libc_rwlock_endread,libc_rwlock_end);

EXPORT(rwlock_endwrite,libc_rwlock_endwrite);
CRT_KOS ATTR_NOTHROW void LIBCCALL
libc_rwlock_endwrite(rwlock_t *__restrict self) {
 futex_t state = ATOMIC_READ(self->rw_state);
 assertf(RWLOCK_MODE(state) == RWLOCK_MODE_FWRITING,
         "The R/W-lock isn't in write-mode");
 assertf(self->rw_owner == GET_CURRENT(),
         "You're not the owner of this R/W-lock");
 assert(RWLOCK_IND(state) >= 1);
 if (RWLOCK_IND(state) == 1) {
  struct readlock *rlock;
  /* This is the last write-lock. */
  rlock = libc_rwlock_getread(self);
  if (rlock) {
   /* Must downgrade as the caller had read-lock(s)
    * before they acquired a write-lock. */
   assert(rlock->rl_count != 0);
   /* Set our pre-existing read-lock as the initial one for the R/W-lock. */
   rlock->rl_pself = &self->rw_locks;
   rlock->rl_next  = NULL;
   self->rw_locks  = rlock;
   /* Switch the R/W-lock to read-mode with a single reader (us). */
   while (!ATOMIC_CMPXCH_WEAK(self->rw_state,state,
                             (state & RWLOCK_MODE_FWWAITERS) |
                              RWLOCK_STATE(RWLOCK_MODE_FREADING,1)))
           state = ATOMIC_READ(self->rw_state);
   /* Check if there are waiting readers that can be woken now! */
   if (state & RWLOCK_MODE_FRWAITERS)
       libc_Xfutex_wake_bitset(&self->rw_state,SIZE_MAX,RWLOCK_MODE_FRWAITERS);
  } else {
   /* No read-lock existed before, so we must switch to
    * ZERO-readers read-mode (a fully unlocked R/W-lock). */
   self->rw_locks = NULL;
   while (!ATOMIC_CMPXCH_WEAK(self->rw_state,state,
                             (state & RWLOCK_MODE_FRWAITERS) |
                              RWLOCK_STATE(RWLOCK_MODE_FREADING,0)))
           state = ATOMIC_READ(self->rw_state);
   /* Wake waiting threads. */
   if ((!(state & RWLOCK_MODE_FWWAITERS) ||
         !libc_Xfutex_wake_bitset(&self->rw_state,1,RWLOCK_MODE_FWWAITERS))) {
    /* No writers were woken. -> Try to wake readers. */
    if ((state & RWLOCK_MODE_FRWAITERS) &&
        (ATOMIC_FETCHAND(self->rw_state,~RWLOCK_MODE_FRWAITERS) & RWLOCK_MODE_FRWAITERS))
         libc_Xfutex_wake_bitset(&self->rw_state,SIZE_MAX,RWLOCK_MODE_FRWAITERS);
   }
  }
 } else {
  /* There are still more write-locks remaining,
   * so only decrement the recursion counter. */
  RWLOCK_ATOMIC_DECIND(self);
 }
}

EXPORT(rwlock_downgrade,libc_rwlock_downgrade);
CRT_KOS void LIBCCALL
libc_rwlock_downgrade(rwlock_t *__restrict self) {
 futex_t state = ATOMIC_READ(self->rw_state);
 assertf(RWLOCK_MODE(state) == RWLOCK_MODE_FWRITING,
         "You're not holding a write-lock");
 assertf(self->rw_owner == GET_CURRENT(),
         "You're not the owner of this write-lock");
 assert(RWLOCK_IND(state) >= 1);
 if (RWLOCK_IND(state) == 1) {
  /* Transform into a read-lock. */
  struct readlock *rlock;
  rlock = libc_rwlock_newread(self);
  ++rlock->rl_count; /* In case there were more read-locks before. */
  rlock->rl_next  = NULL;
  rlock->rl_pself = &self->rw_locks;
  self->rw_locks  = rlock;
  while (!ATOMIC_CMPXCH_WEAK(self->rw_state,state,
                            (state & RWLOCK_MODE_FWWAITERS) |
                             RWLOCK_STATE(RWLOCK_MODE_FREADING,1)))
          state = ATOMIC_READ(self->rw_state);
  /* Wake other readers. */
  if (state & RWLOCK_MODE_FRWAITERS)
      libc_Xfutex_wake_bitset(&self->rw_state,SIZE_MAX,RWLOCK_MODE_FRWAITERS);
 } else {
  RWLOCK_ATOMIC_DECIND(self);
 }
}


EXPORT(rwlock_timedread64,libc_rwlock_timedread64);
CRT_KOS bool LIBCCALL
libc_rwlock_timedread64(rwlock_t *__restrict self,
                        struct timespec64 const *abs_timeout) {
 for (;;) {
  futex_t state,new_state;
  struct readlock *rlock;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  case RWLOCK_MODE_FREADING:
   if (state & RWLOCK_MODE_IRDLOCKBIT) {
    rlock = libc_rwlock_getread(self);
    if (rlock) {
     ++rlock->rl_count;
     return true; /* Recursive read lock */
    }
    /* Wait until regular read-mode is entered again. */
    libc_sched_yield();
    continue;
   }
   rlock = libc_rwlock_newread(self);
   if (rlock->rl_count != 0) {
    ++rlock->rl_count;
    return true; /* Recursive read lock */
   }
   /* First lock (Increase indirection and lock the read-locks chain) */
   new_state = RWLOCK_INCIND(state) | RWLOCK_MODE_IRDLOCKBIT;
   if (!ATOMIC_CMPXCH(self->rw_state,state,new_state))
        continue;
   /* Insert the new read-lock into the read-locks chains. */
   rlock->rl_count = 1;
   rlock->rl_pself = &self->rw_locks;
   if ((rlock->rl_next = self->rw_locks) != NULL)
        assert(rlock->rl_next->rl_lock == self),
        rlock->rl_next->rl_pself = &rlock->rl_next;
   self->rw_locks = rlock;
   ATOMIC_FETCHAND(self->rw_state,~RWLOCK_MODE_IRDLOCKBIT);
   return true;

  case RWLOCK_MODE_FUPGRADING:
   /* When in upgrade mode, existing readers are
    * still allowed to acquire recursive read locks. */
   rlock = libc_rwlock_getread(self);
   if (rlock) {
    assert(rlock->rl_count != 0);
    ++rlock->rl_count;
    return true;
   }
   /* If we don't already have a read-lock, we're not allowed to create
    * a new one, since the R/W-lock is currently in upgrade-mode. */
   goto wait_for_read;
   
  case RWLOCK_MODE_FWRITING:
   /* When already in write-mode, additional read-locks
    * are actually write-locks, as they must still fulfill
    * the exclusive access restriction imposed by the
    * previous lock. */
   if (self->rw_owner != GET_CURRENT()) {
wait_for_read:
    if (!libc_Xfutex_wait64_nmask_bitset(&self->rw_state,
                                          RWLOCK_MODE_FMASK,
                                          RWLOCK_MODE_FREADING,
                                          RWLOCK_MODE_FRWAITERS,
                                          abs_timeout))
         return false;
    continue;
   }
   /* All right! we're the owner (increase the write indirection). */
   RWLOCK_ATOMIC_INCIND(self);
   return true;
  default: __builtin_unreachable();
  }
 }
}

EXPORT(rwlock_timedwrite64,libc_rwlock_timedwrite64);
CRT_KOS bool LIBCCALL
libc_rwlock_timedwrite64(rwlock_t *__restrict self,
                         struct timespec64 const *abs_timeout) {
 for (;;) {
  futex_t state;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  {
   rwlock_t *EXCEPT_VAR xself;
   struct readlock *rlock;
  case RWLOCK_MODE_FREADING:
   /* #1 Without any read locks, we can directly switch to write-mode.
    * #2 There is only a single reader, and that reader appears to be us,
    *    so create a write-lock ontop of that! */
   rlock = libc_rwlock_getread(self);
wait_unshare:
   if (RWLOCK_IND(state) == 0 ||
      (RWLOCK_IND(state) == 1 && rlock)) {
    assertf(RWLOCK_IND(state) != 0 || !rlock,
            "No one should be holding any read locks right now.");
    if (!ATOMIC_CMPXCH(self->rw_state,state,
                      (state & RWLOCK_MODE_FWAITERS) |
                       RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
         continue;
    /* We got the write-lock! (save that it belongs to _US_) */
    self->rw_owner = GET_CURRENT();
    return true;
   }
   assert(!(state & RWLOCK_MODE_IRDLOCKBIT));
   /* Switch to upgrade-mode. */
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                      RWLOCK_MODE_FUPGRADING |
                    ((state & ~RWLOCK_MODE_FMASK) - (rlock ? 1 : 0))))
        continue;
   xself = self;
   TRY {
    if (!libc_Xfutex_wait64_nmask_bitset(&self->rw_state,
                                          RWLOCK_MODE_INDMASK,0,
                                          RWLOCK_MODE_FWWAITERS,
                                          abs_timeout)) {
     futex_t state2;
     do state2 = ATOMIC_READ(self->rw_state),
        assert((state2 & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
     while (!ATOMIC_CMPXCH(self->rw_state,state2,RWLOCK_MODE_FREADING |
                         ((state2 & ~RWLOCK_MODE_FMASK) + (rlock ? 1 : 0))));
     return false;
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    futex_t state2;
    do state2 = ATOMIC_READ(xself->rw_state),
       assert((state2 & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
    while (!ATOMIC_CMPXCH(xself->rw_state,state2,RWLOCK_MODE_FREADING |
                        ((state2 & ~RWLOCK_MODE_FMASK) + (rlock ? 1 : 0))));
    error_rethrow();
   }
   /* We were woken. - Check if we can acquire the write-lock now. */
   state = ATOMIC_READ(self->rw_state);
   assert((state & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
   assert(RWLOCK_IND(state) >= (rlock ? 1 : 0));
   if (RWLOCK_IND(state) != (rlock ? 1 : 0))
       goto wait_unshare; /* Readers are still remaining. */
   /* Switch to write-mode. */
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                     (state & RWLOCK_MODE_FWAITERS) |
                      RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
        goto wait_unshare;
   assert(self->rw_locks == rlock);
   assert(!rlock || rlock->rl_pself == &self->rw_locks);
   assert(!rlock || rlock->rl_next  == NULL);
   self->rw_owner = GET_CURRENT();
   return true;
  }

  case RWLOCK_MODE_FWRITING:
   /* The R/W-lock is in write-mode. -
    * Check if we're the ones holding that lock. */
   if (self->rw_owner == GET_CURRENT()) {
    /* We are the owner! */
    RWLOCK_ATOMIC_INCIND(self);
    return true;
   }
   assertf(!libc_rwlock_getread(self),
           "You can't be holding read locks when another thread is writing");
   __IF0 {
  case RWLOCK_MODE_FUPGRADING:
    if (libc_rwlock_getread(self)) {
     /* We've got a read-lock and the R/W-lock is in upgrade-mode.
      * -> Unwind so we can get rid of our read-lock. */
     struct exception_info *info = error_info();
     libc_memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
     info->e_error.e_code                        = __E_RETRY_RWLOCK;
     info->e_error.e_flag                        = ERR_FNORMAL;
     info->e_error.__e_retry_rwlock.e_rwlock_ptr = self;
     error_throw_current();
    }
   }
   /* Wait until the lock drops down to read-mode. */
   if (!libc_Xfutex_wait64_nmask_bitset(&self->rw_state,
                                         RWLOCK_MODE_FMASK,
                                         RWLOCK_MODE_FREADING,
                                         RWLOCK_MODE_FWWAITERS,
                                         abs_timeout))
        return false;
   continue;

  default: __builtin_unreachable();
  }
 }
}

EXPORT(rwlock_timedupgrade64,libc_rwlock_timedupgrade64);
CRT_KOS bool LIBCCALL
libc_rwlock_timedupgrade64(rwlock_t *__restrict self,
                           struct timespec64 const *abs_timeout) {
 for (;;) {
  futex_t state;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  {
   rwlock_t *EXCEPT_VAR xself;
   struct readlock *rlock;
  case RWLOCK_MODE_FREADING:
   rlock = libc_rwlock_getread(self);
   assertf(rlock != NULL,"You're not holding any read-locks.");
   assert(RWLOCK_IND(state) >= 0);
wait_unshare:
   if (RWLOCK_IND(state) == 1) {
    if (!ATOMIC_CMPXCH(self->rw_state,state,
                      (state & RWLOCK_MODE_FWAITERS) |
                       RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
         continue;
    /* We got the write-lock! (save that it belongs to _US_) */
    self->rw_owner = GET_CURRENT();
    return true;
   }
   assert(!(state & RWLOCK_MODE_IRDLOCKBIT));
   /* Switch to upgrade-mode. */
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                      RWLOCK_MODE_FUPGRADING |
                    ((state & ~RWLOCK_MODE_FMASK) - 1)))
        continue;
   xself = self;
   TRY {
    if (!libc_Xfutex_wait64_nmask_bitset(&self->rw_state,
                                          RWLOCK_MODE_INDMASK,0,
                                          RWLOCK_MODE_FWWAITERS,
                                          abs_timeout)) {
     futex_t state2;
     do state2 = ATOMIC_READ(self->rw_state),
        assert((state2 & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
     while (!ATOMIC_CMPXCH(self->rw_state,state2,RWLOCK_MODE_FREADING |
                         ((state2 & ~RWLOCK_MODE_FMASK) + 1)));
     return false;
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    futex_t state2;
    do state2 = ATOMIC_READ(xself->rw_state),
       assert((state2 & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
    while (!ATOMIC_CMPXCH(xself->rw_state,state2,RWLOCK_MODE_FREADING |
                        ((state2 & ~RWLOCK_MODE_FMASK) + 1)));
    error_rethrow();
   }
   /* We were woken. - Check if we can acquire the write-lock now. */
   state = ATOMIC_READ(self->rw_state);
   assert((state & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
   assert(RWLOCK_IND(state) >= 1);
   if (RWLOCK_IND(state) != 1)
       goto wait_unshare; /* Readers are still remaining. */
   /* Switch to write-mode. */
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                     (state & RWLOCK_MODE_FWAITERS) |
                      RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
        goto wait_unshare;
   assert(self->rw_locks  == rlock);
   assert(rlock->rl_pself == &self->rw_locks);
   assert(rlock->rl_next  == NULL);
   self->rw_owner = GET_CURRENT();
   if (--rlock->rl_count == 0)
       libc_rwlock_delread(rlock);
   return true;
  }

  case RWLOCK_MODE_FWRITING:
   /* The R/W-lock is in write-mode. -
    * Check if we're the ones holding that lock. */
   assertf(self->rw_owner == GET_CURRENT(),
           "Assuming you called, rwlock_read(), you must have already been holding "
           "a write-lock before then, meaning you should have been the owner of the lock");
   /* We are the owner! */
   return true;

  {
   struct exception_info *info;
  case RWLOCK_MODE_FUPGRADING:
   assertf(libc_rwlock_getread(self),
           "You're not holding any read-locks");
   /* We've got a read-lock and the R/W-lock is in upgrade-mode.
    * -> Unwind so we can get rid of our read-lock. */
   info = error_info();
   libc_memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
   info->e_error.e_code                        = __E_RETRY_RWLOCK;
   info->e_error.e_flag                        = ERR_FNORMAL;
   info->e_error.__e_retry_rwlock.e_rwlock_ptr = self;
   error_throw_current();
  } break;

  default: __builtin_unreachable();
  }
 }
}


CRT_KOS unsigned int LIBCCALL
rwlock_killer(rwlock_t *__restrict self) {
 if (libc_rwlock_getread(self)) {
  /* Throw an exception to get our thread to release its lock. */
  struct exception_info *info = error_info();
  libc_memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
  info->e_error.e_code                        = __E_RETRY_RWLOCK;
  info->e_error.e_flag                        = ERR_FNORMAL;
  info->e_error.__e_retry_rwlock.e_rwlock_ptr = self;
  error_throw_current();
 }
 /* Always restart what we interrupted so this is as seamless as possible. */
 return RPC_RETURN_FORCE_RESTART;
}


CRT_KOS void LIBCCALL
libc_rwlock_kill_readers(rwlock_t *__restrict self) {
 pid_t reader_pids[32];
 pid_t *EXCEPT_VAR preader_pids = reader_pids;
 pid_t *EXCEPT_VAR vector = reader_pids;
 size_t i,count,alloc_count = COMPILER_LENOF(reader_pids);
 struct readlock *iter;
again:
 assert(RWLOCK_MODE(self->rw_state) == RWLOCK_MODE_FUPGRADING);
 TRY {
  while (ATOMIC_FETCHOR(self->rw_state,RWLOCK_MODE_IRDLOCKBIT))
     libc_sched_yield();
  assert(RWLOCK_MODE(self->rw_state) == RWLOCK_MODE_FUPGRADING);
  count = 0,iter = self->rw_locks;
  for (; iter; iter = iter->rl_next) {
   if (iter->rl_thread == (uintptr_t)GET_TID())
       continue; /* Skip the calling thread. */
   if (count < alloc_count)
       reader_pids[count] = iter->rl_thread;
   ++count;
  }
  ATOMIC_FETCHAND(self->rw_state,~RWLOCK_MODE_IRDLOCKBIT);
  if (count > alloc_count) {
   /* Must allocate more vector memory. */
   if (vector == reader_pids)
       vector = NULL;
   vector = (pid_t *)libc_Xrealloc(vector,count*sizeof(pid_t));
   alloc_count = libc_malloc_usable_size(vector) / sizeof(pid_t);
   assert(alloc_count >= count);
   goto again;
  }
  /* Schedule an RPC for every target thread. */
  for (i = 0; i < count; ++i) {
   libc_queue_rpc(vector[i],(rpc_t)&rwlock_killer,
                  self,RPC_FSYNCHRONOUS);
  }
 } FINALLY {
  if (vector != preader_pids)
      libc_free(vector);
 }
}



CRT_KOS bool LIBCCALL
libc_rwlock_timedwrite64_aggressive(rwlock_t *__restrict self,
                                    struct timespec64 const *abs_timeout) {
 for (;;) {
  futex_t state;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  {
   rwlock_t *EXCEPT_VAR xself;
   struct readlock *rlock;
  case RWLOCK_MODE_FREADING:
   /* #1 Without any read locks, we can directly switch to write-mode.
    * #2 There is only a single reader, and that reader appears to be us,
    *    so create a write-lock ontop of that! */
   rlock = libc_rwlock_getread(self);
wait_unshare:
   if (RWLOCK_IND(state) == 0 ||
      (RWLOCK_IND(state) == 1 && rlock)) {
    assertf(RWLOCK_IND(state) != 0 || !rlock,
            "No one should be holding any read locks right now.");
    if (!ATOMIC_CMPXCH(self->rw_state,state,
                      (state & RWLOCK_MODE_FWAITERS) |
                       RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
         continue;
    /* We got the write-lock! (save that it belongs to _US_) */
    self->rw_owner = GET_CURRENT();
    return true;
   }
   assert(!(state & RWLOCK_MODE_IRDLOCKBIT));
   /* Switch to upgrade-mode. */
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                      RWLOCK_MODE_FUPGRADING |
                    ((state & ~RWLOCK_MODE_FMASK) - (rlock ? 1 : 0))))
        continue;
   xself = self;
   TRY {
    libc_rwlock_kill_readers(self);
    if (!libc_Xfutex_wait64_nmask_bitset(&self->rw_state,
                                          RWLOCK_MODE_INDMASK,0,
                                          RWLOCK_MODE_FWWAITERS,
                                          abs_timeout)) {
     futex_t state2;
     do state2 = ATOMIC_READ(self->rw_state),
        assert((state2 & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
     while (!ATOMIC_CMPXCH(self->rw_state,state2,RWLOCK_MODE_FREADING |
                         ((state2 & ~RWLOCK_MODE_FMASK) + (rlock ? 1 : 0))));
     return false;
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    futex_t state2;
    do state2 = ATOMIC_READ(xself->rw_state),
       assert((state2 & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
    while (!ATOMIC_CMPXCH(xself->rw_state,state2,RWLOCK_MODE_FREADING |
                        ((state2 & ~RWLOCK_MODE_FMASK) + (rlock ? 1 : 0))));
    error_rethrow();
   }
   /* We were woken. - Check if we can acquire the write-lock now. */
   state = ATOMIC_READ(self->rw_state);
   assert((state & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
   assert(RWLOCK_IND(state) >= (rlock ? 1 : 0));
   if (RWLOCK_IND(state) != (rlock ? 1 : 0))
       goto wait_unshare; /* Readers are still remaining. */
   /* Switch to write-mode. */
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                     (state & RWLOCK_MODE_FWAITERS) |
                      RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
        goto wait_unshare;
   assert(self->rw_locks == rlock);
   assert(!rlock || rlock->rl_pself == &self->rw_locks);
   assert(!rlock || rlock->rl_next  == NULL);
   self->rw_owner = GET_CURRENT();
   return true;
  }

  case RWLOCK_MODE_FWRITING:
   /* The R/W-lock is in write-mode. -
    * Check if we're the ones holding that lock. */
   if (self->rw_owner == GET_CURRENT()) {
    /* We are the owner! */
    RWLOCK_ATOMIC_INCIND(self);
    return true;
   }
   assertf(!libc_rwlock_getread(self),
           "You can't be holding read locks when another thread is writing");
   __IF0 {
  case RWLOCK_MODE_FUPGRADING:
    if (libc_rwlock_getread(self)) {
     /* We've got a read-lock and the R/W-lock is in upgrade-mode.
      * -> Unwind so we can get rid of our read-lock. */
     struct exception_info *info = error_info();
     libc_memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
     info->e_error.e_code                        = __E_RETRY_RWLOCK;
     info->e_error.e_flag                        = ERR_FNORMAL;
     info->e_error.__e_retry_rwlock.e_rwlock_ptr = self;
     error_throw_current();
    }
   }
   /* Wait until the lock drops down to read-mode. */
   if (!libc_Xfutex_wait64_nmask_bitset(&self->rw_state,
                                         RWLOCK_MODE_FMASK,
                                         RWLOCK_MODE_FREADING,
                                         RWLOCK_MODE_FWWAITERS,
                                         abs_timeout))
        return false;
   continue;

  default: __builtin_unreachable();
  }
 }
}

CRT_KOS bool LIBCCALL
libc_rwlock_timedupgrade64_aggressive(rwlock_t *__restrict self,
                                      struct timespec64 const *abs_timeout) {
 for (;;) {
  futex_t state;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  {
   rwlock_t *EXCEPT_VAR xself;
   struct readlock *rlock;
  case RWLOCK_MODE_FREADING:
   rlock = libc_rwlock_getread(self);
   assertf(rlock != NULL,"You're not holding any read-locks.");
   assert(RWLOCK_IND(state) >= 0);
wait_unshare:
   if (RWLOCK_IND(state) == 1) {
    if (!ATOMIC_CMPXCH(self->rw_state,state,
                      (state & RWLOCK_MODE_FWAITERS) |
                       RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
         continue;
    /* We got the write-lock! (save that it belongs to _US_) */
    self->rw_owner = GET_CURRENT();
    return true;
   }
   assert(!(state & RWLOCK_MODE_IRDLOCKBIT));
   /* Switch to upgrade-mode. */
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                      RWLOCK_MODE_FUPGRADING |
                    ((state & ~RWLOCK_MODE_FMASK) - 1)))
        continue;
   xself = self;
   TRY {
    libc_rwlock_kill_readers(self);
    if (!libc_Xfutex_wait64_nmask_bitset(&self->rw_state,
                                          RWLOCK_MODE_INDMASK,0,
                                          RWLOCK_MODE_FWWAITERS,
                                          abs_timeout)) {
     futex_t state2;
     do state2 = ATOMIC_READ(self->rw_state),
        assert((state2 & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
     while (!ATOMIC_CMPXCH(self->rw_state,state2,RWLOCK_MODE_FREADING |
                         ((state2 & ~RWLOCK_MODE_FMASK) + 1)));
     return false;
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    futex_t state2;
    do state2 = ATOMIC_READ(xself->rw_state),
       assert((state2 & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
    while (!ATOMIC_CMPXCH(xself->rw_state,state2,RWLOCK_MODE_FREADING |
                        ((state2 & ~RWLOCK_MODE_FMASK) + 1)));
    error_rethrow();
   }
   /* We were woken. - Check if we can acquire the write-lock now. */
   state = ATOMIC_READ(self->rw_state);
   assert((state & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
   assert(RWLOCK_IND(state) >= 1);
   if (RWLOCK_IND(state) != 1)
       goto wait_unshare; /* Readers are still remaining. */
   /* Switch to write-mode. */
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                     (state & RWLOCK_MODE_FWAITERS) |
                      RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
        goto wait_unshare;
   assert(self->rw_locks  == rlock);
   assert(rlock->rl_pself == &self->rw_locks);
   assert(rlock->rl_next  == NULL);
   self->rw_owner = GET_CURRENT();
   if (--rlock->rl_count == 0)
       libc_rwlock_delread(rlock);
   return true;
  }

  case RWLOCK_MODE_FWRITING:
   /* The R/W-lock is in write-mode. -
    * Check if we're the ones holding that lock. */
   assertf(self->rw_owner == GET_CURRENT(),
           "Assuming you called, rwlock_read(), you must have already been holding "
           "a write-lock before then, meaning you should have been the owner of the lock");
   /* We are the owner! */
   return true;

  {
   struct exception_info *info;
  case RWLOCK_MODE_FUPGRADING:
   assertf(libc_rwlock_getread(self),
           "You're not holding any read-locks");
   /* We've got a read-lock and the R/W-lock is in upgrade-mode.
    * -> Unwind so we can get rid of our read-lock. */
   info = error_info();
   libc_memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
   info->e_error.e_code                        = __E_RETRY_RWLOCK;
   info->e_error.e_flag                        = ERR_FNORMAL;
   info->e_error.__e_retry_rwlock.e_rwlock_ptr = self;
   error_throw_current();
  } break;

  default: __builtin_unreachable();
  }
 }
}



EXPORT(rwlock_read,libc_rwlock_read);
CRT_KOS void LIBCCALL
libc_rwlock_read(rwlock_t *__restrict self) {
 libc_rwlock_timedread64(self,NULL);
}

EXPORT(rwlock_write,libc_rwlock_write);
CRT_KOS void LIBCCALL
libc_rwlock_write(rwlock_t *__restrict self) {
 libc_rwlock_timedwrite64(self,NULL);
}

EXPORT(rwlock_upgrade,libc_rwlock_upgrade);
CRT_KOS void LIBCCALL
libc_rwlock_upgrade(rwlock_t *__restrict self) {
 libc_rwlock_timedupgrade64(self,NULL);
}

EXPORT(rwlock_write_aggressive,libc_rwlock_write_aggressive);
CRT_KOS void LIBCCALL
libc_rwlock_write_aggressive(rwlock_t *__restrict self) {
 libc_rwlock_timedwrite64_aggressive(self,NULL);
}

EXPORT(rwlock_upgrade_aggressive,libc_rwlock_upgrade_aggressive);
CRT_KOS void LIBCCALL
libc_rwlock_upgrade_aggressive(rwlock_t *__restrict self) {
 libc_rwlock_timedupgrade64_aggressive(self,NULL);
}

EXPORT(rwlock_timedread,libc_rwlock_timedread);
CRT_KOS bool LIBCCALL
libc_rwlock_timedread(rwlock_t *__restrict self,
                      struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_rwlock_read(self),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_rwlock_timedread64(self,&t64);
}
EXPORT(rwlock_timedwrite,libc_rwlock_timedwrite);
CRT_KOS bool LIBCCALL
libc_rwlock_timedwrite(rwlock_t *__restrict self,
                       struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_rwlock_write(self),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_rwlock_timedwrite64(self,&t64);
}
EXPORT(rwlock_timedupgrade,libc_rwlock_timedupgrade);
CRT_KOS bool LIBCCALL
libc_rwlock_timedupgrade(rwlock_t *__restrict self,
                         struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_rwlock_upgrade(self),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_rwlock_timedupgrade64(self,&t64);
}
EXPORT(rwlock_timedwrite_aggressive,libc_rwlock_timedwrite_aggressive);
CRT_KOS bool LIBCCALL
libc_rwlock_timedwrite_aggressive(rwlock_t *__restrict self,
                                  struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_rwlock_write_aggressive(self),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_rwlock_timedwrite64_aggressive(self,&t64);
}
EXPORT(rwlock_timedupgrade_aggressive,libc_rwlock_timedupgrade_aggressive);
CRT_KOS bool LIBCCALL
libc_rwlock_timedupgrade_aggressive(rwlock_t *__restrict self,
                                    struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_rwlock_upgrade_aggressive(self),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_rwlock_timedupgrade64_aggressive(self,&t64);
}


/* TODO: Save variants */
EXPORT(rwlock_tryupgrade_s,libc_rwlock_tryupgrade_s);
CRT_KOS WUNUSED ATTR_NOTHROW bool LIBCCALL
libc_rwlock_tryupgrade_s(rwlock_t *__restrict self) {
 struct readlock *rlock;
 rlock = libc_rwlock_getread(self);
 if (!rlock) {
  if (RWLOCK_MODE(self->rw_state) != RWLOCK_MODE_FWRITING ||
      self->rw_owner != GET_CURRENT())
      error_throw(E_ILLEGAL_OPERATION);
  return true;
 }
 assert(rlock->rl_count >= 1);
 /* The simpler case: the read-lock stays, and
  * its thread-local indirection is lowered. */
 for (;;) {
  futex_t state;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  case RWLOCK_MODE_FWRITING:
   if (self->rw_owner != GET_CURRENT())
       error_throw(E_ILLEGAL_OPERATION);
   return true;

  case RWLOCK_MODE_FREADING:
   assertf(RWLOCK_IND(state) >= 1,
           "With us holding a read-lock, how can the read-indirection be ZERO(0)?");
   if (RWLOCK_IND(state) == 1) {
    if (!ATOMIC_CMPXCH(self->rw_state,state,
                      (state & RWLOCK_MODE_FWAITERS) |
                       RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
         continue;
    break;
   }
   return false;
  case RWLOCK_MODE_FUPGRADING:
   /* Some other thread is currently upgrading
    * their lock, so we can't do the same. */
   return false;

  default: __builtin_unreachable();
  }
 }
 /* We got the write-lock! (save that it belongs to _US_) */
 self->rw_owner = GET_CURRENT();
 /* Also: drop the recursive read-lock that got replaced by the write-lock. */
 if (!--rlock->rl_count)
      libc_rwlock_delread(rlock);
 return true;
}

EXPORT(rwlock_end_s,libc_rwlock_end_s);
CRT_KOS WUNUSED ATTR_NOTHROW bool LIBCCALL
libc_rwlock_end_s(rwlock_t *__restrict self) {
 struct readlock *rlock;
 for (;;) {
  futex_t state;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  case RWLOCK_MODE_FREADING:
  case RWLOCK_MODE_FUPGRADING:
   rlock = libc_rwlock_getread(self);
   if (!rlock)
        error_throw(E_ILLEGAL_OPERATION);
   assert(rlock->rl_count >= 1);
   assertf(RWLOCK_IND(state) >= 1,
           "With us holding a read-lock, how can the read-indirection be ZERO(0)?");
   if (rlock->rl_count != 1) {
    --rlock->rl_count;
    return false; /* Recursive read-lock. */
   }
   /* This is the last lock, so we must update the R/W-lock! */
   if unlikely(state & RWLOCK_MODE_IRDLOCKBIT) {
    /* Another thread is currently updating the lock.
     * -> Let them finish, then try again. */
    libc_sched_yield();
    continue;
   }
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                      RWLOCK_MODE_IRDLOCKBIT |
                      RWLOCK_DECIND(state)))
        continue;
   /* Remove our read-lock. */
   if ((*rlock->rl_pself = rlock->rl_next) != NULL)
         rlock->rl_next->rl_pself = rlock->rl_pself;
   ATOMIC_FETCHAND(self->rw_state,~RWLOCK_MODE_IRDLOCKBIT);
   /* Delete the read-lock. */
   assert(rlock->rl_count == 1);
   rlock->rl_count = 0;
   libc_rwlock_delread(rlock);
   /* Wake a single writer. */
   if ((state & RWLOCK_MODE_FWWAITERS) && RWLOCK_IND(state) == 1)
       libc_Xfutex_wake_bitset(&self->rw_state,1,RWLOCK_MODE_FWWAITERS);

   /* Check for a RETRY_RWLOCK exception, and see if it's meant for us.
    * If it is, make sure to consume it so we don't run into any race
    * conditions that may cause us to act on this a second time, when
    * the exception had already been dealt with! */
   if (error_code() == __E_RETRY_RWLOCK &&
       error_info()->e_error.__e_retry_rwlock.e_rwlock_ptr == self) {
    error_info()->e_error.__e_retry_rwlock.e_rwlock_ptr = NULL;
    return true;
   }
   break;

  case RWLOCK_MODE_FWRITING:
   if (self->rw_owner != GET_CURRENT())
       error_throw(E_ILLEGAL_OPERATION);
   assert(RWLOCK_IND(state) >= 1);
   if (RWLOCK_IND(state) == 1) {
    /* This is the last write-lock. */
    rlock = libc_rwlock_getread(self);
    if (rlock) {
     /* Must downgrade as the caller had read-lock(s)
      * before they acquired a write-lock. */
     assert(rlock->rl_count != 0);
     /* Set our pre-existing read-lock as the initial one for the R/W-lock. */
     rlock->rl_pself = &self->rw_locks;
     rlock->rl_next  = NULL;
     self->rw_locks  = rlock;
     /* Switch the R/W-lock to read-mode with a single reader (us). */
     while (!ATOMIC_CMPXCH_WEAK(self->rw_state,state,
                               (state & RWLOCK_MODE_FWWAITERS) |
                                RWLOCK_STATE(RWLOCK_MODE_FREADING,1)))
             state = ATOMIC_READ(self->rw_state);
     /* Check if there are waiting readers that can be woken now! */
     if (state & RWLOCK_MODE_FRWAITERS)
         libc_Xfutex_wake_bitset(&self->rw_state,SIZE_MAX,RWLOCK_MODE_FRWAITERS);
    } else {
     /* No read-lock existed before, so we must switch to
      * ZERO-readers read-mode (a fully unlocked R/W-lock). */
     self->rw_locks = NULL;
     while (!ATOMIC_CMPXCH_WEAK(self->rw_state,state,
                               (state & RWLOCK_MODE_FRWAITERS) |
                                RWLOCK_STATE(RWLOCK_MODE_FREADING,0)))
             state = ATOMIC_READ(self->rw_state);
     /* Wake waiting threads. */
     if ((!(state & RWLOCK_MODE_FWWAITERS) ||
           !libc_Xfutex_wake_bitset(&self->rw_state,1,RWLOCK_MODE_FWWAITERS))) {
      /* No writers were woken. -> Try to wake readers. */
      if ((state & RWLOCK_MODE_FRWAITERS) &&
          (ATOMIC_FETCHAND(self->rw_state,~RWLOCK_MODE_FRWAITERS) & RWLOCK_MODE_FRWAITERS))
           libc_Xfutex_wake_bitset(&self->rw_state,SIZE_MAX,RWLOCK_MODE_FRWAITERS);
     }
    }
   } else {
    /* There are still more write-locks remaining,
     * so only decrement the recursion counter. */
    RWLOCK_ATOMIC_DECIND(self);
   }
   break;

  default: __builtin_unreachable();
  }

 }
 return false;
}

EXPORT(rwlock_endread_s,libc_rwlock_endread_s);
DEFINE_INTERN_ALIAS(libc_rwlock_endread_s,libc_rwlock_end_s);

EXPORT(rwlock_endwrite_s,libc_rwlock_endwrite_s);
CRT_KOS void LIBCCALL
libc_rwlock_endwrite_s(rwlock_t *__restrict self) {
 futex_t state = ATOMIC_READ(self->rw_state);
 if (RWLOCK_MODE(state) != RWLOCK_MODE_FWRITING ||
     self->rw_owner != GET_CURRENT())
     error_throw(E_ILLEGAL_OPERATION);
 assert(RWLOCK_IND(state) >= 1);
 if (RWLOCK_IND(state) == 1) {
  struct readlock *rlock;
  /* This is the last write-lock. */
  rlock = libc_rwlock_getread(self);
  if (rlock) {
   /* Must downgrade as the caller had read-lock(s)
    * before they acquired a write-lock. */
   assert(rlock->rl_count != 0);
   /* Set our pre-existing read-lock as the initial one for the R/W-lock. */
   rlock->rl_pself = &self->rw_locks;
   rlock->rl_next  = NULL;
   self->rw_locks  = rlock;
   /* Switch the R/W-lock to read-mode with a single reader (us). */
   while (!ATOMIC_CMPXCH_WEAK(self->rw_state,state,
                             (state & RWLOCK_MODE_FWWAITERS) |
                              RWLOCK_STATE(RWLOCK_MODE_FREADING,1)))
           state = ATOMIC_READ(self->rw_state);
   /* Check if there are waiting readers that can be woken now! */
   if (state & RWLOCK_MODE_FRWAITERS)
       libc_Xfutex_wake_bitset(&self->rw_state,SIZE_MAX,RWLOCK_MODE_FRWAITERS);
  } else {
   /* No read-lock existed before, so we must switch to
    * ZERO-readers read-mode (a fully unlocked R/W-lock). */
   self->rw_locks = NULL;
   while (!ATOMIC_CMPXCH_WEAK(self->rw_state,state,
                             (state & RWLOCK_MODE_FRWAITERS) |
                              RWLOCK_STATE(RWLOCK_MODE_FREADING,0)))
           state = ATOMIC_READ(self->rw_state);
   /* Wake waiting threads. */
   if ((!(state & RWLOCK_MODE_FWWAITERS) ||
         !libc_Xfutex_wake_bitset(&self->rw_state,1,RWLOCK_MODE_FWWAITERS))) {
    /* No writers were woken. -> Try to wake readers. */
    if ((state & RWLOCK_MODE_FRWAITERS) &&
        (ATOMIC_FETCHAND(self->rw_state,~RWLOCK_MODE_FRWAITERS) & RWLOCK_MODE_FRWAITERS))
         libc_Xfutex_wake_bitset(&self->rw_state,SIZE_MAX,RWLOCK_MODE_FRWAITERS);
   }
  }
 } else {
  /* There are still more write-locks remaining,
   * so only decrement the recursion counter. */
  RWLOCK_ATOMIC_DECIND(self);
 }
}

EXPORT(rwlock_downgrade_s,libc_rwlock_downgrade_s);
CRT_KOS void LIBCCALL
libc_rwlock_downgrade_s(rwlock_t *__restrict self) {
 futex_t state = ATOMIC_READ(self->rw_state);
 if (RWLOCK_MODE(state) != RWLOCK_MODE_FWRITING ||
     self->rw_owner != GET_CURRENT())
     error_throw(E_ILLEGAL_OPERATION);
 assert(RWLOCK_IND(state) >= 1);
 if (RWLOCK_IND(state) == 1) {
  /* Transform into a read-lock. */
  struct readlock *rlock;
  rlock = libc_rwlock_newread(self);
  ++rlock->rl_count; /* In case there were more read-locks before. */
  rlock->rl_next  = NULL;
  rlock->rl_pself = &self->rw_locks;
  self->rw_locks  = rlock;
  while (!ATOMIC_CMPXCH_WEAK(self->rw_state,state,
                            (state & RWLOCK_MODE_FWWAITERS) |
                             RWLOCK_STATE(RWLOCK_MODE_FREADING,1)))
          state = ATOMIC_READ(self->rw_state);
  /* Wake other readers. */
  if (state & RWLOCK_MODE_FRWAITERS)
      libc_Xfutex_wake_bitset(&self->rw_state,SIZE_MAX,RWLOCK_MODE_FRWAITERS);
 } else {
  RWLOCK_ATOMIC_DECIND(self);
 }
}

EXPORT(rwlock_timedupgrade64_s,libc_rwlock_timedupgrade64_s);
CRT_KOS bool LIBCCALL
libc_rwlock_timedupgrade64_s(rwlock_t *__restrict self,
                             struct timespec64 const *abs_timeout) {
 for (;;) {
  futex_t state;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  {
   rwlock_t *EXCEPT_VAR xself;
   struct readlock *rlock;
  case RWLOCK_MODE_FREADING:
   rlock = libc_rwlock_getread(self);
   if (!rlock)
        error_throw(E_ILLEGAL_OPERATION);
   assert(RWLOCK_IND(state) >= 0);
wait_unshare:
   if (RWLOCK_IND(state) == 1) {
    if (!ATOMIC_CMPXCH(self->rw_state,state,
                      (state & RWLOCK_MODE_FWAITERS) |
                       RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
         continue;
    /* We got the write-lock! (save that it belongs to _US_) */
    self->rw_owner = GET_CURRENT();
    return true;
   }
   assert(!(state & RWLOCK_MODE_IRDLOCKBIT));
   /* Switch to upgrade-mode. */
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                      RWLOCK_MODE_FUPGRADING |
                    ((state & ~RWLOCK_MODE_FMASK) - 1)))
        continue;
   xself = self;
   TRY {
    if (!libc_Xfutex_wait64_nmask_bitset(&self->rw_state,
                                          RWLOCK_MODE_INDMASK,0,
                                          RWLOCK_MODE_FWWAITERS,
                                          abs_timeout)) {
     futex_t state2;
     do state2 = ATOMIC_READ(self->rw_state),
        assert((state2 & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
     while (!ATOMIC_CMPXCH(self->rw_state,state2,RWLOCK_MODE_FREADING |
                         ((state2 & ~RWLOCK_MODE_FMASK) + 1)));
     return false;
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    futex_t state2;
    do state2 = ATOMIC_READ(xself->rw_state),
       assert((state2 & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
    while (!ATOMIC_CMPXCH(xself->rw_state,state2,RWLOCK_MODE_FREADING |
                        ((state2 & ~RWLOCK_MODE_FMASK) + 1)));
    error_rethrow();
   }
   /* We were woken. - Check if we can acquire the write-lock now. */
   state = ATOMIC_READ(self->rw_state);
   assert((state & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
   assert(RWLOCK_IND(state) >= 1);
   if (RWLOCK_IND(state) != 1)
       goto wait_unshare; /* Readers are still remaining. */
   /* Switch to write-mode. */
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                     (state & RWLOCK_MODE_FWAITERS) |
                      RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
        goto wait_unshare;
   assert(self->rw_locks  == rlock);
   assert(rlock->rl_pself == &self->rw_locks);
   assert(rlock->rl_next  == NULL);
   self->rw_owner = GET_CURRENT();
   if (--rlock->rl_count == 0)
       libc_rwlock_delread(rlock);
   return true;
  }

  case RWLOCK_MODE_FWRITING:
   /* The R/W-lock is in write-mode. -
    * Check if we're the ones holding that lock. */
   if (self->rw_owner != GET_CURRENT())
       error_throw(E_ILLEGAL_OPERATION);
   /* We are the owner! */
   return true;

  {
   struct exception_info *info;
  case RWLOCK_MODE_FUPGRADING:
   if (!libc_rwlock_getread(self))
       error_throw(E_ILLEGAL_OPERATION);
   /* We've got a read-lock and the R/W-lock is in upgrade-mode.
    * -> Unwind so we can get rid of our read-lock. */
   info = error_info();
   libc_memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
   info->e_error.e_code                        = __E_RETRY_RWLOCK;
   info->e_error.e_flag                        = ERR_FNORMAL;
   info->e_error.__e_retry_rwlock.e_rwlock_ptr = self;
   error_throw_current();
  } break;

  default: __builtin_unreachable();
  }
 }
}

EXPORT(rwlock_timedupgrade64_aggressive_s,libc_rwlock_timedupgrade64_aggressive_s);
CRT_KOS bool LIBCCALL
libc_rwlock_timedupgrade64_aggressive_s(rwlock_t *__restrict self,
                                        struct timespec64 const *abs_timeout) {
 for (;;) {
  futex_t state;
  state = ATOMIC_READ(self->rw_state);
  switch (RWLOCK_MODE(state)) {

  {
   rwlock_t *EXCEPT_VAR xself;
   struct readlock *rlock;
  case RWLOCK_MODE_FREADING:
   rlock = libc_rwlock_getread(self);
   if (!rlock)
        error_throw(E_ILLEGAL_OPERATION);
   assert(RWLOCK_IND(state) >= 0);
wait_unshare:
   if (RWLOCK_IND(state) == 1) {
    if (!ATOMIC_CMPXCH(self->rw_state,state,
                      (state & RWLOCK_MODE_FWAITERS) |
                       RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
         continue;
    /* We got the write-lock! (save that it belongs to _US_) */
    self->rw_owner = GET_CURRENT();
    return true;
   }
   assert(!(state & RWLOCK_MODE_IRDLOCKBIT));
   /* Switch to upgrade-mode. */
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                      RWLOCK_MODE_FUPGRADING |
                    ((state & ~RWLOCK_MODE_FMASK) - 1)))
        continue;
   xself = self;
   TRY {
    libc_rwlock_kill_readers(self);
    if (!libc_Xfutex_wait64_nmask_bitset(&self->rw_state,
                                          RWLOCK_MODE_INDMASK,0,
                                          RWLOCK_MODE_FWWAITERS,
                                          abs_timeout)) {
     futex_t state2;
     do state2 = ATOMIC_READ(self->rw_state),
        assert((state2 & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
     while (!ATOMIC_CMPXCH(self->rw_state,state2,RWLOCK_MODE_FREADING |
                         ((state2 & ~RWLOCK_MODE_FMASK) + 1)));
     return false;
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    futex_t state2;
    do state2 = ATOMIC_READ(xself->rw_state),
       assert((state2 & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
    while (!ATOMIC_CMPXCH(xself->rw_state,state2,RWLOCK_MODE_FREADING |
                        ((state2 & ~RWLOCK_MODE_FMASK) + 1)));
    error_rethrow();
   }
   /* We were woken. - Check if we can acquire the write-lock now. */
   state = ATOMIC_READ(self->rw_state);
   assert((state & RWLOCK_MODE_FMASK) == RWLOCK_MODE_FUPGRADING);
   assert(RWLOCK_IND(state) >= 1);
   if (RWLOCK_IND(state) != 1)
       goto wait_unshare; /* Readers are still remaining. */
   /* Switch to write-mode. */
   if (!ATOMIC_CMPXCH(self->rw_state,state,
                     (state & RWLOCK_MODE_FWAITERS) |
                      RWLOCK_STATE(RWLOCK_MODE_FWRITING,1)))
        goto wait_unshare;
   assert(self->rw_locks  == rlock);
   assert(rlock->rl_pself == &self->rw_locks);
   assert(rlock->rl_next  == NULL);
   self->rw_owner = GET_CURRENT();
   if (--rlock->rl_count == 0)
       libc_rwlock_delread(rlock);
   return true;
  }

  case RWLOCK_MODE_FWRITING:
   /* The R/W-lock is in write-mode. -
    * Check if we're the ones holding that lock. */
   if (self->rw_owner != GET_CURRENT())
       error_throw(E_ILLEGAL_OPERATION);
   /* We are the owner! */
   return true;

  {
   struct exception_info *info;
  case RWLOCK_MODE_FUPGRADING:
   if (!libc_rwlock_getread(self))
       error_throw(E_ILLEGAL_OPERATION);
   /* We've got a read-lock and the R/W-lock is in upgrade-mode.
    * -> Unwind so we can get rid of our read-lock. */
   info = error_info();
   libc_memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
   info->e_error.e_code                        = __E_RETRY_RWLOCK;
   info->e_error.e_flag                        = ERR_FNORMAL;
   info->e_error.__e_retry_rwlock.e_rwlock_ptr = self;
   error_throw_current();
  } break;

  default: __builtin_unreachable();
  }
 }
}

EXPORT(rwlock_upgrade_s,libc_rwlock_upgrade_s);
CRT_KOS void LIBCCALL
libc_rwlock_upgrade_s(rwlock_t *__restrict self) {
 libc_rwlock_timedupgrade64_s(self,NULL);
}
EXPORT(rwlock_upgrade_aggressive_s,libc_rwlock_upgrade_aggressive_s);
CRT_KOS void LIBCCALL
libc_rwlock_upgrade_aggressive_s(rwlock_t *__restrict self) {
 libc_rwlock_timedupgrade64_aggressive_s(self,NULL);
}
EXPORT(rwlock_timedupgrade_s,libc_rwlock_timedupgrade_s);
CRT_KOS bool LIBCCALL
libc_rwlock_timedupgrade_s(rwlock_t *__restrict self,
                           struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_rwlock_upgrade_s(self),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_rwlock_timedupgrade64_s(self,&t64);
}
EXPORT(rwlock_timedupgrade_aggressive_s,libc_rwlock_timedupgrade_aggressive_s);
CRT_KOS bool LIBCCALL
libc_rwlock_timedupgrade_aggressive_s(rwlock_t *__restrict self,
                                      struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_rwlock_upgrade_aggressive_s(self),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_rwlock_timedupgrade64_aggressive_s(self,&t64);
}

DECL_END

#endif /* !GUARD_LIBS_LIBC_RWLOCK_C */
