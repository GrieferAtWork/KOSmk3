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
#ifndef GUARD_KERNEL_I386_KOS_SYSENTER_C
#define GUARD_KERNEL_I386_KOS_SYSENTER_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <i386-kos/syscall.h>
#include <i386-kos/gdt.h>
#include <i386-kos/cpuid.h>
#include <i386-kos/tss.h>
#include <kernel/debug.h>
#include <hybrid/section.h>
#include <asm/cpu-flags.h>
#include <syscall.h>
#include <kos/ushare.h>
#include <string.h>
#include <kos/intrin.h>

#ifndef CONFIG_NO_X86_SYSENTER

DECL_BEGIN

/* The `SYSENTER_CS_MSR' register requires this layout. */
STATIC_ASSERT(X86_SEG_HOST_DATA == X86_SEG_HOST_CODE+1);
STATIC_ASSERT(X86_SEG_USER_CODE == X86_SEG_HOST_CODE+2);
STATIC_ASSERT(X86_SEG_USER_DATA == X86_SEG_HOST_CODE+3);

INTDEF byte_t x86_ushare_sysenter[];
INTDEF byte_t x86_fast_sysenter[];
INTDEF byte_t x86_fast_sysenter_size[];
INTERN byte_t *x86_sysenter_ushare_base = x86_ushare_sysenter;
INTDEF byte_t x86_sysexit_fixup_1[];
INTDEF byte_t x86_sysexit_fixup_2[];


INTDEF void ASMCALL sysenter_kernel_entry(void);
INTERN ATTR_FREETEXT void KCALL x86_initialize_sysenter(void) {
 struct cpu_cpuid *feat = (struct cpu_cpuid *)&CPU_FEATURES;
 if (!(feat->ci_1d & CPUID_1D_SEP)) {
  if (THIS_CPU == &_boot_cpu) {
   x86_sysexit_fixup_1[0] = 0xcf; /* iret */
   x86_sysexit_fixup_2[0] = 0xcf; /* iret */
  }
  return; /* Not available. */
 }
 /* Write sysenter-specific MSRs */
 __wrmsr(SYSENTER_CS_MSR,X86_KERNEL_CS);
#ifdef __x86_64__
 __wrmsr(SYSENTER_ESP_MSR,(uintptr_t)&PERCPU(x86_cputss).t_rsp0);
#else
 __wrmsr(SYSENTER_ESP_MSR,(uintptr_t)&PERCPU(x86_cputss).t_esp0);
#endif
 __wrmsr(SYSENTER_EIP_MSR,(uintptr_t)&sysenter_kernel_entry);

 if (THIS_CPU == &_boot_cpu) {
  /* The boot CPU is responsible for re-writing the ushare
   * segment containing the system call entry points. */
  memcpy(x86_ushare_sysenter,x86_fast_sysenter,
        (size_t)x86_fast_sysenter_size);
  x86_sysenter_ushare_base = x86_fast_sysenter;
 }
}

DECL_END

#endif /* !CONFIG_NO_X86_SYSENTER */

#endif /* !GUARD_KERNEL_I386_KOS_SYSENTER_C */
