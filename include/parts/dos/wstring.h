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
#ifndef _PARTS_DOS_WSTRING_H
#define _PARTS_DOS_WSTRING_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <xlocale.h>

__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef __std_size_t_defined
#define __std_size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__std_size_t_defined */

#ifndef __wcsnlen_defined
#define __wcsnlen_defined 1
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcsnlen)(wchar_t const *__restrict __src, size_t __max_chars);
#endif /* !__wcsnlen_defined */

#ifndef _WSTRING_DEFINED
#define _WSTRING_DEFINED 1

#ifdef __DOS_COMPAT__
__LIBC __NONNULL((1,2,3)) wchar_t *(__LIBCCALL wcstok_s)(wchar_t *__restrict __str, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr);
#else
__REDIRECT(__LIBC,__NONNULL((1,2,3)),wchar_t *,__LIBCCALL,wcstok_s,(wchar_t *__restrict __str, wchar_t const *__restrict __delim, wchar_t **__restrict __ptr),wcstok,(__str,__delim,__ptr))
#endif

#ifdef __CRT_DOS
__REDIRECT_DPB(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(strerror),wchar_t *,__LIBCCALL,wcserror,(errno_t __errnum),(__errnum))
__REDIRECT_DPB(__LIBC,__NONNULL((1)) __PORT_DOSONLY_ALT(strerror),errno_t,__LIBCCALL,wcserror_s,(wchar_t *__restrict __buf, size_t __max_chars, errno_t __errnum),(__buf,__max_chars,__errnum))
__LIBC __WUNUSED __PORT_DOSONLY_ALT(strerror) wchar_t *(__LIBCCALL __wcserror)(wchar_t const *__errmsg);
__LIBC __NONNULL((1)) __PORT_DOSONLY_ALT(strerror) errno_t (__LIBCCALL __wcserror_s)(wchar_t *__restrict __buf, size_t __max_chars, wchar_t const *__errmsg);

#ifdef __GLC_COMPAT__
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__libc_towlower,(int __wc),towlower,(__wc))
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__libc_towupper,(int __wc),towupper,(__wc))
__REDIRECT(__LIBC,__WUNUSED,int,__LIBCCALL,__libc_towlower_l,(int __wc, __locale_t __locale),__SYMNAME_DOSPREFIX(towlower_l),(__wc))
__REDIRECT(__LIBC,__WUNUSED,int,__LIBCCALL,__libc_towupper_l,(int __wc, __locale_t __locale),__SYMNAME_DOSPREFIX(towupper_l),(__wc))
__LOCAL __NONNULL((1)) errno_t (__LIBCCALL _wcsset_s)(wchar_t *__restrict __str, size_t __maxlen, wchar_t __val) { if (wcsnlen(__str,__buflen) == __buflen) return 34; wcsset(__str,__val); return 0; }
__LOCAL __NONNULL((1)) errno_t (__LIBCCALL _wcsnset_s)(wchar_t *__restrict __str, size_t __buflen, wchar_t __val, size_t __maxlen) { if (__maxlen < __buflen) __buflen = __maxlen; while (__buflen-- && *__str) *__str++ = __val; return 0; }
__LOCAL __NONNULL((1)) errno_t (__LIBCCALL _wcslwr_s)(wchar_t *__restrict __str, size_t __maxlen) { while (__maxlen-- && *__str) *__str = __libc_towlower(*__str),++__str; return 0 }
__LOCAL __NONNULL((1)) errno_t (__LIBCCALL _wcsupr_s)(wchar_t *__restrict __str, size_t __maxlen) { while (__maxlen-- && *__str) *__str = __libc_towupper(*__str),++__str; return 0 }
__LOCAL __NONNULL((1)) errno_t (__LIBCCALL _wcslwr_s_l)(wchar_t *__restrict __str, size_t __maxlen, __locale_t __locale) { while (__maxlen-- && *__str) *__str = __libc_towlower_l(*__str,__locale),++__str; return 0 }
__LOCAL __NONNULL((1)) errno_t (__LIBCCALL _wcsupr_s_l)(wchar_t *__restrict __str, size_t __maxlen, __locale_t __locale) { while (__maxlen-- && *__str) *__str = __libc_towupper_l(*__str,__locale),++__str; return 0 }
#else /* __GLC_COMPAT__ */
__REDIRECT_DPB(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,wcsset_s,(wchar_t *__restrict __str, size_t __maxlen, wchar_t __val),(__str,__maxlen,__val))
__REDIRECT_DPB(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,wcsnset_s,(wchar_t *__restrict __str, size_t __buflen, wchar_t __val, size_t __maxlen),(__str,__buflen,__val,__maxlen))
__REDIRECT_DPB(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,wcslwr_s,(wchar_t *__restrict __str, size_t __maxlen),(__str,__maxlen))
__REDIRECT_DPB(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,wcsupr_s,(wchar_t *__restrict __str, size_t __maxlen),(__str,__maxlen))
__REDIRECT_DPB(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,wcslwr_s_l,(wchar_t *__restrict __str, size_t __maxlen, __locale_t __locale),(__str,__maxlen,__locale))
__REDIRECT_DPB(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,wcsupr_s_l,(wchar_t *__restrict __str, size_t __maxlen, __locale_t __locale),(__str,__maxlen,__locale))
#endif /* !__GLC_COMPAT__ */

#ifdef __USE_DOS_SLIB
#ifdef __GLC_COMPAT__
__LOCAL __NONNULL((1,3)) errno_t (__LIBCCALL wcscat_s)
(wchar_t *__restrict __dst, size_t __dstsize, wchar_t const *__restrict __src) {
    wchar_t *__dstend = (wchar_t *)__dst+wcsnlen(__dst,__dstsize);
    size_t __srclen = wcslen(__src);
    __dstsize -= (size_t)(__dstend-__dst);
    if (__srclen+1 > __dstsize) return 34;
    do *__dstend++ = *__src++; while (__srclen--);
    return 0;
}
__LOCAL __NONNULL((1,3)) errno_t (__LIBCCALL wcscpy_s)
(wchar_t *__restrict __dst, size_t __dstsize, wchar_t const *__restrict __src) {
    size_t __srclen = wcslen(__src);
    if (__srclen+1 > __dstsize) return 34;
    do *__dst++ = *__src++; while (__srclen--);
    return 0;
}
__LOCAL __NONNULL((1,3)) errno_t (__LIBCCALL wcsncat_s)
(wchar_t *__restrict __dst, size_t __dstsize, wchar_t const *__restrict __src, size_t __max_chars) {
    wchar_t *__dstend = (wchar_t *)__dst+wcsnlen(__dst,__dstsize);
    size_t __srclen = wcsnlen(__src,__maxlen);
    __dstsize -= (size_t)(__dstend-__dst);
    if (__srclen+1 > __dstsize) return 34;
    while (__srclen--) *__dstend++ = *__src++;
    *__dstend = (wchar_t)'\0';
    return 0;
}
__LOCAL __NONNULL((1,3)) errno_t (__LIBCCALL wcsncpy_s)
(wchar_t *__restrict __dst, size_t __dstsize, wchar_t const *__restrict __src, size_t __max_chars) {
    size_t __srclen = wcsnlen(__src,__maxlen);
    if (__srclen+1 > __dstsize) return 34;
    while (__srclen--) *__dst++ = *__src++;
    *__dst = (wchar_t)'\0';
    return 0;
}
#else
__LIBC __NONNULL((1,3)) errno_t (__LIBCCALL wcscat_s)(wchar_t *__restrict __dst, size_t __dstsize, wchar_t const *__restrict __src);
__LIBC __NONNULL((1,3)) errno_t (__LIBCCALL wcscpy_s)(wchar_t *__restrict __dst, size_t __dstsize, wchar_t const *__restrict __src);
__LIBC __NONNULL((1,3)) errno_t (__LIBCCALL wcsncat_s)(wchar_t *__restrict __dst, size_t __dstsize, wchar_t const *__restrict __src, size_t __max_chars);
__LIBC __NONNULL((1,3)) errno_t (__LIBCCALL wcsncpy_s)(wchar_t *__restrict __dst, size_t __dstsize, wchar_t const *__restrict __src, size_t __max_chars);
#endif
#endif /* __USE_DOS_SLIB */
#endif /* __CRT_DOS */

#ifdef __USE_DOS_SLIB
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcsnlen_s)(wchar_t const *__restrict __src, size_t __max_chars) { return __src ? wcsnlen(__src,__max_chars) : 0; }
#endif /* __USE_DOS_SLIB */
#endif /* !_WSTRING_DEFINED */

__SYSDECL_END

#endif /* !_PARTS_DOS_WSTRING_H */
