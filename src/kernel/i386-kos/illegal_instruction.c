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
#include <i386-kos/cpuid.h>
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
INTDEF void FCALL fix_user_context(struct x86_anycontext *__restrict context);
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
#if 1 /* Machines on which we need to emulate some lock-instruction
       * usually don't have SSE, which implemented `pause'...
       * XXX: Not true! `pause' is actually `rep nop',
       *      which is a no-op on previous processors! */
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



#ifndef __x86_64__
PRIVATE u32 fence_word = 0;
#define ATOMIC_FENCE() \
 __asm__ __volatile__("lock; xchgl %0, %1" \
                      : "+m" (fence_word) \
                      : "r" (0) \
                      : "memory")
#endif





INTERN NOIRQ void FCALL
x86_handle_illegal_instruction(struct x86_anycontext *__restrict context) {
 struct x86_anycontext *EXCEPT_VAR xcontext = context;
 struct exception_info *info; u16 flags;
 byte_t *EXCEPT_VAR text; u32 opcode;
 X86ModRm modrm; bool is_user;
 text = (byte_t *)context->c_pip;
#ifdef __x86_64__
 is_user = (context->c_iret.ir_cs & 3);
#else
 is_user = ((context->c_eflags & EFLAGS_VM) ||
            (context->c_iret.ir_cs & 3));
#endif
 TRY {
  opcode = X86_ReadOpcode(context,(byte_t **)&text,&flags);
  switch (opcode) {

#ifdef X86_READOPCODE_FAILED
  case X86_READOPCODE_FAILED:
   return;
#endif

#ifndef __x86_64__
   /* Emulate atomic instruction that were only added later */
  case 0x0fb0: {
   uintptr_t addr;
   u8 COMPILER_IGNORE_UNINITIALIZED(real_old_value);
   u8 old_value,new_value;
   /* EMULATOR: cmpxchg r/m8,r8 */
   text = X86_ModRmDecode(text,&modrm,flags);
   addr = X86_ModRmGetMem(context,&modrm,flags);
   if (is_user) validate_writable((void *)addr,1);
   old_value = context->c_gpregs.gp_al;
   new_value = MODRM_REGB;
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
   context->c_gpregs.gp_al = real_old_value;
   context->c_eflags &= (ZF|CF|PF|AF|SF|OF);
   context->c_eflags |= __cmpb(old_value,real_old_value) & (ZF|CF|PF|AF|SF|OF);
  } break;

  {
   uintptr_t addr;
  case 0x0fb1:
   /* cmpxchg r/m16,r16 */
   /* cmpxchg r/m32,r32 */
   text = X86_ModRmDecode(text,&modrm,flags);
   addr = X86_ModRmGetMem(context,&modrm,flags);
   if (flags & F_OP16) {
    u16 COMPILER_IGNORE_UNINITIALIZED(real_old_value);
    u16 old_value,new_value;
    if (is_user) validate_writable((void *)addr,2);
    old_value = context->c_gpregs.gp_ax;
    new_value = MODRM_REGW;
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
    context->c_gpregs.gp_ax = real_old_value;
    context->c_eflags &= (ZF|CF|PF|AF|SF|OF);
    context->c_eflags |= __cmpw(old_value,real_old_value) & (ZF|CF|PF|AF|SF|OF);
   } else {
    u32 COMPILER_IGNORE_UNINITIALIZED(real_old_value);
    u32 old_value,new_value;
    if (is_user) validate_writable((void *)addr,4);
    old_value = context->c_gpregs.gp_eax;
    new_value = MODRM_REGL;
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
    context->c_gpregs.gp_eax = real_old_value;
    context->c_eflags &= (ZF|CF|PF|AF|SF|OF);
    context->c_eflags |= __cmpl(old_value,real_old_value) & (ZF|CF|PF|AF|SF|OF);
   }
  } break;

  case 0x0fc0: {
   uintptr_t addr; u8 add_value;
   u8 COMPILER_IGNORE_UNINITIALIZED(old_value);
   /* xadd r/m8, r8 */
   text = X86_ModRmDecode(text,&modrm,flags);
   addr = X86_ModRmGetMem(context,&modrm,flags);
   if (is_user) validate_writable((void *)addr,1);
   add_value = MODRM_REGB;
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
   MODRM_REGB = old_value;
   context->c_eflags &= ~(CF|PF|AF|SF|ZF|OF);
   context->c_eflags |= __cmpb(old_value,0) & (CF|PF|AF|SF|ZF|OF);
  } break;

  {
   uintptr_t addr;
  case 0x0fc1:
   /* xadd r/m16, r16 */
   /* xadd r/m32, r32 */
   text = X86_ModRmDecode(text,&modrm,flags);
   addr = X86_ModRmGetMem(context,&modrm,flags);
   if (flags & F_OP16) {
    u16 COMPILER_IGNORE_UNINITIALIZED(old_value);
    u16 add_value;
    if (is_user) validate_writable((void *)addr,2);
    add_value = MODRM_REGW;
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
    *&MODRM_REGW = old_value;
    context->c_eflags &= ~(CF|PF|AF|SF|ZF|OF);
    context->c_eflags |= __cmpw(old_value,0) & (CF|PF|AF|SF|ZF|OF);
   } else {
    u32 COMPILER_IGNORE_UNINITIALIZED(old_value);
    u32 add_value;
    if (is_user) validate_writable((void *)addr,4);
    add_value = MODRM_REGL;
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
    MODRM_REGL = old_value;
    context->c_eflags &= ~(CF|PF|AF|SF|ZF|OF);
    context->c_eflags |= __cmpl(old_value,0) & (CF|PF|AF|SF|ZF|OF);
   }
  } break;

  case 0x0fc7:
   text = X86_ModRmDecode(text,&modrm,flags);
   if (modrm.mi_reg == 1) {
    u32 value[2];
    /* cmpxchg8b m64 */
    uintptr_t addr;
    if (modrm.mi_type != MODRM_MEMORY)
        goto illegal_addressing_mode;
    addr = X86_ModRmGetMem(context,&modrm,flags);
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

#endif /* !__x86_64__ */


  case 0x0f01:
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   /* LGDT and LIDT throw an #UD when a register operand is used. */
   if (modrm.mi_reg == 2) goto illegal_addressing_mode; /* LGDT m16&32 */
   if (modrm.mi_reg == 3) goto illegal_addressing_mode; /* LIDT m16&32 */

   if (modrm.mi_reg == 7) {
    /* INVLPG m */
    if (modrm.mi_type != MODRM_MEMORY)
        goto illegal_addressing_mode;
    if (is_user) goto privileged_instruction;
    /* Without INVLPG, we can only flush the entire page directory. */
    pagedir_syncall();
    break;
   }

   goto generic_illegal_instruction;


#ifndef __x86_64__
  {
   syscall_ulong_t EXCEPT_VAR sysno;
   syscall_ulong_t EXCEPT_VAR orig_eax;
   syscall_ulong_t EXCEPT_VAR orig_eip;
   syscall_ulong_t EXCEPT_VAR orig_ebp;
   syscall_ulong_t EXCEPT_VAR orig_edi;
   syscall_ulong_t EXCEPT_VAR orig_esp;
  case 0x0f34:
   /* SYSENTER */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
#if 0
   if unlikely(!is_user) goto generic_illegal_instruction;
#endif
   orig_eax = context->c_gpregs.gp_eax;
   orig_eip = context->c_pip;
   orig_ebp = context->c_gpregs.gp_ebp;
   orig_edi = context->c_gpregs.gp_edi;
   orig_esp = context->c_iret.ir_useresp;
restart_sysenter_syscall:
   TRY {
    /* Convert the user-space register context to become `int $0x80'-compatible */
    u8 argc = 6;
    sysno = xcontext->c_gpregs.gp_eax;
    xcontext->c_pip             = orig_edi; /* CLEANUP: return.%eip = %edi */
    xcontext->c_iret.ir_useresp = orig_ebp; /* CLEANUP: return.%esp = %ebp */
    /* Figure out how many arguments this syscall takes. */
    if (sysno <= __NR_syscall_max)
     argc = x86_syscall_argc[sysno];
    else if (sysno >= __NR_xsyscall_min &&
             sysno <= __NR_xsyscall_max) {
     argc = x86_xsyscall_argc[sysno-__NR_xsyscall_min];
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
     xcontext->c_pip             = orig_eip;
     xcontext->c_gpregs.gp_ebp   = orig_ebp;
     xcontext->c_gpregs.gp_edi   = orig_edi;
     xcontext->c_iret.ir_useresp = orig_esp;
     COMPILER_WRITE_BARRIER();
     /* Deal with system call restarts. */
     task_restart_syscall((struct cpu_hostcontext_user *)xcontext,
                           TASK_USERCTX_TYPE_WITHINUSERCODE|
                           TASK_USERCTX_REGS_FPF,
                           sysno);
     COMPILER_BARRIER();
     goto restart_sysenter_syscall;
    }
    error_rethrow();
   }
  } break;

  case 0x0f35:
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   /* SYSEXIT */
   if (is_user) goto privileged_instruction;
   context->c_iret.ir_cs      = X86_USER_CS;
   context->c_iret.ir_ss      = X86_USER_DS;
   context->c_iret.ir_useresp = context->c_gpregs.gp_ecx;
   context->c_iret.ir_eip     = context->c_gpregs.gp_edx;
   break;
#endif /* !__x86_64__ */



#ifndef __x86_64__ /* BOUND doesn't exist in x86_64 */
  {
   uintptr_t bounds_struct;
   u32 low,high,index;
  case 0x62:
   /* BOUND r16, m16&16     (Added with 80186/80188) */
   /* BOUND r32, m32&32     (Added with 80186/80188) */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   bounds_struct = X86_ModRmGetMem(context,&modrm,flags);
   if (flags & F_OP16) {
    low   = ((u16 *)bounds_struct)[0];
    high  = ((u16 *)bounds_struct)[1];
    index = MODRM_REGW;
   } else {
    low  = ((u32 *)bounds_struct)[0];
    high = ((u32 *)bounds_struct)[1];
    index = MODRM_REGL;
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
#endif /* !__x86_64__ */


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
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
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
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
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
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
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
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
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
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
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
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
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
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
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
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
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

#ifndef __x86_64__ /* On x86_64, we can assume that this one exists! */
  {
   u32 value; register u16 temp;
  case 0x0fc8 ... 0x0fcf:
   /* BSWAP r32  -- Added with 80486 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   value = X86_GPREG(context,opcode - 0x0fc8);
   /* Use inline assembly so GCC doesn't optimize by
    * using the instruction we're trying to emulate. */
   __asm__ __volatile__("movw  0+%0, %1\n\t"  /* x = lo; */
                        "xchgb %b1, %h1\n\t"  /* x = x << 8 | x >> 8; */
                        "xchgw 2+%0, %1\n\t"  /* temp = hi,hi = x,x = temp; */
                        "xchgb %b1, %h1\n\t"  /* x = x << 8 | x >> 8; */
                        "movw  %1, 0+%0"      /* lo = x; */
                        : "+m" (value)
                        , "=q" (temp));
   X86_GPREG(context,opcode - 0x0fc8) = value;
  } break;
#endif /* !__x86_64__ */

#ifndef __x86_64__ /* These were added by the 486. - Long before the time of x86_64 */
  case 0x0f08: /* INVD */
  case 0x0f09: /* WBINVD */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   if (is_user) goto privileged_instruction;
   pagedir_syncall(); /* What other instruction caches are there? */
   break;
#endif

#ifndef __x86_64__ /* On x86_64, we can assume that there is SSE */
  case 0x0f1f:
   /* NOP r/m16  -- Added with SSE */
   /* NOP r/m32  -- Added with SSE */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   break;
#endif

#ifndef __x86_64__ /* On x86_64, we can assume that the cmov instructions exist */
  case 0x0f40:
   /* CMOVO r16, r/m16 */
   /* CMOVO r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (context->c_eflags & OF) {
do_cmov:
    if (flags & F_OP16)
         MODRM_REGL = MODRM_MEMW;
    else MODRM_REGL = MODRM_MEML;
   }
   break;
  case 0x0f41:
   /* CMOVNO r16, r/m16 */
   /* CMOVNO r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (!(context->c_eflags & OF))
       goto do_cmov;
   break;
  case 0x0f42:
   /* CMOVB r16, r/m16 */
   /* CMOVB r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (context->c_eflags & CF)
       goto do_cmov;
   break;
  case 0x0f43:
   /* CMOVAE r16, r/m16 */
   /* CMOVAE r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (!(context->c_eflags & CF))
       goto do_cmov;
   break;
  case 0x0f44:
   /* CMOVE r16, r/m16 */
   /* CMOVE r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (context->c_eflags & ZF)
       goto do_cmov;
   break;
  case 0x0f45:
   /* CMOVNE r16, r/m16 */
   /* CMOVNE r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (!(context->c_eflags & ZF))
       goto do_cmov;
   break;
  case 0x0f46:
   /* CMOVBE r16, r/m16 */
   /* CMOVBE r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if ((context->c_eflags & (CF|ZF)) == (CF|ZF))
       goto do_cmov;
   break;
  case 0x0f47:
   /* CMOVA r16, r/m16 */
   /* CMOVA r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (!(context->c_eflags & (CF|ZF)))
       goto do_cmov;
   break;
  case 0x0f48:
   /* CMOVS r16, r/m16 */
   /* CMOVS r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (context->c_eflags & SF)
       goto do_cmov;
   break;
  case 0x0f49:
   /* CMOVNS r16, r/m16 */
   /* CMOVNS r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (!(context->c_eflags & SF))
       goto do_cmov;
   break;
  case 0x0f4a:
   /* CMOVP r16, r/m16 */
   /* CMOVP r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (context->c_eflags & PF)
       goto do_cmov;
   break;
  case 0x0f4b:
   /* CMOVNP r16, r/m16 */
   /* CMOVNP r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (!(context->c_eflags & PF))
       goto do_cmov;
   break;
  case 0x0f4c:
   /* CMOVL r16, r/m16 */
   /* CMOVL r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (!!(context->c_eflags & SF) != !!(context->c_eflags & OF))
       goto do_cmov;
   break;
  case 0x0f4d:
   /* CMOVGE r16, r/m16 */
   /* CMOVGE r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (!!(context->c_eflags & SF) == !!(context->c_eflags & OF))
       goto do_cmov;
   break;
  case 0x0f4e:
   /* CMOVLE r16, r/m16 */
   /* CMOVLE r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if ((context->c_eflags & ZF) ||
     !!(context->c_eflags & SF) != !!(context->c_eflags & OF))
       goto do_cmov;
   break;
  case 0x0f4f:
   /* CMOVG r16, r/m16 */
   /* CMOVG r32, r/m32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (!(context->c_eflags & ZF) &&
      !!(context->c_eflags & SF) == !!(context->c_eflags & OF))
       goto do_cmov;
   break;
#endif /* !__x86_64__ */

#ifndef __x86_64__ /* On x86_64, we can assume that cpuid exists */
  {
   struct cpu_cpuid const *features;
  case 0x0fa2:
   /* CPUID */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   features = &CPU_FEATURES;
   switch (context->c_gpregs.gp_eax) {

   case 0:
    context->c_gpregs.gp_eax = features->ci_0a;
    context->c_gpregs.gp_ecx = features->ci_0c;
    context->c_gpregs.gp_edx = features->ci_0d;
    context->c_gpregs.gp_ebx = features->ci_0b;
    break;

   case 1:
    context->c_gpregs.gp_eax = features->ci_1a;
    context->c_gpregs.gp_ecx = features->ci_1c;
    context->c_gpregs.gp_edx = features->ci_1d;
    context->c_gpregs.gp_ebx = features->ci_1b;
    break;

   case 2: /* Cache information... (ZERO indicates unavailable information) */
   case 3: /* Processor serial number (guess that's zero for our's) */
   case 4:
    context->c_gpregs.gp_eax = 0;
    context->c_gpregs.gp_ecx = 0;
    context->c_gpregs.gp_edx = 0;
    context->c_gpregs.gp_ebx = 0;
    break;

   case 7:
    context->c_gpregs.gp_ecx = features->ci_7c;
    context->c_gpregs.gp_edx = features->ci_7d;
    context->c_gpregs.gp_ebx = features->ci_7b;
    break;

   case 0x80000000:
    context->c_gpregs.gp_eax = features->ci_80000000a;
    break;

   case 0x80000001:
    context->c_gpregs.gp_ecx = features->ci_80000001c;
    context->c_gpregs.gp_edx = features->ci_80000001d;
    break;

   case 0x80000002:
    context->c_gpregs.gp_eax = features->ci_80000002a;
    context->c_gpregs.gp_ecx = features->ci_80000002c;
    context->c_gpregs.gp_edx = features->ci_80000002d;
    context->c_gpregs.gp_ebx = features->ci_80000002b;
    break;

   case 0x80000003:
    context->c_gpregs.gp_eax = features->ci_80000003a;
    context->c_gpregs.gp_ecx = features->ci_80000003c;
    context->c_gpregs.gp_edx = features->ci_80000003d;
    context->c_gpregs.gp_ebx = features->ci_80000003b;
    break;

   case 0x80000004:
    context->c_gpregs.gp_eax = features->ci_80000004a;
    context->c_gpregs.gp_ecx = features->ci_80000004c;
    context->c_gpregs.gp_edx = features->ci_80000004d;
    context->c_gpregs.gp_ebx = features->ci_80000004b;
    break;

#if 1
   case 0x8FFFFFFF:
    context->c_gpregs.gp_eax = 'I' | 'T' << 8 | '\'' << 16 | 'S' << 24;
    context->c_gpregs.gp_ebx = ' ' | 'H' << 8 | 'A' << 16 | 'M' << 24;
    context->c_gpregs.gp_ecx = 'M' | 'E' << 8 | 'R' << 16 | ' ' << 24;
    context->c_gpregs.gp_edx = 'T' | 'I' << 8 | 'M' << 16 | 'E' << 24;
    break;
#endif

   default:
    break;
   }
  } break;
#endif /* !__x86_64__ */

  case 0x0f32: /* RDMSR */
#ifndef __x86_64__
   if (context->c_gpregs.gp_ecx == IA32_TIME_STAMP_COUNTER)
       goto do_rdtsc;
#endif
   ATTR_FALLTHROUGH
  case 0x0f30: /* WRMSR */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   if (is_user) goto privileged_instruction;
   info = error_info();
   info->e_error.e_code = E_ILLEGAL_INSTRUCTION;
   info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
   memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
   info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_UNDEFINED|
                                                  ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
   info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_MSR;
   info->e_error.e_illegal_instruction.ii_register_number = context->c_gpregs.gp_ecx;
   goto throw_exception;

#ifndef __x86_64__ /* Added by PENTIUM. - Long before x86_64 */
  case 0x0f31:
do_rdtsc:
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   /* RDTSC */
   /* TODO: This can be emulated by combining jiffies with the remainder
    *       of the current quantum, which can be read from the LAPIC
    *       CURRENT register, as well as the length of a quantum.
    *       The PIT also has a way of reading out the remaining number
    *       of ~ticks~ until the quantum ends. */
   goto generic_illegal_instruction;
   break;
#endif

#ifndef __x86_64__ /* Added by SSE. - Which is mandatory for x86_64 */
  case 0x0fae:
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   if (*text == 0xf8 || /* sfence */
       *text == 0xe8 || /* lfence */
       *text == 0xf0) { /* mfence */
    /* Serialize memory by executing an atomic instruction. */
    ATOMIC_FENCE();
    break;
   }
   text = X86_ModRmDecode(text,&modrm,flags);
   if (modrm.mi_reg == 7) {
    if (modrm.mi_type != MODRM_MEMORY)
        goto illegal_addressing_mode;
    (void)MODRM_MEMB;
    ATOMIC_FENCE();
    break;
   }
   break;
#endif


  case 0x0fc3:
   /* MOVNTI m32, r32 */
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (modrm.mi_type != MODRM_MEMORY)
       goto illegal_addressing_mode;
#ifdef __x86_64__
   if (flags & F_REX_W) {
    MODRM_MEMQ = MODRM_REGQ;
   } else
#endif
   {
    MODRM_MEML = MODRM_REGL;
   }
   break;


  case 0x0f18:
   if (flags & F_LOCK)
       goto generic_illegal_instruction;
   text = X86_ModRmDecode(text,&modrm,flags);
   if (modrm.mi_type != MODRM_MEMORY)
       goto illegal_addressing_mode;
   if (modrm.mi_reg > 3)
       goto generic_illegal_instruction;
   /* PREFETCHT0 m8 */
   /* PREFETCHT1 m8 */
   /* PREFETCHT2 m8 */
   /* PREFETCHNTA m8 */
   break;


  default: goto generic_illegal_instruction;
  }
  /* Update the instruction pointer to point after the emulated instruction. */
  context->c_pip = (uintptr_t)text;
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
 memcpy(&info->e_context,xcontext,
         sizeof(struct cpu_context));
 /* Throw the error. */
 error_rethrow_atuser((struct cpu_context *)xcontext);
 return;
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
 X86ModRm modrm; u16 EXCEPT_VAR flags;
 u16 EXCEPT_VAR effective_segment_value = 0;
 info = error_info();

#ifdef CONFIG_VM86
 /* Deal with GPF instruction for emulation in vm86 mode. */
 if (context->c_eflags & EFLAGS_VM) {
  if (vm86_gpf((struct cpu_context_vm86 *)context,errcode))
      return;
  /* Clear exception pointers. */
  memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
  info->e_error.e_code = E_ILLEGAL_INSTRUCTION;
  info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
  info->e_error.e_illegal_instruction.ii_errcode = errcode;
  goto e_privileged_instruction;
 }
#endif

 /* Clear exception pointers. */
 memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_code = E_ILLEGAL_INSTRUCTION;
 info->e_error.e_flag = ERR_FRESUMABLE|ERR_FRESUMENEXT;
 info->e_error.e_illegal_instruction.ii_errcode = errcode;

 /* Analyze the source assembly where the GPF happened. */
 TRY {
  byte_t *text; u32 opcode;
  text = (byte_t *)context->c_pip;
#ifdef X86_PAGING_ISNONCANON
  if (X86_PAGING_ISNONCANON((uintptr_t)text)) {
   /* We somehow ended up with a non-canonical instruction pointer.
    * Deal with this as an E_SEGFAULT exception, likely caused by
    * something like this:
    * >> (*(void(*)(int))0x123456789abcdef0ull)(42);
    * In this case, IP is already broken to point to an invalid address.
    * However, we'd rather do exception handling at the call-site, meaning we'll
    * have to dereference the stack to try and get the proper return address.
    * Though because of how low-level all of this is, we have no guaranty that
    * memory at the call-site will actually be mapped, or that this really is
    * we ended up in a situation where the faulting address matches the return-ip.
    * For that reason, a number of additional checks are done to work out if
    * it was code like that above which caused the problem. */
   X86_SEGFAULT_FEXEC;
   TRY {
    uintptr_t return_ip;
    if (context->c_cs & 3)
        validate_readable((void *)xcontext->c_psp,8);
    return_ip = *(uintptr_t *)context->c_psp;
    /* Check if the supposed return-ip is mapped.
     * XXX: Maybe even check if it is executable? */
    if (pagedir_ismapped(VM_ADDR2PAGE(return_ip))) {
     context->c_psp += 8; /* Consume the addressed pushed by `call' */
     context->c_pip = return_ip;
    }
   } CATCH_HANDLED (E_SEGFAULT) {
   }
   /* Construct and emit a SEGFAULT exception. */
   info->e_error.e_code = E_SEGFAULT;
   info->e_error.e_flag = ERR_FRESUMABLE;
   info->e_error.e_segfault.sf_reason = X86_SEGFAULT_FEXEC;
   if (context->c_cs & 3)
       info->e_error.e_segfault.sf_reason |= X86_SEGFAULT_FUSER;
   info->e_error.e_segfault.sf_vaddr = (void *)(uintptr_t)text;
   goto do_throw_error;
  }
#endif

  opcode = X86_ReadOpcode(context,&text,(u16 *)&flags);

  /* TODO: #GPF must also handle the following:
   *   - Attempting to execute an SSE instruction specifying an unaligned memory operand.
   * Taken from table 8-6 (https://support.amd.com/TechDocs/24593.pdf)
   */


#ifdef __x86_64__
  /* Check for #GPFs caused by non-canonical memory accesses. */
  if (xerrcode == 0) {
   uintptr_t nc_addr;
   byte_t *old_text = text;
   switch (opcode) {

   case 0x88:    /* MOV r/m8,r8 */
   case 0x89:    /* MOV r/m16,r16; MOV r/m32,r32; MOV r/m64,r64 */
   case 0x8c:    /* MOV r/m16,Sreg** */
   case 0xc6:    /* MOV r/m8, imm8 */
   case 0xc7:    /* MOV r/m16, imm16; MOV r/m32, imm32; MOV r/m64, imm32 */
   case 0x80:    /* ADC r/m8, imm8 */
   case 0x81:    /* ADC r/m16, imm16; ADC r/m32, imm32; ADC r/m64, imm32 */
   case 0x83:    /* ADC r/m16, imm8; ADC r/m32, imm8; ADC r/m64, imm8 */
   case 0x0f97:  /* seta r/m8 */
   case 0x0f93:  /* setae r/m8 */
   case 0x0f92:  /* setb r/m8 */
   case 0x0f96:  /* setbe r/m8 */
   case 0x0f94:  /* sete r/m8 */
   case 0x0f9f:  /* setg r/m8 */
   case 0x0f9d:  /* setge r/m8 */
   case 0x0f9c:  /* setl r/m8 */
   case 0x0f9e:  /* setle r/m8 */
   case 0x0f95:  /* setne r/m8 */
   case 0x0f91:  /* setno r/m8 */
   case 0x0f9b:  /* setnp r/m8 */
   case 0x0f99:  /* setns r/m8 */
   case 0x0f9a:  /* setp r/m8 */
   case 0x0f98:  /* sets r/m8 */
   case 0x0f47:  /* cmova r/m16, r16 */
   case 0x0f43:  /* cmovae r/m16, r16 */
   case 0x0f42:  /* cmovb r/m16, r16 */
   case 0x0f46:  /* cmovbe r/m16, r16 */
   case 0x0f44:  /* cmove r/m16, r16 */
   case 0x0f4f:  /* cmovg r/m16, r16 */
   case 0x0f4d:  /* cmovge r/m16, r16 */
   case 0x0f4c:  /* cmovl r/m16, r16 */
   case 0x0f4e:  /* cmovle r/m16, r16 */
   case 0x0f45:  /* cmovne r/m16, r16 */
   case 0x0f41:  /* cmovno r/m16, r16 */
   case 0x0f4b:  /* cmovnp r/m16, r16 */
   case 0x0f49:  /* cmovns r/m16, r16 */
   case 0x0f40:  /* cmovo r/m16, r16 */
   case 0x0f4a:  /* cmovp r/m16, r16 */
   case 0x0f48:  /* cmovs r/m16, r16 */
   case 0x0fc3:  /* movnti r32, m32 */
   case 0x0f29:  /* movapd xmm2/m128, xmm1 / movaps xmm2/m128, xmm1 */
   case 0x0f6e:  /* movd r/m32, mm / movd r/m32, xmm */
   case 0x0f12:  /* movddup xmm2/m64, xmm1 / movhlps xmm2, xmm1
                  * movlpd m64, xmm / movlps m64, xmm / movsldup xmm2/m128, xmm1 */
   case 0x0f6f:  /* movdqa xmm2/m128, xmm1 / movdqu xmm2/m128, xmm1
                  * movq mm/m64, mm */
    text = X86_ModRmDecode(text,&modrm,flags);
    nc_addr = MODRM_MEM;
    goto noncanon_write;

   case 0x8a:    /* MOV r8,r/m8 */
   case 0x8b:    /* MOV r16,r/m16; MOV r32,r/m32; MOV r64,r/m64 */
   case 0x8e:    /* MOV Sreg,r/m16** */
   case 0x0fbe:  /* movsx r/m8, r16 / movsx r/m8, r32 */
   case 0x0fbf:  /* movsx r/m16, r32 */
   case 0x0fb6:  /* movzx r/m8, r16 / movzx r/m8, r32 */
   case 0x0fb7:  /* movzx r/m16, r32 */
    /* All of the following are technically read+write, but since the read comes first... */
   case 0x12:    /* adc r/m8, r8 */
   case 0x13:    /* adc r/m16, r16 / adc r/m32, r32 */
   case 0x02:    /* add r/m8, r8 */
   case 0x03:    /* add r/m16, r16 / add r/m32, r32 */
   case 0x22:    /* and r/m8, r8 */
   case 0x23:    /* and r/m16, r16 / and r/m32, r32 */
   case 0x6b:    /* imul $imm8, r/m16, %r16 / imul $imm8, r/m32, %r32 */
   case 0x69:    /* imul $imm16, r/m16, %r16 / imul $imm32, r/m32, %r32 */
   case 0x10:    /* adc r8, r/m8 */
   case 0x11:    /* adc r16, r/m16 / adc r32, r/m32 */
   case 0x00:    /* add r8, r/m8 */
   case 0x01:    /* add r16, r/m16 / add r32, r/m32 */
   case 0x20:    /* and r8, r/m8 */
   case 0x21:    /* and r16, r/m16 / and r32, r/m32 */
   case 0xc0:    /* rcl $imm8, r/m8 / rcr $imm8, r/m8
                  * rol $imm8, r/m8 / ror $imm8, r/m8
                  * sal $imm8, r/m8 / sar $imm8, r/m8
                  * shl $imm8, r/m8 / shr $imm8, r/m8 */
   case 0xc1:    /* rcl $imm8, r/m16 / rcl $imm8, r/m32 / rcr $imm8, r/m16 / rcr $imm8, r/m32
                  * rol $imm8, r/m16 / rol $imm8, r/m32 / ror $imm8, r/m16 / ror $imm8, r/m32
                  * sal $imm8, r/m16 / sal $imm8, r/m32 / sar $imm8, r/m16 / sar $imm8, r/m32
                  * shl $imm8, r/m16 / shl $imm8, r/m32 / shr $imm8, r/m16 / shr $imm8, r/m32 */
   case 0xf6:    /* div r/m8 / idiv r/m8 / mul r/m8 / imul r/m8 / neg r/m8 / not r/m8 */
   case 0xf7:    /* div r/m16 / div r/m32 / idiv r/m16 / idiv r/m32
                  * mul r/m16 / mul r/m32 / imul r/m16 / imul r/m32
                  * neg r/m16 / neg r/m32 / not r/m16 / not r/m32 */
   case 0xff:    /* call *r/m16 / call *r/m32 / dec r/m16 / dec r/m32 / inc r/m16 / inc r/m32
                  * jmp *r/m16 / jmp *r/m32 / ljmp m16 / ljmp m32 */
   case 0x38:    /* cmp r8, r/m8 */
   case 0x39:    /* cmp r16, r/m16 / cmp r32, r/m32 */
   case 0x3a:    /* cmp r/m8, r8 */
   case 0x3b:    /* cmp r/m16, r16 / cmp r/m32, r32 */
   case 0xfe:    /* dec r/m8 / inc r/m8 */
    /* Floating point (st(i)-operations) prefix bytes */
   case 0xd8: case 0xd9: case 0xda: case 0xdb:
   case 0xdc: case 0xdd: case 0xde: case 0xdf:
   case 0xc5:    /* lds m16, r16 / lds m32, r32 */
   case 0xc4:    /* les m16, r16 / les m32, r32 */
   case 0x8d:    /* lea m, r16 / lea m, r32 */
   case 0x08:    /* or r8, r/m8 */
   case 0x09:    /* or r16, r/m16 / or r32, r/m32 */
   case 0x0a:    /* or r/m8, r8 */
   case 0x0b:    /* or r/m16, r16 / or r/m32, r32 */
   case 0x8f:    /* pop r/m16 / pop r/m32 */
   case 0xd0:    /* rcl r/m8 / rcr r/m8
                  * rol r/m8 / ror r/m8
                  * sal r/m8 / sar r/m8
                  * shl r/m8 / shr r/m8 */
   case 0xd1:    /* rcl r/m16 / rcl r/m32 / rcr r/m16 / rcr r/m32
                  * rol r/m16 / rol r/m32 / ror r/m16 / ror r/m32
                  * sal r/m16 / sal r/m32 / sar r/m16 / sar r/m32
                  * shl r/m16 / shl r/m32 / shr r/m16 / shr r/m32 */
   case 0xd2:    /* rcl %cl, r/m8 / rcr %cl, r/m8
                  * rol %cl, r/m8 / ror %cl, r/m8
                  * sal %cl, r/m8 / sar %cl, r/m8
                  * shl %cl, r/m8 / shr %cl, r/m8 */
   case 0xd3:    /* rcl %cl, r/m16 / rcl %cl, r/m32 / rcr %cl, r/m16 / rcr %cl, r/m32
                  * rol %cl, r/m16 / rol %cl, r/m32 / ror %cl, r/m16 / ror %cl, r/m32
                  * sal %cl, r/m16 / sal %cl, r/m32 / sar %cl, r/m16 / sar %cl, r/m32
                  * shl %cl, r/m16 / shl %cl, r/m32 / shr %cl, r/m16 / shr %cl, r/m32 */
   case 0x18:    /* sbb r8, r/m8 */
   case 0x19:    /* sbb r16, r/m16 / sbb r32, r/m32 */
   case 0x1a:    /* sbb r/m8, r8 */
   case 0x1b:    /* sbb r/m16, r16 / sbb r/m32, r32 */
   case 0x28:    /* sub r8, r/m8 */
   case 0x29:    /* sub r16, r/m16 / sub r32, r/m32 */
   case 0x2a:    /* sub r/m8, r8 */
   case 0x2b:    /* sub r/m16, r16 / sub r/m32, r32 */
   case 0x84:    /* test r8, r/m8 */
   case 0x85:    /* test r16, r/m16 / test r32, r/m32 */
   case 0x86:    /* xchg %r8, r/m8 / xchg r/m8, %r8 */
   case 0x87:    /* xchg %r16, r/m16 / xchg r/m16, %r16
                  * xchg %r32, r/m32 / xchg r/m32, %r32 */
   case 0x30:    /* xor r8, r/m8 */
   case 0x31:    /* xor r16, r/m16 / xor r32, r/m32 */
   case 0x32:    /* xor r/m8, r8 */
   case 0x33:    /* xor r/m16, r16 / xor r/m32, r32 */
   case 0x0f58:  /* addpd xmm2/m128, xmm1 */
   case 0x0fd0:  /* addsubpd xmm2/m128, xmm1 */
   case 0x0f54:  /* andpd xmm2/m128, xmm1 */
   case 0x0f55:  /* andnpd xmm2/m128, xmm1 */
   case 0x0fbc:  /* bsf r/m16, r16 */
   case 0x0fbd:  /* bsr r/m16, r16 */
   case 0x0fa3:  /* bt r16, r/m16 / bt r32, r/m32 */
   case 0x0fbb:  /* btc r16, r/m16 / btc r32, r/m32 */
   case 0x0fb3:  /* btr r16, r/m16 / btr r32, r/m32 */
   case 0x0fab:  /* bts r16, r/m16 / bts r32, r/m32 */
   case 0x0fb0:  /* cmpxch r8, r/m8 */
   case 0x0fb1:  /* cmpxch r16, r/m16 / cmpxch r32, r/m32 */
   case 0x0fc7:  /* cmpxchg8b m64 */
   case 0x0f2f:  /* comisd xmm2/m64, xmm1 */
   case 0x0fe6:  /* cvtdq2pd xmm2/m64, xmm1 */
   case 0x0f5b:  /* cvtdq2ps xmm2/m128, xmm1 */
   case 0x0f5a:  /* cvtpd2ps xmm2/m128, xmm1 / cvtsd2ss xmm2/m64, xmm1 */
   case 0x0f2a:  /* cvtpi2pd mm/m64, xmm */
   case 0x0f2d:  /* cvtps2pi xmm/m64, mm */
   case 0x0f2c:  /* cvttpd2pi xmm/m128, mm */
   case 0x0f5e:  /* divpd xmm2/m128, xmm1 */
   case 0x0f7c:  /* haddpd xmm2/m128, xmm1 / haddps xmm2/m128, xmm1 */
   case 0x0f7d:  /* hsubpd xmm2/m128, xmm1 / hsubps xmm2/m128, xmm1 */
   case 0x0faf:  /* imul r/m16, %r16 / imul r/m32, %r32 */
   case 0x0f02:  /* lar r/m16, r16 / lar r/m32, r32 */
   case 0x0ff0:  /* lddqu m128, xmm */
   case 0x0fb2:  /* lss m16, r16 / lss m32, r32 */
   case 0x0fb4:  /* lfs m16, r16 / lfs m32, r32 */
   case 0x0fb5:  /* lgs m16, r16 / lgs m32, r32 */
   case 0x0f00:  /* lldt r/m16 / ltr r/m16 / sldt r/m16 / str r/m16
                  * verr r/m16 / verw r/m16 */
   case 0x0f03:  /* lsl r/m16, %r16 / lsl r/m32, %r32 */
   case 0x0ff7:  /* maskmovdqu xmm2, xmm1 / maskmovq mm2, mm1 */
   case 0x0f5f:  /* maxpd xmm2/m128, xmm1 / maxps xmm2/m128, xmm1
                  * maxpd xmm2/m64, xmm1 / maxps xmm2/m64, xmm1 */
   case 0x0f5d:  /* minpd xmm2/m128, xmm1 / minps xmm2/m128, xmm1
                  * minpd xmm2/m64, xmm1 / minps xmm2/m64, xmm1 */
   case 0x0f28:  /* movapd xmm1, xmm2/m128 / movaps xmm1, xmm2/m128 */
   case 0x0f7e:  /* movd mm, r/m32  / movd xmm, r/m32
                  * movq xmm/m64, xmm */
   case 0x0f13:  /* movlpd xmm, m64 / movlps xmm, m64 */
   case 0x0f7f:  /* movdqa xmm1, xmm2/m128 / movdqu xmm1, xmm2/m128
                  * movq mm, mm/m64 */
   case 0x0fd6:  /* movdq2q xmm, mm / movq xmm, xmm/m64 / movq2dq mm, xmm */
   case 0x0f16:  /* movhpd m64, xmm / movhps m64, xmm / movlhps xmm2, xmm1 / movshdup xmm2/m128, xmm1 */
   case 0x0f17:  /* movhpd xmm, m64 / movhps xmm, m64 */
   case 0x0f50:  /* movmskpd xmm, r32 / movmskps xmm, r32 */
   case 0x0fe7:  /* movntdq xmm, m128 / movntq mm, m64 */
   case 0x0f2b:  /* movntpd xmm, m128 / movntps xmm, m128 */
   case 0x0f10:  /* movsd xmm2/m64, xmm1 / movss xmm2/m32, xmm1
                  * movapd xmm2/m128, xmm1 / movups xmm2/m128, xmm1 */
   case 0x0f11:  /* movsd xmm1, xmm2/m64 / movss xmm1, xmm2/m32
                  * movapd xmm1, xmm2/m128 / movups xmm1, xmm2/m128 */
   case 0x0f59:  /* mulpd xmm2/m128, xmm1 / mulps xmm2/m128, xmm1
                  * mulsd xmm2/m64, xmm1 / mulss xmm2/m64, xmm1 */
   case 0x0f56:  /* orpd xmm2/m128, xmm1 / orps xmm2/m128, xmm1 */
   case 0x0f63:  /* packsswb mm2/m64, mm1 / packsswb xmm2/m128, xmm1 */
   case 0x0f6b:  /* packssdw mm2/m64, mm1 / packssdw xmm2/m128, xmm1 */
   case 0x0f67:  /* packuswb mm2/m64, mm1 / packuswb xmm2/m128, xmm1 */
   case 0x0ffc:  /* paddb mm2/m64, mm1 / paddb xmm2/m128, xmm1 */
   case 0x0ffd:  /* paddw mm2/m64, mm1 / paddw xmm2/m128, xmm1 */
   case 0x0ffe:  /* paddl mm2/m64, mm1 / paddl xmm2/m128, xmm1 */
   case 0x0fd4:  /* paddq mm2/m64, mm1 / paddq xmm2/m128, xmm1 */
   case 0x0fec:  /* paddsb mm2/m64, mm1 / paddsb xmm2/m128, xmm1 */
   case 0x0fed:  /* paddsw mm2/m64, mm1 / paddsw xmm2/m128, xmm1 */
   case 0x0fdc:  /* paddusb mm2/m64, mm1 / paddusb xmm2/m128, xmm1 */
   case 0x0fdd:  /* paddusw mm2/m64, mm1 / paddusw xmm2/m128, xmm1 */
   case 0x0fdb:  /* pand mm2/m64, mm1 / pand xmm2/m128, xmm1 */
   case 0x0fdf:  /* pandn mm2/m64, mm1 / pandn xmm2/m128, xmm1 */
   case 0x0fe0:  /* pavgbb mm2/m64, mm1 / pavgbb xmm2/m128, xmm1 */
   case 0x0fe3:  /* pavgbw mm2/m64, mm1 / pavgbw xmm2/m128, xmm1 */
   case 0x0f74:  /* pcmpeqb mm2/m64, mm1 / pcmpeqb xmm2/m128, xmm1 */
   case 0x0f75:  /* pcmpeqw mm2/m64, mm1 / pcmpeqw xmm2/m128, xmm1 */
   case 0x0f76:  /* pcmpeql mm2/m64, mm1 / pcmpeql xmm2/m128, xmm1 */
   case 0x0f64:  /* pcmpgtb mm2/m64, mm1 / pcmpgtb xmm2/m128, xmm1 */
   case 0x0f65:  /* pcmpgtw mm2/m64, mm1 / pcmpgtw xmm2/m128, xmm1 */
   case 0x0f66:  /* pcmpgtl mm2/m64, mm1 / pcmpgtl xmm2/m128, xmm1 */
   case 0x0ff5:  /* pmaddwd mm2/m64, mm1 / pmaddwd xmm2/m128, xmm1 */
   case 0x0fee:  /* pmaxsw mm2/m64, mm1 / pmaxsw xmm2/m128, xmm1 */
   case 0x0fde:  /* pmaxub mm2/m64, mm1 / pmaxub xmm2/m128, xmm1 */
   case 0x0fea:  /* pminsw mm2/m64, mm1 / pminsw xmm2/m128, xmm1 */
   case 0x0fda:  /* pminub mm2/m64, mm1 / pminub xmm2/m128, xmm1 */
   case 0x0fd7:  /* pmovmskb r32, mm1 / pmovmskb r32, xmm1 */
   case 0x0fe4:  /* pmulhuw mm2/m64, mm1 / pmulhuw xmm2/m128, xmm1 */
   case 0x0fe5:  /* pmulhw mm2/m64, mm1 / pmulhw xmm2/m128, xmm1 */
   case 0x0fd5:  /* pmullw mm2/m64, mm1 / pmullw xmm2/m128, xmm1 */
   case 0x0ff4:  /* pmuludq mm2/m64, mm1 / pmuludq xmm2/m128, xmm1 */
   case 0x0feb:  /* por mm2/m64, mm1 / por xmm2/m128, xmm1 */
   case 0x0f18:  /* prefetcht(0|1|2: m8 / prefetchnta m8 */
   case 0x0ff6:  /* psadbw mm2/m64, mm1 / psadbw xmm2/m128, xmm1 */
   case 0x0ff1:  /* psllw mm2/m64, mm1 / psllw xmm2/m128, xmm1 */
   case 0x0ff2:  /* pslld mm2/m64, mm1 / pslld xmm2/m128, xmm1 */
   case 0x0ff3:  /* psllq mm2/m64, mm1 / psllq xmm2/m128, xmm1 */
   case 0x0fe1:  /* psraw mm2/m64, mm1 / psraw xmm2/m128, xmm1 */
   case 0x0fe2:  /* psrad mm2/m64, mm1 / psrad xmm2/m128, xmm1 */
   case 0x0fd1:  /* psrlw mm2/m64, mm1 / psrlw xmm2/m128, xmm1 */
   case 0x0fd2:  /* psrld mm2/m64, mm1 / psrld xmm2/m128, xmm1 */
   case 0x0fd3:  /* psrlq mm2/m64, mm1 / psrlq xmm2/m128, xmm1 */
   case 0x0ff8:  /* psubb mm2/m64, mm1 / psubb xmm2/m128, xmm1 */
   case 0x0ff9:  /* psubw mm2/m64, mm1 / psubw xmm2/m128, xmm1  */
   case 0x0ffa:  /* psubd mm2/m64, mm1 / psubd xmm2/m128, xmm1  */
   case 0x0ffb:  /* psubq mm2/m64, mm1 / psubq xmm2/m128, xmm1  */
   case 0x0fe8:  /* psubsb mm2/m64, mm1 / psubsb xmm2/m128, xmm1 */
   case 0x0fe9:  /* psubsw mm2/m64, mm1 / psubsw xmm2/m128, xmm1 */
   case 0x0fd8:  /* psubusb mm2/m64, mm1 / psubusb xmm2/m128, xmm1 */
   case 0x0fd9:  /* psubusw mm2/m64, mm1 / psubusw xmm2/m128, xmm1 */
   case 0x0f68:  /* punpckhbw mm2/m64, mm1 / punpckhbw xmm2/m128, xmm1 */
   case 0x0f69:  /* punpckhbd mm2/m64, mm1 / punpckhbd xmm2/m128, xmm1 */
   case 0x0f6a:  /* punpckhbq mm2/m64, mm1 / punpckhbq xmm2/m128, xmm1 */
   case 0x0f6d:  /* punpckhqdq xmm2/m128, xmm1 */
   case 0x0f60:  /* punpcklbw mm2/m64, mm1 / punpcklbw xmm2/m128, xmm1 */
   case 0x0f61:  /* punpcklbd mm2/m64, mm1 / punpcklbd xmm2/m128, xmm1 */
   case 0x0f62:  /* punpcklbq mm2/m64, mm1 / punpcklbq xmm2/m128, xmm1 */
   case 0x0f6c:  /* punpcklqdq xmm2/m128, xmm1 */
   case 0x0fef:  /* pxor mm2/m64, mm1 / pxor xmm2/m128, xmm1 */
   case 0x0f53:  /* rcpps xmm2/m128, xmm1 / rcpss xmm2/m32, xmm1 */
   case 0x0f52:  /* rsqrtps xmm2/m128, xmm1 / rsqrtss xmm2/m128, xmm1 */
   case 0x0fa5:  /* shld %cl, %r16, r/m16 / shld %cl, %r32, r/m32 */
   case 0x0fad:  /* shrd %cl, %r16, r/m16 / shrd %cl, %r32, r/m32 */
   case 0x0f51:  /* sqrtpd xmm2/m128, xmm1 / sqrtps xmm2/m128, xmm1
                  * sqrtsd xmm2/m64, xmm1 / sqrtss xmm2/m32, xmm1 */
   case 0x0f5c:  /* subpd xmm2/m128, xmm1 / subps xmm2/m128, xmm1
                  * subsd xmm2/m64, xmm1 / subss xmm2/m32, xmm1 */
   case 0x0f2e:  /* ucomisd xmm2/m64, xmm1 / ucomiss xmm2/m64, xmm1 */
   case 0x0f15:  /* unpckhpd xmm2/m128, xmm1 / unpckhps xmm2/m128, xmm1 */
   case 0x0f14:  /* unpcklpd xmm2/m128, xmm1 / unpcklps xmm2/m128, xmm1 */
   case 0x0fc0:  /* xadd %r8, r/m8 */
   case 0x0fc1:  /* xadd %r16, r/m16 / xadd %r32, r/m32 */
   case 0x0f57:  /* xorpd xmm2/m128, xmm1 / xorps xmm2/m128, xmm1 */
   case 0x0fc5:  /* pextrw $imm8, mm, r32 / pextrw $imm8, xmm, r32 */
   case 0x0fc4:  /* pinsrw $imm8, r/m32, mm / pinsrw $imm8, r/m32, xmm */
   case 0x0f70:  /* pshufw $imm8, mm2/m64, mm1 / pshufd $imm8, xmm2/m128, xmm1
                  * pshufhw $imm8, xmm2/m128, xmm1 / pshuflw $imm8, xmm2/m128, xmm1 */
   case 0x0f71:  /* psllw $imm8, mm / psllw $imm8, xmm
                  * psraw $imm8, mm1 / psraw $imm8, xmm1
                  * psrlw $imm8, mm / psrlw $imm8, xmm */
   case 0x0f72:  /* pslld $imm8, mm1 / pslld $imm8, xmm1
                  * psrad $imm8, mm1 / psrad $imm8, xmm1
                  * psrld $imm8, mm1 / psrld $imm8, xmm1 */
   case 0x0f73:  /* psllq $imm8, mm1 / psllq $imm8, xmm1
                  * pslldq $imm8, xmm1 / psrldq $imm8, xmm1
                  * psrlq $imm8, mm1 / psrlq $imm8, xmm1 */
   case 0x0fa4:  /* shld $imm8, %r16, r/m16 / shld $imm8, %r32, r/m32 */
   case 0x0fac:  /* shrd $imm8, %r16, r/m16 / shrd $imm8, %r32, r/m32 */
   case 0x0fc6:  /* shufpd $imm8, xmm2/m128, xmm1 / shufps $imm8, xmm2/m128, xmm1 */
   case 0x0fba:  /* bt $imm8, r/m16 / bt $imm8, r/m32
                  * btc $imm8, r/m16 / btc $imm8, r/m32
                  * bts $imm8, r/m16 / bts $imm8, r/m32
                  * btr $imm8, r/m16 / btr $imm8, r/m32 */
   case 0x0fc2:  /* cmppd $imm8, xmm2/m128, xmm1 */
    text = X86_ModRmDecode(text,&modrm,flags);
    nc_addr = MODRM_MEM;
    goto noncanon_read;

   case 0xa0:  /* MOV AL,moffs8* */
    nc_addr = *(u8 *)text;
    nc_addr += X86_SegmentBase(context,flags);
    goto noncanon_read;
   case 0xa1:  /* MOV AX,moffs16*; MOV EAX,moffs32*; MOV RAX,moffs64* */
    if (flags & F_AD64) {
     nc_addr = *(u64 *)text;
    } else {
     nc_addr = *(u32 *)text;
    }
    nc_addr += X86_SegmentBase(context,flags);
    goto noncanon_read;

   case 0x0f01: /* invlpg m / lgdt m48 / lidt m48 / lmsw r/m16
                 * monitor / mwait / sgdt m48 / sidt m48
                 * smsw r/m16 / smsw r32/m16 */
    text = X86_ModRmDecode(text,&modrm,flags);
    if (modrm.mi_type != MODRM_MEMORY) break;
    nc_addr = MODRM_MEM;
    if (modrm.mi_reg == 0) goto noncanon_write; /* sgdt */
    if (modrm.mi_reg == 1) goto noncanon_write; /* sidt */
    if (modrm.mi_reg == 4) goto noncanon_write; /* smsw */
    goto noncanon_read;

   case 0x0fae:  /* clflush m8 / fxrstor m512byte / fxsave m512byte
                  * ldmxcsr m32 / lfence / mfence / sfence
                  * stmxcsr m32 */
    text = X86_ModRmDecode(text,&modrm,flags);
    if (modrm.mi_type != MODRM_MEMORY) break;
    nc_addr = MODRM_MEM;
    if (modrm.mi_reg == 1) goto noncanon_write; /* fxrstor */
    if (modrm.mi_reg == 2) goto noncanon_write; /* ldmxcsr */
    goto noncanon_read;

   case 0xa2:  /* MOV moffs8*,AL */
    nc_addr = *(u8 *)text;
    nc_addr += X86_SegmentBase(context,flags);
    goto noncanon_write;
   case 0xa3:  /* MOV moffs16*,AX; MOV moffs32*,EAX; MOV moffs64*,RAX */
    if (flags & F_AD64) {
     nc_addr = *(u64 *)text;
    } else {
     nc_addr = *(u32 *)text;
    }
    nc_addr += X86_SegmentBase(context,flags);
    goto noncanon_write;

   case 0xac: /* lodsb */
   case 0xad: /* lodsw / lodsl / lodsq */
   case 0x6e: /* outsb */
   case 0x6f: /* outsw / outsl / outsq */
    nc_addr = context->c_gpregs.gp_rsi;
    goto noncanon_read;

   case 0x6c: /* insb */
   case 0x6d: /* insw / insl / insq */
   case 0xaa: /* stosb */
   case 0xab: /* stosw / stosl / stosq */
   case 0xae: /* scasb */
   case 0xaf: /* scasw / scasl / scasq */
    nc_addr = context->c_gpregs.gp_rdi;
    goto noncanon_write;

   case 0xa4: /* movsb */
   case 0xa5: /* movsw / movsl / movsq */
    nc_addr = context->c_gpregs.gp_rsi;
    if (X86_PAGING_ISNONCANON(nc_addr) ||
        X86_PAGING_ISNONCANON(nc_addr+8))
        goto noncanon_read;
    nc_addr = context->c_gpregs.gp_rdi;
    goto noncanon_write;


   default: break;
   }
   __IF0 {
noncanon_read:
    info->e_error.e_segfault.sf_reason = 0;
    if (!X86_PAGING_ISNONCANON(nc_addr) &&
        !X86_PAGING_ISNONCANON(nc_addr+8))
         goto done_noncanon_check;
    goto do_noncanon;
noncanon_write:
    if (!X86_PAGING_ISNONCANON(nc_addr) &&
        !X86_PAGING_ISNONCANON(nc_addr+8))
         goto done_noncanon_check;
    info->e_error.e_segfault.sf_reason = X86_SEGFAULT_FWRITE;
do_noncanon:
    info->e_error.e_code = E_SEGFAULT;
    if (context->c_iret.ir_cs & 3)
        info->e_error.e_segfault.sf_reason |= X86_SEGFAULT_FUSER;
    info->e_error.e_segfault.sf_vaddr = (void *)nc_addr;
    goto do_throw_error;
   }
done_noncanon_check:
   text = old_text;
  }
#endif

  switch (flags & F_SEGMASK) {
#ifdef __x86_64__
  case F_SEGFS: __asm__("movw %%fs, %w0" : "=r" (effective_segment_value)); break;
  case F_SEGGS: __asm__("movw %%gs, %w0" : "=r" (effective_segment_value)); break;
  default: effective_segment_value = context->c_iret.ir_ss; break;
#else
#ifdef CONFIG_X86_FIXED_SEGMENTATION
  case F_SEGDS: effective_segment_value = X86_SEG_DS; break;
  case F_SEGES: effective_segment_value = X86_SEG_ES; break;
#else /* CONFIG_X86_FIXED_SEGMENTATION */
  case F_SEGDS: effective_segment_value = context->c_segments.sg_ds; break;
  case F_SEGES: effective_segment_value = context->c_segments.sg_es; break;
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
  case F_SEGFS: effective_segment_value = context->c_segments.sg_fs; break;
  case F_SEGGS: effective_segment_value = context->c_segments.sg_gs; break;
  case F_SEGCS: effective_segment_value = context->c_iret.ir_cs; break;
  case F_SEGSS: effective_segment_value = context->c_iret.ir_ss; break;
  default: __builtin_unreachable(); break;
#endif
  }

  switch (opcode) {

#ifdef X86_READOPCODE_FAILED
  case X86_READOPCODE_FAILED:
   return;
#endif

  case 0x0f22:
   /* MOV CRx,r32 */
   text = X86_ModRmDecode(text,&modrm,flags);
   info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                  ERROR_ILLEGAL_INSTRUCTION_FVALUE);
   info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_CONTROL;
   info->e_error.e_illegal_instruction.ii_register_number = modrm.mi_reg;
   info->e_error.e_illegal_instruction.ii_value = MODRM_REG;
#ifdef __x86_64__
   if (modrm.mi_reg == 1 || (modrm.mi_reg > 4 && modrm.mi_reg != 8))
#else
   if (modrm.mi_reg == 1 || modrm.mi_reg > 4)
#endif
    /* Error was caused because CR* doesn't exist */
    info->e_error.e_illegal_instruction.ii_type |= ERROR_ILLEGAL_INSTRUCTION_UNDEFINED;
   else if (context->c_iret.ir_cs & 3)
    /* Error was caused because CR* are privileged */
    info->e_error.e_illegal_instruction.ii_type |= ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED;
   else {
    /* Error was caused because value written must be invalid. */
    info->e_error.e_illegal_instruction.ii_type |= ERROR_ILLEGAL_INSTRUCTION_FOPERAND;
   }
   break;

  case 0x0f20:
   /* MOV r32,CRx */
   text = X86_ModRmDecode(text,&modrm,flags);
   info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_CONTROL;
   info->e_error.e_illegal_instruction.ii_register_number = modrm.mi_reg;
#ifdef __x86_64__
   if (modrm.mi_reg == 1 || (modrm.mi_reg > 4 && modrm.mi_reg != 8))
#else
   if (modrm.mi_reg == 1 || modrm.mi_reg > 4)
#endif
   {
    /* Undefined control register. */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                   ERROR_ILLEGAL_INSTRUCTION_UNDEFINED);
   } else {
    /* Userspace tried to read from an control register. */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                   ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED);
   }
   break;


  case 0x0f21:
  case 0x0f23:
   /* MOV r32, DR0-DR7 */
   /* MOV DR0-DR7, r32 */
   text = X86_ModRmDecode(text,&modrm,flags);
   /* Userspace tried to access an control register. */
   info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                  ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED);
   info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_DEBUG;
   info->e_error.e_illegal_instruction.ii_register_number = modrm.mi_reg;
   if (opcode == 0x0f23) {
    /* Save the value user-space tried to write. */
    info->e_error.e_illegal_instruction.ii_type |= ERROR_ILLEGAL_INSTRUCTION_FVALUE;
    info->e_error.e_illegal_instruction.ii_value = MODRM_REG;
   }
   break;

  case 0x0f00:
   text = X86_ModRmDecode(text,&modrm,flags);
   if (effective_segment_value == 0) goto null_segment_error;
   info->e_error.e_illegal_instruction.ii_register_type = X86_REGISTER_MISC;
   if (modrm.mi_reg == 3) {
    /* LTR r/m16 */
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_TR;
   } else if (modrm.mi_reg == 2) {
    /* LLDT r/m16 */
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_LDT;
   } else if (modrm.mi_reg == 0) {
    /* SLDT r/m16 */
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_LDT;
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
    break;
   } else {
    goto generic_failure;
   }
   if (errcode != 0 && !X86_ANYCONTEXT_ISUSER(*context)) {
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
   info->e_error.e_illegal_instruction.ii_value = MODRM_MEMW;
   break;

  case 0x0f01:
   text = X86_ModRmDecode(text,&modrm,flags);
   if (effective_segment_value == 0) goto null_segment_error;
   info->e_error.e_illegal_instruction.ii_register_type = X86_REGISTER_MISC;
   if (modrm.mi_reg == 2) {
    /* LGDT m16&32 */
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_TR;
   } else if (modrm.mi_reg == 3) {
    /* LIDT m16&32 */
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_LDT;
   } else if (modrm.mi_reg == 0) {
    /* SGDT r/m16 */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_GDT;
    break;
   } else if (modrm.mi_reg == 1) {
    /* SIDT r/m16 */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_MISC_IDT;
    break;
   } else if (modrm.mi_reg == 4) {
    /* SMSW r/m16 */
    /* SMSW r/m32 */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
    info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_CONTROL;
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_CONTROL_CR0;
    break;
   } else if (modrm.mi_reg == 6) {
    /* LMSW r/m16 */
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                   ERROR_ILLEGAL_INSTRUCTION_FVALUE);
    info->e_error.e_illegal_instruction.ii_register_type   = X86_REGISTER_CONTROL;
    info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_CONTROL_CR0;
    info->e_error.e_illegal_instruction.ii_value = MODRM_MEMW;
    break;
   } else {
    goto generic_failure;
   }
   /* User-space use of a privileged instruction. */
   info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                  ERROR_ILLEGAL_INSTRUCTION_FREGISTER|
                                                  ERROR_ILLEGAL_INSTRUCTION_FVALUE);
   info->e_error.e_illegal_instruction.ii_value = X86_ModRmGetMem(context,&modrm,flags);
   break;

  case 0x8c:
   /* MOV r/m16,Sreg** */
   text = X86_ModRmDecode(text,&modrm,flags);
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

  case 0x8e:
   /* MOV Sreg**,r/m16 */
   text = X86_ModRmDecode(text,&modrm,flags);
   info->e_error.e_illegal_instruction.ii_value = MODRM_MEMW;
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
   break;

  case 0x0f31: /* rdtsc */
  case 0x0f32: /* rdmsr */
  case 0x0f33: /* rdpmc */
   if (X86_ANYCONTEXT_ISUSER(*context)) {
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
   } else {
    info->e_error.e_illegal_instruction.ii_type = (ERROR_ILLEGAL_INSTRUCTION_UNDEFINED|
                                                   ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
   }
   info->e_error.e_illegal_instruction.ii_register_type = X86_REGISTER_MSR;
   if (opcode == 0x0f31) {
    if (X86_ANYCONTEXT_ISUSER(*context)) {
     /* rdtsc can be enabled for user-space, but apparently it isn't. */
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
   if (X86_ANYCONTEXT_ISUSER(*context)) {
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
 memcpy(&info->e_context,xcontext,sizeof(struct cpu_context));
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
 default: info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_SEGMENT_DS; break;
 case F_SEGFS: info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_SEGMENT_FS; break;
 case F_SEGGS: info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_SEGMENT_GS; break;
#ifndef __x86_64__
 case F_SEGES: info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_SEGMENT_ES; break;
 case F_SEGCS: info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_SEGMENT_CS; break;
 case F_SEGSS: info->e_error.e_illegal_instruction.ii_register_number = X86_REGISTER_SEGMENT_SS; break;
#endif /* !__x86_64__ */
 }
 goto do_throw_error;
generic_failure:
 /* Fallback: Choose between a NULL-segment and generic error. */
 if (!effective_segment_value)
      goto null_segment_error;
#ifdef __x86_64__
 /* On x86_64, a #GPF is thrown when attempting to access a non-canonical address.
  * However, the kernel expects that the only exception that might be thrown when
  * accessing some unchecked pointer is an E_SEGFAULT.
  * Code above tries to inspect the source instruction to determine the faulting
  * memory address. However there are literally thousands of different X86
  * instructions that take a memory operand, and we can only know about so many
  * before we run into one that may not even have existed at the time this
  * decoder was written.
  * So despite the fact that we haven't managed to figure out the faulting memory
  * address, simply assume that a segment selector of ZERO(0) is indicative of a
  * instruction that tried to access a non-canonical address.
  * In this case, we set the first non-canonical address as faulting address.
  * Also: we don't know if it was a write that caused the problem, so we just
  *       always act like it was a read. */
 if (xerrcode == 0) {
  info->e_error.e_code = E_SEGFAULT;
  info->e_error.e_segfault.sf_reason = 0;
  if (xcontext->c_iret.ir_cs & 3)
      info->e_error.e_segfault.sf_reason |= X86_SEGFAULT_FUSER;
  info->e_error.e_segfault.sf_vaddr = (void *)X86_PAGING_NONCANON_MIN;
  goto do_throw_error;
 }
#endif
 /* If the error originated from user-space, default to assuming it's
  * because of some privileged instruction not explicitly handled
  * above. (e.g.: `wbinvd') */
 if (xcontext->c_iret.ir_cs & 3)
     goto e_privileged_instruction;
#ifndef CONFIG_NO_VM86
 if (xcontext->c_iret.ir_pflags & EFLAGS_VM)
     goto e_privileged_instruction;
#endif
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
