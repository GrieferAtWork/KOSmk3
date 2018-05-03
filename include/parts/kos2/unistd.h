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
#ifndef _PARTS_KOS2_UNISTD_H
#define _PARTS_KOS2_UNISTD_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>
#include <hybrid/limits.h>

__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#ifndef __ssize_t_defined
#define __ssize_t_defined 1
typedef __SSIZE_TYPE__ ssize_t;
#endif /* !__ssize_t_defined */

#ifndef __TARGV
#ifdef __USE_DOS
#   define __TARGV  char const *const *___argv
#   define __TENVP  char const *const *___envp
#else
#   define __TARGV  char *const ___argv[__restrict_arr]
#   define __TENVP  char *const ___envp[__restrict_arr]
#endif
#endif /* !__TARGV */

#ifdef __CRT_GLC
#ifndef __fexecve_defined
#define __fexecve_defined 1
__REDIRECT_EXCEPT_XVOID(__LIBC,__XATTR_NORETURN __PORT_NODOS_ALT(execve) __NONNULL((2,3)),int,__LIBCCALL,fexecve,(__fd_t __fd, __TARGV, __TENVP),(__fd,___argv,___envp))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((2,3)) void (__LIBCCALL Xfexecve)(__fd_t __fd, __TARGV, __TENVP);
#endif /* __USE_EXCEPT */
#endif /* !__fexecve_defined */
#ifndef __fexecv_defined
#define __fexecv_defined 1
#ifdef __GLC_COMPAT__
/* When linking against GLibc, we can redirect these against 'fexecve' */
__LOCAL __PORT_NODOS_ALT(execv) __NONNULL((2)) int (__LIBCCALL fexecv)(__fd_t __fd, __TARGV) { return fexecve(__fd,___argv,environ); }
__LOCAL __PORT_NODOS_ALT(execl) __ATTR_SENTINEL int (__ATTR_CDECL fexecl)(__fd_t __fd, char const *__args, ...) __REDIRECT_EXECL(char,fexecv,__fd,__args)
__LOCAL __PORT_NODOS_ALT(execle) __ATTR_SENTINEL int (__ATTR_CDECL fexecle)(__fd_t __fd, char const *__args, ...) __REDIRECT_EXECLE(char,fexecve,__fd,__args)
#else
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS_ALT(execv) __XATTR_NORETURN __NONNULL((2)),int,__LIBCCALL,fexecv,(__fd_t __fd, __TARGV),(__fd,___argv))
__XREDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS_ALT(execl) __XATTR_NORETURN __ATTR_SENTINEL,int,__ATTR_CDECL,fexecl,(__fd_t __fd, char const *__args, ...),__REDIRECT_EXECL(char,fexecv,__fd,__args))
__XREDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS_ALT(execle) __XATTR_NORETURN __ATTR_SENTINEL,int,__ATTR_CDECL,fexecle,(__fd_t __fd, char const *__args, ...),__REDIRECT_EXECLE(char,fexecve,__fd,__args))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((2)) void (__LIBCCALL Xfexecv)(__fd_t __fd, __TARGV);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __ATTR_SENTINEL void (__ATTR_CDECL Xfexecl)(__fd_t __fd, char const *__args, ...);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __ATTR_SENTINEL void (__ATTR_CDECL Xfexecle)(__fd_t __fd, char const *__args, ...);
#endif /* __USE_EXCEPT */
#endif
#endif /* !__fexecv_defined */
#endif /* __CRT_GLC */

#ifdef __CRT_KOS
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(chdir) __NONNULL((2)),int,__LIBCCALL,fchdirat,(__fd_t __dfd, char const *__path, __atflag_t __flags),(__dfd,__path,__flags))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(readlinkat) __NONNULL((2,3)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,freadlinkat,(__fd_t __dfd, char const *__restrict __path, char *__restrict __buf, size_t __buflen, __atflag_t __flags),(__dfd,__path,__buf,__buflen,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(symlinkat) __NONNULL((1,3)),int,__LIBCCALL,fsymlinkat,(char const *__from, __fd_t __tofd, char const *__to, __atflag_t __flags),(__from,__tofd,__to,__flags))
__REDIRECT_EXCEPT_UFS64_XVOID(__LIBC,__PORT_KOSONLY_ALT(truncate) __NONNULL((2)),int,__LIBCCALL,ftruncateat,(__fd_t __dfd, char const *__file, __FS_TYPE(off) __length, __atflag_t __flags),(__dfd,__file,__length,__flags))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY_ALT(readlink) __NONNULL((2)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,freadlink,(__fd_t __fd, char *__restrict __buf, size_t __buflen),(__fd,__buf,__buflen))
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(truncate64) __NONNULL((2)),int,__LIBCCALL,ftruncateat64,(__fd_t __dfd, char const *__file, __off64_t __length, __atflag_t __flags),(__dfd,__file,__length,__flags))
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xfchdirat)(__fd_t __dfd, char const *__path, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((2,3)) size_t (__LIBCCALL Xfreadlinkat)(__fd_t __dfd, char const *__restrict __path, char *__restrict __buf, size_t __buflen, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((1,3)) void (__LIBCCALL Xfsymlinkat)(char const *__from, __fd_t __tofd, char const *__to, __atflag_t __flags);
__REDIRECT_FS64_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xftruncateat,(__fd_t __dfd, char const *__file, __FS_TYPE(off) __length, __atflag_t __flags),(__dfd,__file,__length,__flags))
__LIBC __PORT_KOSONLY __NONNULL((2)) size_t (__LIBCCALL Xfreadlink)(__fd_t __fd, char *__restrict __buf, size_t __buflen);
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xftruncateat64)(__fd_t __dfd, char const *__file, __off64_t __length, __atflag_t __flags);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_EXCEPT */


#ifdef __USE_ATFILE
/* At-file style exec functions. */
#ifndef __fexecvat_defined
#define __fexecvat_defined 1
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execv) __XATTR_NORETURN __NONNULL((3)),int,__LIBCCALL,fexecvat,(__fd_t __dfd, char const *__path, __TARGV, __atflag_t __flags),(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execve) __XATTR_NORETURN __NONNULL((3)),int,__LIBCCALL,fexecveat,(__fd_t __dfd, char const *__path, __TARGV, __TENVP, __atflag_t __flags),(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execvp) __XATTR_NORETURN __NONNULL((1,2)),int,__LIBCCALL,fexecvpat,(char const *__restrict __file, __TARGV, __atflag_t __flags),(__dfd,__file,___argv,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execvpe) __XATTR_NORETURN __NONNULL((1,2,3)),int,__LIBCCALL,fexecvpeat,(char const *__restrict __file, __TARGV, __TENVP, __atflag_t __flags),(__dfd,__file,___argv,__flags))
__XREDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execl) __XATTR_NORETURN __NONNULL((2)) __ATTR_SENTINEL_O(1),int,__ATTR_CDECL,fexeclat,(__fd_t __dfd, char const *__path, char const *__args, ... /*, __atflag_t __flags*/),__REDIRECT_FEXECLAT(char,fexecvat,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execle) __XATTR_NORETURN __NONNULL((2)) __ATTR_SENTINEL_O(2),int,__ATTR_CDECL,fexecleat,(__fd_t __dfd, char const *__path, char const *__args, ... /*, char *const ___envp[], __atflag_t __flags*/),__REDIRECT_FEXECLEAT(char,fexecveat,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execlp) __XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(1),int,__ATTR_CDECL,fexeclpat,(char const *__restrict __file, char const *__args, ... /*, __atflag_t __flags*/),__REDIRECT_FEXECLPAT(char,fexecvpat,__file,__args))
__XREDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execlpe) __XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(2),int,__ATTR_CDECL,fexeclpeat,(char const *__restrict __file, char const *__args, ... /*, char *const ___envp[], __atflag_t __flags*/),__REDIRECT_FEXECLPEAT(char,fexecvpeat,__file,__args))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((3)) void (__LIBCCALL Xfexecvat)(__fd_t __dfd, char const *__path, __TARGV, __atflag_t __flags);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((3)) void (__LIBCCALL Xfexecveat)(__fd_t __dfd, char const *__path, __TARGV, __TENVP, __atflag_t __flags);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1,2)) void (__LIBCCALL Xfexecvpat)(char const *__restrict __file, __TARGV, __atflag_t __flags);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1,2,3)) void (__LIBCCALL Xfexecvpeat)(char const *__restrict __file, __TARGV, __TENVP, __atflag_t __flags);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((2)) __ATTR_SENTINEL_O(1) void (__ATTR_CDECL Xfexeclat)(__fd_t __dfd, char const *__path, char const *__args, ... /*, __atflag_t __flags*/);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((2)) __ATTR_SENTINEL_O(2) void (__ATTR_CDECL Xfexecleat)(__fd_t __dfd, char const *__path, char const *__args, ... /*, char *const ___envp[], __atflag_t __flags*/);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(1) void (__ATTR_CDECL Xfexeclpat)(char const *__restrict __file, char const *__args, ... /*, __atflag_t __flags*/);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(2) void (__ATTR_CDECL Xfexeclpeat)(char const *__restrict __file, char const *__args, ... /*, char *const ___envp[], __atflag_t __flags*/);
#endif /* __USE_EXCEPT */
#endif /* !__fexecvat_defined */
#endif /* __USE_ATFILE */

/* The filesystem mode provided by the KOS kernel is used to hard-configure `AT_*' filesystem flags.
 * When set, any filesystem-related system call will make use of the filesystem mask/mode to change
 * its behavior in accordance to what is requested.
 * The new filesystem mode is calculated as follows:
 *   >> new_mode = (given_mode & fm_mask) | fm_mode;
 * With that in mind, the default state is:
 *   >> fm_mask == -1
 *   >> fm_mode == 0
 * Note that not all bits can be configured.
 *   The masks of immutable bits can be reviewed within the kernel sources
 *   /src/include/fs/fd.h: FDMAN_FSMASK_ALWAYS1 / FDMAN_FSMODE_ALWAYS0
 * EXAMPLE:
 *   >> Force-enable DOS semantics for all file operations,
 *      regardless of `AT_DOSPATH' or linked libc function:
 *      fm_mask = -1;
 *      fm_mode = AT_DOSPATH;
 *   >> Force-disable DOS semantics for all file operations,
 *      regardless of `AT_DOSPATH' or linked libc function:
 *      fm_mask = ~(AT_DOSPATH);
 *      fm_mode = 0;
 * NOTE: This function never fails and always returns the old mask.
 * HINT: When prefixed before a command, the utility 'dosfs' will
 *       set the fsmask to '-1,AT_DOSPATH' before executing the
 *       remainder of the commandline as another command. */
#ifndef __fsmask_defined
#define __fsmask_defined 1
struct fsmask {
#if __KOS_VERSION__ >= 300
    __UINT32_TYPE__ fm_mask; /* Filesystem mode mask. (Set of `AT_*') */
    __UINT32_TYPE__ fm_mode; /* Filesystem mode. (Set of `AT_*') */
#else
    int             fm_mask; /* Filesystem mode mask. (Set of `AT_*') */
    int             fm_mode; /* Filesystem mode. (Set of `AT_*') */
#endif
};
#endif /* !__fsmask_defined */
__LIBC __PORT_KOSONLY struct fsmask (__LIBCCALL fsmode)(struct fsmask new_mode);
__LIBC __PORT_KOSONLY struct fsmask (__LIBCCALL getfsmode)(void);
#endif /* __CRT_KOS */

__SYSDECL_END

/* Pull in process definitions. */
#ifndef _PARTS_KOS2_PROCESS_H
#include "process.h"
#endif

#endif /* !_PARTS_KOS2_UNISTD_H */
