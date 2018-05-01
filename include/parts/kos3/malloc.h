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
#ifndef _PARTS_KOS3_MALLOC_H
#define _PARTS_KOS3_MALLOC_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <parts/kos2/malldefs.h>

#if defined(__CC__) && !defined(__KERNEL__)
__SYSDECL_BEGIN

/* With KOS Mk3 now using my own user-space heap implementation (bye bye dlmalloc),
 * here are some useful functions that can only be implemented if you're controlling
 * the heap itself. (Most notably alignment' & alignment w/ offset-allocators)
 * NOTE: Pointers allocated here are still regular malloc-pointers,
 *       meaning that in order to free any of them, you only have to
 *       call `free()' */


/* Allocate specially aligned memory:
 * @assume(IS_ALIGNED((byte_t *)return + OFFSET,MIN_ALIGNMENT));
 * @assume(malloc_usable_size(return) >= NUM_BYTES); */
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC,void *,__LIBCCALL,
                  memalign_offset,(__size_t __min_alignment, __size_t __num_bytes, __ptrdiff_t __offset),(__min_alignment,__num_bytes,__offset))

/* Same as `memalign()' and `memalign_offset()', but ZERO-initialize new memory. */
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_ALLOC_ALIGN(1) __ATTR_MALLOC,void *,__LIBCCALL,
                  memcalign,(__size_t __min_alignment, __size_t __num_bytes),(__min_alignment,__num_bytes))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC,void *,__LIBCCALL,
                  memcalign_offset,(__size_t __min_alignment, __size_t __num_bytes, __ptrdiff_t __offset),(__min_alignment,__num_bytes,__offset))

/* Same as `realloc()', but ZERO-initialize new memory:
 * >> size_t old_size = malloc_usable_size(PRE(PTR));
 * >> if (old_size > NUM_BYTES)
 * >>     memset((byte_t *)POST(return) + old_size,0,NUM_BYTES - old_size) */
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)),void *,__LIBCCALL,
                  recalloc,(void *__ptr, __size_t __num_bytes),(__ptr,__num_bytes))

/* Same as `realloc()', but use `memalign()' for allocation when `PTR' is NULL,
 * and ensure that a new memory block follows `MIN_ALIGNMENT' when `PTR' must
 * be copied to a new memory block. */
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_ALLOC_ALIGN(1) __ATTR_ALLOC_SIZE((2)),void *,__LIBCCALL,
                  realign,(void *__ptr, __size_t __min_alignment, __size_t __num_bytes),(__ptr,__min_alignment,__num_bytes))

/* Same as `realign()', but new memory blocks follow
 * the restrictions described by `memalign_offset()':
 * @assume(IS_ALIGNED((byte_t *)return + OFFSET,MIN_ALIGNMENT)); */
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_ALLOC_SIZE((3)),void *,__LIBCCALL,
                  realign_offset,(void *__ptr, __size_t __min_alignment, __size_t __num_bytes, __ptrdiff_t __offset),(__ptr,__min_alignment,__num_bytes,__offset))

/* Same as `realign()', but new memory is ZERO-initialized. */
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_ALLOC_ALIGN(2) __ATTR_ALLOC_SIZE((3)),void *,__LIBCCALL,
                  recalign,(void *__ptr, __size_t __min_alignment, __size_t __num_bytes),(__ptr,__min_alignment,__num_bytes))

/* Same as `recalign()', but new memory blocks follow
 * the restrictions described by `memalign_offset()':
 * @assume(IS_ALIGNED((byte_t *)return + OFFSET,MIN_ALIGNMENT)); */
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_ALLOC_SIZE((3)),void *,__LIBCCALL,
                  recalign_offset,(void *__ptr, __size_t __min_alignment, __size_t __num_bytes, __ptrdiff_t __offset),(__ptr,__min_alignment,__num_bytes,__offset))

/* Same as `realloc_in_place()', but new memory is ZERO-initialized. */
__REDIRECT_EXCEPT(__LIBC,__MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)),void *,__LIBCCALL,
                  recalloc_in_place,(void *__ptr, __size_t __num_bytes),(__ptr,__num_bytes))



/* Called to redirect malloc() functions to their debug-counterparts.
 * If used, this function _MUST_ be called _BEFORE_ any call to any
 * malloc() function, and before more than one thread exists.
 * Basically, at the start of your `main()' function.
 * By default, malloc() functions link against fast versions, optimized
 * for speed, but not for safety.
 * However, libc implements all malloc() functions twice internally.
 * Once as a simple mall-header system, allocating a small overhead
 * for every pointer that is used to store contextual information,
 * while using a heap from `kos/heap.h' configured as `CONFIG_NO_DEBUG_HEAP'.
 * The second implementation however uses a node-based system that doesn't
 * store malloc context information anywhere near the actual user-data pointer,
 * instead opting to use a node-based approach that is unlikely to become
 * corrupt from invalid use such as array overruns.
 * Additionally, it makes use of a heap configured with `CONFIG_NO_DEBUG_HEAP'
 * enabled, while also allocating memory that is traced, which then allows for
 * that memory to be detectable as leaks (s.a. `heap_dump_leaks()')
 * When this function is called, malloc() functions are redirected to use those
 * debug-alternatives, essentially enabling the debug functionality of malloc(). */
__LIBC void (__LIBCCALL __set_debug_malloc)(void);


#ifndef __mallinfo_defined
#define __mallinfo_defined 1
#ifdef __USE_KOS_STDEXT
#define __MALLINFO_FIELD_TYPE unsigned int
#else
#define __MALLINFO_FIELD_TYPE int
#endif
struct mallinfo {
    __MALLINFO_FIELD_TYPE arena;     /* Non-mmapped space allocated (bytes) */
    __MALLINFO_FIELD_TYPE ordblks;   /* Number of free chunks */
    __MALLINFO_FIELD_TYPE smblks;    /* Number of free fastbin blocks */
    __MALLINFO_FIELD_TYPE hblks;     /* Number of mmapped regions */
    __MALLINFO_FIELD_TYPE hblkhd;    /* Space allocated in mmapped regions (bytes) */
    __MALLINFO_FIELD_TYPE usmblks;   /* Maximum total allocated space (bytes) */
    __MALLINFO_FIELD_TYPE fsmblks;   /* Space in freed fastbin blocks (bytes) */
    __MALLINFO_FIELD_TYPE uordblks;  /* Total allocated space (bytes) */
    __MALLINFO_FIELD_TYPE fordblks;  /* Total free space (bytes) */
    __MALLINFO_FIELD_TYPE keepcost;  /* Top-most, releasable space (bytes) */
};
#undef __MALLINFO_FIELD_TYPE
#endif /* !__mallinfo_defined */

__LIBC __WUNUSED struct mallinfo (__LIBCCALL mallinfo)(void);
__LIBC void (__LIBCCALL malloc_stats)(void);



__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_MALLOC_H */
