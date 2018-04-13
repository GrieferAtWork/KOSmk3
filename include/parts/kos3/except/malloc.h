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
#ifndef _PARTS_KOS3_EXCEPT_MALLOC_H
#define _PARTS_KOS3_EXCEPT_MALLOC_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <parts/kos2/malldefs.h>

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

/* @EXCEPT: `realloc_in_place()' will return `NULL' if the reallocation isn't
 *           possible due to the requested memory above `MALLPTR' already being
 *           in use. However, an `E_BADALLOC' exception is thrown if insufficient
 *           memory (for internal control structures) is available to complete
 *           the operation. */
__LIBC __PORT_KOSONLY __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL Xrealloc_in_place)(void *__restrict __mallptr, size_t __n_bytes);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __ATTR_ALLOC_ALIGN(1) __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC void *(__LIBCCALL Xmemalign)(size_t __alignment, size_t __n_bytes);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __MALL_ATTR_PAGEALIGNED __ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL Xpvalloc)(size_t __n_bytes);
#ifndef __Xvalloc_defined
#define __Xvalloc_defined 1
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __MALL_ATTR_PAGEALIGNED __ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL Xvalloc)(size_t __n_bytes);
#endif /* !__Xvalloc_defined */
__REDIRECT(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC,
           void *,__LIBCCALL,__Xmemdup,(void const *__restrict __ptr,size_t __n_bytes),Xmemdup,(__ptr,__n_bytes))
__REDIRECT(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC,
           void *,__LIBCCALL,__Xmemcdup,(void const *__restrict __ptr, int __needle, size_t __n_bytes),Xmemcdup,(__ptr,__needle,__n_bytes))

#ifdef __USE_KOS
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2))
__ATTR_MALLOC void *(__LIBCCALL Xmemdup)(void const *__restrict __ptr, size_t __n_bytes);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __MALL_DEFAULT_ALIGNED
__ATTR_MALLOC void *(__LIBCCALL Xmemcdup)(void const *__restrict __ptr, int __needle, size_t __n_bytes);
#endif /* __USE_KOS */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_MALLOC_H */
