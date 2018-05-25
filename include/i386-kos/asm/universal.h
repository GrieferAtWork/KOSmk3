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
#ifndef _X86_KOS_ASM_UNIVERSAL_H
#define _X86_KOS_ASM_UNIVERSAL_H 1

#include <__stdinc.h>
#include <hybrid/asm.h>
#include <hybrid/host.h>
#include "cfi.h"

#ifdef __x86_64__
#define __UNIVERSAL(x) x##q
#define __USIZE        8
#else
#define __UNIVERSAL(x) x##l
#define __USIZE        4
#endif

#ifdef __x86_64__
#define pdi    rdi
#define psi    rsi
#define pbp    rbp
#define psp    rsp
#define pbx    rbx
#define pdx    rdx
#define pcx    rcx
#define pax    rax
#define pip    rip
#define pflags rflags
#else
#define pdi    edi
#define psi    esi
#define pbp    ebp
#define psp    esp
#define pbx    ebx
#define pdx    edx
#define pcx    ecx
#define pax    eax
#define pip    eip
#define pflags eflags
#endif


/* Some instructions that don't exist with a suffix. */
#ifdef __x86_64__
#define rdfsbasel  rdfsbase
#define rdfsbaseq  rdfsbase
#define wrfsbasel  wrfsbase
#define wrfsbaseq  wrfsbase
#define rdgsbasel  rdgsbase
#define rdgsbaseq  rdgsbase
#define wrgsbasel  wrgsbase
#define wrgsbaseq  wrgsbase
#endif

#define aaap      __UNIVERSAL(aaa)
#define aadp      __UNIVERSAL(aad)
#define aamp      __UNIVERSAL(aam)
#define aasp      __UNIVERSAL(aas)
#define adcp      __UNIVERSAL(adc)
#define addp      __UNIVERSAL(add)
#define andp      __UNIVERSAL(and)
#define bsfp      __UNIVERSAL(bsf)
#define bsrp      __UNIVERSAL(bsr)
#define bswapp    __UNIVERSAL(bswap)
#define btcp      __UNIVERSAL(btc)
#define btp       __UNIVERSAL(bt)
#define btrp      __UNIVERSAL(btr)
#define btsp      __UNIVERSAL(bts)
#define callp     __UNIVERSAL(call)
#define cbwp      __UNIVERSAL(cbw)
#define cdqp      __UNIVERSAL(cdq)
#define cflushp   __UNIVERSAL(cflush)
#define cmcp      __UNIVERSAL(cmc)
#define cmovaep   __UNIVERSAL(cmovae)
#define cmovap    __UNIVERSAL(cmova)
#define cmovbep   __UNIVERSAL(cmovbe)
#define cmovbp    __UNIVERSAL(cmovb)
#define cmovcp    __UNIVERSAL(cmovc)
#define cmovep    __UNIVERSAL(cmove)
#define cmovgep   __UNIVERSAL(cmovge)
#define cmovgp    __UNIVERSAL(cmovg)
#define cmovlep   __UNIVERSAL(cmovle)
#define cmovlp    __UNIVERSAL(cmovl)
#define cmovnaep  __UNIVERSAL(cmovnae)
#define cmovnap   __UNIVERSAL(cmovna)
#define cmovnbep  __UNIVERSAL(cmovnbe)
#define cmovnbp   __UNIVERSAL(cmovnb)
#define cmovncp   __UNIVERSAL(cmovnc)
#define cmovnep   __UNIVERSAL(cmovne)
#define cmovngep  __UNIVERSAL(cmovnge)
#define cmovngp   __UNIVERSAL(cmovng)
#define cmovnlep  __UNIVERSAL(cmovnle)
#define cmovnlp   __UNIVERSAL(cmovnl)
#define cmovnop   __UNIVERSAL(cmovno)
#define cmovnpp   __UNIVERSAL(cmovnp)
#define cmovnsp   __UNIVERSAL(cmovns)
#define cmovnzp   __UNIVERSAL(cmovnz)
#define cmovop    __UNIVERSAL(cmovo)
#define cmovpep   __UNIVERSAL(cmovpe)
#define cmovpop   __UNIVERSAL(cmovpo)
#define cmovpp    __UNIVERSAL(cmovp)
#define cmovsp    __UNIVERSAL(cmovs)
#define cmovzp    __UNIVERSAL(cmovz)
#define cmpp      __UNIVERSAL(cmp)
#define cmpsp     __UNIVERSAL(cmps)
#define cmpwp     __UNIVERSAL(cmpw)
#define cmpxchgp  __UNIVERSAL(cmpxchg)
#define cpuidp    __UNIVERSAL(cpuid)
#define cwdep     __UNIVERSAL(cwde)
#define cwdp      __UNIVERSAL(cwd)
#define daap      __UNIVERSAL(daa)
#define dasp      __UNIVERSAL(das)
#define decp      __UNIVERSAL(dec)
#define divp      __UNIVERSAL(div)
#define idivp     __UNIVERSAL(idiv)
#define imulp     __UNIVERSAL(imul)
#define inbp      __UNIVERSAL(inb)
#define incp      __UNIVERSAL(inc)
#define inlp      __UNIVERSAL(inl)
#define inp       __UNIVERSAL(in)
#define insp      __UNIVERSAL(ins)
#define iretdfp   __UNIVERSAL(iretdf)
#define iretp     __UNIVERSAL(iret)
#define jmpp      __UNIVERSAL(jmp)
#define lcallp    __UNIVERSAL(lcall)
#define leap      __UNIVERSAL(lea)
#define leavep    __UNIVERSAL(leave)
#define lgdtp     __UNIVERSAL(lgdt)
#define lidtp     __UNIVERSAL(lidt)
#define ljmpp     __UNIVERSAL(ljmp)
#define lldtp     __UNIVERSAL(lldt)
#define lodsp     __UNIVERSAL(lods)
#define lretp     __UNIVERSAL(lret)
#define movabsp   __UNIVERSAL(movabs)
#define movp      __UNIVERSAL(mov)
#define movsp     __UNIVERSAL(movs)
#define movsbp    __UNIVERSAL(movsb)
#define movslp    __UNIVERSAL(movsl)
#define movswp    __UNIVERSAL(movsw)
#define movsxp    __UNIVERSAL(movsx)
#define movzbp    __UNIVERSAL(movzb)
#define movzlp    __UNIVERSAL(movzl)
#define movzwp    __UNIVERSAL(movzw)
#define movzxp    __UNIVERSAL(movzx)
#define mulp      __UNIVERSAL(mul)
#define negp      __UNIVERSAL(neg)
#define nopp      __UNIVERSAL(nop)
#define notp      __UNIVERSAL(not)
#define orp       __UNIVERSAL(or)
#define outp      __UNIVERSAL(out)
#define outsp     __UNIVERSAL(outs)
#define outwp     __UNIVERSAL(outw)
#define popfp     __UNIVERSAL(popf)
#define popp      __UNIVERSAL(pop)
#define pushfp    __UNIVERSAL(pushf)
#define pushp     __UNIVERSAL(push)
#define rclp      __UNIVERSAL(rcl)
#define rcrp      __UNIVERSAL(rcr)
#define retfp     __UNIVERSAL(retf)
#define retnp     __UNIVERSAL(retn)
#define retp      __UNIVERSAL(ret)
#define rolp      __UNIVERSAL(rol)
#define rorp      __UNIVERSAL(ror)
#define salp      __UNIVERSAL(sal)
#define sarp      __UNIVERSAL(sar)
#define sbbp      __UNIVERSAL(sbb)
#define scasp     __UNIVERSAL(scas)
#define sgdtp     __UNIVERSAL(sgdt)
#define shlp      __UNIVERSAL(shl)
#define shrp      __UNIVERSAL(shr)
#define sidtp     __UNIVERSAL(sidt)
#define sldtp     __UNIVERSAL(sldt)
#define stosp     __UNIVERSAL(stos)
#define strp      __UNIVERSAL(str)
#define subp      __UNIVERSAL(sub)
#define testp     __UNIVERSAL(test)
#define xaddp     __UNIVERSAL(xadd)
#define xbtsp     __UNIVERSAL(xbts)
#define xchgp     __UNIVERSAL(xchg)
#define xorp      __UNIVERSAL(xor)




#ifdef __ASSEMBLER__

#ifdef __x86_64__
/* Mov a 32-bit memory location `mem' into a 64-bit register `reg'
 * On x86_64, a regular mov to the same 32-bit register normally
 * clears the upper 32 bits. However this behavior is often hard
 * to realize, meaning that source assembly relying on this behavior
 * should make use of this `movzlq' */
.macro movzlq mem, reg
.ifc \reg,%rax; movl \mem, %eax; .else
.ifc \reg,%rcx; movl \mem, %ecx; .else
.ifc \reg,%rdx; movl \mem, %edx; .else
.ifc \reg,%rbx; movl \mem, %ebx; .else
.ifc \reg,%rsp; movl \mem, %esp; .else
.ifc \reg,%rbp; movl \mem, %ebp; .else
.ifc \reg,%rsi; movl \mem, %esi; .else
.ifc \reg,%rdi; movl \mem, %edi; .else
.ifc \reg,%r8;  movl \mem, %r8d; .else
.ifc \reg,%r9;  movl \mem, %r9d; .else
.ifc \reg,%r10; movl \mem, %r10d; .else
.ifc \reg,%r11; movl \mem, %r11d; .else
.ifc \reg,%r12; movl \mem, %r12d; .else
.ifc \reg,%r13; movl \mem, %r13d; .else
.ifc \reg,%r14; movl \mem, %r14d; .else
.ifc \reg,%r15; movl \mem, %r15d; .else
.error "Invalid 64-bit x86_64 register: `\reg'"
.endif; .endif; .endif; .endif; .endif; .endif; .endif; .endif
.endif; .endif; .endif; .endif; .endif; .endif; .endif; .endif
.endm
#endif

/* Helper macros for allocating stack memory. */
.macro push_void count=__USIZE
.if (\count) != 0
    leap -(\count)(%psp), %psp
.endif
.endm
.macro push_void_cfi count=__USIZE
.if (\count) != 0
    leap -(\count)(%psp), %psp
    .cfi_adjust_cfa_offset (\count)
.endif
.endm

.macro pushw_void; leap -2(%psp), %psp; .endm
.macro pushw_void_cfi; leap -2(%psp), %psp; .cfi_adjust_cfa_offset 2; .endm
.macro pushl_void; leap -4(%psp), %psp; .endm
.macro pushl_void_cfi; leap -4(%psp), %psp; .cfi_adjust_cfa_offset 4; .endm
.macro pushp_void; leap -__USIZE(%psp), %psp; .endm
.macro pushp_void_cfi; leap -__USIZE(%psp), %psp; .cfi_adjust_cfa_offset __USIZE; .endm
#ifdef __x86_64__
.macro pushq_void; leap -8(%psp), %psp; .endm
.macro pushq_void_cfi; leap -8(%psp), %psp; .cfi_adjust_cfa_offset 8; .endm
#endif

/* Helper macros for deallocating stack memory. */
.macro pop_void count=__USIZE
.if (\count) != 0
    leap (\count)(%psp), %psp
.endif
.endm
.macro pop_void_cfi count=__USIZE
.if (\count) != 0
    leap (\count)(%psp), %psp
    .cfi_adjust_cfa_offset -(\count)
.endif
.endm
.macro popw_void; leap 2(%psp), %psp; .endm
.macro popw_void_cfi; leap 2(%psp), %psp; .cfi_adjust_cfa_offset -2; .endm
.macro popl_void; leap 4(%psp), %psp; .endm
.macro popl_void_cfi; leap 4(%psp), %psp; .cfi_adjust_cfa_offset -4; .endm
.macro popp_void; leap __USIZE(%psp), %psp; .endm
.macro popp_void_cfi; leap __USIZE(%psp), %psp; .cfi_adjust_cfa_offset -__USIZE; .endm
#ifdef __x86_64__
.macro popq_void; leap 8(%psp), %psp; .endm
.macro popq_void_cfi; leap 8(%psp), %psp; .cfi_adjust_cfa_offset -8; .endm
#endif

.macro pushp_cfi arg; pushp \arg; .cfi_adjust_cfa_offset __USIZE; .endm
.macro popp_cfi arg; popp \arg; .cfi_adjust_cfa_offset -__USIZE; .endm
.macro pushfp_cfi; pushfp; .cfi_adjust_cfa_offset __USIZE; .endm
.macro popfp_cfi; popfp; .cfi_adjust_cfa_offset -__USIZE; .endm
.macro pushfp_cfi_r; pushfp; .cfi_adjust_cfa_offset __USIZE; .cfi_rel_offset %pflags, 0; .endm
.macro popfp_cfi_r; popfp; .cfi_adjust_cfa_offset -__USIZE; .cfi_restore %pflags; .endm
.macro pushp_cfi_r arg; pushp \arg; .cfi_adjust_cfa_offset __USIZE; .cfi_rel_offset \arg, 0; .endm
.macro popp_cfi_r arg; popp \arg; .cfi_adjust_cfa_offset -__USIZE; .cfi_restore \arg; .endm

#else
/* ??? */
#endif


#endif /* !_X86_KOS_ASM_UNIVERSAL_H */
