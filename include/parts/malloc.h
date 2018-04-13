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
#ifndef _PARTS_MALLOC_H
#define _PARTS_MALLOC_H 1

#include <__stdinc.h>


__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif

#ifdef __CRT_KOS
/* malloc behavior attributes. */
#define __MALLOC_ZERO_IS_NONNULL  1
#define __REALLOC_ZERO_IS_NONNULL 1
#endif


#if !defined(__KERNEL__) || __KOS_VERSION__ < 300

/* Malloc implementation notes:
 * #if __KOS_VERSION__ >= 300
 *  - libk does not provide malloc(). You must use `kmalloc()' from `<kernel/malloc.h>'
 *  - libc uses <kos/heap.h> to implement its malloc() function.
 * #else
 *  - libk uses `kmalloc()' from `/src/include/kernel/malloc.h'
 *    All allocation functions will pass `GFP_NORMAL' for flags,
 *    meaning that `malloc()' within the kernel will allocate
 *    shared memory above `KERNEL_BASE', that is visible in
 *    all page directories.
 *  - libc uses dlmalloc build on top of `<sys/mman.h>'
 * #endif
 *  - `malloc(0)' does NOT return `NULL', but some small, non-empty block of memory.
 *  - `realloc(p,0)' does NOT act as `free(p)', but return some small, non-empty block of memory.
 *  - `free()' never modifies the currently set value of `errno', even when an underlying `munmap()' fails.
 *  - Any allocation function failing in libc will set `errno' to `ENOMEM' 
 */
__NAMESPACE_STD_BEGIN
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC,void *,__LIBCCALL,malloc,(size_t __n_bytes),(__n_bytes))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1,2)) __ATTR_MALLOC,void *,__LIBCCALL,calloc,(size_t __count, size_t __n_bytes),(__count,__n_bytes))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)),void *,__LIBCCALL,realloc,(void *__restrict __mallptr, size_t __n_bytes),(__mallptr,__n_bytes))
#ifdef __KERNEL__
__REDIRECT_VOID(__LIBC,,__LIBCCALL,free,(void *__restrict __mallptr),kfree,(__mallptr))
#else /* __KERNEL__ */
__LIBC void (__LIBCCALL free)(void *__restrict __mallptr);
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(malloc)
__NAMESPACE_STD_USING(calloc)
__NAMESPACE_STD_USING(realloc)
__NAMESPACE_STD_USING(free)
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL Xmalloc)(size_t __n_bytes);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1,2)) __ATTR_MALLOC void *(__LIBCCALL Xcalloc)(size_t __count, size_t __n_bytes);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL Xrealloc)(void *__restrict __mallptr, size_t __n_bytes);
#endif /* __USE_EXCEPT */


#ifdef __USE_DEBUG
#if __KOS_VERSION__ < 300 && __USE_DEBUG != 0 && defined(__CRT_KOS) && \
  (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
#ifndef __std_malloc_calloc_d_defined
#define __std_malloc_calloc_d_defined 1
/* Must also define these within the std:: namespace to allow the debug macro overrides to work:
 * >> namespace std {
 * >>    void *malloc(size_t);
 * >>    void *_malloc_d(size_t, __DEBUGINFO);
 * >> }
 * >> using std::malloc;
 * >> using std::_malloc_d;
 * >> 
 * >> #define malloc(s) _malloc_d(s,__DEBUGINFO_GEN)
 * >>
 * >> void *p = std::malloc(42); // Expands to `std::_malloc_d(42,...)'
 */
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL _malloc_d)(size_t __n_bytes, __DEBUGINFO);
__LIBC __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1,2)) __ATTR_MALLOC void *(__LIBCCALL _calloc_d)(size_t __count, size_t __n_bytes, __DEBUGINFO);
__LIBC __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL _realloc_d)(void *__restrict __mallptr, size_t __n_bytes, __DEBUGINFO);
#ifdef __KERNEL__
__REDIRECT_VOID(__LIBC,,__LIBCCALL,_free_d,(void *__restrict __mallptr, __DEBUGINFO),_kfree_d,(__mallptr,__DEBUGINFO_FWD))
#else /* __KERNEL__ */
__LIBC void (__LIBCCALL _free_d)(void *__restrict __mallptr, __DEBUGINFO);
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END
#endif /* !__std_malloc_calloc_d_defined */

#ifndef __CXX_SYSTEM_HEADER
#ifndef __malloc_calloc_d_defined
#define __malloc_calloc_d_defined 1
__NAMESPACE_STD_USING(_malloc_d)
__NAMESPACE_STD_USING(_calloc_d)
__NAMESPACE_STD_USING(_realloc_d)
__NAMESPACE_STD_USING(_free_d)
#endif /* !__malloc_calloc_d_defined */
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __USE_DEBUG_HOOK
#undef  malloc
#undef  calloc
#undef  realloc
#undef  free
#define malloc(n_bytes)                           _malloc_d(n_bytes,__DEBUGINFO_GEN)
#define calloc(count,n_bytes)                     _calloc_d(count,n_bytes,__DEBUGINFO_GEN)
#define realloc(ptr,n_bytes)                      _realloc_d(ptr,n_bytes,__DEBUGINFO_GEN)
#define free(ptr)                                 _free_d(ptr,__DEBUGINFO_GEN)
#endif /* __USE_DEBUG_HOOK */

#else /* __USE_DEBUG != 0 */

#ifndef __malloc_calloc_d_defined
#define __malloc_calloc_d_defined 1
#define _malloc_d(n_bytes,...)                    __NAMESPACE_STD_SYM malloc(n_bytes)
#define _calloc_d(count,n_bytes,...)              __NAMESPACE_STD_SYM calloc(count,n_bytes)
#define _realloc_d(ptr,n_bytes,...)               __NAMESPACE_STD_SYM realloc(ptr,n_bytes)
#define _free_d(ptr,...)                          __NAMESPACE_STD_SYM free(ptr)
#endif /* !__malloc_calloc_d_defined */

#endif /* __USE_DEBUG == 0 */
#endif /* __USE_DEBUG */
#endif /* !__KERNEL__ || __KOS_VERSION__ < 300 */

__SYSDECL_END

#endif /* !_PARTS_MALLOC_H */
