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
#ifndef GUARD_KERNEL_I386_KOS_EXCEPT_C
#define GUARD_KERNEL_I386_KOS_EXCEPT_C 1
#define _KOS_SOURCE 1
#define _NOSERVE_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/paging.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sched/task.h>
#include <sched/posix_signals.h>
#include <syscall.h>
#include <kos/context.h>
#include <kos/i386-kos/asm/pf-syscall.h>
#include <unwind/eh_frame.h>
#include <unwind/linker.h>
#include <i386-kos/gdt.h>

#include <asm/cpu-flags.h>
#include <assert.h>
#include <except.h>
#include <string.h>

#include <kernel/debug.h> /* TODO: Get rid of this. */

#include "posix_signals.h"
#include "except.h"

DECL_BEGIN


INTERN ATTR_NORETURN void FCALL
libc_error_rethrow_at(struct cpu_context *__restrict context) {
 struct fde_info info;
 struct exception_handler_info hand;
 struct task_connections cons;
 struct task_connections *EXCEPT_VAR pcons = &cons;
 bool is_first = true; u16 old_state;
 /* Safe the original stack-pointer. */
 uintptr_t sp = CPU_CONTEXT_SP(*context);
 assertf(context != &error_info()->e_context,
         "Remember how this function is allowed to modify the context? "
         "Wouldn't make much sense if you passed the context that's supposed "
         "to represent what was going on when the exception was thrown...");
#if 0
 debug_printf("%[vinfo:%f(%l,%c) : %n : __error_rethrow_at(%p)]\n",
             (uintptr_t)context->c_pip);
#endif
 /* Must disable serving of RPC functions to prevent the following recursion:
  *    #0  -- libc_error_rethrow_at()
  *    #1  -- linker_findfde_consafe()
  *    #2  -- vm_acquire()
  *    #3  -- task_wait()
  *    #4  -- task_serve()
  *    #5  -- TASK_RPC_USER
  *    #6  -- error_throw(E_INTERRUPT)
  *    #7  -- libc_error_rethrow_at()
  * Technically, it's not an infinite recursion, because once the second
  * `libc_error_rethrow_at()' would try to unwind the frame of the frame
  * that call us (which uses the CPU state it passes to us to describe
  * its return state), it would most likely point into nirvana, meaning
  * that it'd end up trying to call into `error_unhandled_exception()',
  * following which, and after some a lot of complex execution that can
  * barely even be comprehended, we'd eventually end up with an unhandled
  * user-RPC in task_exit(), as well as a corrupt (and therefor unwindable)
  * stack, leaving task_exit() to be called recursively, and the kernel to
  * panic() after dumping around 4-5 incomprehensible exception dumps...
  * Yup... So in the end, rethrowing a kernel exception must not cause
  * further exceptions related to RPC serving (and while that might seem
  * obvious at first, you've really got to realize that it took me a couple
  * of hours to figure that this is what was crashing the kernel...)
  */
 old_state = ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FDONTSERVE);
 task_push_connections(&cons);
 TRY {
  for (;;) {
   /* Account for the fact that return addresses point to the
    * first instruction _after_ the `call', when it is actually
    * that call which we are trying to guard. */
   if (!is_first) --context->c_pip;
   if (context->c_pip < KERNEL_BASE &&
     !(THIS_TASK->t_flags & TASK_FKERNELJOB))
       goto no_handler; /* Exception must be propagated to userspace. */
   if (!except_findfde(context->c_pip,&info)) {
    debug_printf("No frame at %p\n",context->c_pip);
    goto no_handler;
   }
   is_first = false;
   /* Search for a suitable exception handler (in reverse order!). */
   if (except_findexcept(context->c_pip,error_code(),&hand)) {
     if (hand.ehi_flag & EXCEPTION_HANDLER_FUSERFLAGS)
         error_info()->e_error.e_flag |= hand.ehi_mask & ERR_FUSERMASK;
    if (hand.ehi_flag & EXCEPTION_HANDLER_FDESCRIPTOR) {
     /* Unwind the stack to the caller-site. */
     if (!eh_return(&info,context,EH_FDONT_UNWIND_SIGFRAME))
          goto cannot_unwind;
     /* Override the IP to use the entry point. */
     context->c_pip = (uintptr_t)hand.ehi_entry;
     assert(hand.ehi_desc.ed_type == EXCEPTION_DESCRIPTOR_TYPE_BYPASS); /* XXX: What should we do here? */

     CPU_CONTEXT_SP(*context) -= hand.ehi_desc.ed_safe;
     error_info()->e_rtdata.xrt_free_sp = CPU_CONTEXT_SP(*context);
     if (!(hand.ehi_desc.ed_flags&EXCEPTION_DESCRIPTOR_FDEALLOC_CONTINUE)) {
      /* Must allocate stack memory at the back and copy data there. */
      sp -= hand.ehi_desc.ed_safe;
      memcpy((void *)sp,(void *)CPU_CONTEXT_SP(*context),hand.ehi_desc.ed_safe);
      CPU_CONTEXT_SP(*context) = sp;
     }
     if (hand.ehi_desc.ed_flags & EXCEPTION_DESCRIPTOR_FDISABLE_PREEMPTION)
         context->c_pflags &= ~EFLAGS_IF;
    } else {
     /* Jump to the entry point of this exception handler. */
     if (!eh_jmp(&info,context,(uintptr_t)hand.ehi_entry,EH_FNORMAL)) {
      debug_printf("Failed to jump to handler at %p\n",
                   hand.ehi_entry);
      goto no_handler;
     }

     error_info()->e_rtdata.xrt_free_sp = CPU_CONTEXT_SP(*context);
     /* Restore the original SP to restore the stack as
      * it were when the exception originally occurred.
      * Without this, it would be impossible to continue
      * execution after an exception occurred. */
     CPU_CONTEXT_SP(*context) = sp;
    }
#if 0
    debug_printf("%[vinfo:%f(%l,%c) : %n : cpu_setcontext(%p)] cs = %p, eflags = %p\n",
                (uintptr_t)CPU_CONTEXT_IP(unwind)-1,
                unwind.c_iret.ir_cs,
                unwind.c_iret.ir_pflags);
    debug_printf("CONTEXT %p: { ds: %p, es: %p, fs: %p, gs: %p }\n",
                 &unwind,
                 unwind.c_segments.sg_ds,
                 unwind.c_segments.sg_es,
                 unwind.c_segments.sg_fs,
                 unwind.c_segments.sg_gs);
#endif
    if (!(old_state&TASK_STATE_FDONTSERVE))
          ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FDONTSERVE);
    task_pop_connections(&cons);
    cpu_setcontext(context);
    /* Never get here... */
   }
#if 0
   else {
    debug_printf("%[vinfo:%f(%l,%c) : %n : Missing except %p\n]",
                 context->c_pip);
   }
#endif
   /* Continue unwinding the stack. */
   if (!eh_return(&info,context,EH_FDONT_UNWIND_SIGFRAME)) {
cannot_unwind:
    debug_printf("Failed to unwind frame at %p\n",context->c_pip);
    goto no_handler;
   }
#if 0
   debug_printf("%[vinfo:%f(%l,%c) : %n :] Unwind %p (ebp %p; fs: %p)\n",
                context->c_pip,context->c_pip,
                context->c_gpregs.gp_ebp,context->c_segments.sg_fs);
#endif
  }
  task_pop_connections(&cons);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  task_pop_connections(pcons);
  debug_printf("\n\n\n"
               "Exception occurred while unwinding another exception\n"
               "\n\n\n");
 }
no_handler:
 error_unhandled_exception();
}
DEFINE_PUBLIC_ALIAS(__error_rethrow_at,libc_error_rethrow_at);

INTERN bool FCALL
unwind_check_signal_frame(struct cpu_context_ss *__restrict context,
                          USER CHECKED sigset_t *signal_set,
                          size_t signal_set_size) {
 struct signal_frame *frame;
 if (context->c_context.c_pip != PERTASK_GET(x86_sysbase)+X86_ENCODE_PFSYSCALL(SYS_sigreturn))
     return false;
 frame = (struct signal_frame *)(context->c_context.c_psp -
                                 COMPILER_OFFSETAFTER(struct signal_frame,sf_sigreturn));
 validate_readable(frame,sizeof(*frame));
 /* XXX: FPU Context? */
 /* Copy the signal mask that would be restored into userspace. */
 if (signal_set_size)
     memcpy(signal_set,&frame->sf_sigmask,signal_set_size);
 frame->sf_return.m_context.c_cs;
#if !defined(CONFIG_NO_X86_SEGMENTATION) && !defined(__x86_64__)
 memcpy(context,&frame->sf_return.m_context,
        sizeof(struct x86_gpregs32)+
        sizeof(struct x86_segments32));
#else /* !CONFIG_NO_X86_SEGMENTATION */
 memcpy(context,&frame->sf_return.m_context,
        sizeof(struct x86_gpregs32));
#endif /* CONFIG_NO_X86_SEGMENTATION */
 context->c_context.c_iret.ir_pip    = frame->sf_return.m_context.c_pip;
 context->c_context.c_iret.ir_cs     = frame->sf_return.m_context.c_cs;
 context->c_context.c_iret.ir_pflags = frame->sf_return.m_context.c_pflags;
#ifdef __x86_64__
 context->c_context.c_iret.ir_ss     = frame->sf_return.m_context.c_ss;
#else
 context->c_ss                       = frame->sf_return.m_context.c_ss;
#endif
 return true;
}


INTERN bool FCALL
error_rethrow_at_user(USER CHECKED struct user_exception_info *except_info,
                      struct cpu_context_ss *__restrict context) {
 struct fde_info info;
 struct exception_handler_info hand;
 bool is_first = true;
 /* Safe the original stack-pointer. */
 uintptr_t sp;
 sp = CPU_CONTEXT_SP(context->c_context);
 for (;;) {
  /* Account for the fact that return addresses point to the
   * first instruction _after_ the `call', when it is actually
   * that call which we are trying to guard. */
  if (!is_first) --CPU_CONTEXT_IP(context->c_context);
  if (CPU_CONTEXT_IP(context->c_context) >= KERNEL_BASE)
      return false;
  if (!linker_findfde(CPU_CONTEXT_IP(context->c_context),&info))
       return false;
  is_first = false;
  /* Search for a suitable exception handler (in reverse order!). */
  if (linker_findexcept(CPU_CONTEXT_IP(context->c_context),
                        except_info->e_error.e_code,&hand)) {
   if (hand.ehi_flag & EXCEPTION_HANDLER_FUSERFLAGS)
       except_info->e_error.e_flag |= hand.ehi_mask & ERR_FUSERMASK;
   if (hand.ehi_flag & EXCEPTION_HANDLER_FDESCRIPTOR) {
    /* Unwind the stack to the caller-site. */
    if (!eh_return(&info,&context->c_context,
                    EH_FRESTRICT_USERSPACE|EH_FDONT_UNWIND_SIGFRAME))
         return false;
    /* Override the IP to use the entry point.  */
    context->c_context.c_pip = (uintptr_t)hand.ehi_entry;
    assert(hand.ehi_desc.ed_type == EXCEPTION_DESCRIPTOR_TYPE_BYPASS); /* XXX: What should we do here? */
    CPU_CONTEXT_SP(context->c_context) -= hand.ehi_desc.ed_safe;
    except_info->e_rtdata.xrt_free_sp = CPU_CONTEXT_SP(context->c_context);
    if (!(hand.ehi_desc.ed_flags&EXCEPTION_DESCRIPTOR_FDEALLOC_CONTINUE)) {
     /* Must allocate stack memory at the back and copy data there. */
     sp -= hand.ehi_desc.ed_safe;
     validate_writable((void *)sp,hand.ehi_desc.ed_safe);
     validate_readable((void *)CPU_CONTEXT_SP(context->c_context),hand.ehi_desc.ed_safe);
     COMPILER_BARRIER();
     memcpy((void *)sp,(void *)CPU_CONTEXT_SP(context->c_context),hand.ehi_desc.ed_safe);
    }
   } else {
    /* Jump to the entry point of this exception handler. */
    if (!eh_jmp(&info,&context->c_context,(uintptr_t)hand.ehi_entry,
                 EH_FRESTRICT_USERSPACE))
         return false;
    except_info->e_rtdata.xrt_free_sp = CPU_CONTEXT_SP(context->c_context);
    /* Restore the original SP to restore the stack as
     * it were when the exception originally occurred.
     * Without this, it would be impossible to continue
     * execution after an exception occurred. */
    CPU_CONTEXT_SP(context->c_context) = sp;
   }
   return true;
  }
  /* Continue unwinding the stack. */
  if (!eh_return(&info,&context->c_context,
                  EH_FRESTRICT_USERSPACE|EH_FDONT_UNWIND_SIGFRAME))
       return false;
  {
   sigset_t sigset;
   if (unwind_check_signal_frame(context,&sigset,sizeof(sigset_t))) {
    /* Set the new signal mask. */
    signal_chmask(&sigset,NULL,sizeof(sigset_t),SIGNAL_CHMASK_FSETMASK);
   }
  }
 }
 return false;
}
DEFINE_PUBLIC_ALIAS(__error_rethrow_at,libc_error_rethrow_at);





DEFINE_SYSCALL_MUSTRESTART(xunwind_except);
DEFINE_SYSCALL3(xunwind_except,
                USER UNCHECKED struct user_exception_info *,except_info,
                USER UNCHECKED struct x86_usercontext *,dispatcher_context,
                USER UNCHECKED struct fpu_context *,dispatcher_fcontext) {
 struct cpu_context_ss unwind;
 validate_writable(except_info,sizeof(struct user_exception_info));
 validate_writable(dispatcher_context,sizeof(struct x86_usercontext));
 unwind.c_context.c_iret.ir_cs     = dispatcher_context->c_cs;
 unwind.c_context.c_iret.ir_pip    = dispatcher_context->c_pip;
 unwind.c_context.c_iret.ir_pflags = dispatcher_context->c_pflags;
#ifdef __x86_64__
 unwind.c_context.c_iret.ir_ss     = dispatcher_context->c_ss;
 if (!unwind.c_context.c_iret.ir_ss) unwind.c_context.c_iret.ir_ss = X86_SEG_USER_SS;
#else
 unwind.c_ss                       = dispatcher_context->c_ss;
 if (!unwind.c_ss)                   unwind.c_ss                   = X86_SEG_USER_SS;
#endif
 if (!unwind.c_context.c_iret.ir_cs) unwind.c_context.c_iret.ir_cs = X86_SEG_USER_CS;
#if !defined(CONFIG_NO_X86_SEGMENTATION) && !defined(__x86_64__)
 memcpy(&unwind.c_context.c_gpregs,dispatcher_context,
        sizeof(struct x86_gpregs)+sizeof(struct x86_segments));
 if (!unwind.c_context.c_segments.sg_ds) unwind.c_context.c_segments.sg_ds = X86_SEG_USER_DS;
 if (!unwind.c_context.c_segments.sg_es) unwind.c_context.c_segments.sg_es = X86_SEG_USER_DS;
 if (!unwind.c_context.c_segments.sg_fs) unwind.c_context.c_segments.sg_fs = X86_SEG_USER_FS;
 if (!unwind.c_context.c_segments.sg_gs) unwind.c_context.c_segments.sg_gs = X86_SEG_USER_GS;
#else
 memcpy(&unwind.c_context.c_gpregs,&dispatcher_context->c_gpregs,sizeof(struct x86_gpregs));
#endif
 COMPILER_BARRIER();
 if (!error_rethrow_at_user(except_info,&unwind))
      return -EPERM; /* No handler found. */
 COMPILER_BARRIER();
 /* Copy the new context back to user-space. */
#if !defined(CONFIG_NO_X86_SEGMENTATION) && !defined(__x86_64__)
 memcpy(&dispatcher_context->c_gpregs,&unwind.c_context.c_gpregs,
        sizeof(struct x86_gpregs)+sizeof(struct x86_segments));
#else
 memcpy(&dispatcher_context->c_gpregs,&unwind.c_context.c_gpregs,
        sizeof(struct x86_gpregs));
#endif
#ifdef __x86_64__
 dispatcher_context->c_ss     = unwind.c_context.c_iret.ir_ss;
#else
 dispatcher_context->c_ss     = unwind.c_ss;
#endif
 dispatcher_context->c_cs     = unwind.c_context.c_iret.ir_cs;
 dispatcher_context->c_pip    = unwind.c_context.c_iret.ir_pip;
 dispatcher_context->c_pflags = unwind.c_context.c_iret.ir_pflags;
 return 0;
}


DEFINE_SYSCALL_MUSTRESTART(xunwind);
DEFINE_SYSCALL4(xunwind,
                USER UNCHECKED struct x86_usercontext *,context,
                USER UNCHECKED struct fpu_context *,fcontext,
                USER UNCHECKED sigset_t *,signal_set,size_t,sigset_size) {
 struct cpu_context_ss unwind; struct fde_info info;
 validate_writable(context,sizeof(struct x86_usercontext));
 if unlikely(sigset_size > sizeof(sigset_t))
             sigset_size = sizeof(sigset_t);
 if (sigset_size)
     validate_writable(signal_set,sigset_size);
 unwind.c_context.c_iret.ir_cs     = context->c_cs;
 unwind.c_context.c_iret.ir_pip    = context->c_pip;
 unwind.c_context.c_iret.ir_pflags = context->c_pflags;
 if (!unwind.c_context.c_iret.ir_cs) unwind.c_context.c_iret.ir_cs = X86_SEG_USER_CS;
#ifdef __x86_64__
 unwind.c_context.c_iret.ir_ss     = context->c_ss;
 if (!unwind.c_context.c_iret.ir_ss) unwind.c_context.c_iret.ir_ss = X86_SEG_USER_SS;
#else
 unwind.c_ss                       = context->c_ss;
 if (!unwind.c_ss)                   unwind.c_ss                   = X86_SEG_USER_SS;
#endif
#if !defined(CONFIG_NO_X86_SEGMENTATION) && !defined(__x86_64__)
 memcpy(&unwind.c_context.c_gpregs,&context->c_gpregs,
        sizeof(struct x86_gpregs)+sizeof(struct x86_segments));
 if (!unwind.c_context.c_segments.sg_ds) unwind.c_context.c_segments.sg_ds = X86_SEG_USER_DS;
 if (!unwind.c_context.c_segments.sg_es) unwind.c_context.c_segments.sg_es = X86_SEG_USER_DS;
 if (!unwind.c_context.c_segments.sg_fs) unwind.c_context.c_segments.sg_fs = X86_SEG_USER_FS;
 if (!unwind.c_context.c_segments.sg_gs) unwind.c_context.c_segments.sg_gs = X86_SEG_USER_GS;
#else
 memcpy(&unwind.c_context.c_gpregs,&context->c_gpregs,sizeof(struct x86_gpregs));
#endif
 /* Search for FDE information */
 if (!linker_findfde(unwind.c_context.c_iret.ir_eip,&info))
      return -EPERM;
 /* Unwind the frame. */
 if (!eh_return(&info,&unwind.c_context,
                 EH_FRESTRICT_USERSPACE|EH_FDONT_UNWIND_SIGFRAME))
      return -EPERM;
 /* Deal with posix-signal frame. */
 unwind_check_signal_frame(&unwind,signal_set,sigset_size);
 /* Copy the new context back to user-space. */
#if !defined(CONFIG_NO_X86_SEGMENTATION) && !defined(__x86_64__)
 memcpy(&context->c_gpregs,&unwind.c_context.c_gpregs,
        sizeof(struct x86_gpregs)+sizeof(struct x86_segments));
#else
 memcpy(&context->c_gpregs,&unwind.c_context.c_gpregs,
        sizeof(struct x86_gpregs));
#endif
#ifdef __x86_64__
 context->c_ss     = unwind.c_context.c_iret.ir_ss;
#else
 context->c_ss     = unwind.c_ss;
#endif
 context->c_cs     = unwind.c_context.c_iret.ir_cs;
 context->c_pip    = unwind.c_context.c_iret.ir_pip;
 context->c_pflags = unwind.c_context.c_iret.ir_pflags;
 return 0;
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_EXCEPT_C */
