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
#ifndef _PARTS_KOS3_EXCEPT_IO_H
#define _PARTS_KOS3_EXCEPT_IO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

/* Functions with the correct names, also present in other headers. */
#ifndef __Xremove_defined
#define __Xremove_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xremove)(char const *__file);
#endif /* !__Xremove_defined */
#ifndef __Xrename_defined
#define __Xrename_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xrename)(char const *__old, char const *__new);
#endif /* !__Xrename_defined */
#ifndef __Xunlink_defined
#define __Xunlink_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xunlink)(char const *__name);
#endif /* !__Xunlink_defined */
#ifndef __Xopen_defined
#define __Xopen_defined 1
__VREDIRECT_FS64(__LIBC,__PORT_KOSONLY __NONNULL((1)),__fd_t,__ATTR_CDECL,Xopen,(char const *__file, __oflag_t __oflag, ...),TODO,(__file,__oflag),__oflag)
#endif /* !__Xopen_defined */
#ifndef __Xcreat_defined
#define __Xcreat_defined 1
__REDIRECT_FS64(__LIBC,__PORT_KOSONLY __NONNULL((1)),__fd_t,__LIBCCALL,Xcreat,(char const *__file, __mode_t __mode),(__file,__mode))
#endif /* !__Xcreat_defined */
#ifndef __Xaccess_defined
#define __Xaccess_defined 1
__LIBC __NONNULL((1)) void (__LIBCCALL Xaccess)(char const *__name, int __type);
#endif /* !__Xaccess_defined */
#ifndef __Xchmod_defined
#define __Xchmod_defined 1
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xchmod)(char const *__file, __mode_t __mode);
#endif /* !__Xchmod_defined */
#ifndef __Xdup_defined
#define __Xdup_defined 1
__LIBC __PORT_KOSONLY __WUNUSED __fd_t (__LIBCCALL Xdup)(__fd_t __fd);
#endif /* !__Xdup_defined */
#ifndef __Xdup2_defined
#define __Xdup2_defined 1
__LIBC __PORT_KOSONLY __fd_t (__LIBCCALL Xdup2)(__fd_t __oldfd, __fd_t __newfd);
#endif /* !__Xdup2_defined */
#ifndef __Xlseek_defined
#define __Xlseek_defined 1
__REDIRECT_FS64(__LIBC,__PORT_KOSONLY,__FS_TYPE(pos),__LIBCCALL,Xlseek,(__fd_t __fd, __FS_TYPE(off) __offset, int __whence),(__fd,__offset,__whence))
#endif /* !__Xlseek_defined */
#ifndef __Xread_defined
#define __Xread_defined 1
__LIBC __PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((2)) __size_t (__LIBCCALL Xread)(__fd_t __fd, void *__dstbuf, __size_t __dstbufsize);
#endif /* !__Xread_defined */
#ifndef __Xwrite_defined
#define __Xwrite_defined 1
__LIBC __PORT_KOSONLY __NONNULL((2)) __size_t (__LIBCCALL Xwrite)(__fd_t __fd, void const *__srcbuf, __size_t __srcbufsize);
#endif /* !__Xwrite_defined */
__REDIRECT(__LIBC,__PORT_KOSONLY __WUNUSED __NONNULL((1)),__fd_t,__LIBCCALL,_Xcreat,(char const *__file, __mode_t __mode),Xcreat,(__file,__mode))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,_Xchmod,(char const *__file, __mode_t __mode),Xchmod,(__file,__mode))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,_Xchsize,(__fd_t __fd, __LONG32_TYPE__ __size),Xftruncate,(__fd,__size))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,_Xchsize_s,(__fd_t __fd, __INT64_TYPE__ __size),Xftruncate64,(__fd,__size))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,_Xcommit,(__fd_t __fd),Xfdatasync,(__fd))
__REDIRECT(__LIBC,__PORT_KOSONLY,__ULONG32_TYPE__,__LIBCCALL,_Xlseek,(__fd_t __fd, __LONG32_TYPE__ __offset, int __whence),Xlseek,(__fd,__offset,__whence))
__REDIRECT(__LIBC,__PORT_KOSONLY,__UINT64_TYPE__,__LIBCCALL,_Xlseeki64,(__fd_t __fd, __INT64_TYPE__ __offset, int __whence),Xlseek64,(__fd,__offset,__whence))
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,_Xlocking,(__fd_t __fd, int __lockmode, __LONG32_TYPE__ __numofbytes),Xlockf,(__fd,__lockmode,__numofbytes))
#ifndef ___Xunlink_defined
#define ___Xunlink_defined 1
__REDIRECT_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,_Xunlink,(char const *__name),Xunlink,(__name))
#endif /* !___Xunlink_defined */
__REDIRECT(__LIBC,__PORT_KOSONLY,__fd_t,__LIBCCALL,_Xdup,(__fd_t __fd),Xdup,(__fd))
__REDIRECT(__LIBC,__PORT_KOSONLY,__fd_t,__LIBCCALL,_Xdup2,(__fd_t __oldfd, __fd_t __newfd),Xdup2,(__oldfd,__newfd))
__REDIRECT(__LIBC,__PORT_KOSONLY __WUNUSED_SUGGESTED __NONNULL((2)),__size_t,__LIBCCALL,_Xread,(__fd_t __fd, void *__dstbuf, __size_t __bufsize),Xread,(__fd,__dstbuf,__bufsize))
__REDIRECT(__LIBC,__PORT_KOSONLY __NONNULL((2)),__size_t,__LIBCCALL,_Xwrite,(__fd_t __fd, void const *__buf, __size_t __bufsize),Xwrite,(__fd,__dstbuf,__bufsize))
__VREDIRECT(__LIBC,__PORT_KOSONLY __WUNUSED,__fd_t,__ATTR_CDECL,_Xopen,(char const *__file, __oflag_t __oflag, ...),Xopen,TODO,(__file,__oflag),__oflag)

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_IO_H */
