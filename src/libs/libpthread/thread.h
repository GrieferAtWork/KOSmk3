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
#ifndef GUARD_LIBS_LIBPTHREAD_THREAD_H
#define GUARD_LIBS_LIBPTHREAD_THREAD_H 1

#include "libpthread.h"
#include "threadattr.h"
#include <hybrid/atomic.h>
#include <hybrid/timespec.h>
#include <kos/types.h>
#include <kos/thread.h>
#include <kos/intrin.h>
#include <kos/futex.h>
#include <stdbool.h>

DECL_BEGIN

typedef struct thread Thread;
typedef struct thread_attributes ThreadAttributes;
typedef void *(*ThreadMain)(void *__arg);

struct thread {
    ATOMIC_DATA ref_t t_refcnt;  /* Reference counter for the thread descriptor.
                                  * WARNING: Successfully joining a thread ignores this
                                  *          reference counter and invoke `thread_destroy()'
                                  *          regardless of its value. */
    pid_t             t_tid;     /* [lock(WRITE_ONCE)]
                                  * TID of the thread (Set of ZERO(0) when the thread
                                  * exits; also used join the thread; this is a futex).
                                  * HINT: This field is passed to `sys_set_tid_address()' */
    pid_t             t_realtid; /* [const] The thread's original TID */
    void             *t_exitval; /* [valid_if(t_tid == 0)] The thread's exit value. */
    ThreadMain        t_entry;   /* [const] The thread's entry point. */
    void             *t_arg;     /* [const] The thread's entry argument. */
    ThreadAttr        t_attr;    /* [const] Attributes of this thread. */
};

struct thread_attributes {
    /* ... */
};


#if defined(__i386__) || defined(__x86_64__)
#ifdef __ASM_TASK_SEGMENT_ISFS
#define GET_CURRENT()  ((Thread *)__readfsptr(offsetof(struct task_segment,ts_pthread)))
#define SET_CURRENT(v)           __writefsptr(offsetof(struct task_segment,ts_pthread),v)
#else
#define GET_CURRENT()  ((Thread *)__readgsptr(offsetof(struct task_segment,ts_pthread)))
#define SET_CURRENT(v)           __writegsptr(offsetof(struct task_segment,ts_pthread),v)
#endif
#else
#define GET_CURRENT()         __current()->ts_pthread
#define SET_CURRENT(v) (void)(__current()->ts_pthread = (v))
#endif


#define thread_incref(self)  ATOMIC_FETCHINC((self)->t_refcnt)
#define thread_decref(self) (ATOMIC_DECFETCH((self)->t_refcnt) || (thread_destroy(self),0))
INTDEF void LIBPCALL thread_destroy(Thread *__restrict self);

/* Return the pthread thread descriptor for the current thread.
 * If not already allocated, allocate it before returning. */
INTDEF ATTR_CONST Thread *LIBPCALL thread_current(void);

/* Exit the current thread without unwinding the stack. */
INTDEF ATTR_NORETURN void LIBPCALL thread_exit(void *retval);

/* Construct a new thread. */
INTDEF errno_t LIBPCALL thread_create(Thread **__restrict presult, ThreadAttributes const *attr, ThreadMain entry, void *arg);
INTDEF ATTR_RETNONNULL Thread *LIBPCALL thread_Xcreate(ThreadAttributes const *attr, ThreadMain entry, void *arg);

/* Detach / Join a thread. */
INTDEF errno_t LIBPCALL thread_detach(Thread *__restrict self);
INTDEF errno_t LIBPCALL thread_join(Thread *__restrict self, void **thread_return);
INTDEF errno_t LIBPCALL thread_tryjoin(Thread *__restrict self, void **thread_return);
INTDEF errno_t LIBPCALL thread_timedjoin(Thread *__restrict self, void **thread_return, struct timespec32 const *abstime);
INTDEF errno_t LIBPCALL thread_timedjoin64(Thread *__restrict self, void **thread_return, struct timespec64 const *abstime);
INTDEF void *LIBPCALL thread_Xjoin(Thread *__restrict self);
INTDEF bool LIBPCALL thread_Xtryjoin(Thread *__restrict self, void **thread_return);
INTDEF bool LIBPCALL thread_Xtimedjoin(Thread *__restrict self, void **thread_return, struct timespec32 const *abstime);
INTDEF bool LIBPCALL thread_Xtimedjoin64(Thread *__restrict self, void **thread_return, struct timespec64 const *abstime);

/* Get the attributes of the given thread. */
INTDEF errno_t LIBPCALL thread_getattr_np(Thread *__restrict self, ThreadAttr *__restrict result);
INTDEF void LIBPCALL Xthread_getattr_np(Thread *__restrict self, ThreadAttr *__restrict result);


INTDEF errno_t LIBPCALL thread_setschedparam(Thread *__restrict self, int policy, struct sched_param const *param);
INTDEF errno_t LIBPCALL thread_getschedparam(Thread *__restrict self, int *__restrict policy, struct sched_param *__restrict param);
INTDEF errno_t LIBPCALL thread_setschedprio(Thread *__restrict self, int prio);
INTDEF void LIBPCALL Xthread_setschedparam(Thread *__restrict self, int policy, struct sched_param const *param);
INTDEF void LIBPCALL Xthread_getschedparam(Thread *__restrict self, int *__restrict policy, struct sched_param *__restrict param);
INTDEF void LIBPCALL Xthread_setschedprio(Thread *__restrict self, int prio);

INTDEF errno_t LIBPCALL thread_getname_np(Thread *__restrict self, char *buf, size_t buflen);
INTDEF errno_t LIBPCALL thread_setname_np(Thread *__restrict self, const char *name);
INTDEF size_t LIBPCALL Xthread_getname_np(Thread *__restrict self, char *buf, size_t buflen);
INTDEF void LIBPCALL Xthread_setname_np(Thread *__restrict self, const char *name);

INTDEF int LIBPCALL thread_getconcurrency(void);
INTDEF errno_t LIBPCALL thread_setconcurrency(int level);
INTDEF int LIBPCALL Xthread_getconcurrency(void);
INTDEF void LIBPCALL Xthread_setconcurrency(int level);

INTDEF errno_t LIBPCALL thread_yield(void);
INTDEF errno_t LIBPCALL thread_setaffinity_np(Thread *__restrict self, size_t cpusetsize, __cpu_set_t const *cpuset);
INTDEF errno_t LIBPCALL thread_getaffinity_np(Thread *__restrict self, size_t cpusetsize, __cpu_set_t *cpuset);
INTDEF void LIBPCALL Xthread_yield(void);
INTDEF void LIBPCALL Xthread_setaffinity_np(Thread *__restrict self, size_t cpusetsize, __cpu_set_t const *cpuset);
INTDEF void LIBPCALL Xthread_getaffinity_np(Thread *__restrict self, size_t cpusetsize, __cpu_set_t *cpuset);

INTDEF int LIBPCALL thread_rpc(Thread *__restrict self, unsigned int (LIBCCALL *func)(void *arg), void *arg, unsigned int mode);
INTDEF bool LIBPCALL Xthread_rpc(Thread *__restrict self, unsigned int (LIBCCALL *func)(void *arg), void *arg, unsigned int mode);


DECL_END

#endif /* !GUARD_LIBS_LIBPTHREAD_THREAD_H */
