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

#include "except_cache.h"

DECL_BEGIN

#ifndef C_LOOKUP_EXCEPTION_WITHOUT_CACHE
#define C_LOOKUP_EXCEPTION_WITHOUT_CACHE lookup_exception_without_cache
#define C_LOAD_EXCEPTION_DESCRIPTOR      load_exception_descriptor
#define C_ALLOCATE_EXCEPTION_INFO        allocate_exception_info
#define C_EXCEPTION_HANDLER              struct exception_handler
#define C_EXCEPTION_DESCRIPTOR           struct exception_descriptor
#endif


PRIVATE ATTR_NOTHROW bool KCALL
C_LOOKUP_EXCEPTION_WITHOUT_CACHE(C_EXCEPTION_HANDLER *__restrict iter,
                                 C_EXCEPTION_HANDLER *__restrict end,
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
    C_EXCEPTION_DESCRIPTOR *descr; u16 descr_type,descr_flag;
    /* The handler uses an exception descriptor. */
    descr = (C_EXCEPTION_DESCRIPTOR *)(uintptr_t)iter->eh_descr;
    if (hand_flags & EXCEPTION_HANDLER_FRELATIVE)
        *(uintptr_t *)&descr += app->a_loadaddr;
    if ((uintptr_t)descr < app->a_bounds.b_min)
         break;
    if ((uintptr_t)descr > (app->a_bounds.b_max+1)-sizeof(C_EXCEPTION_DESCRIPTOR))
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


#ifndef CONFIG_DISABLE_EXCEPTION_HANDLER_CACHE

PRIVATE ATTR_NOTHROW bool KCALL
C_LOAD_EXCEPTION_DESCRIPTOR(struct module *__restrict mod,
                            struct application *__restrict app, uintptr_t loadaddr,
                            C_EXCEPTION_DESCRIPTOR const *__restrict pdesc,
                            struct exception_handler_info *__restrict result) {
 C_EXCEPTION_DESCRIPTOR COMPILER_IGNORE_UNINITIALIZED(desc);
 TRY {
  COMPILER_BARRIER();
  memcpy(&desc,pdesc,sizeof(C_EXCEPTION_DESCRIPTOR));
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
C_ALLOCATE_EXCEPTION_INFO(struct module *__restrict mod,
                          struct application *__restrict app,
                          C_EXCEPTION_HANDLER *__restrict begin,
                          C_EXCEPTION_HANDLER *__restrict end,
                          uintptr_t loadaddr, uintptr_t rel_ip) {
 struct except_info_cache *result;
 C_EXCEPTION_HANDLER *iter;
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
  C_EXCEPTION_HANDLER *iter2,*lastpart;
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
   if (!C_LOAD_EXCEPTION_DESCRIPTOR(mod,app,loadaddr,
                                   (C_EXCEPTION_DESCRIPTOR *)(hand_entry + loadaddr),
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
    if (!C_LOAD_EXCEPTION_DESCRIPTOR(mod,app,loadaddr,
                                    (C_EXCEPTION_DESCRIPTOR *)(hand_entry + loadaddr),
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
   if (!C_LOAD_EXCEPTION_DESCRIPTOR(mod,app,loadaddr,
                                   (C_EXCEPTION_DESCRIPTOR *)(hand_entry + loadaddr),
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
#endif /* !CONFIG_DISABLE_EXCEPTION_HANDLER_CACHE */

#undef C_LOOKUP_EXCEPTION_WITHOUT_CACHE
#undef C_LOAD_EXCEPTION_DESCRIPTOR
#undef C_ALLOCATE_EXCEPTION_INFO
#undef C_EXCEPTION_HANDLER
#undef C_EXCEPTION_DESCRIPTOR

DECL_END

