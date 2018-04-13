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
#ifndef GUARD_INCLUDE_I386_KOS_MEMORY_H
#define GUARD_INCLUDE_I386_KOS_MEMORY_H 1

#include <hybrid/compiler.h>

DECL_BEGIN

/* Predefined memory zones. */
#define             X86_MZONE_1MB  0        /* 0x0000000000000000..0x00000000000fffff */
#define             X86_MZONE_16MB 1        /* 0x0000000000100000..0x0000000000ffffff */
#define             X86_MZONE_1GB X86_MZONE_1GB
#define             X86_MZONE_4GB X86_MZONE_4GB
#ifdef __CC__
DATDEF unsigned int const X86_MZONE_1GB;    /* 0x0000000001000000..0x000000003fffffff (NOTE: `0x000000003fffffff' is the max-end; this zone may end before then) */
DATDEF unsigned int const X86_MZONE_4GB;    /* 0x0000000040000000..0x00000000ffffffff (NOTE: `0x00000000ffffffff' is the max-end; this zone may end before then) */
#endif /* __CC__ */
#define             X86_MZONE_PSE MZONE_ANY /* 0x0000000100000000..0xffffffffffffffff */

#ifdef CONFIG_BUILDING_KERNEL_CORE
#define MZONE_EARLY_IDENTITY  X86_MZONE_1GB /* Any memory that is identity-mapped during early boot. */
#endif /* CONFIG_BUILDING_KERNEL_CORE */
#define MZONE_ADDRESSIBLE     X86_MZONE_4GB /* Any region containing RAM that can be addressed without paging. */


/* Max amount of memory zones. */
#define MZONE_MAXCOUNT        8

/* The stack size of kernel threads. */
#define CONFIG_KERNELSTACK_SIZE (4096*8)

/* The stack size of automatically allocated user-space stacks. */
#define CONFIG_USERSTACK_SIZE   (4096*8)

/* Size of the kernel stack used for handling #DF exceptions. */
#define CONFIG_X86_DFSTACK_SIZE (4096*8)


/* Use the lower end of the boot task's stack
 * as initial buffer for memory information. */
#define MEMINSTALL_EARLY_BUFFER     x86_boot_stack
#define MEMINSTALL_EARLY_BUFSIZE    CONFIG_KERNELSTACK_SIZE
#define MEMINSTALL_EARLY_PREDEFINED 7


DECL_END

#endif /* !GUARD_INCLUDE_I386_KOS_MEMORY_H */
