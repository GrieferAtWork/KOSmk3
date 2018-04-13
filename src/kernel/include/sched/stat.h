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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_STAT_H
#define GUARD_KERNEL_INCLUDE_SCHED_STAT_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/sections.h>
#include <sched/task.h>
#include <kos/thread.h>

DECL_BEGIN

/* Statistical tracking of thread usage. */
#ifndef CONFIG_NO_TASKSTAT
#define TASKSTATE_OFFSETOF_STARTED    0
#define TASKSTATE_OFFSETOF_HSWITCH    __SIZEOF_JTIME_T__
#define TASKSTATE_OFFSETOF_USWITCH   (__SIZEOF_JTIME_T__+__SIZEOF_POINTER__)
#define TASKSTATE_OFFSETOF_HYIELD    (__SIZEOF_JTIME_T__+2*__SIZEOF_POINTER__)
#define TASKSTATE_OFFSETOF_UYIELD    (__SIZEOF_JTIME_T__+3*__SIZEOF_POINTER__)
#define TASKSTATE_OFFSETOF_SLEEP     (__SIZEOF_JTIME_T__+4*__SIZEOF_POINTER__)
#define TASKSTATE_OFFSETOF_XRPC      (__SIZEOF_JTIME_T__+5*__SIZEOF_POINTER__)
#define TASKSTATE_OFFSETOF_QRPC      (__SIZEOF_JTIME_T__+6*__SIZEOF_POINTER__)
#define TASKSTATE_SIZE               (__SIZEOF_JTIME_T__+7*__SIZEOF_POINTER__)
#endif /* !CONFIG_NO_TASKSTAT */

#ifdef __CC__
#ifndef CONFIG_NO_TASKSTAT
struct taskstat {
    /* NOTE: All tasking statistics describe weak data, meaning
     *       that the data may change at any time and should always
     *       be taken with a grain of salt. */
    union {
        jtime_t           ts_started; /* Time (in jiffies) when the thread was started. */
        jtime32_t         ts_started32[2]; /* ... */
    };
    WEAK uintptr_t        ts_hswitch; /* Amount of times the thread was preempted while in kernel-space.
                                       * HINT: Divide by `HZ' to get a conservative estimate of the kernel-space run time. */
    WEAK uintptr_t        ts_uswitch; /* Amount of times the thread was preempted while in user-space.
                                       * HINT: Divide by `HZ' to get a conservative estimate of the user-space run time.
                                       * NOTE: Because threads can be preempted using `task_yield()',
                                       *       this and `ts_host_preemptions' may be smaller than what
                                       *       represents the proper resource usage times of this thread. */
    WEAK uintptr_t        ts_hyield;  /* Amount of times the thread called `task_yield()' (aka. from kernel-space) */
    WEAK uintptr_t        ts_uyield;  /* Amount of times the thread called `sched_yield()' (aka. from user-space) */
    WEAK uintptr_t        ts_sleep;   /* Amount of times the thread entered a sleeping-state using `task_sleep()'
                                       * Aka: How often did the thread have to perform a blocking signal-wait operation. */
    WEAK uintptr_t        ts_xrpc;    /* Amount of RPC functions served by this thread (including those send by the thread itself). */
    WEAK uintptr_t        ts_qrpc;    /* Amount of RPC functions queued (sent) by this thread (for execution by other threads, or the thread itself). */
};

/* Threading statistics for the calling thread.
 * NOTE: Statistics way be read by any thread with restriction to
 *       the fact that all contained data is weak, in the sense
 *       that no absolute meaning should be deciphered from it,
 *       other than its use for hints, or human-readable behavioral
 *       patterns.
 * NOTE: After creation of a thread using `task_alloc()', statistical
 *       tasking information is ZERO-initialized. */
DATDEF ATTR_PERTASK struct taskstat _this_stat;
#define THIS_STAT   PERTASK(_this_stat)
#endif /* !CONFIG_NO_TASKSTAT */

#endif /* __CC__ */



/* Helper macro for incrementing a certain statistical counter. */
#if defined(CONFIG_NO_TASKSTAT)
#ifdef __CC__
#define INCSTAT(x)  (void)0
#else
#define INCSTAT(x)  /* Nothing */
#endif
#elif !defined(__ASSEMBLER__)
#define INCSTAT(x)  (++THIS_STAT.x)
#elif defined(__x86_64__)
#define INCSTAT(x)  incq %taskseg:_this_stat+x
#elif defined(__i386__)
#define INCSTAT(x)  incl %taskseg:_this_stat+x
#else
#warning FIXME
#endif




DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_STAT_H */
