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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_IPI_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_IPI_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/sections.h>
#include <kernel/paging.h>
#include <sched/mutex.h>
#include <kos/context.h>
#include <stdbool.h>

DECL_BEGIN

/* Inter-processor-interrupt */

#define X86_IPI_NOOP             0x0000 /* Does nothing. */
#define X86_IPI_INVLPG           0x0001 /* Invalidate page directory caches. */
#define X86_IPI_INVLPG_ALL       0x0002 /* Invalidate all page directory caches. */
#define X86_IPI_INVLPG_ONE       0x0003 /* Invalidate a single page directory cache entry. */
#define X86_IPI_SCHEDULE         0x0004 /* Schedule pending tasks from the `c_pending' chain (See `i386-kos/scheduler.c'). */
#define X86_IPI_UNSCHEDULE       0x0005 /* Unschedule a given task from the receiving CPU.
                                         * The task is allowed to either be sleeping, or be running.
                                         * If the task is the one that got preempted by the IPI, the
                                         * CPU will continue execution of the next task in line.
                                         * However, if the task is the last one running on that CPU,
                                         * it cannot be unscheduled. */
#define X86_IPI_WAKETASK         0x0006 /* Wake a given task. */
#define X86_IPI_WAKETASK_FOR_RPC 0x0007 /* Wake a given task or serve interrupts if preempted in user-space. */
#define X86_IPI_WAKETASK_P       0x0008 /* Wake a given task and schedule it following another. */
#define X86_IPI_SUSPEND_CPU      0x0009 /* Suspend execution of the CPU until `X86_IPI_RESUME_CPU' has been sent.
                                         * Other IPIs sent until then will still executed immediately.
                                         * NOTE: This type of suspension is done asynchronously, meaning
                                         *       that the caller must implement their own lock to prevent
                                         *       different CPUs from suspending each other, or accidentally
                                         *       acquiring some kind of lock that is already held by a
                                         *       suspended CPU.
                                         *       By default, this mechanism is used to drop back to real-mode
                                         *       in order to perform BIOS calls when the system is already up.
                                         *      (I know, it's not a good idea to do that, but it's the easiest
                                         *       way to get boot-from-usb-stick working out of the box)
                                         * NOTE: In order to prevent a deadlock resulting from multiple CPUs
                                         *       suspending each other, the sending thread must have the
                                         *      `TASK_FKEEPCORE' flag set, as well as be holding a lock to
                                         *       the `x86_ipi_suspension_lock' mutex (defined below).
                                         * NOTE: This IPI and `X86_IPI_RESUME_CPU' operate recursively. */
#define X86_IPI_RESUME_CPU       0x000a /* Resume execution after `X86_IPI_SUSPEND_CPU' has been sent. */
#define X86_IPI_EXEC             0x000b /* Execute some user-defined function.
                                         * WARNING: The function executed must be 100% async-safe
                                         *          and must not attempt to enable interrupts!
                                         *          It must not throw any exceptions (or cause
                                         *          loadcore() to be invoked), and neither may it
                                         *          connect to any signals.
                                         *       >> Currently, this IPI is only used to implement
                                         *          system call tracing, as it can actually be used
                                         *          to modify the interrupt vector, or GDT/LDT table
                                         *          in a non-destructive way. */
#define X86_IPI_PANIC_SHUTDOWN   0x000c /* Shutdown the CPU for the purposes of a kernel panic. */

#define X86_IPI_FNORMAL  0x0000 /* Normal IPI flags. */

#define X86_IPI_OFFSETOF_TYPE   0
#define X86_IPI_SIZE           (4*__SIZEOF_POINTER__)

#ifdef __CC__
struct cpu;
struct x86_ipi {
    u16                     ipi_type;      /* IPI type (One of `X86_IPI_*') */
    u16                     ipi_flag;      /* IPI flags (Set of `X86_IPI_F*') */
#if __SIZEOF_POINTER__ > 4
    u16                     ipi_pad[(sizeof(void *)-4)/2];
#endif
    union PACKED {
        struct PACKED {
            VIRT pagedir_t *ivp_pagedir;   /* [1..1] The page directory within which the INVLPG was requested.
                                            *  When this IPI is received, TLB caches are only invalidated if
                                            *  this pointer matches the virtual address of the page directory
                                            *  active within the task that got interrupted. */
            vm_vpage_t      ivp_pageindex; /* The starting page index to invalidate (See `pagedir_sync') */
            size_t          ivp_numpages;  /* [valid_if(!X86_IPI_INVLPG_ONE)] The number of pages to invalidate (See `pagedir_sync') */
        }                   ipi_invlpg;    /* X86_IPI_INVLPG / X86_IPI_INVLPG_ONE */
        struct PACKED {
            struct task    *us_thread;     /* [1..1] The thread that should get unscheduled. */
#define X86_IPI_UNSCHEDULE_RETRY (-1)      /* Re-try the unschedule (the thread isn't being hosted on this CPU) */
#define X86_IPI_UNSCHEDULE_PENDING 0       /* The command hasn't been processed yet. */
#define X86_IPI_UNSCHEDULE_OK      1       /* Successfully unscheduled the thread.
                                            * NOTE: In this case, the IPI sender inherits a reference to `us_thread' */
#define X86_IPI_UNSCHEDULE_LAST    2       /* The thread could not be unscheduled, because it is the last one still running. */
#define X86_IPI_UNSCHEDULE_DEAD    3       /* The thread has terminated. */
#define X86_IPI_UNSCHEDULE_KEEP    4       /* The thread has the FKEEPCORE flag set. */
            volatile int   *us_status;     /* [1..1] Pointer to the IPI status (One of X86_IPI_UNSCHEDULE_*) */
        }                   ipi_unschedule; /* X86_IPI_UNSCHEDULE */
        struct PACKED {
            struct task    *wt_task;       /* [1..1] The task that should be woken. */
#define X86_IPI_WAKETASK_RETRY   (-1)      /* Re-try the wake (the task isn't running on this CPU) */
#define X86_IPI_WAKETASK_PENDING   0       /* The command hasn't been processed yet. */
#define X86_IPI_WAKETASK_OK        1       /* The task has been woken. */
#define X86_IPI_WAKETASK_DEAD      2       /* The task has terminated. */
            volatile int   *wt_status;     /* [1..1] Pointer to the IPI status (One to one of `X86_IPI_WAKETASK_*') */
        }                   ipi_wake;      /* X86_IPI_WAKETASK */
        struct PACKED {
            struct task    *wt_task;       /* [1..1] The task that should be woken. */
            struct task    *wt_prev;       /* [0..1] The task after which to schedule `wt_task'
                                            *       (When `NULL', or not hosted on this CPU, schedule after the currently running thread).
                                            *  NOTE: The `wt_task' wasn't sleeping, schedule it for execution after `wt_prev' */
            volatile int   *wt_status;     /* [1..1] Pointer to the IPI status (One to one of `X86_IPI_WAKETASK_*') */
        }                   ipi_wake_p;    /* X86_IPI_WAKETASK_P */
        struct PACKED {
            ASYNCSAFE
            void    (KCALL *wt_func)(void *arg); /* [1..1] The function that should be executed. */
            void           *wt_arg;        /* [?..?] The argument passed to `wt_func' */
#define X86_IPI_EXEC_PENDING     0         /* The command hasn't been processed yet. */
#define X86_IPI_EXEC_OK          1         /* The command has finished execution. */
            volatile int   *wt_status;     /* [1..1] Pointer to the IPI status (Set to one of `X86_IPI_EXEC_*') */
        }                   ipi_exec;      /* X86_IPI_EXEC */
    };
};

#ifndef CONFIG_NO_SMP
/* Send (and execute) and IPI to the given `target' CPU.
 * If the given `target' is the calling CPU, the `ipi'
 * argument is simply forwarded to `x86_ipi_handle()'.
 * IPIs are asynchronously executed in order. */
FUNDEF ASYNCSAFE void KCALL
x86_ipi_send(struct cpu *__restrict target,
             struct x86_ipi const *__restrict ipi);

/* Broadcast an IPI to all CPUs (including the
 * calling when `also_send_to_self' is true) */
FUNDEF ASYNCSAFE void KCALL
x86_ipi_broadcast(struct x86_ipi const *__restrict ipi,
                  bool also_send_to_self);

/* Lock that must be held while suspending other CPUs. */
DATDEF mutex_t x86_ipi_suspension_lock;

/* [lock(WRITE(THIS_CPU),READ(x86_ipi_suspension_lock))]
 * Set to `> 0' when an `X86_IPI_SUSPEND_CPU' IPI has been received (and the CPU is now suspended).
 * Set to `<= 0' when an `X86_IPI_RESUME_CPU' IPI has been received (and the CPU is no longer suspended).
 * Used to synchrize `x86_unicore_begin()' functionality. */
DATDEF ATTR_PERCPU int x86_cpu_suspended;

/* Safely begin/end a section of core that can only be executed by a single CPU:
 *   - Acquire a lock to `x86_ipi_suspension_lock'
 *   - Broadcast a `X86_IPI_SUSPEND_CPU' IPI to all other CPUs
 *   - Wait for the IPI to be acknowledged (`x86_cpu_suspended'
 *     must become set in all CPUs other than the caller's)
 * NOTE: The caller must set the `TASK_FKEEPCORE' bit in the calling thread.
 * @throw: E_INTERRUPT: The calling thread was interrupted.
 * @return: false: Failed to start the uni-code section
 *                (another CPU has already acquired the suspension-lock) */
FUNDEF bool KCALL x86_unicore_begin(void);
/* End a unicore section of code:
 *   - Broadcast a `X86_IPI_RESUME_CPU' IPI to all other CPUs
 *   - Release the lock from `x86_ipi_suspension_lock'. */
FUNDEF void KCALL x86_unicore_end(void);



#endif /* !CONFIG_NO_SMP */


#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_IPI_H */
