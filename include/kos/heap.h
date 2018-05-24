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
#ifndef _KOS_HEAP_H
#define _KOS_HEAP_H 1

#include <__stdinc.h>

#ifdef __KERNEL__
#include <kernel/heap.h>
#else /* __KERNEL__ */

#include <asm/types.h>
#include <hybrid/align.h>
#include <hybrid/list/list.h>
#include <hybrid/list/atree.h>
#include <hybrid/sync/atomic-rwlock.h>

__SYSDECL_BEGIN

/* User-space re-implementation of KOS's kernel-space heap objects.
 * The basic logic is the same, and heap validation makes use of a
 * hidden system call (not so hidden now that I mentioned it).
 * Anyways: `libc' exports all possible combinations of heap usage
 *           functions, meaning that an application is able to
 *           configure the desired behavior of heap functions itself. */


//#define CONFIG_NO_DEBUG_HEAP             1
//#define CONFIG_NO_HEAP_RANDOMIZE_OFFSETS 1
//#define CONFIG_NO_HEAP_TRACE_DANGLE      1
//#define CONFIG_HEAP_ALIGNMENT            power-of-2


/* Enable heap memory pre-initialization, as well as special
 * data patterns for unallocated memory, in addition to the
 * ability of tracking use-after-free through `heap_validate()'
 * Note that `heap_validate()' attempts to optimize itself not
 * to check data blocks that haven't been modified since the
 * previous check, making use of `pagedir_haschanged()', should
 * the host support that function (`CONFIG_HAVE_PAGEDIR_CHANGED') */
#ifndef CONFIG_DEBUG_HEAP
#if !defined(CONFIG_NO_DEBUG_HEAP) && !defined(NDEBUG)
#define CONFIG_DEBUG_HEAP 1
#elif !defined(CONFIG_NO_DEBUG_HEAP)
#define CONFIG_NO_DEBUG_HEAP 1
#endif
#elif defined(CONFIG_NO_DEBUG_HEAP)
#undef CONFIG_DEBUG_HEAP
#endif


/* Randomize in-heap allocation offsets (using `rand()') when
 * less memory than the best matching free slot contains is
 * allocated:
 * >> slot(48 bytes):  FREEFREEFREEFREEFREEFREEFREEFREEFREEFREEFREEFREE
 * >> alloc(16 bytes): FREEFREEFREEFREE                                 (Without `CONFIG_HEAP_RANDOMIZE_OFFSETS')
 * >> alloc(16 bytes):                                 FREEFREEFREEFREE (With `CONFIG_HEAP_RANDOMIZE_OFFSETS')
 * >> alloc(16 bytes):                 FREEFREEFREEFREE                 (With `CONFIG_HEAP_RANDOMIZE_OFFSETS')
 * >> alloc(16 bytes): FREEFREEFREEFREE                                 (With `CONFIG_HEAP_RANDOMIZE_OFFSETS')
 * This configuration option affects the results of `heap_alloc()' and `heap_align()'
 * Reasoning:
 *   - Although even without this option, we are randomizing the kernel heap location
 *     during early boot, once that's been established, no further randomization would
 *     be done without this option enabled.
 *     That could lead to bugs going undetected that depend on the relative offsets
 *     between allocated data blocks, something that this option prevents by ensuring
 *     that more randomization is added into the mix whenever memory is allocated.
 *   - Without this option, only address bits above `PAGESIZE' would be randomized
 *     for the kernel's default heaps, while bits between it and `HEAP_ALIGNMENT'
 *     would be predictably consistent for consecutive allocations.
 *     That might be another cause for buggy code that might accidentally rely on
 *     those bits never changing.
 */
#ifndef CONFIG_HEAP_RANDOMIZE_OFFSETS
#if !defined(CONFIG_NO_HEAP_RANDOMIZE_OFFSETS) && \
     defined(CONFIG_DEBUG_HEAP)
#define CONFIG_HEAP_RANDOMIZE_OFFSETS 1
#elif !defined(CONFIG_NO_HEAP_RANDOMIZE_OFFSETS)
#define CONFIG_NO_HEAP_RANDOMIZE_OFFSETS 1
#endif
#elif defined(CONFIG_NO_HEAP_RANDOMIZE_OFFSETS)
#undef CONFIG_HEAP_RANDOMIZE_OFFSETS
#endif



#define HEAP_TYPE_FNORMAL 0x0000 /* No options enabled. */
#define HEAP_TYPE_FDEBUG  0x0001 /* CONFIG_DEBUG_HEAP */
#define HEAP_TYPE_FALIGN  0x0002 /* HEAP_ALIGNMENT != __DEFAULT_HEAP_ALIGNMENT */

/* Figure out the heap type selected by the current configuration. */
#ifdef CONFIG_DEBUG_HEAP
#if HEAP_ALIGNMENT == __DEFAULT_HEAP_ALIGNMENT
#   define HEAP_TYPE_FCURRENT  HEAP_TYPE_FDEBUG
#else
#   define HEAP_TYPE_FCURRENT (HEAP_TYPE_FDEBUG|HEAP_TYPE_FALIGN)
#endif
#else /* CONFIG_DEBUG_HEAP */
#if HEAP_ALIGNMENT == __DEFAULT_HEAP_ALIGNMENT
#   define HEAP_TYPE_FCURRENT  HEAP_TYPE_FNORMAL
#else
#   define HEAP_TYPE_FCURRENT  HEAP_TYPE_FALIGN
#endif
#endif /* !CONFIG_DEBUG_HEAP */



#if __SIZEOF_POINTER__ == 4
/* Using 16 allows a human to quickly notice heap pointers by realizing
 * that the last digit in a hexadecimal representation is ZERO(0). */
#define __HEAP_GET_DEFAULT_ALIGNMENT(heap_type) \
      ((heap_type) & HEAP_TYPE_FDEBUG ? 16 : 8)
#else
#define __HEAP_GET_DEFAULT_ALIGNMENT(heap_type) 16
#endif

#define __DEFAULT_HEAP_ALIGNMENT   \
        __HEAP_GET_DEFAULT_ALIGNMENT(HEAP_TYPE_FCURRENT)


/* Minimum alignment of all heap pointers. */
#ifndef HEAP_ALIGNMENT
#ifdef CONFIG_HEAP_ALIGNMENT
#   define HEAP_ALIGNMENT  CONFIG_HEAP_ALIGNMENT
#else
#   define HEAP_ALIGNMENT __DEFAULT_HEAP_ALIGNMENT
#endif
#endif


#if (HEAP_ALIGNMENT & (HEAP_ALIGNMENT-1))
#error "Invalid `HEAP_ALIGNMENT' must be a power-of-2 number"
#endif


#ifdef CONFIG_DEBUG_HEAP
#define DEBUGHEAP_NO_MANS_LAND  0xdeadbeef /* Debug initialization of unallocated memory. */
#define DEBUGHEAP_FRESH_MEMORY  0xaaaaaaaa /* Debug initialization of freshly allocated memory. */
#endif /* CONFIG_DEBUG_HEAP */


#ifdef __EXPOSE_HEAP_INTERNALS
#define __HEAP_MEMBER(x)     x
#else
#define __HEAP_MEMBER(x) __##x##__
#endif


#ifdef __EXPOSE_HEAP_INTERNALS
#define SIZEOF_MFREE    offsetof(struct mfree,mf_data)
#define HEAP_MINSIZE    CEIL_ALIGN(SIZEOF_MFREE,HEAP_ALIGNMENT)
#else
#define HEAP_MINSIZE    CEIL_ALIGN(offsetof(struct mfree,__mf_data__),HEAP_ALIGNMENT)
#endif

struct __ATTR_PACKED __HEAP_MEMBER(mfree) {
    LIST_NODE(struct __HEAP_MEMBER(mfree))
                              __HEAP_MEMBER(mf_lsize);   /* [lock(:h_lock)][sort(ASCENDING(mf_size))] List of free entries ordered by size. */
    ATREE_XNODE(struct __HEAP_MEMBER(mfree))
                              __HEAP_MEMBER(mf_laddr);   /* [lock(:h_lock)][sort(ASCENDING(self))] List of free entries ordered by address. */
    __size_t                  __HEAP_MEMBER(mf_size);    /* Size of this block (in bytes; aligned by `HEAP_ALIGNMENT'; including this header) */
#ifdef __EXPOSE_HEAP_INTERNALS
#define MFREE_FUNDEFINED      0x00        /* Memory initialization is undefined.
                                           * In debug mode, this means that memory is
                                           * initialized using `DEBUGHEAP_NO_MANS_LAND',
                                           * with portions that haven't been allocated yet
                                           * pending initialization for either `DEBUGHEAP_FRESH_MEMORY'
                                           * or ZERO(0), depending on how they were originally allocated. */
#define MFREE_FZERO           GFP_CALLOC  /* Memory is ZERO-initialized. */
#define MFREE_FMASK           MFREE_FZERO /* Mask of known flags. */
#endif /* __EXPOSE_HEAP_INTERNALS */
    __UINT8_TYPE__            __HEAP_MEMBER(mf_flags);   /* Set of `MFREE_F*' */
#ifdef CONFIG_DEBUG_HEAP
    __UINT8_TYPE__            __HEAP_MEMBER(mf_szchk);   /* Checksum for `mf_size' */
#endif
    __BYTE_TYPE__             __HEAP_MEMBER(mf_data)[1]; /* Block data. */
};
#ifdef __EXPOSE_HEAP_INTERNALS
#define MFREE_MIN(self)   ((__uintptr_t)(self))
#define MFREE_MAX(self)   ((__uintptr_t)(self)+(self)->mf_size-1)
#define MFREE_BEGIN(self) ((__uintptr_t)(self))
#define MFREE_END(self)   ((__uintptr_t)(self)+(self)->mf_size)
#define MFREE_SIZE(self)                (self)->mf_size
#endif

/* Heap configuration:
 * Index offset for the first bucket that should be search for a given size. */
#if HEAP_ALIGNMENT != __DEFAULT_HEAP_ALIGNMENT
#define __HEAP_BUCKET_OFFSET     1
#elif __DEFAULT_HEAP_ALIGNMENT == 4
#define __HEAP_BUCKET_OFFSET     3 /* FFS(__DEFAULT_HEAP_ALIGNMENT) */
#elif __DEFAULT_HEAP_ALIGNMENT == 8
#define __HEAP_BUCKET_OFFSET     4 /* FFS(__DEFAULT_HEAP_ALIGNMENT) */
#elif __DEFAULT_HEAP_ALIGNMENT == 16
#define __HEAP_BUCKET_OFFSET     5 /* FFS(__DEFAULT_HEAP_ALIGNMENT) */
#elif __DEFAULT_HEAP_ALIGNMENT == 32
#define __HEAP_BUCKET_OFFSET     6 /* FFS(__DEFAULT_HEAP_ALIGNMENT) */
#else
#error "Invalid default alignment"
#endif

#ifdef __EXPOSE_HEAP_INTERNALS
#include <hybrid/__bit.h>
#define HEAP_BUCKET_OFFSET        __HEAP_BUCKET_OFFSET
#define HEAP_BUCKET_OF(size)   (((__SIZEOF_SIZE_T__*8)-__hybrid_clz(size))-HEAP_BUCKET_OFFSET)
#define HEAP_BUCKET_MINSIZE(i)   (1 << ((i)+HEAP_BUCKET_OFFSET-1))
#endif /* __EXPOSE_HEAP_INTERNALS */
#define HEAP_BUCKET_COUNT       ((__SIZEOF_SIZE_T__*8)-(__HEAP_BUCKET_OFFSET-1))

/* Figure out how to link against the selected heap type. */
#if HEAP_TYPE_FCURRENT == (HEAP_TYPE_FDEBUG|HEAP_TYPE_FALIGN)
#   define __HEAP_FUNCTION(x)  x##_da
#elif HEAP_TYPE_FCURRENT == HEAP_TYPE_FDEBUG
#   define __HEAP_FUNCTION(x)  x##_d
#elif HEAP_TYPE_FCURRENT == HEAP_TYPE_FALIGN
#   define __HEAP_FUNCTION(x)  x##_a
#elif HEAP_TYPE_FCURRENT == HEAP_TYPE_FNORMAL
#   define __HEAP_FUNCTION(x)  x
#else
#   error "Invalid current heap type"
#endif


/* Heap operation flags. */
#define HEAP_FNORMAL       0x0000
#define HEAP_FUSEHINTS     0x0001 /* Make use of address hints when allocating memory. */
#define HEAP_FDOWNHINT     0x0002 /* The heap grows downwards, rather than upwards. */
#define HEAP_FRANDOMIZE    0x0004 /* CONFIG_HEAP_RANDOMIZE_OFFSETS. */

#ifdef CONFIG_HEAP_RANDOMIZE_OFFSETS
#   define HEAP_FCURRENT   HEAP_FRANDOMIZE
#else /* CONFIG_HEAP_RANDOMIZE_OFFSETS */
#   define HEAP_FCURRENT   HEAP_FNORMAL
#endif /* !CONFIG_HEAP_RANDOMIZE_OFFSETS */

struct __ATTR_PACKED __HEAP_MEMBER(heapstat) {
    WEAK __size_t             __HEAP_MEMBER(hs_alloc);     /* Total number of bytes currently allocated. */
    WEAK __size_t             __HEAP_MEMBER(hs_mmap);      /* Total number of bytes currently mapped for the heap. */
    WEAK __size_t             __HEAP_MEMBER(hs_mmap_peak); /* The greatest number of bytes ever allocated from the system. */
};

struct __ATTR_PACKED heap {
    __UINTPTR_HALF_TYPE__     __HEAP_MEMBER(h_type);       /* [== HEAP_TYPE_FCURRENT][const] The type of heap. */
    __UINTPTR_HALF_TYPE__     __HEAP_MEMBER(h_flags);      /* Heap flags (Set of `HEAP_F*'). */
    atomic_rwlock_t           __HEAP_MEMBER(h_lock);       /* Lock for this heap. */
    ATREE_HEAD(struct __HEAP_MEMBER(mfree))
                              __HEAP_MEMBER(h_addr);       /* [lock(h_lock)][0..1] Heap sorted by address. */
    LIST_HEAD(struct __HEAP_MEMBER(mfree))                 /* [lock(h_lock)][0..1][*] Heap sorted by free range size. */
                              __HEAP_MEMBER(h_size)[HEAP_BUCKET_COUNT];
    WEAK __size_t             __HEAP_MEMBER(h_overalloc);  /* Amount (in bytes) by which to over-allocate memory in heaps.
                                                            * NOTE: Set to ZERO(0) to disable overallocation. */
    WEAK __size_t             __HEAP_MEMBER(h_freethresh); /* Threshold that must be reached before any continuous block of free
                                                            * data is unmapped from the kernel VM. (Should always be `>= PAGESIZE') */
    WEAK void                *__HEAP_MEMBER(h_corehint);   /* [valid_if(HEAP_FUSEHINTS)] Hint for where to allocate new memory. */
    ATOMIC_DATA __size_t      __HEAP_MEMBER(h_dangle);     /* [lock(INCREMENT(h_lock),DECREMENT(atomic),READ(atomic))]
                                                            * Amount of dangling bytes of memory (memory that was allocated, but may be
                                                            * release again shortly) When new memory would have to be requested from the
                                                            * core, this field is checked to see if it is likely that some large block
                                                            * of memory will be released soon, preventing a race condition that would
                                                            * unnecessarily allocate more memory when `heap_free()' is merging a data
                                                            * block with another, larger data block, for which it must temporarily
                                                            * allocate that larger data block. Another thread allocating memory at the
                                                            * same time could otherwise decide that the cache has grown too small for
                                                            * its allocation and unnecessarily request more memory from the core. */
    struct __HEAP_MEMBER(heapstat) __HEAP_MEMBER(h_stat);  /* [lock(h_lock)] Heap statistics. */
#ifdef CONFIG_DEBUG_HEAP
    LIST_NODE(struct heap)    __HEAP_MEMBER(h_chain);      /* [lock(INTERNAL(...))] Chain of all known debug heaps (for `heap_validate_all()') */
#endif /* CONFIG_DEBUG_HEAP */
#if HEAP_ALIGNMENT != __DEFAULT_HEAP_ALIGNMENT
    __size_t                  __HEAP_MEMBER(h_almask);     /* [const] Heap alignment mask (== HEAP_ALIGNMENT-1) */
    __size_t                  __HEAP_MEMBER(h_ialmask);    /* [const] Inverse heap alignment mask (== ~(HEAP_ALIGNMENT-1)) */
    __size_t                  __HEAP_MEMBER(h_minsize);    /* [const] Heap min size (== (offsetof(struct mfree,mf_data) + HEAP_ALIGNMENT-1) & ~(HEAP_ALIGNMENT-1)) */
#endif
};                            


#ifdef CONFIG_DEBUG_HEAP
#define __HEAP_INIT_H_CHAIN  , { NULL, NULL }
#else /* CONFIG_DEBUG_HEAP */
#define __HEAP_INIT_H_CHAIN  /* nothing */
#endif /* !CONFIG_DEBUG_HEAP */
#if HEAP_ALIGNMENT != __DEFAULT_HEAP_ALIGNMENT
#define __HEAP_INIT_H_ALMASK \
    , HEAP_ALIGNMENT-1, ~(HEAP_ALIGNMENT-1)\
    , (__builtin_offsetof(struct __HEAP_MEMBER(mfree),mf_data) + HEAP_ALIGNMENT-1) & ~(HEAP_ALIGNMENT-1)
#else
#define __HEAP_INIT_H_ALMASK /* nothing */
#endif


/* Static initializer for heaps:
 * >> struct heap my_heap = HEAP_INIT(PAGESIZE*2,PAGESIZE*16);
 */
#define HEAP_INIT(overalloc,free_threshold) \
      { HEAP_TYPE_FCURRENT, HEAP_FCURRENT, ATOMIC_RWLOCK_INIT, \
        NULL, { NULL, }, (overalloc), (free_threshold), NULL, 0, \
      { 0, 0, 0 } __HEAP_INIT_H_CHAIN __HEAP_INIT_H_ALMASK }


#ifndef __heapptr_defined
#define __heapptr_defined 1
struct heapptr {
    void     *hp_ptr; /* [1..hp_siz] Pointer base address. */
    __size_t  hp_siz; /* [!0] Size of the pointer. */
};
#endif /* !__heapptr_defined */


#ifndef __heapinfo_defined
#define __heapinfo_defined 1
struct heapinfo {
    __size_t    hi_trimable;   /* Total amount of memory that could be heap_trim()-ed (in bytes). */
    __size_t    hi_free;       /* Total size of currently free memory. */
    __size_t    hi_free_z;     /* [<= hi_free] Amount of ZERO-initialized free memory. */
    __size_t    hi_free_min;   /* [<= hi_free_max] Size of the smallest free heap node, or ZERO if there are none. */
    __size_t    hi_free_max;   /* [>= hi_free_min] Size of the largest free heap node, or ZERO if there are none. */
    __size_t    hi_free_cnt;   /* Amount of free heap nodes available for allocation. */
    __size_t    hi_alloc;      /* Total number of bytes allocated using the heap. */
    __size_t    hi_mmap;       /* Total number of bytes currently mapped for the heap. */
    __size_t    hi_mmap_peak;  /* The greatest number of bytes ever allocated at the same time. */
};
#endif /* !__heapinfo_defined */



#define GFP_CALLOC   0x0010 /* Zero-initialize newly allocated memory. */
#define GFP_ATOMIC   0x0800 /* [== O_NONBLOCK] Do not block, but throw a resumable `E_WOULDBLOCK' error when a lock could not be acquired immediately. */
#define GFP_NOOVER   0x1000 /* Do not overallocate when mapping new memory.
                             * Any (logical) call to `mmap()' and friends will
                             * be limited to `CEIL_ALIGN(n_bytes,PAGESIZE)'. */
#define GFP_NOMAP    0x4000 /* Do not map new memory, but throw an `E_WOULDBLOCK' error when caches do not contain any memory.
                             * Additionally, do not unmap system memory when this flag is set during a free-operation (including realloc when reducing a pointer size) */
#define GFP_NOTRIM   0x4000 /* Don't unmap memory once free blocks grow to sufficient lengths. */
#ifndef __gfp_t_defined
#define __gfp_t_defined 1
typedef unsigned int gfp_t; /* Set of `GFP_*' */
#endif /* !__gfp_t_defined */

/* Link selected heap functions. */
#if HEAP_ALIGNMENT != __DEFAULT_HEAP_ALIGNMENT
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_heap_init,(struct heap *__restrict __self, __size_t __overalloc, __size_t __free_threshold, __UINT16_TYPE__ __flags, __size_t __heap_alignment),__HEAP_FUNCTION(heap_init),(__self,overalloc,free_threshold,__flags,__heap_alignment))
#else
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_heap_init,(struct heap *__restrict __self, __size_t __overalloc, __size_t __free_threshold, __UINT16_TYPE__ __flags),__HEAP_FUNCTION(heap_init),(__self,overalloc,free_threshold,__flags))
#endif
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_heap_fini,(struct heap *__restrict __self),__HEAP_FUNCTION(heap_fini),(__self))
__REDIRECT(__LIBC,,struct heapptr,__LIBCCALL,__os_heap_alloc,(struct heap *__restrict __self, __size_t __num_bytes, gfp_t __flags),__HEAP_FUNCTION(heap_alloc),(__self,__num_bytes,__flags))
__REDIRECT(__LIBC,,struct heapptr,__LIBCCALL,__os_heap_align,(struct heap *__restrict __self, __size_t __min_alignment, __ptrdiff_t __offset, __size_t __num_bytes, gfp_t __flags),__HEAP_FUNCTION(heap_align),(__self,__min_alignment,__offset,__num_bytes,__flags))
__REDIRECT(__LIBC,,__size_t,__LIBCCALL,__os_heap_allat,(struct heap *__restrict __self, void *__restrict __ptr, __size_t __num_bytes, gfp_t __flags),__HEAP_FUNCTION(heap_allat),(__self,__ptr,__num_bytes,__flags))
__REDIRECT(__LIBC,,void,__LIBCCALL,__os_heap_free,(struct heap *__restrict __self, void *__ptr, __size_t __num_bytes, gfp_t __flags),__HEAP_FUNCTION(heap_free),(__self,__ptr,__num_bytes,__flags))
__REDIRECT(__LIBC,,struct heapptr,__LIBCCALL,__os_heap_realloc,(struct heap *__restrict __self, void *__old_ptr, __size_t __old_bytes, __size_t __new_bytes, gfp_t __alloc_flags, gfp_t __free_flags),__HEAP_FUNCTION(heap_realloc),(__self,__old_ptr,__old_bytes,__new_bytes,__alloc_flags,__free_flags))
__REDIRECT(__LIBC,,struct heapptr,__LIBCCALL,__os_heap_realign,(struct heap *__restrict __self, void *__old_ptr, __size_t __old_bytes, __size_t __min_alignment, __ptrdiff_t __offset, __size_t __new_bytes, gfp_t __alloc_flags, gfp_t __free_flags),__HEAP_FUNCTION(heap_realign),(__self,__old_ptr,__old_bytes,__min_alignment,__offset,__new_bytes,__alloc_flags,__free_flags))
__REDIRECT(__LIBC,,__size_t,__LIBCCALL,__os_heap_trim,(struct heap *__restrict __self, __size_t __threshold),__HEAP_FUNCTION(heap_trim),(__self,__threshold))
__REDIRECT(__LIBC,,struct heapinfo,__LIBCCALL,__os_heap_info,(struct heap *__restrict __self),__HEAP_FUNCTION(heap_info),(__self))
#if (HEAP_TYPE_FCURRENT & HEAP_TYPE_FDEBUG)
__REDIRECT(__LIBC,,struct heapptr,__LIBCCALL,__os_heap_alloc_untraced,(struct heap *__restrict __self, __size_t __num_bytes, gfp_t __flags),__HEAP_FUNCTION(heap_alloc_untraced),(__self,__num_bytes,__flags))
__REDIRECT(__LIBC,,struct heapptr,__LIBCCALL,__os_heap_align_untraced,(struct heap *__restrict __self, __size_t __min_alignment, __ptrdiff_t __offset, __size_t __num_bytes, gfp_t __flags),__HEAP_FUNCTION(heap_align_untraced),(__self,__min_alignment,__offset,__num_bytes,__flags))
__REDIRECT(__LIBC,,__size_t,__LIBCCALL,__os_heap_allat_untraced,(struct heap *__restrict __self, void *__restrict __ptr, __size_t __num_bytes, gfp_t __flags),__HEAP_FUNCTION(heap_allat_untraced),(__self,__ptr,__num_bytes,__flags))
__REDIRECT(__LIBC,,void,__LIBCCALL,__os_heap_free_untraced,(struct heap *__restrict __self, void *__ptr, __size_t __num_bytes, gfp_t __flags),__HEAP_FUNCTION(heap_free_untraced),(__self,__ptr,__num_bytes,__flags))
__REDIRECT(__LIBC,,struct heapptr,__LIBCCALL,__os_heap_realloc_untraced,(struct heap *__restrict __self, void *__old_ptr, __size_t __old_bytes, __size_t __new_bytes, gfp_t __alloc_flags, gfp_t __free_flags),__HEAP_FUNCTION(heap_realloc_untraced),(__self,__old_ptr,__old_bytes,__new_bytes,__alloc_flags,__free_flags))
__REDIRECT(__LIBC,,struct heapptr,__LIBCCALL,__os_heap_realign_untraced,(struct heap *__restrict __self, void *__old_ptr, __size_t __old_bytes, __size_t __min_alignment, __ptrdiff_t __offset, __size_t __new_bytes, gfp_t __alloc_flags, gfp_t __free_flags),__HEAP_FUNCTION(heap_realign_untraced),(__self,__old_ptr,__old_bytes,__min_alignment,__offset,__new_bytes,__alloc_flags,__free_flags))
#else
__REDIRECT(__LIBC,,struct heapptr,__LIBCCALL,__os_heap_alloc_untraced,(struct heap *__restrict __self, __size_t __num_bytes, gfp_t __flags),__HEAP_FUNCTION(heap_alloc),(__self,__num_bytes,__flags))
__REDIRECT(__LIBC,,struct heapptr,__LIBCCALL,__os_heap_align_untraced,(struct heap *__restrict __self, __size_t __min_alignment, __ptrdiff_t __offset, __size_t __num_bytes, gfp_t __flags),__HEAP_FUNCTION(heap_align),(__self,__min_alignment,__offset,__num_bytes,__flags))
__REDIRECT(__LIBC,,__size_t,__LIBCCALL,__os_heap_allat_untraced,(struct heap *__restrict __self, void *__restrict __ptr, __size_t __num_bytes, gfp_t __flags),__HEAP_FUNCTION(heap_allat),(__self,__ptr,__num_bytes,__flags))
__REDIRECT(__LIBC,,void,__LIBCCALL,__os_heap_free_untraced,(struct heap *__restrict __self, void *__ptr, __size_t __num_bytes, gfp_t __flags),__HEAP_FUNCTION(heap_free),(__self,__ptr,__num_bytes,__flags))
__REDIRECT(__LIBC,,struct heapptr,__LIBCCALL,__os_heap_realloc_untraced,(struct heap *__restrict __self, void *__old_ptr, __size_t __old_bytes, __size_t __new_bytes, gfp_t __alloc_flags, gfp_t __free_flags),__HEAP_FUNCTION(heap_realloc),(__self,__old_ptr,__old_bytes,__new_bytes,__alloc_flags,__free_flags))
__REDIRECT(__LIBC,,struct heapptr,__LIBCCALL,__os_heap_realign_untraced,(struct heap *__restrict __self, void *__old_ptr, __size_t __old_bytes, __size_t __min_alignment, __ptrdiff_t __offset, __size_t __new_bytes, gfp_t __alloc_flags, gfp_t __free_flags),__HEAP_FUNCTION(heap_realign),(__self,__old_ptr,__old_bytes,__min_alignment,__offset,__new_bytes,__alloc_flags,__free_flags))
#endif


#ifndef __OMIT_HEAP_CONSTANT_P_WRAPPERS

/* Initialize/finalize a dynamically allocated heap. */
__FORCELOCAL void
(__LIBCCALL heap_init)(struct heap *__restrict __self,
                       __size_t __overalloc,
                       __size_t __free_threshold) {
#if HEAP_ALIGNMENT != __DEFAULT_HEAP_ALIGNMENT
 __os_heap_init(__self,__overalloc,__free_threshold,HEAP_FCURRENT,HEAP_ALIGNMENT);
#else
 __os_heap_init(__self,__overalloc,__free_threshold,HEAP_FCURRENT);
#endif
}
__FORCELOCAL void
(__LIBCCALL heap_fini)(struct heap *__restrict __self) {
 __os_heap_fini(__self);
}


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
 *    heap_alloc / heap_align:
 *       - GFP_CALLOC          -- Allocate ZERO-initialized memory (in `return.hp_ptr ...+= return.hp_siz')
 *       - GFP_NOOVER          -- Do not overallocate (by `self->h_overalloc')
 *       - GFP_NOMAP           -- Don't map new memory (throw `E_WOULDBLOCK' instead)
 *       - GFP_ATOMIC          -- Do not block when mapping new memory (throw `E_WOULDBLOCK' instead)
 *       - GFP_NOFS|GFP_NOSWAP -- Behavioral modifiers for swapping memory
 *    heap_free:
 *       - GFP_CALLOC          -- The given memory block is ZERO-initialized (allows for some internal optimizations)
 *       - GFP_NOTRIM          -- Do not `vm_unmap()' free memory blocks larger than
 *                               `h_freethresh', but keep them in cache instead.
 * NOTE: `heap_free()' always completes without blocking.
 *        If `vm_unmap()' needs to be called, the free() operation
 *        is either postponed until the next call to a heap function
 *        that is allowed to block, or is simply kept in cache, as
 *        though `GFP_NOMAP'  has been passed.
 * NOTE:  The `*_untraced' family of functions can be used to allocate memory
 *        which itself is still tracked as a potential candidate for memory
 *        leaks, however will not be used as a forwarding point for other
 *        memory leaks.
 * HINT:  Using heap functions, `malloc()' can be implemented as something as
 *        simple as having a global heap and allocating the requested memory
 *        size + HEAP_ALIGNMENT, then saving the allocated size just before
 *        the user data area, which is then read from by free() and realloc().
 * @param: flags: Set of `GFP_*' flags used for allocation.
 * @throw: E_BADALLOC:    Failed to allocate memory.
 * @throw: E_WOULDBLOCK: `GFP_NOMAP' was specified and new memory would have had to be mapped.
 * @throw: E_WOULDBLOCK: `GFP_ATOMIC' was specified and a lock could not be acquired immediately. */
__FORCELOCAL struct heapptr
(__LIBCCALL heap_alloc)(struct heap *__restrict __self, __size_t __num_bytes, gfp_t __flags) {
 if (__builtin_constant_p(__num_bytes))
     return __os_heap_alloc(__self,
                           (__num_bytes + (HEAP_ALIGNMENT-1)) &
                                         ~(HEAP_ALIGNMENT-1),
                            __flags);
 return __os_heap_alloc(__self,__num_bytes,__flags);
}
__FORCELOCAL struct heapptr
(__LIBCCALL heap_align)(struct heap *__restrict __self, __size_t __min_alignment,
                        __ptrdiff_t __offset, __size_t __num_bytes, gfp_t __flags) {
 if ((__builtin_constant_p(__min_alignment) && __min_alignment <= HEAP_ALIGNMENT) &&
     (__builtin_constant_p(__offset) && (__offset & (__min_alignment-1)) == 0))
      return heap_alloc(__self,__num_bytes,__flags);
 if (__builtin_constant_p(__num_bytes))
     return __os_heap_align(__self,__min_alignment,__offset,
                           (__num_bytes + (HEAP_ALIGNMENT-1)) &
                                         ~(HEAP_ALIGNMENT-1),
                            __flags);
 return __os_heap_align(__self,__min_alignment,__offset,__num_bytes,__flags);
}
#ifndef __OMIT_HEAP_TRACED_CONSTANT_P_WRAPPERS
__FORCELOCAL struct heapptr
(__LIBCCALL heap_alloc_untraced)(struct heap *__restrict __self, __size_t __num_bytes, gfp_t __flags) {
 if (__builtin_constant_p(__num_bytes))
     return __os_heap_alloc_untraced(__self,
                                    (__num_bytes + (HEAP_ALIGNMENT-1)) &
                                                ~(HEAP_ALIGNMENT-1),
                                     __flags);
 return __os_heap_alloc_untraced(__self,__num_bytes,__flags);
}
__FORCELOCAL struct heapptr
(__LIBCCALL heap_align_untraced)(struct heap *__restrict __self, __size_t __min_alignment,
                                 __ptrdiff_t __offset, __size_t __num_bytes, gfp_t __flags) {
 if ((__builtin_constant_p(__min_alignment) && __min_alignment <= HEAP_ALIGNMENT) &&
     (__builtin_constant_p(__offset) && (__offset & (__min_alignment-1)) == 0))
      return heap_alloc_untraced(__self,__num_bytes,__flags);
 if (__builtin_constant_p(__num_bytes))
     return __os_heap_align_untraced(__self,__min_alignment,__offset,
                                    (__num_bytes + (HEAP_ALIGNMENT-1)) &
                                                  ~(HEAP_ALIGNMENT-1),
                                     __flags);
 return __os_heap_align_untraced(__self,__min_alignment,__offset,__num_bytes,__flags);
}
#endif /* !__OMIT_HEAP_TRACED_CONSTANT_P_WRAPPERS */

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
__FORCELOCAL __size_t
(__LIBCCALL heap_allat)(struct heap *__restrict __self,
                        void *__restrict __ptr,
                        __size_t __num_bytes, gfp_t __flags) {
 if (__builtin_constant_p(__num_bytes))
     return __os_heap_allat(__self,__ptr,
                           (__num_bytes + (HEAP_ALIGNMENT-1)) &
                                         ~(HEAP_ALIGNMENT-1),
                            __flags);
 return __os_heap_allat(__self,__ptr,__num_bytes,__flags);
}
#ifndef __OMIT_HEAP_TRACED_CONSTANT_P_WRAPPERS
__FORCELOCAL __size_t
(__LIBCCALL heap_allat_untraced)(struct heap *__restrict __self,
                                 void *__restrict __ptr,
                                 __size_t __num_bytes, gfp_t __flags) {
 if (__builtin_constant_p(__num_bytes))
     return __os_heap_allat_untraced(__self,__ptr,
                                    (__num_bytes + (HEAP_ALIGNMENT-1)) &
                                                  ~(HEAP_ALIGNMENT-1),
                                     __flags);
 return __os_heap_allat_untraced(__self,__ptr,__num_bytes,__flags);
}
#endif /* !__OMIT_HEAP_TRACED_CONSTANT_P_WRAPPERS */

/* A convenience wrapper for `heap_alloc()', `heap_allat()' and
 * `heap_free()', implementing the following realloc()-style semantics:
 *   - realloc(ptr,0)                         --> Realloc to minimal size
 *   - realloc(ptr,malloc_usable_size(ptr)-x) --> Free unused memory at the end
 *   - realloc(ptr,malloc_usable_size(ptr)+x) --> Try to `heap_allat()' at the old end, or move to new memory
 *   - realloc(0,x)                           --> Same as `heap_alloc()'
 * NOTES:
 *   - Nothing these functions do can't easily be
 *     emulated using `heap_alloc()' and `heap_allat()'
 *     In fact: these functions are simple wrappers for them.
 *   - These functions always return a fully allocated heap data block.
 *   - Alignment arguments passed to `heap_realign()' are ignored
 *     unless the initial, or a new data block have to be allocated.
 *   - Allocation and free flags have been split to allow the user
 *     to request zero-initialized memory to be allocated for new
 *     allocations, while simultaniously releasing unused memory
 *     without indicating that it is zero-initialized.
 *   - If realloc() needs to move the data block to a new location,
 *     the old location is freed using `free_flags & ~GFP_CALLOC'
 * @assume(return.hp_siz >= new_bytes);
 * @param: old_ptr:      [valid_if(old_bytes != 0)] Base address of the old data block.
 * @param: old_bytes:     The old size of the data block (can be ZERO(0); must be aligned to `HEAP_ALIGNMENT' by the caller)
 * @param: alloc_flags:   Set of `GFP_*' flags used for allocating data.
 * @param: free_flags:    Set of `GFP_*' flags used for freeing data.
 * @param: new_bytes:     The new size of the data block (will be aligned by `HEAP_ALIGNMENT').
 * @throw: E_BADALLOC:    Failed to allocate memory.
 * @throw: E_WOULDBLOCK: `GFP_NOMAP' was specified and new memory would have had to be mapped.
 * @throw: E_WOULDBLOCK: `GFP_ATOMIC' was specified and a lock could not be acquired immediately. */
__FORCELOCAL struct heapptr
(__LIBCCALL heap_realloc)(struct heap *__restrict __self,
                          void *__old_ptr, __size_t __old_bytes,
                          __size_t __new_bytes, gfp_t __alloc_flags, gfp_t __free_flags) {
 if (__builtin_constant_p(__old_bytes)) {
  if (__old_bytes == 0)
      return heap_alloc(__self,__new_bytes,__alloc_flags);
 }
 if (__builtin_constant_p(__new_bytes))
     return __os_heap_realloc(__self,__old_ptr,__old_bytes,
                             (__new_bytes + (HEAP_ALIGNMENT-1)) &
                                           ~(HEAP_ALIGNMENT-1),
                              __alloc_flags,__free_flags);
 return __os_heap_realloc(__self,__old_ptr,__old_bytes,__new_bytes,
                          __alloc_flags,__free_flags);
}
__FORCELOCAL struct heapptr
(__LIBCCALL heap_realign)(struct heap *__restrict __self,
                          void *__old_ptr, __size_t __old_bytes,
                          __size_t __min_alignment, __ptrdiff_t __offset,
                          __size_t __new_bytes, gfp_t __alloc_flags, gfp_t __free_flags) {
 if (__builtin_constant_p(__old_bytes)) {
  if (__old_bytes == 0)
      return heap_align(__self,__min_alignment,__offset,__new_bytes,__alloc_flags);
 }
 if ((__builtin_constant_p(__min_alignment) && __min_alignment <= HEAP_ALIGNMENT) &&
     (__builtin_constant_p(__offset) && (__offset & (__min_alignment-1)) == 0))
      return heap_realloc(__self,__old_ptr,__old_bytes,__new_bytes,__alloc_flags,__free_flags);
 if (__builtin_constant_p(__new_bytes))
     return __os_heap_realign(__self,__old_ptr,__old_bytes,__min_alignment,
                              __offset,(__new_bytes + (HEAP_ALIGNMENT-1)) &
                                                     ~(HEAP_ALIGNMENT-1),
                              __alloc_flags,__free_flags);
 return __os_heap_realign(__self,__old_ptr,__old_bytes,__min_alignment,
                          __offset,__new_bytes,__alloc_flags,__free_flags);
}
#ifndef __OMIT_HEAP_TRACED_CONSTANT_P_WRAPPERS
__FORCELOCAL struct heapptr
(__LIBCCALL heap_realloc_untraced)(struct heap *__restrict __self,
                                   void *__old_ptr, __size_t __old_bytes,
                                   __size_t __new_bytes, gfp_t __alloc_flags, gfp_t __free_flags) {
 if (__builtin_constant_p(__old_bytes)) {
  if (__old_bytes == 0)
      return heap_alloc_untraced(__self,__new_bytes,__alloc_flags);
 }
 if (__builtin_constant_p(__new_bytes))
     return __os_heap_realloc_untraced(__self,__old_ptr,__old_bytes,
                                      (__new_bytes + (HEAP_ALIGNMENT-1)) &
                                                    ~(HEAP_ALIGNMENT-1),
                                       __alloc_flags,__free_flags);
 return __os_heap_realloc_untraced(__self,__old_ptr,__old_bytes,__new_bytes,
                                   __alloc_flags,__free_flags);
}
__FORCELOCAL struct heapptr
(__LIBCCALL heap_realign_untraced)(struct heap *__restrict __self,
                                   void *__old_ptr, __size_t __old_bytes,
                                   __size_t __min_alignment, __ptrdiff_t __offset,
                                   __size_t __new_bytes, gfp_t __alloc_flags, gfp_t __free_flags) {
 if (__builtin_constant_p(__old_bytes)) {
  if (__old_bytes == 0)
      return heap_align_untraced(__self,__min_alignment,__offset,__new_bytes,__alloc_flags);
 }
 if ((__builtin_constant_p(__min_alignment) && __min_alignment <= HEAP_ALIGNMENT) &&
     (__builtin_constant_p(__offset) && (__offset & (__min_alignment-1)) == 0))
      return heap_realloc_untraced(__self,__old_ptr,__old_bytes,__new_bytes,__alloc_flags,__free_flags);
 if (__builtin_constant_p(__new_bytes))
     return __os_heap_realign_untraced(__self,__old_ptr,__old_bytes,__min_alignment,
                                       __offset,(__new_bytes + (HEAP_ALIGNMENT-1)) &
                                                              ~(HEAP_ALIGNMENT-1),
                                       __alloc_flags,__free_flags);
 return __os_heap_realign_untraced(__self,__old_ptr,__old_bytes,__min_alignment,
                                   __offset,__new_bytes,__alloc_flags,__free_flags);
}
#endif /* !__OMIT_HEAP_TRACED_CONSTANT_P_WRAPPERS */
#endif /* !__OMIT_HEAP_CONSTANT_P_WRAPPERS */

/* Free the given memory, returning it to the heap.
 * The caller must ensure that `ptr' and `num_bytes' are aligned by
 * `HEAP_ALIGNMENT', and that `num_bytes' be least `HEAP_MINSIZE' bytes large.
 * The following flags affect the behavior of this function:
 *       - GFP_NOTRIM          -- Do not release large blocks of free data back to the core.
 *       - GFP_CALLOC          -- The given data block is ZERO-initialized.
 * @param: flags:     The flags that should be used when freeing data. (see above)
 * @param: ptr:       The HEAP_ALIGNMENT-aligned base pointer of the block to-be freed.
 * @param: num_bytes: The amount of bytes that should be freed.
 *                    NOTE: This argument must be aligned by `HEAP_ALIGNMENT',
 *                          and must not be equal to ZERO(0). */
__FORCELOCAL __ATTR_NOTHROW void
(__LIBCCALL heap_free)(struct heap *__restrict __self,
                       void *__ptr, __size_t __num_bytes, gfp_t __flags) {
 __os_heap_free(__self,__ptr,__num_bytes,__flags);
}
__FORCELOCAL __ATTR_NOTHROW void
(__LIBCCALL heap_free_untraced)(struct heap *__restrict __self,
                                void *__ptr, __size_t __num_bytes, gfp_t __flags) {
 __os_heap_free_untraced(__self,__ptr,__num_bytes,__flags);
}


/* Truncate the given heap, releasing unmapping free memory chunks
 * that are greater than, or equal to `CEIL_ALIGN(threshold,PAGESIZE)'
 * @return: * : The total number of bytes released back to the core (a multiple of PAGESIZE) */
__FORCELOCAL __size_t
(__LIBCCALL heap_trim)(struct heap *__restrict __self, __size_t __threshold) {
 return __os_heap_trim(__self,__threshold);
}


/* Collect various statistical information about
 * the given heap, packed together as a snapshot. */
__FORCELOCAL struct heapinfo
(__LIBCCALL heap_info)(struct heap *__restrict __self) {
 return __os_heap_info(__self);
}


#ifndef __pfindleakscallback_defined
#define __pfindleakscallback_defined 1
/* User-defined callback to-be invoked for detected memory leaks.
 * @param: BASE:       The base address of the memory leak.
 * @param: NUM_BYTES:  The number of consecutively leaked bytes.
 * @param: LEAKER_TID: The thread id (`gettid()') of the thread responsible for the allocation.
 *                     WARNING: That thread might not actually exist any more...
 * @param: FRAME_CNT:  The number of valid instruction pointers found in `FRAME_IP'
 * @param: FRAME_IP:   A vector enumerating the top-most `FRAME_CNT' instruction
 *                     pointers forming a traceback where the allocation was made.
 *                     HINT: You can easily convert these to file+line
 *                           using the `vinfo' printf extension:
 *                        >> printf("%[vinfo:%f(%l,%c) : %n : %p : Here\n]",IP);
 * @param: CLOSURE:    The CLOSURE-argument passed to `heap_find_leaks()'
 * @return: * :        Added to the sum of return values
 *                     eventually returned by `heap_find_leaks()'.
 * @return: < 0:       Stop enumeration and have `heap_find_leaks()' re-return
 *                     the same negative return value, indicating an error. */
typedef __ssize_t (__LIBCCALL *pfindleakscallback)(void *__base, __size_t __num_bytes,
                                                   __pid_t __leaker_tid, __size_t __frame_cnt,
                                                   void *const __frame_ip[], void *__closure);
#endif /* !__pfindleakscallback_defined */

#ifdef CONFIG_DEBUG_HEAP
/* Validate the memory of the given heap for
 * consistency, checking for invalid use-after-free. */
__LIBC void (__LIBCCALL heap_validate)(struct heap *__restrict __self);
__LIBC void (__LIBCCALL heap_validate_all)(void);

/* Begin/end tracing heap memory blocks:
 *  - Only memory that is traced can be walked when searching for memory leaks.
 *  - Memory traced as `heap_trace_leakless()' will not show up as memory leaks.
 *  - Memory traced as `heap_trace_root()' is used as a GC root location
 *    when searching for memory leaks.
 *  - Call `heap_untrace()' with any (non-inclusive) address
 *    of a traced data block to untrace that memory block.
 *  - The following memory locations are implicitly used as GC tracing points:
 *     - .data
 *     - .bss
 *     - The active portion of the stack of any thread running in the current VM
 *  - Additionally, any pointer contained in a general purpose register
 *    of any of the previously mentioned threads also counts as reachable.
 * -> The idea if this memory leak detector is not to 100% be able to discover
 *    _all_ memory leaks there are, but rather find allocated memory blocks
 *    that are no longer reachable:
 * >> void foo() {
 * >>     void *p = malloc(42);
 * >>     heap_dump_leaks(); // Nothing
 * >> }
 * >> void bar() {
 * >>     foo();
 * >>     heap_dump_leaks(); // Now it's a leak! (and will be dumped before this call returns)
 * >> }
 * An extremely similar mechanism is also used to detect memory leaks
 * in the kernel, but I find it works just as well in user-space.
 * HINT: As far as tracing leaks goes: When `CONFIG_DEBUG_HEAP' is
 *       enabled, a traceback will be stored in a hidden data block
 *       alongside every heap allocation, which is then used by
 *       the kernel's `addr2line' interface to gain human-readable
 *       source information.
 * @param: NUM_BYTES:      The number of bytes to trace (no-op when ZERO(0))
 * @param: NUM_SKIPFRAMES: The number of frames to skip in tracebacks.
 *                         Passing ZERO(0) will have the first traceback
 *                         entry refer to the location where this function
 *                         was called from.
 *                WARNING: Traceback frames from FORCELOCAL/LOCAL
 *                         functions may be interpreted as a single
 *                         frame due to how FORCELOCAL operates.
 *                         If you wish to skip frames, don't include
 *                         inline function calls, maybe even make use
 *                         of the `ATTR_NOINLINE' function attribute.
 */
__LIBC void (__LIBCCALL heap_trace)(void *__base, __size_t __num_bytes, unsigned int __num_skipframes);
__LIBC void (__LIBCCALL heap_trace_root)(void *__base, __size_t __num_bytes);
__LIBC void (__LIBCCALL heap_trace_leakless)(void *__base, __size_t __num_bytes);
__LIBC __ATTR_NOTHROW void (__LIBCCALL heap_untrace)(void *__ptr);

/* Search for memory leaks and dump them to `stderr' in a human-readable format.
 * When searching for leaked data blocks, KOS will dereference
 * tracked heap-pointers that are referenced in some way by any
 * active component of the associated application's VM:
 *   - Pointers found in general purpose registers of
 *     any thread running within the current VM.
 *   - Pointers found in the used part of the user-space
 *     stack of any running thread
 *   - Pointers found in writable global variables (.data & .bss)
 *   - Pointers found in memory regions traced using `heap_trace_root()'
 * The definition of a pointer is as follows:
 *   - Points into the user-space data segment
 *   - Dereferencable (memory is mapped where it points to)
 *   - Is aligned by at least `sizeof(void *)'
 * The definition of a heap-pointer is as follows:
 *   - Points into any heap_trace()-ed data block
 * Reachable data blocks are then searched recursively and
 * every blocks that could be reached is marked specially.
 * Every block that couldn't be reached is then considered a leak.
 * With that in mind, this function may be called at any point
 * during the execution of an application, as it is not bound by
 * requiring the caller to perform a proper shutdown alongside
 * cleanup in order to prevent any allocated pointer from being
 * dumped. */
__LIBC __size_t (__LIBCCALL heap_dump_leaks)(void);

/* High-level implementation for enumerating memory leaks:
 * Instead of dumping everything to `stderr', invoke `FUNC' for every leak detected.
 * @return: * : The sum of all calls to `FUNC', or the first negative value returned by it. */
__LIBC __ssize_t (__LIBCCALL heap_find_leaks)(pfindleakscallback __func, void *__closure);

#else
#define heap_validate(self)                 (void)0
#define heap_validate_all()                 (void)0
#define heap_trace(base,num_bytes)          (void)0
#define heap_trace_root(base,num_bytes)     (void)0
#define heap_trace_leakless(base,num_bytes) (void)0
#define heap_untrace(ptr)                   (void)0
#define heap_dump_leaks()                         0
#endif


#undef __HEAP_MEMBER

__SYSDECL_END

#endif /* !__KERNEL__ */

#endif /* !_KOS_HEAP_H */
