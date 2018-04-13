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
#ifndef _PARTS_KOS3_SYS_WSTAT_H
#define _PARTS_KOS3_SYS_WSTAT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <bits/stat.h>

__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef __KERNEL__

/* Recognized & known stat() structures and assembly names.
 * 
 * __CRT_KOS: (Used by default)
 * >> int CDECL kwstat(char32_t const *kos_file, struct __kos_stat *buf);
 * >> int CDECL kwstat64(char32_t const *kos_file, struct __kos_stat64 *buf);
 * >> int CDECL kwfstatat(fd_t fd, char32_t const *kos_file, struct __kos_stat *buf, int flag);
 * >> int CDECL kwfstatat64(fd_t fd, char32_t const *kos_file, struct __kos_stat64 *buf, int flag);
 * >> int CDECL kwlstat(char32_t const *kos_file, struct __kos_stat *buf);
 * >> int CDECL kwlstat64(char32_t const *kos_file, struct __kos_stat64 *buf);
 * >> int CDECL ukwstat(char16_t const *kos_file, struct __kos_stat *buf);
 * >> int CDECL ukwstat64(char16_t const *kos_file, struct __kos_stat64 *buf);
 * >> int CDECL ukwfstatat(fd_t fd, char16_t const *kos_file, struct __kos_stat *buf, int flag);
 * >> int CDECL ukwfstatat64(fd_t fd, char16_t const *kos_file, struct __kos_stat64 *buf, int flag);
 * >> int CDECL ukwlstat(char16_t const *kos_file, struct __kos_stat *buf);
 * >> int CDECL ukwlstat64(char16_t const *kos_file, struct __kos_stat64 *buf);
 * >> int CDECL DOS$kwstat(char16_t const *dos_file, struct __kos_stat *buf);
 * >> int CDECL DOS$kwstat64(char16_t const *dos_file, struct __kos_stat64 *buf);
 * >> int CDECL DOS$kwfstatat(fd_t fd, char16_t const *dos_file, struct __kos_stat *buf, int flag);
 * >> int CDECL DOS$kwfstatat64(fd_t fd, char16_t const *dos_file, struct __kos_stat64 *buf, int flag);
 * >> int CDECL DOS$kwlstat(char16_t const *dos_file, struct __kos_stat *buf);
 * >> int CDECL DOS$kwlstat64(char16_t const *dos_file, struct __kos_stat64 *buf);
 * >> int CDECL DOS$Ukwstat(char32_t const *dos_file, struct __kos_stat *buf);
 * >> int CDECL DOS$Ukwstat64(char32_t const *dos_file, struct __kos_stat64 *buf);
 * >> int CDECL DOS$Ukwfstatat(fd_t fd, char32_t const *dos_file, struct __kos_stat *buf, int flag);
 * >> int CDECL DOS$Ukwfstatat64(fd_t fd, char32_t const *dos_file, struct __kos_stat64 *buf, int flag);
 * >> int CDECL DOS$Ukwlstat(char32_t const *dos_file, struct __kos_stat *buf);
 * >> int CDECL DOS$Ukwlstat64(char32_t const *dos_file, struct __kos_stat64 *buf);
 *
 * __CRT_DOS: (__DOS_COMPAT__)
 * >> int CDECL [OLD: DOS$_wstat,     NEW:DOS$_wstat32] (char16_t const *dos_file, struct __dos_stat32 *buf);
 * >> int CDECL [OLD: DOS$_wstati64,  NEW:DOS$_wstat32i64] (char16_t const *dos_file, struct __dos_stat32i64 *buf);
 * >> int CDECL DOS$_wstat64(char16_t const *dos_file, struct __dos_stat64 *buf);
 * >> int CDECL [NEW: DOS$_wstat64i32](char16_t const *dos_file, struct __dos_stat64i32 *buf) = DOS$_wstat64;
 * >> int CDECL [OLD: DOS$Uwstat,     NEW:DOS$Uwstat32] (char32_t const *dos_file, struct __dos_stat32 *buf);
 * >> int CDECL [OLD: DOS$Uwstati64,  NEW:DOS$Uwstat32i64] (char32_t const *dos_file, struct __dos_stat32i64 *buf);
 * >> int CDECL DOS$Uwstat64(char32_t const *dos_file, struct __dos_stat64 *buf);
 * >> int CDECL [NEW: DOS$Uwstat64i32](char32_t const *dos_file, struct __dos_stat64i32 *buf) = DOS$Uwstat64;
 */

#undef stat
#undef stat64
#undef lstat64
#ifdef __DOS_COMPAT__
/* `struct stat' is:
 *    [                                       ] = struct __dos_stat32
 *    [__USE_TIME_BITS64                      ] = struct __dos_stat64i32 == struct __dos_stat64
 *    [__USE_FILE_OFFSET64                    ] = struct __dos_stat32i64
 *    [__USE_TIME_BITS64 + __USE_FILE_OFFSET64] = struct __dos_stat64
 * `struct stat64' is:
 *    [                 ] = struct __dos_stat32i64
 *    [__USE_TIME_BITS64] = struct __dos_stat64 */
#ifdef __USE_TIME_BITS64
#undef _wstat64
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),_wstat64,(__file,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wlstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),_wstat64,(__file,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#ifdef __USE_LARGEFILE64
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wstat64,(wchar_t const *__restrict __file, struct stat64 *__restrict __buf),_wstat64,(__file,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wlstat64,(wchar_t const *__restrict __file, struct stat64 *__restrict __buf),_wstat64,(__file,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#endif /* __USE_LARGEFILE64 */
#else /* __USE_TIME_BITS64 */
#undef _wstat32
#undef _wstat32i64
#ifdef __USE_DOS_LINKOLDFINDSTAT
#   undef _wstat
#   undef _wstati64
#endif /* __USE_DOS_LINKOLDFINDSTAT */
#ifdef __USE_FILE_OFFSET64
#ifdef __USE_DOS_LINKOLDFINDSTAT
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),_wstati64,(__file,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wlstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),_wstati64,(__file,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#else /* __USE_DOS_LINKOLDFINDSTAT */
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),_wstat32i64,(__file,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wlstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),_wstat32i64,(__file,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#endif /* !__USE_DOS_LINKOLDFINDSTAT */
#else /* __USE_FILE_OFFSET64 */
#ifdef __USE_DOS_LINKOLDFINDSTAT
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),_wstat,(__file,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wlstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),_wstat,(__file,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#else /* __USE_DOS_LINKOLDFINDSTAT */
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),_wstat32,(__file,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wlstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),_wstat32,(__file,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#endif /* !__USE_DOS_LINKOLDFINDSTAT */
#endif /* !__USE_FILE_OFFSET64 */
#ifdef __USE_LARGEFILE64
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wstat64,(wchar_t const *__restrict __file, struct stat64 *__restrict __buf),_wstat32i64,(__fd,__buf))
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT(__LIBC,__WARN_NOKOSFS __NONNULL((1,2)),int,__LIBCCALL,wlstat64,(wchar_t const *__restrict __file, struct stat64 *__restrict __buf),_wstat32i64,(__fd,__buf))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#endif /* __USE_LARGEFILE64 */
#endif /* !__USE_TIME_BITS64 */

#else /* Compat... */

#ifdef __USE_FILE_OFFSET64
__REDIRECT_EXCEPT_UFS_XVOID_(__LIBC,__NONNULL((1,2)),int,__LIBCCALL,wstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),kwstat64,(__file,__buf))
#else
__REDIRECT_EXCEPT_UFS_XVOID_(__LIBC,__NONNULL((1,2)),int,__LIBCCALL,wstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),kwstat,(__file,__buf))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_UFS_XVOID_(__LIBC,__NONNULL((1,2)),int,__LIBCCALL,wstat64,(wchar_t const *__restrict __file, struct stat64 *__restrict __buf),kwstat64,(__file,__buf))
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_ATFILE
#ifdef __USE_FILE_OFFSET64
__REDIRECT_EXCEPT_UFS_XVOID_(__LIBC,__PORT_NODOS_ALT(wstat) __NONNULL((2,3)),int,__LIBCCALL,wfstatat,(__fd_t __fd, wchar_t const *__restrict __file, struct stat *__restrict __buf, int __flags),kwfstatat64,(__fd,__file,__buf,__flags))
#else
__REDIRECT_EXCEPT_UFS_XVOID_(__LIBC,__PORT_NODOS_ALT(wstat) __NONNULL((2,3)),int,__LIBCCALL,wfstatat,(__fd_t __fd, wchar_t const *__restrict __file, struct stat *__restrict __buf, int __flags),kwfstatat,(__fd,__file,__buf,__flags))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_UFS_XVOID_(__LIBC,__PORT_NODOS_ALT(wstat) __NONNULL((2,3)),int,__LIBCCALL,wfstatat64,
                            (__fd_t __fd, wchar_t const *__restrict __file, struct stat64 *__restrict __buf, int __flags),
                             kwfstatat64,(__fd,__file,__buf,__flags))
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_ATFILE */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
#ifdef __USE_FILE_OFFSET64
__REDIRECT_EXCEPT_UFS_XVOID_(__LIBC,__NONNULL((1,2)),int,__LIBCCALL,wlstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),kwlstat,(__file,__buf))
#else
__REDIRECT_EXCEPT_UFS_XVOID_(__LIBC,__NONNULL((1,2)),int,__LIBCCALL,wlstat,(wchar_t const *__restrict __file, struct stat *__restrict __buf),kwlstat,(__file,__buf))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_UFS_XVOID_(__LIBC,__NONNULL((1,2)),int,__LIBCCALL,wlstat64,(wchar_t const *__restrict __file, struct stat64 *__restrict __buf),kwlstat64,(__file,__buf))
#endif /* __USE_LARGEFILE64*/
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#endif /* Builtin... */


#ifndef __wmkdir_defined
#define __wmkdir_defined 1
#ifndef __DOS_COMPAT__
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,wmkdir,(wchar_t const *__path, __mode_t __mode),(__path,__mode))
#else
__REDIRECT_UFS_(__LIBC,__NONNULL((1)),int,__LIBCCALL,__dos_wmkdir,(wchar_t const *__path),_wmkdir,(__path))
__LOCAL __WARN_NOKOSFS __NONNULL((1)) int (__LIBCCALL wmkdir)(wchar_t const *__path, __mode_t __UNUSED(__mode)) { return __dos_wmkdir(__path); }
#endif /* !__USE_DOSFS */
#endif /* !__mkdir_defined */

#ifndef __wchmod_defined
#define __wchmod_defined 1
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,wchmod,(wchar_t const *__file, int __mode),(__file,__mode))
#endif /* !__wchmod_defined */

#ifdef __USE_MISC
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,wlchmod,(wchar_t const *__file, __mode_t __mode),_wchmod,(__file,__mode))
#else /* __DOS_COMPAT__ */
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,wlchmod,(wchar_t const *__file, __mode_t __mode),(__file,__mode))
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_MISC */

#ifdef __CRT_KOS
#if defined(__USE_KOS) && defined(__USE_ATFILE)
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,wfmkdirat,(__fd_t __dfd, wchar_t const *__path, __mode_t __mode, int __flags),(__dfd,__path,__mode,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,wfmknodat,(__fd_t __dfd, wchar_t const *__path, __mode_t __mode, __dev_t __dev, int __flags),(__dfd,__path,__mode,__dev,__flags))
#endif

__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,wmkfifo,(wchar_t const *__path, __mode_t __mode),(__path,__mode))
#ifdef __USE_ATFILE
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(wchmod) __NONNULL((2)),int,__LIBCCALL,wfchmodat,(__fd_t __dfd, wchar_t const *__file, __mode_t __mode, int __flags),(__dfd,__file,__mode,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(wmkdir) __NONNULL((2)),int,__LIBCCALL,wmkdirat,(__fd_t __dfd, wchar_t const *__path, __mode_t __mode),(__dfd,__path,__mode))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,wmkfifoat,(__fd_t __dfd, wchar_t const *__path, __mode_t __mode),(__dfd,__path,__mode))
#endif /* __USE_ATFILE */
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,wmknod,(wchar_t const *__path, __mode_t __mode, __dev_t __dev),(__path,__mode,__dev))
#ifdef __USE_ATFILE
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,wmknodat,(__fd_t __dfd, wchar_t const *__path, __mode_t __mode, __dev_t __dev),(__dfd,__path,__mode,__dev))
#endif /* __USE_ATFILE */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */

#ifdef __USE_ATFILE
__REDIRECT_EXCEPT_UFSTM64_XVOID(__LIBC,__PORT_KOSONLY_ALT(wutime) __NONNULL((2)),int,__LIBCCALL,wutimensat,(__fd_t __dfd, wchar_t const *__path, struct timespec const __times[2], int __flags),(__dfd,__path,__times,__flags))
#ifdef __USE_TIME64
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(wutime64) __NONNULL((2)),int,__LIBCCALL,wutimensat64,(__fd_t __dfd, wchar_t const *__path, struct __timespec64 const __times[2], int __flags),(__dfd,__path,__times,__flags))
#endif /* __USE_TIME64 */
#endif /* __USE_ATFILE */
#endif /* __CRT_KOS */
#endif /* !__KERNEL__ */

__SYSDECL_END

#ifdef __USE_EXCEPT
#include <parts/kos3/except/sys/wstat.h>
#endif /* __USE_EXCEPT */

#endif /* !_PARTS_KOS3_SYS_WSTAT_H */
