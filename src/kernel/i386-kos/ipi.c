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
#ifndef GUARD_KERNEL_I386_KOS_IPI_C
#define GUARD_KERNEL_I386_KOS_IPI_C 1
#define _KOS_SOURCE 1

#include "scheduler.h"

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <i386-kos/apic.h>
#include <i386-kos/ipi.h>
#include <i386-kos/interrupt.h>
#include <kernel/debug.h>
#include <kernel/interrupt.h>
#include <kernel/vm.h>
#include <sched/task.h>

#include <asm/cpu-flags.h>
#include <except.h>
#include <string.h>

#ifndef CONFIG_NO_SMP
DECL_BEGIN

STATIC_ASSERT(offsetof(struct x86_ipi,ipi_type) == X86_IPI_OFFSETOF_TYPE);
STATIC_ASSERT(sizeof(struct x86_ipi) == X86_IPI_SIZE);


INTDEF void KCALL x86_scheduler_loadpending(void);
INTDEF NOIRQ bool KCALL x86_scheduler_localwake(struct task *__restrict thread);
INTDEF NOIRQ bool KCALL x86_scheduler_localwake_p(struct task *prev, struct task *__restrict thread);
INTDEF void KCALL x86_redirect_preempted_userspace(struct task *__restrict thread);


/* [lock(WRITE(THIS_CPU),READ(x86_ipi_suspension_lock))]
 * Set to `> 0' when an `X86_IPI_SUSPEND_CPU' IPI has been received (and the CPU is now suspended).
 * Set to `<= 0' when an `X86_IPI_RESUME_CPU' IPI has been received (and the CPU is no longer suspended).
 * Used to synchrize `x86_unicore_begin()' functionality. */
PUBLIC ATTR_PERCPU int x86_cpu_suspended = 0;


/* Per-CPU IPI buffer.
 * NOTE: This buffer _MUST_ be allocated statically, as we'd
 *       never be be to get this working if it used kmalloc(),
 *       as this too must be 100% async-safe. */
INTERN ATTR_PERCPU ATOMIC_DATA u32 ipi_valid; /* Bitset of initialized IPIs. (NOTE: Bits in this set may only be unset by THIS_CPU). */
INTERN ATTR_PERCPU ATOMIC_DATA u32 ipi_alloc; /* Bitset of allocated IPIs. (NOTE: Bits in this set may only be unset by THIS_CPU). */
INTERN ATTR_PERCPU struct x86_ipi ipi_buffer[32];

/* Load the context of the given thread (next_task), which
 * must the the currently active thread on the calling CPU,
 * without saving the state of the calling thread.
 * This function is called at the end of `task_exit()' cleanup in
 * order to load the context of whatever that should be switched to.
 * NOTE: When entered using a `jmp', don't modify the stack of
 *       the old task, meaning that this is function can be used
 *       to switch task contexts in situations where it is unclear
 *       if the calling thread is still alive. */
INTDEF NOIRQ ATTR_NORETURN void FCALL
x86_load_context(struct task *__restrict next_task,
                 PHYS uintptr_t old_pdir);



/* Handle an IPI request.
 * NOTE: The implementation of this function is async-safe.
 *       Additionally, this function must not throw any exceptions.
 * NOTE: This function is called synchronously from within the
 *       context of whatever task had been running when the
 *       interrupt occurred.
 * NOTE: This function is executed with interrupts disabled. */
#define X86_IPI_CONTINUE      0x00
#define X86_IPI_ACKNOWLEDGED  0x01
#define X86_IPI_STOP          0x02
INTERN NOIRQ int KCALL
x86_ipi_handle(struct x86_ipi const *__restrict ipi,
               bool ipi_acknowledged) {
 assert(!PREEMPTION_ENABLED());
#if 0
 debug_printf("HANDLE_IPI(%I16u in CPU #%u)\n",
              ipi->ipi_type,THIS_CPU->cpu_id);
#endif
 switch (ipi->ipi_type) {

 case X86_IPI_INVLPG:
  /* Check if the given page directory matches the current,
   * and invalidate the given address range if it does. */
  if (&THIS_VM->vm_pagedir == ipi->ipi_invlpg.ivp_pagedir ||
      &pagedir_kernel      == ipi->ipi_invlpg.ivp_pagedir)
       pagedir_sync(ipi->ipi_invlpg.ivp_pageindex,
                    ipi->ipi_invlpg.ivp_numpages);
  break;

 case X86_IPI_INVLPG_ALL:
  pagedir_syncall();
  break;

 case X86_IPI_INVLPG_ONE:
  /* Check if the given page directory matches the current,
   * and invalidate the given address range if it does. */
  if (&THIS_VM->vm_pagedir == ipi->ipi_invlpg.ivp_pagedir ||
      &pagedir_kernel      == ipi->ipi_invlpg.ivp_pagedir)
       pagedir_syncone(ipi->ipi_invlpg.ivp_pageindex);
  break;

 case X86_IPI_SCHEDULE:
  x86_scheduler_loadpending();
  break;

 {
  struct task *thread;
 case X86_IPI_UNSCHEDULE:
  thread = ipi->ipi_unschedule.us_thread;
  if (thread->t_cpu != THIS_CPU) {
   /* The thread is being hosted elsewhere. */
   *ipi->ipi_unschedule.us_status = X86_IPI_UNSCHEDULE_RETRY;
  } else if (thread->t_flags & TASK_FKEEPCORE) {
   /* The thread can't be moved right now. */
   *ipi->ipi_unschedule.us_status = X86_IPI_UNSCHEDULE_KEEP;
  } else if (TASK_ISTERMINATING(thread)) {
   /* The thread is dead (or dying). */
   *ipi->ipi_unschedule.us_status = X86_IPI_UNSCHEDULE_DEAD;
  } else if (thread != THIS_TASK) {
   /* Make sure to load pending tasks first, just in case
    * the given thread hasn't actually been started, yet. */
   x86_scheduler_loadpending();
   COMPILER_BARRIER();
   /* Simple case: Unschedule some secondary thread. */
   if (thread->t_state & TASK_STATE_FSLEEPING) {
    LIST_REMOVE(thread,t_sched.sched_list);
   } else {
    RING_REMOVE(thread,t_sched.sched_ring);
   }
   /* Successfully unscheduled the thread.
    * The sender of the IPI must now inherit the
    * reference that used to be held by the scheduler. */
   *ipi->ipi_unschedule.us_status = X86_IPI_UNSCHEDULE_OK;
  } else {
   /* Special case: The interrupted thread should be unscheduled.
    * In this case we must check if it's the last thread, as well
    * as do some pretty complicated stuff to acknowledge the
    * unscheduling before atomically switching to the next task. */
   struct task *sched_next;
   assert(!(thread->t_state & TASK_STATE_FSLEEPING));
   /* TODO: Save the FPU context */
   sched_next = thread->t_sched.sched_ring.re_next;
   if (sched_next == thread) {
    /* TODO: Once implemented, check if we can load threads
     *       with the `TASK_STATE_FIDLETHREAD' flag set. */
    /* Special case: the current thread is the last one being hosted
     *               by the CPU, meaning we can't take take it away. */
    *ipi->ipi_unschedule.us_status = X86_IPI_UNSCHEDULE_LAST;
   } else {
    /* Atomically switch to the next task. */
    RING_REMOVE(thread,t_sched.sched_ring);
    /* Set the next pending thread as the one now actively running. */
    THIS_CPU->c_running = sched_next;
    __asm__ __volatile__ goto("pushfl\n"                         /* IRET.EFLAGS */
                              "orl    %[eflags_if], (%%esp)\n"   /* IRET.EFLAGS */
                              "pushl  %%cs\n"                    /* IRET.CS */
                              "pushl  $%l5\n"                    /* IRET.EIP */
#ifdef CONFIG_X86_SEGMENTATION
                              "pushl  %%ds\n"                    /* SEGMENTS.DS */
                              "pushl  %%es\n"                    /* SEGMENTS.ES */
                              "pushl  %%fs\n"                    /* SEGMENTS.FS */
                              "pushl  %%gs\n"                    /* SEGMENTS.GS */
#endif /* CONFIG_X86_SEGMENTATION */
                              "pushal\n"                         /* GPREGS */
                              /* Saved the IRET tail that we just generated. */
                              "movl   %%esp, %%" PP_STR(__ASM_TASK_SEGMENT) ":" PP_STR(TASK_OFFSETOF_CONTEXT) "\n"
                              /* Indicate a successful unschedule. */
                              "movl   %[status_ok], %[status]\n"
                              /* Resume execution in this CPU by loading the next thread */
                              "jmp    x86_load_context"
                              :
                              : [next]      "c" (sched_next)
                              , [physdir]   "d" ((uintptr_t)thread->t_vm->vm_physdir)
                              , [status]    "m" (*ipi->ipi_unschedule.us_status)
                              , [eflags_if] "i" (EFLAGS_IF)
                              , [status_ok] "i" (X86_IPI_UNSCHEDULE_OK)
                              : "memory", "cc", "esp"
                              : newcpu_continue);
newcpu_continue:
    /* This is where the calling thread continues
     * execution within the context of the new CPU. */
    return X86_IPI_STOP;
   }
  }
 } break;

 case X86_IPI_WAKETASK:
 case X86_IPI_WAKETASK_FOR_RPC:
  if (ipi->ipi_wake.wt_task->t_cpu == THIS_CPU) {
   /* Because interrupts are disabled, we know
    * that our own CPU can't change right now.
    * Additionally, since we've now confirmed that
    * this task is indeed apart of our CPU, we can
    * simply move it from the `c_sleeping' to the
    * `c_running' chain (causing a sporadic wakeup). */
   if (TASK_ISTERMINATED(ipi->ipi_wake.wt_task)) {
    /* The task has died and can no longer be woken. */
    *ipi->ipi_wake.wt_status = X86_IPI_WAKETASK_DEAD;
   } else {
    if (!x86_scheduler_localwake(ipi->ipi_wake.wt_task) &&
         ipi->ipi_type == X86_IPI_WAKETASK_FOR_RPC)
         x86_redirect_preempted_userspace(ipi->ipi_wake.wt_task);
    COMPILER_BARRIER();
    /* Signal that the wakeup was successful.
     * We can't signal this before, because once this
     * is signaled, the caller will no longer keep
     * a reference to the task around for us. */
    *ipi->ipi_wake.wt_status = X86_IPI_WAKETASK_OK;
    COMPILER_WRITE_BARRIER();
   }
  } else {
   /* This task isn't running on our CPU */
   *ipi->ipi_wake.wt_status = X86_IPI_WAKETASK_RETRY;
   COMPILER_WRITE_BARRIER();
  }
  break;

 case X86_IPI_WAKETASK_P:
  if (ipi->ipi_wake_p.wt_task->t_cpu == THIS_CPU) {
   if (TASK_ISTERMINATED(ipi->ipi_wake_p.wt_task)) {
    *ipi->ipi_wake_p.wt_status = X86_IPI_WAKETASK_DEAD;
   } else {
    x86_scheduler_localwake_p(ipi->ipi_wake_p.wt_prev,
                              ipi->ipi_wake_p.wt_task);
    COMPILER_BARRIER();
    *ipi->ipi_wake_p.wt_status = X86_IPI_WAKETASK_OK;
    COMPILER_WRITE_BARRIER();
   }
  } else {
   /* This task isn't running on our CPU */
   *ipi->ipi_wake_p.wt_status = X86_IPI_WAKETASK_RETRY;
   COMPILER_WRITE_BARRIER();
  }
  break;

 case X86_IPI_SUSPEND_CPU:
  if (++PERCPU(x86_cpu_suspended) != 1)
      break; /* Recursively suspended. */
  /* Acknowledge the IPI so we can actually receive the resume-commands. */
  if (!ipi_acknowledged)
       lapic_write(APIC_EOI,APIC_EOI_FSIGNAL);
  COMPILER_WRITE_BARRIER();
  /* TODO: Disable scheduling on this CPU */
  for (;;) {
   COMPILER_READ_BARRIER();
   if (PERCPU(x86_cpu_suspended) < 1)
       break; /* A `X86_IPI_RESUME_CPU' IPI was received. */
#if 1
   /* Because STI only enables interrupts after the next instruction,
    * this code actually doesn't introduce a race condition. */
   __asm__ __volatile__("sti\n" /* Enable interrupts so we can receive `X86_IPI_RESUME_CPU' */
                        "hlt\n" /* Wait for interrupts. */
                        "cli\n" /* Disable interrupts again. */
                        :
                        :
                        : "memory");
#endif
  }
  /* TODO: Re-enable scheduling on this CPU */
  /* Indicate that we've already acknowledged the IPI */
  return X86_IPI_ACKNOWLEDGED;

 case X86_IPI_RESUME_CPU:
  /* Unset the suspended-flag. */
  --PERCPU(x86_cpu_suspended);
  COMPILER_WRITE_BARRIER();
  break;

 case X86_IPI_EXEC:
  /* Execute the provide function. */
  COMPILER_BARRIER();
  (*ipi->ipi_exec.wt_func)(ipi->ipi_exec.wt_arg);
  COMPILER_BARRIER();
  /* Signify that the IPI has finished execution. */
  *ipi->ipi_exec.wt_status = X86_IPI_EXEC_OK;
  COMPILER_WRITE_BARRIER();
  break;

 default: break;
 }
 return X86_IPI_CONTINUE;
}




INTERN NOIRQ bool KCALL x86_ipi_process(void) {
 u32 valid_mask; unsigned int ipi_index; int mode;
 bool result = false; struct x86_ipi ipi;
 while ((valid_mask = ATOMIC_LOAD(PERCPU(ipi_valid))) != 0) {
  /* Find the first valid IPI index. */
  ipi_index = 0;
  while (!(valid_mask & (1 << ipi_index))) ++ipi_index;
  /* Must copy the IPI to prevent recursive IPIs (like SUSPEND)
   * from be re-executed (and causing a deadlock) when RESUME arrives.
   * Similarly, we must deallocate the IPI before executive is (see below) */
  memcpy(&ipi,&PERCPU(ipi_buffer)[ipi_index],sizeof(struct x86_ipi));
  valid_mask = ~(1 << ipi_index);
  COMPILER_READ_BARRIER();
  /* Unset the VALID and ALLOC bits for this IPI.
   * NOTE: The order here is important because other CPUs first
   *       look at the `ipi_alloc' bitset before assuming that any
   *       ZERO-bit within implies a mirrored ZERO-bit in `ipi_valid'. */
  ATOMIC_FETCHAND(PERCPU(ipi_valid),valid_mask);
  ATOMIC_FETCHAND(PERCPU(ipi_alloc),valid_mask);
  COMPILER_BARRIER();

  /* Handle this IPI. */
  mode = x86_ipi_handle(&ipi,result);
  if (mode == X86_IPI_STOP)
      return true; /* Return from IPI after switching to a new CPU */
  if (mode == X86_IPI_ACKNOWLEDGED)
      result = true;
 }
 return result;
}



/* Send (and execute) and IPI to the given `target' CPU.
 * If the given `target' is the calling CPU, the `ipi'
 * argument is simply forwarded to `x86_ipi_handle()'.
 * IPIs are asynchronously executed in order. */
PUBLIC ASYNCSAFE void KCALL
x86_ipi_send(struct cpu *__restrict target,
             struct x86_ipi const *__restrict ipi) {
 u16 flags; pflag_t was;
 /* Ensure that the keep-core flag is set, so we can
  * safely compare `target' against the current CPU
  * without running the risk of being moved to another CPU.
  * This is necessary so we can safely bypass (slow) IPI
  * and just handle the command ourself if it's our own CPU.
  */
restart:
 flags = ATOMIC_FETCHOR(THIS_TASK->t_flags,TASK_FKEEPCORE);
 was = PREEMPTION_PUSHOFF();
 if (THIS_CPU == target) {
  /* Just handle it ourselves */
  x86_ipi_handle(ipi,true);
 } else {
  u32 alloc_mask;
  unsigned int my_bit;
  assert(X86_HAVE_LAPIC);
  /* Allocate an IPI on the target CPU. */
  do {
   alloc_mask = ATOMIC_LOAD(FORCPU(target,ipi_alloc));
   if unlikely(alloc_mask == 0xffffffff) {
    /* Go do something else and try again when the CPU has handled other pending IPIs.
     * NOTE: Getting here is highly unlikely, unless that CPUs is being bombarded with IPIs. */
    PREEMPTION_POP(was);
    if (!(flags&TASK_FKEEPCORE))
          ATOMIC_FETCHAND(THIS_TASK->t_flags,~TASK_FKEEPCORE);
    task_tryyield();
    goto restart;
   }
   /* Find the first, free IPI slot. */
   my_bit = 0;
   while (alloc_mask & (1 << my_bit)) ++my_bit;
   /* Allocate an IPI slot. */
  } while (!ATOMIC_CMPXCH_WEAK(FORCPU(target,ipi_alloc),
                               alloc_mask,alloc_mask | (1 << my_bit)));

  /* Copy IPI data from the caller-given buffer. */
  memcpy(&FORCPU(target,ipi_buffer[my_bit]),ipi,sizeof(struct x86_ipi));

  /* Mark the IPI as valid.
   * NOTE: This 2-step allocation is required to prevent a
   *       race that could arise when the target CPU is already
   *       handling IPIs, and starts processing ours before
   *       we've finished writing out its data. */
  ATOMIC_FETCHOR(FORCPU(target,ipi_valid),1 << my_bit);

  /* Send an IPI (with more than one CPU, we can assume that there is a LAPIC) */
  lapic_write(APIC_ICR1,APIC_ICR1_MKDEST(FORCPU(target,x86_lapic_id)));
  lapic_write(APIC_ICR0,
              X86_INTERRUPT_APIC_IPI |
              APIC_ICR0_TYPE_FNORMAL |
              APIC_ICR0_DEST_PHYSICAL |
              APIC_ICR0_FASSERT |
              APIC_ICR0_TARGET_FICR1);
 }
 PREEMPTION_POP(was);
 if (!(flags&TASK_FKEEPCORE))
       ATOMIC_FETCHAND(THIS_TASK->t_flags,~TASK_FKEEPCORE);
}


/* Broadcast an IPI to all CPUs (including the
 * calling when `also_send_to_self' is true) */
FUNDEF ASYNCSAFE void KCALL
x86_ipi_broadcast(struct x86_ipi const *__restrict ipi,
                  bool also_send_to_self) {
 u32 alloc_mask; pflag_t was; cpuid_t cpunum;
 unsigned int my_bit; u16 flags; struct cpu *caller;
 /* Ensure that the keep-core flag is set, so we can
  * safely compare `target' against the current CPU
  * without running the risk of being moved to another CPU.
  * This is necessary so we can safely bypass (slow) IPI
  * and just handle the command ourself if it's our own CPU. */
restart:
 flags = ATOMIC_FETCHOR(THIS_TASK->t_flags,TASK_FKEEPCORE);
 was = PREEMPTION_PUSHOFF();
 if (!X86_HAVE_LAPIC) {
  /* Special handling when we're running on a uni-core system. */
  if (also_send_to_self)
      x86_ipi_handle(ipi,true);
  PREEMPTION_POP(was);
  return;
 }
 /* Allocate an IPI on every target CPU. */
 caller = THIS_CPU;
 for (cpunum = 0; cpunum < cpu_count; ++cpunum) {
  struct cpu *target = cpu_vector[cpunum];
  if (target == caller) continue;
  do {
   alloc_mask = ATOMIC_LOAD(FORCPU(target,ipi_alloc));
   if unlikely(alloc_mask == 0xffffffff) {
    /* Go do something else and try again when the CPU has handled other pending IPIs.
     * NOTE: Getting here is highly unlikely, unless that CPUs is being bombarded with IPIs. */
    PREEMPTION_POP(was);
    if (!(flags&TASK_FKEEPCORE))
          ATOMIC_FETCHAND(THIS_TASK->t_flags,~TASK_FKEEPCORE);
    task_tryyield();
    goto restart;
   }
   /* Find the first, free IPI slot. */
   my_bit = 0;
   while (alloc_mask & (1 << my_bit)) ++my_bit;
   /* Allocate an IPI slot. */
  } while (!ATOMIC_CMPXCH_WEAK(FORCPU(target,ipi_alloc),
                               alloc_mask,alloc_mask | (1 << my_bit)));

  /* Copy IPI data from the caller-given buffer. */
  memcpy(&FORCPU(target,ipi_buffer[my_bit]),ipi,sizeof(struct x86_ipi));

  /* Mark the IPI as valid.
   * NOTE: This 2-step allocation is required to prevent a
   *       race that could arise when the target CPU is already
   *       handling IPIs, and starts processing ours before
   *       we've finished writing out its data. */
  ATOMIC_FETCHOR(FORCPU(target,ipi_valid),1 << my_bit);
 }

 /* Send an IPI to all CPUs other than the calling
  * NOTE: If the caller wants to execute the IPI too, we do so manually. */
 lapic_write(APIC_ICR0,
             X86_INTERRUPT_APIC_IPI |
             APIC_ICR0_TYPE_FNORMAL |
             APIC_ICR0_DEST_PHYSICAL |
             APIC_ICR0_FASSERT |
             APIC_ICR0_TARGET_FOTHERS);

 /* Execute the IPI on the calling CPU. */
 if (also_send_to_self)
     x86_ipi_handle(ipi,true);
 PREEMPTION_POP(was);
 if (!(flags&TASK_FKEEPCORE))
       ATOMIC_FETCHAND(THIS_TASK->t_flags,~TASK_FKEEPCORE);
}





/* Lock that must be held while suspending other CPUs. */
PUBLIC mutex_t x86_ipi_suspension_lock = MUTEX_INIT;

/* Safely begin/end a section of core that can only be executed by a single CPU:
 *   - Acquire a lock to `x86_ipi_suspension_lock'
 *   - Broadcast a `X86_IPI_SUSPEND_CPU' IPI to all other CPUs
 *   - Wait for the IPI to be acknowledged (`x86_cpu_suspended'
 *     must become set in all CPUs other than the caller's)
 * NOTE: The caller must set the `TASK_FKEEPCORE' bit in the calling thread.
 * @throw: E_INTERRUPT: The calling thread was interrupted. */
PUBLIC bool KCALL x86_unicore_begin(void) {
 struct x86_ipi ipi; cpuid_t cpunum;
 struct cpu *caller; unsigned int num_pending;
 assertf(THIS_TASK->t_flags & TASK_FKEEPCORE,
         "You must set the `TASK_FKEEPCORE' flag before calling `x86_unicore_begin()'");
 if (!mutex_try(&x86_ipi_suspension_lock))
      return false;
 ipi.ipi_type = X86_IPI_SUSPEND_CPU;
 ipi.ipi_flag = X86_IPI_FNORMAL;
 /* Broadcast our request for all other CPUs to suspend execution. */
 x86_ipi_broadcast(&ipi,false);
 /* Wait for all CPUs to become suspended. */
 caller = THIS_CPU;
 for (;;) {
  num_pending = 0;
  for (cpunum = 0; cpunum < cpu_count; ++cpunum) {
   struct cpu *other_cpu = cpu_vector[cpunum];
   if (other_cpu == caller) continue;
   COMPILER_READ_BARRIER();
   if (FORCPU(other_cpu,x86_cpu_suspended) <= 0)
       ++num_pending;
  }
  if (!num_pending) break;
  /* Wait until all other CPUs have been suspended. */
  task_tryyield();
 }
 return true;
}

/* End a unicore section of code:
 *   - Broadcast a `X86_IPI_RESUME_CPU' IPI to all other CPUs
 *   - Release the lock from `x86_ipi_suspension_lock'. */
PUBLIC void KCALL x86_unicore_end(void) {
 struct x86_ipi ipi;
 ipi.ipi_type = X86_IPI_RESUME_CPU;
 ipi.ipi_flag = X86_IPI_FNORMAL;
 /* Broadcast our request for all other CPUs to resume execution. */
 x86_ipi_broadcast(&ipi,false);
 mutex_put(&x86_ipi_suspension_lock);
}



DECL_END
#endif /* !CONFIG_NO_SMP */

#endif /* !GUARD_KERNEL_I386_KOS_IPI_C */
