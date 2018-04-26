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
#ifndef GUARD_KERNEL_SRC_SCHED_AFFINITY_C
#define GUARD_KERNEL_SRC_SCHED_AFFINITY_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <sched/task.h>
#include <sched/affinity.h>
#include <sched/signal.h>
#include <kernel/bind.h>
#include <kernel/sections.h>
#include <except.h>
#include <assert.h>
#include <string.h>

DECL_BEGIN

#ifdef CONFIG_NO_SMP
PUBLIC void KCALL
task_getaffinity(struct task *__restrict UNUSED(thread),
                 kernel_cpuset_t affinity) {
 memset(affinity,-1,sizeof(kernel_cpuset_t));
}
PUBLIC bool KCALL
task_setaffinity(struct task *__restrict UNUSED(thread),
                 kernel_cpuset_t const affinity) {
 /* Emulate setaffinity() by checking of the boot
  * CPU (which everything is hosted by) is allowed. */
 return kernel_cpuset_has(affinity,0);
}

#else

/* The default affinity allows all CPUs. */
INTERN ATTR_PERTASK kernel_cpuset_t _this_affinity = KERNEL_CPUSET_INIT_FULL;
INTERN ATTR_PERTASK DEFINE_ATOMIC_RWLOCK(_this_affinity_lock);

DEFINE_PERTASK_CLONE(thread_affinity_clone);
PRIVATE ATTR_USED void KCALL
thread_affinity_clone(struct task *__restrict new_thread,
                      u32 UNUSED(flags)) {
 /* Copy the affinity of the calling thread into the new thread. */
 atomic_rwlock_read(&PERTASK(_this_affinity_lock));
 memcpy(&FORTASK(new_thread,_this_affinity),
        &PERTASK(_this_affinity),
         sizeof(kernel_cpuset_t));
 atomic_rwlock_endread(&PERTASK(_this_affinity_lock));
}

PUBLIC void KCALL
task_getaffinity(struct task *__restrict thread,
                 kernel_cpuset_t affinity) {
 atomic_rwlock_read(&FORTASK(thread,_this_affinity_lock));
 memcpy(affinity,
        FORTASK(thread,_this_affinity),
        sizeof(kernel_cpuset_t));
 atomic_rwlock_endread(&FORTASK(thread,_this_affinity_lock));
}


PRIVATE void KCALL rpc_broadcast_signal(void *arg) {
 sig_broadcast((struct sig *)arg);
}

PUBLIC bool KCALL
task_setcpu(struct task *__restrict thread,
            struct cpu *__restrict new_cpu,
            bool overrule_affinity) {
 int error;
 assert(!task_isconnected());
 while ((error = task_setcpu_impl(thread,new_cpu,overrule_affinity)) == TASK_SETCPU_AGAIN) {
  struct sig event_signal = SIG_INIT;
  /* Check if the thread will always keep its current core. */
  if (thread->t_flags & TASK_FALWAYSKEEPCORE)
      return false;
  if unlikely(thread == THIS_TASK)
      return false; /* Shouldn't happen, but prevents problems. */
  /* Otherwise, query an RPC that signals back to
   * us that the thread is ready to change CPUs. */
  task_connect(&event_signal);
  if (!task_queue_rpc(thread,
                     &rpc_broadcast_signal,
                     &event_signal,
                      TASK_RPC_NORMAL)) {
   task_disconnect();
   return false;
  }
  /* Wait for the signal to trigger.
   * NOTE: We can't re-try to change the thread's CPU because we must
   *       guaranty that `event_signal' remains in-scope until the RPC
   *       has finished. Otherwise, it might end up accidentally
   *       tripping an unallocated portion of our stack if we were to
   *       return before it could execute. */
#ifdef NDEBUG
  task_wait();
#else
  {
   struct sig *s = task_wait();
   assertf(s == &event_signal,
          "s = %p, event_signal = %p",
           s,&event_signal);
  }
#endif

  /* Since `event_signal' is about to go out-of-scope, make sure that the
   * RPC function has finished using it by simply acquiring a lock to it. */
  sig_get(&event_signal);
 }
 return error != TASK_SETCPU_FAIL;
}



PUBLIC bool KCALL
task_setaffinity(struct task *__restrict thread,
                 kernel_cpuset_t const affinity) {
 struct cpu *new_cpu = NULL;
 cpuid_t id; bool result = true;
 /* Determine what should be the new CPU. */
 KERNEL_CPUSET_FOREACH(affinity,id) {
  if (!new_cpu)
       new_cpu = cpu_vector[id];
  else {
   /* TODO: Compare CPUs based on load? */
  }
 }
 /* No suitable CPU found. */
 if unlikely(!new_cpu)
    return false;
 atomic_rwlock_write(&FORTASK(thread,_this_affinity_lock));
 if (!kernel_cpuset_has(affinity,thread->t_cpu->cpu_id)) {
  /* Set the thread's new CPU if the old one isn't allowed anymore. */
  result = task_setcpu(thread,new_cpu,true);
 }
 /* Copy the new affinity. */
 if (result)
     memcpy(FORTASK(thread,_this_affinity),
            affinity,sizeof(kernel_cpuset_t));
 atomic_rwlock_endwrite(&FORTASK(thread,_this_affinity_lock));
 return result;
}

#endif


DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_AFFINITY_C */
