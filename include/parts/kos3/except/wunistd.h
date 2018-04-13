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
#ifndef _PARTS_KOS3_EXCEPT_WUNISTD_H
#define _PARTS_KOS3_EXCEPT_WUNISTD_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
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

#if 0
__REDIRECT_EXCEPT(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,wchar_t *,__LIBCCALL,wttyname,(__fd_t __fd),(__fd))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,wttyname_r,(__fd_t __fd, wchar_t *__buf, size_t __buflen),(__fd,__buf,__buflen))
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,wsetlogin,(wchar_t const *__name),(__name))
__REDIRECT_EXCEPT(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED,wchar_t *,__LIBCCALL,wgetlogin,(void),())
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


__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwchown)(wchar_t const *__file, __uid_t __owner, __gid_t __group);
__LIBC __PORT_KOSONLY __NONNULL((1)) long int (__LIBCCALL Xwpathconf)(wchar_t const *__path, int __name);
__LIBC __PORT_KOSONLY __NONNULL((1,2)) void (__LIBCCALL Xwlink)(wchar_t const *__from, wchar_t const *__to);
#ifdef __USE_ATFILE
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xwfaccessat)(__fd_t __dfd, wchar_t const *__file, int __type, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xwfchownat)(__fd_t __dfd, wchar_t const *__file, __uid_t __owner, __gid_t __group, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((2,4)) void (__LIBCCALL Xwlinkat)(__fd_t __fromfd, wchar_t const *__from, __fd_t __tofd, wchar_t const *__to, int __flags);
__LIBC __PORT_KOSONLY __NONNULL((1,3)) void (__LIBCCALL Xwsymlinkat)(wchar_t const *__from, __fd_t __tofd, wchar_t const *__to);
__LIBC __PORT_KOSONLY __NONNULL((2,3)) size_t (__LIBCCALL Xwreadlinkat)(__fd_t __dfd, wchar_t const *__restrict __path, wchar_t *__restrict __buf, size_t __buflen);
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xwunlinkat)(__fd_t __dfd, wchar_t const *__name, int __flag);
#endif /* __USE_ATFILE */

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#ifdef __USE_KOS
#define __TRUNCATE_OFF_T    __FS_TYPE(pos)
#define __TRUNCATE_OFF64_T  __pos64_t
#else
#define __TRUNCATE_OFF_T    __FS_TYPE(off)
#define __TRUNCATE_OFF64_T  __off64_t
#endif
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwlchown)(wchar_t const *__file, __uid_t __owner, __gid_t __group);
__REDIRECT_FS64_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xwtruncate,(wchar_t const *__file, __TRUNCATE_OFF_T __length),(__file,__length))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwtruncate64)(wchar_t const *__file, __TRUNCATE_OFF64_T __length);
#endif /* __USE_LARGEFILE64 */
#undef __TRUNCATE_OFF_T
#undef __TRUNCATE_OFF64_T
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_POSIX2
__LIBC __PORT_KOSONLY __WUNUSED_SUGGESTED size_t (__LIBCCALL Xwconfstr)(int __name, wchar_t *__buf, size_t __buflen);
#endif /* __USE_POSIX2 */
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K)
__LIBC __PORT_KOSONLY __NONNULL((1,2)) void (__LIBCCALL Xwsymlink)(wchar_t const *__from, wchar_t const *__to);
__LIBC __PORT_KOSONLY __NONNULL((1,2)) size_t (__LIBCCALL Xwreadlink)(wchar_t const *__restrict __path, wchar_t *__restrict __buf, size_t __buflen);
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K */
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwrevoke)(wchar_t const *__file);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xwacct)(wchar_t const *__name);
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwchroot)(wchar_t const *__restrict __path);
#endif

__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwchdir)(wchar_t const *__path);
__LIBC __ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED_SUGGESTED wchar_t *(__LIBCCALL Xwgetcwd)(wchar_t *__buf, size_t __size);
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwunlink)(wchar_t const *__name);
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwrmdir)(wchar_t const *__path);
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwaccess)(wchar_t const *__name, int __type);
#ifdef __USE_GNU
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xweuidaccess,(wchar_t const *__name, int __type),Xweaccess,(__name,__type))
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xweaccess)(wchar_t const *__name, int __type);
__LIBC __WUNUSED __ATTR_RETNONNULL __PORT_KOSONLY wchar_t *(__LIBCCALL Xwget_current_dir_name)(void);
#endif /* __USE_GNU */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_WUNISTD_H */
