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
#ifndef _FEATURES_H
#define _FEATURES_H 1

#include "__stdinc.h"

/* NOTE: Most of the below is taken glibc <features.h>.
 * The below copy copyright notice can be found in the original. */
/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
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

#undef __USE_ISOC11
#undef __USE_ISOC99
#undef __USE_ISOC95
#undef __USE_ISOCXX11
#undef __USE_ISOCXX14
#undef __USE_POSIX
#undef __USE_POSIX2
#undef __USE_POSIX199309
#undef __USE_POSIX199506
#undef __USE_XOPEN
#undef __USE_XOPEN_EXTENDED
#undef __USE_UNIX98
#undef __USE_XOPEN2K
#undef __USE_XOPEN2KXSI
#undef __USE_XOPEN2K8
#undef __USE_XOPEN2K8XSI
#undef __USE_LARGEFILE
#undef __USE_LARGEFILE64
#undef __USE_FILE_OFFSET64
#undef __USE_MISC
#undef __USE_ATFILE
#undef __USE_GNU
#undef __USE_REENTRANT
#undef __USE_FORTIFY_LEVEL
#undef __KERNEL_STRICT_NAMES

#undef __USE_KOS         /* `#ifdef _KOS_SOURCE'     Additions added & Changes made by KOS. */
#undef __USE_KOS3        /* `#ifdef _KOS_SOURCE && __KOS_VERSION__ >= 300' New KOS functions added in KOS Mk3. */
#undef __USE_KXS         /* `#if _KOS_SOURCE >= 2'   Minor extended functionality that is likely to collide with existing programs. */
#undef __USE_KOS_DEPRECATED /* `#ifdef _KOS_SOURCE'  Enable deprecated KOS functions. */
#undef __USE_UTF         /* `#ifdef _UTF_SOURCE'     Enable additional string functions that accept `char16_t' and `char32_t'. (Referred to as `c16'/`u' and `c32'/`U') */
#undef __USE_CYG         /* `#ifdef _CYG_SOURCE'     Functions usually only found in Cygwin. */
#undef __USE_DOS         /* `#ifdef _DOS_SOURCE'     Functions usually only found in DOS: `spawn', `strlwr', etc. */
#undef __USE_OLD_DOS     /* `#if _DOS_SOURCE >= 2'   Make some old, long deprecated DOS APIs (namely in `<dos.h>') available. */
#undef __USE_DOSFS       /* `#ifdef _DOSFS_SOURCE'   Link filesystem functions that follow DOS path resolution (case-insensitive, '\\' == '/'). - Only when option when linking against __CRT_KOS. (s.a.: `fsmode') */
#undef __USE_DOS_SLIB    /* `#if _DOS_SOURCE && __STDC_WANT_SECURE_LIB__' Enable prototypes for the so-called ~secure~ DOS library. (They're really just functions that perform additional checks on arguments and such...) */
#undef __USE_TIME64      /* `#ifdef _TIME64_SOURCE'  Provide 64-bit time functions (e.g.: `time64()'). - A real implementation of this either requires `__CRT_DOS' or `__CRT_KOS'. - `__CRT_GLC' prototypes are emulated and truncate/zero-extend time arguments. */
#undef __USE_TIME_BITS64 /* `#if _TIME_T_BITS == 64' Use a 64-bit interger for `time_t' and replace all functions taking it as argument with 64-bit variations. */
#undef __USE_DEBUG       /* `#ifdef _DEBUG_SOURCE'   Enable debug function prototypes, either as real debug functions (`_DEBUG_SOURCE' defined as non-zero and `__CRT_KOS' is present), or as stubs/aliases for non-debug version (`_DEBUG_SOURCE' defined as zero or `__CRT_KOS' isn't present) */
#undef __USE_DEBUG_HOOK  /* `#ifndef NDEBUG'         Redirect functions such as `malloc()' to their debug counterparts (Implies `_DEBUG_SOURCE=1'). */
#undef __USE_PORTABLE    /* `#ifdef _PORT_SOURCE'    Mark all non-portable functions as deprecated (So-long as they can't be emulated when linking against any supported LIBC in stand-alone mode). */
#undef __USE_EXCEPT      /* `#ifdef _EXCEPT_SOURCE'  Define exception-enabled versions of various C functions, prefixed with an uppercase `X'. */
#undef __USE_EXCEPT_API  /* `#ifdef _EXCEPT_API'     Enable exception support for various C functions (see detailed documentation below). */

#undef __USE_KOS_PRINTF  /* `#if __KOS_CRT || _KOS_PRINTF_SOURCE || _KOS_SOURCE >= 2'
                          *    Always use KOS's printf() function & extension, as provided through `format_printf()'.
                          *    When running in dos/glc compatibility mode, functions such as `printf()' normally link
                          *    against the platform's integrated libc function, meaning that KOS-specific extensions
                          *    such as `%q' are not available normally.
                          * >> Enabling this option forces such functions to pass through some
                          *    emulation of KOS's printf function, re-enabling its extensions. */

/* `#ifdef __DOS_COMPAT__' Even if CRT-GLC may be available, still emulate extended libc functions using functions also provided by DOS.
 *                         NOTE: Automatically defined when CRT-GLC isn't available, but CRT-DOS is. */
/* `#ifdef __GLC_COMPAT__' Same as `__DOS_COMPAT__' but for GLibc, rather than DOS. */
/* `#ifdef __CYG_COMPAT__' Same as `__DOS_COMPAT__' but for Cygwin, rather than DOS. */

#if !defined(__CRT_DOS) && !defined(__CRT_GLC) && \
    !defined(__CRT_KOS) && !defined(__CRT_CYG)
#ifdef __KOS__
#   define __CRT_KOS 1
#elif defined(__CYGWIN__) || defined(__CYGWIN32__)
#   define __CRT_CYG 1
#elif defined(__linux__) || defined(__linux) || defined(linux) || \
      defined(__unix__) || defined(__unix) || defined(unix)
#   define __CRT_GLC 1
#elif defined(__WINDOWS__) || defined(_MSC_VER) || \
      defined(_WIN16) || defined(WIN16) || defined(_WIN32) || defined(WIN32) || \
      defined(_WIN64) || defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
      defined(_WIN32_WCE) || defined(WIN32_WCE)
/* TinyC links against an older version of MSVCRT by default,
 * so we use the old names by default as well. */
#ifdef __TINYC__
#   define __CRT_DOS 1
#else
#   define __CRT_DOS 2
#endif
#else
#   define __CRT_KOS 1
#endif
#endif

#ifdef __CRT_KOS
#ifndef __arm__
#   define __CRT_DOS 1
#endif /* !__arm__ */
#   define __CRT_GLC 1
#endif

#ifndef __CRT_KOS
#if defined(__CRT_DOS) && !defined(__CRT_GLC)
#   undef __DOS_COMPAT__
#   define __DOS_COMPAT__ 1
#elif defined(__CRT_GLC) && !defined(__CRT_DOS)
#   undef __GLC_COMPAT__
#   define __GLC_COMPAT__ 1
#elif defined(__CRT_CYG)
#   undef __CYG_COMPAT__
#   define __CYG_COMPAT__ 1
#endif
#endif /* !__CRT_KOS */

/* Some DOS exports, such as stdin/stdout/stderr are exported in different ways,
 * some of which are the objects in question themself, while others are indirect
 * functions that simply return that same object.
 * When this option is enabled, KOS system headers attempt
 * to link against objects, rather than functions. */
#undef __USE_DOS_LINKOBJECTS

/* At some point, DOS decided to rename the assembly symbols for the findfirst/findnext/stat functions.
 * And even though KOS exports both the old names, as well as the new, DOS is only exporting
 * either at a time, meaning that during compilation one has to select which names should be used:
 *     OLD           NEW
 *     _findfirst    _findfirst32
 *     _findfirst64  _findfirst64|_findfirst64i32
 *     _findfirsti64 _findfirst32i64
 *     ... (All other triples follow the same overlap)
 * Now it should be obvious that the old function not affected by
 * this is `_findfirst64' (Which only gained an alias we don't ever use)
 * Yet the pure 32-bit and the 32-bit time/64-bit size versions are
 * impossible to link while maintaining binary compatibility.
 * That is where this switch comes into play.
 * When set, old names are used, and when not, new ones are.
 */
#undef __USE_DOS_LINKOLDFINDSTAT

#ifdef __CRT_DOS
#if defined(_LINK_OLD_FINDSTAT_SOURCE) || __CRT_DOS < 2
#define __USE_DOS_LINKOLDFINDSTAT 1
#ifdef _LINK_NEW_FINDSTAT_SOURCE
#undef __USE_DOS_LINKOLDFINDSTAT
#endif /* _LINK_NEW_FINDSTAT_SOURCE */
#endif
#endif /* __CRT_DOS */






#ifndef _LOOSE_KERNEL_NAMES
# define __KERNEL_STRICT_NAMES
#endif

#if (defined(_BSD_SOURCE) || defined(_SVID_SOURCE)) && \
    !defined(_DEFAULT_SOURCE)
# undef  _DEFAULT_SOURCE
# define _DEFAULT_SOURCE 1
#endif

#ifdef _GNU_SOURCE
# undef  _ISOC95_SOURCE
# define _ISOC95_SOURCE 1
# undef  _ISOC99_SOURCE
# define _ISOC99_SOURCE 1
# undef  _ISOC11_SOURCE
# define _ISOC11_SOURCE 1
# undef  _POSIX_SOURCE
# define _POSIX_SOURCE 1
# undef  _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
# undef  _XOPEN_SOURCE
# define _XOPEN_SOURCE 700
# undef  _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED 1
# undef  _LARGEFILE64_SOURCE
# define _LARGEFILE64_SOURCE 1
# undef  _DEFAULT_SOURCE
# define _DEFAULT_SOURCE 1
# undef  _ATFILE_SOURCE
# define _ATFILE_SOURCE 1
#endif

#if (defined(_DEFAULT_SOURCE) || \
   (!defined(__STRICT_ANSI__) && \
    !defined(_ISOC99_SOURCE) && \
    !defined(_POSIX_SOURCE) && \
    !defined(_POSIX_C_SOURCE) && \
    !defined(_XOPEN_SOURCE)))
# undef  _DEFAULT_SOURCE
# define _DEFAULT_SOURCE 1
#endif

#if (defined(_ISOC11_SOURCE) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L))
# define __USE_ISOC11 1
#endif
#if (defined(_ISOC99_SOURCE) || defined(_ISOC11_SOURCE) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L))
# define __USE_ISOC99 1
#endif
#if (defined(_ISOC99_SOURCE) || defined(_ISOC11_SOURCE) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199409L))
# define __USE_ISOC95 1
#endif
#if (defined(_ISOCXX11_SOURCE) || defined(_ISOCXX14_SOURCE)) || \
     defined(__GXX_EXPERIMENTAL_CXX0X__) || \
    (defined(__cplusplus) && __cplusplus >= 201103L)
# define __USE_ISOCXX11 1
#endif
#if defined(_ISOCXX14_SOURCE) || \
   (defined(__cplusplus) && __cplusplus >= 201103L)
#   define __USE_ISOCXX14 1
#endif
#if defined(__cplusplus) && \
  (!defined(__GNUC__) || __GCC_VERSION(4,4,0))
/* Enable proper C++ prototype declarations. */
# define __CORRECT_ISO_CPP_STRING_H_PROTO  1
# define __CORRECT_ISO_CPP_STRINGS_H_PROTO 1
# define __CORRECT_ISO_CPP_WCHAR_H_PROTO   1
# define __CORRECT_ISO_CPP_STDLIB_H_PROTO  1
#endif

#ifdef _DEFAULT_SOURCE
# if !defined(_POSIX_SOURCE) && \
     !defined(_POSIX_C_SOURCE)
#  define __USE_POSIX_IMPLICITLY 1
# endif
# undef  _POSIX_SOURCE
# define _POSIX_SOURCE 1
# undef  _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif
#if (!defined(__STRICT_ANSI__) || \
     (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE+0 >= 500)) && \
     !defined(_POSIX_SOURCE) && !defined(_POSIX_C_SOURCE)
# define _POSIX_SOURCE 1
# if defined(_XOPEN_SOURCE) && _XOPEN_SOURCE+0 < 500
#  define _POSIX_C_SOURCE 2
# elif defined(_XOPEN_SOURCE) && _XOPEN_SOURCE+0 < 600
#  define _POSIX_C_SOURCE 199506L
# elif defined(_XOPEN_SOURCE) && _XOPEN_SOURCE+0 < 700
#  define _POSIX_C_SOURCE 200112L
# else
#  define _POSIX_C_SOURCE 200809L
# endif
# define __USE_POSIX_IMPLICITLY 1
#endif

#if defined(_POSIX_SOURCE) || \
   (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE+0 >= 1) || \
    defined(_XOPEN_SOURCE)
# define __USE_POSIX 1
#endif

#if defined(_XOPEN_SOURCE) || \
   (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE+0 >= 2)
# define __USE_POSIX2 1
#endif

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE+0 >= 199309L
# define __USE_POSIX199309 1
#endif

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE+0 >= 199506L
# define __USE_POSIX199506 1
#endif

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE+0 >= 200112L
# define __USE_XOPEN2K  1
# undef __USE_ISOC95
# define __USE_ISOC95  1
# undef __USE_ISOC99
# define __USE_ISOC99  1
#endif

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE+0 >= 200809L
# define __USE_XOPEN2K8  1
# undef  _ATFILE_SOURCE
# define _ATFILE_SOURCE 1
#endif

#ifdef _XOPEN_SOURCE
# define __USE_XOPEN 1
# if _XOPEN_SOURCE+0 >= 500
#  define __USE_XOPEN_EXTENDED 1
#  define __USE_UNIX98 1
#  undef _LARGEFILE_SOURCE
#  define _LARGEFILE_SOURCE 1
#  if _XOPEN_SOURCE+0 >= 600
#   if _XOPEN_SOURCE+0 >= 700
#    define __USE_XOPEN2K8 1
#    define __USE_XOPEN2K8XSI 1
#   endif
#   define __USE_XOPEN2K 1
#   define __USE_XOPEN2KXSI 1
#   undef __USE_ISOC95
#   define __USE_ISOC95  1
#   undef __USE_ISOC99
#   define __USE_ISOC99  1
#  endif
# else
#  ifdef _XOPEN_SOURCE_EXTENDED
#   define __USE_XOPEN_EXTENDED 1
#  endif
# endif
#endif

#ifdef _LARGEFILE_SOURCE
# define __USE_LARGEFILE 1
#endif

#ifdef _LARGEFILE64_SOURCE
# define __USE_LARGEFILE64 1
#endif

#if defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS+0 == 64
# define __USE_FILE_OFFSET64 1
#endif

#if defined _DEFAULT_SOURCE
# define __USE_MISC 1
#endif

#ifdef _ATFILE_SOURCE
# define __USE_ATFILE 1
#endif

#ifdef _GNU_SOURCE
# define __USE_GNU 1
#endif

#if (defined(_REENTRANT) || defined(_THREAD_SAFE)) || \
    (defined(__CRT_CYG) && defined(__DYNAMIC_REENT__) && !defined(__SINGLE_THREAD__))
#define __USE_REENTRANT 1
#endif

/* When targeting PE or DOS's CRT, enable DOS-extensions and DOS-filesystem by default. */
#if defined(__CRT_DOS) && !defined(__CRT_KOS) && \
   !defined(__CRT_GLC) && !defined(__CRT_CYG)
#define __USE_DOSFS 1 /* Enable DOS filesystem semantics. */
#endif
#if defined(__PE__) && !defined(__CRT_CYG)
#define __USE_DOS   1 /* Enable DOS extensions. */
#endif
#ifdef __CRT_CYG
#define __USE_CYG   1 /* Enable Cygwin extensions. */
#endif

#ifdef _CYG_SOURCE
#undef __USE_CYG
#if _CYG_SOURCE+0 != 0
#   define __USE_CYG   1
#endif
#endif

/* HINT: You can forceably disable DOS extensions in PE-mode by
 *       defining `_DOS_SOURCE' as an empty macro, or as a value
 *       equal to ZERO(0). */
#ifdef _DOS_SOURCE
#undef __USE_DOS
#if _DOS_SOURCE+0 == 0
#ifdef __CRT_KOS
#   undef __USE_DOSFS /* Also disable DOS-FS when linking against KOS's CRT. */
#endif
#else
#   define __USE_DOS   1
#endif
#if _DOS_SOURCE+0 >= 2
#   define __USE_OLD_DOS 1
#endif
#endif

/* HINT: In order to be able to use _DOS_SOURCE or `_DOSFS_SOURCE', `libc'
 *       must be built with `CONFIG_LIBC_NO_DOS_LIBC' disabled! */
#ifdef _DOSFS_SOURCE
#undef __USE_DOSFS
/* Manually enable DOS-FS */
#if _DOSFS_SOURCE+0 != 0
#   define __USE_DOSFS 1
#endif
#endif

#ifdef _ALL_SOURCE
#undef __USE_ISOC11
#undef __USE_ISOC99
#undef __USE_ISOC95
#undef __USE_ISOCXX11
#undef __USE_ISOCXX14
#undef __USE_POSIX
#undef __USE_POSIX2
#undef __USE_POSIX199309
#undef __USE_POSIX199506
#undef __USE_XOPEN
#undef __USE_XOPEN_EXTENDED
#undef __USE_UNIX98
#undef __USE_XOPEN2K
#undef __USE_XOPEN2KXSI
#undef __USE_XOPEN2K8
#undef __USE_XOPEN2K8XSI
#undef __USE_LARGEFILE
#undef __USE_LARGEFILE64
#undef __USE_MISC
#undef __USE_ATFILE
#undef __USE_GNU
#undef __USE_KOS
#undef __USE_KOS_DEPRECATED
#undef __USE_KXS
#undef __USE_UTF
#undef __USE_CYG
#undef __USE_DOS
#undef __USE_DOS_SLIB
#undef __USE_TIME64
#undef __USE_DEBUG
#undef __USE_EXCEPT
#define __USE_ISOC11 1
#define __USE_ISOC99 1
#define __USE_ISOC95 1
#define __USE_ISOCXX11 1
#define __USE_ISOCXX14 1
#define __USE_POSIX 1
#define __USE_POSIX2 1
#define __USE_POSIX199309 1
#define __USE_POSIX199506 1
#define __USE_XOPEN 1
#define __USE_XOPEN_EXTENDED 1
#define __USE_UNIX98 1
#define __USE_XOPEN2K 1
#define __USE_XOPEN2KXSI 1
#define __USE_XOPEN2K8 1
#define __USE_XOPEN2K8XSI 1
#define __USE_LARGEFILE 1
#define __USE_LARGEFILE64 1
#define __USE_MISC 1
#define __USE_ATFILE 1
#define __USE_GNU 1
#define __USE_KOS 1
#define __USE_KOS3 1
#define __USE_KOS_DEPRECATED 1
#define __USE_KXS 1
#define __USE_UTF 1
#define __USE_CYG 1
#define __USE_DOS 1
#define __USE_DOS_SLIB 1
#define __USE_TIME64 1
#define __USE_DEBUG 1
#if !defined(__USE_DOSFS) && defined(__CRT_KOS) && \
    !defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__) && \
    !defined(__CYG_COMPAT__) && __KOS_VERSION__ >= 300
#define __USE_EXCEPT 1
#endif
#endif


/* Try to enable 64-bit time by default (future-proofing) */
#if defined(__USE_DOS) || /* DOS already made this change. */ \
  (!defined(__CRT_GLC) && defined(__CRT_DOS))
#   define __USE_TIME_BITS64 1
#elif (!defined(_NO_EXPERIMENTAL_SOURCE) && 0) && \
      (defined(__CRT_KOS) || defined(__CRT_DOS))
#   define __USE_TIME_BITS64 1
#elif defined(__CYGWIN__) || defined(__CYGWIN32__)
#   include <hybrid/typecore.h>
#if __SIZEOF_POINTER__ >= 8
#   define __USE_TIME_BITS64 1
#endif
#endif

#if defined(__CYGWIN__) || defined(__CYGWIN32__) || \
    defined(__MINGW32__) || defined(WIN32) || defined(_WIN32) || \
    defined(WIN64) || defined(_WIN64)
#   define __WINDOWS_HOST__ 1
#endif


/* 64-bit time_t extensions for KOS
 * (By the time of this writing, but I'm guessing by 2038 this'll be
 *  similar to what glibc will have to do if it doesn't wan'na roll over) */
#ifdef _TIME64_SOURCE
#   define __USE_TIME64 1
#endif
#if defined(_TIME_T_BITS) && _TIME_T_BITS+0 >= 64
#   define __USE_TIME_BITS64 1
#elif defined(_TIME_T_BITS) && _TIME_T_BITS+0 && _TIME_T_BITS+0 < 64
#   undef __USE_TIME_BITS64
#elif defined(_USE_32BIT_TIME_T)
#   undef __USE_TIME_BITS64
#endif

#undef _USE_32BIT_TIME_T
#ifndef __USE_TIME_BITS64
#define _USE_32BIT_TIME_T 1
#endif

#ifdef _KOS_SOURCE
#   define __USE_KOS 1
#if __KOS_VERSION__ >= 300
#   define __USE_KOS3 1
#endif
#   define __USE_KOS_DEPRECATED 1
#if 1
#   define __USE_UTF 1
#endif
#if (_KOS_SOURCE+0) >= 2
#   define __USE_KXS 1
#endif
#endif

#undef __USE_KOS_STDEXT
#if defined(__USE_KXS) && !defined(__GNUC__) && \
    defined(__CRT_KOS) && (__SIZEOF_POINTER__ == __SIZEOF_INT__ || \
  (!defined(__DOS_COMPAT__) && !defined(__CYG_COMPAT__) && !defined(__GLC_COMPAT__)))
/* KOS offers extensions for ~fixing~ some STD-C functions,
 * such as having printf() return `ssize_t' instead of `int',
 * the logic here being that `printf()' returns the number
 * of printed characters, which depends on `strlen(format)',
 * which in turn evaluates to `size_t'.
 * >> This extension is disabled in compatibility mode, or when
 *    GCC is being used (which will otherwise complain about
 *    non-conforming builtin function prototypes). */
#define __USE_KOS_STDEXT 1
#endif

#if defined(__KOS_CRT) || \
    defined(_KOS_PRINTF_SOURCE) || \
    defined(__USE_KXS)
#define __USE_KOS_PRINTF 1
#endif


/* Enable additional utf16/32 functions in system headers.
 * NOTE: Most functions defined by this extension require __CRT_KOS to be available. */
#ifdef _UTF_SOURCE
#   undef __USE_UTF
#   define __USE_UTF 1
#endif

#ifdef __KERNEL__
/* Within the kernel, pre-configure based on config options. */
#   undef __USE_DOSFS
#   undef __USE_LARGEFILE64
#   undef __USE_FILE_OFFSET64
#   undef __USE_TIME64
#   undef __USE_TIME_BITS64
#   undef __USE_EXCEPT_API
#if defined(_FILE_OFFSET_BITS) && \
    ((_FILE_OFFSET_BITS+0) != 32 && (_FILE_OFFSET_BITS+0) != 64)
#warning "Unrecognized `_FILE_OFFSET_BITS'"
#undef _FILE_OFFSET_BITS
#endif
/* Use `CONFIG_32BIT_FILESYSTEM' to default-configure `_FILE_OFFSET_BITS' */
#ifndef _FILE_OFFSET_BITS
#ifdef CONFIG_32BIT_FILESYSTEM
#   define _FILE_OFFSET_BITS   32
#else /* CONFIG_32BIT_FILESYSTEM */
#   define _FILE_OFFSET_BITS   64
#endif /* !CONFIG_32BIT_FILESYSTEM */
#endif /* !_FILE_OFFSET_BITS */
/* Use `CONFIG_32BIT_TIME' to default-configure `_TIME_T_BITS' */
#ifndef _TIME_T_BITS
#ifdef CONFIG_32BIT_TIME
#   define _TIME_T_BITS   32
#else /* CONFIG_32BIT_TIME */
#   define _TIME_T_BITS   64
#endif /* !CONFIG_32BIT_TIME */
#endif /* !_TIME_T_BITS */
#if _FILE_OFFSET_BITS == 64
#   define __USE_LARGEFILE64   1
#   define __USE_FILE_OFFSET64 1
#endif /* _FILE_OFFSET_BITS == 64 */
#if _TIME_T_BITS == 64
#   define __USE_TIME64      1
#   define __USE_TIME_BITS64 1
#endif /* _TIME_T_BITS == 64 */
#endif /* __KERNEL__ */

#if (!defined(__CRT_DOS) && !defined(__CRT_KOS)) && \
    (defined(__USE_TIME64) || defined(__USE_TIME_BITS64)) && \
    (!defined(__CYGWIN__) && !defined(__CYGWIN32__))
#error "The selected CRT does not support 64-bit time() functions (Try building with `-D_TIME_T_BITS=32')"
#endif


/* Enable exception support for specific libC functions:
 * Functions supporting this feature are declared as `__REDIRECT_EXCEPT()',
 * with the following changes applying to semantics:
 *   - Rather than modifying `errno', an exception is thrown in most cases.
 *   - Exceptions to this rule are documented in comments next to the individual functions.
 * Exceptions are often generated by the kernel itself, in which case they
 * are propagated to user-space. With that in mind, most `errno' errors actually
 * start out as exceptions, before being translated to errno codes.
 * When this option is enabled, applications are linked against exception-enabled
 * libc functions, meaning that calling something like `malloc()' may throw `E_BADALLOC'
 * and never return `NULL', rather than returning `NULL' and setting `errno' to `ENOMEM'.
 * WARNING:
 *    This option is disabled by default, and care must be taken if you choose to enable it:
 *      - Try to keep this option consistent through your entire application
 *      - Using this option drastically reduces portability of your code
 *      - Using this option changes the prototypes and semantics of various functions:
 *        - `read()' no longer returns `ssize_t', but `size_t' instead, seeing as
 *           how exceptions are used to indicate any error-state, there no longer
 *           is any need for annoying signed integers and the case of negative
 *           values having to be handled (`if (x >= 0 && x < count)' is slower
 *           than `if (x < count)' which can be used instead when `x' is unsigned)
 *        - `malloc()' never returns `NULL', meaning you don't have to check for
 *           a return value of `NULL', but rather have to guard against `E_BADALLOC'
 *        -  ...
 *           There are a million other cases that could be documented here,
 *           but they all boil down to the same thing: No need for exception handling
 *           to be done by local case, often implemented as return-value checks.
 *           Instead, users can make use of exception handlers, as defined by `<except.h>'
 *      - In some places, `errno' may still be set, rather than throwing an exception.
 *        Such situations include `wait()' not throwing an error for `ECHILD', or
 *        functions taking some kind of timeout still setting `EAGAIN' or `ETIMEDOUT'
 *        like they would if exception were not enabled.
 *      - Exception-variants are not provided for DOSFS functions.
 *        Attempting to use `_DOSFS_SOURCE' with `_EXCEPT_API' is illegal.
 *      - Some functions that could be used (and do actually exist) as exception-enabled
 *        function will still not be linked, regardless of `_EXCEPT_API', and must
 *        be invoked explicitly, such as `munmap()' or `close()', where exceptions would
 *        be more annoying than useful. However, you can define `_EXCEPT_SOURCE' and
 *        call `Xmunmap()' explicitly to gain exception support for those.
 *        Functions affected by this are what I like to call "cleanup"-functions, often
 *        found in finalizers that are better left as NOEXCEPT, as they're usually called
 *        from exception cleanup code, or more precisely: `FINALLY' blocks.
 * Also note that some APIs, such as KOS's heap API in <kos/heap.h> _ALWAYS_ use
 * exceptions, rather than errno & some kind of special error-return-value,
 * irregardless of what this option has been defined as. */
#if defined(_EXCEPT_API) && (_EXCEPT_API+0 != 0)
#define __USE_EXCEPT_API 1
#endif
#if defined(_EXCEPT_SOURCE) && (_EXCEPT_SOURCE+0 != 0)
#define __USE_EXCEPT 1
#endif

#undef __ANY_COMPAT__
#if defined(__DOS_COMPAT__) || \
    defined(__GLC_COMPAT__) || \
    defined(__CYG_COMPAT__)
#define __ANY_COMPAT__ 1
#endif


#if defined(__USE_EXCEPT_API) || defined(__USE_EXCEPT)
/* Check that exception support can be provided in the current configuration. */
#ifdef __USE_DOSFS
#error "Cannot use `_DOSFS_SOURCE' alongside `_EXCEPT_SOURCE' / `_EXCEPT_API'"
#endif
#ifndef __CRT_KOS
#error "Exception supported requires the presense of CRT:KOS"
#endif
#ifdef __ANY_COMPAT__
#error "Cannot build in compatibility mode with `_EXCEPT_SOURCE' / `_EXCEPT_API' enabled"
#endif
#if __KOS_VERSION__ < 300
#error "`_EXCEPT_SOURCE' / `_EXCEPT_API' can only be used starting with KOS Mk3"
#endif
#endif


#ifndef __CRT_KOS
/* Check for illegal feature combinations. */
#if defined(__CRT_DOS) && !defined(__CRT_GLC)
#ifndef __USE_DOSFS
#   error "The linked CRT only supports DOS-FS mode (Try building with `-D_DOSFS_SOURCE=1')"
#endif
#elif (defined(__CRT_GLC) || defined(__CRT_CYG)) && !defined(__CRT_DOS)
#ifdef __USE_DOSFS
#   error "The linked CRT does not support DOS-FS mode (Try building with `-D_DOSFS_SOURCE=0')"
#endif
#endif /* ... */
#ifdef __USE_EXCEPT_API
#warning "Exception support is only provided by CRT:KOS"
#undef __USE_EXCEPT_API
#endif
#endif /* !__CRT_KOS */

#ifdef __DOS_COMPAT__
#ifndef __USE_DOSFS
#   error "DOS-FS mode cannot be disable in DOS compatibility mode (Try building with `-D_DOSFS_SOURCE=1')"
#endif
#elif defined(__GLC_COMPAT__)
#ifdef __USE_DOSFS
#   error "DOS-FS mode cannot be enable in GLibc compatibility mode (Try building with `-D_DOSFS_SOURCE=0')"
#endif
#endif



#ifndef __SYMNAME_DOSW16
#if __KOS_VERSION__ >= 300
/* KOS Mk3 changed the dos-prefix to something that can
 * be expressed by a compiler supporting $-in-identifiers. */
#ifdef __PE__
#   define __SYMNAME_DOSW16_IS_SAME          1
#   define __SYMNAME_DOS_IS_SAME             1
#   define __SYMNAME_W16_IS_SAME             1
#   define __SYMNAME_DOSDPW16(x)             _##x
#   define __SYMNAME_DOSW16(x)               x
#   define __SYMNAME_DOSW32(x)               U##x
#   define __SYMNAME_KOSW16(x)           KOS$u##x
#   define __SYMNAME_KOSW32(x)           KOS$##x
#   define __SYMNAME_W16(x)                  x
#   define __SYMNAME_W32(x)              KOS$##x
#   define __SYMNAME_DOSDP(x)                _##x
#   define __SYMNAME_DOS(x)                  x
#   define __SYMNAME_KOS(x)              KOS$##x
#   define __SYMNAME_EXCEPT_W16(x)           X##x
#   define __SYMNAME_EXCEPT_W32(x)       KOS$X##x
#   define __SYMNAME_EXCEPT(x)               X##x
#else
#   define __SYMNAME_KOSW32_IS_SAME          1
#   define __SYMNAME_KOS_IS_SAME             1
#   define __SYMNAME_W32_IS_SAME             1
#   define __SYMNAME_DOSW16(x)           DOS$##x
#   define __SYMNAME_DOSW32(x)           DOS$U##x
#   define __SYMNAME_DOSDPW16(x)         DOS$_##x
#   define __SYMNAME_KOSW16(x)               u##x
#   define __SYMNAME_KOSW32(x)               x
#   define __SYMNAME_W16(x)              DOS$##x
#   define __SYMNAME_W32(x)                  x
#   define __SYMNAME_DOSDP(x)            DOS$_##x
#   define __SYMNAME_DOS(x)              DOS$##x
#   define __SYMNAME_KOS(x)                  x
#   define __SYMNAME_EXCEPT_W16(x)       DOS$X##x
#   define __SYMNAME_EXCEPT_W32(x)           X##x
#   define __SYMNAME_EXCEPT(x)               X##x
#endif
#else /* __KOS_VERSION__ >= 300 */
#ifdef __PE__
#   define __SYMNAME_DOSW16_IS_SAME   1
#   define __SYMNAME_DOS_IS_SAME      1
#   define __SYMNAME_W16_IS_SAME      1
#   define __SYMNAME_DOSDPW16(x)      _##x
#   define __SYMNAME_DOSW16(x)        x
#   define __SYMNAME_DOSW32(x)        U##x
#   define __SYMNAME_KOSW16(x)   .kos.u##x
#   define __SYMNAME_KOSW32(x)   .kos.x
#   define __SYMNAME_W16(x)           x
#   define __SYMNAME_W32(x)      .kos.x
#   define __SYMNAME_DOSDP(x)         _##x
#   define __SYMNAME_DOS(x)           x
#   define __SYMNAME_KOS(x)      .kos.x
#else
#   define __SYMNAME_KOSW32_IS_SAME   1
#   define __SYMNAME_KOS_IS_SAME      1
#   define __SYMNAME_W32_IS_SAME      1
#   define __SYMNAME_DOSDPW16(x) .dos._##x
#   define __SYMNAME_DOSW16(x)   .dos.x
#   define __SYMNAME_DOSW32(x)   .dos.U##x
#   define __SYMNAME_KOSW16(x)        u##x
#   define __SYMNAME_KOSW32(x)        x
#   define __SYMNAME_W16(x)      .dos.x
#   define __SYMNAME_W32(x)           x
#   define __SYMNAME_DOSDP(x)    .dos._##x
#   define __SYMNAME_DOS(x)      .dos.x
#   define __SYMNAME_KOS(x)           x
#endif
#endif /* __KOS_VERSION__ < 300 */
#endif /* !__SYMNAME_DOSW16 */

#ifdef __SYMNAME_DOSW16_IS_SAME
#define __REDIRECT_TODOSW16(decl,attr,Treturn,cc,name,param,args)                                              __NOREDIRECT(decl,attr,Treturn,cc,name,param,name,args)
#define __REDIRECT_TODOSW16_                                                                                   __REDIRECT
#define __REDIRECT_TODOSW16_VOID(decl,attr,cc,name,param,args)                                                 __NOREDIRECT_VOID(decl,attr,cc,name,param,name,args)
#define __REDIRECT_TODOSW16_VOID_                                                                              __REDIRECT_VOID
#define __REDIRECT_TODOSW16_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                      __NOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name,args)
#define __REDIRECT_TODOSW16_NOTHROW_                                                                           __REDIRECT_NOTHROW
#define __REDIRECT_TODOSW16_VOID_NOTHROW(decl,attr,cc,name,param,args)                                         __NOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name,args)
#define __REDIRECT_TODOSW16_VOID_NOTHROW_                                                                      __REDIRECT_VOID_NOTHROW
#define __VREDIRECT_TODOSW16(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                   __VNOREDIRECT(decl,attr,Treturn,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_TODOSW16_                                                                                  __VREDIRECT
#define __VREDIRECT_TODOSW16_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                      __VNOREDIRECT_VOID(decl,attr,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_TODOSW16_VOID_                                                                             __VREDIRECT_VOID
#define __VREDIRECT_TODOSW16_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)           __VNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_TODOSW16_NOTHROW_                                                                          __VREDIRECT_NOTHROW
#define __VREDIRECT_TODOSW16_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)              __VNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_TODOSW16_VOID_NOTHROW_                                                                     __VREDIRECT_VOID_NOTHROW
#define __XREDIRECT_TODOSW16(decl,attr,Treturn,cc,name,param,code)                                             __XNOREDIRECT(decl,attr,Treturn,cc,name,param,name,code)
#define __XREDIRECT_TODOSW16_                                                                                  __XREDIRECT
#define __XREDIRECT_TODOSW16_VOID(decl,attr,cc,name,param,code)                                                __XNOREDIRECT_VOID(decl,attr,cc,name,param,name,code)
#define __XREDIRECT_TODOSW16_VOID_                                                                             __XREDIRECT_VOID
#define __XREDIRECT_TODOSW16_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                     __XNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name,code)
#define __XREDIRECT_TODOSW16_NOTHROW_                                                                          __XREDIRECT_NOTHROW
#define __XREDIRECT_TODOSW16_VOID_NOTHROW(decl,attr,cc,name,param,code)                                        __XNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name,code)
#define __XREDIRECT_TODOSW16_VOID_NOTHROW_                                                                     __XREDIRECT_VOID_NOTHROW
#else
#define __REDIRECT_TODOSW16(decl,attr,Treturn,cc,name,param,args)                                              __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW16(name),args)
#define __REDIRECT_TODOSW16_(decl,attr,Treturn,cc,name,param,asmname,args)                                     __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW16(asmname),args)
#define __REDIRECT_TODOSW16_VOID(decl,attr,cc,name,param,args)                                                 __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSW16(name),args)
#define __REDIRECT_TODOSW16_VOID_(decl,attr,cc,name,param,asmname,args)                                        __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSW16(asmname),args)
#define __REDIRECT_TODOSW16_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                      __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW16(name),args)
#define __REDIRECT_TODOSW16_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,args)                             __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW16(asmname),args)
#define __REDIRECT_TODOSW16_VOID_NOTHROW(decl,attr,cc,name,param,args)                                         __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSW16(name),args)
#define __REDIRECT_TODOSW16_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,args)                                __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSW16(asmname),args)
#define __VREDIRECT_TODOSW16(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                   __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW16(name),__SYMNAME_DOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW16_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)         __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW16(asmnamef),__SYMNAME_DOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW16_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                      __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSW16(name),__SYMNAME_DOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW16_VOID_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)            __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSW16(asmnamef),__SYMNAME_DOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW16_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)           __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW16(name),__SYMNAME_DOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW16_NOTHROW_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW16(asmnamef),__SYMNAME_DOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW16_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)              __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSW16(name),__SYMNAME_DOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW16_VOID_NOTHROW_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)    __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSW16(asmnamef),__SYMNAME_DOSW16(vasmnamef),args,before_va_start)
#define __XREDIRECT_TODOSW16(decl,attr,Treturn,cc,name,param,code)                                             __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW16(name),code)
#define __XREDIRECT_TODOSW16_(decl,attr,Treturn,cc,name,param,asmname,code)                                    __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW16(asmname),code)
#define __XREDIRECT_TODOSW16_VOID(decl,attr,cc,name,param,code)                                                __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSW16(name),code)
#define __XREDIRECT_TODOSW16_VOID_(decl,attr,cc,name,param,asmname,code)                                       __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSW16(asmname),code)
#define __XREDIRECT_TODOSW16_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                     __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW16(name),code)
#define __XREDIRECT_TODOSW16_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,code)                            __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW16(asmname),code)
#define __XREDIRECT_TODOSW16_VOID_NOTHROW(decl,attr,cc,name,param,code)                                        __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSW16(name),code)
#define __XREDIRECT_TODOSW16_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,code)                               __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSW16(asmname),code)
#endif
#define __REDIRECT_TODOSW32(decl,attr,Treturn,cc,name,param,args)                                              __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW32(name),args)
#define __REDIRECT_TODOSW32_(decl,attr,Treturn,cc,name,param,asmname,args)                                     __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW32(asmname),args)
#define __REDIRECT_TODOSW32_VOID(decl,attr,cc,name,param,args)                                                 __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSW32(name),args)
#define __REDIRECT_TODOSW32_VOID_(decl,attr,cc,name,param,asmname,args)                                        __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSW32(asmname),args)
#define __REDIRECT_TODOSW32_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                      __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW32(name),args)
#define __REDIRECT_TODOSW32_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,args)                             __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW32(asmname),args)
#define __REDIRECT_TODOSW32_VOID_NOTHROW(decl,attr,cc,name,param,args)                                         __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSW32(name),args)
#define __REDIRECT_TODOSW32_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,args)                                __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSW32(asmname),args)
#define __VREDIRECT_TODOSW32(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                   __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW32(name),__SYMNAME_DOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW32_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)         __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW32(asmnamef),__SYMNAME_DOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW32_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                      __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSW32(name),__SYMNAME_DOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW32_VOID_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)            __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSW32(asmnamef),__SYMNAME_DOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW32_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)           __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW32(name),__SYMNAME_DOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW32_NOTHROW_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW32(asmnamef),__SYMNAME_DOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW32_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)              __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSW32(name),__SYMNAME_DOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSW32_VOID_NOTHROW_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)    __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSW32(asmnamef),__SYMNAME_DOSW32(vasmnamef),args,before_va_start)
#define __XREDIRECT_TODOSW32(decl,attr,Treturn,cc,name,param,code)                                             __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW32(name),code)
#define __XREDIRECT_TODOSW32_(decl,attr,Treturn,cc,name,param,asmname,code)                                    __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW32(asmname),code)
#define __XREDIRECT_TODOSW32_VOID(decl,attr,cc,name,param,code)                                                __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSW32(name),code)
#define __XREDIRECT_TODOSW32_VOID_(decl,attr,cc,name,param,asmname,code)                                       __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSW32(asmname),code)
#define __XREDIRECT_TODOSW32_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                     __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW32(name),code)
#define __XREDIRECT_TODOSW32_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,code)                            __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSW32(asmname),code)
#define __XREDIRECT_TODOSW32_VOID_NOTHROW(decl,attr,cc,name,param,code)                                        __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSW32(name),code)
#define __XREDIRECT_TODOSW32_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,code)                               __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSW32(asmname),code)
#define __REDIRECT_TODOSDPW16(decl,attr,Treturn,cc,name,param,args)                                              __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSDPW16(name),args)
#define __REDIRECT_TODOSDPW16_(decl,attr,Treturn,cc,name,param,asmname,args)                                     __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSDPW16(asmname),args)
#define __REDIRECT_TODOSDPW16_VOID(decl,attr,cc,name,param,args)                                                 __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSDPW16(name),args)
#define __REDIRECT_TODOSDPW16_VOID_(decl,attr,cc,name,param,asmname,args)                                        __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSDPW16(asmname),args)
#define __REDIRECT_TODOSDPW16_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                      __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSDPW16(name),args)
#define __REDIRECT_TODOSDPW16_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,args)                             __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSDPW16(asmname),args)
#define __REDIRECT_TODOSDPW16_VOID_NOTHROW(decl,attr,cc,name,param,args)                                         __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSDPW16(name),args)
#define __REDIRECT_TODOSDPW16_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,args)                                __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSDPW16(asmname),args)
#define __VREDIRECT_TODOSDPW16(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                   __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSDPW16(name),__SYMNAME_DOSDPW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSDPW16_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)         __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSDPW16(asmnamef),__SYMNAME_DOSDPW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSDPW16_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                      __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSDPW16(name),__SYMNAME_DOSDPW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSDPW16_VOID_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)            __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSDPW16(asmnamef),__SYMNAME_DOSDPW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSDPW16_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)           __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSDPW16(name),__SYMNAME_DOSDPW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSDPW16_NOTHROW_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSDPW16(asmnamef),__SYMNAME_DOSDPW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSDPW16_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)              __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSDPW16(name),__SYMNAME_DOSDPW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TODOSDPW16_VOID_NOTHROW_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)    __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSDPW16(asmnamef),__SYMNAME_DOSDPW16(vasmnamef),args,before_va_start)
#define __XREDIRECT_TODOSDPW16(decl,attr,Treturn,cc,name,param,code)                                             __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSDPW16(name),code)
#define __XREDIRECT_TODOSDPW16_(decl,attr,Treturn,cc,name,param,asmname,code)                                    __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSDPW16(asmname),code)
#define __XREDIRECT_TODOSDPW16_VOID(decl,attr,cc,name,param,code)                                                __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSDPW16(name),code)
#define __XREDIRECT_TODOSDPW16_VOID_(decl,attr,cc,name,param,asmname,code)                                       __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSDPW16(asmname),code)
#define __XREDIRECT_TODOSDPW16_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                     __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSDPW16(name),code)
#define __XREDIRECT_TODOSDPW16_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,code)                            __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSDPW16(asmname),code)
#define __XREDIRECT_TODOSDPW16_VOID_NOTHROW(decl,attr,cc,name,param,code)                                        __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSDPW16(name),code)
#define __XREDIRECT_TODOSDPW16_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,code)                               __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSDPW16(asmname),code)
#define __REDIRECT_TOKOSW16(decl,attr,Treturn,cc,name,param,args)                                              __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW16(name),args)
#define __REDIRECT_TOKOSW16_(decl,attr,Treturn,cc,name,param,asmname,args)                                     __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW16(asmname),args)
#define __REDIRECT_TOKOSW16_VOID(decl,attr,cc,name,param,args)                                                 __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOSW16(name),args)
#define __REDIRECT_TOKOSW16_VOID_(decl,attr,cc,name,param,asmname,args)                                        __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOSW16(asmname),args)
#define __REDIRECT_TOKOSW16_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                      __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW16(name),args)
#define __REDIRECT_TOKOSW16_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,args)                             __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW16(asmname),args)
#define __REDIRECT_TOKOSW16_VOID_NOTHROW(decl,attr,cc,name,param,args)                                         __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOSW16(name),args)
#define __REDIRECT_TOKOSW16_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,args)                                __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOSW16(asmname),args)
#define __VREDIRECT_TOKOSW16(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                   __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW16(name),__SYMNAME_KOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW16_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)         __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW16(asmnamef),__SYMNAME_KOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW16_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                      __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOSW16(name),__SYMNAME_KOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW16_VOID_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)            __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOSW16(asmnamef),__SYMNAME_KOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW16_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)           __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW16(name),__SYMNAME_KOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW16_NOTHROW_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW16(asmnamef),__SYMNAME_KOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW16_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)              __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOSW16(name),__SYMNAME_KOSW16(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW16_VOID_NOTHROW_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)    __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOSW16(asmnamef),__SYMNAME_KOSW16(vasmnamef),args,before_va_start)
#define __XREDIRECT_TOKOSW16(decl,attr,Treturn,cc,name,param,code)                                             __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW16(name),code)
#define __XREDIRECT_TOKOSW16_(decl,attr,Treturn,cc,name,param,asmname,code)                                    __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW16(asmname),code)
#define __XREDIRECT_TOKOSW16_VOID(decl,attr,cc,name,param,code)                                                __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOSW16(name),code)
#define __XREDIRECT_TOKOSW16_VOID_(decl,attr,cc,name,param,asmname,code)                                       __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOSW16(asmname),code)
#define __XREDIRECT_TOKOSW16_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                     __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW16(name),code)
#define __XREDIRECT_TOKOSW16_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,code)                            __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW16(asmname),code)
#define __XREDIRECT_TOKOSW16_VOID_NOTHROW(decl,attr,cc,name,param,code)                                        __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOSW16(name),code)
#define __XREDIRECT_TOKOSW16_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,code)                               __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOSW16(asmname),code)
#ifdef __SYMNAME_KOSW32_IS_SAME
#define __REDIRECT_TOKOSW32(decl,attr,Treturn,cc,name,param,args)                                              __NOREDIRECT(decl,attr,Treturn,cc,name,param,name,args)
#define __REDIRECT_TOKOSW32_                                                                                   __REDIRECT
#define __REDIRECT_TOKOSW32_VOID(decl,attr,cc,name,param,args)                                                 __NOREDIRECT_VOID(decl,attr,cc,name,param,name,args)
#define __REDIRECT_TOKOSW32_VOID_                                                                              __REDIRECT_VOID
#define __REDIRECT_TOKOSW32_NOTHROW(decl,Treturn,attr,cc,name,param,args)                                      __NOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name,args)
#define __REDIRECT_TOKOSW32_NOTHROW_                                                                           __REDIRECT_NOTHROW
#define __REDIRECT_TOKOSW32_VOID_NOTHROW(decl,attr,cc,name,param,args)                                         __NOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name,args)
#define __REDIRECT_TOKOSW32_VOID_NOTHROW_                                                                      __REDIRECT_VOID_NOTHROW
#define __VREDIRECT_TOKOSW32(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                   __VNOREDIRECT(decl,attr,Treturn,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_TOKOSW32_                                                                                  __VREDIRECT
#define __VREDIRECT_TOKOSW32_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                      __VNOREDIRECT_VOID(decl,attr,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_TOKOSW32_VOID_                                                                             __VREDIRECT_VOID
#define __VREDIRECT_TOKOSW32_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)           __VNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_TOKOSW32_NOTHROW_                                                                          __VREDIRECT_NOTHROW
#define __VREDIRECT_TOKOSW32_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)              __VNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_TOKOSW32_VOID_NOTHROW_                                                                     __VREDIRECT_VOID_NOTHROW
#define __XREDIRECT_TOKOSW32(decl,attr,Treturn,cc,name,param,code)                                             __XNOREDIRECT(decl,attr,Treturn,cc,name,param,name,code)
#define __XREDIRECT_TOKOSW32_                                                                                  __XREDIRECT
#define __XREDIRECT_TOKOSW32_VOID(decl,attr,cc,name,param,code)                                                __XNOREDIRECT_VOID(decl,attr,cc,name,param,name,code)
#define __XREDIRECT_TOKOSW32_VOID_                                                                             __XREDIRECT_VOID
#define __XREDIRECT_TOKOSW32_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                     __XNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name,code)
#define __XREDIRECT_TOKOSW32_NOTHROW_                                                                          __XREDIRECT_NOTHROW
#define __XREDIRECT_TOKOSW32_VOID_NOTHROW(decl,attr,cc,name,param,code)                                        __XNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name,code)
#define __XREDIRECT_TOKOSW32_VOID_NOTHROW_                                                                     __XREDIRECT_VOID_NOTHROW
#else
#define __REDIRECT_TOKOSW32(decl,attr,Treturn,cc,name,param,args)                                              __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW32(name),args)
#define __REDIRECT_TOKOSW32_(decl,attr,Treturn,cc,name,param,asmname,args)                                     __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW32(asmname),args)
#define __REDIRECT_TOKOSW32_VOID(decl,attr,cc,name,param,args)                                                 __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOSW32(name),args)
#define __REDIRECT_TOKOSW32_VOID_(decl,attr,cc,name,param,asmname,args)                                        __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOSW32(asmname),args)
#define __REDIRECT_TOKOSW32_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                      __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW32(name),args)
#define __REDIRECT_TOKOSW32_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,args)                             __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW32(asmname),args)
#define __REDIRECT_TOKOSW32_VOID_NOTHROW(decl,attr,cc,name,param,args)                                         __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOSW32(name),args)
#define __REDIRECT_TOKOSW32_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,args)                                __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOSW32(asmname),args)
#define __VREDIRECT_TOKOSW32(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                   __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW32(name),__SYMNAME_KOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW32_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)         __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW32(asmnamef),__SYMNAME_KOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW32_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                      __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOSW32(name),__SYMNAME_KOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW32_VOID_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)            __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOSW32(asmnamef),__SYMNAME_KOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW32_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)           __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW32(name),__SYMNAME_KOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW32_NOTHROW_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start) __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW32(asmnamef),__SYMNAME_KOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW32_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)              __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOSW32(name),__SYMNAME_KOSW32(vasmnamef),args,before_va_start)
#define __VREDIRECT_TOKOSW32_VOID_NOTHROW_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)    __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOSW32(asmnamef),__SYMNAME_KOSW32(vasmnamef),args,before_va_start)
#define __XREDIRECT_TOKOSW32(decl,attr,Treturn,cc,name,param,code)                                             __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW32(name),code)
#define __XREDIRECT_TOKOSW32_(decl,attr,Treturn,cc,name,param,asmname,code)                                    __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW32(asmname),code)
#define __XREDIRECT_TOKOSW32_VOID(decl,attr,cc,name,param,code)                                                __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOSW32(name),code)
#define __XREDIRECT_TOKOSW32_VOID_(decl,attr,cc,name,param,asmname,code)                                       __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOSW32(asmname),code)
#define __XREDIRECT_TOKOSW32_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                     __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW32(name),code)
#define __XREDIRECT_TOKOSW32_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,code)                            __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOSW32(asmname),code)
#define __XREDIRECT_TOKOSW32_VOID_NOTHROW(decl,attr,cc,name,param,code)                                        __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOSW32(name),code)
#define __XREDIRECT_TOKOSW32_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,code)                               __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOSW32(asmname),code)
#endif

#define __REDIRECT_TOKOS                 __REDIRECT_TOKOSW32
#define __REDIRECT_TODOS                 __REDIRECT_TODOSW16
#define __REDIRECT_TOKOS_                __REDIRECT_TOKOSW32_
#define __REDIRECT_TODOS_                __REDIRECT_TODOSW16_
#define __REDIRECT_TOKOS_VOID            __REDIRECT_TOKOSW32_VOID
#define __REDIRECT_TODOS_VOID            __REDIRECT_TODOSW16_VOID
#define __REDIRECT_TOKOS_VOID_           __REDIRECT_TOKOSW32_VOID_
#define __REDIRECT_TODOS_VOID_           __REDIRECT_TODOSW16_VOID_
#define __REDIRECT_TOKOS_NOTHROW         __REDIRECT_TOKOSW32_NOTHROW
#define __REDIRECT_TODOS_NOTHROW         __REDIRECT_TODOSW16_NOTHROW
#define __REDIRECT_TOKOS_NOTHROW_        __REDIRECT_TOKOSW32_NOTHROW_
#define __REDIRECT_TODOS_NOTHROW_        __REDIRECT_TODOSW16_NOTHROW_
#define __REDIRECT_TOKOS_VOID_NOTHROW    __REDIRECT_TOKOSW32_VOID_NOTHROW
#define __REDIRECT_TODOS_VOID_NOTHROW    __REDIRECT_TODOSW16_VOID_NOTHROW
#define __REDIRECT_TOKOS_VOID_NOTHROW_   __REDIRECT_TOKOSW32_VOID_NOTHROW_
#define __REDIRECT_TODOS_VOID_NOTHROW_   __REDIRECT_TODOSW16_VOID_NOTHROW_
#define __VREDIRECT_TOKOS                __VREDIRECT_TOKOSW32
#define __VREDIRECT_TODOS                __VREDIRECT_TODOSW16
#define __VREDIRECT_TOKOS_               __VREDIRECT_TOKOSW32_
#define __VREDIRECT_TODOS_               __VREDIRECT_TODOSW16_
#define __VREDIRECT_TOKOS_VOID           __VREDIRECT_TOKOSW32_VOID
#define __VREDIRECT_TODOS_VOID           __VREDIRECT_TODOSW16_VOID
#define __VREDIRECT_TOKOS_VOID_          __VREDIRECT_TOKOSW32_VOID_
#define __VREDIRECT_TODOS_VOID_          __VREDIRECT_TODOSW16_VOID_
#define __VREDIRECT_TOKOS_NOTHROW        __VREDIRECT_TOKOSW32_NOTHROW
#define __VREDIRECT_TODOS_NOTHROW        __VREDIRECT_TODOSW16_NOTHROW
#define __VREDIRECT_TOKOS_NOTHROW_       __VREDIRECT_TOKOSW32_NOTHROW_
#define __VREDIRECT_TODOS_NOTHROW_       __VREDIRECT_TODOSW16_NOTHROW_
#define __VREDIRECT_TOKOS_VOID_NOTHROW   __VREDIRECT_TOKOSW32_VOID_NOTHROW
#define __VREDIRECT_TODOS_VOID_NOTHROW   __VREDIRECT_TODOSW16_VOID_NOTHROW
#define __VREDIRECT_TOKOS_VOID_NOTHROW_  __VREDIRECT_TOKOSW32_VOID_NOTHROW_
#define __VREDIRECT_TODOS_VOID_NOTHROW_  __VREDIRECT_TODOSW16_VOID_NOTHROW_
#define __XREDIRECT_TOKOS                __XREDIRECT_TOKOSW32
#define __XREDIRECT_TODOS                __XREDIRECT_TODOSW16
#define __XREDIRECT_TOKOS_               __XREDIRECT_TOKOSW32_
#define __XREDIRECT_TODOS_               __XREDIRECT_TODOSW16_
#define __XREDIRECT_TOKOS_VOID           __XREDIRECT_TOKOSW32_VOID
#define __XREDIRECT_TODOS_VOID           __XREDIRECT_TODOSW16_VOID
#define __XREDIRECT_TOKOS_VOID_          __XREDIRECT_TOKOSW32_VOID_
#define __XREDIRECT_TODOS_VOID_          __XREDIRECT_TODOSW16_VOID_
#define __XREDIRECT_TOKOS_NOTHROW        __XREDIRECT_TOKOSW32_NOTHROW
#define __XREDIRECT_TODOS_NOTHROW        __XREDIRECT_TODOSW16_NOTHROW
#define __XREDIRECT_TOKOS_NOTHROW_       __XREDIRECT_TOKOSW32_NOTHROW_
#define __XREDIRECT_TODOS_NOTHROW_       __XREDIRECT_TODOSW16_NOTHROW_
#define __XREDIRECT_TOKOS_VOID_NOTHROW   __XREDIRECT_TOKOSW32_VOID_NOTHROW
#define __XREDIRECT_TODOS_VOID_NOTHROW   __XREDIRECT_TODOSW16_VOID_NOTHROW
#define __XREDIRECT_TOKOS_VOID_NOTHROW_  __XREDIRECT_TOKOSW32_VOID_NOTHROW_
#define __XREDIRECT_TODOS_VOID_NOTHROW_  __XREDIRECT_TODOSW16_VOID_NOTHROW_

/* Aliases for platform-independent wide-character redirection. */
#define __REDIRECT_W16                   __REDIRECT_TODOSW16_
#define __REDIRECT_W32                   __REDIRECT_TOKOSW32_
#define __REDIRECT_W16_VOID              __REDIRECT_TODOSW16_VOID_
#define __REDIRECT_W32_VOID              __REDIRECT_TOKOSW32_VOID_
#define __REDIRECT_W16_NOTHROW           __REDIRECT_TODOSW16_NOTHROW_
#define __REDIRECT_W32_NOTHROW           __REDIRECT_TOKOSW32_NOTHROW_
#define __REDIRECT_W16_VOID_NOTHROW      __REDIRECT_TODOSW16_VOID_NOTHROW_
#define __REDIRECT_W32_VOID_NOTHROW      __REDIRECT_TOKOSW32_VOID_NOTHROW_
#define __VREDIRECT_W16                  __VREDIRECT_TODOSW16_
#define __VREDIRECT_W32                  __VREDIRECT_TOKOSW32_
#define __VREDIRECT_W16_VOID             __VREDIRECT_TODOSW16_VOID_
#define __VREDIRECT_W32_VOID             __VREDIRECT_TOKOSW32_VOID_
#define __VREDIRECT_W16_NOTHROW          __VREDIRECT_TODOSW16_NOTHROW_
#define __VREDIRECT_W32_NOTHROW          __VREDIRECT_TOKOSW32_NOTHROW_
#define __VREDIRECT_W16_VOID_NOTHROW     __VREDIRECT_TODOSW16_VOID_NOTHROW_
#define __VREDIRECT_W32_VOID_NOTHROW     __VREDIRECT_TOKOSW32_VOID_NOTHROW_
#define __XREDIRECT_W16                  __XREDIRECT_TODOSW16_
#define __XREDIRECT_W32                  __XREDIRECT_TOKOSW32_
#define __XREDIRECT_W16_VOID             __XREDIRECT_TODOSW16_VOID_
#define __XREDIRECT_W32_VOID             __XREDIRECT_TOKOSW32_VOID_
#define __XREDIRECT_W16_NOTHROW          __XREDIRECT_TODOSW16_NOTHROW_
#define __XREDIRECT_W32_NOTHROW          __XREDIRECT_TOKOSW32_NOTHROW_
#define __XREDIRECT_W16_VOID_NOTHROW     __XREDIRECT_TODOSW16_VOID_NOTHROW_
#define __XREDIRECT_W32_VOID_NOTHROW     __XREDIRECT_TOKOSW32_VOID_NOTHROW_
#define __REDIRECT_DPW16                 __REDIRECT_TODOSDPW16_
#define __REDIRECT_DPW16_VOID            __REDIRECT_TODOSDPW16_VOID_
#define __REDIRECT_DPW16_NOTHROW         __REDIRECT_TODOSDPW16_NOTHROW_
#define __REDIRECT_DPW16_VOID_NOTHROW    __REDIRECT_TODOSDPW16_VOID_NOTHROW_
#define __VREDIRECT_DPW16                __VREDIRECT_TODOSDPW16_
#define __VREDIRECT_DPW16_VOID           __VREDIRECT_TODOSDPW16_VOID_
#define __VREDIRECT_DPW16_NOTHROW        __VREDIRECT_TODOSDPW16_NOTHROW_
#define __VREDIRECT_DPW16_VOID_NOTHROW   __VREDIRECT_TODOSDPW16_VOID_NOTHROW_
#define __XREDIRECT_DPW16                __XREDIRECT_TODOSDPW16_
#define __XREDIRECT_DPW16_VOID           __XREDIRECT_TODOSDPW16_VOID_
#define __XREDIRECT_DPW16_NOTHROW        __XREDIRECT_TODOSDPW16_NOTHROW_
#define __XREDIRECT_DPW16_VOID_NOTHROW   __XREDIRECT_TODOSDPW16_VOID_NOTHROW_

#ifdef __USE_DOS
#define __REDIRECT_DOS                   __REDIRECT_TODOS
#define __REDIRECT_DOS_                  __REDIRECT_TODOS_
#define __REDIRECT_DOS_VOID              __REDIRECT_TODOS_VOID
#define __REDIRECT_DOS_VOID_             __REDIRECT_TODOS_VOID_
#define __REDIRECT_DOS_NOTHROW           __REDIRECT_TODOS_NOTHROW
#define __REDIRECT_DOS_NOTHROW_          __REDIRECT_TODOS_NOTHROW_
#define __REDIRECT_DOS_VOID_NOTHROW      __REDIRECT_TODOS_VOID_NOTHROW
#define __REDIRECT_DOS_VOID_NOTHROW_     __REDIRECT_TODOS_VOID_NOTHROW_
#define __VREDIRECT_DOS                  __VREDIRECT_TODOS
#define __VREDIRECT_DOS_                 __VREDIRECT_TODOS_
#define __VREDIRECT_DOS_VOID             __VREDIRECT_TODOS_VOID
#define __VREDIRECT_DOS_VOID_            __VREDIRECT_TODOS_VOID_
#define __VREDIRECT_DOS_NOTHROW          __VREDIRECT_TODOS_NOTHROW
#define __VREDIRECT_DOS_NOTHROW_         __VREDIRECT_TODOS_NOTHROW_
#define __VREDIRECT_DOS_VOID_NOTHROW     __VREDIRECT_TODOS_VOID_NOTHROW
#define __VREDIRECT_DOS_VOID_NOTHROW_    __VREDIRECT_TODOS_VOID_NOTHROW_
#define __XREDIRECT_DOS                  __XREDIRECT_TODOS
#define __XREDIRECT_DOS_                 __XREDIRECT_TODOS_
#define __XREDIRECT_DOS_VOID             __XREDIRECT_TODOS_VOID
#define __XREDIRECT_DOS_VOID_            __XREDIRECT_TODOS_VOID_
#define __XREDIRECT_DOS_NOTHROW          __XREDIRECT_TODOS_NOTHROW
#define __XREDIRECT_DOS_NOTHROW_         __XREDIRECT_TODOS_NOTHROW_
#define __XREDIRECT_DOS_VOID_NOTHROW     __XREDIRECT_TODOS_VOID_NOTHROW
#define __XREDIRECT_DOS_VOID_NOTHROW_    __XREDIRECT_TODOS_VOID_NOTHROW_
#else
#define __REDIRECT_DOS                   __REDIRECT_TOKOS
#define __REDIRECT_DOS_                  __REDIRECT_TOKOS_
#define __REDIRECT_DOS_VOID              __REDIRECT_TOKOS_VOID
#define __REDIRECT_DOS_VOID_             __REDIRECT_TOKOS_VOID_
#define __REDIRECT_DOS_NOTHROW           __REDIRECT_TOKOS_NOTHROW
#define __REDIRECT_DOS_NOTHROW_          __REDIRECT_TOKOS_NOTHROW_
#define __REDIRECT_DOS_VOID_NOTHROW      __REDIRECT_TOKOS_VOID_NOTHROW
#define __REDIRECT_DOS_VOID_NOTHROW_     __REDIRECT_TOKOS_VOID_NOTHROW_
#define __VREDIRECT_DOS                  __VREDIRECT_TOKOS
#define __VREDIRECT_DOS_                 __VREDIRECT_TOKOS_
#define __VREDIRECT_DOS_VOID             __VREDIRECT_TOKOS_VOID
#define __VREDIRECT_DOS_VOID_            __VREDIRECT_TOKOS_VOID_
#define __VREDIRECT_DOS_NOTHROW          __VREDIRECT_TOKOS_NOTHROW
#define __VREDIRECT_DOS_NOTHROW_         __VREDIRECT_TOKOS_NOTHROW_
#define __VREDIRECT_DOS_VOID_NOTHROW     __VREDIRECT_TOKOS_VOID_NOTHROW
#define __VREDIRECT_DOS_VOID_NOTHROW_    __VREDIRECT_TOKOS_VOID_NOTHROW_
#define __XREDIRECT_DOS                  __XREDIRECT_TOKOS
#define __XREDIRECT_DOS_                 __XREDIRECT_TOKOS_
#define __XREDIRECT_DOS_VOID             __XREDIRECT_TOKOS_VOID
#define __XREDIRECT_DOS_VOID_            __XREDIRECT_TOKOS_VOID_
#define __XREDIRECT_DOS_NOTHROW          __XREDIRECT_TOKOS_NOTHROW
#define __XREDIRECT_DOS_NOTHROW_         __XREDIRECT_TOKOS_NOTHROW_
#define __XREDIRECT_DOS_VOID_NOTHROW     __XREDIRECT_TOKOS_VOID_NOTHROW
#define __XREDIRECT_DOS_VOID_NOTHROW_    __XREDIRECT_TOKOS_VOID_NOTHROW_
#endif

#ifndef __REDIRECT_EXCEPT
#ifdef __USE_EXCEPT_API
/* Exception redirection functions */
#define __IF_USE_EXCEPT(x)      x
#define __IF_NUSE_EXCEPT(x)     /* nothing */
#define __EXCEPT_SELECT(tt,ff)  tt
#define __REDIRECT_EXCEPT_TOW16(decl,attr,Treturn,cc,name,param,args)                                                    __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_W16(name),args)
#define __REDIRECT_EXCEPT_TOW16_(decl,attr,Treturn,cc,name,param,asmname,args)                                           __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_W16(asmname),args)
#define __REDIRECT_EXCEPT_TOW16_XVOID(decl,attr,Treturn_nexcept,cc,name,param,args)                                      __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W16(name),args)
#define __REDIRECT_EXCEPT_TOW16_XVOID_(decl,attr,Treturn_nexcept,cc,name,param,asmname,args)                             __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W16(asmname),args)
#define __REDIRECT_EXCEPT_TOW16_VOID(decl,attr,cc,name,param,args)                                                       __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W16(name),args)
#define __REDIRECT_EXCEPT_TOW16_VOID_(decl,attr,cc,name,param,asmname,args)                                              __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W16(asmname),args)
#define __VREDIRECT_EXCEPT_TOW16(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                         __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_W16(name),__SYMNAME_EXCEPT_W16(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_TOW16_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)               __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_W16(asmnamef),__SYMNAME_EXCEPT_W16(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_TOW16_XVOID(decl,attr,Treturn_nexcept,cc,name,param,vasmnamef,args,before_va_start)           __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W16(name),__SYMNAME_EXCEPT_W16(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_TOW16_XVOID_(decl,attr,Treturn_nexcept,cc,name,param,asmnamef,vasmnamef,args,before_va_start) __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W16(asmnamef),__SYMNAME_EXCEPT_W16(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_TOW16_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                            __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W16(name),__SYMNAME_EXCEPT_W16(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_TOW16_VOID_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                  __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W16(asmnamef),__SYMNAME_EXCEPT_W16(vasmnamef),args,before_va_start)
#define __XREDIRECT_EXCEPT_TOW16(decl,attr,Treturn,cc,name,param,code)                                                   __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_W16(name),code)
#define __XREDIRECT_EXCEPT_TOW16_(decl,attr,Treturn,cc,name,param,asmname,code)                                          __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_W16(asmname),code)
#define __XREDIRECT_EXCEPT_TOW16_XVOID(decl,attr,Treturn_nexcept,cc,name,param,code)                                     __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W16(name),code)
#define __XREDIRECT_EXCEPT_TOW16_XVOID_(decl,attr,Treturn_nexcept,cc,name,param,asmname,code)                            __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W16(asmname),code)
#define __XREDIRECT_EXCEPT_TOW16_VOID(decl,attr,cc,name,param,code)                                                      __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W16(name),code)
#define __XREDIRECT_EXCEPT_TOW16_VOID_(decl,attr,cc,name,param,asmname,code)                                             __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W16(asmname),code)
#define __REDIRECT_EXCEPT_TOW32(decl,attr,Treturn,cc,name,param,args)                                                    __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_W32(name),args)
#define __REDIRECT_EXCEPT_TOW32_(decl,attr,Treturn,cc,name,param,asmname,args)                                           __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_W32(asmname),args)
#define __REDIRECT_EXCEPT_TOW32_XVOID(decl,attr,Treturn_nexcept,cc,name,param,args)                                      __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W32(name),args)
#define __REDIRECT_EXCEPT_TOW32_XVOID_(decl,attr,Treturn_nexcept,cc,name,param,asmname,args)                             __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W32(asmname),args)
#define __REDIRECT_EXCEPT_TOW32_VOID(decl,attr,cc,name,param,args)                                                       __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W32(name),args)
#define __REDIRECT_EXCEPT_TOW32_VOID_(decl,attr,cc,name,param,asmname,args)                                              __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W32(asmname),args)
#define __VREDIRECT_EXCEPT_TOW32(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                         __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_W32(name),__SYMNAME_EXCEPT_W32(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_TOW32_(decl,attr,Treturn,cc,name,param,asmnamef,vasmnamef,args,before_va_start)               __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_W32(asmnamef),__SYMNAME_EXCEPT_W32(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_TOW32_XVOID(decl,attr,Treturn_nexcept,cc,name,param,vasmnamef,args,before_va_start)           __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W32(name),__SYMNAME_EXCEPT_W32(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_TOW32_XVOID_(decl,attr,Treturn_nexcept,cc,name,param,asmnamef,vasmnamef,args,before_va_start) __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W32(asmnamef),__SYMNAME_EXCEPT_W32(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_TOW32_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                            __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W32(name),__SYMNAME_EXCEPT_W32(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_TOW32_VOID_(decl,attr,cc,name,param,asmnamef,vasmnamef,args,before_va_start)                  __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W32(asmnamef),__SYMNAME_EXCEPT_W32(vasmnamef),args,before_va_start)
#define __XREDIRECT_EXCEPT_TOW32(decl,attr,Treturn,cc,name,param,code)                                                   __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_W32(name),code)
#define __XREDIRECT_EXCEPT_TOW32_(decl,attr,Treturn,cc,name,param,asmname,code)                                          __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_W32(asmname),code)
#define __XREDIRECT_EXCEPT_TOW32_XVOID(decl,attr,Treturn_nexcept,cc,name,param,code)                                     __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W32(name),code)
#define __XREDIRECT_EXCEPT_TOW32_XVOID_(decl,attr,Treturn_nexcept,cc,name,param,asmname,code)                            __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W32(asmname),code)
#define __XREDIRECT_EXCEPT_TOW32_VOID(decl,attr,cc,name,param,code)                                                      __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W32(name),code)
#define __XREDIRECT_EXCEPT_TOW32_VOID_(decl,attr,cc,name,param,asmname,code)                                             __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_W32(asmname),code)
#define __REDIRECT_EXCEPT_W16                                                                                            __REDIRECT_EXCEPT_TOW16_
#define __REDIRECT_EXCEPT_W32                                                                                            __REDIRECT_EXCEPT_TOW32_
#define __REDIRECT_EXCEPT_W16_XVOID                                                                                      __REDIRECT_EXCEPT_TOW16_XVOID_
#define __REDIRECT_EXCEPT_W32_XVOID                                                                                      __REDIRECT_EXCEPT_TOW32_XVOID_
#define __REDIRECT_EXCEPT_W16_VOID                                                                                       __REDIRECT_EXCEPT_TOW16_VOID_
#define __REDIRECT_EXCEPT_W32_VOID                                                                                       __REDIRECT_EXCEPT_TOW32_VOID_
#define __VREDIRECT_EXCEPT_W16                                                                                           __VREDIRECT_EXCEPT_TOW16_
#define __VREDIRECT_EXCEPT_W32                                                                                           __VREDIRECT_EXCEPT_TOW32_
#define __VREDIRECT_EXCEPT_W16_XVOID                                                                                     __VREDIRECT_EXCEPT_TOW16_XVOID_
#define __VREDIRECT_EXCEPT_W32_XVOID                                                                                     __VREDIRECT_EXCEPT_TOW32_XVOID_
#define __VREDIRECT_EXCEPT_W16_VOID                                                                                      __VREDIRECT_EXCEPT_TOW16_VOID_
#define __VREDIRECT_EXCEPT_W32_VOID                                                                                      __VREDIRECT_EXCEPT_TOW32_VOID_
#define __XREDIRECT_EXCEPT_W16                                                                                           __XREDIRECT_EXCEPT_TOW16_
#define __XREDIRECT_EXCEPT_W32                                                                                           __XREDIRECT_EXCEPT_TOW32_
#define __XREDIRECT_EXCEPT_W16_XVOID                                                                                     __XREDIRECT_EXCEPT_TOW16_XVOID_
#define __XREDIRECT_EXCEPT_W32_XVOID                                                                                     __XREDIRECT_EXCEPT_TOW32_XVOID_
#define __XREDIRECT_EXCEPT_W16_VOID                                                                                      __XREDIRECT_EXCEPT_TOW16_VOID_
#define __XREDIRECT_EXCEPT_W32_VOID                                                                                      __XREDIRECT_EXCEPT_TOW32_VOID_
#ifdef __PE__
#define __REDIRECT_EXCEPT           __REDIRECT_EXCEPT_TOW16
#define __REDIRECT_EXCEPT_          __REDIRECT_EXCEPT_TOW16_
#define __REDIRECT_EXCEPT_XVOID     __REDIRECT_EXCEPT_TOW16_XVOID
#define __REDIRECT_EXCEPT_XVOID_    __REDIRECT_EXCEPT_TOW16_XVOID_
#define __REDIRECT_EXCEPT_VOID      __REDIRECT_EXCEPT_TOW16_VOID
#define __REDIRECT_EXCEPT_VOID_     __REDIRECT_EXCEPT_TOW16_VOID_
#define __VREDIRECT_EXCEPT          __VREDIRECT_EXCEPT_TOW16
#define __VREDIRECT_EXCEPT_         __VREDIRECT_EXCEPT_TOW16_
#define __VREDIRECT_EXCEPT_XVOID    __VREDIRECT_EXCEPT_TOW16_XVOID
#define __VREDIRECT_EXCEPT_XVOID_   __VREDIRECT_EXCEPT_TOW16_XVOID_
#define __VREDIRECT_EXCEPT_VOID     __VREDIRECT_EXCEPT_TOW16_VOID
#define __VREDIRECT_EXCEPT_VOID_    __VREDIRECT_EXCEPT_TOW16_VOID_
#define __XREDIRECT_EXCEPT          __XREDIRECT_EXCEPT_TOW16
#define __XREDIRECT_EXCEPT_         __XREDIRECT_EXCEPT_TOW16_
#define __XREDIRECT_EXCEPT_XVOID    __XREDIRECT_EXCEPT_TOW16_XVOID
#define __XREDIRECT_EXCEPT_XVOID_   __XREDIRECT_EXCEPT_TOW16_XVOID_
#define __XREDIRECT_EXCEPT_VOID     __XREDIRECT_EXCEPT_TOW16_VOID
#define __XREDIRECT_EXCEPT_VOID_    __XREDIRECT_EXCEPT_TOW16_VOID_
#else /* __PE__ */
#define __REDIRECT_EXCEPT           __REDIRECT_EXCEPT_TOW32
#define __REDIRECT_EXCEPT_          __REDIRECT_EXCEPT_TOW32_
#define __REDIRECT_EXCEPT_XVOID     __REDIRECT_EXCEPT_TOW32_XVOID
#define __REDIRECT_EXCEPT_XVOID_    __REDIRECT_EXCEPT_TOW32_XVOID_
#define __REDIRECT_EXCEPT_VOID      __REDIRECT_EXCEPT_TOW32_VOID
#define __REDIRECT_EXCEPT_VOID_     __REDIRECT_EXCEPT_TOW32_VOID_
#define __VREDIRECT_EXCEPT          __VREDIRECT_EXCEPT_TOW32
#define __VREDIRECT_EXCEPT_         __VREDIRECT_EXCEPT_TOW32_
#define __VREDIRECT_EXCEPT_XVOID    __VREDIRECT_EXCEPT_TOW32_XVOID
#define __VREDIRECT_EXCEPT_XVOID_   __VREDIRECT_EXCEPT_TOW32_XVOID_
#define __VREDIRECT_EXCEPT_VOID     __VREDIRECT_EXCEPT_TOW32_VOID
#define __VREDIRECT_EXCEPT_VOID_    __VREDIRECT_EXCEPT_TOW32_VOID_
#define __XREDIRECT_EXCEPT          __XREDIRECT_EXCEPT_TOW32
#define __XREDIRECT_EXCEPT_         __XREDIRECT_EXCEPT_TOW32_
#define __XREDIRECT_EXCEPT_XVOID    __XREDIRECT_EXCEPT_TOW32_XVOID
#define __XREDIRECT_EXCEPT_XVOID_   __XREDIRECT_EXCEPT_TOW32_XVOID_
#define __XREDIRECT_EXCEPT_VOID     __XREDIRECT_EXCEPT_TOW32_VOID
#define __XREDIRECT_EXCEPT_VOID_    __XREDIRECT_EXCEPT_TOW32_VOID_
#endif /* !__PE__ */
#else /* __USE_EXCEPT_API */
#define __IF_USE_EXCEPT(x)      /* nothing */
#define __IF_NUSE_EXCEPT(x)     x
#define __EXCEPT_SELECT(tt,ff)  ff
#define __REDIRECT_EXCEPT_TOW16           __REDIRECT_TODOSW16
#define __REDIRECT_EXCEPT_TOW16_          __REDIRECT_TODOSW16_
#define __REDIRECT_EXCEPT_TOW16_XVOID     __REDIRECT_TODOSW16
#define __REDIRECT_EXCEPT_TOW16_XVOID_    __REDIRECT_TODOSW16_
#define __REDIRECT_EXCEPT_TOW16_VOID      __REDIRECT_TODOSW16_VOID
#define __REDIRECT_EXCEPT_TOW16_VOID_     __REDIRECT_TODOSW16_VOID_
#define __VREDIRECT_EXCEPT_TOW16          __VREDIRECT_TODOSW16
#define __VREDIRECT_EXCEPT_TOW16_         __VREDIRECT_TODOSW16_
#define __VREDIRECT_EXCEPT_TOW16_XVOID    __VREDIRECT_TODOSW16
#define __VREDIRECT_EXCEPT_TOW16_XVOID_   __VREDIRECT_TODOSW16_
#define __VREDIRECT_EXCEPT_TOW16_VOID     __VREDIRECT_TODOSW16_VOID
#define __VREDIRECT_EXCEPT_TOW16_VOID_    __VREDIRECT_TODOSW16_VOID_
#define __XREDIRECT_EXCEPT_TOW16          __XREDIRECT_TODOSW16
#define __XREDIRECT_EXCEPT_TOW16_         __XREDIRECT_TODOSW16_
#define __XREDIRECT_EXCEPT_TOW16_XVOID    __XREDIRECT_TODOSW16
#define __XREDIRECT_EXCEPT_TOW16_XVOID_   __XREDIRECT_TODOSW16_
#define __XREDIRECT_EXCEPT_TOW16_VOID     __XREDIRECT_TODOSW16_VOID
#define __XREDIRECT_EXCEPT_TOW16_VOID_    __XREDIRECT_TODOSW16_VOID_
#define __REDIRECT_EXCEPT_TOW32           __REDIRECT_TOKOSW32
#define __REDIRECT_EXCEPT_TOW32_          __REDIRECT_TOKOSW32_
#define __REDIRECT_EXCEPT_TOW32_XVOID     __REDIRECT_TOKOSW32
#define __REDIRECT_EXCEPT_TOW32_XVOID_    __REDIRECT_TOKOSW32_
#define __REDIRECT_EXCEPT_TOW32_VOID      __REDIRECT_TOKOSW32_VOID
#define __REDIRECT_EXCEPT_TOW32_VOID_     __REDIRECT_TOKOSW32_VOID_
#define __VREDIRECT_EXCEPT_TOW32          __VREDIRECT_TOKOSW32
#define __VREDIRECT_EXCEPT_TOW32_         __VREDIRECT_TOKOSW32_
#define __VREDIRECT_EXCEPT_TOW32_XVOID    __VREDIRECT_TOKOSW32
#define __VREDIRECT_EXCEPT_TOW32_XVOID_   __VREDIRECT_TOKOSW32_
#define __VREDIRECT_EXCEPT_TOW32_VOID     __VREDIRECT_TOKOSW32_VOID
#define __VREDIRECT_EXCEPT_TOW32_VOID_    __VREDIRECT_TOKOSW32_VOID_
#define __XREDIRECT_EXCEPT_TOW32          __XREDIRECT_TOKOSW32
#define __XREDIRECT_EXCEPT_TOW32_         __XREDIRECT_TOKOSW32_
#define __XREDIRECT_EXCEPT_TOW32_XVOID    __XREDIRECT_TOKOSW32
#define __XREDIRECT_EXCEPT_TOW32_XVOID_   __XREDIRECT_TOKOSW32_
#define __XREDIRECT_EXCEPT_TOW32_VOID     __XREDIRECT_TOKOSW32_VOID
#define __XREDIRECT_EXCEPT_TOW32_VOID_    __XREDIRECT_TOKOSW32_VOID_
#define __REDIRECT_EXCEPT_W16             __REDIRECT_W16
#define __REDIRECT_EXCEPT_W32             __REDIRECT_W32
#define __REDIRECT_EXCEPT_W16_XVOID       __REDIRECT_W16
#define __REDIRECT_EXCEPT_W32_XVOID       __REDIRECT_W32
#define __REDIRECT_EXCEPT_W16_VOID        __REDIRECT_W16_VOID
#define __REDIRECT_EXCEPT_W32_VOID        __REDIRECT_W32_VOID
#define __VREDIRECT_EXCEPT_W16            __VREDIRECT_W16
#define __VREDIRECT_EXCEPT_W32            __VREDIRECT_W32
#define __VREDIRECT_EXCEPT_W16_XVOID      __VREDIRECT_W16
#define __VREDIRECT_EXCEPT_W32_XVOID      __VREDIRECT_W32
#define __VREDIRECT_EXCEPT_W16_VOID       __VREDIRECT_W16_VOID
#define __VREDIRECT_EXCEPT_W32_VOID       __VREDIRECT_W32_VOID
#define __XREDIRECT_EXCEPT_W16            __XREDIRECT_W16
#define __XREDIRECT_EXCEPT_W32            __XREDIRECT_W32
#define __XREDIRECT_EXCEPT_W16_XVOID      __XREDIRECT_W16
#define __XREDIRECT_EXCEPT_W32_XVOID      __XREDIRECT_W32
#define __XREDIRECT_EXCEPT_W16_VOID       __XREDIRECT_W16_VOID
#define __XREDIRECT_EXCEPT_W32_VOID       __XREDIRECT_W32_VOID
#define __REDIRECT_EXCEPT(decl,attr,Treturn,cc,name,param,args)                                  __NOREDIRECT(decl,attr,Treturn,cc,name,param,...,args)
#define __REDIRECT_EXCEPT_                __REDIRECT
#define __REDIRECT_EXCEPT_XVOID(decl,attr,Treturn,cc,name,param,args)                            __NOREDIRECT(decl,attr,Treturn,cc,name,param,...,args)
#define __REDIRECT_EXCEPT_XVOID_          __REDIRECT
#define __REDIRECT_EXCEPT_VOID(decl,attr,cc,name,param,args)                                     __NOREDIRECT_VOID(decl,attr,Treturn,cc,name,param,...,args)
#define __REDIRECT_EXCEPT_VOID_           __REDIRECT_VOID
#define __VREDIRECT_EXCEPT(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VNOREDIRECT(decl,attr,Treturn,cc,name,param,...,vasmnamef,args,before_va_start)
#define __VREDIRECT_EXCEPT_               __VREDIRECT
#define __VREDIRECT_EXCEPT_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VNOREDIRECT(decl,attr,Treturn,cc,name,param,...,vasmnamef,args,before_va_start)
#define __VREDIRECT_EXCEPT_XVOID_         __VREDIRECT
#define __VREDIRECT_EXCEPT_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VNOREDIRECT_VOID(decl,attr,cc,name,param,...,vasmnamef,args,before_va_start)
#define __VREDIRECT_EXCEPT_VOID_          __VREDIRECT_VOID
#define __XREDIRECT_EXCEPT(decl,attr,Treturn,cc,name,param,code)                                 __XNOREDIRECT(decl,attr,Treturn,cc,name,param,...,code)
#define __XREDIRECT_EXCEPT_               __XREDIRECT
#define __XREDIRECT_EXCEPT_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XNOREDIRECT(decl,attr,Treturn,cc,name,param,...,code)
#define __XREDIRECT_EXCEPT_XVOID_         __XREDIRECT
#define __XREDIRECT_EXCEPT_VOID(decl,attr,cc,name,param,code)                                    __XNOREDIRECT_VOID(decl,attr,cc,name,param,...,code)
#define __XREDIRECT_EXCEPT_VOID_          __XREDIRECT_VOID
#endif /* !__USE_EXCEPT_API */
#endif /* !__REDIRECT_EXCEPT */
#define __XATTR_RETNONNULL __IF_USE_EXCEPT(__ATTR_RETNONNULL)
#define __XATTR_NORETURN   __IF_USE_EXCEPT(__ATTR_NORETURN)


#ifndef __REDIRECT_UFS
#ifdef __USE_DOSFS
#define __SYMNAME_UFS                      __SYMNAME_DOS
#define __SYMNAME_UFSW16                   __SYMNAME_DOSW16
#define __SYMNAME_UFSW32                   __SYMNAME_DOSW32
#define __REDIRECT_UFS                     __REDIRECT_TODOS
#define __REDIRECT_UFS_                    __REDIRECT_TODOS_
#define __REDIRECT_UFS_VOID                __REDIRECT_TODOS_VOID
#define __REDIRECT_UFS_VOID_               __REDIRECT_TODOS_VOID_
#define __REDIRECT_UFS_NOTHROW             __REDIRECT_TODOS_NOTHROW
#define __REDIRECT_UFS_NOTHROW_            __REDIRECT_TODOS_NOTHROW_
#define __REDIRECT_UFS_VOID_NOTHROW        __REDIRECT_TODOS_VOID_NOTHROW
#define __REDIRECT_UFS_VOID_NOTHROW_       __REDIRECT_TODOS_VOID_NOTHROW_
#define __VREDIRECT_UFS                    __VREDIRECT_TODOS
#define __VREDIRECT_UFS_                   __VREDIRECT_TODOS_
#define __VREDIRECT_UFS_VOID               __VREDIRECT_TODOS_VOID
#define __VREDIRECT_UFS_VOID_              __VREDIRECT_TODOS_VOID_
#define __VREDIRECT_UFS_NOTHROW            __VREDIRECT_TODOS_NOTHROW
#define __VREDIRECT_UFS_NOTHROW_           __VREDIRECT_TODOS_NOTHROW_
#define __VREDIRECT_UFS_VOID_NOTHROW       __VREDIRECT_TODOS_VOID_NOTHROW
#define __VREDIRECT_UFS_VOID_NOTHROW_      __VREDIRECT_TODOS_VOID_NOTHROW_
#define __XREDIRECT_UFS                    __XREDIRECT_TODOS
#define __XREDIRECT_UFS_                   __XREDIRECT_TODOS_
#define __XREDIRECT_UFS_VOID               __XREDIRECT_TODOS_VOID
#define __XREDIRECT_UFS_VOID_              __XREDIRECT_TODOS_VOID_
#define __XREDIRECT_UFS_NOTHROW            __XREDIRECT_TODOS_NOTHROW
#define __XREDIRECT_UFS_NOTHROW_           __XREDIRECT_TODOS_NOTHROW_
#define __XREDIRECT_UFS_VOID_NOTHROW       __XREDIRECT_TODOS_VOID_NOTHROW
#define __XREDIRECT_UFS_VOID_NOTHROW_      __XREDIRECT_TODOS_VOID_NOTHROW_
#define __REDIRECT_UFSW16                  __REDIRECT_TODOSW16_
#define __REDIRECT_UFSW32                  __REDIRECT_TODOSW32_
#define __REDIRECT_UFSW16_VOID             __REDIRECT_TODOSW16_VOID_
#define __REDIRECT_UFSW32_VOID             __REDIRECT_TODOSW32_VOID_
#define __REDIRECT_UFSW16_NOTHROW          __REDIRECT_TODOSW16_NOTHROW_
#define __REDIRECT_UFSW32_NOTHROW          __REDIRECT_TODOSW32_NOTHROW_
#define __REDIRECT_UFSW16_VOID_NOTHROW     __REDIRECT_TODOSW16_VOID_NOTHROW_
#define __REDIRECT_UFSW32_VOID_NOTHROW     __REDIRECT_TODOSW32_VOID_NOTHROW_
#define __VREDIRECT_UFSW16                 __VREDIRECT_TODOSW16_
#define __VREDIRECT_UFSW32                 __VREDIRECT_TODOSW32_
#define __VREDIRECT_UFSW16_VOID            __VREDIRECT_TODOSW16_VOID_
#define __VREDIRECT_UFSW32_VOID            __VREDIRECT_TODOSW32_VOID_
#define __VREDIRECT_UFSW16_NOTHROW         __VREDIRECT_TODOSW16_NOTHROW_
#define __VREDIRECT_UFSW32_NOTHROW         __VREDIRECT_TODOSW32_NOTHROW_
#define __VREDIRECT_UFSW16_VOID_NOTHROW    __VREDIRECT_TODOSW16_VOID_NOTHROW_
#define __VREDIRECT_UFSW32_VOID_NOTHROW    __VREDIRECT_TODOSW32_VOID_NOTHROW_
#define __XREDIRECT_UFSW16                 __XREDIRECT_TODOSW16_
#define __XREDIRECT_UFSW32                 __XREDIRECT_TODOSW32_
#define __XREDIRECT_UFSW16_VOID            __XREDIRECT_TODOSW16_VOID_
#define __XREDIRECT_UFSW32_VOID            __XREDIRECT_TODOSW32_VOID_
#define __XREDIRECT_UFSW16_NOTHROW         __XREDIRECT_TODOSW16_NOTHROW_
#define __XREDIRECT_UFSW32_NOTHROW         __XREDIRECT_TODOSW32_NOTHROW_
#define __XREDIRECT_UFSW16_VOID_NOTHROW    __XREDIRECT_TODOSW16_VOID_NOTHROW_
#define __XREDIRECT_UFSW32_VOID_NOTHROW    __XREDIRECT_TODOSW32_VOID_NOTHROW_
#define __REDIRECT_UFSDPW16                __REDIRECT_TODOSDPW16_
#define __REDIRECT_UFSDPW16_VOID           __REDIRECT_TODOSDPW16_VOID_
#define __REDIRECT_UFSDPW16_NOTHROW        __REDIRECT_TODOSDPW16_NOTHROW_
#define __REDIRECT_UFSDPW16_VOID_NOTHROW   __REDIRECT_TODOSDPW16_VOID_NOTHROW_
#define __VREDIRECT_UFSDPW16               __VREDIRECT_TODOSDPW16_
#define __VREDIRECT_UFSDPW16_VOID          __VREDIRECT_TODOSDPW16_VOID_
#define __VREDIRECT_UFSDPW16_NOTHROW       __VREDIRECT_TODOSDPW16_NOTHROW_
#define __VREDIRECT_UFSDPW16_VOID_NOTHROW  __VREDIRECT_TODOSDPW16_VOID_NOTHROW_
#define __XREDIRECT_UFSDPW16               __XREDIRECT_TODOSDPW16_
#define __XREDIRECT_UFSDPW16_VOID          __XREDIRECT_TODOSDPW16_VOID_
#define __XREDIRECT_UFSDPW16_NOTHROW       __XREDIRECT_TODOSDPW16_NOTHROW_
#define __XREDIRECT_UFSDPW16_VOID_NOTHROW  __XREDIRECT_TODOSDPW16_VOID_NOTHROW_
#define __REDIRECT_EXCEPT_UFS              __REDIRECT_TODOS
#define __REDIRECT_EXCEPT_UFS_             __REDIRECT_TODOS_
#define __REDIRECT_EXCEPT_UFS_VOID         __REDIRECT_TODOS_VOID
#define __REDIRECT_EXCEPT_UFS_VOID_        __REDIRECT_TODOS_VOID_
#define __REDIRECT_EXCEPT_UFS_XVOID        __REDIRECT_TODOS
#define __REDIRECT_EXCEPT_UFS_XVOID_       __REDIRECT_TODOS_
#define __VREDIRECT_EXCEPT_UFS             __VREDIRECT_TODOS
#define __VREDIRECT_EXCEPT_UFS_            __VREDIRECT_TODOS_
#define __VREDIRECT_EXCEPT_UFS_VOID        __VREDIRECT_TODOS_VOID
#define __VREDIRECT_EXCEPT_UFS_VOID_       __VREDIRECT_TODOS_VOID_
#define __VREDIRECT_EXCEPT_UFS_XVOID       __VREDIRECT_TODOS
#define __VREDIRECT_EXCEPT_UFS_XVOID_      __VREDIRECT_TODOS_
#define __XREDIRECT_EXCEPT_UFS             __XREDIRECT_TODOS
#define __XREDIRECT_EXCEPT_UFS_            __XREDIRECT_TODOS_
#define __XREDIRECT_EXCEPT_UFS_VOID        __XREDIRECT_TODOS_VOID
#define __XREDIRECT_EXCEPT_UFS_VOID_       __XREDIRECT_TODOS_VOID_
#define __XREDIRECT_EXCEPT_UFS_XVOID       __XREDIRECT_TODOS
#define __XREDIRECT_EXCEPT_UFS_XVOID_      __XREDIRECT_TODOS_
#define __REDIRECT_EXCEPT_UFSW16           __REDIRECT_TODOSW16_
#define __REDIRECT_EXCEPT_UFSW32           __REDIRECT_TODOSW32_
#define __REDIRECT_EXCEPT_UFSW16_VOID      __REDIRECT_TODOSW16_VOID_
#define __REDIRECT_EXCEPT_UFSW32_VOID      __REDIRECT_TODOSW32_VOID_
#define __REDIRECT_EXCEPT_UFSW16_XVOID     __REDIRECT_TODOSW16_
#define __REDIRECT_EXCEPT_UFSW32_XVOID     __REDIRECT_TODOSW32_
#define __VREDIRECT_EXCEPT_UFSW16          __VREDIRECT_TODOSW16_
#define __VREDIRECT_EXCEPT_UFSW32          __VREDIRECT_TODOSW32_
#define __VREDIRECT_EXCEPT_UFSW16_VOID     __VREDIRECT_TODOSW16_VOID_
#define __VREDIRECT_EXCEPT_UFSW32_VOID     __VREDIRECT_TODOSW32_VOID_
#define __VREDIRECT_EXCEPT_UFSW16_XVOID    __VREDIRECT_TODOSW16_
#define __VREDIRECT_EXCEPT_UFSW32_XVOID    __VREDIRECT_TODOSW32_
#define __XREDIRECT_EXCEPT_UFSW16          __XREDIRECT_TODOSW16_
#define __XREDIRECT_EXCEPT_UFSW32          __XREDIRECT_TODOSW32_
#define __XREDIRECT_EXCEPT_UFSW16_VOID     __XREDIRECT_TODOSW16_VOID_
#define __XREDIRECT_EXCEPT_UFSW32_VOID     __XREDIRECT_TODOSW32_VOID_
#define __XREDIRECT_EXCEPT_UFSW16_XVOID    __XREDIRECT_TODOSW16_
#define __XREDIRECT_EXCEPT_UFSW32_XVOID    __XREDIRECT_TODOSW32_
#define __REDIRECT_EXCEPT_UFSDPW16         __REDIRECT_TODOSDPW16_
#define __REDIRECT_EXCEPT_UFSDPW16_VOID    __REDIRECT_TODOSDPW16_VOID_
#define __REDIRECT_EXCEPT_UFSDPW16_XVOID   __REDIRECT_TODOSDPW16_
#define __VREDIRECT_EXCEPT_UFSDPW16        __VREDIRECT_TODOSDPW16_
#define __VREDIRECT_EXCEPT_UFSDPW16_VOID   __VREDIRECT_TODOSDPW16_VOID_
#define __VREDIRECT_EXCEPT_UFSDPW16_XVOID  __VREDIRECT_TODOSDPW16_
#define __XREDIRECT_EXCEPT_UFSDPW16        __XREDIRECT_TODOSDPW16_
#define __XREDIRECT_EXCEPT_UFSDPW16_VOID   __XREDIRECT_TODOSDPW16_VOID_
#define __XREDIRECT_EXCEPT_UFSDPW16_XVOID  __XREDIRECT_TODOSDPW16_
#else /* __USE_DOSFS */
#define __SYMNAME_UFS                      __SYMNAME_KOS
#define __SYMNAME_UFSW16                   __SYMNAME_KOSW16
#define __SYMNAME_UFSW32                   __SYMNAME_KOSW32
#define __REDIRECT_UFS                     __REDIRECT_TOKOS
#define __REDIRECT_UFS_                    __REDIRECT_TOKOS_
#define __REDIRECT_UFS_VOID                __REDIRECT_TOKOS_VOID
#define __REDIRECT_UFS_VOID_               __REDIRECT_TOKOS_VOID_
#define __REDIRECT_UFS_NOTHROW             __REDIRECT_TOKOS_NOTHROW
#define __REDIRECT_UFS_NOTHROW_            __REDIRECT_TOKOS_NOTHROW_
#define __REDIRECT_UFS_VOID_NOTHROW        __REDIRECT_TOKOS_VOID_NOTHROW
#define __REDIRECT_UFS_VOID_NOTHROW_       __REDIRECT_TOKOS_VOID_NOTHROW_
#define __VREDIRECT_UFS                    __VREDIRECT_TOKOS
#define __VREDIRECT_UFS_                   __VREDIRECT_TOKOS_
#define __VREDIRECT_UFS_VOID               __VREDIRECT_TOKOS_VOID
#define __VREDIRECT_UFS_VOID_              __VREDIRECT_TOKOS_VOID_
#define __VREDIRECT_UFS_NOTHROW            __VREDIRECT_TOKOS_NOTHROW
#define __VREDIRECT_UFS_NOTHROW_           __VREDIRECT_TOKOS_NOTHROW_
#define __VREDIRECT_UFS_VOID_NOTHROW       __VREDIRECT_TOKOS_VOID_NOTHROW
#define __VREDIRECT_UFS_VOID_NOTHROW_      __VREDIRECT_TOKOS_VOID_NOTHROW_
#define __XREDIRECT_UFS                    __XREDIRECT_TOKOS
#define __XREDIRECT_UFS_                   __XREDIRECT_TOKOS_
#define __XREDIRECT_UFS_VOID               __XREDIRECT_TOKOS_VOID
#define __XREDIRECT_UFS_VOID_              __XREDIRECT_TOKOS_VOID_
#define __XREDIRECT_UFS_NOTHROW            __XREDIRECT_TOKOS_NOTHROW
#define __XREDIRECT_UFS_NOTHROW_           __XREDIRECT_TOKOS_NOTHROW_
#define __XREDIRECT_UFS_VOID_NOTHROW       __XREDIRECT_TOKOS_VOID_NOTHROW
#define __XREDIRECT_UFS_VOID_NOTHROW_      __XREDIRECT_TOKOS_VOID_NOTHROW_
#define __REDIRECT_UFSW16                  __REDIRECT_TOKOSW16_
#define __REDIRECT_UFSW32                  __REDIRECT_TOKOSW32_
#define __REDIRECT_UFSW16_VOID             __REDIRECT_TOKOSW16_VOID_
#define __REDIRECT_UFSW32_VOID             __REDIRECT_TOKOSW32_VOID_
#define __REDIRECT_UFSW16_NOTHROW          __REDIRECT_TOKOSW16_NOTHROW_
#define __REDIRECT_UFSW32_NOTHROW          __REDIRECT_TOKOSW32_NOTHROW_
#define __REDIRECT_UFSW16_VOID_NOTHROW     __REDIRECT_TOKOSW16_VOID_NOTHROW_
#define __REDIRECT_UFSW32_VOID_NOTHROW     __REDIRECT_TOKOSW32_VOID_NOTHROW_
#define __VREDIRECT_UFSW16                 __VREDIRECT_TOKOSW16_
#define __VREDIRECT_UFSW32                 __VREDIRECT_TOKOSW32_
#define __VREDIRECT_UFSW16_VOID            __VREDIRECT_TOKOSW16_VOID_
#define __VREDIRECT_UFSW32_VOID            __VREDIRECT_TOKOSW32_VOID_
#define __VREDIRECT_UFSW16_NOTHROW         __VREDIRECT_TOKOSW16_NOTHROW_
#define __VREDIRECT_UFSW32_NOTHROW         __VREDIRECT_TOKOSW32_NOTHROW_
#define __VREDIRECT_UFSW16_VOID_NOTHROW    __VREDIRECT_TOKOSW16_VOID_NOTHROW_
#define __VREDIRECT_UFSW32_VOID_NOTHROW    __VREDIRECT_TOKOSW32_VOID_NOTHROW_
#define __XREDIRECT_UFSW16                 __XREDIRECT_TOKOSW16_
#define __XREDIRECT_UFSW32                 __XREDIRECT_TOKOSW32_
#define __XREDIRECT_UFSW16_VOID            __XREDIRECT_TOKOSW16_VOID_
#define __XREDIRECT_UFSW32_VOID            __XREDIRECT_TOKOSW32_VOID_
#define __XREDIRECT_UFSW16_NOTHROW         __XREDIRECT_TOKOSW16_NOTHROW_
#define __XREDIRECT_UFSW32_NOTHROW         __XREDIRECT_TOKOSW32_NOTHROW_
#define __XREDIRECT_UFSW16_VOID_NOTHROW    __XREDIRECT_TOKOSW16_VOID_NOTHROW_
#define __XREDIRECT_UFSW32_VOID_NOTHROW    __XREDIRECT_TOKOSW32_VOID_NOTHROW_
#define __REDIRECT_UFSDPW16                __REDIRECT_TOKOSW16_
#define __REDIRECT_UFSDPW16_VOID           __REDIRECT_TOKOSW16_VOID_
#define __REDIRECT_UFSDPW16_NOTHROW        __REDIRECT_TOKOSW16_NOTHROW_
#define __REDIRECT_UFSDPW16_VOID_NOTHROW   __REDIRECT_TOKOSW16_VOID_NOTHROW_
#define __VREDIRECT_UFSDPW16               __VREDIRECT_TOKOSW16_
#define __VREDIRECT_UFSDPW16_VOID          __VREDIRECT_TOKOSW16_VOID_
#define __VREDIRECT_UFSDPW16_NOTHROW       __VREDIRECT_TOKOSW16_NOTHROW_
#define __VREDIRECT_UFSDPW16_VOID_NOTHROW  __VREDIRECT_TOKOSW16_VOID_NOTHROW_
#define __XREDIRECT_UFSDPW16               __XREDIRECT_TOKOSW16_
#define __XREDIRECT_UFSDPW16_VOID          __XREDIRECT_TOKOSW16_VOID_
#define __XREDIRECT_UFSDPW16_NOTHROW       __XREDIRECT_TOKOSW16_NOTHROW_
#define __XREDIRECT_UFSDPW16_VOID_NOTHROW  __XREDIRECT_TOKOSW16_VOID_NOTHROW_
#ifdef __USE_EXCEPT_API
#define __REDIRECT_EXCEPT_UFS              __REDIRECT_EXCEPT
#define __REDIRECT_EXCEPT_UFS_             __REDIRECT_EXCEPT_
#define __REDIRECT_EXCEPT_UFS_VOID         __REDIRECT_EXCEPT_VOID
#define __REDIRECT_EXCEPT_UFS_VOID_        __REDIRECT_EXCEPT_VOID_
#define __REDIRECT_EXCEPT_UFS_XVOID        __REDIRECT_EXCEPT_XVOID
#define __REDIRECT_EXCEPT_UFS_XVOID_       __REDIRECT_EXCEPT_XVOID_
#define __VREDIRECT_EXCEPT_UFS             __VREDIRECT_EXCEPT
#define __VREDIRECT_EXCEPT_UFS_            __VREDIRECT_EXCEPT_
#define __VREDIRECT_EXCEPT_UFS_VOID        __VREDIRECT_EXCEPT_VOID
#define __VREDIRECT_EXCEPT_UFS_VOID_       __VREDIRECT_EXCEPT_VOID_
#define __VREDIRECT_EXCEPT_UFS_XVOID       __VREDIRECT_EXCEPT_XVOID
#define __VREDIRECT_EXCEPT_UFS_XVOID_      __VREDIRECT_EXCEPT_XVOID_
#define __XREDIRECT_EXCEPT_UFS             __XREDIRECT_EXCEPT
#define __XREDIRECT_EXCEPT_UFS_            __XREDIRECT_EXCEPT_
#define __XREDIRECT_EXCEPT_UFS_VOID        __XREDIRECT_EXCEPT_VOID
#define __XREDIRECT_EXCEPT_UFS_VOID_       __XREDIRECT_EXCEPT_VOID_
#define __XREDIRECT_EXCEPT_UFS_XVOID       __XREDIRECT_EXCEPT_XVOID
#define __XREDIRECT_EXCEPT_UFS_XVOID_      __XREDIRECT_EXCEPT_XVOID_
#define __REDIRECT_EXCEPT_UFSW16           __REDIRECT_EXCEPT_TOW16_
#define __REDIRECT_EXCEPT_UFSW32           __REDIRECT_EXCEPT_TOW32_
#define __REDIRECT_EXCEPT_UFSW16_VOID      __REDIRECT_EXCEPT_TOW16_VOID_
#define __REDIRECT_EXCEPT_UFSW32_VOID      __REDIRECT_EXCEPT_TOW32_VOID_
#define __REDIRECT_EXCEPT_UFSW16_XVOID     __REDIRECT_EXCEPT_TOW16_XVOID_
#define __REDIRECT_EXCEPT_UFSW32_XVOID     __REDIRECT_EXCEPT_TOW32_XVOID_
#define __VREDIRECT_EXCEPT_UFSW16          __VREDIRECT_EXCEPT_TOW16_
#define __VREDIRECT_EXCEPT_UFSW32          __VREDIRECT_EXCEPT_TOW32_
#define __VREDIRECT_EXCEPT_UFSW16_VOID     __VREDIRECT_EXCEPT_TOW16_VOID_
#define __VREDIRECT_EXCEPT_UFSW32_VOID     __VREDIRECT_EXCEPT_TOW32_VOID_
#define __VREDIRECT_EXCEPT_UFSW16_XVOID    __VREDIRECT_EXCEPT_TOW16_XVOID_
#define __VREDIRECT_EXCEPT_UFSW32_XVOID    __VREDIRECT_EXCEPT_TOW32_XVOID_
#define __XREDIRECT_EXCEPT_UFSW16          __XREDIRECT_EXCEPT_TOW16_
#define __XREDIRECT_EXCEPT_UFSW32          __XREDIRECT_EXCEPT_TOW32_
#define __XREDIRECT_EXCEPT_UFSW16_VOID     __XREDIRECT_EXCEPT_TOW16_VOID_
#define __XREDIRECT_EXCEPT_UFSW32_VOID     __XREDIRECT_EXCEPT_TOW32_VOID_
#define __XREDIRECT_EXCEPT_UFSW16_XVOID    __XREDIRECT_EXCEPT_TOW16_XVOID_
#define __XREDIRECT_EXCEPT_UFSW32_XVOID    __XREDIRECT_EXCEPT_TOW32_XVOID_
#define __REDIRECT_EXCEPT_UFSDPW16         __REDIRECT_EXCEPT_TOW16_
#define __REDIRECT_EXCEPT_UFSDPW16_VOID    __REDIRECT_EXCEPT_TOW16_VOID_
#define __REDIRECT_EXCEPT_UFSDPW16_XVOID   __REDIRECT_EXCEPT_TOW16_XVOID_
#define __VREDIRECT_EXCEPT_UFSDPW16        __VREDIRECT_EXCEPT_TOW16_
#define __VREDIRECT_EXCEPT_UFSDPW16_VOID   __VREDIRECT_EXCEPT_TOW16_VOID_
#define __VREDIRECT_EXCEPT_UFSDPW16_XVOID  __VREDIRECT_EXCEPT_TOW16_XVOID_
#define __XREDIRECT_EXCEPT_UFSDPW16        __XREDIRECT_EXCEPT_TOW16_
#define __XREDIRECT_EXCEPT_UFSDPW16_VOID   __XREDIRECT_EXCEPT_TOW16_VOID_
#define __XREDIRECT_EXCEPT_UFSDPW16_XVOID  __XREDIRECT_EXCEPT_TOW16_XVOID_
#else /* __USE_EXCEPT_API */
#define __REDIRECT_EXCEPT_UFS              __REDIRECT_TOKOS
#define __REDIRECT_EXCEPT_UFS_             __REDIRECT_TOKOS_
#define __REDIRECT_EXCEPT_UFS_VOID         __REDIRECT_TOKOS_VOID
#define __REDIRECT_EXCEPT_UFS_VOID_        __REDIRECT_TOKOS_VOID_
#define __REDIRECT_EXCEPT_UFS_XVOID        __REDIRECT_TOKOS
#define __REDIRECT_EXCEPT_UFS_XVOID_       __REDIRECT_TOKOS_
#define __VREDIRECT_EXCEPT_UFS             __VREDIRECT_TOKOS
#define __VREDIRECT_EXCEPT_UFS_            __VREDIRECT_TOKOS_
#define __VREDIRECT_EXCEPT_UFS_VOID        __VREDIRECT_TOKOS_VOID
#define __VREDIRECT_EXCEPT_UFS_VOID_       __VREDIRECT_TOKOS_VOID_
#define __VREDIRECT_EXCEPT_UFS_XVOID       __VREDIRECT_TOKOS
#define __VREDIRECT_EXCEPT_UFS_XVOID_      __VREDIRECT_TOKOS_
#define __XREDIRECT_EXCEPT_UFS             __XREDIRECT_TOKOS
#define __XREDIRECT_EXCEPT_UFS_            __XREDIRECT_TOKOS_
#define __XREDIRECT_EXCEPT_UFS_VOID        __XREDIRECT_TOKOS_VOID
#define __XREDIRECT_EXCEPT_UFS_VOID_       __XREDIRECT_TOKOS_VOID_
#define __XREDIRECT_EXCEPT_UFS_XVOID       __XREDIRECT_TOKOS
#define __XREDIRECT_EXCEPT_UFS_XVOID_      __XREDIRECT_TOKOS_
#define __REDIRECT_EXCEPT_UFSW16           __REDIRECT_TOKOSW16_
#define __REDIRECT_EXCEPT_UFSW32           __REDIRECT_TOKOSW32_
#define __REDIRECT_EXCEPT_UFSW16_VOID      __REDIRECT_TOKOSW16_VOID_
#define __REDIRECT_EXCEPT_UFSW32_VOID      __REDIRECT_TOKOSW32_VOID_
#define __REDIRECT_EXCEPT_UFSW16_XVOID     __REDIRECT_TOKOSW16_
#define __REDIRECT_EXCEPT_UFSW32_XVOID     __REDIRECT_TOKOSW32_
#define __VREDIRECT_EXCEPT_UFSW16          __VREDIRECT_TOKOSW16_
#define __VREDIRECT_EXCEPT_UFSW32          __VREDIRECT_TOKOSW32_
#define __VREDIRECT_EXCEPT_UFSW16_VOID     __VREDIRECT_TOKOSW16_VOID_
#define __VREDIRECT_EXCEPT_UFSW32_VOID     __VREDIRECT_TOKOSW32_VOID_
#define __VREDIRECT_EXCEPT_UFSW16_XVOID    __VREDIRECT_TOKOSW16_
#define __VREDIRECT_EXCEPT_UFSW32_XVOID    __VREDIRECT_TOKOSW32_
#define __XREDIRECT_EXCEPT_UFSW16          __XREDIRECT_TOKOSW16_
#define __XREDIRECT_EXCEPT_UFSW32          __XREDIRECT_TOKOSW32_
#define __XREDIRECT_EXCEPT_UFSW16_VOID     __XREDIRECT_TOKOSW16_VOID_
#define __XREDIRECT_EXCEPT_UFSW32_VOID     __XREDIRECT_TOKOSW32_VOID_
#define __XREDIRECT_EXCEPT_UFSW16_XVOID    __XREDIRECT_TOKOSW16_
#define __XREDIRECT_EXCEPT_UFSW32_XVOID    __XREDIRECT_TOKOSW32_
#define __REDIRECT_EXCEPT_UFSDPW16         __REDIRECT_TOKOSW16_
#define __REDIRECT_EXCEPT_UFSDPW16_VOID    __REDIRECT_TOKOSW16_VOID_
#define __REDIRECT_EXCEPT_UFSDPW16_XVOID   __REDIRECT_TOKOSW16_
#define __VREDIRECT_EXCEPT_UFSDPW16        __VREDIRECT_TOKOSW16_
#define __VREDIRECT_EXCEPT_UFSDPW16_VOID   __VREDIRECT_TOKOSW16_VOID_
#define __VREDIRECT_EXCEPT_UFSDPW16_XVOID  __VREDIRECT_TOKOSW16_
#define __XREDIRECT_EXCEPT_UFSDPW16        __XREDIRECT_TOKOSW16_
#define __XREDIRECT_EXCEPT_UFSDPW16_VOID   __XREDIRECT_TOKOSW16_VOID_
#define __XREDIRECT_EXCEPT_UFSDPW16_XVOID  __XREDIRECT_TOKOSW16_
#endif /* !__USE_EXCEPT_API */
#endif /* !__USE_DOSFS */
#endif /* !__REDIRECT_UFS */


/* DP (DosPrefix) redirection:
 *    Many DOS names for UNIX functions are exported with a leading underscore.
 *    In DOS compatibility mode, prepend that underscore and redirect.
 *    >> #ifdef __DOS_COMPAT__
 *    >>     UFSDPA(...open) Define `open'   (linked against `open' / `DOS$_open')
 *    >>     UFSDPB(...open) Define `_open'  (linked against `open' / `DOS$_open')
 *    >> #else
 *    >>     UFSDPA(...open) Define `open'   (linked against `open' / `DOS$_open')
 *    >>     UFSDPB(...open) Define `_open'  (linked against `open' / `DOS$_open')
 *    >> #endif
 */
#ifndef __REDIRECT_DPA
#ifdef __DOS_COMPAT__
#define __SYMNAME_DOSPREFIX(x)        __SYMNAME_DOS(_##x)
#define __REDIRECT_DPA(decl,attr,Treturn,cc,name,param,args)                                        __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPA_VOID(decl,attr,cc,name,param,args)                                           __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPA_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPA_VOID_NOTHROW(decl,attr,cc,name,param,args)                                   __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __VREDIRECT_DPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)             __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPA_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)     __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPA_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)        __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_DPA(decl,attr,Treturn,cc,name,param,code)                                       __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPA_VOID(decl,attr,cc,name,param,code)                                          __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPA_NOTHROW(decl,attr,Treturn,cc,name,param,code)                               __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPA_VOID_NOTHROW(decl,attr,cc,name,param,code)                                  __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#define __SYMNAME_EXCEPT_DOSPREFIX    __SYMNAME_DOSPREFIX
#define __REDIRECT_EXCEPT_DPA(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_DPA_VOID(decl,attr,cc,name,param,args)                                     __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_DPA_XVOID(decl,attr,Treturn,cc,name,param,args)                            __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __VREDIRECT_EXCEPT_DPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_DPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_DPA_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_EXCEPT_DPA(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_DPA_VOID(decl,attr,cc,name,param,code)                                    __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_DPA_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#ifdef __SYMNAME_DOS_IS_SAME
#define __REDIRECT_DPB(decl,attr,Treturn,cc,name,param,args)                                        __NOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,args)
#define __REDIRECT_DPB_VOID(decl,attr,cc,name,param,args)                                           __NOREDIRECT_VOID(decl,attr,cc,_##name,param,...,args)
#define __REDIRECT_DPB_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                __NOREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,...,args)
#define __REDIRECT_DPB_VOID_NOTHROW(decl,attr,cc,name,param,args)                                   __NOREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,...,args)
#define __VREDIRECT_DPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)             __VNOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,...,args,before_va_start)
#define __VREDIRECT_DPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                __VNOREDIRECT_VOID(decl,attr,cc,_##name,param,...,...,args,before_va_start)
#define __VREDIRECT_DPB_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)     __VNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,...,...,args,before_va_start)
#define __VREDIRECT_DPB_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)        __VNOREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,...,...,args,before_va_start)
#define __XREDIRECT_DPB(decl,attr,Treturn,cc,name,param,code)                                       __XNOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,code)
#define __XREDIRECT_DPB_VOID(decl,attr,cc,name,param,code)                                          __XNOREDIRECT_VOID(decl,attr,cc,_##name,param,...,code)
#define __XREDIRECT_DPB_NOTHROW(decl,attr,Treturn,cc,name,param,code)                               __XNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,...,code)
#define __XREDIRECT_DPB_VOID_NOTHROW(decl,attr,cc,name,param,code)                                  __XNOREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,...,code)
#define __REDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,args)                                  __NOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,args)
#define __REDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,args)                                     __NOREDIRECT_VOID(decl,attr,cc,_##name,param,...,args)
#define __REDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,args)                            __NOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,args)
#define __VREDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VNOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,...,args,before_va_start)
#define __VREDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VNOREDIRECT_VOID(decl,attr,cc,_##name,param,...,...,args,before_va_start)
#define __VREDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VNOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,...,args,before_va_start)
#define __XREDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,code)                                 __XNOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,code)
#define __XREDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,code)                                    __XNOREDIRECT_VOID(decl,attr,cc,_##name,param,...,code)
#define __XREDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XNOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,code)
#else /* __SYMNAME_DOS_IS_SAME */
#define __REDIRECT_DPB(decl,attr,Treturn,cc,name,param,args)                                        __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPB_VOID(decl,attr,cc,name,param,args)                                           __REDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPB_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                __REDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPB_VOID_NOTHROW(decl,attr,cc,name,param,args)                                   __REDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __VREDIRECT_DPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)             __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                __VREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPB_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)     __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPB_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)        __VREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_DPB(decl,attr,Treturn,cc,name,param,code)                                       __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPB_VOID(decl,attr,cc,name,param,code)                                          __XREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPB_NOTHROW(decl,attr,Treturn,cc,name,param,code)                               __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPB_VOID_NOTHROW(decl,attr,cc,name,param,code)                                  __XREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#define __REDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,args)                                     __REDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,args)                            __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __VREDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,code)                                    __XREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#endif /* !__SYMNAME_DOS_IS_SAME */
#else /* __DOS_COMPAT__ */
#define __SYMNAME_DOSPREFIX        __SYMNAME_KOS
#ifdef __SYMNAME_KOS_IS_SAME
#define __SYMNAME_DOSPREFIX_IS_SAME 1
#endif /* __SYMNAME_KOS_IS_SAME */
#define __REDIRECT_DPB(decl,attr,Treturn,cc,name,param,args)                                        __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPB_VOID(decl,attr,cc,name,param,args)                                           __REDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPB_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                __REDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPB_VOID_NOTHROW(decl,attr,cc,name,param,args)                                   __REDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __VREDIRECT_DPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)             __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                __VREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPB_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)     __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPB_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)        __VREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_DPB(decl,attr,Treturn,cc,name,param,code)                                       __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPB_VOID(decl,attr,cc,name,param,code)                                          __XREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPB_NOTHROW(decl,attr,Treturn,cc,name,param,code)                               __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPB_VOID_NOTHROW(decl,attr,cc,name,param,code)                                  __XREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#ifdef __USE_EXCEPT_API
#define __SYMNAME_EXCEPT_DOSPREFIX  __SYMNAME_EXCEPT
#define __REDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,args)                                     __REDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,args)                            __REDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),args)
#define __VREDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),__SYMNAME_EXCEPT_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),__SYMNAME_EXCEPT_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),__SYMNAME_EXCEPT_DOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,code)                                    __XREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),code)
#else /* __USE_EXCEPT_API */
#define __SYMNAME_EXCEPT_DOSPREFIX  __SYMNAME_KOS
#define __REDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,args)                                     __REDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,args)                            __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),args)
#define __VREDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_EXCEPT_DPB(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_DPB_VOID(decl,attr,cc,name,param,code)                                    __XREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_DPB_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_DOSPREFIX(name),code)
#endif /* !__USE_EXCEPT_API */
#ifdef __SYMNAME_DOSPREFIX_IS_SAME
#define __REDIRECT_DPA(decl,attr,Treturn,cc,name,param,args)                                        __NOREDIRECT(decl,attr,Treturn,cc,name,param,...,args)
#define __REDIRECT_DPA_VOID(decl,attr,cc,name,param,args)                                           __NOREDIRECT_VOID(decl,attr,cc,name,param,...,args)
#define __REDIRECT_DPA_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                __NOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,...,args)
#define __REDIRECT_DPA_VOID_NOTHROW(decl,attr,cc,name,param,args)                                   __NOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,...,args)
#define __VREDIRECT_DPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)             __VNOREDIRECT(decl,attr,Treturn,cc,name,param,...,...,args,before_va_start)
#define __VREDIRECT_DPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                __VNOREDIRECT_VOID(decl,attr,cc,name,param,...,...,args,before_va_start)
#define __VREDIRECT_DPA_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)     __VNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,...,...,args,before_va_start)
#define __VREDIRECT_DPA_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)        __VNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,...,...,args,before_va_start)
#define __XREDIRECT_DPA(decl,attr,Treturn,cc,name,param,code)                                       __XNOREDIRECT(decl,attr,Treturn,cc,name,param,...,code)
#define __XREDIRECT_DPA_VOID(decl,attr,cc,name,param,code)                                          __XNOREDIRECT_VOID(decl,attr,cc,name,param,...,code)
#define __XREDIRECT_DPA_NOTHROW(decl,attr,Treturn,cc,name,param,code)                               __XNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,...,code)
#define __XREDIRECT_DPA_VOID_NOTHROW(decl,attr,cc,name,param,code)                                  __XNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,...,code)
#ifdef __USE_EXCEPT_API
#define __REDIRECT_EXCEPT_DPA(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_DPA_VOID(decl,attr,cc,name,param,args)                                     __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_DPA_XVOID(decl,attr,Treturn,cc,name,param,args)                            __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),args)
#define __VREDIRECT_EXCEPT_DPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),__SYMNAME_EXCEPT_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_DPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),__SYMNAME_EXCEPT_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_DPA_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),__SYMNAME_EXCEPT_DOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_EXCEPT_DPA(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_DPA_VOID(decl,attr,cc,name,param,code)                                    __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_DPA_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),code)
#else /* __USE_EXCEPT_API */
#define __REDIRECT_EXCEPT_DPA(decl,attr,Treturn,cc,name,param,args)                                  __NOREDIRECT(decl,attr,Treturn,cc,name,param,...,args)
#define __REDIRECT_EXCEPT_DPA_VOID(decl,attr,cc,name,param,args)                                     __NOREDIRECT_VOID(decl,attr,cc,name,param,...,args)
#define __REDIRECT_EXCEPT_DPA_XVOID(decl,attr,Treturn,cc,name,param,args)                            __NOREDIRECT(decl,attr,Treturn,cc,name,param,...,args)
#define __VREDIRECT_EXCEPT_DPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VNOREDIRECT(decl,attr,Treturn,cc,name,param,...,...,args,before_va_start)
#define __VREDIRECT_EXCEPT_DPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VNOREDIRECT_VOID(decl,attr,cc,name,param,...,...,args,before_va_start)
#define __VREDIRECT_EXCEPT_DPA_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VNOREDIRECT(decl,attr,Treturn,cc,name,param,...,...,args,before_va_start)
#define __XREDIRECT_EXCEPT_DPA(decl,attr,Treturn,cc,name,param,code)                                 __XNOREDIRECT(decl,attr,Treturn,cc,name,param,...,code)
#define __XREDIRECT_EXCEPT_DPA_VOID(decl,attr,cc,name,param,code)                                    __XNOREDIRECT_VOID(decl,attr,cc,name,param,...,code)
#define __XREDIRECT_EXCEPT_DPA_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XNOREDIRECT(decl,attr,Treturn,cc,name,param,...,code)
#endif /* !__USE_EXCEPT_API */
#else /* __SYMNAME_DOSPREFIX_IS_SAME */
#define __REDIRECT_DPA(decl,attr,Treturn,cc,name,param,args)                                        __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPA_VOID(decl,attr,cc,name,param,args)                                           __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPA_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPA_VOID_NOTHROW(decl,attr,cc,name,param,args)                                   __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __VREDIRECT_DPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)             __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPA_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)     __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPA_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)        __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_DPA(decl,attr,Treturn,cc,name,param,code)                                       __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPA_VOID(decl,attr,cc,name,param,code)                                          __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPA_NOTHROW(decl,attr,Treturn,cc,name,param,code)                               __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPA_VOID_NOTHROW(decl,attr,cc,name,param,code)                                  __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#ifdef __USE_EXCEPT_API
#define __REDIRECT_DPA(decl,attr,Treturn,cc,name,param,args)                                        __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),args)
#define __REDIRECT_DPA_VOID(decl,attr,cc,name,param,args)                                           __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),args)
#define __REDIRECT_DPA_XVOID(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),args)
#define __VREDIRECT_DPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)             __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),__SYMNAME_EXCEPT_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),__SYMNAME_EXCEPT_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPA_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),__SYMNAME_EXCEPT_DOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_DPA(decl,attr,Treturn,cc,name,param,code)                                       __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),code)
#define __XREDIRECT_DPA_VOID(decl,attr,cc,name,param,code)                                          __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),code)
#define __XREDIRECT_DPA_XVOID(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT_DOSPREFIX(name),code)
#else /* __USE_EXCEPT_API */
#define __REDIRECT_DPA(decl,attr,Treturn,cc,name,param,args)                                        __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPA_VOID(decl,attr,cc,name,param,args)                                           __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __REDIRECT_DPA_XVOID(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),args)
#define __VREDIRECT_DPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)             __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_DPA_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),__SYMNAME_DOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_DPA(decl,attr,Treturn,cc,name,param,code)                                       __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPA_VOID(decl,attr,cc,name,param,code)                                          __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#define __XREDIRECT_DPA_XVOID(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_DOSPREFIX(name),code)
#endif /* !__USE_EXCEPT_API */
#endif /* !__SYMNAME_DOSPREFIX_IS_SAME */
#endif /* !__DOS_COMPAT__ */
#endif /* !__REDIRECT_DPA */

#ifndef __REDIRECT_UFSDPA
#ifdef __USE_DOSFS
#define __SYMNAME_UFSDOSPREFIX(x)          __SYMNAME_DOS(_##x)
#define __REDIRECT_UFSDPA(decl,attr,Treturn,cc,name,param,args)                                     __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __REDIRECT_UFSDPA_VOID(decl,attr,cc,name,param,args)                                        __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __REDIRECT_UFSDPA_NOTHROW(decl,attr,Treturn,cc,name,param,args)                             __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __REDIRECT_UFSDPA_VOID_NOTHROW(decl,attr,cc,name,param,args)                                __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __VREDIRECT_UFSDPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_UFSDPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)             __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_UFSDPA_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)  __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_UFSDPA_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)     __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_UFSDPA(decl,attr,Treturn,cc,name,param,code)                                    __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#define __XREDIRECT_UFSDPA_VOID(decl,attr,cc,name,param,code)                                       __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#define __XREDIRECT_UFSDPA_NOTHROW(decl,attr,Treturn,cc,name,param,code)                            __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#define __XREDIRECT_UFSDPA_VOID_NOTHROW(decl,attr,cc,name,param,code)                               __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#define __SYMNAME_EXCEPT_UFSDOSPREFIX      __SYMNAME_UFSDOSPREFIX
#define __REDIRECT_EXCEPT_UFSDPA(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_UFSDPA_VOID(decl,attr,cc,name,param,args)                                     __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_UFSDPA_XVOID(decl,attr,Treturn,cc,name,param,args)                            __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __VREDIRECT_EXCEPT_UFSDPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPA_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_EXCEPT_UFSDPA(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_UFSDPA_VOID(decl,attr,cc,name,param,code)                                    __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_UFSDPA_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#ifdef __SYMNAME_DOS_IS_SAME
#define __REDIRECT_UFSDPB(decl,attr,Treturn,cc,name,param,args)                                     __NOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,args)
#define __REDIRECT_UFSDPB_VOID(decl,attr,cc,name,param,args)                                        __NOREDIRECT_VOID(decl,attr,cc,_##name,param,...,args)
#define __REDIRECT_UFSDPB_NOTHROW(decl,attr,Treturn,cc,name,param,args)                             __NOREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,...,args)
#define __REDIRECT_UFSDPB_VOID_NOTHROW(decl,attr,cc,name,param,args)                                __NOREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,...,args)
#define __VREDIRECT_UFSDPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)          __VNOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,...,args,before_va_start)
#define __VREDIRECT_UFSDPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)             __VNOREDIRECT_VOID(decl,attr,cc,_##name,param,...,...,args,before_va_start)
#define __VREDIRECT_UFSDPB_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)  __VNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,...,...,args,before_va_start)
#define __VREDIRECT_UFSDPB_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)     __VNOREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,...,...,args,before_va_start)
#define __XREDIRECT_UFSDPB(decl,attr,Treturn,cc,name,param,code)                                    __XNOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,code)
#define __XREDIRECT_UFSDPB_VOID(decl,attr,cc,name,param,code)                                       __XNOREDIRECT_VOID(decl,attr,cc,_##name,param,...,code)
#define __XREDIRECT_UFSDPB_NOTHROW(decl,attr,Treturn,cc,name,param,code)                            __XNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,...,code)
#define __XREDIRECT_UFSDPB_VOID_NOTHROW(decl,attr,cc,name,param,code)                               __XNOREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,...,code)
#define __REDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,args)                                  __NOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,args)
#define __REDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,args)                                     __NOREDIRECT_VOID(decl,attr,cc,_##name,param,...,args)
#define __REDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,args)                            __NOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,args)
#define __VREDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VNOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,...,args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VNOREDIRECT_VOID(decl,attr,cc,_##name,param,...,...,args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VNOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,...,args,before_va_start)
#define __XREDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,code)                                 __XNOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,code)
#define __XREDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,code)                                    __XNOREDIRECT_VOID(decl,attr,cc,_##name,param,...,code)
#define __XREDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XNOREDIRECT(decl,attr,Treturn,cc,_##name,param,...,code)
#else /* __SYMNAME_DOS_IS_SAME */
#define __REDIRECT_UFSDPB(decl,attr,Treturn,cc,name,param,args)                                     __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __REDIRECT_UFSDPB_VOID(decl,attr,cc,name,param,args)                                        __REDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __REDIRECT_UFSDPB_NOTHROW(decl,attr,Treturn,cc,name,param,args)                             __REDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __REDIRECT_UFSDPB_VOID_NOTHROW(decl,attr,cc,name,param,args)                                __REDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __VREDIRECT_UFSDPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_UFSDPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)             __VREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_UFSDPB_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)  __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_UFSDPB_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)     __VREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_UFSDPB(decl,attr,Treturn,cc,name,param,code)                                    __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#define __XREDIRECT_UFSDPB_VOID(decl,attr,cc,name,param,code)                                       __XREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#define __XREDIRECT_UFSDPB_NOTHROW(decl,attr,Treturn,cc,name,param,code)                            __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#define __XREDIRECT_UFSDPB_VOID_NOTHROW(decl,attr,cc,name,param,code)                               __XREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#define __REDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,args)                                     __REDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __REDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,args)                            __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),args)
#define __VREDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),__SYMNAME_UFSDOSPREFIX(vasmnamef),args,before_va_start)
#define __XREDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,code)                                    __XREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#define __XREDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_UFSDOSPREFIX(name),code)
#endif /* !__SYMNAME_DOS_IS_SAME */
#else /* __USE_DOSFS */
#ifdef __DOS_COMPAT__
#warning "DOS compatibility mode with _DOSFS_SOURCE disabled, isn't actually compatible with DOS"
#endif /* __DOS_COMPAT__ */
#define __SYMNAME_UFSDOSPREFIX        __SYMNAME_KOS
#ifdef __SYMNAME_KOS_IS_SAME
#define __SYMNAME_UFSDOSPREFIX_IS_SAME 1
#endif /* __SYMNAME_KOS_IS_SAME */
#define __REDIRECT_UFSDPB(decl,attr,Treturn,cc,name,param,args)                                     __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_KOS(name),args)
#define __REDIRECT_UFSDPB_VOID(decl,attr,cc,name,param,args)                                        __REDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_KOS(name),args)
#define __REDIRECT_UFSDPB_NOTHROW(decl,attr,Treturn,cc,name,param,args)                             __REDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,__SYMNAME_KOS(name),args)
#define __REDIRECT_UFSDPB_VOID_NOTHROW(decl,attr,cc,name,param,args)                                __REDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,__SYMNAME_KOS(name),args)
#define __VREDIRECT_UFSDPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __VREDIRECT_UFSDPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)             __VREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __VREDIRECT_UFSDPB_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)  __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __VREDIRECT_UFSDPB_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)     __VREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __XREDIRECT_UFSDPB(decl,attr,Treturn,cc,name,param,code)                                    __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_KOS(name),code)
#define __XREDIRECT_UFSDPB_VOID(decl,attr,cc,name,param,code)                                       __XREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_KOS(name),code)
#define __XREDIRECT_UFSDPB_NOTHROW(decl,attr,Treturn,cc,name,param,code)                            __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,_##name,param,__SYMNAME_KOS(name),code)
#define __XREDIRECT_UFSDPB_VOID_NOTHROW(decl,attr,cc,name,param,code)                               __XREDIRECT_VOID_NOTHROW(decl,attr,cc,_##name,param,__SYMNAME_KOS(name),code)
#ifdef __USE_EXCEPT_API
#define __SYMNAME_EXCEPT_UFSDOSPREFIX __SYMNAME_EXCEPT
#define __REDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_EXCEPT(name),args)
#define __REDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,args)                                     __REDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_EXCEPT(name),args)
#define __REDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,args)                            __REDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_EXCEPT(name),args)
#define __VREDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_EXCEPT(name),__SYMNAME_EXCEPT(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_EXCEPT(name),__SYMNAME_EXCEPT(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_EXCEPT(name),__SYMNAME_EXCEPT(vasmnamef),args,before_va_start)
#define __XREDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_EXCEPT(name),code)
#define __XREDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,code)                                    __XREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_EXCEPT(name),code)
#define __XREDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_EXCEPT(name),code)
#else /* __USE_EXCEPT_API */
#define __SYMNAME_EXCEPT_UFSDOSPREFIX __SYMNAME_KOS
#define __REDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_KOS(name),args)
#define __REDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,args)                                     __REDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_KOS(name),args)
#define __REDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,args)                            __REDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_KOS(name),args)
#define __VREDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __XREDIRECT_EXCEPT_UFSDPB(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_KOS(name),code)
#define __XREDIRECT_EXCEPT_UFSDPB_VOID(decl,attr,cc,name,param,code)                                    __XREDIRECT_VOID(decl,attr,cc,_##name,param,__SYMNAME_KOS(name),code)
#define __XREDIRECT_EXCEPT_UFSDPB_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XREDIRECT(decl,attr,Treturn,cc,_##name,param,__SYMNAME_KOS(name),code)
#endif /* __USE_EXCEPT_API */
#ifdef __SYMNAME_UFSDOSPREFIX_IS_SAME
#define __REDIRECT_UFSDPA(decl,attr,Treturn,cc,name,param,args)                                     __NOREDIRECT(decl,attr,Treturn,cc,name,param,...,args)
#define __REDIRECT_UFSDPA_VOID(decl,attr,cc,name,param,args)                                        __NOREDIRECT_VOID(decl,attr,cc,name,param,...,args)
#define __REDIRECT_UFSDPA_NOTHROW(decl,attr,Treturn,cc,name,param,args)                             __NOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,...,args)
#define __REDIRECT_UFSDPA_VOID_NOTHROW(decl,attr,cc,name,param,args)                                __NOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,...,args)
#define __VREDIRECT_UFSDPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)          __VNOREDIRECT(decl,attr,Treturn,cc,name,param,...,...,args,before_va_start)
#define __VREDIRECT_UFSDPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)             __VNOREDIRECT_VOID(decl,attr,cc,name,param,...,...,args,before_va_start)
#define __VREDIRECT_UFSDPA_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)  __VNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,...,...,args,before_va_start)
#define __VREDIRECT_UFSDPA_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)     __VNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,...,...,args,before_va_start)
#define __XREDIRECT_UFSDPA(decl,attr,Treturn,cc,name,param,code)                                    __XNOREDIRECT(decl,attr,Treturn,cc,name,param,...,code)
#define __XREDIRECT_UFSDPA_VOID(decl,attr,cc,name,param,code)                                       __XNOREDIRECT_VOID(decl,attr,cc,name,param,...,code)
#define __XREDIRECT_UFSDPA_NOTHROW(decl,attr,Treturn,cc,name,param,code)                            __XNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,...,code)
#define __XREDIRECT_UFSDPA_VOID_NOTHROW(decl,attr,cc,name,param,code)                               __XNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,...,code)
#define __REDIRECT_EXCEPT_UFSDPA                                                                    __REDIRECT_EXCEPT
#define __REDIRECT_EXCEPT_UFSDPA_VOID                                                               __REDIRECT_EXCEPT_VOID
#define __REDIRECT_EXCEPT_UFSDPA_XVOID                                                              __REDIRECT_EXCEPT_XVOID
#define __VREDIRECT_EXCEPT_UFSDPA                                                                   __VREDIRECT_EXCEPT
#define __VREDIRECT_EXCEPT_UFSDPA_VOID                                                              __VREDIRECT_EXCEPT_VOID
#define __VREDIRECT_EXCEPT_UFSDPA_XVOID                                                             __VREDIRECT_EXCEPT_XVOID
#define __XREDIRECT_EXCEPT_UFSDPA                                                                   __XREDIRECT_EXCEPT
#define __XREDIRECT_EXCEPT_UFSDPA_VOID                                                              __XREDIRECT_EXCEPT_VOID
#define __XREDIRECT_EXCEPT_UFSDPA_XVOID                                                             __XREDIRECT_EXCEPT_XVOID
#else /* __SYMNAME_UFSDOSPREFIX_IS_SAME */
#define __REDIRECT_UFSDPA(decl,attr,Treturn,cc,name,param,args)                                     __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOS(name),args)
#define __REDIRECT_UFSDPA_VOID(decl,attr,cc,name,param,args)                                        __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOS(name),args)
#define __REDIRECT_UFSDPA_NOTHROW(decl,attr,Treturn,cc,name,param,args)                             __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOS(name),args)
#define __REDIRECT_UFSDPA_VOID_NOTHROW(decl,attr,cc,name,param,args)                                __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOS(name),args)
#define __VREDIRECT_UFSDPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __VREDIRECT_UFSDPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)             __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __VREDIRECT_UFSDPA_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)  __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __VREDIRECT_UFSDPA_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)     __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __XREDIRECT_UFSDPA(decl,attr,Treturn,cc,name,param,code)                                    __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOS(name),code)
#define __XREDIRECT_UFSDPA_VOID(decl,attr,cc,name,param,code)                                       __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOS(name),code)
#define __XREDIRECT_UFSDPA_NOTHROW(decl,attr,Treturn,cc,name,param,code)                            __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,__SYMNAME_KOS(name),code)
#define __XREDIRECT_UFSDPA_VOID_NOTHROW(decl,attr,cc,name,param,code)                               __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,__SYMNAME_KOS(name),code)
#ifdef __USE_EXCEPT_API
#define __REDIRECT_EXCEPT_UFSDPA(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT(name),args)
#define __REDIRECT_EXCEPT_UFSDPA_VOID(decl,attr,cc,name,param,args)                                     __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT(name),args)
#define __REDIRECT_EXCEPT_UFSDPA_XVOID(decl,attr,Treturn,cc,name,param,args)                            __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT(name),args)
#define __VREDIRECT_EXCEPT_UFSDPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT(name),__SYMNAME_EXCEPT(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT(name),__SYMNAME_EXCEPT(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPA_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT(name),__SYMNAME_EXCEPT(vasmnamef),args,before_va_start)
#define __XREDIRECT_EXCEPT_UFSDPA(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_EXCEPT(name),code)
#define __XREDIRECT_EXCEPT_UFSDPA_VOID(decl,attr,cc,name,param,code)                                    __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT(name),code)
#define __XREDIRECT_EXCEPT_UFSDPA_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_EXCEPT(name),code)
#else /* __USE_EXCEPT_API */
#define __REDIRECT_EXCEPT_UFSDPA(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOS(name),args)
#define __REDIRECT_EXCEPT_UFSDPA_VOID(decl,attr,cc,name,param,args)                                     __REDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOS(name),args)
#define __REDIRECT_EXCEPT_UFSDPA_XVOID(decl,attr,Treturn,cc,name,param,args)                            __REDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOS(name),args)
#define __VREDIRECT_EXCEPT_UFSDPA(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPA_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSDPA_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOS(name),__SYMNAME_KOS(vasmnamef),args,before_va_start)
#define __XREDIRECT_EXCEPT_UFSDPA(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOS(name),code)
#define __XREDIRECT_EXCEPT_UFSDPA_VOID(decl,attr,cc,name,param,code)                                    __XREDIRECT_VOID(decl,attr,cc,name,param,__SYMNAME_KOS(name),code)
#define __XREDIRECT_EXCEPT_UFSDPA_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XREDIRECT(decl,attr,Treturn,cc,name,param,__SYMNAME_KOS(name),code)
#endif /* !__USE_EXCEPT_API */
#endif /* !__SYMNAME_UFSDOSPREFIX_IS_SAME */
#endif /* !__USE_DOSFS */
#endif /* !__REDIRECT_UFSDPA */

#ifndef __REDIRECT_FS64
#ifdef __USE_FILE_OFFSET64
#define __REDIRECT_FS64(decl,attr,Treturn,cc,name,param,args)                                          __REDIRECT(decl,attr,Treturn,cc,name,param,name##64,args)
#define __REDIRECT_FS64_VOID(decl,attr,cc,name,param,args)                                             __REDIRECT_VOID(decl,attr,cc,name,param,name##64,args)
#define __REDIRECT_FS64_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name##64,args)
#define __REDIRECT_FS64_VOID_NOTHROW(decl,attr,cc,name,param,args)                                     __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name##64,args)
#define __VREDIRECT_FS64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)               __VREDIRECT(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_FS64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                  __VREDIRECT_VOID(decl,attr,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_FS64_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_FS64_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __XREDIRECT_FS64(decl,attr,Treturn,cc,name,param,code)                                         __XREDIRECT(decl,attr,Treturn,cc,name,param,name##64,code)
#define __XREDIRECT_FS64_VOID(decl,attr,cc,name,param,code)                                            __XREDIRECT_VOID(decl,attr,cc,name,param,name##64,code)
#define __XREDIRECT_FS64_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name##64,code)
#define __XREDIRECT_FS64_VOID_NOTHROW(decl,attr,cc,name,param,code)                                    __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name##64,code)
#define __REDIRECT_UFS64(decl,attr,Treturn,cc,name,param,args)                                         __REDIRECT_UFS_(decl,attr,Treturn,cc,name,param,name##64,args)
#define __REDIRECT_UFS64_VOID(decl,attr,cc,name,param,args)                                            __REDIRECT_UFS_VOID_(decl,attr,cc,name,param,name##64,args)
#define __REDIRECT_UFS64_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                 __REDIRECT_UFS_NOTHROW_(decl,attr,Treturn,cc,name,param,name##64,args)
#define __REDIRECT_UFS64_VOID_NOTHROW(decl,attr,cc,name,param,args)                                    __REDIRECT_UFS_VOID_NOTHROW_(decl,attr,cc,name,param,name##64,args)
#define __VREDIRECT_UFS64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)              __VREDIRECT_UFS_(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_UFS64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                 __VREDIRECT_UFS_VOID_(decl,attr,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_UFS64_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)      __VREDIRECT_UFS_NOTHROW_(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_UFS64_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)         __VREDIRECT_UFS_VOID_NOTHROW_(decl,attr,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __XREDIRECT_UFS64(decl,attr,Treturn,cc,name,param,code)                                        __XREDIRECT_UFS_(decl,attr,Treturn,cc,name,param,name##64,code)
#define __XREDIRECT_UFS64_VOID(decl,attr,cc,name,param,code)                                           __XREDIRECT_UFS_VOID_(decl,attr,cc,name,param,name##64,code)
#define __XREDIRECT_UFS64_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                __XREDIRECT_UFS_NOTHROW_(decl,attr,Treturn,cc,name,param,name##64,code)
#define __XREDIRECT_UFS64_VOID_NOTHROW(decl,attr,cc,name,param,code)                                   __XREDIRECT_UFS_VOID_NOTHROW_(decl,attr,cc,name,param,name##64,code)
#define __REDIRECT_EXCEPT_FS64(decl,attr,Treturn,cc,name,param,args)                                   __REDIRECT_EXCEPT_(decl,attr,Treturn,cc,name,param,name##64,args)
#define __REDIRECT_EXCEPT_FS64_VOID(decl,attr,cc,name,param,args)                                      __REDIRECT_EXCEPT_VOID_(decl,attr,cc,name,param,name##64,args)
#define __REDIRECT_EXCEPT_FS64_XVOID(decl,attr,Treturn,cc,name,param,args)                             __REDIRECT_EXCEPT_XVOID_(decl,attr,Treturn,cc,name,param,name##64,args)
#define __VREDIRECT_EXCEPT_FS64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)        __VREDIRECT_EXCEPT_(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_EXCEPT_FS64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)           __VREDIRECT_EXCEPT_VOID_(decl,attr,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_EXCEPT_FS64_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)  __VREDIRECT_EXCEPT_XVOID_(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __XREDIRECT_EXCEPT_FS64(decl,attr,Treturn,cc,name,param,code)                                  __XREDIRECT_EXCEPT_(decl,attr,Treturn,cc,name,param,name##64,code)
#define __XREDIRECT_EXCEPT_FS64_VOID(decl,attr,cc,name,param,code)                                     __XREDIRECT_EXCEPT_VOID_(decl,attr,cc,name,param,name##64,code)
#define __XREDIRECT_EXCEPT_FS64_XVOID(decl,attr,Treturn,cc,name,param,code)                            __XREDIRECT_EXCEPT_XVOID_(decl,attr,Treturn,cc,name,param,name##64,code)
#define __REDIRECT_EXCEPT_UFS64(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT_EXCEPT_UFS_(decl,attr,Treturn,cc,name,param,name##64,args)
#define __REDIRECT_EXCEPT_UFS64_VOID(decl,attr,cc,name,param,args)                                     __REDIRECT_EXCEPT_UFS_VOID_(decl,attr,cc,name,param,name##64,args)
#define __REDIRECT_EXCEPT_UFS64_XVOID(decl,attr,Treturn,cc,name,param,args)                            __REDIRECT_EXCEPT_UFS_XVOID_(decl,attr,Treturn,cc,name,param,name##64,args)
#define __VREDIRECT_EXCEPT_UFS64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT_EXCEPT_UFS_(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_EXCEPT_UFS64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_EXCEPT_UFS_VOID_(decl,attr,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_EXCEPT_UFS64_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VREDIRECT_EXCEPT_UFS_XVOID_(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __XREDIRECT_EXCEPT_UFS64(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT_EXCEPT_UFS_(decl,attr,Treturn,cc,name,param,name##64,code)
#define __XREDIRECT_EXCEPT_UFS64_VOID(decl,attr,cc,name,param,code)                                    __XREDIRECT_EXCEPT_UFS_VOID_(decl,attr,cc,name,param,name##64,code)
#define __XREDIRECT_EXCEPT_UFS64_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XREDIRECT_EXCEPT_UFS_XVOID_(decl,attr,Treturn,cc,name,param,name##64,code)
#else /* __USE_FILE_OFFSET64 */
#define __REDIRECT_FS64(decl,attr,Treturn,cc,name,param,args)                                          __NOREDIRECT(decl,attr,Treturn,cc,name,param,name,args)
#define __REDIRECT_FS64_VOID(decl,attr,cc,name,param,args)                                             __NOREDIRECT_VOID(decl,attr,cc,name,param,name,args)
#define __REDIRECT_FS64_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                  __NOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name,args)
#define __REDIRECT_FS64_VOID_NOTHROW(decl,attr,cc,name,param,args)                                     __NOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name,args)
#define __VREDIRECT_FS64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)               __VNOREDIRECT(decl,attr,Treturn,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_FS64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                  __VNOREDIRECT_VOID(decl,attr,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_FS64_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_FS64_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name,vasmnamef,args,before_va_start)
#define __XREDIRECT_FS64(decl,attr,Treturn,cc,name,param,code)                                         __XNOREDIRECT(decl,attr,Treturn,cc,name,param,name,code)
#define __XREDIRECT_FS64_VOID(decl,attr,cc,name,param,code)                                            __XNOREDIRECT_VOID(decl,attr,cc,name,param,name,code)
#define __XREDIRECT_FS64_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                 __XNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name,code)
#define __XREDIRECT_FS64_VOID_NOTHROW(decl,attr,cc,name,param,code)                                    __XNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name,code)
#define __REDIRECT_UFS64                                                                               __REDIRECT_UFS
#define __REDIRECT_UFS64_VOID                                                                          __REDIRECT_UFS_VOID
#define __REDIRECT_UFS64_NOTHROW                                                                       __REDIRECT_UFS_NOTHROW
#define __REDIRECT_UFS64_VOID_NOTHROW                                                                  __REDIRECT_UFS_VOID_NOTHROW
#define __VREDIRECT_UFS64                                                                              __VREDIRECT_UFS
#define __VREDIRECT_UFS64_VOID                                                                         __VREDIRECT_UFS_VOID
#define __VREDIRECT_UFS64_NOTHROW                                                                      __VREDIRECT_UFS_NOTHROW
#define __VREDIRECT_UFS64_VOID_NOTHROW                                                                 __VREDIRECT_UFS_VOID_NOTHROW
#define __XREDIRECT_UFS64                                                                              __XREDIRECT_UFS
#define __XREDIRECT_UFS64_VOID                                                                         __XREDIRECT_UFS_VOID
#define __XREDIRECT_UFS64_NOTHROW                                                                      __XREDIRECT_UFS_NOTHROW
#define __XREDIRECT_UFS64_VOID_NOTHROW                                                                 __XREDIRECT_UFS_VOID_NOTHROW
#define __REDIRECT_EXCEPT_FS64                                                                         __REDIRECT_EXCEPT
#define __REDIRECT_EXCEPT_FS64_VOID                                                                    __REDIRECT_EXCEPT_VOID
#define __REDIRECT_EXCEPT_FS64_XVOID                                                                   __REDIRECT_EXCEPT_XVOID
#define __VREDIRECT_EXCEPT_FS64                                                                        __VREDIRECT_EXCEPT
#define __VREDIRECT_EXCEPT_FS64_VOID                                                                   __VREDIRECT_EXCEPT_VOID
#define __VREDIRECT_EXCEPT_FS64_XVOID                                                                  __VREDIRECT_EXCEPT_XVOID
#define __XREDIRECT_EXCEPT_FS64                                                                        __XREDIRECT_EXCEPT
#define __XREDIRECT_EXCEPT_FS64_VOID                                                                   __XREDIRECT_EXCEPT_VOID
#define __XREDIRECT_EXCEPT_FS64_XVOID                                                                  __XREDIRECT_EXCEPT_XVOID
#define __REDIRECT_EXCEPT_UFS64                                                                        __REDIRECT_EXCEPT_UFS
#define __REDIRECT_EXCEPT_UFS64_VOID                                                                   __REDIRECT_EXCEPT_UFS_VOID
#define __REDIRECT_EXCEPT_UFS64_XVOID                                                                  __REDIRECT_EXCEPT_UFS_XVOID
#define __VREDIRECT_EXCEPT_UFS64                                                                       __VREDIRECT_EXCEPT_UFS
#define __VREDIRECT_EXCEPT_UFS64_VOID                                                                  __VREDIRECT_EXCEPT_UFS_VOID
#define __VREDIRECT_EXCEPT_UFS64_XVOID                                                                 __VREDIRECT_EXCEPT_UFS_XVOID
#define __XREDIRECT_EXCEPT_UFS64                                                                       __XREDIRECT_EXCEPT_UFS
#define __XREDIRECT_EXCEPT_UFS64_VOID                                                                  __XREDIRECT_EXCEPT_UFS_VOID
#define __XREDIRECT_EXCEPT_UFS64_XVOID                                                                 __XREDIRECT_EXCEPT_UFS_XVOID
#endif /* !__USE_FILE_OFFSET64 */
#endif /* !__REDIRECT_FS64 */

#ifndef __REDIRECT_TM64
#ifdef __USE_TIME_BITS64
#define __REDIRECT_TM64(decl,attr,Treturn,cc,name,param,args)                                            __REDIRECT(decl,attr,Treturn,cc,name,param,name##64,args)
#define __REDIRECT_TM64_VOID(decl,attr,cc,name,param,args)                                               __REDIRECT_VOID(decl,attr,cc,name,param,name##64,args)
#define __REDIRECT_TM64_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                    __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name##64,args)
#define __REDIRECT_TM64_VOID_NOTHROW(decl,attr,cc,name,param,args)                                       __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name##64,args)
#define __VREDIRECT_TM64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                 __VREDIRECT(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_TM64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                    __VREDIRECT_VOID(decl,attr,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_TM64_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)         __VREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_TM64_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)            __VREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __XREDIRECT_TM64(decl,attr,Treturn,cc,name,param,code)                                           __XREDIRECT(decl,attr,Treturn,cc,name,param,name##64,code)
#define __XREDIRECT_TM64_VOID(decl,attr,cc,name,param,code)                                              __XREDIRECT_VOID(decl,attr,cc,name,param,name##64,code)
#define __XREDIRECT_TM64_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                   __XREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name##64,code)
#define __XREDIRECT_TM64_VOID_NOTHROW(decl,attr,cc,name,param,code)                                      __XREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name##64,code)
#define __REDIRECT_UFSTM64(decl,attr,Treturn,cc,name,param,args)                                         __REDIRECT_UFS_(decl,attr,Treturn,cc,name,param,name##64,args)
#define __REDIRECT_UFSTM64_VOID(decl,attr,cc,name,param,args)                                            __REDIRECT_UFS_VOID_(decl,attr,cc,name,param,name##64,args)
#define __REDIRECT_UFSTM64_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                 __REDIRECT_UFS_NOTHROW_(decl,attr,Treturn,cc,name,param,name##64,args)
#define __REDIRECT_UFSTM64_VOID_NOTHROW(decl,attr,cc,name,param,args)                                    __REDIRECT_UFS_VOID_NOTHROW_(decl,attr,cc,name,param,name##64,args)
#define __VREDIRECT_UFSTM64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)              __VREDIRECT_UFS_(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_UFSTM64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                 __VREDIRECT_UFS_VOID_(decl,attr,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_UFSTM64_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)      __VREDIRECT_UFS_NOTHROW_(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_UFSTM64_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)         __VREDIRECT_UFS_VOID_NOTHROW_(decl,attr,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __XREDIRECT_UFSTM64(decl,attr,Treturn,cc,name,param,code)                                        __XREDIRECT_UFS_(decl,attr,Treturn,cc,name,param,name##64,code)
#define __XREDIRECT_UFSTM64_VOID(decl,attr,cc,name,param,code)                                           __XREDIRECT_UFS_VOID_(decl,attr,cc,name,param,name##64,code)
#define __XREDIRECT_UFSTM64_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                __XREDIRECT_UFS_NOTHROW_(decl,attr,Treturn,cc,name,param,name##64,code)
#define __XREDIRECT_UFSTM64_VOID_NOTHROW(decl,attr,cc,name,param,code)                                   __XREDIRECT_UFS_VOID_NOTHROW_(decl,attr,cc,name,param,name##64,code)
#define __REDIRECT_EXCEPT_TM64(decl,attr,Treturn,cc,name,param,args)                                     __REDIRECT_EXCEPT_(decl,attr,Treturn,cc,name,param,name##64,args)
#define __REDIRECT_EXCEPT_TM64_VOID(decl,attr,cc,name,param,args)                                        __REDIRECT_EXCEPT_VOID_(decl,attr,cc,name,param,name##64,args)
#define __REDIRECT_EXCEPT_TM64_XVOID(decl,attr,Treturn,cc,name,param,args)                               __REDIRECT_EXCEPT_XVOID_(decl,attr,Treturn,cc,name,param,name##64,args)
#define __VREDIRECT_EXCEPT_TM64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_EXCEPT_(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_EXCEPT_TM64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)             __VREDIRECT_EXCEPT_VOID_(decl,attr,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_EXCEPT_TM64_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)    __VREDIRECT_EXCEPT_XVOID_(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __XREDIRECT_EXCEPT_TM64(decl,attr,Treturn,cc,name,param,code)                                    __XREDIRECT_EXCEPT_(decl,attr,Treturn,cc,name,param,name##64,code)
#define __XREDIRECT_EXCEPT_TM64_VOID(decl,attr,cc,name,param,code)                                       __XREDIRECT_EXCEPT_VOID_(decl,attr,cc,name,param,name##64,code)
#define __XREDIRECT_EXCEPT_TM64_XVOID(decl,attr,Treturn,cc,name,param,code)                              __XREDIRECT_EXCEPT_XVOID_(decl,attr,Treturn,cc,name,param,name##64,code)
#define __REDIRECT_EXCEPT_UFSTM64(decl,attr,Treturn,cc,name,param,args)                                  __REDIRECT_EXCEPT_UFS_(decl,attr,Treturn,cc,name,param,name##64,args)
#define __REDIRECT_EXCEPT_UFSTM64_VOID(decl,attr,cc,name,param,args)                                     __REDIRECT_EXCEPT_UFS_VOID_(decl,attr,cc,name,param,name##64,args)
#define __REDIRECT_EXCEPT_UFSTM64_XVOID(decl,attr,Treturn,cc,name,param,args)                            __REDIRECT_EXCEPT_UFS_XVOID_(decl,attr,Treturn,cc,name,param,name##64,args)
#define __VREDIRECT_EXCEPT_UFSTM64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)       __VREDIRECT_EXCEPT_UFS_(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSTM64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)          __VREDIRECT_EXCEPT_UFS_VOID_(decl,attr,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __VREDIRECT_EXCEPT_UFSTM64_XVOID(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start) __VREDIRECT_EXCEPT_UFS_XVOID_(decl,attr,Treturn,cc,name,param,name##64,vasmnamef##64,args,before_va_start)
#define __XREDIRECT_EXCEPT_UFSTM64(decl,attr,Treturn,cc,name,param,code)                                 __XREDIRECT_EXCEPT_UFS_(decl,attr,Treturn,cc,name,param,name##64,code)
#define __XREDIRECT_EXCEPT_UFSTM64_VOID(decl,attr,cc,name,param,code)                                    __XREDIRECT_EXCEPT_UFS_VOID_(decl,attr,cc,name,param,name##64,code)
#define __XREDIRECT_EXCEPT_UFSTM64_XVOID(decl,attr,Treturn,cc,name,param,code)                           __XREDIRECT_EXCEPT_UFS_XVOID_(decl,attr,Treturn,cc,name,param,name##64,code)
#else /* __USE_TIME_BITS64 */
#define __REDIRECT_TM64(decl,attr,Treturn,cc,name,param,args)                                            __NOREDIRECT(decl,attr,Treturn,cc,name,param,name,args)
#define __REDIRECT_TM64_VOID(decl,attr,cc,name,param,args)                                               __NOREDIRECT_VOID(decl,attr,cc,name,param,name,args)
#define __REDIRECT_TM64_NOTHROW(decl,attr,Treturn,cc,name,param,args)                                    __NOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name,args)
#define __REDIRECT_TM64_VOID_NOTHROW(decl,attr,cc,name,param,args)                                       __NOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name,args)
#define __VREDIRECT_TM64(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)                 __VNOREDIRECT(decl,attr,Treturn,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_TM64_VOID(decl,attr,cc,name,param,vasmnamef,args,before_va_start)                    __VNOREDIRECT_VOID(decl,attr,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_TM64_NOTHROW(decl,attr,Treturn,cc,name,param,vasmnamef,args,before_va_start)         __VNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name,vasmnamef,args,before_va_start)
#define __VREDIRECT_TM64_VOID_NOTHROW(decl,attr,cc,name,param,vasmnamef,args,before_va_start)            __VNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name,vasmnamef,args,before_va_start)
#define __XREDIRECT_TM64(decl,attr,Treturn,cc,name,param,code)                                           __XNOREDIRECT(decl,attr,Treturn,cc,name,param,name,code)
#define __XREDIRECT_TM64_VOID(decl,attr,cc,name,param,code)                                              __XNOREDIRECT_VOID(decl,attr,cc,name,param,name,code)
#define __XREDIRECT_TM64_NOTHROW(decl,attr,Treturn,cc,name,param,code)                                   __XNOREDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,name,code)
#define __XREDIRECT_TM64_VOID_NOTHROW(decl,attr,cc,name,param,code)                                      __XNOREDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,name,code)
#define __REDIRECT_UFSTM64                                                                               __REDIRECT_UFS
#define __REDIRECT_UFSTM64_VOID                                                                          __REDIRECT_UFS_VOID
#define __REDIRECT_UFSTM64_NOTHROW                                                                       __REDIRECT_UFS_NOTHROW
#define __REDIRECT_UFSTM64_VOID_NOTHROW                                                                  __REDIRECT_UFS_VOID_NOTHROW
#define __VREDIRECT_UFSTM64                                                                              __VREDIRECT_UFS
#define __VREDIRECT_UFSTM64_VOID                                                                         __VREDIRECT_UFS_VOID
#define __VREDIRECT_UFSTM64_NOTHROW                                                                      __VREDIRECT_UFS_NOTHROW
#define __VREDIRECT_UFSTM64_VOID_NOTHROW                                                                 __VREDIRECT_UFS_VOID_NOTHROW
#define __XREDIRECT_UFSTM64                                                                              __XREDIRECT_UFS
#define __XREDIRECT_UFSTM64_VOID                                                                         __XREDIRECT_UFS_VOID
#define __XREDIRECT_UFSTM64_NOTHROW                                                                      __XREDIRECT_UFS_NOTHROW
#define __XREDIRECT_UFSTM64_VOID_NOTHROW                                                                 __XREDIRECT_UFS_VOID_NOTHROW
#define __REDIRECT_EXCEPT_TM64                                                                           __REDIRECT_EXCEPT
#define __REDIRECT_EXCEPT_TM64_VOID                                                                      __REDIRECT_EXCEPT_VOID
#define __REDIRECT_EXCEPT_TM64_XVOID                                                                     __REDIRECT_EXCEPT_XVOID
#define __VREDIRECT_EXCEPT_TM64                                                                          __VREDIRECT_EXCEPT
#define __VREDIRECT_EXCEPT_TM64_VOID                                                                     __VREDIRECT_EXCEPT_VOID
#define __VREDIRECT_EXCEPT_TM64_XVOID                                                                    __VREDIRECT_EXCEPT_XVOID
#define __XREDIRECT_EXCEPT_TM64                                                                          __XREDIRECT_EXCEPT
#define __XREDIRECT_EXCEPT_TM64_VOID                                                                     __XREDIRECT_EXCEPT_VOID
#define __XREDIRECT_EXCEPT_TM64_XVOID                                                                    __XREDIRECT_EXCEPT_XVOID
#define __REDIRECT_EXCEPT_UFSTM64                                                                        __REDIRECT_EXCEPT_UFS
#define __REDIRECT_EXCEPT_UFSTM64_VOID                                                                   __REDIRECT_EXCEPT_UFS_VOID
#define __REDIRECT_EXCEPT_UFSTM64_XVOID                                                                  __REDIRECT_EXCEPT_UFS_XVOID
#define __VREDIRECT_EXCEPT_UFSTM64                                                                       __VREDIRECT_EXCEPT_UFS
#define __VREDIRECT_EXCEPT_UFSTM64_VOID                                                                  __VREDIRECT_EXCEPT_UFS_VOID
#define __VREDIRECT_EXCEPT_UFSTM64_XVOID                                                                 __VREDIRECT_EXCEPT_UFS_XVOID
#define __XREDIRECT_EXCEPT_UFSTM64                                                                       __XREDIRECT_EXCEPT_UFS
#define __XREDIRECT_EXCEPT_UFSTM64_VOID                                                                  __XREDIRECT_EXCEPT_UFS_VOID
#define __XREDIRECT_EXCEPT_UFSTM64_XVOID                                                                 __XREDIRECT_EXCEPT_UFS_XVOID
#endif /* !__USE_TIME_BITS64 */
#endif /* !__REDIRECT_TM64 */








#ifdef _PORT_SOURCE
#define __USE_PORTABLE 1
#endif

#ifdef __USE_PORTABLE
#   define __PORT_KOSONLY        __ATTR_DEPRECATED("Non-portable KOS extension")
#   define __PORT_DOSONLY        __ATTR_DEPRECATED("Only available under DOS")
#   define __PORT_UNXONLY        __ATTR_DEPRECATED("Non-portable unix extension")
#   define __PORT_NOCYG          __ATTR_DEPRECATED("Not available on cygwin")
#   define __PORT_NODOS          __ATTR_DEPRECATED("Not available under DOS")
#   define __PORT_KOSONLY_ALT(x) __ATTR_DEPRECATED("Non-portable KOS extension (Consider using `" #x "' in portable code)")
#   define __PORT_DOSONLY_ALT(x) __ATTR_DEPRECATED("Only available under DOS (Consider using `" #x "' instead)")
#   define __PORT_NODOS_ALT(x)   __ATTR_DEPRECATED("Not available under DOS (Consider using `" #x "' in portable code)")
#   define __WARN_NONSTD(alt) \
      __ATTR_DEPRECATED("This function does not behave according to the STD-C standard. " \
                        "Consider using compliant function `" #alt "' instead")
#else
#   define __PORT_KOSONLY        /* Nothing */
#   define __PORT_DOSONLY        /* Nothing */
#   define __PORT_UNXONLY        /* Nothing */
#   define __PORT_NOCYG          /* Nothing */
#   define __PORT_NODOS          /* Nothing */
#   define __PORT_KOSONLY_ALT(x) /* Nothing */
#   define __PORT_DOSONLY_ALT(x) /* Nothing */
#   define __PORT_NODOS_ALT(x)   /* Nothing */
#   define __WARN_NONSTD(alt)    /* Nothing */
#endif


#ifdef __USE_DOSFS
#   define __WARN_NODOSFS __ATTR_DEPRECATED("This function does not support DOS filesystem semantics. Try building with `-D_DOSFS_SOURCE=0'")
#   define __WARN_NOKOSFS /* Nothing */
#else
#   define __WARN_NODOSFS /* Nothing */
#   define __WARN_NOKOSFS __ATTR_DEPRECATED("This function does not support KOS filesystem semantics. Try building with `-D_DOSFS_SOURCE=1'")
#endif

#if 1 /* TODO: Option */
/* Set on functions like read() that (may) modify some user-provided data,
 * but can only tell how much and if at all something was done by having the
 * caller inspect the return value. */
#define __WUNUSED_SUGGESTED /* Nothing */
#else
#define __WUNUSED_SUGGESTED __WUNUSED
#endif

#ifdef __USE_DOS
#if defined(__STDC_WANT_SECURE_LIB__) && \
            __STDC_WANT_SECURE_LIB__+0 != 0
#define __USE_DOS_SLIB 1
#endif
#endif

#ifdef _DEBUG_SOURCE
#if _DEBUG_SOURCE+0 == 0
#   define __USE_DEBUG 0
#else
#   define __USE_DEBUG 1
#endif
#endif

#if defined(CONFIG_DEBUG) || defined(_DEBUG) || \
    defined(DEBUG) || !defined(NDEBUG)
#   define __USE_DEBUG_HOOK 1
#endif

#if /*defined(__OPTIMIZE__) || */defined(RELEASE) || \
    defined(_RELEASE) || defined(NDEBUG)
#   undef __USE_DEBUG_HOOK
#endif

#ifdef __USE_DEBUG_HOOK
#ifndef __USE_DEBUG
#define __USE_DEBUG 1
#endif
#endif

#if defined(__OPTIMIZE__) || defined(__KERNEL__)
#ifndef __NO_builtin_constant_p
#   define __OPTIMIZE_CONST__ 1 /* Use `__builtin_constant_p()' to optimize system headers. */
#endif /* !__NO_builtin_constant_p */
#ifndef __OPTIMIZE_SIZE__
#   define __OPTIMIZE_ASM__   1 /* Use arch-specific inline assembly to optimize system headers. */
#endif /* !__OPTIMIZE_SIZE__ */
#ifndef __NO_OPTIMIZE_LIBC__
#   define __OPTIMIZE_LIBC__  1 /* Redirect specific lib-C functions to optimized implementations. */
#endif /* !__NO_OPTIMIZE_LIBC__ */
#endif
#ifdef __OPTIMIZE_LIBC__
#   define __OPT_LOCAL __FORCELOCAL
#else
#   define __OPT_LOCAL __LOCAL
#endif

#if defined(__USE_DEBUG) && __USE_DEBUG == 0
/* No point in hook debug functions if they'll just loop back. */
#undef __USE_DEBUG_HOOK
#endif

#if !defined(__USE_PORTABLE) && \
    (defined(__CRT_KOS) && defined(__USE_KOS))
/* Don't warn about KOS's extensions to printf and friends. */
#   define __ATTR_LIBC_PRINTF(fmt,args)      /* Nothing */
#   define __ATTR_LIBC_PRINTF_P(fmt,args)    /* Nothing */
#   define __ATTR_LIBC_SCANF(fmt,args)       /* Nothing */
#   define __ATTR_LIBC_STRFMON(fmt,args)     /* Nothing */
#   define __ATTR_LIBC_STRFTIME(fmt,args)    /* Nothing */
#   define __ATTR_LIBC_WPRINTF(fmt,args)     /* Nothing */
#   define __ATTR_LIBC_WPRINTF_P(fmt,args)   /* Nothing */
#   define __ATTR_LIBC_WSCANF(fmt,args)      /* Nothing */
#   define __ATTR_LIBC_WCSFMON(fmt,args)     /* Nothing */
#   define __ATTR_LIBC_WCSFTIME(fmt,args)    /* Nothing */
#   define __ATTR_LIBC_W16PRINTF(fmt,args)   /* Nothing */
#   define __ATTR_LIBC_W16PRINTF_P(fmt,args) /* Nothing */
#   define __ATTR_LIBC_W16SCANF(fmt,args)    /* Nothing */
#   define __ATTR_LIBC_W16FMON(fmt,args)     /* Nothing */
#   define __ATTR_LIBC_W16FTIME(fmt,args)    /* Nothing */
#   define __ATTR_LIBC_W32PRINTF(fmt,args)   /* Nothing */
#   define __ATTR_LIBC_W32PRINTF_P(fmt,args) /* Nothing */
#   define __ATTR_LIBC_W32SCANF(fmt,args)    /* Nothing */
#   define __ATTR_LIBC_W32FMON(fmt,args)     /* Nothing */
#   define __ATTR_LIBC_W32FTIME(fmt,args)    /* Nothing */
#else
#   define __ATTR_LIBC_PRINTF(fmt,args)      __ATTR_FORMAT_PRINTF(fmt,args)
#   define __ATTR_LIBC_PRINTF_P(fmt,args)    /* Nothing */
#   define __ATTR_LIBC_SCANF(fmt,args)       __ATTR_FORMAT_SCANF(fmt,args)
#   define __ATTR_LIBC_STRFMON(fmt,args)     __ATTR_FORMAT_STRFMON(fmt,args)
#   define __ATTR_LIBC_STRFTIME(fmt,args)    __ATTR_FORMAT_STRFTIME(fmt,args)
#   define __ATTR_LIBC_WPRINTF(fmt,args)     /* Nothing */
#   define __ATTR_LIBC_WPRINTF_P(fmt,args)   /* Nothing */
#   define __ATTR_LIBC_WSCANF(fmt,args)      /* Nothing */
#   define __ATTR_LIBC_WCSFMON(fmt,args)     /* Nothing */
#   define __ATTR_LIBC_WCSFTIME(fmt,args)    /* Nothing */
#   define __ATTR_LIBC_W16PRINTF(fmt,args)   /* Nothing */
#   define __ATTR_LIBC_W16PRINTF_P(fmt,args) /* Nothing */
#   define __ATTR_LIBC_W16SCANF(fmt,args)    /* Nothing */
#   define __ATTR_LIBC_W16FMON(fmt,args)     /* Nothing */
#   define __ATTR_LIBC_W16FTIME(fmt,args)    /* Nothing */
#   define __ATTR_LIBC_W32PRINTF(fmt,args)   /* Nothing */
#   define __ATTR_LIBC_W32PRINTF_P(fmt,args) /* Nothing */
#   define __ATTR_LIBC_W32SCANF(fmt,args)    /* Nothing */
#   define __ATTR_LIBC_W32FMON(fmt,args)     /* Nothing */
#   define __ATTR_LIBC_W32FTIME(fmt,args)    /* Nothing */
#endif


#endif /* !_FEATURES_H */
