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
#ifndef GUARD_KERNEL_I386_KOS_POSIX_SIGNALS_C
#define GUARD_KERNEL_I386_KOS_POSIX_SIGNALS_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <bits/sigaction.h>
#include <i386-kos/posix_signals.h>
#include <i386-kos/fpu.h>
#include <i386-kos/gdt.h>
#include <kernel/user.h>
#include <kernel/debug.h>
#include <kernel/vm.h>
#include <kernel/syscall.h>
#include <kos/context.h>
#include <kos/intrin.h>
#include <kos/registers.h>
#include <sched/task.h>
#include <sched/posix_signals.h>
#include <sched/task.h>
#include <sched/userstack.h>
#include <sched/pid.h>
#include <assert.h>
#include <string.h>
#include <sys/ucontext.h>
#include <asm/cpu-flags.h>
#include <syscall.h>
#include <except.h>

#include "posix_signals.h"

DECL_BEGIN

#ifndef CONFIG_NO_X86_SEGMENTATION
INTDEF ATTR_NORETURN void (KCALL throw_invalid_segment)(u16 segment_index, u16 segment_register);
#define throw_invalid_segment(segment_index,segment_register) \
       __EXCEPT_INVOKE_THROW_NORETURN(throw_invalid_segment(segment_index,segment_register))
#endif /* !CONFIG_NO_X86_SEGMENTATION */


INTERN void KCALL
x86_sigreturn_impl(void *UNUSED(arg),
                   struct cpu_hostcontext_user *__restrict context,
                   unsigned int UNUSED(mode)) {
 struct cpu_hostcontext_user *EXCEPT_VAR xcontext = context;
 struct signal_frame *frame;
 unsigned int frame_mode;
 assertf(context->c_iret.ir_cs & 3,"sigreturn() invoked from kernel-space");

 /* At this point, `context->c_esp' points after the
  * `sf_sigreturn' field of a signal frame structure. */
 frame = (struct signal_frame *)(context->c_esp -
                                 COMPILER_OFFSETAFTER(struct signal_frame,sf_sigreturn));
 validate_readable(frame,sizeof(*frame));
 /* Let's get to the restoring part of this all!
  * NOTE: User-space register sanitization _must_ be done _after_ we copied register
  *       values, and must be performed on the values then saved in `context'! */
 COMPILER_READ_BARRIER();
#ifndef CONFIG_NO_X86_SEGMENTATION
 memcpy(&context->c_gpregs,&frame->sf_return.m_context.c_gpregs,
        sizeof(struct x86_gpregs32)+sizeof(struct x86_segments32));
 context->c_iret.ir_cs = frame->sf_return.m_context.c_cs;
 context->c_iret.ir_ss = frame->sf_return.m_context.c_ss;
#else /* !CONFIG_NO_X86_SEGMENTATION */
 memcpy(&context->c_gpregs,&frame->sf_return.m_context.c_gpregs,
        sizeof(struct x86_gpregs32));
#if 0 /* Since `context' originates from user-space, these are already set. */
 context->c_iret.ir_cs = X86_USER_CS;
 context->c_iret.ir_ss = X86_USER_DS;
#endif
#endif /* CONFIG_NO_X86_SEGMENTATION */
 context->c_iret.ir_eflags  = frame->sf_return.m_context.c_eflags;
 context->c_iret.ir_eip     = frame->sf_return.m_context.c_eip;
 context->c_iret.ir_useresp = context->c_gpregs.gp_esp;
 frame_mode = frame->sf_mode;

 if (PERTASK_TESTF(this_task.t_flags,TASK_FOWNUSERSEG|TASK_FUSEREXCEPT) &&
     frame->sf_except.e_context.c_eip != (uintptr_t)-1) {
  /* Copy user-space exception information. */
  USER CHECKED struct user_task_segment *useg;
  useg = PERTASK_GET(this_task.t_userseg);
  validate_readable(&useg->ts_self,sizeof(useg->ts_self));
  useg = useg->ts_self;
  validate_writable(&useg->ts_xcurrent,sizeof(useg->ts_xcurrent));
  /* Restore user-space exception information during a regular sigreturn().
   * This behavior is by `EXCEPTIONS AND POSIX SIGNAL HANDLERS' in `<except.h>' */
  memcpy(&useg->ts_xcurrent,&frame->sf_except,sizeof(struct user_exception_info));
 }

#ifndef CONFIG_NO_FPU
 if (frame->sf_return.m_flags & __MCONTEXT_FHAVEFPU) {
  /* Restore the saved FPU state. */
  x86_fpu_alloc();
  COMPILER_BARRIER();
  memcpy(PERTASK_GET(x86_fpu_context),
        &frame->sf_return.m_fpu,
         sizeof(struct fpu_context));
  COMPILER_BARRIER();
  x86_fpu_load();
 } else {
  /* Reset the FPU state. */
  x86_fpu_reset();
 }
#endif /* !CONFIG_NO_FPU */
 COMPILER_READ_BARRIER();
 
 /* Sanitize the new user-space register state. */
 context->c_iret.ir_eflags |=  (EFLAGS_IF);
 context->c_iret.ir_eflags &= ~(EFLAGS_TF|EFLAGS_IOPL(3)|
                                EFLAGS_NT|EFLAGS_RF|EFLAGS_VM|
                                EFLAGS_AC|EFLAGS_VIF|EFLAGS_VIP|
                                EFLAGS_ID);
#ifndef CONFIG_NO_X86_SEGMENTATION
 /* Verify user-space segment indices. */
 TRY {
  if (context->c_segments.sg_ds && !__verw(context->c_segments.sg_ds))
      throw_invalid_segment(context->c_segments.sg_ds,X86_REGISTER_SEGMENT_DS);
  if (context->c_segments.sg_es && !__verw(context->c_segments.sg_es))
      throw_invalid_segment(context->c_segments.sg_es,X86_REGISTER_SEGMENT_ES);
  if (context->c_segments.sg_fs && !__verw(context->c_segments.sg_fs))
      throw_invalid_segment(context->c_segments.sg_fs,X86_REGISTER_SEGMENT_FS);
  if (context->c_segments.sg_gs && !__verw(context->c_segments.sg_gs))
      throw_invalid_segment(context->c_segments.sg_gs,X86_REGISTER_SEGMENT_GS);
  if (context->c_iret.ir_ss && !__verw(context->c_iret.ir_ss))
      throw_invalid_segment(context->c_iret.ir_ss,X86_REGISTER_SEGMENT_SS);
  /* Ensure ring #3 (This is _highly_ important. Without this,
   * user-space would be executed as kernel-code; autsch...) */
  if ((context->c_iret.ir_cs & 3) != 3 || !__verr(context->c_iret.ir_cs))
      throw_invalid_segment(context->c_iret.ir_cs,X86_REGISTER_SEGMENT_CS);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Must fix register values, as otherwise userspace wouldn't be able to handle the exception. */
  if (xcontext->c_segments.sg_ds && !__verw(xcontext->c_segments.sg_ds))
      xcontext->c_segments.sg_ds = X86_SEG_USER_DS;
  if (xcontext->c_segments.sg_es && !__verw(xcontext->c_segments.sg_es))
      xcontext->c_segments.sg_es = X86_SEG_USER_ES;
  if (xcontext->c_segments.sg_fs && !__verw(xcontext->c_segments.sg_fs))
      xcontext->c_segments.sg_fs = X86_SEG_USER_FS;
  if (xcontext->c_segments.sg_gs && !__verw(xcontext->c_segments.sg_gs))
      xcontext->c_segments.sg_gs = X86_SEG_USER_GS;
  if (xcontext->c_iret.ir_ss && !__verw(xcontext->c_iret.ir_ss))
      xcontext->c_iret.ir_ss = X86_SEG_USER_SS;
  if ((xcontext->c_iret.ir_cs & 3) != 3 || !__verr(xcontext->c_iret.ir_cs))
      xcontext->c_iret.ir_cs = X86_SEG_USER_CS;
  error_rethrow();
 }
#endif

 COMPILER_BARRIER();

 /* Restore the saved signal mask. */
 signal_chmask(&frame->sf_sigmask,NULL,
                sizeof(sigset_t),
                SIGNAL_CHMASK_FSETMASK);

#if 0
 debug_printf("SIGRETURN:%d\n",frame_mode);
#endif

 /* Determine how to return to user-space / resume execution. */
 switch (TASK_USERCTX_TYPE(frame_mode)) {
 case TASK_USERCTX_TYPE_WITHINUSERCODE:
  error_info()->e_error.e_code = E_USER_RESUME;
  break;
 case TASK_USERCTX_TYPE_INTR_INTERRUPT:
  error_throw(E_INTERRUPT);
  break;

 {
  syscall_ulong_t EXCEPT_VAR sysno;
  syscall_ulong_t EXCEPT_VAR orig_eax;
  syscall_ulong_t EXCEPT_VAR orig_eip;
#ifndef CONFIG_NO_X86_SYSENTER
  syscall_ulong_t EXCEPT_VAR orig_ebp;
  syscall_ulong_t EXCEPT_VAR orig_edi;
  syscall_ulong_t EXCEPT_VAR orig_esp;
#endif
 case TASK_USERCTX_TYPE_INTR_SYSCALL:
  /* Restart an interrupted system call by executing it now. */
  orig_eax = context->c_gpregs.gp_eax;
  orig_eip = context->c_eip;
#ifndef CONFIG_NO_X86_SYSENTER
  orig_ebp = context->c_gpregs.gp_ebp;
  orig_edi = context->c_gpregs.gp_edi;
  orig_esp = context->c_iret.ir_useresp;
#endif
restart_sigframe_syscall:
  TRY {
   /* Convert the user-space register context to become `int $0x80'-compatible */
#ifndef CONFIG_NO_X86_SYSENTER
   if (frame_mode & X86_SYSCALL_TYPE_FSYSENTER) {
    syscall_ulong_t masked_sysno; u8 argc = 6;
    sysno                      = xcontext->c_gpregs.gp_eax;
    masked_sysno               = sysno & ~0x80000000;
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
   } else
#endif /* !CONFIG_NO_X86_SYSENTER */
   if (frame_mode & X86_SYSCALL_TYPE_FPF) {
    sysno = xcontext->c_eip - PERTASK_GET(x86_sysbase);
    sysno = X86_DECODE_PFSYSCALL(sysno);
    xcontext->c_eip = xcontext->c_gpregs.gp_eax; /* #PF uses EAX as return address. */
    xcontext->c_gpregs.gp_eax = sysno; /* #PF encodes the sysno in EIP. */
   } else {
    sysno = xcontext->c_gpregs.gp_eax;
   }
#if 0
   debug_printf("\n\n"
                "Restart system call after signal (%p)\n"
                "\n\n",
                sysno);
#endif
   /* Actually execute the system call (using the `int $0x80'-compatible register set). */
   x86_syscall_exec80();
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* Set the FSYSCALL flag so that exceptions are propagated accordingly. */
   error_info()->e_error.e_flag |= X86_INTERRUPT_GUARD_FSYSCALL;
   if (error_code() == E_INTERRUPT) {
    /* Restore the original user-space CPU xcontext. */
    xcontext->c_gpregs.gp_eax = orig_eax;
    xcontext->c_eip           = orig_eip;
#ifndef CONFIG_NO_X86_SYSENTER
    xcontext->c_gpregs.gp_ebp = orig_ebp;
    xcontext->c_gpregs.gp_edi = orig_edi;
    xcontext->c_iret.ir_useresp = orig_esp;
#endif
    COMPILER_WRITE_BARRIER();
    /* Deal with system call restarts. */
    task_restart_syscall(xcontext,
                         TASK_USERCTX_TYPE_WITHINUSERCODE|
                         X86_SYSCALL_TYPE_FPF,
                         sysno);
    COMPILER_BARRIER();
    goto restart_sigframe_syscall;
   }
   error_rethrow();
  }
 } break;

 default: break;
 }
}



DEFINE_SYSCALL_DONTRESTART(sigreturn);
DEFINE_SYSCALL0(sigreturn) {
 /* Use an RPC callback to gain access to the user-space register state. */
 task_queue_rpc_user(THIS_TASK,
                    &x86_sigreturn_impl,
                     NULL,
                     TASK_RPC_USER);
 return 0;
}





/* Redirect the given user-space context to call a signal
 * handler `action' before actually returning to user-space. */
PUBLIC void KCALL
arch_posix_signals_redirect_action(struct cpu_hostcontext_user *__restrict context,
                                   siginfo_t const *__restrict info,
                                   struct sigaction const *__restrict action,
                                   unsigned int mode) {
 struct signal_frame *frame;
 validate_executable(action->sa_handler);
#if 0
 debug_printf("REDIRECT_ACTION %u:%p -> %p\n",
              posix_gettid(),
              context->c_eip,
              action->sa_handler);
#endif

 if (action->sa_flags & SA_SIGINFO) {
  struct userstack *stack = PERTASK_GET(_this_user_stack);
  struct signal_frame_ex *xframe;
  xframe = (struct signal_frame_ex *)context->c_esp-1;
  validate_writable(xframe,sizeof(*xframe));
  frame  = &xframe->sf_frame;
  xframe->sf_infop    = &xframe->sf_info;
  xframe->sf_contextp = &xframe->sf_return;
  /* Fill in extended-signal-frame-specific fields. */
  memcpy(&xframe->sf_info,info,sizeof(siginfo_t));
  if (stack) {
   xframe->sf_return.uc_stack.ss_sp    = (void *)VM_PAGE2ADDR(stack->us_pagemin);
   xframe->sf_return.uc_stack.ss_size  = VM_PAGES2SIZE(stack->us_pageend-stack->us_pagemin);
   xframe->sf_return.uc_stack.ss_flags = SS_ONSTACK;
  } else {
   xframe->sf_return.uc_stack.ss_sp    = (void *)context->c_esp;
   xframe->sf_return.uc_stack.ss_size  = 1;
   xframe->sf_return.uc_stack.ss_flags = SS_DISABLE;
  }
  xframe->sf_return.uc_link = NULL;
 } else {
  frame = (struct signal_frame *)context->c_esp-1;
  validate_writable(frame,sizeof(*frame));
 }

 {
  /* Copy the signal-blocking-set to-be applied upon return. */
  struct sigblock *block = PERTASK_GET(_this_sigblock);
  if (!block)
   memset(&frame->sf_sigmask,0,sizeof(sigset_t));
  else {
   memcpy(&frame->sf_sigmask,&block->sb_sigset,sizeof(sigset_t));
  }
 }

 if (PERTASK_TESTF(this_task.t_flags,TASK_FOWNUSERSEG|TASK_FUSEREXCEPT)) {
  /* Copy user-space exception information. */
  USER CHECKED struct user_task_segment *useg;
  useg = PERTASK_GET(this_task.t_userseg);
  validate_readable(&useg->ts_self,sizeof(useg->ts_self));
  useg = useg->ts_self;
  validate_readable(&useg->ts_xcurrent,sizeof(useg->ts_xcurrent));
  memcpy(&frame->sf_except,&useg->ts_xcurrent,sizeof(struct user_exception_info));
 } else {
  /* sys_sigreturn() uses the exception context EIP
   * value to identify the presence of a context. */
  frame->sf_except.e_context.c_eip = (uintptr_t)-1;
 }

 /* Construct the return CPU context. */
#ifndef CONFIG_NO_X86_SEGMENTATION
 memcpy(&frame->sf_return.m_context.c_segments,
        &context->c_segments,sizeof(struct x86_segments));
#endif
 frame->sf_return.m_context.c_gpregs.gp_edi = context->c_gpregs.gp_edi;
 frame->sf_return.m_context.c_gpregs.gp_esi = context->c_gpregs.gp_esi;
 frame->sf_return.m_context.c_gpregs.gp_ebp = context->c_gpregs.gp_ebp;
 frame->sf_return.m_context.c_gpregs.gp_esp = context->c_iret.ir_useresp;
 frame->sf_return.m_context.c_gpregs.gp_ebx = context->c_gpregs.gp_ebx;
 frame->sf_return.m_context.c_gpregs.gp_edx = context->c_gpregs.gp_edx;
 frame->sf_return.m_context.c_gpregs.gp_ecx = context->c_gpregs.gp_ecx;
 frame->sf_return.m_context.c_gpregs.gp_eax = context->c_gpregs.gp_eax;
 frame->sf_return.m_context.c_eip           = context->c_eip;
 frame->sf_return.m_context.c_eflags        = context->c_eflags;
#ifndef CONFIG_NO_X86_SEGMENTATION
 frame->sf_return.m_context.c_cs = context->c_iret.ir_cs;
 frame->sf_return.m_context.c_ss = context->c_iret.ir_ss;
#endif /* !CONFIG_NO_X86_SEGMENTATION */
 frame->sf_return.m_flags = __MCONTEXT_FNORMAL;
 if (info->si_signo == SIGSEGV) {
  frame->sf_return.m_cr2    = (uintptr_t)info->si_addr;
  frame->sf_return.m_flags |= __MCONTEXT_FHAVECR2;
 }
#ifndef CONFIG_NO_FPU
 if (x86_fpu_save()) {
  memcpy(&frame->sf_return.m_fpu,
          PERTASK_GET(x86_fpu_context),
          sizeof(struct fpu_context));
  frame->sf_return.m_flags |= __MCONTEXT_FHAVEFPU;
 }
#endif
 frame->sf_signo = info->si_signo;

 /* Redirect the frame's sig-return pointer to direct it at the `sys_sigreturn' system call.
  * Being able to do this right here is the main reason why #PF-syscalls were introduced. */
 if (TASK_USERCTX_TYPE(mode) == TASK_USERCTX_TYPE_INTR_SYSCALL) {
  syscall_ulong_t sysno = context->c_gpregs.gp_eax;
  if (mode & X86_SYSCALL_TYPE_FPF) {
   /* Deal with #PF system calls. */
   sysno = (uintptr_t)context->c_eip-PERTASK_GET(x86_sysbase);
   sysno = X86_DECODE_PFSYSCALL(sysno);
  }
  if (should_restart_syscall(sysno,
                            (action->sa_flags&SA_RESTART)
                          ? (SHOULD_RESTART_SYSCALL_FPOSIX_SIGNAL|
                             SHOULD_RESTART_SYSCALL_FSA_RESTART)
                          : (SHOULD_RESTART_SYSCALL_FPOSIX_SIGNAL))) {
   /* Setup the signal frame to restart the system
    * call once `sys_sigreturn()' is executed. */
   frame->sf_mode = mode;
  } else {
   /* Throw E_INTERRUPT, or return `-EINTR' from the system call that got interrupted. */
   frame->sf_mode = TASK_USERCTX_TYPE_INTR_INTERRUPT;
  }
  if (!(sysno & 0x80000000))
        goto sigreturn_noexcept;
  /* Set the exceptions-enabled bit in the system call vector number. */
  frame->sf_sigreturn = (void *)(PERTASK_GET(x86_sysbase)+
                                 X86_ENCODE_PFSYSCALL(SYS_sigreturn|0x80000000));
 } else {
  /* Resume execution in user-space normally. */
  frame->sf_mode = TASK_USERCTX_TYPE_WITHINUSERCODE;
sigreturn_noexcept:
  frame->sf_sigreturn = (void *)(PERTASK_GET(x86_sysbase)+
                                 X86_ENCODE_PFSYSCALL(SYS_sigreturn));
 }

 /* With the signal frame now generated, update context registers to execute the signal action. */
 context->c_esp = (uintptr_t)frame;
 context->c_eip = (uintptr_t)action->sa_handler;

 /* Resume user-space by executing the signal handler. */
 error_info()->e_error.e_code = E_USER_RESUME;
}




DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_POSIX_SIGNALS_C */
