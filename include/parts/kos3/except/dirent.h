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
#ifndef _PARTS_KOS3_EXCEPT_DIRENT_H
#define _PARTS_KOS3_EXCEPT_DIRENT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#ifndef _DIRENT_H
#include <dirent.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __NONNULL((1)) DIR *(__LIBCCALL Xopendir)(char const *__name);
#if defined(__USE_KOS) && defined(__USE_ATFILE)
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __NONNULL((2)) DIR *(__LIBCCALL Xopendirat)(__fd_t __dfd, char const *__name);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __NONNULL((2)) DIR *(__LIBCCALL Xfopendirat)(__fd_t __dfd, char const *__name, __atflag_t __flags);
#endif /* __USE_KOS && __USE_ATFILE */
/* @EXCEPT: Returns NULL for end-of-directory; throws an error if something else went wrong. */
__REDIRECT_FS64(__LIBC,__PORT_KOSONLY __NONNULL((1)),struct dirent *,__LIBCCALL,Xreaddir,(DIR *__restrict __dirp),(__dirp))
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xrewinddir)(DIR *__restrict __dirp);
#ifdef __USE_XOPEN2K8
__LIBC __ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED DIR *(__LIBCCALL Xfdopendir)(__fd_t __fd);
#endif /* __USE_XOPEN2K8 */
#ifdef __USE_LARGEFILE64
/* @EXCEPT: Returns NULL for end-of-directory; throws an error if something else went wrong. */
__LIBC __PORT_KOSONLY __NONNULL((1)) struct dirent64 *(__LIBCCALL Xreaddir64)(DIR *__restrict __dirp);
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_POSIX
#ifdef __USE_FILE_OFFSET64
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2,3)),__LIBCCALL,
                Xreaddir_r,(DIR *__restrict __dirp, struct dirent *__restrict __entry, struct dirent **__restrict __result),
                Xreaddir64_r,(__dirp,__entry,__result))
#else
__LIBC __PORT_KOSONLY __NONNULL((1,2,3))
void (__LIBCCALL Xreaddir_r)(DIR *__restrict __dirp, struct dirent *__restrict __entry,
                             struct dirent **__restrict __result);
#endif
#ifdef __USE_LARGEFILE64
/* NOTE: This ~reentrant~ version of readdir() is strongly discouraged from being used in KOS, as the
 *       kernel does not impose a limit on the length of a single directory entry name (s.a. 'xreaddir')
 * >> Instead, simply use `readdir()'/`readdir64()', which will automatically (re-)allocate an internal,
 *    per-directory buffer of sufficient size to house any directory entry (s.a.: `READDIR_DEFAULT') */
__LIBC __PORT_KOSONLY __NONNULL((1,2,3))
void (__LIBCCALL Xreaddir64_r)(DIR *__restrict __dirp, struct dirent64 *__restrict __entry,
                               struct dirent64 **__restrict __result);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_POSIX */
#if defined(__USE_MISC) || defined(__USE_XOPEN)
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xseekdir)(DIR *__restrict __dirp, long int __pos);
__LIBC __PORT_KOSONLY __NONNULL((1)) unsigned long int (__LIBCCALL Xtelldir)(DIR *__restrict __dirp);
#endif /* __USE_MISC || __USE_XOPEN */

#ifdef __USE_XOPEN2K8
__REDIRECT_FS64(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),
                unsigned int,__LIBCCALL,Xscandir,
               (char const *__restrict __dir, struct dirent ***__restrict __namelist,
                int (*__selector)(struct dirent const *),
                int (*__cmp)(struct dirent const **, struct dirent const **)),
               (__dir,__namelist,__selector,__cmp))
#ifdef __USE_GNU
__REDIRECT_FS64(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),
                unsigned int,__LIBCCALL,Xscandirat,
               (__fd_t __dfd, char const *__restrict __dir, struct dirent ***__restrict __namelist,
                int (*__selector)(struct dirent const *),
                int (*__cmp)(struct dirent const **, struct dirent const **)),
               (__dfd,__dir,__namelist,__selector,__cmp))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY __NONNULL((1,2)) unsigned int (__LIBCCALL Xscandir64)
               (char const *__restrict __dir, struct dirent64 ***__restrict __namelist,
                int (*__selector)(const struct dirent64 *),
                int (*__cmp)(const struct dirent64 **, const struct dirent64 **));
__LIBC __PORT_KOSONLY __NONNULL((2,3)) unsigned int (__LIBCCALL Xscandirat64)
               (__fd_t __dfd, char const *__restrict __dir, struct dirent64 ***__restrict __namelist,
                int (*__selector)(const struct dirent64 *),
                int (*__cmp)(const struct dirent64 **, const struct dirent64 **));
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_GNU */
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_MISC
__REDIRECT_FS64(__LIBC,__PORT_KOSONLY __NONNULL((2,4)),__size_t,__LIBCCALL,Xgetdirentries,
               (__fd_t __fd, char *__restrict __buf, size_t __nbytes, __FS_TYPE(off) *__restrict __basep),
               (__fd,__buf,__nbytes,__basep))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY __NONNULL((2,4)) __size_t (__LIBCCALL Xgetdirentries64)
               (__fd_t __fd, char *__restrict __buf, size_t __nbytes, __off64_t *__restrict __basep);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_MISC */

/* The KOS-specific system call for reading a single directory entry
 * from a file descriptor referring to an open directory stream.
 * @param: MODE: One of `READDIR_*' (See below)
 * @return: * : The actually required buffer size for the directory entry (in bytes)
 *              NOTE: When `READDIR_DEFAULT' was passed for `MODE', the directory
 *                    stream will only be advanced when this value is >= 'BUFSIZE'
 * @return: 0 : The end of the directory has been reached.
 * @EXCPET: Failed to read a directory entry for some reason */
__LIBC __PORT_KOSONLY __NONNULL((2)) __WUNUSED
__size_t (__LIBCCALL Xxreaddir)(__fd_t __fd, struct dirent *__buf, size_t __bufsize, int __mode);
__LIBC __PORT_KOSONLY __NONNULL((2)) __WUNUSED
__size_t (__LIBCCALL Xxreaddirf)(__fd_t __fd, struct dirent *__buf, size_t __bufsize, int __mode, __oflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((2)) __WUNUSED
__size_t (__LIBCCALL Xxreaddir64)(__fd_t __fd, struct dirent64 *__buf, size_t __bufsize, int __mode);
__LIBC __PORT_KOSONLY __NONNULL((2)) __WUNUSED
__size_t (__LIBCCALL Xxreaddirf64)(__fd_t __fd, struct dirent64 *__buf, size_t __bufsize, int __mode, __oflag_t __flags);

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_DIRENT_H */
