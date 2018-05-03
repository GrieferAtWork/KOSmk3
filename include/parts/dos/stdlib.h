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
#ifndef _PARTS_DOS_STDLIB_H
#define _PARTS_DOS_STDLIB_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/byteswap.h>
#ifndef _STDLIB_H
#include <stdlib.h>
#endif

__SYSDECL_BEGIN

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef _ONEXIT_T_DEFINED
#define _ONEXIT_T_DEFINED 1
typedef int (__LIBCCALL *_onexit_t)(void);
#define onexit_t         _onexit_t
#endif  /* _ONEXIT_T_DEFINED */

#if defined(__DCC_VERSION__) || \
   (__has_builtin(__builtin_min) && __has_builtin(__builtin_max))
#   define __min(a,b) __builtin_min(a,b)
#   define __max(a,b) __builtin_max(a,b)
#elif defined(__COMPILER_HAVE_TYPEOF) && !defined(__NO_XBLOCK)
#   define __min(a,b) __XBLOCK({ __typeof__(a) _a = (a),_b = (b); __XRETURN _a < _b ? _a : _b; })
#   define __max(a,b) __XBLOCK({ __typeof__(a) _a = (a),_b = (b); __XRETURN _b < _a ? _a : _b; })
#else
#   define __min(a,b) ((a) < (b) ? (a) : (b))
#   define __max(a,b) ((b) < (a) ? (a) : (b))
#endif

#define _MAX_PATH         __DOS_MAX_PATH
#define _MAX_DRIVE        __DOS_MAX_DRIVE
#define _MAX_DIR          __DOS_MAX_DIR
#define _MAX_FNAME        __DOS_MAX_FNAME
#define _MAX_EXT          __DOS_MAX_EXT
#define _OUT_TO_DEFAULT   __DOS_OUT_TO_DEFAULT
#define _OUT_TO_STDERR    __DOS_OUT_TO_STDERR
#define _OUT_TO_MSGBOX    __DOS_OUT_TO_MSGBOX
#define _REPORT_ERRMODE   __DOS_REPORT_ERRMODE
#define _WRITE_ABORT_MSG  __DOS_WRITE_ABORT_MSG
#define _CALL_REPORTFAULT __DOS_CALL_REPORTFAULT
#define _MAX_ENV          __DOS_MAX_ENV

#ifndef _CRT_ERRNO_DEFINED
#define _CRT_ERRNO_DEFINED 1
__NAMESPACE_INT_BEGIN
#ifdef __CYG_COMPAT__
/* NOTE: Cygwin calls it `__errno()' and DOS calls it `_errno()' */
#define errno     (*__NAMESPACE_INT_SYM __errno())
__LIBC __WUNUSED __ATTR_CONST errno_t *(__LIBCCALL __errno)(void);
#elif defined(__DOS_COMPAT__)
#define errno     (*__NAMESPACE_INT_SYM _errno())
__LIBC __WUNUSED __ATTR_CONST errno_t *(__LIBCCALL _errno)(void);
#else
#define errno     (*__NAMESPACE_INT_SYM __errno_location())
__LIBC __WUNUSED __ATTR_CONST errno_t *(__LIBCCALL __errno_location)(void);
#endif
__NAMESPACE_INT_END
#ifdef __DOS_COMPAT__
__REDIRECT_NOTHROW(__LIBC,,errno_t,__LIBCCALL,__set_errno,(errno_t __err),_set_errno,(__err))
#else
__LIBC errno_t (__LIBCCALL __set_errno)(errno_t __err);
#endif
#if defined(__CRT_KOS) && (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
#ifdef __DOS_COMPAT__
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,errno_t,__LIBCCALL,__get_errno,(void),__get_doserrno,())
#else
__LIBC __WUNUSED errno_t (__LIBCCALL __get_errno)(void);
#endif
#else /* Builtin... */
__LOCAL __WUNUSED errno_t __NOTHROW((__LIBCCALL __get_errno)(void)) { return errno; }
#endif /* Compat... */
#if defined(__CRT_DOS) && !defined(__GLC_COMPAT__)
#ifdef __USE_DOS
__LIBC errno_t __NOTHROW((__LIBCCALL _get_errno)(errno_t *__perr));
__LIBC errno_t __NOTHROW((__LIBCCALL _set_errno)(errno_t __err));
#else /* __USE_DOS */
__LIBC errno_t __NOTHROW((__LIBCCALL _get_errno)(errno_t *__perr));
__LIBC errno_t __NOTHROW((__LIBCCALL _set_errno)(errno_t __err));
#endif /* !__USE_DOS */
#else /* Builtin... */
__LOCAL errno_t __NOTHROW((__LIBCCALL _get_errno)(errno_t *__perr)) { if (__perr) *__perr = errno; return 0; }
__LOCAL errno_t __NOTHROW((__LIBCCALL _set_errno)(errno_t __err)) { return (errno = __err); }
#endif /* Compat... */
#endif /* !_CRT_ERRNO_DEFINED */

#define _doserrno     (*__doserrno())
__LIBC __UINT32_TYPE__ *__NOTHROW((__LIBCCALL __doserrno)(void));
__LIBC errno_t __NOTHROW((__LIBCCALL _get_doserrno)(__UINT32_TYPE__ *__perr));
__LIBC errno_t __NOTHROW((__LIBCCALL _set_doserrno)(__UINT32_TYPE__ __err));

#ifndef ___sys_errlist_defined
#define ___sys_errlist_defined 1
#if defined(__CYG_COMPAT__) || defined(__GLC_COMPAT__)
 __LIBC char const *const _sys_errlist[];
 __LIBC int               _sys_nerr;
#else
__LIBC char **__NOTHROW((__LIBCCALL __sys_errlist)(void));
__LIBC int *__NOTHROW((__LIBCCALL __sys_nerr)(void));
#define _sys_errlist (*__sys_errlist())
#define _sys_nerr    (*__sys_nerr())
#endif
#endif /* !___sys_errlist_defined */

#undef _environ
#ifdef __DOS_COMPAT__
__LIBC char ***__NOTHROW((__LIBCCALL __p__environ)(void));
#define _environ  (*__p__environ())
#else /* __DOS_COMPAT__ */
#ifndef ____environ_defined
#define ____environ_defined 1
#ifndef __NO_ASMNAME
#undef __environ
__LIBC char **__environ __ASMNAME("environ");
#else /* __NO_ASMNAME */
#undef environ
__LIBC char **environ;
#define __environ environ
#endif /* !__NO_ASMNAME */
#endif /* !____environ_defined */
#define _environ  __environ
#endif /* !__DOS_COMPAT__ */

__LIBC __PORT_DOSONLY int *__NOTHROW((__LIBCCALL __p___argc)(void));
__LIBC __PORT_DOSONLY char ***__NOTHROW((__LIBCCALL __p___argv)(void));
__LIBC __PORT_DOSONLY char **__NOTHROW((__LIBCCALL __p__pgmptr)(void));
__LIBC __PORT_DOSONLY wchar_t ***(__LIBCCALL __p___wargv)(void);
__LIBC __PORT_DOSONLY wchar_t ***(__LIBCCALL __p__wenviron)(void);
__LIBC __PORT_DOSONLY wchar_t **(__LIBCCALL __p__wpgmptr)(void);
#define __argc    (*__p___argc())
#define __argv    (*__p___argv())
#define __wargv   (*__p___wargv())
#define _wenviron (*__p__wenviron())
#define _pgmptr   (*__p__pgmptr())
#define _wpgmptr  (*__p__wpgmptr())

#ifdef __USE_KOS
/* Access to the initial environment block. */
__LIBC char ***__NOTHROW((__LIBCCALL __p___initenv)(void));
__LIBC __PORT_DOSONLY wchar_t ***(__LIBCCALL __p___winitenv)(void);
#define _initenv  (*__p___initenv())
#define _winitenv (*__p___winitenv())
#endif /* __USE_KOS */

#if defined(__USE_UTF) && defined(__CRT_KOS)
#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */
__REDIRECT_NOTHROW(__LIBC,__PORT_DOSONLY,char16_t ***,__LIBCCALL,__p___uinitenv,(void),__p___winitenv,())
__REDIRECT_NOTHROW(__LIBC,__PORT_DOSONLY,char32_t ***,__LIBCCALL,__p___Uinitenv,(void),wgetinitenv,())
#define _uinitenv  (*__p___uinitenv())
#define _Uinitenv  (*__p___Uinitenv())
#endif /* __USE_UTF */

#ifndef _countof
#define _countof(a) __COMPILER_LENOF(a)
#endif /* !_countof */

#ifdef _MSC_VER
extern __ATTR_CONST __INT64_TYPE__ (__LIBCCALL _abs64)(__INT64_TYPE__ __x);
#pragma intrinsic(_abs64)
#elif __SIZEOF_LONG_LONG__ == 8
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,__INT64_TYPE__,__LIBCCALL,_abs64,(__INT64_TYPE__ __x),llabs,(__x))
#elif __SIZEOF_LONG__ == 8
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,__INT64_TYPE__,__LIBCCALL,_abs64,(__INT64_TYPE__ __x),labs,(__x))
#elif __SIZEOF_INTMAX_T__ == 8
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,__INT64_TYPE__,__LIBCCALL,_abs64,(__INT64_TYPE__ __x),imaxabs,(__x))
#else
__LIBC __ATTR_CONST __INT64_TYPE__ (__LIBCCALL _abs64)(__INT64_TYPE__ __x);
#endif

__REDIRECT(__LIBC,__NONNULL((1,4)),long,__LIBCCALL,__libc_strtol_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),__SYMNAME_DOSPREFIX(strtol_l),(__nptr,__endptr,__base,__locale))
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__NONNULL((1,4)),__LONGLONG,__LIBCCALL,__libc_strtoll_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),_strtoi64_l,(__nptr,__endptr,__base,__locale))
__LIBC double (__LIBCCALL _atof_l)(char const *__restrict __nptr, __locale_t __locale);
__LIBC int (__LIBCCALL _atoi_l)(char const *__restrict __nptr, __locale_t __locale);
__LIBC long int (__LIBCCALL _atol_l)(char const *__restrict __nptr, __locale_t __locale);
__LIBC __LONGLONG (__LIBCCALL _atoll_l)(char const *__restrict __nptr, __locale_t __locale);
#else /* __DOS_COMPAT__ */
__REDIRECT(__LIBC,__NONNULL((1,4)),__LONGLONG,__LIBCCALL,__libc_strtoll_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),strtoll_l,(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,__NONNULL((1,3)),double,__LIBCCALL,__libc_strtod_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __locale),__SYMNAME_DOSPREFIX(strtod_l),(__nptr,__endptr,__locale))
__LOCAL double (__LIBCCALL _atof_l)(char const *__restrict __nptr, __locale_t __locale) { return __libc_strtod_l(__nptr,__NULLPTR,__locale); }
__LOCAL int (__LIBCCALL _atoi_l)(char const *__restrict __nptr, __locale_t __locale) { return (int)__libc_strtol_l(__nptr,__NULLPTR,10,__locale); }
__LOCAL long int (__LIBCCALL _atol_l)(char const *__restrict __nptr, __locale_t __locale) { return __libc_strtol_l(__nptr,__NULLPTR,10,__locale); }
__LOCAL __LONGLONG (__LIBCCALL _atoll_l)(char const *__restrict __nptr, __locale_t __locale) { return __libc_strtoll_l(__nptr,__NULLPTR,10,__locale); }
#endif /* !__DOS_COMPAT__ */

#if defined(__CRT_DOS) && (defined(_MSC_VER) || defined(__OPTIMIZE_SIZE__))
#ifndef ___byteswap_ushort_defined
#define ___byteswap_ushort_defined 1
__LIBC __ATTR_CONST __UINT16_TYPE__ __NOTHROW((__LIBCCALL _byteswap_ushort)(__UINT16_TYPE__ x));
#ifdef _MSC_VER
#pragma intrinsic(_byteswap_ushort)
#endif /* _MSC_VER */
#endif /* !___byteswap_ushort_defined */
#ifndef ___byteswap_ulong_defined
#define ___byteswap_ulong_defined 1
__LIBC __ATTR_CONST __UINT32_TYPE__ __NOTHROW((__LIBCCALL _byteswap_ulong)(__UINT32_TYPE__ x));
#ifdef _MSC_VER
#pragma intrinsic(_byteswap_ulong)
#endif /* _MSC_VER */
#endif /* !___byteswap_ulong_defined */
#ifndef ___byteswap_uint64_defined
#define ___byteswap_uint64_defined 1
__LIBC __ATTR_CONST __UINT64_TYPE__ __NOTHROW((__LIBCCALL _byteswap_uint64)(__UINT64_TYPE__ x));
#ifdef _MSC_VER
#pragma intrinsic(_byteswap_uint64)
#endif /* _MSC_VER */
#endif /* !___byteswap_uint64_defined */
#else /* defined(__CRT_DOS) && defined(__OPTIMIZE_SIZE__) */
__LOCAL __ATTR_CONST __UINT16_TYPE__ __NOTHROW((__LIBCCALL _byteswap_ushort)(__UINT16_TYPE__ __val)) { return __bswap_16(__val); }
__LOCAL __ATTR_CONST __UINT32_TYPE__ __NOTHROW((__LIBCCALL _byteswap_ulong)(__UINT32_TYPE__ __val)) { return __bswap_32(__val); }
__LOCAL __ATTR_CONST __UINT64_TYPE__ __NOTHROW((__LIBCCALL _byteswap_uint64)(__UINT64_TYPE__ __val)) { return __bswap_64(__val); }
#endif /* __CRT_DOS && __OPTIMIZE_SIZE__ */

#ifdef __USE_DOS_SLIB
#ifndef __rsize_t_defined
#define __rsize_t_defined 1
typedef size_t rsize_t;
#endif /* !__rsize_t_defined */

#ifdef __CRT_DOS
#ifndef _CRT_ALGO_DEFINED
#define _CRT_ALGO_DEFINED 1
/* TODO: The following two could be emulated on linux... */
__LIBC __PORT_DOSONLY __NONNULL((1,2,5)) void *(__LIBCCALL bsearch_s)(void const *__key, void const *__base, size_t __nmemb, size_t __size, int (__LIBCCALL *__compar)(void *__arg, void const *__a, void const *__b), void *__arg);
__LIBC __PORT_DOSONLY __NONNULL((1,4)) void (__LIBCCALL qsort_s)(void *__base, size_t __nmemb, size_t __size, int (__LIBCCALL *__compar)(void *__arg, void const *__a, void const *__b), void *__arg);
#endif  /* _CRT_ALGO_DEFINED */
__LIBC __PORT_DOSONLY __WARN_NOKOSFS errno_t (__LIBCCALL getenv_s)(size_t *__psize, char *__buf, rsize_t __bufsize, char const *__name);
__LIBC __PORT_DOSONLY __WARN_NOKOSFS errno_t (__LIBCCALL _dupenv_s)(char **__restrict __pbuf, size_t *__pbuflen, char const *__name);
#endif /* __CRT_DOS */
#endif /* __USE_DOS_SLIB */

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY char *(__LIBCCALL _itoa)(int __val, char *__dst, int __radix);
__LIBC __PORT_DOSONLY char *(__LIBCCALL _i64toa)(__INT64_TYPE__ __val, char *__dst, int __radix);
__LIBC __PORT_DOSONLY char *(__LIBCCALL _ui64toa)(__UINT64_TYPE__ __val, char *__dst, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _itoa_s)(int __val, char *__dst, size_t __bufsize, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _i64toa_s)(__INT64_TYPE__ __val, char *__dst, size_t __bufsize, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ui64toa_s)(__UINT64_TYPE__ __val, char *__dst, size_t __bufsize, int __radix);
#endif /* __CRT_DOS */

#if __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_atoi64,(char const *__restrict __nptr),atol,(__nptr))
#ifdef __CRT_DOS
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_atoi64_l,(char const *__restrict __nptr, __locale_t __locale),_atol_l,(__nptr,__locale))
#else /* __CRT_DOS */
__LOCAL __INT64_TYPE__ (__LIBCCALL _atoi64_l)(char const *__restrict __nptr, __locale_t __locale) { return __libc_strtol_l(__nptr,__NULLPTR,0,__locale); }
#endif /* !__CRT_DOS */
__REDIRECT(__LIBC,,__INT64_TYPE__ ,__LIBCCALL,_strtoi64,(char const *__restrict __nptr, char **__restrict __endptr, int __radix),strtol,(__nptr,__endptr,__radix))
__REDIRECT(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_strtoui64,(char const *__restrict __nptr, char **__restrict __endptr, int __radix),strtoul,(__nptr,__endptr,__radix))
__REDIRECT(__LIBC,,__INT64_TYPE__ ,__LIBCCALL,_strtoi64_l, (char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(strtol_l),(__nptr,__endptr,__radix))
__REDIRECT(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_strtoui64_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(strtoul_l),(__nptr,__endptr,__radix))
#elif __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_atoi64,(char const *__restrict __nptr),atoll,(__nptr))
#ifdef __CRT_DOS
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_atoi64_l,(char const *__restrict __nptr, __locale_t __locale),_atoll_l,(__nptr,__locale))
#else /* __CRT_DOS */
__LOCAL __INT64_TYPE__ (__LIBCCALL _atoi64_l)(char const *__restrict __nptr, __locale_t __locale) { return __libc_strtoll_l(__nptr,__NULLPTR,0,__locale); }
#endif /* !__CRT_DOS */
__REDIRECT(__LIBC,,__INT64_TYPE__ ,__LIBCCALL,_strtoi64,(char const *__restrict __nptr, char **__restrict __endptr, int __radix),strtoll,(__nptr,__endptr,__radix))
__REDIRECT(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_strtoui64,(char const *__restrict __nptr, char **__restrict __endptr, int __radix),strtoull,(__nptr,__endptr,__radix))
__REDIRECT(__LIBC,,__INT64_TYPE__ ,__LIBCCALL,_strtoi64_l, (char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(strtoll_l),(__nptr,__endptr,__radix,__locale))
__REDIRECT(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_strtoui64_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(strtoull_l),(__nptr,__endptr,__radix,__locale))
#else
__LIBC __INT64_TYPE__ (__LIBCCALL _atoi64)(char const *__restrict __nptr);
__LIBC __INT64_TYPE__ (__LIBCCALL _atoi64_l)(char const *__restrict __nptr, __locale_t __locale);
__LIBC __INT64_TYPE__ (__LIBCCALL _strtoi64)(char const *__restrict __nptr, char **__restrict __endptr, int __radix);
__LIBC __INT64_TYPE__ (__LIBCCALL _strtoi64_l)(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale);
__LIBC __UINT64_TYPE__ (__LIBCCALL _strtoui64)(char const *__restrict __nptr, char **__restrict __endptr, int __radix);
__LIBC __UINT64_TYPE__ (__LIBCCALL _strtoui64_l)(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale);
#endif

#ifdef __CRT_DOS
#if __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,_ltoa,(long __val,  char *__buf, int __radix),_i64toa,(__val,__buf,__radix))
__REDIRECT(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ltoa_s,(long __val, char *__buf, size_t __buflen, int __radix),_i64toa_s,(__val,__buf,__buflen,__radix))
#elif __SIZEOF_LONG__ == __SIZEOF_INT__
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,_ltoa,(long __val,  char *__buf, int __radix),_itoa,(__val,__buf,__radix))
__REDIRECT(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ltoa_s,(long __val, char *__buf, size_t __buflen, int __radix),_itoa_s,(__val,__buf,__buflen,__radix))
#else /* ... */
__LIBC __PORT_DOSONLY char *(__LIBCCALL _ltoa)(long __val,  char *__buf, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ltoa_s)(long __val, char *__buf, size_t __buflen, int __radix);
#endif /* !... */
__LIBC __PORT_DOSONLY size_t (__LIBCCALL _mbstrlen)(char const *__str);
__LIBC __PORT_DOSONLY size_t (__LIBCCALL _mbstrlen_l)(char const *__str, __locale_t __locale);
__LIBC __PORT_DOSONLY size_t (__LIBCCALL _mbstrnlen)(char const *__str, size_t __maxlen);
__LIBC __PORT_DOSONLY size_t (__LIBCCALL _mbstrnlen_l)(char const *__str, size_t __maxlen, __locale_t __locale);
#endif /* __CRT_DOS */

#ifdef __USE_KOS
__REDIRECT_DPB(__LIBC,,size_t,__LIBCCALL,mblen_l,(char const *__str, size_t __maxlen, __locale_t __locale),(__str,__maxlen,__locale))
__REDIRECT_DPB(__LIBC,,size_t,__LIBCCALL,mbtowc_l,(wchar_t *__dst, char const *__src, size_t __srclen, __locale_t __locale),(__str,__maxlen,__locale))
#else /* __USE_KOS */
__REDIRECT_DPB(__LIBC,,int,__LIBCCALL,mblen_l,(char const *__str, size_t __maxlen, __locale_t __locale),(__str,__maxlen,__locale))
__REDIRECT_DPB(__LIBC,,int,__LIBCCALL,mbtowc_l,(wchar_t *__dst, char const *__src, size_t __srclen, __locale_t __locale),(__str,__maxlen,__locale))
#endif /* !__USE_KOS */
__REDIRECT_DPB(__LIBC,,size_t,__LIBCCALL,mbstowcs_l,(wchar_t *__buf, char const *__src, size_t __maxlen, __locale_t __locale),(__buf,__src,__maxlen,__locale))

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL mbstowcs_s)(size_t *__presult, wchar_t *__buf, size_t __buflen, char const *__src, size_t __maxlen);
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,mbstowcs_s_l,(size_t *__presult, wchar_t *__buf, size_t __buflen, char const *__src, size_t __maxlen, __locale_t __locale),(__presult,__buf,__buflen,__src,__maxlen,__locale))
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL rand_s)(unsigned int *__restrict __randval);
__LIBC __PORT_DOSONLY int (__LIBCCALL _set_error_mode)(int __mode);
#endif /* __CRT_DOS */

#ifdef __DOS_COMPAT__
#if __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,__NONNULL((1,4)),long,__LIBCCALL,_strtol_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),_strtoi64_l,(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,__NONNULL((1,4)),unsigned long,__LIBCCALL,_strtoul_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),_strtoui64_l,(__nptr,__endptr,__base,__locale))
#else
__LIBC __NONNULL((1,4)) long (__LIBCCALL _strtol_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale);
__LIBC __NONNULL((1,4)) unsigned long (__LIBCCALL _strtoul_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale);
#endif
#else /* __DOS_COMPAT__ */
__REDIRECT(__LIBC,__NONNULL((1,4)),long,__LIBCCALL,_strtol_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),strtol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,__NONNULL((1,4)),unsigned long,__LIBCCALL,_strtoul_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),strtoul_l,(__nptr,__endptr,__base,__locale))
#endif /* !__DOS_COMPAT__ */
__REDIRECT_DPB(__LIBC,__NONNULL((1,4)),__LONGLONG,__LIBCCALL,strtoll_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),(__nptr,__endptr,__base,__locale))
__REDIRECT_DPB(__LIBC,__NONNULL((1,4)),__ULONGLONG,__LIBCCALL,strtoull_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),(__nptr,__endptr,__base,__locale))
__REDIRECT_DPB(__LIBC,__NONNULL((1,3)),float,__LIBCCALL,strtof_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __locale),(__nptr,__endptr,__locale))
__REDIRECT_DPB(__LIBC,__NONNULL((1,3)),double,__LIBCCALL,strtod_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __locale),(__nptr,__endptr,__locale))
__REDIRECT_DPB(__LIBC,__NONNULL((1,3)),long double,__LIBCCALL,strtold_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __locale),(__nptr,__endptr,__locale))

#ifndef _CRT_SYSTEM_DEFINED
#define _CRT_SYSTEM_DEFINED 1
#endif /* !_CRT_SYSTEM_DEFINED */

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ultoa_s)(unsigned long int __val, char *__buf, size_t __bufsize, int __radix);
__LIBC __PORT_DOSONLY char *(__LIBCCALL _ultoa)(unsigned long int __val, char *__buf, int __radix);
#ifdef __USE_KOS
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,size_t,__LIBCCALL,wctomb_l,(char *__buf, wchar_t __wc, __locale_t __locale),(__buf,__wc,__locale))
#else /* __USE_KOS */
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,wctomb_l,(char *__buf, wchar_t __wc, __locale_t __locale),(__buf,__wc,__locale))
#endif /* !__USE_KOS */
#ifdef __USE_DOS_SLIB
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL wctomb_s)(int *__presult, char *__buf, rsize_t __buflen, wchar_t __wc);
#endif /* __USE_DOS_SLIB */
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wctomb_s_l,(int *__presult, char *__buf, size_t __buflen, wchar_t __wc, __locale_t __locale),(__presult,__buf,__buflen,__wc,__locale))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wcstombs_s_l,(size_t *__presult, char *__buf, size_t __buflen, wchar_t const *__src, size_t __maxlen, __locale_t __locale),(__presult,__buf,__buflen,__src,__maxlen,__locale))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,size_t,__LIBCCALL,_wcstombs_l,(char *__dst, wchar_t const *__src, size_t __maxlen, __locale_t __locale),(__dst,__src,__maxlen,__locale))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wcstombs_s,(size_t *__presult, char *__buf, size_t __buflen, wchar_t const *__src, size_t __maxlen),(__presult,__buf,__buflen,__src,__maxlen))

/* DOS malloc extensions. (TODO: With some work, these could be emulated) */
__LIBC __PORT_DOSONLY __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2,3)) void *(__LIBCCALL _recalloc)(void *__mptr, size_t __count, size_t __num_bytes);
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_ALLOC_ALIGN(2) __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL _aligned_malloc)(size_t __num_bytes, size_t __min_alignment);
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL _aligned_offset_malloc)(size_t __num_bytes, size_t __min_alignment, size_t __offset);
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_ALLOC_ALIGN(3) __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL _aligned_realloc)(void *__mptr, size_t __newsize, size_t __min_alignment);
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_ALLOC_ALIGN(4) __ATTR_ALLOC_SIZE((2,3)) void *(__LIBCCALL _aligned_recalloc)(void *__mptr, size_t __count, size_t __num_bytes, size_t __min_alignment);
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL _aligned_offset_realloc)(void *__mptr, size_t __newsize, size_t __min_alignment, size_t __offset);
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_ALLOC_SIZE((1,2)) void *(__LIBCCALL _aligned_offset_recalloc)(void *__mptr, size_t __count, size_t __num_bytes, size_t __min_alignment, size_t __offset);
__LIBC __PORT_DOSONLY __WUNUSED __NONNULL((1)) size_t (__LIBCCALL _aligned_msize)(void *__mptr, size_t __min_alignment, size_t __offset);
__LIBC __PORT_DOSONLY void  (__LIBCCALL _aligned_free)(void *__mptr);
#endif /* __CRT_DOS */

#ifdef __CRT_DOS
#define _CVTBUFSIZE   349
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,fullpath,(char *__buf, char const *__path, size_t __buflen),(__buf,__path,__buflen))
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ecvt_s)(char *__buf, size_t __buflen, double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _fcvt_s)(char *__buf, size_t __buflen, double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _gcvt_s)(char *__buf, size_t __buflen, double __val, int __ndigit);
#endif /* __CRT_DOS */
__REDIRECT_DPB(__LIBC,__WUNUSED __NONNULL((3,4)),char *,__LIBCCALL,ecvt,(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),(__val,__ndigit,__decptr,__sign))
__REDIRECT_DPB(__LIBC,__WUNUSED __NONNULL((3,4)),char *,__LIBCCALL,fcvt,(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),(__val,__ndigit,__decptr,__sign))
__REDIRECT_DPB(__LIBC,__WUNUSED __NONNULL((3)),char *,__LIBCCALL,gcvt,(double __val, int __ndigit, char *__buf),(__val,__ndigit,__buf))

#ifdef __USE_KOS
#define __FIXED_CONST const
#else
#define __FIXED_CONST /* Nothing */
#endif
#if defined(__CRT_DOS) && !defined(__GLC_COMPAT__) && !defined(__CYG_COMPAT__)
__LIBC int (__LIBCCALL _atoflt)(float *__restrict __result, char const *__restrict __nptr);
__LIBC int (__LIBCCALL _atodbl)(double *__restrict __result, char __FIXED_CONST *__restrict __nptr);
__LIBC int (__LIBCCALL _atoldbl)(long double *__restrict __result, char __FIXED_CONST *__restrict __nptr);
__LIBC int (__LIBCCALL _atoflt_l)(float *__restrict __result, char const *__restrict __nptr, __locale_t __locale);
__LIBC int (__LIBCCALL _atodbl_l)(double *__restrict __result, char __FIXED_CONST *__restrict __nptr, __locale_t __locale);
__LIBC int (__LIBCCALL _atoldbl_l)(long double *__restrict __result, char __FIXED_CONST *__restrict __nptr, __locale_t __locale);
#else /* __CRT_DOS... */
__LOCAL int (__LIBCCALL _atoflt)(float *__restrict __result, char const *__restrict __nptr) { *__result = __NAMESPACE_STD_SYM strtof(__nptr,NULL); return 0; }
__LOCAL int (__LIBCCALL _atodbl)(double *__restrict __result, char __FIXED_CONST *__restrict __nptr) { *__result = __NAMESPACE_STD_SYM strtod(__nptr,NULL); return 0; }
__LOCAL int (__LIBCCALL _atoldbl)(long double *__restrict __result, char __FIXED_CONST *__restrict __nptr) { *__result = __NAMESPACE_STD_SYM strtold(__nptr,NULL); return 0; }
__LOCAL int (__LIBCCALL _atoflt_l)(float *__restrict __result, char const *__restrict __nptr, __locale_t __locale) { *__result = _strtof_l(__nptr,NULL,__locale); return 0; }
__LOCAL int (__LIBCCALL _atodbl_l)(double *__restrict __result, char __FIXED_CONST *__restrict __nptr, __locale_t __locale) { *__result = _strtod_l(__nptr,NULL,__locale); return 0; }
__LOCAL int (__LIBCCALL _atoldbl_l)(long double *__restrict __result, char __FIXED_CONST *__restrict __nptr, __locale_t __locale) { *__result = _strtold_l(__nptr,NULL,__locale); return 0; }
#endif /* !__CRT_DOS... */
#undef __FIXED_CONST

#if defined(__CRT_DOS) && !defined(__GLC_COMPAT__) && \
   !defined(__CYG_COMPAT__) && defined(__OPTIMIZE_SIZE__)
__LIBC __ATTR_CONST unsigned int __NOTHROW((__LIBCCALL _rotl)(unsigned int __val, int __shift));
__LIBC __ATTR_CONST unsigned int __NOTHROW((__LIBCCALL _rotr)(unsigned int __val, int __shift));
__LIBC __ATTR_CONST __UINT64_TYPE__ __NOTHROW((__LIBCCALL _rotl64)(__UINT64_TYPE__ __val, int __shift));
__LIBC __ATTR_CONST __UINT64_TYPE__ __NOTHROW((__LIBCCALL _rotr64)(__UINT64_TYPE__ __val, int __shift));
#if __SIZEOF_LONG__ == __SIZEOF_INT__
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,unsigned long int,__LIBCCALL,_lrotl,(unsigned long int __val, int __shift),_rotl,(__val,__shift))
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,unsigned long int,__LIBCCALL,_lrotr,(unsigned long int __val, int __shift),_rotr,(__val,__shift))
#elif __SIZEOF_LONG__ == 8
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,unsigned long int,__LIBCCALL,_lrotl,(unsigned long int __val, int __shift),_rotl64,(__val,__shift))
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST,unsigned long int,__LIBCCALL,_lrotr,(unsigned long int __val, int __shift),_rotr64,(__val,__shift))
#else
__LIBC __ATTR_CONST unsigned long int __NOTHROW((__LIBCCALL _lrotl)(unsigned long int __val, int __shift));
__LIBC __ATTR_CONST unsigned long int __NOTHROW((__LIBCCALL _lrotr)(unsigned long int __val, int __shift));
#endif
#else /* __CRT_DOS */
__SYSDECL_END
#include <bits/rotate.h>
__SYSDECL_BEGIN
#ifdef __WINDOWS_HOST__
extern unsigned int (__LIBCCALL _rotl)(unsigned int __val, int __shift);
extern unsigned int (__LIBCCALL _rotr)(unsigned int __val, int __shift);
#else
__LOCAL __ATTR_CONST unsigned int __NOTHROW((__LIBCCALL _rotl)(unsigned int __val, int __shift)) { return __rol_32(__val,__shift); }
__LOCAL __ATTR_CONST unsigned int __NOTHROW((__LIBCCALL _rotr)(unsigned int __val, int __shift)) { return __ror_32(__val,__shift); }
#endif
#ifdef __WINDOWS_HOST__
extern unsigned long long (__cdecl _rotl64)(unsigned long long __val, int __shift);
extern unsigned long long (__cdecl _rotr64)(unsigned long long __val, int __shift);
#else
__LOCAL __ATTR_CONST __UINT64_TYPE__ __NOTHROW((__LIBCCALL _rotl64)(__UINT64_TYPE__ __val, int __shift)) { return __rol_64(__val,__shift); }
__LOCAL __ATTR_CONST __UINT64_TYPE__ __NOTHROW((__LIBCCALL _rotr64)(__UINT64_TYPE__ __val, int __shift)) { return __ror_64(__val,__shift); }
#endif
#ifdef __WINDOWS_HOST__
extern unsigned long int (__LIBCCALL _lrotl)(unsigned long int __val, int __shift);
extern unsigned long int (__LIBCCALL _lrotr)(unsigned long int __val, int __shift);
#else
__LOCAL __ATTR_CONST unsigned long int __NOTHROW((__LIBCCALL _lrotl)(unsigned long int __val, int __shift)) { return __rol_32(__val,__shift); }
__LOCAL __ATTR_CONST unsigned long int __NOTHROW((__LIBCCALL _lrotr)(unsigned long int __val, int __shift)) { return __ror_32(__val,__shift); }
#endif
#endif /* !__CRT_DOS */


#ifndef _CRT_PERROR_DEFINED
#define _CRT_PERROR_DEFINED 1
#ifndef __std_perror_defined
#define __std_perror_defined 1
__NAMESPACE_STD_BEGIN
__LIBC void (__LIBCCALL perror)(char const *__s);
__NAMESPACE_STD_END
#endif /* !__std_perror_defined */
#ifndef __perror_defined
#define __perror_defined 1
__NAMESPACE_STD_USING(perror)
#endif /* !__perror_defined */
#endif  /* _CRT_PERROR_DEFINED */


__REDIRECT_UFSDPB(__LIBC,__NONNULL((1)),int,__LIBCCALL,putenv,(char *__string),(__string))
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__ && !defined(__DOS_COMPAT__)
__REDIRECT_VOID(__LIBC,__NONNULL((1,2)),__LIBCCALL,_swab,(void const *__restrict __from, void *__restrict __to, int __n_bytes),swab,(__from,__to,__n_bytes))
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ && !__DOS_COMPAT__ */
__LIBC __NONNULL((1,2)) void (__LIBCCALL _swab)(void const *__restrict __from, void *__restrict __to, int __n_bytes);
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ || __DOS_COMPAT__ */

#ifdef __CRT_DOS
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY_ALT(setenv),errno_t,__LIBCCALL,putenv_s,(char const *__name, char const *__val),(__name,__val))
__REDIRECT_UFSDPB_VOID(__LIBC,__PORT_DOSONLY,__LIBCCALL,makepath,(char *__restrict __buf, char const *__drive, char const *__dir, char const *__file, char const *__ext),(__buf,__drive,__dir,__file,__ext))
__REDIRECT_UFSDPB_VOID(__LIBC,__PORT_DOSONLY,__LIBCCALL,searchenv,(char const *__file, char const *__envvar, char *__restrict __resultpath),(__file,__envvar,__resultpath))
__REDIRECT_UFSDPB_VOID(__LIBC,__PORT_DOSONLY,__LIBCCALL,splitpath,(char const *__restrict __abspath, char *__drive, char *__dir, char *__file, char *__ext),(__abspath,__drive,__dir,__file,__ext))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,makepath_s,(char *__buf, size_t __buflen, char const *__drive, char const *__dir, char const *__file, char const *__ext),(__buf,__buflen,__drive,__dir,__file,__ext))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,searchenv_s,(char const *__file, char const *__envvar, char *__restrict __resultpath, size_t __buflen),(__file,__envvar,__resultpath,__buflen))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,splitpath_s,(char const *__restrict __abspath, char *__drive, size_t __drivelen, char *__dir, size_t __dirlen, char *__file, size_t __filelen, char *__ext, size_t __extlen),(__abspath,__drive,__drivelen,__dir,__dirlen,__file,__filelen,__ext,__extlen))
#ifndef _CRT_WPERROR_DEFINED
#define _CRT_WPERROR_DEFINED 1
__REDIRECT_DPB_VOID(__LIBC,__PORT_DOSONLY __ATTR_COLD,__LIBCCALL,wperror,(wchar_t const *__restrict __errmsg),(__errmsg))
#endif /* !_CRT_WPERROR_DEFINED */
__LIBC __PORT_DOSONLY void (__LIBCCALL _seterrormode)(int __mode);
__LIBC __PORT_DOSONLY void (__LIBCCALL _beep)(unsigned int __freq, unsigned int __duration);
#endif /* __CRT_DOS */

#if __SIZEOF_INT__ == 4 && !defined(__DOS_COMPAT__)
__REDIRECT_VOID(__LIBC,,__LIBCCALL,_sleep,(__UINT32_TYPE__ __duration),sleep,(__duration))
#else /* __SIZEOF_INT__ == 4 && !__DOS_COMPAT__ */
__LIBC void (__LIBCCALL _sleep)(__UINT32_TYPE__ __duration);
#endif /* __SIZEOF_INT__ != 4 || __DOS_COMPAT__ */

#ifndef __cplusplus
#define min(a,b)   __min(a,b)
#define max(a,b)   __max(a,b)
#endif /* !__cplusplus */

#define sys_errlist _sys_errlist
#define sys_nerr    _sys_nerr
#ifndef __environ_defined
#define __environ_defined 1
#define environ     _environ
#endif /* !__environ_defined */

#ifndef __swab_defined
#define __swab_defined 1
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__ && !defined(__DOS_COMPAT__)
__LIBC __NONNULL((1,2)) void (__LIBCCALL swab)(void const *__restrict __from, void *__restrict __to, int __n_bytes);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
__REDIRECT_VOID(__LIBC,__NONNULL((1,2)),__LIBCCALL,swab,(void const *__restrict __from, void *__restrict __to, int __n_bytes),_swab,(__from,__to,__n_bytes))
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#endif /* !__swab_defined */

#ifdef __CRT_DOS
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,itoa,(int __val, char *__buf, int __radix),_itoa,(__val,__buf,__radix))
#if __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,ltoa,(long __val, char *__buf, int __radix),_i64toa,(__val,__buf,__radix))
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,ultoa,(unsigned long __val, char *__buf, int __radix),_ui64toa,(__val,__buf,__radix))
#elif __SIZEOF_LONG__ == __SIZEOF_INT__
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,ltoa,(long __val, char *__buf, int __radix),_itoa,(__val,__buf,__radix))
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,ultoa,(unsigned long __val, char *__buf, int __radix),_ultoa,(__val,__buf,__radix))
#else /* ... */
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,ltoa,(long __val, char *__buf, int __radix),_ltoa,(__val,__buf,__radix))
__REDIRECT(__LIBC,__PORT_DOSONLY,char *,__LIBCCALL,ultoa,(unsigned long __val, char *__buf, int __radix),_ultoa,(__val,__buf,__radix))
#endif /* !... */
__LIBC __PORT_DOSONLY_ALT(atexit) onexit_t (__LIBCCALL _onexit)(onexit_t __func);
__REDIRECT(__LIBC,__PORT_DOSONLY_ALT(atexit),onexit_t,__LIBCCALL,onexit,(onexit_t __func),_onexit,(__func))
#endif /* __CRT_DOS */

__SYSDECL_END

#ifndef _PARTS_DOS_WSTDLIB_H
#include "wstdlib.h"
#endif

#endif /* !_PARTS_DOS_STDLIB_H */
