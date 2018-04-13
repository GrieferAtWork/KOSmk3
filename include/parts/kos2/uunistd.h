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
#ifndef _PARTS_KOS2_UUNISTD_H
#define _PARTS_KOS2_UUNISTD_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

#ifndef __ssize_t_defined
#define __ssize_t_defined 1
typedef __ssize_t ssize_t;
#endif /* !__ssize_t_defined */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#ifndef __KERNEL__
#ifdef __CRT_KOS

#if 0
__REDIRECT_EXCEPT_W16(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char16_t *,__LIBCCALL,w16ttyname,(__fd_t __fd),wttyname,(__fd))
__REDIRECT_EXCEPT_W32(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char32_t *,__LIBCCALL,w32ttyname,(__fd_t __fd),wttyname,(__fd))
__REDIRECT_EXCEPT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16ttyname_r,(__fd_t __fd, char16_t *__buf, size_t __buflen),wttyname_r,(__fd,__buf,__buflen))
__REDIRECT_EXCEPT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32ttyname_r,(__fd_t __fd, char32_t *__buf, size_t __buflen),wttyname_r,(__fd,__buf,__buflen))
__REDIRECT_EXCEPT_W16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16setlogin,(char16_t const *__name),wsetlogin,(__name))
__REDIRECT_EXCEPT_W32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32setlogin,(char32_t const *__name),wsetlogin,(__name))
__REDIRECT_EXCEPT_W16(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char16_t *,__LIBCCALL,w16getlogin,(void),wgetlogin,())
__REDIRECT_EXCEPT_W32(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char32_t *,__LIBCCALL,w32getlogin,(void),wgetlogin,())
#if defined(__USE_REENTRANT) || defined(__USE_POSIX199506)
__REDIRECT_EXCEPT_W16_XVOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__LIBCCALL,w16getlogin_r,(char16_t *__name, size_t __name_len),wgetlogin_r,(__name,__name_len))
__REDIRECT_EXCEPT_W32_XVOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__LIBCCALL,w32getlogin_r,(char32_t *__name, size_t __name_len),wgetlogin_r,(__name,__name_len))
#endif /* __USE_REENTRANT || __USE_POSIX199506 */
#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K)
__REDIRECT_EXCEPT_W16_XVOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__LIBCCALL,w16gethostname,(char16_t *__name, size_t __buflen),wgethostname,(__name,__buflen))
__REDIRECT_EXCEPT_W32_XVOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__LIBCCALL,w32gethostname,(char32_t *__name, size_t __buflen),wgethostname,(__name,__buflen))
#endif /* __USE_UNIX98 || __USE_XOPEN2K */
__REDIRECT_EXCEPT_W16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16sethostname,(char16_t const *__name, size_t __len),wsethostname,(__name,__len))
__REDIRECT_EXCEPT_W32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32sethostname,(char32_t const *__name, size_t __len),wsethostname,(__name,__len))
__REDIRECT_EXCEPT_W16_XVOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__LIBCCALL,w16getdomainname,(char16_t *__name, size_t __buflen),wgetdomainname,(__name,__buflen))
__REDIRECT_EXCEPT_W32_XVOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__LIBCCALL,w32getdomainname,(char32_t *__name, size_t __buflen),wgetdomainname,(__name,__buflen))
__REDIRECT_EXCEPT_W16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16setdomainname,(char16_t const *__name, size_t __len),wsetdomainname,(__name,__len))
__REDIRECT_EXCEPT_W32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32setdomainname,(char32_t const *__name, size_t __len),wsetdomainname,(__name,__len))
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__REDIRECT_EXCEPT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __NONNULL((1)),char16_t *,__LIBCCALL,w16getpass,(char16_t const *__restrict __prompt),wgetpass,(__prompt))
__REDIRECT_EXCEPT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __NONNULL((1)),char32_t *,__LIBCCALL,w32getpass,(char32_t const *__restrict __prompt),wgetpass,(__prompt))
#endif
#if defined(_ALL_SOURCE) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
#ifndef __w16ctermid_defined
#define __w16ctermid_defined 1
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED,char16_t *,__LIBCCALL,w16ctermid,(char16_t *__s),wctermid,(__s))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED,char32_t *,__LIBCCALL,w32ctermid,(char32_t *__s),wctermid,(__s))
#endif /* !__wctermid_defined */
#endif /* __USE_XOPEN && !__USE_XOPEN2K */
#endif

__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16chown,(char16_t const *__file, __uid_t __owner, __gid_t __group),wchown,(__file,__owner,__group))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32chown,(char32_t const *__file, __uid_t __owner, __gid_t __group),wchown,(__file,__owner,__group))
__REDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __NONNULL((1)),long int,__LIBCCALL,w16pathconf,(char16_t const *__path, int __name),wpathconf,(__path,__name))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((1)),long int,__LIBCCALL,w32pathconf,(char32_t const *__path, int __name),wpathconf,(__path,__name))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w16link,(char16_t const *__from, char16_t const *__to),wlink,(__from,__to))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w32link,(char32_t const *__from, char32_t const *__to),wlink,(__from,__to))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16revoke,(char16_t const *__file),wrevoke,(__file))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32revoke,(char32_t const *__file),wrevoke,(__file))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,w16acct,(char16_t const *__name),wacct,(__name))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,w32acct,(char32_t const *__name),wacct,(__name))
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16chdir,(char16_t const *__path),wchdir,(__path))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32chdir,(char32_t const *__path),wchdir,(__path))
__REDIRECT_EXCEPT_UFSDPW16(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED_SUGGESTED,char16_t *,__LIBCCALL,w16getcwd,(char16_t *__buf, size_t __size),wgetcwd,(__buf,__size))
__REDIRECT_EXCEPT_UFSW32  (__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED_SUGGESTED,char32_t *,__LIBCCALL,w32getcwd,(char32_t *__buf, size_t __size),wgetcwd,(__buf,__size))
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16unlink,(char16_t const *__name),wunlink,(__name))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32unlink,(char32_t const *__name),wunlink,(__name))
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16rmdir,(char16_t const *__path),wrmdir,(__path))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32rmdir,(char32_t const *__path),wrmdir,(__path))
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__PORT_KOSONLY __IF_NUSE_EXCEPT(__WUNUSED) __NONNULL((1)),int,__LIBCCALL,w16access,(char16_t const *__name, int __type),waccess,(__name,__type))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__PORT_KOSONLY __IF_NUSE_EXCEPT(__WUNUSED) __NONNULL((1)),int,__LIBCCALL,w32access,(char32_t const *__name, int __type),waccess,(__name,__type))

#ifdef __USE_ATFILE
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __IF_NUSE_EXCEPT(__WUNUSED) __NONNULL((2)),int,__LIBCCALL,w16faccessat,(__fd_t __dfd, char16_t const *__file, int __type, __atflag_t __flags),wfaccessat,(__dfd,__file,__type,__flag))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __IF_NUSE_EXCEPT(__WUNUSED) __NONNULL((2)),int,__LIBCCALL,w32faccessat,(__fd_t __dfd, char32_t const *__file, int __type, __atflag_t __flags),wfaccessat,(__dfd,__file,__type,__flag))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16fchownat,(__fd_t __dfd, char16_t const *__file, __uid_t __owner, __gid_t __group, __atflag_t __flags),wfchownat,(__dfd,__file,__owner,__group))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32fchownat,(__fd_t __dfd, char32_t const *__file, __uid_t __owner, __gid_t __group, __atflag_t __flags),wfchownat,(__dfd,__file,__owner,__group))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2,4)),int,__LIBCCALL,w16linkat,(__fd_t __fromfd, char16_t const *__from, __fd_t __tofd, char16_t const *__to, int __flags),wlinkat,(__fromfd,__from,__tofd,__to,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2,4)),int,__LIBCCALL,w32linkat,(__fd_t __fromfd, char32_t const *__from, __fd_t __tofd, char32_t const *__to, int __flags),wlinkat,(__fromfd,__from,__tofd,__to,__flags))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,3)),int,__LIBCCALL,w16symlinkat,(char16_t const *__from, __fd_t __tofd, char16_t const *__to),wsymlinkat,(__from,__tofd,__to))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,3)),int,__LIBCCALL,w32symlinkat,(char32_t const *__from, __fd_t __tofd, char32_t const *__to),wsymlinkat,(__from,__tofd,__to))
__REDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,w16readlinkat,(__fd_t __dfd, char16_t const *__restrict __path, char16_t *__restrict __buf, size_t __buflen),wreadlinkat,(__dfd,__path,__buf,__buflen))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,w32readlinkat,(__fd_t __dfd, char32_t const *__restrict __path, char32_t *__restrict __buf, size_t __buflen),wreadlinkat,(__dfd,__path,__buf,__buflen))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16unlinkat,(__fd_t __dfd, char16_t const *__name, int __flag),wunlinkat,(__dfd,__name,__flag))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32unlinkat,(__fd_t __dfd, char32_t const *__name, int __flag),wunlinkat,(__dfd,__name,__flag))
#endif /* __USE_ATFILE */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#ifdef __USE_KOS
#define __TRUNCATE_OFF_T    __FS_TYPE(pos)
#define __TRUNCATE_OFF64_T  __pos64_t
#else
#define __TRUNCATE_OFF_T    __FS_TYPE(off)
#define __TRUNCATE_OFF64_T  __off64_t
#endif
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16lchown,(char16_t const *__file, __uid_t __owner, __gid_t __group),wlchown,(__file,__owner,__group))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32lchown,(char32_t const *__file, __uid_t __owner, __gid_t __group),wlchown,(__file,__owner,__group))
#ifdef __USE_FILE_OFFSET64
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16truncate,(char16_t const *__file, __TRUNCATE_OFF_T __length),wtruncate64,(__file,__length))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32truncate,(char32_t const *__file, __TRUNCATE_OFF_T __length),wtruncate64,(__file,__length))
#else
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16truncate,(char16_t const *__file, __TRUNCATE_OFF_T __length),wtruncate,(__file,__length))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32truncate,(char32_t const *__file, __TRUNCATE_OFF_T __length),wtruncate,(__file,__length))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16truncate64,(char16_t const *__file, __TRUNCATE_OFF64_T __length),wtruncate64,(__file,__length))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32truncate64,(char32_t const *__file, __TRUNCATE_OFF64_T __length),wtruncate64,(__file,__length))
#endif /* __USE_LARGEFILE64 */
#undef __TRUNCATE_OFF_T
#undef __TRUNCATE_OFF64_T
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_POSIX2
__REDIRECT_EXCEPT_W16(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED,size_t,__LIBCCALL,
                      w16confstr,(int __name, char16_t *__buf, size_t __buflen),
                      wconfstr,(__name,__buf,__buflen))
__REDIRECT_EXCEPT_W32(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED,size_t,__LIBCCALL,
                      w32confstr,(int __name, char32_t *__buf, size_t __buflen),
                      wconfstr,(__name,__buf,__buflen))
#endif /* __USE_POSIX2 */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w16symlink,(char16_t const *__from, char16_t const *__to),wsymlink,(__from,__to))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w32symlink,(char32_t const *__from, char32_t const *__to),wsymlink,(__from,__to))
__REDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,w16readlink,(char16_t const *__restrict __path, char16_t *__restrict __buf, size_t __buflen),wreadlink,(__path,__buf,__buflen))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,w32readlink,(char32_t const *__restrict __path, char32_t *__restrict __buf, size_t __buflen),wreadlink,(__path,__buf,__buflen))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16chroot,(char16_t const *__restrict __path),wchroot,(__path))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32chroot,(char32_t const *__restrict __path),wchroot,(__path))
#endif
#ifdef __USE_GNU
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __IF_NUSE_EXCEPT(__WUNUSED) __NONNULL((1)),int,__LIBCCALL,w16euidaccess,(char16_t const *__name, int __type),weaccess,(__name,__type))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __IF_NUSE_EXCEPT(__WUNUSED) __NONNULL((1)),int,__LIBCCALL,w32euidaccess,(char32_t const *__name, int __type),weaccess,(__name,__type))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __IF_NUSE_EXCEPT(__WUNUSED) __NONNULL((1)),int,__LIBCCALL,w16eaccess,(char16_t const *__name, int __type),weaccess,(__name,__type))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __IF_NUSE_EXCEPT(__WUNUSED) __NONNULL((1)),int,__LIBCCALL,w32eaccess,(char32_t const *__name, int __type),weaccess,(__name,__type))
__REDIRECT_EXCEPT_UFSW16(__LIBC,__WUNUSED __PORT_KOSONLY,char16_t *,__LIBCCALL,w16get_current_dir_name,(void),wget_current_dir_name,())
__REDIRECT_EXCEPT_UFSW32(__LIBC,__WUNUSED __PORT_KOSONLY,char32_t *,__LIBCCALL,w32get_current_dir_name,(void),wget_current_dir_name,())
#endif /* __USE_GNU */

#endif /* __CRT_KOS */
#endif /* !__KERNEL__ */

__SYSDECL_END

/* Pull in unicode process definitions. */
#ifdef __USE_KOS
#ifndef _PARTS_KOS2_UPROCESS_H
#include "uprocess.h"
#endif
#endif /* __USE_KOS */
#ifdef __USE_EXCEPT
#include <parts/kos3/except/uunistd.h>
#endif

#endif /* !_PARTS_KOS2_UUNISTD_H */
