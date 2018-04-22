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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_GROUP_H
#define GUARD_KERNEL_INCLUDE_SCHED_GROUP_H 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <kos/types.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/sections.h>
#include <sched/task.h>
#include <sched/signal.h>

DECL_BEGIN

#ifdef __CC__

struct task_weakref;
struct thread_pid;

/* Process/thread groupings:
 *   - thread-group   -- LEADER = THIS_GROUP.tg_leader
 *   - process-group  -- LEADER = FORTASK(THIS_GROUP.tg_leader,_this_group).tg_process.h_procgroup.pg_leader
 *   - session        -- LEADER = FORTASK(FORTASK(THIS_GROUP.tg_leader,_this_group).tg_process.h_procgroup.pg_leader,_this_group).tg_process.h_procgroup.pg_master.m_session
 * Sorry that these struct-member chains are so long...
 */


struct processgroup {
    atomic_rwlock_t                     pg_lock;       /* [ORDER(BEFORE(FORTASK(pg_leader,_this_group).tg_process.h_procgroup.pg_lock))]
                                                        *  Lock for process group controls. */
    REF struct task                    *pg_leader;     /* [1..1][ref_if(!= THIS_TASK)][lock(pg_lock)]
                                                        * [FORTASK(pg_leader,_this_group).tg_leader == pg_leader]
                                                        * The leader of the process group. (This thread's TID is used by `getpgid()') */
    union {
        struct {
            WEAK LIST_HEAD(struct task) m_members;     /* [lock(pg_lock)] Chain of processes (thread-group-leaders) that are part of this process group. */
            REF struct task            *m_session;     /* [1..1][ref_if(!= THIS_TASK)][lock(pg_lock)]
                                                        * [FORTASK(m_session,_this_group).tg_leader == pg_leader]
                                                        * [FORTASK(m_session,_this_group).tg_process.h_procgroup.pg_leader == m_session]
                                                        * The leader of the current process group session. */
            /* XXX: Keep a list of all processes in a session? - Do we need something like that? */
        }                               pg_master;     /* [valid_if(pg_leader == THIS_TASK)] */
        struct {
            WEAK LIST_NODE(struct task) m_members;     /* [lock(FORTASK(pg_leader,_this_group).tg_process.h_procgroup.pg_lock)]
                                                        * Chain of processes (thread-group-leaders) that are part of this process group. */
        }                               pg_slave;      /* [valid_if(pg_leader != THIS_TASK)] */
    };
};

struct threadgroup {
#ifndef __INTELLISENSE__
    REF
#endif
        REF struct task                 *tg_leader;     /* [1..1][lock(PRIVATE(THIS_TASK))][ref_if(!= THIS_TASK)]
                                                         *  The leader of this thread-group.
                                                         *  The leader really only serves a single purpose, that purpose
                                                         *  being to have child threads return its pid when `getpid()'
                                                         *  is called (posix calls this TGID; thread-group ID).
                                                         *  The leader is usually the ~main thread~ (first thread) of a process.
                                                         *  NOTE: To prevent a race condition where the leader is terminating
                                                         *        after a new thread has been created, but before that thread
                                                         *        could be started, once started, a new thread must first
                                                         *        execute a small piece of kernel-code that registers that
                                                         *        thread to the group of its supposed leader.
                                                         *        However if that fails (because the leader has terminated),
                                                         *        the thread simply exits immediately. */
    union {
        struct {
            atomic_rwlock_t              h_lock;        /* Lock for adding/removing threads from `m_group'. */
            WEAK LIST_HEAD(struct task)  h_group;       /* [lock(m_lock)] Chain of threads that are part of this group.
                                                         *                Threads in this group are implicitly terminated
                                                         *                when `tg_leader' is.
                                                         *          NOTE: When the `TASK_STATE_FTERMINATING' flag has been
                                                         *                set in the group leader, member threads of the
                                                         *                group are no longer allowed to spawn new threads
                                                         *                using the `clone()' system call with the
                                                         *               `CLONE_THREAD' flag set.
                                                         *          NOTE: The leader itself is not part of this chain. */
            REF struct task_weakref     *h_parent;      /* [0..1][const]
                                                         *  The thread that is send a SIGCLD signal when this thread dies and it's a leader.
                                                         *  This field is also used to implement `getppid()' as the PID
                                                         *  of `THIS_GROUP->tg_parent' (aka. TID of `THIS_GROUP->tg_parent->tg_leader')
                                                         *  NOTE: kernel workers and /bin/init have this field set to NULL.
                                                         *  NOTE: When this is a dead link, the thread has been orphaned
                                                         *       (and its parent is implicitly changed to `_boot_task'; aka. /bin/init).
                                                         *  NOTE: When /bin/init has died as well, no signal is
                                                         *        send (prevents undefined behavior during shutdown) */
            u8                           h_sigcld;      /* [const] The signal number+1 (usually SIGCLD+1) send to
                                                         *         `tg_parent' when this thread is the leader of a
                                                         *         thread group and is terminated or changes state.
                                                         *  NOTE:  When ZERO(0), no signal is sent. */
            u8                         __h_pad[sizeof(void *)-1];
            struct sig                   h_cldevent;    /* Signal send when something happens in a child process. */
            REF LIST_HEAD(struct thread_pid) h_children;/* [lock(h_cldevent)] Chain of child processes that someone from this thread group must/can wait() for. */
            struct processgroup          h_procgroup;   /* Process group control. */
        }                                tg_process;    /* [valid_if(tg_leader == THIS_TASK)] Thread-leader (aka. process)-specific data. */
        struct {
            WEAK LIST_NODE(struct task)  g_group;       /* [CHAIN(tg_leader->...->tg_process.h_group)]
                                                         * [lock(tg_leader->...->tg_process.h_lock)]
                                                         *  Chain of threads in this group (excluding the leader). */
        }                                tg_thread;     /* [valid_if(tg_leader != THIS_TASK)] Thread-specific data. */
    };
};


/* The thread-group descriptor for the calling thread. */
DATDEF ATTR_PERTASK struct threadgroup _this_group;
#define THIS_GROUP  PERTASK(_this_group)

/* Return a reference to the parent of the calling thread (as defined by POSIX).
 * If the calling thread doesn't have a parent, return NULL instead. */
FUNDEF REF struct task *KCALL task_get_parent(void);

/* Returns a pointer to the process leader, process group leader, or session leader of the calling thread. */
#ifdef __INTELLISENSE__
FORCELOCAL struct task *(KCALL get_this_process)(void);
#endif
FORCELOCAL REF struct task *KCALL get_this_processgroup(void);
FORCELOCAL REF struct task *KCALL get_this_session(void);
FORCELOCAL WEAK struct task *KCALL get_this_processgroup_weak(void);
FORCELOCAL WEAK struct task *KCALL get_this_session_weak(void);

/* Same as the functions above, but return information about some other thread. */
#ifdef __INTELLISENSE__
FORCELOCAL struct task *(KCALL get_process_of)(struct task *__restrict thread);
#endif
FORCELOCAL REF struct task *KCALL get_processgroup_of(struct task *__restrict thread);
FORCELOCAL REF struct task *KCALL get_session_of(struct task *__restrict thread);
FORCELOCAL WEAK struct task *KCALL get_processgroup_of_weak(struct task *__restrict thread);
FORCELOCAL WEAK struct task *KCALL get_session_of_weak(struct task *__restrict thread);

/* Send an RPC request to every member of the current thread-group.
 * NOTE: The caller must be thread-group leader.
 * @return: * : The number of threads on which the function was queued. */
FUNDEF size_t KCALL
taskgroup_queue_rpc(task_rpc_t func, void *arg,
                    unsigned int mode);
FUNDEF size_t KCALL
taskgroup_queue_rpc_user(task_user_rpc_t func, void *arg,
                         unsigned int mode)
                         ASMNAME("taskgroup_queue_rpc");

/* Get/Set the group association of the given `thread' */
FUNDEF ATTR_NOTHROW ATTR_RETNONNULL REF struct task *KCALL task_get_group(struct task *__restrict thread);
FUNDEF ATTR_NOTHROW void KCALL task_set_group(struct task *__restrict thread, struct task *__restrict group);

/* Return the session leading of the calling thread's process. */
FUNDEF ATTR_NOTHROW ATTR_RETNONNULL REF struct task *KCALL task_get_session(struct task *__restrict thread);

/* Must be executed in the context of the leader of a
 * thread-group (THIS_GROUP.tg_leader == THIS_TASK).
 * This function will send RPC requests to all threads running
 * in the same process and terminate them with `exit_status', which
 * should be constructed using the `__W_*' macros from `<bits/waitstatus.h>'
 * Additionally, this function ensures that all threads have finished terminating
 * by waiting for each RPC to complete before returning itself.
 * The caller must ensure that they are the leader of a thread group.
 * If they are not, they must queue an RPC callback in the thread-group
 * leader, which can then invoke this function.
 * This function is called when a thread-group leader terminates
 * in order to propagate its exit status to all other threads, as
 * well as to ensure that terminating a thread-group leader is the
 * same as terminating a process.
 * Additionally, this function is called when executing `exec()'
 * in order to terminate all secondary thread that may have been
 * running in the process at that time. */
FUNDEF void KCALL task_exit_secondary_threads(int exit_status);

/* Set the process of the calling thread as the leader of its own session.
 * According to POSIX specifications, this function fails if the calling
 * process is already the leader of some process group (why??), in which
 * case this function returns `false'.
 * Otherwise, the calling process is made the leader of its own process
 * group, before being placed in a new session containing only it,
 * following this function returning `true'. */
FUNDEF ATTR_NOTHROW bool KCALL task_set_session(void);


#if !defined(__INTELLISENSE__)
#define get_this_process()   PERTASK_GET(_this_group.tg_leader)
FORCELOCAL REF struct task *KCALL get_this_processgroup(void) {
 REF struct task *result; struct processgroup *group;
 group = &FORTASK(get_this_process(),_this_group).tg_process.h_procgroup;
 atomic_rwlock_read(&group->pg_lock);
 result = group->pg_leader;
 task_incref(result);
 atomic_rwlock_endread(&group->pg_lock);
 return result;
}
FORCELOCAL REF struct task *KCALL get_this_session(void) {
 struct processgroup *group;
 REF struct task *group_leader,*result;
 group_leader = get_this_processgroup();
 group = &FORTASK(group_leader,_this_group).tg_process.h_procgroup;
 atomic_rwlock_read(&group->pg_lock);
 result = group->pg_master.m_session;
 task_incref(result);
 atomic_rwlock_endread(&group->pg_lock);
 task_decref(group_leader);
 return result;
}
FORCELOCAL WEAK struct task *KCALL get_this_processgroup_weak(void) {
 return FORTASK(get_this_process(),_this_group).tg_process.h_procgroup.pg_leader;
}
FORCELOCAL WEAK struct task *KCALL get_this_session_weak(void) {
 struct task *result;
 struct processgroup *group,*leader_group;
 group = &FORTASK(get_this_process(),_this_group).tg_process.h_procgroup;
 atomic_rwlock_read(&group->pg_lock);
 leader_group = &FORTASK(group->pg_leader,_this_group).tg_process.h_procgroup;
 if (leader_group == group) {
  result = leader_group->pg_master.m_session;
 } else {
  atomic_rwlock_read(&leader_group->pg_lock);
  result = leader_group->pg_master.m_session;
  atomic_rwlock_endread(&leader_group->pg_lock);
 }
 atomic_rwlock_endread(&group->pg_lock);
 return result;
}


#define get_process_of(thread) (FORTASK(thread,_this_group).tg_leader)
FORCELOCAL REF struct task *KCALL
get_processgroup_of(struct task *__restrict thread) {
 REF struct task *result; struct processgroup *group;
 group = &FORTASK(get_process_of(thread),_this_group).tg_process.h_procgroup;
 atomic_rwlock_read(&group->pg_lock);
 result = group->pg_leader;
 task_incref(result);
 atomic_rwlock_endread(&group->pg_lock);
 return result;
}
FORCELOCAL REF struct task *KCALL
get_session_of(struct task *__restrict thread) {
 struct processgroup *group;
 REF struct task *group_leader,*result;
 group_leader = get_processgroup_of(thread);
 group = &FORTASK(group_leader,_this_group).tg_process.h_procgroup;
 atomic_rwlock_read(&group->pg_lock);
 result = group->pg_master.m_session;
 task_incref(result);
 atomic_rwlock_endread(&group->pg_lock);
 task_decref(group_leader);
 return result;
}
FORCELOCAL WEAK struct task *KCALL
get_processgroup_of_weak(struct task *__restrict thread) {
 return FORTASK(get_process_of(thread),_this_group).tg_process.h_procgroup.pg_leader;
}
FORCELOCAL WEAK struct task *KCALL
get_session_of_weak(struct task *__restrict thread) {
 struct task *result;
 struct processgroup *group,*leader_group;
 group = &FORTASK(get_process_of(thread),_this_group).tg_process.h_procgroup;
 atomic_rwlock_read(&group->pg_lock);
 leader_group = &FORTASK(group->pg_leader,_this_group).tg_process.h_procgroup;
 if (leader_group == group) {
  result = leader_group->pg_master.m_session;
 } else {
  atomic_rwlock_read(&leader_group->pg_lock);
  result = leader_group->pg_master.m_session;
  atomic_rwlock_endread(&leader_group->pg_lock);
 }
 atomic_rwlock_endread(&group->pg_lock);
 return result;
}
#endif


#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_GROUP_H */
