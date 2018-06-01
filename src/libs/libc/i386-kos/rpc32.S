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
	movl %taskseg:TASK_SEGMENT_OFFSETOF_NO_RPC, %ecx
	incl %ecx
	movl %ecx, %taskseg:TASK_SEGMENT_OFFSETOF_NO_RPC
	xorl %eax, %eax
	cmpl $1, %ecx
	sete %al
	jne  1f
	pushl_cfi $1
	call Xsys_xnosignal
	addl $4, %esp
	.cfi_adjust_cfa_offset -4
1:	ret
SYMEND(libc_rpc_pushoff)
EXPORT(rpc_pushoff,libc_rpc_pushoff)

INTERN_ENTRY(libc_rpc_pop)
	xorl %eax, %eax
	decl %taskseg:TASK_SEGMENT_OFFSETOF_NO_RPC
	setz %al
	jnz  1f
	pushl_cfi $0
	call Xsys_xnosignal
	addl $4, %esp
	.cfi_adjust_cfa_offset -4
1:	ret
SYMEND(libc_rpc_pop)
EXPORT(rpc_pop,libc_rpc_pop)
.cfi_endproc


.section .text.crt.rpc
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
	nop /* This counteracts the -1 adjustment done by tracebacks. */
	.align 16
INTERN_ENTRY(x86_rpc_entry_fast)
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
	jnz     .fast_rpc_restart /* The system call must be restarted. */
	cmpl    $(RPC_RETURN_RESTART), %eax
	je      .fast_rpc_maybe_restart /* The system call can be restarted. */
	cmpl    $(RPC_RETURN_INTERRUPT), %eax
	je      .fast_rpc_interrupt /* The RPC wants us to signal an interrupt. */
	/* DEFAULT: RPC_RETURN_FORCE_RESTART */

.fast_rpc_restart:
	.cfi_remember_state

	/* Exchange jf_eip and jf_eflags, and repeat the last instruction. */
	movl    16(%esp), %ecx
	call    libc_prev_instruction
	/* XXX: What if EAX is NULL now? (I mean, it shouldn't, but what if?) */
	xchgl   20(%esp), %eax
	.cfi_rel_offset %eip,    20
	.cfi_register   %eflags, %eax
	movl    %eax, 16(%esp)
	.cfi_rel_offset %eflags, 16

	/* Load the saved context (see above) */
	popl_void_cfi    /* jf_mode */
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
	.cfi_register   %eflags, %eax
	movl    %eax, 16(%esp)
	.cfi_rel_offset %eflags, 16

	/* Load the saved context (see above) */
	popl_void_cfi    /* jf_mode */
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














.section .text.crt.rpc
	/* ESP:
	 *        [+0]   u32  jf_mode
	 *        [+4]   u32  jf_edi
	 *        [+8]   u32  jf_esi
	 *        [+12]  u32  jf_ebp
	 *        [+16]  u32  jf_esp
	 *        [+20]  u32  jf_ebx
	 *        [+24]  u32  jf_edx
	 *        [+28]  u32  jf_ecx
	 *        [+32]  u32  jf_eax
	 *        [+36]  u32  jf_gs
	 *        [+40]  u32  jf_fs
#ifndef CONFIG_X86_FIXED_SEGMENTATION
	 *        [+44]  u32  jf_es
	 *        [+48]  u32  jf_ds
	 *        [+52]  u32  jf_eip
	 *        [+56]  u32  jf_eflags
	 *        [+60]  u32  jf_cs
	 *        [+64]  u32  jf_ss
#else
	 *        [+44]  u32  jf_eip
	 *        [+48]  u32  jf_eflags
#endif
	 */
	.cfi_startproc simple
	.cfi_def_cfa    %esp,    68
	.cfi_rel_offset %edi,    4
	.cfi_rel_offset %esi,    8
	.cfi_rel_offset %ebp,    12
/*	.cfi_rel_offset %esp,    16 */
	.cfi_rel_offset %ebx,    20
	.cfi_rel_offset %edx,    24
	.cfi_rel_offset %ecx,    28
	.cfi_rel_offset %eax,    32
#ifndef CONFIG_X86_FIXED_SEGMENTATION
#ifndef CONFIG_NO_X86_SEGMENTATION
	.cfi_rel_offset %gs,     36
	.cfi_rel_offset %fs,     40
	.cfi_rel_offset %es,     44
	.cfi_rel_offset %ds,     48
#endif
	.cfi_rel_offset %eip,    52
	.cfi_rel_offset %eflags, 56
/*	.cfi_rel_offset %cs,     60 */
/*	.cfi_rel_offset %ss,     64 */
#else
	.cfi_rel_offset %gs,     36
	.cfi_rel_offset %fs,     40
	.cfi_rel_offset %eip,    44
	.cfi_rel_offset %eflags, 48
#endif
	nop /* This counteracts the -1 adjustment done by tracebacks. */
	.align 16
INTERN_ENTRY(x86_rpc_entry_full)
	/* ECX: rpc_t func */
	/* EDX: void *arg */
	/* Invoke the RPC function. */
	pushl_cfi %esp
DEFINE_INTERN(libc_invoke_rpc_full)
	call    libc_invoke_rpc_full
	/*addl    $4, %esp*/
	.cfi_adjust_cfa_offset -4

	/* If the RPC didn't interrupt a system call, always resume execution. */
	testl   $(RPC_REASON_SYSCALL), 0(%esp)
	jz      .full_rpc_resume /* Wasn't a system call -> Just resume */
	cmpl    $(RPC_RETURN_RESUME), %eax
	je      .full_rpc_resume /* The RPC explicitly wants us to resume -> Resume */
	testl   $(RPC_REASON_FMUST_RESTART), 0(%esp)
	jnz     .full_rpc_restart /* The system call must be restarted. */
	cmpl    $(RPC_RETURN_RESTART), %eax
	je      .full_rpc_maybe_restart /* The system call can be restarted. */
	cmpl    $(RPC_RETURN_INTERRUPT), %eax
	je      .full_rpc_interrupt /* The RPC wants us to signal an interrupt. */
	/* DEFAULT: RPC_RETURN_FORCE_RESTART */

	.cfi_remember_state
.full_rpc_restart:
	/* Exchange jf_eip and jf_eflags, and repeat the last instruction. */
	movl    52(%esp), %ecx
	call    libc_prev_instruction
	/* XXX: What if EAX is NULL now? (I mean, it shouldn't, but what if?) */
	xchgl   56(%esp), %eax
	.cfi_rel_offset %eip,    56
	.cfi_register   %eflags, %eax
	movl    %eax, 52(%esp)
	.cfi_rel_offset %eflags, 52

	/* Load the saved context (see above) */
.full_rpc_load_context:
	popl_void_cfi    /* jf_mode */
	popl_cfi_r %edi
	popl_cfi_r %esi
	popl_cfi_r %ebp
	movl  0(%esp), %eax  /* ESP */
	subl  $8, %eax       /* Make space for a small return tail. */
	movl  16(%esp), %ecx /* EAX */
	movl  %ecx, 0(%eax)
	movl  40(%esp), %ecx /* EIP */
	movl  %ecx, 4(%eax)
/*	popl_cfi_r %esp */ /* TODO */
	addl  $4, %esp
	.cfi_adjust_cfa_offset -4
	popl_cfi_r %ebx
	popl_cfi_r %edx
	popl_cfi_r %ecx
/*	popl_cfi_r %eax */
	.cfi_undefined %eax
	addl  $4, %esp
	.cfi_adjust_cfa_offset -4
#ifndef CONFIG_NO_X86_SEGMENTATION
	popl_cfi_r %gs
	popl_cfi_r %fs
#ifndef CONFIG_NO_X86_SEGMENTATION
	popl_cfi_r %es
	popl_cfi_r %ds
#endif /* !CONFIG_NO_X86_SEGMENTATION */
#else
	addl  $16, %esp
	.cfi_adjust_cfa_offset -16
#endif
/*	movw  8(%esp), %cs */
/*	movw  12(%esp), %ss */
	popfl_cfi_r
	.cfi_def_cfa %eax, 8
	/* Load the new stack. */
	movl  %eax, %esp
	.cfi_def_cfa_register %esp
	.cfi_rel_offset %eax, 0
	.cfi_rel_offset %eip, 4
	popl_cfi_r %eax
	ret
	.cfi_restore_state
.full_rpc_resume:
	.cfi_remember_state
	/* Exchange jf_eip and jf_eflags */
	movl    52(%esp), %eax
/*	.cfi_register   %eip, %eax */
	xchgl   56(%esp), %eax
	.cfi_rel_offset %eip,    56
	.cfi_register   %eflags, %eax
	movl    %eax, 52(%esp)
	.cfi_rel_offset %eflags, 52
	jmp     .full_rpc_load_context

	.cfi_restore_state
.full_rpc_maybe_restart:
	testl   $(RPC_REASON_FSHOULD_RESTART), 0(%esp) /* jf_mode */
	jnz     .full_rpc_restart
.full_rpc_interrupt:
	testl   $(RPC_REASON_NOEXCEPT), 0(%esp) /* jf_mode */
	jnz     1f
	movl    $(E_INTERRUPT), %ecx
	call    libc_error_throw

1:	/* Emulate the system call returning -EINTR */
	movl    $-EINTR, %eax
	jmp     .full_rpc_resume
	.cfi_endproc
SYMEND(x86_rpc_entry_full)





















