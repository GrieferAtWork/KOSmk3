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
#ifndef GUARD_KERNEL_I386_KOS_INTERRUPT_C
#define GUARD_KERNEL_I386_KOS_INTERRUPT_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/section.h>
#include <asm/cpu-flags.h>
#include <i386-kos/idt_pointer.h>
#include <i386-kos/interrupt.h>
#include <i386-kos/pic.h>
#include <except.h>
#include <string.h>
#include <kernel/debug.h>
#include <kernel/interrupt.h>
#include <kos/i386-kos/bits/cpu-context.h>
#include <sched/task.h>

#include <unwind/eh_frame.h>
#include <kos/context.h>

DECL_BEGIN


INTDEF void ASMCALL x86_interrupt_except(void);

/* Define the X86 interrupt exception guard. */
PUBLIC struct exception_descriptor const x86_interrupt_guard = {
    .ed_handler = (void *)&x86_interrupt_except,
    .ed_type    = EXCEPTION_DESCRIPTOR_TYPE_BYPASS,
    .ed_flags   = (EXCEPTION_DESCRIPTOR_FDEALLOC_CONTINUE|
                   EXCEPTION_DESCRIPTOR_FDISABLE_PREEMPTION),
#ifdef __x86_64__
    .ed_safe    = X86_IRREGS64_SIZE
#else
    .ed_safe    = X86_IRREGS_HOST32_SIZE
#endif
};


STATIC_ASSERT(sizeof(struct x86_idtentry) == X86_IDTENTRY_SIZE);

PUBLIC ATTR_PERCPU struct x86_spurious_interrupts x86_spurious_interrupts = { 0, 0, 0 };
INTERN void KCALL x86_pic1_spur(void) {
 debug_printf("Spurious interrupt on PIC #1\n");
 ++PERCPU(x86_spurious_interrupts.sp_pic1);
}
INTERN void KCALL x86_pic2_spur(void) {
 debug_printf("Spurious interrupt on PIC #2\n");
 ++PERCPU(x86_spurious_interrupts.sp_pic2);
}
INTERN void KCALL x86_apic_spur(void) {
 debug_printf("Spurious interrupt on APIC\n");
 ++PERCPU(x86_spurious_interrupts.sp_apic);
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_INTERRUPT_C */
