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
#ifndef GUARD_LIBS_LIBC_HEAP_H
#define GUARD_LIBS_LIBC_HEAP_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     HEAP                                                                              */
/* ===================================================================================== */
#ifndef __gfp_t_defined
#define __gfp_t_defined 1
typedef unsigned int gfp_t; /* Set of `GFP_*' */
#endif /* !__gfp_t_defined */
#ifndef __heapptr_defined
#define __heapptr_defined 1
struct heapptr {
#ifdef __INTELLISENSE__
           void *hp_ptr; /* [1..hp_siz] Pointer base address. */
#else
    __VIRT void *hp_ptr; /* [1..hp_siz] Pointer base address. */
#endif
    __size_t     hp_siz; /* [!0] Size of the pointer. */
};
#endif /* !__heapptr_defined */
#ifndef __pfindleakscallback_defined
#define __pfindleakscallback_defined 1
typedef ssize_t (LIBCCALL *pfindleakscallback)(void *base, size_t num_bytes,
                                               pid_t leaker_tid, size_t frame_cnt,
                                               void *const frame_ip[], void *closure);
#endif /* !__pfindleakscallback_defined */
struct mfree;
struct heap;
struct local_heap;
struct local_heap_d;
/* Basic HEAP API (debug-mode + regular-mode) */
INTDEF struct heapptr LIBCCALL libc_heap_alloc_untraced(struct local_heap *__restrict self, size_t num_bytes, gfp_t flags);
INTDEF struct heapptr LIBCCALL libc_heap_alloc_untraced_d(struct local_heap_d *__restrict self, size_t num_bytes, gfp_t flags);
INTDEF size_t LIBCCALL libc_heap_allat_untraced(struct local_heap *__restrict self, void *__restrict ptr, size_t num_bytes, gfp_t flags);
INTDEF size_t LIBCCALL libc_heap_allat_untraced_d(struct local_heap_d *__restrict self, void *__restrict ptr, size_t num_bytes, gfp_t flags);
INTDEF struct heapptr LIBCCALL libc_heap_align_untraced(struct local_heap *__restrict self, size_t min_alignment, ptrdiff_t offset, size_t num_bytes, gfp_t flags);
INTDEF struct heapptr LIBCCALL libc_heap_align_untraced_d(struct local_heap_d *__restrict self, size_t min_alignment, ptrdiff_t offset, size_t num_bytes, gfp_t flags);
INTDEF struct heapptr LIBCCALL libc_heap_realloc_untraced(struct local_heap *__restrict self, void *old_ptr, size_t old_bytes, size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags);
INTDEF struct heapptr LIBCCALL libc_heap_realloc_untraced_d(struct local_heap_d *__restrict self, void *old_ptr, size_t old_bytes, size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags);
INTDEF struct heapptr LIBCCALL libc_heap_realign_untraced(struct local_heap *__restrict self, void *old_ptr, size_t old_bytes, size_t min_alignment, ptrdiff_t offset, size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags);
INTDEF struct heapptr LIBCCALL libc_heap_realign_untraced_d(struct local_heap_d *__restrict self, void *old_ptr, size_t old_bytes, size_t min_alignment, ptrdiff_t offset, size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags);
INTDEF void LIBCCALL libc_heap_free_untraced(struct local_heap *__restrict self, void *ptr, size_t num_bytes, gfp_t flags);
INTDEF void LIBCCALL libc_heap_free_untraced_d(struct local_heap_d *__restrict self, void *ptr, size_t num_bytes, gfp_t flags);
INTDEF struct heapptr LIBCCALL libc_heap_alloc_d(struct local_heap_d *__restrict self, size_t num_bytes, gfp_t flags);
INTDEF struct heapptr LIBCCALL libc_heap_align_d(struct local_heap_d *__restrict self, size_t min_alignment, ptrdiff_t offset, size_t num_bytes, gfp_t flags);
INTDEF size_t LIBCCALL libc_heap_allat_d(struct local_heap_d *__restrict self, void *__restrict ptr, size_t num_bytes, gfp_t flags);
INTDEF struct heapptr LIBCCALL libc_heap_realloc_d(struct local_heap_d *__restrict self, void *old_ptr, size_t old_bytes, size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags);
INTDEF struct heapptr LIBCCALL libc_heap_realign_d(struct local_heap_d *__restrict self, void *old_ptr, size_t old_bytes, size_t min_alignment, ptrdiff_t offset, size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags);
INTDEF void LIBCCALL libc_heap_free_d(struct local_heap_d *__restrict self, void *ptr, size_t num_bytes, gfp_t flags);
INTDEF size_t LIBCCALL libc_heap_truncate(struct local_heap *__restrict self, size_t threshold);
INTDEF size_t LIBCCALL libc_heap_truncate_d(struct local_heap_d *__restrict self, size_t threshold);

/* Default heaps */
/* Heap used for allocating tracing points for `libc_heap_trace*'
 * This heap is usually placed far away from regular heap locations
 * in order to reduce the chances of it accidentally becoming corrupt
 * from dangling pointers. */
INTDEF struct local_heap libc_heap_trace_allocator;
/* Generic HEAP Helper functions API */
INTDEF ATTR_NORETURN void KCALL libc_heap_allocation_failed(size_t n_bytes);
INTDEF u8 KCALL libc_mfree_get_checksum(struct mfree *__restrict self);
INTDEF void LIBCCALL libc_heap_register_d(struct heap *__restrict self);
INTDEF void LIBCCALL libc_heap_unregister_d(struct heap *__restrict self);
INTDEF void LIBCCALL libc_heap_validate(struct heap *__restrict self);
INTDEF void LIBCCALL libc_heap_validate_all(void);
INTDEF void LIBCCALL libc_heap_trace(void *base, size_t num_bytes, unsigned int num_skipframes);
INTDEF void LIBCCALL libc_heap_trace_root(void *base, size_t num_bytes);
INTDEF void LIBCCALL libc_heap_trace_leakless(void *base, size_t num_bytes);
INTDEF ATTR_NOTHROW void LIBCCALL libc_heap_untrace(void *ptr);
INTDEF size_t LIBCCALL libc_heap_dump_leaks(void);
INTDEF ssize_t LIBCCALL libc_heap_find_leaks(pfindleakscallback func, void *closure);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_HEAP_H */
