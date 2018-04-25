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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_FPU_C
#define GUARD_KERNEL_INCLUDE_I386_KOS_FPU_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/sections.h>
#include <kos/context.h>
#include <stdbool.h>

DECL_BEGIN

#ifndef CONFIG_NO_FPU

#ifdef __CC__
/* [0..1] The task associated with the current FPU register contents, or NULL if none. */
DATDEF ATTR_PERCPU struct task *x86_fpu_current;

/* [0..1][owned] The per-task FPU state (lazily allocated) */
DATDEF ATTR_PERTASK struct fpu_context *x86_fpu_context;

/* Reset the current FPU state and discard any saved state. */
FUNDEF void KCALL x86_fpu_reset(void);

/* Save the current FPU state if it has been
 * modified and no longer matches `x86_fpu_context'.
 * @return: false: The FPU wasn't initialized, and `x86_fpu_context' wasn't allocated.
 * @return: true:  `x86_fpu_context' was allocated and is up-to-date. */
FUNDEF ATTR_NOTHROW bool KCALL x86_fpu_save(void);

/* If `thread' was modified and holds the current FPU
 * context on the calling CPU, save its context.
 * @return: false: The FPU wasn't initialized, and `x86_fpu_context' wasn't allocated.
 * @return: true:  `x86_fpu_context' was allocated and is up-to-date. */
FUNDEF NOIRQ ATTR_NOTHROW bool KCALL x86_fpu_save_thread(struct task *__restrict thread);

/* Reconfigure the FPU to either load `x86_fpu_context' lazily, or
 * load `x86_fpu_context' now. Either way, a previously active FPU
 * state will be discarded.
 * @assume(WAS_ALLOCATED(x86_fpu_context)); */
FUNDEF void KCALL x86_fpu_load(void);

/* Ensure that `x86_fpu_context' has been allocated.
 * NOTE: No-op when `x86_fpu_context' had already been allocated before. */
FUNDEF void KCALL x86_fpu_alloc(void);

#endif /* __CC__ */

#endif /* !CONFIG_NO_FPU */


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_FPU_C */
