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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_ONEXIT_H
#define GUARD_KERNEL_INCLUDE_SCHED_ONEXIT_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <stdbool.h>

DECL_BEGIN

#ifdef __CC__
struct task;

/* Callback executed during task exit, as registered by `task_queue_onexit()'
 * @param: reason: The reason why the callback is invoked (One of `ONEXIT_REASON_*') */
typedef /*ATTR_NOTHROW*/void (KCALL *task_onexit_t)(struct task *__restrict thread,
                                                    unsigned int reason, void *arg);
#define ONEXIT_REASON_TERMINATION  0x0000 /* The callback is executed because the thread is terminating (THIS_TASK == thread) */
#define ONEXIT_REASON_DESTRUCTION  0x0001 /* The callback is executed because the thread is deing destroyed (THIS_TASK != thread) */
#define ONEXIT_REASON_DRIVERCLOSE  0x0002 /* The callback is executed because the driver that was used to register the callback is being closed. */


/* Query a function to-be executed by `thread' during its cleanup phase.
 * NOTES:
 *   - If `thread' is never started and construction is aborted,
 *     callbacks are eiter executed when `task_failed()' is called,
 *     or when the task is destroyed (following `task_decref()')
 *   - onexit() callbacks are executed in the same order they were added.
 *   - onexit() callbacks are executed early on during the task cleanup
 *     phase: After `TASK_STATE_FTERMINATING' was set, and after `task_serve()'
 *     was called to serve any remaining RPC functions, yet prior to
 *     any callbacks defined using `DEFINE_PERTASK_CLEANUP()'
 *   - This function can be called multiple times with the same arguments,
 *     to have the same callback be registered more than once.
 *   - onexit() callbacks are _NOT_ inherited during a clone() system call.
 * @param: callback:        The callback that should be executed (must be a valid )
 * @return: false:         `thread' has already terminated, or is currently in
 *                          the process to terminating.
 *                          To prevent a race condition of new onexit() jobs
 *                          be scheduled once the thread is ready, or has
 *                          started executing already existing jobs, we simply
 *                          disallow the addition of new jobs, just as is the
 *                          case with `task_queue_rpc()'.
 * @return: true:           Successfully queued the given `callback' for execution.
 * @throw: E_BADALLOC:      Failed to allocate sufficient memory for the internal
 *                          on-exit control structures.
 * @throw: E_DRIVER_CLOSED: The calling driver has been closed. */
LOCAL bool KCALL
task_queue_onexit(struct task *__restrict thread,
                  task_onexit_t callback, void *arg);

/* Deque a previously queued onexit() callback.
 * NOTE: Unlike `task_queue_onexit()', this function can actually be called
 *       from another onexit() callback for the same thread, allowing that
 *       callback to prevent the execution of callbacks still left to-be
 *       executed. (onexit() callbacks are executed in the same order in
 *       which they were queued originally)
 * NOTE: If the given `callback-arg' pair been registered more than once,
 *       the most recent registration will be deleted.
 * @return: false: Either all instance of `callback(thread,arg)' have already
 *                 been executed, or not a single one was ever queued to begin
 *                 with.
 * @return: true:  Successfully dequeued the most recent onexit-entry matching
 *                 the given `callback' and `arg' parameters. */
LOCAL ATTR_NOTHROW bool KCALL
task_dequeue_onexit(struct task *__restrict thread,
                    task_onexit_t callback, void *arg);



#ifndef __INTELLISENSE__
struct driver;
#ifdef CONFIG_BUILDING_KERNEL_CORE
INTDEF struct driver this_driver;
#else
DATDEF struct driver this_driver;
#endif
FUNDEF bool KCALL
__os_task_queue_onexit(struct task *__restrict thread,
                       task_onexit_t callback, void *arg,
                       struct driver *__restrict owner)
                       ASMNAME("task_queue_onexit");
FUNDEF ATTR_NOTHROW bool KCALL
__os_task_dequeue_onexit(struct task *__restrict thread,
                         task_onexit_t callback, void *arg,
                         struct driver *__restrict owner)
                         ASMNAME("task_dequeue_onexit");


LOCAL bool KCALL
task_queue_onexit(struct task *__restrict thread,
                  task_onexit_t callback, void *arg) {
 return __os_task_queue_onexit(thread,callback,arg,&this_driver);
}
LOCAL ATTR_NOTHROW bool KCALL
task_dequeue_onexit(struct task *__restrict thread,
                    task_onexit_t callback, void *arg) {
 return __os_task_dequeue_onexit(thread,callback,arg,&this_driver);
}
#endif /* !__INTELLISENSE__ */


#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_ONEXIT_H */
