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
#ifndef _PARTS_KOS2_WUNISTD_H
#define _PARTS_KOS2_WUNISTD_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

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
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,wchar_t *,__LIBCCALL,wttyname,(__fd_t __fd),(__fd))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,wttyname_r,(__fd_t __fd, wchar_t *__buf, size_t __buflen),(__fd,__buf,__buflen))
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,wsetlogin,(wchar_t const *__name),(__name))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,wchar_t *,__LIBCCALL,wgetlogin,(void),())
#if defined(__USE_REENTRANT) || defined(__USE_POSIX199506)
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__LIBCCALL,wgetlogin_r,(wchar_t *__name, size_t __name_len),(__name,__name_len))
#endif /* __USE_REENTRANT || __USE_POSIX199506 */
#if defined(__USE_UNIX98) || defined(__USE_XOPEN2K)
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__LIBCCALL,wgethostname,(wchar_t *__name, size_t __buflen),(__name,__buflen))
#endif /* __USE_UNIX98 || __USE_XOPEN2K */
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,wsethostname,(wchar_t const *__name, size_t __len),(__name,__len))
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((1)),int,__LIBCCALL,wgetdomainname,(wchar_t *__name, size_t __buflen),(__name,__buflen))
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,wsetdomainname,(wchar_t const *__name, size_t __len),(__name,__len))
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__REDIRECT_EXCEPT(__LIBC,__PORT_NODOS __WUNUSED __NONNULL((1)),wchar_t *,__LIBCCALL,wgetpass,(wchar_t const *__restrict __prompt),(__prompt))
#endif
#if defined(_ALL_SOURCE) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
#ifndef __wctermid_defined
#define __wctermid_defined 1
__LIBC __PORT_NODOS __WUNUSED_SUGGESTED wchar_t *(__LIBCCALL wctermid)(wchar_t *__s);
#endif /* !__wctermid_defined */
#endif /* __USE_XOPEN && !__USE_XOPEN2K */
#endif


__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,wchown,(wchar_t const *__file, __uid_t __owner, __gid_t __group),(__file,__owner,__group))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY __NONNULL((1)),long int,__LIBCCALL,wpathconf,(wchar_t const *__path, int __name),(__path,__name))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,wlink,(wchar_t const *__from, wchar_t const *__to),(__from,__to))
#ifdef __USE_ATFILE
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __IF_NUSE_EXCEPT(__WUNUSED) __NONNULL((2)),int,__LIBCCALL,wfaccessat,(__fd_t __dfd, wchar_t const *__file, int __type, __atflag_t __flags),(__dfd,__file,__type,__flag))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,wfchownat,(__fd_t __dfd, wchar_t const *__file, __uid_t __owner, __gid_t __group, __atflag_t __flags),(__dfd,__file,__owner,__group))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2,4)),int,__LIBCCALL,wlinkat,(__fd_t __fromfd, wchar_t const *__from, __fd_t __tofd, wchar_t const *__to, int __flags),(__fromfd,__from,__tofd,__to,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,3)),int,__LIBCCALL,wsymlinkat,(wchar_t const *__from, __fd_t __tofd, wchar_t const *__to),(__from,__tofd,__to))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,wreadlinkat,(__fd_t __dfd, wchar_t const *__restrict __path, wchar_t *__restrict __buf, size_t __buflen),(__dfd,__path,__buf,__buflen))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,wunlinkat,(__fd_t __dfd, wchar_t const *__name, int __flag),(__dfd,__name,__flag))
#endif /* __USE_ATFILE */

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#ifdef __USE_KOS
#define __TRUNCATE_OFF_T    __FS_TYPE(pos)
#define __TRUNCATE_OFF64_T  __pos64_t
#else
#define __TRUNCATE_OFF_T    __FS_TYPE(off)
#define __TRUNCATE_OFF64_T  __off64_t
#endif
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,wlchown,(wchar_t const *__file, __uid_t __owner, __gid_t __group),(__file,__owner,__group))
__REDIRECT_EXCEPT_UFS64_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,wtruncate,(wchar_t const *__file, __TRUNCATE_OFF_T __length),(__file,__length))
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,wtruncate64,(wchar_t const *__file, __TRUNCATE_OFF64_T __length),(__file,__length))
#endif /* __USE_LARGEFILE64 */
#undef __TRUNCATE_OFF_T
#undef __TRUNCATE_OFF64_T
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_POSIX2
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED,size_t,__LIBCCALL,
                  wconfstr,(int __name, wchar_t *__buf, size_t __buflen),(__name,__buf,__buflen))
#endif /* __USE_POSIX2 */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,wsymlink,(wchar_t const *__from, wchar_t const *__to),(__from,__to))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,wreadlink,(wchar_t const *__restrict __path, wchar_t *__restrict __buf, size_t __buflen),(__path,__buf,__buflen))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,wrevoke,(wchar_t const *__file),(__file))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,wacct,(wchar_t const *__name),(__name))
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_NODOS __NONNULL((1)),int,__LIBCCALL,wchroot,(wchar_t const *__restrict __path),(__path))
#endif
#endif /* __CRT_KOS */

#ifdef __CRT_DOS
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,wchdir,(wchar_t const *__path),(__path))
__REDIRECT_EXCEPT_UFSDPA(__LIBC,__XATTR_RETNONNULL __PORT_DOSONLY __WUNUSED_SUGGESTED,wchar_t *,__LIBCCALL,wgetcwd,(wchar_t *__buf, size_t __size),(__buf,__size))
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,wunlink,(wchar_t const *__name),(__name))
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,wrmdir,(wchar_t const *__path),(__path))
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__IF_NUSE_EXCEPT(__WUNUSED) __PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,waccess,(wchar_t const *__name, int __type),(__name,__type))
#ifdef __USE_GNU
#if defined(__DOS_COMPAT__) || !defined(__CRT_KOS)
__REDIRECT(__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,weuidaccess,(wchar_t const *__name, int __type),_waccess,(__name,__type))
__REDIRECT(__LIBC,__WUNUSED __PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,weaccess,(wchar_t const *__name, int __type),_waccess,(__name,__type))
__LOCAL __WUNUSED __XATTR_RETNONNULL __PORT_DOSONLY wchar_t *(__LIBCCALL wget_current_dir_name)(void) { return wgetcwd(NULL,0); }
#else /* __DOS_COMPAT__... */
__REDIRECT_EXCEPT_UFS_XVOID_(__LIBC,__IF_NUSE_EXCEPT(__WUNUSED) __PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,weuidaccess,(wchar_t const *__name, int __type),weaccess,(__name,__type))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__IF_NUSE_EXCEPT(__WUNUSED) __PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,weaccess,(wchar_t const *__name, int __type),(__name,__type))
__REDIRECT_EXCEPT_UFS(__LIBC,__WUNUSED __XATTR_RETNONNULL __PORT_DOSONLY,wchar_t *,__LIBCCALL,wget_current_dir_name,(void),())
#endif /* !__DOS_COMPAT__... */
#endif /* __USE_GNU */
#endif /* __CRT_DOS */
#endif /* !__KERNEL__ */


__SYSDECL_END

/* Pull in wide-character process definitions. */
#ifndef _PARTS_KOS2_WPROCESS_H
#include "wprocess.h"
#endif
#ifdef __USE_EXCEPT
#include <parts/kos3/except/wunistd.h>
#endif

#endif /* !_PARTS_KOS2_WUNISTD_H */
