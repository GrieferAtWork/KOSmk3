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
#ifndef GUARD_LIBS_LIBPTHREAD_THREADATTR_C
#define GUARD_LIBS_LIBPTHREAD_THREADATTR_C 1
#define _EXCEPT_SOURCE 1
#define _GNU_SOURCE 1

#include "libpthread.h"
#include "threadattr.h"
#include <hybrid/minmax.h>
#include <hybrid/atomic.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <sched.h>
#include <malloc.h>
#include <errno.h>
#include <kos/types.h>
#include <stdbool.h>
#include <string.h>
#include <except.h>

DECL_BEGIN

STATIC_ASSERT(sizeof(ThreadAttr) <= sizeof(pthread_attr_t));

INTERN DEFINE_ATOMIC_RWLOCK(thread_attr_default_lock);
INTERN ThreadAttr thread_attr_default = {
    .ta_guardsize = PAGESIZE,
    .ta_stacksize = 4*1024, /* Mustn't actually be true, just a guess (who cares?) */
};

EXPORT(pthread_getattr_default_np,thread_getattr_default_np);
INTERN errno_t LIBPCALL
thread_getattr_default_np(ThreadAttr *__restrict self) {
 atomic_rwlock_read(&thread_attr_default_lock);
 TRY {
  memcpy(self,&thread_attr_default,sizeof(ThreadAttr));
 } FINALLY {
  atomic_rwlock_endread(&thread_attr_default_lock);
 }
 return 0;
}

EXPORT(pthread_setattr_default_np,thread_setattr_default_np);
INTERN errno_t LIBPCALL
thread_setattr_default_np(ThreadAttr const *__restrict self) {
 errno_t result = 0;
 TRY {
  Xthread_setattr_default_np(self);
 } EXCEPT ((result = except_geterrno()) != 0
          ? EXCEPT_EXECUTE_HANDLER
          : EXCEPT_CONTINUE_SEARCH) {
 }
 return result;
}

EXPORT(Xpthread_getattr_default_np,Xthread_getattr_default_np);
DEFINE_INTERN_ALIAS(Xthread_getattr_default_np,thread_getattr_default_np);

EXPORT(Xpthread_setattr_default_np,Xthread_setattr_default_np);
INTERN void LIBPCALL
Xthread_setattr_default_np(ThreadAttr const *__restrict self) {
 /* Don't allow a default stack. */
 if (self->ta_flags & THREADATTR_FSTACKADDR)
     error_throw(E_INVALID_ARGUMENT);
 atomic_rwlock_write(&thread_attr_default_lock);
 TRY {
  /* Copy the default affinity. */
  if (!self->ta_cpuset_sz) {
   free(thread_attr_default.ta_cpuset);
   thread_attr_default.ta_cpuset    = NULL;
   thread_attr_default.ta_cpuset_sz = 0;
  } else {
   if (thread_attr_default.ta_cpuset_sz != self->ta_cpuset_sz) {
    thread_attr_default.ta_cpuset = (__cpu_set_t *)Xrealloc(thread_attr_default.ta_cpuset,
                                                            self->ta_cpuset_sz);
    thread_attr_default.ta_cpuset_sz = self->ta_cpuset_sz;
   }
   memcpy(thread_attr_default.ta_cpuset,self->ta_cpuset,self->ta_cpuset_sz);
  }
  /* Inherit values. */
  if (self->ta_stacksize)
      thread_attr_default.ta_stacksize = self->ta_stacksize;
  thread_attr_default.ta_guardsize   = self->ta_guardsize;
  thread_attr_default.ta_flags       = self->ta_flags;
  thread_attr_default.ta_schedpolicy = self->ta_schedpolicy;
  thread_attr_default.ta_schedparam  = self->ta_schedparam;
 } FINALLY {
  atomic_rwlock_endwrite(&thread_attr_default_lock);
 }
}




EXPORT(pthread_attr_init,thread_attr_init);
INTERN errno_t LIBPCALL
thread_attr_init(ThreadAttr *__restrict self) {
 memset(self,0,sizeof(ThreadAttr));
 return 0;
}
EXPORT(Xpthread_attr_init,Xthread_attr_init);
DEFINE_INTERN_ALIAS(Xthread_attr_init,thread_attr_init);


EXPORT(pthread_attr_destroy,thread_attr_destroy);
INTERN errno_t LIBPCALL
thread_attr_destroy(ThreadAttr *__restrict self) {
 if (self->ta_cpuset)
     free(self->ta_cpuset);
 (void)self;
 return 0;
}

EXPORT(pthread_attr_getdetachstate,thread_attr_getdetachstate);
INTERN errno_t LIBPCALL
thread_attr_getdetachstate(ThreadAttr const *__restrict self,
                           int *__restrict pdetachstate) {
 *pdetachstate = self->ta_flags & THREADATTR_FDETACHSTATE
               ? PTHREAD_CREATE_DETACHED
               : PTHREAD_CREATE_JOINABLE;
 return 0;
}

EXPORT(Xpthread_attr_getdetachstate,Xthread_attr_getdetachstate);
INTERN int LIBPCALL
Xthread_attr_getdetachstate(ThreadAttr const *__restrict self) {
 return self->ta_flags & THREADATTR_FDETACHSTATE
        ? PTHREAD_CREATE_DETACHED
        : PTHREAD_CREATE_JOINABLE;
}

EXPORT(pthread_attr_setdetachstate,thread_attr_setdetachstate);
INTERN errno_t LIBPCALL
thread_attr_setdetachstate(ThreadAttr *__restrict self, int detachstate) {
 if ((unsigned int)detachstate > PTHREAD_CREATE_DETACHED)
     return EINVAL;
 if (detachstate == PTHREAD_CREATE_DETACHED) {
  self->ta_flags |= THREADATTR_FDETACHSTATE;
 } else {
  self->ta_flags &= ~THREADATTR_FDETACHSTATE;
 }
 return 0;
}

EXPORT(Xpthread_attr_setdetachstate,Xthread_attr_setdetachstate);
INTERN void LIBPCALL
Xthread_attr_setdetachstate(ThreadAttr *__restrict self, int detachstate) {
 if ((unsigned int)detachstate > PTHREAD_CREATE_DETACHED)
     error_throw(E_INVALID_ARGUMENT);
 if (detachstate == PTHREAD_CREATE_DETACHED) {
  self->ta_flags |= THREADATTR_FDETACHSTATE;
 } else {
  self->ta_flags &= ~THREADATTR_FDETACHSTATE;
 }
}


EXPORT(pthread_attr_getguardsize,thread_attr_getguardsize);
INTERN errno_t LIBPCALL
thread_attr_getguardsize(ThreadAttr const *__restrict self,
                         size_t *__restrict pguardsize) {
 *pguardsize = self->ta_guardsize;
 return 0;
}

EXPORT(pthread_attr_setguardsize,thread_attr_setguardsize);
INTERN errno_t LIBPCALL
thread_attr_setguardsize(ThreadAttr *__restrict self,
                         size_t guardsize) {
 self->ta_guardsize = guardsize;
 return 0;
}

EXPORT(Xpthread_attr_getguardsize,Xthread_attr_getguardsize);
INTERN size_t LIBPCALL
Xthread_attr_getguardsize(ThreadAttr const *__restrict self) {
 return self->ta_guardsize;
}

EXPORT(Xpthread_attr_setguardsize,Xthread_attr_setguardsize);
INTERN void LIBPCALL
Xthread_attr_setguardsize(ThreadAttr *__restrict self,
                          size_t guardsize) {
 self->ta_guardsize = guardsize;
}


EXPORT(pthread_attr_getschedparam,thread_attr_getschedparam);
INTERN errno_t LIBPCALL
thread_attr_getschedparam(ThreadAttr const *__restrict self,
                          struct sched_param *__restrict param) {
 memcpy(param,&self->ta_schedparam,sizeof(struct sched_param));
 return 0;
}

EXPORT(pthread_attr_setschedparam,thread_attr_setschedparam);
INTERN errno_t LIBPCALL
thread_attr_setschedparam(ThreadAttr *__restrict self,
                          struct sched_param const *__restrict param) {
 memcpy(&self->ta_schedparam,param,sizeof(struct sched_param));
 return 0;
}


EXPORT(Xpthread_attr_getschedparam,Xthread_attr_getschedparam);
DEFINE_INTERN_ALIAS(Xthread_attr_getschedparam,thread_attr_getschedparam);
EXPORT(Xpthread_attr_setschedparam,Xthread_attr_setschedparam);
DEFINE_INTERN_ALIAS(Xthread_attr_setschedparam,thread_attr_setschedparam);



EXPORT(pthread_attr_getschedpolicy,thread_attr_getschedpolicy);
INTERN errno_t LIBPCALL
thread_attr_getschedpolicy(ThreadAttr const *__restrict self,
                           int *__restrict ppolicy) {
 *ppolicy = self->ta_schedpolicy;
 return 0;
}
EXPORT(pthread_attr_setschedpolicy,thread_attr_setschedpolicy);
INTERN errno_t LIBPCALL
thread_attr_setschedpolicy(ThreadAttr *__restrict self, int policy) {
 self->ta_schedpolicy = policy;
 return 0;
}

EXPORT(Xpthread_attr_getschedpolicy,Xthread_attr_getschedpolicy);
DEFINE_INTERN_ALIAS(Xthread_attr_getschedpolicy,thread_attr_getschedpolicy);
EXPORT(Xpthread_attr_setschedpolicy,Xthread_attr_setschedpolicy);
DEFINE_INTERN_ALIAS(Xthread_attr_setschedpolicy,thread_attr_setschedpolicy);


EXPORT(pthread_attr_getinheritsched,thread_attr_getinheritsched);
INTERN errno_t LIBPCALL
thread_attr_getinheritsched(ThreadAttr const *__restrict self,
                           int *__restrict pinhert) {
 *pinhert = self->ta_flags & THREADATTR_FNOTINHERITSCHED
          ? PTHREAD_EXPLICIT_SCHED
          : PTHREAD_INHERIT_SCHED;
 return 0;
}
EXPORT(pthread_attr_setinheritsched,thread_attr_setinheritsched);
INTERN errno_t LIBPCALL
thread_attr_setinheritsched(ThreadAttr *__restrict self, int inhert) {
 if ((unsigned int)inhert > PTHREAD_EXPLICIT_SCHED)
     return EINVAL;
 if (inhert == PTHREAD_EXPLICIT_SCHED) {
  self->ta_flags &= ~THREADATTR_FNOTINHERITSCHED;
 } else {
  self->ta_flags |= THREADATTR_FNOTINHERITSCHED;
 }
 return 0;
}

EXPORT(Xpthread_attr_getinheritsched,Xthread_attr_getinheritsched);
INTERN int LIBPCALL
Xthread_attr_getinheritsched(ThreadAttr const *__restrict self) {
 return self->ta_flags & THREADATTR_FNOTINHERITSCHED
      ? PTHREAD_EXPLICIT_SCHED
      : PTHREAD_INHERIT_SCHED;
}
EXPORT(Xpthread_attr_setinheritsched,Xthread_attr_setinheritsched);
INTERN void LIBPCALL
Xthread_attr_setinheritsched(ThreadAttr *__restrict self, int inhert) {
 if ((unsigned int)inhert > PTHREAD_EXPLICIT_SCHED)
     error_throw(E_INVALID_ARGUMENT);
 if (inhert == PTHREAD_EXPLICIT_SCHED) {
  self->ta_flags &= ~THREADATTR_FNOTINHERITSCHED;
 } else {
  self->ta_flags |= THREADATTR_FNOTINHERITSCHED;
 }
}


EXPORT(pthread_attr_getscope,thread_attr_getscope);
INTERN errno_t LIBPCALL
thread_attr_getscope(ThreadAttr const *__restrict self,
                     int *__restrict pscope) {
 *pscope = self->ta_flags & THREADATTR_FSCOPEPROCESS
         ? PTHREAD_SCOPE_PROCESS
         : PTHREAD_SCOPE_SYSTEM;
 return 0;
}
EXPORT(pthread_attr_setscope,thread_attr_setscope);
INTERN errno_t LIBPCALL
thread_attr_setscope(ThreadAttr *__restrict self, int scope) {
 if ((unsigned int)scope > PTHREAD_SCOPE_PROCESS)
     return EINVAL;
 if (scope == PTHREAD_SCOPE_PROCESS) {
  self->ta_flags |= THREADATTR_FSCOPEPROCESS;
 } else {
  self->ta_flags &= ~THREADATTR_FSCOPEPROCESS;
 }
 return 0;
}

EXPORT(Xpthread_attr_getscope,Xthread_attr_getscope);
INTERN int LIBPCALL
Xthread_attr_getscope(ThreadAttr const *__restrict self) {
 return self->ta_flags & THREADATTR_FSCOPEPROCESS
      ? PTHREAD_SCOPE_PROCESS
      : PTHREAD_SCOPE_SYSTEM;
}
EXPORT(Xpthread_attr_setscope,Xthread_attr_setscope);
INTERN void LIBPCALL
Xthread_attr_setscope(ThreadAttr *__restrict self, int scope) {
 if ((unsigned int)scope > PTHREAD_SCOPE_PROCESS)
     error_throw(E_INVALID_ARGUMENT);
 if (scope == PTHREAD_SCOPE_PROCESS) {
  self->ta_flags |= THREADATTR_FSCOPEPROCESS;
 } else {
  self->ta_flags &= ~THREADATTR_FSCOPEPROCESS;
 }
}

EXPORT(pthread_attr_getstackaddr,thread_attr_getstackaddr);
INTERN errno_t LIBPCALL
thread_attr_getstackaddr(ThreadAttr const *__restrict self, void **__restrict pstackaddr) {
 *pstackaddr = self->ta_stackaddr;
 return 0;
}
EXPORT(pthread_attr_setstackaddr,thread_attr_setstackaddr);
INTERN errno_t LIBPCALL
thread_attr_setstackaddr(ThreadAttr *__restrict self, void *stackaddr) {
 self->ta_stackaddr = stackaddr;
 self->ta_flags    |= THREADATTR_FSTACKADDR;
 return 0;
}
EXPORT(Xpthread_attr_getstackaddr,Xthread_attr_getstackaddr);
INTERN void *LIBPCALL
Xthread_attr_getstackaddr(ThreadAttr const *__restrict self) {
 return self->ta_stackaddr;
}
EXPORT(Xpthread_attr_setstackaddr,Xthread_attr_setstackaddr);
DEFINE_INTERN_ALIAS(Xthread_attr_setstackaddr,thread_attr_setstackaddr);

EXPORT(pthread_attr_getstacksize,thread_attr_getstacksize);
INTERN errno_t LIBPCALL
thread_attr_getstacksize(ThreadAttr const *__restrict self,
                         size_t *__restrict pstacksize) {
 size_t result = self->ta_stacksize;
 if (!result)
      result = ATOMIC_READ(thread_attr_default.ta_stacksize);
 *pstacksize = result;
 return 0;
}

EXPORT(pthread_attr_setstacksize,thread_attr_setstacksize);
INTERN errno_t LIBPCALL
thread_attr_setstacksize(ThreadAttr *__restrict self,
                         size_t stacksize) {
 self->ta_stacksize = stacksize;
 return 0;
}

EXPORT(Xpthread_attr_getstacksize,Xthread_attr_getstacksize);
INTERN size_t LIBPCALL
Xthread_attr_getstacksize(ThreadAttr const *__restrict self) {
 size_t result = self->ta_stacksize;
 if (!result)
      result = ATOMIC_READ(thread_attr_default.ta_stacksize);
 return result;
}

EXPORT(Xpthread_attr_setstacksize,Xthread_attr_setstacksize);
DEFINE_INTERN_ALIAS(Xthread_attr_setstacksize,thread_attr_setstacksize);

EXPORT(pthread_attr_getstack,thread_attr_getstack);
INTERN errno_t LIBPCALL
thread_attr_getstack(ThreadAttr const *__restrict self,
                     void **pstackaddr, size_t *pstacksize) {
 void  *base = self->ta_stackaddr;
 size_t size = self->ta_stacksize;
#ifndef __ARCH_STACK_GROWS_UP
 base = (void *)((uintptr_t)base - size);
#endif
 if (pstackaddr) *pstackaddr = base;
 if (pstacksize) *pstacksize = size;
 return 0;
}

EXPORT(pthread_attr_setstack,thread_attr_setstack);
INTERN errno_t LIBPCALL
thread_attr_setstack(ThreadAttr *__restrict self,
                     void *stackaddr, size_t stacksize) {
 self->ta_stacksize = stacksize;
 self->ta_flags    |= THREADATTR_FSTACKADDR;
#ifndef __ARCH_STACK_GROWS_UP
 self->ta_stackaddr = (void *)((uintptr_t)stackaddr + stacksize);
#else
 self->ta_stackaddr = stackaddr;
#endif
 return 0;
}

EXPORT(Xpthread_attr_getstack,Xthread_attr_getstack);
DEFINE_INTERN_ALIAS(Xthread_attr_getstack,thread_attr_getstack);
EXPORT(Xpthread_attr_setstack,Xthread_attr_setstack);
DEFINE_INTERN_ALIAS(Xthread_attr_setstack,thread_attr_setstack);

EXPORT(pthread_attr_setaffinity_np,thread_attr_setaffinity_np);
INTERN errno_t LIBPCALL
thread_attr_setaffinity_np(ThreadAttr *__restrict self, size_t cpusetsize,
                           __cpu_set_t const *__restrict cpuset) {
 if (!cpusetsize || !cpuset) {
  free(self->ta_cpuset);
  self->ta_cpuset    = NULL;
  self->ta_cpuset_sz = 0;
 } else {
  if (self->ta_cpuset_sz != cpusetsize) {
   cpu_set_t *new_set;
   new_set = (cpu_set_t *)realloc(self->ta_cpuset,cpusetsize);
   if (!new_set) return ENOMEM;
   self->ta_cpuset    = new_set;
   self->ta_cpuset_sz = cpusetsize;
  }
  memcpy(&self->ta_cpuset,cpuset,cpusetsize);
 }
 return 0;
}

EXPORT(pthread_attr_getaffinity_np,thread_attr_getaffinity_np);
INTERN errno_t LIBPCALL
thread_attr_getaffinity_np(ThreadAttr const *__restrict self,
                           size_t cpusetsize, __cpu_set_t *__restrict cpuset) {
 if (self->ta_cpuset_sz == 0) {
  /* All cpus by default. */
  memset(cpuset,0xff,cpusetsize);
 } else {
  /* Make sure that no CPUs are set beyond the requested size. */
  size_t i;
  for (i = cpusetsize; i < self->ta_cpuset_sz; ++i)
      if (((byte_t *)self->ta_cpuset)[i] != 0)
           return EINVAL; /* XXX: ERANGE? */
  cpuset = (__cpu_set_t *)mempcpy(cpuset,self->ta_cpuset,
                                  MIN(self->ta_cpuset_sz,cpusetsize));
  if (cpusetsize > self->ta_cpuset_sz)
      memset(cpuset,0,cpusetsize-self->ta_cpuset_sz);
 }
 return 0;
}

EXPORT(Xpthread_attr_setaffinity_np,Xthread_attr_setaffinity_np);
INTERN void LIBPCALL
Xthread_attr_setaffinity_np(ThreadAttr *__restrict self, size_t cpusetsize,
                            __cpu_set_t const *__restrict cpuset) {
 if (!cpusetsize || !cpuset) {
  free(self->ta_cpuset);
  self->ta_cpuset    = NULL;
  self->ta_cpuset_sz = 0;
 } else {
  if (self->ta_cpuset_sz != cpusetsize) {
   cpu_set_t *new_set;
   new_set = (cpu_set_t *)Xrealloc(self->ta_cpuset,cpusetsize);
   self->ta_cpuset    = new_set;
   self->ta_cpuset_sz = cpusetsize;
  }
  memcpy(&self->ta_cpuset,cpuset,cpusetsize);
 }
}

EXPORT(Xpthread_attr_getaffinity_np,Xthread_attr_getaffinity_np);
INTERN void LIBPCALL
Xthread_attr_getaffinity_np(ThreadAttr const *__restrict self,
                            size_t cpusetsize, __cpu_set_t *__restrict cpuset) {
 if (self->ta_cpuset_sz == 0) {
  /* All cpus by default. */
  memset(cpuset,0xff,cpusetsize);
 } else {
  /* Make sure that no CPUs are set beyond the requested size. */
  size_t i;
  for (i = cpusetsize; i < self->ta_cpuset_sz; ++i) {
   if (((byte_t *)self->ta_cpuset)[i] != 0) {
    size_t last_req = i;
    while (i < self->ta_cpuset_sz)
       if (((byte_t *)self->ta_cpuset)[i] != 0) last_req = i;
    error_throwf(E_BUFFER_TOO_SMALL,cpusetsize,last_req+1);
   }
  }
  cpuset = (__cpu_set_t *)mempcpy(cpuset,self->ta_cpuset,
                                  MIN(self->ta_cpuset_sz,cpusetsize));
  if (cpusetsize > self->ta_cpuset_sz)
      memset(cpuset,0,cpusetsize-self->ta_cpuset_sz);
 }
}

DECL_END

#endif /* !GUARD_LIBS_LIBPTHREAD_THREADATTR_C */
