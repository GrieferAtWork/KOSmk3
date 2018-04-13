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
#ifndef _PARTS_DOS_WCHAR_H
#define _PARTS_DOS_WCHAR_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#ifndef _WCHAR_H
#include <wchar.h>
#endif

__SYSDECL_BEGIN

#ifndef __intptr_t_defined
#define __intptr_t_defined 1
typedef __intptr_t intptr_t;
#endif /* !__intptr_t_defined */

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef _INO_T_DEFINED
#define _INO_T_DEFINED 1
typedef __typedef_ino_t _ino_t;
#endif /* !_INO_T_DEFINED */

#ifndef _DEV_T_DEFINED
#define _DEV_T_DEFINED 1
typedef __typedef_dev_t _dev_t;
#endif /* !_DEV_T_DEFINED */

#ifndef _OFF_T_DEFINED
#define _OFF_T_DEFINED 1
typedef __typedef_off_t _off_t;
#endif /* !_OFF_T_DEFINED */

#ifndef __ino_t_defined
#define __ino_t_defined 1
typedef __typedef_ino_t ino_t;
#endif /* !__ino_t_defined */

#ifndef __dev_t_defined
#define __dev_t_defined 1
typedef __typedef_dev_t dev_t;
#endif /* !__dev_t_defined */

#ifndef __off_t_defined
#define __off_t_defined 1
typedef __typedef_off_t off_t;
#endif /* !__off_t_defined */

#ifndef __rsize_t_defined
#define __rsize_t_defined 1
typedef size_t rsize_t;
#endif /* !__rsize_t_defined */

#ifndef __va_list_defined
#define __va_list_defined 1
typedef __builtin_va_list va_list;
#endif /* !__va_list_defined */

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

typedef wchar_t _Wint_t;

#ifdef __CRT_DOS


#ifndef _CRT_WPERROR_DEFINED
#define _CRT_WPERROR_DEFINED 1
__REDIRECT_DPB_VOID(__LIBC,__PORT_DOSONLY __ATTR_COLD,__LIBCCALL,wperror,(wchar_t const *__restrict __errmsg),(__errmsg))
#endif /* !_CRT_WPERROR_DEFINED */
#endif /* __CRT_DOS */

#ifndef __std_tm_defined
#define __std_tm_defined 1
__NAMESPACE_STD_BEGIN
struct tm {
    int tm_sec;   /* seconds [0,61]. */
    int tm_min;   /* minutes [0,59]. */
    int tm_hour;  /* hour [0,23]. */
    int tm_mday;  /* day of month [1,31]. */
    int tm_mon;   /* month of year [0,11]. */
    int tm_year;  /* years since 1900. */
    int tm_wday;  /* day of week [0,6] )(Sunday = 0). */
    int tm_yday;  /* day of year [0,365]. */
    int tm_isdst; /* daylight savings flag. */
#if !defined(__DOS_COMPAT__) && defined(__CRT_GLC)
#if defined(__USE_MISC) && !defined(__USE_DOS)
    long int    tm_gmtoff;   /* Seconds east of UTC.  */
    char const *tm_zone;     /* Timezone abbreviation.  */
#else
    long int    __tm_gmtoff; /* Seconds east of UTC.  */
    char const *__tm_zone;   /* Timezone abbreviation.  */
#endif
#endif /* !... */
};
__NAMESPACE_STD_END
#endif /* !__std_tm_defined */
#ifndef __tm_defined
#define __tm_defined 1
__NAMESPACE_STD_USING(tm)
#endif /* !__tm_defined */

#ifdef __CRT_DOS
#ifndef __mbstate_t_defined
#define __mbstate_t_defined 1
__NAMESPACE_STD_USING(mbstate_t)
#endif /* !__mbstate_t_defined */

__LIBC __PORT_DOSONLY errno_t (__LIBCCALL mbsrtowcs_s)(size_t *__result, wchar_t *__restrict __buf, size_t __buflen, char const **__restrict __psrc, size_t __srcsize, mbstate_t *__restrict __ps);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL wcsrtombs_s)(size_t *__result, char *__restrict __buf, size_t __buflen, wchar_t const **__restrict __psrc, size_t __srcsize, mbstate_t *__restrict __ps);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL wcrtomb_s)(size_t *__result, char *__restrict __buf, size_t __buflen, wchar_t __wc, mbstate_t *__restrict __ps);
#endif /* __CRT_DOS */

#ifndef __std_memcpy_defined
#define __std_memcpy_defined 1
__NAMESPACE_STD_BEGIN
#ifndef __OPTIMIZE_LIBC__
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes);
#elif defined(__cplusplus)
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes) { return __opt_memcpy(__dst,__src,__n_bytes); }
#else
#define memcpy(dst,src,n_bytes)  __opt_memcpy(dst,src,n_bytes)
#endif
__NAMESPACE_STD_END
#endif /* !__std_memcpy_defined */
#ifndef __memcpy_defined
#define __memcpy_defined 1
__NAMESPACE_STD_USING(memcpy)
#endif /* !__memcpy_defined */
#ifndef __std_memmove_defined
#define __std_memmove_defined 1
__NAMESPACE_STD_BEGIN
#ifndef __OPTIMIZE_LIBC__
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memmove)(void *__dst, void const *__src, size_t __n_bytes);
#elif defined(__cplusplus)
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memmove)(void *__dst, void const *__src, size_t __n_bytes) { return __opt_memmove(__dst,__src,__n_bytes); }
#else
#define memmove(dst,src,n_bytes) __opt_memmove(__dst,__src,__n_bytes)
#endif
__NAMESPACE_STD_END
#endif /* !__std_memmove_defined */
#ifndef __memmove_defined
#define __memmove_defined 1
__NAMESPACE_STD_USING(memmove)
#endif /* !__memmove_defined */

#ifdef __USE_DOS_SLIB
/* TODO: These could be implemented inline. */
#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL memcpy_s)(void *__restrict __dst, rsize_t __dstsize, void const *__restrict __src, rsize_t __srcsize);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL memmove_s)(void *__dst, rsize_t __dstsize, void const *__src, rsize_t __srcsize);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL wmemcpy_s)(wchar_t *__restrict __dst, rsize_t __dstsize, wchar_t const *__restrict __src, rsize_t __srcsize);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL wmemmove_s)(wchar_t *__dst, rsize_t __dstsize, wchar_t const *__src, rsize_t __srcsize);
#endif /* __CRT_DOS */
#endif /* __USE_DOS_SLIB */

__SYSDECL_END

#include "wdirect.h"
#include "wio.h"
#include "wlocale.h"
#include "wstdio.h"
#include "wstdlib.h"
#include "wprocess.h"
#include "wstat.h"
#include "wconio.h"
#include "wstring.h"
#include "wtime.h"

#endif /* !_PARTS_DOS_WCHAR_H */
