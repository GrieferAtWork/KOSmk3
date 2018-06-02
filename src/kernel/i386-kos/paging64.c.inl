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
#ifndef GUARD_KERNEL_I386_KOS_PAGING64_C_INL
#define GUARD_KERNEL_I386_KOS_PAGING64_C_INL 1
#define _KOS_SOURCE 1
#define __VM_INTERNAL_EXCLUDE_PAGEDIR 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>
#include <hybrid/section.h>
#include <hybrid/align.h>
#include <hybrid/minmax.h>
#include <kernel/paging.h>
#include <kernel/vm.h>
#include <string.h>
#include <except.h>
#include <kernel/debug.h>
#include <i386-kos/interrupt.h>
#include <i386-kos/cpuid.h>
#include <asm/cpu-flags.h>

DECL_BEGIN

#undef CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS
#undef CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN
#if !defined(NDEBUG) && 0
#define CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS 1
//#define CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN 1
#endif

/* Define some shorter names for structures and macros. */
typedef union x86_pdir_ent ENT;
typedef union x86_pdir_e1  E1;
typedef union x86_pdir_e2  E2;
typedef union x86_pdir_e3  E3;
typedef union x86_pdir_e4  E4;

#define PAGE_ALIGN     X86_PAGE_ALIGN     /* Required page alignment. */
#define PAGE_SIZE      X86_PAGE_SIZE      /* Required page size. */
#define PAGE_FADDR     X86_PAGE_FADDR     /* Mask of the page address. */
#define PAGE_FMASK     X86_PAGE_FMASK     /* Mask of page flag bits. */
#define PAGE_FGLOBAL   X86_PAGE_FGLOBAL   /* Set to optimize mappings that appear at the same location in all
                                           * directories it appears inside of (aka: Kernel-allocated stack/memory). */
#define PAGE_F2MIB     X86_PAGE_F2MIB     /* Directly map a physical address on level #2, creating a 2-MIB page.
                                           * NOTE: This flag may only be set in `union x86_pdir_e2::p_flag'
                                           * NOTE: Use of this requires the `CR4_PSE' bit to be set. */
#ifndef CONFIG_NO_GIGABYTE_PAGES
#define PAGE_F1GIB     X86_PAGE_F1GIB     /* Directly map a physical address on level #3, creating a 1-GIB page.
                                           * NOTE: This flag may only be set in `union x86_pdir_e3::p_flag'
                                           * NOTE: Use of this requires the `CR4_PSE' bit to be set. */
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
#define PAGE_FDIRTY    X86_PAGE_FDIRTY    /* The page has been written to. */
#define PAGE_FACCESSED X86_PAGE_FACCESSED /* The page has been read from, or written to. */
#define PAGE_FUSER     X86_PAGE_FUSER     /* User-space may access this page (read, or write). */
#define PAGE_FWRITE    X86_PAGE_FWRITE    /* The page is writable. */
#define PAGE_FPRESENT  X86_PAGE_FPRESENT  /* The page is present (When not set, cause a PAGEFAULT that may be used for allocate/load-on-read). */
#ifndef CONFIG_NO_NX_PAGES
#define PAGE_FNOEXEC   X86_PAGE_FNOEXEC   /* Memory within the page cannot be executed. */
#endif /* !CONFIG_NO_NX_PAGES */
#define PAGE_ABSENT    X86_PAGE_ABSENT    /* Value found in any TIER _EXCEPT_ TIER#1 (so just TIER#2) to indicate that the page hasn't been allocated. */


/* Pagesizes of different page directory levels. */
#define E1_SIZE      X86_PDIR_E1_SIZE /* 4 KiB (Same as `PAGESIZE') */
#define E2_SIZE      X86_PDIR_E2_SIZE /* 2 MiB */
#define E3_SIZE      X86_PDIR_E3_SIZE /* 1 GiB */
#define E4_SIZE      X86_PDIR_E4_SIZE /* 512 GiB */

/* Physical address masks of different page directory levels. */
#define E1_MASK      X86_PDIR_E1_ADDRMASK /* == (~(X86_PDIR_E1_SIZE-1) & X86_PAGE_FADDR) */
#define E2_MASK      X86_PDIR_E2_ADDRMASK /* == (~(X86_PDIR_E2_SIZE-1) & X86_PAGE_FADDR) */
#ifdef X86_PDIR_E3_ADDRMASK
#define E3_MASK      X86_PDIR_E3_ADDRMASK /* == (~(X86_PDIR_E3_SIZE-1) & X86_PAGE_FADDR) */
#endif

/* The amount of sub-level entries contained within any given level. */
#define E1_COUNT     X86_PDIR_E1_COUNT /* Amount of level #0 entries (pages). */
#define E2_COUNT     X86_PDIR_E2_COUNT /* Amount of level #1 entries. */
#define E3_COUNT     X86_PDIR_E3_COUNT /* Amount of level #2 entries. */
#define E4_COUNT     X86_PDIR_E4_COUNT /* Amount of level #3 entries. */

/* Total amount of representable addresses of individual levels. */
#define E1_TOTALSIZE X86_PDIR_E1_TOTALSIZE /* 2 MiB */
#define E2_TOTALSIZE X86_PDIR_E2_TOTALSIZE /* 1 GiB */
#define E3_TOTALSIZE X86_PDIR_E3_TOTALSIZE /* 512 GiB */
#define E4_TOTALSIZE X86_PDIR_E4_TOTALSIZE /* 256 TiB */

/* Page index masks of different page directory levels.
 * These masks describe the bits that affect the path chosen to each a given E-level. */
#define E1_PAGEMASK  X86_PDIR_E1_PAGEMASK
#define E2_PAGEMASK  X86_PDIR_E2_PAGEMASK
#define E3_PAGEMASK  X86_PDIR_E3_PAGEMASK
#define E4_PAGEMASK  X86_PDIR_E4_PAGEMASK

/* Page directory level indices. */
#define E4_INDEX     X86_PDIR_E4_INDEX /* For `struct x86_pdir::p_e4' */
#define E3_INDEX     X86_PDIR_E3_INDEX /* For `union x86_pdir_e4::p_e3' */
#define E2_INDEX     X86_PDIR_E2_INDEX /* For `union x86_pdir_e3::p_e2' */
#define E1_INDEX     X86_PDIR_E1_INDEX /* For `union x86_pdir_e2::p_e1' */

/* Page directory address offsets (Added to the mapped address when `X86_PDIR_E?_ISADDR(...)' is true). */
#define E3_OFFSET    PDIR_E3_OFFSET
#define E2_OFFSET    PDIR_E2_OFFSET
#define E1_OFFSET    PDIR_E1_OFFSET

/* NOTE: INDEX GENERATION:
 *    - E1_IDENTITY[E4_INDEX(*)][E3_INDEX(*)][E2_INDEX(*)][E1_INDEX(*)]
 *    - E2_IDENTITY[E4_INDEX(*)][E3_INDEX(*)][E2_INDEX(*)]
 *    - E3_IDENTITY[E4_INDEX(*)][E3_INDEX(*)]
 *    - E4_IDENTITY[E4_INDEX(*)] */
#define E1_IDENTITY  X86_PDIR_E1_IDENTITY
#define E2_IDENTITY  X86_PDIR_E2_IDENTITY
#define E3_IDENTITY  X86_PDIR_E3_IDENTITY
#define E4_IDENTITY  X86_PDIR_E4_IDENTITY



#define PAGING_E4_SHARE_INDEX   E4_INDEX(KERNEL_BASE_PAGE)
#define PAGING_E4_SHARE_COUNT  (E4_COUNT - PAGING_E4_SHARE_INDEX)




STATIC_ASSERT(X86_PDIR_E2_IDENTITY_BASE == (X86_PDIR_E1_IDENTITY_BASE + (X86_PDIR_E4_INDEX(VM_ADDR2PAGE(X86_PDIR_E1_IDENTITY_BASE)) * X86_PDIR_E3_SIZE)));
STATIC_ASSERT(X86_PDIR_E3_IDENTITY_BASE == (X86_PDIR_E2_IDENTITY_BASE + (X86_PDIR_E3_INDEX(VM_ADDR2PAGE(X86_PDIR_E2_IDENTITY_BASE)) * X86_PDIR_E2_SIZE)));
STATIC_ASSERT(X86_PDIR_E4_IDENTITY_BASE == (X86_PDIR_E3_IDENTITY_BASE + (X86_PDIR_E2_INDEX(VM_ADDR2PAGE(X86_PDIR_E3_IDENTITY_BASE)) * X86_PDIR_E1_SIZE)));

/* Code assumes that ZERO-initialization can be used to mark pages as absent. */
STATIC_ASSERT(X86_PAGE_ABSENT == 0);

/* NOTE: In the current configuration, `E4_IDENTITY_INDEX' is 257 (0x0101),
 *       reflecting its position one past the start of kernel-space at 256.
 *       The first 256 GIB of kernel space are not mapped in the default
 *       configuration, so-as to ensure that overflowing user-space pointers
 *       need to overflow by quite a lot before they could reach any mapped
 *       kernel memory. Related to this, the KOS kernel assumes that at the
 *       very least the first page of kernel memory is never mapped. */
enum{
 _PAGING_E4_IDENTITY_INDEX = E4_INDEX(VM_ADDR2PAGE(X86_PDIR_E1_IDENTITY_BASE)),
 _PAGING_E3_IDENTITY_INDEX = E3_INDEX(VM_ADDR2PAGE(X86_PDIR_E2_IDENTITY_BASE)),
 _PAGING_E2_IDENTITY_INDEX = E2_INDEX(VM_ADDR2PAGE(X86_PDIR_E3_IDENTITY_BASE)),
 _PAGING_E1_IDENTITY_INDEX = E1_INDEX(VM_ADDR2PAGE(X86_PDIR_E4_IDENTITY_BASE))
};
#define PAGING_E4_IDENTITY_INDEX _PAGING_E4_IDENTITY_INDEX
#define PAGING_E3_IDENTITY_INDEX _PAGING_E3_IDENTITY_INDEX
#define PAGING_E2_IDENTITY_INDEX _PAGING_E2_IDENTITY_INDEX
#define PAGING_E1_IDENTITY_INDEX _PAGING_E1_IDENTITY_INDEX

STATIC_ASSERT(PAGING_E4_IDENTITY_INDEX < 512);
STATIC_ASSERT(PAGING_E4_IDENTITY_INDEX == PAGING_E3_IDENTITY_INDEX);
STATIC_ASSERT(PAGING_E4_IDENTITY_INDEX == PAGING_E2_IDENTITY_INDEX);
STATIC_ASSERT(PAGING_E4_IDENTITY_INDEX == PAGING_E1_IDENTITY_INDEX);
STATIC_ASSERT(PAGING_E4_IDENTITY_INDEX >= PAGING_E4_SHARE_INDEX);
STATIC_ASSERT_MSG(PAGING_E4_IDENTITY_INDEX != PAGING_E4_SHARE_INDEX,
                  "Even though it might seem like a good idea, you can't "
                  "safely put the page directory identity mapping at the start "
                  "of kernel-space, because then user-space may be able to overflow "
                  "into it and corrupt it to gain access to an arbitrary "
                  "physical memory location");


/* Allocate BSS memory for the initial shared+identity mapping
 * that will later be shared with, and re-appear in all other
 * page directories (except for the identity page)
 * NOTE: This buffer is quite large (1Mb), but we'd need
 *       to allocate it sooner or later, no matter what. */
INTERN ATTR_SECTION(".bss.pagedir.kernel.share") VIRT union x86_pdir_e3
pagedir_kernel_share_e3[PAGING_E4_SHARE_COUNT][X86_PDIR_E3_COUNT];

/* The layer-2 indirection used to describe the initial identity
 * mapping of the last 2Gb of virtual memory, mapping to the first
 * 2Gb of physical memory, which also contain the kernel core.
 * HINT: This goes hand-in-hand with `KERNEL_CORE_BASE', which
 *       maps the kernel core, starting at -2Gb. */
INTERN ATTR_SECTION(".bss.pagedir.kernel.share2") VIRT union x86_pdir_e2
pagedir_kernel_share_e2[X86_PDIR_E3_INDEX(VM_ADDR2PAGE(KERNEL_CORE_SIZE))][X86_PDIR_E2_COUNT];

STATIC_ASSERT_MSG(X86_PDIR_E3_INDEX(VM_ADDR2PAGE(KERNEL_CORE_SIZE)) == 2,
                  "`boot64.S' expects that there are exactly 2 E2-vectors mapped "
                  "to the last 2 entires of `pagedir_kernel_share_e3'");


INTERN ATTR_SECTION(".data.pervm.head")
struct vm vm_kernel_head = {
    .vm_physdir  = 0,
    .vm_refcnt   = 3,
#ifdef CONFIG_VM_USE_RWLOCK
    .vm_lock     = RWLOCK_INIT,
#else
    .vm_lock     = MUTEX_INIT,
#endif
    .vm_map      = NULL,
    .vm_byaddr   = NULL,
    .vm_tasklock = ATOMIC_RWLOCK_INIT,
    .vm_size     = (size_t)kernel_pervm_size+PAGEDIR_SIZE
};

INTERN u64 x86_pageperm_matrix[0xf+1] = {
     /* Configure to always set the DIRTY and ACCESSED bits (KOS doesn't use
      * them, and us already setting them will allow the CPU to skip setting
      * them if the time comes) */
#ifndef CONFIG_NO_NX_PAGES
    [0]                                                                        = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FNOEXEC,
    [PAGEDIR_MAP_FEXEC]                                                        = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT,
    [PAGEDIR_MAP_FWRITE]                                                       = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT|PAGE_FWRITE|PAGE_FNOEXEC,
    [PAGEDIR_MAP_FWRITE|PAGEDIR_MAP_FEXEC]                                     = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT|PAGE_FWRITE,
    [PAGEDIR_MAP_FREAD]                                                        = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT|PAGE_FNOEXEC,
    [PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FEXEC]                                      = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT,
    [PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE]                                     = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT|PAGE_FWRITE|PAGE_FNOEXEC,
    [PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE|PAGEDIR_MAP_FEXEC]                   = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT|PAGE_FWRITE,
    [PAGEDIR_MAP_FUSER]                                                        = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FNOEXEC,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FEXEC]                                      = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FWRITE]                                     = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT|PAGE_FWRITE|PAGE_FNOEXEC,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FEXEC|PAGEDIR_MAP_FWRITE]                   = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT|PAGE_FWRITE,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD]                                      = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT|PAGE_FNOEXEC,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FEXEC]                    = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE]                   = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT|PAGE_FWRITE|PAGE_FNOEXEC,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE|PAGEDIR_MAP_FEXEC] = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT|PAGE_FWRITE,
#else /* !CONFIG_NO_NX_PAGES */
    [0]                                                                        = PAGE_FDIRTY|PAGE_FACCESSED,
    [PAGEDIR_MAP_FEXEC]                                                        = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT,
    [PAGEDIR_MAP_FWRITE]                                                       = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT|PAGE_FWRITE,
    [PAGEDIR_MAP_FWRITE|PAGEDIR_MAP_FEXEC]                                     = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT|PAGE_FWRITE,
    [PAGEDIR_MAP_FREAD]                                                        = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT,
    [PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FEXEC]                                      = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT,
    [PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE]                                     = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT|PAGE_FWRITE,
    [PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE|PAGEDIR_MAP_FEXEC]                   = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FPRESENT|PAGE_FWRITE,
    [PAGEDIR_MAP_FUSER]                                                        = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FEXEC]                                      = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FWRITE]                                     = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT|PAGE_FWRITE,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FEXEC|PAGEDIR_MAP_FWRITE]                   = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT|PAGE_FWRITE,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD]                                      = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FEXEC]                    = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE]                   = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT|PAGE_FWRITE,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE|PAGEDIR_MAP_FEXEC] = PAGE_FDIRTY|PAGE_FACCESSED|PAGE_FUSER|PAGE_FPRESENT|PAGE_FWRITE,
#endif /* CONFIG_NO_NX_PAGES */
};

INTERN u64 x86_page_global = PAGE_FGLOBAL;
#ifndef CONFIG_NO_GIGABYTE_PAGES
INTERN unsigned int x86_paging_features = 0;
#define PAGING_FEATURE_1GIB_PAGES  0x0001 /* Host supports 1GIB pages. */
#endif /* !CONFIG_NO_GIGABYTE_PAGES */


INTERN ATTR_FREETEXT void KCALL x86_configure_paging(void) {
 struct cpu_cpuid const *feat = &CPU_FEATURES;
 if (!(feat->ci_1d & CPUID_1D_PGE))
       x86_page_global = 0;
#ifndef CONFIG_NO_NX_PAGES
 if (!(feat->ci_80000001d & CPUID_80000001D_NX)) {
  unsigned int i; /* The NX bit isn't supported. */
  for (i = 0; i < COMPILER_LENOF(x86_pageperm_matrix); ++i)
       x86_pageperm_matrix[i] &= ~X86_PAGE_FNOEXEC;
 }
#endif /* !CONFIG_NO_NX_PAGES */
#ifndef CONFIG_NO_GIGABYTE_PAGES
 if (feat->ci_80000001d & CPUID_80000001D_PDPE1GB) {
  debug_printf(FREESTR("[X86] Enable 1GIB pages\n"));
  x86_paging_features |= PAGING_FEATURE_1GIB_PAGES;
 }
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
}


/* The memory zone used to allocate memory for page directories. */
#define MZONE_PAGING  MZONE_ANY








/* Initialize the given page directory.
 * The caller is required to allocate the page directory
 * controller itself, which must be aligned and sized
 * according to `PAGEDIR_ALIGN' and `PAGEDIR_SIZE'.
 * @throw E_BADALLOC: Not enough available memory. */
PUBLIC ATTR_NOTHROW void KCALL
pagedir_init(VIRT pagedir_t *__restrict self,
             PHYS vm_phys_t phys_self) {
 /* Map all pages before kernel-space as absent. */
 memsetq(self->p_e4,PAGE_ABSENT,PAGING_E4_IDENTITY_INDEX);
 /* Copy P4 pointers that are shared with the kernel.
  * NOTE: All of these have been initialized as P3 tables apart
  *       of the `pagedir_kernel_share' data block by `boot64.S' */
 memcpyq(&self->p_e4[PAGING_E4_SHARE_INDEX],
         &pagedir_kernel.p_e4[PAGING_E4_SHARE_INDEX],
          PAGING_E4_SHARE_COUNT);
 /* Create the identity mapping for the page directory. */
 self->p_e4[PAGING_E4_IDENTITY_INDEX].p_data = ((u64)phys_self |
                                                (PAGE_FDIRTY | PAGE_FACCESSED |
                                                 PAGE_FWRITE | PAGE_FPRESENT));
}




/* Finalize a given page directory. */
PUBLIC ATTR_NOTHROW void KCALL
pagedir_fini(VIRT pagedir_t *__restrict self) {
 ENT e; unsigned int i,j,k;
 pflag_t was; bool did_switch = false;
 for (i = 0; i < PAGING_E4_SHARE_INDEX; ++i) {
  union x86_pdir_e3 *e3_vector;
  e.e4 = self->p_e4[i];
  if ((e.e4.p_data & PAGE_FADDR) == PAGE_ABSENT)
      continue; /* Block was never mapped. */
  /* Since the E3-vector could still contain E2, or E1 mappings,
   * we must temporarily map it into our own address space, just
   * so we can free its E2- and E1-vectors.
   * To simplify everything, so we don't have to re-map the entire
   * directory within kernel-space, we simply choose to switch to
   * it for a short while.
   * The alternative to doing this would be having to do temporary
   * page mappings, and having to deal with E_BADALLOC causing those
   * temporary mappings to fail, including the possibility of a failure
   * to map the table in question (It would be ~real~ messy). */
  if (!did_switch) {
   was = PREEMPTION_PUSHOFF();
   did_switch = true;
   /* Assuming that page directory contents are valid, we can
    * just make the directory active while we're enumerating it. */
   pagedir_set((pagedir_t *)pagedir_translate((vm_virt_t)self));
  }

  /* Search for E2-vectors. */
  e3_vector = X86_PDIR_E3_IDENTITY[i];
  for (j = 0; j < E3_COUNT; ++j) {
   union x86_pdir_e2 *e2_vector;
   if ((e3_vector[j].p_data & PAGE_FADDR) == PAGE_ABSENT)
       continue; /* Block was never mapped. */
#ifndef CONFIG_NO_GIGABYTE_PAGES
   if (e3_vector[j].p_flag & PAGE_F1GIB)
       continue; /* Block is a gigabyte-page. */
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
   /* Scan the E2-vector */
   e2_vector = X86_PDIR_E2_IDENTITY[i][j];
   for (k = 0; k < E2_COUNT; ++k) {
    if ((e2_vector[k].p_data & PAGE_FADDR) == PAGE_ABSENT)
        continue; /* Block was never mapped. */
    if (e2_vector[k].p_flag & PAGE_F2MIB)
        continue; /* Block is a 2-megabyte-page. */
    /* Free the E1-vector. */
    page_free(VM_ADDR2PAGE(e2_vector[k].p_addr),1);
   }
   /* Free the E2-vector. */
   page_free(VM_ADDR2PAGE(e3_vector[j].p_addr),1);
  }
  /* Free the E3-vector. */
  page_free(VM_ADDR2PAGE(e.e4.p_data),1);
 }
 /* Switch back to the real page directory if we had to change it. */
 if (did_switch) {
  pagedir_set((pagedir_t *)THIS_VM->vm_physdir);
  /* Re-enable preemption if we disabled it momentarily. */
  PREEMPTION_POP(was);
 }
}

/* Similar to `VM_PAGE2ADDR', but don't set sign bits, meaning that
 * the generated address is suitable for `ENT::*::p_data' fields. */
#define VM_PAGE2ENTADDR(pageno) ((pageno) << 12)

PRIVATE void KCALL
pagedir_allocate_e3(unsigned int x4) {
 E4 ent; E3 *vec;
 ent = E4_IDENTITY[x4];
 if ((ent.p_data & PAGE_FADDR) != PAGE_ABSENT)
      return; /* Page has already been allocated. */
 assertf(x4 < E4_INDEX(KERNEL_BASE_PAGE),
        "All E4-vectors above KERNEL_BASE_PAGE must be allocated at all times!\n"
        "They're the indirection vectors that are shared between all directories!\n"
        "x4 = 0x%x",x4);
 COMPILER_WRITE_BARRIER();
 E4_IDENTITY[x4].p_data = (VM_PAGE2ENTADDR(page_malloc(1,MZONE_PAGING)) |
                          (PAGE_FDIRTY | PAGE_FACCESSED |
                           PAGE_FWRITE | PAGE_FPRESENT));
 COMPILER_WRITE_BARRIER();
 /* Load the virtual address of the E3 vector that we've just mapped. */
 vec = E3_IDENTITY[x4];
 pagedir_syncone(VM_ADDR2PAGE(vec));
 COMPILER_BARRIER();
 /* Clear out the E3 vector. */
 memsetq(vec,PAGE_ABSENT,E3_COUNT);
 COMPILER_WRITE_BARRIER();
}
PRIVATE void KCALL
pagedir_allocate_e2(unsigned int x4, unsigned int x3) {
 E3 ent; E2 *vec;
 ent = E3_IDENTITY[x4][x3];
#ifndef CONFIG_NO_GIGABYTE_PAGES
 if (ent.p_flag & PAGE_F1GIB) {
  unsigned int i;
  /* Split the 1Gib E3-entry into an E2-vector of 2Mib pages. */
  assertf((ent.p_data & (X86_PAGE_FADDR & ~X86_PDIR_E3_ADDRMASK)) == 0,
          "Invalid 1Gib page address: %p",ent.p_data);
  COMPILER_WRITE_BARRIER();
  E3_IDENTITY[x4][x3].p_data = (VM_PAGE2ENTADDR(page_malloc(1,MZONE_PAGING)) |
                               (PAGE_FDIRTY | PAGE_FACCESSED |
                                PAGE_FWRITE | PAGE_FPRESENT));
  COMPILER_WRITE_BARRIER();
  vec = E2_IDENTITY[x4][x3];
  pagedir_syncone(VM_ADDR2PAGE(vec));
  COMPILER_BARRIER();
#if PAGE_F1GIB != PAGE_F2MIB
  ent.p_flag &= ~PAGE_F1GIB;
  ent.p_flag |=  PAGE_F2MIB;
#endif
  COMPILER_BARRIER();
  /* Fill in the E2 vector. */
  for (i = 0; i < E2_COUNT; ++i) {
   vec[i].p_data = ent.p_data;
   ent.p_data += E2_SIZE;
  }
  COMPILER_WRITE_BARRIER();
  /* Re-add permission bits. */
  E3_IDENTITY[x4][x3].p_data |= ent.p_flag & (PAGE_FMASK & ~PAGE_F1GIB);
  COMPILER_WRITE_BARRIER();
 } else
#endif
 {
  if ((ent.p_data & PAGE_FADDR) != PAGE_ABSENT)
       return; /* Page has already been allocated. */
  /* NOTE: Don't set the FUSER bit, so user-space
   *       can't fiddle with uninitialized mappings! */
  COMPILER_WRITE_BARRIER();
  E3_IDENTITY[x4][x3].p_data = (VM_PAGE2ENTADDR(page_malloc(1,MZONE_PAGING)) |
                               (PAGE_FDIRTY | PAGE_FACCESSED |
                                PAGE_FWRITE | PAGE_FPRESENT));
  COMPILER_WRITE_BARRIER();
  /* Load the virtual address of the E3 vector that we've just mapped. */
  vec = E2_IDENTITY[x4][x3];
  pagedir_syncone(VM_ADDR2PAGE(vec));
  COMPILER_BARRIER();
  /* Clear out the E2 vector. */
  memsetq(vec,PAGE_ABSENT,E2_COUNT);
  COMPILER_WRITE_BARRIER();
 }
}


PRIVATE ATTR_FREETEXT uintptr_t KCALL
pagedir_map_temporary(pageptr_t e1_pageptr) {
 unsigned int x4,x3; uintptr_t result;
 x4 = E4_INDEX(VM_ADDR2PAGE(KERNEL_CORE_BASE))-1;
 for (; x4 > PAGING_E4_IDENTITY_INDEX; --x4) {
  assertf(E4_IDENTITY[x4].p_data != X86_PAGE_ABSENT,
          "All of these must be allocated _all_ _the_ _time_");
  for (x3 = 0; x3 < E3_COUNT; ++x3) {
   u64 data = E3_IDENTITY[x4][x3].p_data;
   if (data != X86_PAGE_ABSENT)
       continue; /* XXX: Use E2-vectors? */
   E3_IDENTITY[x4][x3].p_data = (VM_PAGE2ENTADDR(e1_pageptr) |
                                (PAGE_FDIRTY | PAGE_FACCESSED |
                                 PAGE_FWRITE | PAGE_FPRESENT));
   result = (uintptr_t)E2_IDENTITY[x4][x3];
   pagedir_syncone(VM_ADDR2PAGE((uintptr_t)result));
   return result;
  }
 }
 assertf(0,"TODO: This should invoke kernel panic!");
}

PRIVATE ATTR_FREETEXT void KCALL
pagedir_unmap_temporary(uintptr_t address) {
 unsigned int x4,x3;
 address  = VM_ADDR2PAGE(address);
 assertf(E4_INDEX(address) == PAGING_E4_IDENTITY_INDEX,
         "E4_INDEX(address) = %u\n",E4_INDEX(address));
 assertf(E3_INDEX(address) == PAGING_E3_IDENTITY_INDEX,
         "E3_INDEX(address) = %u\n",E3_INDEX(address));
 /* I know this looks strange, but because E4 and E3 are constant,
  * the indices used are shifted and we need to use these: */
 x4 = E2_INDEX(address);
 x3 = E1_INDEX(address);
 E3_IDENTITY[x4][x3].p_data = PAGE_ABSENT;
 pagedir_syncone(VM_ADDR2PAGE(address));
}


PRIVATE void KCALL
pagedir_allocate_e1(unsigned int x4, unsigned int x3, unsigned int x2) {
 E2 ent; E1 *vec;
 ent = E2_IDENTITY[x4][x3][x2];
 if (ent.p_flag & PAGE_F2MIB) {
  unsigned int i;
  /* Split the 2Mib E2-entry into an E1-vector of 1Kib pages. */
  assertf((ent.p_data & (X86_PAGE_FADDR & ~X86_PDIR_E2_ADDRMASK)) == 0,
          "Invalid 2Mib page address: %p",ent.p_data);
  if (x4 == E4_INDEX(VM_ADDR2PAGE(KERNEL_CORE_BASE)) &&
      x3 >= E3_INDEX(VM_ADDR2PAGE(KERNEL_CORE_BASE))) {
   /* _very_ specific case:
    *    The 2Mib page in question contains part of
    *    the kernel core itself, and as a consequence,
    *    contains this very function which this comment
    *    is apart of.
    * -> For that reason, we can't just temporarly replace
    *    it with a temporary (and invalid) physical page
    *    while we fill in that one.
    * Other memory mappings (including user-space) don't have this
    * problem thanks to `VM Memory race condition' (you may have
    * seen that system log entry before), which does 2 things:
    *   - Lazily map previously unmapped virtual memory into the
    *     page directory.
    *   - Deal with incomplete memory mappings by waiting for them
    *     to finish (which can be caused if user-space accesses a
    *     2Mib page while we're converting it into a vector of 4Kib
    *     pages)
    * 32-bit mode doesn't have this problem either, because it maps
    * the entire kernel using 4Kib pages from the get-go.
    * However, since in our case the page we're trying to allocate
    * contains the actual code that is trying to do that exact thing,
    * we have to cheat a little bit:
    *   #1: We know that the caller is holding a write-lock on the
    *       kernel VM, meaning that technically we not only have
    *       access to the portion we're supposed to be mapping, but
    *       actually have access to the entire page directory.
    *   #2: Make use of that right to map the E1-vector to a temporary
    *       location somewhere in kernel-space (we can't use user-space,
    *       because we must not expose it to this nastiness)
    *   #3: Fill in the physical memory using this temporary mapping.
    *   #4: Get rid of the temporary mapping
    *   #5: Atomically replace the 2Mib page with the E1-vector, meaning
    *       that it will seamlessly transition from 2Mib to 512*1Kib
    * NOTES:
    *   - Since we know that we're holding a write-lock on the kernel VM,
    *     this also means that we have implicit read+write permissions for
    *     the kernel-space portion of the page directory identity mapping.
    *     And using it, we map one of the E3-vector entries describing memory
    *     after the identity mapping, and before the kernel core.
    *   - Because the kernel is initially mapped using 2Mib pages (see boot.S),
    *     this special behavior is never needed for 1Gib pages, because the
    *     kernel cannot ever be loaded using 1Gib pages:
    *      - The only reason why an existing mapping could ever be converted into
    *        a 1Gib page without being remapped is using `pagedir_merge_before()'
    *      - However, in order for `pagedir_merge_before()' to merge a 1Gib page,
    *        all contained mappings must describe a continuous address space of
    *        non-interrupted 2Mib pages.
    *        That alone is something that should never really happen once early
    *        boot setup has been completed (because it would mean that either
    *        -2Gib...-1Gib are completely mapped, or -1Gib...0Gib are).
    *        And that kind of mapping is impossible to achieve because of various
    *        intended memory holes, such as found around the stack of _boot_task,
    *        of the fact that different parts of the kernel are mapped with different
    *        permissions.
    *      - It could happen for -1Gib...0Gib, but only if the kernel isn't part
    *        of that mapping, in which case the entire problem doesn't exist to
    *        begin with, as it's really just this small snippet of code right
    *        here that has to remain mapped consistently.
    *
    */
   pageptr_t e1_pageptr;
   e1_pageptr = page_malloc(1,MZONE_PAGING);
   vec = (E1 *)pagedir_map_temporary(e1_pageptr);
   COMPILER_BARRIER();
   ent.p_flag &= ~PAGE_F2MIB;
   /* Fill in the E1 vector. */
   for (i = 0; i < E1_COUNT; ++i) {
    vec[i].p_data = ent.p_data;
    ent.p_data += E1_SIZE;
   }
   COMPILER_WRITE_BARRIER();
   pagedir_unmap_temporary((uintptr_t)vec);
   COMPILER_WRITE_BARRIER();
   /* Seamlessly set the new page pointer to replace the 2Mib page.
    * NOTE: Use an atomic store for this to ensure that the memory
    *       get updated without any seams, and without the possibility
    *       of any partial writes. */
   ATOMIC_STORE(E2_IDENTITY[x4][x3][x2].p_data,
               (VM_PAGE2ENTADDR(e1_pageptr) |
               (PAGE_FDIRTY | PAGE_FACCESSED | PAGE_FWRITE | PAGE_FPRESENT) |
               (ent.p_flag & PAGE_FMASK)));
   COMPILER_WRITE_BARRIER();
  } else {
   COMPILER_WRITE_BARRIER();
   E2_IDENTITY[x4][x3][x2].p_data = (VM_PAGE2ENTADDR(page_malloc(1,MZONE_PAGING)) |
                                    (PAGE_FDIRTY | PAGE_FACCESSED |
                                     PAGE_FWRITE | PAGE_FPRESENT));
   COMPILER_WRITE_BARRIER();
   vec = E1_IDENTITY[x4][x3][x2];
   pagedir_syncone(VM_ADDR2PAGE(vec));
   COMPILER_BARRIER();
   ent.p_flag &= ~PAGE_F2MIB;
   /* Fill in the E1 vector. */
   for (i = 0; i < E1_COUNT; ++i) {
    vec[i].p_data = ent.p_data;
    ent.p_data += E1_SIZE;
   }
   COMPILER_WRITE_BARRIER();
   /* Re-add permission bits. */
   E2_IDENTITY[x4][x3][x2].p_data |= ent.p_flag & PAGE_FMASK;
   COMPILER_WRITE_BARRIER();
  }
 } else {
  if ((ent.p_data & PAGE_FADDR) != PAGE_ABSENT)
       return; /* Page has already been allocated. */
  /* NOTE: Don't set the FUSER bit, so user-space
   *       can't fiddle with uninitialized mappings! */
  COMPILER_WRITE_BARRIER();
  E2_IDENTITY[x4][x3][x2].p_data = (VM_PAGE2ENTADDR(page_malloc(1,MZONE_PAGING)) |
                                   (PAGE_FDIRTY | PAGE_FACCESSED |
                                    PAGE_FWRITE | PAGE_FPRESENT));
  COMPILER_WRITE_BARRIER();
  /* Load the virtual address of the E3 vector that we've just mapped. */
  vec = E1_IDENTITY[x4][x3][x2];
  pagedir_syncone(VM_ADDR2PAGE(vec));
  COMPILER_BARRIER();
  /* Clear out the E1 vector. */
  memsetq(vec,PAGE_ABSENT,E1_COUNT);
  COMPILER_WRITE_BARRIER();
 }
}



PRIVATE void KCALL
pagedir_delete_e1(unsigned int x4, unsigned int x3, unsigned int x2) {
 E2 ent = E2_IDENTITY[x4][x3][x2];
 if (ent.p_flag & PAGE_F2MIB) {
  E2_IDENTITY[x4][x3][x2].p_addr = PAGE_ABSENT;
  return;
 }
 if ((ent.p_data & PAGE_FADDR) == PAGE_ABSENT)
      return; /* Nothing mapped here! */
 E2_IDENTITY[x4][x3][x2].p_addr = PAGE_ABSENT;
 page_free(VM_ADDR2PAGE(ent.p_data),1);
}
PRIVATE void KCALL
pagedir_delete_e2(unsigned int x4, unsigned int x3) {
 E3 ent; unsigned int x2;
 ent = E3_IDENTITY[x4][x3];
#ifndef CONFIG_NO_GIGABYTE_PAGES
 if (ent.p_flag & PAGE_F1GIB) {
  E3_IDENTITY[x4][x3].p_addr = PAGE_ABSENT;
  return;
 }
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
 if ((ent.p_data & PAGE_FADDR) == PAGE_ABSENT)
      return; /* Nothing mapped here! */
 for (x2 = 0; x2 < E2_COUNT; ++x2) {
  pagedir_delete_e1(x4,x3,x2);
 }
 E3_IDENTITY[x4][x3].p_data = PAGE_ABSENT;
 page_free(VM_ADDR2PAGE(ent.p_data),1);
}
LOCAL void KCALL
pagedir_trydelete_e1(unsigned int x4, unsigned int x3, unsigned int x2) {
 E4 ent4; E3 ent3;
#ifdef CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN
 debug_printf("DELET[lv3](%p...%p)\n",
              VM_PAGE2ADDR(X86_PDIR_PAGE3(x4,x3,x2)),
              VM_PAGE2ADDR(X86_PDIR_PAGE3(x4,x3,x2+1))-1);
#endif /* CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN */
 ent4 = E4_IDENTITY[x4];
 if ((ent4.p_data & PAGE_FADDR) == PAGE_ABSENT)
      return; /* Nothing mapped here! */
 ent3 = E3_IDENTITY[x4][x3];
 if ((ent3.p_data & PAGE_FADDR) == PAGE_ABSENT
#ifndef CONFIG_NO_GIGABYTE_PAGES
     ||
     (ent3.p_flag & X86_PAGE_F1GIB)
#endif
     )
      return; /* Nothing mapped here! */
 pagedir_delete_e1(x4,x3,x2);
}
LOCAL void KCALL
pagedir_trydelete_e2(unsigned int x4, unsigned int x3) {
 E4 ent4 = E4_IDENTITY[x4];
#ifdef CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN
 debug_printf("DELET[lv2](%p...%p)\n",
              VM_PAGE2ADDR(X86_PDIR_PAGE2(x4,x3)),
              VM_PAGE2ADDR(X86_PDIR_PAGE2(x4,x3+1))-1);
#endif /* CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN */
 if ((ent4.p_data & PAGE_FADDR) == PAGE_ABSENT)
      return; /* Nothing mapped here! */
 pagedir_delete_e2(x4,x3);
}
LOCAL void KCALL
pagedir_trydelete_e3(unsigned int x4) {
 E4 ent; unsigned int x3;
 ent = E4_IDENTITY[x4];
#ifdef CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN
 debug_printf("DELET[lv1](%p...%p)\n",
              VM_PAGE2ADDR(X86_PDIR_PAGE1(x4)),
              VM_PAGE2ADDR(X86_PDIR_PAGE1(x4+1))-1);
#endif /* CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN */
 if ((ent.p_data & PAGE_FADDR) == PAGE_ABSENT)
      return; /* Nothing mapped here! */
 for (x3 = 0; x3 < E3_COUNT; ++x3) {
  pagedir_delete_e2(x4,x3);
 }
 /* Don't free level#4 vectors above KERNEL_BASE. */
 if (x4 < E4_INDEX(KERNEL_BASE_PAGE)) {
  E4_IDENTITY[x4].p_data = PAGE_ABSENT;
  page_free(VM_ADDR2PAGE(ent.p_data),1);
 }
}


LOCAL void KCALL
pagedir_split_before(VIRT vm_vpage_t virt_page, bool for_unmap) {
 unsigned int x4,x3;
 if (!(virt_page & ~E4_PAGEMASK) && for_unmap)
       return; /* Address is located on an E4-boundary (nothing to do here!) */
 pagedir_allocate_e3(x4 = E4_INDEX(virt_page));
 if (!(virt_page & ~E3_PAGEMASK) &&
      (for_unmap
#ifndef CONFIG_NO_GIGABYTE_PAGES
    || x86_paging_features & PAGING_FEATURE_1GIB_PAGES
#endif
       ))
       return; /* Address is located on an E3-boundary */
 pagedir_allocate_e2(x4,x3 = E3_INDEX(virt_page));
 if (!(virt_page & ~E2_PAGEMASK))
       return; /* Address is located on an E3-boundary */
 pagedir_allocate_e1(x4,x3,E2_INDEX(virt_page));
}

LOCAL void KCALL
pagedir_merge_before(VIRT vm_vpage_t virt_page) {
 /* TODO */
}



LOCAL void KCALL
pagedir_addperm_e2(unsigned int x4,
                   unsigned int x3,
                   unsigned int x2,
                   u64 edata) {
 u64 flags;
 //assert(x4 != PAGING_E4_IDENTITY_INDEX);
 flags = E2_IDENTITY[x4][x3][x2].p_flag;
#ifdef CONFIG_NO_NX_PAGES
 flags |= edata & PAGE_FMASK;
#else /* CONFIG_NO_NX_PAGES */
 flags |= edata & (PAGE_FMASK & ~PAGE_FNOEXEC);
 flags &= (edata & PAGE_FNOEXEC) | ~PAGE_FNOEXEC;
#endif /* !CONFIG_NO_NX_PAGES */
 E2_IDENTITY[x4][x3][x2].p_flag = flags;
}

LOCAL void KCALL
pagedir_addperm_e3(unsigned int x4,
                   unsigned int x3,
                   u64 edata) {
 u64 flags;
 //assert(x4 != PAGING_E4_IDENTITY_INDEX);
 flags = E3_IDENTITY[x4][x3].p_flag;
#ifdef CONFIG_NO_NX_PAGES
 flags |= edata & PAGE_FMASK;
#else /* CONFIG_NO_NX_PAGES */
 flags |= edata & (PAGE_FMASK & ~PAGE_FNOEXEC);
 flags &= (edata & PAGE_FNOEXEC) | ~PAGE_FNOEXEC;
#endif /* !CONFIG_NO_NX_PAGES */
 E3_IDENTITY[x4][x3].p_flag = flags;
}

LOCAL void KCALL
pagedir_addperm_e4(unsigned int x4,
                   u64 edata) {
 u64 flags;
 //assert(x4 != PAGING_E4_IDENTITY_INDEX);
 flags = E4_IDENTITY[x4].p_flag;
#ifdef CONFIG_NO_NX_PAGES
 flags |= edata & PAGE_FMASK;
#else /* CONFIG_NO_NX_PAGES */
 flags |= edata & (PAGE_FMASK & ~PAGE_FNOEXEC);
 flags &= (edata & PAGE_FNOEXEC) | ~PAGE_FNOEXEC;
#endif /* !CONFIG_NO_NX_PAGES */
 E4_IDENTITY[x4].p_flag = flags;
}



/* Create/delete a page-directory mapping.
 * @param: perm: A set of `PAGEDIR_MAP_F*' detailing how memory should be mapped.
 * @throw E_BADALLOC: Not enough available memory.
 * `pagedir_sync()' must be called while specifying a virtual address range containing
 * `virt_page...+=num_pages' in order to ensure that changes will become visible.
 * NOTE: This function can be called regardless of which page directory is active. */
PUBLIC void KCALL
pagedir_map(VIRT vm_vpage_t virt_page, size_t num_pages,
            PHYS vm_ppage_t phys_page, u16 perm) {
 vm_vpage_t vpage_start,vpage_end;
 u64 edata; unsigned int max_pages;
 /* Check for special case: no mapping. */
 if (!num_pages) return;
#if !defined(NDEBUG) && 1
 if ((perm & PAGEDIR_MAP_FUSER) &&
      virt_page >= KERNEL_BASE_PAGE)
      asm("int3");
#endif
#ifdef CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS
 debug_printf("pagedir_map(%p...%p,%p...%p,%s%c%c%c%c)\n",
              VM_PAGE2ADDR(virt_page),
              VM_PAGE2ADDR(virt_page+num_pages)-1,
              VM_PAGE2ADDR(phys_page),
              VM_PAGE2ADDR(phys_page+num_pages)-1,
              perm&PAGEDIR_MAP_FUNMAP ? "DELETE," : "",
              perm&PAGEDIR_MAP_FEXEC ? 'X' : '-',
              perm&PAGEDIR_MAP_FWRITE ? 'W' : '-',
              perm&PAGEDIR_MAP_FREAD ? 'R' : '-',
              perm&PAGEDIR_MAP_FUSER ? 'U' : '-');
 /*__asm__("int3");*/
#endif
 vpage_start = virt_page;
 vpage_end   = virt_page+num_pages;
 assertf(!(virt_page+num_pages > VM_ADDR2PAGE(X86_VM_KERNEL_PDIR_IDENTITY_BASE) &&
           virt_page < VM_ADDR2PAGE(X86_VM_KERNEL_PDIR_IDENTITY_BASE+
                                    X86_VM_KERNEL_PDIR_IDENTITY_SIZE)),
         "The map address range overlaps with the page directory self mapping:\n"
         "\tMAP:      %p...%p\n"
         "\tIDENTITY: %p...%p\n",
         VM_PAGE2ADDR(virt_page),
         VM_PAGE2ADDR(virt_page+num_pages)-1,
         X86_VM_KERNEL_PDIR_IDENTITY_BASE,
         X86_VM_KERNEL_PDIR_IDENTITY_BASE+
         X86_VM_KERNEL_PDIR_IDENTITY_SIZE-1);
 assertf(virt_page <= VM_VPAGE_MAX,"virt_page = %p",virt_page);
 assertf((perm&PAGEDIR_MAP_FUNMAP) || phys_page <= VM_PPAGE_MAX,"phys_page = %p",phys_page);
 assert(virt_page+num_pages >= virt_page);
 assert((perm&PAGEDIR_MAP_FUNMAP) || phys_page+num_pages >= phys_page);
 assertf(virt_page+num_pages <= (VM_VPAGE_MAX+1),
         "virt_page = %p\n"
         "num_pages = %p\n",
         virt_page,num_pages);
 assert((perm&PAGEDIR_MAP_FUNMAP) || phys_page+num_pages <= (VM_PPAGE_MAX+1));
 edata = (u64)VM_PAGE2ADDR(phys_page) | x86_pageperm_matrix[perm & 0xf];
 if (virt_page >= KERNEL_BASE_PAGE)
     edata |= x86_page_global; /* Kernel-share mapping. */
 pagedir_split_before(vpage_start,(perm & PAGEDIR_MAP_FUNMAP) != 0);
 pagedir_split_before(vpage_end,(perm & PAGEDIR_MAP_FUNMAP) != 0);

 /* First off: Map all unaligned 4Kib pages.
  *            Thanks to `pagedir_split_before(vpage_start)',
  *            we can be certain that if such pages exist, they
  *            have all been fully allocated! */
 max_pages = E1_INDEX(virt_page);
 if (max_pages != 0) {
  E1 *vec; unsigned int offset;
  unsigned int x4,x3,x2;
  offset    = max_pages;
  max_pages = E1_COUNT - max_pages;
  if (max_pages > num_pages)
      max_pages = num_pages;
  assert(max_pages != 0);
  vec = E1_IDENTITY[x4 = E4_INDEX(virt_page)]
                   [x3 = E3_INDEX(virt_page)]
                   [x2 = E2_INDEX(virt_page)];
  if (perm & PAGEDIR_MAP_FUNMAP) {
#ifdef CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN
   debug_printf("UNMAP[lv4](%p...%p)\n",
                VM_PAGE2ADDR(X86_PDIR_PAGE4(x4,x3,x2,offset)),
                VM_PAGE2ADDR(X86_PDIR_PAGE4(x4,x3,x2,offset+max_pages))-1);
#endif /* CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN */
   memsetq(&vec[offset],PAGE_ABSENT,max_pages);
  } else {
   unsigned int x1;
   for (x1 = 0; x1 < max_pages; ++x1) {
    vec[offset + x1].p_addr = edata;
    edata += E1_SIZE;
   }
   pagedir_addperm_e2(x4,x3,x2,edata);
   pagedir_addperm_e3(x4,x3,edata);
   pagedir_addperm_e4(x4,edata);
  }
  num_pages -= max_pages;
  if (!num_pages)
       goto done;
  virt_page += max_pages;
 }
 max_pages = E2_INDEX(virt_page);
 if (max_pages != 0 &&
    (perm & PAGEDIR_MAP_FUNMAP || IS_ALIGNED(edata & X86_PAGE_FADDR,E2_SIZE))) {
  E2 *vec; unsigned int offset,x4,x3;
  /* At this point, we're aligned for 2Mib pages. */
  offset    = max_pages;
  max_pages = E2_COUNT - max_pages;
  if (max_pages > num_pages / (E2_SIZE / PAGESIZE))
      max_pages = num_pages / (E2_SIZE / PAGESIZE);
  if (!max_pages) goto do_full_1kib_pages;
  assertf(IS_ALIGNED(virt_page,X86_PDIR_E2_SIZE / PAGESIZE),
          "Not aligned: %p (%p)",virt_page,VM_PAGE2ADDR(virt_page));
  vec = E2_IDENTITY[x4 = E4_INDEX(virt_page)]
                   [x3 = E3_INDEX(virt_page)];
  if (perm & PAGEDIR_MAP_FUNMAP) {
#ifdef CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN
   debug_printf("UNMAP[lv3](%p...%p)\n",
                VM_PAGE2ADDR(X86_PDIR_PAGE3(x4,x3,offset)),
                VM_PAGE2ADDR(X86_PDIR_PAGE3(x4,x3,offset+max_pages))-1);
#endif /* CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN */
   memsetq(&vec[offset],PAGE_ABSENT,max_pages);
  } else {
   unsigned int i;
   for (i = 0; i < max_pages; ++i) {
    vec[offset + i].p_addr = edata | PAGE_F2MIB;
    edata += E2_SIZE;
   }
   pagedir_addperm_e3(x4,x3,edata);
   pagedir_addperm_e4(x4,edata);
  }
  num_pages -= max_pages * (E2_SIZE / PAGESIZE);
  if (!num_pages)
       goto done;
  virt_page += max_pages * (E2_SIZE / PAGESIZE);
 }
 max_pages = E3_INDEX(virt_page);
#ifndef CONFIG_NO_GIGABYTE_PAGES
 if (max_pages != 0 &&
    (perm & PAGEDIR_MAP_FUNMAP ||
    (x86_paging_features & PAGING_FEATURE_1GIB_PAGES &&
     IS_ALIGNED(edata & X86_PAGE_FADDR,E3_SIZE))))
#else /* !CONFIG_NO_GIGABYTE_PAGES */
 if (max_pages != 0 && (perm & PAGEDIR_MAP_FUNMAP))
#endif /* CONFIG_NO_GIGABYTE_PAGES */
 {
  E3 *vec; unsigned int offset,x4;
  offset    = max_pages;
  max_pages = E3_COUNT - max_pages;
  if (max_pages > num_pages / (E3_SIZE / PAGESIZE))
      max_pages = num_pages / (E3_SIZE / PAGESIZE);
  if (!max_pages) goto do_full_2mib_pages;
  /* At this point, we're aligned for 1Gib pages. */
  assertf(IS_ALIGNED(virt_page,X86_PDIR_E3_SIZE / PAGESIZE),
          "Not aligned: %p (%p)",virt_page,VM_PAGE2ADDR(virt_page));
  vec = E3_IDENTITY[x4 = E4_INDEX(virt_page)];
#ifndef CONFIG_NO_GIGABYTE_PAGES
  if (!(perm & PAGEDIR_MAP_FUNMAP)) {
   unsigned int i;
   for (i = 0; i < max_pages; ++i) {
    vec[offset + i].p_addr = edata | PAGE_F1GIB;
    edata += E3_SIZE;
   }
   pagedir_addperm_e4(x4,edata);
  } else
#endif
  {
#ifdef CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN
   debug_printf("UNMAP[lv2](%p...%p)\n",
                VM_PAGE2ADDR(X86_PDIR_PAGE2(x4,offset)),
                VM_PAGE2ADDR(X86_PDIR_PAGE2(x4,offset+max_pages))-1);
#endif /* CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN */
   memsetq(&vec[offset],PAGE_ABSENT,max_pages);
  }
  num_pages -= max_pages * (E3_SIZE / PAGESIZE);
  if (!num_pages)
       goto done;
  virt_page += max_pages * (E3_SIZE / PAGESIZE);
 }
 /* Now map all full 512 Gib pages! */
 while (num_pages >= (E4_SIZE / PAGESIZE)) {
  unsigned int x4,x3,x2,x1;
  x4 = E4_INDEX(virt_page);
  assertf(IS_ALIGNED(virt_page,X86_PDIR_E4_SIZE / PAGESIZE),
          "Not aligned: %p (%p)",virt_page,VM_PAGE2ADDR(virt_page));
  if (perm & PAGEDIR_MAP_FUNMAP) {
   pagedir_trydelete_e3(x4);
  } else {
   pagedir_allocate_e3(x4);
#ifndef CONFIG_NO_GIGABYTE_PAGES
   if ((x86_paging_features & PAGING_FEATURE_1GIB_PAGES) &&
        IS_ALIGNED(edata & X86_PAGE_FADDR,E3_SIZE)) {
    for (x3 = 0; x3 < E3_COUNT; ++x3) {
     pagedir_trydelete_e2(x4,x3);
     /* Map as 1GiB pages. */
     E3_IDENTITY[x4][x3].p_addr = edata | PAGE_F1GIB;
     edata += X86_PDIR_E3_SIZE;
    }
    pagedir_addperm_e4(x4,edata);
   } else
#endif
   if (IS_ALIGNED(edata & X86_PAGE_FADDR,E2_SIZE)) {
    /* Map as 512x512 2Mib pages. */
    for (x3 = 0; x3 < E3_COUNT; ++x3) {
     pagedir_allocate_e2(x4,x3);
     for (x2 = 0; x2 < E2_COUNT; ++x2) {
      pagedir_trydelete_e1(x4,x3,x2);
      E2_IDENTITY[x4][x3][x2].p_addr = edata | PAGE_F2MIB;
      edata += X86_PDIR_E2_SIZE;
     }
     pagedir_addperm_e3(x4,x3,edata);
    }
   } else {
    /* Map as 512x512x512 1Kib pages. */
    for (x3 = 0; x3 < E3_COUNT; ++x3) {
     pagedir_allocate_e2(x4,x3);
     for (x2 = 0; x2 < E2_COUNT; ++x2) {
      pagedir_allocate_e1(x4,x3,x2);
      for (x1 = 0; x1 < E1_COUNT; ++x1) {
       E1_IDENTITY[x4][x3][x2][x1].p_addr = edata;
       edata += X86_PDIR_E1_SIZE;
      }
      pagedir_addperm_e2(x4,x3,x2,edata);
     }
     pagedir_addperm_e3(x4,x3,edata);
    }
   }
   pagedir_addperm_e4(x4,edata);
  }
  num_pages -= (E4_SIZE / PAGESIZE);
  if (!num_pages) goto done;
  virt_page += (E4_SIZE / PAGESIZE);
 }
 /* Now map all full 1 Gib pages! */
 while (num_pages >= (E3_SIZE / PAGESIZE)) {
  unsigned int x4,x3,x2,x1;
  x4 = E4_INDEX(virt_page);
  x3 = E3_INDEX(virt_page);
  assertf(IS_ALIGNED(virt_page,X86_PDIR_E3_SIZE / PAGESIZE),
          "Not aligned: %p (%p)",virt_page,VM_PAGE2ADDR(virt_page));
  pagedir_allocate_e3(x4);
  if (perm & PAGEDIR_MAP_FUNMAP) {
   pagedir_trydelete_e2(x4,x3);
  } else {
#ifndef CONFIG_NO_GIGABYTE_PAGES
   if ((x86_paging_features & PAGING_FEATURE_1GIB_PAGES) &&
        IS_ALIGNED(edata & X86_PAGE_FADDR,E3_SIZE)) {
    pagedir_trydelete_e2(x4,x3);
    /* Map as 1GiB pages. */
    E3_IDENTITY[x4][x3].p_addr = edata | PAGE_F1GIB;
    edata += X86_PDIR_E3_SIZE;
    pagedir_addperm_e4(x4,edata);
   } else
#endif
   {
    pagedir_allocate_e2(x4,x3);
    if (IS_ALIGNED(edata & X86_PAGE_FADDR,E2_SIZE)) {
     /* Map as 512 2Mib pages. */
     for (x2 = 0; x2 < E2_COUNT; ++x2) {
      pagedir_trydelete_e1(x4,x3,x2);
      E2_IDENTITY[x4][x3][x2].p_addr = edata | PAGE_F2MIB;
      edata += X86_PDIR_E2_SIZE;
     }
    } else {
     /* Map as 512x512 1Kib pages. */
     for (x2 = 0; x2 < E2_COUNT; ++x2) {
      pagedir_allocate_e1(x4,x3,x2);
      for (x1 = 0; x1 < E1_COUNT; ++x1) {
       E1_IDENTITY[x4][x3][x2][x1].p_addr = edata;
       edata += X86_PDIR_E1_SIZE;
      }
      pagedir_addperm_e2(x4,x3,x2,edata);
     }
    }
    pagedir_addperm_e3(x4,x3,edata);
    pagedir_addperm_e4(x4,edata);
   }
  }
  num_pages -= (E3_SIZE / PAGESIZE);
  if (!num_pages) goto done;
  virt_page += (E3_SIZE / PAGESIZE);
 }
 /* Now map all full 2 Mib pages! */
do_full_2mib_pages:
 while (num_pages >= (E2_SIZE / PAGESIZE)) {
  unsigned int x4,x3,x2,x1;
  x4 = E4_INDEX(virt_page);
  x3 = E3_INDEX(virt_page);
  x2 = E2_INDEX(virt_page);
  assertf(IS_ALIGNED(virt_page,X86_PDIR_E2_SIZE / PAGESIZE),
          "Not aligned: %p (%p)",virt_page,VM_PAGE2ADDR(virt_page));
  pagedir_allocate_e3(x4);
  pagedir_allocate_e2(x4,x3);
  if (perm & PAGEDIR_MAP_FUNMAP) {
   pagedir_trydelete_e1(x4,x3,x2);
  } else {
   if (IS_ALIGNED(edata & X86_PAGE_FADDR,E2_SIZE)) {
    /* Map as 1 2Mib pages. */
    pagedir_trydelete_e1(x4,x3,x2);
    E2_IDENTITY[x4][x3][x2].p_addr = edata | PAGE_F2MIB;
    edata += X86_PDIR_E2_SIZE;
   } else {
    pagedir_allocate_e1(x4,x3,x2);
    /* Map as 512 1Kib pages. */
    for (x1 = 0; x1 < E1_COUNT; ++x1) {
     E1_IDENTITY[x4][x3][x2][x1].p_addr = edata;
     edata += X86_PDIR_E1_SIZE;
    }
    pagedir_addperm_e2(x4,x3,x2,edata);
   }
   pagedir_addperm_e3(x4,x3,edata);
   pagedir_addperm_e4(x4,edata);
  }
  num_pages -= (E2_SIZE / PAGESIZE);
  if (!num_pages) goto done;
  virt_page += (E2_SIZE / PAGESIZE);
 }
 /* Now map all the remaining pages! */
do_full_1kib_pages:
 while (num_pages) {
  STATIC_ASSERT((E1_SIZE / PAGESIZE) == 1);
  unsigned int x4,x3,x2,x1;
  x4 = E4_INDEX(virt_page);
  x3 = E3_INDEX(virt_page);
  x2 = E2_INDEX(virt_page);
  x1 = E1_INDEX(virt_page);
#if 0 /* This would be: aligned-by-one (which everything is.) */
  assertf(IS_ALIGNED(virt_page,X86_PDIR_E1_SIZE / PAGESIZE),
          "Not aligned: %p (%p)",virt_page,VM_PAGE2ADDR(virt_page));
#endif
  pagedir_allocate_e3(x4);
  pagedir_allocate_e2(x4,x3);
  pagedir_allocate_e1(x4,x3,x2);
  if (perm & PAGEDIR_MAP_FUNMAP) {
#ifdef CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN
   if (E1_IDENTITY[x4][x3][x2][x1].p_addr != PAGE_ABSENT) {
    debug_printf("UNMAP[lv4](%p...%p)\n",
                 VM_PAGE2ADDR(X86_PDIR_PAGE4(x4,x3,x2,x1)),
                 VM_PAGE2ADDR(X86_PDIR_PAGE4(x4,x3,x2,x1+1))-1);
   }
#endif /* CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS_INTERN */
   E1_IDENTITY[x4][x3][x2][x1].p_addr = PAGE_ABSENT;
  } else {
   E1_IDENTITY[x4][x3][x2][x1].p_addr = edata;
   edata += X86_PDIR_E1_SIZE;
   pagedir_addperm_e2(x4,x3,x2,edata);
   pagedir_addperm_e3(x4,x3,edata);
   pagedir_addperm_e4(x4,edata);
  }
  --num_pages;
  if (!num_pages) goto done;
  ++virt_page;
 }
 __builtin_unreachable();
done:
 pagedir_merge_before(vpage_end);
 pagedir_merge_before(vpage_start);
 assert((perm & PAGEDIR_MAP_FUNMAP) ||
        VM_PAGE2ADDR(phys_page) == pagedir_translate(VM_PAGE2ADDR(virt_page)));
 assert((perm & PAGEDIR_MAP_FUNMAP) ||
        VM_PAGE2ADDR(phys_page+(vpage_end-virt_page)-1) ==
        pagedir_translate(VM_PAGE2ADDR(vpage_end-1)));
}


/* Map a single page of physical memory. */
PUBLIC void KCALL
pagedir_mapone(VIRT vm_vpage_t virt_page,
               PHYS vm_ppage_t phys_page, u16 perm) {
 /* TODO: Special optimizations. */
 pagedir_map(virt_page,1,phys_page,perm);
}


/* Translate a virtual address into its physical counterpart. */
PUBLIC PHYS vm_phys_t KCALL
pagedir_translate(VIRT vm_virt_t virt_addr) {
 ENT e; unsigned int x4,x3,x2;
 vm_vpage_t vpage = VM_ADDR2PAGE(virt_addr);
 x4 = E4_INDEX(vpage);
 assertf(E4_IDENTITY[x4].p_flag & PAGE_FPRESENT,
         "E4 at %p is not mapped",virt_addr);
 e.e3 = E3_IDENTITY[x4][x3 = E3_INDEX(vpage)];
 assertf(e.e3.p_flag & PAGE_FPRESENT,"E3 at %p is not mapped",virt_addr);
#ifndef CONFIG_NO_GIGABYTE_PAGES
 if (e.e3.p_flag & PAGE_F1GIB)
     return (e.e3.p_addr & X86_PDIR_E3_ADDRMASK) | PDIR_E3_OFFSET(virt_addr);
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
 e.e2 = E2_IDENTITY[x4][x3][x2 = E2_INDEX(vpage)];
 assertf(e.e2.p_flag & PAGE_FPRESENT,"E2 at %p is not mapped",virt_addr);
 if (e.e2.p_flag & PAGE_F2MIB)
     return (e.e2.p_addr & X86_PDIR_E2_ADDRMASK) | PDIR_E2_OFFSET(virt_addr);
 e.e1 = E1_IDENTITY[x4][x3][x2][E1_INDEX(vpage)];
 assertf(e.e1.p_flag & PAGE_FPRESENT,"E1 at %p is not mapped",virt_addr);
 return (e.e1.p_addr & X86_PDIR_E1_ADDRMASK) | PDIR_E1_OFFSET(virt_addr);
}

/* Check if the given page is mapped. */
PUBLIC ATTR_NOTHROW bool KCALL
pagedir_ismapped(vm_vpage_t vpage) {
 ENT e;
 unsigned int x4,x3,x2;
 e.e4 = E4_IDENTITY[x4 = E4_INDEX(vpage)];
 if (!(e.e4.p_flag & PAGE_FPRESENT)) return false;
 e.e3 = E3_IDENTITY[x4][x3 = E3_INDEX(vpage)];
 if (!(e.e3.p_flag & PAGE_FPRESENT)) return false;
#ifndef CONFIG_NO_GIGABYTE_PAGES
 if (e.e3.p_flag & PAGE_F1GIB) return true;
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
 e.e2 = E2_IDENTITY[x4][x3][x2 = E2_INDEX(vpage)];
 if (!(e.e2.p_flag & PAGE_FPRESENT)) return false;
 if (e.e2.p_flag & PAGE_F2MIB) return true;
 e.e1 = E1_IDENTITY[x4][x3][x2][E1_INDEX(vpage)];
 return (e.e1.p_flag & PAGE_FPRESENT) != 0;
}
PUBLIC ATTR_NOTHROW bool KCALL
pagedir_iswritable(vm_vpage_t vpage) {
 ENT e;
 unsigned int x4,x3,x2;
 e.e4 = E4_IDENTITY[x4 = E4_INDEX(vpage)];
 if (!(e.e4.p_flag & PAGE_FPRESENT)) return false;
 e.e3 = E3_IDENTITY[x4][x3 = E3_INDEX(vpage)];
 if (!(e.e3.p_flag & PAGE_FPRESENT)) return false;
#ifndef CONFIG_NO_GIGABYTE_PAGES
 if (e.e3.p_flag & PAGE_F1GIB) return (e.e3.p_flag & PAGE_FWRITE) != 0;
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
 e.e2 = E2_IDENTITY[x4][x3][x2 = E2_INDEX(vpage)];
 if (!(e.e2.p_flag & PAGE_FPRESENT)) return false;
 if (e.e2.p_flag & PAGE_F2MIB) return (e.e2.p_flag & PAGE_FWRITE) != 0;
 e.e1 = E1_IDENTITY[x4][x3][x2][E1_INDEX(vpage)];
 return (e.e1.p_flag & (PAGE_FPRESENT|PAGE_FWRITE)) == (PAGE_FPRESENT|PAGE_FWRITE);
}
PUBLIC ATTR_NOTHROW bool KCALL
pagedir_isuseraccessible(vm_vpage_t vpage) {
 ENT e;
 unsigned int x4,x3,x2;
 e.e4 = E4_IDENTITY[x4 = E4_INDEX(vpage)];
 if (!(e.e4.p_flag & PAGE_FPRESENT)) return false;
 e.e3 = E3_IDENTITY[x4][x3 = E3_INDEX(vpage)];
 if (!(e.e3.p_flag & PAGE_FPRESENT)) return false;
#ifndef CONFIG_NO_GIGABYTE_PAGES
 if (e.e3.p_flag & PAGE_F1GIB) return (e.e3.p_flag & PAGE_FUSER) != 0;
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
 e.e2 = E2_IDENTITY[x4][x3][x2 = E2_INDEX(vpage)];
 if (!(e.e2.p_flag & PAGE_FPRESENT)) return false;
 if (e.e2.p_flag & PAGE_F2MIB) return (e.e2.p_flag & PAGE_FUSER) != 0;
 e.e1 = E1_IDENTITY[x4][x3][x2][E1_INDEX(vpage)];
 return (e.e1.p_flag & (PAGE_FPRESENT|PAGE_FUSER)) == (PAGE_FPRESENT|PAGE_FUSER);
}

PUBLIC ATTR_NOTHROW bool KCALL
pagedir_haschanged(vm_vpage_t vpage) {
 ENT e;
 unsigned int x4,x3,x2;
 e.e4 = E4_IDENTITY[x4 = E4_INDEX(vpage)];
 if (!(e.e4.p_flag & PAGE_FPRESENT)) return false;
 e.e3 = E3_IDENTITY[x4][x3 = E3_INDEX(vpage)];
 if (!(e.e3.p_flag & PAGE_FPRESENT)) return false;
#ifndef CONFIG_NO_GIGABYTE_PAGES
 if (e.e3.p_flag & PAGE_F1GIB) return (e.e3.p_flag & PAGE_FDIRTY) != 0;
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
 e.e2 = E2_IDENTITY[x4][x3][x2 = E2_INDEX(vpage)];
 if (!(e.e2.p_flag & PAGE_FPRESENT)) return false;
 if (e.e2.p_flag & PAGE_F2MIB) return (e.e2.p_flag & PAGE_FDIRTY) != 0;
 e.e1 = E1_IDENTITY[x4][x3][x2][E1_INDEX(vpage)];
 return (e.e1.p_flag & (PAGE_FPRESENT|PAGE_FDIRTY)) == (PAGE_FPRESENT|PAGE_FDIRTY);
}

PUBLIC ATTR_NOTHROW void KCALL
pagedir_unsetchanged(vm_vpage_t vpage) {
 ENT e;
 unsigned int x4,x3,x2,x1;
 e.e4 = E4_IDENTITY[x4 = E4_INDEX(vpage)];
 if (!(e.e4.p_flag & PAGE_FPRESENT)) return;
 e.e3 = E3_IDENTITY[x4][x3 = E3_INDEX(vpage)];
 if (!(e.e3.p_flag & PAGE_FPRESENT)) return;
#ifndef CONFIG_NO_GIGABYTE_PAGES
 if (e.e3.p_flag & PAGE_F1GIB) {
  if (e.e3.p_flag & PAGE_FDIRTY)
      __asm__ __volatile__("andq %1, %0"
                           : "+m" (E3_IDENTITY[x4][x3].p_flag)
                           : "Zr" (~X86_PAGE_FDIRTY));
  return;
 }
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
 e.e2 = E2_IDENTITY[x4][x3][x2 = E2_INDEX(vpage)];
 if (!(e.e2.p_flag & PAGE_FPRESENT)) return;
 if (e.e2.p_flag & PAGE_F2MIB) {
  if (e.e2.p_flag & PAGE_FDIRTY)
      __asm__ __volatile__("andq %1, %0"
                           : "+m" (E2_IDENTITY[x4][x3][x2].p_flag)
                           : "Zr" (~X86_PAGE_FDIRTY));
 }
 e.e1 = E1_IDENTITY[x4][x3][x2][x1 = E1_INDEX(vpage)];
 if (!(e.e1.p_flag & PAGE_FPRESENT)) return;
 if (e.e1.p_flag & PAGE_FDIRTY) {
  __asm__ __volatile__("andq %1, %0"
                       : "+m" (E1_IDENTITY[x4][x3][x2][x1].p_flag)
                       : "Zr" (~X86_PAGE_FDIRTY));
 }
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_PAGING64_C_INL */
