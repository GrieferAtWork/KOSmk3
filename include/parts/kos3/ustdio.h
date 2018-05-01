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
#ifndef _PARTS_KOS3_USTDIO_H
#define _PARTS_KOS3_USTDIO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <xlocale.h>

#ifndef __KERNEL__
__SYSDECL_BEGIN

#ifdef __CRT_KOS
#ifdef __USE_KOS_STDEXT
#define __PRINTF_RETURN_TYPE  __ssize_t
#else
#define __PRINTF_RETURN_TYPE  int
#endif

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w16remove,(char16_t const *__restrict __file),wremove,(__file))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,w32remove,(char32_t const *__restrict __file),wremove,(__file))
__REDIRECT_EXCEPT_UFSDPW16_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w16rename,(char16_t const *__oldname, char16_t const *__newname),wrename,(__oldname,__newname))
__REDIRECT_EXCEPT_UFSW32_XVOID  (__LIBC,__PORT_KOSONLY __NONNULL((1,2)),int,__LIBCCALL,w32rename,(char32_t const *__oldname, char32_t const *__newname),wrename,(__oldname,__newname))
__REDIRECT_UFSDPW16(__LIBC,__PORT_KOSONLY,char16_t *,__LIBCCALL,w16tmpnam,(char16_t *__restrict __buf),wtmpnam,(__buf))
__REDIRECT_UFSW32  (__LIBC,__PORT_KOSONLY,char32_t *,__LIBCCALL,w32tmpnam,(char32_t *__restrict __buf),wtmpnam,(__buf))
#ifdef __CRT_KOS
#ifdef __USE_KOS
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,w16removeat,(__fd_t __dfd, char16_t const *__filename),wremoveat,(__fd,__filename))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,w32removeat,(__fd_t __dfd, char32_t const *__filename),wremoveat,(__fd,__filename))
#endif /* __USE_KOS */
#ifdef __USE_ATFILE
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,w16renameat,(int __oldfd, char16_t const *__old, int __newfd, char16_t const *__new),wrenameat,(__oldfd,__old,__newfd,__new))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,w32renameat,(int __oldfd, char32_t const *__old, int __newfd, char32_t const *__new),wrenameat,(__oldfd,__old,__newfd,__new))
#ifdef __USE_KOS
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,w16frenameat,(int __oldfd, char16_t const *__old, int __newfd, char16_t const *__new, int __flags),wfrenameat,(__oldfd,__old,__newfd,__new,__flags))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,w32frenameat,(int __oldfd, char32_t const *__old, int __newfd, char32_t const *__new, int __flags),wfrenameat,(__oldfd,__old,__newfd,__new,__flags))
#endif /* __USE_KOS */
#endif /* __USE_ATFILE */
#endif /* __CRT_KOS */

#ifdef __USE_EXCEPT
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw16remove,(char16_t const *__restrict __file),Xwremove,(__file))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),__LIBCCALL,Xw32remove,(char32_t const *__restrict __file),Xwremove,(__file))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw16rename,(char16_t const *__oldname, char16_t const *__newname),Xwrename,(__oldname,__newname))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),__LIBCCALL,Xw32rename,(char32_t const *__oldname, char32_t const *__newname),Xwrename,(__oldname,__newname))
#ifdef __USE_KOS
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xw16removeat,(__fd_t __dfd, char16_t const *__filename),Xwremoveat,(__dfd,__filename))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xw32removeat,(__fd_t __dfd, char32_t const *__filename),Xwremoveat,(__dfd,__filename))
#endif /* __USE_KOS */
#ifdef __USE_ATFILE
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xw16renameat,(int __oldfd, char16_t const *__old, int __newfd, char16_t const *__new),Xwrenameat,(__oldfd,__old,__newfd,__new))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xw32renameat,(int __oldfd, char32_t const *__old, int __newfd, char32_t const *__new),Xwrenameat,(__oldfd,__old,__newfd,__new))
#ifdef __USE_KOS
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xw16frenameat,(int __oldfd, char16_t const *__old, int __newfd, char16_t const *__new, int __flags),Xwfrenameat,(__oldfd,__old,__newfd,__new,__flags))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xw32frenameat,(int __oldfd, char32_t const *__old, int __newfd, char32_t const *__new, int __flags),Xwfrenameat,(__oldfd,__old,__newfd,__new,__flags))
#endif /* __USE_KOS */
#endif /* __USE_ATFILE */
#endif /* __USE_EXCEPT */


__VREDIRECT_DPW16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF(1,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,w16printf_l,(char16_t const *__restrict __format, __locale_t __locale, ...),wprintf_l,vwprintf_l,(__format,__locale),__locale)
__VREDIRECT_W32  (__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF(1,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,w32printf_l,(char32_t const *__restrict __format, __locale_t __locale, ...),wprintf_l,vwprintf_l,(__format,__locale),__locale)
__VREDIRECT_DPW16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF(2,4),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fw16printf_l,(__FILE *__restrict __stream, char16_t const *__restrict __format, __locale_t __locale, ...),fwprintf_l,vfwprintf_l,(__stream,__format,__locale),__locale)
__VREDIRECT_W32  (__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF(2,4),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fw32printf_l,(__FILE *__restrict __stream, char32_t const *__restrict __format, __locale_t __locale, ...),fwprintf_l,vfwprintf_l,(__stream,__format,__locale),__locale)
__VREDIRECT_DPW16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF_P(1,2),__PRINTF_RETURN_TYPE,__ATTR_CDECL,w16printf_p,(char16_t const *__restrict __format, ...),wprintf_p,vwprintf_p,(__format),__format)
__VREDIRECT_W32  (__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF_P(1,2),__PRINTF_RETURN_TYPE,__ATTR_CDECL,w32printf_p,(char32_t const *__restrict __format, ...),wprintf_p,vwprintf_p,(__format),__format)
__VREDIRECT_DPW16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF_P(2,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fw16printf_p,(__FILE *__restrict __stream, char16_t const *__restrict __format, ...),fwprintf_p,vfwprintf_p,(__stream,__format),__format)
__VREDIRECT_W32  (__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF_P(2,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fw32printf_p,(__FILE *__restrict __stream, char32_t const *__restrict __format, ...),fwprintf_p,vfwprintf_p,(__stream,__format),__format)
__VREDIRECT_DPW16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF_P(1,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,w16printf_p_l,(char16_t const *__restrict __format, __locale_t __locale, ...),wprintf_p_l,vwprintf_p_l,(__format,__locale),__locale)
__VREDIRECT_W32  (__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF_P(1,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,w32printf_p_l,(char32_t const *__restrict __format, __locale_t __locale, ...),wprintf_p_l,vwprintf_p_l,(__format,__locale),__locale)
__VREDIRECT_DPW16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF_P(2,4),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fw16printf_p_l,(__FILE *__restrict __stream, char16_t const *__restrict __format, __locale_t __locale, ...),fwprintf_p_l,vfwprintf_p_l,(__stream,__format,__locale),__locale)
__VREDIRECT_W32  (__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF_P(2,4),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fw32printf_p_l,(__FILE *__restrict __stream, char32_t const *__restrict __format, __locale_t __locale, ...),fwprintf_p_l,vfwprintf_p_l,(__stream,__format,__locale),__locale)
__REDIRECT_DPW16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw16printf_l,(char16_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vwprintf_l,(__format,__locale,__args))
__REDIRECT_W32  (__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw32printf_l,(char32_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vwprintf_l,(__format,__locale,__args))
__REDIRECT_DPW16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfw16printf_l,(__FILE *__restrict __stream, char16_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vfwprintf_l,(__stream,__format,__locale,__args))
__REDIRECT_W32  (__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfw32printf_l,(__FILE *__restrict __stream, char32_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vfwprintf_l,(__stream,__format,__locale,__args))
__REDIRECT_DPW16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF_P(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw16printf_p,(char16_t const *__restrict __format, __builtin_va_list __args),vwprintf_p,(__format,__args))
__REDIRECT_W32  (__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF_P(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw32printf_p,(char32_t const *__restrict __format, __builtin_va_list __args),vwprintf_p,(__format,__args))
__REDIRECT_DPW16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF_P(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfw16printf_p,(__FILE *__restrict __stream, char16_t const *__restrict __format, __builtin_va_list __args),vfwprintf_p,(__stream,__format,__args))
__REDIRECT_W32  (__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF_P(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfw32printf_p,(__FILE *__restrict __stream, char32_t const *__restrict __format, __builtin_va_list __args),vfwprintf_p,(__stream,__format,__args))
__REDIRECT_DPW16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF_P(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw16printf_p_l,(char16_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vwprintf_p_l,(__format,__locale,__args))
__REDIRECT_W32  (__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF_P(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw32printf_p_l,(char32_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vwprintf_p_l,(__format,__locale,__args))
__REDIRECT_DPW16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF_P(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfw16printf_p_l,(__FILE *__restrict __stream, char16_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vfwprintf_p_l,(__stream,__format,__locale,__args))
__REDIRECT_W32  (__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF_P(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfw32printf_p_l,(__FILE *__restrict __stream, char32_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vfwprintf_p_l,(__stream,__format,__locale,__args))

__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw16dprintf,(__fd_t __fd, char16_t const *__restrict __format, __builtin_va_list __args),vwdprintf,(__fd,__format,__args))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw32dprintf,(__fd_t __fd, char32_t const *__restrict __format, __builtin_va_list __args),vwdprintf,(__fd,__format,__args))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw16dprintf_l,(__fd_t __fd, char16_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vwdprintf_l,(__fd,__format,__locale,__args))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw32dprintf_l,(__fd_t __fd, char32_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vwdprintf_l,(__fd,__format,__locale,__args))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF_P(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw16dprintf_p,(__fd_t __fd, char16_t const *__restrict __format, __builtin_va_list __args),vwdprintf_p,(__fd,__format,__args))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF_P(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw32dprintf_p,(__fd_t __fd, char32_t const *__restrict __format, __builtin_va_list __args),vwdprintf_p,(__fd,__format,__args))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF_P(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw16dprintf_p_l,(__fd_t __fd, char16_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vwdprintf_p_l,(__fd,__format,__locale,__args))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF_P(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vw32dprintf_p_l,(__fd_t __fd, char32_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vwdprintf_p_l,(__fd,__format,__locale,__args))
__VREDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF(2,3),__PRINTF_RETURN_TYPE,__LIBCCALL,w16dprintf,(__fd_t __fd, char16_t const *__restrict __format, ...),wdprintf,vwdprintf,(__fd,__format),__format)
__VREDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF(2,3),__PRINTF_RETURN_TYPE,__LIBCCALL,w32dprintf,(__fd_t __fd, char32_t const *__restrict __format, ...),wdprintf,vwdprintf,(__fd,__format),__format)
__VREDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF(2,4),__PRINTF_RETURN_TYPE,__LIBCCALL,w16dprintf_l,(__fd_t __fd, char16_t const *__restrict __format, __locale_t __locale, ...),wdprintf_l,vwdprintf_l,(__fd,__format,__locale),__locale)
__VREDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF(2,4),__PRINTF_RETURN_TYPE,__LIBCCALL,w32dprintf_l,(__fd_t __fd, char32_t const *__restrict __format, __locale_t __locale, ...),wdprintf_l,vwdprintf_l,(__fd,__format,__locale),__locale)
__VREDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF_P(2,3),__PRINTF_RETURN_TYPE,__LIBCCALL,w16dprintf_p,(__fd_t __fd, char16_t const *__restrict __format, ...),wdprintf_p,vwdprintf_p,(__fd,__format),__format)
__VREDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF_P(2,3),__PRINTF_RETURN_TYPE,__LIBCCALL,w32dprintf_p,(__fd_t __fd, char32_t const *__restrict __format, ...),wdprintf_p,vwdprintf_p,(__fd,__format),__format)
__VREDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W16PRINTF_P(2,4),__PRINTF_RETURN_TYPE,__LIBCCALL,w16dprintf_p_l,(__fd_t __fd, char16_t const *__restrict __format, __locale_t __locale, ...),wdprintf_p_l,vwdprintf_p_l,(__fd,__format,__locale),__locale)
__VREDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_LIBC_W32PRINTF_P(2,4),__PRINTF_RETURN_TYPE,__LIBCCALL,w32dprintf_p_l,(__fd_t __fd, char32_t const *__restrict __format, __locale_t __locale, ...),wdprintf_p_l,vwdprintf_p_l,(__fd,__format,__locale),__locale)

__VREDIRECT_DPW16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_LIBC_W16SCANF(2,4) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fw16scanf_l,(__FILE *__restrict __stream, char16_t const *__restrict __format, __locale_t __locale, ...),fwscanf_l,vfwscanf_l,(__stream,__format,__locale),__locale)
__VREDIRECT_W32  (__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_LIBC_W32SCANF(2,4) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fw32scanf_l,(__FILE *__restrict __stream, char32_t const *__restrict __format, __locale_t __locale, ...),fwscanf_l,vfwscanf_l,(__stream,__format,__locale),__locale)
__VREDIRECT_DPW16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_LIBC_W16SCANF(1,3) __NONNULL((1)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,w16scanf_l,(char16_t const *__restrict __format, __locale_t __locale, ...),wscanf_l,vwscanf_l,(__format,__locale),__locale)
__VREDIRECT_W32  (__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_LIBC_W32SCANF(1,3) __NONNULL((1)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,w32scanf_l,(char32_t const *__restrict __format, __locale_t __locale, ...),wscanf_l,vwscanf_l,(__format,__locale),__locale)
__VREDIRECT_DPW16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_LIBC_W16SCANF(2,4) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,sw16scanf_l,(char16_t const *__restrict __src, char16_t const *__restrict __format, __locale_t __locale, ...),swscanf_l,vswscanf_l,(__src,__format,__locale),__locale)
__VREDIRECT_W32  (__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_LIBC_W32SCANF(2,4) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,sw32scanf_l,(char32_t const *__restrict __src, char32_t const *__restrict __format, __locale_t __locale, ...),swscanf_l,vswscanf_l,(__src,__format,__locale),__locale)
__REDIRECT_DPW16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_LIBC_W16SCANF(2,0) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__LIBCCALL,vfw16scanf_l,(__FILE *__restrict __stream, char16_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vfwscanf_l,(__stream,__format,__locale,__args))
__REDIRECT_W32  (__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_LIBC_W32SCANF(2,0) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__LIBCCALL,vfw32scanf_l,(__FILE *__restrict __stream, char32_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vfwscanf_l,(__stream,__format,__locale,__args))
__REDIRECT_DPW16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_LIBC_W16SCANF(1,0) __NONNULL((1)),__PRINTF_RETURN_TYPE,__LIBCCALL,vw16scanf_l,(char16_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vwscanf_l,(__format,__locale,__args))
__REDIRECT_W32  (__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_LIBC_W32SCANF(1,0) __NONNULL((1)),__PRINTF_RETURN_TYPE,__LIBCCALL,vw32scanf_l,(char32_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vwscanf_l,(__format,__locale,__args))
__REDIRECT_DPW16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_LIBC_W16SCANF(2,0) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__LIBCCALL,vsw16scanf_l,(char16_t const *__restrict __src, char16_t const *__restrict __format, __locale_t __locale, __builtin_va_list __arg),vswscanf_l,(__src,__format,__locale,__arg))
__REDIRECT_W32  (__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_LIBC_W32SCANF(2,0) __NONNULL((1,2)),__PRINTF_RETURN_TYPE,__LIBCCALL,vsw32scanf_l,(char32_t const *__restrict __src, char32_t const *__restrict __format, __locale_t __locale, __builtin_va_list __arg),vswscanf_l,(__src,__format,__locale,__arg))

__VREDIRECT_DPW16(__LIBC,__ATTR_LIBC_W16PRINTF(3,0) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,sw16printf,(char16_t *__restrict __buf, size_t __buflen, char16_t const *__restrict __format, ...),swprintf,vswprintf,(__buf,__buflen,__format),__format)
__VREDIRECT_W32  (__LIBC,__ATTR_LIBC_W32PRINTF(3,0) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,sw32printf,(char32_t *__restrict __buf, size_t __buflen, char32_t const *__restrict __format, ...),swprintf,vswprintf,(__buf,__buflen,__format),__format)
__REDIRECT_DPW16(__LIBC,__ATTR_LIBC_W16PRINTF(3,4) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__LIBCCALL,vsw16printf,(char16_t *__restrict __buf, size_t __buflen, char16_t const *__restrict __format, __builtin_va_list __args),vswprintf,(__buf,__buflen,__format,__args))
__REDIRECT_W32  (__LIBC,__ATTR_LIBC_W32PRINTF(3,4) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__LIBCCALL,vsw32printf,(char32_t *__restrict __buf, size_t __buflen, char32_t const *__restrict __format, __builtin_va_list __args),vswprintf,(__buf,__buflen,__format,__args))
__VREDIRECT_DPW16(__LIBC,__ATTR_LIBC_W16PRINTF_P(3,0) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,sw16printf_p,(char16_t *__restrict __buf, size_t __buflen, char16_t const *__restrict __format, ...),swprintf_p,vswprintf_p,(__buf,__buflen,__format),__format)
__VREDIRECT_W32  (__LIBC,__ATTR_LIBC_W32PRINTF_P(3,0) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,sw32printf_p,(char32_t *__restrict __buf, size_t __buflen, char32_t const *__restrict __format, ...),swprintf_p,vswprintf_p,(__buf,__buflen,__format),__format)
__REDIRECT_DPW16(__LIBC,__ATTR_LIBC_W16PRINTF_P(3,4) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__LIBCCALL,vsw16printf_p,(char16_t *__restrict __buf, size_t __buflen, char16_t const *__restrict __format, __builtin_va_list __args),vswprintf_p,(__buf,__buflen,__format,__args))
__REDIRECT_W32  (__LIBC,__ATTR_LIBC_W32PRINTF_P(3,4) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__LIBCCALL,vsw32printf_p,(char32_t *__restrict __buf, size_t __buflen, char32_t const *__restrict __format, __builtin_va_list __args),vswprintf_p,(__buf,__buflen,__format,__args))
__VREDIRECT_DPW16(__LIBC,__ATTR_LIBC_W16PRINTF(3,0) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,sw16printf_l,(char16_t *__restrict __buf, size_t __buflen, char16_t const *__restrict __format, __locale_t __locale, ...),snwprintf_l,vswprintf_l,(__buf,__buflen,__format,__locale),__locale)
__VREDIRECT_W32  (__LIBC,__ATTR_LIBC_W32PRINTF(3,0) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,sw32printf_l,(char32_t *__restrict __buf, size_t __buflen, char32_t const *__restrict __format, __locale_t __locale, ...),swprintf_l,vswprintf_l,(__buf,__buflen,__format,__locale),__locale)
__REDIRECT_DPW16(__LIBC,__ATTR_LIBC_W16PRINTF(3,4) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__LIBCCALL,vsw16printf_l,(char16_t *__restrict __buf, size_t __buflen, char16_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vswprintf_l,(__buf,__buflen,__format,__locale,__args))
__REDIRECT_W32  (__LIBC,__ATTR_LIBC_W32PRINTF(3,4) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__LIBCCALL,vsw32printf_l,(char32_t *__restrict __buf, size_t __buflen, char32_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vswprintf_l,(__buf,__buflen,__format,__locale,__args))
__VREDIRECT_DPW16(__LIBC,__ATTR_LIBC_W16PRINTF_P(3,0) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,sw16printf_p_l,(char16_t *__restrict __buf, size_t __buflen, char16_t const *__restrict __format, __locale_t __locale, ...),swprintf_p_l,vswprintf_p_l,(__buf,__buflen,__format,__locale),__locale)
__VREDIRECT_W32  (__LIBC,__ATTR_LIBC_W32PRINTF_P(3,0) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__ATTR_CDECL,sw32printf_p_l,(char32_t *__restrict __buf, size_t __buflen, char32_t const *__restrict __format, __locale_t __locale, ...),swprintf_p_l,vswprintf_p_l,(__buf,__buflen,__format,__locale),__locale)
__REDIRECT_DPW16(__LIBC,__ATTR_LIBC_W16PRINTF_P(3,4) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__LIBCCALL,vsw16printf_p_l,(char16_t *__restrict __buf, size_t __buflen, char16_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vswprintf_p_l,(__buf,__buflen,__format,__locale,__args))
__REDIRECT_W32  (__LIBC,__ATTR_LIBC_W32PRINTF_P(3,4) __NONNULL((1,3)),__PRINTF_RETURN_TYPE,__LIBCCALL,vsw32printf_p_l,(char32_t *__restrict __buf, size_t __buflen, char32_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args),vswprintf_p_l,(__buf,__buflen,__format,__locale,__args))

#undef __PRINTF_RETURN_TYPE
#endif /* __CRT_KOS */

__SYSDECL_END
#endif /* !__KERNEL__ */

#endif /* !_PARTS_KOS3_USTDIO_H */
