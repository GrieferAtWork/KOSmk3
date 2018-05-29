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
#include <asm/universal.h>
#include <asm/cpu-flags.h>


.macro define_rdbase_wrapper name, reg, msr
INTERN_ENTRY(\name)
	pushfq_cfi_r
.ifnc \reg,rcx
	pushq_cfi_r %rcx
.endif
.ifnc \reg,rdx
	pushq_cfi_r %rdx
.endif
.ifnc \reg,rax
	pushq_cfi_r %rax
.endif
	movl  $\msr, %ecx
	rdmsr
	shlq  $32, %rdx
.ifc \reg,rax
	orq   %rdx, %rax
.else
.ifc \reg,rdx
	orq   %rax, %rdx
.else
	movq  %rdx, %\reg
	orq   %rax, %\reg
.endif
.endif
.ifnc \reg,rax
	popq_cfi_r  %rax
.endif
.ifnc \reg,rdx
	popq_cfi_r  %rdx
.endif
.ifnc \reg,rcx
	popq_cfi_r  %rcx
.endif
	popfq_cfi_r
	ret
SYMEND(\name)
.endm

.macro define_wrbase_wrapper name, reg, msr
INTERN_ENTRY(\name)
	pushfq_cfi_r
	pushq_cfi_r %rax
	pushq_cfi_r %rcx
	pushq_cfi_r %rdx
.ifc \reg,rax
	movq  %rax, %rdx
.else
.ifc \reg,rdx
	movq  %rdx, %rax
.else
	movq  %\reg, %rax
	movq  %\reg, %rdx
.endif
.endif
	shrq  $32,  %rdx
	movl  $\msr, %ecx
	wrmsr
	popq_cfi_r  %rdx
	popq_cfi_r  %rcx
	popq_cfi_r  %rax
	popfq_cfi_r
	ret
SYMEND(\name)
.endm


.section .text
.cfi_startproc
define_rdbase_wrapper  x86_rdfsbase_rax, rax, IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_rcx, rcx, IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_rdx, rdx, IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_rbx, rbx, IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_rdi, rdi, IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_rsi, rsi, IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_rbp, rbp, IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_r8,  r8,  IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_r9,  r9,  IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_r10, r10, IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_r11, r11, IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_r12, r12, IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_r13, r13, IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_r14, r14, IA32_FS_BASE
define_rdbase_wrapper  x86_rdfsbase_r15, r15, IA32_FS_BASE
define_rdbase_wrapper  x86_rdgsbase_rax, rax, IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_rcx, rcx, IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_rdx, rdx, IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_rbx, rbx, IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_rdi, rdi, IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_rsi, rsi, IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_rbp, rbp, IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_r8,  r8,  IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_r9,  r9,  IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_r10, r10, IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_r11, r11, IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_r12, r12, IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_r13, r13, IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_r14, r14, IA32_GS_BASE
define_rdbase_wrapper  x86_rdgsbase_r15, r15, IA32_GS_BASE

/* We must define these symbols as something, even though
 * they should never be used! */
DEFINE_INTERN_ALIAS(x86_rdfsbase_rsp,x86_rdfsbase_rbp)
DEFINE_INTERN_ALIAS(x86_rdgsbase_rsp,x86_rdgsbase_rbp)


define_wrbase_wrapper  x86_wrfsbase_rax, rax, IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_rcx, rcx, IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_rdx, rdx, IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_rbx, rbx, IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_rdi, rdi, IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_rsi, rsi, IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_rbp, rbp, IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_rsp, rsp, IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_r8,  r8,  IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_r9,  r9,  IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_r10, r10, IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_r11, r11, IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_r12, r12, IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_r13, r13, IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_r14, r14, IA32_FS_BASE
define_wrbase_wrapper  x86_wrfsbase_r15, r15, IA32_FS_BASE
define_wrbase_wrapper  x86_wrgsbase_rax, rax, IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_rcx, rcx, IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_rdx, rdx, IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_rbx, rbx, IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_rdi, rdi, IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_rsi, rsi, IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_rbp, rbp, IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_rsp, rsp, IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_r8,  r8,  IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_r9,  r9,  IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_r10, r10, IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_r11, r11, IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_r12, r12, IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_r13, r13, IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_r14, r14, IA32_GS_BASE
define_wrbase_wrapper  x86_wrgsbase_r15, r15, IA32_GS_BASE
.cfi_endproc






















