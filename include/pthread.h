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
#ifndef _PTHREAD_H
#define _PTHREAD_H 1

#include "__stdinc.h"
#include <features.h>
#include <endian.h>
#include <sched.h>
#include <time.h>

#include <hybrid/timespec.h>
#include <bits/types.h>
#include <bits/pthreadtypes.h>

#define __LIBP     __IMPDEF
#define __LIBPCALL __ATTR_CDECL

__SYSDECL_BEGIN

/*  === LIBPTHREAD AND SYS_exit / clone() / E_EXIT_THREAD ===
 * Although it is POSIX's intended behavior to only create threads using `pthread_create()',
 * and to only terminate them by calling `pthread_exit()', or by returning from a thread's
 * entry function, this is how KOS implements interaction with the SYS_exit system call,
 * as well as the clone() system call, and how pthread interacts with exceptions.
 * SYS_exit:
 *      `SYS_exit' should not be used. Semantically speaking it works for all
 *       cases, and operations like `pthread_join()' will function 100% correctly.
 *       However when used by a thread that has previously bee detached
 *       from its descriptor through use of `pthread_detach()', its descriptor
 *       will not be freed when the thread terminates, causing a memory leak.
 *       Just use `pthread_exit()' instead. It too won't unwind the stack
 *       and force the thread to exit immediately.
 * clone():
 *       A thread created by `clone()' can be used just like any other thread,
 *       but with the obvious exception that the creating thread has no (safe)
 *       way of accessing the clone()-ed thread's pthread descriptor.
 *       A clone()-ed thread can still call `pthread_self()' and have it lazily
 *       allocate its descriptor (If this allocation fails, a symbolic value is
 *       returned that is interpreted as a descriptor for the calling thread by
 *       other pthread functions)
 *       However if a clone()-ed thread ever calls any of pthread's apis, it
 *       must not exit using `SYS_exit' or by returning from its main functions,
 *       but must instead use `pthread_exit()'.
 *       If you're unsure if your clone()-ed thread ever made use of pthread
 *       functions, won't worry. Using `pthread_exit()' to terminate a thread
 *       is always allowed, even if that thread never used any other pthread
 *       function.
 * E_EXIT_THREAD:
 *       Can be used just like always, in that it will unwind the stack and
 *       execute exception handlers.
 *       The function started by `pthread_create()' is called from a catch-all
 *       guard that will cleanup thread descriptor data.
 *       Note however that a thread terminated by throwing an E_EXIT_THREAD
 *       exception will always terminate with `NULL' as its return value,
 *       meaning that `pthread_join()' will write `NULL' into `*THREAD_RETURN'
 */


/*  === LIBPTHREAD AND `_EXCEPT_API' / `_EXCEPT_SOURCE' ===
 * While KOS's pthread library does export exception-enabled equivalents of
 * all library functions for which exception support can be deemed useful,
 * due to the way that pthread functions prototypes are designed, it wouldn't
 * be smart to have the exception-enabled functions be equivalents of their
 * regular counterparts.
 * Take `pthread_create()' for example:
 * >> int pthread_create(pthread_t *newthread, pthread_attr_t const *attr, void *(*start_routine)(void *arg), void *arg);
 * The naive exception version would look like this:
 * >> void Xpthread_create(pthread_t *newthread, pthread_attr_t const *attr, void *(*start_routine)(void *arg), void *arg);
 * But I think that we could improve on this by having the function return the
 * thread handle directory, like other functions such as `pthread_self()' already do:
 * >> pthread_t Xpthread_create(pthread_attr_t const *attr, void *(*start_routine)(void *arg), void *arg);
 * And this. Is exactly how the exception-enabled version of
 * `pthread_create' looks like, as can be seen further down below.
 * However because of this and various other similar cases, the regular pthread
 * functions are not re-mapped when `_EXCEPT_API' is defined. Use of exceptions
 * must be done explicitly by using the X* counterparts. */


#ifdef __USE_EXCEPT
#define E_DEADLOCK   0x0190 /* [ERRNO(EDEADLK)] A deadlock situation happened. */
#endif


/* Detach state.  */
#define PTHREAD_CREATE_JOINABLE	0
#define PTHREAD_CREATE_DETACHED	1

/* Scheduler inheritance.  */
#define PTHREAD_INHERIT_SCHED   0
#define PTHREAD_EXPLICIT_SCHED  1

/* Scope handling.  */
#define PTHREAD_SCOPE_SYSTEM    0
#define PTHREAD_SCOPE_PROCESS   1


struct sched_param;

/* PThread attribute functions. */
__LIBP __NONNULL((1)) __errno_t (__LIBPCALL pthread_attr_init)(pthread_attr_t *__restrict __attr);
__LIBP __NONNULL((1)) __errno_t (__LIBPCALL pthread_attr_destroy)(pthread_attr_t *__restrict __attr);
__LIBP __NONNULL((1,2)) __errno_t (__LIBPCALL pthread_attr_getdetachstate)(pthread_attr_t const *__restrict __attr, int *__restrict __pdetachstate);
__LIBP __NONNULL((1)) __errno_t (__LIBPCALL pthread_attr_setdetachstate)(pthread_attr_t *__restrict __attr, int __detachstate);
__LIBP __NONNULL((1,2)) __errno_t (__LIBPCALL pthread_attr_getguardsize)(pthread_attr_t const *__restrict __attr, size_t *__restrict __pguardsize);
__LIBP __NONNULL((1)) __errno_t (__LIBPCALL pthread_attr_setguardsize)(pthread_attr_t const *__restrict __attr, size_t __guardsize);
__LIBP __NONNULL((1,2)) __errno_t (__LIBPCALL pthread_attr_getschedparam)(pthread_attr_t const *__restrict __attr, struct sched_param *__restrict __param);
__LIBP __NONNULL((1)) __errno_t (__LIBPCALL pthread_attr_setschedparam)(pthread_attr_t const *__restrict __attr, struct sched_param const *__restrict __param);
__LIBP __NONNULL((1,2)) __errno_t (__LIBPCALL pthread_attr_getschedpolicy)(pthread_attr_t const *__restrict __attr, int *__restrict __ppolicy);
__LIBP __NONNULL((1)) __errno_t (__LIBPCALL pthread_attr_setschedpolicy)(pthread_attr_t *__restrict __attr, int __policy);
__LIBP __NONNULL((1,2)) __errno_t (__LIBPCALL pthread_attr_getinheritsched)(pthread_attr_t const *__restrict __attr, int *__restrict __pinherit);
__LIBP __NONNULL((1)) __errno_t (__LIBPCALL pthread_attr_setinheritsched)(pthread_attr_t *__restrict __attr, int __inherit);
__LIBP __NONNULL((1,2)) __errno_t (__LIBPCALL pthread_attr_getscope)(pthread_attr_t const *__restrict __attr, int *__restrict __pscope);
__LIBP __NONNULL((1)) __errno_t (__LIBPCALL pthread_attr_setscope)(pthread_attr_t *__restrict __attr, int __scope);
__LIBP __NONNULL((1,2)) __errno_t (__LIBPCALL pthread_attr_getstackaddr)(pthread_attr_t const *__restrict __attr, void **__restrict __pstackaddr);
__LIBP __NONNULL((1)) __errno_t (__LIBPCALL pthread_attr_setstackaddr)(pthread_attr_t *__restrict __attr, void *__stackaddr);
__LIBP __NONNULL((1,2)) __errno_t (__LIBPCALL pthread_attr_getstacksize)(pthread_attr_t const *__restrict __attr, size_t *__restrict __pstacksize);
__LIBP __NONNULL((1)) __errno_t (__LIBPCALL pthread_attr_setstacksize)(pthread_attr_t *__restrict __attr, size_t __stacksize);
#ifdef __USE_XOPEN2K
__LIBP __NONNULL((1,2)) __errno_t (__LIBPCALL pthread_attr_getstack)(pthread_attr_t const *__restrict __attr, void **__pstackaddr, size_t *__pstacksize);
__LIBP __NONNULL((1)) __errno_t (__LIBPCALL pthread_attr_setstack)(pthread_attr_t *__restrict __attr, void *__stackaddr, size_t __stacksize);
#endif
#ifdef __USE_EXCEPT
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_attr_init)(pthread_attr_t *__restrict __attr);
__LIBP __NONNULL((1)) int (__LIBPCALL Xpthread_attr_getdetachstate)(pthread_attr_t const *__restrict __attr);
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_attr_setdetachstate)(pthread_attr_t *__restrict __attr, int __detachstate);
__LIBP __NONNULL((1)) size_t (__LIBPCALL Xpthread_attr_getguardsize)(pthread_attr_t const *__restrict __attr);
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_attr_setguardsize)(pthread_attr_t const *__restrict __attr, size_t __guardsize);
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_attr_getschedparam)(pthread_attr_t const *__restrict __attr, struct sched_param *__restrict __param);
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_attr_setschedparam)(pthread_attr_t const *__restrict __attr, struct sched_param const *__restrict __param);
__LIBP __NONNULL((1)) int (__LIBPCALL Xpthread_attr_getschedpolicy)(pthread_attr_t const *__restrict __attr);
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_attr_setschedpolicy)(pthread_attr_t *__restrict __attr, int __policy);
__LIBP __NONNULL((1)) int (__LIBPCALL Xpthread_attr_getinheritsched)(pthread_attr_t const *__restrict __attr);
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_attr_setinheritsched)(pthread_attr_t *__restrict __attr, int __inherit);
__LIBP __NONNULL((1)) int (__LIBPCALL Xpthread_attr_getscope)(pthread_attr_t const *__restrict __attr);
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_attr_setscope)(pthread_attr_t *__restrict __attr, int __scope);
__LIBP __NONNULL((1)) void *(__LIBPCALL Xpthread_attr_getstackaddr)(pthread_attr_t const *__restrict __attr);
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_attr_setstackaddr)(pthread_attr_t *__restrict __attr, void *__stackaddr);
__LIBP __NONNULL((1)) size_t (__LIBPCALL Xpthread_attr_getstacksize)(pthread_attr_t const *__restrict __attr);
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_attr_setstacksize)(pthread_attr_t *__restrict __attr, size_t __stacksize);
#ifdef __USE_XOPEN2K
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_attr_getstack)(pthread_attr_t const *__restrict __attr, void **__pstackaddr, size_t *__pstacksize);
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_attr_setstack)(pthread_attr_t *__restrict __attr, void *__stackaddr, size_t __stacksize);
#endif
#endif /* __USE_EXCEPT */
#ifdef __USE_GNU
__LIBP __NONNULL((1,3)) __errno_t (__LIBPCALL pthread_attr_setaffinity_np)(pthread_attr_t *__restrict __attr, size_t __cpusetsize, cpu_set_t const *__restrict __cpuset);
__LIBP __NONNULL((1,3)) __errno_t (__LIBPCALL pthread_attr_getaffinity_np)(pthread_attr_t const *__restrict __attr, size_t __cpusetsize, cpu_set_t *__restrict __cpuset);
__LIBP __NONNULL((1)) __errno_t (__LIBPCALL pthread_getattr_default_np)(pthread_attr_t *__restrict __attr);
__LIBP __NONNULL((1)) __errno_t (__LIBPCALL pthread_setattr_default_np)(pthread_attr_t const *__restrict __attr);
__LIBP __NONNULL((2)) __errno_t (__LIBPCALL pthread_getattr_np)(pthread_t __th, pthread_attr_t *__restrict __attr);
#ifdef __USE_EXCEPT
__LIBP __NONNULL((1,3)) void (__LIBPCALL Xpthread_attr_setaffinity_np)(pthread_attr_t *__restrict __attr, size_t __cpusetsize, cpu_set_t const *__restrict __cpuset);
__LIBP __NONNULL((1,3)) void (__LIBPCALL Xpthread_attr_getaffinity_np)(pthread_attr_t const *__restrict __attr, size_t __cpusetsize, cpu_set_t *__restrict __cpuset);
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_getattr_default_np)(pthread_attr_t *__restrict __attr);
__LIBP __NONNULL((1)) void (__LIBPCALL Xpthread_setattr_default_np)(pthread_attr_t const *__restrict __attr);
__LIBP __NONNULL((2)) void (__LIBPCALL Xpthread_getattr_np)(pthread_t __th, pthread_attr_t *__restrict __attr);
#endif /* __USE_EXCEPT */
#endif /* __USE_GNU */


/* PThread core functions. */
__LIBP __NONNULL((1,3)) __errno_t (__LIBPCALL pthread_create)(pthread_t *__restrict __newthread, pthread_attr_t const *__attr, void *(*__start_routine)(void *__arg), void *__arg);
__LIBP __ATTR_NORETURN void (__LIBPCALL pthread_exit)(void *__retval);
__LIBP __errno_t (__LIBPCALL pthread_join)(pthread_t __th, void **__thread_return);
__LIBP __errno_t (__LIBPCALL pthread_detach)(pthread_t __th);
__LIBP __WUNUSED __ATTR_CONST pthread_t __NOTHROW((__LIBPCALL pthread_self)(void));
__LIBP __WUNUSED __ATTR_CONST int __NOTHROW((__LIBPCALL pthread_equal)(pthread_t __thread1, pthread_t __thread2));
__LIBP __NONNULL((3)) __errno_t (__LIBPCALL pthread_setschedparam)(pthread_t __target_thread, int __policy, struct sched_param const *__param);
__LIBP __NONNULL((2,3)) __errno_t (__LIBPCALL pthread_getschedparam)(pthread_t __target_thread, int *__restrict __policy, struct sched_param *__restrict __param);
__LIBP __errno_t (__LIBPCALL pthread_setschedprio)(pthread_t __target_thread, int __prio);

#ifdef __USE_GNU
__LIBP __NONNULL((2)) __errno_t (__LIBPCALL pthread_getname_np)(pthread_t __target_thread, char *__buf, size_t __buflen);
__LIBP __NONNULL((2)) __errno_t (__LIBPCALL pthread_setname_np)(pthread_t __target_thread, const char *__name);
__LIBP __WUNUSED __errno_t (__LIBPCALL pthread_tryjoin_np)(pthread_t __th, void **__thread_return);
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBP,__WUNUSED,__errno_t,__LIBPCALL,pthread_timedjoin_np,(pthread_t __th, void **__thread_return, struct timespec const *__restrict __abstime),pthread_timedjoin64_np,(__th,__thread_return,__abstime))
#else /* __USE_TIME_BITS64 */
__LIBP __WUNUSED __errno_t (__LIBPCALL pthread_timedjoin_np)(pthread_t __th, void **__thread_return, struct timespec const *__restrict __abstime);
#endif /* !__USE_TIME_BITS64 */
#ifdef __USE_TIME64
__LIBP __WUNUSED __errno_t (__LIBPCALL pthread_timedjoin64_np)(pthread_t __th, void **__thread_return, struct __timespec64 const *__restrict __abstime);
#endif /* __USE_TIME64 */
#endif /* __USE_GNU */

#ifdef __USE_EXCEPT
__LIBP __NONNULL((2)) pthread_t (__LIBPCALL Xpthread_create)(pthread_attr_t const *__attr, void *(*__start_routine)(void *__arg), void *__arg);
__LIBP void *(__LIBPCALL Xpthread_join)(pthread_t __th);
__LIBP __NONNULL((3)) void (__LIBPCALL Xpthread_setschedparam)(pthread_t __target_thread, int __policy, struct sched_param const *__param);
__LIBP __NONNULL((2,3)) void (__LIBPCALL Xpthread_getschedparam)(pthread_t __target_thread, int *__restrict __policy, struct sched_param *__restrict __param);
__LIBP void (__LIBPCALL Xpthread_setschedprio)(pthread_t __target_thread, int __prio);
#ifdef __USE_GNU
/* @return: * : The required buffer size (including a trailing NUL-character). */
__LIBP __NONNULL((2)) size_t (__LIBPCALL Xpthread_getname_np)(pthread_t __target_thread, char *__buf, size_t __buflen);
__LIBP __NONNULL((2)) void (__LIBPCALL Xpthread_setname_np)(pthread_t __target_thread, const char *__name);
__LIBP __WUNUSED __BOOL (__LIBPCALL Xpthread_tryjoin_np)(pthread_t __th, void **__thread_return);
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBP,__WUNUSED,__BOOL,__LIBPCALL,Xpthread_timedjoin_np,(pthread_t __th, void **__thread_return, struct timespec const *__restrict __abstime),Xpthread_timedjoin64_np,(__th,__thread_return,__abstime))
#else /* __USE_TIME_BITS64 */
__LIBP __WUNUSED __BOOL (__LIBPCALL Xpthread_timedjoin_np)(pthread_t __th, void **__thread_return, struct timespec const *__restrict __abstime);
#endif /* !__USE_TIME_BITS64 */
#ifdef __USE_TIME64
__LIBP __WUNUSED __BOOL (__LIBPCALL Xpthread_timedjoin64_np)(pthread_t __th, void **__thread_return, struct __timespec64 const *__restrict __abstime);
#endif /* __USE_TIME64 */
#endif /* __USE_GNU */
#endif /* __USE_EXCEPT */

#ifdef __USE_UNIX98
__LIBP int (__LIBPCALL pthread_getconcurrency)(void);
__LIBP __errno_t (__LIBPCALL pthread_setconcurrency)(int __level);
#ifdef __USE_EXCEPT
__LIBP int (__LIBPCALL Xpthread_getconcurrency)(void);
__LIBP void (__LIBPCALL Xpthread_setconcurrency)(int __level);
#endif /* __USE_EXCEPT */
#endif

#ifdef __USE_GNU
__LIBP __errno_t (__LIBPCALL pthread_yield)(void);
__LIBP __NONNULL((3)) __errno_t (__LIBPCALL pthread_setaffinity_np)(pthread_t __th, size_t __cpusetsize, cpu_set_t const *__cpuset);
__LIBP __NONNULL((3)) __errno_t (__LIBPCALL pthread_getaffinity_np)(pthread_t __th, size_t __cpusetsize, cpu_set_t *__cpuset);
#ifdef __USE_EXCEPT
/* Doesn't actually throw anything right now.
 * However that might change in the future, considering that the
 * kernel-space task_yield() function throws E_WOULD_BLOCK when
 * preemption has been disabled. Considering that, user-space
 * may one day get something similar, meaning that this may start
 * throwing `E_WOULD_BLOCK' errors after the user enabled some sort
 * of realtime mode that doesn't exist, yet. */
__LIBP void (__LIBPCALL Xpthread_yield)(void);
__LIBP __NONNULL((3)) void (__LIBPCALL Xpthread_setaffinity_np)(pthread_t __th, size_t __cpusetsize, cpu_set_t const *__cpuset);
__LIBP __NONNULL((3)) void (__LIBPCALL Xpthread_getaffinity_np)(pthread_t __th, size_t __cpusetsize, cpu_set_t *__cpuset);
#endif /* __USE_EXCEPT */
#endif


__SYSDECL_END

#endif /* !_PTHREAD_H */
