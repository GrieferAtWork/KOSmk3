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
#include <hybrid/limits.h>
#include <asm/cfi.h>
#include <errno.h>
#include <kos/ushare.h>
#include <kos/thread.h>
#include <kernel/vm.h>
#include <kernel/syscall.h>


#ifndef Utaskseg
#define Utaskseg  __ASM_USERTASK_SEGMENT
#define t_errno   USER_TASK_SEGMENT_OFFSETOF_ERRNO
#define t_eformat USER_TASK_SEGMENT_OFFSETOF_EFORMAT
#endif /* !Utaskseg */


#ifdef __WITHOUT_CFI
#define pushl_cfi_r   pushl
#define pushl_cfi     pushl
#define popl_cfi_r    popl
#define popl_cfi      popl
#define CFI_STARTPROC /* nothing */
#define CFI_ENDPROC   /* nothing */
#else
#define CFI_STARTPROC .cfi_startproc
#define CFI_ENDPROC   .cfi_endproc
#endif

#define SYSCALL_ERRNO_TAIL(error) \
	cmpl  $__ERRNO_THRESHOLD32, %eax; \
	jae   error

#ifdef DATA_USHARE_SYSENTER_SECTION_NAME
.section DATA_USHARE_SYSENTER_SECTION_NAME
#else
.section .data.ushare.sysenter
#endif
.align PAGESIZE
INTERN_ENTRY(x86_ushare_sysenter)
CFI_STARTPROC
1:	/* SYSCALL_0 */
	int   $0x80
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_1 */
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	int   $0x80
	popl_cfi_r  %ebx
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_2 */
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	movl  12(%esp), %ecx
	int   $0x80
	popl_cfi_r  %ebx
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_3 */
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	movl  12(%esp), %ecx
	movl  16(%esp), %edx
	int   $0x80
	popl_cfi_r  %ebx
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_4 */
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  12(%esp), %ebx
	movl  16(%esp), %ecx
	movl  20(%esp), %edx
	movl  24(%esp), %esi
	int   $0x80
	popl_cfi_r  %ebx
	popl_cfi_r  %esi
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_5 */
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	movl  24(%esp), %edx
	movl  28(%esp), %esi
	movl  32(%esp), %edi
	int   $0x80
	popl_cfi_r  %ebx
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_6 */
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
	popl_cfi_r  %ebx
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL */
	/* Save callee-save registers */
	pushl_cfi_r %ebx
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebp

	/* Load arguments */
#define   A  20 /* return+4*register (4+4*4) */
	movl  A+ 0(%esp), %eax /* sysno */
	movl  A+ 4(%esp), %ebx /* Arg #1 */
	movl  A+ 8(%esp), %ecx /* Arg #2 */
	movl  A+12(%esp), %edx /* Arg #3 */
	movl  A+16(%esp), %esi /* Arg #4 */
	movl  A+20(%esp), %edi /* Arg #5 */
	movl  A+24(%esp), %ebp /* Arg #6 */
#undef A

	int   $0x80 /* Invoke the system call interrupt vector */

	/* Restore callee-save registers */
	popl_cfi_r  %ebp
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	popl_cfi_r  %ebx
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_0_ERRNO */
	int   $0x80
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
.syscall_int80_errno:
	negl  %eax
	movl  %eax, %Utaskseg:t_errno
	movb  $__X86_TASK_ERRNO_FKOS, %Utaskseg:t_eformat
	movl  $-1, %eax
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_1_ERRNO */
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	int   $0x80
	popl_cfi_r  %ebx
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_2_ERRNO */
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	movl  12(%esp), %ecx
	int   $0x80
	popl_cfi_r  %ebx
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_3_ERRNO */
	pushl_cfi_r %ebx
	movl  8(%esp), %ebx
	movl  12(%esp), %ecx
	movl  16(%esp), %edx
	int   $0x80
	popl_cfi_r  %ebx
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_4_ERRNO */
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  12(%esp), %ebx
	movl  16(%esp), %ecx
	movl  20(%esp), %edx
	movl  24(%esp), %esi
	int   $0x80
	popl_cfi_r  %ebx
	popl_cfi_r  %esi
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_5_ERRNO */
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	movl  24(%esp), %edx
	movl  28(%esp), %esi
	movl  32(%esp), %edi
	int   $0x80
	popl_cfi_r  %ebx
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_6_ERRNO */
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
	popl_cfi_r  %ebx
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_ERRNO */
	/* Save callee-save registers */
	pushl_cfi_r %ebx
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	pushl_cfi_r %ebp

	/* Load arguments */
#define   A  20 /* return+4*register (4+4*4) */
	movl  A+ 0(%esp), %eax /* sysno */
	movl  A+ 4(%esp), %ebx /* Arg #1 */
	movl  A+ 8(%esp), %ecx /* Arg #2 */
	movl  A+12(%esp), %edx /* Arg #3 */
	movl  A+16(%esp), %esi /* Arg #4 */
	movl  A+20(%esp), %edi /* Arg #5 */
	movl  A+24(%esp), %ebp /* Arg #6 */
#undef A

	int   $0x80 /* Invoke the system call interrupt vector */

	/* Restore callee-save registers */
	popl_cfi_r  %ebp
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	popl_cfi_r  %ebx
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

CFI_ENDPROC
.space PAGESIZE - (. - x86_ushare_sysenter)
SYMEND(x86_ushare_sysenter)


.section .text.free
CFI_STARTPROC
INTERN_ENTRY(x86_fast_sysenter)
1:	/* SYSCALL_0 */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	call  x86_sysenter_common
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	ret

	/* Common sysenter stub for the fast_sysenter assembly functions below.
	 * This one's required because `sysenter' doesn't save the return address. */
x86_sysenter_common:
#ifndef __WITHOUT_CFI
	.cfi_remember_state
#endif
	popl_cfi  %edi   /* EIP */
#ifndef __WITHOUT_CFI
	.cfi_register %eip, %edi
#endif
	movl  %esp, %ebp /* ESP */
#ifndef __WITHOUT_CFI
	.cfi_def_cfa_register %ebp
#endif
	sysenter
#ifndef __WITHOUT_CFI
	.cfi_restore_state
#endif
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_1 */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	call  x86_sysenter_common
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_2 */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	call  x86_sysenter_common
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_3 */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	movl  24(%esp), %edx
	call  x86_sysenter_common
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_4 */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	pushl_cfi_r %esi
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	call  x86_sysenter_common
	popl_cfi_r  %esi
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_5 */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	pushl_cfi_r %esi
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	pushl_cfi 36(%esp) /* ARG #4 */
	call  x86_sysenter_common
	addl  $4, %esp
#ifndef __WITHOUT_CFI
	.cfi_adjust_cfa_offset -4
#endif
	popl_cfi_r  %esi
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_6 */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	pushl_cfi_r %esi
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	pushl_cfi 40(%esp) /* ARG #5 */
	pushl_cfi 40(%esp) /* ARG #4 */
	call  x86_sysenter_common
	addl  $8, %esp
#ifndef __WITHOUT_CFI
	.cfi_adjust_cfa_offset -8
#endif
	popl_cfi_r  %esi
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	pushl_cfi_r %esi
	movl      20(%esp), %eax /* REGNO */
	movl      24(%esp), %ebx /* ARG #0 */
	movl      28(%esp), %ecx /* ARG #1 */
	movl      32(%esp), %edx /* ARG #2 */
	movl      36(%esp), %esi /* ARG #3 */
	pushl_cfi 44(%esp)       /* ARG #5 */
	pushl_cfi 44(%esp)       /* ARG #4 */
	call  x86_sysenter_common
	addl  $8, %esp
#ifndef __WITHOUT_CFI
	.cfi_adjust_cfa_offset -8
#endif
	popl_cfi_r  %esi
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_0_ERRNO */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	call  x86_sysenter_common
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	SYSCALL_ERRNO_TAIL(.syscall_sysenter_errno)
	ret
.syscall_sysenter_errno:
	negl  %eax
	movl  %eax, %Utaskseg:t_errno
	movb  $__X86_TASK_ERRNO_FKOS, %Utaskseg:t_eformat
	movl  $-1, %eax
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_1_ERRNO */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	call  x86_sysenter_common
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	SYSCALL_ERRNO_TAIL(.syscall_sysenter_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_2_ERRNO */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	call  x86_sysenter_common
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	SYSCALL_ERRNO_TAIL(.syscall_sysenter_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_3_ERRNO */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	movl  16(%esp), %ebx
	movl  20(%esp), %ecx
	movl  24(%esp), %edx
	call  x86_sysenter_common
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	SYSCALL_ERRNO_TAIL(.syscall_sysenter_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_4_ERRNO */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	pushl_cfi_r %esi
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	call  x86_sysenter_common
	popl_cfi_r  %esi
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	SYSCALL_ERRNO_TAIL(.syscall_sysenter_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_5_ERRNO */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	pushl_cfi_r %esi
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	pushl_cfi 36(%esp) /* ARG #4 */
	call  x86_sysenter_common
	addl  $4, %esp
#ifndef __WITHOUT_CFI
	.cfi_adjust_cfa_offset -4
#endif
	popl_cfi_r  %esi
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	SYSCALL_ERRNO_TAIL(.syscall_sysenter_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_6_ERRNO */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	pushl_cfi_r %esi
	movl  20(%esp), %ebx
	movl  24(%esp), %ecx
	movl  28(%esp), %edx
	movl  32(%esp), %esi
	pushl_cfi 40(%esp) /* ARG #5 */
	pushl_cfi 40(%esp) /* ARG #4 */
	call  x86_sysenter_common
	addl  $8, %esp
#ifndef __WITHOUT_CFI
	.cfi_adjust_cfa_offset -8
#endif
	popl_cfi_r  %esi
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	SYSCALL_ERRNO_TAIL(.syscall_sysenter_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

1:	/* SYSCALL_ERRNO */
	pushl_cfi_r %ebp
	pushl_cfi_r %edi
	pushl_cfi_r %ebx
	pushl_cfi_r %esi
	movl      20(%esp), %eax /* REGNO */
	movl      24(%esp), %ebx /* ARG #0 */
	movl      28(%esp), %ecx /* ARG #1 */
	movl      32(%esp), %edx /* ARG #2 */
	movl      36(%esp), %esi /* ARG #3 */
	pushl_cfi 44(%esp)       /* ARG #5 */
	pushl_cfi 44(%esp)       /* ARG #4 */
	call  x86_sysenter_common
	addl  $8, %esp
#ifndef __WITHOUT_CFI
	.cfi_adjust_cfa_offset -8
#endif
	popl_cfi_r  %esi
	popl_cfi_r  %ebx
	popl_cfi_r  %edi
	popl_cfi_r  %ebp
	SYSCALL_ERRNO_TAIL(.syscall_sysenter_errno)
	ret
.space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 1b)

	CFI_ENDPROC
INTERN_CONST(x86_fast_sysenter_size,. - x86_fast_sysenter)
SYMEND(x86_fast_sysenter)





#undef SYSCALL_ERRNO_TAIL


#ifdef __WITHOUT_CFI
#undef pushl_cfi_r
#undef pushl_cfi
#undef popl_cfi_r
#undef popl_cfi
#endif








