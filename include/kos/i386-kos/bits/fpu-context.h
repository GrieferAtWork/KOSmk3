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
#ifndef _KOS_I386_KOS_BITS_FPU_CONTEXT_H
#define _KOS_I386_KOS_BITS_FPU_CONTEXT_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>
#include <hybrid/host.h>
#include "compat.h"

__SYSDECL_BEGIN

#ifndef __X86_CONTEXT_SYMBOL
#if !defined(__KERNEL__) && !defined(__EXPOSE_CPU_CONTEXT)
#define __X86_CONTEXT_SYMBOL_HIDDEN
#define __X86_CONTEXT_SYMBOL(x)    __##x
#else
#define __X86_CONTEXT_SYMBOL(x)    x
#endif
#endif /* !__X86_CONTEXT_SYMBOL */

#define __X86_FPUCONTEXT_SIZE  512
#define __X86_FPUCONTEXT_ALIGN 16
#define __FPUCONTEXT_SIZE  __X86_FPUCONTEXT_SIZE
#define __FPUCONTEXT_ALIGN __X86_FPUCONTEXT_ALIGN

#ifndef __X86_CONTEXT_SYMBOL_HIDDEN
#define FPUCONTEXT_SIZE    __FPUCONTEXT_SIZE
#define FPUCONTEXT_ALIGN   __FPUCONTEXT_ALIGN
#endif

#define x86_fpucontext         fpu_context
#ifdef __x86_64__
#define x86_fpucontext64       x86_fpucontext
#else
#define x86_fpucontext32       x86_fpucontext
#endif

#ifdef __CC__
struct __ATTR_PACKED x86_fpu_register {
    /* ST(i) / MMi register. */
    __UINT8_TYPE__          __X86_CONTEXT_SYMBOL(fr_data)[10];
    __UINT8_TYPE__          __X86_CONTEXT_SYMBOL(fr_pad)[6];
};

struct __ATTR_ALIGNED(__X86_FPUCONTEXT_ALIGN) __ATTR_PACKED x86_fpucontext32 {
    /* FPU context structure, as described here: 
     *   - http://asm.inightmare.org/opcodelst/index.php?op=FXSAVE
     *   - http://x86.renejeschke.de/html/file_module_x86_id_128.html */
    __UINT16_TYPE__         __X86_CONTEXT_SYMBOL(fp_fcw);   /* Floating point control word. */
    __UINT16_TYPE__         __X86_CONTEXT_SYMBOL(fp_fsw);   /* Floating point status word. */
    __UINT8_TYPE__          __X86_CONTEXT_SYMBOL(fp_ftw);   /* Floating point tag word. */
    __UINT8_TYPE__        __fp_res0;
    __UINT16_TYPE__         __X86_CONTEXT_SYMBOL(fp_fop);   /* Lower 11-bit f.p. opcode. */
    __UINT32_TYPE__         __X86_CONTEXT_SYMBOL(fp_fpuip); /* FPU instruction pointer. */
    __UINT16_TYPE__         __X86_CONTEXT_SYMBOL(fp_fpucs); /* FPU code segment selector. */
    __UINT16_TYPE__       __fp_res1;
    __UINT32_TYPE__         __X86_CONTEXT_SYMBOL(fp_fpudp); /* FPU data pointer. */
    __UINT16_TYPE__         __X86_CONTEXT_SYMBOL(fp_fpuds); /* FPU data segment selector. */
    __UINT16_TYPE__       __fp_res2;
    __UINT32_TYPE__         __X86_CONTEXT_SYMBOL(fp_mxcsr);
    __UINT32_TYPE__         __X86_CONTEXT_SYMBOL(fp_mxcsr_mask);
    struct x86_fpu_register __X86_CONTEXT_SYMBOL(fp_regs)[8];
    struct x86_fpu_register __X86_CONTEXT_SYMBOL(fp_xmm)[8];
    __UINT8_TYPE__        __fp_res3[224];
};

#if defined(__x86_64__) || defined(__EXPOSE_CPU_COMPAT)
__ATTR_ALIGNED(__X86_FPUCONTEXT_ALIGN)
struct __ATTR_PACKED x86_fpucontext64 {
    /* FPU context structure, as described here: 
     *   - http://asm.inightmare.org/opcodelst/index.php?op=FXSAVE
     *   - http://x86.renejeschke.de/html/file_module_x86_id_128.html */
    __UINT16_TYPE__         __X86_CONTEXT_SYMBOL(fp_fcw);   /* Floating point control word. */
    __UINT16_TYPE__         __X86_CONTEXT_SYMBOL(fp_fsw);   /* Floating point status word. */
    __UINT8_TYPE__          __X86_CONTEXT_SYMBOL(fp_ftw);   /* Floating point tag word. */
    __UINT8_TYPE__        __fp_res0;
    __UINT16_TYPE__         __X86_CONTEXT_SYMBOL(fp_fop);   /* Lower 11-bit f.p. opcode. */
    __UINT32_TYPE__         __X86_CONTEXT_SYMBOL(fp_fpuip); /* FPU instruction pointer. */
    __UINT16_TYPE__         __X86_CONTEXT_SYMBOL(fp_fpucs); /* FPU code segment selector. */
    __UINT16_TYPE__       __fp_res1;
    __UINT32_TYPE__         __X86_CONTEXT_SYMBOL(fp_fpudp); /* FPU data pointer. */
    __UINT16_TYPE__         __X86_CONTEXT_SYMBOL(fp_fpuds); /* FPU data segment selector. */
    __UINT16_TYPE__       __fp_res2;
    __UINT32_TYPE__         __X86_CONTEXT_SYMBOL(fp_mxcsr);
    __UINT32_TYPE__         __X86_CONTEXT_SYMBOL(fp_mxcsr_mask);
    struct x86_fpu_register __X86_CONTEXT_SYMBOL(fp_regs)[8];
    struct x86_fpu_register __X86_CONTEXT_SYMBOL(fp_xmm)[16];
    __UINT8_TYPE__        __fp_res3[96];
};
#endif

#endif /* __CC__ */

#undef __X86_CONTEXT_SYMBOL_HIDDEN
#undef __X86_CONTEXT_SYMBOL

__SYSDECL_END

#endif /* !_KOS_I386_KOS_BITS_FPU_CONTEXT_H */
