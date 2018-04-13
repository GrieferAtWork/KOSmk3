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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_VM86_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_VM86_H 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>
#include <kos/types.h>
#include <kos/context.h>

DECL_BEGIN

#ifdef __x86_64__
#undef CONFIG_NO_VM86
#define CONFIG_NO_VM86 1
#endif

#ifndef CONFIG_NO_VM86
#define CONFIG_VM86 1
#ifdef __CC__
struct x86_irregs_vm86 {
    u32 ir_eip;
    u32 ir_cs;
    u32 ir_eflags;       /* Has the `EFLAGS_VM' flag set. */
    u32 ir_esp;
    u32 ir_ss;
    u16 ir_es,__ir_pad0;
    u16 ir_ds,__ir_pad1;
    u16 ir_fs,__ir_pad2;
    u16 ir_gs,__ir_pad3;
};

struct cpu_context_vm86 {
    struct x86_gpregs32              c_gpregs;   /* General purpose registers */
#ifdef CONFIG_X86_SEGMENTATION
    struct x86_segments32            c_segments; /* Segment registers (ignored, but must be valid) */
#endif /* CONFIG_X86_SEGMENTATION */
    union __ATTR_PACKED {
        struct x86_irregs_vm86       c_iret;     /* IRet registers */
        struct __ATTR_PACKED {
             __ULONG32_TYPE__        c_eip;      /* Instruction pointer */
             __ULONG32_TYPE__        __c_pad;    /* ... */
             __ULONG32_TYPE__        c_eflags;   /* Flags register */
             __ULONG32_TYPE__        c_esp;      /* Stack pointer */
        };
    };
};


/* Enter VM86 mode and start executing the given CPU context.
 * This function never returns, and the caller is responsible
 * to unwind the stack before executing this function.
 * -> This function doesn't unwind the caller's stack.
 * NOTE: There are however a few error conditions for which
 *       this function does actually return by throwing an
 *       exception (such as `E_BADALLOC' if it failed to
 *       allocate the initial VM context for the calling
 *       thread) */
FUNDEF ATTR_NORETURN void FCALL
x86_enter_vm86(struct cpu_context_vm86 *__restrict context);

/* Same as `x86_vm86_enter()', but used to leave VM86 mode
 * and continue execution with the given context. */
FUNDEF ATTR_NORETURN void FCALL
x86_leave_vm86(struct cpu_anycontext *__restrict context);

#endif /* __CC__ */


#endif /* !CONFIG_NO_VM86 */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_VM86_H */
