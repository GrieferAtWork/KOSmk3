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
#ifndef GUARD_KERNEL_SRC_SCHED_GROUP_C
#define GUARD_KERNEL_SRC_SCHED_GROUP_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <kos/types.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/bind.h>
#include <kernel/user.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <kernel/sections.h>
#include <kernel/syscall.h>
#include <sched/group.h>
#include <sched/taskref.h>
#include <sched/posix_signals.h>
#include <sched/pid.h>
#include <bits/signum.h>
#include <errno.h>
#include <assert.h>
#include <except.h>
#include <string.h>
#include <bits/siginfo.h>
#include <bits/sched.h>
#include <bits/resource.h>

#undef CONFIG_LOG_GROUP_ADD_DELETE
#if !defined(NDEBUG) && 0
#define CONFIG_LOG_GROUP_ADD_DELETE 1
#endif


DECL_BEGIN

/* The thread-group descriptor for the calling thread. */
PUBLIC ATTR_PERTASK struct threadgroup _this_group = {
    .tg_leader = NULL,
    .tg_process = {
        .h_lock   = ATOMIC_RWLOCK_INIT,
        .h_parent = NULL,
        .h_sigcld = SIGCHLD
    },
};

DEFINE_PERTASK_CLONE(threadgroup_clone);
INTERN void KCALL
threadgroup_clone(struct task *__restrict new_thread, u32 flags) {
 struct threadgroup *new_group;
 new_group = &FORTASK(new_thread,_this_group);
#if 1
 if (flags & CLONE_THREAD) {
  /* Setup the new thread as a member of the caller's thread-group. */
  assertf(!FORTASK(new_thread,_this_group).tg_thread.g_group.le_pself,
          "new_thread                                                  = %p\n"
          "&FORTASK(new_thread,_this_group).tg_thread.g_group.le_pself = %p\n"
          "FORTASK(new_thread,_this_group).tg_thread.g_group.le_pself  = %p\n",
           new_thread,
          &FORTASK(new_thread,_this_group).tg_thread.g_group.le_pself,
           FORTASK(new_thread,_this_group).tg_thread.g_group.le_pself);
  new_group->tg_leader = get_this_process();
  task_incref(new_group->tg_leader);
  assert(new_group->tg_leader != new_thread);
 } else
#endif
 {
  struct processgroup *mygroup;
  struct processgroup *pgroup;
  struct processgroup *leadergroup;
  if unlikely((flags & CSIGNAL) >= _NSIG+1)
     error_throw(E_INVALID_ARGUMENT);
  /* Setup the new thread as a child-process of the caller. */
  if (flags & CLONE_PARENT) {
   /* Re-use the parent of the current process. */
   new_group->tg_process.h_parent = FORTASK(get_this_process(),_this_group).tg_process.h_parent;
   if (new_group->tg_process.h_parent)
    task_weakref_incref(new_group->tg_process.h_parent);
   else {
    /* If the calling thread (the original thread) doesn't
     * have a parent, overrule the `CLONE_PARENT' flag and
     * re-use the current process as parent. */
    new_group->tg_process.h_parent = task_getweakref(get_this_process());
   }
  } else {
   /* Use the current process as parent. */
   new_group->tg_process.h_parent = task_getweakref(get_this_process());
  }
  new_group->tg_leader           = new_thread;
  new_group->tg_process.h_sigcld = flags & CSIGNAL;
  /* The thread starts out within the same process group as the parent. */
  pgroup  = &new_group->tg_process.h_procgroup;
  mygroup = &FORTASK(get_this_process(),_this_group).tg_process.h_procgroup;
  atomic_rwlock_cinit(&pgroup->pg_lock);
  atomic_rwlock_read(&mygroup->pg_lock);
  pgroup->pg_leader = mygroup->pg_leader;
  task_incref(pgroup->pg_leader);
  atomic_rwlock_endread(&mygroup->pg_lock);
  assert(FORTASK(pgroup->pg_leader,_this_group).tg_leader == pgroup->pg_leader);
  /* Add the new thread to that same process thread. */
  leadergroup = &FORTASK(pgroup->pg_leader,_this_group).tg_process.h_procgroup;
  atomic_rwlock_write(&leadergroup->pg_lock);
  pgroup->pg_slave.m_members.le_pself = &leadergroup->pg_master.m_members;
  pgroup->pg_slave.m_members.le_next = leadergroup->pg_master.m_members;
  if (pgroup->pg_slave.m_members.le_next) {
   struct processgroup *next_group;
   next_group = &FORTASK(pgroup->pg_slave.m_members.le_next,_this_group).tg_process.h_procgroup;
   next_group->pg_slave.m_members.le_pself = &pgroup->pg_slave.m_members.le_next;
  }
  atomic_rwlock_endwrite(&leadergroup->pg_lock);
 }
}


PUBLIC REF struct task *KCALL task_get_parent(void) {
 struct task_weakref *ref;
 ref = FORTASK(get_this_process(),_this_group).tg_process.h_parent;
 return ref ? task_weakref_lock(ref) : NULL;
}


PRIVATE void KCALL exit_rpc(void *status) {
 struct exception_info *reason;
 reason = error_info();
 reason->e_error.e_code = E_EXIT_THREAD;
 reason->e_error.e_flag = ERR_FNORMAL;
 memset(reason->e_error.e_pointers,0,sizeof(reason->e_error.e_pointers));
 reason->e_error.e_exit.e_status = (int)(unsigned int)(uintptr_t)status;
 error_throw_current();
}

PUBLIC void KCALL
task_exit_secondary_threads(int exit_status) {
 assertf(get_this_process() == THIS_TASK,
         "You're not the leader of a thread-group");
 for (;;) {
  struct task *EXCEPT_VAR thread;
  atomic_rwlock_write(&PERTASK(_this_group.tg_process.h_lock));
  thread = PERTASK_GET(_this_group.tg_process.h_group);
  assertf(!thread ||
           FORTASK(thread,_this_group).tg_thread.g_group.le_pself ==
          &PERTASK(_this_group.tg_process.h_group),
          "FORTASK(thread,_this_group).tg_thread.g_group.le_pself = %p\n"
          "&PERTASK(_this_group.tg_process.h_group)               = %p\n"
          ,FORTASK(thread,_this_group).tg_thread.g_group.le_pself
          ,&PERTASK(_this_group.tg_process.h_group));
  while (thread &&
        !TASK_ISTERMINATING(thread) &&
        !ATOMIC_INCIFNONZERO(thread->t_refcnt))
         assert(!FORTASK(thread,_this_group).tg_thread.g_group.le_next ||
                 FORTASK(FORTASK(thread,_this_group).tg_thread.g_group.le_next,_this_group).tg_thread.g_group.le_pself ==
                &FORTASK(thread,_this_group).tg_thread.g_group.le_next),
         thread = FORTASK(thread,_this_group).tg_thread.g_group.le_next;
  if (thread) {
   /* Remove the thread from the group of threads registered to us. */
   assert(FORTASK(thread,_this_group).tg_thread.g_group.le_pself);
#ifdef CONFIG_LOG_GROUP_ADD_DELETE
   debug_printf("GROUP_DEL(%p)\n",thread);
#endif
   if ((*FORTASK(thread,_this_group).tg_thread.g_group.le_pself =
         FORTASK(thread,_this_group).tg_thread.g_group.le_next) != NULL) {
    struct threadgroup *next_group = &FORTASK(FORTASK(thread,_this_group).tg_thread.g_group.le_next,_this_group);
    next_group->tg_thread.g_group.le_pself = FORTASK(thread,_this_group).tg_thread.g_group.le_pself;
   }
   FORTASK(thread,_this_group).tg_thread.g_group.le_pself = NULL;
#if 0 /* No necessary */
   FORTASK(thread,_this_group).tg_thread.g_group.le_next  = NULL;
#endif
  }
  atomic_rwlock_endwrite(&PERTASK(_this_group.tg_process.h_lock));
  if (!thread) break; /* All threads have been terminated, or are in the process of. */
  TRY {
   /* Queue an RPC callback that will terminate this thread. */
   task_queue_rpc(thread,&exit_rpc,
                 (void *)(uintptr_t)(unsigned int)exit_status,
                  TASK_RPC_SYNC);
  } FINALLY {
   task_decref(thread);
  }
 }
}


DEFINE_PERTASK_CLEANUP(threadgroup_cleanup);
INTERN void KCALL threadgroup_cleanup(void) {
 struct exception_info *info = error_info();
 int status;
 if (info->e_error.e_code == E_EXIT_PROCESS ||
     info->e_error.e_code == E_EXIT_THREAD) {
  status = info->e_error.e_exit.e_status;
 } else {
  status = __W_EXITCODE(0,0);
 }
 /* Signal that the thread has now exited. */
 task_signal_event(__WIFEXITED(status) ? CLD_EXITED :
                   __WCOREDUMP(status) ? CLD_DUMPED :
                                         CLD_KILLED,
                   status);

 /* Called just prior to a thread that was part of the thread group exiting. */
 if (get_this_process() == THIS_TASK) {
  /* Thread group leader has died.
   * Since the caller has set the `TASK_STATE_FTERMINATING' flag for
   * us, we can be sure that no new threads will appear while we're
   * terminating all threads that are still remaining using RPC functions. */
  task_exit_secondary_threads(error_info()->e_error.e_exit.e_status);
  for (;;) {
   REF struct thread_pid *child,*next;
   /* Drop a reference from all children that weren't joined. */
   sig_get(&PERTASK(_this_group.tg_process.h_cldevent));
   child = PERTASK_GET(_this_group.tg_process.h_children); /* Inherit reference (if non-NULL). */
   if (!child) { sig_put(&PERTASK(_this_group.tg_process.h_cldevent)); break; }
   assert(child->tp_siblings.le_pself == &PERTASK(_this_group.tg_process.h_children));
   next = child->tp_siblings.le_next;
   PERTASK_SET(_this_group.tg_process.h_children,next);
   if (next) next->tp_siblings.le_pself = &PERTASK(_this_group.tg_process.h_children);
   child->tp_siblings.le_pself = NULL;
   child->tp_siblings.le_next  = NULL;
   sig_put(&PERTASK(_this_group.tg_process.h_cldevent));
   thread_pid_decref(child);
  }
 } else {
  /* Not the leader of the thread group. */
  struct threadgroup *leader;
  leader = &FORTASK(get_this_process(),_this_group);
  atomic_rwlock_write(&leader->tg_process.h_lock);
  /* Remove the thread from its group. */
  if (PERTASK_TEST(_this_group.tg_thread.g_group.le_pself)) {
   struct task *next;
#ifdef CONFIG_LOG_GROUP_ADD_DELETE
   debug_printf("GROUP_DEL(%p)\n",THIS_TASK);
#endif
   next = PERTASK_GET(_this_group.tg_thread.g_group.le_next);
   *PERTASK_GET(_this_group.tg_thread.g_group.le_pself) = next;
   if (next) {
    struct threadgroup *next_group;
    next_group = &FORTASK(PERTASK_GET(_this_group.tg_thread.g_group.le_next),_this_group);
    next_group->tg_thread.g_group.le_pself = PERTASK_GET(_this_group.tg_thread.g_group.le_pself);
   }
   PERTASK_SET(_this_group.tg_thread.g_group.le_pself,NULL);
   PERTASK_SET(_this_group.tg_thread.g_group.le_next,NULL);
  }
  atomic_rwlock_endwrite(&leader->tg_process.h_lock);
 }
}



DEFINE_PERTASK_FINI(threadgroup_fini);
INTERN void KCALL
threadgroup_fini(struct task *__restrict thread) {
 struct threadgroup *group = &FORTASK(thread,_this_group);
 if (!group->tg_leader); /* Thread wasn't fully constructed. */
 else if (group->tg_leader != thread) {
  struct threadgroup *leader;
  leader = &FORTASK(group->tg_leader,_this_group);
  atomic_rwlock_write(&leader->tg_process.h_lock);
  /* Remove the thread from its group. */
  if (group->tg_thread.g_group.le_pself) {
#ifdef CONFIG_LOG_GROUP_ADD_DELETE
   debug_printf("GROUP_DEL(%p)\n",thread);
#endif
   if ((*group->tg_thread.g_group.le_pself = group->tg_thread.g_group.le_next) != NULL) {
    struct threadgroup *next_group = &FORTASK(group->tg_thread.g_group.le_next,_this_group);
    next_group->tg_thread.g_group.le_pself = &group->tg_thread.g_group.le_next;
   }
  }
  atomic_rwlock_endwrite(&leader->tg_process.h_lock);
  if (group->tg_process.h_parent)
      task_weakref_decref(group->tg_process.h_parent);
  task_decref(group->tg_leader);
 } else {
  REF struct task *leader;
  struct processgroup *pgroup;
  assertf(!group->tg_process.h_group,
          "Thread group members should have kept us "
          "alive with their `tg_leader' pointers");
  pgroup = &group->tg_process.h_procgroup;
  /* Reminder: When `pgroup->pg_leader' points to the associated
   *           thread, then it doesn't carry a reference. */
  if ((leader = pgroup->pg_leader) != NULL) {
   if (leader == thread) {
    assertf(!pgroup->pg_master.m_members,
            "Process group members should have kept "
            "us alive with their `pg_leader' pointers");
    /* Drop a reference from the process's session (if that was ever assigned) */
    if (pgroup->pg_master.m_session &&
        pgroup->pg_master.m_session != thread) {
     /* XXX: Remove from the chain of process groups found in this session? */
     task_decref(pgroup->pg_master.m_session);
    }
   } else {
    struct processgroup *leadergroup;
    /* Remove this process from its process group. */
    assert(FORTASK(leader,_this_group).tg_leader == leader);
    leadergroup = &FORTASK(leader,_this_group).tg_process.h_procgroup;
    atomic_rwlock_write(&leadergroup->pg_lock);
    /* Remove from the old group */
    if ((*pgroup->pg_slave.m_members.le_pself = pgroup->pg_slave.m_members.le_next) != NULL) {
     struct processgroup *next_group;
     next_group = &FORTASK(pgroup->pg_slave.m_members.le_next,_this_group).tg_process.h_procgroup;
     next_group->pg_slave.m_members.le_pself = pgroup->pg_slave.m_members.le_pself;
    }
    atomic_rwlock_endwrite(&leadergroup->pg_lock);
    /* Drop the reference from `pgroup->pg_leader' */
    task_decref(leader);
   }
  }
 }
}



/* Called in the context of a newly created user-space thread
 * when `CLONE_THREAD' has been used to add the thread to the
 * thread-group of the calling thread.
 * This function will safely check if the thread group leader
 * has, or is terminating and throw an `E_EXIT_THREAD' error
 * if it is (so-as to shut down the calling thread). Otherwise
 * it will add the calling thread to the chain of threads that
 * are apart of the associated thread group's leader.
 * This function requires the following fields to be initialized:
 *   - THIS_GROUP.tg_leader
 * This function will initialize the following fields:
 *   - THIS_GROUP.tg_thread.g_group  (As an entry in `THIS_GROUP.tg_leader->...->tg_process.h_group')
 */
DEFINE_PERTASK_STARTUP(task_startup_group);
INTERN void KCALL task_startup_group(u32 UNUSED(flags)) {
 struct threadgroup *leader_group;
 assert(get_this_process());
 leader_group = &FORTASK(get_this_process(),_this_group);
 atomic_rwlock_write(&leader_group->tg_process.h_lock);
 /* Check if the leader is terminating. */
 if unlikely(TASK_ISTERMINATING(get_this_process())) {
  struct exception_info *reason;
  atomic_rwlock_endwrite(&leader_group->tg_process.h_lock);
  /* Just exit the thread if the leader is dead. */
  reason                          = error_info();
  reason->e_error.e_code          = E_EXIT_THREAD;
  reason->e_error.e_flag          = ERR_FNORMAL;
  reason->e_error.e_exit.e_status = get_this_process()->t_segment.ts_xcurrent.e_error.e_exit.e_status;
  error_throw_current();
  __builtin_unreachable();
 }
#if 1
 if (leader_group != &THIS_GROUP) {
  assertf(!PERTASK_TEST(_this_group.tg_thread.g_group.le_pself),
          "The calling thread was already added to a group\n"
          "THIS_TASK                             = %p\n"
          "leader_group                          = %p\n"
          "THIS_GROUP                            = %p\n"
          "THIS_GROUP.tg_thread.g_group.le_pself = %p",
          THIS_TASK,leader_group,&THIS_GROUP,
          THIS_GROUP.tg_thread.g_group.le_pself);
#ifdef CONFIG_LOG_GROUP_ADD_DELETE
  debug_printf("GROUP_ADD(%p)\n",THIS_TASK);
#endif
  /* Add the calling thread to its associated group. */
#if 1
#define GROUP_PATH(p) FORTASK(p,_this_group).tg_thread.g_group
  LIST_INSERT_P(leader_group->tg_process.h_group,THIS_TASK,GROUP_PATH);
#else
  PERTASK_SET(_this_group.tg_thread.g_group.le_next,
              leader_group->tg_process.h_group);
  if (leader_group->tg_process.h_group != NULL)
      FORTASK(leader_group->tg_process.h_group,_this_group).tg_thread.g_group.le_pself = &PERTASK(_this_group.tg_thread.g_group.le_next);
  PERTASK_SET(_this_group.tg_thread.g_group.le_pself,&leader_group->tg_process.h_group);
  leader_group->tg_process.h_group = THIS_TASK;
#endif
 }
#endif
 atomic_rwlock_endwrite(&leader_group->tg_process.h_lock);
}


#define GROUP_NEXT(p) FORTASK(p,_this_group).tg_thread.g_group.le_next

FUNDEF size_t KCALL
taskgroup_queue_rpc(task_rpc_t func, void *arg,
                    unsigned int mode) {
 size_t result = 0;
 size_t min_threads = 0;
 size_t EXCEPT_VAR num_threads;
 struct task *iter;
 struct task **EXCEPT_VAR thread_vec = NULL;
 assert(get_this_process() == THIS_TASK);
 TRY {
  for (;;) {
again:
   num_threads = 0;
   atomic_rwlock_read(&PERTASK(_this_group.tg_process.h_lock));
   iter = PERTASK_GET(_this_group.tg_process.h_group);
   for (; iter; iter = GROUP_NEXT(iter)) ++num_threads;
   atomic_rwlock_endread(&PERTASK(_this_group.tg_process.h_lock));
   if (!num_threads) break; /* No threads to signal! */
   if (num_threads > min_threads) {
    thread_vec = (struct task **)krealloc(thread_vec,num_threads*
                                          sizeof(struct task *),
                                          GFP_SHARED);
    min_threads = num_threads;
   }
   atomic_rwlock_read(&PERTASK(_this_group.tg_process.h_lock));
   num_threads = 0;
   /* Collect all threads. */
   iter = PERTASK_GET(_this_group.tg_process.h_group);
   for (; iter; iter = GROUP_NEXT(iter)) {
    if (TASK_ISTERMINATING(iter)) continue;
    if (!ATOMIC_INCIFNONZERO(iter->t_refcnt)) continue;
    if unlikely(num_threads == min_threads) {
     atomic_rwlock_endread(&PERTASK(_this_group.tg_process.h_lock));
     task_decref(iter);
     while (num_threads--)
         task_decref(thread_vec[num_threads]);
     goto again;
    }
    thread_vec[num_threads++] = iter;
   }
   atomic_rwlock_endread(&PERTASK(_this_group.tg_process.h_lock));

   /* With a snapshot of all threads now at hand, send an RPC to each of them. */
   TRY {
    size_t i;
    for (i = 0; i < num_threads; ++i) {
     if (task_queue_rpc(thread_vec[i],func,arg,mode))
         ++result;
    }
   } FINALLY {
    while (num_threads--)
        task_decref(thread_vec[num_threads]);
   }
   break;
  }
 } FINALLY {
  kfree(thread_vec);
 }
 return result;
}


PUBLIC ATTR_NOTHROW ATTR_RETNONNULL REF struct task *KCALL
task_get_group(struct task *__restrict thread) {
 REF struct task *result;
 struct processgroup *pgroup;
 /* Dereference to get the thread-group leader of `thread'. */
 thread = FORTASK(thread,_this_group).tg_leader;
 pgroup = &FORTASK(thread,_this_group).tg_process.h_procgroup;
 atomic_rwlock_read(&pgroup->pg_lock);
 result = pgroup->pg_leader;
 assert(result->t_refcnt != 0);
 task_incref(result);
 atomic_rwlock_endread(&pgroup->pg_lock);
 return result;
}
PUBLIC ATTR_NOTHROW void KCALL
task_set_group(struct task *__restrict thread,
               struct task *__restrict new_group) {
 REF struct task *old_group;
 struct processgroup *pgroup;
 struct processgroup *old_pgroup;
 struct processgroup *new_pgroup;
 REF struct task *old_session_leader = NULL;
 /* Dereference to get the thread-group leader of `thread' and `group'. */
 thread    = FORTASK(thread,_this_group).tg_leader;
 new_group = FORTASK(new_group,_this_group).tg_leader;
 pgroup    = &FORTASK(thread,_this_group).tg_process.h_procgroup;
 assert(thread->t_refcnt != 0);
 assert(new_group->t_refcnt != 0);
 atomic_rwlock_write(&pgroup->pg_lock);
 old_group = pgroup->pg_leader;
 if (new_group != thread)
     task_incref(new_group);
 if (new_group != old_group) {
  new_pgroup = &FORTASK(new_group,_this_group).tg_process.h_procgroup;
  old_pgroup = &FORTASK(old_group,_this_group).tg_process.h_procgroup;

  if (old_group != thread) {
   /* Remove from the old group */
   atomic_rwlock_write(&old_pgroup->pg_lock);
   if ((*pgroup->pg_slave.m_members.le_pself = pgroup->pg_slave.m_members.le_next) != NULL) {
    struct processgroup *next_group;
    next_group = &FORTASK(pgroup->pg_slave.m_members.le_next,_this_group).tg_process.h_procgroup;
    next_group->pg_slave.m_members.le_pself = pgroup->pg_slave.m_members.le_pself;
   }
   old_session_leader = old_pgroup->pg_master.m_session;
   assert(old_session_leader);
   task_incref(old_session_leader);
   atomic_rwlock_endwrite(&old_pgroup->pg_lock);
  } else {
   /* XXX: Remove `old_group' from the chain of process groups associated
    *      with the session in `old_pgroup->pg_master.m_session'? */
  }

  if (new_group != thread) {
   /* Add to the new group */
   atomic_rwlock_write(&new_pgroup->pg_lock);
   pgroup->pg_slave.m_members.le_pself = &new_pgroup->pg_master.m_members;
   pgroup->pg_slave.m_members.le_next  = new_pgroup->pg_master.m_members;
   if (pgroup->pg_slave.m_members.le_next != NULL) {
    struct processgroup *next_group;
    next_group = &FORTASK(pgroup->pg_slave.m_members.le_next,_this_group).tg_process.h_procgroup;
    next_group->pg_slave.m_members.le_pself = &pgroup->pg_slave.m_members.le_next;
   }
   atomic_rwlock_endwrite(&new_pgroup->pg_lock);
  } else {
   /* Leader of the new group. (Add to session) */
   assert(old_session_leader);
   pgroup->pg_master.m_members = NULL;
   pgroup->pg_master.m_session = old_session_leader;
   /* XXX: Add to the chain of process groups of this session? */
   old_session_leader = NULL;
  }
  /* Set the new group leader in the associated group descriptor. */
  pgroup->pg_leader = new_group;
 }
 atomic_rwlock_endwrite(&pgroup->pg_lock);
 if (old_session_leader)
     task_decref(old_session_leader);
 if (old_group != thread)
     task_decref(old_group);
}


struct threadlist {
    size_t            tl_size;    /* Number of used thread pointers. */
    size_t            tl_alloc;   /* Allocated number of thread pointers. */
    REF struct task **tl_threads; /* [1..1][0..tl_size|ALLOC(tl_alloc)][owned] Vector of threads. */
};

#define THREADLIST_INIT  {0,0,NULL}

LOCAL void KCALL
threadlist_fini(struct threadlist EXCEPT_VAR *__restrict self) {
 size_t i;
 for (i = 0; i < self->tl_size; ++i)
     task_decref(self->tl_threads[i]);
 kfree(self->tl_threads);
}
LOCAL bool KCALL
threadlist_contains(struct threadlist EXCEPT_VAR *__restrict self,
                    struct task *__restrict thread) {
 size_t i;
 for (i = 0; i < self->tl_size; ++i)
     if (self->tl_threads[i] == thread)
         return true;
 return false;
}
LOCAL bool KCALL
threadlist_append(struct threadlist EXCEPT_VAR *__restrict self,
                  struct task *__restrict thread) {
 if (self->tl_size == self->tl_alloc)
     return false;
 self->tl_threads[self->tl_size++] = thread;
 return true;
}
LOCAL void KCALL
threadlist_reserve(struct threadlist EXCEPT_VAR *__restrict self,
                   size_t num_slots) {
 size_t new_alloc = self->tl_alloc+num_slots;
 /* Allocate more memory. */
 self->tl_threads = (REF struct task **)krealloc(self->tl_threads,
                                                 new_alloc*sizeof(REF struct task *),
                                                 GFP_SHARED);
 self->tl_alloc = new_alloc;
}


PUBLIC size_t KCALL
signal_raise_pgroup(struct task *__restrict processgroup,
                    USER CHECKED siginfo_t *__restrict info) {
 struct task *EXCEPT_VAR xprocessgroup = processgroup;
 size_t result = 0;
 struct processgroup *pgroup;
 processgroup = FORTASK(processgroup,_this_group).tg_leader;
 pgroup = &FORTASK(processgroup,_this_group).tg_process.h_procgroup;
 atomic_rwlock_read(&pgroup->pg_lock);
 processgroup = pgroup->pg_leader;
 task_incref(processgroup);
 atomic_rwlock_endread(&pgroup->pg_lock);
 {
  struct threadlist EXCEPT_VAR threads = THREADLIST_INIT;
  TRY {
   struct task *iter; size_t req_threads,start;
   pgroup = &FORTASK(processgroup,_this_group).tg_process.h_procgroup;
   /* Start out by raising the signal in the leader of the process group.
    * In most cases of job control, that'll already be enough. */
   if (signal_raise_group(processgroup,info)) ++result;
   /* Must collect all processes apart of the group and send
    * the signal to every one of them until we find no more
    * threads that still haven't gotten the signal. */
   for (;;) {
    req_threads = 0;
    atomic_rwlock_read(&pgroup->pg_lock);
    for (iter = pgroup->pg_master.m_members; iter;
         iter = FORTASK(iter,_this_group).
         tg_process.h_procgroup.pg_slave.m_members.le_next) {
     if (ATOMIC_READ(iter->t_refcnt) != 0 &&
        !threadlist_contains(&threads,iter))
         ++req_threads;
    }
    atomic_rwlock_endread(&pgroup->pg_lock);
    if (!req_threads) break; /* Nothing left to do! (highly likely case for single-process jobs) */
    if (!threads.tl_size) ++req_threads; /* Keep track of the first thread. */
    /* Allocate sufficient space for threads that haven't been signaled, yet. */
    threadlist_reserve(&threads,req_threads);
    if (!threads.tl_size) {
     threads.tl_threads[threads.tl_size++] = processgroup; /* Inherit reference. */
     processgroup = NULL; /* Steal reference. */
    }
    /* Figure out where new threads will appear. */
    start = threads.tl_size;
    atomic_rwlock_read(&pgroup->pg_lock);
    for (iter = pgroup->pg_master.m_members;
         iter; iter = FORTASK(iter,_this_group).
         tg_process.h_procgroup.pg_slave.m_members.le_next) {
     if (threadlist_contains(&threads,iter))
         continue; /* Skip threads already handled. */
     if (!threadlist_append(&threads,iter))
         break; /* Skip if we can't add any more threads. */
     if (!ATOMIC_INCIFNONZERO(iter->t_refcnt)) {
      --threads.tl_size; /* Remove the thread we've added before. */
      continue; /* Skip dead threads. */
     }
    }
    atomic_rwlock_endread(&pgroup->pg_lock);
    /* Send the signal to all newly added threads. */
    for (; start < threads.tl_size; ++start) {
     if (signal_raise_group(threads.tl_threads[start],info))
         ++result;
    }
   }
  } FINALLY {
   threadlist_fini(&threads);
   if (xprocessgroup)
       task_decref(xprocessgroup);
  }
 }
 return result;
}


PUBLIC ATTR_NOTHROW ATTR_RETNONNULL REF struct task *
KCALL task_get_session(struct task *__restrict thread) {
 struct processgroup *pgroup;
 REF struct task *result,*leader;
 thread = FORTASK(thread,_this_group).tg_leader;
 pgroup = &FORTASK(thread,_this_group).tg_process.h_procgroup;
 /* Extract the current process group leader. */
 atomic_rwlock_read(&pgroup->pg_lock);
 leader = pgroup->pg_leader; /* Process group-leader */
 task_incref(leader);
 atomic_rwlock_endread(&pgroup->pg_lock);
 /* Extract the associated session. */
 assert(FORTASK(leader,_this_group).tg_leader == leader);
 pgroup = &FORTASK(leader,_this_group).tg_process.h_procgroup;
 atomic_rwlock_read(&pgroup->pg_lock);
 result = pgroup->pg_master.m_session; /* Session leader. */
 task_incref(result);
 atomic_rwlock_endread(&pgroup->pg_lock);
 /* Drop the temporary reference from the process group leader. */
 task_decref(leader);
 return result;
}

PUBLIC ATTR_NOTHROW bool KCALL task_set_session(void) {
 struct processgroup *pgroup;
 struct processgroup *old_pgroup;
 REF struct task *old_leader;
 pgroup = &FORTASK(get_this_process(),_this_group).tg_process.h_procgroup;
 atomic_rwlock_write(&pgroup->pg_lock);
 old_leader = pgroup->pg_leader;
 if (pgroup->pg_leader == get_this_process()) {
  /* The current process is already the leader of a process
   * group. Posix mandates that we fail in this scenario. */
  atomic_rwlock_endwrite(&pgroup->pg_lock);
  return false;
 }
 /* Remove the calling process from its old process group. */
 assert(FORTASK(old_leader,_this_group).tg_leader == old_leader);
 old_pgroup = &FORTASK(old_leader,_this_group).tg_process.h_procgroup;
 atomic_rwlock_write(&old_pgroup->pg_lock);
 /* Remove from the old group */
 if ((*pgroup->pg_slave.m_members.le_pself = pgroup->pg_slave.m_members.le_next) != NULL) {
  struct processgroup *next_group;
  next_group = &FORTASK(pgroup->pg_slave.m_members.le_next,_this_group).tg_process.h_procgroup;
  next_group->pg_slave.m_members.le_pself = pgroup->pg_slave.m_members.le_pself;
 }
 atomic_rwlock_endwrite(&old_pgroup->pg_lock);
 /* Set the process as the leader of its own group and session. */
 pgroup->pg_leader           = get_this_process();
 pgroup->pg_master.m_session = pgroup->pg_leader;
 /* XXX: Clear the chain of sessions members? */
 pgroup->pg_master.m_members = NULL; /* No additional members, yet. */
 atomic_rwlock_endwrite(&pgroup->pg_lock);
 /* Drop a reference from the old leader (inherited from `pgroup->pg_leader') */
 task_decref(old_leader);
 return true;
}



/* Define system calls for process group / session control. */
DEFINE_SYSCALL_MUSTRESTART(setpgid);
DEFINE_SYSCALL2(setpgid,pid_t,pid,pid_t,pgid) {
 REF struct task *EXCEPT_VAR thread;
 REF struct task *leader;
 if (pid == 0) {
  thread = THIS_TASK;
  task_incref(thread);
 } else {
  thread = pid_lookup_task(pid);
 }
 TRY {
  if (pgid == 0) {
   /* Set the given process to be its own leader. */
   leader = thread;
   task_incref(leader);
  } else {
   leader = pid_lookup_task(pid);
  }
  /* Set the leader process for the process associated with the given thread.
   * NOTE: The additional thread -> process indirection is performed by `task_set_group()' */
  task_set_group(thread,leader);
  task_decref(leader);
 } FINALLY {
  task_decref(thread);
 }
 return 0;
}
DEFINE_SYSCALL_MUSTRESTART(getpgid);
DEFINE_SYSCALL1(getpgid,pid_t,pid) {
 pid_t result;
 REF struct task *leader,*temp;
 if (pid == 0) {
  leader = task_get_group(THIS_TASK);
 } else {
  temp = pid_lookup_task(pid);
  leader = task_get_group(temp);
  task_decref(temp);
 }
 /* View the process group leader's TID in
  * the calling thread's PID namespace. */
 result = posix_gettid_view(leader);
 task_decref(leader);
 return result;
}
DEFINE_SYSCALL_MUSTRESTART(getsid);
DEFINE_SYSCALL1(getsid,pid_t,pid) {
 pid_t result;
 REF struct task *leader,*temp;
 /* Lookup the session leader of the
  * thread matching the given PID. */
 if (pid == 0) {
  leader = task_get_session(THIS_TASK);
 } else {
  temp = pid_lookup_task(pid);
  leader = task_get_session(temp);
  task_decref(temp);
 }
 /* View the session leader's TID in the
  * calling thread's PID namespace. */
 result = posix_gettid_view(leader);
 task_decref(leader);
 return result;
}
DEFINE_SYSCALL_MUSTRESTART(setsid);
DEFINE_SYSCALL0(setsid) {
 if (!task_set_session())
      return -EPERM;
 return posix_getpid();
}


INTERN void KCALL
get_rusage(struct thread_pid *__restrict pid,
           USER CHECKED struct rusage *ru) {
 (void)pid; /* XXX: implement this? */
 memset(ru,0,sizeof(struct rusage));
}


PRIVATE pid_t KCALL
posix_waitfor(int which, pid_t upid,
              USER CHECKED int *wstatus,
              USER CHECKED siginfo_t *infop, int options,
              USER CHECKED struct rusage *ru) {
 struct task *my_process;
 struct threadgroup *my_group;
 struct thread_pid *EXCEPT_VAR child;
 size_t my_indirection;
 size_t num_candidates;
 my_process     = get_this_process();
 my_group       = &FORTASK(my_process,_this_group);
 my_indirection = FORTASK(my_process,_this_pid) ? FORTASK(my_process,_this_pid)->tp_ns->pn_indirection : 0;
 /* Posix wants us to do some special handling when SIGCLD is being ignored... */
 {
  struct sighand *myhand = sighand_lock_read();
  if (myhand) {
   if (myhand->sh_hand[SIGCLD] &&
      (myhand->sh_hand[SIGCLD]->sa_handler == SIG_IGN ||
      (myhand->sh_hand[SIGCLD]->sa_flags & SA_NOCLDWAIT))) {
    atomic_rwlock_endread(&myhand->sh_lock);
    /* Block until all child processes have exited
     * and reap all zombies created in the mean time.
     * Then, fail with -ECHILD. */
reapall_again:
    task_connect(&my_group->tg_process.h_cldevent);
reapall_check:
    sig_get(&my_group->tg_process.h_cldevent);
    child = my_group->tg_process.h_children;
    if (!child) {
     sig_put(&my_group->tg_process.h_cldevent);
     if (task_disconnect()) goto reapall_again;
     return -ECHILD;
    }
    if (!(options & WNOREAP)) {
     for (; child; child = child->tp_siblings.le_next) {
      if (!child->tp_task ||
          !ATOMIC_READ(child->tp_task->t_refcnt) ||
           TASK_ISTERMINATED(child->tp_task)) {
       /* Detach (reap) this child. */
       LIST_REMOVE(child,tp_siblings); /* Inherit reference. */
       child->tp_siblings.le_pself = NULL;
       child->tp_siblings.le_next  = NULL;
       sig_put(&my_group->tg_process.h_cldevent);
       thread_pid_decref(child);
       goto reapall_check;
      }
     }
    }
    sig_put(&my_group->tg_process.h_cldevent);
    if (options & WNOHANG) {
     if (task_disconnect())
         goto reapall_again;
     return -ECHILD; /* I feel like this should be EAGAIN, but POSIX
                      * says that wait() doesn't return that error... */
    }
    /* Wait for more state-change signals. */
    task_wait();
    goto reapall_again;
   }
   atomic_rwlock_endread(&myhand->sh_lock);
  }
 }

again:
 /* Always connect to the child-event signal that
  * child processes broadcast when they change state. */
 task_connect(&my_group->tg_process.h_cldevent);
 num_candidates = 0;

 /* Enumerate child processes. */
 sig_get(&my_group->tg_process.h_cldevent);
 for (child = my_group->tg_process.h_children,
      assert(!child ||
              child->tp_siblings.le_pself == &my_group->tg_process.h_children);
      child;
      assert(!child->tp_siblings.le_next ||
              child->tp_siblings.le_next->tp_siblings.le_pself == &child->tp_siblings.le_next),
      child = child->tp_siblings.le_next) {
  bool has_write_lock = false;
  assert(child->tp_siblings.le_pself);
  /* Check if this child process matches requested criteria. */
  if (which == P_PID) {
   if (child->tp_pids[my_indirection] != upid)
       continue;
  }
  atomic_rwlock_read(&child->tp_task_lock);
check_child_again:
  if ((options & WONLYTHREADS) && child->tp_task &&
       FORTASK(child->tp_task,_this_group).tg_leader != get_this_process()) {
   atomic_rwlock_endread(&child->tp_task_lock);
   continue; /* Not a thread of this process. */
  }
  assert(!child->tp_task ||
          FORTASK(child->tp_task,_this_pid) == child);
  /* XXX: We're not tracking the PGID in the PID descriptor,
   *      but apparently POSIX wants us to remember a thread's
   *      process group, even after the thread has died...
   *      As a kind-of crappy work-around, right now we simply
   *      ignore the child's PGID if the thread has already been
   *      destroyed. */
  if (which == P_PGID && child->tp_task) {
   struct processgroup *child_group;
   struct thread_pid *child_group_pid;
   pid_t thread_pgid;
   child_group = &FORTASK(FORTASK(child->tp_task,_this_group).tg_leader,_this_group).tg_process.h_procgroup;
   atomic_rwlock_read(&child_group->pg_lock);
   child_group_pid = FORTASK(child_group->pg_leader,_this_pid);
   thread_pgid     = child_group_pid ? child_group_pid->tp_pids[my_indirection] : 0;
   atomic_rwlock_endread(&child_group->pg_lock);
   /* Match process group IDs. */
   if (thread_pgid != upid) {
continue_next_child_unlock:
    if (has_write_lock)
         atomic_rwlock_endwrite(&child->tp_task_lock);
    else atomic_rwlock_endread(&child->tp_task_lock);
    continue;
   }
  }
  if (!child->tp_task ||
      !ATOMIC_READ(child->tp_task->t_refcnt) ||
       TASK_ISTERMINATED(child->tp_task)) {
   pid_t COMPILER_IGNORE_UNINITIALIZED(result);
child_died:
   /* The thread has died. */
   if (has_write_lock)
        atomic_rwlock_endwrite(&child->tp_task_lock);
   else atomic_rwlock_endread(&child->tp_task_lock);
   if (!(options & WEXITED)) continue; /* Don't wait for exited child processes. */
   /* Remove this child's zombie corpse when `WNOREAP' isn't set. */
   if (!(options & WNOREAP)) {
    LIST_REMOVE(child,tp_siblings); /* Inherit reference. */
    child->tp_siblings.le_pself = NULL;
    child->tp_siblings.le_next  = NULL;
   } else {
    thread_pid_incref(child);
   }
   sig_put(&my_group->tg_process.h_cldevent);
   TRY {
    siginfo_t exit_info;
    /* Disconnect from the state-change notification signal. */
    task_disconnect();
    /* Collect information in this child. */
    result = child->tp_pids[my_indirection];
    memset(&exit_info,0,sizeof(siginfo_t));
    exit_info.si_signo  = SIGCHLD;
    exit_info.si_code   = child->tp_event;
    exit_info.si_status = child->tp_status.w_status;
    if (exit_info.si_code != CLD_EXITED &&
        exit_info.si_code != CLD_KILLED &&
        exit_info.si_code != CLD_DUMPED) {
     /* Shouldn't get here? */
     exit_info.si_status = CLD_EXITED;
     exit_info.si_status = __W_EXITCODE(0,0);
    }
    exit_info.si_pid = result;
    exit_info.si_uid = 0; /* TODO */
    COMPILER_WRITE_BARRIER();
    if (wstatus) *wstatus = exit_info.si_status; /* CAUTION: SEGFAULT */
    if (infop) memcpy(infop,&exit_info,sizeof(siginfo_t)); /* CAUTION: SEGFAULT */
    if (ru) get_rusage(child,ru);
    COMPILER_WRITE_BARRIER();
   } FINALLY {
    thread_pid_decref(child);
   }
   return result;
  }
  if (child->tp_event == 0) {
   /* Track the number of potential child process to join.
    * If there are none, we mustn't wait and fail with -ECHILD. */
   ++num_candidates;
   goto continue_next_child_unlock;
  }
  if (!has_write_lock) {
   has_write_lock = true;
   if (!atomic_rwlock_upgrade(&child->tp_task_lock))
        goto check_child_again;
  }
  /* An event happened in this child.
   * Check if it matches our criteria (stop/continue). */
  if (((child->tp_event == CLD_CONTINUED) && (options & WCONTINUED)) ||
      ((child->tp_event == CLD_STOPPED) && (options & WSTOPPED))) {
   pid_t result; siginfo_t info;
   sig_put(&my_group->tg_process.h_cldevent);
   /* Collect information in this child. */
   result = child->tp_pids[my_indirection];
   memset(&info,0,sizeof(siginfo_t));
   info.si_signo   = SIGCHLD;
   info.si_code    = child->tp_event;
   info.si_status  = child->tp_status.w_status;
   info.si_pid     = result;
   info.si_uid     = 0; /* TODO */
   child->tp_event = 0; /* Continue the event. */
   atomic_rwlock_endwrite(&child->tp_task_lock);
   task_disconnect();
   COMPILER_WRITE_BARRIER();
   /* Copy information to user-space. */
   if (wstatus) *wstatus = info.si_status; /* CAUTION: SEGFAULT */
   if (infop) memcpy(infop,&info,sizeof(siginfo_t)); /* CAUTION: SEGFAULT */
   if (ru) get_rusage(child,ru);
   COMPILER_WRITE_BARRIER();
   return result;
  }
  /* Fallback: All other events (`CLD_EXITED', `CLD_KILLED', `CLD_DUMPED')
   *           are handled as the child having exited. */
  goto child_died;
 }
 sig_put(&my_group->tg_process.h_cldevent);

 /* All possible wait-candidates have been enumerated,
  * however none of them was found to have changed state.
  * Now we must either fail (when `WNOHANG' was set), or
  * wait for the signal that we've connected to above. */
 if ((options & WNOHANG) || !num_candidates) {
  if (task_disconnect())
      goto again; /* Try again, because doing so won't ~hang~ (block) */
  return -ECHILD;
 }
 /* Wait for the signal to be broadcast. */
 task_wait();
 goto again;
}

DEFINE_SYSCALL_DONTRESTART(waitid);
DEFINE_SYSCALL5(waitid,int,which,pid_t,upid,
                USER UNCHECKED siginfo_t *,infop,int,options,
                USER UNCHECKED struct rusage *,ru) {
 if (options & ~(WNOHANG|WNOREAP|WEXITED|WSTOPPED|WCONTINUED|WONLYTHREADS) ||
   !(options & (WEXITED|WSTOPPED|WCONTINUED)))
     error_throw(E_INVALID_ARGUMENT);
 if ((unsigned int)which > P_PGID)
     error_throw(E_INVALID_ARGUMENT);
 validate_writable_opt(infop,sizeof(siginfo_t));
 validate_writable_opt(ru,sizeof(struct rusage));
 return posix_waitfor(which,upid,NULL,infop,options,ru);
}

DEFINE_SYSCALL_DONTRESTART(wait4);
DEFINE_SYSCALL4(wait4,pid_t,upid,
                USER UNCHECKED int *,wstatus,int,options,
                USER UNCHECKED struct rusage *,ru) {
 int which;
 pid_t result;
 if (options & ~(WNOHANG|WSTOPPED|WCONTINUED|
                 /* POSIX doesn't allow the following 2 options,
                  * however besides portability, I see no reason
                  * why they shouldn't be allowed. */
                 WEXITED|WNOREAP|
                 /* KOS extension: Only wait for threads (not child processes). */
                 WONLYTHREADS
                 ))
     error_throw(E_INVALID_ARGUMENT);
 options |= WEXITED;
 validate_writable_opt(wstatus,sizeof(int));
 validate_writable_opt(ru,sizeof(struct rusage));
 if (upid < -1) {
  upid  = -upid;
  which = P_PGID;
 } else if (upid == -1) {
  upid  = 0;
  which = P_ALL;
 } else if (upid == 0) {
  /* wait for any child process whose process group ID is equal to that of the calling process. */
  REF struct task *group = task_get_group(THIS_TASK);
  struct thread_pid *pid = FORTASK(group,_this_pid);
  which = P_PGID;
  upid  = pid ? pid->tp_pids[pid->tp_ns->pn_indirection] : 0;
  task_decref(group);
 } else {
  /* wait for the child whose process ID is equal to the value of pid. */
  which = P_PID;
 }
 result = posix_waitfor(which,upid,wstatus,NULL,options,ru);
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_GROUP_C */
