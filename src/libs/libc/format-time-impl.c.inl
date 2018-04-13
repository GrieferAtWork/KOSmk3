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
#ifdef __INTELLISENSE__
#define _UTF_SOURCE 1
#include "format-time.c"
#include "libc.h"
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#define FORMAT_OPTION_LOCALE     1 /* Enable locale support. */
#endif

#include <format-printer.h>
#include <except.h>
#include <stdbool.h>

#ifndef FORMAT_OPTION_CHARTYPE
#error "Must #define FORMAT_OPTION_CHARTYPE"
#endif

#ifndef format_T_char
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#   define format_T_char       char
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define format_T_char       char16_t
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR32
#   define format_T_char       char32_t
#else
#   error "Invalid character type"
#endif
#endif /* !format_T_char */

#ifndef PFORMATPRINTER
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#   define PFORMATPRINTER      pformatprinter
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define PFORMATPRINTER      pw16formatprinter
#else
#   define PFORMATPRINTER      pw32formatprinter
#endif
#endif /* !PFORMATPRINTER */

#ifndef LIBC_FORMAT_STRFTIME
#ifdef FORMAT_OPTION_LOCALE
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_STRFTIME  libc_format_strftime_l
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_STRFTIME  libc_format_w16ftime_l
#   else
#      define LIBC_FORMAT_STRFTIME  libc_format_w32ftime_l
#   endif
#else /* FORMAT_OPTION_LOCALE */
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_STRFTIME  libc_format_strftime
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_STRFTIME  libc_format_w16ftime
#   else
#      define LIBC_FORMAT_STRFTIME  libc_format_w32ftime
#   endif
#endif /* !FORMAT_OPTION_LOCALE */
#endif /* !LIBC_FORMAT_VPRINTF */

#ifndef LIBC_FORMAT_VPRINTF
#ifdef FORMAT_OPTION_LOCALE
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_PRINTF    libc_format_printf_l
#      define LIBC_FORMAT_VPRINTF   libc_format_vprintf_l
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_PRINTF    libc_format_w16printf_l
#      define LIBC_FORMAT_VPRINTF   libc_format_vw16printf_l
#   else
#      define LIBC_FORMAT_PRINTF    libc_format_w32printf_l
#      define LIBC_FORMAT_VPRINTF   libc_format_vw32printf_l
#   endif
#else /* FORMAT_OPTION_LOCALE */
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_PRINTF    libc_format_printf
#      define LIBC_FORMAT_VPRINTF   libc_format_vprintf
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_PRINTF    libc_format_w16printf
#      define LIBC_FORMAT_VPRINTF   libc_format_vw16printf
#   else
#      define LIBC_FORMAT_PRINTF    libc_format_w32printf
#      define LIBC_FORMAT_VPRINTF   libc_format_vw32printf
#   endif
#endif /* !FORMAT_OPTION_LOCALE */
#endif /* !LIBC_FORMAT_VPRINTF */


DECL_BEGIN

#ifndef TIME_ATTRIBUTES_DEFINED
#define TIME_ATTRIBUTES_DEFINED 1
#define TMEXNAME_MAXLEN  6

INTERN char const libc_abbr_month_names[12][4] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun", 
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};
INTERN char const libc_abbr_wday_names[7][4] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};
INTERN char const libc_full_month_names[12][10] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};
INTERN char const libc_full_wday_names[7][10] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday", 
    "Thursday",
    "Friday",
    "Saturday"
};
INTERN char const libc_am_pm[2][3] = {
    "AM",
    "PM"
};
INTERN char const libc_am_pm_lower[2][3] = {
    "am",
    "pm"
};

struct time_attrib {
    char name[TMEXNAME_MAXLEN+1];
    u8   offset;
};

PRIVATE struct time_attrib const time_attr[] = {
#define TIME_ATTRIBUTE(name,field) \
      { name, (u8)offsetof(struct tm,field) }
    TIME_ATTRIBUTE("Y",      tm_year),
    TIME_ATTRIBUTE("M",      tm_mon),
    TIME_ATTRIBUTE("D",      tm_mday),
    TIME_ATTRIBUTE("H",      tm_hour),
    TIME_ATTRIBUTE("I",      tm_min),
    TIME_ATTRIBUTE("S",      tm_sec),
    TIME_ATTRIBUTE("MD",     tm_mday),
    TIME_ATTRIBUTE("MI",     tm_min),
    TIME_ATTRIBUTE("YD",     tm_yday),
    TIME_ATTRIBUTE("WD",     tm_wday),
    TIME_ATTRIBUTE("year",   tm_year),
    TIME_ATTRIBUTE("month",  tm_mon),
    TIME_ATTRIBUTE("wday",   tm_wday),
    TIME_ATTRIBUTE("mday",   tm_mday),
    TIME_ATTRIBUTE("yday",   tm_yday),
    TIME_ATTRIBUTE("hour",   tm_hour),
    TIME_ATTRIBUTE("minute", tm_min),
    TIME_ATTRIBUTE("second", tm_sec),
#undef TIME_ATTRIBUTE
    { "", 0 },
};
#endif /* !TIME_ATTRIBUTES_DEFINED */


#define PRINT(p,s) \
do{ if unlikely((temp = (*printer)(p,s,closure)) < 0) goto err; \
    result += temp; \
}__WHILE0



INTERN ssize_t LIBCCALL
LIBC_FORMAT_STRFTIME(PFORMATPRINTER printer, void *closure,
                     format_T_char const *__restrict format,
                     struct tm const *__restrict tm
#ifdef FORMAT_OPTION_LOCALE
                     ,
                     locale_t locale
#endif
                     ) {
 ssize_t result = 0;

 /* TODO */

 return result;
}

#undef PRINT
#undef format_T_char
#undef LIBC_FORMAT_QUOTE
#undef PFORMATPRINTER
#undef LIBC_FORMAT_PRINTF
#undef LIBC_FORMAT_VPRINTF
#undef LIBC_FORMAT_STRFTIME
#undef FORMAT_OPTION_CHARTYPE
#undef FORMAT_OPTION_LOCALE

DECL_END
