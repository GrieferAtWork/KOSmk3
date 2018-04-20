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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_SYSCALL_H
#define GUARD_KERNEL_INCLUDE_KERNEL_SYSCALL_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/syscall.h>
#include <stdbool.h>
#endif

DECL_BEGIN

#ifdef __CC__

/* Enable/disable system call tracing on all CPUs.
 * When enabled, `syscall_trace()' will be executed before any system call invocation. */
FUNDEF void KCALL enable_syscall_tracing(void);
FUNDEF void KCALL disable_syscall_tracing(void);

FUNDEF ATTR_NOTHROW void KCALL syscall_trace(struct syscall_trace_regs *__restrict regs);


/* Return `true' if a system call `sysno' should be restarted.
 * @param: sysno:   The system call vector number of the system call
 *                  This number should include the Xsys-bit, as originally
 *                  specified by user-space.
 * @param: context: Set of `SHOULD_RESTART_SYSCALL_F*' describing the
 *                  reason that lead to the system call being interrupted. */
FUNDEF bool KCALL should_restart_syscall(syscall_ulong_t sysno,
                                         unsigned int context);
#define SHOULD_RESTART_SYSCALL_FNORMAL       0x0000 /* The system call was interrupted by an unhandled `E_INTERRUPT'
                                                     * exception about to be propagated back into user-space. */
#define SHOULD_RESTART_SYSCALL_FPOSIX_SIGNAL 0x0001 /* The system call was interrupted by a posix_signal. */
#define SHOULD_RESTART_SYSCALL_FSA_RESTART   0x0002 /* For use with `SHOULD_RESTART_SYSCALL_FPOSIX_SIGNAL':
                                                     * The signal that caused the system call to be
                                                     * interrupted also had the `SA_RESTART' flag set. */

#endif

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_SYSCALL_H */
