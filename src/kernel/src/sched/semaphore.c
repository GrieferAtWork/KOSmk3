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
#ifndef GUARD_KERNEL_SRC_SCHED_SEMAPHORE_C
#define GUARD_KERNEL_SRC_SCHED_SEMAPHORE_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/atomic.h>
#include <sched/task.h>
#include <sched/signal.h>
#include <sched/semaphore.h>
#include <stdbool.h>
#include <assert.h>

DECL_BEGIN

/* Try to acquire, or wait until a ticket
 * has been made available, then acquire it.
 * @return: true:  A ticket has been acquired.
 * @return: false: Failed to acquire a ticket immediately,
 *                 or after `abs_timeout' elapsed. */
PUBLIC bool KCALL
__os_sem_timedwait(sem_t *__restrict self, jtime_t abs_timeout) {
again:
 assertf(!task_isconnected(),
         "You must not already be connected when calling this function");
 /* Quick check if tickets are available without
  * locking the signal at all (pure atomicity). */
 if likely(sem_trywait(self)) {
ok:
  return true;
 }
#if 0 /* No point. - Let `task_waitfor()' handle this case... */
 if (abs_timeout == JTIME_DONTWAIT)
     return false;
#endif
 TASK_POLL_BEFORE_CONNECT({
  if (sem_trywait(self))
      goto ok;
 });
 /* Connect to the signal. */
 task_connect(&self->sem_signal);
 /* Try the read one more time now that we're connected to the signal. */
 if unlikely(sem_trywait(self)) {
  task_disconnect();
  goto ok;
 }
 if (!task_waitfor(abs_timeout))
      return false; /* Timeout */
 /* Loop back and try to acquire a ticket now that we've been signaled. */
 goto again;
}


/* Make `num_tickets' available and wake up to that same number of threads. */
PUBLIC ATTR_NOTHROW void KCALL
sem_post(sem_t *__restrict self, semcount_t num_tickets) {
 ATOMIC_FETCHADD(self->sem_tickets,num_tickets);
 sig_send(&self->sem_signal,(size_t)num_tickets);
}

DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_SEMAPHORE_C */
