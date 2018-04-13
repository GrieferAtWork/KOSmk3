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
#ifndef _PARTS_DOS_INTTYPES_H
#define _PARTS_DOS_INTTYPES_H 1

#include "__stdinc.h"
#include <features.h>
#ifndef _INTTYPES_H
#include <inttypes.h>
#endif

__SYSDECL_BEGIN

#if __SIZEOF_INTMAX_T__ == 8 && defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,,__INTMAX_TYPE__,__LIBCCALL,_strtoimax_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),_strtoi64_l,(__nptr,__endptr,__radix,__locale))
__REDIRECT(__LIBC,,__UINTMAX_TYPE__,__LIBCCALL,_strtoumax_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),_strtoui64_l,(__nptr,__endptr,__radix,__locale))
__REDIRECT(__LIBC,,__INTMAX_TYPE__,__LIBCCALL,_wcstoimax_l,(__WCHAR_TYPE__ const *__restrict __nptr, __WCHAR_TYPE__ **__restrict __endptr, int __radix, __locale_t __locale),_wcstoi64_l,(__nptr,__endptr,__radix,__locale))
__REDIRECT(__LIBC,,__UINTMAX_TYPE__,__LIBCCALL,_wcstoumax_l,(__WCHAR_TYPE__ const *__restrict __nptr, __WCHAR_TYPE__ **__restrict __endptr, int __radix, __locale_t __locale),_wcstoui64_l,(__nptr,__endptr,__radix,__locale))
#elif __SIZEOF_INTMAX_T__ == __SIZEOF_LONG__
__REDIRECT(__LIBC,,__INTMAX_TYPE__,__LIBCCALL,_strtoimax_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(strtol_l),(__nptr,__endptr,__radix,__locale))
__REDIRECT(__LIBC,,__UINTMAX_TYPE__,__LIBCCALL,_strtoumax_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(strtoul_l),(__nptr,__endptr,__radix,__locale))
__REDIRECT(__LIBC,,__INTMAX_TYPE__,__LIBCCALL,_wcstoimax_l,(__WCHAR_TYPE__ const *__restrict __nptr, __WCHAR_TYPE__ **__restrict __endptr, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstol_l),(__nptr,__endptr,__radix,__locale))
__REDIRECT(__LIBC,,__UINTMAX_TYPE__,__LIBCCALL,_wcstoumax_l,(__WCHAR_TYPE__ const *__restrict __nptr, __WCHAR_TYPE__ **__restrict __endptr, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstoul_l),(__nptr,__endptr,__radix,__locale))
#elif __SIZEOF_INTMAX_T__ == __SIZEOF_LONG_LONG__
__REDIRECT(__LIBC,,__INTMAX_TYPE__,__LIBCCALL,_strtoimax_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(strtoll_l),(__nptr,__endptr,__radix,__locale))
__REDIRECT(__LIBC,,__UINTMAX_TYPE__,__LIBCCALL,_strtoumax_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(strtoull_l),(__nptr,__endptr,__radix,__locale))
__REDIRECT(__LIBC,,__INTMAX_TYPE__,__LIBCCALL,_wcstoimax_l,(__WCHAR_TYPE__ const *__restrict __nptr, __WCHAR_TYPE__ **__restrict __endptr, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstoll_l),(__nptr,__endptr,__radix,__locale))
__REDIRECT(__LIBC,,__UINTMAX_TYPE__,__LIBCCALL,_wcstoumax_l,(__WCHAR_TYPE__ const *__restrict __nptr, __WCHAR_TYPE__ **__restrict __endptr, int __radix, __locale_t __locale),__SYMNAME_DOSPREFIX(wcstoull_l),(__nptr,__endptr,__radix,__locale))
#else /* ... */
__REDIRECT_DPB(__LIBC,__INTMAX_TYPE__,__LIBCCALL,strtoimax_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),(__nptr,__endptr,__radix,__locale))
__REDIRECT_DPB(__LIBC,__UINTMAX_TYPE__,__LIBCCALL,strtoumax_l,(char const *__restrict __nptr, char **__restrict __endptr, int __radix, __locale_t __locale),(__nptr,__endptr,__radix,__locale))
__REDIRECT_DPB(__LIBC,__INTMAX_TYPE__,__LIBCCALL,wcstoimax_l,(__WCHAR_TYPE__ const *__restrict __nptr, __WCHAR_TYPE__ **__restrict __endptr, int __radix, __locale_t __locale),(__nptr,__endptr,__radix,__locale))
__REDIRECT_DPB(__LIBC,__UINTMAX_TYPE__,__LIBCCALL,wcstoumax_l,(__WCHAR_TYPE__ const *__restrict __nptr, __WCHAR_TYPE__ **__restrict __endptr, int __radix, __locale_t __locale),(__nptr,__endptr,__radix,__locale))
#endif /* !... */

__SYSDECL_END

#endif /* !_PARTS_DOS_INTTYPES_H */
