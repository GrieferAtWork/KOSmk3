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
#ifndef _PARTS_DOS_STDIO_H
#define _PARTS_DOS_STDIO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/io-file.h>
#ifndef _STDIO_H
#include <stdio.h>
#endif
#include <xlocale.h>

__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef _iobuf
#define _iobuf   __IO_FILE
#endif /* !_iobuf */

#define _NFILE          512
#define _NSTREAM_       512
#define _IOB_ENTRIES    20
#define _P_tmpdir       "\\"
#define _wP_tmpdir     L"\\"
#define _SYS_OPEN       20
#ifdef __USE_DOS_SLIB
#define L_tmpnam_s      18
#define TMP_MAX_S       2147483647
#define _TMP_MAX_S      2147483647
#endif /* __USE_DOS_SLIB */

#ifndef _FPOS_T_DEFINED
#define _FPOS_T_DEFINED 1
#endif /* !_FPOS_T_DEFINED */
#ifndef _STDSTREAM_DEFINED
#define _STDSTREAM_DEFINED 1
#endif /* !_STDSTREAM_DEFINED */
#ifndef _FILE_DEFINED
#define _FILE_DEFINED 1
#endif /* !_FILE_DEFINED */

#define _IOREAD  __IO_FILE_IOR
#define _IOWRT   __IO_FILE_IOW
#define _IOMYBUF __IO_FILE_IOMALLBUF
#define _IOEOF   __IO_FILE_IOEOF
#define _IOERR   __IO_FILE_IOERR
#define _IOSTRG  __IO_FILE_IONOFD
#define _IORW    __IO_FILE_IORW

#define _TWO_DIGIT_EXPONENT 0x1

#ifndef _CRT_PERROR_DEFINED
#define _CRT_PERROR_DEFINED 1
#endif /* !_CRT_PERROR_DEFINED */

#ifndef __KERNEL__
#ifndef _CRT_DIRECTORY_DEFINED
#define _CRT_DIRECTORY_DEFINED 1
#ifndef ___unlink_defined
#define ___unlink_defined 1
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,unlink,(char const *__name),(__name))
#endif /* !___unlink_defined */
#ifndef __unlink_defined
#define __unlink_defined 1
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,unlink,(char const *__name),(__name))
#endif /* !__unlink_defined */
#endif  /* _CRT_DIRECTORY_DEFINED */

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifdef __USE_KOS_STDEXT
#define __PRINTF_RETURN_TYPE __ssize_t
#else
#define __PRINTF_RETURN_TYPE int
#endif

#ifndef __rsize_t_defined
#define __rsize_t_defined 1
typedef size_t rsize_t;
#endif /* !__rsize_t_defined */

#ifndef _STDIO_DEFINED
__REDIRECT_UFSDPB(__LIBC,,__FILE *,__LIBCCALL,popen,(char const *__command, char const *__mode),(__command,__mode))
__REDIRECT_DPB(__LIBC,,int,__LIBCCALL,pclose,(__FILE *__restrict __stream),(__file))
__REDIRECT_UFSDPB(__LIBC,,__FILE *,__LIBCCALL,fsopen,(char const *__file, char const *__mode, int __shflag),(__file,__mode,__shflag))
__REDIRECT_DPB(__LIBC,,__FILE *,__LIBCCALL,fdopen,(__fd_t __fd, char const *__restrict __mode),(__fd,__modes))
#if defined(__GLC_COMPAT__) || defined(__CYG_COMPAT__) || !defined(__CRT_DOS)
__LOCAL int (__LIBCCALL _flushall)(void) { return fflush(NULL); }
#else /* ... */
__REDIRECT_DPB(__LIBC,,int,__LIBCCALL,flushall,(void),())
#endif /* !... */
__REDIRECT_DPB(__LIBC,,int,__LIBCCALL,fcloseall,(void),())
__REDIRECT_DPB(__LIBC,,int,__LIBCCALL,fileno,(__FILE *__restrict __stream),(__file))
__REDIRECT(__LIBC,,int,__LIBCCALL,_fgetchar,(void),getchar,())
__REDIRECT(__LIBC,,int,__LIBCCALL,_fputchar,(int __ch),putchar,(__ch))
__REDIRECT_DPB(__LIBC,,int,__LIBCCALL,getw,(__FILE *__restrict __stream),(__file))
__REDIRECT_DPB(__LIBC,,int,__LIBCCALL,putw,(int __w, __FILE *__restrict __stream),(__w,__file))
__REDIRECT_UFSDPB(__LIBC,__ATTR_MALLOC,char *,__LIBCCALL,tempnam,(char const *__dir, char const *__pfx),(__dir,__pfx))
#ifdef __DOS_COMPAT__
__LIBC int (__LIBCCALL _fseeki64)(__FILE *__restrict __stream, __INT64_TYPE__ __off, int __whence);
__LIBC __INT64_TYPE__ (__LIBCCALL _ftelli64)(__FILE *__restrict __stream);
#else
__REDIRECT(__LIBC,,int,__LIBCCALL,_fseeki64,(__FILE *__restrict __stream, __INT64_TYPE__ __off, int __whence),fseeko64,(__file,__off,__whence))
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_ftelli64,(__FILE *__restrict __stream),ftello64,(__file))
#endif

#ifdef __CRT_DOS
__LIBC __PORT_DOSONLY int (__LIBCCALL _rmtmp)(void);
__LIBC __PORT_DOSONLY int (__LIBCCALL _filbuf)(__FILE *__restrict __stream);
__LIBC __PORT_DOSONLY int (__LIBCCALL _flsbuf)(int __ch, __FILE *__restrict __stream);
__LIBC __PORT_DOSONLY int (__LIBCCALL _getmaxstdio)(void);
__LIBC __PORT_DOSONLY int (__LIBCCALL _setmaxstdio)(int __val);
__LIBC __PORT_DOSONLY int (__LIBCCALL _set_printf_count_output)(int __val);
__LIBC __PORT_DOSONLY int (__LIBCCALL _get_printf_count_output)(void);
__LIBC __PORT_DOSONLY __UINT32_TYPE__ (__LIBCCALL _set_output_format)(__UINT32_TYPE__ __format);
__LIBC __PORT_DOSONLY __UINT32_TYPE__ (__LIBCCALL _get_output_format)(void);

__VREDIRECT_DPB(__LIBC,,__PRINTF_RETURN_TYPE,__ATTR_CDECL,scanf_l,(char const *__restrict __format, __locale_t __locale, ...),vscanf_l,(__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,,__PRINTF_RETURN_TYPE,__ATTR_CDECL,sscanf_l,(char const *__restrict __src, char const *__restrict __format, __locale_t __locale, ...),vsscanf_l,(__src,__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,,__PRINTF_RETURN_TYPE,__ATTR_CDECL,fscanf_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...),vfscanf_l,(__file,__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,,__PRINTF_RETURN_TYPE,__ATTR_CDECL,scanf_s_l,(char const *__restrict __format, __locale_t __locale, ...),vscanf_s_l,(__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,,__PRINTF_RETURN_TYPE,__ATTR_CDECL,sscanf_s_l,(char const *__restrict __src, char const *__restrict __format, __locale_t __locale, ...),vsscanf_s_l,(__src,__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,,__PRINTF_RETURN_TYPE,__ATTR_CDECL,fscanf_s_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...),vfscanf_s_l,(__file,__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,snscanf,(char const *__restrict __src, size_t __buflen, char const *__restrict __format, ...),vsnscanf,(__src,__buflen,__format),__format)
__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,snscanf_l,(char const *__restrict __src, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...),vsnscanf_l,(__src,__buflen,__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,snscanf_s,(char const *__restrict __src, size_t __buflen, char const *__restrict __format, ...),vsnscanf_s,(__src,__buflen,__format),__format)
__VREDIRECT_DPB(__LIBC,__PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,snscanf_s_l,(char const *__restrict __src, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...),vsnscanf_s_l,(__src,__buflen,__format,__locale),__locale)

__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vsprintf_l,(char *__restrict __buf, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__buf,__format,__locale,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(3,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vsprintf_s_l,(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__buf,__bufsize,__format,__locale,__args))
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(2,4),__PRINTF_RETURN_TYPE,__ATTR_CDECL,sprintf_l,(char *__restrict __buf, char const *__restrict __format, __locale_t __locale, ...),vsprintf_l,(__buf,__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(3,5),__PRINTF_RETURN_TYPE,__ATTR_CDECL,sprintf_s_l,(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __locale_t __locale, ...),vsprintf_s_l,(__buf,__bufsize,__format,__locale),__locale)
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(3,0) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vsprintf_p,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args),(__buf,__buflen,__format,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(3,0) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vsprintf_p_l,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale,  __builtin_va_list __args),(__buf,__buflen,__format,__locale, __args))
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(3,4) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,sprintf_p,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...),vsprintf_p,(__buf,__buflen,__format),__format)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(3,5) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,sprintf_p_l,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...),sprintf_p_l,(__buf,__buflen,__format,__locale),__locale)

#ifndef ___scprintf_defined
#define ___scprintf_defined 1
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vscprintf,(char const *__restrict __format, __builtin_va_list __args),(__format,__args))
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(1,2),__PRINTF_RETURN_TYPE,__ATTR_CDECL,scprintf,(char const *__restrict __format, ...),vscprintf,(__format),__format)
#endif /* !___scprintf_defined */
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vscprintf_l,(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(1,0) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vscprintf_p,(char const *__restrict __format, __builtin_va_list __args),(__format,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(1,0) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vscprintf_p_l,(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(1,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,scprintf_l,(char const *__restrict __format, __locale_t __locale, ...),vscprintf_l,(__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(1,2) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,scprintf_p,(char const *__restrict __format, ...),vscprintf_p,(__format),__format)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(1,3) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,scprintf_p_l,(char const *__restrict __format, __locale_t __locale, ...),vscprintf_p_l,(__format,__locale),__locale)

/* The following 2 return an error, rather than the required size when the buffer is too small */
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(3,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vsnprintf,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args),(__buf,__buflen,__format,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(3,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vsnprintf_l,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__buf,__buflen,__format,__locale,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(3,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vsnprintf_c,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args),(__buf,__buflen,__format,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(3,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vsnprintf_c_l,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__buf,__buflen,__format,__locale,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(4,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vsnprintf_s,(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __builtin_va_list __args),(__buf,__bufsize,__buflen,__format,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(4,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vsnprintf_s_l,(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__buf,__bufsize,__buflen,__format,__locale,__args))
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(3,4),__PRINTF_RETURN_TYPE,__ATTR_CDECL,snprintf,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...),vsnprintf,(__buf,__buflen,__format),__format)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(3,5),__PRINTF_RETURN_TYPE,__ATTR_CDECL,snprintf_l,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...),vsnprintf_l,(__buf,__buflen,__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(3,4),__PRINTF_RETURN_TYPE,__ATTR_CDECL,snprintf_c,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...),vsnprintf_c,(__buf,__buflen,__format),__format)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(3,5),__PRINTF_RETURN_TYPE,__ATTR_CDECL,snprintf_c_l,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...),vsnprintf_c_l,(__buf,__buflen,__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(4,5),__PRINTF_RETURN_TYPE,__ATTR_CDECL,snprintf_s,(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, ...),vsnprintf_s,(__buf,__bufsize,__buflen,__format),__format)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(4,6),__PRINTF_RETURN_TYPE,__ATTR_CDECL,snprintf_s_l,(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...),vsnprintf_s_l,(__buf,__bufsize,__buflen,__format,__locale),__locale)

__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vprintf_l,(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vprintf_s_l,(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(1,0) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vprintf_p,(char const *__restrict __format, __builtin_va_list __args),(__format,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(1,0) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vprintf_p_l,(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(1,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,printf_l,(char const *__restrict __format, __locale_t __locale, ...),vprintf_l,(__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(1,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,printf_s_l,(char const *__restrict __format, __locale_t __locale, ...),vprintf_s_l,(__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(1,2) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,printf_p,(char const *__restrict __format, ...),vprintf_p,(__format),__format)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(1,3) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,printf_p_l,(char const *__restrict __format, __locale_t __locale, ...),vprintf_p_l,(__format,__locale),__locale)

__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfprintf_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__stream,__format,__locale,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfprintf_s_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__stream,__format,__locale,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(2,0) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vfprintf_p,(__FILE *__restrict __stream, char const *__restrict __format, __builtin_va_list __args),(__stream,__format,__args))
__REDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(2,0) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__LIBCCALL,vfprintf_p_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__stream,__format,__locale,__args))
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(2,4),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fprintf_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...),vfprintf_l,(__stream,__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF(2,4),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fprintf_s_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...),vfprintf_s_l,(__stream,__format,__locale),__locale)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(2,3) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,fprintf_p,(__FILE *__restrict __stream, char const *__restrict __format, ...),vfprintf_p,(__stream,__format),__format)
__VREDIRECT_DPB(__LIBC,__ATTR_LIBC_PRINTF_P(2,4) __PORT_DOSONLY,__PRINTF_RETURN_TYPE,__ATTR_CDECL,fprintf_p_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...),vfprintf_p_l,(__stream,__format,__locale),__locale)

#else /* __CRT_DOS */

__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _fscanf_l)(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vfscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _fscanf_s_l)(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vfscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _scanf_l)(char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vscanf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _scanf_s_l)(char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vscanf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _sscanf_l)(char const *__restrict __src, char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vsscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _sscanf_s_l)(char const *__restrict __src, char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vsscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vsprintf_l)(char *__restrict __buf, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return vsprintf(__buf,__format,__args); }
__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vsprintf_s_l)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return __libc_vsnprintf(__buf,__bufsize,__format,__args); }
__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vscprintf)(char const *__restrict __format, __builtin_va_list __args) { return __libc_vsnprintf(NULL,0,__format,__args); }
__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vscprintf_l)(char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return __libc_vsnprintf(NULL,0,__format,__args); }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _sprintf_l)(char *__restrict __buf, char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = sprintf(__buf,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _sprintf_s_l)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = snprintf(__buf,__bufsize,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _scprintf)(char const *__restrict __format, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = _vscprintf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _scprintf_l)(char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vscprintf(__format,__args); __builtin_va_end(__args); return __result; }

__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) { __PRINTF_RETURN_TYPE __result = __libc_vsnprintf(__buf,__buflen,__format,__args); return (size_t)__result < __buflen ? __result : -1; }
__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vsnprintf_c)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) { __PRINTF_RETURN_TYPE __result = __libc_vsnprintf(__buf,__buflen,__format,__args); return (size_t)__result < __buflen ? __result : -1; }
__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vsnprintf_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { __PRINTF_RETURN_TYPE __result = __libc_vsnprintf(__buf,__buflen,__format,__args); return (size_t)__result < __buflen ? __result : -1; }
__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vsnprintf_c_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { __PRINTF_RETURN_TYPE __result = __libc_vsnprintf(__buf,__buflen,__format,__args); return (size_t)__result < __buflen ? __result : -1; }
__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vsnprintf_s)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) { __PRINTF_RETURN_TYPE __result = __libc_vsnprintf(__buf,__bufsize < __buflen ? __bufsize : __buflen,__format,__args); return (size_t)__result < __buflen ? __result : -1; }
__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vsnprintf_s_l)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { __PRINTF_RETURN_TYPE __result = __libc_vsnprintf(__buf,__bufsize < __buflen ? __bufsize : __buflen,__format,__args); return (size_t)__result < __buflen ? __result : -1; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = _vsnprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _snprintf_c)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = _vsnprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _snprintf_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vsnprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _snprintf_c_l)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vsnprintf(__buf,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _snprintf_s)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = _vsnprintf_s(__buf,__bufsize,__buflen,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _snprintf_s_l)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = _vsnprintf_s(__buf,__bufsize,__buflen,__format,__args); __builtin_va_end(__args); return __result; }

__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vprintf_l)(char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return vprintf(__format,__args); }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _printf_l)(char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vprintf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vprintf_s_l)(char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return vprintf(__format,__args); }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _printf_s_l)(char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vprintf(__format,__args); __builtin_va_end(__args); return __result; }

__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vfprintf_l)(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return vfprintf(__file,__format,__args); }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _fprintf_l)(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vfprintf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL __PRINTF_RETURN_TYPE (__LIBCCALL _vfprintf_s_l)(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __UNUSED(__locale), __builtin_va_list __args) { return vfprintf(__file,__format,__args); }
__LOCAL __PRINTF_RETURN_TYPE (__ATTR_CDECL _fprintf_s_l)(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...) { __PRINTF_RETURN_TYPE __result; __builtin_va_list __args; __builtin_va_start(__args,__locale); __result = vfprintf(__file,__format,__args); __builtin_va_end(__args); return __result; }

#endif /* !__CRT_DOS */

#ifdef __USE_DOS_SLIB
#ifdef __CRT_DOS
/* WARNING: `fopen_s' and `freopen_s' returns DOS error codes in DOS-FS mode! */
__REDIRECT_UFS(__LIBC,__PORT_DOSONLY_ALT(fopen),errno_t,__LIBCCALL,fopen_s,(__FILE **__pfile, char const *__file, char const *__mode),(__pfile,__file,__mode))
__REDIRECT_UFS(__LIBC,__PORT_DOSONLY_ALT(freopen),errno_t,__LIBCCALL,freopen_s,(__FILE **__pfile, char const *__file, char const *__mode, __FILE *__oldfile),(__pfile,__file,__mode,__oldfile))
__REDIRECT_UFS(__LIBC,__PORT_DOSONLY_ALT(tmpnam),errno_t,__LIBCCALL,tmpnam_s,(char *__restrict __buf, rsize_t __bufsize),(__buf,__bufsize))
__LIBC __PORT_DOSONLY_ALT(clearerr) errno_t (__LIBCCALL clearerr_s)(__FILE *__restrict __stream);
__LIBC __PORT_DOSONLY_ALT(tmpfile) errno_t (__LIBCCALL tmpfile_s)(__FILE **__pfile);
__LIBC size_t (__LIBCCALL fread_s)(void *__buf, size_t __bufsize, size_t __elemsize, size_t __elemcount, __FILE *__restrict __stream);
__LIBC char *(__LIBCCALL gets_s)(char *__restrict __buf, rsize_t __bufsize);
__LIBC int (__ATTR_CDECL printf_s)(char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vprintf_s)(char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL fprintf_s)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vfprintf_s)(__FILE *__restrict __stream, char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL sprintf_s)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vsprintf_s)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL fscanf_s)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vfscanf_s)(__FILE *__restrict __stream, char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL scanf_s)(char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vscanf_s)(char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__ATTR_CDECL sscanf_s)(char const *__restrict __src, char const *__restrict __format, ...);
__LIBC int (__LIBCCALL vsscanf_s)(char const *__restrict __src, char const *__restrict __format, __builtin_va_list __args);
__LIBC int (__LIBCCALL vsnprintf_s)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __builtin_va_list __args);
#else /* __CRT_DOS */
__LOCAL size_t (__LIBCCALL fread_s)(void *__buf, size_t __bufsize, size_t __elemsize, size_t __elemcount, __FILE *__restrict __stream) { __bufsize /= __elemsize; return fread(__buf,__elemsize,__bufsize < __elemcount ? __bufsize : __elemcount,__file); }
__LOCAL char *(__LIBCCALL gets_s)(char *__restrict __buf, rsize_t __bufsize) { return fgets(__buf,__bufsize,stdin); }
__LOCAL int (__LIBCCALL vprintf_s)(char const *__restrict __format, __builtin_va_list __args) { return vprintf(__format,__args); }
__LOCAL int (__LIBCCALL vfprintf_s)(__FILE *__restrict __stream, char const *__restrict __format, __builtin_va_list __args) { return vfprintf(__file,__format,__args); }
__LOCAL int (__LIBCCALL vsprintf_s)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, __builtin_va_list __args) { return __libc_vsnprintf(__buf,__bufsize,__format,__args); }
__LOCAL int (__LIBCCALL vsnprintf_s)(char *__restrict __buf, size_t __bufsize, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) { return __libc_vsnprintf(__buf,__bufsize < __buflen ? __bufsize : __buflen,__format,__args); }
__LOCAL int (__LIBCCALL vfscanf_s)(__FILE *__restrict __stream, char const *__restrict __format, __builtin_va_list __args) { return vfscanf(__file,__format,__args); }
__LOCAL int (__LIBCCALL vscanf_s)(char const *__restrict __format, __builtin_va_list __args) { return vscanf(__format,__args); }
__LOCAL int (__LIBCCALL vsscanf_s)(char const *__restrict __src, char const *__restrict __format, __builtin_va_list __args) { return vsscanf(__src,__format,__args); }
__LOCAL int (__ATTR_CDECL printf_s)(char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vprintf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL fprintf_s)(__FILE *__restrict __stream, char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vfprintf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL sprintf_s)(char *__restrict __buf, size_t __bufsize, char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = __libc_vsnprintf(__buf,__bufsize,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL fscanf_s)(__FILE *__restrict __stream, char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vfscanf(__file,__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL scanf_s)(char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vscanf(__format,__args); __builtin_va_end(__args); return __result; }
__LOCAL int (__ATTR_CDECL sscanf_s)(char const *__restrict __src, char const *__restrict __format, ...) { int __result; __builtin_va_list __args; __builtin_va_start(__args,__format); __result = vsscanf(__src,__format,__args); __builtin_va_end(__args); return __result; }
#endif /* !__CRT_DOS */

#endif /* __USE_DOS_SLIB */

#ifndef _CRT_WPERROR_DEFINED
#define _CRT_WPERROR_DEFINED 1
__REDIRECT_DPB_VOID(__LIBC,__PORT_DOSONLY __ATTR_COLD,__LIBCCALL,wperror,(wchar_t const *__restrict __errmsg),(__errmsg))
#endif /* !_CRT_WPERROR_DEFINED */
#endif  /* _STDIO_DEFINED */

#ifdef __CRT_DOS
#define _fgetc_nolock(stream)    (--(stream)->__f_cnt >= 0 ? 0xff & *(stream)->__f_ptr++ : _filbuf(stream))
#define _fputc_nolock(c,stream)  (--(stream)->__f_cnt >= 0 ? 0xff & (*(stream)->__f_ptr++ = (char)(c)) :  _flsbuf((c),(stream)))
#else /* __CRT_DOS */
#define _fgetc_nolock(stream)    fgetc_unlocked(stream)
#define _fputc_nolock(c,stream)  fputc_unlocked(c,stream)
#endif /* !__CRT_DOS */
#define _getc_nolock(stream)     _fgetc_nolock(stream)
#define _putc_nolock(c,stream)   _fputc_nolock(c, stream)
#define _getchar_nolock()        _getc_nolock(stdin)
#define _putchar_nolock(c)       _putc_nolock((c),stdout)

#ifdef __DOS_COMPAT__
__LIBC void (__LIBCCALL _lock_file)(__FILE *__restrict __stream);
__LIBC void (__LIBCCALL _unlock_file)(__FILE *__restrict __stream);
__LIBC int (__LIBCCALL _fclose_nolock)(__FILE *__restrict __stream);
__LIBC int (__LIBCCALL _fflush_nolock)(__FILE *__restrict __stream);
__LIBC size_t (__LIBCCALL _fread_nolock)(void *__restrict __buf, size_t __elemsize, size_t __elemcount, __FILE *__restrict __stream);
__LIBC size_t (__LIBCCALL _fwrite_nolock)(void const *__restrict __buf, size_t __elemsize, size_t __elemcount, __FILE *__restrict __stream);
#else /* __DOS_COMPAT__ */
__REDIRECT_VOID(__LIBC,,__LIBCCALL,_lock_file,(__FILE *__restrict __stream),flockfile,(__file))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,_unlock_file,(__FILE *__restrict __stream),funlockfile,(__file))
__REDIRECT(__LIBC,,size_t,__LIBCCALL,_fread_nolock,(void *__restrict __buf, size_t __elemsize, size_t __elemcount, __FILE *__restrict __stream),fread_unlocked,(__buf,__elemsize,__elemcount,__file))
__REDIRECT(__LIBC,,size_t,__LIBCCALL,_fwrite_nolock,(void const *__restrict __buf, size_t __elemsize, size_t __elemcount, __FILE *__restrict __stream),fwrite_unlocked,(__buf,__elemsize,__elemcount,__file))
#if defined(__CRT_KOS) && !defined(__GLC_COMPAT__) && !defined(__CYG_COMPAT__)
__REDIRECT(__LIBC,,int,__LIBCCALL,_fclose_nolock,(__FILE *__restrict __stream),fclose_unlocked,(__file))
__REDIRECT(__LIBC,,int,__LIBCCALL,_fflush_nolock,(__FILE *__restrict __stream),fflush_unlocked,(__file))
#else
__REDIRECT(__LIBC,,int,__LIBCCALL,_fclose_nolock,(__FILE *__restrict __stream),fclose,(__file))
__REDIRECT(__LIBC,,int,__LIBCCALL,_fflush_nolock,(__FILE *__restrict __stream),fflush,(__file))
#endif
#endif /* !__DOS_COMPAT__ */

#if defined(__DOS_COMPAT__) || \
   (defined(__CRT_DOS) && !defined(__GLC_COMPAT__) && !defined(__CYG_COMPAT__))
__LIBC size_t (__LIBCCALL _fread_nolock_s)(void *__restrict __buf, size_t __bufsize, size_t __elemsize, size_t __elemcount, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL _fseek_nolock)(__FILE *__restrict __stream, __LONG32_TYPE__ __off, int __whence);
__LIBC __LONG32_TYPE__ (__LIBCCALL _ftell_nolock)(__FILE *__restrict __stream);
__LIBC int (__LIBCCALL _fseeki64_nolock)(__FILE *__restrict __stream, __INT64_TYPE__ __off, int __whence);
__LIBC __INT64_TYPE__ (__LIBCCALL _ftelli64_nolock)(__FILE *__restrict __stream);
__LIBC int (__LIBCCALL _ungetc_nolock)(int __ch, __FILE *__restrict __stream);
#else
__REDIRECT(__LIBC,,int,__LIBCCALL,_ungetc_nolock,(int __ch, __FILE *__restrict __stream),ungetc,(__ch,__file))
__REDIRECT(__LIBC,,size_t,__LIBCCALL,__do_fread_unlocked,(void *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream),fread_unlocked,(__buf,__size,__n,__stream))
__LOCAL size_t (__LIBCCALL _fread_nolock_s)(void *__restrict __buf, size_t __bufsize, size_t __elemsize, size_t __elemcount, __FILE *__restrict __stream) { __bufsize /= __elemsize; return __do_fread_unlocked(__buf,__elemsize,__bufsize < __elemcount ? __bufsize : __elemcount,__file); }
#ifdef __CRT_GLC
__REDIRECT(__LIBC,,int,__LIBCCALL,_fseek_nolock,(__FILE *__restrict __stream, __LONG32_TYPE__ __off, int __whence),fseeko,(__file,__off,__whence))
__REDIRECT(__LIBC,,__LONG32_TYPE__,__LIBCCALL,_ftell_nolock,(__FILE *__restrict __stream),ftello,(__file))
__REDIRECT(__LIBC,,int,__LIBCCALL,_fseeki64_nolock,(__FILE *__restrict __stream, __INT64_TYPE__ __off, int __whence),fseeko64,(__file,__off,__whence))
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_ftelli64_nolock,(__FILE *__restrict __stream),ftello64,(__file))
#elif __SIZEOF_LONG__ == 4
__REDIRECT(__LIBC,,int,__LIBCCALL,_fseek_nolock,(__FILE *__restrict __stream, __LONG32_TYPE__ __off, int __whence),fseek,(__file,__off,__whence))
__REDIRECT(__LIBC,,__LONG32_TYPE__,__LIBCCALL,_ftell_nolock,(__FILE *__restrict __stream),ftell,(__file))
__LOCAL int (__LIBCCALL _fseeki64_nolock)(__FILE *__restrict __stream, __INT64_TYPE__ __off, int __whence) { return fseek(__file,(long int)__off,__whence); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _ftelli64_nolock)(__FILE *__restrict __stream) { return (__INT64_TYPE__)ftell(__file); }
#elif __SIZEOF_LONG__ == 8
__REDIRECT(__LIBC,,int,__LIBCCALL,_fseeki64_nolock,(__FILE *__restrict __stream, __INT64_TYPE__ __off, int __whence),fseek,(__file,__off,__whence))
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,_ftelli64_nolock,(__FILE *__restrict __stream),ftell,(__file))
__LOCAL int (__LIBCCALL _fseek_nolock)(__FILE *__restrict __stream, __LONG32_TYPE__ __off, int __whence) { return fseek(__file,(long int)__off,__whence); }
__LOCAL __LONG32_TYPE__ (__LIBCCALL _ftell_nolock)(__FILE *__restrict __stream) { return (__LONG32_TYPE__)ftell(__file); }
#else /* ... */
__LOCAL int (__LIBCCALL _fseek_nolock)(__FILE *__restrict __stream, __LONG32_TYPE__ __off, int __whence) { return fseek(__file,(long int)__off,__whence); }
__LOCAL __LONG32_TYPE__ (__LIBCCALL _ftell_nolock)(__FILE *__restrict __stream) { return (__LONG32_TYPE__)ftell(__file); }
__LOCAL int (__LIBCCALL _fseeki64_nolock)(__FILE *__restrict __stream, __INT64_TYPE__ __off, int __whence) { return fseek(__file,(long int)__off,__whence); }
__LOCAL __INT64_TYPE__ (__LIBCCALL _ftelli64_nolock)(__FILE *__restrict __stream) { return (__INT64_TYPE__)ftell(__file); }
#endif /* !... */
#endif

#define SYS_OPEN     _SYS_OPEN
__REDIRECT(__LIBC,,int,__LIBCCALL,fgetchar,(void),getchar,())
__REDIRECT(__LIBC,,int,__LIBCCALL,fputchar,(int __ch),putchar,(__ch))
__REDIRECT(__LIBC,,int,__LIBCCALL,flushall,(void),_flushall,())
__REDIRECT(__LIBC,,int,__LIBCCALL,rmtmp,(void),_rmtmp,())

#undef __PRINTF_RETURN_TYPE
#endif /* !__KERNEL__ */

__SYSDECL_END

#ifndef _STDIO_DEFINED
#define _STDIO_DEFINED 1
#include "wstdio.h"
#endif

#endif /* !_PARTS_DOS_STDIO_H */
