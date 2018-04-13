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
#ifndef _PARTS_KOS3_EXCEPT_FCNTL_H
#define _PARTS_KOS3_EXCEPT_FCNTL_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

#ifndef __Xfcntl_defined
#define __Xfcntl_defined 1
#ifdef __USE_KOS
__LIBC __PORT_KOSONLY __ssize_t (__ATTR_CDECL Xfcntl)(__fd_t __fd, int __cmd, ...);
#else /* __USE_KOS */
__LIBC __PORT_KOSONLY int (__ATTR_CDECL Xfcntl)(__fd_t __fd, int __cmd, ...);
#endif /* !__USE_KOS */
#endif /* !__Xfcntl_defined */

#ifndef __Xopen_defined
#define __Xopen_defined 1
__VREDIRECT_FS64(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__ATTR_CDECL,Xopen,(char const *__file, int __oflag, ...),TODO,(__file,__oflag),__oflag)
#endif /* !__Xopen_defined */
#ifndef __Xcreat_defined
#define __Xcreat_defined 1
__REDIRECT_FS64(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,Xcreat,(char const *__file, __mode_t __mode),(__file,__mode))
#endif /* !__Xcreat_defined */

#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY __NONNULL((1)) int (__ATTR_CDECL Xopen64)(char const *__file, int __oflag, ...);
__LIBC __PORT_KOSONLY __NONNULL((1)) int (__LIBCCALL Xcreat64)(char const *__file, __mode_t __mode);
#endif /* __USE_LARGEFILE64 */

#ifdef __USE_ATFILE
__VREDIRECT_FS64(__LIBC,__NONNULL((2)),int,__ATTR_CDECL,Xopenat,(__fd_t __dfd, char const *__file, int __oflag, ...),TODO,(__file,__oflag),__oflag)
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY __NONNULL((2)) int (__ATTR_CDECL Xopenat64)(__fd_t __dfd, char const *__file, int __oflag, ...);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_ATFILE */
#ifdef __USE_XOPEN2K
__REDIRECT_FS64_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xposix_fadvise,(__fd_t __fd, __FS_TYPE(off) __offset, __FS_TYPE(off) __len, int __advise),(__fd,__offset,__len,__advise))
__REDIRECT_FS64_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xposix_fallocate,(__fd_t __fd, __FS_TYPE(off) __offset, __FS_TYPE(off) __len),(__fd,__offset,__len))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY void (__LIBCCALL Xposix_fadvise64)(__fd_t __fd, __off64_t __offset, __off64_t __len, int __advise);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xposix_fallocate64)(__fd_t __fd, __off64_t __offset, __off64_t __len);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_XOPEN2K */

#ifndef __Xlockf_defined
#define __Xlockf_defined 1
__REDIRECT_FS64_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xlockf,(__fd_t __fd, int __cmd, __FS_TYPE(off) __len),(__fd,__cmd,__len))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY void (__LIBCCALL Xlockf64)(__fd_t __fd, int __cmd, __off64_t __len);
#endif /* __USE_LARGEFILE64 */
#endif /* !__Xlockf_defined */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_FCNTL_H */
