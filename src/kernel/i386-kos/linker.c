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
#ifndef GUARD_KERNEL_I386_KOS_LINKER_C
#define GUARD_KERNEL_I386_KOS_LINKER_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <kernel/user.h>
#include <fs/linker.h>
#include <unwind/eh_frame.h>
#include <unwind/linker.h>
#include <syscall.h>
#include <kos/thread.h>
#include <string.h>

#include "posix_signals.h"

DECL_BEGIN

PUBLIC void KCALL
application_loaduserinit(struct application *__restrict app,
                         struct cpu_hostcontext_user *__restrict context) {
 void **esp = (void **)context->c_esp;
 validate_writable(esp-1,sizeof(*esp));
 *--esp = (void *)context->c_eip;     /* pushl %eip */
 esp = application_enuminit(app,esp); /* pushl init... */
 context->c_eip = (uintptr_t)*esp++;  /* popl  %eip */
 context->c_esp = (uintptr_t)esp;
}
PUBLIC void KCALL
application_loaduserfini(struct application *__restrict app,
                         struct cpu_hostcontext_user *__restrict context) {
 void **esp = (void **)context->c_esp;
 validate_writable(esp-1,sizeof(*esp));
 *--esp = (void *)context->c_eip;     /* pushl %eip */
 esp = application_enumfini(app,esp); /* pushl fini... */
 context->c_eip = (uintptr_t)*esp++;  /* popl  %eip */
 context->c_esp = (uintptr_t)esp;
}



PUBLIC bool KCALL
linker_unwind_user(struct cpu_hostcontext_user *__restrict context) {
 /* TODO: FPU support? */
 struct fde_info fde;
 if (!linker_findfde(context->c_eip,&fde))
      return false;
 context->c_gpregs.gp_esp = context->c_esp;
 if (!eh_return(&fde,(struct cpu_context *)context,
                EH_FRESTRICT_USERSPACE|
                EH_FDONT_UNWIND_SIGFRAME))
      return false;
 context->c_esp = context->c_gpregs.gp_esp;
 /* Special case: unwind a signal frame. */
 if (context->c_eip ==
    (PERTASK(x86_sysbase)+X86_ENCODE_PFSYSCALL(SYS_sigreturn))) {
  struct signal_frame *frame;
  frame = (struct signal_frame *)(context->c_esp-4);
  /* XXX: Restore signal mask during exception unwind? */
  memcpy(&context->c_gpregs,
         &frame->sf_return.m_context.c_gpregs,
         sizeof(struct x86_gpregs32)
#ifdef CONFIG_X86_SEGMENTATION
         +
         sizeof(struct x86_segments32)
#endif /* CONFIG_X86_SEGMENTATION */
         );
  context->c_iret.ir_eflags = frame->sf_return.m_context.c_eflags;
  context->c_iret.ir_eip    = frame->sf_return.m_context.c_eip;
  context->c_esp            = context->c_gpregs.gp_esp;
#ifdef CONFIG_X86_SEGMENTATION
  context->c_iret.ir_cs     = frame->sf_return.m_context.c_cs;
  context->c_iret.ir_ss     = frame->sf_return.m_context.c_ss;
#endif /* CONFIG_X86_SEGMENTATION */
  /* XXX: Restore FPU context? */
 }
 return true;
}





DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_LINKER_C */
