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
#ifndef GUARD_KERNEL_I386_KOS_TASK_CLONE_C
#define GUARD_KERNEL_I386_KOS_TASK_CLONE_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kernel/syscall.h>
#include <kernel/malloc.h>
#include <kernel/debug.h>
#include <sched/task.h>
#include <sched/userstack.h>
#include <sched/pid.h>
#include <sched/group.h>
#include <sched/posix_signals.h>
#include <sched/taskref.h>
#include <sched/userthread.h>
#include <kernel/memory.h>
#include <kernel/interrupt.h>
#include <kernel/vm.h>
#include <kernel/user.h>
#include <except.h>
#include <string.h>
#include <bits/sched.h>
#include <kos/context.h>
#include <kos/intrin.h>
#include <kos/registers.h>
#include <asm/cpu-flags.h>
#include <i386-kos/gdt.h>
#include <i386-kos/scheduler.h>

DECL_BEGIN

typedef void (KCALL *task_clone_t)(struct task *__restrict new_thread, u32 flags);
typedef void (KCALL *task_startup_t)(u32 flags);
INTDEF task_clone_t pertask_clone_start[];
INTDEF task_clone_t pertask_clone_end[];

INTDEF task_startup_t pertask_startup_start[];
INTDEF task_startup_t pertask_startup_end[];

PRIVATE void FCALL
clone_entry(struct cpu_hostcontext_user *__restrict user_context, u32 flags) {
 task_startup_t *iter;

 /* Invoke task-context level startup functions. */
 for (iter  = pertask_startup_start;
      iter != pertask_startup_end; ++iter)
      (**iter)(flags);

#ifdef CONFIG_VM86
 if (!PERTASK_TESTF(this_task.t_flags,TASK_FVM86))
#endif
 {
  /* Automatically allocate a stack if user-space wants to. */
  if (interrupt_getsp() == (uintptr_t)CLONE_CHILDSTACK_AUTO)
      interrupt_setsp(VM_PAGE2ADDR(task_alloc_userstack()->us_pageend));

  /* Allocate an automatic user-space TLS segment if
   * no TLS pointer was set, and no new VM was allocated. */
  if (!(flags & CLONE_SETTLS) && (flags & CLONE_VM)) {
   USER struct user_task_segment *segment;
   segment = task_alloc_userseg();
   /* Set the active TLS/TIB pointers, since we won't get preempted
    * another time before entering userspace (if everything goes well) */
   set_user_tls_register(segment);
#ifndef CONFIG_NO_DOS_COMPAT
   set_user_tib_register(&segment->ts_tib);
#endif /* !CONFIG_NO_DOS_COMPAT */
  }
 }


 /* CAUTION: SEGFAULT!
  * If this write segfaults, the error will simply cause the thread to be terminated. */
 if (flags & CLONE_CHILD_SETTID)
    *THIS_TID_ADDRESS = posix_gettid();
 COMPILER_WRITE_BARRIER();

 /* If we're not supposed to clear the thread's TID once
  * it dies, just override that attribute with NULL. */
 if (!(flags & CLONE_CHILD_CLEARTID))
       PERTASK_SET(_this_tid_address,NULL);
 COMPILER_WRITE_BARRIER();

 /* Finally, switch to user-space. */
 cpu_setcontext((struct cpu_context *)user_context);
}

INTDEF void KCALL task_vm_clone(struct task *__restrict new_thread, u32 flags);


#define throw_invalid_segment(segment_index,segment_register) \
       __EXCEPT_INVOKE_THROW_NORETURN(throw_invalid_segment(segment_index,segment_register))
INTERN __EXCEPT_NORETURN void
(KCALL throw_invalid_segment)(u16 segment_index, u16 segment_register) {
 struct exception_info *info;
 info = error_info();
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 /* Construct an invalid-segment error. */
 info->e_error.e_invalid_segment.is_segment  = segment_index;
 info->e_error.e_invalid_segment.is_register = segment_register;
 info->e_error.e_code                        = E_INVALID_SEGMENT;
 info->e_error.e_flag                        = ERR_FNORMAL;
 error_throw_current();
 __builtin_unreachable();
}

INTERN pid_t KCALL
x86_clone_impl(USER CHECKED struct x86_usercontext *context,
               syscall_ulong_t flags,
               USER UNCHECKED pid_t *parent_tidptr,
               USER UNCHECKED void *tls_val,
               USER UNCHECKED pid_t *child_tidptr) {
 pid_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct task *EXCEPT_VAR new_task;
 assert(PREEMPTION_ENABLED());
 if (flags & CLONE_PARENT_SETTID)
     validate_writable(parent_tidptr,sizeof(pid_t));
 if (flags & (CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID))
     validate_writable(child_tidptr,sizeof(pid_t));

 new_task = task_alloc();
 TRY {
  struct cpu_context *host_context;
  struct cpu_hostcontext_user *user_context;
  task_alloc_stack(new_task,CONFIG_KERNELSTACK_SIZE/PAGESIZE);

#ifdef CONFIG_VM86
  if (context->c_eflags & EFLAGS_VM) {
   struct cpu_context_vm86 *vm_context;
   /* Construct a new vm86 thread. */
   new_task->t_flags |= TASK_FVM86;
   vm_context   = (struct cpu_context_vm86 *)new_task->t_stackend-1;
   user_context = (struct cpu_hostcontext_user *)vm_context;
   host_context = (struct cpu_context *)user_context-1;
   new_task->t_context = (struct cpu_anycontext *)host_context;
   memset(host_context,0,
          sizeof(struct cpu_context)+
          sizeof(struct cpu_context_vm86));
   /* Setup the initial register state. */
   host_context->c_gpregs.gp_ecx  = (uintptr_t)vm_context;
   host_context->c_gpregs.gp_edx  = (uintptr_t)flags;
   host_context->c_iret.ir_cs     = X86_KERNEL_CS;
   host_context->c_iret.ir_eflags = EFLAGS_IF;
   host_context->c_iret.ir_eip    = (uintptr_t)&clone_entry;
#ifndef CONFIG_NO_X86_SEGMENTATION
   host_context->c_segments.sg_gs = X86_SEG_GS;
   host_context->c_segments.sg_fs = X86_SEG_FS;
   host_context->c_segments.sg_es = X86_KERNEL_DS;
   host_context->c_segments.sg_ds = X86_KERNEL_DS;
#endif /* !CONFIG_NO_X86_SEGMENTATION */

   /* Copy the user-space CPU context. */
#ifndef CONFIG_NO_X86_SEGMENTATION
   memcpy(&vm_context->c_gpregs,&context->c_gpregs,
          sizeof(struct x86_gpregs)+sizeof(struct x86_segments));
#else
   memcpy(&vm_context->c_gpregs,&context->c_gpregs,
          sizeof(struct x86_gpregs));
#endif
   vm_context->c_iret.ir_cs = context->c_cs;
   vm_context->c_iret.ir_ss = context->c_ss;
   vm_context->c_eip        = context->c_eip;
   vm_context->c_eflags     = context->c_eflags;
   COMPILER_READ_BARRIER();

   vm_context->c_esp           = vm_context->c_gpregs.gp_esp;
   vm_context->c_gpregs.gp_esp = (uintptr_t)new_task->t_stackend-24;

   vm_context->c_iret.ir_eflags |= EFLAGS_IF|EFLAGS_VM;
   vm_context->c_iret.ir_eflags &= ~EFLAGS_ID;
   if (vm_context->c_iret.ir_eflags &
      (EFLAGS_TF|EFLAGS_IOPL(3)|EFLAGS_NT|
       EFLAGS_RF|EFLAGS_AC|EFLAGS_VIF|
       EFLAGS_VIP))
       error_throw(E_INVALID_ARGUMENT);

#ifndef CONFIG_NO_X86_SEGMENTATION
   vm_context->c_iret.ir_es     = vm_context->c_segments.sg_es;
   vm_context->c_iret.ir_ds     = vm_context->c_segments.sg_ds;
   vm_context->c_iret.ir_fs     = vm_context->c_segments.sg_fs;
   vm_context->c_iret.ir_gs     = vm_context->c_segments.sg_gs;
   vm_context->c_segments.sg_gs = X86_SEG_USER_GS;
   vm_context->c_segments.sg_fs = X86_SEG_USER_FS;
   vm_context->c_segments.sg_es = X86_SEG_USER_ES;
   vm_context->c_segments.sg_ds = X86_SEG_USER_DS;
#else /* !CONFIG_NO_X86_SEGMENTATION */
   vm_context->c_iret.ir_es = context->c_segments.sg_es;
   vm_context->c_iret.ir_ds = context->c_segments.sg_ds;
   vm_context->c_iret.ir_fs = context->c_segments.sg_fs;
   vm_context->c_iret.ir_gs = context->c_segments.sg_gs;
#endif /* !CONFIG_NO_X86_SEGMENTATION */
  } else
#endif /* CONFIG_VM86 */
  {
   /* Setup a context for the thread.
    * NOTE: The go-to user-space context _MUST_ be located at the base of the stack.
    *       This is required for signal handlers and user-space redirection to work
    *       properly during thread initialization. */
   user_context = (struct cpu_hostcontext_user *)new_task->t_stackend-1;
   host_context = (struct cpu_context *)user_context-1;
   new_task->t_context = (struct cpu_anycontext *)host_context;
   memset(host_context,0,
          sizeof(struct cpu_context)+
          sizeof(struct cpu_hostcontext_user));
   /* Setup the initial register state. */
   host_context->c_gpregs.gp_ecx  = (uintptr_t)user_context;
   host_context->c_gpregs.gp_edx  = (uintptr_t)flags;
   host_context->c_iret.ir_cs     = X86_KERNEL_CS;
   host_context->c_iret.ir_eflags = EFLAGS_IF;
   host_context->c_iret.ir_eip    = (uintptr_t)&clone_entry;
#ifndef CONFIG_NO_X86_SEGMENTATION
   host_context->c_segments.sg_gs = X86_SEG_GS;
   host_context->c_segments.sg_fs = X86_SEG_FS;
   host_context->c_segments.sg_es = X86_KERNEL_DS;
   host_context->c_segments.sg_ds = X86_KERNEL_DS;
#endif /* !CONFIG_NO_X86_SEGMENTATION */

   /* Copy the user-space CPU context. */
#ifndef CONFIG_NO_X86_SEGMENTATION
   memcpy(&user_context->c_gpregs,&context->c_gpregs,
          sizeof(struct x86_gpregs)+sizeof(struct x86_segments));
#else
   memcpy(&user_context->c_gpregs,&context->c_gpregs,
          sizeof(struct x86_gpregs));
#endif
   user_context->c_iret.ir_cs = context->c_cs;
   user_context->c_iret.ir_ss = context->c_ss;
   user_context->c_eip        = context->c_eip;
   user_context->c_eflags     = context->c_eflags;
   COMPILER_READ_BARRIER();

   /* Force some registers to proper values (don't want user-space to run in ring #0). */
   user_context->c_iret.ir_eflags |= EFLAGS_IF;
   user_context->c_iret.ir_eflags &= ~EFLAGS_ID;
   if (user_context->c_iret.ir_eflags &
      (EFLAGS_TF|EFLAGS_IOPL(3)|
       EFLAGS_NT|EFLAGS_RF|EFLAGS_VM|
       EFLAGS_AC|EFLAGS_VIF|EFLAGS_VIP))
       error_throw(E_INVALID_ARGUMENT);
   if (!user_context->c_iret.ir_cs)
        user_context->c_iret.ir_cs = X86_USER_CS;
   if (!user_context->c_iret.ir_ss)
        user_context->c_iret.ir_ss = X86_USER_DS;
   if (!__verw(user_context->c_iret.ir_ss))
        throw_invalid_segment(user_context->c_iret.ir_ss,X86_REGISTER_SEGMENT_SS);
   /* Ensure ring #3 (This is _highly_ important. Without this,
    * user-space would be executed as kernel-code; autsch...) */
   if (!(user_context->c_iret.ir_cs & 3) || !__verr(user_context->c_iret.ir_cs))
        throw_invalid_segment(user_context->c_iret.ir_cs,X86_REGISTER_SEGMENT_CS);
#ifndef CONFIG_NO_X86_SEGMENTATION
   /* Segment registers set to ZERO are set to their default values. */
   if (!user_context->c_segments.sg_gs)
        user_context->c_segments.sg_gs = X86_SEG_USER_GS;
   if (!user_context->c_segments.sg_fs)
        user_context->c_segments.sg_fs = X86_SEG_USER_FS;
   if (!user_context->c_segments.sg_es)
        user_context->c_segments.sg_es = X86_SEG_USER_ES;
   if (!user_context->c_segments.sg_ds)
        user_context->c_segments.sg_ds = X86_SEG_USER_DS;
   /* Verify user-space segment indices. */
   if (!__verw(user_context->c_segments.sg_ds))
        throw_invalid_segment(user_context->c_segments.sg_ds,X86_REGISTER_SEGMENT_DS);
   if (!__verw(user_context->c_segments.sg_es))
        throw_invalid_segment(user_context->c_segments.sg_es,X86_REGISTER_SEGMENT_ES);
   if (!__verw(user_context->c_segments.sg_fs))
        throw_invalid_segment(user_context->c_segments.sg_fs,X86_REGISTER_SEGMENT_FS);
   if (!__verw(user_context->c_segments.sg_gs))
        throw_invalid_segment(user_context->c_segments.sg_gs,X86_REGISTER_SEGMENT_GS);
#endif /* !CONFIG_NO_X86_SEGMENTATION */
   /* Place the stack-pointer given from user-space in its proper slot. */
   user_context->c_iret.ir_useresp = user_context->c_gpregs.gp_esp;

   /* Set the kernel stack as ESP to-be used when execution is
    * redirected to `x86_redirect_preemption()' during thread
    * initialization.
    * (The stack adjustment here mirrors the expectations of `x86_redirect_preemption()')
    * Without this, we'd end up using the user-space stack, which
    * not only would cause _a_ _lot_ of problems, but also probably
    * just end up in a #DF caused by the user-space stack now being
    * allocated yet (when #PF can't be called) */
#ifdef __x86_64__
   user_context->c_gpregs.gp_rsp = (uintptr_t)new_task->t_stackend;
#else
   user_context->c_gpregs.gp_esp = (uintptr_t)new_task->t_stackend-8;
#endif
  }

  COMPILER_WRITE_BARRIER();

  /* Set the user-space TLS pointer (this will get overwritten
   * during `clone_entry' if `CLONE_SETTLS' isn't set) */
  new_task->t_userseg = (USER struct user_task_segment *)tls_val;
  COMPILER_WRITE_BARRIER();

  /* Invoke clone operators. */
  {
   task_clone_t *iter;
   for (iter  = pertask_clone_start;
        iter != pertask_clone_end; ++iter)
        (**iter)(new_task,flags);
  }
  COMPILER_BARRIER();

  /* Load the child thread's pid */
  {
   struct thread_pid *pid;
   pid = FORTASK(new_task,_this_pid);
   assert(pid);
   /* Determine the thread's TID, as seen from the parent thread. */
   result = pid->tp_pids[THIS_THREAD_PID ? THIS_THREAD_PID->tp_ns->pn_indirection : 0];
  }
  if (flags & CLONE_PARENT_SETTID)
     *parent_tidptr = result;
  COMPILER_WRITE_BARRIER();

  /* Perform VM cloning _AFTER_ the new thread's
   * PID was saved to the user-space buffer.
   * This order is explicitly stated in the documentation of
   * `CLONE_PARENT_SETTID', so we simply follow its example. */
  task_vm_clone(new_task,flags);
  COMPILER_BARRIER();

  /* This will be deleted by the thread when `CLONE_CHILD_CLEARTID' isn't set. */
  FORTASK(new_task,_this_tid_address) = child_tidptr;

#ifndef CONFIG_NO_SMP
  /* Keep cache locality high by spawning the
   * new thread on the same CPU as the caller.
   * Also: Since CPU affinity is always inherited during a clone(),
   *       this will ensure that the new thread starts running on a
   *       core on which it is actually allowed to run. */
  new_task->t_cpu = THIS_CPU;
#endif


  /* Start the new thread. */
  task_start(new_task);

  /*task_yield();*/

 } FINALLY {
  if (FINALLY_WILL_RETHROW)
      task_failed(new_task);
  task_decref(new_task);
 }
 return result;
}



DEFINE_SYSCALL5(clone,
                USER struct x86_usercontext *,context,
                syscall_ulong_t,flags,
                USER pid_t *,parent_tidptr,
                USER void *,tls_val,
                USER pid_t *,child_tidptr) {
 validate_readable(context,sizeof(struct x86_usercontext));
 return x86_clone_impl(context,flags,parent_tidptr,tls_val,child_tidptr);
}



PRIVATE void KCALL
task_fork_impl(void *UNUSED(arg),
               struct cpu_hostcontext_user *__restrict context,
               unsigned int UNUSED(mode)) {
 struct x86_usercontext user_state;
 pid_t child_pid;

 /* Copy and context the given user-space context to
  * one that can be used to construct a new thread. */
#ifndef CONFIG_NO_X86_SEGMENTATION
 memcpy(&user_state,context,sizeof(struct x86_gpregs)+sizeof(struct x86_segments));
#else
 memcpy(&user_state,context,sizeof(struct x86_gpregs));
#endif
 interrupt_getipsp((uintptr_t *)&user_state.c_eip,
                   (uintptr_t *)&user_state.c_esp);
 user_state.c_eflags = context->c_eflags;
 user_state.c_cs     = context->c_iret.ir_cs;
 user_state.c_ss     = context->c_iret.ir_ss;
 /* Return ZERO(0) in the child process. */
 user_state.c_gpregs.gp_eax = 0;

 /* Do the clone. */
 child_pid = x86_clone_impl(&user_state,(SIGCHLD+1),NULL,NULL,NULL);

 /* Return the child process's PID in the parent process. */
 context->c_gpregs.gp_eax = child_pid;
 error_info()->e_error.e_code = E_USER_RESUME;
}


DEFINE_SYSCALL0(fork) {
 /* Queue a clone RPC for when we return to user-space. */
 task_queue_rpc_user(THIS_TASK,&task_fork_impl,NULL,TASK_RPC_USER);
 __builtin_unreachable();
}




DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_TASK_CLONE_C */
