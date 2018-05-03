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
#ifndef _PARTS_KOS3_EXCEPT_UUNISTD_H
#define _PARTS_KOS3_EXCEPT_UUNISTD_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
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

#if 0
__REDIRECT_W16(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char16_t *,__LIBCCALL,Xw16ttyname,(__fd_t __fd),Xwttyname,(__fd))
__REDIRECT_W32(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char32_t *,__LIBCCALL,Xw32ttyname,(__fd_t __fd),Xwttyname,(__fd))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,Xw16ttyname_r,(__fd_t __fd, char16_t *__buf, size_t __buflen),Xwttyname_r,(__fd,__buf,__buflen))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,Xw32ttyname_r,(__fd_t __fd, char32_t *__buf, size_t __buflen),Xwttyname_r,(__fd,__buf,__buflen))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16setlogin,(char16_t const *__name),Xwsetlogin,(__name))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32setlogin,(char32_t const *__name),Xwsetlogin,(__name))
__REDIRECT_W16(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char16_t *,__LIBCCALL,Xw16getlogin,(void),Xwgetlogin,())
__REDIRECT_W32(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,char32_t *,__LIBCCALL,Xw32getlogin,(void),Xwgetlogin,())
#if defined(__USE_REENTRANT) || defined(__USE_POSIX199506)
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),__LIBCCALL,Xw16getlogin_r,(char16_t *__name, size_t __name_len),Xwgetlogin_r,(__name,__name_len))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),__LIBCCALL,Xw32getlogin_r,(char32_t *__name, size_t __name_len),Xwgetlogin_r,(__name,__name_len))
#endif /* __USE_REENTRANT || __USE_POSIX199506 */
#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K)
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),__LIBCCALL,Xw16gethostname,(char16_t *__name, size_t __buflen),Xwgethostname,(__name,__buflen))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),__LIBCCALL,Xw32gethostname,(char32_t *__name, size_t __buflen),Xwgethostname,(__name,__buflen))
#endif /* __USE_UNIX98 || __USE_XOPEN2K */
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16sethostname,(char16_t const *__name, size_t __len),Xwsethostname,(__name,__len))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32sethostname,(char32_t const *__name, size_t __len),Xwsethostname,(__name,__len))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),__LIBCCALL,Xw16getdomainname,(char16_t *__name, size_t __buflen),Xwgetdomainname,(__name,__buflen))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),__LIBCCALL,Xw32getdomainname,(char32_t *__name, size_t __buflen),Xwgetdomainname,(__name,__buflen))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16setdomainname,(char16_t const *__name, size_t __len),Xwsetdomainname,(__name,__len))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32setdomainname,(char32_t const *__name, size_t __len),Xwsetdomainname,(__name,__len))
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __NONNULL((1)),char16_t *,__LIBCCALL,Xw16getpass,(char16_t const *__restrict __prompt),Xwgetpass,(__prompt))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __NONNULL((1)),char32_t *,__LIBCCALL,Xw32getpass,(char32_t const *__restrict __prompt),Xwgetpass,(__prompt))
#endif
#if defined(_ALL_SOURCE) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
#ifndef __Xw16ctermid_defined
#define __Xw16ctermid_defined 1
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED,char16_t *,__LIBCCALL,Xw16ctermid,(char16_t *__s),Xwctermid,(__s))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED,char32_t *,__LIBCCALL,Xw32ctermid,(char32_t *__s),Xwctermid,(__s))
#endif /* !__Xw16ctermid_defined */
#endif /* __USE_XOPEN && !__USE_XOPEN2K */
#endif

__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16chown,(char16_t const *__file, __uid_t __owner, __gid_t __group),Xwchown,(__file,__owner,__group))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32chown,(char32_t const *__file, __uid_t __owner, __gid_t __group),Xwchown,(__file,__owner,__group))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),long int,__LIBCCALL,Xw16pathconf,(char16_t const *__path, int __name),Xwpathconf,(__path,__name))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),long int,__LIBCCALL,Xw32pathconf,(char32_t const *__path, int __name),Xwpathconf,(__path,__name))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw16link,(char16_t const *__from, char16_t const *__to),Xwlink,(__from,__to))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw32link,(char32_t const *__from, char32_t const *__to),Xwlink,(__from,__to))
#ifdef __USE_ATFILE
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,Xw16faccessat,(__fd_t __dfd, char16_t const *__file, int __type, __atflag_t __flags),Xwfaccessat,(__dfd,__file,__type,__flag))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,Xw32faccessat,(__fd_t __dfd, char32_t const *__file, int __type, __atflag_t __flags),Xwfaccessat,(__dfd,__file,__type,__flag))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16fchownat,(__fd_t __dfd, char16_t const *__file, __uid_t __owner, __gid_t __group, __atflag_t __flags),Xwfchownat,(__dfd,__file,__owner,__group))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32fchownat,(__fd_t __dfd, char32_t const *__file, __uid_t __owner, __gid_t __group, __atflag_t __flags),Xwfchownat,(__dfd,__file,__owner,__group))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,4)),__LIBCCALL,Xw16linkat,(__fd_t __fromfd, char16_t const *__from, __fd_t __tofd, char16_t const *__to, __atflag_t __flags),Xwlinkat,(__fromfd,__from,__tofd,__to,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2,4)),__LIBCCALL,Xw32linkat,(__fd_t __fromfd, char32_t const *__from, __fd_t __tofd, char32_t const *__to, __atflag_t __flags),Xwlinkat,(__fromfd,__from,__tofd,__to,__flags))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,3)),__LIBCCALL,Xw16symlinkat,(char16_t const *__from, __fd_t __tofd, char16_t const *__to),Xwsymlinkat,(__from,__tofd,__to))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,3)),__LIBCCALL,Xw32symlinkat,(char32_t const *__from, __fd_t __tofd, char32_t const *__to),Xwsymlinkat,(__from,__tofd,__to))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),size_t,__LIBCCALL,Xw16readlinkat,(__fd_t __dfd, char16_t const *__restrict __path, char16_t *__restrict __buf, size_t __buflen),Xwreadlinkat,(__dfd,__path,__buf,__buflen))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),size_t,__LIBCCALL,Xw32readlinkat,(__fd_t __dfd, char32_t const *__restrict __path, char32_t *__restrict __buf, size_t __buflen),Xwreadlinkat,(__dfd,__path,__buf,__buflen))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16unlinkat,(__fd_t __dfd, char16_t const *__name, __atflag_t __flags),Xwunlinkat,(__dfd,__name,__flag))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32unlinkat,(__fd_t __dfd, char32_t const *__name, __atflag_t __flags),Xwunlinkat,(__dfd,__name,__flag))
#endif /* __USE_ATFILE */

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#ifdef __USE_KOS
#define __TRUNCATE_OFF_T    __FS_TYPE(pos)
#define __TRUNCATE_OFF64_T  __pos64_t
#else
#define __TRUNCATE_OFF_T    __FS_TYPE(off)
#define __TRUNCATE_OFF64_T  __off64_t
#endif
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16lchown,(char16_t const *__file, __uid_t __owner, __gid_t __group),Xwlchown,(__file,__owner,__group))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32lchown,(char32_t const *__file, __uid_t __owner, __gid_t __group),Xwlchown,(__file,__owner,__group))
#ifdef __USE_FILE_OFFSET64
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16truncate,(char16_t const *__file, __TRUNCATE_OFF_T __length),Xwtruncate64,(__file,__length))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32truncate,(char32_t const *__file, __TRUNCATE_OFF_T __length),Xwtruncate64,(__file,__length))
#else
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16truncate,(char16_t const *__file, __TRUNCATE_OFF_T __length),Xwtruncate,(__file,__length))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32truncate,(char32_t const *__file, __TRUNCATE_OFF_T __length),Xwtruncate,(__file,__length))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16truncate64,(char16_t const *__file, __TRUNCATE_OFF64_T __length),Xwtruncate64,(__file,__length))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32truncate64,(char32_t const *__file, __TRUNCATE_OFF64_T __length),Xwtruncate64,(__file,__length))
#endif /* __USE_LARGEFILE64 */
#undef __TRUNCATE_OFF_T
#undef __TRUNCATE_OFF64_T
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_POSIX2
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED,size_t,__LIBCCALL,Xw16confstr,(int __name, char16_t *__buf, size_t __buflen),Xwconfstr,(__name,__buf,__buflen))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED,size_t,__LIBCCALL,Xw32confstr,(int __name, char32_t *__buf, size_t __buflen),Xwconfstr,(__name,__buf,__buflen))
#endif /* __USE_POSIX2 */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw16symlink,(char16_t const *__from, char16_t const *__to),Xwsymlink,(__from,__to))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw32symlink,(char32_t const *__from, char32_t const *__to),Xwsymlink,(__from,__to))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),size_t,__LIBCCALL,Xw16readlink,(char16_t const *__restrict __path, char16_t *__restrict __buf, size_t __buflen),Xwreadlink,(__path,__buf,__buflen))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),size_t,__LIBCCALL,Xw32readlink,(char32_t const *__restrict __path, char32_t *__restrict __buf, size_t __buflen),Xwreadlink,(__path,__buf,__buflen))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16revoke,(char16_t const *__file),Xwrevoke,(__file))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32revoke,(char32_t const *__file),Xwrevoke,(__file))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xw16acct,(char16_t const *__name),Xwacct,(__name))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xw32acct,(char32_t const *__name),Xwacct,(__name))
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__REDIRECT_W16_VOID(__LIBC,__PORT_NODOS __NONNULL((1)),__LIBCCALL,Xw16chroot,(char16_t const *__restrict __path),Xwchroot,(__path))
__REDIRECT_W32_VOID(__LIBC,__PORT_NODOS __NONNULL((1)),__LIBCCALL,Xw32chroot,(char32_t const *__restrict __path),Xwchroot,(__path))
#endif

__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16chdir,(char16_t const *__path),Xwchdir,(__path))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32chdir,(char32_t const *__path),Xwchdir,(__path))
__REDIRECT_W16(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED_SUGGESTED,char16_t *,__LIBCCALL,Xw16getcwd,(char16_t *__buf, size_t __size),Xwgetcwd,(__buf,__size))
__REDIRECT_W32(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED_SUGGESTED,char32_t *,__LIBCCALL,Xw32getcwd,(char32_t *__buf, size_t __size),Xwgetcwd,(__buf,__size))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16unlink,(char16_t const *__name),Xwunlink,(__name))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32unlink,(char32_t const *__name),Xwunlink,(__name))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16rmdir,(char16_t const *__path),Xwrmdir,(__path))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32rmdir,(char32_t const *__path),Xwrmdir,(__path))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16access,(char16_t const *__name, int __type),Xwaccess,(__name,__type))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32access,(char32_t const *__name, int __type),Xwaccess,(__name,__type))
#ifdef __USE_GNU
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16euidaccess,(char16_t const *__name, int __type),Xweaccess,(__name,__type))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32euidaccess,(char32_t const *__name, int __type),Xweaccess,(__name,__type))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16eaccess,(char16_t const *__name, int __type),Xweaccess,(__name,__type))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32eaccess,(char32_t const *__name, int __type),Xweaccess,(__name,__type))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_RETNONNULL __PORT_KOSONLY,char16_t *,__LIBCCALL,Xw16get_current_dir_name,(void),wget_current_dir_name,())
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_RETNONNULL __PORT_KOSONLY,char32_t *,__LIBCCALL,Xw32get_current_dir_name,(void),wget_current_dir_name,())
#endif /* __USE_GNU */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_UUNISTD_H */
