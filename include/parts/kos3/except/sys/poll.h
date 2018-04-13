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
#ifndef _PARTS_KOS3_EXCEPT_SYS_POLL_H
#define _PARTS_KOS3_EXCEPT_SYS_POLL_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#ifdef __USE_GNU
#include <bits/sigset.h>
#include <hybrid/timespec.h>
#endif /* __USE_GNU */

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

#ifndef __nfds_t_defined
#define __nfds_t_defined 1
typedef __UINTPTR_TYPE__ nfds_t;
#endif /* !__nfds_t_defined */

struct pollfd;
struct timespec;

__LIBC unsigned int (__LIBCCALL Xpoll)(struct pollfd *__fds, nfds_t __nfds, int __timeout);

#ifdef __USE_GNU
__REDIRECT_TM64(__LIBC,,unsigned int,__LIBCCALL,Xppoll,
               (struct pollfd *__fds, nfds_t __nfds, struct timespec const *__timeout, __sigset_t const *__ss),
               (__fds,__nfds,__timeout,__ss))
#ifdef __USE_TIME64
__LIBC unsigned int (__LIBCCALL Xppoll64)(struct pollfd *__fds, nfds_t __nfds,
                                          struct __timespec64 const *__timeout,
                                          __sigset_t const *__ss);
#endif /* __USE_TIME64 */
#endif /* __USE_GNU */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SYS_POLL_H */
