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
#ifndef GUARD_LIBS_LIBC_UNISTD_H
#define GUARD_LIBS_LIBC_UNISTD_H 1

#include "libc.h"
#include <kos/types.h>

#if defined(__CC__) && !defined(__KERNEL__)
DECL_BEGIN

/* ===================================================================================== */
/*     UNISTD                                                                            */
/* ===================================================================================== */
struct iovec;
struct file_handle;
#ifndef __nfds_t_defined
#define __nfds_t_defined 1
typedef __UINTPTR_TYPE__ nfds_t;
#endif /* !__nfds_t_defined */
INTDEF int ATTR_CDECL libc_open(char const *filename, oflag_t flags, ...);
INTDEF int ATTR_CDECL libc_openat(fd_t dfd, char const *filename, oflag_t flags, ...);
INTDEF int ATTR_CDECL libc_dos_open(char const *filename, oflag_t flags, ...);
INTDEF int ATTR_CDECL libc_dos_openat(fd_t dfd, char const *filename, oflag_t flags, ...);
INTDEF int LIBCCALL libc_creat(char const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_creat(char const *file, mode_t mode);
INTDEF ssize_t ATTR_CDECL libc_fcntl(fd_t fd, unsigned int cmd, ...);
INTDEF ssize_t ATTR_CDECL libc_ioctl(fd_t fd, unsigned long cmd, ...);
INTDEF ssize_t ATTR_CDECL libc_xioctlf(fd_t fd, unsigned long cmd, oflag_t flags, ...);
INTDEF int LIBCCALL libc_posix_fadvise(fd_t fd, pos32_t offset, pos32_t len, int advise);
INTDEF int LIBCCALL libc_posix_fadvise64(fd_t fd, pos64_t offset, pos64_t len, int advise);
INTDEF int LIBCCALL libc_posix_fallocate(fd_t fd, pos32_t offset, pos32_t len);
INTDEF int LIBCCALL libc_posix_fallocate64(fd_t fd, pos64_t offset, pos64_t len);
INTDEF int LIBCCALL libc_lockf(fd_t fd, int cmd, pos32_t len);
INTDEF int LIBCCALL libc_lockf64(fd_t fd, int cmd, pos64_t len);
INTDEF ssize_t LIBCCALL libc_readahead(fd_t fd, pos64_t offset, size_t count);
INTDEF int LIBCCALL libc_sync_file_range(fd_t fd, pos64_t offset, pos64_t count, unsigned int flags);
INTDEF ssize_t LIBCCALL libc_vmsplice(int fdout, struct iovec const *iov, size_t count, unsigned int flags);
INTDEF ssize_t LIBCCALL libc_splice(int fdin, pos64_t *offin, int fdout, pos64_t *offout, size_t len, unsigned int flags);
INTDEF ssize_t LIBCCALL libc_tee(int fdin, int fdout, size_t len, unsigned int flags);
INTDEF int LIBCCALL libc_name_to_handle_at(fd_t dfd, char const *name, struct file_handle *handle, int *mnt_id, int flags);
INTDEF int LIBCCALL libc_open_by_handle_at(int mountdirfd, struct file_handle *handle, int flags);
INTDEF int LIBCCALL libc_fallocate(fd_t fd, int mode, pos32_t offset, pos32_t len);
INTDEF int LIBCCALL libc_fallocate64(fd_t fd, int mode, pos64_t offset, pos64_t len);
INTDEF pid_t LIBCCALL libc_gettid(void);
INTDEF pid_t LIBCCALL libc_getpid(void);
INTDEF int LIBCCALL libc_pipe(int pipedes[2]);
INTDEF int LIBCCALL libc_pipe2(int pipedes[2], int flags);
INTDEF int LIBCCALL libd_pipe(int pipedes[2], u32 pipesize, int textmode);
INTDEF mode_t LIBCCALL libc_umask(mode_t new_mode);
INTDEF mode_t LIBCCALL libc_getumask(void);
INTDEF errno_t LIBCCALL libc_umask_s(mode_t new_mode, mode_t *old_mode);
INTDEF derrno_t LIBCCALL libd_umask_s(mode_t new_mode, mode_t *old_mode);
INTDEF mode_t LIBCCALL libd_setmode(fd_t fd, mode_t mode);
INTDEF int LIBCCALL _libd_lock_fhandle(fd_t fd);
INTDEF void LIBCCALL libd_unlock_fhandle(fd_t fd);
INTDEF int LIBCCALL libc_fsync(fd_t fd);
INTDEF int LIBCCALL libc_fdatasync(fd_t fd);
INTDEF uid_t LIBCCALL libc_getuid(void);
INTDEF uid_t LIBCCALL libc_geteuid(void);
INTDEF gid_t LIBCCALL libc_getgid(void);
INTDEF gid_t LIBCCALL libc_getegid(void);
INTDEF int LIBCCALL libc_getgroups(int size, gid_t list[]);
INTDEF int LIBCCALL libc_setuid(uid_t uid);
INTDEF int LIBCCALL libc_setgid(gid_t gid);
/* HANDLES */
INTDEF fd_t LIBCCALL libc_dup(fd_t fd);
INTDEF fd_t LIBCCALL libc_dup2(fd_t ofd, fd_t nfd);
INTDEF fd_t LIBCCALL libc_dup3(fd_t ofd, fd_t nfd, int flags);
INTDEF int LIBCCALL libc_close(fd_t fd);
/* FILESYSTEM CONTROL */
INTDEF struct fsmask LIBCCALL libc_fsmode(struct fsmask new_mode);
INTDEF struct fsmask LIBCCALL libc_getfsmode(void);
#define LIBC_DOSMODE_ENABLED()    (libc_getfsmode().fm_mode & AT_DOSPATH)
#define LIBC_DOSMODE_DISABLED() (!(libc_getfsmode().fm_mask & AT_DOSPATH))
INTDEF int LIBCCALL libc_chdir(char const *path);
INTDEF int LIBCCALL libc_dos_chdir(char const *path);
INTDEF int LIBCCALL libc_fchdirat(fd_t dfd, char const *path, int flags);
INTDEF int LIBCCALL libc_dos_fchdirat(fd_t dfd, char const *path, int flags);
INTDEF int LIBCCALL libc_fchdir(fd_t fd);
INTDEF long int LIBCCALL libc_pathconf(char const *path, int name);
INTDEF long int LIBCCALL libc_dos_pathconf(char const *path, int name);
INTDEF long int LIBCCALL libc_fpathconfat(fd_t dfd, char const *file, int name, int flags);
INTDEF long int LIBCCALL libc_dos_fpathconfat(fd_t dfd, char const *file, int name, int flags);
INTDEF long int LIBCCALL libc_fpathconf(fd_t fd, int name);
/* FILESYSTEM OPERATIONS */
INTDEF int LIBCCALL libc_chown(char const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_dos_chown(char const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_fchown(fd_t fd, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_lchown(char const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_dos_lchown(char const *file, uid_t owner, gid_t group);
INTDEF int LIBCCALL libc_fchownat(fd_t dfd, char const *file, uid_t owner, gid_t group, int flags);
INTDEF int LIBCCALL libc_dos_fchownat(fd_t dfd, char const *file, uid_t owner, gid_t group, int flags);
INTDEF int LIBCCALL libc_fchmod(fd_t fd, mode_t mode);
INTDEF int LIBCCALL libc_chmod(char const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_chmod(char const *file, mode_t mode);
INTDEF int LIBCCALL libc_lchmod(char const *file, mode_t mode);
INTDEF int LIBCCALL libc_dos_lchmod(char const *file, mode_t mode);
INTDEF int LIBCCALL libc_fchmodat(fd_t dfd, char const *file, mode_t mode, int flags);
INTDEF int LIBCCALL libc_dos_fchmodat(fd_t dfd, char const *file, mode_t mode, int flags);
INTDEF int LIBCCALL libc_link(char const *from, char const *to);
INTDEF int LIBCCALL libc_dos_link(char const *from, char const *to);
INTDEF int LIBCCALL libc_linkat(int fromfd, char const *from, int tofd, char const *to, int flags);
INTDEF int LIBCCALL libc_dos_linkat(int fromfd, char const *from, int tofd, char const *to, int flags);
INTDEF int LIBCCALL libc_unlink(char const *name);
INTDEF int LIBCCALL libc_dos_unlink(char const *name);
INTDEF int LIBCCALL libc_unlinkat(fd_t dfd, char const *name, int flags);
INTDEF int LIBCCALL libc_dos_unlinkat(fd_t dfd, char const *name, int flags);
INTDEF int LIBCCALL libc_rmdir(char const *name);
INTDEF int LIBCCALL libc_dos_rmdir(char const *name);
INTDEF int LIBCCALL libc_remove(char const *name);
INTDEF int LIBCCALL libc_dos_remove(char const *name);
INTDEF int LIBCCALL libc_removeat(fd_t dfd, char const *name);
INTDEF int LIBCCALL libc_dos_removeat(fd_t dfd, char const *name);
INTDEF int LIBCCALL libc_rename(char const *oldname, char const *newname);
INTDEF int LIBCCALL libc_dos_rename(char const *oldname, char const *newname);
INTDEF int LIBCCALL libc_renameat(fd_t oldfd, char const *oldname, fd_t newfd, char const *newname);
INTDEF int LIBCCALL libc_dos_renameat(fd_t oldfd, char const *oldname, fd_t newfd, char const *newname);
INTDEF int LIBCCALL libc_frenameat(fd_t oldfd, char const *oldname, fd_t newfd, char const *newname, int flags);
INTDEF int LIBCCALL libc_dos_frenameat(fd_t oldfd, char const *oldname, fd_t newfd, char const *newname, int flags);
INTDEF int LIBCCALL libc_mkdir1(char const *name);
INTDEF int LIBCCALL libc_dos_mkdir1(char const *name);
INTDEF int LIBCCALL libc_mkdir(char const *name, mode_t mode);
INTDEF int LIBCCALL libc_dos_mkdir(char const *name, mode_t mode);
INTDEF int LIBCCALL libc_mkdirat(fd_t dfd, char const *name, mode_t mode);
INTDEF int LIBCCALL libc_dos_mkdirat(fd_t dfd, char const *name, mode_t mode);
INTDEF int LIBCCALL libc_fmkdirat(fd_t dfd, char const *name, mode_t mode, int flags);
INTDEF int LIBCCALL libc_dos_fmkdirat(fd_t dfd, char const *name, mode_t mode, int flags);
INTDEF int LIBCCALL libc_symlink(char const *from, char const *to);
INTDEF int LIBCCALL libc_dos_symlink(char const *from, char const *to);
INTDEF int LIBCCALL libc_symlinkat(char const *from, int tofd, char const *to);
INTDEF int LIBCCALL libc_dos_symlinkat(char const *from, int tofd, char const *to);
INTDEF int LIBCCALL libc_fsymlinkat(char const *from, int tofd, char const *to, int flags);
INTDEF int LIBCCALL libc_dos_fsymlinkat(char const *from, int tofd, char const *to, int flags);
INTDEF int LIBCCALL libc_mknod(char const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_dos_mknod(char const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_mknodat(fd_t dfd, char const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_dos_mknodat(fd_t dfd, char const *path, mode_t mode, dev_t dev);
INTDEF int LIBCCALL libc_fmknodat(fd_t dfd, char const *path, mode_t mode, dev_t dev, int flags);
INTDEF int LIBCCALL libc_dos_fmknodat(fd_t dfd, char const *path, mode_t mode, dev_t dev, int flags);
INTDEF int LIBCCALL libc_xmknod(int ver, char const *path, mode_t mode, dev_t *pdev);
INTDEF int LIBCCALL libc_dos_xmknod(int ver, char const *path, mode_t mode, dev_t *pdev);
INTDEF int LIBCCALL libc_xmknodat(int ver, fd_t dfd, char const *path, mode_t mode, dev_t *pdev);
INTDEF int LIBCCALL libc_dos_xmknodat(int ver, fd_t dfd, char const *path, mode_t mode, dev_t *pdev);
INTDEF int LIBCCALL libc_mkfifo(char const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_mkfifo(char const *path, mode_t mode);
INTDEF int LIBCCALL libc_mkfifoat(fd_t dfd, char const *path, mode_t mode);
INTDEF int LIBCCALL libc_dos_mkfifoat(fd_t dfd, char const *path, mode_t mode);
INTDEF errno_t LIBCCALL libd_access_s(char const *file, int type);
INTDEF derrno_t LIBCCALL libd_dos_access_s(char const *file, int type);
INTDEF ssize_t LIBCCALL libc_freadlink(fd_t fd, char *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_readlink(char const *path, char *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_dos_readlink(char const *path, char *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_readlinkat(fd_t dfd, char const *path, char *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_dos_readlinkat(fd_t dfd, char const *path, char *buf, size_t buflen);
INTDEF ssize_t LIBCCALL libc_freadlinkat(fd_t dfd, char const *path, char *buf, size_t buflen, int flags);
INTDEF ssize_t LIBCCALL libc_dos_freadlinkat(fd_t dfd, char const *path, char *buf, size_t buflen, int flags);
INTDEF int LIBCCALL libc_access(char const *name, int type);
INTDEF int LIBCCALL libc_dos_access(char const *name, int type);
INTDEF int LIBCCALL libc_eaccess(char const *name, int type);
INTDEF int LIBCCALL libc_dos_eaccess(char const *name, int type);
INTDEF int LIBCCALL libc_faccessat(fd_t dfd, char const *file, int type, int flags);
INTDEF int LIBCCALL libc_dos_faccessat(fd_t dfd, char const *file, int type, int flags);
INTDEF int LIBCCALL libc_ftruncate(fd_t fd, pos32_t length);
INTDEF int LIBCCALL libc_ftruncate64(fd_t fd, pos64_t length);
INTDEF int LIBCCALL libc_truncate(char const *file, pos32_t length);
INTDEF int LIBCCALL libc_truncate64(char const *file, pos64_t length);
INTDEF int LIBCCALL libc_dos_truncate(char const *file, pos32_t length);
INTDEF int LIBCCALL libc_dos_truncate64(char const *file, pos64_t length);
INTDEF int LIBCCALL libc_ftruncateat(fd_t dfd, char const *file, pos32_t length, int flags);
INTDEF int LIBCCALL libc_ftruncateat64(fd_t dfd, char const *file, pos64_t length, int flags);
INTDEF int LIBCCALL libc_dos_ftruncateat(fd_t dfd, char const *file, pos32_t length, int flags);
INTDEF int LIBCCALL libc_dos_ftruncateat64(fd_t dfd, char const *file, pos64_t length, int flags);
INTDEF int LIBCCALL libc_chroot(char const *path);
INTDEF int LIBCCALL libc_dos_chroot(char const *path);
/* READ/WRITE */
INTDEF ssize_t LIBCCALL libc_read(fd_t fd, void *buf, size_t bufsize);
INTDEF ssize_t LIBCCALL libc_write(fd_t fd, void const *buf, size_t bufsize);
INTDEF int LIBCCALL libd_read(fd_t fd, void *buf, unsigned int bufsize);
INTDEF int LIBCCALL libd_write(fd_t fd, void const *buf, unsigned int bufsize);
INTDEF off32_t LIBCCALL libc_lseek(fd_t fd, off32_t offset, int whence);
INTDEF off64_t LIBCCALL libc_lseek64(fd_t fd, off64_t offset, int whence);
INTDEF ssize_t LIBCCALL libc_pread(fd_t fd, void *buf, size_t bufsize, pos32_t offset);
INTDEF ssize_t LIBCCALL libc_pread64(fd_t fd, void *buf, size_t bufsize, pos64_t offset);
INTDEF ssize_t LIBCCALL libc_pwrite(fd_t fd, void const *buf, size_t bufsize, pos32_t offset);
INTDEF ssize_t LIBCCALL libc_pwrite64(fd_t fd, void const *buf, size_t bufsize, pos64_t offset);
INTDEF ssize_t LIBCCALL libc_xreadf(fd_t fd, void *buf, size_t bufsize, oflag_t flags);
INTDEF ssize_t LIBCCALL libc_xwritef(fd_t fd, void const *buf, size_t bufsize, oflag_t flags);
INTDEF ssize_t LIBCCALL libc_xpreadf(fd_t fd, void *buf, size_t bufsize, pos32_t offset, oflag_t flags);
INTDEF ssize_t LIBCCALL libc_xpwritef(fd_t fd, void const *buf, size_t bufsize, pos32_t offset, oflag_t flags);
INTDEF ssize_t LIBCCALL libc_xpreadf64(fd_t fd, void *buf, size_t bufsize, pos64_t offset, oflag_t flags);
INTDEF ssize_t LIBCCALL libc_xpwritef64(fd_t fd, void const *buf, size_t bufsize, pos64_t offset, oflag_t flags);
struct pollfd;
INTDEF ssize_t LIBCCALL libc_poll(struct pollfd *fds, nfds_t nfds, int timeout);
INTDEF ssize_t LIBCCALL libc_ppoll(struct pollfd *fds, nfds_t nfds, struct timespec32 const *timeout, sigset_t const *ss);
INTDEF ssize_t LIBCCALL libc_ppoll64(struct pollfd *fds, nfds_t nfds, struct timespec64 const *timeout, sigset_t const *ss);
INTDEF ssize_t LIBCCALL libc_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval32 *timeout);
INTDEF ssize_t LIBCCALL libc_select64(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval64 *timeout);
INTDEF ssize_t LIBCCALL libc_pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timespec32 const *timeout, sigset_t const *sigmask);
INTDEF ssize_t LIBCCALL libc_pselect64(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timespec64 const *timeout, sigset_t const *sigmask);
INTDEF int LIBCCALL libc_pause(void);
INTDEF int LIBCCALL libc_syncfs(fd_t fd);
INTDEF int LIBCCALL libc_group_member(gid_t gid);
INTDEF int LIBCCALL libc_getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
INTDEF int LIBCCALL libc_getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid);
INTDEF int LIBCCALL libc_setresuid(uid_t ruid, uid_t euid, uid_t suid);
INTDEF int LIBCCALL libc_setresgid(gid_t rgid, gid_t egid, gid_t sgid);
INTDEF size_t LIBCCALL libc_confstr(int name, char *buf, size_t buflen);
INTDEF void LIBCCALL libc_sync(void);
INTDEF int LIBCCALL libc_setreuid(uid_t ruid, uid_t euid);
INTDEF int LIBCCALL libc_setregid(gid_t rgid, gid_t egid);
INTDEF long int LIBCCALL libc_gethostid(void);
INTDEF int LIBCCALL libc_getpagesize(void) ;
INTDEF int LIBCCALL libc_getdtablesize(void);
INTDEF int LIBCCALL libc_seteuid(uid_t uid);
INTDEF int LIBCCALL libc_setegid(gid_t gid);
INTDEF int LIBCCALL libc_sethostid(long int id);
INTDEF int LIBCCALL libc_frevokeat(fd_t dfd, char const *file, int flags);
INTDEF int LIBCCALL libc_facctat(fd_t dfd, char const *name, int flags);
INTDEF int LIBCCALL libc_revoke(char const *file);
INTDEF int LIBCCALL libc_dos_revoke(char const *file);
INTDEF int LIBCCALL libc_profil(unsigned short int *sample_buffer, size_t size, size_t offset, unsigned int scale);
INTDEF int LIBCCALL libc_acct(char const *name);
INTDEF int LIBCCALL libc_dos_acct(char const *name);
INTDEF char *LIBCCALL libc_getusershell(void);
INTDEF void LIBCCALL libc_endusershell(void);
INTDEF void LIBCCALL libc_setusershell(void);
INTDEF int LIBCCALL libc_daemon(int nochdir, int noclose);
INTDEF long int ATTR_CDECL libc_syscall(long int sysno, ...);
INTDEF long int ATTR_CDECL libc_ssyscall(long int sysno, ...);
INTDEF long long int ATTR_CDECL libc_lsyscall(long int sysno, ...) ASMNAME("libc_syscall");
INTDEF long long int ATTR_CDECL libc_lssyscall(long int sysno, ...) ASMNAME("libc_ssyscall");
INTDEF char *LIBCCALL libc_crypt(char const *key, char const *salt);
INTDEF void LIBCCALL libc_encrypt(char *glibc_block, int edflag);
INTDEF char *LIBCCALL libc_ctermid(char *s);
INTDEF long int LIBCCALL libc_sysconf(int name);
struct sysinfo;
INTDEF int LIBCCALL libc_sysinfo(struct sysinfo *info);
INTDEF int LIBCCALL libc_get_nprocs_conf(void);
INTDEF int LIBCCALL libc_get_nprocs(void);
INTDEF intptr_t LIBCCALL libc_get_phys_pages(void);
INTDEF intptr_t LIBCCALL libc_get_avphys_pages(void);
/* UNISTD W/ exception support */
INTDEF fd_t ATTR_CDECL libc_Xopen(char const *filename, oflag_t flags, ...);
INTDEF fd_t ATTR_CDECL libc_Xopenat(fd_t dfd, char const *filename, oflag_t flags, ...);
INTDEF fd_t LIBCCALL libc_Xcreat(char const *file, mode_t mode);
INTDEF ssize_t ATTR_CDECL libc_Xfcntl(fd_t fd, unsigned int cmd, ...);
INTDEF ssize_t ATTR_CDECL libc_Xioctl(fd_t fd, unsigned long cmd, ...);
INTDEF void LIBCCALL libc_Xpipe(fd_t pipedes[2]);
INTDEF void LIBCCALL libc_Xpipe2(fd_t pipedes[2], oflag_t flags);
INTDEF void LIBCCALL libc_Xfsync(fd_t fd);
INTDEF void LIBCCALL libc_Xfdatasync(fd_t fd);
INTDEF fd_t LIBCCALL libc_Xdup(fd_t fd);
INTDEF fd_t LIBCCALL libc_Xdup2(fd_t ofd, fd_t nfd);
INTDEF fd_t LIBCCALL libc_Xdup3(fd_t ofd, fd_t nfd, int flags);
INTDEF void LIBCCALL libc_Xchdir(char const *path);
INTDEF void LIBCCALL libc_Xfchdirat(fd_t dfd, char const *path, int flags);
INTDEF void LIBCCALL libc_Xfchdir(fd_t fd);
INTDEF ATTR_RETNONNULL char *LIBCCALL libc_Xgetcwd(char *buf, size_t size);
INTDEF ATTR_RETNONNULL char *LIBCCALL libc_Xget_current_dir_name(void);
INTDEF long int LIBCCALL libc_Xpathconf(char const *path, int name);
INTDEF long int LIBCCALL libc_Xfpathconfat(fd_t dfd, char const *file, int name, int flags);
INTDEF long int LIBCCALL libc_Xfpathconf(fd_t fd, int name);
INTDEF void LIBCCALL libc_Xchown(char const *file, uid_t owner, gid_t group);
INTDEF void LIBCCALL libc_Xfchown(fd_t fd, uid_t owner, gid_t group);
INTDEF void LIBCCALL libc_Xlchown(char const *file, uid_t owner, gid_t group);
INTDEF void LIBCCALL libc_Xfchownat(fd_t dfd, char const *file, uid_t owner, gid_t group, int flags);
INTDEF void LIBCCALL libc_Xfchmod(fd_t fd, mode_t mode);
INTDEF void LIBCCALL libc_Xchmod(char const *file, mode_t mode);
INTDEF void LIBCCALL libc_Xlchmod(char const *file, mode_t mode);
INTDEF void LIBCCALL libc_Xfchmodat(fd_t dfd, char const *file, mode_t mode, int flags);
INTDEF void LIBCCALL libc_Xlink(char const *from, char const *to);
INTDEF void LIBCCALL libc_Xlinkat(int fromfd, char const *from, int tofd, char const *to, int flags);
INTDEF void LIBCCALL libc_Xunlink(char const *name);
INTDEF void LIBCCALL libc_Xunlinkat(fd_t dfd, char const *name, int flags);
INTDEF void LIBCCALL libc_Xrmdir(char const *name);
INTDEF void LIBCCALL libc_Xremove(char const *name);
INTDEF void LIBCCALL libc_Xremoveat(fd_t dfd, char const *name);
INTDEF void LIBCCALL libc_Xrename(char const *oldname, char const *newname);
INTDEF void LIBCCALL libc_Xrenameat(fd_t oldfd, char const *oldname, fd_t newfd, char const *newname);
INTDEF void LIBCCALL libc_Xfrenameat(fd_t oldfd, char const *oldname, fd_t newfd, char const *newname, int flags);
INTDEF void LIBCCALL libc_Xmkdir(char const *name, mode_t mode);
INTDEF void LIBCCALL libc_Xmkdirat(fd_t dfd, char const *name, mode_t mode);
INTDEF void LIBCCALL libc_Xfmkdirat(fd_t dfd, char const *name, mode_t mode, int flags);
INTDEF void LIBCCALL libc_Xsymlink(char const *from, char const *to);
INTDEF void LIBCCALL libc_Xsymlinkat(char const *from, int tofd, char const *to);
INTDEF void LIBCCALL libc_Xfsymlinkat(char const *from, int tofd, char const *to, int flags);
INTDEF void LIBCCALL libc_Xmknod(char const *path, mode_t mode, dev_t dev);
INTDEF void LIBCCALL libc_Xmknodat(fd_t dfd, char const *path, mode_t mode, dev_t dev);
INTDEF void LIBCCALL libc_Xfmknodat(fd_t dfd, char const *path, mode_t mode, dev_t dev, int flags);
INTDEF void LIBCCALL libc_Xmkfifo(char const *path, mode_t mode);
INTDEF void LIBCCALL libc_Xmkfifoat(fd_t dfd, char const *path, mode_t mode);
INTDEF size_t LIBCCALL libc_Xfreadlink(fd_t fd, char *buf, size_t buflen);
INTDEF size_t LIBCCALL libc_Xreadlink(char const *path, char *buf, size_t buflen);
INTDEF size_t LIBCCALL libc_Xreadlinkat(fd_t dfd, char const *path, char *buf, size_t buflen);
INTDEF size_t LIBCCALL libc_Xfreadlinkat(fd_t dfd, char const *path, char *buf, size_t buflen, int flags);
INTDEF void LIBCCALL libc_Xaccess(char const *name, int type);
INTDEF void LIBCCALL libc_Xeaccess(char const *name, int type);
INTDEF void LIBCCALL libc_Xfaccessat(fd_t dfd, char const *file, int type, int flags);
INTDEF void LIBCCALL libc_Xftruncate(fd_t fd, pos32_t length);
INTDEF void LIBCCALL libc_Xftruncate64(fd_t fd, pos64_t length);
INTDEF void LIBCCALL libc_Xtruncate(char const *file, pos32_t length);
INTDEF void LIBCCALL libc_Xtruncate64(char const *file, pos64_t length);
INTDEF void LIBCCALL libc_Xftruncateat(fd_t dfd, char const *file, pos32_t length, int flags);
INTDEF void LIBCCALL libc_Xftruncateat64(fd_t dfd, char const *file, pos64_t length, int flags);
INTDEF void LIBCCALL libc_Xchroot(char const *path);
INTDEF size_t LIBCCALL libc_Xread(fd_t fd, void *buf, size_t bufsize);
INTDEF size_t LIBCCALL libc_Xwrite(fd_t fd, void const *buf, size_t bufsize);
INTDEF pos32_t LIBCCALL libc_Xlseek(fd_t fd, off32_t offset, int whence);
INTDEF pos64_t LIBCCALL libc_Xlseek64(fd_t fd, off64_t offset, int whence);
INTDEF size_t LIBCCALL libc_Xpread(fd_t fd, void *buf, size_t bufsize, pos32_t offset);
INTDEF size_t LIBCCALL libc_Xpread64(fd_t fd, void *buf, size_t bufsize, pos64_t offset);
INTDEF size_t LIBCCALL libc_Xpwrite(fd_t fd, void const *buf, size_t bufsize, pos32_t offset);
INTDEF size_t LIBCCALL libc_Xpwrite64(fd_t fd, void const *buf, size_t bufsize, pos64_t offset);
INTDEF size_t LIBCCALL libc_Xxreadf(fd_t fd, void *buf, size_t bufsize, oflag_t flags);
INTDEF size_t LIBCCALL libc_Xxwritef(fd_t fd, void const *buf, size_t bufsize, oflag_t flags);
INTDEF size_t LIBCCALL libc_Xxpreadf(fd_t fd, void *buf, size_t bufsize, pos32_t offset, oflag_t flags);
INTDEF size_t LIBCCALL libc_Xxpwritef(fd_t fd, void const *buf, size_t bufsize, pos32_t offset, oflag_t flags);
INTDEF size_t LIBCCALL libc_Xxpreadf64(fd_t fd, void *buf, size_t bufsize, pos64_t offset, oflag_t flags);
INTDEF size_t LIBCCALL libc_Xxpwritef64(fd_t fd, void const *buf, size_t bufsize, pos64_t offset, oflag_t flags);
INTDEF size_t LIBCCALL libc_Xpoll(struct pollfd *fds, nfds_t nfds, int timeout);
INTDEF size_t LIBCCALL libc_Xppoll(struct pollfd *fds, nfds_t nfds, struct timespec32 const *timeout, sigset_t const *ss);
INTDEF size_t LIBCCALL libc_Xppoll64(struct pollfd *fds, nfds_t nfds, struct timespec64 const *timeout, sigset_t const *ss);
INTDEF size_t LIBCCALL libc_Xselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval32 *timeout);
INTDEF size_t LIBCCALL libc_Xselect64(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval64 *timeout);
INTDEF size_t LIBCCALL libc_Xpselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timespec32 const *timeout, sigset_t const *sigmask);
INTDEF size_t LIBCCALL libc_Xpselect64(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timespec64 const *timeout, sigset_t const *sigmask);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xpause(void);
INTDEF void LIBCCALL libc_Xsyncfs(fd_t fd);
INTDEF void LIBCCALL libc_Xgroup_member(gid_t gid);
INTDEF void LIBCCALL libc_Xgetresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
INTDEF void LIBCCALL libc_Xgetresgid(gid_t *rgid, gid_t *egid, gid_t *sgid);
INTDEF void LIBCCALL libc_Xsetresuid(uid_t ruid, uid_t euid, uid_t suid);
INTDEF void LIBCCALL libc_Xsetresgid(gid_t rgid, gid_t egid, gid_t sgid);
INTDEF size_t LIBCCALL libc_Xconfstr(int name, char *buf, size_t buflen);
INTDEF void LIBCCALL libc_Xsetreuid(uid_t ruid, uid_t euid);
INTDEF void LIBCCALL libc_Xsetregid(gid_t rgid, gid_t egid);
INTDEF long int LIBCCALL libc_Xgethostid(void);
INTDEF void LIBCCALL libc_Xsethostid(long int id);
INTDEF void LIBCCALL libc_Xseteuid(uid_t uid);
INTDEF void LIBCCALL libc_Xsetegid(gid_t gid);
INTDEF void LIBCCALL libc_Xgetdomainname(char *name, size_t buflen);
INTDEF void LIBCCALL libc_Xsetdomainname(char const *name, size_t len);
INTDEF void LIBCCALL libc_Xrevoke(char const *file);
INTDEF void LIBCCALL libc_Xprofil(unsigned short int *sample_buffer, size_t size, size_t offset, unsigned int scale);
INTDEF void LIBCCALL libc_Xacct(char const *name);
INTDEF void LIBCCALL libc_Xdaemon(int nochdir, int noclose);
INTDEF long int LIBCCALL libc_Xsysconf(int name);
INTDEF long int ATTR_CDECL libc_Xsyscall(long int sysno, ...);
INTDEF long long int ATTR_CDECL libc_Xlsyscall(long int sysno, ...) ASMNAME("libc_Xsyscall");
INTDEF void LIBCCALL libc_Xsysinfo(struct sysinfo *info);
INTDEF unsigned int LIBCCALL libc_Xget_nprocs_conf(void);
INTDEF unsigned int LIBCCALL libc_Xget_nprocs(void);
INTDEF uintptr_t LIBCCALL libc_Xget_phys_pages(void);
INTDEF uintptr_t LIBCCALL libc_Xget_avphys_pages(void);
/* Misc. DOS functions. */
INTDEF WUNUSED int LIBCCALL libc_eof(fd_t fd);
INTDEF WUNUSED s32 LIBCCALL libc_filelength(fd_t fd);
INTDEF WUNUSED s64 LIBCCALL libc_filelengthi64(fd_t fd);
INTDEF WUNUSED s32 LIBCCALL libc_tell(fd_t fd);
INTDEF WUNUSED s64 LIBCCALL libc_telli64(fd_t fd);
INTDEF WUNUSED intptr_t LIBCCALL libc_get_osfhandle(fd_t fd);
INTDEF WUNUSED int LIBCCALL libc_open_osfhandle(intptr_t osfd, int flags);
INTDEF WUNUSED int ATTR_CDECL libc_sopen(char const *file, int oflag, int sflag, ...);
INTDEF WUNUSED int ATTR_CDECL libc_dos_sopen(char const *file, int oflag, int sflag, ...);
INTDEF WUNUSED errno_t ATTR_CDECL libc_sopen_s(int *fd, char const *file, int oflag, int sflag, ...);
INTDEF WUNUSED derrno_t ATTR_CDECL libc_dos_sopen_s(int *fd, char const *file, int oflag, int sflag, ...);


DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_UNISTD_H */
