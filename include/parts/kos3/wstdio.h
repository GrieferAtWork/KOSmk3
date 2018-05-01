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
#ifndef _PARTS_KOS3_WSTDIO_H
#define _PARTS_KOS3_WSTDIO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <xlocale.h>

#if defined(__CC__) && !defined(__KERNEL__)
__SYSDECL_BEGIN

#ifdef __USE_KOS_STDEXT
#define __PRINTF_RETURN_TYPE  __ssize_t
#else
#define __PRINTF_RETURN_TYPE  int
#endif

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif

__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1)),int,__LIBCCALL,wremove,(wchar_t const *__restrict __file),(__file))
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,wrename,(wchar_t const *__oldname, wchar_t const *__newname),(__oldname,__newname))
__REDIRECT_UFSDPA(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,wtmpnam,(wchar_t *__restrict __buf),(__buf))
#ifdef __CRT_KOS
#ifdef __USE_KOS
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,wremoveat,(__fd_t __dfd, wchar_t const *__filename),(__fd,__filename))
#endif /* __USE_KOS */
#ifdef __USE_ATFILE
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,wrenameat,(int __oldfd, wchar_t const *__old, int __newfd, wchar_t const *__new),(__oldfd,__old,__newfd,__new))
#ifdef __USE_KOS
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,wfrenameat,(int __oldfd, wchar_t const *__old, int __newfd, wchar_t const *__new, int __flags),(__oldfd,__old,__newfd,__new,__flags))
#endif /* __USE_KOS */
#endif /* __USE_ATFILE */
#endif /* __CRT_KOS */

#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwremove)(wchar_t const *__restrict __file);
__LIBC __PORT_KOSONLY __NONNULL((1,2)) void (__LIBCCALL Xwrename)(wchar_t const *__oldname, wchar_t const *__newname);
#ifdef __USE_KOS
__LIBC __PORT_KOSONLY void (__LIBCCALL Xwremoveat)(__fd_t __dfd, wchar_t const *__filename);
#endif /* __USE_KOS */
#ifdef __USE_ATFILE
__LIBC __PORT_KOSONLY void (__LIBCCALL Xwrenameat)(int __oldfd, wchar_t const *__old, int __newfd, wchar_t const *__new);
#ifdef __USE_KOS
__LIBC __PORT_KOSONLY void (__LIBCCALL Xwfrenameat)(int __oldfd, wchar_t const *__old, int __newfd, wchar_t const *__new, int __flags);
#endif /* __USE_KOS */
#endif /* __USE_ATFILE */
#endif /* __USE_EXCEPT */

#ifdef __DOS_COMPAT__
__VREDIRECT(__LIBC,__PORT_DOSONLY __WUNUSED __ATTR_LIBC_WSCANF(2,4) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fwscanf_l,(__FILE *__restrict __stream, wchar_t const *__restrict __format, __locale_t __locale, ...),_fwscanf_l,_vfwscanf_l,(__stream,__format,__locale),__locale)
__VREDIRECT(__LIBC,__PORT_DOSONLY __WUNUSED __ATTR_LIBC_WSCANF(1,3) __NONNULL((1)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,wscanf_l,(wchar_t const *__restrict __format, __locale_t __locale, ...),_wscanf_l,_vwscanf_l,(__format,__locale),__locale)
__VREDIRECT(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_LIBC_WSCANF(2,4) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,swscanf_l,(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __locale_t __locale, ...),_swscanf_l,_vswscanf_l,(__src,__format,__locale),__locale)
#elif defined(__CRT_KOS)
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_LIBC_WSCANF(2,4) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__ATTR_CDECL fwscanf_l)(__FILE *__restrict __stream, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_DOSONLY __WUNUSED __ATTR_LIBC_WSCANF(1,3) __NONNULL((1)) __PRINTF_RETURN_TYPE (__ATTR_CDECL wscanf_l)(wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_LIBC_WSCANF(2,4) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__ATTR_CDECL swscanf_l)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __locale_t __locale, ...);
#endif

__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_WPRINTF(1,3) __NONNULL((1)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,wprintf_l,(wchar_t const *__restrict __format, __locale_t __locale, ...),vwprintf_l,(__format,__locale),__locale)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_WPRINTF(2,4) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fwprintf_l,(__FILE *__restrict __stream, wchar_t const *__restrict __format, __locale_t __locale, ...),vfwprintf_l,(__stream,__format,__locale),__locale)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_WPRINTF_P(1,2) __NONNULL((1)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,wprintf_p,(wchar_t const *__restrict __format, ...),vwprintf_p,(__format),__format)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_WPRINTF_P(2,3) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fwprintf_p,(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...),vfwprintf_p,(__stream,__format),__format)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_WPRINTF_P(1,3) __NONNULL((1)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,wprintf_p_l,(wchar_t const *__restrict __format, __locale_t __locale, ...),vwprintf_p_l,(__format,__locale),__locale)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_WPRINTF_P(2,4) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fwprintf_p_l,(__FILE *__restrict __stream, wchar_t const *__restrict __format, __locale_t __locale, ...),vfwprintf_p_l,(__stream,__format,__locale),__locale)
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_WPRINTF(1,0) __NONNULL((1)),__PRINTF_RETURN_TYPE,__LIBCCALL,vwprintf_l,(wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_WPRINTF(2,0) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__LIBCCALL,vfwprintf_l,(__FILE *__restrict __stream, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__stream,__format,__locale,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_WPRINTF_P(1,0) __NONNULL((1)),__PRINTF_RETURN_TYPE,__LIBCCALL,vwprintf_p,(wchar_t const *__restrict __format, __builtin_va_list __args),(__format,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_WPRINTF_P(2,0) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__LIBCCALL,vfwprintf_p,(__FILE *__restrict __stream, wchar_t const *__restrict __format, __builtin_va_list __args),(__stream,__format,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_WPRINTF_P(1,0) __NONNULL((1)),__PRINTF_RETURN_TYPE,__LIBCCALL,vwprintf_p_l,(wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_WPRINTF_P(2,0) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__LIBCCALL,vfwprintf_p_l,(__FILE *__restrict __stream, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__stream,__format,__locale,__args))

#ifdef __CRT_KOS
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_LIBC_WSCANF(2,0) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__LIBCCALL vfwscanf_l)(__FILE *__restrict __stream, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_LIBC_WSCANF(1,0) __NONNULL((1)) __PRINTF_RETURN_TYPE (__LIBCCALL vwscanf_l)(wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_LIBC_WSCANF(2,0) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__LIBCCALL vswscanf_l)(wchar_t const *__restrict __src, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __arg);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_LIBC_WPRINTF(2,0) __PRINTF_RETURN_TYPE (__LIBCCALL vwdprintf)(__fd_t __fd, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_LIBC_WPRINTF(2,3) __PRINTF_RETURN_TYPE (__ATTR_CDECL wdprintf)(__fd_t __fd, wchar_t const *__restrict __format, ...);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_LIBC_WPRINTF(2,0) __PRINTF_RETURN_TYPE (__LIBCCALL vwdprintf_l)(__fd_t __fd, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_LIBC_WPRINTF(2,4) __PRINTF_RETURN_TYPE (__ATTR_CDECL wdprintf_l)(__fd_t __fd, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_LIBC_WPRINTF_P(2,0) __PRINTF_RETURN_TYPE (__LIBCCALL vwdprintf_p)(__fd_t __fd, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_LIBC_WPRINTF_P(2,3) __PRINTF_RETURN_TYPE (__ATTR_CDECL wdprintf_p)(__fd_t __fd, wchar_t const *__restrict __format, ...);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_LIBC_WPRINTF_P(2,0) __PRINTF_RETURN_TYPE (__LIBCCALL vwdprintf_p_l)(__fd_t __fd, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __NONNULL((2)) __ATTR_LIBC_WPRINTF_P(2,4) __PRINTF_RETURN_TYPE (__ATTR_CDECL wdprintf_p_l)(__fd_t __fd, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(2,3) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__ATTR_CDECL fwprintf_p)(__FILE *__restrict __stream, wchar_t const *__restrict __format, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(2,0) __NONNULL((1,2)) __PRINTF_RETURN_TYPE (__LIBCCALL vfwprintf_p)(__FILE *__restrict __stream, wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(1,2) __NONNULL((1)) __PRINTF_RETURN_TYPE (__ATTR_CDECL wprintf_p)(wchar_t const *__restrict __format, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(1,0) __NONNULL((1)) __PRINTF_RETURN_TYPE (__LIBCCALL vwprintf_p)(wchar_t const *__restrict __format, __builtin_va_list __arg);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(3,4) __NONNULL((1,3)) __PRINTF_RETURN_TYPE (__ATTR_CDECL swprintf_p)(wchar_t *__restrict __buf, __size_t __buflen, wchar_t const *__restrict __format, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(3,0) __NONNULL((1,3)) __PRINTF_RETURN_TYPE (__LIBCCALL vswprintf_p)(wchar_t *__restrict __buf, __size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args);
#endif /* __CRT_KOS */

#if 0 /* TODO */
/* Wide character STDIO support */
__LIBC wint_t __LIBCCALL getwchar16(void);
__LIBC wint_t __LIBCCALL getwchar32(void);
__LIBC wint_t __LIBCCALL Xgetwchar16(void);
__LIBC wint_t __LIBCCALL Xgetwchar32(void);
__LIBC wint_t __LIBCCALL getwchar16_unlocked(void);
__LIBC wint_t __LIBCCALL getwchar32_unlocked(void);
__LIBC wint_t __LIBCCALL Xgetwchar16_unlocked(void);
__LIBC wint_t __LIBCCALL Xgetwchar32_unlocked(void);
__LIBC wint_t __LIBCCALL fgetwc16(__FILE *__restrict stream);
__LIBC wint_t __LIBCCALL fgetwc32(__FILE *__restrict stream);
__LIBC wint_t __LIBCCALL Xfgetwc16(__FILE *__restrict stream);
__LIBC wint_t __LIBCCALL Xfgetwc32(__FILE *__restrict stream);
__LIBC wint_t __LIBCCALL fgetwc16_unlocked(__FILE *__restrict stream);
__LIBC wint_t __LIBCCALL fgetwc32_unlocked(__FILE *__restrict stream);
__LIBC wint_t __LIBCCALL Xfgetwc16_unlocked(__FILE *__restrict stream);
__LIBC wint_t __LIBCCALL Xfgetwc32_unlocked(__FILE *__restrict stream);

__LIBC wint_t __LIBCCALL fputwc16(char16_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL fputwc32(char32_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL Xfputwc16(char16_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL Xfputwc32(char32_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL fputwc16_unlocked(char16_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL fputwc32_unlocked(char32_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL Xfputwc16_unlocked(char16_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL Xfputwc32_unlocked(char32_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL putwchar16(char16_t wc);
__LIBC wint_t __LIBCCALL putwchar32(char32_t wc);
__LIBC wint_t __LIBCCALL Xputwchar16(char16_t wc);
__LIBC wint_t __LIBCCALL Xputwchar32(char32_t wc);
__LIBC wint_t __LIBCCALL putwchar16_unlocked(char16_t wc);
__LIBC wint_t __LIBCCALL putwchar32_unlocked(char32_t wc);
__LIBC wint_t __LIBCCALL Xputwchar16_unlocked(char16_t wc);
__LIBC wint_t __LIBCCALL Xputwchar32_unlocked(char32_t wc);

__LIBC wint_t __LIBCCALL ungetwc16(wint_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL ungetwc32(wint_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL ungetwc16_unlocked(wint_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL ungetwc32_unlocked(wint_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL Xungetwc16(wint_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL Xungetwc32(wint_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL Xungetwc16_unlocked(wint_t wc, __FILE *__restrict stream);
__LIBC wint_t __LIBCCALL Xungetwc32_unlocked(wint_t wc, __FILE *__restrict stream);

__LIBC char16_t *__LIBCCALL fgetws16(char16_t *__restrict ws, __size_t __count, __FILE *__restrict stream);
__LIBC char32_t *__LIBCCALL fgetws32(char32_t *__restrict ws, __size_t __count, __FILE *__restrict stream);
__LIBC char16_t *__LIBCCALL fgetws16_int(char16_t *__restrict ws, int __count, __FILE *__restrict stream);
__LIBC char32_t *__LIBCCALL fgetws32_int(char32_t *__restrict ws, int __count, __FILE *__restrict stream);
__LIBC char16_t *__LIBCCALL fgetws16_unlocked(char16_t *__restrict ws, __size_t __count, __FILE *__restrict stream);
__LIBC char32_t *__LIBCCALL fgetws32_unlocked(char32_t *__restrict ws, __size_t __count, __FILE *__restrict stream);
__LIBC char16_t *__LIBCCALL fgetws16_int_unlocked(char16_t *__restrict ws, int __count, __FILE *__restrict stream);
__LIBC char32_t *__LIBCCALL fgetws32_int_unlocked(char32_t *__restrict ws, int __count, __FILE *__restrict stream);
__LIBC __ATTR_RETNONNULL char16_t *__LIBCCALL Xfgetws16(char16_t *__restrict ws, __size_t __count, __FILE *__restrict stream);
__LIBC __ATTR_RETNONNULL char32_t *__LIBCCALL Xfgetws32(char32_t *__restrict ws, __size_t __count, __FILE *__restrict stream);
__LIBC __ATTR_RETNONNULL char16_t *__LIBCCALL Xfgetws16_int(char16_t *__restrict ws, int __count, __FILE *__restrict stream);
__LIBC __ATTR_RETNONNULL char32_t *__LIBCCALL Xfgetws32_int(char32_t *__restrict ws, int __count, __FILE *__restrict stream);
__LIBC __ATTR_RETNONNULL char16_t *__LIBCCALL Xfgetws16_unlocked(char16_t *__restrict ws, __size_t __count, __FILE *__restrict stream);
__LIBC __ATTR_RETNONNULL char32_t *__LIBCCALL Xfgetws32_unlocked(char32_t *__restrict ws, __size_t __count, __FILE *__restrict stream);
__LIBC __ATTR_RETNONNULL char16_t *__LIBCCALL Xfgetws16_int_unlocked(char16_t *__restrict ws, int __count, __FILE *__restrict stream);
__LIBC __ATTR_RETNONNULL char32_t *__LIBCCALL Xfgetws32_int_unlocked(char32_t *__restrict ws, int __count, __FILE *__restrict stream);

__LIBC __ssize_t __LIBCCALL fputws16(char16_t const *__restrict ws, __FILE *__restrict stream);
__LIBC __ssize_t __LIBCCALL fputws32(char32_t const *__restrict ws, __FILE *__restrict stream);
__LIBC __ssize_t __LIBCCALL fputws16_unlocked(char16_t const *__restrict ws, __FILE *__restrict stream);
__LIBC __ssize_t __LIBCCALL fputws32_unlocked(char32_t const *__restrict ws, __FILE *__restrict stream);
__LIBC __size_t __LIBCCALL Xfputws16(char16_t const *__restrict ws, __FILE *__restrict stream);
__LIBC __size_t __LIBCCALL Xfputws32(char32_t const *__restrict ws, __FILE *__restrict stream);
__LIBC __size_t __LIBCCALL Xfputws16_unlocked(char16_t const *__restrict ws, __FILE *__restrict stream);
__LIBC __size_t __LIBCCALL Xfputws32_unlocked(char32_t const *__restrict ws, __FILE *__restrict stream);

#endif

#undef __PRINTF_RETURN_TYPE

__SYSDECL_END
#endif /* __CC__ && !__KERNEL__ */

#endif /* !_PARTS_KOS3_WSTDIO_H */
