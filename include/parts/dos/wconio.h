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
#ifndef _PARTS_DOS_WCONIO_H
#define _PARTS_DOS_WCONIO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <xlocale.h>

__SYSDECL_BEGIN

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#ifndef _WCONIO_DEFINED
#define _WCONIO_DEFINED 1
#ifdef __CRT_DOS
#ifdef __USE_KOS_STDEXT
#define __PRINTF_RETURN_TYPE __ssize_t
#else
#define __PRINTF_RETURN_TYPE int
#endif


/* Since KOS doesn't differentiate for these, they just redirect to stdin/stdout.
 * >> If you really want to operate on a console, open your terminal slave under `/dev/'. */
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _cgetws_s)(wchar_t *__buffer, size_t __buflen, size_t *__sizeok);
__LIBC __PORT_DOSONLY wchar_t *(__LIBCCALL _cgetws)(wchar_t *__buffer);
__LIBC __PORT_DOSONLY wint_t (__LIBCCALL _getwch)(void);
__LIBC __PORT_DOSONLY wint_t (__LIBCCALL _getwch_nolock)(void);
__LIBC __PORT_DOSONLY wint_t (__LIBCCALL _getwche)(void);
__LIBC __PORT_DOSONLY wint_t (__LIBCCALL _getwche_nolock)(void);
__LIBC __PORT_DOSONLY wint_t (__LIBCCALL _putwch)(wchar_t __wc);
__LIBC __PORT_DOSONLY wint_t (__LIBCCALL _putwch_nolock)(wchar_t __wc);
__LIBC __PORT_DOSONLY wint_t (__LIBCCALL _ungetwch)(wint_t __wc);
__LIBC __PORT_DOSONLY wint_t (__LIBCCALL _ungetwch_nolock)(wint_t __wc);
__LIBC __PORT_DOSONLY int (__LIBCCALL _cputws)(wchar_t const *__string);

__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vcwprintf,(wchar_t const *__format, __builtin_va_list __args),(__format,__args))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vcwprintf_p,(wchar_t const*__format, __builtin_va_list __args),(__format,__args))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vcwprintf_s,(wchar_t const *__format, __builtin_va_list __args),(__format,__args))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vcwprintf_l,(wchar_t const *__format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vcwprintf_p_l,(wchar_t const *__format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vcwprintf_s_l,(wchar_t const *__format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,cwprintf,(wchar_t const *__format, ...),vcwprintf,(__format),__format)
__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,cwprintf_p,(wchar_t const *__format, ...),vcwprintf_p,(__format),__format)
__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,cwprintf_s,(wchar_t const *__format, ...),vcwprintf_s,(__format),__format)
__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,cwprintf_l,(wchar_t const *__format, __locale_t __locale, ...),vcwprintf_l,(__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,cwprintf_p_l,(wchar_t const *__format, __locale_t __locale, ...),vcwprintf_p_l,(__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,cwprintf_s_l,(wchar_t const *__format, __locale_t __locale, ...),vcwprintf_s_l,(__format,__locale),__locale)

__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,cwscanf,(wchar_t const *__format, ...),vcwscanf,(__format),__format)
__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,cwscanf_s,(wchar_t const *__format, ...),vcwscanf_s,(__format),__format)
__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,cwscanf_l,(wchar_t const *__format, __locale_t __locale, ...),vcwscanf_l,(__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,cwscanf_s_l,(wchar_t const *__format, __locale_t __locale, ...),vcwscanf_s_l,(__format,__locale),__locale)
#undef __PRINTF_RETURN_TYPE
#endif /* __CRT_DOS */
#endif /* !_WCONIO_DEFINED */


__SYSDECL_END

#endif /* !_PARTS_DOS_WCONIO_H */
