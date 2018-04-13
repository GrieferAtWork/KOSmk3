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
#ifndef _SYS_GENERIC_IOCTL_H
#define _SYS_GENERIC_IOCTL_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/ioctls.h>
#include <bits/types.h>
#include <bits/ioctl-types.h>
#include <sys/ttydefaults.h>

__SYSDECL_BEGIN

#ifndef __KERNEL__
#ifdef __CRT_GLC
#if defined(__USE_KOS) && defined(__CRT_KOS)
__VREDIRECT_EXCEPT(__LIBC,__PORT_NODOS,__ssize_t,__ATTR_CDECL,ioctl,(__fd_t __fd, unsigned long int __request, ...),TODO,(__fd,__request),__request)
#else /* __USE_KOS && __CRT_KOS */
__VREDIRECT_EXCEPT(__LIBC,__PORT_NODOS,int,__ATTR_CDECL,ioctl,(__fd_t __fd, unsigned long int __request, ...),TODO,(__fd,__request),__request)
#endif /* !__USE_KOS || !__CRT_KOS */
#endif /* __CRT_GLC */

#ifdef __USE_KOS3
__VREDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,__ssize_t,__ATTR_CDECL,
                   xioctlf,(__fd_t __fd, unsigned long int __request, __oflag_t __flags, ...),
                   TODO,(__fd,__request,__flags),__flags)
#endif /* __USE_KOS3 */

#endif /* !__KERNEL__ */

__SYSDECL_END


#ifdef __USE_EXCEPT
#include <parts/kos3/except/sys/ioctl.h>
#endif

#endif /* !_SYS_GENERIC_IOCTL_H */
