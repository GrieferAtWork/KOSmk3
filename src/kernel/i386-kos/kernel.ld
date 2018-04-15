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

#define SECTION(name) \
	name BLOCK(1) : AT(ADDR(name) - KERNEL_BASE)
#define PHYSICAL_SECTION(name) \
	name BLOCK(1) : AT(ADDR(name))
#define DEFINE_PEROBJECT_SECTION_EX(section_name,name,base_offset,sections) \
	PROVIDE_HIDDEN(name##_start = ABSOLUTE(.)); \
	. = ABSOLUTE(base_offset); \
	section_name base_offset : AT(name##_start - KERNEL_BASE) sections \
	PROVIDE_HIDDEN(name##_size = ABSOLUTE(. - base_offset)); \
	. = ABSOLUTE(name##_start + name##_size); \
	PROVIDE_HIDDEN(name##_end = ABSOLUTE(.));
#define DEFINE_PEROBJECT_SECTION(section_name,name,sections) \
	DEFINE_PEROBJECT_SECTION_EX(section_name,name,0,sections)


#define IP_ALIGN(x) . = ALIGN(x)

ENTRY(_start)
SECTIONS {

	. = KERNEL_BASE + 1M;

	/* Boot header */
	SECTION(.multiboot) {
		KEEP(*(.multiboot))
		PROVIDE_HIDDEN(kernel_multiboot2_begin = ABSOLUTE(.));
		KEEP(*(.multiboot2))
		PROVIDE_HIDDEN(kernel_multiboot2_tag_begin = ABSOLUTE(.));
		KEEP(*(.multiboot2.tag))
		KEEP(*(.multiboot2.tag.end))
		PROVIDE_HIDDEN(kernel_multiboot2_end = ABSOLUTE(.));
		PROVIDE_HIDDEN(kernel_multiboot2_size = ABSOLUTE(kernel_multiboot2_end - kernel_multiboot2_begin));
#if __SIZEOF_POINTER__ >= 8
		PROVIDE_HIDDEN(kernel_multiboot2_hdrlen_chksum =
		              (ABSOLUTE(kernel_multiboot2_size) |
		              (ABSOLUTE(0 - (MB2_HEADER_MAGIC + MB2_ARCHITECTURE + kernel_multiboot2_size)) << 32)));
#else
		PROVIDE_HIDDEN(kernel_multiboot2_hdrlen = ABSOLUTE(kernel_multiboot2_size));
		PROVIDE_HIDDEN(kernel_multiboot2_chksum = ABSOLUTE(0 - (MB2_HEADER_MAGIC + MB2_ARCHITECTURE + kernel_multiboot2_size)));
#endif
	}

	/* Text segment */
	IP_ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(kernel_start = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_rx_start = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_ro_start = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_text_start = ABSOLUTE(.));
	SECTION(.text) {
		*(.text.interrupt_except)
		*(.text.interrupt_common)
		*(.text.interrupt_entry)
		*(.text.hot)
		*(.text.crt.except)
		*(.text.crt.string)
		*(.text)
		*(.text.rare)
		*(.text.cold)
	}
	PROVIDE_HIDDEN(kernel_text_end_raw = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_rx_end_raw = ABSOLUTE(.));
	IP_ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(kernel_text_end = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_text_size_raw = ABSOLUTE(kernel_text_end_raw - kernel_text_start));
	PROVIDE_HIDDEN(kernel_text_size = ABSOLUTE(kernel_text_end - kernel_text_start));
	PROVIDE_HIDDEN(kernel_rx_end = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_rx_size_raw = ABSOLUTE(kernel_rx_end_raw - kernel_rx_start));
	PROVIDE_HIDDEN(kernel_rx_size = ABSOLUTE(kernel_rx_end - kernel_rx_start));
	PROVIDE_HIDDEN(kernel_ronx_start = ABSOLUTE(.));

	IP_ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(kernel_rodata_start = ABSOLUTE(.));
	SECTION(.rodata) {
		*(.rodata.hot)

		IP_ALIGN(CACHELINE);
		*(.rodata)
		*(.rodata.str*)
		*(.rodata.rare)

		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(pertask_init_start = ABSOLUTE(.));
		*(.rodata.pertask.init)
		PROVIDE_HIDDEN(pertask_init_end = ABSOLUTE(.));

		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(pertask_fini_start = ABSOLUTE(.));
		*(.rodata.pertask.fini)
		PROVIDE_HIDDEN(pertask_fini_end = ABSOLUTE(.));

		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(pertask_startup_start = ABSOLUTE(.));
		*(.rodata.pertask.startup)
		PROVIDE_HIDDEN(pertask_startup_end = ABSOLUTE(.));

		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(pertask_cleanup_start = ABSOLUTE(.));
		*(.rodata.pertask.cleanup)
		PROVIDE_HIDDEN(pertask_cleanup_end = ABSOLUTE(.));

		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(pertask_clone_start = ABSOLUTE(.));
		*(.rodata.pertask.clone)
		PROVIDE_HIDDEN(pertask_clone_end = ABSOLUTE(.));

		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(pervm_init_start = ABSOLUTE(.));
		*(.rodata.pervm.init)
		PROVIDE_HIDDEN(pervm_init_end = ABSOLUTE(.));

		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(pervm_fini_start = ABSOLUTE(.));
		*(.rodata.pervm.fini)
		PROVIDE_HIDDEN(pervm_fini_end = ABSOLUTE(.));

		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(pervm_clone_start = ABSOLUTE(.));
		*(.rodata.pervm.clone)
		PROVIDE_HIDDEN(pervm_clone_end = ABSOLUTE(.));

		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(pervm_unmapuser_start = ABSOLUTE(.));
		*(.rodata.pervm.unmapuser)
		PROVIDE_HIDDEN(pervm_unmapuser_end = ABSOLUTE(.));

		IP_ALIGN(CACHELINE);
		*(.rodata.cold)
	}

	IP_ALIGN(__SIZEOF_POINTER__);
	SECTION(.except) {
		PROVIDE_HIDDEN(kernel_except_start = ABSOLUTE(.));
		*(.except)
		PROVIDE_HIDDEN(kernel_except_end = ABSOLUTE(.));
		PROVIDE_HIDDEN(kernel_except_size = ABSOLUTE(kernel_except_end - kernel_except_start));
	}

	IP_ALIGN(__SIZEOF_POINTER__);
	SECTION(.eh_frame) {
		PROVIDE_HIDDEN(kernel_ehframe_start = ABSOLUTE(.));
		*(.eh_frame)
		PROVIDE_HIDDEN(kernel_ehframe_end = ABSOLUTE(.));
		PROVIDE_HIDDEN(kernel_ehframe_size = ABSOLUTE(kernel_ehframe_end - kernel_ehframe_start));
	}
	IP_ALIGN(__SIZEOF_POINTER__);

#ifndef CONFIG_NO_SMP
	/* TODO: This can be added to .free */
	IP_ALIGN(CACHELINE);
	DEFINE_PEROBJECT_SECTION(.percpu,kernel_percpu,{
		*(.data.percpu.head)
		*(.data.percpu)
	})
#endif /* !CONFIG_NO_SMP */

	IP_ALIGN(CACHELINE);
	DEFINE_PEROBJECT_SECTION(.pertask,kernel_pertask,{
		*(.data.pertask.head)
		*(.data.pertask)
	})

	IP_ALIGN(CACHELINE);
	DEFINE_PEROBJECT_SECTION_EX(.permv,kernel_pervm,PAGEDIR_SIZE,{
		*(.data.pervm.head)
		*(.data.pervm)
	})

	PROVIDE_HIDDEN(kernel_rodata_end_raw = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_ro_end_raw = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_ronx_end_raw = ABSOLUTE(.));
	IP_ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(kernel_rodata_end = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_rodata_size_raw = ABSOLUTE(kernel_rodata_end_raw - kernel_rodata_start));
	PROVIDE_HIDDEN(kernel_rodata_size = ABSOLUTE(kernel_rodata_end - kernel_rodata_start));
	PROVIDE_HIDDEN(kernel_ro_end = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_ro_size_raw = ABSOLUTE(kernel_ro_end_raw - kernel_ro_start));
	PROVIDE_HIDDEN(kernel_ro_size = ABSOLUTE(kernel_ro_end - kernel_ro_start));
	PROVIDE_HIDDEN(kernel_ronx_end = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_ronx_size_raw = ABSOLUTE(kernel_ronx_end_raw - kernel_ronx_start));
	PROVIDE_HIDDEN(kernel_ronx_size = ABSOLUTE(kernel_ronx_end - kernel_ronx_start));

	IP_ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(kernel_rw_start = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_rwx_start = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_rwnf_start = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_data_start = ABSOLUTE(.));
	SECTION(.data) {
		*(.xdata.hot)
		*(.xdata)
		*(.xdata.rare)
		*(.xdata.cold)
		PROVIDE_HIDDEN(kernel_rwx_end_raw = ABSOLUTE(.));
		IP_ALIGN(PAGESIZE);

#ifndef CONFIG_NO_DEBUG_MALLOC
		PROVIDE_HIDDEN(mall_tracked_start = ABSOLUTE(.));
#endif
		*(.data.hot)

		IP_ALIGN(CACHELINE);
		*(.data)
		*(.symbols)

		IP_ALIGN(CACHELINE);
		*(.data.rare)
		*(.data.cold)

		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(_idt_start = ABSOLUTE(.));
		/* Use the linker to generate our IDT table.
		 * This way, we don't have to keep a table of pointers
		 * that we'd otherwise need to convert at runtime! */
#include "interrupt.ld"

		/* Ushare data segments. */
		IP_ALIGN(PAGESIZE);
		*(.data.ushare.sysenter)
		IP_ALIGN(PAGESIZE);
		*(.data.ushare.strerror)
		IP_ALIGN(PAGESIZE);
		*(.data.ushare.utsname)

#ifdef CONFIG_NO_SMP
		IP_ALIGN(CACHELINE);
		PROVIDE_HIDDEN(kernel_percpu_start = ABSOLUTE(.));
		PROVIDE_HIDDEN(boot_cpu_start = ABSOLUTE(.));
		*(.data.percpu.head)
		*(.data.percpu)
		PROVIDE_HIDDEN(kernel_percpu_end = ABSOLUTE(.));
		PROVIDE_HIDDEN(kernel_percpu_size = ABSOLUTE(kernel_percpu_end - kernel_percpu_start));
#endif /* !CONFIG_NO_SMP */
	}
	IP_ALIGN(__SIZEOF_POINTER__);
	DEFINE_PEROBJECT_SECTION(.debug_line,kernel_debug_line,{
		*(.debug_line)
		*(.data.debug_line.clutch)
	})
	PROVIDE_HIDDEN(kernel_data_end = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_data_size = ABSOLUTE(kernel_data_end - kernel_data_start));
	PROVIDE_HIDDEN(x86_ushare_sysenter_pageno = ABSOLUTE((x86_ushare_sysenter - KERNEL_BASE) / PAGESIZE));
	PROVIDE_HIDDEN(ushare_strerror_pageno = ABSOLUTE((ushare_strerror - KERNEL_BASE) / PAGESIZE));
	PROVIDE_HIDDEN(ushare_utsname_pageno = ABSOLUTE((ushare_utsname - KERNEL_BASE) / PAGESIZE));
	PROVIDE_HIDDEN(kernel_rwx_end = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_rwx_size_raw = ABSOLUTE(kernel_rwx_end_raw - kernel_rwx_start));
	PROVIDE_HIDDEN(kernel_rwx_size = ABSOLUTE(kernel_rwx_end - kernel_rwx_start));

	PROVIDE_HIDDEN(kernel_bss_start = ABSOLUTE(.));
	SECTION(.bss) {
		IP_ALIGN(PAGESIZE);
		*(.bss.corebase_initial)
		*(.bss.hot)
		*(COMMON)
		*(.bss)
		*(.bss.rare)
		*(.bss.cold)
		IP_ALIGN(PAGESIZE);
		PROVIDE_HIDDEN(x86_boot_stack_guard_addr = ABSOLUTE(.));
		PROVIDE_HIDDEN(x86_boot_stack_guard_page = ABSOLUTE(.) / PAGESIZE);
		. += PAGESIZE;
		IP_ALIGN(CACHELINE);
		*(.bss.boot.boot_stack)
#ifndef __x86_64__
		IP_ALIGN(CACHELINE);
		*(.bss.boot.df_stack)
#endif

		/* Boot CPU/TASK/VM data. */
#ifndef CONFIG_NO_SMP
		PROVIDE_HIDDEN(boot_cpu_start = ABSOLUTE(.));
		. += kernel_percpu_size;
#endif /* !CONFIG_NO_SMP */
		PROVIDE_HIDDEN(boot_task_start = ABSOLUTE(.));
		. += kernel_pertask_size;

		IP_ALIGN(PAGEDIR_ALIGN);
		PROVIDE(vm_kernel = ABSOLUTE(.));
		PROVIDE(pagedir_kernel = ABSOLUTE(.));
		PROVIDE(pagedir_kernel_phys = ABSOLUTE(pagedir_kernel - KERNEL_BASE));
		. += PAGEDIR_SIZE;
		PROVIDE_HIDDEN(boot_vm_start = ABSOLUTE(.));
		. += kernel_pervm_size;

#ifndef CONFIG_NO_DEBUG_MALLOC
		PROVIDE_HIDDEN(mall_tracked_end = ABSOLUTE(.));
#endif
#ifndef __x86_64__
		IP_ALIGN(PAGESIZE);
		*(.bss.pagedir.kernel.share)
#endif
#ifndef CONFIG_NO_DEBUG_MALLOC
		*(.bss.mall.untracked)
#endif
	}
	PROVIDE_HIDDEN(kernel_bss_end_raw = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_rwnf_end_raw = ABSOLUTE(.));
	IP_ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(kernel_bss_end = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_bss_size_raw = ABSOLUTE(kernel_bss_end_raw - kernel_bss_start));
	PROVIDE_HIDDEN(kernel_bss_size = ABSOLUTE(kernel_bss_end - kernel_bss_start));
	PROVIDE_HIDDEN(kernel_rwnf_end = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_rwnf_size_raw = ABSOLUTE(kernel_rw_end_raw - kernel_rw_start));
	PROVIDE_HIDDEN(kernel_rwnf_size = ABSOLUTE(kernel_rw_end - kernel_rw_start));

	/* Free data (to-be released once the kernel is up & running) */
	PROVIDE_HIDDEN(kernel_free_start = ABSOLUTE(.));
	SECTION(.free) {
		*(.text.free)
		*(.rodata.free)
		*(.rodata.str.free)
		PROVIDE_HIDDEN(kernel_coredriver_preinit_start = ABSOLUTE(.));
		*(.rodata.core_driver.preinit.free)
		PROVIDE_HIDDEN(kernel_coredriver_preinit_end = ABSOLUTE(.));
		PROVIDE_HIDDEN(kernel_coredriver_init_start = ABSOLUTE(.));
		*(.rodata.core_driver.init.free)
		PROVIDE_HIDDEN(kernel_coredriver_init_end = ABSOLUTE(.));
		*(.data.free)
		*(.bss.free)
	}
	PROVIDE_HIDDEN(kernel_free_end_raw = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_rw_end_raw = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_end_raw = ABSOLUTE(.));
	IP_ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(kernel_free_end = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_free_size_raw = ABSOLUTE(kernel_free_end_raw - kernel_free_start));
	PROVIDE_HIDDEN(kernel_free_size = ABSOLUTE(kernel_free_end - kernel_free_start));
	PROVIDE_HIDDEN(kernel_rw_end = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_rw_size_raw = ABSOLUTE(kernel_rw_end_raw - kernel_rw_start));
	PROVIDE_HIDDEN(kernel_rw_size = ABSOLUTE(kernel_rw_end - kernel_rw_start));
	PROVIDE_HIDDEN(kernel_end = ABSOLUTE(.));
	PROVIDE_HIDDEN(kernel_size_raw = ABSOLUTE(kernel_end_raw - kernel_start));
	PROVIDE_HIDDEN(kernel_size = ABSOLUTE(kernel_end - kernel_start));

	/* Boot page initialization constants used to initialize paging of the first 1Gb. */
	PROVIDE_HIDDEN(_boot_page0_size = ((kernel_start - KERNEL_BASE) - 0) / PAGESIZE); /* -RW */
	PROVIDE_HIDDEN(_boot_page1_size = kernel_ro_size / PAGESIZE);                     /* XR- */
	PROVIDE_HIDDEN(_boot_page2_size = (((X86_PDIR_E1_IDENTITY_BASE - KERNEL_BASE) & 0xffffffff) / PAGESIZE) - (_boot_page0_size + _boot_page1_size)); /* -WR */
	/* Added together, `_boot_page?_size' must equate to `1Gb / PAGESIZE' */
	

	/* Pre-calculate TSS and TLS segment initializers. */
#ifdef CONFIG_NO_SMP
	PROVIDE_HIDDEN(_x86_gdt_tss_lo   = __X86_SEG_ENCODELO(x86_cputss,X86_SEG_LIMIT_MAX,X86_SEG_TSS));
	PROVIDE_HIDDEN(_x86_gdt_tss_hi   = __X86_SEG_ENCODEHI(x86_cputss,X86_SEG_LIMIT_MAX,X86_SEG_TSS));
	PROVIDE_HIDDEN(_x86_gdt_tssdf_lo = __X86_SEG_ENCODELO(x86_cputssdf,X86_SEG_LIMIT_MAX,X86_SEG_TSS));
	PROVIDE_HIDDEN(_x86_gdt_tssdf_hi = __X86_SEG_ENCODEHI(x86_cputssdf,X86_SEG_LIMIT_MAX,X86_SEG_TSS));
#else
	PROVIDE_HIDDEN(_x86_gdt_tss_lo   = __X86_SEG_ENCODELO(boot_cpu_start + x86_cputss,X86_SEG_LIMIT_MAX,X86_SEG_TSS));
	PROVIDE_HIDDEN(_x86_gdt_tss_hi   = __X86_SEG_ENCODEHI(boot_cpu_start + x86_cputss,X86_SEG_LIMIT_MAX,X86_SEG_TSS));
	PROVIDE_HIDDEN(_x86_gdt_tssdf_lo = __X86_SEG_ENCODELO(boot_cpu_start + x86_cputssdf,X86_SEG_LIMIT_MAX,X86_SEG_TSS));
	PROVIDE_HIDDEN(_x86_gdt_tssdf_hi = __X86_SEG_ENCODEHI(boot_cpu_start + x86_cputssdf,X86_SEG_LIMIT_MAX,X86_SEG_TSS));
#endif
#ifdef CONFIG_X86_SEGMENTATION
	/* With full segmentation, we don't need to expose the TLS segment to user-space. */
	PROVIDE_HIDDEN(_x86_gdt_tls_lo   = __X86_SEG_ENCODELO(boot_task_start,X86_SEG_LIMIT_MAX,X86_SEG_DATA_PL0));
	PROVIDE_HIDDEN(_x86_gdt_tls_hi   = __X86_SEG_ENCODEHI(boot_task_start,X86_SEG_LIMIT_MAX,X86_SEG_DATA_PL0));
#else
	/* Without segmentation, we need to expose the kernel's TLS segment to user-space, too.
	 * If we didn't do this, we'd get a GFP when trying to add ring#3 permissions, and if we
	 * didn't add those permissions, the TLS segment would be set to ZERO by the CPU as soon
	 * as we'd attempt to switch to user-space.
	 * I'd rather the CPU would just ignore the segment and cause a GPF when user-space
	 * attempts to access it, but I guess you can't have that...
	 * Anyways: Since we use paging to protect kernel memory, user-space code
	 *          attempting to access kernel segments'll just get a #PF instead. */
	PROVIDE_HIDDEN(_x86_gdt_tls_lo   = __X86_SEG_ENCODELO(boot_task_start,X86_SEG_LIMIT_MAX,X86_SEG_DATA_PL3));
	PROVIDE_HIDDEN(_x86_gdt_tls_hi   = __X86_SEG_ENCODEHI(boot_task_start,X86_SEG_LIMIT_MAX,X86_SEG_DATA_PL3));
#endif


	/* Pre-calculate page numbers and sizes of kernel segments. */
	PROVIDE_HIDDEN(kernel_rx_minpage     = ABSOLUTE(kernel_rx_start / PAGESIZE));
	PROVIDE_HIDDEN(kernel_rx_maxpage     = ABSOLUTE((kernel_rx_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(kernel_rx_num_pages   = ABSOLUTE((kernel_rx_maxpage - kernel_rx_minpage)+1));
	PROVIDE_HIDDEN(kernel_ro_minpage     = ABSOLUTE(kernel_ro_start / PAGESIZE));
	PROVIDE_HIDDEN(kernel_ro_maxpage     = ABSOLUTE((kernel_ro_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(kernel_ro_num_pages   = ABSOLUTE((kernel_ro_maxpage - kernel_ro_minpage)+1));
	PROVIDE_HIDDEN(kernel_ronx_minpage   = ABSOLUTE(kernel_ronx_start / PAGESIZE));
	PROVIDE_HIDDEN(kernel_ronx_maxpage   = ABSOLUTE((kernel_ronx_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(kernel_ronx_num_pages = ABSOLUTE((kernel_ronx_maxpage - kernel_ronx_minpage)+1));
	PROVIDE_HIDDEN(kernel_rw_minpage     = ABSOLUTE(kernel_rw_start / PAGESIZE));
	PROVIDE_HIDDEN(kernel_rw_maxpage     = ABSOLUTE((kernel_rw_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(kernel_rw_num_pages   = ABSOLUTE((kernel_rw_maxpage - kernel_rw_minpage)+1));
	PROVIDE_HIDDEN(kernel_rwx_minpage    = ABSOLUTE(kernel_rwx_start / PAGESIZE));
	PROVIDE_HIDDEN(kernel_rwx_maxpage    = ABSOLUTE((kernel_rwx_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(kernel_rwx_num_pages  = ABSOLUTE((kernel_rwx_maxpage - kernel_rwx_minpage)+1));
	PROVIDE_HIDDEN(kernel_rwnf_minpage   = ABSOLUTE(kernel_rwnf_start / PAGESIZE));
	PROVIDE_HIDDEN(kernel_rwnf_maxpage   = ABSOLUTE((kernel_rwnf_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(kernel_rwnf_num_pages = ABSOLUTE((kernel_rwnf_maxpage - kernel_rwnf_minpage)+1));
	PROVIDE_HIDDEN(kernel_free_minpage   = ABSOLUTE(kernel_free_start / PAGESIZE));
	PROVIDE_HIDDEN(kernel_free_maxpage   = ABSOLUTE((kernel_free_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(kernel_free_num_pages = ABSOLUTE((kernel_free_maxpage - kernel_free_minpage)+1));

	/DISCARD/ : {
		*(.rel*)
		*(.discard)
		*(.dynamic)
		*(.symtab)
		*(.strtab)
		*(.comment)
		*(.eh_frame_hdr)
		*(.note.gnu.build-id)
	}
}

