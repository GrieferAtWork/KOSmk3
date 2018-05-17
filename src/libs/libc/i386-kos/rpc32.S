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
#include <kos/rpc.h>
#include <errno.h>
#include <except.h>


.section .text.crt.rpc
INTERN_ENTRY(x86_rpc_entry_fast)
	/* ESP:
	 *        [+0]   u32  jf_mode
	 *        [+4]   u32  jf_edx
	 *        [+8]   u32  jf_ecx
	 *        [+12]  u32  jf_eax
	 *        [+16]  u32  jf_eip
	 *        [+20]  u32  jf_eflags
	 */
	.cfi_startproc simple
	.cfi_def_cfa    %esp,    24
	.cfi_rel_offset %edx,    4
	.cfi_rel_offset %ecx,    8
	.cfi_rel_offset %eax,    12
	.cfi_rel_offset %eip,    16
	.cfi_rel_offset %eflags, 20
	/* ECX: rpc_t func */
	/* EDX: void *arg */
	/* Invoke the RPC function. */
	pushl_cfi %esp
DEFINE_INTERN(libc_invoke_rpc_fast)
	call    libc_invoke_rpc_fast
	/*addl    $4, %esp*/
	.cfi_adjust_cfa_offset -4

	/* If the RPC didn't interrupt a system call, always resume execution. */
	testl   $(RPC_REASON_SYSCALL), 0(%esp)
	jz      .fast_rpc_resume /* Wasn't a system call -> Just resume */
	cmpl    $(RPC_RETURN_RESUME), %eax
	je      .fast_rpc_resume /* The RPC explicitly wants us to resume -> Resume */
	testl   $(RPC_REASON_FMUST_RESTART), 0(%esp)
	jz      .fast_rpc_restart /* The system call must be restarted. */
	cmpl    $(RPC_RETURN_RESTART_SYSCALL), %eax
	je      .fast_rpc_maybe_restart /* The system call can be restarted. */
	cmpl    $(RPC_RETURN_INTERRUPT), %eax
	je      .fast_rpc_interrupt /* The RPC wants us to signal an interrupt. */

.fast_rpc_restart:
	.cfi_remember_state

	/* Exchange jf_eip and jf_eflags, and repeat the last instruction. */
	movl    16(%esp), %eax
	pushl_cfi %eax
	call    libc_prev_instruction
	addl    $4, %esp
	.cfi_adjust_cfa_offset -4
	/* XXX: What if EAX is NULL now? (I mean, it shouldn't, but what if?) */
	xchgl   20(%esp), %eax
	.cfi_rel_offset %eip,    20
	.cfi_rel_offset %eflags, 16
	movl    %eax, 16(%esp)

	/* Load the saved context (see above) */
	addl    $4, %esp /* jf_mode */
	.cfi_adjust_cfa_offset -4
	popl_cfi_r %edx
	popl_cfi_r %ecx
	popl_cfi_r %eax
	popfl_cfi_r
	ret
	.cfi_restore_state
.fast_rpc_resume:
	.cfi_remember_state
	/* Exchange jf_eip and jf_eflags */
	movl    16(%esp), %eax
	xchgl   20(%esp), %eax
	.cfi_rel_offset %eip,    20
	.cfi_rel_offset %eflags, 16
	movl    %eax, 16(%esp)

	/* Load the saved context (see above) */
	addl    $4, %esp /* jf_mode */
	.cfi_adjust_cfa_offset -4
	popl_cfi_r %edx
	popl_cfi_r %ecx
	popl_cfi_r %eax
	popfl_cfi_r
	ret

	.cfi_restore_state
.fast_rpc_maybe_restart:
	testl   $(RPC_REASON_FSHOULD_RESTART), 0(%esp) /* jf_mode */
	jnz     .fast_rpc_restart
.fast_rpc_interrupt:
	testl   $(RPC_REASON_NOEXCEPT), 0(%esp) /* jf_mode */
	jnz     1f
	movl    $(E_INTERRUPT), %ecx
	call    libc_error_throw

1:	/* Emulate the system call returning -EINTR */
	movl    $-EINTR, %eax
	jmp     .fast_rpc_resume
	.cfi_endproc
SYMEND(x86_rpc_entry_fast)





















