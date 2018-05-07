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

DECL_BEGIN

/* Amount of static RPC slots before dynamic ones must be allocated. */
#define CONFIG_STATIC_RPC_SLOTS  2

struct rpc_slot {
    task_rpc_t       rs_fun;  /* [1..1] the function that should be called. */
    void            *rs_arg;  /* [?..?] Argument passed to `ars_fun' */
#define RPC_SLOT_FNORMAL 0x0000
#define RPC_SLOT_FUSER   TASK_RPC_USER
    uintptr_t        rs_flag; /* Set of `RPC_SLOT_F*' */
    struct sig      *rs_done; /* [0..1] A signal that will be broadcast when the RPC has finished, or NULL.
                               *  HINT: This signal is used for synchronous execution of RPC commands. */
};

struct rpc_info {
    size_t           ri_cnt;  /* Amount of RPC buffers in use. */
    size_t           ri_siz;  /* Allocated size of `ri_vec' vector. */
    struct rpc_slot *ri_vec;  /* [0..ri_cnt|alloc(ri_siz)][owned_if(!= ri_sbuf)]
                               * Vector of RPC callbacks. */
#if CONFIG_STATIC_RPC_SLOTS != 0
    struct rpc_slot  ri_sbuf[CONFIG_STATIC_RPC_SLOTS]; /* Static (aka. preallocated) RPC buffer. */
#endif
};

PRIVATE ATTR_PERTASK struct rpc_info my_rpc = {
    .ri_siz = CONFIG_STATIC_RPC_SLOTS,
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
PRIVATE struct rpc_slot *KCALL
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
  /* Take away the first RPC */
  PERTASK_DEC(my_rpc.ri_cnt);
  memcpy((void *)&slot,&PERTASK(my_rpc.ri_vec)[0],sizeof(struct rpc_slot));
  memmove(&PERTASK(my_rpc.ri_vec)[0],&PERTASK(my_rpc.ri_vec)[1],
           PERTASK_GET(my_rpc.ri_cnt)*sizeof(struct rpc_slot));
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
 u16 EXCEPT_VAR state = PERTASK_GET(this_task.t_state);
 if (state & TASK_STATE_FINTERRUPTED) {
  /* Serve RPC function calls. */
  struct rpc_slot EXCEPT_VAR slot;
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
   ATOMIC_FETCHAND(THIS_TASK->t_state,
                 ~(TASK_STATE_FINTERRUPTING|TASK_STATE_FINTERRUPTED|
                  (state & TASK_STATE_FINRPC ? 0 : TASK_STATE_FINRPC)));
done_serving:
   task_pop_connections(&connections);
   return true;
  }
  if (PERTASK_GET(my_rpc.ri_vec)[0].rs_flag & RPC_SLOT_FUSER) {
   /* Remaining RPC must be called before returning to user-space. */
   assert(!PERTASK_TESTF(this_task.t_flags,TASK_FKERNELJOB));
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
  memcpy((void *)&slot,&PERTASK(my_rpc.ri_vec)[0],sizeof(struct rpc_slot));
  memmove(&PERTASK(my_rpc.ri_vec)[0],&PERTASK(my_rpc.ri_vec)[1],
           PERTASK_GET(my_rpc.ri_cnt)*sizeof(struct rpc_slot));
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
  goto continue_serving;
 }
 return false;
}



PRIVATE bool KCALL
signal_interrupt(struct task *__restrict thread) {
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
 if (!(state & TASK_STATE_FINTERRUPTED))
     return task_wake_for_rpc(thread);
 return true;
}

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
 slot->rs_flag = mode & RPC_SLOT_FUSER;
 if (mode & TASK_RPC_SYNC) {
  /* Special case: Must synchronize with the completion of the RPC. */
  struct sig done_sig = SIG_INIT;
  slot->rs_done = &done_sig;

  /* Connect to the done-signal. */
  assertf(!task_isconnected(),
          "Calling thread is already connected during "
          "synchronous call to `task_queue_rpc()'");
  TRY {
   task_connect(&done_sig);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* Free the previously allocated RPC slot. */
   --FORTASK(thread,my_rpc).ri_cnt;
   ATOMIC_FETCHAND(thread->t_state,~TASK_STATE_FINTERRUPTING);
   error_rethrow();
  }

  /* Clear the interrupting-bit and set the interrupted-bit.
   * If this was successful, wake the thread. */
  COMPILER_BARRIER();
  if (!signal_interrupt(thread)) {
   if (!task_disconnect())
        return false; /* Thread has terminated. */
   INCSTAT(ts_qrpc);
   return true;
  }
  INCSTAT(ts_qrpc);

  /* Wait for the done-signal to be sent. */
  task_wait();
 } else {
  slot->rs_done = NULL;
  /* Unlock the interrupt-subsystem of the thread,
   * then set the interrupted flag and wake the task. */
  COMPILER_WRITE_BARRIER();
  if (!signal_interrupt(thread))
       return false; /* Thread has terminated. */
  INCSTAT(ts_qrpc);
 }
 return true;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_RPC_C */
