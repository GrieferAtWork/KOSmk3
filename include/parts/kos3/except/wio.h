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
#ifndef _PARTS_KOS3_EXCEPT_WIO_H
#define _PARTS_KOS3_EXCEPT_WIO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

__VREDIRECT(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),__fd_t,__ATTR_CDECL,_Xwopen,(wchar_t const *__restrict __file, __oflag_t __oflag, ...),Xwopen,TODO,(__file,__oflag),__oflag)
#if 0
__VREDIRECT(__LIBC,__PORT_DOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),__fd_t,__ATTR_CDECL,_Xwsopen,(wchar_t const *__restrict __file, __oflag_t __oflag, int __sflag, ...),Xwsopen,TODO,(__file,__oflag,__sflag),__sflag)
#endif
__REDIRECT(__LIBC,__PORT_KOSONLY __WUNUSED __NONNULL((1)),__fd_t,__LIBCCALL,_Xwcreat,(wchar_t const *__restrict __file, __mode_t __mode),Xwcreat,(__file,__mode))
__REDIRECT_VOID(__LIBC,__PORT_DOSONLY __NONNULL((1)),__LIBCCALL,_Xwaccess,(wchar_t const *__restrict __file, int __type),Xwaccess,(__file,__type))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,_Xwchmod,(wchar_t const *__restrict __file, __mode_t __mode),Xwchmod,(__file,__mode))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,_Xwunlink,(wchar_t const *__restrict __file),Xwunlink,(__file))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,_Xwrename,(wchar_t const *__oldname, wchar_t const *__newname),Xwrename,(__oldname,__newname))

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_WIO_H */
