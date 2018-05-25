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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_PAGING_H
#define GUARD_KERNEL_INCLUDE_KERNEL_PAGING_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <stdbool.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/paging.h>
#else
#error "Unsupported architecture"
#endif

DECL_BEGIN

/* Convert addresses/sizes to/from whole pages. */
#ifndef VM_ADDR2PAGE
#define VM_ADDR2PAGE(x)  ((x)/PAGEALIGN)
#define VM_PAGE2ADDR(x)  ((x)*PAGEALIGN)
#endif
#ifndef VM_SIZE2PAGES
#define VM_SIZE2PAGES(x) ((x)/PAGESIZE)
#define VM_PAGES2SIZE(x) ((x)*PAGESIZE)
#endif

#ifdef __CC__

/* The physical and virtual address bindings of the kernel's page directory.
 * This is the initial page directory active when KOS boots, as well as the
 * the directory later used for thread running in kernel-space only. */
DATDEF VIRT pagedir_t pagedir_kernel;
DATDEF PHYS pagedir_t pagedir_kernel_phys;

/* Initialize the given page directory.
 * The caller is required to allocate the page directory
 * controller itself, which must be aligned and sized
 * according to `PAGEDIR_ALIGN' and `PAGEDIR_SIZE'.
 * @throw E_BADALLOC: Not enough available memory. */
#ifdef CONFIG_PAGEDIR_INIT_IS_NOEXCEPT
FUNDEF ATTR_NOTHROW void KCALL
#else
FUNDEF void KCALL
#endif
pagedir_init(VIRT pagedir_t *__restrict self,
             PHYS vm_phys_t phys_self);

/* Finalize a given page directory. */
FUNDEF ATTR_NOTHROW void KCALL pagedir_fini(VIRT pagedir_t *__restrict self);


/* WARNING: If the host does not support some combination of permission
 *          bits, more permissions than specified may be granted.
 *          For example: On X86, write-permissions require read-permissions
 *                       to be granted, and exec-permissions require read. */
#define PAGEDIR_MAP_FEXEC  0x0001 /* Permission bit: Allow execution within the mapping. */
#define PAGEDIR_MAP_FWRITE 0x0002 /* Permission bit: Permit write-access to memory within mapping. */
#define PAGEDIR_MAP_FREAD  0x0004 /* Permission bit: Permit read-access to memory within mapping. */
#define PAGEDIR_MAP_FUSER  0x0008 /* Permission bit: Permit the user access to the mapping. */
#define PAGEDIR_MAP_FUNMAP 0x1000 /* Delete the mapping (permission bits are ignored) */

/* Create/delete a page-directory mapping.
 * @param: perm: A set of `PAGEDIR_MAP_F*' detailing how memory should be mapped.
 * @throw E_BADALLOC: Not enough available memory.
 * `pagedir_sync()' must be called while specifying a virtual address range containing
 * `virt_page...+=num_pages' in order to ensure that changes will become visible.
 * NOTE: This function can be called regardless of which page directory is active. */
FUNDEF void KCALL pagedir_map(VIRT vm_vpage_t virt_page, size_t num_pages,
                              PHYS vm_ppage_t phys_page, u16 perm);
/* Map a single page of physical memory. */
FUNDEF void KCALL pagedir_mapone(VIRT vm_vpage_t virt_page,
                                 PHYS vm_ppage_t phys_page, u16 perm);
/* Synchronize mappings within the given address range. */
FUNDEF void FCALL pagedir_sync(VIRT vm_vpage_t virt_page, size_t num_pages);
FUNDEF void FCALL pagedir_syncone(VIRT vm_vpage_t virt_page);
/* Synchronize the entirety of the current page directory. */
FUNDEF void FCALL pagedir_syncall(void);

/* Translate a virtual address into its physical counterpart. */
FUNDEF PHYS vm_phys_t KCALL pagedir_translate(VIRT vm_virt_t virt_addr);

/* Check if the given page is mapped. */
FUNDEF ATTR_NOTHROW bool KCALL pagedir_ismapped(vm_vpage_t vpage);
FUNDEF ATTR_NOTHROW bool KCALL pagedir_iswritable(vm_vpage_t vpage);
FUNDEF ATTR_NOTHROW bool KCALL pagedir_isuseraccessible(vm_vpage_t vpage);

#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_PAGING_H */
