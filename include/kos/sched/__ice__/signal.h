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
#ifndef _KOS_SCHED_FUTEX_H
#define _KOS_SCHED_FUTEX_H 1

#include <hybrid/compiler.h>
#include <hybrid/__atomic.h>
#include <hybrid/timespec.h>
#include <bits/types.h>
#include <assert.h>
#include <features.h>

#ifdef __KERNEL__
#include <sched/task.h>
#include <sched/signal.h>

#define CONFIG_THREAD_STATIC_CONNECTIONS  CONFIG_TASK_STATIC_CONNECTIONS
#define thread_connection                 task_connection
#define thread_connections                task_connections
#define thread_connect                    task_connect
#define thread_connected                  task_connected
#define thread_isconnected                task_isconnected
#define thread_setsignaled                task_setsignaled
#define thread_disconnect                 task_disconnect
#define thread_push_connections           task_push_connections
#define thread_pop_connections            task_pop_connections
#define thread_trywait                    task_trywait
#define thread_wait                       task_wait
//#define thread_waitfor                  task_waitfor // Uses jiffies in kernel-space

#else

DECL_BEGIN

struct task_segment;

/* In addition to linux futexes, KOS also implements its
 * own with a word with that equals that of pointers
 * allowing user-space to use pointers in futex data
 * words, opening the ability of implementing waiter
 * lists in userspace. */
typedef __uintptr_t futex_word_t;

/* Basic thread-local context control functions. */
__REDIRECT(__LIBC,__WUNUSED __ATTR_CONST,__pid_t,__LIBCCALL,__gettid,(void),gettid,())
__REDIRECT(__LIBC,,int,__LIBCCALL,__sched_yield,(void),sched_yield,())
__LIBC __WUNUSED __ATTR_RETNONNULL __ATTR_CONST struct task_segment *(__LIBCCALL __current)(void);




/* Since KOS is designed to only run on machines with a
 * pointer size of at least 32 bits, that means that
 * all instances of `struct thread_connection' must be
 * aligned by at least 4 bytes.
 * With that in mind, we can use the bottom 2 bits for
 * our own purposes, in this case: As locking bits.
 * This way, we keep `struct sig' the size of a single
 * pointer, meaning it is one of the host light-weight
 * types there are (which is important, considering
 * that signals are the bottom-most structured used
 * to implement all other synchronization primitives) */
#define SIG_FLOCKBIT     0x1   /* Primary locking bit. */
#define SIG_FLOCKBIT2    0x2   /* Secondary locking bit (freely available to the user of the signal)
                                * This bit could be used to implement a quick, non-recursive mutex. */
#define SIG_FLOCKMASK    0x3
#define SIG_FADDRMASK  (~0x3)

struct sig {
    __uintptr_t s_ptr; /* [TYPE(struct thread_connection)]
                        *  Chain of connected threads. */
};
#define SIG_INIT       { 0 }
#define SIG_GETCON(x) ((struct thread_connection *)((x)->s_ptr & SIG_FADDRMASK))
#define sig_init(x)    (void)((x)->s_ptr = 0)
#define sig_cinit(x)   assert((x)->s_ptr == 0)
#define sig_holding(x) ((x)->s_ptr & SIG_FLOCKBIT)

/* Acquire / release a lock on the given signal. */
__FORCELOCAL __ATTR_NOTHROW __BOOL (__LIBCCALL sig_try)(struct sig *__restrict __self) {
 return !(__hybrid_atomic_fetchor(__self->s_ptr,SIG_FLOCKBIT,__ATOMIC_SEQ_CST) & SIG_FLOCKBIT);
}
__FORCELOCAL __ATTR_NOTHROW void (__LIBCCALL sig_get)(struct sig *__restrict __self) {
 while (__hybrid_atomic_fetchor(__self->s_ptr,SIG_FLOCKBIT,__ATOMIC_SEQ_CST) & SIG_FLOCKBIT)
     __sched_yield();
 __COMPILER_BARRIER();
}
__FORCELOCAL __ATTR_NOTHROW void (__LIBCCALL sig_put)(struct sig *__restrict __self) {
 __COMPILER_BARRIER();
#ifdef NDEBUG
 __hybrid_atomic_fetchand(__self->s_ptr,~SIG_FLOCKBIT,__ATOMIC_SEQ_CST);
#else
 __assertef(__hybrid_atomic_fetchand(__self->s_ptr,~SIG_FLOCKBIT,__ATOMIC_SEQ_CST) & SIG_FLOCKBIT,
            "__self = %p, __self->s_ptr = %p",__self,__self->s_ptr);
#endif
}

/* Send signal `__self' to up to `max_threads' connected threads.
 * @return: * : The actual number of threads notified. */
__LIBC __ATTR_NOTHROW __size_t (__LIBCCALL sig_send)(struct sig *__restrict __self, __size_t __max_threads);
__LIBC __ATTR_NOTHROW __size_t (__LIBCCALL sig_broadcast)(struct sig *__restrict __self);

/* Same as `sig_send()', but the caller is responsible for holding
 * a lock to `__self', and that lock will not be let go of in-between
 * sending the signal to different threads. */
__LIBC __ATTR_NOTHROW __size_t (__LIBCCALL sig_send_locked)(struct sig *__restrict __self, __size_t __max_threads);
__LIBC __ATTR_NOTHROW __size_t (__LIBCCALL sig_broadcast_locked)(struct sig *__restrict __self);


/* Same as `sig_send()', but only wake thread connected to
 * any of the channels that are masked by `signal_mask'.
 * For more information, see the detailed documentation of
 * this functionality at the declaration of `thread_channelmask()'.
 * @param: signal_mask: The mask of channels that should be signaled.
 * @return: * : The actual number of threads notified. */
__LIBC __ATTR_NOTHROW __size_t (__LIBCCALL sig_send_channel)(struct sig *__restrict __self, __uintptr_t __signal_mask, __size_t __max_threads);
__LIBC __ATTR_NOTHROW __size_t (__LIBCCALL sig_broadcast_channel)(struct sig *__restrict __self, __uintptr_t __signal_mask);
__LIBC __ATTR_NOTHROW __size_t (__LIBCCALL sig_send_channel_locked)(struct sig *__restrict __self, __uintptr_t __signal_mask, __size_t __max_threads);
__LIBC __ATTR_NOTHROW __size_t (__LIBCCALL sig_broadcast_channel_locked)(struct sig *__restrict __self, __uintptr_t __signal_mask);



/* Process-local RPC (Remote Procedure Call) API.
 * NOTE: This API is implemented in user-space, meaning that the kernel
 *       is not involved in any way. */


/* Check for pending RPC functions yet to be executed.
 * NOTE: When `thread_nothrow_serve()' has been invoked by the calling
 *       thread, this function doesn't propagate any exceptions,
 *       but instead delays them until `thread_nothrow_end()' has
 *       been called a sufficient number of times.
 * NOTE: In the event that RPC function calls are served, this function
 *       will automatically save the calling thread's signal connection
 *       set, meaning that the caller is not required to save it, nor are
 *       RPC functions that may use signal-based connections required to.
 * If an error is thrown, previously connected signals are disconnected after being restored.
 * @throw: * :     A scheduled RPC function threw an error.
 * @return: true:  At least one RPC function was served.
 * @return: false: No RPC functions were served. */
__LIBC __BOOL (__LIBCCALL thread_serve)(void);

/* Recursively begin/end `thread_serve()' being `ATTR_NOTHROW'. */
__LIBC __ATTR_NOTHROW void (__LIBCCALL thread_nothrow_serve)(void);
__LIBC void (__LIBCCALL thread_nothrow_end)(void);





/* Task signaling API. (Low-level scheduling) */
#define CONFIG_THREAD_STATIC_CONNECTIONS  3

struct thread_connection {
    struct thread_connections            *tc_conn;  /* [1..1][lock(tc_sig)][== :self] Pointer to the associated connection set. */
    struct sig                           *tc_sig;   /* [0..1][lock(THIS_TASK)] When non-NULL, pointer to the signal to which this connection is bound. */
    union PACKED {
        struct thread_connection         *tc_last;  /* [valid_if(tc_sig && INTERN(self == SIG_GETCON(tc_sig)))]
                                                     * [lock(tc_sig)][1..1] Pointer to the latest connection established with `tc_sig' */
        struct thread_connection        **tc_pself; /* [valid_if(tc_sig && INTERN(self != SIG_GETCON(tc_sig)))]
                                                     * [lock(tc_sig)][== self][0..1] Self-pointer / pointer (in-)to the previous connect. */
    };
    struct thread_connection             *tc_next;  /* [valid_if(tc_sig && tc_pself)][lock(tc_sig)][0..1]
                                                     * Pointer to the next connection. */
};

struct thread_connections {
    struct thread_connection  tcs_sbuf[CONFIG_THREAD_STATIC_CONNECTIONS];
    __SIZE_TYPE__             tcs_siz; /* [!0][lock(THIS_TASK)] Allocated number of connections. */
    __SIZE_TYPE__             tcs_cnt; /* [lock(THIS_TASK)] Number of used connections. */
    struct thread_connection *tcs_vec; /* [1..tcs_cnt|ALLOC(tcs_siz)][owned_if(!= tcs_sbuf)]
                                        * [lock(THIS_TASK)] Vector of connections. */
    struct sig               *tcs_sig; /* [atomic] The first signal that was received. */
    struct thread            *tcs_tsk; /* The thread to which this connection set is bound. */
};



/* Connect the calling thread to the given signal.
 * If there is insufficient memory to establish the connection, the set of
 * already connected signals is disconnected and an `E_BADALLOC' error is thrown.
 * NOTE: Any thread is guarantied to sustain at least `CONFIG_THREAD_STATIC_CONNECTIONS'
 *       simultaneous connections without running the chance to not be able to
 *       connect to more due to lack of resources.
 * If the given `signal' was already connected, this function is a no-op.
 * Note however, that the implementation will still register the signal
 * a second time, knowing that when send, the calling thread will only
 * receive it once.
 * @throw: E_BADALLOC: There are already more than `CONFIG_THREAD_STATIC_CONNECTIONS'
 *                     other connections, and not enough available memory to allocate
 *                     a new buffer for more. */
__LIBC void (__LIBCCALL thread_connect)(struct sig *__restrict __signal);

/* Check if the calling thread is currently connected to `signal'. */
__LIBC __ATTR_NOTHROW __BOOL (__LIBCCALL thread_connected)(struct sig *__restrict __signal);

/* Check if the calling thread is connected to any signals. */
__LIBC __ATTR_NOTHROW __BOOL (__LIBCCALL thread_isconnected)(void);

/* Switching the calling thread to a signaled-state with `signal'
 * being what was supposedly send. If the thread already was set
 * to a signaled state, return `false', otherwise `true'.
 * Signaled state here means that the next call to `thread_[try]wait[for]'
 * will return immediately, before serving RPC functions, and before
 * potentially changing the calling thread's state to sleeping.
 * WARNING: In order to become signaled, `signal' must be connected
 *          with, meaning that you must call this function following
 *          a previous call to `thread_connect()' with the same signal. */
__LIBC __ATTR_NOTHROW __BOOL (__LIBCCALL thread_setsignaled)(struct sig *__restrict signal);

/* Disconnect all connected signals, returning `NULL' if none
 * had been sent yet, or a pointer to the signal that was sent. */
__LIBC __ATTR_NOTHROW struct sig *(__LIBCCALL thread_disconnect)(void);

/* Push (safe the current, then load an empty set) / pop (restore the given set) of connections.
 * This function's main intent is its use to guard RPC functions from modifying
 * the active connections set of wherever they were called from.
 * Note that saved connections will continue to listen for signals,
 * and once restored using `thread_pop_connections()', those signals
 * will immediately become available to the calling thread, meaning
 * that a signal sent while connections were saved will keep the
 * associated thread from sleeping during the next call to `thread_wait()'. */
__LIBC __ATTR_NOTHROW void (__LIBCCALL thread_push_connections)(struct thread_connections *__restrict safe);
__LIBC __ATTR_NOTHROW void (__LIBCCALL thread_pop_connections)(struct thread_connections *__restrict safe);

/* Without blocking, check if any signal has been sent.
 * If so, disconnect from all other connected signals and
 * return a pointer to the signal that was discovered to be sent. */
__LIBC __ATTR_NOTHROW struct sig *(__LIBCCALL thread_trywait)(void);

/* Wait for any of the connected signals to be sent and
 * return a pointer to the first signal fulfilling this
 * requirement.
 * For the duration of waiting for signals, the calling
 * thread will be unscheduled, meaning that no CPU time
 * is used by this operation.
 * Only send operations executed after connecting to a
 * signal will trigger this function to return. However
 * when a signal was sent between connecting to it, and
 * these function being called, they will return immediately.
 * Otherwise, prior to being unscheduled, `thread_serve()' is
 * called internally to serve any RPC functions that are
 * supposed to run in the calling thread.
 * If an RPC function throws an error, all connected signals are
 * disconnected and the function returns immediately by rethrowing
 * the previous error.
 * Function calling this function are called `synchronization points'
 * and all feature the same exception characteristics.
 * @throw: * : A scheduled RPC callback threw this error.
 * NOTE: Passing `{0,0}' for `abs_timeout' behaves the same as `thread_trywait()'
 * NOTE: Passing `NULL' for `abs_timeout' behaves the same as `thread_wait()',
 *       meaning that the function will not return `NULL' in this case. */
__LIBC __ATTR_RETNONNULL struct sig *(__LIBCCALL thread_wait)(void);
__REDIRECT_TM_FUNC(__LIBC,,struct sig *,__LIBCCALL,thread_waitfor,(struct timespec const *abs_timeout),thread_waitfor,(abs_timeout))
#ifdef __USE_TIME64
__LIBC struct sig *(__LIBCCALL thread_waitfor64)(struct timespec64 const *abs_timeout);
#endif


#if 0 /* TODO: Requires some trickery to prevent a race condition
       *       because in user-space we can't just access thread-local
       *       memory of another thread, not knowing if that thread
       *       might terminate (and unmap() that memory) while we do so... */
typedef void (KCALL *thread_rpc_t)(void *arg);

/* Queue a remote procedure call to-be executed the next time `thread_serve()' is called.
 * Scheduled RPC functions are executed in order or being queued.
 * NOTE: If the thread is currently performing a blocking system call,
 *       that call will be interrupted (causing it to return `-EINTR',
 *       similar to when a signal handler was executed)
 * @param: func:   The function that should be executed in the context of `thread'.
 *                 The function will be run the next time `thread' calls `thread_serve()'.
 * @param: mode:   Set of `THREAD_RPC_*'
 * @return: true:  Successfully queued the RPC (though the thread may terminate
 *                 before execution finished unless `THREAD_RPC_SYNC' was also set)
 * @return: false: No thread matching the given `tid' exists as part of the
 *                 calling processes thread-group and VM.
 * NOTE: When execution of the RPC is requested to be performed synchronously,
 *      `thread_wait()' is used to wait for the function to complete, meaning
 *       that this function is a synchronization point for executing RPC
 *       functions scheduled for the calling thread.
 * NOTE: If `thread' is the calling thread, `func' is executed immediately
 *       and `true' is returned regardless of `THREAD_RPC_SYNC'. */
__LIBC __BOOL (__LIBCCALL thread_queue_rpc)(__pid_t __tid, thread_rpc_t __func, void *__arg, unsigned int __mode);


#define THREAD_RPC_NORMAL 0x0000 /* Normal RPC flags. */
#define THREAD_RPC_SYNC   0x0001 /* Synchronize execution of the RPC.
                                  * When set, `thread_queue_rpc()' will not return until after
                                  * the given `func()' has been executed and has returned. */
#define THREAD_RPC_SINGLE 0x4000 /* Optional, optimization flag:
                                  * Check if the given `func' and `arg' have already been
                                  * scheduled with the same `mode & THREAD_RPC_USER' combination.
                                  * If so, the implementation is allowed not to schedule the
                                  * functions a second time, but instead return as a no-op
                                  * (when `THREAD_RPC_SYNC' isn't set), or wait until the existing
                                  * RPC callback has finished.
                                  * NOTE: This function can be ignored if the implementation
                                  *       chooses not to support it (as KOS currently does). */
#endif


#endif

DECL_END


#endif /* !_KOS_SCHED_FUTEX_H */
