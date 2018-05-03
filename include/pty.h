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
#ifndef _PTY_H
#define _PTY_H 1

#include <features.h>
#include <termios.h>
#include <bits/types.h>
#include <sys/ioctl.h>

#if !defined(__CRT_GLC) && !defined(__CRT_CYG)
#error "<pty.h> is not supported by the linked libc"
#endif /* !__CRT_GLC && !__CRT_CYG */

__SYSDECL_BEGIN

struct termios;
struct winsize;

#ifndef __KERNEL__
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS,int,__LIBCCALL,openpty,(__fd_t *__amaster, __fd_t *__aslave, char *__name, struct termios const *__termp, struct winsize const *__winp),(__amaster,__aslave,__name,__termp,__winp))
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS,__pid_t,__LIBCCALL,forkpty,(__fd_t *__amaster, char *__name, struct termios const *__termp, struct winsize const *__winp),(__amaster,__name,__termp,__winp))
#endif /* !__KERNEL__ */

__SYSDECL_END

#ifdef __USE_EXCEPT
#include "parts/kos3/except/pty.h"
#endif

#endif /* !_PTY_H */
