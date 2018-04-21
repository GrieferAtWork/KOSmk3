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
#ifndef _MALLOC_H
#define _MALLOC_H 1

#include "__stdinc.h"
#include "parts/kos2/malldefs.h"
#include <hybrid/typecore.h>
#include <features.h>

__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#ifdef __CRT_KOS
/* malloc behavior attributes. */
#define __MALLOC_ZERO_IS_NONNULL  1
#define __REALLOC_ZERO_IS_NONNULL 1
#endif

#if !defined(__KERNEL__) || __KOS_VERSION__ < 300

#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)),void *,__LIBCCALL,realloc_in_place,(void *__restrict __mallptr, size_t __n_bytes),_expand,(__mallptr,__n_bytes))
#else
/* @EXCEPT: `realloc_in_place()' will return `NULL' if the reallocation isn't
 *           possible due to the requested memory above `MALLPTR' already being
 *           in use. However, an `E_BADALLOC' exception is thrown if insufficient
 *           memory (for internal control structures) is available to complete
 *           the operation. */
__REDIRECT_EXCEPT(__LIBC,__MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)),void *,__LIBCCALL,realloc_in_place,(void *__restrict __mallptr, size_t __n_bytes),(__mallptr,__n_bytes))
#endif
#ifdef __CRT_GLC
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_NODOS __WUNUSED __ATTR_ALLOC_ALIGN(1) __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC,void *,__LIBCCALL,memalign,(size_t __alignment, size_t __n_bytes),(__alignment,__n_bytes))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_NODOS __WUNUSED __MALL_ATTR_PAGEALIGNED __ATTR_ALLOC_SIZE((1)),void *,__LIBCCALL,pvalloc,(size_t __n_bytes),(__n_bytes))
#ifndef __valloc_defined
#define __valloc_defined 1
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_NODOS __WUNUSED __MALL_ATTR_PAGEALIGNED __ATTR_ALLOC_SIZE((1)),void *,__LIBCCALL,valloc,(size_t __n_bytes),(__n_bytes))
#endif /* !__valloc_defined */
#ifndef __posix_memalign_defined
#define __posix_memalign_defined 1
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL posix_memalign)(void **__restrict __pp, size_t __alignment, size_t __n_bytes);
#endif /* !__posix_memalign_defined */
#endif /* __CRT_GLC */

#define M_TRIM_THRESHOLD     (-1)
#define M_GRANULARITY        (-2)

#ifdef __KERNEL__
#ifndef __cfree_defined
#define __cfree_defined 1
__REDIRECT_VOID(__LIBC,,__LIBCCALL,cfree,(void *__restrict __mallptr),kfree,(__mallptr))
#endif /* !__cfree_defined */
__REDIRECT(__LIBC,__WUNUSED,size_t,__LIBCCALL,malloc_usable_size,(void *__restrict __mallptr),kmalloc_usable_size,(__mallptr))
#else /* __KERNEL__ */
#ifndef __cfree_defined
#define __cfree_defined 1
__REDIRECT_VOID(__LIBC,,__LIBCCALL,cfree,(void *__restrict __mallptr),free,(__mallptr))
#endif /* !__cfree_defined */
#if defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__LIBC __WUNUSED int (__LIBCCALL malloc_trim)(size_t __pad);
#elif defined(__CRT_DOS) && !defined(__GLC_COMPAT__)
__REDIRECT(__LIBC,,int,__LIBCCALL,__libc_heapmin,(void),_heapmin,())
__LOCAL __WUNUSED int (__LIBCCALL malloc_trim)(size_t __UNUSED(__pad)) { return __libc_heapmin() ? 1 : 0; }
#else /* __CRT_GLC */
__LOCAL __WUNUSED int (__LIBCCALL malloc_trim)(size_t __UNUSED(__pad)) { return 0; }
#endif /* !__CRT_GLC */
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED,size_t,__LIBCCALL,malloc_usable_size,(void *__restrict __mallptr),_msize,(__mallptr))
#else
__LIBC __WUNUSED size_t (__LIBCCALL malloc_usable_size)(void *__restrict __mallptr);
#endif
#define M_MMAP_THRESHOLD     (-3)
#endif /* !__KERNEL__ */


#ifdef __CRT_GLC
__LIBC int (__LIBCCALL mallopt)(int __parameter_number, int __parameter_value);
#else /* __CRT_GLC */
__LOCAL int (__LIBCCALL mallopt)(int __UNUSED(__parameter_number), int __UNUSED(__parameter_value)) { return 0; }
#endif /* !__CRT_GLC */

#ifdef __CRT_KOS
__REDIRECT_EXCEPT_(__LIBC,__XATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC,
                   void *,__LIBCCALL,__memdup,(void const *__restrict __ptr,size_t __n_bytes),memdup,(__ptr,__n_bytes))
#ifndef __KERNEL__
__REDIRECT_EXCEPT_(__LIBC,__XATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC,
                   void *,__LIBCCALL,__memcdup,(void const *__restrict __ptr, int __needle, size_t __n_bytes),memcdup,(__ptr,__needle,__n_bytes))
#endif /* !__KERNEL__ */
#ifdef __USE_KOS
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC,void *,__LIBCCALL,
                  memdup,(void const *__restrict __ptr, size_t __n_bytes),(__ptr,__n_bytes))
#ifndef __KERNEL__
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC,void *,__LIBCCALL,
                  memcdup,(void const *__restrict __ptr, int __needle, size_t __n_bytes),(__ptr,__needle,__n_bytes))
#endif /* !__KERNEL__ */
#endif
#else /* __CRT_KOS */
__SYSDECL_END
#include <hybrid/string.h>
__SYSDECL_BEGIN
__LOCAL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC void *(__LIBCCALL __memdup)(void const *__restrict __ptr, size_t __n_bytes) { void *__result = malloc(__n_bytes); if (__result) __hybrid_memcpy(__result,__ptr,__n_bytes); return __result; }
__LOCAL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC void *(__LIBCCALL __memcdup)(void const *__restrict __ptr, int __needle, size_t __n_bytes) { if (__n_bytes) { void const *__endaddr = __hybrid_memchr(__ptr,__needle,__n_bytes-1); if (__endaddr) __n_bytes = ((__UINTPTR_TYPE__)__endaddr-(__UINTPTR_TYPE__)__ptr)+1; } return __memdup(__ptr,__n_bytes); }
#ifdef __USE_KOS
__LOCAL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC void *(__LIBCCALL memdup)(void const *__restrict __ptr, size_t __n_bytes) { return (__memdup)(__ptr,__n_bytes); }
__LOCAL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC void *(__LIBCCALL memcdup)(void const *__restrict __ptr, int __needle, size_t __n_bytes) { return (__memcdup)(__ptr,__needle,__n_bytes); }
#endif /* __USE_KOS */
#endif /* !__CRT_KOS */

#ifdef __USE_DOS
#ifdef __KERNEL__
__REDIRECT(__LIBC,__WUNUSED,size_t,__LIBCCALL,_msize,(void *__restrict __mallptr),kmalloc_usable_size,(__mallptr))
#elif !defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__WUNUSED,size_t,__LIBCCALL,_msize,(void *__restrict __mallptr),malloc_usable_size,(__mallptr))
#else
__LIBC __WUNUSED size_t (__LIBCCALL _msize)(void *__restrict __mallptr);
#endif /* !__KERNEL__ */
#endif /* __USE_DOS */
#endif /* !__KERNEL__ || __KOS_VERSION__ < 300 */

__SYSDECL_END

#include "parts/malloc.h"

#ifdef __USE_KOS
#ifdef _ALLOCA_H
#include "parts/kos2/malloca.h"
#define malloca(s) __malloca(s)
#define calloca(s) __calloca(s)
#define freea(p)   __freea(p)
#ifndef __USE_KOS_DEPRECATED
#define amalloc(s) __malloca(s)
#define acalloc(s) __calloca(s)
#define afree(p)   __freea(p)
#endif /* !__USE_KOS_DEPRECATED */
#endif
#ifndef _PARTS_KOS2_MALLOC_H
#include "parts/kos2/malloc.h"
#endif
#endif /* __USE_KOS */

#ifdef __USE_KOS3
#ifndef _PARTS_KOS3_MALLOC_H
#include "parts/kos3/malloc.h"
#endif
#endif

#ifdef __USE_EXCEPT
#include "parts/kos3/except/malloc.h"
#endif

#endif /* !_MALLOC_H */
