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
#ifndef GUARD_KERNEL_I386_KOS_DOUBLE_FAULT_C
#define GUARD_KERNEL_I386_KOS_DOUBLE_FAULT_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/section.h>
#include <kernel/interrupt.h>
#include <sched/task.h>
#include <asm/cpu-flags.h>
#include <kernel/debug.h>
#include <unwind/eh_frame.h>
#include <unwind/linker.h>
#include <kos/context.h>
#include <sched/pid.h>
#include <i386-kos/tss.h>
#include <except.h>
#include <string.h>
#include <sched/mutex.h>
#include <kernel/vm.h>

DECL_BEGIN


#ifndef __x86_64__
__asm__(".hidden x86_double_fault_handler\n"
        ".global x86_double_fault_handler\n"
        ".section .text\n"
        "x86_double_fault_handler:\n"
        "    call c_x86_double_fault_handler\n"
        "    iret\n"
        "    jmp x86_double_fault_handler\n"
        ".size x86_double_fault_handler, . - x86_double_fault_handler\n"
        );

INTDEF void KCALL print_tb(struct cpu_context *__restrict context);

INTERN void FCALL c_x86_double_fault_handler(void) {
 struct cpu_context context;
 struct exception_info *info = error_info();
 struct x86_tss *tss = &PERCPU(x86_cputss);
 /* Fill in a CPU CONTEXT of where the exception happened.
  * We'll use this to construct an exception record, as well
  * as starting point for jumping back to where the error
  * originated from. */
 context.c_gpregs.gp_edi  = tss->t_edi;
 context.c_gpregs.gp_esi  = tss->t_esi;
 context.c_gpregs.gp_ebp  = tss->t_ebp;
 context.c_gpregs.gp_esp  = tss->t_esp;
 context.c_gpregs.gp_ebx  = tss->t_ebx;
 context.c_gpregs.gp_edx  = tss->t_edx;
 context.c_gpregs.gp_ecx  = tss->t_ecx;
 context.c_gpregs.gp_eax  = tss->t_eax;
#ifndef CONFIG_NO_X86_SEGMENTATION
 context.c_segments.sg_ds = tss->t_ds;
 context.c_segments.sg_es = tss->t_es;
 context.c_segments.sg_fs = tss->t_fs;
 context.c_segments.sg_gs = tss->t_gs;
#endif /* !CONFIG_NO_X86_SEGMENTATION */
 context.c_iret.ir_eip    = tss->t_eip;
 context.c_iret.ir_cs     = tss->t_cs;
 context.c_iret.ir_eflags = tss->t_eflags;

#if 1
 debug_printf("#DF in %X\n",posix_gettid());
 debug_printf("eip    = %p\n",context.c_eip);
 debug_printf("eflags = %p\n",context.c_eflags);
 debug_printf("eax    = %p\n",context.c_gpregs.gp_eax);
 debug_printf("ecx    = %p\n",context.c_gpregs.gp_ecx);
 debug_printf("edx    = %p\n",context.c_gpregs.gp_edx);
 debug_printf("ebx    = %p\n",context.c_gpregs.gp_ebx);
 debug_printf("esp    = %p\n",context.c_gpregs.gp_esp);
 debug_printf("ebp    = %p\n",context.c_gpregs.gp_ebp);
 debug_printf("esi    = %p\n",context.c_gpregs.gp_esi);
 debug_printf("edi    = %p\n",context.c_gpregs.gp_edi);
 debug_printf("es     = %.4I16X\n",tss->t_es);
 debug_printf("cs     = %.4I16X\n",context.c_iret.ir_cs);
 debug_printf("ss     = %.4I16X\n",tss->t_ss);
 debug_printf("ds     = %.4I16X\n",tss->t_ds);
 debug_printf("fs     = %.4I16X\n",tss->t_fs);
 debug_printf("gs     = %.4I16X\n",tss->t_gs);
 debug_printf("cr2    = %p\n",XBLOCK({
  void *r;
  __asm__("movl %%cr2, %0" : "=r" (r));
  XRETURN r;
 }));
#if 0
 task_disconnect();
 print_tb(&context);
 for (;;) asm("hlt");
#endif
#endif

 info->e_error.e_code = E_UNHANDLED_INTERRUPT;
 info->e_error.e_flag = ERR_FRESUMABLE;
 memset(&info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_unhandled_interrupt.ui_intcode = X86_E_SYSTEM_DF & 0xff;
 memcpy(&info->e_context,&context,sizeof(struct cpu_context));

 /* Figure out if this is a stack overflow.
  * For this, we check if the current CR2 value
  * points into the first page below the end of
  * the current thread's kernel stack, as well
  * as that the return ESP value is located below
  * the allocated stack end, too. */
 if (context.c_esp <= (uintptr_t)PERTASK_GET(this_task.t_stackmin)) {
  u32 cr2;
  __asm__ __volatile__("movl %%cr2, %0\n" : "=r" (cr2));
  if (cr2          <  (uintptr_t)PERTASK_GET(this_task.t_stackmin) &&
      cr2+PAGESIZE >= (uintptr_t)PERTASK_GET(this_task.t_stackmin)) {
   /* Apparently, this whole problem was caused by a kernel stack-overflow... */
   info->e_error.e_code               = E_STACK_OVERFLOW;
   info->e_error.e_flag              &= ~ERR_FRESUMABLE;
   info->e_error.e_segfault.sf_vaddr  = (void *)cr2;
   info->e_error.e_segfault.sf_reason = 0; /* XXX: Not strictly correct... */
  }
 }

 {
  uintptr_t base = (uintptr_t)PERTASK_GET(this_task.t_stackmin);
  uintptr_t end = (uintptr_t)PERTASK_GET(this_task.t_stackend);
  if (base < context.c_esp)
      base = context.c_esp;
  debug_printf("SEARCH %p...%p\n",base,end-1);
  debug_printf("STACK  %p...%p\n",
               PERTASK_GET(this_task.t_stackmin),
              (uintptr_t)PERTASK_GET(this_task.t_stackend)-1);
  while (base < end) {
   uintptr_t ptr = *(uintptr_t *)base;
   base += sizeof(uintptr_t);
   if (ptr >= (uintptr_t)kernel_start && ptr < (uintptr_t)kernel_end_raw) {
    debug_printf("%%{vinfo:kernel-i686-kos.bin:%p:%p:%%f(%%l,%%c) : %%n : %%p : Potential kernel address}\n",
                 ptr,ptr);
   }
  }
 }

#if 1
#ifdef CONFIG_VM_USE_RWLOCK
 vm_kernel.vm_lock.rw_state = 0;
 THIS_VM->vm_lock.rw_state  = 0;
#else
 vm_kernel.vm_lock.m_ind = 0;
 THIS_VM->vm_lock.m_ind  = 0;
#endif
 COMPILER_WRITE_BARRIER();
 error_unhandled_exception();
#endif

 /* Unwind the stack. */
 /* TODO: This code is broken. - Rewrite it! */
 {
  struct fde_info fde;
  struct exception_handler_info hand;
  bool is_first = true;
  uintptr_t sp = context.c_esp;
  /* Safe the original stack-pointer. */
  for (;;) {
   /* NOTE: This code may look similar to `__error_rethrow_at()',
    *       but there are 2 _IMPORTANT_ distinctions:
    *         #1: We constantly re-load the SP value
    *             at every call site, rather than only
    *             loading it once before the loop.
    *             That way, we automatically de-allocate
    *             stack memory that isn't actually used
    *             to describe local variables at our
    *             exception call-site.
    *         #2: Instead of directly jumping back to
    *             where the exception happened, we instead
    *             safe the new context into the TSS that's
    *             going to be loaded once our task gate returns. */
   if (info->e_error.e_code == E_STACK_OVERFLOW) {
    /* Deallocate frames to make space to STACK_OVERFLOW handling.
     * NOTE: Although this might break tracebacks showing where
     *       a stack-overflow originated from, it is required for
     *       cleanup code to even get a chance at running, considering
     *       that if we didn't do this, it would just fault again
     *       immediately due to lack of any stack memory needed by
     *       cleanup.
     * XXX:  Maybe solve this using some special, secondary stack
     *       that is only ever used during cleanup of stack overflow
     *       exceptions?
     *       Some per-CPU stack that is acquired here, and released
     *       in one of 3 ways:
     *        - task_exit() is called
     *        - The exception is propagated to user-space
     *        - Another thread (on the same core) needs the
     *          stack. However looking at the restore-state
     *          of the task currently using that stack, its
     *          ESP register does not point inside, or into
     *          the page immediately in front.
     *       If the stack is in use when a stack-overflow happens,
     *       SP is restored as it is here (meaning that frames are
     *       de-allocated as the stack is unwound)
     */
    sp = context.c_esp;
   }
   if (!is_first) --context.c_eip;
   if (!linker_findfde_consafe(context.c_eip,&fde)) goto no_handler;
   is_first = false;
   /* Search for a suitable exception handler (in reverse order!). */
   if (linker_findexcept_consafe(context.c_eip,error_code(),&hand)) {
    if (hand.ehi_flag & EXCEPTION_HANDLER_FUSERFLAGS)
        info->e_error.e_flag |= hand.ehi_mask & ERR_FUSERMASK;
    if (hand.ehi_flag & EXCEPTION_HANDLER_FDESCRIPTOR) {
     /* Unwind the stack to the caller-site. */
     if (!eh_return(&fde,&context,EH_FDONT_UNWIND_SIGFRAME))
          goto no_handler;
     /* Override the IP to use the entry point.  */
     context.c_eip = (uintptr_t)hand.ehi_entry;
     assert(hand.ehi_desc.ed_type == EXCEPT_DESC_TYPE_BYPASS); /* XXX: What should we do here? */
     /* Allocate the stack-save area. */
     context.c_esp -= hand.ehi_desc.ed_safe;
     if (!(hand.ehi_desc.ed_flags & EXCEPT_DESC_FDEALLOC_CONTINUE)) {
      /* Must allocate stack memory at the back and copy data there. */
      sp -= hand.ehi_desc.ed_safe;
      memcpy((void *)sp,(void *)context.c_esp,hand.ehi_desc.ed_safe);
      context.c_esp = sp;
     }
     if (hand.ehi_desc.ed_flags & EXCEPT_DESC_FDISABLE_PREEMPTION)
         context.c_eflags &= ~EFLAGS_IF;
    } else {
     /* Jump to the entry point of this exception handler. */
     if (!eh_jmp(&fde,&context,(uintptr_t)hand.ehi_entry,EH_FNORMAL))
          goto no_handler;
     /* Restore the original SP to restore the stack as
      * it were when the exception originally occurred.
      * Without this, it would be impossible to continue
      * execution after an exception occurred. */
     context.c_esp = sp;
    }
    /* Set the new context as what the task gate will return to. */
    tss->t_edi    = context.c_gpregs.gp_edi;
    tss->t_esi    = context.c_gpregs.gp_esi;
    tss->t_ebp    = context.c_gpregs.gp_ebp;
    tss->t_esp    = context.c_gpregs.gp_esp;
    tss->t_ebx    = context.c_gpregs.gp_ebx;
    tss->t_edx    = context.c_gpregs.gp_edx;
    tss->t_ecx    = context.c_gpregs.gp_ecx;
    tss->t_eax    = context.c_gpregs.gp_eax;
#ifndef CONFIG_NO_X86_SEGMENTATION
    tss->t_ds     = context.c_segments.sg_ds;
    tss->t_es     = context.c_segments.sg_es;
    tss->t_fs     = context.c_segments.sg_fs;
    tss->t_gs     = context.c_segments.sg_gs;
#endif /* !CONFIG_NO_X86_SEGMENTATION */
    tss->t_eip    = context.c_iret.ir_eip;
    tss->t_cs     = context.c_iret.ir_cs;
    tss->t_eflags = context.c_iret.ir_eflags;
    return;
   }
   /* Continue unwinding the stack. */
   if (!eh_return(&fde,&context,EH_FDONT_UNWIND_SIGFRAME))
        goto no_handler;
  }
no_handler:
  error_unhandled_exception();
 }
}
#else
/* TODO */
#endif


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_DOUBLE_FAULT_C */
