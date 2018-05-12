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
#ifndef _KOS_SCHED_MUTEX_H
#define _KOS_SCHED_MUTEX_H 1

#include <hybrid/compiler.h>
#include <hybrid/__atomic.h>
#include <hybrid/typecore.h>
#include <hybrid/timespec.h>
#include <assert.h>
#include <features.h>
#include <kos/futex.h>

DECL_BEGIN

typedef struct mutex mutex_t;

struct mutex {
    __UINT32_TYPE__ __m_futex; /* The futex word that this mutex uses for locking.
                                * Used in conjunction with `FUTEX_LOCK_PI' / `FUTEX_UNLOCK_PI'. */
    unsigned int    __m_rec;   /* [lock(__m_futex)] The owner of the mutex. */
};

#define MUTEX_INIT         {0,0}
#define DEFINE_MUTEX(name) mutex_t name = MUTEX_INIT
#define mutex_cinit(self)  (void)(assert((self)->__m_futex == 0))
#define mutex_init(self)   (void)((self)->__m_futex = 0)
#define mutex_holding(x)   (((x)->__m_futex & FUTEX_TID_MASK) == (__UINT32_TYPE__)__gettid())

/* Atomically try to acquire the given mutex. */
__FORCELOCAL __ATTR_NOTHROW __BOOL (__LIBCCALL mutex_try)(struct mutex *__restrict __self);

/* Acquire/release a mutex.
 * @return:  0: Successfully acquired the mutex.
 * @return: -1: The calling thread was interrupted. (errno = EINTR)
 * @return: -1: The kernel failed to allocate the futex state. (errno = ENOMEM)
 * @return: -1: The given timeout has expired. (errno = ETIMEDOUT) */
__LIBC void (__LIBCCALL mutex_put)(struct mutex *__restrict __self);
__REDIRECT_EXCEPT_TM64(__LIBC,,int,__LIBCCALL,mutex_get_timed,
                      (struct mutex *__restrict __self, struct timespec *__abs_timeout),
                      (__self,__abs_timeout));
#ifdef __USE_TIME64
__REDIRECT_EXCEPT(__LIBC,int,__LIBCCALL,mutex_get_timed64,
                 (struct mutex *__restrict __self, struct __timespec64 *__abs_timeout),
                 (__self,__abs_timeout))
#endif
#ifdef __USE_EXCEPT
__REDIRECT_TM64(__LIBC,,int,__LIBCCALL,Xmutex_get_timed,
               (struct mutex *__restrict __self, struct timespec *__abs_timeout),
               (__self,__abs_timeout));
#ifdef __USE_TIME64
__LIBC int (__LIBCCALL Xmutex_get_timed64)(struct mutex *__restrict __self, struct __timespec64 *__abs_timeout);
#endif
#endif /* __USE_EXCEPT */
#ifdef __INTELLISENSE__
#ifdef __USE_EXCEPT_API
__LIBC void (__LIBCCALL mutex_get)(struct mutex *__restrict __self);
#else
__LIBC int (__LIBCCALL mutex_get)(struct mutex *__restrict __self);
#endif
#ifdef __USE_EXCEPT
__LIBC void (__LIBCCALL Xmutex_get)(struct mutex *__restrict __self);
#endif /* __USE_EXCEPT */
#else
#ifdef __USE_EXCEPT_API
#ifdef __USE_TIME_BITS64
#define mutex_get(self) (void)mutex_get_timed(self,NULL)
#elif defined(__USE_TIME64)
#define mutex_get(self) (void)mutex_get_timed64(self,NULL)
#else
__REDIRECT(__LIBC,,int,__LIBCCALL,__mutex_get_timed64,
          (struct mutex *__restrict __self, struct __timespec64 *__abs_timeout),
           Xmutex_get_timed64,(__self,__abs_timeout));
#define mutex_get(self) (void)__mutex_get_timed64(self,NULL)
#endif
#else /* __USE_EXCEPT_API */
#ifdef __USE_TIME_BITS64
#define mutex_get(self) mutex_get_timed(self,NULL)
#elif defined(__USE_TIME64)
#define mutex_get(self) mutex_get_timed64(self,NULL)
#else
__REDIRECT(__LIBC,,int,__LIBCCALL,__mutex_get_timed64,
          (struct mutex *__restrict __self, struct __timespec64 *__abs_timeout),
           mutex_get_timed64,(__self,__abs_timeout));
#define mutex_get(self) __mutex_get_timed64(self,NULL)
#endif
#endif /* !__USE_EXCEPT_API */
#ifdef __USE_EXCEPT
#ifdef __USE_TIME_BITS64
#define Xmutex_get(self) (void)Xmutex_get_timed(self,NULL)
#elif defined(__USE_TIME64)
#define Xmutex_get(self) (void)Xmutex_get_timed64(self,NULL)
#else
__REDIRECT(__LIBC,,int,__LIBCCALL,__Xmutex_get_timed64,
          (struct mutex *__restrict __self, struct __timespec64 *__abs_timeout),
           Xmutex_get_timed64,(__self,__abs_timeout));
#define Xmutex_get(self) (void)__Xmutex_get_timed64(self,NULL)
#endif
#endif /* __USE_EXCEPT */
#endif


#ifndef __INTELLISENSE__
__FORCELOCAL __ATTR_NOTHROW
__BOOL (__LIBCCALL mutex_try)(struct mutex *__restrict __self) {
 __pid_t __caller = __gettid();
 __UINT32_TYPE__ __owner;
 __owner = __hybrid_atomic_cmpxch_val(__self->__m_futex,0,(__UINT32_TYPE__)__caller,
                                      __ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST);
 if (__owner == 0)
  __self->__m_rec = 1; /* First lock. */
 else if ((__owner & FUTEX_TID_MASK) == (__UINT32_TYPE__)__caller)
  ++__self->__m_rec; /* Recursive lock */
 else {
  /* Lock held by another thread. */
  return 0;
 }
 return 1;
}
#endif


DECL_END


#endif /* !_KOS_SCHED_MUTEX_H */
