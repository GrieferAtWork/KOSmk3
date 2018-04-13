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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_POSIX_SIGNALS_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_POSIX_SIGNALS_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <bits/siginfo.h>

DECL_BEGIN

#ifdef __CC__

struct cpu_hostcontext_user;
struct sigaction;


/* Redirect the given user-space context to call a signal
 * handler `action' before actually returning to user-space.
 * @param: mode: One of `TASK_USERCTX_F*'
 */
FUNDEF void KCALL
arch_posix_signals_redirect_action(struct cpu_hostcontext_user *__restrict context,
                                   siginfo_t const *__restrict info,
                                   struct sigaction const *__restrict action,
                                   unsigned int mode);

#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_POSIX_SIGNALS_H */
