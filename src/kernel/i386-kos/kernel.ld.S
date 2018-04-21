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
#ifndef __INTELLISENSE__
#include <hybrid/compiler.h>
#include <hybrid/limits.h>
#include <kernel/paging.h>
#include <kernel/malloc.h>
#include <kos/ushare.h>
#include <proprietary/multiboot.h>
#include <proprietary/multiboot2.h>
#include <i386-kos/interrupt.h>
#include <i386-kos/gdt.h>
#endif


/* LD is a bit confused about hidden symbols defined in kernel.ld
 * Help out a bit by explicitly marking them as hidden. */

.hidden kernel_multiboot2_start
.hidden kernel_multiboot2_tag_start
.hidden kernel_multiboot2_end
.hidden kernel_multiboot2_size
#if __SIZEOF_POINTER__ >= 8
.hidden kernel_multiboot2_hdrlen_chksum
#else
.hidden kernel_multiboot2_hdrlen
.hidden kernel_multiboot2_chksum
#endif
.hidden kernel_start
.hidden kernel_rx_start
.hidden kernel_ro_start
.hidden kernel_text_start
.hidden kernel_text_end_raw
.hidden kernel_rx_end_raw
.hidden kernel_text_end
.hidden kernel_text_size_raw
.hidden kernel_text_size
.hidden kernel_rx_end
.hidden kernel_rx_size_raw
.hidden kernel_rx_size
.hidden kernel_ronx_start
.hidden kernel_rodata_start
.hidden pertask_init_start
.hidden pertask_init_end
.hidden pertask_fini_start
.hidden pertask_fini_end
.hidden pertask_startup_start
.hidden pertask_startup_end
.hidden pertask_cleanup_start
.hidden pertask_cleanup_end
.hidden pertask_clone_start
.hidden pertask_clone_end
.hidden pervm_init_start
.hidden pervm_init_end
.hidden pervm_fini_start
.hidden pervm_fini_end
.hidden pervm_clone_start
.hidden pervm_clone_end
.hidden pervm_unmapuser_start
.hidden pervm_unmapuser_end
.hidden kernel_except_start
.hidden kernel_except_end
.hidden kernel_except_size
.hidden kernel_ehframe_start
.hidden kernel_ehframe_end
.hidden kernel_ehframe_size
.hidden kernel_percpu_start
.hidden kernel_percpu_end
.hidden kernel_percpu_size
.hidden kernel_pertask_start
.hidden kernel_pertask_end
.hidden kernel_pertask_size
.hidden kernel_pervm_start
.hidden kernel_pervm_end
.hidden kernel_pervm_size
.hidden kernel_rodata_end_raw
.hidden kernel_ro_end_raw
.hidden kernel_ronx_end_raw
.hidden kernel_rodata_end
.hidden kernel_rodata_size_raw
.hidden kernel_rodata_size
.hidden kernel_ro_end
.hidden kernel_ro_size_raw
.hidden kernel_ro_size
.hidden kernel_ronx_end
.hidden kernel_ronx_size_raw
.hidden kernel_ronx_size
.hidden kernel_rw_start
.hidden kernel_rwx_start
.hidden kernel_rwnf_start
.hidden kernel_data_start
.hidden kernel_rwx_end_raw
.hidden kernel_rwx_end
.hidden kernel_rwnx_start
#ifndef CONFIG_NO_DEBUG_MALLOC
.hidden mall_tracked_start
#endif
.hidden _idt_start
.hidden kernel_debug_line_start
.hidden kernel_debug_line_end
.hidden kernel_debug_line_size
.hidden kernel_data_end
.hidden kernel_data_size
.hidden x86_ushare_sysenter_pageno
.hidden ushare_strerror_pageno
.hidden ushare_utsname_pageno
.hidden kernel_rwx_size_raw
.hidden kernel_rwx_size
.hidden kernel_rwnx_end
.hidden kernel_rwnx_size
.hidden kernel_bss_start
.hidden x86_boot_stack_guard_addr
.hidden x86_boot_stack_guard_page
#ifndef CONFIG_NO_SMP
.hidden boot_cpu_start
#endif /* !CONFIG_NO_SMP */
.hidden boot_task_start
.hidden boot_vm_start
.hidden mall_tracked_end
#ifndef CONFIG_NO_DEBUG_MALLOC
.hidden kernel_bss_end_raw
#endif
.hidden kernel_rwnf_end_raw
.hidden kernel_bss_end
.hidden kernel_bss_size_raw
.hidden kernel_bss_size
.hidden kernel_rwnf_end
.hidden kernel_rwnf_size_raw
.hidden kernel_rwnf_size
.hidden kernel_free_start
.hidden kernel_coredriver_preinit_start
.hidden kernel_coredriver_preinit_end
.hidden kernel_coredriver_init_start
.hidden kernel_coredriver_init_end
.hidden kernel_free_end_raw
.hidden kernel_rw_end_raw
.hidden kernel_end_raw
.hidden kernel_free_end
.hidden kernel_free_size_raw
.hidden kernel_free_size
.hidden kernel_rw_end
.hidden kernel_rw_size_raw
.hidden kernel_rw_size
.hidden kernel_end
.hidden kernel_size_raw
.hidden kernel_size
.hidden _boot_page0_size
.hidden _boot_page1_size
.hidden _boot_page2_size
.hidden _x86_gdt_tss_lo
.hidden _x86_gdt_tss_hi
.hidden _x86_gdt_tssdf_lo
.hidden _x86_gdt_tssdf_hi
.hidden _x86_gdt_tls_lo
.hidden _x86_gdt_tls_hi
.hidden kernel_rx_minpage
.hidden kernel_rx_maxpage
.hidden kernel_rx_num_pages
.hidden kernel_ro_minpage
.hidden kernel_ro_maxpage
.hidden kernel_ro_num_pages
.hidden kernel_ronx_minpage
.hidden kernel_ronx_maxpage
.hidden kernel_ronx_num_pages
.hidden kernel_rw_minpage
.hidden kernel_rw_maxpage
.hidden kernel_rw_num_pages
.hidden kernel_rwx_minpage
.hidden kernel_rwx_maxpage
.hidden kernel_rwx_num_pages
.hidden kernel_rwnf_minpage
.hidden kernel_rwnf_maxpage
.hidden kernel_rwnf_num_pages
.hidden kernel_free_minpage
.hidden kernel_free_maxpage
.hidden kernel_free_num_pages

.global vm_kernel
.global pagedir_kernel
.global pagedir_kernel_phys
