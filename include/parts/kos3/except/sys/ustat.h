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
#ifndef _PARTS_KOS3_EXCEPT_SYS_USTAT_H
#define _PARTS_KOS3_EXCEPT_SYS_USTAT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <bits/stat.h>

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

#ifdef __USE_FILE_OFFSET64
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw16stat,(char16_t const *__restrict __file, struct stat *__restrict __buf),Xkwstat64,(__file,__buf))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw32stat,(char32_t const *__restrict __file, struct stat *__restrict __buf),Xkwstat64,(__file,__buf))
#else
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw16stat,(char16_t const *__restrict __file, struct stat *__restrict __buf),Xkwstat,(__file,__buf))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw32stat,(char32_t const *__restrict __file, struct stat *__restrict __buf),Xkwstat,(__file,__buf))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw16stat64,(char16_t const *__restrict __file, struct stat64 *__restrict __buf),Xkwstat64,(__file,__buf))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw32stat64,(char32_t const *__restrict __file, struct stat64 *__restrict __buf),Xkwstat64,(__file,__buf))
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_ATFILE
#ifdef __USE_FILE_OFFSET64
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xw16fstatat,(__fd_t __fd, char16_t const *__restrict __file, struct stat *__restrict __buf, __atflag_t __flags),Xkwfstatat64,(__fd,__file,__buf,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xw32fstatat,(__fd_t __fd, char32_t const *__restrict __file, struct stat *__restrict __buf, __atflag_t __flags),Xkwfstatat64,(__fd,__file,__buf,__flags))
#else
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xw16fstatat,(__fd_t __fd, char16_t const *__restrict __file, struct stat *__restrict __buf, __atflag_t __flags),Xkwfstatat,(__fd,__file,__buf,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xw32fstatat,(__fd_t __fd, char32_t const *__restrict __file, struct stat *__restrict __buf, __atflag_t __flags),Xkwfstatat,(__fd,__file,__buf,__flags))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xw16fstatat64,
                   (__fd_t __fd, char16_t const *__restrict __file, struct stat64 *__restrict __buf, __atflag_t __flags),
                    Xkwfstatat64,(__fd,__file,__buf,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xw32fstatat64,
                   (__fd_t __fd, char32_t const *__restrict __file, struct stat64 *__restrict __buf, __atflag_t __flags),
                    Xkwfstatat64,(__fd,__file,__buf,__flags))
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_ATFILE */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
#ifdef __USE_FILE_OFFSET64
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw16lstat,(char16_t const *__restrict __file, struct stat *__restrict __buf),Xkwlstat,(__file,__buf))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw32lstat,(char32_t const *__restrict __file, struct stat *__restrict __buf),Xkwlstat,(__file,__buf))
#else
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw16lstat,(char16_t const *__restrict __file, struct stat *__restrict __buf),Xkwlstat,(__file,__buf))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw32lstat,(char32_t const *__restrict __file, struct stat *__restrict __buf),Xkwlstat,(__file,__buf))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw16lstat64,(char16_t const *__restrict __file, struct stat64 *__restrict __buf),Xkwlstat64,(__file,__buf))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw32lstat64,(char32_t const *__restrict __file, struct stat64 *__restrict __buf),Xkwlstat64,(__file,__buf))
#endif /* __USE_LARGEFILE64*/
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */

#ifndef __Xw16mkdir_defined
#define __Xw16mkdir_defined 1
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16mkdir,(char16_t const *__path, __mode_t __mode),Xwmkdir,(__path,__mode))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32mkdir,(char32_t const *__path, __mode_t __mode),Xwmkdir,(__path,__mode))
#endif /* !__Xw16mkdir_defined */

#ifndef __Xw16chmod_defined
#define __Xw16chmod_defined 1
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16chmod,(char16_t const *__file, __mode_t __mode),Xwchmod,(__file,__mode))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32chmod,(char32_t const *__file, __mode_t __mode),Xwchmod,(__file,__mode))
#endif /* !__Xw16chmod_defined */

#ifdef __USE_MISC
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16lchmod,(char16_t const *__file, __mode_t __mode),Xwlchmod,(__file,__mode))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32lchmod,(char32_t const *__file, __mode_t __mode),Xwlchmod,(__file,__mode))
#endif /* __USE_MISC */

#if defined(__USE_KOS) && defined(__USE_ATFILE)
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16fmkdirat,(__fd_t __dfd, char16_t const *__path, __mode_t __mode, __atflag_t __flags),Xwfmkdirat,(__dfd,__path,__mode,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32fmkdirat,(__fd_t __dfd, char32_t const *__path, __mode_t __mode, __atflag_t __flags),Xwfmkdirat,(__dfd,__path,__mode,__flags))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16fmknodat,(__fd_t __dfd, char16_t const *__path, __mode_t __mode, __dev_t __dev, __atflag_t __flags),Xwfmknodat,(__dfd,__path,__mode,__dev,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32fmknodat,(__fd_t __dfd, char32_t const *__path, __mode_t __mode, __dev_t __dev, __atflag_t __flags),Xwfmknodat,(__dfd,__path,__mode,__dev,__flags))
#endif
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16mkfifo,(char16_t const *__path, __mode_t __mode),Xwmkfifo,(__path,__mode))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32mkfifo,(char32_t const *__path, __mode_t __mode),Xwmkfifo,(__path,__mode))
#ifdef __USE_ATFILE
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16fchmodat,(__fd_t __dfd, char16_t const *__file, __mode_t __mode, __atflag_t __flags),Xwfchmodat,(__dfd,__file,__mode,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32fchmodat,(__fd_t __dfd, char32_t const *__file, __mode_t __mode, __atflag_t __flags),Xwfchmodat,(__dfd,__file,__mode,__flags))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16mkdirat,(__fd_t __dfd, char16_t const *__path, __mode_t __mode),Xwmkdirat,(__dfd,__path,__mode))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32mkdirat,(__fd_t __dfd, char32_t const *__path, __mode_t __mode),Xwmkdirat,(__dfd,__path,__mode))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16mkfifoat,(__fd_t __dfd, char16_t const *__path, __mode_t __mode),Xwmkfifoat,(__dfd,__path,__mode))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32mkfifoat,(__fd_t __dfd, char32_t const *__path, __mode_t __mode),Xwmkfifoat,(__dfd,__path,__mode))
#endif /* __USE_ATFILE */
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16mknod,(char16_t const *__path, __mode_t __mode, __dev_t __dev),Xwmknod,(__path,__mode,__dev))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32mknod,(char32_t const *__path, __mode_t __mode, __dev_t __dev),Xwmknod,(__path,__mode,__dev))
#ifdef __USE_ATFILE
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16mknodat,(__fd_t __dfd, char16_t const *__path, __mode_t __mode, __dev_t __dev),Xwmknodat,(__dfd,__path,__mode,__dev))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32mknodat,(__fd_t __dfd, char32_t const *__path, __mode_t __mode, __dev_t __dev),Xwmknodat,(__dfd,__path,__mode,__dev))
#endif /* __USE_ATFILE */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */

#ifdef __USE_ATFILE
#ifdef __USE_TIME_BITS64
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16utimensat,(__fd_t __dfd, char16_t const *__path, struct timespec const __times[2], __atflag_t __flags),Xwutimensat64,(__dfd,__path,__times,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32utimensat,(__fd_t __dfd, char32_t const *__path, struct timespec const __times[2], __atflag_t __flags),Xwutimensat64,(__dfd,__path,__times,__flags))
#else
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16utimensat,(__fd_t __dfd, char16_t const *__path, struct timespec const __times[2], __atflag_t __flags),Xwutimensat,(__dfd,__path,__times,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32utimensat,(__fd_t __dfd, char32_t const *__path, struct timespec const __times[2], __atflag_t __flags),Xwutimensat,(__dfd,__path,__times,__flags))
#endif
#ifdef __USE_TIME64
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16utimensat64,(__fd_t __dfd, char16_t const *__path, struct __timespec64 const __times[2], __atflag_t __flags),Xwutimensat64,(__dfd,__path,__times,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32utimensat64,(__fd_t __dfd, char32_t const *__path, struct __timespec64 const __times[2], __atflag_t __flags),Xwutimensat64,(__dfd,__path,__times,__flags))
#endif /* __USE_TIME64 */
#endif /* __USE_ATFILE */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SYS_USTAT_H */
