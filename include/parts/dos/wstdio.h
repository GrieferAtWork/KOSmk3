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
#ifndef _PARTS_DOS_WSTDIO_H
#define _PARTS_DOS_WSTDIO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <xlocale.h>
#ifndef _STDIO_H
#include <stdio.h>
#endif

__SYSDECL_BEGIN

#ifndef _WSTDIO_DEFINED
#define _WSTDIO_DEFINED 1

#ifdef __USE_KOS_STDEXT
#define __PRINTF_RETURN_TYPE __ssize_t
#else
#define __PRINTF_RETURN_TYPE int
#endif

#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef WEOF
#if __SIZEOF_WCHAR_T__ == 4
#   define WEOF             0xffffffffu
#else
#   define WEOF    (wint_t)(0xffff)
#endif
#endif /* !WEOF */

#ifndef __std_wprintf_defined
#define __std_wprintf_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_LIBC_WPRINTF(2,3) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__ATTR_CDECL fwprintf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC __ATTR_LIBC_WPRINTF(2,0) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__LIBCCALL vfwprintf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __ATTR_LIBC_WPRINTF(1,2) __NONNULL((1)) __PRINTF_RETURN_TYPE (__ATTR_CDECL wprintf)(wchar_t const *__restrict __format, ...);
__LIBC __ATTR_LIBC_WPRINTF(1,0) __NONNULL((1)) __PRINTF_RETURN_TYPE (__LIBCCALL vwprintf)(wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __ATTR_LIBC_WSCANF(2,3) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__ATTR_CDECL fwscanf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC __ATTR_LIBC_WSCANF(1,2) __NONNULL((1)) __PRINTF_RETURN_TYPE (__ATTR_CDECL wscanf)(wchar_t const *__restrict __format, ...);
__LIBC __ATTR_LIBC_WSCANF(2,3) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__ATTR_CDECL swscanf)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...);
#if defined(__DOS_COMPAT__) || defined(__PE__)
__VREDIRECT(__LIBC,__ATTR_LIBC_WPRINTF(3,4) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,swprintf,(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...),_vswprintf_c,(__buf,__buflen,__format),__format)
__REDIRECT(__LIBC,__ATTR_LIBC_WPRINTF(3,0) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__LIBCCALL,vswprintf,(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args),_vswprintf_c,(__buf,__buflen,__format,__args))
#else
__LIBC __ATTR_LIBC_WPRINTF(3,4) __NONNULL((1,3)) __PRINTF_RETURN_TYPE (__ATTR_CDECL swprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
__LIBC __ATTR_LIBC_WPRINTF(3,0) __NONNULL((1,3)) __PRINTF_RETURN_TYPE (__LIBCCALL vswprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args);
#endif
__NAMESPACE_STD_END
#endif /* !__std_wprintf_defined */

#ifndef __std_vwscanf_defined
#define __std_vwscanf_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __ATTR_LIBC_WSCANF(2,0) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__LIBCCALL vfwscanf)(__FILE *__restrict __stream, wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __WUNUSED __ATTR_LIBC_WSCANF(1,0) __NONNULL((1)) __PRINTF_RETURN_TYPE (__LIBCCALL vwscanf)(wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __WUNUSED __ATTR_LIBC_WSCANF(2,0) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__LIBCCALL vswscanf)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __builtin_va_list __arg);
__NAMESPACE_STD_END
#endif /* !__std_vwscanf_defined */

#ifndef __CXX_SYSTEM_HEADER
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
#ifndef __vwscanf_defined
#define __vwscanf_defined 1
__NAMESPACE_STD_USING(vfwscanf)
__NAMESPACE_STD_USING(vwscanf)
__NAMESPACE_STD_USING(vswscanf)
#endif /* !__vwscanf_defined */
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __CRT_DOS
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,wfsopen,(wchar_t const *__file, wchar_t const *__mode, int __shflag),(__file,__mode,__shflag))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,wfopen,(wchar_t const *__file, wchar_t const *__mode),(__file,__mode))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,wfreopen,(wchar_t const *__file, wchar_t const *__mode, FILE *__oldfile),(__file,__mode,__oldfile))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wfopen_s,(FILE **__pfile, wchar_t const *__file, wchar_t const *__mode),(__pfile,__file,__mode))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wfreopen_s,(FILE **__pfile, wchar_t const *__file, wchar_t const *__mode, FILE *__oldfile),(__pfile,__file,__mode,__oldfile))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,wfdopen,(__fd_t __fd, wchar_t const *__mode),(__fd,__mode))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,FILE *,__LIBCCALL,wpopen,(wchar_t const *__command, wchar_t const *__mode),(__command,__mode))
#endif /* __CRT_DOS */

/* Get wide character functions */
__REDIRECT(__LIBC,,wint_t,__LIBCCALL,_fgetwchar,(void),getwchar,())
#ifdef __DOS_COMPAT__
__LIBC __NONNULL((1)) wint_t (__LIBCCALL _fgetwc_nolock)(FILE *__restrict __file);
#else
__REDIRECT(__LIBC,__NONNULL((1)),wint_t,__LIBCCALL,_fgetwc_nolock,(FILE *__restrict __file),fgetwc_unlocked,(__file))
#endif
#ifndef __std_getwchar_defined
#define __std_getwchar_defined 1
__NAMESPACE_STD_BEGIN
__LIBC wint_t (__LIBCCALL getwchar)(void);
__LIBC __NONNULL((1)) wint_t (__LIBCCALL fgetwc)(FILE *__restrict __file);
__LIBC __NONNULL((1)) wint_t (__LIBCCALL getwc)(FILE *__restrict __file);
__NAMESPACE_STD_END
#endif /* !__std_getwchar_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __getwchar_defined
#define __getwchar_defined 1
__NAMESPACE_STD_USING(getwchar)
__NAMESPACE_STD_USING(fgetwc)
__NAMESPACE_STD_USING(getwc)
#endif /* !__getwchar_defined */
#endif /* !__CXX_SYSTEM_HEADER */

/* Put wide character functions */
__REDIRECT(__LIBC,,wint_t,__LIBCCALL,_fputwchar,(wchar_t __ch),putwchar,(__ch))
#ifdef __DOS_COMPAT__
__LIBC wint_t (__LIBCCALL _fputwc_nolock)(wchar_t __ch, FILE *__restrict __file);
#else
__REDIRECT(__LIBC,,wint_t,__LIBCCALL,_fputwc_nolock,(wchar_t __ch, FILE *__restrict __file),fputwc_unlocked,(__ch,__file))
#endif
#ifndef __std_putwchar_defined
#define __std_putwchar_defined 1
__NAMESPACE_STD_BEGIN
__LIBC wint_t (__LIBCCALL putwchar)(wchar_t __ch);
__LIBC __NONNULL((2)) wint_t (__LIBCCALL fputwc)(wchar_t __ch, FILE *__restrict __file);
__LIBC __NONNULL((2)) wint_t (__LIBCCALL putwc)(wchar_t __ch, FILE *__restrict __file);
__NAMESPACE_STD_END
#endif /* !__std_putwchar_defined */
#ifndef __putwchar_defined
#define __putwchar_defined 1
__NAMESPACE_STD_USING(putwchar)
__NAMESPACE_STD_USING(fputwc)
__NAMESPACE_STD_USING(putwc)
#endif /* !__putwchar_defined */

/* Unget character functions */
#ifdef __DOS_COMPAT__
__LIBC wint_t (__LIBCCALL _ungetwc_nolock)(wint_t __ch, FILE *__restrict __file);
#else
__REDIRECT(__LIBC,,wint_t,__LIBCCALL,_ungetwc_nolock,(wint_t __ch, FILE *__restrict __file),ungetwc_unlocked,(__ch,__file))
#endif
#ifndef __std_ungetwc_defined
#define __std_ungetwc_defined 1
__NAMESPACE_STD_BEGIN
__LIBC wint_t (__LIBCCALL ungetwc)(wint_t __ch, FILE *__restrict __file);
__NAMESPACE_STD_END
#endif /* !__std_ungetwc_defined */
#ifndef __ungetwc_defined
#define __ungetwc_defined 1
__NAMESPACE_STD_USING(ungetwc)
#endif /* !__ungetwc_defined */

/* Get wide string functions */
#ifdef __CRT_DOS
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,getws,(wchar_t *__restrict __buf),(__buf))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,getws_s,(wchar_t *__restrict __buf, size_t __buflen),(__buf,__buflen))
#endif /* __CRT_DOS */

#ifndef __std_fgetws_defined
#define __std_fgetws_defined 1
__NAMESPACE_STD_BEGIN
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__LIBC __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
/* In PE-mode, we don't export the size_t version */
#if defined(__PE__) || !defined(__CRT_KOS) || \
   (defined(__DOS_COMPAT__) || defined(__GLC_COMPAT__))
#ifdef __NO_ASMNAME
__LIBC __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream);
#define fgetws(buf,n,stream) fgetws(buf,(int)(n),stream)
#else /* __NO_ASMNAME */
__REDIRECT(__LIBC,,wchar_t *,__LIBCCALL,__pe_fgetws,(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream),fgetws,(__buf,__n,__stream))
__LOCAL __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream) { return __pe_fgetws(__buf,(int)__n,__stream); }
#endif /* !__NO_ASMNAME */
#else /* ... */
__REDIRECT(__LIBC,__NONNULL((1,3)),wchar_t *,__LIBCCALL,fgetws,(wchar_t *__restrict __buf, size_t __n, __FILE *__restrict __stream),fgetws_sz,(__buf,__n,__stream))
#endif /* !... */
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
#else /* __USE_KOS */
__LIBC __NONNULL((1,3)) wchar_t *(__LIBCCALL fgetws)(wchar_t *__restrict __buf, int __n, __FILE *__restrict __stream);
#endif /* !__USE_KOS */
__NAMESPACE_STD_END
#endif /* !__std_fgetws_defined */
#ifndef __fgetws_defined
#define __fgetws_defined 1
__NAMESPACE_STD_USING(fgetws)
#endif /* !__fgetws_defined */

/* Put wide string functions */
__REDIRECT_DPB(__LIBC,__NONNULL((1)),int,__LIBCCALL,putws,(wchar_t const *__restrict __str),(__str))
#ifndef __std_fputws_defined
#define __std_fputws_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __NONNULL((1,2)) int (__LIBCCALL fputws)(wchar_t const *__restrict __str, FILE *__restrict __file);
__NAMESPACE_STD_END
#endif /* !__std_fputws_defined */
#ifndef __fputws_defined
#define __fputws_defined 1
__NAMESPACE_STD_USING(fputws)
#endif /* !__fputws_defined */

__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _scwprintf)(wchar_t const *__restrict __format, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vscwprintf)(wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _swprintf_c)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vswprintf_c)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _fwprintf_p)(FILE *__restrict __file, wchar_t const *__restrict __format, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vfwprintf_p)(FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _wprintf_p)(wchar_t const *__restrict __format, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vwprintf_p)(wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _swprintf_p)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vswprintf_p)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _scwprintf_p)(wchar_t const *__restrict __format, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vscwprintf_p)(wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _wprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _wprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _wprintf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vwprintf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _fwprintf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vfwprintf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _fwprintf_p_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vfwprintf_p_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _fwprintf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vfwprintf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _swprintf_c_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vswprintf_c_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _swprintf_p_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vswprintf_p_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _swprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vswprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _scwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vscwprintf_l)(wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL _scwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL _vscwprintf_p_l)(wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#ifdef __DOS_COMPAT__
/* The following return an error, rather than the required size when the buffer is too small */
__LIBC __WARN_NONSTD(snwprintf) __PRINTF_RETURN_TYPE (__ATTR_CDECL _snwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
__LIBC __WARN_NONSTD(vsnwprintf) __PRINTF_RETURN_TYPE (__LIBCCALL _vsnwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __WARN_NONSTD(snwprintf) __PRINTF_RETURN_TYPE (__ATTR_CDECL _snwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __WARN_NONSTD(vsnwprintf) __PRINTF_RETURN_TYPE (__LIBCCALL _vsnwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __WARN_NONSTD(snwprintf) __PRINTF_RETURN_TYPE (__ATTR_CDECL _snwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, ...);
__LIBC __WARN_NONSTD(vsnwprintf) __PRINTF_RETURN_TYPE (__LIBCCALL _vsnwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __WARN_NONSTD(snwprintf) __PRINTF_RETURN_TYPE (__ATTR_CDECL _snwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __WARN_NONSTD(vsnwprintf) __PRINTF_RETURN_TYPE (__LIBCCALL _vsnwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#else /* __DOS_COMPAT__ */
/* Outside of DOS-mode, libc doesn't export DOS's broken wide-string printer functions, so we emulate them here. */
__LOCAL __WARN_NONSTD(vsnwprintf) __PRINTF_RETURN_TYPE (__LIBCCALL _vsnwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args) { size_t __result = vswprintf(__buf,__buflen,__format,__args); return __result < __buflen ? (__PRINTF_RETURN_TYPE)__result : -1; }
__LOCAL __WARN_NONSTD(vsnwprintf) __PRINTF_RETURN_TYPE (__LIBCCALL _vsnwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return _vsnwprintf(__buf,__buflen,__format,__args); }
__LOCAL __WARN_NONSTD(vsnwprintf) __PRINTF_RETURN_TYPE (__LIBCCALL _vsnwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __builtin_va_list __args) { return _vsnwprintf(__buf,__buflen < __maxlen ? __buflen : __maxlen,__format,__args); }
__LOCAL __WARN_NONSTD(vsnwprintf) __PRINTF_RETURN_TYPE (__LIBCCALL _vsnwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return _vsnwprintf_s(__buf,__buflen,__maxlen,__format,__args); }
__LOCAL __WARN_NONSTD(snwprintf) __PRINTF_RETURN_TYPE (__ATTR_CDECL _snwprintf)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = _vsnwprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __WARN_NONSTD(snwprintf) __PRINTF_RETURN_TYPE (__ATTR_CDECL _snwprintf_l)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vsnwprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __WARN_NONSTD(snwprintf) __PRINTF_RETURN_TYPE (__ATTR_CDECL _snwprintf_s)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = _vsnwprintf_s(__buf,__buflen,__maxlen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __WARN_NONSTD(snwprintf) __PRINTF_RETURN_TYPE (__ATTR_CDECL _snwprintf_s_l)(wchar_t *__restrict __buf, size_t __buflen, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vsnwprintf_s(__buf,__buflen,__maxlen,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !__DOS_COMPAT__ */


/* NOTE: ~safe~ functions are re-directed to the regular versions. (For the reason, see below) */
#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY_ALT(swscanf) __PRINTF_RETURN_TYPE (__ATTR_CDECL _snwscanf)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, ...);
__LIBC __PORT_DOSONLY_ALT(swscanf) __PRINTF_RETURN_TYPE (__ATTR_CDECL _snwscanf_l)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PRINTF_RETURN_TYPE (__ATTR_CDECL _fwscanf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PRINTF_RETURN_TYPE (__ATTR_CDECL _swscanf_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PRINTF_RETURN_TYPE (__ATTR_CDECL _wscanf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY_ALT(swscanf) __PRINTF_RETURN_TYPE (__ATTR_CDECL _snwscanf_s)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, ...);
__LIBC __PORT_DOSONLY_ALT(swscanf) __PRINTF_RETURN_TYPE (__ATTR_CDECL _snwscanf_s_l)(wchar_t const *__src, size_t __maxlen, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PRINTF_RETURN_TYPE (__ATTR_CDECL _fwscanf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PRINTF_RETURN_TYPE (__ATTR_CDECL _swscanf_s_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PRINTF_RETURN_TYPE (__ATTR_CDECL _wscanf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
#else /* __CRT_DOS */
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _fwscanf_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vfwscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _fwscanf_s_l)(FILE *__restrict __file, wchar_t const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vfwscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _swscanf_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vswscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _swscanf_s_l)(wchar_t const *__src, wchar_t const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vswscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _wscanf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vwscanf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _wscanf_s_l)(wchar_t const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vwscanf(__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !__CRT_DOS */

#ifdef __USE_DOS_SLIB
#ifdef __DOS_COMPAT__
__LIBC int (__ATTR_CDECL fwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vfwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL wprintf_s)(wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vwprintf_s)(wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL fwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vfwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL wscanf_s)(wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vwscanf_s)(wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL swprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vswprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL swscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...);
__LIBC int (__LIBCCALL vswscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __builtin_va_list __args);
#elif !defined(__NO_asmname)
__LIBC int (__ATTR_CDECL fwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) __ASMNAME("fwprintf");
__LIBC int (__LIBCCALL vfwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME("vfwprintf");
__LIBC int (__ATTR_CDECL wprintf_s)(wchar_t const *__restrict __format, ...) __ASMNAME("wprintf");
__LIBC int (__LIBCCALL vwprintf_s)(wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME("vwprintf");
__LIBC int (__ATTR_CDECL fwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) __ASMNAME("fwscanf");
__LIBC int (__LIBCCALL vfwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME("vfwscanf");
__LIBC int (__ATTR_CDECL wscanf_s)(wchar_t const *__restrict __format, ...) __ASMNAME("wscanf");
__LIBC int (__LIBCCALL vwscanf_s)(wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME("vwscanf");
__LIBC int (__ATTR_CDECL swprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) __ASMNAME("swprintf");
__LIBC int (__LIBCCALL vswprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME("vswprintf");
__LIBC int (__ATTR_CDECL swscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...) __ASMNAME("swscanf");
__LIBC int (__LIBCCALL vswscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __builtin_va_list __args) __ASMNAME("vswscanf");
#else /* ... */
__LOCAL int (__LIBCCALL vfwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __args) { return vfwprintf(__file,__format,__args); }
__LOCAL int (__LIBCCALL vwprintf_s)(wchar_t const *__restrict __format, __builtin_va_list __args) { return vwprintf(__format,__args); }
__LOCAL int (__LIBCCALL vfwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, __builtin_va_list __args) { return vfwscanf(__file,__format,__args); }
__LOCAL int (__LIBCCALL vwscanf_s)(wchar_t const *__restrict __format, __builtin_va_list __args) { return vwscanf(__format,__args); }
__LOCAL int (__LIBCCALL vswprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args) { return vswprintf(__buf,__buflen,__format,__args); }
__LOCAL int (__LIBCCALL vswscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __builtin_va_list __args) { return vswscanf(__src,__format,__args); }
__LOCAL int (__ATTR_CDECL fwprintf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vfwprintf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL wprintf_s)(wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vwprintf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL fwscanf_s)(FILE *__restrict __file, wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vfwscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL wscanf_s)(wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vwscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL swprintf_s)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vswprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL swscanf_s)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vswscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !... */
#endif /* __USE_DOS_SLIB */

#ifdef __CRT_DOS
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,wtmpnam,(wchar_t *__restrict __buf),(__buf))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wtmpnam_s,(wchar_t *__restrict __buf, size_t __buflen),(__buf,__buflen))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,wtempnam,(wchar_t const *__dir, wchar_t const *__pfx),(__dir,__pfx))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,wremove,(wchar_t const *__restrict __file),(__file))
#endif /* __CRT_DOS */

#ifdef __DOS_COMPAT__
/* Versions lacking the C standard mandated BUFLEN argument...
 * NOTE: Internally, these functions will link against `DOS$_swprintf' and `DOS$_vswprintf' */
__LIBC int (__LIBCCALL _vswprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL _swprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, ...);
#else /* __DOS_COMPAT__ */
/* Outside of PE-mode, wchar_t is 32 bits wide and `DOS$' isn't inserted before symbol names. */
/* libc doesn't export these superfluous and confusion version of swprintf.
 * (They're lacking the BUFLEN argument mandated by the C standard).
 * So instead, they're implemented as a hack. */
__LOCAL int (__LIBCCALL _vswprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __builtin_va_list __args) { return vswprintf(__buf,(size_t)-1,__format,__args); }
__LOCAL int (__ATTR_CDECL _swprintf)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, ...) { __builtin_va_list __args; int __result; __builtin_va_start(__args,__format); __result = _vswprintf(__buf,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !__DOS_COMPAT__ */

__LIBC int (__LIBCCALL __vswprintf_l)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL __swprintf_l)(wchar_t *__restrict __buf, wchar_t const *__restrict __format, __locale_t __locale, ...);
__VREDIRECT(__LIBC,,int,__ATTR_CDECL,_swprintf_l,(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, ...),_swprintf_c_l,_vswprintf_c_l,(__buf,__buflen,__format,__locale),__locale)
__REDIRECT(__LIBC,,int,__LIBCCALL,_vswprintf_l,(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),_vswprintf_c_l,(__buf,__buflen,__format,__locale,__args))

#define getwchar()            fgetwc(stdin)
#define putwchar(c)           fputwc((c),stdout)
#define getwc(file)           fgetwc(file)
#define putwc(c,file)         fputwc(c,file)
#define _putwc_nolock(c,file) _fputwc_nolock(c,file)
#define _getwc_nolock(file)   _fgetwc_nolock(file)

#define _getwchar_nolock()    _getwc_nolock(stdin)
#define _putwchar_nolock(c)   _putwc_nolock((c),stdout)

#undef __PRINTF_RETURN_TYPE
#endif  /* _WSTDIO_DEFINED */

__SYSDECL_END

#endif /* !_PARTS_DOS_WSTDIO_H */
