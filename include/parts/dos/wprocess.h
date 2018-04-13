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
#ifndef _PARTS_DOS_WPROCESS_H
#define _PARTS_DOS_WPROCESS_H 1

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

#ifndef __intptr_t_defined
#define __intptr_t_defined 1
typedef __intptr_t intptr_t;
#endif /* !__intptr_t_defined */

#ifdef __USE_DOS
#   define __TWARGV wchar_t const *const *__restrict ___argv
#   define __TWENVP wchar_t const *const *__restrict ___envp
#else
#   define __TWARGV wchar_t *const ___argv[__restrict_arr]
#   define __TWENVP wchar_t *const ___envp[__restrict_arr]
#endif

#ifndef _WPROCESS_DEFINED
#define _WPROCESS_DEFINED 1
#ifdef __CRT_DOS
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,wexecv,(wchar_t const *__restrict __path, __TWARGV),(__path,___argv))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1,2,3)),intptr_t,__LIBCCALL,wexecve,(wchar_t const *__restrict __path, __TWARGV, __TWENVP),(__path,___argv,___envp))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),intptr_t,__LIBCCALL,wexecvp,(wchar_t const *__restrict __file, __TWARGV),(__file,___argv))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1,2,3)),intptr_t,__LIBCCALL,wexecvpe,(wchar_t const *__restrict __file, __TWARGV, __TWENVP),(__file,___argv,___envp))
__XREDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_DOSONLY __ATTR_SENTINEL __NONNULL((1)),intptr_t,__ATTR_CDECL,wexecl,(wchar_t const *__restrict __path, wchar_t const *__args, ...),__REDIRECT_EXECL(wchar_t,_wexecv,__path,__args))
__XREDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_DOSONLY __ATTR_SENTINEL_O(1) __NONNULL((1)),intptr_t,__ATTR_CDECL,wexecle,(wchar_t const *__restrict __path, wchar_t const *__args, ...),__REDIRECT_EXECLE(wchar_t,_wexecve,__path,__args))
__XREDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_DOSONLY __ATTR_SENTINEL __NONNULL((1)),intptr_t,__ATTR_CDECL,wexeclp,(wchar_t const *__restrict __file, wchar_t const *__args, ...),__REDIRECT_EXECL(wchar_t,_wexecvp,__path,__args))
__XREDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_DOSONLY __ATTR_SENTINEL_O(1) __NONNULL((1)),intptr_t,__ATTR_CDECL,wexeclpe,(wchar_t const *__restrict __file, wchar_t const *__args, ...),__REDIRECT_EXECLE(wchar_t,_wexecvpe,__path,__args))
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((2,3)),intptr_t,__LIBCCALL,wspawnv,(int __mode, wchar_t const *__restrict __path, __TWARGV),(__mode,__path,___argv))
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((2,3,4)),intptr_t,__LIBCCALL,wspawnve,(int __mode, wchar_t const *__restrict __path, __TWARGV, __TWENVP),(__mode,__path,___argv,___envp))
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((2,3)),intptr_t,__LIBCCALL,wspawnvp,(int __mode, wchar_t const *__restrict __file, __TWARGV),(__mode,__file,___argv))
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((2,3,4)),intptr_t,__LIBCCALL,wspawnvpe,(int __mode, wchar_t const *__restrict __file, __TWARGV, __TWENVP),(__mode,__file,___argv,___envp))
__XREDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __ATTR_SENTINEL __NONNULL((2)),intptr_t,__ATTR_CDECL,wspawnl,(int __mode, wchar_t const *__restrict __path, wchar_t const *__args, ...),__REDIRECT_SPAWNL(wchar_t,_wspawnv,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __ATTR_SENTINEL_O(1) __NONNULL((2)),intptr_t,__ATTR_CDECL,wspawnle,(int __mode, wchar_t const *__restrict __path, wchar_t const *__args, ...),__REDIRECT_SPAWNLE(wchar_t,_wspawnve,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __ATTR_SENTINEL __NONNULL((2)),intptr_t,__ATTR_CDECL,wspawnlp,(int __mode, wchar_t const *__restrict __file, wchar_t const *__args, ...),__REDIRECT_SPAWNL(wchar_t,_wspawnvp,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __ATTR_SENTINEL_O(1) __NONNULL((2)),intptr_t,__ATTR_CDECL,wspawnlpe,(int __mode, wchar_t const *__restrict __file, wchar_t const *__args, ...),__REDIRECT_SPAWNLE(wchar_t,_wspawnvpe,__mode,__path,__args))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1,2)) void (__LIBCCALL Xwexecv)(wchar_t const *__restrict __path, __TWARGV);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1,2,3)) void (__LIBCCALL Xwexecve)(wchar_t const *__restrict __path, __TWARGV, __TWENVP);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1,2)) void (__LIBCCALL Xwexecvp)(wchar_t const *__restrict __file, __TWARGV);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1,2,3)) void (__LIBCCALL Xwexecvpe)(wchar_t const *__restrict __file, __TWARGV, __TWENVP);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __ATTR_SENTINEL __NONNULL((1)) void (__ATTR_CDECL Xwexecl)(wchar_t const *__restrict __path, wchar_t const *__args, ...);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __ATTR_SENTINEL_O(1) __NONNULL((1)) void (__ATTR_CDECL Xwexecle)(wchar_t const *__restrict __path, wchar_t const *__args, ...);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __ATTR_SENTINEL __NONNULL((1)) void (__ATTR_CDECL Xwexeclp)(wchar_t const *__restrict __file, wchar_t const *__args, ...);
__LIBC __PORT_KOSONLY __ATTR_NORETURN __ATTR_SENTINEL_O(1) __NONNULL((1)) void (__ATTR_CDECL Xwexeclpe)(wchar_t const *__restrict __file, wchar_t const *__args, ...);
__LIBC __PORT_KOSONLY __NONNULL((2,3)) pid_t (__LIBCCALL Xwspawnv)(int __mode, wchar_t const *__restrict __path, __TWARGV);
__LIBC __PORT_KOSONLY __NONNULL((2,3,4)) pid_t (__LIBCCALL Xwspawnve)(int __mode, wchar_t const *__restrict __path, __TWARGV, __TWENVP);
__LIBC __PORT_KOSONLY __NONNULL((2,3)) pid_t (__LIBCCALL Xwspawnvp)(int __mode, wchar_t const *__restrict __file, __TWARGV);
__LIBC __PORT_KOSONLY __NONNULL((2,3,4)) pid_t (__LIBCCALL Xwspawnvpe)(int __mode, wchar_t const *__restrict __file, __TWARGV, __TWENVP);
__LIBC __PORT_KOSONLY __ATTR_SENTINEL __NONNULL((2)) pid_t (__ATTR_CDECL Xwspawnl)(int __mode, wchar_t const *__restrict __path, wchar_t const *__args, ...);
__LIBC __PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((2)) pid_t (__ATTR_CDECL Xwspawnle)(int __mode, wchar_t const *__restrict __path, wchar_t const *__args, ...);
__LIBC __PORT_KOSONLY __ATTR_SENTINEL __NONNULL((2)) pid_t (__ATTR_CDECL Xwspawnlp)(int __mode, wchar_t const *__restrict __file, wchar_t const *__args, ...);
__LIBC __PORT_KOSONLY __ATTR_SENTINEL_O(1) __NONNULL((2)) pid_t (__ATTR_CDECL Xwspawnlpe)(int __mode, wchar_t const *__restrict __file, wchar_t const *__args, ...);
#endif /* __USE_EXCEPT */
#endif /* __CRT_DOS */
#endif /* !_WPROCESS_DEFINED */

#ifndef _CRT_WSYSTEM_DEFINED
#define _CRT_WSYSTEM_DEFINED 1
#ifdef __CRT_DOS
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,wsystem,(wchar_t const *__restrict __cmd),(__cmd))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY int (__LIBCCALL Xwsystem)(wchar_t const *__restrict __cmd);
#endif /* __USE_EXCEPT */
#endif /* __CRT_DOS */
#endif /* !_CRT_WSYSTEM_DEFINED */

__SYSDECL_END

#endif /* !_PARTS_DOS_WPROCESS_H */
