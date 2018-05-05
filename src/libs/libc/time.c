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
#ifndef GUARD_LIBS_LIBC_TIME_C
#define GUARD_LIBS_LIBC_TIME_C 1
#define _TIME64_SOURCE 1

#include "libc.h"
#include "time.h"
#include "errno.h"
#include "system.h"

#include <hybrid/align.h>
#include <hybrid/section.h>
#include <time.h>
#include <errno.h>
#include <bits/dos-errno.h>
#include <hybrid/timeutil.h>
#include <sys/timeb.h>
#include <sys/times.h>

DECL_BEGIN

PRIVATE clock_t clock_start = -1;
INTERN clock_t LIBCCALL libc_clock(void) {
 struct timeval64 now; clock_t result;
 /* Really hacky implementation... */
 if (libc_gettimeofday64(&now,NULL)) return -1;
 result = (now.tv_usec*(CLOCKS_PER_SEC/USEC_PER_SEC)+
           now.tv_sec*CLOCKS_PER_SEC);
 if (clock_start < 0) clock_start = result;
 return result-clock_start;
}
INTERN clock_t LIBCCALL
libc_times(struct tms *__restrict buffer) {
 /* This isn't right either... */
 clock_t clock_now  = libc_clock();
 buffer->tms_stime  = clock_now/2;
 buffer->tms_utime  = clock_now/2;
 buffer->tms_cutime = 0;
 buffer->tms_cstime = 0;
 return clock_now;
}

INTERN clock_t LIBCCALL libc_dos_clock(void) {
 return libc_clock()/(CLOCKS_PER_SEC/__DOS_CLOCKS_PER_SEC);
}
INTERN clock_t LIBCCALL
libc_dos_times(struct tms *__restrict buffer) {
 /* This isn't right either... */
 clock_t clock_now = libc_dos_clock();
 buffer->tms_stime = clock_now/2;
 buffer->tms_utime = clock_now/2;
 buffer->tms_cutime = 0;
 buffer->tms_cstime = 0;
 return clock_now;
}


PRIVATE __DEFINE_MONTH_STARTING_DAY_OF_YEAR(u16);

LOCAL int LIBCCALL tm_calc_isdst(struct tm const *self) {
 /* found here: "http://stackoverflow.com/questions/5590429/calculating-daylight-savings-time-from-only-date" */
 int previousSunday;
 //January, February, and December are out.
 if (self->tm_mon < 3 || self->tm_mon > 11) { return 0; }
 //April to October are in
 if (self->tm_mon > 3 && self->tm_mon < 11) { return 1; }
 previousSunday = self->tm_mday-self->tm_wday;
 //In march, we are DST if our previous Sunday was on or after the 8th.
 if (self->tm_mon == 3) { return previousSunday >= 8; }
 //In November we must be before the first Sunday to be dst.
 //That means the previous Sunday must be before the 1st.
 return previousSunday <= 0;
}

INTERN struct tm *LIBCCALL
libc_gmtime64_r(time64_t const *__restrict timer,
                struct tm *__restrict tp) {
 time64_t t; int i; u16 const *monthvec;
 t = *timer;
 tp->tm_sec  = (int)(t % 60);
 tp->tm_min  = (int)((t/60) % 60);
 tp->tm_hour = (int)((t/(60*60)) % 24);
 t /= SECONDS_PER_DAY;
 t += __yearstodays(LINUX_TIME_START_YEAR);
 tp->tm_wday = (int)(t % 7);
 tp->tm_year = (int)__daystoyears(t);
 t -= __yearstodays(tp->tm_year);
 tp->tm_yday = (int)t;
 monthvec = __time_monthstart_yday[__isleap(tp->tm_year)];
 for (i = 1; i < 12; ++i) if (monthvec[i] >= t) break;
 tp->tm_mon  = i-1;
 t -= monthvec[i-1];
 tp->tm_mday = t;
 tp->tm_isdst = tm_calc_isdst(tp);
 tp->tm_year -= 1900;
 return tp;
}
INTERN struct tm *LIBCCALL
libc_localtime64_r(time64_t const *__restrict timer,
                   struct tm *__restrict tp) {
 /* TODO: Timezones 'n $hit. */
 return libc_gmtime64_r(timer,tp);
}
INTERN struct tm *LIBCCALL
libc_gmtime_r(time32_t const *__restrict timer,
              struct tm *__restrict tp) {
 time64_t t64 = *timer;
 return libc_gmtime64_r(&t64,tp);
}
INTERN struct tm *LIBCCALL
libc_localtime_r(time_t const *__restrict timer,
                 struct tm *__restrict tp) {
 time64_t t64 = *timer;
 return libc_localtime64_r(&t64,tp);
}

INTERN derrno_t LIBCCALL
libc_gmtime_s(struct tm *__restrict tp,
              time32_t const *__restrict timer) {
 return libc_gmtime_r(timer,tp) ? 0 : libc_dos_geterrno();
}
INTERN derrno_t LIBCCALL
libc_gmtime64_s(struct tm *__restrict tp,
                time64_t const *__restrict timer) {
 return libc_gmtime64_r(timer,tp) ? 0 : libc_dos_geterrno();
}
INTERN derrno_t LIBCCALL
libc_localtime_s(struct tm *__restrict tp,
                 time32_t const *__restrict timer) {
 return libc_localtime_r(timer,tp) ? 0 : libc_dos_geterrno();
}
INTERN derrno_t LIBCCALL
libc_localtime64_s(struct tm *__restrict tp,
                   time64_t const *__restrict timer) {
 return libc_localtime64_r(timer,tp) ? 0 : libc_dos_geterrno();
}


PRIVATE ATTR_RAREBSS struct tm gmtime_buf,localtime_buf;
INTERN struct tm *LIBCCALL libc_gmtime(time32_t const *timer) { return libc_gmtime_r(timer,&gmtime_buf); }
INTERN struct tm *LIBCCALL libc_gmtime64(time64_t const *timer) { return libc_gmtime64_r(timer,&gmtime_buf); }
INTERN struct tm *LIBCCALL libc_localtime(time32_t const *timer) { return libc_localtime_r(timer,&localtime_buf); }
INTERN struct tm *LIBCCALL libc_localtime64(time64_t const *timer) { return libc_localtime64_r(timer,&localtime_buf); }
INTERN double LIBCCALL libc_difftime(time_t time1, time_t time0) { return time1 > time0 ? time1-time0 : time0-time1; }
INTERN double LIBCCALL libc_difftime64(time64_t time1, time64_t time0) { return time1 > time0 ? time1-time0 : time0-time1; }

INTERN time64_t LIBCCALL
libc_mktime64(struct tm const *__restrict tp) {
 time64_t result;
 result = __yearstodays(tp->tm_year) -
          __yearstodays(LINUX_TIME_START_YEAR);
 result += tp->tm_yday;
 result *= SECONDS_PER_DAY;
 result += tp->tm_hour*60*60;
 result += tp->tm_min*60;
 result += tp->tm_sec;
 return result;
}
INTERN time64_t LIBCCALL
libc_mkgmtime64(struct tm const *__restrict tp) {
 time64_t result;
 /* XXX: Timezones? */
 result = __yearstodays(tp->tm_year) -
          __yearstodays(LINUX_TIME_START_YEAR);
 result += tp->tm_yday;
 result *= SECONDS_PER_DAY;
 result += tp->tm_hour*60*60;
 result += tp->tm_min*60;
 result += tp->tm_sec;
 return result;
}

#ifdef LIBC_LIBCCALL_RET64_IS_RET32
DEFINE_INTERN_ALIAS(libc_mktime,libc_mktime64);
#else
INTERN time32_t LIBCCALL
libc_mktime(struct tm const *__restrict tp) {
 return (time32_t)libc_mktime64(tp);
}
#endif

#ifdef LIBC_LIBCCALL_RET64_IS_RET32
DEFINE_INTERN_ALIAS(libc_mkgmtime,libc_mkgmtime64);
#else
INTERN time32_t LIBCCALL
libc_mkgmtime(struct tm const *__restrict tp) {
 return (time32_t)libc_mkgmtime64(tp);
}
#endif

INTERN int LIBCCALL
libc_settimeofday64(struct timeval64 const *tv,
                    struct timezone const *tz) {
 return FORWARD_SYSTEM_ERROR(sys_settimeofday(tv,tz));
}

EXPORT(timegm,libc_timegm);
DEFINE_INTERN_ALIAS(libc_timegm,libc_mkgmtime);

EXPORT(timegm64,libc_timegm64);
DEFINE_INTERN_ALIAS(libc_timegm64,libc_mkgmtime64);

EXPORT(timelocal,libc_timelocal);
DEFINE_INTERN_ALIAS(libc_timelocal,libc_mktime);

EXPORT(timelocal64,libc_timelocal64);
DEFINE_INTERN_ALIAS(libc_timelocal64,libc_mktime64);


INTERN int LIBCCALL
libc_settimeofday(struct timeval32 const *tv,
                  struct timezone const *tz) {
 struct timeval64 tv64;
 LIBC_TRY {
  tv64.tv_sec  = tv->tv_sec;
  tv64.tv_usec = tv->tv_usec;
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return libc_settimeofday64(&tv64,tz);
}
INTERN int LIBCCALL
libc_gettimeofday(struct timeval32 *__restrict tv,
                  struct timezone *__restrict tz) {
 int result; struct timeval64 tv64;
 result = libc_gettimeofday64(&tv64,tz);
 if (!result) {
  LIBC_TRY {
   tv->tv_sec  = (time32_t)tv64.tv_sec;
   tv->tv_usec = tv64.tv_usec;
 } LIBC_EXCEPT(libc_except_errno()) {
   result = -1;
  }
 }
 return result;
}
INTERN int LIBCCALL
libc_nanosleep(struct timespec32 const *requested_time,
               struct timespec32 *remaining) {
 struct timespec64 areq,arem; int result;
 LIBC_TRY {
  areq.tv_sec  = requested_time->tv_sec;
  areq.tv_nsec = requested_time->tv_nsec;
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 result = libc_nanosleep64(&areq,remaining ? &arem : NULL);
 if (!result && remaining) {
  LIBC_TRY {
   remaining->tv_sec  = (time32_t)arem.tv_sec;
   remaining->tv_nsec = arem.tv_nsec;
  } LIBC_EXCEPT(libc_except_errno()) {
   result = -1;
  }
 }
 return result;
}

INTERN unsigned int LIBCCALL libc_sleep(unsigned int seconds) {
 struct timespec64 req,rem;
 req.tv_sec  = seconds;
 req.tv_nsec = 0;
 libc_nanosleep64(&req,&rem);
 return rem.tv_sec;
}
INTERN int LIBCCALL libc_usleep(useconds_t useconds) {
 struct timespec64 req,rem; int error;
 req.tv_sec  =  useconds/USEC_PER_SEC;
 req.tv_nsec = (useconds%USEC_PER_SEC)*NSEC_PER_USEC;
 while ((error = libc_nanosleep64(&req,&rem)) != 0 &&
         libc_geterrno() == EINTR) req = rem;
 return error;
}
INTERN unsigned int LIBCCALL
libc_alarm(unsigned int seconds) {
 libc_seterrno(ENOSYS);
 return seconds;
}
INTERN useconds_t LIBCCALL
libc_ualarm(useconds_t value, useconds_t interval) {
 libc_seterrno(ENOSYS);
 return -1;
}
INTERN void LIBCCALL _libc_crtSleep(u32 milliseconds) {
 libc_usleep(milliseconds * USEC_PER_MSEC);
}


INTERN time64_t LIBCCALL libc_time64(time64_t *timer) {
 struct timeval64 t64;
 if (libc_gettimeofday64(&t64,NULL))
     return (time64_t)-1;
 if (timer) {
  LIBC_TRY {
   *timer = t64.tv_sec;
  } LIBC_EXCEPT(libc_except_errno()) {
   return (time64_t)-1;
  }
 }
 return t64.tv_sec;
}
INTERN time32_t LIBCCALL libc_time(time32_t *timer) {
 time32_t t32 = (time32_t)libc_time64(NULL);
 if (timer) *timer = t32;
 return t32;
}
INTERN int LIBCCALL libc_stime64(time64_t const *when) {
 struct timeval64 COMPILER_IGNORE_UNINITIALIZED(now);
 LIBC_TRY {
  now.tv_sec = *when;
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 now.tv_usec = 0;
 return libc_settimeofday64(&now,NULL);
}
INTERN int LIBCCALL libc_stime(time32_t const *when) {
 time64_t COMPILER_IGNORE_UNINITIALIZED(w64);
 LIBC_TRY {
  w64 = *when;
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return libc_stime64(&w64);
}
INTERN u32 LIBCCALL
libc_getsystime(struct tm *__restrict tp) {
 struct timeval64 now;
 if (libc_gettimeofday64(&now,NULL))
     libc_memset(&now,0,sizeof(struct timeval64));
 libc_gmtime64_r(&now.tv_sec,tp);
 /* NOTE: Returns Milliseconds. */
 return now.tv_usec/USEC_PER_MSEC;
}
INTERN u32 LIBCCALL
libc_setsystime(struct tm const *__restrict tp, u32 msec) {
 struct timeval64 now;
 now.tv_sec = libc_mktime64(tp);
 now.tv_usec = msec*USEC_PER_MSEC;
 /* Returns an NT error code, or ZERO on success. */
 return libc_settimeofday64(&now,NULL) ? libc_nt_geterrno() : 0;
}
INTERN int LIBCCALL libc_ftime64(struct timeb64 *timebuf) {
 struct timeval64 tv; struct timezone tz;
 int result = libc_gettimeofday64(&tv,&tz);
 if (!result) {
  LIBC_TRY {
   timebuf->time     = tv.tv_sec;
   timebuf->millitm  = tv.tv_usec / USEC_PER_MSEC;
   timebuf->timezone = tz.tz_minuteswest;
   timebuf->dstflag  = tz.tz_dsttime;
  } LIBC_EXCEPT(libc_except_errno()) {
   return -1;
  }
 }
 return result;
}
INTERN int LIBCCALL libc_ftime(struct timeb32 *timebuf) {
 struct timeval64 tv; struct timezone tz;
 int result = libc_gettimeofday64(&tv,&tz);
 if (!result) {
  LIBC_TRY {
   timebuf->time     = (time32_t)tv.tv_sec;
   timebuf->millitm  = tv.tv_usec / USEC_PER_MSEC;
   timebuf->timezone = tz.tz_minuteswest;
   timebuf->dstflag  = tz.tz_dsttime;
  } LIBC_EXCEPT(libc_except_errno()) {
   return -1;
  }
 }
 return result;
}
INTERN derrno_t LIBCCALL
libc_ftime_s(struct timeb *timebuf) {
 return libc_ftime(timebuf) ? libc_dos_geterrno() : 0;
}
INTERN derrno_t LIBCCALL
libc_ftime64_s(struct timeb64 *timebuf) {
 return libc_ftime64(timebuf) ? libc_dos_geterrno() : 0;
}

EXPORT(timespec_get64,libc_timespec_get64);
INTERN int LIBCCALL
libc_timespec_get64(struct timespec64 *ts, int base) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(timespec_get,libc_timespec_get);
INTERN int LIBCCALL
libc_timespec_get(struct timespec32 *ts, int base) {
 struct timespec64 t64;
 int error;
 if ((error = libc_timespec_get64(&t64,base)) != 0)
      return error;
 LIBC_TRY {
  ts->tv_sec  = t64.tv_sec;
  ts->tv_nsec = t64.tv_nsec;
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return 0;
}

EXPORT(timer_create,libc_timer_create);
CRT_TIMER int LIBCCALL
libc_timer_create(clockid_t clock_id,
                  struct sigevent *__restrict evp,
                  timer_t *__restrict timerid) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(timer_delete,libc_timer_delete);
CRT_TIMER int LIBCCALL
libc_timer_delete(timer_t timerid) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(timer_getoverrun,libc_timer_getoverrun);
CRT_TIMER int LIBCCALL
libc_timer_getoverrun(timer_t timerid) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(timer_gettime64,libc_timer_gettime64);
CRT_TIMER int LIBCCALL
libc_timer_gettime64(timer_t timerid,
                     struct itimerspec64 *value) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(timer_settime64,libc_timer_settime64);
CRT_TIMER int LIBCCALL
libc_timer_settime64(timer_t timerid, int flags,
                     struct itimerspec64 const *__restrict value,
                     struct itimerspec64 *__restrict ovalue) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(timer_gettime,libc_timer_gettime);
CRT_TIMER int LIBCCALL
libc_timer_gettime(timer_t timerid,
                   struct itimerspec32 *value) {
 struct itimerspec64 t64;
 int error;
 if ((error = libc_timer_gettime64(timerid,&t64)) != 0)
      return error;
 LIBC_TRY {
  value->it_interval.tv_sec  = (time32_t)t64.it_interval.tv_sec;
  value->it_interval.tv_nsec = t64.it_interval.tv_nsec;
  value->it_value.tv_sec     = (time32_t)t64.it_value.tv_sec;
  value->it_value.tv_nsec    = t64.it_value.tv_nsec;
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return 0;
}

EXPORT(timer_settime,libc_timer_settime);
CRT_TIMER int LIBCCALL
libc_timer_settime(timer_t timerid, int flags,
                   struct itimerspec32 const *__restrict value,
                   struct itimerspec32 *ovalue) {
 struct itimerspec64 v64,ov64; int error;
 LIBC_TRY {
  v64.it_interval.tv_sec  = value->it_interval.tv_sec;
  v64.it_interval.tv_nsec = value->it_interval.tv_nsec;
  v64.it_value.tv_sec     = value->it_value.tv_sec;
  v64.it_value.tv_nsec    = value->it_value.tv_nsec;
  if ((error = libc_timer_settime64(timerid,flags,&v64,ovalue ? &ov64 : NULL)) != 0)
       return error;
  if (ovalue) {
   ovalue->it_interval.tv_sec  = (time32_t)ov64.it_interval.tv_sec;
   ovalue->it_interval.tv_nsec = ov64.it_interval.tv_nsec;
   ovalue->it_value.tv_sec     = (time32_t)ov64.it_value.tv_sec;
   ovalue->it_value.tv_nsec    = ov64.it_value.tv_nsec;
  }
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return 0;
}

EXPORT(getitimer64,libc_getitimer64);
CRT_ITIMER int LIBCCALL
libc_getitimer64(int which, struct itimerval64 *value) {
 libc_seterrno(ENOSYS);
 return -1;
}
EXPORT(setitimer64,libc_setitimer64);
CRT_ITIMER int LIBCCALL
libc_setitimer64(int which,
                 struct itimerval64 const *__restrict new_,
                 struct itimerval64 *__restrict old) {
 libc_seterrno(ENOSYS);
 return -1;
}
EXPORT(getitimer,libc_getitimer);
CRT_ITIMER int LIBCCALL
libc_getitimer(int which, struct itimerval32 *value) {
 struct itimerval64 t64;
 int error;
 if ((error = libc_getitimer64(which,&t64)) != 0)
      return error;
 LIBC_TRY {
  value->it_interval.tv_sec  = (time32_t)t64.it_interval.tv_sec;
  value->it_interval.tv_usec = t64.it_interval.tv_usec;
  value->it_value.tv_sec     = (time32_t)t64.it_value.tv_sec;
  value->it_value.tv_usec    = t64.it_value.tv_usec;
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return 0;
}
EXPORT(setitimer,libc_setitimer);
CRT_ITIMER int LIBCCALL
libc_setitimer(int which,
               struct itimerval32 const *__restrict value,
               struct itimerval32 *__restrict ovalue) {
 struct itimerval64 v64,ov64; int error;
 LIBC_TRY {
  v64.it_interval.tv_sec  = value->it_interval.tv_sec;
  v64.it_interval.tv_usec = value->it_interval.tv_usec;
  v64.it_value.tv_sec     = value->it_value.tv_sec;
  v64.it_value.tv_usec    = value->it_value.tv_usec;
  if ((error = libc_setitimer64(which,&v64,ovalue ? &ov64 : NULL)) != 0)
       return error;
  if (ovalue) {
   ovalue->it_interval.tv_sec  = (time32_t)ov64.it_interval.tv_sec;
   ovalue->it_interval.tv_usec = ov64.it_interval.tv_usec;
   ovalue->it_value.tv_sec     = (time32_t)ov64.it_value.tv_sec;
   ovalue->it_value.tv_usec    = ov64.it_value.tv_usec;
  }
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return 0;
}



EXPORT(adjtime64,libc_adjtime64);
CRT_TIME int LIBCCALL
libc_adjtime64(struct timeval64 const *delta,
               struct timeval64 *olddelta) {
 libc_seterrno(ENOSYS);
 return -1;
}
EXPORT(adjtime,libc_adjtime);
CRT_TIME int LIBCCALL
libc_adjtime(struct timeval32 const *delta,
             struct timeval32 *olddelta) {
 struct timeval64 v64,ov64; int error;
 LIBC_TRY {
  if (delta) {
   v64.tv_sec  = delta->tv_sec;
   v64.tv_usec = delta->tv_usec;
  }
  if ((error = libc_adjtime64(delta ? &v64 : NULL,olddelta ? &ov64 : NULL)) != 0)
       return error;
  if (olddelta) {
   olddelta->tv_sec  = (time32_t)ov64.tv_sec;
   olddelta->tv_usec = ov64.tv_usec;
  }
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return 0;
}



EXPORT(clock_getcpuclockid,libc_clock_getcpuclockid);
CRT_CLOCK int LIBCCALL
libc_clock_getcpuclockid(pid_t pid, clockid_t *clock_id) {
 libc_seterrno(ENOSYS);
 return -1;
}
EXPORT(clock_getres64,libc_clock_getres64);
CRT_CLOCK int LIBCCALL
libc_clock_getres64(clockid_t clock_id,
                    struct timespec64 *res) {
 libc_seterrno(ENOSYS);
 return -1;
}
EXPORT(clock_gettime64,libc_clock_gettime64);
CRT_CLOCK int LIBCCALL
libc_clock_gettime64(clockid_t clock_id,
                     struct timespec64 *tp) {
 libc_seterrno(ENOSYS);
 return -1;
}
EXPORT(clock_settime64,libc_clock_settime64);
CRT_CLOCK int LIBCCALL
libc_clock_settime64(clockid_t clock_id,
                     struct timespec64 const *tp) {
 libc_seterrno(ENOSYS);
 return -1;
}
EXPORT(clock_nanosleep64,libc_clock_nanosleep64);
CRT_CLOCK int LIBCCALL
libc_clock_nanosleep64(clockid_t clock_id, int flags,
                       struct timespec64 const *requested_time,
                       struct timespec64 *remaining) {
 /* XXX: Not actually implemented! */
 return libc_nanosleep64(requested_time,remaining);
}
EXPORT(clock_getres,libc_clock_getres);
CRT_CLOCK int LIBCCALL
libc_clock_getres(clockid_t clock_id,
                  struct timespec32 *res) {
 struct timespec64 r64;
 int error;
 if ((error = libc_clock_getres64(clock_id,&r64)) != 0)
      return error;
 res->tv_sec  = (time32_t)r64.tv_sec;
 res->tv_nsec = r64.tv_nsec;
 return 0;
}
EXPORT(clock_gettime,libc_clock_gettime);
CRT_CLOCK int LIBCCALL
libc_clock_gettime(clockid_t clock_id,
                   struct timespec32 *tp) {
 struct timespec64 r64;
 int error;
 if ((error = libc_clock_gettime64(clock_id,&r64)) != 0)
      return error;
 tp->tv_sec  = (time32_t)r64.tv_sec;
 tp->tv_nsec = r64.tv_nsec;
 return 0;
}
EXPORT(clock_settime,libc_clock_settime);
CRT_CLOCK int LIBCCALL
libc_clock_settime(clockid_t clock_id,
                   struct timespec32 const *tp) {
 struct timespec64 r64;
 r64.tv_sec  = tp->tv_sec;
 r64.tv_nsec = tp->tv_nsec;
 return libc_clock_settime64(clock_id,&r64);
}
EXPORT(clock_nanosleep,libc_clock_nanosleep);
CRT_CLOCK int LIBCCALL
libc_clock_nanosleep(clockid_t clock_id, int flags,
                     struct timespec32 const *requested_time,
                     struct timespec32 *remaining) {
 struct timespec64 areq,arem;
 int COMPILER_IGNORE_UNINITIALIZED(result);
 LIBC_TRY {
  areq.tv_sec  = requested_time->tv_sec;
  areq.tv_nsec = requested_time->tv_nsec;
  result = libc_clock_nanosleep64(clock_id,flags,&areq,remaining ? &arem : NULL);
  if (!result && remaining) {
   remaining->tv_sec  = (time32_t)arem.tv_sec;
   remaining->tv_nsec = arem.tv_nsec;
  }
 } LIBC_EXCEPT(libc_except_errno()) {
  result = -1;
 }
 return result;
}



PUBLIC char *tzname[2] = { NULL, NULL };
PUBLIC int daylight = 0;
PUBLIC long int timezone = 0; /* Seconds West of UTC (Universal time); aka.: gmtime()+timezone == localtime(). */

INTERN void LIBCCALL libc_tzset(void) {
 /* TODO */
 tzname[0] = "foo";
 tzname[1] = "bar";
 daylight = 1;
 timezone = 42;
}
INTERN derrno_t LIBCCALL
libc_get_tzname(size_t *pres, char *buf, size_t buflen, int idx) {
 size_t reslen;
 /* My god... - Nobody asked for it, but here it is:
  * The most useless function and probably funniest way to ruin portability.
  * It mean really? DOS even allows you direct access to the 'tzname' vector,
  *                 but somebody still decided that this kind of encapsulation
  *                 would be the future?
  *                 Doing something like this is some kernel-level $h1t,
  *                 but not anything you'd ever want to put in user-space,
  *                 because all it does here is cause redundancy, as well
  *                 as degradation of overall performance.
  * But I guess I'm disproving at least one part of that statement:
  *  - Your application can still be ported to KOS,
  *    since this is where KOS implements that function...
  */
 if ((unsigned int)idx >= 2) return __DOS_EINVAL;
 reslen = libc_strlen(tzname[idx])+1;
 if (pres) *pres = reslen;
 if (reslen > buflen) return __DOS_ERANGE;
 libc_memcpy(buf,tzname[idx],reslen*sizeof(char));
 return 0;
}
INTERN s32 *LIBCCALL libc_p_timezone(void) { return (s32 *)&timezone; }
INTERN s32 *LIBCCALL libc_daylight(void) { return (s32 *)&daylight; }
DEFINE_INTERN_ALIAS(libc_timezone,libc_p_timezone);
INTERN derrno_t LIBCCALL libc_get_daylight(s32 *pres) { return pres ? (*pres = (s32)daylight,0) : __DOS_EINVAL; }
INTERN derrno_t LIBCCALL libc_get_timezone(s32 *pres) { return pres ? (*pres = (s32)timezone,0) : __DOS_EINVAL; }
INTERN char **LIBCCALL libc_tzname(void) { return tzname; }




EXPORT(__KSYM(clock),              libc_clock);
EXPORT(__DSYM(clock),              libc_dos_clock);
EXPORT(__KSYM(times),              libc_times);
EXPORT(__DSYM(times),              libc_dos_times);
EXPORT(__KSYM(tzset),              libc_tzset);
EXPORT(__DSYM(_tzset),             libc_tzset);
EXPORT(gmtime,                     libc_gmtime);
EXPORT(gmtime64,                   libc_gmtime64);
EXPORT(localtime,                  libc_localtime);
EXPORT(localtime64,                libc_localtime64);
EXPORT(gmtime_r,                   libc_gmtime_r);
EXPORT(gmtime64_r,                 libc_gmtime64_r);
EXPORT(localtime_r,                libc_localtime_r);
EXPORT(localtime64_r,              libc_localtime64_r);
EXPORT(_gmtime32,                  libc_gmtime);
EXPORT(_gmtime64,                  libc_gmtime64);
EXPORT(_gmtime32_s,                libc_gmtime_s);
EXPORT(_gmtime64_s,                libc_gmtime64_s);
EXPORT(_localtime32,               libc_localtime);
EXPORT(_localtime64,               libc_localtime64);
EXPORT(_localtime32_s,             libc_localtime_s);
EXPORT(_localtime64_s,             libc_localtime64_s);
EXPORT(difftime,                   libc_difftime);
EXPORT(difftime64,                 libc_difftime64);
EXPORT(_difftime32,                libc_difftime);
EXPORT(_difftime64,                libc_difftime64);
EXPORT(mktime,                     libc_mktime);
EXPORT(mktime64,                   libc_mktime64);
EXPORT(_mktime32,                  libc_mktime);
EXPORT(_mktime64,                  libc_mktime64);
EXPORT(mkgmtime,                   libc_mkgmtime);
EXPORT(mkgmtime64,                 libc_mkgmtime64);
EXPORT(_mkgmtime32,                libc_mkgmtime);
EXPORT(_mkgmtime64,                libc_mkgmtime64);
EXPORT(sigtimedwait,               libc_sigtimedwait);
EXPORT(sigtimedwait64,             libc_sigtimedwait64);
EXPORT(settimeofday,               libc_settimeofday);
EXPORT(settimeofday64,             libc_settimeofday64);
EXPORT(gettimeofday,               libc_gettimeofday);
EXPORT(nanosleep,                  libc_nanosleep);
EXPORT(time,                       libc_time);
EXPORT(time64,                     libc_time64);
EXPORT(_time32,                    libc_time);
EXPORT(_time64,                    libc_time64);
EXPORT(__KSYM(sleep),              libc_sleep);
EXPORT(__DSYM(_sleep),             libc_sleep);
EXPORT(stime,                      libc_stime);
EXPORT(stime64,                    libc_stime64);
EXPORT(_getsystime,                libc_getsystime);
EXPORT(_setsystime,                libc_setsystime);
EXPORT(usleep,                     libc_usleep);
EXPORT(alarm,                      libc_alarm);
EXPORT(ualarm,                     libc_ualarm);
EXPORT(__crtSleep,                _libc_crtSleep);
EXPORT(ftime,                      libc_ftime);
EXPORT(ftime64,                    libc_ftime64);
EXPORT(_ftime,                     libc_ftime); /* This is not an error. - DOS defines this name, too. */
EXPORT(_ftime32,                   libc_ftime);
EXPORT(_ftime64,                   libc_ftime64);
EXPORT(_ftime32_s,                 libc_ftime_s);
EXPORT(_ftime64_s,                 libc_ftime64_s);



/* DOS-specific functions and aliases. */
EXPORT(__daylight,                 libc_daylight);
EXPORT(__timezone,                 libc_timezone);
EXPORT(__tzname,                   libc_tzname);
EXPORT(__p__timezone,              libc_p_timezone);
EXPORT(_get_daylight,              libc_get_daylight);
EXPORT(_get_timezone,              libc_get_timezone);
EXPORT(_get_tzname,                libc_get_tzname);
EXPORT(_tzname,                    tzname);
EXPORT(_timezone,                  timezone);

EXPORT(dysize,libc_dysize);
CRT_TIME int LIBCCALL libc_dysize(int year) {
 return __isleap(year) ? 366 : 365;
}

/* GLibc aliases */
EXPORT_STRONG(__nanosleep,libc_nanosleep);
EXPORT_STRONG(__clock_getcpuclockid,libc_clock_getcpuclockid);
EXPORT_STRONG(__clock_getres,libc_clock_getres);
EXPORT_STRONG(__clock_gettime,libc_clock_gettime);
EXPORT_STRONG(__clock_settime,libc_clock_settime);
EXPORT_STRONG(__clock_nanosleep,libc_clock_nanosleep);


DECL_END

#endif /* !GUARD_LIBS_LIBC_TIME_C */
