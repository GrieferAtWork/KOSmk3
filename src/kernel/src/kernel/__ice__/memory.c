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
#include <hybrid/types.h>
#include <hybrid/section.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/debug.h>
#include <hybrid/align.h>
#include <kernel/paging.h>
#include <kernel/memory.h>
#include <sched/task.h>
#include <string.h>
#include <assert.h>
#include <except.h>
#include <endian.h>
#include <byteswap.h>

#ifndef CONFIG_USE_NEW_PAGEMALLOC
DECL_BEGIN

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


#if __BYTE_ORDER == __BIG_ENDIAN
/* free page information is encoded in little-endian:
 * DATA:   00011010 00011100 00101010 00011111
 * PAGE:   00000000 00000000 11111111 11111111
 *         76543210 FEDCBA98 76543210 FEDCBA98 */
#if __SIZEOF_POINTER__ == 8
#define BSWAP_FREEMASK(mask)  bswap_64(mask)
#elif __SIZEOF_POINTER__ == 4
#define BSWAP_FREEMASK(mask)  bswap_32(mask)
#else
#error "Unsupported pointer size"
#endif
#else
#define BSWAP_FREEMASK(mask)  (mask)
#endif

#define MZONE_GETBIT(self,x) \
 ((self)->mz_fpages[(x)/8] & (1 << ((x) % 8)))

#undef CONFIG_LOG_ALLOCATIONS
/*#define CONFIG_LOG_ALLOCATIONS 1*/


PRIVATE void KCALL
mzone_clear_bits(struct mzone *__restrict self,
                 pageptr_t zone_relative_start,
                 size_t num_bits) {
 u8 *start,mask;
#ifdef CONFIG_LOG_ALLOCATIONS
 debug_printf("ALLOC(%p...%p)\n",
             (self->mz_min+zone_relative_start)*PAGESIZE,
             (self->mz_min+zone_relative_start+num_bits-1)*PAGESIZE);
#endif /* CONFIG_LOG_ALLOCATIONS */
#ifndef NDEBUG
 size_t i;
 for (i = 0; i < num_bits; ++i) {
  size_t bitno = zone_relative_start + i;
  assertf(MZONE_GETBIT(self,bitno) != 0,
          "Page at %p (of %p...%p) has already been allocated\n"
          "bitno               = %p\n"
          "self->mz_fpages[%u] = %p",
         (self->mz_min+bitno)*PAGESIZE,
         (self->mz_min+zone_relative_start)*PAGESIZE,
         (self->mz_min+zone_relative_start+num_bits-1)*PAGESIZE,
          bitno,bitno/8,self->mz_fpages[bitno/8]);
 }
#endif
 while (zone_relative_start & 7) {
  if (!num_bits) return;
  start   = self->mz_fpages + zone_relative_start/8;
  mask    = 1 << (zone_relative_start & 7);
  *start &= ~mask;
  ++zone_relative_start,--num_bits;
 }
 if (num_bits > 8) {
  /* Clear full bits. */
  start = self->mz_fpages + zone_relative_start/8;
  memset(start,0,num_bits/8);
  zone_relative_start += num_bits & ~7;
  num_bits            &= 7;
 }
 while (num_bits) {
  start   = self->mz_fpages + zone_relative_start/8;
  mask    = 1 << (zone_relative_start & 7);
  *start &= ~mask;
  ++zone_relative_start,--num_bits;
 }
}
PRIVATE void KCALL
mzone_set_bits(struct mzone *__restrict self,
               pageptr_t zone_relative_start,
               size_t num_bits) {
 u8 *start,mask;
#ifdef CONFIG_LOG_ALLOCATIONS
 debug_printf("FREE(%p...%p)\n",
             (self->mz_min+zone_relative_start)*PAGESIZE,
             (self->mz_min+zone_relative_start+num_bits-1)*PAGESIZE);
#endif /* CONFIG_LOG_ALLOCATIONS */
#ifndef NDEBUG
 size_t i;
 for (i = 0; i < num_bits; ++i) {
  size_t bitno = zone_relative_start + i;
  assertf(MZONE_GETBIT(self,bitno) == 0,
          "Page at %p has already been freed",
         (self->mz_min+bitno)*PAGESIZE);
 }
#endif
 while (zone_relative_start & 7) {
  if (!num_bits) return;
  start   = self->mz_fpages + zone_relative_start/8;
  mask    = 1 << (zone_relative_start & 7);
  *start |= mask;
  ++zone_relative_start,--num_bits;
 }
 if (num_bits > 8) {
  /* Clear full bits. */
  start = self->mz_fpages + zone_relative_start/8;
  memset(start,0xff,num_bits/8);
  zone_relative_start += num_bits & ~7;
  num_bits            &= 7;
 }
 while (num_bits) {
  start   = self->mz_fpages + zone_relative_start/8;
  mask    = 1 << (zone_relative_start & 7);
  *start |= mask;
  ++zone_relative_start,--num_bits;
 }
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
 pageptr_t result; mzone_t zone_id;
 assert(min_pages <= max_pages);
 assert(max_zone < mzone_count);
again:
 zone_id = max_zone;
 do {
  struct mzone *zone  = mzones[zone_id];
  bool has_write_lock = false; unsigned int i;
  size_t largest_smallpage,result_size;
  struct mrange smallest_largerange;
  atomic_rwlock_read(&zone->mz_lock);
  /* @noexcept.begin */

zone_again:
  /* Search for small, viable free-ranges. */
  largest_smallpage = zone->mz_fsmall[MZONE_FSMALLANGE_COUNT-1].mr_size;
  for (i = 0; i < MZONE_FSMALLANGE_COUNT; ++i) {
   assertf(i == 0 || !zone->mz_fsmall[i].mr_size ||
          (zone->mz_fsmall[i].mr_size >=
           zone->mz_fsmall[i-1].mr_size),
           "i                            = %d\n"
           "zone->mz_fsmall[i].mr_size   = %Iu\n"
           "zone->mz_fsmall[i-1].mr_size = %Iu\n",
           i,zone->mz_fsmall[i].mr_size,
           zone->mz_fsmall[i-1].mr_size);
   if (zone->mz_fsmall[i].mr_size < min_pages) {
    if (!zone->mz_fsmall[i].mr_size) {
     largest_smallpage = i ? zone->mz_fsmall[i-1].mr_size : 0;
     break; /* No region of sufficient size. */
    }
    continue; /* Region is too small. */
   }
   if (!has_write_lock) {
    has_write_lock = true;
    if (!atomic_rwlock_upgrade(&zone->mz_lock))
         goto zone_again;
   }

   /* Found a small region of sufficient size!
    * Determine how many pages we want to allocate from the region. */
   result_size = max_pages;
   if (result_size > zone->mz_fsmall[i].mr_size)
       result_size = zone->mz_fsmall[i].mr_size;
   assert(zone->mz_free >= result_size);
   result = zone->mz_fsmall[i].mr_addr;
   *res_pages = result_size;
   zone->mz_fsmall[i].mr_addr += result_size;
   zone->mz_fsmall[i].mr_size -= result_size;
   /* Clear the free-bits from this range. */
   assertf(result             >= zone->mz_min &&
           result+result_size <= zone->mz_max+1,
           "result             = %p\n"
           "result_size        = %p\n"
           "result+result_size = %p\n"
           "zone_id            = %p\n"
           "zone               = %p\n"
           "zone->mz_min       = %p\n"
           "zone->mz_max       = %p\n"
           ,result
           ,result_size
           ,result+result_size
           ,zone_id
           ,zone
           ,zone->mz_min
           ,zone->mz_max
           );
   mzone_clear_bits(zone,result-zone->mz_min,result_size);
   /* Keep track of how much memory is being used. */
   zone->mz_used += result_size;
   zone->mz_free -= result_size;
   if (!zone->mz_fsmall[i].mr_size) {
    /* Region is now empty (move later regions forward). */
    memmove(&zone->mz_fsmall[i],
            &zone->mz_fsmall[i+1],
           (MZONE_FSMALLANGE_COUNT-(i-1))*
            sizeof(struct mrange));
    /* Zero out the last region. */
    zone->mz_fsmall[MZONE_FSMALLANGE_COUNT-1].mr_size = 0;
#ifndef NDEBUG
    memset(&zone->mz_fsmall[MZONE_FSMALLANGE_COUNT-1].mr_addr,
            0xcc,sizeof(pageptr_t));
#endif
   } else {
    /* The region is now smaller. - Check if we
     * must move it further towards the front. */
    while (i != 0) {
     struct mrange temp;
     if (zone->mz_fsmall[i-1].mr_size <=
         zone->mz_fsmall[i].mr_size)
         break;
     memcpy(&temp,&zone->mz_fsmall[i-1],sizeof(struct mrange));
     memcpy(&zone->mz_fsmall[i-1],&zone->mz_fsmall[i],sizeof(struct mrange));
     memcpy(&zone->mz_fsmall[i],&temp,sizeof(struct mrange));
     --i;
    }
   }
   atomic_rwlock_endwrite(&zone->mz_lock);
   return result;
  }
  /* All known free small ranges are too small. Instead, search the
   * entirety of the free bit-set for a sufficiently large range.
   * But first, we can quickly determine if there is even a change
   * of such a range existing by looking at the `mz_free' field.
   */
  if (zone->mz_free < min_pages)
      goto next_zone; /* There aren't even enough free pages in this zone. */

  /* Now search the free set of pages for a larger section of memory.
   * Also: collect information about additional small ranges. */
  smallest_largerange.mr_addr = 0;
  smallest_largerange.mr_size = (size_t)-1;
  result_size = 0;
  {
   uintptr_t *iter,*end,mask;
   iter = (uintptr_t *)zone->mz_fpages;
   end  = (uintptr_t *)(zone->mz_fpages+((zone->mz_max-zone->mz_min)/8)+1);
   for (; iter < end; ++iter) {
    pageptr_t free_start_page;
    size_t free_size;
    if ((mask = *iter) == 0)
         continue; /* Quick check: no free pages in these 32/64 pages. */
    /* There are at least ~some~ free pages here! */
    free_start_page = (pageptr_t)(((uintptr_t)iter - (uintptr_t)zone->mz_fpages)*8);
    mask = BSWAP_FREEMASK(mask);
parse_mask:
    while (!(mask&1)) mask >>= 1,++free_start_page;
    free_size = 0;
extend_range:
    while (mask&1) mask >>= 1,++free_size;
    if (((free_start_page+free_size) % (sizeof(uintptr_t)*8)) == 0) {
     uintptr_t next_mask;
     assertf(!mask,"mask = %p",mask);
     /* Check if the range continues into the next 32/64 pages. */
     next_mask = BSWAP_FREEMASK(iter[1]);
     if (next_mask&1) { ++iter; mask = next_mask; goto extend_range; }
    }
    if (free_size >= min_pages) {
     /* Found a range of sufficient size! (take from its front) */
     result_size = max_pages;
     if (result_size > free_size)
         result_size = free_size;
     if (!has_write_lock) {
      has_write_lock = true;
      if (!atomic_rwlock_upgrade(&zone->mz_lock))
           goto zone_again;
     }
     /* Now clear the bitrange for the newly allocated pages. */
     mzone_clear_bits(zone,free_start_page,result_size);
     /* Make the result address absolute. */
     result = free_start_page + zone->mz_min;
     /* Keep track of the smallest large-range. */
     free_size -= result_size;
     if (free_size > largest_smallpage &&
         free_size < smallest_largerange.mr_size) {
      smallest_largerange.mr_addr = free_start_page + result_size;
      smallest_largerange.mr_size = free_size;
     }
     /* Keep track of how much memory is being used. */
     zone->mz_used += result_size;
     zone->mz_free -= result_size;
     goto got_large_range;
    }
    /* Keep track of the smallest large-range. */
    if (free_size > largest_smallpage &&
        free_size < smallest_largerange.mr_size) {
     smallest_largerange.mr_addr = free_start_page;
     smallest_largerange.mr_size = free_size;
    }
    if (mask) {
     /* There are other free ranges in the currently loaded mask. */
     free_start_page += free_size;
     goto parse_mask;
    }
   }
  }

got_large_range:
  /* Save the newly collect smallest-large-range information
   * when there are unused entries in the small-range cache. */
  if (i < MZONE_FSMALLANGE_COUNT &&
      smallest_largerange.mr_size != (size_t)-1) {
   if (!has_write_lock) {
    assert(result_size == 0);
    has_write_lock = true;
    if (!atomic_rwlock_upgrade(&zone->mz_lock))
         goto zone_again;
   }
   /* Make the starting address of the smallest free-range absolute. */
   smallest_largerange.mr_addr += zone->mz_min;
   memcpy(&zone->mz_fsmall[i],&smallest_largerange,
           sizeof(struct mrange));
  }
  if (result_size != 0) {
   /* Managed to find a sufficiently sized large range. */
   assert(result_size >= min_pages);
   assert(result_size <= max_pages);
   assert(has_write_lock);
   atomic_rwlock_endwrite(&zone->mz_lock);
   *res_pages = result_size;
   return result;
  }

next_zone:
  if (has_write_lock) {
   atomic_rwlock_endwrite(&zone->mz_lock);
  } else {
   atomic_rwlock_endread(&zone->mz_lock);
  }
  /* @noexcept.end */
 } while (zone_id--);
 /* Throw a bad-allocation error. */
 {
  struct exception_info *info;
  info = error_info();
  info->e_code = E_BADALLOC;
  info->e_flag = ERR_FNORMAL|ERR_FCONTOK|ERR_FAFTERFAULT;
  memset(&info->__e_data,0,EXCEPTION_INFO_SIZEOF_DATA);
  info->e_badalloc.ba_resource = RESOURCE_TYPE_PHYSMEMORY;
  info->e_badalloc.ba_amount   = min_pages * PAGESIZE;
  error_throw_current();
  /* Allow the exception to be continued, in which
   * case the allocation is attempted again. */
  goto again;
 }
}



PRIVATE void KCALL
mzone_free(struct mzone *__restrict zone,
           pageptr_t zone_realtive_page,
           size_t num_pages) {
 unsigned int i;
 pageptr_t relative_end;
 assert(num_pages != 0);
 assertf(zone_realtive_page+num_pages <=
        ((zone->mz_max+1)-zone->mz_min),
        "zone_realtive_page = %p\n"
        "num_pages          = %p\n"
        "zone->mz_min       = %p\n"
        "zone->mz_max       = %p\n",
        zone_realtive_page,num_pages,
        zone->mz_min,zone->mz_max);
#if 0
 debug_printf("FREE(%p...%p)\n",
             (zone->mz_min+zone_realtive_page)*PAGESIZE,
             (zone->mz_min+zone_realtive_page+num_pages)*PAGESIZE-1);
#endif
 atomic_rwlock_write(&zone->mz_lock);
 /* Set all the bits within the range back to free. */
 mzone_set_bits(zone,zone_realtive_page,num_pages);
 /* Invalidate free-range caches for any region
  * touching, or overlapping with this one.
  * This must be done because we can no longer guaranty
  * that they can still be considered to be small. */
 relative_end = zone_realtive_page + num_pages;
 /* XXX: This could be optimized by looking at the free-state of
  *      the pages immediately before and after the free-range. */
 for (i = 0; i < MZONE_FSMALLANGE_COUNT; ++i) {
  pageptr_t range_end;
recheck_range:
  if (!zone->mz_fsmall[i].mr_size) break; /* End of the small-page cache list. */
  if (zone->mz_fsmall[i].mr_addr > relative_end)
      continue; /* Range starts above the end of the free-range. */
  range_end  = zone->mz_fsmall[i].mr_addr;
  range_end += zone->mz_fsmall[i].mr_size;
  if (range_end < zone_realtive_page && range_end != 0)
      continue; /* Range ends before the start of the free-range. */
  /* Delete this range. */
  memmove(&zone->mz_fsmall[i],&zone->mz_fsmall[i+1],
         ((MZONE_FSMALLANGE_COUNT-1)-i)*sizeof(struct mrange));
  /* Clear out the last range that got invalidated. */
  memset(&zone->mz_fsmall[MZONE_FSMALLANGE_COUNT-1],0,sizeof(struct mrange));
  goto recheck_range;
 }
 /* Track the total number of free/used pages. */
 zone->mz_used -= num_pages; /* NOTE: This is allowed to underflow during initialization. */
 zone->mz_free += num_pages;
 atomic_rwlock_endwrite(&zone->mz_lock);
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



PRIVATE bool KCALL
mzone_malloc_at(struct mzone *__restrict zone,
                pageptr_t zone_realtive_page,
                size_t num_pages) {
 pageptr_t relative_end,iter;
 unsigned int i;
 bool has_write_lock = false;
 assert(num_pages != 0);
 assert(zone_realtive_page+num_pages <=
       ((zone->mz_max+1)-zone->mz_min));
 relative_end = zone_realtive_page + num_pages;
 atomic_rwlock_read(&zone->mz_lock);
 /* Quick check: are there even enough 1-bits at all? */
 if unlikely(zone->mz_free < num_pages)
    goto nope;
check_free_bits:

 /* Check all the free-bits for the requested pages.
  * XXX: This could be optimized to check 32/64 bits at once for large request. */
 for (iter = zone_realtive_page;
      iter < relative_end; ++iter) {
  if (!MZONE_GETBIT(zone,iter))
       goto nope;
 }

 /* Upgrade to use a write-lock. */
 if (!has_write_lock) {
  has_write_lock = true;
  if (!atomic_rwlock_upgrade(&zone->mz_lock))
       goto check_free_bits;
 }

 /* Clear all the free-bits for the requested range. */
 mzone_clear_bits(zone,zone_realtive_page,num_pages);

 /* Search for a free range that contained the allocated range. */
 for (i = 0; i < MZONE_FSMALLANGE_COUNT; ++i) {
  pageptr_t range_end;
recheck_range:
  if (!zone->mz_fsmall[i].mr_size) break;
  if (zone->mz_fsmall[i].mr_addr >= relative_end) continue;
  range_end  = zone->mz_fsmall[i].mr_addr;
  range_end += zone->mz_fsmall[i].mr_size;
  if (range_end <= zone_realtive_page && range_end != 0)
      continue; /* Range ends before the start of the free-range. */
  /* XXX: If we are only scraping the region, we could truncate it
   *      and move it closer to the back of the free-range list. */
  /* Delete this range. */
  memmove(&zone->mz_fsmall[i],&zone->mz_fsmall[i+1],
         ((MZONE_FSMALLANGE_COUNT-1)-i)*sizeof(struct mrange));
  /* Clear out the last range that got invalidated. */
  memset(&zone->mz_fsmall[MZONE_FSMALLANGE_COUNT-1],0,sizeof(struct mrange));
  goto recheck_range;
 }
 assert(zone->mz_free >= num_pages);
 /* Update zone usage statistics. */
 zone->mz_used += num_pages;
 zone->mz_free -= num_pages;
 assert(has_write_lock);
 atomic_rwlock_endwrite(&zone->mz_lock);
 return true;
nope:
 if (has_write_lock) {
  atomic_rwlock_endwrite(&zone->mz_lock);
 } else {
  atomic_rwlock_endread(&zone->mz_lock);
 }
 return false;
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
#endif /* !CONFIG_USE_NEW_PAGEMALLOC */

#endif /* !GUARD_KERNEL_SRC_KERNEL_MEMORY_C */
