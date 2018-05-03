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
#ifndef _PARTS_DOS_PROCESS_H
#define _PARTS_DOS_PROCESS_H 1

#include "__stdinc.h"
#include <features.h>
#ifndef _PROCESS_H
#include <process.h>
#endif

__SYSDECL_BEGIN

#define _P_WAIT          P_WAIT
#define _P_NOWAIT        P_NOWAIT
#define _P_OVERLAY       P_OVERLAY
#define _P_NOWAITO       P_NOWAITO
#define _P_DETACH        P_DETACH
#define _OLD_P_OVERLAY   P_OVERLAY
#define _WAIT_CHILD      WAIT_CHILD
#define _WAIT_GRANDCHILD WAIT_GRANDCHILD
#define OLD_P_OVERLAY    _OLD_P_OVERLAY


#ifndef __intptr_t_defined
#define __intptr_t_defined 1
typedef __intptr_t   intptr_t;
#endif /* !__intptr_t_defined */
#ifndef __uintptr_t_defined
#define __uintptr_t_defined 1
typedef __uintptr_t   uintptr_t;
#endif /* !__uintptr_t_defined */
#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */


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


#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY uintptr_t (__LIBCCALL _beginthread)(void (__LIBCCALL *__entry)(void *__arg), __UINT32_TYPE__ __stacksz, void *__arg);
__LIBC __PORT_DOSONLY uintptr_t (__LIBCCALL _beginthreadex)(void *__sec, __UINT32_TYPE__ __stacksz, __UINT32_TYPE__ (__ATTR_STDCALL *__entry)(void *__arg), void *__arg, __UINT32_TYPE__ __flags, __UINT32_TYPE__ *__threadaddr);
__LIBC __PORT_DOSONLY void (__LIBCCALL _endthread)(void);
__LIBC __PORT_DOSONLY void (__LIBCCALL _endthreadex)(__UINT32_TYPE__ __exitcode);
#endif /* __CRT_DOS */

#ifndef _CRT_TERMINATE_DEFINED
#define _CRT_TERMINATE_DEFINED 1
#ifndef __std_exit_defined
#define __std_exit_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_NORETURN void (__LIBCCALL exit)(int __status);
__NAMESPACE_STD_END
#endif /* !__std_exit_defined */
#ifndef __std_abort_defined
#define __std_abort_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_NORETURN void (__LIBCCALL abort)(void);
__NAMESPACE_STD_END
#endif /* !__std_abort_defined */

#ifndef __CXX_SYSTEM_HEADER
#ifndef __exit_defined
#define __exit_defined 1
__NAMESPACE_STD_USING(exit)
#endif /* !__exit_defined */
#ifndef __abort_defined
#define __abort_defined 1
__NAMESPACE_STD_USING(abort)
#endif /* !__abort_defined */
#endif /* !__CXX_SYSTEM_HEADER */

#ifndef ___exit_defined
#define ___exit_defined 1
__LIBC __ATTR_NORETURN void (__LIBCCALL _exit)(int __status);
#endif /* !___exit_defined */
#endif /* !_CRT_TERMINATE_DEFINED */

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY void (__LIBCCALL _cexit)(void);
__LIBC __PORT_DOSONLY void (__LIBCCALL _c_exit)(void);
#endif /* __CRT_DOS */
__REDIRECT_DPB(__LIBC,__WUNUSED,__pid_t,__LIBCCALL,getpid,(void),())

__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1,2)),intptr_t,__LIBCCALL,execv,(char const *__path, __TARGV),(__path,___argv))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1,2,3)),intptr_t,__LIBCCALL,execve,(char const *__path, __TARGV, __TENVP),(__path,___argv,___envp))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1,2)),intptr_t,__LIBCCALL,execvp,(char const *__file, __TARGV),(__file,___argv))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1,2,3)),intptr_t,__LIBCCALL,execvpe,(char const *__file, __TARGV, __TENVP),(__file,___argv,___envp))
__XREDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1)) __ATTR_SENTINEL,intptr_t,__LIBCCALL,execl,(char const *__path, char const *__args, ...),__REDIRECT_EXECL(char,execv,__path,__args))
__XREDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1)) __ATTR_SENTINEL_O(1),intptr_t,__LIBCCALL,execle,(char const *__path, char const *__args, ...),__REDIRECT_EXECLE(char,execve,__path,__args))
__XREDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1)) __ATTR_SENTINEL,intptr_t,__LIBCCALL,execlp,(char const *__file, char const *__args, ...),__REDIRECT_EXECL(char,execvp,__path,__args))
__XREDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1)) __ATTR_SENTINEL_O(1),intptr_t,__LIBCCALL,execlpe,(char const *__file, char const *__args, ...),__REDIRECT_EXECLE(char,execvpe,__path,__args))

#ifdef __CRT_DOS
__REDIRECT_EXCEPT_DPB(__LIBC,__PORT_DOSONLY,intptr_t,__LIBCCALL,cwait,(int *__tstat, intptr_t __pid, int __action),(__tstat,__pid,__action))
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((2,3)),intptr_t,__LIBCCALL,spawnv,(int __mode, char const *__path, __TARGV),(__mode,__path,___argv))
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((2,3,4)),intptr_t,__LIBCCALL,spawnve,(int __mode, char const *__path, __TARGV, __TENVP),(__mode,__path,___argv,___envp))
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((2,3)),intptr_t,__LIBCCALL,spawnvp,(int __mode, char const *__file, __TARGV),(__mode,__file,___argv));
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((2,3,4)),intptr_t,__LIBCCALL,spawnvpe,(int __mode, char const *__file, __TARGV, __TENVP),(__mode,__file,___argv,___envp));
__XREDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((2)) __ATTR_SENTINEL,intptr_t,__LIBCCALL,spawnl,(int __mode, char const *__restrict __path, char const *__args, ...),__REDIRECT_SPAWNL(char,_spawnv,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),intptr_t,__LIBCCALL,spawnle,(int __mode, char const *__restrict __path, char const *__args, ...),__REDIRECT_SPAWNLE(char,_spawnve,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((2)) __ATTR_SENTINEL,intptr_t,__LIBCCALL,spawnlp,(int __mode, char const *__restrict __file, char const *__args, ...),__REDIRECT_SPAWNL(char,_spawnvp,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),intptr_t,__LIBCCALL,spawnlpe,(int __mode, char const *__restrict __file, char const *__args, ...),__REDIRECT_SPAWNLE(char,_spawnvpe,__mode,__path,__args))
#endif /* __CRT_DOS */

#ifndef __std_system_defined
#define __std_system_defined 1
__NAMESPACE_STD_BEGIN
__REDIRECT_EXCEPT_UFS(__LIBC,,int,__LIBCCALL,system,(char const *__command),(__command))
__NAMESPACE_STD_END
#endif /* !__std_system_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __system_defined
#define __system_defined 1
__NAMESPACE_STD_USING(system)
#endif /* !__system_defined */
#endif /* !__CXX_SYSTEM_HEADER */

#ifndef __getpid_defined
#define __getpid_defined 1
__REDIRECT_DPA(__LIBC,__WUNUSED,__pid_t,__LIBCCALL,getpid,(void),())
#endif /* !__getpid_defined */

#ifndef __execl_defined
#define __execl_defined 1
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1,2)),int,__LIBCCALL,execv,(char const *__restrict __path, __TARGV),(__path,___argv))
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1,2,3)),int,__LIBCCALL,execve,(char const *__restrict __path, __TARGV, __TENVP),(__path,___argv,___envp))
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1,2)),int,__LIBCCALL,execvp,(char const *__restrict __file, __TARGV),(__path,___argv))
__XREDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL,int,__ATTR_CDECL,execl,(char const *__restrict __path, char const *__args, ...),__REDIRECT_EXECL(char,execv,__path,__args))
__XREDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(1),int,__ATTR_CDECL,execle,(char const *__restrict __path, char const *__args, ...),__REDIRECT_EXECLE(char,execve,__path,__args))
__XREDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL,int,__ATTR_CDECL,execlp,(char const *__restrict __file, char const *__args, ...),__REDIRECT_EXECL(char,execvp,__path,__args))
#ifdef __USE_EXCEPT
__LIBC __ATTR_NORETURN __NONNULL((1,2)) void (__LIBCCALL Xexecv)(char const *__restrict __path, __TARGV);
__LIBC __ATTR_NORETURN __NONNULL((1,2,3)) void (__LIBCCALL Xexecve)(char const *__restrict __path, __TARGV, __TENVP);
__LIBC __ATTR_NORETURN __NONNULL((1,2)) void (__LIBCCALL Xexecvp)(char const *__restrict __file, __TARGV);
__LIBC __ATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL void (__ATTR_CDECL Xexecl)(char const *__restrict __path, char const *__args, ...);
__LIBC __ATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(1) void (__ATTR_CDECL Xexecle)(char const *__restrict __path, char const *__args, ...);
__LIBC __ATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL void (__ATTR_CDECL Xexeclp)(char const *__restrict __file, char const *__args, ...);
#endif /* __USE_EXCEPT */
#endif /* !__execl_defined */
#ifndef __execvpe_defined
#define __execvpe_defined 1
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1,2,3)),int,__LIBCCALL,execvpe,(char const *__file, __TARGV, __TENVP),(__file,___argv,___envp))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1,2,3)) void (__LIBCCALL Xexecvpe)(char const *__file, __TARGV, __TENVP);
#endif /* __USE_EXCEPT */
#endif /* !__execvpe_defined */
#ifndef __execlpe_defined
#define __execlpe_defined 1
__XREDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(1),int,__LIBCCALL,execlpe,(char const *__restrict __file, char const *__args, ...),__REDIRECT_EXECLE(char,execvpe,__file,__args))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(1) void (__LIBCCALL Xexeclpe)(char const *__restrict __file, char const *__args, ...);
#endif /* __USE_EXCEPT */
#endif /* !__execlpe_defined */

#ifdef __CRT_DOS
#ifndef __cwait_defined
#define __cwait_defined 1
__REDIRECT_EXCEPT_DPA(__LIBC,__PORT_DOSONLY,intptr_t,__LIBCCALL,cwait,(int *__tstat, intptr_t __pid, int __action),(__tstat,__pid,__action))
#ifdef __USE_EXCEPT
__LIBC __PORT_DOSONLY pid_t (__LIBCCALL Xcwait)(int *__tstat, pid_t __pid, int __action);
#endif /* __USE_EXCEPT */
#endif /* !__cwait_defined */
#ifndef __spawnv_defined
#define __spawnv_defined 1
__REDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2,3)),intptr_t,__LIBCCALL,spawnv,(int __mode, char const *__restrict __path, __TARGV),(__mode,__path,___argv))
__REDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2,3,4)),intptr_t,__LIBCCALL,spawnve,(int __mode, char const *__restrict __path, __TARGV, __TENVP),(__mode,__path,___argv,___envp))
__REDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2,3)),intptr_t,__LIBCCALL,spawnvp,(int __mode, char const *__restrict __file, __TARGV),(__mode,__file,___argv))
__REDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2,3,4)),intptr_t,__LIBCCALL,spawnvpe,(int __mode, char const *__restrict __file, __TARGV, __TENVP),(__mode,__file,___argv,___envp))
__XREDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2)) __ATTR_SENTINEL,intptr_t,__LIBCCALL,spawnl,(int __mode, char const *__restrict __path, char const *__args, ...),__REDIRECT_SPAWNL(char,spawnv,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),intptr_t,__LIBCCALL,spawnle,(int __mode, char const *__restrict __path, char const *__args, ...),__REDIRECT_SPAWNLE(char,spawnve,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2)) __ATTR_SENTINEL,intptr_t,__LIBCCALL,spawnlp,(int __mode, char const *__restrict __file, char const *__args, ...),__REDIRECT_SPAWNL(char,spawnvp,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),intptr_t,__LIBCCALL,spawnlpe,(int __mode, char const *__restrict __file, char const *__args, ...),__REDIRECT_SPAWNLE(char,spawnvpe,__mode,__path,__args))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __NONNULL((2,3)) pid_t (__LIBCCALL Xspawnv)(int __mode, char const *__restrict __path, __TARGV);
__LIBC __PORT_KOSONLY __NONNULL((2,3,4)) pid_t (__LIBCCALL Xspawnve)(int __mode, char const *__restrict __path, __TARGV, __TENVP);
__LIBC __PORT_KOSONLY __NONNULL((2,3)) pid_t (__LIBCCALL Xspawnvp)(int __mode, char const *__restrict __file, __TARGV);
__LIBC __PORT_KOSONLY __NONNULL((2,3,4)) pid_t (__LIBCCALL Xspawnvpe)(int __mode, char const *__restrict __file, __TARGV, __TENVP);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL pid_t (__LIBCCALL Xspawnl)(int __mode, char const *__restrict __path, char const *__args, ...);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1) pid_t (__LIBCCALL Xspawnle)(int __mode, char const *__restrict __path, char const *__args, ...);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL pid_t (__LIBCCALL Xspawnlp)(int __mode, char const *__restrict __file, char const *__args, ...);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1) pid_t (__LIBCCALL Xspawnlpe)(int __mode, char const *__restrict __file, char const *__args, ...);
#endif /* __USE_EXCEPT */
#endif /* !__spawnv_defined */
#endif /* __CRT_DOS */

#ifdef __CRT_DOS

#ifdef __DOS_COMPAT__
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,intptr_t,__LIBCCALL,loaddll,(char *__file),(__file))
__LIBC __PORT_DOSONLY int (__LIBCCALL _unloaddll)(intptr_t __hnd);
#elif defined(__CRT_KOS)
__REDIRECT_UFS_(__LIBC,__PORT_DOSONLY,intptr_t,__LIBCCALL,__xdlopen,(char *__file, int __flags),xdlopen,(__file))
__LOCAL __PORT_DOSONLY intptr_t (__LIBCCALL _loaddll)(char *__file) { return __xdlopen(__file,0); }
__REDIRECT(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_unloaddll,(intptr_t __hnd),xdlclose,(__hnd))
#endif

#ifdef __USE_KOS
#define __FIXED_CONST const
#else
#define __FIXED_CONST /* nothing (*sigh*) */
#endif
typedef int (__LIBCCALL *__procfun)(void);
#ifdef __DOS_COMPAT__
__LIBC __PORT_DOSONLY __procfun (__LIBCCALL _getdllprocaddr)(intptr_t __hnd, char __FIXED_CONST *__symname, intptr_t __ord);
#elif defined(__CRT_KOS)
__REDIRECT(__LIBC,__XATTR_RETNONNULL __PORT_DOSONLY,__procfun,__LIBCCALL,
           _getdllprocaddr,(intptr_t __hnd, char __FIXED_CONST *__symname, intptr_t __ord),
           xdlsym,(__hnd,__symname,__ord))
#endif
#undef __FIXED_CONST
#endif /* __CRT_DOS */

#endif /* !__KERNEL__ */

__SYSDECL_END

#ifndef _PARTS_DOS_WPROCESS_H
#include "wprocess.h"
#endif

#endif /* !_PARTS_DOS_PROCESS_H */
