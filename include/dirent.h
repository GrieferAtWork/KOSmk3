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
#ifndef _DIRENT_H
#define _DIRENT_H 1

#include "__stdinc.h"
#include <features.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#ifdef __USE_XOPEN
#ifndef __ino_t_defined
#define __ino_t_defined 1
typedef __typedef_ino_t ino_t;
#endif /* !__ino_t_defined */
#ifdef __USE_LARGEFILE64
#ifndef __ino64_t_defined
#define __ino64_t_defined 1
typedef __ino64_t ino64_t;
#endif /* !__ino64_t_defined */
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_XOPEN */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

__SYSDECL_END

#ifndef __DOS_COMPAT__
#include <bits/dirent.h>
__SYSDECL_BEGIN

#ifdef __USE_MISC
enum {
#undef  DT_UNKNOWN
        DT_UNKNOWN = 0,
#define DT_UNKNOWN   0
#undef  DT_FIFO
        DT_FIFO    = 1,
#define DT_FIFO      1
#undef  DT_CHR
        DT_CHR     = 2,
#define DT_CHR       2
#undef  DT_DIR
        DT_DIR     = 4,
#define DT_DIR       4
#undef  DT_BLK
        DT_BLK     = 6,
#define DT_BLK       6
#undef  DT_REG
        DT_REG     = 8,
#define DT_REG       8
#undef  DT_LNK
        DT_LNK     = 10,
#define DT_LNK       10
#undef  DT_SOCK
        DT_SOCK    = 12,
#define DT_SOCK      12
#undef  DT_WHT
        DT_WHT     = 14
#define DT_WHT       14
};

/* Convert between stat structure types and directory types. */
#define IFTODT(mode)    (((mode) & 0170000) >> 12)
#define DTTOIF(dirtype) ((dirtype) << 12)
#endif /* __USE_MISC */

#ifndef __KERNEL__
#ifndef __DIR_defined
#define __DIR_defined 1
typedef struct __dirstream DIR;
#endif /* !__DIR_defined */

/* >> opendir(3)
 * Open and return a new directory stream for reading, referring to `name' */
__REDIRECT_EXCEPT_UFS(__LIBC,__XATTR_RETNONNULL __WUNUSED __NONNULL((1)),DIR *,__LIBCCALL,opendir,(char const *__name),(__name))

#if defined(__CRT_KOS) && defined(__USE_KOS) && defined(__USE_ATFILE)
/* >> opendirat(3), fopendirat(3)
 * Directory-handle-relative, and flags-enabled versions of `opendir(3)' */
__REDIRECT_EXCEPT_UFS(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY_ALT(opendir) __WUNUSED __NONNULL((2)),DIR *,__LIBCCALL,opendirat,(__fd_t __dfd, char const *__name),(__dfd,__name))
__REDIRECT_EXCEPT_UFS(__LIBC,__XATTR_RETNONNULL __PORT_KOSONLY_ALT(opendir) __WUNUSED __NONNULL((2)),DIR *,__LIBCCALL,fopendirat,(__fd_t __dfd, char const *__name, __atflag_t __flags),(__dfd,__name,__flags))
#endif /* __CRT_KOS && __USE_KOS && __USE_ATFILE */

/* >> closedir(3)
 * Close a directory stream previously returned by `opendir(3)' and friends. */
__LIBC __NONNULL((1)) int (__LIBCCALL closedir)(DIR *__dirp);

/* >> readdir(3)
 * Read and return the next pending directory entry of the given directory stream `DIRP'
 * @EXCEPT: Returns NULL for end-of-directory; throws an error if something else went wrong. */
__REDIRECT_EXCEPT_FS64(__LIBC,__NONNULL((1)),struct dirent *,__LIBCCALL,readdir,(DIR *__restrict __dirp),(__dirp))

/* >> rewinddir(3)
 * Rewind the given directory stream in such a way that the next call to `readdir(3)'
 * will once again return the first directory entry. */
__LIBC __PORT_NODOS __NONNULL((1)) void (__LIBCCALL rewinddir)(DIR *__restrict __dirp);

#ifdef __USE_XOPEN2K8
/* >> fopendir(3)
 * Create a new directory stream by inheriting the given `FD' as stream handle. */
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __PORT_NODOS __WUNUSED,DIR *,__LIBCCALL,fdopendir,(__fd_t __fd),(__fd))
#endif /* __USE_XOPEN2K8 */

#ifdef __USE_LARGEFILE64
/* >> readdir64(3)
 * 64-bit equivalent of `readdir(3)'
 * @EXCEPT: Returns NULL for end-of-directory; throws an error if something else went wrong. */
__REDIRECT_EXCEPT(__LIBC,__NONNULL((1)),struct dirent64 *,__LIBCCALL,readdir64,(DIR *__restrict __dirp),(__dirp))
#endif /* __USE_LARGEFILE64 */

#ifdef __USE_POSIX
/* >> readdir_r(3)
 * Reentrant version of `readdir(3)' (Using this is not recommended in KOS). */
#ifdef __USE_FILE_OFFSET64
__REDIRECT_EXCEPT_XVOID_(__LIBC,__PORT_NODOS __NONNULL((1,2,3)),int,__LIBCCALL,
                         readdir_r,(DIR *__restrict __dirp, struct dirent *__restrict __entry, struct dirent **__restrict __result),
                         readdir64_r,(__dirp,__entry,__result))
#else
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS __NONNULL((1,2,3)),int,__LIBCCALL,readdir_r,
                       (DIR *__restrict __dirp, struct dirent *__restrict __entry, struct dirent **__restrict __result),
                       (__dirp,__entry,__result))
#endif

#ifdef __USE_LARGEFILE64
/* NOTE: This ~reentrant~ version of readdir() is strongly discouraged from being used in KOS, as the
 *       kernel does not impose a limit on the length of a single directory entry name (s.a. 'xreaddir')
 * >> Instead, simply use `readdir()'/`readdir64()', which will automatically (re-)allocate an internal,
 *    per-directory buffer of sufficient size to house any directory entry (s.a.: `READDIR_DEFAULT') */
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_NODOS __NONNULL((1,2,3)),int,__LIBCCALL,readdir64_r,
                       (DIR *__restrict __dirp, struct dirent64 *__restrict __entry, struct dirent64 **__restrict __result),
                       (__dirp,__entry,__result))
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_POSIX */


#if defined(__USE_MISC) || defined(__USE_XOPEN)
/* >> seekdir(3), telldir(3)
 * Get/Set the directory stream position. */
__LIBC __PORT_NODOS __NONNULL((1)) void (__LIBCCALL seekdir)(DIR *__restrict __dirp, long int __pos);
__REDIRECT_EXCEPT(__LIBC,__PORT_NODOS __NONNULL((1)),long int,__LIBCCALL,telldir,(DIR *__restrict __dirp),(__dirp))
#endif /* __USE_MISC || __USE_XOPEN */

#ifdef __USE_XOPEN2K8
/* >> dirfd(3)
 * Return the underlying file descriptor of the given directory stream. */
__LIBC __WUNUSED __ATTR_CONST __NONNULL((1)) __fd_t (__LIBCCALL dirfd)(DIR *__restrict __dirp);

#if defined(__USE_MISC) && !defined(MAXNAMLEN)
#define MAXNAMLEN    255 /* == 'NAME_MAX' from <linux/limits.h> */
#endif

/* >> scandir(3)
 * Scan a directory `DIR' for all contained directory entries. */
__REDIRECT_EXCEPT_UFS64(__LIBC,__PORT_NODOS __NONNULL((1,2)),
                        __EXCEPT_SELECT(unsigned int,int),__LIBCCALL,scandir,
                       (char const *__restrict __dir, struct dirent ***__restrict __namelist,
                        int (*__selector)(struct dirent const *),
                        int (*__cmp)(struct dirent const **, struct dirent const **)),
                       (__dir,__namelist,__selector,__cmp))
/* >> alphasort(3)
 * Sort the 2 given directory entries `E1' and `E2' the same way `strcmp(3)' would. */
__REDIRECT_FS64(__LIBC,__PORT_NODOS __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,
                alphasort,(struct dirent const **__e1, struct dirent const **__e2),(__e1,__e2))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_NODOS __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL alphasort64)
      (struct dirent64 const **__e1, struct dirent64 const **__e2);
#endif /* __USE_LARGEFILE64 */

/* >> scandirat(3)
 * Scan a directory `DFD:DIR' for all contained directory entries. */
#ifdef __USE_GNU
__REDIRECT_EXCEPT_UFS64(__LIBC,__PORT_NODOS __NONNULL((2,3)),
                        __EXCEPT_SELECT(unsigned int,int),__LIBCCALL,scandirat,
                       (__fd_t __dfd, char const *__restrict __dir, struct dirent ***__restrict __namelist,
                        int (*__selector)(struct dirent const *),
                        int (*__cmp)(struct dirent const **, struct dirent const **)),
                       (__dfd,__dir,__namelist,__selector,__cmp))
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_NODOS __NONNULL((1,2)),
                      __EXCEPT_SELECT(unsigned int,int),__LIBCCALL,scandir64,
                     (char const *__restrict __dir, struct dirent64 ***__restrict __namelist,
                      int (*__selector)(const struct dirent64 *),
                      int (*__cmp)(const struct dirent64 **, const struct dirent64 **)),
                     (__dir,__namelist,__selector,__cmp))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_NODOS __NONNULL((2,3)),
                      __EXCEPT_SELECT(unsigned int,int),__LIBCCALL,scandirat64,
                     (__fd_t __dfd, char const *__restrict __dir, struct dirent64 ***__restrict __namelist,
                      int (*__selector)(const struct dirent64 *),
                      int (*__cmp)(const struct dirent64 **, const struct dirent64 **)),
                     (__dfd,__dir,__namelist,__selector,__cmp))
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_GNU */
#endif /* __USE_XOPEN2K8 */

/* >> getdirentries(2)
 * Linux's underlying system call for reading the entries of a directory */
#ifdef __USE_MISC
__REDIRECT_EXCEPT_FS64(__LIBC,__PORT_NODOS __NONNULL((2,4)),
                       __EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,getdirentries,
                      (__fd_t __fd, char *__restrict __buf, size_t __nbytes, __FS_TYPE(off) *__restrict __basep),
                      (__fd,__buf,__nbytes,__basep))
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT(__LIBC,__PORT_NODOS __NONNULL((2,4)),
                  __EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,getdirentries64,
                 (__fd_t __fd, char *__restrict __buf, size_t __nbytes, __off64_t *__restrict __basep),
                 (__fd,__buf,__nbytes,__basep))
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_MISC */

/* >> versionsort(3)
 * Sort the 2 given directory entries `E1' and `E2' the same way `strvercmp(3)' would. */
#ifdef __USE_GNU
__REDIRECT_FS64(__LIBC,__PORT_NODOS __ATTR_PURE __NONNULL((1,2)),int,__LIBCCALL,versionsort,
               (struct dirent const **__e1, struct dirent const **__e2),(__e1,__e2))
#ifdef __USE_LARGEFILE64
__LIBC __PORT_NODOS __ATTR_PURE __NONNULL((1,2)) int (__LIBCCALL versionsort64)
      (struct dirent64 const **__e1, struct dirent64 const **__e2);
#endif /* __USE_LARGEFILE64 */
#endif /* __USE_GNU */

/* >> xreaddir(2), xreaddirf(2), xreaddir64(2), xreaddirf64(2)
 * The KOS-specific system call for reading a single directory entry
 * from a file descriptor referring to an open directory stream.
 * @param: MODE: One of `READDIR_*' (See below)
 * @return: * : The actually required buffer size for the directory entry (in bytes)
 *              NOTE: When `READDIR_DEFAULT' was passed for `MODE', the directory
 *                    stream will only be advanced when this value is >= 'BUFSIZE'
 * @return: 0 : The end of the directory has been reached.
 * @return: -1: Failed to read a directory entry for some reason (s.a.: `errno') */
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY_ALT(readdir) __NONNULL((2)) __WUNUSED,
                  __EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xreaddir,
                 (__fd_t __fd, struct dirent *__buf, size_t __bufsize, unsigned int __mode),
                 (__fd,__buf,__bufsize,__mode))
#if __KOS_VERSION__ >= 300
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY_ALT(readdir) __NONNULL((2)) __WUNUSED,
                  __EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xreaddirf,
                 (__fd_t __fd, struct dirent *__buf, size_t __bufsize, unsigned int __mode, __oflag_t __flags),
                 (__fd,__buf,__bufsize,__mode,__flags))
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY_ALT(readdir) __NONNULL((2)) __WUNUSED,
                  __EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xreaddir64,
                 (__fd_t __fd, struct dirent64 *__buf, size_t __bufsize, unsigned int __mode),
                 (__fd,__buf,__bufsize,__mode))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY_ALT(readdir) __NONNULL((2)) __WUNUSED,
                  __EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xreaddirf64,
                 (__fd_t __fd, struct dirent64 *__buf, size_t __bufsize, unsigned int __mode, __oflag_t __flags),
                 (__fd,__buf,__bufsize,__mode,__flags))
#endif /* __USE_LARGEFILE64 */
#else /* __KOS_VERSION__ >= 300 */
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT_(__LIBC,__PORT_KOSONLY_ALT(readdir) __NONNULL((2)) __WUNUSED,
                   __EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xreaddir64,
                  (__fd_t __fd, struct dirent64 *__buf, size_t __bufsize, unsigned int __mode),
                   xreaddir,(__fd,__buf,__bufsize,__mode))
#endif /* __USE_LARGEFILE64 */
#endif /* __KOS_VERSION__ < 300 */
#endif /* !__KERNEL__ */

#if defined(__USE_KOS) && \
   (defined(__CRT_KOS) && !defined(__GLC_COMPAT__))
#ifndef READDIR_DEFAULT
#define READDIR_DEFAULT  0x0000 /* Yield to next entry when `buf' was of sufficient size. */
#define READDIR_CONTINUE 0x0001 /* Always yield to next entry. */
#define READDIR_PEEK     0x0002 /* Never yield to next entry. */
#if __KOS_VERSION__ >= 300
#define READDIR_SKIPREL  0x4000 /* Skip reading the `.' and `..' directory entries. */
#define READDIR_WANTEOF  0x8000 /* Minor optimization for `READDIR_MULTIPLE':
                                 * The system is allowed to append an empty directory entry
                                 * (with `d_namlen = 0' and `d_name[0] = '\0''; other fields are undefined).
                                 * If there isn't enough space for such an entry, no such entry will be emit.
                                 * Since no other directory entries can ever have a length of ZERO(0),
                                 * this allows user-space to detect end-of-directory without the need
                                 * of re-invoking the xreaddir() system call and inspecting its return
                                 * value for being equal to ZERO(0).
                                 * However, that check is still required, as this flag may be ignored
                                 * for no reason immediately apparent (if the EOF entry can't fit into
                                 * the buffer, there's no way of knowing if there's a missing entry that's
                                 * supposed to go into the buffer, or if it was actually an EOF entry).
                                 * Additionally, no eof entry may be written if xreaddir() is invoked
                                 * on a directory handle who's stream position is at the end of the directory.
                                 * For usage, see the example below, as well as `READDIR_MULTIPLE_ISEOF()' */
#define READDIR_MODEMASK 0x001f /* Mask for the xreaddir() mode. */
#define READDIR_FLAGMASK 0xc000 /* Mask of known xreaddir() flags. */
#define READDIR_MODEMAX  0x0003 /* Mask recognized mode ID. */
#define READDIR_MULTIPLE 0x0003 /* Read as many directory entries as can fit into the buffer.
                                 * If at least one entry could be read, return the combined size
                                 * of all read entries (in bytes) (in this case, `return <= bufsize')
                                 * If the buffer was too small to contain the next entry,
                                 * return the required size to house that pending entry,
                                 * but don't yield it, the same way `READDIR_DEFAULT' wouldn't.
                                 * To enumerate multiple directories in some buffer, use the
                                 * macros below. */
/* READDIR_MULTIPLE buffer helpers:
 * >> for (;;) {
 * >>     char buffer[2048]; size_t bufsize;
 * >>     struct dirent *iter = (struct dirent *)buffer;
 * >>     // Read as many entries as our buffer can fit
 * >>     bufsize = xreaddir(fd,iter,sizeof(buffer),
 * >>                        READDIR_MULTIPLE|
 * >>                        READDIR_WANTEOF);
 * >>     if (!bufsize) break; // End of directory
 * >>     if (bufsize > sizeof(buffer)) {
 * >>         printf("A directory entry is too larger for the buffer\n");
 * >>         break;
 * >>     }
 * >>     // Process entries that were read.
 * >>     for (;;) {
 * >>         // This check is only required when `READDIR_WANTEOF' is passed.
 * >>         if (READDIR_MULTIPLE_ISEOF(iter))
 * >>             goto done;
 * >>         printf("Entry: %q\n",iter->d_name);
 * >>         iter = READDIR_MULTIPLE_GETNEXT(iter);
 * >>         if (!READDIR_MULTIPLE_ISVALID(iter,buffer,bufsize))
 * >>             break;
 * >>     }
 * >> }
 * >>done:
 */
#define READDIR_MULTIPLE_GETNEXT(p) \
   ((struct dirent *)(((uintptr_t)((p)->d_name+((p)->d_namlen+1))+ \
                       (sizeof(__ino64_t)-1)) & ~(sizeof(__ino64_t)-1)))
#define READDIR_MULTIPLE_ISVALID(p,buf,bufsize) \
   (((__BYTE_TYPE__ *)((p)->d_name)) < ((__BYTE_TYPE__ *)(buf)+(bufsize)) && \
    ((__BYTE_TYPE__ *)((p)->d_name+(p)->d_namlen)) < ((__BYTE_TYPE__ *)(buf)+(bufsize)))
#define READDIR_MULTIPLE_ISEOF(p) ((p)->d_namlen == 0)
#ifdef __USE_LARGEFILE64
#define READDIR_MULTIPLE_GETNEXT64(p) ((struct dirent64 *)READDIR_MULTIPLE_GETNEXT(p))
#endif /* __USE_LARGEFILE64 */
#endif /* __KOS_VERSION__ >= 300 */
#endif /* !READDIR_DEFAULT */
#endif /* !__DOS_COMPAT__ */

__SYSDECL_END
#elif defined(__CRT_DOS)

#include <errno.h>
#include <hybrid/malloc.h>
#include <hybrid/string.h>

__SYSDECL_BEGIN

#ifdef __USE_LARGEFILE64
#ifdef __INTELLISENSE__
struct dirent64 { __dos_ino_t d_ino; char d_name[260]; };
#else
#define dirent64 dirent
#endif
#endif /* __USE_LARGEFILE64 */

struct dirent {
    __dos_ino_t       d_ino; /* Mandatory */
#ifndef __INTELLISENSE__
    __UINT16_TYPE__ __d_pad;
    /* Members below are arranged for binary compatibility with `struct _finddata32_t' */
    __UINT32_TYPE__ __d_attrib;
    __UINT32_TYPE__ __d_time_create;
    __UINT32_TYPE__ __d_time_access;
    __UINT32_TYPE__ __d_time_write;
    __UINT32_TYPE__ __d_size;
#endif /* !__INTELLISENSE__ */
    char              d_name[260];
};
#define d_fileno d_ino /* Backwards compatibility. */

#undef  _DIRENT_HAVE_D_RECLEN
#undef  _DIRENT_HAVE_D_NAMLEN
#undef  _DIRENT_HAVE_D_TYPE
#define _DIRENT_MATCHES_DIRENT64 1

#ifndef __DIR_defined
#define __DIR_defined 1
typedef struct __dirstream DIR;
#endif /* !__DIR_defined */
struct __dirstream {
    __INTPTR_TYPE__ __d_hnd;
    int             __d_isfirst;
    struct dirent   __d_ent;
};

#ifdef __USE_DOS_LINKOLDFINDSTAT
__REDIRECT_UFS_(__LIBC,__NONNULL((1,2)),__INTPTR_TYPE__,__LIBCCALL,__dos_findfirst,(char const *__file, void *__buf),_findfirst,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,__dos_findnext,(__INTPTR_TYPE__ __findfd, void *__buf),_findnext,(__findfd,__buf))
#else /* __USE_DOS_LINKOLDFINDSTAT */
__REDIRECT_UFS_(__LIBC,__NONNULL((1,2)),__INTPTR_TYPE__,__LIBCCALL,__dos_findfirst,(char const *__file, void *__buf),_findfirst32,(__file,__buf))
__REDIRECT(__LIBC,__NONNULL((2)),int,__LIBCCALL,__dos_findnext,(__INTPTR_TYPE__ __findfd, void *__buf),_findnext32,(__findfd,__buf))
#endif /* !__USE_DOS_LINKOLDFINDSTAT */
__REDIRECT(__LIBC,,int,__LIBCCALL,__dos_findclose,(__INTPTR_TYPE__ __findfd),_findclose,(__findfd))
__LOCAL __WUNUSED __NONNULL((1)) DIR *(__LIBCCALL opendir)(char const *__name);
__LOCAL __NONNULL((1)) int (__LIBCCALL closedir)(DIR *__dirp);
__LOCAL __NONNULL((1)) struct dirent *(__LIBCCALL readdir)(DIR *__dirp);
#ifdef __INTELLISENSE__
#ifdef __USE_LARGEFILE64
__LOCAL __NONNULL((1)) struct dirent64 *(__LIBCCALL readdir64)(DIR *__dirp);
#endif /* __USE_LARGEFILE64 */
#else
__LOCAL DIR *(__LIBCCALL opendir)(char const *__name) {
 DIR *__result; size_t __namelen = __hybrid_strlen(__name);
 char *__query = (char *)__hybrid_malloc((__namelen+3)*sizeof(char));
 if __unlikely(!__query) return __NULLPTR;
 __result = (DIR *)__hybrid_malloc(sizeof(DIR));
 if __unlikely(!__result) goto __end;
 __hybrid_memcpy(__query,__name,__namelen*sizeof(char));
#ifdef __USE_DOSFS
 __query[__namelen]   = '\\';
#else
 __query[__namelen]   = '/';
#endif
 __query[__namelen+1] = '*';
 __query[__namelen+2] = '\0';
 __result->__d_isfirst = 1;
 __result->__d_ent.d_ino = 0;
 __result->__d_hnd = __dos_findfirst(__query,&__result->__d_ent.__d_attrib);
 if __unlikely(__result->__d_hnd == -1) { __hybrid_free(__result); __result = 0; }
__end:
 __hybrid_free(__query);
 return __result;
}
__LOCAL int (__LIBCCALL closedir)(DIR *__dirp) {
 if __unlikely(!__dirp) { __set_errno(EINVAL); return -1; }
 __dos_findclose(__dirp->__d_hnd);
 __hybrid_free(__dirp);
 return 0;
}
__LOCAL struct dirent *(__LIBCCALL readdir)(DIR *__dirp) {
 if __unlikely(!__dirp) { __set_errno(EINVAL); return __NULLPTR; }
 if (!__dirp->__d_isfirst) {
  if (__dos_findnext(__dirp->__d_hnd,&__dirp->__d_ent.__d_attrib))
      return __NULLPTR;
  ++__dirp->__d_ent.d_ino;
 }
 __dirp->__d_isfirst = 0;
 return &__dirp->__d_ent;
}

#ifdef __USE_LARGEFILE64
#define readdir64(dir)                readdir(dir)
#endif /* __USE_LARGEFILE64 */
#endif /* !__INTELLISENSE__ */

__SYSDECL_END
#else
#error "<dirent.h> is not supported by the linked libc"
#endif /* __CRT_GLC */

#ifdef _DIRENT_HAVE_D_NAMLEN
#   define _D_EXACT_NAMLEN(d) ((d)->d_namlen)
#   define _D_ALLOC_NAMLEN(d) (_D_EXACT_NAMLEN(d)+1)
#else
#   define _D_EXACT_NAMLEN(d) strlen((d)->d_name)
#ifdef _DIRENT_HAVE_D_RECLEN
#   define _D_ALLOC_NAMLEN(d) (((char *)(d)+(d)->d_reclen)-&(d)->d_name[0])
#else
#   define _D_ALLOC_NAMLEN(d) (sizeof((d)->d_name) > 1 ? sizeof((d)->d_name) : _D_EXACT_NAMLEN(d)+1)
# endif
#endif

#ifdef __USE_MISC
#ifndef d_fileno
#   define d_ino d_fileno
#endif /* !d_fileno */
#endif

#ifdef __USE_EXCEPT
#include "parts/kos3/except/dirent.h"
#endif

#endif /* !_DIRENT_H */
