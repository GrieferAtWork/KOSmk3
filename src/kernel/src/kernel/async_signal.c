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
#ifndef GUARD_KERNEL_SRC_SCHED_ASYNC_SIGNAL_C
#define GUARD_KERNEL_SRC_SCHED_ASYNC_SIGNAL_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <kos/types.h>
#include <sched/task.h>
#include <sched/async_signal.h>
#include <kernel/debug.h>
#include <dev/wall.h>
#include <assert.h>
#include <except.h>

DECL_BEGIN

#if 1

/* [0..1][lock(THIS_TASK)] Chain of asynchronous connections active in the thread. */
PRIVATE ATTR_PERTASK struct async_task_connection *async_connections = NULL;

/* Connect to an async-safe signal.
 * WARNING: This function is _NOT_ async-safe. Only `async_sig_broadcast()' is!
 * @throw: E_WOULDBLOCK: Preemption has been disabled and a lock couldn't be acquired immediately. */
PUBLIC void KCALL
task_connect_async(struct async_task_connection *__restrict connection,
                   struct async_sig *__restrict signal) {
 connection->atc_delivered = ASYNC_SIG_STATUS_WAITING;
 connection->atc_thread    = THIS_TASK;
 connection->atc_signal    = signal;
 COMPILER_WRITE_BARRIER();
 if (ATOMIC_CMPXCH(signal->as_ptr,NULL,connection)) {
  /* Primary connection (finish initialization as such). */
  connection->atc_thrnext = PERTASK_XCH(async_connections,connection);
  COMPILER_WRITE_BARRIER();
 } else {
  /* Secondary connection (connect to the regular signal). */
  task_connect(&signal->as_sig);
 }
}


PRIVATE struct async_sig *KCALL
task_disconnect_async1(struct async_task_connection *__restrict connection) {
 struct async_sig *result = NULL;
 struct async_sig *signal = connection->atc_signal;
 if (ATOMIC_CMPXCH(signal->as_ptr,connection,NULL)) {
  /* Disconnected prematurely. */
 } else {
  /* Wait until the signal got delivered. */
  while (ATOMIC_READ(connection->atc_delivered) == ASYNC_SIG_STATUS_WAITING)
      task_yield();
  /* The signal was sent. */
  result = signal;
 }
 /* Always wake all the other threads.
  * If the signal got send, they must be notified as well.
  * If it wasn't, then this is just another sporadic wakeup. */
 sig_broadcast(&signal->as_sig);
 return result;
}


/* Disconnect all currently connected async-safe signals.
 * @throw: E_WOULDBLOCK: Preemption has been disabled and a lock couldn't be acquired immediately.
 * @return: * :          Randomly, one of the signals that were delivered.
 * @return: NULL:        No signals were sent. */
PUBLIC struct sig *KCALL task_disconnect_async(void) {
 struct async_sig *temp,*result = NULL;
 struct async_task_connection *chain;
 chain = PERTASK_XCH(async_connections,NULL);
 for (; chain; chain = chain->atc_thrnext) {
  temp = task_disconnect_async1(chain);
  if (!result) result = temp;
 }
 temp = (struct async_sig *)task_disconnect();
 if (!result) result = temp;
 return (struct sig *)result;
}


INTDEF ATTR_PERTASK struct task_connections my_connections;

/* Wait for async-signals to be boardcast and disconnect all remaining connections.
 * @throw: * :           This error was thrown by an RPC function call.
 * @throw: E_INTERRUPT:  The calling thread was interrupted.
 * @throw: E_WOULDBLOCK: Preemption has been disabled and a lock couldn't be acquired immediately.
 * @return: * :          Randomly, one of the signals that were delivered.
 * @return: NULL:        The given timeout has expired. */
PUBLIC struct sig *KCALL
task_waitfor_async(jtime_t abs_timeout) {
 struct task_connections *mycon;
 struct async_task_connection *chain;
 struct sig *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(result);
 struct sig *EXCEPT_VAR new_result;
 bool sleep_ok;
 mycon = &PERTASK(my_connections);
 TRY {
  /* We require that the caller have preemption enable. */
  if (!PREEMPTION_ENABLED())
      error_throw(E_WOULDBLOCK);
  for (;;) {
   /* Disable preemption to ensure that no task
    * for the current CPU could send the signal.
    * Additionally, no other CPU will be able to
    * interrupt us while we check for signals, meaning
    * that any task_wake IPIs will only be received once
    * `task_sleep()' gets around to re-enable interrupts. */
   PREEMPTION_DISABLE();
   COMPILER_READ_BARRIER();

   /* Check for synchronous signals. */
   result = mycon->tcs_sig;
   if (result) { PREEMPTION_ENABLE(); break; }

   /* Check for asynchronous signals. */
   chain = PERTASK_GET(async_connections);
   for (; chain; chain = chain->atc_thrnext) {
    if (ATOMIC_READ(chain->atc_delivered) != ASYNC_SIG_STATUS_WAITING) {
     result = (struct sig *)chain->atc_signal;
     PREEMPTION_ENABLE();
     goto got_signal;
    }
   }

   /* Serve RPC functions. */
   if (task_serve()) continue;

   /* Sleep for a bit, or until we're interrupted. */
   sleep_ok = task_sleep(abs_timeout);

   COMPILER_READ_BARRIER();
   result = mycon->tcs_sig;
   /* A signal was received in the mean time. */
   if (result) break;
   if (!sleep_ok) break; /* Timeout */
   /* Continue spinning */
  }
got_signal:;
 } FINALLY {
  /* Always disconnect all connected signals (synchronous + asynchronous). */
  new_result = (struct sig *)task_disconnect_async();
  if (!result) result = new_result;
 }
 return result;
}
PUBLIC ATTR_RETNONNULL struct sig *KCALL task_wait_async(void) {
 return task_waitfor_async(JTIME_INFINITE);
}


/* Wait for async-signals to be boardcast and disconnect all remaining connections.
 * @throw: * :           This error was thrown by an RPC function call.
 * @throw: E_INTERRUPT:  The calling thread was interrupted.
 * @throw: E_WOULDBLOCK: Preemption has been disabled and a lock couldn't be acquired immediately.
 * @return: * :          Randomly, one of the signals that were delivered.
 * @return: NULL:        The given timeout has expired. */
PUBLIC struct sig *KCALL
task_waitfor_async_noserve(jtime_t abs_timeout) {
 struct task_connections *mycon;
 struct async_task_connection *chain;
 struct sig *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(result);
 struct sig *EXCEPT_VAR new_result;
 bool sleep_ok;
 mycon = &PERTASK(my_connections);
 TRY {
  /* We require that the caller have preemption enable. */
  if (!PREEMPTION_ENABLED())
      error_throw(E_WOULDBLOCK);
  for (;;) {
   /* Disable preemption to ensure that no task
    * for the current CPU could send the signal.
    * Additionally, no other CPU will be able to
    * interrupt us while we check for signals, meaning
    * that any task_wake IPIs will only be received once
    * `task_sleep()' gets around to re-enable interrupts. */
   PREEMPTION_DISABLE();
   COMPILER_READ_BARRIER();

   /* Check for synchronous signals. */
   result = mycon->tcs_sig;
   if (result) { PREEMPTION_ENABLE(); break; }

   /* Check for asynchronous signals. */
   chain = PERTASK_GET(async_connections);
   for (; chain; chain = chain->atc_thrnext) {
    if (ATOMIC_READ(chain->atc_delivered) != ASYNC_SIG_STATUS_WAITING) {
     result = (struct sig *)chain->atc_signal;
     PREEMPTION_ENABLE();
     goto got_signal;
    }
   }

   /* Sleep for a bit, or until we're interrupted. */
   sleep_ok = task_sleep(abs_timeout);

   COMPILER_READ_BARRIER();
   result = mycon->tcs_sig;
   /* A signal was received in the mean time. */
   if (result) break;
   if (!sleep_ok) break; /* Timeout */
   /* Continue spinning */
  }
got_signal:;
 } FINALLY {
  /* Always disconnect all connected signals (synchronous + asynchronous). */
  new_result = (struct sig *)task_disconnect_async();
  if (!result) result = new_result;
 }
 return result;
}
PUBLIC ATTR_RETNONNULL struct sig *KCALL task_wait_async_noserve(void) {
 return task_waitfor_async_noserve(JTIME_INFINITE);
}

/* Wake a task waiting for the given async-signal.
 * NOTE: This function is ASYNC-SAFE, meaning it can safely be used from interrupt handlers.
 *       Also note that the requirement of an async-safe signal-broadcast function
 *       was the only reason why async-safe signals had to be implemented.
 *       Most notably is their use by keyboard input buffers,
 *       which are then filled by interrupt handlers.
 * @return: true:  A task was waiting for the signal. - That task may wake other waiters.
 * @return: false: No task was waiting for the signal. */
PUBLIC ASYNCSAFE bool KCALL
async_sig_broadcast(struct async_sig *__restrict signal) {
 struct async_task_connection *primary;
 primary = ATOMIC_XCH(signal->as_ptr,NULL);
 if (!primary) return false;
 task_wake(primary->atc_thread);
 ATOMIC_WRITE(primary->atc_delivered,ASYNC_SIG_STATUS_DELIVERED);
 return true;
}

/* Helper functions for user-space wait requests. */
PUBLIC struct sig *KCALL
task_uwaitfor_tmrel(USER CHECKED struct timespec const *rel_timeout) {
 return task_uwaitfor(jiffies + jiffies_from_timespec(*rel_timeout));
}
PUBLIC struct sig *KCALL
task_uwaitfor_tmabs(USER CHECKED struct timespec const *abs_timeout) {
 struct timespec diff = *abs_timeout;
 struct timespec wall = wall_gettime(&wall_kernel);
 if (TIMESPEC_LOWER(diff,wall))
     return task_udisconnect();
 TIMESPEC_SUB(diff,wall);
 return task_uwaitfor(jiffies + jiffies_from_timespec(diff));
}



#else

/* [0..1][lock(THIS_TASK)] Chain of asynchronous connections active in the thread. */
PRIVATE ATTR_PERTASK struct async_task_connection *async_connections = NULL;

DEFINE_PERTASK_CLEANUP(task_cleanup_async_connections);
INTERN void KCALL task_cleanup_async_connections(void) {
 /* Because we serve RPC calls before waiting for async signals,
  * we must make sure to disconnect all of them if such an RPC
  * call terminates the thread. */
 task_disconnect_async();
}

/* Connect to an async-safe signal.
 * WARNING: This function is _NOT_ async-safe. Only `async_sig_broadcast()' is! */
PUBLIC void KCALL
task_connect_async(struct async_task_connection *__restrict connection,
                   struct async_sig *__restrict signal) {
 /* Initialize the connection state. */
 connection->atc_thread     = THIS_TASK;
 connection->atc_sig        = signal;
 connection->atc_delivered  = ASYNC_SIG_STATUS_WAITING;
 connection->atc_thrnext    = PERTASK(async_connections);
 PERTASK(async_connections) = connection;
read_signal_pointer:
 connection->atc_secondary.cs_next = signal->as_ptr;
 COMPILER_READ_BARRIER();
 if (connection->atc_secondary.cs_next) {
  struct async_task_connection *next;
  /* Special state: The signal is currently being delivered. */
  if (connection->atc_secondary.cs_next == (struct async_task_connection *)-1) {
   task_yield();
   goto read_signal_pointer;
  }

  /* Other connections already exist.
   * Use the secondary-connections route and add ourself to that chain.
   * NOTE: By acquiring this lock, we ensure that the currently connect
   *       task is in a consistent state. */
  atomic_rwlock_write(&signal->as_lock);
  COMPILER_READ_BARRIER();
  connection->atc_secondary.cs_next = signal->as_ptr;
  /* Deal with the race condition of the previously connected task having disconnected. */
  if unlikely(!connection->atc_secondary.cs_next) {
   atomic_rwlock_endwrite(&signal->as_lock);
   goto set_primary_connection;
  }
  /* Walk the chain of secondary connections. */
  next = connection->atc_secondary.cs_next;
  while (next->atc_secondary.cs_next)
      next = next->atc_secondary.cs_next;
  /* Append the new connection at the end. */
  connection->atc_secondary.cs_next = NULL;
  connection->atc_secondary.cs_pself = &next->atc_secondary.cs_next;
  next->atc_secondary.cs_next = connection;
  /* Release the signal, allowing the connection to be broadcast. */
  atomic_rwlock_endwrite(&signal->as_lock);

 } else {
set_primary_connection:
  connection->atc_secondary.cs_pself = NULL;
  /* Atomically connect to the signal. */
  if (!ATOMIC_CMPXCH(signal->as_ptr,NULL,connection))
       goto read_signal_pointer;
 }
}


/* Wake a task waiting for the given async-signal.
 * @return: true:  A task was waiting for the signal. - That task may wake other waiters.
 * @return: false: No task was waiting for the signal. */
PUBLIC ASYNCSAFE bool KCALL
async_sig_broadcast(struct async_sig *__restrict signal) {
 struct async_task_connection *target;
 struct task *target_task;
 debug_printf("BROADCAST... ");
 target = ATOMIC_XCH(signal->as_ptr,(struct async_task_connection *)-1);
 if (!target)
     goto no_target; /* No waiters. */
 if (target == (struct async_task_connection *)-1)
     goto no_target; /* No waiters. */
 /* NOTE: The race condition between extracting the target and waking
  *       it is solved by `task_disconnect1_async()' checking if it
  *       is matching the signal's primary connection, and waiting
  *       for `target->atc_thread' to become `NULL', as we do below. */

 /* Wake the task.
  * NOTE: This may fail, but even if it does, we can still be sure
  *       that the thread will receive the signal before it just
  *       hasn't started sleeping, yet. */
 target_task = target->atc_thread;
 /* Indicate to the thread that it is being signaled. */
 ATOMIC_WRITE(target->atc_delivered,ASYNC_SIG_STATUS_DELIVERING);
 /* With the target's state set, we can ~unlock~ (in a sense) the signal. */
 ATOMIC_WRITE(signal->as_ptr,NULL);
 /* NOTE: Having set `ASYNC_SIG_STATUS_DELIVERING' guaranties that the
  *       thread will wait for async signal delivery to complete. */
 task_wake(target_task);
 ATOMIC_WRITE(target->atc_delivered,ASYNC_SIG_STATUS_DELIVERED);
 debug_printf("Woken\n");
 return true;
no_target:
 debug_printf("No target\n");
 ATOMIC_WRITE(signal->as_ptr,NULL);
 return false;
}

/* Disconnect the given async-task connection. */
PRIVATE struct async_sig *KCALL
task_disconnect1_async(struct async_task_connection *__restrict connection) {
 struct async_task_connection *next;
 struct async_task_connection *oldcon;
 struct async_sig *sig;
 struct task *next_thread;
 uintptr_t state;
 sig = connection->atc_sig;
again:
 atomic_rwlock_write(&sig->as_lock);
 next = connection->atc_secondary.cs_next;
 state = ATOMIC_READ(connection->atc_delivered);
 if (state != ASYNC_SIG_STATUS_WAITING) {
  /* Connection has already been delivered / is being delivered. */
  if (state == ASYNC_SIG_STATUS_DELIVERING)
      goto unlock_yield_and_try_again; /* Wait for delivery to complete if it's happening right now. */
 } else if ((oldcon = ATOMIC_CMPXCH_VAL(sig->as_ptr,connection,next)) == connection) {
  /* Deleted the primary connection before it could be sent. */
  debug_printf("next = %p\n",next);
  atomic_rwlock_endwrite(&sig->as_lock);
  return NULL;
 } else if (oldcon == (struct async_task_connection *)-1) {
  /* Special case: The signal is currently being delivered. */
unlock_yield_and_try_again:
  atomic_rwlock_endwrite(&sig->as_lock);
  task_yield();
  goto again;
#if 1 /* Optimization when the signal was delivered with no new waiters having appeared since. */
 } else if (!oldcon) {
  /* No existing connections. --> The signal had to be sent at some point. */
#endif
 } else {
  /* We know we're not the primary connection.
   * If our signal got sent at some point, but some other thread was
   * notified, we'll be getting notified by it once the time comes. */
  atomic_rwlock_endwrite(&sig->as_lock);
  return NULL;
 }
 if (!next) {
  /* No secondary connections (remaining) */
  atomic_rwlock_endwrite(&sig->as_lock);
  return sig;
 }
 /* Wake the next thread in the chain of waiters. */
 next_thread = next->atc_thread;
 task_incref(next_thread);
 if (next->atc_delivered == ASYNC_SIG_STATUS_WAITING) {
  /* Indicate to all waiting tasks that the signal was delivered. */
  for (; next; next = next->atc_secondary.cs_next)
      ATOMIC_WRITE(next->atc_delivered,ASYNC_SIG_STATUS_DELIVERED);
 }
 atomic_rwlock_endwrite(&sig->as_lock);
 assertef(task_wake(next_thread),"Thread with active async-signals has terminated");
 task_decref(next_thread);
 return sig;
}

/* Disconnect all currently connected async-safe signals.
 * @return: * :   Randomly, one of the signals that were delivered.
 * @return: NULL: No signals were sent. */
PUBLIC struct async_sig *KCALL
task_disconnect_async(void) {
 struct async_sig *temp,*result = NULL;
 struct async_task_connection *con;
 con = XCH(PERTASK(async_connections),NULL);
 for (; con; con = con->atc_thrnext) {
  temp = task_disconnect1_async(con);
  if (!result) result = temp;
 }
 return result;
}

/* Wait for async-signals to be boardcast.
 * @return: * :   The signal that was delivered.
 * @return: NULL: The given timeout has expired. */
PUBLIC struct async_sig *KCALL
task_waitfor_async(jtime_t abs_timeout) {
 struct async_sig *EXCEPT_VAR result;
 struct async_task_connection *chain;
 chain = PERTASK(async_connections);
 TRY {
  /* We require that the caller have preemption enable. */
  if (!PREEMPTION_ENABLED())
      error_throw(E_WOULDBLOCK);
check_delivery:
  /* Disable preemption, so we can safely check
   * if any async-signal has already been delivered. */
  PREEMPTION_DISABLE();
  COMPILER_BARRIER();
  for (; chain; chain = chain->atc_thrnext) {
   if (ATOMIC_READ(chain->atc_delivered) != ASYNC_SIG_STATUS_WAITING) {
    result = chain->atc_sig;
    PREEMPTION_ENABLE();
    task_disconnect_async();
    return result;
   }
  }
  /* Serve RPC function calls before starting to wait. */
  if (task_serve())
      goto check_delivery;
  /* Wait for signals to be delivered */
  if (task_sleep(abs_timeout))
      goto check_delivery;
  assert(abs_timeout != JTIME_INFINITE);
 } FINALLY {
  /* Disconnect all signals. */
  result = task_disconnect_async();
 }
 return result;
}
PUBLIC ATTR_RETNONNULL struct async_sig *
KCALL task_wait_async(void) {
 return task_waitfor_async(JTIME_INFINITE);
}




/* Wait for any kind of signal and disconnect all before returning.
 * @return: * :   A pointer to a `struct async_sig' or `struct sig' that was sent.
 * @return: NULL: The given `abs_timeout' has expired. */
PUBLIC ATTR_RETNONNULL void *KCALL task_uwait(void) {
 return task_uwaitfor(JTIME_INFINITE);
}

INTDEF ATTR_PERTASK struct task_connections my_connections;
PUBLIC void *KCALL task_uwaitfor(jtime_t abs_timeout) {
 struct task_connections *mycon;
 struct async_task_connection *chain;
 void *EXCEPT_VAR result;
 void *EXCEPT_VAR new_result;
 bool sleep_ok;
 mycon = &PERTASK(my_connections);
 TRY {
  /* We require that the caller have preemption enable. */
  if (!PREEMPTION_ENABLED())
      error_throw(E_WOULDBLOCK);
  for (;;) {
   /* Disable preemption to ensure that no task
    * for the current CPU could send the signal.
    * Additionally, no other CPU will be able to
    * interrupt us while we check for signals, meaning
    * that any task_wake IPIs will only be received once
    * `task_sleep()' gets around to re-enable interrupts. */
   PREEMPTION_DISABLE();
   COMPILER_READ_BARRIER();

   /* Check for synchronous signals. */
   result = mycon->tcs_sig;
   if (result) { PREEMPTION_ENABLE(); break; }

   /* Check for asynchronous signals. */
   chain = PERTASK(async_connections);
   for (; chain; chain = chain->atc_thrnext) {
    if (ATOMIC_READ(chain->atc_delivered) != ASYNC_SIG_STATUS_WAITING) {
     result = chain->atc_sig;
     PREEMPTION_ENABLE();
     goto got_signal;
    }
   }

   /* Serve RPC functions. */
   if (task_serve()) continue;

   /* Sleep for a bit, or until we're interrupted. */
   sleep_ok = task_sleep(abs_timeout);

   COMPILER_READ_BARRIER();
   result = mycon->tcs_sig;
   /* A signal was received in the mean time. */
   if (result) break;
   if (!sleep_ok) break; /* Timeout */
   /* Continue spinning */
  }
got_signal:;
 } FINALLY {
  /* Always disconnect all connected signals (synchronous + asynchronous). */
  new_result = task_disconnect_async();
  if (!result) result = new_result;
  new_result = task_disconnect();
  if (!result) result = new_result;
 }
 return result;
}

/* Disconnect any kind of signal.
 * @return: * :   A pointer to a `struct async_sig' or `struct sig' that was sent.
 * @return: NULL: No signals were sent, or have been connected. */
PUBLIC void *KCALL task_udisconnect(void) {
 void *result,*new_result;
 /* Disconnect asynchronous signals, as well as regular signals. */
 result     = task_disconnect_async();
 new_result = task_disconnect();
 if (!result) result = new_result;
 return result;
}

PUBLIC void *KCALL
task_uwaitfor_tmrel(USER CHECKED struct timespec const *rel_timeout) {
 return task_uwaitfor(jiffies + jiffies_from_timespec(*rel_timeout));
}
PUBLIC void *KCALL
task_uwaitfor_tmabs(USER CHECKED struct timespec const *abs_timeout) {
 struct timespec diff = *abs_timeout;
 struct timespec wall = wall_gettime(&wall_kernel);
 if (TIMESPEC_LOWER(diff,wall))
     return task_udisconnect();
 TIMESPEC_SUB(diff,wall);
 return task_uwaitfor(jiffies + jiffies_from_timespec(diff));
}

#endif

DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_ASYNC_SIGNAL_C */
