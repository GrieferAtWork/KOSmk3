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
#include <kos/i386-kos/context.h>
#include <sched/task.h>

#include <unwind/eh_frame.h>
#include <kos/context.h>

DECL_BEGIN


INTDEF void ASMCALL x86_interrupt_except(void);
INTDEF void ASMCALL x86_interrupt_except_syscall(void);
INTDEF void ASMCALL x86_interrupt_except_syscall_except(void);

/* Define the X86 interrupt exception guard. */
PUBLIC struct except_desc const x86_interrupt_guard = {
    .ed_handler = (void *)&x86_interrupt_except,
    .ed_type    = EXCEPT_DESC_TYPE_BYPASS,
    .ed_flags   = (EXCEPT_DESC_FDEALLOC_CONTINUE|
                   EXCEPT_DESC_FDISABLE_PREEMPTION),
    .ed_safe    = X86_IRREGS_HOST32_SIZE
};
PUBLIC struct except_desc const x86_syscall_guard = {
    .ed_handler = (void *)&x86_interrupt_except_syscall,
    .ed_type    = EXCEPT_DESC_TYPE_BYPASS,
    .ed_flags   = (EXCEPT_DESC_FDEALLOC_CONTINUE|
                   EXCEPT_DESC_FDISABLE_PREEMPTION),
    .ed_safe    = X86_IRREGS_HOST32_SIZE
};
PUBLIC struct except_desc const x86_syscall_except_guard = {
    .ed_handler = (void *)&x86_interrupt_except_syscall_except,
    .ed_type    = EXCEPT_DESC_TYPE_BYPASS,
    .ed_flags   = (EXCEPT_DESC_FDEALLOC_CONTINUE|
                   EXCEPT_DESC_FDISABLE_PREEMPTION),
    .ed_safe    = X86_IRREGS_HOST32_SIZE
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


INTDEF struct x86_idtentry _idt_start[256];
INTERN ATTR_FREETEXT void KCALL x86_interrupt_initialize(void) {
 struct x86_idt_pointer idt;
 idt.ip_idt   = _idt_start;
 idt.ip_limit = sizeof(_idt_start)-1;
 __asm__ __volatile__("lidt %0" : : "g" (idt) : "memory");
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_INTERRUPT_C */
