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
#include <kos/context.h>
#include <asm/cfi.h>
#include "../hybrid.h"

#ifndef __KERNEL__
PUBLIC_ENTRY(cpu_getcontext)
INTERN_ENTRY(libc_cpu_getcontext)
	.cfi_startproc
	/* ECX: struct cpu_context *__restrict old_context */
	movl   $0,   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX(%ecx)
	movl   %ecx, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX(%ecx)
	movl   %edx, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX(%ecx)
	movl   %ebx, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBX(%ecx)
	movl   %ebp, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBP(%ecx)
	movl   %edi, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDI(%ecx)
	movl   %esi, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ESI(%ecx)
	popl_cfi     X86_CONTEXT_OFFSETOF_EIP(%ecx)
	.cfi_undefined %eip /* XXX: Maybe encode using expressions? */
	pushfl_cfi
	popl_cfi     X86_CONTEXT_OFFSETOF_EFLAGS(%ecx)
	movl   %esp, X86_CONTEXT_OFFSETOF_ESP(%ecx)
#ifdef CONFIG_X86_SEGMENTATION
	movw   %gs,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS(%ecx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS+2(%ecx)
	movw   %fs,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS(%ecx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS+2(%ecx)
	movw   %es,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES(%ecx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES+2(%ecx)
	movw   %ds,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS(%ecx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS+2(%ecx)
	movw   %cs,  X86_CONTEXT32_OFFSETOF_CS(%ecx)
	movw   $0,   X86_CONTEXT32_OFFSETOF_CS+2(%ecx)
	movw   %ss,  X86_CONTEXT32_OFFSETOF_SS(%ecx)
	movw   $0,   X86_CONTEXT32_OFFSETOF_SS+2(%ecx)
#endif /* CONFIG_X86_SEGMENTATION */
	movl   $1,   %eax
	jmp    *X86_CONTEXT_OFFSETOF_EIP(%ecx)
	.cfi_endproc
SYMEND(libc_cpu_getcontext)
SYMEND(cpu_getcontext)

PUBLIC_ENTRY(cpu_xchcontext)
INTERN_ENTRY(libc_cpu_xchcontext)
	/* ECX: struct cpu_context const *__restrict new_context */
	/* EDX: struct cpu_context *__restrict old_context */
	.cfi_startproc
	movl   %eax, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX(%edx)
	movl   %ecx, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX(%edx)
	movl   %edx, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX(%edx)
	movl   %ebx, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBX(%edx)
	movl   %ebp, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBP(%edx)
	movl   %edi, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDI(%edx)
	movl   %esi, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ESI(%edx)
	popl_cfi     X86_CONTEXT_OFFSETOF_EIP(%edx)
	.cfi_undefined %eip /* XXX: Maybe encode using expressions? */
	pushfl_cfi
	popl_cfi     X86_CONTEXT_OFFSETOF_EFLAGS(%edx)
	movl   %esp, X86_CONTEXT_OFFSETOF_ESP(%edx)
#ifdef CONFIG_X86_SEGMENTATION
	movw   %gs,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS(%edx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS+2(%edx)
	movw   %fs,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS(%edx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS+2(%edx)
	movw   %es,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES(%edx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES+2(%edx)
	movw   %ds,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS(%edx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS+2(%edx)
	movw   %cs,  X86_CONTEXT32_OFFSETOF_CS(%edx)
	movw   $0,   X86_CONTEXT32_OFFSETOF_CS+2(%edx)
	movw   %ss,  X86_CONTEXT32_OFFSETOF_SS(%edx)
	movw   $0,   X86_CONTEXT32_OFFSETOF_SS+2(%edx)
#endif /* CONFIG_X86_SEGMENTATION */
	.cfi_endproc
	/* Now load the new context. */
PUBLIC_ENTRY(cpu_setcontext)
INTERN_ENTRY(libc_cpu_setcontext)
	/* ECX: struct cpu_context const *__restrict new_context */
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX(%ecx), %eax
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX(%ecx), %edx
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBX(%ecx), %ebx
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBP(%ecx), %ebp
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDI(%ecx), %edi
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ESI(%ecx), %esi
	pushl  X86_CONTEXT_OFFSETOF_EFLAGS(%ecx)
	popfl
	movl   X86_CONTEXT_OFFSETOF_ESP(%ecx), %esp
#ifdef CONFIG_X86_SEGMENTATION
	movw   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS(%ecx), %gs
	movw   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS(%ecx), %fs
	movw   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES(%ecx), %es
	movw   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS(%ecx), %ds
//	movw   X86_CONTEXT32_OFFSETOF_CS(%ecx), %cs /* Not how it works... */
	movw   X86_CONTEXT32_OFFSETOF_SS(%ecx), %ss
#endif /* CONFIG_X86_SEGMENTATION */
	pushl  X86_CONTEXT_OFFSETOF_EIP(%ecx)
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX(%ecx), %ecx
	ret
SYMEND(libc_cpu_setcontext)
SYMEND(libc_cpu_xchcontext)
SYMEND(cpu_setcontext)
SYMEND(cpu_xchcontext)

#else /* !__KERNEL__ */

PUBLIC_ENTRY(cpu_getcontext)
INTERN_ENTRY(libc_cpu_getcontext)
	.cfi_startproc
	/* ECX: struct cpu_context *__restrict old_context */
	movl   $0,   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX(%ecx)
	movl   %ecx, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX(%ecx)
	movl   %edx, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX(%ecx)
	movl   %ebx, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBX(%ecx)
	movl   %ebp, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBP(%ecx)
	movl   %edi, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDI(%ecx)
	movl   %esi, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ESI(%ecx)
	popl_cfi     X86_CONTEXT_OFFSETOF_EIP(%ecx)
	.cfi_undefined %eip /* XXX: Maybe encode using expressions? */
	pushfl_cfi
	popl_cfi     X86_CONTEXT_OFFSETOF_EFLAGS(%ecx)
	movl   %esp, X86_CONTEXT_OFFSETOF_ESP(%ecx)
#ifdef CONFIG_X86_SEGMENTATION
	movw   %gs,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS(%ecx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS+2(%ecx)
	movw   %fs,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS(%ecx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS+2(%ecx)
	movw   %es,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES(%ecx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES+2(%ecx)
	movw   %ds,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS(%ecx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS+2(%ecx)
#endif /* CONFIG_X86_SEGMENTATION */
	xorl   %eax, %eax
	movw   %cs,  %ax
	movl   %eax, X86_CONTEXT_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_CS(%ecx)
	movl   $1,   %eax
	jmp    *X86_CONTEXT_OFFSETOF_EIP(%ecx)
	.cfi_endproc
SYMEND(libc_cpu_getcontext)
SYMEND(cpu_getcontext)

PUBLIC_ENTRY(cpu_xchcontext)
INTERN_ENTRY(libc_cpu_xchcontext)
	/* ECX: struct cpu_context const *__restrict new_context */
	/* EDX: struct cpu_context *__restrict old_context */
	.cfi_startproc
	movl   %eax, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX(%edx)
	movl   %ecx, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX(%edx)
	movl   %edx, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX(%edx)
	movl   %ebx, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBX(%edx)
	movl   %ebp, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBP(%edx)
	movl   %edi, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDI(%edx)
	movl   %esi, X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ESI(%edx)
	popl_cfi     X86_CONTEXT_OFFSETOF_EIP(%edx)
	.cfi_undefined %eip /* XXX: Maybe encode using expressions? */
	pushfl_cfi
	popl_cfi     X86_CONTEXT_OFFSETOF_EFLAGS(%edx)
	movl   %esp, X86_CONTEXT_OFFSETOF_ESP(%edx)
#ifdef CONFIG_X86_SEGMENTATION
	movw   %gs,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS(%edx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS+2(%edx)
	movw   %fs,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS(%edx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS+2(%edx)
	movw   %es,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES(%edx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES+2(%edx)
	movw   %ds,  X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS(%edx)
	movw   $0,   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS+2(%edx)
#endif /* CONFIG_X86_SEGMENTATION */
	xorl   %eax, %eax
	movw   %cs,  %ax
	movl   %eax, X86_CONTEXT_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_CS(%edx)
	.cfi_endproc
#ifdef __KERNEL__
	jmp    libc_cpu_setcontext
INTERN_ENTRY(cpu_setcontext_pop)
	/* Used during the initial transition to
	 * user-space, after .free has been deleted. */
	popl   %ecx
#endif
	/* Now load the new context. */
PUBLIC_ENTRY(cpu_setcontext)
INTERN_ENTRY(libc_cpu_setcontext)
	/* ECX: struct cpu_context const *__restrict new_context */
#ifdef CONFIG_X86_SEGMENTATION
#define REGSPACE 12
#else
#define REGSPACE 0
#endif

	/* Copy the IRET tail into the target stack. */
	movl   X86_CONTEXT_OFFSETOF_ESP(%ecx), %eax
	subl   $12 + REGSPACE, %eax
	testl  $3, X86_CONTEXT_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_CS(%ecx)
	jz     1f
	/* Must copy additional data if it's actually a user-context. */
#if 1
	subl   $(8 + 12 + REGSPACE), %esp
	movl   %esp, %eax /* Use the real stack as temporary buffer. */
#else
	subl   $8, %eax
#endif
	movl   X86_CONTEXT_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_SS(%ecx), %edx
	movl   %edx, REGSPACE+16(%eax)
	movl   X86_CONTEXT_OFFSETOF_IRET+X86_IRREGS_USER32_OFFSETOF_USERESP(%ecx), %edx
	movl   %edx, REGSPACE+12(%eax)
1:	movl   X86_CONTEXT_OFFSETOF_EFLAGS(%ecx), %edx
	movl   %edx, REGSPACE+8(%eax)
	movl   X86_CONTEXT_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_CS(%ecx), %edx
	movl   %edx, REGSPACE+4(%eax)
	movl   X86_CONTEXT_OFFSETOF_EIP(%ecx), %edx
	movl   %edx, REGSPACE+0(%eax)

	/* Load general-purpose registers. */
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBX(%ecx), %ebx
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBP(%ecx), %ebp
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDI(%ecx), %edi
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ESI(%ecx), %esi

#ifndef CONFIG_X86_SEGMENTATION
	/* Load the new stack. */
	movl   %eax, %esp

	/* Load remaining registers. */
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX(%ecx), %edx
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX(%ecx), %eax
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX(%ecx), %ecx
#else /* !CONFIG_X86_SEGMENTATION */
	/* Load the new segment registers (NOTE: Load `DS' last!)
	 * NOTE: Must be loaded before we push the segment prefix in
	 *       case the target stack overlaps with the given CPU state. */
	movw   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS(%ecx), %gs
	movw   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS(%ecx), %fs
	movw   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES(%ecx), %es
	movw   X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS(%ecx), %ds
	/* Copy remaining registers onto the target stack.
	 * Can only be done now in case the given CPU state
	 * overlapped with the target stack. */
	movl   %ss:X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX(%ecx), %edx
	movl   %edx, 4(%eax)
	movl   %ss:X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX(%ecx), %edx
	movl   %edx, 0(%eax)

	/* Load the new stack. */
	movl   %eax, %esp

	/* Load remaining registers. */
	popl   %ecx
	popl   %eax
	popl   %edx
#endif /* CONFIG_X86_SEGMENTATION */

	/* Execute the IRET */
	iret
SYMEND(libc_cpu_setcontext)
SYMEND(cpu_setcontext)
#ifdef __KERNEL__
SYMEND(cpu_setcontext_pop)
#endif
SYMEND(libc_cpu_xchcontext)
SYMEND(cpu_xchcontext)
#endif /* __KERNEL__ */















