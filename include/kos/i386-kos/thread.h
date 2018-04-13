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
#ifndef _KOS_I386_KOS_THREAD_H
#define _KOS_I386_KOS_THREAD_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <kos/except.h>
#include "tls.h"

__SYSDECL_BEGIN

#define __TASK_SEGMENT_OFFSETOF_SELF        0
#define __TASK_SEGMENT_OFFSETOF_XCURRENT    __SIZEOF_POINTER__
#ifdef __KERNEL__
#define __TASK_SEGMENT_SIZE                (__SIZEOF_POINTER__+EXCEPTION_INFO_SIZE)
#else
#define __TASK_SEGMENT_OFFSETOF_STATE      (__SIZEOF_POINTER__+EXCEPTION_INFO_SIZE)
#define __TASK_SEGMENT_OFFSETOF_EFORMAT    (__SIZEOF_POINTER__+EXCEPTION_INFO_SIZE+3)
#define __TASK_SEGMENT_OFFSETOF_ERRNO      (__SIZEOF_POINTER__+EXCEPTION_INFO_SIZE+4)
#define __TASK_SEGMENT_OFFSETOF_DOS_ERRNO  (__SIZEOF_POINTER__+EXCEPTION_INFO_SIZE+8)
#define __TASK_SEGMENT_OFFSETOF_NT_ERRNO   (__SIZEOF_POINTER__+EXCEPTION_INFO_SIZE+12)
#define __TASK_SEGMENT_OFFSETOF_TID        (__SIZEOF_POINTER__+EXCEPTION_INFO_SIZE+16)
#define __TASK_SEGMENT_OFFSETOF_PROCESS    (__SIZEOF_POINTER__+EXCEPTION_INFO_SIZE+16+__SIZEOF_PID_T__)
#define __TASK_SEGMENT_OFFSETOF_UEH        (2*__SIZEOF_POINTER__+EXCEPTION_INFO_SIZE+16+__SIZEOF_PID_T__)
#define __TASK_SEGMENT_OFFSETOF_UEH_SP     (3*__SIZEOF_POINTER__+EXCEPTION_INFO_SIZE+16+__SIZEOF_PID_T__)
#define __TASK_SEGMENT_OFFSETOF_X86SYSBASE (4*__SIZEOF_POINTER__+EXCEPTION_INFO_SIZE+16+__SIZEOF_PID_T__)
#define __TASK_SEGMENT_SIZE                (5*__SIZEOF_POINTER__+EXCEPTION_INFO_SIZE+16+__SIZEOF_PID_T__)
#endif
#if defined(__KERNEL__) || defined(__USE_KOS)
#define TASK_SEGMENT_OFFSETOF_SELF          __TASK_SEGMENT_OFFSETOF_SELF
#define TASK_SEGMENT_OFFSETOF_XCURRENT      __TASK_SEGMENT_OFFSETOF_XCURRENT
#ifndef __KERNEL__
#define TASK_SEGMENT_OFFSETOF_STATE         __TASK_SEGMENT_OFFSETOF_STATE
#define TASK_SEGMENT_OFFSETOF_EFORMAT       __TASK_SEGMENT_OFFSETOF_EFORMAT
#define TASK_SEGMENT_OFFSETOF_ERRNO         __TASK_SEGMENT_OFFSETOF_ERRNO
#define TASK_SEGMENT_OFFSETOF_DOS_ERRNO     __TASK_SEGMENT_OFFSETOF_DOS_ERRNO
#define TASK_SEGMENT_OFFSETOF_NT_ERRNO      __TASK_SEGMENT_OFFSETOF_NT_ERRNO
#define TASK_SEGMENT_OFFSETOF_TID           __TASK_SEGMENT_OFFSETOF_TID
#define TASK_SEGMENT_OFFSETOF_PROCESS       __TASK_SEGMENT_OFFSETOF_PROCESS
#define TASK_SEGMENT_OFFSETOF_UEH           __TASK_SEGMENT_OFFSETOF_UEH
#define TASK_SEGMENT_OFFSETOF_UEH_SP        __TASK_SEGMENT_OFFSETOF_UEH_SP
#define TASK_SEGMENT_OFFSETOF_X86SYSBASE    __TASK_SEGMENT_OFFSETOF_X86SYSBASE
#endif
#define TASK_SEGMENT_SIZE                   __TASK_SEGMENT_SIZE
#endif

#define TASK_ERRNO_FKOS    0x00          /* Use `ts_errno' */
#define TASK_ERRNO_FDOS    0x01          /* Use `ts_dos_errno' */
#define TASK_ERRNO_FNT     0x02          /* Use `ts_nt_errno' */

#ifdef __CC__
struct process_env;
#ifdef __KERNEL__
struct task_segment {
     struct task_segment  *ts_self;      /* [1..1][const] Self-pointer. */
     struct exception_info ts_xcurrent;  /* Task error information. (The last exception that occurred)
                                          * A pointer to this field is returned by `error_info()' */
};
struct user_task_segment
#else
struct task_segment
#endif
{
#ifdef __KERNEL__
     struct user_task_segment  *ts_self;       /* [1..1][const] Self-pointer. */
     struct {
         struct exception_data  e_error;
         struct x86_usercontext e_context;
     }                          ts_xcurrent;   /* Task error information. (The last exception that occurred)
                                                * A pointer to this field is returned by `error_info()' */
#else
     struct task_segment       *ts_self;       /* [1..1][const] Self-pointer. */
     struct exception_info      ts_xcurrent;   /* Task error information. (The last exception that occurred)
                                                * A pointer to this field is returned by `error_info()' */
#endif
#define THREAD_STATE_FNORMAL    0x0000         /* Normal thread state. */
#define THREAD_STATE_FALONE     0x4000         /* The thread is meant to act on its own, and if it crashes,
                                                * then it should not bring down the remainder of the process.
                                                * This flag must be set manually and affects the intended behavior
                                                * of custom unhandled, and the behavior of the default (kernel)
                                                * unhandled exception handler. */
#define THREAD_STATE_FINUEH     0x8000         /* The thread is currently execution the unhandled-exception-handler.
                                                * If another unhandled exception occurs while this flag is set, the
                                                * thread (`THREAD_STATE_FALONE == 1') or process (`THREAD_STATE_FALONE' == 0)
                                                * will be terminated after the kernel decides how deal with the exception.
                                                * This flag is automatically set before the unhandled exception handler
                                                * is entered, and must be unset manually if user-space wishes to re-enable
                                                * unhandled exception handling. */
     __UINT16_TYPE__            ts_state;      /* Thread state (Set of `THREAD_STATE_F*') */
     __UINT8_TYPE__           __ts_pad1;       /* ... */
     __UINT8_TYPE__             ts_eformat;    /* The format for the most recent errno value (One of `TASK_ERRNO_F*'). */
     __UINT32_TYPE__            ts_errno;      /* The `errno' value used by libc. */
     __UINT32_TYPE__            ts_dos_errno;  /* The `errno' value used by libc (in DOS emulation). */
     __UINT32_TYPE__            ts_nt_errno;   /* The `errno' value used by kernel32.dll (GetLastError()). */
     __pid_t                    ts_tid;        /* The thread ID of this thread (as returned by `gettid()')
                                                * This field is originally initialized by the kernel, but
                                                * after that only ever used by user-space.
                                                * Note that when modified, the original value can be re-loaded using
                                                * `syscall(SYS_gettid)' (the libc export `gettid()' simply returns this field). */
     struct process_environ    *ts_process;    /* Pointer to the process environment block. */
     void                     (*ts_ueh)(void); /* [0..1] When non-NULL, the unhandled exception handler used by this thread.
                                                *        When NULL, use the default UEH handler instead.
                                                *  NOTE: Before being invoked, `THREAD_STATE_FINUEH' is set, and this
                                                *        function will not be invoked if that flag is already set.
                                                *        This function should not return normally. If it doesn't actually wish
                                                *        to handle the exception, it should call `error_unhandled_exception()'. */
     void                      *ts_ueh_sp;     /* [0..1] When non-NULL, the kernel will use this value as address
                                                *        for a user-space stack that should be set before executing
                                                *        the unhandled exception handler. */
     __UINTPTR_TYPE__           ts_x86sysbase; /* Base address added to #PF-based system call vector numbers.
                                                * On i386, this value is located between `0xc0000000 ... 0xc0fbffff'
                                                * Or more precisely: `0xc0ffffff - X86_ENCODE_PFSYSCALL_SIZE'
                                                * NOTE: Additionally, this address is aligned on a 16 byte boundary. */
};
#endif

/* Encode a system call vector address for use with `ts_syscall'.
 * This type of system call invocation was originally designed to implement `sigreturn'
 * without the need of mapping a piece of kernel code into user-space by
 * simply having a signal handler function ~return~ into kernel-space.
 * >>signal_handler:
 * >>    ...
 * >>    ret
 * >> 
 * >>foobar:
 * >>    ... // Generate an on-stack signal-frame, as described
 * >>        // by `/src/kernel/i386-kos/posix_signal.c'
 * >>    
 * >>    movl   %taskseg:TASK_SEGMENT_OFFSETOF_X86SYSBASE, %eax
 * >>    leal   X86_ENCODE_PFSYSCALL(SYS_sigreturn)(%eax), %eax
 * >>    pushl  %eax // ~supposed~ return address
 * >>    jmp    signal_handler
 * Once `signal_handler' returns, it will try to jump to a kernel-space address,
 * which then causes a PAGEFAULT as user-space isn't allowed to access that memory.
 * NOTE: This only works when the kernel detects
 *      `EIP == CR2 && CR2 in [ts_sysbase ... += X86_ENCODE_PFSYSCALL_SIZE]',
 *       meaning that regular pagefaults for that address range do not trigger
 *       this behavior. - Only jumping into that range does.
 * Special rules for these types of system calls:
 *  - The system call number is encoded as part of the
 *    faulting address (s.a. `X86_DECODE_PFSYSCALL')
 *  - Since this method involves `EIP == CR2', but leaves EAX unused, PF-syscall
 *    convention follows that of the `int $0x80' instruction with the addition
 *    of using EAX as return address register.
 *    Note however that system calls which never actually return do not make
 *    use of the return address, meaning that those are the only ones that
 *    can safely (and easily) be used for redirected function return addresses.
 *  - The kernel is aware of the special case of the return address override
 *    generated by a signal frame, which would normally redirect a user-space
 *    stack frame into kernel-space, or more precisely, at
 *    `ts_x86sysbase + X86_ENCODE_PFSYSCALL(SYS_sigreturn)'
 *    Knowing that, as well as the ~real~ (kernel-side) value of `ts_x86sysbase',
 *    KOS will correctly unwind signal frames when using the `SYS_xunwind'
 *    system call, or when throwing an exception from within a signal handler.
 */
#define X86_ENCODE_PFSYSCALL(sysno) \
      (((sysno) & 0xffff) | ((sysno) & 0xc0000000) >> 14)
#define X86_DECODE_PFSYSCALL(rel_faultaddr) \
      (((rel_faultaddr) & 0xffff) | ((rel_faultaddr) & 0x30000) << 14)
#define X86_ENCODE_PFSYSCALL_BITS 18 /* 16 ID bits; +1 for xsyscalls; +1 for exception mode. */
#define X86_ENCODE_PFSYSCALL_SIZE (1 << X86_ENCODE_PFSYSCALL_BITS)


__SYSDECL_END

#endif /* !_KOS_I386_KOS_THREAD_H */
