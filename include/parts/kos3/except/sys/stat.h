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
#ifndef _PARTS_KOS3_EXCEPT_SYS_STAT_H
#define _PARTS_KOS3_EXCEPT_SYS_STAT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#ifndef _SYS_GENERIC_STAT_H
#include <sys/stat.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

#ifdef __USE_FILE_OFFSET64
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xstat,(char const *__restrict __file, struct stat *__restrict __buf),Xkstat64,(__file,__buf))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xfstat,(__fd_t __fd, struct stat *__buf),Xkfstat64,(__fd,__buf))
#else
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xstat,(char const *__restrict __file, struct stat *__restrict __buf),Xkstat,(__file,__buf))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xfstat,(__fd_t __fd, struct stat *__buf),Xkfstat,(__fd,__buf))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xstat64,(char const *__restrict __file, struct stat64 *__restrict __buf),Xkstat64,(__file,__buf))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xfstat64,(__fd_t __fd, struct stat64 *__buf),Xkfstat64,(__fd,__buf))
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_ATFILE
#ifdef __USE_FILE_OFFSET64
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xfstatat,(__fd_t __fd, char const *__restrict __file, struct stat *__restrict __buf, __atflag_t __flags),Xkfstatat64,(__fd,__file,__buf,__flags))
#else
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xfstatat,(__fd_t __fd, char const *__restrict __file, struct stat *__restrict __buf, __atflag_t __flags),Xkfstatat,(__fd,__file,__buf,__flags))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xfstatat64,
               (__fd_t __fd, char const *__restrict __file, struct stat64 *__restrict __buf, __atflag_t __flags),
                Xkfstatat64,(__fd,__file,__buf,__flags))
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_ATFILE */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
#ifdef __USE_FILE_OFFSET64
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xlstat,(char const *__restrict __file, struct stat *__restrict __buf),Xklstat,(__file,__buf))
#else
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xlstat,(char const *__restrict __file, struct stat *__restrict __buf),Xklstat,(__file,__buf))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xlstat64,(char const *__restrict __file, struct stat64 *__restrict __buf),Xklstat64,(__file,__buf))
#endif /* __USE_LARGEFILE64*/
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */

#ifndef __Xmkdir_defined
#define __Xmkdir_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xmkdir)(char const *__path, __mode_t __mode);
#endif /* !__Xmkdir_defined */
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xchmod)(char const *__file, __mode_t __mode);
#ifdef __USE_MISC
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xlchmod)(char const *__file, __mode_t __mode);
#endif /* __USE_MISC */
#if defined(__USE_KOS) && defined(__USE_ATFILE)
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xfmkdirat)(__fd_t __dfd, char const *__path, __mode_t __mode, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xfmknodat)(__fd_t __dfd, char const *__path, __mode_t __mode, __dev_t __dev, __atflag_t __flags);
#endif

__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xmkfifo)(char const *__path, __mode_t __mode);
#ifdef __USE_ATFILE
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xfchmodat)(__fd_t __dfd, char const *__file, __mode_t __mode, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xmkdirat)(__fd_t __dfd, char const *__path, __mode_t __mode);
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xmkfifoat)(__fd_t __dfd, char const *__path, __mode_t __mode);
#endif /* __USE_ATFILE */
#ifdef __USE_POSIX
__LIBC __PORT_KOSONLY void (__LIBCCALL Xfchmod)(__fd_t __fd, __mode_t __mode);
#endif /* __USE_POSIX */
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xmknod)(char const *__path, __mode_t __mode, __dev_t __dev);
#ifdef __USE_ATFILE
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xmknodat)(__fd_t __dfd, char const *__path, __mode_t __mode, __dev_t __dev);
#endif /* __USE_ATFILE */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */

#ifdef __USE_ATFILE
__REDIRECT_TM64_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xutimensat,(__fd_t __dfd, char const *__path, struct timespec const __times[2], __atflag_t __flags),(__dfd,__path,__times,__flags))
#ifdef __USE_TIME64
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xutimensat64)(__fd_t __dfd, char const *__path, struct __timespec64 const __times[2], __atflag_t __flags);
#endif /* __USE_TIME64 */
#endif /* __USE_ATFILE */

#ifdef __USE_XOPEN2K8
__REDIRECT_TM64_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xfutimens,(__fd_t __fd, struct timespec const __times[2]),(__fd,__times))
#ifdef __USE_TIME64
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xfutimens64)(__fd_t __fd, struct __timespec64 const __times[2]);
#endif /* __USE_TIME64 */
#endif /* __USE_XOPEN2K8 */


__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SYS_STAT_H */
