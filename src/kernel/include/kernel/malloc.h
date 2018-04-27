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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_MALLOC_H
#define GUARD_KERNEL_INCLUDE_KERNEL_MALLOC_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>

DECL_BEGIN

//#define CONFIG_NO_DEBUG_HEAP             1
//#define CONFIG_NO_HEAP_RANDOMIZE_OFFSETS 1
//#define CONFIG_NO_HEAP_TRACE_DANGLE      1
//#define CONFIG_NO_DEBUG_MALLOC           1



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


/* Trace dangling heap data in order to minimize unnecessary allocations:
 * Since the introduction of `CONFIG_ATOMIC_HEAP' and it having become
 * mandatory shortly after, one problem arose that could potentially result
 * in unnecessary and excessive core allocations when there is still enough
 * heap memory available:
 *   - Say you want to allocate 64 bytes of memory. However
 *     the nearest free data block has a size of 8Mib.
 *     Since the heap can't know how much of that data block
 *     has already been faulted, it doesn't want to run the
 *     risk of keeping a lock to the heap while splitting that
 *     data block in 2, potentially causing a #PF and keeping
 *     a heap lock for an eternity (#PF still can happen regardless,
 *     but keeping them to a minimum is the best way to ensure heaps
 *     operating as lock-less as possible)
 *   - With that in mind, what `heap_alloc()' must do is allocate the entirety
 *     of that data block before releasing any portion that isn't actually
 *     being used (this is also where `CONFIG_HEAP_RANDOMIZE_OFFSETS' into
 *     play, as it chooses a random position in that larger data block to
 *     return as newly allocated memory, rather than always returning its
 *     starting address, which while be faster as at most one remaining data
 *     block will have to released to the heap following the split, degrades
 *     resilience to faulty code; see above...)
 *   - Yet before your thread has finished splitting the 8Mib data
 *     block, another thread wants to allocate more memory, too.
 *     Since there are no other data blocks sufficient for what it
 *     needs, it would have to request more pages from the core,
 *     despite the fact that ~7.995Mib are about to become free again.
 *  -> The TRACE_DANGLE heap configuration option fixes this problem
 *     by introducing a mechanism for tracing dangling heap allocation
 *     in the form of the `h_dangle' dangle field of heap structures
 *     containing information about those ~7.995Mib and allowing a
 *     thread that wants to allocate less than that to wait for the
 *     first thread to release that data, rather than immediately
 *     inquire the core for more memory.
 * NOTE: The situation described above is quite rare, because of which
 *       it could be argued that this option slows down heap allocations
 *       rather than speeding them up.
 *       However, since there really isn't a true upper limit to how large
 *       such ~dangling~ blocks of memory can grow, having this turned on
 *       counteracts heap fragmentation and unnecessary restrictions to
 *       system resources in situations where large blocks of memory are
 *       concurrently being freed at the same time other large blocks are
 *       being allocated in other threads.
 *      (Such a situation could arise from excessive use of `fork()' and
 *       the consequentially required duplication of the caller's VM needing
 *       at least a couple of pages of virtual memory at once) */
#ifndef CONFIG_HEAP_TRACE_DANGLE
#ifndef CONFIG_NO_HEAP_TRACE_DANGLE
#define CONFIG_HEAP_TRACE_DANGLE 1
#endif
#elif defined(CONFIG_NO_HEAP_TRACE_DANGLE)
#undef CONFIG_HEAP_TRACE_DANGLE
#endif




/* Configuration option to enable/disable a debug functionality
 * of `kmalloc()', as well as make it much more robust than it
 * would otherwise be:
 *   - Enable GC-style tracing of reachable memory blocks, including the ability
 *     to dump the kernel's dynamic memory leaks using `mall_dump_leaks()', or
 *     alternatively the user-space kernel control command `KERNEL_CONTROL_DBG_DUMP_LEAKS'
 *   - Disconnect the header containing the data block's size and allocation heap
 *     from the data block itself, instead storing that information elsewhere
 *     in order to prevent it from being corrupted as the reuslt of invalid use.
 *   - Add a small header and tail data block surrounding every allocation
 *     that can be verified for modifications to detect array overruns, or
 *     data block underflows.
 *     Validation of the header and tail is automatically performed for a
 *     heap data block whenever it is passed to one of `kfree()',
 *    `krealloc()' (and friends) or `kmalloc_usable_size()'
 *     Alternatively, `mall_validate_padding()' or the user-space kernel
 *     control command `KERNEL_CONTROL_DBG_CHECK_PADDING' can be used to
 *     verify this ~padding~ for all allocated data blocks. */
#ifndef CONFIG_DEBUG_MALLOC
#if !defined(CONFIG_NO_DEBUG_MALLOC) && !defined(NDEBUG)
#define CONFIG_DEBUG_MALLOC 1
#elif !defined(CONFIG_NO_DEBUG_MALLOC)
#define CONFIG_NO_DEBUG_MALLOC 1
#endif
#elif defined(CONFIG_NO_DEBUG_MALLOC)
#undef CONFIG_DEBUG_MALLOC
#endif




/* Minimum alignment of all heap pointers. */
#ifndef HEAP_ALIGNMENT
#ifdef CONFIG_HEAP_ALIGNMENT
#   define HEAP_ALIGNMENT  CONFIG_HEAP_ALIGNMENT
#elif __SIZEOF_POINTER__ == 4 && !defined(CONFIG_DEBUG_HEAP)
#   define HEAP_ALIGNMENT  8
#else
/* Using 16 allows a human to quickly notice heap pointers by realizing
 * that the last digit in a hexadecimal representation is ZERO(0). */
#   define HEAP_ALIGNMENT 16
#endif
#endif
#if (HEAP_ALIGNMENT & (HEAP_ALIGNMENT-1))
#error "Invalid `HEAP_ALIGNMENT' must be a power-of-2 number"
#endif



#define GFP_NORMAL 0x0000 /* Normal allocation of memory only accessible from
                           * ring#0, but shared between all page directories.
                           *(In fact, this is the same as `GFP_SHARED') */
#define GFP_SHARED 0x0000 /* Allocate memory shared between all page directories. (_ALWAYS_ ZERO(0))
                           * NOTE: All memory allocated by this is located within `addr_ishost()'.
                           * NOTE: Despite being mapped in all page directories,
                           *       only the kernel can access this type of memory.
                           * WARNING: Unless stated otherwise, _ALL_ dynamically allocated
                           *          structures must be allocated as shared (that is they
                           *          are accessible for all page directories) */
#define GFP_LOCKED 0x0001 /* Allocate in-core-locked virtual memory.
                           * NOTE: May only or'd together with `GFP_KERNEL' and `GFP_SHARED'
                           * NOTE: To prevent #PF recursion, this flag must be set when allocating
                           *       `vm_part', `vm_region', `vm_node' or `vm' structures!
                           * NOTE: This flag prevents loadcore() from being invoked during
                           *       allocation of memory, allocating memory that is suitable
                           *       for use during loadcore() itself. */
#define GFP_KERNEL 0x0002 /* Allocate virtual memory only visible to the kernel
                           * WARNING: When this bit is set, the calling thread must be a
                           *          kernel worker thread (aka. `THIS_VM == &vm_kernel') */
#define __GFP_HEAPCOUNT 4 /* Amount of different heaps used by the kernel (Use the above macros to address them). */
#define __GFP_HEAPMASK  3 /* Mask for the heap ID. */


#define GFP_CALLOC   0x0010 /* Zero-initialize newly allocated memory. */
/*      GFP_......   0x00e0  * Unused memory attribute flags. */
#define GFP_NOSWAP   0x0200 /* Don't initialize any kind of I/O during swap.
                             * NOTE: `GFP_NOFS' only prevents write-mapped files from
                             *        being synced, while this flag is required to
                             *        prevents any use of a potential swap partition. */
#define GFP_NOFS     0x0400 /* Don't sync + unload write-mapped files to free up core memory. */
#define GFP_ATOMIC   0x0800 /* [== IO_NONBLOCK] Do not block, but throw a resumable `E_WOULDBLOCK' error when a lock could not be acquired immediately. */
#define GFP_NOOVER   0x1000 /* Do not overallocate when mapping new memory.
                             * Any (logical) call to `vm_map()' and friends will be limited
                             * to `CEIL_ALIGN(n_bytes + INTERNAL_OVERHEAD,PAGESIZE)', where
                             * `INTERNAL_OVERHEAD' can be assumed to be constant-length, and
                             * be at most `CEIL_ALIGN(512,HEAP_ALIGNMENT)' bytes (that number
                             * I just came up with. In actuality the overhead is much smaller,
                             * but you shouldn't rely on that fact, meaning I'm allowed to
                             * make it larger if I want to... Though not larger than 512
                             * bytes per allocation)
                             * NOTE: When directly used with `heap_*' functions, allocations
                             *       are restricted to ``CEIL_ALIGN(n_bytes,PAGESIZE)' */
#define GFP_NOMOVE   0x2000 /* For `krealloc' & `krealign': Use `realloc_in_place()' semantics.
                             * NOTE: This is flag is set and `NULL' is passed for `ptr', the return value is always `NULL' */
#define GFP_NOMAP    0x4000 /* Do not map new memory, but throw an `E_WOULDBLOCK' error when caches do not contain any memory.
                             * Additionally, do not unmap system memory when this flag is set during a free-operation (including realloc when reducing a pointer size) */
#define GFP_NOTRIM   0x4000 /* Don't unmap memory once free blocks grow to sufficient lengths. */
#define GFP_INCORE   0x8000 /* Allocate all memory directly. - Don't use allocate-on-access.
                             * WARNING: Unless the caller is either read, or write-locking
                             *         `mman_kernel.m_lock', memory may have already been
                             *          swapped by the time `kmalloc' and friends return. */

#define GFP_NOIO     (GFP_NOSWAP|GFP_NOFS) /* Don't make use of any kind of I/O when trying to free up available memory. */
#define GFP_NOFREE   (GFP_NOSWAP|GFP_NOFS) /* Don't take any actions to free up available memory. */


#ifdef __CC__
#ifndef __gfp_t_defined
#define __gfp_t_defined 1
typedef unsigned int gfp_t; /* Set of `GFP_*' */
#endif /* !__gfp_t_defined */

/* Allocate/Reallocate/Free virtual memory kernel memory.
 * @throw: E_BADALLOC: Not enough available memory. */
FUNDEF WUNUSED ATTR_MALLOC ATTR_RETNONNULL VIRT void *(KCALL __os_malloc)(size_t n_bytes, gfp_t flags) ASMNAME("kmalloc");
FUNDEF WUNUSED ATTR_MALLOC ATTR_RETNONNULL VIRT void *(KCALL __os_memalign)(size_t min_alignment, size_t n_bytes, gfp_t flags) ASMNAME("kmemalign");
FUNDEF WUNUSED ATTR_MALLOC ATTR_RETNONNULL VIRT void *(KCALL __os_memalign_offset)(size_t min_alignment, ptrdiff_t offset, size_t n_bytes, gfp_t flags) ASMNAME("kmemalign_offset");
/* NOTE: `krealloc' / `krealign' both return `NULL' when `GFP_NOMOVE' is set and the reallocation
 *        is not possible due to an overlap (when `n_bytes' is larger than the old size). */
FUNDEF VIRT void *(KCALL __os_realloc)(VIRT void *ptr, size_t n_bytes, gfp_t flags) ASMNAME("krealloc");
FUNDEF VIRT void *(KCALL __os_realign)(VIRT void *ptr, size_t min_alignment, size_t n_bytes, gfp_t flags) ASMNAME("krealign");
FUNDEF VIRT void *(KCALL __os_realign_offset)(VIRT void *ptr, size_t min_alignment, ptrdiff_t offset, size_t n_bytes, gfp_t flags) ASMNAME("krealign_offset");
FUNDEF ATTR_NOTHROW WUNUSED size_t (KCALL __os_malloc_usable_size)(VIRT void *ptr) ASMNAME("kmalloc_usable_size");
/* NOTE: `kffree' is NOTHROW unless `GFP_ATOMIC' is given. */
FUNDEF ATTR_NOTHROW void (KCALL __os_free)(VIRT void *ptr) ASMNAME("kfree");
FUNDEF void (KCALL __os_ffree)(VIRT void *ptr, gfp_t flags) ASMNAME("kffree");


#ifndef __OMIT_KMALLOC_CONSTANT_P_WRAPPERS
#define kcalloc(n_bytes,flags)                 kmalloc(n_bytes,(flags)|GFP_CALLOC)
#define krecalign(ptr,min_alignment,n_bytes,flags) krealign(ptr,min_alignment,n_bytes,(flags)|GFP_CALLOC)
#define krecalloc(ptr,n_bytes,flags)           krealloc(ptr,n_bytes,(flags)|GFP_CALLOC)
#define krealloc_in_place(ptr,n_bytes,flags)   krealloc(ptr,n_bytes,(flags)|GFP_NOMOVE)
#define krecalloc_in_place(ptr,n_bytes,flags)  krealloc(ptr,n_bytes,(flags)|GFP_NOMOVE|GFP_CALLOC)

FORCELOCAL WUNUSED ATTR_MALLOC ATTR_RETNONNULL
VIRT void *(KCALL kmalloc)(size_t n_bytes, gfp_t flags) {
 if (__builtin_constant_p(n_bytes)) /* Pre-align `n_bytes' so that heap functions don't need to. */
     return __os_malloc((n_bytes+HEAP_ALIGNMENT-1) & ~(HEAP_ALIGNMENT-1),flags);
 return __os_malloc(n_bytes,flags);
}
FORCELOCAL WUNUSED ATTR_MALLOC ATTR_RETNONNULL
VIRT void *(KCALL kmemalign)(size_t min_alignment,
                             size_t n_bytes, gfp_t flags) {
 if (__builtin_constant_p(min_alignment) && min_alignment <= HEAP_ALIGNMENT)
     return kmalloc(n_bytes,flags);
 return __os_memalign(min_alignment,n_bytes,flags);
}
FORCELOCAL WUNUSED ATTR_MALLOC ATTR_RETNONNULL
VIRT void *(KCALL kmemalign_offset)(size_t min_alignment, ptrdiff_t offset,
                                    size_t n_bytes, gfp_t flags) {
 if (__builtin_constant_p(offset) && offset == 0)
     return kmemalign(min_alignment,n_bytes,flags);
 return __os_memalign_offset(min_alignment,offset,n_bytes,flags);
}

/* NOTE: `krealloc' / `krealign' both return `NULL' when `GFP_NOMOVE' is set and the reallocation
 *        is not possible due to an overlap (when `n_bytes' is larger than the old size). */
FORCELOCAL VIRT void *(KCALL krealloc)(VIRT void *ptr, size_t n_bytes, gfp_t flags) {
 if (__builtin_constant_p(ptr) && ptr == NULL)
     return kmalloc(n_bytes,flags);
 if (__builtin_constant_p(n_bytes))
     return __os_realloc(ptr,(n_bytes+HEAP_ALIGNMENT-1) & ~(HEAP_ALIGNMENT-1),flags);
 return __os_realloc(ptr,n_bytes,flags);
}
FORCELOCAL VIRT void *(KCALL krealign)(VIRT void *ptr, size_t min_alignment,
                                       size_t n_bytes, gfp_t flags) {
 if (__builtin_constant_p(min_alignment) && min_alignment <= HEAP_ALIGNMENT)
     return krealloc(ptr,n_bytes,flags);
 if (__builtin_constant_p(ptr) && ptr == NULL)
     return kmemalign(min_alignment,n_bytes,flags);
 if (__builtin_constant_p(n_bytes))
     return __os_realign(ptr,min_alignment,(n_bytes+HEAP_ALIGNMENT-1) & ~(HEAP_ALIGNMENT-1),flags);
 return __os_realign(ptr,min_alignment,n_bytes,flags);
}
FORCELOCAL VIRT void *(KCALL krealign_offset)(VIRT void *ptr, size_t min_alignment,
                                              ptrdiff_t offset, size_t n_bytes, gfp_t flags) {
 if (__builtin_constant_p(offset) && offset == 0)
     return krealign(ptr,min_alignment,n_bytes,flags);
 if (__builtin_constant_p(ptr) && ptr == NULL)
     return kmemalign_offset(min_alignment,offset,n_bytes,flags);
 if (__builtin_constant_p(n_bytes))
     return __os_realign_offset(ptr,min_alignment,offset,(n_bytes+HEAP_ALIGNMENT-1) & ~(HEAP_ALIGNMENT-1),flags);
 return __os_realign_offset(ptr,min_alignment,offset,n_bytes,flags);
}

FORCELOCAL ATTR_NOTHROW WUNUSED
size_t (KCALL kmalloc_usable_size)(VIRT void *ptr) {
 if (__builtin_constant_p(ptr) && ptr == NULL)
     return 0;
 return __os_malloc_usable_size(ptr);
}

/* NOTE: `kffree' is NOTHROW unless `GFP_ATOMIC' is given. */
FORCELOCAL ATTR_NOTHROW void (KCALL kfree)(VIRT void *ptr) {
 if (!__builtin_constant_p(ptr) || ptr != NULL)
      __os_free(ptr);
}
FORCELOCAL void (KCALL kffree)(VIRT void *ptr, gfp_t flags) {
 if (__builtin_constant_p(flags) && !(flags & (GFP_CALLOC|GFP_ATOMIC))) {
  kfree(ptr);
 } else {
  __os_ffree(ptr,flags);
 }
}
#endif /* !__OMIT_KMALLOC_CONSTANT_P_WRAPPERS */

#endif /* __CC__ */

#ifdef CONFIG_DEBUG_MALLOC
#ifdef __CC__
/* Search for and dump kernel memory leaks,
 * returning the number of encountered leaks. */
FUNDEF size_t KCALL mall_dump_leaks(gfp_t heap_max);

/* Validate padding surrounding heap pointers to
 * check if memory has been written past allocated
 * end addresses. */
FUNDEF void KCALL mall_validate_padding(gfp_t heap_max);

/* Add/Remove a given address range as a valid tracing junction:
 *   - Tracing junctions are blocks of memory which may contain
 *     points to other tracing junction, or regular mall-pointers.
 *   - When searching for leaked data blocks, KOS will dereference
 *     mall-pointers that are referenced in some way by any active
 *     component of the operating system:
 *       - Pointers found in general purpose registers of any
 *         running thread (if preempted while in the kernel)
 *       - Pointers found in the used part of the kernel stack
 *         of any running thread
 *       - Pointers found in writable global variables (.data & .bss)
 *     The definition of a pointer is as follows:
 *       - Points into the kernel-share segment (only required for `GFP_SHARED' heap pointers)
 *       - Dereferencable (memory is mapped where it points to)
 *       - Is aligned by at least `sizeof(void *)'
 *     The definition of a mall-pointer is as follows:
 *       - Points into the usable memory portion of any kmalloc()-allocated
 *         data block (including leading and trailing padding data used to
 *         verify write-past-the-end errors using `mall_validate_padding()')
 *   - This is all well and good, and the kernel can easily detect
 *     leaked memory by recursively traversing all allocated memory
 *     in search of pointers to other allocated memory blocks.
 *     The fact that memory is pre-initialized to a consistent
 *     debug state when `CONFIG_DEBUG_HEAP' is enabled makes this
 *     even easier.
 *     However the problem comes when code uses custom allocators
 *     that make use of the `heap_alloc()' API.
 *     Since the mall API is built ontop of that API, there is no
 *     connection that points back to the heap API and would allow
 *     the MALL GC checker to differentiate between some random data
 *     word, and a pointer that is directed into a data block of some
 *     specific length, which in itself is allowed to hold pointers to
 *     other data blocks, some of which are allowed to be other mall
 *     blocks.
 *     However despite the fact that it might seem obvious to the programmer
 *     that a data block allocated using `heap_alloc_u()' should be able to
 *     hold pointers to data blocks allocated using `kmalloc()', this is
 *     not the case (out of the box), because again: the MALL GC checker
 *     would not be able to know what (if any) memory pointed to by that
 *     data block can carry GC-able pointers.
 *   - The solution is to use this function pair to inform mall about
 *     some custom data block that can be used during traversable of
 *     reachable memory.
 *     If itself reachable (through the means described above), that data
 *     block will be analyzed just like any other data block allocated
 *     using `kmalloc()' would have been.
 *     Additionally, if not reachable during MALL GC search, the data
 *     block will be dumped as a leak alongside a traceback to the
 *     call to `mall_trace()' that registered that data block.
 *     An exception to this is to use `mall_trace_leakless()', which
 *     allows the data block to act as a proxy during GC search, yet
 *     doesn't have it count as a leak if it in fact isn't reachable.
 *   - Be sure to use `mall_untrace()' to delete the data block once
 *     it should no longer act as a proxy (usually just before a call
 *     to `heap_free_untraced()'), by passing the address of any pointer that
 *     is considered to be part of that block `>= base && < base+num_bytes'.
 *     Usually, you'll just be passing `base', as `heap_free_untraced()' will
 *     be needing that same pointer from you.
 *    (Or just use `heap_free()' to have that be done automatically for you)
 */
FUNDEF void KCALL mall_trace(void *base, size_t num_bytes);
FUNDEF void KCALL mall_trace_leakless(void *base, size_t num_bytes);
FUNDEF ATTR_NOTHROW void KCALL mall_untrace(void *ptr);
FUNDEF void KCALL mall_print_traceback(void *ptr);


#endif
#   define ATTR_MALL_UNTRACKED  ATTR_SECTION(".bss.mall.untracked")
#else
#ifdef __CC__
#define mall_dump_leaks(heap_max)                 0
#define mall_validate_padding(heap_max)     (void)0
#define mall_trace(base,num_bytes)          (void)0
#define mall_trace_leakless(base,num_bytes) (void)0
#define mall_untrace(ptr)                   (void)0
#endif
#   define ATTR_MALL_UNTRACKED  ATTR_SECTION(".bss")
#endif

DECL_END

#ifdef __USE_KOS
#ifdef _ALLOCA_H
#include <parts/kos2/malloca.h>
#define malloca(s) __malloca(s)
#define calloca(s) __calloca(s)
#define freea(p)   __freea(p)
#endif
#endif /* __USE_KOS */

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_MALLOC_H */
