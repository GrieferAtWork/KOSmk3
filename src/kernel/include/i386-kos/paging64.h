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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_PAGING64_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_PAGING64_H 1

#include <hybrid/compiler.h>
#include <hybrid/typecore.h>
#include <kos/types.h>
#include <stdbool.h>

DECL_BEGIN

#undef CONFIG_NO_GIGABYTE_PAGES
#if 0
#define CONFIG_NO_GIGABYTE_PAGES 1
#endif

#undef CONFIG_NO_NX_PAGES
#if 0
#define CONFIG_NO_NX_PAGES 1
#endif


#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Access a physical memory pointer during early boot. */
#define X86_EARLY_PHYS2VIRT(pptr) ((__typeof__(pptr))((uintptr_t)(pptr) + KERNEL_CORE_BASE))
#define X86_EARLY_VIRT2PHYS(pptr) ((__typeof__(pptr))((uintptr_t)(pptr) - KERNEL_CORE_BASE))
#endif


#define VM_VPAGE_SIZE  8
#define VM_PPAGE_SIZE  8
#define VM_VIRT_SIZE   8
#define VM_PHYS_SIZE   8
#ifdef __CC__
typedef VIRT u64 vm_vpage_t; /* Virtual memory page index (`virtual_address / PAGESIZE'). */
typedef PHYS u64 vm_ppage_t; /* Physical memory page index (`physical_address / PAGESIZE'). */
typedef VIRT u64 vm_virt_t;  /* A virtual memory pointer. */
typedef PHYS u64 vm_phys_t;  /* A physical memory pointer. */
#endif /* __CC__ */


#define VM_ADDRBITS        48                             /* The number of usable bits in an address. */
#define VM_ADDRMASK        __UINT64_C(0x0000ffffffffffff) /* Mask of all address bits that can actually be used.
                                                           * NOTE: On x86_64, this is 48 bits. */
#define VM_VPAGE_MAX       __UINT64_C(0xfffffffff)        /* Max VPAGE (48 - 12bits == 36 bits) */
#define VM_PPAGE_MAX       __UINT64_C(0xffffffffff)       /* This matches what is allowed by `X86_PAGE_FADDR' */
#define KERNEL_BASE_PAGE   __UINT64_C(0x800000000)        /* The page index of the start of kernel memory. */
#define KERNEL_NUM_PAGES   __UINT64_C(0x800000000)        /* The number of pages associated with the kernel. */

/* The range of non-canonical virtual addresses. */
#define X86_PAGING_NONCANON_MIN  __UINT64_C(0x0001000000000000)
#define X86_PAGING_NONCANON_MAX  __UINT64_C(0xffff7fffffffffff)
#define X86_PAGING_ISNONCANON(x) ((x) >= X86_PAGING_NONCANON_MIN && (x) <= X86_PAGING_NONCANON_MAX)


/* Convert an address into a page number.
 * NOTE: This function masks out address sign bits, meaning that
 *       on x86_64, only the bottom 48 bits are actually used. */
#define VM_ADDR2PAGE(x)  (__CCAST(vm_vpage_t)((__CCAST(uintptr_t)(x) & VM_ADDRMASK) >> 12))

/* Convert a page number into its virtual base address.
 * NOTE: This function takes sign extension into account, meaning
 *       that page numbers `>= 0x800000000' will produce sign-extended
 *       memory addresses. */
#ifdef __CC__
#define VM_PAGE2ADDR(x)  ((uintptr_t)(((intptr_t)((x) << 28)) >> 16))
#else
#define VM_PAGE2ADDR(x)  ((0xffff800000000000 * (((x) & 0x800000000) >> 35)) + \
                                                (((x) & 0x7ffffffff) << 12))
#endif


/* Configure to indicate that the kernel is a high-kernel. */
#undef CONFIG_LOW_KERNEL
#undef CONFIG_HIGH_KERNEL
#define CONFIG_HIGH_KERNEL 1

/* KOS resides in the upper (signed) half of virtual memory.
 * NOTE: This macro is only defined when `CONFIG_HIGH_KERNEL' is set.
 * NOTE: The upper 16 bits are ignored during memory mapping operations. */
#define KERNEL_BASE       __UINT64_C(0xffff800000000000)

/* Base address of where to load the kernel core into memory.
 * HINT: This address is located at -2GB */
#define KERNEL_CORE_BASE  __UINT64_C(0xffffffff80000000)
#define KERNEL_CORE_BASE_ASM         0xffffffff80000000
#define KERNEL_CORE_SIZE  __UINT64_C(0x0000000080000000)

#define ADDR_ISUSER(x)  ((__CCAST(uintptr_t)(x) & VM_ADDRMASK) < (KERNEL_BASE & VM_ADDRMASK))
#define ADDR_ISHOST(x)  ((__CCAST(uintptr_t)(x) & VM_ADDRMASK) >= (KERNEL_BASE & VM_ADDRMASK))


#define X86_VM_KERNEL_PDIR_IDENTITY_BASE     __UINT64_C(0xffff808000000000) /* KERNEL_BASE + 512GiB */
#define X86_VM_KERNEL_PDIR_IDENTITY_SIZE     __UINT64_C(0x0000008000000000) /* 512GiB */
#define X86_VM_KERNEL_PDIR_RESERVED_BASE     __UINT64_C(0xffff808000000000) /* Start of the address range reserved for page-directory self-modifications. */
#define X86_VM_KERNEL_PDIR_RESERVED_SIZE     __UINT64_C(0x0000008000000000) /* Amount of memory reserved for page-directory self-modifications. */

#define X86_VM_KERNEL_KERNELHEAP_LOCKED_BASE __UINT64_C(0xffff9fffff800000)
#define X86_VM_KERNEL_KERNELHEAP_TAIL        __UINT64_C(0xffff800000000000)
#define X86_VM_KERNEL_SHAREDHEAP_BASE        __UINT64_C(0xffffa000a1200000)
#define X86_VM_KERNEL_SHAREDHEAP_LOCKED_TAIL __UINT64_C(0xffffa000a1a00000)
#define X86_VM_KERNEL_TEMPPAGE_BASE          __UINT64_C(0xffffa000af100000) /* Hint for where to allocate thread-local temporary pages. */
#define X86_VM_KERNEL_MALLHEAP_TAIL          __UINT64_C(0xffffa000a0000000)
#define X86_VM_KERNEL_DRIVER_BASE            __UINT64_C(0xffffa000a0000000)
#define X86_VM_KERNEL_DEBUG_BASE             __UINT64_C(0xffffa000aa000000)
#define X86_VM_KERNEL_COREBASE_TAIL          __UINT64_C(0xffffa000b0000000) /* Hint for where to allocate core-base pointers. */
#define X86_VM_KERNEL_STACK_TAIL             __UINT64_C(0xffffa000bfbff000)
#define X86_VM_KERNEL_LAPIC_TAIL             __UINT64_C(0xffffa000bfc00000)


/* VM hints.
 * TODO: These hints are still designed for 32-bits.
 *       Move them around so they make better use of the large address space. */
#define VM_KERNEL_KERNELHEAP_LOCKED_HINT VM_ADDR2PAGE(X86_VM_KERNEL_KERNELHEAP_LOCKED_BASE)
#define VM_KERNEL_SHAREDHEAP_LOCKED_MODE VM_GETFREE_FABOVE
#define VM_KERNEL_KERNELHEAP_HINT        VM_ADDR2PAGE(X86_VM_KERNEL_KERNELHEAP_TAIL)
#define VM_KERNEL_KERNELHEAP_MODE        VM_GETFREE_FBELOW
#define VM_KERNEL_SHAREDHEAP_HINT        VM_ADDR2PAGE(X86_VM_KERNEL_SHAREDHEAP_BASE)
#define VM_KERNEL_SHAREDHEAP_MODE        VM_GETFREE_FABOVE
#define VM_KERNEL_SHAREDHEAP_LOCKED_HINT VM_ADDR2PAGE(X86_VM_KERNEL_SHAREDHEAP_LOCKED_TAIL)
#define VM_KERNEL_KERNELHEAP_LOCKED_MODE VM_GETFREE_FBELOW
#define VM_TEMPPAGE_HINT                 VM_ADDR2PAGE(X86_VM_KERNEL_TEMPPAGE_BASE)
#define VM_TEMPPAGE_MODE                 VM_GETFREE_FABOVE
#define VM_KERNEL_MALLHEAP_HINT          VM_ADDR2PAGE(X86_VM_KERNEL_MALLHEAP_TAIL)
#define VM_KERNEL_MALLHEAP_MODE          VM_GETFREE_FBELOW
#define VM_KERNELDRIVER_HINT             VM_ADDR2PAGE(X86_VM_KERNEL_DRIVER_BASE)
#define VM_KERNELDRIVER_MODE             VM_GETFREE_FABOVE
#define VM_KERNELDEBUG_HINT              VM_ADDR2PAGE(X86_VM_KERNEL_DEBUG_BASE)
#define VM_KERNELDEBUG_MODE              VM_GETFREE_FABOVE
#define VM_COREBASE_HINT                 VM_ADDR2PAGE(X86_VM_KERNEL_COREBASE_TAIL)
#define VM_COREBASE_MODE                 VM_GETFREE_FBELOW
#define VM_KERNELSTACK_HINT              VM_ADDR2PAGE(X86_VM_KERNEL_STACK_TAIL)
#define VM_KERNELSTACK_MODE              VM_GETFREE_FBELOW
#define X86_VM_LAPIC_HINT                VM_ADDR2PAGE(X86_VM_KERNEL_LAPIC_TAIL)
#define X86_VM_LAPIC_MODE                VM_GETFREE_FBELOW



/* TODO: These hints are still designed for 32-bits.
 *       Move them around so they make better use of the large address space. */
#define VM_USERLIB_HINT                  __UINT64_C(0x18000)
#define VM_USERLIB_MODE                  VM_GETFREE_FABOVE
#define VM_USERSEG_HINT                  __UINT64_C(0x40000) /* Hint for user-space thread segments. */
#define VM_USERSEG_MODE                  VM_GETFREE_FABOVE
#define VM_USERHEAP_HINT                 __UINT64_C(0x80000) /* Hint for automatic user-space heaps. */
#define VM_USERHEAP_MODE                 VM_GETFREE_FABOVE   /* Heap grows upwards; stack grows down. */
#define VM_USERSTACK_HINT                __UINT64_C(0xb0000) /* Hint for automatic user-space stacks. */
#define VM_USERSTACK_MODE                VM_GETFREE_FBELOW   /* Heap grows upwards; stack grows down. */
#define VM_USERSTACK_GAP                 8                   /* Size of the gap between automatically allocated user-space stacks. */
#define VM_USERENV_HINT                  __UINT64_C(0xc0000) /* Hint for user-space process environment. */
#define VM_USERENV_MODE                  VM_GETFREE_FBELOW


#define X86_PAGESHIFT      12 /* Shift to convert between pages and addresses. */

#define X86_PAGE_ALIGN     __UINT64_C(0x0000000000001000) /* Required page alignment. */
#define X86_PAGE_SIZE      __UINT64_C(0x0000000000001000) /* Required page size. */
#define X86_PAGE_FADDR     __UINT64_C(0x000ffffffffff000) /* Mask of the page address. */
#define X86_PAGE_FMASK     __UINT64_C(0xfff0000000000fff) /* Mask of page flag bits. */
#define X86_PAGE_FGLOBAL   __UINT64_C(0x0000000000000100) /* Set to optimize mappings that appear at the same location in all
                                                           * directories it appears inside of (aka: Kernel-allocated stack/memory). */
#define X86_PAGE_F2MIB     __UINT64_C(0x0000000000000080) /* Directly map a physical address on level #2, creating a 2-MIB page.
                                                           * NOTE: This flag may only be set in `union x86_pdir_e2::p_flag'
                                                           * NOTE: Use of this requires the `CR4_PSE' bit to be set. */
#ifndef CONFIG_NO_GIGABYTE_PAGES
#define X86_PAGE_F1GIB     __UINT64_C(0x0000000000000080) /* Directly map a physical address on level #3, creating a 1-GIB page.
                                                           * NOTE: This flag may only be set in `union x86_pdir_e3::p_flag'
                                                           * NOTE: Use of this requires the `CR4_PSE' bit to be set. */
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
#define X86_PAGE_FDIRTY    __UINT64_C(0x0000000000000040) /* The page has been written to. */
#define X86_PAGE_FACCESSED __UINT64_C(0x0000000000000020) /* The page has been read from, or written to. */
#define X86_PAGE_FUSER     __UINT64_C(0x0000000000000004) /* User-space may access this page (read, or write). */
#define X86_PAGE_FWRITE    __UINT64_C(0x0000000000000002) /* The page is writable. */
#define X86_PAGE_FPRESENT  __UINT64_C(0x0000000000000001) /* The page is present (When not set, cause a PAGEFAULT that may be used for allocate/load-on-read). */
#ifndef CONFIG_NO_NX_PAGES
#define X86_PAGE_FNOEXEC   __UINT64_C(0x8000000000000000) /* Memory within the page cannot be executed. */
#endif /* !CONFIG_NO_NX_PAGES */
#define X86_PAGE_ABSENT    __UINT64_C(0x0000000000000000) /* Value found in any TIER _EXCEPT_ TIER#1 (so just TIER#2) to indicate that the page hasn't been allocated. */

/* Pagesizes of different page directory levels. */
#define X86_PDIR_E1_SIZE     __UINT64_C(0x0000000000001000) /* 4 KiB (Same as `PAGESIZE') */
#define X86_PDIR_E2_SIZE     __UINT64_C(0x0000000000200000) /* 2 MiB */
#define X86_PDIR_E3_SIZE     __UINT64_C(0x0000000040000000) /* 1 GiB */
#define X86_PDIR_E4_SIZE     __UINT64_C(0x0000008000000000) /* 512 GiB */

/* Physical address masks of different page directory levels. */
#define X86_PDIR_E1_ADDRMASK __UINT64_C(0x000ffffffffff000) /* == (~(X86_PDIR_E1_SIZE-1) & X86_PAGE_FADDR) (For 4KiB pages) */
#define X86_PDIR_E2_ADDRMASK __UINT64_C(0x000fffffffe00000) /* == (~(X86_PDIR_E2_SIZE-1) & X86_PAGE_FADDR) (For 2Mib pages) */
#ifndef CONFIG_NO_GIGABYTE_PAGES
#define X86_PDIR_E3_ADDRMASK __UINT64_C(0x000fffffc0000000) /* == (~(X86_PDIR_E3_SIZE-1) & X86_PAGE_FADDR) (For 1Gib pages) */
#endif

/* The amount of sub-level entries contained within any given level. */
#define X86_PDIR_E1_COUNT    512 /* Amount of level #0 entries (pages). */
#define X86_PDIR_E2_COUNT    512 /* Amount of level #1 entries. */
#define X86_PDIR_E3_COUNT    512 /* Amount of level #2 entries. */
#define X86_PDIR_E4_COUNT    512 /* Amount of level #3 entries. */

/* Total amount of representable addresses of individual levels. */
#define X86_PDIR_E1_TOTALSIZE  __UINT64_C(0x0000000000200000) /* 2 MiB */
#define X86_PDIR_E2_TOTALSIZE  __UINT64_C(0x0000000040000000) /* 1 GiB */
#define X86_PDIR_E3_TOTALSIZE  __UINT64_C(0x0000008000000000) /* 512 GiB */
#define X86_PDIR_E4_TOTALSIZE  __UINT64_C(0x0001000000000000) /* 256 TiB */

/* Page index masks of different page directory levels.
 * These masks describe the bits that affect the path chosen to each a given E-level. */
#define X86_PDIR_E1_PAGEMASK   __UINT64_C(0xfffffffff) /* 1 Kib */
#define X86_PDIR_E2_PAGEMASK   __UINT64_C(0xffffffe00) /* 2 MiB */
#define X86_PDIR_E3_PAGEMASK   __UINT64_C(0xffffc0000) /* 1 GiB */
#define X86_PDIR_E4_PAGEMASK   __UINT64_C(0xff8000000) /* 512 GiB */

/* Page directory level indices. */
#define X86_PDIR_E1_INDEX(vpage)   (__CCAST(u64)(vpage) & 0x1ff)        /* For `union x86_pdir_e2::p_e1' */
#define X86_PDIR_E2_INDEX(vpage)  ((__CCAST(u64)(vpage) >> 9) & 0x1ff)  /* For `union x86_pdir_e3::p_e2' */
#define X86_PDIR_E3_INDEX(vpage)  ((__CCAST(u64)(vpage) >> 18) & 0x1ff) /* For `union x86_pdir_e4::p_e3' */
#define X86_PDIR_E4_INDEX(vpage)  ((__CCAST(u64)(vpage) >> 27) & 0x1ff) /* For `struct x86_pdir::p_e4' */

#define X86_PDIR_PAGE4(x4,x3,x2,x1) \
      ((__CCAST(uintptr_t)(x4) << 27) | \
       (__CCAST(uintptr_t)(x3) << 18) | \
       (__CCAST(uintptr_t)(x2) << 9) | \
        __CCAST(uintptr_t)(x1))
#define X86_PDIR_PAGE3(x4,x3,x2) \
      ((__CCAST(uintptr_t)(x4) << 27) | \
       (__CCAST(uintptr_t)(x3) << 18) | \
       (__CCAST(uintptr_t)(x2) << 9))
#define X86_PDIR_PAGE2(x4,x3) \
      ((__CCAST(uintptr_t)(x4) << 27) | \
       (__CCAST(uintptr_t)(x3) << 18))
#define X86_PDIR_PAGE1(x4) \
       (__CCAST(uintptr_t)(x4) << 27)

/* Page directory address offsets (Added to the mapped address when `X86_PDIR_E?_ISADDR(...)' is true). */
#define PDIR_E1_OFFSET(virt_addr)  (__CCAST(u64)(virt_addr) & (X86_PDIR_E1_SIZE-1)) /* 1KIB offsets */
#define PDIR_E2_OFFSET(virt_addr)  (__CCAST(u64)(virt_addr) & (X86_PDIR_E2_SIZE-1)) /* 2MIB offsets */
#ifndef CONFIG_NO_GIGABYTE_PAGES
#define PDIR_E3_OFFSET(virt_addr)  (__CCAST(u64)(virt_addr) & (X86_PDIR_E3_SIZE-1)) /* 1GIB offsets */
#endif

#ifdef __CC__
union x86_pdir_e1 {
    /* Level #1 (PT) entry. */
    u64                      p_data;      /* Mapping data. */
    PHYS u64                 p_addr;      /* [MASK(X86_PDIR_E1_ADDRMASK)]
                                           * [valid_if(X86_PAGE_FPRESENT)]
                                           * _Physical_ page address. */
    u64                      p_flag;      /* [MASK(X86_PAGE_FMASK)] Page flags. */
};

union x86_pdir_e2 {
    /* Level #2 (PD) entry. */
    u64                      p_data;     /* Mapping data. */
    PHYS u64                 p_addr;     /* [MASK(X86_PDIR_E2_ADDRMASK)]
                                          * [valid_if(X86_PAGE_FPRESENT && X86_PAGE_F2MIB)]
                                          *  _Physical_ page address. */
    u64                      p_flag;     /* [MASK(X86_PAGE_FMASK)] Page flags. */
    PHYS union x86_pdir_e1 (*p_e1)[X86_PDIR_E1_COUNT]; /* [MASK(X86_PAGE_FADDR)]
                                          * [owned_if(p_data != X86_PAGE_ABSENT && !X86_PAGE_F2MIB)]
                                          * [valid_if(X86_PAGE_FPRESENT && !X86_PAGE_F2MIB)]
                                          *  _Physical_ pointer to a level #1 paging vector.
                                          */
};
union x86_pdir_e3 {
    /* Level #3 (PDP) entry. */
    u64                      p_data;     /* Mapping data. */
    u64                      p_flag;     /* [MASK(X86_PAGE_FMASK)] Page flags. */
#ifdef CONFIG_NO_GIGABYTE_PAGES
    PHYS union x86_pdir_e2 (*p_e2)[X86_PDIR_E2_COUNT]; /* [MASK(X86_PAGE_FADDR)]
                                          * [owned_if(p_data != X86_PAGE_ABSENT)]
                                          * [valid_if(X86_PAGE_FPRESENT)]
                                          *  _Physical_ pointer to a level #2 paging vector. */
#else
    PHYS u64                 p_addr;     /* [MASK(X86_PDIR_E3_ADDRMASK)]
                                          * [valid_if(X86_PAGE_FPRESENT && X86_PAGE_F1GIB)]
                                          *  _Physical_ page address. */
    PHYS union x86_pdir_e2 (*p_e2)[X86_PDIR_E2_COUNT]; /* [MASK(X86_PAGE_FADDR)]
                                          * [owned_if(p_data != X86_PAGE_ABSENT && !X86_PAGE_F1GIB)]
                                          * [valid_if(X86_PAGE_FPRESENT && !X86_PAGE_F1GIB)]
                                          *  _Physical_ pointer to a level #2 paging vector. */
#endif
};
union x86_pdir_e4 {
    /* Level #4 (PML4) entry. */
    u64                      p_data;     /* Mapping data. */
    u64                      p_flag;     /* [MASK(X86_PAGE_FMASK)] Page flags. */
    PHYS union x86_pdir_e3 (*p_e3)[X86_PDIR_E3_COUNT]; /* [MASK(X86_PAGE_FADDR)]
                                          * [owned_if(p_data != X86_PAGE_ABSENT &&
                                          *         ((self - :p_e4) < X86_PDIR_E4_INDEX(KERNEL_BASE_PAGE)))]
                                          * [const_if((self - :p_e4) >= X86_PDIR_E4_INDEX(KERNEL_BASE_PAGE))]
                                          * [valid_if(X86_PAGE_FPRESENT)]
                                          *  _Physical_ pointer to a level #3 paging vector.
                                          * NOTE: If `(self - :p_e4) >= X86_PDIR_E4_INDEX(KERNEL_BASE_PAGE)',
                                          *       then the caller can assume that the `X86_PAGE_FPRESENT' flag
                                          *       has been set, and that a vector is assigned. */
};
union x86_pdir_ent {
    union x86_pdir_e1  e1;
    union x86_pdir_e2  e2;
    union x86_pdir_e3  e3;
    union x86_pdir_e4  e4;
};
#endif /* __CC__ */


#define X86_PDIR_ALIGN  4096 /* Alignment required for instances of `struct x86_pdir' */
#define X86_PDIR_SIZE   4096 /* The total size of `struct x86_pdir' in bytes. */

#ifdef __CC__
struct x86_pdir {
    union x86_pdir_e4   p_e4[X86_PDIR_E4_COUNT];  /* Level#4 page directory mappings. */
};
#endif /* __CC__ */


/* Page directory self-mapping addresses.
 * NOTE: We put the self-mapping 512 GIB after `KERNEL_BASE', just
 *       so we can ensure that the first kernel-page can remain
 *       unmapped, meaning that the kernel can allow user-space
 *       pointers to overflow into kernel-space, at which point
 *       any read/write operation will fault long before it would
 *       ever reach any real kernel data.
 *       The reason why we can't place it closer to the beginning,
 *       is that since the self-mapping takes up a total of 512 GIB
 *       of memory (at most), we can only place it with a granularity
 *       of that same size. */
#define X86_PDIR_E1_IDENTITY_BASE  X86_VM_KERNEL_PDIR_IDENTITY_BASE /* KERNEL_BASE + 512GiB */
#define X86_PDIR_E1_IDENTITY_SIZE  X86_VM_KERNEL_PDIR_IDENTITY_SIZE /* 512GiB */
#if 1
#define X86_PDIR_E2_IDENTITY_BASE  __UINT64_C(0xffff80c040000000)
#define X86_PDIR_E3_IDENTITY_BASE  __UINT64_C(0xffff80c060200000)
#define X86_PDIR_E4_IDENTITY_BASE  __UINT64_C(0xffff80c060301000)
#else
/* These are the actual addresses, as well as how they are calculated.
 * However, for clarity and compilation speed, we use the versions above instead. */
#define X86_PDIR_E2_IDENTITY_BASE (X86_PDIR_E1_IDENTITY_BASE + (X86_PDIR_E4_INDEX(VM_ADDR2PAGE(X86_PDIR_E1_IDENTITY_BASE)) * X86_PDIR_E3_SIZE))
#define X86_PDIR_E3_IDENTITY_BASE (X86_PDIR_E2_IDENTITY_BASE + (X86_PDIR_E3_INDEX(VM_ADDR2PAGE(X86_PDIR_E2_IDENTITY_BASE)) * X86_PDIR_E2_SIZE))
#define X86_PDIR_E4_IDENTITY_BASE (X86_PDIR_E3_IDENTITY_BASE + (X86_PDIR_E2_INDEX(VM_ADDR2PAGE(X86_PDIR_E3_IDENTITY_BASE)) * X86_PDIR_E1_SIZE))
#endif

#ifdef __CC__
/* NOTE: INDEX GENERATION:
 *    - X86_PDIR_E1_IDENTITY[X86_PDIR_E4_INDEX(*)][X86_PDIR_E3_INDEX(*)][X86_PDIR_E2_INDEX(*)][X86_PDIR_E1_INDEX(*)]
 *    - X86_PDIR_E2_IDENTITY[X86_PDIR_E4_INDEX(*)][X86_PDIR_E3_INDEX(*)][X86_PDIR_E2_INDEX(*)]
 *    - X86_PDIR_E3_IDENTITY[X86_PDIR_E4_INDEX(*)][X86_PDIR_E3_INDEX(*)]
 *    - X86_PDIR_E4_IDENTITY[X86_PDIR_E4_INDEX(*)] */
#define X86_PDIR_E1_IDENTITY  (*(x86_pdir_e1_identity_t *)X86_PDIR_E1_IDENTITY_BASE)
#define X86_PDIR_E2_IDENTITY  (*(x86_pdir_e2_identity_t *)X86_PDIR_E2_IDENTITY_BASE)
#define X86_PDIR_E3_IDENTITY  (*(x86_pdir_e3_identity_t *)X86_PDIR_E3_IDENTITY_BASE)
#define X86_PDIR_E4_IDENTITY  (*(x86_pdir_e4_identity_t *)X86_PDIR_E4_IDENTITY_BASE)
typedef union x86_pdir_e1 x86_pdir_e1_identity_t[X86_PDIR_E4_COUNT][X86_PDIR_E3_COUNT][X86_PDIR_E2_COUNT][X86_PDIR_E1_COUNT];
typedef union x86_pdir_e2 x86_pdir_e2_identity_t[X86_PDIR_E4_COUNT][X86_PDIR_E3_COUNT][X86_PDIR_E2_COUNT];
typedef union x86_pdir_e3 x86_pdir_e3_identity_t[X86_PDIR_E4_COUNT][X86_PDIR_E3_COUNT];
typedef union x86_pdir_e4 x86_pdir_e4_identity_t[X86_PDIR_E4_COUNT];
#endif

/* Define platform-independent symbols. */
#define PAGEDIR_ALIGN X86_PDIR_ALIGN
#define PAGEDIR_SIZE  X86_PDIR_SIZE
#ifdef __CC__
typedef struct x86_pdir pagedir_t;
#endif /* __CC__ */


/* On x86, the `pagedir_init' function never throws an error. */
#define CONFIG_PAGEDIR_INIT_IS_NOEXCEPT 1

/* x86 implements the `pagedir_(has|unset)changed' API. */
#define CONFIG_HAVE_PAGEDIR_CHANGED 1

#ifdef __CC__
/* Low-level Get/Set the physical address
 * of the currently active page directory. */
FORCELOCAL PHYS pagedir_t *KCALL pagedir_get(void);
FORCELOCAL void KCALL pagedir_set(PHYS pagedir_t *__restrict value);


#ifdef __x86_64__
FORCELOCAL PHYS pagedir_t *KCALL pagedir_get(void) {
 pagedir_t *result;
 __asm__("movq %%cr3, %0" : "=r" (result) : : "memory");
 return result;
}
FORCELOCAL void KCALL pagedir_set(PHYS pagedir_t *__restrict value) {
 __asm__("movq %0, %%cr3" : : "r" (value) : "memory");
}
#else
FORCELOCAL PHYS pagedir_t *KCALL pagedir_get(void) {
 pagedir_t *result;
 __asm__("movl %%cr3, %0" : "=r" (result) : : "memory");
 return result;
}
FORCELOCAL void KCALL pagedir_set(PHYS pagedir_t *__restrict value) {
 __asm__("movl %0, %%cr3" : : "r" (value) : "memory");
}
#endif

FUNDEF ATTR_NOTHROW bool KCALL pagedir_haschanged(vm_vpage_t vpage);
FUNDEF ATTR_NOTHROW void KCALL pagedir_unsetchanged(vm_vpage_t vpage);
#endif /* __CC__ */


#undef PAGEALIGN
#undef PAGESIZE
#define PAGEALIGN  X86_PAGE_ALIGN /* Required page alignment. */
#define PAGESIZE   X86_PAGE_SIZE  /* Required page size. */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_PAGING64_H */
