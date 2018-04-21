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

#include <hybrid/compiler.h>
#include <hybrid/typecore.h>
#include <fs/driver.h>

/* Common module source file (contains the runtime linkage definitions for the module) */

#define SPECS  __$$OS$driver_specs

#define EMIT_INT8(x)   .byte x;
#define EMIT_INT16(x)  .word x;
#define EMIT_INT32(x)  .int x;
#define EMIT_INT64(x)  .quad x;
#if __SIZEOF_POINTER__ == 8
#define EMIT_PTR(x)    EMIT_INT64(x)
#elif __SIZEOF_POINTER__ == 4
#define EMIT_PTR(x)    EMIT_INT32(x)
#elif __SIZEOF_POINTER__ == 2
#define EMIT_PTR(x)    EMIT_INT16(x)
#else
#define EMIT_PTR(x)    EMIT_INT8(x)
#endif
#define EMIT_RELPTR(x) .reloc .,R_386_RELATIVE, x; EMIT_PTR(0)
#define EMIT_SIZE      EMIT_PTR

.hidden module_init_start
.hidden module_init_count
.hidden module_fini_start
.hidden module_fini_count
.hidden module_parm_start
.hidden module_parm_count

.hidden module_main
.weak   module_main

.section .rodata.driver_specs
.global SPECS
SPECS:
	EMIT_PTR(DRIVER_SPECS_VERSION)     /* ds_version */
	EMIT_RELPTR(module_free_minpage)   /* ds_free */
	EMIT_RELPTR(module_free_num_pages) /* ds_free_sz */
	EMIT_RELPTR(module_init_start)     /* ds_init */
	EMIT_RELPTR(module_init_count)     /* ds_init_sz */
	EMIT_RELPTR(module_fini_start)     /* ds_fini */
	EMIT_RELPTR(module_fini_count)     /* ds_fini_sz */
	EMIT_RELPTR(module_parm_start)     /* ds_parm */
	EMIT_RELPTR(module_parm_count)     /* ds_parm_sz */
	EMIT_RELPTR(module_main)           /* ds_main */
.size SPECS, . - SPECS

