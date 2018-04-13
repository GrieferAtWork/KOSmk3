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
#ifndef _PARTS_KOS3_STRING_H
#define _PARTS_KOS3_STRING_H 1

#include "__stdinc.h"
#include <features.h>
#ifndef _STRING_H
#include <string.h>
#endif /* !_STRING_H */

#if defined(__CC__) && !defined(__KERNEL__)
__SYSDECL_BEGIN

/* Locale/position-enabled formatted string duplication. */
__VREDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF(1,3) __PORT_KOSONLY __WUNUSED,char *,__ATTR_CDECL,strdupf_l,(char const *__restrict __format, __locale_t __locale, ...),vstrdupf_l,(__format,__locale),__locale)
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF(1,0) __PORT_KOSONLY __WUNUSED,char *,__LIBCCALL,vstrdupf_l,(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__VREDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF_P(1,2) __PORT_KOSONLY __WUNUSED,char *,__ATTR_CDECL,strdupf_p,(char const *__restrict __format, ...),vstrdupf_p,(__format),__format)
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF_P(1,0) __PORT_KOSONLY __WUNUSED,char *,__LIBCCALL,vstrdupf_p,(char const *__restrict __format, __builtin_va_list __args),(__format,__args))
__VREDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF_P(1,3) __PORT_KOSONLY __WUNUSED,char *,__ATTR_CDECL,strdupf_p_l,(char const *__restrict __format, __locale_t __locale, ...),vstrdupf_p_l,(__format,__locale),__locale)
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF_P(1,0) __PORT_KOSONLY __WUNUSED,char *,__LIBCCALL,vstrdupf_p_l,(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))

/* multi-byte fuzzy memory compare */
__REDIRECT(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_memcmpb,(void const *__a, size_t __a_bytes, void const *__b, size_t __b_bytes),fuzzy_memcmp,(__a,__a_bytes,__b,__b_bytes))
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcmpw)(void const *__a, size_t __a_words, void const *__b, size_t __b_words);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcmpl)(void const *__a, size_t __a_dwords, void const *__b, size_t __b_dwords);
__REDIRECT(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_memcasecmpb,(void const *__a, size_t __a_bytes, void const *__b, size_t __b_bytes),fuzzy_memcasecmp,(__a,__a_bytes,__b,__b_bytes))
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcasecmpw)(void const *__a, size_t __a_words, void const *__b, size_t __b_words);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcasecmpl)(void const *__a, size_t __a_dwords, void const *__b, size_t __b_dwords);
__REDIRECT(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_memcasecmpb_l,(void const *__a, size_t __a_bytes, void const *__b, size_t __b_bytes, __locale_t __locale),fuzzy_memcasecmp_l,(__a,__a_bytes,__b,__b_bytes,__locale))
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcasecmpw_l)(void const *__a, size_t __a_words, void const *__b, size_t __b_words, __locale_t __locale);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcasecmpl_l)(void const *__a, size_t __a_dwords, void const *__b, size_t __b_dwords, __locale_t __locale);

/* Misc. functions that weren't defined KOS Mk2 */
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) char *(__LIBCCALL strcasestr_l)(char const *__haystack, char const *__needle, __locale_t __locale);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnrev)(char *__restrict __str, size_t __max_chars);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memrev)(void *__restrict __base, size_t __num_bytes);
__REDIRECT(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,memrevb,(void *__restrict __base, size_t __num_bytes),memrev,(__base,__num_bytes))
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memrevw)(void *__restrict __base, size_t __num_words);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memrevl)(void *__restrict __base, size_t __num_dwords);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memrevq)(void *__restrict __base, size_t __num_qwords);

__SYSDECL_END
#endif /* __CC__ && !__KERNEL__ */

#ifdef _WCHAR_H
#ifndef _PARTS_KOS3_WSTRING_H
#include "wstring.h"
#endif
#endif


#endif /* !_PARTS_KOS3_STRING_H */
