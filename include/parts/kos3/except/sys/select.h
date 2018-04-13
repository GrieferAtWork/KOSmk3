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
#ifndef _PARTS_KOS3_EXCEPT_SYS_SELECT_H
#define _PARTS_KOS3_EXCEPT_SYS_SELECT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#ifndef _SYS_GENERIC_SELECT_H
#include <sys/select.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

__REDIRECT_TM64(__LIBC,,unsigned int,__LIBCCALL,Xselect,
               (int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds,
                fd_set *__restrict __exceptfds, struct timeval *__restrict __timeout),
               (__nfds,__readfds,__writefds,__exceptfds,__timeout))
#ifdef __USE_XOPEN2K
__REDIRECT_TM64(__LIBC,,unsigned int,__LIBCCALL,Xpselect,
               (int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds,
                fd_set *__restrict __exceptfds, struct timespec const *__restrict __timeout,
                __sigset_t const *__restrict __sigmask),
               (__nfds,__readfds,__writefds,__exceptfds,__timeout,__sigmask))
#endif /* __USE_XOPEN2K */
#ifdef __USE_TIME64
__LIBC unsigned int (__LIBCCALL Xselect64)(int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds,
                                           fd_set *__restrict __exceptfds, struct timeval64 *__restrict __timeout);
#ifdef __USE_XOPEN2K
__LIBC unsigned int (__LIBCCALL Xpselect64)(int __nfds, fd_set *__restrict __readfds, fd_set *__restrict __writefds,
                                            fd_set *__restrict __exceptfds, struct __timespec64 const *__restrict __timeout,
                                            __sigset_t const *__restrict __sigmask);
#endif /* __USE_XOPEN2K */
#endif /* __USE_TIME64 */


__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SYS_SELECT_H */
