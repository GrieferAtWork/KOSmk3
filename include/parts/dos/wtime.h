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
#ifndef _PARTS_DOS_WTIME_H
#define _PARTS_DOS_WTIME_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <xlocale.h>

#ifndef __KERNEL__
__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

__NAMESPACE_STD_BEGIN
struct tm;
__NAMESPACE_STD_END
#ifndef __tm_defined
#define __tm_defined 1
__NAMESPACE_STD_USING(tm)
#endif /* !__tm_defined */

#ifndef _WTIME_DEFINED
#define _WTIME_DEFINED 1
__REDIRECT_DPB(__LIBC,,size_t,__LIBCCALL,wcsftime_l,(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, struct tm const *__restrict __ptm, __locale_t __locale),(__buf,__maxlen,__format,__ptm,__locale))
#ifdef __CRT_DOS
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wasctime_s,(wchar_t __buf[26], size_t __maxlen, struct tm const *__restrict __ptm),(__buf,__maxlen,__ptm))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wstrdate_s,(wchar_t __buf[9], size_t __maxlen),(__buf,__maxlen))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wstrtime_s,(wchar_t __buf[9], size_t __maxlen),(__buf,__maxlen))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,wasctime,(struct tm const *__restrict __ptm),(__ptm))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,wstrdate,(wchar_t *__restrict __buf),(__buf))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,wstrtime,(wchar_t *__restrict __buf),(__buf))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,wctime32,(__time32_t const *__restrict __timer),(__timer))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,wchar_t *,__LIBCCALL,wctime64,(__time64_t const *__restrict __timer),(__timer))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wctime32_s,(wchar_t __buf[26], size_t __maxlen, __time32_t const *__timer),(__buf,__maxlen,__timer))
__REDIRECT_DPB(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,wctime64_s,(wchar_t __buf[26], size_t __maxlen, __time64_t const *__timer),(__buf,__maxlen,__timer))
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBC,__WUNUSED __PORT_DOSONLY,wchar_t *,__LIBCCALL,_wctime,(__TM_TYPE(time) const *__restrict __timer),_wctime64,(__timer))
__REDIRECT(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wctime_s,(wchar_t *__restrict __buf, size_t __maxlen, __TM_TYPE(time) const *__restrict __timer),_wctime64_s,(__buf,__maxlen,__timer))
#else /* __USE_TIME_BITS64 */
__REDIRECT(__LIBC,__WUNUSED __PORT_DOSONLY,wchar_t *,__LIBCCALL,_wctime,(__TM_TYPE(time) const *__restrict __timer),_wctime32,(__timer))
__REDIRECT(__LIBC,__PORT_DOSONLY,errno_t,__LIBCCALL,_wctime_s,(wchar_t *__restrict __buf, size_t __maxlen, __TM_TYPE(time) const *__restrict __timer),_wctime32_s,(__buf,__maxlen,__timer))
#endif /* !__USE_TIME_BITS64 */
#endif /* __CRT_DOS */

#ifndef __std_wcsftime_defined
#define __std_wcsftime_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __NONNULL((1,3,4)) size_t (__LIBCCALL wcsftime)(wchar_t *__restrict __buf, size_t __maxlen, wchar_t const *__restrict __format, struct tm const *__restrict __ptm);
__NAMESPACE_STD_END
#endif /* !__std_wcsftime_defined */
#ifndef __wcsftime_defined
#define __wcsftime_defined 1
__NAMESPACE_STD_USING(wcsftime)
#endif /* !__wcsftime_defined */
#endif /* !_WTIME_DEFINED */


__SYSDECL_END
#endif /* !__KERNEL__ */

#endif /* !_PARTS_DOS_WTIME_H */
