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
#ifndef GUARD_KERNEL_I386_KOS_SYSCALL_C
#define GUARD_KERNEL_I386_KOS_SYSCALL_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <i386-kos/syscall.h>
#include <i386-kos/ipi.h>
#include <i386-kos/interrupt.h>
#include <i386-kos/cpuid.h>
#include <kernel/syscall.h>
#include <kernel/debug.h>
#include <sched/task.h>
#include <except.h>
#include <alloca.h>
#include <string.h>
#include <kos/intrin.h>
#include <asm/syscallno.ci>

DECL_BEGIN

INTERN u8 KCALL restart_syscall_mode(syscall_ulong_t sysno) {
 u8 mode;
 if (sysno <= __NR_syscall_max)
  mode = x86_syscall_restart[sysno];
 else if (sysno >= __NR_xsyscall_min && sysno <= __NR_xsyscall_max)
  mode = x86_xsyscall_restart[sysno-__NR_xsyscall_min];
 else {
  mode = X86_SYSCALL_RESTART_FAUTO;
 }
 return mode;
}

PUBLIC bool KCALL
should_restart_syscall(syscall_ulong_t sysno,
                       unsigned int context) {
 u8 mode = restart_syscall_mode(sysno);
 debug_printf("should_restart_syscall(%p,%x)\n",sysno,context);
 switch (mode) {

 case X86_SYSCALL_RESTART_FAUTO:
  return (context & (SHOULD_RESTART_SYSCALL_FPOSIX_SIGNAL|
                     SHOULD_RESTART_SYSCALL_FSA_RESTART)) ==
                    (SHOULD_RESTART_SYSCALL_FPOSIX_SIGNAL|
                     SHOULD_RESTART_SYSCALL_FSA_RESTART);
  break;

 case X86_SYSCALL_RESTART_FDONT:
  return false;

 default: break;
 }
 return true;
}



PRIVATE void KCALL
x86_run_on_each_cpu(void (KCALL *func)(void *arg), void *arg) {
 volatile int *status;
 cpuid_t id;
 status = (volatile int *)alloca(cpu_count*sizeof(volatile int));
 memsetl((void *)status,X86_IPI_EXEC_PENDING,cpu_count);
 COMPILER_WRITE_BARRIER();
 /* Send an IPI to each CPU */
 for (id = 0; id < cpu_count; ++id) {
  struct x86_ipi ipi;
  ipi.ipi_type           = X86_IPI_EXEC;
  ipi.ipi_flag           = X86_IPI_FNORMAL;
  ipi.ipi_exec.wt_func   = func;
  ipi.ipi_exec.wt_arg    = arg;
  ipi.ipi_exec.wt_status = &status[id];
  x86_ipi_send(cpu_vector[id],&ipi);
 }
 /* Wait for execution to finish on all cpus. */
 for (;;) {
  COMPILER_READ_BARRIER();
  for (id = 0; id < cpu_count; ++id) {
   if (status[id] == X86_IPI_EXEC_PENDING)
       goto wait_for_cpu;
  }
  break;
wait_for_cpu:
  task_tryyield();
 }
}


INTDEF u32 x86_syscall_exec80_fixup;
INTDEF uintptr_t x86_syscall_exec80_trace[];
INTDEF uintptr_t x86_syscall_exec80_ntrace[];
INTDEF struct x86_idtentry x86_idt_start[256];
INTDEF void ASMCALL irq_80(void);
INTDEF void ASMCALL irq_80_trace(void);
INTDEF void ASMCALL sysenter_kernel_entry(void);
INTDEF void ASMCALL sysenter_kernel_entry_trace(void);
#ifdef __x86_64__
INTDEF void ASMCALL syscall_kernel_entry(void);
INTDEF void ASMCALL syscall_kernel_entry_trace(void);
#endif



PRIVATE NOIRQ void KCALL
x86_set_sysenter_ip(void *arg) {
 if (CPU_FEATURES.ci_1d & CPUID_1D_SEP)
     __wrmsr(IA32_SYSENTER_EIP,(uintptr_t)arg);
#ifdef __x86_64__
 if (CPU_FEATURES.ci_80000001d & CPUID_80000001D_SYSCALL) {
  __wrmsr(IA32_LSTAR,
         (uintptr_t)arg == (uintptr_t)&sysenter_kernel_entry_trace
          ? (uintptr_t)&syscall_kernel_entry_trace
          : (uintptr_t)&syscall_kernel_entry);
 }
#endif
}




/* Enable/disable system call tracing on all CPUs.
 * When enabled, `syscall_trace()' will be executed before any system call invocation. */
PUBLIC void KCALL
enable_syscall_tracing(void) {
 uintptr_t addr = (uintptr_t)&irq_80_trace;
 u16 EXCEPT_VAR old_flags;
 old_flags = ATOMIC_FETCHOR(THIS_TASK->t_flags,TASK_FKEEPCORE);
 TRY {
  pflag_t was;
  /* Suspend all other CPUs */
  while (!x86_unicore_begin()) task_tryyield();
  /* Disable preemption locally */
  was = PREEMPTION_PUSHOFF();
  COMPILER_BARRIER();
  /* Override the IDT vector. */
  x86_idt_start[0x80].ie_off1 = (u16)addr;
  x86_idt_start[0x80].ie_off2 = (u16)(addr >> 16);
#ifdef __x86_64__
  x86_idt_start[0x80].ie_off3 = (u32)(addr >> 32);
#endif
  /* Override exec80 jump address. */
  x86_syscall_exec80_fixup = ((u32)(uintptr_t)x86_syscall_exec80_trace -
                             ((u32)(uintptr_t)&x86_syscall_exec80_fixup + 4));
  COMPILER_BARRIER();
  /* Re-enable preemption. */
  PREEMPTION_POP(was);
  /* Resume execution on other CPUs. */
  x86_unicore_end();
 } FINALLY {
  if (!(old_flags & TASK_FKEEPCORE))
        ATOMIC_FETCHAND(THIS_TASK->t_flags,~TASK_FKEEPCORE);
 }
 x86_run_on_each_cpu(&x86_set_sysenter_ip,
                    (void *)&sysenter_kernel_entry_trace);
}

PUBLIC void KCALL
disable_syscall_tracing(void) {
 uintptr_t addr = (uintptr_t)&irq_80;
 u16 EXCEPT_VAR old_flags;
 old_flags = ATOMIC_FETCHOR(THIS_TASK->t_flags,TASK_FKEEPCORE);
 TRY {
  pflag_t was;
  while (!x86_unicore_begin()) task_tryyield();
  was = PREEMPTION_PUSHOFF();
  COMPILER_BARRIER();
  x86_idt_start[0x80].ie_off1 = (u16)addr;
  x86_idt_start[0x80].ie_off2 = (u16)(addr >> 16);
#ifdef __x86_64__
  x86_idt_start[0x80].ie_off3 = (u32)(addr >> 32);
#endif
  x86_syscall_exec80_fixup = ((u32)(uintptr_t)x86_syscall_exec80_ntrace -
                             ((u32)(uintptr_t)&x86_syscall_exec80_fixup + 4));
  COMPILER_BARRIER();
  PREEMPTION_POP(was);
  x86_unicore_end();
 } FINALLY {
  if (!(old_flags & TASK_FKEEPCORE))
        ATOMIC_FETCHAND(THIS_TASK->t_flags,~TASK_FKEEPCORE);
 }
 x86_run_on_each_cpu(&x86_set_sysenter_ip,
                    (void *)&sysenter_kernel_entry);
}


struct bad_syscall_info {
    uintptr_t si_sysno;
    uintptr_t si_args[6];
};

INTERN ATTR_NORETURN void FCALL
x86_throw_bad_syscall(struct bad_syscall_info *__restrict syscall) {
 struct exception_info *info = error_info();
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_unknown_systemcall.us_sysno = syscall->si_sysno;
 memcpy(info->e_error.e_unknown_systemcall.us_args,
        syscall->si_args,6*sizeof(uintptr_t));
 info->e_error.e_code = E_UNKNOWN_SYSTEMCALL;
 info->e_error.e_flag = ERR_FNORMAL;
 error_throw_current();
 __builtin_unreachable();
}


#ifndef __x86_64__
struct bad_syscall_sysenter_info {
    uintptr_t si_ebp;
    uintptr_t si_sysno;
    uintptr_t si_args[4];
};
INTERN ATTR_NORETURN void FCALL
x86_throw_bad_syscall_sysenter(struct bad_syscall_sysenter_info *__restrict syscall) {
 struct exception_info *info = error_info();
 uintptr_t arg4 = 0,arg5 = 0;
 if (syscall->si_ebp < KERNEL_BASE) {
  /* Try to load additional arguments. */
  TRY {
   COMPILER_READ_BARRIER();
   arg4 = ((uintptr_t *)syscall->si_ebp)[0];
   arg5 = ((uintptr_t *)syscall->si_ebp)[1];
   COMPILER_READ_BARRIER();
  } CATCH_HANDLED (E_SEGFAULT) {
  }
 }
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_unknown_systemcall.us_sysno = syscall->si_sysno;
 memcpy(info->e_error.e_unknown_systemcall.us_args,
        syscall->si_args,4*sizeof(uintptr_t));
 info->e_error.e_unknown_systemcall.us_args[4] = arg4;
 info->e_error.e_unknown_systemcall.us_args[5] = arg5;
 info->e_error.e_code = E_UNKNOWN_SYSTEMCALL;
 info->e_error.e_flag = ERR_FNORMAL;
 error_throw_current();
 __builtin_unreachable();
}
#endif /* !__x86_64__ */


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_SYSCALL_C */
