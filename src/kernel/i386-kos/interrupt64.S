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
#include <asm/universal.h>
#include <asm/cpu-flags.h>
#include <i386-kos/pic.h>
#include <i386-kos/interrupt.h>
#include <i386-kos/gdt.h>
#include <i386-kos/memory.h>
#include <i386-kos/syscall.h>
#include <i386-kos/vm86.h>
#include <kos/thread.h>
#include <kos/context.h>
#include <sched/task.h>

#define INTERRUPT_TYPE_NORMAL 0 /* Normal interrupt */
#define INTERRUPT_TYPE_ERROR  1 /* This interrupt pushes an error code onto the stack. */
#define INTERRUPT_TYPE_SPUR1  2 /* This interrupt may happen spuriously (PIC#1). */
#define INTERRUPT_TYPE_SPUR2  3 /* This interrupt may happen spuriously (PIC#2). */


.section .text
INTERN_ENTRY(x86_check_spurious_interrupt_PIC1)
	/* Check if the interrupt has been spurious
	 * and don't invoke the interrupt if it was */
	.cfi_startproc
	pushq_cfi_r %rax
	/* Check PIC1 */
	movb   $(X86_PIC_READ_ISR), %al
	outb   %al,      $(X86_PIC1_CMD) /* outb(X86_PIC1_CMD,X86_PIC_READ_ISR); */
	inb    $(X86_PIC2_CMD),     %al
	testb  $0x80,               %al
	jz     99f /* if (!(inb(X86_PIC2_CMD) & 0x80)) goto 2f; */
	.cfi_remember_state
	popq_cfi_r %rax
	ret
	.cfi_restore_state
99:	/* Preserve scratch registers. */
	pushq_cfi_r %rcx
	pushq_cfi_r %rdx
	pushq_cfi_r %rdx
	pushq_cfi_r %rsi
	pushq_cfi_r %rdi
	pushq_cfi_r %r8
	pushq_cfi_r %r9
	pushq_cfi_r %r10
	pushq_cfi_r %r11
	pushq_cfi   $98f
	jmp    x86_pic2_spur
	.cfi_endproc
SYMEND(x86_check_spurious_interrupt_PIC1)

INTERN_ENTRY(x86_check_spurious_interrupt_PIC2)
	/* Check if the interrupt has been spurious
	 * and don't invoke the interrupt if it was */
	.cfi_startproc
	pushq_cfi_r %rax
	/* Check PIC2 */
	movb   $(X86_PIC_READ_ISR), %al
	outb   %al,      $(X86_PIC2_CMD) /* outb(X86_PIC2_CMD,X86_PIC_READ_ISR); */
	inb    $(X86_PIC2_CMD),     %al
	testb  $0x80,               %al
	jz     99f /* if (!(inb(X86_PIC2_CMD) & 0x80)) goto 2f; */
	.cfi_remember_state
	popq_cfi_r %rax
	ret
	.cfi_restore_state
99:	/* Preserve scratch registers. */
	pushq_cfi_r %rcx
	pushq_cfi_r %rdx
	pushq_cfi_r %rdx
	pushq_cfi_r %rsi
	pushq_cfi_r %rdi
	pushq_cfi_r %r8
	pushq_cfi_r %r9
	pushq_cfi_r %r10
	pushq_cfi_r %r11
	call   x86_pic1_spur
98:	popq_cfi_r  %r11
	popq_cfi_r  %r10
	popq_cfi_r  %r9
	popq_cfi_r  %r8
	popq_cfi_r  %rdi
	popq_cfi_r  %rsi
	popq_cfi_r  %rdx
	popq_cfi_r  %rdx
	popq_cfi_r  %rcx
	popq_cfi_r  %rax
	popq_void_cfi /* Return address. */
	irq_leave
	.cfi_endproc
SYMEND(x86_check_spurious_interrupt_PIC2)


.macro define_interrupt id, dpl=0, type=INTERRUPT_TYPE_NORMAL
	.weak   irq_\id
	.hidden irq_\id
	.global irq_\id
	.type   irq_\id, @function
	.weak   irqdpl_\id
	.hidden irqdpl_\id
	.global irqdpl_\id
	.weak   irqist_\id
	.hidden irqist_\id
	.global irqist_\id
irqdpl_\id = \dpl
irqist_\id = 0
irq_\id:
	.cfi_startproc simple
	.cfi_signal_frame
	/* Save the interrupt number and error code just above the CPU-state:
	 * struct {
	 *     u64                intno;
	 *     u64                errcode;
	 *     struct cpu_context context;
	 * };
	 */
.if \type == INTERRUPT_TYPE_ERROR
	.cfi_def_cfa %rsp,    6*8
	/*.cfi_offset %ss,   -1*8*/
	/*.cfi_offset %rsp,  -2*8*/
	.cfi_offset %rflags, -3*8
	/*.cfi_offset %cs,   -4*8*/
	.cfi_offset %rip,    -5*8
	irq_enter_errcode
	/* Check for spurios interrupts */
.if \type == INTERRUPT_TYPE_SPUR1
	call    x86_check_spurious_interrupt_PIC1
.elseif \type == INTERRUPT_TYPE_SPUR2
	call    x86_check_spurious_interrupt_PIC2
.endif
	popq_cfi -(8+X86_GPREGS_SIZE)(%rsp) /* ECODE */
.else
	.cfi_def_cfa %rsp,    5*8
	/*.cfi_offset %ss,   -1*8*/
	/*.cfi_offset %rsp,  -2*8*/
	.cfi_offset %rflags, -3*8
	/*.cfi_offset %cs,   -4*8*/
	.cfi_offset %rip,    -5*8
	irq_enter
	/* Check for spurios interrupts */
.if \type == INTERRUPT_TYPE_SPUR1
	call    x86_check_spurious_interrupt_PIC1
.elseif \type == INTERRUPT_TYPE_SPUR2
	call    x86_check_spurious_interrupt_PIC2
.endif
	movq    $0, -(8+X86_GPREGS_SIZE)(%rsp) /* ECODE */
.endif
	movq    $0x\id, -(16+X86_GPREGS_SIZE)(%rsp) /* INTNO */
	jmp     irq_common
	.cfi_endproc
.size irq_\id, . - irq_\id
.endm


.section .text.interrupt_entry
.irq_begin:
define_interrupt 00                          /* #DE  Divide-by-zero. */
define_interrupt 01                          /* #DB  Debug. */
define_interrupt 02                          /* #NMI Non-maskable Interrupt. */
define_interrupt 03, 3                       /* #BP  Breakpoint. */
define_interrupt 04, 3                       /* #OF  Overflow. */
define_interrupt 05, 3                       /* #BR  Bound Range Exceeded. */
define_interrupt 06                          /* #UD  Invalid Opcode. */
define_interrupt 07                          /* #NM  Device Not Available. */
#if 1 /* I'm not seeing any exception code here... */
define_interrupt 08, 0                       /* #DF  Double Fault. */
#else
define_interrupt 08, 0, INTERRUPT_TYPE_ERROR /* #DF  Double Fault. */
#endif
define_interrupt 09
define_interrupt 0a, 0, INTERRUPT_TYPE_ERROR /* #TS  Invalid TSS. */
define_interrupt 0b, 0, INTERRUPT_TYPE_ERROR /* #NP  Segment Not Present. */
define_interrupt 0c, 0, INTERRUPT_TYPE_ERROR /* #SS  Stack-Segment Fault. */
define_interrupt 0d, 0, INTERRUPT_TYPE_ERROR /* #GP  General Protection Fault. */
define_interrupt 0e, 0, INTERRUPT_TYPE_ERROR /* #PF  Page Fault. */
define_interrupt 0f
define_interrupt 10                          /* #MF  x87 Floating-Point Exception. */
define_interrupt 11, 0, INTERRUPT_TYPE_ERROR /* #AC  Alignment Check. */
define_interrupt 12                          /* #MC  Machine Check. */
define_interrupt 13                          /* #XM  SIMD Floating-Point Exception. */
define_interrupt 14                          /* #VE  Virtualization Exception. */
define_interrupt 15
define_interrupt 16
define_interrupt 17
define_interrupt 18
define_interrupt 19
define_interrupt 1a
define_interrupt 1b
define_interrupt 1c
define_interrupt 1d
define_interrupt 1e, 0, INTERRUPT_TYPE_ERROR /* #SX  Security Exception. */
define_interrupt 1f
define_interrupt 20
define_interrupt 21
define_interrupt 22
define_interrupt 23
define_interrupt 24
define_interrupt 25
define_interrupt 26
define_interrupt 27
define_interrupt 28
define_interrupt 29
define_interrupt 2a
define_interrupt 2b
define_interrupt 2c
define_interrupt 2d
define_interrupt 2e
define_interrupt 2f
define_interrupt 30
define_interrupt 31
define_interrupt 32
define_interrupt 33
define_interrupt 34
define_interrupt 35
define_interrupt 36
define_interrupt 37
define_interrupt 38
define_interrupt 39
define_interrupt 3a
define_interrupt 3b
define_interrupt 3c
define_interrupt 3d
define_interrupt 3e
define_interrupt 3f
define_interrupt 40
define_interrupt 41
define_interrupt 42
define_interrupt 43
define_interrupt 44
define_interrupt 45
define_interrupt 46
define_interrupt 47
define_interrupt 48
define_interrupt 49
define_interrupt 4a
define_interrupt 4b
define_interrupt 4c
define_interrupt 4d
define_interrupt 4e
define_interrupt 4f
define_interrupt 50
define_interrupt 51
define_interrupt 52
define_interrupt 53
define_interrupt 54
define_interrupt 55
define_interrupt 56
define_interrupt 57
define_interrupt 58
define_interrupt 59
define_interrupt 5a
define_interrupt 5b
define_interrupt 5c
define_interrupt 5d
define_interrupt 5e
define_interrupt 5f
define_interrupt 60
define_interrupt 61
define_interrupt 62
define_interrupt 63
define_interrupt 64
define_interrupt 65
define_interrupt 66
define_interrupt 67
define_interrupt 68
define_interrupt 69
define_interrupt 6a
define_interrupt 6b
define_interrupt 6c
define_interrupt 6d
define_interrupt 6e
define_interrupt 6f
define_interrupt 70
define_interrupt 71
define_interrupt 72
define_interrupt 73
define_interrupt 74
define_interrupt 75
define_interrupt 76
define_interrupt 77
define_interrupt 78
define_interrupt 79
define_interrupt 7a
define_interrupt 7b
define_interrupt 7c
define_interrupt 7d
define_interrupt 7e
define_interrupt 7f
define_interrupt 80, 3 /* Syscall */
define_interrupt 81
define_interrupt 82
define_interrupt 83
define_interrupt 84
define_interrupt 85
define_interrupt 86
define_interrupt 87
define_interrupt 88
define_interrupt 89
define_interrupt 8a
define_interrupt 8b
define_interrupt 8c
define_interrupt 8d
define_interrupt 8e
define_interrupt 8f
define_interrupt 90
define_interrupt 91
define_interrupt 92
define_interrupt 93
define_interrupt 94
define_interrupt 95
define_interrupt 96
define_interrupt 97
define_interrupt 98
define_interrupt 99
define_interrupt 9a
define_interrupt 9b
define_interrupt 9c
define_interrupt 9d
define_interrupt 9e
define_interrupt 9f
define_interrupt a0
define_interrupt a1
define_interrupt a2
define_interrupt a3
define_interrupt a4
define_interrupt a5
define_interrupt a6
define_interrupt a7
define_interrupt a8
define_interrupt a9
define_interrupt aa
define_interrupt ab
define_interrupt ac
define_interrupt ad
define_interrupt ae
define_interrupt af
define_interrupt b0
define_interrupt b1
define_interrupt b2
define_interrupt b3
define_interrupt b4
define_interrupt b5
define_interrupt b6
define_interrupt b7
define_interrupt b8
define_interrupt b9
define_interrupt ba
define_interrupt bb
define_interrupt bc
define_interrupt bd
define_interrupt be
define_interrupt bf
define_interrupt c0
define_interrupt c1
define_interrupt c2
define_interrupt c3
define_interrupt c4
define_interrupt c5
define_interrupt c6
define_interrupt c7
define_interrupt c8
define_interrupt c9
define_interrupt ca
define_interrupt cb
define_interrupt cc
define_interrupt cd
define_interrupt ce
define_interrupt cf
define_interrupt d0
define_interrupt d1
define_interrupt d2
define_interrupt d3
define_interrupt d4
define_interrupt d5
define_interrupt d6
define_interrupt d7
define_interrupt d8
define_interrupt d9
define_interrupt da
define_interrupt db
define_interrupt dc
define_interrupt dd
define_interrupt de
define_interrupt df
define_interrupt e0
define_interrupt e1
define_interrupt e2
define_interrupt e3
define_interrupt e4
define_interrupt e5
define_interrupt e6
define_interrupt e7
define_interrupt e8
define_interrupt e9
define_interrupt ea
define_interrupt eb
define_interrupt ec
define_interrupt ed
define_interrupt ee
define_interrupt ef
define_interrupt f0
define_interrupt f1
define_interrupt f2
define_interrupt f3
define_interrupt f4
define_interrupt f5
define_interrupt f6
define_interrupt f7, 0, INTERRUPT_TYPE_SPUR1
define_interrupt f8
define_interrupt f9
define_interrupt fa
define_interrupt fb
define_interrupt fc
define_interrupt fd
define_interrupt fe
define_interrupt ff, 0, INTERRUPT_TYPE_SPUR2
.irq_end = .
X86_DEFINE_INTERRUPT_GUARD(.irq_begin,.irq_end)


.section .text.interrupt_common
INTERN_ENTRY(irq_common)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	/*.cfi_offset %ss,   -1*8*/
	/*.cfi_offset %rsp,  -2*8*/
	.cfi_offset %rflags, -3*8
	/*.cfi_offset %cs,   -4*8*/
	.cfi_offset %rip,    -5*8

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

	subq    $16, %rsp
	.cfi_adjust_cfa_offset 16

	/* Load the ERRCODE and INTNO values into registers. */
	popq_cfi %rsi      /* register_t intno */
	popq_cfi %rdx      /* register_t errcode */
	movq    %rsp, %rdi /* struct cpu_anycontext *__restrict context */

	/* With everything now set up, switch to the interrupt handler. */
	call    x86_interrupt_handler

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
.irq_common_end = .
X86_DEFINE_INTERRUPT_GUARD(irq_common,.irq_common_end)
SYMEND(irq_common)



.section .text.interrupt_except
INTERN_ENTRY(x86_interrupt_except)
	/* TODO */
	int3
1:	hlt
	jmp 1b
SYMEND(x86_interrupt_except)





