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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_ELF_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_ELF_H 1

#include <hybrid/compiler.h>
#include <elf.h>

DECL_BEGIN

#undef CONFIG_ELF_USING_RELA
#undef CONFIG_ELF_SUPPORT_CLASS3264
#ifdef __x86_64__
#define EM_HOST  EM_X86_64
#define ELF_HEADER_SUPPORTED(x) \
 ((x).e_ident[EI_DATA] == ELFDATA2LSB && \
 (((x).e_ident[EI_CLASS] == ELFCLASS64 && (x).e_machine == EM_X86_64) || \
  ((x).e_ident[EI_CLASS] == ELFCLASS32 && (x).e_machine == EM_386)))

/* Given a machine code, return indicative of that machine being 32-bit, or 64-bit */
#define ELF_ISMACHINE32(x) ((x) == EM_386)
#define ELF_ISMACHINE64(x) ((x) == EM_X86_64)

/* On x86_64, we must support both 32-bit and 64-bit ELF objects.
 * NOTE: This option can only be defined when the preferred class is 64-bit! */
#define CONFIG_ELF_SUPPORT_CLASS3264 1

#define CONFIG_ELF_USING_RELA 1
#else
#define EM_HOST  EM_386
#define ELF_HEADER_SUPPORTED(x) \
 ((x).e_ident[EI_DATA] == ELFDATA2LSB && \
 ((x).e_ident[EI_CLASS] == ELFCLASS32 && (x).e_machine == EM_386))
#endif


#ifdef __x86_64__
#define R_NONE      R_X86_64_NONE
#define R_32        R_X86_64_32
#define R_PC32      R_X86_64_PC32
#define R_COPY      R_X86_64_COPY
#define R_GLOB_DAT  R_X86_64_GLOB_DAT
#define R_JMP_SLOT  R_X86_64_JUMP_SLOT
#define R_RELATIVE  R_X86_64_RELATIVE
#define R_16        R_X86_64_16
#define R_PC16      R_X86_64_PC16
#define R_8         R_X86_64_8
#define R_PC8       R_X86_64_PC8
#define R_64        R_X86_64_64
#define R_PC64      R_X86_64_PC64
#else
#define R_NONE      R_386_NONE
#define R_32        R_386_32
#define R_PC32      R_386_PC32
#define R_COPY      R_386_COPY
#define R_GLOB_DAT  R_386_GLOB_DAT
#define R_JMP_SLOT  R_386_JMP_SLOT
#define R_RELATIVE  R_386_RELATIVE
#define R_16        R_386_16
#define R_PC16      R_386_PC16
#define R_8         R_386_8
#define R_PC8       R_386_PC8
#endif

typedef uintptr_t Elf_RelValue;



DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_ELF_H */
