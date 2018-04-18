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
#ifndef GUARD_KERNEL_SRC_SCHED_SUSPEND_C
#define GUARD_KERNEL_SRC_SCHED_SUSPEND_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <sched/suspend.h>
#include <sched/task.h>
#include <stdbool.h>
#include <assert.h>
#include <except.h>

DECL_BEGIN


PRIVATE void KCALL
task_suspend_userspace(void *arg,
                       struct cpu_hostcontext_user *__restrict UNUSED(context),
                       unsigned int UNUSED(mode)) {
 /* TODO: sizeof(jtime_t) > sizeof(void *) */
 task_suspend((jtime_t)(uintptr_t)arg);
}

/* Suspend the calling thread.
 * @return: true:  Some other thread called `task_resume()'
 *                 to resume the calling thread.
 * @return: false: The given `abs_timeout' has expired. */
PUBLIC bool KCALL task_suspend(jtime_t abs_timeout) {
 u16 EXCEPT_VAR old_flags;
 assert(PREEMPTION_ENABLED());
 if (ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FSUSPENDED) & TASK_STATE_FSUSPENDED) {
  /* Invoked from an RPC function */
  assertf(THIS_TASK->t_state & TASK_STATE_FINRPC,
          "The only way that the SUSPENDED flag could already be set "
          "is is an RPC function was invoked by the `task_serve()' below, "
          "which then tried to call this function again.\n"
          "That means that the thread _must_ be execution RPC functions right now!");
  return true;
 }
 /* Enable recursive RPC handling, allowing other RPC functions to be called from here.
  * If we didn't do this, we'd be stuck because other threads would be unable
  * to continue this one if they couldn't schedule new RPC callbacks. */
 old_flags = ATOMIC_FETCHOR(THIS_TASK->t_flags,TASK_FRPCRECURSION);
 TRY {
  COMPILER_BARRIER();
  for (;;) {
   PREEMPTION_DISABLE();
   if (!PERTASK_TESTF(this_task.t_state,TASK_STATE_FSUSPENDED)) {
    PREEMPTION_ENABLE();
    break;
   }
   COMPILER_READ_BARRIER();
   /* Serve RPC functions before sleeping. */
   if (task_serve()) continue;
   /* Wait until we're no longer suspended. */
   if (!task_sleep(abs_timeout))
        return false;
   if (!PERTASK_TESTF(this_task.t_state,TASK_STATE_FSUSPENDED))
        break;
   COMPILER_READ_BARRIER();
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  u16 state;
  /* Unset the RPC recursion flag if it was set before. */
  if (!(old_flags & TASK_FRPCRECURSION))
        ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_FRPCRECURSION);
  /* Unset the SUSPENDED flag, or re-schedule the suspend
   * command for execution prior to returning to user-space.
   * This can happen when a posix_signal raises an E_INTERRUPT
   * exception a the thread that has been suspended. */
  state = ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FSUSPENDED);
  if ((state & TASK_STATE_FSUSPENDED) &&
       error_code() == E_INTERRUPT) {
   /* Re-schedule the suspension request to resume once we're back to user-space. */
   task_queue_rpc_user(THIS_TASK,&task_suspend_userspace,
                      (void *)(uintptr_t)abs_timeout,
                       TASK_RPC_NORMAL|TASK_RPC_USER);
   /* TODO: sizeof(jtime_t) > sizeof(void *) */
  }
  /* Propagate all errors. */
  error_rethrow();
 }
 if (!(old_flags & TASK_FRPCRECURSION))
       ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_FRPCRECURSION);
 return true;
}

/* Resume execution within the given thread.
 * @return: true:  Successfully resumed execution in `thread'.
 * @return: false: The given `thread' has terminated.
 * @return: false: The given `thread' wasn't suspended. */
PUBLIC bool KCALL task_resume(struct task *__restrict thread) {
 /* Unset the suspended-bit. */
 if (!(ATOMIC_FETCHAND(thread->t_state,~TASK_STATE_FSUSPENDED) & TASK_STATE_FSUSPENDED))
       return false;
 /* Wake the thread. */
 return task_wake(thread);
}

DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_SUSPEND_C */
