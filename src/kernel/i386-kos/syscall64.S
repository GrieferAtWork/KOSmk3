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
#include <asm/universal.h>
#include <asm/cpu-flags.h>
#include <syscall.h>



.macro define_syscall_entry id, sym, bad
	.if \id < .last_sysno
		.error "Unordered system call: `\sym' and its predecessor"
	.endif
	.rept \id - .last_sysno
		.quad \bad
	.endr
	.quad \sym
	.last_sysno = \id + 1
.endm

.section .rodata.hot
PUBLIC_OBJECT(x86_syscall_router)
.last_sysno = __NR_syscall_min
#define __SYSCALL(id,sym)     define_syscall_entry id, sym, x86_bad_syscall;
#include <asm/syscallno.ci>
	.rept (__NR_syscall_max+1) - .last_sysno
		.quad x86_bad_syscall
	.endr
SYMEND(x86_syscall_router)

.section .rodata.hot
PUBLIC_OBJECT(x86_xsyscall_router)
.last_sysno = __NR_xsyscall_min
#define __XSYSCALL(id,sym)     define_syscall_entry id, sym, x86_bad_syscall;
#include <asm/syscallno.ci>
	.rept (__NR_xsyscall_max+1) - .last_sysno
		.quad x86_bad_syscall
	.endr
SYMEND(x86_xsyscall_router)

.section .rodata.hot
PUBLIC_OBJECT(x86_syscall_compat_router)
.last_sysno = __NR_syscall_min
#define __SYSCALL(id,sym)     define_syscall_entry id, sym ## _compat, x86_bad_syscall;
#include <asm/syscallno.ci>
	.rept (__NR_syscall_max+1) - .last_sysno
		.quad x86_bad_syscall
	.endr
SYMEND(x86_syscall_compat_router)

.section .rodata.hot
PUBLIC_OBJECT(x86_xsyscall_compat_router)
.last_sysno = __NR_xsyscall_min
#define __XSYSCALL(id,sym)     define_syscall_entry id, sym ## _compat, x86_bad_syscall;
#include <asm/syscallno.ci>
	.rept (__NR_xsyscall_max+1) - .last_sysno
		.quad x86_bad_syscall
	.endr
SYMEND(x86_xsyscall_compat_router)


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

.section .rodata
PUBLIC_OBJECT(x86_syscall_compat_argc)
.last_sysno = __NR_syscall_min
#define __SYSCALL(id,sym)     define_syscall_argc id, sym ## _compat;
#include <asm/syscallno.ci>
	.rept (__NR_syscall_max+1) - .last_sysno
		.byte 0
	.endr
SYMEND(x86_syscall_compat_argc)

.section .rodata
PUBLIC_OBJECT(x86_xsyscall_compat_argc)
.last_sysno = __NR_xsyscall_min
#define __XSYSCALL(id,sym)     define_syscall_argc id, sym ## _compat;
#include <asm/syscallno.ci>
	.rept (__NR_xsyscall_max+1) - .last_sysno
		.byte 0
	.endr
SYMEND(x86_xsyscall_compat_argc)



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
	.hidden sym, argc_##sym, restart_##sym, sym##_compat, argc_##sym##_compat; \
	.global sym, argc_##sym, restart_##sym, sym##_compat, argc_##sym##_compat; \
	.weak sym, argc_##sym, restart_##sym, sym##_compat, argc_##sym##_compat; \
	.set sym##_compat, x86_bad_syscall; \
	.set sym, x86_bad_syscall; \
	.set argc_##sym##_compat, 6; \
	.set argc_##sym, 6; \
	__SET_RESTART(sym)
#define __XSYSCALL(id,sym)     __SYSCALL(id,sym)
#include <asm/syscallno.ci>



/* Since only user-space is allowed to invoke system calls,
 * we can optimize interrupt entry using that assumption. */
.macro irq_enter_syscall
	swapgs
	sti
#ifdef __X86_IRQENTER_CHK_GSBASE
	__X86_IRQENTER_CHK_GSBASE
#endif
.endm
.macro irq_leave_syscall
	cli
	swapgs
	iretq
.endm

.section .text.hot
.Lsyscall_except_guard_start = .


/* ========================================================================================= */
/* WITHOUT TRACING                                                                           */
/* ========================================================================================= */
INTERN_ENTRY(syscall_kernel_entry)
	/* Kernel entry for `syscall' when tracing is disabled. */
	swapgs
	movq    %taskseg:TASK_OFFSETOF_STACKEND, %rsp
	/* Construct an IRET tail. */
	pushq   $(X86_USER_DS)   /* %ss */
	pushq   %rcx             /* %rsp */
	pushq   %r11             /* %rflags */
	pushq   $(X86_USER_CS)   /* %cs */
	pushq   %rdi             /* %rip */
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
	sti

.Lsyscall64_entry:
	/* Save clobber registers. */
	pushq_cfi_r %rax
	pushq_cfi_r %rdx
	pushq_cfi_r %rsi
	pushq_cfi_r %rdi
	pushq_cfi_r %r8
	pushq_cfi_r %r9
	pushq_cfi_r %r10
	pushq_cfi_r %r11
	pushq_cfi_r %rcx

	/* Adjust for RCX / R10 exchange. */
	xchgq   %rcx, %r10

.Lsyscall_begin:
	cmpq    $__NR_syscall_max, %rax
	ja      .Lsyscall_xsyscall
	callq   *x86_syscall_router(,%rax,8)
.Lsyscall_return:
	.cfi_remember_state
	popq_cfi_r %rcx
	popq_cfi_r %r11
	popq_cfi_r %r10
	popq_cfi_r %r9
	popq_cfi_r %r8
	popq_cfi_r %rdi
	popq_cfi_r %rsi
	popq_cfi_r %rdx
	popq_void_cfi   /* RAX */
	irq_leave_syscall
	.cfi_restore_state
.Lsyscall_xsyscall:
	subq    $__NR_xsyscall_min, %rax
	jb      .Lsyscall_bad_syscall
	cmpq    $(__NR_xsyscall_max - __NR_xsyscall_min), %rax
	ja      .Lsyscall_except_syscall
	pushq_cfi $.Lsyscall_return
	jmpq    *x86_xsyscall_router(,%rax,8)
	.cfi_adjust_cfa_offset -8
.Lsyscall_except_syscall:
	cmpl    $(0x80000000 - __NR_xsyscall_min), %eax
	jb      .Lsyscall_bad_syscall
	addl    $(__NR_xsyscall_min - 0x80000000), %eax
	jmp     .Lsyscall_begin
.Lsyscall_bad_syscall:
	addq    $__NR_xsyscall_min, %rax
	call    x86_bad_syscall
	.cfi_endproc
SYMEND(syscall_kernel_entry)


INTERN_ENTRY(irq_80)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
	irq_enter_syscall

	/* Check for compatibility mode. */
	cmpq    $X86_SEG_USER_CS32, X86_IRREGS64_OFFSETOF_CS(%rsp)
	jne     .Lsyscall64_entry

INTERN_ENTRY(irq_80_compat)
	/* Save clobber registers visible in 32-bit mode. */
	pushq_cfi_r %rax
	pushq_cfi_r %rdx
	pushq_cfi_r %rsi
	pushq_cfi_r %rdi
	pushq_cfi_r %rcx

	/* Transform compatibility-mode registers */
	/*    - %ebx  --> %rdi */
	/*    - %ecx  --> %rsi */
	/*    - %edx  --> %rdx */
	/*    - %esi  --> %r10 */
	/*    - %edi  --> %r8  */
	/*    - %ebp  --> %r9  */
	movzlq  %edi, %r8
	movzlq  %ebp, %r9
	movzlq  %esi, %r10
	movzlq  %ebx, %rdi
	movzlq  %ecx, %rsi

.Lirq_80_compat_begin:
	cmpq    $__NR_syscall_max, %rax
	ja      .Lirq_80_compat_xsyscall
.Lirq_80_compat_do_route:
	callq   *x86_syscall_compat_router(,%rax,8)
.Lirq_80_compat_return:
	.cfi_remember_state
	popq_cfi_r  %rcx
	popq_cfi_r  %rdi
	popq_cfi_r  %rsi
	popq_cfi_r  %rdx
	popq_void_cfi
	irq_leave_syscall
	.cfi_restore_state
.Lirq_80_compat_return64:
	.cfi_remember_state
	popq_cfi_r  %rcx
	popq_cfi_r  %rdi
	popq_cfi_r  %rsi
	pop_void_cfi 2*8
	irq_leave_syscall
	.cfi_restore_state
.Lirq_80_compat_xsyscall:
	subq    $__NR_xsyscall_min, %rax
	jb      .Lirq_80_compat_bad_syscall
	cmpq    $(__NR_xsyscall_max - __NR_xsyscall_min), %rax
	ja      .Lirq_80_compat_except_syscall
.Lirq_80_compat_do_xroute:
	pushq_cfi $.Lirq_80_compat_return
	jmpq    *x86_xsyscall_compat_router(,%rax,8)
	.cfi_adjust_cfa_offset -8
.Lirq_80_compat_except_syscall:
	cmpl    $(0x80000000 - __NR_xsyscall_min), %eax
	jb      .Lirq_80_compat_bad_syscall
	addl    $(__NR_xsyscall_min - 0x80000000), %eax
	jmp     .Lirq_80_compat_begin
.Lirq_80_compat_bad_syscall:
	addq    $__NR_xsyscall_min, %rax
.Lirq_80_compat_bad_syscall2:
	call    x86_bad_syscall
	.cfi_endproc
SYMEND(irq_80_compat)
SYMEND(irq_80)

/* This must be added to the return RIP if a system call wants to return 64 bits. */
INTERN_CONST(x86_syscall64_compat_adjustment,
	.Lirq_80_compat_return64 -
	.Lirq_80_compat_return)


INTERN_ENTRY(sysenter_kernel_entry)
	/* Kernel entry for `sysenter' when tracing is disabled. */
	movq    0(%rsp), %rsp
	/* Construct an IRET tail. */
	pushq   $(X86_USER_DS32) /* %ss */
	pushq   %rbp             /* %rsp */
	pushfq
	orq     $(EFLAGS_IF), (%rsp)
	pushq   $(X86_USER_CS32) /* %cs */
	pushq   %rdi             /* %rip */
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
	irq_enter_syscall

	/* Save clobber registers visible in 32-bit mode. */
	pushq_cfi_r %rax
	pushq_cfi_r %rdx
	pushq_cfi_r %rsi
	pushq_cfi_r %rdi
	pushq_cfi_r %rcx

	/* Transform compatibility-mode registers */
	/*    - %ebx  --> %rdi */
	/*    - %ecx  --> %rsi */
	/*    - %edx  --> %rdx */
	/*    - %esi  --> %r10 */
	movzlq  %esi, %r10
	movzlq  %ebx, %rdi
	movzlq  %ecx, %rsi

	/* Load stack-based arguments (if those exist) */
	/*    - 0(%ebp) --> %r8  */
	/*    - 4(%ebp) --> %r9  */
.Lsysenter_compat_begin:
	cmpq    $__NR_syscall_max, %rax
	ja      .Lsysenter_compat_xsyscall
	cmpb    $4, x86_syscall_compat_argc(%rax)
	jbe     .Lirq_80_compat_do_route
	movzlq  0(%ebp), %r8
	cmpb    $5, x86_syscall_compat_argc(%rax)
	jbe     .Lirq_80_compat_do_route
	movzlq  4(%ebp), %r9
	jmp     .Lirq_80_compat_do_route
.Lsysenter_compat_xsyscall:
	subq    $__NR_xsyscall_min, %rax
	jb      .Lirq_80_compat_bad_syscall
	cmpq    $(__NR_xsyscall_max - __NR_xsyscall_min), %rax
	ja      .Lsysenter_compat_except_syscall
	cmpb    $4, x86_xsyscall_compat_argc(%rax)
	jbe     .Lirq_80_compat_do_xroute
	movzlq  0(%ebp), %r8
	cmpb    $5, x86_xsyscall_compat_argc(%rax)
	jbe     .Lirq_80_compat_do_xroute
	movzlq  4(%ebp), %r9
	jmp     .Lirq_80_compat_do_xroute
.Lsysenter_compat_except_syscall:
	cmpl    $(0x80000000 - __NR_xsyscall_min), %eax
	jb      .Lirq_80_compat_bad_syscall
	addl    $(__NR_xsyscall_min - 0x80000000), %eax
	jmp     .Lsysenter_compat_begin
	.cfi_endproc
SYMEND(sysenter_kernel_entry)









/* ========================================================================================= */
/* WITH TRACING                                                                              */
/* ========================================================================================= */

INTERN_ENTRY(syscall_kernel_entry_trace)
	/* Kernel entry for `syscall' when tracing is enabled. */
	swapgs
	movq    %taskseg:TASK_OFFSETOF_STACKEND, %rsp
	/* Construct an IRET tail. */
	pushq   $(X86_USER_DS)   /* %ss */
	pushq   %rcx             /* %rsp */
	pushq   %r11             /* %rflags */
	pushq   $(X86_USER_CS)   /* %cs */
	pushq   %rdi             /* %rip */
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
	sti
.Lsyscall64_entry_trace:
	/* Save clobber registers. */
	pushq_cfi_r %rax
	pushq_cfi_r %rdx
	pushq_cfi_r %rsi
	pushq_cfi_r %rdi
	pushq_cfi_r %r8
	pushq_cfi_r %r9
	pushq_cfi_r %r10
	movq    %rsp, %rdi
	pushq_cfi_r %r11
	pushq_cfi_r %rcx

	/* Trace this system call. */
	call    syscall_trace

	/* Invoke the system call for real! */
	movq    2 * 8(%rsp), %r10
	movq    3 * 8(%rsp), %r9
	movq    4 * 8(%rsp), %r8
	movq    5 * 8(%rsp), %rdi
	movq    6 * 8(%rsp), %rsi
	movq    7 * 8(%rsp), %rdx
	movq    8 * 8(%rsp), %rax

	jmp     .Lsyscall_begin
	.cfi_endproc
SYMEND(syscall_kernel_entry_trace)


INTERN_ENTRY(irq_80_trace)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
	irq_enter_syscall

	/* Check for compatibility mode. */
	cmpq    $X86_SEG_USER_CS32, X86_IRREGS64_OFFSETOF_CS(%rsp)
	jne     .Lsyscall64_entry_trace
INTERN_ENTRY(irq_80_compat_trace)
	/* Construct the syscall-trace structure. */
	pushq_cfi_r %rax
	pushq_cfi_r %rdx
	pushq_cfi_r %rcx
	pushq_cfi_r %rbx
	pushq_cfi_r %rdi
	pushq_cfi_r %rbp
	pushq_cfi_r %rsi

.Lirq_80_compat_trace_entry:
	movq    %rsp, %rdi
	call    syscall_trace


	popq_cfi_r  %rsi
	popq_cfi_r  %rbp
	/* Transform RSP:
	 *  32  -- RAX -> [RAX]
	 *  24  -- RDX -> [RDX]
	 *  16  -- RCX -> [RSI]
	 *   8  -- RBX -> [RDI]
	 *   0  -- RDI -> [RCX]
	 */
	movq    0(%rsp),  %rbx  /* %rbx = RDI */
	xchgq   8(%rsp),  %rbx  /* [RDI] = %rbx; %rbx = RBX; */
	xchgq   16(%rsp), %rsi  /* [RSI] = %rsi; %rsi = RCX; */
	movq    %rsi,   0(%rsp) /* [RCX] = %rsi; */

	/* Load SYSV-style registers. */
	movzlq  8(%rsp),  %r8  /* [RDI] */
	movzlq  %ebp,     %r9
	movzlq  %edi,     %r8
	movzlq  %esi,     %r10
//	movzlq  %esi,     %rsi /* NOTE: %rsi = [RCX] */
	movzlq  %ebx,     %rdi /* ARG#0 */
	movzlq  24(%rsp), %rdx
	movzlq  32(%rsp), %rax

	jmp     .Lirq_80_compat_begin
	.cfi_endproc
SYMEND(irq_80_compat_trace)
SYMEND(irq_80_trace)


INTERN_ENTRY(sysenter_kernel_entry_trace)
	/* Kernel entry for `sysenter' when tracing is enabled. */
	movq    0(%rsp), %rsp
	/* Construct an IRET tail. */
	pushq   $(X86_USER_DS32) /* %ss */
	pushq   %rbp             /* %rsp */
	pushfq
	orq     $(EFLAGS_IF), (%rsp)
	pushq   $(X86_USER_CS32) /* %cs */
	pushq   %rdi             /* %rip */
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
	irq_enter_syscall


	/* Construct the syscall-trace structure. */
	pushq_cfi_r %rax
	pushq_cfi_r %rdx
	pushq_cfi_r %rcx
	pushq_cfi_r %rbx

	/* Load stack-based arguments (if those exist) */
	/*    - 0(%ebp) --> %rdi */
	/*    - 4(%ebp) --> %rbp */
.Lsysenter_compat_trace_begin:
	cmpq    $__NR_syscall_max, %rax
	ja      .Lsysenter_compat_trace_xsyscall
	cmpb    $4, x86_syscall_compat_argc(%rax)
	jbe     .Lsysenter_compat_entry
	movzlq  0(%ebp), %rdi
	cmpb    $5, x86_syscall_compat_argc(%rax)
	jbe     .Lsysenter_compat_entry
	movzlq  4(%ebp), %rbp
	jmp     .Lsysenter_compat_entry
.Lsysenter_compat_trace_xsyscall:
	subq    $__NR_xsyscall_min, %rax
	jb      .Lsysenter_compat_entry
	cmpq    $(__NR_xsyscall_max - __NR_xsyscall_min), %rax
	ja      .Lsysenter_compat_trace_except_syscall
	cmpb    $4, x86_xsyscall_compat_argc(%rax)
	jbe     .Lsysenter_compat_entry
	movzlq  0(%ebp), %rdi
	cmpb    $5, x86_xsyscall_compat_argc(%rax)
	jbe     .Lsysenter_compat_entry
	movzlq  4(%ebp), %rbp
	jmp     .Lsysenter_compat_entry
.Lsysenter_compat_trace_except_syscall:
	cmpl    $(0x80000000 - __NR_xsyscall_min), %eax
	jb      .Lsysenter_compat_entry
	addl    $(__NR_xsyscall_min - 0x80000000), %eax
.Lsysenter_compat_entry:

	pushq_cfi_r %rdi
	pushq_cfi_r %rbp
	pushq_cfi_r %rsi
	jmp     .Lirq_80_compat_trace_entry
	.cfi_endproc
SYMEND(sysenter_kernel_entry_trace)


/* Define an exception handler to propagate
 * exceptions from system calls, back into user-space. */
.Lsyscall_except_guard_end = .
X86_DEFINE_SYSCALL_GUARD(
	.Lsyscall_except_guard_start,
	.Lsyscall_except_guard_end,
	X86_INTERRUPT_GUARD_FREG_INT80
)













/* Helper function: Invalid system call */
.section .text.hot
INTERN_ENTRY(x86_bad_syscall)
	.cfi_startproc
	movq  %rdi, %taskseg:TASK_OFFSETOF_SEGMENT+EXCEPTION_INFO_OFFSETOF_DATA+8
	movq  %rsi, %taskseg:TASK_OFFSETOF_SEGMENT+EXCEPTION_INFO_OFFSETOF_DATA+16
	movq  %rdx, %taskseg:TASK_OFFSETOF_SEGMENT+EXCEPTION_INFO_OFFSETOF_DATA+24
	movq  %rcx, %taskseg:TASK_OFFSETOF_SEGMENT+EXCEPTION_INFO_OFFSETOF_DATA+32
	movq  %r8,  %taskseg:TASK_OFFSETOF_SEGMENT+EXCEPTION_INFO_OFFSETOF_DATA+40
	movq  %r9,  %taskseg:TASK_OFFSETOF_SEGMENT+EXCEPTION_INFO_OFFSETOF_DATA+48
#if __EXCEPTION_INFO_NUM_DATA_POINTERS > 7
	movq  $(__EXCEPTION_INFO_NUM_DATA_POINTERS - 7), %rcx
	leaq  EXCEPTION_INFO_OFFSETOF_DATA+56, %rdi
	addq  %taskseg:TASK_SEGMENT_OFFSETOF_SELF, %rdi
	xorq  %rax, %rax
	rep   stosq
#endif
	movq  $(E_UNKNOWN_SYSTEMCALL | (ERR_FNORMAL << 16)),  \
	      %taskseg:TASK_OFFSETOF_SEGMENT+EXCEPTION_INFO_OFFSETOF_CODE

	/* jmp: Might as well throw this at the call-site */
	jmp   error_throw_current
	.cfi_endproc
SYMEND(x86_bad_syscall)





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
	/* TODO */
	int3
1:	cli
	hlt
	jmp 1b
	.cfi_endproc
SYMEND(x86_syscall_exec80_trace)

.section .text
PUBLIC_ENTRY(x86_syscall_exec80_ntrace)
	.cfi_startproc
	/* TODO */
	int3
1:	cli
	hlt
	jmp 1b
	.cfi_endproc
SYMEND(x86_syscall_exec80_ntrace)

















