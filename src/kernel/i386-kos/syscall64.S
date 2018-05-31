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
#define __XSYSCALL(id,sym)     define_syscall_entry id, sym, x86_bad_syscall_except;
#include <asm/syscallno.ci>
	.rept (__NR_xsyscall_max+1) - .last_sysno
		.quad x86_bad_syscall_except
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
#define __XSYSCALL(id,sym)     define_syscall_entry id, sym ## _compat, x86_bad_syscall_except;
#include <asm/syscallno.ci>
	.rept (__NR_xsyscall_max+1) - .last_sysno
		.quad x86_bad_syscall_except
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

.section .rodata
PUBLIC_OBJECT(x86_syscall_compat_restart)
.last_sysno = __NR_syscall_min
#define __SYSCALL(id,sym)     define_syscall_restart id, sym ## _compat;
#include <asm/syscallno.ci>
	.rept (__NR_syscall_max+1) - .last_sysno
		.byte X86_SYSCALL_RESTART_FAUTO
	.endr
SYMEND(x86_syscall_compat_restart)

.section .rodata
PUBLIC_OBJECT(x86_xsyscall_compat_restart)
.last_sysno = __NR_xsyscall_min
#define __XSYSCALL(id,sym)     define_syscall_restart id, sym ## _compat;
#include <asm/syscallno.ci>
	.rept (__NR_xsyscall_max+1) - .last_sysno
		.byte X86_SYSCALL_RESTART_FAUTO
	.endr
SYMEND(x86_xsyscall_compat_restart)



/* Weakly redirect all unimplemented system calls. */
#if X86_SYSCALL_RESTART_FAUTO != 0
#define __SET_RESTART(x)  restart_##x = X86_SYSCALL_RESTART_FAUTO;
#else
#define __SET_RESTART(x)  /* nothing */
#endif

#define __SYSCALL(id,sym) \
	.hidden sym, argc_##sym, restart_##sym, sym##_compat, argc_##sym##_compat, restart_##sym##_compat; \
	.global sym, argc_##sym, restart_##sym, sym##_compat, argc_##sym##_compat, restart_##sym##_compat; \
	.weak sym, argc_##sym, restart_##sym, sym##_compat, argc_##sym##_compat, restart_##sym##_compat; \
	sym##_compat = sym; \
	argc_##sym##_compat = argc_##sym; \
	restart_##sym##_compat = restart_##sym; \
	sym = x86_bad_syscall; \
	argc_##sym = 6; \
	__SET_RESTART(sym)
#define __XSYSCALL(id,sym)     __SYSCALL(id,sym)
#include <asm/syscallno.ci>



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
	irq_enter
	/* TODO */
	int3
1:	cli
	hlt
	jmp 1b
	irq_leave
	.cfi_endproc
SYMEND(irq_80)


.section .text.hot
INTERN_ENTRY(sysenter_kernel_entry_trace)
	.cfi_startproc simple
	.cfi_signal_frame
	/* TODO */
	int3
1:	cli
	hlt
	jmp 1b
	.cfi_endproc
SYMEND(sysenter_kernel_entry_trace)

.section .text.hot
INTERN_ENTRY(irq_80_trace)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %rflags, -3*8
	.cfi_offset %rip,    -5*8
	irq_enter
	/* TODO */
	int3
1:	cli
	hlt
	jmp 1b
	irq_leave
	.cfi_endproc
SYMEND(irq_80_trace)


.section .text.hot
INTERN_ENTRY(x86_bad_syscall)
	.cfi_startproc
	movq    $(-ENOSYS), %rax
	ret
	.cfi_endproc
SYMEND(x86_bad_syscall)

.section .text.hot
INTERN_ENTRY(x86_bad_syscall_except)
	.cfi_startproc
	/* TODO */
	int3
1:	cli
	hlt
	jmp 1b
	.cfi_endproc
SYMEND(x86_bad_syscall_except)


/* This must be added to the return EIP if a system call wants to return 64 bits. */
INTERN_CONST(x86_syscall64_adjustment,0)
INTERN_CONST(x86_syscall64_adjustment_compat,0)


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

















