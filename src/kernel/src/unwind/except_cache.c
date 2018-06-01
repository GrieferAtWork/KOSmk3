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
#define ATREE(x)               except_cache_tree_##x
#define Tkey                   uintptr_t
#define T                      struct except_info_cache
#define path                   ic_node
#define ATREE_NO_CONVENIENCE   1
#include <hybrid/list/atree-abi.h>
#undef ATREE_NO_CONVENIENCE

#include "except_cache.h"

#ifndef __INTELLISENSE__
#include "except_cache-impl.c.inl"

#ifdef CONFIG_ELF_SUPPORT_CLASS3264
#define C_LOOKUP_EXCEPTION_WITHOUT_CACHE lookup_exception_without_cache_compat
#define C_LOAD_EXCEPTION_DESCRIPTOR      load_exception_descriptor_compat
#define C_ALLOCATE_EXCEPTION_INFO        allocate_exception_info_compat
#define C_EXCEPTION_HANDLER              struct exception_handler_compat
#define C_EXCEPTION_DESCRIPTOR           struct exception_descriptor_compat
#include "except_cache-impl.c.inl"
#endif
#endif


DECL_BEGIN


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


#ifndef CONFIG_DISABLE_EXCEPTION_HANDLER_CACHE
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
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
 if (ELF_ISMACHINE32(app->a_module->m_machine)) {
  info = allocate_exception_info_compat(mod,app,
                                       (struct exception_handler_compat *)iter,
                                       (struct exception_handler_compat *)end,
                                        app->a_loadaddr,
                                        rel_ip);
 } else
#endif
 {
  info = allocate_exception_info(mod,app,iter,end,app->a_loadaddr,rel_ip);
 }

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
   mask = (image_rva_t)1 << first_uncommon_bit;
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
  assert((image_min & ~(((image_rva_t)1 << (first_uncommon_bit+1))-1)) ==
         (image_max & ~(((image_rva_t)1 << (first_uncommon_bit+1))-1)));
  mod->m_exc_cache.ec_semi0  = image_min & ~(((image_rva_t)1 << (first_uncommon_bit+1))-1);
  mod->m_exc_cache.ec_semi0 |= (image_rva_t)1 << first_uncommon_bit;
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
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
 if (ELF_ISMACHINE32(app->a_module->m_machine)) {
  return lookup_exception_without_cache_compat((struct exception_handler_compat *)iter,
                                               (struct exception_handler_compat *)end,
                                                rel_ip,exception_code,app,result);
 }
#endif
 return lookup_exception_without_cache(iter,end,rel_ip,exception_code,app,result);
}
#else
INTERN ATTR_NOTHROW bool KCALL
except_cache_lookup(struct exception_handler *__restrict iter,
                    struct exception_handler *__restrict end,
                    uintptr_t rel_ip, u16 exception_code,
                    struct application *__restrict app,
                    struct exception_handler_info *__restrict result) {
#ifdef CONFIG_ELF_SUPPORT_CLASS3264
 if (ELF_ISMACHINE32(app->a_module->m_machine)) {
  return lookup_exception_without_cache_compat((struct exception_handler_compat *)iter,
                                               (struct exception_handler_compat *)end,
                                                rel_ip,exception_code,app,result);
 }
#endif
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
