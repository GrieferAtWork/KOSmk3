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
#ifndef GUARD_KERNEL_SRC_KERNEL_MALLOC_C
#define GUARD_KERNEL_SRC_KERNEL_MALLOC_C 1
#define _KOS_SOURCE 1
#define __OMIT_KMALLOC_CONSTANT_P_WRAPPERS 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/minmax.h>
#include <hybrid/align.h>
#include <kernel/malloc.h>
#include <kernel/heap.h>
#include <string.h>
#include <kernel/debug.h>

DECL_BEGIN
struct PACKED mptr {
    union PACKED {
        struct PACKED {
            size_t mp_size; /* Size of the block, including this header
                             * (in bytes; aligned by `HEAP_ALIGNMENT') */
            u8     mp_heap; /* [<= __GFP_HEAPCOUNT] The originating heap id */
        };
        byte_t     mp_align[HEAP_ALIGNMENT]; /* ... */
    };
};
STATIC_ASSERT(sizeof(struct mptr) == HEAP_ALIGNMENT);


#define mptr_get(x)  ((struct mptr *)(x)-1)
#define mptr_user(x) ((struct mptr *)(x)+1)

INTDEF ATTR_NORETURN void KCALL
heap_allocation_overflow(size_t n_bytes);

PUBLIC WUNUSED ATTR_MALLOC ATTR_RETNONNULL ATTR_WEAK
VIRT void *(KCALL kmalloc)(size_t n_bytes, gfp_t flags) {
 struct heapptr ptr;
 struct mptr *result;
 size_t alloc_size;
 if unlikely(__builtin_add_overflow(sizeof(struct mptr),n_bytes,&alloc_size))
    heap_allocation_overflow(n_bytes);
 ptr = heap_alloc_untraced(&kernel_heaps[flags & __GFP_HEAPMASK],
                            alloc_size,flags);
 assert(ptr.hp_siz >= alloc_size);
 result = (struct mptr *)ptr.hp_ptr;
 result->mp_size = ptr.hp_siz;
 result->mp_heap = flags & __GFP_HEAPMASK;
 return mptr_user(result);
}
PUBLIC WUNUSED ATTR_MALLOC ATTR_RETNONNULL ATTR_WEAK
VIRT void *(KCALL kmemalign)(size_t min_alignment,
                             size_t n_bytes, gfp_t flags) {
 struct heapptr hptr;
 struct mptr *result;
 size_t alloc_size;
 if unlikely(__builtin_add_overflow(sizeof(struct mptr),n_bytes,&alloc_size))
    heap_allocation_overflow(n_bytes);
 hptr = heap_align_untraced(&kernel_heaps[flags & __GFP_HEAPMASK],
                             min_alignment,sizeof(struct mptr),
                             alloc_size,flags);
 result = (struct mptr *)hptr.hp_ptr;
 result->mp_size = hptr.hp_siz;
 result->mp_heap = flags & __GFP_HEAPMASK;
 return mptr_user(result);
}
PUBLIC WUNUSED ATTR_MALLOC ATTR_RETNONNULL ATTR_WEAK
VIRT void *(KCALL kmemalign_offset)(size_t min_alignment, ptrdiff_t offset,
                                    size_t n_bytes, gfp_t flags) {
 struct heapptr hptr;
 struct mptr *result;
 size_t alloc_size;
 if unlikely(__builtin_add_overflow(sizeof(struct mptr),n_bytes,&alloc_size))
    heap_allocation_overflow(n_bytes);
 hptr = heap_align_untraced(&kernel_heaps[flags & __GFP_HEAPMASK],
                             min_alignment,sizeof(struct mptr)+offset,
                             alloc_size,flags);
 result = (struct mptr *)hptr.hp_ptr;
 result->mp_size = hptr.hp_siz;
 result->mp_heap = flags & __GFP_HEAPMASK;
 return mptr_user(result);
}

FORCELOCAL size_t KCALL get_realloc_size(size_t n_bytes) {
 size_t result;
 if unlikely(__builtin_add_overflow(n_bytes,HEAP_ALIGNMENT-1,&result))
    goto err_overflow;
 result &= ~(HEAP_ALIGNMENT-1);
 if unlikely(__builtin_add_overflow(result,sizeof(struct mptr),&result))
    goto err_overflow;
 return result;
err_overflow:
 heap_allocation_overflow(n_bytes);
}

PUBLIC ATTR_WEAK
VIRT void *(KCALL krealloc)(VIRT void *ptr,
                            size_t n_bytes, gfp_t flags) {
 struct heapptr hptr; struct mptr *result;
 struct heap *heap; size_t more_size;
 if (!ptr) return kmalloc(n_bytes,flags);
 assert(IS_ALIGNED((uintptr_t)ptr,HEAP_ALIGNMENT));
 /* Align the given n_bytes and add the overhead caused by the mptr. */
 n_bytes = get_realloc_size(n_bytes);
 result = mptr_get(ptr);
 assert(result->mp_heap < __GFP_HEAPCOUNT);
 if (n_bytes <= result->mp_size) {
  if (n_bytes == result->mp_size)
      return ptr; /* Unchanged. */

  /* Truncate the pointer. */
  hptr.hp_siz = result->mp_size-n_bytes;
  if (hptr.hp_siz >= HEAP_MINSIZE) {
   /* Only do the truncation if the memory
    * that gets freed by this is large enough. */
   hptr.hp_ptr = (VIRT void *)((uintptr_t)result+n_bytes);
   heap_free_untraced(&kernel_heaps[result->mp_heap],hptr.hp_ptr,n_bytes,GFP_NORMAL);
   result->mp_size = n_bytes;
  }
  return ptr;
 }
 /* Increase the pointer (try to do so in-place at first) */
 flags &= ~__GFP_HEAPMASK;
 flags |= result->mp_heap;
 heap      = &kernel_heaps[result->mp_heap];
 more_size = heap_allat_untraced(heap,
                                (void *)((uintptr_t)result + result->mp_size),
                                (size_t)(n_bytes - result->mp_size),flags);
 if (more_size != 0) {
  /* Allocation was OK. Update the mptr. */
  result->mp_size += more_size;
  assert(result->mp_size >= n_bytes);
  return ptr;
 }
 /* Return NULL if we're not allowed to move the pointer. */
 if (flags&GFP_NOMOVE)
     return NULL;
 /* Overlap with another pointer. - Allocate a new block. */
 hptr = heap_alloc_untraced(heap,sizeof(struct mptr)+n_bytes,flags);
 memcpy(hptr.hp_ptr,result,result->mp_size);
 /* Free the old pointer. */
 heap_free_untraced(heap,result,result->mp_size,GFP_NORMAL);
 /* Initialize the new pointer. */
 result          = (struct mptr *)hptr.hp_ptr;
 result->mp_size = hptr.hp_siz;
 result->mp_heap = flags & __GFP_HEAPMASK;
 return mptr_user(result);
}
PUBLIC ATTR_WEAK
VIRT void *(KCALL krealign)(VIRT void *ptr, size_t min_alignment,
                            size_t n_bytes, gfp_t flags) {
 struct heapptr hptr; struct mptr *result;
 struct heap *heap; size_t more_size;
 if (!ptr) return kmemalign(min_alignment,n_bytes,flags);
 assert(IS_ALIGNED((uintptr_t)ptr,HEAP_ALIGNMENT));
 /* Align the given n_bytes and add the overhead caused by the mptr. */
 n_bytes = get_realloc_size(n_bytes);
 result = mptr_get(ptr);
 assert(result->mp_heap < __GFP_HEAPCOUNT);
 if (n_bytes <= result->mp_size) {
  if (n_bytes == result->mp_size)
      return ptr; /* Unchanged. */

  /* Truncate the pointer. */
  hptr.hp_siz = result->mp_size-n_bytes;
  if (hptr.hp_siz >= HEAP_MINSIZE) {
   /* Only do the truncation if the memory
    * that gets freed by this is large enough. */
   hptr.hp_ptr = (VIRT void *)((uintptr_t)result+n_bytes);
   heap_free_untraced(&kernel_heaps[result->mp_heap],hptr.hp_ptr,n_bytes,GFP_NORMAL);
   result->mp_size = n_bytes;
  }
  return ptr;
 }
 /* Increase the pointer (try to do so in-place at first) */
 flags &= ~__GFP_HEAPMASK;
 flags |= result->mp_heap;
 heap      = &kernel_heaps[result->mp_heap];
 more_size = heap_allat_untraced(heap,
                                (void *)((uintptr_t)result + result->mp_size),
                                (size_t)(n_bytes - result->mp_size),flags);
 if (more_size != 0) {
  /* Allocation was OK. Update the mptr. */
  result->mp_size += more_size;
  assert(result->mp_size >= n_bytes);
  return ptr;
 }
 /* Return NULL if we're not allowed to move the pointer. */
 if (flags&GFP_NOMOVE)
     return NULL;
 /* Overlap with another pointer. - Allocate a new block. */
 hptr = heap_align_untraced(heap,min_alignment,
                            sizeof(struct mptr),
                            sizeof(struct mptr)+n_bytes,flags);
 memcpy(hptr.hp_ptr,result,result->mp_size);
 /* Free the old pointer. */
 heap_free_untraced(heap,result,result->mp_size,GFP_NORMAL);
 /* Initialize the new pointer. */
 result          = (struct mptr *)hptr.hp_ptr;
 result->mp_size = hptr.hp_siz;
 result->mp_heap = flags & __GFP_HEAPMASK;
 return mptr_user(result);
}
PUBLIC ATTR_WEAK
VIRT void *(KCALL krealign_offset)(VIRT void *ptr, size_t min_alignment,
                                   ptrdiff_t offset, size_t n_bytes, gfp_t flags) {
 struct heapptr hptr; struct mptr *result;
 struct heap *heap; size_t more_size;
 if (!ptr) return kmemalign_offset(min_alignment,offset,n_bytes,flags);
 assert(IS_ALIGNED((uintptr_t)ptr,HEAP_ALIGNMENT));
 /* Align the given n_bytes and add the overhead caused by the mptr. */
 n_bytes = get_realloc_size(n_bytes);
 result = mptr_get(ptr);
 assert(result->mp_heap < __GFP_HEAPCOUNT);
 if (n_bytes <= result->mp_size) {
  if (n_bytes == result->mp_size)
      return ptr; /* Unchanged. */

  /* Truncate the pointer. */
  hptr.hp_siz = result->mp_size-n_bytes;
  if (hptr.hp_siz >= HEAP_MINSIZE) {
   /* Only do the truncation if the memory
    * that gets freed by this is large enough. */
   hptr.hp_ptr = (VIRT void *)((uintptr_t)result+n_bytes);
   heap_free_untraced(&kernel_heaps[result->mp_heap],hptr.hp_ptr,n_bytes,GFP_NORMAL);
   result->mp_size = n_bytes;
  }
  return ptr;
 }
 /* Increase the pointer (try to do so in-place at first) */
 flags &= ~__GFP_HEAPMASK;
 flags |= result->mp_heap;
 heap      = &kernel_heaps[result->mp_heap];
 more_size = heap_allat_untraced(heap,
                                (void *)((uintptr_t)result + result->mp_size),
                                (size_t)(n_bytes - result->mp_size),flags);
 if (more_size != 0) {
  /* Allocation was OK. Update the mptr. */
  result->mp_size += more_size;
  assert(result->mp_size >= n_bytes);
  return ptr;
 }
 /* Return NULL if we're not allowed to move the pointer. */
 if (flags&GFP_NOMOVE)
     return NULL;
 /* Overlap with another pointer. - Allocate a new block. */
 hptr = heap_align_untraced(heap,min_alignment,
                            sizeof(struct mptr)+offset,
                            sizeof(struct mptr)+n_bytes,flags);
 memcpy(hptr.hp_ptr,result,result->mp_size);
 /* Free the old pointer. */
 heap_free_untraced(heap,result,result->mp_size,GFP_NORMAL);
 /* Initialize the new pointer. */
 result          = (struct mptr *)hptr.hp_ptr;
 result->mp_size = hptr.hp_siz;
 result->mp_heap = flags & __GFP_HEAPMASK;
 return mptr_user(result);
}

PUBLIC ATTR_WEAK ATTR_NOTHROW WUNUSED size_t (KCALL kmalloc_usable_size)(VIRT void *ptr) {
 return ptr ? mptr_get(ptr)->mp_size-sizeof(struct mptr) : 0;
}
PUBLIC ATTR_WEAK ATTR_NOTHROW void (KCALL kfree)(VIRT void *ptr) {
 struct mptr *result;
 if (!ptr) return; /* Ignore NULL-pointers. */
 assert(IS_ALIGNED((uintptr_t)ptr,HEAP_ALIGNMENT));
 result = mptr_get(ptr);
 assert(result->mp_heap < __GFP_HEAPCOUNT);
 assert(IS_ALIGNED((uintptr_t)result->mp_size,HEAP_ALIGNMENT));
 assert(result->mp_size >= HEAP_MINSIZE);
 heap_free_untraced(&kernel_heaps[result->mp_heap],
                     result,result->mp_size,
                     result->mp_heap);
}
PUBLIC ATTR_WEAK void (KCALL kffree)(VIRT void *ptr, gfp_t flags) {
 struct mptr *result;
 if (!ptr) return; /* Ignore NULL-pointers. */
 assert(IS_ALIGNED((uintptr_t)ptr,HEAP_ALIGNMENT));
 result = mptr_get(ptr);
 assert(result->mp_heap < __GFP_HEAPCOUNT);
 assert(IS_ALIGNED((uintptr_t)result->mp_size,HEAP_ALIGNMENT));
 assert(result->mp_size >= HEAP_MINSIZE);
 heap_free_untraced(&kernel_heaps[result->mp_heap],
                     result,result->mp_size,
                     result->mp_heap | (flags & ~__GFP_HEAPMASK));
}



/* Weakly define mall's debug functions are no-ops. */
PUBLIC ATTR_WEAK size_t KCALL mall_dump_leaks(gfp_t UNUSED(heap_max)) { return 0; }
PUBLIC ATTR_WEAK void KCALL mall_validate_padding(gfp_t UNUSED(heap_max)) { }
PUBLIC ATTR_WEAK void KCALL mall_untrace(void *UNUSED(ptr)) { }
PUBLIC ATTR_WEAK void KCALL mall_trace(void *UNUSED(base), size_t UNUSED(num_bytes)) { }
DEFINE_PUBLIC_WEAK_ALIAS(mall_trace_leakless,mall_trace);



DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_MALLOC_C */
