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
#include <kos/types.h>
#include <hybrid/asm.h>
#include <kernel/interrupt.h>
#include <sched/task.h>
#include <i386-kos/interrupt.h>
#include <i386-kos/syscall.h>
#include <i386-kos/gdt.h>
#include <errno.h>
#include <except.h>
#include <asm/cfi.h>
#include <asm/cpu-flags.h>
#include <syscall.h>



.macro define_syscall_entry id, sym, bad
	.if \id < .last_sysno
		.error "Unordered system call: `\sym' and its predecessor"
	.endif
	.rept \id - .last_sysno
		.long \bad
	.endr
	.long \sym
	.last_sysno = \id + 1
.endm

.section .rodata.hot
PUBLIC_OBJECT(x86_syscall_router)
.last_sysno = __NR_syscall_min
#define __SYSCALL(id,sym)     define_syscall_entry id, sym, x86_bad_syscall;
#include <asm/syscallno.ci>
	.rept (__NR_syscall_max+1) - .last_sysno
		.long x86_bad_syscall
	.endr
SYMEND(x86_syscall_router)

.section .rodata.hot
PUBLIC_OBJECT(x86_xsyscall_router)
.last_sysno = __NR_xsyscall_min
#define __XSYSCALL(id,sym)     define_syscall_entry id, sym, x86_bad_syscall;
#include <asm/syscallno.ci>
	.rept (__NR_xsyscall_max+1) - .last_sysno
		.long x86_bad_syscall
	.endr
SYMEND(x86_xsyscall_router)


.macro define_syscall_argc id, sym
	.if \id < .last_sysno
		.error "Unordered system call: `\sym' and its predecessor"
	.endif
	.rept \id - .last_sysno
		.byte 0
	.endr
	.byte argc_\sym
	.last_sysno = \id + 1
.endm

.section .rodata
PUBLIC_OBJECT(x86_syscall_argc)
.last_sysno = __NR_syscall_min
#define __SYSCALL(id,sym)     define_syscall_argc id, sym;
#include <asm/syscallno.ci>
	.rept (__NR_syscall_max+1) - .last_sysno
		.byte 0
	.endr
SYMEND(x86_syscall_argc)

.section .rodata
PUBLIC_OBJECT(x86_xsyscall_argc)
.last_sysno = __NR_xsyscall_min
#define __XSYSCALL(id,sym)     define_syscall_argc id, sym;
#include <asm/syscallno.ci>
	.rept (__NR_xsyscall_max+1) - .last_sysno
		.byte 0
	.endr
SYMEND(x86_xsyscall_argc)



.macro define_syscall_restart id, sym
	.if \id < .last_sysno
		.error "Unordered system call: `\sym' and its predecessor"
	.endif
	.rept \id - .last_sysno
		.byte X86_SYSCALL_RESTART_FAUTO
	.endr
	.byte restart_\sym
	.last_sysno = \id + 1
.endm

.section .rodata
PUBLIC_OBJECT(x86_syscall_restart)
.last_sysno = __NR_syscall_min
#define __SYSCALL(id,sym)     define_syscall_restart id, sym;
#include <asm/syscallno.ci>
	.rept (__NR_syscall_max+1) - .last_sysno
		.byte X86_SYSCALL_RESTART_FAUTO
	.endr
SYMEND(x86_syscall_restart)

.section .rodata
PUBLIC_OBJECT(x86_xsyscall_restart)
.last_sysno = __NR_xsyscall_min
#define __XSYSCALL(id,sym)     define_syscall_restart id, sym;
#include <asm/syscallno.ci>
	.rept (__NR_xsyscall_max+1) - .last_sysno
		.byte X86_SYSCALL_RESTART_FAUTO
	.endr
SYMEND(x86_xsyscall_restart)



/* Weakly redirect all unimplemented system calls. */
#if X86_SYSCALL_RESTART_FAUTO != 0
#define __SET_RESTART(x)  restart_##x = X86_SYSCALL_RESTART_FAUTO;
#else
#define __SET_RESTART(x)  /* nothing */
#endif

#define __SYSCALL(id,sym) \
	.hidden sym, argc_##sym, restart_##sym; \
	.global sym, argc_##sym, restart_##sym; \
	.weak sym, argc_##sym, restart_##sym; \
	sym = x86_bad_syscall; \
	argc_##sym = 6; \
	__SET_RESTART(sym)
#define __XSYSCALL(id,sym)     __SYSCALL(id,sym)
#include <asm/syscallno.ci>



#ifdef CONFIG_X86_FIXED_SEGMENTATION
#define IF_CONFIG_X86_SEGMENTATION(x)           x
#define IF_NOT_CONFIG_X86_FIXED_SEGMENTATION(x) /* nothing */
#else /* CONFIG_X86_FIXED_SEGMENTATION */
#define IF_CONFIG_X86_SEGMENTATION(x)           x
#define IF_NOT_CONFIG_X86_FIXED_SEGMENTATION(x) x
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */

#define SYSCALL_EXIT_BLOCK(name) \
	.cfi_remember_state; \
name:; \
	popl_cfi_r  X86_SYSCALL_REG0; \
	popl_cfi_r  X86_SYSCALL_REG1; \
	popl_cfi_r  X86_SYSCALL_REG2; \
	popl_cfi_r  X86_SYSCALL_REG3; \
	popl_cfi_r  X86_SYSCALL_REG4; \
	popl_cfi_r  X86_SYSCALL_REG5; \
	addl  $4, %esp; /* ORIG_EAX */ \
	.cfi_adjust_cfa_offset -4; \
name##_segments:; \
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %gs); \
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %fs); \
	IF_NOT_CONFIG_X86_FIXED_SEGMENTATION(popl_cfi_r %es); \
	IF_NOT_CONFIG_X86_FIXED_SEGMENTATION(popl_cfi_r %ds); \
name##_iret_fixup:; \
	cli; \
	cmpl    $(X86_USER_CS), X86_IRREGS_USER32_OFFSETOF_CS(%esp); \
	jne     1f /* Custom return location (just use IRET) */; \
	/* Use `sysexit' */ \
	movl    X86_IRREGS_USER32_OFFSETOF_USERESP(%esp), %ecx; \
	movl    X86_IRREGS_USER32_OFFSETOF_EIP(%esp), %edx; \
	addl    $8, %esp; \
	.cfi_adjust_cfa_offset -8; \
	/* Prevent popfl from enabling interrupts to bypass a race condition \
	 * that could result in an interrupt attempting to re-direct user-space \
	 * at a time where registers it would modify were already loaded. \
	 * This race condition doesn't happen with IRET, because it executes \
	 * atomically (or rather: without interrupts). */ \
	andl    $(~EFLAGS_IF), (%esp); \
	popfl; \
	/* Enable interrupts in a way that delays its execution for 1 \
	 * additional instruction, meaning that no interrupts can occurr \
	 * before `sysexit' actually returns to user-space. \
	 * NOTE: `popfl' doesn't have this effect, so this clutch is required. */ \
	sti; \
	sysexit; \
1:	iret; \
	.cfi_restore_state; \
	nop; \
	.cfi_remember_state; \
name##64:; \
	popl_cfi_r  X86_SYSCALL_REG0; \
	popl_cfi_r  X86_SYSCALL_REG1; \
	addl        $4, %esp; \
	.cfi_adjust_cfa_offset -4; \
	.cfi_same_value %edx; /* Don't override the high 32 bits of the return value. */ \
	popl_cfi_r  X86_SYSCALL_REG3; \
	popl_cfi_r  X86_SYSCALL_REG4; \
	popl_cfi_r  X86_SYSCALL_REG5; \
	addl  $4, %esp; /* ORIG_EAX */ \
	.cfi_adjust_cfa_offset -4; \
	/*jmp         name##_segments;*/ \
	/* Must use `iret', because we mustn't clobber EDX for 64-bit return values. */ \
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %gs); \
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %fs); \
	IF_NOT_CONFIG_X86_FIXED_SEGMENTATION(popl_cfi_r %es); \
	IF_NOT_CONFIG_X86_FIXED_SEGMENTATION(popl_cfi_r %ds); \
	iret; \
	.cfi_restore_state


.section .text.hot
.sysenter_kernel_entry_start = .
INTERN_ENTRY(sysenter_kernel_entry)
	/* `IA32_SYSENTER_ESP' points at the ESP0 field of the CPU's TSS
	 * In other words: we only need to replace ESP with its own
	 * dereference, and we already have `THIS_TASK->t_stackend'. */
	movl  %ss:0(%esp), %esp

	/* Construct an IRET tail. */
	pushl    $(X86_USER_DS) /* %ss */
	pushl    %ebp           /* %useresp */
	pushfl
	orl      $(EFLAGS_IF), %ss:(%esp)
	pushl    $(X86_USER_CS) /* %cs */
	pushl    %edi           /* %eip */

	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4

	/* Enable interrupts. */
	sti

#ifndef CONFIG_X86_FIXED_SEGMENTATION
	pushl_cfi_r %ds
	pushl_cfi_r %es
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
	pushl_cfi_r %fs
	pushl_cfi_r %gs
INTERN_ENTRY(sysenter_after_segments)
	pushl_cfi_r %eax /* ORIG_EAX */

	/* Push system call registers. */
	pushl_cfi_r X86_SYSCALL_REG5
	pushl_cfi_r X86_SYSCALL_REG4
	pushl_cfi_r X86_SYSCALL_REG3
	pushl_cfi_r X86_SYSCALL_REG2
	pushl_cfi_r X86_SYSCALL_REG1
	pushl_cfi_r X86_SYSCALL_REG0

	/* Load segments. */
	call    x86_load_segments_ecx
.sysenter_after_tracing:

	/* Do the system call */
	cmpl    $(__NR_syscall_max), %eax
	ja      .sysenter_extended_syscall

	/* Load additional arguments from user-space. */
	cmpb    $5, x86_syscall_argc(%eax)
	jb      1f
	cmpl    $(KERNEL_BASE), %ebp
	jae     .sysenter_bad_args
	movl    0(%ebp), %ecx /* ARG#4 */
	movl    %ecx, 16(%esp)
	cmpb    $6, x86_syscall_argc(%eax)
	jb      1f
	movl    4(%ebp), %ecx /* ARG#5 */
	movl    %ecx, 20(%esp)
1:	calll  *x86_syscall_router(,%eax,4)
	SYSCALL_EXIT_BLOCK(.sysenter_return)

.sysenter_extended_syscall:
	cmpl    $(__NR_xsyscall_max), %eax
	ja      .sysenter_except_syscall
	cmpl    $(__NR_xsyscall_min), %eax
	jb      .sysenter_bad_syscall
	/* Load additional arguments from user-space. */
	cmpb    $5, (x86_xsyscall_argc - __NR_xsyscall_min)(%eax)
	jb      1f
	cmpl    $(KERNEL_BASE), %ebp
	jae     .sysenter_bad_args
	movl    0(%ebp), %ecx /* ARG#4 (CAUTION: SEGFAULT) */
	movl    %ecx, 16(%esp)
	cmpb    $6, (x86_xsyscall_argc - __NR_xsyscall_min)(%eax)
	jb      1f
	movl    4(%ebp), %ecx /* ARG#5 (CAUTION: SEGFAULT) */
	movl    %ecx, 20(%esp)
	/* Extended, kos-specific system call */
1:	pushl_cfi $.sysenter_return /* Return address... */
	jmpl    *(x86_xsyscall_router - (__NR_xsyscall_min*4) & 0xffffffff)(,%eax,4)
	.cfi_adjust_cfa_offset -4
.sysenter_bad_args:
	pushl_cfi %ebp          /* addr */
	pushl_cfi $0            /* reason */
	pushl_cfi $(E_SEGFAULT) /* code */
	call      error_throwf
	.cfi_adjust_cfa_offset -12
.sysenter_except_syscall:
	testl     $0x80000000, %eax
	jz        .sysenter_bad_syscall
	andl      $~0x80000000, %eax
	jmp       .sysenter_after_tracing
.sysenter_bad_syscall:
	pushl_cfi %eax /* Sysno */
	pushl_cfi %ebp /* Extended arguments vector */
	movl      %esp, %ecx
	call      x86_throw_bad_syscall_sysenter
SYMEND(sysenter_after_segments)
SYMEND(sysenter_kernel_entry)
	.cfi_endproc
.sysenter_kernel_entry_end = .

X86_DEFINE_SYSCALL_GUARD(
	.sysenter_kernel_entry_start,
	.sysenter_kernel_entry_end
)





/* .xdata, because we may modify this code */
.section .xdata.hot

.irq_80_start = .
INTERN_ENTRY(irq_80)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4
#ifndef CONFIG_X86_FIXED_SEGMENTATION
	pushl_cfi_r  %ds
	pushl_cfi_r  %es
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
	pushl_cfi_r  %fs
	pushl_cfi_r  %gs
INTERN_ENTRY(irq_80_after_segments)
	pushl_cfi_r %eax /* ORIG_EAX */

	pushl_cfi_r X86_SYSCALL_REG5
	pushl_cfi_r X86_SYSCALL_REG4
	pushl_cfi_r X86_SYSCALL_REG3
	pushl_cfi_r X86_SYSCALL_REG2
	pushl_cfi_r X86_SYSCALL_REG1
	pushl_cfi_r X86_SYSCALL_REG0

	call    x86_load_segments_ecx
.irq_80_after_tracing:

	cmpl    $(__NR_syscall_max), %eax
	ja      .irq_80_extended_syscall

	/* Regular, linux-compatible system call */
	calll   *x86_syscall_router(,%eax,4)
	SYSCALL_EXIT_BLOCK(.irq_80_return)

.irq_80_extended_syscall:
	cmpl    $(__NR_xsyscall_max), %eax
	ja      .irq_80_except_syscall
	cmpl    $(__NR_xsyscall_min), %eax
	jb      .irq_80_bad_syscall
	/* Extended, kos-specific system call */
	pushl_cfi $.irq_80_return /* Return address... */
	jmpl    *(x86_xsyscall_router - (__NR_xsyscall_min*4) & 0xffffffff)(,%eax,4)
	.cfi_adjust_cfa_offset -4

.irq_80_except_syscall:
	testl   $0x80000000, %eax
	jz      .irq_80_bad_syscall
	andl    $~0x80000000, %eax
	jmp     .irq_80_after_tracing
.irq_80_bad_syscall:
	pushl_cfi %eax /* Sysno */
	movl      %esp, %ecx
	call      x86_throw_bad_syscall
	.cfi_endproc
SYMEND(irq_80)
.irq_80_end = .

/* Define a syscall guard that will translate exceptions into errno values. */
X86_DEFINE_SYSCALL_GUARD(
	.irq_80_start,
	.irq_80_end
)









/* System call tracing wrappers */
.section .text.hot
.syscall_trace_begin = .
INTERN_ENTRY(irq_80_trace)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4

#ifndef CONFIG_X86_FIXED_SEGMENTATION
	pushl_cfi_r  %ds
	pushl_cfi_r  %es
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
	pushl_cfi_r  %fs
	pushl_cfi_r  %gs
	pushl_cfi_r %eax /* ORIG_EAX */

	pushl_cfi_r X86_SYSCALL_REG5
	pushl_cfi_r X86_SYSCALL_REG4
	pushl_cfi_r X86_SYSCALL_REG3
	pushl_cfi_r X86_SYSCALL_REG2
	pushl_cfi_r X86_SYSCALL_REG1
	pushl_cfi_r X86_SYSCALL_REG0

	call    x86_load_segments_ecx
	pushl_cfi %esp
	call    syscall_trace
	.cfi_adjust_cfa_offset -4
	movl    24(%esp), %eax /* Reload EAX */

	jmp     .irq_80_after_tracing
	.cfi_endproc
SYMEND(irq_80_trace)


.section .text.hot
INTERN_ENTRY(sysenter_kernel_entry_trace)
	/* `IA32_SYSENTER_ESP' points at the ESP0 field of the CPU's TSS
	 * In other words: we only need to replace ESP with its own
	 * dereference, and we already have `THIS_TASK->t_stackend'. */
	movl  %ss:0(%esp), %esp

	/* Construct an IRET tail. */
	pushl    $(X86_USER_DS) /* %ss */
	pushl    %ebp           /* %useresp */
	pushfl
	orl      $(EFLAGS_IF), %ss:(%esp)
	pushl    $(X86_USER_CS) /* %cs */
	pushl    %edi           /* %eip */

	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4

	/* Enable interrupts. */
	sti

#ifndef CONFIG_X86_FIXED_SEGMENTATION
	pushl_cfi_r  %ds
	pushl_cfi_r  %es
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
	pushl_cfi_r  %fs
	pushl_cfi_r  %gs
	pushl_cfi_r %eax /* ORIG_EAX */

	/* Push system call registers. */
	pushl_cfi_r X86_SYSCALL_REG5
	pushl_cfi_r X86_SYSCALL_REG4
	pushl_cfi_r X86_SYSCALL_REG3
	pushl_cfi_r X86_SYSCALL_REG2
	pushl_cfi_r X86_SYSCALL_REG1
	pushl_cfi_r X86_SYSCALL_REG0

	/* Load segments. */
	call    x86_load_segments_ecx

	cmpl    $(KERNEL_BASE), %ebp
	jae     1f

	/* Load additional arguments. */
	andl    $~0x80000000, %eax
	cmpl    $(__NR_syscall_max), %eax
	ja      2f
	cmpb    $5, x86_syscall_argc(%eax)
	jb      1f
	movl    0(%ebp), %ecx /* ARG#4 (CAUTION: SEGFAULT) */
	movl    %ecx, 16(%esp)
	cmpb    $6, x86_syscall_argc(%eax)
	jb      1f
	movl    4(%ebp), %ecx /* ARG#5 (CAUTION: SEGFAULT) */
	movl    %ecx, 20(%esp)
	jmp     1f
2:	cmpl    $(__NR_xsyscall_min), %eax
	jb      1f
	cmpl    $(__NR_xsyscall_max), %eax
	ja      1f
	cmpb    $5, (x86_xsyscall_argc - __NR_xsyscall_min)(%eax)
	jb      1f
	movl    0(%ebp), %ecx /* ARG#4 (CAUTION: SEGFAULT) */
	movl    %ecx, 16(%esp)
	cmpb    $6, (x86_xsyscall_argc - __NR_xsyscall_min)(%eax)
	jb      1f
	movl    4(%ebp), %ecx /* ARG#5 (CAUTION: SEGFAULT) */
	movl    %ecx, 20(%esp)

1:	pushl_cfi %esp /* regs */
	call    syscall_trace
	.cfi_adjust_cfa_offset -4
	movl    24(%esp), %eax /* Reload EAX */

	jmp     .sysenter_after_tracing
	.cfi_endproc
SYMEND(sysenter_kernel_entry_trace)
.syscall_trace_end = .
X86_DEFINE_SYSCALL_GUARD(.syscall_trace_begin,.syscall_trace_end)





.section .text.cold
INTERN_ENTRY(x86_bad_syscall)
	.cfi_startproc
	popl_cfi  %edx /* Return address */
	.cfi_register %eip, %edx
	pushl_cfi %eax /* Sysno */
	movl      %esp, %ecx
	pushl_cfi %edx /* Return address */
	.cfi_rel_offset %eip, 0
	call      x86_throw_bad_syscall
	popl_cfi  %edx /* Return address */
	.cfi_register %eip, %edx
	addl      $4, %esp /* Sysno */
	.cfi_adjust_cfa_offset -4
	jmp       *%edx
	.cfi_endproc
SYMEND(x86_bad_syscall)



/* This must be added to the return EIP if a system call wants to return 64 bits. */
INTERN_CONST(x86_syscall64_adjustment,.irq_80_return64 - .irq_80_return)


/* Define the memory location that need to be updated if
 * the `sysexit' instruction isn't supported by the host. */
DEFINE_INTERN_ALIAS(x86_sysexit_fixup_1,.irq_80_return_iret_fixup)






.section .data
PUBLIC_ENTRY(x86_syscall_exec80)
	.cfi_startproc
	.byte 0xe9 /* `jmp x86_syscall_exec80_ntrace' */
INTERN_ENTRY(x86_syscall_exec80_fixup)
	.long x86_syscall_exec80_ntrace - (. + 4)
SYMEND(x86_syscall_exec80_fixup)
	.cfi_endproc
SYMEND(x86_syscall_exec80)

.section .text
PUBLIC_ENTRY(x86_syscall_exec80_trace)
	.cfi_startproc
	movl  %taskseg:TASK_OFFSETOF_STACKEND, %ecx
	/* Generate a `struct syscall_trace_regs' to-be passed to `syscall_trace()' */
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_SS)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_USERESP)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_EFLAGS)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_CS)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_EIP)(%ecx)
#ifndef CONFIG_X86_FIXED_SEGMENTATION
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_SEGMENTS+X86_SEGMENTS32_OFFSETOF_DS)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_SEGMENTS+X86_SEGMENTS32_OFFSETOF_ES)(%ecx)
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_SEGMENTS+X86_SEGMENTS32_OFFSETOF_FS)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_SEGMENTS+X86_SEGMENTS32_OFFSETOF_GS)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EAX)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EBP)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EDI)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_ESI)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EDX)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_ECX)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EBX)(%ecx)
	pushl_cfi %esp /* regs */
	call  syscall_trace
	.cfi_adjust_cfa_offset -4 /* STDCALL... (callee-argument-cleanup) */
#ifdef CONFIG_X86_FIXED_SEGMENTATION
	addl  $(14 * 4), %esp
	.cfi_adjust_cfa_offset -(14 * 4)
#else /* CONFIG_X86_FIXED_SEGMENTATION */
	addl  $(16 * 4), %esp
	.cfi_adjust_cfa_offset -(16 * 4)
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
PUBLIC_ENTRY(x86_syscall_exec80_ntrace)
	movl  %taskseg:TASK_OFFSETOF_STACKEND, %ecx
	movl  ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EAX)(%ecx), %eax

	/* Since this mechanism is mainly used for `sigreturn', as well as
	 * the fact that `sys_sigreturn' would normally have to query an RPC
	 * callback to gain access to the full user-space register state, we
	 * specifically optimize for it, as we already have that state! */
	andl  $~0x80000000, %eax
	cmpl  $(SYS_sigreturn), %eax
	jne   1f
	leal  (-X86_HOSTCONTEXT_USER32_SIZE)(%ecx), %eax
	pushl_cfi $(TASK_USERCTX_TYPE_INTR_INTERRUPT) /* mode */
	pushl_cfi %eax /* context */
	pushl_cfi $0   /* arg (unused) */
	call  x86_sigreturn_impl
	.cfi_adjust_cfa_offset -12 /* STDCALL (callee-argument-cleanup) */
	ret

1:	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EBP)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EDI)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_ESI)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EDX)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_ECX)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EBX)(%ecx)

	cmpl  $__NR_syscall_max, %eax
	ja    .exec80_extended_syscall
	/* Basic, linux-compatible system call */
	calll *x86_syscall_router(,%eax,4)
.exec80_return:
	addl  $24, %esp
	.cfi_adjust_cfa_offset -24
	movl  %taskseg:TASK_OFFSETOF_STACKEND, %ecx
	movl  %eax, ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EAX)(%ecx)
	ret
	.cfi_adjust_cfa_offset 24
.exec80_bad_syscall:
	pushl_cfi %eax /* Sysno */
	movl      %esp, %ecx
	call      x86_throw_bad_syscall
	.cfi_adjust_cfa_offset -4
.exec80_return64:
	addl  $24, %esp
	.cfi_adjust_cfa_offset -24
	movl  %taskseg:TASK_OFFSETOF_STACKEND, %ecx
	movl  %eax, ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EAX)(%ecx)
	movl  %edx, ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EDX)(%ecx)
	ret
	.cfi_adjust_cfa_offset 24
.exec80_extended_syscall:
	cmpl  $(__NR_xsyscall_min), %eax
	jb    .exec80_bad_syscall
	cmpl  $(__NR_xsyscall_max), %eax
	ja    .exec80_bad_syscall
	/* Extended, kos-specific system call */
	pushl_cfi $.exec80_return
	jmpl  *(x86_xsyscall_router - (__NR_xsyscall_min*4) & 0xffffffff)(,%eax,4)
//	.cfi_adjust_cfa_offset -4
	.cfi_endproc
SYMEND(x86_syscall_exec80_ntrace)

















