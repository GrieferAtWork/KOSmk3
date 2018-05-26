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
#ifndef GUARD_KERNEL_I386_KOS_CPUID_C
#define GUARD_KERNEL_I386_KOS_CPUID_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <hybrid/section.h>
#include <kernel/sections.h>
#include <kernel/debug.h>
#include <sched/task.h>
#include <asm/cpu-flags.h>
#include <i386-kos/cpuid.h>
#include <kos/intrin.h>

DECL_BEGIN

PUBLIC ATTR_PERCPU u16 _cpu_basic_features ASMNAME("cpu_basic_features") = CPU_BASIC_FEATURE_FNONE;
PUBLIC ATTR_PERCPU struct cpu_cpuid _cpu_id_features ASMNAME("cpu_id_features") = { 0, };

INTERN ATTR_FREETEXT void KCALL x86_load_cpuid(void) {
 struct cpu_cpuid *info;
#ifndef __x86_64__
 u32 cpuid_available;
 /* Check if CPUID is even available. */
 __asm__ __volatile__("pushfl\n"
                      "pushfl\n"
                      "xorl %1, (%%esp)\n"
                      "popfl\n"
                      "pushfl\n"
                      "popl %0\n"
                      "xorl (%%esp), %0\n"
                      "popfl\n"
                      : "=r" (cpuid_available)
                      : "i" (EFLAGS_ID));
 if (!(cpuid_available & EFLAGS_ID))
       return;
#endif /* !__x86_64__ */
 /* Indicate the availability of CPUID. */
 PERCPU(_cpu_basic_features) |= CPU_BASIC_FEATURE_FCPUID;
 info = &PERCPU(_cpu_id_features);
 __asm__ __volatile__("cpuid"
                      : "=a" (info->ci_0a)
                      , "=c" (info->ci_0c)
                      , "=d" (info->ci_0d)
                      , "=b" (info->ci_0b)
                      : "a" (0));
 __asm__ __volatile__("cpuid"
                      : "=a" (info->ci_80000000a)
                      : "a" (0x80000000)
                      : "ecx", "edx", "ebx");
 if (info->ci_bleaf_max >= 1) {
  __asm__ __volatile__("cpuid"
                       : "=a" (info->ci_1a)
                       , "=b" (info->ci_1b)
                       , "=d" (info->ci_1d)
                       , "=c" (info->ci_1c)
                       : "a" (1));
  /* Not available. (Flag was set accidentally) */
  if (info->ci_version < 0x633)
      info->ci_1d &= ~CPUID_1D_SEP;
 }
 if (info->ci_stepping > 6 ||
    (info->ci_stepping == 6 && info->ci_model >= 0xd)) {
  u32 enabled;
  /* Try to disable CPUID limitations that may have been imposed by the BIOS. */
  enabled = __rdmsr(IA32_MISC_ENABLE);
  if (enabled & IA32_MISC_ENABLE_LIMIT_CPUID) {
   debug_printf(FREESTR("[X86] Disabling limited CPUID\n"));
   enabled &= ~IA32_MISC_ENABLE_LIMIT_CPUID;
   __wrmsr(IA32_MISC_ENABLE,enabled);
   __asm__ __volatile__("cpuid"
                        : "=a" (info->ci_0a)
                        : "a" (0)
                        : "ecx", "edx", "ebx");
  }
 }
 if (info->ci_bleaf_max >= 7) {
  __asm__ __volatile__("cpuid"
                       : "=b" (info->ci_7b)
                       , "=d" (info->ci_7d)
                       , "=c" (info->ci_7c)
                       : "a" (7), "c" (0));
 }
 if (info->ci_eleaf_max >= 0x80000001) {
  __asm__ __volatile__("cpuid"
                       : "=d" (info->ci_80000001d)
                       , "=c" (info->ci_80000001c)
                       : "a" (0x80000001)
                       : "ebx");
 }
 if (info->ci_eleaf_max >= 0x80000004) {
  __asm__ __volatile__("cpuid"
                       : "=a" (info->ci_80000002a)
                       , "=b" (info->ci_80000002b)
                       , "=d" (info->ci_80000002d)
                       , "=c" (info->ci_80000002c)
                       : "a" (0x80000002));
  __asm__ __volatile__("cpuid"
                       : "=a" (info->ci_80000003a)
                       , "=b" (info->ci_80000003b)
                       , "=d" (info->ci_80000003d)
                       , "=c" (info->ci_80000003c)
                       : "a" (0x80000003));
  __asm__ __volatile__("cpuid"
                       : "=a" (info->ci_80000004a)
                       , "=b" (info->ci_80000004b)
                       , "=d" (info->ci_80000004d)
                       , "=c" (info->ci_80000004c)
                       : "a" (0x80000004));
 }


 /* Enable some optional features, if they're supported by the hardware. */
 {
  register_t old_cr4;
  register_t new_cr4;
  old_cr4 = new_cr4 = __rdcr4();

  new_cr4 &= ~(CR4_TSD);

  if ((info->ci_1d & CPUID_1D_PGE) ||
      (info->ci_80000001d & CPUID_80000001D_PGE)) {
   debug_printf(FREESTR("[X86] Enable PGE\n"));
   new_cr4 |= CR4_PGE;
  }
#ifdef __x86_64__
  if (info->ci_7b & CPUID_7B_FSGSBASE) {
   debug_printf(FREESTR("[X86] Enable userspace fs/gs base access\n"));
   new_cr4 |= CR4_FSGSBASE;
  }
#endif
  if (old_cr4 != new_cr4)
      __wrcr4(new_cr4);
 }
#ifdef __x86_64__
 if (info->ci_80000001d & CPUID_80000001D_NX) {
  debug_printf(FREESTR("[X86] Enable NX support\n"));
  __wrmsr(IA32_EFER,__rdmsr(IA32_EFER) | IA32_EFER_NXE);
 }
#endif

}




DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_CPUID_C */
