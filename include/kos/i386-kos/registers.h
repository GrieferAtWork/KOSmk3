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
#ifndef _KOS_I386_KOS_REGISTERS_H
#define _KOS_I386_KOS_REGISTERS_H 1

#include <__stdinc.h>
#include <hybrid/host.h>

__SYSDECL_BEGIN


#define X86_REGISTER_NONE              0x0000 /* No registers. */

/* Universal masks for registers. */
#define X86_REGISTER_SIZEMASK   0xc000
#define X86_REGISTER_IDMASK     0x3fff

/* Return the size of a general-purpose, or misc. register. (in bytes) */
#ifdef __x86_64__
#define X86_REGISTER_SIZEOF(x) (1 << (3-(((x)&0xc000) >> 14)))
#else
#define X86_REGISTER_SIZEOF(x) (1 << (2-(((x)&0xc000) >> 14)))
#endif


/* General-purpose registers. */
#define X86_REGISTER_GENERAL_PURPOSE   0x0001
#ifdef __x86_64__
#   define X86_REGISTER_GENERAL_PURPOSE_RAX 0x0000 /* %rax */
#   define X86_REGISTER_GENERAL_PURPOSE_RCX 0x0001 /* %rcx */
#   define X86_REGISTER_GENERAL_PURPOSE_RDX 0x0002 /* %rdx */
#   define X86_REGISTER_GENERAL_PURPOSE_RBX 0x0003 /* %rbx */
#   define X86_REGISTER_GENERAL_PURPOSE_RSP 0x0004 /* %rsp */
#   define X86_REGISTER_GENERAL_PURPOSE_RBP 0x0005 /* %rbp */
#   define X86_REGISTER_GENERAL_PURPOSE_RSI 0x0006 /* %rsi */
#   define X86_REGISTER_GENERAL_PURPOSE_RDI 0x0007 /* %rdi */
#   define X86_REGISTER_GENERAL_PURPOSE_R8  0x0008 /* %r8 */
#   define X86_REGISTER_GENERAL_PURPOSE_R9  0x0009 /* %r9 */
#   define X86_REGISTER_GENERAL_PURPOSE_R10 0x000a /* %r10 */
#   define X86_REGISTER_GENERAL_PURPOSE_R11 0x000b /* %r11 */
#   define X86_REGISTER_GENERAL_PURPOSE_R12 0x000c /* %r12 */
#   define X86_REGISTER_GENERAL_PURPOSE_R13 0x000d /* %r13 */
#   define X86_REGISTER_GENERAL_PURPOSE_R14 0x000e /* %r14 */
#   define X86_REGISTER_GENERAL_PURPOSE_R15 0x000f /* %r15 */
#   define X86_REGISTER_GENERAL_PURPOSE_EAX 0x4000 /* %eax */
#   define X86_REGISTER_GENERAL_PURPOSE_ECX 0x4001 /* %ecx */
#   define X86_REGISTER_GENERAL_PURPOSE_EDX 0x4002 /* %edx */
#   define X86_REGISTER_GENERAL_PURPOSE_EBX 0x4003 /* %ebx */
#   define X86_REGISTER_GENERAL_PURPOSE_ESP 0x4004 /* %esp */
#   define X86_REGISTER_GENERAL_PURPOSE_EBP 0x4005 /* %ebp */
#   define X86_REGISTER_GENERAL_PURPOSE_ESI 0x4006 /* %esi */
#   define X86_REGISTER_GENERAL_PURPOSE_EDI 0x4007 /* %edi */
#   define X86_REGISTER_GENERAL_PURPOSE_AX  0x8000 /* %ax */
#   define X86_REGISTER_GENERAL_PURPOSE_CX  0x8001 /* %cx */
#   define X86_REGISTER_GENERAL_PURPOSE_DX  0x8002 /* %dx */
#   define X86_REGISTER_GENERAL_PURPOSE_BX  0x8003 /* %bx */
#   define X86_REGISTER_GENERAL_PURPOSE_SP  0x8004 /* %sp */
#   define X86_REGISTER_GENERAL_PURPOSE_BP  0x8005 /* %bp */
#   define X86_REGISTER_GENERAL_PURPOSE_SI  0x8006 /* %si */
#   define X86_REGISTER_GENERAL_PURPOSE_DI  0x8007 /* %di */
#   define X86_REGISTER_GENERAL_PURPOSE_AL  0xc000 /* %al */
#   define X86_REGISTER_GENERAL_PURPOSE_CL  0xc001 /* %cl */
#   define X86_REGISTER_GENERAL_PURPOSE_DL  0xc002 /* %dl */
#   define X86_REGISTER_GENERAL_PURPOSE_BL  0xc003 /* %bl */
#   define X86_REGISTER_GENERAL_PURPOSE_AH  0xc004 /* %ah */
#   define X86_REGISTER_GENERAL_PURPOSE_CH  0xc005 /* %ch */
#   define X86_REGISTER_GENERAL_PURPOSE_DH  0xc006 /* %dh */
#   define X86_REGISTER_GENERAL_PURPOSE_BH  0xc007 /* %bh */
#else
#   define X86_REGISTER_GENERAL_PURPOSE_EAX 0x0000 /* %eax */
#   define X86_REGISTER_GENERAL_PURPOSE_ECX 0x0001 /* %ecx */
#   define X86_REGISTER_GENERAL_PURPOSE_EDX 0x0002 /* %edx */
#   define X86_REGISTER_GENERAL_PURPOSE_EBX 0x0003 /* %ebx */
#   define X86_REGISTER_GENERAL_PURPOSE_ESP 0x0004 /* %esp */
#   define X86_REGISTER_GENERAL_PURPOSE_EBP 0x0005 /* %ebp */
#   define X86_REGISTER_GENERAL_PURPOSE_ESI 0x0006 /* %esi */
#   define X86_REGISTER_GENERAL_PURPOSE_EDI 0x0007 /* %edi */
#   define X86_REGISTER_GENERAL_PURPOSE_AX  0x4000 /* %ax */
#   define X86_REGISTER_GENERAL_PURPOSE_CX  0x4001 /* %cx */
#   define X86_REGISTER_GENERAL_PURPOSE_DX  0x4002 /* %dx */
#   define X86_REGISTER_GENERAL_PURPOSE_BX  0x4003 /* %bx */
#   define X86_REGISTER_GENERAL_PURPOSE_SP  0x4004 /* %sp */
#   define X86_REGISTER_GENERAL_PURPOSE_BP  0x4005 /* %bp */
#   define X86_REGISTER_GENERAL_PURPOSE_SI  0x4006 /* %si */
#   define X86_REGISTER_GENERAL_PURPOSE_DI  0x4007 /* %di */
#   define X86_REGISTER_GENERAL_PURPOSE_AL  0x8000 /* %al */
#   define X86_REGISTER_GENERAL_PURPOSE_CL  0x8001 /* %cl */
#   define X86_REGISTER_GENERAL_PURPOSE_DL  0x8002 /* %dl */
#   define X86_REGISTER_GENERAL_PURPOSE_BL  0x8003 /* %bl */
#   define X86_REGISTER_GENERAL_PURPOSE_AH  0x8004 /* %ah */
#   define X86_REGISTER_GENERAL_PURPOSE_CH  0x8005 /* %ch */
#   define X86_REGISTER_GENERAL_PURPOSE_DH  0x8006 /* %dh */
#   define X86_REGISTER_GENERAL_PURPOSE_BH  0x8007 /* %bh */
#endif

/* Segment registers. */
#define X86_REGISTER_SEGMENT           0x0002
#ifndef __x86_64__
#   define X86_REGISTER_SEGMENT_ES          0 /* %es */
#   define X86_REGISTER_SEGMENT_DS          3 /* %ds */
#endif
#   define X86_REGISTER_SEGMENT_CS          1 /* %cs */
#   define X86_REGISTER_SEGMENT_SS          2 /* %ss */
#   define X86_REGISTER_SEGMENT_FS          4 /* %fs */
#   define X86_REGISTER_SEGMENT_GS          5 /* %gs */


/* Control registers. */
#define X86_REGISTER_CONTROL           0x0003
#   define X86_REGISTER_CONTROL_CR0         0 /* %cr0 */
#   define X86_REGISTER_CONTROL_CR2         2 /* %cr2 */
#   define X86_REGISTER_CONTROL_CR3         3 /* %cr3 */
#   define X86_REGISTER_CONTROL_CR4         4 /* %cr4 */

/* MMX registers. */
#define X86_REGISTER_MMX               0x0004
#   define X86_REGISTER_MMX_MM0             0 /* %mm0 */
#   define X86_REGISTER_MMX_MM1             1 /* %mm1 */
#   define X86_REGISTER_MMX_MM2             2 /* %mm2 */
#   define X86_REGISTER_MMX_MM3             3 /* %mm3 */
#   define X86_REGISTER_MMX_MM4             4 /* %mm4 */
#   define X86_REGISTER_MMX_MM5             5 /* %mm5 */
#   define X86_REGISTER_MMX_MM6             6 /* %mm6 */
#   define X86_REGISTER_MMX_MM7             7 /* %mm7 */

/* XMM registers. */
#define X86_REGISTER_XMM               0x0005
#   define X86_REGISTER_XMM_XMM0            0 /* %xmm0 */
#   define X86_REGISTER_XMM_XMM1            1 /* %xmm1 */
#   define X86_REGISTER_XMM_XMM2            2 /* %xmm2 */
#   define X86_REGISTER_XMM_XMM3            3 /* %xmm3 */
#   define X86_REGISTER_XMM_XMM4            4 /* %xmm4 */
#   define X86_REGISTER_XMM_XMM5            5 /* %xmm5 */
#   define X86_REGISTER_XMM_XMM6            6 /* %xmm6 */
#   define X86_REGISTER_XMM_XMM7            7 /* %xmm7 */
#ifdef __x86_64__
#   define X86_REGISTER_XMM_XMM8            8 /* %xmm8 */
#   define X86_REGISTER_XMM_XMM9            9 /* %xmm9 */
#   define X86_REGISTER_XMM_XMM10          10 /* %xmm10 */
#   define X86_REGISTER_XMM_XMM11          11 /* %xmm11 */
#   define X86_REGISTER_XMM_XMM12          12 /* %xmm12 */
#   define X86_REGISTER_XMM_XMM13          13 /* %xmm13 */
#   define X86_REGISTER_XMM_XMM14          14 /* %xmm14 */
#   define X86_REGISTER_XMM_XMM15          15 /* %xmm15 */
#endif

/* Float registers. */
#define X86_REGISTER_FLOAT             0x0006
#   define X86_REGISTER_FLOAT_ST(x)    ((x)&7)/* %st(x) */
#   define X86_REGISTER_FLOAT_ST0           0 /* %st(0) */
#   define X86_REGISTER_FLOAT_ST1           1 /* %st(1) */
#   define X86_REGISTER_FLOAT_ST2           2 /* %st(2) */
#   define X86_REGISTER_FLOAT_ST3           3 /* %st(3) */
#   define X86_REGISTER_FLOAT_ST4           4 /* %st(4) */
#   define X86_REGISTER_FLOAT_ST5           5 /* %st(5) */
#   define X86_REGISTER_FLOAT_ST6           6 /* %st(6) */
#   define X86_REGISTER_FLOAT_ST7           7 /* %st(7) */

/* Debug registers. */
#define X86_REGISTER_DEBUG             0x0007
#   define X86_REGISTER_DEBUG_DB0           0 /* %db0 */
#   define X86_REGISTER_DEBUG_DB1           1 /* %db1 */
#   define X86_REGISTER_DEBUG_DB2           2 /* %db2 */
#   define X86_REGISTER_DEBUG_DB3           3 /* %db3 */
#   define X86_REGISTER_DEBUG_DB4           4 /* %db4 */
#   define X86_REGISTER_DEBUG_DB5           5 /* %db5 */
#   define X86_REGISTER_DEBUG_DB6           6 /* %db6 */
#   define X86_REGISTER_DEBUG_DB7           7 /* %db7 */

/* Model-specific registers. */
#define X86_REGISTER_MSR               0x0010


/* Misc. registers. */
#define X86_REGISTER_MISC              0xffff
#ifdef __x86_64__
#   define X86_REGISTER_MISC_RFLAGS    0x0000 /* %rflags */
#   define X86_REGISTER_MISC_EFLAGS    0x4000 /* %eflags */
#   define X86_REGISTER_MISC_FLAGS     0x8000 /* %flags */
#   define X86_REGISTER_MISC_TR        0x8010 /* %tr (ltr, str; TaskRegister) */
#   define X86_REGISTER_MISC_LDT       0x8011 /* %ldt (lldt, sldt; LocalDescriptorTable) */
#   define X86_REGISTER_MISC_GDT       0x8012 /* %gdt (lgdt, sgdt; GlobalDescriptorTable) */
#   define X86_REGISTER_MISC_IDT       0x8013 /* %idt (lidt, sidt; InterruptDescriptorTable) */
#else
#   define X86_REGISTER_MISC_EFLAGS    0x0000 /* %eflags */
#   define X86_REGISTER_MISC_FLAGS     0x4000 /* %flags */
#   define X86_REGISTER_MISC_TR        0x4010 /* %tr (ltr, str; TaskRegister) */
#   define X86_REGISTER_MISC_LDT       0x4011 /* %ldt (lldt, sldt; LocalDescriptorTable) */
#   define X86_REGISTER_MISC_GDT       0x4012 /* %gdt (lgdt, sgdt; GlobalDescriptorTable) */
#   define X86_REGISTER_MISC_IDT       0x4013 /* %idt (lidt, sidt; InterruptDescriptorTable) */
#endif




__SYSDECL_END

#endif /* !_KOS_I386_KOS_REGISTERS_H */
