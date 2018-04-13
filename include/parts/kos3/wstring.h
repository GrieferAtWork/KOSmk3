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
#ifndef _PARTS_KOS3_WSTRING_H
#define _PARTS_KOS3_WSTRING_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <xlocale.h>

#if defined(__CC__) && !defined(__KERNEL__)
__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

/* wide-character fuzzy string compare */
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL fuzzy_wcscmp)(wchar_t const *__a, wchar_t const *__b);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_wcsncmp)(wchar_t const *__a, size_t __max_a_chars, wchar_t const *__b, size_t __max_b_chars);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL fuzzy_wcscasecmp)(wchar_t const *__a, wchar_t const *__b);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_wcsncasecmp)(wchar_t const *__a, size_t __max_a_chars, wchar_t const *__b, size_t __max_b_chars);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL fuzzy_wcscasecmp_l)(wchar_t const *__a, wchar_t const *__b, __locale_t __locale);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_wcsncasecmp_l)(wchar_t const *__a, size_t __max_a_chars, wchar_t const *__b, size_t __max_b_chars, __locale_t __locale);

/* wide-character wild string compare */
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wildwcscmp)(wchar_t const *__pattern, wchar_t const *__string);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wildwcscasecmp)(wchar_t const *__pattern, wchar_t const *__string);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wildwcscasecmp_l)(wchar_t const *__pattern, wchar_t const *__string, __locale_t __locale);

/* Misc. functions that weren't defined KOS Mk2 */
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) wchar_t *(__LIBCCALL wcscasestr_l)(wchar_t const *__haystack, wchar_t const *__needle, __locale_t __locale);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcsverscmp)(wchar_t const *__s1, wchar_t const *__s2);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsnrev)(wchar_t *__restrict __str, size_t __max_chars);

__SYSDECL_END
#endif /* __CC__ && !__KERNEL__ */

#endif /* !_PARTS_KOS3_WSTRING_H */
