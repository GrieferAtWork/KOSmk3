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
#ifndef GUARD_LIBS_LIBC_FORMAT_TIME_H
#define GUARD_LIBS_LIBC_FORMAT_TIME_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     FORMAT_TIME                                                                       */
/* ===================================================================================== */
INTDEF char const libc_abbr_month_names[12][4];
INTDEF char const libc_abbr_wday_names[7][4];
INTDEF char const libc_full_month_names[12][10];
INTDEF char const libc_full_wday_names[7][10];
INTDEF char const libc_am_pm[2][3];
INTDEF char const libc_am_pm_lower[2][3];

INTDEF char const libc_3_questionmarks[];

INTDEF char *LIBCCALL libc_strptime(char const *__restrict s, char const *__restrict format, struct tm *tp);
INTDEF char *LIBCCALL libc_strptime_l(char const *__restrict s, char const *__restrict format, struct tm *tp, locale_t locale);

DATDEF int getdate_err;
INTDEF struct tm *LIBCCALL libc_getdate(char const *string);
INTDEF int LIBCCALL libc_getdate_r(char const *__restrict string, struct tm *__restrict resbufp);

/* asctime_r() */
INTDEF char *LIBCCALL libc_asctime_r(struct tm const *__restrict tp, char *__restrict buf);
INTDEF char16_t *LIBCCALL libc_w16asctime_r(struct tm const *__restrict tp, char16_t *__restrict buf);
INTDEF char32_t *LIBCCALL libc_w32asctime_r(struct tm const *__restrict tp, char32_t *__restrict buf);
/* asctime_s() */
INTDEF errno_t LIBCCALL libc_asctime_s(char buf[26], size_t bufsize, struct tm const *__restrict tp);
INTDEF derrno_t LIBCCALL libc_dos_asctime_s(char buf[26], size_t bufsize, struct tm const *__restrict tp);
INTDEF errno_t LIBCCALL libc_w16asctime_s(char16_t buf[26], size_t bufsize, struct tm const *__restrict tp);
INTDEF derrno_t LIBCCALL libc_dos_w16asctime_s(char16_t buf[26], size_t bufsize, struct tm const *__restrict tp);
INTDEF errno_t LIBCCALL libc_w32asctime_s(char32_t buf[26], size_t bufsize, struct tm const *__restrict tp);
INTDEF derrno_t LIBCCALL libc_dos_w32asctime_s(char32_t buf[26], size_t bufsize, struct tm const *__restrict tp);
/* asctime() */
INTDEF char *LIBCCALL libc_asctime(struct tm const *tp);
INTDEF char16_t *LIBCCALL libc_w16asctime(struct tm const *tp);
INTDEF char32_t *LIBCCALL libc_w32asctime(struct tm const *tp);
/* ctime_r() / ctime64_r() */
INTDEF char *LIBCCALL libc_ctime_r(time32_t const *__restrict timer, char *__restrict buf);
INTDEF char *LIBCCALL libc_ctime64_r(time64_t const *__restrict timer, char *__restrict buf);
INTDEF char16_t *LIBCCALL libc_w16ctime_r(time32_t const *__restrict timer, char16_t *__restrict buf);
INTDEF char16_t *LIBCCALL libc_w16ctime64_r(time64_t const *__restrict timer, char16_t *__restrict buf);
INTDEF char32_t *LIBCCALL libc_w32ctime_r(time32_t const *__restrict timer, char32_t *__restrict buf);
INTDEF char32_t *LIBCCALL libc_w32ctime64_r(time64_t const *__restrict timer, char32_t *__restrict buf);
/* ctime_s() / ctime64_s() */
INTDEF errno_t LIBCCALL libc_ctime_s(char buf[26], size_t bufsize, time32_t const *__restrict timer);
INTDEF errno_t LIBCCALL libc_ctime64_s(char buf[26], size_t bufsize, time64_t const *__restrict timer);
INTDEF derrno_t LIBCCALL libc_dos_ctime_s(char buf[26], size_t bufsize, time32_t const *__restrict timer);
INTDEF derrno_t LIBCCALL libc_dos_ctime64_s(char buf[26], size_t bufsize, time64_t const *__restrict timer);
INTDEF errno_t LIBCCALL libc_w16ctime_s(char16_t buf[26], size_t bufsize, time32_t const *__restrict timer);
INTDEF errno_t LIBCCALL libc_w16ctime64_s(char16_t buf[26], size_t bufsize, time64_t const *__restrict timer);
INTDEF errno_t LIBCCALL libc_w32ctime_s(char32_t buf[26], size_t bufsize, time32_t const *__restrict timer);
INTDEF errno_t LIBCCALL libc_w32ctime64_s(char32_t buf[26], size_t bufsize, time64_t const *__restrict timer);
INTDEF derrno_t LIBCCALL libc_dos_w16ctime_s(char16_t buf[26], size_t bufsize, time32_t const *__restrict timer);
INTDEF derrno_t LIBCCALL libc_dos_w16ctime64_s(char16_t buf[26], size_t bufsize, time64_t const *__restrict timer);
INTDEF derrno_t LIBCCALL libc_dos_w32ctime_s(char32_t buf[26], size_t bufsize, time32_t const *__restrict timer);
INTDEF derrno_t LIBCCALL libc_dos_w32ctime64_s(char32_t buf[26], size_t bufsize, time64_t const *__restrict timer);
/* ctime() / ctime64() */
INTDEF char *LIBCCALL libc_ctime(time32_t const *timer);
INTDEF char *LIBCCALL libc_ctime64(time64_t const *timer);
INTDEF char16_t *LIBCCALL libc_w16ctime(time32_t const *__restrict timer);
INTDEF char16_t *LIBCCALL libc_w16ctime64(time64_t const *__restrict timer);
INTDEF char32_t *LIBCCALL libc_w32ctime(time32_t const *__restrict timer);
INTDEF char32_t *LIBCCALL libc_w32ctime64(time64_t const *__restrict timer);


INTDEF char *LIBCCALL libc_strdate(char buf[9]);
INTDEF char *LIBCCALL libc_strtime(char buf[9]);
INTDEF char16_t *LIBCCALL libc_w16date(char16_t buf[9]);
INTDEF char16_t *LIBCCALL libc_w16time(char16_t buf[9]);
INTDEF char32_t *LIBCCALL libc_w32date(char32_t buf[9]);
INTDEF char32_t *LIBCCALL libc_w32time(char32_t buf[9]);
INTDEF errno_t LIBCCALL libc_strdate_s(char buf[9]);
INTDEF errno_t LIBCCALL libc_strtime_s(char buf[9]);
INTDEF derrno_t LIBCCALL libc_dos_strdate_s(char buf[9]);
INTDEF derrno_t LIBCCALL libc_dos_strtime_s(char buf[9]);
INTDEF errno_t LIBCCALL libc_w16date_s(char16_t buf[9]);
INTDEF errno_t LIBCCALL libc_w16time_s(char16_t buf[9]);
INTDEF errno_t LIBCCALL libc_w32date_s(char32_t buf[9]);
INTDEF errno_t LIBCCALL libc_w32time_s(char32_t buf[9]);
INTDEF derrno_t LIBCCALL libc_dos_w16date_s(char16_t buf[9]);
INTDEF derrno_t LIBCCALL libc_dos_w16time_s(char16_t buf[9]);
INTDEF derrno_t LIBCCALL libc_dos_w32date_s(char32_t buf[9]);
INTDEF derrno_t LIBCCALL libc_dos_w32time_s(char32_t buf[9]);

/* Some dos-specific functions... */
INTDEF size_t LIBCCALL libc_Strftime(char *__restrict s, size_t maxsize, char const *__restrict format, struct tm const *__restrict tp, void *lc_time_arg);
INTDEF size_t LIBCCALL libc_Strftime_l(char *__restrict s, size_t maxsize, char const *__restrict format, struct tm const *__restrict tp, void *lc_time_arg, locale_t locale);



DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_FORMAT_TIME_H */
