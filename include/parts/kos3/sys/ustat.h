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
#ifndef _PARTS_KOS3_SYS_USTAT_H
#define _PARTS_KOS3_SYS_USTAT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <bits/stat.h>

__SYSDECL_BEGIN

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

#ifndef __KERNEL__
#ifdef __CRT_KOS


#undef stat
#undef stat64
#undef lstat64
#ifdef __USE_FILE_OFFSET64
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w16stat,(char16_t const *__restrict __file, struct stat *__restrict __buf),kwstat64,(__file,__buf))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w32stat,(char32_t const *__restrict __file, struct stat *__restrict __buf),kwstat64,(__file,__buf))
#else
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w16stat,(char16_t const *__restrict __file, struct stat *__restrict __buf),kwstat,(__file,__buf))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w32stat,(char32_t const *__restrict __file, struct stat *__restrict __buf),kwstat,(__file,__buf))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w16stat64,(char16_t const *__restrict __file, struct stat64 *__restrict __buf),kwstat64,(__file,__buf))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w32stat64,(char32_t const *__restrict __file, struct stat64 *__restrict __buf),kwstat64,(__file,__buf))
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_ATFILE
#ifdef __USE_FILE_OFFSET64
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),int,__LIBCCALL,w16fstatat,(__fd_t __fd, char16_t const *__restrict __file, struct stat *__restrict __buf, int __flags),kwfstatat64,(__fd,__file,__buf,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),int,__LIBCCALL,w32fstatat,(__fd_t __fd, char32_t const *__restrict __file, struct stat *__restrict __buf, int __flags),kwfstatat64,(__fd,__file,__buf,__flags))
#else
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),int,__LIBCCALL,w16fstatat,(__fd_t __fd, char16_t const *__restrict __file, struct stat *__restrict __buf, int __flags),kwfstatat,(__fd,__file,__buf,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),int,__LIBCCALL,w32fstatat,(__fd_t __fd, char32_t const *__restrict __file, struct stat *__restrict __buf, int __flags),kwfstatat,(__fd,__file,__buf,__flags))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),int,__LIBCCALL,w16fstatat64,
                              (__fd_t __fd, char16_t const *__restrict __file, struct stat64 *__restrict __buf, int __flags),
                               kwfstatat64,(__fd,__file,__buf,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),int,__LIBCCALL,w32fstatat64,
                              (__fd_t __fd, char32_t const *__restrict __file, struct stat64 *__restrict __buf, int __flags),
                               kwfstatat64,(__fd,__file,__buf,__flags))
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_ATFILE */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
#ifdef __USE_FILE_OFFSET64
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w16lstat,(char16_t const *__restrict __file, struct stat *__restrict __buf),kwlstat,(__file,__buf))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w32lstat,(char32_t const *__restrict __file, struct stat *__restrict __buf),kwlstat,(__file,__buf))
#else
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w16lstat,(char16_t const *__restrict __file, struct stat *__restrict __buf),kwlstat,(__file,__buf))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w32lstat,(char32_t const *__restrict __file, struct stat *__restrict __buf),kwlstat,(__file,__buf))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w16lstat64,(char16_t const *__restrict __file, struct stat64 *__restrict __buf),kwlstat64,(__file,__buf))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w32lstat64,(char32_t const *__restrict __file, struct stat64 *__restrict __buf),kwlstat64,(__file,__buf))
#endif /* __USE_LARGEFILE64*/
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */

#ifndef __w16mkdir_defined
#define __w16mkdir_defined 1
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16mkdir,(char16_t const *__path, __mode_t __mode),wmkdir,(__path,__mode))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32mkdir,(char32_t const *__path, __mode_t __mode),wmkdir,(__path,__mode))
#endif /* !__m16kdir_defined */

#ifndef __wchmod_defined
#define __wchmod_defined 1
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,w16chmod,(char16_t const *__file, int __mode),wchmod,(__file,__mode))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__NONNULL((1)),int,__LIBCCALL,w32chmod,(char32_t const *__file, int __mode),wchmod,(__file,__mode))
#endif /* !__wchmod_defined */

#ifdef __USE_MISC
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,w16lchmod,(char16_t const *__file, __mode_t __mode),wlchmod,(__file,__mode))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,w32lchmod,(char32_t const *__file, __mode_t __mode),wlchmod,(__file,__mode))
#endif /* __USE_MISC */

#if defined(__USE_KOS) && defined(__USE_ATFILE)
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16fmkdirat,(__fd_t __dfd, char16_t const *__path, __mode_t __mode, int __flags),wfmkdirat,(__dfd,__path,__mode,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32fmkdirat,(__fd_t __dfd, char32_t const *__path, __mode_t __mode, int __flags),wfmkdirat,(__dfd,__path,__mode,__flags))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16fmknodat,(__fd_t __dfd, char16_t const *__path, __mode_t __mode, __dev_t __dev, int __flags),wfmknodat,(__dfd,__path,__mode,__dev,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32fmknodat,(__fd_t __dfd, char32_t const *__path, __mode_t __mode, __dev_t __dev, int __flags),wfmknodat,(__dfd,__path,__mode,__dev,__flags))
#endif

__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16mkfifo,(char16_t const *__path, __mode_t __mode),wmkfifo,(__path,__mode))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32mkfifo,(char32_t const *__path, __mode_t __mode),wmkfifo,(__path,__mode))
#ifdef __USE_ATFILE
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16fchmodat,(__fd_t __dfd, char16_t const *__file, __mode_t __mode, int __flags),wfchmodat,(__dfd,__file,__mode,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32fchmodat,(__fd_t __dfd, char32_t const *__file, __mode_t __mode, int __flags),wfchmodat,(__dfd,__file,__mode,__flags))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16mkdirat,(__fd_t __dfd, char16_t const *__path, __mode_t __mode),wmkdirat,(__dfd,__path,__mode))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32mkdirat,(__fd_t __dfd, char32_t const *__path, __mode_t __mode),wmkdirat,(__dfd,__path,__mode))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16mkfifoat,(__fd_t __dfd, char16_t const *__path, __mode_t __mode),wmkfifoat,(__dfd,__path,__mode))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32mkfifoat,(__fd_t __dfd, char32_t const *__path, __mode_t __mode),wmkfifoat,(__dfd,__path,__mode))
#endif /* __USE_ATFILE */
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16mknod,(char16_t const *__path, __mode_t __mode, __dev_t __dev),wmknod,(__path,__mode,__dev))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32mknod,(char32_t const *__path, __mode_t __mode, __dev_t __dev),wmknod,(__path,__mode,__dev))
#ifdef __USE_ATFILE
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16mknodat,(__fd_t __dfd, char16_t const *__path, __mode_t __mode, __dev_t __dev),wmknodat,(__dfd,__path,__mode,__dev))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32mknodat,(__fd_t __dfd, char32_t const *__path, __mode_t __mode, __dev_t __dev),wmknodat,(__dfd,__path,__mode,__dev))
#endif /* __USE_ATFILE */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */

#ifdef __USE_ATFILE
#ifdef __USE_TIME_BITS64
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16utimensat,(__fd_t __dfd, char16_t const *__path, struct timespec const __times[2], int __flags),wutimensat64,(__dfd,__path,__times,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32utimensat,(__fd_t __dfd, char32_t const *__path, struct timespec const __times[2], int __flags),wutimensat64,(__dfd,__path,__times,__flags))
#else
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16utimensat,(__fd_t __dfd, char16_t const *__path, struct timespec const __times[2], int __flags),wutimensat,(__dfd,__path,__times,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32utimensat,(__fd_t __dfd, char32_t const *__path, struct timespec const __times[2], int __flags),wutimensat,(__dfd,__path,__times,__flags))
#endif
#ifdef __USE_TIME64
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16utimensat64,(__fd_t __dfd, char16_t const *__path, struct __timespec64 const __times[2], int __flags),wutimensat64,(__dfd,__path,__times,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32utimensat64,(__fd_t __dfd, char32_t const *__path, struct __timespec64 const __times[2], int __flags),wutimensat64,(__dfd,__path,__times,__flags))
#endif /* __USE_TIME64 */
#endif /* __USE_ATFILE */

#endif /* __CRT_KOS */
#endif /* !__KERNEL__ */

__SYSDECL_END

#ifdef __USE_EXCEPT
#include <parts/kos3/except/sys/ustat.h>
#endif /* __USE_EXCEPT */

#endif /* !_PARTS_KOS3_SYS_USTAT_H */
