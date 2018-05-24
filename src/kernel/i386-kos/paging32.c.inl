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
#ifndef GUARD_KERNEL_I386_KOS_PAGING32_C_INL
#define GUARD_KERNEL_I386_KOS_PAGING32_C_INL 1
#define _KOS_SOURCE 1
#define __VM_INTERNAL_EXCLUDE_PAGEDIR 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>
#include <hybrid/align.h>
#include <hybrid/minmax.h>
#include <kernel/paging.h>
#include <kernel/vm.h>
#include <string.h>
#include <kernel/debug.h>
#include <i386-kos/interrupt.h>

DECL_BEGIN

#undef CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS
#if !defined(NDEBUG) && 0
#define CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS 1
#endif

/* The starting VEC2-index (in `x86_pdir::p_e2') and amount
 * of continuous indices thereafter of the kernel-share segment.
 * That is the segment where the kernel itself resides at, which
 * is then mapped again in all other page directories. */
#define VEC2_SHARE_BEGIN     X86_PDIR_VEC2INDEX(KERNEL_BASE)
#define VEC2_SHARE_SIZE     (VEC2_IDENTITY_BEGIN-VEC2_SHARE_BEGIN)

/* Similar to the SHARE indices, but for the identity mapping instead. */
#define VEC2_IDENTITY_BEGIN  X86_PDIR_VEC2INDEX(X86_PDIR_E1_IDENTITY_BASE)
#define VEC2_IDENTITY_SIZE  (1024-VEC2_IDENTITY_BEGIN)

/* Allocate BSS memory for the initial shared+identity mapping
 * that will later be shared with, and re-appear in all other
 * page directories (except for the identity page)
 * NOTE: This buffer is quite large (1Mb), but we'd need
 *       to allocate it sooner or later, no matter what. */
INTERN ATTR_SECTION(".bss.pagedir.kernel.share") VIRT
union x86_pdir_e1 pagedir_kernel_share[VEC2_SHARE_SIZE+VEC2_IDENTITY_SIZE][1024];

/* The kernel's builtin page directory (initialized in `boot32.S') */
//PUBLIC ATTR_SECTION(".bss.pagedir.kernel") VIRT pagedir_t pagedir_kernel;

INTERN ATTR_SECTION(".data.pervm.head")
struct vm vm_kernel_head = {
#if 0
    .vm_pagedir = {
        .p_e2 = {
#define MAP_P1_VECTOR(vidx,pidx) \
            [vidx] = { (uintptr_t)&pagedir_kernel_share[pidx] - KERNEL_BASE + (X86_PAGE_FGLOBAL | X86_PAGE_FDIRTY | X86_PAGE_FACCESSED | X86_PAGE_FWRITE | X86_PAGE_FPRESENT) }
#define MAP_P1_VECTOR_X16(vidx,pidx) \
            MAP_P1_VECTOR(vidx+0,pidx+0), MAP_P1_VECTOR(vidx+1,pidx+1), \
            MAP_P1_VECTOR(vidx+2,pidx+2), MAP_P1_VECTOR(vidx+3,pidx+3), \
            MAP_P1_VECTOR(vidx+4,pidx+4), MAP_P1_VECTOR(vidx+5,pidx+5), \
            MAP_P1_VECTOR(vidx+6,pidx+6), MAP_P1_VECTOR(vidx+7,pidx+7), \
            MAP_P1_VECTOR(vidx+8,pidx+8), MAP_P1_VECTOR(vidx+9,pidx+9), \
            MAP_P1_VECTOR(vidx+10,pidx+10), MAP_P1_VECTOR(vidx+11,pidx+11), \
            MAP_P1_VECTOR(vidx+12,pidx+12), MAP_P1_VECTOR(vidx+13,pidx+14), \
            MAP_P1_VECTOR(vidx+14,pidx+14), MAP_P1_VECTOR(vidx+15,pidx+15)
            MAP_P1_VECTOR_X16(0x0000,0x0000),
            MAP_P1_VECTOR_X16(0x0010,0x0010),
            MAP_P1_VECTOR_X16(0x0020,0x0020),
            MAP_P1_VECTOR_X16(0x0030,0x0030),
            MAP_P1_VECTOR_X16(0x0040,0x0040),
            MAP_P1_VECTOR_X16(0x0050,0x0050),
            MAP_P1_VECTOR_X16(0x0060,0x0060),
            MAP_P1_VECTOR_X16(0x0070,0x0070),
            MAP_P1_VECTOR_X16(0x0080,0x0080),
            MAP_P1_VECTOR_X16(0x0090,0x0090),
            MAP_P1_VECTOR_X16(0x00a0,0x00a0),
            MAP_P1_VECTOR_X16(0x00b0,0x00b0),
            MAP_P1_VECTOR_X16(0x00c0,0x00c0),
            MAP_P1_VECTOR_X16(0x00d0,0x00d0),
            MAP_P1_VECTOR_X16(0x00e0,0x00e0),
            MAP_P1_VECTOR_X16(0x00f0,0x00f0),

            MAP_P1_VECTOR_X16(0x0300,0x0000),
            MAP_P1_VECTOR_X16(0x0310,0x0010),
            MAP_P1_VECTOR_X16(0x0320,0x0020),
            MAP_P1_VECTOR_X16(0x0330,0x0030),
            MAP_P1_VECTOR_X16(0x0340,0x0040),
            MAP_P1_VECTOR_X16(0x0350,0x0050),
            MAP_P1_VECTOR_X16(0x0360,0x0060),
            MAP_P1_VECTOR_X16(0x0370,0x0070),
            MAP_P1_VECTOR_X16(0x0380,0x0080),
            MAP_P1_VECTOR_X16(0x0390,0x0090),
            MAP_P1_VECTOR_X16(0x03a0,0x00a0),
            MAP_P1_VECTOR_X16(0x03b0,0x00b0),
            MAP_P1_VECTOR_X16(0x03c0,0x00c0),
            MAP_P1_VECTOR_X16(0x03d0,0x00d0),
            MAP_P1_VECTOR_X16(0x03e0,0x00e0),
            MAP_P1_VECTOR_X16(0x03f0,0x00f0)
        }
    },
#endif
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

/* Make sure that our calculations are correct and that the
 * identity mapping requires 1/1024 of the entire address space. */
STATIC_ASSERT(VEC2_IDENTITY_SIZE == 1);

/* Code assumes that ZERO-initialization can be used to mark pages as absent. */
STATIC_ASSERT(X86_PAGE_ABSENT == 0);

INTDEF bool x86_config_enable_pse;

/* The memory zone used to allocate memory for page directories. */
#define MZONE_PAGING  X86_MZONE_4GB

/* Initialize the given page directory.
 * The caller is required to allocate the page directory
 * controller itself, which must be aligned and sized
 * according to `PAGEDIR_ALIGN' and `PAGEDIR_SIZE'.
 * @throw E_BADALLOC: Not enough available memory. */
PUBLIC void KCALL
pagedir_init(VIRT pagedir_t *__restrict self,
             PHYS vm_phys_t phys_self) {
 assert(IS_ALIGNED((uintptr_t)self,PAGESIZE));
 assert(IS_ALIGNED(phys_self,PAGESIZE));

 /* Map all pages before the share-segment as absent. */
 memsetl(self->p_e2,X86_PAGE_ABSENT,VEC2_SHARE_BEGIN);
 /* Copy P2 pointers that are shared with the kernel.
  * NOTE: All of these have been initialized as P1 tables apart
  *       of the `pagedir_kernel_share' data block by `boot32.S' */
 memcpyl(&self->p_e2[VEC2_SHARE_BEGIN],
         &pagedir_kernel.p_e2[VEC2_SHARE_BEGIN],
          VEC2_SHARE_SIZE);
 /* Create the identity mapping */
 self->p_e2[VEC2_IDENTITY_BEGIN].p_data = ((u32)phys_self |
                                           (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED |
                                            X86_PAGE_FWRITE | X86_PAGE_FPRESENT));
}

PUBLIC void KCALL
pagedir_fini(VIRT pagedir_t *__restrict self) {
 unsigned int i;
 /* Free all dynamically allocated E1
  * vectors below the kernel-share segment. */
 for (i = 0; i < VEC2_SHARE_BEGIN; ++i) {
  union x86_pdir_e2 e2 = self->p_e2[i];
  if (!(e2.p_flag&X86_PAGE_F4MIB) &&
       (e2.p_addr&X86_PAGE_FADDR) != X86_PAGE_ABSENT)
        page_free(VM_ADDR2PAGE(e2.p_addr),1);
 }
}



/* Ensure that the virtual address space in `pdir'
 * is split before the given virtual page index.
 * This means that if the E1 vector of the E2 entry at `virt_page'
 * hasn't been allocated yet, allocate it now and fill it with the
 * required data. */
PRIVATE void KCALL
pagedir_split_before(VIRT vm_vpage_t virt_page) {
 VIRT union x86_pdir_e2 *e2; u32 e2_data;
 VIRT union x86_pdir_e1 *e1_vector; unsigned int i;
 assert(virt_page <= VM_VPAGE_MAX);
 if (X86_PDIR_VEC1INDEX_VPAGE(virt_page) == 0)
     return; /* Mapping is at the start of a E1-vector */
 e2 = &X86_PDIR_E2_IDENTITY[X86_PDIR_VEC2INDEX_VPAGE(virt_page)];
 e2_data = e2->p_data;
 if (e2_data&X86_PAGE_F4MIB) {
  /* Convert a 4MIB page to a E1-page vector. */
  assertf(virt_page < X86_KERNEL_BASE_PAGE,
          "Kernel-share pages must not be 4MIB pages");
  assertf(x86_config_enable_pse,"4MIB Page without PSE support");
  /* Start by mapping the page in the page directory identity mapping. */
  e2_data &= ~(X86_PAGE_F4MIB);
  e2->p_addr = ((u32)VM_PAGE2ADDR(page_malloc(1,MZONE_PAGING)) |
                (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED |
                 X86_PAGE_FWRITE | X86_PAGE_FPRESENT));
  /* Load the virtual address of the E1 vector that we've just mapped. */
  e1_vector = X86_PDIR_E1_IDENTITY[X86_PDIR_VEC2INDEX_VPAGE(virt_page)];
  /* Synchronize the page at the E1 vector (to ensure that it's been mapped to the new address). */
  pagedir_syncone(VM_ADDR2PAGE((uintptr_t)e1_vector));
  COMPILER_BARRIER();
  /* Fill in the new E1 vector. */
  for (i = 0; i < 1024; ++i) {
   e1_vector[i].p_data = e2_data;
   e2_data            += PAGESIZE;
  }
  COMPILER_WRITE_BARRIER();
  /* Re-add bits. */
  e2->p_addr |= (e2_data & X86_PAGE_FMASK);
  COMPILER_WRITE_BARRIER();
 } else if ((e2_data&X86_PAGE_FADDR) == X86_PAGE_ABSENT) {
  assertf(virt_page < X86_KERNEL_BASE_PAGE,
          "All kernel-share pages must be pre-allocated, "
          "but page %p...%p isn't",
          VM_PAGE2ADDR(virt_page),
          VM_PAGE2ADDR(virt_page)+PAGESIZE-1);
  e2->p_addr = ((u32)VM_PAGE2ADDR(page_malloc(1,MZONE_PAGING)) |
                (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED |
                 X86_PAGE_FWRITE | X86_PAGE_FPRESENT));
  e1_vector = X86_PDIR_E1_IDENTITY[X86_PDIR_VEC2INDEX_VPAGE(virt_page)];
  pagedir_syncone(VM_ADDR2PAGE((uintptr_t)e1_vector));
  COMPILER_BARRIER();
  /* Clear out the E1 vector. */
  memsetl(e1_vector,0,1024);
  COMPILER_WRITE_BARRIER();
 }
}

PRIVATE void KCALL
pagedir_merge_before(VIRT vm_vpage_t virt_page) {
 VIRT union x86_pdir_e2 *e2; u32 e2_data; u32 expected_mapping;
 VIRT union x86_pdir_e1 *e1_vector; unsigned int i;
 if (X86_PDIR_VEC1INDEX_VPAGE(virt_page) == 0)
     return; /* Mapping is at the start of a E1-vector */
 if (virt_page >= X86_KERNEL_BASE_PAGE)
     return; /* Mappings above the kernel-base must not have their E1 vectors merged.
              * (We'd loose the indirection allowing the kernel-share segment) */
 if ((X86_PDIR_E2_IDENTITY[X86_PDIR_VEC2INDEX_VPAGE(virt_page)].p_data&
     (X86_PAGE_FPRESENT|X86_PAGE_F4MIB)) != X86_PAGE_FPRESENT)
      return; /* Vector has already been merged. */
 e1_vector        = X86_PDIR_E1_IDENTITY[X86_PDIR_VEC2INDEX_VPAGE(virt_page)];
 expected_mapping = e1_vector[0].p_data & ~(X86_PAGE_FDIRTY|X86_PAGE_FACCESSED);
 if (expected_mapping & X86_PAGE_FPRESENT) {
  /* Check if there is any page that doesn't fit a linear mapping. */
  if (!x86_config_enable_pse)
      return; /* Without PSE, we can't actually make use of `X86_PAGE_F4MIB' */
  for (i = 0; i < 1024; ++i) {
   if ((e1_vector[i].p_addr & ~(X86_PAGE_FDIRTY|X86_PAGE_FACCESSED)) !=
        expected_mapping) return; /* Cannot combine */
   expected_mapping += PAGESIZE;
  }
  expected_mapping -= 1024*PAGESIZE;
  expected_mapping |= (X86_PAGE_F4MIB|X86_PAGE_FDIRTY|X86_PAGE_FACCESSED);
 } else {
  /* Check if there is any present page. */
  for (i = 0; i < 1024; ++i) {
   if (e1_vector[i].p_addr & X86_PAGE_FPRESENT)
       return; /* Cannot combine */
  }
  expected_mapping = X86_PAGE_ABSENT;
 }
 e2 = &X86_PDIR_E2_IDENTITY[X86_PDIR_VEC2INDEX_VPAGE(virt_page)];
 e2_data = e2->p_data;
 /* Convert the mapping into a 4MIB page. */
 e2->p_data = expected_mapping;
 COMPILER_WRITE_BARRIER();
 /* Free the E1 vector page (now no longer used) */
 page_free(VM_ADDR2PAGE(e2_data),1);
}


PRIVATE u16 const x86_pageperm_matrix[0xf+1] = {
     /* Configure to always set the DIRTY and ACCESSED bits (KOS doesn't use
      * them, and us already setting them will allow the CPU to skip setting
      * them if the time comes) */
    [0]                                                                        = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED,
    [PAGEDIR_MAP_FEXEC]                                                        = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT,
    [PAGEDIR_MAP_FWRITE]                                                       = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT|X86_PAGE_FWRITE,
    [PAGEDIR_MAP_FEXEC|PAGEDIR_MAP_FWRITE]                                     = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT|X86_PAGE_FWRITE,
    [PAGEDIR_MAP_FREAD]                                                        = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT,
    [PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FEXEC]                                      = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT,
    [PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE]                                     = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT|X86_PAGE_FWRITE,
    [PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE|PAGEDIR_MAP_FEXEC]                   = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT|X86_PAGE_FWRITE,
    [PAGEDIR_MAP_FUSER]                                                        = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FEXEC]                                      = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FWRITE]                                     = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT|X86_PAGE_FWRITE,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FEXEC|PAGEDIR_MAP_FWRITE]                   = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT|X86_PAGE_FWRITE,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD]                                      = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FEXEC]                    = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE]                   = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT|X86_PAGE_FWRITE,
    [PAGEDIR_MAP_FUSER|PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE|PAGEDIR_MAP_FEXEC] = X86_PAGE_FDIRTY|X86_PAGE_FACCESSED|X86_PAGE_FUSER|X86_PAGE_FPRESENT|X86_PAGE_FWRITE,
};


PUBLIC void KCALL
pagedir_map(VIRT vm_vpage_t virt_page, size_t num_pages,
            PHYS vm_ppage_t phys_page, u16 perm) {
 unsigned int e2_index; u32 edata;
 unsigned int e1_begin,i;
 VIRT union x86_pdir_e1 *e1_vector;
 vm_vpage_t vpage_start,vpage_end;
 /* Check for special case: no mapping. */
 if (!num_pages) return;
#if 1
 if ((perm & PAGEDIR_MAP_FUSER) &&
      virt_page >= X86_KERNEL_BASE_PAGE)
      asm("int3");
#endif
#if 1
 if (num_pages == 1) {
  /* Optimization: Use the faster function when only mapping a single page. */
  pagedir_mapone(virt_page,phys_page,perm);
  return;
 }
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
#if 0
 if unlikely(virt_page           <= 0xC0007 &&
             virt_page+num_pages >  0xC0007) {
  __asm__("int3\ncli");
  for (;;) { asm("hlt"); }
 }
#endif

 vpage_start = virt_page;
 vpage_end   = virt_page+num_pages;
 assertf(virt_page < VM_VPAGE_MAX,"virt_page = %p",virt_page);
 assertf(phys_page < VM_PPAGE_MAX,"phys_page = %p",phys_page);
 assert(virt_page+num_pages >= virt_page);
 assert(phys_page+num_pages >= phys_page);
 assertf(virt_page+num_pages < VM_VPAGE_MAX,
         "virt_page = %p\n"
         "num_pages = %p\n",
         virt_page,num_pages);
 assert(phys_page+num_pages < VM_PPAGE_MAX);
 edata = (u32)VM_PAGE2ADDR(phys_page) | x86_pageperm_matrix[perm & 0xf];
 if (virt_page >= X86_KERNEL_BASE_PAGE)
     edata |= X86_PAGE_FGLOBAL; /* Kernel-share mapping. */
 /* Create splits in the virtual address space. */
 pagedir_split_before(vpage_start);
 pagedir_split_before(vpage_end);

 /* Figure out the lowest and greatest E2 vectors affected. */
 e2_index = X86_PDIR_VEC2INDEX_VPAGE(virt_page);
 e1_begin = X86_PDIR_VEC1INDEX_VPAGE(virt_page);
 assert(e1_begin < 1024);
 if (e1_begin != 0) {
  unsigned int e1_count;
  /* Do a partial map in `e2_index' */
  e1_count = 1024 - e1_begin;
  if (e1_count > num_pages)
      e1_count = num_pages;
  assert(e1_count != 0);
  assertf(!(X86_PDIR_E2_IDENTITY[e2_index].p_flag&X86_PAGE_F4MIB),
            "This should have been cleared by `pagedir_split_before(vpage_start=%p...%p)'\n"
            "X86_PDIR_E2_IDENTITY[e2_index=%u].p_flag = %p",
          VM_PAGE2ADDR(vpage_start),VM_PAGE2ADDR(vpage_start)+PAGESIZE-1,
          e2_index,X86_PDIR_E2_IDENTITY[e2_index].p_flag);
  assertf(X86_PDIR_E2_IDENTITY[e2_index].p_flag&X86_PAGE_FPRESENT,
          "This should have been done by `pagedir_split_before(vpage_start=%p...%p)'\n"
          "X86_PDIR_E2_IDENTITY[e2_index=%u].p_flag = %p",
          VM_PAGE2ADDR(vpage_start),VM_PAGE2ADDR(vpage_start)+PAGESIZE-1,
          e2_index,X86_PDIR_E2_IDENTITY[e2_index].p_flag);
  assertf(X86_PDIR_E2_IDENTITY[e2_index].p_flag&X86_PAGE_FWRITE,
          "This should have been done by `pagedir_split_before(vpage_start=%p...%p)'\n"
          "X86_PDIR_E2_IDENTITY[e2_index=%u].p_flag = %p",
          VM_PAGE2ADDR(vpage_start),VM_PAGE2ADDR(vpage_start)+PAGESIZE-1,
          e2_index,X86_PDIR_E2_IDENTITY[e2_index].p_flag);
  assertf((X86_PDIR_E2_IDENTITY[e2_index].p_flag&X86_PAGE_FADDR) != X86_PAGE_ABSENT,
           "This should have been allocated by `pagedir_split_before(vpage_start=%p...%p)'\n"
           "X86_PDIR_E2_IDENTITY[e2_index=%u].p_flag = %p",
          VM_PAGE2ADDR(vpage_start),VM_PAGE2ADDR(vpage_start)+PAGESIZE-1,
          e2_index,X86_PDIR_E2_IDENTITY[e2_index].p_flag);
  e1_vector = X86_PDIR_E1_IDENTITY[e2_index];
  /* Fill in the vector. */
  if (perm & PAGEDIR_MAP_FUNMAP) {
   memsetl(e1_vector+e1_begin,0,e1_count);
  } else {
   unsigned int e1_end;
   e1_end = e1_begin+e1_count;
   for (i = e1_begin; i < e1_end; ++i) {
    e1_vector[i].p_data = edata;
    edata += PAGESIZE;
   }
   /* Add permission bits to the E2 table. */
   X86_PDIR_E2_IDENTITY[e2_index].p_data |= edata & X86_PAGE_FMASK;
  }
  num_pages -= e1_count;
  if (!num_pages) goto done;
  ++e2_index;
  virt_page += e1_count;
 }
 while (num_pages >= 1024) {
  VIRT union x86_pdir_e2 *e2_entry; u32 e2_data;
  /* Map entire E2 tables (Using `X86_PAGE_F4MIB' if available). */
  e2_entry = &X86_PDIR_E2_IDENTITY[e2_index];
  e2_data  = e2_entry->p_data;
#if 0 /* XXX: This doesn't seem to be working... */
  if (x86_config_enable_pse &&
      e2_index < VEC2_SHARE_BEGIN) {
   /* Override the E2 entries data. */
   e2_entry->p_data = (perm & PAGEDIR_MAP_FUNMAP) ? X86_PAGE_ABSENT : (edata|X86_PAGE_F4MIB);
   COMPILER_WRITE_BARRIER();
   /* Free a previously allocated E1 vector at this location. */
   if (!(e2_data&X86_PAGE_F4MIB) &&
        (e2_data&X86_PAGE_FADDR) != X86_PAGE_ABSENT)
         page_free(VM_ADDR2PAGE(e2_data),1);
   edata += 1024*PAGESIZE;
  } else
#endif
  if (perm & PAGEDIR_MAP_FUNMAP) {
   if (e2_index < VEC2_SHARE_BEGIN) {
    /* When unmapping, we can simply get rid of this E1 vector. */
    e2_entry->p_data = X86_PAGE_ABSENT;
    COMPILER_WRITE_BARRIER();
    /* Free the old table if one was allocated before. */
    if ((e2_data&X86_PAGE_FADDR) != X86_PAGE_ABSENT)
         page_free(VM_ADDR2PAGE(e2_data),1);
   } else {
    /* Unmap each entry */
    e1_vector = X86_PDIR_E1_IDENTITY[e2_index];
    memsetl(e1_vector,0,1024);
   }
  } else {
   /* Without PSE, we must manually map this entry. */
   assertf(!(e2_data&X86_PAGE_F4MIB),"PSE isn't enabled. The 4MIB flag is illegal");
   assert(!(edata&X86_PAGE_F4MIB));
   e1_vector = X86_PDIR_E1_IDENTITY[e2_index];
   if ((e2_data&X86_PAGE_FADDR) == X86_PAGE_ABSENT) {
    /* Must allocate the vector. */
    e2_entry->p_data = (VM_PAGE2ADDR(page_malloc(1,MZONE_PAGING)) |
                       (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED |
                        X86_PAGE_FWRITE | X86_PAGE_FPRESENT));
    COMPILER_WRITE_BARRIER();
    /* Synchronize to ensure that the new map appears in `e1_vector' */
    pagedir_syncone(VM_ADDR2PAGE((uintptr_t)e1_vector));
    COMPILER_BARRIER();
   }
   /* Fill in the vector. */
   for (i = 0; i < 1024; ++i) {
    e1_vector[i].p_data = edata;
    edata += PAGESIZE;
   }
   /* Set permissions flags. */
   e2_entry->p_data |= edata&X86_PAGE_FMASK;
   COMPILER_WRITE_BARRIER();
  }
  /* Advance to the next part. */
  num_pages -= 1024;
  ++e2_index;
 }

 /* Map the remainder. */
 if (num_pages) {
  assertf(!(X86_PDIR_E2_IDENTITY[e2_index].p_flag&X86_PAGE_F4MIB),
          "This should have been cleared by `pagedir_split_before(vpage_end)'");
  assertf((X86_PDIR_E2_IDENTITY[e2_index].p_flag&X86_PAGE_FADDR) != X86_PAGE_ABSENT,
          "This should have been allocated by `pagedir_split_before(vpage_end)'");
  e1_vector = X86_PDIR_E1_IDENTITY[e2_index];
  /* Fill in the vector. */
  if (perm & PAGEDIR_MAP_FUNMAP) {
   memsetl(e1_vector,0,num_pages);
  } else {
   for (i = 0; i < num_pages; ++i) {
    e1_vector[i].p_data = edata;
    edata += PAGESIZE;
   }
   /* Add permission bits to the E2 table. */
   X86_PDIR_E2_IDENTITY[e2_index].p_data |= edata & X86_PAGE_FMASK;
  }
 }

done:
 /* Merge the splits from before. */
 pagedir_merge_before(vpage_end);
 pagedir_merge_before(vpage_start);
}
PUBLIC void KCALL
pagedir_mapone(VIRT vm_vpage_t virt_page,
               PHYS vm_ppage_t phys_page, u16 perm) {
 unsigned int e2_index; u32 edata;
 unsigned int e1_begin;
 VIRT union x86_pdir_e1 *e1_vector;
 vm_vpage_t vpage_start;
#ifdef CONFIG_LOG_PAGEDIRECTORY_MAPPING_CALLS
 debug_printf("pagedir_map(%p...%p,%p...%p,%s%c%c%c%c)\n",
              VM_PAGE2ADDR(virt_page),
              VM_PAGE2ADDR(virt_page+1)-1,
              VM_PAGE2ADDR(phys_page),
              VM_PAGE2ADDR(phys_page+1)-1,
              perm&PAGEDIR_MAP_FUNMAP ? "DELETE," : "",
              perm&PAGEDIR_MAP_FEXEC ? 'X' : '-',
              perm&PAGEDIR_MAP_FWRITE ? 'W' : '-',
              perm&PAGEDIR_MAP_FREAD ? 'R' : '-',
              perm&PAGEDIR_MAP_FUSER ? 'U' : '-');
#endif
 vpage_start = virt_page;
 assertf(virt_page < VM_VPAGE_MAX,"virt_page = %p",virt_page);
 assertf(phys_page < VM_PPAGE_MAX,"phys_page = %p",phys_page);
 assert(virt_page+1 >= virt_page);
 assert(phys_page+1 >= phys_page);
 assert(virt_page+1 < VM_VPAGE_MAX);
 assert(phys_page+1 < VM_PPAGE_MAX);
 edata = (u32)VM_PAGE2ADDR(phys_page) | x86_pageperm_matrix[perm & 0xf];
 if (virt_page >= X86_KERNEL_BASE_PAGE)
     edata |= X86_PAGE_FGLOBAL; /* Kernel-share mapping. */
 /* Create splits in the virtual address space. */
 pagedir_split_before(vpage_start);
 pagedir_split_before(vpage_start+1);

 /* Figure out the lowest and greatest E2 vectors affected. */
 e2_index = X86_PDIR_VEC2INDEX_VPAGE(virt_page);
 e1_begin = X86_PDIR_VEC1INDEX_VPAGE(virt_page);
 assert(e1_begin < 1024);
 assertf(!(X86_PDIR_E2_IDENTITY[e2_index].p_flag&X86_PAGE_F4MIB),
           "This should have been cleared by `pagedir_split_before(vpage_start=%p...%p)'\n"
           "X86_PDIR_E2_IDENTITY[e2_index=%u].p_flag = %p",
         VM_PAGE2ADDR(vpage_start),VM_PAGE2ADDR(vpage_start)+PAGESIZE-1,
         e2_index,X86_PDIR_E2_IDENTITY[e2_index].p_flag);
 assertf((X86_PDIR_E2_IDENTITY[e2_index].p_flag&X86_PAGE_FADDR) != X86_PAGE_ABSENT,
          "This should have been allocated by `pagedir_split_before(vpage_start=%p...%p)'\n"
          "X86_PDIR_E2_IDENTITY[e2_index=%u].p_flag = %p",
         VM_PAGE2ADDR(vpage_start),VM_PAGE2ADDR(vpage_start)+PAGESIZE-1,
         e2_index,X86_PDIR_E2_IDENTITY[e2_index].p_flag);
 e1_vector = X86_PDIR_E1_IDENTITY[e2_index];
 /* Fill in the vector. */
 if (perm & PAGEDIR_MAP_FUNMAP) {
  e1_vector[e1_begin].p_data = 0;
 } else {
  e1_vector[e1_begin].p_data = edata;
  /* Add permission bits to the E2 table. */
  X86_PDIR_E2_IDENTITY[e2_index].p_data |= edata & X86_PAGE_FMASK;
 }
 /* Merge the splits from before. */
 pagedir_merge_before(vpage_start+1);
 pagedir_merge_before(vpage_start);
}

PUBLIC PHYS vm_phys_t KCALL
pagedir_translate(VIRT vm_virt_t virt_addr) {
 u32 result;
 result = X86_PDIR_E1_IDENTITY[X86_PDIR_VEC2INDEX(virt_addr)]
                              [X86_PDIR_VEC1INDEX(virt_addr)].
          p_data;
 return (result & X86_PAGE_FADDR) | X86_PDIR_PAGEINDEX(virt_addr);
}

PUBLIC ATTR_NOTHROW bool KCALL
pagedir_ismapped(vm_vpage_t virt_addr) {
 u32 temp; unsigned int vec2;
 vec2 = X86_PDIR_VEC2INDEX_VPAGE(virt_addr);
 temp = X86_PDIR_E2_IDENTITY[vec2].p_data;
 if (!(temp & X86_PAGE_FPRESENT)) return false;
 if (temp & X86_PAGE_F4MIB) return true;
 return !!(X86_PDIR_E1_IDENTITY[vec2][X86_PDIR_VEC1INDEX_VPAGE(virt_addr)].p_flag & X86_PAGE_FPRESENT);
}
PUBLIC ATTR_NOTHROW bool KCALL
pagedir_iswritable(vm_vpage_t virt_addr) {
 u32 temp; unsigned int vec2;
 vec2 = X86_PDIR_VEC2INDEX_VPAGE(virt_addr);
 temp = X86_PDIR_E2_IDENTITY[vec2].p_data;
 if (!(temp & X86_PAGE_FPRESENT)) return false;
 if (temp & X86_PAGE_F4MIB) return temp & X86_PAGE_FWRITE;
 return (X86_PDIR_E1_IDENTITY[vec2][X86_PDIR_VEC1INDEX_VPAGE(virt_addr)].p_flag &
        (X86_PAGE_FWRITE|X86_PAGE_FPRESENT)) == (X86_PAGE_FWRITE|X86_PAGE_FPRESENT);
}
PUBLIC ATTR_NOTHROW bool KCALL
pagedir_isuseraccessible(vm_vpage_t virt_addr) {
 u32 temp; unsigned int vec2;
 vec2 = X86_PDIR_VEC2INDEX_VPAGE(virt_addr);
 temp = X86_PDIR_E2_IDENTITY[vec2].p_data;
 if (!(temp & X86_PAGE_FPRESENT)) return false;
 if (temp & X86_PAGE_F4MIB) return temp & X86_PAGE_FUSER;
 return (X86_PDIR_E1_IDENTITY[vec2][X86_PDIR_VEC1INDEX_VPAGE(virt_addr)].p_flag &
        (X86_PAGE_FUSER|X86_PAGE_FPRESENT)) == (X86_PAGE_FUSER|X86_PAGE_FPRESENT);
}

PUBLIC ATTR_NOTHROW bool KCALL
pagedir_haschanged(vm_vpage_t virt_addr) {
 u32 temp; unsigned int vec2;
 vec2 = X86_PDIR_VEC2INDEX_VPAGE(virt_addr);
 temp = X86_PDIR_E2_IDENTITY[vec2].p_data;
 if (!(temp & X86_PAGE_FPRESENT)) return false;
 if (temp & X86_PAGE_F4MIB)
     return !!(temp & X86_PAGE_FDIRTY);
 return (X86_PDIR_E1_IDENTITY[vec2][X86_PDIR_VEC1INDEX_VPAGE(virt_addr)].p_flag &
        (X86_PAGE_FDIRTY|X86_PAGE_FPRESENT)) == (X86_PAGE_FDIRTY|X86_PAGE_FPRESENT);
}
PUBLIC ATTR_NOTHROW void KCALL
pagedir_unsetchanged(vm_vpage_t virt_addr) {
 u32 temp,*e1; unsigned int vec2;
 vec2 = X86_PDIR_VEC2INDEX_VPAGE(virt_addr);
 temp = X86_PDIR_E2_IDENTITY[vec2].p_data;
 if (!(temp & X86_PAGE_FPRESENT)) return;
 if (temp & X86_PAGE_F4MIB) {
  if (temp & X86_PAGE_FDIRTY)
      X86_PDIR_E2_IDENTITY[vec2].p_data &= ~X86_PAGE_FDIRTY;
 }
 e1 = &X86_PDIR_E1_IDENTITY[vec2][X86_PDIR_VEC1INDEX_VPAGE(virt_addr)].p_flag;
 if ((*e1 & (X86_PAGE_FDIRTY|X86_PAGE_FPRESENT)) == (X86_PAGE_FDIRTY|X86_PAGE_FPRESENT))
      *e1 &= ~X86_PAGE_FDIRTY;
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_PAGING32_C_INL */
