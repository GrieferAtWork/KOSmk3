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
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4
INTERN_ENTRY(X86_IRQ_ATA0)
	pushl_cfi_r %eax
	pushl_cfi_r %ecx
	pushl_cfi_r %edx
#ifndef CONFIG_NO_X86_SEGMENTATION
	/* Save segment registers. */
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs
	pushl_cfi_r %gs
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	call   x86_load_segments
	incl   Ata_BusInterruptCounter + 0

	pushl_cfi $Ata_BusInterruptSignal + 0*ASYNC_SIG_SIZE
	call   async_sig_broadcast
	.cfi_adjust_cfa_offset -4

	/* IMPORTANT: Must read from the status port to clear the interrupt flag of the drive!
	 *            The OSDEV wiki doesn't mention this, but if you don't, you won't be
	 *            getting any additional interrupts.
	 * However, StackOverflow comes to the rescue:
	 *  https://stackoverflow.com/questions/7487312/what-is-the-proper-way-to-acknowledge-an-ata-ide-interrupt
	 */
	movw   $(ATA_STATUS(ATA_BUS_PRIMARY)), %dx
	inb    %dx, %al /* inb(ATA_STATUS(ATA_BUS_PRIMARY)); */
1:	movb   $(X86_PIC_CMD_EOI), %al
	outb   %al, $(X86_PIC2_CMD) /* outb(X86_PIC2_CMD,X86_PIC_CMD_EOI); */
	outb   %al, $(X86_PIC1_CMD) /* outb(X86_PIC1_CMD,X86_PIC_CMD_EOI); */
#ifndef CONFIG_NO_X86_SEGMENTATION
	popl_cfi_r %gs
	popl_cfi_r %fs
	popl_cfi_r %es
	popl_cfi_r %ds
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	popl_cfi_r %edx
	popl_cfi_r %ecx
	popl_cfi_r %eax
	iret
SYMEND(X86_IRQ_ATA0)
INTERN_ENTRY(X86_IRQ_ATA1)
	call   x86_check_spurious_interrupt_PIC2
	pushl_cfi_r %eax
	pushl_cfi_r %ecx
	pushl_cfi_r %edx
#ifndef CONFIG_NO_X86_SEGMENTATION
	/* Save segment registers. */
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs
	pushl_cfi_r %gs
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	call   x86_load_segments
	incl   Ata_BusInterruptCounter + 4

	pushl_cfi $Ata_BusInterruptSignal + 1*ASYNC_SIG_SIZE
	call   async_sig_broadcast
	.cfi_adjust_cfa_offset -4

	movw   $(ATA_STATUS(ATA_BUS_SECONDARY)), %dx
	inb    %dx, %al /* inb(ATA_STATUS(ATA_BUS_SECONDARY)); */
	jmp    1b
SYMEND(X86_IRQ_ATA1)
	.cfi_endproc

#ifdef CONFIG_HAVE_DEV_PS2
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4
INTERN_ENTRY(X86_IRQ_KBD)
	pushl_cfi_r %eax
	pushl_cfi_r %ecx
	pushl_cfi_r %edx
#ifndef CONFIG_NO_X86_SEGMENTATION
	/* Save segment registers. */
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs
	pushl_cfi_r %gs
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	call   x86_load_segments
	call   ps2_irq_1
	movb   $(X86_PIC_CMD_EOI), %al
	outb   %al, $(X86_PIC1_CMD) /* outb(X86_PIC1_CMD,X86_PIC_CMD_EOI); */
#ifndef CONFIG_NO_X86_SEGMENTATION
	popl_cfi_r %gs
	popl_cfi_r %fs
	popl_cfi_r %es
	popl_cfi_r %ds
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	popl_cfi_r %edx
	popl_cfi_r %ecx
	popl_cfi_r %eax
	iret
SYMEND(X86_IRQ_KBD)

INTERN_ENTRY(X86_IRQ_PS2M)
	pushl_cfi_r %eax
	pushl_cfi_r %ecx
	pushl_cfi_r %edx
#ifndef CONFIG_NO_X86_SEGMENTATION
	/* Save segment registers. */
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs
	pushl_cfi_r %gs
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	call   x86_load_segments
	call   ps2_irq_2
	movb   $(X86_PIC_CMD_EOI), %al
	outb   %al, $(X86_PIC2_CMD) /* outb(X86_PIC2_CMD,X86_PIC_CMD_EOI); */
	outb   %al, $(X86_PIC1_CMD) /* outb(X86_PIC1_CMD,X86_PIC_CMD_EOI); */
#ifndef CONFIG_NO_X86_SEGMENTATION
	popl_cfi_r %gs
	popl_cfi_r %fs
	popl_cfi_r %es
	popl_cfi_r %ds
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	popl_cfi_r %edx
	popl_cfi_r %ecx
	popl_cfi_r %eax
	iret
SYMEND(X86_IRQ_PS2M)
	.cfi_endproc
#endif /* CONFIG_HAVE_DEV_PS2 */

.guard_end = .
X86_DEFINE_INTERRUPT_GUARD(.guard_begin,.guard_end)



















