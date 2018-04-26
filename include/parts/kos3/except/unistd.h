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
#ifndef _PARTS_KOS3_EXCEPT_UNISTD_H
#define _PARTS_KOS3_EXCEPT_UNISTD_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

__REDIRECT(__LIBC,__PORT_KOSONLY __WUNUSED,__pid_t,__LIBCCALL,__Xgetpgid,(__pid_t __pid),Xgetpgid,(__pid))
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsetpgid)(__pid_t __pid, __pid_t __pgid);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xgetgroups)(int __size, __gid_t __list[]);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsetuid)(__uid_t __uid);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsetgid)(__gid_t __gid);
__LIBC __PORT_KOSONLY __WUNUSED __pid_t (__LIBCCALL Xfork)(void);
__LIBC __PORT_KOSONLY __ATTR_NORETURN void (__LIBCCALL Xpause)(void);
__LIBC __PORT_KOSONLY __WUNUSED long int (__LIBCCALL Xfpathconf)(__fd_t __fd, int __name);
#if 0
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED char *(__LIBCCALL Xttyname)(__fd_t __fd);
__LIBC __PORT_KOSONLY __NONNULL((2)) int (__LIBCCALL Xttyname_r)(__fd_t __fd, char *__buf, size_t __buflen);
#endif
__LIBC __PORT_KOSONLY __WUNUSED __pid_t (__LIBCCALL Xtcgetpgrp)(__fd_t __fd);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xtcsetpgrp)(__fd_t __fd, __pid_t __pgrp_id);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED char *(__LIBCCALL Xgetlogin)(void);
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xchown)(char const *__file, __uid_t __owner, __gid_t __group);
__LIBC __PORT_KOSONLY __NONNULL((1)) long int (__LIBCCALL Xpathconf)(char const *__path, int __name);
__LIBC __PORT_KOSONLY __NONNULL((1,2)) void (__LIBCCALL Xlink)(char const *__from, char const *__to);
#ifndef __Xread_defined
#define __Xread_defined 1
__LIBC __PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((2)) __size_t (__LIBCCALL Xread)(__fd_t __fd, void *__dstbuf, __size_t __dstbufsize);
#endif /* !__Xread_defined */
#ifndef __Xwrite_defined
#define __Xwrite_defined 1
__LIBC __PORT_KOSONLY __NONNULL((2)) __size_t (__LIBCCALL Xwrite)(__fd_t __fd, void const *__srcbuf, __size_t __srcbufsize);
#endif /* !__Xwrite_defined */
#ifndef __Xlseek_defined
#define __Xlseek_defined 1
__REDIRECT_FS64(__LIBC,__PORT_KOSONLY,__FS_TYPE(pos),__LIBCCALL,Xlseek,(__fd_t __fd, __FS_TYPE(off) __offset, int __whence),(__fd,__offset,__whence))
#endif /* !__Xlseek_defined */
#ifndef __Xdup_defined
#define __Xdup_defined 1
__LIBC __PORT_KOSONLY __WUNUSED __fd_t (__LIBCCALL Xdup)(__fd_t __fd);
#endif /* !__Xdup_defined */
#ifndef __Xdup2_defined
#define __Xdup2_defined 1
__LIBC __PORT_KOSONLY __fd_t (__LIBCCALL Xdup2)(__fd_t __ofd, __fd_t __nfd);
#endif /* !__Xdup2_defined */
#ifndef __Xgetcwd_defined
#define __Xgetcwd_defined 1
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED_SUGGESTED char *(__LIBCCALL Xgetcwd)(char *__buf, size_t __size);
#endif /* !__Xgetcwd_defined */
#ifndef __Xchdir_defined
#define __Xchdir_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xchdir)(char const *__path);
#endif /* !__Xchdir_defined */
#ifndef __Xunlink_defined
#define __Xunlink_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xunlink)(char const *__name);
#endif /* !__Xunlink_defined */
#ifndef __Xrmdir_defined
#define __Xrmdir_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xrmdir)(char const *__path);
#endif /* !__Xrmdir_defined */
#ifndef __Xaccess_defined
#define __Xaccess_defined 1
__LIBC __NONNULL((1)) void (__LIBCCALL Xaccess)(char const *__name, int __type);
#endif /* !__Xaccess_defined */

#ifdef __USE_GNU
__REDIRECT_VOID(__LIBC,__NONNULL((1)),__LIBCCALL,Xeuidaccess,(char const *__name, int __type),Xeaccess,(__name,__type))
__LIBC __NONNULL((1)) void (__LIBCCALL Xeaccess)(char const *__name, int __type);
#endif /* __USE_GNU */
#ifdef __USE_ATFILE
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xfaccessat)(__fd_t __dfd, char const *__file, int __type, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xfchownat)(__fd_t __dfd, char const *__file, __uid_t __owner, __gid_t __group, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((2,4)) void (__LIBCCALL Xlinkat)(__fd_t __fromfd, char const *__from, __fd_t __tofd, char const *__to, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((1,3)) void (__LIBCCALL Xsymlinkat)(char const *__from, __fd_t __tofd, char const *__to);
__LIBC __PORT_KOSONLY __NONNULL((2,3)) size_t (__LIBCCALL Xreadlinkat)(__fd_t __dfd, char const *__restrict __path, char *__restrict __buf, size_t __buflen);
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xunlinkat)(__fd_t __dfd, char const *__name, __atflag_t __flags);
#endif /* __USE_ATFILE */

#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY __pos64_t (__LIBCCALL Xlseek64)(__fd_t __fd, __off64_t __offset, int __whence);
#endif /* __USE_LARGEFILE64 */

#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K8)
#ifdef __USE_KOS
#define __PIO_OFFSET     __FS_TYPE(off)
#define __PIO_OFFSET64   __off64_t
#else
#define __PIO_OFFSET     __FS_TYPE(pos)
#define __PIO_OFFSET64   __pos64_t
#endif
__REDIRECT_FS64(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED,size_t,__LIBCCALL,Xpread,(__fd_t __fd, void *__buf, size_t __n_bytes, __PIO_OFFSET __offset),(__fd,__buf,__n_bytes,__offset))
__REDIRECT_FS64(__LIBC,__PORT_KOSONLY,size_t,__LIBCCALL,Xpwrite,(__fd_t __fd, void const *__buf, size_t __n_bytes, __PIO_OFFSET __offset),(__fd,__buf,__n_bytes,__offset))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY __WUNUSED_SUGGESTED size_t (__LIBCCALL Xpread64)(__fd_t __fd, void *__buf, size_t __n_bytes, __PIO_OFFSET64 __offset);
__LIBC __PORT_KOSONLY size_t (__LIBCCALL Xpwrite64)(__fd_t __fd, void const *__buf, size_t __n_bytes, __PIO_OFFSET64 __offset);
#endif /* __USE_LARGEFILE64 */
#undef __PIO_OFFSET64
#undef __PIO_OFFSET
#endif /* __USE_UNIX98 || __USE_XOPEN2K8 */

#ifdef __USE_GNU
__LIBC __PORT_KOSONLY void (__LIBCCALL Xpipe2)(__fd_t __pipedes[2], __oflag_t __flags);
__LIBC __PORT_KOSONLY __fd_t (__LIBCCALL Xdup3)(__fd_t __ofd, __fd_t __nfd, __oflag_t __flags);
__LIBC __WUNUSED char *(__LIBCCALL Xget_current_dir_name)(void);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsyncfs)(__fd_t __fd);
__LIBC __PORT_KOSONLY int (__LIBCCALL Xgroup_member)(__gid_t __gid);
__LIBC __PORT_KOSONLY int (__LIBCCALL Xgetresuid)(__uid_t *__ruid, __uid_t *__euid, __uid_t *__suid);
__LIBC __PORT_KOSONLY int (__LIBCCALL Xgetresgid)(__gid_t *__rgid, __gid_t *__egid, __gid_t *__sgid);
__LIBC __PORT_KOSONLY __WUNUSED int (__LIBCCALL Xsetresuid)(__uid_t __ruid, __uid_t __euid, __uid_t __suid);
__LIBC __PORT_KOSONLY __WUNUSED int (__LIBCCALL Xsetresgid)(__gid_t __rgid, __gid_t __egid, __gid_t __sgid);
#endif /* __USE_GNU */

#if (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8)) || \
     defined(__USE_MISC)
#ifndef __Xvfork_defined
#define __Xvfork_defined 1
__LIBC __ATTR_RETURNS_TWICE __PORT_KOSONLY __WUNUSED __pid_t (__LIBCCALL Xvfork)(void);
#endif /* !__Xvfork_defined */
#endif

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#ifdef __USE_KOS
#define __TRUNCATE_OFF_T    __FS_TYPE(pos)
#define __TRUNCATE_OFF64_T  __pos64_t
#else
#define __TRUNCATE_OFF_T    __FS_TYPE(off)
#define __TRUNCATE_OFF64_T  __off64_t
#endif
__LIBC __PORT_KOSONLY void (__LIBCCALL Xfchown)(__fd_t __fd, __uid_t __owner, __gid_t __group);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xfchdir)(__fd_t __fd);
__LIBC __PORT_KOSONLY __WUNUSED __pid_t (__LIBCCALL Xgetpgid)(__pid_t __pid);
__LIBC __PORT_KOSONLY __WUNUSED __pid_t (__LIBCCALL Xgetsid)(__pid_t __pid);
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xlchown)(char const *__file, __uid_t __owner, __gid_t __group);
__REDIRECT_FS64_VOID(__LIBC,__NONNULL((1)),__LIBCCALL,Xtruncate,(char const *__file, __TRUNCATE_OFF_T __length),(__file,__length))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xtruncate64)(char const *__file, __TRUNCATE_OFF64_T __length);
#endif /* __USE_LARGEFILE64 */
#undef __TRUNCATE_OFF_T
#undef __TRUNCATE_OFF64_T
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */

#if defined(__USE_MISC) || defined(__USE_XOPEN)
__LIBC __PORT_KOSONLY __WUNUSED int (__LIBCCALL Xnice)(int __inc);
#endif /* __USE_MISC || __USE_XOPEN */

#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__LIBC __PORT_KOSONLY __pid_t (__LIBCCALL Xsetpgrp)(void);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsetreuid)(__uid_t __ruid, __uid_t __euid);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsetregid)(__gid_t __rgid, __gid_t __egid);
__LIBC __PORT_KOSONLY __WUNUSED long int (__LIBCCALL Xgethostid)(void);
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */

#ifdef __USE_XOPEN2K
__LIBC __PORT_KOSONLY void (__LIBCCALL Xseteuid)(__uid_t __uid);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsetegid)(__gid_t __gid);
#endif /* __USE_XOPEN2K */

#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_UNIX98))
__LIBC __PORT_KOSONLY __WUNUSED int (__LIBCCALL Xttyslot)(void);
#endif

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__LIBC __PORT_KOSONLY __NONNULL((1,2)) void (__LIBCCALL Xsymlink)(char const *__from, char const *__to);
__LIBC __PORT_KOSONLY __NONNULL((1,2)) size_t (__LIBCCALL Xreadlink)(char const *__restrict __path, char *__restrict __buf, size_t __buflen);
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */

#if defined(__USE_REENTRANT) || defined(__USE_POSIX199506)
__LIBC __PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)) void (__LIBCCALL Xgetlogin_r)(char *__name, size_t __name_len);
#endif /* __USE_REENTRANT || __USE_POSIX199506 */

#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K)
__LIBC __PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)) void (__LIBCCALL Xgethostname)(char *__name, size_t __buflen);
#endif /* __USE_UNIX98 || __USE_XOPEN2K */

#ifdef __USE_MISC
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xsetlogin)(char const *__name);
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xsethostname)(char const *__name, size_t __len);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsethostid)(long int __id);
__LIBC __PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)) void (__LIBCCALL Xgetdomainname)(char *__name, size_t __buflen);
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xsetdomainname)(char const *__name, size_t __len);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xvhangup)(void);
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xprofil)(unsigned short int *__sample_buffer, size_t __size, size_t __offset, unsigned int __scale);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xdaemon)(int __nochdir, int __noclose);
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xrevoke)(char const *__file);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xacct)(char const *__name);
#ifdef __USE_KOS
/* Execute a system call, returning in both EAX and EDX */
#ifdef __NO_ASMNAME
#ifndef __Xsyscall_defined
#define __Xsyscall_defined 1
__LIBC __PORT_KOSONLY long long int (__ATTR_CDECL Xsyscall)(long int __sysno, ...);
#endif /* !__Xsyscall_defined */
#define Xlsyscall(...)             (Xsyscall)(__VA_ARGS__)
#define Xsyscall(...)   ((long int)(Xsyscall)(__VA_ARGS__))
#else
__LIBC __PORT_KOSONLY long int (__ATTR_CDECL Xsyscall)(long int __sysno, ...);
__LIBC __PORT_KOSONLY long long int (__ATTR_CDECL Xlsyscall)(long int __sysno, ...) __ASMNAME("Xsyscall");
#endif /* !__NO_ASMNAME */
#else /* __USE_KOS */
__LIBC __PORT_KOSONLY long int (__ATTR_CDECL Xsyscall)(long int __sysno, ...);
#endif /* !__USE_KOS */
#endif /* __USE_MISC */

#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xchroot)(char const *__restrict __path);
__LIBC __PORT_KOSONLY __WUNUSED __NONNULL((1)) char *(__LIBCCALL Xgetpass)(char const *__restrict __prompt);
#endif

#if defined(__USE_POSIX199309) || defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT_FS64_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xftruncate,(__fd_t __fd, __FS_TYPE(off) __length),(__fd,__length))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY void (__LIBCCALL Xftruncate64)(__fd_t __fd, __off64_t __length);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_POSIX199309 || __USE_XOPEN_EXTENDED || __USE_XOPEN2K */

#if (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K)) || \
     defined(__USE_MISC)
#ifndef __intptr_t_defined
#define __intptr_t_defined 1
typedef __intptr_t intptr_t;
#endif /* !__intptr_t_defined */
__LIBC __PORT_KOSONLY void (__LIBCCALL Xbrk)(void *__addr);
__LIBC __PORT_KOSONLY void *(__LIBCCALL Xsbrk)(intptr_t __delta);
#endif

#if defined(__USE_POSIX199309) || defined(__USE_UNIX98)
__LIBC void (__LIBCCALL Xfdatasync)(__fd_t __fd);
#endif /* __USE_POSIX199309 || __USE_UNIX98 */

#if defined(_ALL_SOURCE) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__LIBC __PORT_NODOS __WUNUSED long int (__LIBCCALL Xsysconf)(int __name);
#endif /* __CRT_GLC */

#ifndef __Xlockf_defined
#define __Xlockf_defined 1
__REDIRECT_FS64_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xlockf,(__fd_t __fd, int __cmd, __FS_TYPE(off) __len),(__fd,__cmd,__len))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY void (__LIBCCALL Xlockf64)(__fd_t __fd, int __cmd, __off64_t __len);
#endif /* __USE_LARGEFILE64 */
#endif /* !__Xlockf_defined */

#ifdef __USE_KOS
/* Resolve a `PATH' relative to `FD`, using `FLAGS' and fill in
 * the given `BUF...+=BUFSIZE' with its name according to `TYPE'.
 * @param: FD:          A file descriptor for the base path to lookup.
 * @param: PATH:        A path relative to `FD' (can be empty when `AT_EMPTY_PATH' is passed)
 * @param: FLAGS:       Set of `AT_*' (AT-path constants)
 * @param: BUF:         A buffer to be filled with the name of the path.
 *                      [xfrealpathat] When NULL, malloc() a new buffer of `BUFSIZE' bytes, or if
 *                                    `BUFSIZE' is ZERO(0), allocate a buffer of appropriate size.
 * @param: BUFSIZE:     The size of the provided `BUF' (in characters)
 * @param: TYPE:        The type of path that should be returned. (Set of `REALPATH_F*'; see above)
 * @return: * :         [Xxfrealpathat] Either `BUF', or the newly allocated buffer when `BUF' was NULL
 * @return: * :         [Xxfrealpathat2] The required buffer size (in characters), excluding a terminated NUL-character
 * @return: >= BUFSIZE: [Xxfrealpathat2] Only a portion of the path was printed. Pass a buffer capable of holding at least `return+1' characters. */
__LIBC __PORT_KOSONLY char *(__LIBCCALL Xxfrealpath)(__fd_t __fd, char *__buf, __size_t __bufsize, unsigned int __type);
__LIBC __PORT_KOSONLY char *(__LIBCCALL Xxrealpath)(char const *__path, char *__buf, __size_t __bufsize, unsigned int __type);
__LIBC __PORT_KOSONLY char *(__LIBCCALL Xxfrealpathat)(__fd_t __dfd, char const *__path, __atflag_t __flags, char *__buf, __size_t __bufsize, unsigned int __type);
__LIBC __PORT_KOSONLY __size_t (__LIBCCALL Xxfrealpath2)(__fd_t __fd, char *__buf, __size_t __bufsize, unsigned int __type);
__LIBC __PORT_KOSONLY __size_t (__LIBCCALL Xxrealpath2)(char const *__path, char *__buf, __size_t __bufsize, unsigned int __type);
__LIBC __PORT_KOSONLY __size_t (__LIBCCALL Xxfrealpathat2)(__fd_t __dfd, char const *__path, __atflag_t __flags, char *__buf, __size_t __bufsize, unsigned int __type);

__LIBC __PORT_KOSONLY __size_t (__LIBCCALL Xxreadf)(__fd_t __fd, void *__buf, __size_t __bufsize, __oflag_t __flags);
__LIBC __PORT_KOSONLY __size_t (__LIBCCALL Xxwritef)(__fd_t __fd, void const *__buf, __size_t __bufsize, __oflag_t __flags);
__REDIRECT_FS64(__LIBC,__PORT_KOSONLY,__size_t,__LIBCCALL,Xxpreadf,(__fd_t __fd, void *__buf, __size_t __bufsize, __FS_TYPE(off) __offset, __oflag_t __flags),(__fd,__buf,__bufsize,__offset,__flags))
__REDIRECT_FS64(__LIBC,__PORT_KOSONLY,__size_t,__LIBCCALL,Xxpwritef,(__fd_t __fd, void const *__buf, __size_t __bufsize, __FS_TYPE(off) __offset, __oflag_t __flags),(__fd,__buf,__bufsize,__offset,__flags))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY __size_t (__LIBCCALL Xxreadf64)(__fd_t __fd, void *__buf, __size_t __bufsize, __off64_t __offset, __oflag_t __flags);
__LIBC __PORT_KOSONLY __size_t (__LIBCCALL Xxwritef64)(__fd_t __fd, void const *__buf, __size_t __bufsize, __off64_t __offset, __oflag_t __flags);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_KOS */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_UNISTD_H */
