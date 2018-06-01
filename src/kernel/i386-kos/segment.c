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
#ifndef GUARD_KERNEL_I386_KOS_SEGMENT_C
#define GUARD_KERNEL_I386_KOS_SEGMENT_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>

#ifdef __x86_64__
#include <hybrid/section.h>
#include <i386-kos/segment.h>
#include <i386-kos/cpuid.h>
#include <i386-kos/paging64.h>
#include <asm/cpu-flags.h>
#include <kos/intrin.h>
#include <kernel/debug.h>

DECL_BEGIN

INTDEF u32 kernel_fixup_fsgsbase_start[];
INTDEF u32 kernel_fixup_fsgsbase_end[];


#define FOREACH_GP_REGISTER(FUNC) \
    FUNC(rax) \
    FUNC(rcx) \
    FUNC(rdx) \
    FUNC(rbx) \
    FUNC(rsp) \
    FUNC(rbp) \
    FUNC(rsi) \
    FUNC(rdi) \
    FUNC(r8) \
    FUNC(r9) \
    FUNC(r10) \
    FUNC(r11) \
    FUNC(r12) \
    FUNC(r13) \
    FUNC(r14) \
    FUNC(r15) \
/**/


#define X86_DEFINE_FSGSBASE_SYMBOLS(name) \
INTDEF byte_t x86_rdfsbase_##name[],x86_rdgsbase_##name[]; \
INTDEF byte_t x86_wrfsbase_##name[],x86_wrgsbase_##name[];
FOREACH_GP_REGISTER(X86_DEFINE_FSGSBASE_SYMBOLS)
#undef X86_DEFINE_FSGSBASE_SYMBOLS

struct register_lookup_table {
    uintptr_t rlt_regs[16];
};

PRIVATE ATTR_FREERODATA struct register_lookup_table const rlt_rdfsbase = {{
#define X86_LINK_RDFSBASE_REGISTERS(name) (uintptr_t)x86_rdfsbase_##name,
FOREACH_GP_REGISTER(X86_LINK_RDFSBASE_REGISTERS)
#undef X86_LINK_RDFSBASE_REGISTERS
}};
PRIVATE ATTR_FREERODATA struct register_lookup_table const rlt_rdgsbase = {{
#define X86_LINK_RDGSBASE_REGISTERS(name) (uintptr_t)x86_rdgsbase_##name,
FOREACH_GP_REGISTER(X86_LINK_RDGSBASE_REGISTERS)
#undef X86_LINK_RDGSBASE_REGISTERS
}};
PRIVATE ATTR_FREERODATA struct register_lookup_table const rlt_wrfsbase = {{
#define X86_LINK_WRFSBASE_REGISTERS(name) (uintptr_t)x86_wrfsbase_##name,
FOREACH_GP_REGISTER(X86_LINK_WRFSBASE_REGISTERS)
#undef X86_LINK_WRFSBASE_REGISTERS
}};
PRIVATE ATTR_FREERODATA struct register_lookup_table const rlt_wrgsbase = {{
#define X86_LINK_WRGSBASE_REGISTERS(name) (uintptr_t)x86_wrgsbase_##name,
FOREACH_GP_REGISTER(X86_LINK_WRGSBASE_REGISTERS)
#undef X86_LINK_WRGSBASE_REGISTERS
}};



INTERN ATTR_FREETEXT void KCALL x86_fixup_fsgsbase(void) {
 u32 *iter;
#if 0
 /* If the CPU supports these instructions
  * natively, then there's nothing to do for us! */
 if (CPU_FEATURES.ci_7b & CPUID_7B_FSGSBASE) {
  /* Enable access to fs/gs-base instruction in user-space. */
  __wrcr4(__rdcr4() | CR4_FSGSBASE);
  return;
 }
#endif
 debug_printf("[X86] Fixup FS/GS_BASE instruction\n");
 for (iter = kernel_fixup_fsgsbase_start;
      iter < kernel_fixup_fsgsbase_end; ++iter) {
  byte_t *code = (byte_t *)(KERNEL_CORE_BASE + *iter);
  uintptr_t target; unsigned int regno;
  assertf(code[0] == 0xf3 &&
         (code[1] == 0x48 || code[1] == 0x49) &&
          code[2] == 0x0f && code[3] == 0xae &&
          code[4] >= 0xc0 && code[4] <= 0xdf,
          "Not an `(rd|wr)(gs|gs)base' instruction");
  regno  = (code[1] & 1) << 3;
  regno |= (code[4] & 7);
  switch (code[4] & ~7) {
  case 0xc0: target = rlt_rdfsbase.rlt_regs[regno]; break;
  case 0xc8: target = rlt_rdgsbase.rlt_regs[regno]; break;
  case 0xd0: target = rlt_wrfsbase.rlt_regs[regno]; break;
  case 0xd8: target = rlt_wrgsbase.rlt_regs[regno]; break;
  default: __builtin_unreachable();
  }
  /* Generate a call instruction. */
  *code++ = 0xe8;
  code += 4;
  ((u32 *)code)[-1] = (u32)((uintptr_t)target-(uintptr_t)code);
 }
}


DECL_END
#endif

#endif /* !GUARD_KERNEL_I386_KOS_SEGMENT_C */
