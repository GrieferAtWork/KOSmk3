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
#ifndef _PARTS_KOS2_UFCNTL_H
#define _PARTS_KOS2_UFCNTL_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

#ifdef __CRT_KOS

#ifdef __USE_FILE_OFFSET64
__VREDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__ATTR_CDECL,w16open,(char16_t const *__file, int __oflag, ...),wopen64,TODO,(__file,__oflag),__oflag)
__VREDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__ATTR_CDECL,w32open,(char32_t const *__file, int __oflag, ...),wopen64,TODO,(__file,__oflag),__oflag)
__REDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16creat,(char16_t const *__file, __mode_t __mode),wcreat64,(__file,__mode))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32creat,(char32_t const *__file, __mode_t __mode),wcreat64,(__file,__mode))
#ifdef __USE_EXCEPT
__VREDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__ATTR_CDECL,Xw16open,(char16_t const *__file, int __oflag, ...),Xwopen64,TODO,(__file,__oflag),__oflag)
__VREDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__ATTR_CDECL,Xw32open,(char32_t const *__file, int __oflag, ...),Xwopen64,TODO,(__file,__oflag),__oflag)
__REDIRECT_W16 (__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,Xw16creat,(char16_t const *__file, __mode_t __mode),Xwcreat64,(__file,__mode))
__REDIRECT_W32 (__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,Xw32creat,(char32_t const *__file, __mode_t __mode),Xwcreat64,(__file,__mode))
#endif /* __USE_EXCEPT */
#else /* __USE_FILE_OFFSET64 */
__VREDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__ATTR_CDECL,w16open,(char16_t const *__file, int __oflag, ...),wopen,TODO,(__file,__oflag),__oflag)
__VREDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__ATTR_CDECL,w32open,(char32_t const *__file, int __oflag, ...),wopen,TODO,(__file,__oflag),__oflag)
__REDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16creat,(char16_t const *__file, __mode_t __mode),wcreat,(__file,__mode))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32creat,(char32_t const *__file, __mode_t __mode),wcreat,(__file,__mode))
#ifdef __USE_EXCEPT
__VREDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__ATTR_CDECL,Xw16open,(char16_t const *__file, int __oflag, ...),Xwopen,TODO,(__file,__oflag),__oflag)
__VREDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__ATTR_CDECL,Xw32open,(char32_t const *__file, int __oflag, ...),Xwopen,TODO,(__file,__oflag),__oflag)
__REDIRECT_W16 (__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,Xw16creat,(char16_t const *__file, __mode_t __mode),Xwcreat,(__file,__mode))
__REDIRECT_W32 (__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,Xw32creat,(char32_t const *__file, __mode_t __mode),Xwcreat,(__file,__mode))
#endif /* __USE_EXCEPT */
#endif /* !__USE_FILE_OFFSET64 */

#endif /* __CRT_KOS */

__SYSDECL_END

#endif /* !_PARTS_KOS2_UFCNTL_H */
