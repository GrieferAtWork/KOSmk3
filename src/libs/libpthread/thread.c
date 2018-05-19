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
#ifndef GUARD_LIBS_LIBPTHREAD_THREAD_C
#define GUARD_LIBS_LIBPTHREAD_THREAD_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1
#define _TIME64_SOURCE 1
#define _EXCEPT_SOURCE 1

#include "libpthread.h"
#include "thread.h"
#include <sys/wait.h>
#include <malloc.h>
#include <except.h>
#include <sched.h>
#include <unistd.h>
#include <syscall.h>
#include <errno.h>
#include <string.h>

DECL_BEGIN


EXPORT(pthread_self,thread_current);
INTERN ATTR_CONST Thread *LIBPCALL thread_current(void) {
 Thread *result = GET_CURRENT();
 if likely(result) goto done;
 /* Allocate the descriptor lazily. */
 result = (Thread *)calloc(1,sizeof(Thread));
 if unlikely(!result) goto done; /* NULL symbolically referrs to the current thread. */
 result->t_refcnt = 1;
 result->t_tid    = __gettid();
done:
 return result;
}

EXPORT(pthread_equal,thread_equals);
INTERN ATTR_CONST ATTR_NOTHROW int LIBPCALL
thread_equals(Thread *a, Thread *b) {
 return (a == b ||
       (!a && b == GET_CURRENT()) ||
       (!b && a == GET_CURRENT()));
}


EXPORT(pthread_create,thread_create);
INTERN errno_t LIBPCALL
thread_create(Thread **__restrict presult,
              ThreadAttributes const *attr,
              ThreadMain entry, void *arg) {
 errno_t result = 0;
 TRY {
  *presult = thread_Xcreate(attr,entry,arg);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  result = except_geterrno();
 }
 return result;
}


INTERN void LIBPCALL
thread_destroy(Thread *__restrict self) {
 /* ... */
 free(self);
}

PRIVATE int LIBCCALL
thread_core_entry(Thread *__restrict self) {
 Thread *EXCEPT_VAR xself = self;
 /* Set the given thread as the current-thread-context. */
 SET_CURRENT(self);
 TRY {
  /* Invoke the user's thread entry function and save its return value. */
  self->t_exitval = (*self->t_entry)(self->t_arg);
 } FINALLY {
  if (FINALLY_WILL_RETHROW)
      xself->t_exitval = NULL;
  /* Drop a reference from the current thread.
   * Since the pthread controller isn't managed by
   * the kernel, it's our job to clean up after it! */
  thread_decref(xself);
 }
 return 0;
}


EXPORT(Xpthread_create,thread_Xcreate);
INTERN ATTR_RETNONNULL Thread *LIBPCALL
thread_Xcreate(ThreadAttributes const *attr,
               ThreadMain entry, void *arg) {
 Thread *EXCEPT_VAR result;
 u32 COMPILER_IGNORE_UNINITIALIZED(clone_flags);
 result = (Thread *)Xcalloc(1,sizeof(Thread));
 TRY {
  /* Copy thread attributes. */
  if (attr)
      memcpy(&result->t_attr,attr,sizeof(ThreadAttributes));
  result->t_refcnt  = 2;     /* +1 for the caller; +1 for the thread itself. */
  result->t_entry   = entry;
  result->t_arg     = arg;
  clone_flags = (/* NOTE: `CLONE_THREAD' does _NOT_ mean detached in KOS! */
                 CLONE_THREAD | CLONE_VM | CLONE_FS |
                 CLONE_FILES | CLONE_SIGHAND |
                 CLONE_IO | CLONE_CHILD_CLEARTID |
                 CLONE_PARENT_SETTID);
  if (result->t_attr.ta_flags & THREADATTR_FDETACHSTATE)
      clone_flags |= CLONE_DETACHED;
  /* If the user didn't define a stack for the thread,
   * let the kernel automatically generate one for it! */
  if (!(result->t_attr.ta_flags & THREADATTR_FSTACKADDR))
        result->t_attr.ta_stackaddr = CLONE_CHILDSTACK_AUTO;
  result->t_realtid = Xclone((int(*)(void *))&thread_core_entry,
                              result->t_attr.ta_stackaddr,
                              clone_flags,
                              result,
                              /* NOTE: Must let the kernel initialize `t_tid',
                               *       so it is filled properly if the thread
                               *       terminates before we would have set this
                               *       field. */
                             &result->t_tid);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  free(result);
  error_rethrow();
 }
 if (clone_flags & CLONE_DETACHED) {
  /* The thread was spawned as already detached! */
  result->t_realtid = 0;
  thread_decref(result);
  result = NULL;
 }
 return result;
}

EXPORT(pthread_exit,thread_exit);
INTERN ATTR_NORETURN void LIBPCALL thread_exit(void *retval) {
 Thread *me = GET_CURRENT();
 if (me) {
  /* Save the exit code. */
  me->t_exitval = retval;
  /* Drop a reference from the calling thread's controller. */
  thread_decref(me);
 }
 if (__current()->ts_type == THREAD_TYPE_MAINTHREAD) {
  /* POSIX wants pthread_exit() invoked from the main() thread to
   * wait until all worker threads have exited before exiting the
   * process. */
  while (waitid(P_ALL,0,NULL,WEXITED|WONLYTHREADS) >= 0);
 }
 exit_thread(0);
}

EXPORT(pthread_detach,thread_detach);
INTERN errno_t LIBPCALL
thread_detach(Thread *__restrict self) {
 if (!self) {
  /* Try to detach the calling thread. */
  detach(0);
  return 0;
 }
 if (!self->t_realtid)
      return ESRCH; /* Already detached. */
 self->t_attr.ta_flags |= THREADATTR_FDETACHSTATE;
 detach(self->t_realtid);
 thread_decref(self);
 return 0;
}

EXPORT(pthread_join,thread_join);
INTERN errno_t LIBPCALL
thread_join(Thread *__restrict self, void **thread_return) {
 errno_t result = 0;
 TRY {
  void *exitcode;
  exitcode = thread_Xjoin(self);
  if (thread_return)
     *thread_return = exitcode;
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  result = except_geterrno();
 }
 return result;
}

EXPORT(pthread_tryjoin_np,thread_tryjoin);
INTERN errno_t LIBPCALL
thread_tryjoin(Thread *__restrict self, void **thread_return) {
 return thread_Xtryjoin(self,thread_return) ? 0 : EBUSY;
}

EXPORT(pthread_timedjoin_np,thread_timedjoin);
INTERN errno_t LIBPCALL
thread_timedjoin(Thread *__restrict self, void **thread_return,
                 struct timespec32 const *abstime) {
 struct timespec64 t64;
 t64.tv_sec  = abstime->tv_sec;
 t64.tv_nsec = abstime->tv_nsec;
 return thread_timedjoin64(self,thread_return,&t64);
}

EXPORT(pthread_timedjoin64_np,thread_timedjoin64);
INTERN errno_t LIBPCALL
thread_timedjoin64(Thread *__restrict self, void **thread_return,
                   struct timespec64 const *abstime) {
 errno_t result = 0;
 TRY {
  if (!thread_Xtimedjoin64(self,thread_return,abstime))
       result = EBUSY;
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  result = except_geterrno();
 }
 return result;
}

EXPORT(Xpthread_tryjoin_np,thread_Xtryjoin);
INTERN bool LIBPCALL
thread_Xtryjoin(Thread *__restrict self, void **thread_return) {
 if (!self || ATOMIC_READ(self->t_tid) != 0)
     return false;
 if (thread_return)
    *thread_return = self->t_exitval;
 /* Detach, and destroy the thread descriptor. */
 detach(self->t_realtid);
 thread_destroy(self);
 return true;
}

EXPORT(Xpthread_join,thread_Xjoin);
INTERN void *LIBPCALL
thread_Xjoin(Thread *__restrict self) {
 void *result; pid_t tid;
 if (!self) goto is_a_dealock;
 /* NOTE: We could use wait(2) here, but futex is faster
  *       if the thread has already terminated. */
 for (;;) {
  tid = ATOMIC_READ(self->t_tid);
  if (tid == 0)
      break; /* Thread has terminated. */
  if (self == GET_CURRENT())
      goto is_a_dealock;
  /* Wait while `t_tid == tid' */
  Xfutex_wait((futex_t *)&self->t_tid,tid,NULL);
 }
 /* This detach goes more along the lines of what you'd call `clone()' */
 detach(self->t_realtid);
 /* Return the thread's exit value. */
 result = self->t_exitval;
 /* Destroy the thread descriptor. */
 thread_destroy(self);
 return result;
is_a_dealock:
 error_throw(E_DEADLOCK);
}

EXPORT(Xpthread_timedjoin_np,thread_Xtimedjoin);
INTERN bool LIBPCALL
thread_Xtimedjoin(Thread *__restrict self, void **thread_return,
                  struct timespec32 const *abstime) {
 struct timespec64 t64;
 t64.tv_sec  = abstime->tv_sec;
 t64.tv_nsec = abstime->tv_nsec;
 return thread_Xtimedjoin64(self,thread_return,&t64);
}

EXPORT(Xpthread_timedjoin64_np,thread_Xtimedjoin64);
INTERN bool LIBPCALL
thread_Xtimedjoin64(Thread *__restrict self, void **thread_return,
                    struct timespec64 const *abstime) {
 void *exitval; pid_t tid;
 if (!self) goto is_a_dealock;
 for (;;) {
  tid = ATOMIC_READ(self->t_tid);
  if (tid == 0)
      break; /* Thread has terminated. */
  if (self == GET_CURRENT())
      goto is_a_dealock;
  /* Wait while `t_tid == tid' */
  if (!Xfutex_wait64((futex_t *)&self->t_tid,tid,abstime))
       return false;
 }
 /* This detach goes more along the lines of what you'd call `clone()' */
 detach(self->t_realtid);
 /* Return the thread's exit value. */
 exitval = self->t_exitval;
 /* Destroy the thread descriptor. */
 thread_destroy(self);
 COMPILER_WRITE_BARRIER();
 if (thread_return)
    *thread_return = exitval;
 return true;
is_a_dealock:
 error_throw(E_DEADLOCK);
}


EXPORT(pthread_getattr_np,thread_getattr_np);
INTERN errno_t LIBPCALL
thread_getattr_np(Thread *__restrict self, ThreadAttr *__restrict result) {
 memcpy(result,&self->t_attr,sizeof(ThreadAttr));
 return 0;
}

EXPORT(Xpthread_getattr_np,Xthread_getattr_np);
DEFINE_INTERN_ALIAS(Xthread_getattr_np,thread_getattr_np);


EXPORT(pthread_setschedparam,thread_setschedparam);
INTERN errno_t LIBPCALL
thread_setschedparam(Thread *__restrict self,
                     int policy, struct sched_param const *param) {
 return ENOSYS;
}

EXPORT(Xpthread_setschedparam,Xthread_setschedparam);
INTERN void LIBPCALL
Xthread_setschedparam(Thread *__restrict self,
                      int policy, struct sched_param const *param) {
 error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(pthread_getschedparam,thread_getschedparam);
INTERN errno_t LIBPCALL
thread_getschedparam(Thread *__restrict self,
                     int *__restrict policy,
                     struct sched_param *__restrict param) {
 return ENOSYS;
}

EXPORT(Xpthread_getschedparam,Xthread_getschedparam);
INTERN void LIBPCALL
Xthread_getschedparam(Thread *__restrict self,
                      int *__restrict policy,
                      struct sched_param *__restrict param) {
 error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(pthread_setschedprio,thread_setschedprio);
INTERN errno_t LIBPCALL
thread_setschedprio(Thread *__restrict self, int prio) {
 return ENOSYS;
}
EXPORT(Xpthread_setschedprio,Xthread_setschedprio);
INTERN void LIBPCALL
Xthread_setschedprio(Thread *__restrict self, int prio) {
 error_throw(E_NOT_IMPLEMENTED);
}



EXPORT(pthread_getname_np,thread_getname_np);
INTERN errno_t LIBPCALL
thread_getname_np(Thread *__restrict self, char *buf, size_t buflen) {
 return ENOSYS;
}
EXPORT(pthread_setname_np,thread_setname_np);
INTERN errno_t LIBPCALL
thread_setname_np(Thread *__restrict self, const char *name) {
 return ENOSYS;
}
EXPORT(Xpthread_getname_np,Xthread_getname_np);
INTERN size_t LIBPCALL
Xthread_getname_np(Thread *__restrict self, char *buf, size_t buflen) {
 error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(Xpthread_setname_np,Xthread_setname_np);
INTERN void LIBPCALL
Xthread_setname_np(Thread *__restrict self, const char *name) {
 error_throw(E_NOT_IMPLEMENTED);
}




PRIVATE int concurrency_level = 0;
EXPORT(pthread_getconcurrency,thread_getconcurrency);
INTDEF int LIBPCALL thread_getconcurrency(void) {
 return concurrency_level;
}
EXPORT(pthread_setconcurrency,thread_setconcurrency);
INTDEF errno_t LIBPCALL thread_setconcurrency(int level) {
 if (level < 0) return EINVAL;
 concurrency_level = level;
 return 0;
}
EXPORT(Xpthread_getconcurrency,Xthread_getconcurrency);
DEFINE_INTERN_ALIAS(Xthread_getconcurrency,thread_getconcurrency);
EXPORT(Xpthread_setconcurrency,Xthread_setconcurrency);
INTDEF void LIBPCALL Xthread_setconcurrency(int level) {
 if (level < 0)
     error_throw(E_INVALID_ARGUMENT);
 concurrency_level = level;
}


EXPORT(pthread_yield,thread_yield);
INTERN errno_t LIBPCALL thread_yield(void) {
 return sched_yield();
}
EXPORT(Xpthread_yield,Xthread_yield);
INTERN void LIBPCALL Xthread_yield(void) {
 Xsched_yield();
}

EXPORT(pthread_setaffinity_np,thread_setaffinity_np);
INTERN errno_t LIBPCALL
thread_setaffinity_np(Thread *__restrict self,
                      size_t cpusetsize,
                      __cpu_set_t const *cpuset) {
 pid_t tid = ATOMIC_READ(self->t_tid);
 if (!tid) return ESRCH;
 if (!sched_setaffinity(tid,cpusetsize,cpuset))
      return 0;
 return GET_ERRNO();
}

EXPORT(pthread_getaffinity_np,thread_getaffinity_np);
INTERN errno_t LIBPCALL
thread_getaffinity_np(Thread *__restrict self,
                      size_t cpusetsize,
                      __cpu_set_t *cpuset) {
 pid_t tid = ATOMIC_READ(self->t_tid);
 if (!tid) return ESRCH;
 if (!sched_getaffinity(tid,cpusetsize,cpuset))
      return 0;
 return GET_ERRNO();
}

EXPORT(Xpthread_setaffinity_np,Xthread_setaffinity_np);
INTERN void LIBPCALL
Xthread_setaffinity_np(Thread *__restrict self,
                       size_t cpusetsize, __cpu_set_t const *cpuset) {
 pid_t tid = ATOMIC_READ(self->t_tid);
 if (!tid) error_throw(E_PROCESS_EXITED);
 Xsched_setaffinity(tid,cpusetsize,cpuset);
}

EXPORT(Xpthread_getaffinity_np,Xthread_getaffinity_np);
INTERN void LIBPCALL
Xthread_getaffinity_np(Thread *__restrict self,
                       size_t cpusetsize, __cpu_set_t *cpuset) {
 pid_t tid = ATOMIC_READ(self->t_tid);
 if (!tid) error_throw(E_PROCESS_EXITED);
 Xsched_getaffinity(tid,cpusetsize,cpuset);
}



DECL_END

#endif /* !GUARD_LIBS_LIBPTHREAD_THREAD_C */
