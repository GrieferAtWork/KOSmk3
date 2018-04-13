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
#ifndef GUARD_KERNEL_SRC_KERNEL_MEMORY_C
#define GUARD_KERNEL_SRC_KERNEL_MEMORY_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/section.h>
#include <hybrid/minmax.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/byteswap.h>
#include <hybrid/align.h>
#include <kernel/paging.h>
#include <kernel/memory.h>
#include <sched/task.h>
#include <string.h>
#include <assert.h>
#include <except.h>
#include <endian.h>
#include <byteswap.h>
#include <kernel/debug.h>


DECL_BEGIN

//#define CONFIG_LOG_PAGE_ALLOCATIONS 1


/* Allocate `num_pages' continuous pages of
 * physical memory and return their page number.
 * WARNING: Physical memory cannot be dereferenced prior to being mapped.
 * @throw E_BADALLOC: Not enough available physical memory.
 * @return: * : The page number of the newly allocated memory range.
 * HINT: This function is identical to `page_malloc_part(num_pages,num_pages,...,max_zone)' */
PUBLIC pageptr_t KCALL
page_malloc(size_t num_pages, mzone_t max_zone) {
 size_t alloc_size;
 return page_malloc_part(num_pages,num_pages,
                        &alloc_size,max_zone);
}

#if __SIZEOF_POINTER__ == 4
#define DATAWORD_BITS  32
#define SWAPPTR(x) BSWAP_BE2H32((be32)(x))
#else
#define DATAWORD_BITS  64
#define SWAPPTR(x) BSWAP_BE2H64((be64)(x))
#endif

/* We need an SHL implementation that returns `0' for
 * input values that are larger than 32/64 bits.
 * On X86, the regular << operator will truncate the
 * number of bits by which to shift with mask `31' (or `63'),
 * meaning that `1 << 32' evaluates to 1 (because 1 << (32&1) == 1 << 0 == 1),
 * instead of `0', which we need for the bit masking logic below. */
#if 1
#define FIXED_SHL(x,y) ((y) >= DATAWORD_BITS ? 0 : ((x) << (y)))
#else
#define FIXED_SHL(x,y) ((x) << (y))
#endif


PRIVATE void KCALL
mzone_free(struct mzone *__restrict zone,
           pageptr_t zone_realtive_page,
           size_t num_pages) {
 uintptr_t *iter;
 assert(num_pages != 0);
 assertf(zone_realtive_page+num_pages <=
        ((zone->mz_max+1)-zone->mz_min),
        "zone_realtive_page = %p\n"
        "num_pages          = %p\n"
        "zone->mz_min       = %p\n"
        "zone->mz_max       = %p\n",
        zone_realtive_page,num_pages,
        zone->mz_min,zone->mz_max);
#ifdef CONFIG_LOG_PAGE_ALLOCATIONS
 debug_printf("FREE(%p...%p)\n",
             (zone->mz_min+zone_realtive_page)*PAGESIZE,
             (zone->mz_min+zone_realtive_page+num_pages)*PAGESIZE-1);
#endif
 iter = (uintptr_t *)zone->mz_fpages +
        (zone_realtive_page/DATAWORD_BITS);
 for (; num_pages; ++iter) {
  uintptr_t word_offset,free_pages,free_mask;
  word_offset = (uintptr_t)zone_realtive_page & (DATAWORD_BITS-1);
  /* Generate the free mask. */
  free_pages = MIN(DATAWORD_BITS - word_offset,num_pages);
  free_mask  = (FIXED_SHL((uintptr_t)1,free_pages)-1) << word_offset;
  free_mask  = SWAPPTR(free_mask);
#ifdef NDEBUG
  ATOMIC_FETCHOR(*iter,free_mask);
#else
  {
   uintptr_t old_mask;
   old_mask = ATOMIC_FETCHOR(*iter,free_mask);
   assertf(!(old_mask & free_mask),
           "Memory has already been freed:\n"
           "  old_mask  = %p\n"
           "  free_mask = %p",
           old_mask,free_mask);
  }
#endif
  /* Update statistics. */
  ATOMIC_FETCHADD(zone->mz_free,free_pages);
  ATOMIC_FETCHSUB(zone->mz_used,free_pages);
  num_pages          -= free_pages;
  zone_realtive_page += free_pages;
 }
}

PRIVATE bool KCALL
mzone_malloc_at(struct mzone *__restrict zone,
                pageptr_t zone_realtive_page,
                size_t num_pages) {
 uintptr_t *iter;
 size_t missing_pages; pageptr_t base;
 assert(num_pages != 0);
 assert(zone_realtive_page+num_pages <=
       ((zone->mz_max+1)-zone->mz_min));
 /* Quick check: are there even enough 1-bits at all? */
 if unlikely(zone->mz_free < num_pages)
    goto nope;
 iter = (uintptr_t *)zone->mz_fpages +
        (zone_realtive_page/DATAWORD_BITS);
 missing_pages = num_pages;
 base          = zone_realtive_page;
 for (; missing_pages; ++iter) {
  uintptr_t word_offset,alloc_pages,origword,alloc_mask;
  word_offset = (uintptr_t)base & (DATAWORD_BITS-1);
  /* Generate the allocation mask. */
  alloc_pages = MIN(DATAWORD_BITS - word_offset,missing_pages);
  alloc_mask  = (FIXED_SHL((uintptr_t)1,alloc_pages)-1) << word_offset;
  alloc_mask  = SWAPPTR(alloc_mask);
  do {
   origword = ATOMIC_READ(*iter);
   if ((origword & alloc_mask) != alloc_mask)
        goto nope_release; /* Memory in question has already been allocated. */
  } while (!ATOMIC_CMPXCH(*iter,origword,origword & ~alloc_mask));
  /* Update statistics. */
  ATOMIC_FETCHSUB(zone->mz_free,alloc_pages);
  ATOMIC_FETCHADD(zone->mz_used,alloc_pages);
  missing_pages -= alloc_pages;
  base          += alloc_pages;
 }
 return true;
nope_release:
 /* Free what was already allocated. */
 assert(base >= zone_realtive_page);
 if (base != zone_realtive_page)
     mzone_free(zone,zone_realtive_page,
                base-zone_realtive_page);
nope:
 return false;
}



/* Allocate some number of pages, though at least `min_pages', but
 * not more than `max_pages', preferring to allocate from smaller
 * clusters of memory, though not moving on to lower-level zones
 * until the zone currently being searched turns out not to contain
 * a range of at least `min_pages' pages.
 * WARNING: Physical memory cannot be dereferenced prior to being mapped.
 * @param: res_pages: Filled with the allocated number of pages.
 * @throw E_BADALLOC: Not enough available physical memory.
 * @return: * : The page number of the newly allocated memory range. */
PUBLIC pageptr_t KCALL
page_malloc_part(size_t min_pages, size_t max_pages,
                 size_t *__restrict res_pages,
                 mzone_t max_zone) {
 mzone_t zone_id;
 bool cmpxch_failed;
 assert(min_pages <= max_pages);
 assert(max_zone < mzone_count);
again:
 cmpxch_failed = false;
 zone_id = max_zone;
 do {
  uintptr_t *iter,*end;
  struct mzone *zone = mzones[zone_id];
  if (ATOMIC_READ(zone->mz_free) < min_pages) {
   debug_printf("ZONE %u/%u IS TOO SMALL (%Iu < %Iu; used %Iu)\n",
                zone_id,max_zone,zone->mz_free,min_pages,zone->mz_used);
   goto next_zone;
  }
  iter = (uintptr_t *)zone->mz_fpages;
  end  = (uintptr_t *)((uintptr_t)iter+
                       (uintptr_t)((zone->mz_max - zone->mz_min)/8)+1);
  for (; iter < end; ++iter) {
   pageptr_t alloc_base;
   size_t alloc_offset,alloc_size;
   uintptr_t dataword,alloc_mask;
   uintptr_t origword;
retry_dataword:
   origword = ATOMIC_READ(*iter);
   if (!origword) continue;
   dataword = SWAPPTR(origword);
   alloc_offset = 0;
scan_alloc_offset:
   while (!(dataword & 1))
            dataword >>= 1,++alloc_offset;
   alloc_size = 0;
   while ((dataword & 1) &&
          (alloc_size < max_pages))
           dataword >>= 1,++alloc_size;
   assert(alloc_offset+alloc_size <= DATAWORD_BITS);
   alloc_mask = (FIXED_SHL((uintptr_t)1,alloc_size)-1) << alloc_offset;
   alloc_mask = SWAPPTR(alloc_mask);
   assertf((origword & alloc_mask) == alloc_mask,
           "origword   = %p (%p)\n"
           "alloc_mask = %p (%p)\n",
            origword,SWAPPTR(origword),
            alloc_mask,SWAPPTR(alloc_mask));
   alloc_base = (pageptr_t)((uintptr_t *)iter -
                            (uintptr_t *)zone->mz_fpages)*
                             DATAWORD_BITS;
   alloc_base += alloc_offset;

   /* `alloc_base' is now the zone-relative
    * starting address of the allocation candidate. */

   if (alloc_size < min_pages) {
    size_t missing_pages;
    pageptr_t max_avail;
    /* Must allocate more than found in this data word.
     * We get here for 2 reasons:
     *   #1: There isn't enough memory in the dataword
     *       (This is the case when `dataword' isn't ZERO(0), or when
     *        `alloc_offset' + `alloc_size' != DATAWORD_BITS)
     *   #2: The dataword has been fully allocated, but the caller needs more.
     *       This is the case when #1 isn't true. */
    if (dataword != 0) {
     assert(alloc_offset+alloc_size != DATAWORD_BITS);
     /* Check if enough memory is located elsewhere in the data word. */
     alloc_offset += alloc_size;
     goto scan_alloc_offset;
    }
    if (alloc_offset+alloc_size != DATAWORD_BITS) {
     /* The free range is too small. */
     continue;
    }

    missing_pages = min_pages-alloc_size;
    max_avail     = ((zone->mz_max-zone->mz_min)+1)-alloc_base;
    /* Check if the zone could theoretically contain all the missing pages. */
    if unlikely(missing_pages > max_avail) {
     debug_printf("missing_pages(%Iu) > max_avail(%Iu)\n",
                  missing_pages,max_avail);
     goto next_zone;
    }

    /* The caller needs more memory and the current
     * dataword might extend into the next one.
     * Allocate memory from this word, then try to
     * allocate the remainder that would be following. */
    if (!ATOMIC_CMPXCH(*iter,origword,origword & ~alloc_mask)) {
     cmpxch_failed = true;
     goto retry_dataword;
    }
    /* Update statistics. */
    ATOMIC_FETCHSUB(zone->mz_free,alloc_size);
    ATOMIC_FETCHADD(zone->mz_used,alloc_size);

    /* All right! Now try to allocate the remainder from following data words. */
    if (!mzone_malloc_at(zone,alloc_base+alloc_size,missing_pages)) {
     /* Failed to allocate the missing portion.
      * -> Free what we've already allocated. */
     ATOMIC_FETCHOR(*iter,alloc_mask);
     /* Update statistics. */
     ATOMIC_FETCHADD(zone->mz_free,alloc_size);
     ATOMIC_FETCHSUB(zone->mz_used,alloc_size);
     continue;
    }

    /* The missing part could be allocated! */
    alloc_base += zone->mz_min;
    *res_pages = alloc_size + missing_pages;
#ifdef CONFIG_LOG_PAGE_ALLOCATIONS
    debug_printf("PAGE_MALLOC() -> %p...%p\n",
                (uintptr_t)alloc_base*PAGESIZE,
               ((uintptr_t)alloc_base+*res_pages)*PAGESIZE-1);
#endif
    return alloc_base;
   }
   
   /* Found a memory range of sufficient size. - Try to allocate it */
   if (!ATOMIC_CMPXCH(*iter,origword,origword & ~alloc_mask)) {
    cmpxch_failed = true;
    goto retry_dataword;
   }

   /* Update statistics. */
   ATOMIC_FETCHSUB(zone->mz_free,alloc_size);
   ATOMIC_FETCHADD(zone->mz_used,alloc_size);

   /* We've managed to allocate the memory!
    * Now just determine its physical base address. */
   alloc_base += zone->mz_min;
   *res_pages = alloc_size;
#ifdef CONFIG_LOG_PAGE_ALLOCATIONS
   debug_printf("PAGE_MALLOC() -> %p...%p (from %p...%p; mz_free = %Iu, mz_used = %Iu)\n",
               (uintptr_t)alloc_base*PAGESIZE,
              ((uintptr_t)alloc_base+*res_pages)*PAGESIZE-1,
                zone->mz_min*PAGESIZE,
               ((zone->mz_max+1)*PAGESIZE)-1,
                zone->mz_free,zone->mz_used);
#endif
   return alloc_base;
  }

next_zone:;
 } while (zone_id--);
 /* If a compare-exchange failed at some point, it is highly likely
  * that some other thread is trying to allocate memory at the same
  * time.
  * With that in mind, it is also likely that the state of the memory
  * zones has changed since we've began allocation, meaning there's a
  * chance that if we were to try again, we might be able to succeed
  * after all! */
 if (cmpxch_failed) goto again;

 /* Throw a bad-allocation error. */
 {
  struct exception_info *info;
  info = error_info();
  info->e_error.e_code = E_BADALLOC;
  info->e_error.e_flag = ERR_FNORMAL|ERR_FRESUMABLE|ERR_FRESUMEFUNC;
  memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
  info->e_error.e_badalloc.ba_resource = ERROR_BADALLOC_PHYSMEMORY;
  info->e_error.e_badalloc.ba_amount   = min_pages * PAGESIZE;
  /* Allow the exception to be continued, in which
   * case the allocation is attempted again. */
  if (error_throw_current())
      goto again;
  /* The error should be ignored... Let's see what you do about this. */
  return (pageptr_t)-1;
 }
}


/* Free a given physical address range. */
PUBLIC void KCALL
page_free(pageptr_t base, size_t num_pages) {
 mzone_t i = mzone_count;
 assert(base+num_pages >= base ||
        base+num_pages == 0);
 while (num_pages && i--) {
  pageptr_t zone_start,zone_end;
  struct mzone *zone = mzones[i];
  zone_start = zone->mz_min;
  if (zone_start >= base+num_pages &&
     (base+num_pages) != 0) continue;
  zone_end   = base+num_pages;
  zone_start = zone->mz_min;
  if (zone_start < base)
      zone_start = base;
  assertf(zone_end-1 <= zone->mz_max,
          "Address range %.16I64X...%.16I64X is not zoned\n"
          "i            = %u\n"
          "zone->mz_min = %p\n"
          "zone->mz_max = %p\n"
          "zone_start   = %p\n"
          "zone_end     = %p\n",
         (u64)(zone->mz_max+1)*PAGESIZE,((u64)zone_end*PAGESIZE)-1,
          i,zone->mz_min,zone->mz_max,zone_start,zone_end);
  assertf(zone_end > zone_start,
          "zone_start = %p\n"
          "zone_end   = %p\n"
          "base       = %p\n"
          "num_pages  = %p\n",
          zone_start,zone_end,
          base,num_pages);
  /* Free all pages that are apart of this zone. */
  mzone_free(zone,zone_start - zone->mz_min,
             zone_end-zone_start);
  /* Update the remaining number of pages. */
  num_pages = zone_start - (uintptr_t)base;
 }
}


PUBLIC bool KCALL
page_malloc_at(pageptr_t base, size_t num_pages) {
 mzone_t i = mzone_count;
 size_t remaining_num_pages = num_pages;
 pageptr_t zone_start,zone_end;
 while (remaining_num_pages && i--) {
  struct mzone *zone = mzones[i];
  zone_start = zone->mz_min;
  if (zone_start >= base+num_pages &&
     (base+num_pages) != 0) continue;
  zone_end   = base+num_pages;
  zone_start = zone->mz_min;
  if (zone_start < base)
      zone_start = base;
  if (zone_end-1 > zone->mz_max)
      goto err; /* Some portion of the requested range doesn't belong to any zone. */
  assertf(zone_end > zone_start,
          "zone_start = %p\n"
          "zone_end   = %p\n"
          "base       = %p\n"
          "num_pages  = %p\n",
          zone_start,zone_end,
          base,num_pages);
  assert(zone_end != zone_start);
  /* Allocate all pages that are apart of this zone. */
  if (!mzone_malloc_at(zone,zone_start - zone->mz_min,
                       zone_end-zone_start))
       goto err;
  /* Update the remaining number of pages. */
  remaining_num_pages = zone_start - (uintptr_t)base;
 }
 return true;
err:
 page_free(zone_end,num_pages - remaining_num_pages);
 return false;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_MEMORY_C */
