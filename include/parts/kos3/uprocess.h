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
#ifndef _PARTS_KOS3_UPROCESS_H
#define _PARTS_KOS3_UPROCESS_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <parts/redirect-exec.h>

__SYSDECL_BEGIN

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

#ifndef __pid_t_defined
#define __pid_t_defined 1
typedef __pid_t pid_t;
#endif /* !__pid_t_defined */

#ifndef __KERNEL__
#ifdef __CRT_KOS

#ifndef __TW16ARGV
#ifdef __USE_DOS
#   define __TW16ARGV char16_t const *const *__restrict ___argv
#   define __TW32ARGV char32_t const *const *__restrict ___argv
#   define __TW16ENVP char16_t const *const *__restrict ___envp
#   define __TW32ENVP char32_t const *const *__restrict ___envp
#else
#   define __TW16ARGV char16_t *const ___argv[__restrict_arr]
#   define __TW32ARGV char32_t *const ___argv[__restrict_arr]
#   define __TW16ENVP char16_t *const ___envp[__restrict_arr]
#   define __TW32ENVP char32_t *const ___envp[__restrict_arr]
#endif
#endif /* !__TW16ARGV */

/* KOS Mk3-specific unicode exec/spawn functions */
__REDIRECT_EXCEPT_W16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16fexecv,(__fd_t __fd, __TW16ARGV),wfexecv,(__fd,___argv))
__REDIRECT_EXCEPT_W32_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32fexecv,(__fd_t __fd, __TW32ARGV),wfexecv,(__fd,___argv))
__REDIRECT_EXCEPT_W16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((2,3)),int,__LIBCCALL,w16fexecve,(__fd_t __fd, __TW16ARGV, __TW16ENVP),wfexecve,(__fd,___argv,__envp))
__REDIRECT_EXCEPT_W32_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((2,3)),int,__LIBCCALL,w32fexecve,(__fd_t __fd, __TW32ARGV, __TW32ENVP),wfexecve,(__fd,___argv,__envp))
__XREDIRECT_EXCEPT_W16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL,int,__ATTR_CDECL,w16fexecl,(__fd_t __fd, char16_t const *__args, ...),wfexecl,__REDIRECT_EXECL(char16_t,w16fexecv,__fd,__args))
__XREDIRECT_EXCEPT_W32_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL,int,__ATTR_CDECL,w32fexecl,(__fd_t __fd, char32_t const *__args, ...),wfexecl,__REDIRECT_EXECL(char32_t,w32fexecv,__fd,__args))
__XREDIRECT_EXCEPT_W16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL,int,__ATTR_CDECL,w16fexecle,(__fd_t __fd, char16_t const *__args, ...),wfexecle,__REDIRECT_EXECLE(char16_t,w16fexecve,__fd,__args))
__XREDIRECT_EXCEPT_W32_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL,int,__ATTR_CDECL,w32fexecle,(__fd_t __fd, char32_t const *__args, ...),wfexecle,__REDIRECT_EXECLE(char32_t,w32fexecve,__fd,__args))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((3)),int,__LIBCCALL,w16fexecvat,(__fd_t __dfd, char16_t const *__path, __TW16ARGV, __atflag_t __flags),wfexecvat,(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((3)),int,__LIBCCALL,w32fexecvat,(__fd_t __dfd, char32_t const *__path, __TW32ARGV, __atflag_t __flags),wfexecvat,(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((3)),int,__LIBCCALL,w16fexecveat,(__fd_t __dfd, char16_t const *__path, __TW16ARGV, __TW16ENVP, __atflag_t __flags),wfexecveat,(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((3)),int,__LIBCCALL,w32fexecveat,(__fd_t __dfd, char32_t const *__path, __TW32ARGV, __TW32ENVP, __atflag_t __flags),wfexecveat,(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w16fexecvpat,(char16_t const *__restrict __file, __TW16ARGV, __atflag_t __flags),wfexecvpat,(__dfd,__file,___argv,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w32fexecvpat,(char32_t const *__restrict __file, __TW32ARGV, __atflag_t __flags),wfexecvpat,(__dfd,__file,___argv,__flags))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2,3)),int,__LIBCCALL,w16fexecvpeat,(char16_t const *__restrict __file, __TW16ARGV, __TW16ENVP, __atflag_t __flags),wfexecvpeat,(__dfd,__file,___argv,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2,3)),int,__LIBCCALL,w32fexecvpeat,(char32_t const *__restrict __file, __TW32ARGV, __TW32ENVP, __atflag_t __flags),wfexecvpeat,(__dfd,__file,___argv,__flags))
__XREDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),int,__ATTR_CDECL,w16fexeclat,(__fd_t __dfd, char16_t const *__path, char16_t const *__args, ... /*, __atflag_t __flags*/),wfexeclat,__REDIRECT_FEXECLAT(char16_t,w16fexecvat,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),int,__ATTR_CDECL,w32fexeclat,(__fd_t __dfd, char32_t const *__path, char32_t const *__args, ... /*, __atflag_t __flags*/),wfexeclat,__REDIRECT_FEXECLAT(char32_t,w32fexecvat,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(2),int,__ATTR_CDECL,w16fexecleat,(__fd_t __dfd, char16_t const *__path, char16_t const *__args, ... /*, char16_t *const ___envp[], __atflag_t __flags*/),wfexecleat,__REDIRECT_FEXECLEAT(char16_t,w16fexecveat,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(2),int,__ATTR_CDECL,w32fexecleat,(__fd_t __dfd, char32_t const *__path, char32_t const *__args, ... /*, char32_t *const ___envp[], __atflag_t __flags*/),wfexecleat,__REDIRECT_FEXECLEAT(char32_t,w32fexecveat,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1)) __ATTR_SENTINEL_O(1),int,__ATTR_CDECL,w16fexeclpat,(char16_t const *__restrict __file, char16_t const *__args, ... /*, __atflag_t __flags*/),wfexeclpat,__REDIRECT_FEXECLPAT(char16_t,w16fexecvpat,__file,__args))
__XREDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1)) __ATTR_SENTINEL_O(1),int,__ATTR_CDECL,w32fexeclpat,(char32_t const *__restrict __file, char32_t const *__args, ... /*, __atflag_t __flags*/),wfexeclpat,__REDIRECT_FEXECLPAT(char32_t,w32fexecvpat,__file,__args))
__XREDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1)) __ATTR_SENTINEL_O(2),int,__ATTR_CDECL,w16fexeclpeat,(char16_t const *__restrict __file, char16_t const *__args, ... /*, char16_t *const ___envp[], __atflag_t __flags*/),wfexeclpeat,__REDIRECT_FEXECLPEAT(char16_t,w16fexecvpeat,__file,__args))
__XREDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1)) __ATTR_SENTINEL_O(2),int,__ATTR_CDECL,w32fexeclpeat,(char32_t const *__restrict __file, char32_t const *__args, ... /*, char32_t *const ___envp[], __atflag_t __flags*/),wfexeclpeat,__REDIRECT_FEXECLPEAT(char32_t,w32fexecvpeat,__file,__args))
__REDIRECT_EXCEPT_W16(__LIBC,__PORT_KOSONLY __NONNULL((3)),pid_t,__LIBCCALL,w16fspawnv,(int __mode, __fd_t __fd, __TW16ARGV),wfspawnv,(__mode,__fd,___argv))
__REDIRECT_EXCEPT_W32(__LIBC,__PORT_KOSONLY __NONNULL((3)),pid_t,__LIBCCALL,w32fspawnv,(int __mode, __fd_t __fd, __TW32ARGV),wfspawnv,(__mode,__fd,___argv))
__REDIRECT_EXCEPT_W16(__LIBC,__PORT_KOSONLY __NONNULL((3,4)),pid_t,__LIBCCALL,w16fspawnve,(int __mode, __fd_t __fd, __TW16ARGV, __TW16ENVP),wfspawnve,(__mode,__fd,___argv,__envp))
__REDIRECT_EXCEPT_W32(__LIBC,__PORT_KOSONLY __NONNULL((3,4)),pid_t,__LIBCCALL,w32fspawnve,(int __mode, __fd_t __fd, __TW32ARGV, __TW32ENVP),wfspawnve,(__mode,__fd,___argv,__envp))
__XREDIRECT_EXCEPT_W16(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL,pid_t,__ATTR_CDECL,w16fspawnl,(int __mode, __fd_t __fd, char16_t const *__args, ...),wfspawnl,__REDIRECT_SPAWNL(char16_t,w16fspawnv,__mode,__fd,__args))
__XREDIRECT_EXCEPT_W32(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL,pid_t,__ATTR_CDECL,w32fspawnl,(int __mode, __fd_t __fd, char32_t const *__args, ...),wfspawnl,__REDIRECT_SPAWNL(char32_t,w32fspawnv,__mode,__fd,__args))
__XREDIRECT_EXCEPT_W16(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL,pid_t,__ATTR_CDECL,w16fspawnle,(int __mode, __fd_t __fd, char16_t const *__args, ...),wfspawnle,__REDIRECT_SPAWNLE(char16_t,w16fspawnve,__mode,__fd,__args))
__XREDIRECT_EXCEPT_W32(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL,pid_t,__ATTR_CDECL,w32fspawnle,(int __mode, __fd_t __fd, char32_t const *__args, ...),wfspawnle,__REDIRECT_SPAWNLE(char32_t,w32fspawnve,__mode,__fd,__args))
__REDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __NONNULL((4)),pid_t,__LIBCCALL,w16fspawnvat,(int __mode, __fd_t __dfd, char16_t const *__path, __TW16ARGV, __atflag_t __flags),wfspawnvat,(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((4)),pid_t,__LIBCCALL,w32fspawnvat,(int __mode, __fd_t __dfd, char32_t const *__path, __TW32ARGV, __atflag_t __flags),wfspawnvat,(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __NONNULL((4)),pid_t,__LIBCCALL,w16fspawnveat,(int __mode, __fd_t __dfd, char16_t const *__path, __TW16ARGV, __TW16ENVP, __atflag_t __flags),wfspawnveat,(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((4)),pid_t,__LIBCCALL,w32fspawnveat,(int __mode, __fd_t __dfd, char32_t const *__path, __TW32ARGV, __TW32ENVP, __atflag_t __flags),wfspawnveat,(__dfd,__path,___argv,__flags))
__REDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,w16fspawnvpat,(int __mode, char16_t const *__restrict __file, __TW16ARGV, __atflag_t __flags),wfspawnvpat,(__dfd,__file,___argv,__flags))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,w32fspawnvpat,(int __mode, char32_t const *__restrict __file, __TW32ARGV, __atflag_t __flags),wfspawnvpat,(__dfd,__file,___argv,__flags))
__REDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,w16fspawnvpeat,(int __mode, char16_t const *__restrict __file, __TW16ARGV, __TW16ENVP, __atflag_t __flags),wfspawnvpeat,(__dfd,__file,___argv,__flags))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,w32fspawnvpeat,(int __mode, char32_t const *__restrict __file, __TW32ARGV, __TW32ENVP, __atflag_t __flags),wfspawnvpeat,(__dfd,__file,___argv,__flags))
__XREDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __NONNULL((3)) __ATTR_SENTINEL_O(1),pid_t,__ATTR_CDECL,w16fspawnlat,(int __mode, __fd_t __dfd, char16_t const *__path, char16_t const *__args, ... /*, __atflag_t __flags*/),wfspawnlat,__REDIRECT_FSPAWNLAT(char16_t,w16fspawnvat,__mode,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((3)) __ATTR_SENTINEL_O(1),pid_t,__ATTR_CDECL,w32fspawnlat,(int __mode, __fd_t __dfd, char32_t const *__path, char32_t const *__args, ... /*, __atflag_t __flags*/),wfspawnlat,__REDIRECT_FSPAWNLAT(char32_t,w32fspawnvat,__mode,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __NONNULL((3)) __ATTR_SENTINEL_O(2),pid_t,__ATTR_CDECL,w16fspawnleat,(int __mode, __fd_t __dfd, char16_t const *__path, char16_t const *__args, ... /*, char16_t *const ___envp[], __atflag_t __flags*/),wfspawnleat,__REDIRECT_FSPAWNLEAT(char16_t,w16fspawnveat,__mode,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((3)) __ATTR_SENTINEL_O(2),pid_t,__ATTR_CDECL,w32fspawnleat,(int __mode, __fd_t __dfd, char32_t const *__path, char32_t const *__args, ... /*, char32_t *const ___envp[], __atflag_t __flags*/),wfspawnleat,__REDIRECT_FSPAWNLEAT(char32_t,w32fspawnveat,__mode,__dfd,__path,__args))
__XREDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),pid_t,__ATTR_CDECL,w16fspawnlpat,(int __mode, char16_t const *__restrict __file, char16_t const *__args, ... /*, __atflag_t __flags*/),wfspawnlpat,__REDIRECT_FSPAWNLPAT(char16_t,w16fspawnvpat,__mode,__file,__args))
__XREDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),pid_t,__ATTR_CDECL,w32fspawnlpat,(int __mode, char32_t const *__restrict __file, char32_t const *__args, ... /*, __atflag_t __flags*/),wfspawnlpat,__REDIRECT_FSPAWNLPAT(char32_t,w32fspawnvpat,__mode,__file,__args))
__XREDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(2),pid_t,__ATTR_CDECL,w16fspawnlpeat,(int __mode, char16_t const *__restrict __file, char16_t const *__args, ... /*, char16_t *const ___envp[], __atflag_t __flags*/),wfspawnlpeat,__REDIRECT_FSPAWNLPEAT(char16_t,w16fspawnvpeat,__mode,__file,__args))
__XREDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(2),pid_t,__ATTR_CDECL,w32fspawnlpeat,(int __mode, char32_t const *__restrict __file, char32_t const *__args, ... /*, char32_t *const ___envp[], __atflag_t __flags*/),wfspawnlpeat,__REDIRECT_FSPAWNLPEAT(char32_t,w32fspawnvpeat,__mode,__file,__args))

#ifdef __USE_EXCEPT
__REDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16fexecv,(__fd_t __fd, __TW16ARGV),Xwfexecv,(__fd,___argv))
__REDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32fexecv,(__fd_t __fd, __TW32ARGV),Xwfexecv,(__fd,___argv))
__REDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xw16fexecve,(__fd_t __fd, __TW16ARGV, __TW16ENVP),Xwfexecve,(__fd,___argv,__envp))
__REDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((2,3)),__LIBCCALL,Xw32fexecve,(__fd_t __fd, __TW32ARGV, __TW32ENVP),Xwfexecve,(__fd,___argv,__envp))
__XREDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL,__ATTR_CDECL,Xw16fexecl,(__fd_t __fd, char16_t const *__args, ...),Xwfexecl,__REDIRECT_XEXECL(char16_t,Xw16fexecv,__fd,__args))
__XREDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL,__ATTR_CDECL,Xw32fexecl,(__fd_t __fd, char32_t const *__args, ...),Xwfexecl,__REDIRECT_XEXECL(char32_t,Xw32fexecv,__fd,__args))
__XREDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL,__ATTR_CDECL,Xw16fexecle,(__fd_t __fd, char16_t const *__args, ...),Xwfexecle,__REDIRECT_XEXECLE(char16_t,Xw16fexecve,__fd,__args))
__XREDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL,__ATTR_CDECL,Xw32fexecle,(__fd_t __fd, char32_t const *__args, ...),Xwfexecle,__REDIRECT_XEXECLE(char32_t,Xw32fexecve,__fd,__args))
__REDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((3)),__LIBCCALL,Xw16fexecvat,(__fd_t __dfd, char16_t const *__path, __TW16ARGV, __atflag_t __flags),Xwfexecvat,(__dfd,__path,___argv,__flags))
__REDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((3)),__LIBCCALL,Xw32fexecvat,(__fd_t __dfd, char32_t const *__path, __TW32ARGV, __atflag_t __flags),Xwfexecvat,(__dfd,__path,___argv,__flags))
__REDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((3)),__LIBCCALL,Xw16fexecveat,(__fd_t __dfd, char16_t const *__path, __TW16ARGV, __TW16ENVP, __atflag_t __flags),Xwfexecveat,(__dfd,__path,___argv,__flags))
__REDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((3)),__LIBCCALL,Xw32fexecveat,(__fd_t __dfd, char32_t const *__path, __TW32ARGV, __TW32ENVP, __atflag_t __flags),Xwfexecveat,(__dfd,__path,___argv,__flags))
__REDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw16fexecvpat,(char16_t const *__restrict __file, __TW16ARGV, __atflag_t __flags),Xwfexecvpat,(__dfd,__file,___argv,__flags))
__REDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw32fexecvpat,(char32_t const *__restrict __file, __TW32ARGV, __atflag_t __flags),Xwfexecvpat,(__dfd,__file,___argv,__flags))
__REDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2,3)),__LIBCCALL,Xw16fexecvpeat,(char16_t const *__restrict __file, __TW16ARGV, __TW16ENVP, __atflag_t __flags),Xwfexecvpeat,(__dfd,__file,___argv,__flags))
__REDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2,3)),__LIBCCALL,Xw32fexecvpeat,(char32_t const *__restrict __file, __TW32ARGV, __TW32ENVP, __atflag_t __flags),Xwfexecvpeat,(__dfd,__file,___argv,__flags))
__XREDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),__ATTR_CDECL,Xw16fexeclat,(__fd_t __dfd, char16_t const *__path, char16_t const *__args, ... /*, __atflag_t __flags*/),Xwfexeclat,__REDIRECT_XFEXECLAT(char16_t,Xw16fexecvat,__dfd,__path,__args))
__XREDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),__ATTR_CDECL,Xw32fexeclat,(__fd_t __dfd, char32_t const *__path, char32_t const *__args, ... /*, __atflag_t __flags*/),Xwfexeclat,__REDIRECT_XFEXECLAT(char32_t,Xw32fexecvat,__dfd,__path,__args))
__XREDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(2),__ATTR_CDECL,Xw16fexecleat,(__fd_t __dfd, char16_t const *__path, char16_t const *__args, ... /*, char16_t *const ___envp[], __atflag_t __flags*/),Xwfexecleat,__REDIRECT_XFEXECLEAT(char16_t,Xw16fexecveat,__dfd,__path,__args))
__XREDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(2),__ATTR_CDECL,Xw32fexecleat,(__fd_t __dfd, char32_t const *__path, char32_t const *__args, ... /*, char32_t *const ___envp[], __atflag_t __flags*/),Xwfexecleat,__REDIRECT_XFEXECLEAT(char32_t,Xw32fexecveat,__dfd,__path,__args))
__XREDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1)) __ATTR_SENTINEL_O(1),__ATTR_CDECL,Xw16fexeclpat,(char16_t const *__restrict __file, char16_t const *__args, ... /*, __atflag_t __flags*/),Xwfexeclpat,__REDIRECT_XFEXECLPAT(char16_t,Xw16fexecvpat,__file,__args))
__XREDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1)) __ATTR_SENTINEL_O(1),__ATTR_CDECL,Xw32fexeclpat,(char32_t const *__restrict __file, char32_t const *__args, ... /*, __atflag_t __flags*/),Xwfexeclpat,__REDIRECT_XFEXECLPAT(char32_t,Xw32fexecvpat,__file,__args))
__XREDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1)) __ATTR_SENTINEL_O(2),__ATTR_CDECL,Xw16fexeclpeat,(char16_t const *__restrict __file, char16_t const *__args, ... /*, char16_t *const ___envp[], __atflag_t __flags*/),Xwfexeclpeat,__REDIRECT_XFEXECLPEAT(char16_t,Xw16fexecvpeat,__file,__args))
__XREDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1)) __ATTR_SENTINEL_O(2),__ATTR_CDECL,Xw32fexeclpeat,(char32_t const *__restrict __file, char32_t const *__args, ... /*, char32_t *const ___envp[], __atflag_t __flags*/),Xwfexeclpeat,__REDIRECT_XFEXECLPEAT(char32_t,Xw32fexecvpeat,__file,__args))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((3)),pid_t,__LIBCCALL,Xw16fspawnv,(int __mode, __fd_t __fd, __TW16ARGV),Xwfspawnv,(__mode,__fd,___argv))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((3)),pid_t,__LIBCCALL,Xw32fspawnv,(int __mode, __fd_t __fd, __TW32ARGV),Xwfspawnv,(__mode,__fd,___argv))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((3,4)),pid_t,__LIBCCALL,Xw16fspawnve,(int __mode, __fd_t __fd, __TW16ARGV, __TW16ENVP),Xwfspawnve,(__mode,__fd,___argv,__envp))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((3,4)),pid_t,__LIBCCALL,Xw32fspawnve,(int __mode, __fd_t __fd, __TW32ARGV, __TW32ENVP),Xwfspawnve,(__mode,__fd,___argv,__envp))
__XREDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL,pid_t,__ATTR_CDECL,Xw16fspawnl,(int __mode, __fd_t __fd, char16_t const *__args, ...),Xwfspawnl,__REDIRECT_XSPAWNL(char16_t,Xw16fspawnv,__mode,__fd,__args))
__XREDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL,pid_t,__ATTR_CDECL,Xw32fspawnl,(int __mode, __fd_t __fd, char32_t const *__args, ...),Xwfspawnl,__REDIRECT_XSPAWNL(char32_t,Xw32fspawnv,__mode,__fd,__args))
__XREDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL,pid_t,__ATTR_CDECL,Xw16fspawnle,(int __mode, __fd_t __fd, char16_t const *__args, ...),Xwfspawnle,__REDIRECT_XSPAWNLE(char16_t,Xw16fspawnve,__mode,__fd,__args))
__XREDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL,pid_t,__ATTR_CDECL,Xw32fspawnle,(int __mode, __fd_t __fd, char32_t const *__args, ...),Xwfspawnle,__REDIRECT_XSPAWNLE(char32_t,Xw32fspawnve,__mode,__fd,__args))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((4)),pid_t,__LIBCCALL,Xw16fspawnvat,(int __mode, __fd_t __dfd, char16_t const *__path, __TW16ARGV, __atflag_t __flags),Xwfspawnvat,(__dfd,__path,___argv,__flags))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((4)),pid_t,__LIBCCALL,Xw32fspawnvat,(int __mode, __fd_t __dfd, char32_t const *__path, __TW32ARGV, __atflag_t __flags),Xwfspawnvat,(__dfd,__path,___argv,__flags))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((4)),pid_t,__LIBCCALL,Xw16fspawnveat,(int __mode, __fd_t __dfd, char16_t const *__path, __TW16ARGV, __TW16ENVP, __atflag_t __flags),Xwfspawnveat,(__dfd,__path,___argv,__flags))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((4)),pid_t,__LIBCCALL,Xw32fspawnveat,(int __mode, __fd_t __dfd, char32_t const *__path, __TW32ARGV, __TW32ENVP, __atflag_t __flags),Xwfspawnveat,(__dfd,__path,___argv,__flags))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,Xw16fspawnvpat,(int __mode, char16_t const *__restrict __file, __TW16ARGV, __atflag_t __flags),Xwfspawnvpat,(__dfd,__file,___argv,__flags))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,Xw32fspawnvpat,(int __mode, char32_t const *__restrict __file, __TW32ARGV, __atflag_t __flags),Xwfspawnvpat,(__dfd,__file,___argv,__flags))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,Xw16fspawnvpeat,(int __mode, char16_t const *__restrict __file, __TW16ARGV, __TW16ENVP, __atflag_t __flags),Xwfspawnvpeat,(__dfd,__file,___argv,__flags))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,Xw32fspawnvpeat,(int __mode, char32_t const *__restrict __file, __TW32ARGV, __TW32ENVP, __atflag_t __flags),Xwfspawnvpeat,(__dfd,__file,___argv,__flags))
__XREDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((3)) __ATTR_SENTINEL_O(1),pid_t,__ATTR_CDECL,Xw16fspawnlat,(int __mode, __fd_t __dfd, char16_t const *__path, char16_t const *__args, ... /*, __atflag_t __flags*/),Xwfspawnlat,__REDIRECT_XFSPAWNLAT(char16_t,Xw16fspawnvat,__mode,__dfd,__path,__args))
__XREDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((3)) __ATTR_SENTINEL_O(1),pid_t,__ATTR_CDECL,Xw32fspawnlat,(int __mode, __fd_t __dfd, char32_t const *__path, char32_t const *__args, ... /*, __atflag_t __flags*/),Xwfspawnlat,__REDIRECT_XFSPAWNLAT(char32_t,Xw32fspawnvat,__mode,__dfd,__path,__args))
__XREDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((3)) __ATTR_SENTINEL_O(2),pid_t,__ATTR_CDECL,Xw16fspawnleat,(int __mode, __fd_t __dfd, char16_t const *__path, char16_t const *__args, ... /*, char16_t *const ___envp[], __atflag_t __flags*/),Xwfspawnleat,__REDIRECT_XFSPAWNLEAT(char16_t,Xw16fspawnveat,__mode,__dfd,__path,__args))
__XREDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((3)) __ATTR_SENTINEL_O(2),pid_t,__ATTR_CDECL,Xw32fspawnleat,(int __mode, __fd_t __dfd, char32_t const *__path, char32_t const *__args, ... /*, char32_t *const ___envp[], __atflag_t __flags*/),Xwfspawnleat,__REDIRECT_XFSPAWNLEAT(char32_t,Xw32fspawnveat,__mode,__dfd,__path,__args))
__XREDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),pid_t,__ATTR_CDECL,Xw16fspawnlpat,(int __mode, char16_t const *__restrict __file, char16_t const *__args, ... /*, __atflag_t __flags*/),Xwfspawnlpat,__REDIRECT_XFSPAWNLPAT(char16_t,Xw16fspawnvpat,__mode,__file,__args))
__XREDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),pid_t,__ATTR_CDECL,Xw32fspawnlpat,(int __mode, char32_t const *__restrict __file, char32_t const *__args, ... /*, __atflag_t __flags*/),Xwfspawnlpat,__REDIRECT_XFSPAWNLPAT(char32_t,Xw32fspawnvpat,__mode,__file,__args))
__XREDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(2),pid_t,__ATTR_CDECL,Xw16fspawnlpeat,(int __mode, char16_t const *__restrict __file, char16_t const *__args, ... /*, char16_t *const ___envp[], __atflag_t __flags*/),Xwfspawnlpeat,__REDIRECT_XFSPAWNLPEAT(char16_t,Xw16fspawnvpeat,__mode,__file,__args))
__XREDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(2),pid_t,__ATTR_CDECL,Xw32fspawnlpeat,(int __mode, char32_t const *__restrict __file, char32_t const *__args, ... /*, char32_t *const ___envp[], __atflag_t __flags*/),Xwfspawnlpeat,__REDIRECT_XFSPAWNLPEAT(char32_t,Xw32fspawnvpeat,__mode,__file,__args))
#endif /* __USE_EXCEPT */

#endif /* __CRT_KOS */
#endif /* __KERNEL__ */

__SYSDECL_END

#endif /* !_PARTS_KOS3_UPROCESS_H */
