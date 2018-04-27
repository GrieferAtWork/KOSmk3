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
#ifndef GUARD_LIBS_LIBC_WIDECHAR_H
#define GUARD_LIBS_LIBC_WIDECHAR_H 1

#include "libc.h"

DECL_BEGIN

#ifdef __CC__

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

#define UTF_STACK_BUFFER_SIZE  128
INTDEF char *LIBCCALL libc_loadutf32(char buf[UTF_STACK_BUFFER_SIZE], char32_t const *string);
INTDEF char *LIBCCALL libc_loadutf16(char buf[UTF_STACK_BUFFER_SIZE], char16_t const *string);
INTDEF void LIBCCALL libc_freeutf(char buf[UTF_STACK_BUFFER_SIZE], char *str);
INTDEF ATTR_RETNONNULL char *LIBCCALL libc_Xloadutf32(char buf[UTF_STACK_BUFFER_SIZE], char32_t const *string);
INTDEF ATTR_RETNONNULL char *LIBCCALL libc_Xloadutf16(char buf[UTF_STACK_BUFFER_SIZE], char16_t const *string);

INTDEF int LIBCCALL libc_shm_w16open(char16_t const *name, oflag_t oflag, mode_t mode);
INTDEF int LIBCCALL libc_shm_w32open(char32_t const *name, oflag_t oflag, mode_t mode);
INTDEF int LIBCCALL libc_dos_shm_w16open(char16_t const *name, oflag_t oflag, mode_t mode);
INTDEF int LIBCCALL libc_dos_shm_w32open(char32_t const *name, oflag_t oflag, mode_t mode);
INTDEF int LIBCCALL libc_shm_w16unlink(char16_t const *name);
INTDEF int LIBCCALL libc_shm_w32unlink(char32_t const *name);
INTDEF int LIBCCALL libc_dos_shm_w16unlink(char16_t const *name);
INTDEF int LIBCCALL libc_dos_shm_w32unlink(char32_t const *name);
INTDEF int LIBCCALL libc_Xshm_w16open(char16_t const *name, oflag_t oflag, mode_t mode);
INTDEF int LIBCCALL libc_Xshm_w32open(char32_t const *name, oflag_t oflag, mode_t mode);
INTDEF void LIBCCALL libc_Xshm_w16unlink(char16_t const *name);
INTDEF void LIBCCALL libc_Xshm_w32unlink(char32_t const *name);

INTDEF int ATTR_CDECL libc_w16open(char16_t const *filename, oflag_t flags, ...);
INTDEF int ATTR_CDECL libc_w32open(char32_t const *filename, oflag_t flags, ...);
INTDEF int ATTR_CDECL libc_w16openat(fd_t dfd, char16_t const *filename, oflag_t flags, ...);
INTDEF int ATTR_CDECL libc_w32openat(fd_t dfd, char32_t const *filename, oflag_t flags, ...);
INTDEF int ATTR_CDECL libc_dos_w16open(char16_t const *filename, oflag_t flags, ...);
INTDEF int ATTR_CDECL libc_dos_w32open(char32_t const *filename, oflag_t flags, ...);
INTDEF int ATTR_CDECL libc_dos_w16openat(fd_t dfd, char16_t const *filename, oflag_t flags, ...);
INTDEF int ATTR_CDECL libc_dos_w32openat(fd_t dfd, char32_t const *filename, oflag_t flags, ...);
INTDEF int LIBCCALL libc_w16creat(char16_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_w32creat(char32_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_w16creat(char16_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_w32creat(char32_t const *file, mode_t mode);
INTDEF int ATTR_CDECL libc_Xw16open(char16_t const *filename, oflag_t flags, ...);
INTDEF int ATTR_CDECL libc_Xw32open(char32_t const *filename, oflag_t flags, ...);
INTDEF int ATTR_CDECL libc_Xw16openat(fd_t dfd, char16_t const *filename, oflag_t flags, ...);
INTDEF int ATTR_CDECL libc_Xw32openat(fd_t dfd, char32_t const *filename, oflag_t flags, ...);
INTDEF int LIBCCALL libc_Xw16creat(char16_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_Xw32creat(char32_t const *file, mode_t mode);

INTDEF int LIBCCALL libc_w16chdir(char16_t const *path);
INTDEF int LIBCCALL libc_w32chdir(char32_t const *path);
INTDEF int LIBCCALL libc_dos_w16chdir(char16_t const *path);
INTDEF int LIBCCALL libc_dos_w32chdir(char32_t const *path);
INTDEF int LIBCCALL libc_w16fchdirat(fd_t dfd, char16_t const *path, int flags);
INTDEF int LIBCCALL libc_w32fchdirat(fd_t dfd, char32_t const *path, int flags);
INTDEF int LIBCCALL libc_dos_w16fchdirat(fd_t dfd, char16_t const *path, int flags);
INTDEF int LIBCCALL libc_dos_w32fchdirat(fd_t dfd, char32_t const *path, int flags);
INTDEF char16_t *LIBCCALL libc_w16getcwd(char16_t *buf, size_t size);
INTDEF char32_t *LIBCCALL libc_w32getcwd(char32_t *buf, size_t size);
INTDEF char16_t *LIBCCALL libc_dos_w16getcwd(char16_t *buf, size_t size);
INTDEF char32_t *LIBCCALL libc_dos_w32getcwd(char32_t *buf, size_t size);
INTDEF char16_t *LIBCCALL libd_w16getdcwd(int drive, char16_t *buf, size_t size);
INTDEF char32_t *LIBCCALL libd_w32getdcwd(int drive, char32_t *buf, size_t size);
INTDEF char16_t *LIBCCALL libd_dos_w16getdcwd(int drive, char16_t *buf, size_t size);
INTDEF char32_t *LIBCCALL libd_dos_w32getdcwd(int drive, char32_t *buf, size_t size);
INTDEF ATTR_MALLOC char16_t *LIBCCALL libc_w16get_current_dir_name(void);
INTDEF ATTR_MALLOC char32_t *LIBCCALL libc_w32get_current_dir_name(void);
INTDEF ATTR_MALLOC char16_t *LIBCCALL libc_dos_w16get_current_dir_name(void);
INTDEF ATTR_MALLOC char32_t *LIBCCALL libc_dos_w32get_current_dir_name(void);

INTDEF void LIBCCALL libc_Xw16chdir(char16_t const *path);
INTDEF void LIBCCALL libc_Xw32chdir(char32_t const *path);
INTDEF void LIBCCALL libc_Xw16fchdirat(fd_t dfd, char16_t const *path, int flags);
INTDEF void LIBCCALL libc_Xw32fchdirat(fd_t dfd, char32_t const *path, int flags);
INTDEF ATTR_RETNONNULL char16_t *LIBCCALL libc_Xw16getcwd(char16_t *buf, size_t size);
INTDEF ATTR_RETNONNULL char32_t *LIBCCALL libc_Xw32getcwd(char32_t *buf, size_t size);
INTDEF ATTR_RETNONNULL char16_t *LIBCCALL libc_Xw16getdcwd(int drive, char16_t *buf, size_t size);
INTDEF ATTR_RETNONNULL char32_t *LIBCCALL libc_Xw32getdcwd(int drive, char32_t *buf, size_t size);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char16_t *LIBCCALL libc_Xw16get_current_dir_name(void);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char32_t *LIBCCALL libc_Xw32get_current_dir_name(void);

INTDEF long int LIBCCALL libc_w16pathconf(char16_t const *path, int name);
INTDEF long int LIBCCALL libc_w32pathconf(char32_t const *path, int name);
INTDEF long int LIBCCALL libc_dos_w16pathconf(char16_t const *path, int name);
INTDEF long int LIBCCALL libc_dos_w32pathconf(char32_t const *path, int name);
INTDEF long int LIBCCALL libc_w16fpathconfat(fd_t dfd, char16_t const *file, int name, int flags);
INTDEF long int LIBCCALL libc_w32fpathconfat(fd_t dfd, char32_t const *file, int name, int flags);
INTDEF long int LIBCCALL libc_dos_w16fpathconfat(fd_t dfd, char16_t const *file, int name, int flags);
INTDEF long int LIBCCALL libc_dos_w32fpathconfat(fd_t dfd, char32_t const *file, int name, int flags);
INTDEF long int LIBCCALL libc_Xw16pathconf(char16_t const *path, int name);
INTDEF long int LIBCCALL libc_Xw32pathconf(char32_t const *path, int name);
INTDEF long int LIBCCALL libc_Xw16fpathconfat(fd_t dfd, char16_t const *file, int name, int flags);
INTDEF long int LIBCCALL libc_Xw32fpathconfat(fd_t dfd, char32_t const *file, int name, int flags);

INTDEF int LIBCCALL libc_w16chown(char16_t const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_w32chown(char32_t const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_dos_w16chown(char16_t const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_dos_w32chown(char32_t const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_w16lchown(char16_t const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_w32lchown(char32_t const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_dos_w16lchown(char16_t const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_dos_w32lchown(char32_t const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_w16fchownat(fd_t dfd, char16_t const *file, uid_t owner, gid_t group, int flags);
INTDEF int LIBCCALL libc_w32fchownat(fd_t dfd, char32_t const *file, uid_t owner, gid_t group, int flags);
INTDEF int LIBCCALL libc_dos_w16fchownat(fd_t dfd, char16_t const *file, uid_t owner, gid_t group, int flags);
INTDEF int LIBCCALL libc_dos_w32fchownat(fd_t dfd, char32_t const *file, uid_t owner, gid_t group, int flags);
INTDEF int LIBCCALL libc_w16chmod(char16_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_w32chmod(char32_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_w16chmod(char16_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_w32chmod(char32_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_w16lchmod(char16_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_w32lchmod(char32_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_w16lchmod(char16_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_w32lchmod(char32_t const *file, mode_t mode);
INTDEF int LIBCCALL libc_w16fchmodat(fd_t dfd, char16_t const *file, mode_t mode, int flags);
INTDEF int LIBCCALL libc_w32fchmodat(fd_t dfd, char32_t const *file, mode_t mode, int flags);
INTDEF int LIBCCALL libc_dos_w16fchmodat(fd_t dfd, char16_t const *file, mode_t mode, int flags);
INTDEF int LIBCCALL libc_dos_w32fchmodat(fd_t dfd, char32_t const *file, mode_t mode, int flags);
INTDEF int LIBCCALL libc_w16link(char16_t const *from, char16_t const *to);
INTDEF int LIBCCALL libc_w32link(char32_t const *from, char32_t const *to);
INTDEF int LIBCCALL libc_dos_w16link(char16_t const *from, char16_t const *to);
INTDEF int LIBCCALL libc_dos_w32link(char32_t const *from, char32_t const *to);
INTDEF int LIBCCALL libc_w16linkat(int fromfd, char16_t const *from, int tofd, char16_t const *to, int flags);
INTDEF int LIBCCALL libc_w32linkat(int fromfd, char32_t const *from, int tofd, char32_t const *to, int flags);
INTDEF int LIBCCALL libc_dos_w16linkat(int fromfd, char16_t const *from, int tofd, char16_t const *to, int flags);
INTDEF int LIBCCALL libc_dos_w32linkat(int fromfd, char32_t const *from, int tofd, char32_t const *to, int flags);
INTDEF int LIBCCALL libc_w16unlink(char16_t const *name);
INTDEF int LIBCCALL libc_w32unlink(char32_t const *name);
INTDEF int LIBCCALL libc_dos_w16unlink(char16_t const *name);
INTDEF int LIBCCALL libc_dos_w32unlink(char32_t const *name);
INTDEF int LIBCCALL libc_w16unlinkat(fd_t dfd, char16_t const *name, int flags);
INTDEF int LIBCCALL libc_w32unlinkat(fd_t dfd, char32_t const *name, int flags);
INTDEF int LIBCCALL libc_dos_w16unlinkat(fd_t dfd, char16_t const *name, int flags);
INTDEF int LIBCCALL libc_dos_w32unlinkat(fd_t dfd, char32_t const *name, int flags);
INTDEF int LIBCCALL libc_w16rmdir(char16_t const *name);
INTDEF int LIBCCALL libc_w32rmdir(char32_t const *name);
INTDEF int LIBCCALL libc_dos_w16rmdir(char16_t const *name);
INTDEF int LIBCCALL libc_dos_w32rmdir(char32_t const *name);
INTDEF int LIBCCALL libc_w16remove(char16_t const *name);
INTDEF int LIBCCALL libc_w32remove(char32_t const *name);
INTDEF int LIBCCALL libc_dos_w16remove(char16_t const *name);
INTDEF int LIBCCALL libc_dos_w32remove(char32_t const *name);
INTDEF int LIBCCALL libc_w16removeat(fd_t dfd, char16_t const *name);
INTDEF int LIBCCALL libc_w32removeat(fd_t dfd, char32_t const *name);
INTDEF int LIBCCALL libc_dos_w16removeat(fd_t dfd, char16_t const *name);
INTDEF int LIBCCALL libc_dos_w32removeat(fd_t dfd, char32_t const *name);
INTDEF int LIBCCALL libc_w16rename(char16_t const *oldname, char16_t const *newname);
INTDEF int LIBCCALL libc_w32rename(char32_t const *oldname, char32_t const *newname);
INTDEF int LIBCCALL libc_dos_w16rename(char16_t const *oldname, char16_t const *newname);
INTDEF int LIBCCALL libc_dos_w32rename(char32_t const *oldname, char32_t const *newname);
INTDEF int LIBCCALL libc_w16renameat(fd_t oldfd, char16_t const *oldname, fd_t newfd, char16_t const *newname);
INTDEF int LIBCCALL libc_w32renameat(fd_t oldfd, char32_t const *oldname, fd_t newfd, char32_t const *newname);
INTDEF int LIBCCALL libc_dos_w16renameat(fd_t oldfd, char16_t const *oldname, fd_t newfd, char16_t const *newname);
INTDEF int LIBCCALL libc_dos_w32renameat(fd_t oldfd, char32_t const *oldname, fd_t newfd, char32_t const *newname);
INTDEF int LIBCCALL libc_w16frenameat(fd_t oldfd, char16_t const *oldname, fd_t newfd, char16_t const *newname, int flags);
INTDEF int LIBCCALL libc_w32frenameat(fd_t oldfd, char32_t const *oldname, fd_t newfd, char32_t const *newname, int flags);
INTDEF int LIBCCALL libc_dos_w16frenameat(fd_t oldfd, char16_t const *oldname, fd_t newfd, char16_t const *newname, int flags);
INTDEF int LIBCCALL libc_dos_w32frenameat(fd_t oldfd, char32_t const *oldname, fd_t newfd, char32_t const *newname, int flags);
INTDEF int LIBCCALL libc_w16mkdir1(char16_t const *name);
INTDEF int LIBCCALL libc_w32mkdir1(char32_t const *name);
INTDEF int LIBCCALL libc_dos_w16mkdir1(char16_t const *name);
INTDEF int LIBCCALL libc_dos_w32mkdir1(char32_t const *name);
INTDEF int LIBCCALL libc_w16mkdir(char16_t const *name, mode_t mode);
INTDEF int LIBCCALL libc_w32mkdir(char32_t const *name, mode_t mode);
INTDEF int LIBCCALL libc_dos_w16mkdir(char16_t const *name, mode_t mode);
INTDEF int LIBCCALL libc_dos_w32mkdir(char32_t const *name, mode_t mode);
INTDEF int LIBCCALL libc_w16mkdirat(fd_t dfd, char16_t const *name, mode_t mode);
INTDEF int LIBCCALL libc_w32mkdirat(fd_t dfd, char32_t const *name, mode_t mode);
INTDEF int LIBCCALL libc_dos_w16mkdirat(fd_t dfd, char16_t const *name, mode_t mode);
INTDEF int LIBCCALL libc_dos_w32mkdirat(fd_t dfd, char32_t const *name, mode_t mode);
INTDEF int LIBCCALL libc_w16fmkdirat(fd_t dfd, char16_t const *name, mode_t mode, int flags);
INTDEF int LIBCCALL libc_w32fmkdirat(fd_t dfd, char32_t const *name, mode_t mode, int flags);
INTDEF int LIBCCALL libc_dos_w16fmkdirat(fd_t dfd, char16_t const *name, mode_t mode, int flags);
INTDEF int LIBCCALL libc_dos_w32fmkdirat(fd_t dfd, char32_t const *name, mode_t mode, int flags);
INTDEF int LIBCCALL libc_w16symlink(char16_t const *from, char16_t const *to);
INTDEF int LIBCCALL libc_w32symlink(char32_t const *from, char32_t const *to);
INTDEF int LIBCCALL libc_dos_w16symlink(char16_t const *from, char16_t const *to);
INTDEF int LIBCCALL libc_dos_w32symlink(char32_t const *from, char32_t const *to);
INTDEF int LIBCCALL libc_w16symlinkat(char16_t const *from, int tofd, char16_t const *to);
INTDEF int LIBCCALL libc_w32symlinkat(char32_t const *from, int tofd, char32_t const *to);
INTDEF int LIBCCALL libc_dos_w16symlinkat(char16_t const *from, int tofd, char16_t const *to);
INTDEF int LIBCCALL libc_dos_w32symlinkat(char32_t const *from, int tofd, char32_t const *to);
INTDEF int LIBCCALL libc_w16fsymlinkat(char16_t const *from, int tofd, char16_t const *to, int flags);
INTDEF int LIBCCALL libc_w32fsymlinkat(char32_t const *from, int tofd, char32_t const *to, int flags);
INTDEF int LIBCCALL libc_dos_w16fsymlinkat(char16_t const *from, int tofd, char16_t const *to, int flags);
INTDEF int LIBCCALL libc_dos_w32fsymlinkat(char32_t const *from, int tofd, char32_t const *to, int flags);
INTDEF int LIBCCALL libc_w16mknod(char16_t const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_w32mknod(char32_t const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_dos_w16mknod(char16_t const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_dos_w32mknod(char32_t const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_w16mknodat(fd_t dfd, char16_t const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_w32mknodat(fd_t dfd, char32_t const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_dos_w16mknodat(fd_t dfd, char16_t const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_dos_w32mknodat(fd_t dfd, char32_t const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_w16fmknodat(fd_t dfd, char16_t const *path, mode_t mode, dev_t dev, int flags);
INTDEF int LIBCCALL libc_w32fmknodat(fd_t dfd, char32_t const *path, mode_t mode, dev_t dev, int flags);
INTDEF int LIBCCALL libc_dos_w16fmknodat(fd_t dfd, char16_t const *path, mode_t mode, dev_t dev, int flags);
INTDEF int LIBCCALL libc_dos_w32fmknodat(fd_t dfd, char32_t const *path, mode_t mode, dev_t dev, int flags);
INTDEF int LIBCCALL libc_w16mkfifo(char16_t const *path, mode_t mode);
INTDEF int LIBCCALL libc_w32mkfifo(char32_t const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_w16mkfifo(char16_t const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_w32mkfifo(char32_t const *path, mode_t mode);
INTDEF int LIBCCALL libc_w16mkfifoat(fd_t dfd, char16_t const *path, mode_t mode);
INTDEF int LIBCCALL libc_w32mkfifoat(fd_t dfd, char32_t const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_w16mkfifoat(fd_t dfd, char16_t const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_w32mkfifoat(fd_t dfd, char32_t const *path, mode_t mode);
INTDEF errno_t LIBCCALL libd_w16access_s(char16_t const *file, int type);
INTDEF errno_t LIBCCALL libd_w32access_s(char32_t const *file, int type);
INTDEF derrno_t LIBCCALL libd_dos_w16access_s(char16_t const *file, int type);
INTDEF derrno_t LIBCCALL libd_dos_w32access_s(char32_t const *file, int type);
INTDEF ssize_t LIBCCALL libc_w16freadlink(fd_t fd, char16_t *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_w32freadlink(fd_t fd, char32_t *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_w16readlink(char16_t const *path, char16_t *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_w32readlink(char32_t const *path, char32_t *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_dos_w16readlink(char16_t const *path, char16_t *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_dos_w32readlink(char32_t const *path, char32_t *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_w16readlinkat(fd_t dfd, char16_t const *path, char16_t *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_w32readlinkat(fd_t dfd, char32_t const *path, char32_t *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_dos_w16readlinkat(fd_t dfd, char16_t const *path, char16_t *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_dos_w32readlinkat(fd_t dfd, char32_t const *path, char32_t *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_w16freadlinkat(fd_t dfd, char16_t const *path, char16_t *buf, size_t buflen, int flags);
INTDEF ssize_t LIBCCALL libc_w32freadlinkat(fd_t dfd, char32_t const *path, char32_t *buf, size_t buflen, int flags);
INTDEF ssize_t LIBCCALL libc_dos_w16freadlinkat(fd_t dfd, char16_t const *path, char16_t *buf, size_t buflen, int flags);
INTDEF ssize_t LIBCCALL libc_dos_w32freadlinkat(fd_t dfd, char32_t const *path, char32_t *buf, size_t buflen, int flags);
INTDEF int LIBCCALL libc_w16access(char16_t const *name, int type);
INTDEF int LIBCCALL libc_w32access(char32_t const *name, int type);
INTDEF int LIBCCALL libc_dos_w16access(char16_t const *name, int type);
INTDEF int LIBCCALL libc_dos_w32access(char32_t const *name, int type);
INTDEF int LIBCCALL libc_w16eaccess(char16_t const *name, int type);
INTDEF int LIBCCALL libc_w32eaccess(char32_t const *name, int type);
INTDEF int LIBCCALL libc_dos_w16eaccess(char16_t const *name, int type);
INTDEF int LIBCCALL libc_dos_w32eaccess(char32_t const *name, int type);
INTDEF int LIBCCALL libc_w16faccessat(fd_t dfd, char16_t const *file, int type, int flags);
INTDEF int LIBCCALL libc_w32faccessat(fd_t dfd, char32_t const *file, int type, int flags);
INTDEF int LIBCCALL libc_dos_w16faccessat(fd_t dfd, char16_t const *file, int type, int flags);
INTDEF int LIBCCALL libc_dos_w32faccessat(fd_t dfd, char32_t const *file, int type, int flags);
INTDEF int LIBCCALL libc_w16truncate(char16_t const *file, pos32_t length);
INTDEF int LIBCCALL libc_w32truncate(char32_t const *file, pos32_t length);
INTDEF int LIBCCALL libc_dos_w16truncate(char16_t const *file, pos32_t length);
INTDEF int LIBCCALL libc_dos_w32truncate(char32_t const *file, pos32_t length);
INTDEF int LIBCCALL libc_w16truncate64(char16_t const *file, pos64_t length);
INTDEF int LIBCCALL libc_w32truncate64(char32_t const *file, pos64_t length);
INTDEF int LIBCCALL libc_dos_w16truncate64(char16_t const *file, pos64_t length);
INTDEF int LIBCCALL libc_dos_w32truncate64(char32_t const *file, pos64_t length);
INTDEF int LIBCCALL libc_w16ftruncateat(fd_t dfd, char16_t const *file, pos32_t length, int flags);
INTDEF int LIBCCALL libc_w32ftruncateat(fd_t dfd, char32_t const *file, pos32_t length, int flags);
INTDEF int LIBCCALL libc_dos_w16ftruncateat(fd_t dfd, char16_t const *file, pos32_t length, int flags);
INTDEF int LIBCCALL libc_dos_w32ftruncateat(fd_t dfd, char32_t const *file, pos32_t length, int flags);
INTDEF int LIBCCALL libc_w16ftruncateat64(fd_t dfd, char16_t const *file, pos64_t length, int flags);
INTDEF int LIBCCALL libc_w32ftruncateat64(fd_t dfd, char32_t const *file, pos64_t length, int flags);
INTDEF int LIBCCALL libc_dos_w16ftruncateat64(fd_t dfd, char16_t const *file, pos64_t length, int flags);
INTDEF int LIBCCALL libc_dos_w32ftruncateat64(fd_t dfd, char32_t const *file, pos64_t length, int flags);
INTDEF int LIBCCALL libc_w16chroot(char16_t const *path);
INTDEF int LIBCCALL libc_w32chroot(char32_t const *path);
INTDEF int LIBCCALL libc_dos_w16chroot(char16_t const *path);
INTDEF int LIBCCALL libc_dos_w32chroot(char32_t const *path);


INTDEF void LIBCCALL libc_Xw16chown(char16_t const *file, uid_t owner, gid_t group);
INTDEF void LIBCCALL libc_Xw32chown(char32_t const *file, uid_t owner, gid_t group);
INTDEF void LIBCCALL libc_Xw16lchown(char16_t const *file, uid_t owner, gid_t group);
INTDEF void LIBCCALL libc_Xw32lchown(char32_t const *file, uid_t owner, gid_t group);
INTDEF void LIBCCALL libc_Xw16fchownat(fd_t dfd, char16_t const *file, uid_t owner, gid_t group, int flags);
INTDEF void LIBCCALL libc_Xw32fchownat(fd_t dfd, char32_t const *file, uid_t owner, gid_t group, int flags);
INTDEF void LIBCCALL libc_Xw16chmod(char16_t const *file, mode_t mode);
INTDEF void LIBCCALL libc_Xw32chmod(char32_t const *file, mode_t mode);
INTDEF void LIBCCALL libc_Xw16lchmod(char16_t const *file, mode_t mode);
INTDEF void LIBCCALL libc_Xw32lchmod(char32_t const *file, mode_t mode);
INTDEF void LIBCCALL libc_Xw16fchmodat(fd_t dfd, char16_t const *file, mode_t mode, int flags);
INTDEF void LIBCCALL libc_Xw32fchmodat(fd_t dfd, char32_t const *file, mode_t mode, int flags);
INTDEF void LIBCCALL libc_Xw16link(char16_t const *from, char16_t const *to);
INTDEF void LIBCCALL libc_Xw32link(char32_t const *from, char32_t const *to);
INTDEF void LIBCCALL libc_Xw16linkat(int fromfd, char16_t const *from, int tofd, char16_t const *to, int flags);
INTDEF void LIBCCALL libc_Xw32linkat(int fromfd, char32_t const *from, int tofd, char32_t const *to, int flags);
INTDEF void LIBCCALL libc_Xw16unlink(char16_t const *name);
INTDEF void LIBCCALL libc_Xw32unlink(char32_t const *name);
INTDEF void LIBCCALL libc_Xw16unlinkat(fd_t dfd, char16_t const *name, int flags);
INTDEF void LIBCCALL libc_Xw32unlinkat(fd_t dfd, char32_t const *name, int flags);
INTDEF void LIBCCALL libc_Xw16rmdir(char16_t const *name);
INTDEF void LIBCCALL libc_Xw32rmdir(char32_t const *name);
INTDEF void LIBCCALL libc_Xw16remove(char16_t const *name);
INTDEF void LIBCCALL libc_Xw32remove(char32_t const *name);
INTDEF void LIBCCALL libc_Xw16removeat(fd_t dfd, char16_t const *name);
INTDEF void LIBCCALL libc_Xw32removeat(fd_t dfd, char32_t const *name);
INTDEF void LIBCCALL libc_Xw16rename(char16_t const *oldname, char16_t const *newname);
INTDEF void LIBCCALL libc_Xw32rename(char32_t const *oldname, char32_t const *newname);
INTDEF void LIBCCALL libc_Xw16renameat(fd_t oldfd, char16_t const *oldname, fd_t newfd, char16_t const *newname);
INTDEF void LIBCCALL libc_Xw32renameat(fd_t oldfd, char32_t const *oldname, fd_t newfd, char32_t const *newname);
INTDEF void LIBCCALL libc_Xw16frenameat(fd_t oldfd, char16_t const *oldname, fd_t newfd, char16_t const *newname, int flags);
INTDEF void LIBCCALL libc_Xw32frenameat(fd_t oldfd, char32_t const *oldname, fd_t newfd, char32_t const *newname, int flags);
INTDEF void LIBCCALL libc_Xw16mkdir(char16_t const *name, mode_t mode);
INTDEF void LIBCCALL libc_Xw32mkdir(char32_t const *name, mode_t mode);
INTDEF void LIBCCALL libc_Xw16mkdirat(fd_t dfd, char16_t const *name, mode_t mode);
INTDEF void LIBCCALL libc_Xw32mkdirat(fd_t dfd, char32_t const *name, mode_t mode);
INTDEF void LIBCCALL libc_Xw16fmkdirat(fd_t dfd, char16_t const *name, mode_t mode, int flags);
INTDEF void LIBCCALL libc_Xw32fmkdirat(fd_t dfd, char32_t const *name, mode_t mode, int flags);
INTDEF void LIBCCALL libc_Xw16symlink(char16_t const *from, char16_t const *to);
INTDEF void LIBCCALL libc_Xw32symlink(char32_t const *from, char32_t const *to);
INTDEF void LIBCCALL libc_Xw16symlinkat(char16_t const *from, int tofd, char16_t const *to);
INTDEF void LIBCCALL libc_Xw32symlinkat(char32_t const *from, int tofd, char32_t const *to);
INTDEF void LIBCCALL libc_Xw16fsymlinkat(char16_t const *from, int tofd, char16_t const *to, int flags);
INTDEF void LIBCCALL libc_Xw32fsymlinkat(char32_t const *from, int tofd, char32_t const *to, int flags);
INTDEF void LIBCCALL libc_Xw16mknod(char16_t const *path, mode_t mode, dev_t dev);
INTDEF void LIBCCALL libc_Xw32mknod(char32_t const *path, mode_t mode, dev_t dev);
INTDEF void LIBCCALL libc_Xw16mknodat(fd_t dfd, char16_t const *path, mode_t mode, dev_t dev);
INTDEF void LIBCCALL libc_Xw32mknodat(fd_t dfd, char32_t const *path, mode_t mode, dev_t dev);
INTDEF void LIBCCALL libc_Xw16fmknodat(fd_t dfd, char16_t const *path, mode_t mode, dev_t dev, int flags);
INTDEF void LIBCCALL libc_Xw32fmknodat(fd_t dfd, char32_t const *path, mode_t mode, dev_t dev, int flags);
INTDEF void LIBCCALL libc_Xw16mkfifo(char16_t const *path, mode_t mode);
INTDEF void LIBCCALL libc_Xw32mkfifo(char32_t const *path, mode_t mode);
INTDEF void LIBCCALL libc_Xw16mkfifoat(fd_t dfd, char16_t const *path, mode_t mode);
INTDEF void LIBCCALL libc_Xw32mkfifoat(fd_t dfd, char32_t const *path, mode_t mode);
INTDEF size_t LIBCCALL libc_Xw16freadlink(fd_t fd, char16_t *buf, size_t buflen);
INTDEF size_t LIBCCALL libc_Xw32freadlink(fd_t fd, char32_t *buf, size_t buflen);
INTDEF size_t LIBCCALL libc_Xw16readlink(char16_t const *path, char16_t *buf, size_t buflen);
INTDEF size_t LIBCCALL libc_Xw32readlink(char32_t const *path, char32_t *buf, size_t buflen);
INTDEF size_t LIBCCALL libc_Xw16readlinkat(fd_t dfd, char16_t const *path, char16_t *buf, size_t buflen);
INTDEF size_t LIBCCALL libc_Xw32readlinkat(fd_t dfd, char32_t const *path, char32_t *buf, size_t buflen);
INTDEF size_t LIBCCALL libc_Xw16freadlinkat(fd_t dfd, char16_t const *path, char16_t *buf, size_t buflen, int flags);
INTDEF size_t LIBCCALL libc_Xw32freadlinkat(fd_t dfd, char32_t const *path, char32_t *buf, size_t buflen, int flags);
INTDEF void LIBCCALL libc_Xw16access(char16_t const *name, int type);
INTDEF void LIBCCALL libc_Xw32access(char32_t const *name, int type);
INTDEF void LIBCCALL libc_Xw16eaccess(char16_t const *name, int type);
INTDEF void LIBCCALL libc_Xw32eaccess(char32_t const *name, int type);
INTDEF void LIBCCALL libc_Xw16faccessat(fd_t dfd, char16_t const *file, int type, int flags);
INTDEF void LIBCCALL libc_Xw32faccessat(fd_t dfd, char32_t const *file, int type, int flags);
INTDEF void LIBCCALL libc_Xw16truncate(char16_t const *file, pos32_t length);
INTDEF void LIBCCALL libc_Xw32truncate(char32_t const *file, pos32_t length);
INTDEF void LIBCCALL libc_Xw16truncate64(char16_t const *file, pos64_t length);
INTDEF void LIBCCALL libc_Xw32truncate64(char32_t const *file, pos64_t length);
INTDEF void LIBCCALL libc_Xw16ftruncateat(fd_t dfd, char16_t const *file, pos32_t length, int flags);
INTDEF void LIBCCALL libc_Xw32ftruncateat(fd_t dfd, char32_t const *file, pos32_t length, int flags);
INTDEF void LIBCCALL libc_Xw16ftruncateat64(fd_t dfd, char16_t const *file, pos64_t length, int flags);
INTDEF void LIBCCALL libc_Xw32ftruncateat64(fd_t dfd, char32_t const *file, pos64_t length, int flags);
INTDEF void LIBCCALL libc_Xw16chroot(char16_t const *path);
INTDEF void LIBCCALL libc_Xw32chroot(char32_t const *path);

INTDEF int LIBCCALL libc_w16frevokeat(fd_t dfd, char16_t const *file, int flags);
INTDEF int LIBCCALL libc_w32frevokeat(fd_t dfd, char32_t const *file, int flags);
INTDEF int LIBCCALL libc_w16revoke(char16_t const *file);
INTDEF int LIBCCALL libc_w32revoke(char32_t const *file);
INTDEF int LIBCCALL libc_dos_w16revoke(char16_t const *file);
INTDEF int LIBCCALL libc_dos_w32revoke(char32_t const *file);
INTDEF int LIBCCALL libc_w16facctat(fd_t dfd, char16_t const *name, int flags);
INTDEF int LIBCCALL libc_w32facctat(fd_t dfd, char32_t const *name, int flags);
INTDEF int LIBCCALL libc_w16acct(char16_t const *name);
INTDEF int LIBCCALL libc_w32acct(char32_t const *name);
INTDEF int LIBCCALL libc_dos_w16acct(char16_t const *name);
INTDEF int LIBCCALL libc_dos_w32acct(char32_t const *name);
INTDEF void LIBCCALL libc_Xw16revoke(char16_t const *file);
INTDEF void LIBCCALL libc_Xw32revoke(char32_t const *file);
INTDEF void LIBCCALL libc_Xw16acct(char16_t const *name);
INTDEF void LIBCCALL libc_Xw32acct(char32_t const *name);
INTDEF WUNUSED int ATTR_CDECL libc_w16sopen(char16_t const *file, int oflag, int sflag, ...);
INTDEF WUNUSED int ATTR_CDECL libc_w32sopen(char32_t const *file, int oflag, int sflag, ...);
INTDEF WUNUSED int ATTR_CDECL libc_dos_w16sopen(char16_t const *file, int oflag, int sflag, ...);
INTDEF WUNUSED int ATTR_CDECL libc_dos_w32sopen(char32_t const *file, int oflag, int sflag, ...);
INTDEF WUNUSED errno_t ATTR_CDECL libc_w16sopen_s(int *fd, char16_t const *file, int oflag, int sflag, ...);
INTDEF WUNUSED errno_t ATTR_CDECL libc_w32sopen_s(int *fd, char32_t const *file, int oflag, int sflag, ...);
INTDEF WUNUSED derrno_t ATTR_CDECL libc_dos_w16sopen_s(int *fd, char16_t const *file, int oflag, int sflag, ...);
INTDEF WUNUSED derrno_t ATTR_CDECL libc_dos_w32sopen_s(int *fd, char32_t const *file, int oflag, int sflag, ...);

INTDEF int LIBCCALL libc_w16system(char16_t const *command);
INTDEF int LIBCCALL libc_w32system(char32_t const *command);
INTDEF int LIBCCALL libc_dos_w16system(char16_t const *command);
INTDEF int LIBCCALL libc_dos_w32system(char32_t const *command);
INTDEF int LIBCCALL libc_Xw16system(char16_t const *command);
INTDEF int LIBCCALL libc_Xw32system(char32_t const *command);


#endif /* __CC__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_WIDECHAR_H */
