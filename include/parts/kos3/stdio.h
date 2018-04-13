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
#ifndef _PARTS_KOS3_STDIO_H
#define _PARTS_KOS3_STDIO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <xlocale.h>

#if defined(__CC__) && !defined(__KERNEL__)
__SYSDECL_BEGIN

#ifdef __USE_KOS
#define __PRINTF_RETURN_TYPE  __ssize_t
#else
#define __PRINTF_RETURN_TYPE  int
#endif

#ifdef __CRT_DOS

__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF(1,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,printf_l,(char const *__restrict __format, __locale_t __locale, ...),vprintf_l,(__format,__locale),__locale)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF(2,4),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fprintf_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...),vfprintf_l,(__stream,__format,__locale),__locale)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(1,2),__PRINTF_RETURN_TYPE,__ATTR_CDECL,printf_p,(char const *__restrict __format, ...),vprintf_p,(__format),__format)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(2,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fprintf_p,(__FILE *__restrict __stream, char const *__restrict __format, ...),vfprintf_p,(__stream,__format),__format)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(1,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,printf_p_l,(char const *__restrict __format, __locale_t __locale, ...),vprintf_p_l,(__format,__locale),__locale)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(2,4),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fprintf_p_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...),vfprintf_p_l,(__stream,__format,__locale),__locale)
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vprintf_l,(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfprintf_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__stream,__format,__locale,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vprintf_p,(char const *__restrict __format, __builtin_va_list __args),(__format,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfprintf_p,(__FILE *__restrict __stream, char const *__restrict __format, __builtin_va_list __args),(__stream,__format,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vprintf_p_l,(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfprintf_p_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__stream,__format,__locale,__args))

#ifdef __CRT_KOS
__VREDIRECT_DPA(__LIBC,__ATTR_LIBC_SCANF(1,2) __WUNUSED,__PRINTF_RETURN_TYPE,__ATTR_CDECL,scanf_l,(char const *__restrict __format, __locale_t __locale, ...),vscanf_l,(__format,__locale),__locale)
__VREDIRECT_DPA(__LIBC,__ATTR_LIBC_SCANF(2,0) __WUNUSED,__PRINTF_RETURN_TYPE,__ATTR_CDECL,fscanf_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...),vfscanf_l,(__stream,__format,__locale),__locale)
__LIBC __PORT_KOSONLY __ATTR_LIBC_SCANF(1,0) __WUNUSED __PRINTF_RETURN_TYPE (__LIBCCALL vscanf_l)(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __ATTR_LIBC_SCANF(2,0) __WUNUSED __PRINTF_RETURN_TYPE (__LIBCCALL vfscanf_l)(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#else
__LIBC __ATTR_LIBC_SCANF(1,2) __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL _scanf_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC __ATTR_LIBC_SCANF(2,0) __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL _fscanf_l)(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...);
#define scanf_l(format,locale)             ((_scanf_l)(format,locale))
#define fscanf_l(stream,format,locale,...) ((_fscanf_l)(stream,format,locale,##__VA_ARGS__))
#endif /* __CRT_KOS */
#endif /* __CRT_DOS */

#ifdef __CRT_KOS
#ifdef __USE_XOPEN2K8
__LIBC __ATTR_LIBC_PRINTF(2,0) __PORT_KOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL vdprintf_l)(__fd_t __fd, char const *__restrict __format, __builtin_va_list __args, __locale_t __locale);
__LIBC __ATTR_LIBC_PRINTF(2,4) __PORT_KOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL dprintf_l)(__fd_t __fd, char const *__restrict __format, __locale_t __locale, ...);
__LIBC __ATTR_LIBC_PRINTF_P(2,0) __PORT_KOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL vdprintf_p)(__fd_t __fd, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_PRINTF_P(2,3) __PORT_KOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL dprintf_p)(__fd_t __fd, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF_P(2,0) __PORT_KOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL vdprintf_p_l)(__fd_t __fd, char const *__restrict __format, __builtin_va_list __args, __locale_t __locale);
__LIBC __ATTR_LIBC_PRINTF_P(2,4) __PORT_KOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL dprintf_p_l)(__fd_t __fd, char const *__restrict __format, __locale_t __locale, ...);
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_GNU
__LIBC __ATTR_LIBC_PRINTF(2,4) __PORT_KOSONLY __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL asprintf_l)(char **__restrict __pstr, char const *__restrict __format, __locale_t __locale, ...);
__LIBC __ATTR_LIBC_PRINTF(2,0) __PORT_KOSONLY __WUNUSED __PRINTF_RETURN_TYPE (__LIBCCALL vasprintf_l)(char **__restrict __pstr, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __ATTR_LIBC_PRINTF_P(2,3) __PORT_KOSONLY __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL asprintf_p)(char **__restrict __pstr, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF_P(2,0) __PORT_KOSONLY __WUNUSED __PRINTF_RETURN_TYPE (__LIBCCALL vasprintf_p)(char **__restrict __pstr, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_PRINTF_P(2,4) __PORT_KOSONLY __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL asprintf_p_l)(char **__restrict __pstr, char const *__restrict __format, __locale_t __locale, ...);
__LIBC __ATTR_LIBC_PRINTF_P(2,0) __PORT_KOSONLY __WUNUSED __PRINTF_RETURN_TYPE (__LIBCCALL vasprintf_p_l)(char **__restrict __pstr, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#endif /* __USE_GNU */


#endif /* __CRT_KOS */

#undef __PRINTF_RETURN_TYPE

__SYSDECL_END
#endif /* __CC__ && !__KERNEL__ */

#ifdef _WCHAR_H
#ifndef _PARTS_KOS3_WSTDIO_H
#include "wstdio.h"
#endif
#endif

#endif /* !_PARTS_KOS3_STDIO_H */
