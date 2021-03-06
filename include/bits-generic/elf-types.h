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
#ifndef _BITS_GENERIC_ELF_TYPES_H
#define _BITS_GENERIC_ELF_TYPES_H 1
#define _BITS_ELF_TYPES_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifdef __CC__
/* Type for a 16-bit quantity. */
typedef __UINT16_TYPE__ Elf32_Half;
typedef __UINT16_TYPE__ Elf64_Half;

/* Types for signed and unsigned 32-bit quantities. */
typedef __UINT32_TYPE__ Elf32_Word;
typedef __INT32_TYPE__  Elf32_Sword;
typedef __UINT32_TYPE__ Elf64_Word;
typedef __INT32_TYPE__  Elf64_Sword;

/* Types for signed and unsigned 64-bit quantities. */
typedef __UINT64_TYPE__ Elf32_Xword;
typedef __INT64_TYPE__  Elf32_Sxword;
typedef __UINT64_TYPE__ Elf64_Xword;
typedef __INT64_TYPE__  Elf64_Sxword;

/* Type of addresses. */
typedef __UINT32_TYPE__ Elf32_Addr;
typedef __UINT64_TYPE__ Elf64_Addr;

/* Type of file offsets. */
typedef __UINT32_TYPE__ Elf32_Off;
typedef __UINT64_TYPE__ Elf64_Off;

/* Type for section indices, which are 16-bit quantities. */
typedef __UINT16_TYPE__ Elf32_Section;
typedef __UINT16_TYPE__ Elf64_Section;

/* Type for version symbol information. */
typedef Elf32_Half Elf32_Versym;
typedef Elf64_Half Elf64_Versym;
#endif /* __CC__ */

__SYSDECL_END

#endif /* !_BITS_GENERIC_ELF_TYPES_H */
