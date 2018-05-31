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
#ifndef GUARD_KERNEL_I386_KOS_GDT_C
#define GUARD_KERNEL_I386_KOS_GDT_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/intrin.h>
#include <kernel/sections.h>
#include <i386-kos/gdt.h>
#include <kos/thread.h>
#include <sched/task.h>

DECL_BEGIN

#ifdef __x86_64__
STATIC_ASSERT_MSG(X86_SEG_USER_CS32 == 0x53,
                 "Update `interrupt_iscompat()'");
#endif

INTDEF byte_t _x86_gdt_tss_lo[];
INTDEF byte_t _x86_gdt_tss_hi[];
#ifndef __x86_64__
INTDEF byte_t _x86_gdt_tssdf_lo[];
INTDEF byte_t _x86_gdt_tssdf_hi[];
#endif
INTDEF byte_t _x86_gdt_tls_lo[];
INTDEF byte_t _x86_gdt_tls_hi[];

PUBLIC ATTR_PERCPU struct x86_segment x86_cpugdt[X86_SEG_BUILTIN] = {
    [X86_SEG_NULL]        = X86_SEGMENT_INIT(0,0,0), /* NULL segment */
    [X86_SEG_HOST_CODE]   = X86_SEGMENT_INIT(0,X86_SEG_LIMIT_MAX,X86_SEG_CODE_PL0), /* Kernel code segment */
    [X86_SEG_HOST_DATA]   = X86_SEGMENT_INIT(0,X86_SEG_LIMIT_MAX,X86_SEG_DATA_PL0), /* Kernel data segment */
    [X86_SEG_USER_CODE]   = X86_SEGMENT_INIT(0,X86_SEG_LIMIT_MAX,X86_SEG_CODE_PL3), /* User code */
    [X86_SEG_USER_DATA]   = X86_SEGMENT_INIT(0,X86_SEG_LIMIT_MAX,X86_SEG_DATA_PL3), /* User data */
#ifdef __x86_64__
    [X86_SEG_USER_CODE32] = X86_SEGMENT_INIT(0,X86_SEG_LIMIT_MAX,X86_SEG_CODE_PL3_32), /* Ring #3 32-bit (compatibility mode) code segment. */
    [X86_SEG_USER_DATA32] = X86_SEGMENT_INIT(0,X86_SEG_LIMIT_MAX,X86_SEG_DATA_PL3_32), /* Ring #3 32-bit (compatibility mode) data segment. */
#else
    [X86_SEG_HOST_CODE16] = X86_SEGMENT_INIT(0,X86_SEG_LIMIT_MAX,X86_SEG_CODE_PL0_16), /* 16-bit kernel code segment. */
    [X86_SEG_HOST_DATA16] = X86_SEGMENT_INIT(0,X86_SEG_LIMIT_MAX,X86_SEG_DATA_PL0_16), /* 16-bit kernel data segment. */
#endif
    [X86_SEG_CPUTSS]      = {{{(uintptr_t)_x86_gdt_tss_lo,(uintptr_t)_x86_gdt_tss_hi}}}, /* CPU TSS */
#ifndef __x86_64__
    [X86_SEG_CPUTSS_DF]   = {{{(uintptr_t)_x86_gdt_tssdf_lo,(uintptr_t)_x86_gdt_tssdf_hi}}}, /* CPU TSS (for #DF) */
#endif
    [X86_SEG_KERNEL_LDT]  = X86_SEGMENT_INIT(0,0,X86_SEG_LDT), /* Kernel LDT table. */
    [X86_SEG_HOST_TLS]    = {{{(uintptr_t)_x86_gdt_tls_lo,(uintptr_t)_x86_gdt_tls_hi}}}, /* task-self */
    [X86_SEG_USER_TLS]    = X86_SEGMENT_INIT(0,X86_SEG_LIMIT_MAX,X86_SEG_DATA_PL3),
#ifndef CONFIG_NO_DOS_COMPAT
    [X86_SEG_USER_TIB]    = X86_SEGMENT_INIT(0,X86_SEG_LIMIT_MAX,X86_SEG_DATA_PL3),
#endif
};

PUBLIC void KCALL set_user_tls_register(void *value) {
#ifdef __x86_64__
 if (interrupt_iscompat()) {
  WR_USER_FSBASE((u64)value);
 } else {
  WR_USER_GSBASE((u64)value);
 }
#else
 struct x86_segment *seg;
 /* Set the base address of the user-space TLS segment. */
 seg = &PERCPU(x86_cpugdt)[X86_SEG_USER_TLS];
 X86_SEGMENT_STBASE(*seg,value);
#ifdef CONFIG_NO_X86_SEGMENTATION
 /* Reload the user TLS segment register immediately
  * if switching back to user-space won't do that. */
 __asm__ __volatile__("movw %w0, %%" PP_STR(__ASM_USERTASK_SEGMENT)
                      :
                      : "r" (X86_USER_TLS));
#endif
#endif
}

#ifndef CONFIG_NO_DOS_COMPAT
PUBLIC void KCALL set_user_tib_register(void *value) {
#ifdef __x86_64__
 if (interrupt_iscompat()) {
  WR_USER_GSBASE((u64)value);
 } else {
  WR_USER_FSBASE((u64)value);
 }
#else
 struct x86_segment *seg;
 /* Set the base address of the user-space TLS segment. */
 seg = &PERCPU(x86_cpugdt)[X86_SEG_USER_TIB];
 X86_SEGMENT_STBASE(*seg,value);
#endif
}
#endif /* !CONFIG_NO_DOS_COMPAT */



DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_GDT_C */
