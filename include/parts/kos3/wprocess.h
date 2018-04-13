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
#ifndef _PARTS_KOS3_WPROCESS_H
#define _PARTS_KOS3_WPROCESS_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <parts/redirect-exec.h>

__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef __pid_t_defined
#define __pid_t_defined 1
typedef __pid_t pid_t;
#endif /* !__pid_t_defined */

#ifndef __KERNEL__
#ifdef __CRT_KOS

#ifndef __TWARGV
#ifdef __USE_DOS
#   define __TWARGV wchar_t const *const *__restrict ___argv
#   define __TWENVP wchar_t const *const *__restrict ___envp
#else
#   define __TWARGV wchar_t *const ___argv[__restrict_arr]
#   define __TWENVP wchar_t *const ___envp[__restrict_arr]
#endif
#endif /* !__TWARGV */

/* KOS Mk3-specific wide-character exec/spawn functions */
__REDIRECT_EXCEPT_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,wfexecv,(__fd_t __fd, __TWARGV),(__fd,___argv))
__REDIRECT_EXCEPT_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((2,3)),int,__LIBCCALL,wfexecve,(__fd_t __fd, __TWARGV, __TWENVP),(__fd,___argv,___envp))
__XREDIRECT_EXCEPT_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL,int,__ATTR_CDECL,wfexecl,(__fd_t __fd, wchar_t const *__args, ...),__REDIRECT_EXECL(wchar_t,wfexecv,__fd,__args))
__XREDIRECT_EXCEPT_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL,int,__ATTR_CDECL,wfexecle,(__fd_t __fd, wchar_t const *__args, ...),__REDIRECT_EXECLE(wchar_t,wfexecve,__fd,__args))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(wexecv) __XATTR_NORETURN __NONNULL((3)),int,__LIBCCALL,wfexecvat,(__fd_t __dfd, wchar_t const *__path, __TWARGV, int __flags),(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(wexecve) __XATTR_NORETURN __NONNULL((3)),int,__LIBCCALL,wfexecveat,(__fd_t __dfd, wchar_t const *__path, __TWARGV, __TWENVP, int __flags),(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(wexecvp) __XATTR_NORETURN __NONNULL((1,2)),int,__LIBCCALL,wfexecvpat,(wchar_t const *__restrict __file, __TWARGV, int __flags),(__dfd,__file,___argv,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(wexecvpe) __XATTR_NORETURN __NONNULL((1,2,3)),int,__LIBCCALL,wfexecvpeat,(wchar_t const *__restrict __file, __TWARGV, __TWENVP, int __flags),(__dfd,__file,___argv,__flags))
__XREDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(wexecl) __XATTR_NORETURN __NONNULL((2)) __ATTR_SENTINEL_O(1),int,__ATTR_CDECL,wfexeclat,(__fd_t __dfd, wchar_t const *__path, wchar_t const *__args, ... /*, int __flags*/),__REDIRECT_FEXECLAT(wchar_t,wfexecvat,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(wexecle) __XATTR_NORETURN __NONNULL((2)) __ATTR_SENTINEL_O(2),int,__ATTR_CDECL,wfexecleat,(__fd_t __dfd, wchar_t const *__path, wchar_t const *__args, ... /*, wchar_t *const ___envp[], int __flags*/),__REDIRECT_FEXECLEAT(wchar_t,wfexecveat,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(wexeclp) __XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(1),int,__ATTR_CDECL,wfexeclpat,(wchar_t const *__restrict __file, wchar_t const *__args, ... /*, int __flags*/),__REDIRECT_FEXECLPAT(wchar_t,wfexecvpat,__file,__args))
__XREDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(wexeclpe) __XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(2),int,__ATTR_CDECL,wfexeclpeat,(wchar_t const *__restrict __file, wchar_t const *__args, ... /*, wchar_t *const ___envp[], int __flags*/),__REDIRECT_FEXECLPEAT(wchar_t,wfexecvpeat,__file,__args))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY __NONNULL((3)),pid_t,__LIBCCALL,wfspawnv,(int __mode, __fd_t __fd, __TWARGV),(__mode,__fd,___argv))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY __NONNULL((3,4)),pid_t,__LIBCCALL,wfspawnve,(int __mode, __fd_t __fd, __TWARGV, __TWENVP),(__mode,__fd,___argv,___envp))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL,pid_t,__ATTR_CDECL,wfspawnl,(int __mode, __fd_t __fd, wchar_t const *__args, ...),__REDIRECT_SPAWNL(wchar_t,wfspawnv,__mode,__fd,__args))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL,pid_t,__ATTR_CDECL,wfspawnle,(int __mode, __fd_t __fd, wchar_t const *__args, ...),__REDIRECT_SPAWNLE(wchar_t,wfspawnv,__mode,__fd,__args))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(wspawnv) __NONNULL((4)),pid_t,__LIBCCALL,wfspawnvat,(int __mode, __fd_t __dfd, wchar_t const *__path, __TWARGV, int __flags),(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(wspawnve) __NONNULL((4)),pid_t,__LIBCCALL,wfspawnveat,(int __mode, __fd_t __dfd, wchar_t const *__path, __TWARGV, __TWENVP, int __flags),(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(wspawnvp) __NONNULL((2,3)),pid_t,__LIBCCALL,wfspawnvpat,(int __mode, wchar_t const *__restrict __file, __TWARGV, int __flags),(__dfd,__file,___argv,__flags))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(wspawnvpe) __NONNULL((2,3,4)),pid_t,__LIBCCALL,wfspawnvpeat,(int __mode, wchar_t const *__restrict __file, __TWARGV, __TWENVP, int __flags),(__dfd,__file,___argv,__flags))
__XREDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(wspawnl) __NONNULL((3)) __ATTR_SENTINEL_O(1),pid_t,__ATTR_CDECL,wfspawnlat,(int __mode, __fd_t __dfd, wchar_t const *__path, wchar_t const *__args, ... /*, int __flags*/),__REDIRECT_FSPAWNLAT(wchar_t,wfspawnvat,__mode,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(wspawnle) __NONNULL((3)) __ATTR_SENTINEL_O(2),pid_t,__ATTR_CDECL,wfspawnleat,(int __mode, __fd_t __dfd, wchar_t const *__path, wchar_t const *__args, ... /*, wchar_t *const ___envp[], int __flags*/),__REDIRECT_FSPAWNLEAT(wchar_t,wfspawnveat,__mode,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(wspawnlp) __NONNULL((2)) __ATTR_SENTINEL_O(1),pid_t,__ATTR_CDECL,wfspawnlpat,(int __mode, wchar_t const *__restrict __file, wchar_t const *__args, ... /*, int __flags*/),__REDIRECT_FSPAWNLPAT(wchar_t,wfspawnvpat,__mode,__file,__args))
__XREDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY_ALT(wspawnlpe) __NONNULL((2)) __ATTR_SENTINEL_O(2),pid_t,__ATTR_CDECL,wfspawnlpeat,(int __mode, wchar_t const *__restrict __file, wchar_t const *__args, ... /*, wchar_t *const ___envp[], int __flags*/),__REDIRECT_FSPAWNLPEAT(wchar_t,wfspawnvpeat,__mode,__file,__args))
#ifdef __USE_EXCEPT
__LIBC __ATTR_NORETURN __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xwfexecv)(__fd_t __fd, __TWARGV);
__LIBC __ATTR_NORETURN __PORT_KOSONLY __NONNULL((2,3)) void (__LIBCCALL Xwfexecve)(__fd_t __fd, __TWARGV, __TWENVP);
__LIBC __ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL void (__ATTR_CDECL Xwfexecl)(__fd_t __fd, wchar_t const *__args, ...);
__LIBC __ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL void (__ATTR_CDECL Xwfexecle)(__fd_t __fd, wchar_t const *__args, ...);
__LIBC __ATTR_NORETURN __PORT_KOSONLY __NONNULL((3)) void (__LIBCCALL Xwfexecvat)(__fd_t __dfd, wchar_t const *__path, __TWARGV, int __flags);
__LIBC __ATTR_NORETURN __PORT_KOSONLY __NONNULL((3)) void (__LIBCCALL Xwfexecveat)(__fd_t __dfd, wchar_t const *__path, __TWARGV, __TWENVP, int __flags);
__LIBC __ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2)) void (__LIBCCALL Xwfexecvpat)(wchar_t const *__restrict __file, __TWARGV, int __flags);
__LIBC __ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2,3)) void (__LIBCCALL Xwfexecvpeat)(wchar_t const *__restrict __file, __TWARGV, __TWENVP, int __flags);
__LIBC __ATTR_NORETURN __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1) void (__ATTR_CDECL Xwfexeclat)(__fd_t __dfd, wchar_t const *__path, wchar_t const *__args, ... /*, int __flags*/);
__LIBC __ATTR_NORETURN __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(2) void (__ATTR_CDECL Xwfexecleat)(__fd_t __dfd, wchar_t const *__path, wchar_t const *__args, ... /*, wchar_t *const ___envp[], int __flags*/);
__LIBC __ATTR_NORETURN __PORT_KOSONLY __NONNULL((1)) __ATTR_SENTINEL_O(1) void (__ATTR_CDECL Xwfexeclpat)(wchar_t const *__restrict __file, wchar_t const *__args, ... /*, int __flags*/);
__LIBC __ATTR_NORETURN __PORT_KOSONLY __NONNULL((1)) __ATTR_SENTINEL_O(2) void (__ATTR_CDECL Xwfexeclpeat)(wchar_t const *__restrict __file, wchar_t const *__args, ... /*, wchar_t *const ___envp[], int __flags*/);
__LIBC __PORT_KOSONLY __NONNULL((3)) pid_t (__LIBCCALL Xwfspawnv)(int __mode, __fd_t __fd, __TWARGV);
__LIBC __PORT_KOSONLY __NONNULL((3,4)) pid_t (__LIBCCALL Xwfspawnve)(int __mode, __fd_t __fd, __TWARGV, __TWENVP);
__LIBC __PORT_KOSONLY __ATTR_SENTINEL pid_t (__ATTR_CDECL Xwfspawnl)(int __mode, __fd_t __fd, wchar_t const *__args, ...);
__LIBC __PORT_KOSONLY __ATTR_SENTINEL pid_t (__ATTR_CDECL Xwfspawnle)(int __mode, __fd_t __fd, wchar_t const *__args, ...);
__LIBC __PORT_KOSONLY __NONNULL((4)) pid_t (__LIBCCALL Xwfspawnvat)(int __mode, __fd_t __dfd, wchar_t const *__path, __TWARGV, int __flags);
__LIBC __PORT_KOSONLY __NONNULL((4)) pid_t (__LIBCCALL Xwfspawnveat)(int __mode, __fd_t __dfd, wchar_t const *__path, __TWARGV, __TWENVP, int __flags);
__LIBC __PORT_KOSONLY __NONNULL((2,3)) pid_t (__LIBCCALL Xwfspawnvpat)(int __mode, wchar_t const *__restrict __file, __TWARGV, int __flags);
__LIBC __PORT_KOSONLY __NONNULL((2,3,4)) pid_t (__LIBCCALL Xwfspawnvpeat)(int __mode, wchar_t const *__restrict __file, __TWARGV, __TWENVP, int __flags);
__LIBC __PORT_KOSONLY __NONNULL((3)) __ATTR_SENTINEL_O(1) pid_t (__ATTR_CDECL Xwfspawnlat)(int __mode, __fd_t __dfd, wchar_t const *__path, wchar_t const *__args, ... /*, int __flags*/);
__LIBC __PORT_KOSONLY __NONNULL((3)) __ATTR_SENTINEL_O(2) pid_t (__ATTR_CDECL Xwfspawnleat)(int __mode, __fd_t __dfd, wchar_t const *__path, wchar_t const *__args, ... /*, wchar_t *const ___envp[], int __flags*/);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1) pid_t (__ATTR_CDECL Xwfspawnlpat)(int __mode, wchar_t const *__restrict __file, wchar_t const *__args, ... /*, int __flags*/);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(2) pid_t (__ATTR_CDECL Xwfspawnlpeat)(int __mode, wchar_t const *__restrict __file, wchar_t const *__args, ... /*, wchar_t *const ___envp[], int __flags*/);
#endif /* __USE_EXCEPT */

#endif /* __CRT_KOS */
#endif /* __KERNEL__ */

__SYSDECL_END

#endif /* !_PARTS_KOS3_WPROCESS_H */
