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
#ifndef _PARTS_KOS3_EXCEPT_SYS_TIME_H
#define _PARTS_KOS3_EXCEPT_SYS_TIME_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#ifndef _SYS_GENERIC_TIME_H
#include <sys/time.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

/* TODO: GLibc compatibility mode. */
__REDIRECT_TM64_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xgettimeofday,(struct timeval *__restrict __tv, __timezone_ptr_t __tz),(__tv,__tz))
__REDIRECT_TM64_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xgetitimer,(__itimer_which_t __which, struct itimerval *__value),(__which,__value))
__REDIRECT_TM64_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xsetitimer,(__itimer_which_t __which, struct itimerval const *__restrict __new, struct itimerval *__restrict __old),(__which,__new,__old))
__REDIRECT_TM64_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xutimes,(char const *__file, struct timeval const __tvp[2]),(__file,__tvp))
#ifdef __USE_GNU
__REDIRECT_TM64_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xfutimesat,(__fd_t __fd, char const *__file, struct timeval const __tvp[2]),(__fd,__file,__tvp))
#endif /* __USE_GNU */
#ifdef __USE_MISC
__REDIRECT_TM64_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xsettimeofday,(struct timeval const *__tv, struct timezone const *__tz),(__tv,__tz))
__REDIRECT_TM64_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xadjtime,(struct timeval const *__delta, struct timeval *__olddelta),(__delta,__olddelta))
__REDIRECT_TM64_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xlutimes,(char const *__file, struct timeval const __tvp[2]),(__file,__tvp))
__REDIRECT_TM64_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xfutimes,(__fd_t __fd, struct timeval const __tvp[2]),(__fd,__tvp))
#endif /* __USE_MISC */

#ifdef __USE_TIME64
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xgettimeofday64)(struct timeval64 *__restrict __tv, __timezone_ptr_t __tz);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xgetitimer64)(__itimer_which_t __which, struct itimerval64 *__value);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsetitimer64)(__itimer_which_t __which, struct itimerval64 const *__restrict __new, struct itimerval64 *__restrict __old);
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xutimes64)(char const *__file, struct timeval64 const __tvp[2]);
#ifdef __USE_MISC
__LIBC __PORT_KOSONLY void (__LIBCCALL Xsettimeofday64)(struct timeval64 const *__tv, struct timezone const *__tz);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xadjtime64)(struct timeval64 const *__delta, struct timeval64 *__olddelta);
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xlutimes64)(char const *__file, struct timeval64 const __tvp[2]);
__LIBC __PORT_KOSONLY void (__LIBCCALL Xfutimes64)(__fd_t __fd, struct timeval64 const __tvp[2]);
#endif /* __USE_MISC */
#ifdef __USE_GNU
__LIBC __PORT_KOSONLY void (__LIBCCALL Xfutimesat64)(__fd_t __fd, char const *__file, struct timeval64 const __tvp[2]);
#endif /* __USE_GNU */
#endif /* __USE_TIME64 */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SYS_TIME_H */
