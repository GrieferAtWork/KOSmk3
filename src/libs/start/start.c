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
#ifndef GUARD_LIBS_START_START_C
#define GUARD_LIBS_START_START_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include <assert.h>

DECL_BEGIN

typedef int (*pmain)(int argc, char **argv, char **envp);
extern int (main)(int argc, char *argv[], char *envp[]);
IMPDEF ATTR_NORETURN void (FCALL __entry1)(pmain m);

#if 0 && (defined(__i386__) || defined(__x86_64__))
__asm__(".hidden _start\n"
        ".global _start\n"
        ".section .data.free\n"
        "_start:\n"
#ifdef CONFIG_START_DEBUG
        "    call __set_debug_malloc\n"
#endif
#ifdef __x86_64__
        "    leaq main, %rdi\n"
#else
        "    leal main, %ecx\n"
#endif
        ".global __entry1\n"
        "    jmp  __entry1\n"
        ".size _start, . - _start\n"
        );
#else
INTERN ATTR_SECTION(".text.free") void (_start)(void) {
#ifdef CONFIG_START_DEBUG
 __set_debug_malloc();
#endif
 __entry1(&main);
}
#endif

DECL_END

#endif /* !GUARD_LIBS_START_START_C */
