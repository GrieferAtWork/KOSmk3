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
#ifndef _KOS_THREAD_H
#define _KOS_THREAD_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <bits/types.h>

#if defined(__i386__) || defined(__x86_64__)
#include "i386-kos/thread.h"
#else
#error "Unsupported arch"
#endif

__DECL_BEGIN


#if 0 /* XXX: Isn't this what /proc is for? */
/* NOTE: Thread scope enumerate is still restricted to PID namespaces:
 *       You can't enumerate threads apart of a PID
 *       namespace that is invisible to you own thread. */
#define THREAD_SCOPE_CURRENT        0x01 /* Only enumerate the named thread (but still validate that it exists). */
#define THREAD_SCOPE_PROCESS        0x02 /* Enumerate all threads in the same process as the one specified. */
#define THREAD_SCOPE_CHILDREN       0x03 /* Enumerate all threads in, and parented by the one specified. */
#define THREAD_SCOPE_GRANDCHILDREN  0x04 /* Enumerate all threads in, and parented by the one specified, and any parented by those. */
#define THREAD_SCOPE_PROCESS_GROUP  0x05 /* Enumerate all threads apart of processes within the same process group as the thread specified. */
#define THREAD_SCOPE_SESSION        0x06 /* Enumerate all threads apart of processes within the same session as the thread specified. */
#define THREAD_SCOPE_GLOBAL         0x07 /* Enumerate all threads on the entire system (The given PID is ignored). */

#ifndef __KERNEL__
/* Enumerate threads.
 * @param: SCOPE:   The scope to enumerate (One of `THREAD_SCOPE_*')
 * @param: PID:     The PID of a thread apart of some grouping related to `SCOPE'.
 *                  You may pass ZERO(0) to use your own TID here.
 * @param: BUF:     A user-provided buffer to fill with PIDs.
 * @param: BUFSIZE: The size of the provided buffer (in bytes)
 * @return: * :     The required buffer size (in bytes). */
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__ssize_t,__LIBCCALL,enumerate_threads,(unsigned int __scope, __pid_t __pid, __pid_t *__buf, __size_t __bufsize),(__scope,__pid,__buf,__bufsize))
#ifdef __USE_EXCEPT
__LIBC __WUNUSED __size_t (__LIBCCALL Xenumerate_threads)(unsigned int __scope, __pid_t __pid, __pid_t *__buf, __size_t __bufsize);
#endif /* __USE_EXCEPT */
#endif
#endif


__DECL_END

#endif /* !_KOS_THREAD_H */
