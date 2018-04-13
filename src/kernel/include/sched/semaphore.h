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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_SEMAPHORE_H
#define GUARD_KERNEL_INCLUDE_SCHED_SEMAPHORE_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/atomic.h>
#include <sched/task.h>
#include <sched/signal.h>
#include <stdbool.h>

#ifdef __CC__
DECL_BEGIN

typedef struct sem sem_t;
typedef unsigned long semcount_t;

struct sem {
    struct sig             sem_signal;  /* Signal send to `n' threads when `n' tickets become available. */
    ATOMIC_DATA semcount_t sem_tickets; /* Number of currently available tickets. */
};
#define SEM_INIT(initial_tickets)      {SIG_INIT,initial_tickets}
#define sem_init(self,initial_tickets) \
    (void)(sig_init(&(self)->sem_signal), \
          (self)->sem_tickets = initial_tickets)
#define sem_cinit(self,initial_tickets) \
    (void)(sig_cinit(&(self)->sem_signal), \
          (__builtin_constant_p(initial_tickets) && (initial_tickets) == 0) \
           ? 0 : ((self)->sem_tickets = initial_tickets))

/* Try to acquire, or wait until a ticket
 * has been made available, then acquire it.
 * @return: true:  A ticket has been acquired.
 * @return: false: Failed to acquire a ticket immediately,
 *                 or after `abs_timeout' elapsed. */
LOCAL ATTR_NOTHROW bool KCALL sem_trywait(struct sem *__restrict self);
FORCELOCAL bool KCALL sem_timedwait(struct sem *__restrict self, jtime_t abs_timeout);
#ifdef __INTELLISENSE__
FUNDEF void KCALL sem_wait(struct sem *__restrict self);
#else
#define sem_wait(self) (void)sem_timedwait(self,JTIME_INFINITE)
#endif

/* Make `num_tickets' available and wake up to that same number of threads. */
FUNDEF ATTR_NOTHROW void KCALL
sem_post(struct sem *__restrict self, semcount_t num_tickets);


#ifndef __INTELLISENSE__
FUNDEF bool KCALL
__os_sem_timedwait(struct sem *__restrict self,
                   jtime_t abs_timeout)
                   ASMNAME("sem_timedwait");
FORCELOCAL bool KCALL
sem_timedwait(struct sem *__restrict self, jtime_t abs_timeout) {
 if (!__os_sem_timedwait(self,abs_timeout))
      return false;
 COMPILER_BARRIER();
 return true;
}
LOCAL ATTR_NOTHROW bool KCALL
sem_trywait(struct sem *__restrict self) {
 semcount_t count;
 do if ((count = ATOMIC_READ(self->sem_tickets)) == 0)
         return false;
 while (!ATOMIC_CMPXCH_WEAK(self->sem_tickets,count,count-1));
 COMPILER_BARRIER();
 return true;
}
#endif


DECL_END
#endif /* __CC__ */

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_SEMAPHORE_H */
