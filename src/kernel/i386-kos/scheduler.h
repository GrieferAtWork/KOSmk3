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
#ifndef GUARD_KERNEL_I386_KOS_SCHEDULER_H
#define GUARD_KERNEL_I386_KOS_SCHEDULER_H 1
#define _KOS_SOURCE 1
#ifdef STRUCT_CPU_DEFINED
#error "This file must be included before <sched/task.h>"
#endif
#define STRUCT_CPU_DEFINED 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kos/types.h>
#include <sched/task.h>
#include <sched/signal.h>

DECL_BEGIN


#define CPU_OFFSETOF_RUNNING   CPU_OFFSETOF_ARCH
#define CPU_OFFSETOF_SLEEPING (CPU_OFFSETOF_ARCH+__SIZEOF_POINTER__)
#ifndef CONFIG_NO_SMP
#define CPU_OFFSETOF_PENDING  (CPU_OFFSETOF_ARCH+2*__SIZEOF_POINTER__)
#endif
#define CPU_OFFSETOF_DATA     (CPU_OFFSETOF_ARCH+3*__SIZEOF_POINTER__)

#if CPU_OFFSETOF_ARCH != __SIZEOF_POINTER__
#error "Must update the structure below"
#endif

#ifdef __CC__
/* The CPU control structure is private because there is
 * no acceptable situation in which scheduler-unrelated
 * code should access these fields. */
struct cpu {
    union PACKED {
        cpuid_t           cpu_id;    /* [const] CPU Id of this CPU. */
        uintptr_t       __cpu_pad;   /* ... */
    };
    REF struct task  *c_running;  /* [1..1][lock(PRIVATE(THIS_CPU))]
                                   *  The currently running task (RING).
                                   *  NOTE: Interrupts must be disabled when modifying this field!
                                   *        However, this field should not be read from to determine
                                   *        the actual, current task. - Use `%taskseg:t_self' for that! */
    /* TODO: Chain of tasks with the `TASK_STATE_FIDLETHREAD' flag set, when `c_running' contains other tasks. */
    REF struct task  *c_sleeping; /* [0..1][lock(PRIVATE(THIS_CPU))][sort(ASCENDING(->t_timeout))]
                                   *  The task with a time that will expire next (LIST).
                                   *  NOTE: Interrupts must be disabled when modifying this field! */
#ifndef CONFIG_NO_SMP
    REF struct task  *c_pending; /* [0..1][lock(c_pendlck)]
                                  * Chain of tasks that should be scheduled. (ATOMIC_SLIST) */
#endif
};
#endif /* __CC__ */


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_SCHEDULER_H */
