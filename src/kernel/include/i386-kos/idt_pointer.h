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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_IDT_POINTER_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_IDT_POINTER_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>

DECL_BEGIN

#define X86_IDT_POINTER_OFFSETOF_LIMIT 0
#define X86_IDT_POINTER_OFFSETOF_IDT   2
#define X86_IDT_POINTER_OFFSETOF_GDT   2
#define X86_IDT_POINTER_OFFSETOF_LDT   2
#define X86_IDT_POINTER_SIZE          (2+__SIZEOF_POINTER__)

#ifdef __CC__
struct x86_idtentry;
struct x86_segment;

struct ATTR_ALIGNED(2) PACKED x86_idt_pointer {
    /* Interrupt/GDT descriptor pointer. */
    u16                      ip_limit;
    union PACKED {
        struct x86_idtentry *ip_idt;
        struct x86_segment  *ip_gdt;
        struct x86_segment  *ip_ldt;
    };
};
#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_IDT_POINTER_H */
