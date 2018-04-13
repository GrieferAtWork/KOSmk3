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
#include <asm/cfi.h>

PUBLIC_ENTRY(syscall)
INTERN_ENTRY(libc_syscall)
	.cfi_startproc
	/* Must transform registers as follows:
	 * %rdi, %rsi, %rdx, %rcx, %r8,  %r9, 8(%rsp)
	 *   v     v     v     v     v    v    v
	 * %rax, %rdi, %rsi, %rdx, %r10, %r8, %r9
	 * NOTE: No registers must be saved, because none that are
	 *       callee-save ones are used for system calls arguments. */
	movq  %rdi,    %rax /* SYSNO */
	movq  %rsi,    %rdi /* ARG #1 */
	movq  %rdx,    %rsi /* ARG #2 */
	movq  %rcx,    %rdx /* ARG #3 */
	movq  %r8,     %r10 /* ARG #4 */
	movq  %r9,     %r8  /* ARG #5 */
	movq  8(%rsp), %r9  /* ARG #6 */
	int   $0x80         /* Invoke the system call. */
	ret
	.cfi_endproc
SYMEND(libc_syscall)
SYMEND(syscall)

