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
#ifndef _PARTS_KOS3_EXCEPT_DIRECT_H
#define _PARTS_KOS3_EXCEPT_DIRECT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#ifndef _DIRECT_H
#include <direct.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

__REDIRECT(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED_SUGGESTED,char *,__LIBCCALL,_Xgetcwd,(char *__buf, __size_t __size),Xgetcwd,(__buf,__size))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,_Xchdir,(char const *__path),Xchdir,(__path))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,_Xrmdir,(char const *__path),Xrmdir,(__path))

#ifdef __CRT_DOS
#define _Xgetdcwd_nolock _Xgetdcwd
__REDIRECT(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED_SUGGESTED,char *,__LIBCCALL,_Xgetdcwd,(int __drive, char *__buf, __size_t __size),Xgetdcwd,(__drive,__buf,__size))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,_Xchdrive,(int __drive),Xchdrive,(__drive))
__REDIRECT(__LIBC,__WUNUSED __PORT_KOSONLY,int,__LIBCCALL,_Xgetdrive,(void),Xgetdrive,())
#endif /* __CRT_DOS */

/* A small hand full of functions defined in '<direct.h>' */
#ifndef __Xgetcwd_defined
#define __Xgetcwd_defined 1
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED_SUGGESTED char *(__LIBCCALL Xgetcwd)(char *__buf, __size_t __size);
#endif /* !__Xgetcwd_defined */
#ifndef __Xchdir_defined
#define __Xchdir_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xchdir)(char const *__path);
#endif /* !__Xchdir_defined */
#ifndef __Xrmdir_defined
#define __Xrmdir_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xrmdir)(char const *__path);
#endif /* !__Xrmdir_defined */
__REDIRECT_VOID(__LIBC,__NONNULL((1)),__LIBCCALL,__kos_Xmkdir,(char const *__path, __mode_t __mode),Xmkdir,(__path,__mode))
__LOCAL __NONNULL((1)) void (__LIBCCALL _Xmkdir)(char const *__path) { __kos_Xmkdir(__path,0755); }

#ifndef __Xmkdir_defined
#define __Xmkdir_defined 1
__LOCAL __NONNULL((1)) void (__LIBCCALL Xmkdir)(char const *__path) { __kos_Xmkdir(__path,0755); }
#endif /* !__Xmkdir_defined */


__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_DIRECT_H */
