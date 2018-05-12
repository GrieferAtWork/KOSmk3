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

INTERN byte_t *KCALL
x86_decode_modrm(byte_t *__restrict text,
                 struct modrm_info *__restrict info) {
 u8 rmbyte = *text++;
 info->mi_reg    = X86_MODRM_GETREG(rmbyte);
 info->mi_rm     = X86_MODRM_GETRM(rmbyte);
 info->mi_type   = MODRM_REGISTER;
 if ((rmbyte&X86_MODRM_MOD_MASK) == (0x3 << X86_MODRM_MOD_SHIFT))
      goto done; /* Register operand. */
 info->mi_type = MODRM_MEMORY;
 info->mi_shift = 1;
 info->mi_index = 0xff;
 info->mi_offset = 0;
 if (info->mi_rm == X86_REG_EBP) {
  info->mi_rm     = 0xff;
  info->mi_offset = *(u32 *)text;
  text += 4;
 } else  {
  if (info->mi_rm == X86_REG_ESP) {
   u8 sibbyte = *text++;
   info->mi_shift = X86_MODRM_GETMOD(sibbyte);
   info->mi_index = X86_MODRM_GETREG(sibbyte);
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

#define X86_GPREG(context,no)  ((uintptr_t *)&(context).c_gpregs)[7-(no)]
#define X86_GPREG8(context,no) ((u8 *)&(context).c_gpregs+32)[7-(no)]


#define modrm_getreg(context,modrm) \
     X86_GPREG(*(context),(modrm)->mi_rm)
#define modrm_getreg8(context,modrm) \
     X86_GPREG8(*(context),(modrm)->mi_rm)

#ifdef __x86_64__
#define fix_user_context(context) (void)0
#else
INTDEF void FCALL fix_user_context(struct x86_anycontext *__restrict context);
#endif

INTDEF void FCALL
error_rethrow_atuser(struct cpu_context *__restrict context);

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

INTERN uintptr_t KCALL
x86_modrm_getmem(struct cpu_anycontext *__restrict context,
                 struct modrm_info *__restrict modrm,
                 u16 flags) {
 uintptr_t result;
 if (modrm->mi_type == MODRM_REGISTER)
     return (uintptr_t)&X86_GPREG(*context,modrm->mi_reg);
 result = modrm->mi_offset;
 if (modrm->mi_reg != 0xff)
     result += X86_GPREG(*context,modrm->mi_reg);
 if (modrm->mi_index != 0xff)
     result += X86_GPREG(*context,modrm->mi_index) << modrm->mi_shift;
 switch (flags & F_SEGMASK) {
#ifndef CONFIG_NO_X86_SEGMENTATION
 case F_SEGDS: result += get_segment_base(context,context->c_segments.sg_ds); break;
 case F_SEGES: result += get_segment_base(context,context->c_segments.sg_es); break;
 case F_SEGFS: result += get_segment_base(context,context->c_segments.sg_fs); break;
 case F_SEGGS: result += get_segment_base(context,context->c_segments.sg_gs); break;
#else
#ifdef __ASM_TASK_SEGMENT_ISFS /* If we're using %fs, user-space is using %gs */
 case F_SEGGS: result += (uintptr_t)THIS_TASK->t_userseg; break;
#else /* If we're using %gs, user-space is using %fs */
 case F_SEGFS: result += (uintptr_t)THIS_TASK->t_userseg; break;
#endif
#endif
 case F_SEGCS: result += get_segment_base(context,context->c_iret.ir_cs); break;
 case F_SEGSS: result += get_segment_base(context,context->c_iret.ir_ss); break;

 default: break;
 }
 return result;
}

INTERN u8 KCALL
x86_modrm_getb(struct cpu_anycontext *__restrict context,
               struct modrm_info *__restrict modrm, u16 flags) {
 uintptr_t addr;
 if (modrm->mi_type == MODRM_REGISTER)
     return modrm_getreg8(context,modrm);
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
     return (u16)modrm_getreg(context,modrm);
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
     return (u32)modrm_getreg(context,modrm);
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
     modrm_getreg8(context,modrm) = value;
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
     modrm_getreg(context,modrm) = value;
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
     modrm_getreg(context,modrm) = value;
 else {
  uintptr_t addr;
  addr = x86_modrm_getmem(context,modrm,flags);
  if ((context->c_iret.ir_cs & 3) ||
      (context->c_eflags & EFLAGS_VM))
       validate_writable((void *)addr,4);
  *(u32 *)addr = value;
 }
}



DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_EMULATOR_C */
