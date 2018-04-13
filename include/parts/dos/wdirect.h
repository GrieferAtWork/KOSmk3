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
#ifndef _PARTS_DOS_WDIRECT_H
#define _PARTS_DOS_WDIRECT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef _WDIRECT_DEFINED
#define _WDIRECT_DEFINED 1
#ifdef __CRT_DOS
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__XATTR_RETNONNULL __PORT_NODOS __NONNULL((1)),wchar_t *,__LIBCCALL,wgetcwd,(wchar_t *__restrict __dstbuf, int __dstlen),(__dstbuf,__dstlen))
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__XATTR_RETNONNULL __PORT_NODOS __NONNULL((2)),wchar_t *,__LIBCCALL,wgetdcwd,(int __drive, wchar_t *__dstbuf, int __dstlen),(__drive,__dstbuf,__dstlen))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,wchdir,(wchar_t const *__restrict __path),(__path))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,wrmdir,(wchar_t const *__restrict __path),(__path))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,wmkdir,(wchar_t const *__restrict __path),(__path))
#define _wgetdcwd_nolock    _wgetdcwd
#endif /* __CRT_DOS */
#endif /* !_WDIRECT_DEFINED */

__SYSDECL_END

#endif /* !_PARTS_DOS_WDIRECT_H */
