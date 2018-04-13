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
#ifndef _I386_KOS_BITS_SIGNUM_H
#define _I386_KOS_BITS_SIGNUM_H 1
#define _BITS_SIGNUM_H 1

#include <__stdinc.h>
#include <features.h>

__SYSDECL_BEGIN

/* Signal number definitions.  Linux version.
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* Fake signal functions. */
#define SIG_ERR   ((__sighandler_t)-1) /* Error return. */
#define SIG_DFL   ((__sighandler_t)0)  /* Default action. */
#define SIG_IGN   ((__sighandler_t)1)  /* Ignore signal. */

#define __DOS_SIG_GET  ((__sighandler_t)2)  /* Return current value. */
#define __DOS_SIG_SGE  ((__sighandler_t)3)  /* Signal gets error. */
#define __DOS_SIG_ACK  ((__sighandler_t)4)  /* Acknowledge. */
#define __DOS_SIG_HOLD ((__sighandler_t)99) /* Add signal to hold mask. (Needs a different value due to collision with `__DOS_SIG_GET') */

#ifdef __USE_DOS
#define SIG_GET   __DOS_SIG_GET
#define SIG_SGE   __DOS_SIG_SGE
#define SIG_ACK   __DOS_SIG_ACK
#ifdef __USE_UNIX98
#define SIG_HOLD  __DOS_SIG_HOLD
#endif /* __USE_UNIX98 */
#else /* __USE_DOS */
#ifdef __USE_UNIX98
#define SIG_HOLD  ((__sighandler_t)2)  /* Add signal to hold mask. */
#endif /* __USE_UNIX98 */
#endif /* !__USE_DOS */

#ifdef __USE_KOS
#define SIG_CONT  ((__sighandler_t)8)  /* Continue execution. */
#define SIG_STOP  ((__sighandler_t)9)  /* Suspend execution. */
#define SIG_CORE  ((__sighandler_t)10) /* Create a coredump and terminate. */
/* Max special signal mask supported by the kernel */
#define __SIG_SPECIAL_MAX   SIG_CORE
#endif /* __USE_KOS */

#ifdef __KERNEL__
#define __KOS_SIGNO(x) (x-1)
#else
#define __KOS_SIGNO(x)  x
#endif

/* Signals. */
#define SIGHUP    __KOS_SIGNO(1)  /* Hangup (POSIX). */
#define SIGINT    __KOS_SIGNO(2)  /* Interrupt (ANSI). */
#define SIGQUIT   __KOS_SIGNO(3)  /* Quit (POSIX). */
#define SIGILL    __KOS_SIGNO(4)  /* Illegal instruction (ANSI). */
#define SIGTRAP   __KOS_SIGNO(5)  /* Trace trap (POSIX). */
#define SIGABRT   __KOS_SIGNO(6)  /* Abort (ANSI). */
#define SIGIOT    __KOS_SIGNO(6)  /* IOT trap (4.2 BSD). */
#ifdef __CYG_COMPAT__
#define SIGEMT    __KOS_SIGNO(7)  /* EMT instruction. */
#else
#define SIGBUS    __KOS_SIGNO(7)  /* BUS error (4.2 BSD). */
#endif
#define SIGFPE    __KOS_SIGNO(8)  /* Floating-point exception (ANSI). */
#define SIGKILL   __KOS_SIGNO(9)  /* Kill, unblockable (POSIX). */
#ifdef __CYG_COMPAT__
#define SIGBUS    __KOS_SIGNO(10) /* BUS error (4.2 BSD). */
#else
#define SIGUSR1   __KOS_SIGNO(10) /* User-defined signal 1 (POSIX). */
#endif
#define SIGSEGV   __KOS_SIGNO(11) /* Segmentation violation (ANSI). */
#ifdef __CYG_COMPAT__
#define SIGSYS    __KOS_SIGNO(12) /* Bad system call. */
#else
#define SIGUSR2   __KOS_SIGNO(12) /* User-defined signal 2 (POSIX). */
#endif
#define SIGPIPE   __KOS_SIGNO(13) /* Broken pipe (POSIX). */
#define SIGALRM   __KOS_SIGNO(14) /* Alarm clock (POSIX). */
#define SIGTERM   __KOS_SIGNO(15) /* Termination (ANSI). */
#define SIGCLD    SIGCHLD         /* Same as SIGCHLD (System V). */
#ifdef __CYG_COMPAT__
#define SIGURG    __KOS_SIGNO(16) /* Urgent condition on socket (4.2 BSD). */
#define SIGSTOP   __KOS_SIGNO(17) /* Stop, unblockable (POSIX). */
#define SIGTSTP   __KOS_SIGNO(18) /* Keyboard stop (POSIX). */
#define SIGCONT   __KOS_SIGNO(19) /* Continue (POSIX). */
#define SIGCHLD   __KOS_SIGNO(20) /* Child status has changed (POSIX). */
#else
#define SIGSTKFLT __KOS_SIGNO(16) /* Stack fault. */
#define SIGCHLD   __KOS_SIGNO(17) /* Child status has changed (POSIX). */
#define SIGCONT   __KOS_SIGNO(18) /* Continue (POSIX). */
#define SIGSTOP   __KOS_SIGNO(19) /* Stop, unblockable (POSIX). */
#define SIGTSTP   __KOS_SIGNO(20) /* Keyboard stop (POSIX). */
#endif
#define SIGTTIN   __KOS_SIGNO(21) /* Background read from tty (POSIX). */
#define SIGTTOU   __KOS_SIGNO(22) /* Background write to tty (POSIX). */
#ifdef __CYG_COMPAT__
#define SIGIO     __KOS_SIGNO(23) /* I/O now possible (4.2 BSD). */
#else
#define SIGURG    __KOS_SIGNO(23) /* Urgent condition on socket (4.2 BSD). */
#endif
#define SIGXCPU   __KOS_SIGNO(24) /* CPU limit exceeded (4.2 BSD). */
#define SIGXFSZ   __KOS_SIGNO(25) /* File size limit exceeded (4.2 BSD). */
#define SIGVTALRM __KOS_SIGNO(26) /* Virtual alarm clock (4.2 BSD). */
#define SIGPROF   __KOS_SIGNO(27) /* Profiling alarm clock (4.2 BSD). */
#define SIGWINCH  __KOS_SIGNO(28) /* Window size change (4.3 BSD, Sun). */
#define SIGPOLL   SIGIO           /* Pollable event occurred (System V). */
#ifdef __CYG_COMPAT__
#define SIGLOST   __KOS_SIGNO(29) /* Resource lost. */
#define SIGPWR    SIGLOST         /* Power failure restart (System V). */
#define SIGUSR1   __KOS_SIGNO(30) /* User-defined signal 1 (POSIX). */
#define SIGUSR2   __KOS_SIGNO(31) /* User-defined signal 2 (POSIX). */
#else
#define SIGIO     __KOS_SIGNO(29) /* I/O now possible (4.2 BSD). */
#define SIGPWR    __KOS_SIGNO(30) /* Power failure restart (System V). */
#define SIGSYS    __KOS_SIGNO(31) /* Bad system call. */
#define SIGUNUSED __KOS_SIGNO(31)
#endif
#define _NSIG     __KOS_SIGNO(65) /* Biggest signal number + 1 (including real-time signals). */

/* These are the hard limits of the kernel.
 * These values should not be used directly at user level. */
#define __SIGRTMIN  __KOS_SIGNO(32)
#define __SIGRTMAX (_NSIG-1)

#ifdef __CYG_COMPAT__
#define SIGRTMIN    __SIGRTMIN
#define SIGRTMAX    __SIGRTMAX
#else
#define SIGRTMIN   (__libc_current_sigrtmin())
#define SIGRTMAX   (__libc_current_sigrtmax())
#endif

#ifdef __USE_DOS
/* Define DOS's signal name aliases. */
#define SIGBREAK        __KOS_SIGNO(21) /* Background read from tty (POSIX). */
#define SIGABRT_COMPAT  __KOS_SIGNO(6)  /* Abort (ANSI). */
#ifdef __DOS_COMPAT__
/* Wow! Except for this oddity, DOS's signal codes are actually quite compatible. */
#undef SIGABRT
#define SIGABRT         __KOS_SIGNO(22) /* Background write to tty (POSIX). */
#endif /* __DOS_COMPAT__ */
#endif


__SYSDECL_END

#endif /* !_I386_KOS_BITS_SIGNUM_H */
