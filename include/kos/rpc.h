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
#ifndef _KOS_RPC_H
#define _KOS_RPC_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <bits/types.h>

#if defined(__i386__) || defined(__x86_64__)
#include "i386-kos/asm/rpc.h"
#else
#error "Unsupported arch"
#endif

__DECL_BEGIN

/* User-space RPC is a functionality exclusive to KOS that allows for
 * user-space threads to schedule functions to-be executed by one another,
 * either asynchronously (meaning at some arbitrary time), or synchronously
 * (meaning once the targeted thread performs a blocking operation, or if
 * the thread was already blocking when the RPC was issued, immediately)
 * Their behavior is quite similar to that of POSIX_SIGNALS, however they
 * offer much better control over how and when execution occurs, while not
 * relying on an arbitrarily limited set of signal numbers which the target
 * thread must know how to handle, the main problem therein being that signals
 * are absolutely useless for libraries, since there are literally only 2
 * signals that can be used for arbitrary purposes.
 * Anyways. KOS's user-space RPC functionality has the same power to interrupt
 * blocking system calls, but when scheduling an RPC, you tell just kernel what
 * function, with which argument to invoke. And believe me: This is a _very_
 * powerful mechanism. As a matter of fact, in kernel-space practically all
 * inter-process communication and synchronization is done using RPCs:
 *  - You want to terminate a thread? Schedule an RPC that throws an E_EXIT_THREAD
 *  - You want to inspect variables private to that thread? Use an RPC!
 * The list goes on... */




/* RPC return codes.
 * The actual return behavior is performed like this (in that order):
 * >> if (!(reason & RPC_REASON_SYSCALL))    RESUME();
 * >> if (return == RPC_RETURN_RESUME)       RESUME();
 * >> if (reason & RPC_REASON_FMUST_RESTART) RESTART();
 * >> if (return == RPC_RETURN_RESTART) {
 * >>     if (reason & RPC_REASON_FSHOULD_RESTART)
 * >>         RESTART();
 * >>     INTERRUPT();
 * >> }
 * >> if (return == RPC_RETURN_INTERRUPT)
 * >>     INTERRUPT();
 * >> RESTART();
 * RESUME:
 *   Resume execution at the saved register state.
 *   This behavior is forced when is wasn't a system call
 *   that was interrupted (only possible for asynchronous RPCs)
 * RESTART:
 *   Rewind to re-execute the last instruction in the
 *   calling context, using the saved register state.
 *   Only done if the RPC interrupted a system call!
 * INTERRUPT:
 *   Throw an `E_INTERRUPT' exception (which may be
 *   translated into `EINTR' if the interrupted system
 *   call as invoked without exception support)
 *   >> if (reason & RPC_REASON_NOEXCEPT) {
 *   >>     errno = EINTR;
 *   >>     return ERROR_RETURN;
 *   >> }
 *   >> error_throw(E_INTERRUPT);
 * NOTES:
 *   - Exceptions thrown by a RPC function that interrupted
 *     a system call invoked without exceptions enabled will
 *     have the exception it threw translated into an errno.
 *   - An exception to this rule are high-priority exceptions,
 *     as indicated by `ERRORCODE_ISRTLPRIORITY()', which are
 *     always propagated. */
#define RPC_RETURN_RESTART         0 /* Restart the system call that was interrupted by the RPC,
                                      * but only if the `RPC_REASON_FSHOULD_RESTART' reason flag is set.
                                      * Otherwise, throw an error like `RPC_RETURN_INTERRUPT' would.
                                      * The behavior caused by this return value mirrors that of
                                      * a POSIX_SIGNAL with the SA_RESTART flag set. */
#define RPC_RETURN_FORCE_RESTART   1 /* Restart the system call that was interrupted by the RPC. */
#define RPC_RETURN_INTERRUPT       2 /* Don't restart the system call, but throw an E_INTERRUPT exception.
                                      * Just like any other exception thrown by an RPC invoked by an
                                      * interrupted system call, the exception will either be propagated
                                      * to the system call invocation site, or be translated into an errno
                                      * value which is emulated as though it was returned by the system call. */
#define RPC_RETURN_RESUME          3 /* Resume execution at the interrupted call-site.
                                      * NOTE: When returned by `rpc_interrupt_t', the function
                                      *       may have updated `*CTX', in which case the updated
                                      *       variant is loaded, allowing well-timed RPCs to
                                      *       implement additional behavior for system calls when
                                      *       they were used to interrupt them.
                                      * WARNING: Returning this from an `rpc_t' will leave the system call return
                                      *          value undefined (or at least have it contain unexpected values)!
                                      *          This value should only be returned from an `rpc_interrupt_t'
                                      *          after it modified the provided `struct cpu_context' */



/* An RPC callback executed by a target thread.
 * @param: arg: The argument passed to `queue_rpc'
 * @return: * : One of `RPC_RETURN_*' describing how the interrupt
 *              operation should fail, or be restarted.
 *        NOTE: The return value is ignored for asynchronous RPC callbacks! */
#ifdef __CC__
typedef unsigned int (__LIBCCALL *rpc_t)(void *__arg);
#endif /* __CC__ */

#define RPC_REASON_ASYNC           0x0000 /* The RPC is being served asynchronously. */
#define RPC_REASON_DIDBLOCK        0x0001 /* FLAG: The interrupt was triggered when the thread was blocking.
                                           *       This means that `CTX' refers to a piece of system call
                                           *       entry assembly. However since some platforms offer multiple
                                           *       ways of entering the kernel, the register format of CTX
                                           *       is saved in the arch-specific portion of the reason.
                                           *       e.g. On X86, the associated flags are `X86_RPC_REASON_*',
                                           *            as found in <kos/i386-kos/asm/rpc.h> (already included above) */
#define RPC_REASON_SYSCALL         0x0002 /* The RPC interrupted a system call */
#define RPC_REASON_NOEXCEPT        0x0004 /* (Used with `RPC_REASON_SYSCALL') The system call was invoked without exceptions. */
#define RPC_REASON_FSHOULD_RESTART 0x0008 /* FLAG: The interrupted system call should be restarted. */
#define RPC_REASON_FMUST_RESTART   0x0010 /* FLAG: The interrupted system call must be restarted. */
#define RPC_REASON_MASK            0x00ff /* Mask of platform-independent flags. */
#ifdef __CC__
struct cpu_context;
typedef unsigned int (__LIBCCALL *rpc_interrupt_t)(void *__arg, struct cpu_context *__restrict __ctx, unsigned int __reason);
#endif /* __CC__ */





#ifdef __CC__
#ifndef __KERNEL__
/* Queue an RPC callback, or interrupt, to-be executed the thread `PID'.
 * @param: MODE:   Set of `RPC_F*'
 * @return: true / >0: The RPC was successfully enqueued.
 *                     WARNING: Unless `RPC_FWAITFOR' is passed as well, there is no way
 *                              of confirming that the RPC finished, or even started.
 * @return: false / 0: The thread terminated before execution of the RPC could be completed.
 * @return: -1:       [queue_rpc] See errno.
 * @throw: E_BADALLOC:       Failed to allocate the internal descriptor for the RPC.
 * @throw: E_SEGFAULT:       The given `JOB' is faulty.
 * @throw: E_PROCESS_EXITED: The given PID is invalid, or the associated thread was detached and died. */
__REDIRECT_EXCEPT(__LIBC,,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,queue_rpc,
                 (__pid_t __pid, rpc_t __func, void *__arg, unsigned int __mode),
                 (__pid,__func,__arg,__mode))
#ifdef __USE_EXCEPT
__LIBC __BOOL (__LIBCCALL Xqueue_rpc)(__pid_t __pid, rpc_t __func, void *__arg, unsigned int __mode);
#endif /* __USE_EXCEPT */

/* Same as `queue_rpc()', but the queued function is
 * invoked with 3 arguments, rather than a single.
 * Because of the requirement of generating a full cpu_context,
 * this version is slightly slower than `queue_rpc()', however
 * using this one, an RPC is able to manipulate the location
 * where the RPC will return to (s.a. `RPC_RETURN_RESUME')
 * @param: MODE:   Set of `RPC_F*'
 * @return: true / >0: The RPC was successfully enqueued.
 *                     WARNING: Unless `RPC_FWAITFOR' is passed as well, there is no way
 *                              of confirming that the RPC finished, or even started.
 * @return: false / 0: The thread terminated before execution of the RPC could be completed.
 * @return: -1:       [queue_interrupt] See errno.
 * @throw: E_BADALLOC:       Failed to allocate the internal descriptor for the RPC.
 * @throw: E_SEGFAULT:       The given `JOB' is faulty.
 * @throw: E_PROCESS_EXITED: The given PID is invalid, or the associated thread was detached and died. */
__REDIRECT_EXCEPT(__LIBC,,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,queue_interrupt,
                 (__pid_t __pid, rpc_interrupt_t __func, void *__arg, unsigned int __mode),
                 (__pid,__func,__arg,__mode))
#ifdef __USE_EXCEPT
__LIBC __BOOL (__LIBCCALL Xqueue_interrupt)(__pid_t __pid, rpc_interrupt_t __func, void *__arg, unsigned int __mode);
#endif /* __USE_EXCEPT */
#endif /* !__KERNEL__ */
#endif /* __CC__ */
#define RPC_FSYNCHRONOUS  0x0000 /* The RPC will be served once the thread attempts a blocking operation.
                                  * Note that certain kernel operations that can normally block will not
                                  * trigger synchronous RPC servings. These include any code invoked by
                                  * hardware interrupts, or CPU exceptions/traps (including #PF when not
                                  * used to execute a system call), as well as exception propagation.
                                  * Note however that asynchronous RPCs can truly be handled between
                                  * any 2 instruction (even while already inside an ASYNC RPC callback)
                                  * Also note that this does not necessarily imply that it was a system
                                  * call which got interrupted. - user-space RPCs are also served by the
                                  * kernel when an exception is propagated into user-space, meaning that
                                  * a system call throwing an exception will serve RPCs, as well as any
                                  * operation that can trigger a hardware exception (like DIVIDE_BY_ZERO)
                                  * will server synchronous RPCs as well! */
#define RPC_FASYNCHRONOUS 0x0001 /* The RPC will be served at a random point in time, or once the thread attempts a blocking operation.
                                  * Note however that the RPC will not be served while the thread is in kernel-space!
                                  * Asynchronous interrupts can only happen while in user-space, so as far as they go,
                                  * you can view any system call as though it as an atomic operation.
                                  * Additionally, serving of asynchronous interrupts can be disabled
                                  * locally though use of `rpc_pushoff()' and `rpc_pop()'. */
#define RPC_FWAITACK      0x0002 /* Wait until the thread has acknowledged the RPC.
                                  * WARNING: Anything might still go wrong before, or during execution
                                  *          of `FUNC', including the thread being kill(2)-ed off, or
                                  *          some other `RPC_FASYNCHRONOUS' RPC being executed ontop. */
#define RPC_FWAITFOR      0x0004 /* Wait for execution of the RPC to finish. (Implemented in user-space) */

#ifdef __CC__
#ifndef __KERNEL__
/* Test for pending RPC callbacks to be served, and serve them synchronously.
 * NOTE: Any pending asynchronous RPCs are served as well (causing TRUE to be
 *       returned, also), but also note that new ones may be scheduled by other
 *       threads before the functions returns, at which point they can still be
 *       served at any arbitrary point in time.
 * @return: true / 1:  RPC jobs have been served. (meaning that the
 *                     states of thread-local variables may have changed)
 * @return: false / 0: Nothing was served.
 * @return: -1:       [rpc_serve] See errno.
 * @throw: * :         An exception thrown by an RPC callback.
 * NOTE: Just so you know, synchronous RPCs are
 *       _NOT_ served by calls to `sched_yield()'! */
__REDIRECT_EXCEPT(__LIBC,,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,rpc_serve,(void),())
#ifdef __USE_EXCEPT
__LIBC __BOOL (__LIBCCALL Xrpc_serve)(void);
#endif /* __USE_EXCEPT */

/* Recursively disable reception of asynchronous RPC callbacks.
 * NOTE: This also temporarily disables delivery of asynchronous
 *       posix signals with the exception of `SIGKILL' and `SIGSTOP'
 * @return: true / 1:  The ASYNC-RPC-serve state changed.
 * @return: false / 0: The ASYNC-RPC-serve state didn't change.
 * @return: -1:       [rpc_pop] See errno.
 * @throw: * :         An exception thrown by an RPC callback. */
__LIBC __ATTR_NOTHROW __BOOL (__LIBCCALL rpc_pushoff)(void);
__REDIRECT_EXCEPT(__LIBC,,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,rpc_pop,(void),())
#ifdef __USE_EXCEPT
__LIBC __BOOL (__LIBCCALL Xrpc_pop)(void);
#endif /* __USE_EXCEPT */
#endif /* !__KERNEL__ */


#ifndef __KERNEL__
/* This is the actual kernel interface for queuing RPCs.
 * It is exposed here, but is way too low-level and arch-specific
 * to really be of any use other than to implement the higher-level
 * functions already seen above.
 * WARNING: This function should only be used for thread running as
 *          part of the same VM, or at the very least in a VM who's
 *          address space layout is known to the sender.
 * @param: MODE: Set of `JOB_F*'
 * @return: true / 1:           The job was successfully enqueued.
 * @return: false / 0:          The thread terminated before execution of the job could be completed.
 * @return: -1:                [queue_job] See errno.
 * @throw: E_BADALLOC:          Failed to allocate the internal descriptor for the job.
 * @throw: E_SEGFAULT:          The given `JOB' is faulty.
 * @throw: E_PROCESS_EXITED:    The given `PID' is invalid, or the associated thread was detached and died.
 * @throw: E_ILLEGAL_OPERATION: The process does not have permission to send signals to `PID'
 *                              The permission checking here is done the same way it is done for `kill(2)' */
__REDIRECT_EXCEPT(__LIBC,,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,queue_job,
                 (__pid_t __pid, struct cpu_context const *__restrict __job, unsigned int __mode),
                 (__pid,__job,__mode))
#ifdef __USE_EXCEPT
__LIBC __BOOL (__LIBCCALL Xqueue_job)(__pid_t __pid, struct cpu_context const *__restrict __job, unsigned int __mode);
#endif /* __USE_EXCEPT */
#endif /* !__KERNEL__ */
#endif /* __CC__ */
#define JOB_FSYNCHRONOUS  RPC_FSYNCHRONOUS  /* Serve the job synchronously */
#define JOB_FASYNCHRONOUS RPC_FASYNCHRONOUS /* Serve the job asynchronously */
#define JOB_FWAITACK      RPC_FWAITACK      /* Wait for ACK */
#define JOB_FPRESERVE     0xff00 /* Arch-specific mask of what to push onto the target's
                                  * stack, as well as which registers to load from `JOB'.
                                  * On X86, this is a set of `X86_JOB_FSAVE_*' and `X86_JOB_FLOAD_*' */


__DECL_END

#endif /* !_KOS_RPC_H */
