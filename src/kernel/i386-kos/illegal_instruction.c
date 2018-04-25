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
#ifndef GUARD_KERNEL_I386_KOS_ILLEGAL_INSTRUCTION_C
#define GUARD_KERNEL_I386_KOS_ILLEGAL_INSTRUCTION_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/context.h>
#include <kos/registers.h>
#include <kernel/interrupt.h>
#include <kernel/debug.h>
#include <i386-kos/vm86.h>
#include <asm/cpu-flags.h>
#include <except.h>
#include <string.h>
#include <stdbool.h>

#include "emulator.h"

DECL_BEGIN

#ifdef __x86_64__
#define fix_user_context(context) (void)0
#else
PRIVATE void FCALL
fix_user_context(struct x86_anycontext *__restrict context) {
 /* Copy the USER SP into the HOST SP pointer. */
 if (X86_ANYCONTEXT32_ISUSER(*context))
     context->c_host.c_esp = context->c_user.c_esp;
}
#endif

INTDEF void FCALL
error_rethrow_atuser(struct cpu_context *__restrict context);


#ifdef CONFIG_VM86
INTDEF bool FCALL
vm86_gpf(struct cpu_context_vm86 *__restrict context,
         register_t error_code);
#endif


INTERN NOIRQ void FCALL
x86_handle_illegal_instruction(struct x86_anycontext *__restrict context) {
 struct exception_info *info;
 /* Re-enable interrupts if they were enabled before. */
 if (context->c_eflags&EFLAGS_IF)
     x86_interrupt_enable();
 /* Emulate some instructions that may not be supported natively. */
 if (x86_emulate_instruction(context))
     return;
 /* TODO: Besides emulating instruction, we should also deal with invalid operands:
  *       e.g.: `lgdt' or `lidt' being used with a register modrm
  */

 /* Construct and emit a illegal-instruction exception. */
 info                 = error_info();
 info->e_error.e_code = E_ILLEGAL_INSTRUCTION;
 info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
 memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_illegal_instruction.ii_type;
 /* Copy the CPU context at the time of the exception. */
 fix_user_context(context);
 memcpy(&info->e_context,&context->c_host,
         sizeof(struct cpu_context));
 /* Throw the error. */
 error_rethrow_atuser((struct cpu_context *)context);
}


INTERN void FCALL
x86_handle_gpf(struct cpu_anycontext *__restrict context,
               register_t errcode) {
 struct cpu_anycontext *EXCEPT_VAR xcontext = context;
 register_t EXCEPT_VAR xerrcode = errcode;
 struct exception_info *EXCEPT_VAR info;
 u16 EXCEPT_VAR flags;
 u16 EXCEPT_VAR effective_segment_value = 0;
 info = error_info();

#ifdef CONFIG_VM86
 /* Deal with GPF instruction for emulation in vm86 mode. */
 if (context->c_eflags & EFLAGS_VM) {
  if (vm86_gpf((struct cpu_context_vm86 *)context,errcode))
      return;
  /* Clear exception pointers. */
  memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
  info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
  goto e_privileged_instruction;
 }
#endif

 /* Clear exception pointers. */
 memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_code = E_ILLEGAL_INSTRUCTION;
 info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
 info->e_error.e_illegal_instruction.ii_errcode = errcode;

 /* Analyze the source assembly where the GPF happened. */
 flags = 0;
 TRY {
  byte_t *text; u32 opcode;
  text = (byte_t *)context->c_eip;
next_byte:
  opcode = 0;
extend_instruction:
  opcode |= *text++;
#define GPREG(no)    ((uintptr_t *)&context->c_gpregs)[7-(no)]
#define GPREG8(no)   ((u8 *)&context->c_gpregs+32)[7-(no)]
  switch (opcode) {

   /* Prefix bytes */
  case 0x66: flags |= F_OP16; goto next_byte;
  case 0x67: flags |= F_AD16; goto next_byte;
  case 0xf0: flags |= F_LOCK; goto next_byte;
  case 0xf2: flags |= F_REPNE; goto next_byte;
  case 0xf3: flags |= F_REP; goto next_byte;
  case 0x26: flags = (flags & ~F_SEGMASK) | F_SEGES; goto next_byte;
  case 0x2e: flags = (flags & ~F_SEGMASK) | F_SEGCS; goto next_byte;
  case 0x36: flags = (flags & ~F_SEGMASK) | F_SEGSS; goto next_byte;
  case 0x3e: flags = (flags & ~F_SEGMASK) | F_SEGDS; goto next_byte;
  case 0x64: flags = (flags & ~F_SEGMASK) | F_SEGFS; goto next_byte;
  case 0x65: flags = (flags & ~F_SEGMASK) | F_SEGGS; goto next_byte;
  case 0x0f: opcode <<= 8; goto extend_instruction;
  default: break;
  }

  switch (flags & F_SEGMASK) {
#ifndef CONFIG_NO_X86_SEGMENTATION
  case F_SEGDS: effective_segment_value = context->c_segments.sg_ds; break;
  case F_SEGES: effective_segment_value = context->c_segments.sg_es; break;
  case F_SEGFS: effective_segment_value = context->c_segments.sg_fs; break;
  case F_SEGGS: effective_segment_value = context->c_segments.sg_gs; break;
#endif
  case F_SEGCS: effective_segment_value = context->c_iret.ir_cs; break;
  case F_SEGSS: effective_segment_value = context->c_iret.ir_ss; break;
  default: effective_segment_value = 0; break;
  }

  switch (opcode) {

  {
   struct modrm_info modrm;
  case 0x0f22:
   /* MOV CRx,r32 */
   text = x86_decode_modrm(text,&modrm);
   info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                  ERROR_ILLEGAL_INSTRUCTION_FVALUE);
   info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_CONTROL;
   info->e_error.e_illegal_instruction.ii_register_number = modrm.mi_rm;
   info->e_error.e_illegal_instruction.ii_value           = (u32)GPREG(modrm.mi_reg);
   if (modrm.mi_rm == 1 || modrm.mi_rm > 4)
    /* Error was caused because CR* doesn't exist */
    info->e_error.e_illegal_instruction.ii_type |= ERROR_ILLEGAL_INSTRUCTION_UNDEFINED;
   else if (context->c_iret.ir_cs & 3)
    /* Error was caused because CR* are privileged */
    info->e_error.e_illegal_instruction.ii_type |= ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED;
   else {
    /* Error was caused because value written must be invalid. */
    info->e_error.e_illegal_instruction.ii_type |= ERROR_ILLEGAL_INSTRUCTION_FOPERAND;
   }
  } break;

  {
   struct modrm_info modrm;
  case 0x0f20:
   /* MOV r32,CRx */
   text = x86_decode_modrm(text,&modrm);
   info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_CONTROL;
   info->e_error.e_illegal_instruction.ii_register_number = modrm.mi_rm;
   if (modrm.mi_rm == 1 || modrm.mi_rm > 4) {
    /* Undefined control register. */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                   ERROR_ILLEGAL_INSTRUCTION_UNDEFINED);
   } else {
    /* Userspace tried to read from an control register. */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                   ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED);
   }
  } break;


  {
   struct modrm_info modrm;
  case 0x0f21:
  case 0x0f23:
   /* MOV r32, DR0-DR7 */
   /* MOV DR0-DR7, r32 */
   text = x86_decode_modrm(text,&modrm);
   /* Userspace tried to access an control register. */
   info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                  ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED);
   info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_DEBUG;
   info->e_error.e_illegal_instruction.ii_register_number = modrm.mi_rm;
   if (opcode == 0x0f23) {
    /* Save the value user-space tried to write. */
    info->e_error.e_illegal_instruction.ii_type |= ERROR_ILLEGAL_INSTRUCTION_FVALUE;
    info->e_error.e_illegal_instruction.ii_value = (u32)GPREG(modrm.mi_reg);
   }
  } break;

  {
   struct modrm_info modrm;
  case 0x0f00:
   text = x86_decode_modrm(text,&modrm);
   if (effective_segment_value == 0) goto null_segment_error;
   info->e_error.e_illegal_instruction.ii_register_type = X86_REGISTER_MISC;
   if (modrm.mi_rm == 3) {
    /* LTR r/m16 */
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_TR;
   } else if (modrm.mi_rm == 2) {
    /* LLDT r/m16 */
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_LDT;
   } else if (modrm.mi_rm == 0) {
    /* SLDT r/m16 */
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_LDT;
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
    break;
   } else {
    goto generic_failure;
   }
   if (errcode != 0 && !X86_ANYCONTEXT32_ISUSER(*context)) {
    /* Invalid operand for this opcode. */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_UNDEFINED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                   ERROR_ILLEGAL_INSTRUCTION_FOPERAND|
                                                   ERROR_ILLEGAL_INSTRUCTION_FVALUE);
   } else {
    /* User-space use of a privileged instruction. */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                   ERROR_ILLEGAL_INSTRUCTION_FVALUE);
   }
   info->e_error.e_illegal_instruction.ii_value = x86_modrm_getw(context,&modrm,flags);
  } break;

  {
   struct modrm_info modrm;
  case 0x0f01:
   text = x86_decode_modrm(text,&modrm);
   if (effective_segment_value == 0) goto null_segment_error;
   info->e_error.e_illegal_instruction.ii_register_type = X86_REGISTER_MISC;
   if (modrm.mi_rm == 2) {
    /* LGDT m16&32 */
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_TR;
   } else if (modrm.mi_rm == 3) {
    /* LIDT m16&32 */
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_LDT;
   } else if (modrm.mi_rm == 0) {
    /* SGDT r/m16 */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_GDT;
    break;
   } else if (modrm.mi_rm == 1) {
    /* SIDT r/m16 */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_IDT;
    break;
   } else if (modrm.mi_rm == 4) {
    /* SMSW r/m16 */
    /* SMSW r/m32 */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
    info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_CONTROL;
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_CONTROL_CR0;
    break;
   } else if (modrm.mi_rm == 6) {
    /* LMSW r/m16 */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                   ERROR_ILLEGAL_INSTRUCTION_FVALUE);
    info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_CONTROL;
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_CONTROL_CR0;
    info->e_error.e_illegal_instruction.ii_value = x86_modrm_getw(context,&modrm,flags);
    break;
   } else {
    goto generic_failure;
   }
   /* User-space use of a privileged instruction. */
   info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                  ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                  ERROR_ILLEGAL_INSTRUCTION_FVALUE);
   info->e_error.e_illegal_instruction.ii_value = x86_modrm_getmem(context,&modrm,flags);
  } break;

  {
   struct modrm_info modrm;
  case 0x8c:
   /* MOV r/m16,Sreg** */
   text = x86_decode_modrm(text,&modrm);
   if (modrm.mi_reg > 5) {
    /* Non-existent segment register */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_UNDEFINED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                   ERROR_ILLEGAL_INSTRUCTION_FOPERAND);
    info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_SEGMENT;
    info->e_error.e_illegal_instruction.ii_register_number = modrm.mi_reg;
    break;
   }
   goto generic_failure;
  } break;

  {
   struct modrm_info modrm;
  case 0x8e:
   /* MOV Sreg**,r/m16 */
   text = x86_decode_modrm(text,&modrm);
   info->e_error.e_illegal_instruction.ii_value = x86_modrm_getw(context,&modrm,flags);
   info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_SEGMENT;
   info->e_error.e_illegal_instruction.ii_register_number = modrm.mi_reg;
   /* Invalid segment index. */
   info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_UNDEFINED|
                                                  ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                  ERROR_ILLEGAL_INSTRUCTION_FVALUE);
   if (modrm.mi_reg > 5) /* Non-existent segment register */
    info->e_error.e_illegal_instruction.ii_type |= ERROR_ILLEGAL_INSTRUCTION_FOPERAND;
   else {
#ifdef E_INVALID_SEGMENT
    /* Transform this into an invalid-segment exception. */
    info->e_error.e_code = E_INVALID_SEGMENT;
#endif /* E_INVALID_SEGMENT */
   }
  } break;

  case 0x0f31: /* rdtsc */
  case 0x0f32: /* rdmsr */
  case 0x0f33: /* rdpmc */
   if (X86_ANYCONTEXT32_ISUSER(*context)) {
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
   } else {
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_UNDEFINED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
   }
   info->e_error.e_illegal_instruction.ii_register_type = X86_REGISTER_MSR;
   if (opcode == 0x0f31) {
    if (X86_ANYCONTEXT32_ISUSER(*context)) {
     /* rdtsc can be enabled for user-space, but aparently it isn't. */
     info->e_error.e_illegal_instruction.ii_type &= ~ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED;
     info->e_error.e_illegal_instruction.ii_type |= ERROR_ILLEGAL_INSTRUCTION_RESTRICTED;
    }
    info->e_error.e_illegal_instruction.ii_register_number = IA32_TIME_STAMP_COUNTER;
   } else {
    info->e_error.e_illegal_instruction.ii_register_number = context->c_gpregs.gp_ecx;
   }
   break;

  case 0x0f30:
   /* wrmsr */
   if (X86_ANYCONTEXT32_ISUSER(*context)) {
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                   ERROR_ILLEGAL_INSTRUCTION_FVALUE);
   } else {
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_UNDEFINED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                   ERROR_ILLEGAL_INSTRUCTION_FVALUE);
   }
   info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_MSR;
   info->e_error.e_illegal_instruction.ii_register_number = context->c_gpregs.gp_ecx;
   info->e_error.e_illegal_instruction.ii_value  = (u64)context->c_gpregs.gp_edx << 32;
   info->e_error.e_illegal_instruction.ii_value |= (u64)context->c_gpregs.gp_eax;
   break;

  default:
   goto generic_failure;
  }
 } CATCH (E_SEGFAULT) {
  goto generic_failure;
 }
do_throw_error:
 /* Copy the CPU context at the time of the exception. */
 fix_user_context(xcontext);
 memcpy(&info->e_context,&xcontext->c_host,sizeof(struct cpu_context));
 error_rethrow_atuser((struct cpu_context *)xcontext);
 return;
null_segment_error:
#ifdef E_INVALID_SEGMENT
 info->e_error.e_code = E_INVALID_SEGMENT;
#else
 info->e_error.e_code = E_ILLEGAL_INSTRUCTION;
#endif
 info->e_error.e_illegal_instruction.ii_errcode = xerrcode;
 info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_UNDEFINED|
                                                ERROR_ILLEGAL_INSTRUCTION_FVALUE|
                                                ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
 info->e_error.e_illegal_instruction.ii_register_type = X86_REGISTER_SEGMENT;
 info->e_error.e_illegal_instruction.ii_value = effective_segment_value;
 switch (flags & F_SEGMASK) {
 default: info->e_error.e_illegal_instruction.ii_value = X86_REGISTER_SEGMENT_DS; break;
 case F_SEGES: info->e_error.e_illegal_instruction.ii_value = X86_REGISTER_SEGMENT_ES; break;
 case F_SEGFS: info->e_error.e_illegal_instruction.ii_value = X86_REGISTER_SEGMENT_FS; break;
 case F_SEGGS: info->e_error.e_illegal_instruction.ii_value = X86_REGISTER_SEGMENT_GS; break;
 case F_SEGCS: info->e_error.e_illegal_instruction.ii_value = X86_REGISTER_SEGMENT_CS; break;
 case F_SEGSS: info->e_error.e_illegal_instruction.ii_value = X86_REGISTER_SEGMENT_SS; break;
 }
 goto do_throw_error;
generic_failure:
 /* Fallback: Choose between a NULL-segment and generic error. */
 if (!effective_segment_value)
      goto null_segment_error;
 /* If the error originated from user-space, default to assuming it's
  * because of some privileged instruction not explicitly handled
  * above. (e.g.: `wbinvd') */
 if (xcontext->c_iret.ir_cs & 3)
     goto e_privileged_instruction;
 /* In kernel space, this one's a wee bit more complicated... */
 info->e_error.e_code = E_UNHANDLED_INTERRUPT;
 info->e_error.e_unhandled_interrupt.ui_intcode = X86_E_SYSTEM_GP & 0xff;
 info->e_error.e_unhandled_interrupt.ui_errcode = xerrcode;
 goto do_throw_error;
e_privileged_instruction:
 /* Throw a privileged-instruction error. */
 info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                ERROR_ILLEGAL_INSTRUCTION_FINSTRUCTION);
 goto do_throw_error;
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_ILLEGAL_INSTRUCTION_C */
