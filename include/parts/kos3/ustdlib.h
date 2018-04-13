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
#ifndef _PARTS_KOS3_USTDLIB_H
#define _PARTS_KOS3_USTDLIB_H 1

#include "__stdinc.h"
#include <features.h>
#include <bits/types.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

#ifndef __KERNEL__
#ifdef __CRT_KOS

__REDIRECT_UFSDPW16(__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),char16_t *,__LIBCCALL,w16getenv,(char16_t const *__name),wgetenv,(__name))
__REDIRECT_UFSW32  (__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),char32_t *,__LIBCCALL,w32getenv,(char32_t const *__name),wgetenv,(__name))
#if defined(__USE_MISC) || defined(__USE_XOPEN) || defined(__USE_DOS)
__REDIRECT_UFSDPW16(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,w16putenv,(char16_t const *__restrict __envstr),wputenv,(__envstr))
__REDIRECT_UFSW32  (__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,w32putenv,(char32_t const *__restrict __envstr),wputenv,(__envstr))
#endif /* __USE_MISC || __USE_XOPEN || __USE_DOS */

#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__REDIRECT_EXCEPT_UFSW16(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char16_t *,__LIBCCALL,w16realpath,(char16_t const *__restrict __name, char16_t *__resolved),wrealpath,(__name,__resolved))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char32_t *,__LIBCCALL,w32realpath,(char32_t const *__restrict __name, char32_t *__resolved),wrealpath,(__name,__resolved))
__REDIRECT_EXCEPT_W16(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char16_t *,__LIBCCALL,w16frealpath,(__fd_t __fd, char16_t *__resolved, __size_t __bufsize),wfrealpath,(__fd,__resolved,__bufsize))
__REDIRECT_EXCEPT_W32(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char32_t *,__LIBCCALL,w32frealpath,(__fd_t __fd, char32_t *__resolved, __size_t __bufsize),wfrealpath,(__fd,__resolved,__bufsize))
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */
#ifdef __USE_GNU
__REDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __XATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __NONNULL((1)),char16_t *,__LIBCCALL,w16canonicalize_file_name,(char16_t const *__name),wcanonicalize_file_name,(__name))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __XATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __NONNULL((1)),char32_t *,__LIBCCALL,w32canonicalize_file_name,(char32_t const *__name),wcanonicalize_file_name,(__name))
#endif /* __USE_GNU */

#ifdef __USE_EXCEPT
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__REDIRECT_W16(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char16_t *,__LIBCCALL,Xw16realpath,(char16_t const *__restrict __name, char16_t *__resolved),Xwrealpath,(__name,__resolved))
__REDIRECT_W32(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char32_t *,__LIBCCALL,Xw32realpath,(char32_t const *__restrict __name, char32_t *__resolved),Xwrealpath,(__name,__resolved))
__REDIRECT_W16(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char16_t *,__LIBCCALL,Xw16frealpath,(__fd_t __fd, char16_t *__resolved, __size_t __bufsize),Xwfrealpath,(__fd,__resolved,__bufsize))
__REDIRECT_W32(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char32_t *,__LIBCCALL,Xw32frealpath,(__fd_t __fd, char32_t *__resolved, __size_t __bufsize),Xwfrealpath,(__fd,__resolved,__bufsize))
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */
#ifdef __USE_GNU
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __NONNULL((1)),char16_t *,__LIBCCALL,Xw16canonicalize_file_name,(char16_t const *__name),Xwcanonicalize_file_name,(__name))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __NONNULL((1)),char32_t *,__LIBCCALL,Xw32canonicalize_file_name,(char32_t const *__name),Xwcanonicalize_file_name,(__name))
#endif /* __USE_GNU */
#endif /* __USE_EXCEPT */

#endif /* __CRT_KOS */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_PARTS_KOS3_USTDLIB_H */
