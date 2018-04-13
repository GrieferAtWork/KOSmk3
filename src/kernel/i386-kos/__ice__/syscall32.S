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
#include <hybrid/types.h>
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
#include <asm/syscallno.ci>



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


.section .text.hot
INTERN_ENTRY(irq_80)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4

#ifdef CONFIG_X86_SEGMENTATION
	pushl_cfi_r  %ds
	pushl_cfi_r  %es
	pushl_cfi_r  %fs
	pushl_cfi_r  %gs
#endif

	pushl_cfi_r X86_SYSCALL_REG5
	pushl_cfi_r X86_SYSCALL_REG4
	pushl_cfi_r X86_SYSCALL_REG3
	pushl_cfi_r X86_SYSCALL_REG2
	pushl_cfi_r X86_SYSCALL_REG1
	pushl_cfi_r X86_SYSCALL_REG0

	pushl_cfi_r %eax
	call    x86_load_segments
	popl_cfi_r %eax

	.cfi_remember_state
	cmpl    $(__NR_syscall_max), %eax
	ja      60f
	call   *x86_syscall_router(,%eax,4)

1:	popl_cfi_r  X86_SYSCALL_REG0
	popl_cfi_r  X86_SYSCALL_REG1
	popl_cfi_r  X86_SYSCALL_REG2
	popl_cfi_r  X86_SYSCALL_REG3
	popl_cfi_r  X86_SYSCALL_REG4
	popl_cfi_r  X86_SYSCALL_REG5

#ifdef CONFIG_X86_SEGMENTATION
44:	popl_cfi_r  %gs
	popl_cfi_r  %fs
	popl_cfi_r  %es
	popl_cfi_r  %ds
#endif /* CONFIG_X86_SEGMENTATION */

	iret
	.cfi_restore_state
60:	cmpl    $(__NR_xsyscall_min), %eax
	jb      99f
	cmpl    $(__NR_xsyscall_max), %eax
	ja      99f
	/* Extended system call. */
	pushl_cfi $1b
	jmpl    *(x86_xsyscall_router - (__NR_xsyscall_min*4) & 0xffffffff)(,%eax,4)
	.cfi_adjust_cfa_offset -4
	.hidden x86_syscall_return64
	.global x86_syscall_return64
x86_syscall_return64:
	.cfi_remember_state
	popl_cfi_r  X86_SYSCALL_REG0
	popl_cfi_r  X86_SYSCALL_REG1
	addl    $4, %esp
	.cfi_undefined %edx /* Don't override the high 32 bits of the return value. */
	popl_cfi_r  X86_SYSCALL_REG3
	popl_cfi_r  X86_SYSCALL_REG4
	popl_cfi_r  X86_SYSCALL_REG5
#ifdef CONFIG_X86_SEGMENTATION
	jmp     44b
#else
	iret
#endif

	.cfi_restore_state
99:	call    x86_bad_syscall
	jmp     1b
	.cfi_endproc
.irq_80_end = .
SYMEND(irq_80)
X86_DEFINE_SYSCALL_GUARD(irq_80,.irq_80_end)

INTERN_ENTRY(x86_bad_syscall)
	.cfi_startproc
	movl    $(-ENOSYS), %eax
	ret
	.cfi_endproc
SYMEND(x86_bad_syscall)


/* Weakly redirect all unimplemented system calls. */
#ifdef CONFIG_NO_X86_SYSENTER
#define __SYSCALL(id,sym) \
	.hidden sym; \
	.global sym; \
	.weak sym; \
	sym = x86_bad_syscall;
#else
#define __SYSCALL(id,sym) \
	.hidden sym, argc_##sym; \
	.global sym, argc_##sym; \
	.weak sym, argc_##sym; \
	sym = x86_bad_syscall; \
	argc_##sym = 6;
#endif
#define __SYSCALL_ASM(id,sym)  __SYSCALL(id,sym)
#define __XSYSCALL(id,sym)     __SYSCALL(id,sym)
#define __XSYSCALL_ASM(id,sym) __SYSCALL(id,sym)
#include <asm/syscallno.ci>





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

#ifdef CONFIG_X86_SEGMENTATION
	pushl_cfi_r  %ds
	pushl_cfi_r  %es
	pushl_cfi_r  %fs
	pushl_cfi_r  %gs
#endif

	/* Push system call registers. */
	pushl_cfi_r X86_SYSCALL_REG5
	pushl_cfi_r X86_SYSCALL_REG4
	pushl_cfi_r X86_SYSCALL_REG3
	pushl_cfi_r X86_SYSCALL_REG2
	pushl_cfi_r X86_SYSCALL_REG1
	pushl_cfi_r X86_SYSCALL_REG0

	/* Load segments. */
	pushl_cfi_r %eax
	call    x86_load_segments
	popl_cfi_r %eax

	/* Do the system call */
	cmpl    $(__NR_syscall_max), %eax
	ja      60f
	.cfi_remember_state

	/* Load additional arguments from user-space. */
	cmpl    $5, x86_syscall_argc(,%eax,1)
	jb      7f
	cmpl    $(KERNEL_BASE), %ebp
	jae     .sysenter_bad_args
	movl    0(%ebp), %ecx /* ARG#4 */
	movl    %ecx, 16(%esp)
	cmpl    $6, x86_syscall_argc(,%eax,1)
	jb      7f
	movl    4(%ebp), %ecx /* ARG#5 */
	movl    %ecx, 20(%esp)

7:	call   *x86_syscall_router(,%eax,4)

1:	popl_cfi_r  X86_SYSCALL_REG0
	popl_cfi_r  X86_SYSCALL_REG1
	popl_cfi_r  X86_SYSCALL_REG2
	popl_cfi_r  X86_SYSCALL_REG3
	popl_cfi_r  X86_SYSCALL_REG4
	popl_cfi_r  X86_SYSCALL_REG5

#ifdef CONFIG_X86_SEGMENTATION
	popl_cfi_r  %gs
	popl_cfi_r  %fs
	popl_cfi_r  %es
	popl_cfi_r  %ds
#endif /* CONFIG_X86_SEGMENTATION */

	cli
	cmpl    $(X86_USER_CS), X86_IRREGS_USER32_OFFSETOF_CS(%esp)
	jne     6f /* Custom return location (just use IRET) */
	/* Use `sysexit' */
	movl    X86_IRREGS_USER32_OFFSETOF_USERESP(%esp), %ecx
	movl    X86_IRREGS_USER32_OFFSETOF_EIP(%esp), %edx
	addl    $8, %esp
	.cfi_adjust_cfa_offset -8
	/* Prevent popfl from enabling interrupts to bypass a race condition
	 * that could result in an interrupt attempting to re-direct user-space
	 * at a time where registers it would modify were already loaded.
	 * This race condition doesn't happen with IRET, because it executes
	 * atomically (or rather: without interrupts). */
	andl    $(~EFLAGS_IF), (%esp)
	popfl
	/* Enable interrupts in a way that delays its execution for 1
	 * additional instruction, meaning that no interrupts can occurr
	 * before `sysexit' actually returns to user-space.
	 * NOTE: `popfl' doesn't have this effect, so this clutch is required. */
	sti
	sysexit
6:	iret

	.cfi_restore_state
60:	cmpl    $(__NR_xsyscall_min), %eax
	jb      99f
	cmpl    $(__NR_xsyscall_max), %eax
	ja      99f
	/* Extended system call. */

	/* Load additional arguments from user-space. */
	cmpl    $5, (x86_xsyscall_argc - (__NR_xsyscall_min*1) & 0xffffffff)(,%eax,1)
	jb      7f
	cmpl    $(KERNEL_BASE), %ebp
	.cfi_remember_state
	jae     .sysenter_bad_args
	movl    0(%ebp), %ecx /* ARG#4 */
	movl    %ecx, 16(%esp)
	cmpl    $6, (x86_xsyscall_argc - (__NR_xsyscall_min*1) & 0xffffffff)(,%eax,1)
	jb      7f
	movl    4(%ebp), %ecx /* ARG#5 */
	movl    %ecx, 20(%esp)

7:	pushl_cfi $1b
	jmpl    *(x86_xsyscall_router - (__NR_xsyscall_min*4) & 0xffffffff)(,%eax,4)
	.cfi_adjust_cfa_offset -4

99:	call    x86_bad_syscall
	jmp     1b
.sysenter_bad_args:
	.cfi_restore_state
	.global throw_segfault
	.hidden throw_segfault
	pushl_cfi $0   /* reason */
	pushl_cfi %ebp /* addr */
	call    throw_segfault
	.cfi_endproc
.sysenter_kernel_entry_end = .
SYMEND(sysenter_kernel_entry)
X86_DEFINE_SYSCALL_GUARD(sysenter_kernel_entry,.sysenter_kernel_entry_end)

#endif /* !CONFIG_NO_X86_SYSENTER */













