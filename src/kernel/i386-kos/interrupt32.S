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


.section .text.hot

#undef CONFIG_LOAD_SEGMENTS_IFCHANGED
#define CONFIG_LOAD_SEGMENTS_IFCHANGED 1


#ifdef CONFIG_LOAD_SEGMENTS_IFCHANGED
.macro restore_segment reg, value, clobber=%ax
	movw    \reg, \clobber
	cmpw    \value, \clobber
	je      1f
	movw    \value, \clobber
	movw    \clobber, \reg
1:
.endm
#endif

/* Load kernel segment registers.
 * Usually called at the start of an interrupt.
 * CLOBBER: %ax */
INTERN_ENTRY(x86_load_segments)
	.cfi_startproc
#ifdef CONFIG_LOAD_SEGMENTS_IFCHANGED
	restore_segment %ds, $(X86_KERNEL_DS)
	restore_segment %es, $(X86_KERNEL_DS)
	/* NOTE: FS is always cleared by the CPU when switching to user-space
	 *       because it is loaded with a privilege level #0 descriptor. */
	movw    $(X86_SEG_FS), %ax
	movw    %ax, %fs
#ifndef CONFIG_X86_LOADSEGMENTS_IGNORE_USERTLS
	restore_segment %gs, $(X86_SEG_USER_GS)
#endif /* !CONFIG_X86_LOADSEGMENTS_IGNORE_USERTLS */
#else /* CONFIG_LOAD_SEGMENTS_IFCHANGED */
	movw    $(X86_KERNEL_DS), %ax
	movw    %ax, %ds
	movw    %ax, %es
	movw    %ax, %ss
	movw    $(X86_SEG_FS), %ax
	movw    %ax, %fs
#ifndef CONFIG_X86_LOADSEGMENTS_IGNORE_USERTLS
	movw    $(X86_SEG_USER_GS), %ax
	movw    %ax, %gs
#endif /* !CONFIG_X86_LOADSEGMENTS_IGNORE_USERTLS */
#endif /* !CONFIG_LOAD_SEGMENTS_IFCHANGED */
	ret
	.cfi_endproc
SYMEND(x86_load_segments)


/* Load kernel segment registers.
 * Usually called at the start of an interrupt.
 * CLOBBER: %cx */
INTERN_ENTRY(x86_load_segments_ecx)
	.cfi_startproc
#ifdef CONFIG_LOAD_SEGMENTS_IFCHANGED
	restore_segment %ds, $(X86_KERNEL_DS), %cx
	restore_segment %es, $(X86_KERNEL_DS), %cx
	/* NOTE: FS is always cleared by the CPU when switching to user-space
	 *       because it is loaded with a privilege level #0 descriptor. */
	movw    $(X86_SEG_FS), %cx
	movw    %cx, %fs
#ifndef CONFIG_X86_LOADSEGMENTS_IGNORE_USERTLS
	restore_segment %gs, $(X86_SEG_USER_GS), %cx
#endif /* !CONFIG_X86_LOADSEGMENTS_IGNORE_USERTLS */
#else /* CONFIG_LOAD_SEGMENTS_IFCHANGED */
	movw    $(X86_KERNEL_DS), %cx
	movw    %cx, %ds
	movw    %cx, %es
	movw    %cx, %ss
	movw    $(X86_SEG_FS), %cx
	movw    %cx, %fs
#ifndef CONFIG_X86_LOADSEGMENTS_IGNORE_USERTLS
	movw    $(X86_SEG_USER_GS), %cx
	movw    %cx, %gs
#endif /* !CONFIG_X86_LOADSEGMENTS_IGNORE_USERTLS */
#endif /* !CONFIG_LOAD_SEGMENTS_IFCHANGED */
	ret
	.cfi_endproc
SYMEND(x86_load_segments_ecx)


.section .text
INTERN_ENTRY(x86_check_spurious_interrupt_PIC1)
	/* Check if the interrupt has been spurious
	 * and don't invoke the interrupt if it was */
	.cfi_startproc
	pushl_cfi_r %eax
	/* Check PIC1 */
	movb   $(X86_PIC_READ_ISR), %al
	outb   %al,      $(X86_PIC1_CMD) /* outb(X86_PIC1_CMD,X86_PIC_READ_ISR); */
	inb    $(X86_PIC2_CMD),     %al
	testb  $0x80,               %al
	jz     99f /* if (!(inb(X86_PIC2_CMD) & 0x80)) goto 2f; */
	popl_cfi %eax
	ret
99:	/* Preserve scratch registers. */
	pushl_cfi_r %ecx
	pushl_cfi_r %edx
#ifndef CONFIG_NO_X86_SEGMENTATION
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs
	pushl_cfi_r %gs
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	call   x86_load_segments
	call   x86_pic2_spur
	jmp    98f
	.cfi_endproc
SYMEND(x86_check_spurious_interrupt_PIC1)

INTERN_ENTRY(x86_check_spurious_interrupt_PIC2)
	/* Check if the interrupt has been spurious
	 * and don't invoke the interrupt if it was */
	.cfi_startproc
	pushl_cfi_r %eax
	/* Check PIC2 */
	movb   $(X86_PIC_READ_ISR), %al
	outb   %al,      $(X86_PIC2_CMD) /* outb(X86_PIC2_CMD,X86_PIC_READ_ISR); */
	inb    $(X86_PIC2_CMD),     %al
	testb  $0x80,               %al
	jz     99f /* if (!(inb(X86_PIC2_CMD) & 0x80)) goto 2f; */
	popl_cfi %eax
	ret
	.cfi_adjust_cfa_offset 4
99:	/* Preserve scratch registers. */
	pushl_cfi_r %ecx
	pushl_cfi_r %edx
#ifndef CONFIG_NO_X86_SEGMENTATION
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs
	pushl_cfi_r %gs
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	call   x86_load_segments
	call   x86_pic1_spur
98:
#ifndef CONFIG_NO_X86_SEGMENTATION
	popl_cfi_r  %gs
	popl_cfi_r  %fs
	popl_cfi_r  %es
	popl_cfi_r  %ds
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	popl_cfi_r  %edx
	popl_cfi_r  %ecx
	popl_cfi_r  %eax
	addl   $4,  %esp
	.cfi_adjust_cfa_offset -4
	iret
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
irqdpl_\id = \dpl
irq_\id:
	.cfi_startproc simple
	.cfi_signal_frame
	/* Save the interrupt number and error code just above the CPU-state:
	 * struct {
	 *     u32                intno;
	 *     u32                errcode;
	 *     struct cpu_context context;
	 * };
	 */
.if \type == INTERRUPT_TYPE_ERROR
	.cfi_def_cfa %esp,    4*4
	.cfi_offset %eflags, -1*4
	/*.cfi_offset %cs,   -2*4*/
	.cfi_offset %eip,    -3*4
	/* Check for spurios interrupts */
.if \type == INTERRUPT_TYPE_SPUR1
	call    x86_check_spurious_interrupt_PIC1
.elseif \type == INTERRUPT_TYPE_SPUR2
	call    x86_check_spurious_interrupt_PIC2
.endif
#ifndef CONFIG_NO_X86_SEGMENTATION
	popl_cfi %ss:-(4+X86_GPREGS_SIZE+X86_SEGMENTS_SIZE)(%esp) /* ECODE */
#else
	popl_cfi %ss:-(4+X86_GPREGS_SIZE)(%esp) /* ECODE */
#endif
.else
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	/*.cfi_offset %cs,   -2*4*/
	.cfi_offset %eip,    -3*4
	/* Check for spurios interrupts */
.if \type == INTERRUPT_TYPE_SPUR1
	call    x86_check_spurious_interrupt_PIC1
.elseif \type == INTERRUPT_TYPE_SPUR2
	call    x86_check_spurious_interrupt_PIC2
.endif
#ifndef CONFIG_NO_X86_SEGMENTATION
	movl    $0, %ss:-(4+X86_GPREGS_SIZE+X86_SEGMENTS_SIZE)(%esp) /* ECODE */
#else
	movl    $0, %ss:-(4+X86_GPREGS_SIZE)(%esp) /* ECODE */
#endif
.endif
#ifndef CONFIG_NO_X86_SEGMENTATION
	movl    $0x\id, %ss:-(8+X86_GPREGS_SIZE+X86_SEGMENTS_SIZE)(%esp) /* INTNO */
#else
	movl    $0x\id, %ss:-(8+X86_GPREGS_SIZE)(%esp) /* INTNO */
#endif
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
//define_interrupt 08, 0, INTERRUPT_TYPE_ERROR /* #DF  Double Fault. */
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
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	/*.cfi_offset %cs,   -2*4*/
	.cfi_offset %eip,    -3*4

#ifndef CONFIG_NO_X86_SEGMENTATION
	/* Save segment registers. */
	pushl_cfi_r  %ds
	pushl_cfi_r  %es
	pushl_cfi_r  %fs
	pushl_cfi_r  %gs
#endif /* !CONFIG_NO_X86_SEGMENTATION */

	/* Save general-purpose registers. */
	pushal_cfi_r

	/* Adjust the stack to cover INTNO + ECODE */
	subl    $8, %esp
	.cfi_adjust_cfa_offset 8

#if defined(CONFIG_X86_LOADSEGMENTS_ONLY_USER) && !defined(CONFIG_VM86)
	/* Only load segment registers if the interrupt came from user-space. */
	testl   $3, 8+X86_CONTEXT_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_EFLAGS(%esp)
	jz      1f
	call    x86_load_segments
1:
#else
	call    x86_load_segments
#endif

	/* Push a pointer to the CPU context. */
	leal    8(%esp), %ecx
	pushl_cfi %ecx

	/* Fix the stack-pointer `c_gpregs.gp_esp' to
	 * point directly past the host's IRET tail.
	 * Technicaly we'd only need to do this if
	 * the interrupt originated from kernel-space,
	 * but it doesn't hurt to just always do it.
	 * NOTE: According to PUSHAL, the currently stored
	 *       ESP points at the end of GPREGS, aka. at
	 *       the start of the segment state. */
#ifndef CONFIG_NO_X86_SEGMENTATION
	addl    $(X86_SEGMENTS_SIZE+X86_IRREGS_HOST32_SIZE), X86_GPREGS_OFFSETOF_ESP(%ecx)
#else /* !CONFIG_NO_X86_SEGMENTATION */
	addl    $(X86_IRREGS_HOST32_SIZE), X86_GPREGS_OFFSETOF_ESP(%ecx)
#endif /* CONFIG_NO_X86_SEGMENTATION */

	/* With everything now set up, switch to the interrupt handler. */
	call    x86_interrupt_handler
	/* Adjust the stack to pop $context, INTNO & ECODE
	 * NOTE: Because `x86_interrupt_handler' is STDCALL, ESP is already updated. */
	.cfi_adjust_cfa_offset -12

	/* Load general-purpose registers. */
	popal_cfi_r
#ifndef CONFIG_NO_X86_SEGMENTATION
	/* Load segment registers. */
	popl_cfi_r   %gs
	popl_cfi_r   %fs
	popl_cfi_r   %es
	popl_cfi_r   %ds
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	iret
	.cfi_endproc
.irq_common_end = .
X86_DEFINE_INTERRUPT_GUARD(irq_common,.irq_common_end)
SYMEND(irq_common)





.section .rodata.interrupt_except
INTERN_ENTRY(x86_interrupt_mode_select)
	/* X86_INTERRUPT_GUARD_FINTERRUPT >> 8 */ .word  TASK_USERCTX_TYPE_INTR_INTERRUPT
#ifndef CONFIG_NO_X86_SYSENTER
	/* X86_INTERRUPT_GUARD_FSYSENTER  >> 8 */ .word  TASK_USERCTX_TYPE_INTR_SYSCALL|X86_SYSCALL_TYPE_FSYSENTER
#endif /* !CONFIG_NO_X86_SYSENTER */
	/* X86_INTERRUPT_GUARD_FSYSCALL   >> 8 */ .word  TASK_USERCTX_TYPE_INTR_SYSCALL|X86_SYSCALL_TYPE_FINT80
SYMEND(x86_interrupt_mode_select)


.section .text.interrupt_except
INTERN_ENTRY(x86_interrupt_except)
	.cfi_startproc simple
	.cfi_signal_frame
	/* At this point, the stack-pointer is pointed at the start
	 * the IRET tail that brought us into the interrupt handler
	 * that eventually threw an error.
	 * The caller will have disabled interrupts for us, because
	 * of the `EXCEPTION_HANDLER_FNOINTERRUPT' flag, so we can
	 * be sure that the stack-memory still contains the required
	 * block of data describing the interrupt return info. */
	.cfi_def_cfa %esp,     X86_IRREGS_HOST32_SIZE
	.cfi_offset %eflags, (-X86_IRREGS_HOST32_SIZE) + X86_IRREGS_HOST32_OFFSETOF_EFLAGS
	.cfi_offset %eip,    (-X86_IRREGS_HOST32_SIZE) + X86_IRREGS_HOST32_OFFSETOF_EIP
#ifdef CONFIG_VM86
	testl   $(EFLAGS_VM), %ss:X86_IRREGS_HOST32_OFFSETOF_EFLAGS(%esp)
	jnz     .interrupt_except_user
#endif

	/* First off: Check where the interrupt wants to go. */
	testl   $3, %ss:X86_IRREGS_HOST32_OFFSETOF_CS(%esp)
	jnz     .interrupt_except_user

	/* Deal with the special case of exception propagation after user-space got redirected. */
	cmpl    $(x86_redirect_preemption), %ss:X86_IRREGS_HOST32_OFFSETOF_EIP(%esp)
	je      .interrupt_except_redirect

.interrupt_except_host:
	/* Interrupt originates from kernel-space (continue unwinding). */

	/* We must rewind the stack to reclaim all the stack-space
	 * that was deallocated when exception handling unwound the
	 * stack to account for our `EXCEPTION_DESCRIPTOR_FDEALLOC_CONTINUE' flag.
	 * Luckily, the original stack-value (where the exception happend)
	 * is still saved in the thread-local exception-context block.
	 * However, we also need to keep the current stack-pointer around,
	 * as it poinst to the interrupt return tail which we must still
	 * reproduce in order to continue handling errors. (it contains
	 * information about where we're supposed to return to)
	 * With that in mind, temporarily exchange the 2 pointers, as we
	 * know that our stack-pointer is thread-local and therefor inaccessible
	 * by other threads, but obviously restore the proper pointer further below.
	 */
	xchgl   %taskseg:TASK_SEGMENT_OFFSETOF_XCURRENT+ \
	                 EXCEPTION_INFO_OFFSETOF_CONTEXT+ \
	                 X86_CONTEXT32_OFFSETOF_ESP, %esp
	.cfi_undefined %eflags
	.cfi_undefined %eip
	.cfi_def_cfa_offset 0

	/* Allocate space for a copy of the IRET tail. */
	subl    $(X86_IRREGS_HOST32_SIZE), %esp
	.cfi_def_cfa_offset X86_IRREGS_HOST32_SIZE

#ifndef CONFIG_NO_X86_SEGMENTATION
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs
	pushl_cfi_r %gs
#endif /* !CONFIG_NO_X86_SEGMENTATION */

	/* Save general-purpose registers. */
	pushal_cfi_r

	call    x86_load_segments

	/* We already know that the interrupt is originating from kernel-space,
	 * so we also know that we must fix the ESP of the cpu-context. */
#ifndef CONFIG_NO_X86_SEGMENTATION
	addl    $(X86_SEGMENTS_SIZE+X86_IRREGS_HOST32_SIZE), X86_GPREGS_OFFSETOF_ESP(%esp)
#else
	addl    $(X86_IRREGS_HOST32_SIZE), X86_GPREGS_OFFSETOF_ESP(%esp)
#endif

	/* Restore the original stack pointer now that we can use all the GP
	 * registers after already having saved them to the CPU-state. */
	leal    X86_CONTEXT_SIZE(%esp), %esi
	xchgl   %taskseg:TASK_SEGMENT_OFFSETOF_XCURRENT+ \
	                 EXCEPTION_INFO_OFFSETOF_CONTEXT+ \
	                 X86_CONTEXT32_OFFSETOF_ESP, %esi

	/* Copy IRET tail information from the original stack-pointer (now located in ESI) */
	leal    X86_CONTEXT32_OFFSETOF_IRET(%esp), %edi
	movl    $(X86_IRREGS_HOST32_SIZE/4), %ecx
	movsl   /* EIP */
	.cfi_def_cfa_offset X86_CONTEXT32_SIZE
	.cfi_offset %eip, (-X86_CONTEXT32_SIZE)+(X86_CONTEXT32_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_EIP)
	movsl   /* CS */
	movsl   /* EFLAGS */
	.cfi_offset %eflags, (-X86_CONTEXT32_SIZE)+(X86_CONTEXT32_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_EFLAGS)

	/* Re-enable interrupts if they are where the interrupt would have lead. */
	testl   $(EFLAGS_IF), X86_CONTEXT_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_EFLAGS(%esp)
	jz      1f
	sti
1:

	movl    %esp, %ecx
	call    __error_rethrow_at
	.cfi_endproc

	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %esp,     X86_IRREGS_HOST32_SIZE
	.cfi_offset %eflags, (-X86_IRREGS_HOST32_SIZE) + X86_IRREGS_HOST32_OFFSETOF_EFLAGS
	.cfi_offset %eip,    (-X86_IRREGS_HOST32_SIZE) + X86_IRREGS_HOST32_OFFSETOF_EIP
.interrupt_except_redirect:
	/* We know the following:
	 *   - `X86_IRREGS_HOST32_OFFSETOF_EIP(%esp) == x86_redirect_preemption'
	 *   - `X86_IRREGS_HOST32_OFFSETOF_CS(%esp) == X86_KERNEL_CS'
	 *   - `%ss == X86_KERNEL_DS'
	 * Now we must copy `iret_saved' into the on-stack IRET tail.
	 * NOTE: `task_propagate_user_exception()' below will serve RPC functions
	 *        that were requested for execution by the redirection.
	 */
#ifndef CONFIG_NO_X86_SEGMENTATION
	movw    %taskseg, %ss:(+0 + (-CONFIG_KERNELSTACK_SIZE) + X86_IRREGS_USER32_SIZE)(%esp)
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	movl    %eax, %ss:0(%esp) /* Preserve EAX */
	movw    $(X86_HOST_TLS), %ax
	movw    %ax, %taskseg
	/* Now copy the saved IRET tail into the current one. */
	movl    %taskseg:iret_saved + 4, %eax
	movl    %eax, %ss:4(%esp)
	movl    %taskseg:iret_saved + 8, %eax
	movl    %eax, %ss:8(%esp)
	movl    %taskseg:iret_saved + 16, %eax
	movl    %eax, %ss:16(%esp)
	movl    %taskseg:iret_saved + 20, %eax
	movl    %eax, %ss:20(%esp)
	movl    %taskseg:iret_saved + 0, %eax
	xchgl   %ss:0(%esp), %eax /* Restore EAX */
#ifndef CONFIG_NO_X86_SEGMENTATION
	/* Restore the user-space version of the %fs register. */
	movw    %ss:(+0 + (-CONFIG_KERNELSTACK_SIZE) + X86_IRREGS_USER32_SIZE)(%esp), %taskseg
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	/* Fallthrough to propagate the exception to user-space. */

.interrupt_except_user:

	/* Interrupt originates from user-space.
	 * In this case, we don't want to preserve stack information
	 * about where exactly in kernel-space the interrupt happened. */
#ifndef NDEBUG
	/* So-as not to break tracebacks, we also want to preserve
	 * the kernel-stack at the time when the exception happened.
	 * Because of that, we need to carefully redirect execution
	 * to the upper half of the kernel stack, above the part
	 * that was used by the error. */
#ifndef CONFIG_NO_X86_SEGMENTATION
	movw    %taskseg, %ss:(+0 + (-CONFIG_KERNELSTACK_SIZE) + X86_IRREGS_USER32_SIZE)(%esp)
	movw    %ax,      %ss:(+2 + (-CONFIG_KERNELSTACK_SIZE) + X86_IRREGS_USER32_SIZE)(%esp)
	movw    $(X86_HOST_TLS), %ax
	movw    %ax, %taskseg
	movw    %ss:(+2 + (-CONFIG_KERNELSTACK_SIZE) + X86_IRREGS_USER32_SIZE)(%esp), %ax
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	/* Load the stack address below the exception location. */
	movl    %taskseg:TASK_OFFSETOF_SEGMENT+ \
	                 TASK_SEGMENT_OFFSETOF_XCURRENT+ \
	                 EXCEPTION_INFO_OFFSETOF_CONTEXT+ \
	                 X86_CONTEXT32_OFFSETOF_ESP, %esp
	subl    $(X86_IRREGS_USER32_SIZE), %esp /* Reserve memory for a copy of the IRET tail. */
#ifndef CONFIG_NO_X86_SEGMENTATION
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs /* Will get overwritten below. */
	pushl_cfi_r %gs
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	pushal_cfi_r /* Save general-purpose registers. */
	call    x86_load_segments

	movl    %taskseg:TASK_OFFSETOF_STACKEND, %edx

#ifndef CONFIG_NO_X86_SEGMENTATION
	/* Fix the user-space %fs register, which we temporaryly
	 * saved at the other end of the kernel stack above. */
	movzwl  (+0 + (-CONFIG_KERNELSTACK_SIZE))(%edx), %eax
	movl    %eax, X86_HOSTCONTEXT_USER32_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS(%esp)
#endif

	/* Copy the proper IRET Tail information. */
	movl    ((-X86_IRREGS_USER32_SIZE)+X86_IRREGS_USER32_OFFSETOF_SS)(%edx), %eax
	movl    %eax, X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_SS(%esp)
	movl    ((-X86_IRREGS_USER32_SIZE)+X86_IRREGS_USER32_OFFSETOF_USERESP)(%edx), %eax
	movl    %eax, X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_USERESP(%esp)
	movl    ((-X86_IRREGS_USER32_SIZE)+X86_IRREGS_USER32_OFFSETOF_EFLAGS)(%edx), %eax
	movl    %eax, X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_EFLAGS(%esp)
	movl    ((-X86_IRREGS_USER32_SIZE)+X86_IRREGS_USER32_OFFSETOF_CS)(%edx), %eax
	movl    %eax, X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_CS(%esp)
	movl    ((-X86_IRREGS_USER32_SIZE)+X86_IRREGS_USER32_OFFSETOF_EIP)(%edx), %eax
	movl    %eax, X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_EIP(%esp)
#else /* !NDEBUG */
#ifndef CONFIG_NO_X86_SEGMENTATION
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs
	pushl_cfi_r %gs
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	/* Save general-purpose registers. */
	pushal_cfi_r
	call    x86_load_segments
#endif /* NDEBUG */

	sti                 /* Re-enable interrupts. */
	movl    %esp, %ecx  /* context */
	xorl    %edx, %edx
	xchgb   %dl, %taskseg:TASK_OFFSETOF_SEGMENT+ \
	                      TASK_SEGMENT_OFFSETOF_XCURRENT+ \
	                      EXCEPTION_INFO_OFFSETOF_FLAG+1
	movzwl  x86_interrupt_mode_select(,%edx,2), %edx /* mode */
#if 1
	/* Check if a system call should be restarted after an E_INTERRUPT. */
	testw   $(TASK_USERCTX_TYPE_INTR_SYSCALL), %dx
	jz      9f
	cmpw    $(E_INTERRUPT), %taskseg:TASK_OFFSETOF_SEGMENT+ \
	                                 TASK_SEGMENT_OFFSETOF_XCURRENT+ \
	                                 EXCEPTION_INFO_OFFSETOF_CODE
	jne     9f
	pushl_cfi %edx /* saved */
	pushl_cfi X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX(%ecx) /* sysno */
	call    task_tryrestart_syscall
	.cfi_adjust_cfa_offset -4
	popl_cfi %edx
	testb    %al, %al
	jz      .exit_after_propagate
	/* Restart the system call. */

#ifndef NDEBUG
	/* Must deallocate exception continue information before restarting the system call.
	 * In debug-mode, we usually keep this information around to ensure that tracebacks
	 * remain valid for as long as possible, however when restarting the system call,
	 * we must make sure that the kernel stack is reset to its original state, while
	 * also being careful to deal with redirected preemption. */
	movl    %taskseg:TASK_OFFSETOF_STACKEND, %edi
	subl    $(X86_HOSTCONTEXT_USER32_SIZE), %edi
	cli     /* Disable interrupts to ensure a consistent redirection state */
	cmpl    $x86_redirect_preemption, \
	        X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+ \
	        X86_IRREGS_USER32_OFFSETOF_EIP(%edi) /* Check if user-space got redirected */
	jne     1f
	/* Copy the IRET tail of the context into iret_saved */
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_EIP(%esp), %eax
	movl    %eax, %taskseg:iret_saved+X86_IRREGS_USER32_OFFSETOF_EIP
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_CS(%esp), %eax
	movl    %eax, %taskseg:iret_saved+X86_IRREGS_USER32_OFFSETOF_CS
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_EFLAGS(%esp), %eax
	movl    %eax, %taskseg:iret_saved+X86_IRREGS_USER32_OFFSETOF_EFLAGS
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_USERESP(%esp), %eax
	movl    %eax, %taskseg:iret_saved+X86_IRREGS_USER32_OFFSETOF_USERESP
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_SS(%esp), %eax
	movl    %eax, %taskseg:iret_saved+X86_IRREGS_USER32_OFFSETOF_SS
	/* Copy the IRET tail that was redirected into the context */
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_EIP(%edi), %eax
	movl    %eax, X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_EIP(%esp)
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_CS(%edi), %eax
	movl    %eax, X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_CS(%esp)
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_EFLAGS(%edi), %eax
	movl    %eax, X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_EFLAGS(%esp)
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_USERESP(%edi), %eax
	movl    %eax, X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_USERESP(%esp)
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_SS(%edi), %eax
	movl    %eax, X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_SS(%esp)
1:
	/* Copy general-purpose and segment registers. */
	movl    %esp, %esi
	movl    $((X86_HOSTCONTEXT_USER32_SIZE - X86_IRREGS_USER32_SIZE) / 4), %ecx
	rep;    movsl
	/* Load the CPU context now generated at the base of the stack into ESP */
	leal    -(X86_HOSTCONTEXT_USER32_SIZE - X86_IRREGS_USER32_SIZE)(%edi), %esp
	sti     /* Re-enable interrupts */
#endif

#ifndef CONFIG_NO_X86_SYSENTER
	testw   $(X86_SYSCALL_TYPE_FSYSENTER), %dx
	jz      1f
	.cfi_remember_state
	popal_cfi_r
	DEFINE_INTERN(sysenter_after_segments)
	jmp     sysenter_after_segments
	.cfi_restore_state
1:
#endif /* !CONFIG_NO_X86_SYSENTER */
	.cfi_remember_state
	popal_cfi_r
	DEFINE_INTERN(irq_80_after_segments)
	jmp     irq_80_after_segments
	.cfi_restore_state

8:	movl    %esp, %ecx /* context */
9:
#endif

	/* Propgate the exception to user-space. */
	call    task_propagate_user_exception

.exit_after_propagate:
#ifndef NDEBUG
	movl    %taskseg:TASK_OFFSETOF_STACKEND, %edx
	cli     /* Disable interrupts to ensure a consistent redirection state */
	cmpl    $x86_redirect_preemption, ((-X86_IRREGS_USER32_SIZE)+X86_IRREGS_USER32_OFFSETOF_EIP)(%edx) /* Check if user-space got redirected */
	jne     1f
	.cfi_remember_state

	/* Copy the updated IRET tail of the context into iret_saved */
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_EIP(%esp), %eax
	movl    %eax, %taskseg:iret_saved+X86_IRREGS_USER32_OFFSETOF_EIP
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_CS(%esp), %eax
	movl    %eax, %taskseg:iret_saved+X86_IRREGS_USER32_OFFSETOF_CS
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_EFLAGS(%esp), %eax
	movl    %eax, %taskseg:iret_saved+X86_IRREGS_USER32_OFFSETOF_EFLAGS
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_USERESP(%esp), %eax
	movl    %eax, %taskseg:iret_saved+X86_IRREGS_USER32_OFFSETOF_USERESP
	movl    X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_SS(%esp), %eax
	movl    %eax, %taskseg:iret_saved+X86_IRREGS_USER32_OFFSETOF_SS

	/* Must adjust the IRET tail at the base of our stack so we can use it
	 * to iret to where we need to go, with the register state we need to
	 * have once we get there.
	 * HINT: We know that the EIP field is $x86_redirect_preemption */
#ifndef CONFIG_NO_X86_SEGMENTATION
	movl    X86_CONTEXT32_OFFSETOF_SEGMENTS+X86_SEGMENTS32_OFFSETOF_FS(%esp), %eax
	movl    %eax, ((-X86_IRREGS_USER32_SIZE)+X86_IRREGS_USER32_OFFSETOF_EIP)(%edx)
#endif
	popal_cfi_r
#ifndef CONFIG_NO_X86_SEGMENTATION
	popl_cfi_r  %gs /* Load segment registers. */
	addl    $4, %esp
	.cfi_adjust_cfa_offset -4
	.cfi_same_value %fs
	popl_cfi_r  %es
	popl_cfi_r  %ds
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	/* Load the stack base. */
	movl    %taskseg:TASK_OFFSETOF_STACKEND, %esp
	subl    $(X86_IRREGS_USER32_SIZE), %esp

#ifndef CONFIG_NO_X86_SEGMENTATION
	/* Load the user-space FS register and setup the IRET target. */
#if X86_IRREGS_USER32_OFFSETOF_EIP == 0
	popl    %fs
	pushl   $x86_redirect_preemption
#else
	xchgl   %eax, %ss:X86_IRREGS_USER32_OFFSETOF_EIP(%esp)
	movw    %ax, %fs
	movl    $x86_redirect_preemption, %eax
	xchgl   %eax, %ss:X86_IRREGS_USER32_OFFSETOF_EIP(%esp)
#endif
#endif

	/* Do the iret to get to where we need to! */
	iret

	.cfi_restore_state
1:	popal_cfi_r /* Load general-purpose registers. */
#ifndef CONFIG_NO_X86_SEGMENTATION
	popl_cfi_r  %gs /* Load segment registers. */
	popl_cfi_r  %fs
	popl_cfi_r  %es
	popl_cfi_r  %ds
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	iret
#else /* !NDEBUG */
	/* Load general-purpose registers. */
	popal_cfi_r
#ifndef CONFIG_NO_X86_SEGMENTATION
	/* Load segment registers. */
	popl_cfi_r  %gs
	popl_cfi_r  %fs
	popl_cfi_r  %es
	popl_cfi_r  %ds
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	iret
#endif /* NDEBUG */
	.cfi_endproc
SYMEND(x86_interrupt_except)

