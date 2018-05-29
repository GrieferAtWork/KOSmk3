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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_SEGMENT_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_SEGMENT_H 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>

#ifdef __x86_64__
#include "paging64.h"

DECL_BEGIN

#ifdef __ASSEMBLER__
#define __FIXUP_FSGSBASE \
.pushsection .rodata.fixup_fsgsbase.free; \
    .long 931f - KERNEL_CORE_BASE; \
.popsection; \
931:
.macro safe_rdfsbase reg
    __FIXUP_FSGSBASE rdfsbase \reg
.endm
.macro safe_rdgsbase reg
    __FIXUP_FSGSBASE rdgsbase \reg
.endm
.macro safe_wrfsbase reg
    __FIXUP_FSGSBASE wrfsbase \reg
.endm
.macro safe_wrgsbase reg
    __FIXUP_FSGSBASE wrgsbase \reg
.endm
#elif defined(__CC__)
#define __FIXUP_FSGSBASE \
".pushsection .rodata.fixup_fsgsbase.free\n\t" \
"\t.long 931f - " PP_STR(KERNEL_CORE_BASE_ASM) "\n\t" \
".popsection\n\t" \
"931:\n\t"

__asm__(
".macro safe_rdfsbase reg\n\t"
__FIXUP_FSGSBASE
"\trdfsbase \\reg\n\t"
".endm\n\t"
".macro safe_rdgsbase reg\n\t"
__FIXUP_FSGSBASE
"\trdgsbase \\reg\n\t"
".endm\n\t"
".macro safe_wrfsbase reg\n\t"
__FIXUP_FSGSBASE
"\twrfsbase \\reg\n\t"
".endm\n\t"
".macro safe_wrgsbase reg\n\t"
__FIXUP_FSGSBASE
"\twrgsbase \\reg\n\t"
".endm\n\t"
);
#endif

DECL_END
#endif /* __x86_64__ */

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_SEGMENT_H */
