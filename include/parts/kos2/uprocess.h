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
#ifndef _PARTS_KOS2_UPROCESS_H
#define _PARTS_KOS2_UPROCESS_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <xlocale.h>
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

/* Wide-character enabled exec() and spawn() functions. */
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w16execv,(char16_t const *__restrict __path, __TW16ARGV),wexecv,(__path,___argv))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w32execv,(char32_t const *__restrict __path, __TW32ARGV),wexecv,(__path,___argv))
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2,3)),int,__LIBCCALL,w16execve,(char16_t const *__restrict __path, __TW16ARGV, __TW16ENVP),wexecve,(__path,___argv,___envp))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2,3)),int,__LIBCCALL,w32execve,(char32_t const *__restrict __path, __TW32ARGV, __TW32ENVP),wexecve,(__path,___argv,___envp))
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w16execvp,(char16_t const *__restrict __file, __TW16ARGV),wexecvp,(__file,___argv))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w32execvp,(char32_t const *__restrict __file, __TW32ARGV),wexecvp,(__file,___argv))
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2,3)),int,__LIBCCALL,w16execvpe,(char16_t const *__restrict __file, __TW16ARGV, __TW16ENVP),wexecvpe,(__file,___argv,___envp))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__XATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2,3)),int,__LIBCCALL,w32execvpe,(char32_t const *__restrict __file, __TW32ARGV, __TW32ENVP),wexecvpe,(__file,___argv,___envp))
__XREDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL __NONNULL((1)),int,__ATTR_CDECL,w16execl,(char16_t const *__restrict __path, char16_t const *__args, ...),wexecl,__REDIRECT_EXECL(char16_t,w16execv,__path,__args))
__XREDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL __NONNULL((1)),int,__ATTR_CDECL,w32execl,(char32_t const *__restrict __path, char32_t const *__args, ...),wexecl,__REDIRECT_EXECL(char32_t,w32execv,__path,__args))
__XREDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((1)),int,__ATTR_CDECL,w16execle,(char16_t const *__restrict __path, char16_t const *__args, ...),wexecle,__REDIRECT_EXECLE(char16_t,w16execve,__path,__args))
__XREDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((1)),int,__ATTR_CDECL,w32execle,(char32_t const *__restrict __path, char32_t const *__args, ...),wexecle,__REDIRECT_EXECLE(char32_t,w32execve,__path,__args))
__XREDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL __NONNULL((1)),int,__ATTR_CDECL,w16execlp,(char16_t const *__restrict __file, char16_t const *__args, ...),wexeclp,__REDIRECT_EXECL(char16_t,w16execvp,__path,__args))
__XREDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL __NONNULL((1)),int,__ATTR_CDECL,w32execlp,(char32_t const *__restrict __file, char32_t const *__args, ...),wexeclp,__REDIRECT_EXECL(char32_t,w32execvp,__path,__args))
__XREDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((1)),int,__ATTR_CDECL,w16execlpe,(char16_t const *__restrict __file, char16_t const *__args, ...),wexeclpe,__REDIRECT_EXECLE(char16_t,w16execvpe,__path,__args))
__XREDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__XATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((1)),int,__ATTR_CDECL,w32execlpe,(char32_t const *__restrict __file, char32_t const *__args, ...),wexeclpe,__REDIRECT_EXECLE(char32_t,w32execvpe,__path,__args))
__REDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,w16spawnv,(int __mode, char16_t const *__restrict __path, __TW16ARGV),wspawnv,(__mode,__path,___argv))
__REDIRECT_EXCEPT_UFSW32  (__LIBC,__PORT_KOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,w32spawnv,(int __mode, char32_t const *__restrict __path, __TW32ARGV),wspawnv,(__mode,__path,___argv))
__REDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_KOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,w16spawnve,(int __mode, char16_t const *__restrict __path, __TW16ARGV, __TW16ENVP),wspawnve,(__mode,__path,___argv,___envp))
__REDIRECT_EXCEPT_UFSW32  (__LIBC,__PORT_KOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,w32spawnve,(int __mode, char32_t const *__restrict __path, __TW32ARGV, __TW32ENVP),wspawnve,(__mode,__path,___argv,___envp))
__REDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,w16spawnvp,(int __mode, char16_t const *__restrict __file, __TW16ARGV),wspawnvp,(__mode,__file,___argv))
__REDIRECT_EXCEPT_UFSW32  (__LIBC,__PORT_KOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,w32spawnvp,(int __mode, char32_t const *__restrict __file, __TW32ARGV),wspawnvp,(__mode,__file,___argv))
__REDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_KOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,w16spawnvpe,(int __mode, char16_t const *__restrict __file, __TW16ARGV, __TW16ENVP),wspawnvpe,(__mode,__file,___argv,___envp))
__REDIRECT_EXCEPT_UFSW32  (__LIBC,__PORT_KOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,w32spawnvpe,(int __mode, char32_t const *__restrict __file, __TW32ARGV, __TW32ENVP),wspawnvpe,(__mode,__file,___argv,___envp))
__XREDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL __NONNULL((2)),pid_t,__ATTR_CDECL,w16spawnl,(int __mode, char16_t const *__restrict __path, char16_t const *__args, ...),wspawnl,__REDIRECT_SPAWNL(char16_t,w16spawnv,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSW32  (__LIBC,__PORT_KOSONLY __ATTR_SENTINEL __NONNULL((2)),pid_t,__ATTR_CDECL,w32spawnl,(int __mode, char32_t const *__restrict __path, char32_t const *__args, ...),wspawnl,__REDIRECT_SPAWNL(char32_t,w32spawnv,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((2)),pid_t,__ATTR_CDECL,w16spawnle,(int __mode, char16_t const *__restrict __path, char16_t const *__args, ...),wspawnle,__REDIRECT_SPAWNLE(char16_t,w16spawnve,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSW32  (__LIBC,__PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((2)),pid_t,__ATTR_CDECL,w32spawnle,(int __mode, char32_t const *__restrict __path, char32_t const *__args, ...),wspawnle,__REDIRECT_SPAWNLE(char32_t,w32spawnve,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL __NONNULL((2)),pid_t,__ATTR_CDECL,w16spawnlp,(int __mode, char16_t const *__restrict __file, char16_t const *__args, ...),wspawnlp,__REDIRECT_SPAWNL(char16_t,w16spawnvp,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSW32  (__LIBC,__PORT_KOSONLY __ATTR_SENTINEL __NONNULL((2)),pid_t,__ATTR_CDECL,w32spawnlp,(int __mode, char32_t const *__restrict __file, char32_t const *__args, ...),wspawnlp,__REDIRECT_SPAWNL(char32_t,w32spawnvp,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((2)),pid_t,__ATTR_CDECL,w16spawnlpe,(int __mode, char16_t const *__restrict __file, char16_t const *__args, ...),wspawnlpe,__REDIRECT_SPAWNLE(char16_t,w16spawnvpe,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSW32  (__LIBC,__PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((2)),pid_t,__ATTR_CDECL,w32spawnlpe,(int __mode, char32_t const *__restrict __file, char32_t const *__args, ...),wspawnlpe,__REDIRECT_SPAWNLE(char32_t,w32spawnvpe,__mode,__path,__args))

#ifdef __USE_EXCEPT
__REDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw16execv,(char16_t const *__restrict __path, __TW16ARGV),Xwexecv,(__path,___argv))
__REDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw32execv,(char32_t const *__restrict __path, __TW32ARGV),Xwexecv,(__path,___argv))
__REDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2,3)),__LIBCCALL,Xw16execve,(char16_t const *__restrict __path, __TW16ARGV, __TW16ENVP),Xwexecve,(__path,___argv,___envp))
__REDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2,3)),__LIBCCALL,Xw32execve,(char32_t const *__restrict __path, __TW32ARGV, __TW32ENVP),Xwexecve,(__path,___argv,___envp))
__REDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw16execvp,(char16_t const *__restrict __file, __TW16ARGV),Xwexecvp,(__file,___argv))
__REDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw32execvp,(char32_t const *__restrict __file, __TW32ARGV),Xwexecvp,(__file,___argv))
__REDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2,3)),__LIBCCALL,Xw16execvpe,(char16_t const *__restrict __file, __TW16ARGV, __TW16ENVP),Xwexecvpe,(__file,___argv,___envp))
__REDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __NONNULL((1,2,3)),__LIBCCALL,Xw32execvpe,(char32_t const *__restrict __file, __TW32ARGV, __TW32ENVP),Xwexecvpe,(__file,___argv,___envp))
__XREDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL __NONNULL((1)),__ATTR_CDECL,Xw16execl,(char16_t const *__restrict __path, char16_t const *__args, ...),Xwexecl,__REDIRECT_XEXECL(char16_t,Xw16execv,__path,__args))
__XREDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL __NONNULL((1)),__ATTR_CDECL,Xw32execl,(char32_t const *__restrict __path, char32_t const *__args, ...),Xwexecl,__REDIRECT_XEXECL(char32_t,Xw32execv,__path,__args))
__XREDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((1)),__ATTR_CDECL,Xw16execle,(char16_t const *__restrict __path, char16_t const *__args, ...),Xwexecle,__REDIRECT_XEXECLE(char16_t,Xw16execve,__path,__args))
__XREDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((1)),__ATTR_CDECL,Xw32execle,(char32_t const *__restrict __path, char32_t const *__args, ...),Xwexecle,__REDIRECT_XEXECLE(char32_t,Xw32execve,__path,__args))
__XREDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL __NONNULL((1)),__ATTR_CDECL,Xw16execlp,(char16_t const *__restrict __file, char16_t const *__args, ...),Xwexeclp,__REDIRECT_XEXECL(char16_t,Xw16execvp,__path,__args))
__XREDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL __NONNULL((1)),__ATTR_CDECL,Xw32execlp,(char32_t const *__restrict __file, char32_t const *__args, ...),Xwexeclp,__REDIRECT_XEXECL(char32_t,Xw32execvp,__path,__args))
__XREDIRECT_W16_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((1)),__ATTR_CDECL,Xw16execlpe,(char16_t const *__restrict __file, char16_t const *__args, ...),Xwexeclpe,__REDIRECT_XEXECLE(char16_t,Xw16execvpe,__path,__args))
__XREDIRECT_W32_VOID(__LIBC,__ATTR_NORETURN __PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((1)),__ATTR_CDECL,Xw32execlpe,(char32_t const *__restrict __file, char32_t const *__args, ...),Xwexeclpe,__REDIRECT_XEXECLE(char32_t,Xw32execvpe,__path,__args))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,Xw16spawnv,(int __mode, char16_t const *__restrict __path, __TW16ARGV),Xwspawnv,(__mode,__path,___argv))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,Xw32spawnv,(int __mode, char32_t const *__restrict __path, __TW32ARGV),Xwspawnv,(__mode,__path,___argv))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,Xw16spawnve,(int __mode, char16_t const *__restrict __path, __TW16ARGV, __TW16ENVP),Xwspawnve,(__mode,__path,___argv,___envp))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,Xw32spawnve,(int __mode, char32_t const *__restrict __path, __TW32ARGV, __TW32ENVP),Xwspawnve,(__mode,__path,___argv,___envp))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,Xw16spawnvp,(int __mode, char16_t const *__restrict __file, __TW16ARGV),Xwspawnvp,(__mode,__file,___argv))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,Xw32spawnvp,(int __mode, char32_t const *__restrict __file, __TW32ARGV),Xwspawnvp,(__mode,__file,___argv))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,Xw16spawnvpe,(int __mode, char16_t const *__restrict __file, __TW16ARGV, __TW16ENVP),Xwspawnvpe,(__mode,__file,___argv,___envp))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,Xw32spawnvpe,(int __mode, char32_t const *__restrict __file, __TW32ARGV, __TW32ENVP),Xwspawnvpe,(__mode,__file,___argv,___envp))
__XREDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL __NONNULL((2)),pid_t,__ATTR_CDECL,Xw16spawnl,(int __mode, char16_t const *__restrict __path, char16_t const *__args, ...),Xwspawnl,__REDIRECT_XSPAWNL(char16_t,Xw16spawnv,__mode,__path,__args))
__XREDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL __NONNULL((2)),pid_t,__ATTR_CDECL,Xw32spawnl,(int __mode, char32_t const *__restrict __path, char32_t const *__args, ...),Xwspawnl,__REDIRECT_XSPAWNL(char32_t,Xw32spawnv,__mode,__path,__args))
__XREDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((2)),pid_t,__ATTR_CDECL,Xw16spawnle,(int __mode, char16_t const *__restrict __path, char16_t const *__args, ...),Xwspawnle,__REDIRECT_XSPAWNLE(char16_t,Xw16spawnve,__mode,__path,__args))
__XREDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((2)),pid_t,__ATTR_CDECL,Xw32spawnle,(int __mode, char32_t const *__restrict __path, char32_t const *__args, ...),Xwspawnle,__REDIRECT_XSPAWNLE(char32_t,Xw32spawnve,__mode,__path,__args))
__XREDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL __NONNULL((2)),pid_t,__ATTR_CDECL,Xw16spawnlp,(int __mode, char16_t const *__restrict __file, char16_t const *__args, ...),Xwspawnlp,__REDIRECT_XSPAWNL(char16_t,Xw16spawnvp,__mode,__path,__args))
__XREDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL __NONNULL((2)),pid_t,__ATTR_CDECL,Xw32spawnlp,(int __mode, char32_t const *__restrict __file, char32_t const *__args, ...),Xwspawnlp,__REDIRECT_XSPAWNL(char32_t,Xw32spawnvp,__mode,__path,__args))
__XREDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((2)),pid_t,__ATTR_CDECL,Xw16spawnlpe,(int __mode, char16_t const *__restrict __file, char16_t const *__args, ...),Xwspawnlpe,__REDIRECT_XSPAWNLE(char16_t,Xw16spawnvpe,__mode,__path,__args))
__XREDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((2)),pid_t,__ATTR_CDECL,Xw32spawnlpe,(int __mode, char32_t const *__restrict __file, char32_t const *__args, ...),Xwspawnlpe,__REDIRECT_XSPAWNLE(char32_t,Xw32spawnvpe,__mode,__path,__args))
#endif /* __USE_EXCEPT */

#endif /* __CRT_KOS */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_PARTS_KOS2_UPROCESS_H */
