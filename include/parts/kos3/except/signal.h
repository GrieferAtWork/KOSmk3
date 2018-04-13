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
#ifndef _PARTS_KOS3_EXCEPT_SIGNAL_H
#define _PARTS_KOS3_EXCEPT_SIGNAL_H 1

#include "__stdinc.h"
#include <features.h>
#ifndef _SIGNAL_H
#include <signal.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

__LIBC __PORT_KOSONLY int (__LIBCCALL Xraise)(int __sig);
__REDIRECT(__LIBC,__PORT_KOSONLY,__sighandler_t,__LIBCCALL,__Xsysv_signal,(int __sig, __sighandler_t __handler),Xsysv_signal,(__sig,__handler))
#ifdef __USE_GNU
__LIBC __PORT_KOSONLY __sighandler_t (__LIBCCALL Xsysv_signal)(int __sig, __sighandler_t __handler);
#endif /* __USE_GNU */
#ifdef __USE_MISC
__LIBC __PORT_KOSONLY __sighandler_t (__LIBCCALL Xsignal)(int __sig, __sighandler_t __handler);
__LIBC __PORT_KOSONLY __sighandler_t (__LIBCCALL Xssignal)(int __sig, __sighandler_t __handler);
#if 1
__LIBC __PORT_KOSONLY void (__LIBCCALL Xgsignal)(int __sig);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsigblock)(unsigned int __mask);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsigsetmask)(unsigned int __mask);
#else
__LIBC __PORT_KOSONLY __ATTR_DEPRECATED("Using `Xsigprocmask()' instead") void (__LIBCCALL Xgsignal)(int __sig);
__LIBC __PORT_KOSONLY __ATTR_DEPRECATED("Using `Xsigprocmask()' instead") void (__LIBCCALL Xsigblock)(unsigned int __mask);
__LIBC __PORT_KOSONLY __ATTR_DEPRECATED("Using `Xsigprocmask()' instead") void (__LIBCCALL Xsigsetmask)(unsigned int __mask);
#endif
__LIBC unsigned int (__LIBCCALL Xsiggetmask)(void);
#else /* __USE_MISC */
__REDIRECT(__LIBC,__PORT_KOSONLY,__sighandler_t,__LIBCCALL,Xsignal,(int __sig, __sighandler_t __handler),Xsysv_signal,(__sig,__handler))
#endif /* !__USE_MISC */
#ifdef __USE_XOPEN
__LIBC __PORT_KOSONLY __sighandler_t (__LIBCCALL Xbsd_signal)(int __sig, __sighandler_t __handler);
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __ATTR_NORETURN,__LIBCCALL,Xsigpause,(int __sig),X__xpg_sigpause,(__sig))
#endif /* __USE_XOPEN */
#ifdef __USE_POSIX
__LIBC __PORT_KOSONLY void (__LIBCCALL Xkill)(__pid_t __pid, int __sig);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsigprocmask)(int __how, sigset_t const *__restrict __set, sigset_t *__restrict __oset);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1)) void (__LIBCCALL Xsigsuspend)(sigset_t const *__set);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsigaction)(int __sig, struct sigaction const *__restrict __act, struct sigaction *__restrict __oact);
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xsigpending)(sigset_t *__set);
__LIBC __PORT_KOSONLY __NONNULL((1,2)) void (__LIBCCALL Xsigwait)(sigset_t const *__restrict __set, int *__restrict __sig);
#ifdef __USE_POSIX199309
__LIBC __PORT_KOSONLY __NONNULL((1)) unsigned int (__LIBCCALL Xsigwaitinfo)(sigset_t const *__restrict __set, siginfo_t *__restrict __info);
__REDIRECT_TM64(__LIBC,__PORT_KOSONLY __NONNULL((1)),unsigned int,__LIBCCALL,Xsigtimedwait,
               (sigset_t const *__restrict __set, siginfo_t *__restrict __info, struct timespec const *__timeout),
               (__set,__info,__timeout))
__LIBC void (__LIBCCALL Xsigqueue)(__pid_t __pid, int __sig, union sigval const __val);
#ifdef __USE_TIME64
__LIBC __PORT_KOSONLY __NONNULL((1)) unsigned int (__LIBCCALL Xsigtimedwait64)(sigset_t const *__restrict __set, siginfo_t *__restrict __info, struct __timespec64 const *__timeout);
#endif /* __USE_TIME64 */
#endif /* __USE_POSIX199309 */
#endif /* __USE_POSIX */
#ifdef __USE_KOS
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsigqueueinfo)(__pid_t __tgid, int __sig, siginfo_t const *__uinfo);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xtgsigqueueinfo)(__pid_t __tgid, __pid_t __tid, int __sig, siginfo_t const *__uinfo);
#endif /* __USE_KOS */
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__LIBC __PORT_KOSONLY void (__LIBCCALL Xkillpg)(__pid_t __pgrp, int __sig);
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsiginterrupt)(int __sig, int __interrupt);
__LIBC __PORT_KOSONLY __ATTR_DEPRECATED_ void (__LIBCCALL Xsigstack)(struct sigstack *__ss, struct sigstack *__oss);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsigaltstack)(struct sigaltstack const *__restrict __ss, struct sigaltstack *__restrict __oss);
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_XOPEN_EXTENDED
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsighold)(int __sig);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsigrelse)(int __sig);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsigignore)(int __sig);
__LIBC __PORT_KOSONLY __sighandler_t (__LIBCCALL Xsigset)(int __sig, __sighandler_t __disp);
#endif /* __USE_XOPEN_EXTENDED */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SIGNAL_H */
