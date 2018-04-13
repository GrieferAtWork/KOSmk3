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
#ifndef GUARD_LIBS_LIBC_SYNC_C
#define GUARD_LIBS_LIBC_SYNC_C 1

#include "libc.h"
#include "sync.h"
#include "unistd.h"
#include "sched.h"

#include <kos/sched/futex.h>
#include <kos/sched/mutex.h>
#include <hybrid/atomic.h>

DECL_BEGIN

EXPORT(mutex_get_timed,libc_mutex_get_timed);
INTERN int LIBCCALL
libc_mutex_get_timed(struct mutex *__restrict self,
                     struct timespec32 *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_mutex_get_timed64(self,NULL);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_mutex_get_timed64(self,&t64);
}


EXPORT(mutex_get_timed64,libc_mutex_get_timed64);
INTERN int LIBCCALL
libc_mutex_get_timed64(struct mutex *__restrict self,
                       struct timespec64 *abs_timeout) {
 u32 old_futex;
again:
 if ((old_futex = ATOMIC_CMPXCH_VAL(self->__m_futex,0,1)) != 0) {
ok:
  self->__m_owner = libc_gettid();
  return 0;
 }
 if (self->__m_owner == libc_gettid()) {
  /* Recursion (only when the indirection counter is non-ZERO). */
  ++self->__m_futex;
  return 0;
 }

 THREAD_POLL_BEFORE_CONNECT({
  if ((old_futex = ATOMIC_CMPXCH_VAL(self->__m_futex,0,1)) != 0)
       goto ok;
 });

 if (libc_futex64(&self->__m_futex,FUTEX_WAIT,old_futex,abs_timeout,NULL,0))
     return -1;
 goto again;
}

EXPORT(mutex_put,libc_mutex_put);
INTERN void LIBCCALL
libc_mutex_put(struct mutex *__restrict self) {
 assertf(self->__m_futex >= 1,"Noone is holding any locks");
 assertf(self->__m_owner == libc_gettid(),"You're not the owner of this mutex");
 /* Signal the first waiting task that the mutex is now free. */
 if (self->__m_futex == 1) {
  self->__m_owner = 0;
  self->__m_futex = 0;
  COMPILER_WRITE_BARRIER();
  libc_futex64(&self->__m_futex,FUTEX_WAKE,1,NULL,NULL,0);
 } else {
  --self->__m_futex;
 }
}




EXPORT(Xmutex_get_timed,libc_Xmutex_get_timed);
INTERN int LIBCCALL
libc_Xmutex_get_timed(struct mutex *__restrict self,
                      struct timespec32 *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_Xmutex_get_timed64(self,NULL);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xmutex_get_timed64(self,&t64);
}

EXPORT(Xmutex_get_timed64,libc_Xmutex_get_timed64);
INTERN int LIBCCALL
libc_Xmutex_get_timed64(struct mutex *__restrict self,
                        struct timespec64 *abs_timeout) {
 u32 old_futex;
again:
 if ((old_futex = ATOMIC_CMPXCH_VAL(self->__m_futex,0,1)) != 0) {
ok:
  self->__m_owner = libc_gettid();
  return 0;
 }
 if (self->__m_owner == libc_gettid()) {
  /* Recursion (only when the indirection counter is non-ZERO). */
  ++self->__m_futex;
  return 0;
 }
 THREAD_POLL_BEFORE_CONNECT({
  if ((old_futex = ATOMIC_CMPXCH_VAL(self->__m_futex,0,1)) != 0)
       goto ok;
 });
 if (libc_Xfutex64(&self->__m_futex,FUTEX_WAIT,old_futex,abs_timeout,NULL,0))
     return -1;
 goto again;
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_SYNC_C */
