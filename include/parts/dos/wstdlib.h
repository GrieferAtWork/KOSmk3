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
#ifndef _PARTS_DOS_WSTDLIB_H
#define _PARTS_DOS_WSTDLIB_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <xlocale.h>

__SYSDECL_BEGIN

#ifndef _WSTDLIB_DEFINED
#define _WSTDLIB_DEFINED 1

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifdef __CRT_DOS
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,i64tow,(__INT64_TYPE__ __val, wchar_t *__restrict __dst, int __radix),(__val,__dst,__radix))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,ui64tow,(__UINT64_TYPE__ __val, wchar_t *__restrict __dst, int __radix),(__val,__dst,__radix))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,i64tow_s,(__INT64_TYPE__ __val, wchar_t *__restrict __dst, __SIZE_TYPE__ __maxlen, int __radix),(__val,__dst,__maxlen,__radix))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,ui64tow_s,(__UINT64_TYPE__ __val, wchar_t *__restrict __dst, __SIZE_TYPE__ __maxlen, int __radix),(__val,__dst,__maxlen,__radix))
#if __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_ultow,(unsigned long int __val, wchar_t *__restrict __dst, int __radix),__SYMNAME_DOSPREFIX(ui64tow),(__val,__dst,__radix))
__REDIRECT(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,_ltow,(long int __val, wchar_t *__restrict __dst, int __radix),__SYMNAME_DOSPREFIX(i64tow),(__val,__dst,__radix))
__REDIRECT(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ultow_s,(unsigned long int __val, wchar_t *__dst, __SIZE_TYPE__ __maxlen, int __radix),__SYMNAME_DOSPREFIX(ui64tow_s),(__val,__dst,__maxlen,__radix))
__REDIRECT(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_ltow_s,(long int __val, wchar_t *__restrict __dst, __SIZE_TYPE__ __maxlen, int __radix),__SYMNAME_DOSPREFIX(i64tow_s),(__val,__dst,__maxlen,__radix))
#elif __SIZEOF_LONG__ == 4
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,ultow,(unsigned long int __val, wchar_t *__restrict __dst, int __radix),(__val,__dst,__radix))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,ltow,(long int __val, wchar_t *__restrict __dst, int __radix),(__val,__dst,__radix))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,ultow_s,(unsigned long int __val, wchar_t *__dst, __SIZE_TYPE__ __maxlen, int __radix),(__val,__dst,__maxlen,__radix))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,ltow_s,(long int __val, wchar_t *__restrict __dst, __SIZE_TYPE__ __maxlen, int __radix),(__val,__dst,__maxlen,__radix))
#else
__LIBC __PORT_DOSONLY wchar_t *(__LIBCCALL _ultow)(unsigned long int __val, wchar_t *__restrict __dst, int __radix);
__LIBC __PORT_DOSONLY wchar_t *(__LIBCCALL _ltow)(long int __val, wchar_t *__restrict __dst, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ultow_s)(unsigned long int __val, wchar_t *__dst, __SIZE_TYPE__ __maxlen, int __radix);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _ltow_s)(long int __val, wchar_t *__restrict __dst, __SIZE_TYPE__ __maxlen, int __radix);
#endif /* __SIZEOF_LONG__ != 8 */
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,itow,(int __val, wchar_t *__restrict __dst, int __radix),(__val,__dst,__radix))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,itow_s,(int __val, wchar_t *__restrict __dst, __SIZE_TYPE__ __maxlen, int __radix),(__val,__dst,__maxlen,__radix))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,wgetenv,(wchar_t const *__restrict __varname),(__varname))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wgetenv_s,(__SIZE_TYPE__ *__restrict __psize, wchar_t *__restrict __buf, __SIZE_TYPE__ __buflen, wchar_t const *__restrict __varname),(__psize,__buf,__buflen,__varname))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wdupenv_s,(wchar_t **__restrict __pbuf, __SIZE_TYPE__ *__restrict __pbuflen, wchar_t const *__restrict __varname),(__pbuf,__pbuflen,__varname))
#endif /* __CRT_DOS */

__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,long int,__LIBCCALL,wcstol_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),(__s,__pend,__radix,__locale))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,unsigned long int,__LIBCCALL,wcstoul_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),(__s,__pend,__radix,__locale))
#if defined(__DOS_COMPAT__) && defined(__CRT_DOS) && __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,,__LONGLONG,__LIBCCALL,_wcstoll_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),_wcstoi64_l,(__s,__pend,__radix,__locale))
__REDIRECT(__LIBC,,__ULONGLONG,__LIBCCALL,_wcstoull_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstoui64_l,(__s,__pend,__radix,__locale))
#else
__REDIRECT_DPB(__LIBC,,__LONGLONG,__LIBCCALL,wcstoll_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),(__s,__pend,__radix,__locale))
__REDIRECT_DPB(__LIBC,,__ULONGLONG,__LIBCCALL,wcstoull_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),(__s,__pend,__radix,__locale))
#endif
__REDIRECT_DPB(__LIBC,,float,__LIBCCALL,wcstof_l,(wchar_t const *__restrict __s, wchar_t **__restrict __pend, __locale_t __locale),(__s,__pend,__locale))
__REDIRECT_DPB(__LIBC,,double,__LIBCCALL,wcstod_l,(wchar_t const *__restrict __s, wchar_t **__pend, __locale_t __locale),(__s,__pend,__locale))
__REDIRECT_DPB(__LIBC,,long double,__LIBCCALL,wcstold_l,(wchar_t const *__restrict __s, wchar_t **__pend, __locale_t __locale),(__s,__pend,__locale))

#ifdef __CRT_DOS
__REDIRECT_DPB(__LIBC,,long int,__LIBCCALL,wtol,(wchar_t const *__restrict __s),(__s))
__REDIRECT_DPB(__LIBC,,long int,__LIBCCALL,wtol_l,(wchar_t const *__restrict __s, __locale_t __locale),(__s,__locale))
#if defined(__DOS_COMPAT__) && __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,,__LONGLONG,__LIBCCALL,_wtoll,(wchar_t const *__restrict __s),__SYMNAME_DOSPREFIX(wtoi64),(__s))
__REDIRECT(__LIBC,,__LONGLONG,__LIBCCALL,_wtoll_l,(wchar_t const *__restrict __s, __locale_t __locale),__SYMNAME_DOSPREFIX(wtoi64_l),(__s,__locale))
#else
__REDIRECT_DPB(__LIBC,,__LONGLONG,__LIBCCALL,wtoll,(wchar_t const *__restrict __s),(__s))
__REDIRECT_DPB(__LIBC,,__LONGLONG,__LIBCCALL,wtoll_l,(wchar_t const *__restrict __s, __locale_t __locale),(__s,__locale))
#endif
#if __SIZEOF_INT__ == __SIZEOF_LONG__
__REDIRECT(__LIBC,,int,__LIBCCALL,_wtoi,(wchar_t const *__restrict __s),__SYMNAME_DOSPREFIX(wtol),(__s))
__REDIRECT(__LIBC,,int,__LIBCCALL,_wtoi_l,(wchar_t const *__restrict __s, __locale_t __locale),__SYMNAME_DOSPREFIX(wtol_l),(__s,__locale))
#else /* __SIZEOF_INT__ == __SIZEOF_LONG__ */
__REDIRECT_DPB(__LIBC,,int,__LIBCCALL,wtoi,(wchar_t const *__restrict __s),(__s))
__REDIRECT_DPB(__LIBC,,int,__LIBCCALL,wtoi_l,(wchar_t const *__restrict __s, __locale_t __locale),(__s,__locale))
#endif /* __SIZEOF_INT__ != __SIZEOF_LONG__ */
__REDIRECT_DPB(__LIBC,,double,__LIBCCALL,wtof,(wchar_t const *__restrict __s),(__s))
__REDIRECT_DPB(__LIBC,,double,__LIBCCALL,wtof_l,(wchar_t const *__restrict __s, __locale_t __locale),(__s,__locale))
#else /* __CRT_DOS */
__LOCAL long int (__LIBCCALL _wtol)(wchar_t const *__restrict __s) { return __NAMESPACE_STD_SYM wcstol(__s,0,10); }
__LOCAL long int (__LIBCCALL _wtol_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstol_l(__s,0,10,__locale); }
__LOCAL __LONGLONG (__LIBCCALL _wtoll)(wchar_t const *__restrict __s) { return __NAMESPACE_STD_SYM wcstoll(__s,0,10); }
__LOCAL __LONGLONG (__LIBCCALL _wtoll_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstoll_l(__s,0,10,__locale); }
__LOCAL int (__LIBCCALL _wtoi)(wchar_t const *__restrict __s) { return (int)__NAMESPACE_STD_SYM wcstol(__s,0,10); }
__LOCAL int (__LIBCCALL _wtoi_l)(wchar_t const *__restrict __s, __locale_t __locale) { return (int)_wcstol_l(__s,0,10,__locale); }
__LOCAL double (__LIBCCALL _wtof)(wchar_t const *__restrict __s) { return __NAMESPACE_STD_SYM wcstod(__s,0); }
__LOCAL double (__LIBCCALL _wtof_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstod_l(__s,0,__locale); }
#endif /* !__CRT_DOS */

#if __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wcstoi64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),wcstol,(__s,__pend,__radix))
__REDIRECT(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_wcstoui64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),wcstoul,(__s,__pend,__radix))
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wcstoi64_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstol_l),(__s,__pend,__radix,__locale))
__REDIRECT(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_wcstoui64_l,(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstoul_l),(__s,__pend,__radix,__locale))
#ifdef __CRT_DOS
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wtoi64,(wchar_t const *__restrict __s),__SYMNAME_DOSPREFIX(wtol),(__s))
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wtoi64_l,(wchar_t const *__restrict __s, __locale_t __locale),__SYMNAME_DOSPREFIX(wtol_l),(__s))
#else /* __CRT_DOS */
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) { return _wcstoi64(__s,0,10); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstoi64_l(__s,0,10,__locale); }
#endif /* !__CRT_DOS */
#elif __SIZEOF_LONG_LONG__ == 8
#ifdef __DOS_COMPAT__
__REDIRECT_DPB(__LIBC,,__INT64_TYPE__,__LIBCCALL,wcstoi64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),(__s,__pend,__radix))
__REDIRECT_DPB(__LIBC,,__UINT64_TYPE__,__LIBCCALL,wcstoui64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),(__s,__pend,__radix))
__REDIRECT_DPB(__LIBC,,__INT64_TYPE__,__LIBCCALL,wcstoi64_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),(__s,__pend,__radix,__locale))
__REDIRECT_DPB(__LIBC,,__UINT64_TYPE__,__LIBCCALL,wcstoui64_l,(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale),(__s,__pend,__radix,__locale))
#else
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wcstoi64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),wcstoll,(__s,__pend,__radix))
__REDIRECT(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_wcstoui64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),wcstoull,(__s,__pend,__radix))
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_wcstoi64_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),wcstoll_l,(__s,__pend,__radix,__locale))
__REDIRECT(__LIBC,,__UINT64_TYPE__,__LIBCCALL,_wcstoui64_l,(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale),wcstoull_l,(__s,__pend,__radix,__locale))
#endif
#ifdef __CRT_DOS
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s);
__LIBC __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale);
#else /* __CRT_DOS */
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) { return _wcstoi64(__s,0,10); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstoi64_l(__s,0,10,__locale); }
#endif /* !__CRT_DOS */
#elif defined(__CRT_DOS)
__REDIRECT_DPB(__LIBC,,__INT64_TYPE__,__LIBCCALL,wcstoi64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),(__s,__pend,__radix))
__REDIRECT_DPB(__LIBC,,__INT64_TYPE__,__LIBCCALL,wcstoi64_l,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale),(__s,__pend,__radix,__locale))
__REDIRECT_DPB(__LIBC,,__UINT64_TYPE__,__LIBCCALL,wcstoui64,(wchar_t const *__restrict __s, wchar_t **__pend, int __radix),(__s,__pend,__radix))
__REDIRECT_DPB(__LIBC,,__UINT64_TYPE__,__LIBCCALL,wcstoui64_l,(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale),(__s ,__pend,__radix,__locale))
__REDIRECT_DPB(__LIBC,,__INT64_TYPE__,__LIBCCALL,wtoi64,(wchar_t const *__restrict __s),(__s))
__REDIRECT_DPB(__LIBC,,__INT64_TYPE__,__LIBCCALL,wtoi64_l,(wchar_t const *__restrict __s, __locale_t __locale),(__s,__locale))
#else
__LOCAL __INT64_TYPE__ (__LIBCCALL _wcstoi64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) { return (__INT64_TYPE__)__NAMESPACE_STD_SYM wcstoll(__s,__pend,__radix); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wcstoi64_l)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix, __locale_t __locale) { return (__INT64_TYPE__)_wcstoll_l(__s,__pend,__radix,__locale); }
__LOCAL __UINT64_TYPE__ (__LIBCCALL _wcstoui64)(wchar_t const *__restrict __s, wchar_t **__pend, int __radix) { return (__UINT64_TYPE__)__NAMESPACE_STD_SYM wcstoull(__s,__pend,__radix); }
__LOCAL __UINT64_TYPE__ (__LIBCCALL _wcstoui64_l)(wchar_t const *__restrict __s , wchar_t **__pend, int __radix, __locale_t __locale) { return (__UINT64_TYPE__)_wcstoull_l(__s,__pend,__radix,__locale); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64)(wchar_t const *__restrict __s) { return _wcstoi64(__s,0,10); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _wtoi64_l)(wchar_t const *__restrict __s, __locale_t __locale) { return _wcstoi64_l(__s,0,10,__locale); }
#endif
#endif /* !_WSTDLIB_DEFINED */

#ifndef _WSTDLIBP_DEFINED
#define _WSTDLIBP_DEFINED 1
#ifdef __CRT_DOS
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,wfullpath,(wchar_t *__restrict __abspath, wchar_t const *__restrict __path, __SIZE_TYPE__ __maxlen),(__abspath,__path,__maxlen))
#ifndef __wputenv_defined
#define __wputenv_defined 1
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,wputenv,(wchar_t const *__restrict __envstr),(__envstr))
#endif /* !__wputenv_defined */
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,void,__LIBCCALL,wmakepath,(wchar_t *__restrict __dst, wchar_t const *__drive, wchar_t const *__dir, wchar_t const *__file, wchar_t const *__ext),(__dst,__drive,__dir,__file,__ext))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,void,__LIBCCALL,wsearchenv,(wchar_t const *__file, wchar_t const *__varname, wchar_t *__restrict __dst),(__file,__varname,__dst))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,void,__LIBCCALL,wsplitpath,(wchar_t const *__restrict __abspath, wchar_t *__drive, wchar_t *__dir, wchar_t *__file, wchar_t *__ext),(__abspath,__drive,__dir,__file,__ext))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wmakepath_s,(wchar_t *__restrict __dst, __SIZE_TYPE__ __maxlen, wchar_t const *__drive, wchar_t const *__dir, wchar_t const *__file, wchar_t const *__ext),(__dst,__maxlen,__drive,__dir,__file,__ext))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wputenv_s,(wchar_t const *__name, wchar_t const *__val),(__name,__val))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wsearchenv_s,(wchar_t const *__file, wchar_t const *__varname, wchar_t *__restrict __dst, __SIZE_TYPE__ __maxlen),(__file,__varname,__dst,__maxlen))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wsplitpath_s,(wchar_t const *__restrict __abspath, wchar_t *__drive, __SIZE_TYPE__ __drivelen, wchar_t *__dir, __SIZE_TYPE__ __dirlen, wchar_t *__file, __SIZE_TYPE__ __filelen, wchar_t *__ext, __SIZE_TYPE__ __extlen),(__abspath,__drive,__drivelen,__dir,__dirlen,__file,__filelen,__ext,__extlen))
#endif /* __CRT_DOS */
#endif /* !_WSTDLIBP_DEFINED */

#ifndef _CRT_WSYSTEM_DEFINED
#define _CRT_WSYSTEM_DEFINED 1
#ifdef __CRT_DOS
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,wsystem,(wchar_t const *__restrict __cmd),(__cmd))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY int (__LIBCCALL Xwsystem)(wchar_t const *__restrict __cmd);
#endif /* __USE_EXCEPT */
#endif /* __CRT_DOS */
#endif /* !_CRT_WSYSTEM_DEFINED */


__SYSDECL_END

#endif /* !_PARTS_DOS_WSTDLIB_H */
