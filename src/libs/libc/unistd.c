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
#ifndef GUARD_LIBS_LIBC_UNISTD_C
#define GUARD_LIBS_LIBC_UNISTD_C 1
#define _GNU_SOURCE 1

#include "libc.h"
#include "widechar.h"
#include "unistd.h"
#include "errno.h"
#include "err.h"
#include "system.h"
#include "malloc.h"
#include "environ.h"
#include "sched.h"
#include "realpath.h"
#include "rtl.h"

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/limits.h>
#include <kos/fcntl.h>
#include <bits/stat.h>
#include <syscall.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <except.h>
#include <alloca.h>
#include <stdbool.h>
#include <bits/dos-errno.h>
#include <sys/time.h>
#include <time.h>
#include <utime.h>
#include <unistd.h>
#include <syslog.h>
#include <linux/limits.h>


/* System call interface. */

DECL_BEGIN

INTERN char const libc_empty_string[] = {0,0,0,0};

INTERN int ATTR_CDECL
libc_open(char const *filename, oflag_t flags, ...) {
 va_list args; int result;
 va_start(args,flags);
 result = libc_openat(AT_FDCWD,filename,flags,va_arg(args,mode_t));
 va_end(args);
 return result;
}

CRT_DOS int ATTR_CDECL
libc_dos_open(char const *filename, oflag_t flags, ...) {
 va_list args; int result;
 va_start(args,flags);
 result = libc_openat(AT_FDCWD,filename,O_DOSPATH|flags,va_arg(args,mode_t));
 va_end(args);
 return result;
}

CRT_DOS_EXT int ATTR_CDECL
libc_dos_openat(fd_t dfd, char const *filename, oflag_t flags, ...) {
 va_list args; int result;
 va_start(args,flags);
 result = libc_openat(dfd,filename,O_DOSPATH|flags,va_arg(args,mode_t));
 va_end(args);
 return result;
}

INTERN int LIBCCALL libc_creat(char const *file, mode_t mode) {
 return libc_openat(AT_FDCWD,file,O_CREAT|O_WRONLY|O_TRUNC,mode);
}
CRT_DOS int LIBCCALL libc_dos_creat(char const *file, mode_t mode) {
 return libc_openat(AT_FDCWD,file,O_CREAT|O_WRONLY|O_TRUNC|O_DOSPATH,mode);
}
INTERN int LIBCCALL libc_posix_fadvise64(fd_t fd, pos64_t offset, pos64_t len, int advise) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_posix_fallocate64(fd_t fd, pos64_t offset, pos64_t len) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_lockf64(fd_t fd, int cmd, pos64_t len) { libc_seterrno(ENOSYS); return -1; }
INTERN ssize_t LIBCCALL libc_readahead(fd_t fd, pos64_t offset, size_t count) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_sync_file_range(fd_t fd, pos64_t offset, pos64_t count, unsigned int flags) { libc_seterrno(ENOSYS); return -1; }
INTERN ssize_t LIBCCALL libc_vmsplice(int fdout, struct iovec const *iov, size_t count, unsigned int flags) { libc_seterrno(ENOSYS); return -1; }
INTERN ssize_t LIBCCALL libc_splice(int fdin, pos64_t *offin, int fdout, pos64_t *offout, size_t len, unsigned int flags) { libc_seterrno(ENOSYS); return -1; }
INTERN ssize_t LIBCCALL libc_tee(int fdin, int fdout, size_t len, unsigned int flags) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_posix_fadvise(fd_t fd, pos32_t offset, pos32_t len, int advise) { return libc_posix_fadvise64(fd,(pos64_t)offset,(pos64_t)len,advise); }
INTERN int LIBCCALL libc_posix_fallocate(fd_t fd, pos32_t offset, pos32_t len) { return libc_posix_fallocate64(fd,(pos64_t)offset,(pos64_t)len); }
INTERN int LIBCCALL libc_lockf(fd_t fd, int cmd, pos32_t len) { return libc_lockf64(fd,cmd,(pos64_t)len); }
INTERN int LIBCCALL libc_name_to_handle_at(fd_t dfd, char const *name, struct file_handle *handle, int *mnt_id, int flags) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_open_by_handle_at(int mountdirfd, struct file_handle *handle, int flags) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_fallocate(fd_t fd, int mode, pos32_t offset, pos32_t len) { return libc_fallocate64(fd,mode,(pos64_t)offset,(pos64_t)len); }

CRT_DOS int LIBCCALL libd_pipe(int pipedes[2], u32 UNUSED(pipesize), int UNUSED(textmode)) { return libc_pipe2(pipedes,0); }
INTERN mode_t LIBCCALL libc_getumask(void) { mode_t result = sys_umask(0); sys_umask(result); return result; }
CRT_DOS errno_t LIBCCALL libc_umask_s(mode_t new_mode, mode_t *old_mode) { if (!old_mode) return EINVAL; *old_mode = libc_umask(new_mode); return 0; }
CRT_DOS derrno_t LIBCCALL libd_umask_s(mode_t new_mode, mode_t *old_mode) { if (!old_mode) return __DOS_EINVAL; *old_mode = libc_umask(new_mode); return 0; }
CRT_DOS int LIBCCALL _libd_lock_fhandle(int UNUSED(fd)) { return 0; }
CRT_DOS void LIBCCALL libd_unlock_fhandle(int UNUSED(fd)) { }
CRT_DOS mode_t LIBCCALL libd_setmode(fd_t fd, mode_t mode){ return libc_fcntl(fd,F_SETFL_XCH,mode); }
INTERN uid_t LIBCCALL libc_getuid(void) { libc_seterrno(ENOSYS); return -1; }
INTERN uid_t LIBCCALL libc_geteuid(void) { libc_seterrno(ENOSYS); return -1; }
INTERN gid_t LIBCCALL libc_getgid(void) { libc_seterrno(ENOSYS); return -1; }
INTERN gid_t LIBCCALL libc_getegid(void) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getgroups(int size, gid_t list[]) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_setuid(uid_t uid) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_setgid(gid_t gid) { libc_seterrno(ENOSYS); return -1; }
CRT_DOS int LIBCCALL libc_dos_chdir(char const *path) { return libc_fchdirat(AT_FDCWD,path,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fchdirat(fd_t dfd, char const *path, int flags) { return libc_fchdirat(dfd,path,flags|AT_DOSPATH); }
CRT_DOS_EXT long int LIBCCALL libc_dos_fpathconfat(fd_t dfd, char const *file, int name, int flags) { return libc_fpathconfat(dfd,file,name,flags|AT_DOSPATH); }
INTERN long int LIBCCALL libc_pathconf(char const *path, int name) { return libc_fpathconfat(AT_FDCWD,path,name,0); }
CRT_DOS_EXT long int LIBCCALL libc_dos_pathconf(char const *path, int name) { return libc_fpathconfat(AT_FDCWD,path,name,AT_DOSPATH); }
INTERN long int LIBCCALL libc_fpathconf(fd_t fd, int name) { return libc_fpathconfat(fd,libc_empty_string,name,AT_EMPTY_PATH); }
INTERN int LIBCCALL libc_chown(char const *file, uid_t owner, gid_t group) { return libc_fchownat(AT_FDCWD,file,owner,group,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_chown(char const *file, uid_t owner, gid_t group) { return libc_fchownat(AT_FDCWD,file,owner,group,AT_DOSPATH); }
INTERN int LIBCCALL libc_lchown(char const *file, uid_t owner, gid_t group) { return libc_fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_NOFOLLOW); }
CRT_DOS_EXT int LIBCCALL libc_dos_lchown(char const *file, uid_t owner, gid_t group) { return libc_fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fchownat(fd_t dfd, char const *file, uid_t owner, gid_t group, int flags) { return libc_fchownat(dfd,file,owner,group,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_chmod(char const *file, mode_t mode) { return libc_fchmodat(AT_FDCWD,file,mode,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_chmod(char const *file, mode_t mode) { return libc_fchmodat(AT_FDCWD,file,mode,AT_DOSPATH); }
INTERN int LIBCCALL libc_lchmod(char const *file, mode_t mode) { return libc_fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_NOFOLLOW); }
CRT_DOS_EXT int LIBCCALL libc_dos_lchmod(char const *file, mode_t mode) { return libc_fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fchmodat(fd_t dfd, char const *file, mode_t mode, int flags) { return libc_fchmodat(dfd,file,mode,flags); }
INTERN int LIBCCALL libc_link(char const *from, char const *to) { return libc_linkat(AT_FDCWD,from,AT_FDCWD,to,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_link(char const *from, char const *to) { return libc_linkat(AT_FDCWD,from,AT_FDCWD,to,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_linkat(int fromfd, char const *from, int tofd, char const *to, int flags) { return libc_linkat(fromfd,from,tofd,to,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_unlink(char const *name) { return libc_unlinkat(AT_FDCWD,name,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_unlink(char const *name) { return libc_unlinkat(AT_FDCWD,name,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_unlinkat(fd_t dfd, char const *name, int flags) { return libc_unlinkat(dfd,name,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_rmdir(char const *name) { return libc_unlinkat(AT_FDCWD,name,AT_REMOVEDIR); }
CRT_DOS_EXT int LIBCCALL libc_dos_rmdir(char const *name) { return libc_unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_DOSPATH); }
INTERN int LIBCCALL libc_remove(char const *name) { return libc_unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_REMOVEREG); }
CRT_DOS_EXT int LIBCCALL libc_dos_remove(char const *name) { return libc_unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_REMOVEREG|AT_DOSPATH); }
INTERN int LIBCCALL libc_removeat(fd_t dfd, char const *name) { return libc_unlinkat(dfd,name,AT_REMOVEDIR|AT_REMOVEREG); }
CRT_DOS_EXT int LIBCCALL libc_dos_removeat(fd_t dfd, char const *name) { return libc_unlinkat(dfd,name,AT_REMOVEDIR|AT_REMOVEREG|AT_DOSPATH); }
INTERN int LIBCCALL libc_rename(char const *oldname, char const *newname) { return libc_renameat(AT_FDCWD,oldname,AT_FDCWD,newname); }
CRT_DOS_EXT int LIBCCALL libc_dos_rename(char const *oldname, char const *newname) { return libc_frenameat(AT_FDCWD,oldname,AT_FDCWD,newname,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_renameat(fd_t oldfd, char const *oldname, fd_t newfd, char const *newname) { return libc_frenameat(oldfd,oldname,newfd,newname,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_frenameat(fd_t oldfd, char const *oldname, fd_t newfd, char const *newname, int flags) { return libc_frenameat(oldfd,oldname,newfd,newname,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_mkdir1(char const *name) { return libc_mkdirat(AT_FDCWD,name,0755); }
CRT_DOS_EXT int LIBCCALL libc_dos_mkdir1(char const *name) { return libc_fmkdirat(AT_FDCWD,name,0755,AT_DOSPATH); }
INTERN int LIBCCALL libc_mkdir(char const *name, mode_t mode) { return libc_mkdirat(AT_FDCWD,name,mode); }
CRT_DOS_EXT int LIBCCALL libc_dos_mkdir(char const *name, mode_t mode) { return libc_fmkdirat(AT_FDCWD,name,mode,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_mkdirat(fd_t dfd, char const *name, mode_t mode) { return libc_fmkdirat(dfd,name,mode,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fmkdirat(fd_t dfd, char const *name, mode_t mode, int flags) { return libc_fmkdirat(dfd,name,mode,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_symlink(char const *from, char const *to) { return libc_symlinkat(from,AT_FDCWD,to); }
CRT_DOS_EXT int LIBCCALL libc_dos_symlink(char const *from, char const *to) { return libc_fsymlinkat(from,AT_FDCWD,to,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_symlinkat(char const *from, int tofd, char const *to) { return libc_fsymlinkat(from,tofd,to,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fsymlinkat(char const *from, int tofd, char const *to, int flags) { return libc_fsymlinkat(from,tofd,to,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_mknod(char const *path, mode_t mode, dev_t dev) { return libc_mknodat(AT_FDCWD,path,mode,dev); }
CRT_DOS_EXT int LIBCCALL libc_dos_mknod(char const *path, mode_t mode, dev_t dev) { return libc_fmknodat(AT_FDCWD,path,mode,dev,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_mknodat(fd_t dfd, char const *path, mode_t mode, dev_t dev) { return libc_fmknodat(dfd,path,mode,dev,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fmknodat(fd_t dfd, char const *path, mode_t mode, dev_t dev, int flags) { return libc_fmknodat(dfd,path,mode,dev,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_xmknod(int ver, char const *path, mode_t mode, dev_t *pdev) { return libc_xmknodat(ver,AT_FDCWD,path,mode,pdev); }
CRT_DOS_EXT int LIBCCALL libc_dos_xmknod(int ver, char const *path, mode_t mode, dev_t *pdev) { return libc_dos_xmknodat(ver,AT_FDCWD,path,mode,pdev); }
INTERN int LIBCCALL libc_xmknodat(int ver, fd_t dfd, char const *path, mode_t mode, dev_t *pdev) { if (ver != 0) { libc_seterrno(EINVAL); return -1; } return libc_mknodat(dfd,path,mode,*pdev); }
CRT_DOS_EXT int LIBCCALL libc_dos_xmknodat(int ver, fd_t dfd, char const *path, mode_t mode, dev_t *pdev) { if (ver != 0) { libc_seterrno(EINVAL); return -1; } return libc_dos_mknodat(dfd,path,mode,*pdev); }
INTERN int LIBCCALL libc_mkfifo(char const *path, mode_t mode) { return libc_mknod(path,mode,S_IFIFO); }
CRT_DOS_EXT int LIBCCALL libc_dos_mkfifo(char const *path, mode_t mode) { return libc_dos_mknod(path,mode,S_IFIFO); }
INTERN int LIBCCALL libc_mkfifoat(fd_t dfd, char const *path, mode_t mode) { return libc_mknodat(dfd,path,mode,S_IFIFO); }
CRT_DOS_EXT int LIBCCALL libc_dos_mkfifoat(fd_t dfd, char const *path, mode_t mode) { return libc_dos_mknodat(dfd,path,mode,S_IFIFO); }
CRT_DOS errno_t LIBCCALL libd_access_s(char const *file, int type) { return libc_access(file,type) ? libc_geterrno() : 0; }
CRT_DOS derrno_t LIBCCALL libd_dos_access_s(char const *file, int type) { return libc_dos_access(file,type) ? libc_dos_geterrno() : 0; }
INTERN ssize_t LIBCCALL libc_freadlink(fd_t fd, char *buf, size_t buflen) { return libc_freadlinkat(fd,libc_empty_string,buf,buflen,AT_EMPTY_PATH); }
INTERN ssize_t LIBCCALL libc_readlink(char const *path, char *buf, size_t buflen) { return libc_readlinkat(AT_FDCWD,path,buf,buflen); }
CRT_DOS_EXT ssize_t LIBCCALL libc_dos_readlink(char const *path, char *buf, size_t buflen) { return libc_freadlinkat(AT_FDCWD,path,buf,buflen,AT_DOSPATH); }
CRT_DOS_EXT ssize_t LIBCCALL libc_dos_readlinkat(fd_t dfd, char const *path, char *buf, size_t buflen) { return libc_freadlinkat(dfd,path,buf,buflen,AT_DOSPATH); }
CRT_DOS_EXT ssize_t LIBCCALL libc_dos_freadlinkat(fd_t dfd, char const *path, char *buf, size_t buflen, int flags) { return libc_freadlinkat(dfd,path,buf,buflen,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_access(char const *name, int type) { return libc_faccessat(AT_FDCWD,name,type,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_access(char const *name, int type) { return libc_faccessat(AT_FDCWD,name,type,AT_DOSPATH); }
INTERN int LIBCCALL libc_eaccess(char const *name, int type) { return libc_faccessat(AT_FDCWD,name,type,AT_EACCESS); }
CRT_DOS_EXT int LIBCCALL libc_dos_eaccess(char const *name, int type) { return libc_faccessat(AT_FDCWD,name,type,AT_EACCESS|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_faccessat(fd_t dfd, char const *file, int type, int flags) { return libc_faccessat(dfd,file,type,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_ftruncate(fd_t fd, pos32_t length) { return libc_ftruncate64(fd,(pos64_t)length); }
INTERN int LIBCCALL libc_truncate(char const *file, pos32_t length) { return libc_truncate64(file,(pos64_t)length); }
CRT_DOS_EXT int LIBCCALL libc_dos_truncate(char const *file, pos32_t length) { return libc_ftruncateat64(AT_FDCWD,file,(pos64_t)length,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_truncate64(char const *file, pos64_t length) { return libc_ftruncateat64(AT_FDCWD,file,length,AT_DOSPATH); }
INTERN int LIBCCALL libc_ftruncateat(fd_t dfd, char const *file, pos32_t length, int flags) { return libc_ftruncateat64(dfd,file,(pos64_t)length,flags); }
CRT_DOS_EXT int LIBCCALL libc_dos_ftruncateat(fd_t dfd, char const *file, pos32_t length, int flags) { return libc_ftruncateat64(dfd,file,(pos64_t)length,flags|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_ftruncateat64(fd_t dfd, char const *file, pos64_t length, int flags) { return libc_ftruncateat64(dfd,file,length,flags|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_chroot(char const *path) {
 int result,new_root;
 new_root = libc_dos_open(path,O_RDONLY|O_DIRECTORY);
 if (new_root < 0) return -1;
 result = libc_dup2(new_root,AT_FDROOT);
 sys_close(new_root);
 return result < 0 ? result : 0;
}

INTERN off32_t LIBCCALL libc_lseek(fd_t fd, off32_t offset, int whence) { return (off32_t)libc_lseek64(fd,(off64_t)offset,whence); }
INTERN ssize_t LIBCCALL libc_pread(fd_t fd, void *buf, size_t bufsize, pos32_t offset) { return libc_pread64(fd,buf,bufsize,(pos64_t)offset); }
INTERN ssize_t LIBCCALL libc_pwrite(fd_t fd, void const *buf, size_t bufsize, pos32_t offset) { return libc_pwrite64(fd,buf,bufsize,(pos64_t)offset); }
INTERN ssize_t LIBCCALL libc_xpreadf(fd_t fd, void *buf, size_t bufsize, pos32_t offset, oflag_t flags) { return libc_xpreadf64(fd,buf,bufsize,offset,flags); }
INTERN ssize_t LIBCCALL libc_xpwritef(fd_t fd, void const *buf, size_t bufsize, pos32_t offset, oflag_t flags) { return libc_xpwritef64(fd,buf,bufsize,offset,flags); }
INTERN ssize_t LIBCCALL libc_ppoll64(struct pollfd *fds, nfds_t nfds, struct timespec64 const *timeout, sigset_t const *ss) { return FORWARD_SYSTEM_VALUE(sys_ppoll(fds,nfds,timeout,ss,sizeof(sigset_t))); }
INTERN ssize_t LIBCCALL
libc_pselect64(int nfds, fd_set *readfds, fd_set *writefds,
               fd_set *exceptfds, struct timespec64 const *timeout,
               sigset_t const *sigmask) {
 ssize_t error;
 if (sigmask) {
  struct {
   sigset_t const *p;
   size_t          s;
  } sgm = { sigmask, sizeof(sigset_t) };
  error = sys_pselect6(nfds,readfds,writefds,exceptfds,timeout,&sgm);
 } else {
  error = sys_pselect6(nfds,readfds,writefds,exceptfds,timeout,NULL);
 }
 if (E_ISERR(error)) {
  libc_seterrno(-error);
  return -1;
 }
 return error;
}
INTERN ssize_t LIBCCALL
libc_select64(int nfds, fd_set *readfds,
              fd_set *writefds,
              fd_set *exceptfds,
              struct timeval64 *timeout) {
 struct timespec64 tmo;
 if (!timeout) return libc_pselect64(nfds,readfds,writefds,exceptfds,NULL,NULL);
 TIMEVAL_TO_TIMESPEC(timeout,&tmo);
 return libc_pselect64(nfds,readfds,writefds,exceptfds,&tmo,NULL);
}
INTERN ssize_t LIBCCALL
libc_ppoll(struct pollfd *fds, nfds_t nfds,
           struct timespec32 const *timeout,
           sigset_t const *ss) {
 struct timespec64 t64;
 if (!timeout) return libc_ppoll64(fds,nfds,NULL,ss);
 t64.tv_sec  = timeout->tv_sec;
 t64.tv_nsec = timeout->tv_nsec;
 return libc_ppoll64(fds,nfds,&t64,ss);
}
INTERN ssize_t LIBCCALL
libc_select(int nfds, fd_set *readfds,
            fd_set *writefds,
            fd_set *exceptfds,
            struct timeval32 *timeout) {
 struct timeval64 t64;
 if (!timeout) return libc_select64(nfds,readfds,writefds,exceptfds,NULL);
 t64.tv_sec  = timeout->tv_sec;
 t64.tv_usec = timeout->tv_usec;
 return libc_select64(nfds,readfds,writefds,exceptfds,&t64);
}
INTERN ssize_t LIBCCALL
libc_pselect(int nfds, fd_set *readfds,
             fd_set *writefds,
             fd_set *exceptfds,
             struct timespec32 const *timeout,
             sigset_t const *sigmask) {
 struct timespec64 t64;
 if (!timeout) return libc_pselect64(nfds,readfds,writefds,exceptfds,NULL,sigmask);
 t64.tv_sec  = timeout->tv_sec;
 t64.tv_nsec = timeout->tv_nsec;
 return libc_pselect64(nfds,readfds,writefds,exceptfds,&t64,sigmask);
}
INTERN ssize_t LIBCCALL
libc_poll(struct pollfd *fds, nfds_t nfds, int timeout) {
 struct timespec64 tmo;
 /* NOTE: A negative value means infinite timeout! */
 if (timeout < 0)
     return libc_ppoll64(fds,nfds,NULL,NULL);
 /* NOTE: A timeout of ZERO(0) means try once; stop immediately. */
 if (!timeout) {
  tmo.tv_sec  = 0;
  tmo.tv_nsec = 0;
 } else {
  tmo.tv_sec  = timeout/MSEC_PER_SEC;
  tmo.tv_nsec = NSEC_PER_MSEC*(timeout%MSEC_PER_SEC);
 }
 return libc_ppoll64(fds,nfds,&tmo,NULL);
}
INTERN int LIBCCALL libc_pause(void) {
 return libc_pselect64(0,NULL,NULL,NULL,NULL,NULL);
}
INTERN int LIBCCALL libc_group_member(gid_t gid) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getresuid(uid_t *ruid, uid_t *euid, uid_t *suid) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_setresuid(uid_t ruid, uid_t euid, uid_t suid) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_setresgid(gid_t rgid, gid_t egid, gid_t sgid) { libc_seterrno(ENOSYS); return -1; }
PRIVATE struct fsmask current_fs_mask = { .fm_mask = 0 };
INTERN struct fsmask LIBCCALL libc_fsmode(struct fsmask new_mode) {
 struct fsmask result;
 *(u64 *)&result = sys_xfsmask(*(u64 *)&new_mode);
 current_fs_mask.fm_mask = 0;
 return result;
}
INTERN struct fsmask LIBCCALL libc_getfsmode(void) {
 if (current_fs_mask.fm_mask == 0) {
  struct fsmask temp = { (u32)-1, 0 };
  current_fs_mask = libc_fsmode(temp);
  libc_fsmode(current_fs_mask);
 }
 return current_fs_mask;
}
INTERN size_t LIBCCALL libc_confstr(int name, char *buf, size_t buflen) { libc_seterrno(ENOSYS); return 0; }
INTERN int LIBCCALL libc_setreuid(uid_t ruid, uid_t euid) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_setregid(gid_t rgid, gid_t egid) { libc_seterrno(ENOSYS); return -1; }
INTERN long int LIBCCALL libc_gethostid(void) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getpagesize(void)  { return PAGESIZE; }
INTERN int LIBCCALL libc_getdtablesize(void) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_seteuid(uid_t uid) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_setegid(gid_t gid) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_sethostid(long int id) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_frevokeat(fd_t dfd, char const *file, int flags) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_facctat(fd_t dfd, char const *name, int flags) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_revoke(char const *file) { return libc_frevokeat(AT_FDCWD,file,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_revoke(char const *file) { return libc_frevokeat(AT_FDCWD,file,AT_DOSPATH); }
INTERN int LIBCCALL libc_profil(unsigned short int *sample_buffer, size_t size, size_t offset, unsigned int scale) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_acct(char const *name) { return libc_facctat(AT_FDCWD,name,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_acct(char const *name) { return libc_facctat(AT_FDCWD,name,AT_DOSPATH); }
INTERN char *LIBCCALL libc_getusershell(void) { libc_seterrno(ENOSYS); return NULL; }
INTERN void LIBCCALL libc_endusershell(void) { }
INTERN void LIBCCALL libc_setusershell(void) { }
INTERN int LIBCCALL libc_daemon(int nochdir, int noclose) { libc_seterrno(ENOSYS); return -1; }
INTERN char *LIBCCALL libc_crypt(char const *key, char const *salt) { libc_seterrno(ENOSYS); return NULL; }
INTERN void LIBCCALL libc_encrypt(char *glibc_block, int edflag) { }
INTERN char *LIBCCALL libc_ctermid(char *s) { libc_seterrno(ENOSYS); return NULL; }


/* Export symbols (STAT). */
EXPORT(__KSYM(open),               libc_open);
EXPORT(__DSYM(_open),              libc_dos_open);
EXPORT("DOS$?_open%%YAHPEBDHH%Z",  libc_dos_open);
EXPORT(__KSYM(open64),             libc_open);
EXPORT(__DSYM(open64),             libc_dos_open);
EXPORT(__DSYM(openat),             libc_dos_openat);
EXPORT(__DSYM(openat64),           libc_dos_openat);
EXPORT(__KSYM(creat),              libc_creat);
EXPORT(__DSYM(_creat),             libc_dos_creat);
EXPORT(__KSYM(creat64),            libc_creat);
EXPORT(__DSYM(creat64),            libc_dos_creat);
EXPORT(posix_fadvise,              libc_posix_fadvise);
EXPORT(posix_fadvise64,            libc_posix_fadvise64);
EXPORT(posix_fallocate,            libc_posix_fallocate);
EXPORT(posix_fallocate64,          libc_posix_fallocate64);
EXPORT(lockf,                      libc_lockf);
EXPORT(lockf64,                    libc_lockf64);
EXPORT(fallocate,                  libc_fallocate);
EXPORT(readahead,                  libc_readahead);
EXPORT(sync_file_range,            libc_sync_file_range);
EXPORT(vmsplice,                   libc_vmsplice);
EXPORT(splice,                     libc_splice);
EXPORT(tee,                        libc_tee);
EXPORT(name_to_handle_at,          libc_name_to_handle_at);
EXPORT(open_by_handle_at,          libc_open_by_handle_at);
EXPORT(getumask,                   libc_getumask);
EXPORT(getuid,                     libc_getuid);
EXPORT(geteuid,                    libc_geteuid);
EXPORT(getgid,                     libc_getgid);
EXPORT(getegid,                    libc_getegid);
EXPORT(getgroups,                  libc_getgroups);
EXPORT(setuid,                     libc_setuid);
EXPORT(setgid,                     libc_setgid);
EXPORT(close,                      libc_close);
EXPORT(__DSYM(chdir),              libc_dos_chdir);
EXPORT(__DSYM(fchdirat),           libc_dos_fchdirat);
EXPORT(__KSYM(fpathconfat),        libc_fpathconfat);
EXPORT(__DSYM(fpathconfat),        libc_dos_fpathconfat);
EXPORT(__KSYM(pathconf),           libc_pathconf);
EXPORT(__DSYM(pathconf),           libc_dos_pathconf);
EXPORT(fpathconf,                  libc_fpathconf);
EXPORT(__KSYM(chown),              libc_chown);
EXPORT(__DSYM(chown),              libc_dos_chown);
EXPORT(__KSYM(lchown),             libc_lchown);
EXPORT(__DSYM(lchown),             libc_dos_lchown);
EXPORT(__DSYM(fchownat),           libc_dos_fchownat);
EXPORT(__KSYM(chmod),              libc_chmod);
EXPORT(__DSYM(chmod),              libc_dos_chmod);
EXPORT(__KSYM(lchmod),             libc_lchmod);
EXPORT(__DSYM(lchmod),             libc_dos_lchmod);
EXPORT(__DSYM(fchmodat),           libc_dos_fchmodat);
EXPORT(__KSYM(link),               libc_link);
EXPORT(__DSYM(link),               libc_dos_link);
EXPORT(__DSYM(linkat),             libc_dos_linkat);
EXPORT(__KSYM(unlink),             libc_unlink);
EXPORT(__DSYM(unlink),             libc_dos_unlink);
EXPORT(__DSYM(unlinkat),           libc_dos_unlinkat);
EXPORT(__KSYM(rmdir),              libc_rmdir);
EXPORT(__DSYM(rmdir),              libc_dos_rmdir);
EXPORT(__KSYM(remove),             libc_remove);
EXPORT(__DSYM(remove),             libc_dos_remove);
EXPORT(__KSYM(removeat),           libc_removeat);
EXPORT(__DSYM(removeat),           libc_dos_removeat);
EXPORT(__KSYM(rename),             libc_rename);
EXPORT(__DSYM(rename),             libc_dos_rename);
EXPORT(__DSYM(renameat),           libc_dos_renameat);
EXPORT(__DSYM(frenameat),          libc_dos_frenameat);
EXPORT(__KSYM(mkdir),              libc_mkdir);
EXPORT(__DSYM(mkdir),              libc_dos_mkdir);
EXPORT(__DSYM(mkdirat),            libc_dos_mkdirat);
EXPORT(__DSYM(fmkdirat),           libc_dos_fmkdirat);
EXPORT(__KSYM(symlink),            libc_symlink);
EXPORT(__DSYM(symlink),            libc_dos_symlink);
EXPORT(__DSYM(symlinkat),          libc_dos_symlinkat);
EXPORT(__DSYM(fsymlinkat),         libc_dos_fsymlinkat);
EXPORT(__KSYM(mknod),              libc_mknod);
EXPORT(__DSYM(mknod),              libc_dos_mknod);
EXPORT(__DSYM(mknodat),            libc_dos_mknodat);
EXPORT(__DSYM(fmknodat),           libc_dos_fmknodat);
EXPORT(__KSYM(__xmknod),           libc_xmknod);
EXPORT(__DSYM(__xmknod),           libc_dos_xmknod);
EXPORT(__KSYM(__xmknodat),         libc_xmknodat);
EXPORT(__DSYM(__xmknodat),         libc_dos_xmknodat);
EXPORT(__KSYM(mkfifo),             libc_mkfifo);
EXPORT(__DSYM(mkfifo),             libc_dos_mkfifo);
EXPORT(__KSYM(mkfifoat),           libc_mkfifoat);
EXPORT(__DSYM(mkfifoat),           libc_dos_mkfifoat);
EXPORT(freadlink,                  libc_freadlink);
EXPORT(__KSYM(readlink),           libc_readlink);
EXPORT(__DSYM(readlink),           libc_dos_readlink);
EXPORT(__DSYM(readlinkat),         libc_dos_readlinkat);
EXPORT(__DSYM(freadlinkat),        libc_dos_freadlinkat);
EXPORT(__KSYM(access),             libc_access);
EXPORT(__DSYM(access),             libc_dos_access);
EXPORT(__KSYM(eaccess),            libc_eaccess);
EXPORT(__DSYM(eaccess),            libc_dos_eaccess);
EXPORT(__DSYM(faccessat),          libc_dos_faccessat);
EXPORT(ftruncate,                  libc_ftruncate);
EXPORT(__KSYM(truncate),           libc_truncate);
EXPORT(__DSYM(truncate),           libc_dos_truncate);
EXPORT(__KSYM(ftruncateat),        libc_ftruncateat);
EXPORT(__DSYM(ftruncateat),        libc_dos_ftruncateat);
EXPORT(__DSYM(truncate64),         libc_dos_truncate64);
EXPORT(__DSYM(ftruncateat64),      libc_dos_ftruncateat64);
EXPORT(__DSYM(chroot),             libc_dos_chroot);
EXPORT(lseek,                      libc_lseek);
EXPORT(read,                       libc_read);
EXPORT(write,                      libc_write);
EXPORT(pread,                      libc_pread);
EXPORT(pwrite,                     libc_pwrite);
EXPORT(xpreadf,                    libc_xpreadf);
EXPORT(xpwritef,                   libc_xpwritef);
EXPORT(poll,                       libc_poll);
EXPORT(ppoll,                      libc_ppoll);
EXPORT(ppoll64,                    libc_ppoll64);
EXPORT(select,                     libc_select);
EXPORT(select64,                   libc_select64);
EXPORT(pselect,                    libc_pselect);
EXPORT(pselect64,                  libc_pselect64);
EXPORT(pause,                      libc_pause);
EXPORT(group_member,               libc_group_member);
EXPORT(getresuid,                  libc_getresuid);
EXPORT(getresgid,                  libc_getresgid);
EXPORT(setresuid,                  libc_setresuid);
EXPORT(setresgid,                  libc_setresgid);
EXPORT(fsmode,                     libc_fsmode);
EXPORT(getfsmode,                  libc_getfsmode);
EXPORT(confstr,                    libc_confstr);
EXPORT(setreuid,                   libc_setreuid);
EXPORT(setregid,                   libc_setregid);
EXPORT(gethostid,                  libc_gethostid);
EXPORT(getpagesize,                libc_getpagesize);
EXPORT(getdtablesize,              libc_getdtablesize);
EXPORT(seteuid,                    libc_seteuid);
EXPORT(setegid,                    libc_setegid);
EXPORT(gethostname,                libc_gethostname);
EXPORT(sethostname,                libc_sethostname);
EXPORT(sethostid,                  libc_sethostid);
EXPORT(__KSYM(revoke),             libc_revoke);
EXPORT(__DSYM(revoke),             libc_dos_revoke);
EXPORT(profil,                     libc_profil);
EXPORT(__KSYM(acct),               libc_acct);
EXPORT(__DSYM(acct),               libc_dos_acct);
EXPORT(getusershell,               libc_getusershell);
EXPORT(endusershell,               libc_endusershell);
EXPORT(setusershell,               libc_setusershell);
EXPORT(daemon,                     libc_daemon);
EXPORT(crypt,                      libc_crypt);
EXPORT(encrypt,                    libc_encrypt);
EXPORT(ctermid,                    libc_ctermid);

/* DOS function aliases. */
EXPORT(__DSYM(_chsize),            libc_ftruncate);
EXPORT(__DSYM(_close),             libc_close);
EXPORT(__DSYM(_commit),            libc_fdatasync);
EXPORT(__DSYM(_dup),               libc_dup);
EXPORT(__DSYM(_dup2),              libc_dup2);
EXPORT(__DSYM(_locking),           libc_lockf);
EXPORT(__DSYM(_lseek),             libc_lseek);
EXPORT(__DSYM(_chdir),             libc_dos_chdir);
EXPORT(__DSYM(_access),            libc_dos_access);
EXPORT(__DSYM(_chmod),             libc_dos_chmod);
EXPORT(__DSYM(_mkdir),             libc_dos_mkdir1);
EXPORT(__DSYM(_rmdir),             libc_dos_rmdir);
EXPORT(__DSYM(_unlink),            libc_dos_unlink);
EXPORT(__KSYM(access_s),           libd_access_s);
EXPORT(__DSYM(_access_s),          libd_dos_access_s);




/* DOS-exclusive functions. */
EXPORT(__lock_fhandle,            _libd_lock_fhandle);
EXPORT(_unlock_fhandle,            libd_unlock_fhandle);
EXPORT(_setmode,                   libd_setmode);
EXPORT(__KSYM(umask_s),            libc_umask_s);
EXPORT(__DSYM(_umask_s),           libd_umask_s);
EXPORT(_pipe,                      libd_pipe);



EXPORT(get_nprocs_conf,libc_get_nprocs_conf);
INTERN int LIBCCALL libc_get_nprocs_conf(void) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(get_nprocs,libc_get_nprocs);
INTERN int LIBCCALL libc_get_nprocs(void) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(get_phys_pages,libc_get_phys_pages);
INTERN intptr_t LIBCCALL libc_get_phys_pages(void) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(get_avphys_pages,libc_get_avphys_pages);
INTERN intptr_t LIBCCALL libc_get_avphys_pages(void) {
 libc_seterrno(ENOSYS);
 return -1;
}














EXPORT(Xopen,libc_Xopen);
EXPORT(Xopen64,libc_Xopen);
CRT_EXCEPT fd_t ATTR_CDECL
libc_Xopen(char const *filename, oflag_t flags, ...) {
 int COMPILER_IGNORE_UNINITIALIZED(result);
 va_list __EXCEPTVAR_VALIST args;
 va_start(args,flags);
 __TRY_VALIST {
  result = Xsys_openat(AT_FDCWD,filename,flags,va_arg(args,mode_t));
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}

EXPORT(Xcreat,libc_Xcreat);
EXPORT(Xcreat64,libc_Xcreat);
CRT_EXCEPT fd_t LIBCCALL libc_Xcreat(char const *file, mode_t mode) {
 return libc_Xopenat(AT_FDCWD,file,O_CREAT|O_WRONLY|O_TRUNC,mode);
}

EXPORT(Xpathconf,libc_Xpathconf);
CRT_EXCEPT long int LIBCCALL
libc_Xpathconf(char const *path, int name) {
 return libc_Xfpathconfat(AT_FDCWD,path,name,0);
}
EXPORT(Xfpathconf,libc_Xfpathconf);
CRT_EXCEPT long int LIBCCALL
libc_Xfpathconf(fd_t fd, int name) {
 return libc_Xfpathconfat(fd,libc_empty_string,name,AT_EMPTY_PATH);
}

EXPORT(Xchown,libc_Xchown);
CRT_EXCEPT void LIBCCALL
libc_Xchown(char const *file, uid_t owner, gid_t group) {
 Xsys_fchownat(AT_FDCWD,file,owner,group,0);
}
EXPORT(Xlchown,libc_Xlchown);
CRT_EXCEPT void LIBCCALL
libc_Xlchown(char const *file, uid_t owner, gid_t group) {
 Xsys_fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_NOFOLLOW);
}
EXPORT(Xchmod,libc_Xchmod);
CRT_EXCEPT void LIBCCALL
libc_Xchmod(char const *file, mode_t mode) {
 Xsys_fchmodat(AT_FDCWD,file,mode,0);
}
EXPORT(Xlchmod,libc_Xlchmod);
CRT_EXCEPT void LIBCCALL
libc_Xlchmod(char const *file, mode_t mode) {
 Xsys_fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_NOFOLLOW);
}
EXPORT(Xlink,libc_Xlink);
CRT_EXCEPT void LIBCCALL
libc_Xlink(char const *from, char const *to) {
 Xsys_linkat(AT_FDCWD,from,AT_FDCWD,to,0);
}
EXPORT(Xunlink,libc_Xunlink);
CRT_EXCEPT void LIBCCALL
libc_Xunlink(char const *name) {
 Xsys_unlinkat(AT_FDCWD,name,0);
}
EXPORT(Xrmdir,libc_Xrmdir);
CRT_EXCEPT void LIBCCALL
libc_Xrmdir(char const *name) {
 Xsys_unlinkat(AT_FDCWD,name,AT_REMOVEDIR);
}
EXPORT(Xremove,libc_Xremove);
CRT_EXCEPT void LIBCCALL
libc_Xremove(char const *name) {
 Xsys_unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_REMOVEREG);
}
EXPORT(Xremoveat,libc_Xremoveat);
CRT_EXCEPT void LIBCCALL
libc_Xremoveat(fd_t dfd, char const *name) {
 Xsys_unlinkat(dfd,name,AT_REMOVEDIR|AT_REMOVEREG);
}
EXPORT(Xrename,libc_Xrename);
CRT_EXCEPT void LIBCCALL
libc_Xrename(char const *oldname, char const *newname) {
 Xsys_renameat(AT_FDCWD,oldname,AT_FDCWD,newname);
}
EXPORT(Xmkdir,libc_Xmkdir);
CRT_EXCEPT void LIBCCALL
libc_Xmkdir(char const *name, mode_t mode) {
 Xsys_mkdirat(AT_FDCWD,name,mode);
}
EXPORT(Xsymlink,libc_Xsymlink);
CRT_EXCEPT void LIBCCALL
libc_Xsymlink(char const *from, char const *to) {
 Xsys_symlinkat(from,AT_FDCWD,to);
}
EXPORT(Xmknod,libc_Xmknod);
CRT_EXCEPT void LIBCCALL
libc_Xmknod(char const *path, mode_t mode, dev_t dev) {
 Xsys_mknodat(AT_FDCWD,path,mode,dev);
}
EXPORT(Xmkfifo,libc_Xmkfifo);
CRT_EXCEPT void LIBCCALL
libc_Xmkfifo(char const *path, mode_t mode) {
 if (mode & S_IFMT)
     libc_error_throw(E_INVALID_ARGUMENT);
 Xsys_mknodat(AT_FDCWD,path,S_IFIFO|mode,0);
}
EXPORT(Xmkfifoat,libc_Xmkfifoat);
CRT_EXCEPT void LIBCCALL
libc_Xmkfifoat(fd_t dfd, char const *path, mode_t mode) {
 if (mode & S_IFMT)
     libc_error_throw(E_INVALID_ARGUMENT);
 Xsys_mknodat(dfd,path,S_IFIFO|mode,0);
}
EXPORT(Xfreadlink,libc_Xfreadlink);
CRT_EXCEPT size_t LIBCCALL
libc_Xfreadlink(fd_t fd, char *buf, size_t buflen) {
 return Xsys_xfreadlinkat(fd,libc_empty_string,buf,buflen,AT_EMPTY_PATH);
}
EXPORT(Xreadlink,libc_Xreadlink);
CRT_EXCEPT size_t LIBCCALL
libc_Xreadlink(char const *path, char *buf, size_t buflen) {
 return Xsys_readlinkat(AT_FDCWD,path,buf,buflen);
}
EXPORT(Xaccess,libc_Xaccess);
CRT_EXCEPT void LIBCCALL
libc_Xaccess(char const *name, int type) {
 libc_Xfaccessat(AT_FDCWD,name,type,0);
}
EXPORT(Xeaccess,libc_Xeaccess);
CRT_EXCEPT void LIBCCALL
libc_Xeaccess(char const *name, int type) {
 libc_Xfaccessat(AT_FDCWD,name,type,AT_EACCESS);
}
EXPORT(Xftruncate,libc_Xftruncate);
CRT_EXCEPT void LIBCCALL
libc_Xftruncate(fd_t fd, pos32_t length) {
 Xsys_ftruncate(fd,length);
}
EXPORT(Xtruncate,libc_Xtruncate);
CRT_EXCEPT void LIBCCALL
libc_Xtruncate(char const *file, pos32_t length) {
 Xsys_truncate(file,length);
}
EXPORT(Xftruncateat,libc_Xftruncateat);
CRT_EXCEPT void LIBCCALL
libc_Xftruncateat(fd_t dfd, char const *file, pos32_t length, int flags) {
 Xsys_xftruncateat(dfd,file,length,flags);
}

EXPORT(Xlseek,libc_Xlseek);
CRT_EXCEPT pos32_t LIBCCALL
libc_Xlseek(fd_t fd, off32_t offset, int whence) {
 return (pos32_t)Xsys_lseek(fd,offset,whence);
}

EXPORT(Xpread,libc_Xpread);
CRT_EXCEPT size_t LIBCCALL
libc_Xpread(fd_t fd, void *buf, size_t bufsize, pos32_t offset) {
 return Xsys_pread64(fd,buf,bufsize,offset);
}

EXPORT(Xpwrite,libc_Xpwrite);
CRT_EXCEPT size_t LIBCCALL
libc_Xpwrite(fd_t fd, void const *buf, size_t bufsize, pos32_t offset) {
 return Xsys_pwrite64(fd,buf,bufsize,offset);
}

EXPORT(Xxpreadf,libc_Xxpreadf);
CRT_EXCEPT size_t LIBCCALL
libc_Xxpreadf(fd_t fd, void *buf, size_t bufsize, pos32_t offset, oflag_t flags) {
 return Xsys_xpreadf64(fd,buf,bufsize,offset,flags);
}

EXPORT(Xxpwritef,libc_Xxpwritef);
CRT_EXCEPT size_t LIBCCALL
libc_Xxpwritef(fd_t fd, void const *buf, size_t bufsize, pos32_t offset, oflag_t flags) {
 return Xsys_xpwritef64(fd,buf,bufsize,offset,flags);
}

EXPORT(Xppoll64,libc_Xppoll64);
CRT_EXCEPT size_t LIBCCALL
libc_Xppoll64(struct pollfd *fds, nfds_t nfds,
              struct timespec64 const *timeout,
              sigset_t const *ss) {
 return Xsys_ppoll(fds,nfds,timeout,ss,sizeof(sigset_t));
}

EXPORT(Xpselect64,libc_Xpselect64);
CRT_EXCEPT size_t LIBCCALL
libc_Xpselect64(int nfds, fd_set *readfds, fd_set *writefds,
                fd_set *exceptfds, struct timespec64 const *timeout,
                sigset_t const *sigmask) {
 if (sigmask) {
  struct {
   sigset_t const *p;
   size_t          s;
  } sgm = { sigmask, sizeof(sigset_t) };
  return Xsys_pselect6(nfds,readfds,writefds,exceptfds,timeout,&sgm);
 }
 return Xsys_pselect6(nfds,readfds,writefds,exceptfds,timeout,NULL);
}

EXPORT(Xpause,libc_Xpause);
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xpause(void) {
 libc_Xpselect64(0,NULL,NULL,NULL,NULL,NULL);
 assertf(0,"libc_Xpselect64() returned");
 __builtin_unreachable();
}

EXPORT(Xselect64,libc_Xselect64);
CRT_EXCEPT size_t LIBCCALL
libc_Xselect64(int nfds, fd_set *readfds,
               fd_set *writefds,
               fd_set *exceptfds,
               struct timeval64 *timeout) {
 struct timespec64 tmo;
 if (!timeout) return libc_Xpselect64(nfds,readfds,writefds,exceptfds,NULL,NULL);
 TIMEVAL_TO_TIMESPEC(timeout,&tmo);
 return libc_Xpselect64(nfds,readfds,writefds,exceptfds,&tmo,NULL);
}

EXPORT(Xppoll,libc_Xppoll);
CRT_EXCEPT size_t LIBCCALL
libc_Xppoll(struct pollfd *fds, nfds_t nfds,
            struct timespec32 const *timeout,
            sigset_t const *ss) {
 struct timespec64 t64;
 if (!timeout) return libc_Xppoll64(fds,nfds,NULL,ss);
 t64.tv_sec  = timeout->tv_sec;
 t64.tv_nsec = timeout->tv_nsec;
 return libc_Xppoll64(fds,nfds,&t64,ss);
}

EXPORT(Xselect,libc_Xselect);
CRT_EXCEPT size_t LIBCCALL
libc_Xselect(int nfds, fd_set *readfds,
             fd_set *writefds,
             fd_set *exceptfds,
             struct timeval32 *timeout) {
 struct timeval64 t64;
 if (!timeout) return libc_Xselect64(nfds,readfds,writefds,exceptfds,NULL);
 t64.tv_sec  = timeout->tv_sec;
 t64.tv_usec = timeout->tv_usec;
 return libc_Xselect64(nfds,readfds,writefds,exceptfds,&t64);
}

EXPORT(Xpselect,libc_Xpselect);
CRT_EXCEPT size_t LIBCCALL
libc_Xpselect(int nfds, fd_set *readfds,
              fd_set *writefds,
              fd_set *exceptfds,
              struct timespec32 const *timeout,
              sigset_t const *sigmask) {
 struct timespec64 t64;
 if (!timeout) return libc_Xpselect64(nfds,readfds,writefds,exceptfds,NULL,sigmask);
 t64.tv_sec  = timeout->tv_sec;
 t64.tv_nsec = timeout->tv_nsec;
 return libc_Xpselect64(nfds,readfds,writefds,exceptfds,&t64,sigmask);
}

EXPORT(Xpoll,libc_Xpoll);
CRT_EXCEPT size_t LIBCCALL
libc_Xpoll(struct pollfd *fds, nfds_t nfds, int timeout) {
 struct timespec64 tmo;
 /* NOTE: A negative value means infinite timeout! */
 if (timeout < 0)
     return libc_Xppoll64(fds,nfds,NULL,NULL);
 /* NOTE: A timeout of ZERO(0) means try once; stop immediately. */
 if (!timeout) {
  tmo.tv_sec  = 0;
  tmo.tv_nsec = 0;
 } else {
  tmo.tv_sec  = timeout/MSEC_PER_SEC;
  tmo.tv_nsec = NSEC_PER_MSEC*(timeout%MSEC_PER_SEC);
 }
 return libc_Xppoll64(fds,nfds,&tmo,NULL);
}

EXPORT(Xgroup_member,libc_Xgroup_member);
CRT_EXCEPT void LIBCCALL
libc_Xgroup_member(gid_t gid) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xgetresuid,libc_Xgetresuid);
CRT_EXCEPT void LIBCCALL
libc_Xgetresuid(uid_t *ruid, uid_t *euid, uid_t *suid) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xgetresgid,libc_Xgetresgid);
CRT_EXCEPT void LIBCCALL
libc_Xgetresgid(gid_t *rgid, gid_t *egid, gid_t *sgid) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsetresuid,libc_Xsetresuid);
CRT_EXCEPT void LIBCCALL
libc_Xsetresuid(uid_t ruid, uid_t euid, uid_t suid) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsetresgid,libc_Xsetresgid);
CRT_EXCEPT void LIBCCALL
libc_Xsetresgid(gid_t rgid, gid_t egid, gid_t sgid) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xconfstr,libc_Xconfstr);
CRT_EXCEPT size_t LIBCCALL
libc_Xconfstr(int name, char *buf, size_t buflen) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsetreuid,libc_Xsetreuid);
CRT_EXCEPT void LIBCCALL
libc_Xsetreuid(uid_t ruid, uid_t euid) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsetregid,libc_Xsetregid);
CRT_EXCEPT void LIBCCALL
libc_Xsetregid(gid_t rgid, gid_t egid) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xgethostid,libc_Xgethostid);
CRT_EXCEPT long int LIBCCALL
libc_Xgethostid(void) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsethostid,libc_Xsethostid);
CRT_EXCEPT void LIBCCALL
libc_Xsethostid(long int id) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xseteuid,libc_Xseteuid);
CRT_EXCEPT void LIBCCALL
libc_Xseteuid(uid_t uid) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsetegid,libc_Xsetegid);
CRT_EXCEPT void LIBCCALL
libc_Xsetegid(gid_t gid) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xrevoke,libc_Xrevoke);
CRT_EXCEPT void LIBCCALL
libc_Xrevoke(char const *file) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xprofil,libc_Xprofil);
CRT_EXCEPT void LIBCCALL
libc_Xprofil(unsigned short int *sample_buffer,
             size_t size, size_t offset,
             unsigned int scale) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xacct,libc_Xacct);
CRT_EXCEPT void LIBCCALL
libc_Xacct(char const *name) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xdaemon,libc_Xdaemon);
CRT_EXCEPT void LIBCCALL
libc_Xdaemon(int nochdir, int noclose) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xget_nprocs_conf,libc_Xget_nprocs_conf);
INTERN unsigned int LIBCCALL libc_Xget_nprocs_conf(void) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xget_nprocs,libc_Xget_nprocs);
INTERN unsigned int LIBCCALL libc_Xget_nprocs(void) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xget_phys_pages,libc_Xget_phys_pages);
INTERN uintptr_t LIBCCALL libc_Xget_phys_pages(void) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xget_avphys_pages,libc_Xget_avphys_pages);
INTERN uintptr_t LIBCCALL libc_Xget_avphys_pages(void) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}


EXPORT(_eof,libc_eof);
CRT_DOS WUNUSED int LIBCCALL libc_eof(fd_t fd) {
 byte_t temp; ssize_t error;
 error = libc_read(fd,&temp,1);
 if (error < 0) return -1;
 if (!error) return 1;
 libc_lseek(fd,-1,SEEK_CUR);
 return 0;
}

EXPORT(_filelength,libc_filelength);
CRT_DOS WUNUSED s32 LIBCCALL
libc_filelength(fd_t fd) {
 return (s32)libc_filelengthi64(fd);
}

EXPORT(_filelengthi64,libc_filelengthi64);
CRT_DOS WUNUSED s64 LIBCCALL
libc_filelengthi64(fd_t fd) {
 s64 oldpos = libc_lseek64(fd,0,SEEK_CUR);
 s64 length = oldpos >= 0 ? libc_lseek64(fd,0,SEEK_END) : -1;
 return oldpos >= 0 ? (libc_lseek64(fd,oldpos,SEEK_SET) >= 0 ? length : -1) : -1;
}

EXPORT(_tell,libc_tell);
CRT_DOS WUNUSED s32 LIBCCALL libc_tell(fd_t fd) {
 return libc_lseek(fd,0,SEEK_CUR);
}
EXPORT(_telli64,libc_telli64);
CRT_DOS WUNUSED s64 LIBCCALL libc_telli64(fd_t fd) {
 return libc_lseek64(fd,0,SEEK_CUR);
}
EXPORT(_get_osfhandle,libc_get_osfhandle);
CRT_DOS WUNUSED intptr_t LIBCCALL
libc_get_osfhandle(fd_t fd) {
 return (intptr_t)fd;
}
EXPORT(_open_osfhandle,libc_open_osfhandle);
CRT_DOS WUNUSED int LIBCCALL
libc_open_osfhandle(intptr_t osfd, int flags) {
 return (flags&__DOS_O_NOINHERIT)
       ? libc_fcntl((int)osfd,F_DUPFD_CLOEXEC)
       : libc_dup(flags);
}

EXPORT(__KSYM(sopen),libc_sopen);
CRT_DOS WUNUSED int ATTR_CDECL
libc_sopen(char const *file, int oflag, int sflag, ...) {
 va_list args; int result;
 va_start(args,sflag);
 result = libc_open(file,oflag,va_arg(args,mode_t));
 va_end(args);
 return result;
}
EXPORT(__DSYM(_sopen),libc_dos_sopen);
EXPORT("DOS$?_sopen%%YAHPEBDHHH%Z",libc_dos_sopen);
CRT_DOS WUNUSED int ATTR_CDECL
libc_dos_sopen(char const *file, int oflag, int sflag, ...) {
 va_list args; int result;
 va_start(args,sflag);
 result = libc_dos_open(file,oflag,va_arg(args,mode_t));
 va_end(args);
 return result;
}
EXPORT(__KSYM(sopen_s),libc_sopen_s);
CRT_DOS WUNUSED errno_t ATTR_CDECL
libc_sopen_s(int *fd, char const *file, int oflag, int sflag, ...) {
 va_list args; int result;
 if (!fd) return EINVAL;
 va_start(args,sflag);
 result = libc_open(file,oflag,va_arg(args,mode_t));
 va_end(args);
 return result >= 0 ? result : libc_geterrno();
}
EXPORT(__DSYM(_sopen_s),libc_dos_sopen_s);
CRT_DOS WUNUSED derrno_t ATTR_CDECL
libc_dos_sopen_s(int *fd, char const *file, int oflag, int sflag, ...) {
 va_list args; int result;
 if (!fd) return __DOS_EINVAL;
 va_start(args,sflag);
 result = libc_dos_open(file,oflag,va_arg(args,mode_t));
 va_end(args);
 return result >= 0 ? result : libc_dos_geterrno();
}




/* GLibc aliases. */
EXPORT_STRONG(__select,libc_select);
EXPORT_STRONG(__poll,libc_poll);
EXPORT_STRONG(__pipe,libc_pipe);
EXPORT_STRONG(__open,libc_open);
EXPORT_STRONG(__lseek,libc_lseek);
EXPORT_STRONG(__libc_pread,libc_pread);
EXPORT_STRONG(__libc_pwrite,libc_pwrite);
EXPORT_STRONG(__getpagesize,libc_getpagesize);


EXPORT_STRONG(__open_2,libc_open_2);
EXPORT_STRONG(__open64_2,libc_open_2);
CRT_RARE_RODATA char const libc_open_2_error_message[] = "invalid open call: O_CREAT without mode";
CRT_RARE int LIBCCALL libc_open_2(char const *file, oflag_t oflag) {
 if (oflag & O_CREAT) libc_fortify_fail(libc_open_2_error_message);
 return libc_open(file,oflag);
}

EXPORT_STRONG(__openat_2,libc_openat_2);
EXPORT_STRONG(__openat64_2,libc_openat_2);
CRT_RARE_RODATA char const libc_openat_2_error_message[] = "invalid openat call: O_CREAT without mode";
CRT_RARE int LIBCCALL libc_openat_2(fd_t dfd, char const *file, oflag_t oflag) {
 if (oflag & O_CREAT) libc_fortify_fail(libc_openat_2_error_message);
 return libc_openat(dfd,file,oflag);
}



DECL_END

#endif /* !GUARD_LIBS_LIBC_UNISTD_C */
