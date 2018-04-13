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
#include <kernel/paging.h>

.section .text
.cfi_startproc
PUBLIC_ENTRY(pagedir_syncall)
	movl    %cr3, %eax
	movl    %eax, %cr3
	ret
.pagedir_syncall_size = . - pagedir_syncall
SYMEND(pagedir_syncall)

PUBLIC_ENTRY(pagedir_sync)
	cmpl    $64, %edx
	jbe     2f
	/* Try to do a sync-all for large requests.
	 * However, because of the `X86_PAGE_FGLOBAL' bit,
	 * this method cannot be used to sync kernel mappings. */
	leal    (%ecx,%edx,1), %eax
	cmpl    %ecx, %eax
	/* >> if ((start + num_pages) <= start) goto 3f; */
	jbe     3f
	cmpl    $(X86_KERNEL_BASE_PAGE), %eax
	/* >> if ((start + num_pages) <= X86_KERNEL_BASE_PAGE) goto pagedir_syncall; */
	jbe     pagedir_syncall
	leal    64(%ecx),                %eax
	cmpl    $(X86_KERNEL_BASE_PAGE), %eax
	/* >> if ((start + 64) >= X86_KERNEL_BASE_PAGE) goto 2f; */
	jae     2f
	/* If we can save enough iterations doing it, still reload the
	 * entirety of the user-space portion of the page directory. */
	/* >> num_pages = (start + num_pages) - X86_KERNEL_BASE_PAGE */
	leal    -X86_KERNEL_BASE_PAGE(%ecx,%edx,1), %edx
	/* >> start     = X86_KERNEL_BASE_PAGE */
	movl    $(X86_KERNEL_BASE_PAGE), %ecx
4:	movl    %cr3, %eax
	movl    %eax, %cr3
2:	xchgl   %ecx, %edx
	shll    $(X86_PAGESHIFT), %edx
1:	invlpg  (%edx)
	addl    $(PAGESIZE), %edx
	loop    1b
	ret
3:	/* The address space is overflowing. -> Just flush _everything_ */
	movl    $((VM_VPAGE_MAX-X86_KERNEL_BASE_PAGE)+1), %edx
	movl    $(X86_KERNEL_BASE_PAGE), %ecx
	jmp     4b
SYMEND(pagedir_sync)

PUBLIC_ENTRY(pagedir_syncone)
	shll    $(X86_PAGESHIFT), %ecx
	invlpg  (%ecx)
	ret
SYMEND(pagedir_syncone)
.cfi_endproc

DEFINE_PUBLIC_WEAK_ALIAS(vm_sync,pagedir_sync)
DEFINE_PUBLIC_WEAK_ALIAS(vm_syncone,pagedir_syncone)
DEFINE_PUBLIC_WEAK_ALIAS(vm_syncall,pagedir_syncall)



.section .text.free
INTERN_ENTRY(x86_config_pagedir_invlpg_unsupported)
	/* XXX: We could use an `E_ILLEGAL_INSTRUCTION' exception handler to do this lazily */
	.cfi_startproc
	pushl_cfi_r %esi
	pushl_cfi_r %edi
	/* Override the assembly of the sync-functions with sync-all */
	movl  $(.pagedir_syncall_size), %ecx
	leal  pagedir_syncall, %esi
	leal  pagedir_sync, %edi
	rep   movsb
	movl  $(.pagedir_syncall_size), %ecx
	leal  pagedir_syncone, %esi
	leal  pagedir_sync, %edi
	rep   movsb
	popl_cfi_r %edi
	popl_cfi_r %esi
	.cfi_endproc
	ret
SYMEND(x86_config_pagedir_invlpg_unsupported)

















