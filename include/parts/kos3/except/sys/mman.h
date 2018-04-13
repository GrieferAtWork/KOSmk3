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
#ifndef _PARTS_KOS3_EXCEPT_SYS_MMAN_H
#define _PARTS_KOS3_EXCEPT_SYS_MMAN_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

#ifndef __off_t_defined
#define __off_t_defined 1
typedef __typedef_off_t off_t;
#endif /* !__off_t_defined */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __size_t size_t;
#endif /* !__size_t_defined */

#ifndef __mode_t_defined
#define __mode_t_defined 1
typedef __mode_t mode_t;
#endif /* !__mode_t_defined */

__REDIRECT_FS64(__LIBC,__PORT_KOSONLY __WUNUSED,void *,__LIBCCALL,Xmmap,
               (void *__addr, size_t __len, int __prot, int __flags, __fd_t __fd, __FS_TYPE(off) __offset),
               (__addr,__len,__prot,__flags,__fd,__offset))
__LIBC __PORT_KOSONLY void (__LIBCCALL Xmunmap)(void *__addr, size_t __len);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xmprotect)(void *__addr, size_t __len, int __prot);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xmsync)(void *__addr, size_t __len, int __flags);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xmlock)(void const *__addr, size_t __len);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xmunlock)(void const *__addr, size_t __len);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xmlockall)(int __flags);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xmunlockall)(void);
__LIBC __PORT_KOSONLY int (__LIBCCALL Xshm_open)(char const *__name, int __oflag, mode_t __mode);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xshm_unlink)(char const *__name);
#ifdef __USE_MISC
__LIBC __PORT_KOSONLY void (__LIBCCALL Xmadvise)(void *__addr, size_t __len, int __advice);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xmincore)(void *__start, size_t __len, unsigned char *__vec);
#endif /* __USE_MISC */
#ifdef __USE_LARGEFILE64
__LIBC __PORT_KOSONLY void *(__LIBCCALL Xmmap64)(void *__addr, size_t __len, int __prot, int __flags, __fd_t __fd, __off64_t __offset);
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_XOPEN2K
__LIBC __PORT_KOSONLY void (__LIBCCALL Xposix_madvise)(void *__addr, size_t __len, int __advice);
#endif /* __USE_XOPEN2K */
#ifdef __USE_GNU
__LIBC __PORT_KOSONLY void *(__ATTR_CDECL Xmremap)(void *__addr, size_t __old_len, size_t __new_len, int __flags, ...);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xremap_file_pages)(void *__start, size_t __size, int __prot, size_t __pgoff, int __flags);
#endif /* __USE_GNU */
__REDIRECT(__LIBC,__PORT_KOSONLY,void *,__LIBCCALL,Xxmmap,(struct mmap_info const *__data),Xxmmap1,(__data))
/* @param: flags: Set of `XUNMAP_*'; @return: * : Number of modified bytes */
__LIBC __PORT_KOSONLY size_t (__LIBCCALL Xxmunmap)(void *__addr, size_t __len, int __flags, void *__tag);
#if __KOS_VERSION__ >= 300
/* @param: flags: Set of `XUNMAP_*'; @return: * : Number of modified bytes */
__LIBC __PORT_KOSONLY size_t (__LIBCCALL Xxmprotect)(void *__addr, size_t __len, int __protmask, int __protflag, int __flags, void *__tag);
#endif

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SYS_MMAN_H */
