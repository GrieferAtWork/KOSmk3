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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_POSIX_SIGNALS_H
#define GUARD_KERNEL_INCLUDE_SCHED_POSIX_SIGNALS_H 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <kos/types.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <bits/sigaction.h>
#include <sched/mutex.h>
#include <bits/sigset.h>
#include <bits/siginfo.h>
#include <bits/signum.h>
#include <bits/sigstack.h>
#include <kernel/sections.h>
#include <kernel/syscall.h>
#include <sched/signal.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/posix_signals.h>
#endif

#undef CONFIG_SIGPENDING_TRACK_MASK
#ifndef CONFIG_NO_SIGPENDING_TRACK_MASK
#define CONFIG_SIGPENDING_TRACK_MASK 1
#endif


DECL_BEGIN

#ifdef __CC__

#ifndef __sigset_t_defined
#define __sigset_t_defined 1
typedef __sigset_t sigset_t;
#endif

struct sigblock {
    /* Descriptor for signals that are being blocked. */
    ATOMIC_DATA ref_t sb_share;  /* Amount of threads that are sharing this sig-block.
                                  * NOTE: Modifications may only be made to the sig-block
                                  *       when this value is ONE(1). */
    sigset_t          sb_sigset; /* [const_if(sb_share > 1)]
                                  * Bit-set of signals that are being blocked. */
};


/* [0..1][lock(PRIVATE(THIS_TASK))][ref(sb_share)]
 * The set of signals being blocked in the current thread.
 * When NULL, assume that no signals are being blocked. */
DATDEF ATTR_PERTASK REF struct sigblock *_this_sigblock;

/* Allocate a missing `_this_sigblock', or replace it
 * with a copy of itself when `sb_share > 1'.
 * @return: * : Always returns `PERTASK_GET(_this_sigblock)'
 * @throw E_BADALLOC: Not enough available memory. */
FUNDEF ATTR_RETNONNULL struct sigblock *KCALL sigblock_unique(void);

/* A faster version of `sigblock_unique()' that doesn't lazily
 * allocate the sigblock, rather returning NULL if no sigblock
 * exists, or if it is shared with another thread. */
FUNDEF struct sigblock *KCALL sigblock_unique_fast(void);


struct sighand {
    /* Descriptor for how signals ought to be handled. */
    atomic_rwlock_t   sh_lock;        /* Lock for accessing elements of `sh_hand' */
    ref_t             sh_share;       /* [lock(sh_lock)] Amount of unrelated processes sharing this sighand. */
    struct sigaction *sh_hand[_NSIG]; /* [0..1][lock(sh_lock)][owned][*][config_if(sh_share > 1)]
                                       * Vector of user-space signal handler.
                                       * NOTE: NULL-entries imply default behavior, which is signal-dependent. */
};


struct sighand_ptr {
    /* Secondary indirection for sighand that allows
     * for copy-on-write between different processes
     * that were fork()-ed from each other.
     * Because of the fact that fork() would otherwise have to
     * copy the sighand table, only to have a likely following
     * call to exec() destroy that table again, we simply share
     * the sighand table between the old and new process until
     * either one of the processes dies or calls exec(), or until
     * one of them attempts to modify the sighand table, in which
     * case this indirection allows for lazy copy-on-write. */
    ATOMIC_DATA ref_t   sp_refcnt;      /* Amount of threads using `sp_hand' */
    atomic_rwlock_t     sp_lock;        /* Lock for the `sp_hand' pointer. */
    REF struct sighand *sp_hand;        /* [0..1][ref(sh_share)][lock(sp_lock,WRITE_ONCE[ALLOW_EXCHANGE])]
                                         * Pointer to the shared signal handler table. */
};


/* [0..1][lock(PRIVATE(THIS_TASK))]
 * User-space signal handlers for the calling thread.
 * NOTE: When NULL, default behavior should be used for handling any signal. */
DATDEF ATTR_PERTASK REF struct sighand_ptr *_this_sighand_ptr;

/* Allocates a missing or unshare a shared signal handler
 * table and acquire a write-lock on (return->sh_lock).
 * The caller may then modify the `sh_hand' vector freely,
 * before calling `atomic_rwlock_endwrite(&return->sh_lock)'
 * to commit any changes that were made.
 * This function automatically deals with all inter-process
 * copy-on-write, as well as inter-thread sighand-share behavior. */
FUNDEF ATTR_RETNONNULL struct sighand *KCALL sighand_lock_write(void);
/* Similar to `sighand_lock_write()', but don't
 * unshare and acquire a read-lock instead. 
 * If no handlers have been defined, return `NULL'
 * instead (implies DEFAULT behavior for all signals). */
FUNDEF struct sighand *KCALL sighand_lock_read(void);


struct sigqueue {
    /* Descriptor for a signal that was send, but was
     * blocked and therefor scheduled to be received later. */
    SLIST_NODE(struct sigqueue) sq_chain;  /* [owned] Chain of queued signals. */
    siginfo_t                   sq_info;   /* Signal information. */
};

struct sigpending {
    /* Descriptor for pending signals (always per-thread). */
    mutex_t                     sp_lock;   /* Lock used to guard this descriptor. */
    struct sig                  sp_newsig; /* Signal used to broadcast addition of new signals. */
    SLIST_HEAD(struct sigqueue) sp_queue;  /* [0..1][owned][lock(sp_lock)] List of queued signals. */
#ifdef CONFIG_SIGPENDING_TRACK_MASK
    sigset_t                    sp_mask;   /* [lock(sp_lock)] Mask of all signals found in 'sp_queue'. */
#endif
};

struct sigshare {
    /* Descriptor for shared, pending signals (usually per-process). */
    struct sigpending  ss_pending; /* Shared, pending signals. */
    ATOMIC_DATA ref_t  ss_refcnt;  /* Reference counter. */
};

#ifdef CONFIG_SIGPENDING_TRACK_MASK
/* [0..1][owned][lock(WRITE_ONCE)]
 * Pending task chain for the calling thread. */
DATDEF ATTR_PERTASK struct sigpending *_this_sigpending_task;
#else
/* Pending task chain for the calling thread. */
DATDEF ATTR_PERTASK struct sigpending _this_sigpending_task;
#endif

/* [0..1][lock(WRITE_ONCE)]
 * Pending task chain for the calling process. */
DATDEF ATTR_PERTASK REF struct sigshare *_this_sigpending_proc;


/* Lazily allocate if missing, and return the
 * per-task or per-process sigpending controllers. */
#ifdef CONFIG_SIGPENDING_TRACK_MASK
FUNDEF ATTR_RETNONNULL struct sigpending *KCALL sigpending_gettask(void);
FUNDEF ATTR_RETNONNULL struct sigpending *KCALL sigpending_getfor(struct task *__restrict thread);
#else
#define sigpending_gettask()      (&PERTASK_GET(_this_sigpending_task))
#define sigpending_getfor(thread) (&FORTASK(thread,_this_sigpending_task))
#endif
FUNDEF ATTR_RETNONNULL struct sigpending *KCALL sigpending_getproc(void);
FUNDEF ATTR_RETNONNULL struct sigpending *KCALL sigpending_getprocfor(struct task *__restrict thread);




/* Actions codes of default actions to perform the no handler is set. */
#define SIGNAL_ACTION_IGNORE   1 /* Ignore the signal and continue. */
#define SIGNAL_ACTION_TERM     3 /* Terminate the calling process (thread-group). */
#define SIGNAL_ACTION_EXIT     4 /* Terminate the calling thread. */
#define SIGNAL_ACTION_CONT     8 /* Continue execution. */
#define SIGNAL_ACTION_STOP     9 /* Stop execution. */
#define SIGNAL_ACTION_CORE    10 /* Create a core-dump. */
#define SIGNAL_ISACTION(x)   ((x) <= 10)


/* Default actions codes for standard signals. */
DATDEF u8 const signal_default_actions[_NSIG];



/* Using RPC function calls, raise a signal in the given thread.
 * If `thread' is the calling thread, this function will not return.
 * @return: true:  The RPC was scheduled for delivery and will be executed (asynchronously).
 * @return: false: The RPC call failed because `thread' has terminated.
 * @throw: E_INVALID_ARGUMENT: The signal number in `info' is ZERO(0) or >= `_NSIG+1'
 * @throw: E_SEGFAULT:         The user-buffers are faulty. */
FUNDEF bool KCALL signal_raise_thread(struct task *__restrict thread,
                                      USER CHECKED siginfo_t *__restrict info);
/* Enqueue in `sigshare' and find some thread in `threadgroup'
 * that isn't blocking the signal, then post an RPC functions
 * to that thread which will search for pending signals that
 *       are not being ignored and handle them.
 * NOTE: If `threadgroup' isn't the leader of a group,
 *       the signal is send to its leader thread. */
FUNDEF bool KCALL signal_raise_group(struct task *__restrict threadgroup,
                                     USER CHECKED siginfo_t *__restrict info);
/* Similar to `signal_raise_group()', but send a copy of the signal to
 * every process in the given `processgroup' (s.a. `task_get_group()')
 * @return: * : The number of successfully sent signals.
 * If `processgroup' isn't the leader of a process, or process group,
 * it's process leader will be dereferenced, following that process's
 * process group leader being dereferenced. */
FUNDEF size_t KCALL signal_raise_pgroup(struct task *__restrict processgroup,
                                        USER CHECKED siginfo_t *__restrict info);

/* Change the signal mask of the calling thread.
 * Following this, check for signals that have been unblocked
 * and throw an `E_INTERRUPT' exception if any were.
 * When `old_mask' is non-NULL, save the old mask inside
 * before changing anything.
 * @throw: E_INVALID_ARGUMENT: The given `how' is invalid.
 * @throw: E_INVALID_ARGUMENT: The given `sigsetsize' is too large.
 * @throw: E_SEGFAULT:         The user-buffers are faulty. */
FUNDEF void KCALL signal_chmask(USER CHECKED sigset_t const *mask,
                                USER CHECKED sigset_t *old_mask,
                                size_t sigsetsize, int how);
#define SIGNAL_CHMASK_FBLOCK     0 /* Block signals. */
#define SIGNAL_CHMASK_FUNBLOCK   1 /* Unblock signals. */
#define SIGNAL_CHMASK_FSETMASK   2 /* Set the set of blocked signals. */

/* Change the action for the given `signo'.
 * @throw: E_SEGFAULT: The user-buffers are faulty. */
FUNDEF void KCALL
signal_chaction(int signo,
                USER CHECKED struct sigaction const *new_action,
                USER CHECKED struct sigaction *old_action);
#ifdef CONFIG_SYSCALL_COMPAT
FUNDEF void KCALL
signal_chaction_compat(int signo,
                       USER CHECKED struct sigaction_compat const *new_action,
                       USER CHECKED struct sigaction_compat *old_action);
#endif

/* Change the signal disposition of all signal actions using
 * a user-space disposition to `SIG_DFL'. (called during exec()) */
FUNDEF void KCALL signal_resetexec(void);


/* Wait for one of the signals specified in `wait_mask' to become
 * available within the calling thread's pending list, or the
 * shared pending list of the calling thread.
 * @throw: E_INVALID_ARGUMENT: The given `sigsetsize' is too large.
 * @throw: E_SEGFAULT:         The user-buffers are faulty.
 * @return: true:  A Signal was received and stored in `info'
 * @return: false: The given `abs_timeout' has expired. */
FUNDEF bool KCALL
signal_waitfor(USER CHECKED sigset_t const *wait_mask, size_t sigsetsize,
               USER CHECKED siginfo_t *info, jtime_t abs_timeout);


/* Similar to `signal_waitfor()', but replace the calling thread's
 * signal mask with `wait_mask' for the duration of this call,
 * then proceeds to wait until one of the signals masked has been
 * queued for execution in the calling thread.
 * NOTE: This function must not be called for kernel-worker threads.
 * NOTE: When `JTIME_INFINITE' is passed, this function doesn't return.
 * @return: void: The given timeout has expired.
 * @throw: * :                 An RPC function threw this error...
 * @throw: E_INTERRUPT:        The calling thread was interrupted (Dealt with before transitioning to user-space)
 * @throw: E_INVALID_ARGUMENT: The given `sigsetsize' is too large. */
FUNDEF void KCALL
signal_suspend(USER CHECKED sigset_t const *wait_mask,
               size_t sigsetsize, jtime_t abs_timeout);


/* Get a snapshot of all pending signals.
 * NOTE: This function is O(1) when KOS was built
 *       with `CONFIG_SIGPENDING_TRACK_MASK' enabled.
 * @throw: E_INVALID_ARGUMENT: The given `sigsetsize' is too large. */
FUNDEF void KCALL
signal_pending(USER CHECKED sigset_t *wait_mask,
               size_t sigsetsize);

#endif /* __CC__ */


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_POSIX_SIGNALS_H */
