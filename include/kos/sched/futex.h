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
#ifndef _KOS_SCHED_FUTEX_H
#define _KOS_SCHED_FUTEX_H 1

#include <hybrid/compiler.h>
#include <hybrid/timespec.h>
#include <linux/futex.h>

DECL_BEGIN

__REDIRECT_EXCEPT_TM64(__LIBC,,int,__LIBCCALL,futex,
                      (__UINT32_TYPE__ *uaddr, int futex_op, __UINT32_TYPE__ val,
                       struct timespec const *timeout /* or: __UINT32_TYPE__ val2 */,
                       __UINT32_TYPE__ *uaddr2, __UINT32_TYPE__ val3),
                      (uaddr,futex_op,val,timeout,uaddr2,val3))
#ifdef __USE_TIME64
__REDIRECT_EXCEPT(__LIBC,,int,__LIBCCALL,futex64,
                 (__UINT32_TYPE__ *uaddr, int futex_op, __UINT32_TYPE__ val,
                  struct timespec64 const *timeout /* or: __UINT32_TYPE__ val2 */,
                  __UINT32_TYPE__ *uaddr2, __UINT32_TYPE__ val3),
                 (uaddr,futex_op,val,timeout,uaddr2,val3))
#endif
__REDIRECT_EXCEPT_(__LIBC,,int,__LIBCCALL,futexv,
                  (__UINT32_TYPE__ *uaddr, int futex_op, __UINT32_TYPE__ val,
                   __UINT32_TYPE__ val2, __UINT32_TYPE__ *uaddr2, __UINT32_TYPE__ val3),
                   futex64,(uaddr,futex_op,val,timeout,uaddr2,val3));
#ifdef __USE_EXCEPT
__REDIRECT_TM64(__LIBC,,int,__LIBCCALL,Xfutex,
               (__UINT32_TYPE__ *uaddr, int futex_op, __UINT32_TYPE__ val,
                struct timespec const *timeout /* or: __UINT32_TYPE__ val2 */,
                __UINT32_TYPE__ *uaddr2, __UINT32_TYPE__ val3),
               (uaddr,futex_op,val,timeout,uaddr2,val3))
#ifdef __USE_TIME64
__LIBC int (__LIBCCALL Xfutex64)(__UINT32_TYPE__ *uaddr, int futex_op, __UINT32_TYPE__ val,
                                 struct timespec64 const *timeout /* or: __UINT32_TYPE__ val2 */,
                                 __UINT32_TYPE__ *uaddr2, __UINT32_TYPE__ val3);
#endif
__REDIRECT(__LIBC,,int,__LIBCCALL,Xfutexv,
          (__UINT32_TYPE__ *uaddr, int futex_op, __UINT32_TYPE__ val,
           __UINT32_TYPE__ val2, __UINT32_TYPE__ *uaddr2, __UINT32_TYPE__ val3),
           Xfutex64,(uaddr,futex_op,val,timeout,uaddr2,val3));
#endif /* __USE_EXCEPT */


struct task_segment;
/* Basic thread-local context control functions. */
#ifdef __BUILDING_LIBC
__REDIRECT(__LIBC,__WUNUSED __ATTR_CONST,__pid_t,__LIBCCALL,__gettid,(void),libc_gettid,())
__REDIRECT(__LIBC,__WUNUSED __ATTR_RETNONNULL __ATTR_CONST,struct task_segment *,__LIBCCALL,__current,(void),libc_current,())
__REDIRECT(__LIBC,,int,__LIBCCALL,__sched_yield,(void),libc_sched_yield,())
#else
__REDIRECT(__LIBC,__WUNUSED __ATTR_CONST,__pid_t,__LIBCCALL,__gettid,(void),gettid,())
__LIBC __WUNUSED __ATTR_RETNONNULL __ATTR_CONST struct task_segment *(__LIBCCALL __current)(void);
__REDIRECT(__LIBC,,int,__LIBCCALL,__sched_yield,(void),sched_yield,())
#endif


/* Configuration option for standard synchronization primitives.
 * Before connecting to a signal, try to yield a couple of times
 * to try and get other threads to release some kind of lock,
 * as `sched_yield()' is a much faster operation than full scheduling.
 * Doing this may improve performance, especially on single-core machines. */
#ifndef CONFIG_YIELD_BEFORE_CONNECT
#ifndef CONFIG_NO_YIELD_BEFORE_CONNECT
#define CONFIG_YIELD_BEFORE_CONNECT  4
#endif
#elif defined(CONFIG_NO_YIELD_BEFORE_CONNECT)
#undef CONFIG_YIELD_BEFORE_CONNECT
#endif

#ifdef CONFIG_YIELD_BEFORE_CONNECT
#if (CONFIG_YIELD_BEFORE_CONNECT+0) == 0
#undef CONFIG_YIELD_BEFORE_CONNECT
#define THREAD_POLL_BEFORE_CONNECT(...) (void)0
#elif CONFIG_YIELD_BEFORE_CONNECT == 1
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ __sched_yield(); __VA_ARGS__; }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 2
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
   }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 3
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
   }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 4
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
   }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 5
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
   }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 6
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
       __sched_yield(); __VA_ARGS__; \
   }__WHILE0
#else
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ unsigned int __poll_count = CONFIG_YIELD_BEFORE_CONNECT; \
       do { \
           __sched_yield(); \
           __VA_ARGS__; \
       } while (--__poll_count); \
   }__WHILE0
#endif
#else
#define THREAD_POLL_BEFORE_CONNECT(...) (void)0
#endif




DECL_END


#endif /* !_KOS_SCHED_FUTEX_H */
