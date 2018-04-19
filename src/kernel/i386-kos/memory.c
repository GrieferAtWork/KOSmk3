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
#ifndef GUARD_KERNEL_I386_KOS_MEMORY_C
#define GUARD_KERNEL_I386_KOS_MEMORY_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/align.h>
#include <kernel/memory.h>
#include <kernel/malloc.h>
#include <kernel/sections.h>
#include <kernel/debug.h>
#include <kernel/paging.h>
#include <kernel/vm.h>
#include <assert.h>
#include <string.h>

DECL_BEGIN

PUBLIC struct mzone *_mzones[MZONE_MAXCOUNT] ASMNAME("mzones");
PUBLIC size_t _mzone_count ASMNAME("mzone_count");


/* Predefined memory information. */
INTERN ATTR_FREERODATA struct meminfo const predefined_meminfo[MEMINSTALL_EARLY_PREDEFINED] = {
      /*  Pre-allocate the first page of physical memory
       * (Paging uses its address to indicate a non-allocated page) */
    { .mi_type = MEMTYPE_ALLOCATED, .mi_addr   = __UINT64_C(0x0000000000000000) },
    { .mi_type = MEMTYPE_NDEF,      .mi_addr   = __UINT64_C(0x0000000000001000) },
      /* VGA display buffer (Not defined by BIOS functions) */
    { .mi_type = MEMTYPE_DEVICE,    .mi_addr   = __UINT64_C(0x00000000000A0000), },
    { .mi_type = MEMTYPE_NDEF,      .mi_addr   = __UINT64_C(0x0000000000100000), },
#ifdef __x86_64__
    { .mi_type = MEMTYPE_KERNEL,    .mi_addr   = (uintptr_t)kernel_start - KERNEL_BASE },
    { .mi_type = MEMTYPE_KFREE,     .mi_addr   = (uintptr_t)kernel_free_start - KERNEL_BASE },
    { .mi_type = MEMTYPE_NDEF,      .mi_addr   = (uintptr_t)kernel_free_end - KERNEL_BASE }
#else
    { .mi_type = MEMTYPE_KERNEL,    .mi_addr32 = (uintptr_t)kernel_start - KERNEL_BASE },
    { .mi_type = MEMTYPE_KFREE,     .mi_addr32 = (uintptr_t)kernel_free_start - KERNEL_BASE },
    { .mi_type = MEMTYPE_NDEF,      .mi_addr32 = (uintptr_t)kernel_free_end - KERNEL_BASE }
#endif
};

/* Fallback heap address for memory zones. */
INTERN ATTR_FREEDATA PHYS byte_t *mzone_heap_end = kernel_end_raw - KERNEL_BASE;

#if 1 /* Don't allow physical memory that would identity-map into the identity mapping. */
#define MZONE_HEAP_END  (X86_PDIR_E1_IDENTITY_BASE - 0xc0000000)
#else
#define MZONE_HEAP_END  0x40000000
#endif

/* Returns the virtual variant of identity-mapped
 * physical memory within the first 1Gb. */
PRIVATE ATTR_FREETEXT VIRT void *KCALL
mzone_heap_malloc(size_t n_bytes) {
 PHYS uintptr_t result = (PHYS uintptr_t)-1;
 struct meminfo const *iter;
 MEMINFO_FOREACH(iter) {
  uintptr_t block_begin,block_end;
  if (iter->mi_type != MEMTYPE_RAM) continue;
  if (iter->mi_addr >= MZONE_HEAP_END-n_bytes) break;
  /* Prefer non-page aligned (and therefor otherwise unusable) memory. */
  block_begin = MEMINFO_BEGIN(iter);
  block_end   = MEMINFO_END(iter);
  if unlikely((block_end-block_begin) < n_bytes) continue;
  if (!IS_ALIGNED(block_end,PAGESIZE) &&
      ((block_end-n_bytes)/PAGESIZE) == (block_end/PAGESIZE)) {
   /* Take away from the back. */
   result = block_end-n_bytes;
  } else if (!IS_ALIGNED(block_begin,PAGESIZE) &&
            ((block_begin+n_bytes-1)/PAGESIZE) == (block_begin/PAGESIZE)) {
   /* Take away from the front. */
   result = block_begin;
  } else if (result == (PHYS uintptr_t)-1) {
   /* Take away from this block if we haven't found anything better, yet. */
   result = block_end-n_bytes;
  }
  /* Keep on looping to find the highest-possible candidate. */
 }
 if (result == (PHYS uintptr_t)-1) {
  /* Fallback: Allocate memory directly after the kernel. */
  result          = (uintptr_t)mzone_heap_end;
  mzone_heap_end += n_bytes;
 }
 assertf(result+n_bytes <= MZONE_HEAP_END,"Not part of the first 1Gb");
 /* Override the memory region to already be allocated. */
 mem_install(result,n_bytes,MEMTYPE_ALLOCATED);
 COMPILER_BARRIER();
 return (VIRT void *)(result + KERNEL_BASE);
}


/* The first physical address that can no longer be managed by memory zones. */
#define X86_MZONE_END_PAGE_ADDRESS ((u64)0xffffffff*PAGESIZE)

struct mzone_spec {
 pageptr_t ms_min; /* Lowest page */
 pageptr_t ms_max; /* Greatest page */
};

PUBLIC unsigned int _X86_MZONE_1GB ASMNAME("X86_MZONE_1GB");
PUBLIC unsigned int _X86_MZONE_4GB ASMNAME("X86_MZONE_4GB");

struct below_addr_zone {
 unsigned int *baz_pid;
 pageptr_t     baz_maxpage;
};
PRIVATE ATTR_FREERODATA
struct below_addr_zone const special_zones[] = {
    { &_X86_MZONE_1GB, (pageptr_t)(((__UINT64_C(0x000000003fffffff)+1)/PAGESIZE)-1) },
    { &_X86_MZONE_4GB, (pageptr_t)(((__UINT64_C(0x00000000ffffffff)+1)/PAGESIZE)-1) },
};


/* Construct memory zone specifications where each zone may
 * waste at most `max_unused_pages' bits of free-range memory. */
PRIVATE ATTR_FREETEXT bool KCALL
define_zone_specs(struct mzone_spec *specs,
                  size_t max_unused_pages) {
 unsigned int specc,special_i; struct meminfo const *iter;
 /* Clear out specifications memory. */
 memset(specs,0,MZONE_MAXCOUNT*sizeof(struct mzone_spec));
 /* Define fixed-address zones. */
 specs[X86_MZONE_1MB].ms_min  = ((0x00000000  )/PAGESIZE);
 specs[X86_MZONE_1MB].ms_max  = ((0x000fffff+1)/PAGESIZE)-1;
 specs[X86_MZONE_16MB].ms_min = ((0x00100000  )/PAGESIZE);
 specs[X86_MZONE_16MB].ms_max = ((0x00ffffff+1)/PAGESIZE)-1;
 specc = 2;
 MEMINFO_FOREACH(iter) {
  if (MEMINFO_END(iter) > 0x000fffff) break;
  if (iter->mi_type < MEMTYPE_RAM ||
      iter->mi_type > MEMTYPE_KFREE)
      continue;
  specs[X86_MZONE_1MB].ms_max = (MEMINFO_END(iter)/PAGESIZE)-1;
 }
 MEMINFO_FOREACH(iter) {
  pageptr_t zone_end;
  if (MEMINFO_END(iter) > 0x00ffffff) break;
  if (iter->mi_type < MEMTYPE_RAM ||
      iter->mi_type > MEMTYPE_KFREE)
      continue;
  zone_end = (MEMINFO_END(iter)/PAGESIZE);
  if (zone_end > specs[X86_MZONE_16MB].ms_min)
      specs[X86_MZONE_16MB].ms_max = zone_end-1;
 }

 MEMINFO_FOREACH(iter) {
  u64 iter_begin,iter_end;
  pageptr_t zone_min,zone_max;
  /* Must only create zones for memory
   * types that will eventually be used. */
  if (iter->mi_type < MEMTYPE_RAM ||
      iter->mi_type > MEMTYPE_KFREE)
      continue;
  if (iter->mi_addr >= X86_MZONE_END_PAGE_ADDRESS)
      break; /* End of addressable memory. */
  iter_begin = MEMINFO_BEGIN(iter);
  iter_end   = MEMINFO_END(iter);
  if (iter_end > X86_MZONE_END_PAGE_ADDRESS)
      iter_end = X86_MZONE_END_PAGE_ADDRESS;
  iter_begin = CEIL_ALIGN(iter_begin,PAGESIZE);
  iter_end   = FLOOR_ALIGN(iter_end,PAGESIZE);
  if unlikely(iter_begin >= iter_end)
     continue; /* Too small to matter, or too high to manage */
  zone_min = iter_begin/PAGESIZE;
  zone_max = (iter_end/PAGESIZE)-1;

  /* Check if the unused-page threshold allows us
   * to merge the mapping with the previous spec. */
  if (specs[specc-1].ms_max+max_unused_pages >= zone_min) {
   /* Extend the previous spec.
    * NOTE: Because of the static zones above, we
    *       must make sure not to subtract from this. */
   if (specs[specc-1].ms_max < zone_max)
       specs[specc-1].ms_max = zone_max;
  } else {
   /* Must create a new spec */
   if (specc == MZONE_MAXCOUNT)
       goto fail; /* Too small threshold. */
   specs[specc].ms_min = zone_min;
   specs[specc].ms_max = zone_max;
   ++specc;
  }
 }

 /* X86 memory zoning has some additional requirements for 2 zones:
  *   - X86_MZONE_4GB -- ms_max <= 0xffffffff
  *   - X86_MZONE_1GB -- ms_max <= 0x3fffffff */
 for (special_i = 0;
      special_i < COMPILER_LENOF(special_zones);
      ++special_i) {
  if (specs[specc-1].ms_max <= special_zones[special_i].baz_maxpage) {
   /* The greatest zone does not exceed 4 GB, so it implicitly is the 4Gb-zone */
   *special_zones[special_i].baz_pid = specc-1;
  } else {
   unsigned int i;
   /* Find the first zone that starts below 4Gb.
    * If it extends beyond, split it at the 4Gb mark, otherwise use it as 4Gb zone. */
   i = specc;
   while (i--) {
    if (specs[i].ms_min <= special_zones[special_i].baz_maxpage)
        break;
   }
   if (specs[i].ms_max <= special_zones[special_i].baz_maxpage) {
    /* The first zone below 4Gb also ends there,
     * meaning it can also be used as the 4Gb-zone. */
   } else {
    /* The zone crosses the 4Gb mark, meaning we must split it! */
    if (specc == MZONE_MAXCOUNT)
        goto fail;
    memmove(&specs[i+1],&specs[i],
            (specc-i)*sizeof(struct mzone_spec));
    ++specc;
    /* Update the min/max pointers of the split zone. */
    specs[i].ms_max   = special_zones[special_i].baz_maxpage;
    specs[i+1].ms_min = special_zones[special_i].baz_maxpage+1;
   }
   /* Save the index of the 4Gb zone. */
   *special_zones[special_i].baz_pid = i;
  }
 }
 return true;
fail:
 return false;
}


INTERN ATTR_FREETEXT void KCALL x86_mem_construct_zones(void) {
 struct mzone_spec specs[MZONE_MAXCOUNT];
 unsigned int i; size_t max_unused_pages = 0;
 struct meminfo const *iter;
 /* Calculate suitable specifications for memory zoning. */
 while (!define_zone_specs(specs,max_unused_pages)) {
  max_unused_pages *= 3;
  max_unused_pages /= 2;
  if (!max_unused_pages)
       max_unused_pages = 32;
 }
 /* Construct the required memory zones. */
 for (i = 0; i < MZONE_MAXCOUNT; ++i) {
  struct mzone *zone; size_t zone_size;
  if (specs[i].ms_max == specs[i].ms_min)
      break; /* Stop when there are no more zones. */
#if 0
  debug_printf("ZONE %u %.16I64X...%.16I64X\n",
               i,(u64)specs[i].ms_min,(u64)specs[i].ms_max);
#endif
  zone_size = (offsetof(struct mzone,mz_fpages)+
              ((specs[i].ms_max-specs[i].ms_min)+1+7)/8+2*sizeof(void *));
  zone = (struct mzone *)mzone_heap_malloc(zone_size);
  /* Clear out data of the zone descriptor. */
  memset(zone,0,zone_size);
  /* Initialize the zone. */
  zone->mz_min = specs[i].ms_min;
  zone->mz_max = specs[i].ms_max;
  /* Save the generated zone. */
  _mzones[i] = zone;
 }
 /* Save the total number of existing zones. */
 _mzone_count = i;

 /* Make all unused physical memory available to our memory zones. */
 MEMINFO_FOREACH(iter) {
  u64 iter_begin,iter_end;
  if (iter->mi_type != MEMTYPE_RAM) continue;
  if (iter->mi_addr >= X86_MZONE_END_PAGE_ADDRESS) break;
  iter_begin = MEMINFO_BEGIN(iter);
  iter_end   = MEMINFO_END(iter);
  if (iter_end > X86_MZONE_END_PAGE_ADDRESS)
      iter_end = X86_MZONE_END_PAGE_ADDRESS;
  iter_begin = CEIL_ALIGN(iter_begin,PAGESIZE);
  iter_end   = FLOOR_ALIGN(iter_end,PAGESIZE);
  if unlikely(iter_begin >= iter_end) continue; /* Too small to matter, or too high to manage */
  /* Make it available. */
  page_free((pageptr_t)(iter_begin/PAGESIZE),
            (size_t)((iter_end-iter_begin)/PAGESIZE));
 }

 /* With all memory now made available, reset the in-use counters of zones, as
  * they have underflown due to us using `page_free()' to register initial memory. */
 while (i--) _mzones[i]->mz_used = 0;
}


PRIVATE ATTR_FREETEXT void KCALL
x86_vm_add_identity(vm_vpage_t page_index, size_t num_pages) {
 struct vm_node *node;
 struct vm_region *region;
 vm_vpage_t page_max = page_index+num_pages-1;
 assert(num_pages);
 assert(page_index >= X86_KERNEL_BASE_PAGE);
 node = vm_pop_nodes(&vm_kernel,
                     page_index,page_max,
                     VM_UNMAP_IMMUTABLE,NULL);
 if (node) {
  size_t more;
  /* Another node already exists here (extend it). */
  assert(node->vn_byaddr.le_next == NULL);
  region = node->vn_region;
  assert(region->vr_refcnt == 1);
  assert(region->vr_parts == &region->vr_part0);
  assert(region->vr_type == VM_REGION_PHYSICAL);
  assert(region->vr_part0.vp_refcnt >= 1);
  assert(region->vr_part0.vp_chain.le_next == NULL);
  assert(region->vr_part0.vp_state == VM_PART_INCORE);
  assert(region->vr_part0.vp_phys.py_num_scatter == 1);
  assert(region->vr_part0.vp_phys.py_iscatter[0].ps_size == region->vr_size);
  assert(region->vr_part0.vp_phys.py_iscatter[0].ps_addr == VM_NODE_BEGIN(node) - X86_KERNEL_BASE_PAGE);
  if (page_index < node->vn_node.a_vmin) {
   more = node->vn_node.a_vmin - page_index;
   region->vr_size                                 += more;
   region->vr_part0.vp_phys.py_iscatter[0].ps_size += more;
   region->vr_part0.vp_phys.py_iscatter[0].ps_addr -= more;
   node->vn_node.a_vmin                             = page_index;
  }
  if (page_max > node->vn_node.a_vmax) {
   more = page_max - node->vn_node.a_vmax;
   region->vr_size                                 += more;
   region->vr_part0.vp_phys.py_iscatter[0].ps_size += more;
   node->vn_node.a_vmax                             = page_max;
  }
  vm_insert_node(&vm_kernel,node);
  return;
 }
 /* Create a new node for the region. */
 region = (struct vm_region *)kmalloc(sizeof(struct vm_region),
                                      GFP_SHARED|GFP_LOCKED|GFP_CALLOC);
 node = (struct vm_node *)kmalloc(sizeof(struct vm_node),
                                  GFP_SHARED|GFP_LOCKED);
 /* Initialize the node and region pair. */
 region->vr_flags = (VM_REGION_FIMMUTABLE|
                     VM_REGION_FDONTMERGE|
                     VM_REGION_FLEAKINGPARTS);
 region->vr_type                                 = VM_REGION_PHYSICAL;
 region->vr_refcnt                               = 1;
 region->vr_size                                 = num_pages;
 region->vr_parts                                = &region->vr_part0;
 region->vr_part0.vp_chain.le_pself              = &region->vr_parts;
 region->vr_part0.vp_refcnt                      = 2;
 region->vr_part0.vp_state                       = VM_PART_INCORE;
 region->vr_part0.vp_flags                       = VM_PART_FKEEP|VM_PART_FWEAKREF|VM_PART_FNOSWAP;
 region->vr_part0.vp_locked                      = 0x3fff;
 region->vr_part0.vp_phys.py_num_scatter         = 1;
 region->vr_part0.vp_phys.py_iscatter[0].ps_addr = page_index - X86_KERNEL_BASE_PAGE;
 region->vr_part0.vp_phys.py_iscatter[0].ps_size = num_pages;
 mutex_cinit(&region->vr_lock);
 atomic_rwptr_cinit(&region->vr_futex);

 node->vn_node.a_vmin = page_index;
 node->vn_node.a_vmax = page_max;
 node->vn_start       = 0;
 node->vn_region      = region;
 node->vn_notify      = NULL;
 node->vn_closure     = NULL;
 node->vn_prot        = PROT_SHARED|PROT_READ|PROT_WRITE|PROT_NOUSER;
 node->vn_flag        = VM_NODE_FIMMUTABLE;

 /* Insert the new node. */
 vm_insert_node(&vm_kernel,node);

}



/* Create memory nodes and regions for memory used by memory zones. */
INTERN ATTR_FREETEXT void KCALL
x86_mem_construct_zone_nodes(void) {
 unsigned int i;
 vm_acquire(&vm_kernel);
 for (i = 0; i < mzone_count; ++i) {
  VIRT struct mzone *zone;
  size_t zone_size; vm_vpage_t zone_page;
  zone       = mzones[i];
  zone_size  = (offsetof(struct mzone,mz_fpages)+
               ((zone->mz_max-zone->mz_min)+1+7)/8+2*sizeof(void *));
  zone_size += (uintptr_t)zone & (PAGESIZE-1);
  zone_page  = (uintptr_t)zone / PAGESIZE;
  zone_size += (PAGESIZE-1);
  zone_size /= PAGESIZE;
#if 0
  debug_printf("ZONE_DATA %p...%p\n",
               VM_PAGE2ADDR(zone_page),
               VM_PAGE2ADDR(zone_page+zone_size)-1);
#endif
  x86_vm_add_identity(zone_page,zone_size);
 }
 vm_release(&vm_kernel);
}


DATDEF size_t          _mem_info_c ASMNAME("mem_info_c");
DATDEF struct meminfo *_mem_info_v ASMNAME("mem_info_v");
DATDEF struct meminfo *_mem_info_last ASMNAME("mem_info_last");

INTERN ATTR_FREETEXT void KCALL x86_mem_relocate_info(void) {
 struct meminfo *info; size_t size;
 size = _mem_info_c*sizeof(struct meminfo);
 info = (struct meminfo *)kmalloc(size,GFP_SHARED);
 memcpy(info,_mem_info_v,size);
#ifndef NDEBUG
 /* This member is part of the boot task's stack (our stack)
  * So to improve debugging, restore its debug initialization. */
 memset(_mem_info_v,0xcc,size);
#endif
 /* Save the new memory information vector. */
 _mem_info_v    = info;
 _mem_info_last = info+(_mem_info_c-1);
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_MEMORY_C */
