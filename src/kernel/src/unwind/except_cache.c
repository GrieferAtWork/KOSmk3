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
#ifndef GUARD_KERNEL_SRC_KERNEL_UNWIND_EXCEPT_CACHE_C
#define GUARD_KERNEL_SRC_KERNEL_UNWIND_EXCEPT_CACHE_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <unwind/linker.h>
#include <kernel/debug.h>
#include <kernel/heap.h>
#include <kernel/malloc.h>
#include <fs/linker.h>
#include <fs/driver.h>
#include <string.h>
#include <assert.h>
#include <except.h>

/* Define the ABI for the address tree used by vm. */
#define ATREE(x)               except_cache_tree_##x
#define Tkey                   uintptr_t
#define T                      struct except_info_cache
#define path                   ic_node
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
except_free_info_node(struct except_info_cache *__restrict node) {
 struct except_info_cache *lo,*hi;
again:
 lo = node->ic_node.a_min;
 hi = node->ic_node.a_max;
 FREE_INFO(node);
 /* Recursively free all reachable nodes, but
  * try to keep things as linear as possible. */
 if (lo) {
  if (hi)
      except_free_info_node(hi);
  node = lo;
  goto again;
 }
 if (hi) {
  node = hi;
  goto again;
 }
}

PRIVATE ATTR_NOTHROW bool KCALL
lookup_exception_without_cache(struct exception_handler *__restrict iter,
                               struct exception_handler *__restrict end,
                               uintptr_t rel_ip, u16 exception_code,
                               struct application *__restrict app,
                               struct exception_handler_info *__restrict result) {
 struct module *mod = app->a_module;
 uintptr_t ip = rel_ip + app->a_loadaddr;
 /* Lookup exception information. */
 TRY {
  for (; iter < end; ++iter) {
   uintptr_t hand_begin,hand_end; u16 hand_flags;
   hand_flags = iter->eh_flag;
   COMPILER_READ_BARRIER();
   if (hand_flags & ~EXCEPTION_HANDLER_FMASK)
       break; /* Handler contains unknown flags (something's corrupt here). */
   if ((hand_flags & EXCEPTION_HANDLER_FHASMASK) &&
        iter->eh_mask != exception_code)
        continue; /* Non-matching mask. */
   hand_begin = (uintptr_t)iter->eh_begin;
   hand_end   = (uintptr_t)iter->eh_end;
   if (iter->eh_flag & EXCEPTION_HANDLER_FRELATIVE) {
    hand_begin += app->a_loadaddr;
    hand_end   += app->a_loadaddr;
   }
   /* Check handler IP ranges. */
   if (ip < hand_begin) continue;
   if (ip >= hand_end) continue;
   /* Got a matching handler! */
   if unlikely(hand_begin > hand_end)
      break; /* Corruption... */
   if unlikely(hand_begin < app->a_bounds.b_min)
      break; /* Invalid range start */
   if unlikely(hand_end > app->a_bounds.b_max+1)
      break; /* Invalid range end */
   /* All right! we found the handler we're looking for! */
   result->ehi_flag = hand_flags;
   result->ehi_mask = iter->eh_mask;
   if (hand_flags & EXCEPTION_HANDLER_FDESCRIPTOR) {
    uintptr_t entry;
    struct exception_descriptor *descr; u16 descr_type,descr_flag;
    /* The handler uses an exception descriptor. */
    descr = (struct exception_descriptor *)iter->eh_descr;
    if (hand_flags & EXCEPTION_HANDLER_FRELATIVE)
        *(uintptr_t *)&descr += app->a_loadaddr;
    if ((uintptr_t)descr < app->a_bounds.b_min)
         break;
    if ((uintptr_t)descr > (app->a_bounds.b_max+1)-sizeof(struct exception_descriptor))
         break;
    descr_type = descr->ed_type;
    descr_flag = descr->ed_flags;
    if unlikely(descr_type != EXCEPTION_DESCRIPTOR_TYPE_BYPASS)
       break; /* Unknown descriptor type. */
    if unlikely(descr_flag & ~EXCEPTION_DESCRIPTOR_FMASK)
       break; /* Unknown descriptor flags. */
    if unlikely((descr_flag & EXCEPTION_DESCRIPTOR_FDISABLE_PREEMPTION) &&
                (app->a_type != APPLICATION_TYPE_FDRIVER))
       break; /* User-space isn't allowed to use this flag. */
    entry = (uintptr_t)descr->ed_handler;
    if (descr_flag & EXCEPTION_DESCRIPTOR_FRELATIVE)
        entry += app->a_loadaddr;
    if unlikely(entry <  (app->a_loadaddr + mod->m_imagemin) ||
                entry >= (app->a_loadaddr + mod->m_imageend))
       break; /* Invalid handler entry point */
    /* Save additional information available through the descriptor. */
    result->ehi_entry         = (CHECKED void *)entry;
    result->ehi_desc.ed_type  = descr_type;
    result->ehi_desc.ed_flags = descr_flag;
    result->ehi_desc.ed_safe  = descr->ed_safe;
   } else {
    uintptr_t entry;
    entry = (uintptr_t)iter->eh_entry;
    if (hand_flags & EXCEPTION_HANDLER_FRELATIVE)
        entry += app->a_loadaddr;
    /* Validate the range of the entry point. */
    if unlikely(entry < (app->a_loadaddr + mod->m_imagemin) ||
                entry > (app->a_loadaddr + mod->m_imageend))
       break; /* Invalid handler entry point */
    result->ehi_entry = (UNCHECKED void *)entry;
   }
   return true;
  }
 } EXCEPT_HANDLED (EXCEPT_EXECUTE_HANDLER) {
  return false;
 }
 return false;
}



/* Finalize the given EXCEPT cache. */
INTERN ATTR_NOTHROW void KCALL
except_cache_fini(struct except_cache *__restrict self) {
 if (self->ec_tree)
     except_free_info_node(self->ec_tree);
}

/* Clear the EXCEPT cache (should be called during `kernel_cc_invoke()') */
INTERN ATTR_NOTHROW void KCALL
except_cache_clear(struct except_cache *__restrict self) {
 struct except_info_cache *tree;
 atomic_rwlock_write(&self->ec_lock);
 tree = self->ec_tree;
 self->ec_tree = NULL;
 atomic_rwlock_endwrite(&self->ec_lock);
 if (tree)
     except_free_info_node(tree);
}


#if 1

PRIVATE bool is_allocating_cache_entry = false;

PRIVATE ATTR_NOINLINE struct except_info_cache *
KCALL alloc_info(size_t info_count) {
 struct heapptr result;
 struct exception_info old_exception;
 memcpy(&old_exception,error_info(),sizeof(struct exception_info));
 if (ATOMIC_XCH(is_allocating_cache_entry,true)) {
  debug_printf("IS_ALLOCATING_CACHE_ENTRY == TRUE\n");
  return NULL; /* Prevent infinite recursion caused by this
                * function being called by the unwind machinery,
                * following an exception within this function itself. */
 }
 TRY {
  result = HEAP_ALLOC_INFO(&kernel_heaps[GFP_SHARED],
                            offsetof(struct except_info_cache,ic_info) +
                           (info_count * sizeof(struct exception_handler_info)),
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
 ((struct except_info_cache *)result.hp_ptr)->ic_size  = result.hp_siz;
 ((struct except_info_cache *)result.hp_ptr)->ic_count = info_count;
 return (struct except_info_cache *)result.hp_ptr;
}

PRIVATE ATTR_NOTHROW bool KCALL
load_exception_descriptor(struct module *__restrict mod,
                          struct application *__restrict app, uintptr_t loadaddr,
                          struct exception_descriptor const *__restrict pdesc,
                          struct exception_handler_info *__restrict result) {
 struct exception_descriptor COMPILER_IGNORE_UNINITIALIZED(desc);
 TRY {
  COMPILER_BARRIER();
  memcpy(&desc,pdesc,sizeof(struct exception_descriptor));
  COMPILER_BARRIER();
 } EXCEPT_HANDLED (EXCEPT_EXECUTE_HANDLER) {
  return false;
 }
 if (!(desc.ed_flags & EXCEPTION_DESCRIPTOR_FRELATIVE))
     *(uintptr_t *)&desc.ed_handler -= loadaddr;
 if ((uintptr_t)desc.ed_handler <  mod->m_imagemin ||
     (uintptr_t)desc.ed_handler >= mod->m_imageend)
      return false;
 if unlikely(desc.ed_type != EXCEPTION_DESCRIPTOR_TYPE_BYPASS)
    return false; /* Unknown descriptor type. */
 if unlikely(desc.ed_flags & ~EXCEPTION_DESCRIPTOR_FMASK)
    return false; /* Unknown descriptor flags. */
 if unlikely((desc.ed_flags & EXCEPTION_DESCRIPTOR_FDISABLE_PREEMPTION) &&
             (app->a_type != APPLICATION_TYPE_FDRIVER))
    return false; /* User-space isn't allowed to use this flag. */
 result->ehi_entry         = (void *)(uintptr_t)desc.ed_handler;
 result->ehi_desc.ed_type  = desc.ed_type;
 result->ehi_desc.ed_flags = desc.ed_flags;
 result->ehi_desc.ed_safe  = desc.ed_safe;
 return true;
}

PRIVATE ATTR_NOTHROW struct except_info_cache *KCALL
allocate_exception_info(struct module *__restrict mod,
                        struct application *__restrict app,
                        struct exception_handler *__restrict begin,
                        struct exception_handler *__restrict end,
                        uintptr_t loadaddr, uintptr_t rel_ip) {
 struct except_info_cache *result;
 struct exception_handler *iter;
 uintptr_t hand_min;
 uintptr_t hand_end;
 uintptr_t hand_entry;
 uintptr_half_t flags;
 /* Step #1: Find the first exception handler guarding the given `rel_ip' */
 for (iter = begin;; ++iter) {
  if (iter >= end)
      goto no_handler_exists;
  hand_min = (uintptr_t)iter->eh_begin;
  hand_end = (uintptr_t)iter->eh_end;
  flags    = iter->eh_flag;
  COMPILER_READ_BARRIER();
  if (flags & ~EXCEPTION_HANDLER_FMASK)
      goto no_handler_exists;
  if (!(flags & EXCEPTION_HANDLER_FRELATIVE)) {
   /* We're using relative addresses here. */
   hand_min -= loadaddr;
   hand_end -= loadaddr;
  }
  /* Check handler IP ranges. */
  if (rel_ip <  hand_min) continue;
  if (rel_ip >= hand_end) continue;
  if unlikely(hand_min > hand_end)
     goto corrupted_handler; /* Corruption... */
  break; /* Found a matching handler! */
 }
 hand_entry = iter->eh_entry;
 if (!(flags & EXCEPTION_HANDLER_FRELATIVE))
       hand_entry -= loadaddr;
 if (hand_entry <  mod->m_imagemin ||
     hand_entry >= mod->m_imageend)
     goto corrupted_handler;

 /* All right. So now we know that at least
  * one entry exists for this address range. */
 if (flags & EXCEPTION_HANDLER_FMASK) {
  size_t handler_count,i;
  struct exception_handler *iter2,*lastpart;
  /* The handler uses an exception mask, meaning we must search
   * for additional handlers that may also be visible at this
   * location, by not being obstructed by this one. */
  handler_count = 1;
  lastpart      = iter;
  for (iter2 = iter; iter2 < end; ++iter2) {
   uintptr_t inner_min,inner_end;
   uintptr_half_t inner_flags;
   inner_min   = (uintptr_t)iter2->eh_begin;
   inner_end   = (uintptr_t)iter2->eh_end;
   inner_flags = iter2->eh_flag;
   COMPILER_READ_BARRIER();
   if (!(inner_flags & EXCEPTION_HANDLER_FRELATIVE)) {
    /* We're using relative addresses here. */
    inner_min -= loadaddr;
    inner_end -= loadaddr;
   }
   /* Check handler IP ranges. */
   if (rel_ip <  inner_min) continue;
   if (rel_ip >= inner_end) continue;
   /* Found another one! */
   ++handler_count;
   lastpart = iter2;
   /* Iteration stops when a non-masking entry is found,
    * as that one will be able to deal with all the other
    * exceptions. */
   if (!(inner_flags & EXCEPTION_HANDLER_FMASK))
         break;
  }
  /* Allocate a cache entry for this location. */
  result = alloc_info(handler_count);
  if (!result) goto done; /* Check for bad allocation. */
  /* Setup the initial (CATCH) handler. */
  result->ic_info[0].ehi_entry = (void *)hand_entry;
  result->ic_info[0].ehi_flag  = flags;
  result->ic_info[0].ehi_mask  = iter->eh_mask;
  if (flags & EXCEPTION_HANDLER_FDESCRIPTOR) {
   /* This is an extended exception descriptor. */
   if (!load_exception_descriptor(mod,app,loadaddr,
                                 (struct exception_descriptor *)(hand_entry + loadaddr),
                                 &result->ic_info[0]))
        goto corrupted_handler_free;
  }

  /* Enumerate all entries that define handlers for our range. */
  for (iter2 = iter,i = 1;
       iter2 <= lastpart && i < handler_count; ++iter2) {
   uintptr_t inner_min,inner_end,inner_entry;
   inner_min = (uintptr_t)iter2->eh_begin;
   inner_end = (uintptr_t)iter2->eh_end;
   flags     = iter2->eh_flag;
   COMPILER_READ_BARRIER();
   if (!(flags & EXCEPTION_HANDLER_FRELATIVE)) {
    /* We're using relative addresses here. */
    inner_min -= loadaddr;
    inner_end -= loadaddr;
   }
   /* Check handler IP ranges. */
   if (rel_ip <  inner_min) continue;
   if (rel_ip >= inner_end) continue;
   /* This is one of them! */

   inner_entry = iter2->eh_entry;
   if (!(flags & EXCEPTION_HANDLER_FRELATIVE))
       *(uintptr_t *)&inner_entry -= loadaddr;
   /* Validate the entry point of the secondary handler. */
   if (hand_entry <  mod->m_imagemin ||
       hand_entry >= mod->m_imageend)
       goto corrupted_handler_free;
   result->ic_info[i].ehi_entry = (void *)inner_entry;
   result->ic_info[i].ehi_flag  = flags;
   result->ic_info[i].ehi_mask  = iter2->eh_mask;
   if (flags & EXCEPTION_HANDLER_FDESCRIPTOR) {
    /* This is an extended exception descriptor. */
    if (!load_exception_descriptor(mod,app,loadaddr,
                                  (struct exception_descriptor *)(hand_entry + loadaddr),
                                  &result->ic_info[i]))
         goto corrupted_handler_free;
   }
   /* Track the number of successfully parsed secondary handlers. */
   ++i;

   /* Truncate the range of the outer handler, using the
    * intersection of the 2 overlapping handlers instead. */
   if (hand_min < inner_min)
       hand_min = inner_min;
   if (hand_end > inner_end)
       hand_end = inner_end;
   /* Just like the first iteration, stop if a non-masking handler is found. */
   if (!(flags & EXCEPTION_HANDLER_FMASK))
         break;
  }
  /* NOTE: Technically, `ic_count' should equal `i' already, but
   *       because user-space may be tinkering with the exception
   *       vector, we can't take any chances and must assume that
   *       there is a chance that at least one of the handlers
   *       disappeared before we got here.
   *       In any case, simply truncate the count if that happened. */
  assert(i <= result->ic_count);
  result->ic_count = i;

 } else {
  /* Without a mask, this is a catch-all handler, meaning that
   * alongside the fact that we've already found the location
   * of the first handler, exceptions originating from `rel_ip'
   * must always be handled by `iter', and `iter' alone! */
  result = alloc_info(1);
  if (!result) goto done; /* Check for bad allocation. */
  result->ic_info[0].ehi_entry = (void *)hand_entry;
  result->ic_info[0].ehi_flag  = flags;
  result->ic_info[0].ehi_mask  = iter->eh_mask;
  if (flags & EXCEPTION_HANDLER_FDESCRIPTOR) {
   /* This is an extended exception descriptor. */
   if (!load_exception_descriptor(mod,app,loadaddr,
                                 (struct exception_descriptor *)(hand_entry + loadaddr),
                                  &result->ic_info[0]))
        goto corrupted_handler_free;
  }
 }
 assert(hand_min < hand_end);
 /* Trim the effective range of this handler,
  * as overlapped by other handlers. */
 for (iter = begin;; ++iter) {
  uintptr_t inner_min;
  uintptr_t inner_end;
  if (iter >= end) break;
  inner_min = (uintptr_t)iter->eh_begin;
  inner_end = (uintptr_t)iter->eh_end;
  flags     = iter->eh_flag;
  COMPILER_READ_BARRIER();
  if (!(flags & EXCEPTION_HANDLER_FRELATIVE)) {
   /* We're using relative addresses here. */
   inner_min -= loadaddr;
   inner_end -= loadaddr;
  }
  /* Trim the range of our new handler. */
  if (inner_min > rel_ip && inner_min < hand_end)
      hand_end = inner_min;
  if (inner_end < rel_ip && inner_end > hand_min)
      hand_min = inner_end;
 }
 assert(hand_min < hand_end);
 result->ic_node.a_vmin = hand_min;
 result->ic_node.a_vmax = hand_end-1;

 /* Clamp the ranges of the handler to the image.
  * NOTE: The handler entry point has already been validated for this! */
 if (result->ic_node.a_vmin < mod->m_imagemin)
     result->ic_node.a_vmin = mod->m_imagemin;
 if (result->ic_node.a_vmax >= mod->m_imageend)
     result->ic_node.a_vmax = mod->m_imageend-1;
done:
 return result;
corrupted_handler_free:
 FREE_INFO(result);
corrupted_handler:
 return (struct except_info_cache *)-1;
no_handler_exists:
 result = alloc_info(0);
 if (result) {
  /* Search for the nearest handler below and above. */
  uintptr_t next_minend = mod->m_imagemin;
  uintptr_t next_endmin = mod->m_imageend;
  for (iter = begin;; ++iter) {
   uintptr_t hand_min,hand_end;
   if (iter >= end)
       break;
   hand_min = (uintptr_t)iter->eh_begin;
   hand_end = (uintptr_t)iter->eh_end;
   flags    = iter->eh_flag;
   COMPILER_READ_BARRIER();
   if (flags & ~EXCEPTION_HANDLER_FMASK)
       break;
   if (!(flags & EXCEPTION_HANDLER_FRELATIVE)) {
    /* We're using relative addresses here. */
    hand_min -= loadaddr;
    hand_end -= loadaddr;
   }
   /* Figure out how large this region of ~nothingness~ really is. */
   if (next_endmin > hand_min && hand_min > rel_ip)
       next_endmin = hand_min;
   if (next_minend < hand_end && hand_end < rel_ip)
       next_minend = hand_end;
  }
  /* Save the size of the nothing-region. */
  assert(next_minend < next_endmin);
  result->ic_node.a_vmin = next_minend;
  result->ic_node.a_vmax = next_endmin-1;
 }
 return result;
}

LOCAL ATTR_NOTHROW bool KCALL
except_cache_exists(struct except_info_cache *__restrict self,
                    uintptr_t ip_min, uintptr_t ip_max,
                    ATREE_SEMI_T(uintptr_t) page_semi,
                    ATREE_LEVEL_T page_level) {
again:
 assert(self != self->ic_node.a_min);
 assert(self != self->ic_node.a_max);
 if (ip_min <= self->ic_node.a_vmax &&
     ip_max >= self->ic_node.a_vmin)
     return true;
 {
  bool walk_min,walk_max;
  walk_min = ip_min <  page_semi && self->ic_node.a_min;
  walk_max = ip_max >= page_semi && self->ic_node.a_max;
  if (walk_min) {
   /* Recursively continue searching left. */
   if (walk_max) {
    if (except_cache_exists(self->ic_node.a_max,ip_min,ip_max,
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

INTERN ATTR_NOTHROW bool KCALL
except_cache_lookup(struct exception_handler *__restrict iter,
                    struct exception_handler *__restrict end,
                    uintptr_t rel_ip, u16 exception_code,
                    struct application *__restrict app,
                    struct exception_handler_info *__restrict result) {
 struct except_info_cache *info;
 struct module *mod = app->a_module;
 bool lookup_ok; size_t i;
 /* Search the existing exception cache for a handler at this address. */
 if (!atomic_rwlock_tryread(&mod->m_exc_cache.ec_lock)) {
  if (!PREEMPTION_ENABLED())
       goto lookup_fallback;
  atomic_rwlock_read(&mod->m_exc_cache.ec_lock);
 }
 if (mod->m_exc_cache.ec_tree) {
  info = except_cache_tree_locate_at(mod->m_exc_cache.ec_tree,
                                     rel_ip,
                                     mod->m_exc_cache.ec_semi0,
                                     mod->m_exc_cache.ec_level0);
  if (info) {
   /* Found a cached set of handlers for the given IP.
    * Now to check which one (if any) applies to our exception code. */
   for (i = 0; i < info->ic_count; ++i) {
    /* Check the handler's mask (if it has one) */
    if ((info->ic_info[i].ehi_flag & EXCEPTION_HANDLER_FHASMASK) &&
        (info->ic_info[i].ehi_mask != exception_code))
         continue;
    /* This is the one! */
    memcpy(result,&info->ic_info[i],
           sizeof(struct exception_handler_info));
    atomic_rwlock_endread(&mod->m_exc_cache.ec_lock);
    *(uintptr_t *)&result->ehi_entry += app->a_loadaddr;
    return true;
   }
   atomic_rwlock_endread(&mod->m_exc_cache.ec_lock);
   /* None of the handlers match our exception code. */
   return false;
  }
 }
 atomic_rwlock_endread(&mod->m_exc_cache.ec_lock);

 /* Do a quick prediction if allocation of a
  * new cache entry could succeed right now. */
 if (ATOMIC_READ(is_allocating_cache_entry))
     goto lookup_fallback;

 /* Construct a new info descriptor. */
 info = allocate_exception_info(mod,app,iter,end,app->a_loadaddr,rel_ip);

 /* Deal with errors that may have occurred while we did this. */
 if (info == NULL)
     goto lookup_fallback; /* Allocation failure */
 if (info == (struct except_info_cache *)-1)
     return false; /* Corruption */

 lookup_ok = false;
 /* Search the newly created exception info for
  * a block matching the required exception code,
  * thus filling in the caller's info buffer, as
  * well as determining if there are handlers for
  * the requested IP.
  * The generated info node is only inserted into
  * the cache afterwards, so we don't have to loop
  * around and start over when some other thread
  * created the same cache entry in the mean time. */
 for (i = 0; i < info->ic_count; ++i) {
  /* Check the handler's mask (if it has one) */
  if ((info->ic_info[i].ehi_flag & EXCEPTION_HANDLER_FHASMASK) &&
      (info->ic_info[i].ehi_mask != exception_code))
       continue;
  /* This is the one! */
  memcpy(result,&info->ic_info[i],
         sizeof(struct exception_handler_info));
  *(uintptr_t *)&result->ehi_entry += app->a_loadaddr;
  lookup_ok = true;
  break;
 }

 /* Regardless of whether or not we managed to find a matching
  * handler, try to save the set of active handlers in cache. */
 if (!atomic_rwlock_trywrite(&mod->m_exc_cache.ec_lock)) {
  /* Give up on saving the info if preemption isn't enabled. */
  if (!PREEMPTION_ENABLED()) {
   FREE_INFO(info);
   goto done;
  }
  atomic_rwlock_write(&mod->m_exc_cache.ec_lock);
 }
 /* Now's its time to actually save the cache entry.
  * However, we still need to make sure that no other
  * thread created the cache in the mean time.
  * HINT: This check also ensures that the user isn't
  *       able to break the no-overlap expectation of
  *       the exception handler cache's ATREE by tinkering
  *       with handler bounds in a way that might produce
  *       overlapping handlers where there should be none. */
 if (!mod->m_exc_cache.ec_tree) {
  /* Calculate LEVEL0 and SEM0 during the first pass. */
  image_rva_t image_min,image_max;
  unsigned int first_uncommon_bit;
  image_min = mod->m_imagemin;
  image_max = mod->m_imageend-1;
  /* Determine the number of leading bits that are identical for both
   * `image_min' and `image_max' (usually these are mostly 0-bits) */
  first_uncommon_bit = (sizeof(image_rva_t)*8)-1;
  while (first_uncommon_bit) {
   image_rva_t mask;
   mask = 1 << first_uncommon_bit;
   if ((image_min & mask) != (image_max & mask))
        break;
   --first_uncommon_bit;
  }
  /* Now we know that we only have to keep track of the first
   * `first_uncommon_bit' least significant bits in our ATREE.
   * And with that, we already have our LEVEL0! */
  mod->m_exc_cache.ec_level0 = first_uncommon_bit;
  /* Now with that out of the way, SEMI0 is quite simple, as
   * all it really is, are all the common bits with the first
   * uncommon bit set to ONE(1) */
  assert((image_min & ~((1 << (first_uncommon_bit+1))-1)) ==
         (image_max & ~((1 << (first_uncommon_bit+1))-1)));
  mod->m_exc_cache.ec_semi0  = image_min & ~((1 << (first_uncommon_bit+1))-1);
  mod->m_exc_cache.ec_semi0 |= 1 << first_uncommon_bit;
  assertf(ATREE_MAPMIN(uintptr_t,mod->m_exc_cache.ec_semi0,mod->m_exc_cache.ec_level0) <= image_min,
          "ATREE_MAPMIN(uintptr_t,%p,%u) = %p\n"
          "image_min                     = %p\n",
          mod->m_exc_cache.ec_semi0,mod->m_exc_cache.ec_level0,
          ATREE_MAPMIN(uintptr_t,mod->m_exc_cache.ec_semi0,mod->m_exc_cache.ec_level0),
          image_min);
  assertf(ATREE_MAPMAX(uintptr_t,mod->m_exc_cache.ec_semi0,mod->m_exc_cache.ec_level0) >= image_max,
         "ATREE_MAPMAX(uintptr_t,%p,%u) = %p\n"
         "image_max                     = %p\n",
          mod->m_exc_cache.ec_semi0,mod->m_exc_cache.ec_level0,
          ATREE_MAPMAX(uintptr_t,mod->m_exc_cache.ec_semi0,mod->m_exc_cache.ec_level0),
          image_max);
 } else if (except_cache_exists(mod->m_exc_cache.ec_tree,
                                info->ic_node.a_vmin,
                                info->ic_node.a_vmax,
                                mod->m_exc_cache.ec_semi0,
                                mod->m_exc_cache.ec_level0)) {
  /* The leaf has already been mapped (rare race condition, or user-space corruption) */
  atomic_rwlock_endwrite(&mod->m_exc_cache.ec_lock);
  FREE_INFO(info);
  goto done;
 }
 /* Insert the new leaf. */
 except_cache_tree_insert_at(&mod->m_exc_cache.ec_tree,
                              info,
                              mod->m_exc_cache.ec_semi0,
                              mod->m_exc_cache.ec_level0);
 atomic_rwlock_endwrite(&mod->m_exc_cache.ec_lock);
done:
 return lookup_ok;
lookup_fallback:
 /* Fallback in case of allocation, or locking failure:
  * Do a lookup that is semantically equivalent, but doesn't
  * make use of the exception handler cache. */
 return lookup_exception_without_cache(iter,end,rel_ip,exception_code,app,result);
}
#else
INTERN ATTR_NOTHROW bool KCALL
except_cache_lookup(struct exception_handler *__restrict iter,
                    struct exception_handler *__restrict end,
                    uintptr_t rel_ip, u16 exception_code,
                    struct application *__restrict app,
                    struct exception_handler_info *__restrict result) {
 return lookup_exception_without_cache(iter,end,rel_ip,exception_code,app,result);
}
#endif



INTDEF struct exception_handler kernel_except_start[];
INTDEF struct exception_handler kernel_except_end[];
INTDEF byte_t kernel_except_size[];

/* Find an EXCEPT entry belonging to the kernel core. */
INTERN ATTR_NOTHROW bool KCALL
kernel_findexcept(uintptr_t ip, u16 exception_code,
                  struct exception_handler_info *__restrict result) {
 if (ip <  (uintptr_t)kernel_start ||
     ip >= (uintptr_t)kernel_end_raw)
     return false;
 return except_cache_lookup(kernel_except_start,
                            kernel_except_end,
                            ip,
                            exception_code,
                           &kernel_driver.d_app,
                            result);
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_UNWIND_EXCEPT_CACHE_C */
