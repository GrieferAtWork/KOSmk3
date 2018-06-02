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

.macro define_hardware_exception_handler id, name, do_sti=0, has_ecode=0
INTERN_ENTRY(irq_\id)
	.cfi_startproc simple
	.cfi_signal_frame
.if \has_ecode != 0
	.cfi_def_cfa %rsp,    6*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
	irq_enter_errcode
	/* Move the exception code above the CPU-state. */
	popq_cfi -(8+X86_GPREGS_SIZE)(%rsp) /* ECODE */
.else
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
.ifc \id,03
	irq_enter 0,0
.else
	irq_enter \do_sti
.endif
.endif

	/* Save general-purpose registers. */
	pushq_cfi_r %rax
	pushq_cfi_r %rcx
	pushq_cfi_r %rdx
	pushq_cfi_r %rbx
	pushq_cfi_r %rbp
	pushq_cfi_r %rsi
	pushq_cfi_r %rdi
	pushq_cfi_r %r8
	pushq_cfi_r %r9
	pushq_cfi_r %r10
	pushq_cfi_r %r11
	pushq_cfi_r %r12
	pushq_cfi_r %r13
	pushq_cfi_r %r14
	pushq_cfi_r %r15

	movq    %rsp,    %rdi /* context */
.if \has_ecode != 0
	movq    -8(%rsp), %rsi /* ECODE */
.endif

	/* Invoke the high-level interrupt handler. */
	call    \name

	/* Restore registers. */
	popq_cfi_r %r15
	popq_cfi_r %r14
	popq_cfi_r %r13
	popq_cfi_r %r12
	popq_cfi_r %r11
	popq_cfi_r %r10
	popq_cfi_r %r9
	popq_cfi_r %r8
	popq_cfi_r %rdi
	popq_cfi_r %rsi
	popq_cfi_r %rbp
	popq_cfi_r %rbx
	popq_cfi_r %rdx
	popq_cfi_r %rcx
	popq_cfi_r %rax
	irq_leave \do_sti
	.cfi_endproc
SYMEND(irq_\id)
.endm

define_hardware_exception_handler 00, x86_handle_divide_by_zero,      1, 0
define_hardware_exception_handler 03, x86_handle_breakpoint,          1, 0
define_hardware_exception_handler 04, x86_handle_overflow,            1, 0
define_hardware_exception_handler 05, x86_handle_bound,               1, 0
define_hardware_exception_handler 06, x86_handle_illegal_instruction, 1, 0
define_hardware_exception_handler 0d, x86_handle_gpf,                 1, 1
define_hardware_exception_handler 0e, x86_handle_pagefault,           0, 1


.hidden irqdpl_08
.global irqdpl_08
irqdpl_08 = 0

.hidden irqist_08
.global irqist_08
irqist_08 = 1 /* Use the secondary stack when handling #DFs */

#if 1
INTERN_ENTRY(irq_08)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    6*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
1:	cli
	hlt
	jmp 1b
	irq_enter_errcode
	/* Move the exception code above the CPU-state. */
	popq_cfi -(8+X86_GPREGS_SIZE)(%rsp) /* ECODE */

	/* Save general-purpose registers. */
	pushq_cfi_r %rax
	pushq_cfi_r %rcx
	pushq_cfi_r %rdx
	pushq_cfi_r %rbx
	pushq_cfi_r %rbp
	pushq_cfi_r %rsi
	pushq_cfi_r %rdi
	pushq_cfi_r %r8
	pushq_cfi_r %r9
	pushq_cfi_r %r10
	pushq_cfi_r %r11
	pushq_cfi_r %r12
	pushq_cfi_r %r13
	pushq_cfi_r %r14
	pushq_cfi_r %r15

	movq    %rsp,    %rdi /* context */
	movq    -8(%rsp), %rsi /* ECODE */

	/* Invoke the high-level interrupt handler. */
	call    x86_handle_df

	/* Restore registers. */
	popq_cfi_r %r15
	popq_cfi_r %r14
	popq_cfi_r %r13
	popq_cfi_r %r12
	popq_cfi_r %r11
	popq_cfi_r %r10
	popq_cfi_r %r9
	popq_cfi_r %r8
	popq_cfi_r %rdi
	popq_cfi_r %rsi
	popq_cfi_r %rbp
	popq_cfi_r %rbx
	popq_cfi_r %rdx
	popq_cfi_r %rcx
	popq_cfi_r %rax
	irq_leave
	.cfi_endproc
SYMEND(irq_08)
#else
define_hardware_exception_handler 08, x86_handle_df, 1
#endif


#ifndef CONFIG_NO_FPU
INTERN_ENTRY(irq_07)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
	irq_enter 1
	pushq_cfi_r %rax
	pushq_cfi_r %rcx
	pushq_cfi_r %rdx
	pushq_cfi_r %rsi
	pushq_cfi_r %rdi
	pushq_cfi_r %r8
	pushq_cfi_r %r9
	pushq_cfi_r %r10
	pushq_cfi_r %r11

	/* Try to invoke the FPU interrupt handler. */
	call   x86_fpu_interrupt_handler

	popq_cfi_r  %r11
	popq_cfi_r  %r10
	popq_cfi_r  %r9
	popq_cfi_r  %r8
	popq_cfi_r  %rdi
	popq_cfi_r  %rsi
	popq_cfi_r  %rdx
	popq_cfi_r  %rcx
	testq  %rax, %rax
	popq_cfi_r  %rax
	jz     1f
	irq_leave 1
	/* If that failed, handle the IRQ as an unhandled interrupt. */
1:	cli
	movq   $0,    -(8+X86_GPREGS_SIZE)(%rsp)  /* ECODE */
	movq   $0x07, -(16+X86_GPREGS_SIZE)(%rsp) /* INTNO */
	jmp    irq_common
	.cfi_endproc
SYMEND(irq_07)
#endif


.hwexcept_end = .











