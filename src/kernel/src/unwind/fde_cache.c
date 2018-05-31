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
#ifndef GUARD_KERNEL_SRC_KERNEL_UNWIND_FDE_CACHE_C
#define GUARD_KERNEL_SRC_KERNEL_UNWIND_FDE_CACHE_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <unwind/eh_frame.h>
#include <kernel/debug.h>
#include <kernel/heap.h>
#include <kernel/malloc.h>
#include <fs/linker.h>
#include <fs/driver.h>
#include <string.h>
#include <assert.h>
#include <except.h>

/* Define the ABI for the address tree used by vm. */
#define ATREE(x)               fde_cache_tree_##x
#define Tkey                   uintptr_t
#define T                      struct fde_info_cache
#define path                   ic_node
#define ATREE_NODE_MIN(x)    ((x)->ic_info.fi_pcbegin)
#define ATREE_NODE_MAX(x)    ((x)->ic_info.fi_pcend)
#define ATREE_NO_CONVENIENCE   1
#include <hybrid/list/atree-abi.h>
#undef ATREE_NO_CONVENIENCE

DECL_BEGIN

#if 1
/* Reduce the memory impact of these caches and speed up
 * a valid, but _really_ extensive scenario where there is
 * little pre-cached memory available, meaning that trying
 * to allocate anything results in 4 exceptions being stacked
 * ontop of each other.
 * While that scenario is completely valid (and does resolve itself),
 * it takes quite a while until it does.
 * By disabling tracing of caches, that scenario doesn't happen:
 * ..\src\kernel\src\unwind\except_cache.c(588,0) : except_cache_lookup : C013E9D9 : ESP C02462C0, EBP C0246344
 * ..\src\kernel\src\unwind\except_cache.c(728,0) : kernel_findexcept : C013EE13 : ESP C024634C, EBP C0246368
 * ..\src\kernel\src\unwind\linker.c(172,0) : linker_findexcept : C0140F99 : ESP C0246370, EBP C02463B8
 * ..\src\kernel\src\unwind\linker.c(46,0) : linker_findexcept_consafe : C0140AE8 : ESP C02463C0, EBP C0246428
 * ..\src\kernel\i386-kos\except.c(105,0) : libc_error_rethrow_at : C010768D : ESP C0246430, EBP C02464D8
 * ..\src\hybrid\i386-kos\except32.S(68,0) : .error_rethrow_eax_is_ip : C0103940 : ESP C02464E0, EBP C0246520
 * ..\src\kernel\i386-kos\scheduler32.S(299,0) : task_yield : C0124D2A : ESP C024651C, EBP C0246520
 * ..\include\hybrid\sync\atomic-rwlock.h(178,0) : atomic_rwlock_write : C0130B46 : ESP C0246520, EBP C0246520
 * ..\src\kernel\src\kernel\mall.c(1104,0) : define_user_traceing_point : C01338A1 : ESP C0246528, EBP C0246530
 * ..\src\kernel\src\kernel\mall.c(1120,0) : mall_trace : C0133932 : ESP C0246538, EBP C0246548
 * ..\src\kernel\src\kernel\mall.c(1212,0) : heap_alloc : C0133BFC : ESP C0246550, EBP C0246588
 * ..\src\kernel\include\kernel\heap.h(219,0) : heap_alloc : C013DCAD
 * ..\src\kernel\src\unwind\except_cache.c(214,0) : alloc_info : C013DCAD : ESP C0246590, EBP C02466C4
 * ..\src\kernel\src\unwind\except_cache.c(472,0) : allocate_exception_info : C013E5A7 : ESP C02466CC, EBP C0246738
 * ..\src\kernel\src\unwind\except_cache.c(592,0) : except_cache_lookup : C013E9F8 : ESP C0246740, EBP C02467DC
 * ..\src\kernel\src\unwind\except_cache.c(728,0) : kernel_findexcept : C013EE13 : ESP C02467E4, EBP C0246800
 * ..\src\kernel\src\unwind\linker.c(172,0) : linker_findexcept : C0140F99 : ESP C0246808, EBP C0246850
 * ..\src\kernel\src\unwind\linker.c(46,0) : linker_findexcept_consafe : C0140AE8 : ESP C0246858, EBP C02468C0
 * ..\src\kernel\i386-kos\except.c(105,0) : libc_error_rethrow_at : C010768D : ESP C02468C8, EBP C0246970
 * ..\src\hybrid\i386-kos\except32.S(68,0) : .error_rethrow_eax_is_ip : C0103940 : ESP C0246978, EBP C02469B8
 * ..\src\kernel\i386-kos\scheduler32.S(299,0) : task_yield : C0124D2A : ESP C02469B4, EBP C02469B8
 * ..\include\hybrid\sync\atomic-rwlock.h(178,0) : atomic_rwlock_write : C0130B46 : ESP C02469B8, EBP C02469B8
 * ..\src\kernel\src\kernel\mall.c(1104,0) : define_user_traceing_point : C01338A1 : ESP C02469C0, EBP C02469C8
 * ..\src\kernel\src\kernel\mall.c(1120,0) : mall_trace : C0133932 : ESP C02469D0, EBP C02469E0
 * ..\src\kernel\src\kernel\mall.c(1212,0) : heap_alloc : C0133BFC : ESP C02469E8, EBP C0246A20
 * ..\src\kernel\include\kernel\heap.h(219,0) : heap_alloc : C0140268
 * ..\src\kernel\src\unwind\fde_cache.c(152,0) : alloc_info : C0140268 : ESP C0246A28, EBP C0246B5C
 * ..\src\kernel\src\unwind\fde_cache.c(234,0) : fde_cache_insert : C0140684 : ESP C0246B64, EBP C0246BB0
 * ..\src\kernel\src\unwind\fde_cache.c(342,0) : kernel_eh_findfde : C0140A30 : ESP C0246BB8, EBP C0246BC0
 * ..\src\kernel\src\unwind\linker.c(79,0) : linker_findfde : C0140C25 : ESP C0246BC8, EBP C0246C10
 * ..\src\kernel\src\unwind\linker.c(40,0) : linker_findfde_consafe : C0140A78 : ESP C0246C18, EBP C0246C78
 * ..\src\kernel\i386-kos\except.c(99,0) : libc_error_rethrow_at : C010763A : ESP C0246C80, EBP C0246D24
 * ..\src\kernel\i386-kos\hw_except.c(108,0) : error_rethrow_atuser : C0108E6D : ESP C0246D2C, EBP C0246D34
 * ..\src\kernel\i386-kos\hw_except.c(369,0) : x86_handle_pagefault : C010979F : ESP C0246D3C, EBP C0246E4C
 * ..\src\kernel\i386-kos\hw_except32.S(138,0) : irq_0e : C0101362 : ESP C0246E54, EBP C0246EB4
 * -> All the way down here is where the original exception happened.
 *    All of the stuff above is able to resolve itself, but it really
 *    takes a while. - A more permanent solution for this would be to
 *    introduce a heap_alloc() function that never throws an exception,
 *    but rather indicates an allocation failure in some other way.
 *    It wouldn't even have to be able to interface with core_alloc(),
 *    as it would only ever be used with GFP_NOMAP.
 * ..\src\kernel\src\kernel\mall.c(235,0) : mall_reachable_data : C0131991 : ESP C0246E90, EBP C0246EB4
 * ..\src\kernel\src\kernel\mall.c(359,0) : mall_search_leaks_impl : C0131DFD : ESP C0246EBC, EBP C0246F00
 * ..\src\kernel\src\kernel\mall.c(564,0) : mall_dump_leaks : C0132607 : ESP C0246F08, EBP C0246F48
 * ..\src\kernel\src\kernel\kernctl.c(53,0) : kernel_control : C0130668 : ESP C0246F50, EBP C0246F78
 * ..\src\kernel\src\kernel\kernctl.c(145,0) : SYSC_xkernctl : C01308D4 : ESP C0246F80, EBP C0246F98
 * ..\src\kernel\src\kernel\kernctl.c(140,0) : sys_xkernctl : C01308B5 : ESP C0246FA0, EBP C0246FB8
 * ..\src\kernel\i386-kos\syscall32.S(326,0) : .sysenter_after_tracing : C0102C92 : ESP C0246FC0, EBP AFFFFE94
 * ..\??(0,0) : ?? : BEEFCF24 : ESP AFFFFE94, EBP CCCCCCCC
 */
#define HEAP_ALLOC_INFO  heap_alloc_untraced
#define HEAP_FREE_INFO   heap_free_untraced
#else
#define HEAP_ALLOC_INFO  heap_alloc
#define HEAP_FREE_INFO   heap_free
#endif

#define FREE_INFO(x) \
   HEAP_FREE_INFO(&kernel_heaps[GFP_SHARED],x,(x)->ic_size,GFP_SHARED)


PRIVATE ATTR_NOTHROW void KCALL
fde_free_info_node(struct fde_info_cache *__restrict node) {
 struct fde_info_cache *lo,*hi;
again:
 lo = node->ic_node.a_min;
 hi = node->ic_node.a_max;
 FREE_INFO(node);
 /* Recursively free all reachable nodes, but
  * try to keep things as linear as possible. */
 if (lo) {
  if (hi)
      fde_free_info_node(hi);
  node = lo;
  goto again;
 }
 if (hi) {
  node = hi;
  goto again;
 }
}

INTERN ATTR_NOTHROW void KCALL
fde_cache_fini(struct fde_cache *__restrict self) {
 if (self->fc_tree)
     fde_free_info_node(self->fc_tree);
}

INTERN ATTR_NOTHROW void KCALL
fde_cache_clear(struct fde_cache *__restrict self) {
 struct fde_info_cache *tree;
 atomic_rwlock_write(&self->fc_lock);
 tree = self->fc_tree;
 self->fc_tree = NULL;
 atomic_rwlock_endwrite(&self->fc_lock);
 if (tree)
     fde_free_info_node(tree);
}

INTERN ATTR_NOTHROW void KCALL
fde_info_mkabs(struct fde_info *__restrict self, uintptr_t loadaddr) {
 *(uintptr_t *)&self->fi_pcbegin  += loadaddr;
 *(uintptr_t *)&self->fi_pcend    += loadaddr;
 *(uintptr_t *)&self->fi_persofun += loadaddr;
 *(uintptr_t *)&self->fi_lsdaaddr += loadaddr;
 *(uintptr_t *)&self->fi_inittext += loadaddr;
 *(uintptr_t *)&self->fi_evaltext += loadaddr;
 ++self->fi_pcend;
}
INTERN ATTR_NOTHROW void KCALL
fde_info_mkrel(struct fde_info *__restrict self, uintptr_t loadaddr) {
 *(uintptr_t *)&self->fi_pcbegin  -= loadaddr;
 *(uintptr_t *)&self->fi_pcend    -= loadaddr;
 *(uintptr_t *)&self->fi_persofun -= loadaddr;
 *(uintptr_t *)&self->fi_lsdaaddr -= loadaddr;
 *(uintptr_t *)&self->fi_inittext -= loadaddr;
 *(uintptr_t *)&self->fi_evaltext -= loadaddr;
 --self->fi_pcend;
}

INTERN ATTR_NOTHROW bool KCALL
fde_cache_lookup(struct fde_cache *__restrict self,
                 struct fde_info *__restrict rel_info,
                 uintptr_t relative_ip) {
 struct fde_info_cache *node = NULL;
 if (!atomic_rwlock_tryread(&self->fc_lock)) {
  /* Without preemption being enabled, we can't actually
   * block until we can acquire this atomic lock.
   * However, since we're just an optional cache, we can
   * assume that the caller has plans for some fallback
   * way of figuring out this FDE. */
  if (!PREEMPTION_ENABLED())
       return false;
  atomic_rwlock_read(&self->fc_lock);
 }
 if (self->fc_tree)
     node = fde_cache_tree_locate_at(self->fc_tree,relative_ip,
                                     self->fc_semi0,self->fc_level0);
 if (!node) {
  atomic_rwlock_endread(&self->fc_lock);
  return false;
 }
 /* Copy information into the provided buffer. */
 memcpy(rel_info,&node->ic_info,sizeof(struct fde_info));
 atomic_rwlock_endread(&self->fc_lock);
 return true;
}


PRIVATE bool is_allocating_cache_entry = false;
PRIVATE ATTR_NOINLINE ATTR_NOTHROW
struct fde_info_cache *KCALL alloc_info(void) {
 struct heapptr result;
 struct exception_info old_exception;
 memcpy(&old_exception,error_info(),sizeof(struct exception_info));
 if (ATOMIC_XCH(is_allocating_cache_entry,true)) {
  return NULL; /* Prevent infinite recursion caused by this
                * function being called by the unwind machinery,
                * following an exception within this function itself. */
 }
 TRY {
  result = HEAP_ALLOC_INFO(&kernel_heaps[GFP_SHARED],
                            sizeof(struct fde_info_cache),
                            GFP_SHARED|GFP_NOMAP);
 } EXCEPT_HANDLED (EXCEPT_EXECUTE_HANDLER) {
  /* Catch all errors and restore old exception info. */
  struct exception_info *EXCEPT_VAR perror_info = error_info();
  ATOMIC_WRITE(is_allocating_cache_entry,false);
  memcpy(perror_info,&old_exception,sizeof(struct exception_info));
  return NULL;
 }
 ATOMIC_WRITE(is_allocating_cache_entry,false);
 /* Even if the call was OK, some underlying function may have
  * overwritten exceptions when it dealt with some other kind
  * of error. */
 memcpy(error_info(),&old_exception,sizeof(struct exception_info));
 /* Save the allocate size in the cache entry and return it. */
 ((struct fde_info_cache *)result.hp_ptr)->ic_size = result.hp_siz;
 return (struct fde_info_cache *)result.hp_ptr;
}


LOCAL ATTR_NOTHROW bool KCALL
fde_cache_exists(struct fde_info_cache *__restrict self,
                 uintptr_t ip_min, uintptr_t ip_max,
                 ATREE_SEMI_T(uintptr_t) page_semi,
                 ATREE_LEVEL_T page_level) {
again:
 assert(self != self->ic_node.a_min);
 assert(self != self->ic_node.a_max);
 if (ip_min <= self->ic_info.fi_pcend &&
     ip_max >= self->ic_info.fi_pcbegin)
     return true;
 {
  bool walk_min,walk_max;
  walk_min = ip_min <  page_semi && self->ic_node.a_min;
  walk_max = ip_max >= page_semi && self->ic_node.a_max;
  if (walk_min) {
   /* Recursively continue searching left. */
   if (walk_max) {
    if (fde_cache_exists(self->ic_node.a_max,ip_min,ip_max,
                         ATREE_NEXTMAX(uintptr_t,page_semi,page_level),
                         ATREE_NEXTLEVEL(page_level)))
        return true;
   }
   ATREE_WALKMIN(uintptr_t,page_semi,page_level);
   self = self->ic_node.a_min;
   goto again;
  } else if (walk_max) {
   /* Recursively continue searching right. */
   ATREE_WALKMAX(uintptr_t,page_semi,page_level);
   self = self->ic_node.a_max;
   goto again;
  }
 }
 return false;
}


INTERN ATTR_NOTHROW void KCALL
fde_cache_insert(struct module *__restrict self,
                 struct fde_info const *__restrict rel_info) {
 struct fde_info_cache *entry;
 /* Make sure that the FDE entry can actually _be_ cached. */
 if unlikely(rel_info->fi_pcbegin < self->m_imagemin ||
             rel_info->fi_pcend  >= self->m_imageend) {
  if (self == &kernel_module &&
      rel_info->fi_pcbegin >= (uintptr_t)kernel_start &&
      rel_info->fi_pcend   <  (uintptr_t)kernel_end_raw)
      goto pc_ok; /* Special case required because the kernel deletes its .free segment,
                   * however we must continue to track that segment's FDE descriptors. */
  debug_printf("[FDE] Corrupt FDE entry at %p...%p, out-of-bounds of %p...%p\n",
               rel_info->fi_pcbegin,rel_info->fi_pcend,
               self->m_imagemin,self->m_imageend-1);
  return;
 }
pc_ok:

 /* Allocate an info descriptor for the entry we're about to insert.
  * Due to the fact that it's quite unlikely that the given `rel_info'
  * has been inserted by another thread in the mean time, we always
  * allocate a cache entry here, just so we don't have to deallocate
  * it below.
  */
 entry = alloc_info();
 if (!entry) return;
 /* Copy information into the cache entry.
  * Do this now, so we don't have to do it while holding a
  * write lock on this potentially quite hot cache lock. */
 memcpy(&entry->ic_info,rel_info,sizeof(struct fde_info));
 COMPILER_WRITE_BARRIER();
 if (!atomic_rwlock_trywrite(&self->m_fde_cache.fc_lock)) {
  /* Don't block if preemption is disabled. */
  if (!PREEMPTION_ENABLED()) {
   FREE_INFO(entry);
   return;
  }
  atomic_rwlock_write(&self->m_fde_cache.fc_lock);
 }
 /* Verify that no entry for this FDE has already been created,
  * while also calculating the LEVEL0/SEM0 ATREE values. */
 if (self->m_fde_cache.fc_tree == NULL) {
  /* Calculate LEVEL0 and SEM0 during the first pass. */
  image_rva_t image_min,image_max;
  unsigned int first_uncommon_bit;
  image_min = self->m_imagemin;
  image_max = self->m_imageend-1;

  /* Special case: always include the .free section in
   *               the FDE cache of the kernel itself. */
  if (self == &kernel_module)
      image_max = (uintptr_t)kernel_end_raw-1;

  /* Determine the number of leading bits that are identical for both
   * `image_min' and `image_max' (usually these are mostly 0-bits) */
  first_uncommon_bit = (sizeof(image_rva_t)*8)-1;
  while (first_uncommon_bit) {
   image_rva_t mask;
   mask = (image_rva_t)1 << first_uncommon_bit;
   if ((image_min & mask) != (image_max & mask))
        break;
   --first_uncommon_bit;
  }
  /* Now we know that we only have to keep track of the first
   * `first_uncommon_bit' least significant bits in our ATREE.
   * And with that, we already have our LEVEL0! */
  self->m_fde_cache.fc_level0 = first_uncommon_bit;
  /* Now with that out of the way, SEMI0 is quite simple, as
   * all it really is, are all the common bits with the first
   * uncommon bit set to ONE(1) */
  assert((image_min & ~(((image_rva_t)1 << (first_uncommon_bit+1))-1)) ==
         (image_max & ~(((image_rva_t)1 << (first_uncommon_bit+1))-1)));
  self->m_fde_cache.fc_semi0  = image_min & ~(((image_rva_t)1 << (first_uncommon_bit+1))-1);
  self->m_fde_cache.fc_semi0 |= (image_rva_t)1 << first_uncommon_bit;
  assertf(ATREE_MAPMIN(uintptr_t,self->m_fde_cache.fc_semi0,self->m_fde_cache.fc_level0) <= image_min,
          "ATREE_MAPMIN(uintptr_t,%p,%u) = %p\n"
          "image_min                     = %p\n",
          self->m_fde_cache.fc_semi0,self->m_fde_cache.fc_level0,
          ATREE_MAPMIN(uintptr_t,self->m_fde_cache.fc_semi0,self->m_fde_cache.fc_level0),
          image_min);
  assertf(ATREE_MAPMAX(uintptr_t,self->m_fde_cache.fc_semi0,self->m_fde_cache.fc_level0) >= image_max,
         "ATREE_MAPMAX(uintptr_t,%p,%u) = %p\n"
         "image_max                     = %p\n",
          self->m_fde_cache.fc_semi0,self->m_fde_cache.fc_level0,
          ATREE_MAPMAX(uintptr_t,self->m_fde_cache.fc_semi0,self->m_fde_cache.fc_level0),
          image_max);
 } else if (fde_cache_exists(self->m_fde_cache.fc_tree,
                             entry->ic_info.fi_pcbegin,
                             entry->ic_info.fi_pcend,
                             self->m_fde_cache.fc_semi0,
                             self->m_fde_cache.fc_level0)) {
  /* The leaf has already been mapped (rare race condition) */
  atomic_rwlock_endwrite(&self->m_fde_cache.fc_lock);
  FREE_INFO(entry);
  return;
 }
 /* Insert the new leaf. */
#if 0
 debug_printf("fde_cache_tree_insert_at():\n");
 debug_printf("    entry  = { %p, %p }\n",
              entry->ic_info.fi_pcbegin,
              entry->ic_info.fi_pcend);
 debug_printf("    semi0  = %p\n",self->m_fde_cache.fc_semi0);
 debug_printf("    level0 = %p\n",self->m_fde_cache.fc_level0);
#endif
 fde_cache_tree_insert_at(&self->m_fde_cache.fc_tree,
                           entry,
                           self->m_fde_cache.fc_semi0,
                           self->m_fde_cache.fc_level0);
 atomic_rwlock_endwrite(&self->m_fde_cache.fc_lock);
}



INTDEF byte_t kernel_ehframe_start[];
INTDEF byte_t kernel_ehframe_end[];
INTDEF byte_t kernel_ehframe_size[];

INTERN ATTR_NOTHROW bool KCALL
kernel_eh_findfde(uintptr_t ip,
                  struct fde_info *__restrict result) {
 if (ip <  (uintptr_t)kernel_start ||
     ip >= (uintptr_t)kernel_end_raw)
     return false;
 if (fde_cache_lookup(&kernel_module.m_fde_cache,
                       result,ip)) {
  /* Since the kernel is mapped with a load address of ZERO(0),
   * all we need to do to adjust the resulting FDE information,
   * is to increase the PCEND field to referr to the end, rather
   * than the max. */
  ++result->fi_pcend;
  return true;
 }
 /* Search the kernel's .eh_frame section. */
 if (!eh_findfde(kernel_ehframe_start,
                (size_t)kernel_ehframe_size,
                 ip,result))
      return false;
 /* If we managed to find something, cache it. */
 --result->fi_pcend;
 fde_cache_insert(&kernel_module,result);
 ++result->fi_pcend;
 return true;
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_UNWIND_FDE_CACHE_C */
