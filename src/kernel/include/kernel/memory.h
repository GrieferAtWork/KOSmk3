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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_MEMORY2_H
#define GUARD_KERNEL_INCLUDE_KERNEL_MEMORY2_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/paging.h>
#include <endian.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/memory.h>
#else
#error "Unsupported architecture"
#endif


DECL_BEGIN

#ifdef __CC__
typedef unsigned int mzone_t;

/* A page pointer is just like a normal physical pointer, but divided by PAGESIZE */
typedef PHYS vm_ppage_t pageptr_t;
#endif /* __CC__ */


#define MZONE_FSMALLANGE_COUNT   16
#ifdef __CC__
struct mzone {
    pageptr_t       mz_min;       /* [const][PAGE_ALIGNED] The lowest physical page apart of this zone (inclusive). */
    pageptr_t       mz_max;       /* [const][PAGE_ALIGNED-1] The greatest physical page apart of this zone (inclusive). */
    size_t          mz_used;      /* [lock(mz_lock)] Total number of pages currently in use. */
    size_t          mz_free;      /* [lock(mz_lock)] Total number of free pages. (1-bits in `mz_fpages') */
    u8              mz_fpages[1]; /* [((mz_max-mz_min)+1+7)/8+2*sizeof(void *)][lock(mz_lock)]
                                   *  Bitset of all free pages (1-bits are free pages; the last 4/8 bytes are always ZERO).
                                   *  NOTE: Each byte encodes 8 pages, the first at 0x01, the second at 0x02, the third at 0x04, etc... */
};

/* [1..1][mzone_count] Memory zone descriptors. */
DATDEF struct mzone *const mzones[MZONE_MAXCOUNT];
/* The amount of memory zones in use. */
DATDEF size_t const mzone_count;
#define MZONE_ANY  (mzone_count-1)
#endif /* __CC__ */


#ifdef __CC__
/* Allocate `num_pages' continuous pages of
 * physical memory and return their page number.
 * WARNING: Physical memory cannot be dereferenced prior to being mapped.
 * @throw E_BADALLOC: Not enough available physical memory.
 * @return: * : The page number of the newly allocated memory range.
 * HINT: This function is identical to `page_malloc_part(num_pages,num_pages,...,max_zone)' */
FUNDEF pageptr_t KCALL
page_malloc(size_t num_pages, mzone_t max_zone);

/* Try to allocate memory at a given address, or return `false' if this fails. */
FUNDEF bool KCALL page_malloc_at(pageptr_t ptr, size_t num_pages);

/* Allocate some number of pages, though at least `min_pages', but
 * not more than `max_pages', preferring to allocate from smaller
 * clusters of memory, though not moving on to lower-level zones
 * until the zone currently being searched turns out not to contain
 * a range of at least `min_pages' pages.
 * WARNING: Physical memory cannot be dereferenced prior to being mapped.
 * @param: res_pages: Filled with the allocated number of pages.
 * @throw E_BADALLOC: Not enough available physical memory.
 * @return: * : The page number of the newly allocated memory range. */
FUNDEF pageptr_t KCALL
page_malloc_part(size_t min_pages, size_t max_pages,
                 size_t *__restrict res_pages,
                 mzone_t max_zone);

/* Free a given physical address range.
 * The caller is responsible to ensure that the
 * given range has previously been allocated. */
FUNDEF void KCALL page_free(pageptr_t base, size_t num_pages);
#endif /* __CC__ */




/* Physical memory layout information. */

/* NOTE: The order of these is important, in that greater
 *       memory types override lower ones during re-definitions. */
#define MEMTYPE_NDEF       0 /* Undefined memory (Handled identically to `MEMTYPE_BADRAM') */
#define MEMTYPE_RAM        1 /* [USE] Dynamic; aka. RAM memory (Main source of memory for `page_malloc') */
#ifdef CONFIG_BUILDING_KERNEL_CORE
#define MEMTYPE_PRESERVE   2 /* [USE_LATER_0|MAP] Preserve the original content of this memory until `mem_unpreserve()' is
                              *  called, at which point the memory will be transformed into `MEMTYPE_RAM',
                              *  and made available to the physical memory allocator. */
#define MEMTYPE_ALLOCATED  3 /* [USE_LATER_1|MAP] Same as 'MEMTYPE_RAM', but don't make memory available to `page_malloc' */
#endif /* CONFIG_BUILDING_KERNEL_CORE */
#define MEMTYPE_KFREE      4 /* [USE_LATER_2|MAP] Kernel memory later turned into `MEMTYPE_RAM' (The `.free' section...) */
#define MEMTYPE_KERNEL     5 /* [MAP] Kernel core memory (mapped, not used) */
#define MEMTYPE_NVS        6 /* Non-volatile memory (that is: memory that doesn't get wiped after shutdown) */
#define MEMTYPE_DEVICE     7 /* Device memory (mapped, but not used as RAM) */
#define MEMTYPE_BADRAM     8 /* Broken memory (Neither mapped nor used, but known to be present) */
#define MEMTYPE_COUNT      9 /* Amount of known memory types. */

#ifdef __CC__
DATDEF char const memtype_names[MEMTYPE_COUNT][8];
typedef int memtype_t;
#endif /* __CC__ */

#define MEMTYPE_ISUSE(x) ((x) == MEMTYPE_RAM) /* Should this type of memory be used as generic RAM? */
#define MEMTYPE_ISMAP(x) ((x) > MEMTYPE_NDEF && (x) < MEMTYPE_BADRAM) /* Should this type of memory be mapped and be accessible. */

#ifdef __CC__
struct PACKED meminfo {
    union PACKED {
        memtype_t              mi_type; /* [const] Memory type (One of `MEMTYPE_*')
                                         * NOTE: Adjacent meminfo descriptors never feature the same type!
                                         *       If a situation arises where they do, they are split. */
        uintptr_t            __mi_pad0; /* ... */
    };
#if __SIZEOF_POINTER__ == 4
    u32                      __mi_pad1; /* ... */
#endif
#if __SIZEOF_POINTER__ == 8
    u64                        mi_addr;   /* [const] First associated address.
                                           * NOTE: This pointer is _NOT_ necessarily page-aligned! */
#else
    union PACKED {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        u32                    mi_addr32; /* [const] 32-bit starting address. */
#else
        struct PACKED {
            u32              __mi_pad2;   /* ... */
            u32                mi_addr32; /* [const] 32-bit starting address. */
        };
#endif
        u64                    mi_addr;   /* [const] First associated address.
                                           * NOTE: This pointer is _NOT_ necessarily page-aligned! */
    };
#endif
};

#define MEMINFO_BEGIN(x) ((x)->mi_addr)
#define MEMINFO_END(x)   (((x) != __mem_info_last ? ((x)[1].mi_addr) : 0))
#define MEMINFO_MIN(x)   ((x)->mi_addr)
#define MEMINFO_MAX(x)   (MEMINFO_END(x)-1)
#define MEMINFO_SIZE(x)  (MEMINFO_END(x)-MEMINFO_BEGIN(x))

/* Physical memory information.
 * Information is stored as a vector sorted ascendingly by starting
 * address, with each entry extending until the next, and the last one
 * extending into infinity (Or until the end of the addressable memory). */
DATDEF size_t         const        mem_info_c; /* [!0] */
DATDEF struct meminfo const *const mem_info_v;
DATDEF struct meminfo const *const __mem_info_last ASMNAME("mem_info_last"); /* == mem_info_v+mem_info_c-1 */
#define MEMINFO_FOREACH(iter) \
 for ((iter) = mem_info_v; \
      (iter) <= __mem_info_last; ++(iter))

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Helper functions for installing the initial memory map. */
INTDEF INITCALL void KCALL mem_install(PHYS u64 base_address, u64 num_bytes, memtype_t type);

/* Release all memory regions marked as `MEMTYPE_PRESERVE'.
 * HINT: This function can be called multiple times. */
INTDEF INITCALL void KCALL mem_unpreserve(void);

/* Relocate `mem_info' into permanent storage allocated
 * within swappable, virtual shared memory. */
INTDEF INITCALL void KCALL mem_relocate_info(void);

#endif /* CONFIG_BUILDING_KERNEL_CORE */
#endif /* __CC__ */


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_MEMORY2_H */
