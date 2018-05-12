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
#include <asm/cfi.h>
#include <syscall.h>
#include <kos/ushare.h>
#include <kos/thread.h>

/* Select how system calls are performed. */
#define CONFIG_USE_USHARE_LAZY_SYSCALLS   1 /* Use `USHARE_X86_SYSCALL_FNAME' and let the kernel decide. */
//#define CONFIG_USE_INT80                1 /* Use `int 0x80' */
//#define CONFIG_USE_SYSENTER             1 /* Use `sysenter' */
//#define CONFIG_USE_PF                   1 /* Use pagefaults (s.a. `X86_ENCODE_PFSYSCALL()') */


#ifdef CONFIG_USE_USHARE_LAZY_SYSCALLS

.section .data
INTERN_ENTRY(libc_Xsyscall)
	/* Set the exception-support bit in the system call number
	 * Other than that, this is exactly the same as the regular syscall() function. */
	orl    $0x80000000, 4(%esp)
INTERN_ENTRY(libc_ssyscall)
	.byte  0xe9 /* jmp --- */
	.long  0
	.byte  0xe8 /* pushl %eip;   (call 1f; 1:) */
	.long  0
	pushl  %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(7)), %eax
	jmp    .x86_syscall_load_common
SYMEND(libc_ssyscall)
SYMEND(libc_Xsyscall)

INTERN_ENTRY(libc_syscall)
	.byte  0xe9 /* jmp --- */
	.long  0
	.byte  0xe8 /* pushl %eip;   (call 1f; 1:) */
	.long  0
	pushl  %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER_ERRNO(7)), %eax
	jmp    .x86_syscall_load_common
SYMEND(libc_syscall)


#else

#define SYSCALL_ERRNO_TAIL(error) \
	cmpl  $__ERRNO_THRESHOLD, %eax; \
	jae   error

.section .text
INTERN_ENTRY(libc_Xsyscall)
	/* Set the exception-support bit in the system call number
	 * Other than that, this is exactly the same as the regular syscall() function. */
	orl    $0x80000000, 4(%esp)
INTERN_ENTRY(libc_ssyscall)
	.cfi_startproc
	/* Save callee-save registers */
	pushl_cfi_r %ebx
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebp

	/* Load arguments */
#define  A  20 /* return+4*register (4+4*4) */
	movl A+ 0(%esp), %eax /* sysno */
	.cfi_undefined   %eax
	movl A+ 4(%esp), %ebx /* Arg #1 */
	movl A+ 8(%esp), %ecx /* Arg #2 */
	.cfi_undefined   %ecx
	movl A+12(%esp), %edx /* Arg #3 */
	.cfi_undefined   %edx
	movl A+16(%esp), %esi /* Arg #4 */
	movl A+20(%esp), %edi /* Arg #5 */
	movl A+24(%esp), %ebp /* Arg #6 */
#undef A

	int  $0x80 /* Invoke the system call interrupt vector */

	/* Restore callee-save registers */
	popl_cfi_r %ebp
	popl_cfi_r %esi
	popl_cfi_r %edi
	popl_cfi_r %ebx
	ret
	.cfi_endproc
SYMEND(libc_ssyscall)
SYMEND(libc_Xsyscall)

INTERN_ENTRY(libc_syscall)
	.cfi_startproc
	/* Save callee-save registers */
	pushl_cfi_r %ebx
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebp

	/* Load arguments */
#define  A  20 /* return+4*register (4+4*4) */
	movl A+ 0(%esp), %eax /* sysno */
	.cfi_undefined   %eax
	movl A+ 4(%esp), %ebx /* Arg #1 */
	movl A+ 8(%esp), %ecx /* Arg #2 */
	.cfi_undefined   %ecx
	movl A+12(%esp), %edx /* Arg #3 */
	.cfi_undefined   %edx
	movl A+16(%esp), %esi /* Arg #4 */
	movl A+20(%esp), %edi /* Arg #5 */
	movl A+24(%esp), %ebp /* Arg #6 */
#undef A

	int  $0x80 /* Invoke the system call interrupt vector */

	/* Restore callee-save registers */
	popl_cfi_r %ebp
	popl_cfi_r %esi
	popl_cfi_r %edi
	popl_cfi_r %ebx
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
.syscall_errno:
	negl  %eax
	movl  %eax, %Utaskseg:t_errno
	movb  $TASK_ERRNO_FKOS, %Utaskseg:t_eformat
	movl  $-1, %eax
	ret
	.cfi_endproc
SYMEND(libc_syscall)


#endif



#ifdef CONFIG_USE_USHARE_LAZY_SYSCALLS
.section .text.hot
.cfi_startproc
INTERN_ENTRY(x86_syscall_load_0)
	pushl_cfi %eax
	call   x86_get_syscall_segment
#if USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(0) != 0
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(0)), %eax
#endif
	jmp    .x86_syscall_load_common
	.cfi_adjust_cfa_offset -4
INTERN_ENTRY(x86_syscall_load_1)
	pushl_cfi %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(1)), %eax
	jmp    .x86_syscall_load_common
	.cfi_adjust_cfa_offset -4
INTERN_ENTRY(x86_syscall_load_2)
	pushl_cfi %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(2)), %eax
	jmp    .x86_syscall_load_common
	.cfi_adjust_cfa_offset -4
INTERN_ENTRY(x86_syscall_load_3)
	pushl_cfi %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(3)), %eax
	jmp    .x86_syscall_load_common
	.cfi_adjust_cfa_offset -4
INTERN_ENTRY(x86_syscall_load_4)
	pushl_cfi %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(4)), %eax
	jmp    .x86_syscall_load_common
	.cfi_adjust_cfa_offset -4
INTERN_ENTRY(x86_syscall_load_5)
	pushl_cfi %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(5)), %eax
	jmp    .x86_syscall_load_common
	.cfi_adjust_cfa_offset -4
INTERN_ENTRY(x86_syscall_load_6)
	pushl_cfi %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(6)), %eax
	jmp    .x86_syscall_load_common
	.cfi_adjust_cfa_offset -4
INTERN_ENTRY(x86_syscall_load_errno_0)
	pushl_cfi %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER_ERRNO(0)), %eax
	jmp    .x86_syscall_load_common
	.cfi_adjust_cfa_offset -4
INTERN_ENTRY(x86_syscall_load_errno_1)
	pushl_cfi %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER_ERRNO(1)), %eax
	jmp    .x86_syscall_load_common
	.cfi_adjust_cfa_offset -4
INTERN_ENTRY(x86_syscall_load_errno_2)
	pushl_cfi %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER_ERRNO(2)), %eax
	jmp    .x86_syscall_load_common
	.cfi_adjust_cfa_offset -4
INTERN_ENTRY(x86_syscall_load_errno_3)
	pushl_cfi %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER_ERRNO(3)), %eax
	jmp    .x86_syscall_load_common
	.cfi_adjust_cfa_offset -4
INTERN_ENTRY(x86_syscall_load_errno_4)
	pushl_cfi %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER_ERRNO(4)), %eax
	jmp    .x86_syscall_load_common
	.cfi_adjust_cfa_offset -4
INTERN_ENTRY(x86_syscall_load_errno_5)
	pushl_cfi %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER_ERRNO(5)), %eax
	jmp    .x86_syscall_load_common
	.cfi_adjust_cfa_offset -4
INTERN_ENTRY(x86_syscall_load_errno_6)
	pushl_cfi %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER_ERRNO(6)), %eax
.x86_syscall_load_common:
#if 0
	movl   %eax, %ecx
	popl_cfi %eax
	addl   $4, %esp
	jmp    *%ecx
#else
	movl   4(%esp),  %ecx
	subl   $5,       %ecx
	subl   %ecx,     %eax  /* Make the syscall entry address relative */
	subl   $5,       %ecx  /* Load the start of the `call' instruction */
	.cfi_register %eip, %ecx
	movl   %eax,   1(%ecx) /* Override the jmp-instruction's target */
	popl_cfi %eax          /* Restore the system call number */
	addl   $4, %esp        /* Pop the return address */
	.cfi_adjust_cfa_offset -4
	jmp    *%ecx           /* Re-execute the (now) `jmp' instruction */
#endif
SYMEND(x86_syscall_load_errno_6)
SYMEND(x86_syscall_load_errno_5)
SYMEND(x86_syscall_load_errno_4)
SYMEND(x86_syscall_load_errno_3)
SYMEND(x86_syscall_load_errno_2)
SYMEND(x86_syscall_load_errno_1)
SYMEND(x86_syscall_load_errno_0)
SYMEND(x86_syscall_load_6)
SYMEND(x86_syscall_load_5)
SYMEND(x86_syscall_load_4)
SYMEND(x86_syscall_load_3)
SYMEND(x86_syscall_load_2)
SYMEND(x86_syscall_load_1)
SYMEND(x86_syscall_load_0)
.cfi_endproc

.section .data.hot


#elif defined(CONFIG_USE_PF)

#define ENCODE_SYSNO   X86_ENCODE_PFSYSCALL

/* syscall #PF version */
.section .text.hot
.cfi_startproc
INTERN_ENTRY(libc_sysenter_0)
	/* Generate the syscall enter address. */
	addl  %taskseg:TASK_SEGMENT_OFFSETOF_X86SYSBASE, %eax
	/* Use that address as return address of this function,
	 * and save the ~real~ return address in EAX. */
	xchgl %eax, 0(%esp)
	.cfi_register %eip, %eax
	/* Invoke the system call by doing a `ret', which will
	 * jump to the address offset from sysbase with EIP == CR2. */
	ret
	.cfi_restore %eip
SYMEND(libc_sysenter_0)

INTERN_ENTRY(libc_sysenter_1)
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	call  libc_sysenter_0
	popl_cfi_r %ebx
	ret
SYMEND(libc_sysenter_1)

INTERN_ENTRY(libc_sysenter_2)
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	movl  12(%esp), %ecx
	call  libc_sysenter_0
	popl_cfi_r %ebx
	ret
SYMEND(libc_sysenter_2)

INTERN_ENTRY(libc_sysenter_3)
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	movl  12(%esp), %ecx
	movl  16(%esp), %edx
	call  libc_sysenter_0
	popl_cfi_r %ebx
	ret
SYMEND(libc_sysenter_3)

INTERN_ENTRY(libc_sysenter_4)
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  12(%esp), %ebx
	movl  16(%esp), %ecx
	movl  20(%esp), %edx
	movl  24(%esp), %esi
	call  libc_sysenter_0
	popl_cfi_r %ebx
	popl_cfi_r %esi
	ret
SYMEND(libc_sysenter_4)

INTERN_ENTRY(libc_sysenter_5)
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	movl  24(%esp), %edx
	movl  28(%esp), %esi
	movl  32(%esp), %edi
	call  libc_sysenter_0
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	ret
SYMEND(libc_sysenter_5)

INTERN_ENTRY(libc_sysenter_6)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	movl  36(%esp), %edi
	movl  40(%esp), %ebp
	call  libc_sysenter_0
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	popl_cfi_r %ebp
	ret
SYMEND(libc_sysenter_6)

INTERN_ENTRY(libc_sysenter_errno_0)
	call libc_sysenter_0
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
	.cfi_restore %eip
SYMEND(libc_sysenter_errno_0)

INTERN_ENTRY(libc_sysenter_errno_1)
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	call  libc_sysenter_0
	popl_cfi_r %ebx
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_1)

INTERN_ENTRY(libc_sysenter_errno_2)
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	movl  12(%esp), %ecx
	call  libc_sysenter_0
	popl_cfi_r %ebx
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_2)

INTERN_ENTRY(libc_sysenter_errno_3)
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	movl  12(%esp), %ecx
	movl  16(%esp), %edx
	call  libc_sysenter_0
	popl_cfi_r %ebx
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_3)

INTERN_ENTRY(libc_sysenter_errno_4)
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  12(%esp), %ebx
	movl  16(%esp), %ecx
	movl  20(%esp), %edx
	movl  24(%esp), %esi
	call  libc_sysenter_0
	popl_cfi_r %ebx
	popl_cfi_r %esi
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_4)

INTERN_ENTRY(libc_sysenter_errno_5)
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	movl  24(%esp), %edx
	movl  28(%esp), %esi
	movl  32(%esp), %edi
	call  libc_sysenter_0
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_5)

INTERN_ENTRY(libc_sysenter_errno_6)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	movl  36(%esp), %edi
	movl  40(%esp), %ebp
	call  libc_sysenter_0
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	popl_cfi_r %ebp
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_6)

#elif defined(CONFIG_USE_SYSENTER)

/* sysenter version */
.section .text.hot
INTERN_ENTRY(x86_sysenter)
	.cfi_startproc
	popl  %edi       /* EIP */
	.cfi_adjust_cfa_offset -4
	.cfi_register %eip, %edi
	movl  %esp, %ebp /* ESP */
	sysenter
	.cfi_endproc
SYMEND(x86_sysenter)

.cfi_startproc
INTERN_ENTRY(libc_sysenter_0)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	call  x86_sysenter
	popl_cfi_r %edi
	popl_cfi_r %ebp
	ret
SYMEND(libc_sysenter_0)

INTERN_ENTRY(libc_sysenter_1)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	call  x86_sysenter
	popl_cfi_r %ebx
	popl_cfi_r %edi
	popl_cfi_r %ebp
	ret
SYMEND(libc_sysenter_1)

INTERN_ENTRY(libc_sysenter_2)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	call  x86_sysenter
	popl_cfi_r %ebx
	popl_cfi_r %edi
	popl_cfi_r %ebp
	ret
SYMEND(libc_sysenter_2)

INTERN_ENTRY(libc_sysenter_3)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	movl  24(%esp), %edx
	call  x86_sysenter
	popl_cfi_r %ebx
	popl_cfi_r %edi
	popl_cfi_r %ebp
	ret
SYMEND(libc_sysenter_3)

INTERN_ENTRY(libc_sysenter_4)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	call  x86_sysenter
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	popl_cfi_r %ebp
	ret
SYMEND(libc_sysenter_4)

INTERN_ENTRY(libc_sysenter_5)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	pushl_cfi 36(%esp) /* ARG #4 */
	call  x86_sysenter
	addl  $4, %esp
	.cfi_adjust_cfa_offset -4
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	popl_cfi_r %ebp
	ret
SYMEND(libc_sysenter_5)

INTERN_ENTRY(libc_sysenter_6)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	pushl_cfi 36(%esp) /* ARG #4 */
	pushl_cfi 40(%esp) /* ARG #5 */
	call  x86_sysenter
	addl  $8, %esp
	.cfi_adjust_cfa_offset -8
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	popl_cfi_r %ebp
	ret
SYMEND(libc_sysenter_6)

INTERN_ENTRY(libc_sysenter_errno_0)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	call  x86_sysenter
	popl_cfi_r %edi
	popl_cfi_r %ebp
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_0)

INTERN_ENTRY(libc_sysenter_errno_1)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	call  x86_sysenter
	popl_cfi_r %ebx
	popl_cfi_r %edi
	popl_cfi_r %ebp
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_1)

INTERN_ENTRY(libc_sysenter_errno_2)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	call  x86_sysenter
	popl_cfi_r %ebx
	popl_cfi_r %edi
	popl_cfi_r %ebp
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_2)

INTERN_ENTRY(libc_sysenter_errno_3)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	movl  24(%esp), %edx
	call  x86_sysenter
	popl_cfi_r %ebx
	popl_cfi_r %edi
	popl_cfi_r %ebp
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_3)

INTERN_ENTRY(libc_sysenter_errno_4)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	call  x86_sysenter
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	popl_cfi_r %ebp
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_4)

INTERN_ENTRY(libc_sysenter_errno_5)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	pushl_cfi 36(%esp) /* ARG #4 */
	call  x86_sysenter
	addl  $4, %esp
	.cfi_adjust_cfa_offset -4
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	popl_cfi_r %ebp
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_5)

INTERN_ENTRY(libc_sysenter_errno_6)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	pushl_cfi 36(%esp) /* ARG #4 */
	pushl_cfi 40(%esp) /* ARG #5 */
	call  x86_sysenter
	addl  $8, %esp
	.cfi_adjust_cfa_offset -8
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	popl_cfi_r %ebp
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_6)

#elif defined(CONFIG_USE_INT80)

/* int $0x80 version */
.section .text.hot
.cfi_startproc
INTERN_ENTRY(libc_sysenter_0)
	int   $0x80
	ret
SYMEND(libc_sysenter_0)

INTERN_ENTRY(libc_sysenter_1)
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	int   $0x80
	popl_cfi_r %ebx
	ret
SYMEND(libc_sysenter_1)

INTERN_ENTRY(libc_sysenter_2)
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	movl  12(%esp), %ecx
	int   $0x80
	popl_cfi_r %ebx
	ret
SYMEND(libc_sysenter_2)

INTERN_ENTRY(libc_sysenter_3)
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	movl  12(%esp), %ecx
	movl  16(%esp), %edx
	int   $0x80
	popl_cfi_r %ebx
	ret
SYMEND(libc_sysenter_3)

INTERN_ENTRY(libc_sysenter_4)
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  12(%esp), %ebx
	movl  16(%esp), %ecx
	movl  20(%esp), %edx
	movl  24(%esp), %esi
	int   $0x80
	popl_cfi_r %ebx
	popl_cfi_r %esi
	ret
SYMEND(libc_sysenter_4)

INTERN_ENTRY(libc_sysenter_5)
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	movl  24(%esp), %edx
	movl  28(%esp), %esi
	movl  32(%esp), %edi
	int   $0x80
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	ret
SYMEND(libc_sysenter_5)

INTERN_ENTRY(libc_sysenter_6)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	movl  36(%esp), %edi
	movl  40(%esp), %ebp
	int   $0x80
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	popl_cfi_r %ebp
	ret
SYMEND(libc_sysenter_6)


INTERN_ENTRY(libc_sysenter_errno_0)
	int   $0x80
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_0)

INTERN_ENTRY(libc_sysenter_errno_1)
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	int   $0x80
	popl_cfi_r %ebx
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_1)

INTERN_ENTRY(libc_sysenter_errno_2)
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	movl  12(%esp), %ecx
	int   $0x80
	popl_cfi_r %ebx
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_2)

INTERN_ENTRY(libc_sysenter_errno_3)
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	movl  12(%esp), %ecx
	movl  16(%esp), %edx
	int   $0x80
	popl_cfi_r %ebx
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_3)

INTERN_ENTRY(libc_sysenter_errno_4)
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  12(%esp), %ebx
	movl  16(%esp), %ecx
	movl  20(%esp), %edx
	movl  24(%esp), %esi
	int   $0x80
	popl_cfi_r %ebx
	popl_cfi_r %esi
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_4)

INTERN_ENTRY(libc_sysenter_errno_5)
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	movl  24(%esp), %edx
	movl  28(%esp), %esi
	movl  32(%esp), %edi
	int   $0x80
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_5)

INTERN_ENTRY(libc_sysenter_errno_6)
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	movl  36(%esp), %edi
	movl  40(%esp), %ebp
	int   $0x80
	popl_cfi_r %ebx
	popl_cfi_r %esi
	popl_cfi_r %edi
	popl_cfi_r %ebp
	SYSCALL_ERRNO_TAIL(.syscall_errno)
	ret
SYMEND(libc_sysenter_errno_6)
#else
#error "No system call mechanism defined"
#endif

#ifdef CONFIG_USE_USHARE_LAZY_SYSCALLS
.cfi_startproc
#endif


#ifndef ENCODE_SYSNO
#define ENCODE_SYSNO(x) x
#endif

#ifdef CONFIG_USE_USHARE_LAZY_SYSCALLS
#define SYSCALL_WRAPPER_INVOKE \
	.byte  0xe9; /* jmp --- */ \
	.long  0; \
	call  x86_syscall_load_\argc
#define SYSCALL_WRAPPER_INVOKE_ERRNO \
	.byte  0xe9; /* jmp --- */ \
	.long  0; \
	call  x86_syscall_load_errno_\argc
#else
#define SYSCALL_WRAPPER_INVOKE \
	jmp   libc_sysenter_\argc
#define SYSCALL_WRAPPER_INVOKE_ERRNO \
	jmp   libc_sysenter_errno_\argc
#endif


#define F      0x0010 /* export \name  = sys_\name;                    define libc_\name  = sys_\name; */
#define E      0x0020 /* export \name  = ERRNO_ENABLED(sys_\name);     define libc_\name  = ERRNO_ENABLED(sys_\name); */
#define X_     0x0040 /* export X\name = EXCEPTION_ENABLED(sys_\name); define libc_X\name = EXCEPTION_ENABLED(sys_\name); */
#define sys    0x0100 /* define sys_\name = sys_\name; */
#define Xsys   0x0200 /* define Xsys_\name = EXCEPTION_ENABLED(sys_\name); */
#define Esys   0x0400 /* define Xsys_\name = EXCEPTION_ENABLED(sys_\name); */


/* System call wrappers. */
.macro define_wrapper name, nr, argc, flags = (sys | Xsys)
.if (\flags) & (F | sys)
	.if (\flags) & F
		DEFINE_INTERN_ALIAS(libc_\name,sys_\name);
		EXPORT(\name,sys_\name)
	.endif
	.align 16
	INTERN_ENTRY(sys_\name)
		movl  $(ENCODE_SYSNO(\nr)), %eax
		SYSCALL_WRAPPER_INVOKE
	SYMEND(sys_\name)
.endif
.if (\flags) & (X_ | Xsys)
	.if (\flags) & X_
		DEFINE_INTERN_ALIAS(libc_X\name,Xsys_\name)
		EXPORT(X\name,Xsys_\name)
	.endif
	.align 16
	INTERN_ENTRY(Xsys_\name)
		movl  $(ENCODE_SYSNO(0x80000000 | \nr)), %eax
		SYSCALL_WRAPPER_INVOKE
	SYMEND(Xsys_\name)
.endif
.if (\flags) & (E | Esys)
	.if (\flags) & E
		DEFINE_INTERN_ALIAS(libc_\name,Esys_\name)
		EXPORT(\name,Esys_\name)
	.endif
	.align 16
	INTERN_ENTRY(Esys_\name)
		movl  $(ENCODE_SYSNO(\nr)), %eax
		SYSCALL_WRAPPER_INVOKE_ERRNO
	SYMEND(Esys_\name)
.endif
.endm

#define X X_

#define DEFINE_SYSCALL(name,nargs,flags...) \
	define_wrapper name, SYS_##name, nargs, ## flags

DEFINE_SYSCALL(openat,4,     E|X)
EXPORT(openat64,libc_openat)
EXPORT(Xopenat64,libc_Xopenat)

DEFINE_SYSCALL(close,1,      E|sys)
EXPORT_STRONG(__close,libc_close)

DEFINE_SYSCALL(exit,1,       sys)

DEFINE_SYSCALL(exit_group,1, sys)
DEFINE_INTERN_ALIAS(_libc_exit,sys_exit_group)
EXPORT(_exit,_libc_exit)
EXPORT(_Exit,_libc_exit)

DEFINE_SYSCALL(unshare,1,    E|X)
DEFINE_SYSCALL(munmap,2,     E|X|sys)
DEFINE_SYSCALL(mremap,5,     E|X)
DEFINE_SYSCALL(mmap,6,       E|X)

DEFINE_SYSCALL(read,3,       E|X)
DEFINE_INTERN_ALIAS(libd_read,libc_read)
EXPORT(__DSYM(_read),libd_read)
EXPORT_STRONG(__read,libc_read)

DEFINE_SYSCALL(write,3,      E|X)
DEFINE_INTERN_ALIAS(libd_write,libc_write)
EXPORT(__DSYM(_write),libd_write)
EXPORT_STRONG(__write,libc_write)

DEFINE_SYSCALL(faccessat,4,  E|X)
DEFINE_SYSCALL(chdir,1,      E|X)
DEFINE_SYSCALL(chroot,1,     E|X)
DEFINE_SYSCALL(fchdir,1,     E|X)
DEFINE_SYSCALL(fchmod,2,     E|X)
DEFINE_SYSCALL(fchown,3,     E|X)
DEFINE_SYSCALL(fchmodat,4,   E|X)
DEFINE_SYSCALL(fchownat,5,   E|X)
DEFINE_SYSCALL(dup,1,        E|X)

DEFINE_SYSCALL(dup2,2,       E|X)
EXPORT_STRONG(__dup2,libc_dup2)

DEFINE_SYSCALL(dup3,3,       E|X)

DEFINE_SYSCALL(fcntl,3,      E|X)
EXPORT_STRONG(__fcntl,libc_fcntl)

DEFINE_SYSCALL(ioctl,3,      E|X)
DEFINE_SYSCALL(execve,3,     E|X)
DEFINE_SYSCALL(sched_yield,0,F|sys)

DEFINE_SYSCALL(fork,0,       E|X)
DEFINE_INTERN_ALIAS(libc_vfork,libc_fork)
DEFINE_INTERN_ALIAS(libc_Xvfork,libc_Xfork)
EXPORT(vfork,libc_vfork)
EXPORT(Xvfork,libc_Xvfork)

DEFINE_SYSCALL(sync,0,       E|X)
DEFINE_SYSCALL(fsync,1,      E|X)
DEFINE_SYSCALL(fdatasync,1,  E|X)
DEFINE_SYSCALL(syncfs,1,     E|X)

DEFINE_SYSCALL(fstat64,2,    Esys|Xsys)
DEFINE_SYSCALL(fstatat64,4,  Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_kfstat64,Esys_fstat64)
DEFINE_INTERN_ALIAS(libc_Xkfstat64,Xsys_fstat64)
DEFINE_INTERN_ALIAS(libc_kfstatat64,Esys_fstatat64)
DEFINE_INTERN_ALIAS(libc_Xkfstatat64,Xsys_fstatat64)
DEFINE_INTERN_ALIAS(libc_kfstat,libc_kfstat64)
DEFINE_INTERN_ALIAS(libc_Xkfstat,libc_Xkfstat64)
DEFINE_INTERN_ALIAS(libc_kfstatat,libc_kfstatat64)
DEFINE_INTERN_ALIAS(libc_Xkfstatat,libc_Xkfstatat64)
EXPORT(kfstat64,libc_kfstat64)
EXPORT(Xkfstat64,libc_Xkfstat64)
EXPORT(kfstatat64,libc_kfstatat64)
EXPORT(Xkfstatat64,libc_Xkfstatat64)
EXPORT(kfstat,libc_kfstat)
EXPORT(Xkfstat,libc_Xkfstat)
EXPORT(kfstatat,libc_kfstatat)
EXPORT(Xkfstatat,libc_Xkfstatat)

DEFINE_SYSCALL(readlinkat,4, E|X)
DEFINE_SYSCALL(mknodat,4,    E|X)
DEFINE_SYSCALL(mkdirat,3,    E|X)
DEFINE_SYSCALL(unlinkat,3,   E|X)
DEFINE_SYSCALL(symlinkat,3,  E|X)
DEFINE_SYSCALL(linkat,5,     E|X)
DEFINE_SYSCALL(renameat,4,   E|X)

DEFINE_SYSCALL(utimensat,4,  Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_utimensat64,Esys_utimensat)
DEFINE_INTERN_ALIAS(libc_Xutimensat64,Xsys_utimensat)
EXPORT(utimensat64,libc_utimensat64)
EXPORT(Xutimensat64,libc_Xutimensat64)

DEFINE_SYSCALL(getpid,0,     sys)
DEFINE_INTERN_ALIAS(libc_getpid,sys_getpid)
EXPORT(getpid,libc_getpid)
EXPORT(__DSYM(_getpid),libc_getpid);
EXPORT_STRONG(__getpid,libc_getpid);

DEFINE_SYSCALL(getppid,0,    sys)
DEFINE_INTERN_ALIAS(libc_getppid,sys_getppid)
EXPORT(getppid,libc_getppid)

DEFINE_SYSCALL(gettid,0,     sys)
/* gettid() is implemented using the task segment. */
//DEFINE_INTERN_ALIAS(libc_gettid,sys_gettid)
//EXPORT(gettid,libc_gettid)

DEFINE_SYSCALL(getpgid,1,    E|X)
EXPORT_STRONG(__getpgid,libc_getpgid)

DEFINE_SYSCALL(setpgid,2,    E|X)
EXPORT_STRONG(__setpgid,libc_setpgid)

DEFINE_SYSCALL(getsid,1,     E|X)
DEFINE_SYSCALL(setsid,0,     E|X)

DEFINE_SYSCALL(sethostname,2,E|X)
DEFINE_SYSCALL(setdomainname,2,E|X)

DEFINE_SYSCALL(sigaction,4,  Esys|Xsys)
DEFINE_SYSCALL(sigprocmask,4,Esys|Xsys)
DEFINE_SYSCALL(sigtimedwait,4,Esys|Xsys)
DEFINE_SYSCALL(sigpending,2, Esys|Xsys)
DEFINE_SYSCALL(sigsuspend,2, Esys|Xsys)

DEFINE_SYSCALL(kill,2,       E|X)
DEFINE_SYSCALL(tkill,2,      E|X)
DEFINE_SYSCALL(tgkill,3,     E|X)

//DEFINE_SYSCALL(getcwd,2) /* Libc implements this using sys_xfrealpathat() */
DEFINE_SYSCALL(waitid,5,     Esys|Xsys)

DEFINE_SYSCALL(wait4,4,      E|X)

DEFINE_SYSCALL(pselect6,6,   Esys|Xsys)
DEFINE_SYSCALL(ppoll,5,      Esys|Xsys)
DEFINE_SYSCALL(xppoll,6,     Esys|Xsys)

DEFINE_SYSCALL(pipe,1,       E|X)
DEFINE_SYSCALL(pipe2,2,      E|X)

DEFINE_SYSCALL(mount,5,      E|X)
DEFINE_SYSCALL(umount2,2,    E|X)

DEFINE_SYSCALL(getcpu,2)

DEFINE_SYSCALL(gettimeofday,2,Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_gettimeofday64,Esys_gettimeofday)
DEFINE_INTERN_ALIAS(libc_Xgettimeofday64,Xsys_gettimeofday)
EXPORT(gettimeofday64,libc_gettimeofday64)
EXPORT(Xgettimeofday64,libc_Xgettimeofday64)

DEFINE_SYSCALL(settimeofday,2,Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_settimeofday64,Esys_settimeofday)
DEFINE_INTERN_ALIAS(libc_Xsettimeofday64,Xsys_settimeofday)
EXPORT(settimeofday64,libc_settimeofday64)
EXPORT(Xsettimeofday64,libc_Xsettimeofday64)

DEFINE_SYSCALL(nanosleep,2,  Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_nanosleep64,Esys_nanosleep)
DEFINE_INTERN_ALIAS(libc_Xnanosleep64,Xsys_nanosleep)
EXPORT(nanosleep64,libc_nanosleep64)
EXPORT(Xnanosleep64,libc_Xnanosleep64)

DEFINE_SYSCALL(umask,1,      sys)
DEFINE_INTERN_ALIAS(libc_umask,sys_umask)
EXPORT(__KSYM(umask),libc_umask)
EXPORT(__DSYM(_umask),libc_umask) /* DOS Alias */

DEFINE_SYSCALL(mprotect,3,   E|X)
DEFINE_SYSCALL(swapon,2,     E|X)
DEFINE_SYSCALL(swapoff,1,    E|X)

DEFINE_SYSCALL(futex,6,      Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_futex64,Esys_futex)
DEFINE_INTERN_ALIAS(libc_Xfutex64,Xsys_futex)
EXPORT(futex64,libc_futex64)
EXPORT(Xfutex64,libc_Xfutex64)


DEFINE_SYSCALL(sysinfo,1,    E|X)
DEFINE_SYSCALL(sigaltstack,2,E|X)

DEFINE_SYSCALL(set_tid_address,1,sys)

DEFINE_SYSCALL(rt_sigqueueinfo,3,Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_sigqueueinfo,Esys_rt_sigqueueinfo)
DEFINE_INTERN_ALIAS(libc_Xsigqueueinfo,Xsys_rt_sigqueueinfo)
EXPORT(sigqueueinfo,libc_sigqueueinfo)
EXPORT(Xsigqueueinfo,libc_Xsigqueueinfo)

DEFINE_SYSCALL(rt_tgsigqueueinfo,4,Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_tgsigqueueinfo,Esys_rt_tgsigqueueinfo)
DEFINE_INTERN_ALIAS(libc_Xtgsigqueueinfo,Xsys_rt_tgsigqueueinfo)
EXPORT(tgsigqueueinfo,libc_tgsigqueueinfo)
EXPORT(Xtgsigqueueinfo,libc_Xtgsigqueueinfo)

DEFINE_SYSCALL(clone,5,         Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_xclone,Esys_clone)
DEFINE_INTERN_ALIAS(libc_Xxclone,Xsys_clone)
EXPORT(xclone,libc_xclone)
EXPORT(Xxclone,libc_Xxclone)


DEFINE_SYSCALL(execveat,5,      Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_fexecveat,Esys_execveat)
DEFINE_INTERN_ALIAS(libc_Xfexecveat,Xsys_execveat)
EXPORT(fexecveat,libc_fexecveat)
EXPORT(Xfexecveat,libc_Xfexecveat)

DEFINE_SYSCALL(xreaddir,4,      E|X)
DEFINE_INTERN_ALIAS(libc_xreaddir64,libc_xreaddir)
DEFINE_INTERN_ALIAS(libc_Xxreaddir64,libc_Xxreaddir)
EXPORT(xreaddir64,libc_xreaddir64)
EXPORT(Xxreaddir64,libc_Xxreaddir64)

DEFINE_SYSCALL(xopenpty,3,      sys|Xsys)

DEFINE_SYSCALL(xfrealpathat,6,  Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_xfrealpathat2,Esys_xfrealpathat)
DEFINE_INTERN_ALIAS(libc_Xxfrealpathat2,Xsys_xfrealpathat)
EXPORT(xfrealpathat2,libc_xfrealpathat2)
EXPORT(Xxfrealpathat2,libc_Xxfrealpathat2)

DEFINE_SYSCALL(xpipe,1)
DEFINE_SYSCALL(xsyslog,3,    sys)
DEFINE_SYSCALL(xmmap,2,      Esys|Xsys)
DEFINE_SYSCALL(xmunmap,4,    E|X)
DEFINE_SYSCALL(xmprotect,6,  E|X)

DEFINE_SYSCALL(xfchdirat,3,  Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_fchdirat,Esys_xfchdirat)
DEFINE_INTERN_ALIAS(libc_Xfchdirat,Xsys_xfchdirat)
EXPORT(fchdirat,libc_fchdirat)
EXPORT(Xfchdirat,libc_Xfchdirat)

DEFINE_SYSCALL(xfrenameat,5, Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_frenameat,Esys_xfrenameat)
DEFINE_INTERN_ALIAS(libc_Xfrenameat,Xsys_xfrenameat)
EXPORT(frenameat,libc_frenameat)
EXPORT(Xfrenameat,libc_Xfrenameat)

DEFINE_SYSCALL(xfdlopenat,5, sys|X)
DEFINE_SYSCALL(xdlsym,2,     sys|X)
DEFINE_SYSCALL(xdlfini,0,    sys)

DEFINE_SYSCALL(xdlclose,1,   E|X)
DEFINE_INTERN_ALIAS(libc_unloaddll,libc_xdlclose)
EXPORT(_unloaddll,libc_unloaddll)
EXPORT_STRONG(__libc_dlclose,libc_xdlclose)

DEFINE_SYSCALL(xdlmodule_info,4,E|X)

DEFINE_SYSCALL(xfsymlinkat,4,Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_fsymlinkat,Esys_xfsymlinkat)
DEFINE_INTERN_ALIAS(libc_Xfsymlinkat,Xsys_xfsymlinkat)
EXPORT(fsymlinkat,libc_fsymlinkat)
EXPORT(Xfsymlinkat,libc_Xfsymlinkat)

DEFINE_SYSCALL(xfreadlinkat,5,Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_freadlinkat,Esys_xfreadlinkat)
DEFINE_INTERN_ALIAS(libc_Xfreadlinkat,Xsys_xfreadlinkat)
EXPORT(freadlinkat,libc_freadlinkat)
EXPORT(Xfreadlinkat,libc_Xfreadlinkat)

DEFINE_SYSCALL(xfmknodat,5, Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_fmknodat,Esys_xfmknodat)
DEFINE_INTERN_ALIAS(libc_Xfmknodat,Xsys_xfmknodat)
EXPORT(fmknodat,libc_fmknodat)
EXPORT(Xfmknodat,libc_Xfmknodat)

DEFINE_SYSCALL(xfmkdirat,4, Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_fmkdirat,Esys_xfmkdirat)
DEFINE_INTERN_ALIAS(libc_Xfmkdirat,Xsys_xfmkdirat)
EXPORT(fmkdirat,libc_fmkdirat)
EXPORT(Xfmkdirat,libc_Xfmkdirat)

DEFINE_SYSCALL(xgetdrives,0,sys)
DEFINE_INTERN_ALIAS(libd_getdrives,sys_xgetdrives)
EXPORT(__KSYM(getdrives),libd_getdrives)
EXPORT(__DSYM(_getdrives),libd_getdrives)

DEFINE_SYSCALL(xfpathconfat,4,Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_fpathconfat,Esys_xfpathconfat)
DEFINE_INTERN_ALIAS(libc_Xfpathconfat,Xsys_xfpathconfat)
EXPORT(fpathconfat,libc_fpathconfat)
EXPORT(Xfpathconfat,libc_Xfpathconfat)

DEFINE_SYSCALL(xsysconf,1,   Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_sysconf,Esys_xsysconf)
DEFINE_INTERN_ALIAS(libc_Xsysconf,Xsys_xsysconf)
EXPORT(sysconf,libc_sysconf)
EXPORT(Xsysconf,libc_Xsysconf)
EXPORT_STRONG(__sysconf,libc_sysconf)


DEFINE_SYSCALL(xreset_debug_data,3,sys)
DEFINE_SYSCALL(xfind_modified_address,3,sys)
DEFINE_SYSCALL(xgc_search,4, sys)
DEFINE_SYSCALL(xcapture_traceback,4,sys)
DEFINE_SYSCALL(xreadf,4,     E|X)
DEFINE_SYSCALL(xwritef,4,    E|X)

DEFINE_SYSCALL(xreaddirf,5,  E|X)
DEFINE_INTERN_ALIAS(libc_xreaddirf64,libc_xreaddirf)
DEFINE_INTERN_ALIAS(libc_Xxreaddirf64,libc_Xxreaddirf)
EXPORT(xreaddirf64,libc_xreaddirf64)
EXPORT(Xxreaddirf64,libc_Xxreaddirf64)

DEFINE_SYSCALL(xioctlf,4,    E|X)
DEFINE_SYSCALL(xunwind_except,3,sys|Xsys)
DEFINE_SYSCALL(xunwind,4,    sys|Xsys)
DEFINE_SYSCALL(xaddr2line,3, Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_xdladdr2line,Esys_xaddr2line)
DEFINE_INTERN_ALIAS(libc_Xxdladdr2line,Xsys_xaddr2line)
EXPORT(xdladdr2line,libc_xdladdr2line)
EXPORT(Xxdladdr2line,libc_Xxdladdr2line)

/* Socket API */
DEFINE_SYSCALL(socket,3,     E|X)
DEFINE_SYSCALL(bind,3,       E|X)
DEFINE_SYSCALL(listen,2,     E|X)
DEFINE_SYSCALL(accept4,4,    E|X)
DEFINE_SYSCALL(accept,3,     E|X)
DEFINE_SYSCALL(connect,3,    E|X)
DEFINE_SYSCALL(xgetsockname,3,E|X)
DEFINE_SYSCALL(xgetpeername,3,E|X)
DEFINE_SYSCALL(getsockname,3,E|X)
DEFINE_SYSCALL(getpeername,3,E|X)
DEFINE_SYSCALL(sendmmsg,4,   E|X)
DEFINE_SYSCALL(sendmsg,3,    E|X)
DEFINE_SYSCALL(sendto,6,     E|X)
DEFINE_SYSCALL(send,4,       E|X)
DEFINE_SYSCALL(recvmsg,3,    E|X)
DEFINE_SYSCALL(recvfrom,6,   E|X)
DEFINE_SYSCALL(recv,4,       E|X)
DEFINE_SYSCALL(xgetsockopt,5,E|X)
DEFINE_SYSCALL(getsockopt,5, E|X)
DEFINE_SYSCALL(setsockopt,5, E|X)
DEFINE_SYSCALL(shutdown,2,   E)

#ifdef __ARCH_WIDE_64BIT_SYSCALL
#define ARGC64(c64,c32) c32
#else
#define ARGC64(c64,c32) c64
#endif

DEFINE_SYSCALL(lseek,ARGC64(3,4),Esys|Xsys|sys)
DEFINE_INTERN_ALIAS(libc_lseek64,Esys_lseek)
DEFINE_INTERN_ALIAS(libc_Xlseek64,Xsys_lseek)
EXPORT(lseek64,libc_lseek64)
EXPORT(Xlseek64,libc_Xlseek64)
EXPORT(__DSYM(_lseeki64),libc_lseek64)

DEFINE_SYSCALL(pread64,ARGC64(4,5),E|X)
EXPORT_STRONG(__pread64,libc_pread64)

DEFINE_SYSCALL(pwrite64,ARGC64(4,5),E|X)
EXPORT_STRONG(__pwrite64,libc_pwrite64)

DEFINE_SYSCALL(truncate,ARGC64(2,3),Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_truncate64,Esys_truncate)
DEFINE_INTERN_ALIAS(libc_Xtruncate64,Xsys_truncate)
EXPORT(truncate64,libc_truncate64)
EXPORT(Xtruncate64,libc_Xtruncate64)

DEFINE_SYSCALL(fallocate,ARGC64(4,5),Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_fallocate64,Esys_fallocate)
DEFINE_INTERN_ALIAS(libc_Xfallocate64,Xsys_fallocate)
EXPORT(fallocate64,libc_fallocate64)
EXPORT(Xfallocate64,libc_Xfallocate64)

DEFINE_SYSCALL(xfsmask,ARGC64(1,2),sys)

DEFINE_SYSCALL(ftruncate,ARGC64(2,3),Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_ftruncate64,Esys_ftruncate)
DEFINE_INTERN_ALIAS(libc_Xftruncate64,Xsys_ftruncate)
EXPORT(ftruncate64,libc_ftruncate64)
EXPORT(Xftruncate64,libc_Xftruncate64)
EXPORT(__DSYM(_chsize_s),libc_ftruncate64)

DEFINE_SYSCALL(xftruncateat,ARGC64(4,5),Esys|Xsys)
DEFINE_INTERN_ALIAS(libc_ftruncateat64,Esys_xftruncateat)
DEFINE_INTERN_ALIAS(libc_Xftruncateat64,Xsys_xftruncateat)
EXPORT(ftruncateat64,libc_ftruncateat64)
EXPORT(Xftruncateat64,libc_Xftruncateat64)

DEFINE_SYSCALL(xpreadf64,ARGC64(5,6),E|X)
DEFINE_SYSCALL(xpwritef64,ARGC64(5,6),E|X)

.cfi_endproc




EXPORT(syscall,libc_syscall)
EXPORT(lsyscall,libc_syscall)
EXPORT(Xsyscall,libc_Xsyscall)
EXPORT(Xlsyscall,libc_Xsyscall)






#ifdef CONFIG_USE_USHARE_LAZY_SYSCALLS
.section .data
#else
.section .text
#endif
INTERN_ENTRY(libc_sigreturn)
	movl   4(%esp),          %esp /* Load the signal frame. */
	movl   $(SYS_sigreturn), %eax /* Load the system call number. */
#ifdef CONFIG_USE_USHARE_LAZY_SYSCALLS
	.byte  0xe9 /* jmp --- */
	.long  0
	call  x86_syscall_load_0
#else
	jmp   libc_sysenter_0
#endif
SYMEND(libc_sigreturn)
EXPORT(sigreturn,libc_sigreturn)

