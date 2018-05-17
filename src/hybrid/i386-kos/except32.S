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
#include <asm/cfi.h>
#include <except.h>

.section .text.crt.except

#define e_code       EXCEPTION_INFO_OFFSETOF_CODE
#define e_flag       EXCEPTION_INFO_OFFSETOF_FLAG
#define e_data       EXCEPTION_INFO_OFFSETOF_DATA
#define e_context    EXCEPTION_INFO_OFFSETOF_CONTEXT

#define ts_self      TASK_SEGMENT_OFFSETOF_SELF
#define ts_xcurrent  TASK_SEGMENT_OFFSETOF_XCURRENT

#define c_gpregs     X86_CONTEXT_OFFSETOF_GPREGS
#define c_esp        X86_CONTEXT_OFFSETOF_ESP
#define c_eip        X86_CONTEXT_OFFSETOF_EIP
#define c_eflags     X86_CONTEXT_OFFSETOF_EFLAGS

INTERN_ENTRY(libc_error_rethrow)
	.cfi_startproc
	/* Construct a new CPU-state for the caller. */
	popl_cfi %eax   /* Load EIP */
	.cfi_register %eip, %eax
.error_rethrow_eax_is_ip:
	pushfl_cfi_r        /* EFLAGS */
#ifdef __KERNEL__
	pushl_cfi    %cs    /* CS */
#endif
	pushl_cfi    %eax   /* EIP */
	.cfi_rel_offset %eip, 0
#ifndef CONFIG_NO_X86_SEGMENTATION
	pushl_cfi_r  %ds
	pushl_cfi_r  %es
	pushl_cfi_r  %fs
	pushl_cfi_r  %gs
#endif /* !CONFIG_NO_X86_SEGMENTATION */
	pushal_cfi_r
	/* Fix ESP to point to caller, aka. the start of the context structure. */
	leal   X86_CONTEXT_SIZE(%esp), %ecx
	movl   %ecx, X86_CONTEXT_OFFSETOF_ESP(%esp)
	movl   %esp, %ecx
	/* Fix EIP to point into the middle of the
	 * instruction the caller used to get here. */
	decl   X86_CONTEXT_OFFSETOF_EIP(%ecx)
	/* Rethrow the exception. */
	call   libc_error_rethrow_at
	.cfi_endproc
SYMEND(libc_error_rethrow)
EXPORT(error_rethrow,libc_error_rethrow)

.cfi_startproc
INTERN_ENTRY(libc_error_throw_resumable_ex)
	movw   $(ERR_FRESUMABLE|ERR_FRESUMEFUNC), e_flag(%edx)
	jmp    1f
INTERN_ENTRY(libc_error_throw_ex)
	movw   $(ERR_FNORMAL), e_flag(%edx)
1:	movw   %cx, e_code(%edx)
	/* Copy exception information into TLS data. */
	movl   $(__EXCEPTION_INFO_OFFSETOF_CONTEXT/4), %ecx
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	leal   ts_xcurrent, %edi
	addl   %taskseg:ts_self, %edi
	movl   %edx, %esi
	rep    movsl
	popl_cfi_r %esi
	popl_cfi_r %edi
	jmp    libc_error_throw_current
INTERN_ENTRY(libc_error_throw_resumablef)
	movw   $(ERR_FRESUMABLE|ERR_FRESUMEFUNC), %taskseg:ts_xcurrent+e_flag
	jmp    1f
INTERN_ENTRY(libc_error_throwf)
1:	movl   4(%esp), %ecx /* except_t code; */
	movw   %cx, %taskseg:ts_xcurrent+e_code
	leal   8(%esp), %edx /* va_list args; */
DEFINE_INTERN(libc_error_setf)
	call   libc_error_setf
	jmp    libc_error_throw_current
INTERN_ENTRY(libc_error_throw_resumable)
	movw   $(ERR_FRESUMABLE|ERR_FRESUMEFUNC), %taskseg:ts_xcurrent+e_flag
	jmp    1f
INTERN_ENTRY(libc_error_throw)
	movw   $(ERR_FNORMAL), %taskseg:ts_xcurrent+e_flag
1:	movw   %cx, %taskseg:ts_xcurrent+e_code
	/* Save basic registers. */
	movl   %eax, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX
	movl   %ecx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX
	movl   %edx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX
	movl   %ebx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBX
	movl   %ebp, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBP
	movl   %edi, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDI
	movl   %esi, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ESI
	pushfl_cfi
	popl_cfi     %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_EFLAGS
	/* Clear extended exception informations. */
	movl   $(ts_xcurrent+e_data), %edi
	addl   %taskseg:ts_self, %edi
	movl   $(EXCEPTION_INFO_NUM_DATA_POINTERS), %ecx
	xorl   %eax, %eax
#ifdef __KERNEL__
	cld
#endif
	rep    stosl
	jmp    1f
INTERN_ENTRY(libc_error_throw_current)
	/* Save the current CPU context. */
	movl   %eax, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX
	movl   %ecx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX
	movl   %edx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX
	movl   %ebx, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBX
	movl   %ebp, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBP
	movl   %edi, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDI
	movl   %esi, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ESI
	pushfl_cfi
	popl_cfi     %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_EFLAGS
1:	popl_cfi %eax  /* Return address. */
	.cfi_register %eip, %eax
	movl   %eax, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_EIP
	movl   %esp, %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_ESP
#ifndef CONFIG_NO_X86_SEGMENTATION
	movw   %gs,  %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS
	movw   $0,   %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS+2
	movw   %fs,  %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS
	movw   $0,   %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS+2
	movw   %es,  %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES
	movw   $0,   %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES+2
	movw   %ds,  %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS
	movw   $0,   %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS+2
#endif /* !CONFIG_NO_X86_SEGMENTATION */
#ifdef __KERNEL__
	movw   %cs,  %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_CS
	movw   $0,   %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_CS+2
#elif !defined(CONFIG_NO_X86_SEGMENTATION)
	movw   %cs,  %taskseg:ts_xcurrent+e_context+X86_CONTEXT32_OFFSETOF_CS
	movw   $0,   %taskseg:ts_xcurrent+e_context+X86_CONTEXT32_OFFSETOF_CS+2
	movw   %ss,  %taskseg:ts_xcurrent+e_context+X86_CONTEXT32_OFFSETOF_SS
	movw   $0,   %taskseg:ts_xcurrent+e_context+X86_CONTEXT32_OFFSETOF_SS+2
#endif
	jmp    .error_rethrow_eax_is_ip
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
	cmpl   $0, %ecx
	jl     1f                       /* EXCEPT_CONTINUE_RETRY or EXCEPT_CONTINUE_IGNORE */
	je     libc_error_throw_current /* EXCEPT_CONTINUE_SEARCH */
	ret                             /* EXCEPT_EXECUTE_HANDLER */
1:	cmpl   $(EXCEPT_CONTINUE_RETRY), %ecx
	sete   %al       /* AL = mode == EXCEPT_CONTINUE_RETRY */
	movzbl %al, %ecx
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
	movzwl %taskseg:ts_xcurrent+e_code, %eax
	ret
SYMEND(libc_error_code)
EXPORT(error_code,libc_error_code)

INTERN_ENTRY(libc_error_info)
	movl $(ts_xcurrent),   %eax
	addl %taskseg:ts_self, %eax
	ret
SYMEND(libc_error_info)
EXPORT(error_info,libc_error_info)


#if 0
INTERN_ENTRY(libc_error_handled)
	ret
SYMEND(libc_error_handled)
EXPORT(error_handled,libc_error_handled)

INTERN_ENTRY(libc_error_dealloc_continue)
	xorl %eax, %eax
	ret
SYMEND(libc_error_dealloc_continue)
EXPORT(error_dealloc_continue,libc_error_dealloc_continue)
#else

INTERN_ENTRY(libc_error_handled)
	.cfi_def_cfa %esp, 4
	popl_cfi %edx
	.cfi_register %eip, %edx
	cmpl     %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_ESP, %esp
	jne      1f
	movl     %taskseg:ts_xcurrent+EXCEPTION_INFO_OFFSETOF_RTDATA+__EXCEPTION_RT_DATA_OFFSETOF_FREE_SP, %esp
1:	jmpl     *%edx
SYMEND(libc_error_handled)
EXPORT(error_handled,libc_error_handled)

INTERN_ENTRY(libc_error_dealloc_continue)
	popl_cfi %edx
	.cfi_register %eip, %edx
	cmpl     %taskseg:ts_xcurrent+e_context+X86_CONTEXT_OFFSETOF_ESP, %esp
	jne      1f
	movl     %taskseg:ts_xcurrent+EXCEPTION_INFO_OFFSETOF_RTDATA+__EXCEPTION_RT_DATA_OFFSETOF_FREE_SP, %ecx
	.cfi_def_cfa %ecx, 0
	movl     %ecx, %eax
	subl     %esp, %eax
	movl     %ecx, %esp
	jmpl     *%edx
1:	.cfi_def_cfa %esp, 0
	jmpl     *%edx
SYMEND(libc_error_dealloc_continue)
EXPORT(error_dealloc_continue,libc_error_dealloc_continue)
#endif


.cfi_endproc
