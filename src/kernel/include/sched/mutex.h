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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_MUTEX_H
#define GUARD_KERNEL_INCLUDE_SCHED_MUTEX_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/sections.h>
#include <hybrid/atomic.h>
#include <sched/task.h>
#include <sched/signal.h>
#include <fs/iomode.h>
#ifndef __INTELLISENSE__
#include <except.h>
#endif

#ifdef __CC__
DECL_BEGIN

/* Recursive, mutually exclusive synchronization primitive */
typedef struct mutex mutex_t;

struct mutex {
    struct sig             m_signal; /* Signal used to schedule this mutex. */
    struct task           *m_owner;  /* [?..1][valid_if(m_ind != 0)] The thread currently holding a lock to this mutex. */
#ifdef __INTELLISENSE__
                uintptr_t  m_ind;    /* Indirection of how often `m_owner' called `mutex_get' (including the first time) */
#else
    ATOMIC_DATA uintptr_t  m_ind;    /* Indirection of how often `m_owner' called `mutex_get' (including the first time) */
#endif
};

#define MUTEX_INIT         {SIG_INIT,NULL,0}
#define DEFINE_MUTEX(name) mutex_t name = MUTEX_INIT

#define mutex_cinit(self)  \
   (void)(sig_cinit(&(self)->m_signal), \
          assert((self)->m_owner == NULL), \
          assert((self)->m_ind == 0))
#define mutex_init(self)   \
   (void)(sig_init(&(self)->m_signal), \
         (self)->m_owner = NULL, \
         (self)->m_ind = 0)
#define mutex_holding(x)   ((x)->m_owner == THIS_TASK && (x)->m_ind != 0)


/* Atomically try to acquire the given mutex. */
FORCELOCAL ATTR_NOTHROW bool KCALL mutex_try(struct mutex *__restrict self);

/* Acquire/release a mutex.
 * @return: true:  Successfully acquired the mutex.
 * @return: false: The given timeout has expired.
 * NOTE: `nothrow' when interrupts are enabled and `task_nothrow_serve()' has been called. */
FUNDEF bool KCALL __os_mutex_get_timed(struct mutex *__restrict self, jtime_t abs_timeout) ASMNAME("mutex_get_timed");
FUNDEF ATTR_NOTHROW void KCALL __os_mutex_put(struct mutex *__restrict self) ASMNAME("mutex_put");

/* Place some strategic compiler barriers... */
FORCELOCAL bool KCALL
mutex_get_timed(struct mutex *__restrict self, jtime_t abs_timeout) {
 if (!__os_mutex_get_timed(self,abs_timeout))
      return false;
 COMPILER_BARRIER();
 return true;
}
FORCELOCAL ATTR_NOTHROW
void KCALL mutex_put(struct mutex *__restrict self) {
 COMPILER_BARRIER();
 __os_mutex_put(self);
}

#ifdef __INTELLISENSE__
FUNDEF void KCALL mutex_get(struct mutex *__restrict self);
FUNDEF void KCALL mutex_getf(struct mutex *__restrict self, iomode_t flags);
#else
#define mutex_get(self) \
       (void)mutex_get_timed(self,JTIME_INFINITE)
FORCELOCAL ATTR_NOTHROW bool KCALL
mutex_try(struct mutex *__restrict self) {
 if (!ATOMIC_CMPXCH(self->m_ind,0,1)) {
  if (self->m_owner == THIS_TASK) {
   ++self->m_ind;
   return true;
  }
  return false;
 }
 self->m_owner = THIS_TASK;
 COMPILER_BARRIER();
 return true;
}
LOCAL void KCALL
mutex_getf(struct mutex *__restrict self, iomode_t flags) {
 if (!mutex_try(self)) {
  if (flags & IO_NONBLOCK)
      error_throw(E_WOULDBLOCK);
  mutex_get(self);
 }
}
#endif


/* Connect to the signal of the given mutex and return
 * `true' if the mutex is currently available. */
FUNDEF bool KCALL mutex_poll(struct mutex *__restrict self);




DECL_END
#endif /* __CC__ */

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_MUTEX_H */
