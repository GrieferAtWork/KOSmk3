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
#ifndef _WCHAR_H
#define _WCHAR_H 1

#include "__stdinc.h"
#include "parts/kos2/malldefs.h"
#include <features.h>
#include <bits/wchar.h>
#include <bits/mbstate.h>
#include <hybrid/typecore.h>
#include <hybrid/limitcore.h>
#include <bits/types.h>
#ifdef __DOS_COMPAT__
#include <hybrid/string.h>
#endif
#ifdef __USE_DOS
#include <bits/stat.h>
#include <bits/io-file.h>
#ifdef __OPTIMIZE_LIBC__
#include <asm-generic/string.h>
#endif /* __OPTIMIZE_LIBC__ */
#endif /* __USE_DOS */
#ifdef __CYG_COMPAT__
#include <sys/reent.h>
#endif /* __CYG_COMPAT__ */

#if defined(__USE_XOPEN2K8) || defined(__USE_GNU)
#   include <xlocale.h>
#endif /* __USE_XOPEN2K8 || __USE_GNU */
#if defined(__USE_XOPEN) && !defined(__USE_UNIX98)
#   include <wctype.h>
#elif (defined(__USE_UNIX98) && !defined(__USE_GNU)) || \
       defined(__USE_DOS)
#   include <bits/wctype.h>
#endif

__SYSDECL_BEGIN

/* Define 'FILE' */
#if !defined(__KERNEL__) && \
    (defined(__USE_UNIX98) || defined(__USE_XOPEN2K) || \
     defined(__USE_DOS))
#ifndef __std_FILE_defined
#define __std_FILE_defined 1
__NAMESPACE_STD_BEGIN
typedef __FILE FILE;
__NAMESPACE_STD_END
#endif /* !__std_FILE_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __FILE_defined
#define __FILE_defined 1
__NAMESPACE_STD_USING(FILE)
#endif /* !__FILE_defined */
#endif /* !__CXX_SYSTEM_HEADER */
#endif /* !__KERNEL__ && (__USE_UNIX98 || __USE_XOPEN2K) */

/* Define 'wchar_t' */
#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

/* Define `size_t' */
__NAMESPACE_STD_BEGIN
#ifndef __std_size_t_defined
#define __std_size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__std_size_t_defined */
#ifndef __std_wint_t_defined
#define __std_wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__std_wint_t_defined */
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
#ifndef __size_t_defined
#define __size_t_defined 1
__NAMESPACE_STD_USING(size_t)
#endif /* !__size_t_defined */
#ifndef __wint_t_defined
#define __wint_t_defined 1
__NAMESPACE_STD_USING(wint_t)
#endif /* !__wint_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */

/* Define `NULL' */
#ifndef NULL
#define NULL __NULLPTR
#endif

#ifndef WCHAR_MIN
#define WCHAR_MIN __WCHAR_MIN__
#define WCHAR_MAX __WCHAR_MAX__
#endif

#ifndef WEOF
#if __SIZEOF_WCHAR_T__ == 4
#define WEOF             0xffffffffu
#else
#define WEOF    (wint_t)(0xffff)
#endif
#endif

#ifdef __USE_KOS_STDEXT
#define __PRINTF_RETURN_TYPE __ssize_t
#else
#define __PRINTF_RETURN_TYPE int
#endif

__NAMESPACE_STD_BEGIN
struct tm;
#ifndef __KERNEL__
__LIBC __WUNUSED wint_t (__LIBCCALL btowc)(int __c);
__LIBC __WUNUSED int (__LIBCCALL wctob)(wint_t __c);
__LIBC __NONNULL((1,2,4)) size_t (__LIBCCALL mbrtowc)(wchar_t *__restrict __pwc, char const *__restrict __s, size_t __n, mbstate_t *__restrict __p);
__LIBC __NONNULL((1,3)) size_t (__LIBCCALL wcrtomb)(char *__restrict __s, wchar_t __wc, mbstate_t *__restrict __ps);
__REDIRECT(__LIBC,__WUNUSED __NONNULL((1,3)),size_t,__LIBCCALL,__mbrlen,(char const *__restrict __s, size_t __n, mbstate_t *__restrict __ps),mbrlen,(__s,__n,__ps))
__LIBC __WUNUSED __NONNULL((1,3)) size_t (__LIBCCALL mbrlen)(char const *__restrict __s, size_t __n, mbstate_t *__restrict __ps);
__LIBC __NONNULL((1,2)) size_t (__LIBCCALL mbsrtowcs)(wchar_t *__restrict __dst, char const **__restrict __psrc, size_t __len, mbstate_t *__restrict __ps);
__LIBC __NONNULL((1,2)) size_t (__LIBCCALL wcsrtombs)(char *__restrict __dst, wchar_t const **__restrict __psrc, size_t __len, mbstate_t *__restrict __ps);
__LIBC __NONNULL((1)) double (__LIBCCALL wcstod)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr);
__LIBC __NONNULL((1)) long int (__LIBCCALL wcstol)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
__LIBC __NONNULL((1)) unsigned long int (__LIBCCALL wcstoul)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
#ifdef __DOS_COMPAT__
__LOCAL __WUNUSED __ATTR_PURE int (__LIBCCALL mbsinit)(mbstate_t const *__ps) { return !__ps || !__ps->__val; }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wmemcmp)(wchar_t const *__s1, wchar_t const *__s2, size_t __n) { return __hybrid_memcmp(__s1,__s2,__n*sizeof(wchar_t)); }
__LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wmemcpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __n) { return (wchar_t *)__hybrid_memcpy(__dst,__src,__n*sizeof(wchar_t)); }
__LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wmemmove)(wchar_t *__dst, wchar_t const *__src, size_t __n) { return (wchar_t *)__hybrid_memmove(__dst,__src,__n*sizeof(wchar_t)); }
#if __SIZEOF_WCHAR_T__ == 4
__LOCAL __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wmemset)(wchar_t *__dst, wchar_t __filler, size_t __n) { return (wchar_t *)__libc_memsetl(__dst,(__UINT32_TYPE__)__filler,__n); }
#elif __SIZEOF_WCHAR_T__ == 2
__LOCAL __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wmemset)(wchar_t *__dst, wchar_t __filler, size_t __n) { return (wchar_t *)__libc_memsetw(__dst,(__UINT16_TYPE__)__filler,__n); }
#else
__LOCAL __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wmemset)(wchar_t *__dst, wchar_t __filler, size_t __n) { return (wchar_t *)__libc_memset(__dst,(int)__filler,__n*sizeof(wchar_t)); }
#endif
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED __ATTR_PURE int (__LIBCCALL mbsinit)(mbstate_t const *__ps);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wmemcmp)(wchar_t const *__s1, wchar_t const *__s2, size_t __n);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wmemcpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __n);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wmemmove)(wchar_t *__dst, wchar_t const *__src, size_t __n);
__LIBC __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wmemset)(wchar_t *__restrict __dst, wchar_t __filler, size_t __n);
#endif /* !__DOS_COMPAT__ */

#ifndef __std_wcscpy_defined
#define __std_wcscpy_defined 1
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcscpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcscat)(wchar_t *__restrict __dst, wchar_t const *__restrict __src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcsncat)(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __max_chars);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcsncpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __max_chars);
#endif /* !__std_wcscpy_defined */
#ifndef __std_wcscmp_defined
#define __std_wcscmp_defined 1
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcscmp)(wchar_t const *__s1, wchar_t const *__s2);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcsncmp)(wchar_t const *__s1, wchar_t const *__s2, size_t __n);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcscoll)(wchar_t const *__s1, wchar_t const *__s2);
__LIBC __NONNULL((2)) size_t (__LIBCCALL wcsxfrm)(wchar_t *__dst, wchar_t const *__restrict __src, size_t __n);
#endif /* !__std_wcscmp_defined */
#ifndef __std_getwchar_defined
#define __std_getwchar_defined 1
__LIBC wint_t (__LIBCCALL getwchar)(void);
__LIBC __NONNULL((1)) wint_t (__LIBCCALL fgetwc)(__FILE *__stream);
__LIBC __NONNULL((1)) wint_t (__LIBCCALL getwc)(__FILE *__stream);
#endif /* !__std_getwchar_defined */
#ifndef __std_putwchar_defined
#define __std_putwchar_defined 1
__LIBC wint_t (__LIBCCALL putwchar)(wchar_t __wc);
__LIBC __NONNULL((2)) wint_t (__LIBCCALL fputwc)(wchar_t __wc, __FILE *__stream);
__LIBC __NONNULL((2)) wint_t (__LIBCCALL putwc)(wchar_t __wc, __FILE *__stream);
#endif /* !__std_putwchar_defined */
#ifndef __std_fgetws_defined
#define __std_fgetws_defined 1
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__LIBC __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#if defined(__PE__) || !defined(__CRT_KOS) /* In PE-mode, we don't export the size_t version */
__REDIRECT(__LIBC,,wchar_t *,__LIBCCALL,__fgetws_int,(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream),fgetws,(__buf,__n,__stream))
__LOCAL __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) { return __fgetws_int(__buf,(int)__n,__stream); }
#else /* ... */
__REDIRECT(__LIBC,__NONNULL((1,3)),wchar_t *,__LIBCCALL,fgetws,(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream),fgetws_sz,(__buf,__n,__stream))
#endif /* !... */
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#else /* __USE_KOS */
__LIBC __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream);
#endif /* !__USE_KOS */
#endif /* !__std_fgetws_defined */
#ifndef __std_fputws_defined
#define __std_fputws_defined 1
__LIBC __NONNULL((1,2)) int (__LIBCCALL fputws)(wchar_t const *__restrict __str, __FILE *__restrict __stream);
#endif /* !__std_fputws_defined */
#ifndef __std_ungetwc_defined
#define __std_ungetwc_defined 1
__LIBC __NONNULL((2)) wint_t (__LIBCCALL ungetwc)(wint_t __wc, __FILE *__stream);
#endif /* !__std_ungetwc_defined */
#ifndef __std_wcsftime_defined
#define __std_wcsftime_defined 1
__LIBC __NONNULL((1,3,4)) size_t (__LIBCCALL wcsftime)(wchar_t *__restrict __s, size_t __maxsize, wchar_t const *__restrict __format, struct tm const *__restrict __tp);
#endif /* !__std_wcsftime_defined */
#ifndef __std_wcstok_defined
#define __std_wcstok_defined 1
#if defined(__USE_DOS) && !defined(__USE_ISOC95)
/* Define wcstok() incorrectly, the same way DOS does. */
#if defined(__CRT_DOS) && !defined(__GLC_COMPAT__) && !defined(__CYG_COMPAT__)
__LIBC __NONNULL((1,2)) wchar_t *(__LIBCCALL wcstok)(wchar_t *__restrict __s, wchar_t const *__restrict __delim);
#else /* __CRT_DOS... */
__REDIRECT(__LIBC,,wchar_t *,__LIBCCALL,__wcstok_impl,(wchar_t *__restrict __s, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr),wcstok,(__s,__delim,__ptr))
__INTERN __ATTR_WEAK __ATTR_UNUSED wchar_t *__wcstok_safe = 0;
__LOCAL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcstok)(wchar_t *__restrict __s, wchar_t const *__restrict __delim) { return __wcstok_impl(__s,__delim,&__wcstok_safe); }
#endif /* !__CRT_DOS... */
#elif defined(__CRT_DOS) && __SIZEOF_WCHAR_T__ == 2
__REDIRECT(__LIBC,__NONNULL((1,2,3)),wchar_t *,__LIBCCALL,wcstok,(wchar_t *__restrict __s, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr),wcstok_s,(__s,__delim,__ptr))
#else
__LIBC __NONNULL((1,2,3)) wchar_t *(__LIBCCALL wcstok)(wchar_t *__restrict __s, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr);
#endif
#endif /* !__std_wcstok_defined */
#ifndef __std_wcslen_defined
#define __std_wcslen_defined 1
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcslen)(wchar_t const *__restrict __str);
#endif /* !__std_wcslen_defined */
#ifndef __std_wcsspn_defined
#define __std_wcsspn_defined 1
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL wcsspn)(wchar_t const *__haystack, wchar_t const *__accept);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL wcscspn)(wchar_t const *__haystack, wchar_t const *__reject);
#endif /* !__std_wcsspn_defined */
#ifdef __CORRECT_ISO_CPP_WCHAR_H_PROTO
extern "C++" {
#ifndef __std_wcschr_defined
#define __std_wcschr_defined 1
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t *,__LIBCCALL,wcschr,(wchar_t *__restrict __str, wchar_t __needle),wcschr,(__str,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t *,__LIBCCALL,wcsrchr,(wchar_t *__restrict __str, wchar_t __needle),wcsrchr,(__str,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t const *,__LIBCCALL,wcschr,(wchar_t const *__restrict __str, wchar_t __needle),wcschr,(__str,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t const *,__LIBCCALL,wcsrchr,(wchar_t const *__restrict __str, wchar_t __needle),wcsrchr,(__str,__needle))
#endif /* !__std_wcschr_defined */
#ifndef __std_wcsstr_defined
#define __std_wcsstr_defined 1
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),wchar_t *,__LIBCCALL,wcspbrk,(wchar_t *__haystack, wchar_t const *__accept),wcspbrk,(__haystack,__accept))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),wchar_t const *,__LIBCCALL,wcspbrk,(wchar_t const *__haystack, wchar_t const *__accept),wcspbrk,(__haystack,__accept))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),wchar_t *,__LIBCCALL,wcsstr,(wchar_t *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),wchar_t const *,__LIBCCALL,wcsstr,(wchar_t const *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
#endif /* !__std_wcsstr_defined */
#ifdef __DOS_COMPAT__
#if __SIZEOF_WCHAR_T__ == 4
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wmemchr)(wchar_t *__restrict __haystack, wchar_t __needle, size_t __n) { return (wchar_t *)__libc_memchrl((__UINT32_TYPE__ *)__haystack,(__UINT32_TYPE__)__needle,__n); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t const *(__LIBCCALL wmemchr)(wchar_t const *__restrict __haystack, wchar_t __needle, size_t __n) { return (wchar_t *)__libc_memchrl((__UINT32_TYPE__ *)__haystack,(__UINT32_TYPE__)__needle,__n); }
#elif __SIZEOF_WCHAR_T__ == 2
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wmemchr)(wchar_t *__restrict __haystack, wchar_t __needle, size_t __n) { return (wchar_t *)__libc_memchrw((__UINT16_TYPE__ *)__haystack,(__UINT16_TYPE__)__needle,__n); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t const *(__LIBCCALL wmemchr)(wchar_t const *__restrict __haystack, wchar_t __needle, size_t __n) { return (wchar_t *)__libc_memchrw((__UINT16_TYPE__ *)__haystack,(__UINT16_TYPE__)__needle,__n); }
#else /* ... */
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wmemchr)(wchar_t *__restrict __haystack, wchar_t __needle, size_t __n) { return (wchar_t *)__libc_memchr(__haystack,(int)__needle,__n*sizeof(wchar_t)); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t const *(__LIBCCALL wmemchr)(wchar_t const *__restrict __haystack, wchar_t __needle, size_t __n) { return (wchar_t *)__libc_memchr(__haystack,(int)__needle,__n*sizeof(wchar_t)); }
#endif /* !... */
#else /* __DOS_COMPAT__ */
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE,wchar_t *,__LIBCCALL,wmemchr,(wchar_t *__haystack, wchar_t __needle, size_t __n),wmemchr,(__haystack,__needle,__n))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE,wchar_t const *,__LIBCCALL,wmemchr,(wchar_t const *__haystack, wchar_t __needle, size_t __n),wmemchr,(__haystack,__needle,__n))
#endif /* !__DOS_COMPAT__ */
}
#else /* __CORRECT_ISO_CPP_WCHAR_H_PROTO */
#ifndef __std_wcschr_defined
#define __std_wcschr_defined 1
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcschr)(wchar_t const *__restrict __str, wchar_t __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsrchr)(wchar_t const *__restrict __str, wchar_t __needle);
#endif /* !__wcschr_defined */
#ifndef __std_wcsstr_defined
#define __std_wcsstr_defined 1
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) wchar_t *(__LIBCCALL wcspbrk)(wchar_t const *__haystack, wchar_t const *__accept);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) wchar_t *(__LIBCCALL wcsstr)(wchar_t const *__haystack, wchar_t const *__needle);
#endif /* !__wcsstr_defined */
#ifdef __DOS_COMPAT__
#if __SIZEOF_WCHAR_T__ == 4
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wmemchr)(wchar_t const *__restrict __haystack, wchar_t __needle, size_t __n) { return (wchar_t *)__libc_memchrl((__UINT32_TYPE__ *)__haystack,(__UINT32_TYPE__)__needle,__n); }
#elif __SIZEOF_WCHAR_T__ == 2
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wmemchr)(wchar_t const *__restrict __haystack, wchar_t __needle, size_t __n) { return (wchar_t *)__libc_memchrw((__UINT16_TYPE__ *)__haystack,(__UINT16_TYPE__)__needle,__n); }
#else /* ... */
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wmemchr)(wchar_t const *__restrict __haystack, wchar_t __needle, size_t __n) { return (wchar_t *)__libc_memchr(__haystack,(int)__needle,__n*sizeof(wchar_t)); }
#endif /* !... */
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wmemchr)(wchar_t const *__restrict __haystack, wchar_t __needle, size_t __n);
#endif /* !__DOS_COMPAT__ */
#endif /* !__CORRECT_ISO_CPP_WCHAR_H_PROTO */
#if defined(__USE_ISOC95) || defined(__USE_UNIX98)
#if defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__LIBC __NONNULL((1)) int (__LIBCCALL fwide)(__FILE *__fp, int __mode);
#else /* __CRT_GLC && !__DOS_COMPAT__ */
__LOCAL __NONNULL((1)) int (__LIBCCALL fwide)(__FILE *__UNUSED(__fp), int __UNUSED(__mode)) { return 0; }
#endif /* !__CRT_GLC || __DOS_COMPAT__ */
#endif /* __USE_ISOC95 || __USE_UNIX98 */
#if defined(__USE_ISOC95) || defined(__USE_UNIX98) || defined(__USE_DOS)
#ifndef __std_wprintf_defined
#define __std_wprintf_defined 1
__LIBC __ATTR_LIBC_WPRINTF(2,3) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__ATTR_CDECL fwprintf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC __ATTR_LIBC_WPRINTF(2,0) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__LIBCCALL vfwprintf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __ATTR_LIBC_WPRINTF(1,2) __NONNULL((1)) __PRINTF_RETURN_TYPE (__ATTR_CDECL wprintf)(wchar_t const *__restrict __format, ...);
__LIBC __ATTR_LIBC_WPRINTF(1,0) __NONNULL((1)) __PRINTF_RETURN_TYPE (__LIBCCALL vwprintf)(wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __ATTR_LIBC_WSCANF(2,3) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__ATTR_CDECL fwscanf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC __ATTR_LIBC_WSCANF(1,2) __NONNULL((1)) __PRINTF_RETURN_TYPE (__ATTR_CDECL wscanf)(wchar_t const *__restrict __format, ...);
__LIBC __ATTR_LIBC_WSCANF(2,3) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__ATTR_CDECL swscanf)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...);
#if defined(__DOS_COMPAT__) || defined(__PE__)
__REDIRECT(__LIBC,__ATTR_LIBC_WPRINTF(3,0) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__LIBCCALL,vswprintf,(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args),_vswprintf_c,(__buf,__buflen,__format,__args))
__VREDIRECT(__LIBC,__ATTR_LIBC_WPRINTF(3,4) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,swprintf,(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...),_vswprintf_c,(__buf,__buflen,__format),__format)
#else
__LIBC __ATTR_LIBC_WPRINTF(3,0) __NONNULL((1,3)) __PRINTF_RETURN_TYPE (__LIBCCALL vswprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_WPRINTF(3,4) __NONNULL((1,3)) __PRINTF_RETURN_TYPE (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
#endif
#endif /* !__std_wprintf_defined */
#endif /* __USE_ISOC95 || __USE_UNIX98 || __USE_DOS */
#ifdef __USE_ISOC99
__LIBC __NONNULL((1)) float (__LIBCCALL wcstof)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr);
__LIBC __NONNULL((1)) long double (__LIBCCALL wcstold)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr);
#if (defined(__PE__) || defined(__DOS_COMPAT__)) && __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,__NONNULL((1)),__LONGLONG,__LIBCCALL,wcstoll,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),_wcstoi64,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__ULONGLONG,__LIBCCALL,wcstoull,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),_wcstoui64,(__nptr,__endptr,__base))
#elif __SIZEOF_LONG__ == __SIZEOF_LONG_LONG__
__REDIRECT(__LIBC,__NONNULL((1)),__LONGLONG,__LIBCCALL,wcstoll,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__ULONGLONG,__LIBCCALL,wcstoull,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),wcstoul,(__nptr,__endptr,__base))
#else
__LIBC __NONNULL((1)) __LONGLONG (__LIBCCALL wcstoll)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
__LIBC __NONNULL((1)) __ULONGLONG (__LIBCCALL wcstoull)(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base);
#endif
#ifndef __std_vwscanf_defined
#define __std_vwscanf_defined 1
__LIBC __WUNUSED __ATTR_LIBC_WSCANF(2,0) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__LIBCCALL vfwscanf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __WUNUSED __ATTR_LIBC_WSCANF(1,0) __NONNULL((1)) __PRINTF_RETURN_TYPE (__LIBCCALL vwscanf)(wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __WUNUSED __ATTR_LIBC_WSCANF(2,0) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__LIBCCALL vswscanf)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __builtin_va_list __arg);
#endif /* !__std_vwscanf_defined */
#endif /* __USE_ISOC99 */
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END
#undef __PRINTF_RETURN_TYPE


#ifndef __CXX_SYSTEM_HEADER
#ifndef __tm_defined
#define __tm_defined 1
__NAMESPACE_STD_USING(tm)
#endif /* !__tm_defined */
#ifndef __KERNEL__
__NAMESPACE_STD_USING(wcscoll)
__NAMESPACE_STD_USING(wcsxfrm)
__NAMESPACE_STD_USING(btowc)
__NAMESPACE_STD_USING(wctob)
__NAMESPACE_STD_USING(mbsinit)
__NAMESPACE_STD_USING(mbrtowc)
__NAMESPACE_STD_USING(wcrtomb)
__NAMESPACE_STD_USING(__mbrlen)
__NAMESPACE_STD_USING(mbrlen)
__NAMESPACE_STD_USING(mbsrtowcs)
__NAMESPACE_STD_USING(wcsrtombs)
__NAMESPACE_STD_USING(wmemcmp)
__NAMESPACE_STD_USING(wmemcpy)
__NAMESPACE_STD_USING(wmemmove)
__NAMESPACE_STD_USING(wmemset)
__NAMESPACE_STD_USING(wmemchr)
__NAMESPACE_STD_USING(wcstoll)
__NAMESPACE_STD_USING(wcstoull)
__NAMESPACE_STD_USING(wcstod)
#ifndef __wcscpy_defined
#define __wcscpy_defined 1
__NAMESPACE_STD_USING(wcscpy)
__NAMESPACE_STD_USING(wcsncpy)
__NAMESPACE_STD_USING(wcscat)
__NAMESPACE_STD_USING(wcsncat)
#endif /* !__wcscpy_defined */
#ifndef __wcscmp_defined
#define __wcscmp_defined 1
__NAMESPACE_STD_USING(wcscmp)
__NAMESPACE_STD_USING(wcsncmp)
#endif /* !__wcscmp_defined */
#ifndef __wcstol_defined
#define __wcstol_defined 1
__NAMESPACE_STD_USING(wcstol)
__NAMESPACE_STD_USING(wcstoul)
#endif /* !__wcstol_defined */
#ifndef __getwchar_defined
#define __getwchar_defined 1
__NAMESPACE_STD_USING(getwchar)
__NAMESPACE_STD_USING(fgetwc)
__NAMESPACE_STD_USING(getwc)
#endif /* !__getwchar_defined */
#ifndef __putwchar_defined
#define __putwchar_defined 1
__NAMESPACE_STD_USING(putwchar)
__NAMESPACE_STD_USING(fputwc)
__NAMESPACE_STD_USING(putwc)
#endif /* !__putwchar_defined */
#ifndef __fgetws_defined
#define __fgetws_defined 1
__NAMESPACE_STD_USING(fgetws)
#endif /* !__fgetws_defined */
#ifndef __fputws_defined
#define __fputws_defined 1
__NAMESPACE_STD_USING(fputws)
#endif /* !__fputws_defined */
#ifndef __ungetwc_defined
#define __ungetwc_defined 1
__NAMESPACE_STD_USING(ungetwc)
#endif /* !__ungetwc_defined */
#ifndef __wcsftime_defined
#define __wcsftime_defined 1
__NAMESPACE_STD_USING(wcsftime)
#endif /* !__wcsftime_defined */
#ifndef __wcstok_defined
#define __wcstok_defined 1
__NAMESPACE_STD_USING(wcstok)
#endif /* !__wcstok_defined */
#ifndef __wcslen_defined
#define __wcslen_defined 1
__NAMESPACE_STD_USING(wcslen)
#endif /* !__wcslen_defined */
#ifndef __wcsspn_defined
#define __wcsspn_defined 1
__NAMESPACE_STD_USING(wcsspn)
__NAMESPACE_STD_USING(wcscspn)
#endif /* !__wcsspn_defined */
#ifndef __wcschr_defined
#define __wcschr_defined 1
__NAMESPACE_STD_USING(wcschr)
__NAMESPACE_STD_USING(wcsrchr)
#endif /* !__wcschr_defined */
#ifndef __wcsstr_defined
#define __wcsstr_defined 1
__NAMESPACE_STD_USING(wcsstr)
__NAMESPACE_STD_USING(wcspbrk)
#endif /* !__wcsstr_defined */
#if defined(__USE_ISOC95) || defined(__USE_UNIX98)
__NAMESPACE_STD_USING(fwide)
#endif /* __USE_ISOC95 || __USE_UNIX98 */
#if defined(__USE_ISOC95) || defined(__USE_UNIX98) || defined(__USE_DOS)
#ifndef __wprintf_defined
#define __wprintf_defined 1
__NAMESPACE_STD_USING(fwprintf)
__NAMESPACE_STD_USING(wprintf)
__NAMESPACE_STD_USING(swprintf)
__NAMESPACE_STD_USING(vfwprintf)
__NAMESPACE_STD_USING(vwprintf)
__NAMESPACE_STD_USING(vswprintf)
__NAMESPACE_STD_USING(fwscanf)
__NAMESPACE_STD_USING(wscanf)
__NAMESPACE_STD_USING(swscanf)
#endif /* !__wprintf_defined */
#endif /* __USE_ISOC95 || __USE_UNIX98 || __USE_DOS */
#ifdef __USE_ISOC99
#ifndef __wcstof_defined
#define __wcstof_defined 1
__NAMESPACE_STD_USING(wcstof)
__NAMESPACE_STD_USING(wcstold)
#endif /* !__wcstof_defined */
#ifndef __vwscanf_defined
#define __vwscanf_defined 1
__NAMESPACE_STD_USING(vfwscanf)
__NAMESPACE_STD_USING(vwscanf)
__NAMESPACE_STD_USING(vswscanf)
#endif /* !__vwscanf_defined */
#endif /* __USE_ISOC99 */
#endif /* !__KERNEL__ */
#endif /* !__CXX_SYSTEM_HEADER */

#ifndef __KERNEL__
#ifdef __USE_XOPEN2K8
#ifndef __size_t_defined
#define __size_t_defined 1
__NAMESPACE_STD_USING(size_t)
#endif /* !__size_t_defined */
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcscasecmp,(wchar_t const *__s1, wchar_t const *__s2),_wcsicmp,(__s1,__s2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsncasecmp,(wchar_t const *__s1, wchar_t const *__s2, size_t __n),_wcsnicmp,(__s1,__s2,__n))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcscasecmp_l,(wchar_t const *__s1, wchar_t const *__s2, __locale_t __locale),_wcsicmp_l,(__s1,__s2,__locale))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsncasecmp_l,(wchar_t const *__s1, wchar_t const *__s2, size_t __n, __locale_t __locale),_wcsnicmp_l,(__s1,__s2,__n,__locale))
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcscasecmp)(wchar_t const *__s1, wchar_t const *__s2);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcsncasecmp)(wchar_t const *__s1, wchar_t const *__s2, size_t __n);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcscasecmp_l)(wchar_t const *__s1, wchar_t const *__s2, __locale_t __locale);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcsncasecmp_l)(wchar_t const *__s1, wchar_t const *__s2, size_t __n, __locale_t __locale);
#endif /* !__DOS_COMPAT__ */
__REDIRECT_DPA(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcscoll_l,(wchar_t const *__s1, wchar_t const *__s2, __locale_t __locale),(__s1,__s2,__locale))
__REDIRECT_DPA(__LIBC,__NONNULL((2)),size_t,__LIBCCALL,wcsxfrm_l,(wchar_t *__dst, wchar_t const *__restrict __src, size_t __n, __locale_t __locale),(__dst,__src,__n,__locale))
#ifdef __CRT_GLC
#ifndef __mbstate_t_defined
#define __mbstate_t_defined 1
__NAMESPACE_STD_USING(mbstate_t)
#endif /* !__mbstate_t_defined */
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcpcpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcpncpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __n);
/* TODO: Check if there really isn't any way of emulate the following two (because they're _very_ useful...) */
__LIBC __PORT_NODOS __NONNULL((1,2,5)) size_t (__LIBCCALL mbsnrtowcs)(wchar_t *__restrict __dst, char const **__restrict __psrc, size_t __nmc, size_t __len, mbstate_t *__restrict __ps);
__LIBC __PORT_NODOS __NONNULL((1,2,5)) size_t (__LIBCCALL wcsnrtombs)(char *__restrict __dst, wchar_t const **__restrict __psrc, size_t __nwc, size_t __len, mbstate_t *__restrict __ps);
__LIBC __PORT_NODOS __FILE *(__LIBCCALL open_wmemstream)(wchar_t **__bufloc, size_t *__sizeloc);
#else /* __CRT_GLC */
__LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcpcpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src) { return __NAMESPACE_STD_SYM wcscpy(__dst,__src)+__NAMESPACE_STD_SYM wcslen(__dst); }
__LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcpncpy)(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __n) { return __NAMESPACE_STD_SYM wcsncpy(__dst,__src,__n)+__NAMESPACE_STD_SYM wcslen(__dst); }
#endif /* !__CRT_GLC */
#endif /* __USE_XOPEN2K8 */

#if defined(__USE_XOPEN2K8) || defined(__USE_DOS)
#ifndef __wcsnlen_defined
#define __wcsnlen_defined 1
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcsnlen)(wchar_t const *__restrict __src, size_t __max_chars);
#endif /* !__wcsnlen_defined */
__REDIRECT_DPA(__LIBC,__WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC __NONNULL((1)),wchar_t *,__LIBCCALL,wcsdup,(wchar_t const *__restrict __str),(__str))
#endif /* __USE_XOPEN2K8 || __USE_DOS */

#ifdef __USE_XOPEN
#ifdef __CRT_GLC
#if defined(__USE_KOS) && (__SIZEOF_SIZE_T__ <= __SIZEOF_INT__ || defined(__CRT_KOS))
__LIBC __PORT_NODOS __WUNUSED __ssize_t (__LIBCCALL wcwidth)(wchar_t __c);
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1)) __ssize_t (__LIBCCALL wcswidth)(wchar_t const *__restrict __s, size_t __n);
#else /* __USE_KOS */
__LIBC __PORT_NODOS __WUNUSED int (__LIBCCALL wcwidth)(wchar_t __c);
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1)) int (__LIBCCALL wcswidth)(wchar_t const *__restrict __s, size_t __n);
#endif /* !__USE_KOS */
#endif /* __CRT_GLC */
#endif /* __USE_XOPEN */

#if defined(__USE_XOPEN) || defined(__USE_DOS)
#ifndef __wcswcs_defined
#define __wcswcs_defined 1
#ifdef __CORRECT_ISO_CPP_WCHAR_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __NONNULL((1,2)),wchar_t *,__LIBCCALL,wcswcs,(wchar_t *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __NONNULL((1,2)),wchar_t const *,__LIBCCALL,wcswcs,(wchar_t const *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
}
#else /* __CORRECT_ISO_CPP_WCHAR_H_PROTO */
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __NONNULL((1,2)),wchar_t *,__LIBCCALL,wcswcs,(wchar_t const *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
#endif /* !__CORRECT_ISO_CPP_WCHAR_H_PROTO */
#endif /* !__wcswcs_defined */
#endif /* __USE_XOPEN || __USE_DOS */

#ifdef __USE_GNU
#ifdef __CORRECT_ISO_CPP_WCHAR_H_PROTO
extern "C++" {
#if defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __NONNULL((1)),wchar_t *,__LIBCCALL,wcschrnul,(wchar_t *__haystack, wchar_t __needle),wcschrnul,(__haystack,__needle))
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __NONNULL((1)),wchar_t const *,__LIBCCALL,wcschrnul,(wchar_t const *__haystack, wchar_t __needle),wcschrnul,(__haystack,__needle))
#else /* __CRT_GLC */
__LOCAL __ATTR_PURE __WUNUSED __NONNULL((1)) wchar_t *(__LIBCCALL wcschrnul)(wchar_t *__haystack, wchar_t __needle) { wchar_t *__iter = __haystack; for (; *__iter && *__iter != __needle; ++__iter); return __iter; }
__LOCAL __ATTR_PURE __WUNUSED __NONNULL((1)) wchar_t const *(__LIBCCALL wcschrnul)(wchar_t const *__haystack, wchar_t __needle) { wchar_t const *__iter = __haystack; for (; *__iter && *__iter != __needle; ++__iter); return __iter; }
#endif /* !__CRT_GLC */
}
#else /* __CORRECT_ISO_CPP_WCHAR_H_PROTO */
#if defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) wchar_t *(__LIBCCALL wcschrnul)(wchar_t const *__haystack, wchar_t __needle);
#else /* __CRT_GLC */
__LOCAL __ATTR_PURE __WUNUSED __NONNULL((1)) wchar_t *(__LIBCCALL wcschrnul)(wchar_t const *__haystack, wchar_t __needle) { wchar_t *__iter = (wchar_t *)__haystack; for (; *__iter && *__iter != __needle; ++__iter); return __iter; }
#endif /* !__CRT_GLC */
#endif /* !__CORRECT_ISO_CPP_WCHAR_H_PROTO */
#if defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wmempcpy)(wchar_t *__restrict __s1, wchar_t const *__restrict __s2, size_t __n);
#else /* __CRT_GLC */
__LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wmempcpy)(wchar_t *__restrict __s1, wchar_t const *__restrict __s2, size_t __n) { return __NAMESPACE_STD_SYM wmemcpy(__s1,__s2,__n)+__n; }
#endif /* !__CRT_GLC */
__REDIRECT_DPA(__LIBC,__NONNULL((1)),long int,__LIBCCALL,wcstol_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),(__nptr,__endptr,__base,__locale))
__REDIRECT_DPA(__LIBC,__NONNULL((1)),unsigned long int,__LIBCCALL,wcstoul_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),(__nptr,__endptr,__base,__locale))
#if defined(__CRT_DOS) && __SIZEOF_LONG_LONG__ == 8 && \
  ((defined(__PE__) && !defined(__GLC_COMPAT__)) || defined(__DOS_COMPAT__))
__REDIRECT(__LIBC,__NONNULL((1)),__LONGLONG,__LIBCCALL,wcstoq,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),_wcstoi64,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__ULONGLONG,__LIBCCALL,wcstouq,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),_wcstoui64,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__LONGLONG,__LIBCCALL,wcstoll_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),_wcstoi64_l,(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,__NONNULL((1)),__ULONGLONG,__LIBCCALL,wcstoull_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),_wcstoui64_l,(__nptr,__endptr,__base,__locale))
#elif __SIZEOF_LONG__ == __SIZEOF_LONG_LONG__
__REDIRECT(__LIBC,__NONNULL((1)),__LONGLONG,__LIBCCALL,wcstoq,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),wcstol,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__ULONGLONG,__LIBCCALL,wcstouq,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),wcstoul,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__LONGLONG,__LIBCCALL,wcstoll_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstol_l),(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,__NONNULL((1)),__ULONGLONG,__LIBCCALL,wcstoull_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstoul_l),(__nptr,__endptr,__base,__locale))
#else
__REDIRECT(__LIBC,__NONNULL((1)),__LONGLONG,__LIBCCALL,wcstoq,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),__SYMNAME_DOSPREFIX(wcstoll_l),(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__ULONGLONG,__LIBCCALL,wcstouq,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base),__SYMNAME_DOSPREFIX(wcstoull_l),(__nptr,__endptr,__base))
__REDIRECT_DPA(__LIBC,__NONNULL((1)),__LONGLONG,__LIBCCALL,wcstoll_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),(__nptr,__endptr,__base,__locale))
__REDIRECT_DPA(__LIBC,__NONNULL((1)),__ULONGLONG,__LIBCCALL,wcstoull_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, int __base, __locale_t __locale),(__nptr,__endptr,__base,__locale))
#endif
#ifndef __wint_t_defined
#define __wint_t_defined 1
__NAMESPACE_STD_USING(wint_t)
#endif /* !__wint_t_defined */
__REDIRECT_DPA(__LIBC,__NONNULL((1)),float,__LIBCCALL,wcstof_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, __locale_t __locale),(__nptr,__endptr,__locale))
__REDIRECT_DPA(__LIBC,__NONNULL((1)),double,__LIBCCALL,wcstod_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, __locale_t __locale),(__nptr,__endptr,__locale))
__REDIRECT_DPA(__LIBC,__NONNULL((1)),long double,__LIBCCALL,wcstold_l,(wchar_t const *__restrict __nptr, wchar_t **__restrict __endptr, __locale_t __locale),(__nptr,__endptr,__locale))
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,,wint_t,__LIBCCALL,getwchar_unlocked,(void),_getwchar_nolock,())
__REDIRECT(__LIBC,__NONNULL((1)),wint_t,__LIBCCALL,getwc_unlocked,(__FILE *__stream),_fgetwc_nolock,(__stream))
__REDIRECT(__LIBC,__NONNULL((2)),wint_t,__LIBCCALL,putwc_unlocked,(wchar_t __wc, __FILE *__stream),_fputwc_nolock,(__wc,__stream))
__REDIRECT(__LIBC,__NONNULL((1)),wint_t,__LIBCCALL,fgetwc_unlocked,(__FILE *__stream),_fgetwc_nolock,(__stream))
__REDIRECT(__LIBC,,wint_t,__LIBCCALL,putwchar_unlocked,(wchar_t __wc),_putwchar_nolock,(__wc))
__REDIRECT(__LIBC,__NONNULL((2)),wint_t,__LIBCCALL,fputwc_unlocked,(wchar_t __wc, __FILE *__stream),_fputwc_nolock,(__wc,__stream))
#else /* __DOS_COMPAT__ */
__REDIRECT(__LIBC,__NONNULL((1)),wint_t,__LIBCCALL,getwc_unlocked,(__FILE *__stream),fgetwc_unlocked,(__stream))
__REDIRECT(__LIBC,__NONNULL((2)),wint_t,__LIBCCALL,putwc_unlocked,(wchar_t __wc, __FILE *__stream),fputwc_unlocked,(__wc,__stream))
__LIBC wint_t (__LIBCCALL getwchar_unlocked)(void);
__LIBC __NONNULL((1)) wint_t (__LIBCCALL fgetwc_unlocked)(__FILE *__stream);
__LIBC wint_t (__LIBCCALL putwchar_unlocked)(wchar_t __wc);
__LIBC __NONNULL((2)) wint_t (__LIBCCALL fputwc_unlocked)(wchar_t __wc, __FILE *__stream);
#endif /* !__DOS_COMPAT__ */
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__NONNULL((1,3)),wchar_t *,__LIBCCALL,fgetws_unlocked,(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream),_fgetws_nolock,(__buf,__n,__stream))
#else
__LIBC __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws_unlocked)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream);
#endif
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#if defined(__CRT_KOS) && !defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__)
__REDIRECT(__LIBC,__NONNULL((1,3)),wchar_t *,__LIBCCALL,fgetws_unlocked,(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream),fgetws_unlocked_sz,(__buf,__n,__stream))
#else /* Builtin... */
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__NONNULL((1,3)),wchar_t *,__LIBCCALL,__fgetws_int_unlocked,(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream),_fgetws_nolock,(__buf,__n,__stream))
#else
__REDIRECT(__LIBC,__NONNULL((1,3)),wchar_t *,__LIBCCALL,__fgetws_int_unlocked,(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream),fgetws_unlocked,(__buf,__n,__stream))
#endif
__LOCAL __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws_unlocked)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) { return __fgetws_int_unlocked(__buf,(int)__n,__stream); }
#endif /* Compat... */
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#else /* __USE_KOS */
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__NONNULL((1,3)),wchar_t *,__LIBCCALL,fgetws_unlocked,(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream),_fgetws_nolock,(__buf,__n,__stream))
#else
__LIBC __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws_unlocked)(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream);
#endif
#endif /* !__USE_KOS */
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__NONNULL((1,2)),int,__LIBCCALL,fputws_unlocked,(wchar_t const *__restrict __str, __FILE *__restrict __stream),_fputws_nolock,(__str,__stream))
#else
__LIBC __NONNULL((1,2)) int (__LIBCCALL fputws_unlocked)(wchar_t const *__restrict __str, __FILE *__restrict __stream);
#endif
#ifndef __tm_defined
#define __tm_defined 1
__NAMESPACE_STD_USING(tm)
#endif /* !__tm_defined */
__REDIRECT_DPA(__LIBC,__NONNULL((1,3,4)),size_t,__LIBCCALL,wcsftime_l,(wchar_t *__restrict __buf, size_t __maxsize, wchar_t const *__restrict __format, struct tm const *__restrict __tp, __locale_t __locale),(__buf,__maxsize,__format,__tp,__locale))
#endif /* __USE_GNU */
#endif /* !__KERNEL__ */

__SYSDECL_END

#ifdef __USE_KOS
#ifdef _STRING_H
#ifndef _PARTS_KOS2_WSTRING_H
#include "parts/kos2/wstring.h"
#endif
#endif
#ifdef _STDLIB_H
#ifndef _PARTS_KOS2_WSTDLIB_H
#include "parts/kos2/wstdlib.h"
#endif
#endif
#ifdef _PROCESS_H
#ifndef _PARTS_KOS2_WPROCESS_H
#include "parts/kos2/wprocess.h"
#endif
#endif
#ifdef _FCNTL_H
#ifndef _PARTS_KOS2_WFCNTL_H
#include "parts/kos2/wfcntl.h"
#endif
#endif
#ifdef _UNISTD_H
#ifndef _PARTS_KOS2_WUNISTD_H
#include "parts/kos2/wunistd.h"
#endif
#endif
#ifdef _FORMAT_PRINTER_H
#ifndef _PARTS_KOS2_WFORMATPRINTER_H
#include "parts/kos2/wformatprinter.h"
#endif
#endif
#endif /* __USE_KOS */

#ifdef __USE_KOS3
#ifdef _STDIO_H
#ifndef _PARTS_KOS3_WSTDIO_H
#include "parts/kos3/wstdio.h"
#endif
#endif
#ifdef _STRING_H
#ifndef _PARTS_KOS3_WSTRING_H
#include "parts/kos3/wstring.h"
#endif
#endif
#ifdef _UNISTD_H
#ifndef _PARTS_KOS3_WUNISTD_H
#include "parts/kos3/wunistd.h"
#endif
#endif
#ifdef _STDLIB_H
#ifndef _PARTS_KOS3_WSTDLIB_H
#include "parts/kos3/wstdlib.h"
#endif
#endif
#ifdef _PROCESS_H
#ifndef _PARTS_KOS3_WPROCESS_H
#include "parts/kos3/wprocess.h"
#endif
#endif
#ifdef _FORMAT_PRINTER_H
#ifndef _PARTS_KOS3_WFORMATPRINTER_H
#include "parts/kos3/wformatprinter.h"
#endif
#endif
#ifdef _SYS_STAT_H
#ifndef _PARTS_KOS3_SYS_WSTAT_H
#include "parts/kos3/sys/wstat.h"
#endif
#endif
#ifdef _SYS_MMAN_H
#ifndef _PARTS_KOS3_SYS_WMMAN_H
#include "parts/kos3/sys/wmman.h"
#endif
#endif
#endif /* __USE_KOS3 */

#ifdef __USE_DOS
#ifndef _PARTS_DOS_WCHAR_H
#include "parts/dos/wchar.h"
#endif
#endif /* __USE_DOS */

#ifdef __DOCGEN__
#include "parts/dos/wchar.h"
#include "parts/kos2/wfcntl.h"
#include "parts/kos2/wformatprinter.h"
#include "parts/kos2/wprocess.h"
#include "parts/kos2/wstdlib.h"
#include "parts/kos2/wstring.h"
#include "parts/kos2/wunistd.h"
#include "parts/kos3/sys/wstat.h"
#include "parts/kos3/sys/wmman.h"
#include "parts/kos3/wformatprinter.h"
#include "parts/kos3/wprocess.h"
#include "parts/kos3/wstdio.h"
#include "parts/kos3/wstdlib.h"
#include "parts/kos3/wstring.h"
#include "parts/kos3/wunistd.h"
#endif

#endif /* !_WCHAR_H */
