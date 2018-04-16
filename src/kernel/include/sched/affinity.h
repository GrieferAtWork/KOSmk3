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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_AFFINITY_H
#define GUARD_KERNEL_INCLUDE_SCHED_AFFINITY_H 1

#include <hybrid/compiler.h>
#include <sched/task.h>

DECL_BEGIN

#ifdef __CC__

#ifndef CONFIG_NO_SMP
/* Transfer the given thread to `new_cpu'.
 * @param: overrule_affinity: Ignore the task's affinity
 * @return: true:  The thread is now executing in the context of `new_cpu'
 * @return: true:  `new_cpu' was the cpu `thread' was already running on (no change)
 * @return: false: The `TASK_FKEEPCORE' flag is set.
 * @return: false: `overrule_affinity' is `false' and `new_cpu' isn't part of `thread's affinity.
 * @return: false: `thread' is the last task still being hosted
 *                 by `new_cpu' and therefor cannot be moved.
 *                 Shouldn't really happen because of per-cpu IDLE threads,
 *                 however can actually happen because /bin/init is the IDLE
 *                 thread of the boot CPU, meaning that the ability of moving
 *                 it (which is possible) means that some thread could end up
 *                 being the last thread still running on _boot_cpu.
 * If `thread' is `THIS_TASK', this function will return in the context
 * of `new_cpu', however unless the calling thread's affinity has been
 * updated to disallow the previous CPU (s.a. `task_setaffinity()'),
 * scheduling behavior may have chosen to move the task to another CPU
 * (potentially even the original one) before this function returns.
 * (In other words: This function isn't excluded from the usual rule
 *                  that `THIS_CPU' is always volatile) */
FUNDEF bool KCALL
task_setcpu(struct task *__restrict thread,
            struct cpu *__restrict new_cpu,
            bool overrule_affinity);
#else /* !CONFIG_NO_SMP */
#define task_setcpu(thread,new_cpu,overrule_affinity)  true
#endif /* CONFIG_NO_SMP */


/* Get/Set the CPU affinity of the given thread. */
FUNDEF void KCALL task_getaffinity(struct task *__restrict thread, kernel_cpuset_t affinity);
FUNDEF bool KCALL task_setaffinity(struct task *__restrict thread, kernel_cpuset_t const affinity);

#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_AFFINITY_H */
