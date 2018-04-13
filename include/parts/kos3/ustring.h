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
#ifndef _PARTS_KOS3_USTRING_H
#define _PARTS_KOS3_USTRING_H 1

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
#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

/* UTF fuzzy string compare */
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,fuzzy_w16cmp,(char16_t const *__a, char16_t const *__b),fuzzy_wcscmp,(__a,__b))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,fuzzy_w32cmp,(char32_t const *__a, char32_t const *__b),fuzzy_wcscmp,(__a,__b))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_w16ncmp,(char16_t const *__a, size_t __max_a_chars, char16_t const *__b, size_t __max_b_chars),fuzzy_wcsncmp,(__a,__max_a_chars,__b,__max_b_chars))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_w32ncmp,(char32_t const *__a, size_t __max_a_chars, char32_t const *__b, size_t __max_b_chars),fuzzy_wcsncmp,(__a,__max_a_chars,__b,__max_b_chars))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,fuzzy_w16casecmp,(char16_t const *__a, char16_t const *__b),fuzzy_wcscasecmp,(__a,__b))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,fuzzy_w32casecmp,(char32_t const *__a, char32_t const *__b),fuzzy_wcscasecmp,(__a,__b))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_w16ncasecmp,(char16_t const *__a, size_t __max_a_chars, char16_t const *__b, size_t __max_b_chars),fuzzy_wcsncasecmp,(__a,__max_a_chars,__b,__max_b_chars))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_w32ncasecmp,(char32_t const *__a, size_t __max_a_chars, char32_t const *__b, size_t __max_b_chars),fuzzy_wcsncasecmp,(__a,__max_a_chars,__b,__max_b_chars))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,fuzzy_w16casecmp_l,(char16_t const *__a, char16_t const *__b, __locale_t __locale),fuzzy_wcscasecmp_l,(__a,__b,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,fuzzy_w32casecmp_l,(char32_t const *__a, char32_t const *__b, __locale_t __locale),fuzzy_wcscasecmp_l,(__a,__b,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_w16ncasecmp_l,(char16_t const *__a, size_t __max_a_chars, char16_t const *__b, size_t __max_b_chars, __locale_t __locale),fuzzy_wcsncasecmp_l,(__a,__max_a_chars,__b,__max_b_chars,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_w32ncasecmp_l,(char32_t const *__a, size_t __max_a_chars, char32_t const *__b, size_t __max_b_chars, __locale_t __locale),fuzzy_wcsncasecmp_l,(__a,__max_a_chars,__b,__max_b_chars,__locale))

/* UTF wild string compare */
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wildw16cmp,(char16_t const *__pattern, char16_t const *__string),wildwcscmp,(__pattern,__string))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wildw32cmp,(char32_t const *__pattern, char32_t const *__string),wildwcscmp,(__pattern,__string))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wildw16casecmp,(char16_t const *__pattern, char16_t const *__string),wildwcscasecmp,(__pattern,__string))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wildw32casecmp,(char32_t const *__pattern, char32_t const *__string),wildwcscasecmp,(__pattern,__string))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wildw16casecmp_l,(char16_t const *__pattern, char16_t const *__string, __locale_t __locale),wildwcscasecmp_l,(__pattern,__string,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wildw32casecmp_l,(char32_t const *__pattern, char32_t const *__string, __locale_t __locale),wildwcscasecmp_l,(__pattern,__string,__locale))

/* Misc. UTF functions that weren't defined KOS Mk2 */
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),char16_t *,__LIBCCALL,w16casestr_l,(char16_t const *__haystack, char16_t const *__needle, __locale_t __locale),wcscasestr_l,(__haystack,__needle,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),char32_t *,__LIBCCALL,w32casestr_l,(char32_t const *__haystack, char32_t const *__needle, __locale_t __locale),wcscasestr_l,(__haystack,__needle,__locale))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w16verscmp,(char16_t const *__s1, char16_t const *__s2),wcsverscmp,(__s1,__s2))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w32verscmp,(char32_t const *__s1, char32_t const *__s2),wcsverscmp,(__s1,__s2))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16nrev,(char16_t *__restrict __wcs, size_t __max_chars),wcsnrev,(__wcs,__max_chars))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32nrev,(char32_t *__restrict __wcs, size_t __max_chars),wcsnrev,(__wcs,__max_chars))

/* UTF to integer/float conversion. */

__SYSDECL_END
#endif /* __CC__ && !__KERNEL__ */

#endif /* !_PARTS_KOS3_USTRING_H */
