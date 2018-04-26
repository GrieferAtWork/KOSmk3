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
#ifndef GUARD_KERNEL_SRC_SCHED_SIGNAL_C
#define GUARD_KERNEL_SRC_SCHED_SIGNAL_C 1
#define _KOS_SOURCE 1
#define _NOSERVE_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <kos/types.h>
#include <hybrid/section.h>
#include <kernel/sections.h>
#include <kernel/malloc.h>
#include <kernel/vm.h>
#include <dev/wall.h>
#include <kernel/debug.h>
#include <kernel/interrupt.h>
#include <kernel/bind.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <sched/pertask-arith.h>
#include <except.h>
#include <assert.h>
#include <string.h>

DECL_BEGIN


INTERN ATTR_PERTASK
struct task_connections my_connections = {
    .tcs_siz = CONFIG_TASK_STATIC_CONNECTIONS
};



LOCAL void KCALL
relocate_connection(struct task_connection *__restrict dst,
                    struct task_connection *__restrict src) {
 struct task_connection *primary;
 struct sig *signal;
 assert(src->tc_sig);
 signal = dst->tc_sig = src->tc_sig;
 signal = (struct sig *)((uintptr_t)signal & TASK_CONNECTION_SIG_FMASK);
 sig_get(signal);
 COMPILER_READ_BARRIER();
 dst->tc_pself = src->tc_pself; /* Also sets `tc_last' */
 if unlikely(!src->tc_pself) {
  /* Dead connection */
  sig_put(signal);
 } else {
  primary = SIG_GETCON(signal);
  dst->tc_next = src->tc_next;
  if (dst->tc_next) {
   assert(src != primary->tc_last);
   assert(TASK_CONNECTION_GETSIG(dst->tc_next) == signal);
   assert(dst->tc_next->tc_pself == &src->tc_next);
   dst->tc_next->tc_pself = &dst->tc_next;
   if (src == primary) {
    /* Relocate the new primary connection. */
    ATOMIC_WRITE(signal->s_ptr,(uintptr_t)dst);
   } else {
    *dst->tc_pself = dst;
    sig_put(signal);
   }
  } else {
   assert(src == primary->tc_last);
   if (src == primary) {
    /* Relocate the sole primary connection. */
    dst->tc_last = dst; /* `dst' will be the new primary */
    ATOMIC_WRITE(signal->s_ptr,(uintptr_t)dst);
   } else {
    /* Relocate the new primary connection. */
    primary->tc_last = dst;
    *dst->tc_pself = dst;
    sig_put(signal);
   }
  }
 }
}


LOCAL void KCALL
delete_connection(struct task_connection *__restrict con) {
 struct task_connection *primary;
 struct sig *signal = TASK_CONNECTION_GETSIG(con);
 assert(signal);
 sig_get(signal);
 if (!con->tc_pself) {
  /* Dead connection */
  assert(con != SIG_GETCON(signal));
  sig_put(signal);
 } else {
  primary = SIG_GETCON(signal);
  assert(TASK_CONNECTION_GETSIG(primary) == signal);
  if (con == primary) {
   /* Primary connection. */
   if ((primary = con->tc_next) != NULL)
        assert(TASK_CONNECTION_GETSIG(primary) == signal),
        assert(primary->tc_pself == &con->tc_next),
        primary->tc_last = con->tc_last;
   ATOMIC_WRITE(signal->s_ptr,(uintptr_t)primary);
  } else {
   if ((*con->tc_pself = con->tc_next) == NULL) {
    /* Last connection. */
    assert(con == primary->tc_last);
    primary->tc_last = COMPILER_CONTAINER_OF(con->tc_pself,
                                             struct task_connection,
                                             tc_next);
   } else {
    /* Secondary connection. */
    assert(con != primary->tc_last);
    con->tc_next->tc_pself = con->tc_pself;
   }
   sig_put(signal);
  }
 }
}




DEFINE_PERTASK_INIT(connect_init);
INTERN ATTR_NOTHROW void KCALL
connect_init(struct task *__restrict thread) {
 unsigned int i;
 struct task_connections *con;
 /* Initialize the static connections vector. */
 con = &FORTASK(thread,my_connections);
 con->tcs_tsk = thread;
 con->tcs_vec = con->tcs_sbuf;
 assert(con->tcs_siz == CONFIG_TASK_STATIC_CONNECTIONS);
 for (i = 0; i < CONFIG_TASK_STATIC_CONNECTIONS; ++i) {
  assert(con->tcs_sbuf[i].tc_sig == NULL);
  con->tcs_sbuf[i].tc_conn = con;
 }
 COMPILER_WRITE_BARRIER();
}

DEFINE_PERTASK_FINI(connect_fini);
INTERN ATTR_NOTHROW void KCALL
connect_fini(struct task *__restrict thread) {
 struct task_connections *mycon;
 struct task_connection *vec;
 unsigned int i;
 mycon = &FORTASK(thread,my_connections);
 assert(mycon->tcs_tsk == thread);
 vec = mycon->tcs_vec;
 /* Disconnect remaining signals. (Prevents a race condition when a
  * thread is destroyed with signals when one of those signals is sent) */
 for (i = 0; i < mycon->tcs_cnt; ++i) {
  assert(vec[i].tc_conn == mycon);
  delete_connection(&vec[i]);
 }
 /* Free a dynamic connections buffer. */
 if (vec != mycon->tcs_sbuf)
     kfree(vec);
}

PRIVATE void *KCALL
kmalloc_consafe(size_t n_bytes, gfp_t flags) {
 void *COMPILER_IGNORE_UNINITIALIZED(result);
 struct task_connections consave;
 task_push_connections(&consave);
 TRY {
  result = kmalloc(n_bytes,flags);
 } FINALLY {
  task_pop_connections(&consave);
 }
 return result;
}

PRIVATE void *KCALL
krealloc_consafe(void *ptr, size_t n_bytes, gfp_t flags) {
 void *COMPILER_IGNORE_UNINITIALIZED(result);
 struct task_connections consave;
 task_push_connections(&consave);
 TRY {
  result = krealloc(ptr,n_bytes,flags);
 } FINALLY {
  task_pop_connections(&consave);
 }
 return result;
}

PRIVATE void KCALL
kfree_consafe(void *ptr) {
 struct task_connections consave;
 task_push_connections(&consave);
 TRY {
  kfree(ptr);
 } FINALLY {
  task_pop_connections(&consave);
 }
}

#if !defined(NDEBUG) && 1
PRIVATE ATTR_PERTASK unsigned int connections_push_recursion = 0;
PRIVATE ATTR_PERTASK struct task_connections *connections_push_stack[16];
#define DEBUG_PUSH_CONNECTIONS(safe) do_debug_push_connections(safe)
#define DEBUG_POP_CONNECTIONS(safe)  do_debug_pop_connections(safe)
PRIVATE void KCALL
do_debug_push_connections(struct task_connections *__restrict safe) {
 unsigned int i,index = PERTASK_GET(connections_push_recursion);
 assert(index != (unsigned int)-1);
 if (index < COMPILER_LENOF(connections_push_stack)) {
  PERTASK_SET(connections_push_stack[index],safe);
  for (i = 0; i < index; ++i) {
   assertf(safe <  PERTASK(connections_push_stack[i]) ||
           safe >= PERTASK(connections_push_stack[i])+1,
           "Overlapping connections restore descriptors:\n"
           "Task connections set %p overlaps with set at %p\n"
           "This is likely caused by a loop that doesn't "
           "properly restore the saved connections set",
           safe,PERTASK(connections_push_stack[i]));
  }
 }
 PERTASK_INC(connections_push_recursion);
}
PRIVATE void KCALL
do_debug_pop_connections(struct task_connections *__restrict safe) {
 unsigned int count;
 count = PERTASK_GET(connections_push_recursion);
 assertf(count != 0,"Connections weren't pushed");
 if (count < COMPILER_LENOF(connections_push_stack)) {
  assertf(PERTASK_GET(connections_push_stack[count-1]) == safe,
          "Incorrect restore location (expected %p, but got %p)\n",
          PERTASK_GET(connections_push_stack[count-1]),safe);
 }
 PERTASK_DEC(connections_push_recursion);
}
#else
#define DEBUG_PUSH_CONNECTIONS(safe) (void)0
#define DEBUG_POP_CONNECTIONS(safe)  (void)0
#endif


PUBLIC ATTR_NOTHROW void KCALL
task_push_connections(struct task_connections *__restrict safe) {
 struct task_connections *mycon;
 unsigned int i;
 DEBUG_PUSH_CONNECTIONS(safe);
 mycon = &PERTASK(my_connections);
 assert(mycon->tcs_cnt <= mycon->tcs_siz);
 assert(mycon->tcs_tsk == THIS_TASK);
 /* Save the used and allocates connection size. */
 safe->tcs_chn = mycon->tcs_chn;
 safe->tcs_cnt = mycon->tcs_cnt;
 if (!safe->tcs_cnt) return; /* No active connections. */
 safe->tcs_siz = mycon->tcs_siz;
 safe->tcs_tsk = mycon->tcs_tsk;
 if likely(mycon->tcs_vec == mycon->tcs_sbuf) {
  /* The static buffer is being used. */
  assert(mycon->tcs_siz == CONFIG_TASK_STATIC_CONNECTIONS);
  safe->tcs_vec = safe->tcs_sbuf;
  for (i = 0; i < safe->tcs_cnt; ++i) {
   struct task_connection *dst = &safe->tcs_sbuf[i];
   struct task_connection *src = &mycon->tcs_sbuf[i];
   assert(src->tc_conn == mycon);
   dst->tc_conn = safe;
   relocate_connection(dst,src);
  }
  COMPILER_BARRIER();
  safe->tcs_sig = mycon->tcs_sig;
 } else {
  /* A dynamic buffer is being used. */
  safe->tcs_vec = mycon->tcs_vec;
  for (i = 0; i < safe->tcs_cnt; ++i) {
   struct task_connection *con = &safe->tcs_vec[i];
   struct sig *signal;
   signal = TASK_CONNECTION_GETSIG(con);
   assert(signal);
   assert(con->tc_conn == mycon);
   sig_get(signal);
   con->tc_conn = safe;
   sig_put(signal);
  }
  COMPILER_BARRIER();
  safe->tcs_sig  = mycon->tcs_sig;
  mycon->tcs_vec = mycon->tcs_sbuf;
  mycon->tcs_siz = CONFIG_TASK_STATIC_CONNECTIONS;
 }
 mycon->tcs_cnt = 0;
 mycon->tcs_sig = NULL;
 COMPILER_WRITE_BARRIER();
}

PUBLIC ATTR_NOTHROW void KCALL
task_pop_connections(struct task_connections *__restrict safe) {
 struct task_connections *mycon;
 unsigned int i;
 DEBUG_POP_CONNECTIONS(safe);
 task_disconnect();
 mycon = &PERTASK(my_connections);
 mycon->tcs_chn = safe->tcs_chn;
 if (!safe->tcs_cnt) return;
 assert(!mycon->tcs_sig);
 assert(!mycon->tcs_cnt);
 assert(safe->tcs_tsk == THIS_TASK);
 assert(mycon->tcs_tsk == THIS_TASK);
 mycon->tcs_cnt = safe->tcs_cnt;
 if likely(safe->tcs_vec == safe->tcs_sbuf) {
  if (mycon->tcs_vec != mycon->tcs_sbuf) {
   kfree(mycon->tcs_vec);
   mycon->tcs_vec = mycon->tcs_sbuf;
   mycon->tcs_siz = CONFIG_TASK_STATIC_CONNECTIONS;
  }
  /* Relocate the static buffer. */
  assert(safe->tcs_siz  == CONFIG_TASK_STATIC_CONNECTIONS);
  assert(mycon->tcs_siz == CONFIG_TASK_STATIC_CONNECTIONS);
  for (i = 0; i < safe->tcs_cnt; ++i) {
   struct task_connection *dst = &mycon->tcs_sbuf[i];
   struct task_connection *src = &safe->tcs_sbuf[i];
   assert(src->tc_conn == safe);
   assert(dst->tc_conn == mycon);
   relocate_connection(dst,src);
  }
 } else {
  if (mycon->tcs_vec != mycon->tcs_sbuf)
      kfree(mycon->tcs_vec);
  /* A dynamic buffer is being used. */
  mycon->tcs_vec = safe->tcs_vec;
  mycon->tcs_siz = safe->tcs_siz;
  for (i = 0; i < safe->tcs_cnt; ++i) {
   struct task_connection *con = &mycon->tcs_vec[i];
   struct sig *signal;
   signal = TASK_CONNECTION_GETSIG(con);
   assert(signal);
   assert(con->tc_conn == safe);
   sig_get(signal);
   con->tc_conn = mycon;
   sig_put(signal);
  }
 }
 COMPILER_BARRIER();
 assert(mycon->tcs_cnt <= mycon->tcs_siz);
 /* If a signal was send to the saved connections set, inherit it. */
 ATOMIC_CMPXCH(mycon->tcs_sig,NULL,safe->tcs_sig);
}

PUBLIC ATTR_HOTTEXT void KCALL
task_connect(struct sig *__restrict signal) {
 struct task_connections *mycon;
 struct task_connection *con;
 struct task_connection *primary;
 mycon = &PERTASK(my_connections);
 if (mycon->tcs_sig) return; /* A signal was already send. */
 assert(mycon->tcs_cnt <= mycon->tcs_siz);
 assert(mycon->tcs_siz != 0);
 assert(mycon->tcs_tsk == THIS_TASK);
 con = mycon->tcs_vec;
 /* Most likely case: first connection. */
 if likely(!mycon->tcs_cnt) goto fill_con;
 if unlikely(mycon->tcs_cnt == mycon->tcs_siz) {
  unsigned int i;
  size_t new_siz = mycon->tcs_siz;
  TRY {
   if (con == mycon->tcs_sbuf) {
    assert(mycon->tcs_siz == CONFIG_TASK_STATIC_CONNECTIONS);
    /* The static buffer was being used. */
    con = (struct task_connection *)kmalloc_consafe(new_siz*sizeof(struct task_connection),
                                                    GFP_SHARED);
    for (i = 0; i < CONFIG_TASK_STATIC_CONNECTIONS; +i) {
     con[i].tc_conn = mycon;
     relocate_connection(&con[i],&mycon->tcs_sbuf[i]);
    }
    mycon->tcs_vec = con;
   } else {
    /* Try to inplace-extend the vector of existing
     * connections, so we don't have to relocate it. */
    if (!krealloc_consafe(con,new_siz*sizeof(struct task_connection),
                          GFP_SHARED|GFP_NOMOVE)) {
     struct task_connection *new_con;
     /* Didn't work. -> Must allocate a new vector and relocate into it. */
     new_con = (struct task_connection *)kmalloc_consafe(new_siz*sizeof(struct task_connection),
                                                         GFP_SHARED);
     /* Relocate connections into the new buffer. */
     for (i = 0; i < mycon->tcs_siz; ++i) {
      new_con[i].tc_conn = mycon;
      relocate_connection(&new_con[i],&con[i]);
     }
     kfree_consafe(mycon->tcs_vec);
     mycon->tcs_vec = con = new_con;
    }
    /* Setup connection set pointers for newly allocated connections. */
    for (i = mycon->tcs_siz; i < new_siz; ++i)
         con[i].tc_conn = mycon;
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* Disconnect all signals that were already
    * connected if we've failed to allocate more slots. */
   task_disconnect();
   error_rethrow();
  }
  mycon->tcs_siz = new_siz;
 }
 /* Append a new connection at the end of the vector. */
 con += mycon->tcs_cnt;
fill_con:
 assert(con->tc_conn == mycon);
 con->tc_sig  = signal;
 con->tc_next = NULL;
 ++mycon->tcs_cnt;
#ifndef NDEBUG
 assert((u32)signal->s_ptr != 0xdeadbeef);
#endif
 sig_get(signal);
 primary = SIG_GETCON(signal);
 if likely(primary == NULL) {
  /* Primary connection. */
  con->tc_last = con;
  ATOMIC_WRITE(signal->s_ptr,(uintptr_t)con); /* Write + unlock. */
 } else {
  struct task_connection *last;
  /* Secondary connection. */
#ifndef NDEBUG
  last = primary;
  while (last->tc_next) last = last->tc_next;
  assertf(last == primary->tc_last,
          "Broken last: %p != %p",
          last,primary->tc_last);
#else
  last = primary->tc_last;
#endif
  assert(last);
  con->tc_pself    = &last->tc_next;
  last->tc_next    = con;
  primary->tc_last = con;
  sig_put(signal);
 }
}

PUBLIC void KCALL
task_connect_ghost(struct sig *__restrict signal) {
 struct task_connections *mycon;
 struct task_connection *con;
 struct task_connection *primary;
 mycon = &PERTASK(my_connections);
 if (mycon->tcs_sig) return; /* A signal was already send. */
 assert(mycon->tcs_cnt <= mycon->tcs_siz);
 assert(mycon->tcs_siz != 0);
 assert(mycon->tcs_tsk == THIS_TASK);
 con = mycon->tcs_vec;
 /* Most likely case: first connection. */
 if likely(!mycon->tcs_cnt) goto fill_con;
 if unlikely(mycon->tcs_cnt == mycon->tcs_siz) {
  unsigned int i;
  size_t new_siz = mycon->tcs_siz;
  TRY {
   if (con == mycon->tcs_sbuf) {
    assert(mycon->tcs_siz == CONFIG_TASK_STATIC_CONNECTIONS);
    /* The static buffer was being used. */
    con = (struct task_connection *)kmalloc_consafe(new_siz*sizeof(struct task_connection),
                                                    GFP_SHARED);
    for (i = 0; i < CONFIG_TASK_STATIC_CONNECTIONS; +i) {
     con[i].tc_conn = mycon;
     relocate_connection(&con[i],&mycon->tcs_sbuf[i]);
    }
    mycon->tcs_vec = con;
   } else {
    /* Try to inplace-extend the vector of existing
     * connections, so we don't have to relocate it. */
    if (!krealloc_consafe(con,new_siz*sizeof(struct task_connection),
                          GFP_SHARED|GFP_NOMOVE)) {
     struct task_connection *new_con;
     /* Didn't work. -> Must allocate a new vector and relocate into it. */
     new_con = (struct task_connection *)kmalloc_consafe(new_siz*sizeof(struct task_connection),
                                                         GFP_SHARED);
     /* Relocate connections into the new buffer. */
     for (i = 0; i < mycon->tcs_siz; ++i) {
      new_con[i].tc_conn = mycon;
      relocate_connection(&new_con[i],&con[i]);
     }
     kfree_consafe(mycon->tcs_vec);
     mycon->tcs_vec = con = new_con;
    }
    /* Setup connection set pointers for newly allocated connections. */
    for (i = mycon->tcs_siz; i < new_siz; ++i)
         con[i].tc_conn = mycon;
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* Disconnect all signals that were already
    * connected if we've failed to allocate more slots. */
   task_disconnect();
   error_rethrow();
  }
  mycon->tcs_siz = new_siz;
 }
 /* Append a new connection at the end of the vector. */
 con += mycon->tcs_cnt;
fill_con:
 assert(con->tc_conn == mycon);
 con->tc_sig  = (struct sig *)((uintptr_t)signal |
                                TASK_CONNECTION_SIG_FGHOST);
 con->tc_next = NULL;
 ++mycon->tcs_cnt;
#ifndef NDEBUG
 assert((u32)signal->s_ptr != 0xdeadbeef);
#endif
 sig_get(signal);
 primary = SIG_GETCON(signal);
 if likely(primary == NULL) {
  /* Primary connection. */
  con->tc_last = con;
  ATOMIC_WRITE(signal->s_ptr,(uintptr_t)con); /* Write + unlock. */
 } else {
  struct task_connection *last;
  /* Secondary connection. */
#ifndef NDEBUG
  last = primary;
  while (last->tc_next) last = last->tc_next;
  assertf(last == primary->tc_last,
          "Broken last: %p != %p",
          last,primary->tc_last);
#else
  last = primary->tc_last;
#endif
  assert(last);
  con->tc_pself    = &last->tc_next;
  last->tc_next    = con;
  primary->tc_last = con;
  sig_put(signal);
 }
}


PUBLIC ATTR_HOTTEXT ATTR_NOTHROW
struct sig *KCALL task_disconnect(void) {
 struct sig *result;
 struct task_connections *mycon;
 struct task_connection *vec;
 unsigned int i;
 mycon = &PERTASK(my_connections);
 if (!mycon->tcs_cnt) {
  assert(!mycon->tcs_sig);
  return NULL;
 }
 vec = mycon->tcs_vec;
 for (i = 0; i < mycon->tcs_cnt; ++i) {
  assert(vec[i].tc_conn == mycon);
  delete_connection(&vec[i]);
 }
 COMPILER_BARRIER();
 mycon->tcs_cnt = 0;
 result = mycon->tcs_sig;
 mycon->tcs_sig = NULL;
 COMPILER_BARRIER();
 return result;
}




PUBLIC ATTR_NOTHROW bool KCALL
task_isconnected(void) {
 return PERTASK_TEST(my_connections.tcs_cnt);
}
PUBLIC ATTR_NOTHROW size_t KCALL
task_numconnected(void) {
 return PERTASK_GET(my_connections.tcs_cnt);
}
PUBLIC ATTR_NOTHROW bool KCALL
task_connected(struct sig *__restrict signal) {
 unsigned int i;
 struct task_connections *mycon;
 mycon = &PERTASK(my_connections);
 /* Check if the given `signal' appears in any connection slot. */
 for (i = 0; i < mycon->tcs_cnt; ++i) {
  if (TASK_CONNECTION_GETSIG(&mycon->tcs_vec[i]) == signal)
      return true;
 }
 return false;
}
PUBLIC ATTR_NOTHROW bool KCALL
task_setsignaled(struct sig *__restrict signal) {
 assertf(task_connected(signal),
         "The calling thread is not connected to a signal at %p",
         signal);
 return ATOMIC_CMPXCH(PERTASK(my_connections.tcs_sig),NULL,signal);
}

PUBLIC ATTR_HOTTEXT struct sig *
KCALL __os_task_waitfor(jtime_t abs_timeout) {
 struct task_connections *mycon;
 struct sig *COMPILER_IGNORE_UNINITIALIZED(result);
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
   result = ATOMIC_READ(mycon->tcs_sig);
   if (result) { PREEMPTION_ENABLE(); break; }
   /* Serve RPC functions. */
   if (task_serve()) continue;

   /* Sleep for a bit, or until we're interrupted. */
   sleep_ok = task_sleep(abs_timeout);

   result = ATOMIC_READ(mycon->tcs_sig);
   /* A signal was received in the mean time. */
   if (result) break;
   if (!sleep_ok) break; /* Timeout */
   /* Continue spinning */
  }
 } FINALLY {
  /* Always disconnect all connected signals,
   * thus resetting the connections list. */
  task_disconnect();
 }
 return result;
}
PUBLIC ATTR_NOTHROW struct sig *KCALL task_trywait(void) {
 struct sig *result;
 result = PERTASK_GET(my_connections.tcs_sig);
 COMPILER_READ_BARRIER();
 if (result) task_disconnect();
 return result;
}
PUBLIC ATTR_HOTTEXT ATTR_RETNONNULL
struct sig *KCALL task_wait(void) {
 return __os_task_waitfor(JTIME_INFINITE);
}






PRIVATE ATTR_HOTTEXT ATTR_NOTHROW bool KCALL
sig_sendone_locked(struct sig *__restrict self, bool unlock) {
 struct task_connection *primary,*next_con,*last_con;
 struct task_connections *cons;
 bool wake_ok = false;
 assert(sig_holding(self));
again:
 primary = SIG_GETCON(self);
 if (!primary) {
  if (unlock)
      sig_put(self);
  return false;
 }

 cons = primary->tc_conn;
#if 0 /* Not necessarily the case while relocating... */
 assertf(primary >= cons->tcs_vec &&
         primary <= cons->tcs_vec+cons->tcs_cnt,

         "primary                                = %p\n"
         "cons->tcs_tsk                          = %p\n"
         "cons                                   = %p\n"
         "&FORTASK(cons->tcs_tsk,my_connections) = %p\n"
         "cons->tcs_vec                          = %p\n"
         "cons->tcs_vec+cons->tcs_cnt            = %p\n"
         "cons->tcs_cnt                          = %p\n"
         ,primary
         ,cons->tcs_tsk
         ,cons
         ,&FORTASK(cons->tcs_tsk,my_connections)
         ,cons->tcs_vec
         ,cons->tcs_vec+cons->tcs_cnt
         ,cons->tcs_cnt);
#endif

 /* Try to set this signal as the one that will be received by the task. */
 if likely(ATOMIC_CMPXCH(cons->tcs_sig,NULL,self)) {
  /* If the signal got delivered to the main connection set, wake the task. */
  if likely(cons == &FORTASK(cons->tcs_tsk,my_connections)) {
   wake_ok = task_wake(cons->tcs_tsk);
   if (wake_ok) /* Ghost connections don't count as active receivers. */
       wake_ok = !((uintptr_t)primary->tc_sig & TASK_CONNECTION_SIG_FGHOST);
  } else {
#if 1
   /* Even though we can guaranty that the target thread will eventually
    * receive this signal, we have no way of ensuring that it will actually
    * act on it.
    * Because of this (and the fact that we didn't actually wake it), we must
    * assume the worst and act as though the thread will never receive it:
    * >> struct sig s = SIG_INIT;
    * >> 
    * >> thread:
    * >>     struct task_connections cons;
    * >>     task_connect(&s);
    * >>
    * >>     // This code is basically what is executed during #PF handling
    * >>     // But since something might go wrong during that, despite the
    * >>     // fact that the thread will actually restore its old connections
    * >>     // properly, there is no way of predicting if it will actually
    * >>     // act on them.
    * >>     // If the caller only wants us to wake ~1~ task, they should be
    * >>     // able to assume that at least ~1~ task was woken (not at most)
    * >>     // Otherwise, what would be the point of it when a mutex could
    * >>     // be sure that at least one waiter was woken unless it woke all
    * >>     // of them whenever the lock gets released.
    * >>     task_push_connections(&cons);
    * >>     TRY {
    * >>         if (!try_some_stuff())
    * >>              error_throw(E_BADALLOC);
    * >>     } FINALLY {
    * >>         task_pop_connections(&cons);
    * >>     }
    * >>
    * >>     // Wait for the signal
    * >>     task_wait();
    * >>     
    */
   wake_ok = false;
#else
   /* This still counts as an OK wake, as the thread will
    * eventually notice what we've done when its saved
    * connection set is restored. */
   wake_ok = true;
#endif
  }
 }
 /* Remove this connection. */
 assert(TASK_CONNECTION_GETSIG(primary) == self);
 next_con = primary->tc_next;
 last_con = primary->tc_last;
 COMPILER_READ_BARRIER();
 primary->tc_pself = NULL; /* Indicate that a signal was send to this slot */
 assert(TASK_CONNECTION_GETSIG(primary) == self);
 COMPILER_WRITE_BARRIER();
 assertf((last_con == primary) == (next_con == NULL),
         "last_con = %p\n"
         "con      = %p\n"
         "next_con = %p\n",
         last_con,primary,next_con);

 if (!next_con) {
  ATOMIC_WRITE(self->s_ptr,unlock ? 0 : SIG_FLOCKBIT);
 } else {
  /* Load the next connection. */
  assert(TASK_CONNECTION_GETSIG(next_con) == self);
  assert(next_con->tc_pself == &primary->tc_next);
  /* Update the last-pointer of the next connection. */
  next_con->tc_last = last_con;
  COMPILER_WRITE_BARRIER();
  /* Save a pointer to the next connection in the signal. */
  if (!unlock) *(uintptr_t *)&next_con |= SIG_FLOCKBIT;
  ATOMIC_WRITE(self->s_ptr,(uintptr_t)next_con);
 }
 /* If we didn't manage the wake any thread, start again.
  * The caller is either expecting us to wake everyone (in which
  * case they'll re-run us no matter what), or they expect us
  * to wake up to a certain number of tasks (in which case us
  * failing to wake some task shouldn't count to the number of
  * supposedly woken tasks)
  * Consider a mutex for example:
  *   - During an unlock, only a single task is woken,
  *     considering the fact that only a single task
  *     can even hold the mutex at the same time.
  *   - If that wake fails, the caller will think
  *     that there are no tasks connected to the signal
  *     and will continue, assuming that noone needed
  *     to be woken.
  *   - This in turn could lead to a deadlock a mutex
  *     UP operation wouldn't actually wake any tasks,
  *     despite the fact that there are tasks waiting.
  */
 if unlikely(!wake_ok) {
  if (unlock)
      sig_get(self);
  goto again;
 }
 return true;
}

PUBLIC ATTR_HOTTEXT ATTR_NOTHROW size_t KCALL
sig_send(struct sig *__restrict self, size_t max_threads) {
 size_t result = 0;
 /* Optimization: If there are no threads
  * waiting for this signal, stop immediately. */
 if (!ATOMIC_READ(self->s_ptr)) goto done;

 while (result < max_threads) {
  bool send_ok;
  /* Temporarily acquire a lock, then send the signal. */
  sig_get(self);
  send_ok = sig_sendone_locked(self,true);
  if (!send_ok) break;
  ++result;
 }
done:
 return result;
}

PUBLIC ATTR_HOTTEXT ATTR_NOTHROW size_t KCALL
sig_broadcast(struct sig *__restrict self) {
 size_t result = 0;
 /* Optimization: If there are no threads
  * waiting for this signal, stop immediately. */
 if (!ATOMIC_READ(self->s_ptr)) goto done;

 for (;;) {
  bool send_ok;
  /* Temporarily acquire a lock, then send the signal. */
  sig_get(self);
  send_ok = sig_sendone_locked(self,true);
  if (!send_ok) break;
  ++result;
 }
done:
 return result;
}

PUBLIC ATTR_HOTTEXT ATTR_NOTHROW size_t KCALL
sig_send_locked(struct sig *__restrict self, size_t max_threads) {
 size_t result = 0;
 while (result < max_threads &&
        sig_sendone_locked(self,false))
        ++result;
 return result;
}
PUBLIC ATTR_HOTTEXT ATTR_NOTHROW size_t KCALL
sig_broadcast_locked(struct sig *__restrict self) {
 size_t result = 0;
 while (sig_sendone_locked(self,false))
        ++result;
 return result;
}

/* Channel-based signal delivery. */
PRIVATE ATTR_NOTHROW bool KCALL
sig_sendone_channel_locked(struct sig *__restrict self,
                           uintptr_t signal_mask, bool unlock) {
 struct task_connection *primary,*target;
 struct task_connection *next_con,*last_con;
 struct task_connections *cons;
 bool wake_ok = false;
 assert(sig_holding(self));
again:
 primary = SIG_GETCON(self);
 if (!primary) {
stop_sending:
  if (unlock)
      sig_put(self);
  return false;
 }

 target = primary;
 /* Find the first target listening for the given channel mask. */
 for (;;) {
  assert(TASK_CONNECTION_GETSIG(target) == self);
  cons = target->tc_conn;
  if ((cons->tcs_chn & signal_mask) != 0)
       break;
  target = target->tc_next;
  if (!target)
       goto stop_sending;
 }

 /* Try to set this signal as the one that will be received by the task. */
 if likely(ATOMIC_CMPXCH(cons->tcs_sig,NULL,self)) {
  /* If the signal got delivered to the main connection set, wake the task. */
  if likely(cons == &FORTASK(cons->tcs_tsk,my_connections)) {
   wake_ok = task_wake(cons->tcs_tsk);
   if (wake_ok) /* Ghost connections don't count as active receivers. */
       wake_ok = !((uintptr_t)primary->tc_sig & TASK_CONNECTION_SIG_FGHOST);
  } else {
   wake_ok = false;
  }
 }
 /* Remove this connection. */
 assert(TASK_CONNECTION_GETSIG(target) == self);
 if (target == primary) {
  next_con = primary->tc_next;
  last_con = primary->tc_last;
  COMPILER_READ_BARRIER();
  primary->tc_pself = NULL; /* Indicate that a signal was send to this slot */
  assert(TASK_CONNECTION_GETSIG(primary) == self);
  COMPILER_WRITE_BARRIER();
  assertf((last_con == primary) == (next_con == NULL),
          "last_con = %p\n"
          "con      = %p\n"
          "next_con = %p\n",
          last_con,primary,next_con);

  if (!next_con) {
   ATOMIC_WRITE(self->s_ptr,unlock ? 0 : SIG_FLOCKBIT);
  } else {
   /* Load the next connection. */
   assert(TASK_CONNECTION_GETSIG(next_con) == self);
   assert(next_con->tc_pself == &primary->tc_next);
   /* Update the last-pointer of the next connection. */
   next_con->tc_last = last_con;
   COMPILER_WRITE_BARRIER();
   /* Save a pointer to the next connection in the signal. */
   if (!unlock) *(uintptr_t *)&next_con |= SIG_FLOCKBIT;
   ATOMIC_WRITE(self->s_ptr,(uintptr_t)next_con);
  }
 } else {
  if ((*target->tc_pself = target->tc_next) != NULL) {
   assert(primary->tc_last != target);
   assert(target->tc_next->tc_pself == &target->tc_next);
   target->tc_next->tc_pself = target->tc_pself;
  } else {
   assert(primary->tc_last == target);
   /* Update the new last connection pointer. */
   primary->tc_last = COMPILER_CONTAINER_OF(target->tc_pself,
                                            struct task_connection,
                                            tc_next);
  }
  COMPILER_WRITE_BARRIER();
  target->tc_pself = NULL; /* Indicate that a signal was send to this slot */
  COMPILER_WRITE_BARRIER();
  if (unlock)
      sig_put(self);
 }
 if unlikely(!wake_ok) {
  if (unlock)
      sig_get(self);
  goto again;
 }
 return true;
}

PUBLIC ATTR_NOTHROW size_t KCALL
sig_send_channel(struct sig *__restrict self,
                 uintptr_t signal_mask,
                 size_t max_threads) {
 size_t result = 0;
 /* Optimization: If there are no threads
  * waiting for this signal, stop immediately. */
 if (!ATOMIC_READ(self->s_ptr)) goto done;

 while (result < max_threads) {
  bool send_ok;
  /* Temporarily acquire a lock, then send the signal. */
  sig_get(self);
  send_ok = sig_sendone_channel_locked(self,signal_mask,true);
  if (!send_ok) break;
  ++result;
 }
done:
 return result;
}

PUBLIC ATTR_NOTHROW size_t KCALL
sig_broadcast_channel(struct sig *__restrict self,
                      uintptr_t signal_mask) {
 size_t result = 0;
 /* Optimization: If there are no threads
  * waiting for this signal, stop immediately. */
 if (!ATOMIC_READ(self->s_ptr)) goto done;

 for (;;) {
  bool send_ok;
  /* Temporarily acquire a lock, then send the signal. */
  sig_get(self);
  send_ok = sig_sendone_channel_locked(self,signal_mask,true);
  if (!send_ok) break;
  ++result;
 }
done:
 return result;
}

PUBLIC ATTR_NOTHROW size_t KCALL
sig_send_channel_locked(struct sig *__restrict self,
                        uintptr_t signal_mask,
                        size_t max_threads) {
 size_t result = 0;
 while (result < max_threads &&
        sig_sendone_channel_locked(self,signal_mask,false))
        ++result;
 return result;
}
PUBLIC ATTR_NOTHROW size_t KCALL
sig_broadcast_channel_locked(struct sig *__restrict self,
                             uintptr_t signal_mask) {
 size_t result = 0;
 while (sig_sendone_channel_locked(self,signal_mask,false))
        ++result;
 return result;
}





PUBLIC ATTR_NOTHROW uintptr_t KCALL
task_channelmask(uintptr_t mask) {
 /* Simply set the channel mask. */
 assertf(!PERTASK_GET(my_connections.tcs_cnt) ||
          PERTASK_GET(my_connections.tcs_chn) == mask,
         "You may not change the channel mask after already being connected");
 return ATOMIC_XCH(PERTASK(my_connections.tcs_chn),mask);
}

PUBLIC ATTR_NOTHROW uintptr_t KCALL
task_openchannel(uintptr_t mask) {
 /* Simply open more channels. */
 return ATOMIC_FETCHOR(PERTASK(my_connections.tcs_chn),mask);
}


/* TODO: Implement these for real (Using `task_wake_p()') */
DEFINE_PUBLIC_ALIAS(sig_send_p,sig_send);
DEFINE_PUBLIC_ALIAS(sig_broadcast_p,sig_broadcast);
DEFINE_PUBLIC_ALIAS(sig_send_locked_p,sig_send_locked);
DEFINE_PUBLIC_ALIAS(sig_broadcast_locked_p,sig_broadcast_locked);
DEFINE_PUBLIC_ALIAS(sig_send_channel_p,sig_send_channel);
DEFINE_PUBLIC_ALIAS(sig_broadcast_channel_p,sig_broadcast_channel);
DEFINE_PUBLIC_ALIAS(sig_send_channel_locked_p,sig_send_channel_locked);
DEFINE_PUBLIC_ALIAS(sig_broadcast_channel_locked_p,sig_broadcast_channel_locked);



PUBLIC struct sig *KCALL
task_waitfor_tmrel(USER CHECKED struct timespec const *rel_timeout) {
 return task_waitfor(jiffies + jiffies_from_timespec(*rel_timeout));
}
PUBLIC struct sig *KCALL
task_waitfor_tmabs(USER CHECKED struct timespec const *abs_timeout) {
 struct timespec diff = *abs_timeout;
 struct timespec wall = wall_gettime(&wall_kernel);
 if (TIMESPEC_LOWER(diff,wall))
     return task_disconnect();
 TIMESPEC_SUB(diff,wall);
 return task_waitfor(jiffies + jiffies_from_timespec(diff));
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_SIGNAL_C */
