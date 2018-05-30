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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_SECTIONS_H
#define GUARD_KERNEL_INCLUDE_KERNEL_SECTIONS_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/sections.h>
#else
#error "Unsupported architecture"
#endif

DECL_BEGIN

#ifdef __CC__

#ifndef LOAD_FAR_POINTER
#define LOAD_FAR_POINTER(x) ((__UINTPTR_TYPE__)(x))
#endif


#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Section addresses within the kernel core. */

/* .text:
 *    - .text.hot
 *    - .text
 *    - .text.cold
 */
INTDEF byte_t kernel_text_start[];    /* Page-aligned text start address */
INTDEF byte_t kernel_text_end_raw[];  /* Unaligned text end address */
INTDEF byte_t kernel_text_size_raw[]; /* Unaligned text size (in bytes) */
INTDEF byte_t kernel_text_end[];      /* Page-aligned text end address */
INTDEF byte_t kernel_text_size[];     /* Page-aligned text size (in bytes) */

/* .rodata:
 *    - .rodata.hot
 *    - .rodata
 *    - .rodata.st
 *    - .rodata.cold
 * #ifndef CONFIG_NO_SMP
 *    - .data.percpu.head
 *    - .data.percpu
 * #endif
 *    - .data.pertask.head
 *    - .data.pertask
 *    - .data.pervm.head
 *    - .data.pervm
 */
INTDEF byte_t kernel_rodata_start[];    /* Page-aligned rodata start address */
INTDEF byte_t kernel_rodata_end_raw[];  /* Unaligned rodata end address */
INTDEF byte_t kernel_rodata_size_raw[]; /* Unaligned rodata size (in bytes) */
INTDEF byte_t kernel_rodata_end[];      /* Page-aligned rodata end address */
INTDEF byte_t kernel_rodata_size[];     /* Page-aligned rodata size (in bytes) */

#ifndef CONFIG_NO_SMP
/* .data.percpu.head & .data.percpu  (Per-CPU data template) */
INTDEF byte_t kernel_percpu_start[];
INTDEF byte_t kernel_percpu_end[];
INTDEF byte_t kernel_percpu_size[];
#endif

/* .data.pertask.head & .data.pertask  (Per-task data template) */
INTDEF byte_t kernel_pertask_start[];
INTDEF byte_t kernel_pertask_end[];
INTDEF byte_t kernel_pertask_size[];

/* .data.pervm.head & .data.pervm  (Per-VM data template) */
INTDEF byte_t kernel_pervm_start[];
INTDEF byte_t kernel_pervm_end[];
INTDEF byte_t kernel_pervm_size[];


/* .data:
 *    - .data.hot
 *    - .data
 *    - .symbols
 *    - .data.cold
 * #ifdef CONFIG_NO_SMP
 *    - .data.percpu.head
 *    - .data.percpu
 * #endif
 */
INTDEF byte_t kernel_data_start[];
INTDEF byte_t kernel_data_end[];
INTDEF byte_t kernel_data_size[];

#ifdef CONFIG_NO_SMP
/* .data.percpu.head & .data.percpu  (Per-CPU data) */
INTDEF byte_t kernel_percpu_start[];
INTDEF byte_t kernel_percpu_end[];
INTDEF byte_t kernel_percpu_size[];
#endif


/* .bss:
 *    - .bss.hot
 *    - .bss
 *    - .bss.cold
 */
INTDEF byte_t kernel_bss_start[];     /* Page-aligned bss start address */
INTDEF byte_t kernel_bss_end_raw[];   /* Unaligned bss end address */
INTDEF byte_t kernel_bss_size_raw[];  /* Unaligned bss size (in bytes) */
INTDEF byte_t kernel_bss_end[];       /* Page-aligned bss end address */
INTDEF byte_t kernel_bss_size[];      /* Page-aligned bss size (in bytes) */


/* .free:
 *    - .text.free
 *    - .rodata.free
 *    - .rodata.str.free
 *    - .data.free
 *    - .bss.free */
INTDEF byte_t kernel_free_start[];    /* Page-aligned free start address */
INTDEF byte_t kernel_free_end_raw[];  /* Unaligned free end address */
INTDEF byte_t kernel_free_size_raw[]; /* Unaligned free size (in bytes) */
INTDEF byte_t kernel_free_end[];      /* Page-aligned free end address */
INTDEF byte_t kernel_free_size[];     /* Page-aligned free size (in bytes) */




/* XR-  - Execute/read kernel segment (.text). */
INTDEF byte_t kernel_rx_start[];      /* Page-aligned read/exec start address */
INTDEF byte_t kernel_rx_end_raw[];    /* Unaligned read/exec end address */
INTDEF byte_t kernel_rx_size_raw[];   /* Unaligned read/exec size (in bytes) */
INTDEF byte_t kernel_rx_end[];        /* Page-aligned read/exec end address */
INTDEF byte_t kernel_rx_size[];       /* Page-aligned read/exec size (in bytes) */
INTDEF byte_t kernel_rx_minpage[];    /* The lowest read/exec page number (vm_vpage_t) */
INTDEF byte_t kernel_rx_maxpage[];    /* The greater read/exec page number (vm_vpage_t) (kernel_rx_minpage + kernel_rx_num_pages - 1) */
INTDEF byte_t kernel_rx_num_pages[];  /* The number of read/exec pages. */

/* -R-  - Read-only kernel segment (.text & .rodata). */
INTDEF byte_t kernel_ro_start[];      /* Page-aligned read-only start address */
INTDEF byte_t kernel_ro_end_raw[];    /* Unaligned read-only end address */
INTDEF byte_t kernel_ro_size_raw[];   /* Unaligned read-only size (in bytes) */
INTDEF byte_t kernel_ro_end[];        /* Page-aligned read-only end address */
INTDEF byte_t kernel_ro_size[];       /* Page-aligned read-only size (in bytes) */
INTDEF byte_t kernel_ro_minpage[];    /* The lowest read-only page number (vm_vpage_t) */
INTDEF byte_t kernel_ro_maxpage[];    /* The greater read-only page number (vm_vpage_t) (kernel_ro_minpage + kernel_ro_num_pages - 1) */
INTDEF byte_t kernel_ro_num_pages[];  /* The number of read-only pages. */

/* -R-  - Read-only, no-execute kernel segment (.rodata). */
INTDEF byte_t kernel_ronx_start[];    /* Page-aligned read-only start address */
INTDEF byte_t kernel_ronx_end_raw[];  /* Unaligned read-only, no-execute end address */
INTDEF byte_t kernel_ronx_size_raw[]; /* Unaligned read-only, no-execute size (in bytes) */
INTDEF byte_t kernel_ronx_end[];      /* Page-aligned read-only, no-execute end address */
INTDEF byte_t kernel_ronx_size[];     /* Page-aligned read-only, no-execute size (in bytes) */
INTDEF byte_t kernel_ronx_minpage[];  /* The lowest read-only, no-execute page number (vm_vpage_t) */
INTDEF byte_t kernel_ronx_maxpage[];  /* The greater read-only, no-execute page number (vm_vpage_t) (kernel_ro_minpage + kernel_ro_num_pages - 1) */
INTDEF byte_t kernel_ronx_num_pages[];/* The number of read-only, no-execute pages. */

/* -RW - Read/write kernel segment (.xdata, .data, .bss, .free).
 * WARNING: The .free portion must be executable. */
INTDEF byte_t kernel_rw_start[];      /* Page-aligned read/write start address */
INTDEF byte_t kernel_rw_end_raw[];    /* Unaligned read/write end address */
INTDEF byte_t kernel_rw_size_raw[];   /* Unaligned read/write size (in bytes) */
INTDEF byte_t kernel_rw_end[];        /* Page-aligned read/write end address */
INTDEF byte_t kernel_rw_size[];       /* Page-aligned read/write size (in bytes) */
INTDEF byte_t kernel_rw_minpage[];    /* The lowest read/write page number (vm_vpage_t) */
INTDEF byte_t kernel_rw_maxpage[];    /* The greater read/write page number (vm_vpage_t) (kernel_rw_minpage + kernel_rw_num_pages - 1) */
INTDEF byte_t kernel_rw_num_pages[];  /* The number of read/write pages. */

/* XRW - Read/write/execute kernel segment (.xdata). */
INTDEF byte_t kernel_rwx_start[];      /* Page-aligned read/write/exec start address */
INTDEF byte_t kernel_rwx_end_raw[];    /* Unaligned read/write/exec end address */
INTDEF byte_t kernel_rwx_size_raw[];   /* Unaligned read/write/exec size (in bytes) */
INTDEF byte_t kernel_rwx_end[];        /* Page-aligned read/write/exec end address */
INTDEF byte_t kernel_rwx_size[];       /* Page-aligned read/write/exec size (in bytes) */
INTDEF byte_t kernel_rwx_minpage[];    /* The lowest read/write/exec page number (vm_vpage_t) */
INTDEF byte_t kernel_rwx_maxpage[];    /* The greater read/write/exec page number (vm_vpage_t) (kernel_rw_minpage + kernel_rw_num_pages - 1) */
INTDEF byte_t kernel_rwx_num_pages[];  /* The number of read/write/exec pages. */

/* -RW - Read/write kernel segment (.xdata, .data, .bss). */
INTDEF byte_t kernel_rwnf_start[];    /* Page-aligned read/write start address */
INTDEF byte_t kernel_rwnf_end_raw[];  /* Unaligned read/write end address */
INTDEF byte_t kernel_rwnf_size_raw[]; /* Unaligned read/write size (in bytes) */
INTDEF byte_t kernel_rwnf_end[];      /* Page-aligned read/write end address */
INTDEF byte_t kernel_rwnf_size[];     /* Page-aligned read/write size (in bytes) */
INTDEF byte_t kernel_rwnf_minpage[];  /* The lowest read/write page number (vm_vpage_t) */
INTDEF byte_t kernel_rwnf_maxpage[];  /* The greater read/write page number (vm_vpage_t) (kernel_rwnf_minpage + kernel_rwnf_num_pages - 1) */
INTDEF byte_t kernel_rwnf_num_pages[];/* The number of read/write pages. */

/* XRW  - Read/write/exec kernel free segment (.free). */
INTDEF byte_t kernel_free_minpage[];  /* The lowest free page number (vm_vpage_t) */
INTDEF byte_t kernel_free_maxpage[];  /* The greater free page number (vm_vpage_t) */
INTDEF byte_t kernel_free_num_pages[];/* The number of free pages. */


/* _everything_ */
INTDEF byte_t kernel_start[];         /* Page-aligned kernel start address */
INTDEF byte_t kernel_end_raw[];       /* Unaligned kernel end address */
INTDEF byte_t kernel_size_raw[];      /* Unaligned kernel size (in bytes) */
INTDEF byte_t kernel_end[];           /* Page-aligned kernel end address */
INTDEF byte_t kernel_size[];          /* Page-aligned kernel size (in bytes) */
#endif /* CONFIG_BUILDING_KERNEL_CORE */

#define ATTR_PERCPU    ATTR_SECTION(".data.percpu")  /* Per-cpu template data. */
#define ATTR_PERVM     ATTR_SECTION(".data.pervm")   /* Per-virtual-memory template data. */
#define ATTR_PERTASK   ATTR_SECTION(".data.pertask") /* Per-task template data. */
#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_SECTIONS_H */
