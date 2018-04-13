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
#ifndef GUARD_KERNEL_I386_KOS_SYSCALL_TABLE_C
#define GUARD_KERNEL_I386_KOS_SYSCALL_TABLE_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <i386-kos/syscall.h>

DECL_BEGIN


#ifdef CONFIG_NO_X86_SYSENTER
#define __SYSCALL(id,sym)      INTDEF byte_t sym[];
#define __SYSCALL_ASM(id,sym)  INTDEF byte_t sym[];
#define __XSYSCALL(id,sym)     INTDEF byte_t sym[];
#define __XSYSCALL_ASM(id,sym) INTDEF byte_t sym[];
#else
#define __SYSCALL(id,sym)      INTDEF byte_t sym[]; INTDEF byte_t argc_##sym[];
#define __SYSCALL_ASM(id,sym)  INTDEF byte_t sym[]; INTDEF byte_t argc_##sym[];
#define __XSYSCALL(id,sym)     INTDEF byte_t sym[]; INTDEF byte_t argc_##sym[];
#define __XSYSCALL_ASM(id,sym) INTDEF byte_t sym[]; INTDEF byte_t argc_##sym[];
#endif
#include <asm/syscallno.ci>

INTDEF byte_t x86_bad_syscall_except[];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
PUBLIC void *const x86_syscall_router[__NR_syscall_max + 1] = {
    [0 ... __NR_syscall_max] = x86_bad_syscall_except,
#define __SYSCALL(id,sym)     [id] = sym,
#define __SYSCALL_ASM(id,sym) [id] = sym,
#include <asm/syscallno.ci>
};
PUBLIC void *const x86_xsyscall_router[(__NR_xsyscall_max - __NR_xsyscall_min) + 1] = {
    [0 ... (__NR_xsyscall_max - __NR_xsyscall_min)] = x86_bad_syscall_except,
#define __XSYSCALL(id,sym)     [id - __NR_xsyscall_min] = sym,
#define __XSYSCALL_ASM(id,sym) [id - __NR_xsyscall_min] = sym,
#include <asm/syscallno.ci>
};
#pragma GCC diagnostic pop





DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_SYSCALL_TABLE_C */
