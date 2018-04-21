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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_KERNEL_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_KERNEL_H 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>

DECL_BEGIN

#define KERNEL_BIN_FILENAME      "kernel-i686-kos.bin"
#define KERNEL_BIN_FILENAME_HASH 42 /* TODO */


#ifdef CONFIG_BUILDING_KERNEL_CORE
#else
#endif

#ifdef __ASSEMBLER__
#ifdef __x86_64__
#define DEFINE_DRIVER_PARAM_EX(name,type,handler) \
    .pushsection .rodata.driver.param; \
    .section .discard; \
       991: .string name; \
       991 = . - 991b; \
    .previous; \
       .byte type; \
    .if 991b <= 23; \
       991: .string name; \
       .space 23 - (. - 991b); \
    .else; \
       .quad 0; \
       .section .rodata.str; \
          991: .string name; \
       .previous; \
       .reloc .,R_386_RELATIVE,991b; .quad 0; \
       .quad 0; \
    .endif; \
       .quad handler; \
    .popsection
#else
#define DEFINE_DRIVER_PARAM_EX(name,type,handler) \
    .pushsection .rodata.driver.param; \
    .section .discard; \
       991: .string name; \
       991 = . - 991b; \
    .previous; \
       .byte type; \
    .if 991b <= 11; \
       991: .string name; \
       .space 11 - (. - 991b); \
    .else; \
       .long 0; \
       .section .rodata.str; \
          991: .string name; \
       .previous; \
       .reloc .,R_386_RELATIVE,991b; .long 0; \
       .long 0; \
    .endif; \
       .long handler; \
    .popsection
#endif
#else /* __ASSEMBLER__ */
#ifdef __x86_64__
#define DEFINE_DRIVER_PARAM_EX(name,type,handler) \
 __asm__(".pushsection .rodata.driver.param\n" \
         ".section .discard\n" \
         "   991: .string " #name "\n" \
         "   991 = . - 991b\n" \
         ".previous\n" \
         "   .byte " PP_PRIVATE_STR(type) "\n" \
         ".if 991b <= 23\n" \
         "   991: .string " #name "\n" \
         "   .space 23 - (. - 991b)\n" \
         ".else\n" \
         "   .quad 0\n" \
         "   .section .rodata.str\n" \
         "      991: .string " #name "\n" \
         "   .previous\n" \
         "   .reloc .,R_386_RELATIVE,991b; .quad 0\n" \
         "   .quad 0\n" \
         ".endif\n" \
         "   .quad " PP_PRIVATE_STR(handler) "\n" \
         ".popsection")
#else
#define DEFINE_DRIVER_PARAM_EX(name,type,handler) \
 __asm__(".pushsection .rodata.driver.param\n" \
         ".section .discard\n" \
         "   991: .string " #name "\n" \
         "   991 = . - 991b\n" \
         ".previous\n" \
         "   .byte " PP_PRIVATE_STR(type) "\n" \
         ".if 991b <= 11\n" \
         "   991: .string " #name "\n" \
         "   .space 11 - (. - 991b)\n" \
         ".else\n" \
         "   .long 0\n" \
         "   .section .rodata.str\n" \
         "      991: .string " #name "\n" \
         "   .previous\n" \
         "   .reloc .,R_386_RELATIVE,991b; .long 0\n" \
         "   .long 0\n" \
         ".endif\n" \
         "   .long " PP_PRIVATE_STR(handler) "\n" \
         ".popsection")
#endif
#endif /* !__ASSEMBLER__ */


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_KERNEL_H */
