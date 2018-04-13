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
#ifndef _PARTS_KOS2_USTRING_H
#define _PARTS_KOS2_USTRING_H 1

#include "__stdinc.h"
#include "malldefs.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <xlocale.h>

__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */
#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */
#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */

#ifndef __KERNEL__
#ifdef __CRT_KOS

__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,w16len,(char16_t const *__restrict __s),wcslen,(__s))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,w32len,(char32_t const *__restrict __s),wcslen,(__s))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,w16nlen,(char16_t const *__str, size_t __max_chars),wcsnlen,(__str,__max_chars))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,w32nlen,(char32_t const *__str, size_t __max_chars),wcsnlen,(__str,__max_chars))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w16cmp,(char16_t const *__s1, char16_t const *__s2),wcscmp,(__s1,__s2))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w32cmp,(char32_t const *__s1, char32_t const *__s2),wcscmp,(__s1,__s2))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w16ncmp,(char16_t const *__s1, char16_t const *__s2, size_t __n),wcsncmp,(__s1,__s2,__n))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w32ncmp,(char32_t const *__s1, char32_t const *__s2, size_t __n),wcsncmp,(__s1,__s2,__n))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),char16_t *,__LIBCCALL,w16str,(char16_t const *__haystack, char16_t const *__needle),wcsstr,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),char32_t *,__LIBCCALL,w32str,(char32_t const *__haystack, char32_t const *__needle),wcsstr,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),char16_t *,__LIBCCALL,w16casestr,(char16_t const *__haystack, char16_t const *__needle),wcscasestr,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),char32_t *,__LIBCCALL,w32casestr,(char32_t const *__haystack, char32_t const *__needle),wcscasestr,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1,2)),char16_t *,__LIBCCALL,w16cpy,(char16_t *__restrict __dst, char16_t const *__restrict __src),wcscpy,(__dst,__src))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1,2)),char32_t *,__LIBCCALL,w32cpy,(char32_t *__restrict __dst, char32_t const *__restrict __src),wcscpy,(__dst,__src))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1,2)),char16_t *,__LIBCCALL,w16pcpy,(char16_t *__restrict __dst, char16_t const *__restrict __src),wcpcpy,(__dst,__src))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1,2)),char32_t *,__LIBCCALL,w32pcpy,(char32_t *__restrict __dst, char32_t const *__restrict __src),wcpcpy,(__dst,__src))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1,2)),char16_t *,__LIBCCALL,w16ncpy,(char16_t *__restrict __dst, char16_t const *__restrict __src, size_t __dstsize),wcsncpy,(__dst,__src,__dstsize))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1,2)),char32_t *,__LIBCCALL,w32ncpy,(char32_t *__restrict __dst, char32_t const *__restrict __src, size_t __dstsize),wcsncpy,(__dst,__src,__dstsize))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1,2)),char16_t *,__LIBCCALL,w16pncpy,(char16_t *__restrict __dst, char16_t const *__restrict __src, size_t __dstsize),wcpncpy,(__dst,__src,__dstsize))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1,2)),char32_t *,__LIBCCALL,w32pncpy,(char32_t *__restrict __dst, char32_t const *__restrict __src, size_t __dstsize),wcpncpy,(__dst,__src,__dstsize))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1,2)),char16_t *,__LIBCCALL,w16cat,(char16_t *__restrict __dst, char16_t const *__restrict __src),wcscat,(__dst,__src))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1,2)),char32_t *,__LIBCCALL,w32cat,(char32_t *__restrict __dst, char32_t const *__restrict __src),wcscat,(__dst,__src))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1,2)),char16_t *,__LIBCCALL,w16ncat,(char16_t *__restrict __dst, char16_t const *__restrict __src, size_t __max_chars),wcsncat,(__dst,__src,__max_chars))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1,2)),char32_t *,__LIBCCALL,w32ncat,(char32_t *__restrict __dst, char32_t const *__restrict __src, size_t __max_chars),wcsncat,(__dst,__src,__max_chars))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,w16spn,(char16_t const *__s, char16_t const *__accept),wcsspn,(__s,__accept))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,w32spn,(char32_t const *__s, char32_t const *__accept),wcsspn,(__s,__accept))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,w16cspn,(char16_t const *__s, char16_t const *__reject),wcscspn,(__s,__reject))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,w32cspn,(char32_t const *__s, char32_t const *__reject),wcscspn,(__s,__reject))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),char16_t *,__LIBCCALL,w16pbrk,(char16_t const *__s, char16_t const *__accept),wcspbrk,(__s,__accept))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),char32_t *,__LIBCCALL,w32pbrk,(char32_t const *__s, char32_t const *__accept),wcspbrk,(__s,__accept))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1)),char16_t *,__LIBCCALL,w16fry,(char16_t *__str),wcsfry,(__str))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1)),char32_t *,__LIBCCALL,w32fry,(char32_t *__str),wcsfry,(__str))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),char16_t *,__LIBCCALL,w16sep,(char16_t **__restrict __wcsingp, char16_t const *__restrict __delim),wcssep,(__wcsingp,__delim))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1,2)),char32_t *,__LIBCCALL,w32sep,(char32_t **__restrict __wcsingp, char32_t const *__restrict __delim),wcssep,(__wcsingp,__delim))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((1,2,3)),char16_t *,__LIBCCALL,w16tok,(char16_t *__restrict __s, char16_t const *__restrict __delim, char16_t **__restrict __save_ptr),wcstok_s,(__s,__delim,__save_ptr))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((1,2,3)),char32_t *,__LIBCCALL,w32tok,(char32_t *__restrict __s, char32_t const *__restrict __delim, char32_t **__restrict __save_ptr),wcstok,(__s,__delim,__save_ptr))
__REDIRECT_DPW16(__LIBC,__PORT_KOSONLY __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC,char16_t *,__LIBCCALL,w16dup,(char16_t const *__restrict __str),wcsdup,(__str))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC,char32_t *,__LIBCCALL,w32dup,(char32_t const *__restrict __str),wcsdup,(__str))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC,char16_t *,__LIBCCALL,w16ndup,(char16_t const *__restrict __str, size_t __max_chars),wcsndup,(__str,__max_chars))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC,char32_t *,__LIBCCALL,w32ndup,(char32_t const *__restrict __str, size_t __max_chars),wcsndup,(__str,__max_chars))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w16casecmp,(char16_t const *__s1, char16_t const *__s2),wcscasecmp,(__s1,__s2))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w32casecmp,(char32_t const *__s1, char32_t const *__s2),wcscasecmp,(__s1,__s2))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w16ncasecmp,(char16_t const *__s1, char16_t const *__s2, size_t __n),wcsncasecmp,(__s1,__s2,__n))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w32ncasecmp,(char32_t const *__s1, char32_t const *__s2, size_t __n),wcsncasecmp,(__s1,__s2,__n))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w16casecmp_l,(char16_t const *__s1, char16_t const *__s2, __locale_t __locale),wcscasecmp_l,(__s1,__s2,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w32casecmp_l,(char32_t const *__s1, char32_t const *__s2, __locale_t __locale),wcscasecmp_l,(__s1,__s2,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w16ncasecmp_l,(char16_t const *__s1, char16_t const *__s2, size_t __n, __locale_t __locale),wcsncasecmp_l,(__s1,__s2,__n,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,w32ncasecmp_l,(char32_t const *__s1, char32_t const *__s2, size_t __n, __locale_t __locale),wcsncasecmp_l,(__s1,__s2,__n,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,w16off,(char16_t const *__restrict __haystack, wint_t __needle),wcsoff,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,w32off,(char32_t const *__restrict __haystack, wint_t __needle),wcsoff,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,w16roff,(char16_t const *__restrict __haystack, wint_t __needle),wcsroff,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,w32roff,(char32_t const *__restrict __haystack, wint_t __needle),wcsroff,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,w16noff,(char16_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnoff,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,w32noff,(char32_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnoff,(__haystack,__needle,__max_chars))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,w16nroff,(char16_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnroff,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,w32nroff,(char32_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnroff,(__haystack,__needle,__max_chars))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2)),size_t,__LIBCCALL,w16xfrm,(char16_t *__dst, char16_t const *__restrict __src, size_t __n),wcsxfrm,(__dst,__src,__n))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2)),size_t,__LIBCCALL,w32xfrm,(char32_t *__dst, char32_t const *__restrict __src, size_t __n),wcsxfrm,(__dst,__src,__n))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __NONNULL((2)),size_t,__LIBCCALL,w16xfrm_l,(char16_t *__dst, char16_t const *__restrict __src, size_t __n, __locale_t __locale),wcsxfrm_l,(__dst,__src,__n,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __NONNULL((2)),size_t,__LIBCCALL,w32xfrm_l,(char32_t *__dst, char32_t const *__restrict __src, size_t __n, __locale_t __locale),wcsxfrm_l,(__dst,__src,__n,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w16coll,(char16_t const *__s1, char16_t const *__s2),wcscoll,(__s1,__s2))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w32coll,(char32_t const *__s1, char32_t const *__s2),wcscoll,(__s1,__s2))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w16coll_l,(char16_t const *__s1, char16_t const *__s2, __locale_t __locale),wcscoll_l,(__s1,__s2,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w32coll_l,(char32_t const *__s1, char32_t const *__s2, __locale_t __locale),wcscoll_l,(__s1,__s2,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w16ncoll,(char16_t const *__str1, char16_t const *__str2, size_t __max_chars),wcsncoll,(__str1,__str2,__max_chars))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w32ncoll,(char32_t const *__str1, char32_t const *__str2, size_t __max_chars),wcsncoll,(__str1,__str2,__max_chars))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w16ncoll_l,(char16_t const *__str1, char16_t const *__str2, size_t __max_chars, __locale_t __locale),wcsncoll_l,(__str1,__str2,__max_chars,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w32ncoll_l,(char32_t const *__str1, char32_t const *__str2, size_t __max_chars, __locale_t __locale),wcsncoll_l,(__str1,__str2,__max_chars,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w16casecoll,(char16_t const *__str1, char16_t const *__str2),wcscasecoll,(__str1,__str2))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w32casecoll,(char32_t const *__str1, char32_t const *__str2),wcscasecoll,(__str1,__str2))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w16casecoll_l,(char16_t const *__str1, char16_t const *__str2, __locale_t __locale),wcscasecoll_l,(__str1,__str2,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w32casecoll_l,(char32_t const *__str1, char32_t const *__str2, __locale_t __locale),wcscasecoll_l,(__str1,__str2,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w16ncasecoll,(char16_t const *__str1, char16_t const *__str2, size_t __max_chars),wcsncasecoll,(__str1,__str2,__max_chars))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w32ncasecoll,(char32_t const *__str1, char32_t const *__str2, size_t __max_chars),wcsncasecoll,(__str1,__str2,__max_chars))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w16ncasecoll_l,(char16_t const *__str1, char16_t const *__str2, size_t __max_chars, __locale_t __locale),wcsncasecoll_l,(__str1,__str2,__max_chars,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_PURE __WUNUSED __NONNULL((1,2)),int,__LIBCCALL,w32ncasecoll_l,(char32_t const *__str1, char32_t const *__str2, size_t __max_chars, __locale_t __locale),wcsncasecoll_l,(__str1,__str2,__max_chars,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16lwr,(char16_t *__restrict __str),wcslwr,(__str))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32lwr,(char32_t *__restrict __str),wcslwr,(__str))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16lwr_l,(char16_t *__restrict __str, __locale_t __locale),wcslwr_l,(__str,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32lwr_l,(char32_t *__restrict __str, __locale_t __locale),wcslwr_l,(__str,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16upr,(char16_t *__restrict __str),wcsupr,(__str))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32upr,(char32_t *__restrict __str),wcsupr,(__str))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16upr_l,(char16_t *__restrict __str, __locale_t __locale),wcsupr_l,(__str,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32upr_l,(char32_t *__restrict __str, __locale_t __locale),wcsupr_l,(__str,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16rev,(char16_t *__restrict __str),wcsrev,(__str))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32rev,(char32_t *__restrict __str),wcsrev,(__str))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16set,(char16_t *__restrict __str, wint_t __char),wcsset,(__str,__char))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32set,(char32_t *__restrict __str, wint_t __char),wcsset,(__str,__char))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16nset,(char16_t *__restrict __str, wint_t __char, size_t __max_chars),wcsnset,(__str,__char,__max_chars))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32nset,(char32_t *__restrict __str, wint_t __char, size_t __max_chars),wcsnset,(__str,__char,__max_chars))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,fuzzy_w16cmp,(char16_t const *__a, char16_t const *__b),fuzzy_wcscmp,(__a,__b))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,fuzzy_w32cmp,(char32_t const *__a, char32_t const *__b),fuzzy_wcscmp,(__a,__b))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_w16ncmp,(char16_t const *__a, size_t __max_a_chars, char16_t const *__b, size_t __max_b_chars),fuzzy_wcsncmp,(__a,__max_a_chars,__b,__max_b_chars))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_w32ncmp,(char32_t const *__a, size_t __max_a_chars, char32_t const *__b, size_t __max_b_chars),fuzzy_wcsncmp,(__a,__max_a_chars,__b,__max_b_chars))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,fuzzy_w16casecmp,(char16_t const *__a, char16_t const *__b),fuzzy_wcscasecmp,(__a,__b))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,fuzzy_w32casecmp,(char32_t const *__a, char32_t const *__b),fuzzy_wcscasecmp,(__a,__b))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_w16ncasecmp,(char16_t const *__a, size_t __max_a_chars, char16_t const *__b, size_t __max_b_chars),fuzzy_wcsncasecmp,(__a,__max_a_chars,__b,__max_b_chars))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_w32ncasecmp,(char32_t const *__a, size_t __max_a_chars, char32_t const *__b, size_t __max_b_chars),fuzzy_wcsncasecmp,(__a,__max_a_chars,__b,__max_b_chars))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,fuzzy_w16casecmp_l,(char16_t const *__a, char16_t const *__b, __locale_t __locale),fuzzy_wcscasecmp_l,(__a,__b,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),size_t,__LIBCCALL,fuzzy_w32casecmp_l,(char32_t const *__a, char32_t const *__b, __locale_t __locale),fuzzy_wcscasecmp_l,(__a,__b,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_w16ncasecmp_l,(char16_t const *__a, size_t __max_a_chars, char16_t const *__b, size_t __max_b_chars, __locale_t __locale),fuzzy_wcsncasecmp_l,(__a,__max_a_chars,__b,__max_b_chars,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)),size_t,__LIBCCALL,fuzzy_w32ncasecmp_l,(char32_t const *__a, size_t __max_a_chars, char32_t const *__b, size_t __max_b_chars, __locale_t __locale),fuzzy_wcsncasecmp_l,(__a,__max_a_chars,__b,__max_b_chars,__locale))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wildw16cmp,(char16_t const *__pattern, char16_t const *__string),wildwcscmp,(__pattern,__string))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wildw32cmp,(char32_t const *__pattern, char32_t const *__string),wildwcscmp,(__pattern,__string))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wildw16casecmp,(char16_t const *__pattern, char16_t const *__string),wildwcscasecmp,(__pattern,__string))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wildw32casecmp,(char32_t const *__pattern, char32_t const *__string),wildwcscasecmp,(__pattern,__string))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wildw16casecmp_l,(char16_t const *__pattern, char16_t const *__string, __locale_t __locale),wildwcscasecmp_l,(__pattern,__string,__locale))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,wildw32casecmp_l,(char32_t const *__pattern, char32_t const *__string, __locale_t __locale),wildwcscasecmp_l,(__pattern,__string,__locale))



#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t *,__LIBCCALL,w16index,(char16_t *__restrict __haystack, wint_t __needle),wcsindex,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t *,__LIBCCALL,w32index,(char32_t *__restrict __haystack, wint_t __needle),wcsindex,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t const *,__LIBCCALL,w16index,(char16_t const *__restrict __haystack, wint_t __needle),wcsindex,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t const *,__LIBCCALL,w32index,(char32_t const *__restrict __haystack, wint_t __needle),wcsindex,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t *,__LIBCCALL,w16rindex,(char16_t *__restrict __haystack, wint_t __needle),wcsrindex,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t *,__LIBCCALL,w32rindex,(char32_t *__restrict __haystack, wint_t __needle),wcsrindex,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t const *,__LIBCCALL,w16rindex,(char16_t const *__restrict __haystack, wint_t __needle),wcsrindex,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t const *,__LIBCCALL,w32rindex,(char32_t const *__restrict __haystack, wint_t __needle),wcsrindex,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t *,__LIBCCALL,w16chr,(char16_t *__restrict __haystack, wint_t __needle),wcschr,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t *,__LIBCCALL,w32chr,(char32_t *__restrict __haystack, wint_t __needle),wcschr,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t const *,__LIBCCALL,w16chr,(char16_t const *__restrict __haystack, wint_t __needle),wcschr,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t const *,__LIBCCALL,w32chr,(char32_t const *__restrict __haystack, wint_t __needle),wcschr,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t *,__LIBCCALL,w16rchr,(char16_t *__restrict __haystack, wint_t __needle),wcsrchr,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t *,__LIBCCALL,w32rchr,(char32_t *__restrict __haystack, wint_t __needle),wcsrchr,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t const *,__LIBCCALL,w16rchr,(char16_t const *__restrict __haystack, wint_t __needle),wcsrchr,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t const *,__LIBCCALL,w32rchr,(char32_t const *__restrict __haystack, wint_t __needle),wcsrchr,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t *,__LIBCCALL,w16nchr,(char16_t *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchr,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t *,__LIBCCALL,w32nchr,(char32_t *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchr,(__haystack,__needle,__max_chars))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t const *,__LIBCCALL,w16nchr,(char16_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchr,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t const *,__LIBCCALL,w32nchr,(char32_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchr,(__haystack,__needle,__max_chars))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t *,__LIBCCALL,w16nrchr,(char16_t *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchr,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t *,__LIBCCALL,w32nrchr,(char32_t *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchr,(__haystack,__needle,__max_chars))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t const *,__LIBCCALL,w16nrchr,(char16_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchr,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t const *,__LIBCCALL,w32nrchr,(char32_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchr,(__haystack,__needle,__max_chars))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16end,(char16_t *__restrict __str),wcsend,(__str))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32end,(char32_t *__restrict __str),wcsend,(__str))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t const *,__LIBCCALL,w16end,(char16_t const *__restrict __str),wcsend,(__str))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t const *,__LIBCCALL,w32end,(char32_t const *__restrict __str),wcsend,(__str))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16nend,(char16_t *__restrict __str, size_t __max_chars),wcsnend,(__str,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32nend,(char32_t *__restrict __str, size_t __max_chars),wcsnend,(__str,__max_chars))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t const *,__LIBCCALL,w16nend,(char16_t const *__restrict __str, size_t __max_chars),wcsnend,(__str,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t const *,__LIBCCALL,w32nend,(char32_t const *__restrict __str, size_t __max_chars),wcsnend,(__str,__max_chars))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16chrnul,(char16_t *__restrict __haystack, wint_t __needle),wcschrnul,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32chrnul,(char32_t *__restrict __haystack, wint_t __needle),wcschrnul,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t const *,__LIBCCALL,w16chrnul,(char16_t const *__restrict __haystack, wint_t __needle),wcschrnul,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t const *,__LIBCCALL,w32chrnul,(char32_t const *__restrict __haystack, wint_t __needle),wcschrnul,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16rchrnul,(char16_t *__restrict __haystack, wint_t __needle),wcsrchrnul,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32rchrnul,(char32_t *__restrict __haystack, wint_t __needle),wcsrchrnul,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t const *,__LIBCCALL,w16rchrnul,(char16_t const *__restrict __haystack, wint_t __needle),wcsrchrnul,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t const *,__LIBCCALL,w32rchrnul,(char32_t const *__restrict __haystack, wint_t __needle),wcsrchrnul,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16nchrnul,(char16_t *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchrnul,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32nchrnul,(char32_t *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchrnul,(__haystack,__needle,__max_chars))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t const *,__LIBCCALL,w16nchrnul,(char16_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchrnul,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t const *,__LIBCCALL,w32nchrnul,(char32_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchrnul,(__haystack,__needle,__max_chars))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16nrchrnul,(char16_t *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchrnul,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32nrchrnul,(char32_t *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchrnul,(__haystack,__needle,__max_chars))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t const *,__LIBCCALL,w16nrchrnul,(char16_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchrnul,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t const *,__LIBCCALL,w32nrchrnul,(char32_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchrnul,(__haystack,__needle,__max_chars))
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t *,__LIBCCALL,w16index,(char16_t const *__restrict __haystack, wint_t __needle),wcsindex,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t *,__LIBCCALL,w32index,(char32_t const *__restrict __haystack, wint_t __needle),wcsindex,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t *,__LIBCCALL,w16rindex,(char16_t const *__restrict __haystack, wint_t __needle),wcsrindex,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t *,__LIBCCALL,w32rindex,(char32_t const *__restrict __haystack, wint_t __needle),wcsrindex,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t *,__LIBCCALL,w16chr,(char16_t const *__restrict __haystack, wint_t __needle),wcschr,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t *,__LIBCCALL,w32chr,(char32_t const *__restrict __haystack, wint_t __needle),wcschr,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t *,__LIBCCALL,w16rchr,(char16_t const *__restrict __haystack, wint_t __needle),wcsrchr,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t *,__LIBCCALL,w32rchr,(char32_t const *__restrict __haystack, wint_t __needle),wcsrchr,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t *,__LIBCCALL,w16nchr,(char16_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchr,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t *,__LIBCCALL,w32nchr,(char32_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchr,(__haystack,__needle,__max_chars))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char16_t *,__LIBCCALL,w16nrchr,(char16_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchr,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char32_t *,__LIBCCALL,w32nrchr,(char32_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchr,(__haystack,__needle,__max_chars))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16end,(char16_t const *__restrict __str),wcsend,(__str))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32end,(char32_t const *__restrict __str),wcsend,(__str))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16nend,(char16_t const *__restrict __str, size_t __max_chars),wcsnend,(__str,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32nend,(char32_t const *__restrict __str, size_t __max_chars),wcsnend,(__str,__max_chars))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16chrnul,(char16_t const *__restrict __haystack, wint_t __needle),wcschrnul,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32chrnul,(char32_t const *__restrict __haystack, wint_t __needle),wcschrnul,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16rchrnul,(char16_t const *__restrict __haystack, wint_t __needle),wcsrchrnul,(__haystack,__needle))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32rchrnul,(char32_t const *__restrict __haystack, wint_t __needle),wcsrchrnul,(__haystack,__needle))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16nchrnul,(char16_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchrnul,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32nchrnul,(char32_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchrnul,(__haystack,__needle,__max_chars))
__REDIRECT_W16(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char16_t *,__LIBCCALL,w16nrchrnul,(char16_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchrnul,(__haystack,__needle,__max_chars))
__REDIRECT_W32(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char32_t *,__LIBCCALL,w32nrchrnul,(char32_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchrnul,(__haystack,__needle,__max_chars))
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */

#endif /* __CRT_KOS */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_PARTS_KOS2_USTRING_H */
