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
#ifndef GUARD_KERNEL_SRC_SCHED_MUTEX_C
#define GUARD_KERNEL_SRC_SCHED_MUTEX_C 1
#define _KOS_SOURCE 1
#define _NOSERVE_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/atomic.h>
#include <kernel/sections.h>
#include <kernel/vm.h>
#include <sched/task.h>
#include <sched/mutex.h>
#include <except.h>
#include <assert.h>
#include <kernel/debug.h>
#include <unwind/eh_frame.h>

DECL_BEGIN

PUBLIC bool KCALL
__os_mutex_get_timed(struct mutex *__restrict self,
                     jtime_t abs_timeout) {
 assertf(!task_isconnected(),
         "You mustn't be connected when calling this function (%p) -> %p",
         self,__builtin_return_address(0));
again:
 if (ATOMIC_CMPXCH(self->m_ind,0,1)) {
ok:
  self->m_owner = THIS_TASK;
  return true;
 }
 if (self->m_owner == THIS_TASK) {
  /* Recursion (only when the indirection counter is non-ZERO). */
  ++self->m_ind;
  return true;
 }

 TASK_POLL_BEFORE_CONNECT({
  if (ATOMIC_CMPXCH(self->m_ind,0,1))
      goto ok;
 });

 /* Connect to the mutex signal. */
 task_connect(&self->m_signal);
 /* Prevent a race condition by checking the indirection again. */
 if unlikely(ATOMIC_CMPXCH(self->m_ind,0,1)) {
  task_disconnect();
  goto ok;
 }

 /* Wait for the mutex to be signaled. */
 if (task_waitfor(abs_timeout))
     goto again;
 /* Timeout has expired. */
 return false;
}

PUBLIC ATTR_NOTHROW void KCALL
__os_mutex_put(struct mutex *__restrict self) {
 assertf(self->m_ind >= 1,"Noone is holding any locks");
 assertf(THIS_TASK == self->m_owner,"You're not the owner (%p != %p)",
         THIS_TASK,self->m_owner);
 /* Signal the first waiting task that the mutex is now free. */
 if (self->m_ind == 1) {
  self->m_owner = NULL;
  self->m_ind   = 0;
  COMPILER_WRITE_BARRIER();
  sig_send(&self->m_signal,1);
 } else {
  --self->m_ind;
 }
}

PUBLIC bool KCALL
mutex_poll(struct mutex *__restrict self) {
 /* Use ghost connections to prevent the deadlock
  * scenario described by `task_connect_ghost()' */
 task_connect_ghost(&self->m_signal);
 return self->m_ind == 0 || self->m_owner == THIS_TASK;
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_MUTEX_C */
