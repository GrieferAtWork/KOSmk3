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
#include <kernel/paging.h>
#include <kernel/memory.h>
#include <kos/context.h>
#include <asm/cpu-flags.h>

#ifndef CONFIG_KERNELSTACK_SIZE
#define CONFIG_KERNELSTACK_SIZE 4096*8
#endif

/* The starting VEC2-index (in `x86_pdir::p_e2') and amount
 * of continuous indices thereafter of the kernel-share segment.
 * That is the segment where the kernel itself resides at, which
 * is then mapped again in all other page directories. */
#define VEC2_SHARE_BEGIN     X86_PDIR_VEC2INDEX(KERNEL_BASE)
#define VEC2_SHARE_SIZE     (VEC2_IDENTITY_BEGIN-VEC2_SHARE_BEGIN)

/* Similar to the SHARE indices, but for the identity mapping instead. */
#define VEC2_IDENTITY_BEGIN  X86_PDIR_VEC2INDEX(X86_PDIR_E1_IDENTITY_BASE)
#define VEC2_IDENTITY_SIZE  (1024-VEC2_IDENTITY_BEGIN)


.section .bss.boot.boot_stack
INTERN_OBJECT(x86_boot_stack)
	.space CONFIG_KERNELSTACK_SIZE
INTERN_LABEL(x86_boot_stack_top)
#ifndef NDEBUG
	.long 0 /* Terminate tracebacks with a NULL-eip. */
#endif
SYMEND(x86_boot_stack)

#if 1
.section .text.free
PUBLIC_ENTRY(_start)
	/* Save the initial register state. */
	movl   %eax,    (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EAX
	movl   %ecx,    (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ECX
	movl   %edx,    (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDX
	movl   %ebx,    (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBX
	movl   %esp,    (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_ESP
	movl   %ebp,    (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EBP
	movl   %esi,    (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_ESI
	movl   %edi,    (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_GPREGS+X86_GPREGS_OFFSETOF_EDI
#ifdef CONFIG_X86_SEGMENTATION
	movw   %ds,     (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_DS
	movw   %es,     (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_ES
	movw   %fs,     (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_FS
	movw   %gs,     (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_SEGMENTS+X86_SEGMENTS_OFFSETOF_GS
#endif /* CONFIG_X86_SEGMENTATION */
	movw   %cs,     (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_CS
	movl   $(_start - KERNEL_BASE), (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_EIP /* Kind-of unnecessary, but eh... */
	leal   x86_boot_stack + CONFIG_KERNELSTACK_SIZE - KERNEL_BASE, %esp
	pushfl
	popl   (x86_boot_context - KERNEL_BASE)+X86_CONTEXT_OFFSETOF_EFLAGS

	/* Setup the kernel page directory. */
	leal   pagedir_kernel_phys + VEC2_SHARE_BEGIN * 4, %edi
	leal   (pagedir_kernel_share - KERNEL_BASE) + \
	       (X86_PAGE_FGLOBAL | X86_PAGE_FDIRTY | X86_PAGE_FACCESSED | \
	        X86_PAGE_FWRITE | X86_PAGE_FPRESENT),   %eax
	movl   $(VEC2_SHARE_SIZE), %ecx
1:	stosl
	addl   $4096, %eax /* Advance to the next table. */
	loop   1b
	/* Initialize the identity page. */
	movl   $(pagedir_kernel_phys + \
	        (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED | \
	         X86_PAGE_FWRITE | X86_PAGE_FPRESENT)), \
	         pagedir_kernel_phys + VEC2_IDENTITY_BEGIN * 4
	/* Create an identity mapping of first 1Gb(-1K) by mapping the
	 * shared kernel segment a second time to cover the low memory area. */
	leal   pagedir_kernel_phys,                     %edi
	leal   (pagedir_kernel_share - KERNEL_BASE) + \
	       (X86_PAGE_FGLOBAL | X86_PAGE_FDIRTY | X86_PAGE_FACCESSED | \
	        X86_PAGE_FWRITE | X86_PAGE_FPRESENT),   %eax
	movl   $(VEC2_SHARE_SIZE/* + VEC2_IDENTITY_SIZE*/), %ecx
1:	stosl
	addl   $4096, %eax /* Advance to the next table. */
	loop   1b

#define PERM_XR   (X86_PAGE_FGLOBAL|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT)
#define PERM_RW   (X86_PAGE_FGLOBAL|X86_PAGE_FACCESSED|X86_PAGE_FPRESENT|X86_PAGE_FDIRTY|X86_PAGE_FWRITE)
	/* With all that set up, now we must initialize
	 * the P1 entires from `pagedir_kernel_share'.
	 * We do this in 3 stages:
	 *   0  ...  kernel_start           --> -RW
	 *   kernel_start ... kernel_ro_end --> XR-  (If possible, execute permissions are removed later)
	 *   kernel_ro_end ... 1Gb          --> -RW
	 */
	leal   pagedir_kernel_share - KERNEL_BASE, %edi

	movl   $_boot_page0_size, %ecx
	leal   0 + PERM_RW, %eax
1:	stosl
	addl   $(PAGESIZE), %eax /* Advance to the next page. */
	loop   1b

	movl   $_boot_page1_size, %ecx
	leal   kernel_start - KERNEL_BASE + PERM_XR, %eax
1:	stosl
	addl   $(PAGESIZE), %eax /* Advance to the next page. */
	loop   1b

	movl   $_boot_page2_size, %ecx
	leal   kernel_ro_end - KERNEL_BASE + PERM_RW, %eax
1:	stosl
	addl   $(PAGESIZE), %eax /* Advance to the next page. */
	loop   1b

	/* EDI = (pagedir_kernel_share - KERNEL_BASE) + VEC2_IDENTITY_BEGIN */
	/* Finally, initialize the page-directory self mapping. */
	movl   $1024, %ecx
	leal   pagedir_kernel_share - KERNEL_BASE + PERM_RW, %eax
1:	stosl
	addl   $(PAGESIZE), %eax /* Advance to the next page. */
	loop   1b


	/* And with that, the kernel page directory is fully initialized.
	 * Now to activate paging. */
	leal   pagedir_kernel_phys, %esi
	movl   %esi, %cr3
	movl   %cr0, %eax
	orl    $(CR0_PG|CR0_WP), %eax
	movl   %eax, %cr0

	/* Setup a stack. */
	leal   x86_boot_stack + CONFIG_KERNELSTACK_SIZE, %esp

	/* Jump into virtual memory. */
	jmp    1f + KERNEL_BASE
1:

	/* Initialize the kernel. */
	jmp    x86_kernel_main
SYMEND(_start)

#endif

