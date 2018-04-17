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

#ifdef CONFIG_X86_SEGMENTATION
INTDEF ATTR_NORETURN void KCALL
throw_invalid_segment(u16 segment_index, u16 segment_register);
#endif /* CONFIG_X86_SEGMENTATION */


INTERN void KCALL
x86_sigreturn_impl(void *UNUSED(arg),
                   struct cpu_hostcontext_user *__restrict EXCEPT_VAR context,
                   unsigned int UNUSED(mode)) {
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
#ifdef CONFIG_X86_SEGMENTATION
 memcpy(&context->c_gpregs,&frame->sf_return.m_context.c_gpregs,
        sizeof(struct x86_gpregs32)+sizeof(struct x86_segments32));
 context->c_iret.ir_cs = frame->sf_return.m_context.c_cs;
 context->c_iret.ir_ss = frame->sf_return.m_context.c_ss;
#else /* CONFIG_X86_SEGMENTATION */
 memcpy(&context->c_gpregs,&frame->sf_return.m_context.c_gpregs,
        sizeof(struct x86_gpregs32));
#if 0 /* Since `context' originates from user-space, these are already set. */
 context->c_iret.ir_cs = X86_USER_CS;
 context->c_iret.ir_ss = X86_USER_DS;
#endif
#endif /* !CONFIG_X86_SEGMENTATION */
 context->c_iret.ir_eflags  = frame->sf_return.m_context.c_eflags;
 context->c_iret.ir_eip     = frame->sf_return.m_context.c_eip;
 context->c_iret.ir_useresp = context->c_gpregs.gp_esp;
 frame_mode = frame->sf_mode;

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
#ifdef CONFIG_X86_SEGMENTATION
 /* Verify user-space segment indices. */
 TRY {
  if (context->c_segments.sg_ds && !__verw(context->c_segments.sg_ds))
      throw_invalid_segment(context->c_segments.sg_ds,INVALID_SEGMENT_REGISTER_DS);
  if (context->c_segments.sg_es && !__verw(context->c_segments.sg_es))
      throw_invalid_segment(context->c_segments.sg_es,INVALID_SEGMENT_REGISTER_ES);
  if (context->c_segments.sg_fs && !__verw(context->c_segments.sg_fs))
      throw_invalid_segment(context->c_segments.sg_fs,INVALID_SEGMENT_REGISTER_FS);
  if (context->c_segments.sg_gs && !__verw(context->c_segments.sg_gs))
      throw_invalid_segment(context->c_segments.sg_gs,INVALID_SEGMENT_REGISTER_GS);
  if (context->c_iret.ir_ss && !__verw(context->c_iret.ir_ss))
      throw_invalid_segment(context->c_iret.ir_ss,INVALID_SEGMENT_REGISTER_SS);
  /* Ensure ring #3 (This is _highly_ important. Without this,
   * user-space would be executed as kernel-code; autsch...) */
  if (!(context->c_iret.ir_cs&3) || !__verr(context->c_iret.ir_cs))
      throw_invalid_segment(context->c_iret.ir_cs,INVALID_SEGMENT_REGISTER_CS);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Must fix register values, as otherwise userspace wouldn't be able to handle the exception. */
  if (context->c_segments.sg_ds && !__verw(context->c_segments.sg_ds))
      context->c_segments.sg_ds = X86_USER_DS;
  if (context->c_segments.sg_es && !__verw(context->c_segments.sg_es))
      context->c_segments.sg_es = X86_USER_DS;
  if (context->c_segments.sg_fs && !__verw(context->c_segments.sg_fs))
      context->c_segments.sg_fs = X86_SEG_FS;
  if (context->c_segments.sg_gs && !__verw(context->c_segments.sg_gs))
      context->c_segments.sg_gs = X86_SEG_GS;
  if (context->c_iret.ir_ss && !__verw(context->c_iret.ir_ss))
      context->c_iret.ir_ss = X86_USER_DS;
  if (!(context->c_iret.ir_cs&3) || !__verr(context->c_iret.ir_cs))
      context->c_iret.ir_cs = X86_USER_CS;
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

 /* Determine how to return to user-space. */
 switch (frame_mode) {
 case TASK_USERCTX_FWITHINUSERCODE:
 case TASK_USERCTX_FAFTERSYSCALL:
  error_info()->e_error.e_code = E_USER_RESUME;
  break;
 case TASK_USERCTX_FAFTERINTERRUPT:
  error_throw(E_INTERRUPT);
  break;
 default:
  break;
 }
}

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
 frame->sf_mode = mode;

 /* Construct the return CPU context. */
#ifdef CONFIG_X86_SEGMENTATION
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
#ifdef CONFIG_X86_SEGMENTATION
 frame->sf_return.m_context.c_cs = context->c_iret.ir_cs;
 frame->sf_return.m_context.c_ss = context->c_iret.ir_ss;
#endif /* CONFIG_X86_SEGMENTATION */
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
 if ((mode == TASK_USERCTX_FAFTERINTERRUPT) &&
     (error_info()->e_error.e_flag & ERR_FSYSCALL_EXC)) {
  /* Set the exceptions-enabled bit in the system call vector number. */
  frame->sf_sigreturn = (void *)(PERTASK_GET(x86_sysbase)+
                                 X86_ENCODE_PFSYSCALL(SYS_sigreturn|0x80000000));
 } else {
  frame->sf_sigreturn = (void *)(PERTASK_GET(x86_sysbase)+
                                 X86_ENCODE_PFSYSCALL(SYS_sigreturn));
 }

 /* With the signal frame now generated, update context registers to execute the signal action. */
 context->c_esp = (uintptr_t)frame;
 context->c_eip = (uintptr_t)action->sa_handler;
}




DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_POSIX_SIGNALS_C */
