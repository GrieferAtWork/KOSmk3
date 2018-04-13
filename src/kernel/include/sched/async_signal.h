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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_ASYNC_SIGNAL_H
#define GUARD_KERNEL_INCLUDE_SCHED_ASYNC_SIGNAL_H 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <kos/types.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <stdbool.h>
#include <assert.h>
#include <sched/signal.h>


#define ASYNC_SIG_SIZE  (2*__SIZEOF_POINTER__)

#ifdef __CC__
DECL_BEGIN

/* Async-signals operate independently of regular signals:
 *    - `task_push_connections()' does not safe async connections.
 *    - Using regular signals and async signals at the same time is allowed.
 *    - Every async-signal connection requires its own caller-allocated
 *      `struct async_task_connection' structure (usually allocated on-stack)
 *    - Async-signals aren't unordered
 *    - Async-signals are inefficient when it comes to multiple waiters
 *    - `task_wait_async()' only waits for async signals, and `task_wait()'
 *       only waits for regular signals.
 *    - `task_uwait()' (UniversalWAIT) can be used to wait for any kind of signal,
 *       allowing regular signals, and asynchronous signals to be used at once. */

#if 1
struct async_task_connection {
    struct async_task_connection      *atc_thrnext;   /* [0..1][lock(THIS_TASK == atc_thread)]
                                                       * Next async-safe signal connected in this thread. */
    struct task                       *atc_thread;    /* [1..1][const][== THIS_TASK]
                                                       * The connected thread. (Set to `NULL' when the signal was sent) */
    struct async_sig                  *atc_signal;    /* [1..1][const] The signal associated. */
#define ASYNC_SIG_STATUS_WAITING       0              /* [INITIAL] The connection is currently waiting. */
#define ASYNC_SIG_STATUS_DELIVERED     1              /* [lock(WRITE_ONCE)] Final state: The signal was delivered and the primary task was woken (secondary tasks will be woken shortly, or already have been). */
    uintptr_t                          atc_delivered; /* One of `ASYNC_SIG_STATUS_*'. */
};

struct async_sig {
    struct sig                                as_sig; /* Secondary connections. */
    ATOMIC_DATA struct async_task_connection *as_ptr; /* [0..1] The Primary connection. */
};
#define ASYNC_SIG_INIT       { SIG_INIT, NULL }
#define async_sig_init(x)     (void)((x)->as_ptr = NULL,sig_init(&(x)->as_sig))
#define async_sig_cinit(x)   (assert((x)->as_ptr == NULL),sig_cinit(&(x)->as_sig))

/* Connect to an async-safe signal.
 * WARNING: This function is _NOT_ async-safe. Only `async_sig_broadcast()' is!
 * @throw: E_WOULDBLOCK: Preemption has been disabled and a lock couldn't be acquired immediately. */
FUNDEF void KCALL
task_connect_async(struct async_task_connection *__restrict connection,
                   struct async_sig *__restrict signal);

/* Disconnect all currently connected async-safe signals.
 * @throw: E_WOULDBLOCK: Preemption has been disabled and a lock couldn't be acquired immediately.
 * @return: * :          Randomly, one of the signals that were delivered.
 * @return: NULL:        No signals were sent. */
FUNDEF struct sig *KCALL task_disconnect_async(void);

/* Wait for async-signals to be boardcast and disconnect all remaining connections.
 * @throw: * :           This error was thrown by an RPC function call.
 * @throw: E_INTERRUPT:  The calling thread was interrupted.
 * @throw: E_WOULDBLOCK: Preemption has been disabled and a lock couldn't be acquired immediately.
 * @return: * :          Randomly, one of the signals that were delivered.
 * @return: NULL:        The given timeout has expired. */
FUNDEF ATTR_RETNONNULL struct sig *KCALL task_wait_async(void);
FUNDEF struct sig *KCALL task_waitfor_async(jtime_t abs_timeout);

/* Wake a task waiting for the given async-signal.
 * NOTE: This function is ASYNC-SAFE, meaning it can safely be used from interrupt handlers.
 *       Also note that the requirement of an async-safe signal-broadcast function
 *       was the only reason why async-safe signals had to be implemented.
 *       Most notably is their use by keyboard input buffers,
 *       which are then filled by interrupt handlers.
 * @return: true:  A task was waiting for the signal. - That task may wake other waiters.
 * @return: false: No task was waiting for the signal. */
FUNDEF ASYNCSAFE bool KCALL async_sig_broadcast(struct async_sig *__restrict signal);

/* Wait for any kind of signal and disconnect all before returning.
 * NOTE: These functions must be used for handling generic poll-operators
 *       of anonymous handles or devices, as the type of signal used by
 *       them isn't defined and can be either asynchronous, or synchronous.
 * @return: * :   A pointer to a `struct async_sig' or `struct sig' that was sent.
 * @return: NULL: The given `abs_timeout' has expired. */
#define task_uwait()               task_wait_async()
#define task_uwaitfor(abs_timeout) task_waitfor_async(abs_timeout)

struct timespec;
/* Helper functions for user-space wait requests. */
FUNDEF struct sig *KCALL task_uwaitfor_tmrel(USER CHECKED struct timespec const *rel_timeout); /* Relative timeout */
FUNDEF struct sig *KCALL task_uwaitfor_tmabs(USER CHECKED struct timespec const *abs_timeout); /* Absolute timeout */

/* Disconnect any kind of signal.
 * @return: * :   A pointer to a `struct async_sig' or `struct sig' that was sent.
 * @return: NULL: No signals were sent, or have been connected. */
#define task_udisconnect() task_disconnect_async()
#else
struct async_task_connection {
    struct async_task_connection      *atc_thrnext;   /* [0..1][lock(THIS_TASK == atc_thread)]
                                                       * Next async-safe signal connected in this thread. */
    struct task                       *atc_thread;    /* [1..1][const][== THIS_TASK]
                                                       *  The connected thread. (Set to `NULL' when the signal was sent) */
    struct async_sig                  *atc_sig;       /* [1..1][const] The bound signal. */
    struct {
        struct async_task_connection **cs_pself;      /* [1..1][==self][0..1][lock(atc_sig->as_lock)]
                                                       * [valid_if(self != atc_sig->as_ptr)] Self pointer. */
        struct async_task_connection  *cs_next;       /* [0..1][lock(atc_sig->as_lock)]
                                                       * A chain of secondary threads that this one
                                                       * must wake when this async-signal is received. */
    }                                  atc_secondary; /* Secondary connections. */
#define ASYNC_SIG_STATUS_WAITING       0              /* [INITIAL] The connection is currently waiting. */
#define ASYNC_SIG_STATUS_DELIVERING    1              /* [lock(WRITE_ONCE,in(async_sig_broadcast))] The signal is currently being delivered (Set interlocked with `ASYNC_SIG_PTR_DELIVERING') */
#define ASYNC_SIG_STATUS_DELIVERED     2              /* [lock(WRITE_ONCE)] Final state: The signal was delivered and the primary task was woken (secondary tasks will be woken shortly, or already have been). */
    uintptr_t                          atc_delivered; /* One of `ASYNC_SIG_STATUS_*'. */
};

struct async_sig {
#define ASYNC_SIG_PTR_DELIVERING  ((struct async_task_connection *)-1)
    ATOMIC_DATA struct async_task_connection *as_ptr;  /* [0..1] The task connected to this signal.
                                                        * NOTE: Set to `ASYNC_SIG_PTR_DELIVERING' while the signal is being delivered. */
    atomic_rwlock_t                           as_lock; /* Lock for the chain of secondary tasks. */
};
#define ASYNC_SIG_INIT       { NULL, ATOMIC_RWLOCK_INIT }
#define async_sig_init(x)     (void)((x)->as_ptr = NULL,atomic_rwlock_init(&(x)->as_lock))
#define async_sig_cinit(x)   (assert((x)->as_ptr == NULL),atomic_rwlock_cinit(&(x)->as_lock))

/* Connect to an async-safe signal.
 * WARNING: This function is _NOT_ async-safe. Only `async_sig_broadcast()' is!
 * @throw: E_WOULDBLOCK: Preemption has been disabled and a lock couldn't be acquired immediately. */
FUNDEF void KCALL
task_connect_async(struct async_task_connection *__restrict connection,
                   struct async_sig *__restrict signal);

/* Disconnect all currently connected async-safe signals.
 * @throw: E_WOULDBLOCK: Preemption has been disabled and a lock couldn't be acquired immediately.
 * @return: * :          Randomly, one of the signals that were delivered.
 * @return: NULL:        No signals were sent. */
FUNDEF struct async_sig *KCALL task_disconnect_async(void);

/* Wait for async-signals to be boardcast and disconnect all remaining connections.
 * @throw: * :           This error was thrown by an RPC function call.
 * @throw: E_INTERRUPT:  The calling thread was interrupted.
 * @throw: E_WOULDBLOCK: Preemption has been disabled and a lock couldn't be acquired immediately.
 * @return: * :          Randomly, one of the signals that were delivered.
 * @return: NULL:        The given timeout has expired. */
FUNDEF ATTR_RETNONNULL struct async_sig *KCALL task_wait_async(void);
FUNDEF struct async_sig *KCALL task_waitfor_async(jtime_t abs_timeout);

/* Wake a task waiting for the given async-signal.
 * NOTE: This function is ASYNC-SAFE, meaning it can safely be used from interrupt handlers.
 *       Also note that the requirement of an async-safe signal-broadcast function
 *       was the only reason why async-safe signals had to be implemented.
 *       Most notably is their use by keyboard input buffers,
 *       which are then filled by interrupt handlers.
 * @return: true:  A task was waiting for the signal. - That task may wake other waiters.
 * @return: false: No task was waiting for the signal. */
FUNDEF ASYNCSAFE bool KCALL async_sig_broadcast(struct async_sig *__restrict signal);



/* Wait for any kind of signal and disconnect all before returning.
 * NOTE: These functions must be used for handling generic poll-operators
 *       of anonymous handles or devices, as the type of signal used by
 *       them isn't defined and can be either asynchronous, or synchronous.
 * @return: * :   A pointer to a `struct async_sig' or `struct sig' that was sent.
 * @return: NULL: The given `abs_timeout' has expired. */
FUNDEF ATTR_RETNONNULL void *KCALL task_uwait(void);
FUNDEF void *KCALL task_uwaitfor(jtime_t abs_timeout);

struct timespec;
/* Helper functions for user-space wait requests. */
FUNDEF void *KCALL task_uwaitfor_tmrel(USER CHECKED struct timespec const *rel_timeout); /* Relative timeout */
FUNDEF void *KCALL task_uwaitfor_tmabs(USER CHECKED struct timespec const *abs_timeout); /* Absolute timeout */

/* Disconnect any kind of signal.
 * @return: * :   A pointer to a `struct async_sig' or `struct sig' that was sent.
 * @return: NULL: No signals were sent, or have been connected. */
FUNDEF void *KCALL task_udisconnect(void);
#endif



DECL_END
#endif /* __CC__ */

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_ASYNC_SIGNAL_H */
