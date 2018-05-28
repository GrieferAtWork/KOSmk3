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
#if !defined(NDEBUG) && 0
#define CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS 1
#endif

/* Define some shorter names for structures and macros. */
typedef union x86_pdir_ent ENT;

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
#define PAGE_FNOEXEC   X86_PAGE_FNOEXEC   /* Memory within the page cannot be executed. */
#define PAGE_ABSENT    X86_PAGE_ABSENT    /* Value found in any TIER _EXCEPT_ TIER#1 (so just TIER#2) to indicate that the page hasn't been allocated. */


/* Pagesizes of different page directory levels. */
#define E1_SIZE      X86_PDIR_E1_SIZE /* 4 KiB (Same as `PAGESIZE') */
#define E2_SIZE      X86_PDIR_E2_SIZE /* 2 MiB */
#define E3_SIZE      X86_PDIR_E3_SIZE /* 1 GiB */
#define E4_SIZE      X86_PDIR_E4_SIZE /* 512 GiB */

/* Physical address masks of different page directory levels. */
#define E1_MASK      X86_PDIR_E1_MASK /* == (~(X86_PDIR_E1_SIZE-1) & X86_PAGE_FADDR) */
#define E2_MASK      X86_PDIR_E2_MASK /* == (~(X86_PDIR_E2_SIZE-1) & X86_PAGE_FADDR) */
#define E3_MASK      X86_PDIR_E3_MASK /* == (~(X86_PDIR_E3_SIZE-1) & X86_PAGE_FADDR) */

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
#define PAGING_E4_IDENTITY_INDEX  E4_INDEX(VM_ADDR2PAGE(X86_PDIR_E1_IDENTITY_BASE))
enum{_PAGING_E4_IDENTITY_INDEX = PAGING_E4_IDENTITY_INDEX};
#undef PAGING_E4_IDENTITY_INDEX
#define PAGING_E4_IDENTITY_INDEX _PAGING_E4_IDENTITY_INDEX

STATIC_ASSERT(PAGING_E4_IDENTITY_INDEX < 512);
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
INTERN ATTR_SECTION(".bss.pagedir.kernel.share") VIRT
union x86_pdir_e3 pagedir_kernel_share_e3[PAGING_E4_SHARE_COUNT][X86_PDIR_E3_COUNT];

/* The layer-2 indirection used to describe the permanent identity
 * mapping of the last 2Gb of virtual memory, mapping to the first
 * 2Gb of physical memory, which also contain the kernel core.
 * HINT: This goes hand-in-hand with `KERNEL_CORE_BASE', which
 *       maps the kernel core, starting at -2Gb.
 * NOTE: These are only used when the host doesn't support `CPUID_80000001D_PDPE1GB' */
INTERN ATTR_SECTION(".bss.pagedir.kernel.share2") VIRT
union x86_pdir_e2 pagedir_kernel_share_e2[X86_PDIR_E3_INDEX(VM_ADDR2PAGE(KERNEL_CORE_SIZE))][X86_PDIR_E2_COUNT];

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

PRIVATE u64 x86_pageperm_matrix[0xf+1] = {
     /* Configure to always set the DIRTY and ACCESSED bits (KOS doesn't use
      * them, and us already setting them will allow the CPU to skip setting
      * them if the time comes) */
    [0]                                                                        = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FNOEXEC,
    [PAGEDIR_MAP_FEXEC]                                                        = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT,
    [PAGEDIR_MAP_FWRITE]                                                       = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT|X86_PAGE_FWRITE|X86_PAGE_FNOEXEC,
    [PAGEDIR_MAP_FWRITE|PAGEDIR_MAP_FEXEC]                                     = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT|X86_PAGE_FWRITE,
    [PAGEDIR_MAP_FREAD]                                                        = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT|X86_PAGE_FNOEXEC,
    [PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FEXEC]                                      = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT,
    [PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE]                                     = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT|X86_PAGE_FWRITE|X86_PAGE_FNOEXEC,
    [PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE|PAGEDIR_MAP_FEXEC]                   = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT|X86_PAGE_FWRITE,
    [PAGEDIR_MAP_FUSER]                                                        = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FNOEXEC,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FEXEC]                                      = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FWRITE]                                     = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT|X86_PAGE_FWRITE|X86_PAGE_FNOEXEC,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FEXEC|PAGEDIR_MAP_FWRITE]                   = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT|X86_PAGE_FWRITE,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD]                                      = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT|X86_PAGE_FNOEXEC,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FEXEC]                    = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE]                   = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT|X86_PAGE_FWRITE|X86_PAGE_FNOEXEC,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE|PAGEDIR_MAP_FEXEC] = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT|X86_PAGE_FWRITE,
};

PRIVATE u64 x86_page_global = PAGE_FGLOBAL;
#ifndef CONFIG_NO_GIGABYTE_PAGES
PRIVATE unsigned int x86_paging_features = 0;
#define PAGING_FEATURE_1GIB_PAGES  0x0001 /* Host supports 1GIB pages. */
#endif /* !CONFIG_NO_GIGABYTE_PAGES */


INTERN ATTR_FREETEXT void KCALL x86_configure_paging(void) {
 struct cpu_cpuid const *feat = &CPU_FEATURES;
 if (!(feat->ci_1d & CPUID_1D_PGE))
       x86_page_global = 0;
#ifndef CONFIG_NO_GIGABYTE_PAGES
 if (feat->ci_80000001d & CPUID_80000001D_PDPE1GB) {
  debug_printf(FREESTR("[X86] Enable 1GIB pages\n"));
  x86_paging_features |= PAGING_FEATURE_1GIB_PAGES;
 }
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
}










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
    page_free(e2_vector[k].p_addr & PAGE_FADDR,1);
   }
   /* Free the E2-vector. */
   page_free(e3_vector[j].p_addr & PAGE_FADDR,1);
  }
  /* Free the E3-vector. */
  page_free(e.e4.p_data & PAGE_FADDR,1);
 }
 /* Switch back to the real page directory if we had to change it. */
 if (did_switch) {
  pagedir_set((pagedir_t *)THIS_VM->vm_physdir);
  /* Re-enable preemption if we disabled it momentarily. */
  PREEMPTION_POP(was);
 }
}


PRIVATE void KCALL
pagedir_split_before(VIRT vm_vpage_t virt_page) {
 /* TODO */
}

PRIVATE void KCALL
pagedir_merge_before(VIRT vm_vpage_t virt_page) {
 /* TODO */
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
 u64 edata;
 /* Check for special case: no mapping. */
 if (!num_pages) return;
#if 1
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
 assertf(virt_page < VM_VPAGE_MAX,"virt_page = %p",virt_page);
 assertf((perm&PAGEDIR_MAP_FUNMAP) || phys_page < VM_PPAGE_MAX,"phys_page = %p",phys_page);
 assert(virt_page+num_pages >= virt_page);
 assert((perm&PAGEDIR_MAP_FUNMAP) || phys_page+num_pages >= phys_page);
 assertf(virt_page+num_pages < VM_VPAGE_MAX,
         "virt_page = %p\n"
         "num_pages = %p\n",
         virt_page,num_pages);
 assert((perm&PAGEDIR_MAP_FUNMAP) || phys_page+num_pages < VM_PPAGE_MAX);
 edata = (u64)VM_PAGE2ADDR(phys_page) | x86_pageperm_matrix[perm & 0xf];
 if (virt_page >= KERNEL_BASE_PAGE)
     edata |= x86_page_global; /* Kernel-share mapping. */

 pagedir_split_before(virt_page);
 pagedir_split_before(vpage_end);

 /* TODO */

 pagedir_merge_before(vpage_end);
 pagedir_merge_before(virt_page);
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
     return (e.e3.p_addr & X86_PDIR_E3_MASK) | PDIR_E3_OFFSET(virt_addr);
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
 e.e2 = E2_IDENTITY[x4][x3][x2 = E2_INDEX(vpage)];
 assertf(e.e2.p_flag & PAGE_FPRESENT,"E2 at %p is not mapped",virt_addr);
 if (e.e2.p_flag & PAGE_F2MIB)
     return (e.e2.p_addr & X86_PDIR_E2_MASK) | PDIR_E2_OFFSET(virt_addr);
 e.e1 = E1_IDENTITY[x4][x3][x2][E1_INDEX(vpage)];
 assertf(e.e1.p_flag & PAGE_FPRESENT,"E1 at %p is not mapped",virt_addr);
 return (e.e1.p_addr & X86_PDIR_E1_MASK) | PDIR_E1_OFFSET(virt_addr);
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
