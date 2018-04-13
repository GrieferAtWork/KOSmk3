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
#ifndef _PARTS_KOS3_UNISTD_H
#define _PARTS_KOS3_UNISTD_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>
#include <hybrid/limits.h>

__SYSDECL_BEGIN

/* Arguments for the `type' argument of the KOS system calls `xfdname()'
 * NOTE: Strings are generated in a way that ensures validating and
 *       canonicality of the resulting path, following one of these operations:
 *       >> REALPATH_FPATH                   == REALPATH_FHEAD + "/" + REALPATH_FTAIL
 *       >> REALPATH_FPATH|REALPATH_FDOSPATH == REALPATH_FDRIVE + REALPATH_FDRIVEEPATH
 */
#define REALPATH_FPATH      0x0000 /* "/etc/passwd" */
#define REALPATH_FHEAD      0x0001 /* "/etc" */
#define REALPATH_FTAIL      0x0002 /* "passwd" */
#define REALPATH_FDRIVE     0x8003 /* "C:" */
#define REALPATH_FDRIVEPATH 0x8004 /* "\etc\passwd" (path relative to the nearest drive) */
#define REALPATH_FDOSPATH   0x8000 /* FLAG: "C:\etc\passwd"  "C:\etc"  "passwd"
                                    * May be or'd with any of the other type-flags
                                    * to produce the DOS-equivalent of the path.
                                    * If the path of the file handle is mounted on a dos drive,
                                    * the first encountered drive mounting point is used.
                                    * Otherwise, when no mounted drive is encountered along the
                                    * path (and the filesystem root isn't mounted as a drive either),
                                    * `::\etc\passwd' is returned instead (in a sense using `:' as drive name)
                                    * NOTE: If the filesystem mask (s.a. `fsmask') is configured to disable
                                    *       the `AT_DOSPATH' bit, this flag is ignored and forced to be ZERO(0).
                                    *       If the filesystem mask is configured to enable the `AT_DOSPATH'
                                    *       bit, this flag is ignored and forced to be set.
                                    * NOTE: A prefix of `::' is universally (in DOS and UNIX mode) as a prefix
                                    *       that is meant to identify the filesystem root. Coupled that with
                                    *       the fact that `/' is still allowed as directory separator in DOS
                                    *       mode, universal paths can be constructed that are recognized
                                    *       regardless of the current mode: `::/proc/self/exe' */
#define REALPATH_FTYPEMASK  0x003f /* Mask for the fdname type. */
#define REALPATH_FFLAGMASK  0x8000 /* Mask of known flags. */


#ifndef __KERNEL__
#if defined(__USE_KOS) && defined(__CRT_KOS) && __KOS_VERSION__ >= 300
/* Resolve a `PATH' relative to `FD`, using `FLAGS' and fill in
 * the given `BUF...+=BUFSIZE' with its name according to `TYPE'.
 * HINT: `xfdname()' can be implemented as `xfrealpathat(fd,"",AT_EMPTY_PATH,...)'
 * @param: FD:          A file descriptor for the base path to lookup.
 * @param: PATH:        A path relative to `FD' (can be empty when `AT_EMPTY_PATH' is passed)
 * @param: FLAGS:       Set of `AT_*' (AT-path constants)
 * @param: BUF:         A buffer to be filled with the name of the path.
 *                      [xfrealpathat] When NULL, malloc() a new buffer of `BUFSIZE' bytes, or if
 *                                    `BUFSIZE' is ZERO(0), allocate a buffer of appropriate size.
 * @param: BUFSIZE:     The size of the provided `BUF' (in characters)
 * @param: TYPE:        The type of path that should be returned. (Set of `REALPATH_F*'; see above)
 * @return: * :         [xfrealpathat] Either `BUF', or the newly allocated buffer when `BUF' was NULL
 * @return: NULL:       [xfrealpathat] An error occurred (see `errno')
 * @return: * :         [xfrealpathat2] The required buffer size (in characters), excluding a terminated NUL-character
 * @return: >= BUFSIZE: [xfrealpathat2] Only a portion of the path was printed. Pass a buffer capable of holding at least `return+1' characters.
 * @return: -1 :        [xfrealpathat2] An error occurred (see `errno') */
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,char *,__LIBCCALL,xfrealpath,(__fd_t __fd, char *__buf, __size_t __bufsize, unsigned int __type),(__fd,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY,char *,__LIBCCALL,xrealpath,(char const *__path, char *__buf, __size_t __bufsize, unsigned int __type),(__path,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY,char *,__LIBCCALL,xfrealpathat,(__fd_t __dfd, char const *__path, __atflag_t __flags, char *__buf, __size_t __bufsize, unsigned int __type),(__dfd,__path,__flags,__buf,__bufsize,__type))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xfrealpath2,(__fd_t __fd, char *__buf, __size_t __bufsize, unsigned int __type),(__fd,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xrealpath2,(char const *__path, char *__buf, __size_t __bufsize, unsigned int __type),(__path,__buf,__bufsize,__type))
__REDIRECT_EXCEPT_UFS(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xfrealpathat2,(__fd_t __dfd, char const *__path, __atflag_t __flags, char *__buf, __size_t __bufsize, unsigned int __type),(__dfd,__path,__flags,__buf,__bufsize,__type))


/* Extended read/write system calls (s.a. `xreaddirf' and `xioctlf')
 * These system calls behave the same as their non-extended variants,
 * however when invoked, `FLAGS' may specify a set of O_*-style flags
 * to be used during the operation.
 * The main intended use of this is the `O_NONBLOCK' flag, which when
 * pass can be used to make the operation non-blocking without the need
 * to modify file handle flags.
 * Set the set of flags that may be passed is the same as that accepted
 * by the `F_SETFD' `fcntl()' command (which doesn't permit O_ACCMODE)
 * `O_*' bits not accepted by `F_SETFD' are set according to flags that
 * would have been used if the regular operator had been called.
 * Passing flags that are not supported resulting in EINVAL/E_INVALID_ARGUMENT.
 * The following set of flags is currently supported:
 *    - O_APPEND
 *    - O_NONBLOCK
 *    - O_SYNC
 *    - O_ASYNC
 *    - O_DIRECT
 */
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xreadf,(__fd_t __fd, void *__buf, __size_t __bufsize, __oflag_t __flags),(__fd,__buf,__bufsize,__flags))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xwritef,(__fd_t __fd, void const *__buf, __size_t __bufsize, __oflag_t __flags),(__fd,__buf,__bufsize,__flags))
__REDIRECT_EXCEPT_FS64(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xpreadf,(__fd_t __fd, void *__buf, __size_t __bufsize, __FS_TYPE(off) __offset, __oflag_t __flags),(__fd,__buf,__bufsize,__offset,__flags))
__REDIRECT_EXCEPT_FS64(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xpwritef,(__fd_t __fd, void const *__buf, __size_t __bufsize, __FS_TYPE(off) __offset, __oflag_t __flags),(__fd,__buf,__bufsize,__offset,__flags))
#ifdef __USE_LARGEFILE64
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xpreadf64,(__fd_t __fd, void *__buf, __size_t __bufsize, __off64_t __offset, __oflag_t __flags),(__fd,__buf,__bufsize,__offset,__flags))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xpwritef64,(__fd_t __fd, void const *__buf, __size_t __bufsize, __off64_t __offset, __oflag_t __flags),(__fd,__buf,__bufsize,__offset,__flags))
#endif /* __USE_LARGEFILE64 */

#endif /* __USE_KOS && __CRT_KOS && __KOS_VERSION__ >= 300 */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_PARTS_KOS3_UNISTD_H */
