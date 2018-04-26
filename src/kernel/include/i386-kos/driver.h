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


#ifdef CONFIG_BUILDING_KERNEL_CORE
#ifdef __ASSEMBLER__
#define __DRIVER_PARAM_POINTER(x)  .long x;
#else
#define __DRIVER_PARAM_POINTER(x)  "\t.long " x "\n\t"
#endif
#else /* CONFIG_BUILDING_KERNEL_CORE */
#ifdef __ASSEMBLER__
#define __DRIVER_PARAM_POINTER(x)  .reloc .,R_386_RELATIVE,x; .long 0;
#else
#define __DRIVER_PARAM_POINTER(x)  "\t.reloc .,R_386_RELATIVE," x "; .long 0\n\t"
#endif
#endif /* !CONFIG_BUILDING_KERNEL_CORE */

#ifdef __ASSEMBLER__
#ifdef __x86_64__
#define __IMPL_DEFINE_DRIVER_PARAM_EX(name,type,handler) \
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
       .byte 0, 0, 0, 0, 0, 0, 0; \
       .section .rodata.str; \
          991: .string name; \
       .previous; \
       __DRIVER_PARAM_POINTER(991b) \
       .quad 0; \
    .endif; \
       .quad handler; \
    .popsection
#define __IMPL_DEFINE_DRIVER_TAG(tag_symbol,name,flags,start,count) \
    .ifndef tag_symbol; \
    .pushsection .rodata.driver_specs.tag_symbol,"aG",@progbits,tag_symbol,comdat; \
        .set tag_symbol, .; \
        .word name; \
        .word flags; \
        .int 0; \
        .hidden start, count; \
        __DRIVER_PARAM_POINTER(start) \
        __DRIVER_PARAM_POINTER(count) \
        .size tag_symbol, . - tag_symbol; \
    .popsection; \
    .endif;
#else
#define __IMPL_DEFINE_DRIVER_PARAM_EX(name,type,handler) \
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
       .byte 0, 0, 0; \
       .section .rodata.str; \
          991: .string name; \
       .previous; \
       __DRIVER_PARAM_POINTER(991b) \
       .long 0; \
    .endif; \
       .long handler; \
    .popsection
#define __IMPL_DEFINE_DRIVER_TAG(tag_symbol,name,flags,start,count) \
    .ifndef tag_symbol; \
    .pushsection .rodata.driver_specs.tag_symbol,"aG",@progbits,tag_symbol,comdat; \
        .set tag_symbol, .; \
        .word name; \
        .word flags; \
        .hidden start, count; \
        __DRIVER_PARAM_POINTER(start) \
        __DRIVER_PARAM_POINTER(count) \
        .size tag_symbol, . - tag_symbol; \
    .popsection; \
    .endif;
    
#endif
#else /* __ASSEMBLER__ */
#ifdef __x86_64__
#define __IMPL_DEFINE_DRIVER_PARAM_EX(name,type,handler) \
 __asm__(".pushsection " __DRIVER_PARAM_SECTION_NAME "\n\t" \
         ".section .discard\n\t" \
         "\t991: .string " PP_PRIVATE_STR(name) "\n\t" \
         "\t.set .Lstrlen, . - 991b\n\t" \
         ".previous\n\t" \
         "\t.byte " PP_PRIVATE_STR(type) "\n\t" \
         ".if .Lstrlen <= 23\n\t" \
         "\t991: .string " PP_PRIVATE_STR(name) "\n\t" \
         "\t.space 23 - (. - 991b)\n\t" \
         ".else\n\t" \
         "\t.byte 0, 0, 0, 0, 0, 0, 0\n\t" \
         "\t.section .rodata.str\n\t" \
         "\t\t991: .string " PP_PRIVATE_STR(name) "\n\t" \
         "\t.previous\n\t" \
         __DRIVER_PARAM_POINTER("991b") \
         "\t.quad 0\n\t" \
         ".endif\n\t" \
         "\t.quad " PP_PRIVATE_STR(handler) "\n\t" \
         ".popsection")
#define __IMPL_DEFINE_DRIVER_TAG(tag_symbol,name,flags,start,count) \
 __asm__(".ifndef " tag_symbol "\n\t"\
         ".pushsection .rodata.driver_specs." tag_symbol \
         ",\"aG\",@progbits," tag_symbol ",comdat\n\t" \
         "\t.set " tag_symbol ", .\n\t" \
         "\t.word " PP_PRIVATE_STR(name) "\n\t" \
         "\t.word " PP_PRIVATE_STR(flags) "\n\t" \
         "\t.int 0\n\t" \
         "\t.hidden " PP_PRIVATE_STR(start) "," PP_PRIVATE_STR(count) "\n\t" \
         "\t" __DRIVER_PARAM_POINTER(PP_PRIVATE_STR(start)) "\n\t" \
         "\t" __DRIVER_PARAM_POINTER(PP_PRIVATE_STR(count)) "\n\t" \
         "\t.size " tag_symbol ", . - " tag_symbol "\n\t" \
         ".popsection\n\t" \
         ".endif");
#else
#define __IMPL_DEFINE_DRIVER_PARAM_EX(name,type,handler) \
 __asm__(".pushsection " __DRIVER_PARAM_SECTION_NAME "\n\t" \
         ".section .discard\n\t" \
         "\t991: .string " PP_PRIVATE_STR(name) "\n\t" \
         "\t.set .Lstrlen, . - 991b\n\t" \
         ".previous\n\t" \
         "\t.byte " PP_PRIVATE_STR(type) "\n\t" \
         ".if .Lstrlen <= 11\n\t" \
         "\t991: .string " PP_PRIVATE_STR(name) "\n\t" \
         "\t.space 11 - (. - 991b)\n\t" \
         ".else\n\t" \
         "\t.byte 0, 0, 0\n\t" \
         "\t.section .rodata.str\n\t" \
         "\t\t991: .string " PP_PRIVATE_STR(name) "\n\t" \
         "\t.previous\n\t" \
         __DRIVER_PARAM_POINTER("991b") \
         "\t.long 0\n\t" \
         ".endif\n\t" \
         "\t.long " PP_PRIVATE_STR(handler) "\n\t" \
         ".popsection")
#define __IMPL_DEFINE_DRIVER_TAG(tag_symbol,name,flags,start,count) \
 __asm__(".ifndef " tag_symbol "\n\t" \
         ".pushsection .rodata.driver_specs." tag_symbol \
         ",\"aG\",@progbits," tag_symbol ",comdat\n\t" \
         "\t.set " tag_symbol ", .\n\t" \
         "\t.hidden " PP_PRIVATE_STR(start) "," PP_PRIVATE_STR(count) "\n\t" \
         "\t.word " PP_PRIVATE_STR(name) "\n\t" \
         "\t.word " PP_PRIVATE_STR(flags) "\n\t" \
         "\t" __DRIVER_PARAM_POINTER(PP_PRIVATE_STR(start)) "\n\t" \
         "\t" __DRIVER_PARAM_POINTER(PP_PRIVATE_STR(count)) "\n\t" \
         "\t.size " tag_symbol ", . - " tag_symbol "\n\t" \
         ".popsection\n\t" \
         ".endif");
#endif
#endif /* !__ASSEMBLER__ */

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Driver tags can't appear within the kernel core. */
#undef __IMPL_DEFINE_DRIVER_TAG
#define __IMPL_DEFINE_DRIVER_TAG(tag_symbol,name,flags,start,count) /* nothing */
#endif


/* Define a driver tag that should appear within the driver's specifications.
 * The tag will only appear once, and all driver compilation units must specify
 * the same `flags', `start' and `count' values for the same `name'.
 * Since the number of driver tags is unspecific, using this mechanism only
 * functionality actually used by a driver will appear within its specifications,
 * meaning less redundancy and overhead.
 * @param: name:  One of `DRIVER_TAG_*' (from <kernel/driver.h>)
 * @param: flags: Set of `DRIVER_TAG_F*' (from <kernel/driver.h>)
 * @param: start: The starting address of the driver data block. (Must be a symbol; usually defined in `module.ld')
 * @param: count: The number of elements within the data block. (Must be a symbol; usually defined in `module.ld') */
#ifdef __ASSEMBLER__
#define DEFINE_DRIVER_TAG(name,flags,start,count) \
 __IMPL_DEFINE_DRIVER_TAG(start##_##count,name,flags,start,count)
#else
#define DEFINE_DRIVER_TAG(name,flags,start,count) \
 __IMPL_DEFINE_DRIVER_TAG(#start "_" #count,name,flags,start,count)
#endif



DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_KERNEL_H */
