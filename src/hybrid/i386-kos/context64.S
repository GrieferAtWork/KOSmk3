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
#include <kos/context.h>
#include <asm/universal.h>
#include <asm/cpu-flags.h>
#include "../hybrid.h"
#ifdef __KERNEL__
#include <i386-kos/gdt.h>
#endif

PUBLIC_ENTRY(cpu_getcontext)
INTERN_ENTRY(libc_cpu_getcontext)
	.cfi_startproc
	movq   %r15, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R15(%rdi)
	movq   %r14, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R14(%rdi)
	movq   %r13, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R13(%rdi)
	movq   %r12, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R12(%rdi)
	movq   %r11, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R11(%rdi)
	movq   %r10, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R10(%rdi)
	movq   %r9,  X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R9(%rdi)
	movq   %r8,  X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R8(%rdi)
	movq   %rdi, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RDI(%rdi)
	movq   %rsi, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RSI(%rdi)
	movq   %rbp, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RBP(%rdi)
	movq   %rbx, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RBX(%rdi)
	movq   %rdx, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RDX(%rdi)
	movq   %rcx, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RCX(%rdi)
	movq   $0,   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RAX(%rdi) /* Return 0 the second time */
#ifndef __KERNEL__
	rdgsbaseq  %rsi
	movq   %rsi, X86_CONTEXT64_OFFSETOF_SEGMENTS+X86_SEGMENTS64_OFFSETOF_GSBASE(%rdi)
	rdfsbaseq  %rsi
	movq   %rsi, X86_CONTEXT64_OFFSETOF_SEGMENTS+X86_SEGMENTS64_OFFSETOF_FSBASE(%rdi)
#endif /* !__KERNEL__ */
	popq_cfi %rsi
	.cfi_register %rip, %rsi
	movq   %rsi, X86_CONTEXT64_OFFSETOF_RIP(%rdi)
#ifdef __KERNEL__
	movq   $(X86_SEG_HOST_CS), X86_CONTEXT64_OFFSETOF_CS(%rdi)
#else
	movq   $0,   X86_CONTEXT64_OFFSETOF_CS(%rdi) /* The kernel will fill this in automatically. */
#endif
	pushfq_cfi
	popq_cfi     X86_CONTEXT64_OFFSETOF_RFLAGS(%rdi)
	movq   %rsp, X86_CONTEXT64_OFFSETOF_RSP(%rdi)
#ifdef __KERNEL__
	movq   $(X86_SEG_HOST_SS), X86_CONTEXT64_OFFSETOF_CS(%rdi)
#else
	movq   $0,   X86_CONTEXT64_OFFSETOF_SS(%rdi) /* The kernel will fill this in automatically. */
#endif
	movq   $1,   %rax
	jmpq   *%rsi
	.cfi_endproc
SYMEND(libc_cpu_getcontext)
SYMEND(cpu_getcontext)


PUBLIC_ENTRY(cpu_xchcontext)
INTERN_ENTRY(libc_cpu_xchcontext)
	/* EDI: struct cpu_context const *__restrict new_context */
	/* ESI: struct cpu_context *__restrict old_context */
	.cfi_startproc
	movq   %r15, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R15(%rsi)
	movq   %r14, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R14(%rsi)
	movq   %r13, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R13(%rsi)
	movq   %r12, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R12(%rsi)
	movq   %r11, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R11(%rsi)
	movq   %r10, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R10(%rsi)
	movq   %r9,  X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R9(%rsi)
	movq   %r8,  X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R8(%rsi)
	movq   %rdi, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RDI(%rsi)
	movq   %rsi, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RSI(%rsi)
	movq   %rbp, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RBP(%rsi)
	movq   %rbx, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RBX(%rsi)
	movq   %rdx, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RDX(%rsi)
	movq   %rcx, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RCX(%rsi)
	movq   %rax, X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RAX(%rsi)
#ifndef __KERNEL__
	rdgsbaseq  %rcx
	movq   %rcx, X86_CONTEXT64_OFFSETOF_SEGMENTS+X86_SEGMENTS64_OFFSETOF_GSBASE(%rsi)
	rdfsbaseq  %rcx
	movq   %rcx, X86_CONTEXT64_OFFSETOF_SEGMENTS+X86_SEGMENTS64_OFFSETOF_FSBASE(%rsi)
#endif
	movq   0(%rsp), %rax
	movq   %rax, X86_CONTEXT64_OFFSETOF_RIP(%rsi)
#ifdef __KERNEL__
	movq   $(X86_SEG_HOST_CS), X86_CONTEXT64_OFFSETOF_CS(%rsi)
#else
	movq   $0,   X86_CONTEXT64_OFFSETOF_CS(%rsi) /* The kernel will fill this in automatically. */
#endif
	pushfq_cfi
	popq_cfi     X86_CONTEXT64_OFFSETOF_RFLAGS(%rsi)
	movq   %rsp, X86_CONTEXT64_OFFSETOF_RSP(%rsi)
#ifdef __KERNEL__
	movq   $(X86_SEG_HOST_SS), X86_CONTEXT64_OFFSETOF_CS(%rsi)
#else
	movq   $0,   X86_CONTEXT64_OFFSETOF_SS(%rsi) /* The kernel will fill this in automatically. */
#endif
	.cfi_endproc
	/* Now load the new context. */
PUBLIC_ENTRY(cpu_setcontext)
INTERN_ENTRY(libc_cpu_setcontext)
	/* RDI: struct cpu_context const *__restrict new_context */
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R15(%rdi), %r15
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R14(%rdi), %r14
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R13(%rdi), %r13
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R12(%rdi), %r12
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R11(%rdi), %r11
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R10(%rdi), %r10
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R9(%rdi),  %r9
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R8(%rdi),  %r8
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RSI(%rdi), %rsi
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RBP(%rdi), %rbp
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RBX(%rdi), %rbx
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RDX(%rdi), %rdx
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RCX(%rdi), %rcx
#ifndef __KERNEL__
	movq   X86_CONTEXT64_OFFSETOF_SEGMENTS+X86_SEGMENTS64_OFFSETOF_GSBASE(%rdi), %rax
	wrgsbaseq  %rax
	movq   X86_CONTEXT64_OFFSETOF_SEGMENTS+X86_SEGMENTS64_OFFSETOF_FSBASE(%rdi), %rax
	wrfsbaseq  %rax
#endif /* !__KERNEL__ */
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RAX(%rdi), %rax
#ifdef __KERNEL__
	cli    /* Prevent interrupts while we have an invalid stack loaded */
	leaq   X86_CONTEXT64_OFFSETOF_IRET(%rdi), %rsp
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RDI(%rdi), %rdi
	iretq  /* Return to the context target. */
#else /* __KERNEL__ */
/*	movq   X86_CONTEXT64_OFFSETOF_CS(%rdi), %cs */
/*	movq   X86_CONTEXT64_OFFSETOF_SS(%rdi), %ss */
	pushq  X86_CONTEXT64_OFFSETOF_RFLAGS(%rdi)
	popfq

	/* The only GP-registers still left to be loaded is RDI */
	movq   X86_CONTEXT64_OFFSETOF_RSP(%rdi), %rsp /* Load the new stack. */
	pushq  X86_CONTEXT64_OFFSETOF_RIP(%rdi) /* Push the return address */
	movq   X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RDI(%rdi), %rdi
	ret    /* Return to the context target. */
#endif /* !__KERNEL__ */
SYMEND(libc_cpu_setcontext)
SYMEND(libc_cpu_xchcontext)
SYMEND(cpu_setcontext)
SYMEND(cpu_xchcontext)


#ifdef __KERNEL__
INTERN_ENTRY(cpu_setcontext_pop)
	popq %rdi
	jmp  libc_cpu_setcontext
SYMEND(cpu_setcontext_pop)
#endif

