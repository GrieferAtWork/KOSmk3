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

#ifdef __CC__
__SYSDECL_BEGIN

#ifndef __KERNEL__
#ifdef __CRT_KOS
/* Locale/position-enabled formatted string duplication. */
__VREDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF(1,3) __PORT_KOSONLY __WUNUSED,char *,__ATTR_CDECL,strdupf_l,(char const *__restrict __format, __locale_t __locale, ...),vstrdupf_l,(__format,__locale),__locale)
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF(1,0) __PORT_KOSONLY __WUNUSED,char *,__LIBCCALL,vstrdupf_l,(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__VREDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF_P(1,2) __PORT_KOSONLY __WUNUSED,char *,__ATTR_CDECL,strdupf_p,(char const *__restrict __format, ...),vstrdupf_p,(__format),__format)
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF_P(1,0) __PORT_KOSONLY __WUNUSED,char *,__LIBCCALL,vstrdupf_p,(char const *__restrict __format, __builtin_va_list __args),(__format,__args))
__VREDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF_P(1,3) __PORT_KOSONLY __WUNUSED,char *,__ATTR_CDECL,strdupf_p_l,(char const *__restrict __format, __locale_t __locale, ...),vstrdupf_p_l,(__format,__locale),__locale)
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF_P(1,0) __PORT_KOSONLY __WUNUSED,char *,__LIBCCALL,vstrdupf_p_l,(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
#ifdef __USE_EXCEPT
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF(1,3) __PORT_KOSONLY __WUNUSED char *(__ATTR_CDECL Xstrdupf_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF(1,0) __PORT_KOSONLY __WUNUSED char *(__LIBCCALL Xvstrdupf_l)(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF_P(1,2) __PORT_KOSONLY __WUNUSED char *(__ATTR_CDECL Xstrdupf_p)(char const *__restrict __format, ...);
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF_P(1,0) __PORT_KOSONLY __WUNUSED char *(__LIBCCALL Xvstrdupf_p)(char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF_P(1,3) __PORT_KOSONLY __WUNUSED char *(__ATTR_CDECL Xstrdupf_p_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __ATTR_LIBC_PRINTF_P(1,0) __PORT_KOSONLY __WUNUSED char *(__LIBCCALL Xvstrdupf_p_l)(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#endif /* __USE_EXCEPT */

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
#endif /* __CRT_KOS */
#endif /* __KERNEL__ */


#ifdef __ANY_COMPAT__
/* TODO: Emulate as inline functions */
#else /* __ANY_COMPAT__ */

/* TODO: These functions could be implemented using arch-specific inline assembly. */

/* Inverted (eXclusive) memory scanners.
 * `memxchr()' is the same as `memchr()', but rather than
 * searching for the first byte equal to `needle', search
 * to the first byte that isn't equal to `needle'
 * Mainly useful for validating memory when checking for use-after-free */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memxlen)(void const *__restrict __haystack, int __byte, size_t __num_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrxlen)(void const *__restrict __haystack, int __byte, size_t __num_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemxlen)(void const *__restrict __haystack, int __byte);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrxlen)(void const *__restrict __haystack, int __byte);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memxchr)(void const *__restrict __haystack, int __byte, size_t __num_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memrxchr)(void const *__restrict __haystack, int __byte, size_t __num_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memxend)(void const *__restrict __haystack, int __byte, size_t __num_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memrxend)(void const *__restrict __haystack, int __byte, size_t __num_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL rawmemxchr)(void const *__restrict __haystack, int __byte);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL rawmemrxchr)(void const *__restrict __haystack, int __byte);

__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,memxlenb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte, size_t __num_bytes),memxlen,(__haystack,__byte,__num_bytes))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memxlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memxlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memxlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords);
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,memrxlenb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte, size_t __num_bytes),memrxlen,(__haystack,__byte,__num_bytes))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrxlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrxlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrxlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords);
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,rawmemxlenb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte),rawmemxlen,(__haystack,__byte))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemxlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemxlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemxlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword);
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,rawmemrxlenb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte),rawmemrxlen,(__haystack,__byte))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrxlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrxlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrxlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword);

#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memxchrb,(__UINT8_TYPE__ *__restrict __haystack, int __byte, size_t __num_bytes),memxchr,(__haystack,__byte,__num_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,memxchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte, size_t __num_bytes),memxchr,(__haystack,__byte,__num_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memrxchrb,(__UINT8_TYPE__ *__restrict __haystack, int __byte, size_t __num_bytes),memrxchr,(__haystack,__byte,__num_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,memrxchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte, size_t __num_bytes),memrxchr,(__haystack,__byte,__num_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memxendb,(__UINT8_TYPE__ *__restrict __haystack, int __byte, size_t __num_bytes),memxend,(__haystack,__byte,__num_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,memxendb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte, size_t __num_bytes),memxend,(__haystack,__byte,__num_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memrxendb,(__UINT8_TYPE__ *__restrict __haystack, int __byte, size_t __num_bytes),memrxend,(__haystack,__byte,__num_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,memrxendb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte, size_t __num_bytes),memrxend,(__haystack,__byte,__num_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,rawmemxchrb,(__UINT8_TYPE__ *__restrict __haystack, int __byte),rawmemxchr,(__haystack,__byte))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,rawmemxchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte),rawmemxchr,(__haystack,__byte))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,rawmemrxchrb,(__UINT8_TYPE__ *__restrict __haystack, int __byte),rawmemrxchr,(__haystack,__byte))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,rawmemrxchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte),rawmemrxchr,(__haystack,__byte))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,memxchrw,(__UINT16_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words),memxchrw,(__haystack,__word,__num_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,memxchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words),memxchrw,(__haystack,__word,__num_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,memxchrl,(__UINT32_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords),memxchrl,(__haystack,__dword,__num_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,memxchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords),memxchrl,(__haystack,__dword,__num_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,memxchrq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords),memxchrq,(__haystack,__qword,__num_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,memxchrq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords),memxchrq,(__haystack,__qword,__num_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,memrxchrw,(__UINT16_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words),memrxchrw,(__haystack,__word,__num_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,memrxchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words),memrxchrw,(__haystack,__word,__num_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,memrxchrl,(__UINT32_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords),memrxchrl,(__haystack,__dword,__num_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,memrxchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords),memrxchrl,(__haystack,__dword,__num_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,memrxchrq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords),memrxchrq,(__haystack,__qword,__num_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,memrxchrq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords),memrxchrq,(__haystack,__qword,__num_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,memxendw,(__UINT16_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words),memxendw,(__haystack,__word,__num_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,memxendw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words),memxendw,(__haystack,__word,__num_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,memxendl,(__UINT32_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords),memxendl,(__haystack,__dword,__num_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,memxendl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords),memxendl,(__haystack,__dword,__num_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,memxendq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords),memxendq,(__haystack,__qword,__num_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,memxendq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords),memxendq,(__haystack,__qword,__num_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,memrxendw,(__UINT16_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words),memrxendw,(__haystack,__word,__num_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,memrxendw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words),memrxendw,(__haystack,__word,__num_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,memrxendl,(__UINT32_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords),memrxendl,(__haystack,__dword,__num_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,memrxendl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords),memrxendl,(__haystack,__dword,__num_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,memrxendq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords),memrxendq,(__haystack,__qword,__num_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,memrxendq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords),memrxendq,(__haystack,__qword,__num_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,rawmemxchrw,(__UINT16_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __word),rawmemxchrw,(__haystack,__word))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,rawmemxchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word),rawmemxchrw,(__haystack,__word))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,rawmemxchrl,(__UINT32_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __dword),rawmemxchrl,(__haystack,__dword))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,rawmemxchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword),rawmemxchrl,(__haystack,__dword))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,rawmemxchrq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __qword),rawmemxchrq,(__haystack,__qword))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,rawmemxchrq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword),rawmemxchrq,(__haystack,__qword))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,rawmemrxchrw,(__UINT16_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __word),rawmemrxchrw,(__haystack,__word))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,rawmemrxchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word),rawmemrxchrw,(__haystack,__word))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,rawmemrxchrl,(__UINT32_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __dword),rawmemrxchrl,(__haystack,__dword))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,rawmemrxchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword),rawmemrxchrl,(__haystack,__dword))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,rawmemrxchrq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __qword),rawmemrxchrq,(__haystack,__qword))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,rawmemrxchrq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword),rawmemrxchrq,(__haystack,__qword))
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memxchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte, size_t __num_bytes),memxchr,(__haystack,__byte,__num_bytes))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memxchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memxchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memxchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords);
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memrxchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte, size_t __num_bytes),memrxchr,(__haystack,__byte,__num_bytes))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrxchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrxchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrxchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords);
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memxendb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte, size_t __num_bytes),memxend,(__haystack,__byte,__num_bytes))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memxendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memxendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memxendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords);
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memrxendb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte, size_t __num_bytes),memrxend,(__haystack,__byte,__num_bytes))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrxendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word, size_t __num_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrxendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword, size_t __num_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrxendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword, size_t __num_qwords);
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,rawmemxchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte),rawmemxchr,(__haystack,__byte))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemxchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemxchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemxchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword);
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,rawmemrxchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __byte),rawmemrxchr,(__haystack,__byte))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemrxchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __word);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemrxchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __dword);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemrxchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __qword);
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */
#endif /* !__ANY_COMPAT__ */

__SYSDECL_END
#endif /* __CC__ */

#ifdef _WCHAR_H
#ifndef _PARTS_KOS3_WSTRING_H
#include "wstring.h"
#endif
#endif


#endif /* !_PARTS_KOS3_STRING_H */
