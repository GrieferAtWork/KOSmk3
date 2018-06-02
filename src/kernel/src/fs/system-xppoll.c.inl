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
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/timespec.h>

#include <kos/futex.h>

#include <fs/handle.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <kernel/vm.h>
#include <sched/posix_signals.h>
#include <sched/pid.h>
#include <sched/group.h>
#include <sched/async_signal.h>

#include <alloca.h>
#include <except.h>
#include <signal.h>
#include <string.h>
#include <sys/poll.h>


DECL_BEGIN

#ifndef SYSTEM_XPPOLL_HELPERS_DEFINED
#define SYSTEM_XPPOLL_HELPERS_DEFINED 1
INTDEF void KCALL
get_rusage(struct thread_pid *__restrict pid,
           USER CHECKED struct rusage *ru);


PRIVATE ATTR_RETNONNULL REF struct futex *
KCALL vm_futex_consafe(VIRT void *addr) {
 return TASK_EVAL_CONSAFE(vm_futex(addr));
}

PRIVATE REF struct futex *
KCALL vm_getfutex_consafe(VIRT void *addr) {
 return TASK_EVAL_CONSAFE(vm_getfutex(addr));
}
#endif /* !SYSTEM_XPPOLL_HELPERS_DEFINED */

#ifdef XPPOLL_COMPAT
#define F_POLL_PID   poll_pid_compat
#define T_POLLPID    struct pollpid_compat
#define T_POLLINFO   struct __os_pollinfo_compat
#define T_POLLFUTEX  struct pollfutex_compat
#define T_SIGINFO_T  siginfo_compat_t
#else
#define F_POLL_PID   poll_pid
#define T_POLLPID    struct pollpid
#define T_POLLINFO   struct __os_pollinfo
#define T_POLLFUTEX  struct pollfutex
#define T_SIGINFO_T  siginfo_t
#endif



LOCAL bool KCALL
F_POLL_PID(USER CHECKED T_POLLPID *__restrict pinfo) {
 T_POLLPID info;
 struct task *my_process;
 struct threadgroup *my_group;
 struct thread_pid *EXCEPT_VAR child;
 size_t my_indirection;
 memcpy(&info,pinfo,sizeof(T_POLLPID));
 COMPILER_READ_BARRIER();
 info.pp_pid;
 my_process     = get_this_process();
 my_group       = &FORTASK(my_process,_this_group);
 my_indirection = FORTASK(my_process,_this_pid) ? FORTASK(my_process,_this_pid)->tp_ns->pn_indirection : 0;
 /* Always connect to the child-event signal that
  * child processes broadcast when they change state. */
 if (!task_connected(&my_group->tg_process.h_cldevent))
      task_connect(&my_group->tg_process.h_cldevent);
 /* Enumerate child processes. */
 sig_get(&my_group->tg_process.h_cldevent);
 child = my_group->tg_process.h_children;
 for (; child; child = child->tp_siblings.le_next) {
  bool has_write_lock = false;
  assert(child->tp_siblings.le_pself);
  /* Check if this child process matches requested criteria. */
  if (info.pp_which == P_PID) {
   if (child->tp_pids[my_indirection] != info.pp_pid)
       continue;
  }

  atomic_rwlock_read(&child->tp_task_lock);
check_child_again:
  if ((info.pp_options & WONLYTHREADS) && child->tp_task &&
       FORTASK(child->tp_task,_this_group).tg_leader != get_this_process()) {
   atomic_rwlock_endread(&child->tp_task_lock);
   continue; /* Not a thread of this process. */
  }
  /* XXX: We're not tracking the PGID in the PID descriptor,
   *      but apparently POSIX wants us to remember a thread's
   *      process group, even after the thread has died...
   *      As a kind-of crappy work-around, right now we simply
   *      ignore the child's PGID if the thread has already been
   *      destroyed. */
  if (info.pp_which == P_PGID && child->tp_task) {
   struct processgroup *child_group;
   struct thread_pid *child_group_pid;
   pid_t thread_pgid;
   child_group = &FORTASK(FORTASK(child->tp_task,_this_group).tg_leader,_this_group).tg_process.h_procgroup;
   atomic_rwlock_read(&child_group->pg_lock);
   child_group_pid = FORTASK(child_group->pg_leader,_this_pid);
   thread_pgid     = child_group_pid ? child_group_pid->tp_pids[my_indirection] : 0;
   atomic_rwlock_endread(&child_group->pg_lock);
   /* Match process group IDs. */
   if (thread_pgid != info.pp_pid) {
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
child_died:
   /* The thread has died. */
   if (has_write_lock)
        atomic_rwlock_endwrite(&child->tp_task_lock);
   else atomic_rwlock_endread(&child->tp_task_lock);
   if (!(info.pp_options & WEXITED)) continue; /* Don't wait for exited child processes. */
   /* Remove this child's zombie corpse when `WNOREAP' isn't set. */
   if (!(info.pp_options & WNOREAP)) {
    LIST_REMOVE(child,tp_siblings); /* Inherit reference. */
    child->tp_siblings.le_pself = NULL;
    child->tp_siblings.le_next  = NULL;
   } else {
    thread_pid_incref(child);
   }
   sig_put(&my_group->tg_process.h_cldevent);
   TRY {
    T_SIGINFO_T exit_info;
    /* Disconnect from the state-change notification signal. */
    task_disconnect();
    /* Collect information in this child. */
    memset(&exit_info,0,sizeof(T_SIGINFO_T));
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
    exit_info.si_pid = child->tp_pids[my_indirection];
    exit_info.si_uid = 0; /* TODO */
    COMPILER_WRITE_BARRIER();
    pinfo->pp_result = child->tp_pids[my_indirection];
    if (info.pp_info)
        memcpy((void *)(uintptr_t)info.pp_info,&exit_info,sizeof(T_SIGINFO_T)); /* CAUTION: SEGFAULT */
    if (info.pp_ru)
        get_rusage(child,(struct rusage *)(uintptr_t)info.pp_ru);
    COMPILER_WRITE_BARRIER();
   } FINALLY {
    thread_pid_decref(child);
   }
   return true;
  }
  if (child->tp_event == 0) {
   /* Track the number of potential child process to join.
    * If there are none, we mustn't wait and fail with -ECHILD. */
   goto continue_next_child_unlock;
  }
  if (!has_write_lock) {
   has_write_lock = true;
   if (!atomic_rwlock_upgrade(&child->tp_task_lock))
        goto check_child_again;
  }
  /* An event happened in this child.
   * Check if it matches our criteria (stop/continue). */
  if (((child->tp_event == CLD_CONTINUED) && (info.pp_options & WCONTINUED)) ||
      ((child->tp_event == CLD_STOPPED) && (info.pp_options & WSTOPPED))) {
   T_SIGINFO_T child_info;
   sig_put(&my_group->tg_process.h_cldevent);
   /* Collect information in this child. */
   memset(&child_info,0,sizeof(T_SIGINFO_T));
   child_info.si_signo   = SIGCHLD;
   child_info.si_code    = child->tp_event;
   child_info.si_status  = child->tp_status.w_status;
   child_info.si_pid     = child->tp_pids[my_indirection];
   child_info.si_uid     = 0; /* TODO */
   atomic_rwlock_endwrite(&child->tp_task_lock);
   task_disconnect();
   COMPILER_WRITE_BARRIER();
   pinfo->pp_result = child->tp_pids[my_indirection];
   /* Copy information to user-space. */
   if (info.pp_info)
       memcpy((void *)(uintptr_t)info.pp_info,&child_info,sizeof(T_SIGINFO_T)); /* CAUTION: SEGFAULT */
   if (info.pp_ru) get_rusage(child,(struct rusage *)(uintptr_t)info.pp_ru);
   COMPILER_WRITE_BARRIER();
   return true;
  }
  /* Fallback: All other events (`CLD_EXITED', `CLD_KILLED', `CLD_DUMPED')
   *           are handled as the child having exited. */
  goto child_died;
 }
 sig_put(&my_group->tg_process.h_cldevent);
 return false;
}


#ifdef XPPOLL_COMPAT
DEFINE_SYSCALL_COMPAT4(xppoll,
                       USER UNCHECKED T_POLLINFO *,uinfo,
                       USER UNCHECKED struct timespec const *,tsp,
                       USER UNCHECKED sigset_t const *,signal_set,
                       size_t,sigset_size)
#else
DEFINE_SYSCALL_DONTRESTART(xppoll);
DEFINE_SYSCALL4(xppoll,
                USER UNCHECKED T_POLLINFO *,uinfo,
                USER UNCHECKED struct timespec const *,tsp,
                USER UNCHECKED sigset_t const *,signal_set,
                size_t,sigset_size)
#endif
{
 USER UNCHECKED sigset_t const *EXCEPT_VAR xsigmask = signal_set;
 size_t EXCEPT_VAR xsigsetsize = sigset_size;
 USER UNCHECKED struct pollfd *EXCEPT_VAR ufds;
 USER UNCHECKED T_POLLFUTEX *EXCEPT_VAR uftx;
 USER UNCHECKED T_POLLPID *EXCEPT_VAR upid;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 size_t EXCEPT_VAR nfds;
 size_t EXCEPT_VAR nftx;
 size_t EXCEPT_VAR npid;
 size_t EXCEPT_VAR i; sigset_t old_blocking;
 REF struct futex **EXCEPT_VAR futex_vec = NULL;
 size_t EXCEPT_VAR futex_cnt;
 /* Load user-space poll descriptors. */
 validate_readable(uinfo,sizeof(*uinfo));
 COMPILER_READ_BARRIER();
 ufds = (struct pollfd *)(uintptr_t)uinfo->i_ufdvec;
 nfds = (size_t)uinfo->i_ufdcnt;
 uftx = (T_POLLFUTEX *)(uintptr_t)uinfo->i_ftxvec;
 nftx = (size_t)uinfo->i_ftxcnt;
 upid = (T_POLLPID *)(uintptr_t)uinfo->i_pidvec;
 npid = (size_t)uinfo->i_pidcnt;
 COMPILER_READ_BARRIER();
 /* Validate the extracted vectors. */
 validate_readablem(ufds,nfds,sizeof(*ufds));
 validate_readablem(uftx,nftx,sizeof(*uftx));
 validate_readablem(upid,npid,sizeof(*upid));
 validate_readable_opt(tsp,sizeof(*tsp));

 /* Must restrict the number of futexes because of the malloca() below. */
 if unlikely(nftx > 0x1000)
    error_throw(E_INVALID_ARGUMENT);

 /* */if (!signal_set);
 else {
  if (xsigsetsize > sizeof(sigset_t))
      error_throw(E_INVALID_ARGUMENT);
  validate_readable(signal_set,sigset_size);
  signal_chmask(signal_set,&old_blocking,sigset_size,SIGNAL_CHMASK_FBLOCK);
 }
 TRY {
  futex_vec = (REF struct futex **)calloca(nftx*sizeof(REF struct futex *));
  futex_cnt = 0;
scan_again:
  assert(futex_cnt == 0);
  result = 0;
  /* Clear the channel mask. Individual channels
   * may be re-opened by poll-callbacks as needed. */
  task_channelmask(0);
  /* Scan file descriptors. */
  for (i = 0; i < nfds; ++i) {
   struct handle EXCEPT_VAR hnd;
   TRY {
    hnd = handle_get(ufds[i].fd);
    TRY {
     unsigned int mask;
     size_t num_connections = task_numconnected();
     mask = handle_poll(hnd,ufds[i].events);
     if (mask) {
      ++result;
      ufds[i].revents = (u16)mask;
     } else if (num_connections == task_numconnected()) {
      /* This handle didn't add any new connections,
       * and neither are any of its states signaled.
       * As a conclusion: this handle doesn't support poll() */
      ufds[i].revents = POLLNVAL;
     } else {

      ufds[i].revents = 0;
     }
    } FINALLY {
     handle_decref(hnd);
    }
   } CATCH_HANDLED (E_INVALID_HANDLE) {
    ufds[i].revents = POLLERR;
   }
  }
  /* Scan futex objects. */
  for (i = 0; i < nftx; ++i) {
   REF struct futex *EXCEPT_VAR ftx;
   USER UNCHECKED futex_t *uaddr;
   uaddr = (USER UNCHECKED futex_t *)(uintptr_t)uftx[i].pf_futex;
   COMPILER_READ_BARRIER();
   TRY {
    validate_writable(uaddr,sizeof(futex_t));
    switch (uftx[i].pf_action) {

    case FUTEX_NOP:
     break;

    {
     futex_t probe_value;
    case FUTEX_WAIT_BITSET:
    case FUTEX_WAIT_BITSET_GHOST:
     /* Open all the channels described by the futex poll operation. */
     task_openchannel(uftx[i].pf_val3);
    case FUTEX_WAIT:
    case FUTEX_WAIT_GHOST:
     probe_value = (futex_t)uftx[i].pf_val;
     COMPILER_READ_BARRIER();
     if (ATOMIC_READ(*uaddr) != probe_value) {
      uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
      ++result;
     } else {
      /* Lookup and connect to the futex in question. */
      ftx = vm_futex_consafe(uaddr);
      COMPILER_BARRIER();
      futex_vec[futex_cnt++] = ftx; /* Inherit reference. */
      COMPILER_BARRIER();
      task_connect_ghost(&ftx->f_sig);

      /* Now that we're connected (and therefor interlocked), repeat the check. */
      COMPILER_READ_BARRIER();
      if (ATOMIC_READ(*uaddr) == probe_value) {
       uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
       ++result;
      }
      COMPILER_BARRIER();
     }
     break;
    } break;

    case FUTEX_LOCK_PI:
     /* A futex lock not held by anyone is indicated by a value equal to ZERO(0) */
     if (ATOMIC_READ(*uaddr) == 0) {
      uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
      ++result;
     } else {
      /* Lookup and connect to the futex in question. */
      ftx = vm_futex_consafe(uaddr);
      COMPILER_BARRIER();
      futex_vec[futex_cnt++] = ftx; /* Inherit reference. */
      COMPILER_BARRIER();
      task_connect_ghost(&ftx->f_sig);
      COMPILER_READ_BARRIER();
      for (;;) {
       /* Set the WAITERS bit to ensure that user-space can see that
        * someone is waiting for the futex to become available.
        * If we didn't do this, a user-space mutex implementation might
        * neglect to wake us when the associated lock becomes ready. */
       futex_t word = ATOMIC_READ(*uaddr);
       if (word == 0) {
        uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
        ++result;
        break;
       }
       if (ATOMIC_CMPXCH_WEAK(*uaddr,word,word|FUTEX_WAITERS))
           break;
      }
      COMPILER_BARRIER();
     }
     break;

    {
     futex_t probe_mask;
     futex_t probe_value;
     futex_t enable_bits;
    case FUTEX_WAIT_MASK_BITSET:
    case FUTEX_WAIT_MASK_BITSET_GHOST:
     enable_bits = (futex_t)(uintptr_t)uftx[i].pf_val3;
     COMPILER_READ_BARRIER();
     task_openchannel(enable_bits);
     goto do_mask;
    case FUTEX_WAIT_MASK:
    case FUTEX_WAIT_MASK_GHOST:
     enable_bits = (futex_t)(uintptr_t)uftx[i].pf_val3;
do_mask:
     probe_mask  = (futex_t)(uintptr_t)uftx[i].pf_val;
     probe_value = (futex_t)(uintptr_t)uftx[i].pf_uaddr2;
     COMPILER_READ_BARRIER();
     if ((ATOMIC_READ(*uaddr) & probe_mask) != probe_value) {
      uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
      ++result;
     } else {
      u32 old_value;
      ftx = vm_futex_consafe(uaddr);
      COMPILER_BARRIER();
      futex_vec[futex_cnt++] = ftx; /* Inherit reference. */
      COMPILER_BARRIER();
      task_connect_ghost(&ftx->f_sig);
      /* Atomically set bits from val3, while also ensuring that the mask still applies. */
      do if (((old_value = ATOMIC_READ(*uaddr)) & probe_mask) != probe_value) {
       uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
       ++result;
       break;
      } while (!ATOMIC_CMPXCH(*uaddr,old_value,old_value|enable_bits));
     }
    } break;

    {
     futex_t probe_mask;
     futex_t probe_value;
     futex_t enable_bits;
    case FUTEX_WAIT_NMASK_BITSET:
    case FUTEX_WAIT_NMASK_BITSET_GHOST:
     enable_bits = (futex_t)(uintptr_t)uftx[i].pf_val3;
     COMPILER_READ_BARRIER();
     task_openchannel(enable_bits);
     goto do_nmask;
    case FUTEX_WAIT_NMASK:
    case FUTEX_WAIT_NMASK_GHOST:
     enable_bits = (futex_t)(uintptr_t)uftx[i].pf_val3;
do_nmask:
     probe_mask  = (futex_t)(uintptr_t)uftx[i].pf_val;
     probe_value = (futex_t)(uintptr_t)uftx[i].pf_uaddr2;
     COMPILER_READ_BARRIER();
     if ((ATOMIC_READ(*uaddr) & probe_mask) == probe_value) {
      uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
      ++result;
     } else {
      u32 old_value;
      ftx = vm_futex_consafe(uaddr);
      COMPILER_BARRIER();
      futex_vec[futex_cnt++] = ftx; /* Inherit reference. */
      COMPILER_BARRIER();
      task_connect_ghost(&ftx->f_sig);
      /* Atomically set bits from val3, while also ensuring that the mask still applies. */
      do if (((old_value = ATOMIC_READ(*uaddr)) & probe_mask) == probe_value) {
       uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
       ++result;
       break;
      } while (!ATOMIC_CMPXCH(*uaddr,old_value,old_value|enable_bits));
     }
    } break;

    {
     futex_t old_value;
     futex_t new_value;
     futex_t *uaddr2;
     u32 real_old_value;
    case FUTEX_WAIT_CMPXCH:
    case FUTEX_WAIT_CMPXCH_GHOST:
     old_value = (futex_t)uftx[i].pf_val;
     new_value = (futex_t)uftx[i].pf_val3;
     uaddr2    = uftx[i].pf_uaddr2;
     validate_writable_opt(uaddr2,sizeof(*uaddr2));
     COMPILER_READ_BARRIER();
     ftx = vm_futex_consafe(uaddr);
     COMPILER_BARRIER();
     futex_vec[futex_cnt++] = ftx; /* Inherit reference. */
     COMPILER_BARRIER();
     task_connect_ghost(&ftx->f_sig);
     /* Now that we're connected, try the CMPXCH again, this
      * time storing the old value in uaddr2 (if provided). */
     real_old_value = ATOMIC_CMPXCH_VAL(*uaddr,old_value,new_value);
     if (uaddr2) {
      /* Save the old futex value in the provided uaddr2 */
      ATOMIC_WRITE(*uaddr2,real_old_value);
      /* Broadcast a futex located at the given address. */
      ftx = vm_getfutex_consafe(uaddr2);
      if (ftx) {
       size_t thread_count;
       thread_count = sig_broadcast(&ftx->f_sig);
       futex_decref(ftx);
       COMPILER_BARRIER();
       uftx[i].pf_result = thread_count;
       COMPILER_WRITE_BARRIER();
      }
     }
     if (real_old_value != old_value) {
      /* The futex entered the required state while we were connected to it. */
      uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
      ++result;
     }
    } break;

    {
     futex_t old_value;
     futex_t new_value;
     futex_t *uaddr2;
     u32 real_old_value;
    case FUTEX_WAIT_CMPXCH2:
    case FUTEX_WAIT_CMPXCH2_GHOST:
     old_value = (futex_t)uftx[i].pf_val;
     new_value = (futex_t)uftx[i].pf_val3;
     uaddr2    = uftx[i].pf_uaddr2;
     validate_writable(uaddr2,sizeof(*uaddr2));
     COMPILER_READ_BARRIER();
     /* The initial CMPXCH failed. - Connect to the futex at that address. */
     ftx = vm_futex_consafe(uaddr);
     COMPILER_BARRIER();
     futex_vec[futex_cnt++] = ftx; /* Inherit reference. */
     COMPILER_BARRIER();
     task_connect_ghost(&ftx->f_sig);
     /* Now that we're connected, try the CMPXCH again, this
      * time storing the old value in uaddr2 (if provided). */
     real_old_value = ATOMIC_CMPXCH_VAL(*uaddr,old_value,new_value);
     if (real_old_value == old_value) {
      /* Save the old futex value in the provided uaddr2 */
      ATOMIC_WRITE(*uaddr2,real_old_value);
      /* Broadcast a futex located at the given address. */
      ftx = vm_getfutex_consafe(uaddr2);
      if (ftx) {
       size_t thread_count;
       thread_count = sig_broadcast(&ftx->f_sig);
       futex_decref(ftx);
       COMPILER_BARRIER();
       uftx[i].pf_result = thread_count;
       COMPILER_WRITE_BARRIER();
      }
     } else {
      /* The futex entered the required state while we were connected to it. */
      uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
      ++result;
     }
    } break;

    default:
     uftx[i].pf_status = POLLFUTEX_STATUS_BADACTION;
     break;
    }
   } CATCH_HANDLED (E_SEGFAULT) {
    uftx[i].pf_status = POLLFUTEX_STATUS_BADFUTEX;
   }
  }
  /* Scan PIDs. */
  for (i = 0; i < npid; ++i) {
   if (F_POLL_PID(&upid[i]))
       ++result;
  }

  if (result) {
   /* At least one of the handles has been signaled. */
   task_udisconnect();
  } else if (!tsp) {
   task_uwait();
scan_again_drop_futex:
   /* Drop all saved futex references. */
   while (futex_cnt) {
    --futex_cnt;
    futex_decref(futex_vec[futex_cnt]);
   }
   goto scan_again;
  } else if (task_isconnected() || (!nfds && !nftx && !npid)) {
   /* Wait for signals to arrive and scan again. */
   if (task_waitfor_tmrel(tsp))
       goto scan_again_drop_futex;
   /* NOTE: If the timeout expires, ZERO(0) is returned. */
  }
 } FINALLY {
  if (futex_vec) {
   /* Cleanup saved futex references. */
   while (futex_cnt--)
      futex_decref(futex_vec[futex_cnt]);
   freea(futex_vec);
  }
  if (xsigmask)
      signal_chmask(&old_blocking,NULL,xsigsetsize,SIGNAL_CHMASK_FBLOCK);
 }
 return result;
}

#undef F_POLL_PID
#undef T_POLLPID
#undef T_POLLINFO
#undef T_POLLFUTEX
#undef T_SIGINFO_T

DECL_END
