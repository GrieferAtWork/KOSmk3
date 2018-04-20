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

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/paging.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sched/task.h>
#include <kos/context.h>
#include <unwind/eh_frame.h>
#include <unwind/linker.h>
#include <i386-kos/gdt.h>

#include <asm/cpu-flags.h>
#include <assert.h>
#include <except.h>
#include <string.h>

#include <kernel/debug.h> /* TODO: Get rid of this. */

DECL_BEGIN


INTERN ATTR_NORETURN void FCALL
libc_error_rethrow_at(struct cpu_context *__restrict context,
                      int ip_is_after_faulting) {
 struct cpu_context unwind;
 struct fde_info info;
 struct exception_handler_info hand;
 bool is_first = true;
 /* Safe the original stack-pointer. */
 uintptr_t sp;
 memcpy(&unwind,context,sizeof(struct cpu_context));

 sp = CPU_CONTEXT_SP(unwind);
#if 0
 debug_printf("%[vinfo:%f(%l,%c) : %n : __error_rethrow_at(%p)\n]",
             (uintptr_t)CPU_CONTEXT_IP(unwind)-1);
#endif
 for (;;) {
  uintptr_t ip = CPU_CONTEXT_IP(unwind);
  if (ip < KERNEL_BASE && !(THIS_TASK->t_flags & TASK_FKERNELJOB))
      goto no_handler; /* Exception must be propagated to userspace. */

  /* Account for the fact that return addresses point to the
   * first instruction _after_ the `call', when it is actually
   * that call which we are trying to guard. */
  if (!is_first || ip_is_after_faulting) --ip;
  if (!linker_findfde_consafe(ip,&info)) {
   debug_printf("No frame at %p\n",ip);
   goto no_handler;
  }
  is_first = false;
  /* Search for a suitable exception handler (in reverse order!). */
  if (linker_findexcept_consafe(ip,error_code(),&hand)) {
    if (hand.ehi_flag & EXCEPTION_HANDLER_FUSERFLAGS)
        error_info()->e_error.e_flag |= hand.ehi_mask & ERR_FUSERMASK;
   if (hand.ehi_flag & EXCEPTION_HANDLER_FDESCRIPTOR) {
    /* Unwind the stack to the caller-site. */
    if (!eh_return(&info,&unwind,EH_FDONT_UNWIND_SIGFRAME))
         goto cannot_unwind;
    /* Override the IP to use the entry point. */
    unwind.c_eip = (uintptr_t)hand.ehi_entry;
    assert(hand.ehi_desc.ed_type == EXCEPT_DESC_TYPE_BYPASS); /* XXX: What should we do here? */

    /* Allocate the stack-save area. */
    CPU_CONTEXT_SP(unwind) -= hand.ehi_desc.ed_safe;
    if (!(hand.ehi_desc.ed_flags&EXCEPT_DESC_FDEALLOC_CONTINUE)) {
     /* Must allocate stack memory at the back and copy data there. */
     sp -= hand.ehi_desc.ed_safe;
     memcpy((void *)sp,(void *)CPU_CONTEXT_SP(unwind),hand.ehi_desc.ed_safe);
     CPU_CONTEXT_SP(unwind) = sp;
    }
    if (hand.ehi_desc.ed_flags & EXCEPT_DESC_FDISABLE_PREEMPTION)
        unwind.c_eflags &= ~EFLAGS_IF;
   } else {
    /* Jump to the entry point of this exception handler. */
    if (!eh_jmp(&info,&unwind,(uintptr_t)hand.ehi_entry,EH_FNORMAL)) {
     debug_printf("Failed to jump to handler at %p\n",
                  hand.ehi_entry);
     goto no_handler;
    }
    /* Restore the original SP to restore the stack as
     * it were when the exception originally occurred.
     * Without this, it would be impossible to continue
     * execution after an exception occurred. */
    CPU_CONTEXT_SP(unwind) = sp;
   }
#if 0
   debug_printf("%[vinfo:%f(%l,%c) : %n : cpu_setcontext(%p)] cs = %p, eflags = %p\n",
               (uintptr_t)CPU_CONTEXT_IP(unwind)-1,
               unwind.c_iret.ir_cs,
               unwind.c_iret.ir_eflags);
   debug_printf("CONTEXT %p: { ds: %p, es: %p, fs: %p, gs: %p }\n",
                &unwind,
                unwind.c_segments.sg_ds,
                unwind.c_segments.sg_es,
                unwind.c_segments.sg_fs,
                unwind.c_segments.sg_gs);
#endif
   cpu_setcontext(&unwind);
   /* Never get here... */
  }
  /* Continue unwinding the stack. */
  if (!eh_return(&info,&unwind,EH_FDONT_UNWIND_SIGFRAME)) {
cannot_unwind:
   debug_printf("Failed to unwind frame at %p\n",ip);
   goto no_handler;
  }
#if 0
  debug_printf("%[vinfo:%f(%l,%c) : %n :] Unwind %p -> %p (ebp %p; fs: %p)\n",
               ip,ip,unwind.c_eip,unwind.c_gpregs.gp_ebp,unwind.c_segments.sg_fs);
#endif
 }
no_handler:
 error_unhandled_exception();
}
DEFINE_PUBLIC_ALIAS(__error_rethrow_at,libc_error_rethrow_at);

INTERN bool FCALL
libc_error_rethrow_at_user(struct cpu_hostcontext_user *__restrict context,
                           u16 exception_code, int ip_is_after_faulting) {
 struct fde_info info;
 struct exception_handler_info hand;
 bool is_first = true;
 /* Safe the original stack-pointer. */
 uintptr_t sp;
 sp = CPU_CONTEXT_SP(*context);
 context->c_gpregs.gp_esp = context->c_iret.ir_useresp;
 for (;;) {
  uintptr_t ip = CPU_CONTEXT_IP(*context);
  if (ip >= KERNEL_BASE)
      return false;
  /* Account for the fact that return addresses point to the
   * first instruction _after_ the `call', when it is actually
   * that call which we are trying to guard. */
  if (!is_first || ip_is_after_faulting) --ip;
  if (!linker_findfde(ip,&info))
       return false;
  is_first = false;
  /* Search for a suitable exception handler (in reverse order!). */
  if (linker_findexcept(ip,exception_code,&hand)) {
   if (hand.ehi_flag & EXCEPTION_HANDLER_FUSERFLAGS) {
    /* TODO */
    /*info->e_error.e_flag |= hand.ehi_mask & ERR_FUSERMASK*/
   }
   if (hand.ehi_flag & EXCEPTION_HANDLER_FDESCRIPTOR) {
    /* Unwind the stack to the caller-site. */
    if (!eh_return(&info,(struct cpu_context *)context,
                    EH_FRESTRICT_USERSPACE|EH_FDONT_UNWIND_SIGFRAME))
         return false;
    /* Override the IP to use the entry point.  */
    context->c_eip = (uintptr_t)hand.ehi_entry;
    assert(hand.ehi_desc.ed_type == EXCEPT_DESC_TYPE_BYPASS); /* XXX: What should we do here? */
    /* Allocate the stack-save area. */
    CPU_CONTEXT_SP(*context) -= hand.ehi_desc.ed_safe;
    if (!(hand.ehi_desc.ed_flags&EXCEPT_DESC_FDEALLOC_CONTINUE)) {
     /* Must allocate stack memory at the back and copy data there. */
     sp -= hand.ehi_desc.ed_safe;
     validate_writable((void *)sp,hand.ehi_desc.ed_safe);
     COMPILER_WRITE_BARRIER();
     memcpy((void *)sp,(void *)CPU_CONTEXT_SP(*context),hand.ehi_desc.ed_safe);
     CPU_CONTEXT_SP(*context) = sp;
    }
   } else {
    /* Jump to the entry point of this exception handler. */
    if (!eh_jmp(&info,(struct cpu_context *)context,
                      (uintptr_t)hand.ehi_entry,
                 EH_FRESTRICT_USERSPACE))
         return false;
    /* Restore the original SP to restore the stack as
     * it were when the exception originally occurred.
     * Without this, it would be impossible to continue
     * execution after an exception occurred. */
    CPU_CONTEXT_SP(*context) = sp;
   }
   /* Force user-space target. */
   context->c_iret.ir_cs     |= 3;
   context->c_iret.ir_useresp = context->c_gpregs.gp_esp;
   context->c_gpregs.gp_esp   = (uintptr_t)PERTASK_GET(this_task.t_stackend);
   context->c_iret.ir_eflags |= EFLAGS_IF;
   context->c_iret.ir_eflags &= ~(EFLAGS_TF|EFLAGS_IOPL(3)|
                                  EFLAGS_NT|EFLAGS_RF|EFLAGS_VM|
                                  EFLAGS_AC|EFLAGS_VIF|EFLAGS_VIP|
                                  EFLAGS_ID);
   return true;
  }
  /* Continue unwinding the stack. */
  if (!eh_return(&info,(struct cpu_context *)context,
                  EH_FRESTRICT_USERSPACE|EH_FDONT_UNWIND_SIGFRAME))
       return false;
 }
 return false;
}
DEFINE_PUBLIC_ALIAS(__error_rethrow_at,libc_error_rethrow_at);





DEFINE_SYSCALL3(xunwind,
                USER UNCHECKED struct x86_usercontext *,context,
                u16,exception_code,int,ip_is_after_faulting) {
 /* TODO: This system call is incorrectly being used for unwinding.
  *       Rename this one to `xunwind_except' and refactor add a new
  *       one called `xunwind' that only unwinds a single stack frame. */
 struct cpu_hostcontext_user unwind;
 validate_writable(context,sizeof(struct x86_usercontext));
#ifdef CONFIG_X86_SEGMENTATION
 memcpy(&unwind.c_gpregs,context,sizeof(struct x86_gpregs)+sizeof(struct x86_segments));
 if (!unwind.c_iret.ir_cs) unwind.c_iret.ir_cs = X86_USER_CS;
 if (!unwind.c_iret.ir_ss) unwind.c_iret.ir_ss = X86_USER_DS;
 if (!unwind.c_segments.sg_ds) unwind.c_segments.sg_ds = X86_USER_DS;
 if (!unwind.c_segments.sg_es) unwind.c_segments.sg_es = X86_USER_DS;
 if (!unwind.c_segments.sg_fs) unwind.c_segments.sg_fs = X86_SEG_FS;
 if (!unwind.c_segments.sg_gs) unwind.c_segments.sg_gs = X86_SEG_GS;
#else
 memcpy(&unwind.c_gpregs,context,sizeof(struct x86_gpregs));
#endif
 unwind.c_iret.ir_cs     = context->c_cs;
 unwind.c_iret.ir_ss     = context->c_ss;
 unwind.c_iret.ir_eip    = context->c_eip;
 unwind.c_iret.ir_eflags = context->c_eflags;
 if (!libc_error_rethrow_at_user(&unwind,exception_code,ip_is_after_faulting))
      return -EPERM; /* No handler found. */
 COMPILER_BARRIER();
 /* Copy the new context back to user-space. */
#ifdef CONFIG_X86_SEGMENTATION
 memcpy(context,&unwind.c_gpregs,sizeof(struct x86_gpregs)+sizeof(struct x86_segments));
#else
 memcpy(&unwind.c_gpregs,context,sizeof(struct x86_gpregs));
#endif
 context->c_cs     = unwind.c_iret.ir_cs;
 context->c_ss     = unwind.c_iret.ir_ss;
 context->c_eip    = unwind.c_iret.ir_eip;
 context->c_eflags = unwind.c_iret.ir_eflags;
 return 0;
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_EXCEPT_C */
