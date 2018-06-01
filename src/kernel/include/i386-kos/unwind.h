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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_UNWIND_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_UNWIND_H 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>
#include <kos/types.h>

DECL_BEGIN

#ifdef __x86_64__
/*
General Purpose Register RAX             0             %rax
General Purpose Register RDX             1             %rdx
General Purpose Register RCX             2             %rcx
General Purpose Register RBX             3             %rbx
General Purpose Register RSI             4             %rsi
General Purpose Register RDI             5             %rdi
Frame Pointer Register RBP               6             %rbp
Stack Pointer Register RSP               7             %rsp
Extended Integer Registers 8-15          8-15          %r8–%r15
Return Address RA                        16            %rip
SSE Registers 0–7                        17-24         %xmm0–%xmm7
Extended SSE Registers 8–15              25-32         %xmm8–%xmm15
Floating Point Registers 0–7             33-40         %st0–%st7
MMX Registers 0–7                        41-48         %mm0–%mm7
// Not part of the specs, but GCC seems to use this.
// -> I feel like sysv forgot about rflags when they designed the
//    specs, and this had to be re-added as an afterthought.
//    Oh well...
Flag Register                            49            %rflags
*/

#define UNWIND_NUM_REGISTERS           18
#define UNWIND_FRAME_REGSITER          6
#define UNWIND_REMEMBER_STACK_SIZE     2  /* Remember stack-size. */
#define UNWIND_CONTEXT_RETURN_REGISTER 16
#define UNWIND_CONTEXT_IP(x)         ((x)->c_rip)
#define UNWIND_CONTEXT_SP(x)         ((x)->c_rsp)
PRIVATE u16 const unwind_register_offsets[UNWIND_NUM_REGISTERS] = {
    [0]  = offsetof(struct cpu_context,c_gpregs.gp_rax),
    [1]  = offsetof(struct cpu_context,c_gpregs.gp_rdx),
    [2]  = offsetof(struct cpu_context,c_gpregs.gp_rcx),
    [3]  = offsetof(struct cpu_context,c_gpregs.gp_rbx),
    [4]  = offsetof(struct cpu_context,c_gpregs.gp_rsi),
    [5]  = offsetof(struct cpu_context,c_gpregs.gp_rdi),
    [6]  = offsetof(struct cpu_context,c_gpregs.gp_rbp),
    [7]  = offsetof(struct cpu_context,c_rsp),
    [8]  = offsetof(struct cpu_context,c_gpregs.gp_r8),
    [9]  = offsetof(struct cpu_context,c_gpregs.gp_r9),
    [10] = offsetof(struct cpu_context,c_gpregs.gp_r10),
    [11] = offsetof(struct cpu_context,c_gpregs.gp_r11),
    [12] = offsetof(struct cpu_context,c_gpregs.gp_r12),
    [13] = offsetof(struct cpu_context,c_gpregs.gp_r13),
    [14] = offsetof(struct cpu_context,c_gpregs.gp_r14),
    [15] = offsetof(struct cpu_context,c_gpregs.gp_r15),
    [16] = offsetof(struct cpu_context,c_rip),
    [17] = offsetof(struct cpu_context,c_rflags),
};
#define UNWIND_TRANSFORM_REGISTER(x) \
                 unwind_register_matrix[(u8)(x)]
PRIVATE u8 const unwind_register_matrix[256] = {
    [0]  = 0,  /* %rax */
    [1]  = 1,  /* %rdx */
    [2]  = 2,  /* %rcx */
    [3]  = 3,  /* %rbx */
    [4]  = 4,  /* %rsi */
    [5]  = 5,  /* %rdi */
    [6]  = 6,  /* %rbp */
    [7]  = 7,  /* %rsp */
    [8]  = 8,  /* %r8  */
    [9]  = 9,  /* %r9  */
    [10] = 10, /* %r10 */
    [11] = 11, /* %r11 */
    [12] = 12, /* %r12 */
    [13] = 13, /* %r13 */
    [14] = 14, /* %r14 */
    [15] = 15, /* %r15 */
    [16] = 16, /* %rip */
    [49] = 17,
};

#else /* __x86_64__ */

/*
General Purpose Register EAX                0            %eax
General Purpose Register ECX                1            %ecx
General Purpose Register EDX                2            %edx
General Purpose Register EBX                3            %ebx
Stack Pointer Register ESP                  4            %esp
Frame Pointer Register EBP                  5            %ebp
General Purpose Register ESI                6            %esi
General Purpose Register EDI                7            %edi
Return Address RA                           8            %eip
Flag Register                               9            %eflags
Reserved                                    10
Floating Point Registers 0–7                11-18        %st0–%st7
Reserved                                    19-20
Vector Registers 0–7                        21-28        %xmm0–%xmm7
MMX Registers 0–7                           29-36        %mm0–%mm7
Media Control and Status                    39           %mxcsr
Segment Register ES                         40           %es
Segment Register CS                         41           %cs
Segment Register SS                         42           %ss
Segment Register DS                         43           %ds
Segment Register FS                         44           %fs
Segment Register GS                         45           %gs
*/

#ifndef CONFIG_X86_FIXED_SEGMENTATION
#define UNWIND_NUM_REGISTERS        12
#elif !defined(CONFIG_NO_X86_SEGMENTATION)
#define UNWIND_NUM_REGISTERS        14
#else
#define UNWIND_NUM_REGISTERS        10
#endif
#define UNWIND_FRAME_REGSITER       5
#define UNWIND_REMEMBER_STACK_SIZE  2 /* Remember stack-size. */
#define UNWIND_CONTEXT_RETURN_REGISTER 8
#define UNWIND_CONTEXT_IP(x) ((x)->c_eip)
#define UNWIND_CONTEXT_SP(x) ((x)->c_esp)
PRIVATE u16 const unwind_register_offsets[UNWIND_NUM_REGISTERS] = {
    [0]  = offsetof(struct cpu_context,c_gpregs.gp_eax),
    [1]  = offsetof(struct cpu_context,c_gpregs.gp_ecx),
    [2]  = offsetof(struct cpu_context,c_gpregs.gp_edx),
    [3]  = offsetof(struct cpu_context,c_gpregs.gp_ebx),
    [4]  = offsetof(struct cpu_context,c_esp),
    [5]  = offsetof(struct cpu_context,c_gpregs.gp_ebp),
    [6]  = offsetof(struct cpu_context,c_gpregs.gp_esi),
    [7]  = offsetof(struct cpu_context,c_gpregs.gp_edi),
    [8]  = offsetof(struct cpu_context,c_eip), /* Return Address RA ??? */
    [9]  = offsetof(struct cpu_context,c_eflags)
#ifndef CONFIG_NO_X86_SEGMENTATION
    ,
#ifdef CONFIG_X86_FIXED_SEGMENTATION
    [10] = offsetof(struct cpu_context,c_segments.sg_fs), /* %fs */
    [11] = offsetof(struct cpu_context,c_segments.sg_gs)  /* %gs */
#else
    [10] = offsetof(struct cpu_context,c_segments.sg_es), /* %es */
    [11] = offsetof(struct cpu_context,c_segments.sg_ds), /* %ds */
    [12] = offsetof(struct cpu_context,c_segments.sg_fs), /* %fs */
    [13] = offsetof(struct cpu_context,c_segments.sg_gs)  /* %gs */
#endif
#endif
};

#ifndef CONFIG_NO_X86_SEGMENTATION
#define UNWIND_TRANSFORM_REGISTER(x) \
                 unwind_register_matrix[(u8)(x)]
PRIVATE u8 const unwind_register_matrix[256] = {
    [0]  = 0,  /* %eax */
    [1]  = 1,  /* %ecx */
    [2]  = 2,  /* %edx */
    [3]  = 3,  /* %ebx */
    [4]  = 4,  /* %esp */
    [5]  = 5,  /* %ebp */
    [6]  = 6,  /* %esi */
    [7]  = 7,  /* %edi */
    [8]  = 8,  /* %eip */
    [9]  = 9,  /* %eflags */
#ifdef CONFIG_X86_FIXED_SEGMENTATION
    [44] = 10, /* %fs */
    [45] = 11, /* %gs */
#else
    [40] = 10, /* %es */
    [43] = 11, /* %ds */
    [44] = 12, /* %fs */
    [45] = 13, /* %gs */
#endif
};
#endif
#endif /* !__x86_64__ */

#ifdef __x86_64__
typedef u64 UnwindRegister;
#else
typedef u32 UnwindRegister;
#endif

#define UNWIND_GET_REGISTER(self,regno) \
     (*(UnwindRegister const *)((uintptr_t)(self)+unwind_register_offsets[regno]))
#if 1
#define UNWIND_SET_REGISTER(self,regno,value) \
     (*(UnwindRegister *)((uintptr_t)(self)+unwind_register_offsets[regno]) = (value))
#else
#define UNWIND_SET_REGISTER(self,regno,value) \
     (debug_printf("UNWIND_SET_REGISTER(%I16u,%p)\n",regno,value),\
      *(UnwindRegister *)((uintptr_t)(self)+unwind_register_offsets[regno]) = (value))
#endif



DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_UNWIND_H */
