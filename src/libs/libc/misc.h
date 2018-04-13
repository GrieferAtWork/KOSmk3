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
#ifndef GUARD_LIBS_LIBC_MISC_H
#define GUARD_LIBS_LIBC_MISC_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     MISC                                                                              */
/* ===================================================================================== */
#ifndef __COMPAR_FN_T
#define __COMPAR_FN_T
typedef int (__LIBCCALL *__compar_fn_t)(void const *a, void const *b);
typedef __compar_fn_t comparison_fn_t;
#endif /* __COMPAR_FN_T */
__NAMESPACE_STD_BEGIN
#ifndef __std_div_t_defined
#define __std_div_t_defined 1
typedef struct { int quot,rem; } div_t;
#endif /* !__std_div_t_defined */
#ifndef __std_ldiv_t_defined
#define __std_ldiv_t_defined 1
typedef struct { long quot,rem; } ldiv_t;
#endif /* !__std_ldiv_t_defined */
#ifndef __std_lldiv_t_defined
#define __std_lldiv_t_defined 1
typedef struct { __LONGLONG quot,rem; } lldiv_t;
#endif /* !__std_lldiv_t_defined */
__NAMESPACE_STD_END
#ifndef __div_t_defined
#define __div_t_defined 1
__NAMESPACE_STD_USING(div_t)
#endif /* !__div_t_defined */
#ifndef __ldiv_t_defined
#define __ldiv_t_defined 1
__NAMESPACE_STD_USING(ldiv_t)
#endif /* !__ldiv_t_defined */
#ifndef __lldiv_t_defined
#define __lldiv_t_defined 1
__NAMESPACE_STD_USING(lldiv_t)
#endif /* !__lldiv_t_defined */
#ifndef __imaxdiv_t_defined
#define __imaxdiv_t_defined 1
typedef struct {
    __INTMAX_TYPE__ quot; /* Quotient. */
    __INTMAX_TYPE__ rem;  /* Remainder. */
} imaxdiv_t;
#endif /* !__imaxdiv_t_defined */
typedef int (__LIBCCALL *compar_d_fn_t)(void const *a, void const *b, void *arg);
typedef int (__LIBCCALL *compar_s_fn_t)(void *arg, void const *a, void const *b);
INTDEF void LIBCCALL libc_qsort(void *base, size_t nmemb, size_t size, comparison_fn_t compar);
INTDEF void LIBCCALL libc_qsort_r(void *base, size_t nmemb, size_t size, compar_d_fn_t compar, void *arg);
INTDEF void LIBCCALL libc_qsort_s(void *base, size_t nmemb, size_t size, compar_s_fn_t compar, void *arg);
INTDEF void *LIBCCALL libc_bsearch(void const *key, void const *base, size_t nmemb, size_t size, comparison_fn_t compar);
INTDEF void *LIBCCALL libc_bsearch_r(void const *key, void const *base, size_t nmemb, size_t size, compar_d_fn_t compar, void *arg);
INTDEF void *LIBCCALL libc_bsearch_s(void const *key, void const *base, size_t nmemb, size_t size, compar_s_fn_t compar, void *arg);
INTDEF int LIBCCALL libc_abs(int x);
INTDEF long LIBCCALL libc_labs(long x);
INTDEF long long LIBCCALL libc_llabs(long long x);
INTDEF s64 LIBCCALL libc_abs64(s64 x);
INTDEF intmax_t LIBCCALL libc_imaxabs(intmax_t x);
INTDEF div_t LIBCCALL libc_div(int numer, int denom);
INTDEF ldiv_t LIBCCALL libc_ldiv(long numer, long denom);
INTDEF lldiv_t LIBCCALL libc_lldiv(long long numer, long long denom);
INTDEF imaxdiv_t LIBCCALL libc_imaxdiv(intmax_t numer, intmax_t denom);
INTDEF u32 LIBCCALL libc_rol32(u32 val, unsigned int shift);
INTDEF u32 LIBCCALL libc_ror32(u32 val, unsigned int shift);
INTDEF u64 LIBCCALL libc_rol64(u64 val, unsigned int shift);
INTDEF u64 LIBCCALL libc_ror64(u64 val, unsigned int shift);
INTDEF u16 LIBCCALL libc_bswap16(u16 x);
INTDEF u32 LIBCCALL libc_bswap32(u32 x);
INTDEF u64 LIBCCALL libc_bswap64(u64 x);
INTDEF major_t LIBCCALL libc_gnu_dev_major(dev_t dev);
INTDEF minor_t LIBCCALL libc_gnu_dev_minor(dev_t dev);
INTDEF dev_t LIBCCALL libc_gnu_dev_makedev(major_t major, minor_t minor);
INTDEF int LIBCCALL libc_ffs8(u8 i);
INTDEF int LIBCCALL libc_ffs16(u16 i);
INTDEF int LIBCCALL libc_ffs32(u32 i);
INTDEF int LIBCCALL libc_ffs64(u64 i);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_MISC_H */
