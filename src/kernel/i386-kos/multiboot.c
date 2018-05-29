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
#ifndef GUARD_KERNEL_I386_KOS_MULTIBOOT_C
#define GUARD_KERNEL_I386_KOS_MULTIBOOT_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/section.h>
#include <fs/driver.h>
#include <kernel/debug.h>
#include <kernel/memory.h>
#include <kernel/sections.h>
#include <proprietary/multiboot.h>
#include <proprietary/multiboot2.h>
#include <string.h>
#include <except.h>

DECL_BEGIN

INTERN ATTR_FREERODATA u8 const memtype_bios_matrix[6] = {
    [0] = MEMTYPE_NDEF,   /* Undefined (Fallback). */
    [1] = MEMTYPE_RAM,    /* Available. */
    [2] = MEMTYPE_DEVICE, /* Reserved. */
    [3] = MEMTYPE_COUNT,  /* ACPI-Reclaimable. (Ignored) */
    [4] = MEMTYPE_NVS,    /* NVS. */
    [5] = MEMTYPE_BADRAM, /* Badram. */
};

INTERN ATTR_FREETEXT void KCALL
x86_load_mb1info(PHYS mb_info_t *__restrict info) {
 VIRT mb_info_t *vinfo = X86_EARLY_PHYS2VIRT(info);
 /* Save the kernel commandline pointer (will be loaded later) */
 kernel_driver.d_cmdline = (char *)(uintptr_t)vinfo->cmdline;
 if (kernel_driver.d_cmdline) {
  kernel_driver.d_cmdline = X86_EARLY_PHYS2VIRT(kernel_driver.d_cmdline);
  if (!kernel_driver.d_cmdline[0])
       kernel_driver.d_cmdline = NULL;
  else {
   mem_install((uintptr_t)X86_EARLY_VIRT2PHYS(kernel_driver.d_cmdline),
                strlen(kernel_driver.d_cmdline),
                MEMTYPE_PRESERVE);
  }
 }

 if (vinfo->flags & MB_INFO_MEM_MAP) {
  mb_memory_map_t *iter,*end;
  mem_install(vinfo->mmap_addr,vinfo->mmap_length,MEMTYPE_PRESERVE);
  iter = (struct mb_mmap_entry *)(uintptr_t)vinfo->mmap_addr;
  end  = (mb_memory_map_t *)((uintptr_t)iter+vinfo->mmap_length);
  iter = X86_EARLY_PHYS2VIRT(iter);
  end  = X86_EARLY_PHYS2VIRT(end);
  for (; iter < end; iter = (mb_memory_map_t *)((uintptr_t)&iter->addr+iter->size)) {
   if (iter->type >= COMPILER_LENOF(memtype_bios_matrix)) iter->type = 0;
   if (memtype_bios_matrix[iter->type] >= MEMTYPE_COUNT) continue;
   mem_install(iter->addr,iter->len,memtype_bios_matrix[iter->type]);
  }
 }
}

INTERN ATTR_FREETEXT void KCALL
x86_load_mb2info(PHYS u8 *__restrict info) {
 /* TODO */
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_MULTIBOOT_C */
