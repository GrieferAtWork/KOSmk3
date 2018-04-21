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
#ifndef _STDLIB_H
#define _STDLIB_H 1

#include "__stdinc.h"
#include "parts/kos2/malldefs.h"
#include <features.h>
#include <hybrid/typecore.h>

#ifdef __USE_MISC
#include <alloca.h>
#endif /* __USE_MISC */
#ifdef __USE_DOS
#include <xlocale.h>
#include <bits/byteswap.h>
#endif /* __USE_DOS */

#if defined(__USE_DEBUG) && __USE_DEBUG != 0 && \
  (!defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
#include <hybrid/debuginfo.h>
#endif

#ifdef __GNUC__
#pragma GCC system_header
#endif

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

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef NULL
#define NULL __NULLPTR
#endif

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K8)
#ifndef __WAIT_MACROS_DEFINED
#define __WAIT_MACROS_DEFINED 1
#include <bits/waitflags.h>
#include <bits/waitstatus.h>

#ifdef __USE_MISC
#if defined(__GNUC__) && !defined(__cplusplus)
#   define __WAIT_INT(status) (__extension__(((union{ __typeof__(status) __in; int __i; }) { .__in = (status) }).__i))
#else
#   define __WAIT_INT(status) (*(int *)&(status))
#endif
#ifdef __NO_ATTR_TRANSPARENT_UNION
#   define __WAIT_STATUS      void *
#   define __WAIT_STATUS_DEFN void *
#else
typedef union {
    union wait *__uptr;
    int        *__iptr;
} __WAIT_STATUS __ATTR_TRANSPARENT_UNION;
#   define __WAIT_STATUS_DEFN int *
#endif
#else /* __USE_MISC */
#   define __WAIT_INT(status)  (status)
#   define __WAIT_STATUS        int *
#   define __WAIT_STATUS_DEFN   int *
#endif /* !__USE_MISC */
#   define WEXITSTATUS(status)  __WEXITSTATUS(__WAIT_INT(status))
#   define WTERMSIG(status)     __WTERMSIG(__WAIT_INT(status))
#   define WSTOPSIG(status)     __WSTOPSIG(__WAIT_INT(status))
#   define WIFEXITED(status)    __WIFEXITED(__WAIT_INT(status))
#   define WIFSIGNALED(status)  __WIFSIGNALED(__WAIT_INT(status))
#   define WIFSTOPPED(status)   __WIFSTOPPED(__WAIT_INT(status))
#ifdef __WIFCONTINUED
#   define WIFCONTINUED(status) __WIFCONTINUED(__WAIT_INT(status))
#endif
#endif /* !__WAIT_MACROS_DEFINED */
#endif /* __USE_XOPEN || __USE_XOPEN2K8 */

__NAMESPACE_STD_BEGIN
#ifndef __std_div_t_defined
#define __std_div_t_defined 1
typedef struct { int quot,rem; } div_t;
#endif /* !__std_div_t_defined */
#ifndef __std_ldiv_t_defined
#define __std_ldiv_t_defined 1
typedef struct { long quot,rem; } ldiv_t;
#endif /* !__std_ldiv_t_defined */
#ifdef __USE_ISOC99
#ifndef __std_lldiv_t_defined
#define __std_lldiv_t_defined 1
typedef struct { __LONGLONG quot,rem; } lldiv_t;
#endif /* !__std_lldiv_t_defined */
#endif /* __USE_ISOC99 */
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
#ifndef __div_t_defined
#define __div_t_defined 1
__NAMESPACE_STD_USING(div_t)
#endif /* !__div_t_defined */
#ifndef __ldiv_t_defined
#define __ldiv_t_defined 1
__NAMESPACE_STD_USING(ldiv_t)
#endif /* !__ldiv_t_defined */
#ifdef __USE_ISOC99
#ifndef __lldiv_t_defined
#define __lldiv_t_defined 1
__NAMESPACE_STD_USING(lldiv_t)
#endif /* !__lldiv_t_defined */
#endif /* __USE_ISOC99 */
#endif /* !__CXX_SYSTEM_HEADER */


#ifdef __KERNEL__
#define RAND_MAX 0xffffffffu
#else
#define RAND_MAX 0x7fffffff
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1


#ifndef __COMPAR_FN_T
#define __COMPAR_FN_T
typedef int (__LIBCCALL *__compar_fn_t)(void const *__a, void const *__b);
#ifdef __USE_GNU
typedef __compar_fn_t comparison_fn_t;
#endif /* __USE_GNU */
#endif /* __COMPAR_FN_T */

#ifdef __USE_GNU
#ifndef __compar_d_fn_t_defined
#define __compar_d_fn_t_defined 1
typedef int (__LIBCCALL *__compar_d_fn_t)(void const *__a, void const *__b, void *__arg);
#endif /* !__compar_d_fn_t_defined */
__LIBC __NONNULL((1,4)) void (__LIBCCALL qsort_r)(void *__base, size_t __nmemb, size_t __size, __compar_d_fn_t __compar, void *__arg);
#endif /* __USE_GNU */

__NAMESPACE_STD_BEGIN
__LIBC __NONNULL((1,2,5)) __WUNUSED void *(__LIBCCALL bsearch)(void const *__key, void const *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar);
__LIBC __NONNULL((1,4)) void (__LIBCCALL qsort)(void *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar);

#ifdef __KERNEL__
#if defined(__CORRECT_ISO_CPP_STDLIB_H_PROTO) && 0
extern "C++" {
__LOCAL __ATTR_CONST __WUNUSED int __NOTHROW((__LIBCCALL abs)(int __x)) { return __x < 0 ? -__x : __x; }
__LOCAL __ATTR_CONST __WUNUSED long __NOTHROW((__LIBCCALL abs)(long __x)) { return __x < 0 ? -__x : __x; }
__LOCAL __ATTR_CONST __WUNUSED __LONGLONG __NOTHROW((__LIBCCALL abs)(__LONGLONG __x)) { return __x < 0 ? -__x : __x; }
__LOCAL __ATTR_CONST __WUNUSED div_t (__LIBCCALL div)(int __numer, int __denom) { div_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
__LOCAL __ATTR_CONST __WUNUSED ldiv_t (__LIBCCALL div)(long __numer, long __denom) { ldiv_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
__LOCAL __ATTR_CONST __WUNUSED lldiv_t (__LIBCCALL div)(__LONGLONG __numer, __LONGLONG __denom) { lldiv_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
}
#else /* __CORRECT_ISO_CPP_STDLIB_H_PROTO */
__LOCAL __ATTR_CONST __WUNUSED int __NOTHROW((__LIBCCALL abs)(int __x)) { return __x < 0 ? -__x : __x; }
__LOCAL __ATTR_CONST __WUNUSED div_t (__LIBCCALL div)(int __numer, int __denom) { div_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
#endif /* !__CORRECT_ISO_CPP_STDLIB_H_PROTO */
__LOCAL __ATTR_CONST __WUNUSED long __NOTHROW((__LIBCCALL labs)(long __x)) { return __x < 0 ? -__x : __x; }
__LOCAL __ATTR_CONST __WUNUSED ldiv_t (__LIBCCALL ldiv)(long __numer, long __denom) { ldiv_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
#ifdef __USE_ISOC99
__LOCAL __ATTR_CONST __WUNUSED __LONGLONG __NOTHROW((__LIBCCALL llabs)(__LONGLONG __x)) { return __x < 0 ? -__x : __x; }
__LOCAL __ATTR_CONST __WUNUSED lldiv_t (__LIBCCALL lldiv)(__LONGLONG __numer, __LONGLONG __denom) { lldiv_t __res; __res.quot = __numer/__denom; __res.rem  = __numer%__denom; return __res; }
#endif /* __USE_ISOC99 */
#else /* __KERNEL__ */
#if defined(__CORRECT_ISO_CPP_STDLIB_H_PROTO) && 0
extern "C++" {
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST __WUNUSED,int,__LIBCCALL,abs,(int __x),abs,(__x))
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST __WUNUSED,long,__LIBCCALL,abs,(long __x),labs,(__x))
__REDIRECT_NOTHROW(__LIBC,__ATTR_CONST __WUNUSED,__LONGLONG,__LIBCCALL,abs,(__LONGLONG __x),llabs,(__x))
__REDIRECT(__LIBC,__ATTR_CONST __WUNUSED,div_t,__LIBCCALL,div,(int __numer, int __denom),div,(__numer,__denom))
__REDIRECT(__LIBC,__ATTR_CONST __WUNUSED,ldiv_t,__LIBCCALL,div,(long __numer, long __denom),ldiv,(__numer,__denom))
__REDIRECT(__LIBC,__ATTR_CONST __WUNUSED,lldiv_t,__LIBCCALL,div,(__LONGLONG __numer, __LONGLONG __denom),lldiv,(__numer,__denom))
}
#else /* __CORRECT_ISO_CPP_STDLIB_H_PROTO */
__LIBC __ATTR_CONST __WUNUSED int __NOTHROW((__LIBCCALL abs)(int __x));
__LIBC __ATTR_CONST __WUNUSED div_t (__LIBCCALL div)(int __numer, int __denom);
#endif /* !__CORRECT_ISO_CPP_STDLIB_H_PROTO */
__LIBC __ATTR_CONST __WUNUSED long __NOTHROW((__LIBCCALL labs)(long __x));
__LIBC __ATTR_CONST __WUNUSED ldiv_t (__LIBCCALL ldiv)(long __numer, long __denom);
#ifdef __USE_ISOC99
__LIBC __ATTR_CONST __WUNUSED __LONGLONG __NOTHROW((__LIBCCALL llabs)(__LONGLONG __x));
__LIBC __ATTR_CONST __WUNUSED lldiv_t (__LIBCCALL lldiv)(__LONGLONG __numer, __LONGLONG __denom);
#endif /* __USE_ISOC99 */
#ifdef _MSC_VER
#pragma intrinsic(abs)
#pragma intrinsic(labs)
#endif
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(bsearch)
__NAMESPACE_STD_USING(qsort)
__NAMESPACE_STD_USING(abs)
__NAMESPACE_STD_USING(labs)
__NAMESPACE_STD_USING(div)
__NAMESPACE_STD_USING(ldiv)
#ifdef __USE_ISOC99
__NAMESPACE_STD_USING(llabs)
__NAMESPACE_STD_USING(lldiv)
#endif /* __USE_ISOC99 */
#endif /* !__CXX_SYSTEM_HEADER */

#if defined(__USE_MISC) || defined(__USE_DOS) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8))
__REDIRECT_DPA(__LIBC,__WUNUSED __NONNULL((3)),char *,__LIBCCALL,gcvt,(double __val, int __ndigit, char *__buf),(__val,__ndigit,__buf))
#endif
#ifdef __USE_MISC
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED __NONNULL((3)),char *,__LIBCCALL,qgcvt,(long double __val, int __ndigit, char *__buf),_gcvt,(__val,__ndigit,__buf))
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_ecvt_s,(char *__buf, size_t __buflen, double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),_ecvt_s,(__buf,__buflen,__val,__ndigit,__decptr,__sign))
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_fcvt_s,(char *__buf, size_t __buflen, double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),_fcvt_s,(__buf,__buflen,__val,__ndigit,__decptr,__sign))
__LOCAL __NONNULL((3,4,5)) int (__LIBCCALL ecvt_r)(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len) { return __dos_ecvt_s(__buf,__len,__val,__ndigit,__decptr,__sign); }
__LOCAL __NONNULL((3,4,5)) int (__LIBCCALL fcvt_r)(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len) { return __dos_fcvt_s(__buf,__len,__val,__ndigit,__decptr,__sign); }
__LOCAL __NONNULL((3,4,5)) int (__LIBCCALL qecvt_r)(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len) { return __dos_ecvt_s(__buf,__len,(double)__val,__ndigit,__decptr,__sign); }
__LOCAL __NONNULL((3,4,5)) int (__LIBCCALL qfcvt_r)(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len) { return __dos_fcvt_s(__buf,__len,(double)__val,__ndigit,__decptr,__sign); }
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED __NONNULL((3)) char *(__LIBCCALL qgcvt)(long double __val, int __ndigit, char *__buf);
__LIBC __NONNULL((3,4,5)) int (__LIBCCALL ecvt_r)(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len);
__LIBC __NONNULL((3,4,5)) int (__LIBCCALL fcvt_r)(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len);
__LIBC __NONNULL((3,4,5)) int (__LIBCCALL qecvt_r)(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len);
__LIBC __NONNULL((3,4,5)) int (__LIBCCALL qfcvt_r)(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign, char *__restrict __buf, size_t __len);
#endif /* !__DOS_COMPAT__ */
__REDIRECT(__LIBC,__NONNULL((1)),__LONGLONG,__LIBCCALL,strtoq,(char const *__restrict __nptr, char **__restrict __endptr, int __base),strtoll,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__ULONGLONG,__LIBCCALL,strtouq,(char const *__restrict __nptr, char **__restrict __endptr, int __base),strtoull,(__nptr,__endptr,__base))
#endif /* __USE_MISC */

__NAMESPACE_STD_BEGIN
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) double (__LIBCCALL atof)(char const *__restrict __nptr);
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) int (__LIBCCALL atoi)(char const *__restrict __nptr);
#if __SIZEOF_LONG__ == __SIZEOF_INT__
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __NONNULL((1)),long,__LIBCCALL,atol,(char const *__restrict __nptr),atoi,(__nptr))
#else /* __SIZEOF_LONG__ == __SIZEOF_INT__ */
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) long (__LIBCCALL atol)(char const *__restrict __nptr);
#endif /* __SIZEOF_LONG__ != __SIZEOF_INT__ */
__LIBC __NONNULL((1)) double (__LIBCCALL strtod)(char const *__restrict __nptr, char **__restrict __endptr);
__LIBC __NONNULL((1)) long (__LIBCCALL strtol)(char const *__restrict __nptr, char **__restrict __endptr, int __base);
__LIBC __NONNULL((1)) unsigned long (__LIBCCALL strtoul)(char const *__restrict __nptr, char **__restrict __endptr, int __base);
#ifdef __USE_ISOC99
__LIBC __NONNULL((1)) float (__LIBCCALL strtof)(char const *__restrict __nptr, char **__restrict __endptr);
__LIBC __NONNULL((1)) long double (__LIBCCALL strtold)(char const *__restrict __nptr, char **__restrict __endptr);
#if __SIZEOF_LONG_LONG__ == __SIZEOF_LONG__
__REDIRECT(__LIBC,__ATTR_PURE __WUNUSED __NONNULL((1)),__LONGLONG,__LIBCCALL,atoll,(char const *__restrict __nptr),atol,(__nptr))
__REDIRECT(__LIBC,__NONNULL((1)),__LONGLONG,__LIBCCALL,strtoll,(char const *__restrict __nptr, char **__restrict __endptr, int __base),strtol,(__nptr,__endptr,__base))
__REDIRECT(__LIBC,__NONNULL((1)),__ULONGLONG,__LIBCCALL,strtoull,(char const *__restrict __nptr, char **__restrict __endptr, int __base),strtoul,(__nptr,__endptr,__base))
#else /* __SIZEOF_LONG_LONG__ == __SIZEOF_LONG__ */
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) __LONGLONG (__LIBCCALL atoll)(char const *__restrict __nptr);
__LIBC __NONNULL((1)) __LONGLONG (__LIBCCALL strtoll)(char const *__restrict __nptr, char **__restrict __endptr, int __base);
__LIBC __NONNULL((1)) __ULONGLONG (__LIBCCALL strtoull)(char const *__restrict __nptr, char **__restrict __endptr, int __base);
#endif /* __SIZEOF_LONG_LONG__ != __SIZEOF_LONG__ */
#endif /* __USE_ISOC99 */
#ifdef __KERNEL__
__LIBC __UINT32_TYPE__ (__LIBCCALL rand)(void);
__LIBC void (__LIBCCALL srand)(__UINT32_TYPE__ __seed);
#else /* __KERNEL__ */
__LIBC int (__LIBCCALL rand)(void);
__LIBC void (__LIBCCALL srand)(long __seed);
#endif /* !__KERNEL__ */
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(atof)
__NAMESPACE_STD_USING(atoi)
__NAMESPACE_STD_USING(atol)
__NAMESPACE_STD_USING(strtod)
__NAMESPACE_STD_USING(strtol)
__NAMESPACE_STD_USING(strtoul)
__NAMESPACE_STD_USING(rand)
__NAMESPACE_STD_USING(srand)
#ifdef __USE_ISOC99
__NAMESPACE_STD_USING(atoll)
__NAMESPACE_STD_USING(strtof)
__NAMESPACE_STD_USING(strtold)
__NAMESPACE_STD_USING(strtoll)
__NAMESPACE_STD_USING(strtoull)
#endif /* __USE_ISOC99 */
#endif /* !__NAMESPACE_STD_USING */

#ifdef __USE_POSIX
#ifdef __CRT_GLC
#ifdef __KERNEL__
__LIBC __PORT_NODOS_ALT(rand) __NONNULL((1)) __UINT32_TYPE__ (__LIBCCALL rand_r)(__UINT32_TYPE__ *__restrict __seed);
#else /* __KERNEL__ */
__LIBC __PORT_NODOS_ALT(rand) __NONNULL((1)) int (__LIBCCALL rand_r)(unsigned int *__restrict __seed);
#endif /* !__KERNEL__ */
#endif /* __CRT_GLC */
#endif /* __USE_POSIX */

#ifdef __USE_MISC
#ifndef __cfree_defined
#define __cfree_defined 1
#ifdef __KERNEL__
__REDIRECT_VOID(__LIBC,,__LIBCCALL,cfree,(void *__restrict __mallptr),kfree,(__mallptr))
#else /* __KERNEL__ */
__REDIRECT_VOID(__LIBC,,__LIBCCALL,cfree,(void *__restrict __mallptr),free,(__mallptr))
#endif /* !__KERNEL__ */
#endif /* !__cfree_defined */
#ifdef __CRT_GLC
__LIBC __PORT_NODOS int (__LIBCCALL getloadavg)(double __loadavg[], int __nelem);
#endif /* __CRT_GLC */
#endif /* __USE_MISC */

#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K))
#ifdef __CRT_GLC
#ifndef __valloc_defined
#define __valloc_defined 1
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_NODOS __WUNUSED
                  __MALL_ATTR_PAGEALIGNED __ATTR_ALLOC_SIZE((1)),
                  void *,__LIBCCALL,valloc,(size_t __n_bytes),(__n_bytes))
#endif /* !__valloc_defined */
#endif /* __CRT_GLC */
#endif

#ifdef __USE_XOPEN2K
#ifdef __CRT_GLC
#ifndef __posix_memalign_defined
#define __posix_memalign_defined 1
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL posix_memalign)(void **__restrict __pp, size_t __alignment, size_t __n_bytes);
#endif /* !__posix_memalign_defined */
#endif /* __CRT_GLC */
#endif /* __USE_XOPEN2K */

#ifdef __USE_ISOC11
#ifdef __CRT_GLC /* XXX: DOS may add this one some time in the future. */
__REDIRECT_EXCEPT_(__LIBC,
                   __XATTR_RETNONNULL __PORT_NODOS __WUNUSED
                   __ATTR_ALLOC_ALIGN(1) __ATTR_ALLOC_SIZE((2))
                   __ATTR_MALLOC,void *,__LIBCCALL,aligned_alloc,
                  (size_t __alignment, size_t __n_bytes),
                   memalign,(__alignment,__n_bytes))
#endif /* __CRT_GLC */
#endif /* __USE_ISOC11 */

#ifdef __KERNEL__
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
#if __SIZEOF_LONG__ == 4
__REDIRECT(__LIBC,,long,__LIBCCALL,random,(void),rand,())
#endif /* __SIZEOF_LONG__ == 4 */
#if __SIZEOF_INT__ == 4
__REDIRECT_VOID(__LIBC,,__LIBCCALL,srandom,(unsigned int __seed),srand,(__seed))
#endif /* __SIZEOF_INT__ == 4 */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED  */

#else /* __KERNEL__ */

#define MB_CUR_MAX                 (__ctype_get_mb_cur_max())
__LIBC __WUNUSED size_t (__LIBCCALL __ctype_get_mb_cur_max)(void);

__NAMESPACE_STD_BEGIN
__REDIRECT_UFS(__LIBC,__WUNUSED __NONNULL((1)),char *,__LIBCCALL,getenv,(char const *__name),(__name))
__LIBC int (__LIBCCALL mblen)(char const *__s, size_t __n);
__LIBC int (__LIBCCALL mbtowc)(wchar_t *__restrict __pwc, char const *__restrict __s, size_t __n);
__LIBC int (__LIBCCALL wctomb)(char *__s, wchar_t __wchar);
__LIBC size_t (__LIBCCALL mbstowcs)(wchar_t *__restrict __pwcs, char const *__restrict __s, size_t __n);
__LIBC size_t (__LIBCCALL wcstombs)(char *__restrict __s, wchar_t const *__restrict __pwcs, size_t __n);
#ifndef __std_system_defined
#define __std_system_defined 1
__REDIRECT_EXCEPT_UFS(__LIBC,,int,__LIBCCALL,system,(char const *__command),(__command))
#endif /* !__std_system_defined */
#ifndef __std_abort_defined
#define __std_abort_defined 1
__LIBC __ATTR_NORETURN void (__LIBCCALL abort)(void);
#endif /* !__std_abort_defined */
#ifndef __std_exit_defined
#define __std_exit_defined 1
__LIBC __ATTR_NORETURN void (__LIBCCALL exit)(int __status);
#endif /* !__std_exit_defined */
__LIBC int (__LIBCCALL atexit)(void (*__LIBCCALL __func)(void));
#if defined(__USE_ISOC11) || defined(__USE_ISOCXX11)
#ifdef __DOS_COMPAT__
__REDIRECT_VOID(__LIBC,__ATTR_NORETURN,__LIBCCALL,quick_exit,(int __status),exit,(__status))
#ifdef __cplusplus
extern "C++" { __REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,at_quick_exit,(void (*__LIBCCALL __func)(void)),at_quick_exit,(__func)) }
#else /* __cplusplus */
__REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,at_quick_exit,(void (*__LIBCCALL __func)(void)),atexit,(__func))
#endif /* !__cplusplus */
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_NORETURN void (__LIBCCALL quick_exit)(int __status);
#ifdef __cplusplus
extern "C++" { __REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,at_quick_exit,(void (*__LIBCCALL __func)(void)),at_quick_exit,(__func)) }
#else /* __cplusplus */
__LIBC __NONNULL((1)) int (__LIBCCALL at_quick_exit)(void (*__LIBCCALL __func)(void));
#endif /* !__cplusplus */
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_ISOC11 || __USE_ISOCXX11 */
#ifdef __USE_ISOC99
__REDIRECT_VOID(__LIBC,__ATTR_NORETURN,__LIBCCALL,_Exit,(int __status),_exit,(__status))
#endif /* __USE_ISOC99 */
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(getenv)
__NAMESPACE_STD_USING(mblen)
__NAMESPACE_STD_USING(mbtowc)
__NAMESPACE_STD_USING(wctomb)
__NAMESPACE_STD_USING(mbstowcs)
__NAMESPACE_STD_USING(wcstombs)
#ifndef __system_defined
#define __system_defined 1
__NAMESPACE_STD_USING(system)
#endif /* !__system_defined */
#ifndef __abort_defined
#define __abort_defined 1
__NAMESPACE_STD_USING(abort)
#endif /* !__abort_defined */
#ifndef __exit_defined
#define __exit_defined 1
__NAMESPACE_STD_USING(exit)
#endif /* !__exit_defined */
__NAMESPACE_STD_USING(atexit)
#if defined(__USE_ISOC11) || defined(__USE_ISOCXX11)
__NAMESPACE_STD_USING(quick_exit)
__NAMESPACE_STD_USING(at_quick_exit)
#endif /* __USE_ISOC11 || __USE_ISOCXX11 */
#ifdef __USE_ISOC99
__NAMESPACE_STD_USING(_Exit)
#endif /* __USE_ISOC99 */
#endif /* !__CXX_SYSTEM_HEADER */

#if defined(__USE_MISC) || defined(__USE_XOPEN)
#ifdef __CRT_GLC
__LIBC __PORT_NODOS double (__LIBCCALL drand48)(void);
__LIBC __PORT_NODOS long (__LIBCCALL lrand48)(void);
__LIBC __PORT_NODOS long (__LIBCCALL mrand48)(void);
__LIBC __PORT_NODOS __NONNULL((1)) double (__LIBCCALL erand48)(unsigned short __xsubi[3]);
__LIBC __PORT_NODOS __NONNULL((1)) long (__LIBCCALL nrand48)(unsigned short __xsubi[3]);
__LIBC __PORT_NODOS __NONNULL((1)) long (__LIBCCALL jrand48)(unsigned short __xsubi[3]);
__LIBC __PORT_NODOS void (__LIBCCALL srand48)(long __seedval);
__LIBC __PORT_NODOS __NONNULL((1)) unsigned short *(__LIBCCALL seed48)(unsigned short __seed16v[3]);
__LIBC __PORT_NODOS __NONNULL((1)) void (__LIBCCALL lcong48)(unsigned short __param[7]);
#endif /* __CRT_GLC */
#endif /* __USE_MISC || __USE_XOPEN */

#if defined(__USE_MISC) || defined(__USE_XOPEN) || defined(__USE_DOS)
__REDIRECT_UFSDPA(__LIBC,__NONNULL((1)),int,__LIBCCALL,putenv,(char *__string),(__string))
#endif /* __USE_MISC || __USE_XOPEN || __USE_DOS */

#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
#if (defined(__CRT_KOS) || !defined(__CRT_GLC))
#if __SIZEOF_LONG__ == __SIZEOF_INT__
__REDIRECT(__LIBC,,long,__LIBCCALL,random,(void),rand,())
__REDIRECT_VOID(__LIBC,,__LIBCCALL,srandom,(unsigned int __seed),srand,(__seed))
#else /* __SIZEOF_LONG__ == __SIZEOF_INT__ */
__LOCAL long (__LIBCCALL random)(void) { return (long)rand(); }
__LOCAL void (__LIBCCALL srandom)(unsigned int __seed) { srand(__seed); }
#endif /* __SIZEOF_LONG__ != __SIZEOF_INT__ */
#else /* __CRT_KOS || !__CRT_GLC */
__LIBC long (__LIBCCALL random)(void);
__LIBC void (__LIBCCALL srandom)(unsigned int __seed);
#endif /* !__CRT_KOS && __CRT_GLC */
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __NONNULL((2)) char *(__LIBCCALL initstate)(unsigned int __seed, char *__statebuf, size_t __statelen);
__LIBC __PORT_NODOS __NONNULL((1)) char *(__LIBCCALL setstate)(char *__statebuf);
__LIBC __PORT_NODOS __WUNUSED char *(__LIBCCALL l64a)(long __n);
__LIBC __PORT_NODOS __WUNUSED __ATTR_PURE __NONNULL((1)) long (__LIBCCALL a64l)(char const *__s);
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_NODOS __XATTR_RETNONNULL __WUNUSED,char *,__LIBCCALL,realpath,(char const *__restrict __name, char *__resolved),(__name,__resolved))
#endif /* __CRT_GLC */
#ifdef __CRT_KOS
/* NOTE: I didn't come up with this function (https://docs.oracle.com/cd/E36784_01/html/E36874/frealpath-3c.html),
 *       but it seems to be something that GLibC isn't implementing for some reason...
 *       Because of that I didn't really know where to put this, so I put it in the
 *       same _SOURCE-block as its `realpath()' companion. */
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY __XATTR_RETNONNULL __WUNUSED,char *,__LIBCCALL,frealpath,
                     (__fd_t __fd, char *__resolved, size_t __bufsize),(__fd,__resolved,__bufsize))
#endif /* __CRT_KOS */
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */

#ifdef __USE_MISC
#ifdef __CRT_GLC
struct drand48_data {
    unsigned short __x[3];
    unsigned short __old_x[3];
    unsigned short __c;
    unsigned short __init;
    __ULONGLONG    __a;
};

__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL drand48_r)(struct drand48_data *__restrict __buffer, double *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL erand48_r)(unsigned short __xsubi[3], struct drand48_data *__restrict __buffer, double *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL lrand48_r)(struct drand48_data *__restrict __buffer, long *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL nrand48_r)(unsigned short __xsubi[3], struct drand48_data *__restrict __buffer, long *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL mrand48_r)(struct drand48_data *__restrict __buffer, long *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL jrand48_r)(unsigned short __xsubi[3], struct drand48_data *__restrict __buffer, long *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((2))   int (__LIBCCALL srand48_r)(long __seedval, struct drand48_data *__buffer);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL seed48_r)(unsigned short __seed16v[3], struct drand48_data *__buffer);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL lcong48_r)(unsigned short __param[7], struct drand48_data *__buffer);

struct random_data {
    __INT32_TYPE__ *fptr;
    __INT32_TYPE__ *rptr;
    __INT32_TYPE__ *state;
    int             rand_type;
    int             rand_deg;
    int             rand_sep;
    __INT32_TYPE__ *end_ptr;
};
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL random_r)(struct random_data *__restrict __buf, __INT32_TYPE__ *__restrict __result);
__LIBC __PORT_NODOS __NONNULL((2)) int (__LIBCCALL srandom_r)(unsigned int __seed, struct random_data *__buf);
__LIBC __PORT_NODOS __NONNULL((2,4)) int (__LIBCCALL initstate_r)(unsigned int __seed, char *__restrict __statebuf, size_t __statelen, struct random_data *__restrict __buf);
__LIBC __PORT_NODOS __NONNULL((1,2)) int (__LIBCCALL setstate_r)(char *__restrict __statebuf, struct random_data *__restrict __buf);
__LIBC __PORT_NODOS __NONNULL((1)) int (__LIBCCALL on_exit)(void (__LIBCCALL *__func)(int __status, void *__arg), void *__arg);
__LIBC __PORT_NODOS int (__LIBCCALL clearenv)(void);
__REDIRECT_UFS64(__LIBC,__PORT_NODOS __WUNUSED __NONNULL((1)),int,__LIBCCALL,mkstemps,(char *__template, int __suffixlen),(__template,__suffixlen))
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1)) int (__LIBCCALL rpmatch)(char const *__response);
#ifdef __USE_LARGEFILE64
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1)) int (__LIBCCALL mkstemps64)(char *__template, int __suffixlen);
#endif /* __USE_LARGEFILE64 */
#endif /* __CRT_GLC */
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED __NONNULL((3,4)),char *,__LIBCCALL,qecvt,(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),_ecvt,(__val,__ndigit,__decptr,__sign))
__REDIRECT(__LIBC,__WUNUSED __NONNULL((3,4)),char *,__LIBCCALL,qfcvt,(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),_fcvt,(__val,__ndigit,__decptr,__sign))
#else
__LIBC __WUNUSED __NONNULL((3,4)) char *(__LIBCCALL qecvt)(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign);
__LIBC __WUNUSED __NONNULL((3,4)) char *(__LIBCCALL qfcvt)(long double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign);
#endif
#endif /* __USE_MISC */

#ifdef __USE_GNU
#include <xlocale.h>
#if defined(__CRT_DOS) && __SIZEOF_LONG__ == 4
__REDIRECT(__LIBC,__NONNULL((1,4)),long,__LIBCCALL,strtol_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),_strtol_l,(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,__NONNULL((1,4)),unsigned long,__LIBCCALL,strtoul_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),_strtoul_l,(__nptr,__endptr,__base,__locale))
#elif defined(__CRT_DOS) && __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,__NONNULL((1,4)),long,__LIBCCALL,strtol_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),_strtoi64_l,(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,__NONNULL((1,4)),unsigned long,__LIBCCALL,strtoul_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),_strtoui64_l,(__nptr,__endptr,__base,__locale))
#else
__LIBC __NONNULL((1,4)) long (__LIBCCALL strtol_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale);
__LIBC __NONNULL((1,4)) unsigned long (__LIBCCALL strtoul_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale);
#endif
#if defined(__CRT_DOS) && __SIZEOF_LONG_LONG__ == 8
__REDIRECT(__LIBC,__NONNULL((1,4)),__LONGLONG,__LIBCCALL,strtoll_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),_strtoi64_l,(__nptr,__endptr,__base,__locale))
__REDIRECT(__LIBC,__NONNULL((1,4)),__ULONGLONG,__LIBCCALL,strtoull_l,(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale),_strtoui64_l,(__nptr,__endptr,__base,__locale))
#else
__LIBC __NONNULL((1,4)) __LONGLONG (__LIBCCALL strtoll_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale);
__LIBC __NONNULL((1,4)) __ULONGLONG (__LIBCCALL strtoull_l)(char const *__restrict __nptr, char **__restrict __endptr, int __base, __locale_t __locale);
#endif
__REDIRECT_DPA(__LIBC,__NONNULL((1,3)),double,__LIBCCALL,strtod_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __locale),(__nptr,__endptr,__locale))
__REDIRECT_DPA(__LIBC,__NONNULL((1,3)),float,__LIBCCALL,strtof_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __locale),(__nptr,__endptr,__locale))
__REDIRECT_DPA(__LIBC,__NONNULL((1,3)),long double,__LIBCCALL,strtold_l,(char const *__restrict __nptr, char **__restrict __endptr, __locale_t __locale),(__nptr,__endptr,__locale))
#ifdef __DOS_COMPAT__
__REDIRECT_UFS_(__LIBC,__WUNUSED __NONNULL((1)),char *,__LIBCCALL,secure_getenv,(char const *__name),getenv,(__name))
#else /* __DOS_COMPAT__ */
__REDIRECT_UFS(__LIBC,__WUNUSED __NONNULL((1)),char *,__LIBCCALL,secure_getenv,(char const *__name),(__name))
#endif /* !__DOS_COMPAT__ */

#ifdef __CRT_GLC
__LIBC __PORT_NODOS __NONNULL((2)) int (__LIBCCALL ptsname_r)(__fd_t __fd, char *__buf, size_t __buflen);
__LIBC __PORT_NODOS int (__LIBCCALL getpt)(void);
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_NODOS __XATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __NONNULL((1)),char *,__LIBCCALL,canonicalize_file_name,(char const *__name),(__name))
__REDIRECT_UFS64(__LIBC,__PORT_NODOS __WUNUSED __NONNULL((1)),int,__LIBCCALL,mkostemp,(char *__template, int __flags),(__template,__flags))
__REDIRECT_UFS64(__LIBC,__PORT_NODOS __WUNUSED __NONNULL((1)),int,__LIBCCALL,mkostemps,(char *__template, int __suffixlen, int __flags),(__template,__suffixlen,__flags))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1)) int (__LIBCCALL mkostemp64)(char *__template, int __flags);
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1)) int (__LIBCCALL mkostemps64)(char *__template, int __suffixlen, int __flags);
#endif /* __USE_LARGEFILE64 */
#endif /* __CRT_GLC */
#endif /* __USE_GNU */

#ifdef __USE_XOPEN2K
#ifdef __DOS_COMPAT__
__SYSDECL_END
#include <hybrid/malloc.h>
#include <hybrid/string.h>
__SYSDECL_BEGIN
__LOCAL __NONNULL((2)) int (__LIBCCALL setenv)(char const *__name, char const *__val, int __replace);
__LOCAL __NONNULL((1)) int (__LIBCCALL unsetenv)(char const *__name);

__REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,__dos_putenv,(char *__string),_putenv,(__string))
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_setenv,(char const *__name, char const *__val),_putenv_s,(__name,__val))
__LOCAL __NONNULL((2)) int (__LIBCCALL setenv)(char const *__name, char const *__val, int __replace) {
    if (!__replace && __NAMESPACE_STD_SYM getenv(__name)) return 0;
    return __dos_setenv(__name,__val);
}
__LOCAL __NONNULL((1)) int (__LIBCCALL unsetenv)(char const *__name) {
    char *__copy; size_t __namelen; int __result; if (!__name) return -1;
    __namelen = __hybrid_strlen(__name);
    __copy = (char *)__hybrid_malloc((__namelen+2)*sizeof(char));
    if __unlikely(!__copy) return -1;
    __hybrid_memcpy(__copy,__name,__namelen*sizeof(char));
    __copy[__namelen] = '=';
    __copy[__namelen+1] = '\0';
    __result = __dos_putenv(__copy);
    __hybrid_free(__copy);
    return __result;
}
#else /* __DOS_COMPAT__ */
__LIBC __NONNULL((2)) int (__LIBCCALL setenv)(char const *__name, char const *__val, int __replace);
__LIBC __NONNULL((1)) int (__LIBCCALL unsetenv)(char const *__name);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_XOPEN2K */

#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8))
#ifndef __mktemp_defined
#define __mktemp_defined 1
__REDIRECT_UFSDPA(__LIBC,__NONNULL((1)),char *,__LIBCCALL,mktemp,(char *__template),(__template))
#endif /* !__mktemp_defined */
#endif

#if defined(__USE_MISC) || defined(__USE_DOS) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K8))
__REDIRECT_DPA(__LIBC,__WUNUSED __NONNULL((3,4)),char *,__LIBCCALL,ecvt,(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),(__val,__ndigit,__decptr,__sign))
__REDIRECT_DPA(__LIBC,__WUNUSED __NONNULL((3,4)),char *,__LIBCCALL,fcvt,(double __val, int __ndigit, int *__restrict __decptr, int *__restrict __sign),(__val,__ndigit,__decptr,__sign))
#endif

#if defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8)
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __WUNUSED __NONNULL((1,2,3)) int (__LIBCCALL getsubopt)(char **__restrict __optionp, char *const *__restrict __tokens, char **__restrict __valuep);
#endif /* __CRT_GLC */

#ifdef __DOS_COMPAT__
#ifndef __mktemp_defined
__REDIRECT(__LIBC,__NONNULL((1)),char *,__LIBCCALL,__dos_mktemp,(char *__template),_mktemp,(__template))
#else
#define __dos_mktemp(template) mktemp(template)
#endif /* !__mktemp_defined */
__LOCAL __WUNUSED __NONNULL((1)) int (__LIBCCALL mkstemp)(char *__template) { return __dos_mktemp(__template) ? 0 : -1; }
#ifdef __USE_LARGEFILE64
__LOCAL __WUNUSED __NONNULL((1)) int (__LIBCCALL mkstemp64)(char *__template) { return __dos_mktemp(__template) ? 0 : -1; }
#endif /* __USE_LARGEFILE64 */
#else /* __DOS_COMPAT__ */
__REDIRECT_UFS64(__LIBC,__WUNUSED __NONNULL((1)),int,__LIBCCALL,mkstemp,(char *__template),(__template))
#ifdef __USE_LARGEFILE64
__LIBC __WUNUSED __NONNULL((1)) int (__LIBCCALL mkstemp64)(char *__template);
#endif /* __USE_LARGEFILE64 */
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_XOPEN_EXTENDED || __USE_XOPEN2K8 */

#ifdef __USE_XOPEN2K8
#ifdef __DOS_COMPAT__
__REDIRECT_UFS_(__LIBC,__WUNUSED __NONNULL((1)),char *,__LIBCCALL,mkdtemp,(char *__template),_mktemp,(__template))
#else /* __DOS_COMPAT__ */
__REDIRECT_UFS(__LIBC,__WUNUSED __NONNULL((1)),char *,__LIBCCALL,mkdtemp,(char *__template),(__template))
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_XOPEN2K8 */

#ifdef __CRT_GLC
#ifdef __USE_XOPEN
__LIBC __PORT_NODOS __NONNULL((1)) void (__LIBCCALL setkey)(char const *__key);
__LIBC __PORT_NODOS int (__LIBCCALL grantpt)(__fd_t __fd);
__LIBC __PORT_NODOS int (__LIBCCALL unlockpt)(__fd_t __fd);
__LIBC __PORT_NODOS __WUNUSED char *(__LIBCCALL ptsname)(__fd_t __fd);
#endif /* __USE_XOPEN */

#ifdef __USE_XOPEN2KXSI
__LIBC __PORT_NODOS __WUNUSED int (__LIBCCALL posix_openpt)(int __oflag);
#endif /* __USE_XOPEN2KXSI */
#endif /* __CRT_GLC */
#endif /* !__KERNEL__ */

#define __DOS_MAX_PATH         260
#define __DOS_MAX_DRIVE        3
#define __DOS_MAX_DIR          256
#define __DOS_MAX_FNAME        256
#define __DOS_MAX_EXT          256
#define __DOS_OUT_TO_DEFAULT   0
#define __DOS_OUT_TO_STDERR    1
#define __DOS_OUT_TO_MSGBOX    2
#define __DOS_REPORT_ERRMODE   3
#define __DOS_WRITE_ABORT_MSG  0x1
#define __DOS_CALL_REPORTFAULT 0x2
#define __DOS_MAX_ENV          0x7fff

__SYSDECL_END

#include "parts/malloc.h"

#ifdef __USE_KOS
#ifdef _ALLOCA_H
#include "parts/kos2/malloca.h"
#define malloca(s) __malloca(s)
#define calloca(s) __calloca(s)
#define freea(p)   __freea(p)
#ifndef __USE_KOS_DEPRECATED
#define amalloc(s) __malloca(s)
#define acalloc(s) __calloca(s)
#define afree(p)   __freea(p)
#endif /* !__USE_KOS_DEPRECATED */
#endif
#ifndef _PARTS_KOS2_STDLIB_H
#include "parts/kos2/stdlib.h"
#endif
#ifdef _WCHAR_H
#ifndef _PARTS_KOS2_WSTDLIB_H
#include "parts/kos2/wstdlib.h"
#endif
#endif
#ifdef _UCHAR_H
#ifndef _PARTS_KOS2_USTDLIB_H
#include "parts/kos2/ustdlib.h"
#endif
#endif
#endif /* __USE_KOS */

#ifdef __USE_KOS3
#ifdef _WCHAR_H
#ifndef _PARTS_KOS3_WSTDLIB_H
#include "parts/kos3/wstdlib.h"
#endif
#endif
#ifdef _UCHAR_H
#ifndef _PARTS_KOS3_USTDLIB_H
#include "parts/kos3/ustdlib.h"
#endif
#endif
#endif /* __USE_KOS3 */

#ifdef __USE_DOS
#ifndef _PARTS_DOS_STDLIB_H
#include "parts/dos/stdlib.h"
#endif
#endif

#ifdef __USE_EXCEPT
#include "parts/kos3/except/stdlib.h"
#endif

#endif /* !_STDLIB_H */
