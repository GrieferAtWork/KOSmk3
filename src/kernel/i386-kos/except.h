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
#ifndef GUARD_KERNEL_I386_KOS_EXCEPT_H
#define GUARD_KERNEL_I386_KOS_EXCEPT_H 1

#include <hybrid/compiler.h>
#include <kos/context.h>
#include <kos/types.h>
#include <signal.h>
#include <except.h>

DECL_BEGIN

struct cpu_context_ss {
    struct cpu_context c_context;
#ifndef __x86_64__
#ifndef CONFIG_X86_FIXED_SEGMENTATION
    uintptr_t          c_ss;
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
#endif
};

INTDEF bool FCALL
unwind_check_signal_frame(struct cpu_context_ss *__restrict context,
                          USER CHECKED sigset_t *signal_set,
                          size_t signal_set_size);
INTDEF bool FCALL
error_rethrow_at_user(USER CHECKED struct user_exception_info *except_info,
                      struct cpu_context_ss *__restrict context);

DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_EXCEPT_H */
