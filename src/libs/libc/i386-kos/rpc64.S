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
#include "../libc.h"
#include <hybrid/compiler.h>
#include <hybrid/asm.h>
#include <asm/universal.h>
#include <kos/rpc.h>
#include <kos/thread.h>
#include <errno.h>
#include <except.h>


.section .text.crt.rpc
.cfi_startproc
INTERN_ENTRY(libc_rpc_pushoff)
	movzlq %taskseg:USER_TASK_SEGMENT_OFFSETOF_NO_RPC, %rcx
	incq   %rcx
	movl   %ecx, %taskseg:USER_TASK_SEGMENT_OFFSETOF_NO_RPC
	xorq   %rax, %rax
	cmpq   $1, %rcx
	sete   %al
	jne    1f
	movq   $1, %rdi
	jmp    Xsys_xnosignal
1:	ret
SYMEND(libc_rpc_pushoff)
EXPORT(rpc_pushoff,libc_rpc_pushoff)

INTERN_ENTRY(libc_rpc_pop)
	xorq   %rax, %rax
	decq   %taskseg:USER_TASK_SEGMENT_OFFSETOF_NO_RPC
	setz   %al
	jnz    1f
	movq   $0, %rdi
	jmp    Xsys_xnosignal
1:	ret
SYMEND(libc_rpc_pop)
EXPORT(rpc_pop,libc_rpc_pop)
.cfi_endproc


.section .text.crt.rpc
	/* RSP:
	 *        [+0]   u64  jf_mode
	 *        [+8]   u64  jf_r11
	 *        [+16]  u64  jf_r10
	 *        [+24]  u64  jf_r9
	 *        [+32]  u64  jf_r8
	 *        [+40]  u64  jf_rdi
	 *        [+48]  u64  jf_rsi
	 *        [+56]  u64  jf_rdx
	 *        [+64]  u64  jf_rcx
	 *        [+72]  u64  jf_rax
	 *        [+80]  u64  jf_rip
	 *        [+88]  u64  jf_rflags
	 */
	.cfi_startproc simple
	.cfi_rel_offset %r11, 8
	.cfi_rel_offset %r10, 16
	.cfi_rel_offset %r9,  24
	.cfi_rel_offset %r8,  32
	.cfi_rel_offset %rdi, 40
	.cfi_rel_offset %rsi, 48
	.cfi_rel_offset %rdx, 56
	.cfi_rel_offset %rcx, 64
	.cfi_rel_offset %rax, 72
	.cfi_rel_offset %rip, 80
	.cfi_rel_offset %rflags, 88
	nop /* This counteracts the -1 adjustment done by tracebacks. */
	.align 16
INTERN_ENTRY(x86_rpc_entry_fast)
	/* RDI: rpc_t func */
	/* RSI: void *arg */
	/* Invoke the RPC function. */
	movq    %rsp, %rdx /* `struct fast_rpc_regs *__restrict regs' */
DEFINE_INTERN(libc_invoke_rpc_fast)
	call    libc_invoke_rpc_fast

	/* If the RPC didn't interrupt a system call, always resume execution. */
	testq   $(RPC_REASON_SYSCALL), 0(%rsp)
	jz      .fast_rpc_resume /* Wasn't a system call -> Just resume */
	cmpq    $(RPC_RETURN_RESUME), %rax
	je      .fast_rpc_resume /* The RPC explicitly wants us to resume -> Resume */
	testq   $(RPC_REASON_FMUST_RESTART), 0(%rsp)
	jnz     .fast_rpc_restart /* The system call must be restarted. */
	cmpq    $(RPC_RETURN_RESTART), %rax
	je      .fast_rpc_maybe_restart /* The system call can be restarted. */
	cmpq    $(RPC_RETURN_INTERRUPT), %rax
	je      .fast_rpc_interrupt /* The RPC wants us to signal an interrupt. */
	/* DEFAULT: RPC_RETURN_FORCE_RESTART */

.fast_rpc_restart:
	.cfi_remember_state

	/* Exchange jf_eip and jf_eflags, and repeat the last instruction. */
	movq    80(%rsp), %rdi
	call    libc_prev_instruction
	/* XXX: What if EAX is NULL now? (I mean, it shouldn't, but what if?) */
	xchgq   88(%rsp), %rax
	.cfi_rel_offset %rip,    88
	.cfi_register   %rflags, %rax
	movq    %rax, 80(%esp)
	.cfi_rel_offset %rflags, 80

	/* Load the saved context (see above) */
	popq_void_cfi    /* jf_mode */
	popq_cfi_r %r11
	popq_cfi_r %r10
	popq_cfi_r %r9
	popq_cfi_r %r8
	popq_cfi_r %rdi
	popq_cfi_r %rsi
	popq_cfi_r %rdx
	popq_cfi_r %rcx
	popq_cfi_r %rax
	popfq_cfi_r
	ret
	.cfi_restore_state
.fast_rpc_resume:
	.cfi_remember_state
	/* Exchange jf_eip and jf_eflags */
	movq    80(%rsp), %rdi
	xchgq   88(%rsp), %rdi
	.cfi_rel_offset %rip,    88
	.cfi_register   %rflags, %rdi
	movq    %rdi, 80(%esp)
	.cfi_rel_offset %rflags, 80

	/* Load the saved context (see above) */
	popq_void_cfi    /* jf_mode */
	popq_cfi_r %r11
	popq_cfi_r %r10
	popq_cfi_r %r9
	popq_cfi_r %r8
	popq_cfi_r %rdi
	popq_cfi_r %rsi
	popq_cfi_r %rdx
	popq_cfi_r %rcx
	popq_cfi_r %rax
	popfq_cfi_r
	ret

	.cfi_restore_state
.fast_rpc_maybe_restart:
	testq   $(RPC_REASON_FSHOULD_RESTART), 0(%rsp) /* jf_mode */
	jnz     .fast_rpc_restart
.fast_rpc_interrupt:
	testq   $(RPC_REASON_NOEXCEPT), 0(%rsp) /* jf_mode */
	jnz     1f
	movq    $(E_INTERRUPT), %rdi
	call    libc_error_throw

1:	/* Emulate the system call returning -EINTR */
	movq    $-EINTR, %rax
	jmp     .fast_rpc_resume
	.cfi_endproc
SYMEND(x86_rpc_entry_fast)














.section .text.crt.rpc
	/* ESP:
	 *        [+0]   u64  jf_mode
	 *        [+8]   u64  jf_r15
	 *        [+16]  u64  jf_r14
	 *        [+24]  u64  jf_r13
	 *        [+32]  u64  jf_r12
	 *        [+40]  u64  jf_r11
	 *        [+48]  u64  jf_r10
	 *        [+56]  u64  jf_r9
	 *        [+64]  u64  jf_r8
	 *        [+72]  u64  jf_rdi
	 *        [+80]  u64  jf_rsi
	 *        [+88]  u64  jf_rbp
	 *        [+96]  u64  jf_rbx
	 *        [+104] u64  jf_rdx
	 *        [+112] u64  jf_rcx
	 *        [+120] u64  jf_rax
	 *        [+128] u64  jf_gsbase
	 *        [+136] u64  jf_fsbase
	 *        [+144] u64  jf_rip
	 *        [+152] u64  jf_cs
	 *        [+160] u64  jf_rflags
	 *        [+168] u64  jf_rsp
	 *        [+176] u64  jf_ss
	 */
	.cfi_startproc simple
	.cfi_def_cfa    %rsp,    184
	.cfi_rel_offset %r15, 8
	.cfi_rel_offset %r14, 16
	.cfi_rel_offset %r13, 24
	.cfi_rel_offset %r12, 32
	.cfi_rel_offset %r11, 40
	.cfi_rel_offset %r10, 48
	.cfi_rel_offset %r9,  56
	.cfi_rel_offset %r8,  64
	.cfi_rel_offset %rdi, 72
	.cfi_rel_offset %rsi, 80
	.cfi_rel_offset %rbp, 88
	.cfi_rel_offset %rbx, 96
	.cfi_rel_offset %rdx, 104
	.cfi_rel_offset %rcx, 112
	.cfi_rel_offset %rax, 120
/*	.cfi_rel_offset %gsbase, 128 */
/*	.cfi_rel_offset %fsbase, 136 */
	.cfi_rel_offset %rip, 144
/*	.cfi_rel_offset %cs,  152 */
	.cfi_rel_offset %rflags, 160
/*	.cfi_rel_offset %rsp, 168 */
/*	.cfi_rel_offset %ss,  176 */
	nop /* This counteracts the -1 adjustment done by tracebacks. */
	.align 16
INTERN_ENTRY(x86_rpc_entry_full)
	/* ECX: rpc_t func */
	/* EDX: void *arg */
	/* Invoke the RPC function. */
	movq    %rsp, %rdx /* `struct full_rpc_regs *__restrict regs' */
DEFINE_INTERN(libc_invoke_rpc_full)
	call    libc_invoke_rpc_full

	/* If the RPC didn't interrupt a system call, always resume execution. */
	testq   $(RPC_REASON_SYSCALL), 0(%rsp)
	jz      .full_rpc_resume /* Wasn't a system call -> Just resume */
	cmpq    $(RPC_RETURN_RESUME), %rax
	je      .full_rpc_resume /* The RPC explicitly wants us to resume -> Resume */
	testq   $(RPC_REASON_FMUST_RESTART), 0(%rsp)
	jnz     .full_rpc_restart /* The system call must be restarted. */
	cmpq    $(RPC_RETURN_RESTART), %rax
	je      .full_rpc_maybe_restart /* The system call can be restarted. */
	cmpq    $(RPC_RETURN_INTERRUPT), %rax
	je      .full_rpc_interrupt /* The RPC wants us to signal an interrupt. */
	/* DEFAULT: RPC_RETURN_FORCE_RESTART */

	.cfi_remember_state
.full_rpc_restart:
	/* Write jf_eip to jf_ss, and repeat the last instruction. */
	movq    144(%rsp), %rdi
	call    libc_prev_instruction
	/* XXX: What if EAX is NULL now? (I mean, it shouldn't, but what if?) */
	.cfi_register %rip, %rax
.full_rpc_load_context_rax_is_rip:
	/* Load the saved context (see above) */
	popq_void_cfi   /* jf_mode */
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
	/* ESP:
	 *        [+0]  u64  jf_rcx
	 *        [+8]  u64  jf_rax
	 *        [+16] u64  jf_gsbase
	 *        [+24] u64  jf_fsbase
	 *        [+32] u64  jf_rip
	 *        [+40] u64  jf_cs
	 *        [+48] u64  jf_rflags
	 *        [+56] u64  jf_rsp
	 *        [+64] u64  jf_ss
	 */
	movq     %rsp, %rcx /* Keep a pointer to the remainder of the return context. */
	pushq_cfi 48(%rcx) /* Load RFLAGS */
	popfq_cfi_r
	movq     56(%rcx), %rsp  /* Load the new stack. */
	.cfi_def_cfa_offset 0
	pushq_cfi 32(%rcx) /* RIP */

	/* Load segment registers. */
	movq     16(%rcx), %rax
	wrgsbaseq %rax
	movq     24(%rcx), %rax
	wrfsbaseq %rax

	/* Load remaining GP registers and return. */
	movq     8(%rcx), %rax
	movq     0(%rcx), %rcx
	ret
	.cfi_restore_state
.full_rpc_resume:
	.cfi_remember_state
	movq    144(%rsp), %rax
	jmp     .full_rpc_load_context_rax_is_rip

	.cfi_restore_state
.full_rpc_maybe_restart:
	testq   $(RPC_REASON_FSHOULD_RESTART), 0(%rsp) /* jf_mode */
	jnz     .full_rpc_restart
.full_rpc_interrupt:
	testq   $(RPC_REASON_NOEXCEPT), 0(%rsp) /* jf_mode */
	jnz     1f
	movq    $(E_INTERRUPT), %rdi
	call    libc_error_throw

1:	/* Emulate the system call returning -EINTR */
	movq    $-EINTR, %rax
	jmp     .full_rpc_resume
	.cfi_endproc
SYMEND(x86_rpc_entry_full)





















