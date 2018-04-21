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
#include <asm/cpu-flags.h>
#include "../hybrid.h"

#ifdef __KERNEL__
#include <i386-kos/vm86.h>
#endif

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

#ifdef CONFIG_VM86
	testl  $EFLAGS_VM, X86_CONTEXT_OFFSETOF_EFLAGS(%ecx)
	jnz    .cpu_setcontext_vm86
#endif

#ifdef CONFIG_X86_SEGMENTATION
#define REGSPACE 12
#else
#define REGSPACE 0
#endif

	/* Copy the IRET tail into the target stack. */
	movl   X86_CONTEXT_OFFSETOF_ESP(%ecx), %eax
	subl   $12, %eax
	testl  $3, X86_CONTEXT_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_CS(%ecx)
	jz     1f
	/* Jump to user-space. */
	subl   $20, %esp
	movl   %esp, %eax /* Use the real stack as temporary buffer. */
	movl   X86_CONTEXT_OFFSETOF_IRET+16(%ecx), %edx
	movl   %edx, 16(%eax)
	movl   X86_CONTEXT_OFFSETOF_IRET+12(%ecx), %edx
	movl   %edx, 12(%eax)
1:	movl   X86_CONTEXT_OFFSETOF_IRET+8(%ecx), %edx
	movl   %edx, 8(%eax)
	movl   X86_CONTEXT_OFFSETOF_IRET+4(%ecx), %edx
	movl   %edx, 4(%eax)
	movl   X86_CONTEXT_OFFSETOF_IRET+0(%ecx), %edx
	movl   %edx, 0(%eax)

	/* Load general-purpose registers. */
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBX(%ecx), %ebx
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBP(%ecx), %ebp
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDI(%ecx), %edi
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ESI(%ecx), %esi
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX(%ecx), %edx
#ifndef CONFIG_X86_SEGMENTATION
	/* Load the new stack. */
	cli
	movl   %eax, %esp
	/* Load remaining registers. */
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
	/* Load the new stack. */
	cli
	movl   %eax, %esp
	/* Load the remaining registers. */
	movl   %ss:X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX(%ecx), %eax
	movl   %ss:X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX(%ecx), %ecx
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


#ifdef CONFIG_VM86
.section .text
.cpu_setcontext_vm86:
	/* Load a `struct cpu_context_vm86' currently stored in `ECX' */
	movl   X86_GPREGS32_OFFSETOF_ESP(%ecx), %eax
	subl   $40, %eax /* 4+sizeof(struct x86_irregs_vm86) */
#ifdef CONFIG_X86_SEGMENTATION
#define VM86_OFFSETOF_IRET  (X86_GPREGS_SIZE+X86_SEGMENTS_SIZE)
#else
#define VM86_OFFSETOF_IRET   X86_GPREGS_SIZE
#endif

	movl   VM86_OFFSETOF_IRET+ 0(%ecx), %edx
	movl   %edx,  4(%eax) /* EIP */
	movl   VM86_OFFSETOF_IRET+ 4(%ecx), %edx
	movl   %edx,  8(%eax) /* CS */
	movl   VM86_OFFSETOF_IRET+ 8(%ecx), %edx
	movl   %edx, 12(%eax) /* EFLAGS */
	movl   VM86_OFFSETOF_IRET+12(%ecx), %edx
	movl   %edx, 16(%eax) /* ESP */
	movl   VM86_OFFSETOF_IRET+16(%ecx), %edx
	movl   %edx, 20(%eax) /* SS */
	movl   VM86_OFFSETOF_IRET+20(%ecx), %edx
	movl   %edx, 24(%eax) /* ES */
	movl   VM86_OFFSETOF_IRET+24(%ecx), %edx
	movl   %edx, 28(%eax) /* DS */
	movl   VM86_OFFSETOF_IRET+28(%ecx), %edx
	movl   %edx, 32(%eax) /* FS */
	movl   VM86_OFFSETOF_IRET+32(%ecx), %edx
	movl   %edx, 36(%eax) /* GS */

	/* Load general-purpose registers. */
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDI(%ecx), %edi
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ESI(%ecx), %esi
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBP(%ecx), %ebp
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBX(%ecx), %ebx
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX(%ecx), %edx
	movl   X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX(%ecx), %ecx

	/* Enter VM86 mode. */
	cli
	movl   %eax, %esp
	popl   %eax /* Restore the last register. */
	iret        /* Do the iret */
#endif

#endif /* __KERNEL__ */















