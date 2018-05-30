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
#include <hybrid/typecore.h>
#include <kernel/paging.h>
#include <kernel/memory.h>
#include <kos/context.h>
#include <i386-kos/gdt.h>
#include <asm/cpu-flags.h>
#include <asm/universal.h>

.section .bss.boot.boot_stack
INTERN_OBJECT(x86_boot_stack)
	.space CONFIG_KERNELSTACK_SIZE
INTERN_LABEL(x86_boot_stack_top)
#ifndef NDEBUG
	.quad 0 /* Terminate tracebacks with a NULL-eip. */
#endif
SYMEND(x86_boot_stack)


#define SYM(x) (x - KERNEL_CORE_BASE)

.code32
.section .bss.free
INTERN_OBJECT(boot_failure_vga_ptr)
	.long 0
SYMEND(boot_failure_vga_ptr)

#define VGA_BASE   0xb8000
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

.section .text.free
/* CLOBBER: []
 * OUT: [pointer_to_vga_base: %edi]
 * Clear the VGA screen and re-initialize the VGA pointer. */
PRIVATE_ENTRY(vga_cls)
	pushl  %eax
	pushl  %ecx
	leal   VGA_BASE, %edi
	movl   %edi, SYM(boot_failure_vga_ptr)
	movl   $(VGA_WIDTH * VGA_HEIGHT), %ecx
	movl   $0, %eax
	rep    stosw
	leal   VGA_BASE, %edi
	popl   %ecx
	popl   %eax
	ret
SYMEND(vga_cls)


/* CLOBBER: []
 * OUT: [pointer_to_last_line: %edi]
 * Scroll the VGA display by 1 line and set
 * the VGA point to the start of the last line. */
PRIVATE_ENTRY(vga_scroll)
	pushl  %esi
	pushl  %ecx
	pushl  %eax
	leal   VGA_BASE, %edi
	leal   VGA_BASE + (VGA_WIDTH * 2), %esi
	movl   $(VGA_WIDTH * (VGA_HEIGHT - 1)), %ecx
	rep    movsw /* Scroll display */
	movl   %edi, SYM(boot_failure_vga_ptr) /* Set to last line */
	movl   $(VGA_WIDTH), %ecx
	xorl   %eax, %eax
	rep    stosw /* Clear text from last line */
	subl   $(VGA_WIDTH * 2), %edi
	popl   %eax
	popl   %ecx
	popl   %esi
	ret
SYMEND(vga_scroll)


/* IN:      [ch: %al]
 * CLOBBER: [%eax, %eflags]
 * Output the given character to screen */
PRIVATE_ENTRY(vga_putc)
	pushl  %edi
	movl   SYM(boot_failure_vga_ptr), %edi
	testl  %edi, %edi
	jnz    1f
	call   vga_cls
1:	cmpl   $(VGA_BASE + (VGA_WIDTH * VGA_HEIGHT * 2)), %edi
	jnae   1f
	call   vga_scroll /* Scroll if past the end */
1:	cmpb   $'\n', %al
	jne    98f
	/* Special character: `\n' */
	pushl  %ecx
	pushl  %edx
	leal   -VGA_BASE(%edi), %ecx
	movl   $(VGA_WIDTH * 2), %edx
	movw   %cx, %ax
	divb   %dl
	/* EDI = VGA_BASE + (ECX + ((VGA_WIDTH * 2) - AH)) */
	leal   (VGA_BASE + (VGA_WIDTH * 2))(%ecx), %edi
	/* EDI = EDI - AH */
	movzbl %ah, %eax
	/* EDI = EDI - EAX */
	subl   %eax, %edi
	popl   %edx
	popl   %ecx
	cmpl   $(VGA_BASE + (VGA_WIDTH * VGA_HEIGHT * 2)), %edi
	jnae   99f
	call   vga_scroll /* Scroll if past the end */
	jmp    99f
98:	movb   $(7 | 0 << 4), %ah /* Light gray on black. */
	stosw  /* Print the character */
99:	movl   %edi, SYM(boot_failure_vga_ptr)
	popl   %edi
	ret
SYMEND(vga_putc)


/* IN:      [str: %esi]
 * CLOBBER: [%esi, %eflags]
 * Output the given string to screen. */
PRIVATE_ENTRY(vga_puts)
	pushl  %eax
1:	lodsb
	testb  %al, %al
	jz     2f
	call   vga_putc
	jmp    1b
2:	popl   %eax
	ret
SYMEND(vga_puts)


.pushsection .data.free
PRIVATE_STRING(str_bootfailure_1,"Failed to boot into 64-bit mode:\n    ")
PRIVATE_STRING(str_bootfailure_2,"\n\nPress CTRL+ALT+DEL to reboot")
PRIVATE_STRING(str_nocpuid,"The <CPUID> instruction is not supported")
PRIVATE_STRING(str_nocpuid_0x1,"The cpuid leaf <1h> is not implemented")
PRIVATE_STRING(str_nocpuid_0x80000001,"The cpuid leaf <80000001h> is not implemented")
PRIVATE_STRING(str_no_longmode,"\
Long mode (64-bit mode) is not supported\n\
   [cpuid leaf 80000001h:%edx is lacking #LM (bit #29)]")
PRIVATE_STRING(str_no_pae,"\
PAE (Physical Address Extension) Paging is not supported\n\
   [cpuid leaf 80000001h:%edx is lacking #PAE (bit #6)]")
PRIVATE_STRING(str_no_pse,"\
PSE (Page Size Extension) Paging is not supported\n\
   [cpuid leaf 80000001h:%edx is lacking #PSE (bit #3)]")
PRIVATE_STRING(str_no_msr,"\
MSR (Model Specific Registers) are not implemented\n\
   [cpuid leaf 80000001h:%edx is lacking #PSE (bit #3)]")
PRIVATE_STRING(str_bootwarning_1,"WARNING: ")
PRIVATE_STRING(str_bootwarning_2,".\n\
KOS may behave unexpectedly, crash, or not be able to boot at all.\n\n")
PRIVATE_STRING(str_no_sse,"SSE is not supported by the host")
PRIVATE_STRING(str_no_sse2,"SSE2 is not supported by the host")
PRIVATE_STRING(str_no_mmx,"MMX instruction are not provided by the host")
PRIVATE_STRING(str_no_fpu,"No on-board FPU provided by the host")
PRIVATE_STRING(str_no_tsc,"The <RDTSC> instruction is not provided by the host")
PRIVATE_STRING(str_no_cmov,"The <CMOVcc> instructions are not provided by the host")
PRIVATE_STRING(str_no_cx8,"The <cmpxchg8> instruction is not provided by the host")
PRIVATE_STRING(str_no_fxsr,"The <fxsave> and <fxrstor> instructions are not provided by the host")
.popsection

/* IN: [reason_string: %edi]
 * Print an error message and abort booting. */
INTERN_ENTRY(boot_failure)
	pushl  %esi
	leal   SYM(str_bootfailure_1), %esi
	call   vga_puts
	popl   %esi
	call   vga_puts
	leal   SYM(str_bootfailure_2), %esi
	call   vga_puts
1:	hlt
	jmp 1b
SYMEND(boot_failure)

/* IN: [reason_string: %edi]
 * CLOBBER: []
 * Print a warning message and return. */
INTERN_ENTRY(boot_warning)
	pushl  %esi
	leal   SYM(str_bootwarning_1), %esi
	call   vga_puts
	popl   %esi
	call   vga_puts
	leal   SYM(str_bootwarning_2), %esi
	call   vga_puts
	/* Only print the warning implication message once */
	movb   $0, SYM(str_bootwarning_2) + 2
	ret
SYMEND(boot_warning)


INTERN_ENTRY(boot_failure_no_cpuid)
	leal   SYM(str_nocpuid), %esi
	jmp    boot_failure
SYMEND(boot_failure_no_cpuid)
INTERN_ENTRY(boot_failure_no_cpuid_0x80000001)
	leal   SYM(str_nocpuid_0x80000001), %esi
	jmp    boot_failure
SYMEND(boot_failure_no_cpuid_0x80000001)
INTERN_ENTRY(boot_failure_no_cpuid_0x1)
	leal   SYM(str_nocpuid_0x1), %esi
	jmp    boot_failure
SYMEND(boot_failure_no_cpuid_0x1)
INTERN_ENTRY(boot_failure_no_longmode)
	leal   SYM(str_no_longmode), %esi
	jmp    boot_failure
SYMEND(boot_failure_no_longmode)
INTERN_ENTRY(boot_failure_no_pae)
	leal   SYM(str_no_pae), %esi
	jmp    boot_failure
SYMEND(boot_failure_no_pae)
INTERN_ENTRY(boot_failure_no_pse)
	leal   SYM(str_no_pse), %esi
	jmp    boot_failure
SYMEND(boot_failure_no_pse)
INTERN_ENTRY(boot_failure_no_msr)
	leal   SYM(str_no_msr), %esi
	jmp    boot_failure
SYMEND(boot_failure_no_msr)
INTERN_ENTRY(boot_warning_no_sse)
	leal   SYM(str_no_sse), %esi
	jmp    boot_warning
SYMEND(boot_warning_no_sse)
INTERN_ENTRY(boot_warning_no_sse2)
	leal   SYM(str_no_sse2), %esi
	jmp    boot_warning
SYMEND(boot_warning_no_sse2)
INTERN_ENTRY(boot_warning_no_mmx)
	leal   SYM(str_no_mmx), %esi
	jmp    boot_warning
SYMEND(boot_warning_no_mmx)
INTERN_ENTRY(boot_warning_no_fpu)
	leal   SYM(str_no_fpu), %esi
	jmp    boot_warning
SYMEND(boot_warning_no_fpu)
INTERN_ENTRY(boot_warning_no_tsc)
	leal   SYM(str_no_tsc), %esi
	jmp    boot_warning
SYMEND(boot_warning_no_tsc)
INTERN_ENTRY(boot_warning_no_cmov)
	leal   SYM(str_no_cmov), %esi
	jmp    boot_warning
SYMEND(boot_warning_no_cmov)
INTERN_ENTRY(boot_warning_no_cx8)
	leal   SYM(str_no_cx8), %esi
	jmp    boot_warning
SYMEND(boot_warning_no_cx8)
INTERN_ENTRY(boot_warning_no_fxsr)
	leal   SYM(str_no_fxsr), %esi
	jmp    boot_warning
SYMEND(boot_warning_no_fxsr)

#define KERNEL_IDENTITY_SIZE   \
	X86_PDIR_E3_INDEX(VM_ADDR2PAGE(KERNEL_CORE_SIZE))

#if KERNEL_IDENTITY_SIZE == 2
#define KERNEL_IDENTITY_FOREACH(FUNC) \
	FUNC(0) \
	FUNC(1)
#else
#error FIXME
#endif



INTERN_ENTRY(_start)
	/* Save the initial register state. */
	movl   %eax,    SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EAX
	movl   %ecx,    SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_ECX
	movl   %edx,    SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EDX
	movl   %ebx,    SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EBX
	movl   %esp,    SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_ESP
	movl   %ebp,    SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EBP
	movl   %esi,    SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_ESI
	movl   %edi,    SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_GPREGS+X86_GPREGS32_OFFSETOF_EDI
	movw   %ds,     SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_SEGMENTS+X86_SEGMENTS32_OFFSETOF_DS
	movw   %es,     SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_SEGMENTS+X86_SEGMENTS32_OFFSETOF_ES
	movw   %fs,     SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_SEGMENTS+X86_SEGMENTS32_OFFSETOF_FS
	movw   %gs,     SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_SEGMENTS+X86_SEGMENTS32_OFFSETOF_GS
	movw   %cs,     SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_IRET+X86_IRREGS_HOST32_OFFSETOF_CS
	movl   $(_start - KERNEL_CORE_BASE), SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_EIP /* Kind-of unnecessary, but eh... */
	leal   SYM(x86_boot_stack) + CONFIG_KERNELSTACK_SIZE, %esp
	pushfl
	movl   0(%esp), %eax
	movl   %eax, SYM(x86_boot_context)+X86_CONTEXT32_OFFSETOF_EFLAGS

	cli  /* Disable interrupts */
	cld  /* Clear the direction bit (required for REP instructions) */

	/* Check if CPUID is supported. */
	xorl  $(EFLAGS_ID), 0(%esp)
	popf
	pushf
	movl  0(%esp), %ebx
	andl  $(~EFLAGS_ID), 0(%esp)
	popf
	andl  $(EFLAGS_ID), %eax
	andl  $(EFLAGS_ID), %ebx
	cmpl  %eax, %ebx
	je    boot_failure_no_cpuid

	/* Check which CPUID leafs are actually implemented. */
	movl  $0x80000000, %eax
	cpuid
	cmpl  $0x80000001, %eax
	jb    boot_failure_no_cpuid_0x80000001
	movl  %eax, SYM(boot_cpu_id_features) + 0  /* ci_0a */

	movl  $0x0, %eax
	cpuid
	cmpl  $0x1, %eax
	jb    boot_failure_no_cpuid_0x1
	movl  %eax, SYM(boot_cpu_id_features) + 4  /* ci_80000000a */

	movl  $0x80000001, %eax
	cpuid
	testl $(CPUID_80000001D_LM),  %edx /* Test the LM bit. */
	jz    boot_failure_no_longmode
	testl $(CPUID_80000001D_PAE), %edx /* Test the PAE bit. */
	jz    boot_failure_no_pae
	testl $(CPUID_80000001D_PSE), %edx /* Test the PSE bit. */
	jz    boot_failure_no_pse
	testl $(CPUID_80000001D_CX8), %edx /* Test the CX8 bit. */
	jnz   1f
	call  boot_warning_no_cx8
1:	testl $(CPUID_80000001D_FXSR), %edx /* Test the FXSR bit. */
	jnz   1f
	call  boot_warning_no_fxsr
1:	movl  %ecx, SYM(boot_cpu_id_features) + 40 /* ci_80000001c */
	movl  %edx, SYM(boot_cpu_id_features) + 44 /* ci_80000001d */


	movl  $0x1, %eax
	cpuid
	testl $(CPUID_1D_MSR),  %edx /* Test the MSR bit. */
	jz    boot_failure_no_msr
	testl $(CPUID_1D_SSE), %edx  /* Check for SSE support */
	jnz   1f
	call  boot_warning_no_sse
1:	testl $(CPUID_1D_SSE2), %edx /* Check for SSE2 support */
	jnz   1f
	call  boot_warning_no_sse2
1:	testl $(CPUID_1D_MMX), %edx  /* Check for MMX support */
	jnz   1f
	call  boot_warning_no_mmx
1:	testl $(CPUID_1D_FPU), %edx  /* Check for FPU support */
	jnz   1f
	call  boot_warning_no_fpu
1:	testl $(CPUID_1D_TSC), %edx  /* Check for TSC support */
	jnz   1f
	call  boot_warning_no_tsc
1:	testl $(CPUID_1D_CMOV), %edx /* Check for CMOV support */
	jnz   1f
	call  boot_warning_no_cmov
1:	testl $(CPUID_1D_PGE), %edx  /* Check for PGE support */
	jnz   1f
	movl  $0,   SYM(x86_page_global)
1:	movl  %eax, SYM(boot_cpu_id_features) + 8  /* ci_1a */
	movl  %ebx, SYM(boot_cpu_id_features) + 12 /* ci_1b */
	movl  %edx, SYM(boot_cpu_id_features) + 16 /* ci_1d */
	movl  %ecx, SYM(boot_cpu_id_features) + 20 /* ci_1c */


#define PAGING_E4_SHARE_INDEX   X86_PDIR_E4_INDEX(KERNEL_BASE_PAGE)
#define PAGING_E4_SHARE_COUNT  (X86_PDIR_E4_COUNT - PAGING_E4_SHARE_INDEX)

	/* Link the statically allocated kernel-share page directory indirection.
	 * This is the portion of the kernel page directory that is never
	 * deallocated and shared between all existing directories, allowing
	 * for the single-level indirection that implements a single location
	 * what needs to be changed to change a memory-mapping in kernel-space:
	 * >> for (i = 0; i < PAGING_E4_SHARE_COUNT; ++i) {
	 * >>     pagedir_kernel_phys.p_e4[PAGING_E4_SHARE_INDEX + i]
	 * >>         .p_data = &pagedir_kernel_share_e3 +
	 * >>         (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED |
	 * >>          X86_PAGE_FWRITE | X86_PAGE_FPRESENT) +
	 * >>          i * PAGESIZE;
	 * >> }
	 */
	leal  pagedir_kernel_phys + (PAGING_E4_SHARE_INDEX * 8), %edi
	movl  $PAGING_E4_SHARE_COUNT, %ecx
	leal  SYM(pagedir_kernel_share_e3) + \
	( \
	      X86_PAGE_FDIRTY | \
	      X86_PAGE_FACCESSED | \
	      X86_PAGE_FWRITE | \
	      X86_PAGE_FPRESENT \
	), %edx
	orl   SYM(x86_page_global), %edx
1:	movl  %edx, %eax
	addl  $(X86_PDIR_E3_COUNT * 8), %edx
	stosl
	xorl  %eax, %eax
	stosl
	loop  1b

#ifndef CONFIG_NO_GIGABYTE_PAGES
	testl $CPUID_80000001D_PDPE1GB, SYM(boot_cpu_id_features) + 44 /* ci_80000001d */
	jz    .Lsetup_2mib_pages

	/* NOTE: If the host supports 1Gib pages, we can create this
	 *       identity mapping using those instead of having to
	 *       construct 512 times as many 2Mib pages! */
#define IDENTITY_MAP_HIGH(i) \
	movl  $( \
	      (i * X86_PDIR_E2_TOTALSIZE) + \
	      (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED | \
	       X86_PAGE_FWRITE | X86_PAGE_FPRESENT | \
	       X86_PAGE_F1GIB) \
	),    %eax; \
	orl   SYM(x86_page_global), %eax; \
	movl  %eax, SYM(pagedir_kernel_share_e3) + ((PAGING_E4_SHARE_COUNT - (KERNEL_IDENTITY_SIZE - i)) * 8);
	KERNEL_IDENTITY_FOREACH(IDENTITY_MAP_HIGH)
#undef IDENTITY_MAP_HIGH
	jmp   .Ldone_2mib_pages
.Lsetup_2mib_pages:
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
	/* Map the E2-identity vectors (which also contain the kernel itself)
	 * onto the last 2 entries in `pagedir_kernel_share_e3'.
	 * Each E2-vector describes 1Gib, meaning that this way we map the
	 * first 2 Gib of physical memory to the last 2 Gib of virtual memory,
	 * following the requirement of the kernel being loaded to that location.
	 * >> pagedir_kernel_share_e3[PAGING_E4_SHARE_COUNT-1][X86_PDIR_E3_COUNT-2] = pagedir_kernel_share_e2[0] + ...;
	 * >> pagedir_kernel_share_e3[PAGING_E4_SHARE_COUNT-1][X86_PDIR_E3_COUNT-1] = pagedir_kernel_share_e2[1] + ...;
	 */
#define IDENTITY_MAP_HIGH(i) \
	movl  $(SYM(pagedir_kernel_share_e2) + \
	      (i * (X86_PDIR_E2_COUNT * 8)) + \
	      X86_PAGE_FDIRTY | X86_PAGE_FACCESSED | \
	      X86_PAGE_FWRITE | X86_PAGE_FPRESENT \
	),    %eax; \
	orl   SYM(x86_page_global), %eax; \
	movl  %eax, SYM(pagedir_kernel_share_e3) + ((PAGING_E4_SHARE_COUNT * X86_PDIR_E3_COUNT) - (KERNEL_IDENTITY_SIZE - i)) * 8;
	KERNEL_IDENTITY_FOREACH(IDENTITY_MAP_HIGH)
#undef IDENTITY_MAP_HIGH
	/* With our 2 E2-identity vectors now linked, we must still map
	 * them to referr to the first 2 Gib of physical memory:
	 * >> for (i = 0; i < KERNEL_IDENTITY_SIZE * X86_PDIR_E2_COUNT; ++i) {
	 * >>     ((union x86_pdir_e2 *)pagedir_kernel_share_e2)[i]
	 * >>         .p_addr = (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED |
	 * >>                    X86_PAGE_FWRITE | X86_PAGE_FPRESENT |
	 * >>                    X86_PAGE_F2MIB) +
	 * >>                    i * X86_PDIR_E1_TOTALSIZE; // i * 2Mib
	 * >> } */
	movl  $(KERNEL_IDENTITY_SIZE * X86_PDIR_E2_COUNT), %ecx
	leal  SYM(pagedir_kernel_share_e2), %edi
	leal  ( \
	      X86_PAGE_FDIRTY | X86_PAGE_FACCESSED | \
	      X86_PAGE_FWRITE | X86_PAGE_FPRESENT | \
	      X86_PAGE_F2MIB \
	) + 0, %edx /* Map physical memory */
1:	movl  %edx, %eax
	addl  $(X86_PDIR_E2_SIZE), %edx /* EDX += 2Mib */
	stosl
	xorl  %eax, %eax
	stosl
	loop  1b
.Ldone_2mib_pages:


	/* Prepare for enabling paging. */
	leal  pagedir_kernel_phys, %eax
	movl  %eax, %cr3

	/* Configure the %CR4 register. */
	movl  %cr4, %eax
	/* Enable PageSizeExtension (required for 2Mib pages & long-mode),
	 * as well as PhysicalAddressExtension (required for the 48-bit
	 * physical address space needed in long-mode) */
	orl   $(CR4_PSE|CR4_PAE|CR4_OSXMMEXCPT), %eax
	/* XXX: How do we detect if `CR4_OSXMMEXCPT' is supported? */
	testl $CPUID_80000001D_FXSR, SYM(boot_cpu_id_features) + 44
	jz    1f
	orl   $(CR4_OSFXSR), %eax
1:	/* Enable PGE if supported by the host */
	testl $CPUID_80000001D_PGE, SYM(boot_cpu_id_features) + 44
	jz    1f
	orl   $(CR4_PGE), %eax
1:	movl  %eax, %cr4

	/* Set the LME bit in the EFER MSR register. */
	movl  $IA32_EFER, %ecx
	rdmsr
	orl   $IA32_EFER_LME, %eax
	/* Since we're already here, try to enable some other long-mode extensions,
	 * such as the NXE bit, as well as SCE (SysCallExtensions) */
	testl $CPUID_80000001D_NX, SYM(boot_cpu_id_features) + 44 /* ci_80000001d */
	jz    1f
	orl   $IA32_EFER_NXE, %eax
1:	testl $CPUID_80000001D_SYSCALL, SYM(boot_cpu_id_features) + 44 /* ci_80000001d */
	jz    1f
	orl   $IA32_EFER_SCE, %eax
1:	wrmsr /* Save the new configuration. */


	/* Now would be the time to enable paging for the first time.
	 * However, if you take a look at how we initialized the kernel
	 * page directory, you may notice a problem:
	 *    Right now, we're running in physical, low memory. However
	 *    the location at which we are running isn't actually mapped
	 *    in the page directory, meaning that if we were to enable
	 *    paging _right_ _now_, we'd just triple fault.
	 * -> Because of that, we must first create a tiny identity
	 *    mapping of the first 2 Gigabyte, so our own code remains
	 *    mapped and we can continue setting up long mode.
	 * >> pagedir_kernel_phys.p_e4[0].p_data =
	 * >>       (u64)pagedir_kernel_share_e3[0] +
	 * >>       (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED |
	 * >>        X86_PAGE_FWRITE | X86_PAGE_FPRESENT);
	 * >> pagedir_kernel_share_e3[0][0] = pagedir_kernel_share_e2[0] + ...;
	 * >> pagedir_kernel_share_e3[0][1] = pagedir_kernel_share_e2[1] + ...;
	 * >>
	 */
	movl  $(SYM(pagedir_kernel_share_e3) + \
	      (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED | \
	       X86_PAGE_FWRITE | X86_PAGE_FPRESENT) \
	), pagedir_kernel_phys + (0 * 8)
#ifndef CONFIG_NO_GIGABYTE_PAGES
	testl $CPUID_80000001D_PDPE1GB, SYM(boot_cpu_id_features) + 44 /* ci_80000001d */
	jz    1f
#define IDENTITY_MAP_LOW(i) \
	movl  $( \
	      (i * X86_PDIR_E2_TOTALSIZE) + /* The first Gigabyte */ \
	      (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED | \
	       X86_PAGE_FWRITE | X86_PAGE_FPRESENT | \
	       X86_PAGE_F1GIB) \
	), SYM(pagedir_kernel_share_e3) + (i * 8);
	KERNEL_IDENTITY_FOREACH(IDENTITY_MAP_LOW)
#undef IDENTITY_MAP_LOW
	jmp   2f
1:
#endif /* !CONFIG_NO_GIGABYTE_PAGES */
#define IDENTITY_MAP_LOW(i) \
	movl  $(SYM(pagedir_kernel_share_e2) + \
	      (i * X86_PDIR_E2_COUNT) + \
	      (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED | \
	       X86_PAGE_FWRITE | X86_PAGE_FPRESENT) \
	), SYM(pagedir_kernel_share_e3) + (i * 8);
	KERNEL_IDENTITY_FOREACH(IDENTITY_MAP_LOW)
#undef IDENTITY_MAP_LOW
2:


#define PAGING_E4_IDENTITY_INDEX  X86_PDIR_E4_INDEX(VM_ADDR2PAGE(X86_PDIR_E1_IDENTITY_BASE))
	/* One last thing before we enable paging: Add the page-directory self-mapping
	 * for the kernel page-directory, thus allowing us to easily manipulate the
	 * page directory later, without the need of any special early-boot behavior
	 * in regard to the kernel page directory.
	 * >> pagedir_kernel_phys.p_e4[PAGING_E4_IDENTITY_INDEX].p_data =
	 * >>      (u64)phys_self |
	 * >>          (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED |
	 * >>           X86_PAGE_FWRITE | X86_PAGE_FPRESENT);
	 */
	movl  $(pagedir_kernel_phys + \
	      (X86_PAGE_FDIRTY | X86_PAGE_FACCESSED | \
	       X86_PAGE_FWRITE | X86_PAGE_FPRESENT) \
	), pagedir_kernel_phys + (PAGING_E4_IDENTITY_INDEX * 8)

	/* All right! Now that we're identitiy mapped, we should be
	 * safe to actually enable paging for the first time.
	 * HINT: Right now our VM looks like this:
	 *     0000000000000000..000000007fffffff --> 0000000000000000..000000007fffffff
	 *     0000007f80000000..0000007fffffffff --> 0000000000000000..000000007fffffff (Redundant, unavoidable, and never used)
	 *     ffff808000000000..ffff80ffffffffff --> <recursive page-directory self-mapping>
	 *     ffffff8000000000..ffffff807fffffff --> 0000000000000000..000000007fffffff (Redundant, unavoidable, and never used)
	 *     ffffffff80000000..ffffffffffffffff --> 0000000000000000..000000007fffffff
	 * NOTE: The 2 redundant mappings only exist because the same E3-vector is used
	 *       to both create the identity mapping at -2Gb, as well as the one at ZERO(0).
	 *       Because of this, it contains an identity mapping at index 0+1 and 254+255.
	 *       Combine that with the fact that it is mapped at E4-index 0 and 255, and you
	 *       get a total of 4 identity mappings, only 2 of which are ever used (#0 + #3),
	 *       and only one of which is actually kept around (#3)
	 */
	movl  %cr0, %eax
	orl   $CR0_PG, %eax
	/* Also try to enable SSE while we're at it. */
	testl $(CPUID_1D_SSE), SYM(boot_cpu_id_features) + 16 /* ci_1d */
	jz    1f
	andl  $~CR0_EM, %eax
	orl   $CR0_MP, %eax
1:	movl  %eax, %cr0

	/* Even though paging's been enabled now, we can't unmap the unused identity mappings
	 * just yet, because we're still running from low-memory and can't just to our target
	 * located at the end of the 64-bit address space until we've actually went into long
	 * mode. */


	/* Continue by loading a GDT we can use to jump to long mode. */
.pushsection .rodata.free
.align 8
PRIVATE_OBJECT(bootstrap_gdt_vector)
#define SEGMENT(base,size,config) \
	(__ASM_X86_SEG_ENCODELO_32(base,size,config) | \
	(__ASM_X86_SEG_ENCODEHI_32(base,size,config) << 32))
#define COMPAT_SEG_CODE_PL0 (X86_SEG_FLAG_LONGMODE|X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(0)|X86_SEG_ACCESS_EXECUTE|X86_SEG_ACCESS_RW)
#define COMPAT_SEG_DATA_PL0 (X86_SEG_FLAG_LONGMODE|X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(0)|X86_SEG_ACCESS_RW)
	.quad SEGMENT(0,0,0)
	.quad SEGMENT(0,X86_SEG_LIMIT_MAX,COMPAT_SEG_CODE_PL0) /* code */
	.quad SEGMENT(0,X86_SEG_LIMIT_MAX,COMPAT_SEG_DATA_PL0) /* data */
.bootstrap_gdt_vector_end = .
SYMEND(bootstrap_gdt_vector)
.align 2
PRIVATE_OBJECT(bootstrap_gdt)
	.word (.bootstrap_gdt_vector_end - bootstrap_gdt_vector)-1
	.long  SYM(bootstrap_gdt_vector)
SYMEND(bootstrap_gdt)
.popsection

	lgdt  SYM(bootstrap_gdt)

	/* Load 64-bit segment registers. */
	movw  $0x10, %ax
	movw  %ax, %ds
	movw  %ax, %es
	movw  %ax, %fs
	movw  %ax, %gs
	movw  %ax, %ss

	/* And finally, jump into 64-bit mode! */
	ljmpl $0x08, $SYM(_start64)
SYMEND(_start)


.code64
.section .text.free
INTERN_ENTRY(_start64)
	/* Do an absolute jump to get out of physical
	 * memory and into the virtual world! */
	movabs $1f, %rax
	jmpq  *%rax
1:	/* Also reload our stack-pointer for its virtual version. */
	leaq   x86_boot_stack + CONFIG_KERNELSTACK_SIZE, %rsp
	/* Now we're truly in virtual, 64-bit mode! */

	/* Unmap the 3 unused identity mappings. */
	movq  $X86_PAGE_ABSENT, pagedir_kernel + (0 * 8)
#define IDENTITY_MAP_LOW(i) \
	movq  $X86_PAGE_ABSENT, pagedir_kernel_share_e3 + (i * 8);
	KERNEL_IDENTITY_FOREACH(IDENTITY_MAP_LOW)
#undef IDENTITY_MAP_LOW

	/* Copy per-component templates into active data buffers.
	 * This must be done before we load the GDT, because the
	 * kernel's boot-cpu GDT is apart of one of those buffers. */

#ifndef CONFIG_NO_SMP
	leaq   boot_cpu_start,      %rdi
	leaq   kernel_percpu_start, %rsi
	movq   $kernel_percpu_size, %rdx
	call   memcpy
#endif /* !CONFIG_NO_SMP */

	leaq   boot_task_start,      %rdi
	leaq   kernel_pertask_start, %rsi
	movq   $kernel_pertask_size, %rdx
	call   memcpy

	leaq   boot_vm_start,        %rdi
	leaq   kernel_pervm_start,   %rsi
	movq   $kernel_pervm_size,   %rdx
	call   memcpy

	/* Setup proper values for EFLAGS (Shouldn't be necessary...). */
	/* NOTE: This is done to ensure that the NT flag isn't set
	 *      (Which would break the IRETQ used to set CS below). */
	pushq  $EFLAGS_IOPL(0)
	popfq

	/* Load the ~real~ 64-bit GDT (from virtual memory) */
.pushsection .rodata.free
.align 2
1:	.word  (X86_SEG_BUILTIN * 16) - 1
	.quad  boot_cpu_x86_cpugdt
.popsection
	lgdt   1b

	/* Now load all the segments with indices pointing into the 64-bit GDT */
	movq   $X86_KERNEL_DS, %rax
	movw   %ax, %ds /* XXX: Why do DS and ES still exist? */
	movw   %ax, %es
	movw   %ax, %fs
	movq   $X86_SEG_HOST_GS, %rax
	movw   %ax, %gs

	/* Don't forget about this segment register! */
	movw   $(X86_SEG(X86_SEG_CPUTSS) | 3), %ax
	ltr    %ax

	/* With the TLS segment registers all set, we still have
	 * to set the current GD_BASE value to the boot_task_start.
	 * Normally, we'd just use `wrgsbaseq' for this, however
	 * at this point we havn't yet checked if that instruction
	 * even exists (if it doesn't all kernel-space uses get
	 * replaced with a function call that emulates the behavior).
	 * However that relocation is done later, so we must still
	 * rely on MSRs to write that register! */
	leaq   boot_task_start, %rdx
	movl   %edx, %eax
	shrq   $32,  %rdx
	movl   $IA32_GS_BASE, %ecx
	wrmsr

	/* Load SS and CS. In PMODE, we'd use ljmp for this, but that
	 * instruction doesn't exist in LONGMODE, so we need to use an IRET.
	 * That's not too bad though, considering we can also use that to
	 * jump to the C entry point in `boot.c'! */
	pushq  $X86_SEG_HOST_SS
	pushq  $x86_boot_stack + CONFIG_KERNELSTACK_SIZE
	pushq  $EFLAGS_IOPL(0)
	pushq  $X86_SEG_HOST_CS
	pushq  $x86_kernel_main
	iretq
SYMEND(_start64)

