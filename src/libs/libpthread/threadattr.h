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
#ifndef GUARD_LIBS_LIBPTHREAD_THREADATTR_H
#define GUARD_LIBS_LIBPTHREAD_THREADATTR_H 1

#include "libpthread.h"
#include <kos/types.h>
#include <stdbool.h>
#include <bits/sched.h>
#include <hybrid/sync/atomic-rwlock.h>

DECL_BEGIN

typedef struct thread_attr ThreadAttr;
struct thread_attr {
    /* NOTE: The layout of this structure has
     *       binary compatibility with glibc! */
    struct sched_param ta_schedparam;
    int                ta_schedpolicy;
    int                ta_flags;       /* Set of `THREADATTR_F*' */
#define THREADATTR_FDETACHSTATE     0x0001
#define THREADATTR_FNOTINHERITSCHED 0x0002
#define THREADATTR_FSCOPEPROCESS    0x0004
#define THREADATTR_FSTACKADDR       0x0008
#define THREADATTR_FOLDATTR         0x0010
#define THREADATTR_FSCHED_SET       0x0020
#define THREADATTR_FPOLICY_SET      0x0040
    size_t             ta_guardsize;
    void              *ta_stackaddr;
    size_t             ta_stacksize;
    __cpu_set_t       *ta_cpuset;
    size_t             ta_cpuset_sz;
};

INTERN errno_t LIBPCALL thread_attr_init(ThreadAttr *__restrict self);
INTERN errno_t LIBPCALL thread_attr_destroy(ThreadAttr *__restrict self);
INTERN errno_t LIBPCALL thread_attr_getdetachstate(ThreadAttr const *__restrict self, int *__restrict pdetachstate);
INTERN errno_t LIBPCALL thread_attr_setdetachstate(ThreadAttr *__restrict self, int detachstate);
INTERN errno_t LIBPCALL thread_attr_getguardsize(ThreadAttr const *__restrict self, size_t *__restrict pguardsize);
INTERN errno_t LIBPCALL thread_attr_setguardsize(ThreadAttr *__restrict self, size_t guardsize);
INTERN errno_t LIBPCALL thread_attr_getschedparam(ThreadAttr const *__restrict self, struct sched_param *__restrict param);
INTERN errno_t LIBPCALL thread_attr_setschedparam(ThreadAttr *__restrict self, struct sched_param const *__restrict param);
INTERN errno_t LIBPCALL thread_attr_getschedpolicy(ThreadAttr const *__restrict self, int *__restrict ppolicy);
INTERN errno_t LIBPCALL thread_attr_setschedpolicy(ThreadAttr *__restrict self, int policy);
INTERN errno_t LIBPCALL thread_attr_getinheritsched(ThreadAttr const *__restrict self, int *__restrict pinherit);
INTERN errno_t LIBPCALL thread_attr_setinheritsched(ThreadAttr *__restrict self, int inherit);
INTERN errno_t LIBPCALL thread_attr_getscope(ThreadAttr const *__restrict self, int *__restrict pscope);
INTERN errno_t LIBPCALL thread_attr_setscope(ThreadAttr *__restrict self, int scope);
INTERN errno_t LIBPCALL thread_attr_getstackaddr(ThreadAttr const *__restrict self, void **__restrict pstackaddr);
INTERN errno_t LIBPCALL thread_attr_setstackaddr(ThreadAttr *__restrict self, void *stackaddr);
INTERN errno_t LIBPCALL thread_attr_getstacksize(ThreadAttr const *__restrict self, size_t *__restrict pstacksize);
INTERN errno_t LIBPCALL thread_attr_setstacksize(ThreadAttr *__restrict self, size_t stacksize);
INTERN errno_t LIBPCALL thread_attr_getstack(ThreadAttr const *__restrict self, void **pstackaddr, size_t *pstacksize);
INTERN errno_t LIBPCALL thread_attr_setstack(ThreadAttr *__restrict self, void *stackaddr, size_t stacksize);
INTERN void LIBPCALL Xthread_attr_init(ThreadAttr *__restrict self);
INTERN int LIBPCALL Xthread_attr_getdetachstate(ThreadAttr const *__restrict self);
INTERN void LIBPCALL Xthread_attr_setdetachstate(ThreadAttr *__restrict self, int detachstate);
INTERN size_t LIBPCALL Xthread_attr_getguardsize(ThreadAttr const *__restrict self);
INTERN void LIBPCALL Xthread_attr_setguardsize(ThreadAttr *__restrict self, size_t guardsize);
INTERN void LIBPCALL Xthread_attr_getschedparam(ThreadAttr const *__restrict self, struct sched_param *__restrict param);
INTERN void LIBPCALL Xthread_attr_setschedparam(ThreadAttr *__restrict self, struct sched_param const *__restrict param);
INTERN int LIBPCALL Xthread_attr_getschedpolicy(ThreadAttr const *__restrict self);
INTERN void LIBPCALL Xthread_attr_setschedpolicy(ThreadAttr *__restrict self, int policy);
INTERN int LIBPCALL Xthread_attr_getinheritsched(ThreadAttr const *__restrict self);
INTERN void LIBPCALL Xthread_attr_setinheritsched(ThreadAttr *__restrict self, int inherit);
INTERN int LIBPCALL Xthread_attr_getscope(ThreadAttr const *__restrict self);
INTERN void LIBPCALL Xthread_attr_setscope(ThreadAttr *__restrict self, int scope);
INTERN void *LIBPCALL Xthread_attr_getstackaddr(ThreadAttr const *__restrict self);
INTERN void LIBPCALL Xthread_attr_setstackaddr(ThreadAttr *__restrict self, void *stackaddr);
INTERN size_t LIBPCALL Xthread_attr_getstacksize(ThreadAttr const *__restrict self);
INTERN void LIBPCALL Xthread_attr_setstacksize(ThreadAttr *__restrict self, size_t stacksize);
INTERN void LIBPCALL Xthread_attr_getstack(ThreadAttr const *__restrict self, void **pstackaddr, size_t *__pstacksize);
INTERN void LIBPCALL Xthread_attr_setstack(ThreadAttr *__restrict self, void *stackaddr, size_t stacksize);

INTDEF atomic_rwlock_t thread_attr_default_lock;
INTDEF ThreadAttr thread_attr_default;

INTERN errno_t LIBPCALL thread_attr_setaffinity_np(ThreadAttr *__restrict self, size_t cpusetsize, __cpu_set_t const *__restrict cpuset);
INTERN errno_t LIBPCALL thread_attr_getaffinity_np(ThreadAttr const *__restrict self, size_t cpusetsize, __cpu_set_t *__restrict cpuset);
INTERN errno_t LIBPCALL thread_getattr_default_np(ThreadAttr *__restrict self);
INTERN errno_t LIBPCALL thread_setattr_default_np(ThreadAttr const *__restrict self);
INTERN void LIBPCALL Xthread_attr_setaffinity_np(ThreadAttr *__restrict self, size_t cpusetsize, __cpu_set_t const *__restrict cpuset);
INTERN void LIBPCALL Xthread_attr_getaffinity_np(ThreadAttr const *__restrict self, size_t cpusetsize, __cpu_set_t *__restrict cpuset);
INTERN void LIBPCALL Xthread_getattr_default_np(ThreadAttr *__restrict self);
INTERN void LIBPCALL Xthread_setattr_default_np(ThreadAttr const *__restrict self);


DECL_END

#endif /* !GUARD_LIBS_LIBPTHREAD_THREADATTR_H */
