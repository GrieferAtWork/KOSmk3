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
#ifndef _PARTS_KOS3_EXCEPT_SYS_WSTAT_H
#define _PARTS_KOS3_EXCEPT_SYS_WSTAT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <bits/stat.h>

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */


#ifdef __USE_FILE_OFFSET64
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xwstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),Xkwstat64,(__file,__buf))
#else
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xwstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),Xkwstat,(__file,__buf))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xwstat64,(wchar_t const *__restrict __file, struct stat64 *__restrict __buf),Xkwstat64,(__file,__buf))
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_ATFILE
#ifdef __USE_FILE_OFFSET64
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xwfstatat,(__fd_t __fd, wchar_t const *__restrict __file, struct stat *__restrict __buf, __atflag_t __flags),Xkwfstatat64,(__fd,__file,__buf,__flags))
#else
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xwfstatat,(__fd_t __fd, wchar_t const *__restrict __file, struct stat *__restrict __buf, __atflag_t __flags),Xkwfstatat,(__fd,__file,__buf,__flags))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xwfstatat64,
               (__fd_t __fd, wchar_t const *__restrict __file, struct stat64 *__restrict __buf, __atflag_t __flags),
                Xkwfstatat64,(__fd,__file,__buf,__flags))
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_ATFILE */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
#ifdef __USE_FILE_OFFSET64
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xwlstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),Xkwlstat,(__file,__buf))
#else
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xwlstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),Xkwlstat,(__file,__buf))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xwlstat64,(wchar_t const *__restrict __file, struct stat64 *__restrict __buf),Xkwlstat64,(__file,__buf))
#endif /* __USE_LARGEFILE64*/
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */

#ifndef __Xwmkdir_defined
#define __Xwmkdir_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwmkdir)(wchar_t const *__path, __mode_t __mode);
#endif /* !__Xwmkdir_defined */

#ifndef __Xwchmod_defined
#define __Xwchmod_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwchmod)(wchar_t const *__file, __mode_t __mode);
#endif /* !__Xwchmod_defined */

#ifdef __USE_MISC
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwlchmod)(wchar_t const *__file, __mode_t __mode);
#endif /* __USE_MISC */

#if defined(__USE_KOS) && defined(__USE_ATFILE)
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xwfmkdirat)(__fd_t __dfd, wchar_t const *__path, __mode_t __mode, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xwfmknodat)(__fd_t __dfd, wchar_t const *__path, __mode_t __mode, __dev_t __dev, __atflag_t __flags);
#endif
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwmkfifo)(wchar_t const *__path, __mode_t __mode);
#ifdef __USE_ATFILE
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xwfchmodat)(__fd_t __dfd, wchar_t const *__file, __mode_t __mode, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xwmkdirat)(__fd_t __dfd, wchar_t const *__path, __mode_t __mode);
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xwmkfifoat)(__fd_t __dfd, wchar_t const *__path, __mode_t __mode);
#endif /* __USE_ATFILE */
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwmknod)(wchar_t const *__path, __mode_t __mode, __dev_t __dev);
#ifdef __USE_ATFILE
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xwmknodat)(__fd_t __dfd, wchar_t const *__path, __mode_t __mode, __dev_t __dev);
#endif /* __USE_ATFILE */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */

#ifdef __USE_ATFILE
__REDIRECT_TM64_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xwutimensat,(__fd_t __dfd, wchar_t const *__path, struct timespec const __times[2], __atflag_t __flags),(__dfd,__path,__times,__flags))
#ifdef __USE_TIME64
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xwutimensat64)(__fd_t __dfd, wchar_t const *__path, struct __timespec64 const __times[2], __atflag_t __flags);
#endif /* __USE_TIME64 */
#endif /* __USE_ATFILE */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SYS_WSTAT_H */
