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
#ifndef _KOS_SCHED_SEMAPHORE_H
#define _KOS_SCHED_SEMAPHORE_H 1

#include <hybrid/compiler.h>
#include <hybrid/__atomic.h>
#include <hybrid/typecore.h>
#include <hybrid/timespec.h>
#include <hybrid/limitcore.h>
#include <assert.h>
#include <features.h>
#include <kos/futex.h>
#include <errno.h>

__DECL_BEGIN

/* A simple, but fully featured user-space semaphore,
 * implemented using futex() functionality. */
typedef struct semaphore semaphore_t;
#define __SEMAPHORE_FWAITERS  0x80000000

struct semaphore {
    futex_t __m_tickets; /* The current number of available tickets. */
};

/* Initialize a new semaphore. */
#define SEMAPHORE_INIT(num_tickets)       { num_tickets }
#define semaphore_init(self,num_tickets)  ((self)->__m_tickets = (num_tickets))
#define semaphore_cinit(self,num_tickets) \
   ((__builtin_constant_p(num_tickets) && (num_tickets) == 0) ? \
    (assert((self)->__m_tickets) == 0) : (void)((self)->__m_tickets = (num_tickets)))
#define DEFINE_SEMAPHORE(name,num_tickets) semaphore_t name = SEMAPHORE_INIT(num_tickets)


/* Try to acquire a ticket from the semaphore, but return FALSE if doing so failed. */
__LOCAL __BOOL (__LIBCCALL semaphore_tryacquire)(struct semaphore *__restrict __self);

/* Release a total of `NUM_TICKETS' back to the
 * semaphore and wake the same number of waiting threads.
 * @return: * : The total number of threads woken. */
__LOCAL __size_t (__LIBCCALL semaphore_release)(struct semaphore *__restrict __self, futex_t __num_tickets);

/* Setup the given futex poll descriptor to wait for the semaphore to become ready. */
__LOCAL void (__LIBCCALL semaphore_poll)(struct semaphore *__restrict __self, struct pollfutex *__restrict __pftx);

/* Acquire a ticket from the given semaphore and wait if there is none right now.
 * @return: __EXCEPT_SELECT(false,-1 && errno == ETIMEDOUT):
 *          Only when `TIMEOUT != NULL': The specified timeout has expired.
 * @return: __EXCEPT_SELECT(true,0):
 *          The futex was triggered, and the caller should
 *          re-attempt acquiring the associated lock.
 * @throw:  E_SEGFAULT: The given `SELF' or `TIMEOUT' are faulty.
 * @throw:  E_BADALLOC: Failed to allocate the futex controller in kernel-space. 
 */
__FORCELOCAL int (__LIBCCALL semaphore_acquire)(struct semaphore *__restrict __self, struct timespec const *__abs_timeout);
#ifdef __USE_TIME64
__FORCELOCAL int (__LIBCCALL semaphore_acquire64)(struct semaphore *__restrict __self, struct __timespec64 const *__abs_timeout);
#endif
#ifdef __USE_EXCEPT
__FORCELOCAL __BOOL (__LIBCCALL Xsemaphore_acquire)(struct semaphore *__restrict __self, struct timespec const *__abs_timeout);
#ifdef __USE_TIME64
__FORCELOCAL __BOOL (__LIBCCALL Xsemaphore_acquire64)(struct semaphore *__restrict __self, struct __timespec64 const *__abs_timeout);
#endif
#endif /* __USE_EXCEPT */




#ifndef __INTELLISENSE__
__LOCAL __BOOL (__LIBCCALL semaphore_tryacquire)(struct semaphore *__restrict __self) {
 register futex_t __count;
 do if ((__count = ATOMIC_READ(__self->__m_tickets),
        (__count & ~__SEMAPHORE_FWAITERS) == 0)) return 0;
 while (!__hybrid_atomic_cmpxch_weak(__self->__m_tickets,__count,__count-1,
                                     __ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST));
 return 1;
}
__FORCELOCAL int (__LIBCCALL semaphore_acquire)(struct semaphore *__restrict __self,
                                                struct timespec const *__abs_timeout) {
 for (;;) {
  register int __result;
  register futex_t __count;
  __count = __self->__m_tickets;
  __COMPILER_READ_BARRIER();
  if ((__count & ~__SEMAPHORE_FWAITERS) != 0) {
   if (__hybrid_atomic_cmpxch_weak(__self->__m_tickets,__count,__count-1,
                                   __ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST))
       return 0;
   continue;
  }
  /* Wait while `FUTEX & ~__SEMAPHORE_FWAITERS' is ZERO, and set the __SEMAPHORE_FWAITERS flag. */
  if ((__result = futex_wait_mask(&__self->__m_tickets,
                                  ~__SEMAPHORE_FWAITERS,0,
                                   __SEMAPHORE_FWAITERS,
                                   __abs_timeout)) != 0 &&
       errno != EAGAIN) return __result;
 }
}
#ifdef __USE_TIME64
__FORCELOCAL int (__LIBCCALL semaphore_acquire64)(struct semaphore *__restrict __self,
                                                  struct __timespec64 const *__abs_timeout) {
 for (;;) {
  register int __result;
  register futex_t __count;
  __count = __self->__m_tickets;
  __COMPILER_READ_BARRIER();
  if ((__count & ~__SEMAPHORE_FWAITERS) != 0) {
   if (__hybrid_atomic_cmpxch_weak(__self->__m_tickets,__count,__count-1,
                                   __ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST))
       return 0;
   continue;
  }
  /* Wait while `FUTEX & ~__SEMAPHORE_FWAITERS' is ZERO, and set the __SEMAPHORE_FWAITERS flag. */
  if ((__result = futex_wait64_mask(&__self->__m_tickets,
                                    ~__SEMAPHORE_FWAITERS,0,
                                     __SEMAPHORE_FWAITERS,
                                     __abs_timeout)) != 0 &&
       errno != EAGAIN) return __result;
 }
}
#endif
#ifdef __USE_EXCEPT
__FORCELOCAL __BOOL (__LIBCCALL Xsemaphore_acquire)(struct semaphore *__restrict __self,
                                                    struct timespec const *__abs_timeout) {
 for (;;) {
  register futex_t __count;
  __count = __self->__m_tickets;
  __COMPILER_READ_BARRIER();
  if ((__count & ~__SEMAPHORE_FWAITERS) != 0) {
   if (__hybrid_atomic_cmpxch_weak(__self->__m_tickets,__count,__count-1,
                                   __ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST))
       return 1;
   continue;
  }
  /* Wait while `FUTEX & ~__SEMAPHORE_FWAITERS' is ZERO, and set the __SEMAPHORE_FWAITERS flag. */
  if (!Xfutex_wait_mask(&__self->__m_tickets,
                        ~__SEMAPHORE_FWAITERS,0,
                         __SEMAPHORE_FWAITERS,
                         __abs_timeout))
       return 0;
 }
}
#ifdef __USE_TIME64
__FORCELOCAL __BOOL (__LIBCCALL Xsemaphore_acquire64)(struct semaphore *__restrict __self,
                                                      struct __timespec64 const *__abs_timeout) {
 for (;;) {
  register int __result;
  register futex_t __count;
  __count = __self->__m_tickets;
  __COMPILER_READ_BARRIER();
  if ((__count & ~__SEMAPHORE_FWAITERS) != 0) {
   if (__hybrid_atomic_cmpxch_weak(__self->__m_tickets,__count,__count-1,
                                   __ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST))
       return 1;
   continue;
  }
  /* Wait while `FUTEX & ~__SEMAPHORE_FWAITERS' is ZERO, and set the __SEMAPHORE_FWAITERS flag. */
  if (!Xfutex_wait64_mask(&__self->__m_tickets,
                          ~__SEMAPHORE_FWAITERS,0,
                           __SEMAPHORE_FWAITERS,
                           __abs_timeout))
       return 0;
 }
}
#endif
#endif /* __USE_EXCEPT */

__LOCAL void (__LIBCCALL semaphore_poll)(struct semaphore *__restrict __self,
                                         struct pollfutex *__restrict __pftx) {
 __pftx->pf_futex  = &__self->__m_tickets;
 /* Set the WAITERS bit and wait if there are no tickets.
  * >> if ((FUTEX & ~__SEMAPHORE_FWAITERS) == 0) {
  * >>      FUTEX |= __SEMAPHORE_FWAITERS;
  * >>      WAIT();
  * >> }
  */
 __pftx->pf_action = FUTEX_WAIT_MASK;
 __pftx->pf_val    = (futex_t)~__SEMAPHORE_FWAITERS;
 __pftx->pf_val2   = (futex_t)0;
 __pftx->pf_val3   = (futex_t)__SEMAPHORE_FWAITERS;
}


__LOCAL __size_t (__LIBCCALL semaphore_release)(struct semaphore *__restrict __self,
                                                futex_t __num_tickets) {
 futex_t __old_word;
 __size_t __result;
 for (;;) {
  __old_word = __self->__m_tickets;
  __COMPILER_READ_BARRIER();
  if (__hybrid_atomic_cmpxch_weak(__self->__m_tickets,__old_word,
                                  __old_word+__num_tickets,
                                  __ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST))
      break;
 }
 /* Either there already are some tickets, or no one is waiting right now.
  * This check safes us _a_ _lot_ of otherwise unnecessary futex(WAKE) system calls. */
 if (__old_word != __SEMAPHORE_FWAITERS)
     return 0;
 /* It's our job to wake up to `NUM_TICKETS' threads.
  * If fewer threads were woken, then we know that all threads waiting for
  * tickets have been woken and we can try to unset the WAITERS bits. */
 __result = futex_wake(&__self->__m_tickets,__num_tickets);
 if (__result < __num_tickets &&
    (__hybrid_atomic_fetchand(__self->__m_tickets,
                             ~__SEMAPHORE_FWAITERS,
                              __ATOMIC_SEQ_CST) &
                              __SEMAPHORE_FWAITERS)) {
  /* Since we can't prevent new threads from appearing in the short
   * moment between us having woken all old threads, and us clearing
   * the WAITERS bit, we must wake _everyone_ again (even though most likely
   * no one is actually there), just so if there is someone, they can set
   * the WAITERS bit again after realizing that no tickets are available. */
  futex_wake(&__self->__m_tickets,__SIZE_MAX__);
 }
 return __result;
}
#endif

__DECL_END

#endif /* !_KOS_SCHED_SEMAPHORE_H */
