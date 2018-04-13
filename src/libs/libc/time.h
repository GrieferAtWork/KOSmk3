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
#ifndef GUARD_LIBS_LIBC_TIME_H
#define GUARD_LIBS_LIBC_TIME_H 1

#include "libc.h"
#include <kos/types.h>
#include <stdarg.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     TIME                                                                              */
/* ===================================================================================== */
struct tm;
struct tms;
struct timeval64;
struct timespec64;
struct itimerval64;
struct itimerspec64;
struct timeb64;
struct timezone;
INTDEF void LIBCCALL _libc_crtSleep(u32 milliseconds);
INTDEF unsigned int LIBCCALL libc_sleep(unsigned int seconds);
INTDEF unsigned int LIBCCALL libc_alarm(unsigned int seconds);
INTDEF int LIBCCALL libc_usleep(useconds_t useconds);
INTDEF useconds_t LIBCCALL libc_ualarm(useconds_t value, useconds_t interval);
INTDEF clock_t LIBCCALL libc_clock(void);
INTDEF clock_t LIBCCALL libc_dos_clock(void);
INTDEF clock_t LIBCCALL libc_times(struct tms *__restrict buffer);
INTDEF clock_t LIBCCALL libc_dos_times(struct tms *__restrict buffer);
INTDEF time32_t LIBCCALL libc_time(time32_t *timer);
INTDEF time64_t LIBCCALL libc_time64(time64_t *timer);
INTDEF time32_t LIBCCALL libc_timelocal(struct tm *tp);
INTDEF time64_t LIBCCALL libc_timelocal64(struct tm *tp);
INTDEF time32_t LIBCCALL libc_mktime(struct tm const *__restrict tp);
INTDEF time64_t LIBCCALL libc_mktime64(struct tm const *__restrict tp);
INTDEF time32_t LIBCCALL libc_mkgmtime(struct tm const *__restrict tp);
INTDEF time64_t LIBCCALL libc_mkgmtime64(struct tm const *__restrict tp);
INTDEF u32 LIBCCALL libc_getsystime(struct tm *__restrict tp);
INTDEF u32 LIBCCALL libc_setsystime(struct tm const *__restrict tp, u32 msec);
INTDEF struct tm *LIBCCALL libc_gmtime(time32_t const *timer);
INTDEF struct tm *LIBCCALL libc_gmtime64(time64_t const *timer);
INTDEF struct tm *LIBCCALL libc_localtime(time32_t const *timer);
INTDEF struct tm *LIBCCALL libc_localtime64(time64_t const *timer);
INTDEF struct tm *LIBCCALL libc_gmtime_r(time32_t const *__restrict timer, struct tm *__restrict tp);
INTDEF struct tm *LIBCCALL libc_gmtime64_r(time64_t const *__restrict timer, struct tm *__restrict tp);
INTDEF struct tm *LIBCCALL libc_localtime_r(time32_t const *__restrict timer, struct tm *__restrict tp);
INTDEF struct tm *LIBCCALL libc_localtime64_r(time64_t const *__restrict timer, struct tm *__restrict tp);
INTDEF derrno_t LIBCCALL libc_gmtime_s(struct tm *__restrict tp, time32_t const *__restrict timer);
INTDEF derrno_t LIBCCALL libc_gmtime64_s(struct tm *__restrict tp, time64_t const *__restrict timer);
INTDEF derrno_t LIBCCALL libc_localtime_s(struct tm *__restrict tp, time32_t const *__restrict timer);
INTDEF derrno_t LIBCCALL libc_localtime64_s(struct tm *__restrict tp, time64_t const *__restrict timer);
INTDEF double LIBCCALL libc_difftime(time32_t time1, time32_t time0);
INTDEF double LIBCCALL libc_difftime64(time64_t time1, time64_t time0);
INTDEF int LIBCCALL libc_timer_create(clockid_t clock_id, struct sigevent *__restrict evp, timer_t *__restrict timerid);
INTDEF int LIBCCALL libc_timer_delete(timer_t timerid);
INTDEF int LIBCCALL libc_timer_getoverrun(timer_t timerid);
INTDEF int LIBCCALL libc_clock_getcpuclockid(pid_t pid, clockid_t *clock_id);
INTDEF int LIBCCALL libc_timespec_get(struct timespec32 *ts, int base);
INTDEF int LIBCCALL libc_stime(time32_t const *when);
INTDEF int LIBCCALL libc_stime64(time64_t const *when);
INTDEF int LIBCCALL libc_settimeofday(struct timeval32 const *tv, struct timezone const *tz);
INTDEF int LIBCCALL libc_settimeofday64(struct timeval64 const *tv, struct timezone const *tz);
INTDEF int LIBCCALL libc_gettimeofday(struct timeval32 *__restrict tv, struct timezone *__restrict tz);
INTDEF int LIBCCALL libc_gettimeofday64(struct timeval64 *__restrict tv, struct timezone *__restrict tz);
INTDEF int LIBCCALL libc_nanosleep(struct timespec32 const *requested_time, struct timespec32 *remaining);
INTDEF int LIBCCALL libc_nanosleep64(struct timespec64 const *requested_time, struct timespec64 *remaining);
INTDEF int LIBCCALL libc_adjtime(struct timeval32 const *delta, struct timeval32 *olddelta);
INTDEF int LIBCCALL libc_adjtime64(struct timeval64 const *delta, struct timeval64 *olddelta);
INTDEF int LIBCCALL libc_getitimer(int which, struct itimerval32 *value);
INTDEF int LIBCCALL libc_getitimer64(int which, struct itimerval64 *value);
INTDEF int LIBCCALL libc_setitimer(int which, struct itimerval32 const *__restrict value, struct itimerval32 *ovalue);
INTDEF int LIBCCALL libc_setitimer64(int which, struct itimerval64 const *__restrict value, struct itimerval64 *ovalue);
INTDEF int LIBCCALL libc_clock_getres(clockid_t clock_id, struct timespec32 *res);
INTDEF int LIBCCALL libc_clock_getres64(clockid_t clock_id, struct timespec64 *res);
INTDEF int LIBCCALL libc_clock_gettime(clockid_t clock_id, struct timespec32 *tp);
INTDEF int LIBCCALL libc_clock_gettime64(clockid_t clock_id, struct timespec64 *tp);
INTDEF int LIBCCALL libc_clock_settime(clockid_t clock_id, struct timespec32 const *tp);
INTDEF int LIBCCALL libc_clock_settime64(clockid_t clock_id, struct timespec64 const *tp);
INTDEF int LIBCCALL libc_timer_settime(timer_t timerid, int flags, struct itimerspec32 const *__restrict value, struct itimerspec32 *ovalue);
INTDEF int LIBCCALL libc_timer_settime64(timer_t timerid, int flags, struct itimerspec64 const *__restrict value, struct itimerspec64 *ovalue);
INTDEF int LIBCCALL libc_timer_gettime(timer_t timerid, struct itimerspec32 *value);
INTDEF int LIBCCALL libc_timer_gettime64(timer_t timerid, struct itimerspec64 *value);
INTDEF int LIBCCALL libc_clock_nanosleep(clockid_t clock_id, int flags, struct timespec32 const *requested_time, struct timespec32 *remaining);
INTDEF int LIBCCALL libc_clock_nanosleep64(clockid_t clock_id, int flags, struct timespec64 const *requested_time, struct timespec64 *remaining);
INTDEF int LIBCCALL libc_ftime(struct timeb32 *timebuf);
INTDEF int LIBCCALL libc_ftime64(struct timeb64 *timebuf);
INTDEF derrno_t LIBCCALL libc_ftime_s(struct timeb *timebuf);
INTDEF derrno_t LIBCCALL libc_ftime64_s(struct timeb64 *timebuf);
/* Time zone information */
DATDEF char *tzname[2];
DATDEF int daylight;
DATDEF long int timezone; /* Seconds West of UTC (Universal time); aka.: gmtime()+timezone == localtime(). */
INTDEF void LIBCCALL libc_tzset(void);
INTDEF s32 *LIBCCALL libc_p_timezone(void);
INTDEF s32 *LIBCCALL libc_daylight(void);
INTDEF s32 *LIBCCALL libc_timezone(void);
INTDEF derrno_t LIBCCALL libc_get_daylight(s32 *pres);
INTDEF derrno_t LIBCCALL libc_get_timezone(s32 *pres);
INTDEF char **LIBCCALL libc_tzname(void);
INTDEF derrno_t LIBCCALL libc_get_tzname(size_t *pres, char *buf, size_t buflen, int idx);
INTDEF int LIBCCALL libc_dysize(int year);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_TIME_H */
