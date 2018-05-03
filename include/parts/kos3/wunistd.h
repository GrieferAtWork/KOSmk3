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
#ifndef _PARTS_KOS3_WUNISTD_H
#define _PARTS_KOS3_WUNISTD_H 1

#include "__stdinc.h"
#include <features.h>
#include <bits/types.h>
#include <hybrid/typecore.h>

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
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,wfchdirat,(__fd_t __dfd, wchar_t const *__path, __atflag_t __flags),(__dfd,__path,__flags))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,wfreadlinkat,(__fd_t __dfd, wchar_t const *__restrict __path, wchar_t *__restrict __buf, size_t __buflen, __atflag_t __flags),(__dfd,__path,__buf,__buflen,__flags))
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,3)),int,__LIBCCALL,wfsymlinkat,(wchar_t const *__from, __fd_t __tofd, wchar_t const *__to, __atflag_t __flags),(__from,__tofd,__to,__flags))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY __NONNULL((2)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,wfreadlink,(__fd_t __fd, wchar_t *__restrict __buf, size_t __buflen),(__fd,__buf,__buflen))
__REDIRECT_EXCEPT_UFS64_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,wftruncateat,(__fd_t __dfd, wchar_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),(__dfd,__file,__length,__flags))
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,wftruncateat64,(__fd_t __dfd, wchar_t const *__file, __off64_t __length, __atflag_t __flags),(__dfd,__file,__length,__flags))
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xwfchdirat)(__fd_t __dfd, wchar_t const *__path, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((2,3)) size_t (__LIBCCALL Xwfreadlinkat)(__fd_t __dfd, wchar_t const *__restrict __path, wchar_t *__restrict __buf, size_t __buflen, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((1,3)) void (__LIBCCALL Xwfsymlinkat)(wchar_t const *__from, __fd_t __tofd, wchar_t const *__to, __atflag_t __flags);
__LIBC __PORT_KOSONLY __NONNULL((2)) size_t (__LIBCCALL Xwfreadlink)(__fd_t __fd, wchar_t *__restrict __buf, size_t __buflen);
__REDIRECT_FS64_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xwftruncateat,(__fd_t __dfd, wchar_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),(__dfd,__file,__length,__flags))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY __NONNULL((2)) void (__LIBCCALL Xwftruncateat64)(__fd_t __dfd, wchar_t const *__file, __off64_t __length, __atflag_t __flags);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_EXCEPT */

/* Resolve a `PATH' relative to `FD`, using `FLAGS' and fill in
 * the given `BUF...+=BUFSIZE' with its name according to `TYPE'.
 * HINT: `xfdname()' can be implemented as `xfrealpathat(fd,"",AT_EMPTY_PATH,...)'
 * @param: FD:          A file descriptor for the base path to lookup.
 * @param: PATH:        A path relative to `FD' (can be empty when `AT_EMPTY_PATH' is passed)
 * @param: FLAGS:       Set of `AT_*' (AT-path constants)
 * @param: BUF:         A buffer to be filled with the name of the path.
 *                      [xfrealpathat] When NULL, malloc() a new buffer of `BUFSIZE' bytes, or if
 *                                    `BUFSIZE' is ZERO(0), allocate a buffer of appropriate size.
 * @param: BUFSIZE:     The size of the provided `BUF' (in characters)
 * @param: TYPE:        The type of path that should be returned. (Set of `REALPATH_F*'; see above)
 * @return: * :         [xfrealpathat] Either `BUF', or the newly allocated buffer when `BUF' was NULL
 * @return: NULL:       [xfrealpathat] An error occurred (see `errno')
 * @return: * :         [xfrealpathat2] The required buffer size (in characters), excluding a terminated NUL-character
 * @return: >= BUFSIZE: [xfrealpathat2] Only a portion of the path was printed. Pass a buffer capable of holding at least `return+1' characters.
 * @return: -1 :        [xfrealpathat2] An error occurred (see `errno') */
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY,wchar_t *,__LIBCCALL,xwfrealpath,(__fd_t __fd, wchar_t *__buf, __size_t __bufsize, unsigned int __type),(__fd,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFS(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY,wchar_t *,__LIBCCALL,xwrealpath,(wchar_t const *__path, wchar_t *__buf, __size_t __bufsize, unsigned int __type),(__path,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFS(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY,wchar_t *,__LIBCCALL,xwfrealpathat,(__fd_t __dfd, wchar_t const *__path, __atflag_t __flags, wchar_t *__buf, __size_t __bufsize, unsigned int __type),(__dfd,__path,__flags,__buf,__bufsize,__type))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,xwfrealpath2,(__fd_t __fd, wchar_t *__buf, __size_t __bufsize, unsigned int __type),(__fd,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,xwrealpath2,(wchar_t const *__path, wchar_t *__buf, __size_t __bufsize, unsigned int __type),(__path,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,xwfrealpathat2,(__fd_t __dfd, wchar_t const *__path, __atflag_t __flags, wchar_t *__buf, __size_t __bufsize, unsigned int __type),(__dfd,__path,__flags,__buf,__bufsize,__type))
#ifdef __USE_EXCEPT
__LIBC __ATTR_RETNONNULL __PORT_KOSONLY wchar_t *(__LIBCCALL xwfrealpath)(__fd_t __fd, wchar_t *__buf, __size_t __bufsize, unsigned int __type);
__LIBC __ATTR_RETNONNULL __PORT_KOSONLY wchar_t *(__LIBCCALL xwrealpath)(wchar_t const *__path, wchar_t *__buf, __size_t __bufsize, unsigned int __type);
__LIBC __ATTR_RETNONNULL __PORT_KOSONLY wchar_t *(__LIBCCALL xwfrealpathat)(__fd_t __dfd, wchar_t const *__path, __atflag_t __flags, wchar_t *__buf, __size_t __bufsize, unsigned int __type);
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL xwfrealpath2)(__fd_t __fd, wchar_t *__buf, __size_t __bufsize, unsigned int __type);
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL xwrealpath2)(wchar_t const *__path, wchar_t *__buf, __size_t __bufsize, unsigned int __type);
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL xwfrealpathat2)(__fd_t __dfd, wchar_t const *__path, __atflag_t __flags, wchar_t *__buf, __size_t __bufsize, unsigned int __type);
#endif /* __USE_EXCEPT */
#endif /* __CRT_KOS */
#endif /* !__KERNEL__ */

__SYSDECL_END

/* Pull in wide-character process definitions. */
#ifndef _PARTS_KOS3_WPROCESS_H
#include "wprocess.h"
#endif


#endif /* !_PARTS_KOS3_WUNISTD_H */
