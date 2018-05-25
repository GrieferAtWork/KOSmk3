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
#include "../hybrid.h"
#include <hybrid/compiler.h>
#include <hybrid/asm.h>
#include <kos/thread.h>
#include <asm/universal.h>
#include <except.h>
#ifdef __KERNEL__
#include <i386-kos/gdt.h>
#endif

.section .text.crt.except

#define e_code       EXCEPTION_INFO_OFFSETOF_CODE
#define e_flag       EXCEPTION_INFO_OFFSETOF_FLAG
#define e_data       EXCEPTION_INFO_OFFSETOF_DATA
#define e_context    EXCEPTION_INFO_OFFSETOF_CONTEXT

#define ts_self      TASK_SEGMENT_OFFSETOF_SELF
#define ts_xcurrent  TASK_SEGMENT_OFFSETOF_XCURRENT

#define c_gpregs     X86_CONTEXT_OFFSETOF_GPREGS
#define c_rsp        X86_CONTEXT_OFFSETOF_RSP
#define c_rip        X86_CONTEXT_OFFSETOF_RIP
#define c_rflags     X86_CONTEXT_OFFSETOF_RFLAGS

#ifdef __KERNEL__
#define __KERNEL_SELECT(tt,ff) tt
#else
#define __KERNEL_SELECT(tt,ff) ff
#endif

INTERN_ENTRY(libc_error_rethrow)
	.cfi_startproc
	/* Construct a new CPU-state for the caller. */
	popq_cfi %rax   /* Load RIP */
	.cfi_register %rip, %rax
.error_rethrow_rax_is_ip:
	pushq_cfi    $(__KERNEL_SELECT(X86_SEG_HOST_SS,0)) /* SS */
	pushq_cfi    %rsp                                  /* RSP */
	addq         $8, 0(%rsp) /* Fix RSP to point to the caller's RSP value. */
	pushfq_cfi_r                                       /* RFLAGS */
	pushq_cfi    $(__KERNEL_SELECT(X86_SEG_HOST_CS,0)) /* CS */
	pushq_cfi    %rax                                  /* RIP */
	.cfi_rel_offset %rip, 0

#ifndef __KERNEL__
	/* Push segment registers. */
	rdfsbaseq    %rax
	pushq_cfi    %rax
	rdgsbaseq    %rax
	pushq_cfi    %rax
#endif /* !__KERNEL__ */

	/* Push general purpose registers. */
#if 0 /* TODO: Introduce a function `__sysv_error_rethrow', that is allowed to optimize for this. */
	push_void_cfi 8 * 3 /* rax, rcx, rdx */
	pushq_cfi_r  %rbx
	pushq_cfi_r  %rbp
	push_void_cfi 8 * 6 /* rsi, rdi, r8, r9, r10, r11 */
	pushq_cfi_r  %r12
	pushq_cfi_r  %r13
	pushq_cfi_r  %r14
	pushq_cfi_r  %r15
#else
	pushq_cfi_r  %rax
	pushq_cfi_r  %rcx
	pushq_cfi_r  %rdx
	pushq_cfi_r  %rbx
	pushq_cfi_r  %rbp
	pushq_cfi_r  %rsi
	pushq_cfi_r  %rdi
	pushq_cfi_r  %r8
	pushq_cfi_r  %r9
	pushq_cfi_r  %r10
	pushq_cfi_r  %r11
	pushq_cfi_r  %r12
	pushq_cfi_r  %r13
	pushq_cfi_r  %r14
	pushq_cfi_r  %r15
#endif
	/* Fix RIP to point into the middle of the
	 * instruction the caller used to get here. */
	decq   X86_CONTEXT_OFFSETOF_RIP(%rsp)
	/* Rethrow the exception. */
	movq   %rsp, %rdi
	call   libc_error_rethrow_at
	.cfi_endproc
SYMEND(libc_error_rethrow)
EXPORT(error_rethrow,libc_error_rethrow)

.cfi_startproc
INTERN_ENTRY(libc_error_throw_resumable_ex)
	movw   $(ERR_FRESUMABLE|ERR_FRESUMEFUNC), e_flag(%rsi)
	jmp    1f
INTERN_ENTRY(libc_error_throw_ex)
	movw   $(ERR_FNORMAL), e_flag(%rsi)
1:	movw   %di, e_code(%rsi)
	/* Copy exception information into TLS data. */
	movq   $(__EXCEPTION_INFO_OFFSETOF_CONTEXT/8), %rcx
	leaq   ts_xcurrent, %rdi
	addq   %taskseg:ts_self, %rdi
#ifdef __KERNEL__
	cld
#endif
	rep    movsq
	jmp    libc_error_throw_current
INTERN_ENTRY(libc_error_throw_resumablef)
	movw   $(ERR_FRESUMABLE|ERR_FRESUMEFUNC), %taskseg:ts_xcurrent+e_flag
	jmp    1f
INTERN_ENTRY(libc_error_throwf)
	movw   $(ERR_FNORMAL), %taskseg:ts_xcurrent+e_flag
1:	movw   %di, %taskseg:ts_xcurrent+e_code
#if 0 /* TODO */
	leaq   8(%rsp), %rdx /* va_list args; */
	DEFINE_INTERN(libc_error_setf)
	call   libc_error_setf
#endif
	jmp    libc_error_throw_current
INTERN_ENTRY(libc_error_throw_resumable)
	movw   $(ERR_FRESUMABLE|ERR_FRESUMEFUNC), %taskseg:ts_xcurrent+e_flag
	jmp    1f
INTERN_ENTRY(libc_error_throw)
	movw   $(ERR_FNORMAL), %taskseg:ts_xcurrent+e_flag
1:	movw   %di, %taskseg:ts_xcurrent+e_code
	/* Save basic registers. */
	movq   %r15, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R15
	movq   %r14, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R14
	movq   %r13, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R13
	movq   %r12, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R12
	movq   %r11, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R11
	movq   %r10, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R10
	movq   %r9,  %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R9
	movq   %r8,  %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R8
	movq   %rdi, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RDI
	movq   %rsi, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RSI
	movq   %rbp, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RBP
	movq   %rbx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RBX
	movq   %rdx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RDX
	movq   %rcx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RCX
	movq   %rax, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RAX
	pushfq_cfi
	popq_cfi     %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_RFLAGS
	/* Clear extended exception informations. */
	movq   $(ts_xcurrent+e_data), %rdi
	addq   %taskseg:ts_self, %rdi
	movq   $(EXCEPTION_INFO_NUM_DATA_POINTERS), %rcx
	xorq   %rax, %rax
#ifdef __KERNEL__
	cld
#endif
	rep    stosq
	jmp    1f
INTERN_ENTRY(libc_error_throw_current)
	/* Save the current CPU context. */
	movq   %r15, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R15
	movq   %r14, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R14
	movq   %r13, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R13
	movq   %r12, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R12
	movq   %r11, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R11
	movq   %r10, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R10
	movq   %r9,  %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R9
	movq   %r8,  %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_R8
	movq   %rdi, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RDI
	movq   %rsi, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RSI
	movq   %rbp, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RBP
	movq   %rbx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RBX
	movq   %rdx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RDX
	movq   %rcx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RCX
	movq   %rax, %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_GPREGS+X86_GPREGS64_OFFSETOF_RAX
	pushfq_cfi
	popq_cfi     %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_RFLAGS
1:	popq_cfi %rax  /* Return address. */
	.cfi_register %rip, %rax
	movq   %rax, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_RIP
	movq   %rsp, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_RSP
#ifndef __KERNEL__
	rdgsbase %rcx
	movq   %rcx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS64_OFFSETOF_GSBASE
	rdfsbase %rcx
	movq   %rcx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS64_OFFSETOF_FSBASE
#endif
#ifdef __KERNEL__
	movq   $(X86_SEG_HOST_CS), %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_CS
	movq   $(X86_SEG_HOST_SS), %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_SS
#else
	movq   $0,   %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_CS
	movq   $0,   %taskseg:ts_xcurrent+e_context+X86_CONTEXT64_OFFSETOF_SS
#endif
	jmp    .error_rethrow_rax_is_ip
SYMEND(libc_error_throw_current)
SYMEND(libc_error_throw)
SYMEND(libc_error_throw_resumable)
SYMEND(libc_error_throwf)
SYMEND(libc_error_throw_resumablef)
SYMEND(libc_error_throw_ex)
SYMEND(libc_error_throw_resumable_ex)
.cfi_endproc

EXPORT(error_throw_resumable_ex,libc_error_throw_resumable_ex)
EXPORT(error_throw_ex,          libc_error_throw_ex)
EXPORT(error_throw_resumable,   libc_error_throw_resumable)
EXPORT(error_throw,             libc_error_throw)
EXPORT(error_throw_resumablef,  libc_error_throw_resumablef)
EXPORT(error_throwf,            libc_error_throwf)
EXPORT(error_throw_current,     libc_error_throw_current)


.cfi_startproc

INTERN_ENTRY(libc_error_except)
	cmpq   $0, %rdi
	jl     1f                       /* EXCEPT_CONTINUE_RETRY or EXCEPT_CONTINUE_IGNORE */
	je     libc_error_throw_current /* EXCEPT_CONTINUE_SEARCH */
	ret                             /* EXCEPT_EXECUTE_HANDLER */
1:	cmpq   $(EXCEPT_CONTINUE_RETRY), %rdi
	sete   %al       /* AL = mode == EXCEPT_CONTINUE_RETRY */
	movzbq %al, %rdi
INTERN_ENTRY(libc_error_continue)
	/* Forward the call to a C version that does most of the work. */
	call   private_error_continue
	/* Throw the non-continuable error constructed by `private_error_continue()'. */
	jmp    libc_error_throw_current
SYMEND(libc_error_continue)
SYMEND(libc_error_except)
EXPORT(error_continue,libc_error_continue)
EXPORT(error_except,libc_error_except)

INTERN_ENTRY(libc_error_code)
	movzwq %taskseg:ts_xcurrent+e_code, %rax
	ret
SYMEND(libc_error_code)
EXPORT(error_code,libc_error_code)

INTERN_ENTRY(libc_error_info)
	movq $(ts_xcurrent),   %rax
	addq %taskseg:ts_self, %rax
	ret
SYMEND(libc_error_info)
EXPORT(error_info,libc_error_info)


#if 0
INTERN_ENTRY(libc_error_handled)
	ret
SYMEND(libc_error_handled)
EXPORT(error_handled,libc_error_handled)

INTERN_ENTRY(libc_error_dealloc_continue)
	xorq %rax, %rax
	ret
SYMEND(libc_error_dealloc_continue)
EXPORT(error_dealloc_continue,libc_error_dealloc_continue)
#else

INTERN_ENTRY(libc_error_handled)
	.cfi_def_cfa %rsp, 8
	popq_cfi %rdx
	.cfi_register %rip, %rdx
	cmpq     %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_RSP, %rsp
	jne      1f
	movq     %taskseg:ts_xcurrent+EXCEPTION_INFO_OFFSETOF_RTDATA+__EXCEPTION_RT_DATA_OFFSETOF_FREE_SP, %rsp
1:	jmpq     *%rdx
SYMEND(libc_error_handled)
EXPORT(error_handled,libc_error_handled)

INTERN_ENTRY(libc_error_dealloc_continue)
	popq_cfi %rdx
	.cfi_register %rip, %rdx
	cmpq     %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_RSP, %rsp
	jne      1f
	movq     %taskseg:ts_xcurrent+EXCEPTION_INFO_OFFSETOF_RTDATA+__EXCEPTION_RT_DATA_OFFSETOF_FREE_SP, %rcx
	.cfi_def_cfa %rcx, 0
	movq     %rcx, %rax
	subq     %rsp, %rax
	movq     %rcx, %rsp
	jmpq     *%rdx
1:	.cfi_def_cfa %rsp, 0
	jmpq     *%rdx
SYMEND(libc_error_dealloc_continue)
EXPORT(error_dealloc_continue,libc_error_dealloc_continue)
#endif


.cfi_endproc
