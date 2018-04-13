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
#ifndef _PARTS_KOS3_EXCEPT_WDIRECT_H
#define _PARTS_KOS3_EXCEPT_WDIRECT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#ifndef _DIRECT_H
#include <direct.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

__REDIRECT(__LIBC,__XATTR_RETNONNULL __PORT_NODOS __NONNULL((1)),wchar_t *,__LIBCCALL,_Xwgetcwd,(wchar_t *__restrict __dstbuf, int __dstlen),Xwgetcwd,(__dstbuf,__dstlen))
__REDIRECT(__LIBC,__XATTR_RETNONNULL __PORT_NODOS __NONNULL((2)),wchar_t *,__LIBCCALL,_Xwgetdcwd,(int __drive, wchar_t *__dstbuf, int __dstlen),Xwgetdcwd,(__drive,__dstbuf,__dstlen))
__REDIRECT_VOID(__LIBC,__PORT_NODOS __NONNULL((1)),__LIBCCALL,_Xwchdir,(wchar_t const *__restrict __path),Xwchdir,(__path))
__REDIRECT_VOID(__LIBC,__PORT_NODOS __NONNULL((1)),__LIBCCALL,_Xwrmdir,(wchar_t const *__restrict __path),Xwrmdir,(__path))
__REDIRECT_VOID(__LIBC,__PORT_NODOS __NONNULL((1)),__LIBCCALL,_Xwmkdir,(wchar_t const *__restrict __path),Xwmkdir,(__path))
#define _Xwgetdcwd_nolock    _Xwgetdcwd

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_WDIRECT_H */
