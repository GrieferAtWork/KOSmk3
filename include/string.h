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
#ifndef _STRING_H
#define _STRING_H 1

#include "__stdinc.h"
#include "parts/kos2/malldefs.h"
#include "features.h"
#include "hybrid/typecore.h"
#include "libc/string.h"
#include "xlocale.h"
#include <bits/types.h>
#ifdef __OPTIMIZE_LIBC__
#include <asm-generic/string.h>
#endif

#ifdef __GNUC__
#pragma GCC system_header
#endif

/* Memory functions (An optional `[b|w|l|q]' suffix is a KOS extension):
 *   [STD] memset[b|w|l|q]     - Fill memory with the given byte/word/dword
 *   [STD] memcpy[b|w|l|q]     - Copy memory between non-overlapping memory blocks.
 *   [GLC] mempcpy[b|w|l|q]    - Same as `memcpy[b|w|l|q]', but return `DST+N_(BYTES|WORDS|DWORDS)', rather than `DST'
 *   [STD] memmove[b|w|l|q]    - Move memory between potentially overlapping memory blocks.
 *   [STD] memchr[b|w|l|q]     - Ascendingly search for `NEEDLE', starting at `HAYSTACK'. - Return `NULL' if `NEEDLE' wasn't found.
 *   [GLC] memrchr[b|w|l|q]    - Descendingly search for `NEEDLE', starting at `HAYSTACK+N_(BYTES|WORDS|DWORDS)'. - Return `NULL' if `NEEDLE' wasn't found.
 *   [GLC] rawmemchr[b|w|l|q]  - Same as `memchr[b|w|l|q]' with a search limit of `(size_t)-1/sizeof(T)'
 *   [KOS] rawmemrchr[b|w|l|q] - Same as `memrchr[b|w|l|q]' without a search limit, starting at `HAYSTACK-sizeof(T)'
 *   [KOS] memend[b|w|l|q]     - Same as `memchr[b|w|l|q]', but return `HAYSTACK+N_(BYTES|WORDS|DWORDS)', rather than `NULL' if `NEEDLE' wasn't found.
 *   [KOS] memrend[b|w|l|q]    - Same as `memrchr[b|w|l|q]', but return `HAYSTACK-1', rather than `NULL' if `NEEDLE' wasn't found.
 *   [KOS] memlen[b|w|l|q]     - Same as `memend[b|w|l|q]', but return the offset from `HAYSTACK', rather than the actual address.
 *   [KOS] memrlen[b|w|l|q]    - Same as `memrend[b|w|l|q]', but return the offset from `HAYSTACK', rather than the actual address.
 *   [KOS] rawmemlen[b|w|l|q]  - Same as `rawmemchr[b|w|l|q]', but return the offset from `HAYSTACK', rather than the actual address.
 *   [KOS] rawmemrlen[b|w|l|q] - Same as `rawmemrchr[b|w|l|q]', but return the offset from `HAYSTACK', rather than the actual address.
 *   [KOS] mempat[b|w|l|q]     - Same as `memset', but repeat a multi-byte pattern on aligned addresses.
 * String functions:
 *   [STD] strlen              - Return the length of the string in characters (Same as `rawmemlen[...](STR,'\0')´)
 *   [STD] strnlen             - Same as `strlen', but don't exceed `MAX_CHARS' characters (Same as `memlen[...](STR,'\0',MAX_CHARS)´)
 *   [KOS] strend              - Same as `STR+strlen(STR)'
 *   [KOS] strnend             - Same as `STR+strnlen(STR,MAX_CHARS)'
 *   [STD] strchr              - Return the pointer of the first instance of `NEEDLE', or `NULL' if `NEEDLE' wasn't found.
 *   [STD] strrchr             - Return the pointer of the last instance of `NEEDLE', or `NULL' if `NEEDLE' wasn't found.
 *   [KOS] strnchr             - Same as `strchr', but don't exceed `MAX_CHARS' characters.
 *   [KOS] strnrchr            - Same as `strrchr', but don't exceed `MAX_CHARS' characters.
 *   [GLC] strchrnul           - Same as `strchr', but return `strend(STR)', rather than `NULL' if `NEEDLE' wasn't found.
 *   [KOS] strrchrnul          - Same as `strrchr', but return `STR-1', rather than `NULL' if `NEEDLE' wasn't found.
 *   [KOS] strnchrnul          - Same as `strnchr', but return `strnend(STR,MAX_CHARS)', rather than `NULL' if `NEEDLE' wasn't found.
 *   [KOS] strnrchrnul         - Same as `strnrchr', but return `STR-1', rather than `NULL' if `NEEDLE' wasn't found.
 *   [KOS] stroff              - Same as `strchrnul', but return the offset from `STR', rather than the actual address.
 *   [KOS] strroff             - Same as `strrchrnul', but return the offset from `STR', rather than the actual address.
 *   [KOS] strnoff             - Same as `strnchrnul', but return the offset from `STR', rather than the actual address.
 *   [KOS] strnroff            - Same as `strnrchrnul', but return the offset from `STR', rather than the actual address.
 *   [STD] strcpy              - Same as `memcpy(DST,SRC,(strlen(SRC)+1)*sizeof(char))´
 *   [STD] strcat              - Same as `memcpy(strend(DST),SRC,(strlen(SRC)+1)*sizeof(char))'
 *   [STD] strncpy             - Similar to `strcpy', but always write `DSTSIZE' characters, copying from `SRC' and filling the rest with padding ZEROes.
 *   [STD] strncat             - Same as Copy `strnlen(SRC,MAX_CHARS)' characters to `strned(DST)', then append a NUL-character thereafter. - Return `DST'.
 *   [GLC] stpcpy              - Same as `mempcpy(DST,SRC,(strlen(SRC)+1)*sizeof(char))-1´
 *   [GLC] stpncpy             - Same as `strncpy(DST,SRC,DSTSIZE)+strnlen(SRC,DSTSIZE)' (Returns a pointer to the end of `DST', or to the first NUL-character)
 */

__SYSDECL_BEGIN

#ifndef __std_size_t_defined
#define __std_size_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __SIZE_TYPE__ size_t;
__NAMESPACE_STD_END
#endif /* !__std_size_t_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __size_t_defined
#define __size_t_defined 1
__NAMESPACE_STD_USING(size_t)
#endif /* !__size_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */

#ifndef NULL
#define NULL __NULLPTR
#endif


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


__NAMESPACE_STD_BEGIN
#ifndef __std_memcpy_defined
#define __std_memcpy_defined 1
#ifndef __OPTIMIZE_LIBC__
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes);
#else
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes) { return __local_memcpy(__dst,__src,__n_bytes); }
#endif
#endif /* !__std_memcpy_defined */
#ifndef __std_memmove_defined
#define __std_memmove_defined 1
#ifndef __OPTIMIZE_LIBC__
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memmove)(void *__dst, void const *__src, size_t __n_bytes);
#else
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL memmove)(void *__dst, void const *__src, size_t __n_bytes) { return __local_memmove(__dst,__src,__n_bytes); }
#endif
#endif /* !__std_memmove_defined */
#ifndef __std_strlen_defined
#define __std_strlen_defined 1
#if defined(_MSC_VER) || (defined(__NO_opt_strlen) || !defined(__OPTIMIZE_LIBC__))
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strlen)(char const *__restrict __s);
#ifdef _MSC_VER
#pragma intrinsic(strlen)
#endif
#else /* Builtin... */
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strlen)(char const *__restrict __s) { return __opt_strlen(__s); }
#ifdef __opt_strlen_needs_macro
#ifdef __cplusplus
/* Fix `strlen()' being used as `std::strlen()' */
__FORCELOCAL __WUNUSED __ATTR_PURE size_t (__LIBCCALL __return_size_t)(size_t __x) { return __x; }
#define strlen(s)  __return_size_t(__opt_strlen(s))
#else
#define strlen(s)  __opt_strlen(s)
#endif
#endif /* __opt_strlen_needs_macro */
#endif /* Optimized... */
#endif /* !__std_strlen_defined */
#if defined(__OPTIMIZE_LIBC__) && !defined(_MSC_VER)
__OPT_LOCAL __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memset)(void *__dst, int __byte, size_t __n_bytes) { return __local_memset(__dst,__byte,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmp)(void const *__a, void const *__b, size_t __n_bytes) { return __local_memcmp(__a,__b,__n_bytes); }
#else /* __OPTIMIZE_LIBC__ */
__LIBC __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL memset)(void *__dst, int __byte, size_t __n_bytes);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL memcmp)(void const *__a, void const *__b, size_t __n_bytes);
#ifdef _MSC_VER
#pragma intrinsic(memset)
#pragma intrinsic(memcmp)
#endif
#endif /* !__OPTIMIZE_LIBC__ */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strcmp)(char const *__s1, char const *__s2);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strncmp)(char const *__s1, char const *__s2, size_t __n);
#ifdef _MSC_VER
#pragma intrinsic(strcmp)
#endif
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) char *(__LIBCCALL strstr)(char const *__haystack, char const *__needle);
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
#ifndef __memcpy_defined
#define __memcpy_defined 1
__NAMESPACE_STD_USING(memcpy)
#endif /* !__memcpy_defined */
#ifndef __memmove_defined
#define __memmove_defined 1
__NAMESPACE_STD_USING(memmove)
#endif /* !__memmove_defined */
__NAMESPACE_STD_USING(strlen)
#ifdef __opt_strlen_needs_macro
#ifdef __cplusplus
__NAMESPACE_STD_USING(__return_size_t)
#endif
#endif /* __opt_strlen_needs_macro */
__NAMESPACE_STD_USING(memset)
__NAMESPACE_STD_USING(memcmp)
__NAMESPACE_STD_USING(strcmp)
__NAMESPACE_STD_USING(strncmp)
__NAMESPACE_STD_USING(strstr)
#endif /* !__CXX_SYSTEM_HEADER */

#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL strcpy)(char *__restrict __dst, char const *__restrict __src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL strncpy)(char *__restrict __dst, char const *__restrict __src, size_t __dstsize);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL strcat)(char *__restrict __dst, char const *__restrict __src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL strncat)(char *__restrict __dst, char const *__restrict __src, size_t __max_chars);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strcoll)(char const *__s1, char const *__s2);
__LIBC __NONNULL((2)) size_t (__LIBCCALL strxfrm)(char *__dst, char const *__restrict __src, size_t __n);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL strcspn)(char const *__s, char const *__reject);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) size_t (__LIBCCALL strspn)(char const *__s, char const *__accept);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) char *(__LIBCCALL strpbrk)(char const *__s, char const *__accept);
__LIBC __NONNULL((2)) char *(__LIBCCALL strtok)(char *__restrict __s, char const *__restrict __delim);
#ifdef _MSC_VER
#pragma intrinsic(strcpy)
#pragma intrinsic(strcat)
#endif
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(strcpy)
__NAMESPACE_STD_USING(strncpy)
__NAMESPACE_STD_USING(strcat)
__NAMESPACE_STD_USING(strncat)
__NAMESPACE_STD_USING(strcoll)
__NAMESPACE_STD_USING(strxfrm)
__NAMESPACE_STD_USING(strcspn)
__NAMESPACE_STD_USING(strspn)
__NAMESPACE_STD_USING(strpbrk)
__NAMESPACE_STD_USING(strtok)
#endif /* !__CXX_SYSTEM_HEADER */

#if defined(__DOS_COMPAT__) || defined(__OPTIMIZE_LIBC__) || defined(__KERNEL__)
__OPT_LOCAL __NONNULL((1)) void (__LIBCCALL __bzero)(void *__s, size_t __n) { __local_memset(__s,0,__n); }
#else /* __DOS_COMPAT__ */
__REDIRECT_VOID(__LIBC,__NONNULL((1)),__LIBCCALL,__bzero,(void *__s, size_t __n),bzero,(__s,__n))
#endif /* !__DOS_COMPAT__ */
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__NONNULL((1,2,3)),char *,__LIBCCALL,__strtok_r,(char *__restrict __s, char const *__restrict __delim, char **__restrict __save_ptr),strtok_s,(__s,__delim,__save_ptr))
#else
__REDIRECT(__LIBC,__NONNULL((1,2,3)),char *,__LIBCCALL,__strtok_r,(char *__restrict __s, char const *__restrict __delim, char **__restrict __save_ptr),strtok_r,(__s,__delim,__save_ptr))
#endif
#ifdef __USE_POSIX
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__NONNULL((1,2,3)),char *,__LIBCCALL,strtok_r,(char *__restrict __s, char const *__restrict __delim, char **__restrict __save_ptr),strtok_s,(__s,__delim,__save_ptr))
#else
__LIBC __NONNULL((1,2,3)) char *(__LIBCCALL strtok_r)(char *__restrict __s, char const *__restrict __delim, char **__restrict __save_ptr);
#endif
#endif /* __USE_POSIX */
#ifdef __USE_XOPEN2K
#ifndef __USE_GNU
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,strerror_r,(int __errnum, char *__buf, size_t __buflen),__xpg_strerror_r,(__errnum,__buf,__buflen))
#else /* !__USE_GNU */
#ifdef __CRT_GLC
__LIBC __PORT_NODOS_ALT(strerror) __ATTR_RETNONNULL __NONNULL((2)) char *(__LIBCCALL strerror_r)(int __errnum, char *__buf, size_t __buflen);
#endif /* __CRT_GLC */
#endif /* __USE_GNU */
#endif /* __USE_XOPEN2K */
#endif /* !__KERNEL__ */

__NAMESPACE_STD_BEGIN
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
#ifndef __OPTIMIZE_LIBC__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),void *,__LIBCCALL,memchr,(void *__restrict __haystack, int __needle, size_t __n_bytes),memchr,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),void const *,__LIBCCALL,memchr,(void const *__restrict __haystack, int __needle, size_t __n_bytes),memchr,(__haystack,__needle,__n_bytes))
#else
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memchr)(void *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memchr(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void const *(__LIBCCALL memchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memchr(__haystack,__needle,__n_bytes); }
#endif
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,strchr,(char *__restrict __haystack, int __needle),strchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,strchr,(char const *__restrict __haystack, int __needle),strchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,strrchr,(char *__restrict __haystack, int __needle),strrchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,strrchr,(char const *__restrict __haystack, int __needle),strrchr,(__haystack,__needle))
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
#ifdef __OPTIMIZE_LIBC__
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
#else
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memchr(__haystack,__needle,__n_bytes); }
#endif
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strchr)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL strrchr)(char const *__restrict __haystack, int __needle);
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(memchr)
__NAMESPACE_STD_USING(strchr)
__NAMESPACE_STD_USING(strrchr)
#endif /* !__CXX_SYSTEM_HEADER */


#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED __ATTR_RETNONNULL char *(__LIBCCALL strerror)(int __errnum);
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(strerror)
#endif /* !__CXX_SYSTEM_HEADER */
#endif /* !__KERNEL__ */

#ifdef __USE_XOPEN2K8
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) size_t (__LIBCCALL strnlen)(char const *__str, size_t __max_chars);
#ifndef __DOS_COMPAT__
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),char *,__LIBCCALL,__stpcpy,(char *__restrict __dst, char const *__restrict __src),stpcpy,(__dst,__src))
__REDIRECT(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),char *,__LIBCCALL,__stpncpy,(char *__restrict __dst, char const *__restrict __src, size_t __dstsize),stpncpy,(__dst,__src,__dstsize))
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL stpcpy)(char *__restrict __dst, char const *__restrict __src);
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL stpncpy)(char *__restrict __dst, char const *__restrict __src, size_t __dstsize);
#else /* !__DOS_COMPAT__ */
__LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL stpcpy)(char *__restrict __dst, char const *__restrict __src) { return __NAMESPACE_STD_SYM strcpy(__dst,__src)+__NAMESPACE_STD_SYM strlen(__dst); }
__LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL stpncpy)(char *__restrict __dst, char const *__restrict __src, size_t __dstsize) { return __NAMESPACE_STD_SYM strncpy(__dst,__src,__dstsize)+__NAMESPACE_STD_SYM strlen(__dst); }
__LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL __stpcpy)(char *__restrict __dst, char const *__restrict __src) { return stpcpy(__dst,__src); }
__LOCAL __ATTR_RETNONNULL __NONNULL((1,2)) char *(__LIBCCALL __stpncpy)(char *__restrict __dst, char const *__restrict __src, size_t __dstsize) { return stpncpy(__dst,__src,__dstsize); }
#endif /* __DOS_COMPAT__ */
#ifndef __KERNEL__
__REDIRECT_DPA(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strcoll_l,(char const *__s1, char const *__s2, __locale_t __locale),(__s1,__s2,__locale))
__REDIRECT_DPA(__LIBC,__NONNULL((2)),size_t,__LIBCCALL,strxfrm_l,(char *__dst, char const *__restrict __src, size_t __n, __locale_t __locale),(__dst,__src,__n,__locale))
#ifndef __DOS_COMPAT__
__LIBC __WUNUSED __NONNULL((2)) char *(__LIBCCALL strerror_l)(int __errnum, __locale_t __locale);
__LIBC __PORT_NODOS __WUNUSED __ATTR_RETNONNULL char *(__LIBCCALL strsignal)(int __signo);
__LIBC __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL strndup)(char const *__restrict __str, size_t __max_chars);
#else /* !__DOS_COMPAT__ */
__SYSDECL_END
#include "hybrid/malloc.h"
__SYSDECL_BEGIN
__LOCAL __WUNUSED __NONNULL((2)) char *(__LIBCCALL strerror_l)(int __errnum, __locale_t __UNUSED(__locale)) { return strerror(__errnum); }
__LOCAL __WUNUSED __MALL_DEFAULT_ALIGNED
__ATTR_MALLOC char *(__LIBCCALL strndup)(char const *__restrict __str, size_t __max_chars) {
    size_t __resultlen = strnlen(__str,__max_chars);
    char *__result = (char *)__hybrid_malloc((__resultlen+1)*sizeof(char));
    if (__result) {
        __NAMESPACE_STD_SYM memcpy(__result,__str,__resultlen*sizeof(char));
        __result[__resultlen] = '\0';
    }
    return __result;
}
#endif /* __DOS_COMPAT__ */
#endif /* !__KERNEL__ */
#endif /* __USE_XOPEN2K8 */

#ifndef __KERNEL__
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8) || defined(__USE_DOS)
__REDIRECT_DPA(__LIBC,__WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC __NONNULL((1)),
               char *,__LIBCCALL,strdup,(char const *__restrict __str),(__str))
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 || __USE_DOS */
#endif /* !__KERNEL__ */

#ifdef __USE_GNU
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" {
#if !defined(__DOS_COMPAT__) && !defined(__OPTIMIZE_LIBC__)
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),void *,__LIBCCALL,memrchr,(void *__restrict __haystack, int __needle, size_t __n_bytes),memrchr,(__haystack,__needle,__n_bytes))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),void const *,__LIBCCALL,memrchr,(void const *__restrict __haystack, int __needle, size_t __n_bytes),memrchr,(__haystack,__needle,__n_bytes))
#else
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memrchr)(void *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrchr(__haystack,__needle,__n_bytes); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void const *(__LIBCCALL memrchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrchr(__haystack,__needle,__n_bytes); }
#endif
#ifndef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void *,__LIBCCALL,rawmemchr,(void *__restrict __haystack, int __needle),rawmemchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),void const *,__LIBCCALL,rawmemchr,(void const *__restrict __haystack, int __needle),rawmemchr,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char *,__LIBCCALL,strchrnul,(char *__restrict __haystack, int __needle),strchrnul,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)),char const *,__LIBCCALL,strchrnul,(char const *__restrict __haystack, int __needle),strchrnul,(__haystack,__needle))
#else /* !__DOS_COMPAT__ */
__OPT_LOCAL __WUNUSED __ATTR_RETNONNULL  __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL rawmemchr)(void *__restrict __haystack, int __needle) { return __local_rawmemchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_RETNONNULL  __ATTR_PURE __NONNULL((1)) void const *(__LIBCCALL rawmemchr)(void const *__restrict __haystack, int __needle) { return __local_rawmemchr(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strchrnul)(char *__restrict __haystack, int __needle) { return __libc_strchrnul(__haystack,__needle); }
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char const *(__LIBCCALL strchrnul)(char const *__restrict __haystack, int __needle) { return __libc_strchrnul(__haystack,__needle); }
#endif /* __DOS_COMPAT__ */
#if !defined(basename) && defined(__CRT_GLC)
__REDIRECT(__LIBC,__PORT_NODOS __WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,basename,(char *__restrict __filename),basename,(__filename))
__REDIRECT(__LIBC,__PORT_NODOS __WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,basename,(char const *__restrict __filename),basename,(__filename))
#endif /* !basename && __CRT_GLC */
}
#else /* __CORRECT_ISO_CPP_STRING_H_PROTO */
#if !defined(__DOS_COMPAT__) && !defined(__OPTIMIZE_LIBC__)
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memrchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes);
#else
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1)) void *(__LIBCCALL memrchr)(void const *__restrict __haystack, int __needle, size_t __n_bytes) { return __local_memrchr(__haystack,__needle,__n_bytes); }
#endif
#if !defined(__DOS_COMPAT__) && !defined(__OPTIMIZE_LIBC__)
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL rawmemchr)(void const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strchrnul)(char const *__restrict __haystack, int __needle);
#else /* !__DOS_COMPAT__ */
__OPT_LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) void *(__LIBCCALL rawmemchr)(void const *__restrict __haystack, int __needle) { return __local_rawmemchr(__haystack,__needle); }
__LOCAL __WUNUSED __ATTR_PURE __ATTR_RETNONNULL __NONNULL((1)) char *(__LIBCCALL strchrnul)(char const *__restrict __haystack, int __needle) { return __libc_strchrnul(__haystack,__needle); }
#endif /* __DOS_COMPAT__ */
#if !defined(basename) && defined(__CRT_GLC)
__LIBC __PORT_NODOS __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL basename)(char const *__restrict __filename);
#endif /* !basename && __CRT_GLC */
#endif /* !__CORRECT_ISO_CPP_STRING_H_PROTO */

#ifdef __DOS_COMPAT__
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2))
char *(__LIBCCALL strcasestr)(char const *__haystack, char const *__needle) {
    for (; *__haystack; ++__haystack) {
        if (__libc_strcasecmp(__haystack,__needle) == 0)
            return (char *)__haystack;
    }
    return __NULLPTR;
}
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,3))
void *(__LIBCCALL memmem)(void const *__haystack, size_t __haystacklen,
                          void const *__needle, size_t __needlelen) {
    __BYTE_TYPE__ *__iter = (__BYTE_TYPE__ *)__haystack;
    while (__haystacklen >= __needlelen) {
        if (__NAMESPACE_STD_SYM memcmp(__iter,__needle,__needlelen) == 0)
            return (void *)__iter;
        ++__iter;
    }
    return __NULLPTR;
}
__LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strverscmp)(char const *__s1, char const *__s2) { return __NAMESPACE_STD_SYM strcmp(__s1,__s2); /* TODO. */ }
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) char *(__LIBCCALL strcasestr)(char const *__haystack, char const *__needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,3)) void *(__LIBCCALL memmem)(void const *__haystack, size_t __haystacklen, void const *__needle, size_t __needlelen);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strverscmp)(char const *__s1, char const *__s2);
#endif /* !__DOS_COMPAT__ */


#if defined(__OPTIMIZE_LIBC__) || defined(__DOS_COMPAT__)
__OPT_LOCAL __NONNULL((1,2)) void *(__LIBCCALL __mempcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n) { return __local_mempcpy(__dst,__src,__n); }
__OPT_LOCAL __NONNULL((1,2)) void *(__LIBCCALL mempcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n) { return __local_mempcpy(__dst,__src,__n); }
#else /* __DOS_COMPAT__ */
__REDIRECT(__LIBC,__NONNULL((1,2)),void *,__LIBCCALL,__mempcpy,(void *__restrict __dst, void const *__restrict __src, size_t __n),mempcpy,(__dst,__src,__n))
__LIBC __NONNULL((1,2)) void *(__LIBCCALL mempcpy)(void *__restrict __dst, void const *__restrict __src, size_t __n);
#endif /* !__DOS_COMPAT__ */

#ifndef __KERNEL__
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __NONNULL((1)) char *(__LIBCCALL strfry)(char *__str);
__LIBC __PORT_NODOS __NONNULL((1)) void *(__LIBCCALL memfrob)(void *__s, size_t __n);
#endif /* __CRT_GLC */
#ifndef __strcasecmp_l_defined
#define __strcasecmp_l_defined 1
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strcasecmp_l,(char const *__s1, char const *__s2, __locale_t __locale),_stricmp_l,(__s1,__s2,__locale))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strncasecmp_l,(char const *__s1, char const *__s2, size_t __n, __locale_t __locale),_strnicmp_l,(__s1,__s2,__n,__locale))
#else
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strcasecmp_l)(char const *__s1, char const *__s2, __locale_t __locale);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strncasecmp_l)(char const *__s1, char const *__s2, size_t __n, __locale_t __locale);
#endif
#endif /* !__strcasecmp_l_defined */
#endif /* !__KERNEL__ */
#endif /* __USE_GNU */

#ifdef __USE_MISC
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __NONNULL((1,2)) char *(__LIBCCALL strsep)(char **__restrict __stringp, char const *__restrict __delim);
#endif /* __CRT_GLC */
#ifndef __bstring_defined
#define __bstring_defined 1
#if defined(__DOS_COMPAT__) || defined(__KERNEL__)
__OPT_LOCAL __NONNULL((1,2)) void (__LIBCCALL bcopy)(void const *__src, void *__dst, size_t __n) { __local_memmove(__dst,__src,__n); }
__OPT_LOCAL __NONNULL((1)) void (__LIBCCALL bzero)(void *__s, size_t __n) { __local_memset(__s,0,__n); }
#else /* __DOS_COMPAT__ */
__LIBC __NONNULL((1,2)) void (__LIBCCALL bcopy)(void const *__src, void *__dst, size_t __n);
__LIBC __NONNULL((1)) void (__LIBCCALL bzero)(void *__s, size_t __n);
#endif /* !__DOS_COMPAT__ */
#ifndef __OPTIMIZE_LIBC__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,bcmp,
          (void const *__s1, void const *__s2, size_t __n),memcmp,(__s1,__s2,__n))
#else
__OPT_LOCAL __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL bcmp)(void const *__s1, void const *__s2, size_t __n) { return __local_memcmp(__s1,__s2,__n); }
#endif
#endif /* !__bstring_defined */
#ifndef __KERNEL__
#ifndef __index_defined
#define __index_defined 1
#ifdef __CORRECT_ISO_CPP_STRINGS_H_PROTO
extern "C++" {
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,index,(char *__restrict __haystack, int __needle),index,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,index,(char const *__restrict __haystack, int __needle),index,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char *,__LIBCCALL,rindex,(char *__restrict __haystack, int __needle),rindex,(__haystack,__needle))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),char const *,__LIBCCALL,rindex,(char const *__restrict __haystack, int __needle),rindex,(__haystack,__needle))
}
#else /* __CORRECT_ISO_CPP_STRINGS_H_PROTO */
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL index)(char const *__restrict __haystack, int __needle);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1)) char *(__LIBCCALL rindex)(char const *__restrict __haystack, int __needle);
#endif /* !__CORRECT_ISO_CPP_STRINGS_H_PROTO */
#endif /* !__index_defined */
#endif /* !__KERNEL__ */

#define ____IMPL_DO_FFS(T,i) \
{ unsigned int __result; \
  if (!i) return 0; \
  for (__result = 1; !(((T)i)&1); ++__result) \
       i = ((T)i >> 1); \
  return __result; \
}

#ifdef __USE_GNU
#if __has_builtin(__builtin_ffsl) && __has_builtin(__builtin_ffsll)
#ifdef __USE_KOS
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL ffsl)(long __i) { return (unsigned int)__builtin_ffsl(__i); }
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL ffsll)(__LONGLONG __i) { return (unsigned int)__builtin_ffsll(__i); }
#define ffsl(__i)  (unsigned int)__builtin_ffsl(__i)
#define ffsll(__i) (unsigned int)__builtin_ffsll(__i)
#else
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL ffsl)(long __i) { return __builtin_ffsl(__i); }
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL ffsll)(__LONGLONG __i) { return __builtin_ffsll(__i); }
#define ffsl(__i)  __builtin_ffsl(__i)
#define ffsll(__i) __builtin_ffsll(__i)
#endif
#elif defined(__ANY_COMPAT__) && !defined(__GLC_COMPAT__)
#ifdef __USE_KOS
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL ffsl)(long __i) ____IMPL_DO_FFS(unsigned long,__i)
__LOCAL __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL ffsll)(__LONGLONG __i) ____IMPL_DO_FFS(__ULONGLONG,__i)
#else
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL ffsl)(long __i) ____IMPL_DO_FFS(unsigned long,__i)
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL ffsll)(__LONGLONG __i) ____IMPL_DO_FFS(__ULONGLONG,__i)
#endif
#else /* Compat... */
#ifdef __USE_KOS
__LIBC __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL ffsl)(long __i);
__LIBC __WUNUSED __ATTR_CONST unsigned int (__LIBCCALL ffsll)(__LONGLONG __i);
#else
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL ffsl)(long __i);
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL ffsll)(__LONGLONG __i);
#endif
#endif /* Native... */
#endif /* __USE_GNU */

#ifndef __ffs_defined
#define __ffs_defined 1
#ifdef __USE_KOS
#include <hybrid/bit.h>
/* unsigned int FFS(INTEGER i):
 *     FindFirstSet
 *     Returns the index (starting at 1 for 0x01) of the first
 *     1-bit in given value, or ZERO(0) if the given value is ZERO(0).
 *     >> assert(!x ||  (x &  (1 << (ffs(x)-1))));    // FFS-bit is set
 *     >> assert(!x || !(x & ((1 << (ffs(x)-1))-1))); // Less significant bits are clear */
#define ffs(i) __hybrid_ffs(i)
#elif __has_builtin(__builtin_ffs)
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL ffs)(int __i) { return __builtin_ffs(__i); }
#define ffs(i) __builtin_ffs(i)
#elif defined(__ANY_COMPAT__) && !defined(__GLC_COMPAT__)
__LOCAL __WUNUSED __ATTR_CONST int (__LIBCCALL ffs)(int __i) ____IMPL_DO_FFS(unsigned int,__i)
#else
__LIBC __WUNUSED __ATTR_CONST int (__LIBCCALL ffs)(int __i);
#endif /* !__USE_KOS */
#endif /* !__ffs_defined */
#undef ____IMPL_DO_FFS

#ifndef __strcasecmp_defined
#define __strcasecmp_defined 1
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strcasecmp,(char const *__s1, char const *__s2),_stricmp,(__s1,__s2))
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,strncasecmp,(char const *__s1, char const *__s2, size_t __n),_strnicmp,(__s1,__s2,__n))
#else
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strcasecmp)(char const *__s1, char const *__s2);
__LIBC __WUNUSED __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL strncasecmp)(char const *__s1, char const *__s2, size_t __n);
#endif
#endif /* !__strcasecmp_defined */
#endif /* __USE_MISC */

#ifndef __KERNEL__
#if defined(__USE_MISC) || defined(__USE_XOPEN)
__REDIRECT_DPA(__LIBC,__ATTR_RETNONNULL __NONNULL((1,2)),void *,__LIBCCALL,memccpy,(void *__restrict __dst, void const *__restrict __src, int __c, size_t __n),(__dst,__src,__c,__n))
#endif /* __USE_MISC || __USE_XOPEN */
#endif /* !__KERNEL__ */

#ifdef __USE_DEBUG
#include "hybrid/debuginfo.h"
#if __USE_DEBUG != 0 && defined(__CRT_KOS) && \
  (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _memcpy_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes, __DEBUGINFO);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(_memcpy_d)
#ifdef __USE_GNU
__LIBC __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL _mempcpy_d)(void *__restrict __dst, void const *__restrict __src, size_t __n_bytes, __DEBUGINFO);
#endif /* __USE_GNU */
#ifndef __KERNEL__
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
__LIBC __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL _strdup_d)(char const *__restrict __str, __DEBUGINFO);
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_XOPEN2K8
__LIBC __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL _strndup_d)(char const *__restrict __str, size_t __max_chars, __DEBUGINFO);
#endif /* __USE_XOPEN2K8 */
#ifdef __USE_KOS
__LIBC __ATTR_LIBC_PRINTF(4,5) __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__ATTR_CDECL _strdupf_d)(__DEBUGINFO, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(1,0) __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC char *(__LIBCCALL _vstrdupf_d)(char const *__restrict __format, __builtin_va_list __args, __DEBUGINFO);
#endif /* __USE_KOS */
#endif /* !__KERNEL__ */
#else /* __USE_DEBUG != 0 */
#   define _memcpy_d(dst,src,n_bytes,...) memcpy(dst,src,n_bytes)
#ifdef __USE_GNU
#   define _mempcpy_d(dst,src,n_bytes,...) mempcpy(dst,src,n_bytes)
#endif /* __USE_GNU */
#ifndef __KERNEL__
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#   define _strdup_d(str,...)             strdup(str)
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_XOPEN2K8
#   define _strndup_d(str,max_chars,...)  strndup(str,max_chars)
#endif /* __USE_XOPEN2K8 */
#ifdef __USE_KOS
#   define _strdupf_d(file,line,func,...)   strdupf(__VA_ARGS__)
#   define _vstrdupf_d(format,args,...)     vstrdupf(format,args)
#endif /* __USE_KOS */
#endif /* !__KERNEL__ */
#endif /* __USE_DEBUG == 0 */
#ifdef __USE_DEBUG_HOOK
#ifndef __OPTIMIZE_LIBC__
#   define memcpy(dst,src,n_bytes)    _memcpy_d(dst,src,n_bytes,__DEBUGINFO_GEN)
#ifdef __USE_GNU
#   define mempcpy(dst,src,n_bytes)   _mempcpy_d(dst,src,n_bytes,__DEBUGINFO_GEN)
#endif /* __USE_GNU */
#endif /* !__OPTIMIZE_LIBC__ */
#ifndef __KERNEL__
#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#   define strdup(str)                _strdup_d(str,__DEBUGINFO_GEN)
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */
#ifdef __USE_XOPEN2K8
#   define strndup(str,max_chars)     _strndup_d(str,max_chars,__DEBUGINFO_GEN)
#endif /* __USE_XOPEN2K8 */
#ifdef __USE_KOS
#ifdef _strdupf_d
#   define strdupf(...)               _strdupf_d(__FILE__,__LINE__,__FUNCTION__,__VA_ARGS__)
#else /* _strdupf_d */
#   define strdupf(...)               _strdupf_d(__DEBUGINFO_GEN,__VA_ARGS__)
#endif /* !_strdupf_d */
#   define vstrdupf(format,args)      _vstrdupf_d(format,args,__DEBUGINFO_GEN)
#endif /* __USE_KOS */
#endif /* !__KERNEL__ */
#endif /* __USE_DEBUG_HOOK */
#endif /* __USE_DEBUG */

#ifdef __USE_MISC
#if (defined(__CRT_KOS) && __KOS_VERSION__ >= 300) && \
    !defined(__ANY_COMPAT__)
__LIBC size_t (__LIBCCALL strlcat)(char *__restrict __dst, char const *__restrict __src, size_t __dst_size);
__LIBC size_t (__LIBCCALL strlcpy)(char *__restrict __dst, char const *__restrict __src, size_t __dst_size);
#else
#ifdef __USE_XOPEN2K8
#define __impl_strnlen(str,max_chars) strnlen(str,max_chars)
#else
__REDIRECT(__LIBC,__WUNUSED __ATTR_PURE __NONNULL((1)),size_t,__LIBCCALL,__impl_strnlen,(char const *__str, size_t __max_chars),strnlen,(__str,__max_chars))
#endif
__LOCAL size_t (__LIBCCALL strlcat)(char *__restrict __dst, char const *__restrict __src, size_t __dst_size) {
 size_t __result = __NAMESPACE_STD_SYM strlen(__src);
 char *__new_dst = __dst + __impl_strnlen(__dst,__dst_size);
 size_t __copy_size = (__dst_size -= (__new_dst-__dst),
                       __result < __dst_size ? __result : __dst_size-1);
 __NAMESPACE_STD_SYM memcpy(__new_dst,__src,__copy_size*sizeof(char));
 __new_dst[__copy_size] = '\0';
 return __result + (__new_dst-__dst);
}
__LOCAL size_t (__LIBCCALL strlcpy)(char *__restrict __dst, char const *__restrict __src, size_t __dst_size) {
 size_t __result = __NAMESPACE_STD_SYM strlen(__src);
 size_t __copy_size = __result < __dst_size ? __result : __dst_size-1;
 __NAMESPACE_STD_SYM memcpy(__dst,__src,__copy_size*sizeof(char));
 __dst[__copy_size] = '\0';
 return __result;
}
#undef __impl_strnlen
#endif
#endif


__SYSDECL_END

#ifdef __USE_GNU
#include "hybrid/alloca.h"
#include "libc/string.h"
#ifndef __NO_XBLOCK
#define strdupa(s) \
 __XBLOCK({ \
   char const *const __old = (s); \
   size_t const __len = __libc_strlen(__old)+1; \
   char *const __new = (char *)__ALLOCA(__len); \
   __XRETURN (char *)__NAMESPACE_STD_SYM memcpy(__new,__old,__len); \
 })
#define strndupa(s,n) \
 __XBLOCK({ \
   char const *const __old = (s); \
   size_t const __len = __hybrid_strnlen(__old,(n)); \
   char *const __new = (char *)__ALLOCA(__len+1); \
   __new[__len] = '\0'; \
   __XRETURN (char *)__NAMESPACE_STD_SYM memcpy(__new,__old,__len); \
 })
#else /* !__NO_XBLOCK */
__NAMESPACE_INT_BEGIN
__LOCAL char *(__LIBCCALL __strndupa_init)(void *__restrict __buf, char const *__restrict __src, size_t __n) {
    __n = __libc_strnlen(__src,__n);
    __NAMESPACE_STD_SYM memcpy(__buf,__src,__n*sizeof(char));
    ((char *)__buf)[__n] = '\0';
    return (char *)__buf;
}
__NAMESPACE_INT_END
#define strdupa(s) \
  (__NAMESPACE_STD_SYM strcpy((char *)__ALLOCA((__libc_strlen(s)+1)*sizeof(char)),(s)))
#define strndupa(s,n) \
  (__NAMESPACE_INT_SYM __strndupa_init(__ALLOCA((__libc_strnlen((s),(n))+1)*sizeof(char)),(s),(n)))
#endif /* __NO_XBLOCK */
#ifdef __USE_KOS
#include "parts/kos2/malloca.h"
#ifndef __NO_XBLOCK
#define mstrdupa(s)	\
 __XBLOCK({ \
   char const *const __old = (s); \
   size_t const __len = __libc_strlen(__old)+1; \
   char *const __new = (char *)__malloca(__len); \
   __XRETURN __new ? (char *)__NAMESPACE_STD_SYM memcpy(__new,__old,__len) : (char *)0; \
 })
#define mstrndupa(s,n) \
 __XBLOCK({ \
   char const *const __old = (s); \
   size_t const __len = __hybrid_strnlen(__old,(n)); \
   char *const __new = (char *)__malloca(__len+1); \
   __XRETURN __new ? (__new[__len] = '\0',(char *)__NAMESPACE_STD_SYM memcpy(__new,__old,__len)) : (char *)0; \
 })
#else /* !__NO_XBLOCK */
__NAMESPACE_INT_BEGIN
__LOCAL char *(__LIBCCALL __mstrdupa_init)(void *__buf, char const *__restrict __src) {
    if (__buf) __NAMESPACE_STD_SYM strcpy((char *)__buf,__src);
    return (char *)__buf;
}
__LOCAL char *(__LIBCCALL __mstrndupa_init)(void *__buf, char const *__restrict __src, size_t __n) {
    if (__buf) {
        __n = __libc_strnlen(__src,__n);
        __NAMESPACE_STD_SYM memcpy(__buf,__src,__n*sizeof(char));
        ((char *)__buf)[__n] = '\0';
    }
    return (char *)__buf;
}
__NAMESPACE_INT_END
#define mstrdupa(s) \
  (__NAMESPACE_INT_SYM __mstrdupa_init(__malloca((__libc_strlen(s)+1)*sizeof(char)),(s)))
#define mstrndupa(s,n) \
  (__NAMESPACE_INT_SYM __mstrndupa_init(__malloca((__libc_strnlen((s),(n))+1)*sizeof(char)),(s),(n)))
#endif /* __NO_XBLOCK */
#ifdef __USE_KOS_DEPRECATED
#define strdupma(s,n)  mstrdupa(s,n)
#define strndupma(s,n) mstrndupa(s,n)
#endif
#endif /* __USE_KOS */
#endif /* __USE_GNU */

#ifdef __USE_KOS
#ifndef _PARTS_KOS2_STRING_H
#include "parts/kos2/string.h"
#endif
#ifdef __USE_UTF
#ifdef _UCHAR_H
#ifndef _PARTS_KOS2_USTRING_H
#include "parts/kos2/ustring.h"
#endif
#endif
#endif /* __USE_UTF */
#endif /* __USE_KOS */

#ifdef __USE_KOS3
#ifdef __USE_UTF
#ifdef _UCHAR_H
#ifndef _PARTS_KOS3_USTRING_H
#include "parts/kos3/ustring.h"
#endif
#endif
#endif /* __USE_UTF */
#endif /* __USE_KOS3 */

#ifdef __USE_DOS
#ifndef _PARTS_DOS_STRING_H
#include "parts/dos/string.h"
#endif
#endif

#endif /* !_STRING_H */
