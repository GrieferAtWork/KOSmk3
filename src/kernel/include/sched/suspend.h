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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_SUSPEND_H
#define GUARD_KERNEL_INCLUDE_SCHED_SUSPEND_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <stdbool.h>

DECL_BEGIN

#ifdef __CC__
struct task;

/* Suspend the calling thread.
 * @return: true:  Some other thread called `task_resume()'
 *                 to resume the calling thread.
 * @return: true:  The calling thread is already suspended
 *                (Returned when called from an RPC callback, as
 *                 those are still served, even while suspended)
 * @return: false: The given `abs_timeout' has expired. */
FUNDEF bool KCALL task_suspend(jtime_t abs_timeout);

/* Resume execution within the given thread.
 * @return: true:  Successfully resumed execution in `thread'.
 * @return: false: The given `thread' has terminated.
 * @return: false: The given `thread' wasn't suspended. */
FUNDEF bool KCALL task_resume(struct task *__restrict thread);
#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_SUSPEND_H */
