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
#ifndef _PARTS_KOS3_EXCEPT_UIO_H
#define _PARTS_KOS3_EXCEPT_UIO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

__VREDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__ATTR_CDECL,_Xw16open,(char16_t const *__restrict __file, int __oflag, ...),Xwopen,TODO,(__file,__oflag),__oflag)
__VREDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__ATTR_CDECL,_Xw32open,(char32_t const *__restrict __file, int __oflag, ...),Xwopen,TODO,(__file,__oflag),__oflag)
#if 0 /* Nope. Doesn't exist. */
__VREDIRECT_W16(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__ATTR_CDECL,_Xw16sopen,(char16_t const *__restrict __file, int __oflag, int __sflag, ...),Xwsopen,TODO,(__file,__oflag,__sflag),__sflag)
__VREDIRECT_W32(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__ATTR_CDECL,_Xw32sopen,(char32_t const *__restrict __file, int __oflag, int __sflag, ...),Xwsopen,TODO,(__file,__oflag,__sflag),__sflag)
#endif
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __NONNULL((1)),int,__LIBCCALL,_Xw16creat,(char16_t const *__restrict __file, int __pmode),Xwcreat,(__file,__pmode))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __NONNULL((1)),int,__LIBCCALL,_Xw32creat,(char32_t const *__restrict __file, int __pmode),Xwcreat,(__file,__pmode))
__REDIRECT_W16_VOID(__LIBC,__PORT_DOSONLY __NONNULL((1)),__LIBCCALL,_Xw16access,(char16_t const *__restrict __file, int __type),Xwaccess,(__file,__type))
__REDIRECT_W32_VOID(__LIBC,__PORT_DOSONLY __NONNULL((1)),__LIBCCALL,_Xw32access,(char32_t const *__restrict __file, int __type),Xwaccess,(__file,__type))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,_Xw16chmod,(char16_t const *__restrict __file, int __mode),Xwchmod,(__file,__mode))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,_Xw32chmod,(char32_t const *__restrict __file, int __mode),Xwchmod,(__file,__mode))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,_Xw16unlink,(char16_t const *__restrict __file),Xwunlink,(__file))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,_Xw32unlink,(char32_t const *__restrict __file),Xwunlink,(__file))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,_Xw16rename,(char16_t const *__oldname, wchar_t const *__newname),Xwrename,(__oldname,__newname))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,_Xw32rename,(char32_t const *__oldname, wchar_t const *__newname),Xwrename,(__oldname,__newname))

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_UIO_H */
