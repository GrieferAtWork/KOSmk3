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
#ifndef _PARTS_KOS2_WSTRING_H
#define _PARTS_KOS2_WSTRING_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#ifndef _STRING_H
#include <string.h>
#endif
#ifndef _WCHAR_H
#include <wchar.h>
#endif

__SYSDECL_BEGIN

#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */

#ifndef __KERNEL__

#if (defined(__DOS_COMPAT__) || defined(__GLC_COMPAT__)) || !defined(__CRT_KOS)
#ifndef __wcsnlen_defined
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,__libc_wcsnlen,
          (wchar_t const *__restrict __s, size_t __maxlen),wcsnlen,(__s,__maxlen))
#else /* !__wcsnlen_defined */
#define __libc_wcsnlen(s,maxlen) wcsnlen(s,maxlen)
#endif /* __wcsnlen_defined */
#ifdef __CORRECT_ISO_CPP_WCHAR_H_PROTO
extern "C++" {
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsend)(wchar_t *__restrict __s) { return __s+__NAMESPACE_STD_SYM wcslen(__s); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsnend)(wchar_t *__restrict __s, size_t __n) { return __s+__libc_wcsnlen(__s,__n); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t const *(__LIBCCALL wcsend)(wchar_t const *__restrict __s) { return __s+__NAMESPACE_STD_SYM wcslen(__s); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t const *(__LIBCCALL wcsnend)(wchar_t const *__restrict __s, size_t __n) { return __s+__libc_wcsnlen(__s,__n); }
}
#else /* __CORRECT_ISO_CPP_WCHAR_H_PROTO */
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsend)(wchar_t const *__restrict __s) { return (wchar_t *)__s+__NAMESPACE_STD_SYM wcslen(__s); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsnend)(wchar_t const *__restrict __s, size_t __n) { return (wchar_t *)__s+__libc_wcsnlen(__s,__n); }
#endif /* !__CORRECT_ISO_CPP_WCHAR_H_PROTO */
#else /* Compat... */
#ifdef __CORRECT_ISO_CPP_WCHAR_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t *,__LIBCCALL,wcsend,(wchar_t *__restrict __s),wcsend,(__s))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t const *,__LIBCCALL,wcsend,(wchar_t const *__restrict __s),wcsend,(__s))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t *,__LIBCCALL,wcsnend,(wchar_t *__restrict __s, size_t __n),wcsnend,(__s,__n))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t const *,__LIBCCALL,wcsnend,(wchar_t const *__restrict __s, size_t __n),wcsnend,(__s,__n))
}
#else /* __CORRECT_ISO_CPP_WCHAR_H_PROTO */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsend)(wchar_t const *__restrict __s);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsnend)(wchar_t const *__restrict __s, size_t __n);
#endif /* !__CORRECT_ISO_CPP_WCHAR_H_PROTO */
#endif /* Builtin... */

#ifndef __USE_GNU
#ifdef __CORRECT_ISO_CPP_WCHAR_H_PROTO
extern "C++" {
#if defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __NONNULL((1)),wchar_t *,__LIBCCALL,wcschrnul,(wchar_t *__haystack, wchar_t __needle),wcschrnul,(__haystack,__needle))
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __NONNULL((1)),wchar_t const *,__LIBCCALL,wcschrnul,(wchar_t const *__haystack, wchar_t __needle),wcschrnul,(__haystack,__needle))
#else /* __CRT_GLC */
__LOCAL __ATTR_PURE __WUNUSED __NONNULL((1)) wchar_t *(__LIBCCALL wcschrnul)(wchar_t *__haystack, wchar_t __needle) { wchar_t *__iter = __haystack; for (; *__iter && *__iter != __needle; ++__iter); return __iter; }
__LOCAL __ATTR_PURE __WUNUSED __NONNULL((1)) wchar_t const *(__LIBCCALL wcschrnul)(wchar_t const *__haystack, wchar_t __needle) { wchar_t const *__iter = __haystack; for (; *__iter && *__iter != __needle; ++__iter); return __iter; }
#endif /* !__CRT_GLC */
}
#else /* __CORRECT_ISO_CPP_WCHAR_H_PROTO */
#if defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) wchar_t *(__LIBCCALL wcschrnul)(wchar_t const *__haystack, wchar_t __needle);
#else /* __CRT_GLC */
__LOCAL __ATTR_PURE __WUNUSED __NONNULL((1)) wchar_t *(__LIBCCALL wcschrnul)(wchar_t const *__haystack, wchar_t __needle) { wchar_t *__iter = (wchar_t *)__haystack; for (; *__iter && *__iter != __needle; ++__iter); return __iter; }
#endif /* !__CRT_GLC */
#endif /* !__CORRECT_ISO_CPP_WCHAR_H_PROTO */
#endif /* !__USE_GNU */

#if !defined(__CRT_KOS) || defined(__DOS_COMPAT__) || defined(__GLC_COMPAT__) || defined(__CYG_COMPAT__)
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *
(__LIBCCALL __local_wcsend)(wchar_t const *__restrict __wcs) {
    while (*__wcs) ++__wcs;
    return (wchar_t *)__wcs;
}
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1))
wchar_t *(__LIBCCALL __local_wcsnend)(wchar_t const *__restrict __wcs, size_t __max_chars) {
    wchar_t const *__end = __wcs+__max_chars;
    while (__wcs != __end && *__wcs) ++__wcs;
    return (wchar_t *)__wcs;
}
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1))
wchar_t *(__LIBCCALL __local_wcsrchrnul)(wchar_t const *__restrict __haystack, wint_t __needle) {
    wchar_t const *__result = __haystack-1,*__iter = __haystack;
    do if (*__iter == __needle) __result = __iter;
    while (*__iter++);
    return (wchar_t *)__result;
}
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1))
wchar_t *(__LIBCCALL __local_wcsnchr)(wchar_t const *__restrict __haystack,
                                      wint_t __needle, size_t __max_chars) {
    wchar_t const *__end = __haystack+__max_chars;
    for (; __haystack != __end && *__haystack; ++__haystack)
        if (*__haystack == __needle)
            return (wchar_t *)__haystack;
    return __NULLPTR;
}
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1))
wchar_t *(__LIBCCALL __local_wcsnrchr)(wchar_t const *__restrict __haystack,
                                       wint_t __needle, size_t __max_chars) {
    wchar_t const *__end = __haystack+__max_chars,*__result = NULL;
    for (; __haystack != __end && *__haystack; ++__haystack)
        if (*__haystack == __needle)
            __result = __haystack;
    return (wchar_t *)__result;
}
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1))
wchar_t *(__LIBCCALL __local_wcsnchrnul)(wchar_t const *__restrict __haystack,
                                         wint_t __needle, size_t __max_chars) {
    wchar_t const *__end = __haystack+__max_chars;
    for (; __haystack != __end &&
          *__haystack &&
          *__haystack != __needle;
         ++__haystack);
    return (wchar_t *)__haystack;
}
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1))
wchar_t *(__LIBCCALL __local_wcsnrchrnul)(wchar_t const *__restrict __haystack,
                                          wint_t __needle, size_t __max_chars) {
    wchar_t const *__end = __haystack+__max_chars,*__result = __haystack-1;
    for (; __haystack != __end && *__haystack; ++__haystack)
        if (*__haystack == __needle)
            __result = __haystack;
    return (wchar_t *)__result;
}

#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsend)(wchar_t *__restrict __wcs) { return __local_wcsend(__wcs); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t const *(__LIBCCALL wcsend)(wchar_t const *__restrict __wcs) { return __local_wcsend(__wcs); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsnend)(wchar_t *__restrict __wcs, size_t __max_chars) { return __local_wcsnend(__wcs,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t const *(__LIBCCALL wcsnend)(wchar_t const *__restrict __wcs, size_t __max_chars) { return __local_wcsnend(__wcs,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsrchrnul)(wchar_t *__restrict __haystack, wint_t __needle) { return __local_wcsrchrnul(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t const *(__LIBCCALL wcsrchrnul)(wchar_t const *__restrict __haystack, wint_t __needle) { return __local_wcsrchrnul(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsnchr)(wchar_t *__restrict __haystack, wint_t __needle, size_t __max_chars) { return __local_wcsnchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t const *(__LIBCCALL wcsnchr)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars) { return __local_wcsnchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsnrchr)(wchar_t *__restrict __haystack, wint_t __needle, size_t __max_chars) { return __local_wcsnrchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t const *(__LIBCCALL wcsnrchr)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars) { return __local_wcsnrchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsnchrnul)(wchar_t *__restrict __haystack, wint_t __needle, size_t __max_chars) { return __local_wcsnchrnul(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t const *(__LIBCCALL wcsnchrnul)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars) { return __local_wcsnchrnul(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsnrchrnul)(wchar_t *__restrict __haystack, wint_t __needle, size_t __max_chars) { return __local_wcsnrchrnul(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t const *(__LIBCCALL wcsnrchrnul)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars) { return __local_wcsnrchrnul(__haystack,__needle,__max_chars); }
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsend)(wchar_t const *__wcs) { return __local_wcsend(__wcs); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsnend)(wchar_t const *__wcs, size_t __max_chars) { return __local_wcsnend(__wcs,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsrchrnul)(wchar_t const *__restrict __haystack, wint_t __needle) { return __local_wcsrchrnul(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsnchr)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars) { return __local_wcsnchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsnrchr)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars) { return __local_wcsnrchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsnchrnul)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars) { return __local_wcsnchrnul(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsnrchrnul)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars) { return __local_wcsnrchrnul(__haystack,__needle,__max_chars); }
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcsoff)(wchar_t const *__restrict __haystack, wint_t __needle) { return (size_t)(wcschrnul(__haystack,__needle)-__haystack); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcsroff)(wchar_t const *__restrict __haystack, wint_t __needle) { return (size_t)(__local_wcsrchrnul(__haystack,__needle)-__haystack); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcsnoff)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars) { return (size_t)(__local_wcsnchrnul(__haystack,__needle,__max_chars)-__haystack); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcsnroff)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars) { return (size_t)(wcsnrchrnul(__haystack,__needle,__max_chars)-__haystack); }
#else /* Emulate extensions... */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcsoff)(wchar_t const *__restrict __haystack, wint_t __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcsroff)(wchar_t const *__restrict __haystack, wint_t __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcsnoff)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL wcsnroff)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars);
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsend,(wchar_t *__restrict __wcs),wcsend,(__wcs))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),wchar_t const *,__LIBCCALL,wcsend,(wchar_t const *__restrict __wcs),wcsend,(__wcs))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsnend,(wchar_t *__restrict __wcs, size_t __max_chars),wcsnend,(__wcs,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),wchar_t const *,__LIBCCALL,wcsnend,(wchar_t const *__restrict __wcs, size_t __max_chars),wcsnend,(__wcs,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsrchrnul,(wchar_t *__restrict __haystack, wint_t __needle),wcsrchrnul,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),wchar_t const *,__LIBCCALL,wcsrchrnul,(wchar_t const *__restrict __haystack, wint_t __needle),wcsrchrnul,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t *,__LIBCCALL,wcsnchr,(wchar_t *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchr,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t const *,__LIBCCALL,wcsnchr,(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchr,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t *,__LIBCCALL,wcsnrchr,(wchar_t *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchr,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),wchar_t const *,__LIBCCALL,wcsnrchr,(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchr,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsnchrnul,(wchar_t *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchrnul,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),wchar_t const *,__LIBCCALL,wcsnchrnul,(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnchrnul,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),wchar_t *,__LIBCCALL,wcsnrchrnul,(wchar_t *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchrnul,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),wchar_t const *,__LIBCCALL,wcsnrchrnul,(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars),wcsnrchrnul,(__haystack,__needle,__max_chars))
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsend)(wchar_t const *__restrict __wcs);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsnend)(wchar_t const *__restrict __wcs, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsrchrnul)(wchar_t const *__restrict __haystack, wint_t __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsnchr)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) wchar_t *(__LIBCCALL wcsnrchr)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsnchrnul)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wcsnrchrnul)(wchar_t const *__restrict __haystack, wint_t __needle, size_t __max_chars);
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */
#endif /* !Emulate extensions... */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_PARTS_KOS2_WSTRING_H */
