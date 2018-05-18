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
#ifndef GUARD_KERNEL_SRC_SCHED_RPC_C
#define GUARD_KERNEL_SRC_SCHED_RPC_C 1
#define _KOS_SOURCE 1
#define _NOSERVE_SOURCE 1 /* So we can implement `TASK_STATE_FDONTSERVE' */

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <kos/types.h>
#include <kos/rpc.h>
#include <kernel/debug.h>
#include <kernel/sections.h>
#include <kernel/malloc.h>
#include <kernel/bind.h>
#include <sched/task.h>
#include <sched/pertask-arith.h>
#include <sched/signal.h>
#include <sched/stat.h>
#include <string.h>
#include <except.h>
#include <kos/safecall.h>

#include "rpc.h"

DECL_BEGIN

INTERN ATTR_PERTASK struct rpc_info my_rpc = {
    .ri_cnt = 0,
    .ri_siz = CONFIG_STATIC_RPC_SLOTS
};


#if CONFIG_STATIC_RPC_SLOTS != 0
DEFINE_PERTASK_INIT(rpc_init);
INTERN void KCALL rpc_init(struct task *__restrict thread) {
 struct rpc_info *info = &FORTASK(thread,my_rpc);
 info->ri_vec = info->ri_sbuf;
}
#endif

DEFINE_PERTASK_FINI(rpc_fini);
INTERN void KCALL rpc_fini(struct task *__restrict thread) {
 struct rpc_info *info = &FORTASK(thread,my_rpc);
#if CONFIG_STATIC_RPC_SLOTS != 0
 if (info->ri_vec == info->ri_sbuf)
     return;
#endif
 kfree(info->ri_vec);
}


/* Allocate a new RPC slot and set the `TASK_STATE_FINTERRUPTING' bit.
 * Returns NULL and clear the `TASK_STATE_FINTERRUPTING'
 * bit if the `TASK_STATE_FTERMINATING' bit has been set. */
INTERN struct rpc_slot *KCALL
rpc_alloc(struct task *__restrict thread) {
 struct rpc_info *info;
 u16 state;
again:
 while ((state = ATOMIC_FETCHOR(thread->t_state,TASK_STATE_FINTERRUPTING),
        (state&TASK_STATE_FINTERRUPTING))) {
  if (thread != THIS_TASK)
      task_serve();
  task_yield();
 }
 /* Check if the thread is currently terminating, or has terminated. */
 if (state & (TASK_STATE_FTERMINATING|TASK_STATE_FTERMINATED)) {
  ATOMIC_FETCHAND(thread->t_state,~TASK_STATE_FINTERRUPTING);
  return NULL;
 }
 info = &FORTASK(thread,my_rpc);
 assert(info->ri_cnt <= info->ri_siz);
 if unlikely(info->ri_cnt == info->ri_siz) {
  size_t EXCEPT_VAR new_alloc;
  struct rpc_slot *old_vector;
  struct rpc_slot *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(new_vector);
  /* Must allocate more memory for RPC functions. */
  new_alloc = info->ri_siz * 2;
#if CONFIG_STATIC_RPC_SLOTS == 0
  if unlikely(!new_alloc)
     new_alloc = 4;
#else
  assert(new_alloc != 0);
#endif
  ATOMIC_FETCHAND(thread->t_state,~TASK_STATE_FINTERRUPTING);
  /* Allocate a new RPC vector. */
  TRY {
   new_vector = (struct rpc_slot *)kmalloc(new_alloc*sizeof(struct rpc_slot),
                                           GFP_SHARED);
  } CATCH_HANDLED (E_BADALLOC) {
   /* Try half the size */
   new_alloc = (new_alloc/2)+1;
   new_vector = (struct rpc_slot *)kmalloc(new_alloc*sizeof(struct rpc_slot),
                                           GFP_SHARED);
  }
  COMPILER_BARRIER();
  /* Try-acquire a lock on the interrupting-bit. */
  while (ATOMIC_FETCHOR(thread->t_state,TASK_STATE_FINTERRUPTING) &
                                        TASK_STATE_FINTERRUPTING) {
   TRY {
    if (thread != THIS_TASK)
        task_serve();
    task_yield();
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    /* Free our new vector on error. */
    kfree(new_vector);
    error_rethrow();
   }
  }
  COMPILER_BARRIER();
  assert(info->ri_siz >= info->ri_cnt);
  if (new_alloc > info->ri_siz) {
   old_vector = info->ri_vec;
   /* Copy data into the new vector. */
   memcpy(new_vector,old_vector,
          info->ri_cnt*sizeof(struct rpc_slot));
   /* Replace the new vector to increase its size. */
   info->ri_siz = new_alloc;
   info->ri_vec = new_vector;
#if CONFIG_STATIC_RPC_SLOTS != 0
   /* Prevent the kfree() below from attempting to free the
    * static buffer used by this thread (kfree(NULL) is a no-op). */
   if (old_vector == info->ri_sbuf)
       old_vector = NULL;
#endif
  } else {
   /* Another thread allocate more in the mean time. Use their handywork. */
   old_vector = new_vector;
  }
  /* Release the interrupting-lock, so we're not
   * holding it while freeing the old RPC vector. */
  ATOMIC_FETCHAND(thread->t_state,~TASK_STATE_FINTERRUPTING);
  /* Now that there's more memory, free the old vector and start over. */
  kfree(old_vector);
  goto again;
 }
 /* Reserve an additional slot and return a pointer to it. */
 return &info->ri_vec[info->ri_cnt++];
}


/* Serve RPC callbacks prior to switching to user-space.
 * Called before user-space exception propagation to serve RPC
 * functions that should be executed before returning to user-space.
 * NOTE: The regular `task_serve()' throws an `E_INTERRUPT' error in
 *       order to interrupt the current system call when RPC functions
 *       have been registered that should be executed prior to jumping
 *       back to user-space.
 * @param: context: The user-space CPU context as it will be restored during the switch.
 * @param: mode:    One of `TASK_USERCTX_F*' (see above) */
INTERN void FCALL
task_serve_before_user(struct cpu_hostcontext_user *__restrict context,
                       unsigned int mode) {
 if (PERTASK_TESTF(this_task.t_state,TASK_STATE_FINTERRUPTED)) {
  /* Serve RPC function calls. */
  struct rpc_slot EXCEPT_VAR slot;
  struct rpc_slot *vec;
  size_t i,count;
again:
  while (ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FINTERRUPTING) &
                                           TASK_STATE_FINTERRUPTING)
         task_yield();
  if (!PERTASK_TEST(my_rpc.ri_cnt)) {
   /* No more RPCs (delete the interrupted-flag) */
   ATOMIC_FETCHAND(THIS_TASK->t_state,
                 ~(TASK_STATE_FINTERRUPTING|
                   TASK_STATE_FINTERRUPTED));
   return;
  }
  vec = PERTASK_GET(my_rpc.ri_vec);
  if (mode & TASK_USERCTX_FTIMER) {
   /* When run from a timed preemption interrupt, don't serve RPCs
    * that have been created using the `RPC_SLOT_FUSYNC' flag. */
   i = 0;
   count = PERTASK_GET(my_rpc.ri_cnt);
   for (;;) {
    assert(i < count);
    if (!(vec[i].rs_flag & RPC_SLOT_FUSYNC))
          break; /* This one can be served. */
next_rpc:
    ++i;
    if (i >= count) {
     /* All remaining RPC callbacks have the `RPC_SLOT_FUSYNC' flag set. */
     ATOMIC_FETCHAND(THIS_TASK->t_state,
                   ~(TASK_STATE_FINTERRUPTING));
     return;
    }
   }
   /* Take the RPC at index `i' */
   if ((vec[0].rs_flag & RPC_SLOT_FUASYNC) &&
        PERTASK_TESTF(this_task.t_flags,TASK_FNOSIGNALS))
        goto next_rpc;
   PERTASK_DEC(my_rpc.ri_cnt);
   memcpy((void *)&slot,&vec[i],sizeof(struct rpc_slot));
   memmove(&vec[i],&vec[i+1],
           (PERTASK_GET(my_rpc.ri_cnt)-i)*sizeof(struct rpc_slot));
  } else {
   /* Just take away the first RPC */
   if ((vec[0].rs_flag & RPC_SLOT_FUASYNC) &&
        PERTASK_TESTF(this_task.t_flags,TASK_FNOSIGNALS)) {
    /* Mustn't handle this RPC right now! */
    i     = 1;
    count = PERTASK_GET(my_rpc.ri_cnt);
    goto next_rpc;
   }
   PERTASK_DEC(my_rpc.ri_cnt);
   memcpy((void *)&slot,&vec[0],sizeof(struct rpc_slot));
   memmove(&vec[0],&vec[1],
            PERTASK_GET(my_rpc.ri_cnt)*sizeof(struct rpc_slot));
  }
  ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FINTERRUPTING);
  TRY {
   /* Execute the RPC. */
   INCSTAT(ts_xrpc);
   if (slot.rs_flag & RPC_SLOT_FUSER) {
    SAFECALL_KCALL_VOID_3((task_user_rpc_t)slot.rs_fun,slot.rs_arg,context,mode);
   } else {
    SAFECALL_KCALL_VOID_1(slot.rs_fun,slot.rs_arg);
   }
  } FINALLY {
   /* Signal execution completion. */
   if (slot.rs_done)
       sig_broadcast(slot.rs_done);
  }
  goto again;
 }
}

/* Check for pending signals and serve RPC functions yet to be executed.
 * NOTE: If the task is currently running in user-space, this function
 *       is also called during interrupt-driven preemption, though the
 *       main idea is for tasks to call this function before entering
 *       a sleep-state, as is automatically done when `task_sleep()'
 *       is called.
 * NOTE: When the `t_nothrow_serve' counter of the calling thread is
 *       non-ZERO(0), this function doesn't propagate any exceptions,
 *       but instead delays them until the counter is decremented.
 * NOTE: In the event that RPC function calls are served, this function
 *       will automatically save the calling thread's signal connection
 *       set, meaning that the caller is not required to save it, nor are
 *       RPC functions that may use signal-based connections required to.
 * @throw: E_INTERRUPT:   A user-space signal is pending delivery.
 * @throw: E_EXIT_THREAD: A scheduled RPC function threw an `E_EXIT_THREAD' error.
 * @throw: * :            A scheduled RPC function threw an error.
 * @return: true:  At least one RPC function was served.
 * @return: false: No RPC functions were served. */
PUBLIC bool KCALL task_serve(void) {
 bool result = false;
 u16 EXCEPT_VAR state = PERTASK_GET(this_task.t_state);
 if (state & TASK_STATE_FINTERRUPTED) {
  /* Serve RPC function calls. */
  struct rpc_slot EXCEPT_VAR slot;
  struct rpc_slot *vec;
  struct task_connections connections;
#ifndef NDEBUG
  u32 old_nothrow_serve_recursion;
  u16 old_flags = PERTASK_GET(this_task.t_flags);
#endif
  /* If we're still supposed to serve an RPC exception once
   * the nothrow counter hits ZERO(0), we must not execute
   * additional RPC functions until that has been accomplished. */
  if (state & (TASK_STATE_FSERVEEXCEPT|TASK_STATE_FDONTSERVE))
      return false;
  /* When already inside an RPC callback, only serve
   * interrupts when that callback set the recursion flag. */
  if ((state & TASK_STATE_FINRPC) &&
      !PERTASK_TESTF(this_task.t_flags,TASK_FRPCRECURSION))
       return false;
  /* Enable preemption while serving RPC functions. */
  PREEMPTION_ENABLE();
  /* Save connections of the caller (as they're likely waiting for
   * some kind of signal). Otherwise, RPC function calls could be
   * able to clobber the connection set. */
  task_push_connections(&connections);
continue_serving:
  while (ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FINTERRUPTING) &
                                           TASK_STATE_FINTERRUPTING)
         task_yield();
  if (!PERTASK_TEST(my_rpc.ri_cnt)) {
   /* No more RPCs (delete the interrupted-flag) */
done_serving_2:
   ATOMIC_FETCHAND(THIS_TASK->t_state,
                 ~(TASK_STATE_FINTERRUPTING|TASK_STATE_FINTERRUPTED|
                  (state & TASK_STATE_FINRPC ? 0 : TASK_STATE_FINRPC)));
done_serving:
   task_pop_connections(&connections);
   return result;
  }
  vec = PERTASK_GET(my_rpc.ri_vec);
  if (vec[0].rs_flag & RPC_SLOT_FUSER) {
   assert(!PERTASK_TESTF(this_task.t_flags,TASK_FKERNELJOB));
   if ((vec[0].rs_flag & RPC_SLOT_FUASYNC) &&
        PERTASK_TESTF(this_task.t_flags,TASK_FNOSIGNALS)) {
    /* Don't serve async user RPCs right now. */
    size_t i = 1;
    size_t count = PERTASK_GET(my_rpc.ri_cnt);
    for (; i < count; ++i) {
     if (vec[i].rs_flag & RPC_SLOT_FUASYNC)
         continue;
     PERTASK_DEC(my_rpc.ri_cnt);
     memcpy((void *)&slot,&vec[i],sizeof(struct rpc_slot));
     memmove(&vec[i],&vec[i+1],
            (PERTASK_GET(my_rpc.ri_cnt)-i)*sizeof(struct rpc_slot));
     goto do_exec_rpc;
    }
    ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FINTERRUPTING);
    /* All remaining RPCs are served later! */
    goto done_serving_2;
   }
   /* Remaining RPC must be called before returning to user-space. */
   ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FINTERRUPTING);
   if (PERTASK_TEST(this_task.t_nothrow_serve)) {
    error_info()->e_error.e_code = E_INTERRUPT;
    error_info()->e_error.e_flag = ERR_FNORMAL;
    ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FSERVEEXCEPT);
    goto done_serving;
   }
   /* Throw an interrupt error, so we will return to user-space more quickly. */
   task_pop_connections(&connections);
   task_disconnect();
   error_throw(E_INTERRUPT);
  }
  /* Take away the first RPC */
  PERTASK_DEC(my_rpc.ri_cnt);
  memcpy((void *)&slot,&vec[0],sizeof(struct rpc_slot));
  memmove(&vec[0],&vec[1],
           PERTASK_GET(my_rpc.ri_cnt)*sizeof(struct rpc_slot));
do_exec_rpc:
  ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FINTERRUPTING);
#ifndef NDEBUG
  old_nothrow_serve_recursion = PERTASK_GET(this_task.t_nothrow_serve);
#endif
  /* Indicate that we're now executing RPC functions. */
  ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FINRPC);
  {
   struct task_connections *EXCEPT_VAR pconnections = &connections;
   TRY {
    /* Execute the RPC. */
#if 0
    debug_printf("%[vinfo:%f(%l,%c) : %n : %p] : RPC at %p, with %p in %p\n",
                 slot.rs_fun,slot.rs_arg,THIS_TASK);
#endif
    INCSTAT(ts_xrpc);
    (*slot.rs_fun)(slot.rs_arg);
   } FINALLY {
    assertf(PERTASK_TESTF(this_task.t_state,TASK_STATE_FINRPC),
            "%[vinfo:%f(%l,%c) : %n : %p : RPC callback deleted the `TASK_STATE_FINRPC' flag\n]",
            slot.rs_fun);
#ifndef NDEBUG
    assertf(PERTASK_TESTF(this_task.t_flags,TASK_FRPCRECURSION) ==
            (old_flags & TASK_FRPCRECURSION),
            "%[vinfo:%f(%l,%c) : %n : %p : RPC callback did not clean up `TASK_FRPCRECURSION'\n]",
            slot.rs_fun);
    /* Assert that the `nothrow_serve()' recursion was restored. */
    assertf(old_nothrow_serve_recursion == PERTASK_GET(this_task.t_nothrow_serve) ||
            FINALLY_WILL_RETHROW,
            "%[vinfo:%f(%l,%c) : %n] : RPC function call at %p with %p did not restore "
            "nothrow_serve recursion (expected %I32u, but got %I32u)",
            slot.rs_fun,slot.rs_fun,slot.rs_arg,
            old_nothrow_serve_recursion,
            PERTASK_GET(this_task.t_nothrow_serve));
#endif
    /* Unset the in-rpc bit if we're not here recursively. */
    if (!(state & TASK_STATE_FINRPC))
          ATOMIC_FETCHAND(THIS_TASK->t_state,~(TASK_STATE_FINRPC));
    /* Signal execution completion. */
    if (slot.rs_done)
        sig_broadcast(slot.rs_done);
    if (FINALLY_WILL_RETHROW) {
     if (PERTASK_TEST(this_task.t_nothrow_serve)) {
      /*  Set the exception-serve flag and don't continue
       *  running more RPC functions until this exception
       *  will have been dealt with once the caller invokes
       * `task_nothrow_end()' a sufficient number of times. */
      ATOMIC_FETCHOR(THIS_TASK->t_state,
                     TASK_STATE_FSERVEEXCEPT);
      goto done_serving;
     }
     /* Restore connections if the finally will rethrow */
     task_pop_connections(pconnections);
     /* Then, disconnect all signals (enter exception
      * handling with an empty set of connections) */
     task_disconnect();
    } /* FINALLY_WILL_RETHROW */
   }
  }
  result = true;
  goto continue_serving;
 }
 return false;
}



PRIVATE bool KCALL
signal_interrupt(struct task *__restrict thread,
                 unsigned int mode) {
 u16 state;
 do {
  state = ATOMIC_READ(thread->t_state);
  if unlikely(state & TASK_STATE_FTERMINATED) {
   if (!ATOMIC_CMPXCH_WEAK(thread->t_state,state,
                           state & ~(TASK_STATE_FINTERRUPTING)))
        continue;
   return false;
  }
 } while (!ATOMIC_CMPXCH_WEAK(thread->t_state,state,
                             (state & ~(TASK_STATE_FINTERRUPTING)) |
                                       (TASK_STATE_FINTERRUPTED)));
 /* If the thread wasn't already interrupted, wake it now. */
 if (!(state & TASK_STATE_FINTERRUPTED)) {
  /* NOTE: Restrict outself to a regular wakeup, because we're not
   *       supposed to interrupt user-space (as `task_wake_for_rpc()'
   *       would do) in order to get the thread to serve RPCs. */
  if (mode & TASK_RPC_USYNC)
      return task_wake(thread);
  return task_wake_for_rpc(thread);
 }
 return true;
}

INTDEF ATTR_PERTASK struct sig task_join_signal;

PUBLIC bool KCALL
task_queue_rpc(struct task *__restrict thread,
               task_rpc_t func, void *arg,
               unsigned int mode) {
 struct rpc_slot *slot;
 /* Check: if the FUSER flag is set, we must return `false'
  *        immediately if the thread is a kernel-job
  *       (aka.: never actually returns to userspace). */
 if unlikely((mode & RPC_SLOT_FUSER) &&
             (thread->t_flags & TASK_FKERNELJOB))
    return false;
 assertf(!(mode & RPC_SLOT_FUSYNC) || (mode & RPC_SLOT_FUSER),
         "The `TASK_RPC_NOIRQ' flag can only be used with `TASK_RPC_USER'");
 assertf(!(mode & RPC_SLOT_FUASYNC) || (mode & RPC_SLOT_FUSER),
         "The `RPC_SLOT_FUASYNC' flag can only be used with `TASK_RPC_USER'");
 if (thread == THIS_TASK) {
  INCSTAT(ts_qrpc);
  if (mode & TASK_RPC_USER) {
   u16 state;
   /* Special case: deal with interrupts in our own task. */
   slot          = rpc_alloc(thread);
   assertf(slot,"Your own thread is terminating and "
                "you try to queue an RPC for yourself?");
   slot->rs_fun  = func;
   slot->rs_arg  = arg;
   slot->rs_flag = mode & RPC_SLOT_FUSER;
   slot->rs_done = NULL;
   COMPILER_WRITE_BARRIER();
   do state = ATOMIC_READ(thread->t_state);
   while (!ATOMIC_CMPXCH_WEAK(thread->t_state,state,
                             (state & ~TASK_STATE_FINTERRUPTING) |
                                       TASK_STATE_FINTERRUPTED));
   COMPILER_WRITE_BARRIER();
   /* Interrupt the current system call. */
   error_throw(E_INTERRUPT);
  }
  /* Execute the function immediately. */
  TRY {
   INCSTAT(ts_xrpc);
   (*func)(arg);
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   if (PERTASK_TEST(this_task.t_nothrow_serve)) {
    /*  Set the exception-serve flag and don't continue
     *  running more RPC functions until this exception
     *  will has been dealt with once the caller invokes
     * `task_nothrow_end()' a sufficient number of times. */
    ATOMIC_FETCHOR(THIS_TASK->t_state,
                   TASK_STATE_FSERVEEXCEPT);
    return true;
   }
   error_rethrow();
  }
  return true;
 }
 /* Add the callback to the given thread. */
 slot = rpc_alloc(thread);
 if (!slot) return false; /* Thread is terminating. */
 slot->rs_fun  = func;
 slot->rs_arg  = arg;
 slot->rs_flag = mode & (RPC_SLOT_FUSER|RPC_SLOT_FUSYNC|RPC_SLOT_FUASYNC);
 if (mode & TASK_RPC_SYNC) {
  /* Special case: Must synchronize with the completion of the RPC. */
  struct sig done_sig = SIG_INIT;
  slot->rs_done = &done_sig;

  /* Connect to the done-signal. */
  assertf(!task_isconnected(),
          "Calling thread is already connected during "
          "synchronous call to `task_queue_rpc()'");
#if CONFIG_TASK_STATIC_CONNECTIONS >= 1
  task_connect(&done_sig);
#else
  TRY {
   task_connect(&done_sig);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* Free the previously allocated RPC slot. */
   --FORTASK(thread,my_rpc).ri_cnt;
   ATOMIC_FETCHAND(thread->t_state,~TASK_STATE_FINTERRUPTING);
   error_rethrow();
  }
#endif

  /* Clear the interrupting-bit and set the interrupted-bit.
   * If this was successful, wake the thread. */
  COMPILER_BARRIER();
  if (!signal_interrupt(thread,mode)) {
   if (!task_disconnect())
        return false; /* Thread has terminated. */
   INCSTAT(ts_qrpc);
   return true;
  }
  INCSTAT(ts_qrpc);

  /* Wait for the done-signal to be sent.
   * NOTE: `task_serve_before_exit()' is kind-of cheaty in that
   *        it will broadcast USER|SYNC RPCs, but set the thread's
   *        join-signal as the sender even though we aren't
   *        connected to it (s.a. `sig_altsend()'). */
  if (task_wait() != &done_sig)
      return false;
 } else {
  slot->rs_done = NULL;
  /* Unlock the interrupt-subsystem of the thread,
   * then set the interrupted flag and wake the task. */
  COMPILER_WRITE_BARRIER();
  if (!signal_interrupt(thread,mode))
       return false; /* Thread has terminated. */
  INCSTAT(ts_qrpc);
 }
 return true;
}

INTERN ATTR_NOTHROW void KCALL task_serve_before_exit(void) {
 u16 state; size_t EXCEPT_VAR i;
 struct rpc_info *EXCEPT_VAR rpc;
 /* Wait for a lock to the FINTERRUPTING bit. */
 while ((state = ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FINTERRUPTING),
        (state & TASK_STATE_FINTERRUPTING)))
         task_yield();
 /* Clear the INTERRUPTING and INTERRUPTED flags,
  * now that we've locked them for a moment.
  * This is enough to ensure that all threads that may have
  * been trying to queue new RPCs while the caller set the
  * `TASK_STATE_FTERMINATING' flag got the message that the
  * thread has started terminating, meaning that even though
  * we're about to unlock the INTERRUPTING bit again, we can
  * be sure that no new RPCs will ever be queued again. */
 ATOMIC_FETCHAND(THIS_TASK->t_state,
                (TASK_STATE_FINTERRUPTING|TASK_STATE_FINTERRUPTED));
 /* Indicate that we're now serving RPCs. */
 ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FINRPC);
 assert(state & TASK_STATE_FTERMINATING);
 assert(!(state & TASK_STATE_FTERMINATED));
 rpc = &PERTASK(my_rpc);
 for (i = 0; i < rpc->ri_cnt; ++i) {
  struct rpc_slot *EXCEPT_VAR slot;
  slot = &rpc->ri_vec[i];
  INCSTAT(ts_xrpc);
  if (slot->rs_flag & RPC_SLOT_FUSER) {
   /* Special case for user-signals. */
   if (slot->rs_done) {
    /* Broadcast RPC completion using the join-signal.
     * This way, the sender will know that the RPC was never completed. */
    sig_altbroadcast(slot->rs_done,&PERTASK(task_join_signal));
   }
  } else {
   TRY {
    (*slot->rs_fun)(slot->rs_arg);
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    except_t code = error_code();
    if (code != E_EXIT_THREAD &&
        code != E_EXIT_PROCESS)
        error_printf("Unhandled exception in RPC during `task_exit()'");
    error_handled();
   }
   if (slot->rs_done)
       sig_broadcast(slot->rs_done);
  }
 }
 /* Indicate that no more RPCs are remaining. */
 rpc->ri_cnt = 0;
 /* Indicate that we're done serving RPCs. */
 ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FINRPC);
}


INTDEF void KCALL posix_signal_test(void);
PUBLIC void KCALL task_clear_nosignals(void) {
 u16 flags;
 flags = ATOMIC_FETCHAND(THIS_TASK->t_flags,~TASK_FNOSIGNALS);
 if (!(flags & TASK_FNOSIGNALS)) return;
 /* Check for async user-RPCs and re-set the INTERRUPTED flag if
  * there are unhandled RPCs (likely ones with the `RPC_SLOT_FUASYNC'
  * flag set, although there might be others due to race conditions)
  * still left to be executed. */
 if (PERTASK_GET(my_rpc.ri_cnt) != 0)
     ATOMIC_FETCHOR(THIS_TASK->t_flags,TASK_STATE_FINTERRUPTED);
 /* Check for posix signals. */
 posix_signal_test();
 /* Check for pending RPCs */
 task_serve();
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_RPC_C */
