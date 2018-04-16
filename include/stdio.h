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
#ifndef _STDIO_H
#define _STDIO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <bits/stdio_lim.h>
#if !defined(__CRT_GLC) || \
    (defined(__USE_DOS) || defined(__BUILDING_LIBC))
#include <bits/io-file.h>
#endif /* __USE_DOS || __BUILDING_LIBC */
#ifdef __USE_DOS
#include <xlocale.h>
#endif /* __USE_DOS */
#ifdef __CYG_COMPAT__
#include <sys/reent.h>
#endif /* __CYG_COMPAT__ */

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

#ifdef __USE_XOPEN2K8
#ifndef __off_t_defined
#define __off_t_defined 1
typedef __typedef_off_t off_t;
#endif /* !__off_t_defined */

#ifndef __ssize_t_defined
#define __ssize_t_defined 1
typedef __ssize_t ssize_t;
#endif /* !__ssize_t_defined */

#ifdef __USE_LARGEFILE64
#ifndef __off64_t_defined
#define __off64_t_defined 1
typedef __off64_t off64_t;
#endif /* !__off64_t_defined */
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_XOPEN2K8 */

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

/* Dos has different values for some of these.
 * Yet since they don't collide with each other, `setvbuf()' accepts either. */
#define __DOS_IOFBF 0x0000 /* Fully buffered. */
#define __DOS_IOLBF 0x0040 /* Line buffered. */
#define __DOS_IONBF 0x0004 /* No buffering. */

/* The possibilities for the third argument to `setvbuf()'. */
#ifdef __USE_DOS
#define _IOFBF __DOS_IOFBF /* Fully buffered. */
#define _IOLBF __DOS_IOLBF /* Line buffered. */
#define _IONBF __DOS_IONBF /* No buffering. */
#else
#define _IOFBF 0 /* Fully buffered. */
#define _IOLBF 1 /* Line buffered. */
#define _IONBF 2 /* No buffering. */
#endif


/* Default buffer size.  */
#ifndef BUFSIZ
#ifdef __USE_DOS
#define BUFSIZ 512
#else
#define BUFSIZ 8192
#endif
#endif

#ifndef EOF
#define EOF (-1)
#endif

#define SEEK_SET  0 /* Seek from beginning of file. */
#define SEEK_CUR  1 /* Seek from current position. */
#define SEEK_END  2 /* Seek from end of file. */
#ifdef __USE_GNU
#define SEEK_DATA 3 /* Seek to next data. */
#define SEEK_HOLE 4 /* Seek to next hole. */
#endif

#if defined(__USE_MISC) || defined(__USE_XOPEN)
#ifdef __USE_DOS
#define P_tmpdir "\\"
#else
#define P_tmpdir "/tmp"
#endif
#endif

#ifndef __KERNEL__
#ifndef __std_FILE_defined
#define __std_FILE_defined 1
__NAMESPACE_STD_BEGIN
typedef __FILE FILE;
__NAMESPACE_STD_END
#endif /* !__std_FILE_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __FILE_defined
#define __FILE_defined 1
__NAMESPACE_STD_USING(FILE)
#endif /* !__FILE_defined */
#endif /* !__CXX_SYSTEM_HEADER */
#endif /* !__KERNEL__ */

#ifndef __KERNEL__

/* Standard streams. */
#ifndef __stdstreams_defined
#define __stdstreams_defined 1
#undef stdin
#undef stdout
#undef stderr
#ifdef __CYG_COMPAT__
#   define stdin  (__CYG_REENT->__cyg_stdin)
#   define stdout (__CYG_REENT->__cyg_stdout)
#   define stderr (__CYG_REENT->__cyg_stderr)
#elif defined(__DOS_COMPAT__)
#ifdef __USE_DOS_LINKOBJECTS
__LIBC FILE _iob[];
#   define stdin  (_iob+0)
#   define stdout (_iob+1)
#   define stderr (_iob+2)
#else /* __USE_DOS_LINKOBJECTS */
__LIBC FILE *(__LIBCCALL __iob_func)(void);
#   define stdin  (__iob_func()+0)
#   define stdout (__iob_func()+1)
#   define stderr (__iob_func()+2)
#endif /* !__USE_DOS_LINKOBJECTS */
#else /* __DOS_COMPAT__ */
__LIBC __FILE *stdin;
__LIBC __FILE *stdout;
__LIBC __FILE *stderr;
#   define stdin  stdin
#   define stdout stdout
#   define stderr stderr
#endif /* !__DOS_COMPAT__ */
#endif /* !__stdstreams_defined */

#ifdef __USE_KOS
#define __PRINTF_RETURN_TYPE  __ssize_t
#else
#define __PRINTF_RETURN_TYPE  int
#endif

__NAMESPACE_STD_BEGIN
#ifndef __std_remove_defined
#define __std_remove_defined 1
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,remove,(char const *__file),(__file))
#endif /* !__std_remove_defined */
#ifndef __std_rename_defined
#define __std_rename_defined 1
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,rename,(char const *__old, char const *__new),(__old,__new))
#endif /* !__std_rename_defined */
#ifdef __USE_KOS
__REDIRECT_UFS(__LIBC,__WUNUSED,char *,__LIBCCALL,tmpnam,(char __buf[L_tmpnam]),(__buf))
#else /* __USE_KOS */
__REDIRECT_UFS(__LIBC,__WUNUSED,char *,__LIBCCALL,tmpnam,(char *__buf),(__buf))
#endif /* !__USE_KOS */
__LIBC int (__LIBCCALL fclose)(__FILE *__stream);
__LIBC int (__LIBCCALL fflush)(__FILE *__stream);
__LIBC void (__LIBCCALL setbuf)(__FILE *__restrict __stream, char *__restrict __buf);
__LIBC int (__LIBCCALL setvbuf)(__FILE *__restrict __stream, char *__restrict __buf, int __modes, size_t __n);
__LIBC int (__LIBCCALL fgetc)(__FILE *__stream);
__LIBC int (__LIBCCALL getc)(__FILE *__stream);
__LIBC int (__LIBCCALL getchar)(void);
__LIBC int (__LIBCCALL fputc)(int __ch, __FILE *__stream);
__LIBC int (__LIBCCALL putc)(int __ch, __FILE *__stream);
__LIBC int (__LIBCCALL putchar)(int __ch);

#ifdef __USE_KOS_STDEXT
#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
__LIBC __WUNUSED char *(__LIBCCALL fgets)(char *__restrict __buf, size_t __n, __FILE *__restrict __stream);
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
__REDIRECT(__LIBC,__WUNUSED,char *,__LIBCCALL,fgets,(char *__restrict __buf, size_t __n, __FILE *__restrict __stream),fgets_sz,(__buf,__n,__stream))
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */
__LIBC __ssize_t (__LIBCCALL fputs)(char const *__restrict __buf, __FILE *__restrict __stream);
__LIBC __ssize_t (__LIBCCALL puts)(char const *__restrict __str);
#else /* __USE_KOS_STDEXT */
__LIBC __WUNUSED char *(__LIBCCALL fgets)(char *__restrict __buf, int __n, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL fputs)(char const *__restrict __str, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL puts)(char const *__restrict __str);
#endif /* !__USE_KOS_STDEXT */
__LIBC int (__LIBCCALL ungetc)(int __ch, __FILE *__restrict __stream);
__LIBC __WUNUSED size_t (__LIBCCALL fread)(void *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream);
__LIBC size_t (__LIBCCALL fwrite)(void const *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream);
__LIBC int (__LIBCCALL fseek)(__FILE *__restrict __stream, long int __off, int __whence);
__LIBC __WUNUSED long int (__LIBCCALL ftell)(__FILE *__restrict __stream);
__LIBC void (__LIBCCALL rewind)(__FILE *__restrict __stream);
__LIBC void (__LIBCCALL clearerr)(__FILE *__restrict __stream);
__LIBC __WUNUSED int (__LIBCCALL feof)(__FILE *__restrict __stream);
__LIBC __WUNUSED int (__LIBCCALL ferror)(__FILE *__restrict __stream);
#ifndef __perror_defined
__LIBC void (__LIBCCALL perror)(char const *__restrict __message);
#endif /* !__perror_defined */
__REDIRECT_UFS(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,tmpfile,(void),())
__REDIRECT_UFS64(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,fopen,(char const *__restrict __filename, char const *__restrict __modes),(__filename,__modes))
__REDIRECT_UFS64(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,freopen,(char const *__restrict __filename, char const *__restrict __modes, __FILE *__restrict __stream),(__filename,__modes,__stream))
#if defined(__DOS_COMPAT__) && defined(__USE_FILE_OFFSET64)
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_fseeki64,(__FILE *__restrict __stream, __INT64_TYPE__ __off, int __whence),_fseeki64,(__file,__off,__whence))
__REDIRECT(__LIBC,,__INT64_TYPE__,__LIBCCALL,__dos_ftelli64,(__FILE *__restrict __stream),_ftelli64,(__file))
__LOCAL int (__LIBCCALL fsetpos)(__FILE *__restrict __stream, fpos_t const *__restrict __pos) { return __dos_fseeki64(__stream,*__pos,SEEK_SET); }
__LOCAL int (__LIBCCALL fgetpos)(__FILE *__restrict __stream, fpos_t *__restrict __pos) { return (*__pos = __dos_ftelli64(__stream)) < 0; }
#else
__REDIRECT_FS64(__LIBC,,int,__LIBCCALL,fgetpos,(__FILE *__restrict __stream, fpos_t *__restrict __pos),(__stream,__pos))
__REDIRECT_FS64(__LIBC,,int,__LIBCCALL,fsetpos,(__FILE *__restrict __stream, fpos_t const *__restrict __pos),(__stream,__pos))
#endif
__LIBC __ATTR_LIBC_PRINTF(1,2) __PRINTF_RETURN_TYPE (__ATTR_CDECL printf)(char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(2,3) __PRINTF_RETURN_TYPE (__ATTR_CDECL fprintf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(1,0) __PRINTF_RETURN_TYPE (__LIBCCALL vprintf)(char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_PRINTF(2,0) __PRINTF_RETURN_TYPE (__LIBCCALL vfprintf)(__FILE *__restrict __stream, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_SCANF(2,3) __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL fscanf)(__FILE *__restrict __stream, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_SCANF(1,2) __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL scanf)(char const *__restrict __format, ...);
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__LIBC __ATTR_LIBC_SCANF(2,0) __WUNUSED __PRINTF_RETURN_TYPE (__LIBCCALL vfscanf)(__FILE *__restrict __stream, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_SCANF(1,0) __WUNUSED __PRINTF_RETURN_TYPE (__LIBCCALL vscanf)(char const *__restrict __format, __builtin_va_list __args);
#endif /* __USE_ISOC99 || __USE_DOS */
#if !defined(__USE_ISOC11) || \
    (defined(__cplusplus) && __cplusplus <= 201103L)
__LIBC __WUNUSED __ATTR_DEPRECATED("No buffer size checks") char *(__LIBCCALL gets)(char *__restrict __buf);
#endif
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
#ifndef __remove_defined
#define __remove_defined 1
__NAMESPACE_STD_USING(remove)
#endif /* !__remove_defined */
#ifndef __rename_defined
#define __rename_defined 1
__NAMESPACE_STD_USING(rename)
#endif /* !__rename_defined */
#ifndef __perror_defined
#define __perror_defined 1
__NAMESPACE_STD_USING(perror)
#endif /* !__perror_defined */
__NAMESPACE_STD_USING(tmpnam)
__NAMESPACE_STD_USING(fclose)
__NAMESPACE_STD_USING(fflush)
__NAMESPACE_STD_USING(setbuf)
__NAMESPACE_STD_USING(setvbuf)
__NAMESPACE_STD_USING(fgetc)
__NAMESPACE_STD_USING(getc)
__NAMESPACE_STD_USING(getchar)
__NAMESPACE_STD_USING(fputc)
__NAMESPACE_STD_USING(putc)
__NAMESPACE_STD_USING(putchar)
__NAMESPACE_STD_USING(fgets)
__NAMESPACE_STD_USING(fputs)
__NAMESPACE_STD_USING(puts)
__NAMESPACE_STD_USING(ungetc)
__NAMESPACE_STD_USING(fread)
__NAMESPACE_STD_USING(fwrite)
__NAMESPACE_STD_USING(fseek)
__NAMESPACE_STD_USING(ftell)
__NAMESPACE_STD_USING(rewind)
__NAMESPACE_STD_USING(clearerr)
__NAMESPACE_STD_USING(feof)
__NAMESPACE_STD_USING(ferror)
__NAMESPACE_STD_USING(tmpfile)
__NAMESPACE_STD_USING(fopen)
__NAMESPACE_STD_USING(freopen)
__NAMESPACE_STD_USING(fgetpos)
__NAMESPACE_STD_USING(fsetpos)
__NAMESPACE_STD_USING(printf)
__NAMESPACE_STD_USING(fprintf)
__NAMESPACE_STD_USING(vprintf)
__NAMESPACE_STD_USING(vfprintf)
__NAMESPACE_STD_USING(fscanf)
__NAMESPACE_STD_USING(scanf)
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__NAMESPACE_STD_USING(vfscanf)
__NAMESPACE_STD_USING(vscanf)
#endif /* __USE_ISOC99 || __USE_DOS */
#if !defined(__USE_ISOC11) || \
    (defined(__cplusplus) && __cplusplus <= 201103L)
__NAMESPACE_STD_USING(gets)
#endif
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __USE_KOS
/* Reopen the given file stream using a provided file descriptor.
 * @param: MODE: Set of `FDREOPEN_*'
 * @return: STREAM: Successfully re-opened the given file.
 * @return: NULL:   Failed to re-open the file (see `errno' for details) */
__LIBC __PORT_KOSONLY __WUNUSED __FILE *(__LIBCCALL fdreopen)(__fd_t __fd, char const *__restrict __modes,
                                                              __FILE *__restrict __stream, int __mode);
#define FDREOPEN_DUP            0x0 /* Duplicate the given descriptor, creating a private copy for the stream. */
#define FDREOPEN_INHERIT        0x1 /* Inherit the given `fd' on success, using that same number for the stream. */
#define FDREOPEN_CLOSE_ON_ERROR 0x2 /* Close `FD' if an error occurred during the attempt at re-opening it. */
#endif /* __USE_KOS */

#ifdef __CRT_GLC
#ifdef __USE_XOPEN2K8
__LIBC __ATTR_LIBC_PRINTF(2,0) __PORT_NODOS_ALT(fdopen+vfprintf) __PRINTF_RETURN_TYPE (__LIBCCALL vdprintf)(__fd_t __fd, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_PRINTF(2,3) __PORT_NODOS_ALT(fdopen+fprintf)  __PRINTF_RETURN_TYPE (__ATTR_CDECL dprintf)(__fd_t __fd, char const *__restrict __format, ...);
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_KOS
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_NODOS_ALT(remove),int,__LIBCCALL,removeat,(__fd_t __dfd, char const *__filename),(__fd,__filename))
#endif /* __USE_KOS */
#ifdef __USE_ATFILE
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_NODOS_ALT(rename),int,__LIBCCALL,renameat,(int __oldfd, char const *__old, int __newfd, char const *__new),(__oldfd,__old,__newfd,__new))
#ifdef __USE_KOS
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__PORT_KOSONLY_ALT(renameat),int,__LIBCCALL,frenameat,(int __oldfd, char const *__old, int __newfd, char const *__new, int __flags),(__oldfd,__old,__newfd,__new,__flags))
#endif /* __USE_KOS */
#endif /* __USE_ATFILE */
#endif /* __CRT_GLC */

#ifdef __USE_LARGEFILE64
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,tmpfile64,(void),tmpfile,())
__REDIRECT(__LIBC,,int,__LIBCCALL,fseeko64,(__FILE *__stream, __off64_t __off, int __whence),_fseeki64,(__stream,__off,__whence))
__REDIRECT(__LIBC,__WUNUSED,__off64_t,__LIBCCALL,ftello64,(__FILE *__stream),_ftelli64,(__stream))
__REDIRECT_UFS_(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,fopen64,(char const *__restrict __filename, char const *__restrict __modes),fopen,(__filename,__modes))
__REDIRECT_UFS_(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,freopen64,(char const *__restrict __filename, char const *__restrict __modes, __FILE *__restrict __stream),freopen,(__filename,__modes,__stream))
__LOCAL int (__LIBCCALL fgetpos64)(__FILE *__restrict __stream, fpos64_t *__restrict __pos) { return (__pos && (__off64_t)(*__pos = (fpos64_t)ftello64(__stream)) >= 0) ? 0 : -1; }
__LOCAL int (__LIBCCALL fsetpos64)(__FILE *__stream, fpos64_t const *__pos) { return (__pos && fseeko64(__stream,(__off64_t)*__pos,SEEK_SET) >= 0) ? 0 : -1; }
#else /* __DOS_COMPAT__ */
__LIBC __WUNUSED __FILE *(__LIBCCALL tmpfile64)(void);
__LIBC int (__LIBCCALL fseeko64)(__FILE *__stream, __off64_t __off, int __whence);
__LIBC __WUNUSED __off64_t (__LIBCCALL ftello64)(__FILE *__stream);
__REDIRECT_UFS(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,fopen64,(char const *__restrict __filename, char const *__restrict __modes),(__filename,__modes))
__REDIRECT_UFS(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,freopen64,(char const *__restrict __filename, char const *__restrict __modes, __FILE *__restrict __stream),(__filename,__modes,__stream))
__LIBC int (__LIBCCALL fgetpos64)(__FILE *__restrict __stream, fpos64_t *__restrict __pos);
__LIBC int (__LIBCCALL fsetpos64)(__FILE *__stream, fpos64_t const *__pos);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_LARGEFILE64 */
#ifdef __USE_MISC
#ifdef __DOS_COMPAT__
__LOCAL __WUNUSED char *(__LIBCCALL tmpnam_r)(char *__buf) { return __buf ? tmpnam(__buf) : NULL; }
__LOCAL void (__LIBCCALL setbuffer)(__FILE *__restrict __stream, char *__restrict __buf, size_t __size) { setvbuf(__stream,__buf,__buf ? _IOFBF : _IONBF,__buf ? __size : (size_t)0); }
__LOCAL void (__LIBCCALL setlinebuf)(__FILE *__stream) { setvbuf(__stream,NULL,_IOLBF,0); }
__REDIRECT(__LIBC,,int,__LIBCCALL,fflush_unlocked,(__FILE *__stream),_fflush_nolock,(__stream))
__REDIRECT(__LIBC,,size_t,__LIBCCALL,fread_unlocked,(void *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream),_fread_nolock,(__buf,__size,__n,__stream))
__REDIRECT(__LIBC,,size_t,__LIBCCALL,fwrite_unlocked,(void const *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream),_fwrite_nolock,(__buf,__size,__n,__stream))
__REDIRECT(__LIBC,__WUNUSED,int,__LIBCCALL,feof_unlocked,(__FILE *__restrict __stream),feof,(__stream))
__REDIRECT(__LIBC,__WUNUSED,int,__LIBCCALL,ferror_unlocked,(__FILE *__restrict __stream),ferror,(__stream))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,clearerr_unlocked,(__FILE *__stream),clearerr,(__stream))
#else /* __DOS_COMPAT__ */
__REDIRECT_UFS(__LIBC,__WUNUSED,char *,__LIBCCALL,tmpnam_r,(char *__buf),(__buf))
__LIBC void (__LIBCCALL setbuffer)(__FILE *__restrict __stream, char *__restrict __buf, size_t __size);
__LIBC void (__LIBCCALL setlinebuf)(__FILE *__stream);
__LIBC int (__LIBCCALL fflush_unlocked)(__FILE *__stream);
__LIBC size_t (__LIBCCALL fread_unlocked)(void *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream);
__LIBC size_t (__LIBCCALL fwrite_unlocked)(void const *__restrict __buf, size_t __size, size_t __n, __FILE *__restrict __stream);
__LIBC __WUNUSED int (__LIBCCALL feof_unlocked)(__FILE *__restrict __stream);
__LIBC __WUNUSED int (__LIBCCALL ferror_unlocked)(__FILE *__restrict __stream);
__LIBC void (__LIBCCALL clearerr_unlocked)(__FILE *__stream);
#endif /* !__DOS_COMPAT__ */
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__WUNUSED,int,__LIBCCALL,fileno_unlocked,(__FILE *__stream),_fileno,(__stream))
#elif defined(__GLC_COMPAT__)
__LIBC __WUNUSED int (__LIBCCALL fileno_unlocked)(__FILE *__stream);
#else
__REDIRECT(__LIBC,__WUNUSED,int,__LIBCCALL,fileno_unlocked,(__FILE *__stream),fileno,(__stream))
#endif
#ifdef __DOS_COMPAT__
#ifndef ____dos_flsbuf_defined
#define ____dos_flsbuf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_flsbuf,(int __ch, __FILE *__restrict __stream),_flsbuf,(__ch,__file))
#endif /* !____dos_flsbuf_defined */
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_filbuf,(__FILE *__restrict __stream),_filbuf,(__file))
#define fgetc_unlocked(stream)    (--(stream)->__f_cnt >= 0 ? 0xff & *(stream)->__f_ptr++ : __dos_filbuf(stream))
#define fputc_unlocked(c,stream)  (--(stream)->__f_cnt >= 0 ? 0xff & (*(stream)->__f_ptr++ = (char)(c)) :  __dos_flsbuf((c),(stream)))
#else /* __DOS_COMPAT__ */
__LIBC int (__LIBCCALL fgetc_unlocked)(__FILE *__stream);
__LIBC int (__LIBCCALL fputc_unlocked)(int __ch, __FILE *__stream);
#endif /* __DOS_COMPAT__ */
#endif /* __USE_MISC */
#if defined(__USE_MISC) || defined(__USE_XOPEN) || defined(__USE_DOS)
#ifdef __USE_DOSFS
__REDIRECT(__LIBC,__ATTR_MALLOC __WUNUSED,char *,__LIBCCALL,tempnam,(char const *__dir, char const *__pfx),_tempnam,(__dir,__pfx))
#else /* __USE_DOSFS */
__LIBC __ATTR_MALLOC __WUNUSED char *(__LIBCCALL tempnam)(char const *__dir, char const *__pfx);
#endif /* !__USE_DOSFS */
#endif /* __USE_MISC || __USE_XOPEN || __USE_DOS */
#if defined(__USE_POSIX) || defined(__USE_DOS)
__REDIRECT_DPA(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,fdopen,(__fd_t __fd, char const *__restrict __modes),(__fd,__modes))
__REDIRECT_DPA(__LIBC,__WUNUSED,int,__LIBCCALL,fileno,(__FILE *__stream),(__stream))
#endif /* __USE_POSIX || __USE_DOS */
#ifdef __USE_XOPEN2K8
#ifdef __CRT_GLC
__LIBC __PORT_NODOS __WUNUSED __FILE *(__LIBCCALL fmemopen)(void *__mem, size_t __len, char const *__modes);
__LIBC __PORT_NODOS __WUNUSED __FILE *(__LIBCCALL open_memstream)(char **__bufloc, size_t *__sizeloc);
__REDIRECT(__LIBC,__PORT_NODOS __WUNUSED,__ssize_t,__LIBCCALL,__getdelim,(char **__restrict __lineptr, size_t *__restrict __n, int __delimiter, __FILE *__restrict __stream),getdelim,(__lineptr,__n,__delimiter,__stream))
__LIBC __PORT_NODOS __WUNUSED __ssize_t (__LIBCCALL getdelim)(char **__restrict __lineptr, size_t *__restrict __n, int __delimiter, __FILE *__restrict __stream);
__LIBC __PORT_NODOS __WUNUSED __ssize_t (__LIBCCALL getline)(char **__restrict __lineptr, size_t *__restrict __n, __FILE *__restrict __stream);
#endif /* __CRT_GLC */
#endif /* __USE_XOPEN2K8 */
#ifdef __USE_POSIX
#ifdef __DOS_COMPAT__
#ifndef ____dos_flsbuf_defined
#define ____dos_flsbuf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_flsbuf,(int __ch, __FILE *__restrict __stream),_flsbuf,(__ch,__file))
#endif /* !____dos_flsbuf_defined */
__LOCAL int (__LIBCCALL putc_unlocked)(int __ch, __FILE *__stream) { return --__stream->__f_cnt >= 0 ? 0xff & (*__stream->__f_ptr++ = (char)__ch) :  __dos_flsbuf(__ch,__stream); }
__REDIRECT(__LIBC,,int,__LIBCCALL,getc_unlocked,(__FILE *__stream),_getc_nolock,(__stream))
#else /* __DOS_COMPAT__ */
__LIBC int (__LIBCCALL putc_unlocked)(int __ch, __FILE *__stream);
__LIBC int (__LIBCCALL getc_unlocked)(__FILE *__stream);
#endif /* !__DOS_COMPAT__ */
#ifdef __DOS_COMPAT__
__LOCAL int (__LIBCCALL getchar_unlocked)(void) { return getc_unlocked(stdin); }
__LOCAL int (__LIBCCALL putchar_unlocked)(int __ch) { return putc_unlocked(__ch,stdout); }
__REDIRECT_VOID(__LIBC,,__LIBCCALL,flockfile,(__FILE *__stream),_lock_file,(__stream))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,funlockfile,(__FILE *__stream),_unlock_file,(__stream))
#else /* __DOS_COMPAT__ */
__LIBC int (__LIBCCALL getchar_unlocked)(void);
__LIBC int (__LIBCCALL putchar_unlocked)(int __ch);
__LIBC void (__LIBCCALL flockfile)(__FILE *__stream);
__LIBC void (__LIBCCALL funlockfile)(__FILE *__stream);
#endif /* !__DOS_COMPAT__ */
#ifdef __CRT_GLC
#ifndef __ctermid_defined
#define __ctermid_defined 1
__LIBC __PORT_NODOS char *(__LIBCCALL ctermid)(char *__buf);
#endif /* !__ctermid_defined */
__LIBC __PORT_NODOS __WUNUSED int (__LIBCCALL ftrylockfile)(__FILE *__stream);
#endif /* __CRT_GLC */
#endif /* __USE_POSIX */
#ifdef __USE_POSIX2
__REDIRECT_DPA(__LIBC,__WUNUSED,__FILE *,__LIBCCALL,popen,(char const *__command, char const *__modes),(__command,__modes))
__REDIRECT_DPA(__LIBC,,int,__LIBCCALL,pclose,(__FILE *__stream),(__stream))
#endif /* __USE_POSIX2 */
#if defined(__USE_MISC) || defined(__USE_DOS) || \
   (defined(__USE_XOPEN) && !defined(__USE_XOPEN2K))
__REDIRECT_DPA(__LIBC,,int,__LIBCCALL,getw,(__FILE *__stream),(__stream))
__REDIRECT_DPA(__LIBC,,int,__LIBCCALL,putw,(int __w, __FILE *__stream),(__w,__stream))
#endif
#if defined(__USE_GNU) || defined(__USE_DOS)
__REDIRECT_DPA(__LIBC,,int,__LIBCCALL,fcloseall,(void),())
#endif /* __USE_GNU || __USE_DOS */
#ifdef __USE_GNU
#ifdef __CRT_GLC
//__LIBC __PORT_NODOS __WUNUSED __FILE *(__LIBCCALL fopencookie)(void *__restrict __magic_cookie, char const *__restrict __modes, _IO_cookie_io_functions_t __io_funcs);
#ifdef __USE_KOS
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
__LIBC __PORT_NODOS_ALT(fgets) __WUNUSED char *(__LIBCCALL fgets_unlocked)(char *__restrict __buf, size_t __n, __FILE *__restrict __stream);
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
__REDIRECT(__LIBC,__PORT_NODOS_ALT(fgets) __WUNUSED,char *,__LIBCCALL,fgets_unlocked,
          (char *__restrict __buf, size_t __n, __FILE *__restrict __stream),fgets_unlocked_sz,(__buf,__n,__stream))
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */
__LIBC __PORT_NODOS_ALT(fputs) __ssize_t (__LIBCCALL fputs_unlocked)(char const *__restrict __str, __FILE *__restrict __stream);
#else /* __USE_KOS */
__LIBC __PORT_NODOS_ALT(fgets) __WUNUSED char *(__LIBCCALL fgets_unlocked)(char *__restrict __str, int __n, __FILE *__restrict __stream);
__LIBC __PORT_NODOS_ALT(fputs) int (__LIBCCALL fputs_unlocked)(char const *__restrict __str, __FILE *__restrict __stream);
#endif /* !__USE_KOS */
struct obstack;
__LIBC __ATTR_LIBC_PRINTF(2,3) __PORT_NODOS int (__LIBCCALL obstack_printf)(struct obstack *__restrict __obstack, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(2,0) __PORT_NODOS int (__LIBCCALL obstack_vprintf)(struct obstack *__restrict __obstack, char const *__restrict __format, __builtin_va_list __args);
#endif /* __CRT_GLC */
#endif /* __USE_GNU */

#if defined(__USE_LARGEFILE) || defined(__USE_XOPEN2K)
#ifdef __DOS_COMPAT__
#ifdef __USE_FILE_OFFSET64
__REDIRECT(__LIBC,,int,__LIBCCALL,fseeko,(__FILE *__stream, __FS_TYPE(off) __off, int __whence),_fseeki64,(__stream,__off,__whence))
__REDIRECT(__LIBC,__WUNUSED,__FS_TYPE(off),__LIBCCALL,ftello,(__FILE *__stream),_ftelli64,(__stream))
#else /* __USE_FILE_OFFSET64 */
__REDIRECT(__LIBC,,int,__LIBCCALL,fseeko,(__FILE *__stream, __FS_TYPE(off) __off, int __whence),fseek,(__stream,__off,__whence))
__REDIRECT(__LIBC,__WUNUSED,__FS_TYPE(off),__LIBCCALL,ftello,(__FILE *__stream),ftell,(__stream))
#endif /* !__USE_FILE_OFFSET64 */
#else /* __DOS_COMPAT__ */
__REDIRECT_FS64(__LIBC,,int,__LIBCCALL,fseeko,(__FILE *__stream, __FS_TYPE(off) __off, int __whence),(__stream,__off,__whence))
__REDIRECT_FS64(__LIBC,__WUNUSED,__FS_TYPE(off),__LIBCCALL,ftello,(__FILE *__stream),(__stream))
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_LARGEFILE || __USE_XOPEN2K */
#ifdef __USE_XOPEN
#ifdef __CRT_GLC
__LIBC __PORT_NODOS char *(__LIBCCALL cuserid)(char *__buf);
#endif /* __CRT_GLC */
#endif /* __USE_XOPEN */
#ifdef __USE_KOS
/* For use with `format_printf()' and friends: Prints to a `FILE *' closure argument. */
#if !defined(__CRT_KOS) || (defined(__DOS_COMPAT__) || defined(__GLC_COMPAT__))
__LOCAL __ssize_t (__LIBCCALL file_printer)(char const *__restrict __data,
                                            size_t __datalen, void *__closure) {
 return (__ssize_t)fwrite(__data,sizeof(char),__datalen,(FILE *)__closure);
}
#else
__LIBC __ssize_t (__LIBCCALL file_printer)(char const *__restrict __data,
                                           size_t __datalen, void *__closure);
#endif
#endif /* __USE_KOS */
#endif /* !__KERNEL__ */

#ifndef ____libc_vsnprintf_defined
#define ____libc_vsnprintf_defined 1
#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_vsnprintf,(char *__restrict __buf, __size_t __buflen, char const *__restrict __format, __builtin_va_list __args),vsnprintf,(__buf,__buflen,__format,__args))
#ifndef ____dos_vscprintf_defined
#define ____dos_vscprintf_defined 1
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_vscprintf,(char const *__restrict __format, __builtin_va_list __args),_vscprintf,(__format,__args))
#endif /* !____dos_vsnprintf_defined */
__LOCAL int (__LIBCCALL __libc_vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) {
 /* Workaround for DOS's broken vsnprintf() implementation. */
 int __result = __dos_vsnprintf(__buf,__buflen,__format,__args);
 if (__result < 0) __result = __dos_vscprintf(__format,__args);
 return __result;
}
#else /* __DOS_COMPAT__ */
__REDIRECT(__LIBC,,int,__LIBCCALL,__libc_vsnprintf,(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args),vsnprintf,(__buf,__buflen,__format,__args))
#endif /* !__DOS_COMPAT__ */
#endif /* !____libc_vsnprintf_defined */


__NAMESPACE_STD_BEGIN
#if defined(__USE_KOS) && ((defined(__CRT_KOS) && !defined(__GLC_COMPAT__) && !defined(__DOS_COMPAT__)) || __SIZEOF_SIZE_T__ <= __SIZEOF_INT__)
#ifndef __std_sprintf_defined
#define __std_sprintf_defined 1
__LIBC __ATTR_LIBC_PRINTF(2,3) __ssize_t (__ATTR_CDECL sprintf)(char *__restrict __buf, char const *__restrict __format, ...);
#endif /* !__std_sprintf_defined */
__LIBC __ATTR_LIBC_PRINTF(2,0) __ssize_t (__LIBCCALL vsprintf)(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_SCANF(2,3) __size_t (__ATTR_CDECL sscanf)(char const *__restrict __buf, char const *__restrict __format, ...);
#if defined(__USE_ISOC99) || defined(__USE_UNIX98) || defined(__USE_DOS)
#ifdef __DOS_COMPAT__
__LOCAL __ATTR_LIBC_PRINTF(3,0) __ssize_t (__LIBCCALL vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) { return (__ssize_t)__libc_vsnprintf(__buf,__buflen,__format,__args); }
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_LIBC_PRINTF(3,0) __ssize_t (__LIBCCALL vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_ISOC99 || __USE_UNIX98 || __USE_DOS */
#if defined(__USE_ISOC99) || defined(__USE_UNIX98)
#ifndef __std_snprintf_defined
#define __std_snprintf_defined 1
#ifdef __DOS_COMPAT__
__LOCAL __ATTR_LIBC_PRINTF(3,4) __ssize_t (__ATTR_CDECL snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) {
 __ssize_t __result; __builtin_va_list __args; __builtin_va_start(__args,__format);
 __result = __libc_vsnprintf(__buf,__buflen,__format,__args);
 __builtin_va_end(__args);
 return __result;
}
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_LIBC_PRINTF(3,4) __ssize_t (__ATTR_CDECL snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...);
#endif /* !__DOS_COMPAT__ */
#endif /* !__std_snprintf_defined */
#endif /* __USE_ISOC99 || __USE_UNIX98 */
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__LIBC __ATTR_LIBC_SCANF(2,0) __size_t (__LIBCCALL vsscanf)(char const *__restrict __buf, char const *__restrict __format, __builtin_va_list __args);
#endif /* __USE_ISOC99 || __USE_DOS */
#else /* __USE_KOS */
#ifndef __std_sprintf_defined
#define __std_sprintf_defined 1
__LIBC __ATTR_LIBC_PRINTF(2,3) int (__ATTR_CDECL sprintf)(char *__restrict __buf, char const *__restrict __format, ...);
#endif /* !__std_sprintf_defined */
__LIBC __ATTR_LIBC_PRINTF(2,0) int (__LIBCCALL vsprintf)(char *__restrict __buf, char const *__restrict __format, __builtin_va_list __args);
__LIBC __ATTR_LIBC_SCANF(2,3) int (__ATTR_CDECL sscanf)(char const *__restrict __buf, char const *__restrict __format, ...);
#if defined(__USE_ISOC99) || defined(__USE_UNIX98) || defined(__USE_DOS)
#ifdef __DOS_COMPAT__
__LOCAL __ATTR_LIBC_PRINTF(3,0) int (__LIBCCALL vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args) { return __libc_vsnprintf(__buf,__buflen,__format,__args); }
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_LIBC_PRINTF(3,0) int (__LIBCCALL vsnprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, __builtin_va_list __args);
#endif /* !__DOS_COMPAT__ */
#endif /* __USE_ISOC99 || __USE_UNIX98 || __USE_DOS */
#if defined(__USE_ISOC99) || defined(__USE_UNIX98)
#ifndef __std_snprintf_defined
#define __std_snprintf_defined 1
#ifdef __DOS_COMPAT__
__LOCAL __ATTR_LIBC_PRINTF(3,4) int (__ATTR_CDECL snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...) {
 int __result; __builtin_va_list __args; __builtin_va_start(__args,__format);
 __result = __libc_vsnprintf(__buf,__buflen,__format,__args);
 __builtin_va_end(__args);
 return __result;
}
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_LIBC_PRINTF(3,4) int (__ATTR_CDECL snprintf)(char *__restrict __buf, size_t __buflen, char const *__restrict __format, ...);
#endif /* !__DOS_COMPAT__ */
#endif /* !__std_snprintf_defined */
#endif /* __USE_ISOC99 || __USE_UNIX98 */
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__LIBC __ATTR_LIBC_SCANF(2,0) int (__LIBCCALL vsscanf)(char const *__restrict __buf, char const *__restrict __format, __builtin_va_list __args);
#endif /* __USE_ISOC99 || __USE_DOS */
#endif /* !__USE_KOS */
__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
#ifndef __sprintf_defined
#define __sprintf_defined 1
__NAMESPACE_STD_USING(sprintf)
#endif /* !__sprintf_defined */
__NAMESPACE_STD_USING(vsprintf)
__NAMESPACE_STD_USING(sscanf)
#if defined(__USE_ISOC99) || defined(__USE_UNIX98)
#ifndef __snprintf_defined
#define __snprintf_defined 1
__NAMESPACE_STD_USING(snprintf)
#endif /* !__snprintf_defined */
#endif /* __USE_ISOC99 || __USE_UNIX98 */
#if defined(__USE_ISOC99) || defined(__USE_UNIX98) || defined(__USE_DOS)
__NAMESPACE_STD_USING(vsnprintf)
#endif /* __USE_ISOC99 || __USE_UNIX98 || __USE_DOS */
#if defined(__USE_ISOC99) || defined(__USE_DOS)
__NAMESPACE_STD_USING(vsscanf)
#endif /* __USE_ISOC99 || __USE_DOS */
#endif /* !__CXX_SYSTEM_HEADER */


#ifdef __USE_GNU
#if !defined(__KERNEL__) && defined(__CRT_GLC)
__LIBC __ATTR_LIBC_PRINTF(2,3) __PORT_NODOS __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL asprintf)(char **__restrict __pstr, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(2,3) __PORT_NODOS __WUNUSED __PRINTF_RETURN_TYPE (__ATTR_CDECL __asprintf)(char **__restrict __pstr, char const *__restrict __format, ...);
__LIBC __ATTR_LIBC_PRINTF(2,0) __PORT_NODOS __WUNUSED __PRINTF_RETURN_TYPE (__LIBCCALL vasprintf)(char **__restrict __pstr, char const *__restrict __format, __builtin_va_list __args);
#endif /* !__KERNEL__ && __CRT_GLC */
#endif /* __USE_GNU */

#undef __PRINTF_RETURN_TYPE

#if defined(__USE_XOPEN) && !defined(__USE_XOPEN2K) && \
   !defined(__USE_GNU) && defined(__CRT_GLC)
#include <getopt.h>
#endif

__SYSDECL_END

#ifdef __USE_KOS3
#ifndef _PARTS_KOS3_STDIO_H
#include "parts/kos3/stdio.h"
#endif
#endif /* __USE_KOS3 */

#ifdef __USE_DOS
#ifndef _PARTS_DOS_STDIO_H
#include "parts/dos/stdio.h"
#endif
#endif

#ifdef __USE_EXCEPT
#include "parts/kos3/except/stdio.h"
#endif

#endif /* !_STDIO_H */
