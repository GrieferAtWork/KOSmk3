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
#ifndef _KOS_I386_KOS_RPC_H
#define _KOS_I386_KOS_RPC_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>
#include "context.h"

__SYSDECL_BEGIN

#define X86_RPC_REASON_FFORMAT   0x0700 /* Mask of the register format found in `CTX' */
#define X86_RPC_REASON_FUSER     0x0000 /* The register contents follow no pattern (set for `RPC_REASON_ASYNC',
                                         * which has an context pointing to the arbitrary location where the
                                         * thread got interrupted) */
#define X86_RPC_REASON_FINT80    0x0100 /* The register format describes an `int $0x80'-compatible system call. */
#define X86_RPC_REASON_FSYSENTER 0x0200 /* The register format describes a `sysenter'-compatible system call. */
#define X86_RPC_REASON_FPF       0x0300 /* The register format describes a `#PF'-compatible system call. */




/* JOB preserve flags. */
#define X86_JOB_FSAVE_RETURN   0x0000 /* Always enabled: Push return information. */
#define X86_JOB_FLOAD_RETURN   0x0000 /* Always enabled: Load EIP / RIP information. */
#define X86_JOB_FSAVE_STACK    0x0100 /* Preserve ESP / RSP. */
#define X86_JOB_FLOAD_STACK    0x1000 /* Load ESP / RSP. */
#ifdef __x86_64__
#define X86_JOB_FSAVE_SEGMENTS 0x0200 /* Preserve segment registers (gs_base, fs_base). */
#define X86_JOB_FLOAD_SEGMENTS 0x2000 /* Load segment registers (gs_base, fs_base). */
#define X86_JOB_FSAVE_CREGS    0x0400 /* Preserve scratch registers (rflags, rax, rcx, rdx, rsi, rdi, r8, r9, r10, r11). */
#define X86_JOB_FLOAD_CREGS    0x4000 /* Load scratch registers (rflags, rax, rcx, rdx, rsi, rdi, r8, r9, r10, r11). */
#define X86_JOB_FSAVE_PREGS    0x0800 /* Preserve preserve registers (rbx, rbp, r12, r13, r14, r15). */
#define X86_JOB_FLOAD_PREGS    0x8000 /* Load preserve registers (rbx, rbp, r12, r13, r14, r15). */
#else
#define X86_JOB_FSAVE_SEGMENTS 0x0200 /* Preserve segment registers (gs, fs, es, ds, cs, ss). */
#define X86_JOB_FLOAD_SEGMENTS 0x2000 /* Load segment registers (gs, fs, es, ds, cs, ss). */
#define X86_JOB_FSAVE_CREGS    0x0400 /* Preserve scratch registers (eflags, eax, ecx, edx). */
#define X86_JOB_FLOAD_CREGS    0x4000 /* Load scratch registers (eflags, eax, ecx, edx). */
#define X86_JOB_FSAVE_PREGS    0x0800 /* Preserve preserve registers (ebx, ebp, esi, edi). */
#define X86_JOB_FLOAD_PREGS    0x8000 /* Load preserve registers (ebx, ebp, esi, edi). */
#endif

#ifdef __CC__
#ifdef __x86_64__
#define x86_job_frame64  x86_job_frame
#else /* __x86_64__ */
#define x86_job_frame32  x86_job_frame
#endif /* !__x86_64__ */

#if defined(__x86_64__) || !defined(__KERNEL__)
struct x86_job_frame64 {
    /* This structure represents the order in which preserve fields are pushed
     * onto the target stack of the JOB frame. However, the presence of individual
     * fields is determined by the flags used to enqueue the job. Additionally,
     * a field missing means that the offsets of all following fields have changed,
     * meaning that the register preserve format must be known to the JOB entry point,
     * in order for it to encode proper CFI unwind information, as well as for it to
     * correctly return to the interrupted location. */
    __ULONG64_TYPE__      jf_mode;     /* Interrupt reason (Set of `RPC_REASON_*'). */
    /* HINT: When everything is preserved, the remainder of
     *       this structure is identical to `x86_usercontext64' */
    __ULONG64_TYPE__      jf_r15;      /* [exists_if(X86_JOB_FSAVE_PREGS)] General purpose register #15 */
    __ULONG64_TYPE__      jf_r14;      /* [exists_if(X86_JOB_FSAVE_PREGS)] General purpose register #14 */
    __ULONG64_TYPE__      jf_r13;      /* [exists_if(X86_JOB_FSAVE_PREGS)] General purpose register #13 */
    __ULONG64_TYPE__      jf_r12;      /* [exists_if(X86_JOB_FSAVE_PREGS)] General purpose register #12 */
    __ULONG64_TYPE__      jf_r11;      /* [exists_if(X86_JOB_FSAVE_CREGS)] General purpose register #11 */
    __ULONG64_TYPE__      jf_r10;      /* [exists_if(X86_JOB_FSAVE_CREGS)] General purpose register #10 */
    __ULONG64_TYPE__      jf_r9;       /* [exists_if(X86_JOB_FSAVE_CREGS)] General purpose register #9 */
    __ULONG64_TYPE__      jf_r8;       /* [exists_if(X86_JOB_FSAVE_CREGS)] General purpose register #8 */
    __ULONG64_TYPE__      jf_rdi;      /* [exists_if(X86_JOB_FSAVE_CREGS)] Destination pointer */
    __ULONG64_TYPE__      jf_rsi;      /* [exists_if(X86_JOB_FSAVE_CREGS)] Source pointer */
    __ULONG64_TYPE__      jf_rbp;      /* [exists_if(X86_JOB_FSAVE_PREGS)] Frame base pointer */
    __ULONG64_TYPE__      jf_rbx;      /* [exists_if(X86_JOB_FSAVE_PREGS)] Base register */
    __ULONG64_TYPE__      jf_rdx;      /* [exists_if(X86_JOB_FSAVE_CREGS)] Data register */
    __ULONG64_TYPE__      jf_rcx;      /* [exists_if(X86_JOB_FSAVE_CREGS)] Count register */
    __ULONG64_TYPE__      jf_rax;      /* [exists_if(X86_JOB_FSAVE_CREGS)] Accumulator register */
    __ULONG64_TYPE__      jf_gsbase;   /* [exists_if(X86_JOB_FSAVE_SEGMENTS)] G segment base address */
    __ULONG64_TYPE__      jf_fsbase;   /* [exists_if(X86_JOB_FSAVE_SEGMENTS)] F segment base address */
    __ULONG64_TYPE__      jf_rip;      /* Instruction pointer */
    __ULONG64_TYPE__      jf_cs;       /* [exists_if(X86_JOB_FSAVE_SEGMENTS)] Code segment */
    __ULONG64_TYPE__      jf_rflags;   /* [exists_if(X86_JOB_FSAVE_CREGS)] Flags register */
    __ULONG64_TYPE__      jf_rsp;      /* [exists_if(X86_JOB_FSAVE_STACK)] Stack pointer */
    __ULONG64_TYPE__      jf_ss;       /* [exists_if(X86_JOB_FSAVE_SEGMENTS)] Stack segment */
};
#endif

struct x86_job_frame32 {
    /* This structure represents the order in which preserve fields are pushed
     * onto the target stack of the JOB frame. However, the presence of individual
     * fields is determined by the flags used to enqueue the job. Additionally,
     * a field missing means that the offsets of all following fields have changed,
     * meaning that the register preserve format must be known to the JOB entry point,
     * in order for it to encode proper CFI unwind information, as well as for it to
     * correctly return to the interrupted location. */
    __ULONG32_TYPE__      jf_mode;     /* Interrupt reason (Set of `RPC_REASON_*'). */
    /* HINT: When everything is preserved, the remainder of
     *       this structure is identical to `x86_usercontext32' */
    __ULONG32_TYPE__      jf_edi;      /* [exists_if(X86_JOB_FSAVE_PREGS)] Destination pointer */
    __ULONG32_TYPE__      jf_esi;      /* [exists_if(X86_JOB_FSAVE_PREGS)] Source pointer */
    __ULONG32_TYPE__      jf_ebp;      /* [exists_if(X86_JOB_FSAVE_PREGS)] Frame base pointer */
    __ULONG32_TYPE__      jf_esp;      /* [exists_if(X86_JOB_FSAVE_STACK)] Stack pointer */
    __ULONG32_TYPE__      jf_ebx;      /* [exists_if(X86_JOB_FSAVE_PREGS)] Base register */
    __ULONG32_TYPE__      jf_edx;      /* [exists_if(X86_JOB_FSAVE_CREGS)] Data register */
    __ULONG32_TYPE__      jf_ecx;      /* [exists_if(X86_JOB_FSAVE_CREGS)] Count register */
    __ULONG32_TYPE__      jf_eax;      /* [exists_if(X86_JOB_FSAVE_CREGS)] Accumulator register */
    __ULONG32_TYPE__      jf_gs;       /* [exists_if(X86_JOB_FSAVE_SEGMENTS)] G segment register */
    __ULONG32_TYPE__      jf_fs;       /* [exists_if(X86_JOB_FSAVE_SEGMENTS)] F segment register */
    __ULONG32_TYPE__      jf_es;       /* [exists_if(X86_JOB_FSAVE_SEGMENTS)] E (source) segment register */
    __ULONG32_TYPE__      jf_ds;       /* [exists_if(X86_JOB_FSAVE_SEGMENTS)] D (destination) segment register */
    __ULONG32_TYPE__      jf_eip;      /* Instruction pointer */
    __ULONG32_TYPE__      jf_eflags;   /* [exists_if(X86_JOB_FSAVE_CREGS)] Flags register */
    __ULONG32_TYPE__      jf_cs;       /* [exists_if(X86_JOB_FSAVE_SEGMENTS)] Code segment */
    __ULONG32_TYPE__      jf_ss;       /* [exists_if(X86_JOB_FSAVE_SEGMENTS)] Stack segment */
};
#endif /* __CC__ */


__SYSDECL_END

#endif /* !_KOS_I386_KOS_RPC_H */
