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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_VM86_SOFT_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_VM86_SOFT_H 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>
#include <kos/types.h>
#include <kos/context.h>
#include <kernel/user.h>

DECL_BEGIN

#ifndef __x86_64__
#undef CONFIG_NO_VM86_SOFT
#define CONFIG_NO_VM86_SOFT 1
#endif

#ifndef CONFIG_NO_VM86_SOFT
/* A software emulator for vm86 mode, meant to be used
 * to access BIOS functions when running in long mode.
 * This is literally just an emulator. - There are no
 * real hardware requirements, and in return it doesn't
 * actually require any new special cases in other
 * components of the kernel.
 * However, because it is software-based, it might have
 * some quirks that the real hardware wouldn't have, and
 * any non-standard instruction obviously aren't
 * recognized, either.
 * WARNING:
 *    Because this emulator is designed for 32/64-bit mode,
 *    it makes the assumption that no virtual memory location
 *    addressable from realmode maps to a component of the
 *    kernel, meaning that no 21-bit (which is the greatest
 *    bit addressable using segmentation) address ever violates
 *    the expectation of realmode being mapped. */


struct vm86_regs {
#define __VM86_GPREG(name) \
    union PACKED { \
        u32    e##name##x; \
        u16    name##x; \
        struct PACKED { \
            u8 name##l; \
            u8 name##h; \
        }; \
    }
#define __VM86_GPREG_PTR(name) \
    union PACKED { \
        u32    e##name; \
        u16    name; \
    }
    __VM86_GPREG(a);
    __VM86_GPREG(c);
    __VM86_GPREG(d);
    __VM86_GPREG(b);
    __VM86_GPREG_PTR(sp);
    __VM86_GPREG_PTR(bp);
    __VM86_GPREG_PTR(si);
    __VM86_GPREG_PTR(di);
    u16  es;
    u16  cs;
    u16  ss;
    u16  ds;
    u16  fs;
    u16  gs;
    u16  ip;
    u16  flags;
};
#undef __VM86_GPREG_PTR
#undef __VM86_GPREG

struct vm86_state {
    struct vm86_regs vs_regs;  /* VM86 register state. */
#define VM86_FNORMAL 0x0000    /* Normal operations flags. */
    unsigned int     vs_flags; /* VM86 operation flags (Set of `VM86_F*'). */
};


/* Execute a single instruction, update the register state accordingly, and return.
 * NOTE: Exceptions other than #PFs occurring during evaluation may be
 *       processed by emulating the VM86 IDT located at address ZERO(0).
 *       This includes DIVIDE_BY_ZERO(#DE), OVERFLOW(#OF) and INDEX_ERROR(#BR).
 *       In such a case, this function returns once again before the first
 *       instruction of the associated interrupt is executed.
 * @throw: E_ILLEGAL_INSTRUCTION: The executed instruction was undefined.
 */
FUNDEF void KCALL vm86_soft_exec(struct vm86_state *__restrict st);





struct vm86_modrm {
#define MODRM_REGISTER 0
#define MODRM_MEMORY   1
    u8  mr_type;   /* mod R/M type (One of `MODRM_*') */
    u8  mr_reg;    /* Secondary register operand, or instruction sub-class. */
    u8  mr_rm;     /* Base register (or 0xff when not set). */
    u8  mr_index;  /* Index register (or 0xff when not set). */
    u16 mr_offset; /* Memory address. */
};

#define X86_MODRM_MOD_MASK   0xc0 /* 0b11000000 */
#define X86_MODRM_REG_MASK   0x38 /* 0b00111000 */
#define X86_MODRM_RM_MASK    0x07 /* 0b00000111 */
#define X86_MODRM_MOD_SHIFT  6
#define X86_MODRM_REG_SHIFT  3
#define X86_MODRM_RM_SHIFT   0
#define X86_MODRM_GETMOD(x) (((x)&X86_MODRM_MOD_MASK) >> X86_MODRM_MOD_SHIFT)
#define X86_MODRM_GETREG(x) (((x)&X86_MODRM_REG_MASK) >> X86_MODRM_REG_SHIFT)
#define X86_MODRM_GETRM(x)  (((x)&X86_MODRM_RM_MASK) >> X86_MODRM_RM_SHIFT)

#define X86_REG_EAX     0 /* Accumulator. */
#define X86_REG_ECX     1 /* Counter register. */
#define X86_REG_EDX     2 /* General purpose d-register. */
#define X86_REG_EBX     3 /* General purpose b-register. */
#define X86_REG_ESP     4 /* Stack pointer. */
#define X86_REG_EBP     5 /* Stack base pointer. */
#define X86_REG_ESI     6 /* Source pointer. */
#define X86_REG_EDI     7 /* Destination pointer. */

#endif /* !CONFIG_NO_VM86_SOFT */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_VM86_SOFT_H */
