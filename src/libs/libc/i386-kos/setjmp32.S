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

.hidden sys_sigprocmask
.global sys_sigprocmask

#define SAVEMASK_OFFSET 28

.section .text
INTERN_ENTRY(libc_setjmp)
	.cfi_startproc
	popl_cfi %ecx
	.cfi_register %eip, %ecx
	movl 0(%esp),                %eax
	movl   %ebx,               0(%eax)
	movl   %esp,               4(%eax)
	movl   %ebp,               8(%eax)
	movl   %esi,              12(%eax)
	movl   %edi,              16(%eax)
	movl   %ecx,              20(%eax) /* EIP */
	movl   $0,   SAVEMASK_OFFSET(%eax)
	xorl   %eax,                 %eax /* Return ZERO(0) the first time around. */
	jmpl  *%ecx
	.cfi_endproc
SYMEND(libc_setjmp)

EXPORT(setjmp,libc_setjmp)



INTERN_ENTRY(libc_sigsetjmp)
	.cfi_startproc
	popl_cfi %ecx
	.cfi_register %eip, %ecx
	movl 0(%esp),                  %eax
	movl 4(%esp),                  %edx
	movl   %ebx,                 0(%eax)
	movl   %esp,                 4(%eax)
	movl   %ebp,                 8(%eax)
	movl   %esi,                12(%eax)
	movl   %edi,                16(%eax)
	movl   %ecx,                20(%eax) /* EIP */
	movl   %edx,   SAVEMASK_OFFSET(%eax) /* SAFEMASK */
	testl  %edx,                   %edx
	jz     1f /* Skip the signal mask if if shouldn't be saved */
#if 1
	pushl_cfi $(__SIZEOF_SIGSET_T__)
	leal   32(%eax), %eax
	pushl_cfi %eax           /* oldset = &buf->__sig; */
	pushl_cfi $0             /* set = NULL; */
	pushl_cfi $(SIG_SETMASK) /* how = SIG_SETMASK; */
	call   sys_sigprocmask
	addl   $16, %esp
	.cfi_adjust_cfa_offset -16
#else
	pushl_cfi_r %ebx  /* Save protected registers. */
	pushl_cfi_r %esi
	pushl_cfi   %ecx  /* ECX contains the return address (preserve it) */
	.cfi_rel_offset %eip, 0
	movl   %eax,                   %edx /* EDX = __buf; */
	movl   $(__NR_sigprocmask),    %eax
	movl   $(SIG_SETMASK),         %ebx /* how = SIG_SETMASK; */
	movl   $0,                     %ecx /* set = NULL; */
	addl   $32,                    %edx /* oldset = &__buf->__sig; */
	movl   $(__SIZEOF_SIGSET_T__), %esi /* sigsetsize = sizeof(sigset_t); */
	int    $0x80                        /* Restore signal mask */
	popl_cfi %ecx  /* Restore return address */
	.cfi_register %eip, %ecx
	popl_cfi_r %esi
	popl_cfi_r %ebx
#endif
1:	xorl   %eax,    %eax  /* Return ZERO(0) the first time around. */
	jmpl  *%ecx
	.cfi_endproc
SYMEND(libc_sigsetjmp)

EXPORT(sigsetjmp,libc_sigsetjmp)
EXPORT_STRONG(__sigsetjmp,libc_sigsetjmp) /* GLibc alias */




.section .text
INTERN_ENTRY(libc_siglongjmp)
	.cfi_startproc
	movl   4(%esp), %eax
	testl  $-1,    SAVEMASK_OFFSET(%eax)
	jz     1f /* If the signal mask wasn't saved, don't restore it. */
#if 1
	pushl_cfi $(__SIZEOF_SIGSET_T__)
	pushl_cfi $0             /* oldset = NULL; */
	leal   32(%eax), %eax
	pushl_cfi %eax           /* set = &buf->__sig; */
	pushl_cfi $(SIG_SETMASK) /* how = SIG_SETMASK; */
	call   sys_sigprocmask
	addl   $16, %esp
	.cfi_adjust_cfa_offset -16
#else
	movl   $(__NR_sigprocmask),    %eax
	movl   $(SIG_SETMASK),         %ebx /* how = SIG_SETMASK; */
	movl   4(%esp),                %ecx
	addl   $32,                    %ecx /* set = &buf->__sig; */
	movl   $0,                     %edx /* oldset = NULL; */
	movl   $(__SIZEOF_SIGSET_T__), %esi /* sigsetsize = sizeof(sigset_t); */
	int    $0x80                        /* Restore signal mask */
#endif
INTERN_ENTRY(libc_longjmp)
1:	movl   8(%esp), %eax
	testl  %eax, %eax
	jnz    1f
	incl   %eax /* Return 1 instead! */
	jmp    1f
INTERN_ENTRY(libc___longjmp2)
	movl   8(%esp),  %eax
1:	movl   4(%esp),  %edx /* buf */
	movl   0(%edx),  %ebx
	movl   4(%edx),  %ecx
	movl   8(%edx),  %ebp
	movl   12(%edx), %esi
	movl   16(%edx), %edi
	movl   %ecx,     %esp
	jmpl   *20(%edx)
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
	popl_cfi %edx     /* Return address. */
	.cfi_register %eip, %edx
	popl_cfi %ecx     /* Allocation %ecx. */
	.cfi_undefined %esp
	cmpl  $(PAGESIZE), %ecx
	jbe   70f
	/* Align ESP by `PAGESIZE' */
	movl  %esp, %eax
	andl  $(~(PAGESIZE-1)), %esp
	andl  $( (PAGESIZE-1)), %eax
	subl  %eax, %ecx
1:	cmpl  $(PAGESIZE), %ecx
	jbe   70f
	subl  $(PAGESIZE), %esp
	subl  $(PAGESIZE), %ecx
	testb $0,  (%esp) /* Probe the next page. */
	jmp   1b
70:	subl  %ecx, %esp
	testb $0,  (%esp) /* Probe the last page. */
	movl  %esp, %eax
	jmpl *%edx
	.cfi_endproc
SYMEND(libc_alloca)

EXPORT(alloca,libc_alloca)























