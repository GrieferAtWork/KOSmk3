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
#ifndef GUARD_KERNEL_I386_KOS_SCHEDULER_C
#define GUARD_KERNEL_I386_KOS_SCHEDULER_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1
#define _XOPEN_SOURCE 1
#define _XOPEN_SOURCE_EXTENDED 1

#include "scheduler.h"

#include <bits/signum.h>
#include <hybrid/align.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kos/types.h>
#include <unwind/eh_frame.h>
#include <unwind/linker.h>
#include <i386-kos/gdt.h>
#include <i386-kos/pic.h>
#include <i386-kos/scheduler.h>
#include <kernel/interrupt.h>
#include <kernel/debug.h>
#include <kernel/paging.h>
#include <kernel/vm.h>
#include <kernel/user.h>
#include <kernel/syscall.h>
#include <fs/linker.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <sched/posix_signals.h>
#include <sched/userstack.h>
#include <sched/group.h>
#include <sched/pid.h>
#include <sched/stat.h>
#include <sched/affinity.h>
#include <fs/path.h>
#include <fs/handle.h>
#include <sys/io.h>
#include <bits/sigaction.h>

#include <asm/cpu-flags.h>
#include <stdint.h>
#include <except.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <alloca.h>

#ifndef CONFIG_NO_SMP
#include <i386-kos/ipi.h>
#endif

DECL_BEGIN

STATIC_ASSERT(offsetof(struct cpu,c_running) == CPU_OFFSETOF_RUNNING);
STATIC_ASSERT(offsetof(struct cpu,c_sleeping) == CPU_OFFSETOF_SLEEPING);
#ifndef CONFIG_NO_SMP
STATIC_ASSERT(offsetof(struct cpu,c_pending) == CPU_OFFSETOF_PENDING);
#endif
STATIC_ASSERT(sizeof(struct cpu) == CPU_OFFSETOF_DATA);

#ifndef CONFIG_NO_SMP
PUBLIC cpuid_t _cpu_count ASMNAME("cpu_count") = 1;
PUBLIC struct cpu *_cpu_vector[CONFIG_MAX_CPU_COUNT] ASMNAME("cpu_vector") = { &_boot_cpu };
#else
PUBLIC cpuid_t const cpu_count = 1;
PUBLIC struct cpu *const cpu_vector[CONFIG_MAX_CPU_COUNT] = { &_boot_cpu };
#endif


INTERN ATTR_SECTION(".data.percpu.head")
struct cpu cpu_template = {
     .cpu_id    = 0, /* Boot CPU. */
     .c_running = &_boot_task,
#ifndef CONFIG_NO_SMP
     .c_pending = NULL,
#endif
};

INTDEF byte_t x86_boot_stack[];
INTDEF byte_t x86_boot_stack_top[];

typedef void (KCALL *task_func_t)(struct task *__restrict thread);
INTDEF task_func_t pertask_init_start[];
INTDEF task_func_t pertask_init_end[];


INTERN ATTR_FREETEXT void KCALL
x86_scheduler_initialize(void) {
 task_func_t *iter;
 assert(!_boot_task.t_vm);
 assert(!vm_kernel.vm_tasks);
 assert(!_boot_task.t_vmtasks.le_pself);
 assert(!_boot_task.t_vmtasks.le_next);
#ifndef CONFIG_NO_SMP
 assert(_boot_task.t_cpu == &_boot_cpu);
#endif /* !CONFIG_NO_SMP */
 /* Setup some additional hooks for the boot
  * tasks that are not pre-defined by the template. */
 vm_kernel.vm_physdir                                                        = (vm_phys_t)(uintptr_t)&pagedir_kernel_phys;
 vm_kernel.vm_tasks                                                          = &_boot_task;
 _boot_task.t_segment.ts_self                                                = &_boot_task.t_segment;
 _boot_task.t_vm                                                             = &vm_kernel;
 _boot_task.t_vmtasks.le_pself                                               = &vm_kernel.vm_tasks;
 _boot_task.t_stackmin                                                       = x86_boot_stack;
 _boot_task.t_stackend                                                       = x86_boot_stack_top;
 _boot_task.t_sched.sched_ring.re_next                                       = &_boot_task;
 _boot_task.t_sched.sched_ring.re_prev                                       = &_boot_task;
 FORTASK(&_boot_task,_this_fs)                                               = &fs_kernel;
 FORTASK(&_boot_task,_this_handle_manager)                                   = &handle_manager_kernel;
 FORTASK(&_boot_task,_this_group).tg_leader                                  = &_boot_task;
 FORTASK(&_boot_task,_this_group).tg_process.h_procgroup.pg_leader           = &_boot_task;
 FORTASK(&_boot_task,_this_group).tg_process.h_procgroup.pg_master.m_session = &_boot_task;

 /* Create references for pointers we've just created. */
 vm_incref(&vm_kernel);                         /* `_boot_task.t_vm' */
 fs_incref(&fs_kernel);                         /* `FORTASK(&_boot_task,_this_fs)' */
 handle_manager_incref(&handle_manager_kernel); /* `FORTASK(&_boot_task,_this_handle_manager)' */

 /* Execute initializers. */
 for (iter = pertask_init_start;
      iter < pertask_init_end; ++iter)
     (**iter)(&_boot_task);
}


/* Safely check for pending tasks before
 * idling until the next hardware interrupt.
 * If another task can be reawoken, it is woken before returning.
 * NOTE: The caller must disable preemption beforehand.
 * NOTE: This function will enable preemption before returning. */
INTDEF void KCALL x86_cpu_idle(void);

/* NOTE: Must be called with interrupts disabled.
 * NOTE: Will return with interrupts enabled.
 * Save the caller's CPU context into the current task,
 * then proceed to load a new task to switch to from
 * `THIS_CPU->c_running' and start executing it. */
INTDEF void FCALL x86_exchange_context(void);

PRIVATE NOIRQ void KCALL
x86_scheduler_addsleeper(struct task *__restrict thread) {
 struct task **pinsert,*insert;
 jtime_t thread_timeout = thread->t_timeout;
 pinsert = &THIS_CPU->c_sleeping;
 while ((insert = *pinsert) != NULL) {
  assert(insert->t_sched.sched_list.le_pself == pinsert);
  assert(insert->t_state & TASK_STATE_FSLEEPING);
  /* If the timeout of `insert' is `>' our timeout, insert `thread' before it.
   * Technically, we could check for `>=', but this way we implement a minor
   * priority improvement, where tasks that began sleeping first will be
   * re-scheduled prior to others (XXX: I think the re-scheduling algorithm
   * currently inverts the order when multiple threads was for the same
   * timeout. - Maybe fix that?). */
  if (insert->t_timeout > thread_timeout)
      break;
  pinsert = &insert->t_sched.sched_list.le_next;
 }
 /* Insert `thread' before `insert' / after `pinsert' */
 if ((thread->t_sched.sched_list.le_next = insert) != NULL)
      insert->t_sched.sched_list.le_pself = &thread->t_sched.sched_list.le_next;
 *(thread->t_sched.sched_list.le_pself = pinsert) = thread;
}

#ifndef CONFIG_NO_SMP

/* Called by the `X86_IPI_WAKETASK' IPI */
INTERN NOIRQ bool KCALL
x86_scheduler_localwake(struct task *__restrict thread) {
 /* Simply transfer the thread from the sleeping list, to the running ring.
  * However, don't do anything if the task wasn't sleeping before. */
 if (thread->t_state & TASK_STATE_FSLEEPING) {
  LIST_REMOVE(thread,t_sched.sched_list);
#if 1 /* Woken threads are scheduled with high priority to improve responsiveness. */
  RING_INSERT_AFTER(THIS_CPU->c_running,thread,
                    t_sched.sched_ring);
#else
  RING_INSERT_BEFORE(THIS_CPU->c_running,thread,
                     t_sched.sched_ring);
#endif
  thread->t_state &= ~TASK_STATE_FSLEEPING;
  return true;
 }
 return false;
}

INTERN NOIRQ bool KCALL
x86_scheduler_localwake_p(struct task *prev,
                          struct task *__restrict thread) {
 /* Simply transfer the thread from the sleeping list, to the running ring.
  * However, don't do anything if the task wasn't sleeping before. */
 if (thread->t_state & TASK_STATE_FSLEEPING) {
  LIST_REMOVE(thread,t_sched.sched_list);
  if (!prev || prev->t_cpu != THIS_CPU)
       prev = THIS_CPU->c_running;
  RING_INSERT_AFTER(prev,thread,t_sched.sched_ring);
  ATOMIC_FETCHAND(thread->t_state,~TASK_STATE_FSLEEPING);
  return true;
 } else if (prev && prev->t_cpu == THIS_CPU &&
          !(prev->t_state & TASK_STATE_FSLEEPING)) {
  /* Re-insert `thread' after `prev' */
  RING_REMOVE(thread,t_sched.sched_ring);
  RING_INSERT_AFTER(prev,thread,t_sched.sched_ring);
 }
 return false;
}

INTERN NOIRQ void KCALL
x86_scheduler_loadpending_chain(struct task *chain) {
 struct cpu *me = THIS_CPU;
 struct task *next;
 /* Transfer all pending task to the list of running tasks. */
 while (chain) {
  next = chain->t_sched.sched_list.le_next;
  if (chain->t_state & TASK_STATE_FSLEEPING) {
   /* Start as sleeping (could be used to implement start-as-suspended). */
   x86_scheduler_addsleeper(chain);
  } else {
   RING_INSERT_BEFORE(me->c_running,chain,t_sched.sched_ring);
  }
  chain = next;
 }
}

/* Called by the `X86_IPI_SCHEDULE' IPI */
INTERN NOIRQ void KCALL x86_scheduler_loadpending(void) {
 struct cpu *me = THIS_CPU;
 struct task *chain,*next;

 /* Extract the list of pending tasks. */
 chain = ATOMIC_XCH(me->c_pending,NULL);

 /* Transfer all pending task to the list of running tasks. */
 while (chain) {
  next = chain->t_sched.sched_slist.le_next;
  if (ATOMIC_FETCHOR(chain->t_state,TASK_STATE_FSTARTED) & TASK_STATE_FSLEEPING) {
   /* Start as sleeping (could be used to implement start-as-suspended). */
   x86_scheduler_addsleeper(chain);
  } else {
   /* XXX: What if the `TASK_STATE_FTERMINATED' flag had already been set.
    *      _we_ can't be held responsible to clean up the reference, and
    *      neither can we send an RPC to have someone else do it for us.
    *   >> So what do we do then? */
   RING_INSERT_BEFORE(me->c_running,chain,t_sched.sched_ring);
  }
  chain = next;
 }
}
#endif /* !CONFIG_NO_SMP */

/* The original return location for user-space threads
 * that were pre-empted while in kernel-space.
 * [lock(NOIRQ,PRIVATE(THIS_CPU))] */
INTERN ATTR_PERTASK struct x86_irregs_user iret_saved;

/* Entry point of an assembly function that will save
 * user-space registers, serve RPC functions, and return to
 * user-space by loading registers from `PERTASK(iret_saved)'
 * WARNING: The address of this function is used to identify
 *          a user-space redirected thread, meaning that
 *          you must not set this IP as an override in the
 *          register state returned by `x86_interrupt_getiret()'! */
INTDEF NOIRQ void ASMCALL x86_redirect_preemption(void);
#define REAL_IRET() ((struct x86_irregs_user *)PERTASK_GET(this_task.t_stackend)-1)


PUBLIC NOIRQ ATTR_RETNONNULL
struct x86_irregs_user *KCALL x86_interrupt_getiret(void) {
 struct x86_irregs_user *iret = REAL_IRET();
 assertf(!PREEMPTION_ENABLED(),
         "Preemption must be disabled to "
         "prevent sporadic modifications");
 assertf(!PERTASK_TESTF(this_task.t_flags,TASK_FKERNELJOB),
         "Kernel worker threads never return to user-space");
#ifdef CONFIG_VM86
 if (PERTASK_TESTF(this_task.t_flags,TASK_FVM86)) {
  /* Adjust for the additional fields saved by VM86 mode. */
  *(uintptr_t *)&iret -= (sizeof(struct x86_irregs_vm86)-
                          sizeof(struct x86_irregs_user));
 }
#endif
 if likely(iret->ir_eip != (uintptr_t)&x86_redirect_preemption)
    return iret; /* Not redirected. */
 /* The original IRET was saved before being overwritten. */
 assert(iret->ir_cs == X86_KERNEL_CS);
 return (struct x86_irregs_user *)&PERTASK(iret_saved);
}

INTERN NOIRQ void KCALL
x86_redirect_preempted_userspace(struct task *__restrict thread) {
 struct x86_irregs_user *iret;
#ifndef CONFIG_NO_SMP
 assert(thread->t_cpu == THIS_CPU);
#endif
 assert(!(thread->t_state & TASK_STATE_FSLEEPING));
 assert(!PREEMPTION_ENABLED());
 if (thread->t_flags & TASK_FKERNELJOB)
     return; /* Kernel jobs don't originate from user-space. */
 iret = (struct x86_irregs_user *)thread->t_stackend-1;
#ifdef CONFIG_VM86
 if (THIS_TASK->t_state&TASK_FVM86) {
  /* Adjust for the additional fields saved by VM86 mode. */
  *(uintptr_t *)&iret -= (sizeof(struct x86_irregs_vm86)-
                          sizeof(struct x86_irregs_user));
 }
#endif

 /* We can't return to user-space by throwing an interrupt
  * exception because this RPC is called asynchronously.
  * Instead, we must assume the presence of an IRET tail
  * at the base of our stack and modify that to return to a
  * kernel-space wrapper that will invoke `task_serve_before_user()'.
  * However, we must also be careful in the event that we've
  * already done this, in which case we must return as a no-op.
  * NOTE: Synchronization is guarantied by the fact that interrupts
  *       are disabled and any CPU can only do this for _ITS_ threads.
  * WARNING: Because of this hack, new user-space threads must be constructed
  *          with a valid user-space IRET tail at the base of their stack! */
 if (iret->ir_eip == (uintptr_t)&x86_redirect_preemption)
     return; /* Already redirected. */
#if 0
 debug_printf("REDIRECT_USERSPACE @ %p.%p:{ %p, %p, %p, %p, %p }\n",
              thread,iret,
              iret->ir_eip,
              iret->ir_cs,
              iret->ir_eflags,
              iret->ir_useresp,
              iret->ir_ss);
 asm("int3");
#endif
 /* Save the original IRET */
 memcpy(&FORTASK(thread,iret_saved),iret,
         sizeof(struct x86_irregs_user));
 /* Redirect to our kernel-space wrapper, so we always serve interrupts. */
 iret->ir_cs      = X86_KERNEL_CS;
#ifdef __x86_64__
 iret->ir_rflags  = 0; /* Disable interrupts on entry. */
 iret->ir_rip     = (uintptr_t)&x86_redirect_preemption;
 iret->ir_userrsp = thread->t_stackend;
#ifdef CONFIG_X86_SEGMENTATION
 iret->ir_ss      = X86_KERNEL_DS;
#else
 iret->ir_ss      = X86_USER_DS;
#endif
#else
 iret->ir_eflags  = 0; /* Disable interrupts on entry. */
 iret->ir_eip     = (uintptr_t)&x86_redirect_preemption;
 /* These registers aren't restored if we got here from user-space.
  * On 32-bit, `x86_redirect_preemption()' is
  * entered with `ESP = thread->t_stackend - 8' */
 /*iret->ir_useresp = ...;*/
 /*iret->ir_ss      = ...;*/
#endif
}


PUBLIC void FCALL
task_restart_interrupt(struct cpu_anycontext *__restrict context) {
again:
 assertf(error_code() == E_INTERRUPT,
         "Only call this function from a CATCH(E_INTERRUPT) block");
 if unlikely(!PREEMPTION_ENABLED())
    goto rethrow; /* Caller can't originate from user-space if they neglected to re-enable interrupts. */
 /* Check if the context returns to user-space. */
 if (X86_ANYCONTEXT32_ISUSER(*context))
     goto do_serve;
 /* Even if it doesn't, there's still the whole thing about `x86_redirect_preemption' */
 if (context->c_eip != (uintptr_t)&x86_redirect_preemption)
     goto rethrow_p;
 assertf(!PERTASK_TESTF(this_task.t_flags,TASK_FKERNELJOB),
         "How did a kernel job manage to redirect its preemption? "
         "Also from what should that preemption have been redirected?");
 /* Ok. So we got here as the result of a preemption redirection (`task_wake_for_rpc()'),
  * meaning that at some point the calling interrupt must have been waiting for some
  * kind of lock, before it got interrupted when some other thread scheduled an RPC.
  * Our job now is to restore the saved IRET tail and act as though we weren't waiting
  * at all, instead opting to handle the RPC function the same way we would if
  * preemption was never redirected. */
 assertf(&context->c_iret == REAL_IRET(),
         "Only the directed CPU context could point at `x86_redirect_preemption'. "
         "This means that the given context must be the real IRET tail");
 /* Restore the saved IRET tail. */
 memcpy(&context->c_iret,&PERTASK(iret_saved),
         sizeof(struct x86_irregs_user));
do_serve:
 PREEMPTION_ENABLE();
 TRY {
#if 0
  error_printf("TASK_RESTART_INTERRUPT()\n");
#endif
  /* Since the interrupt will be restarted once RPC
   * functions have been served, semantically speaking,
   * the user-space CPU context is the same as if the
   * RPC was being served while the thread was still
   * in user-space, about to trigger the interrupt that
   * was interrupted in order to serve RPC functions. */
  task_serve_before_user(&context->c_user,
                          TASK_USERCTX_TYPE_WITHINUSERCODE);
 } CATCH (E_INTERRUPT) {
  /* Deal with recursive restart attempts. */
  goto again;
 }
 return;
rethrow_p:
 PREEMPTION_ENABLE();
rethrow:
 error_rethrow();
}

/* Same as `task_restart_interrupt()', but meant to be called from a
 * `CATCH(E_INTERRUPT)' block surrounding a system call invocation.
 * When these functions return normally, the system call should be restarted.
 * When it shouldn't, these functions rethrow the dangling `E_INTERRUPT'. */
PUBLIC bool FCALL
task_restart_syscall(struct cpu_hostcontext_user *__restrict context,
                     unsigned int mode, syscall_ulong_t sysno) {
again:
 assertf(error_code() == E_INTERRUPT,
         "Only call this function from a CATCH(E_INTERRUPT) block");
 assertf(PREEMPTION_ENABLED(),
         "Preemption must be enabled during execution of a system call");
 /* Check if the context returns to user-space. */
 assertf(X86_ANYCONTEXT32_ISUSER(*context) ||
         context->c_eip != (uintptr_t)&x86_redirect_preemption,
         "Only user-space must be allowed to execute system calls");
 assertf(!PERTASK_TESTF(this_task.t_flags,TASK_FKERNELJOB),
         "How did a kernel job manage to redirect its preemption? "
         "Also from what should that preemption have been redirected?");
 assert((mode & TASK_USERCTX_TYPE_FMASK) == TASK_USERCTX_TYPE_INTR_SYSCALL);
 if (context->c_eip == (uintptr_t)&x86_redirect_preemption) {
  /* Ok. So we got here as the result of a preemption redirection (`task_wake_for_rpc()'),
   * meaning that at some point the calling interrupt must have been waiting for some
   * kind of lock, before it got interrupted when some other thread scheduled an RPC.
   * Our job now is to restore the saved IRET tail and act as though we weren't waiting
   * at all, instead opting to handle the RPC function the same way we would if
   * preemption was never redirected. */
  assertf(&context->c_iret == REAL_IRET(),
          "Only the directed CPU context could point at `x86_redirect_preemption'. "
          "This means that the given context must be the real IRET tail");
  /* Restore the saved IRET tail. */
  memcpy(&context->c_iret,&PERTASK(iret_saved),
          sizeof(struct x86_irregs_user));
 }
 TRY {
#if 0
  error_printf("TASK_RESTART_SYSCALL()\n");
#endif
  /* Since the system call will be restarted once RPC
   * functions have been served, semantically speaking,
   * the user-space CPU context is the same as if the
   * RPC was being served while the thread was still
   * in user-space, about to trigger the system call that
   * was interrupted in order to serve RPC functions. */
  task_serve_before_user(context,mode);
 } CATCH (E_INTERRUPT) {
  /* Deal with recursive restart attempts. */
  goto again;
 }
 /* If none of the RPC functions overwrote the E_INTERRUPT exception,
  * or if the system call shouldn't be restarted, don't restart the
  * system call. Otherwise, do. */
 if (error_code() != E_INTERRUPT)
     return false;
 return should_restart_syscall(sysno,SHOULD_RESTART_SYSCALL_FNORMAL);
}





PRIVATE ASYNCSAFE ATTR_NOTHROW bool KCALL
task_wake_ex(struct task *__restrict thread,
             unsigned int mode) {
#ifdef CONFIG_NO_SMP
 bool result = true;
 pflag_t was = PREEMPTION_PUSHOFF();
 if (TASK_ISTERMINATED(thread))
  result = false;
 else if (thread->t_state & TASK_STATE_FSLEEPING) {
  LIST_REMOVE(thread,t_sched.sched_list);
  RING_INSERT_BEFORE(THIS_CPU->c_running,thread,
                     t_sched.sched_ring);
 } else if (mode == X86_IPI_WAKETASK_FOR_RPC) {
  x86_redirect_preempted_userspace(thread);
 }
 PREEMPTION_POP(was);
 return result;
#else
 volatile int status;
 for (;;) {
  struct cpu *hosting_cpu;
  struct x86_ipi ipi;
  status = X86_IPI_WAKETASK_PENDING;
  hosting_cpu = ATOMIC_READ(thread->t_cpu);
  /* Setup an IPI to wake this task. */
  ipi.ipi_type           = mode;
  ipi.ipi_flag           = X86_IPI_FNORMAL;
  ipi.ipi_wake.wt_task   = thread;
  ipi.ipi_wake.wt_status = &status;

  /* Send the IPI */
  x86_ipi_send(hosting_cpu,&ipi);

  /* Wait for the IPI to be processed. */
  while (status == X86_IPI_WAKETASK_PENDING) {
   /* Use `tryyield()' here because we can't be sure
    * if the caller has left interrupts enabled for us. */
   task_tryyield();
  }

  /* Check if the IPI succeeded. */
  if (status != X86_IPI_WAKETASK_RETRY)
      break;
 }
 return status != X86_IPI_WAKETASK_DEAD;
#endif
}

PUBLIC ASYNCSAFE ATTR_NOTHROW bool KCALL
task_wake_for_rpc(struct task *__restrict thread) {
 return task_wake_ex(thread,X86_IPI_WAKETASK_FOR_RPC);
}

PUBLIC ASYNCSAFE ATTR_NOTHROW bool KCALL
task_wake(struct task *__restrict thread) {
 return task_wake_ex(thread,X86_IPI_WAKETASK);
}


#ifndef CONFIG_NO_SMP
PUBLIC bool KCALL
task_wake_p(REF struct task *prev_threads[/*CONFIG_MAX_CPU_COUNT*/],
            struct task *__restrict next) {
 volatile int status;
 cpuid_t hosting_id;
 /* Keep a reference for the `prev_threads' vector. */
 task_incref(next);
 for (;;) {
  struct cpu *hosting_cpu;
  struct x86_ipi ipi;
  status = X86_IPI_WAKETASK_PENDING;
  hosting_cpu = ATOMIC_READ(next->t_cpu);
  /* Setup an IPI to wake this task. */
  ipi.ipi_type             = X86_IPI_WAKETASK_P;
  ipi.ipi_flag             = X86_IPI_FNORMAL;
  hosting_id               = hosting_cpu->cpu_id;
  ipi.ipi_wake_p.wt_prev   = prev_threads[hosting_id];
  ipi.ipi_wake_p.wt_task   = next;
  ipi.ipi_wake_p.wt_status = &status;

  /* Send the IPI */
  x86_ipi_send(hosting_cpu,&ipi);

  /* Wait for the IPI to be processed. */
  while (status == X86_IPI_WAKETASK_PENDING) {
   /* Use `tryyield()' here because we can't be sure
    * if the caller has left interrupts enabled for us. */
   task_tryyield();
  }

  /* Check if the IPI succeeded. */
  if (status != X86_IPI_WAKETASK_RETRY)
      break;
 }
 if (status == X86_IPI_WAKETASK_DEAD) {
  task_decref(next);
  return false;
 }
 /* Update the prev-thread vector. */
 if (prev_threads[hosting_id])
     task_decref(prev_threads[hosting_id]);
 prev_threads[hosting_id] = next; /* Inherit reference. */
 return true;
}
#endif /* !CONFIG_NO_SMP */



PUBLIC NOIRQ ATTR_HOTTEXT bool KCALL task_sleep(jtime_t abs_timeout) {
 struct task *caller = THIS_TASK;
 assert(!PREEMPTION_ENABLED());
 /* Timeout immediately for `JTIME_DONTWAIT' */
 if unlikely(abs_timeout == JTIME_DONTWAIT) {
  PREEMPTION_ENABLE();
  return false;
 }
 assertf(!(caller->t_state&TASK_STATE_FSLEEPING),
         "caller->t_state = %x\n",caller->t_state);
 /* Remove the task for the running-set. */
 assert((caller->t_sched.sched_ring.re_prev == caller) ==
        (caller->t_sched.sched_ring.re_next == caller));
 if unlikely(caller->t_sched.sched_ring.re_next == caller) {
  /* Special case: nothing to switch to.
   * This really shouldn't happen unless a custom scheduler loop
   * is constructed that doesn't contain a regular IDLE task.
   * But since dealing with this isn't actually that hard, I don't
   * want to add some special exception that would make this illegal. */
  /* XXX: Disable PIT interrupts and clear the associated
   *      interrupt PIN (because it's likely set right now)
   *      That way, we could safe up on some energy when we
   *      don't get constant awoken by the PIT clock, only
   *      to find that there is no other task to switch to. */
  x86_cpu_idle();
  return (abs_timeout > jiffies ||
          abs_timeout == JTIME_INFINITE);
 }
 THIS_CPU->c_running = caller->t_sched.sched_ring.re_next;
 assert(THIS_CPU->c_running);
 RING_REMOVE(caller,t_sched.sched_ring);

 /* Enter a sleeping-task state. */
 INCSTAT(ts_sleep);
 assertf(caller->t_state & TASK_STATE_FSTARTED,"caller = %p",caller);
 caller->t_state  |= TASK_STATE_FSLEEPING;
 caller->t_timeout = abs_timeout;
 COMPILER_WRITE_BARRIER();
 x86_scheduler_addsleeper(caller);

 /* Switch CPU context to `THIS_CPU->c_running' */
 COMPILER_BARRIER();
 x86_exchange_context();
 COMPILER_BARRIER();

 /* Once re-scheduled, the task must no longer be sleeping. */
 assertf(THIS_TASK == caller,
         "THIS_TASK        = %p\n"
         "caller           = %p\n"
         "TLS_SEGMENT_BASE = %p\n",THIS_TASK,caller,
         X86_SEGMENT_GTBASE(FORCPU(&_boot_cpu,x86_cpugdt[X86_SEG_HOST_TLS])));
 assert(!(caller->t_state&TASK_STATE_FSLEEPING));
 /* Test the timed-out flag. */
 if (caller->t_state&TASK_STATE_FTIMEDOUT) {
  ATOMIC_FETCHAND(caller->t_state,~TASK_STATE_FTIMEDOUT);
  /* Don't indicate a timeout if the given timeout is infinite. */
  return abs_timeout == JTIME_INFINITE;
 }
 return true;
}


#ifndef CONFIG_NO_SMP
INTDEF ATTR_PERTASK kernel_cpuset_t _this_affinity;
INTDEF ATTR_PERTASK atomic_rwlock_t _this_affinity_lock;

PUBLIC int KCALL
task_setcpu_impl(struct task *__restrict thread,
                 struct cpu *__restrict new_cpu,
                 bool overrule_affinity) {
 struct cpu *old_cpu;
 struct x86_ipi ipi;
 struct task *next_task;
 volatile int status;
 if (!overrule_affinity) {
  atomic_rwlock_read(&FORTASK(thread,_this_affinity_lock));
  /* Check affinity */
  if (!kernel_cpuset_has(FORTASK(thread,_this_affinity),
                         new_cpu->cpu_id)) {
   atomic_rwlock_endread(&FORTASK(thread,_this_affinity_lock));
   return TASK_SETCPU_FAIL;
  }
 }
 for (;;) {
  old_cpu = ATOMIC_READ(thread->t_cpu);
  if (old_cpu == new_cpu) {
   if (!overrule_affinity)
        atomic_rwlock_endread(&FORTASK(thread,_this_affinity_lock));
   return TASK_SETCPU_OK; /* Nothing to do here! */
  }
  ipi.ipi_type                 = X86_IPI_UNSCHEDULE;
  ipi.ipi_flag                 = X86_IPI_FNORMAL;
  ipi.ipi_unschedule.us_thread = thread;
  ipi.ipi_unschedule.us_status = &status;
  status                       = X86_IPI_UNSCHEDULE_PENDING;
  /* Send the IPI. */
  x86_ipi_send(old_cpu,&ipi);
  /* Wait for the IPI to get acknowledged. */
  while (status == X86_IPI_UNSCHEDULE_PENDING)
     task_tryyield();
  if (status != X86_IPI_UNSCHEDULE_RETRY)
      break;
 }
 /* Deal with error status codes. */
 if (status != X86_IPI_UNSCHEDULE_OK) {
  if (!overrule_affinity)
       atomic_rwlock_endread(&FORTASK(thread,_this_affinity_lock));
  if (status == X86_IPI_UNSCHEDULE_KEEP)
      return TASK_SETCPU_AGAIN;
  return TASK_SETCPU_FAIL;
 }
 COMPILER_BARRIER();
 /* At this point, we own exclusive scheduling permissions for `thread',
  * meaning we're in charge of starting the thread on a different core! */
 thread->t_cpu = new_cpu;
 COMPILER_WRITE_BARRIER();

 /* Add the task to the pending-launch chain of the target CPU. */
 do thread->t_sched.sched_slist.le_next = next_task = ATOMIC_READ(new_cpu->c_pending);
 while (!ATOMIC_CMPXCH_WEAK(new_cpu->c_pending,
                            thread->t_sched.sched_slist.le_next,thread));
 if (!overrule_affinity)
      atomic_rwlock_endread(&FORTASK(thread,_this_affinity_lock));
 /* Setup an IPI to get the target CPU to load its pending task list. */
 ipi.ipi_type = X86_IPI_SCHEDULE;
 ipi.ipi_flag = X86_IPI_FNORMAL;

 /* Send an IPI to the target CPU. */
 x86_ipi_send(new_cpu,&ipi);

 return TASK_SETCPU_OK;
}
#endif /* !CONFIG_NO_SMP */


PUBLIC REF struct task *KCALL
task_runnext(struct task *__restrict prev,
             struct task *__restrict next) {
 struct task *result;
 pflag_t was;
 was = PREEMPTION_PUSHOFF();
 if (prev->t_cpu != THIS_CPU)
  result = THIS_TASK;
 else {
  if (prev->t_state & TASK_STATE_FSLEEPING)
      prev = THIS_TASK;
  if (prev->t_cpu != next->t_cpu ||
     (next->t_state & TASK_STATE_FSLEEPING))
   result = prev;
  else {
   /* Everything checks out. - Do the rescheduling. */
   result = next;
   /* Remove `next' from its current scheduler chain. */
   next->t_sched.sched_ring.re_prev->t_sched.sched_ring.re_next = next->t_sched.sched_ring.re_next;
   next->t_sched.sched_ring.re_next->t_sched.sched_ring.re_prev = next->t_sched.sched_ring.re_prev;
   /* Insert `next' after `prev'. */
   next->t_sched.sched_ring.re_next = prev->t_sched.sched_ring.re_next;
   next->t_sched.sched_ring.re_next->t_sched.sched_ring.re_prev = next;
   prev->t_sched.sched_ring.re_next = next;
   next->t_sched.sched_ring.re_prev = prev;
  }
 }
 /* Return a reference to prevent a race condition of the `result'
  * thread exiting before, or while the caller is using it. */
 task_incref(result);
 PREEMPTION_POP(was);
 return result;
}



/* The kernel-space equivalent of the `ts_x86sysbase'
 * field found in the user-space per-task data segment.
 * Used to implement `sigreturn' without the need of mapping additional
 * code into user-space, or hiding that code in some USHARE segment... */
PUBLIC ATTR_PERTASK uintptr_t x86_sysbase = KERNEL_BASE;
DEFINE_PERTASK_CLONE(task_clone_sysbase);
INTERN void KCALL
task_clone_sysbase(struct task *__restrict new_thread, u32 flags) {
 if (flags & CLONE_VM)
  /* Create a new SYSBASE address if the VM is shared. */
  FORTASK(new_thread,x86_sysbase) = X86_SYSBASE_RAND();
 else {
  /* If a new VM will be created (as done for `fork()'), keep
   * the same sysbase address so we're not forced to fault the
   * page containing the user-space task segment by writing to
   * it even before returning to user-space. */
  FORTASK(new_thread,x86_sysbase) = PERTASK_GET(x86_sysbase);
 }
}



INTERN void FCALL
task_setup_kernel_environ(struct task *__restrict thread) {
 /* Setup kernel environment components of the thread. */
 FORTASK(thread,_this_fs)                                               = &fs_kernel;
 FORTASK(thread,_this_handle_manager)                                   = &handle_manager_kernel;
 FORTASK(thread,_this_group).tg_leader                                  = thread; /* Own process leader. (`CLONE_THREAD' not set) */
 FORTASK(thread,_this_group).tg_process.h_procgroup.pg_leader           = thread; /* Own process group leader. (`setpgid(0,0)' - style) */
 FORTASK(thread,_this_group).tg_process.h_procgroup.pg_master.m_session = thread; /* Own process sessions. (`setsid()' - style) */
 fs_incref(&fs_kernel);
 handle_manager_incref(&handle_manager_kernel);
 pidns_incref(&pidns_kernel);
 /* Generate a random base address for the sysbase segment. */
 FORTASK(thread,x86_sysbase) = X86_SYSBASE_RAND();
}


PUBLIC void KCALL
task_setup_kernel(struct task *__restrict thread,
                  task_main_t thread_main, void *arg) {
 struct cpu_context *context;
 task_alloc_stack(thread,CONFIG_KERNELSTACK_SIZE / PAGESIZE);
 thread->t_vm = &vm_kernel;
 vm_incref(&vm_kernel);

 /* Set the kernel filesystem to-be used by this thread. */
 task_setup_kernel_environ(thread);

#ifndef CONFIG_NO_SMP
 /* Keep cache locality high by spawning the
  * new thread on the same CPU as the caller.
  * NOTE: Prior to task_start(), the caller will
  *       still be able to change this again. */
 thread->t_cpu = THIS_CPU;
#endif

 /* Setup the thread's initial CPU state (arch-specific portion). */
 ((void **)thread->t_stackend)[-1] = arg;        /* `thread_main()' argument. */
 ((void **)thread->t_stackend)[-2] = &task_exit; /* `thread_main()' return address. */
 context = ((struct cpu_context *)((uintptr_t)thread->t_stackend - 2*sizeof(void *)))-1;
 thread->t_context = (struct cpu_anycontext *)context;
 memset(context,0,sizeof(struct cpu_context));
 context->c_eflags         = EFLAGS_IF;
 context->c_eip            = (uintptr_t)thread_main;
 context->c_iret.ir_cs     = X86_KERNEL_CS;
#ifdef CONFIG_X86_SEGMENTATION
 context->c_segments.sg_ds = X86_KERNEL_DS;
 context->c_segments.sg_es = X86_KERNEL_DS;
 context->c_segments.sg_fs = X86_SEG_FS;
 context->c_segments.sg_gs = X86_SEG_GS;
#endif /* CONFIG_X86_SEGMENTATION */

 /* Make sure to set the kernel-job flag in the thread. */
 thread->t_flags |= TASK_FKERNELJOB;
}

PUBLIC void KCALL
task_start(struct task *__restrict self) {
 assertf(self->t_refcnt >= 1,"Invalid task");
 assertf(self->t_context,"No CPU context assigned");
 assertf(self->t_stackmin != NULL,"No stack assigned");
 assertf(self->t_stackend > self->t_stackmin,"No stack assigned");
#ifndef CONFIG_NO_SMP
 assertf(self->t_cpu != NULL,"No CPU assigned");
#endif /* !CONFIG_NO_SMP */
 assertf(self->t_vm != NULL,"No VM assigned");
 assert(!self->t_vmtasks.le_pself);
 assert(!self->t_vmtasks.le_next);
 assertf(FORTASK(self,_this_fs) != NULL,
         "No THIS_FS defined for the thread");
 assertf(FORTASK(self,_this_handle_manager) != NULL,
         "No THIS_HANDLE_MANAGER defined for the thread");
 assertf(FORTASK(self,_this_group).tg_leader != NULL,
         "No THIS_GROUP->tg_leader defined for the thread");
 assertf(FORTASK(self,_this_group).tg_leader != self ||
         FORTASK(self,_this_group).tg_process.h_procgroup.pg_leader != NULL,
         "No THIS_GROUP->tg_process.h_procgroup.pg_leader defined for a thread-group leader");
 assertf(FORTASK(self,_this_group).tg_leader != self ||
         FORTASK(self,_this_group).tg_process.h_procgroup.pg_leader != self ||
         FORTASK(self,_this_group).tg_process.h_procgroup.pg_master.m_session != NULL,
         "No THIS_GROUP->tg_process.h_procgroup.pg_master.m_session defined for a process-group leader");

#ifndef NDEBUG
 /* Do some validation on where the thread will start. */
#ifdef CONFIG_VM86
 if (!(self->t_flags & TASK_FVM86))
#endif
 {
  struct x86_irregs_user *iret = (struct x86_irregs_user *)self->t_stackend-1;
  assertf(iret->ir_cs == X86_KERNEL_CS ||
        !(self->t_flags & TASK_FKERNELJOB),
         "iret->ir_cs = %p:%p",&iret->ir_cs,iret->ir_cs);
 }
#if 0 /* Can't be asserted. - If user-space doesn't follow this, it'll just get a GPF after launch. */
 assert((iret->ir_cs == X86_USER_CS) ? (iret->ir_eip < KERNEL_BASE) :
       ((iret->ir_eip >= KERNEL_BASE) || (self->t_vm == &vm_kernel)));
#endif
#endif

 /* Add the task to the chain of tasks using the associated VM. */
 atomic_rwlock_write(&self->t_vm->vm_tasklock);
 LIST_INSERT(self->t_vm->vm_tasks,self,t_vmtasks);
 atomic_rwlock_endwrite(&self->t_vm->vm_tasklock);

 /* Create the reference that is stored in the target cpu's scheduler chain. */
 task_incref(self);

#ifndef CONFIG_NO_TASKSTAT
 /* Save a timestamp of when the thread was started. */
 FORTASK(self,_this_stat).ts_started = jiffies;
#endif

 /* Start scheduling this task on its launch CPU. */
#ifndef CONFIG_NO_SMP
 {
  struct x86_ipi ipi;
  struct cpu *launch_cpu = self->t_cpu;
  struct task *next_task;
  /* Add the task to the pending-launch chain of the target CPU. */
  do self->t_sched.sched_slist.le_next = next_task = ATOMIC_READ(launch_cpu->c_pending);
  while (!ATOMIC_CMPXCH_WEAK(launch_cpu->c_pending,
                             self->t_sched.sched_slist.le_next,
                             self));

  /* Setup an IPI to get the target CPU to load its pending task list. */
  ipi.ipi_type = X86_IPI_SCHEDULE;
  ipi.ipi_flag = X86_IPI_FNORMAL;

  /* Send an IPI to the target CPU. */
  x86_ipi_send(launch_cpu,&ipi);
 }
#else
 {
  /* Simply add the task to the ring of running CPUs. */
  pflag_t was = PREEMPTION_PUSHOFF();
  if (ATOMIC_FETCHOR(self->t_state,TASK_STATE_FSTARTED) & TASK_STATE_FSLEEPING) {
   LIST_INSERT(_boot_cpu.c_sleeping,self,t_sched.sched_list);
  } else {
   RING_INSERT_BEFORE(_boot_cpu.c_running,
                      self,t_sched.sched_ring);
  }
  /* Set the started-flag before re-enabling interrupts. */
  PREEMPTION_POP(was);
 }
#endif
 /* The task is now scheduled and will/has start(ed) running. */

}





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

PRIVATE void KCALL task_decref_rpc(void *arg) {
 task_decref((struct task *)arg);
}

/* Per-thread variable: the join signal broadcast
 *                      immediately prior to termination. */
INTERN ATTR_PERTASK struct sig task_join_signal = SIG_INIT;

PUBLIC void KCALL
task_connect_join(struct task *__restrict other_task) {
 struct sig *joinsig;
 joinsig = &FORTASK(other_task,task_join_signal);

 /* Connect to the join-signal. */
 task_connect(joinsig);

 /* Now that we're receiving information about the task being terminated,
  * check if the task has already terminated (meaning that we may have
  * missed the join signal).
  * If we did miss it, simply set a signaled state using `joinsig'. */
 COMPILER_BARRIER();
 if (TASK_ISTERMINATED(other_task))
     task_setsignaled(joinsig);
}


#if 0
PRIVATE void KCALL print_here(void *arg) {
 TRY {
  error_throw(E_NOT_IMPLEMENTED);
 } EXCEPT (1) {
  error_printf("HERE\n");
 }
}
#endif

/* The last function ever called by any task. */
PUBLIC ATTR_NORETURN void KCALL task_exit(void) {
 struct cpu *calling_cpu;
 struct task *calling_task;
 struct task *next_task;
 PHYS uintptr_t old_pagedir;
 bool did_clear_vm = false;

 /* Ensure that preemption is enabled while we do this. */
 PREEMPTION_ENABLE();

 /* Disconnect arbitrary task signal connections that may still be active. */
 task_disconnect();

 /* Set the terminating flag to prevent new RPCs from being added. */
 ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FTERMINATING);
 while (PERTASK_TESTF(this_task.t_state,TASK_STATE_FINTERRUPTED)) {
  /* With the flag now set, serve all remaining RPCs. */
  TRY {
   /* XXX: This `task_serve()' will throw an `E_INTERRUPT' exception if
    *      there are any RPC callbacks left with the `TASK_RPC_USER' flag
    *      set. - How should we deal with those? */
   task_serve();
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   /* XXX: Seriously: What should we do now?
    *         I mean: We couldn't propagate the error, but
    *                 we might want to log it somewhere? */
   /* XXX: Free stack memory by restoring ESP to a value before `task_serve()' was called */
   error_printf("Error in task_serve() during task_exit()\n");
   if (error_code() == E_INTERRUPT)
       goto done_rpc; /* ??? (See problem above...) */
  }
 }

done_rpc:
 assert(PREEMPTION_ENABLED());

 {
  /* Invoke custom task cleanup functions. */
  typedef /*ATTR_NOTHROW*/void (KCALL *task_cleanup_t)(void);
  INTDEF task_cleanup_t pertask_cleanup_start[];
  INTDEF task_cleanup_t pertask_cleanup_end[];
  task_cleanup_t *iter;
  for (iter  = pertask_cleanup_start;
       iter != pertask_cleanup_end; ++iter)
       (**iter)();
 }

 /* Destroy the task's temporary virtual page. */
 if (PERTASK_GET(this_task.t_temppage) != VM_VPAGE_MAX+1)
     vm_unmap(PERTASK_GET(this_task.t_temppage),1,VM_UNMAP_NOEXCEPT,NULL);

 if (!PERTASK_TESTF(this_task.t_flags,TASK_FKERNELJOB)) {
  if (PERTASK_TESTF(this_task.t_flags,TASK_FOWNUSERSEG)) {
   /* Delete the user task segment. */
   assert(IS_ALIGNED((uintptr_t)PERTASK_GET(this_task.t_userseg),PAGEALIGN));
   assert(THIS_VM != &vm_kernel);
   vm_unmap(VM_ADDR2PAGE((uintptr_t)PERTASK_GET(this_task.t_userseg)),
            CEILDIV(sizeof(struct task_segment),PAGESIZE),
            VM_UNMAP_TAG|VM_UNMAP_NOEXCEPT|VM_UNMAP_SYNC,THIS_TASK);
  }
 }


 /* Set the terminated flag to indicate that the thread can now be joined. */
 ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FTERMINATED);
 /* Broadcast the join-signal to anyone is listening. */
 sig_broadcast(&PERTASK(task_join_signal));

 /*    #1   Disable preemption
  *    #2   Rotate to load the next task
  *    #2.1 If there is no next task, x86_cpu_idle() until there is one
  * --- SPLIT: In assembly
  *    #3   if (atomic_dec_if_not_one(THIS_TASK)) switch_to_next_task();
  * --- At this point, we could switch back to C.
  *     The previous part only needed to be in assembly because
  *     we need to be sure not to have to use any more stack
  *     memory in the event that the decref() succeeds, as in
  *     that case it would be a race condition to still be using
  *     a that that could be unmapped at any point when whatever
  *     other thread is still using our task descriptor finally
  *     does the decref() (which could be before we actually go away)
  *    #4   construct an RPC call for the next task to decref() our task
  *    #5   Disable preemption
  *    #6   Schedule the RPC call
  *    #7   Switch to the next task (the one we scheduled the RPC for)
  * Think back to how KOS Mk2 solved this using ~jobs~. Now, using RPCs, we
  * can solve the task-cleaning-up-after-itself problem much more easily. */

#if 0 /* Increase the likelihood of ending up with multiple terminated threads in a loop. */
 {
  unsigned int i;
  for (i = 0; i < 200; ++i)
       task_yield();
 }
#endif

restart_final_decref:
 for (;;) {
  PREEMPTION_DISABLE();
  /* If this is the last task of the calling CPU, we
   * must wait until more tasks show up (from other CPUs). */
  if unlikely(PERTASK_GET(this_task.t_sched.sched_ring.re_next) == THIS_TASK) {
   PREEMPTION_ENABLE();

   if (!did_clear_vm) {
    did_clear_vm = true;
    if (THIS_VM->vm_refcnt == 1 &&
         /* Kernel-jobs use the kernel VM, meaning it would be a bad idea
          * for them to arbitrarily delete all mappings below the kernel... */
       !PERTASK_TESTF(this_task.t_flags,TASK_FKERNELJOB) &&
         /* NOTE: We can't do something as convoluted as unmapping
          *       memory if we must fear getting terminated at random. */
       !PERTASK_TESTF(this_task.t_state,TASK_STATE_FHELPMETERM)) {
     /* If we're the last thing keeping our VM going, then we
      * can already go ahead and unmap everything from userspace.
      * That way, we don't have to wait until we actually die
      * before releasing all that sweet memory to the kernel. */
     vm_unmap(0,X86_KERNEL_BASE_PAGE,VM_UNMAP_NOEXCEPT,NULL);
    }
#if 0
    if (THIS_TASK != &_boot_task)
        task_queue_rpc(&_boot_task,&print_here,NULL,TASK_RPC_SYNC);
#endif
   }
   x86_cpu_idle(); /* XXX: Disable PIT */
   continue;
  }
  break;
 }
 /* Switch to the next task. */
 calling_cpu            = THIS_CPU;
 calling_task           = THIS_TASK;
 next_task              = calling_task->t_sched.sched_ring.re_next;
 calling_cpu->c_running = next_task;
 assert(next_task != calling_task);
 RING_REMOVE(calling_task,t_sched.sched_ring);
 old_pagedir = (uintptr_t)calling_task->t_vm->vm_physdir;
 /* Try to decrement our own reference counter, but only do so
  * if it wouldn't result in us having to destroy ourselves.
  * If the decrement worked, immediately jump to `x86_load_context' */
 for (;;) {
  ref_t refcnt = ATOMIC_READ(calling_task->t_refcnt);
  assert(refcnt >= 1);
  if (refcnt == 1) break; /* Use an RPC to have another thread on our own CPU get rid of us. */
  __asm__ __volatile__("lock; cmpxchgl %1, %0\n"
                       /* If the exchange we OK, we no longer have any guaranty
                        * that our stack still exists (another CPU could be doing the
                        * final decref() any second now, or even worse: already has?).
                        * So we must be careful not to do anything that would touch
                        * the stack until we switch to the next task.
                        * Luckily, we know that interrupts on our own CPU are
                        * currently disabled, so we can simply jump to the
                        * assembly implementation of a function that does
                        * exactly that without touching the stack. */
                       "jz       x86_load_context\n"
                       :
                       : "m" (calling_task->t_refcnt)
                       , "r" (refcnt-1)
                       , "a" (refcnt)
                       , "c" (next_task)
                       , "d" (old_pagedir)
                       : "memory", "cc");
 }
 /* We're the last thing holding this task alive.
  * But since we can't very well destroy ourselves, we must
  * instruct the next task to do it for us (using an RPC) */
 TRY {
  if (task_queue_rpc(calling_cpu->c_running,&task_decref_rpc,
                     calling_task,TASK_RPC_NORMAL))
      goto load_next_task;
 } CATCH (E_WOULDBLOCK) {
  /* Preemption is disabled and `task_queue_rpc()' would have blocked.
   * However, we can't enable preemption during the queue, because we
   * need it disabled to keep us alive once the RPC _has_ been scheduled. */
 }
 /* Re-schedule the calling thread and yield to the other task,
  * then proceed to start over with our destruction process, hoping
  * that we'll get lucky the next time. */
 RING_INSERT_BEFORE(next_task,calling_task,t_sched.sched_ring);
 calling_cpu->c_running = calling_task;
 /* NOTE: There is a chance of an infinite loop here when all remaining
  *       tasks of the calling CPU are trying to terminate themselves.
  *       However considering that that would mean, an infinite loop
  *       is probably the only thing we _could_ do in such a case.
  * Special handling is done to deal with such a situation
  * and narrow down remaining tasks to only a single thread,
  * thus turning the IDLE-spinning loop that would otherwise
  * happen into one containing a `hlt' instruction, thus not
  * actually keeping the CPU busy _all_ the time... */
 if (next_task->t_state & TASK_STATE_FHELPMETERM) {
  /* Let's help out a friend... (to take their own life)
   *  -> Damn 'puter, you scary! */
  ATOMIC_FETCHAND(calling_task->t_state,~TASK_STATE_FHELPMETERM);
  RING_REMOVE(next_task,t_sched.sched_ring);
  /* Re-enable preemption while we destroy this other thread. */
  PREEMPTION_ENABLE();
#if 0
  debug_printf("HELP_TERMINATION(%p)\n",next_task);
#endif
  task_decref(next_task); /* This will probably destroy it... */
  goto restart_final_decref;
 }
 if (TASK_ISTERMINATING(next_task)) {
  /* The next thread won't be able to terminate us using RPC calls.
   * Let's ask around for help. */
#if 0
  debug_printf("ASK_TERMINATION(%p)\n",calling_task);
#endif
  ATOMIC_FETCHOR(calling_task->t_state,TASK_STATE_FHELPMETERM);
 }

 /* Re-enable preemption. */
 PREEMPTION_ENABLE();
 /* Yield to the other task. */
 task_yield();
 /* Try the final decref() again. */
 goto restart_final_decref;
load_next_task:
 /* Finally, load the next task (who will then decref()
  * us the next time it serves RPC function calls).
  * NOTE: We know that our task (and stack) hasn't been destroyed
  *       yet, because preemption is still disabled, and the thread
  *       that will eventually destroy us is running on our own CPU. */
 x86_load_context(next_task,old_pagedir);
 /* Never get here. */
}



PRIVATE void KCALL exit_group_rpc(void *info) {
 struct exception_info *reason;
 reason         = error_info();
 reason->e_error.e_code = E_EXIT_THREAD;
 reason->e_error.e_flag = ERR_FNORMAL;
 memset(reason->e_error.e_pointers,0,sizeof(reason->e_error.e_pointers));
 reason->e_error.e_exit.e_status = (int)(unsigned int)(uintptr_t)info;
 error_throw_current();
}

INTERN void KCALL take_down_process(unsigned int status) {
 struct task *proc = get_this_process();
 if (proc != THIS_TASK) {
  /* Send an RPC to terminate the leader with the same reason. */
  task_queue_rpc(proc,&exit_group_rpc,
                (void *)(uintptr_t)status,
                 TASK_RPC_SINGLE);
 }
}


PRIVATE void KCALL
errorinfo_copy_to_user(USER CHECKED struct user_task_segment *useg,
                       struct exception_info *__restrict info,
                       struct cpu_hostcontext_user *__restrict context) {
#if __SIZEOF_POINTER__ > 4
 memset(info.e_error.__e_pad,0,sizeof(info.e_error.__e_pad));
#endif
 if (info->e_error.e_code == E_NONCONTINUABLE &&
     info->e_error.e_noncont.nc_origip >= KERNEL_BASE)
     info->e_error.e_noncont.nc_origip = context->c_eip;
 memcpy(&useg->ts_xcurrent.e_error,
        &info->e_error,
        sizeof(struct exception_data));
#ifdef CONFIG_X86_SEGMENTATION
 memcpy(&useg->ts_xcurrent.e_context.c_gpregs,
        &context->c_gpregs,
        sizeof(struct x86_gpregs)+
        sizeof(struct x86_segments));
#else /* CONFIG_X86_SEGMENTATION */
 memcpy(&useg->ts_xcurrent.e_context.c_gpregs,
        &context->c_gpregs,
        sizeof(struct x86_gpregs));
#endif /* !CONFIG_X86_SEGMENTATION */
 useg->ts_xcurrent.e_context.c_eip    = context->c_iret.ir_eip;
 useg->ts_xcurrent.e_context.c_eflags = context->c_iret.ir_eflags;
#ifdef CONFIG_X86_SEGMENTATION
 useg->ts_xcurrent.e_context.c_cs     = context->c_iret.ir_cs;
 useg->ts_xcurrent.e_context.c_ss     = context->c_iret.ir_ss;
#endif /* CONFIG_X86_SEGMENTATION */
}


INTDEF ATTR_NORETURN void FCALL
x86_default_error_unhandled_exception(bool is_standalone);

PUBLIC void FCALL
task_propagate_user_exception(struct cpu_hostcontext_user *__restrict context,
                              unsigned int mode) {
 struct exception_info *EXCEPT_VAR error = error_info();
 struct exception_info EXCEPT_VAR info;
 bool is_standalone = false;
copy_error:
 /* Save exception information if something goes wrong during cleanup. */
 memcpy((void *)&info,error,sizeof(struct exception_info));
serve_rpc:

 /* Disconnect arbitrary task signal connections that may still be active. */
 task_disconnect();

 /* Serve RPC functions scheduled for
  * execution before returning to user-space. */
 TRY {
  task_serve_before_user(context,mode);
  COMPILER_BARRIER();
  /* RPC callbacks are allowed to change the error code to USER_RESUME
   * as an indicator that the purpose of the original exception was to
   * return to user-space. (E_INTERRUPT --> E_USER_RESUME after RPC
   * callback, where the actual work was done by the RPC callback) */
  if (error->e_error.e_code == E_USER_RESUME &&
     !ERRORCODE_ISHIGHPRIORITY(info.e_error.e_code))
      info.e_error.e_code = E_USER_RESUME;

  COMPILER_BARRIER();
  switch (info.e_error.e_code) {

  case E_USER_RESUME:
   return; /* Nothing left to be done here! */

  case E_EXIT_PROCESS:
   /* Exit the process */
   take_down_process((unsigned int)info.e_error.e_exit.e_status);
   ATTR_FALLTHROUGH;
  case E_EXIT_THREAD:
   task_exit(); /* Simple enough... */
   __builtin_unreachable();

  default: break;
  }
#if 1
  if (info.e_error.e_code == E_SEGFAULT &&
   !((uintptr_t)info.e_error.e_segfault.sf_reason & X86_SEGFAULT_FUSER) &&
      info.e_error.e_segfault.sf_vaddr != 0)
      goto unhandled_exception;
#endif

#if 1
  assertf(!(mode & X86_SYSCALL_TYPE_FPF),
          "Exception propagation cannot be done for #PF system calls. "
          "The caller should convert the register state to `int $0x80' "
          "prior to exception propagation");
  if (TASK_USERCTX_TYPE(mode) == TASK_USERCTX_TYPE_INTR_SYSCALL &&
    !(context->c_gpregs.gp_eax & 0x80000000)) {
   /* Translate the exception into an errno. */
#if 0
   error_printf("Translate to errno\n");
#endif
   context->c_gpregs.gp_eax = -exception_errno((struct exception_info *)&info);
   return;
  }

  if (PERTASK_TESTF(this_task.t_flags,TASK_FOWNUSERSEG|TASK_FUSEREXCEPT)) {
   struct cpu_context unwind;
   unwind.c_gpregs = context->c_gpregs;
#ifdef CONFIG_X86_SEGMENTATION
   memcpy(&unwind.c_gpregs,&context->c_gpregs,
          sizeof(struct x86_gpregs)+
          sizeof(struct x86_segments));
#else /* CONFIG_X86_SEGMENTATION */
   memcpy(&unwind.c_gpregs,&context->c_gpregs,
          sizeof(struct x86_gpregs));
#endif /* !CONFIG_X86_SEGMENTATION */
   unwind.c_esp            = context->c_iret.ir_useresp;
   unwind.c_iret.ir_eip    = context->c_iret.ir_eip;
   unwind.c_iret.ir_cs     = context->c_iret.ir_cs;
   unwind.c_iret.ir_eflags = context->c_iret.ir_eflags;
   if (info.e_error.e_flag & ERR_FRESUMENEXT) ++unwind.c_iret.ir_eip;
   TRY {
    /* Unwind the stack and search for user-space exception handlers. */
    while (unwind.c_iret.ir_eip < KERNEL_BASE) {
     struct fde_info fde;
     struct exception_handler_info hand;
     if (!linker_findfde(unwind.c_iret.ir_eip-1,&fde))
          goto cannot_unwind;
     if (linker_findexcept(unwind.c_iret.ir_eip-1,info.e_error.e_code,&hand)) {
      /* Found a user-space exception handler! */
      if (hand.ehi_flag & EXCEPTION_HANDLER_FUSERFLAGS)
          info.e_error.e_flag |= hand.ehi_mask & ERR_FUSERMASK;
      if (hand.ehi_flag & EXCEPTION_HANDLER_FDESCRIPTOR) {
       /* Unwind the stack to the caller-site. */
       if (!eh_return(&fde,&unwind,EH_FRESTRICT_USERSPACE|EH_FDONT_UNWIND_SIGFRAME))
            goto cannot_unwind;
       /* Override the IP to use the entry point.  */
       unwind.c_eip = (uintptr_t)hand.ehi_entry;
       assert(hand.ehi_desc.ed_type == EXCEPT_DESC_TYPE_BYPASS); /* XXX: What should we do here? */

       /* Allocate the stack-save area. */
       CPU_CONTEXT_SP(unwind) -= hand.ehi_desc.ed_safe;
       if (!(hand.ehi_desc.ed_flags&EXCEPT_DESC_FDEALLOC_CONTINUE)) {
        uintptr_t sp = context->c_iret.ir_useresp;
        /* Must allocate stack memory at the back and copy data there. */
        sp -= hand.ehi_desc.ed_safe;
        validate_writable((void *)sp,hand.ehi_desc.ed_safe);
        COMPILER_WRITE_BARRIER();
        memcpy((void *)sp,(void *)CPU_CONTEXT_SP(unwind),
                hand.ehi_desc.ed_safe);
        CPU_CONTEXT_SP(unwind) = sp;
       }
      } else {
       /* Jump to the entry point of this exception handler. */
       if (!eh_jmp(&fde,&unwind,(uintptr_t)hand.ehi_entry,
                    EH_FRESTRICT_USERSPACE))
            goto cannot_unwind;
       /* Restore the original SP to restore the stack as
        * it was when the exception originally occurred.
        * Without this, it would be impossible to continue
        * execution after an exception occurred. */
       CPU_CONTEXT_SP(unwind) = context->c_iret.ir_useresp;
      }

      /* Force user-space target. */
      unwind.c_iret.ir_cs     |= 3;
      unwind.c_iret.ir_eflags |= EFLAGS_IF;
      unwind.c_iret.ir_eflags &= ~(EFLAGS_TF|EFLAGS_IOPL(3)|
                                   EFLAGS_NT|EFLAGS_RF|EFLAGS_VM|
                                   EFLAGS_AC|EFLAGS_VIF|EFLAGS_VIP|
                                   EFLAGS_ID);
      /* Now copy the exception context to user-space. */
      {
       USER CHECKED struct user_task_segment *useg;
       /* NOTE: Dereference the user-space segment's self-pointer, so
        *       user-space is able to quickly re-direct exception storage
        *       by overriding the thread-local TLS self-pointer. */
       useg = PERTASK_GET(this_task.t_userseg)->ts_self;
       validate_writable(useg,sizeof(struct user_task_segment));
       errorinfo_copy_to_user(useg,(struct exception_info *)&info,context);
      }
      /* Set the unwound context as what should be returned to for user-space. */
#ifdef CONFIG_X86_SEGMENTATION
      memcpy(&context->c_gpregs,&unwind.c_gpregs,
             sizeof(struct x86_gpregs)+
             sizeof(struct x86_segments));
#else /* CONFIG_X86_SEGMENTATION */
      memcpy(&context->c_gpregs,&unwind.c_gpregs,
             sizeof(struct x86_gpregs));
#endif /* !CONFIG_X86_SEGMENTATION */
      context->c_iret.ir_useresp = unwind.c_esp;
      context->c_iret.ir_eip     = unwind.c_iret.ir_eip;
      context->c_iret.ir_cs      = unwind.c_iret.ir_cs;
      context->c_iret.ir_eflags  = unwind.c_iret.ir_eflags;
      return;
     }
     if (!eh_return(&fde,&unwind,EH_FRESTRICT_USERSPACE|EH_FDONT_UNWIND_SIGFRAME))
          goto cannot_unwind;
    }
   } CATCH (E_SEGFAULT) {
   }
  }
 cannot_unwind:
#endif

#if 1
  /* Handle certain exceptions using POSIX signal handlers:
   * e.g.: E_SEGFAULT -> SIGSEGV */
  TRY {
   siginfo_t sinfo;
   struct sighand *hand;
   struct sigaction *action;
   struct sigaction *action_copy;
   memset(&sinfo,0,sizeof(sinfo));
   switch (info.e_error.e_code) {

   case E_SEGFAULT:
   case E_STACK_OVERFLOW:
    sinfo.si_signo = 1+SIGSEGV;
    if (info.e_error.e_segfault.sf_reason & X86_SEGFAULT_FPRESENT)
         sinfo.si_code = SEGV_ACCERR;
    else sinfo.si_code = SEGV_MAPERR;
    sinfo.si_addr  = info.e_error.e_segfault.sf_vaddr;
    sinfo.si_lower = info.e_error.e_segfault.sf_vaddr;
    sinfo.si_upper = info.e_error.e_segfault.sf_vaddr;
    break;

   case E_INTERRUPT:
    sinfo.si_signo = 1+SIGINT;
    break;

   case E_ILLEGAL_INSTRUCTION:
    if (info.e_error.e_illegal_instruction.ii_type &
        ERROR_ILLEGAL_INSTRUCTION_FADDRESS)
     sinfo.si_code = ILL_ILLADR;
    else if (info.e_error.e_illegal_instruction.ii_type &
             ERROR_ILLEGAL_INSTRUCTION_FTRAP)
     sinfo.si_code = ILL_ILLADR;
    else if (info.e_error.e_illegal_instruction.ii_type &
            (ERROR_ILLEGAL_INSTRUCTION_RESTRICTED|
             ERROR_ILLEGAL_INSTRUCTION_PRIVILEGED))
     sinfo.si_code = (info.e_error.e_illegal_instruction.ii_type &
                      ERROR_ILLEGAL_INSTRUCTION_FREGISTER)
                    ? ILL_PRVREG : ILL_PRVOPC;
    else {
     sinfo.si_code = (info.e_error.e_illegal_instruction.ii_type&
                     (ERROR_ILLEGAL_INSTRUCTION_FOPERAND|
                      ERROR_ILLEGAL_INSTRUCTION_FVALUE|
                      ERROR_ILLEGAL_INSTRUCTION_FREGISTER)
                      ? ILL_ILLOPC : ILL_ILLOPN);
    }
    sinfo.si_signo = 1+SIGILL;
 do_addrip:
    sinfo.si_addr  = (void *)CPU_CONTEXT_IP(*context);
    sinfo.si_lower = (void *)CPU_CONTEXT_IP(*context);
    sinfo.si_upper = (void *)CPU_CONTEXT_IP(*context);
    break;

   case E_BREAKPOINT:
    sinfo.si_signo = 1+SIGTRAP;
    sinfo.si_code  = TRAP_BRKPT;
    break;

   case E_INVALID_ALIGNMENT:
    sinfo.si_signo = 1+SIGBUS;
    sinfo.si_code  = BUS_ADRALN;
    goto do_addrip;

   case E_OVERFLOW:
    sinfo.si_signo = 1+SIGFPE;
    sinfo.si_code  = FPE_INTOVF;
    goto do_addrip;

   case E_DIVIDE_BY_ZERO:
    sinfo.si_signo = 1+SIGFPE;
    if (info.e_error.e_divide_by_zero.dz_type & ERROR_DIVIDE_BY_ZERO_FFLT)
         sinfo.si_code = FPE_FLTDIV;
    else sinfo.si_code = FPE_INTDIV;
    goto do_addrip;

   case E_UNKNOWN_SYSTEMCALL:
    sinfo.si_signo     = 1+SIGSYS;
    sinfo.si_call_addr = (void *)CPU_CONTEXT_IP(*context);
    sinfo.si_syscall   = (s32)info.e_error.e_unknown_systemcall.us_sysno;
    sinfo.si_arch      = 0; /* ??? */
    break;

   default:
    goto cannot_signal;
   }
   /* Lock signal handlers for reading. */
   hand = sighand_lock_read();
   if (!hand) goto cannot_signal;
   /* Check for a custom action associated to this signal. */
   action = hand->sh_hand[sinfo.si_signo-1];
   if (!action) {
    /* No user-defined handler for this signal. */
    atomic_rwlock_endread(&hand->sh_lock);
    goto cannot_signal;
   }
   /* Create a local copy of the signal action to-be performed. */
   action_copy = (struct sigaction *)alloca(sizeof(struct sigaction));
   memcpy(action_copy,action,sizeof(struct sigaction));
   atomic_rwlock_endread(&hand->sh_lock);

   memcpy(error,(void *)&info,sizeof(struct exception_info));

   if (action_copy->sa_handler <= __SIG_SPECIAL_MAX) {
    /* TODO: Special actions, such as SIG_SUSP, etc. */
    goto cannot_signal;
   } else {
    /* Redirect user-space to perform this action. */
    arch_posix_signals_redirect_action(context,
                                      &sinfo,
                                       action_copy,
                                       mode);
   }
   {
    /* Get the currently active sigblock set. */
    byte_t *iter,*end,*dst;
    struct sigblock *block = sigblock_unique();
    /* Mask out all additional signals. */
    end = (iter = (byte_t *)&action->sa_mask)+sizeof(sigset_t);
    dst = (byte_t *)&block->sb_sigset;
    for (; iter != end; ++iter) *dst |= *iter;
    /* Also block the signal itself when `SA_NODEFER' isn't set. */
    if (!(action->sa_flags & SA_NODEFER))
         __sigaddset(&block->sb_sigset,sinfo.si_signo);
   }

   /* Return to user-space now that it's been re-directed. */
   return;
  } CATCH (E_SEGFAULT) {
  }
 cannot_signal:;
#endif

#if 0
  if (PERTASK_TESTF(this_task.t_flags,TASK_FOWNUSERSEG|TASK_FUSEREXCEPT)) {
   /* This is an exception-enabled user-space thread, but we
    * weren't able to find an exception handler, or signal handler.
    * Check if one of the linked applications contains an unhandled
    * exception handler. */
   TRY {
    u16 user_state;
    USER CHECKED struct user_task_segment *useg;
    /* NOTE: Dereference the user-space segment's self-pointer, so
     *       user-space is able to quickly re-direct exception storage
     *       by overriding the thread-local TLS self-pointer. */
    useg = PERTASK_GET(this_task.t_userseg)->ts_self;
    validate_writable(useg,sizeof(struct user_task_segment));
 reload_state:
    user_state = useg->ts_state;
    COMPILER_READ_BARRIER();
    if (user_state & THREAD_STATE_FALONE)
        is_standalone = true; /* If you say so... */
    if (!(user_state & THREAD_STATE_FINUEH)) {
     /* Search for the UEH callback. */
     struct module_symbol ueh;
     if (!ATOMIC_CMPXCH(useg->ts_state,user_state,user_state|THREAD_STATE_FINUEH))
          goto reload_state; /* Shouldn't happen, but give this one the benefit of the doubt */
     ueh.ms_base = useg->ts_ueh;
     COMPILER_READ_BARRIER();
     if (ueh.ms_base != NULL)
         ueh.ms_type = MODULE_SYMBOL_NORMAL;
     else {
      ueh = vm_apps_dlsym("__$$OS$error_unhandled_exception");
     }
     if (ueh.ms_type != MODULE_SYMBOL_INVALID) {
      void *ueh_sp;
      /* Found a handler! - Copy exception information to user-space. */
      errorinfo_copy_to_user(useg,&info,context);
      /* Set the user-defined context that should be used for UEH handlers. */
      ueh_sp = useg->ts_ueh_sp;
      COMPILER_READ_BARRIER();
      if (ueh_sp)
          CPU_CONTEXT_SP(*context) = (uintptr_t)ueh_sp;
      CPU_CONTEXT_IP(*context) = (uintptr_t)ueh.ms_base;
      debug_printf("ueh.ms_base = %p\n",ueh.ms_base);
      debug_printf("ueh.ms_size = %p\n",ueh.ms_size);
      debug_printf("ueh.ms_type = %p\n",ueh.ms_type);
#ifdef CONFIG_X86_SEGMENTATION
      /* Set the user-space TLS segment to ensure
       * that the thread can read exception information. */
#ifdef __x86_64__
      context->c_segments.sg_fs = X86_USER_TLS;
#else
      context->c_segments.sg_gs = X86_USER_TLS;
#endif
#endif      
      return;
     } else {
      debug_printf("WTF? No UEH handler?\n");
     }
    }
   } CATCH (E_SEGFAULT) {
   }
  }
#endif

  /* TODO: posix-signal default actions (e.g. SIGSEGV -> SIG_CORE) */

 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  COMPILER_BARRIER();
  /* Re-prioritize secondary exceptions. */
  if (ERRORCODE_ISHIGHPRIORITY(error->e_error.e_code)) {
   if (!ERRORCODE_ISHIGHPRIORITY(info.e_error.e_code) ||
       info.e_error.e_code < error->e_error.e_code)
       goto copy_error;
  }
  /* Override low-priority exceptions. */
  if (ERRORCODE_ISLOWPRIORITY(info.e_error.e_code))
      goto copy_error;
  /* Ignore low-priority exceptions (such as additional interrupts...) */
  if (ERRORCODE_ISLOWPRIORITY(error->e_error.e_code)) {
   memcpy(error,(void *)&info,sizeof(struct exception_info));
   goto serve_rpc;
  }
#ifdef NDEBUG
  memcpy(error,(void *)&info,sizeof(struct exception_info));
  goto serve_rpc;
#else
  /* This shouldn't happen... */
  error_rethrow();
#endif
 }


unhandled_exception: ATTR_UNUSED
 /* Fallback: Restore the original exception and enter the
  *           kernel's default unhandled exception handler. */
 memcpy(error,(void *)&info,sizeof(struct exception_info));
 x86_default_error_unhandled_exception(is_standalone);
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_SCHEDULER_C */
