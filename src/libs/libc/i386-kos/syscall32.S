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
INTERN_ENTRY(libc_syscall)
	.byte  0xe9 /* jmp --- */
	.long  0
	.byte  0xe8 /* pushl %eip;   (call 1f; 1:) */
	.long  0
	pushl  %eax
	call   x86_get_syscall_segment
	addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(7)), %eax
	jmp    .x86_syscall_load_common
SYMEND(libc_syscall)
SYMEND(libc_Xsyscall)

#else

.section .text
INTERN_ENTRY(libc_Xsyscall)
	/* Set the exception-support bit in the system call number
	 * Other than that, this is exactly the same as the regular syscall() function. */
	orl    $0x80000000, 4(%esp)
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
	ret
	.cfi_endproc
SYMEND(libc_syscall)
SYMEND(libc_Xsyscall)

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
#else
#error "No system call mechanism defined"
#endif

#ifdef CONFIG_USE_USHARE_LAZY_SYSCALLS
.cfi_startproc
#endif


#ifndef ENCODE_SYSNO
#define ENCODE_SYSNO(x) x
#endif

/* System call wrappers. */
.macro define_wrapper name, nr, argc, define_Xsys = 1
INTERN_ENTRY(sys_\name)
	movl  $(ENCODE_SYSNO(\nr)), %eax
#ifdef CONFIG_USE_USHARE_LAZY_SYSCALLS
	.byte  0xe9 /* jmp --- */
	.long  0
	call  x86_syscall_load_\argc
#else
	jmp   libc_sysenter_\argc
#endif
SYMEND(sys_\name)
.if \define_Xsys != 0
INTERN_ENTRY(Xsys_\name)
	movl  $(ENCODE_SYSNO(0x80000000 | \nr)), %eax
#ifdef CONFIG_USE_USHARE_LAZY_SYSCALLS
	.byte  0xe9 /* jmp --- */
	.long  0
	call  x86_syscall_load_\argc
#else
	jmp   libc_sysenter_\argc
#endif
SYMEND(Xsys_\name)
.endif
.endm

define_wrapper openat, SYS_openat, 4
define_wrapper close, SYS_close, 1
define_wrapper exit, SYS_exit, 1, 0
define_wrapper exit_group, SYS_exit_group, 1, 0
define_wrapper unshare, SYS_unshare, 1
define_wrapper munmap, SYS_munmap, 2
define_wrapper mremap, SYS_mremap, 5
define_wrapper mmap, SYS_mmap, 6
define_wrapper read, SYS_read, 3
define_wrapper write, SYS_write, 3
define_wrapper faccessat, SYS_faccessat, 4
define_wrapper chdir, SYS_chdir, 1
define_wrapper chroot, SYS_chroot, 1
define_wrapper fchdir, SYS_fchdir, 1
define_wrapper fchmod, SYS_fchmod, 2
define_wrapper fchown, SYS_fchown, 3
define_wrapper fchmodat, SYS_fchmodat, 4
define_wrapper fchownat, SYS_fchownat, 5
define_wrapper dup, SYS_dup, 1
define_wrapper dup2, SYS_dup2, 1
define_wrapper dup3, SYS_dup3, 3
define_wrapper fcntl, SYS_fcntl, 3
define_wrapper ioctl, SYS_ioctl, 3
define_wrapper execve, SYS_execve, 3
define_wrapper sched_yield, SYS_sched_yield, 0, 0
define_wrapper fork, SYS_fork, 0
define_wrapper sync, SYS_sync, 0
define_wrapper fsync, SYS_fsync, 1
define_wrapper fdatasync, SYS_fdatasync, 1
define_wrapper syncfs, SYS_syncfs, 1
define_wrapper fstat64, SYS_fstat64, 2
define_wrapper fstatat64, SYS_fstatat64, 4
define_wrapper readlinkat, SYS_readlinkat, 4
define_wrapper mknodat, SYS_mknodat, 4
define_wrapper mkdirat, SYS_mkdirat, 3
define_wrapper unlinkat, SYS_unlinkat, 3
define_wrapper symlinkat, SYS_symlinkat, 3
define_wrapper linkat, SYS_linkat, 5
define_wrapper renameat, SYS_renameat, 4
define_wrapper utimensat, SYS_utimensat, 4
define_wrapper getpid, SYS_getpid, 0, 0
define_wrapper getppid, SYS_getppid, 0, 0
define_wrapper gettid, SYS_gettid, 0, 0
define_wrapper getpgid, SYS_getpgid, 1
define_wrapper setpgid, SYS_setpgid, 2
define_wrapper getsid, SYS_getsid, 1
define_wrapper setsid, SYS_setsid, 0
define_wrapper sethostname, SYS_sethostname, 2
define_wrapper setdomainname, SYS_setdomainname, 2
define_wrapper sigaction, SYS_sigaction, 4
define_wrapper sigprocmask, SYS_sigprocmask, 4
define_wrapper sigtimedwait, SYS_sigtimedwait, 4
define_wrapper sigpending, SYS_sigpending, 2
define_wrapper sigsuspend, SYS_sigsuspend, 2
define_wrapper kill, SYS_kill, 2
define_wrapper tkill, SYS_tkill, 2
define_wrapper tgkill, SYS_tgkill, 3
define_wrapper getcwd, SYS_getcwd, 2
define_wrapper waitid, SYS_waitid, 5
define_wrapper wait4, SYS_wait4, 4
define_wrapper pselect6, SYS_pselect6, 6
define_wrapper ppoll, SYS_ppoll, 5
define_wrapper pipe, SYS_pipe, 1
define_wrapper pipe2, SYS_pipe2, 2
define_wrapper mount, SYS_mount, 5
define_wrapper umount2, SYS_umount2, 2
define_wrapper getcpu, SYS_getcpu, 2
define_wrapper gettimeofday, SYS_gettimeofday, 2
define_wrapper settimeofday, SYS_settimeofday, 2
define_wrapper nanosleep, SYS_nanosleep, 2
define_wrapper umask, SYS_umask, 1, 0
define_wrapper mprotect, SYS_mprotect, 3
define_wrapper swapon, SYS_swapon, 2
define_wrapper swapoff, SYS_swapoff, 1
define_wrapper futex, SYS_futex, 6
define_wrapper sysinfo, SYS_sysinfo, 1
define_wrapper sigaltstack, SYS_sigaltstack, 2
define_wrapper set_tid_address, SYS_set_tid_address, 1, 0
define_wrapper rt_sigqueueinfo, SYS_rt_sigqueueinfo, 3
define_wrapper rt_tgsigqueueinfo, SYS_rt_tgsigqueueinfo, 4
define_wrapper clone, SYS_clone, 5
define_wrapper execveat, SYS_execveat, 5
define_wrapper xreaddir, SYS_xreaddir, 4
define_wrapper xopenpty, SYS_xopenpty, 3
define_wrapper xfrealpathat, SYS_xfrealpathat, 6
define_wrapper xpipe, SYS_xpipe, 1
define_wrapper xsyslog, SYS_xsyslog, 3
define_wrapper xmmap, SYS_xmmap, 2
define_wrapper xmunmap, SYS_xmunmap, 4
define_wrapper xmprotect, SYS_xmprotect, 6
define_wrapper xfchdirat, SYS_xfchdirat, 3
define_wrapper xfrenameat, SYS_xfrenameat, 5
define_wrapper xfdlopenat, SYS_xfdlopenat, 5
define_wrapper xdlsym, SYS_xdlsym, 2
define_wrapper xdlfini, SYS_xdlfini, 0
define_wrapper xdlclose, SYS_xdlclose, 1
define_wrapper xdlmodule_info, SYS_xdlmodule_info, 4
define_wrapper xfsymlinkat, SYS_xfsymlinkat, 4
define_wrapper xfreadlinkat, SYS_xfreadlinkat, 5
define_wrapper xfmknodat, SYS_xfmknodat, 5
define_wrapper xfmkdirat, SYS_xfmkdirat, 4
define_wrapper xgetdrives, SYS_xgetdrives, 0, 0
define_wrapper xfpathconfat, SYS_xfpathconfat, 4
define_wrapper xsysconf, SYS_xsysconf, 1
define_wrapper xreset_debug_data, SYS_xreset_debug_data, 3
define_wrapper xfind_modified_address, SYS_xfind_modified_address, 3
define_wrapper xgc_search, SYS_xgc_search, 4
define_wrapper xcapture_traceback, SYS_xcapture_traceback, 4
define_wrapper xreadf, SYS_xreadf, 4
define_wrapper xwritef, SYS_xwritef, 4
define_wrapper xreaddirf, SYS_xreaddirf, 5
define_wrapper xioctlf, SYS_xioctlf, 4
define_wrapper xunwind_except, SYS_xunwind_except, 3, 0
define_wrapper xunwind, SYS_xunwind, 4
define_wrapper xaddr2line, SYS_xaddr2line, 3

/* Socket API */
define_wrapper socket, SYS_socket, 3
define_wrapper bind, SYS_bind, 3
define_wrapper listen, SYS_listen, 2
define_wrapper accept4, SYS_accept4, 4
define_wrapper accept, SYS_accept, 3
define_wrapper connect, SYS_connect, 3
define_wrapper xgetsockname, SYS_xgetsockname, 3
define_wrapper xgetpeername, SYS_xgetpeername, 3
define_wrapper getsockname, SYS_getsockname, 3
define_wrapper getpeername, SYS_getpeername, 3
define_wrapper sendto, SYS_sendto, 6
define_wrapper send, SYS_send, 4
define_wrapper recvfrom, SYS_recvfrom, 6
define_wrapper recv, SYS_recv, 4
define_wrapper xgetsockopt, SYS_xgetsockopt, 5
define_wrapper getsockopt, SYS_getsockopt, 5
define_wrapper setsockopt, SYS_setsockopt, 5
define_wrapper shutdown, SYS_shutdown, 2, 0

#ifdef __ARCH_WIDE_64BIT_SYSCALL
define_wrapper lseek, SYS_lseek, 4
define_wrapper pread64, SYS_pread64, 5
define_wrapper pwrite64, SYS_pwrite64, 5
define_wrapper truncate, SYS_truncate, 3
define_wrapper ftruncate, SYS_ftruncate, 3
define_wrapper fallocate, SYS_fallocate, 6
define_wrapper xfsmask, SYS_xfsmask, 2, 0
define_wrapper xftruncateat, SYS_xftruncateat, 5
define_wrapper xpreadf64, SYS_xpreadf64, 6
define_wrapper xpwritef64, SYS_xpwritef64, 6
#else
define_wrapper lseek, SYS_lseek, 3
define_wrapper pread64, SYS_pread64, 4
define_wrapper pwrite64, SYS_pwrite64, 4
define_wrapper truncate, SYS_truncate, 2
define_wrapper ftruncate, SYS_ftruncate, 2
define_wrapper fallocate, SYS_fallocate, 4
define_wrapper xfsmask, SYS_xfsmask, 1, 0
define_wrapper xftruncateat, SYS_xftruncateat, 4
define_wrapper xpreadf64, SYS_xpreadf64, 5
define_wrapper xpwritef64, SYS_xpwritef64, 5
#endif
.cfi_endproc







EXPORT(syscall,libc_syscall)
EXPORT(lsyscall,libc_syscall)
EXPORT(Xsyscall,libc_Xsyscall)
EXPORT(Xlsyscall,libc_Xsyscall)

/* To reduce overhead, direct link libc functions
 * that don't change anything against system calls. */
DEFINE_INTERN_ALIAS(libc_Xpipe,Xsys_pipe)
EXPORT(Xpipe,libc_Xpipe)

DEFINE_INTERN_ALIAS(libc_Xpipe2,Xsys_pipe2)
EXPORT(Xpipe2,libc_Xpipe2)

DEFINE_INTERN_ALIAS(libc_Xfsync,Xsys_fsync)
EXPORT(Xfsync,libc_Xfsync)

DEFINE_INTERN_ALIAS(libc_Xfdatasync,Xsys_fdatasync)
EXPORT(Xfdatasync,libc_Xfdatasync)

DEFINE_INTERN_ALIAS(libc_Xdup,Xsys_dup)
EXPORT(Xdup,libc_Xdup)

DEFINE_INTERN_ALIAS(libc_Xdup2,Xsys_dup2)
EXPORT(Xdup2,libc_Xdup2)

DEFINE_INTERN_ALIAS(libc_Xdup3,Xsys_dup3)
EXPORT(Xdup3,libc_Xdup3)

DEFINE_INTERN_ALIAS(libc_Xchdir,Xsys_chdir)
EXPORT(Xchdir,libc_Xchdir)

DEFINE_INTERN_ALIAS(libc_Xfchdirat,Xsys_xfchdirat)
EXPORT(Xfchdirat,libc_Xfchdirat)

DEFINE_INTERN_ALIAS(libc_Xfchdir,Xsys_fchdir)
EXPORT(Xfchdir,libc_Xfchdir)

DEFINE_INTERN_ALIAS(libc_Xxfrealpathat2,Xsys_xfrealpathat)
EXPORT(Xxfrealpathat2,libc_Xxfrealpathat2)

DEFINE_INTERN_ALIAS(libc_Xfpathconfat,Xsys_xfpathconfat)
EXPORT(Xfpathconfat,libc_Xfpathconfat)

DEFINE_INTERN_ALIAS(libc_Xfchown,Xsys_fchown)
EXPORT(Xfchown,libc_Xfchown)

DEFINE_INTERN_ALIAS(libc_Xfchownat,Xsys_fchownat)
EXPORT(Xfchownat,libc_Xfchownat)

DEFINE_INTERN_ALIAS(libc_Xfchmod,Xsys_fchmod)
EXPORT(Xfchmod,libc_Xfchmod)

DEFINE_INTERN_ALIAS(libc_Xfchmodat,Xsys_fchmodat)
EXPORT(Xfchmodat,libc_Xfchmodat)

DEFINE_INTERN_ALIAS(libc_Xlinkat,Xsys_linkat)
EXPORT(Xlinkat,libc_Xlinkat)

DEFINE_INTERN_ALIAS(libc_Xunlinkat,Xsys_unlinkat)
EXPORT(Xunlinkat,libc_Xunlinkat)

DEFINE_INTERN_ALIAS(libc_Xrenameat,Xsys_renameat)
EXPORT(Xrenameat,libc_Xrenameat)

DEFINE_INTERN_ALIAS(libc_Xfrenameat,Xsys_xfrenameat)
EXPORT(Xfrenameat,libc_Xfrenameat)

DEFINE_INTERN_ALIAS(libc_Xmkdirat,Xsys_mkdirat)
EXPORT(Xmkdirat,libc_Xmkdirat)

DEFINE_INTERN_ALIAS(libc_Xfmkdirat,Xsys_xfmkdirat)
EXPORT(Xfmkdirat,libc_Xfmkdirat)

DEFINE_INTERN_ALIAS(libc_Xsymlinkat,Xsys_symlinkat)
EXPORT(Xsymlinkat,libc_Xsymlinkat)

DEFINE_INTERN_ALIAS(libc_Xfsymlinkat,Xsys_xfsymlinkat)
EXPORT(Xfsymlinkat,libc_Xfsymlinkat)

DEFINE_INTERN_ALIAS(libc_Xmknodat,Xsys_mknodat)
EXPORT(Xmknodat,libc_Xmknodat)

DEFINE_INTERN_ALIAS(libc_Xfmknodat,Xsys_xfmknodat)
EXPORT(Xfmknodat,libc_Xfmknodat)

DEFINE_INTERN_ALIAS(libc_Xreadlinkat,Xsys_readlinkat)
EXPORT(Xreadlinkat,libc_Xreadlinkat)

DEFINE_INTERN_ALIAS(libc_Xfreadlinkat,Xsys_xfreadlinkat)
EXPORT(Xfreadlinkat,libc_Xfreadlinkat)

DEFINE_INTERN_ALIAS(libc_Xftruncate64,Xsys_ftruncate)
EXPORT(Xftruncate64,libc_Xftruncate64)

DEFINE_INTERN_ALIAS(libc_Xtruncate64,Xsys_truncate)
EXPORT(Xtruncate64,libc_Xtruncate64)

DEFINE_INTERN_ALIAS(libc_Xftruncateat64,Xsys_xftruncateat)
EXPORT(Xftruncateat64,libc_Xftruncateat64)

DEFINE_INTERN_ALIAS(libc_Xchroot,Xsys_chroot)
EXPORT(Xchroot,libc_Xchroot)

DEFINE_INTERN_ALIAS(libc_Xkfstat64,Xsys_fstat64)
DEFINE_INTERN_ALIAS(libc_Xkfstat,Xsys_fstat64)
EXPORT(Xkfstat64,libc_Xkfstat64)
EXPORT(Xkfstat,libc_Xkfstat)

DEFINE_INTERN_ALIAS(libc_Xkfstatat64,Xsys_fstatat64)
DEFINE_INTERN_ALIAS(libc_Xkfstatat,Xsys_fstatat64)
EXPORT(Xkfstatat64,libc_Xkfstatat64)
EXPORT(Xkfstatat,libc_Xkfstatat)

DEFINE_INTERN_ALIAS(libc_Xutimensat64,Xsys_utimensat)
EXPORT(Xutimensat64,libc_Xutimensat64)

DEFINE_INTERN_ALIAS(libc_Xopenat,Xsys_openat)
EXPORT(Xopenat,libc_Xopenat)
EXPORT(Xopenat64,libc_Xopenat)

DEFINE_INTERN_ALIAS(libc_Xfcntl,Xsys_fcntl)
EXPORT(Xfcntl,libc_Xfcntl)

DEFINE_INTERN_ALIAS(libc_Xioctl,Xsys_ioctl)
EXPORT(Xioctl,libc_Xioctl)

DEFINE_INTERN_ALIAS(libc_Xread,Xsys_read)
EXPORT(Xread,libc_Xread)

DEFINE_INTERN_ALIAS(libc_Xxreadf,Xsys_xreadf)
EXPORT(Xxreadf,libc_Xxreadf)

DEFINE_INTERN_ALIAS(libc_Xwrite,Xsys_write)
EXPORT(Xwrite,libc_Xwrite)

DEFINE_INTERN_ALIAS(libc_Xxwritef,Xsys_xwritef)
EXPORT(Xxwritef,libc_Xxwritef)

DEFINE_INTERN_ALIAS(libc_Xlseek64,Xsys_lseek)
EXPORT(Xlseek64,libc_Xlseek64)

DEFINE_INTERN_ALIAS(libc_Xpread64,Xsys_pread64)
EXPORT(Xpread64,libc_Xpread64)

DEFINE_INTERN_ALIAS(libc_Xxpreadf64,Xsys_xpreadf64)
EXPORT(Xxpreadf64,libc_Xxpreadf64)

DEFINE_INTERN_ALIAS(libc_Xpwrite64,Xsys_pwrite64)
EXPORT(Xpwrite64,libc_Xpwrite64)

DEFINE_INTERN_ALIAS(libc_Xxpwritef64,Xsys_xpwritef64)
EXPORT(Xxpwritef64,libc_Xxpwritef64)

DEFINE_INTERN_ALIAS(libc_Xsyncfs,Xsys_syncfs)
EXPORT(Xsyncfs,libc_Xsyncfs)

DEFINE_INTERN_ALIAS(libc_Xxreaddir,Xsys_xreaddir)
DEFINE_INTERN_ALIAS(libc_Xxreaddir64,Xsys_xreaddir)
EXPORT(Xxreaddir,libc_Xxreaddir)
EXPORT(Xxreaddir64,libc_Xxreaddir64)

DEFINE_INTERN_ALIAS(libc_Xxreaddirf,Xsys_xreaddirf)
DEFINE_INTERN_ALIAS(libc_Xxreaddirf64,Xsys_xreaddirf)
EXPORT(Xxreaddirf,libc_Xxreaddirf)
EXPORT(Xxreaddirf64,libc_Xxreaddirf64)

DEFINE_INTERN_ALIAS(libc_Xxmunmap,Xsys_xmunmap)
EXPORT(Xxmunmap,libc_Xxmunmap)

DEFINE_INTERN_ALIAS(libc_Xmmap,Xsys_mmap)
EXPORT(Xmmap,libc_Xmmap)

DEFINE_INTERN_ALIAS(libc_Xmunmap,Xsys_munmap)
EXPORT(Xmunmap,libc_Xmunmap)

DEFINE_INTERN_ALIAS(libc_Xxmprotect,Xsys_xmprotect)
EXPORT(Xxmprotect,libc_Xxmprotect)

DEFINE_INTERN_ALIAS(libc_Xmprotect,Xsys_mprotect)
EXPORT(Xmprotect,libc_Xmprotect)

DEFINE_INTERN_ALIAS(libc_Xexecve,Xsys_execve)
EXPORT(Xexecve,libc_Xexecve)

DEFINE_INTERN_ALIAS(libc_Xfexecveat,Xsys_execveat)
EXPORT(Xfexecveat,libc_Xfexecveat)

DEFINE_INTERN_ALIAS(libc_Xsysinfo,Xsys_sysinfo)
EXPORT(Xsysinfo,libc_Xsysinfo)

DEFINE_INTERN_ALIAS(libc_Xgetpgid,Xsys_getpgid)
EXPORT(Xgetpgid,libc_Xgetpgid)

DEFINE_INTERN_ALIAS(libc_Xsetpgid,Xsys_setpgid)
EXPORT(Xsetpgid,libc_Xsetpgid)

DEFINE_INTERN_ALIAS(libc_Xsetsid,Xsys_setsid)
EXPORT(Xsetsid,libc_Xsetsid)

DEFINE_INTERN_ALIAS(libc_Xgetsid,Xsys_getsid)
EXPORT(Xgetsid,libc_Xgetsid)

DEFINE_INTERN_ALIAS(libc_Xfork,Xsys_fork)
EXPORT(Xfork,libc_Xfork)

DEFINE_INTERN_ALIAS(libc_Xwait4,Xsys_wait4)
EXPORT(Xwait4,libc_Xwait4)

DEFINE_INTERN_ALIAS(libc_Xunshare,Xsys_unshare)
EXPORT(Xunshare,libc_Xunshare)

DEFINE_INTERN_ALIAS(libc_Xfutex64,Xsys_futex)
EXPORT(Xfutex64,libc_Xfutex64)

DEFINE_INTERN_ALIAS(libc_Xswapon,Xsys_swapon)
EXPORT(Xswapon,libc_Xswapon)

DEFINE_INTERN_ALIAS(libc_Xswapoff,Xsys_swapoff)
EXPORT(Xswapoff,libc_Xswapoff)

DEFINE_INTERN_ALIAS(libc_Xkill,Xsys_kill)
EXPORT(Xkill,libc_Xkill)

DEFINE_INTERN_ALIAS(libc_Xsigqueueinfo,Xsys_rt_sigqueueinfo)
EXPORT(Xsigqueueinfo,libc_Xsigqueueinfo)

DEFINE_INTERN_ALIAS(libc_Xtgsigqueueinfo,Xsys_rt_tgsigqueueinfo)
EXPORT(Xtgsigqueueinfo,libc_Xtgsigqueueinfo)

DEFINE_INTERN_ALIAS(libc_Xsigaltstack,Xsys_sigaltstack)
EXPORT(Xsigaltstack,libc_Xsigaltstack)

DEFINE_INTERN_ALIAS(libc_Xsethostname,Xsys_sethostname)
EXPORT(Xsethostname,libc_Xsethostname)

DEFINE_INTERN_ALIAS(libc_Xsetdomainname,Xsys_setdomainname)
EXPORT(Xsetdomainname,libc_Xsetdomainname)

DEFINE_INTERN_ALIAS(libc_Xfaccessat,Xsys_faccessat)
EXPORT(Xfaccessat,libc_Xfaccessat)

DEFINE_INTERN_ALIAS(libc_Xxdlclose,Xsys_xdlclose)
EXPORT(Xxdlclose,libc_Xxdlclose)

DEFINE_INTERN_ALIAS(libc_Xxdlsym,Xsys_xdlsym)
EXPORT(Xxdlsym,libc_Xxdlsym)

DEFINE_INTERN_ALIAS(libc_Xxfdlopenat,Xsys_xfdlopenat)
EXPORT(Xxfdlopenat,libc_Xxfdlopenat)

DEFINE_INTERN_ALIAS(libc_Xxdlmodule_info,Xsys_xfdlopenat)
EXPORT(Xxdlmodule_info,libc_Xxdlmodule_info)

DEFINE_INTERN_ALIAS(libc_Xsysconf,Xsys_xsysconf)
EXPORT(Xsysconf,libc_Xsysconf)

DEFINE_INTERN_ALIAS(libc_Xmremap,Xsys_mremap)
EXPORT(Xmremap,libc_Xmremap)

DEFINE_INTERN_ALIAS(libc_Xxdladdr2line,Xsys_xaddr2line)
EXPORT(Xxdladdr2line,libc_Xxdladdr2line)




DEFINE_INTERN_ALIAS(libc_umask,sys_umask)
EXPORT(__KSYM(umask),libc_umask)
EXPORT(__DSYM(_umask),libc_umask) /* DOS Alias */

DEFINE_INTERN_ALIAS(libc_sync,sys_sync)
EXPORT(sync,libc_sync)

DEFINE_INTERN_ALIAS(libc_Xmount,Xsys_mount)
EXPORT(Xmount,libc_Xmount)

DEFINE_INTERN_ALIAS(libc_Xumount2,Xsys_umount2)
EXPORT(Xumount2,libc_Xumount2)

DEFINE_INTERN_ALIAS(libc_xclone,sys_clone)
EXPORT(xclone,libc_xclone)

DEFINE_INTERN_ALIAS(libc_Xxclone,Xsys_clone)
EXPORT(Xxclone,libc_Xxclone)

DEFINE_INTERN_ALIAS(_libc_exit,sys_exit_group)
EXPORT(_exit,_libc_exit)
EXPORT(_Exit,_libc_exit)


























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








