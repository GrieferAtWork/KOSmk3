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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_PID_H
#define GUARD_KERNEL_INCLUDE_SCHED_PID_H 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <kos/types.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/sections.h>
#include <sched/task.h>
#include <bits/siginfo.h>
#include <bits/waitstatus.h>
#include <sys/wait.h>

DECL_BEGIN

#ifdef __CC__

struct pidns;

/* Mask that is applied to truncate randomly generated PIDs. */
#define PID_MASK  0x3fffffff

struct thread_pid {
    ATOMIC_DATA ref_t                tp_refcnt;      /* Reference counter for this descriptor. */
    REF struct pidns                *tp_ns;          /* [1..1][const] The main PID namespace of this thread PID. */
    atomic_rwlock_t                  tp_task_lock;   /* Lock for accessing `tp_task' (which is set to NULL when the thread is deallocated) */
    WEAK struct task                *tp_task;        /* [0..1][lock(tp_task_lock,WRITE(THIS_TASK))][== THIS_TASK]
                                                      *  Weak pointer to the task referred to by the PID. */
    int                              tp_event;       /* [lock(tp_task_lock)] The last event that happened in this thread (One of `CLD_*', or ZERO(0) if no pending event happened). */
#ifdef __USE_MISC
    union wait                       tp_status;      /* [lock(tp_task_lock,WRITE(THIS_TASK))]
                                                      *  The wait status of this thread. */
#else
    int                              tp_status;      /* [lock(tp_task_lock,WRITE(THIS_TASK))]
                                                      *  The wait status of this thread. */
#endif
    REF LIST_NODE(struct thread_pid) tp_siblings;    /* [lock(FORTASK(FORTASK(task_get_parent(),_this_group)->tg_leader,_this_group).tg_process.h_cldlock)]
                                                      * [CHAIN(FORTASK(FORTASK(task_get_parent(),_this_group)->tg_leader,_this_group).tg_process.h_children)]
                                                      * [0..1] Chain of sibling processes spawned by the parent process of the calling thread. */
    pid_t                            tp_pids[1];     /* [const][tp_ns->pn_indirection+1]
                                                      *  Inline-vector of PIDs associated to this thread,
                                                      *  as seen from different PID namespaces.
                                                      * `pn_indirection' should be used as index for
                                                      *  this vector, where the specific `pn_indirection'
                                                      *  used should be reachable by walking
                                                      * `tp_ns->[pn_parent...]' */
};

/* Destroy a previously allocated thread_pid. */
FUNDEF void KCALL thread_pid_destroy(struct thread_pid *__restrict self);

/* Increment/decrement the reference counter of the given thread_pid `x' */
#define thread_pid_incref(x)  ATOMIC_FETCHINC((x)->tp_refcnt)
#define thread_pid_decref(x) (ATOMIC_DECFETCH((x)->tp_refcnt) || (thread_pid_destroy(x),0))

/* [0..1][const] PID Descriptor of this thread (or `NULL' for kernel workers, or
 *               hacky threads that don't have a PID (cannot be created in userspace)) */
DATDEF ATTR_PERTASK REF struct thread_pid *_this_pid;
#define THIS_THREAD_PID PERTASK(_this_pid)
#define THIS_PIDNS     (THIS_THREAD_PID ? THIS_THREAD_PID->tp_ns : &pidns_kernel)


/* Try to dereference the given thread_pid, returning the associated task.
 * If the associated thread has died (its reference counter hit ZERO(0)), return `NULL' instead.
 * If the associated thread hasn't started yet, idly wait until it has, or was terminated.
 * Upon success, a reference to the pointed-to task is returned. */
FUNDEF ATTR_NOTHROW REF struct task *
KCALL thread_pid_get(struct thread_pid *__restrict self);

/* Weakly read the task pointer of the given thread pid.
 * The returned pointer may only be used for comparison, but not be dereferenced.
 * Additionally, no guaranty can be made that the task pointed to by the returned
 * value still exists by the time this function (macro) returns. */
#define thread_pid_weak(self) \
    ATOMIC_READ((self)->tp_task)

/* Similar to `posix_gettid_view()', but return the PID
 * for viewing `self' in `ns', rather than `THIS_PIDNS' */
FUNDEF ATTR_NOTHROW pid_t KCALL
thread_pid_view(struct thread_pid *__restrict self,
                struct pidns *__restrict ns);


/* Return posix process IDs for threads that are special to the caller.
 * NOTE: We return ZERO(0) as PID if the target process lies outside
 *       the calling process's PID namespace, or if the target process
 *       doesn't have any PID associated.
 * NOTE: We return ONE(1) for `posix_getppid()' if the parent process has died (indicative
 *       of POSIX's notion of the thread being adopted by the namespace's init process) */
FUNDEF ATTR_NOTHROW ATTR_CONST pid_t KCALL posix_gettid(void);
FUNDEF ATTR_NOTHROW pid_t KCALL posix_getpid(void);
FUNDEF pid_t KCALL posix_getppid(void);
FUNDEF pid_t KCALL posix_getpgid(void);
FUNDEF pid_t KCALL posix_getsid(void);

/* Return the PID for the given `thread', as seen from the PID namespace of the calling thread.
 * If the given `thread' cannot be found in that namespace, return ZERO(0) instead.
 * Additionally, ZERO(0) is also returned if the given `thread' doesn't have a PID. */
FUNDEF ATTR_NOTHROW pid_t KCALL posix_gettid_view(struct task *__restrict thread);
FUNDEF ATTR_NOTHROW pid_t KCALL posix_gettid_viewpid(struct thread_pid *__restrict thread);


/* Signal an event in to someone wait()-ing for the calling thread.
 * This function wakes a blocking `wait()' call, as well as sends
 * a signal to parent process of the calling thread group.
 * @param: event_code: One of `CLD_*' from <bits/siginfo.h>
 * @param: status:     Signal status (s.a. `union wait' from <bits/waitstatus.h>) */
FUNDEF void KCALL task_signal_event(int event_code, int status);


struct pidns {
    ATOMIC_DATA ref_t        pn_refcnt;      /* PID Namespace reference counter. */
    size_t                   pn_indirection; /* [const] Namespace indirection of this PID NS.
                                              * This also describes the number of PIDs that a
                                              * thread that is added to this namespace will
                                              * gain (+1), from this namespace and all namespaces
                                              * that are reachable from `pn_parent'. */
    REF struct pidns        *pn_parent;      /* [0..1][const]
                                              * [->pn_indirection == pn_indirection-1]
                                              * [(!= NULL) == (pn_indirection != 0)]
                                              * [(== NULL) == (self == &pidns_kernel)]
                                              * The parenting PID namespace with one less indirection. */
    atomic_rwlock_t          pn_lock;        /* Lock for accessing the hash-vector below. */
    size_t                   pn_used;        /* [lock(pn_lock)] Amount of used (non-NULL and non-DUMMY) PIDs */
    size_t                   pn_size;        /* [lock(pn_lock)] Amount of (non-NULL) PIDs entires. */
    size_t                   pn_mask;        /* [lock(pn_lock)] Hash-mask for `pn_list' */
    WEAK struct thread_pid **pn_list;        /* [0..1|DUMMY(PIDNS_DUMMY)][0..pn_mask+1][owned] Hash-vector of PIDs. */
#define PIDNS_DUMMY  ((struct thread_pid *)-1)
};

/* Allocate a new child PID namespace for `parent' */
FUNDEF ATTR_RETNONNULL ATTR_MALLOC
REF struct pidns *KCALL pidns_alloc(struct pidns *__restrict parent);

/* Destroy a previously allocated pidns. */
FUNDEF void KCALL pidns_destroy(struct pidns *__restrict self);

/* Increment/decrement the reference counter of the given pidns `x' */
#define pidns_incref(x)  ATOMIC_FETCHINC((x)->pn_refcnt)
#define pidns_decref(x) (ATOMIC_DECFETCH((x)->pn_refcnt) || (pidns_destroy(x),0))

/* Allocate a new PID descriptor for the given thread.
 * @param: fixed_pid: When non-ZERO, the fixed PID to allocate in `self'
 *                   (set to `1' for the init-process of the namespace)
 *                    When ZERO, or already in use, new PIDs are randomly
 *                    generated until one is found that isn't in use already. */
FUNDEF ATTR_RETNONNULL ATTR_MALLOC REF
struct thread_pid *KCALL pidns_newpid(struct pidns *__restrict self, pid_t fixed_pid);

/* Lookup the a PID descriptor matching the
 * given PID in the calling thread's PID namespace.
 * @throw: E_PROCESS_EXITED: No process exists that matches `pid'
 * NOTE: If the calling thread doesn't have a
 *       PID, `pidns_kernel' is searched instead. */
FUNDEF ATTR_RETNONNULL REF struct thread_pid *KCALL pid_lookup(pid_t pid);
/* Same as `pid_lookup()', but use `thread_pid_get()' to extract the thread.
 * If that fails, throw an `E_PROCESS_EXITED' instead. */
FUNDEF ATTR_RETNONNULL REF struct task *KCALL pid_lookup_task(pid_t pid);

/* The kernel's own PID namespace.
 * This is the root namespace that is used by
 * /bin/init (and usually most other processes, too)
 * NOTE: This is the _only_ PID namespace with an indirection of ZERO(0)! */
DATDEF struct pidns pidns_kernel;

#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_PID_H */
