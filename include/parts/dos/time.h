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
#ifndef _PARTS_DOS_TIME_H
#define _PARTS_DOS_TIME_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <xlocale.h>
#ifndef _TIME_H
#include <time.h>
#endif

#ifndef __KERNEL__
__SYSDECL_BEGIN

#ifdef __USE_KOS
#define __FIXED_CONST const
#else
#define __FIXED_CONST /* Nothing */
#endif

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

__REDIRECT_DPB(__LIBC,,size_t,__LIBCCALL,strftime_l,
              (char *__restrict __buf, size_t __bufsize, char const *__restrict __format,
               struct tm const *__restrict __tp, __locale_t __locale),
              (__buf,__bufsize,__format,__tp,__locale))
__REDIRECT_DPB_VOID(__LIBC,,__LIBCCALL,tzset,(void),())

#ifdef __DOS_COMPAT__
__LIBC __WUNUSED char *(__LIBCCALL _ctime32)(__time32_t const *__restrict __timer);
__LIBC __WUNUSED double (__LIBCCALL _difftime32)(__time32_t __time1, __time32_t __time0);
__LIBC __WUNUSED struct tm *(__LIBCCALL _gmtime32)(__time32_t const *__restrict __timer);
__LIBC __WUNUSED struct tm *(__LIBCCALL _localtime32)(__time32_t const *__restrict __timer);
__LIBC __time32_t (__LIBCCALL _time32)(__time32_t *__timer);
__LIBC __WUNUSED __time32_t (__LIBCCALL _mktime32)(struct tm __FIXED_CONST *__restrict __tp);
__LIBC __WUNUSED __time32_t (__LIBCCALL _mkgmtime32)(struct tm __FIXED_CONST *__restrict __tp);
#else
__REDIRECT(__LIBC,__WUNUSED,char *,__LIBCCALL,_ctime32,(__time32_t const *__restrict __timer),ctime,(__timer))
__REDIRECT(__LIBC,__WUNUSED,double,__LIBCCALL,_difftime32,(__time32_t __time1, __time32_t __time0),difftime,(__time1,__time0))
__REDIRECT(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,_gmtime32,(__time32_t const *__restrict __timer),gmtime,(__timer))
__REDIRECT(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,_localtime32,(__time32_t const *__restrict __timer),localtime,(__timer))
__REDIRECT(__LIBC,,__time32_t,__LIBCCALL,_time32,(__time32_t *__timer),time,(__timer))
__REDIRECT(__LIBC,__WUNUSED,__time32_t,__LIBCCALL,_mktime32,(struct tm __FIXED_CONST *__restrict __tp),mktime,(__tp))
__REDIRECT(__LIBC,__WUNUSED,__time32_t,__LIBCCALL,_mkgmtime32,(struct tm __FIXED_CONST *__restrict __tp),timegm,(__tp))
#endif

#ifdef __GLC_COMPAT__
__LOCAL __WUNUSED double (__LIBCCALL _difftime64)(__time64_t __time1, __time64_t __time0) { return _difftime32((__time32_t)__time1,(__time32_t)__time0); }
__LOCAL __WUNUSED char *(__LIBCCALL _ctime64)(__time64_t const *__restrict __timer) { __time32_t __tm = (__time32_t)*__timer; return _ctime32(&__tm); }
__LOCAL __WUNUSED struct tm *(__LIBCCALL _gmtime64)(__time64_t const *__restrict __timer) { __time32_t __tm = (__time32_t)*__timer; return _gmtime32(&__tm); }
__LOCAL __WUNUSED struct tm *(__LIBCCALL _localtime64)(__time64_t const *__restrict __timer) { __time32_t __tm = (__time32_t)*__timer; return _localtime32(&__tm); }
__LOCAL __WUNUSED __time64_t (__LIBCCALL _mktime64)(struct tm __FIXED_CONST *__restrict __tp) { return (__time64_t)_mktime32(__tp); }
__LOCAL __WUNUSED __time64_t (__LIBCCALL _mkgmtime64)(struct tm __FIXED_CONST *__restrict __tp) { return (__time64_t)_mkgmtime32(__tp); }
__LOCAL __time64_t (__LIBCCALL _time64)(__time64_t *__timer) { __time32_t __res = _time32(NULL); if (__timer) *__timer = __res; return __res; }
#else /* __GLC_COMPAT__ */
__REDIRECT_DPB(__LIBC,__WUNUSED,double,__LIBCCALL,difftime64,(__time64_t __time1, __time64_t __time0),(__time1,__time0))
__REDIRECT_DPB(__LIBC,__WUNUSED,char *,__LIBCCALL,ctime64,(__time64_t const *__restrict __timer),(__timer))
__REDIRECT_DPB(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,gmtime64,(__time64_t const *__restrict __timer),(__timer))
__REDIRECT_DPB(__LIBC,__WUNUSED,struct tm *,__LIBCCALL,localtime64,(__time64_t const *__restrict __timer),(__timer))
__REDIRECT_DPB(__LIBC,__WUNUSED,__time64_t,__LIBCCALL,mktime64,(struct tm __FIXED_CONST *__restrict __tp),(__tp))
__REDIRECT_DPB(__LIBC,__WUNUSED,__time64_t,__LIBCCALL,mkgmtime64,(struct tm __FIXED_CONST *__restrict __tp),(__tp))
__REDIRECT_DPB(__LIBC,,__time64_t,__LIBCCALL,time64,(__time64_t *__timer),(__timer))
#endif /* !__GLC_COMPAT__ */

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY __UINT32_TYPE__ (__LIBCCALL _getsystime)(struct tm *__restrict __tp);
__LIBC __PORT_DOSONLY __UINT32_TYPE__ (__LIBCCALL _setsystime)(struct tm __FIXED_CONST *__restrict __tp, __UINT32_TYPE__ __msec);
__LIBC __PORT_DOSONLY char *(__LIBCCALL _strdate)(char __buf[9]);
__LIBC __PORT_DOSONLY char *(__LIBCCALL _strtime)(char __buf[9]);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _strdate_s)(char __buf[9], size_t __bufsize);
__LIBC __PORT_DOSONLY errno_t (__LIBCCALL _strtime_s)(char __buf[9], size_t __bufsize);
#endif /* __CRT_DOS */

#ifdef __GLC_COMPAT__
#ifndef ____ctime32_r_defined
#define ____ctime32_r_defined 1
__REDIRECT(__LIBC,,struct tm *,__LIBCCALL,__gmtime32_r,(__time32_t const *__restrict __timer, struct tm *__restrict __tp),gmtime_r,(__timer,__tp))
__REDIRECT(__LIBC,,struct tm *,__LIBCCALL,__localtime32_r,(__time32_t const *__restrict __timer, struct tm *__restrict __tp),localtime_r,(__timer,__tp))
__REDIRECT(__LIBC,,char *,__LIBCCALL,__ctime32_r,(__time32_t const *__restrict __timer, char *__restrict __buf),ctime_r,(__timer,__buf))
#endif /* !____ctime32_r_defined */
__LOCAL errno_t (__LIBCCALL _ctime32_s)(char __buf[26], size_t __bufsize, __time32_t const *__restrict __timer) { return __bufsize >= 26 && __ctime32_r(__timer,__buf) ? 0 : 1; }
__LOCAL errno_t (__LIBCCALL _ctime64_s)(char __buf[26], size_t __bufsize, __time64_t const *__restrict __timer) { __time32_t __tm = (__time32_t)*__timer; return _ctime32_s(__buf,__bufsize,&__tm); }
__LOCAL errno_t (__LIBCCALL _gmtime32_s)(struct tm *__restrict __tp, __time32_t const *__restrict __timer) { return __gmtime32_r(__timer,__tp) ? 0 : 1; }
__LOCAL errno_t (__LIBCCALL _localtime32_s)(struct tm *__restrict __tp, __time32_t const *__restrict __timer) { return __localtime32_r(__timer,__tp) ? 0 : 1; }
__LOCAL errno_t (__LIBCCALL _gmtime64_s)(struct tm *__restrict __tp, __time64_t const *__restrict __timer) { __time32_t __tm = (__time32_t)*__timer; return _gmtime32_s(__tp,&__tm); }
__LOCAL errno_t (__LIBCCALL _localtime64_s)(struct tm *__restrict __tp, __time64_t const *__restrict __timer) { __time32_t __tm = (__time32_t)*__timer; return _localtime32_s(__tp,&__tm); }
#ifdef __USE_DOS_SLIB
__REDIRECT(__LIBC,,char *,__LIBCCALL,__asctime_r,(struct tm const *__restrict __tp, char *__restrict __buf),asctime_r,(__tp,__buf))
__LOCAL errno_t (__LIBCCALL asctime_s)(char __buf[26], size_t __bufsize, struct tm const *__restrict __tp) { return __bufsize >= 26 && __asctime_r(__tp,__buf) ? 0 : 1; }
#endif /* __USE_DOS_SLIB */
#else /* __GLC_COMPAT__ */
/* WARNING: The following functions always return DOS error codes! */
__LIBC errno_t (__LIBCCALL _ctime32_s)(char __buf[26], size_t __bufsize, __time32_t const *__restrict __timer);
__LIBC errno_t (__LIBCCALL _gmtime32_s)(struct tm *__restrict __tp, __time32_t const *__restrict __timer);
__LIBC errno_t (__LIBCCALL _localtime32_s)(struct tm *__restrict __tp, __time32_t const *__restrict __timer);
__LIBC errno_t (__LIBCCALL _ctime64_s)(char __buf[26], size_t __bufsize, __time64_t const *__restrict __timer);
__LIBC errno_t (__LIBCCALL _gmtime64_s)(struct tm *__restrict __tp, __time64_t const *__restrict __timer);
__LIBC errno_t (__LIBCCALL _localtime64_s)(struct tm *__restrict __tp, __time64_t const *__restrict __timer);
#ifdef __USE_DOS_SLIB
__LIBC errno_t (__LIBCCALL asctime_s)(char __buf[26], size_t __bufsize, struct tm const *__restrict __tp);
#endif /* __USE_DOS_SLIB */
#endif /* !__GLC_COMPAT__ */

#undef __FIXED_CONST

__SYSDECL_END

#include "wtime.h"

#endif /* !__KERNEL__ */

#endif /* !_PARTS_DOS_TIME_H */
