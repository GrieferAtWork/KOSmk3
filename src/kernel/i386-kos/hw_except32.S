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
#include <asm/cfi.h>
#include <kernel/interrupt.h>
#include <i386-kos/gdt.h>
#include <i386-kos/vm86.h>
#include <kos/except.h>


/* Define an exception guard for propagating all hardware exceptions. */
X86_DEFINE_INTERRUPT_GUARD(.hwexcept_begin,.hwexcept_end)

.section .text.interrupt_entry
.hwexcept_begin = .

.macro define_hardware_exception_handler id, name, has_ecode=0, use_pushal=1
INTERN_ENTRY(irq_\id)
	.cfi_startproc simple
	.cfi_signal_frame
.if \has_ecode != 0
	.cfi_def_cfa %esp,    4*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4
	/* Move the exception code above the CPU-state. */
#ifndef CONFIG_NO_X86_SEGMENTATION
	popl_cfi %ss:-(4+X86_GPREGS_SIZE+X86_SEGMENTS_SIZE)(%esp) /* ECODE */
#else
	popl_cfi %ss:-(4+X86_GPREGS_SIZE)(%esp) /* ECODE */
#endif
.else
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4
.endif

#ifndef CONFIG_NO_X86_SEGMENTATION
	/* Save segment registers. */
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs
	pushl_cfi_r %gs
#endif /* !CONFIG_NO_X86_SEGMENTATION */

	/* Save general-purpose registers. */
.if \use_pushal != 0
	pushal_cfi_r
.else
	pushl_cfi_r %eax
	pushl_cfi_r %ecx
	pushl_cfi_r %edx
	pushl_cfi_r %ebx
	pushl_cfi   %esp
	pushl_cfi_r %ebp
	pushl_cfi_r %esi
	pushl_cfi_r %edi
	addl    $16, 12(%esp) /* Adjust the pushed ESP to mirror pushal */
.endif

.if \has_ecode != 0
	movl    %ss:-4(%esp), %edx /* ECODE */
.endif

	/* Only clobbers EAX and ECX, so EDX above is still OK. */
#if defined(CONFIG_X86_LOADSEGMENTS_ONLY_USER) && !defined(CONFIG_VM86)
	testl   $3, X86_CONTEXT_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_EFLAGS(%esp)
	jz      1f
	call    x86_load_segments
1:
#else
	call    x86_load_segments
#endif

	/* Fix the stack-pointer `c_gpregs.gp_esp'. */
#ifndef CONFIG_NO_X86_SEGMENTATION
	addl    $(X86_SEGMENTS_SIZE+X86_IRREGS_HOST32_SIZE), X86_GPREGS32_OFFSETOF_ESP(%esp)
#else /* !CONFIG_NO_X86_SEGMENTATION */
	addl    $(X86_IRREGS_HOST32_SIZE), X86_GPREGS32_OFFSETOF_ESP(%esp)
#endif /* CONFIG_NO_X86_SEGMENTATION */

	movl    %esp,     %ecx /* context */

	/* Invoke the high-level interrupt handler. */
	call    \name

	/* Reload registers. */
.if \use_pushal != 0
	popal_cfi_r
.else
	popl_cfi_r %edi
	popl_cfi_r %esi
	popl_cfi_r %ebp
	popl_cfi   %eax /* Don't pop ESP (pop EAX which will be restored for real below) */
	popl_cfi_r %ebx
	popl_cfi_r %edx
	popl_cfi_r %ecx
	popl_cfi_r %eax
.endif

#ifndef CONFIG_NO_X86_SEGMENTATION
	popl_cfi_r %gs
	popl_cfi_r %fs
	popl_cfi_r %es
	popl_cfi_r %ds
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	iret
	.cfi_endproc
SYMEND(irq_\id)
.endm

define_hardware_exception_handler 00, x86_handle_divide_by_zero
define_hardware_exception_handler 03, x86_handle_breakpoint
define_hardware_exception_handler 04, x86_handle_overflow
define_hardware_exception_handler 05, x86_handle_bound
#if 0 /* ... No situation exists where we don't have popal on i386+... */
define_hardware_exception_handler 06, x86_handle_illegal_instruction, 0, 0
#else
define_hardware_exception_handler 06, x86_handle_illegal_instruction
#endif
define_hardware_exception_handler 0d, x86_handle_gpf, 1
define_hardware_exception_handler 0e, x86_handle_pagefault, 1


.hidden irqdpl_08
.global irqdpl_08
irqdpl_08 = 0

#ifndef CONFIG_NO_FPU
INTERN_ENTRY(irq_07)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4
	pushl_cfi_r  %eax
	pushl_cfi_r  %ecx
	pushl_cfi_r  %edx

#ifndef CONFIG_NO_X86_SEGMENTATION
	pushl_cfi_r  %ds
	pushl_cfi_r  %es
	pushl_cfi_r  %fs
	pushl_cfi_r  %gs
#endif
	call   x86_load_segments

	/* Try to invoke the FPU interrupt handler. */
	call   x86_fpu_interrupt_handler

#ifndef CONFIG_NO_X86_SEGMENTATION
	popl_cfi_r   %gs
	popl_cfi_r   %fs
	popl_cfi_r   %es
	popl_cfi_r   %ds
#endif

	testl  %eax, %eax
	popl_cfi_r   %edx
	popl_cfi_r   %ecx
	popl_cfi_r   %eax
	jz     1f
	iret
	/* If that failed, handle the IRQ as an unhandled interrupt. */
1:	cli
#ifndef CONFIG_NO_X86_SEGMENTATION
	movl   $0,    %ss:-(4+X86_GPREGS_SIZE+X86_SEGMENTS_SIZE)(%esp) /* ECODE */
	movl   $0x07, %ss:-(8+X86_GPREGS_SIZE+X86_SEGMENTS_SIZE)(%esp) /* INTNO */
#else /* !CONFIG_NO_X86_SEGMENTATION */
	movl   $0,    %ss:-(4+X86_GPREGS_SIZE)(%esp) /* ECODE */
	movl   $0x07, %ss:-(8+X86_GPREGS_SIZE)(%esp) /* INTNO */
#endif /* CONFIG_NO_X86_SEGMENTATION */
	jmp    irq_common
	.cfi_endproc
SYMEND(irq_07)
#endif


.hwexcept_end = .











