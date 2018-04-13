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
#ifndef GUARD_KERNEL_SRC_FS_RECENT_MODULES_C
#define GUARD_KERNEL_SRC_FS_RECENT_MODULES_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/debug.h>
#include <hybrid/align.h>
#include <kernel/heap.h>
#include <kernel/vm.h>
#include <kernel/malloc.h>
#include <kernel/sections.h>
#include <fs/linker.h>
#include <string.h>
#include <except.h>

#ifndef CONFIG_NO_RECENT_MODULES

DECL_BEGIN

#ifndef CONFIG_RECENT_MODULES_CACHESIZE
#define CONFIG_RECENT_MODULES_CACHESIZE  32
#endif


/* Cache of recently used modules. */
PRIVATE DEFINE_ATOMIC_RWLOCK(recent_modules_lock);
PRIVATE REF struct module *recent_modules[CONFIG_RECENT_MODULES_CACHESIZE];
PRIVATE unsigned int recent_modules_count = 0;


/* Cache the given module as recently-used.
 * NOTE: This function is automatically called by `application_load()'
 * The recently-used module cache is designed to keep metadata required
 * to load some module that is being used quite often loaded in memory.
 * That way, applications using the module can quickly be loaded again,
 * without the need of re-loading all associated meta-data.
 * This cache operates similar to the block-page cache used by block_device,
 * meaning that modules used more often than others will stay in cache longer
 * before being discarded new new modules that have been used even more.
 * XXX: This could be used to generate a sort-of heatmap of binaries
 *      executed more often than others in a running system... */
PUBLIC void KCALL
module_recent(struct module *__restrict mod) {
 unsigned int i;
 atomic_rwlock_write(&recent_modules_lock);
 if (mod->m_recent != (u16)-1) ++mod->m_recent;
 if (mod->m_recent != 1) goto done; /* Already cached as recently used. */
again:
 /* Add this module to the cache. */
 assert(recent_modules_count <= CONFIG_RECENT_MODULES_CACHESIZE);
 if unlikely(recent_modules_count == CONFIG_RECENT_MODULES_CACHESIZE) {
  /* Cache is full. -> Remove entries that were used the least. */
  u16 min_use = (u16)-1,max_use = 0,avg_use;
  REF struct module *removed_modules[CONFIG_RECENT_MODULES_CACHESIZE];
  unsigned int num_removed = 0;
  for (i = 0; i < CONFIG_RECENT_MODULES_CACHESIZE; ++i) {
   struct module *mod = recent_modules[i];
   if (min_use > mod->m_recent)
       min_use = mod->m_recent;
   if (max_use < mod->m_recent)
       max_use = mod->m_recent;
  }
  avg_use = min_use + CEILDIV(max_use - min_use,2);
  i = 0;
check_modules:
  for (; i < recent_modules_count; ++i) {
   struct module *mod = recent_modules[i];
   if (mod->m_recent > avg_use) {
    /* This module stays (Subtract a portion of the average use). */
    mod->m_recent -= CEILDIV(avg_use,2);
   } else {
    /* This module must go. */
    removed_modules[num_removed++] = mod;
    mod->m_recent = 0;
    --recent_modules_count;
    /* Shift to take up the slot that got removed. */
    memmove(recent_modules+i,recent_modules+i+1,
           (recent_modules_count-i)*
            sizeof(REF struct module *));
    goto check_modules;
   }
  }
  /* Drop references from all modules that were removed. */
  atomic_rwlock_endwrite(&recent_modules_lock);
  while (num_removed--)
      module_decref(removed_modules[num_removed]);
  atomic_rwlock_write(&recent_modules_lock);
  goto again;
 }
 /* Add the new module to the cache. */
 recent_modules[recent_modules_count++] = mod;
 module_incref(mod);
done:
 atomic_rwlock_endwrite(&recent_modules_lock);
}


/* Clear the cache of recently-used modules. */
PUBLIC void KCALL module_clear_recent(void) {
 REF struct module *old_cache[CONFIG_RECENT_MODULES_CACHESIZE];
 unsigned int count;
 atomic_rwlock_write(&recent_modules_lock);
 count = recent_modules_count;
 recent_modules_count = 0;
 memcpy(old_cache,recent_modules,count*
        sizeof(REF struct module *));
 atomic_rwlock_endwrite(&recent_modules_lock);
 while (count--) module_decref(old_cache[count]);
}



DECL_END

#endif /* !CONFIG_NO_RECENT_MODULES */

#endif /* !GUARD_KERNEL_SRC_FS_RECENT_MODULES_C */
