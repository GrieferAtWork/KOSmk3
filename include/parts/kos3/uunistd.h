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
#ifndef _PARTS_KOS3_UUNISTD_H
#define _PARTS_KOS3_UUNISTD_H 1

#include "__stdinc.h"
#include <features.h>
#include <bits/types.h>
#include <hybrid/typecore.h>

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

__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16fchdirat,(__fd_t __dfd, char16_t const *__path, __atflag_t __flags),wfchdirat,(__dfd,__path,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32fchdirat,(__fd_t __dfd, char32_t const *__path, __atflag_t __flags),wfchdirat,(__dfd,__path,__flags))
__REDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,w16freadlinkat,(__fd_t __dfd, char16_t const *__restrict __path, char16_t *__restrict __buf, size_t __buflen, int __flags),wfreadlinkat,(__dfd,__path,__buf,__buflen,__flags))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,w32freadlinkat,(__fd_t __dfd, char32_t const *__restrict __path, char32_t *__restrict __buf, size_t __buflen, int __flags),wfreadlinkat,(__dfd,__path,__buf,__buflen,__flags))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,3)),int,__LIBCCALL,w16fsymlinkat,(char16_t const *__from, __fd_t __tofd, char16_t const *__to, __atflag_t __flags),wfsymlinkat,(__from,__tofd,__to,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,3)),int,__LIBCCALL,w32fsymlinkat,(char32_t const *__from, __fd_t __tofd, char32_t const *__to, __atflag_t __flags),wfsymlinkat,(__from,__tofd,__to,__flags))
__REDIRECT_EXCEPT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,w16freadlink,(__fd_t __fd, char16_t *__restrict __buf, size_t __buflen),wfreadlink,(__fd,__buf,__buflen))
__REDIRECT_EXCEPT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2)),__EXCEPT_SELECT(size_t,ssize_t),__LIBCCALL,w32freadlink,(__fd_t __fd, char32_t *__restrict __buf, size_t __buflen),wfreadlink,(__fd,__buf,__buflen))
#ifdef __USE_FILE_OFFSET64
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16ftruncateat,(__fd_t __dfd, char16_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),wftruncateat64,(__dfd,__file,__length,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32ftruncateat,(__fd_t __dfd, char32_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),wftruncateat64,(__dfd,__file,__length,__flags))
#else
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16ftruncateat,(__fd_t __dfd, char16_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),wftruncateat,(__dfd,__file,__length,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32ftruncateat,(__fd_t __dfd, char32_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),wftruncateat,(__dfd,__file,__length,__flags))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w16ftruncateat64,(__fd_t __dfd, char16_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),wftruncateat64,(__dfd,__file,__length,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),int,__LIBCCALL,w32ftruncateat64,(__fd_t __dfd, char32_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),wftruncateat64,(__dfd,__file,__length,__flags))
#endif
#ifdef __USE_EXCEPT
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16fchdirat,(__fd_t __dfd, char16_t const *__path, __atflag_t __flags),Xwfchdirat,(__dfd,__path,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32fchdirat,(__fd_t __dfd, char32_t const *__path, __atflag_t __flags),Xwfchdirat,(__dfd,__path,__flags))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),size_t,__LIBCCALL,Xw16freadlinkat,(__fd_t __dfd, char16_t const *__restrict __path, char16_t *__restrict __buf, size_t __buflen, __atflag_t __flags),Xwfreadlinkat,(__dfd,__path,__buf,__buflen,__flags))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2,3)),size_t,__LIBCCALL,Xw32freadlinkat,(__fd_t __dfd, char32_t const *__restrict __path, char32_t *__restrict __buf, size_t __buflen, __atflag_t __flags),Xwfreadlinkat,(__dfd,__path,__buf,__buflen,__flags))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,3)),__LIBCCALL,Xw16fsymlinkat,(char16_t const *__from, __fd_t __tofd, char16_t const *__to, __atflag_t __flags),Xwfsymlinkat,(__from,__tofd,__to,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,3)),__LIBCCALL,Xw32fsymlinkat,(char32_t const *__from, __fd_t __tofd, char32_t const *__to, __atflag_t __flags),Xwfsymlinkat,(__from,__tofd,__to,__flags))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2)),size_t,__LIBCCALL,Xw16freadlink,(__fd_t __fd, char16_t *__restrict __buf, size_t __buflen),Xwfreadlink,(__fd,__buf,__buflen))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2)),size_t,__LIBCCALL,Xw32freadlink,(__fd_t __fd, char32_t *__restrict __buf, size_t __buflen),Xwfreadlink,(__fd,__buf,__buflen))
#ifdef __USE_FILE_OFFSET64
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16ftruncateat,(__fd_t __dfd, char16_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),Xwftruncateat64,(__dfd,__file,__length,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32ftruncateat,(__fd_t __dfd, char32_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),Xwftruncateat64,(__dfd,__file,__length,__flags))
#else
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16ftruncateat,(__fd_t __dfd, char16_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),Xwftruncateat,(__dfd,__file,__length,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32ftruncateat,(__fd_t __dfd, char32_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),Xwftruncateat,(__dfd,__file,__length,__flags))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw16ftruncateat64,(__fd_t __dfd, char16_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),Xwftruncateat64,(__dfd,__file,__length,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((2)),__LIBCCALL,Xw32ftruncateat64,(__fd_t __dfd, char32_t const *__file, __FS_TYPE(off) __length, __atflag_t __flags),Xwftruncateat64,(__dfd,__file,__length,__flags))
#endif
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
__REDIRECT_EXCEPT_W16(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY,char16_t *,__LIBCCALL,xw16frealpath,(__fd_t __fd, char16_t *__buf, __size_t __bufsize, unsigned int __type),xwfrealpath,(__dfd,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_W32(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY,char32_t *,__LIBCCALL,xw32frealpath,(__fd_t __fd, char32_t *__buf, __size_t __bufsize, unsigned int __type),xwfrealpath,(__dfd,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFSW16(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY,char16_t *,__LIBCCALL,xw16realpath,(char16_t const *__path, char16_t *__buf, __size_t __bufsize, unsigned int __type),xwrealpath,(__path,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY,char32_t *,__LIBCCALL,xw32realpath,(char32_t const *__path, char32_t *__buf, __size_t __bufsize, unsigned int __type),xwrealpath,(__path,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFSW16(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY,char16_t *,__LIBCCALL,xw16frealpathat,(__fd_t __dfd, char16_t const *__path, __atflag_t __flags, char16_t *__buf, __size_t __bufsize, unsigned int __type),xwfrealpathat,(__dfd,__path,__flags,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY,char32_t *,__LIBCCALL,xw32frealpathat,(__fd_t __dfd, char32_t const *__path, __atflag_t __flags, char32_t *__buf, __size_t __bufsize, unsigned int __type),xwfrealpathat,(__dfd,__path,__flags,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_W16(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xw16frealpath2,(__fd_t __fd, char16_t *__buf, __size_t __bufsize, unsigned int __type),xwfrealpath2,(__dfd,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_W32(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xw32frealpath2,(__fd_t __fd, char32_t *__buf, __size_t __bufsize, unsigned int __type),xwfrealpath2,(__dfd,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xw16realpath2,(char16_t const *__path, char16_t *__buf, __size_t __bufsize, unsigned int __type),xwrealpath2,(__path,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xw32realpath2,(char32_t const *__path, char32_t *__buf, __size_t __bufsize, unsigned int __type),xwrealpath2,(__path,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xw16frealpathat2,(__fd_t __dfd, char16_t const *__path, __atflag_t __flags, char16_t *__buf, __size_t __bufsize, unsigned int __type),xwfrealpathat2,(__dfd,__path,__flags,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xw32frealpathat2,(__fd_t __dfd, char32_t const *__path, __atflag_t __flags, char32_t *__buf, __size_t __bufsize, unsigned int __type),xwfrealpathat2,(__dfd,__path,__flags,__buf,__bufsize,__type))
#ifdef __USE_EXCEPT
__REDIRECT_W16(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY,char16_t *,__LIBCCALL,Xxw16frealpath,(__fd_t __fd, char16_t *__buf, __size_t __bufsize, unsigned int __type),Xxwfrealpath,(__dfd,__buf,__bufsize,__type))
__REDIRECT_W32(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY,char32_t *,__LIBCCALL,Xxw32frealpath,(__fd_t __fd, char32_t *__buf, __size_t __bufsize, unsigned int __type),Xxwfrealpath,(__dfd,__buf,__bufsize,__type))
__REDIRECT_W16(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY,char16_t *,__LIBCCALL,Xxw16realpath,(char16_t const *__path, char16_t *__buf, __size_t __bufsize, unsigned int __type),Xxwrealpath,(__path,__buf,__bufsize,__type))
__REDIRECT_W32(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY,char32_t *,__LIBCCALL,Xxw32realpath,(char32_t const *__path, char32_t *__buf, __size_t __bufsize, unsigned int __type),Xxwrealpath,(__path,__buf,__bufsize,__type))
__REDIRECT_W16(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY,char16_t *,__LIBCCALL,Xxw16frealpathat,(__fd_t __dfd, char16_t const *__path, __atflag_t __flags, char16_t *__buf, __size_t __bufsize, unsigned int __type),Xxwfrealpathat,(__dfd,__path,__flags,__buf,__bufsize,__type))
__REDIRECT_W32(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY,char32_t *,__LIBCCALL,Xxw32frealpathat,(__fd_t __dfd, char32_t const *__path, __atflag_t __flags, char32_t *__buf, __size_t __bufsize, unsigned int __type),Xxwfrealpathat,(__dfd,__path,__flags,__buf,__bufsize,__type))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY,__size_t,__LIBCCALL,Xxw16frealpath2,(__fd_t __fd, char16_t *__buf, __size_t __bufsize, unsigned int __type),Xxwfrealpath2,(__dfd,__buf,__bufsize,__type))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY,__size_t,__LIBCCALL,Xxw32frealpath2,(__fd_t __fd, char32_t *__buf, __size_t __bufsize, unsigned int __type),Xxwfrealpath2,(__dfd,__buf,__bufsize,__type))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY,__size_t,__LIBCCALL,Xxw16realpath2,(char16_t const *__path, char16_t *__buf, __size_t __bufsize, unsigned int __type),Xxwrealpath2,(__path,__buf,__bufsize,__type))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY,__size_t,__LIBCCALL,Xxw32realpath2,(char32_t const *__path, char32_t *__buf, __size_t __bufsize, unsigned int __type),Xxwrealpath2,(__path,__buf,__bufsize,__type))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY,__size_t,__LIBCCALL,Xxw16frealpathat2,(__fd_t __dfd, char16_t const *__path, __atflag_t __flags, char16_t *__buf, __size_t __bufsize, unsigned int __type),Xxwfrealpathat2,(__dfd,__path,__flags,__buf,__bufsize,__type))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY,__size_t,__LIBCCALL,Xxw32frealpathat2,(__fd_t __dfd, char32_t const *__path, __atflag_t __flags, char32_t *__buf, __size_t __bufsize, unsigned int __type),Xxwfrealpathat2,(__dfd,__path,__flags,__buf,__bufsize,__type))
#endif /* __USE_EXCEPT */
#endif /* __CRT_KOS */
#endif /* !__KERNEL__ */

__SYSDECL_END

/* Pull in wide-character process definitions. */
#ifndef _PARTS_KOS3_UPROCESS_H
#include "uprocess.h"
#endif


#endif /* !_PARTS_KOS3_UUNISTD_H */
