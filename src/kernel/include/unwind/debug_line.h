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
#ifndef GUARD_KERNEL_INCLUDE_UNWIND_DEBUG_LINE_H
#define GUARD_KERNEL_INCLUDE_UNWIND_DEBUG_LINE_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <fs/linker.h>
#include <kos/addr2line.h>

DECL_BEGIN

struct module_debug {
    struct dl_section     md_debug_line; /* The .debug_line section. */
    VIRT HOST byte_t     *md_dl_data;    /* [1..1][owned] Starting address where data of
                                          *               this module has been mapped at.
                                          *  -> This address is locate in kernel-space and
                                          *     is a simple file-mapping of the .debug_line
                                          *     section. Data is lazily loaded into memory
                                          *     as it is accessed, and the mapping goes away
                                          *     when module_debug data is deleted (which
                                          *     happens when the module itself goes away)
                                          * NOTE: This address is page-aligned. */
};


/* Delete module debug information. */
FUNDEF ATTR_NOTHROW void KCALL
module_debug_delete(struct module_debug *__restrict self);

/* Create a new debug information descriptor for `app'.
 * If `app' doesn't contain debug information, return `NULL' instead. */
FUNDEF /*inherit*/struct module_debug *KCALL
module_debug_alloc(struct application *__restrict app);


/* Similar to `module_debug_alloc()', but lazily check if debug
 * information had already been opened for the associated module,
 * before using `module_debug_alloc()' to create new information
 * and storing them in `app->a_module->m_debug', as well as
 * returning them. 
 * @return: * :   Pointer to debug debug information.
 * @return: NULL: No debug information present. */
FUNDEF struct module_debug *KCALL
module_debug_open(struct application *__restrict app);


/* Query address2line debug information about the given application.
 * @return: true:  Successfully found debug information.
 * @return: false: Application doesn't contain debug information. */
FUNDEF bool KCALL
module_debug_query(struct application *__restrict app,
                   image_rva_t image_relative_ip,
                   struct dl_addr2line *__restrict result);

/* Lookup an application at `ip' (an absolute address) and invoke `module_debug_query()'.
 * @return: * :            The load address of the application containing addr2line information.
 * @return: (uintptr_t)-1: No application found, or application didn't contain debug information. */
FUNDEF uintptr_t KCALL
linker_debug_query(uintptr_t ip,
                   struct dl_addr2line *__restrict result);



DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_UNWIND_DEBUG_LINE_H */
