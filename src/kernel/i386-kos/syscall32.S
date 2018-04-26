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



.section .rodata.hot
.macro define_syscall_entry id, sym
	.if \id < .last_sysno
		.error "Unordered system call: `\sym' and its predecessor"
	.endif
	.rept \id - .last_sysno
		.long x86_bad_syscall_except
	.endr
	.long \sym
	.last_sysno = \id + 1
.endm

PUBLIC_OBJECT(x86_syscall_router)
.last_sysno = __NR_syscall_min
#define __SYSCALL(id,sym)     define_syscall_entry id, sym;
#define __SYSCALL_ASM(id,sym) __SYSCALL(id,sym)
#include <asm/syscallno.ci>
	.rept (__NR_syscall_max+1) - .last_sysno
		.long x86_bad_syscall_except
	.endr
SYMEND(x86_syscall_router)

.section .rodata.hot
PUBLIC_OBJECT(x86_xsyscall_router)
.last_sysno = __NR_xsyscall_min
#define __XSYSCALL(id,sym)     define_syscall_entry id, sym;
#define __XSYSCALL_ASM(id,sym) __XSYSCALL(id,sym)
#include <asm/syscallno.ci>
	.rept (__NR_xsyscall_max+1) - .last_sysno
		.long x86_bad_syscall_except
	.endr
SYMEND(x86_xsyscall_router)


#ifndef CONFIG_NO_X86_SYSENTER
.section .rodata
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

PUBLIC_OBJECT(x86_syscall_argc)
.last_sysno = __NR_syscall_min
#define __SYSCALL(id,sym)     define_syscall_argc id, sym;
#define __SYSCALL_ASM(id,sym) __SYSCALL(id,sym)
#include <asm/syscallno.ci>
	.rept (__NR_syscall_max+1) - .last_sysno
		.byte 0
	.endr
SYMEND(x86_syscall_argc)

.section .rodata
PUBLIC_OBJECT(x86_xsyscall_argc)
.last_sysno = __NR_xsyscall_min
#define __XSYSCALL(id,sym)     define_syscall_argc id, sym;
#define __XSYSCALL_ASM(id,sym) __XSYSCALL(id,sym)
#include <asm/syscallno.ci>
	.rept (__NR_xsyscall_max+1) - .last_sysno
		.byte 0
	.endr
SYMEND(x86_xsyscall_argc)
#endif /* !CONFIG_NO_X86_SYSENTER */



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
#define __SYSCALL_ASM(id,sym) __SYSCALL(id,sym)
#include <asm/syscallno.ci>
	.rept (__NR_syscall_max+1) - .last_sysno
		.byte X86_SYSCALL_RESTART_FAUTO
	.endr
SYMEND(x86_syscall_restart)

.section .rodata
PUBLIC_OBJECT(x86_xsyscall_restart)
.last_sysno = __NR_xsyscall_min
#define __XSYSCALL(id,sym)     define_syscall_restart id, sym;
#define __XSYSCALL_ASM(id,sym) __XSYSCALL(id,sym)
#include <asm/syscallno.ci>
	.rept (__NR_xsyscall_max+1) - .last_sysno
		.byte X86_SYSCALL_RESTART_FAUTO
	.endr
SYMEND(x86_xsyscall_restart)



/* Weakly redirect all unimplemented system calls. */
#ifdef CONFIG_NO_X86_SYSENTER
#define __SYSCALL(id,sym) \
	.hidden sym, restart_##sym; \
	.global sym, restart_##sym; \
	.weak sym, restart_##sym; \
	sym = x86_bad_syscall; \
	restart_##sym = X86_SYSCALL_RESTART_FAUTO;
#else
#define __SYSCALL(id,sym) \
	.hidden sym, argc_##sym, restart_##sym; \
	.global sym, argc_##sym, restart_##sym; \
	.weak sym, argc_##sym, restart_##sym; \
	sym = x86_bad_syscall; \
	argc_##sym = 6; \
	restart_##sym = X86_SYSCALL_RESTART_FAUTO;
#endif
#define __SYSCALL_ASM(id,sym)  __SYSCALL(id,sym)
#define __XSYSCALL(id,sym)     __SYSCALL(id,sym)
#define __XSYSCALL_ASM(id,sym) __SYSCALL(id,sym)
#include <asm/syscallno.ci>



#ifndef CONFIG_NO_X86_SEGMENTATION
#define IF_CONFIG_X86_SEGMENTATION(x) x
#else
#define IF_CONFIG_X86_SEGMENTATION(x) /* nothing */
#endif

#ifdef CONFIG_NO_X86_SYSENTER
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
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %gs); \
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %fs); \
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %es); \
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %ds); \
	iret; \
	.cfi_restore_state; \
	nop; \
	.cfi_remember_state; \
name##_return64:; \
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
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %gs); \
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %fs); \
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %es); \
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %ds); \
	iret; \
	.cfi_restore_state
#else /* CONFIG_NO_X86_SYSENTER */
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
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %es); \
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %ds); \
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
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %es); \
	IF_CONFIG_X86_SEGMENTATION(popl_cfi_r %ds); \
	iret; \
	.cfi_restore_state
#endif /* !CONFIG_NO_X86_SYSENTER */


#ifndef CONFIG_NO_X86_SYSENTER
.section .text.hot
INTERN_ENTRY(sysenter_kernel_entry)
	/* `SYSENTER_ESP_MSR' points at the ESP0 field of the CPU's TSS
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

#ifndef CONFIG_NO_X86_SEGMENTATION
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs
	pushl_cfi_r %gs
#endif
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
	cmpb    $5, x86_syscall_argc(,%eax,1)
	jb      1f
	cmpl    $(KERNEL_BASE), %ebp
	jae     .sysenter_bad_args
	movl    0(%ebp), %ecx /* ARG#4 */
	movl    %ecx, 16(%esp)
	cmpb    $6, x86_syscall_argc(,%eax,1)
	jb      1f
	movl    4(%ebp), %ecx /* ARG#5 */
	movl    %ecx, 20(%esp)
1:	calll  *x86_syscall_router(,%eax,4)
	SYSCALL_EXIT_BLOCK(.sysenter_return)

.sysenter_bad_args:
	movl    $(-EFAULT), %eax
	jmp     .sysenter_return

.sysenter_extended_syscall:
	cmpl    $(__NR_xsyscall_max), %eax
	ja      .sysenter_except_syscall
	cmpl    $(__NR_xsyscall_min), %eax
	jb      .sysenter_bad_syscall
	/* Load additional arguments from user-space. */
	cmpb    $5, (x86_xsyscall_argc - __NR_xsyscall_min)(,%eax,1)
	jb      1f
	cmpl    $(KERNEL_BASE), %ebp
	jae     .sysenter_bad_args
	movl    0(%ebp), %ecx /* ARG#4 (CAUTION: SEGFAULT) */
	movl    %ecx, 16(%esp)
	cmpb    $6, (x86_xsyscall_argc - __NR_xsyscall_min)(,%eax,1)
	jb      1f
	movl    4(%ebp), %ecx /* ARG#5 (CAUTION: SEGFAULT) */
	movl    %ecx, 20(%esp)
	/* Extended, kos-specific system call */
1:	pushl_cfi $.sysenter_return /* Return address... */
	jmpl    *(x86_xsyscall_router - (__NR_xsyscall_min*4) & 0xffffffff)(,%eax,4)
	.cfi_adjust_cfa_offset -4

.sysenter_except_syscall:
	testl   $(0x80000000), %eax
	jnz     .sysenter_with_exceptions
.sysenter_bad_syscall:
	pushl_cfi $.sysenter_return /* Return address... */
	jmp     x86_bad_syscall
	.cfi_adjust_cfa_offset -4
SYMEND(sysenter_after_segments)
SYMEND(sysenter_kernel_entry)

.sysenter_with_exceptions:
	cmpl    $(__NR_syscall_max + 0x80000000), %eax
	ja      2f
	/* Load additional arguments from user-space. */
	cmpl    $5, (x86_syscall_argc - 0x80000000)(,%eax,1)
	jb      1f
	cmpl    $(KERNEL_BASE), %ebp
	jae     .sysenter_bad_args_except
	movl    0(%ebp), %ecx /* ARG#4 (CAUTION: SEGFAULT) */
	movl    %ecx, 16(%esp)
	cmpl    $6, (x86_syscall_argc - 0x80000000)(,%eax,1)
	jb      1f
	movl    4(%ebp), %ecx /* ARG#5 (CAUTION: SEGFAULT) */
	movl    %ecx, 20(%esp)
	/* Regular, linux-compatible system call */
1:	pushl_cfi $.sysenter_return_except /* Return address... */
	jmpl    *(x86_syscall_router - (0x80000000*4) & 0xffffffff)(,%eax,4)
	.cfi_adjust_cfa_offset -4
2:	cmpl    $(__NR_xsyscall_min + 0x80000000), %eax
	jb      .sysenter_bad_syscall_sysenter_except
	cmpl    $(__NR_xsyscall_max + 0x80000000), %eax
	ja      .sysenter_bad_syscall_sysenter_except
	/* Load additional arguments from user-space. */
	cmpb    $5, (x86_xsyscall_argc - (__NR_xsyscall_min + 0x80000000))(,%eax,1)
	jb      1f
	cmpl    $(KERNEL_BASE), %ebp
	jae     .sysenter_bad_args_except
	movl    0(%ebp), %ecx /* ARG#4 (CAUTION: SEGFAULT) */
	movl    %ecx, 16(%esp)
	cmpb    $6, (x86_xsyscall_argc - (__NR_xsyscall_min + 0x80000000))(,%eax,1)
	jb      1f
	movl    4(%ebp), %ecx /* ARG#5 (CAUTION: SEGFAULT) */
	movl    %ecx, 20(%esp)
	/* Extended, kos-specific system call */
1:	calll   *(x86_xsyscall_router - ((__NR_xsyscall_min+0x80000000)*4) & 0xffffffff)(,%eax,4)
	SYSCALL_EXIT_BLOCK(.sysenter_return_except)

.sysenter_bad_args_except:
	pushl_cfi %ebp          /* addr */
	pushl_cfi $0            /* reason */
	pushl_cfi $(E_SEGFAULT) /* code */
	call      error_throwf
#if 0 /* Never returns */
	addl      $12, %esp
	.cfi_adjust_cfa_offset -8
	jmp       .sysenter_return_except
#else
	.cfi_adjust_cfa_offset -8
#endif
.sysenter_bad_syscall_sysenter_except:
	pushl_cfi %eax /* Sysno */
	pushl_cfi %ebp /* Extended arguments vector */
	movl      %esp, %ecx
	call      x86_throw_bad_syscall_sysenter
#if 0 /* Never returns */
	addl      $8, %esp /* Sysno + extended arguments vector */
	.cfi_adjust_cfa_offset -8
	jmp       .sysenter_return_except
#else
/*	.cfi_adjust_cfa_offset -8 */
#endif
	.cfi_endproc
SYMEND(.sysenter_with_exceptions)
.sysenter_kernel_entry_end = .
X86_DEFINE_SYSCALL_GUARD(sysenter_kernel_entry,.sysenter_kernel_entry_end)
#endif /* !CONFIG_NO_X86_SYSENTER */





#ifdef CONFIG_NO_X86_SYSENTER
.section .text.hot
#else
/* .xdata, because we may modify this code */
.section .xdata.hot
#endif
INTERN_ENTRY(irq_80)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4
#ifndef CONFIG_NO_X86_SEGMENTATION
	pushl_cfi_r  %ds
	pushl_cfi_r  %es
	pushl_cfi_r  %fs
	pushl_cfi_r  %gs
#endif
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
	testl   $(0x80000000), %eax
	jnz     .irq_80_with_exceptions
.irq_80_bad_syscall:
	pushl_cfi $.irq_80_return /* Return address... */
	jmp     x86_bad_syscall
	.cfi_adjust_cfa_offset -4
SYMEND(irq_80)

.irq_80_with_exceptions:
	cmpl    $(__NR_syscall_max + 0x80000000), %eax
	ja      1f
	/* Regular, linux-compatible system call */
	pushl_cfi $.irq_80_return_except
	jmpl    *(x86_syscall_router - (0x80000000*4) & 0xffffffff)(,%eax,4)
	.cfi_adjust_cfa_offset -4
1:	cmpl    $(__NR_xsyscall_min + 0x80000000), %eax
	jb      .irq_80_bad_syscall_except
	cmpl    $(__NR_xsyscall_max + 0x80000000), %eax
	ja      .irq_80_bad_syscall_except
	/* Extended, kos-specific system call */
	calll   *(x86_xsyscall_router - ((__NR_xsyscall_min+0x80000000)*4) & 0xffffffff)(,%eax,4)
	SYSCALL_EXIT_BLOCK(.irq_80_return_except)

.irq_80_bad_syscall_except:
	pushl_cfi %eax /* Sysno */
	movl      %esp, %ecx
	call      x86_throw_bad_syscall
#if 0 /* Never returns */
	addl      $4, %esp /* Sysno */
	.cfi_adjust_cfa_offset -4
	jmp       .irq_80_return_except
#else
/*	.cfi_adjust_cfa_offset -4 */
#endif
	.cfi_endproc
.irq_80_end = .
SYMEND(.irq_80_with_exceptions)
/* Define a syscall guard that will translate exceptions into errno values. */
X86_DEFINE_SYSCALL_GUARD(irq_80,.irq_80_end)









/* System call tracing wrappers */
.section .text.hot
.syscall_trace_begin = .
INTERN_ENTRY(irq_80_trace)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4

#ifndef CONFIG_NO_X86_SEGMENTATION
	pushl_cfi_r  %ds
	pushl_cfi_r  %es
	pushl_cfi_r  %fs
	pushl_cfi_r  %gs
#endif
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

#ifndef CONFIG_NO_X86_SYSENTER
.section .text.hot
INTERN_ENTRY(sysenter_kernel_entry_trace)
	/* `SYSENTER_ESP_MSR' points at the ESP0 field of the CPU's TSS
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

#ifndef CONFIG_NO_X86_SEGMENTATION
	pushl_cfi_r  %ds
	pushl_cfi_r  %es
	pushl_cfi_r  %fs
	pushl_cfi_r  %gs
#endif
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
	cmpb    $5, x86_syscall_argc(,%eax,1)
	jb      1f
	movl    0(%ebp), %ecx /* ARG#4 (CAUTION: SEGFAULT) */
	movl    %ecx, 16(%esp)
	cmpb    $6, x86_syscall_argc(,%eax,1)
	jb      1f
	movl    4(%ebp), %ecx /* ARG#5 (CAUTION: SEGFAULT) */
	movl    %ecx, 20(%esp)
	jmp     1f
2:	cmpl    $(__NR_xsyscall_min), %eax
	jb      1f
	cmpl    $(__NR_xsyscall_max), %eax
	ja      1f
	cmpb    $5, (x86_xsyscall_argc - __NR_xsyscall_min)(,%eax,1)
	jb      1f
	movl    0(%ebp), %ecx /* ARG#4 (CAUTION: SEGFAULT) */
	movl    %ecx, 16(%esp)
	cmpb    $6, (x86_xsyscall_argc - __NR_xsyscall_min)(,%eax,1)
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
#endif /* !CONFIG_NO_X86_SYSENTER */
.syscall_trace_end = .
X86_DEFINE_SYSCALL_GUARD(.syscall_trace_begin,.syscall_trace_end)

















.section .text.hot
INTERN_ENTRY(x86_bad_syscall)
	.cfi_startproc
	movl    $(-ENOSYS), %eax
	ret
	.cfi_endproc
SYMEND(x86_bad_syscall)

.section .text.hot
INTERN_ENTRY(x86_bad_syscall_except)
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
SYMEND(x86_bad_syscall_except)



/* This must be added to the return EIP if a system call wants to return 64 bits. */
INTERN_CONST(x86_syscall64_adjustment,.irq_80_return64 - .irq_80_return)


#ifndef CONFIG_NO_X86_SYSENTER
/* Define the 2 memory locations that need to be updated
 * if the `sysexit' instruction isn't supported by the host. */
DEFINE_INTERN_ALIAS(x86_sysexit_fixup_1,.irq_80_return_iret_fixup)
DEFINE_INTERN_ALIAS(x86_sysexit_fixup_2,.irq_80_return_except_iret_fixup)
#endif






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
#ifndef CONFIG_NO_X86_SEGMENTATION
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS)(%ecx)
#endif
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBP)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDI)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ESI)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBX)(%ecx)
	pushl_cfi %esp /* regs */
	call  syscall_trace
	.cfi_adjust_cfa_offset -4 /* STDCALL... (callee-argument-cleanup) */
#ifndef CONFIG_NO_X86_SEGMENTATION
	addl  $(16 * 4), %esp
	.cfi_adjust_cfa_offset -(16 * 4)
#else
	addl  $(12 * 4), %esp
	.cfi_adjust_cfa_offset -(12 * 4)
#endif
PUBLIC_ENTRY(x86_syscall_exec80_ntrace)
	movl  %taskseg:TASK_OFFSETOF_STACKEND, %ecx
	movl  ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX)(%ecx), %eax

	/* Since this mechanism is mainly used for `sigreturn', as well as
	 * the fact that `sys_sigreturn' would normally have to query an RPC
	 * callback to gain access to the full user-space register state, we
	 * specifically optimize for it, as we already have that state! */
	cmpl  $(SYS_sigreturn), %eax
	jne   1f
	leal  (-X86_HOSTCONTEXT_USER32_SIZE)(%ecx), %eax
	pushl_cfi $(TASK_USERCTX_TYPE_INTR_INTERRUPT) /* mode */
	pushl_cfi %eax /* context */
	pushl_cfi $0   /* arg (unused) */
	call  x86_sigreturn_impl
	.cfi_adjust_cfa_offset -12 /* STDCALL (callee-argument-cleanup) */
	ret

1:	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBP)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDI)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ESI)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX)(%ecx)
	pushl_cfi ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBX)(%ecx)

	cmpl  $__NR_syscall_max, %eax
	ja    .exec80_extended_syscall
	/* Basic, linux-compatible system call */
	calll *x86_syscall_router(,%eax,4)
.exec80_return:
	addl  $24, %esp
	.cfi_adjust_cfa_offset -24
	movl  %taskseg:TASK_OFFSETOF_STACKEND, %ecx
	movl  %eax, ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX)(%ecx)
	ret
	.cfi_adjust_cfa_offset 24
.exec80_bad_syscall:
	movl  $-ENOSYS, %eax
	jmp   .exec80_return
.space x86_syscall64_adjustment - (. - .exec80_return)
.exec80_return64:
	addl  $24, %esp
	.cfi_adjust_cfa_offset -24
	movl  %taskseg:TASK_OFFSETOF_STACKEND, %ecx
	movl  %eax, ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX)(%ecx)
	movl  %edx, ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX)(%ecx)
	ret
	.cfi_adjust_cfa_offset 24
.exec80_extended_syscall:
	cmpl  $(__NR_xsyscall_min), %eax
	jb    .exec80_bad_syscall
	cmpl  $(__NR_xsyscall_max), %eax
	ja    .exec80_with_exceptions
	/* Extended, kos-specific system call */
	pushl_cfi $.exec80_return
	jmpl  *(x86_xsyscall_router - (__NR_xsyscall_min*4) & 0xffffffff)(,%eax,4)
	.cfi_adjust_cfa_offset -4
.exec80_with_exceptions:
	/* System call w/ exceptions. */
	cmpl    $(__NR_syscall_max + 0x80000000), %eax
	ja      2f
	/* Regular, linux-compatible system call */
1:	pushl_cfi $.exec80_with_exceptions_return /* Return address... */
	jmpl    *(x86_syscall_router - (0x80000000*4) & 0xffffffff)(,%eax,4)
	.cfi_adjust_cfa_offset -4
2:	cmpl    $(__NR_xsyscall_min + 0x80000000), %eax
	jb      .exec80_bad_syscall_except
	cmpl    $(__NR_xsyscall_max + 0x80000000), %eax
	ja      .exec80_bad_syscall_except
	/* Extended, kos-specific system call */
1:	calll   *(x86_xsyscall_router - ((__NR_xsyscall_min+0x80000000)*4) & 0xffffffff)(,%eax,4)
.exec80_with_exceptions_return:
	addl  $24, %esp
	.cfi_adjust_cfa_offset -24
	movl  %taskseg:TASK_OFFSETOF_STACKEND, %ecx
	movl  %eax, ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX)(%ecx)
	ret
.exec80_bad_syscall_except:
	pushl_cfi %eax /* Sysno */
	movl      %esp, %ecx
	call      x86_throw_bad_syscall
#if 0 /* Never returns */
	addl      $4, %esp /* Sysno */
	.cfi_adjust_cfa_offset -4
	jmp       .exec80_with_exceptions_return
#else
	.cfi_adjust_cfa_offset -4
#endif
.space x86_syscall64_adjustment - (. - .exec80_with_exceptions_return)
.exec80_with_exceptions_return64:
	addl  $24, %esp
	.cfi_adjust_cfa_offset -24
	movl  %taskseg:TASK_OFFSETOF_STACKEND, %ecx
	movl  %eax, ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX)(%ecx)
	movl  %edx, ((-X86_HOSTCONTEXT_USER32_SIZE)+X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX)(%ecx)
	ret
//	.cfi_adjust_cfa_offset 24
	.cfi_endproc
SYMEND(x86_syscall_exec80_ntrace)

















