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
#include <asm/universal.h>
#include <errno.h>
#include <kos/ushare.h>
#include <kos/thread.h>
#include <kernel/vm.h>
#include <kernel/syscall.h>


.code32

/* Define 32-bit compatibility mode ushare system call segments. */
#define __WITHOUT_CFI /* XXX: How will we deal with this? */
#define Utaskseg               __ASM_USERTASK_SEGMENT_COMPAT
#define t_errno                USER_TASK_SEGMENT_COMPAT_OFFSETOF_ERRNO
#define t_eformat              USER_TASK_SEGMENT_COMPAT_OFFSETOF_EFORMAT
#define DATA_USHARE_SYSENTER_SECTION_NAME  .data.ushare.sysenter

#include "ushare32.S"

#undef __WITHOUT_CFI
#undef Utaskseg
#undef t_errno
#undef t_eformat
#undef DATA_USHARE_SYSENTER_SECTION_NAME

.code64

#define Utaskseg         __ASM_USERTASK_SEGMENT
#define t_errno          USER_TASK_SEGMENT_OFFSETOF_ERRNO
#define t_eformat        USER_TASK_SEGMENT_OFFSETOF_EFORMAT


#define SYSCALL_BEGIN  991:
#define SYSCALL_END    .space USHARE_X86_SYSCALL_ENTRY_STRIDE - (. - 991b)

.section .data.ushare.syscall
.align PAGESIZE
INTERN_ENTRY(x86_ushare_syscall)
.cfi_startproc

SYSCALL_BEGIN          /* SYSCALL_0 */
	/* USER: [] */
	/* HOST: [] */
	int   $0x80
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_1 */
	/* USER: [%rdi] */
	/* HOST: [%rdi] */
	int   $0x80
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_2 */
	/* USER: [%rdi,%rsi] */
	/* HOST: [%rdi,%rsi] */
	int   $0x80
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_3 */
	/* USER: [%rdi,%rsi,%rdx] */
	/* HOST: [%rdi,%rsi,%rdx] */
	int   $0x80
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_4 */
	/* USER: [%rdi,%rsi,%rdx,%rcx] */
	/* HOST: [%rdi,%rsi,%rdx,%r10] */
	movq  %rcx, %r10
	int   $0x80
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_5 */
	/* USER: [%rdi,%rsi,%rdx,%rcx,%r8] */
	/* HOST: [%rdi,%rsi,%rdx,%r10,%r8] */
	movq  %rcx, %r10
	int   $0x80
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_6 */
	/* USER: [%rdi,%rsi,%rdx,%rcx,%r8,%r9] */
	/* HOST: [%rdi,%rsi,%rdx,%r10,%r8,%r9] */
	movq  %rcx, %r10
	int   $0x80
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL(...) */
	/* USER: [%rdi,%rsi,%rdx,%rcx,%r8, %r9,8(%rsp)] */
	/* HOST: [%rax,%rdi,%rsi,%rdx,%r10,%r8,%r9    ] */
	movq  %rdi,    %rax
	movq  %rsi,    %rdi
	movq  %rdx,    %rsi
	movq  %rcx,    %rdx
	movq  %r8,     %r10
	movq  %r9,     %r8
	movq  8(%rsp), %r9
	int   $0x80
	ret
SYSCALL_END

#define SYSCALL_ERRNO_TAIL(error) \
	cmpq  $__ERRNO_THRESHOLD64, %rax; \
	jae   error

/* With builtin ERRNO support. */
SYSCALL_BEGIN          /* SYSCALL_0 */
	/* USER: [] */
	/* HOST: [] */
	int   $0x80
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
.syscall_int80_errno:
	negl  %eax
	movl  %eax, %Utaskseg:t_errno
	movb  $__X86_TASK_ERRNO_FKOS, %Utaskseg:t_eformat
	movq  $-1, %rax
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_1 */
	/* USER: [%rdi] */
	/* HOST: [%rdi] */
	int   $0x80
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_2 */
	/* USER: [%rdi,%rsi] */
	/* HOST: [%rdi,%rsi] */
	int   $0x80
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_3 */
	/* USER: [%rdi,%rsi,%rdx] */
	/* HOST: [%rdi,%rsi,%rdx] */
	int   $0x80
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_4 */
	/* USER: [%rdi,%rsi,%rdx,%rcx] */
	/* HOST: [%rdi,%rsi,%rdx,%r10] */
	movq  %rcx, %r10
	int   $0x80
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_5 */
	/* USER: [%rdi,%rsi,%rdx,%rcx,%r8] */
	/* HOST: [%rdi,%rsi,%rdx,%r10,%r8] */
	movq  %rcx, %r10
	int   $0x80
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_6 */
	/* USER: [%rdi,%rsi,%rdx,%rcx,%r8,%r9] */
	/* HOST: [%rdi,%rsi,%rdx,%r10,%r8,%r9] */
	movq  %rcx, %r10
	int   $0x80
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL(...) */
	/* USER: [%rdi,%rsi,%rdx,%rcx,%r8, %r9,8(%rsp)] */
	/* HOST: [%rax,%rdi,%rsi,%rdx,%r10,%r8,%r9    ] */
	movq  %rdi,    %rax
	movq  %rsi,    %rdi
	movq  %rdx,    %rsi
	movq  %rcx,    %rdx
	movq  %r8,     %r10
	movq  %r9,     %r8
	movq  8(%rsp), %r9
	int   $0x80
	SYSCALL_ERRNO_TAIL(.syscall_int80_errno)
	ret
SYSCALL_END

.cfi_endproc
SYMEND(x86_ushare_syscall)






/* Fast system calls in long mode. */
.section .text.free
INTERN_ENTRY(x86_fast_syscall)
.cfi_startproc

SYSCALL_BEGIN          /* SYSCALL_0 */
	/* USER: [] */
	/* HOST: [] */
	syscall
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_1 */
	/* USER: [%rdi] */
	/* HOST: [%rdi] */
	syscall
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_2 */
	/* USER: [%rdi,%rsi] */
	/* HOST: [%rdi,%rsi] */
	syscall
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_3 */
	/* USER: [%rdi,%rsi,%rdx] */
	/* HOST: [%rdi,%rsi,%rdx] */
	syscall
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_4 */
	/* USER: [%rdi,%rsi,%rdx,%rcx] */
	/* HOST: [%rdi,%rsi,%rdx,%r10] */
	movq  %rcx, %r10
	syscall
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_5 */
	/* USER: [%rdi,%rsi,%rdx,%rcx,%r8] */
	/* HOST: [%rdi,%rsi,%rdx,%r10,%r8] */
	movq  %rcx, %r10
	syscall
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_6 */
	/* USER: [%rdi,%rsi,%rdx,%rcx,%r8,%r9] */
	/* HOST: [%rdi,%rsi,%rdx,%r10,%r8,%r9] */
	movq  %rcx, %r10
	syscall
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL(...) */
	/* USER: [%rdi,%rsi,%rdx,%rcx,%r8, %r9,8(%rsp)] */
	/* HOST: [%rax,%rdi,%rsi,%rdx,%r10,%r8,%r9    ] */
	movq  %rdi,    %rax
	movq  %rsi,    %rdi
	movq  %rdx,    %rsi
	movq  %rcx,    %rdx
	movq  %r8,     %r10
	movq  %r9,     %r8
	movq  8(%rsp), %r9
	syscall
	ret
SYSCALL_END

#define SYSCALL_ERRNO_TAIL(error) \
	cmpq  $__ERRNO_THRESHOLD64, %rax; \
	jae   error

/* With builtin ERRNO support. */
SYSCALL_BEGIN          /* SYSCALL_0 */
	/* USER: [] */
	/* HOST: [] */
	syscall
	SYSCALL_ERRNO_TAIL(.syscall_syscall_errno)
	ret
.syscall_syscall_errno:
	negl  %eax
	movl  %eax, %Utaskseg:t_errno
	movb  $__X86_TASK_ERRNO_FKOS, %Utaskseg:t_eformat
	movq  $-1, %rax
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_1 */
	/* USER: [%rdi] */
	/* HOST: [%rdi] */
	syscall
	SYSCALL_ERRNO_TAIL(.syscall_syscall_errno)
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_2 */
	/* USER: [%rdi,%rsi] */
	/* HOST: [%rdi,%rsi] */
	syscall
	SYSCALL_ERRNO_TAIL(.syscall_syscall_errno)
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_3 */
	/* USER: [%rdi,%rsi,%rdx] */
	/* HOST: [%rdi,%rsi,%rdx] */
	syscall
	SYSCALL_ERRNO_TAIL(.syscall_syscall_errno)
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_4 */
	/* USER: [%rdi,%rsi,%rdx,%rcx] */
	/* HOST: [%rdi,%rsi,%rdx,%r10] */
	movq  %rcx, %r10
	syscall
	SYSCALL_ERRNO_TAIL(.syscall_syscall_errno)
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_5 */
	/* USER: [%rdi,%rsi,%rdx,%rcx,%r8] */
	/* HOST: [%rdi,%rsi,%rdx,%r10,%r8] */
	movq  %rcx, %r10
	syscall
	SYSCALL_ERRNO_TAIL(.syscall_syscall_errno)
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL_6 */
	/* USER: [%rdi,%rsi,%rdx,%rcx,%r8,%r9] */
	/* HOST: [%rdi,%rsi,%rdx,%r10,%r8,%r9] */
	movq  %rcx, %r10
	syscall
	SYSCALL_ERRNO_TAIL(.syscall_syscall_errno)
	ret
SYSCALL_END
SYSCALL_BEGIN          /* SYSCALL(...) */
	/* USER: [%rdi,%rsi,%rdx,%rcx,%r8, %r9,8(%rsp)] */
	/* HOST: [%rax,%rdi,%rsi,%rdx,%r10,%r8,%r9    ] */
	movq  %rdi,    %rax
	movq  %rsi,    %rdi
	movq  %rdx,    %rsi
	movq  %rcx,    %rdx
	movq  %r8,     %r10
	movq  %r9,     %r8
	movq  8(%rsp), %r9
	syscall
	SYSCALL_ERRNO_TAIL(.syscall_syscall_errno)
	ret
//SYSCALL_END
.cfi_endproc
INTERN_CONST(x86_fast_syscall_size,. - x86_fast_syscall)
SYMEND(x86_fast_syscall)
































