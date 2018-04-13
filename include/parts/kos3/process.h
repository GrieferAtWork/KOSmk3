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
#ifndef _PARTS_KOS3_PROCESS_H
#define _PARTS_KOS3_PROCESS_H 1

#include "__stdinc.h"
#include <features.h>
#include <bits/types.h>
#ifndef _PROCESS_H
#include <process.h>
#endif

__SYSDECL_BEGIN

#ifndef __pid_t_defined
#define __pid_t_defined 1
typedef __pid_t pid_t;
#endif /* !__pid_t_defined */

#ifndef __KERNEL__
/* Argument types used by exec() and spawn() functions. */
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

#ifdef __USE_ATFILE
/* At-file style exec functions. */
#ifndef __fexecvat_defined
#define __fexecvat_defined 1
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execv) __XATTR_NORETURN __NONNULL((3)),int,__LIBCCALL,fexecvat,(__fd_t __dfd, char const *__path, __TARGV, int __flags),(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execve) __XATTR_NORETURN __NONNULL((3)),int,__LIBCCALL,fexecveat,(__fd_t __dfd, char const *__path, __TARGV, __TENVP, int __flags),(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execvp) __XATTR_NORETURN __NONNULL((1,2)),int,__LIBCCALL,fexecvpat,(char const *__restrict __file, __TARGV, int __flags),(__dfd,__file,___argv,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execvpe) __XATTR_NORETURN __NONNULL((1,2,3)),int,__LIBCCALL,fexecvpeat,(char const *__restrict __file, __TARGV, __TENVP, int __flags),(__dfd,__file,___argv,__flags))
__XREDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execl) __XATTR_NORETURN __NONNULL((2)) __ATTR_SENTINEL_O(1),int,__ATTR_CDECL,fexeclat,(__fd_t __dfd, char const *__path, char const *__args, ... /*, int __flags*/),__REDIRECT_FEXECLAT(char,fexecvat,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execle) __XATTR_NORETURN __NONNULL((2)) __ATTR_SENTINEL_O(2),int,__ATTR_CDECL,fexecleat,(__fd_t __dfd, char const *__path, char const *__args, ... /*, char *const ___envp[], int __flags*/),__REDIRECT_FEXECLEAT(char,fexecveat,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execlp) __XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(1),int,__ATTR_CDECL,fexeclpat,(char const *__restrict __file, char const *__args, ... /*, int __flags*/),__REDIRECT_FEXECLPAT(char,fexecvpat,__file,__args))
__XREDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(execlpe) __XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(2),int,__ATTR_CDECL,fexeclpeat,(char const *__restrict __file, char const *__args, ... /*, char *const ___envp[], int __flags*/),__REDIRECT_FEXECLPEAT(char,fexecvpeat,__file,__args))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((3)) void (__LIBCCALL Xfexecvat)(__fd_t __dfd, char const *__path, __TARGV, int __flags);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((3)) void (__LIBCCALL Xfexecveat)(__fd_t __dfd, char const *__path, __TARGV, __TENVP, int __flags);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1,2)) void (__LIBCCALL Xfexecvpat)(char const *__restrict __file, __TARGV, int __flags);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1,2,3)) void (__LIBCCALL Xfexecvpeat)(char const *__restrict __file, __TARGV, __TENVP, int __flags);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((2)) __ATTR_SENTINEL_O(1) void (__ATTR_CDECL Xfexeclat)(__fd_t __dfd, char const *__path, char const *__args, ... /*, int __flags*/);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((2)) __ATTR_SENTINEL_O(2) void (__ATTR_CDECL Xfexecleat)(__fd_t __dfd, char const *__path, char const *__args, ... /*, char *const ___envp[], int __flags*/);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(1) void (__ATTR_CDECL Xfexeclpat)(char const *__restrict __file, char const *__args, ... /*, int __flags*/);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(2) void (__ATTR_CDECL Xfexeclpeat)(char const *__restrict __file, char const *__args, ... /*, char *const ___envp[], int __flags*/);
#endif /* __USE_EXCEPT */
#endif /* !__fexecvat_defined */

#ifndef __cwait_defined
#define __cwait_defined 1
__REDIRECT_EXCEPT_DPA(__LIBC,__PORT_DOSONLY,pid_t,__LIBCCALL,cwait,(int *__tstat, pid_t __pid, int __action),(__tstat,__pid,__action))
#ifdef __USE_EXCEPT
__LIBC __PORT_DOSONLY pid_t (__LIBCCALL Xcwait)(int *__tstat, pid_t __pid, int __action);
#endif /* __USE_EXCEPT */
#endif /* !__cwait_defined */

/* At-file style spawn functions. */
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(spawnv) __NONNULL((4)),pid_t,__LIBCCALL,fspawnvat,(int __mode, __fd_t __dfd, char const *__path, __TARGV, int __flags),(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(spawnve) __NONNULL((4)),pid_t,__LIBCCALL,fspawnveat,(int __mode, __fd_t __dfd, char const *__path, __TARGV, __TENVP, int __flags),(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(spawnvp) __NONNULL((2,3)),pid_t,__LIBCCALL,fspawnvpat,(int __mode, char const *__restrict __file, __TARGV, int __flags),(__dfd,__file,___argv,__flags))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(spawnvpe) __NONNULL((2,3,4)),pid_t,__LIBCCALL,fspawnvpeat,(int __mode, char const *__restrict __file, __TARGV, __TENVP, int __flags),(__dfd,__file,___argv,__flags))
__XREDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(spawnl) __NONNULL((3)) __ATTR_SENTINEL_O(1),pid_t,__ATTR_CDECL,fspawnlat,(int __mode, __fd_t __dfd, char const *__path, char const *__args, ... /*, int __flags*/),__REDIRECT_FSPAWNLAT(char,fspawnvat,__mode,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(spawnle) __NONNULL((3)) __ATTR_SENTINEL_O(2),pid_t,__ATTR_CDECL,fspawnleat,(int __mode, __fd_t __dfd, char const *__path, char const *__args, ... /*, char *const ___envp[], int __flags*/),__REDIRECT_FSPAWNLEAT(char,fspawnveat,__mode,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(spawnlp) __NONNULL((2)) __ATTR_SENTINEL_O(1),pid_t,__ATTR_CDECL,fspawnlpat,(int __mode, char const *__restrict __file, char const *__args, ... /*, int __flags*/),__REDIRECT_FSPAWNLPAT(char,fspawnvpat,__mode,__file,__args))
__XREDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(spawnlpe) __NONNULL((2)) __ATTR_SENTINEL_O(2),pid_t,__ATTR_CDECL,fspawnlpeat,(int __mode, char const *__restrict __file, char const *__args, ... /*, char *const ___envp[], int __flags*/),__REDIRECT_FSPAWNLPEAT(char,fspawnvpeat,__mode,__file,__args))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __NONNULL((4)) pid_t (__LIBCCALL Xfspawnvat)(int __mode, __fd_t __dfd, char const *__path, __TARGV, int __flags);
__LIBC __PORT_KOSONLY __NONNULL((4)) pid_t (__LIBCCALL Xfspawnveat)(int __mode, __fd_t __dfd, char const *__path, __TARGV, __TENVP, int __flags);
__LIBC __PORT_KOSONLY __NONNULL((2,3)) pid_t (__LIBCCALL Xfspawnvpat)(int __mode, char const *__restrict __file, __TARGV, int __flags);
__LIBC __PORT_KOSONLY __NONNULL((2,3,4)) pid_t (__LIBCCALL Xfspawnvpeat)(int __mode, char const *__restrict __file, __TARGV, __TENVP, int __flags);
__LIBC __PORT_KOSONLY __NONNULL((3)) __ATTR_SENTINEL_O(1) pid_t (__ATTR_CDECL Xfspawnlat)(int __mode, __fd_t __dfd, char const *__path, char const *__args, ... /*, int __flags*/);
__LIBC __PORT_KOSONLY __NONNULL((3)) __ATTR_SENTINEL_O(2) pid_t (__ATTR_CDECL Xfspawnleat)(int __mode, __fd_t __dfd, char const *__path, char const *__args, ... /*, char *const ___envp[], int __flags*/);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1) pid_t (__ATTR_CDECL Xfspawnlpat)(int __mode, char const *__restrict __file, char const *__args, ... /*, int __flags*/);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(2) pid_t (__ATTR_CDECL Xfspawnlpeat)(int __mode, char const *__restrict __file, char const *__args, ... /*, char *const ___envp[], int __flags*/);
#endif /* __USE_EXCEPT */
#endif /* __USE_ATFILE */

#endif /* !__KERNEL__ */

__SYSDECL_END

#ifdef _WCHAR_H
#ifndef _PARTS_KOS3_WPROCESS_H
#include "wprocess.h"
#endif
#endif

#endif /* !_PARTS_KOS3_PROCESS_H */
