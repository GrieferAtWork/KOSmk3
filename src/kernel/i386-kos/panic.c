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
#ifndef GUARD_KERNEL_I386_KOS_PANIC_C
#define GUARD_KERNEL_I386_KOS_PANIC_C 1
#define _KOS_SOURCE 1

#include "scheduler.h"

#include <kos/intrin.h>
#include <hybrid/compiler.h>
#include <kernel/panic.h>
#include <kernel/debug.h>
#include <i386-kos/ipi.h>
#include <hybrid/section.h>
#include <asm/cpu-flags.h>

DECL_BEGIN

PRIVATE ATTR_COLDBSS ATOMIC_DATA struct cpu *panic_cpu = NULL;

PUBLIC ATTR_NOTHROW void KCALL kernel_panic_recover(void) {
#ifdef __x86_64__
 /* Try to fix a corrupted GS_BASE value. */
 u64 gs_base = __rdgsbaseq();
 if (gs_base < KERNEL_BASE) {
  debug_printf("[PANIC] Corrupted gs_base = %p. Trying to recover\n",gs_base);
  /* Maybe just a missing / superfluous swapgs? */
  __swapgs();
  gs_base = __rdgsbaseq();
  if (gs_base < KERNEL_BASE) {
   /* This is bad... */
   debug_printf("[PANIC] user_gs_base = %p isn't it, either\n",gs_base);
#ifndef CONFIG_NO_SMP
   if (cpu_count == 1)
#endif
   {
    /* Only one CPU, so we know that's us! */
    gs_base = (u64)cpu_vector[0]->c_running;
   }
#ifndef CONFIG_NO_SMP
   else {
    /* XXX: Use the LAPIC ID to find our CPU? */
   }
#endif

   if (gs_base < KERNEL_BASE) {
    /* Just go with the boot task. - We'll shut down
     * everything anyways, so it shouldn't hurt too much... */
    debug_printf("[PANIC] CPU->c_running = %p is broken, too\n",gs_base);
    gs_base = (u64)&_boot_task;
   }
   __wrgsbaseq(gs_base);
   debug_printf("[PANIC] Recovered gs_base = %p\n",gs_base);
  }
 }
#endif
}


PUBLIC ATTR_NOTHROW NOIRQ bool KCALL kernel_panic_shootdown(void) {
 struct x86_ipi command;
 struct task *caller = THIS_TASK;
 struct cpu *mycpu = THIS_CPU;
 struct cpu *oldcpu;
 PREEMPTION_DISABLE();
 oldcpu = ATOMIC_XCH(panic_cpu,mycpu);
 if (oldcpu != NULL) {
  if (oldcpu == mycpu)
      return false; /* Recursion (oh ooh...) */
  for (;;) asm("hlt");
 }

 /* Send an IPI to every other thread to get them to stop. */
 command.ipi_type = X86_IPI_PANIC_SHUTDOWN;
 command.ipi_flag = X86_IPI_FNORMAL;
 x86_ipi_broadcast(&command,false);

 /* Delete scheduling for all thread other than the caller. */
 mycpu->c_running = caller;
 caller->t_sched.sched_ring.re_next = caller;
 caller->t_sched.sched_ring.re_prev = caller;
 mycpu->c_sleeping = NULL;
 mycpu->c_pending = NULL;

 return true;
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_PANIC_C */
