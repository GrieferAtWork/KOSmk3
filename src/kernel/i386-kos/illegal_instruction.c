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
#include <kernel/user.h>
#include <i386-kos/vm86.h>
#include <i386-kos/gdt.h>
#include <asm/cpu-flags.h>
#include <sched/task.h>
#include <except.h>
#include <string.h>
#include <stdbool.h>
#include <syscall.h>
#include <kernel/syscall.h>
#include <sys/io.h>

#include "emulator.h"

DECL_BEGIN


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
#define X86_GPREG(no)  ((uintptr_t *)&context->c_gpregs)[7-(no)]
#define X86_GPREG8(no) ((u8 *)&context->c_gpregs+32)[7-(no)]
#define modrm_getreg(modrm)  X86_GPREG((modrm).mi_rm)
#define modrm_getreg8(modrm) X86_GPREG8((modrm).mi_rm)


LOCAL register_t KCALL __cmpb(u8 a, u8 b) {
 register_t result;
 __asm__("cmpb %b2, %b1\n\r"
         "pushfl\n\r"
         "popl %0"
         : "=g" (result)
         : "q" (a)
         , "q" (b)
         : "cc");
 return result;
}
LOCAL register_t KCALL __cmpw(u16 a, u16 b) {
 register_t result;
 __asm__("cmpw %w2, %w1\n\r"
         "pushfl\n\r"
         "popl %0"
         : "=g" (result)
         : "q" (a)
         , "q" (b)
         : "cc");
 return result;
}
LOCAL register_t KCALL __cmpl(u32 a, u32 b) {
 register_t result;
 __asm__("cmpl %2, %1\n\r"
         "pushfl\n\r"
         "popl %0"
         : "=g" (result)
         : "r" (a)
         , "r" (b)
         : "cc");
 return result;
}

#define CF       EFLAGS_CF
#define PF       EFLAGS_PF
#define AF       EFLAGS_AF
#define ZF       EFLAGS_ZF
#define SF       EFLAGS_SF
#define TF       EFLAGS_TF
#define IF       EFLAGS_IF
#define DF       EFLAGS_DF
#define OF       EFLAGS_OF
#define IOPL     EFLAGS_IOPL
#define NT       EFLAGS_NT
#define RF       EFLAGS_RF
#define VM       EFLAGS_VM
#define AC       EFLAGS_AC
#define VIF      EFLAGS_VIF
#define VIP      EFLAGS_VIP
#define ID       EFLAGS_ID


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


#ifndef __x86_64__
#ifdef CONFIG_NO_SMP
#define BUS_ACQUIRE() do{ COMPILER_BARRIER(); PREEMPTION_DISABLE(); }__WHILE0
#define BUS_RELEASE() do{ PREEMPTION_POP(context->c_eflags); COMPILER_BARRIER(); }__WHILE0
#else
#define BUS_ACQUIRE() COMPILER_BARRIER(); impl_bus_acquire(); TRY
#define BUS_RELEASE() FINALLY { impl_bus_release(context->c_eflags); COMPILER_BARRIER(); }

PRIVATE ATTR_USED u32 bus_cpu = 0;
PRIVATE ATTR_USED u32 bus_recursion = 0;

LOCAL ATTR_NOTHROW void KCALL impl_bus_acquire(void) {
 register u32 lock,my_cpuid;
 PREEMPTION_DISABLE();
 my_cpuid = THIS_CPU->cpu_id;
 for (;;) {
  lock = my_cpuid;
  __asm__ __volatile__("xchgl bus_cpu, %0"
                       : "+r" (lock)
                       :
                       : "memory");
  if (!lock) break;
  if (lock == my_cpuid) { ++bus_recursion; break; }
#if 0 /* Machines on which we need to emulate some lock-instruction
       * usually don't have SSE, which implemented `pause'... */
  __asm__ __volatile__("pause");
#endif
 }
}
LOCAL ATTR_NOTHROW void KCALL
impl_bus_release(register_t old_eflags) {
 register u32 temp;
 if (bus_recursion)
   --bus_recursion;
 else {
  __asm__ __volatile__("xchgl bus_cpu, %0"
                       : "=r" (temp)
                       : "0" (0)
                       : "memory");
  PREEMPTION_POP(old_eflags);
 }
}
#endif
#endif /* !__x86_64__ */





INTERN NOIRQ void FCALL
x86_handle_illegal_instruction(struct x86_anycontext *__restrict context) {
 struct x86_anycontext *EXCEPT_VAR xcontext = context;
 struct exception_info *info; u16 flags;
 byte_t *EXCEPT_VAR text; u32 opcode;
 bool is_user;

 flags = 0;
 text = (byte_t *)context->c_eip;
 is_user = ((xcontext->c_eflags & EFLAGS_VM) ||
            (xcontext->c_iret.ir_cs & 3));

next_byte:
 opcode = 0;
extend_instruction:
 TRY {
  opcode |= *text++;
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

#ifndef __x86_64__
   /* Emulate atomic instruction that were only added later */
  case 0x0fb0: {
   struct modrm_info modrm;
   uintptr_t addr;
   u8 COMPILER_IGNORE_UNINITIALIZED(real_old_value);
   u8 old_value,new_value;
   /* EMULATOR: cmpxchg r/m8,r8 */
   text = x86_decode_modrm(text,&modrm);
   addr = x86_modrm_getmem(context,&modrm,flags);
   if (is_user) validate_writable((void *)addr,1);
   old_value = (u8)CONTEXT_AREG(*context);
   new_value = modrm_getreg8(modrm);
   if (flags & F_LOCK) {
    BUS_ACQUIRE() {
     real_old_value = *(u8 volatile *)addr;
     if (old_value == real_old_value)
         *(u8 volatile *)addr = new_value;
    }
    BUS_RELEASE();
   } else {
    real_old_value = *(u8 volatile *)addr;
    if (old_value == real_old_value)
        *(u8 volatile *)addr = new_value;
   }
   *(u8 *)&CONTEXT_AREG(*context) = real_old_value;
   context->c_eflags &= (ZF|CF|PF|AF|SF|OF);
   context->c_eflags |= __cmpb(old_value,real_old_value) & (ZF|CF|PF|AF|SF|OF);
  } break;

  {
   struct modrm_info modrm;
   uintptr_t addr;
  case 0x0fb1:
   /* cmpxchg r/m16,r16 */
   /* cmpxchg r/m32,r32 */
   text = x86_decode_modrm(text,&modrm);
   addr = x86_modrm_getmem(context,&modrm,flags);
   if (flags & F_OP16) {
    u16 COMPILER_IGNORE_UNINITIALIZED(real_old_value);
    u16 old_value,new_value;
    if (is_user) validate_writable((void *)addr,2);
    old_value = (u16)CONTEXT_AREG(*context);
    new_value = (u16)modrm_getreg(modrm);
    if (flags & F_LOCK) {
     BUS_ACQUIRE() {
      real_old_value = *(u16 volatile *)addr;
      if (old_value == real_old_value)
          *(u16 volatile *)addr = new_value;
     }
     BUS_RELEASE();
    } else {
     real_old_value = *(u16 volatile *)addr;
     if (old_value == real_old_value)
         *(u16 volatile *)addr = new_value;
    }
    *(u16 *)&CONTEXT_AREG(*context) = real_old_value;
    context->c_eflags &= (ZF|CF|PF|AF|SF|OF);
    context->c_eflags |= __cmpw(old_value,real_old_value) & (ZF|CF|PF|AF|SF|OF);
   } else {
    u32 COMPILER_IGNORE_UNINITIALIZED(real_old_value);
    u32 old_value,new_value;
    if (is_user) validate_writable((void *)addr,4);
    old_value = (u32)CONTEXT_AREG(*context);
    new_value = (u32)modrm_getreg(modrm);
    if (flags & F_LOCK) {
     BUS_ACQUIRE() {
      real_old_value = *(u32 volatile *)addr;
      if (old_value == real_old_value)
          *(u32 volatile *)addr = new_value;
     }
     BUS_RELEASE();
    } else {
     real_old_value = *(u32 volatile *)addr;
     if (old_value == real_old_value)
         *(u32 volatile *)addr = new_value;
    }
    *(u32 *)&CONTEXT_AREG(*context) = real_old_value;
    context->c_eflags &= (ZF|CF|PF|AF|SF|OF);
    context->c_eflags |= __cmpl(old_value,real_old_value) & (ZF|CF|PF|AF|SF|OF);
   }
  } break;

  case 0x0fc0: {
   struct modrm_info modrm;
   uintptr_t addr; u8 add_value;
   u8 COMPILER_IGNORE_UNINITIALIZED(old_value);
   /* xadd r/m8, r8 */
   text = x86_decode_modrm(text,&modrm);
   addr = x86_modrm_getmem(context,&modrm,flags);
   if (is_user) validate_writable((void *)addr,1);
   add_value = modrm_getreg8(modrm);
   if (flags & F_LOCK) {
    BUS_ACQUIRE() {
     old_value = *(u8 volatile *)addr;
     *(u8 volatile *)addr = old_value + add_value;
    }
    BUS_RELEASE();
   } else {
    old_value = *(u8 volatile *)addr;
    *(u8 volatile *)addr = old_value + add_value;
   }
   modrm_getreg8(modrm) = old_value;
   context->c_eflags &= ~(CF|PF|AF|SF|ZF|OF);
   context->c_eflags |= __cmpb(old_value,0) & (CF|PF|AF|SF|ZF|OF);
  } break;

  {
   struct modrm_info modrm;
   uintptr_t addr;
  case 0x0fc1:
   /* xadd r/m16, r16 */
   /* xadd r/m32, r32 */
   text = x86_decode_modrm(text,&modrm);
   addr = x86_modrm_getmem(context,&modrm,flags);
   if (flags & F_OP16) {
    u16 COMPILER_IGNORE_UNINITIALIZED(old_value);
    u16 add_value;
    if (is_user) validate_writable((void *)addr,2);
    add_value = (u16)modrm_getreg(modrm);
    if (flags & F_LOCK) {
     BUS_ACQUIRE() {
      old_value = *(u16 volatile *)addr;
      *(u16 volatile *)addr = old_value + add_value;
     }
     BUS_RELEASE();
    } else {
     old_value = *(u16 volatile *)addr;
     *(u16 volatile *)addr = old_value + add_value;
    }
    *(u16 *)&modrm_getreg(modrm) = old_value;
    context->c_eflags &= ~(CF|PF|AF|SF|ZF|OF);
    context->c_eflags |= __cmpw(old_value,0) & (CF|PF|AF|SF|ZF|OF);
   } else {
    u32 COMPILER_IGNORE_UNINITIALIZED(old_value);
    u32 add_value;
    if (is_user) validate_writable((void *)addr,4);
    add_value = (u32)modrm_getreg(modrm);
    if (flags & F_LOCK) {
     BUS_ACQUIRE() {
      old_value = *(u32 volatile *)addr;
      *(u32 volatile *)addr = old_value + add_value;
     }
     BUS_RELEASE();
    } else {
     old_value = *(u32 volatile *)addr;
     *(u32 volatile *)addr = old_value + add_value;
    }
    *(u32 *)&modrm_getreg(modrm) = old_value;
    context->c_eflags &= ~(CF|PF|AF|SF|ZF|OF);
    context->c_eflags |= __cmpl(old_value,0) & (CF|PF|AF|SF|ZF|OF);
   }
  } break;

  {
   struct modrm_info modrm;
  case 0x0fc7:
   text = x86_decode_modrm(text,&modrm);
   if (modrm.mi_rm == 1) {
    u32 value[2];
    /* cmpxchg8b m64 */
    uintptr_t addr;
    if (modrm.mi_type != MODRM_MEMORY)
        goto illegal_addressing_mode;
    addr = x86_modrm_getmem(context,&modrm,flags);
    if (is_user) validate_writable((void *)addr,8);
    if (flags & F_LOCK) {
     BUS_ACQUIRE() {
      value[0] = ((u32 *)addr)[0];
      value[1] = ((u32 *)addr)[1];
      if (value[0] == CONTEXT_AREG(*context) &&
          value[1] == CONTEXT_DREG(*context)) {
       ((u32 *)addr)[0] = CONTEXT_BREG(*context);
       ((u32 *)addr)[1] = CONTEXT_CREG(*context);
       CONTEXT_FLAGS(*context) |= EFLAGS_ZF;
      } else {
       CONTEXT_AREG(*context)   = value[0];
       CONTEXT_DREG(*context)   = value[1];
       CONTEXT_FLAGS(*context) &= ~EFLAGS_ZF;
      }
     }
     BUS_RELEASE();
    } else {
     value[0] = ((u32 *)addr)[0];
     value[1] = ((u32 *)addr)[1];
     if (value[0] == CONTEXT_AREG(*context) &&
         value[1] == CONTEXT_DREG(*context)) {
      ((u32 *)addr)[0] = CONTEXT_BREG(*context);
      ((u32 *)addr)[1] = CONTEXT_CREG(*context);
      CONTEXT_FLAGS(*context) |= EFLAGS_ZF;
     } else {
      CONTEXT_AREG(*context)   = value[0];
      CONTEXT_DREG(*context)   = value[1];
      CONTEXT_FLAGS(*context) &= ~EFLAGS_ZF;
     }
    }
    break;
   }
   goto generic_illegal_instruction;
  } break;

#endif /* !__x86_64__ */


  {
   struct modrm_info modrm;
  case 0x0f01:
   text = x86_decode_modrm(text,&modrm);
   /* LGDT and LIDT throw an #UD when a register operand is used. */
   if (modrm.mi_rm == 2) goto illegal_addressing_mode; /* LGDT m16&32 */
   if (modrm.mi_rm == 3) goto illegal_addressing_mode; /* LIDT m16&32 */
   goto generic_illegal_instruction;
  } break;

  {
   syscall_ulong_t EXCEPT_VAR sysno;
   syscall_ulong_t EXCEPT_VAR orig_eax;
   syscall_ulong_t EXCEPT_VAR orig_eip;
   syscall_ulong_t EXCEPT_VAR orig_ebp;
   syscall_ulong_t EXCEPT_VAR orig_edi;
   syscall_ulong_t EXCEPT_VAR orig_esp;
  case 0x0f34:
   /* SYSENTER */
#if 0
   if unlikely(!is_user) goto generic_illegal_instruction;
#endif
   orig_eax = context->c_gpregs.gp_eax;
   orig_eip = context->c_eip;
   orig_ebp = context->c_gpregs.gp_ebp;
   orig_edi = context->c_gpregs.gp_edi;
   orig_esp = context->c_iret.ir_useresp;
restart_sysenter_syscall:
   TRY {
    /* Convert the user-space register context to become `int $0x80'-compatible */
    syscall_ulong_t masked_sysno; u8 argc = 6;
    sysno                       = xcontext->c_gpregs.gp_eax;
    masked_sysno                = sysno & ~0x80000000;
    xcontext->c_eip             = orig_edi; /* CLEANUP: return.%eip = %edi */
    xcontext->c_iret.ir_useresp = orig_ebp; /* CLEANUP: return.%esp = %ebp */
    /* Figure out how many arguments this syscall takes. */
    if (masked_sysno <= __NR_syscall_max)
     argc = x86_syscall_argc[masked_sysno];
    else if (masked_sysno >= __NR_xsyscall_min &&
             masked_sysno <= __NR_xsyscall_max) {
     argc = x86_xsyscall_argc[masked_sysno-__NR_xsyscall_min];
    }
    /* Load additional arguments from user-space. */
    if (argc >= 4)
        xcontext->c_gpregs.gp_edi = *((u32 *)(orig_ebp + 0));
    if (argc >= 5)
        xcontext->c_gpregs.gp_ebp = *((u32 *)(orig_ebp + 4));
    COMPILER_READ_BARRIER();
    /* Actually execute the system call (using the `int $0x80'-compatible register set). */
    x86_syscall_exec80();
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    /* Set the FSYSCALL flag so that exceptions are propagated accordingly. */
    error_info()->e_error.e_flag |= X86_INTERRUPT_GUARD_FSYSCALL;
    if (error_code() == E_INTERRUPT) {
     /* Restore the original user-space CPU xcontext. */
     xcontext->c_gpregs.gp_eax   = orig_eax;
     xcontext->c_eip             = orig_eip;
     xcontext->c_gpregs.gp_ebp   = orig_ebp;
     xcontext->c_gpregs.gp_edi   = orig_edi;
     xcontext->c_iret.ir_useresp = orig_esp;
     COMPILER_WRITE_BARRIER();
     /* Deal with system call restarts. */
     task_restart_syscall((struct cpu_hostcontext_user *)xcontext,
                           TASK_USERCTX_TYPE_WITHINUSERCODE|
                           X86_SYSCALL_TYPE_FPF,
                           sysno);
     COMPILER_BARRIER();
     goto restart_sysenter_syscall;
    }
    error_rethrow();
   }
  } break;

  case 0x0f35:
   /* SYSEXIT */
   if (is_user) goto privileged_instruction;
   context->c_iret.ir_cs      = X86_USER_CS;
   context->c_iret.ir_ss      = X86_USER_DS;
   context->c_iret.ir_useresp = context->c_gpregs.gp_ecx;
   context->c_iret.ir_eip     = context->c_gpregs.gp_edx;
   break;

  {
   struct modrm_info modrm;
   uintptr_t bounds_struct;
   u32 low,high,index;
  case 0x62:
   /* BOUND r16, m16&16     (Added with 80186/80188) */
   /* BOUND r32, m32&32     (Added with 80186/80188) */
   text = x86_decode_modrm(text,&modrm);
   bounds_struct = x86_modrm_getmem(context,&modrm,flags);
   if (flags & F_OP16) {
    low   = ((u16 *)bounds_struct)[0];
    high  = ((u16 *)bounds_struct)[1];
    index = (u16)modrm_getreg(modrm);
   } else {
    low  = ((u32 *)bounds_struct)[0];
    high = ((u32 *)bounds_struct)[1];
    index = (u32)modrm_getreg(modrm);
   }
   /* Do the actual bounds check. */
   if (index >= low && index <= high)
       break; /* Check was OK. */
   /* The check failed. - Throw an `E_INDEX_ERROR', like we do for this case. */
   info                 = error_info();
   info->e_error.e_code = E_INDEX_ERROR;
   info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
   memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
   info->e_error.e_index_error.b_boundmin = low;
   info->e_error.e_index_error.b_boundmax = high;
   info->e_error.e_index_error.b_index    = index;
   goto throw_exception;
  } break;


#if 0 /* Ups... I read the wikipedia article wrong.
       * These instructions may not have been present on the first CPU in the x86 line,
       * however that first CPU didn't have protected mode either, so there's really no
       * point in trying to emulate these, considering that if ever were to run on hardware
       * where we had to emulate them, we would have never gotten this far. */
  {
   u16 frame_size;
   u8  nesting_level;
   uintptr_t frame_temp;
   uintptr_t new_esp;
   uintptr_t new_ebp;
  case 0xc8:
   /* ENTER imm16,imm8 */
   frame_size     = *(u16 *)text,text += 2;
   nesting_level  = *(u8 *)text,text += 1;
   nesting_level %= 32;
   /* Seeing how we're currently _on_ the stack we're trying to modify,
    * emulating this one might be possible, but would be unnecessary
    * and complicated. (especially unnecessary because we don't
    * actually use this instruction) */
   if (!is_user)
        goto generic_illegal_instruction;
   new_esp = context->c_useresp;
   new_ebp = context->c_gpregs.gp_ebp;
   /* I don't really understand the purpose of this nesting-level, but
    * this emulation should be identical to Intel's documentation... */
   if (!(flags & F_AD16)) {
    new_esp -= 4;
    validate_writable((void *)new_esp,4);
    *(u32 *)new_esp = new_ebp;
    frame_temp = new_esp;
   } else {
    new_esp -= 2;
    *(u16 *)new_esp = (u16)new_ebp;
    frame_temp = new_esp & 0xffff;
   }
   if (nesting_level != 0) {
    while (--nesting_level) {
     if (!(flags & F_OP16)) {
      new_ebp -= 4;
     } else {
      new_ebp -= 2;
     }
     if (flags & F_AD16)
         new_ebp &= 0xffff;
     new_esp -= 4;
     validate_writable((void *)new_esp,4);
     *(u32 *)new_esp = new_ebp;
    }
    if (!(flags & F_OP16)) {
     new_esp -= 4;
     validate_writable((void *)new_esp,4);
     *(u32 *)new_esp = frame_temp;
    } else {
     new_esp -= 2;
     validate_writable((void *)new_esp,2);
     *(u16 *)new_esp = (u16)frame_temp;
    }
   }
   if (!(flags & F_AD16)) {
    new_ebp = frame_temp;
    new_esp = new_ebp - frame_size;
   } else {
    new_ebp = frame_temp & 0xffff;
    new_esp = (new_ebp - frame_size) & 0xffff;
   }
   context->c_useresp       = new_esp;
   context->c_gpregs.gp_ebp = new_ebp;
  } break;

  {
  case 0xc9:
   /* LEAVE */
   if (is_user) {
    validate_readable((void *)((flags & F_AD16
                              ? context->c_gpregs.gp_ebp & 0xffff
                              : context->c_gpregs.gp_ebp)-4),4);
    context->c_useresp = context->c_gpregs.gp_ebp;
    if (flags & F_AD16) context->c_useresp &= 0xffff;
    context->c_useresp -= 4;
    context->c_gpregs.gp_ebp = *(u32 *)context->c_useresp;
    if (flags & F_OP16) context->c_gpregs.gp_ebp &= 0xffff;
   } else {
    context->c_hostesp = (uintptr_t)(&context->c_host+1);
    if (flags & F_AD16) context->c_hostesp &= 0xffff;
    context->c_hostesp -= 4;
    context->c_gpregs.gp_ebp = *(u32 *)context->c_hostesp;
    if (flags & F_OP16) context->c_gpregs.gp_ebp &= 0xffff;
    /* Manually load the host-portion of the CPU context,
     * because the `iret' used to re-load the updated
     * context if we were to return normally wouldn't
     * do that. */
    cpu_setcontext(&context->c_host);
   }
  } break;

  {
  case 0x6c:
   /* INSB */
   if (is_user) goto privileged_instruction;
   if (!(flags & F_REP)) {
    *(u8 *)context->c_gpregs.gp_edi = inb((u16)context->c_gpregs.gp_edx);
    context->c_gpregs.gp_edi += (context->c_eflags & DF) ? -1 : +1;
   } else while (context->c_gpregs.gp_ecx) {
    if ((flags & F_AD16) && !(context->c_gpregs.gp_ecx & 0xffff)) break;
    *(u8 *)context->c_gpregs.gp_edi = inb((u16)context->c_gpregs.gp_edx);
    context->c_gpregs.gp_edi += (context->c_eflags & DF) ? -1 : +1;
    --context->c_gpregs.gp_ecx;
   }
  } break;

  {
  case 0x6d:
   /* INSW */
   /* INSD */
   if (is_user) goto privileged_instruction;
   if (!(flags & F_REP)) {
    if (flags & F_OP16) {
     *(u16 *)context->c_gpregs.gp_edi = inw((u16)context->c_gpregs.gp_edx);
     context->c_gpregs.gp_edi += (context->c_eflags & DF) ? -2 : +2;
    } else {
     *(u32 *)context->c_gpregs.gp_edi = inl((u16)context->c_gpregs.gp_edx);
     context->c_gpregs.gp_edi += (context->c_eflags & DF) ? -4 : +4;
    }
   } else while (context->c_gpregs.gp_ecx) {
    if ((flags & F_AD16) && !(context->c_gpregs.gp_ecx & 0xffff)) break;
    if (flags & F_OP16) {
     *(u16 *)context->c_gpregs.gp_edi = inw((u16)context->c_gpregs.gp_edx);
     context->c_gpregs.gp_edi += (context->c_eflags & DF) ? -2 : +2;
    } else {
     *(u32 *)context->c_gpregs.gp_edi = inl((u16)context->c_gpregs.gp_edx);
     context->c_gpregs.gp_edi += (context->c_eflags & DF) ? -4 : +4;
    }
    --context->c_gpregs.gp_ecx;
   }
  } break;

  {
  case 0x6e:
   /* OUTSB */
   if (is_user) goto privileged_instruction;
   if (!(flags & F_REP)) {
    outb((u16)context->c_gpregs.gp_edx,
        *(u8 *)context->c_gpregs.gp_esi);
    context->c_gpregs.gp_esi += (context->c_eflags & DF) ? -1 : +1;
   } else while (context->c_gpregs.gp_ecx) {
    if ((flags & F_AD16) && !(context->c_gpregs.gp_ecx & 0xffff)) break;
    outb((u16)context->c_gpregs.gp_edx,
        *(u8 *)context->c_gpregs.gp_esi);
    context->c_gpregs.gp_esi += (context->c_eflags & DF) ? -1 : +1;
    --context->c_gpregs.gp_ecx;
   }
  } break;

  {
  case 0x6f:
   /* OUTSW */
   /* OUTSD */
   if (is_user) goto privileged_instruction;
   if (!(flags & F_REP)) {
    if (flags & F_OP16) {
     outw((u16)context->c_gpregs.gp_edx,
         *(u16 *)context->c_gpregs.gp_esi);
     context->c_gpregs.gp_esi += (context->c_eflags & DF) ? -2 : +2;
    } else {
     outl((u16)context->c_gpregs.gp_edx,
         *(u32 *)context->c_gpregs.gp_esi);
     context->c_gpregs.gp_esi += (context->c_eflags & DF) ? -4 : +4;
    }
   } else while (context->c_gpregs.gp_ecx) {
    if ((flags & F_AD16) && !(context->c_gpregs.gp_ecx & 0xffff)) break;
    if (flags & F_OP16) {
     outw((u16)context->c_gpregs.gp_edx,
         *(u16 *)context->c_gpregs.gp_esi);
     context->c_gpregs.gp_esi += (context->c_eflags & DF) ? -2 : +2;
    } else {
     outl((u16)context->c_gpregs.gp_edx,
         *(u32 *)context->c_gpregs.gp_esi);
     context->c_gpregs.gp_esi += (context->c_eflags & DF) ? -4 : +4;
    }
    --context->c_gpregs.gp_ecx;
   }
  } break;

#ifndef __x86_64__
  {
  case 0x60:
   /* pushal */
   if (!is_user) {
    /* Special case for kernel-space emulation, because IRET wouldn't restore ESP */
    struct cpu_context return_context;
    memcpy(&return_context,
           &context->c_host,
           sizeof(struct cpu_context));
    /* Since we're about to destroy the IRET tail that got us here,
     * we must be sure that we won't get preempted before the
     * `cpu_setcontext()' below will actually restore the caller's context. */
    PREEMPTION_DISABLE();
    return_context.c_gpregs.gp_esp = (uintptr_t)(&context->c_host+1);
    /* Our `struct x86_gpregs' is compatible with pushal! */
    if (flags & F_OP16) {
     ((u16 *)return_context.c_gpregs.gp_esp)[-1] = (u16)return_context.c_gpregs.gp_edi;
     ((u16 *)return_context.c_gpregs.gp_esp)[-2] = (u16)return_context.c_gpregs.gp_esi;
     ((u16 *)return_context.c_gpregs.gp_esp)[-3] = (u16)return_context.c_gpregs.gp_ebp;
     ((u16 *)return_context.c_gpregs.gp_esp)[-4] = (u16)return_context.c_gpregs.gp_esp;
     ((u16 *)return_context.c_gpregs.gp_esp)[-5] = (u16)return_context.c_gpregs.gp_ebx;
     ((u16 *)return_context.c_gpregs.gp_esp)[-6] = (u16)return_context.c_gpregs.gp_edx;
     ((u16 *)return_context.c_gpregs.gp_esp)[-7] = (u16)return_context.c_gpregs.gp_ecx;
     ((u16 *)return_context.c_gpregs.gp_esp)[-8] = (u16)return_context.c_gpregs.gp_eax;
     return_context.c_gpregs.gp_esp -= 2*8;
    } else {
     memcpy((void *)(return_context.c_gpregs.gp_esp-(4*8)),
                    &return_context.c_gpregs,4*8);
     return_context.c_gpregs.gp_esp -= 4*8;
    }
    cpu_setcontext(&return_context);
   }
   if (flags & F_OP16) {
    ((u16 *)context->c_useresp)[-1] = (u16)context->c_gpregs.gp_edi;
    ((u16 *)context->c_useresp)[-2] = (u16)context->c_gpregs.gp_esi;
    ((u16 *)context->c_useresp)[-3] = (u16)context->c_gpregs.gp_ebp;
    ((u16 *)context->c_useresp)[-4] = (u16)context->c_useresp;
    ((u16 *)context->c_useresp)[-5] = (u16)context->c_gpregs.gp_ebx;
    ((u16 *)context->c_useresp)[-6] = (u16)context->c_gpregs.gp_edx;
    ((u16 *)context->c_useresp)[-7] = (u16)context->c_gpregs.gp_ecx;
    ((u16 *)context->c_useresp)[-8] = (u16)context->c_gpregs.gp_eax;
    context->c_useresp -= 2*8;
   } else {
    ((u32 *)context->c_useresp)[-1] = (u32)context->c_gpregs.gp_edi;
    ((u32 *)context->c_useresp)[-2] = (u32)context->c_gpregs.gp_esi;
    ((u32 *)context->c_useresp)[-3] = (u32)context->c_gpregs.gp_ebp;
    ((u32 *)context->c_useresp)[-4] = (u32)context->c_useresp;
    ((u32 *)context->c_useresp)[-5] = (u32)context->c_gpregs.gp_ebx;
    ((u32 *)context->c_useresp)[-6] = (u32)context->c_gpregs.gp_edx;
    ((u32 *)context->c_useresp)[-7] = (u32)context->c_gpregs.gp_ecx;
    ((u32 *)context->c_useresp)[-8] = (u32)context->c_gpregs.gp_eax;
    context->c_useresp -= 4*8;
   }
  } break;

  {
  case 0x61:
   if (!is_user) {
    /* Special case for kernel-space emulation, because IRET wouldn't restore ESP */
    context->c_host.c_esp = (uintptr_t)(&context->c_host+1);
    if (flags & F_OP16) {
     context->c_gpregs.gp_edi  = ((u16 *)context->c_host.c_esp)[0];
     context->c_gpregs.gp_esi  = ((u16 *)context->c_host.c_esp)[1];
     context->c_gpregs.gp_ebp  = ((u16 *)context->c_host.c_esp)[2];
  /* context->c_gpregs.gp_esp  = ((u16 *)context->c_host.c_esp)[3]; */
     context->c_gpregs.gp_ebx  = ((u16 *)context->c_host.c_esp)[4];
     context->c_gpregs.gp_edx  = ((u16 *)context->c_host.c_esp)[5];
     context->c_gpregs.gp_ecx  = ((u16 *)context->c_host.c_esp)[6];
     context->c_gpregs.gp_eax  = ((u16 *)context->c_host.c_esp)[7];
     context->c_gpregs.gp_esp += 2*8;
    } else {
     context->c_gpregs.gp_edi  = ((u32 *)context->c_host.c_esp)[0];
     context->c_gpregs.gp_esi  = ((u32 *)context->c_host.c_esp)[1];
     context->c_gpregs.gp_ebp  = ((u32 *)context->c_host.c_esp)[2];
  /* context->c_gpregs.gp_esp  = ((u32 *)context->c_host.c_esp)[3]; */
     context->c_gpregs.gp_ebx  = ((u32 *)context->c_host.c_esp)[4];
     context->c_gpregs.gp_edx  = ((u32 *)context->c_host.c_esp)[5];
     context->c_gpregs.gp_ecx  = ((u32 *)context->c_host.c_esp)[6];
     context->c_gpregs.gp_eax  = ((u32 *)context->c_host.c_esp)[7];
     context->c_gpregs.gp_esp += 4*8;
    }
    cpu_setcontext(&context->c_host);
   }
   if (flags & F_OP16) {
    context->c_gpregs.gp_edi  = ((u16 *)context->c_useresp)[0];
    context->c_gpregs.gp_esi  = ((u16 *)context->c_useresp)[1];
    context->c_gpregs.gp_ebp  = ((u16 *)context->c_useresp)[2];
 /* context->c_gpregs.gp_esp  = ((u16 *)context->c_useresp)[3]; */
    context->c_gpregs.gp_ebx  = ((u16 *)context->c_useresp)[4];
    context->c_gpregs.gp_edx  = ((u16 *)context->c_useresp)[5];
    context->c_gpregs.gp_ecx  = ((u16 *)context->c_useresp)[6];
    context->c_gpregs.gp_eax  = ((u16 *)context->c_useresp)[7];
    context->c_useresp += 2*8;
   } else {
    context->c_gpregs.gp_edi  = ((u32 *)context->c_useresp)[0];
    context->c_gpregs.gp_esi  = ((u32 *)context->c_useresp)[1];
    context->c_gpregs.gp_ebp  = ((u32 *)context->c_useresp)[2];
 /* context->c_gpregs.gp_esp  = ((u32 *)context->c_useresp)[3]; */
    context->c_gpregs.gp_ebx  = ((u32 *)context->c_useresp)[4];
    context->c_gpregs.gp_edx  = ((u32 *)context->c_useresp)[5];
    context->c_gpregs.gp_ecx  = ((u32 *)context->c_useresp)[6];
    context->c_gpregs.gp_eax  = ((u32 *)context->c_useresp)[7];
    context->c_useresp += 4*8;
   }
  } break;

#endif /* !__x86_64__ */
#endif


  default: goto generic_illegal_instruction;
  }
  /* Update the instruction pointer to point after the emulated instruction. */
  context->c_eip = (uintptr_t)text;
 } CATCH (E_SEGFAULT) {
  COMPILER_READ_BARRIER();
  if ((xcontext->c_eflags & EFLAGS_VM) ||
      (xcontext->c_iret.ir_cs & 3))
       error_info()->e_error.e_segfault.sf_reason |= X86_SEGFAULT_FUSER;
  error_rethrow();
 }
 return; /* Emulated instruction. */
generic_illegal_instruction:
 /* Construct and emit a illegal-instruction exception. */
 info                 = error_info();
 info->e_error.e_code = E_ILLEGAL_INSTRUCTION;
 info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
 memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_illegal_instruction.ii_type = ERROR_ILLEGAL_INSTRUCTION_UNDEFINED;
throw_exception:
 /* Copy the CPU context at the time of the exception. */
 fix_user_context(xcontext);
 memcpy(&info->e_context,&xcontext->c_host,
         sizeof(struct cpu_context));
 /* Throw the error. */
 error_rethrow_atuser((struct cpu_context *)xcontext);
illegal_addressing_mode:
 info                 = error_info();
 info->e_error.e_code = E_ILLEGAL_INSTRUCTION;
 info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
 memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_FOPERAND|
                                                ERROR_ILLEGAL_INSTRUCTION_FADDRESS);
 goto throw_exception;
privileged_instruction:
 info                 = error_info();
 info->e_error.e_code = E_ILLEGAL_INSTRUCTION;
 info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
 memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                ERROR_ILLEGAL_INSTRUCTION_FINSTRUCTION);
 goto throw_exception;
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
 } CATCH_HANDLED (E_SEGFAULT) {
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
