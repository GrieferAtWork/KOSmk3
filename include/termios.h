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
#ifndef _TERMIOS_H
#define _TERMIOS_H 1

#include <features.h>
#include <sys/ioctl.h>
#include <bits/termios.h>
#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)
#include <bits/types.h>
#endif /* __USE_UNIX98 || __USE_XOPEN2K8 */
#ifdef __USE_MISC
#include <sys/ttydefaults.h>
#endif /* __USE_MISC */

__SYSDECL_BEGIN

#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)
#ifndef __pid_t_defined
#define __pid_t_defined 1
typedef __pid_t pid_t;
#endif /* !__pid_t_defined */
#endif /* __USE_UNIX98 || __USE_XOPEN2K8 */

#ifndef __KERNEL__
#ifdef __CRT_GLC
__REDIRECT_EXCEPT(__LIBC,__PORT_NODOS,speed_t,__LIBCCALL,cfgetospeed,(struct termios const *__termios_p),(__termios_p))
__REDIRECT_EXCEPT(__LIBC,__PORT_NODOS,speed_t,__LIBCCALL,cfgetispeed,(struct termios const *__termios_p),(__termios_p))
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS,int,__LIBCCALL,cfsetospeed,(struct termios *__termios_p, speed_t __speed),(__termios_p,__speed))
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS,int,__LIBCCALL,cfsetispeed,(struct termios *__termios_p, speed_t __speed),(__termios_p,__speed))
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS,int,__LIBCCALL,tcgetattr,(__fd_t __fd, struct termios *__termios_p),(__fd,__termios_p))
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS,int,__LIBCCALL,tcsetattr,(__fd_t __fd, int __optional_actions, struct termios const *__termios_p),(__fd,__optional_actions,__termios_p))
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS,int,__LIBCCALL,tcsendbreak,(__fd_t __fd, int __duration),(__fd,__duration))
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS,int,__LIBCCALL,tcdrain,(__fd_t __fd),(__fd))
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS,int,__LIBCCALL,tcflush,(__fd_t __fd, int __queue_selector),(__fd,__queue_selector))
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS,int,__LIBCCALL,tcflow,(__fd_t __fd, int __action),(__fd,__action))
#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS,__pid_t,__LIBCCALL,tcgetsid,(__fd_t __fd),(__fd))
#endif /* __USE_UNIX98 || __USE_XOPEN2K8 */
#ifdef __USE_MISC
#define CCEQ(val,c)   ((c) == (val) && (val) != _POSIX_VDISABLE)
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS,int,__LIBCCALL,cfsetspeed,(struct termios *__termios_p, speed_t __speed),(__termios_p,__speed))
__LIBC __PORT_NODOS void (__LIBCCALL cfmakeraw)(struct termios *__termios_p);
#endif /* __USE_MISC */
#endif /* __CRT_GLC */
#endif /* !__KERNEL__ */

__SYSDECL_END

#ifdef __USE_EXCEPT
#include "parts/kos3/except/termios.h"
#endif

#endif /* !_TERMIOS_H */
