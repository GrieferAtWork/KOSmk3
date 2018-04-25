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


#ifdef __x86_64__
#define CONTEXT_IP(x)    ((x).c_rip)
#define CONTEXT_FLAGS(x) ((x).c_rflags)
#define CONTEXT_AREG(x)  ((x).c_gpregs.gp_rax)
#define CONTEXT_CREG(x)  ((x).c_gpregs.gp_rcx)
#define CONTEXT_DREG(x)  ((x).c_gpregs.gp_rdx)
#define CONTEXT_BREG(x)  ((x).c_gpregs.gp_rbx)
#else
#define CONTEXT_IP(x)    ((x).c_eip)
#define CONTEXT_FLAGS(x) ((x).c_eflags)
#define CONTEXT_AREG(x)  ((x).c_gpregs.gp_eax)
#define CONTEXT_CREG(x)  ((x).c_gpregs.gp_ecx)
#define CONTEXT_DREG(x)  ((x).c_gpregs.gp_edx)
#define CONTEXT_BREG(x)  ((x).c_gpregs.gp_ebx)
#endif
#define X86_GPREG(context,no)  ((uintptr_t *)&(context).c_gpregs)[7-(no)]
#define X86_GPREG8(context,no) ((u8 *)&(context).c_gpregs+32)[7-(no)]


#ifndef CONFIG_NO_SMP
PRIVATE DEFINE_ATOMIC_RWLOCK(bus_lock);
#define BUS_ACQUIRE() atomic_rwlock_write(&bus_lock); TRY {
#define BUS_RELEASE() } FINALLY { atomic_rwlock_endwrite(&bus_lock); }
#else
#define BUS_ACQUIRE() PREEMPTION_DISABLE()
#define BUS_RELEASE() PREEMPTION_ENABLE()
#endif

PRIVATE u8 KCALL
read_byte(struct cpu_anycontext *__restrict context,
          VIRT uintptr_t addr) {
 if (context->c_iret.ir_cs & 3)
     validate_readable((void *)addr,1);
 return *(u8 *)addr;
}
PRIVATE u16 KCALL
read_word(struct cpu_anycontext *__restrict context,
          VIRT uintptr_t addr) {
 if (context->c_iret.ir_cs & 3)
     validate_readable((void *)addr,2);
 return *(u16 *)addr;
}
PRIVATE u32 KCALL
read_dword(struct cpu_anycontext *__restrict context,
           VIRT uintptr_t addr) {
 if (context->c_iret.ir_cs & 3)
     validate_readable((void *)addr,4);
 return *(u32 *)addr;
}

PRIVATE void KCALL
write_byte(struct cpu_anycontext *__restrict context,
           VIRT uintptr_t addr, u8 value) {
 if (context->c_iret.ir_cs & 3)
     validate_writable((void *)addr,1);
 *(u8 *)addr = value;
}
PRIVATE void KCALL
write_word(struct cpu_anycontext *__restrict context,
           VIRT uintptr_t addr, u16 value) {
 if (context->c_iret.ir_cs & 3)
     validate_writable((void *)addr,2);
 *(u16 *)addr = value;
}
PRIVATE void KCALL
write_dword(struct cpu_anycontext *__restrict context,
            VIRT uintptr_t addr, u32 value) {
 if (context->c_iret.ir_cs & 3)
     validate_writable((void *)addr,4);
 *(u32 *)addr = value;
}

#define modrm_getreg(context,modrm) \
     X86_GPREG(*(context),(modrm)->mi_rm)
#define modrm_getreg8(context,modrm) \
     X86_GPREG8(*(context),(modrm)->mi_rm)

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
     return X86_GPREG(*context,modrm->mi_reg);
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
 if (modrm->mi_type == MODRM_REGISTER)
     return modrm_getreg8(context,modrm);
 return *(u8 *)x86_modrm_getmem(context,modrm,flags);
}
INTERN u16 KCALL
x86_modrm_getw(struct cpu_anycontext *__restrict context,
               struct modrm_info *__restrict modrm, u16 flags) {
 if (modrm->mi_type == MODRM_REGISTER)
     return (u16)modrm_getreg(context,modrm);
 return *(u16 *)x86_modrm_getmem(context,modrm,flags);
}
INTERN u32 KCALL
x86_modrm_getl(struct cpu_anycontext *__restrict context,
               struct modrm_info *__restrict modrm, u16 flags) {
 if (modrm->mi_type == MODRM_REGISTER)
     return (u32)modrm_getreg(context,modrm);
 return *(u32 *)x86_modrm_getmem(context,modrm,flags);
}


/* Compare operands and return EFLAGS. */
PRIVATE register_t KCALL x86_cmpb(u8 a, u8 b);
PRIVATE register_t KCALL x86_cmpw(u16 a, u16 b);
PRIVATE register_t KCALL x86_cmpl(u32 a, u32 b);

PRIVATE register_t KCALL x86_cmpb(u8 a, u8 b) {
 register_t result;
 __asm__("cmpb %b2, %b1\n"
         "pushfl\n"
         "popl %0\n"
         : "=g" (result)
         : "q" (a), "q" (b)
         : "cc");
 return result;
}
PRIVATE register_t KCALL x86_cmpw(u16 a, u16 b) {
 register_t result;
 __asm__("cmpw %w2, %w1\n"
         "pushfl\n"
         "popl %0\n"
         : "=g" (result)
         : "q" (a), "q" (b)
         : "cc");
 return result;
}
PRIVATE register_t KCALL x86_cmpl(u32 a, u32 b) {
 register_t result;
 __asm__("cmpl %2, %1\n"
         "pushfl\n"
         "popl %0\n"
         : "=g" (result)
         : "r" (a), "r" (b)
         : "cc");
 return result;
}


/* Called by the #UD interrupt to emulate instruction not supported natively.
 * When `true' is returned, the instruction has been emulated and registers were updated. */
INTERN bool KCALL
x86_emulate_instruction(struct cpu_anycontext *__restrict context) {
 struct modrm_info modrm;
 u32 instruction; byte_t *text;
 u16 flags = 0;
 text = (byte_t *)CONTEXT_IP(*context);
next_byte:
 instruction = 0;
extend_instruction:
 TRY instruction |= *text++;
 CATCH_HANDLED (E_SEGFAULT) goto fail;
 switch (instruction) {
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
 case 0x0f: instruction <<= 8; goto extend_instruction;

 {
  uintptr_t addr;
  register_t new_flags;
  u8 value;
 case 0x0fb0:
  /* cmpxchg r/m8,r8 */
  text = x86_decode_modrm(text,&modrm);
  addr = x86_modrm_getmem(context,&modrm,flags);
  /* We ignore the LOCK-prefix and always
   * acquire a lock to our emulated BUS. */
  BUS_ACQUIRE();
  value = read_byte(context,addr);
  new_flags = x86_cmpb((u8)CONTEXT_AREG(*context),value);
  if (new_flags & EFLAGS_ZF) {
   write_byte(context,addr,modrm_getreg8(context,&modrm));
  } else {
   *(u8 *)&CONTEXT_AREG(*context) = value;
  }
  BUS_RELEASE();
  context->c_eflags &= ~(EFLAGS_ZF|EFLAGS_CF|EFLAGS_PF|
                         EFLAGS_AF|EFLAGS_SF|EFLAGS_OF);
  context->c_eflags |= new_flags;
  goto ok;

 case 0x0fb1:
  /* cmpxchg r/m16,r16 */
  /* cmpxchg r/m32,r32 */
  text = x86_decode_modrm(text,&modrm);
  addr = x86_modrm_getmem(context,&modrm,flags);
  /* We ignore the LOCK-prefix and always
   * acquire a lock to our emulated BUS. */
  BUS_ACQUIRE();
  register_t value;
  if (flags&F_OP16) {
   value     = read_word(context,addr);
   new_flags = x86_cmpw((u16)CONTEXT_AREG(*context),(u16)value);
   if (new_flags & EFLAGS_ZF) {
    write_word(context,addr,(u16)modrm_getreg(context,&modrm));
   } else {
    *(u16 *)&CONTEXT_AREG(*context) = value;
   }
  } else {
   value     = read_dword(context,addr);
   new_flags = x86_cmpl((u32)CONTEXT_AREG(*context),(u32)value);
   if (new_flags & EFLAGS_ZF) {
    write_dword(context,addr,(u32)modrm_getreg(context,&modrm));
   } else {
    *(u32 *)&CONTEXT_AREG(*context) = value;
   }
  }
  BUS_RELEASE();
  context->c_eflags &= ~(EFLAGS_ZF|EFLAGS_CF|EFLAGS_PF|
                         EFLAGS_AF|EFLAGS_SF|EFLAGS_OF);
  context->c_eflags |= new_flags;
  goto ok;
 }

 {
  uintptr_t addr;
  register_t new_flags;
  u8 value;
 case 0x0fc0:
  /* xadd r/m8, r8 */
  text = x86_decode_modrm(text,&modrm);
  addr = x86_modrm_getmem(context,&modrm,flags);
  BUS_ACQUIRE();
  value = read_byte(context,addr);
  write_byte(context,addr,(u8)(value+modrm_getreg8(context,&modrm)));
  modrm_getreg8(context,&modrm) = value;
  new_flags = x86_cmpb(value,0);
  BUS_RELEASE();
  context->c_eflags &= ~(EFLAGS_CF|EFLAGS_PF|EFLAGS_AF|
                         EFLAGS_SF|EFLAGS_ZF|EFLAGS_OF);
  context->c_eflags |= new_flags;
  goto ok;
 } break;

 {
  uintptr_t addr;
  register_t new_flags;
 case 0x0fc1:
  /* xadd r/m16, r16 */
  /* xadd r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  addr = x86_modrm_getmem(context,&modrm,flags);
  BUS_ACQUIRE();
  if (flags & F_OP16) {
   u16 value = read_word(context,addr);
   write_word(context,addr,(u16)(value+modrm_getreg(context,&modrm)));
   *(u16 *)&modrm_getreg(context,&modrm) = value;
   new_flags = x86_cmpw(value,0);
  } else {
   u32 value = read_dword(context,addr);
   write_dword(context,addr,(u32)(value+modrm_getreg(context,&modrm)));
   *(u32 *)&modrm_getreg(context,&modrm) = value;
   new_flags = x86_cmpl(value,0);
  }
  BUS_RELEASE();
  context->c_eflags &= ~(EFLAGS_CF|EFLAGS_PF|EFLAGS_AF|
                         EFLAGS_SF|EFLAGS_ZF|EFLAGS_OF);
  context->c_eflags |= new_flags;
  goto ok;
 } break;

 case 0x0fc7:
  text = x86_decode_modrm(text,&modrm);
  if (modrm.mi_rm == 1) {
   /* cmpxchg8b m64 */
   uintptr_t addr;
   if (modrm.mi_type != MODRM_MEMORY)
       goto fail;
   addr = x86_modrm_getmem(context,&modrm,flags);
   /* We ignore the LOCK-prefix and always
    * acquire a lock to our emulated BUS. */
   BUS_ACQUIRE();
   u32 value[2];
   value[0] = read_dword(context,addr);
   value[1] = read_dword(context,addr+4);
   if (value[0] == CONTEXT_AREG(*context) &&
       value[1] == CONTEXT_DREG(*context)) {
    write_dword(context,addr,CONTEXT_BREG(*context));
    write_dword(context,addr+4,CONTEXT_CREG(*context));
    CONTEXT_FLAGS(*context) |= EFLAGS_ZF;
   } else {
    CONTEXT_AREG(*context)   = value[0];
    CONTEXT_DREG(*context)   = value[1];
    CONTEXT_FLAGS(*context) &= ~EFLAGS_ZF;
   }
   BUS_RELEASE();
   goto ok;
  }
  break;

  /* TODO: cpuid */

 default: break;
 }
fail:
 return false;
ok:
 CONTEXT_IP(*context) = (register_t)text;
 return true;
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_EMULATOR_C */
