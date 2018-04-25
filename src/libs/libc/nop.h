/* Copyright (c) 2018 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following __restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_LIBS_LIBC_NOP_H
#define GUARD_LIBS_LIBC_NOP_H 1

#include "libc.h"

#ifdef __CC__
DECL_BEGIN

#if (defined(__x86_64__) || defined(__i386__)) && 0
/* Doesn't work? (It links, but readelf is upset and runtime linking breaks...) */
__asm__(".pushsection .text.__x86.nop,\"axG\",@progbits,__x86.nop,comdat\n"
        "__x86.nop:\n\t"
        ".cfi_startproc\n\t"
#ifdef __x86_64__
        "xorq %rax, %rax\n"
#else
        "xorl %eax, %eax\n"
#endif
        "ret\n\t"
        ".cfi_endproc\n\t"
        ".size __x86.nop, . - __x86.nop\n\t"
        ".popsection");
#define DEFINE_NOP_FUNCTION_ZERO(decl,Treturn,name,args) \
        DEFINE_INTERN_ALIAS(name,__x86.nop);
#define DEFINE_NOP_FUNCTION_VOID(decl,name,args) \
        DEFINE_INTERN_ALIAS(name,__x86.nop);
#else
#define DEFINE_NOP_FUNCTION_ZERO(decl,Treturn,name,args) \
 decl Treturn LIBCCALL name args { return (Treturn)0; }
#define DEFINE_NOP_FUNCTION_VOID(decl,name,args) \
 decl void LIBCCALL name args { }
#endif



DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_NOP_H */
