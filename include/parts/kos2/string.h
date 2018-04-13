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
#ifndef _PARTS_KOS2_STRING_H
#define _PARTS_KOS2_STRING_H 1

#include <__stdinc.h>
#include "malldefs.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <libc/string.h>
#include <xlocale.h>
#include <bits/types.h>
#ifdef __OPTIMIZE_LIBC__
#include <asm-generic/string.h>
#endif

__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif


#if !defined(__CRT_KOS) && \
    (defined(__CYG_COMPAT__) || defined(__GLC_COMPAT__))
/* GLibC/Cygwin compatibility (NOTE: Doesn't work in KOS) */
__LOCAL __WUNUSED __ATTR_CONST char const *(__LIBCCALL strerror_s)(int __errnum) {
#ifndef ___sys_errlist_defined
 __LIBC char const *const _sys_errlist[]; __LIBC int _sys_nerr;
#endif
 return (unsigned int)__errnum < (unsigned int)_sys_nerr ? _sys_errlist[__errnum] : __NULLPTR;
}
#elif !defined(__CRT_KOS) && defined(__DOS_COMPAT__)
/* DOS/MSVcrt compatibility (NOTE: Doesn't work in KOS) */
#ifndef __int___sys_errlist_defined
#define __int___sys_errlist_defined 1
__NAMESPACE_INT_BEGIN
__LIBC __WUNUSED __ATTR_CONST char **(__LIBCCALL __sys_errlist)(void);
__LIBC __WUNUSED __ATTR_CONST int *(__LIBCCALL __sys_nerr)(void);
__NAMESPACE_INT_END
#endif /* !__int___sys_errlist_defined */
__LOCAL __WUNUSED __ATTR_CONST char const *(__LIBCCALL strerror_s)(int __errnum) {
 return (unsigned int)__errnum < (unsigned int)*(__NAMESPACE_INT_SYM __sys_nerr)() ?
        (__NAMESPACE_INT_SYM __sys_errlist)()[__errnum] : __NULLPTR;
}
#else
/* KOS direct function implementation. */
__LIBC __WUNUSED __ATTR_CONST char const *(__LIBCCALL strerror_s)(int __errnum);
#endif
#if defined(__CRT_KOS) && !defined(__CYG_COMPAT__) && \
   !defined(__GLC_COMPAT__) && !defined(__DOS_COMPAT__)
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_CONST char const *(__LIBCCALL strerrorname_s)(int __errnum);
#endif /* ... */


#ifndef __local_memset
#ifdef __OPTIMIZE_LIBC__
#define __local_memset(dst,byte,n_bytes)                              __opt_memset(dst,byte,n_bytes)
#define __local_memcmp(a,b,n_bytes)                                   __opt_memcmp(a,b,n_bytes)
#define __local_memcpy(dst,src,n_bytes)                               __opt_memcpy(dst,src,n_bytes)
#define __local_memmove(dst,src,n_bytes)                              __opt_memmove(dst,src,n_bytes)
#define __local_mempcpy(dst,src,n_bytes)                              __opt_mempcpy(dst,src,n_bytes)
#define __local_memchr(haystack,needle,n_bytes)                       __opt_memchr(haystack,needle,n_bytes)
#define __local_memrchr(haystack,needle,n_bytes)                      __opt_memrchr(haystack,needle,n_bytes)
#define __local_memend(haystack,needle,n_bytes)                       __opt_memend(haystack,needle,n_bytes)
#define __local_memrend(haystack,needle,n_bytes)                      __opt_memrend(haystack,needle,n_bytes)
#define __local_rawmemchr(haystack,needle)                            __opt_rawmemchr(haystack,needle)
#define __local_rawmemrchr(haystack,needle)                           __opt_rawmemrchr(haystack,needle)
#define __local_memlen(haystack,needle,n_bytes)                       __opt_memlen(haystack,needle,n_bytes)
#define __local_memrlen(haystack,needle,n_bytes)                      __opt_memrlen(haystack,needle,n_bytes)
#define __local_rawmemlen(haystack,needle)                            __opt_rawmemlen(haystack,needle)
#define __local_rawmemrlen(haystack,needle)                           __opt_rawmemrlen(haystack,needle)
#else /* __OPTIMIZE_LIBC__ */
#define __local_memset(dst,byte,n_bytes)                              __libc_memset(dst,byte,n_bytes)
#define __local_memcmp(a,b,n_bytes)                                   __libc_memcmp(a,b,n_bytes)
#define __local_memcpy(dst,src,n_bytes)                               __libc_memcpy(dst,src,n_bytes)
#define __local_memmove(dst,src,n_bytes)                              __libc_memmove(dst,src,n_bytes)
#define __local_mempcpy(dst,src,n_bytes)                              __libc_mempcpy(dst,src,n_bytes)
#define __local_memchr(haystack,needle,n_bytes)                       __libc_memchr(haystack,needle,n_bytes)
#define __local_memrchr(haystack,needle,n_bytes)                      __libc_memrchr(haystack,needle,n_bytes)
#define __local_memend(haystack,needle,n_bytes)                       __libc_memend(haystack,needle,n_bytes)
#define __local_memrend(haystack,needle,n_bytes)                      __libc_memrend(haystack,needle,n_bytes)
#define __local_rawmemchr(haystack,needle)                            __libc_rawmemchr(haystack,needle)
#define __local_rawmemrchr(haystack,needle)                           __libc_rawmemrchr(haystack,needle)
#define __local_memlen(haystack,needle,n_bytes)                       __libc_memlen(haystack,needle,n_bytes)
#define __local_memrlen(haystack,needle,n_bytes)                      __libc_memrlen(haystack,needle,n_bytes)
#define __local_rawmemlen(haystack,needle)                            __libc_rawmemlen(haystack,needle)
#define __local_rawmemrlen(haystack,needle)                           __libc_rawmemrlen(haystack,needle)
#endif /* !__OPTIMIZE_LIBC__ */
#endif /* !__local_memset */

#ifdef __OPTIMIZE_LIBC__
#define __local_memsetw(dst,word,n_words)                             __opt_memsetw(dst,word,n_words)
#define __local_memsetl(dst,dword,n_dwords)                           __opt_memsetl(dst,dword,n_dwords)
#define __local_memsetq(dst,qword,n_qwords)                           __opt_memsetq(dst,qword,n_qwords)
#define __local_memcmpw(a,b,n_words)                                  __opt_memcmpw(a,b,n_words)
#define __local_memcmpl(a,b,n_dwords)                                 __opt_memcmpl(a,b,n_dwords)
#define __local_memcmpq(a,b,n_qwords)                                 __opt_memcmpq(a,b,n_qwords)
#define __local_memcpyw(dst,src,n_words)                              __opt_memcpyw(dst,src,n_words)
#define __local_memcpyl(dst,src,n_dwords)                             __opt_memcpyl(dst,src,n_dwords)
#define __local_memcpyq(dst,src,n_qwords)                             __opt_memcpyq(dst,src,n_qwords)
#define __local_memmovew(dst,src,n_words)                             __opt_memmovew(dst,src,n_words)
#define __local_memmovel(dst,src,n_dwords)                            __opt_memmovel(dst,src,n_dwords)
#define __local_memmoveq(dst,src,n_qwords)                            __opt_memmoveq(dst,src,n_qwords)
#define __local_mempcpyw(dst,src,n_words)                             __opt_mempcpyw(dst,src,n_words)
#define __local_mempcpyl(dst,src,n_dwords)                            __opt_mempcpyl(dst,src,n_dwords)
#define __local_mempcpyq(dst,src,n_qwords)                            __opt_mempcpyq(dst,src,n_qwords)
#define __local_memchrw(haystack,needle,n_words)                      __opt_memchrw(haystack,needle,n_words)
#define __local_memchrl(haystack,needle,n_dwords)                     __opt_memchrl(haystack,needle,n_dwords)
#define __local_memchrq(haystack,needle,n_qwords)                     __opt_memchrq(haystack,needle,n_qwords)
#define __local_memrchrw(haystack,needle,n_words)                     __opt_memrchrw(haystack,needle,n_words)
#define __local_memrchrl(haystack,needle,n_dwords)                    __opt_memrchrl(haystack,needle,n_dwords)
#define __local_memrchrq(haystack,needle,n_qwords)                    __opt_memrchrq(haystack,needle,n_qwords)
#define __local_memendw(haystack,needle,n_words)                      __opt_memendw(haystack,needle,n_words)
#define __local_memendl(haystack,needle,n_dwords)                     __opt_memendl(haystack,needle,n_dwords)
#define __local_memendq(haystack,needle,n_qwords)                     __opt_memendq(haystack,needle,n_qwords)
#define __local_memrendw(haystack,needle,n_words)                     __opt_memrendw(haystack,needle,n_words)
#define __local_memrendl(haystack,needle,n_dwords)                    __opt_memrendl(haystack,needle,n_dwords)
#define __local_memrendq(haystack,needle,n_qwords)                    __opt_memrendq(haystack,needle,n_qwords)
#define __local_rawmemchrw(haystack,needle)                           __opt_rawmemchrw(haystack,needle)
#define __local_rawmemchrl(haystack,needle)                           __opt_rawmemchrl(haystack,needle)
#define __local_rawmemchrq(haystack,needle)                           __opt_rawmemchrq(haystack,needle)
#define __local_rawmemrchrw(haystack,needle)                          __opt_rawmemrchrw(haystack,needle)
#define __local_rawmemrchrl(haystack,needle)                          __opt_rawmemrchrl(haystack,needle)
#define __local_rawmemrchrq(haystack,needle)                          __opt_rawmemrchrq(haystack,needle)
#define __local_memlenw(haystack,needle,n_words)                      __opt_memlenw(haystack,needle,n_words)
#define __local_memlenl(haystack,needle,n_dwords)                     __opt_memlenl(haystack,needle,n_dwords)
#define __local_memlenq(haystack,needle,n_qwords)                     __opt_memlenq(haystack,needle,n_qwords)
#define __local_memrlenw(haystack,needle,n_words)                     __opt_memrlenw(haystack,needle,n_words)
#define __local_memrlenl(haystack,needle,n_dwords)                    __opt_memrlenl(haystack,needle,n_dwords)
#define __local_memrlenq(haystack,needle,n_qwords)                    __opt_memrlenq(haystack,needle,n_qwords)
#define __local_rawmemlenw(haystack,needle)                           __opt_rawmemlenw(haystack,needle)
#define __local_rawmemlenl(haystack,needle)                           __opt_rawmemlenl(haystack,needle)
#define __local_rawmemlenq(haystack,needle)                           __opt_rawmemlenq(haystack,needle)
#define __local_rawmemrlenw(haystack,needle)                          __opt_rawmemrlenw(haystack,needle)
#define __local_rawmemrlenl(haystack,needle)                          __opt_rawmemrlenl(haystack,needle)
#define __local_rawmemrlenq(haystack,needle)                          __opt_rawmemrlenq(haystack,needle)
#else /* __OPTIMIZE_LIBC__ */
#define __local_memsetw(dst,word,n_words)                             __libc_memsetw(dst,word,n_words)
#define __local_memsetl(dst,dword,n_dwords)                           __libc_memsetl(dst,dword,n_dwords)
#define __local_memsetq(dst,qword,n_qwords)                           __libc_memsetq(dst,qword,n_qwords)
#define __local_memcmpw(a,b,n_words)                                  __libc_memcmpw(a,b,n_words)
#define __local_memcmpl(a,b,n_dwords)                                 __libc_memcmpl(a,b,n_dwords)
#define __local_memcmpq(a,b,n_qwords)                                 __libc_memcmpq(a,b,n_qwords)
#define __local_memcpyw(dst,src,n_words)                              __libc_memcpyw(dst,src,n_words)
#define __local_memcpyl(dst,src,n_dwords)                             __libc_memcpyl(dst,src,n_dwords)
#define __local_memcpyq(dst,src,n_qwords)                             __libc_memcpyq(dst,src,n_qwords)
#define __local_memmovew(dst,src,n_words)                             __libc_memmovew(dst,src,n_words)
#define __local_memmovel(dst,src,n_dwords)                            __libc_memmovel(dst,src,n_dwords)
#define __local_memmoveq(dst,src,n_qwords)                            __libc_memmoveq(dst,src,n_qwords)
#define __local_mempcpyw(dst,src,n_words)                             __libc_mempcpyw(dst,src,n_words)
#define __local_mempcpyl(dst,src,n_dwords)                            __libc_mempcpyl(dst,src,n_dwords)
#define __local_mempcpyq(dst,src,n_qwords)                            __libc_mempcpyq(dst,src,n_qwords)
#define __local_memchrw(haystack,needle,n_words)                      __libc_memchrw(haystack,needle,n_words)
#define __local_memchrl(haystack,needle,n_dwords)                     __libc_memchrl(haystack,needle,n_dwords)
#define __local_memchrq(haystack,needle,n_qwords)                     __libc_memchrq(haystack,needle,n_qwords)
#define __local_memrchrw(haystack,needle,n_words)                     __libc_memrchrw(haystack,needle,n_words)
#define __local_memrchrl(haystack,needle,n_dwords)                    __libc_memrchrl(haystack,needle,n_dwords)
#define __local_memrchrq(haystack,needle,n_qwords)                    __libc_memrchrq(haystack,needle,n_qwords)
#define __local_memendw(haystack,needle,n_words)                      __libc_memendw(haystack,needle,n_words)
#define __local_memendl(haystack,needle,n_dwords)                     __libc_memendl(haystack,needle,n_dwords)
#define __local_memendq(haystack,needle,n_qwords)                     __libc_memendq(haystack,needle,n_qwords)
#define __local_memrendw(haystack,needle,n_words)                     __libc_memrendw(haystack,needle,n_words)
#define __local_memrendl(haystack,needle,n_dwords)                    __libc_memrendl(haystack,needle,n_dwords)
#define __local_memrendq(haystack,needle,n_qwords)                    __libc_memrendq(haystack,needle,n_qwords)
#define __local_rawmemchrw(haystack,needle)                           __libc_rawmemchrw(haystack,needle)
#define __local_rawmemchrl(haystack,needle)                           __libc_rawmemchrl(haystack,needle)
#define __local_rawmemchrq(haystack,needle)                           __libc_rawmemchrq(haystack,needle)
#define __local_rawmemrchrw(haystack,needle)                          __libc_rawmemrchrw(haystack,needle)
#define __local_rawmemrchrl(haystack,needle)                          __libc_rawmemrchrl(haystack,needle)
#define __local_rawmemrchrq(haystack,needle)                          __libc_rawmemrchrq(haystack,needle)
#define __local_memlenw(haystack,needle,n_words)                      __libc_memlenw(haystack,needle,n_words)
#define __local_memlenl(haystack,needle,n_dwords)                     __libc_memlenl(haystack,needle,n_dwords)
#define __local_memlenq(haystack,needle,n_qwords)                     __libc_memlenq(haystack,needle,n_qwords)
#define __local_memrlenw(haystack,needle,n_words)                     __libc_memrlenw(haystack,needle,n_words)
#define __local_memrlenl(haystack,needle,n_dwords)                    __libc_memrlenl(haystack,needle,n_dwords)
#define __local_memrlenq(haystack,needle,n_qwords)                    __libc_memrlenq(haystack,needle,n_qwords)
#define __local_rawmemlenw(haystack,needle)                           __libc_rawmemlenw(haystack,needle)
#define __local_rawmemlenl(haystack,needle)                           __libc_rawmemlenl(haystack,needle)
#define __local_rawmemlenq(haystack,needle)                           __libc_rawmemlenq(haystack,needle)
#define __local_rawmemrlenw(haystack,needle)                          __libc_rawmemrlenw(haystack,needle)
#define __local_rawmemrlenl(haystack,needle)                          __libc_rawmemrlenl(haystack,needle)
#define __local_rawmemrlenq(haystack,needle)                          __libc_rawmemrlenq(haystack,needle)
#endif /* !__OPTIMIZE_LIBC__ */

/* KOS String extensions. */
#if defined(__OPTIMIZE_LIBC__) || !defined(__CRT_KOS) || \
   (defined(__DOS_COMPAT__) || defined(__GLC_COMPAT__))
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlen)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memlen(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlen)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrlen(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlen)(void const *__restrict __haystack, int __needle) { return __local_rawmemlen(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlen)(void const *__restrict __haystack, int __needle) { return __local_rawmemrlen(__haystack,__needle); }
#else
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlen)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlen)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlen)(void const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlen)(void const *__restrict __haystack, int __needle);
#endif

#if !defined(__CRT_KOS) || defined(__DOS_COMPAT__) || \
     defined(__GLC_COMPAT__) || defined(__CYG_COMPAT__)
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL stroff)(char const *__restrict __haystack, int __needle) { return __libc_stroff(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strroff)(char const *__restrict __haystack, int __needle) { return __libc_strroff(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strnoff)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnoff(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strnroff)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnroff(__haystack,__needle,__max_chars); }

#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strend)(char *__restrict __str) { return __libc_strend(__str); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strend)(char const *__restrict __str) { return __libc_strend(__str); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnend)(char *__restrict __str, size_t __max_chars) { return __libc_strnend(__str,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strnend)(char const *__restrict __str, size_t __max_chars) { return __libc_strnend(__str,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strrchrnul)(char *__restrict __haystack, int __needle) { return __libc_strrchrnul(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strrchrnul)(char const *__restrict __haystack, int __needle) { return __libc_strrchrnul(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnchr)(char *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char const *(__LIBCCALL strnchr)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnrchr)(char *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnrchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char const *(__LIBCCALL strnrchr)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnrchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnchrnul)(char *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnchrnul(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strnchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnchrnul(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnrchrnul)(char *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnrchrnul(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strnrchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnrchrnul(__haystack,__needle,__max_chars); }
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strend)(char const *__str) { return __libc_strend(__str); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnend)(char const *__str, size_t __max_chars) { return __libc_strnend(__str,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strrchrnul)(char const *__restrict __haystack, int __needle) { return __libc_strrchrnul(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnchr)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnrchr)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnrchr(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnchrnul(__haystack,__needle,__max_chars); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnrchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars) { return __libc_strnrchrnul(__haystack,__needle,__max_chars); }
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */
#else /* Emulate extensions... */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL stroff)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strroff)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strnoff)(char const *__restrict __haystack, int __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strnroff)(char const *__restrict __haystack, int __needle, size_t __max_chars);
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strend,(char *__restrict __str),strend,(__str))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char const *,__LIBCCALL,strend,(char const *__restrict __str),strend,(__str))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strnend,(char *__restrict __str, size_t __max_chars),strnend,(__str,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char const *,__LIBCCALL,strnend,(char const *__restrict __str, size_t __max_chars),strnend,(__str,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strrchrnul,(char *__restrict __haystack, int __needle),strrchrnul,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char const *,__LIBCCALL,strrchrnul,(char const *__restrict __haystack, int __needle),strrchrnul,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,strnchr,(char *__restrict __haystack, int __needle, size_t __max_chars),strnchr,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,strnchr,(char const *__restrict __haystack, int __needle, size_t __max_chars),strnchr,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,strnrchr,(char *__restrict __haystack, int __needle, size_t __max_chars),strnrchr,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,strnrchr,(char const *__restrict __haystack, int __needle, size_t __max_chars),strnrchr,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strnchrnul,(char *__restrict __haystack, int __needle, size_t __max_chars),strnchrnul,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char const *,__LIBCCALL,strnchrnul,(char const *__restrict __haystack, int __needle, size_t __max_chars),strnchrnul,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strnrchrnul,(char *__restrict __haystack, int __needle, size_t __max_chars),strnrchrnul,(__haystack,__needle,__max_chars))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char const *,__LIBCCALL,strnrchrnul,(char const *__restrict __haystack, int __needle, size_t __max_chars),strnrchrnul,(__haystack,__needle,__max_chars))
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strend)(char const *__restrict __str);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnend)(char const *__restrict __str, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strrchrnul)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnchr)(char const *__restrict __haystack, int __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strnrchr)(char const *__restrict __haystack, int __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strnrchrnul)(char const *__restrict __haystack, int __needle, size_t __max_chars);
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */
#endif /* !Emulate extensions... */

#if defined(__OPTIMIZE_LIBC__) || !defined(__CRT_KOS) || \
    defined(__DOS_COMPAT__) || defined(__GLC_COMPAT__)
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL rawmemrchr)(void *__restrict __haystack, int __needle) { return __local_rawmemrchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void const *(__LIBCCALL rawmemrchr)(void const *__restrict __haystack, int __needle) { return __local_rawmemrchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memend)(void *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void const *(__LIBCCALL memend)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memrend)(void *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void const *(__LIBCCALL memrend)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrend(__haystack,__needle,__n_bytes); }
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL rawmemrchr)(void const *__restrict __haystack, int __needle) { return __local_rawmemrchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memend)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memrend)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrend(__haystack,__needle,__n_bytes); }
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */
#else /* Emulate extensions... */
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,rawmemrchr,(void *__restrict __haystack, int __needle),rawmemrchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void const *,__LIBCCALL,rawmemrchr,(void const *__restrict __haystack, int __needle),rawmemrchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,memend,(void *__restrict __haystack, int __needle, size_t __n_bytes),memend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void const *,__LIBCCALL,memend,(void const *__restrict __haystack, int __needle, size_t __n_bytes),memend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,memrend,(void *__restrict __haystack, int __needle, size_t __n_bytes),memrend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void const *,__LIBCCALL,memrend,(void const *__restrict __haystack, int __needle, size_t __n_bytes),memrend,(__haystack,__needle,__n_bytes))
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL rawmemrchr)(void const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memend)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memrend)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */
#endif /* !Emulate extensions... */

#ifndef __KERNEL__
#if defined(__CRT_GLC) && defined(__GLC_COMPAT__)
/* Implement using `asprintf' */
__REDIRECT(__LIBC,,__ssize_t,__LIBCCALL,__libc_vasprintf,(char **__restrict __pstr, char const *__restrict __format, __builtin_va_list __args),vasprintf,(__ptr,__format,__args))
__LOCAL __ATTR_LIBC_PRINTF(1,0) __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC
char *(__LIBCCALL vstrdupf)(char const *__restrict __format, __builtin_va_list __args) {
    char *__result;
    return __libc_vasprintf(&__result,__format,__args) >= 0 ? __result : 0;
}
__LOCAL __ATTR_LIBC_PRINTF(1,2) __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC
char *(__ATTR_CDECL strdupf)(char const *__restrict __format, ...) {
    char *__result; __builtin_va_list __args; __builtin_va_start(__args,__format);
    __result = vstrdupf(__format,__args);
    __builtin_va_end(__args);
    return __result;
}
#elif defined(__CRT_DOS) && defined(__DOS_COMPAT__)
/* Implement using scprintf()+malloc()+sprintf() */
__SYSDECL_END
#include "hybrid/malloc.h"
__SYSDECL_BEGIN
#ifndef ____dos_vscprintf_defined
#define ____dos_vscprintf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_vscprintf,(char const *__restrict __format, __builtin_va_list __args),_vscprintf,(__format,__args))
#endif /* !____dos_vsnprintf_defined */
#ifndef ____libc_vsprintf_defined
#define ____libc_vsprintf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__libc_vsprintf,(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args),vsprintf,(__buf,__format,__args))
#endif /* !____libc_vsprintf_defined */
__LOCAL __ATTR_LIBC_PRINTF(1,0) __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC
char *(__LIBCCALL vstrdupf)(char const *__restrict __format, __builtin_va_list __args) {
    int __resultlen = __dos_vscprintf(__format,__args);
    char *__result = __resultlen >= 0 ? (char *)__hybrid_malloc((__resultlen+1)*sizeof(char)) : 0;
    if (__result) __libc_vsprintf(__result,__format,__args);
    return __result;
}
__LOCAL __ATTR_LIBC_PRINTF(1,2) __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC
char *(__ATTR_CDECL strdupf)(char const *__restrict __format, ...) {
    char *__result; __builtin_va_list __args; __builtin_va_start(__args,__format);
    __result = vstrdupf(__format,__args);
    __builtin_va_end(__args);
    return __result;
}
#else /* ... */
/* Use actual functions exported from libc. */
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_LIBC_PRINTF(1,0) __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC,char *,__LIBCCALL,
                  vstrdupf,(char const *__restrict __format, __builtin_va_list __args),(__format,__args))
__VREDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_LIBC_PRINTF(1,2) __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC,char *,
                   __ATTR_CDECL,strdupf,(char const *__restrict __format, ...),vstrdupf,(__format),__format)
#endif /* !... */
#endif /* !__KERNEL__ */

/* byte/word/dword-wise string operations. */
#ifndef __OPTIMIZE_LIBC__
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,memcpyb,(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes),memcpy,(__dst,__src,__n_bytes))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1))  ,void *,__LIBCCALL,memsetb,(void *__restrict __dst, int __byte, size_t __n_bytes),memset,(__dst,__byte,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int   ,__LIBCCALL,memcmpb,(void const *__a, void const *__b, size_t __n_bytes),memcmp,(__a,__b,__n_bytes))
__REDIRECT(__LIBC,__ATTR_RETNONNULL     __NONNULL((1,2)),void *,__LIBCCALL,memmoveb,(void *__dst, void const *__src, size_t __n_bytes),memmove,(__dst,__src,__n_bytes))
#else
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyb)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes) { return __local_memcpy(__dst,__src,__n_bytes); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetb)(void *__restrict __dst, int __byte, size_t __n_bytes) { return __local_memset(__dst,__byte,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int   (__LIBCCALL memcmpb)(void const *__a, void const *__b, size_t __n_bytes) { return __local_memcmp(__a,__b,__n_bytes); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmoveb)(void *__dst, void const *__src, size_t __n_bytes) { return __local_memmove(__dst,__src,__n_bytes); }
#endif
#if defined(__CRT_KOS) && \
  (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyw)(void *__restrict __dst, void const *__restrict __src, size_t __n_words);
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyl)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords);
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyq)(void *__restrict __dst, void const *__restrict __src, size_t __n_qwords);
__LIBC __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetw)(void *__restrict __dst, __UINT16_TYPE__ __word, size_t __n_words);
__LIBC __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetl)(void *__restrict __dst, __UINT32_TYPE__ __dword, size_t __n_dwords);
__LIBC __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetq)(void *__restrict __dst, __UINT64_TYPE__ __qword, size_t __n_qwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmpw)(void const *__a, void const *__b, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmpl)(void const *__a, void const *__b, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmpq)(void const *__a, void const *__b, size_t __n_qwords);
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmovew)(void *__dst, void const *__src, size_t __n_words);
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmovel)(void *__dst, void const *__src, size_t __n_dwords);
__LIBC __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmoveq)(void *__dst, void const *__src, size_t __n_qwords);
#ifdef __USE_GNU
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,mempcpyb,(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes),mempcpy,(__dst,__src,__n_bytes))
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyw)(void *__restrict __dst, void const *__restrict __src, size_t __n_words);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyl)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyq)(void *__restrict __dst, void const *__restrict __src, size_t __n_qwords);
#endif /* __USE_GNU */
#else /* Builtin... */
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyw)(void *__restrict __dst, void const *__restrict __src, size_t __n_words) { return __local_memcpyw(__dst,__src,__n_words); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyl)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords) { return __local_memcpyl(__dst,__src,__n_dwords); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memcpyq)(void *__restrict __dst, void const *__restrict __src, size_t __n_qwords) { return __local_memcpyq(__dst,__src,__n_qwords); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetw)(void *__restrict __dst, __UINT16_TYPE__ __word, size_t __n_words) { return __local_memsetw(__dst,__word,__n_words); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetl)(void *__restrict __dst, __UINT32_TYPE__ __dword, size_t __n_dwords) { return __local_memsetl(__dst,__dword,__n_dwords); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1))   void *(__LIBCCALL memsetq)(void *__restrict __dst, __UINT64_TYPE__ __qword, size_t __n_qwords) { return __local_memsetq(__dst,__qword,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmpw)(void const *__a, void const *__b, size_t __n_words) { return __local_memcmpw(__a,__b,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmpl)(void const *__a, void const *__b, size_t __n_dwords) { return __local_memcmpl(__a,__b,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmpq)(void const *__a, void const *__b, size_t __n_qwords) { return __local_memcmpq(__a,__b,__n_qwords); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmovew)(void *__dst, void const *__src, size_t __n_words) { return __local_memmovew(__dst,__src,__n_words); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmovel)(void *__dst, void const *__src, size_t __n_dwords) { return __local_memmovel(__dst,__src,__n_dwords); }
__OPT_LOCAL __ATTR_RETNONNULL     __NONNULL((1,2)) void *(__LIBCCALL memmoveq)(void *__dst, void const *__src, size_t __n_qwords) { return __local_memmoveq(__dst,__src,__n_qwords); }
#ifdef __USE_GNU
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyb)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes) { return __local_mempcpy(__dst,__src,__n_bytes); }
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyw)(void *__restrict __dst, void const *__restrict __src, size_t __n_words) { return __local_mempcpyw(__dst,__src,__n_words); }
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyl)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords) { return __local_mempcpyl(__dst,__src,__n_dwords); }
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL mempcpyq)(void *__restrict __dst, void const *__restrict __src, size_t __n_qwords) { return __local_mempcpyq(__dst,__src,__n_qwords); }
#endif /* __USE_GNU */
#endif /* Compat... */


/* Similar to memset(), but fill memory using the given pattern:
 * >> mempatb(addr,0x12,7):
 *    Same as regular memset().
 * >> mempatw(addr,0x12fd,7):
 *    addr&1 == 0: 12fd12fd12fd12
 *    addr&1 == 1:   fd12fd12fd1212
 *    >> `*byte = (__pattern >> 8*((uintptr_t)byte & 0x2)) & 0xff;'
 * >> mempatl(addr,0x12345678,11):
 *    addr&3 == 0: 12345678123
 *    addr&3 == 1:   34567812312
 *    addr&3 == 2:     56781231234
 *    addr&3 == 3:       78123123456
 *    >> `*byte = (__pattern >> 8*((uintptr_t)byte & 0x3)) & 0xff;'
 * WARNING: PATTERN is encoded in host endian, meaning that
 *          byte-order is reversed on little-endian machines. */
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,mempatb,(void *__restrict __dst, int __pattern, size_t __n_bytes),memset,(__dst,__pattern,__n_bytes))
#if defined(__CRT_KOS) && \
  (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__LIBC __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL mempatw)(void *__restrict __dst, __UINT16_TYPE__ __pattern, size_t __n_bytes);
__LIBC __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL mempatl)(void *__restrict __dst, __UINT32_TYPE__ __pattern, size_t __n_bytes);
__LIBC __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL mempatq)(void *__restrict __dst, __UINT64_TYPE__ __pattern, size_t __n_bytes);
#else /* Builtin... */
__LOCAL __ATTR_RETNONNULL __NONNULL((1))
void *(__LIBCCALL mempatw)(void *__restrict __dst, __UINT16_TYPE__ __pattern, size_t __n_bytes) {
    __BYTE_TYPE__ *__iter = (__BYTE_TYPE__ *)__dst;
    if (__n_bytes && (__UINTPTR_TYPE__)__iter & 1) {
        *__iter = ((__UINT8_TYPE__ *)&__pattern)[1];
        ++__iter,--__n_bytes;
    }
    memsetw(__iter,__pattern,__n_bytes/2);
    __iter += __n_bytes,__n_bytes &= 1;
    if (__n_bytes) *__iter = ((__UINT8_TYPE__ *)&__pattern)[0];
    return __dst;
}
__LOCAL __ATTR_RETNONNULL __NONNULL((1))
void *(__LIBCCALL mempatl)(void *__restrict __dst, __UINT32_TYPE__ __pattern, size_t __n_bytes) {
    __BYTE_TYPE__ *__iter = (__BYTE_TYPE__ *)__dst;
    while (__n_bytes && (__UINTPTR_TYPE__)__iter & 3) {
        *__iter = ((__UINT8_TYPE__ *)&__pattern)[(__UINTPTR_TYPE__)__iter & 3];
        ++__iter,--__n_bytes;
    }
    memsetl(__iter,__pattern,__n_bytes/4);
    __iter += __n_bytes,__n_bytes &= 3;
    while (__n_bytes) {
        *__iter = ((__UINT8_TYPE__ *)&__pattern)[(__UINTPTR_TYPE__)__iter & 3];
        ++__iter,--__n_bytes;
    }
    return __dst;
}
__LOCAL __ATTR_RETNONNULL __NONNULL((1))
void *(__LIBCCALL mempatq)(void *__restrict __dst, __UINT64_TYPE__ __pattern, size_t __n_bytes) {
    __BYTE_TYPE__ *__iter = (__BYTE_TYPE__ *)__dst;
    while (__n_bytes && (__UINTPTR_TYPE__)__iter & 7) {
        *__iter = ((__UINT8_TYPE__ *)&__pattern)[(__UINTPTR_TYPE__)__iter & 7];
        ++__iter,--__n_bytes;
    }
    memsetq(__iter,__pattern,__n_bytes/8);
    __iter += __n_bytes,__n_bytes &= 7;
    while (__n_bytes) {
        *__iter = ((__UINT8_TYPE__ *)&__pattern)[(__UINTPTR_TYPE__)__iter & 7];
        ++__iter,--__n_bytes;
    }
    return __dst;
}
#endif /* Compat... */

#if !defined(__OPTIMIZE_LIBC__) && defined(__CRT_KOS) && \
   (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,memlenb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memlen,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,memrlenb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memrlen,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,rawmemlenb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle),rawmemlen,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,rawmemrlenb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle),rawmemrlen,(__haystack,__needle))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle);
#else /* Builtin... */
/* Compatibility/optimized multibyte memory functions. */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memlen(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrlen(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle) { return __local_rawmemlen(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle) { return __local_rawmemrlen(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memlenw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memlenl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memlenq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrlenw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrlenl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL memrlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrlenq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemlenw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemlenl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemlenq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemrlenw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemrlenl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL rawmemrlenq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemrlenq(__haystack,__needle); }
#endif /* Compat... */

#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
#ifndef __OPTIMIZE_LIBC__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memchrb,(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes),memchr,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,memchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memchr,(__haystack,__needle,__n_bytes))
#else /* !__OPTIMIZE_LIBC__ */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memchrb)(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memchr(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL memchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memchr(__haystack,__needle,__n_bytes); }
#endif /* __OPTIMIZE_LIBC__ */
#if !defined(__OPTIMIZE_LIBC__) && defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memrchrb,(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes),memrchr,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,memrchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memrchr,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,rawmemchrb,(__UINT8_TYPE__ *__restrict __haystack, int __needle),rawmemchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,rawmemchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle),rawmemchr,(__haystack,__needle))
#else /* GLC... */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memrchrb)(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memrchr(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL memrchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memrchr(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL rawmemchrb)(__UINT8_TYPE__ *__restrict __haystack, int __needle) { return (__UINT8_TYPE__ *)__local_rawmemchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL rawmemchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle) { return (__UINT8_TYPE__ *)__local_rawmemchr(__haystack,__needle); }
#endif /* !GLC... */
#if !defined(__OPTIMIZE_LIBC__) && defined(__CRT_KOS) && \
   (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,memchrw,(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memchrw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,memchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memchrw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,memchrl,(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memchrl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,memchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memchrl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,memchrq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_dwords),memchrq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,memchrq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_dwords),memchrq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,memrchrw,(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memrchrw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,memrchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memrchrw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,memrchrl,(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memrchrl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,memrchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memrchrl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,memrchrq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords),memrchrq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,memrchrq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords),memrchrq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,rawmemchrw,(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle),rawmemchrw,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,rawmemchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle),rawmemchrw,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,rawmemchrl,(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle),rawmemchrl,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,rawmemchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle),rawmemchrl,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,rawmemchrq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle),rawmemchrq,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,rawmemchrq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle),rawmemchrq,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,rawmemrchrb,(__UINT8_TYPE__ *__restrict __haystack, int __needle),rawmemrchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,rawmemrchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle),rawmemrchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,rawmemrchrw,(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle),rawmemrchrw,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,rawmemrchrw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle),rawmemrchrw,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,rawmemrchrl,(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle),rawmemrchrl,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,rawmemrchrl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle),rawmemrchrl,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,rawmemrchrq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle),rawmemrchrq,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,rawmemrchrq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle),rawmemrchrq,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memendb,(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes),memend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,memendb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,memendw,(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memendw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,memendw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memendw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,memendl,(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memendl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,memendl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memendl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,memendq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords),memendq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,memendq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords),memendq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memrendb,(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes),memrend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ const *,__LIBCCALL,memrendb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memrend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ *,__LIBCCALL,memrendw,(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memrendw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT16_TYPE__ const *,__LIBCCALL,memrendw,(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words),memrendw,(__haystack,__needle,__n_words))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ *,__LIBCCALL,memrendl,(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memrendl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT32_TYPE__ const *,__LIBCCALL,memrendl,(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords),memrendl,(__haystack,__needle,__n_dwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ *,__LIBCCALL,memrendq,(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords),memrendq,(__haystack,__needle,__n_qwords))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT64_TYPE__ const *,__LIBCCALL,memrendq,(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords),memrendq,(__haystack,__needle,__n_qwords))
#else /* KOS... */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memchrw)(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memchrw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL memchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memchrw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memchrl)(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memchrl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL memchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memchrl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrchrw)(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrchrw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL memrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrchrw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrchrl)(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrchrl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL memrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrchrl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL rawmemrchrb)(__UINT8_TYPE__ *__restrict __haystack, int __needle) { return (__UINT8_TYPE__ *)__local_rawmemrchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL rawmemrchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle) { return (__UINT8_TYPE__ *)__local_rawmemrchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemchrw)(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemchrw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL rawmemchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemchrw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemchrl)(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemchrl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL rawmemchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemchrl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemrchrw)(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemrchrw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL rawmemrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemrchrw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemrchrl)(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemrchrl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL rawmemrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemrchrl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memendb)(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL memendb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memrendb)(__UINT8_TYPE__ *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memrend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ const *(__LIBCCALL memrendb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memrend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memendw)(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memendw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL memendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memendw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memendl)(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memendl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL memendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memendl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrendw)(__UINT16_TYPE__ *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrendw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ const *(__LIBCCALL memrendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrendw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrendl)(__UINT32_TYPE__ *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrendl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ const *(__LIBCCALL memrendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrendl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memchrq)(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memchrq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ const *(__LIBCCALL memchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memchrq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrchrq)(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrchrq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ const *(__LIBCCALL memrchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrchrq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemchrq)(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemchrq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ const *(__LIBCCALL rawmemchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemchrq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemrchrq)(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemrchrq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ const *(__LIBCCALL rawmemrchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemrchrq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memendq)(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memendq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ const *(__LIBCCALL memendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memendq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrendq)(__UINT64_TYPE__ *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrendq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ const *(__LIBCCALL memrendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrendq(__haystack,__needle,__n_qwords); }
#endif /* !KOS... */
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
#ifndef __OPTIMIZE_LIBC__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memchr,(__haystack,__needle,__n_bytes))
#else /* !__OPTIMIZE_LIBC__ */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memchr(__haystack,__needle,__n_bytes); }
#endif /* __OPTIMIZE_LIBC__ */
#if !defined(__OPTIMIZE_LIBC__) && defined(__CRT_GLC) && !defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memrchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memrchr,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,rawmemchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle),rawmemchr,(__haystack,__needle))
#else /* GLC... */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memrchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memrchr(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL rawmemchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle) { return (__UINT8_TYPE__ *)__local_rawmemchr(__haystack,__needle); }
#endif /* !GLC... */
#if !defined(__OPTIMIZE_LIBC__) && defined(__CRT_KOS) && \
   (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle);
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,rawmemrchrb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle),rawmemrchr,(__haystack,__needle))
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemrchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle);
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memendb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memend,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),__UINT8_TYPE__ *,__LIBCCALL,memrendb,(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes),memrend,(__haystack,__needle,__n_bytes))
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords);
#else
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memchrw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memchrl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memchrq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrchrw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrchrl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrchrq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL rawmemrchrb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle) { return (__UINT8_TYPE__ *)__local_rawmemrchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemchrw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemchrl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemchrq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL rawmemrchrw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle) { return __local_rawmemrchrw(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL rawmemrchrl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle) { return __local_rawmemrchrl(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL rawmemrchrq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle) { return __local_rawmemrchrq(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memendb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT8_TYPE__ *(__LIBCCALL memrendb)(__UINT8_TYPE__ const *__restrict __haystack, int __needle, size_t __n_bytes) { return (__UINT8_TYPE__ *)__local_memrend(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memendw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memendl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memendq(__haystack,__needle,__n_qwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT16_TYPE__ *(__LIBCCALL memrendw)(__UINT16_TYPE__ const *__restrict __haystack, __UINT16_TYPE__ __needle, size_t __n_words) { return __local_memrendw(__haystack,__needle,__n_words); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT32_TYPE__ *(__LIBCCALL memrendl)(__UINT32_TYPE__ const *__restrict __haystack, __UINT32_TYPE__ __needle, size_t __n_dwords) { return __local_memrendl(__haystack,__needle,__n_dwords); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) __UINT64_TYPE__ *(__LIBCCALL memrendq)(__UINT64_TYPE__ const *__restrict __haystack, __UINT64_TYPE__ __needle, size_t __n_qwords) { return __local_memrendq(__haystack,__needle,__n_qwords); }
#endif
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */


#ifdef __CRT_KOS

/* Fuzzy string compare extensions.
 *  - Lower return values indicate more closely matching data.
 *  - ZERO(0) indicates perfectly matching data. */
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL fuzzy_strcmp)(char const *__a, char const *__b);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcmp)(void const *__a, size_t __a_bytes, void const *__b, size_t __b_bytes);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_strncmp)(char const *__a, size_t __max_a_chars, char const *__b, size_t __max_b_chars);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL fuzzy_strcasecmp)(char const *__a, char const *__b);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcasecmp)(void const *__a, size_t __a_bytes, void const *__b, size_t __b_bytes);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_strncasecmp)(char const *__a, size_t __max_a_chars, char const *__b, size_t __max_b_chars);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL fuzzy_strcasecmp_l)(char const *__a, char const *__b, __locale_t __locale);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_memcasecmp_l)(void const *__a, size_t __a_bytes, void const *__b, size_t __b_bytes, __locale_t __locale);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,3)) size_t (__LIBCCALL fuzzy_strncasecmp_l)(char const *__a, size_t __max_a_chars, char const *__b, size_t __max_b_chars, __locale_t __locale);
/* Perform a wildcard string comparison, returning ZERO(0) upon match, or non-zero when not. */
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wildstrcmp)(char const *__pattern, char const *__string);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wildstrcasecmp)(char const *__pattern, char const *__string);
__LIBC __PORT_KOSONLY __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL wildstrcasecmp_l)(char const *__pattern, char const *__string, __locale_t __locale);
#endif /* __CRT_KOS */

#if !defined(__GLC_COMPAT__) && !defined(__CYG_COMPAT__)
#if __KOS_VERSION__ >= 300 && !defined(__DOS_COMPAT__)
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcasecmp)(void const *__a, void const *__b, size_t __n_bytes);
#else
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,memcasecmp,(void const *__a, void const *__b, size_t __n_bytes),_memicmp,(__a,__b,__n_bytes))
#endif
#ifndef __KERNEL__
#if __KOS_VERSION__ >= 300 && !defined(__DOS_COMPAT__)
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcasecmp_l)(void const *__a, void const *__b, size_t __n_bytes, __locale_t __locale);
#else
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,memcasecmp_l,
          (void const *__a, void const *__b, size_t __n_bytes, __locale_t __locale),_memicmp_l,(__a,__b,__n_bytes,__locale))
#endif
#endif /* !__KERNEL__ */
#else /* !__GLC_COMPAT__ */
#ifndef ____libc_tolower_defined
#define ____libc_tolower_defined 1
__REDIRECT_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,__libc_tolower,(int __c),tolower,(__c))
#endif /* !____libc_tolower_defined */
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2))
int (__LIBCCALL memcasecmp)(void const *__a, void const *__b, size_t __n_bytes) {
    __BYTE_TYPE__ *__ai = (__BYTE_TYPE__ *)__a,*__bi = (__BYTE_TYPE__ *)__b; int __temp;
    while (__n_bytes--) if ((__temp = __libc_tolower(*__ai++) - __libc_tolower(*__bi++)) != 0) return __temp;
    return 0;
}
#ifndef __KERNEL__
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2))
int (__LIBCCALL memcasecmp_l)(void const *__a, void const *__b, size_t __n_bytes,
                              __locale_t __UNUSED(__locale)) {
    return memcasecmp(__a,__b,__n_bytes);
}
#endif /* !__KERNEL__ */
#endif /* __GLC_COMPAT__ */


#ifndef __KERNEL__

#ifdef __DOS_COMPAT__
__SYSDECL_END
#include <parts/kos2/amalloc.h>
__SYSDECL_BEGIN
#ifndef ____dos_vscprintf_defined
#define ____dos_vscprintf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_vscprintf,(char const *__restrict __format, __builtin_va_list __args),_vscprintf,(__format,__args))
#endif /* !____dos_vsnprintf_defined */
#ifndef ____libc_vsprintf_defined
#define ____libc_vsprintf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__libc_vsprintf,(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args),vsprintf,(__buf,__format,__args))
#endif /* !____libc_vsprintf_defined */

#ifndef __NO_ASMNAME
__LIBC __ATTR_LIBC_PRINTF(1,2) int (__ATTR_CDECL __dos_scprintf)(char const *__restrict __format, ...) __ASMNAME("_scprintf");
__LIBC __ATTR_LIBC_PRINTF(2,3) __ssize_t (__ATTR_CDECL __libc_sprintf)(char *__restrict __buf, char const *__restrict __format, ...) __ASMNAME("sprintf");
#else /* !__NO_ASMNAME */
#define __dos_scprintf(...) _scprintf(__VA_ARGS__)
#define __libc_sprintf(...) __NAMESPACE_STD_SYM sprintf(__VA_ARGS__)
#ifndef ___scprintf_defined
#define ___scprintf_defined 1
#ifdef __USE_KOS_STDEXT
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(1,0),__ssize_t,__LIBCCALL,vscprintf,(char const *__restrict __format, __builtin_va_list __args),(__format,__args))
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(1,2),__ssize_t,__ATTR_CDECL,scprintf,(char const *__restrict __format, ...),vscprintf,(__format),__format)
#else
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(1,0),int,__LIBCCALL,vscprintf,(char const *__restrict __format, __builtin_va_list __args),(__format,__args))
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(1,2),int,__ATTR_CDECL,scprintf,(char const *__restrict __format, ...),vscprintf,(__format),__format)
#endif
#endif /* !___scprintf_defined */
#ifndef __std_sprintf_defined
#define __std_sprintf_defined
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_LIBC_PRINTF(2,3) __ssize_t (__ATTR_CDECL sprintf)(char *__restrict __buf, char const *__restrict __format, ...);
__NAMESPACE_STD_END
#endif /* !__std_sprintf_defined */
#ifndef __sprintf_defined
#define __sprintf_defined
__NAMESPACE_STD_USING(sprintf)
#endif /* !__sprintf_defined */
#endif /* __NO_ASMNAME */
__LOCAL __ATTR_LIBC_PRINTF(2,3) char *(__ATTR_CDECL __forward_sprintf)(char *__restrict __buf, char const *__restrict __format, ...) { __builtin_va_list __args; __builtin_va_start(__args,__format); __libc_vsprintf(__buf,__format,__args); __builtin_va_end(__args); return __buf; }
__LOCAL __ATTR_LIBC_PRINTF(2,0) char *(__LIBCCALL __forward_vsprintf)(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args) { __libc_vsprintf(__buf,__format,__args); return __buf; }

/* Without dedicated Libc functionality, double-evaluation can't be prevented. */
#define strdupaf(...) \
      __forward_sprintf((char *)__ALLOCA(((size_t)__dos_scprintf(__VA_ARGS__)+1)*sizeof(char)),__VA_ARGS__)
#ifdef __NO_XBLOCK
#define vstrdupaf(format,args) \
      __forward_vsprintf((char *)__ALLOCA(((size_t)__dos_vscprintf(format,args)+1)*sizeof(char)),format,args)
#else /* __NO_XBLOCK */
#define vstrdupaf(format,args) \
  __XBLOCK({ char const *const __format = (format); \
             __builtin_va_list __args = (args); \
             __XRETURN __forward_vsprintf((char *)__ALLOCA(((size_t) \
                       __dos_vscprintf(__format,__args)+1)*sizeof(char)),__format,__args); \
  })
#endif /* !__NO_XBLOCK */

#elif defined(__GLC_COMPAT__) || defined(__CYG_COMPAT__)
__SYSDECL_END
#include <hybrid/alloca.h>
__SYSDECL_BEGIN

#ifndef ____libc_vsprintf_defined
#define ____libc_vsprintf_defined 1
__REDIRECT(__LIBC,__ATTR_LIBC_PRINTF(2,0),int,__LIBCCALL,__libc_vsprintf,(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args),vsprintf,(__buf,__format,__args))
#endif /* !____libc_vsprintf_defined */

#ifndef ____libc_vsnprintf_defined
#define ____libc_vsnprintf_defined 1
__REDIRECT(__LIBC,__ATTR_LIBC_PRINTF(3,0),int,__LIBCCALL,__libc_vsnprintf,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args),vsnprintf,(__buf,__buflen,__format,__args))
#endif /* !____libc_vsnprintf_defined */

#ifndef __NO_ASMNAME
__LIBC __ATTR_LIBC_PRINTF(2,3) __ssize_t (__ATTR_CDECL __libc_sprintf)(char *__restrict __buf, char const *__restrict __format, ...) __ASMNAME("sprintf");
__LIBC __ATTR_LIBC_PRINTF(3,0) __ssize_t (__ATTR_CDECL __libc_snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) __ASMNAME("snprintf");
#else /* !__NO_ASMNAME */
#define __libc_sprintf(...)   __NAMESPACE_STD_SYM sprintf(__VA_ARGS__)
#define __libc_snprintf(...)  __NAMESPACE_STD_SYM snprintf(__VA_ARGS__)
#ifndef __std_sprintf_defined
#define __std_sprintf_defined
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_LIBC_PRINTF(2,3) __ssize_t (__ATTR_CDECL sprintf)(char *__restrict __buf, char const *__restrict __format, ...);
__NAMESPACE_STD_END
#endif /* !__std_sprintf_defined */
#ifndef __std_snprintf_defined
#define __std_snprintf_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_LIBC_PRINTF(3,4) __ssize_t (__ATTR_CDECL snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...);
__NAMESPACE_STD_END
#endif /* !__std_snprintf_defined */
#ifndef __sprintf_defined
#define __sprintf_defined
__NAMESPACE_STD_USING(sprintf)
#endif /* !__sprintf_defined */
#ifndef __snprintf_defined
#define __snprintf_defined 1
__NAMESPACE_STD_USING(snprintf)
#endif /* !__snprintf_defined */
#endif /* __NO_ASMNAME */

__LOCAL __ATTR_LIBC_PRINTF(2,3) char *(__ATTR_CDECL __forward_sprintf)(char *__restrict __buf, char const *__restrict __format, ...) { __builtin_va_list __args; __builtin_va_start(__args,__format); __libc_vsprintf(__buf,__format,__args); __builtin_va_end(__args); return __buf; }
__LOCAL __ATTR_LIBC_PRINTF(2,0) char *(__LIBCCALL __forward_vsprintf)(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args) { __libc_vsprintf(__buf,__format,__args); return __buf; }

/* TODO: Better code for handling `__DOS_COMPAT__' here. */

/* Without dedicated Libc functionality, double-evaluation can't be prevented. */
#define strdupaf(...) \
      __forward_sprintf((char *)__ALLOCA(((size_t)__libc_snprintf(NULL,0,__VA_ARGS__)+1)*sizeof(char)),__VA_ARGS__)
#ifdef __NO_XBLOCK
#define vstrdupaf(format,args) \
      __forward_vsprintf((char *)__ALLOCA(((size_t)__libc_snprintf(NULL,0,format,args)+1)*sizeof(char)),format,args)
#else /* __NO_XBLOCK */
#define vstrdupaf(format,args) \
  __XBLOCK({ char const *const __format = (format); \
             __builtin_va_list __args = (args); \
             __XRETURN __forward_vsprintf((char *)__ALLOCA(((size_t) \
                       __libc_snprintf(NULL,0,__format,__args)+1)*sizeof(char)),__format,__args); \
  })
#endif /* !__NO_XBLOCK */

#else /* Compat... */
/* >> char *strdupaf(char const *__restrict format, ...);
 * String duplicate as fu$k!
 * Similar to strdupf, but allocates memory of the stack, instead of the heap.
 * While this function is _very_ useful, be warned that due to the way variadic
 * arguments are managed by cdecl (the only calling convention possible to use
 * for them on most platforms) it is nearly impossible not to waste the stack
 * space that was originally allocated for the arguments (Because in cdecl, the
 * callee is responsible for argument cleanup).
 * ANYWAYS: Since its the stack, it shouldn't really matter, but please be advised
 *          that use of these functions fall under the same restrictions as all
 *          other alloca-style functions.
 * >> int open_file_in_folder(char const *folder, char const *file) {
 * >>   return open(strdupaf("%s/%s",folder,file),O_RDONLY);
 * >> }
 */
__LIBC __ATTR_LIBC_PRINTF(1,2) __WUNUSED __ATTR_MALLOC char *(__ATTR_CDECL strdupaf)(char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(1,0) __WUNUSED __ATTR_MALLOC char *(__LIBCCALL vstrdupaf)(char const *__restrict __format, __builtin_va_list __args);

#ifdef __INTELLISENSE__
#elif defined(__GNUC__)
/* Dear GCC devs: WHY IS THERE NO `__attribute__((alloca))'?
 * Or better yet! Add something like: `__attribute__((clobber("%esp")))'
 *
 * Here's what the hacky code below does:
 * We must use `__builtin_alloca' to inform the compiler that the stack pointer
 * contract has been broken, meaning that %ESP can (no longer) be used for offsets.
 * NOTE: If you don't believe me that this is required, and think this is just me
 *       ranting about missing GCC functionality, try the following code yourself:
 * >> printf("path = `%s'\n",strdupaf("%s/%s","/usr","lib")); // OK (Also try cloning this line a bunch of times)
 * >> #undef strdupaf
 * >> printf("path = `%s'\n",strdupaf("%s/%s","/usr","lib")); // Breaks
 *
 * NOTE: We also can't do __builtin_alloca(0) because that's optimized away too early
 *       and the compiler will (correctly) not mark %ESP as clobbered internally.
 *       So we're left with no choice but to waste another bit of
 *       stack memory, and more importantly: instructions!
 * Oh and by-the-way: Unlike with str(n)dupa It's only possible to implement
 *                    this as a dedicated function, when wanting to ensure
 *                    one-time evaluation of the variadic arguments.
 *                 -> So we can't just implement the whole thing as a macro.
 *                   (OK: '_vstrdupaf' could be, but when are you even going to use any to begin with...)
 * HINT: A standard-compliant, but double-evaluating version would look something like this:
 * >> #define strdupaf(...) \
 * >>   ({ char *result; size_t s;\
 * >>      s = (snprintf(NULL,0,__VA_ARGS__)+1)*sizeof(char);\
 * >>      result = (char *)__builtin_alloca(s);\
 * >>      snprintf(result,s,__VA_ARGS__);\
 * >>      result;\
 * >>   })
 */
#define strdupaf(...) \
 __XBLOCK({ char *const __sdares = strdupaf(__VA_ARGS__);\
            (void)__builtin_alloca(1);\
            __XRETURN __sdares;\
 })
#define vstrdupaf(fmt,args) \
 __XBLOCK({ char *__sdares = _vstrdupaf(fmt,args);\
            (void)__builtin_alloca(1);\
            __XRETURN __sdares;\
 })
#else
/* This might not work, because the compiler has no
 * idea these functions are violating the stack layout. */
#endif
#endif /* Builtin... */

#ifdef __CRT_DOS
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strlwr_l,(char *__restrict __str, __locale_t __locale),(__str,__locale))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strupr_l,(char *__restrict __str, __locale_t __locale),(__str,__locale))
__REDIRECT_DPA(__LIBC,__ATTR_PURE __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strncoll,(char const *__str1, char const *__str2, size_t __max_chars),(__str1,__str2,__max_chars))
__REDIRECT_DPA(__LIBC,__ATTR_PURE __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strncoll_l,(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __locale),(__str1,__str2,__max_chars,__locale))
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strcasecoll,(char const *__str1, char const *__str2),_stricoll,(__str1,__str2))
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strcasecoll_l,(char const *__str1, char const *__str2, __locale_t __locale),_stricoll_l,(__str1,__str2,__locale))
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strncasecoll,(char const *__str1, char const *__str2, size_t __max_chars),_strnicoll,(__str1,__str2,__max_chars))
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __PORT_DOSONLY __NONNULL((1,2)),int,__LIBCCALL,strncasecoll_l,(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __locale),_strnicoll_l,(__str1,__str2,__max_chars,__locale))
#else
__LIBC __ATTR_PURE __WUNUSED __PORT_DOSONLY __NONNULL((1,2)) int (__LIBCCALL strcasecoll)(char const *__str1, char const *__str2);
__LIBC __ATTR_PURE __WUNUSED __PORT_DOSONLY __NONNULL((1,2)) int (__LIBCCALL strcasecoll_l)(char const *__str1, char const *__str2, __locale_t __locale);
__LIBC __ATTR_PURE __WUNUSED __PORT_DOSONLY __NONNULL((1,2)) int (__LIBCCALL strncasecoll)(char const *__str1, char const *__str2, size_t __max_chars);
__LIBC __ATTR_PURE __WUNUSED __PORT_DOSONLY __NONNULL((1,2)) int (__LIBCCALL strncasecoll_l)(char const *__str1, char const *__str2, size_t __max_chars, __locale_t __locale);
#endif

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
#endif /* !__KERNEL__ */


#ifdef __USE_DEBUG
#include "hybrid/debuginfo.h"
#if __USE_DEBUG != 0 && defined(__CRT_KOS) && \
  (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,_memcpyb_d,(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes, __DEBUGINFO),_memcpy_d,(__dst,__src,__n_bytes,__DEBUGINFO_FWD))
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _memcpyw_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_words, __DEBUGINFO);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _memcpyl_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords, __DEBUGINFO);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _memcpyq_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_qwords, __DEBUGINFO);
#ifdef __USE_GNU
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,_mempcpyb_d,(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes, __DEBUGINFO),_mempcpy_d,(__dst,__src,__n_bytes,__DEBUGINFO_FWD))
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _mempcpyw_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_words, __DEBUGINFO);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _mempcpyl_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_dwords, __DEBUGINFO);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _mempcpyq_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_qwords, __DEBUGINFO);
#endif /* __USE_GNU */
#else /* __USE_DEBUG != 0 */
#   define _memcpyb_d(dst,src,n_bytes,...)  memcpyb(dst,src,n_bytes)
#   define _memcpyw_d(dst,src,n_words,...)  memcpyw(dst,src,n_words)
#   define _memcpyl_d(dst,src,n_dwords,...) memcpyl(dst,src,n_dwords)
#   define _memcpyq_d(dst,src,n_qwords,...) memcpyq(dst,src,n_qwords)
#ifdef __USE_GNU
#   define _mempcpyb_d(dst,src,n_bytes,...)  mempcpyb(dst,src,n_bytes)
#   define _mempcpyw_d(dst,src,n_words,...)  mempcpyw(dst,src,n_words)
#   define _mempcpyl_d(dst,src,n_dwords,...) mempcpyl(dst,src,n_dwords)
#   define _mempcpyq_d(dst,src,n_dwords,...) mempcpyq(dst,src,n_qwords)
#endif /* __USE_GNU */
#endif /* __USE_DEBUG == 0 */
#ifdef __USE_DEBUG_HOOK
#ifndef __OPTIMIZE_LIBC__
#   define memcpyb(dst,src,n_bytes)   _memcpyb_d(dst,src,n_bytes,__DEBUGINFO_GEN)
#   define memcpyw(dst,src,n_words)   _memcpyw_d(dst,src,n_words,__DEBUGINFO_GEN)
#   define memcpyl(dst,src,n_dwords)  _memcpyl_d(dst,src,n_dwords,__DEBUGINFO_GEN)
#   define memcpyq(dst,src,n_qwords)  _memcpyq_d(dst,src,n_qwords,__DEBUGINFO_GEN)
#ifdef __USE_GNU
#   define mempcpyb(dst,src,n_bytes)  _mempcpyb_d(dst,src,n_bytes,__DEBUGINFO_GEN)
#   define mempcpyw(dst,src,n_words)  _mempcpyw_d(dst,src,n_words,__DEBUGINFO_GEN)
#   define mempcpyl(dst,src,n_dwords) _mempcpyl_d(dst,src,n_dwords,__DEBUGINFO_GEN)
#   define mempcpyq(dst,src,n_qwords) _mempcpyq_d(dst,src,n_qwords,__DEBUGINFO_GEN)
#endif /* __USE_GNU */
#endif /* !__OPTIMIZE_LIBC__ */
#endif /* __USE_DEBUG_HOOK */
#endif /* __USE_DEBUG */

__SYSDECL_END

#ifdef _WCHAR_H
#ifndef _PARTS_KOS2_WSTRING_H
#include "wstring.h"
#endif
#endif

#ifdef __USE_KOS3
#ifndef _PARTS_KOS3_STRING_H
#include <parts/kos3/string.h>
#endif
#endif

#endif /* !_PARTS_KOS2_STRING_H */
