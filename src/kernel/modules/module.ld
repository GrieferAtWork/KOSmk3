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
#include <hybrid/typecore.h>
#include <hybrid/limits.h>
#include <kernel/paging.h>
#include <fs/driver.h>
#endif

#define SECTION(name) \
	name BLOCK(1) : AT(ADDR(name))
#define IP_ALIGN(x) . = ALIGN(x)

SECTIONS {

#undef NULL
	PROVIDE_HIDDEN(NULL = 0);
	IP_ALIGN(PAGESIZE);

	/* Dynamic information (dependencies, load information, etc.) */
	.dynamic : { *(.dynamic) }
	.hash : { *(.hash) }
	.gnu.hash : { *(.gnu.hash) }
	.dynsym : { *(.dynsym) }
	.dynstr : { *(.dynstr) }
	.rel.dyn : {
		*(.rel.init)
		*(.rel.text .rel.text.* .rel.gnu.linkonce.t.*)
		*(.rel.fini)
		*(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*)
		*(.rel.data.rel.ro .rel.data.rel.ro.* .rel.gnu.linkonce.d.rel.ro.*)
		*(.rel.data .rel.data.* .rel.gnu.linkonce.d.*)
		*(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*)
		*(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*)
		*(.rel.ctors)
		*(.rel.dtors)
		*(.rel.got)
		*(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*)
		*(.rel.ifunc)
	}
	.rel.plt : {
		*(.rel.plt)
		PROVIDE_HIDDEN(__rel_iplt_start = .);
		*(.rel.iplt)
		PROVIDE_HIDDEN(__rel_iplt_end = .);
	}


	/* Text segment */
	PROVIDE_HIDDEN(module_start = .);
	PROVIDE_HIDDEN(module_rx_start = .);
	PROVIDE_HIDDEN(module_ro_start = .);
	PROVIDE_HIDDEN(module_text_start = .);
	SECTION(.text) {
		*(.text.hot)
		*(.text)
		*(.text.rare)
		*(.text.cold)
		*(.text.__x86.*)
	}
	PROVIDE_HIDDEN(module_text_end_raw = .);
	PROVIDE_HIDDEN(module_rx_end_raw = .);
	IP_ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(module_text_end = .);
	PROVIDE_HIDDEN(module_text_size_raw = ABSOLUTE(module_text_end_raw - module_text_start));
	PROVIDE_HIDDEN(module_text_size = ABSOLUTE(module_text_end - module_text_start));
	PROVIDE_HIDDEN(module_rx_end = .);
	PROVIDE_HIDDEN(module_rx_size_raw = ABSOLUTE(module_rx_end_raw - module_rx_start));
	PROVIDE_HIDDEN(module_rx_size = ABSOLUTE(module_rx_end - module_rx_start));

	PROVIDE_HIDDEN(module_ronx_start = .);
	PROVIDE_HIDDEN(module_rodata_start = .);
	SECTION(.rodata) {
		*(.rodata.hot)
		*(.rodata)
		*(.rodata.str*)
		*(.rodata.rare)
		*(.rodata.cold)

		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE("__$$OS$driver_specs" = .);
		*(.rodata.driver_specs)
		SHORT(DRIVER_TAG_STOP);
		SHORT(DRIVER_TAG_FNORMAL);

		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(module_fini_start = .);
		*(.rodata.driver.postfini)
		*(.rodata.driver.fini)
		*(.rodata.driver.prefini)
		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(module_fini_end = .);
	}

	IP_ALIGN(__SIZEOF_POINTER__);
	SECTION(.except) {
		PROVIDE_HIDDEN(module_except_start = .);
		*(.except)
		PROVIDE_HIDDEN(module_except_end = .);
	}
	PROVIDE_HIDDEN(module_except_size = ABSOLUTE(module_except_end - module_except_start));

	IP_ALIGN(__SIZEOF_POINTER__);
	SECTION(.eh_frame) {
		PROVIDE_HIDDEN(module_ehframe_start = .);
		*(.eh_frame)
		PROVIDE_HIDDEN(module_ehframe_end = .);
	}
	PROVIDE_HIDDEN(module_ehframe_size = ABSOLUTE(module_ehframe_end - module_ehframe_start));
	PROVIDE_HIDDEN(module_rodata_end_raw = .);
	PROVIDE_HIDDEN(module_ronx_end_raw = .);
	PROVIDE_HIDDEN(module_ro_end_raw = .);
	IP_ALIGN(PAGESIZE);

	PROVIDE_HIDDEN(module_ro_end = .);
	PROVIDE_HIDDEN(module_ro_size_raw = ABSOLUTE(module_ro_end_raw - module_ro_start));
	PROVIDE_HIDDEN(module_ro_size = ABSOLUTE(module_ro_end - module_ro_start));
	PROVIDE_HIDDEN(module_ronx_end = .);
	PROVIDE_HIDDEN(module_ronx_size_raw = ABSOLUTE(module_ronx_end_raw - module_ronx_start));
	PROVIDE_HIDDEN(module_ronx_size = ABSOLUTE(module_ronx_end - module_ronx_start));
	PROVIDE_HIDDEN(module_rodata_end = .);
	PROVIDE_HIDDEN(module_rodata_size_raw = ABSOLUTE(module_rodata_end_raw - module_rodata_start));
	PROVIDE_HIDDEN(module_rodata_size = ABSOLUTE(module_rodata_end - module_rodata_start));

	IP_ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(module_rw_start = .);
	PROVIDE_HIDDEN(module_rwx_start = .);
	PROVIDE_HIDDEN(module_rwnf_start = .);
	PROVIDE_HIDDEN(module_data_start = .);
	SECTION(.xdata) {
		*(.xdata.hot)
		*(.xdata)
		*(.xdata.rare)
		*(.xdata.cold)
	}
	PROVIDE_HIDDEN(module_rwx_end_raw = .);
	IP_ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(module_rwx_end = .);
	PROVIDE_HIDDEN(module_rwx_size_raw = ABSOLUTE(module_rwx_end_raw - module_rwx_start));
	PROVIDE_HIDDEN(module_rwx_size = ABSOLUTE(module_rwx_end - module_rwx_start));

	PROVIDE_HIDDEN(module_rwnx_start = .);
	SECTION(.data) {
		*(.data.hot)
		*(.data)
		*(.symbols)
		*(.data.rare)
		*(.data.cold)
	}
	PROVIDE_HIDDEN(module_rwnx_end = .);
	PROVIDE_HIDDEN(module_data_end = .);
	PROVIDE_HIDDEN(module_data_size = ABSOLUTE(module_data_end - module_data_start));

	PROVIDE_HIDDEN(module_bss_start = .);
	SECTION(.bss) {
		*(.bss.hot)
		*(COMMON)
		*(.bss)
		*(.bss.rare)
		*(.bss.cold)
	}
	PROVIDE_HIDDEN(module_bss_end_raw = .);
	PROVIDE_HIDDEN(module_rwnf_end_raw = .);
	IP_ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(module_bss_end = .);
	PROVIDE_HIDDEN(module_bss_size_raw = ABSOLUTE(module_bss_end_raw - module_bss_start));
	PROVIDE_HIDDEN(module_bss_size = ABSOLUTE(module_bss_end - module_bss_start));
	PROVIDE_HIDDEN(module_rwnf_end = .);
	PROVIDE_HIDDEN(module_rwnf_size_raw = ABSOLUTE(module_rw_end_raw - module_rw_start));
	PROVIDE_HIDDEN(module_rwnf_size = ABSOLUTE(module_rw_end - module_rw_start));

	/* Free data (to-be released once the module is up & running) */
	PROVIDE_HIDDEN(module_free_start = .);
	IP_ALIGN(PAGESIZE);
	SECTION(.free) {
		*(.text.free)
		*(.rodata.free)
		*(.rodata.str.free)
		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(module_init_start = .);
		*(.rodata.driver.preinit)
		*(.rodata.driver.init)
		*(.rodata.driver.postinit)
		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(module_init_end = .);
		PROVIDE_HIDDEN(module_parm_start = .);
		*(.rodata.driver.param)
		IP_ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(module_parm_end = .);
		*(.data.free)
		*(.bss.free)
	}
	PROVIDE_HIDDEN(module_free_end_raw = .);
	PROVIDE_HIDDEN(module_rw_end_raw = .);
	PROVIDE_HIDDEN(module_end_raw = .);
	IP_ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(module_free_end = .);
	PROVIDE_HIDDEN(module_free_size_raw = ABSOLUTE(module_free_end_raw - module_free_start));
	PROVIDE_HIDDEN(module_free_size = ABSOLUTE(module_free_end - module_free_start));
	PROVIDE_HIDDEN(module_rw_end = .);
	PROVIDE_HIDDEN(module_rw_size_raw = ABSOLUTE(module_rw_end_raw - module_rw_start));
	PROVIDE_HIDDEN(module_rw_size = ABSOLUTE(module_rw_end - module_rw_start));
	PROVIDE_HIDDEN(module_end = .);
	PROVIDE_HIDDEN(module_size_raw = ABSOLUTE(module_end_raw - module_start));
	PROVIDE_HIDDEN(module_size = ABSOLUTE(module_end - module_start));


	/* Pre-calculate page numbers and sizes of module segments. */
	PROVIDE_HIDDEN(module_rx_minpage     = ABSOLUTE(module_rx_start / PAGESIZE));
	PROVIDE_HIDDEN(module_rx_maxpage     = ABSOLUTE((module_rx_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(module_rx_num_pages   = ABSOLUTE((module_rx_maxpage - module_rx_minpage)+1));
	PROVIDE_HIDDEN(module_ro_minpage     = ABSOLUTE(module_ro_start / PAGESIZE));
	PROVIDE_HIDDEN(module_ro_maxpage     = ABSOLUTE((module_ro_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(module_ro_num_pages   = ABSOLUTE((module_ro_maxpage - module_ro_minpage)+1));
	PROVIDE_HIDDEN(module_ronx_minpage   = ABSOLUTE(module_ronx_start / PAGESIZE));
	PROVIDE_HIDDEN(module_ronx_maxpage   = ABSOLUTE((module_ronx_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(module_ronx_num_pages = ABSOLUTE((module_ronx_maxpage - module_ronx_minpage)+1));
	PROVIDE_HIDDEN(module_rw_minpage     = ABSOLUTE(module_rw_start / PAGESIZE));
	PROVIDE_HIDDEN(module_rw_maxpage     = ABSOLUTE((module_rw_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(module_rw_num_pages   = ABSOLUTE((module_rw_maxpage - module_rw_minpage)+1));
	PROVIDE_HIDDEN(module_rwx_minpage    = ABSOLUTE(module_rwx_start / PAGESIZE));
	PROVIDE_HIDDEN(module_rwx_maxpage    = ABSOLUTE((module_rwx_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(module_rwx_num_pages  = ABSOLUTE((module_rwx_maxpage - module_rwx_minpage)+1));
	PROVIDE_HIDDEN(module_rwnf_minpage   = ABSOLUTE(module_rwnf_start / PAGESIZE));
	PROVIDE_HIDDEN(module_rwnf_maxpage   = ABSOLUTE((module_rwnf_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(module_rwnf_num_pages = ABSOLUTE((module_rwnf_maxpage - module_rwnf_minpage)+1));
	PROVIDE_HIDDEN(module_free_minpage   = ABSOLUTE(module_free_start / PAGESIZE));
	PROVIDE_HIDDEN(module_free_maxpage   = ABSOLUTE((module_free_end / PAGESIZE)-1));
	PROVIDE_HIDDEN(module_free_num_pages = ABSOLUTE((module_free_maxpage - module_free_minpage)+1));

	PROVIDE_HIDDEN(module_init_size  = ABSOLUTE(ABSOLUTE(module_init_end) - ABSOLUTE(module_init_start)));
	PROVIDE_HIDDEN(module_init_count = ABSOLUTE(module_init_size / __SIZEOF_POINTER__));
	PROVIDE_HIDDEN(module_parm_size  = ABSOLUTE(ABSOLUTE(module_parm_end) - ABSOLUTE(module_parm_start)));
	PROVIDE_HIDDEN(module_parm_count = ABSOLUTE(module_parm_size / __SIZEOF_POINTER__));
	PROVIDE_HIDDEN(module_fini_size  = ABSOLUTE(ABSOLUTE(module_fini_end) - ABSOLUTE(module_fini_start)));
	PROVIDE_HIDDEN(module_fini_count = ABSOLUTE(module_fini_size / __SIZEOF_POINTER__));

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

