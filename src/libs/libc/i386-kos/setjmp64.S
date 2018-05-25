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
#define _KOS_SOURCE 2

#include <hybrid/compiler.h>
#include <hybrid/asm.h>
#include <hybrid/limits.h>
#include <asm/cfi.h>
#include <kos/thread.h>
#include <bits/sigaction.h>
#include <bits/sigset.h>
#include <syscall.h>
#include "../libc.h"

.hidden Xsys_sigprocmask
.global Xsys_sigprocmask

#define SAVEMASK_OFFSET 28

.section .text
INTERN_ENTRY(libc_setjmp)
	.cfi_startproc
	popq_cfi %rcx
	.cfi_register %rip, %rcx
	movq   %rbx,  0(%rdi)
	movq   %rbp,  8(%rdi)
	movq   %r12, 16(%rdi)
	movq   %r13, 24(%rdi)
	movq   %r14, 32(%rdi)
	movq   %r15, 40(%rdi)
	movq   %rsp, 48(%rdi)
	movq   %rcx, 56(%rdi) /* RIP */
	xorq   %rax,    %rax  /* Return ZERO(0) the first time around. */
	jmpq  *%rcx
	.cfi_endproc
SYMEND(libc_setjmp)
EXPORT(setjmp,libc_setjmp)



INTERN_ENTRY(libc_sigsetjmp)
	.cfi_startproc
	popq_cfi %rcx
	.cfi_register %rip, %rcx
	movq   %rbx,  0(%rdi)
	movq   %rbp,  8(%rdi)
	movq   %r12, 16(%rdi)
	movq   %r13, 24(%rdi)
	movq   %r14, 32(%rdi)
	movq   %r15, 40(%rdi)
	movq   %rsp, 48(%rdi)
	movq   %rcx, 56(%rdi) /* RIP */
	movq   %rsi, 64(%rdi) /* SAFEMASK */
	testq  %rsi, %rsi
	jz     1f /* Skip the signal mask if if shouldn't be saved */
	pushq_cfi %rcx /* Preserve RCX */
	.cfi_rel_offset %rip, 0
	movq   $(__SIZEOF_SIGSET_T__), %rcx
	leaq   72(%rdi), %rdx       /* oldset = &buf->__sig; */
	xorq   %rsi, %rsi           /* set = NULL; */
	movq   $(SIG_SETMASK), %rdi /* how = SIG_SETMASK; */
	call   Xsys_sigprocmask
	popq_cfi %rcx /* Restore RCX */
	.cfi_register %rip, %rcx
1:	xorq   %rax, %rax  /* Return ZERO(0) the first time around. */
	jmpq  *%rcx
	.cfi_endproc
SYMEND(libc_sigsetjmp)

EXPORT(sigsetjmp,libc_sigsetjmp)
EXPORT_STRONG(__sigsetjmp,libc_sigsetjmp) /* GLibc alias */




.section .text
INTERN_ENTRY(libc_siglongjmp)
	.cfi_startproc
	testq  $-1, SAVEMASK_OFFSET(%rdi)
	jz     1f /* If the signal mask wasn't saved, don't restore it. */
	pushq_cfi %rdi
	pushq_cfi %rsi
	movq   $(__SIZEOF_SIGSET_T__), %rcx
	xorq   %rdx, %rdx           /* oldset = NULL; */
	leaq   72(%rdi), %rsi       /* set = &buf->__sig; */
	movq   $(SIG_SETMASK), %rdi /* how = SIG_SETMASK; */
	call   Xsys_sigprocmask
	popq_cfi %rsi
	popq_cfi %rdi
INTERN_ENTRY(libc_longjmp)
1:	testq  %rsi, %rsi
	jnz    1f
	incq   %rsi /* Return 1 instead! */
	jmp    1f
INTERN_ENTRY(libc___longjmp2)
1:	movq   %rsi, %rax
	movq    0(%rdi), %rbx
	movq    8(%rdi), %rbp
	movq   16(%rdi), %r12
	movq   24(%rdi), %r13
	movq   32(%rdi), %r14
	movq   40(%rdi), %r15
	movq   48(%rdi), %rsp
	jmpq  *56(%rdi)
SYMEND(libc___longjmp2)
SYMEND(libc_longjmp)
	.cfi_endproc
SYMEND(libc_siglongjmp)

EXPORT(siglongjmp,libc_siglongjmp)
EXPORT(__libc_siglongjmp,libc_siglongjmp)
EXPORT(longjmp,libc_longjmp)
EXPORT(__libc_longjmp,libc_longjmp)
EXPORT(__longjmp2,libc___longjmp2)



.section .text
INTERN_ENTRY(libc_alloca)
	.cfi_startproc
	popq_cfi %rdx     /* Return address. */
	.cfi_register %rip, %rdx
	movq  %rdi, %rcx  /* Allocation %rcx. */
	.cfi_undefined %rsp
	cmpq  $(PAGESIZE), %rcx
	jbe   70f
	/* Align RSP by `PAGESIZE' */
	movq  %rsp, %rax
	andq  $(~(PAGESIZE-1)), %rsp
	andq  $( (PAGESIZE-1)), %rax
	subq  %rax, %rcx
1:	cmpq  $(PAGESIZE), %rcx
	jbe   70f
	subq  $(PAGESIZE), %rsp
	subq  $(PAGESIZE), %rcx
	testb $0,  (%esp) /* Probe the next page. */
	jmp   1b
70:	subq  %rcx, %rsp
	testb $0,  (%rsp) /* Probe the last page. */
	movq  %rsp, %rax
	jmpq *%rdx
	.cfi_endproc
SYMEND(libc_alloca)
EXPORT(alloca,libc_alloca)























