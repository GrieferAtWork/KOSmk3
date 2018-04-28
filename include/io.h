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
#ifndef _IO_H
#define _IO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __size_t size_t;
#endif /* !__size_t_defined */

#ifndef __intptr_t_defined
#define __intptr_t_defined 1
typedef __intptr_t intptr_t;
#endif /* !__intptr_t_defined */

#ifndef _FSIZE_T_DEFINED
#define _FSIZE_T_DEFINED
typedef __UINT32_TYPE__ _fsize_t;
#endif /* _FSIZE_T_DEFINED */

struct _finddata32_t;
struct __finddata64_t; /* I guess something else already using the more obvious choice... */
struct _finddata32i64_t;
struct _finddata64i32_t;

#ifndef _A_NORMAL
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20
#endif /* !_A_NORMAL */

#ifndef __KERNEL__

/* Functions with the correct names, also present in other headers. */
#ifndef __std_remove_defined
#define __std_remove_defined 1
__NAMESPACE_STD_BEGIN
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,remove,(char const *__file),(__file))
__NAMESPACE_STD_END
#endif /* !__std_remove_defined */
#ifndef __remove_defined
#define __remove_defined 1
__NAMESPACE_STD_USING(remove)
#endif /* !__remove_defined */
#ifndef __std_rename_defined
#define __std_rename_defined 1
__NAMESPACE_STD_BEGIN
__REDIRECT_EXCEPT_UFS_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,rename,(char const *__old, char const *__new),(__old,__new))
__NAMESPACE_STD_END
#endif /* !__std_rename_defined */
#ifndef __rename_defined
#define __rename_defined 1
__NAMESPACE_STD_USING(rename)
#endif /* !__rename_defined */
#ifndef __unlink_defined
#define __unlink_defined 1
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,unlink,(char const *__name),(__name))
#endif /* !__unlink_defined */
#ifndef __open_defined
#define __open_defined 1
#if defined(__USE_FILE_OFFSET64) && !defined(__DOS_COMPAT__)
__VREDIRECT_EXCEPT_UFS64(__LIBC,__NONNULL((1)),int,__ATTR_CDECL,open,(char const *__file, int __oflag, ...),TODO,(__file,__oflag),__oflag)
#else
__VREDIRECT_EXCEPT_UFSDPA(__LIBC,__NONNULL((1)),int,__ATTR_CDECL,open,(char const *__file, int __oflag, ...),TODO,(__file,__oflag),__oflag)
#endif
#endif /* !__open_defined */
#ifndef __creat_defined
#define __creat_defined 1
#if defined(__USE_FILE_OFFSET64) && !defined(__DOS_COMPAT__)
__REDIRECT_EXCEPT_UFS64(__LIBC,__NONNULL((1)),int,__LIBCCALL,creat,(char const *__file, __mode_t __mode),(__file,__mode))
#else
__REDIRECT_EXCEPT_UFSDPA(__LIBC,__NONNULL((1)),int,__LIBCCALL,creat,(char const *__file, __mode_t __mode),(__file,__mode))
#endif
#endif /* !__creat_defined */
#ifndef __access_defined
#define __access_defined 1
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__IF_NUSE_EXCEPT(__WUNUSED) __NONNULL((1)),int,__LIBCCALL,access,(char const *__name, int __type),(__name,__type))
#endif /* !__access_defined */
#ifndef __chmod_defined
#define __chmod_defined 1
__REDIRECT_EXCEPT_UFSDPA_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,chmod,(char const *__file, int __mode),(__file,__mode))
#endif /* !__chmod_defined */
#ifndef __close_defined
#define __close_defined 1
__REDIRECT_DPA(__LIBC,__CLEANUP,int,__LIBCCALL,close,(__fd_t __fd),(__fd))
#endif /* !__close_defined */
#ifndef __dup_defined
#define __dup_defined 1
__REDIRECT_EXCEPT_DPA(__LIBC,__WUNUSED,__fd_t,__LIBCCALL,dup,(__fd_t __fd),(__fd))
#endif /* !__dup_defined */
#ifndef __dup2_defined
#define __dup2_defined 1
__REDIRECT_EXCEPT_DPA(__LIBC,,__fd_t,__LIBCCALL,dup2,(__fd_t __ofd, __fd_t __nfd),(__ofd,__nfd))
#endif /* !__dup2_defined */
#ifndef __isatty_defined
#define __isatty_defined 1
__REDIRECT_DPA(__LIBC,__WUNUSED,int,__LIBCCALL,isatty,(__fd_t __fd),(__fd))
#endif /* !__isatty_defined */
#ifndef __lseek_defined
#define __lseek_defined 1
__REDIRECT_EXCEPT_DPA(__LIBC,,__LONG32_TYPE__,__LIBCCALL,lseek,(__fd_t __fd, __LONG32_TYPE__ __offset, int __whence),(__fd,__offset,__whence))
#endif /* !__lseek_defined */
#ifndef __mktemp_defined
#define __mktemp_defined 1
__REDIRECT_UFSDPA(__LIBC,__NONNULL((1)),char *,__LIBCCALL,mktemp,(char *__template),(__template))
#endif /* !__mktemp_defined */
#ifndef __umask_defined
#define __umask_defined 1
__REDIRECT_DPA(__LIBC,,__mode_t,__LIBCCALL,umask,(__mode_t __mode),(__mode))
#endif /* !__umask_defined */
#ifndef __read_defined
#define __read_defined 1
#if __SIZEOF_SIZE_T__ <= 4
__REDIRECT_EXCEPT_DPA(__LIBC,__WUNUSED_SUGGESTED __NONNULL((2)),__EXCEPT_SELECT(size_t,__ssize_t),__LIBCCALL,
                      read,(__fd_t __fd, void *__dstbuf, size_t __dstbufsize),(__fd,__dstbuf,__dstbufsize))
#elif defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __NONNULL((2)),__INT32_TYPE__,__LIBCCALL,__read32,(__fd_t __fd, void *__dstbuf, __UINT32_TYPE__ __n_bytes),_read,(__fd,__dstbuf,__n_bytes))
__LOCAL __WUNUSED_SUGGESTED __NONNULL((2)) __ssize_t (__LIBCCALL read)(__fd_t __fd, void *__dstbuf, size_t __n_bytes) { return __read32(__fd,__dstbuf,__n_bytes > (size_t)(__UINT32_TYPE__)-1 ? (__UINT32_TYPE__)-1 : (__UINT32_TYPE__)__n_bytes); }
#else
__REDIRECT_EXCEPT(__LIBC,__WUNUSED_SUGGESTED __NONNULL((2)),__EXCEPT_SELECT(size_t,__ssize_t),__LIBCCALL,
                  read,(__fd_t __fd, void *__dstbuf, size_t __dstbufsize),(__fd,__dstbuf,__dstbufsize))
#endif
#endif /* !__read_defined */
#ifndef __write_defined
#define __write_defined 1
#if __SIZEOF_SIZE_T__ <= 4
__REDIRECT_EXCEPT_DPA(__LIBC,__NONNULL((2)),__EXCEPT_SELECT(size_t,__ssize_t),__LIBCCALL,
                      write,(__fd_t __fd, void *__buf, size_t __bufsize),(__fd,__buf,__bufsize))
#elif defined(__DOS_COMPAT__)
__REDIRECT(__LIBC,__NONNULL((2)),__INT32_TYPE__,__LIBCCALL,__write32,(__fd_t __fd, void *__buf, __UINT32_TYPE__ __n_bytes),_write,(__fd,__buf,__n_bytes))
__LOCAL __NONNULL((2)) __ssize_t (__LIBCCALL write)(__fd_t __fd, void *__buf, size_t __n_bytes) { return __write32(__fd,__buf,__n_bytes > (size_t)(__UINT32_TYPE__)-1 ? (__UINT32_TYPE__)-1 : (__UINT32_TYPE__)__n_bytes); }
#else
__REDIRECT_EXCEPT(__LIBC,__NONNULL((2)),__EXCEPT_SELECT(size_t,__ssize_t),__LIBCCALL,
                  write,(__fd_t __fd, void *__buf, size_t __bufsize),(__fd,__buf,__bufsize))
#endif
#endif /* !__write_defined */


__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,access,(char const *__file, int __type),(__file,__type))
__REDIRECT_EXCEPT_UFSDPB(__LIBC,__WUNUSED __NONNULL((1)),int,__LIBCCALL,creat,(char const *__file, int __pmode),(__file,__pmode))
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,chmod,(char const *__file, int __mode),(__file,__mode))
__REDIRECT_UFSDPB(__LIBC,__NONNULL((1)),errno_t,__LIBCCALL,access_s,(char const *__file, int __type),(__file,__type))
#ifdef __DOS_COMPAT__
__LIBC int (__LIBCCALL _chsize)(__fd_t __fd, __LONG32_TYPE__ __size);
__LIBC errno_t (__LIBCCALL _chsize_s)(__fd_t __fd, __INT64_TYPE__ __size);
__LIBC int (__LIBCCALL _commit)(__fd_t __fd);
__LIBC __LONG32_TYPE__ (__LIBCCALL _lseek)(__fd_t __fd, __LONG32_TYPE__ __offset, int __whence);
__LIBC __INT64_TYPE__ (__LIBCCALL _lseeki64)(__fd_t __fd, __INT64_TYPE__ __offset, int __whence);
__LIBC int (__LIBCCALL _locking)(__fd_t __fd, int __lockmode, __LONG32_TYPE__ __numofbytes);
#else
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,_chsize,(__fd_t __fd, __LONG32_TYPE__ __size),ftruncate,(__fd,__size))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,errno_t,__LIBCCALL,_chsize_s,(__fd_t __fd, __INT64_TYPE__ __size),ftruncate64,(__fd,__size))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,_commit,(__fd_t __fd),fdatasync,(__fd))
__REDIRECT_EXCEPT_(__LIBC,,__EXCEPT_SELECT(__ULONG32_TYPE__,__LONG32_TYPE__),__LIBCCALL,_lseek,(__fd_t __fd, __LONG32_TYPE__ __offset, int __whence),lseek,(__fd,__offset,__whence))
__REDIRECT_EXCEPT_(__LIBC,,__EXCEPT_SELECT(__UINT64_TYPE__,__INT64_TYPE__),__LIBCCALL,_lseeki64,(__fd_t __fd, __INT64_TYPE__ __offset, int __whence),lseek64,(__fd,__offset,__whence))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,_locking,(__fd_t __fd, int __lockmode, __LONG32_TYPE__ __numofbytes),lockf,(__fd,__lockmode,__numofbytes))
#endif

#ifndef ___unlink_defined
#define ___unlink_defined 1
__REDIRECT_EXCEPT_UFSDPB_XVOID(__LIBC,__NONNULL((1)),int,__LIBCCALL,unlink,(char const *__name),(__name))
#endif /* !___unlink_defined */
__REDIRECT_DPB(__LIBC,,int,__LIBCCALL,close,(__fd_t __fd),(__fd))
__REDIRECT_EXCEPT_DPB(__LIBC,,int,__LIBCCALL,dup,(__fd_t __fd),(__fd))
__REDIRECT_EXCEPT_DPB(__LIBC,,int,__LIBCCALL,dup2,(int __ofd, int __nfd),(__ofd,__nfd))

#if __SIZEOF_SIZE_T__ <= 4 || defined(__DOS_COMPAT__)
__REDIRECT_EXCEPT_DPB(__LIBC,__WUNUSED_SUGGESTED __NONNULL((2)),__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,read,(__fd_t __fd, void *__dstbuf, __size_t __bufsize),(__fd,__dstbuf,__bufsize))
__REDIRECT_EXCEPT_DPB(__LIBC,__NONNULL((2)),__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,write,(__fd_t __fd, void const *__buf, __size_t __bufsize),(__fd,__dstbuf,__bufsize))
#else
__REDIRECT_EXCEPT_(__LIBC,,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,__read_sz,(__fd_t __fd, void *__dstbuf, __size_t __bufsize),read,(__fd,__dstbuf,__bufsize))
__REDIRECT_EXCEPT_(__LIBC,,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,__write_sz,(__fd_t __fd, void const *__buf, __size_t __bufsize),write,(__fd,__dstbuf,__bufsize))
__LOCAL __WUNUSED_SUGGESTED __NONNULL((2)) __EXCEPT_SELECT(__UINT32_TYPE__,__INT32_TYPE__) (__LIBCCALL _read)(__fd_t __fd, void *__dstbuf, __UINT32_TYPE__ __bufsize) { return (__EXCEPT_SELECT(__UINT32_TYPE__,__INT32_TYPE__))__read_sz(__fd,__dstbuf,(size_t)__bufsize); }
__LOCAL __NONNULL((2)) __EXCEPT_SELECT(__UINT32_TYPE__,__INT32_TYPE__) (__LIBCCALL _write)(__fd_t __fd, void const *__buf, __UINT32_TYPE__ __bufsize) { return (__EXCEPT_SELECT(__UINT32_TYPE__,__INT32_TYPE__))__write_sz(__fd,__dstbuf,(size_t)__bufsize); }
#endif

__VREDIRECT_EXCEPT_UFSDPB(__LIBC,__WUNUSED,int,__ATTR_CDECL,open,(char const *__file, int __oflag, ...),TODO,(__file,__oflag),__oflag)
__REDIRECT_DPB(__LIBC,,int,__LIBCCALL,setmode,(__fd_t __fd, int __mode),(__fd,__mode))
__REDIRECT_DPB(__LIBC,,__mode_t,__LIBCCALL,umask,(__mode_t __mode),(__mode))
__REDIRECT_DPB(__LIBC,__WUNUSED,int,__LIBCCALL,isatty,(__fd_t __fd),(__fd))

#if defined(__CRT_DOS) && (defined(__DOS_COMPAT__) || !defined(__ANY_COMPAT__))
__LIBC __PORT_DOSONLY_ALT(closedir) int (__LIBCCALL _findclose)(intptr_t __findfd);
#ifdef __USE_DOS_LINKOLDFINDSTAT
__REDIRECT_UFS_(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(opendir) __NONNULL((1,2)),intptr_t,__LIBCCALL,_findfirst32,(char const *__restrict __file, struct _finddata32_t *__restrict __finddata),_findfirst,(__file,__finddata))
__REDIRECT_UFS_(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(opendir) __NONNULL((1,2)),intptr_t,__LIBCCALL,_findfirst32i64,(char const *__restrict __file, struct _finddata32i64_t *__restrict __finddata),_findfirsti64,(__file,__finddata))
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(readdir) __NONNULL((2)),int,__LIBCCALL,_findnext32,(intptr_t __findfd, struct _finddata32_t *__restrict __finddata),_findnext,(__findfd,__finddata))
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(readdir) __NONNULL((2)),int,__LIBCCALL,_findnext32i64,(intptr_t __findfd, struct _finddata32i64_t *__restrict __finddata),_findnexti64,(__findfd,__finddata))
#else /* __USE_DOS_LINKOLDFINDSTAT */
__REDIRECT_UFSDPA(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(opendir) __NONNULL((1,2)),intptr_t,__LIBCCALL,_findfirst32,(char const *__file, struct _finddata32_t *__restrict __finddata),(__file,__finddata))
__REDIRECT_UFSDPA(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(opendir) __NONNULL((1,2)),intptr_t,__LIBCCALL,_findfirst32i64,(char const *__file, struct _finddata32i64_t *__restrict __finddata),(__file,__finddata))
__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(readdir) __NONNULL((2)) int (__LIBCCALL _findnext32)(intptr_t __findfd, struct _finddata32_t *__restrict __finddata);
__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(readdir) __NONNULL((2)) int (__LIBCCALL _findnext32i64)(intptr_t __findfd, struct _finddata32i64_t *__restrict __finddata);
#endif /* !__USE_DOS_LINKOLDFINDSTAT */
__REDIRECT_UFS_(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(opendir) __NONNULL((1,2)),intptr_t,__LIBCCALL,_findfirst64i32,(char const *__restrict __file, struct _finddata64i32_t *__restrict __finddata),_findfirst64,(__file,__finddata))
__REDIRECT_UFS(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(opendir) __NONNULL((1,2)),intptr_t,__LIBCCALL,_findfirst64,(char const *__restrict __file, struct __finddata64_t *__restrict __finddata),(__file,__finddata))
__LIBC __WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(readdir) __NONNULL((2)) int (__LIBCCALL _findnext64)(intptr_t __findfd, struct __finddata64_t *__restrict __finddata);
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(readdir) __NONNULL((2)),int,__LIBCCALL,_findnext64i32,(intptr_t __findfd, struct _finddata64i32_t *__restrict __finddata),_findnext64,(__findfd,__finddata))

__REDIRECT_UFSDPB(__LIBC,__WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(open) __NONNULL((1,2)),errno_t,__LIBCCALL,sopen_s,(int *__fd, char const *__file, int __oflag, int __sflag, int __pmode),(__fd,__file,__oflag,__sflag,__pmode))
#ifdef __USE_DOSFS
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(open) __NONNULL((1,2)),errno_t,__LIBCCALL,_sopen_s_nolock,(int *__fd, char const *__file, int __oflag, int __sflag, int __pmode),__SYMNAME_DOSDP(sopen_s),(__fd,__file,__oflag,__sflag,__pmode))
#else
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __PORT_DOSONLY_ALT(open) __NONNULL((1,2)),errno_t,__LIBCCALL,_sopen_s_nolock,(int *__fd, char const *__file, int __oflag, int __sflag, int __pmode),__SYMNAME_KOS(sopen_s),(__fd,__file,__oflag,__sflag,__pmode))
#endif
__REDIRECT_UFSDPB(__LIBC,__PORT_DOSONLY_ALT(mktemp) __NONNULL((1)),errno_t,__LIBCCALL,mktemp_s,(char *__templatename, size_t __size),(__templatename,__size))
__VREDIRECT_UFSDPB(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(open),int,__ATTR_CDECL,sopen,(char const *__file, int __oflag, int __sflag, ...),TODO,(__file,__oflag,__sflag),__sflag)

__LIBC __WUNUSED_SUGGESTED __NONNULL((1)) int (__LIBCCALL _pipe)(int __pipedes[2], __UINT32_TYPE__ __pipesize, int __textmode);
__LIBC __WUNUSED int (__LIBCCALL _eof)(__fd_t __fd);
__LIBC __WUNUSED __LONG32_TYPE__ (__LIBCCALL _filelength)(__fd_t __fd);
__LIBC __WUNUSED __INT64_TYPE__ (__LIBCCALL _filelengthi64)(__fd_t __fd);
__LIBC __WUNUSED __LONG32_TYPE__ (__LIBCCALL _tell)(__fd_t __fd);
__LIBC __WUNUSED __INT64_TYPE__ (__LIBCCALL _telli64)(__fd_t __fd);
__REDIRECT_DPB(__LIBC,,errno_t,__LIBCCALL,umask_s,(int __newmode, int *__oldmode),(__newmode,__oldmode))
__LIBC int (__LIBCCALL __lock_fhandle)(__fd_t __fd);
__LIBC void (__LIBCCALL _unlock_fhandle)(__fd_t __fd);
__LIBC __WUNUSED intptr_t (__LIBCCALL _get_osfhandle)(__fd_t __fd);
__LIBC __WUNUSED int (__LIBCCALL _open_osfhandle)(intptr_t __osfd, int __flags);
#else /* __CRT_DOS */
__REDIRECT(__LIBC,__WUNUSED_SUGGESTED __NONNULL((1)),int,__LIBCCALL,__os_pipe,(int __pipedes[2]),pipe,(__pipedes))
__LOCAL __WUNUSED_SUGGESTED __NONNULL((1)) int (__LIBCCALL _pipe)(int __pipedes[2], __UINT32_TYPE__ __UNUSED(__pipesize), int __UNUSED(__textmode)) { return __os_pipe(__pipedes); }
__LOCAL __WUNUSED __INT64_TYPE__ (__LIBCCALL _filelengthi64)(__fd_t __fd) {
 __INT64_TYPE__ __oldpos = _lseeki64(__fd,0,1);
 __INT64_TYPE__ __length = __oldpos >= 0 ? _lseeki64(__fd,0,2) : -1;
 return __oldpos >= 0 ? (_lseeki64(__fd,__oldpos,0) >= 0 ? __length : -1) : -1;
}
__LOCAL __WUNUSED int (__LIBCCALL _eof)(__fd_t __fd) {
 __BYTE_TYPE__ __temp; int __error;
 __error = read(__fd,&__temp,1);
 if (__error < 0) return -1;
 if (!__error) return 1;
 lseek(__fd,-1,1);
 return 0;
}
__LOCAL __WUNUSED __LONG32_TYPE__ (__LIBCCALL _filelength)(__fd_t __fd) { return (__LONG32_TYPE__)_filelengthi64(__fd); }
__LOCAL __WUNUSED __LONG32_TYPE__ (__LIBCCALL _tell)(__fd_t __fd) { return _lseek(__fd,0,1); }
__LOCAL __WUNUSED __INT64_TYPE__ (__LIBCCALL _telli64)(__fd_t __fd) { return _lseeki64(__fd,0,1); }
__LOCAL errno_t (__LIBCCALL _umask_s)(int __newmode, int *__oldmode) { *__oldmode = _umask(__newmode); return 0; }
__LOCAL int (__LIBCCALL __lock_fhandle)(int __UNUSED(__fd)) { return 0; }
__LOCAL void (__LIBCCALL _unlock_fhandle)(int __UNUSED(__fd)) { }
__LOCAL __WUNUSED intptr_t (__LIBCCALL _get_osfhandle)(__fd_t __fd) { return __fd; }
__LOCAL int (__LIBCCALL _open_osfhandle)(intptr_t __osfd, int __UNUSED(__flags)) { return _dup((int)__osfd); }
#endif /* __CRT_DOS */

/* Weird, new functions not apart of any standard. */
#ifdef __DOS_COMPAT__
__REDIRECT_DPA(__LIBC,,int,__LIBCCALL,setmode,(__fd_t __fd, int __mode),(__fd,__mode)) /* F_SETFL */
__REDIRECT_DPA(__LIBC,,int,__LIBCCALL,chsize,(__fd_t __fd, __LONG32_TYPE__ __size),(__fd,__size))
__REDIRECT_DPA(__LIBC,,int,__LIBCCALL,locking,(__fd_t __fd, int __lockmode, __LONG32_TYPE__ __numofbytes),(__fd,__lockmode,__numofbytes))
#else
#ifndef __fcntl_defined
#define __fcntl_defined 1
#ifdef __USE_KOS
__VREDIRECT_EXCEPT(__LIBC,__PORT_NODOS,__ssize_t,__ATTR_CDECL,fcntl,(__fd_t __fd, int __cmd, ...),TODO,(__fd,__cmd),__cmd)
#else /* __USE_KOS */
__VREDIRECT_EXCEPT(__LIBC,__PORT_NODOS,int,__ATTR_CDECL,fcntl,(__fd_t __fd, int __cmd, ...),TODO,(__fd,__cmd),__cmd)
#endif /* !__USE_KOS */
#endif /* !__fcntl_defined */
#ifdef __USE_KOS
__LOCAL int (__LIBCCALL setmode)(__fd_t __fd, int __mode) { return fcntl(__fd,__mode,5163); }
#else
__LOCAL int (__LIBCCALL setmode)(__fd_t __fd, int __mode) { fcntl(__fd,__mode,4); return fcntl(__fd,__mode,3); }
#endif
__REDIRECT(__LIBC,,int,__LIBCCALL,chsize,(__fd_t __fd, __LONG32_TYPE__ __size),ftruncate,(__fd,__size))
__REDIRECT(__LIBC,,int,__LIBCCALL,locking,(__fd_t __fd, int __lockmode, __LONG32_TYPE__ __numofbytes),lockf,(__fd,__lockmode,__numofbytes))
#endif
#ifdef __CRT_DOS
__VREDIRECT_UFSDPA(__LIBC,__WUNUSED __PORT_DOSONLY_ALT(open),int,__ATTR_CDECL,sopen,(char const *__file, int __oflag, int __sflag, ...),TODO,(__file,__oflag,__sflag),__sflag);
__REDIRECT(__LIBC,__WUNUSED,__LONG32_TYPE__,__LIBCCALL,filelength,(__fd_t __fd),_filelength,(__fd)) /* lseek(fd,SEEK_END,0) */
__REDIRECT(__LIBC,__WUNUSED,__LONG32_TYPE__,__LIBCCALL,tell,(__fd_t __fd),_tell,(__fd)) /* lseek(fd,SEEK_CUR,0) */
__REDIRECT(__LIBC,__WUNUSED,int,__LIBCCALL,eof,(__fd_t __fd),_eof,(__fd)) /* lseek(fd,SEEK_CUR,0) == lseek(fd,SEEK_END,0) */
#else /* __CRT_DOS */
__LOCAL __WUNUSED __LONG32_TYPE__ (__LIBCCALL filelength)(__fd_t __fd) { return _filelength(__fd); }
__LOCAL __WUNUSED __LONG32_TYPE__ (__LIBCCALL tell)(__fd_t __fd) { return _tell(__fd); }
__LOCAL __WUNUSED int (__LIBCCALL eof)(__fd_t __fd) { return _eof(__fd); }
#endif /* __CRT_DOS */
#endif /* !__KERNEL__ */

/* Safely first! */
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("attrib")
#pragma push_macro("time_create")
#pragma push_macro("time_access")
#pragma push_macro("time_write")
#pragma push_macro("size")
#pragma push_macro("name")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

#undef attrib
#undef time_create
#undef time_access
#undef time_write
#undef size
#undef name

#ifndef _FINDDATA_T_DEFINED
#define _FINDDATA_T_DEFINED 1
struct _finddata32_t {
    __UINT32_TYPE__      attrib;
    __time32_t           time_create;
    __time32_t           time_access;
    __time32_t           time_write;
    _fsize_t             size;
    char                 name[260];
};
struct _finddata32i64_t {
    __UINT32_TYPE__      attrib;
    __time32_t           time_create;
    __time32_t           time_access;
    __time32_t           time_write;
    /* Microsoft:
     * A: "I mean: we could use an unsigned type for this, seeing as how _fsize_t is also unsigned."
     * B: "Nah! - Lets rather p1$$ off anyone that notices. - That'll be way more fun." */
    __INT64_TYPE__       size;
    char                 name[260];
};
struct _finddata64i32_t {
    __UINT32_TYPE__      attrib;
    __time64_t           time_create;
    __time64_t           time_access;
    __time64_t           time_write;
    union {
        _fsize_t         size;
        __INT64_TYPE__ __pad; /* I think this is correct? */
    };
    char                 name[260];
};
struct __finddata64_t {
    __UINT32_TYPE__      attrib;
    __time64_t           time_create;
    __time64_t           time_access;
    __time64_t           time_write;
    __INT64_TYPE__       size;
    char                 name[260];
};


#ifdef __USE_TIME_BITS64
#define _finddata_t     _finddata64i32_t
#define _finddatai64_t  __finddata64_t
#define _findfirst(file,finddata)     _findfirst64i32(file,finddata)
#define _findnext(findfd,finddata)    _findnext64i32(findfd,finddata)
#define _findfirsti64(file,finddata)  _findfirst64(file,finddata)
#define _findnexti64(findfd,finddata) _findnext64(findfd,finddata)
#else /* __USE_TIME_BITS64 */
#define _finddata_t     _finddata32_t
#define _finddatai64_t  _finddata32i64_t
#define _findfirst(file,finddata)     _findfirst32(file,finddata)
#define _findnext(findfd,finddata)    _findnext32(findfd,finddata)
#define _findfirsti64(file,finddata)  _findfirst32i64(file,finddata)
#define _findnexti64(findfd,finddata) _findnext32i64(findfd,finddata)
#endif /* !__USE_TIME_BITS64 */
#endif /* !_FINDDATA_T_DEFINED */

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("name")
#pragma pop_macro("size")
#pragma pop_macro("time_write")
#pragma pop_macro("time_access")
#pragma pop_macro("time_create")
#pragma pop_macro("attrib")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

__SYSDECL_END

#ifdef __USE_DOS
#include "parts/dos/wio.h"
#endif /* __USE_DOS */

#ifdef __USE_EXCEPT
#include "parts/kos3/except/io.h"
#ifdef __USE_DOS
#include "parts/kos3/except/wio.h"
#endif /* __USE_DOS */
#endif

#ifdef __USE_UTF
#ifdef _UCHAR_H
#ifndef _PARTS_KOS2_UIO_H
#include "parts/kos2/uio.h"
#endif
#endif /* _UCHAR_H */
#endif /* __USE_UTF */

#endif /* !_IO_H */
