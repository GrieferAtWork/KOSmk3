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


#ifdef __ASSEMBLER__
#ifdef CONFIG_BUILDING_KERNEL_CORE
#define __DRIVER_PARAM_SECTION_NAME   .rodata.core_driver.param
#else
#define __DRIVER_PARAM_SECTION_NAME   .rodata.driver.param
#endif
#else
#ifdef CONFIG_BUILDING_KERNEL_CORE
#define __DRIVER_PARAM_SECTION_NAME  ".rodata.core_driver.param"
#else
#define __DRIVER_PARAM_SECTION_NAME  ".rodata.driver.param"
#endif
#endif


#ifdef __ASSEMBLER__
#ifdef __x86_64__
#define DEFINE_DRIVER_PARAM_EX(name,type,handler) \
    .pushsection __DRIVER_PARAM_SECTION_NAME; \
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
    .pushsection __DRIVER_PARAM_SECTION_NAME; \
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
 __asm__(".pushsection " __DRIVER_PARAM_SECTION_NAME "\n\t" \
         ".section .discard\n\t" \
         "\t991: .string " #name "\n\t" \
         "\t991 = . - 991b\n\t" \
         ".previous\n\t" \
         "\t.byte " PP_PRIVATE_STR(type) "\n\t" \
         ".if 991b <= 23\n\t" \
         "\t991: .string " #name "\n\t" \
         "\t.space 23 - (. - 991b)\n\t" \
         ".else\n\t" \
         "\t.quad 0\n\t" \
         "\t.section .rodata.str\n\t" \
         "\t\t991: .string " #name "\n\t" \
         "\t.previous\n\t" \
         "\t.reloc .,R_386_RELATIVE,991b; .quad 0\n\t" \
         "\t.quad 0\n\t" \
         ".endif\n\t" \
         "\t.quad " PP_PRIVATE_STR(handler) "\n\t" \
         ".popsection")
#else
#define DEFINE_DRIVER_PARAM_EX(name,type,handler) \
 __asm__(".pushsection " __DRIVER_PARAM_SECTION_NAME "\n\t" \
         ".section .discard\n\t" \
         "\t991: .string " #name "\n\t" \
         "\t991 = . - 991b\n\t" \
         ".previous\n\t" \
         "\t.byte " PP_PRIVATE_STR(type) "\n\t" \
         ".if 991b <= 11\n\t" \
         "\t991: .string " #name "\n\t" \
         "\t.space 11 - (. - 991b)\n\t" \
         ".else\n\t" \
         "\t.long 0\n" \
         "\t.section .rodata.str\n\t" \
         "\t\t991: .string " #name "\n\t" \
         "\t.previous\n\t" \
         "\t.reloc .,R_386_RELATIVE,991b; .long 0\n\t" \
         "\t.long 0\n\t" \
         ".endif\n\t" \
         "\t.long " PP_PRIVATE_STR(handler) "\n\t" \
         ".popsection")
#endif
#endif /* !__ASSEMBLER__ */


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_KERNEL_H */
