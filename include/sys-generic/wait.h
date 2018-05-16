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
#ifndef _SYS_GENERIC_WAIT_H
#define _SYS_GENERIC_WAIT_H 1
#define _SYS_WAIT_H 1

#include <features.h>
#include <signal.h>
#include <bits/types.h>
#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K8)
#include <bits/siginfo.h> /* We'd only need `siginfo_t' */
#endif /* __USE_XOPEN || __USE_XOPEN2K8 */

/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
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

#ifndef __CRT_GLC
#error "<sys/wait.h> is not supported by the linked libc"
#endif /* !__CRT_GLC */

#ifndef __WAIT_MACROS_DEFINED
#include <bits/waitflags.h>
#include <bits/waitstatus.h>
#endif /* !__WAIT_MACROS_DEFINED */

__SYSDECL_BEGIN

#ifndef __WAIT_MACROS_DEFINED
#define __WAIT_MACROS_DEFINED 1

#ifdef __USE_MISC
#if defined(__GNUC__) && !defined(__cplusplus)
#   define __WAIT_INT(status) (__extension__(((union{ __typeof__(status) __in; int __i; }) { .__in = (status) }).__i))
#else
#   define __WAIT_INT(status) (*(int *)&(status))
#endif
#ifdef __NO_ATTR_TRANSPARENT_UNION
#   define __WAIT_STATUS      void *
#   define __WAIT_STATUS_DEFN void *
#else
typedef union {
    union wait *__uptr;
    int        *__iptr;
} __WAIT_STATUS __ATTR_TRANSPARENT_UNION;
#   define __WAIT_STATUS_DEFN int *
#endif
#else /* __USE_MISC */
#   define __WAIT_INT(status)  (status)
#   define __WAIT_STATUS        int *
#   define __WAIT_STATUS_DEFN   int *
#endif /* !__USE_MISC */
#   define WEXITSTATUS(status)  __WEXITSTATUS(__WAIT_INT(status))
#   define WTERMSIG(status)     __WTERMSIG(__WAIT_INT(status))
#   define WSTOPSIG(status)     __WSTOPSIG(__WAIT_INT(status))
#   define WIFEXITED(status)    __WIFEXITED(__WAIT_INT(status))
#   define WIFSIGNALED(status)  __WIFSIGNALED(__WAIT_INT(status))
#   define WIFSTOPPED(status)   __WIFSTOPPED(__WAIT_INT(status))
#ifdef __WIFCONTINUED
#   define WIFCONTINUED(status) __WIFCONTINUED(__WAIT_INT(status))
#endif
#endif /* !__WAIT_MACROS_DEFINED */

#ifdef __USE_MISC
#   define WCOREFLAG           __WCOREFLAG
#   define WCOREDUMP(status)   __WCOREDUMP(__WAIT_INT(status))
#   define W_EXITCODE(ret,sig) __W_EXITCODE(ret,sig)
#   define W_STOPCODE(sig)     __W_STOPCODE(sig)
#endif

#ifdef __USE_MISC
#   define WAIT_ANY     (-1) /* Any process. */
#   define WAIT_MYPGRP    0  /* Any process in my process group. */
#endif


#ifndef __KERNEL__
__REDIRECT_EXCEPT(__LIBC,,__pid_t,__LIBCCALL,wait,(__WAIT_STATUS __stat_loc),(__stat_loc))
__REDIRECT_EXCEPT(__LIBC,,__pid_t,__LIBCCALL,waitpid,(__pid_t __pid, __WAIT_STATUS __stat_loc, int __options),(__pid,__stat_loc,__options))
#endif /* !__KERNEL__ */

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K8)
#ifndef __id_t_defined
#define __id_t_defined 1
typedef __id_t id_t;
#endif /* !__id_t_defined */
#ifndef __KERNEL__
__REDIRECT_EXCEPT(__LIBC,,int,__LIBCCALL,waitid,(idtype_t __idtype, __id_t __id, siginfo_t *__infop, int __options),(__idtype,__id,__infop,__options))
#endif /* !__KERNEL__ */
#endif /* __USE_XOPEN || __USE_XOPEN2K8 */

#ifndef __KERNEL__
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
struct rusage;
__REDIRECT_EXCEPT(__LIBC,,__pid_t,__LIBCCALL,wait3,(__WAIT_STATUS __stat_loc, int __options, struct rusage *__usage),(__stat_loc,__options,__usage))
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */
#ifdef __USE_MISC
__REDIRECT_EXCEPT(__LIBC,,__pid_t,__LIBCCALL,wait4,(__pid_t __pid, __WAIT_STATUS __stat_loc, int __options, struct rusage *__usage),(__pid,__stat_loc,__options,__usage))
#endif /* __USE_MISC */

#ifdef __USE_KOS3
/* >> detach(2)
 * Detach the descriptor of `PID' from the thread that
 * would have received a signal when it changes state,
 * as well as prevent the thread from turning into a
 * zombie once it dies.
 * For simplicity, think of it like this:
 *   - pthread_create()  -->  clone()
 *   - pthread_join()    -->  wait()
 *   - pthread_detach()  -->  detach()  // Linux's missing link, now implemented
 * Before this is done, the thread referred to by this is one of the following:
 *   - The leader of the process that called `fork()' or `clone()' without
 *    `CLONE_PARENT' to create the thread referred to by `PID'
 *   - The creator of the process containing a thread that called
 *    `clone()' with `CLONE_PARENT', which then created the thread
 *     referred to by `PID'.
 *   - Even if the thread doesn't deliver a signal upon it terminating,
 *     the process that would have received such a signal is still relevant.
 *   -> In other words: The thread `PID' must be one of your children,
 *                      or you had to have been assigned as its child.
 * If the calling thread isn't part of that process that will receive
 * the signal if the thread dies without being detached first, then
 * the call fails by throwing an `E_ILLEGAL_OPERATION'.
 * If the thread had already been detached, then the call fails by
 * throwing an `E_ILLEGAL_OPERATION' as well.
 * Upon success, the thread referred to by `PID' will clean up its own
 * PID descriptor without the need of anyone to wait() for it, a behavior
 * that linux implements using `CLONE_THREAD' (which you shouldn't use,
 * because it's flawed by design)
 * Once detached, any further use of PID results in a race condition
 * (which linux neglects to mention for `CLONE_THREAD'), because there
 * is no way of ensuring that PID still refers to the original thread,
 * as another thread may have been created using the same PID, after
 * the detached thread exited.
 * NOTE: If a thread is crated using clone() with `CLONE_DETACHED' set,
 *       it will behave effectively as though this function had already
 *       be called.
 * NOTE: If the thread already has terminated, detaching it will kill
 *       its zombie the same way wait() would.
 * NOTE: Passing ZERO(0) for `PID' will detach the calling thread.
 *       However, this operation fails if the calling thread isn't
 *       part of the same process as the parent process of the thread.
 *       In other words, the child of a fork() can't do this, and
 *       neither can the spawnee of clone(CLONE_THREAD|CLONE_PARENT),
 *       clone(0) or clone(CLONE_PARENT).
 * @throw: E_ILLEGAL_OPERATION: The calling process isn't the recipient of signals
 *                              delivered when `PID' changes state. This can either
 *                              be because `PID' has already been detached, or because
 *                              YOU CAN'T DETACH SOMEONE ELSE'S THREAD!
 *                              Another possibility is that the thread was already
 *                              detached, then exited, following which a new thread
 *                              got created and had been assigned the PID of your
 *                              ancient, no longer existent thread.
 * @throw: E_PROCESS_EXITED:    The process referred to by `PID' doesn't exist.
 *                              This could mean that it had already been detached
 *                              and exited, or that the `PID' is just invalid (which
 *                              would also be the case if it was valid at some point) */
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,detach,(__pid_t __pid),(__pid))
#ifdef __USE_EXCEPT
__LIBC void (__LIBCCALL Xdetach)(__pid_t __pid);
#endif /* __USE_EXCEPT */
#endif /* __USE_KOS3 */
#endif /* !__KERNEL__ */

__SYSDECL_END

#ifdef __USE_EXCEPT
#include <parts/kos3/except/sys/wait.h>
#endif

#endif /* !_SYS_GENERIC_WAIT_H */
