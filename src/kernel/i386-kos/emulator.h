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
#ifndef GUARD_KERNEL_I386_KOS_EMULATOR_H
#define GUARD_KERNEL_I386_KOS_EMULATOR_H 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/interrupt.h>
#include <kos/context.h>
#include <stdbool.h>

DECL_BEGIN

#define X86_REG_EAX  0 /* Accumulator. */
#define X86_REG_ECX  1 /* Counter register. */
#define X86_REG_EDX  2 /* General purpose d-register. */
#define X86_REG_EBX  3 /* General purpose b-register. */
#define X86_REG_ESP  4 /* Stack pointer. */
#define X86_REG_EBP  5 /* Stack base pointer. */
#define X86_REG_ESI  6 /* Source pointer. */
#define X86_REG_EDI  7 /* Destination pointer. */

#define X86_MODRM_MOD_MASK   0xc0 /* 0b11000000 */
#define X86_MODRM_REG_MASK   0x38 /* 0b00111000 */
#define X86_MODRM_RM_MASK    0x07 /* 0b00000111 */
#define X86_MODRM_MOD_SHIFT  6
#define X86_MODRM_REG_SHIFT  3
#define X86_MODRM_RM_SHIFT   0
#define X86_MODRM_GETMOD(x) (((x)&X86_MODRM_MOD_MASK) >> X86_MODRM_MOD_SHIFT)
#define X86_MODRM_GETREG(x) (((x)&X86_MODRM_REG_MASK) >> X86_MODRM_REG_SHIFT)
#define X86_MODRM_GETRM(x)  (((x)&X86_MODRM_RM_MASK) >> X86_MODRM_RM_SHIFT)

struct modrm_info {
    u64 mi_offset; /* Memory address. */
    u8  mi_reg;    /* Base register (or 0xff when not set). */
    u8  mi_rm;     /* Secondary register operand, or instruction sub-class. */
#define MODRM_REGISTER 0
#define MODRM_MEMORY   1
    u8  mi_type;   /* mod R/M type (One of `MODRM_*') */
    u8  mi_index;  /* Index register (or 0xff when not set). */
    u8  mi_shift;  /* Index shift (or 0). */
};

INTDEF byte_t *KCALL
x86_decode_modrm(byte_t *__restrict text,
                 struct modrm_info *__restrict info);


/* Called by the #UD interrupt to emulate instruction not supported natively.
 * When `true' is returned, the instruction has been emulated and registers were updated. */
INTDEF bool KCALL
x86_emulate_instruction(struct cpu_anycontext *__restrict context);

DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_EMULATOR_H */
