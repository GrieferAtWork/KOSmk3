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
#define DATA_USHARE_SYSENTER_SECTION_NAME  .data.ushare.sysenter_compat
#define x86_ushare_sysenter    x86_ushare_sysenter_compat
#define syscall_int80_errno    syscall_int80_errno_compat
#define x86_fast_sysenter      x86_fast_sysenter_compat
#define x86_sysenter_common    x86_sysenter_common_compat
#define syscall_sysenter_errno syscall_sysenter_errno_compat
#define x86_fast_sysenter_size x86_fast_sysenter_size_compat

#include "ushare32.S"

#undef __WITHOUT_CFI
#undef Utaskseg
#undef t_errno
#undef t_eformat
#undef DATA_USHARE_SYSENTER_SECTION_NAME
#undef x86_ushare_sysenter
#undef syscall_int80_errno
#undef x86_fast_sysenter
#undef x86_sysenter_common
#undef syscall_sysenter_errno
#undef x86_fast_sysenter_size

.code64

#define Utaskseg         __ASM_USERTASK_SEGMENT
#define t_errno          USER_TASK_SEGMENT_OFFSETOF_ERRNO
#define t_eformat        USER_TASK_SEGMENT_OFFSETOF_EFORMAT


#define SYSCALL_BEGIN  991:
#define SYSCALL_END    .space USHARE_X86_SYSCALL_SYSENTER_STRIDE - (. - 991b)

.section .data.ushare.sysenter
.align PAGESIZE
INTERN_ENTRY(x86_ushare_sysenter)
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
SYMEND(x86_ushare_sysenter)
































