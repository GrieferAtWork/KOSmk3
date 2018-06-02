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
#ifndef _PARTS_KOS3_EXCEPT_SCHED_H
#define _PARTS_KOS3_EXCEPT_SCHED_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#ifndef _SCHED_H
#include <sched.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

#ifdef __USE_GNU

/* @return: 0: Another thread was executed prior to the function returning.
 *             The thread may not necessarily be apart of the calling process.
 * @return: 1: The function returned immediately when no other thread was executed. */
__LIBC __PORT_KOSONLY int (__LIBCCALL Xsched_yield)(void);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsched_setparam)(__pid_t __pid, struct sched_param const *__param);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsched_getparam)(__pid_t __pid, struct sched_param *__param);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsched_setscheduler)(__pid_t __pid, int __policy, struct sched_param const *__param);
__LIBC __PORT_KOSONLY int (__LIBCCALL Xsched_getscheduler)(__pid_t __pid);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsched_setaffinity)(__pid_t __pid, __size_t __cpusetsize, cpu_set_t const *__cpuset);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsched_getaffinity)(__pid_t __pid, __size_t __cpusetsize, cpu_set_t *__cpuset);
__REDIRECT_TM64(__LIBC,__PORT_KOSONLY,void,__LIBCCALL,Xsched_rr_get_interval,(__pid_t __pid, struct timespec *__t),(__pid,__t))
#ifdef __USE_TIME64
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsched_rr_get_interval64)(__pid_t __pid, struct __timespec64 *__t);
#endif /* __USE_TIME64 */

#endif /* __USE_GNU */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SCHED_H */
