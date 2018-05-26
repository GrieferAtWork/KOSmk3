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
#include <asm/universal.h>
#include <kernel/paging.h>

.section .text
.cfi_startproc
PUBLIC_ENTRY(pagedir_syncall)
	movq    %cr3, %rax
	movq    %rax, %cr3
	ret
.pagedir_syncall_size = . - pagedir_syncall
SYMEND(pagedir_syncall)

PUBLIC_ENTRY(pagedir_sync)
	movq    %rdi, %rcx
	movabs  $KERNEL_BASE_PAGE, %rdi
	cmpq    $64, %rsi
	jbe     2f
	/* Try to do a sync-all for large requests.
	 * However, because of the `X86_PAGE_FGLOBAL' bit,
	 * this method cannot be used to sync kernel mappings. */
	leaq    (%rcx,%rsi,1), %rax
	cmpq    %rcx, %rax
	/* >> if ((start + num_pages) <= start) goto 3f; */
	jbe     3f
	cmpq    %rdi, %rax
	/* >> if ((start + num_pages) <= KERNEL_BASE_PAGE) goto pagedir_syncall; */
	jbe     pagedir_syncall
	leaq    64(%rcx), %rax
	cmpq    %rdi, %rax
	/* >> if ((start + 64) >= KERNEL_BASE_PAGE) goto 2f; */
	jae     2f
	/* If we can save enough iterations doing it, still reload the
	 * entirety of the user-space portion of the page directory. */
	/* >> num_pages = (start + num_pages) - KERNEL_BASE_PAGE */
	leaq    (%rcx,%rsi,1), %rsi
	subq    %rdi, %rsi
	/* >> start     = KERNEL_BASE_PAGE */
	movq    %rdi, %rcx
4:	movq    %cr3, %rax
	movq    %rax, %cr3
2:	xchgq   %rcx, %rsi
	shlq    $(X86_PAGESHIFT), %rsi
1:	invlpg  (%rsi)
	addq    $(PAGESIZE), %rsi
	loop    1b
	ret
3:	/* The address space is overflowing. -> Just flush _everything_ */
	movabs  $((VM_VPAGE_MAX-KERNEL_BASE_PAGE)+1), %rsi
	movq    %rdi, %rcx
	jmp     4b
SYMEND(pagedir_sync)

PUBLIC_ENTRY(pagedir_syncone)
	shlq    $(X86_PAGESHIFT), %rdi
	invlpg  (%rdi)
	ret
SYMEND(pagedir_syncone)
.cfi_endproc

DEFINE_PUBLIC_WEAK_ALIAS(vm_sync,pagedir_sync)
DEFINE_PUBLIC_WEAK_ALIAS(vm_syncone,pagedir_syncone)
DEFINE_PUBLIC_WEAK_ALIAS(vm_syncall,pagedir_syncall)













