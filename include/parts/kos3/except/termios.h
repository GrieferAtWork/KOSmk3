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
#ifndef _PARTS_KOS3_EXCEPT_TERMIOS_H
#define _PARTS_KOS3_EXCEPT_TERMIOS_H 1

#include "__stdinc.h"
#include <features.h>
#ifndef _TERMIOS_H
#include <termios.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

__LIBC __PORT_KOSONLY speed_t (__LIBCCALL Xcfgetospeed)(struct termios const *__termios_p);
__LIBC __PORT_KOSONLY speed_t (__LIBCCALL Xcfgetispeed)(struct termios const *__termios_p);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xcfsetospeed)(struct termios *__termios_p, speed_t __speed);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xcfsetispeed)(struct termios *__termios_p, speed_t __speed);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xtcgetattr)(__fd_t __fd, struct termios *__termios_p);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xtcsetattr)(__fd_t __fd, int __optional_actions, struct termios const *__termios_p);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xtcsendbreak)(__fd_t __fd, int __duration);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xtcdrain)(__fd_t __fd);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xtcflush)(__fd_t __fd, int __queue_selector);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xtcflow)(__fd_t __fd, int __action);
#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)
__LIBC __PORT_KOSONLY __pid_t (__LIBCCALL Xtcgetsid)(__fd_t __fd);
#endif /* __USE_UNIX98 || __USE_XOPEN2K8 */
#ifdef __USE_MISC
__LIBC __PORT_KOSONLY void (__LIBCCALL Xcfsetspeed)(struct termios *__termios_p, speed_t __speed);
#endif /* __USE_MISC */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_TERMIOS_H */
