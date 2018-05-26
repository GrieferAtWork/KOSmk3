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
#ifndef GUARD_KERNEL_I386_KOS_EMULATOR_C
#define GUARD_KERNEL_I386_KOS_EMULATOR_C 1
#define _KOS_SOURCE 1

#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/section.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kos/registers.h>
#include <kernel/user.h>
#include <kernel/interrupt.h>
#include <kos/context.h>
#include <kos/intrin.h>
#include <asm/cpu-flags.h>
#include <kernel/debug.h>
#include <sched/task.h>
#include <i386-kos/gdt.h>
#include <except.h>
#include <string.h>
#include <stdbool.h>

#include "emulator.h"

DECL_BEGIN

#ifdef __x86_64__
INTERN uintptr_t const x86_reg8_offsets[32] = {
    [X86_REG_EAX]              = offsetof(struct cpu_context,c_gpregs.gp_al),
    [X86_REG_ECX]              = offsetof(struct cpu_context,c_gpregs.gp_cl),
    [X86_REG_EDX]              = offsetof(struct cpu_context,c_gpregs.gp_dl),
    [X86_REG_EBX]              = offsetof(struct cpu_context,c_gpregs.gp_bl),
    [X86_REG_ESP]              = offsetof(struct cpu_context,c_gpregs.gp_ah),
    [X86_REG_EBP]              = offsetof(struct cpu_context,c_gpregs.gp_ch),
    [X86_REG_ESI]              = offsetof(struct cpu_context,c_gpregs.gp_dh),
    [X86_REG_EDI]              = offsetof(struct cpu_context,c_gpregs.gp_bh),
    [X86_REG_R8 ]              = offsetof(struct cpu_context,c_gpregs.gp_r8l),
    [X86_REG_R9 ]              = offsetof(struct cpu_context,c_gpregs.gp_r9l),
    [X86_REG_R10]              = offsetof(struct cpu_context,c_gpregs.gp_r10l),
    [X86_REG_R11]              = offsetof(struct cpu_context,c_gpregs.gp_r11l),
    [X86_REG_R12]              = offsetof(struct cpu_context,c_gpregs.gp_r12l),
    [X86_REG_R13]              = offsetof(struct cpu_context,c_gpregs.gp_r13l),
    [X86_REG_R14]              = offsetof(struct cpu_context,c_gpregs.gp_r14l),
    [X86_REG_R15]              = offsetof(struct cpu_context,c_gpregs.gp_r15l),
    [X86_REG_EAX|X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_al),
    [X86_REG_ECX|X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_cl),
    [X86_REG_EDX|X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_dl),
    [X86_REG_EBX|X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_bl),
    [X86_REG_ESP|X86_REG_FREX] = offsetof(struct cpu_context,c_spl),
    [X86_REG_EBP|X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_bpl),
    [X86_REG_ESI|X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_sil),
    [X86_REG_EDI|X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_dil),
    [X86_REG_R8 |X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_r8l),
    [X86_REG_R9 |X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_r9l),
    [X86_REG_R10|X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_r10l),
    [X86_REG_R11|X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_r11l),
    [X86_REG_R12|X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_r12l),
    [X86_REG_R13|X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_r13l),
    [X86_REG_R14|X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_r14l),
    [X86_REG_R15|X86_REG_FREX] = offsetof(struct cpu_context,c_gpregs.gp_r15l),
};
INTERN uintptr_t const x86_reg64_offsets[16] = {
    [X86_REG_EAX] = offsetof(struct cpu_context,c_gpregs.gp_rax),
    [X86_REG_ECX] = offsetof(struct cpu_context,c_gpregs.gp_rcx),
    [X86_REG_EDX] = offsetof(struct cpu_context,c_gpregs.gp_rdx),
    [X86_REG_EBX] = offsetof(struct cpu_context,c_gpregs.gp_rbx),
    [X86_REG_ESP] = offsetof(struct cpu_context,c_rsp),
    [X86_REG_EBP] = offsetof(struct cpu_context,c_gpregs.gp_rbp),
    [X86_REG_ESI] = offsetof(struct cpu_context,c_gpregs.gp_rsi),
    [X86_REG_EDI] = offsetof(struct cpu_context,c_gpregs.gp_rdi),
    [X86_REG_R8 ] = offsetof(struct cpu_context,c_gpregs.gp_r8),
    [X86_REG_R9 ] = offsetof(struct cpu_context,c_gpregs.gp_r9),
    [X86_REG_R10] = offsetof(struct cpu_context,c_gpregs.gp_r10),
    [X86_REG_R11] = offsetof(struct cpu_context,c_gpregs.gp_r11),
    [X86_REG_R12] = offsetof(struct cpu_context,c_gpregs.gp_r12),
    [X86_REG_R13] = offsetof(struct cpu_context,c_gpregs.gp_r13),
    [X86_REG_R14] = offsetof(struct cpu_context,c_gpregs.gp_r14),
    [X86_REG_R15] = offsetof(struct cpu_context,c_gpregs.gp_r15),
};
#else
INTERN uintptr_t const x86_reg8_offsets[8] = {
    [X86_REG_EAX] = offsetof(struct x86_gpregs,gp_al),
    [X86_REG_ECX] = offsetof(struct x86_gpregs,gp_cl),
    [X86_REG_EDX] = offsetof(struct x86_gpregs,gp_dl),
    [X86_REG_EBX] = offsetof(struct x86_gpregs,gp_bl),
    [X86_REG_ESP] = offsetof(struct x86_gpregs,gp_ah),
    [X86_REG_EBP] = offsetof(struct x86_gpregs,gp_ch),
    [X86_REG_ESI] = offsetof(struct x86_gpregs,gp_dh),
    [X86_REG_EDI] = offsetof(struct x86_gpregs,gp_bh),
};
#endif


INTERN byte_t *
(KCALL x86_decode_modrm)(byte_t *__restrict text,
                         struct modrm_info *__restrict info
#ifdef __x86_64__
                         , u16 flags
#endif
                         ) {
 u8 rmbyte = *text++;
 info->mi_reg  = X86_MODRM_GETREG(rmbyte);
 info->mi_rm   = X86_MODRM_GETRM(rmbyte);
 info->mi_type = MODRM_REGISTER;
#ifdef __x86_64__
 if (flags & F_HASREX) info->mi_reg |= 0x10;
 if (flags & F_REX_R)  info->mi_reg |= 0x8;
 if (flags & F_REX_B)  info->mi_rm |= 0x8;
#endif
 if ((rmbyte & X86_MODRM_MOD_MASK) == (0x3 << X86_MODRM_MOD_SHIFT))
      goto done; /* Register operand. */
 info->mi_type = MODRM_MEMORY;
 info->mi_shift = 1;
 info->mi_index = 0xff;
 info->mi_offset = 0;
 if (info->mi_rm == X86_REG_EBP) {
  info->mi_rm     = 0xff;
  info->mi_offset = *(u32 *)text;
  text += 4;
 } else {
  if (info->mi_rm == X86_REG_ESP) {
   u8 sibbyte = *text++;
   info->mi_shift = X86_MODRM_GETMOD(sibbyte);
   info->mi_index = X86_MODRM_GETREG(sibbyte);
#ifdef __x86_64__
   if (flags & F_REX_X) info->mi_index |= 0x8;
#endif
   if (info->mi_index == X86_REG_ESP)
       info->mi_index = 0xff;
   info->mi_rm = X86_MODRM_GETRM(sibbyte);
  }
  /* Read the trailing immediate value. */
  if ((rmbyte&X86_MODRM_MOD_MASK) == (0x1 << X86_MODRM_MOD_SHIFT))
       info->mi_offset = (u32)(s32)*(s8 *)text,text += 1;
  if ((rmbyte&X86_MODRM_MOD_MASK) == (0x2 << X86_MODRM_MOD_SHIFT))
       info->mi_offset = *(u32 *)text,text += 4;
 }
done:
 return text;
}

INTDEF void FCALL
error_rethrow_atuser(struct cpu_context *__restrict context);

#ifndef __x86_64__
PRIVATE uintptr_t FCALL
get_segment_base(struct cpu_anycontext *__restrict context, u16 segid) {
 struct PACKED {
   u16                 limit;
   struct x86_segment *base;
 } gdt;
 struct x86_segment segment;
 pflag_t was = PREEMPTION_PUSHOFF();
 __sgdt(&gdt);
 if (segid & 4) {
  u16 ldt = __sldt() & ~7;
  if unlikely(!ldt || ldt > (gdt.limit & ~7)) {
   struct exception_info *info;
   PREEMPTION_POP(was);
   /* Deal with an invalid / disabled LDT by
    * throwing an error indicating an invalid LDT. */
   memset(info->e_error.e_pointers,0,
          sizeof(info->e_error.e_pointers));
   info->e_error.e_code = E_ILLEGAL_INSTRUCTION;
   info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
   info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_UNDEFINED|
                                                  ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                  ERROR_ILLEGAL_INSTRUCTION_FVALUE);
   info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_MISC;
   info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_LDT;
   info->e_error.e_illegal_instruction.ii_value           = ldt;
   fix_user_context(context);
   memcpy(&info->e_context,&context->c_host,sizeof(struct cpu_context));
   error_rethrow_atuser((struct cpu_context *)context);
   error_throw(E_USER_RESUME);
  }
  segment   = gdt.base[ldt/8];
  gdt.base  = (struct x86_segment *)X86_SEGMENT_GTBASE(segment);
  gdt.limit = X86_SEGMENT_GTSIZE(segment);
 }
 segid &= ~7;
 if (!segid || segid > (gdt.limit & ~7))
     goto fail;
 segment = gdt.base[segid/8];
 PREEMPTION_POP(was);
 return X86_SEGMENT_GTBASE(segment);
fail:
 PREEMPTION_POP(was);
 return 0;
}
#endif

INTERN uintptr_t KCALL
x86_modrm_getmem(struct cpu_anycontext *__restrict context,
                 struct modrm_info *__restrict modrm,
                 u16 flags) {
 uintptr_t result;
 if (modrm->mi_type == MODRM_REGISTER)
     return (uintptr_t)&X86_GPREG(modrm->mi_reg);
 result = modrm->mi_offset;
 if (modrm->mi_reg != 0xff)
     result += X86_GPREG(modrm->mi_reg);
 if (modrm->mi_index != 0xff)
     result += X86_GPREG(modrm->mi_index) << modrm->mi_shift;
 switch (flags & F_SEGMASK) {
#if !defined(CONFIG_NO_X86_SEGMENTATION) && !defined(__x86_64__)
 case F_SEGDS: result += get_segment_base(context,context->c_segments.sg_ds); break;
 case F_SEGES: result += get_segment_base(context,context->c_segments.sg_es); break;
 case F_SEGFS: result += get_segment_base(context,context->c_segments.sg_fs); break;
 case F_SEGGS: result += get_segment_base(context,context->c_segments.sg_gs); break;
#elif defined(__x86_64__)
 case F_SEGFS:
  result += __rdfsbaseq();
  break;
 case F_SEGGS:
  if (context->c_cs & 3)
       /* After the swapgs executed at the start of a user-space interrupt,
        * the KERNEL_GS_BASE MSR contains the old user-space GS base value. */
       result += __rdmsr(IA32_KERNEL_GS_BASE);
  else result += (uintptr_t)THIS_TASK;
  result += __rdgsbaseq();
  break;
#else
#ifdef __ASM_TASK_SEGMENT_ISFS /* If we're using %fs, user-space is using %gs */
 case F_SEGGS: result += (uintptr_t)THIS_TASK->t_userseg; break;
 case F_SEGFS:
  if (context->c_cs & 3)
       result += (uintptr_t)&THIS_TASK->t_userseg->ts_tib;
  else result += (uintptr_t)THIS_TASK;
  break;
#else /* If we're using %gs, user-space is using %fs */
 case F_SEGFS: result += (uintptr_t)THIS_TASK->t_userseg; break;
 case F_SEGGS:
  if (context->c_cs & 3)
       result += (uintptr_t)&THIS_TASK->t_userseg->ts_tib;
  else result += (uintptr_t)THIS_TASK;
  break;
#endif
#endif
#ifndef __x86_64__
 case F_SEGCS: result += get_segment_base(context,context->c_iret.ir_cs); break;
 case F_SEGSS: result += get_segment_base(context,context->c_iret.ir_ss); break;
#endif
 default: break;
 }
 return result;
}

INTERN u8 KCALL
x86_modrm_getb(struct cpu_anycontext *__restrict context,
               struct modrm_info *__restrict modrm, u16 flags) {
 uintptr_t addr;
 if (modrm->mi_type == MODRM_REGISTER)
     return modrm_getreg8(*modrm);
 addr = x86_modrm_getmem(context,modrm,flags);
 if ((context->c_iret.ir_cs & 3) ||
     (context->c_eflags & EFLAGS_VM))
      validate_readable((void *)addr,1);
 return *(u8 *)addr;
}
INTERN u16 KCALL
x86_modrm_getw(struct cpu_anycontext *__restrict context,
               struct modrm_info *__restrict modrm, u16 flags) {
 uintptr_t addr;
 if (modrm->mi_type == MODRM_REGISTER)
     return modrm_getreg32(*modrm);
 addr = x86_modrm_getmem(context,modrm,flags);
 if ((context->c_iret.ir_cs & 3) ||
     (context->c_eflags & EFLAGS_VM))
      validate_readable((void *)addr,2);
 return *(u16 *)addr;
}
INTERN u32 KCALL
x86_modrm_getl(struct cpu_anycontext *__restrict context,
               struct modrm_info *__restrict modrm, u16 flags) {
 uintptr_t addr;
 if (modrm->mi_type == MODRM_REGISTER)
     return modrm_getreg32(*modrm);
 addr = x86_modrm_getmem(context,modrm,flags);
 if ((context->c_iret.ir_cs & 3) ||
     (context->c_eflags & EFLAGS_VM))
      validate_readable((void *)addr,4);
 return *(u32 *)addr;
}

INTERN void KCALL
x86_modrm_setb(struct cpu_anycontext *__restrict context,
               struct modrm_info *__restrict modrm,
               u16 flags, u8 value) {
 if (modrm->mi_type == MODRM_REGISTER)
     modrm_getreg8(*modrm) = value;
 else {
  uintptr_t addr;
  addr = x86_modrm_getmem(context,modrm,flags);
  if ((context->c_iret.ir_cs & 3) ||
      (context->c_eflags & EFLAGS_VM))
       validate_writable((void *)addr,1);
  *(u8 *)addr = value;
 }
}
INTERN void KCALL
x86_modrm_setw(struct cpu_anycontext *__restrict context,
               struct modrm_info *__restrict modrm,
               u16 flags, u16 value) {
 if (modrm->mi_type == MODRM_REGISTER)
     modrm_getreg32(*modrm) = value;
 else {
  uintptr_t addr;
  addr = x86_modrm_getmem(context,modrm,flags);
  if ((context->c_iret.ir_cs & 3) ||
      (context->c_eflags & EFLAGS_VM))
       validate_writable((void *)addr,2);
  *(u16 *)addr = value;
 }
}
INTERN void KCALL
x86_modrm_setl(struct cpu_anycontext *__restrict context,
               struct modrm_info *__restrict modrm,
               u16 flags, u32 value) {
 if (modrm->mi_type == MODRM_REGISTER)
     modrm_getreg32(*modrm) = value;
 else {
  uintptr_t addr;
  addr = x86_modrm_getmem(context,modrm,flags);
  if ((context->c_iret.ir_cs & 3) ||
      (context->c_eflags & EFLAGS_VM))
       validate_writable((void *)addr,4);
  *(u32 *)addr = value;
 }
}


#ifndef __x86_64__
INTERN void FCALL
fix_user_context(struct x86_anycontext *__restrict context) {
 /* Copy the USER SP into the HOST SP pointer. */
#ifdef CONFIG_VM86
 if (context->c_eflags & EFLAGS_VM) {
#ifndef CONFIG_NO_X86_SEGMENTATION
  context->c_segments.sg_gs = ((struct x86_irregs_vm86 *)&context->c_iret)->ir_gs;
  context->c_segments.sg_fs = ((struct x86_irregs_vm86 *)&context->c_iret)->ir_fs;
  context->c_segments.sg_es = ((struct x86_irregs_vm86 *)&context->c_iret)->ir_es;
  context->c_segments.sg_ds = ((struct x86_irregs_vm86 *)&context->c_iret)->ir_ds;
#endif /* !CONFIG_NO_X86_SEGMENTATION */
  context->c_host.c_esp = ((struct x86_irregs_vm86 *)&context->c_iret)->ir_esp;
 } else
#endif
 if (X86_ANYCONTEXT_ISUSER(*context)) {
  context->c_host.c_esp = context->c_user.c_esp;
 }
}
#endif


INTERN u32 (KCALL x86_readopcode)(
#ifdef __x86_64__
                                  struct x86_anycontext *__restrict context,
#endif
                                  byte_t **__restrict ptext,
                                  u16 *__restrict pflags) {
 u32 result;
 byte_t *text = *ptext;
 *pflags = 0;
 result = 0;
#ifdef __x86_64__
 if (context->c_iret.ir_cs == X86_SEG_USER_CS32)
     *pflags |= F_IS_X32;
#endif
next_byte:
 result = 0;
extend_instruction:
 result |= *text++;
 switch (result) {

  /* Prefix bytes */
 case 0x66: *pflags |= F_OP16; goto next_byte;
 case 0x67: *pflags |= F_AD16; goto next_byte;
 case 0xf0: *pflags |= F_LOCK; goto next_byte;
 case 0xf2: *pflags |= F_REPNE; goto next_byte;
 case 0xf3: *pflags |= F_REP; goto next_byte;
#ifdef __x86_64__
 case 0x40 ... 0x4f:
  if (*pflags & F_IS_X32) goto illegal_instruction;
  *pflags |= F_HASREX | ((result & 0xf) << F_REXSHFT);
  goto next_byte;
 case 0x26:
  if (!(*pflags & F_IS_X32)) goto illegal_instruction;
  *pflags = (*pflags & ~F_SEGMASK) | F_SEGES;
  goto next_byte;
 case 0x2e:
  if (!(*pflags & F_IS_X32)) goto illegal_instruction;
  *pflags = (*pflags & ~F_SEGMASK) | F_SEGCS;
  goto next_byte;
 case 0x36:
  if (!(*pflags & F_IS_X32)) goto illegal_instruction;
  *pflags = (*pflags & ~F_SEGMASK) | F_SEGSS;
  goto next_byte;
 case 0x3e:
  if (!(*pflags & F_IS_X32)) goto illegal_instruction;
  *pflags = (*pflags & ~F_SEGMASK) | F_SEGDS;
  goto next_byte;
#else
 case 0x26: *pflags = (*pflags & ~F_SEGMASK) | F_SEGES; goto next_byte;
 case 0x2e: *pflags = (*pflags & ~F_SEGMASK) | F_SEGCS; goto next_byte;
 case 0x36: *pflags = (*pflags & ~F_SEGMASK) | F_SEGSS; goto next_byte;
 case 0x3e: *pflags = (*pflags & ~F_SEGMASK) | F_SEGDS; goto next_byte;
#endif
 case 0x64: *pflags = (*pflags & ~F_SEGMASK) | F_SEGFS; goto next_byte;
 case 0x65: *pflags = (*pflags & ~F_SEGMASK) | F_SEGGS; goto next_byte;
 case 0x0f: result <<= 8; goto extend_instruction;

 default: break;
 }
 *ptext = text;
 return result;
#ifdef __x86_64__
illegal_instruction:
 {
  struct exception_info *info;
  info                 = error_info();
  info->e_error.e_code = E_ILLEGAL_INSTRUCTION;
  info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
  memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
  info->e_error.e_illegal_instruction.ii_type = ERROR_ILLEGAL_INSTRUCTION_UNDEFINED;
  fix_user_context(context);
  memcpy(&info->e_context,context,sizeof(struct cpu_context));
  /* Throw the error. */
  error_rethrow_atuser((struct cpu_context *)context);
  return X86_READOPCODE_FAILED;
 }
#endif
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_EMULATOR_C */
