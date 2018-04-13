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
#ifndef GUARD_LIBS_LIBC_FORMAT_TIME_C
#define GUARD_LIBS_LIBC_FORMAT_TIME_C 1
#define _UTF_SOURCE 1

#include "libc.h"
#include "format.h"
#include "errno.h"
#include "time.h"
#include "format-time.h"

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <format-printer.h>
#include <errno.h>
#include <bits/dos-errno.h>
#include <limits.h>

#ifndef __INTELLISENSE__
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR
#define FORMAT_OPTION_LOCALE     1
#include "format-time-impl.c.inl" /* libc_format_strftime_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#define FORMAT_OPTION_LOCALE     1
#include "format-time-impl.c.inl" /* libc_format_w16ftime_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR32
#define FORMAT_OPTION_LOCALE     1
#include "format-time-impl.c.inl" /* libc_format_w32ftime_l */
#endif

DECL_BEGIN

EXPORT(__SYMw16(format_wcsftime_l),libc_format_w16ftime_l);
EXPORT(__SYMw32(format_wcsftime_l),libc_format_w32ftime_l);


PUBLIC int getdate_err = 0;
EXPORT(strptime_l,libc_strptime_l);
INTERN char *LIBCCALL
libc_strptime_l(char const *__restrict s,
                char const *__restrict format,
                struct tm *tp, locale_t loc) {
 libc_seterrno(ENOSYS);
 return NULL;
}

EXPORT(strptime,libc_strptime);
INTERN char *LIBCCALL
libc_strptime(char const *__restrict s,
              char const *__restrict format,
              struct tm *tp) {
 return libc_strptime_l(s,format,tp,NULL);
}


EXPORT(getdate_r,libc_getdate_r);
INTERN int LIBCCALL
libc_getdate_r(char const *__restrict string,
               struct tm *__restrict resbufp) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(getdate,libc_getdate);
PRIVATE ATTR_RAREBSS struct tm getdate_buf;
INTERN struct tm *LIBCCALL
libc_getdate(char const *string) {
 if (libc_getdate_r(string,&getdate_buf))
     return NULL;
 return &getdate_buf;
}


#define ASCTIME_BUFSIZE 26

CRT_RARE_RODATA char const libc_3_questionmarks[] = {'?','?','?',0};
CRT_RARE_RODATA char const libc_asctime_format[] = {
 '%','.','3','s',' ','%','.','3','s','%','3','d',' ','%','.','2',
 'd',':','%','.','2','d',':','%','.','2','d',' ','%','d','\n',0
};
CRT_DOS_RODATA char16_t const libc_asctime_format16[] = {
 '%','.','3','s',' ','%','.','3','s','%','3','d',' ','%','.','2',
 'd',':','%','.','2','d',':','%','.','2','d',' ','%','d','\n',0
};
CRT_WIDECHAR_RODATA char32_t const libc_asctime_format32[] = {
 '%','.','3','s',' ','%','.','3','s','%','3','d',' ','%','.','2',
 'd',':','%','.','2','d',':','%','.','2','d',' ','%','d','\n',0
};


EXPORT(asctime_r,libc_asctime_r);
CRT_TIME char *LIBCCALL
libc_asctime_r(struct tm const *__restrict tp, char *__restrict buf) {
 if unlikely(!tp) { libc_seterrno(EINVAL); return NULL; }
 if unlikely(tp->tm_year > INT_MAX-1900) { libc_seterrno(EOVERFLOW); return NULL; }
 libc_sprintf(buf,libc_asctime_format,
            ((unsigned int)tp->tm_wday >= 7 ? libc_3_questionmarks : libc_abbr_wday_names[tp->tm_wday]),
            ((unsigned int)tp->tm_mon >= 12 ? libc_3_questionmarks : libc_abbr_month_names[tp->tm_mon]),
              tp->tm_mday,tp->tm_hour,tp->tm_min,tp->tm_sec,tp->tm_year+1900);
 return buf;
}
EXPORT(__SYMw16(wasctime_r),libc_w16asctime_r);
CRT_DOS_EXT char16_t *LIBCCALL
libc_w16asctime_r(struct tm const *__restrict tp, char16_t *__restrict buf) {
 if unlikely(!tp) { libc_seterrno(EINVAL); return NULL; }
 if unlikely(tp->tm_year > INT_MAX-1900) { libc_seterrno(EOVERFLOW); return NULL; }
 libc_sw16printf(buf,libc_asctime_format16,
               ((unsigned int)tp->tm_wday >= 7 ? libc_3_questionmarks : libc_abbr_wday_names[tp->tm_wday]),
               ((unsigned int)tp->tm_mon >= 12 ? libc_3_questionmarks : libc_abbr_month_names[tp->tm_mon]),
                 tp->tm_mday,tp->tm_hour,tp->tm_min,tp->tm_sec,tp->tm_year+1900);
 return buf;
}
EXPORT(__SYMw32(wasctime_r),libc_w32asctime_r);
CRT_WIDECHAR char32_t *LIBCCALL
libc_w32asctime_r(struct tm const *__restrict tp, char32_t *__restrict buf) {
 if unlikely(!tp) { libc_seterrno(EINVAL); return NULL; }
 if unlikely(tp->tm_year > INT_MAX-1900) { libc_seterrno(EOVERFLOW); return NULL; }
 libc_sw32printf(buf,libc_asctime_format32,
               ((unsigned int)tp->tm_wday >= 7 ? libc_3_questionmarks : libc_abbr_wday_names[tp->tm_wday]),
               ((unsigned int)tp->tm_mon >= 12 ? libc_3_questionmarks : libc_abbr_month_names[tp->tm_mon]),
                 tp->tm_mday,tp->tm_hour,tp->tm_min,tp->tm_sec,tp->tm_year+1900);
 return buf;
}



EXPORT(__KSYM(asctime_s),libc_asctime_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_asctime_s(char buf[26], size_t bufsize, struct tm const *__restrict tp) {
 return bufsize < 26 ? EINVAL : (libc_asctime_r(tp,buf) ? 0 : libc_geterrno());
}
EXPORT(__DSYM(asctime_s),libc_dos_asctime_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_asctime_s(char buf[26], size_t bufsize,
                   struct tm const *__restrict tp) {
 return libc_errno_kos2dos(libc_asctime_s(buf,bufsize,tp));
}

EXPORT(__KSYMw16(wasctime_s),libc_w16asctime_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w16asctime_s(char16_t buf[26], size_t bufsize,
                  struct tm const *__restrict tp) {
 return bufsize < 26 ? EINVAL : (libc_w16asctime_r(tp,buf) ? 0 : libc_geterrno());
}
EXPORT(__DSYMw16(_wasctime_s),libc_dos_w16asctime_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16asctime_s(char16_t buf[26], size_t bufsize,
                      struct tm const *__restrict tp) {
 return libc_errno_kos2dos(libc_w16asctime_s(buf,bufsize,tp));
}

EXPORT(__KSYMw32(wasctime_s),libc_w32asctime_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w32asctime_s(char32_t buf[26], size_t bufsize,
                  struct tm const *__restrict tp) {
 return bufsize < 26 ? EINVAL : (libc_w32asctime_r(tp,buf) ? 0 : libc_geterrno());
}
EXPORT(__DSYMw32(wasctime_s),libc_dos_w32asctime_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w32asctime_s(char32_t buf[26], size_t bufsize,
                      struct tm const *__restrict tp) {
 return libc_errno_kos2dos(libc_w32asctime_s(buf,bufsize,tp));
}


CRT_RARE_BSS char libc_asctime_buf[ASCTIME_BUFSIZE*4];
CRT_RARE_BSS char libc_ctime_buf[ASCTIME_BUFSIZE*4]; 

EXPORT(asctime,libc_asctime);
CRT_TIME char *LIBCCALL libc_asctime(struct tm const *tp) { return libc_asctime_r(tp,libc_asctime_buf); }
EXPORT(__SYMw16(_wasctime),libc_w16asctime);
CRT_DOS char16_t *LIBCCALL libc_w16asctime(struct tm const *tp) { return libc_w16asctime_r(tp,(char16_t *)libc_asctime_buf); }
EXPORT(__SYMw32(wasctime),libc_w32asctime);
CRT_DOS_EXT char32_t *LIBCCALL libc_w32asctime(struct tm const *tp) { return libc_w32asctime_r(tp,(char32_t *)libc_asctime_buf); }


EXPORT(ctime_r,libc_ctime_r);
CRT_TIME char *LIBCCALL
libc_ctime_r(time_t const *__restrict timer,
             char *__restrict buf) {
 struct tm ltm;
 return libc_asctime_r(libc_localtime_r(timer,&ltm),buf);
}
EXPORT(ctime64_r,libc_ctime64_r);
CRT_TIME char *LIBCCALL
libc_ctime64_r(time64_t const *__restrict timer,
               char *__restrict buf) {
 struct tm ltm;
 return libc_asctime_r(libc_localtime64_r(timer,&ltm),buf);
}
EXPORT(__SYMw16(wctime_r),libc_w16ctime_r);
CRT_DOS_EXT char16_t *LIBCCALL
libc_w16ctime_r(time_t const *__restrict timer,
                char16_t *__restrict buf) {
 struct tm ltm;
 return libc_w16asctime_r(libc_localtime_r(timer,&ltm),buf);
}
EXPORT(__SYMw16(wctime64_r),libc_w16ctime64_r);
CRT_DOS_EXT char16_t *LIBCCALL
libc_w16ctime64_r(time64_t const *__restrict timer,
                  char16_t *__restrict buf) {
 struct tm ltm;
 return libc_w16asctime_r(libc_localtime64_r(timer,&ltm),buf);
}
EXPORT(__SYMw32(wctime_r),libc_w32ctime_r);
CRT_DOS_EXT char32_t *LIBCCALL
libc_w32ctime_r(time_t const *__restrict timer,
                char32_t *__restrict buf) {
 struct tm ltm;
 return libc_w32asctime_r(libc_localtime_r(timer,&ltm),buf);
}
EXPORT(__SYMw32(wctime64_r),libc_w32ctime64_r);
CRT_DOS_EXT char32_t *LIBCCALL
libc_w32ctime64_r(time64_t const *__restrict timer,
                  char32_t *__restrict buf) {
 struct tm ltm;
 return libc_w32asctime_r(libc_localtime64_r(timer,&ltm),buf);
}


EXPORT(__KSYM(ctime_s),libc_ctime_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_ctime_s(char buf[26], size_t bufsize,
             time32_t const *__restrict timer) {
 return bufsize < 26 ? EINVAL : (libc_ctime_r(timer,buf) ? 0 : libc_geterrno());
}
EXPORT(__KSYM(ctime64_s),libc_ctime64_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_ctime64_s(char buf[26], size_t bufsize,
               time64_t const *__restrict timer) {
 return bufsize < 26 ? EINVAL : (libc_ctime64_r(timer,buf) ? 0 : libc_geterrno());
}
EXPORT(__DSYM(_ctime32_s),libc_dos_ctime_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_ctime_s(char buf[26], size_t bufsize,
                 time32_t const *__restrict timer) {
 return libc_errno_kos2dos(libc_ctime_s(buf,bufsize,timer));
}
EXPORT(__DSYM(_ctime64_s),libc_dos_ctime64_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_ctime64_s(char buf[26], size_t bufsize,
                   time64_t const *__restrict timer) {
 return libc_errno_kos2dos(libc_ctime64_s(buf,bufsize,timer));
}


EXPORT(__KSYMw16(wctime_s),libc_w16ctime_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w16ctime_s(char16_t buf[26], size_t bufsize,
                time32_t const *__restrict timer) {
 return bufsize < 26 ? EINVAL : (libc_w16ctime_r(timer,buf) ? 0 : libc_geterrno());
}
EXPORT(__KSYMw32(wctime_s),libc_w32ctime_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w16ctime64_s(char16_t buf[26], size_t bufsize,
                  time64_t const *__restrict timer) {
 return bufsize < 26 ? EINVAL : (libc_w16ctime64_r(timer,buf) ? 0 : libc_geterrno());
}
EXPORT(__KSYMw16(wctime64_s),libc_w16ctime64_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w32ctime_s(char32_t buf[26], size_t bufsize,
                time32_t const *__restrict timer) {
 return bufsize < 26 ? EINVAL : (libc_w32ctime_r(timer,buf) ? 0 : libc_geterrno());
}
EXPORT(__KSYMw32(wctime64_s),libc_w32ctime64_s);
CRT_DOS_EXT errno_t LIBCCALL
libc_w32ctime64_s(char32_t buf[26], size_t bufsize,
                  time64_t const *__restrict timer) {
 return bufsize < 26 ? EINVAL : (libc_w32ctime64_r(timer,buf) ? 0 : libc_geterrno());
}
EXPORT(__DSYMw16(_wctime32_s),libc_dos_w16ctime_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16ctime_s(char16_t buf[26], size_t bufsize,
                    time32_t const *__restrict timer) {
 return libc_errno_kos2dos(libc_w16ctime_s(buf,bufsize,timer));
}
EXPORT(__DSYMw32(wctime_s),libc_dos_w32ctime_s);
CRT_DOS derrno_t LIBCCALL
libc_dos_w16ctime64_s(char16_t buf[26], size_t bufsize,
                      time64_t const *__restrict timer) {
 return libc_errno_kos2dos(libc_w16ctime64_s(buf,bufsize,timer));
}
EXPORT(__DSYMw16(_wctime64_s),libc_dos_w16ctime64_s);
CRT_DOS_EXT derrno_t LIBCCALL
libc_dos_w32ctime_s(char32_t buf[26], size_t bufsize,
                    time32_t const *__restrict timer) {
 return libc_errno_kos2dos(libc_w32ctime_s(buf,bufsize,timer));
}
EXPORT(__DSYMw32(wctime64_s),libc_dos_w32ctime64_s);
CRT_DOS_EXT derrno_t LIBCCALL
libc_dos_w32ctime64_s(char32_t buf[26], size_t bufsize,
                      time64_t const *__restrict timer) {
 return libc_errno_kos2dos(libc_w32ctime64_s(buf,bufsize,timer));
}




EXPORT(__KSYM(ctime),libc_ctime);
EXPORT(__DSYM(_ctime32),libc_ctime);
CRT_TIME char *LIBCCALL libc_ctime(time32_t const *timer) { return libc_ctime_r(timer,libc_ctime_buf); }
EXPORT(__KSYM(ctime64),libc_ctime64);
EXPORT(__DSYM(_ctime64),libc_ctime64);
CRT_TIME char *LIBCCALL libc_ctime64(time64_t const *timer) { return libc_ctime64_r(timer,libc_ctime_buf); }

EXPORT(__KSYMw16(wctime),libc_w16ctime);
EXPORT(__DSYMw16(_wctime32),libc_w16ctime);
CRT_DOS char16_t *LIBCCALL libc_w16ctime(time32_t const *__restrict timer) { return libc_w16ctime_r(timer,(char16_t *)libc_ctime_buf); }
EXPORT(__KSYMw16(wctime64),libc_w16ctime64);
EXPORT(__DSYMw16(_wctime64),libc_w16ctime64);
CRT_DOS char16_t *LIBCCALL libc_w16ctime64(time64_t const *__restrict timer) { return libc_w16ctime64_r(timer,(char16_t *)libc_ctime_buf); }
EXPORT(__KSYMw32(wctime),libc_w32ctime);
EXPORT(__DSYMw32(_wctime32),libc_w32ctime);
CRT_DOS_EXT char32_t *LIBCCALL libc_w32ctime(time32_t const *__restrict timer) { return libc_w32ctime_r(timer,(char32_t *)libc_ctime_buf); }
EXPORT(__KSYMw32(wctime64),libc_w32ctime64);
EXPORT(__DSYMw32(_wctime64),libc_w32ctime64);
CRT_DOS_EXT char32_t *LIBCCALL libc_w32ctime64(time64_t const *__restrict timer) { return libc_w32ctime64_r(timer,(char32_t *)libc_ctime_buf); }



EXPORT(_strdate,libc_strdate);
CRT_DOS char *LIBCCALL libc_strdate(char buf[9]) {
 struct timeval64 tmnow; struct tm now;
 if (libc_gettimeofday64(&tmnow,NULL) ||
    !libc_gmtime64_r(&tmnow.tv_sec,&now))
     return NULL;
 buf[0] = (char)('0'+(now.tm_mon / 10));
 buf[1] = (char)('0'+(now.tm_mon % 10));
 buf[2] = '/';
 buf[3] = (char)('0'+(now.tm_mday / 10));
 buf[4] = (char)('0'+(now.tm_mday % 10));
 buf[5] = '/';
 buf[6] = (char)('0'+((now.tm_year / 10) % 10));
 buf[7] = (char)('0'+(now.tm_year % 10));
 buf[8] = 0;
 return buf;
}

EXPORT(_strtime,libc_strtime);
CRT_DOS char *LIBCCALL libc_strtime(char buf[9]) {
 struct timeval64 tmnow; struct tm now;
 if (libc_gettimeofday64(&tmnow,NULL) ||
    !libc_gmtime64_r(&tmnow.tv_sec,&now))
     return NULL;
 buf[0] = (char)('0'+(now.tm_hour / 10));
 buf[1] = (char)('0'+(now.tm_hour % 10));
 buf[2] = '/';
 buf[3] = (char)('0'+(now.tm_min / 10));
 buf[4] = (char)('0'+(now.tm_min % 10));
 buf[5] = '/';
 buf[6] = (char)('0'+(now.tm_sec / 10));
 buf[7] = (char)('0'+(now.tm_sec % 10));
 buf[8] = 0;
 return buf;
}

CRT_DOS void LIBCCALL
copy_time16(char16_t dst[9], char src[9]) {
 unsigned int i;
 for (i = 0; i < 9; ++i)
     dst[i] = (char16_t)src[i];
}
CRT_DOS_EXT void LIBCCALL
copy_time32(char32_t dst[9], char src[9]) {
 unsigned int i;
 for (i = 0; i < 9; ++i)
     dst[i] = (char32_t)src[i];
}

EXPORT(__SYMw16(_wstrdate),libc_w16date);
CRT_DOS char16_t *LIBCCALL libc_w16date(char16_t buf[9]) {
 char cbuf[9];
 if (!libc_strdate(cbuf)) return NULL;
 copy_time16(buf,cbuf);
 return buf;
}
EXPORT(__SYMw16(_wstrtime),libc_w16time);
CRT_DOS char16_t *LIBCCALL libc_w16time(char16_t buf[9]) {
 char cbuf[9];
 if (!libc_strtime(cbuf)) return NULL;
 copy_time16(buf,cbuf);
 return buf;
}
EXPORT(__SYMw32(wstrdate),libc_w32date);
CRT_DOS_EXT char32_t *LIBCCALL libc_w32date(char32_t buf[9]) {
 char cbuf[9];
 if (!libc_strdate(cbuf)) return NULL;
 copy_time32(buf,cbuf);
 return buf;
}
EXPORT(__SYMw32(wstrtime),libc_w32time);
CRT_DOS_EXT char32_t *LIBCCALL libc_w32time(char32_t buf[9]) {
 char cbuf[9];
 if (!libc_strtime(cbuf)) return NULL;
 copy_time32(buf,cbuf);
 return buf;
}

EXPORT(__KSYM(strdate_s),libc_strdate_s);
CRT_DOS_EXT errno_t LIBCCALL libc_strdate_s(char buf[9]) {
 return libc_strdate(buf) ? 0 : libc_geterrno();
}
EXPORT(__KSYM(strtime_s),libc_strtime_s);
CRT_DOS_EXT errno_t LIBCCALL libc_strtime_s(char buf[9]) {
 return libc_strtime(buf) ? 0 : libc_geterrno();
}
EXPORT(__KSYMw16(wstrdate_s),libc_w16date_s);
CRT_DOS_EXT errno_t LIBCCALL libc_w16date_s(char16_t buf[9]) {
 return libc_w16date(buf) ? 0 : libc_geterrno();
}
EXPORT(__KSYMw32(wstrdate_s),libc_w32date_s);
CRT_DOS_EXT errno_t LIBCCALL libc_w32date_s(char32_t buf[9]) {
 return libc_w32date(buf) ? 0 : libc_geterrno();
}
EXPORT(__KSYMw16(wstrtime_s),libc_w16time_s);
CRT_DOS_EXT errno_t LIBCCALL libc_w16time_s(char16_t buf[9]) {
 return libc_w16time(buf) ? 0 : libc_geterrno();
}
EXPORT(__KSYMw32(wstrtime_s),libc_w32time_s);
CRT_DOS_EXT errno_t LIBCCALL libc_w32time_s(char32_t buf[9]) {
 return libc_w32time(buf) ? 0 : libc_geterrno();
}


EXPORT(__DSYM(_strdate_s),libc_dos_strdate_s);
CRT_DOS derrno_t LIBCCALL libc_dos_strdate_s(char buf[9]) {
 return libc_errno_kos2dos(libc_strdate_s(buf));
}
EXPORT(_strtime_s,libc_dos_strtime_s);
CRT_DOS derrno_t LIBCCALL libc_dos_strtime_s(char buf[9]) {
 return libc_errno_kos2dos(libc_strtime_s(buf));
}
EXPORT(__SYMw16(_wstrdate_s),libc_dos_w16date_s);
CRT_DOS derrno_t LIBCCALL libc_dos_w16date_s(char16_t buf[9]) {
 return libc_errno_kos2dos(libc_w16date_s(buf));
}
EXPORT(__SYMw32(wstrdate_s),libc_dos_w32date_s);
CRT_DOS_EXT derrno_t LIBCCALL libc_dos_w32date_s(char32_t buf[9]) {
 return libc_errno_kos2dos(libc_w32date_s(buf));
}
EXPORT(__SYMw16(_wstrtime_s),libc_dos_w16time_s);
CRT_DOS derrno_t LIBCCALL libc_dos_w16time_s(char16_t buf[9]) {
 return libc_errno_kos2dos(libc_w16time_s(buf));
}
EXPORT(__SYMw32(wstrtime_s),libc_dos_w32time_s);
CRT_DOS_EXT derrno_t LIBCCALL libc_dos_w32time_s(char32_t buf[9]) {
 return libc_errno_kos2dos(libc_w32time_s(buf));
}




/* These are defined by DOS... */
CRT_DOS size_t LIBCCALL
libc_Strftime(char *__restrict s, size_t maxsize, char const *__restrict format,
              struct tm const *__restrict tp, void *UNUSED(lc_time_arg)) {
 return libc_strftime(s,maxsize,format,tp);
}
CRT_DOS size_t LIBCCALL
libc_Strftime_l(char *__restrict s, size_t maxsize, char const *__restrict format,
                struct tm const *__restrict tp, void *UNUSED(lc_time_arg), locale_t locale) {
 return libc_strftime_l(s,maxsize,format,tp,locale);
}
EXPORT(__DSYM(_Strftime),libc_Strftime);
EXPORT(__DSYM(_Strftime_l),libc_Strftime_l);

DECL_END

#endif /* !GUARD_LIBS_LIBC_FORMAT_TIME_C */
