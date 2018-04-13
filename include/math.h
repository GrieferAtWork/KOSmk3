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
#ifndef _MATH_H
#define _MATH_H 1

/* math.h header designed to link against an unmodified -lm */

#include <__stdinc.h>
#include <features.h>
#include <bits/math-vector.h>
#include <bits/huge_val.h>
#ifdef __USE_ISOC99
#include <bits/huge_valf.h>
#include <bits/huge_vall.h>
#include <bits/inf.h>
#include <bits/nan.h>
#endif /* __USE_ISOC99 */
#include <bits/mathdef.h>

/* Declarations for math functions.
   Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

__SYSDECL_BEGIN

#define __SIMD_DECL(function)                               __PP_CAT2(__DECL_SIMD_,function)
#define __MATHCALL_VEC(function,suffix,args)                __SIMD_DECL(__MATH_PRECNAME(function,suffix)) __MATHCALL(function,suffix,args)
#define __MATHDECL_VEC(type,function,suffix,args)           __SIMD_DECL(__MATH_PRECNAME(function,suffix)) __MATHDECL(type,function,suffix,args)
#define __MATHCALL(function,suffix,args)                    __MATHDECL(_Mdouble_,function,suffix,args)
#define __MATHDECL(type,function,suffix,args)               __MATHDECL_1(type,function,suffix,args); __MATHDECL_1(type,__PP_CAT2(__,function),suffix,args)
#define __MATHCALLX(function,suffix,args,attrib)            __MATHDECLX(_Mdouble_,function,suffix,args,attrib)
#define __MATHDECLX(type,function,suffix,args,attrib)       __MATHDECL_1_ATTR(type,function,suffix,args,attrib); __MATHDECL_1_ATTR(type,__PP_CAT2(__,function),suffix,args,attrib)
#define __MATHDECL_1(type,function,suffix,args)             __LIBC type __NOTHROW((__LIBCCALL __MATH_PRECNAME(function,suffix)) args)
#define __MATHDECL_1_ATTR(type,function,suffix,args,attrib) __LIBC attrib type __NOTHROW((__LIBCCALL __MATH_PRECNAME(function,suffix)) args)
#define _Mdouble_                double
#define __MATH_PRECNAME(name,r)  name##r
#define __MATH_DECLARING_DOUBLE  1
#define __MATHNS_BEGIN           __NAMESPACE_STD_BEGIN
#define __MATHNS_END             __NAMESPACE_STD_END
#ifdef __CXX_SYSTEM_HEADER
#define __MATHNS_USING(name,r)   /* Nothing */
#else
#define __MATHNS_USING(name,r)   __NAMESPACE_STD_USING(name##r)
#endif
#include <bits/mathcalls.h>
#undef  _Mdouble_
#undef  __MATH_PRECNAME
#undef  __MATH_DECLARING_DOUBLE
#undef  __MATHNS_BEGIN
#undef  __MATHNS_END
#undef  __MATHNS_USING

#ifdef __USE_ISOC99
#ifndef _Mfloat_
#define _Mfloat_                 float
#endif
#define _Mdouble_                _Mfloat_
#define __MATH_PRECNAME(name,r)  name##f##r
#define __MATH_DECLARING_DOUBLE  0
#define __MATHNS_BEGIN           __NAMESPACE_STD_BEGIN
#define __MATHNS_END             __NAMESPACE_STD_END
#ifdef __CXX_SYSTEM_HEADER
#define __MATHNS_USING(name,r)   /* Nothing */
#else
#define __MATHNS_USING(name,r)   __NAMESPACE_STD_USING(name##f##r)
#endif
#include <bits/mathcalls.h>
#undef  _Mdouble_
#undef  __MATH_PRECNAME
#undef  __MATH_DECLARING_DOUBLE
#undef  __MATHNS_BEGIN
#undef  __MATHNS_END
#undef  __MATHNS_USING
#ifdef __COMPILER_HAVE_LONGDOUBLE
#ifndef _Mlong_double_
#define _Mlong_double_  long double
#endif
#define _Mdouble_        _Mlong_double_
#define __MATH_PRECNAME(name,r) name##l##r
#define __MATH_DECLARING_DOUBLE  0
#define __MATH_DECLARE_LDOUBLE   1
#define __MATHNS_BEGIN           __NAMESPACE_STD_BEGIN
#define __MATHNS_END             __NAMESPACE_STD_END
#ifdef __CXX_SYSTEM_HEADER
#define __MATHNS_USING(name,r)   /* Nothing */
#else
#define __MATHNS_USING(name,r)   __NAMESPACE_STD_USING(name##l##r)
#endif
#include <bits/mathcalls.h>
#undef  _Mdouble_
#undef  __MATH_PRECNAME
#undef  __MATH_DECLARING_DOUBLE
#undef  __MATHNS_BEGIN
#undef  __MATHNS_END
#undef  __MATHNS_USING
#endif /* __COMPILER_HAVE_LONGDOUBLE */
#endif /* __USE_ISOC99 */
#undef  __MATHDECL_1_ATTR
#undef  __MATHDECL_1
#undef  __MATHDECL
#undef  __MATHCALL


#if defined __USE_MISC || defined __USE_XOPEN
/* This variable is used by `gamma' and `lgamma'. */
#undef signgam
__LIBC int (signgam);
#endif

#ifdef __USE_ISOC99
/* Get the architecture specific values describing the floating-point
 * evaluation.  The following symbols will get defined:
 *
 *  float_t    floating-point type at least as wide as `float' used
 *      to evaluate `float' expressions
 *  double_t    floating-point type at least as wide as `double' used
 *      to evaluate `double' expressions
 *
 *  FLT_EVAL_METHOD
 *      Defined to
 *        0    if `float_t' is `float' and `double_t' is `double'
 *        1    if `float_t' and `double_t' are `double'
 *        2    if `float_t' and `double_t' are `long double'
 *        else    `float_t' and `double_t' are unspecified
 *
 *  INFINITY    representation of the infinity value of type `float'
 *
 *  FP_FAST_FMA
 *  FP_FAST_FMAF
 *  FP_FAST_FMAL
 *      If defined it indicates that the `fma' function
 *      generally executes about as fast as a multiply and an add.
 *      This macro is defined only iff the `fma' function is
 *      implemented directly with a hardware multiply-add instructions.
 *
 *  FP_ILOGB0    Expands to a value returned by `ilogb (0.0)'.
 *  FP_ILOGBNAN    Expands to a value returned by `ilogb (NAN)'.
 *
 *  DECIMAL_DIG    Number of decimal digits supported by conversion between
 *      decimal and all internal floating-point formats. */

/* All floating-point numbers can be put in one of these categories. */
#undef FP_NAN
#undef FP_INFINITE
#undef FP_ZERO
#undef FP_SUBNORMAL
#undef FP_NORMAL
enum {
 FP_NAN       = 0,
 FP_INFINITE  = 1,
 FP_ZERO      = 2,
 FP_SUBNORMAL = 3,
 FP_NORMAL    = 4
};
#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

#ifdef __COMPILER_HAVE_C11_GENERIC
#ifdef __COMPILER_HAVE_LONGDOUBLE
#define __FPFUNC(x,f,d,l)     _Generic(x,float: f(x),double: d(x),default: l(x))
#define __FPFUNC2(x,y,f,d,l)  _Generic((x)+(y),float: f(x,y),double: d(x,y),default: l(x,y))
#else /* __COMPILER_HAVE_LONGDOUBLE */
#define __FPFUNC(x,f,d,l)     _Generic(x,float: f(x),default: d(x))
#define __FPFUNC2(x,y,f,d,l)  _Generic((x)+(y),float: f(x,y),default: d(x,y))
#endif /* !__COMPILER_HAVE_LONGDOUBLE */
#elif !defined(__NO_builtin_choose_expr) && \
      !defined(__NO_builtin_types_compatible_p) && \
       defined(__COMPILER_HAVE_TYPEOF)
#ifdef __COMPILER_HAVE_LONGDOUBLE
#define __FPFUNC(x,f,d,l) \
   __builtin_choose_expr(__builtin_types_compatible_p(__typeof__((x)+(float)0),float),f(x), \
   __builtin_choose_expr(__builtin_types_compatible_p(__typeof__((x)+(double)0),double),d(x),l(x)))
#define __FPFUNC2(x,y,f,d,l) \
   __builtin_choose_expr(__builtin_types_compatible_p(__typeof__((x)+(y)+(float)0),float),f(x,y), \
   __builtin_choose_expr(__builtin_types_compatible_p(__typeof__((x)+(y)+(double)0),double),d(x,y),l(x,y)))
#else /* __COMPILER_HAVE_LONGDOUBLE */
#define __FPFUNC(x,f,d,l) \
   __builtin_choose_expr(__builtin_types_compatible_p(__typeof__((x)+(float)0),float),f(x),d(x))
#define __FPFUNC2(x,y,f,d,l) \
   __builtin_choose_expr(__builtin_types_compatible_p(__typeof__((x)+(y)+(float)0),float),f(x,y),d(x,y))
#endif /* !__COMPILER_HAVE_LONGDOUBLE */
#elif defined(__COMPILER_HAVE_LONGDOUBLE)
#define __FPFUNC(x,f,d,l)    (sizeof((x)+(float)0) == sizeof(float) ? f((float)(x)) : \
                              sizeof((x)+(double)0) == sizeof(double) ? d((double)(x)) : l((long double)(x)))
#define __FPFUNC2(x,y,f,d,l) (sizeof((x)+(y)+(float)0) == sizeof(float) ? f((float)(x),(float)(y)) : \
                              sizeof((x)+(y)+(double)0) == sizeof(double) ? d((double)(x),(double)(y)) : l((long double)(x),(long double)(y)))
#else
#define __FPFUNC(x,f,d,l)    (sizeof((x)+(float)0) == sizeof(float) ? f((float)(x)) : d((double)(x)))
#define __FPFUNC2(x,y,f,d,l) (sizeof((x)+(y)+(float)0) == sizeof(float) ? f((float)(x),(float)(y)) : d((double)(x),(double)(y)))
#endif


#ifndef __OPTIMIZE_SIZE__
#if __GCC_VERSION(4,0,0) || \
   (__has_builtin(__builtin_signbitf) && __has_builtin(__builtin_signbit) && \
    __has_builtin(__builtin_signbitl))
#define signbit(x) __FPFUNC(x,__builtin_signbitf,__builtin_signbit,__builtin_signbitl)
#endif
#if __GCC_VERSION(4,4,0) && !defined(__SUPPORT_SNAN__)
#define fpclassify(x) __builtin_fpclassify(FP_NAN,FP_INFINITE,FP_NORMAL,FP_SUBNORMAL,FP_ZERO,x)
#define isfinite(x)   __builtin_isfinite(x)
#define isnormal(x)   __builtin_isnormal(x)
#define isnan(x)      __builtin_isnan(x)
#define isinf(x)      __builtin_isinf_sign(x)
#endif
#endif /* !__OPTIMIZE_SIZE__ */

#ifndef fpclassify
#define fpclassify(x) __FPFUNC(x,__fpclassifyf,__fpclassify,__fpclassifyl)
#endif
#ifndef signbit
#define signbit(x)    __FPFUNC(x,__signbitf,__signbit,__signbitl)
#endif
#ifndef isnormal
#define isnormal(x)  (fpclassify(x) == FP_NORMAL)
#endif
#ifndef isfinite
#ifdef __DOS_COMPAT__
#define isfinite(x)  (fpclassify(x) <= 0)
#else /* __DOS_COMPAT__ */
#define isfinite(x)   __FPFUNC(x,__finitef,__finite,__finitel)
#endif /* !__DOS_COMPAT__ */
#endif /* !isfinite */
#ifndef isnan
#ifdef __DOS_COMPAT__
#define isnan(x)      __FPFUNC(x,__isnanf,__isnan,__isnanl)
#else /* __DOS_COMPAT__ */
#define isnan(x)      __FPFUNC(x,__isnanf,__isnan,__isnanl)
#endif /* !__DOS_COMPAT__ */
#endif /* !isnan */
#ifndef isinf
#ifdef __DOS_COMPAT__
#define isinf(x)     (__FPFUNC(x,__fpclassifyf,__fpclassify,__fpclassifyl) == FP_INFINITE)
#else /* __DOS_COMPAT__ */
#define isinf(x)      __FPFUNC(x,__isinff,__isinf,__isinfl)
#endif /* !__DOS_COMPAT__ */
#endif /* !isinf */

#ifdef __USE_ISOC99
#if __GCC_VERSION(2,97,0) && (!defined(__DOS_COMPAT__) || !defined(__OPTIMIZE_SIZE__))
#define isunordered(u,v)    __builtin_isunordered(u,v)
#define isgreater(x,y)      __builtin_isgreater(x,y)
#define isgreaterequal(x,y) __builtin_isgreaterequal(x,y)
#define isless(x,y)         __builtin_isless(x,y)
#define islessequal(x,y)    __builtin_islessequal(x,y)
#define islessgreater(x,y)  __builtin_islessgreater(x,y)
#elif defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__ATTR_CONST,int,__LIBCCALL,__dos_pcompf,(float __x, float __y),_fdpcomp,(__x,__y))
__REDIRECT(__LIBC,__ATTR_CONST,int,__LIBCCALL,__dos_pcomp,(double __x, double __y),_dpcomp,(__x,__y))
#ifdef __COMPILER_HAVE_LONGDOUBLE
__REDIRECT(__LIBC,__ATTR_CONST,int,__LIBCCALL,__dos_pcompl,(long double __x, long double __y),_ldpcomp,(__x,__y))
#endif /* __COMPILER_HAVE_LONGDOUBLE */
#define __DOS_FPCOMPARE(x,y) __FPFUNC(x,__dos_pcompf,__dos_pcomp,__dos_pcompl)
#define isgreater(x,y)      ((__DOS_FPCOMPARE(x,y)&4) != 0)
#define isgreaterequal(x,y) ((__DOS_FPCOMPARE(x,y)&6) != 0)
#define isless(x,y)         ((__DOS_FPCOMPARE(x,y)&1) != 0)
#define islessequal(x,y)    ((__DOS_FPCOMPARE(x,y)&3) != 0)
#define islessgreater(x,y)  ((__DOS_FPCOMPARE(x,y)&5) != 0)
#define isunordered(x,y)     (__DOS_FPCOMPARE(x,y) == 0)
#endif


/* Generic... */
#ifndef isunordered
#define isunordered(u,v)   (fpclassify(u) == FP_NAN || fpclassify(v) == FP_NAN)
#endif
#ifndef isgreater
#define isgreater(x,y)      __XBLOCK({ __typeof__(x) __x = (x); __typeof__(y) __y = (y); __XRETURN !isunordered(__x,__y) && __x > __y; })
#endif
#ifndef isgreaterequal
#define isgreaterequal(x,y) __XBLOCK({ __typeof__(x) __x = (x); __typeof__(y) __y = (y); __XRETURN !isunordered(__x,__y) && __x >= __y; })
#endif
#ifndef isless
#define isless(x,y)         __XBLOCK({ __typeof__(x) __x = (x); __typeof__(y) __y = (y); __XRETURN !isunordered(__x,__y) && __x < __y; })
#endif
#ifndef islessequal
#define islessequal(x,y)    __XBLOCK({ __typeof__(x) __x = (x); __typeof__(y) __y = (y); __XRETURN !isunordered(__x,__y) && __x <= __y; })
#endif
#ifndef islessgreater
#define islessgreater(x,y)  __XBLOCK({ __typeof__(x) __x = (x); __typeof__(y) __y = (y); __XRETURN !isunordered(__x,__y) &&(__x < __y || __y < __x); })
#endif
#endif /* __USE_ISOC99 */




/* Bitmasks for the math_errhandling macro. */
#define MATH_ERRNO     1 /* errno set by math functions. */
#define MATH_ERREXCEPT 2 /* Exceptions raised by math functions. */

/* By default all functions support both errno and exception handling.
 * In gcc's fast math mode and if inline functions are defined this might not be true. */
#ifndef __FAST_MATH__
#define math_errhandling    (MATH_ERRNO|MATH_ERREXCEPT)
#endif
#endif /* __USE_ISOC99 */
#ifdef __USE_GNU
#ifndef __COMPILER_HAVE_LONGDOUBLE
#   define issignaling(x) (sizeof(x) == sizeof(float) ? __issignalingf(x) : __issignaling(x))
#else
#   define issignaling(x) (sizeof(x) == sizeof(float)  ? __issignalingf(x) : \
                           sizeof(x) == sizeof(double) ? __issignaling(x) : __issignalingl(x))
#endif
#endif /* __USE_GNU */

#ifdef __USE_MISC
/* Support for various different standard error handling behaviors. */
typedef enum {
  _IEEE_  = -1, /* According to IEEE 754/IEEE 854. */
  _SVID_  = 0,  /* According to System V, release 4. */
  _XOPEN_ = 1,  /* Nowadays also Unix98. */
  _POSIX_ = 2,
  _ISOC_  = 3   /* Actually this is ISO C99. */
} _LIB_VERSION_TYPE;

/* This variable can be changed at run-time to any of the values above to
 * affect floating point error handling behavior (it may also be necessary
 * to change the hardware FPU exception settings). */
#ifndef __KERNEL__
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("_LIB_VERSION")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef _LIB_VERSION
__LIBC _LIB_VERSION_TYPE _LIB_VERSION;
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("_LIB_VERSION")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#endif /* !__KERNEL__ */
#endif /* __USE_MISC */


#ifdef __USE_MISC

/* In SVID error handling, `matherr' is called with this description of the exceptional condition.
 * We have a problem when using C++ since `exception' is a reserved name in C++. */
#ifdef __cplusplus
struct __exception
#else
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("exception")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef exception
struct exception
#endif
{
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("type")
#pragma push_macro("name")
#pragma push_macro("arg1")
#pragma push_macro("arg2")
#pragma push_macro("retval")
#pragma push_macro("err")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef type
#undef name
#undef arg1
#undef arg2
#undef retval
#undef err
    int    type;
    char  *name;
    double arg1;
    double arg2;
    double retval;
    int    err;
#ifdef _MSC_VER
    int    __pad;
#endif
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("type")
#pragma pop_macro("name")
#pragma pop_macro("arg1")
#pragma pop_macro("arg2")
#pragma pop_macro("retval")
#pragma pop_macro("err")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
};

#ifndef __KERNEL__
#ifdef __cplusplus
__LIBC int __NOTHROW((__LIBCCALL matherr)(struct __exception *__exc));
#else
__LIBC int __NOTHROW((__LIBCCALL matherr)(struct exception *__exc));
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("exception")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#endif
#endif /* !__KERNEL__ */

#define X_TLOSS    1.41484755040568800000e+16

/* Types of exceptions in the `type' field. */
#define DOMAIN     1
#define SING       2
#define OVERFLOW   3
#define UNDERFLOW  4
#define TLOSS      5
#define PLOSS      6

/* SVID mode specifies returning this large value instead of infinity. */
#define HUGE       3.40282347e+38F

#else /* __USE_MISC */
#ifdef __USE_XOPEN
/* X/Open wants another strange constant. */
#define MAXFLOAT   3.40282347e+38F
#endif
#endif /* !__USE_MISC */

/* Some useful constants. */
#if defined(__USE_MISC) || defined(__USE_XOPEN)
#   define M_E        2.7182818284590452354  /* e */
#   define M_LOG2E    1.4426950408889634074  /* log_2 e */
#   define M_LOG10E   0.43429448190325182765 /* log_10 e */
#   define M_LN2      0.69314718055994530942 /* log_e 2 */
#   define M_LN10     2.30258509299404568402 /* log_e 10 */
#   define M_PI       3.14159265358979323846 /* pi */
#   define M_PI_2     1.57079632679489661923 /* pi/2 */
#   define M_PI_4     0.78539816339744830962 /* pi/4 */
#   define M_1_PI     0.31830988618379067154 /* 1/pi */
#   define M_2_PI     0.63661977236758134308 /* 2/pi */
#   define M_2_SQRTPI 1.12837916709551257390 /* 2/sqrt(pi) */
#   define M_SQRT2    1.41421356237309504880 /* sqrt(2) */
#   define M_SQRT1_2  0.70710678118654752440 /* 1/sqrt(2) */
#endif

/* The above constants are not adequate for computation using `long double's.
 * Therefore we provide as an extension constants with similar names as a
 * GNU extension.  Provide enough digits for the 128-bit IEEE quad. */
#ifdef __USE_GNU
#   define M_El        2.718281828459045235360287471352662498L /* e */
#   define M_LOG2El    1.442695040888963407359924681001892137L /* log_2 e */
#   define M_LOG10El   0.434294481903251827651128918916605082L /* log_10 e */
#   define M_LN2l      0.693147180559945309417232121458176568L /* log_e 2 */
#   define M_LN10l     2.302585092994045684017991454684364208L /* log_e 10 */
#   define M_PIl       3.141592653589793238462643383279502884L /* pi */
#   define M_PI_2l     1.570796326794896619231321691639751442L /* pi/2 */
#   define M_PI_4l     0.785398163397448309615660845819875721L /* pi/4 */
#   define M_1_PIl     0.318309886183790671537767526745028724L /* 1/pi */
#   define M_2_PIl     0.636619772367581343075535053490057448L /* 2/pi */
#   define M_2_SQRTPIl 1.128379167095512573896158903121545172L /* 2/sqrt(pi) */
#   define M_SQRT2l    1.414213562373095048801688724209698079L /* sqrt(2) */
#   define M_SQRT1_2l  0.707106781186547524400844362104849039L /* 1/sqrt(2) */
#endif

#if defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ > 0
//#include <bits/math-finite.h>
#endif

__SYSDECL_END

#endif /* _MATH_H */
