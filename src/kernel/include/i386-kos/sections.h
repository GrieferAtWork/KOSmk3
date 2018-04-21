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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_SECTIONS_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_SECTIONS_H 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>

DECL_BEGIN

#ifdef __ASSEMBLER__
#ifdef __x86_64__
#define DEFINE_ABS_CALLBACK(sect,func) \
    .pushsection sect; \
        .qword func; \
    .popsection
#define DEFINE_REL_CALLBACK(sect,func) \
    .pushsection sect; \
        .reloc .,R_X86_64_RELATIVE,func; .qword 0; \
    .popsection
#else
#define DEFINE_ABS_CALLBACK(sect,func) \
    .pushsection sect; \
        .long func; \
    .popsection
#define DEFINE_REL_CALLBACK(sect,func) \
    .pushsection sect; \
        .reloc .,R_386_RELATIVE,func; .long 0; \
    .popsection
#endif
#else
#ifdef __x86_64__
#define DEFINE_ABS_CALLBACK(sect,func) \
    __asm__(".pushsection " sect "\n\t" \
            "\t.qword " PP_PRIVATE_STR(func) "\n\t" \
            ".popsection")
#define DEFINE_REL_CALLBACK(sect,func) \
    __asm__(".pushsection " sect "\n\t" \
            "\t.reloc .,R_X86_64_RELATIVE," PP_PRIVATE_STR(func) "\n\t" \
            "\t.qword 0\n\t" \
            ".popsection")
#else
#define DEFINE_ABS_CALLBACK(sect,func) \
    __asm__(".pushsection " sect "\n\t" \
            "\t.long " PP_PRIVATE_STR(func) "\n\t" \
            ".popsection")
#define DEFINE_REL_CALLBACK(sect,func) \
    __asm__(".pushsection " sect "\n\t" \
            "\t.reloc .,R_386_RELATIVE," PP_PRIVATE_STR(func) "\n\t" \
            "\t.long 0\n\t" \
            ".popsection")
#endif
#endif

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_SECTIONS_H */
