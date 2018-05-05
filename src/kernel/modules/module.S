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

.hidden module_free_minpage
.hidden module_free_num_pages
.hidden module_main
.weak   module_main
.global __$$OS$driver_specs
#undef NULL

DEFINE_DRIVER_TAG(DRIVER_TAG_MAIN,DRIVER_TAG_FNORMAL,module_main,NULL)
DEFINE_DRIVER_TAG(DRIVER_TAG_FREE,DRIVER_TAG_FOPTIONAL,module_free_minpage,module_free_num_pages)

DEFINE_PUBLIC(__stack_chk_fail)
DEFINE_PUBLIC(__stack_chk_guard)

.section .data
INTERN_ENTRY(__stack_chk_fail_local)
	jmp  __stack_chk_fail
SYMEND(__stack_chk_fail_local)





