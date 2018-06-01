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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_VM86_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_VM86_H 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>
#include <kos/types.h>
#include <kos/context.h>
#include <kernel/user.h>

DECL_BEGIN

#ifdef __x86_64__
#undef CONFIG_NO_VM86
#define CONFIG_NO_VM86 1
#endif

#ifndef CONFIG_NO_VM86
#define CONFIG_VM86 1
#ifdef __CC__
struct __ATTR_PACKED x86_irregs_vm86 {
    u32 ir_eip;
    u32 ir_cs;
    u32 ir_eflags;       /* Has the `EFLAGS_VM' flag set. */
    u32 ir_esp;
    u32 ir_ss;
    u16 ir_es,__ir_pad0;
    u16 ir_ds,__ir_pad1;
    u16 ir_fs,__ir_pad2;
    u16 ir_gs,__ir_pad3;
};

struct __ATTR_PACKED cpu_context_vm86 {
    struct x86_gpregs32              c_gpregs;   /* General purpose registers */
    struct x86_segments32            c_segments; /* Segment registers (ignored, but must be valid) */
    union __ATTR_PACKED {
        struct x86_irregs_vm86       c_iret;     /* IRet registers */
        struct __ATTR_PACKED {
             __ULONG32_TYPE__        c_eip;      /* Instruction pointer */
             __ULONG32_TYPE__      __c_pad;      /* ... */
             __ULONG32_TYPE__        c_eflags;   /* Flags register */
             __ULONG32_TYPE__        c_esp;      /* Stack pointer */
        };
    };
};

struct vm_region;

/* A VM region describing a identity mapping of the first 1Mb */
DATDEF struct vm_region vm86_identity_1mb;
#define VM86_IDENTITY_1MB_SIZE   ((1 * 1024 * 1024)/PAGESIZE)

/* NOTE: The higher 16 bits of `offset' are ignored. */
LOCAL u8 FCALL vm86_peekb(u16 segment, u32 offset);
LOCAL u16 FCALL vm86_peekw(u16 segment, u32 offset);
LOCAL u32 FCALL vm86_peekl(u16 segment, u32 offset);
LOCAL void FCALL vm86_pokeb(u16 segment, u32 offset, u8 value);
LOCAL void FCALL vm86_pokew(u16 segment, u32 offset, u16 value);
LOCAL void FCALL vm86_pokel(u16 segment, u32 offset, u32 value);
LOCAL void FCALL vm86_pushb(struct cpu_context_vm86 *__restrict ctx, u8 value);
LOCAL void FCALL vm86_pushw(struct cpu_context_vm86 *__restrict ctx, u16 value);
LOCAL void FCALL vm86_pushl(struct cpu_context_vm86 *__restrict ctx, u32 value);
LOCAL u8 FCALL vm86_popb(struct cpu_context_vm86 *__restrict ctx);
LOCAL u16 FCALL vm86_popw(struct cpu_context_vm86 *__restrict ctx);
LOCAL u32 FCALL vm86_popl(struct cpu_context_vm86 *__restrict ctx);


#define VM86_SEGMENT_ADDRESS(segment,offset) ((uintptr_t)((segment) * 16 + ((offset) & 0xffff)))

#ifndef __INTELLISENSE__
LOCAL u8 FCALL vm86_peekb(u16 segment, u32 offset) {
 u8 *presult;
 presult = (u8 *)VM86_SEGMENT_ADDRESS(segment,offset);
 validate_readable(presult,1);
 COMPILER_READ_BARRIER();
 return *presult;
}
LOCAL u16 FCALL vm86_peekw(u16 segment, u32 offset) {
 u16 *presult;
 presult = (u16 *)VM86_SEGMENT_ADDRESS(segment,offset);
 validate_readable(presult,2);
 COMPILER_READ_BARRIER();
 return *presult;
}
LOCAL u32 FCALL vm86_peekl(u16 segment, u32 offset) {
 u32 *presult;
 presult = (u32 *)VM86_SEGMENT_ADDRESS(segment,offset);
 validate_readable(presult,4);
 COMPILER_READ_BARRIER();
 return *presult;
}
LOCAL void FCALL vm86_pokeb(u16 segment, u32 offset, u8 value) {
 u8 *ptarget;
 ptarget = (u8 *)VM86_SEGMENT_ADDRESS(segment,offset);
 validate_writable(ptarget,1);
 COMPILER_WRITE_BARRIER();
 *(u8 *)ptarget = value;
 COMPILER_WRITE_BARRIER();
}
LOCAL void FCALL vm86_pokew(u16 segment, u32 offset, u16 value) {
 u16 *ptarget;
 ptarget = (u16 *)VM86_SEGMENT_ADDRESS(segment,offset);
 validate_writable(ptarget,2);
 COMPILER_WRITE_BARRIER();
 *ptarget = value;
 COMPILER_WRITE_BARRIER();
}
LOCAL void FCALL vm86_pokel(u16 segment, u32 offset, u32 value) {
 u32 *ptarget;
 ptarget = (u32 *)VM86_SEGMENT_ADDRESS(segment,offset);
 validate_writable(ptarget,4);
 COMPILER_WRITE_BARRIER();
 *ptarget = value;
 COMPILER_WRITE_BARRIER();
}
LOCAL void FCALL vm86_pushb(struct cpu_context_vm86 *__restrict ctx, u8 value) {
 u8 *ptarget;
 ptarget = (u8 *)VM86_SEGMENT_ADDRESS(ctx->c_iret.ir_ss,ctx->c_esp)-1;
 validate_writable(ptarget,4);
 COMPILER_WRITE_BARRIER();
 *ptarget = value;
 COMPILER_WRITE_BARRIER();
 ctx->c_esp -= 1;
}
LOCAL void FCALL vm86_pushw(struct cpu_context_vm86 *__restrict ctx, u16 value) {
 u16 *ptarget;
 ptarget = (u16 *)VM86_SEGMENT_ADDRESS(ctx->c_iret.ir_ss,ctx->c_esp)-1;
 validate_writable(ptarget,2);
 COMPILER_WRITE_BARRIER();
 *ptarget = value;
 COMPILER_WRITE_BARRIER();
 ctx->c_esp -= 2;
}
LOCAL void FCALL vm86_pushl(struct cpu_context_vm86 *__restrict ctx, u32 value) {
 u32 *ptarget;
 ptarget = (u32 *)VM86_SEGMENT_ADDRESS(ctx->c_iret.ir_ss,ctx->c_esp)-1;
 validate_writable(ptarget,4);
 COMPILER_WRITE_BARRIER();
 *ptarget = value;
 COMPILER_WRITE_BARRIER();
 ctx->c_esp -= 4;
}
LOCAL u8 FCALL vm86_popb(struct cpu_context_vm86 *__restrict ctx) {
 u8 *presult;
 presult = (u8 *)VM86_SEGMENT_ADDRESS(ctx->c_iret.ir_ss,ctx->c_esp);
 validate_readable(presult,1);
 COMPILER_WRITE_BARRIER();
 ctx->c_esp += 1;
 COMPILER_READ_BARRIER();
 return *presult;
}
LOCAL u16 FCALL vm86_popw(struct cpu_context_vm86 *__restrict ctx) {
 u16 *presult;
 presult = (u16 *)VM86_SEGMENT_ADDRESS(ctx->c_iret.ir_ss,ctx->c_esp);
 validate_readable(presult,2);
 COMPILER_WRITE_BARRIER();
 ctx->c_esp += 2;
 COMPILER_READ_BARRIER();
 return *presult;
}
LOCAL u32 FCALL vm86_popl(struct cpu_context_vm86 *__restrict ctx) {
 u32 *presult;
 presult = (u32 *)VM86_SEGMENT_ADDRESS(ctx->c_iret.ir_ss,ctx->c_esp);
 validate_readable(presult,4);
 COMPILER_WRITE_BARRIER();
 ctx->c_esp += 4;
 COMPILER_READ_BARRIER();
 return *presult;
}
#endif /* __INTELLISENSE__ */

#endif /* __CC__ */

#endif /* !CONFIG_NO_VM86 */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_VM86_H */
