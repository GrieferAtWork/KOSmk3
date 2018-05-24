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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_HEAP_H
#define GUARD_KERNEL_INCLUDE_KERNEL_HEAP_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/__bit.h>
#include <hybrid/align.h>
#include <hybrid/list/list.h>
#include <hybrid/list/atree.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/malloc.h>
#include <kernel/vm.h>
#include <stdbool.h>

DECL_BEGIN

/* Heap debug initialization DWORDs */
#ifdef CONFIG_DEBUG_HEAP
#define DEBUGHEAP_NO_MANS_LAND  0xdeadbeef /* Debug initialization of unallocated memory. */
#define DEBUGHEAP_FRESH_MEMORY  0xaaaaaaaa /* Debug initialization of freshly allocated memory. */
#endif /* CONFIG_DEBUG_HEAP */

#define SIZEOF_MFREE    offsetof(struct mfree,mf_data)
#define HEAP_MINSIZE    CEIL_ALIGN(SIZEOF_MFREE,HEAP_ALIGNMENT)

struct mfree {
    LIST_NODE(struct mfree)   mf_lsize;   /* [lock(:h_lock)][sort(ASCENDING(mf_size))] List of free entries ordered by size. */
    ATREE_XNODE(struct mfree) mf_laddr;   /* [lock(:h_lock)][sort(ASCENDING(self))] List of free entries ordered by address. */
    size_t                    mf_size;    /* Size of this block (in bytes; aligned by `HEAP_ALIGNMENT'; including this header) */
#define MFREE_FUNDEFINED      0x00        /* Memory initialization is undefined.
                                           * In debug mode, this means that memory is
                                           * initialized using `DEBUGHEAP_NO_MANS_LAND',
                                           * with portions that haven't been allocated yet
                                           * pending initialization for either `DEBUGHEAP_FRESH_MEMORY'
                                           * or ZERO(0), depending on how they were originally allocated. */
#define MFREE_FZERO           GFP_CALLOC  /* Memory is ZERO-initialized. */
#define MFREE_FMASK           MFREE_FZERO /* Mask of known flags. */
    u8                        mf_flags;   /* Set of `MFREE_F*' */
#ifdef CONFIG_DEBUG_HEAP
    u8                        mf_szchk;   /* Checksum for `mf_size' */
#endif
    byte_t                    mf_data[1]; /* Block data. */
};
#define MFREE_MIN(self)   ((uintptr_t)(self))
#define MFREE_MAX(self)   ((uintptr_t)(self)+(self)->mf_size-1)
#define MFREE_BEGIN(self) ((uintptr_t)(self))
#define MFREE_END(self)   ((uintptr_t)(self)+(self)->mf_size)
#define MFREE_SIZE(self)              (self)->mf_size


/* Heap configuration:
 * Index offset for the first bucket that should be search for a given size. */
#if HEAP_ALIGNMENT == 1
#   define HEAP_BUCKET_OFFSET     0 /* FFS(HEAP_ALIGNMENT) */
#elif HEAP_ALIGNMENT == 2
#   define HEAP_BUCKET_OFFSET     1 /* FFS(HEAP_ALIGNMENT) */
#elif HEAP_ALIGNMENT == 4
#   define HEAP_BUCKET_OFFSET     2 /* FFS(HEAP_ALIGNMENT) */
#elif HEAP_ALIGNMENT == 8
#   define HEAP_BUCKET_OFFSET     4 /* FFS(HEAP_ALIGNMENT) */
#elif HEAP_ALIGNMENT == 16
#   define HEAP_BUCKET_OFFSET     5 /* FFS(HEAP_ALIGNMENT) */
#elif HEAP_ALIGNMENT == 32
#   define HEAP_BUCKET_OFFSET     6 /* FFS(HEAP_ALIGNMENT) */
#elif HEAP_ALIGNMENT == 64
#   define HEAP_BUCKET_OFFSET     7 /* FFS(HEAP_ALIGNMENT) */
#elif HEAP_ALIGNMENT == 128
#   define HEAP_BUCKET_OFFSET     8 /* FFS(HEAP_ALIGNMENT) */
#elif HEAP_ALIGNMENT == 256
#   define HEAP_BUCKET_OFFSET     9 /* FFS(HEAP_ALIGNMENT) */
#else
#   define HEAP_BUCKET_OFFSET     __hybrid_ffs(HEAP_ALIGNMENT)
#endif

#define HEAP_BUCKET_OF(size)   (((__SIZEOF_SIZE_T__*8)-__hybrid_clz(size))-HEAP_BUCKET_OFFSET)
#define HEAP_BUCKET_MINSIZE(i)   (1 << ((i)+HEAP_BUCKET_OFFSET-1))
#define HEAP_BUCKET_COUNT       ((__SIZEOF_SIZE_T__*8)-(HEAP_BUCKET_OFFSET-1))

struct heap {
    atomic_rwlock_t           h_lock;       /* Lock for this heap. */
    ATREE_HEAD(struct mfree)  h_addr;       /* [lock(h_lock)][0..1] Heap sorted by address. */
    LIST_HEAD(struct mfree)   h_size[HEAP_BUCKET_COUNT];
                                            /* [lock(h_lock)][0..1][*] Heap sorted by free range size. */
    WEAK size_t               h_overalloc;  /* Amount (in bytes) by which to over-allocate memory in heaps.
                                             * NOTE: Set to ZERO(0) to disable overallocation. */
    WEAK size_t               h_freethresh; /* Threshold that must be reached before any continuous block
                                             * of free data is unmapped from the kernel VM. (Should always be `>= PAGESIZE') */
    WEAK vm_vpage_t           h_hintpage;   /* Page number to use as next allocation hint. */
    WEAK unsigned int         h_hintmode;   /* Mode used when searching for new pages. */
#ifdef CONFIG_HEAP_TRACE_DANGLE
    ATOMIC_DATA size_t        h_dangle;     /* [lock(INCREMENT(h_lock),DECREMENT(atomic),READ(atomic))]
                                             * Amount of dangling bytes of memory (memory that was allocated, but may be
                                             * release again shortly) When new memory would have to be requested from the
                                             * core, this field is checked to see if it is likely that some large block
                                             * of memory will be released soon, preventing a race condition that would
                                             * unnecessarily allocate more memory when `heap_free_untraced()' is merging a data
                                             * block with another, larger data block, for which it must temporarily
                                             * allocate that larger data block. Another thread allocating memory at the
                                             * same time may then think that the cache has grown too small for the allocation
                                             * and unnecessarily request more memory from the core. */
#endif /* CONFIG_HEAP_TRACE_DANGLE */
};                            


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


/* The standard heaps with which kernel data is allocated.
 * When allocating from any of these heaps, you must pass the
 * heap's index or'd to the `flags' you're using for allocation:
 * >> unsigned int heap_index = 2;
 * >> heap_alloc(&kernel_heaps[heap_index],42,heap_index|GFP_CALLOC); */
DATDEF struct heap kernel_heaps[__GFP_HEAPCOUNT];

FUNDEF struct heapptr KCALL __os_heap_alloc_untraced(struct heap *__restrict self, size_t num_bytes, gfp_t flags) ASMNAME("heap_alloc_untraced");
FUNDEF struct heapptr KCALL __os_heap_align_untraced(struct heap *__restrict self, size_t min_alignment, ptrdiff_t offset, size_t num_bytes, gfp_t flags) ASMNAME("heap_align_untraced");
FUNDEF size_t KCALL __os_heap_allat_untraced(struct heap *__restrict self, VIRT void *__restrict ptr, size_t num_bytes, gfp_t flags) ASMNAME("heap_allat_untraced");
FUNDEF struct heapptr KCALL __os_heap_realloc_untraced(struct heap *__restrict self, VIRT void *old_ptr, size_t old_bytes, size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags) ASMNAME("heap_realloc_untraced");
FUNDEF struct heapptr KCALL __os_heap_realign_untraced(struct heap *__restrict self, VIRT void *old_ptr, size_t old_bytes, size_t min_alignment, ptrdiff_t offset, size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags) ASMNAME("heap_realign_untraced");
FUNDEF struct heapptr KCALL __os_heap_alloc(struct heap *__restrict self, size_t num_bytes, gfp_t flags) ASMNAME("heap_alloc");
FUNDEF struct heapptr KCALL __os_heap_align(struct heap *__restrict self, size_t min_alignment, ptrdiff_t offset, size_t num_bytes, gfp_t flags) ASMNAME("heap_align");
FUNDEF size_t KCALL __os_heap_allat(struct heap *__restrict self, VIRT void *__restrict ptr, size_t num_bytes, gfp_t flags) ASMNAME("heap_allat");
FUNDEF struct heapptr KCALL __os_heap_realloc(struct heap *__restrict self, VIRT void *old_ptr, size_t old_bytes, size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags) ASMNAME("heap_realloc");
FUNDEF struct heapptr KCALL __os_heap_realign(struct heap *__restrict self, VIRT void *old_ptr, size_t old_bytes, size_t min_alignment, ptrdiff_t offset, size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags) ASMNAME("heap_realign");

#ifndef __OMIT_HEAP_CONSTANT_P_WRAPPERS
/* Allocate at least `num_bytes' of dynamic memory,
 * aligned by at least `min_alignment' or `HEAP_ALIGNMENT'.
 * NOTES:
 *   - Passing `0', or anything less than `HEAP_MINSIZE'
 *     for `num_bytes' is the same as passing `HEAP_MINSIZE'.
 *   - Passing anything less than `HEAP_ALIGNMENT' for `min_alignment'
 *     is the same as passing `HEAP_ALIGNMENT' for `min_alignment'
 *   - The `offset' parameter is a delta added to the returned pointer
 *     where the given `min_alignment' applies:
 *     >> assert(IS_ALIGNED((uintptr_t)return.hp_ptr + offset,min_alignment));
 *     However, it doesn't guaranty that memory at `(uintptr_t)return.hp_ptr + offset'
 *     is actually part of the allocation.
 *     HINT: `offset' is internally truncated by `min_alignment':
 *           `offset = offset & (min_alignment-1)'
 *   - These functions are written to be able to deal with _ANY_ `num_bytes'
 *     value, including unaligned value, or absurdly large values that were
 *     designed only to cause internal overflow errors.
 * The following flags affect behavior:
 *    heap_alloc_untraced / heap_align_untraced:
 *       - GFP_CALLOC          -- Allocate ZERO-initialized memory (in `return.hp_ptr ...+= return.hp_siz')
 *       - GFP_NOOVER          -- Do not overallocate (by `self->h_overalloc')
 *       - GFP_NOMAP           -- Don't map new memory (throw `E_WOULDBLOCK' instead)
 *       - GFP_ATOMIC          -- Do not block when mapping new memory (throw `E_WOULDBLOCK' instead)
 *       - GFP_NOFS|GFP_NOSWAP -- Behavioral modifiers for swapping memory
 *    heap_free_untraced:
 *       - GFP_CALLOC          -- The given memory block is ZERO-initialized (allows for some internal optimizations)
 *       - GFP_NOTRIM          -- Do not `vm_unmap()' free memory blocks larger than
 *                               `h_freethresh', but keep them in cache instead.
 * NOTE: `heap_free_untraced()' always completes without blocking.
 *        If `vm_unmap()' needs to be called, the free() operation
 *        is either postponed until the next call to a heap function
 *        that is allowed to block, or is simply kept in cache, as
 *        though `GFP_NOMAP'  has been passed.
 * NOTE:  The `*_traced' family of functions will automatically call `mall_trace' / `mall_untrace'
 *        in order to register / unregister the data blocks as a MALL GC search data block.
 *        When building without `CONFIG_DEBUG_MALLOC', they are aliasing the regular versions.
 * @param: flags: Set of `GFP_*' flags used for allocation.
 * @throw: E_BADALLOC:    Failed to allocate memory.
 * @throw: E_WOULDBLOCK: `GFP_NOMAP' was specified and new memory would have had to be mapped.
 * @throw: E_WOULDBLOCK: `GFP_ATOMIC' was specified and a lock could not be acquired immediately. */
#ifndef __OMIT_HEAP_TRACED_CONSTANT_P_WRAPPERS
FORCELOCAL struct heapptr KCALL
heap_alloc(struct heap *__restrict self, size_t num_bytes, gfp_t flags) {
 if (__builtin_constant_p(num_bytes))
     return __os_heap_alloc(self,
                           (num_bytes + (HEAP_ALIGNMENT-1)) &
                                       ~(HEAP_ALIGNMENT-1),
                            flags);
 return __os_heap_alloc(self,num_bytes,flags);
}
FORCELOCAL struct heapptr KCALL
heap_align(struct heap *__restrict self, size_t min_alignment,
           ptrdiff_t offset, size_t num_bytes, gfp_t flags) {
 if ((__builtin_constant_p(min_alignment) && min_alignment <= HEAP_ALIGNMENT) &&
     (__builtin_constant_p(offset) && (offset & (min_alignment-1)) == 0))
      return heap_alloc(self,num_bytes,flags);
 if (__builtin_constant_p(num_bytes))
     return __os_heap_align(self,min_alignment,offset,
                           (num_bytes + (HEAP_ALIGNMENT-1)) &
                                       ~(HEAP_ALIGNMENT-1),
                            flags);
 return __os_heap_align(self,min_alignment,offset,num_bytes,flags);
}
#endif /* !__OMIT_HEAP_TRACED_CONSTANT_P_WRAPPERS */
FORCELOCAL struct heapptr KCALL
heap_alloc_untraced(struct heap *__restrict self, size_t num_bytes, gfp_t flags) {
 if (__builtin_constant_p(num_bytes))
     return __os_heap_alloc_untraced(self,
                                    (num_bytes + (HEAP_ALIGNMENT-1)) &
                                                ~(HEAP_ALIGNMENT-1),
                                     flags);
 return __os_heap_alloc_untraced(self,num_bytes,flags);
}
FORCELOCAL struct heapptr KCALL
heap_align_untraced(struct heap *__restrict self, size_t min_alignment,
                    ptrdiff_t offset, size_t num_bytes, gfp_t flags) {
 if ((__builtin_constant_p(min_alignment) && min_alignment <= HEAP_ALIGNMENT) &&
     (__builtin_constant_p(offset) && (offset & (min_alignment-1)) == 0))
      return heap_alloc_untraced(self,num_bytes,flags);
 if (__builtin_constant_p(num_bytes))
     return __os_heap_align_untraced(self,min_alignment,offset,
                                    (num_bytes + (HEAP_ALIGNMENT-1)) &
                                                ~(HEAP_ALIGNMENT-1),
                                     flags);
 return __os_heap_align_untraced(self,min_alignment,offset,num_bytes,flags);
}

/* Try to allocate memory at a given address, returning the actual
 * number of bytes that the function managed to allocate, ZERO(0) if
 * allocation is not possible due to an overlap, or throw an `E_BADALLOC'
 * if the additional `num_bytes' could not be allocated.
 * NOTE: When `num_bytes' is less than `HEAP_MINSIZE', this function
 *       may actually return less than `HEAP_MINSIZE', too!
 *       With that in mind, always make sure that you've either already
 *       allocated neighboring data block, or that `num_bytes' is larger
 *       than, or equal to `HEAP_MINSIZE'.
 * @return: * : The amount of continuous bytes allocated (`>= num_bytes').
 * @return: 0 : Memory at the given address has already been allocated.
 * @assume(return == 0 || return >= num_bytes)
 * @param: flags: GFP flags used for allocation.
 * @throw: E_BADALLOC:    Failed to allocate sufficient memory for the operation.
 * @throw: E_WOULDBLOCK: `GFP_NOMAP' was specified and new memory would have had to be mapped.
 * @throw: E_WOULDBLOCK: `GFP_ATOMIC' was specified and a lock could not be acquired immediately. */
#ifndef __OMIT_HEAP_TRACED_CONSTANT_P_WRAPPERS
FORCELOCAL size_t KCALL
heap_allat(struct heap *__restrict self,
                  VIRT void *__restrict ptr,
                  size_t num_bytes, gfp_t flags) {
 if (__builtin_constant_p(num_bytes))
     return __os_heap_allat(self,ptr,
                           (num_bytes + (HEAP_ALIGNMENT-1)) &
                                       ~(HEAP_ALIGNMENT-1),
                            flags);
 return __os_heap_allat(self,ptr,num_bytes,flags);
}
#endif /* !__OMIT_HEAP_TRACED_CONSTANT_P_WRAPPERS */
FORCELOCAL size_t KCALL
heap_allat_untraced(struct heap *__restrict self,
           VIRT void *__restrict ptr,
           size_t num_bytes, gfp_t flags) {
 if (__builtin_constant_p(num_bytes))
     return __os_heap_allat_untraced(self,ptr,
                                    (num_bytes + (HEAP_ALIGNMENT-1)) &
                                                ~(HEAP_ALIGNMENT-1),
                                     flags);
 return __os_heap_allat_untraced(self,ptr,num_bytes,flags);
}

/* A convenience wrapper for `heap_alloc_untraced()', `heap_allat_untraced()' and
 * `heap_free_untraced()', implementing the following realloc()-style semantics:
 *   - realloc(ptr,0)                         --> Realloc to minimal size
 *   - realloc(ptr,malloc_usable_size(ptr)-x) --> Free unused memory at the end
 *   - realloc(ptr,malloc_usable_size(ptr)+x) --> Try to `heap_allat_untraced()' at the old end, or move to new memory
 *   - realloc(0,x)                           --> Same as `heap_alloc_untraced()'
 * NOTE: These functions always return a fully allocated heap data block.
 * NOTE: Alignment arguments passed to `heap_realign_untraced()' are ignored
 *       unless the initial, or a new data block have to be allocated.
 * NOTE: Allocation and free flags have been split to allow the user
 *       to request zero-initialized memory to be allocated for new
 *       allocations, while simultaniously releasing unused memory
 *       without indicating that it is zero-initialized.
 * NOTE: If realloc() needs to move the data block to a new location,
 *       the old location is freed using `free_flags & ~GFP_CALLOC'
 * NOTE: These functions do _NOT_ implement `GFP_NOMOVE' support, as it would
 *       not only be ambiguous in which set of flags that flag should appear,
 *       but it also wouldn't fit the image if this function returned anything
 *       other than a valid HEAP data block.
 *       If you want to extend a heap data block, simply use `heap_allat_untraced()'
 * @assume(return.hp_siz >= new_bytes);
 * @param: old_ptr:      [valid_if(old_bytes != 0)] Base address of the old data block.
 * @param: old_bytes:     The old size of the data block (can be ZERO(0); must be aligned to `HEAP_ALIGNMENT' by the caller)
 * @param: alloc_flags:   Set of `GFP_*' flags used for allocating data.
 * @param: free_flags:    Set of `GFP_*' flags used for freeing data.
 * @param: new_bytes:     The new size of the data block (will be aligned by `HEAP_ALIGNMENT').
 * @throw: E_BADALLOC:    Failed to allocate memory.
 * @throw: E_WOULDBLOCK: `GFP_NOMAP' was specified and new memory would have had to be mapped.
 * @throw: E_WOULDBLOCK: `GFP_ATOMIC' was specified and a lock could not be acquired immediately. */
#ifndef __OMIT_HEAP_TRACED_CONSTANT_P_WRAPPERS
FORCELOCAL struct heapptr KCALL
heap_realloc(struct heap *__restrict self,
             VIRT void *old_ptr, size_t old_bytes,
             size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags) {
 if (__builtin_constant_p(old_bytes)) {
  if (old_bytes == 0)
      return heap_alloc(self,new_bytes,alloc_flags);
 }
 if (__builtin_constant_p(new_bytes))
     return __os_heap_realloc(self,old_ptr,old_bytes,
                             (new_bytes + (HEAP_ALIGNMENT-1)) &
                                         ~(HEAP_ALIGNMENT-1),
                              alloc_flags,free_flags);
 return __os_heap_realloc(self,old_ptr,old_bytes,new_bytes,
                          alloc_flags,free_flags);
}
FORCELOCAL struct heapptr KCALL
heap_realign(struct heap *__restrict self,
             VIRT void *old_ptr, size_t old_bytes,
             size_t min_alignment, ptrdiff_t offset,
             size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags) {
 if (__builtin_constant_p(old_bytes)) {
  if (old_bytes == 0)
      return heap_align(self,min_alignment,offset,new_bytes,alloc_flags);
 }
 if ((__builtin_constant_p(min_alignment) && min_alignment <= HEAP_ALIGNMENT) &&
     (__builtin_constant_p(offset) && (offset & (min_alignment-1)) == 0))
      return heap_realloc(self,old_ptr,old_bytes,new_bytes,alloc_flags,free_flags);
 if (__builtin_constant_p(new_bytes))
     return __os_heap_realign(self,old_ptr,old_bytes,min_alignment,
                              offset,(new_bytes + (HEAP_ALIGNMENT-1)) &
                                                 ~(HEAP_ALIGNMENT-1),
                              alloc_flags,free_flags);
 return __os_heap_realign(self,old_ptr,old_bytes,min_alignment,
                          offset,new_bytes,alloc_flags,free_flags);
}
#endif /* !__OMIT_HEAP_TRACED_CONSTANT_P_WRAPPERS */
FORCELOCAL struct heapptr KCALL
heap_realloc_untraced(struct heap *__restrict self,
                      VIRT void *old_ptr, size_t old_bytes,
                      size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags) {
 if (__builtin_constant_p(old_bytes)) {
  if (old_bytes == 0)
      return heap_alloc_untraced(self,new_bytes,alloc_flags);
 }
 if (__builtin_constant_p(new_bytes))
     return __os_heap_realloc_untraced(self,old_ptr,old_bytes,
                                      (new_bytes + (HEAP_ALIGNMENT-1)) &
                                                  ~(HEAP_ALIGNMENT-1),
                                       alloc_flags,free_flags);
 return __os_heap_realloc_untraced(self,old_ptr,old_bytes,new_bytes,
                                   alloc_flags,free_flags);
}
FORCELOCAL struct heapptr KCALL
heap_realign_untraced(struct heap *__restrict self,
                      VIRT void *old_ptr, size_t old_bytes,
                      size_t min_alignment, ptrdiff_t offset,
                      size_t new_bytes, gfp_t alloc_flags, gfp_t free_flags) {
 if (__builtin_constant_p(old_bytes)) {
  if (old_bytes == 0)
      return heap_align_untraced(self,min_alignment,offset,new_bytes,alloc_flags);
 }
 if ((__builtin_constant_p(min_alignment) && min_alignment <= HEAP_ALIGNMENT) &&
     (__builtin_constant_p(offset) && (offset & (min_alignment-1)) == 0))
      return heap_realloc_untraced(self,old_ptr,old_bytes,new_bytes,alloc_flags,free_flags);
 if (__builtin_constant_p(new_bytes))
     return __os_heap_realign_untraced(self,old_ptr,old_bytes,min_alignment,
                                       offset,(new_bytes + (HEAP_ALIGNMENT-1)) &
                                                          ~(HEAP_ALIGNMENT-1),
                                       alloc_flags,free_flags);
 return __os_heap_realign_untraced(self,old_ptr,old_bytes,min_alignment,
                                   offset,new_bytes,alloc_flags,free_flags);
}
#endif /* !__OMIT_HEAP_CONSTANT_P_WRAPPERS */

/* Free the given memory, returning it to the heap.
 * The caller must ensure that `ptr' and `num_bytes' are aligned by
 * `HEAP_ALIGNMENT', and that `num_bytes' be least `HEAP_MINSIZE' bytes large.
 * The following flags affect the behavior of this function:
 *       - GFP_NOTRIM          -- Do not release large blocks of free data back to the core.
 *       - GFP_CALLOC          -- The given data block is ZERO-initialized.
 * @param: flags:     The flags that should be used when freeing data. (see above)
 *                    NOTE: The heap flags (`__GFP_HEAPMASK') must match those
 *                          passed during original allocation of the data block.
 * @param: ptr:       The HEAP_ALIGNMENT-aligned base pointer of the block to-be freed.
 * @param: num_bytes: The amount of bytes that should be freed.
 *                    NOTE: This argument must be aligned by `HEAP_ALIGNMENT',
 *                          and must not be equal to ZERO(0). */
FUNDEF ATTR_NOTHROW void KCALL
heap_free(struct heap *__restrict self,
          VIRT void *ptr, size_t num_bytes, gfp_t flags);
FUNDEF ATTR_NOTHROW void KCALL
heap_free_untraced(struct heap *__restrict self,
                   VIRT void *ptr, size_t num_bytes, gfp_t flags);

#if 0
/* Tries to allocate memory from cache. If no cached memory is available, `return.hp_siz == 0'
 * Additionally, this function never throws an exception or blocks, meaning that it's kind-of
 * a combination between GFP_NOMAP and GFP_ATOMIC, just without the exceptions...
 * NOTE: The returned memory is always untraced!
 * The intended use of this function is for light-weight caches, as well as RTL
 * components which themself might be invoked during exception handling, and
 * stack unwinding.
 * If the function fails to lock the heap and preemption has been disabled,
 * the error szenario of returning `return.hp_siz == 0' happens, too. */
FUNDEF ATTR_NOTHROW struct heapptr KCALL
heap_alloc_nothrow(struct heap *__restrict self, size_t num_bytes, gfp_t flags);
#endif


/* Truncate the given heap, releasing unmapping free memory chunks
 * that are greater than, or equal to `CEIL_ALIGN(threshold,PAGESIZE)'
 * This function is automatically invoked for kernel heaps as part of
 * the clear-cache machinery, though regular should never feel moved
 * to invoke this function manually, as all it really does is slow down
 * future calls to allocating heap functions.
 * @return: * : The total number of bytes released back to the core (a multiple of PAGESIZE) */
FUNDEF size_t KCALL heap_trim(struct heap *__restrict self, size_t threshold);


#ifdef CONFIG_DEBUG_HEAP
/* Validate the memory of the given heap for
 * consistency, checking for invalid use-after-free.
 * NOTE: When `heap_validate_all()' is called in a user-task,
 *      `GFP_KERNEL' heaps are not validated as their memory
 *       is inaccessible to the active VM of that thread. */
FUNDEF void KCALL heap_validate(struct heap *__restrict self);
FUNDEF void KCALL heap_validate_all(void);
#else
#define heap_validate(self)    (void)0
#define heap_validate_all()    (void)0
#endif


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_HEAP_H */
