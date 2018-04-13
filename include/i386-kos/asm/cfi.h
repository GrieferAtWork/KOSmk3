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
#ifndef _X86_KOS_ASM_CFI_H
#define _X86_KOS_ASM_CFI_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/asm.h>


/* Reminder on how `pop' functions with a stack-operand:
 * >> #include <stdio.h>
 * >> int main() {
 * >>     int value;
 * >>     __asm__("pushl $0\n"
 * >>             "pushl $42\n"
 * >>             "popl  0(%%esp)\n" // This overrides the `0' above! (meaning the new SP is used as base address)
 * >>             "popl  %0\n"
 * >>             : "=r" (value));
 * >>     printf("value = %d\n",value); // 42
 * >>     return 0;
 * >> }
 */

GLOBAL_ASM(
L(.macro pushw_cfi arg; pushw \arg; .cfi_adjust_cfa_offset 2; .endm)
L(.macro popw_cfi arg; popw \arg; .cfi_adjust_cfa_offset -2; .endm)
L(.macro pushl_cfi arg; pushl \arg; .cfi_adjust_cfa_offset 4; .endm)
L(.macro popl_cfi arg; popl \arg; .cfi_adjust_cfa_offset -4; .endm)
L(.macro pushfl_cfi; pushfl; .cfi_adjust_cfa_offset 4; .endm)
L(.macro popfl_cfi; popfl; .cfi_adjust_cfa_offset -4; .endm)
L(.macro pushfl_cfi_r; pushfl; .cfi_adjust_cfa_offset 4; .cfi_rel_offset %eflags, 0; .endm)
L(.macro popfl_cfi_r; popfl; .cfi_adjust_cfa_offset -4; .cfi_restore %eflags; .endm)
);

#ifdef __x86_64__
GLOBAL_ASM(
L(.macro pushq_cfi arg; pushq \arg; .cfi_adjust_cfa_offset 8; .endm)
L(.macro popq_cfi arg; popq \arg; .cfi_adjust_cfa_offset -8; .endm)
L(.macro pushq_cfi_r arg; pushq \arg; .cfi_adjust_cfa_offset 8; .cfi_rel_offset \arg, 0; .endm)
L(.macro popq_cfi_r arg; popq \arg; .cfi_adjust_cfa_offset -8; .cfi_restore \arg; .endm)
L(.macro pushfq_cfi; pushfq; .cfi_adjust_cfa_offset 8; .endm)
L(.macro popfq_cfi; popfq; .cfi_adjust_cfa_offset -8; .endm)
L(.macro pushfq_cfi_r; pushfq; .cfi_adjust_cfa_offset 8; .cfi_rel_offset %eflags, 0; .endm)
L(.macro popfq_cfi_r; popfq; .cfi_adjust_cfa_offset -8; .cfi_restore %eflags; .endm)
);
#else
GLOBAL_ASM(
L(.macro pushl_cfi_r arg; pushl \arg; .cfi_adjust_cfa_offset 4; .cfi_rel_offset \arg, 0; .endm)
L(.macro popl_cfi_r arg; popl \arg; .cfi_adjust_cfa_offset -4; .cfi_restore \arg; .endm)
L(.macro pushal_cfi; pushal; .cfi_adjust_cfa_offset 32; .endm)
L(.macro popal_cfi; popal; .cfi_adjust_cfa_offset -32; .endm)
L(.macro pushal_cfi_r; pushal; .cfi_adjust_cfa_offset 32;
  .cfi_rel_offset %edi, 0;  .cfi_rel_offset %esi, 4;  .cfi_rel_offset %ebp, 8; /*.cfi_rel_offset %esp, 12;*/
  .cfi_rel_offset %ebx, 16; .cfi_rel_offset %edx, 20; .cfi_rel_offset %ecx, 24;  .cfi_rel_offset %eax, 28;
  .endm)
L(.macro popal_cfi_r; popal; .cfi_adjust_cfa_offset -32;
  .cfi_restore %edi; .cfi_restore %esi; .cfi_restore %ebp; /*.cfi_restore %esp;*/
  .cfi_restore %ebx; .cfi_restore %edx; .cfi_restore %ecx;   .cfi_restore %eax;
  .endm)
);
#endif


#endif /* !_X86_KOS_ASM_CFI_H */
