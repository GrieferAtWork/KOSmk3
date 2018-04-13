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
#ifndef _PARTS_KOS3_EXCEPT_SYS_WAIT_H
#define _PARTS_KOS3_EXCEPT_SYS_WAIT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#ifndef _SYS_GENERIC_WAIT_H
#include <sys/wait.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

__LIBC __PORT_KOSONLY __pid_t (__LIBCCALL Xwait)(__WAIT_STATUS __stat_loc);
__LIBC __PORT_KOSONLY __pid_t (__LIBCCALL Xwaitpid)(__pid_t __pid, __WAIT_STATUS __stat_loc, int __options);

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K8)
__LIBC __PORT_KOSONLY int (__LIBCCALL Xwaitid)(idtype_t __idtype, __id_t __id, siginfo_t *__infop, int __options);
#endif /* __USE_XOPEN || __USE_XOPEN2K8 */

#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__LIBC __PORT_KOSONLY __pid_t (__LIBCCALL Xwait3)(__WAIT_STATUS __stat_loc, int __options, struct rusage *__usage);
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */
#ifdef __USE_MISC
__LIBC __PORT_KOSONLY __pid_t (__LIBCCALL Xwait4)(__pid_t __pid, __WAIT_STATUS __stat_loc, int __options, struct rusage *__usage);
#endif /* __USE_MISC */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SYS_WAIT_H */
