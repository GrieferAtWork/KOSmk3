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

#include <hybrid/compiler.h>
#include <hybrid/asm.h>
#include <asm/cfi.h>
#include <i386-kos/interrupt.h>
#include <i386-kos/pic.h>
#include <dev/devconfig.h>
#include <sched/async_signal.h>
#include "../src/core/ata.h"


.section .text


.guard_begin = .
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
INTERN_ENTRY(X86_IRQ_ATA0)
	irq_enter
	pushq_cfi_r %rax
	pushq_cfi_r %rcx
	pushq_cfi_r %rdx
	pushq_cfi_r %rsi
	pushq_cfi_r %rdi
	pushq_cfi_r %r8
	pushq_cfi_r %r9
	pushq_cfi_r %r10
	pushq_cfi_r %r11
	incl   Ata_BusInterruptCounter + 0
	leaq   Ata_BusInterruptSignal  + 0*ASYNC_SIG_SIZE, %rdi
	call   async_sig_broadcast
	/* IMPORTANT: Must read from the status port to clear the interrupt flag of the drive!
	 *            The OSDEV wiki doesn't mention this, but if you don't, you won't be
	 *            getting any additional interrupts.
	 * However, StackOverflow comes to the rescue:
	 *  https://stackoverflow.com/questions/7487312/what-is-the-proper-way-to-acknowledge-an-ata-ide-interrupt */
	movw   $(ATA_STATUS(ATA_BUS_PRIMARY)), %dx
	inb    %dx, %al /* inb(ATA_STATUS(ATA_BUS_PRIMARY)); */
1:	movb   $(X86_PIC_CMD_EOI), %al
	outb   %al, $(X86_PIC2_CMD) /* outb(X86_PIC2_CMD,X86_PIC_CMD_EOI); */
	outb   %al, $(X86_PIC1_CMD) /* outb(X86_PIC1_CMD,X86_PIC_CMD_EOI); */
	popq_cfi_r  %r11
	popq_cfi_r  %r10
	popq_cfi_r  %r9
	popq_cfi_r  %r8
	popq_cfi_r  %rdi
	popq_cfi_r  %rsi
	popq_cfi_r  %rdx
	popq_cfi_r  %rcx
	popq_cfi_r  %rax
	irq_leave
SYMEND(X86_IRQ_ATA0)
INTERN_ENTRY(X86_IRQ_ATA1)
	irq_enter
	call   x86_check_spurious_interrupt_PIC2
	pushq_cfi_r %rax
	pushq_cfi_r %rcx
	pushq_cfi_r %rdx
	pushq_cfi_r %rsi
	pushq_cfi_r %rdi
	pushq_cfi_r %r8
	pushq_cfi_r %r9
	pushq_cfi_r %r10
	pushq_cfi_r %r11
	incl   Ata_BusInterruptCounter + 4
	leaq   Ata_BusInterruptSignal  + 1*ASYNC_SIG_SIZE, %rdi
	call   async_sig_broadcast
	movw   $(ATA_STATUS(ATA_BUS_SECONDARY)), %dx
	inb    %dx, %al /* inb(ATA_STATUS(ATA_BUS_SECONDARY)); */
	jmp    1b
SYMEND(X86_IRQ_ATA1)
	.cfi_endproc




#ifdef CONFIG_HAVE_DEV_PS2
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
INTERN_ENTRY(X86_IRQ_KBD)
	irq_enter
	pushq_cfi_r %rax
	pushq_cfi_r %rcx
	pushq_cfi_r %rdx
	pushq_cfi_r %rsi
	pushq_cfi_r %rdi
	pushq_cfi_r %r8
	pushq_cfi_r %r9
	pushq_cfi_r %r10
	pushq_cfi_r %r11
	call   ps2_irq_1
	movb   $(X86_PIC_CMD_EOI), %al
	outb   %al, $(X86_PIC1_CMD) /* outb(X86_PIC1_CMD,X86_PIC_CMD_EOI); */
	popq_cfi_r  %r11
	popq_cfi_r  %r10
	popq_cfi_r  %r9
	popq_cfi_r  %r8
	popq_cfi_r  %rdi
	popq_cfi_r  %rsi
	popq_cfi_r  %rdx
	popq_cfi_r  %rcx
	popq_cfi_r  %rax
	irq_leave
SYMEND(X86_IRQ_KBD)

INTERN_ENTRY(X86_IRQ_PS2M)
	irq_enter
	pushq_cfi_r %rax
	pushq_cfi_r %rcx
	pushq_cfi_r %rdx
	pushq_cfi_r %rsi
	pushq_cfi_r %rdi
	pushq_cfi_r %r8
	pushq_cfi_r %r9
	pushq_cfi_r %r10
	pushq_cfi_r %r11
	call   ps2_irq_2
	movb   $(X86_PIC_CMD_EOI), %al
	outb   %al, $(X86_PIC2_CMD) /* outb(X86_PIC2_CMD,X86_PIC_CMD_EOI); */
	outb   %al, $(X86_PIC1_CMD) /* outb(X86_PIC1_CMD,X86_PIC_CMD_EOI); */
	popq_cfi_r  %r11
	popq_cfi_r  %r10
	popq_cfi_r  %r9
	popq_cfi_r  %r8
	popq_cfi_r  %rdi
	popq_cfi_r  %rsi
	popq_cfi_r  %rdx
	popq_cfi_r  %rcx
	popq_cfi_r  %rax
	irq_leave
SYMEND(X86_IRQ_PS2M)
	.cfi_endproc
#endif /* CONFIG_HAVE_DEV_PS2 */

.guard_end = .
X86_DEFINE_INTERRUPT_GUARD(.guard_begin,.guard_end)



















