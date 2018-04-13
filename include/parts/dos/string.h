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
#ifndef _PARTS_DOS_STRING_H
#define _PARTS_DOS_STRING_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#ifndef _STRING_H
#include <string.h>
#endif

__SYSDECL_BEGIN

#ifdef __CRT_DOS
#ifndef __strlwr_defined
#define __strlwr_defined 1
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strlwr,(char *__restrict __str),(__str))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strnset,(char *__restrict __str, int __char, size_t __max_chars),(__str,__char,__max_chars))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strrev,(char *__restrict __str),(__str))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strupr,(char *__restrict __str),(__str))
#ifdef _MSC_VER
#ifndef ___strset_defined
#define ___strset_defined 1
__LIBC __PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL _strset)(char *__restrict __str, int __char);
#endif /* !___strset_defined */
#pragma intrinsic(_strset)
#define strset    _strset
#else
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strset,(char *__restrict __str, int __char),(__str,__char))
#endif
#endif /* !__strlwr_defined */
#endif /* __CRT_DOS */

__REDIRECT_DPB(__LIBC,__WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC __NONNULL((1)),char *,__LIBCCALL,strdup,(char const *__restrict __str),(__str))
#ifdef __GLC_COMPAT__
#ifndef ____libc_tolower_defined
#define ____libc_tolower_defined 1
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__libc_tolower,(int __c),tolower,(__c))
#endif /* !____libc_tolower_defined */
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2))
int (__LIBCCALL _memicmp)(void const *__a, void const *__b, size_t __n_bytes) {
    __BYTE_TYPE__ *__ai = (__BYTE_TYPE__ *)__a,*__bi = (__BYTE_TYPE__ *)__b; int __temp;
    while (__n_bytes--) if ((__temp = __libc_tolower(*__ai++) - __libc_tolower(*__bi++)) != 0) return __temp;
    return 0;
}
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2))
int (__LIBCCALL _memicmp_l)(void const *__a, void const *__b,
                            size_t __n_bytes, __locale_t __UNUSED(__locale)) {
    return _memicmp(__a,__b,__n_bytes);
}
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2))
int (__LIBCCALL memicmp)(void const *__a, void const *__b, size_t __n_bytes) {
    return _memicmp(__a,__b,__n_bytes);
}
#else
#ifndef ___strset_defined
#define ___strset_defined 1
#ifdef _MSC_VER
__LIBC __PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL _strset)(char *__restrict __str, int __char);
#pragma intrinsic(_strset)
#else /* _MSC_VER */
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strset,(char *__restrict __str, int __char),(__str,__char))
#endif /* !_MSC_VER */
#endif /* !___strset_defined */
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strnset,(char *__restrict __str, int __char, size_t __max_chars),(__str,__char,__max_chars))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strrev,(char *__restrict __str),(__str))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strlwr,(char *__restrict __str),(__str))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strupr,(char *__restrict __str),(__str))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strlwr_l,(char *__restrict __str, __locale_t __locale),(__str,__locale))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strupr_l,(char *__restrict __str, __locale_t __locale),(__str,__locale))
__REDIRECT_DPB(__LIBC,__ATTR_PURE __WUNUSED __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strncoll,(char const *__str1, char const *__str2, size_t __max_chars),(__str1,__str2,__max_chars))
__REDIRECT_DPB(__LIBC,__ATTR_PURE __WUNUSED __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strncoll_l,(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __locale),(__str1,__str2,__max_chars,__locale))
#ifdef __DOS_COMPAT__
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,stricoll,(char const *__str1, char const *__str2),(__str1,__str2))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,stricoll_l,(char const *__str1, char const *__str2, __locale_t __locale),(__str1,__str2,__locale))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strnicoll,(char const *__str1, char const *__str2, size_t __max_chars),(__str1,__str2,__max_chars))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strnicoll_l,(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __locale),(__str1,__str2,__max_chars,__locale))
__REDIRECT_DPA(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,memicmp,(void const *__a, void const *__b, size_t __n_bytes),(__a,__b,__n_bytes))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,memicmp,(void const *__a, void const *__b, size_t __n_bytes),(__a,__b,__n_bytes))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,memicmp_l,(void const *__a, void const *__b, size_t __n_bytes, __locale_t __locale),(__a,__b,__n_bytes,__locale))
#else
__REDIRECT(__LIBC,__PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_stricoll,(char const *__str1, char const *__str2),strcasecoll,(__str1,__str2))
__REDIRECT(__LIBC,__PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_stricoll_l,(char const *__str1, char const *__str2, __locale_t __locale),strcasecoll_l,(__str1,__str2,__locale))
__REDIRECT(__LIBC,__PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_strnicoll,(char const *__str1, char const *__str2, size_t __max_chars),strncasecoll,(__str1,__str2,__max_chars))
__REDIRECT(__LIBC,__PORT_DOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_strnicoll_l,(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __locale),strncasecoll_l,(__str1,__str2,__max_chars,__locale))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,memicmp,(void const *__a, void const *__b, size_t __n_bytes),memcasecmp,(__a,__b,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_memicmp,(void const *__a, void const *__b, size_t __n_bytes),memcasecmp,(__a,__b,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_memicmp_l,(void const *__a, void const *__b, size_t __n_bytes, __locale_t __locale),memcasecmp_l,(__a,__b,__n_bytes,__locale))
#endif
#endif /* Builtin... */

__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strcoll_l,(char const *__str1, char const *__str2, __locale_t __locale),(__str1,__str2,__locale))

#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_strcmpi,(char const *__str1, char const *__str2),_stricmp,(__str1,__str2))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,stricmp,(char const *__str1, char const *__str2),(__str1,__str2))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,stricmp_l,(char const *__str1, char const *__str2, __locale_t __locale),(__str1,__str2,__locale))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strnicmp,(char const *__str1, char const *__str2, size_t __max_chars),(__str1,__str2,__max_chars))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strnicmp_l,(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __locale),(__str1,__str2,__max_chars,__locale))
__REDIRECT_DPB(__LIBC,__NONNULL((1,2)),size_t,__LIBCCALL,strxfrm_l,(char *__dst, char const *__src, size_t __max_chars, __locale_t __locale),(__dst,__src,__max_chars,__locale))
__REDIRECT_DPB(__LIBC,__NONNULL((1,2)),void *,__LIBCCALL,memccpy,(void *__dst, void const *__src, int __needle, size_t __max_chars),(__dst,__src,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strcmpi,(char const *__str1, char const *__str2),_stricmp,(__str1,__str2))
__REDIRECT_DPA(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,stricmp,(char const *__str1, char const *__str2),(__str1,__str2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strnicmp,(char const *__str1, char const *__str2, size_t __max_chars),_strnicmp,(__str1,__str2,__max_chars))
#else
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_strcmpi,(char const *__str1, char const *__str2),strcasecmp,(__str1,__str2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_stricmp,(char const *__str1, char const *__str2),strcasecmp,(__str1,__str2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strcmpi,(char const *__str1, char const *__str2),strcasecmp,(__str1,__str2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,stricmp,(char const *__str1, char const *__str2),strcasecmp,(__str1,__str2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_stricmp_l,(char const *__str1, char const *__str2, __locale_t __locale),strcasecmp_l,(__str1,__str2,__locale))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_strnicmp,(char const *__str1, char const *__str2, size_t __max_chars),strncasecmp,(__str1,__str2,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strnicmp,(char const *__str1, char const *__str2, size_t __max_chars),strncasecmp,(__str1,__str2,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_strnicmp_l,(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __locale),strncasecmp_l,(__str1,__str2,__max_chars,__locale))
__REDIRECT(__LIBC,__NONNULL((1,2)),size_t,__LIBCCALL,_strxfrm_l,(char *__dst, char const *__src, size_t __max_chars, __locale_t __locale),strxfrm_l,(__dst,__src,__max_chars,__locale))
__REDIRECT(__LIBC,__NONNULL((1,2)),void *,__LIBCCALL,_memccpy,(void *__dst, void const *__src, int __needle, size_t __max_chars),memccpy,(__dst,__src,__needle,__max_chars))
#endif



/* Fulfill DOS's need to put all the wide-string stuff in here as well... */
#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifdef __DOS_COMPAT__
__REDIRECT_DPA(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsicmp,(wchar_t const *__str1, wchar_t const *__str2),(__str1,__str2))
__REDIRECT_DPA(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsnicmp,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),(__str1,__str2,__max_chars))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsicmp,(wchar_t const *__str1, wchar_t const *__str2),(__str1,__str2))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsnicmp,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),(__str1,__str2,__max_chars))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsicmp_l,(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale),(__str1,__str2,__locale))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsnicmp_l,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale),(__str1,__str2,__max_chars,__locale))
#else
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsicmp,(wchar_t const *__str1, wchar_t const *__str2),wcscasecmp,(__str1,__str2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsnicmp,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),wcsncasecmp,(__str1,__str2,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsicmp,(wchar_t const *__str1, wchar_t const *__str2),wcscasecmp,(__str1,__str2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsnicmp,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),wcsncasecmp,(__str1,__str2,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsicmp_l,(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale),wcscasecmp_l,(__str1,__str2,__locale))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsnicmp_l,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale),wcsncasecmp_l,(__str1,__str2,__max_chars,__locale))
#endif

__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcscoll_l,(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale),(__str1,__str2,__locale))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsncoll,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),(__str1,__str2,__max_chars))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsncoll_l,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale),(__str1,__str2,__max_chars,__locale))
#ifdef __DOS_COMPAT__
__REDIRECT_DPA(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsicoll,(wchar_t const *__str1, wchar_t const *__str2),(__str1,__str2))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsicoll,(wchar_t const *__str1, wchar_t const *__str2),(__str1,__str2))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsicoll_l,(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale),(__str1,__str2,__locale))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsnicoll,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),(__str1,__str2,__max_chars))
__REDIRECT_DPB(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsnicoll_l,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale),(__str1,__str2,__max_chars,__locale))
#else
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wcsicoll,(wchar_t const *__str1, wchar_t const *__str2),wcscasecoll,(__str1,__str2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsicoll,(wchar_t const *__str1, wchar_t const *__str2),wcscasecoll,(__str1,__str2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsicoll_l,(wchar_t const *__str1, wchar_t const *__str2, __locale_t __locale),wcscasecoll_l,(__str1,__str2,__locale))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsnicoll,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars),wcsncasecoll,(__str1,__str2,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,_wcsnicoll_l,(wchar_t const *__str1, wchar_t const *__str2, size_t __max_chars, __locale_t __locale),wcsncasecoll_l,(__str1,__str2,__max_chars,__locale))
#endif
__REDIRECT_DPB(__LIBC,__NONNULL((1,2)),size_t,__LIBCCALL,wcsxfrm_l,(wchar_t *__restrict __dst, wchar_t const *__restrict __src, size_t __max_chars, __locale_t __locale),(__dst,__src,__max_chars,__locale))
__REDIRECT_DPB(__LIBC,__WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC __NONNULL((1)),wchar_t *,__LIBCCALL,wcsdup,(wchar_t const *__restrict __str),(__str))


#ifdef __CRT_DOS
/* TODO: Add GLC compat implementations for these. */
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsrev,(wchar_t *__restrict __str),(__str))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsset,(wchar_t *__restrict __str, wchar_t __needle),(__str,__needle))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsnset,(wchar_t *__restrict __str, wchar_t __needle, size_t __max_chars),(__str,__needle,__max_chars))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcslwr,(wchar_t *__restrict __str),(__str))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsupr,(wchar_t *__restrict __str),(__str))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsrev,(wchar_t *__restrict __str),(__str))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsset,(wchar_t *__restrict __str, wchar_t __char),(__str,__char))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsnset,(wchar_t *__restrict __str, wchar_t __char, size_t __max_chars),(__str,__char,__max_chars))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcslwr,(wchar_t *__restrict __str),(__str))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcslwr_l,(wchar_t *__restrict __str, __locale_t __locale),(__str,__locale))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsupr,(wchar_t *__restrict __str),(__str))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsupr_l,(wchar_t *__restrict __str, __locale_t __locale),(__str,__locale))
#endif /* __CRT_DOS */

#ifndef __std_wcscmp_defined
#define __std_wcscmp_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcscmp)(wchar_t const *__s1, wchar_t const *__s2);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wcsncmp)(wchar_t const *__s1, wchar_t const *__s2, size_t __n);
__LIBC __WUNUSED __NONNULL((1,2)) int (__LIBCCALL wcscoll)(wchar_t const *__s1, wchar_t const *__s2);
__LIBC __NONNULL((1,2)) size_t (__LIBCCALL wcsxfrm)(wchar_t *__restrict __s1, wchar_t const *__restrict __s2, size_t __n);
__NAMESPACE_STD_END
#endif /* !__std_wcscmp_defined */
#ifndef __wcscmp_defined
#define __wcscmp_defined 1
__NAMESPACE_STD_USING(wcscmp)
__NAMESPACE_STD_USING(wcsncmp)
__NAMESPACE_STD_USING(wcscoll)
__NAMESPACE_STD_USING(wcsxfrm)
#endif /* !__wcscmp_defined */

#ifndef __std_wcsspn_defined
#define __std_wcsspn_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL wcsspn)(wchar_t const *__haystack, wchar_t const *__accept);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL wcscspn)(wchar_t const *__haystack, wchar_t const *__reject);
__NAMESPACE_STD_END
#endif /* !__std_wcsspn_defined */
#ifndef __wcsspn_defined
#define __wcsspn_defined 1
__NAMESPACE_STD_USING(wcsspn)
__NAMESPACE_STD_USING(wcscspn)
#endif /* !__wcsspn_defined */

#ifndef __std_wcslen_defined
#define __std_wcslen_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcslen)(wchar_t const *__restrict __str);
__NAMESPACE_STD_END
#endif /* !__std_wcslen_defined */
#ifndef __wcslen_defined
#define __wcslen_defined 1
__NAMESPACE_STD_USING(wcslen)
#endif /* !__wcslen_defined */

#ifndef __std_wcschr_defined
#define __std_wcschr_defined 1
__NAMESPACE_STD_BEGIN
#ifdef __CORRECT_ISO_CPP_WCHAR_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t *,__LIBCCALL,wcschr,(wchar_t *__restrict __str, wchar_t __needle),wcschr,(__str,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t *,__LIBCCALL,wcsrchr,(wchar_t *__restrict __str, wchar_t __needle),wcsrchr,(__str,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t const *,__LIBCCALL,wcschr,(wchar_t const *__restrict __str, wchar_t __needle),wcschr,(__str,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t const *,__LIBCCALL,wcsrchr,(wchar_t const *__restrict __str, wchar_t __needle),wcsrchr,(__str,__needle))
}
#else /* __CORRECT_ISO_CPP_WCHAR_H_PROTO */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcschr)(wchar_t const *__restrict __str, wchar_t __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsrchr)(wchar_t const *__restrict __str, wchar_t __needle);
#endif /* !__CORRECT_ISO_CPP_WCHAR_H_PROTO */
__NAMESPACE_STD_END
#endif /* !__std_wcschr_defined */
#ifndef __wcschr_defined
#define __wcschr_defined 1
__NAMESPACE_STD_USING(wcschr)
__NAMESPACE_STD_USING(wcsrchr)
#endif /* !__wcschr_defined */

#ifndef __std_wcscpy_defined
#define __std_wcscpy_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcscpy)(wchar_t *__dst, wchar_t const *__src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcscat)(wchar_t *__dst, wchar_t const *__src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcsncat)(wchar_t *__dst, wchar_t const *__src, size_t __max_chars);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) wchar_t *(__LIBCCALL wcsncpy)(wchar_t *__dst, wchar_t const *__src, size_t __max_chars);
__NAMESPACE_STD_END
#endif /* !__std_wcscpy_defined */
#ifndef __wcscpy_defined
#define __wcscpy_defined 1
__NAMESPACE_STD_USING(wcscpy)
__NAMESPACE_STD_USING(wcscat)
__NAMESPACE_STD_USING(wcsncat)
__NAMESPACE_STD_USING(wcsncpy)
#endif /* !__wcscpy_defined */

#ifndef __std_wcsstr_defined
#define __std_wcsstr_defined 1
__NAMESPACE_STD_BEGIN
#ifdef __CORRECT_ISO_CPP_WCHAR_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),wchar_t *,__LIBCCALL,wcspbrk,(wchar_t *__haystack, wchar_t const *__accept),wcspbrk,(__haystack,__accept))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),wchar_t const *,__LIBCCALL,wcspbrk,(wchar_t const *__haystack, wchar_t const *__accept),wcspbrk,(__haystack,__accept))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),wchar_t *,__LIBCCALL,wcsstr,(wchar_t *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),wchar_t const *,__LIBCCALL,wcsstr,(wchar_t const *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
}
#else /* __CORRECT_ISO_CPP_WCHAR_H_PROTO */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) wchar_t *(__LIBCCALL wcspbrk)(wchar_t const *__haystack, wchar_t const *__accept);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) wchar_t *(__LIBCCALL wcsstr)(wchar_t const *__haystack, wchar_t const *__needle);
#endif /* !__CORRECT_ISO_CPP_WCHAR_H_PROTO */
__NAMESPACE_STD_END
#endif /* !__std_wcsstr_defined */
#ifndef __wcsstr_defined
#define __wcsstr_defined 1
__NAMESPACE_STD_USING(wcspbrk)
__NAMESPACE_STD_USING(wcsstr)
#endif /* !__wcsstr_defined */

#ifndef __wcswcs_defined
#define __wcswcs_defined 1
#ifdef __CORRECT_ISO_CPP_WCHAR_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__ATTR_PURE,wchar_t *,__LIBCCALL,wcswcs,(wchar_t *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
__REDIRECT(__LIBC,__ATTR_PURE,wchar_t const *,__LIBCCALL,wcswcs,(wchar_t const *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
}
#else /* __CORRECT_ISO_CPP_WCHAR_H_PROTO */
__REDIRECT(__LIBC,__ATTR_PURE,wchar_t *,__LIBCCALL,wcswcs,(wchar_t const *__haystack, wchar_t const *__needle),wcsstr,(__haystack,__needle))
#endif /* !__CORRECT_ISO_CPP_WCHAR_H_PROTO */
#endif /* !__wcswcs_defined */

#ifndef __std_wcstok_defined
#define __std_wcstok_defined 1
__NAMESPACE_STD_BEGIN
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
__NAMESPACE_STD_END
#endif /* !__std_wcstok_defined */
#ifndef __wcstok_defined
#define __wcstok_defined 1
__NAMESPACE_STD_USING(wcstok)
#endif /* !__wcstok_defined */

__SYSDECL_END

#include "wstring.h"

#endif /* !_PARTS_DOS_STRING_H */
