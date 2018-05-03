/* Copyright (__c) 2018 Griefer@Work                                            *
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
#ifndef _PARTS_KOS3_STDIO_H
#define _PARTS_KOS3_STDIO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <xlocale.h>

#if defined(__CC__) && !defined(__KERNEL__)
__SYSDECL_BEGIN

#ifndef __std_fpos_t_defined
#define __std_fpos_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __FS_TYPE(pos) fpos_t;
__NAMESPACE_STD_END
#endif /* !__std_fpos_t_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __fpos_t_defined
#define __fpos_t_defined 1
__NAMESPACE_STD_USING(fpos_t)
#endif /* !__fpos_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __USE_LARGEFILE64
#ifndef __fpos64_t_defined
#define __fpos64_t_defined 1
typedef __pos64_t      fpos64_t;
#endif /* !__fpos64_t_defined */
#endif


#ifdef __USE_KOS_STDEXT
#define __PRINTF_RETURN_TYPE  __ssize_t
#else
#define __PRINTF_RETURN_TYPE  int
#endif

#ifdef __CRT_DOS

__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF(1,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,printf_l,(char const *__restrict __format, __locale_t __locale, ...),vprintf_l,(__format,__locale),__locale)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF(2,4),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fprintf_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...),vfprintf_l,(__stream,__format,__locale),__locale)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(1,2),__PRINTF_RETURN_TYPE,__ATTR_CDECL,printf_p,(char const *__restrict __format, ...),vprintf_p,(__format),__format)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(2,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fprintf_p,(__FILE *__restrict __stream, char const *__restrict __format, ...),vfprintf_p,(__stream,__format),__format)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(1,3),__PRINTF_RETURN_TYPE,__ATTR_CDECL,printf_p_l,(char const *__restrict __format, __locale_t __locale, ...),vprintf_p_l,(__format,__locale),__locale)
__VREDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(2,4),__PRINTF_RETURN_TYPE,__ATTR_CDECL,fprintf_p_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...),vfprintf_p_l,(__stream,__format,__locale),__locale)
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vprintf_l,(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfprintf_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__stream,__format,__locale,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vprintf_p,(char const *__restrict __format, __builtin_va_list __args),(__format,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfprintf_p,(__FILE *__restrict __stream, char const *__restrict __format, __builtin_va_list __args),(__stream,__format,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(1,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vprintf_p_l,(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__format,__locale,__args))
__REDIRECT_DPA(__LIBC,__PORT_DOSONLY __ATTR_LIBC_PRINTF_P(2,0),__PRINTF_RETURN_TYPE,__LIBCCALL,vfprintf_p_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args),(__stream,__format,__locale,__args))

#ifdef __CRT_KOS
__VREDIRECT_DPA(__LIBC,__ATTR_LIBC_SCANF(1,2) __WUNUSED,__PRINTF_RETURN_TYPE,__ATTR_CDECL,scanf_l,(char const *__restrict __format, __locale_t __locale, ...),vscanf_l,(__format,__locale),__locale)
__VREDIRECT_DPA(__LIBC,__ATTR_LIBC_SCANF(2,0) __WUNUSED,__PRINTF_RETURN_TYPE,__ATTR_CDECL,fscanf_l,(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...),vfscanf_l,(__stream,__format,__locale),__locale)
__LIBC __PORT_KOSONLY __ATTR_LIBC_SCANF(1,0) __WUNUSED __PRINTF_RETURN_TYPE (__LIBCCALL vscanf_l)(char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __ATTR_LIBC_SCANF(2,0) __WUNUSED __PRINTF_RETURN_TYPE (__LIBCCALL vfscanf_l)(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#else
__LIBC __ATTR_LIBC_SCANF(1,2) __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL _scanf_l)(char const *__restrict __format, __locale_t __locale, ...);
__LIBC __ATTR_LIBC_SCANF(2,0) __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL _fscanf_l)(__FILE *__restrict __stream, char const *__restrict __format, __locale_t __locale, ...);
#define scanf_l(format,locale)             ((_scanf_l)(format,locale))
#define fscanf_l(stream,format,locale,...) ((_fscanf_l)(stream,format,locale,##__VA_ARGS__))
#endif /* __CRT_KOS */
#endif /* __CRT_DOS */

#ifdef __CRT_KOS
#ifdef __USE_XOPEN2K8
__LIBC __ATTR_LIBC_PRINTF(2,0) __PORT_KOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL vdprintf_l)(__fd_t __fd, char const *__restrict __format, __builtin_va_list __args, __locale_t __locale);
__LIBC __ATTR_LIBC_PRINTF(2,4) __PORT_KOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL dprintf_l)(__fd_t __fd, char const *__restrict __format, __locale_t __locale, ...);
__LIBC __ATTR_LIBC_PRINTF_P(2,0) __PORT_KOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL vdprintf_p)(__fd_t __fd, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_PRINTF_P(2,3) __PORT_KOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL dprintf_p)(__fd_t __fd, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF_P(2,0) __PORT_KOSONLY __PRINTF_RETURN_TYPE (__LIBCCALL vdprintf_p_l)(__fd_t __fd, char const *__restrict __format, __builtin_va_list __args, __locale_t __locale);
__LIBC __ATTR_LIBC_PRINTF_P(2,4) __PORT_KOSONLY __PRINTF_RETURN_TYPE (__ATTR_CDECL dprintf_p_l)(__fd_t __fd, char const *__restrict __format, __locale_t __locale, ...);
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_GNU
__LIBC __ATTR_LIBC_PRINTF(2,4) __PORT_KOSONLY __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL asprintf_l)(char **__restrict __pstr, char const *__restrict __format, __locale_t __locale, ...);
__LIBC __ATTR_LIBC_PRINTF(2,0) __PORT_KOSONLY __WUNUSED __PRINTF_RETURN_TYPE (__LIBCCALL vasprintf_l)(char **__restrict __pstr, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __ATTR_LIBC_PRINTF_P(2,3) __PORT_KOSONLY __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL asprintf_p)(char **__restrict __pstr, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF_P(2,0) __PORT_KOSONLY __WUNUSED __PRINTF_RETURN_TYPE (__LIBCCALL vasprintf_p)(char **__restrict __pstr, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_PRINTF_P(2,4) __PORT_KOSONLY __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL asprintf_p_l)(char **__restrict __pstr, char const *__restrict __format, __locale_t __locale, ...);
__LIBC __ATTR_LIBC_PRINTF_P(2,0) __PORT_KOSONLY __WUNUSED __PRINTF_RETURN_TYPE (__LIBCCALL vasprintf_p_l)(char **__restrict __pstr, char const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
#endif /* __USE_GNU */

__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,fseek_unlocked,(__FILE *__restrict __stream, long int __off, int __whence),(__stream,__off,__whence))
__REDIRECT_EXCEPT_FS64_XVOID(__LIBC,,int,__LIBCCALL,fseeko_unlocked,(__FILE *__restrict __stream, __FS_TYPE(off) __off, int __whence),(__stream,__off,__whence))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,long int,__LIBCCALL,ftell_unlocked,(__FILE *__restrict __stream),(__stream))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__FS_TYPE(pos),__FS_TYPE(off)),__LIBCCALL,ftello_unlocked,(__FILE *__restrict __stream),(__stream))
__REDIRECT_EXCEPT_FS64_XVOID(__LIBC,,int,__LIBCCALL,fgetpos_unlocked,(__FILE *__restrict __stream, fpos_t *__restrict __pos),(__stream,__pos))
__REDIRECT_EXCEPT_FS64_XVOID(__LIBC,,int,__LIBCCALL,fsetpos_unlocked,(__FILE *__restrict __stream, fpos_t const *__restrict __pos),(__stream,__pos))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED_SUGGESTED,int,__LIBCCALL,getw_unlocked,(__FILE *__restrict __stream),(__stream))
__REDIRECT_EXCEPT(__LIBC,,int,__LIBCCALL,putw_unlocked,(int __w, __FILE *__restrict __stream),(__w,__stream))
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,setvbuf_unlocked,(__FILE *__restrict __stream, char *__restrict __buf, int __modes, __size_t __count),(__stream,__buf,__modes,__count))
__REDIRECT_EXCEPT(__LIBC,,int,__LIBCCALL,ungetc_unlocked,(int __c, __FILE *__restrict __stream),(__c,__stream))
__REDIRECT_EXCEPT(__LIBC,,__ssize_t,__LIBCCALL,getdelim_unlocked,(char **__restrict __lineptr, __size_t *__restrict __count, int __delimiter, __FILE *__restrict __stream),(__lineptr,__count,__delimiter,__stream))
__REDIRECT_EXCEPT(__LIBC,,__ssize_t,__LIBCCALL,getline_unlocked,(char **__restrict __lineptr, __size_t *__restrict __count, __FILE *__restrict __stream),(__lineptr,__count,__stream))
__LIBC void __LIBCCALL rewind_unlocked(__FILE *__restrict __stream);
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,int,__LIBCCALL,fisatty,(__FILE *__restrict __stream),(__stream))
#ifdef __USE_KOS_STDEXT
__REDIRECT_EXCEPT(__LIBC,,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,puts_unlocked,(char const *__restrict __s),(__s))
#else
__REDIRECT_EXCEPT(__LIBC,,__EXCEPT_SELECT(unsigned int,int),__LIBCCALL,puts_unlocked,(char const *__restrict __s),(__s))
#endif
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,fseeko64_unlocked,(__FILE *__restrict __stream, __off64_t __off, int __whence),(__stream,__off,__whence))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__pos64_t,__off64_t),__LIBCCALL,ftello64_unlocked,(__FILE *__restrict __stream),(__stream))
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,fgetpos64_unlocked,(__FILE *__restrict __stream, fpos64_t *__restrict __pos),(__stream,__pos))
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,fsetpos64_unlocked,(__FILE *__restrict __stream, fpos64_t const *__restrict __pos),(__stream,__pos))
#endif /* __USE_LARGEFILE64 */

#ifdef __USE_EXCEPT
__LIBC __ssize_t (__LIBCCALL Xfile_printer)(char const *__restrict __data, __size_t __datalen, void *__closure);
__LIBC __ssize_t (__LIBCCALL Xfile_printer_unlocked)(char const *__restrict __data, __size_t __datalen, void *__closure);
__LIBC __WUNUSED __size_t (__LIBCCALL Xfread_unlocked)(void *__restrict __buf, __size_t __size, __size_t __count, __FILE *__restrict __stream);
__LIBC __size_t (__LIBCCALL Xfwrite_unlocked)(void const *__restrict __buf, __size_t __size, __size_t __count, __FILE *__restrict __stream);
__LIBC __WUNUSED __size_t (__LIBCCALL Xfread)(void *__restrict __buf, __size_t __size, __size_t __count, __FILE *__restrict __stream);
__LIBC __size_t (__LIBCCALL Xfwrite)(void const *__restrict __buf, __size_t __size, __size_t __count, __FILE *__restrict __stream);
__LIBC void (__LIBCCALL Xflockfile)(__FILE *__restrict __stream);
__LIBC void (__LIBCCALL Xfseek)(__FILE *__restrict __stream, long int __off, int __whence);
__LIBC void (__LIBCCALL Xfseek_unlocked)(__FILE *__restrict __stream, long int __off, int __whence);
__REDIRECT_FS64_VOID(__LIBC,,__LIBCCALL,Xfseeko,(__FILE *__restrict __stream, __FS_TYPE(off) __off, int __whence),(__stream,__off,__whence))
__REDIRECT_FS64_VOID(__LIBC,,__LIBCCALL,Xfseeko_unlocked,(__FILE *__restrict __stream, __FS_TYPE(off) __off, int __whence),(__stream,__off,__whence))
__LIBC unsigned long int (__LIBCCALL Xftell)(__FILE *__restrict __stream);
__LIBC unsigned long int (__LIBCCALL Xftell_unlocked)(__FILE *__restrict __stream);
__REDIRECT_FS64(__LIBC,__WUNUSED,__FS_TYPE(pos),__LIBCCALL,Xftello,(__FILE *__restrict __stream),(__stream))
__REDIRECT_FS64(__LIBC,__WUNUSED,__FS_TYPE(pos),__LIBCCALL,Xftello_unlocked,(__FILE *__restrict __stream),(__stream))
__REDIRECT_FS64_VOID(__LIBC,,__LIBCCALL,Xfgetpos,(__FILE *__restrict __stream, fpos_t *__restrict __pos),(__stream,__pos))
__REDIRECT_FS64_VOID(__LIBC,,__LIBCCALL,Xfgetpos_unlocked,(__FILE *__restrict __stream, fpos_t *__restrict __pos),(__stream,__pos))
__REDIRECT_FS64_VOID(__LIBC,,__LIBCCALL,Xfsetpos,(__FILE *__restrict __stream, fpos_t const *__restrict __pos),(__stream,__pos))
__REDIRECT_FS64_VOID(__LIBC,,__LIBCCALL,Xfsetpos_unlocked,(__FILE *__restrict __stream, fpos_t const *__restrict __pos),(__stream,__pos))
__REDIRECT_FS64(__LIBC,__ATTR_MALLOC __ATTR_RETNONNULL,__FILE *,__LIBCCALL,Xtmpfile,(void),())
__LIBC __ATTR_MALLOC __ATTR_RETNONNULL __FILE *(__LIBCCALL Xfdopen)(__fd_t fd, char const *__restrict __modes);
__REDIRECT_FS64(__LIBC,__ATTR_MALLOC __ATTR_RETNONNULL,__FILE *,__LIBCCALL,Xfopenat,(__fd_t __dfd, char const *__restrict __filename, char const *__restrict __modes, __atflag_t __flags),(__dfd,__filename,__modes,__flags))
__REDIRECT_FS64(__LIBC,__ATTR_MALLOC __ATTR_RETNONNULL,__FILE *,__LIBCCALL,Xfopen,(char const *__restrict __filename, char const *__restrict __modes),(__filename,__modes))
__REDIRECT_FS64(__LIBC,__ATTR_RETNONNULL,__FILE *,__LIBCCALL,Xfreopenat,(__fd_t __dfd, char const *__restrict __filename, char const *__restrict __modes, __atflag_t __flags, __FILE *__restrict __stream),(__dfd,__filename,__modes,__flags,__stream))
__LIBC __ATTR_RETNONNULL __FILE *(__LIBCCALL Xfdreopen)(__fd_t fd, char const *__restrict __modes, __FILE *__restrict __stream, int __mode);
__REDIRECT_FS64(__LIBC,__ATTR_RETNONNULL,__FILE *,__LIBCCALL,Xfreopen,(char const *__restrict __filename, char const *__restrict __modes, __FILE *__restrict __stream),(__filename,__modes,__stream))
__LIBC __ATTR_MALLOC __ATTR_RETNONNULL __FILE *(__LIBCCALL Xfmemopen)(void *__s, __size_t __len, char const *__modes);
__LIBC __ATTR_MALLOC __ATTR_RETNONNULL __FILE *(__LIBCCALL Xopen_memstream)(char **__bufloc, __size_t *__sizeloc);
__LIBC __ATTR_MALLOC __ATTR_RETNONNULL __FILE *(__LIBCCALL Xpopen)(char const *__command, char const *__modes);
//__LIBC __PORT_NODOS __WUNUSED __ATTR_RETNONNULL __FILE *(__LIBCCALL Xfopencookie)(void *__restrict __magic_cookie, char const *__restrict __modes, _IO_cookie_io_functions_t __io_funcs);
__LIBC __WUNUSED_SUGGESTED int (__LIBCCALL Xfgetc)(__FILE *__restrict __stream);
__LIBC __WUNUSED_SUGGESTED int (__LIBCCALL Xfgetc_unlocked)(__FILE *__restrict __stream);
__LIBC int (__LIBCCALL Xfputc)(int __c, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL Xfputc_unlocked)(int __c, __FILE *__restrict __stream);
__LIBC __WUNUSED_SUGGESTED int (__LIBCCALL Xgetw)(__FILE *__restrict __stream);
__LIBC int (__LIBCCALL Xputw)(int __w, __FILE *__restrict __stream);
__LIBC __WUNUSED_SUGGESTED int (__LIBCCALL Xgetw_unlocked)(__FILE *__restrict __stream);
__LIBC int (__LIBCCALL Xputw_unlocked)(int __w, __FILE *__restrict __stream);
#ifdef __USE_KOS_STDEXT
__LIBC __ssize_t (__LIBCCALL Xfputs)(char const *__restrict __s, __FILE *__restrict __stream);
__LIBC __ssize_t (__LIBCCALL Xfputs_unlocked)(char const *__restrict __s, __FILE *__restrict __stream);
#else
__LIBC int (__LIBCCALL Xfputs)(char const *__restrict __s, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL Xfputs_unlocked)(char const *__restrict __s, __FILE *__restrict __stream);
#endif
__LIBC void (__LIBCCALL Xfflush)(__FILE *__self);
__LIBC void (__LIBCCALL Xfflush_unlocked)(__FILE *__self);
__LIBC void (__LIBCCALL Xsetvbuf)(__FILE *__restrict __stream, char *__restrict __buf, int __modes, __size_t __count);
__LIBC void (__LIBCCALL Xsetvbuf_unlocked)(__FILE *__restrict __stream, char *__restrict __buf, int __modes, __size_t __count);
__LIBC int (__LIBCCALL Xungetc)(int __c, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL Xungetc_unlocked)(int __c, __FILE *__restrict __stream);
__LIBC __size_t (__LIBCCALL Xgetdelim)(char **__restrict __lineptr, __size_t *__restrict __count, int __delimiter, __FILE *__restrict __stream);
__LIBC __size_t (__LIBCCALL Xgetdelim_unlocked)(char **__restrict __lineptr, __size_t *__restrict __count, int __delimiter, __FILE *__restrict __stream);
__LIBC __size_t (__LIBCCALL Xgetline)(char **__restrict __lineptr, __size_t *__restrict __count, __FILE *__restrict __stream);
__LIBC __size_t (__LIBCCALL Xgetline_unlocked)(char **__restrict __lineptr, __size_t *__restrict __count, __FILE *__restrict __stream);
#ifdef __USE_KOS_STDEXT
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__LIBC char *(__LIBCCALL Xfgets)(char *__restrict __s, __size_t __count, __FILE *__restrict __stream);
__LIBC char *(__LIBCCALL Xfgets_unlocked)(char *__restrict __s, __size_t __count, __FILE *__restrict __stream);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
__REDIRECT(__LIBC,,char *,__LIBCCALL,Xfgets,(char *__restrict __s, __size_t __count, __FILE *__restrict __stream),Xfgets_sz,(__s,__count,__stream))
__REDIRECT(__LIBC,,char *,__LIBCCALL,Xfgets_unlocked,(char *__restrict __s, __size_t __count, __FILE *__restrict __stream),Xfgets_unlocked_sz,(__s,__count,__stream))
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
__LIBC __size_t __LIBCCALL Xputs(char const *__restrict __s);
__LIBC __size_t __LIBCCALL Xputs_unlocked(char const *__restrict __s);
#else /* __USE_KOS_STDEXT */
__LIBC char *__LIBCCALL Xfgets(char *__restrict __s, int __count, __FILE *__restrict __stream);
__LIBC char *__LIBCCALL Xfgets_unlocked(char *__restrict __s, int __count, __FILE *__restrict __stream);
__LIBC unsigned int __LIBCCALL Xputs(char const *__restrict __s);
__LIBC unsigned int __LIBCCALL Xputs_unlocked(char const *__restrict __s);
#endif /* !__USE_KOS_STDEXT */
__LIBC __WUNUSED_SUGGESTED int (__LIBCCALL Xgetchar)(void);
__LIBC __WUNUSED_SUGGESTED int (__LIBCCALL Xgetchar_unlocked)(void);
__LIBC int (__LIBCCALL Xputchar)(int __c);
__LIBC int (__LIBCCALL Xputchar_unlocked)(int __c);
__LIBC __WUNUSED int (__LIBCCALL Xfisatty)(__FILE *__restrict __stream);
#ifdef __USE_LARGEFILE64
__LIBC void (__LIBCCALL Xfseeko64)(__FILE *__restrict __stream, __off64_t __off, int __whence);
__LIBC void (__LIBCCALL Xfseeko64_unlocked)(__FILE *__restrict __stream, __off64_t __off, int __whence);
__LIBC __pos64_t (__LIBCCALL Xftello64)(__FILE *__restrict __stream);
__LIBC __pos64_t (__LIBCCALL Xftello64_unlocked)(__FILE *__restrict __stream);
__LIBC void (__LIBCCALL Xfgetpos64)(__FILE *__restrict __stream, fpos64_t *__restrict __pos);
__LIBC void (__LIBCCALL Xfgetpos64_unlocked)(__FILE *__restrict __stream, fpos64_t *__restrict __pos);
__LIBC void (__LIBCCALL Xfsetpos64)(__FILE *__restrict __stream, fpos64_t const *__restrict __pos);
__LIBC void (__LIBCCALL Xfsetpos64_unlocked)(__FILE *__restrict __stream, fpos64_t const *__restrict __pos);
__LIBC __ATTR_MALLOC __ATTR_RETNONNULL __FILE *(__LIBCCALL Xtmpfile64)(void);
__LIBC __ATTR_MALLOC __ATTR_RETNONNULL __FILE *(__LIBCCALL Xfopenat64)(__fd_t __dfd, char const *__restrict __filename, char const *__restrict __modes, __atflag_t __flags);
__LIBC __ATTR_MALLOC __ATTR_RETNONNULL __FILE *(__LIBCCALL Xfopen64)(char const *__restrict __filename, char const *__restrict __modes);
__LIBC __ATTR_RETNONNULL __FILE *(__LIBCCALL Xfreopenat64)(__fd_t __dfd, char const *__restrict __filename, char const *__restrict __modes, __atflag_t __flags, __FILE *__restrict __stream);
__LIBC __ATTR_RETNONNULL __FILE *(__LIBCCALL Xfreopen64)(char const *__restrict __filename, char const *__restrict __modes, __FILE *__restrict __stream);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_EXCEPT */
#endif /* __CRT_KOS */
#undef __PRINTF_RETURN_TYPE

__SYSDECL_END
#endif /* __CC__ && !__KERNEL__ */

#ifdef _WCHAR_H
#ifndef _PARTS_KOS3_WSTDIO_H
#include "wstdio.h"
#endif
#endif /* _WCHAR_H */
#ifdef __USE_UTF
#ifdef _UCHAR_H
#ifndef _PARTS_KOS3_USTDIO_H
#include "ustdio.h"
#endif
#endif /* _UCHAR_H */
#endif /* __USE_UTF */

#endif /* !_PARTS_KOS3_STDIO_H */
