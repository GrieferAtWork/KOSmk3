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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_PAGING32_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_PAGING32_H 1

#include <hybrid/compiler.h>
#include <hybrid/typecore.h>
#include <kos/types.h>
#include <stdbool.h>

DECL_BEGIN

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Access a physical memory pointer during early boot. */
#define X86_EARLY_PHYS2VIRT(pptr) (pptr)
#define X86_EARLY_VIRT2PHYS(pptr) (pptr)
#endif


#define VM_VPAGE_SIZE  4
#define VM_PPAGE_SIZE  4
#define VM_VIRT_SIZE   4
#define VM_PHYS_SIZE   8
#ifdef __CC__
typedef VIRT __ULONGPTR_TYPE__ vm_vpage_t; /* Virtual memory page index (`virtual_address / PAGESIZE'). */
typedef PHYS __ULONGPTR_TYPE__ vm_ppage_t; /* Physical memory page index (`physical_address / PAGESIZE'). */
typedef VIRT __ULONGPTR_TYPE__ vm_virt_t;  /* A virtual memory pointer. */
typedef PHYS u64               vm_phys_t;  /* A physical memory pointer. */
#endif /* __CC__ */
#define VM_VPAGE_MAX           0xfffff
#define VM_PPAGE_MAX           0xfffff /* Without PAE, we're restricted to a 32-bit physical address space. */
#define KERNEL_BASE_PAGE   0xc0000 /* The page index of the kernel-base */
#define KERNEL_NUM_PAGES   0x40000 /* The number of pages associated with the kernel. */


/* Configure to indicate that the kernel is a high-kernel. */
#undef CONFIG_LOW_KERNEL
#undef CONFIG_HIGH_KERNEL
#define CONFIG_HIGH_KERNEL 1

/* KOS resides in the last 1Gb of virtual memory.
 * NOTE: This macro is only defined when `CONFIG_HIGH_KERNEL' is set. */
#define KERNEL_BASE        0xc0000000

#define ADDR_ISUSER(x)  (__CCAST(uintptr_t)(x) < KERNEL_BASE)
#define ADDR_ISHOST(x)  (__CCAST(uintptr_t)(x) >= KERNEL_BASE)


/* VM hints. */
#define VM_KERNEL_SHAREDHEAP_HINT        0xe1200
#define VM_KERNEL_SHAREDHEAP_MODE        VM_GETFREE_FABOVE
#define VM_KERNEL_KERNELHEAP_HINT        0xc0000
#define VM_KERNEL_KERNELHEAP_MODE        VM_GETFREE_FBELOW
#define VM_KERNEL_SHAREDHEAP_LOCKED_HINT 0xe1a00
#define VM_KERNEL_SHAREDHEAP_LOCKED_MODE VM_GETFREE_FABOVE
#define VM_KERNEL_KERNELHEAP_LOCKED_HINT 0xbf800
#define VM_KERNEL_KERNELHEAP_LOCKED_MODE VM_GETFREE_FBELOW
#define VM_KERNELSTACK_HINT              0xffbff
#define VM_KERNELSTACK_MODE              VM_GETFREE_FBELOW
#define VM_COREBASE_HINT                 0xf0000 /* Hint for where to allocate core-base pointers. */
#define VM_COREBASE_MODE                 VM_GETFREE_FBELOW
#define VM_TEMPPAGE_HINT                 0xef100 /* Hint for where to allocate thread-local temporary pages. */
#define VM_TEMPPAGE_MODE                 VM_GETFREE_FABOVE
#define VM_KERNEL_MALLHEAP_HINT          0xe0000
#define VM_KERNEL_MALLHEAP_MODE          VM_GETFREE_FBELOW
#define VM_KERNELDRIVER_HINT             0xe0000
#define VM_KERNELDRIVER_MODE             VM_GETFREE_FABOVE
#define VM_KERNELDEBUG_HINT              0xea000
#define VM_KERNELDEBUG_MODE              VM_GETFREE_FABOVE
#define X86_VM_LAPIC_HINT                0xffc00
#define X86_VM_LAPIC_MODE                VM_GETFREE_FBELOW

/* Likely memory layout of user-space applications:
 * 08048000...08048FFF x-r-u      app     -- .text    (hard-coded)
 * 08049000...08049FFF -wr-u      app     -- .data    (hard-coded)
 * 18045000...180B3FFF x-r-u      libc.so -- .text    (VM_USERLIB_HINT)
 * 180B4000...180B5FFF -wr-u      libc.so -- .data    (VM_USERLIB_HINT)
 * 7FFFF000...7FFFFFFF -wr-u      thread0 -- USERSEG  (VM_USERSEG_HINT)
 * 80000000...80000FFF -wr-u                 HEAP     (VM_USERHEAP_HINT)
 * AFFF8000...AFFFFFFF -wr-u      thread0 -- STACK    (VM_USERSTACK_HINT)
 * BEEFC000...BEEFCFFF x-r-u      syscall -- USHARE   (mapped in `/src/libs/libc/i386-kos/syscall.c')
 */
#define VM_USERLIB_HINT                  0x18000
#define VM_USERLIB_MODE                  VM_GETFREE_FABOVE
#define VM_USERSEG_HINT                  0x40000 /* Hint for user-space thread segments. */
#define VM_USERSEG_MODE                  VM_GETFREE_FABOVE
#define VM_USERHEAP_HINT                 0x80000 /* Hint for automatic user-space heaps. */
#define VM_USERHEAP_MODE                 VM_GETFREE_FABOVE
/* Heap grows upwards; stack grows down. */
#define VM_USERSTACK_HINT                0xb0000 /* Hint for automatic user-space stacks. */
#define VM_USERSTACK_MODE                VM_GETFREE_FBELOW
#define VM_USERSTACK_GAP                 8       /* Size of the gap between automatically allocated user-space stacks. */
#define VM_USERENV_HINT                  0xc0000 /* Hint for user-space process environment. */
#define VM_USERENV_MODE                  VM_GETFREE_FBELOW


#define X86_PAGESHIFT      12 /* Shift to convert between pages and addresses. */

#define X86_PAGE_ALIGN     0x00001000 /* Required page alignment. */
#define X86_PAGE_SIZE      0x00001000 /* Required page size. */
#define X86_PAGE_FADDR     0xfffff000 /* Mask of the page address. */
#define X86_PAGE_FMASK     0x00000fff /* Mask of page flag bits. */
#define X86_PAGE_FGLOBAL   0x00000100 /* Set to optimize mappings that appear at the same location in all
                                       * directories it appears inside of (aka: Kernel-allocated stack/memory). */
#define X86_PAGE_F4MIB     0x00000080 /* Directly map a physical address on level #2.
                                       * NOTE: Use of this requires the `CR4_PSE' bit to be set. */
#define X86_PAGE_FDIRTY    0x00000040 /* The page has been written to. */
#define X86_PAGE_FACCESSED 0x00000020 /* The page has been read from, or written to. */
#define X86_PAGE_FUSER     0x00000004 /* User-space may access this page (read, or write). */
#define X86_PAGE_FWRITE    0x00000002 /* The page is writable. */
#define X86_PAGE_FPRESENT  0x00000001 /* The page is present (When not set, cause a PAGEFAULT that may be used for allocate/load-on-read). */
#define X86_PAGE_ABSENT    0x00000000 /* Value found in any TIER _EXCEPT_ TIER#1 (so just TIER#2) to indicate that the page hasn't been allocated. */

#ifdef __CC__
union x86_pdir_e1 {
    /* Lowest-level page-directory entry. */
    u32                 p_data;      /* Mapping data. */
    PHYS u32            p_addr;      /* [MASK(X86_PAGE_FADDR)]
                                      * [valid_if(X86_PAGE_FPRESENT)]
                                      * _Physical_ page address. */
    u32                 p_flag;      /* [MASK(X86_PAGE_FMASK)] Page flags.
                                      *  NOTE: Must not contain `X86_PAGE_F4MIB'. */
};

union x86_pdir_e2 {
    /* TIER#2 page-directory entry. */
    u32                      p_data;      /* Mapping data. */
    PHYS u32                 p_addr;      /* [MASK(X86_PAGE_FADDR)]
                                           * [valid_if(X86_PAGE_FPRESENT && X86_PAGE_F4MIB)]
                                           *  _Physical_ page address. */
    u32                      p_flag;      /* [MASK(X86_PAGE_FMASK)] Page flags. */
    PHYS union x86_pdir_e1 (*p_e1)[1024]; /* [MASK(X86_PAGE_FADDR)]
                                           * [owned_if(p_data != X86_PAGE_ABSENT && !X86_PAGE_F4MIB &&
                                           *         ((self - :p_e2) < X86_PDIR_VEC2INDEX(KERNEL_BASE)))]
                                           * [const_if((self - :p_e2) >= X86_PDIR_VEC2INDEX(KERNEL_BASE))]
                                           * [valid_if(X86_PAGE_FPRESENT && !X86_PAGE_F4MIB)]
                                           *  _Physical_ pointer to a level #1 paging vector.
                                           */
};
#endif /* __CC__ */


#define X86_PDIR_ALIGN  4096 /* Alignment required for instances of `struct x86_pdir' */
#define X86_PDIR_SIZE   4096 /* The total size of `struct x86_pdir' in bytes. */

#ifdef __CC__
struct x86_pdir {
    union x86_pdir_e2   p_e2[1024];  /* Level#2 page directory mappings. */
};
#endif /* __CC__ */



/* Virtual memory addresses at which we place
 * identity mappings all allocated E1 vectors that are stored in E2 descriptors.
 * With that in mind, the `i'-th E1 vector (where `1' is the E2 index with
 * `x86_pdir::p_e2') can be found at `X86_PDIR_E1_IDENTITY_BASE+i*4096'
 * This address range is always mapped, and the caller must ensure validity
 * of an index by looking at their own page directory (which is not mapped
 * at a fixed address, but instead somewhere within the kernel page directory's
 * shared address space)
 */
#define X86_PDIR_E1_IDENTITY_BASE 0xffc00000 /* E1 vectors are mapped here. */
#define X86_PDIR_E1_IDENTITY_SIZE 0x00400000
#define X86_PDIR_E2_IDENTITY_BASE 0xfffff000 /* The `x86_pdir::p_e2' vector is mapped here. */

#define X86_VM_KERNEL_PDIR_RESERVED_BASE 0xffc00000 /* Start of the address range reserved for page-directory self-modifications. */
#define X86_VM_KERNEL_PDIR_RESERVED_SIZE 0x00400000 /* Amount of memory reserved for page-directory self-modifications. */

#ifdef __CC__
typedef union x86_pdir_e1 x86_pdir_e1_identity_t[1024][1024];
typedef union x86_pdir_e2 x86_pdir_e2_identity_t[1024];

/* E1 identity mapping access for the current page directory.
 *    Index #0: VEC2 -- The same index as used in `x86_pdir::p_e2'
 *    Index #1: VEC1 -- The same index as used in `x86_pdir_e2::p_e1'
 * Example:
 * >> VIRT void *pointer = get_pointer();
 * >> union x86_pdir_e1 *desc;
 * >> unsigned int vec2 = X86_PDIR_VEC2INDEX(pointer);
 * >> unsigned int vec1 = X86_PDIR_VEC1INDEX(pointer);
 * >> if (!(X86_PDIR_E2_IDENTITY[vec2].p_flag&X86_PAGE_FPRESENT)) {
 * >>     ...; // Pointer isn't mapped.
 * >> } else if (X86_PDIR_E2_IDENTITY[vec2].p_flag&X86_PAGE_F4MIB) {
 * >>     ...; // Pointer is mapped as a 4 MIB mapping
 * >> } else {
 * >>     // Load the descriptor
 * >>     desc = &X86_PDIR_E1_IDENTITY[vec2][vec1];
 * >>     if (!(desc->p_flag&X86_PAGE_FPRESENT)) {
 * >>         ...; // Pointer isn't mapped.
 * >>     } else {
 * >>         ...; // `desc' now points to the descriptor for the `pointer'
 * >>     }
 * >> }
 */
#define X86_PDIR_E1_IDENTITY  (*(x86_pdir_e1_identity_t *)X86_PDIR_E1_IDENTITY_BASE)
#define X86_PDIR_E2_IDENTITY  (*(x86_pdir_e2_identity_t *)X86_PDIR_E2_IDENTITY_BASE)


/* Return the VEC2 (for `x86_pdir::p_e2') or VEC1 (for `x86_pdir_e2::p_e1')
 * indices of a given pointer. */
#define X86_PDIR_VEC2INDEX(ptr)         (((uintptr_t)(ptr) >> 22))
#define X86_PDIR_VEC1INDEX(ptr)         (((uintptr_t)(ptr) >> 12) & 0x3ff)

/* Same as the 2 macros before, but taking VPAGE indices, rather than pointers. */
#define X86_PDIR_VEC2INDEX_VPAGE(vpage)  ((vm_vpage_t)(vpage) >> 10)
#define X86_PDIR_VEC1INDEX_VPAGE(vpage)  ((vm_vpage_t)(vpage) & 0x3ff)

/* Return the in-page offset of a regular pointer mapping,
 * or one that has been mapped within a 4MIB page. */
#define X86_PDIR_PAGEINDEX(ptr)     ((uintptr_t)(ptr) & 0xfff)
#define X86_PDIR_4MIBPAGEINDEX(ptr) ((uintptr_t)(ptr) & 0x3fffff)

#else /* __CC__ */

#define X86_PDIR_E1_IDENTITY          X86_PDIR_E1_IDENTITY_BASE
#define X86_PDIR_VEC2INDEX(ptr)    (((ptr) >> 22))
#define X86_PDIR_VEC1INDEX(ptr)    (((ptr) >> 12) & 0x3ff)
#define X86_PDIR_PAGEINDEX(ptr)     ((ptr) & 0xfff)
#define X86_PDIR_4MIBPAGEINDEX(ptr) ((ptr) & 0x3fffff)

#endif /* !__CC__ */





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

FORCELOCAL PHYS pagedir_t *KCALL pagedir_get(void) {
 pagedir_t *result;
 __asm__("movl %%cr3, %0" : "=r" (result) : : "memory");
 return result;
}
FORCELOCAL void KCALL pagedir_set(PHYS pagedir_t *__restrict value) {
 __asm__("movl %0, %%cr3" : : "r" (value) : "memory");
}

FUNDEF ATTR_NOTHROW bool KCALL pagedir_haschanged(vm_vpage_t vpage);
FUNDEF ATTR_NOTHROW void KCALL pagedir_unsetchanged(vm_vpage_t vpage);
#endif /* __CC__ */


#undef PAGEALIGN
#undef PAGESIZE
#define PAGEALIGN  X86_PAGE_ALIGN /* Required page alignment. */
#define PAGESIZE   X86_PAGE_SIZE  /* Required page size. */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_PAGING32_H */
