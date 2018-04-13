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
#ifndef _PARTS_KOS2_WFCNTL_H
#define _PARTS_KOS2_WFCNTL_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef __KERNEL__
#ifdef __CRT_DOS

#if defined(__USE_FILE_OFFSET64) && !defined(__DOS_COMPAT__)
__VREDIRECT_EXCEPT_UFS64(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__ATTR_CDECL,wopen,(wchar_t const *__file, int __oflag, ...),TODO,(__file,__oflag),__oflag)
__REDIRECT_EXCEPT_UFS64(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,wcreat,(wchar_t const *__file, __mode_t __mode),(__file,__mode))
#else
__VREDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__ATTR_CDECL,wopen,(wchar_t const *__file, int __oflag, ...),TODO,(__file,__oflag),__oflag)
__REDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,wcreat,(wchar_t const *__file, __mode_t __mode),(__file,__mode))
#endif

#ifdef __USE_EXCEPT
__VREDIRECT_FS64(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__ATTR_CDECL,Xwopen,(wchar_t const *__file, int __oflag, ...),TODO,(__file,__oflag),__oflag)
__REDIRECT_FS64(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,Xwcreat,(wchar_t const *__file, __mode_t __mode),(__file,__mode))
#endif /* __USE_EXCEPT */

#endif /* __CRT_DOS */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_PARTS_KOS2_WFCNTL_H */
