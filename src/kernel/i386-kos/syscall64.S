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
INTERN_ENTRY(sysenter_kernel_entry)
	/* TODO */
	int3
1:	cli
	hlt
	jmp 1b
SYMEND(sysenter_kernel_entry)

.section .text.hot
INTERN_ENTRY(irq_80)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
	irq_enter_syscall
	/* TODO */
	int3
1:	cli
	hlt
	jmp 1b
	irq_leave_syscall
	.cfi_endproc
SYMEND(irq_80)

.section .text.hot
INTERN_ENTRY(sysenter_kernel_entry_trace)
//1:	cli; hlt; jmp 1b
	movq     0(%rsp), %rsp
	/* Construct an IRET tail. */
	pushq    $(X86_USER_DS) /* %ss */
	pushq    %rbp           /* %rsp */
	pushfq
	orq      $(EFLAGS_IF), (%rsp)
	pushq    $(X86_USER_CS) /* %cs */
	pushq    %rdi           /* %rip */

	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8

	/* TODO */
	int3
1:	cli
	hlt
	jmp 1b
	.cfi_endproc
SYMEND(sysenter_kernel_entry_trace)


.section .text.hot
.irq_80_trace_start = .
INTERN_ENTRY(irq_80_trace)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
	irq_enter_syscall

	/* Check for compatibility mode. */
	cmpq    $X86_SEG_USER_CS32, X86_IRREGS64_OFFSETOF_CS(%rsp)
	je      irq_80_compat_trace

	/* Save callee-clobber registers. */
	pushq_cfi_r %rax  /* System call number. */
	pushq_cfi_r %rdx  /* Arg #2 */
	pushq_cfi_r %r11
	pushq_cfi_r %r10  /* Arg #3 */
	pushq_cfi_r %r9   /* Arg #5 */
	pushq_cfi_r %r8   /* Arg #4 */
	pushq_cfi_r %rdi  /* Arg #0 */
	pushq_cfi_r %rsi  /* Arg #1 */
	pushq_cfi_r %rcx

	/* Trace this system call. */
	movq    %rsp, %rdi
	call    syscall_trace

	/* Invoke the system call for real! */
	movq    8 * 8(%rsp), %rax
	movq    2 * 8(%rsp), %rdi
	movq    1 * 8(%rsp), %rsi
	movq    7 * 8(%rsp), %rdx
	movq    5 * 8(%rsp), %rcx
	movq    3 * 8(%rsp), %r8
	movq    4 * 8(%rsp), %r9

.Lirq_80_trace_begin:
	cmpq    $__NR_syscall_max, %rax
	ja      .Lirq_80_trace_xsyscall
	callq   *x86_syscall_router(,%rax,8)
.Lirq_80_trace_return:
	.cfi_remember_state
	popq_cfi_r  %rcx
	popq_cfi_r  %rsi
	popq_cfi_r  %rdi
	popq_cfi_r  %r8
	popq_cfi_r  %r9
	popq_cfi_r  %r10
	popq_cfi_r  %r11
	popq_cfi_r  %rdx
	popq_void_cfi
	irq_leave_syscall
	.cfi_restore_state
.Lirq_80_trace_return64:
	.cfi_remember_state
	popq_cfi_r  %rcx
	popq_cfi_r  %rsi
	popq_cfi_r  %rdi
	popq_cfi_r  %r8
	popq_cfi_r  %r9
	popq_cfi_r  %r10
	popq_cfi_r  %r11
	pop_void_cfi 2*8
	irq_leave_syscall
	.cfi_restore_state
.Lirq_80_trace_xsyscall:
	subq    $__NR_xsyscall_min, %rax
	jb      .Lirq_80_trace_bad_syscall
	cmpq    $(__NR_xsyscall_max - __NR_xsyscall_min), %rax
	ja      .Lirq_80_trace_except_syscall
	pushq_cfi $.Lirq_80_trace_return
	jmpq    *x86_xsyscall_router(,%rax,8)
	.cfi_adjust_cfa_offset -8
.Lirq_80_trace_except_syscall:
	cmpl    $(0x80000000 - __NR_xsyscall_min), %eax
	jb      .Lirq_80_trace_bad_syscall
	addl    $(__NR_xsyscall_min - 0x80000000), %eax
	jmp     .Lirq_80_trace_begin
.Lirq_80_trace_bad_syscall:
	addq    $__NR_xsyscall_min, %rax
	call     x86_bad_syscall
	.cfi_endproc
SYMEND(irq_80_trace)

/* This must be added to the return RIP if a system call wants to return 64 bits. */
INTERN_CONST(x86_syscall64_adjustment,.Lirq_80_trace_return64 - .Lirq_80_trace_return)

INTERN_ENTRY(irq_80_compat_trace)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
	pushq_cfi_r %rax  /* System call number. */
	pushq_cfi_r %rdx  /* Arg #2 */
	pushq_void_cfi
	pushq_cfi_r %rsi  /* Arg #3 */
	pushq_cfi_r %rbp  /* Arg #5 */
	pushq_cfi_r %rdi  /* Arg #4 */
	pushq_cfi_r %rbx  /* Arg #0 */
	pushq_cfi_r %rcx  /* Arg #1 */
	pushq_void_cfi
	movq    %rsp, %rdi

	/* Trace this system call. */
	call    syscall_trace

	/* Invoke the system call for real! */
	movq    8 * 8(%rsp), %rax
	movq    2 * 8(%rsp), %rdi
	movq    1 * 8(%rsp), %rsi
	movq    7 * 8(%rsp), %rdx
	movq    5 * 8(%rsp), %rcx
	movq    3 * 8(%rsp), %r8
	movq    4 * 8(%rsp), %r9
	popq_void_cfi

.Lirq_80_compat_trace_begin:
	cmpq    $__NR_syscall_max, %rax
	ja      .Lirq_80_compat_trace_xsyscall
	callq   *x86_syscall_compat_router(,%rax,8)
.Lirq_80_compat_trace_return:
	.cfi_remember_state
	popq_cfi_r %rcx
	popq_cfi_r %rbx
	popq_cfi_r %rdi
	popq_cfi_r %rbp
	popq_cfi_r %rsi
	popq_void_cfi
	popq_cfi_r %rdx
	popq_void_cfi
	irq_leave_syscall
	.cfi_restore_state
	.space x86_syscall64_adjustment - (. - .Lirq_80_compat_trace_return)
.Lirq_80_compat_trace_return64:
	.cfi_remember_state
	popq_cfi_r %rcx
	popq_cfi_r %rbx
	popq_cfi_r %rdi
	popq_cfi_r %rbp
	popq_cfi_r %rsi
	pop_void_cfi 3*8
	irq_leave_syscall
	.cfi_restore_state
.Lirq_80_compat_trace_xsyscall:
	subq    $__NR_xsyscall_min, %rax
	jb      .Lirq_80_compat_trace_bad_syscall
	cmpq    $(__NR_xsyscall_max - __NR_xsyscall_min), %rax
	ja      .Lirq_80_compat_trace_except_syscall
	pushq_cfi $.Lirq_80_compat_trace_return
	jmpq    *x86_xsyscall_compat_router(,%rax,8)
	.cfi_adjust_cfa_offset -8
.Lirq_80_compat_trace_except_syscall:
	cmpl    $(0x80000000 - __NR_xsyscall_min), %eax
	jb      .Lirq_80_compat_trace_bad_syscall
	addl    $(__NR_xsyscall_min - 0x80000000), %eax
	jmp     .Lirq_80_compat_trace_begin
.Lirq_80_compat_trace_bad_syscall:
	addq    $__NR_xsyscall_min, %rax
	call     x86_bad_syscall
	.cfi_endproc
SYMEND(irq_80_compat_trace)

.irq_80_trace_end = .
X86_DEFINE_SYSCALL_GUARD(
	.irq_80_trace_start,
	.irq_80_trace_end
)


.section .text.hot
INTERN_ENTRY(x86_bad_syscall)
	.cfi_startproc
	/* TODO */
	int3
1:	cli
	hlt
	jmp 1b
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

















