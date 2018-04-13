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
#include <xlocale.h>

#if defined(__CC__) && !defined(__KERNEL__)
__SYSDECL_BEGIN

#ifdef __USE_KOS
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
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(3,4) __NONNULL((1,3)) __PRINTF_RETURN_TYPE (__ATTR_CDECL swprintf_p)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(3,0) __NONNULL((1,3)) __PRINTF_RETURN_TYPE (__LIBCCALL vswprintf_p)(wchar_t *__restrict __buf, size_t __buflen, wchar_t const *__restrict __format, __builtin_va_list __args);
#endif /* __CRT_KOS */

#undef __PRINTF_RETURN_TYPE

__SYSDECL_END
#endif /* __CC__ && !__KERNEL__ */

#endif /* !_PARTS_KOS3_WSTDIO_H */
