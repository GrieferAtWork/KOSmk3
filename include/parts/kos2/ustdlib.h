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
#ifndef _PARTS_KOS2_USTDLIB_H
#define _PARTS_KOS2_USTDLIB_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <xlocale.h>

__SYSDECL_BEGIN

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

#ifndef __KERNEL__
#ifdef __CRT_KOS
__REDIRECT_EXCEPT_UFSDPW16(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,w16system,(char16_t const *__restrict __cmd),wsystem,(__cmd))
__REDIRECT_EXCEPT_UFSW32  (__LIBC,__PORT_KOSONLY,int,__LIBCCALL,w32system,(char32_t const *__restrict __cmd),wsystem,(__cmd))

__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),long,__LIBCCALL,w16tol,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),long,__LIBCCALL,w32tol,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),long,__LIBCCALL,w16tol_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),long,__LIBCCALL,w32tol_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),unsigned long,__LIBCCALL,w16toul,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),unsigned long,__LIBCCALL,w32toul,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),unsigned long,__LIBCCALL,w16toul_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoul_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),unsigned long,__LIBCCALL,w32toul_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoul_l,(__nptr,__endptr,__base,__locale))
#if __SIZEOF_LONG__ == 8
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w16toll,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w32toll,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w16toll_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w32toll_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w16toull,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w32toull,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w16toull_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoul_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w32toull_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoul_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w16toq,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w32toq,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w16toq_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w32toq_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w16touq,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w32touq,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w16touq_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoul_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w32touq_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoul_l,(__nptr,__endptr,__base,__locale))
#else
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w16toll,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoll,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w32toll,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoll,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w16toll_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoll_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w32toll_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoll_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w16toull,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoull,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w32toull,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoull,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w16toull_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoull_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w32toull_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoull_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w16toq,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoll,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w32toq,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoll,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w16toq_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoll_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LONGLONG,__LIBCCALL,w32toq_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoll_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w16touq,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoull,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w32touq,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoull,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w16touq_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoull_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__ULONGLONG,__LIBCCALL,w32touq_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoull_l,(__nptr,__endptr,__base,__locale))
#endif
#if __SIZEOF_INTMAX_T__ == __SIZEOF_LONG__
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INTMAX_TYPE__,__LIBCCALL,w16toimax,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INTMAX_TYPE__,__LIBCCALL,w32toimax,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INTMAX_TYPE__,__LIBCCALL,w16toimax_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INTMAX_TYPE__,__LIBCCALL,w32toimax_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINTMAX_TYPE__,__LIBCCALL,w16toumax,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINTMAX_TYPE__,__LIBCCALL,w32toumax,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINTMAX_TYPE__,__LIBCCALL,w16toumax_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoul_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINTMAX_TYPE__,__LIBCCALL,w32toumax_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoul_l,(__nptr,__endptr,__base,__locale))
#elif __SIZEOF_INTMAX_T__ == __SIZEOF_LONG_LONG__
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INTMAX_TYPE__,__LIBCCALL,w16toimax,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoll,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INTMAX_TYPE__,__LIBCCALL,w32toimax,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoll,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INTMAX_TYPE__,__LIBCCALL,w16toimax_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoll_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INTMAX_TYPE__,__LIBCCALL,w32toimax_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoll_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINTMAX_TYPE__,__LIBCCALL,w16toumax,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoull,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINTMAX_TYPE__,__LIBCCALL,w32toumax,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoull,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINTMAX_TYPE__,__LIBCCALL,w16toumax_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoull_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINTMAX_TYPE__,__LIBCCALL,w32toumax_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoull_l,(__nptr,__endptr,__base,__locale))
#else
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INTMAX_TYPE__,__LIBCCALL,w16toimax,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoimax,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INTMAX_TYPE__,__LIBCCALL,w32toimax,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoimax,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INTMAX_TYPE__,__LIBCCALL,w16toimax_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoimax_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INTMAX_TYPE__,__LIBCCALL,w32toimax_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoimax_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINTMAX_TYPE__,__LIBCCALL,w16toumax,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoumax,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINTMAX_TYPE__,__LIBCCALL,w32toumax,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoumax,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINTMAX_TYPE__,__LIBCCALL,w16toumax_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoumax_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINTMAX_TYPE__,__LIBCCALL,w32toumax_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoumax_l,(__nptr,__endptr,__base,__locale))
#endif

#if __SIZEOF_LONG__ == 4
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT32_TYPE__,__LIBCCALL,w16to32,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT32_TYPE__,__LIBCCALL,w32to32,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT32_TYPE__,__LIBCCALL,w16to32_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT32_TYPE__,__LIBCCALL,w32to32_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,w16tou32,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,w32tou32,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,w16tou32_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoul_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,w32tou32_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoul_l,(__nptr,__endptr,__base,__locale))
#elif __SIZEOF_LONG_LONG__ == 4
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT32_TYPE__,__LIBCCALL,w16to32,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoll,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT32_TYPE__,__LIBCCALL,w32to32,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoll,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT32_TYPE__,__LIBCCALL,w16to32_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoll_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT32_TYPE__,__LIBCCALL,w32to32_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoll_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,w16tou32,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoull,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,w32tou32,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoull,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,w16tou32_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoull_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,w32tou32_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoull_l,(__nptr,__endptr,__base,__locale))
#else
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT32_TYPE__,__LIBCCALL,w16to32,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcsto32,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT32_TYPE__,__LIBCCALL,w32to32,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcsto32,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT32_TYPE__,__LIBCCALL,w16to32_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcsto32_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT32_TYPE__,__LIBCCALL,w32to32_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcsto32_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,w16tou32,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstou32,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,w32tou32,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstou32,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,w16tou32_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstou32_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,w32tou32_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstou32_l,(__nptr,__endptr,__base,__locale))
#endif

#if __SIZEOF_LONG__ == 8
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT64_TYPE__,__LIBCCALL,w16to64,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT64_TYPE__,__LIBCCALL,w32to64,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT64_TYPE__,__LIBCCALL,w16to64_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT64_TYPE__,__LIBCCALL,w32to64_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,w16tou64,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,w32tou64,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,w16tou64_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoul_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,w32tou64_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoul_l,(__nptr,__endptr,__base,__locale))
#elif __SIZEOF_LONG_LONG__ == 8
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT64_TYPE__,__LIBCCALL,w16to64,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoll,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT64_TYPE__,__LIBCCALL,w32to64,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoll,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT64_TYPE__,__LIBCCALL,w16to64_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoll_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT64_TYPE__,__LIBCCALL,w32to64_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoll_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,w16tou64,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstoull,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,w32tou64,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstoull,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,w16tou64_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstoull_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,w32tou64_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstoull_l,(__nptr,__endptr,__base,__locale))
#else
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT64_TYPE__,__LIBCCALL,w16to64,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcsto64,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT64_TYPE__,__LIBCCALL,w32to64,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcsto64,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT64_TYPE__,__LIBCCALL,w16to64_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcsto64_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__INT64_TYPE__,__LIBCCALL,w32to64_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcsto64_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,w16tou64,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstou64,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,w32tou64,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstou64,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,w16tou64_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstou64_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,w32tou64_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstou64_l,(__nptr,__endptr,__base,__locale))
#endif

__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),float,__LIBCCALL,w16tof,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstof,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),float,__LIBCCALL,w32tof,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstof,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),float,__LIBCCALL,w16tof_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstof_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),float,__LIBCCALL,w32tof_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstof_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),double,__LIBCCALL,w16tod,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstod,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),double,__LIBCCALL,w32tod,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstod,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),double,__LIBCCALL,w16tod_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstod_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),double,__LIBCCALL,w32tod_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstod_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),long double,__LIBCCALL,w16told,(char16_t *__restrict __nptr, char16_t **__endptr, int __base),wcstold,(__nptr,__endptr,__base))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),long double,__LIBCCALL,w32told,(char32_t *__restrict __nptr, char32_t **__endptr, int __base),wcstold,(__nptr,__endptr,__base))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),long double,__LIBCCALL,w16told_l,(char16_t *__restrict __nptr, char16_t **__endptr, int __base, __locale_t __locale),wcstold_l,(__nptr,__endptr,__base,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),long double,__LIBCCALL,w32told_l,(char32_t *__restrict __nptr, char32_t **__endptr, int __base, __locale_t __locale),wcstold_l,(__nptr,__endptr,__base,__locale))
#endif /* __CRT_KOS */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_PARTS_KOS2_USTDLIB_H */
