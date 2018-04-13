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
#ifndef _UTIME_H
#define _UTIME_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>

#ifdef __CC__
__SYSDECL_BEGIN

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("actime")
#pragma push_macro("modtime")
#endif

#undef actime
#undef modtime
#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K) || \
    defined(__USE_DOS)
#ifndef __time_t_defined
#define __time_t_defined 1
typedef __TM_TYPE(time) time_t;
#endif /* !__time_t_defined */
#endif /* __USE_XOPEN || __USE_XOPEN2K */

struct utimbuf {
    __TM_TYPE(time) actime;  /* Access time. */
    __TM_TYPE(time) modtime; /* Modification time. */
};

#ifdef __USE_KOS
#ifdef __USE_TIME_BITS64
struct utimbuf32 {
    __time32_t      actime;  /* Access time. */
    __time32_t      modtime; /* Modification time. */
};
#else
#define utimbuf32 utimbuf
#endif
#endif

#if defined(__USE_TIME64) || defined(__USE_KOS)
struct utimbuf64 {
    __time64_t      actime;  /* Access time. */
    __time64_t      modtime; /* Modification time. */
};
#endif /* __USE_TIME64 || __USE_KOS */

#ifdef __USE_DOS
#ifndef _UTIMBUF_DEFINED
#define _UTIMBUF_DEFINED 1
struct _utimbuf {
    __TM_TYPE(time) actime;  /* Access time. */
    __TM_TYPE(time) modtime; /* Modification time. */
};
struct __utimbuf32 {
    __time32_t      actime;  /* Access time. */
    __time32_t      modtime; /* Modification time. */
};

struct __utimbuf64 {
    __time64_t      actime;  /* Access time. */
    __time64_t      modtime; /* Modification time. */
};
#ifndef __USE_KOS
struct utimbuf32 {
    __time32_t      actime;  /* Access time. */
    __time32_t      modtime; /* Modification time. */
};
#endif /* !__USE_KOS */
#endif /* !_UTIMBUF_DEFINED */
#endif /* __USE_DOS */

/* Used assembly names (Required for binary compatibility):
 * FMT       TIME  KOS         DOS(HOSTED)      DOS        GLC
 * char      32    utime       DOS$_utime32    _utime32   utime
 * char      64    utime64     DOS$_utime64    _utime64   ---
 * char32_t  32    wutime      DOS$U_wutime32  ---        ---
 * char32_t  64    wutime64    DOS$U_wutime64  ---        ---
 * char16_t  32    uwutime     DOS$_wutime32   _wutime32  ---
 * char16_t  64    uwutime64   DOS$_wutime64   _wutime64  ---
 * fd_t fd    32    futime      _futime32       _futime32  ---
 * fd_t fd    64    futime64    _futime64       _futime64  ---
 */

#ifndef __KERNEL__
#ifdef __USE_DOSFS
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,utime,(char const *__file, struct utimbuf const *__file_times),_utime64,(__file,__file_times))
#else /* __USE_TIME_BITS64 */
__REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,utime,(char const *__file, struct utimbuf const *__file_times),_utime32,(__file,__file_times))
#endif /* !__USE_TIME_BITS64 */
#ifdef __USE_TIME64
__REDIRECT_UFS_(__LIBC,__NONNULL((1)),int,__LIBCCALL,utime64,(char const *__file, struct utimbuf64 const *__file_times),_utime64,(__file,__file_times))
#endif /* __USE_TIME64 */
#elif defined(__GLC_COMPAT__)
#ifdef __USE_TIME_BITS64
struct __glc_utimbuf32 { __time32_t actime,modtime; };
__REDIRECT(__LIBC,__NONNULL((1)),int,__LIBCCALL,__utime32,(char const *__file, struct __glc_utimbuf32 const *__file_times),utime,(__file,__file_times))
__LOCAL __NONNULL((1)) int (__LIBCCALL utime)(char const *__file, struct utimbuf const *__file_times) {
 struct __glc_utimbuf32 __buf32;
 if (__file_times) __buf32.actime = (__time32_t)__file_times->actime,
                   __buf32.modtime = (__time32_t)__file_times->modtime;
 return __utime32(__file,__file_times ? &__buf32 : 0);
}
#else
__LIBC __NONNULL((1)) int (__LIBCCALL utime)(char const *__file, struct utimbuf const *__file_times);
#endif
#ifdef __USE_TIME64
__LOCAL __NONNULL((1)) int (__LIBCCALL utime64)(char const *__file, struct utimbuf64 const *__file_times) {
#ifdef __USE_TIME_BITS64
 return utime(__file,__file_times);
#else
 struct utimbuf __buf32;
 if (__file_times) __buf32.actime = (__time32_t)__file_times->actime,
                   __buf32.modtime = (__time32_t)__file_times->modtime;
 return utime(__file,__file_times ? &__buf32 : 0);
#endif
}
#endif /* __USE_TIME64 */
#else /* __USE_DOSFS */
__REDIRECT_UFSTM64(__LIBC,__NONNULL((1)),int,__LIBCCALL,utime,(char const *__file, struct utimbuf const *__file_times),(__file,__file_times))
#ifdef __USE_TIME64
__REDIRECT_UFS(__LIBC,__NONNULL((1)),int,__LIBCCALL,utime64,(char const *__file, struct utimbuf64 const *__file_times),(__file,__file_times))
#endif /* __USE_TIME64 */
#endif /* !__USE_DOSFS */

#if defined(__USE_KOS) && defined(__CRT_DOS)
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBC,__PORT_DOSONLY_ALT(utime),int,__LIBCCALL,futime,(__fd_t __fd, struct utimbuf const *__file_times),_futime64,(__fd,__file_times))
#else /* __USE_TIME_BITS64 */
__REDIRECT(__LIBC,__PORT_DOSONLY_ALT(utime),int,__LIBCCALL,futime,(__fd_t __fd, struct utimbuf const *__file_times),_futime32,(__fd,__file_times))
#endif /* !__USE_TIME_BITS64 */
#ifdef __USE_TIME64
__REDIRECT(__LIBC,__PORT_DOSONLY_ALT(utime),int,__LIBCCALL,futime64,(__fd_t __fd, struct utimbuf64 const *__file_times),_futime64,(__fd,__file_times))
#endif /* __USE_TIME64 */
#endif /* __USE_KOS && __CRT_DOS */

#ifdef __USE_DOS
#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifdef __USE_KOS
/* Fix the const-ness of arguments as a KOS extension. (*sigh*) */
#define __FIXED_CONST const
#else
#define __FIXED_CONST
#endif

#ifdef __CRT_DOS
__LOCAL int (__LIBCCALL _utime)(char const *__file, struct _utimbuf __FIXED_CONST *__file_times) { return utime(__file,(struct utimbuf *)__file_times); }
__LIBC __PORT_DOSONLY_ALT(utime) int (__LIBCCALL _futime32)(__fd_t __fd, struct __utimbuf32 __FIXED_CONST *__file_times);
__LIBC __PORT_DOSONLY_ALT(utime64) int (__LIBCCALL _futime64)(__fd_t __fd, struct __utimbuf64 __FIXED_CONST *__file_times);
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,wutime32,(wchar_t const *__file, struct __utimbuf32 __FIXED_CONST *__file_times),(__file,__file_times))
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,wutime64,(wchar_t const *__file, struct __utimbuf64 __FIXED_CONST *__file_times),(__file,__file_times))
#ifdef __DOC_COMPAT__
__REDIRECT_UFSDPB(__LIBC,,int,__LIBCCALL,utime32,(char const *__file, struct __utimbuf32 __FIXED_CONST *__file_times),(__file,__file_times))
__REDIRECT_UFSDPB(__LIBC,,int,__LIBCCALL,utime64,(char const *__file, struct __utimbuf64 __FIXED_CONST *__file_times),(__file,__file_times))
#else /* __USE_DOSFS */
__REDIRECT_UFS_(__LIBC,,int,__LIBCCALL,_utime32,(char const *__file, struct __utimbuf32 __FIXED_CONST *__file_times),utime,(__file,__file_times))
__REDIRECT_UFS_(__LIBC,,int,__LIBCCALL,_utime64,(char const *__file, struct __utimbuf64 __FIXED_CONST *__file_times),utime64,(__file,__file_times))
#endif /* !__USE_DOSFS */
#ifdef __USE_TIME_BITS64
__REDIRECT_UFS_(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wutime,(wchar_t const *__file, struct _utimbuf __FIXED_CONST *__file_times),_wutime64,(__file,__file_times))
__REDIRECT(__LIBC,__PORT_DOSONLY_ALT(utime),int,__LIBCCALL,_futime,(__fd_t __fd, struct _utimbuf __FIXED_CONST *__file_times),_futime64,(__fd,__file_times))
#else /* __USE_TIME_BITS64 */
__REDIRECT_UFS_(__LIBC,__PORT_DOSONLY,int,__LIBCCALL,_wutime,(wchar_t const *__file, struct _utimbuf __FIXED_CONST *__file_times),_wutime32,(__file,__file_times))
__REDIRECT(__LIBC,__PORT_DOSONLY_ALT(utime),int,__LIBCCALL,_futime,(__fd_t __fd, struct _utimbuf __FIXED_CONST *__file_times),_futime32,(__fd,__file_times))
#endif /* !__USE_TIME_BITS64 */
#else /* __CRT_DOS */
__LOCAL int (__LIBCCALL _utime)(char const *__file, struct _utimbuf __FIXED_CONST *__file_times) { return utime(__file,(struct utimbuf *)__file_times); }
__REDIRECT(__LIBC,,int,__LIBCCALL,_utime32,(char const *__file, struct __utimbuf32 __FIXED_CONST *__file_times),utime,(__file,__file_times))
__LOCAL int (__LIBCCALL _utime64)(char const *__file, struct __utimbuf64 __FIXED_CONST *__file_times) {
 struct __utimbuf32 __buf32;
 if (__file_times) __buf32.actime = __file_times->actime,
                   __buf32.modtime = __file_times->modtime;
 return _utime32(__file,__file_times ? &__buf32 : 0);
}
#endif /* !__CRT_DOS */

#undef __FIXED_CONST
#endif /* __USE_DOS */

#endif /* !__KERNEL__ */

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("modtime")
#pragma pop_macro("actime")
#endif

__SYSDECL_END
#endif /* __CC__ */

#endif /* !_UTIME_H */
