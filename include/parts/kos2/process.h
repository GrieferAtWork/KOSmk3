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
#ifndef _PARTS_KOS2_PROCESS_H
#define _PARTS_KOS2_PROCESS_H 1

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

#ifndef __execl_defined
#define __execl_defined 1

/* >> execv(3), execve(3), execvp(3), execl(3), execle(3), execlp(3)
 * Replace the calling process with the application image referred to by `PATH' / `FILE'
 * and execute it's `main()' method, passing the given `ARGV', and setting `environ' to `ENVP' */
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1,2)),int,__LIBCCALL,execv,(char const *__restrict __path, __TARGV),(__path,___argv))
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1,2,3)),int,__LIBCCALL,execve,(char const *__restrict __path, __TARGV, __TENVP),(__path,___argv,___envp))
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1,2)),int,__LIBCCALL,execvp,(char const *__restrict __file, __TARGV),(__path,___argv))
__XREDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL,int,__ATTR_CDECL,execl,(char const *__restrict __path, char const *__args, ...),__REDIRECT_EXECL(char,execv,__path,__args))
__XREDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(1),int,__ATTR_CDECL,execle,(char const *__restrict __path, char const *__args, ...),__REDIRECT_EXECLE(char,execve,__path,__args))
__XREDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL,int,__ATTR_CDECL,execlp,(char const *__restrict __file, char const *__args, ...),__REDIRECT_EXECL(char,execvp,__path,__args))

#ifdef __USE_EXCEPT
/* >> execv(3), execve(3), execvp(3), execl(3), execle(3), execlp(3)
 * Replace the calling process with the application image referred to by `PATH' / `FILE'
 * and execute it's `main()' method, passing the given `ARGV', and setting `environ' to `ENVP' */
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
/* >> execvpe(3)
 * Replace the calling process with the application image referred to by `FILE'
 * and execute it's `main()' method, passing the given `ARGV', and setting `environ' to `ENVP' */
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1,2,3)),int,__LIBCCALL,execvpe,(char const *__file, __TARGV, __TENVP),(__file,___argv,___envp))

#ifdef __USE_EXCEPT
/* >> execvpe(3)
 * Replace the calling process with the application image referred to by `FILE'
 * and execute it's `main()' method, passing the given `ARGV', and setting `environ' to `ENVP' */
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1,2,3)) void (__LIBCCALL Xexecvpe)(char const *__file, __TARGV, __TENVP);
#endif /* __USE_EXCEPT */
#endif /* !__execvpe_defined */

#ifndef __execlpe_defined
#define __execlpe_defined 1

/* >> execlpe(3)
 * Replace the calling process with the application image referred to by `FILE'
 * and execute it's `main()' method, passing the given `ARGV', and setting `environ' to `ENVP' */
__XREDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__XATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(1),int,__LIBCCALL,execlpe,(char const *__restrict __file, char const *__args, ...),__REDIRECT_EXECLE(char,execvpe,__file,__args))

#ifdef __USE_EXCEPT
/* >> execlpe(3)
 * Replace the calling process with the application image referred to by `FILE'
 * and execute it's `main()' method, passing the given `ARGV', and setting `environ' to `ENVP' */
__LIBC __PORT_KOSONLY __ATTR_NORETURN __NONNULL((1)) __ATTR_SENTINEL_O(1) void (__LIBCCALL Xexeclpe)(char const *__restrict __file, char const *__args, ...);
#endif /* __USE_EXCEPT */
#endif /* !__execlpe_defined */

#ifdef __CRT_DOS
#ifndef __cwait_defined
#define __cwait_defined 1

/* >> cwait(3)
 * Same as `waitpid(PID,TSTAT,WEXITED)'. The `ACTION' argument is ignored.
 * s.a. `waitpid(3)' */
__REDIRECT_EXCEPT_DPA(__LIBC,__PORT_DOSONLY,pid_t,__LIBCCALL,cwait,(int *__tstat, pid_t __pid, int __action),(__tstat,__pid,__action))

#ifdef __USE_EXCEPT
/* >> cwait(3)
 * Same as `waitpid(PID,TSTAT,WEXITED)'. The `ACTION' argument is ignored.
 * s.a. `waitpid(3)' */
__LIBC __PORT_DOSONLY pid_t (__LIBCCALL Xcwait)(int *__tstat, pid_t __pid, int __action);
#endif /* __USE_EXCEPT */
#endif /* !__cwait_defined */

#ifndef __spawnv_defined
#define __spawnv_defined 1

/* >> spawnv(3), spawnve(3), spawnvp(3), spawnvpe(3), spawnl(3), spawnle(3), spawnlp(3), spawnlpe(3)
 * @param: mode: One of `P_*' (e.g. `P_WAIT')
 * A combination of `fork(2)' and `exec(2)' that can be used
 * to create a new process and load it with a new application. */
__REDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,spawnv,(int __mode, char const *__restrict __path, __TARGV),(__mode,__path,___argv))
__REDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,spawnve,(int __mode, char const *__restrict __path, __TARGV, __TENVP),(__mode,__path,___argv,___envp))
__REDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2,3)),pid_t,__LIBCCALL,spawnvp,(int __mode, char const *__restrict __file, __TARGV),(__mode,__file,___argv))
__REDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2,3,4)),pid_t,__LIBCCALL,spawnvpe,(int __mode, char const *__restrict __file, __TARGV, __TENVP),(__mode,__file,___argv,___envp))
__XREDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2)) __ATTR_SENTINEL,pid_t,__LIBCCALL,spawnl,(int __mode, char const *__restrict __path, char const *__args, ...),__REDIRECT_SPAWNL(char,spawnv,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),pid_t,__LIBCCALL,spawnle,(int __mode, char const *__restrict __path, char const *__args, ...),__REDIRECT_SPAWNLE(char,spawnve,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2)) __ATTR_SENTINEL,pid_t,__LIBCCALL,spawnlp,(int __mode, char const *__restrict __file, char const *__args, ...),__REDIRECT_SPAWNL(char,spawnvp,__mode,__path,__args))
__XREDIRECT_EXCEPT_UFSDPA(__LIBC,__PORT_DOSONLY __NONNULL((2)) __ATTR_SENTINEL_O(1),pid_t,__LIBCCALL,spawnlpe,(int __mode, char const *__restrict __file, char const *__args, ...),__REDIRECT_SPAWNLE(char,spawnvpe,__mode,__path,__args))

#ifdef __USE_EXCEPT
/* >> spawnv(3), spawnve(3), spawnvp(3), spawnvpe(3), spawnl(3), spawnle(3), spawnlp(3), spawnlpe(3)
 * @param: mode: One of `P_*' (e.g. `P_WAIT')
 * A combination of `fork(2)' and `exec(2)' that can be used
 * to create a new process and load it with a new application. */
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

#ifdef __CRT_KOS

/* >> fspawnv(3), fspawnve(3), fspawnl(3), fspawnle(3)
 * As an extension, just like with the exec() family,
 * KOS supports spawning a process from a file descriptor.
 * NOTE: These functions were not apart of the DOS Application Binary Interface
 *       and are not supported on anything other than KOS itself! */
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY_ALT(spawnv) __NONNULL((3)),pid_t,__LIBCCALL,fspawnv,(int __mode, __fd_t __fd, __TARGV),(__mode,__fd,___argv))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY_ALT(spawnve) __NONNULL((3,4)),pid_t,__LIBCCALL,fspawnve,(int __mode, __fd_t __fd, __TARGV, __TENVP),(__mode,__fd,___argv,___envp))
__XREDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY_ALT(spawnl) __ATTR_SENTINEL,pid_t,__LIBCCALL,fspawnl,(int __mode, __fd_t __fd, char const *__args, ...),__REDIRECT_SPAWNL(char,fspawnv,__mode_t,__fd,__args))
__XREDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY_ALT(spawnle) __ATTR_SENTINEL_O(1),pid_t,__LIBCCALL,fspawnle,(int __mode, __fd_t __fd, char const *__args, ...),__REDIRECT_SPAWNLE(char,fspawnve,__mode_t,__fd,__args))

#ifdef __USE_EXCEPT
/* >> fspawnv(3), fspawnve(3), fspawnl(3), fspawnle(3)
 * As an extension, just like with the exec() family,
 * KOS supports spawning a process from a file descriptor.
 * NOTE: These functions were not apart of the DOS Application Binary Interface
 *       and are not supported on anything other than KOS itself! */
__LIBC __PORT_KOSONLY __NONNULL((3)) pid_t (__LIBCCALL Xfspawnv)(int __mode, __fd_t __fd, __TARGV);
__LIBC __PORT_KOSONLY __NONNULL((3,4)) pid_t (__LIBCCALL Xfspawnve)(int __mode, __fd_t __fd, __TARGV, __TENVP);
__LIBC __PORT_KOSONLY __ATTR_SENTINEL pid_t (__LIBCCALL Xfspawnl)(int __mode, __fd_t __fd, char const *__args, ...);
__LIBC __PORT_KOSONLY __ATTR_SENTINEL_O(1) pid_t (__LIBCCALL Xfspawnle)(int __mode, __fd_t __fd, char const *__args, ...);
#endif /* __USE_EXCEPT */
#endif /* __CRT_KOS */


#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_PARTS_KOS2_PROCESS_H */
