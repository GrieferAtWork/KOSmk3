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
#ifndef _PARTS_KOS3_WSTDLIB_H
#define _PARTS_KOS3_WSTDLIB_H 1

#include "__stdinc.h"
#include <features.h>
#include <bits/types.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef __KERNEL__

__REDIRECT_UFSDPA(__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),wchar_t *,__LIBCCALL,wgetenv,(wchar_t const *__name),(__name))
#if defined(__USE_MISC) || defined(__USE_XOPEN) || defined(__USE_DOS)
#ifndef __wputenv_defined
#define __wputenv_defined 1
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,wputenv,(wchar_t const *__restrict __envstr),(__envstr))
#endif /* !__wputenv_defined */
#endif /* __USE_MISC || __USE_XOPEN || __USE_DOS */

#ifdef __CRT_KOS
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__REDIRECT_EXCEPT_UFS(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,wchar_t *,__LIBCCALL,wrealpath,
                     (wchar_t const *__restrict __name, wchar_t *__resolved),(__name,__resolved))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,wchar_t *,__LIBCCALL,wfrealpath,
                 (__fd_t __fd, wchar_t *__resolved, __size_t __bufsize),(__fd,__resolved,__bufsize))
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */
#ifdef __USE_GNU
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY __XATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __NONNULL((1)),wchar_t *,__LIBCCALL,wcanonicalize_file_name,(wchar_t const *__name),(__name))
#endif /* __USE_GNU */

#ifdef __USE_EXCEPT
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__LIBC __ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED wchar_t *(__LIBCCALL Xwrealpath)(wchar_t const *__restrict __name, wchar_t *__resolved);
__LIBC __ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED wchar_t *(__LIBCCALL Xwfrealpath)(__fd_t __fd, wchar_t *__resolved, __size_t __bufsize);
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */
#ifdef __USE_GNU
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __NONNULL((1)) wchar_t *(__LIBCCALL Xwcanonicalize_file_name)(wchar_t const *__name);
#endif /* __USE_GNU */
#endif /* __USE_EXCEPT */
#endif /* __CRT_KOS */

#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_PARTS_KOS3_WSTDLIB_H */
