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
#ifndef _PARTS_KOS2_STDLIB_H
#define _PARTS_KOS2_STDLIB_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifndef __KERNEL__
#ifdef __CRT_KOS
/* KOS extensions for accessing the kernel linker without needing to link against -ldl.
 * NOTE: Since KOS integrates the user-space linker in kernel-space, so as to speed up
 *       load times and especially cache characteristics significantly, there is no need
 *       for libraries to be loaded by a custom binary that would otherwise always sit
 *       in user-space, clobbering mapped memory just for the sake of it...
 *    >> Don't believe me? - Take a look at any dynamic binary's /proc/PID/map_files and
 *       tell me what you see. I'll tell you: '/lib/i386-linux-gnu/ld-2.23.so'
 *       That's right! That one's always linked in just to serve dynamic linking.
 *       Now that's not necessarily a bad thing, but when it comes to KOS, I chose
 *       to go a different route by hiding general purpose linking from userspace,
 *       keeping redundancy low and eliminating dependency on a second library probably
 *       even more important than libc itself.
 * NOTE: These functions follow usual LIBC semantics, returning -1/NULL and setting errno on error.
 *    >> the `dlerror()' function you can find in libdl.so is literally just a swapper
 *       around `strerror()' with the error number internally saved by libdl, so as not
 *       to clobber libc's thread-local `errno' variable. */
#ifndef __xdlopen_defined
#define __xdlopen_defined 1
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY_ALT(dlopen),void *,__LIBCCALL,xdlopen,(char const *__filename, int __flags),(__filename,__flags))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY_ALT(dlopen),void *,__LIBCCALL,xfdlopen,(__fd_t __fd, int __flags),(__fd,__flags))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY_ALT(dlsym),void *,__LIBCCALL,xdlsym,(void *__handle, char const *__symbol),(__handle,__symbol))
__LIBC __PORT_KOSONLY_ALT(dlclose) int (__LIBCCALL xdlclose)(void *__handle);
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL void *(__LIBCCALL Xxdlopen)(char const *__filename, int __flags);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL void *(__LIBCCALL Xxfdlopen)(__fd_t __fd, int __flags);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL void *(__LIBCCALL Xxdlsym)(void *__handle, char const *__symbol);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xxdlclose)(void *__handle);
#endif /* __USE_EXCEPT */
#endif /* !__xdlopen_defined */
#endif /* __CRT_KOS */
#endif /* !__KERNEL__ */

/* Debug hooks for malloc() and friends. */
#ifdef __USE_DEBUG
#if __KOS_VERSION__ < 300 && __USE_DEBUG != 0 && defined(__CRT_KOS) && \
  (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
#ifdef __USE_MISC
#ifndef __cfree_d_defined
#define __cfree_d_defined 1
#ifdef __KERNEL__
__REDIRECT_VOID(__LIBC,,__LIBCCALL,_cfree_d,(void *__restrict __mallptr, __DEBUGINFO),_kfree_d,(__mallptr,__DEBUGINFO_FWD))
#else /* __KERNEL__ */
__REDIRECT_VOID(__LIBC,,__LIBCCALL,_cfree_d,(void *__restrict __mallptr, __DEBUGINFO),_free_d,(__mallptr,__DEBUGINFO_FWD))
#endif /* !__KERNEL__ */
#endif /* !__cfree_d_defined */
#endif /* __USE_MISC */
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K))
#ifndef __valloc_d_defined
#define __valloc_d_defined 1
__LIBC __WUNUSED __MALL_ATTR_PAGEALIGNED __ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL _valloc_d)(size_t __n_bytes, __DEBUGINFO);
#endif /* !__valloc_d_defined */
#endif
#ifdef __USE_XOPEN2K
#ifndef __posix_memalign_d_defined
#define __posix_memalign_d_defined 1
__LIBC __NONNULL((1)) int (__LIBCCALL _posix_memalign_d)(void **__restrict __pp, size_t __alignment, size_t __n_bytes, __DEBUGINFO);
#endif /* !__posix_memalign_d_defined */
#endif /* __USE_XOPEN2K */
#ifdef __USE_ISOC11
__REDIRECT(__LIBC,__WUNUSED __ATTR_ALLOC_ALIGN(1) __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC,void *,__LIBCCALL,
           _aligned_alloc_d,(size_t __alignment, size_t __n_bytes, __DEBUGINFO),_memalign_d,(__alignment,__n_bytes,__DEBUGINFO_FWD))
#endif /* __USE_ISOC11 */

#ifdef __USE_DEBUG_HOOK
#ifdef __USE_MISC
#   undef  cfree
#   define cfree(ptr)                                _cfree_d(ptr,__DEBUGINFO_GEN)
#endif /* __USE_MISC */
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K))
#   undef  valloc
#   define valloc(n_bytes)                           _valloc_d(n_bytes,__DEBUGINFO_GEN)
#endif
#ifdef __USE_XOPEN2K
#   undef  posix_memalign
#   define posix_memalign(pp,alignment,n_bytes)      _posix_memalign_d(pp,alignment,n_bytes,__DEBUGINFO_GEN)
#endif /* __USE_XOPEN2K */
#ifdef __USE_ISOC11
#   undef  aligned_alloc
#   define aligned_alloc(alignment,n_bytes)          _aligned_alloc_d(alignment,n_bytes,__DEBUGINFO_GEN)
#endif /* __USE_ISOC11 */
#endif /* __USE_DEBUG_HOOK */

#else /* __USE_DEBUG != 0 */

#ifdef __USE_MISC
#ifndef __cfree_d_defined
#define __cfree_d_defined 1
#   define _cfree_d(ptr,...)                           cfree(ptr)
#endif /* !__cfree_d_defined */
#endif /* __USE_MISC */
#ifdef __CRT_GLC
#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K))
#ifndef __valloc_d_defined
#define __valloc_d_defined 1
#   define _valloc_d(n_bytes,...)                      valloc(n_bytes)
#endif /* !__valloc_d_defined */
#endif
#ifdef __USE_XOPEN2K
#ifndef __posix_memalign_d_defined
#define __posix_memalign_d_defined 1
#   define _posix_memalign_d(pp,alignment,n_bytes,...) posix_memalign(pp,alignment,n_bytes)
#endif /* !__posix_memalign_d_defined */
#endif /* __USE_XOPEN2K */
#ifdef __USE_ISOC11
#   define _aligned_alloc_d(alignment,n_bytes,...)     aligned_alloc(alignment,n_bytes)
#endif /* __USE_ISOC11 */
#endif /* __CRT_GLC */

#endif /* __USE_DEBUG == 0 */
#endif /* __USE_DEBUG */

__SYSDECL_END

#endif /* !_PARTS_KOS2_STDLIB_H */
