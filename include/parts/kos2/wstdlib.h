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
#ifndef _PARTS_KOS2_WSTDLIB_H
#define _PARTS_KOS2_WSTDLIB_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <xlocale.h>

__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef __KERNEL__

#ifdef __CRT_DOS
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,wsystem,(wchar_t const *__restrict __cmd),(__cmd))
#endif /* __CRT_DOS */

__REDIRECT(__LIBC,__NONNULL((1)),__LONGLONG,__LIBCCALL,wcstoq_l,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale),wcstoll_l,(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,__NONNULL((1)),__ULONGLONG,__LIBCCALL,wcstouq_l,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale),wcstoull_l,(__nptr,__endptr,__base,__locale))

#if __SIZEOF_LONG__ == 4
__REDIRECT(__LIBC,__NONNULL((1)),__INT32_TYPE__,__LIBCCALL,wcsto32,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__INT32_TYPE__,__LIBCCALL,wcsto32_l,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstol_l),(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,__NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,wcstou32,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,wcstou32_l,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstoul_l),(__nptr,__endptr,__base,__locale))
#elif __SIZEOF_LONG_LONG__ == 4
__REDIRECT(__LIBC,__NONNULL((1)),__INT32_TYPE__,__LIBCCALL,wcsto32,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base),wcstoll,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__INT32_TYPE__,__LIBCCALL,wcsto32_l,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstoll_l),(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,__NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,wcstou32,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base),wcstoull,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__UINT32_TYPE__,__LIBCCALL,wcstou32_l,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstoull_l),(__nptr,__endptr,__base,__locale))
#else
__LIBC __NONNULL((1)) __INT32_TYPE__ (__LIBCCALL wcsto32)(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base);
__LIBC __NONNULL((1)) __INT32_TYPE__ (__LIBCCALL wcsto32_l)(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale);
__LIBC __NONNULL((1)) __UINT32_TYPE__ (__LIBCCALL wcstou32)(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base);
__LIBC __NONNULL((1)) __UINT32_TYPE__ (__LIBCCALL wcstou32_l)(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale);
#endif
#if __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,__NONNULL((1)),__INT64_TYPE__,__LIBCCALL,wcsto64,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__INT64_TYPE__,__LIBCCALL,wcsto64_l,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstol_l),(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,__NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,wcstou64,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,wcstou64_l,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstoul_l),(__nptr,__endptr,__base,__locale))
#elif __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,__NONNULL((1)),__INT64_TYPE__,__LIBCCALL,wcsto64,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base),wcstoll,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__INT64_TYPE__,__LIBCCALL,wcsto64_l,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstoll_l),(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,__NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,wcstou64,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base),wcstoull,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__UINT64_TYPE__,__LIBCCALL,wcstou64_l,(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstoull_l),(__nptr,__endptr,__base,__locale))
#else
__LIBC __NONNULL((1)) __INT64_TYPE__ (__LIBCCALL wcsto64)(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base);
__LIBC __NONNULL((1)) __INT64_TYPE__ (__LIBCCALL wcsto64_l)(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale);
__LIBC __NONNULL((1)) __UINT64_TYPE__ (__LIBCCALL wcstou64)(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base);
__LIBC __NONNULL((1)) __UINT64_TYPE__ (__LIBCCALL wcstou64_l)(wchar_t *__restrict __nptr, wchar_t **__endptr, int __base, __locale_t __locale);
#endif
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_PARTS_KOS2_WSTDLIB_H */
