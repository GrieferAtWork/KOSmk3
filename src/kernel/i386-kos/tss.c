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
#ifndef GUARD_KERNEL_I386_KOS_TSS_C
#define GUARD_KERNEL_I386_KOS_TSS_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <i386-kos/tss.h>
#include <i386-kos/gdt.h>
#include <i386-kos/memory.h>
#include <kernel/paging.h>
#include <asm/cpu-flags.h>

DECL_BEGIN

INTDEF byte_t x86_boot_stack_top[];

PUBLIC ATTR_PERCPU struct x86_tss x86_cputss = {
#ifdef __x86_64__
    .t_rsp0       = (uintptr_t)x86_boot_stack_top,
#else
    .t_esp0       = (uintptr_t)x86_boot_stack_top,
    .t_ss0        = X86_KERNEL_DS,
    .t_eflags     = 0,
    .t_es         = X86_KERNEL_DS,
    .t_cs         = X86_KERNEL_CS,
    .t_ss         = X86_KERNEL_DS,
    .t_ds         = X86_KERNEL_DS,
    .t_fs         = X86_SEG_FS,
    .t_gs         = X86_SEG_GS,
    .t_ldtr       = X86_SEG(X86_SEG_KERNEL_LDT),
#endif
    .t_iomap_base = sizeof(struct x86_tss)
};

#ifndef __x86_64__
INTDEF void KCALL x86_double_fault_handler(void);

/* Considerably smaller than a regular stack. */
INTERN ATTR_SECTION(".bss.boot.df_stack")
byte_t x86_bootcpu_df_stack[CONFIG_X86_DFSTACK_SIZE];

PUBLIC ATTR_PERCPU struct x86_tss x86_cputssdf = {
    .t_esp0       = (uintptr_t)COMPILER_ENDOF(x86_bootcpu_df_stack),
    .t_ss0        = X86_KERNEL_DS,
    .t_eflags     = 0,
    .t_cr3        = (u32)(uintptr_t)&pagedir_kernel_phys,
    .t_eip        = (u32)(uintptr_t)&x86_double_fault_handler,
    .t_esp        = (u32)(uintptr_t)COMPILER_ENDOF(x86_bootcpu_df_stack),
    .t_es         = X86_KERNEL_DS,
    .t_cs         = X86_KERNEL_CS,
    .t_ss         = X86_KERNEL_DS,
    .t_ds         = X86_KERNEL_DS,
    .t_fs         = X86_SEG_FS,
    .t_gs         = X86_SEG_GS,
    .t_ldtr       = X86_SEG(X86_SEG_KERNEL_LDT),
    .t_iomap_base = sizeof(struct x86_tss)
};
#endif


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_TSS_C */
