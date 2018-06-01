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
#ifndef _BITS_MMAN_H
#define _BITS_MMAN_H 1

#include <__stdinc.h>
#include <bits-generic/mman.h>
#include <hybrid/host.h>
#include <kos/i386-kos/bits/compat.h>

#if defined(__EXPOSE_CPU_COMPAT) && defined(__CC__)
__SYSDECL_BEGIN

#define mmap_info_v1_compat mmap_info_compat

struct mmap_virt_compat {
    __X86_INTPTRCC                mv_file;
    __X86_INTPTRCC                mv_begin;
    __pos64_t                     mv_off;
    __X86_INTPTRCC                mv_len;
    __X86_INTPTRCC                mv_fill;
    __X86_INTPTRCC                mv_guard;
    __uint16_t                    mv_funds;
};
struct mmap_phys_compat {
    union {
        __PHYS __X86_INTPTRCC     mp_addr;
        __PHYS __UINT64_TYPE__    mp_addr64;
    };
};
struct mmap_ushare_compat {
    __X86_INTPTRCC                mu_name;
    __X86_INTPTRCC                mu_start;
};
struct mmap_info_v1_compat {
    __X86_INTPTRCC                mi_prot;
    __X86_INTPTRCC                mi_flags;
    __X86_INTPTRCC                mi_xflag;
    __VIRT __X86_PTRCC(void)      mi_addr;
    __X86_INTPTRCC                mi_size;
    __X86_INTPTRCC                mi_align;
    __X86_INTPTRCC                mi_gap;
    __X86_PTRCC(void)             mi_tag;
    union {
        struct mmap_virt_compat   mi_virt;
        struct mmap_phys_compat   mi_phys;
        struct mmap_ushare_compat mi_ushare;
    };
};

__SYSDECL_END
#endif /* __x86_64__ */

#endif /* !_BITS_MMAN_H */
