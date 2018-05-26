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

#define X86_REG_EAX     0 /* Accumulator. */
#define X86_REG_ECX     1 /* Counter register. */
#define X86_REG_EDX     2 /* General purpose d-register. */
#define X86_REG_EBX     3 /* General purpose b-register. */
#define X86_REG_ESP     4 /* Stack pointer. */
#define X86_REG_EBP     5 /* Stack base pointer. */
#define X86_REG_ESI     6 /* Source pointer. */
#define X86_REG_EDI     7 /* Destination pointer. */
#ifdef __x86_64__
#define X86_REG_R8      8 /* R8 */
#define X86_REG_R9      9 /* R9 */
#define X86_REG_R10    10 /* R10 */
#define X86_REG_R11    11 /* R11 */
#define X86_REG_R12    12 /* R12 */
#define X86_REG_R13    13 /* R13 */
#define X86_REG_R14    14 /* R14 */
#define X86_REG_R15    15 /* R15 */
#define X86_REG_FREX 0x10 /* FLAG: A REX prefix is being used (only affects `x86_reg8_offsets'). */
#endif


#define X86_MODRM_MOD_MASK   0xc0 /* 0b11000000 */
#define X86_MODRM_REG_MASK   0x38 /* 0b00111000 */
#define X86_MODRM_RM_MASK    0x07 /* 0b00000111 */
#define X86_MODRM_MOD_SHIFT  6
#define X86_MODRM_REG_SHIFT  3
#define X86_MODRM_RM_SHIFT   0
#define X86_MODRM_GETMOD(x) (((x)&X86_MODRM_MOD_MASK) >> X86_MODRM_MOD_SHIFT)
#define X86_MODRM_GETREG(x) (((x)&X86_MODRM_REG_MASK) >> X86_MODRM_REG_SHIFT)
#define X86_MODRM_GETRM(x)  (((x)&X86_MODRM_RM_MASK) >> X86_MODRM_RM_SHIFT)



#define CONTEXT_IP(x)    ((x).c_pip)
#define CONTEXT_FLAGS(x) ((x).c_pflags)
#define CONTEXT_AREG(x)  ((x).c_gpregs.gp_pax)
#define CONTEXT_CREG(x)  ((x).c_gpregs.gp_pcx)
#define CONTEXT_DREG(x)  ((x).c_gpregs.gp_pdx)
#define CONTEXT_BREG(x)  ((x).c_gpregs.gp_pbx)
#define CONTEXT_DIREG(x) ((x).c_gpregs.gp_pdi)
#define CONTEXT_SIREG(x) ((x).c_gpregs.gp_psi)


#ifdef __x86_64__
INTDEF uintptr_t const x86_reg8_offsets[32];
INTDEF uintptr_t const x86_reg64_offsets[16];
#define X86_GPREG64(no)    (*(u64 *)((byte_t *)context+x86_reg64_offsets[(no) & 0xf]))
#define X86_GPREG32(no)    (*(u32 *)((byte_t *)context+x86_reg64_offsets[(no) & 0xf]))
#define X86_GPREG16(no)    (*(u16 *)((byte_t *)context+x86_reg64_offsets[(no) & 0xf]))
#define X86_GPREG8(no)      (*(u8 *)((byte_t *)context+x86_reg8_offsets[no]))
#define X86_GPREG(no)             X86_GPREG64(no)
#else
INTDEF uintptr_t const x86_reg8_offsets[8];
#define X86_GPREG32(no)          ((u32 *)&context->c_gpregs)[7-(no)]
#define X86_GPREG16(no)    (*(u16 *)&(((u32 *)&context->c_gpregs)[7-(no)]))
#define X86_GPREG8(no)          (*((u8 *)&context->c_gpregs+x86_reg8_offsets[no]))
#define X86_GPREG(no)             X86_GPREG32(no)
#endif

#ifdef X86_GPREG64
#define modrm_getreg64(modrm)   X86_GPREG64((modrm).mi_reg)
#endif
#define modrm_getreg32(modrm)   X86_GPREG32((modrm).mi_reg)
#define modrm_getreg16(modrm)   X86_GPREG16((modrm).mi_reg)
#define modrm_getreg8(modrm)     X86_GPREG8((modrm).mi_reg)
#define modrm_getreg(modrm)       X86_GPREG((modrm).mi_reg)


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

/* @param: flags: Set of `F_*' (see below) */
INTDEF byte_t *
(KCALL x86_decode_modrm)(byte_t *__restrict text,
                         struct modrm_info *__restrict info
#ifdef __x86_64__
                         , u16 flags
#endif
                         );
#ifndef __x86_64__
#define x86_decode_modrm(text,info,flags) x86_decode_modrm(text,info)
#endif

#ifdef __x86_64__
#define fix_user_context(context) (void)0
#else
INTDEF void FCALL fix_user_context(struct x86_anycontext *__restrict context);
#endif


/* @param: flags: Set of `F_*' (see below) */
INTDEF uintptr_t KCALL
x86_modrm_getmem(struct cpu_anycontext *__restrict context,
                 struct modrm_info *__restrict modrm,
                 u16 flags);

INTDEF u8  KCALL x86_modrm_getb(struct cpu_anycontext *__restrict context, struct modrm_info *__restrict modrm, u16 flags);
INTDEF u16 KCALL x86_modrm_getw(struct cpu_anycontext *__restrict context, struct modrm_info *__restrict modrm, u16 flags);
INTDEF u32 KCALL x86_modrm_getl(struct cpu_anycontext *__restrict context, struct modrm_info *__restrict modrm, u16 flags);
INTDEF void KCALL x86_modrm_setb(struct cpu_anycontext *__restrict context, struct modrm_info *__restrict modrm, u16 flags, u8  value);
INTDEF void KCALL x86_modrm_setw(struct cpu_anycontext *__restrict context, struct modrm_info *__restrict modrm, u16 flags, u16 value);
INTDEF void KCALL x86_modrm_setl(struct cpu_anycontext *__restrict context, struct modrm_info *__restrict modrm, u16 flags, u32 value);


/* Read (and return) an X86 opcode from `*ptext', updating that pointer
 * to refer to the the opcodes operands, or the next instruction, as well
 * as filling in `*pflags' to contain a set of `F_*'
 * In the event that the opcode reading process failed, `X86_READOPCODE_FAILED'
 * is returned and `context' has been updated to the return location of the
 * interrupted code segment that should be restored.
 * If no such error condition exists, `X86_READOPCODE_FAILED' isn't defined. */
#ifdef __x86_64__
INTDEF u32 KCALL
x86_readopcode(struct x86_anycontext *__restrict context,
               byte_t **__restrict ptext,
               u16 *__restrict pflags);
#define X86_READOPCODE_FAILED 0x66
#else
INTDEF u32 (KCALL x86_readopcode)(byte_t **__restrict ptext, u16 *__restrict pflags);
#define x86_readopcode(context,ptext,pflags) x86_readopcode(ptext,pflags)
#endif


#define F_OP16    0x0001 /* The 0x66 prefix is being used. */
#define F_AD16    0x0002 /* The 0x67 prefix is being used. */
#define F_LOCK    0x0004 /* The `lock' prefix is being used. */
#define F_REPNE   0x0010 /* The `repne' prefix is being used. */
#define F_REP     0x0020 /* The `rep' prefix is being used. */
#define F_SEGMASK 0xf000 /* Mask for segment overrides. */
#ifdef __x86_64__
#define F_IS_X32  0x0008 /* The hosted process is running in compatibility mode (32-bit). */
#define F_HASREX  0x0040 /* A REX prefix is being used. */
#define F_REXSHFT      8 /* Shift for the REX prefix byte. */
#define F_REXMASK 0x0f00 /* Mask of the REX prefix byte. */
#define F_REX_W   0x0800 /* The REX.W flag (Indicates 64-bit operands). */
#define F_REX_R   0x0400 /* The REX.R flag (1-bit extension to MODRM.reg). */
#define F_REX_X   0x0200 /* The REX.X flag (1-bit extension to SIB.index). */
#define F_REX_B   0x0100 /* The REX.B flag (1-bit extension to MODRM.rm). */
#define F_SEGDS   0x0000 /* DS override (compatibility mode ONLY). */
#define F_SEGES   0x1000 /* ES override (compatibility mode ONLY). */
#define F_SEGCS   0x2000 /* CS override (compatibility mode ONLY). */
#define F_SEGSS   0x3000 /* SS override (compatibility mode ONLY). */
#else
#define F_SEGDS   0x0000 /* DS override. */
#define F_SEGES   0x1000 /* ES override. */
#define F_SEGCS   0x2000 /* CS override. */
#define F_SEGSS   0x3000 /* SS override. */
#endif
#define F_SEGFS   0x4000 /* FS override. */
#define F_SEGGS   0x5000 /* GS override. */

DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_EMULATOR_H */
