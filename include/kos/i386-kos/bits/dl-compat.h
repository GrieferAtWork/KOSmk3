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
#ifndef _KOS_I386_KOS_BITS_DL_COMPAT_H
#define _KOS_I386_KOS_BITS_DL_COMPAT_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include "compat.h"

#if defined(__EXPOSE_CPU_COMPAT) && defined(__CC__)
__SYSDECL_BEGIN

#ifdef __x86_64__
#define module_basic_info32        module_basic_info_compat
#define module_basic_info64        module_basic_info
#define module_state_info32        module_state_info_compat
#define module_state_info64        module_state_info
#define module_path_info32         module_path_info_compat
#define module_path_info64         module_path_info
#define dl_symbol32                dl_symbol_compat
#define dl_symbol64                dl_symbol
#define module_symbol_info32       module_symbol_info_compat
#define module_symbol_info64       module_symbol_info
#define dl_section32               dl_section_compat
#define dl_section64               dl_section
#define module_section_info32      module_section_info_compat
#define module_section_info64      module_section_info
#define module_callback_info32     module_callback_info_compat
#define module_callback_info64     module_callback_info
#define module_requirements_info32 module_requirements_info_compat
#define module_requirements_info64 module_requirements_info
#define module_tls_info32          module_tls_info_compat
#define module_tls_info64          module_tls_info
#else
#define module_basic_info32        module_basic_info
#define module_basic_info64        module_basic_info_compat
#define module_state_info32        module_state_info
#define module_state_info64        module_state_info_compat
#define module_path_info32         module_path_info
#define module_path_info64         module_path_info_compat
#define dl_symbol32                dl_symbol
#define dl_symbol64                dl_symbol_compat
#define module_symbol_info32       module_symbol_info
#define module_symbol_info64       module_symbol_info_compat
#define dl_section32               dl_section
#define dl_section64               dl_section_compat
#define module_section_info32      module_section_info
#define module_section_info64      module_section_info_compat
#define module_callback_info32     module_callback_info
#define module_callback_info64     module_callback_info_compat
#define module_requirements_info32 module_requirements_info
#define module_requirements_info64 module_requirements_info_compat
#define module_tls_info32          module_tls_info
#define module_tls_info64          module_tls_info_compat
#endif

struct module_basic_info_compat {
    __X86_INTPTRCC mi_loadaddr;
    __X86_INTPTRCC mi_segstart;
    __X86_INTPTRCC mi_segend;
};
struct module_state_info_compat {
    __X86_INTPTRCC  si_mapcnt;
    __X86_INTPTRCC  si_loadcnt;
    __uint16_t      si_appflags;
    __uint16_t      si_modflags;
#if __SIZEOF_X86_INTPTRCC__ > 4
    __uint16_t    __si_pad[(sizeof(void *)-2)/2];
#endif
    __X86_PTRCC(void) si_entry;
};
struct module_path_info_compat {
    unsigned int      pi_format;
#if __SIZEOF_X86_INTPTRCC__ > __SIZEOF_INT__
    __byte_t        __pi_pad[__SIZEOF_X86_INTPTRCC__-__SIZEOF_INT__];
#endif
    __X86_INTPTRCC    pi_loadaddr;
    __X86_PTRCC(char) pi_path;
    __X86_INTPTRCC    pi_pathlen;
};
struct dl_symbol_compat {
    __uint16_t               ds_type;
    __uint16_t __ds_pad[(sizeof(__X86_PTRCC(void))-2)/2];
    __VIRT __X86_PTRCC(void) ds_base;
    __X86_INTPTRCC           ds_size;
};
struct module_symbol_info_compat {
    __X86_PTRCC(char const)  si_name;
    struct dl_symbol_compat  si_symbol;
};
struct dl_section_compat {
    __X86_PTRCC(void) ds_base;
    __X86_INTPTRCC    ds_size;
    __pos64_t         ds_offset;
    __uint16_t        ds_type;
    __uint16_t        ds_flags;
    __uint32_t        ds_entsize;
};
struct module_section_info_compat {
    __X86_PTRCC(char const)  si_name;
    struct dl_section_compat si_section;
};
struct module_callback_info_compat {
    __X86_PTRCC(void(void)) ci_callbacks[1];
};
struct module_requirements_info_compat {
    __X86_PTRCC(void) ri_deps[1];
};
struct module_tls_info_compat {
    __X86_INTPTRCC    ti_tls_size;
    __X86_INTPTRCC    ti_tls_align;
    __X86_PTRCC(void) ti_template_base;
    __X86_INTPTRCC    ti_template_size;
};


__SYSDECL_END
#endif

#endif /* !_KOS_I386_KOS_BITS_DL_COMPAT_H */
