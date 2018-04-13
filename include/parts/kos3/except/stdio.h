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
#ifndef _PARTS_KOS3_EXCEPT_STDIO_H
#define _PARTS_KOS3_EXCEPT_STDIO_H 1

#include "__stdinc.h"
#include <features.h>
#ifndef _STDIO_H
#include <stdio.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

#ifndef __Xremove_defined
#define __Xremove_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xremove)(char const *__file);
#endif /* !__Xremove_defined */
#ifndef __Xrename_defined
#define __Xrename_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xrename)(char const *__old, char const *__new);
#endif /* !__Xrename_defined */

#ifdef __USE_KOS
__LIBC __PORT_KOSONLY void (__LIBCCALL Xremoveat)(__fd_t __dfd, char const *__filename);
#endif /* __USE_KOS */
#ifdef __USE_ATFILE
__LIBC __PORT_KOSONLY void (__LIBCCALL Xrenameat)(int __oldfd, char const *__old, int __newfd, char const *__new);
#ifdef __USE_KOS
__LIBC __PORT_KOSONLY void (__LIBCCALL Xfrenameat)(int __oldfd, char const *__old, int __newfd, char const *__new, int __flags);
#endif /* __USE_KOS */
#endif /* __USE_ATFILE */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_STDIO_H */
