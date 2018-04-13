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
#ifndef GUARD_KERNEL_SRC_I386_KOS_SAFECALL_C
#define GUARD_KERNEL_SRC_I386_KOS_SAFECALL_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <kernel/debug.h>
#include <kos/safecall.h>
#include <assert.h>

#ifndef CONFIG_NO_SAFECALL
DECL_BEGIN

INTDEF ATTR_NORETURN void KCALL
x86_safecall_panic(uintptr_t safecall_eip,
                   uintptr_t target_eip,
                   uintptr_t invalid_esp,
                   uintptr_t expected_esp) {
 assertf(0,
         "Invalid STDCALL invocation (%Id unmatched bytes) (negative means too few parameters in target)\n"
         "%[vinfo:%f(%l,%c) : %p : See reference to callee-site\n]"
         "%[vinfo:%f(%l,%c) : %p : See reference to caller-site\n]",
         invalid_esp-expected_esp,target_eip,safecall_eip);
}

DECL_END
#endif /* !CONFIG_NO_SAFECALL */

#endif /* !GUARD_KERNEL_SRC_I386_KOS_SAFECALL_C */
