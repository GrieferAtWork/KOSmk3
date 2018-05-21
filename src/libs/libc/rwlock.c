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
 /* TODO */
}

/* Lookup, or create a new read-lock on `self'.
 * If a new lock is created, the returned lock's `rl_count' field is ZERO(0). */
CRT_KOS ATTR_RETNONNULL struct readlock *LIBCCALL
libc_rwlock_newread(rwlock_t *__restrict self) {
 /* TODO */
}

/* Same as `libc_rwlock_newread()', but return NULL instead
 * of throwing E_BADALLOC if allocation of a read lock fails. */
CRT_KOS ATTR_NOTHROW struct readlock *LIBCCALL
libc_rwlock_trynewread(rwlock_t *__restrict self) {
 /* TODO */
}

/* Delete a read-lock that is no longer in use. */
CRT_KOS ATTR_NOTHROW void LIBCCALL
libc_rwlock_delread(struct readlock *__restrict lock) {
#ifndef NDEBUG
 libc_memset(&lock->rl_pself,0xcc,sizeof(lock->rl_pself));
 libc_memset(&lock->rl_next,0xcc,sizeof(lock->rl_next));
#endif
 /* TODO */
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
   libc_rwlock_delread(rlock);
   /* Signal writers.
    * XXX: Only wake one writer?
    * XXX: What does a waiting reader here mean? Can that even happen?
    * NOTE: The only reason we wake everything here is because there
    *       might be waiting ?readers?, in which case one of them might
    *       prevent a writer from being woken.
    *       The kernel-version of R/W-lock does the following here:
    *    >> if (__RWLOCK_IND(control_word) == 1)
    *    >>     sig_send(&self->rw_unshare,1);
    */
   if (state & RWLOCK_MODE_FWAITERS)
       libc_Xfutex_wake(&self->rw_state,SIZE_MAX);

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
         libc_Xfutex_wake(&self->rw_state,SIZE_MAX);
    } else {
     /* No read-lock existed before, so we must switch to
      * ZERO-readers read-mode (a fully unlocked R/W-lock). */
     self->rw_locks = NULL;
     state = ATOMIC_XCH(self->rw_state,
                        RWLOCK_STATE(RWLOCK_MODE_FREADING,1));
     /* Both readers and writers might be able to do something now (wake all of them!)
      * XXX: Only wake one writer? */
     if (state & RWLOCK_MODE_FWAITERS)
         libc_Xfutex_wake(&self->rw_state,SIZE_MAX);
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
       libc_Xfutex_wake(&self->rw_state,SIZE_MAX);
  } else {
   /* No read-lock existed before, so we must switch to
    * ZERO-readers read-mode (a fully unlocked R/W-lock). */
   self->rw_locks = NULL;
   state = ATOMIC_XCH(self->rw_state,
                      RWLOCK_STATE(RWLOCK_MODE_FREADING,1));
   /* Both readers and writers might be able to do something now (wake all of them!) */
   if (state & RWLOCK_MODE_FWAITERS)
       libc_Xfutex_wake(&self->rw_state,SIZE_MAX);
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
      libc_Xfutex_wake(&self->rw_state,SIZE_MAX);
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
    if (!libc_Xfutex_wait64_nmask(&self->rw_state,
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

CRT_KOS bool LIBCCALL
libc_rwlock_timedwrite64(rwlock_t *__restrict self,
                         struct timespec64 const *abs_timeout) {
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


EXPORT(rwlock_read,libc_rwlock_read);
CRT_KOS void LIBCCALL
libc_rwlock_read(rwlock_t *__restrict self) {
 libc_rwlock_timedread64(self,NULL);
}
CRT_KOS void LIBCCALL
libc_rwlock_write(rwlock_t *__restrict self);


CRT_KOS void LIBCCALL libc_rwlock_upgrade(rwlock_t *__restrict self);
CRT_KOS void LIBCCALL libc_rwlock_write_aggressive(rwlock_t *__restrict self);
CRT_KOS void LIBCCALL libc_rwlock_upgrade_aggressive(rwlock_t *__restrict self);
CRT_KOS bool LIBCCALL libc_rwlock_timedread(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
CRT_KOS bool LIBCCALL libc_rwlock_timedwrite(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
CRT_KOS bool LIBCCALL libc_rwlock_timedupgrade(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
CRT_KOS bool LIBCCALL libc_rwlock_timedwrite_aggressive(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
CRT_KOS bool LIBCCALL libc_rwlock_timedupgrade_aggressive(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
CRT_KOS bool LIBCCALL libc_rwlock_timedupgrade64(rwlock_t *__restrict self, struct timespec64 const *abs_timeout);
CRT_KOS bool LIBCCALL libc_rwlock_timedwrite64_aggressive(rwlock_t *__restrict self, struct timespec64 const *abs_timeout);
CRT_KOS bool LIBCCALL libc_rwlock_timedupgrade64_aggressive(rwlock_t *__restrict self, struct timespec64 const *abs_timeout);
CRT_KOS WUNUSED bool LIBCCALL libc_rwlock_tryupgrade_s(rwlock_t *__restrict self);
CRT_KOS WUNUSED bool LIBCCALL libc_rwlock_end_s(rwlock_t *__restrict self);
CRT_KOS WUNUSED bool LIBCCALL libc_rwlock_endread_s(rwlock_t *__restrict self);
CRT_KOS void LIBCCALL libc_rwlock_endwrite_s(rwlock_t *__restrict self);
CRT_KOS void LIBCCALL libc_rwlock_downgrade_s(rwlock_t *__restrict self);
CRT_KOS void LIBCCALL libc_rwlock_upgrade_s(rwlock_t *__restrict self);
CRT_KOS void LIBCCALL libc_rwlock_upgrade_aggressive_s(rwlock_t *__restrict self);
CRT_KOS bool LIBCCALL libc_rwlock_timedupgrade_s(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
CRT_KOS bool LIBCCALL libc_rwlock_timedupgrade_aggressive_s(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
CRT_KOS bool LIBCCALL libc_rwlock_timedupgrade64_s(rwlock_t *__restrict self, struct timespec64 const *abs_timeout);
CRT_KOS bool LIBCCALL libc_rwlock_timedupgrade64_aggressive_s(rwlock_t *__restrict self, struct timespec64 const *abs_timeout);

DECL_END

#endif /* !GUARD_LIBS_LIBC_RWLOCK_C */
