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
#define __EXPOSE_STAT_STRUCTURES 1

#include "libc.h"
#include "widechar.h"
#include "unistd.h"
#include "errno.h"
#include "err.h"
#include "system.h"
#include "malloc.h"
#include "environ.h"
#include "sched.h"

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
#include <linux/limits.h>


/* System call interface. */

DECL_BEGIN

INTERN char const libc_empty_string[] = {0,0,0,0};

INTERN int ATTR_CDECL
libc_open(char const *filename, oflag_t flags, ...) {
 va_list args; int result;
 va_start(args,flags);
 result = FORWARD_SYSTEM_VALUE(sys_openat(AT_FDCWD,filename,flags,va_arg(args,mode_t)));
 va_end(args);
 return result;
}

INTERN int ATTR_CDECL
libc_openat(fd_t dfd, char const *filename, oflag_t flags, ...) {
 va_list args; int result;
 va_start(args,flags);
 result = FORWARD_SYSTEM_VALUE(sys_openat(dfd,filename,flags,va_arg(args,mode_t)));
 va_end(args);
 return result;
}

CRT_DOS int ATTR_CDECL
libc_dos_open(char const *filename, oflag_t flags, ...) {
 va_list args; int result;
 va_start(args,flags);
 result = FORWARD_SYSTEM_VALUE(sys_openat(AT_FDCWD,filename,O_DOSPATH|flags,va_arg(args,mode_t)));
 va_end(args);
 return result;
}

CRT_DOS_EXT int ATTR_CDECL
libc_dos_openat(fd_t dfd, char const *filename, oflag_t flags, ...) {
 va_list args; int result;
 va_start(args,flags);
 result = FORWARD_SYSTEM_VALUE(sys_openat(dfd,filename,O_DOSPATH|flags,va_arg(args,mode_t)));
 va_end(args);
 return result;
}

INTERN int LIBCCALL libc_creat(char const *file, mode_t mode) {
 return FORWARD_SYSTEM_VALUE(sys_openat(AT_FDCWD,file,O_CREAT|O_WRONLY|O_TRUNC,mode));
}
CRT_DOS int LIBCCALL libc_dos_creat(char const *file, mode_t mode) {
 return FORWARD_SYSTEM_VALUE(sys_openat(AT_FDCWD,file,O_CREAT|O_WRONLY|O_TRUNC|O_DOSPATH,mode));
}
INTERN ssize_t ATTR_CDECL libc_fcntl(fd_t fd, unsigned int cmd, ...) {
 va_list args; int result;
 va_start(args,cmd);
 result = FORWARD_SYSTEM_VALUE(sys_fcntl(fd,cmd,va_arg(args,void *)));
 va_end(args);
 return result;
}
INTERN ssize_t ATTR_CDECL libc_ioctl(fd_t fd, unsigned long cmd, ...) {
 va_list args; int result;
 va_start(args,cmd);
 result = FORWARD_SYSTEM_VALUE(sys_ioctl(fd,cmd,va_arg(args,void *)));
 va_end(args);
 return result;
}
INTERN ssize_t ATTR_CDECL libc_xioctlf(fd_t fd, unsigned long cmd, oflag_t flags, ...) {
 va_list args; int result;
 va_start(args,flags);
 result = FORWARD_SYSTEM_VALUE(sys_xioctlf(fd,cmd,flags,va_arg(args,void *)));
 va_end(args);
 return result;
}
INTERN int LIBCCALL libc_posix_fadvise64(fd_t fd, pos64_t offset, pos64_t len, int advise) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_posix_fallocate64(fd_t fd, pos64_t offset, pos64_t len) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_lockf64(fd_t fd, int cmd, pos64_t len) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_fallocate64(fd_t fd, int mode, pos64_t offset, pos64_t len) { libc_seterrno(ENOSYS); return -1; }
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

INTERN int LIBCCALL libc_pipe(int pipedes[2]) { return libc_pipe2(pipedes,0); }
INTERN int LIBCCALL libc_pipe2(int pipedes[2], int flags) { return FORWARD_SYSTEM_ERROR(sys_pipe2(pipedes,flags)); }
CRT_DOS int LIBCCALL libd_pipe(int pipedes[2], u32 UNUSED(pipesize), int UNUSED(textmode)) { return libc_pipe2(pipedes,0); }
INTERN mode_t LIBCCALL libc_getumask(void) { mode_t result = sys_umask(0); sys_umask(result); return result; }
CRT_DOS errno_t LIBCCALL libc_umask_s(mode_t new_mode, mode_t *old_mode) { if (!old_mode) return EINVAL; *old_mode = libc_umask(new_mode); return 0; }
CRT_DOS derrno_t LIBCCALL libd_umask_s(mode_t new_mode, mode_t *old_mode) { if (!old_mode) return __DOS_EINVAL; *old_mode = libc_umask(new_mode); return 0; }
CRT_DOS int LIBCCALL _libd_lock_fhandle(int UNUSED(fd)) { return 0; }
CRT_DOS void LIBCCALL libd_unlock_fhandle(int UNUSED(fd)) { }
CRT_DOS mode_t LIBCCALL libd_setmode(fd_t fd, mode_t mode){ return libc_fcntl(fd,F_SETFL_XCH,mode); }
INTERN int LIBCCALL libc_fsync(fd_t fd) { return FORWARD_SYSTEM_ERROR(sys_fsync(fd)); }
INTERN int LIBCCALL libc_fdatasync(fd_t fd) { return FORWARD_SYSTEM_ERROR(sys_fdatasync(fd)); }
INTERN uid_t LIBCCALL libc_getuid(void) { libc_seterrno(ENOSYS); return -1; }
INTERN uid_t LIBCCALL libc_geteuid(void) { libc_seterrno(ENOSYS); return -1; }
INTERN gid_t LIBCCALL libc_getgid(void) { libc_seterrno(ENOSYS); return -1; }
INTERN gid_t LIBCCALL libc_getegid(void) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getgroups(int size, gid_t list[]) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_setuid(uid_t uid) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_setgid(gid_t gid) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_dup(fd_t fd) { return FORWARD_SYSTEM_VALUE(sys_dup(fd)); }
INTERN int LIBCCALL libc_dup2(int ofd, int nfd) { return libc_dup3(ofd,nfd,0); }
INTERN int LIBCCALL libc_dup3(int ofd, int nfd, int flags) { return FORWARD_SYSTEM_VALUE(sys_dup3(ofd,nfd,flags)); }
INTERN int LIBCCALL libc_close(fd_t fd) { return FORWARD_SYSTEM_ERROR(sys_close(fd)); }
INTERN int LIBCCALL libc_chdir(char const *path) { return FORWARD_SYSTEM_ERROR(sys_chdir(path)); }
CRT_DOS int LIBCCALL libc_dos_chdir(char const *path) { return libc_fchdirat(AT_FDCWD,path,AT_DOSPATH); }
INTERN int LIBCCALL libc_fchdirat(fd_t dfd, char const *path, int flags) { return FORWARD_SYSTEM_ERROR(sys_xfchdirat(dfd,path,flags)); }
CRT_DOS_EXT int LIBCCALL libc_dos_fchdirat(fd_t dfd, char const *path, int flags) { return libc_fchdirat(dfd,path,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_fchdir(fd_t fd) { return FORWARD_SYSTEM_ERROR(sys_fchdir(fd)); }
INTERN char *LIBCCALL libc_getcwd(char *buf, size_t size) { return libc_xfrealpathat(AT_FDCWD,libc_empty_string,AT_EMPTY_PATH,buf,size,REALPATH_FPATH); }
CRT_DOS char *LIBCCALL libc_dos_getcwd(char *buf, size_t size) { return libc_xfrealpathat(AT_FDCWD,libc_empty_string,AT_EMPTY_PATH,buf,size,REALPATH_FPATH|REALPATH_FDOSPATH); }
INTERN char *LIBCCALL libc_getwd(char *buf) { return libc_getcwd(buf,(size_t)-1); }
CRT_DOS_EXT char *LIBCCALL libc_dos_getwd(char *buf) { return libc_dos_getcwd(buf,(size_t)-1); }
INTERN char *LIBCCALL libc_get_current_dir_name(void) { return libc_getcwd(NULL,0); }
CRT_DOS_EXT char *LIBCCALL libc_dos_get_current_dir_name(void) { return libc_dos_getcwd(NULL,0); }

INTERN ssize_t LIBCCALL
libc_xfrealpathat2(fd_t fd, char const *path, int flags,
                   char *buf, size_t bufsize, unsigned int type) {
 return FORWARD_SYSTEM_VALUE(sys_xfrealpathat(fd,path,flags,buf,bufsize,type));
}
CRT_DOS_EXT ssize_t LIBCCALL
libc_dos_xfrealpathat2(fd_t dfd, char const *path, int flags,
                       char *buf, size_t bufsize, unsigned int type) {
 return libc_xfrealpathat2(dfd,path,flags|AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH);
}
INTERN char *LIBCCALL
libc_xfrealpathat(fd_t fd, char const *path, int flags,
                  char *buf, size_t bufsize, unsigned int type) {
 ssize_t reqsize; bool is_libc_buffer = false;
 if (!buf && bufsize && (is_libc_buffer = true,
      buf = (char *)libc_malloc(bufsize*sizeof(char))) == NULL) return NULL;
#if 1
 else if (!buf && !bufsize) {
  char *result;
  /* Allocate a small, initial buffer */
  bufsize = (260+1);
  if ((buf = (char *)libc_malloc(bufsize*sizeof(char))) == NULL) return NULL;
  reqsize = libc_xfrealpathat2(fd,path,flags,buf,bufsize,type);
  if ((ssize_t)reqsize == -1) goto err_buffer;
  if ((size_t)reqsize >= bufsize) goto do_dynamic;
  /* Free unused memory. */
  result = (char *)libc_realloc(buf,((size_t)reqsize+1));
  return likely(result) ? result : buf;
 }
#endif
 reqsize = libc_xfrealpathat2(fd,path,flags,buf,bufsize,type);
 if (reqsize == -1) {
  if (is_libc_buffer)
      goto err_buffer;
  return NULL;
 }
 if ((size_t)reqsize >= bufsize) {
  if (!buf) {
   /* Allocate a new buffer dynamically. */
do_dynamic:
   do {
    char *new_buf;
    bufsize = (size_t)reqsize;
    new_buf = (char *)libc_realloc(buf,(bufsize+1)*sizeof(char));
    if unlikely(!new_buf) {err_buffer: libc_free(buf); return NULL; }
    buf = new_buf;
    reqsize = sys_xfrealpathat(fd,path,flags,buf,bufsize+1,type);
    if unlikely(reqsize == -1) goto err_buffer;
   } while ((size_t)reqsize != bufsize);
   return buf;
  }
  libc_seterrno(ERANGE);
  return NULL;
 } else if (!reqsize && !buf) {
  /* Must allocate an empty buffer... */
  buf = (char *)libc_malloc(1*sizeof(char));
  if (buf) buf[0] = '\0';
 }
 return buf;
}
CRT_DOS_EXT char *LIBCCALL libc_dos_xfrealpathat(fd_t dfd, char const *path, int flags, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat(dfd,path,flags|AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH); }
INTERN char *LIBCCALL libc_xrealpath(char const *path, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat(AT_FDCWD,path,0,buf,bufsize,type); }
CRT_DOS_EXT char *LIBCCALL libc_dos_xrealpath(char const *path, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat(AT_FDCWD,path,AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH); }
INTERN char *LIBCCALL libc_xfrealpath(fd_t fd, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat(AT_FDCWD,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type); }
CRT_DOS_EXT char *LIBCCALL libc_dos_xfrealpath(fd_t fd, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat(AT_FDCWD,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type|REALPATH_FDOSPATH); }
INTERN ssize_t LIBCCALL libc_xrealpath2(char const *path, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat2(AT_FDCWD,path,0,buf,bufsize,type); }
CRT_DOS_EXT ssize_t LIBCCALL libc_dos_xrealpath2(char const *path, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat2(AT_FDCWD,path,AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH); }
INTERN ssize_t LIBCCALL libc_xfrealpath2(fd_t fd, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat2(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type); }
CRT_DOS_EXT ssize_t LIBCCALL libc_dos_xfrealpath2(fd_t fd, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat2(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type|REALPATH_FDOSPATH); }

INTERN char *LIBCCALL libc_realpath(char const *name, char *resolved) { return libc_xrealpath(name,resolved,resolved ? PATH_MAX : 0,REALPATH_FPATH); }
CRT_DOS_EXT char *LIBCCALL libc_dos_realpath(char const *name, char *resolved) { return libc_xrealpath(name,resolved,resolved ? PATH_MAX : 0,REALPATH_FPATH|REALPATH_FDOSPATH); }
INTERN char *LIBCCALL libc_frealpath(fd_t fd, char *resolved, size_t bufsize) { return libc_xfrealpath(fd,resolved,bufsize,REALPATH_FPATH); }
CRT_DOS_EXT char *LIBCCALL libc_dos_frealpath(fd_t fd, char *resolved, size_t bufsize) { return libc_xfrealpath(fd,resolved,bufsize,REALPATH_FPATH|REALPATH_FDOSPATH); }
INTERN char *LIBCCALL libc_canonicalize_file_name(char const *name) { return libc_realpath(name,NULL); }
CRT_DOS_EXT char *LIBCCALL libc_dos_canonicalize_file_name(char const *name) { return libc_dos_realpath(name,NULL); }

INTERN char *LIBCCALL libc_xfdname(fd_t fd, int type, char *buf, size_t bufsize) { return libc_xfrealpathat(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type); }
CRT_DOS_EXT char *LIBCCALL libc_dos_xfdname(fd_t fd, int type, char *buf, size_t bufsize) { return libc_xfrealpathat(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type|REALPATH_FDOSPATH); }
INTERN ssize_t LIBCCALL libc_xfdname2(fd_t fd, int type, char *buf, size_t bufsize) { return libc_xfrealpathat2(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type); }
CRT_DOS_EXT ssize_t LIBCCALL libc_dos_xfdname2(fd_t fd, int type, char *buf, size_t bufsize) { return libc_xfrealpathat2(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type|REALPATH_FDOSPATH); }
INTERN long int LIBCCALL libc_fpathconfat(fd_t dfd, char const *file, int name, int flags) { return FORWARD_SYSTEM_VALUE(sys_xfpathconfat(dfd,file,name,flags)); }
CRT_DOS_EXT long int LIBCCALL libc_dos_fpathconfat(fd_t dfd, char const *file, int name, int flags) { return libc_fpathconfat(dfd,file,name,flags|AT_DOSPATH); }
INTERN long int LIBCCALL libc_pathconf(char const *path, int name) { return libc_fpathconfat(AT_FDCWD,path,name,0); }
CRT_DOS_EXT long int LIBCCALL libc_dos_pathconf(char const *path, int name) { return libc_fpathconfat(AT_FDCWD,path,name,AT_DOSPATH); }
INTERN long int LIBCCALL libc_fpathconf(fd_t fd, int name) { return libc_fpathconfat(fd,libc_empty_string,name,AT_EMPTY_PATH); }
INTERN int LIBCCALL libc_chown(char const *file, uid_t owner, gid_t group) { return libc_fchownat(AT_FDCWD,file,owner,group,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_chown(char const *file, uid_t owner, gid_t group) { return libc_fchownat(AT_FDCWD,file,owner,group,AT_DOSPATH); }
INTERN int LIBCCALL libc_fchown(fd_t fd, uid_t owner, gid_t group) { return FORWARD_SYSTEM_ERROR(sys_fchown(fd,owner,group)); }
INTERN int LIBCCALL libc_lchown(char const *file, uid_t owner, gid_t group) { return libc_fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_NOFOLLOW); }
CRT_DOS_EXT int LIBCCALL libc_dos_lchown(char const *file, uid_t owner, gid_t group) { return libc_fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
INTERN int LIBCCALL libc_fchownat(fd_t dfd, char const *file, uid_t owner, gid_t group, int flags) { return FORWARD_SYSTEM_ERROR(sys_fchownat(dfd,file,owner,group,flags)); }
CRT_DOS_EXT int LIBCCALL libc_dos_fchownat(fd_t dfd, char const *file, uid_t owner, gid_t group, int flags) { return libc_fchownat(dfd,file,owner,group,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_fchmod(fd_t fd, mode_t mode) { return FORWARD_SYSTEM_ERROR(sys_fchmod(fd,mode)); }
INTERN int LIBCCALL libc_chmod(char const *file, mode_t mode) { return libc_fchmodat(AT_FDCWD,file,mode,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_chmod(char const *file, mode_t mode) { return libc_fchmodat(AT_FDCWD,file,mode,AT_DOSPATH); }
INTERN int LIBCCALL libc_lchmod(char const *file, mode_t mode) { return libc_fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_NOFOLLOW); }
CRT_DOS_EXT int LIBCCALL libc_dos_lchmod(char const *file, mode_t mode) { return libc_fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
INTERN int LIBCCALL libc_fchmodat(fd_t dfd, char const *file, mode_t mode, int flags) { return FORWARD_SYSTEM_ERROR(sys_fchmodat(dfd,file,mode,flags)); }
CRT_DOS_EXT int LIBCCALL libc_dos_fchmodat(fd_t dfd, char const *file, mode_t mode, int flags) { return libc_fchmodat(dfd,file,mode,flags); }
INTERN int LIBCCALL libc_link(char const *from, char const *to) { return libc_linkat(AT_FDCWD,from,AT_FDCWD,to,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_link(char const *from, char const *to) { return libc_linkat(AT_FDCWD,from,AT_FDCWD,to,AT_DOSPATH); }
INTERN int LIBCCALL libc_linkat(int fromfd, char const *from, int tofd, char const *to, int flags) { return FORWARD_SYSTEM_ERROR(sys_linkat(fromfd,from,tofd,to,flags)); }
CRT_DOS_EXT int LIBCCALL libc_dos_linkat(int fromfd, char const *from, int tofd, char const *to, int flags) { return libc_linkat(fromfd,from,tofd,to,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_unlink(char const *name) { return libc_unlinkat(AT_FDCWD,name,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_unlink(char const *name) { return libc_unlinkat(AT_FDCWD,name,AT_DOSPATH); }
INTERN int LIBCCALL libc_unlinkat(fd_t dfd, char const *name, int flags) { return FORWARD_SYSTEM_ERROR(sys_unlinkat(dfd,name,flags)); }
CRT_DOS_EXT int LIBCCALL libc_dos_unlinkat(fd_t dfd, char const *name, int flags) { return libc_unlinkat(dfd,name,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_rmdir(char const *name) { return libc_unlinkat(AT_FDCWD,name,AT_REMOVEDIR); }
CRT_DOS_EXT int LIBCCALL libc_dos_rmdir(char const *name) { return libc_unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_DOSPATH); }
INTERN int LIBCCALL libc_remove(char const *name) { return libc_unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_REMOVEREG); }
CRT_DOS_EXT int LIBCCALL libc_dos_remove(char const *name) { return libc_unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_REMOVEREG|AT_DOSPATH); }
INTERN int LIBCCALL libc_removeat(fd_t dfd, char const *name) { return libc_unlinkat(dfd,name,AT_REMOVEDIR|AT_REMOVEREG); }
CRT_DOS_EXT int LIBCCALL libc_dos_removeat(fd_t dfd, char const *name) { return libc_unlinkat(dfd,name,AT_REMOVEDIR|AT_REMOVEREG|AT_DOSPATH); }
INTERN int LIBCCALL libc_rename(char const *oldname, char const *newname) { return libc_renameat(AT_FDCWD,oldname,AT_FDCWD,newname); }
CRT_DOS_EXT int LIBCCALL libc_dos_rename(char const *oldname, char const *newname) { return libc_frenameat(AT_FDCWD,oldname,AT_FDCWD,newname,AT_DOSPATH); }
INTERN int LIBCCALL libc_renameat(fd_t oldfd, char const *oldname, fd_t newfd, char const *newname) { return FORWARD_SYSTEM_ERROR(sys_renameat(oldfd,oldname,newfd,newname)); }
CRT_DOS_EXT int LIBCCALL libc_dos_renameat(fd_t oldfd, char const *oldname, fd_t newfd, char const *newname) { return libc_frenameat(oldfd,oldname,newfd,newname,AT_DOSPATH); }
INTERN int LIBCCALL libc_frenameat(fd_t oldfd, char const *oldname, fd_t newfd, char const *newname, int flags) { return FORWARD_SYSTEM_ERROR(sys_xfrenameat(oldfd,oldname,newfd,newname,flags)); }
CRT_DOS_EXT int LIBCCALL libc_dos_frenameat(fd_t oldfd, char const *oldname, fd_t newfd, char const *newname, int flags) { return libc_frenameat(oldfd,oldname,newfd,newname,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_mkdir1(char const *name) { return libc_mkdirat(AT_FDCWD,name,0755); }
CRT_DOS_EXT int LIBCCALL libc_dos_mkdir1(char const *name) { return libc_fmkdirat(AT_FDCWD,name,0755,AT_DOSPATH); }
INTERN int LIBCCALL libc_mkdir(char const *name, mode_t mode) { return libc_mkdirat(AT_FDCWD,name,mode); }
CRT_DOS_EXT int LIBCCALL libc_dos_mkdir(char const *name, mode_t mode) { return libc_fmkdirat(AT_FDCWD,name,mode,AT_DOSPATH); }
INTERN int LIBCCALL libc_mkdirat(fd_t dfd, char const *name, mode_t mode) { return FORWARD_SYSTEM_ERROR(sys_mkdirat(dfd,name,mode)); }
CRT_DOS_EXT int LIBCCALL libc_dos_mkdirat(fd_t dfd, char const *name, mode_t mode) { return libc_fmkdirat(dfd,name,mode,AT_DOSPATH); }
INTERN int LIBCCALL libc_fmkdirat(fd_t dfd, char const *name, mode_t mode, int flags) { return FORWARD_SYSTEM_ERROR(sys_xfmkdirat(dfd,name,mode,flags)); }
CRT_DOS_EXT int LIBCCALL libc_dos_fmkdirat(fd_t dfd, char const *name, mode_t mode, int flags) { return libc_fmkdirat(dfd,name,mode,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_symlink(char const *from, char const *to) { return libc_symlinkat(from,AT_FDCWD,to); }
CRT_DOS_EXT int LIBCCALL libc_dos_symlink(char const *from, char const *to) { return libc_fsymlinkat(from,AT_FDCWD,to,AT_DOSPATH); }
INTERN int LIBCCALL libc_symlinkat(char const *from, int tofd, char const *to) { return FORWARD_SYSTEM_ERROR(sys_symlinkat(from,tofd,to)); }
CRT_DOS_EXT int LIBCCALL libc_dos_symlinkat(char const *from, int tofd, char const *to) { return libc_fsymlinkat(from,tofd,to,AT_DOSPATH); }
INTERN int LIBCCALL libc_fsymlinkat(char const *from, int tofd, char const *to, int flags) { return FORWARD_SYSTEM_ERROR(sys_xfsymlinkat(from,tofd,to,flags)); }
CRT_DOS_EXT int LIBCCALL libc_dos_fsymlinkat(char const *from, int tofd, char const *to, int flags) { return libc_fsymlinkat(from,tofd,to,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_mknod(char const *path, mode_t mode, dev_t dev) { return libc_mknodat(AT_FDCWD,path,mode,dev); }
CRT_DOS_EXT int LIBCCALL libc_dos_mknod(char const *path, mode_t mode, dev_t dev) { return libc_fmknodat(AT_FDCWD,path,mode,dev,AT_DOSPATH); }
INTERN int LIBCCALL libc_mknodat(fd_t dfd, char const *path, mode_t mode, dev_t dev) { return FORWARD_SYSTEM_ERROR(sys_mknodat(dfd,path,mode,dev)); }
CRT_DOS_EXT int LIBCCALL libc_dos_mknodat(fd_t dfd, char const *path, mode_t mode, dev_t dev) { return libc_fmknodat(dfd,path,mode,dev,AT_DOSPATH); }
INTERN int LIBCCALL libc_fmknodat(fd_t dfd, char const *path, mode_t mode, dev_t dev, int flags) { return FORWARD_SYSTEM_ERROR(sys_xfmknodat(dfd,path,mode,dev,flags)); }
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
INTERN ssize_t LIBCCALL libc_readlinkat(fd_t dfd, char const *path, char *buf, size_t buflen) { return FORWARD_SYSTEM_VALUE(sys_readlinkat(dfd,path,buf,buflen)); }
CRT_DOS_EXT ssize_t LIBCCALL libc_dos_readlinkat(fd_t dfd, char const *path, char *buf, size_t buflen) { return libc_freadlinkat(dfd,path,buf,buflen,AT_DOSPATH); }
INTERN ssize_t LIBCCALL libc_freadlinkat(fd_t dfd, char const *path, char *buf, size_t buflen, int flags) { return FORWARD_SYSTEM_VALUE(sys_xfreadlinkat(dfd,path,buf,buflen,flags)); }
CRT_DOS_EXT ssize_t LIBCCALL libc_dos_freadlinkat(fd_t dfd, char const *path, char *buf, size_t buflen, int flags) { return libc_freadlinkat(dfd,path,buf,buflen,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_access(char const *name, int type) { return libc_faccessat(AT_FDCWD,name,type,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_access(char const *name, int type) { return libc_faccessat(AT_FDCWD,name,type,AT_DOSPATH); }
INTERN int LIBCCALL libc_eaccess(char const *name, int type) { return libc_faccessat(AT_FDCWD,name,type,AT_EACCESS); }
CRT_DOS_EXT int LIBCCALL libc_dos_eaccess(char const *name, int type) { return libc_faccessat(AT_FDCWD,name,type,AT_EACCESS|AT_DOSPATH); }
INTERN int LIBCCALL libc_faccessat(fd_t dfd, char const *file, int type, int flags) { return FORWARD_SYSTEM_ERROR(sys_faccessat(dfd,file,type,flags)); }
CRT_DOS_EXT int LIBCCALL libc_dos_faccessat(fd_t dfd, char const *file, int type, int flags) { return libc_faccessat(dfd,file,type,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_ftruncate(fd_t fd, pos32_t length) { return libc_ftruncate64(fd,(pos64_t)length); }
INTERN int LIBCCALL libc_ftruncate64(fd_t fd, pos64_t length) { return FORWARD_SYSTEM_ERROR(sys_ftruncate(fd,length)); }
INTERN int LIBCCALL libc_truncate(char const *file, pos32_t length) { return libc_truncate64(file,(pos64_t)length); }
INTERN int LIBCCALL libc_truncate64(char const *file, pos64_t length) { return FORWARD_SYSTEM_ERROR(sys_truncate(file,length)); }
CRT_DOS_EXT int LIBCCALL libc_dos_truncate(char const *file, pos32_t length) { return libc_ftruncateat64(AT_FDCWD,file,(pos64_t)length,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_truncate64(char const *file, pos64_t length) { return libc_ftruncateat64(AT_FDCWD,file,length,AT_DOSPATH); }
INTERN int LIBCCALL libc_ftruncateat(fd_t dfd, char const *file, pos32_t length, int flags) { return libc_ftruncateat64(dfd,file,(pos64_t)length,flags); }
INTERN int LIBCCALL libc_ftruncateat64(fd_t dfd, char const *file, pos64_t length, int flags) { return FORWARD_SYSTEM_ERROR(sys_xftruncateat(dfd,file,length,flags)); }
CRT_DOS_EXT int LIBCCALL libc_dos_ftruncateat(fd_t dfd, char const *file, pos32_t length, int flags) { return libc_ftruncateat64(dfd,file,(pos64_t)length,flags|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_ftruncateat64(fd_t dfd, char const *file, pos64_t length, int flags) { return libc_ftruncateat64(dfd,file,length,flags|AT_DOSPATH); }
INTERN int LIBCCALL libc_chroot(char const *path) { return FORWARD_SYSTEM_ERROR(sys_chroot(path)); }
CRT_DOS_EXT int LIBCCALL libc_dos_chroot(char const *path) {
 int result,new_root;
 new_root = libc_dos_open(path,O_RDONLY|O_DIRECTORY);
 if (new_root < 0) return -1;
 result = libc_dup2(new_root,AT_FDROOT);
 sys_close(new_root);
 return result < 0 ? result : 0;
}
CRT_DOS_NATIVE char *LIBCCALL libd_getdcwd(int drive, char *buf, size_t size) { return libc_xfdname(AT_FDDRIVE_CWD('A'+drive),REALPATH_FPATH,buf,size); }
CRT_DOS char *LIBCCALL libd_dos_getdcwd(int drive, char *buf, size_t size) { return libc_xfdname(AT_FDDRIVE_CWD('A'+drive),REALPATH_FPATH|REALPATH_FDOSPATH,buf,size); }
CRT_DOS_NATIVE int LIBCCALL libd_chdrive(int drive) { char temp[3] = "?:"; temp[0] = 'A'+drive; return libc_dos_chdir(temp); }
CRT_DOS_NATIVE int LIBCCALL libd_getdrive(void) { char buf[1]; return libc_xfdname2(AT_FDCWD,REALPATH_FDRIVE,buf,1) < 0 ? -1 : (buf[0]-'A'); }
CRT_DOS_NATIVE u32 LIBCCALL libd_getdrives(void) { return sys_xgetdrives(); }

INTERN ssize_t LIBCCALL libc_read(fd_t fd, void *buf, size_t bufsize) {
 return FORWARD_SYSTEM_VALUE(sys_read(fd,buf,bufsize));
}
INTERN ssize_t LIBCCALL libc_write(fd_t fd, void const *buf, size_t bufsize) { return FORWARD_SYSTEM_VALUE(sys_write(fd,buf,bufsize)); }
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
DEFINE_INTERN_ALIAS(libd_read,libc_read);
DEFINE_INTERN_ALIAS(libd_write,libc_write);
#else
CRT_DOS int LIBCCALL libd_read(fd_t fd, void *buf, unsigned int bufsize) { return (int)libc_read(fd,buf,(size_t)bufsize); }
CRT_DOS int LIBCCALL libd_write(fd_t fd, void const *buf, unsigned int bufsize) { return (int)libc_write(fd,buf,(size_t)bufsize); }
#endif
INTERN off64_t LIBCCALL libc_lseek64(fd_t fd, off64_t offset, int whence) { return FORWARD_SYSTEM_VALUE(sys_lseek(fd,offset,whence)); }
INTERN off32_t LIBCCALL libc_lseek(fd_t fd, off32_t offset, int whence) { return (off32_t)libc_lseek64(fd,(off64_t)offset,whence); }
INTERN ssize_t LIBCCALL libc_pread(fd_t fd, void *buf, size_t bufsize, pos32_t offset) { return libc_pread64(fd,buf,bufsize,(pos64_t)offset); }
INTERN ssize_t LIBCCALL libc_pread64(fd_t fd, void *buf, size_t bufsize, pos64_t offset) { return FORWARD_SYSTEM_VALUE(sys_pread64(fd,buf,bufsize,offset)); }
INTERN ssize_t LIBCCALL libc_pwrite(fd_t fd, void const *buf, size_t bufsize, pos32_t offset) { return libc_pwrite64(fd,buf,bufsize,(pos64_t)offset); }
INTERN ssize_t LIBCCALL libc_pwrite64(fd_t fd, void const *buf, size_t bufsize, pos64_t offset) { return FORWARD_SYSTEM_VALUE(sys_pwrite64(fd,buf,bufsize,offset)); }
INTERN ssize_t LIBCCALL libc_xreadf(fd_t fd, void *buf, size_t bufsize, oflag_t flags) { return FORWARD_SYSTEM_VALUE(sys_xreadf(fd,buf,bufsize,flags)); }
INTERN ssize_t LIBCCALL libc_xwritef(fd_t fd, void const *buf, size_t bufsize, oflag_t flags) { return FORWARD_SYSTEM_VALUE(sys_xwritef(fd,buf,bufsize,flags)); }
INTERN ssize_t LIBCCALL libc_xpreadf(fd_t fd, void *buf, size_t bufsize, pos32_t offset, oflag_t flags) { return FORWARD_SYSTEM_VALUE(sys_xpreadf64(fd,buf,bufsize,offset,flags)); }
INTERN ssize_t LIBCCALL libc_xpwritef(fd_t fd, void const *buf, size_t bufsize, pos32_t offset, oflag_t flags) { return FORWARD_SYSTEM_VALUE(sys_xpwritef64(fd,buf,bufsize,offset,flags)); }
INTERN ssize_t LIBCCALL libc_xpreadf64(fd_t fd, void *buf, size_t bufsize, pos64_t offset, oflag_t flags) { return FORWARD_SYSTEM_VALUE(sys_xpreadf64(fd,buf,bufsize,offset,flags)); }
INTERN ssize_t LIBCCALL libc_xpwritef64(fd_t fd, void const *buf, size_t bufsize, pos64_t offset, oflag_t flags) { return FORWARD_SYSTEM_VALUE(sys_xpwritef64(fd,buf,bufsize,offset,flags)); }
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
  struct fsmask temp = {(u32)-1,0};
  current_fs_mask = libc_fsmode(temp);
  libc_fsmode(current_fs_mask);
 }
 return current_fs_mask;
}
INTERN size_t LIBCCALL libc_confstr(int name, char *buf, size_t buflen) { libc_seterrno(ENOSYS); return 0; }
INTERN int LIBCCALL libc_syncfs(fd_t fd) { return FORWARD_SYSTEM_ERROR(sys_syncfs(fd)); }
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
INTERN long int LIBCCALL libc_sysconf(int name) { return FORWARD_SYSTEM_VALUE(sys_xsysconf((unsigned int)name)); }
INTERN int LIBCCALL libc_kfstat64(fd_t fd, struct __kos_stat64 *buf) { return FORWARD_SYSTEM_ERROR(sys_fstat64(fd,buf)); }
INTERN int LIBCCALL libc_kstat64(char const *file, struct __kos_stat64 *buf) { return libc_kfstatat64(AT_FDCWD,file,buf,0); }
INTERN int LIBCCALL libc_kfstatat64(fd_t dfd, char const *file, struct __kos_stat64 *buf, int flags) { return FORWARD_SYSTEM_ERROR(sys_fstatat64(dfd,file,buf,flags)); }
INTERN int LIBCCALL libc_klstat64(char const *file, struct __kos_stat64 *buf) { return libc_kfstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW); }
CRT_DOS_EXT int LIBCCALL libc_dos_kstat64(char const *file, struct __kos_stat64 *buf) { return libc_kfstatat64(AT_FDCWD,file,buf,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_kfstatat64(fd_t dfd, char const *file, struct __kos_stat64 *buf, int flags) { return libc_kfstatat64(dfd,file,buf,flags|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_klstat64(char const *file, struct __kos_stat64 *buf) { return libc_kfstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
DEFINE_INTERN_ALIAS(libc_kfstat,libc_kfstat64);
DEFINE_INTERN_ALIAS(libc_kstat,libc_kstat64);
DEFINE_INTERN_ALIAS(libc_dos_kstat,libc_dos_kstat64);
DEFINE_INTERN_ALIAS(libc_kfstatat,libc_kfstatat64);
DEFINE_INTERN_ALIAS(libc_dos_kfstatat,libc_dos_kfstatat64);
DEFINE_INTERN_ALIAS(libc_klstat,libc_klstat64);
DEFINE_INTERN_ALIAS(libc_dos_klstat,libc_dos_klstat64);

CRT_GLC void LIBCCALL
stat2glc(struct __kos_stat64 const *__restrict in,
         struct __glc_stat *__restrict out) {
 out->st_dev     = in->st_dev;
 out->st_ino32   = in->st_ino32;
 out->st_nlink   = in->st_nlink;
 out->st_mode    = in->st_mode;
 out->st_uid     = in->st_uid;
 out->st_gid     = in->st_gid;
 out->st_rdev    = in->st_rdev;
 out->st_size32  = in->st_size32;
 out->st_blksize = in->st_blksize;
 out->st_blocks  = in->st_blocks;
 out->st_atim    = in->st_atim32;
 out->st_mtim    = in->st_mtim32;
 out->st_ctim    = in->st_ctim32;
}
CRT_GLC void LIBCCALL
stat2glc64(struct __kos_stat64 const *__restrict in,
           struct __glc_stat64 *__restrict out) {
 out->st_dev     = in->st_dev;
 out->st_ino32   = in->st_ino32;
 out->st_ino64   = in->st_ino64;
 out->st_nlink   = in->st_nlink;
 out->st_mode    = in->st_mode;
 out->st_uid     = in->st_uid;
 out->st_gid     = in->st_gid;
 out->st_rdev    = in->st_rdev;
 out->st_size    = in->st_size64;
 out->st_blksize = in->st_blksize;
 out->st_blocks  = in->st_blocks;
 out->st_atim    = in->st_atim32;
 out->st_mtim    = in->st_mtim32;
 out->st_ctim    = in->st_ctim32;
}

CRT_GLC int LIBCCALL libc_fstat(fd_t fd, struct __glc_stat *buf) {
 struct __kos_stat64 temp; int result;
 result = libc_kfstat64(fd,&temp);
 if (!result) stat2glc(&temp,buf);
 return result;
}
CRT_GLC int LIBCCALL libc_fstat64(fd_t fd, struct __glc_stat64 *buf) {
 struct __kos_stat64 temp; int result;
 result = libc_kfstat64(fd,&temp);
 if (!result) stat2glc64(&temp,buf);
 return result;
}
CRT_GLC int LIBCCALL libc_fstatat(fd_t dfd, char const *file, struct __glc_stat *buf, int flags) {
 struct __kos_stat64 temp; int result;
 result = libc_kfstatat64(dfd,file,&temp,flags);
 if (!result) stat2glc(&temp,buf);
 return result;
}
CRT_GLC int LIBCCALL libc_fstatat64(fd_t dfd, char const *file, struct __glc_stat64 *buf, int flags) {
 struct __kos_stat64 temp; int result;
 result = libc_kfstatat64(dfd,file,&temp,flags);
 if (!result) stat2glc64(&temp,buf);
 return result;
}

CRT_GLC int LIBCCALL libc_stat(char const *file, struct __glc_stat *buf) { return libc_fstatat(AT_FDCWD,file,buf,0); }
CRT_GLC int LIBCCALL libc_stat64(char const *file, struct __glc_stat64 *buf) { return libc_fstatat64(AT_FDCWD,file,buf,0); }
CRT_GLC int LIBCCALL libc_lstat(char const *file, struct __glc_stat *buf) { return libc_fstatat(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW); }
CRT_GLC int LIBCCALL libc_lstat64(char const *file, struct __glc_stat64 *buf) { return libc_fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW); }
CRT_DOS_EXT int LIBCCALL libc_dos_stat(char const *file, struct __glc_stat *buf) { return libc_fstatat(AT_FDCWD,file,buf,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_stat64(char const *file, struct __glc_stat64 *buf) { return libc_fstatat64(AT_FDCWD,file,buf,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_lstat(char const *file, struct __glc_stat *buf) { return libc_fstatat(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_lstat64(char const *file, struct __glc_stat64 *buf) { return libc_fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fstatat(fd_t dfd, char const *file, struct __glc_stat *buf, int flags) { return libc_fstatat(dfd,file,buf,flags|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fstatat64(fd_t dfd, char const *file, struct __glc_stat64 *buf, int flags) { return libc_fstatat64(dfd,file,buf,flags|AT_DOSPATH); }

#define VERCHK { if (ver != 0) { libc_seterrno(EINVAL); return -1; } }
CRT_GLC int LIBCCALL libc_version_fxstat(int ver, fd_t fd, struct __glc_stat *statbuf) { VERCHK return libc_fstat(fd,statbuf); }
CRT_GLC int LIBCCALL libc_version_fxstat64(int ver, fd_t fd, struct __glc_stat64 *statbuf) { VERCHK return libc_fstat64(fd,statbuf); }
CRT_GLC int LIBCCALL libc_version_xstat(int ver, char const *filename, struct __glc_stat *statbuf) { VERCHK return libc_stat(filename,statbuf); }
CRT_GLC int LIBCCALL libc_version_xstat64(int ver, char const *filename, struct __glc_stat64 *statbuf) { VERCHK return libc_stat64(filename,statbuf); }
CRT_GLC int LIBCCALL libc_version_lxstat(int ver, char const *filename, struct __glc_stat *statbuf) { VERCHK return libc_lstat(filename,statbuf); }
CRT_GLC int LIBCCALL libc_version_lxstat64(int ver, char const *filename, struct __glc_stat64 *statbuf) { VERCHK return libc_lstat64(filename,statbuf); }
CRT_GLC int LIBCCALL libc_version_fxstatat(int ver, fd_t fd, char const *filename, struct __glc_stat *statbuf, int flags) { VERCHK return libc_fstatat(fd,filename,statbuf,flags); }
CRT_GLC int LIBCCALL libc_version_fxstatat64(int ver, fd_t fd, char const *filename, struct __glc_stat64 *statbuf, int flags) { VERCHK return libc_fstatat64(fd,filename,statbuf,flags); }
CRT_DOS_EXT int LIBCCALL libc_dos_version_xstat(int ver, char const *filename, struct __glc_stat *statbuf) { VERCHK return libc_dos_stat(filename,statbuf); }
CRT_DOS_EXT int LIBCCALL libc_dos_version_xstat64(int ver, char const *filename, struct __glc_stat64 *statbuf) { VERCHK return libc_dos_stat64(filename,statbuf); }
CRT_DOS_EXT int LIBCCALL libc_dos_version_lxstat(int ver, char const *filename, struct __glc_stat *statbuf) { VERCHK return libc_dos_lstat(filename,statbuf); }
CRT_DOS_EXT int LIBCCALL libc_dos_version_lxstat64(int ver, char const *filename, struct __glc_stat64 *statbuf) { VERCHK return libc_dos_lstat64(filename,statbuf); }
CRT_DOS_EXT int LIBCCALL libc_dos_version_fxstatat(int ver, fd_t fd, char const *filename, struct __glc_stat *statbuf, int flags) { VERCHK return libc_dos_fstatat(fd,filename,statbuf,flags); }
CRT_DOS_EXT int LIBCCALL libc_dos_version_fxstatat64(int ver, fd_t fd, char const *filename, struct __glc_stat64 *statbuf, int flags) { VERCHK return libc_dos_fstatat64(fd,filename,statbuf,flags); }
#undef VERCHK

/* DOS-FS mode + DOS-binary-compatible stat functions.
 * NOTE: These are only used when user-apps are configures as `__PE__' + `__USE_DOS'.
 *       Otherwise, KOS's kernel stat buffer data layout is used to maximize performance.  */
CRT_DOS void LIBCCALL
stat2comdos(struct __kos_stat64 const *__restrict src,
            struct __dos_stat32 *__restrict buf) {
 buf->st_dev   = (__dos_dev_t)src->st_dev;
 buf->st_ino   = (__dos_ino_t)src->st_ino;
 buf->st_mode  = (__uint16_t)src->st_mode;
 buf->st_nlink = (__int16_t)src->st_nlink;
 buf->st_uid   = (__int16_t)src->st_uid;
 buf->st_gid   = (__int16_t)src->st_gid;
 buf->st_rdev  = (__dos_dev_t)src->st_rdev;
}
CRT_DOS void LIBCCALL
stat2dos32(struct __kos_stat64 const *__restrict src,
           struct __dos_stat32 *__restrict buf) {
 stat2comdos(src,(struct __dos_stat32 *)buf);
 buf->st_size  = src->st_size32;
 buf->st_atime = src->st_atime32;
 buf->st_mtime = src->st_mtime32;
 buf->st_ctime = src->st_ctime32;
}
CRT_DOS void LIBCCALL
stat2dos64(struct __kos_stat64 const *__restrict src,
           struct __dos_stat64 *__restrict buf) {
 stat2comdos(src,(struct __dos_stat32 *)buf);
 buf->st_size  = src->st_size64;
 buf->st_atime = src->st_atime64;
 buf->st_mtime = src->st_mtime64;
 buf->st_ctime = src->st_ctime64;
}
CRT_DOS void LIBCCALL
stat2dos32i64(struct __kos_stat64 const *__restrict src,
              struct __dos_stat32i64 *__restrict buf) {
 stat2comdos(src,(struct __dos_stat32 *)buf);
 buf->st_size  = src->st_size64;
 buf->st_atime = src->st_atime32;
 buf->st_mtime = src->st_mtime32;
 buf->st_ctime = src->st_ctime32;
}

CRT_DOS int LIBCCALL libd_fstat32(fd_t fd, struct __dos_stat32 *buf) { struct __kos_stat64 temp; int result = libc_kfstat64(fd,&temp); if (!result) stat2dos32(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_fstat32i64(fd_t fd, struct __dos_stat32i64 *buf) { struct __kos_stat64 temp; int result = libc_kfstat64(fd,&temp); if (!result) stat2dos32i64(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_fstat64(fd_t fd, struct __dos_stat64 *buf) { struct __kos_stat64 temp; int result = libc_kfstat64(fd,&temp); if (!result) stat2dos64(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_stat32(char const *file, struct __dos_stat32 *buf) { struct __kos_stat64 temp; int result = libc_kstat64(file,&temp); if (!result) stat2dos32(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_stat32i64(char const *file, struct __dos_stat32i64 *buf) { struct __kos_stat64 temp; int result = libc_kstat64(file,&temp); if (!result) stat2dos32i64(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_stat64(char const *file, struct __dos_stat64 *buf) { struct __kos_stat64 temp; int result = libc_kstat64(file,&temp); if (!result) stat2dos64(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_dos_stat32(char const *file, struct __dos_stat32 *buf) { struct __kos_stat64 temp; int result = libc_dos_kstat64(file,&temp); if (!result) stat2dos32(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_dos_stat32i64(char const *file, struct __dos_stat32i64 *buf) { struct __kos_stat64 temp; int result = libc_dos_kstat64(file,&temp); if (!result) stat2dos32i64(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_dos_stat64(char const *file, struct __dos_stat64 *buf) { struct __kos_stat64 temp; int result = libc_dos_kstat64(file,&temp); if (!result) stat2dos64(&temp,buf); return result; }


/* FILESYSTEM TIME MODIFICATION. */
INTERN int LIBCCALL
libc_utimensat(fd_t dfd, char const *path, struct timespec32 const times[2], int flags) {
 struct timespec64 t64[3];
 if (!times) return libc_utimensat64(dfd,path,NULL,flags);
 t64[0].tv_sec  = times[0].tv_sec;
 t64[0].tv_nsec = times[0].tv_nsec;
 t64[1].tv_sec  = times[1].tv_sec;
 t64[1].tv_nsec = times[1].tv_nsec;
 if (flags & AT_CHANGE_CTIME) {
  t64[2].tv_sec  = times[2].tv_sec;
  t64[2].tv_nsec = times[2].tv_nsec;
 }
 return libc_utimensat64(dfd,path,t64,flags);
}
CRT_DOS_EXT int LIBCCALL
libc_dos_utimensat(fd_t dfd, char const *path,
                   struct timespec32 const times[2],
                   int flags) {
 return libc_utimensat(dfd,path,times,AT_DOSPATH|flags);
}
INTERN int LIBCCALL
libc_utimensat64(fd_t dfd, char const *path,
                 struct timespec64 const times[2], int flags) {
 return FORWARD_SYSTEM_ERROR(sys_utimensat(dfd,path,times,flags));
}
CRT_DOS_EXT int LIBCCALL
libc_dos_utimensat64(fd_t dfd, char const *path,
                     struct timespec64 const times[2], int flags) {
 return libc_utimensat64(dfd,path,times,AT_DOSPATH|flags);
}
INTERN int LIBCCALL
libc_futimens(fd_t fd, struct timespec32 const times[2]) {
 return libc_utimensat(fd,libc_empty_string,times,AT_EMPTY_PATH);
}
INTERN int LIBCCALL
libc_futimens64(fd_t fd, struct timespec64 const times[2]) {
 return libc_utimensat64(fd,libc_empty_string,times,AT_EMPTY_PATH);
}
INTERN int LIBCCALL
libc_futime(fd_t fd, struct utimbuf32 const *file_times) {
 return libc_futimeat(fd,libc_empty_string,file_times,0);
}
INTERN int LIBCCALL
libc_futime64(fd_t fd, struct utimbuf64 const *file_times) {
 return libc_futimeat64(fd,libc_empty_string,file_times,0);
}
INTERN int LIBCCALL
libc_futimes(fd_t fd, struct timeval32 const tvp[2]) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 return libc_futimens64(fd,times);
}
INTERN int LIBCCALL
libc_futimes64(fd_t fd, struct timeval64 const tvp[2]) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 return libc_futimens64(fd,times);
}
INTERN int LIBCCALL
libc_futimeat(fd_t dfd, char const *file,
              struct utimbuf32 const *file_times, int flags) {
 struct timespec64 times[2];
 if (!file_times) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  times[0].tv_sec  = file_times->actime;
  times[1].tv_sec  = file_times->modtime;
  times[0].tv_nsec = 0;
  times[1].tv_nsec = 0;
 }
 return libc_utimensat64(dfd,file,times,flags);
}
CRT_DOS_EXT int LIBCCALL
libc_dos_futimeat(fd_t dfd, char const *file,
                  struct utimbuf32 const *file_times, int flags) {
 return libc_futimeat(dfd,file,file_times,flags|AT_DOSPATH);
}
INTERN int LIBCCALL
libc_futimeat64(fd_t dfd, char const *file,
                struct utimbuf64 const *file_times, int flags) {
 struct timespec64 times[2];
 if (!file_times) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  times[0].tv_sec  = file_times->actime;
  times[1].tv_sec  = file_times->modtime;
  times[0].tv_nsec = 0;
  times[1].tv_nsec = 0;
 }
 return libc_utimensat64(dfd,file,times,flags);
}
CRT_DOS_EXT int LIBCCALL
libc_dos_futimeat64(fd_t dfd, char const *file, struct utimbuf64 const *file_times, int flags) {
 return libc_futimeat64(dfd,file,file_times,flags|AT_DOSPATH);
}
INTERN int LIBCCALL libc_utime(char const *file, struct utimbuf32 const *file_times) {
 return libc_futimeat(AT_FDCWD,file,file_times,0);
}
CRT_DOS_EXT int LIBCCALL
libc_dos_utime(char const *file, struct utimbuf32 const *file_times) {
 return libc_futimeat(AT_FDCWD,file,file_times,AT_DOSPATH);
}
INTERN int LIBCCALL libc_utime64(char const *file, struct utimbuf64 const *file_times) {
 return libc_futimeat64(AT_FDCWD,file,file_times,0);
}
CRT_DOS_EXT int LIBCCALL
libc_dos_utime64(char const *file, struct utimbuf64 const *file_times) {
 return libc_futimeat64(AT_FDCWD,file,file_times,AT_DOSPATH);
}
INTERN int LIBCCALL
libc_impl_futimesat64(fd_t dfd, char const *file,
                      struct timeval64 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 return libc_utimensat64(dfd,file,times,flags);
}
INTERN int LIBCCALL
libc_impl_futimesat(fd_t dfd, char const *file,
                    struct timeval32 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 return libc_utimensat64(dfd,file,times,flags);
}
INTERN int LIBCCALL libc_utimes(char const *file, struct timeval32 const tvp[2]) { return libc_impl_futimesat(AT_FDCWD,file,tvp,0);  }
CRT_DOS_EXT int LIBCCALL libc_dos_utimes(char const *file, struct timeval32 const tvp[2]) { return libc_impl_futimesat(AT_FDCWD,file,tvp,AT_DOSPATH);  }
INTERN int LIBCCALL libc_utimes64(char const *file, struct timeval64 const tvp[2]) { return libc_impl_futimesat64(AT_FDCWD,file,tvp,0);  }
CRT_DOS_EXT int LIBCCALL libc_dos_utimes64(char const *file, struct timeval64 const tvp[2]) { return libc_impl_futimesat64(AT_FDCWD,file,tvp,AT_DOSPATH);  }
INTERN int LIBCCALL libc_lutimes(char const *file, struct timeval32 const tvp[2]) { return libc_impl_futimesat(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW);  }
CRT_DOS_EXT int LIBCCALL libc_dos_lutimes(char const *file, struct timeval32 const tvp[2]) { return libc_impl_futimesat(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW|AT_DOSPATH);  }
INTERN int LIBCCALL libc_lutimes64(char const *file, struct timeval64 const tvp[2]) { return libc_impl_futimesat64(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW);  }
CRT_DOS_EXT int LIBCCALL libc_dos_lutimes64(char const *file, struct timeval64 const tvp[2]) { return libc_impl_futimesat64(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW|AT_DOSPATH);  }
INTERN int LIBCCALL libc_futimesat(fd_t dfd, char const *file, struct timeval32 const tvp[2]) { return libc_impl_futimesat(dfd,file,tvp,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_futimesat(fd_t dfd, char const *file, struct timeval32 const tvp[2]) { return libc_impl_futimesat(dfd,file,tvp,AT_DOSPATH); }
INTERN int LIBCCALL libc_futimesat64(fd_t dfd, char const *file, struct timeval64 const tvp[2]) { return libc_impl_futimesat64(dfd,file,tvp,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_futimesat64(fd_t dfd, char const *file, struct timeval64 const tvp[2]) { return libc_impl_futimesat64(dfd,file,tvp,AT_DOSPATH); }




INTERN int LIBCCALL libc_execv(char const *path, char *const argv[]) { return libc_execve(path,argv,environ); }
INTERN int LIBCCALL libc_execve(char const *path, char *const argv[], char *const envp[]) { return SET_SYSTEM_ERROR(sys_execve(path,argv,envp)); }
INTERN int LIBCCALL libc_execvp(char const *file, char *const argv[]) { return libc_fexecvpeat(file,argv,environ,0); }
INTERN int LIBCCALL libc_execvpe(char const *file, char *const argv[], char *const envp[]) { return libc_fexecvpeat(file,argv,envp,0); }
CRT_DOS int LIBCCALL libc_dos_execv(char const *path, char *const argv[]) { return libc_fexecveat(AT_FDCWD,path,argv,environ,AT_DOSPATH); }
CRT_DOS int LIBCCALL libc_dos_execve(char const *path, char *const argv[], char *const envp[]) { return libc_fexecveat(AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_DOS int LIBCCALL libc_dos_execvp(char const *file, char *const argv[]) { return libc_fexecvpeat(file,argv,environ,AT_DOSPATH); }
CRT_DOS int LIBCCALL libc_dos_execvpe(char const *file, char *const argv[], char *const envp[]) { return libc_fexecvpeat(file,argv,envp,AT_DOSPATH); }
INTERN int LIBCCALL libc_fexecv(int exec_fd, char *const argv[]) { return libc_fexecveat(exec_fd,libc_empty_string,argv,environ,AT_EMPTY_PATH); }
INTERN int LIBCCALL libc_fexecve(int exec_fd, char *const argv[], char *const envp[]) { return libc_fexecveat(exec_fd,libc_empty_string,argv,envp,AT_EMPTY_PATH); }
INTERN int LIBCCALL libc_fexecvat(fd_t dfd, char const *path, char *const argv[], int flags) { return libc_fexecveat(dfd,path,argv,environ,flags); }
INTERN int LIBCCALL libc_fexecveat(fd_t dfd, char const *path, char *const argv[], char *const envp[], int flags) { return SET_SYSTEM_ERROR(sys_execveat(dfd,path,argv,envp,flags)); }
INTERN int LIBCCALL libc_fexecvpat(char const *file, char *const argv[], int flags) { return libc_fexecvpeat(file,argv,environ,flags); }
CRT_DOS_EXT int LIBCCALL libc_dos_fexecvat(fd_t dfd, char const *path, char *const argv[], int flags) { return libc_fexecveat(dfd,path,argv,environ,flags|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fexecveat(fd_t dfd, char const *path, char *const argv[], char *const envp[], int flags) { return libc_fexecveat(dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fexecvpat(char const *file, char *const argv[], int flags) { return libc_fexecvpeat(file,argv,environ,flags|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fexecvpeat(char const *file, char *const argv[], char *const envp[], int flags) { return libc_fexecvpeat(file,argv,envp,flags|AT_DOSPATH); }
INTERN int LIBCCALL
libc_fexecvpeat(char const *file,
                char *const argv[],
                char *const envp[],
                int flags) {
 /* TODO: Search $PATH */
 libc_seterrno(ENOSYS);
 return -1;
}


CRT_DOS pid_t LIBCCALL libc_spawnv(int mode, char const *path, char *const argv[]) { return libc_fspawnveat(mode,AT_FDCWD,path,argv,environ,0); }
CRT_DOS pid_t LIBCCALL libc_spawnve(int mode, char const *path, char *const argv[], char *const envp[]) { return libc_fspawnveat(mode,AT_FDCWD,path,argv,envp,0); }
CRT_DOS pid_t LIBCCALL libc_spawnvp(int mode, char const *file, char *const argv[]) { return libc_fspawnvpeat(mode,file,argv,environ,0); }
CRT_DOS pid_t LIBCCALL libc_spawnvpe(int mode, char const *file, char *const argv[], char *const envp[]) { return libc_fspawnvpeat(mode,file,argv,envp,0); }
CRT_DOS pid_t LIBCCALL libc_dos_spawnv(int mode, char const *path, char *const argv[]) { return libc_fspawnveat(mode,AT_FDCWD,path,argv,environ,AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_dos_spawnve(int mode, char const *path, char *const argv[], char *const envp[]) { return libc_fspawnveat(mode,AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_dos_spawnvp(int mode, char const *file, char *const argv[]) { return libc_fspawnvpeat(mode,file,argv,environ,AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_dos_spawnvpe(int mode, char const *file, char *const argv[], char *const envp[]) { return libc_fspawnvpeat(mode,file,argv,envp,AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_fspawnv(int mode, int exec_fd, char *const argv[]) { return libc_fspawnveat(mode,exec_fd,libc_empty_string,argv,environ,AT_EMPTY_PATH); }
CRT_DOS pid_t LIBCCALL libc_fspawnve(int mode, int exec_fd, char *const argv[], char *const envp[]) { return libc_fspawnveat(mode,exec_fd,libc_empty_string,argv,envp,AT_EMPTY_PATH); }
CRT_DOS pid_t LIBCCALL libc_fspawnvat(int mode, fd_t dfd, char const *path, char *const argv[], int flags) { return libc_fspawnveat(mode,dfd,path,argv,environ,flags); }
CRT_DOS pid_t LIBCCALL libc_fspawnvpat(int mode, char const *file, char *const argv[], int flags) { return libc_fspawnvpeat(mode,file,argv,environ,flags); }
CRT_DOS pid_t LIBCCALL libc_dos_fspawnvat(int mode, fd_t dfd, char const *path, char *const argv[], int flags) { return libc_fspawnveat(mode,dfd,path,argv,environ,flags|AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_dos_fspawnveat(int mode, fd_t dfd, char const *path, char *const argv[], char *const envp[], int flags) { return libc_fspawnveat(mode,dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_dos_fspawnvpat(int mode, char const *file, char *const argv[], int flags) { return libc_fspawnvpeat(mode,file,argv,environ,flags|AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL libc_dos_fspawnvpeat(int mode, char const *file, char *const argv[], char *const envp[], int flags) { return libc_fspawnvpeat(mode,file,argv,envp,flags|AT_DOSPATH); }
CRT_DOS pid_t LIBCCALL
libc_fspawnveat(int mode, fd_t dfd,
                char const *path,
                char *const argv[],
                char *const envp[],
                int flags) {
 /* TODO */
 libc_seterrno(ENOSYS);
 return -1;
}
CRT_DOS pid_t LIBCCALL
libc_fspawnvpeat(int mode,
                 char const *file,
                 char *const argv[],
                 char *const envp[],
                 int flags) {
 /* TODO: Search $PATH */
 libc_seterrno(ENOSYS);
 return -1;
}

CRT_WIDECHAR int LIBCCALL libc_w16execv(char16_t const *path, char16_t *const argv[]) { return libc_w16fexecveat(AT_FDCWD,path,argv,libc_get_w16environ(),0); }
CRT_WIDECHAR int LIBCCALL libc_w32execv(char32_t const *path, char32_t *const argv[]) { return libc_w32fexecveat(AT_FDCWD,path,argv,libc_get_w32environ(),0); }
CRT_WIDECHAR int LIBCCALL libc_w16execve(char16_t const *path, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fexecveat(AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR int LIBCCALL libc_w32execve(char32_t const *path, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fexecveat(AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR int LIBCCALL libc_w16execvp(char16_t const *file, char16_t *const argv[]) { return libc_w16fexecvpeat(file,argv,libc_get_w16environ(),0); }
CRT_WIDECHAR int LIBCCALL libc_w32execvp(char32_t const *file, char32_t *const argv[]) { return libc_w32fexecvpeat(file,argv,libc_get_w32environ(),0); }
CRT_WIDECHAR int LIBCCALL libc_w16execvpe(char16_t const *file, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fexecvpeat(file,argv,envp,0); }
CRT_WIDECHAR int LIBCCALL libc_w32execvpe(char32_t const *file, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fexecvpeat(file,argv,envp,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16execv(char16_t const *path, char16_t *const argv[]) { return libc_w16fexecveat(AT_FDCWD,path,argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32execv(char32_t const *path, char32_t *const argv[]) { return libc_w32fexecveat(AT_FDCWD,path,argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16execve(char16_t const *path, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fexecveat(AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32execve(char32_t const *path, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fexecveat(AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16execvp(char16_t const *file, char16_t *const argv[]) { return libc_w16fexecvpeat(file,argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32execvp(char32_t const *file, char32_t *const argv[]) { return libc_w32fexecvpeat(file,argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16execvpe(char16_t const *file, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fexecvpeat(file,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32execvpe(char32_t const *file, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fexecvpeat(file,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_w16fexecv(int exec_fd, char16_t *const argv[]) { return libc_w16fexecveat(exec_fd,libc_empty_string16,argv,libc_get_w16environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR int LIBCCALL libc_w32fexecv(int exec_fd, char32_t *const argv[]) { return libc_w32fexecveat(exec_fd,libc_empty_string32,argv,libc_get_w32environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR int LIBCCALL libc_w16fexecve(int exec_fd, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fexecveat(exec_fd,libc_empty_string16,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR int LIBCCALL libc_w32fexecve(int exec_fd, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fexecveat(exec_fd,libc_empty_string32,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR int LIBCCALL libc_w16fexecvat(fd_t dfd, char16_t const *path, char16_t *const argv[], int flags) { return libc_w16fexecveat(dfd,path,argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR int LIBCCALL libc_w32fexecvat(fd_t dfd, char32_t const *path, char32_t *const argv[], int flags) { return libc_w32fexecveat(dfd,path,argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR int LIBCCALL libc_w16fexecvpat(char16_t const *file, char16_t *const argv[], int flags) { return libc_w16fexecvpeat(file,argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR int LIBCCALL libc_w32fexecvpat(char32_t const *file, char32_t *const argv[], int flags) { return libc_w32fexecvpeat(file,argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16fexecvat(fd_t dfd, char16_t const *path, char16_t *const argv[], int flags) { return libc_w16fexecveat(dfd,path,argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32fexecvat(fd_t dfd, char32_t const *path, char32_t *const argv[], int flags) { return libc_w32fexecveat(dfd,path,argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16fexecveat(fd_t dfd, char16_t const *path, char16_t *const argv[], char16_t *const envp[], int flags) { return libc_w16fexecveat(dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32fexecveat(fd_t dfd, char32_t const *path, char32_t *const argv[], char32_t *const envp[], int flags) { return libc_w32fexecveat(dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16fexecvpat(char16_t const *file, char16_t *const argv[], int flags) { return libc_w16fexecvpeat(file,argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32fexecvpat(char32_t const *file, char32_t *const argv[], int flags) { return libc_w32fexecvpeat(file,argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16fexecvpeat(char16_t const *file, char16_t *const argv[], char16_t *const envp[], int flags) { return libc_w16fexecvpeat(file,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32fexecvpeat(char32_t const *file, char32_t *const argv[], char32_t *const envp[], int flags) { return libc_w32fexecvpeat(file,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16spawnv(int mode, char16_t const *path, char16_t *const argv[]) { return libc_w16fspawnveat(mode,AT_FDCWD,path,argv,libc_get_w16environ(),0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32spawnv(int mode, char32_t const *path, char32_t *const argv[]) { return libc_w32fspawnveat(mode,AT_FDCWD,path,argv,libc_get_w32environ(),0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16spawnve(int mode, char16_t const *path, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fspawnveat(mode,AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32spawnve(int mode, char32_t const *path, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fspawnveat(mode,AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16spawnvp(int mode, char16_t const *file, char16_t *const argv[]) { return libc_w16fspawnvpeat(mode,file,argv,libc_get_w16environ(),0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32spawnvp(int mode, char32_t const *file, char32_t *const argv[]) { return libc_w32fspawnvpeat(mode,file,argv,libc_get_w32environ(),0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16spawnvpe(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fspawnvpeat(mode,file,argv,envp,0); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32spawnvpe(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fspawnvpeat(mode,file,argv,envp,0); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16spawnv(int mode, char16_t const *path, char16_t *const argv[]) { return libc_w16fspawnveat(mode,AT_FDCWD,path,argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32spawnv(int mode, char32_t const *path, char32_t *const argv[]) { return libc_w32fspawnveat(mode,AT_FDCWD,path,argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16spawnve(int mode, char16_t const *path, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fspawnveat(mode,AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32spawnve(int mode, char32_t const *path, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fspawnveat(mode,AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16spawnvp(int mode, char16_t const *file, char16_t *const argv[]) { return libc_w16fspawnvpeat(mode,file,argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32spawnvp(int mode, char32_t const *file, char32_t *const argv[]) { return libc_w32fspawnvpeat(mode,file,argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16spawnvpe(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fspawnvpeat(mode,file,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32spawnvpe(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fspawnvpeat(mode,file,argv,envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16fspawnv(int mode, int exec_fd, char16_t *const argv[]) { return libc_w16fspawnveat(mode,exec_fd,libc_empty_string16,argv,libc_get_w16environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32fspawnv(int mode, int exec_fd, char32_t *const argv[]) { return libc_w32fspawnveat(mode,exec_fd,libc_empty_string32,argv,libc_get_w32environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16fspawnve(int mode, int exec_fd, char16_t *const argv[], char16_t *const envp[]) { return libc_w16fspawnveat(mode,exec_fd,libc_empty_string16,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32fspawnve(int mode, int exec_fd, char32_t *const argv[], char32_t *const envp[]) { return libc_w32fspawnveat(mode,exec_fd,libc_empty_string32,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16fspawnvat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], int flags) { return libc_w16fspawnveat(mode,dfd,path,argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32fspawnvat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], int flags) { return libc_w32fspawnveat(mode,dfd,path,argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR pid_t LIBCCALL libc_w16fspawnvpat(int mode, char16_t const *file, char16_t *const argv[], int flags) { return libc_w16fspawnvpeat(mode,file,argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR pid_t LIBCCALL libc_w32fspawnvpat(int mode, char32_t const *file, char32_t *const argv[], int flags) { return libc_w32fspawnvpeat(mode,file,argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16fspawnvat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], int flags) { return libc_w16fspawnveat(mode,dfd,path,argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32fspawnvat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], int flags) { return libc_w32fspawnveat(mode,dfd,path,argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16fspawnveat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], char16_t *const envp[], int flags) { return libc_w16fspawnveat(mode,dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32fspawnveat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], char32_t *const envp[], int flags) { return libc_w32fspawnveat(mode,dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16fspawnvpat(int mode, char16_t const *file, char16_t *const argv[], int flags) { return libc_w16fspawnvpeat(mode,file,argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32fspawnvpat(int mode, char32_t const *file, char32_t *const argv[], int flags) { return libc_w32fspawnvpeat(mode,file,argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w16fspawnvpeat(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[], int flags) { return libc_w16fspawnvpeat(mode,file,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t LIBCCALL libc_dos_w32fspawnvpeat(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[], int flags) { return libc_w32fspawnvpeat(mode,file,argv,envp,flags|AT_DOSPATH); }



EXPORT(cwait,libc_cwait);
EXPORT(_cwait,libc_cwait);
CRT_DOS pid_t LIBCCALL
libc_cwait(int *tstat, pid_t pid, int UNUSED(action)) {
 /* This one's pretty simple, because it's literally just a waitpid() system call...
  * (It even returns the same thing, that being the PID of the joined process...) */
 return libc_waitpid(pid,tstat,WEXITED);
 /* NOTE: Apparently, the `action' argument is completely ignored... */
}



#if defined(__i386__) && !defined(__x86_64__)
#define CAPTURE_ARGV() \
 char **argv = (char **)&args
#define CAPTURE_ARGV_FLAGS() \
 char **argv = (char **)&args; \
 char **_temp = argv; int flags; \
 while (*_temp++); \
 flags = *(int *)_temp
#define CAPTURE_ARGV_ENVP() \
 char **argv = (char **)&args; \
 char **envp = argv; \
 while (*envp++); \
 envp = *(char ***)envp
#define CAPTURE_ARGV_ENVP_FLAGS() \
 char **argv = (char **)&args; \
 char **envp = argv; int flags; \
 while (*envp++); \
 envp = *(char ***)envp; \
 flags = *(int *)((char ***)envp+1)
#else
#define CAPTURE_ARGV() \
 char **argv; \
 va_list va; size_t argc = 1; \
 va_start(va,args); \
 while (va_arg(va,char *)) ++argc; \
 va_end(va); \
 argv = (char **)alloca(argc*sizeof(char *)); \
 va_start(va,args); argc = 0; \
 while ((argv[argc++] = va_arg(va,char *)) != NULL); \
 va_end(va)
#define CAPTURE_ARGV_FLAGS() \
 char **argv; int flags; \
 va_list va; size_t argc = 1; \
 va_start(va,args); \
 while (va_arg(va,char *)) ++argc; \
 va_end(va); \
 argv = (char **)alloca(argc*sizeof(char *)); \
 va_start(va,args); argc = 0; \
 while ((argv[argc++] = va_arg(va,char *)) != NULL); \
 flags = va_arg(va,int); \
 va_end(va)
#define CAPTURE_ARGV_ENVP() \
 char **argv,**envp; \
 va_list va; size_t argc = 1; \
 va_start(va,args); \
 while (va_arg(va,char *)) ++argc; \
 va_end(va); \
 argv = (char **)alloca(argc*sizeof(char *)); \
 va_start(va,args); argc = 0; \
 while ((argv[argc++] = va_arg(va,char *)) != NULL); \
 envp = va_arg(va,char **); \
 va_end(va)
#define CAPTURE_ARGV_ENVP_FLAGS() \
 char **argv,**envp; int flags; \
 va_list va; size_t argc = 1; \
 va_start(va,args); \
 while (va_arg(va,char *)) ++argc; \
 va_end(va); \
 argv = (char **)alloca(argc*sizeof(char *)); \
 va_start(va,args); argc = 0; \
 while ((argv[argc++] = va_arg(va,char *)) != NULL); \
 envp = va_arg(va,char **); \
 flags = va_arg(va,int); \
 va_end(va)
#endif

INTERN int ATTR_CDECL libc_execl(char const *path, char const *args, ...) { CAPTURE_ARGV(); return libc_execve(path,argv,environ); }
INTERN int ATTR_CDECL libc_execle(char const *path, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_execve(path,argv,envp); }
INTERN int ATTR_CDECL libc_execlp(char const *file, char const *args, ...) { CAPTURE_ARGV(); return libc_execvpe(file,argv,environ); }
INTERN int ATTR_CDECL libc_execlpe(char const *file, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_execvpe(file,argv,envp); }
CRT_DOS int ATTR_CDECL libc_dos_execl(char const *path, char const *args, ...) { CAPTURE_ARGV(); return libc_fexecveat(AT_FDCWD,path,argv,environ,AT_DOSPATH); }
CRT_DOS int ATTR_CDECL libc_dos_execle(char const *path, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_fexecveat(AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_DOS int ATTR_CDECL libc_dos_execlp(char const *file, char const *args, ...) { CAPTURE_ARGV(); return libc_fexecvpeat(file,argv,environ,AT_DOSPATH); }
CRT_DOS int ATTR_CDECL libc_dos_execlpe(char const *file, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_fexecvpeat(file,argv,envp,AT_DOSPATH); }
INTERN int ATTR_CDECL libc_fexecl(int exec_fd, char const *args, ...) { CAPTURE_ARGV(); return libc_fexecve(exec_fd,argv,environ); }
INTERN int ATTR_CDECL libc_fexecle(int exec_fd, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_fexecve(exec_fd,argv,envp); }
INTERN int ATTR_CDECL libc_fexeclat(fd_t dfd, char const *path, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fexecveat(dfd,path,argv,environ,flags); }
INTERN int ATTR_CDECL libc_fexecleat(fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fexecveat(dfd,path,argv,envp,flags); }
INTERN int ATTR_CDECL libc_fexeclpat(char const *file, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fexecvpeat(file,argv,environ,flags); }
INTERN int ATTR_CDECL libc_fexeclpeat(char const *file, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fexecvpeat(file,argv,envp,flags); }
CRT_DOS int ATTR_CDECL libc_dos_fexeclat(fd_t dfd, char const *path, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fexecveat(dfd,path,argv,environ,flags|AT_DOSPATH); }
CRT_DOS int ATTR_CDECL libc_dos_fexecleat(fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fexecveat(dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_DOS int ATTR_CDECL libc_dos_fexeclpat(char const *file, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fexecvpeat(file,argv,environ,flags|AT_DOSPATH); }
CRT_DOS int ATTR_CDECL libc_dos_fexeclpeat(char const *file, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fexecvpeat(file,argv,envp,flags|AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_spawnl(int mode, char const *path, char const *args, ...) { CAPTURE_ARGV(); return libc_spawnve(mode,path,argv,environ); }
CRT_DOS pid_t ATTR_CDECL libc_spawnle(int mode, char const *path, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_spawnve(mode,path,argv,envp); }
CRT_DOS pid_t ATTR_CDECL libc_spawnlp(int mode, char const *file, char const *args, ...) { CAPTURE_ARGV(); return libc_spawnvpe(mode,file,argv,environ); }
CRT_DOS pid_t ATTR_CDECL libc_spawnlpe(int mode, char const *file, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_spawnvpe(mode,file,argv,envp); }
CRT_DOS pid_t ATTR_CDECL libc_dos_spawnl(int mode, char const *path, char const *args, ...) { CAPTURE_ARGV(); return libc_fspawnveat(mode,AT_FDCWD,path,argv,environ,AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_dos_spawnle(int mode, char const *path, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_fspawnveat(mode,AT_FDCWD,path,argv,envp,AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_dos_spawnlp(int mode, char const *file, char const *args, ...) { CAPTURE_ARGV(); return libc_fspawnvpeat(mode,file,argv,environ,AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_dos_spawnlpe(int mode, char const *file, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_fspawnvpeat(mode,file,argv,envp,AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_fspawnl(int mode, int exec_fd, char const *args, ...) { CAPTURE_ARGV(); return libc_fspawnve(mode,exec_fd,argv,environ); }
CRT_DOS pid_t ATTR_CDECL libc_fspawnle(int mode, int exec_fd, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_fspawnve(mode,exec_fd,argv,envp); }
CRT_DOS pid_t ATTR_CDECL libc_fspawnlat(int mode, fd_t dfd, char const *path, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fspawnveat(mode,dfd,path,argv,environ,flags); }
CRT_DOS pid_t ATTR_CDECL libc_fspawnleat(int mode, fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fspawnveat(mode,dfd,path,argv,envp,flags); }
CRT_DOS pid_t ATTR_CDECL libc_fspawnlpat(int mode, char const *file, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fspawnvpeat(mode,file,argv,environ,flags); }
CRT_DOS pid_t ATTR_CDECL libc_fspawnlpeat(int mode, char const *file, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fspawnvpeat(mode,file,argv,envp,flags); }
CRT_DOS pid_t ATTR_CDECL libc_dos_fspawnlat(int mode, fd_t dfd, char const *path, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fspawnveat(mode,dfd,path,argv,environ,flags|AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_dos_fspawnleat(int mode, fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fspawnveat(mode,dfd,path,argv,envp,flags|AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_dos_fspawnlpat(int mode, char const *file, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_fspawnvpeat(mode,file,argv,environ,flags|AT_DOSPATH); }
CRT_DOS pid_t ATTR_CDECL libc_dos_fspawnlpeat(int mode, char const *file, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_fspawnvpeat(mode,file,argv,envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_w16execl(char16_t const *path, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16execve(path,(char16_t **)argv,libc_get_w16environ()); }
CRT_WIDECHAR int ATTR_CDECL libc_w32execl(char32_t const *path, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32execve(path,(char32_t **)argv,libc_get_w32environ()); }
CRT_WIDECHAR int ATTR_CDECL libc_w16execle(char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16execve(path,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR int ATTR_CDECL libc_w32execle(char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32execve(path,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR int ATTR_CDECL libc_w16execlp(char16_t const *file, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16execvpe(file,(char16_t **)argv,libc_get_w16environ()); }
CRT_WIDECHAR int ATTR_CDECL libc_w32execlp(char32_t const *file, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32execvpe(file,(char32_t **)argv,libc_get_w32environ()); }
CRT_WIDECHAR int ATTR_CDECL libc_w16execlpe(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16execvpe(file,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR int ATTR_CDECL libc_w32execlpe(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32execvpe(file,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16execl(char16_t const *path, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16fexecveat(AT_FDCWD,path,(char16_t **)argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32execl(char32_t const *path, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32fexecveat(AT_FDCWD,path,(char32_t **)argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16execle(char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16fexecveat(AT_FDCWD,path,(char16_t **)argv,(char16_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32execle(char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32fexecveat(AT_FDCWD,path,(char32_t **)argv,(char32_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16execlp(char16_t const *file, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16fexecvpeat(file,(char16_t **)argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32execlp(char32_t const *file, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32fexecvpeat(file,(char32_t **)argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16execlpe(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16fexecvpeat(file,(char16_t **)argv,(char16_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32execlpe(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32fexecvpeat(file,(char32_t **)argv,(char32_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_w16fexecl(int exec_fd, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16fexecve(exec_fd,(char16_t **)argv,libc_get_w16environ()); }
CRT_WIDECHAR int ATTR_CDECL libc_w32fexecl(int exec_fd, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32fexecve(exec_fd,(char32_t **)argv,libc_get_w32environ()); }
CRT_WIDECHAR int ATTR_CDECL libc_w16fexecle(int exec_fd, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16fexecve(exec_fd,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR int ATTR_CDECL libc_w32fexecle(int exec_fd, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32fexecve(exec_fd,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR int ATTR_CDECL libc_w16fexeclat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fexecveat(dfd,path,(char16_t **)argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w32fexeclat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fexecveat(dfd,path,(char32_t **)argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w16fexecleat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fexecveat(dfd,path,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w32fexecleat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fexecveat(dfd,path,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w16fexeclpat(char16_t const *file, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fexecvpeat(file,(char16_t **)argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w32fexeclpat(char32_t const *file, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fexecvpeat(file,(char32_t **)argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w16fexeclpeat(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fexecvpeat(file,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR int ATTR_CDECL libc_w32fexeclpeat(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fexecvpeat(file,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16fexeclat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fexecveat(dfd,path,(char16_t **)argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32fexeclat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fexecveat(dfd,path,(char32_t **)argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16fexecleat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fexecveat(dfd,path,(char16_t **)argv,(char16_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32fexecleat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fexecveat(dfd,path,(char32_t **)argv,(char32_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16fexeclpat(char16_t const *file, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fexecvpeat(file,(char16_t **)argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32fexeclpat(char32_t const *file, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fexecvpeat(file,(char32_t **)argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w16fexeclpeat(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fexecvpeat(file,(char16_t **)argv,(char16_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR int ATTR_CDECL libc_dos_w32fexeclpeat(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fexecvpeat(file,(char32_t **)argv,(char32_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16spawnl(int mode, char16_t const *path, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16spawnve(mode,path,(char16_t **)argv,libc_get_w16environ()); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32spawnl(int mode, char32_t const *path, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32spawnve(mode,path,(char32_t **)argv,libc_get_w32environ()); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16spawnle(int mode, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16spawnve(mode,path,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32spawnle(int mode, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32spawnve(mode,path,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16spawnlp(int mode, char16_t const *file, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16spawnvpe(mode,file,(char16_t **)argv,libc_get_w16environ()); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32spawnlp(int mode, char32_t const *file, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32spawnvpe(mode,file,(char32_t **)argv,libc_get_w32environ()); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16spawnlpe(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16spawnvpe(mode,file,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32spawnlpe(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32spawnvpe(mode,file,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16spawnl(int mode, char16_t const *path, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16fspawnveat(mode,AT_FDCWD,path,(char16_t **)argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32spawnl(int mode, char32_t const *path, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32fspawnveat(mode,AT_FDCWD,path,(char32_t **)argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16spawnle(int mode, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16fspawnveat(mode,AT_FDCWD,path,(char16_t **)argv,(char16_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32spawnle(int mode, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32fspawnveat(mode,AT_FDCWD,path,(char32_t **)argv,(char32_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16spawnlp(int mode, char16_t const *file, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16fspawnvpeat(mode,file,(char16_t **)argv,libc_get_w16environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32spawnlp(int mode, char32_t const *file, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32fspawnvpeat(mode,file,(char32_t **)argv,libc_get_w32environ(),AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16spawnlpe(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16fspawnvpeat(mode,file,(char16_t **)argv,(char16_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32spawnlpe(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32fspawnvpeat(mode,file,(char32_t **)argv,(char32_t **)envp,AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16fspawnl(int mode, int exec_fd, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_w16fspawnve(mode,exec_fd,(char16_t **)argv,libc_get_w16environ()); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32fspawnl(int mode, int exec_fd, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_w32fspawnve(mode,exec_fd,(char32_t **)argv,libc_get_w32environ()); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16fspawnle(int mode, int exec_fd, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w16fspawnve(mode,exec_fd,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32fspawnle(int mode, int exec_fd, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_w32fspawnve(mode,exec_fd,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16fspawnlat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fspawnveat(mode,dfd,path,(char16_t **)argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32fspawnlat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fspawnveat(mode,dfd,path,(char32_t **)argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16fspawnleat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fspawnveat(mode,dfd,path,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32fspawnleat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fspawnveat(mode,dfd,path,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16fspawnlpat(int mode, char16_t const *file, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fspawnvpeat(mode,file,(char16_t **)argv,libc_get_w16environ(),flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32fspawnlpat(int mode, char32_t const *file, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fspawnvpeat(mode,file,(char32_t **)argv,libc_get_w32environ(),flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w16fspawnlpeat(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fspawnvpeat(mode,file,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_w32fspawnlpeat(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fspawnvpeat(mode,file,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16fspawnlat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fspawnveat(mode,dfd,path,(char16_t **)argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32fspawnlat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fspawnveat(mode,dfd,path,(char32_t **)argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16fspawnleat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fspawnveat(mode,dfd,path,(char16_t **)argv,(char16_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32fspawnleat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fspawnveat(mode,dfd,path,(char32_t **)argv,(char32_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16fspawnlpat(int mode, char16_t const *file, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w16fspawnvpeat(mode,file,(char16_t **)argv,libc_get_w16environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32fspawnlpat(int mode, char32_t const *file, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_w32fspawnvpeat(mode,file,(char32_t **)argv,libc_get_w32environ(),flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w16fspawnlpeat(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w16fspawnvpeat(mode,file,(char16_t **)argv,(char16_t **)envp,flags|AT_DOSPATH); }
CRT_WIDECHAR pid_t ATTR_CDECL libc_dos_w32fspawnlpeat(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_w32fspawnvpeat(mode,file,(char32_t **)argv,(char32_t **)envp,flags|AT_DOSPATH); }




































/* Export symbols (STAT). */
EXPORT(__KSYM(open),               libc_open);
EXPORT(__DSYM(_open),              libc_dos_open);
EXPORT("DOS$?_open%%YAHPEBDHH%Z",  libc_dos_open);
EXPORT(__KSYM(open64),             libc_open);
EXPORT(__DSYM(open64),             libc_dos_open);
EXPORT(__KSYM(openat),             libc_openat);
EXPORT(__DSYM(openat),             libc_dos_openat);
EXPORT(__KSYM(openat64),           libc_openat);
EXPORT(__DSYM(openat64),           libc_dos_openat);
EXPORT(__KSYM(creat),              libc_creat);
EXPORT(__DSYM(_creat),             libc_dos_creat);
EXPORT(__KSYM(creat64),            libc_creat);
EXPORT(__DSYM(creat64),            libc_dos_creat);
EXPORT(fcntl,                      libc_fcntl);
EXPORT(ioctl,                      libc_ioctl);
EXPORT(xioctlf,                    libc_xioctlf);
EXPORT(posix_fadvise,              libc_posix_fadvise);
EXPORT(posix_fadvise64,            libc_posix_fadvise64);
EXPORT(posix_fallocate,            libc_posix_fallocate);
EXPORT(posix_fallocate64,          libc_posix_fallocate64);
EXPORT(lockf,                      libc_lockf);
EXPORT(lockf64,                    libc_lockf64);
EXPORT(fallocate,                  libc_fallocate);
EXPORT(fallocate64,                libc_fallocate64);
EXPORT(readahead,                  libc_readahead);
EXPORT(sync_file_range,            libc_sync_file_range);
EXPORT(vmsplice,                   libc_vmsplice);
EXPORT(splice,                     libc_splice);
EXPORT(tee,                        libc_tee);
EXPORT(name_to_handle_at,          libc_name_to_handle_at);
EXPORT(open_by_handle_at,          libc_open_by_handle_at);
EXPORT(pipe,                       libc_pipe);
EXPORT(pipe2,                      libc_pipe2);
EXPORT(getumask,                   libc_getumask);
EXPORT(fsync,                      libc_fsync);
EXPORT(fdatasync,                  libc_fdatasync);
EXPORT(getuid,                     libc_getuid);
EXPORT(geteuid,                    libc_geteuid);
EXPORT(getgid,                     libc_getgid);
EXPORT(getegid,                    libc_getegid);
EXPORT(getgroups,                  libc_getgroups);
EXPORT(setuid,                     libc_setuid);
EXPORT(setgid,                     libc_setgid);
EXPORT(dup,                        libc_dup);
EXPORT(dup2,                       libc_dup2);
EXPORT(dup3,                       libc_dup3);
EXPORT(close,                      libc_close);
EXPORT(__KSYM(chdir),              libc_chdir);
EXPORT(__DSYM(chdir),              libc_dos_chdir);
EXPORT(__KSYM(fchdirat),           libc_fchdirat);
EXPORT(__DSYM(fchdirat),           libc_dos_fchdirat);
EXPORT(fchdir,                     libc_fchdir);
EXPORT(__KSYM(getcwd),             libc_getcwd);
EXPORT(__DSYM(_getcwd),            libc_dos_getcwd);
EXPORT(__KSYM(getwd),              libc_getwd);
EXPORT(__DSYM(getwd),              libc_dos_getwd);
EXPORT(__KSYM(get_current_dir_name),libc_get_current_dir_name);
EXPORT(__DSYM(get_current_dir_name),libc_dos_get_current_dir_name);
EXPORT(__KSYM(fpathconfat),        libc_fpathconfat);
EXPORT(__DSYM(fpathconfat),        libc_dos_fpathconfat);
EXPORT(__KSYM(pathconf),           libc_pathconf);
EXPORT(__DSYM(pathconf),           libc_dos_pathconf);
EXPORT(fpathconf,                  libc_fpathconf);
EXPORT(__KSYM(xfdname),            libc_xfdname);
EXPORT(__DSYM(xfdname),            libc_dos_xfdname);
EXPORT(__KSYM(xfrealpath),         libc_xfrealpath);
EXPORT(__DSYM(xfrealpath),         libc_dos_xfrealpath);
EXPORT(__KSYM(xrealpath),          libc_xrealpath);
EXPORT(__DSYM(xrealpath),          libc_dos_xrealpath);
EXPORT(__KSYM(xfrealpathat),       libc_xfrealpathat);
EXPORT(__DSYM(xfrealpathat),       libc_dos_xfrealpathat);
EXPORT(__KSYM(xfdname2),           libc_xfdname2);
EXPORT(__DSYM(xfdname2),           libc_dos_xfdname2);
EXPORT(__KSYM(xfrealpath2),        libc_xfrealpath2);
EXPORT(__DSYM(xfrealpath2),        libc_dos_xfrealpath2);
EXPORT(__KSYM(xrealpath2),         libc_xrealpath2);
EXPORT(__DSYM(xrealpath2),         libc_dos_xrealpath2);
EXPORT(__KSYM(xfrealpathat2),      libc_xfrealpathat2);
EXPORT(__DSYM(xfrealpathat2),      libc_dos_xfrealpathat2);
EXPORT(__KSYM(realpath),           libc_realpath);
EXPORT(__DSYM(realpath),           libc_dos_realpath);
EXPORT(__KSYM(frealpath),          libc_frealpath);
EXPORT(__DSYM(frealpath),          libc_dos_frealpath);
EXPORT(__KSYM(canonicalize_file_name),libc_canonicalize_file_name);
EXPORT(__DSYM(canonicalize_file_name),libc_dos_canonicalize_file_name);
EXPORT(fchown,                     libc_fchown);
EXPORT(__KSYM(chown),              libc_chown);
EXPORT(__DSYM(chown),              libc_dos_chown);
EXPORT(__KSYM(lchown),             libc_lchown);
EXPORT(__DSYM(lchown),             libc_dos_lchown);
EXPORT(__KSYM(fchownat),           libc_fchownat);
EXPORT(__DSYM(fchownat),           libc_dos_fchownat);
EXPORT(fchmod,                     libc_fchmod);
EXPORT(__KSYM(chmod),              libc_chmod);
EXPORT(__DSYM(chmod),              libc_dos_chmod);
EXPORT(__KSYM(lchmod),             libc_lchmod);
EXPORT(__DSYM(lchmod),             libc_dos_lchmod);
EXPORT(__KSYM(fchmodat),           libc_fchmodat);
EXPORT(__DSYM(fchmodat),           libc_dos_fchmodat);
EXPORT(__KSYM(link),               libc_link);
EXPORT(__DSYM(link),               libc_dos_link);
EXPORT(__KSYM(linkat),             libc_linkat);
EXPORT(__DSYM(linkat),             libc_dos_linkat);
EXPORT(__KSYM(unlink),             libc_unlink);
EXPORT(__DSYM(unlink),             libc_dos_unlink);
EXPORT(__KSYM(unlinkat),           libc_unlinkat);
EXPORT(__DSYM(unlinkat),           libc_dos_unlinkat);
EXPORT(__KSYM(rmdir),              libc_rmdir);
EXPORT(__DSYM(rmdir),              libc_dos_rmdir);
EXPORT(__KSYM(remove),             libc_remove);
EXPORT(__DSYM(remove),             libc_dos_remove);
EXPORT(__KSYM(removeat),           libc_removeat);
EXPORT(__DSYM(removeat),           libc_dos_removeat);
EXPORT(__KSYM(rename),             libc_rename);
EXPORT(__DSYM(rename),             libc_dos_rename);
EXPORT(__KSYM(renameat),           libc_renameat);
EXPORT(__DSYM(renameat),           libc_dos_renameat);
EXPORT(__KSYM(frenameat),          libc_frenameat);
EXPORT(__DSYM(frenameat),          libc_dos_frenameat);
EXPORT(__KSYM(mkdir),              libc_mkdir);
EXPORT(__DSYM(mkdir),              libc_dos_mkdir);
EXPORT(__KSYM(mkdirat),            libc_mkdirat);
EXPORT(__DSYM(mkdirat),            libc_dos_mkdirat);
EXPORT(__KSYM(fmkdirat),           libc_fmkdirat);
EXPORT(__DSYM(fmkdirat),           libc_dos_fmkdirat);
EXPORT(__KSYM(symlink),            libc_symlink);
EXPORT(__DSYM(symlink),            libc_dos_symlink);
EXPORT(__KSYM(symlinkat),          libc_symlinkat);
EXPORT(__DSYM(symlinkat),          libc_dos_symlinkat);
EXPORT(__KSYM(fsymlinkat),         libc_fsymlinkat);
EXPORT(__DSYM(fsymlinkat),         libc_dos_fsymlinkat);
EXPORT(__KSYM(mknod),              libc_mknod);
EXPORT(__DSYM(mknod),              libc_dos_mknod);
EXPORT(__KSYM(mknodat),            libc_mknodat);
EXPORT(__DSYM(mknodat),            libc_dos_mknodat);
EXPORT(__KSYM(fmknodat),           libc_fmknodat);
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
EXPORT(__KSYM(readlinkat),         libc_readlinkat);
EXPORT(__DSYM(readlinkat),         libc_dos_readlinkat);
EXPORT(__KSYM(freadlinkat),        libc_freadlinkat);
EXPORT(__DSYM(freadlinkat),        libc_dos_freadlinkat);
EXPORT(__KSYM(access),             libc_access);
EXPORT(__DSYM(access),             libc_dos_access);
EXPORT(__KSYM(eaccess),            libc_eaccess);
EXPORT(__DSYM(eaccess),            libc_dos_eaccess);
EXPORT(__KSYM(faccessat),          libc_faccessat);
EXPORT(__DSYM(faccessat),          libc_dos_faccessat);
EXPORT(ftruncate,                  libc_ftruncate);
EXPORT(__KSYM(truncate),           libc_truncate);
EXPORT(__DSYM(truncate),           libc_dos_truncate);
EXPORT(__KSYM(ftruncateat),        libc_ftruncateat);
EXPORT(__DSYM(ftruncateat),        libc_dos_ftruncateat);
EXPORT(ftruncate64,                libc_ftruncate64);
EXPORT(__KSYM(truncate64),         libc_truncate64);
EXPORT(__DSYM(truncate64),         libc_dos_truncate64);
EXPORT(__KSYM(ftruncateat64),      libc_ftruncateat64);
EXPORT(__DSYM(ftruncateat64),      libc_dos_ftruncateat64);
EXPORT(__KSYM(chroot),             libc_chroot);
EXPORT(__DSYM(chroot),             libc_dos_chroot);
EXPORT(lseek64,                    libc_lseek64);
EXPORT(lseek,                      libc_lseek);
EXPORT(read,                       libc_read);
EXPORT(write,                      libc_write);
EXPORT(pread,                      libc_pread);
EXPORT(pread64,                    libc_pread64);
EXPORT(pwrite,                     libc_pwrite);
EXPORT(pwrite64,                   libc_pwrite64);
EXPORT(xreadf,                     libc_xreadf);
EXPORT(xwritef,                    libc_xwritef);
EXPORT(xpreadf,                    libc_xpreadf);
EXPORT(xpreadf64,                  libc_xpreadf64);
EXPORT(xpwritef,                   libc_xpwritef);
EXPORT(xpwritef64,                 libc_xpwritef64);
EXPORT(poll,                       libc_poll);
EXPORT(ppoll,                      libc_ppoll);
EXPORT(ppoll64,                    libc_ppoll64);
EXPORT(select,                     libc_select);
EXPORT(select64,                   libc_select64);
EXPORT(pselect,                    libc_pselect);
EXPORT(pselect64,                  libc_pselect64);
EXPORT(pause,                      libc_pause);
EXPORT(syncfs,                     libc_syncfs);
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
EXPORT(sysconf,                    libc_sysconf);

/* FILESYSTEM TIME MODIFICATION. */
EXPORT(__KSYM(utimensat),          libc_utimensat);
EXPORT(__DSYM(utimensat),          libc_dos_utimensat);
EXPORT(__KSYM(utimensat64),        libc_utimensat64);
EXPORT(__DSYM(utimensat64),        libc_dos_utimensat64);
EXPORT(futimens,                   libc_futimens);
EXPORT(futimens64,                 libc_futimens64);
EXPORT(futime,                     libc_futime);
EXPORT(futime64,                   libc_futime64);
EXPORT(futimens,                   libc_futimens);
EXPORT(futimens64,                 libc_futimens64);
EXPORT(futimes,                    libc_futimes);
EXPORT(futimes64,                  libc_futimes64);
EXPORT(__KSYM(utimensat),          libc_utimensat);
EXPORT(__DSYM(utimensat),          libc_dos_utimensat);
EXPORT(__KSYM(utimensat64),        libc_utimensat64);
EXPORT(__DSYM(utimensat64),        libc_dos_utimensat64);
EXPORT(__KSYM(futimeat),           libc_futimeat);
EXPORT(__DSYM(futimeat),           libc_dos_futimeat);
EXPORT(__KSYM(futimeat64),         libc_futimeat64);
EXPORT(__DSYM(futimeat64),         libc_dos_futimeat64);
EXPORT(__KSYM(utime),              libc_utime);
EXPORT(__DSYM(utime),              libc_dos_utime);
EXPORT(__KSYM(utime64),            libc_utime64);
EXPORT(__DSYM(utime64),            libc_dos_utime64);
EXPORT(__KSYM(utimes),             libc_utimes);
EXPORT(__DSYM(utimes),             libc_dos_utimes);
EXPORT(__KSYM(utimes64),           libc_utimes64);
EXPORT(__DSYM(utimes64),           libc_dos_utimes64);
EXPORT(__KSYM(lutimes),            libc_lutimes);
EXPORT(__DSYM(lutimes),            libc_dos_lutimes);
EXPORT(__KSYM(lutimes64),          libc_lutimes64);
EXPORT(__DSYM(lutimes64),          libc_dos_lutimes64);
EXPORT(__KSYM(futimesat),          libc_futimesat);
EXPORT(__DSYM(futimesat),          libc_dos_futimesat);
EXPORT(__KSYM(futimesat64),        libc_futimesat64);
EXPORT(__DSYM(futimesat64),        libc_dos_futimesat64);

/* DOS aliases */
EXPORT(_futime,                    libc_futime); /* This is not an error. - DOS defines this name, too. */
EXPORT(_futime32,                  libc_futime);
EXPORT(_futime64,                  libc_futime64);
EXPORT(__KSYM(_utime),             libc_utime); /* This is not an error. - DOS defines this name, too. */
EXPORT(__DSYM(_utime),             libc_dos_utime); /* This is not an error. - DOS defines this name, too. */
EXPORT(__KSYM(_utime32),           libc_utime);
EXPORT(__DSYM(_utime32),           libc_dos_utime);
EXPORT(__KSYM(_utime64),           libc_utime64);
EXPORT(__DSYM(_utime64),           libc_dos_utime64);

/* Exec functions */
EXPORT(fexecv,                     libc_fexecv);
EXPORT(fexecve,                    libc_fexecve);
EXPORT(__KSYM(execv),              libc_execv);
EXPORT(__DSYM(_execv),             libc_dos_execv);
EXPORT(__KSYM(execve),             libc_execve);
EXPORT(__DSYM(_execve),            libc_dos_execve);
EXPORT(__KSYM(execvp),             libc_execvp);
EXPORT(__DSYM(_execvp),            libc_dos_execvp);
EXPORT(__KSYM(execvpe),            libc_execvpe);
EXPORT(__DSYM(_execvpe),           libc_dos_execvpe);
EXPORT(__KSYM(fexecvat),           libc_fexecvat);
EXPORT(__DSYM(fexecvat),           libc_dos_fexecvat);
EXPORT(__KSYM(fexecveat),          libc_fexecveat);
EXPORT(__DSYM(fexecveat),          libc_dos_fexecveat);
EXPORT(__KSYM(fexecvpat),          libc_fexecvpat);
EXPORT(__DSYM(fexecvpat),          libc_dos_fexecvpat);
EXPORT(__KSYM(fexecvpeat),         libc_fexecvpeat);
EXPORT(__DSYM(fexecvpeat),         libc_dos_fexecvpeat);
EXPORT(__KSYM(execl),              libc_execl);
EXPORT(__DSYM(_execl),             libc_dos_execl);
EXPORT(__KSYM(execle),             libc_execle);
EXPORT(__DSYM(_execle),            libc_dos_execle);
EXPORT(__KSYM(execlp),             libc_execlp);
EXPORT(__DSYM(_execlp),            libc_dos_execlp);
EXPORT(__KSYM(execlpe),            libc_execlpe);
EXPORT(__DSYM(_execlpe),           libc_dos_execlpe);
EXPORT(fexecl,                     libc_fexecl);
EXPORT(fexecle,                    libc_fexecle);
EXPORT(__KSYM(fexeclat),           libc_fexeclat);
EXPORT(__DSYM(fexeclat),           libc_dos_fexeclat);
EXPORT(__KSYM(fexecleat),          libc_fexecleat);
EXPORT(__DSYM(fexecleat),          libc_dos_fexecleat);
EXPORT(__KSYM(fexeclpat),          libc_fexeclpat);
EXPORT(__DSYM(fexeclpat),          libc_dos_fexeclpat);
EXPORT(__KSYM(fexeclpeat),         libc_fexeclpeat);
EXPORT(__DSYM(fexeclpeat),         libc_dos_fexeclpeat);

/* Spawn functions */
EXPORT(fspawnv,                    libc_fspawnv);
EXPORT(fspawnve,                   libc_fspawnve);
EXPORT(__KSYM(spawnv),             libc_spawnv);
EXPORT(__DSYM(_spawnv),            libc_dos_spawnv);
EXPORT(__KSYM(spawnve),            libc_spawnve);
EXPORT(__DSYM(_spawnve),           libc_dos_spawnve);
EXPORT(__KSYM(spawnvp),            libc_spawnvp);
EXPORT(__DSYM(_spawnvp),           libc_dos_spawnvp);
EXPORT(__KSYM(spawnvpe),           libc_spawnvpe);
EXPORT(__DSYM(_spawnvpe),          libc_dos_spawnvpe);
EXPORT(__KSYM(fspawnvat),          libc_fspawnvat);
EXPORT(__DSYM(fspawnvat),          libc_dos_fspawnvat);
EXPORT(__KSYM(fspawnveat),         libc_fspawnveat);
EXPORT(__DSYM(fspawnveat),         libc_dos_fspawnveat);
EXPORT(__KSYM(fspawnvpat),         libc_fspawnvpat);
EXPORT(__DSYM(fspawnvpat),         libc_dos_fspawnvpat);
EXPORT(__KSYM(fspawnvpeat),        libc_fspawnvpeat);
EXPORT(__DSYM(fspawnvpeat),        libc_dos_fspawnvpeat);
EXPORT(__KSYM(spawnl),             libc_spawnl);
EXPORT(__DSYM(_spawnl),            libc_dos_spawnl);
EXPORT(__KSYM(spawnle),            libc_spawnle);
EXPORT(__DSYM(_spawnle),           libc_dos_spawnle);
EXPORT(__KSYM(spawnlp),            libc_spawnlp);
EXPORT(__DSYM(_spawnlp),           libc_dos_spawnlp);
EXPORT(__KSYM(spawnlpe),           libc_spawnlpe);
EXPORT(__DSYM(_spawnlpe),          libc_dos_spawnlpe);
EXPORT(fspawnl,                    libc_fspawnl);
EXPORT(fspawnle,                   libc_fspawnle);
EXPORT(__KSYM(fspawnlat),          libc_fspawnlat);
EXPORT(__DSYM(fspawnlat),          libc_dos_fspawnlat);
EXPORT(__KSYM(fspawnleat),         libc_fspawnleat);
EXPORT(__DSYM(fspawnleat),         libc_dos_fspawnleat);
EXPORT(__KSYM(fspawnlpat),         libc_fspawnlpat);
EXPORT(__DSYM(fspawnlpat),         libc_dos_fspawnlpat);
EXPORT(__KSYM(fspawnlpeat),        libc_fspawnlpeat);
EXPORT(__DSYM(fspawnlpeat),        libc_dos_fspawnlpeat);


/* Export symbols (STAT). */
EXPORT(kfstat,                     libc_kfstat);
EXPORT(kfstat64,                   libc_kfstat64);
EXPORT(__KSYM(kstat),              libc_kstat);
EXPORT(__DSYM(kstat),              libc_dos_kstat);
EXPORT(__KSYM(kstat64),            libc_kstat64);
EXPORT(__DSYM(kstat64),            libc_dos_kstat64);
EXPORT(__KSYM(kfstatat),           libc_kfstatat);
EXPORT(__DSYM(kfstatat),           libc_dos_kfstatat);
EXPORT(__KSYM(kfstatat64),         libc_kfstatat64);
EXPORT(__DSYM(kfstatat64),         libc_dos_kfstatat64);
EXPORT(__KSYM(klstat),             libc_klstat);
EXPORT(__DSYM(klstat),             libc_dos_klstat);
EXPORT(__KSYM(klstat64),           libc_klstat64);
EXPORT(__DSYM(klstat64),           libc_dos_klstat64);

EXPORT(fstat,                      libc_fstat);
EXPORT(fstat64,                    libc_fstat64);
EXPORT(__KSYM(stat),               libc_stat);
EXPORT(__DSYM(stat),               libc_dos_stat);
EXPORT(__KSYM(stat64),             libc_stat64);
EXPORT(__DSYM(stat64),             libc_dos_stat64);
EXPORT(__KSYM(fstatat),            libc_fstatat);
EXPORT(__DSYM(fstatat),            libc_dos_fstatat);
EXPORT(__KSYM(fstatat64),          libc_fstatat64);
EXPORT(__DSYM(fstatat64),          libc_dos_fstatat64);
EXPORT(__KSYM(lstat),              libc_lstat);
EXPORT(__DSYM(lstat),              libc_dos_lstat);
EXPORT(__KSYM(lstat64),            libc_lstat64);
EXPORT(__DSYM(lstat64),            libc_dos_lstat64);

EXPORT(__fxstat,                   libc_version_fxstat);
EXPORT(__fxstat64,                 libc_version_fxstat64);
EXPORT(__KSYM(__xstat),            libc_version_xstat);
EXPORT(__DSYM(__xstat),            libc_dos_version_xstat);
EXPORT(__KSYM(__xstat64),          libc_version_xstat64);
EXPORT(__DSYM(__xstat64),          libc_dos_version_xstat64);
EXPORT(__KSYM(__lxstat),           libc_version_lxstat);
EXPORT(__DSYM(__lxstat),           libc_dos_version_lxstat);
EXPORT(__KSYM(__lxstat64),         libc_version_lxstat64);
EXPORT(__DSYM(__lxstat64),         libc_dos_version_lxstat64);
EXPORT(__KSYM(__fxstatat),         libc_version_fxstatat);
EXPORT(__DSYM(__fxstatat),         libc_dos_version_fxstatat);
EXPORT(__KSYM(__fxstatat64),       libc_version_fxstatat64);
EXPORT(__DSYM(__fxstatat64),       libc_dos_version_fxstatat64);

/* DOS function aliases. */
EXPORT(__DSYM(_chsize),            libc_ftruncate);
EXPORT(__DSYM(_chsize_s),          libc_ftruncate64);
EXPORT(__DSYM(_close),             libc_close);
EXPORT(__DSYM(_commit),            libc_fdatasync);
EXPORT(__DSYM(_dup),               libc_dup);
EXPORT(__DSYM(_dup2),              libc_dup2);
EXPORT(__DSYM(_locking),           libc_lockf);
EXPORT(__DSYM(_lseek),             libc_lseek);
EXPORT(__DSYM(_lseeki64),          libc_lseek64);
EXPORT(__DSYM(_chdir),             libc_dos_chdir);
EXPORT(__DSYM(_access),            libc_dos_access);
EXPORT(__DSYM(_chmod),             libc_dos_chmod);
EXPORT(__DSYM(_mkdir),             libc_dos_mkdir1);
EXPORT(__DSYM(_rmdir),             libc_dos_rmdir);
EXPORT(__DSYM(_unlink),            libc_dos_unlink);
EXPORT(__KSYM(access_s),           libd_access_s);
EXPORT(__DSYM(_access_s),          libd_dos_access_s);

#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
EXPORT(__DSYMw16(_getcwd_dbg),libc_dos_getcwd);
EXPORT(__DSYMw16(_getdcwd_dbg),libd_dos_getdcwd);
#else
EXPORT(__DSYMw16(_getcwd_dbg),libc_dos_getcwd_dbg);
CRT_DOS char *LIBCCALL
libc_dos_getcwd_dbg(char *buf, size_t size,
                    int UNUSED(blocktype),
                    char const *UNUSED(filename),
                    int UNUSED(lno)) {
 return libc_dos_getcwd(buf,size);
}
EXPORT(__DSYMw16(_getdcwd_dbg),libd_dos_getdcwd_dbg);
CRT_DOS char *LIBCCALL
libd_dos_getdcwd_dbg(int drive, char *buf, size_t size,
                     int UNUSED(blocktype),
                     char const *UNUSED(filename),
                     int UNUSED(lno)) {
 return libc_dos_getdcwd(drive,buf,size);
}
#endif



/* DOS-exclusive functions. */
EXPORT(__lock_fhandle,            _libd_lock_fhandle);
EXPORT(_unlock_fhandle,            libd_unlock_fhandle);
EXPORT(_setmode,                   libd_setmode);
EXPORT(__KSYM(umask_s),            libc_umask_s);
EXPORT(__DSYM(_umask_s),           libd_umask_s);
EXPORT(_pipe,                      libd_pipe);
EXPORT(__DSYM(_read),              libd_read);
EXPORT(__DSYM(_write),             libd_write);
EXPORT(__KSYM(chdrive),            libd_chdrive);
EXPORT(__DSYM(_chdrive),           libd_chdrive);
EXPORT(__KSYM(getdrive),           libd_getdrive);
EXPORT(__DSYM(_getdrive),          libd_getdrive);
EXPORT(__KSYM(getdrives),          libd_getdrives);
EXPORT(__DSYM(_getdrives),         libd_getdrives);
EXPORT(_fstat,                     libd_fstat32);
EXPORT(_fstat32,                   libd_fstat32);
EXPORT(_fstati64,                  libd_fstat32i64);
EXPORT(_fstat32i64,                libd_fstat32i64);
EXPORT(_fstat64,                   libd_fstat64);
EXPORT(_fstat64i32,                libd_fstat64);
EXPORT(__KSYM(getdcwd),            libd_getdcwd);
EXPORT(__DSYM(_getdcwd),           libd_dos_getdcwd);
EXPORT(__KSYM(_stat),              libd_stat32);
EXPORT(__DSYM(_stat),              libd_dos_stat32);
EXPORT(__KSYM(_stat32),            libd_stat32);
EXPORT(__DSYM(_stat32),            libd_dos_stat32);
EXPORT(__KSYM(_stati64),           libd_stat32i64);
EXPORT(__DSYM(_stati64),           libd_dos_stat32i64);
EXPORT(__KSYM(_stat32i64),         libd_stat32i64);
EXPORT(__DSYM(_stat32i64),         libd_dos_stat32i64);
EXPORT(__KSYM(_stat64),            libd_stat64);
EXPORT(__DSYM(_stat64),            libd_dos_stat64);
EXPORT(__KSYM(_stat64i32),         libd_stat64);
EXPORT(__DSYM(_stat64i32),         libd_dos_stat64);



EXPORT(sysinfo,libc_sysinfo);
INTERN int LIBCCALL
libc_sysinfo(struct sysinfo *info) {
 return FORWARD_SYSTEM_ERROR(sys_sysinfo(info));
}

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



/* Export wide-character exec() and spawn() functions. */
EXPORT(__SYMw16(wfexecl),          libc_w16fexecl);
EXPORT(__SYMw32(wfexecl),          libc_w32fexecl);
EXPORT(__SYMw16(wfexecle),         libc_w16fexecle);
EXPORT(__SYMw32(wfexecle),         libc_w32fexecle);
EXPORT(__SYMw16(wfexecv),          libc_w16fexecv);
EXPORT(__SYMw32(wfexecv),          libc_w32fexecv);
EXPORT(__SYMw16(wfexecve),         libc_w16fexecve);
EXPORT(__SYMw32(wfexecve),         libc_w32fexecve);
EXPORT(__KSYMw16(wexecl),          libc_w16execl);
EXPORT(__KSYMw32(wexecl),          libc_w32execl);
EXPORT(__DSYMw16(_wexecl),         libc_dos_w16execl);
EXPORT(__DSYMw32(wexecl),          libc_dos_w32execl);
EXPORT(__KSYMw16(wexecle),         libc_w16execle);
EXPORT(__KSYMw32(wexecle),         libc_w32execle);
EXPORT(__DSYMw16(_wexecle),        libc_dos_w16execle);
EXPORT(__DSYMw32(wexecle),         libc_dos_w32execle);
EXPORT(__KSYMw16(wexeclp),         libc_w16execlp);
EXPORT(__KSYMw32(wexeclp),         libc_w32execlp);
EXPORT(__DSYMw16(_wexeclp),        libc_dos_w16execlp);
EXPORT(__DSYMw32(wexeclp),         libc_dos_w32execlp);
EXPORT(__KSYMw16(wexeclpe),        libc_w16execlpe);
EXPORT(__KSYMw32(wexeclpe),        libc_w32execlpe);
EXPORT(__DSYMw16(_wexeclpe),       libc_dos_w16execlpe);
EXPORT(__DSYMw32(wexeclpe),        libc_dos_w32execlpe);
EXPORT(__KSYMw16(wexecv),          libc_w16execv);
EXPORT(__KSYMw32(wexecv),          libc_w32execv);
EXPORT(__DSYMw16(_wexecv),         libc_dos_w16execv);
EXPORT(__DSYMw32(wexecv),          libc_dos_w32execv);
EXPORT(__KSYMw16(wexecve),         libc_w16execve);
EXPORT(__KSYMw32(wexecve),         libc_w32execve);
EXPORT(__DSYMw16(_wexecve),        libc_dos_w16execve);
EXPORT(__DSYMw32(wexecve),         libc_dos_w32execve);
EXPORT(__KSYMw16(wexecvp),         libc_w16execvp);
EXPORT(__KSYMw32(wexecvp),         libc_w32execvp);
EXPORT(__DSYMw16(_wexecvp),        libc_dos_w16execvp);
EXPORT(__DSYMw32(wexecvp),         libc_dos_w32execvp);
EXPORT(__KSYMw16(wexecvpe),        libc_w16execvpe);
EXPORT(__KSYMw32(wexecvpe),        libc_w32execvpe);
EXPORT(__DSYMw16(_wexecvpe),       libc_dos_w16execvpe);
EXPORT(__DSYMw32(wexecvpe),        libc_dos_w32execvpe);
EXPORT(__KSYMw16(wfexeclat),       libc_w16fexeclat);
EXPORT(__KSYMw32(wfexeclat),       libc_w32fexeclat);
EXPORT(__DSYMw16(wfexeclat),       libc_dos_w16fexeclat);
EXPORT(__DSYMw32(wfexeclat),       libc_dos_w32fexeclat);
EXPORT(__KSYMw16(wfexecleat),      libc_w16fexecleat);
EXPORT(__KSYMw32(wfexecleat),      libc_w32fexecleat);
EXPORT(__DSYMw16(wfexecleat),      libc_dos_w16fexecleat);
EXPORT(__DSYMw32(wfexecleat),      libc_dos_w32fexecleat);
EXPORT(__KSYMw16(wfexeclpat),      libc_w16fexeclpat);
EXPORT(__KSYMw32(wfexeclpat),      libc_w32fexeclpat);
EXPORT(__DSYMw16(wfexeclpat),      libc_dos_w16fexeclpat);
EXPORT(__DSYMw32(wfexeclpat),      libc_dos_w32fexeclpat);
EXPORT(__KSYMw16(wfexeclpeat),     libc_w16fexeclpeat);
EXPORT(__KSYMw32(wfexeclpeat),     libc_w32fexeclpeat);
EXPORT(__DSYMw16(wfexeclpeat),     libc_dos_w16fexeclpeat);
EXPORT(__DSYMw32(wfexeclpeat),     libc_dos_w32fexeclpeat);
EXPORT(__KSYMw16(wfexecvat),       libc_w16fexecvat);
EXPORT(__KSYMw32(wfexecvat),       libc_w32fexecvat);
EXPORT(__DSYMw16(wfexecvat),       libc_dos_w16fexecvat);
EXPORT(__DSYMw32(wfexecvat),       libc_dos_w32fexecvat);
EXPORT(__KSYMw16(wfexecvpat),      libc_w16fexecvpat);
EXPORT(__KSYMw32(wfexecvpat),      libc_w32fexecvpat);
EXPORT(__DSYMw16(wfexecvpat),      libc_dos_w16fexecvpat);
EXPORT(__DSYMw32(wfexecvpat),      libc_dos_w32fexecvpat);
//EXPORT(__KSYMw16(wfexecveat),    libc_w16fexecveat); /* Exported by `widechar.c' */
//EXPORT(__KSYMw32(wfexecveat),    libc_w32fexecveat); /* Exported by `widechar.c' */
EXPORT(__DSYMw16(wfexecveat),      libc_dos_w16fexecveat);
EXPORT(__DSYMw32(wfexecveat),      libc_dos_w32fexecveat);
//EXPORT(__KSYMw16(wfexecvpeat),   libc_w16fexecvpeat); /* Exported by `widechar.c' */
//EXPORT(__KSYMw32(wfexecvpeat),   libc_w32fexecvpeat); /* Exported by `widechar.c' */
EXPORT(__DSYMw16(wfexecvpeat),     libc_dos_w16fexecvpeat);
EXPORT(__DSYMw32(wfexecvpeat),     libc_dos_w32fexecvpeat);
EXPORT(__SYMw16(wfspawnl),         libc_w16fspawnl);
EXPORT(__SYMw32(wfspawnl),         libc_w32fspawnl);
EXPORT(__SYMw16(wfspawnle),        libc_w16fspawnle);
EXPORT(__SYMw32(wfspawnle),        libc_w32fspawnle);
EXPORT(__SYMw16(wfspawnv),         libc_w16fspawnv);
EXPORT(__SYMw32(wfspawnv),         libc_w32fspawnv);
EXPORT(__SYMw16(wfspawnve),        libc_w16fspawnve);
EXPORT(__SYMw32(wfspawnve),        libc_w32fspawnve);
EXPORT(__KSYMw16(wspawnl),         libc_w16spawnl);
EXPORT(__KSYMw32(wspawnl),         libc_w32spawnl);
EXPORT(__DSYMw16(_wspawnl),        libc_dos_w16spawnl);
EXPORT(__DSYMw32(wspawnl),         libc_dos_w32spawnl);
EXPORT(__KSYMw16(wspawnle),        libc_w16spawnle);
EXPORT(__KSYMw32(wspawnle),        libc_w32spawnle);
EXPORT(__DSYMw16(_wspawnle),       libc_dos_w16spawnle);
EXPORT(__DSYMw32(wspawnle),        libc_dos_w32spawnle);
EXPORT(__KSYMw16(wspawnlp),        libc_w16spawnlp);
EXPORT(__KSYMw32(wspawnlp),        libc_w32spawnlp);
EXPORT(__DSYMw16(_wspawnlp),       libc_dos_w16spawnlp);
EXPORT(__DSYMw32(wspawnlp),        libc_dos_w32spawnlp);
EXPORT(__KSYMw16(wspawnlpe),       libc_w16spawnlpe);
EXPORT(__KSYMw32(wspawnlpe),       libc_w32spawnlpe);
EXPORT(__DSYMw16(_wspawnlpe),      libc_dos_w16spawnlpe);
EXPORT(__DSYMw32(wspawnlpe),       libc_dos_w32spawnlpe);
EXPORT(__KSYMw16(wspawnv),         libc_w16spawnv);
EXPORT(__KSYMw32(wspawnv),         libc_w32spawnv);
EXPORT(__DSYMw16(_wspawnv),        libc_dos_w16spawnv);
EXPORT(__DSYMw32(wspawnv),         libc_dos_w32spawnv);
EXPORT(__KSYMw16(wspawnve),        libc_w16spawnve);
EXPORT(__KSYMw32(wspawnve),        libc_w32spawnve);
EXPORT(__DSYMw16(_wspawnve),       libc_dos_w16spawnve);
EXPORT(__DSYMw32(wspawnve),        libc_dos_w32spawnve);
EXPORT(__KSYMw16(wspawnvp),        libc_w16spawnvp);
EXPORT(__KSYMw32(wspawnvp),        libc_w32spawnvp);
EXPORT(__DSYMw16(_wspawnvp),       libc_dos_w16spawnvp);
EXPORT(__DSYMw32(wspawnvp),        libc_dos_w32spawnvp);
EXPORT(__KSYMw16(wspawnvpe),       libc_w16spawnvpe);
EXPORT(__KSYMw32(wspawnvpe),       libc_w32spawnvpe);
EXPORT(__DSYMw16(_wspawnvpe),      libc_dos_w16spawnvpe);
EXPORT(__DSYMw32(wspawnvpe),       libc_dos_w32spawnvpe);
EXPORT(__KSYMw16(wfspawnlat),      libc_w16fspawnlat);
EXPORT(__KSYMw32(wfspawnlat),      libc_w32fspawnlat);
EXPORT(__DSYMw16(wfspawnlat),      libc_dos_w16fspawnlat);
EXPORT(__DSYMw32(wfspawnlat),      libc_dos_w32fspawnlat);
EXPORT(__KSYMw16(wfspawnleat),     libc_w16fspawnleat);
EXPORT(__KSYMw32(wfspawnleat),     libc_w32fspawnleat);
EXPORT(__DSYMw16(wfspawnleat),     libc_dos_w16fspawnleat);
EXPORT(__DSYMw32(wfspawnleat),     libc_dos_w32fspawnleat);
EXPORT(__KSYMw16(wfspawnlpat),     libc_w16fspawnlpat);
EXPORT(__KSYMw32(wfspawnlpat),     libc_w32fspawnlpat);
EXPORT(__DSYMw16(wfspawnlpat),     libc_dos_w16fspawnlpat);
EXPORT(__DSYMw32(wfspawnlpat),     libc_dos_w32fspawnlpat);
EXPORT(__KSYMw16(wfspawnlpeat),    libc_w16fspawnlpeat);
EXPORT(__KSYMw32(wfspawnlpeat),    libc_w32fspawnlpeat);
EXPORT(__DSYMw16(wfspawnlpeat),    libc_dos_w16fspawnlpeat);
EXPORT(__DSYMw32(wfspawnlpeat),    libc_dos_w32fspawnlpeat);
EXPORT(__KSYMw16(wfspawnvat),      libc_w16fspawnvat);
EXPORT(__KSYMw32(wfspawnvat),      libc_w32fspawnvat);
EXPORT(__DSYMw16(wfspawnvat),      libc_dos_w16fspawnvat);
EXPORT(__DSYMw32(wfspawnvat),      libc_dos_w32fspawnvat);
EXPORT(__KSYMw16(wfspawnvpat),     libc_w16fspawnvpat);
EXPORT(__KSYMw32(wfspawnvpat),     libc_w32fspawnvpat);
EXPORT(__DSYMw16(wfspawnvpat),     libc_dos_w16fspawnvpat);
EXPORT(__DSYMw32(wfspawnvpat),     libc_dos_w32fspawnvpat);
//EXPORT(__KSYMw16(wfspawnveat),   libc_w16fspawnveat); /* Exported by `widechar.c' */
//EXPORT(__KSYMw32(wfspawnveat),   libc_w32fspawnveat); /* Exported by `widechar.c' */
EXPORT(__DSYMw16(wfspawnveat),     libc_dos_w16fspawnveat);
EXPORT(__DSYMw32(wfspawnveat),     libc_dos_w32fspawnveat);
//EXPORT(__KSYMw16(wfspawnvpeat),  libc_w16fspawnvpeat); /* Exported by `widechar.c' */
//EXPORT(__KSYMw32(wfspawnvpeat),  libc_w32fspawnvpeat); /* Exported by `widechar.c' */
EXPORT(__DSYMw16(wfspawnvpeat),    libc_dos_w16fspawnvpeat);
EXPORT(__DSYMw32(wfspawnvpeat),    libc_dos_w32fspawnvpeat);


















EXPORT(Xopen,libc_Xopen);
EXPORT(Xopen64,libc_Xopen);
CRT_EXCEPT int ATTR_CDECL
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
CRT_EXCEPT int LIBCCALL libc_Xcreat(char const *file, mode_t mode) {
 return libc_Xopenat(AT_FDCWD,file,O_CREAT|O_WRONLY|O_TRUNC,mode);
}
EXPORT(Xpipe,libc_Xpipe);
CRT_EXCEPT void LIBCCALL libc_Xpipe(int pipedes[2]) {
 libc_Xpipe2(pipedes,0);
}
EXPORT(Xdup2,libc_Xdup2);
CRT_EXCEPT int LIBCCALL
libc_Xdup2(int ofd, int nfd) {
 return libc_Xdup3(ofd,nfd,0);
}
EXPORT(Xxfrealpath2,libc_Xxfrealpath2);
CRT_EXCEPT size_t LIBCCALL
libc_Xxfrealpath2(fd_t fd, char *buf, size_t bufsize, unsigned int type) {
 return libc_Xxfrealpathat2(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type);
}
EXPORT(Xxrealpath2,libc_Xxrealpath2);
CRT_EXCEPT size_t LIBCCALL
libc_Xxrealpath2(char const *path, char *buf, size_t bufsize, unsigned int type) {
 return libc_Xxfrealpathat2(AT_FDCWD,path,0,buf,bufsize,type);
}
EXPORT(Xxfrealpath,libc_Xxfrealpath);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libc_Xxfrealpath(fd_t fd, char *buf, size_t bufsize, unsigned int type) {
 return libc_Xxfrealpathat(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type);
}
EXPORT(Xxrealpath,libc_Xxrealpath);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libc_Xxrealpath(char const *path, char *buf, size_t bufsize, unsigned int type) {
 return libc_Xxfrealpathat(AT_FDCWD,path,0,buf,bufsize,type);
}

CRT_EXCEPT ATTR_NORETURN void LIBCCALL
libc_throw_buffer_too_small(size_t reqsize, size_t bufsize) {
 struct exception_info *info = libc_error_info();
 libc_memset(info->e_error.e_pointers,0,
             sizeof(info->e_error.e_pointers));
 info->e_error.e_code = E_BUFFER_TOO_SMALL;
 info->e_error.e_flag = ERR_FNORMAL;
 info->e_error.e_buffer_too_small.bs_reqsize = reqsize;
 info->e_error.e_buffer_too_small.bs_bufsize = bufsize;
 libc_error_throw_current();
 __builtin_unreachable();
}

EXPORT(Xxfrealpathat,libc_Xxfrealpathat);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libc_Xxfrealpathat(fd_t fd, char const *path, int flags,
                   char *EXCEPT_VAR buf, size_t bufsize, unsigned int type) {
 size_t COMPILER_IGNORE_UNINITIALIZED(reqsize);
 bool EXCEPT_VAR is_libc_buffer = false;
 if (!buf && bufsize)
      is_libc_buffer = true,
      buf = (char *)libc_Xmalloc(bufsize*sizeof(char));
#if 1
 else if (!buf && !bufsize) {
  char *result;
  /* Allocate a small, initial buffer */
  bufsize = (260+1);
  buf = (char *)libc_Xmalloc(bufsize*sizeof(char));
  TRY {
   reqsize = libc_Xxfrealpathat2(fd,path,flags,buf,bufsize,type);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   libc_free(buf);
   error_rethrow();
  }
  if ((size_t)reqsize >= bufsize)
       goto do_dynamic;
  /* Free unused memory. */
  result = (char *)libc_realloc(buf,((size_t)reqsize+1));
  return likely(result) ? result : buf;
 }
#endif
 TRY {
  reqsize = libc_Xxfrealpathat2(fd,path,flags,buf,bufsize,type);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  if (is_libc_buffer)
      libc_free(buf);
  error_rethrow();
 }
 if (reqsize >= bufsize) {
  if (!buf) {
   /* Allocate a new buffer dynamically. */
do_dynamic:
   do {
    bufsize = (size_t)reqsize;
    TRY {
     buf = (char *)libc_Xrealloc(buf,(bufsize+1)*sizeof(char));
     reqsize = Xsys_xfrealpathat(fd,path,flags,buf,bufsize+1,type);
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     libc_free(buf);
     error_rethrow();
    }
   } while ((size_t)reqsize != bufsize);
   return buf;
  }
  libc_throw_buffer_too_small(reqsize,bufsize);
 } else if (!reqsize && !buf) {
  /* Must allocate an empty buffer... */
  buf = (char *)libc_Xmalloc(1*sizeof(char));
  buf[0] = '\0';
 }
 return buf;
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
EXPORT(Xrealpath,libc_Xrealpath);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libc_Xrealpath(char const *name, char *resolved) {
 return libc_Xxrealpath(name,resolved,resolved ? PATH_MAX : 0,REALPATH_FPATH);
}
EXPORT(Xfrealpath,libc_Xfrealpath);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libc_Xfrealpath(fd_t fd, char *resolved, size_t bufsize) {
 return libc_Xxfrealpath(fd,resolved,bufsize,REALPATH_FPATH);
}
EXPORT(Xcanonicalize_file_name,libc_Xcanonicalize_file_name);
CRT_EXCEPT ATTR_MALLOC ATTR_RETNONNULL char *LIBCCALL
libc_Xcanonicalize_file_name(char const *name) {
 return libc_Xrealpath(name,NULL);
}

EXPORT(Xgetcwd,libc_Xgetcwd);
CRT_EXCEPT char *LIBCCALL
libc_Xgetcwd(char *buf, size_t size) {
 return libc_Xxfrealpathat(AT_FDCWD,libc_empty_string,AT_EMPTY_PATH,buf,size,REALPATH_FPATH);
}
EXPORT(Xget_current_dir_name,libc_Xget_current_dir_name);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libc_Xget_current_dir_name(void) {
 return libc_Xgetcwd(NULL,0);
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
EXPORT(Xgetdcwd,libd_Xgetdcwd);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libd_Xgetdcwd(int drive, char *buf, size_t size) {
 return libc_Xxfrealpathat(AT_FDDRIVE_CWD('A'+drive),libc_empty_string,
                           AT_EMPTY_PATH,buf,size,REALPATH_FPATH);
}
EXPORT(Xchdrive,libd_Xchdrive);
CRT_EXCEPT void LIBCCALL libd_Xchdrive(int drive) {
 char temp[3] = "?:";
 temp[0] = 'A'+drive;
 libc_Xfchdirat(AT_FDCWD,temp,AT_DOSPATH);
}
EXPORT(Xgetdrive,libd_Xgetdrive);
CRT_EXCEPT int LIBCCALL libd_Xgetdrive(void) {
 char buf[1];
 if (!libc_Xxfrealpath2(AT_FDCWD,buf,1,REALPATH_FDRIVE))
      error_throw(E_NO_DATA); /* Shouldn't happen... */
 return buf[0]-'A';
}

EXPORT(Xkstat64,libc_Xkstat64);
CRT_EXCEPT void LIBCCALL
libc_Xkstat64(char const *file, struct __kos_stat64 *buf) {
 Xsys_fstatat64(AT_FDCWD,file,buf,0);
}
DEFINE_INTERN_ALIAS(libc_Xkstat,libc_Xkstat64);
EXPORT(Xkstat,libc_Xkstat);

EXPORT(Xklstat64,libc_Xklstat64);
CRT_EXCEPT void LIBCCALL
libc_Xklstat64(char const *file, struct __kos_stat64 *buf) {
 Xsys_fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW);
}
DEFINE_INTERN_ALIAS(libc_Xklstat,libc_Xklstat64);
EXPORT(Xklstat,libc_Xklstat);


EXPORT(Xfutimens,libc_Xfutimens);
CRT_EXCEPT void LIBCCALL
libc_Xfutimens(fd_t fd, struct timespec32 const times[2]) {
 libc_Xutimensat(fd,libc_empty_string,times,AT_EMPTY_PATH);
}
EXPORT(Xfutimens64,libc_Xfutimens64);
CRT_EXCEPT void LIBCCALL
libc_Xfutimens64(fd_t fd, struct timespec64 const times[2]) {
 Xsys_utimensat(fd,libc_empty_string,times,AT_EMPTY_PATH);
}
EXPORT(Xutimensat,libc_Xutimensat);
CRT_EXCEPT void LIBCCALL
libc_Xutimensat(fd_t dfd, char const *path, struct timespec32 const times[2], int flags) {
 if (!times) {
  Xsys_utimensat(dfd,path,NULL,flags);
 } else {
  struct timespec64 t64[3];
  t64[0].tv_sec  = times[0].tv_sec;
  t64[0].tv_nsec = times[0].tv_nsec;
  t64[1].tv_sec  = times[1].tv_sec;
  t64[1].tv_nsec = times[1].tv_nsec;
  if (flags & AT_CHANGE_CTIME) {
   t64[2].tv_sec  = times[2].tv_sec;
   t64[2].tv_nsec = times[2].tv_nsec;
  }
  Xsys_utimensat(dfd,path,t64,flags);
 }
}
EXPORT(Xfutimeat,libc_Xfutimeat);
CRT_EXCEPT void LIBCCALL
libc_Xfutimeat(fd_t dfd, char const *file, struct utimbuf32 const *file_times, int flags) {
 struct timespec64 times[2];
 if (!file_times) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  times[0].tv_sec  = file_times->actime;
  times[1].tv_sec  = file_times->modtime;
  times[0].tv_nsec = 0;
  times[1].tv_nsec = 0;
 }
 Xsys_utimensat(dfd,file,times,flags);
}
EXPORT(Xfutimeat64,libc_Xfutimeat64);
CRT_EXCEPT void LIBCCALL
libc_Xfutimeat64(fd_t dfd, char const *file, struct utimbuf64 const *file_times, int flags) {
 struct timespec64 times[2];
 if (!file_times) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  times[0].tv_sec  = file_times->actime;
  times[1].tv_sec  = file_times->modtime;
  times[0].tv_nsec = 0;
  times[1].tv_nsec = 0;
 }
 Xsys_utimensat(dfd,file,times,flags);
}
EXPORT(Xfutime,libc_Xfutime);
CRT_EXCEPT void LIBCCALL
libc_Xfutime(fd_t fd, struct utimbuf32 const *file_times) {
 libc_Xfutimeat(fd,libc_empty_string,file_times,0); 
}
EXPORT(Xfutime64,libc_Xfutime64);
CRT_EXCEPT void LIBCCALL
libc_Xfutime64(fd_t fd, struct utimbuf64 const *file_times) {
 libc_Xfutimeat64(fd,libc_empty_string,file_times,0); 
}
EXPORT(Xutime,libc_Xutime);
CRT_EXCEPT void LIBCCALL
libc_Xutime(char const *file, struct utimbuf32 const *file_times) {
 libc_Xfutimeat(AT_FDCWD,file,file_times,0); 
}
EXPORT(Xutime64,libc_Xutime64);
CRT_EXCEPT void LIBCCALL
libc_Xutime64(char const *file, struct utimbuf64 const *file_times) {
 libc_Xfutimeat64(AT_FDCWD,file,file_times,0); 
}


CRT_EXCEPT void LIBCCALL
libc_impl_Xfutimesat64(fd_t fd, char const *file, struct timeval64 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 Xsys_utimensat(fd,file,times,flags);
}
CRT_EXCEPT void LIBCCALL
libc_impl_Xfutimesat(fd_t fd, char const *file, struct timeval32 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 Xsys_utimensat(fd,file,times,flags);
}

EXPORT(Xfutimesat,libc_Xfutimesat);
CRT_EXCEPT void LIBCCALL
libc_Xfutimesat(fd_t fd, char const *file, struct timeval32 const tvp[2]) {
 libc_impl_Xfutimesat(fd,file,tvp,0);
}

EXPORT(Xfutimesat64,libc_Xfutimesat64);
CRT_EXCEPT void LIBCCALL
libc_Xfutimesat64(fd_t fd, char const *file, struct timeval64 const tvp[2]) {
 libc_impl_Xfutimesat64(fd,file,tvp,0);
}

EXPORT(Xfutimes,libc_Xfutimes);
CRT_EXCEPT void LIBCCALL libc_Xfutimes(fd_t fd, struct timeval32 const tvp[2]) {
 libc_impl_Xfutimesat(fd,libc_empty_string,tvp,0);
}

EXPORT(Xfutimes64,libc_Xfutimes64);
CRT_EXCEPT void LIBCCALL
libc_Xfutimes64(fd_t fd, struct timeval64 const tvp[2]) {
 libc_impl_Xfutimesat64(fd,libc_empty_string,tvp,0);
}

EXPORT(Xutimes,libc_Xutimes);
CRT_EXCEPT void LIBCCALL
libc_Xutimes(char const *file, struct timeval32 const tvp[2]) {
 libc_impl_Xfutimesat(AT_FDCWD,file,tvp,0);
}

EXPORT(Xutimes64,libc_Xutimes64);
CRT_EXCEPT void LIBCCALL
libc_Xutimes64(char const *file, struct timeval64 const tvp[2]) {
 libc_impl_Xfutimesat64(AT_FDCWD,file,tvp,0);
}

EXPORT(Xlutimes,libc_Xlutimes);
CRT_EXCEPT void LIBCCALL
libc_Xlutimes(char const *file, struct timeval32 const tvp[2]) {
 libc_impl_Xfutimesat(AT_FDCWD,file,tvp,0);
}

EXPORT(Xlutimes64,libc_Xlutimes64);
CRT_EXCEPT void LIBCCALL
libc_Xlutimes64(char const *file, struct timeval64 const tvp[2]) {
 libc_impl_Xfutimesat64(AT_FDCWD,file,tvp,0);
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


EXPORT(Xexecv,libc_Xexecv);
EXPORT(Xexecvp,libc_Xexecvp);
EXPORT(Xexecvpe,libc_Xexecvpe);
EXPORT(Xfexecv,libc_Xfexecv);
EXPORT(Xfexecve,libc_Xfexecve);
EXPORT(Xfexecvat,libc_Xfexecvat);
EXPORT(Xfexecvpat,libc_Xfexecvpat);
EXPORT(Xfexecvpeat,libc_Xfexecvpeat);
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xexecv(char const *path, char *const argv[]) { Xsys_execve(path,argv,environ); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xexecvp(char const *file, char *const argv[]) { libc_Xfexecvpeat(file,argv,environ,0); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xexecvpe(char const *file, char *const argv[], char *const envp[]) { libc_Xfexecvpeat(file,argv,envp,0); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xfexecv(int exec_fd, char *const argv[]) { Xsys_execveat(exec_fd,libc_empty_string,argv,environ,AT_EMPTY_PATH); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xfexecve(int exec_fd, char *const argv[], char *const envp[]) { Xsys_execveat(exec_fd,libc_empty_string,argv,envp,AT_EMPTY_PATH); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xfexecvat(fd_t dfd, char const *path, char *const argv[], int flags) { Xsys_execveat(dfd,path,argv,environ,flags); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xfexecvpat(char const *file, char *const argv[], int flags) { libc_Xfexecvpeat(file,argv,environ,flags); }
CRT_EXCEPT ATTR_NORETURN void LIBCCALL
libc_Xfexecvpeat(char const *file,
                 char *const argv[],
                 char *const envp[],
                 int flags) {
 /* TODO: Search $PATH */
 error_throw(E_NOT_IMPLEMENTED);
}


EXPORT(Xexecl,libc_Xexecl);
EXPORT(Xexecle,libc_Xexecle);
EXPORT(Xexeclp,libc_Xexeclp);
EXPORT(Xexeclpe,libc_Xexeclpe);
EXPORT(Xfexecl,libc_Xfexecl);
EXPORT(Xfexecle,libc_Xfexecle);
EXPORT(Xfexeclat,libc_Xfexeclat);
EXPORT(Xfexecleat,libc_Xfexecleat);
EXPORT(Xfexeclpat,libc_Xfexeclpat);
EXPORT(Xfexeclpeat,libc_Xfexeclpeat);
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xexecl(char const *path, char const *args, ...) { CAPTURE_ARGV(); libc_Xexecve(path,argv,environ); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xexecle(char const *path, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xexecve(path,argv,envp); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xexeclp(char const *file, char const *args, ...) { CAPTURE_ARGV(); libc_Xexecvpe(file,argv,environ); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xexeclpe(char const *file, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xexecvpe(file,argv,envp); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xfexecl(int exec_fd, char const *args, ...) { CAPTURE_ARGV(); libc_Xfexecve(exec_fd,argv,environ); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xfexecle(int exec_fd, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xfexecve(exec_fd,argv,envp); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xfexeclat(fd_t dfd, char const *path, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); Xsys_execveat(dfd,path,argv,environ,flags); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xfexecleat(fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); Xsys_execveat(dfd,path,argv,envp,flags); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xfexeclpat(char const *file, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); libc_Xfexecvpeat(file,argv,environ,flags); }
CRT_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xfexeclpeat(char const *file, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); libc_Xfexecvpeat(file,argv,envp,flags); }

EXPORT(Xcwait,libc_Xcwait);
INTERN pid_t LIBCCALL
libc_Xcwait(int *tstat, pid_t pid, int UNUSED(action)) {
 /* This one's pretty simple, because it's literally just a waitpid() system call...
  * (It even returns the same thing, that being the PID of the joined process...) */
 return libc_Xwaitpid(pid,tstat,WEXITED);
 /* NOTE: Apparently, the `action' argument is completely ignored... */
}




EXPORT(Xspawnv,libc_Xspawnv);
EXPORT(Xspawnve,libc_Xspawnve);
EXPORT(Xspawnvp,libc_Xspawnvp);
EXPORT(Xspawnvpe,libc_Xspawnvpe);
EXPORT(Xfspawnv,libc_Xfspawnv);
EXPORT(Xfspawnve,libc_Xfspawnve);
EXPORT(Xfspawnvat,libc_Xfspawnvat);
EXPORT(Xfspawnveat,libc_Xfspawnveat);
EXPORT(Xfspawnvpat,libc_Xfspawnvpat);
EXPORT(Xfspawnvpeat,libc_Xfspawnvpeat);
CRT_EXCEPT pid_t LIBCCALL libc_Xspawnv(int mode, char const *path, char *const argv[]) { return libc_Xfspawnveat(mode,AT_FDCWD,path,argv,environ,0); }
CRT_EXCEPT pid_t LIBCCALL libc_Xspawnve(int mode, char const *path, char *const argv[], char *const envp[]) { return libc_Xfspawnveat(mode,AT_FDCWD,path,argv,envp,0); }
CRT_EXCEPT pid_t LIBCCALL libc_Xspawnvp(int mode, char const *file, char *const argv[]) { return libc_Xfspawnvpeat(mode,file,argv,environ,0); }
CRT_EXCEPT pid_t LIBCCALL libc_Xspawnvpe(int mode, char const *file, char *const argv[], char *const envp[]) { return libc_Xfspawnvpeat(mode,file,argv,envp,0); }
CRT_EXCEPT pid_t LIBCCALL libc_Xfspawnv(int mode, int exec_fd, char *const argv[]) { return libc_Xfspawnveat(mode,exec_fd,libc_empty_string,argv,environ,AT_EMPTY_PATH); }
CRT_EXCEPT pid_t LIBCCALL libc_Xfspawnve(int mode, int exec_fd, char *const argv[], char *const envp[]) { return libc_Xfspawnveat(mode,exec_fd,libc_empty_string,argv,envp,AT_EMPTY_PATH); }
CRT_EXCEPT pid_t LIBCCALL libc_Xfspawnvat(int mode, fd_t dfd, char const *path, char *const argv[], int flags) { return libc_Xfspawnveat(mode,dfd,path,argv,environ,flags); }
CRT_EXCEPT pid_t LIBCCALL libc_Xfspawnvpat(int mode, char const *file, char *const argv[], int flags) { return libc_Xfspawnvpeat(mode,file,argv,environ,flags); }
CRT_EXCEPT pid_t LIBCCALL
libc_Xfspawnveat(int mode, fd_t dfd,
                 char const *path,
                 char *const argv[],
                 char *const envp[],
                 int flags) {
 /* TODO */
 error_throw(E_NOT_IMPLEMENTED);
}
CRT_EXCEPT pid_t LIBCCALL
libc_Xfspawnvpeat(int mode,
                  char const *file,
                  char *const argv[],
                  char *const envp[],
                  int flags) {
 /* TODO: Search $PATH */
 error_throw(E_NOT_IMPLEMENTED);
}


EXPORT(Xspawnl,libc_Xspawnl);
EXPORT(Xspawnle,libc_Xspawnle);
EXPORT(Xspawnlp,libc_Xspawnlp);
EXPORT(Xspawnlpe,libc_Xspawnlpe);
EXPORT(Xfspawnl,libc_Xfspawnl);
EXPORT(Xfspawnle,libc_Xfspawnle);
EXPORT(Xfspawnlat,libc_Xfspawnlat);
EXPORT(Xfspawnleat,libc_Xfspawnleat);
EXPORT(Xfspawnlpat,libc_Xfspawnlpat);
EXPORT(Xfspawnlpeat,libc_Xfspawnlpeat);
CRT_EXCEPT pid_t ATTR_CDECL libc_Xspawnl(int mode, char const *path, char const *args, ...) { CAPTURE_ARGV(); return libc_Xspawnve(mode,path,argv,environ); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xspawnle(int mode, char const *path, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xspawnve(mode,path,argv,envp); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xspawnlp(int mode, char const *file, char const *args, ...) { CAPTURE_ARGV(); return libc_Xspawnvpe(mode,file,argv,environ); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xspawnlpe(int mode, char const *file, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xspawnvpe(mode,file,argv,envp); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xfspawnl(int mode, int exec_fd, char const *args, ...) { CAPTURE_ARGV(); return libc_Xfspawnve(mode,exec_fd,argv,environ); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xfspawnle(int mode, int exec_fd, char const *args, ... /*, char *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xfspawnve(mode,exec_fd,argv,envp); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xfspawnlat(int mode, fd_t dfd, char const *path, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_Xfspawnveat(mode,dfd,path,argv,environ,flags); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xfspawnleat(int mode, fd_t dfd, char const *path, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_Xfspawnveat(mode,dfd,path,argv,envp,flags); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xfspawnlpat(int mode, char const *file, char const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_Xfspawnvpeat(mode,file,argv,environ,flags); }
CRT_EXCEPT pid_t ATTR_CDECL libc_Xfspawnlpeat(int mode, char const *file, char const *args, ... /*, char *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_Xfspawnvpeat(mode,file,argv,envp,flags); }





EXPORT(__SYMw16(Xwexecv),libc_Xw16execv);
EXPORT(__SYMw32(Xwexecv),libc_Xw32execv);
EXPORT(__SYMw16(Xwexecve),libc_Xw16execve);
EXPORT(__SYMw32(Xwexecve),libc_Xw32execve);
EXPORT(__SYMw16(Xwexecvp),libc_Xw16execvp);
EXPORT(__SYMw32(Xwexecvp),libc_Xw32execvp);
EXPORT(__SYMw16(Xwexecvpe),libc_Xw16execvpe);
EXPORT(__SYMw32(Xwexecvpe),libc_Xw32execvpe);
EXPORT(__SYMw16(Xwfexecv),libc_Xw16fexecv);
EXPORT(__SYMw32(Xwfexecv),libc_Xw32fexecv);
EXPORT(__SYMw16(Xwfexecve),libc_Xw16fexecve);
EXPORT(__SYMw32(Xwfexecve),libc_Xw32fexecve);
EXPORT(__SYMw16(Xwfexecvat),libc_Xw16fexecvat);
EXPORT(__SYMw32(Xwfexecvat),libc_Xw32fexecvat);
EXPORT(__SYMw16(Xwfexecvpat),libc_Xw16fexecvpat);
EXPORT(__SYMw32(Xwfexecvpat),libc_Xw32fexecvpat);
EXPORT(__SYMw16(Xwspawnv),libc_Xw16spawnv);
EXPORT(__SYMw32(Xwspawnv),libc_Xw32spawnv);
EXPORT(__SYMw16(Xwspawnve),libc_Xw16spawnve);
EXPORT(__SYMw32(Xwspawnve),libc_Xw32spawnve);
EXPORT(__SYMw16(Xwspawnvp),libc_Xw16spawnvp);
EXPORT(__SYMw32(Xwspawnvp),libc_Xw32spawnvp);
EXPORT(__SYMw16(Xwspawnvpe),libc_Xw16spawnvpe);
EXPORT(__SYMw32(Xwspawnvpe),libc_Xw32spawnvpe);
EXPORT(__SYMw16(Xwfspawnv),libc_Xw16fspawnv);
EXPORT(__SYMw32(Xwfspawnv),libc_Xw32fspawnv);
EXPORT(__SYMw16(Xwfspawnve),libc_Xw16fspawnve);
EXPORT(__SYMw32(Xwfspawnve),libc_Xw32fspawnve);
EXPORT(__SYMw16(Xwfspawnvat),libc_Xw16fspawnvat);
EXPORT(__SYMw32(Xwfspawnvat),libc_Xw32fspawnvat);
EXPORT(__SYMw16(Xwfspawnvpat),libc_Xw16fspawnvpat);
EXPORT(__SYMw32(Xwfspawnvpat),libc_Xw32fspawnvpat);
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16execv(char16_t const *path, char16_t *const argv[]) { libc_Xw16fexecveat(AT_FDCWD,path,argv,libc_Xget_w16environ(),0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32execv(char32_t const *path, char32_t *const argv[]) { libc_Xw32fexecveat(AT_FDCWD,path,argv,libc_Xget_w32environ(),0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16execve(char16_t const *path, char16_t *const argv[], char16_t *const envp[]) { libc_Xw16fexecveat(AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32execve(char32_t const *path, char32_t *const argv[], char32_t *const envp[]) { libc_Xw32fexecveat(AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16execvp(char16_t const *file, char16_t *const argv[]) { libc_Xw16fexecvpeat(file,argv,libc_Xget_w16environ(),0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32execvp(char32_t const *file, char32_t *const argv[]) { libc_Xw32fexecvpeat(file,argv,libc_Xget_w32environ(),0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16execvpe(char16_t const *file, char16_t *const argv[], char16_t *const envp[]) { libc_Xw16fexecvpeat(file,argv,envp,0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32execvpe(char32_t const *file, char32_t *const argv[], char32_t *const envp[]) { libc_Xw32fexecvpeat(file,argv,envp,0); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16fexecv(int exec_fd, char16_t *const argv[]) { libc_Xw16fexecveat(exec_fd,libc_empty_string16,argv,libc_Xget_w16environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32fexecv(int exec_fd, char32_t *const argv[]) { libc_Xw32fexecveat(exec_fd,libc_empty_string32,argv,libc_Xget_w32environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16fexecve(int exec_fd, char16_t *const argv[], char16_t *const envp[]) { libc_Xw16fexecveat(exec_fd,libc_empty_string16,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32fexecve(int exec_fd, char32_t *const argv[], char32_t *const envp[]) { libc_Xw32fexecveat(exec_fd,libc_empty_string32,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16fexecvat(fd_t dfd, char16_t const *path, char16_t *const argv[], int flags) { libc_Xw16fexecveat(dfd,path,argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32fexecvat(fd_t dfd, char32_t const *path, char32_t *const argv[], int flags) { libc_Xw32fexecveat(dfd,path,argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw16fexecvpat(char16_t const *file, char16_t *const argv[], int flags) { libc_Xw16fexecvpeat(file,argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL libc_Xw32fexecvpat(char32_t const *file, char32_t *const argv[], int flags) { libc_Xw32fexecvpeat(file,argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16spawnv(int mode, char16_t const *path, char16_t *const argv[]) { return libc_Xw16fspawnveat(mode,AT_FDCWD,path,argv,libc_Xget_w16environ(),0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32spawnv(int mode, char32_t const *path, char32_t *const argv[]) { return libc_Xw32fspawnveat(mode,AT_FDCWD,path,argv,libc_Xget_w32environ(),0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16spawnve(int mode, char16_t const *path, char16_t *const argv[], char16_t *const envp[]) { return libc_Xw16fspawnveat(mode,AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32spawnve(int mode, char32_t const *path, char32_t *const argv[], char32_t *const envp[]) { return libc_Xw32fspawnveat(mode,AT_FDCWD,path,argv,envp,0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16spawnvp(int mode, char16_t const *file, char16_t *const argv[]) { return libc_Xw16fspawnvpeat(mode,file,argv,libc_Xget_w16environ(),0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32spawnvp(int mode, char32_t const *file, char32_t *const argv[]) { return libc_Xw32fspawnvpeat(mode,file,argv,libc_Xget_w32environ(),0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16spawnvpe(int mode, char16_t const *file, char16_t *const argv[], char16_t *const envp[]) { return libc_Xw16fspawnvpeat(mode,file,argv,envp,0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32spawnvpe(int mode, char32_t const *file, char32_t *const argv[], char32_t *const envp[]) { return libc_Xw32fspawnvpeat(mode,file,argv,envp,0); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16fspawnv(int mode, int exec_fd, char16_t *const argv[]) { return libc_Xw16fspawnveat(mode,exec_fd,libc_empty_string16,argv,libc_Xget_w16environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32fspawnv(int mode, int exec_fd, char32_t *const argv[]) { return libc_Xw32fspawnveat(mode,exec_fd,libc_empty_string32,argv,libc_Xget_w32environ(),AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16fspawnve(int mode, int exec_fd, char16_t *const argv[], char16_t *const envp[]) { return libc_Xw16fspawnveat(mode,exec_fd,libc_empty_string16,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32fspawnve(int mode, int exec_fd, char32_t *const argv[], char32_t *const envp[]) { return libc_Xw32fspawnveat(mode,exec_fd,libc_empty_string32,argv,envp,AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16fspawnvat(int mode, fd_t dfd, char16_t const *path, char16_t *const argv[], int flags) { return libc_Xw16fspawnveat(mode,dfd,path,argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32fspawnvat(int mode, fd_t dfd, char32_t const *path, char32_t *const argv[], int flags) { return libc_Xw32fspawnveat(mode,dfd,path,argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw16fspawnvpat(int mode, char16_t const *file, char16_t *const argv[], int flags) { return libc_Xw16fspawnvpeat(mode,file,argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL libc_Xw32fspawnvpat(int mode, char32_t const *file, char32_t *const argv[], int flags) { return libc_Xw32fspawnvpeat(mode,file,argv,libc_Xget_w32environ(),flags); }


EXPORT(__SYMw16(Xwexecl),libc_Xw16execl);
EXPORT(__SYMw32(Xwexecl),libc_Xw32execl);
EXPORT(__SYMw16(Xwexecle),libc_Xw16execle);
EXPORT(__SYMw32(Xwexecle),libc_Xw32execle);
EXPORT(__SYMw16(Xwexeclp),libc_Xw16execlp);
EXPORT(__SYMw32(Xwexeclp),libc_Xw32execlp);
EXPORT(__SYMw16(Xwexeclpe),libc_Xw16execlpe);
EXPORT(__SYMw32(Xwexeclpe),libc_Xw32execlpe);
EXPORT(__SYMw16(Xwfexecl),libc_Xw16fexecl);
EXPORT(__SYMw32(Xwfexecl),libc_Xw32fexecl);
EXPORT(__SYMw16(Xwfexecle),libc_Xw16fexecle);
EXPORT(__SYMw32(Xwfexecle),libc_Xw32fexecle);
EXPORT(__SYMw16(Xwfexeclat),libc_Xw16fexeclat);
EXPORT(__SYMw32(Xwfexeclat),libc_Xw32fexeclat);
EXPORT(__SYMw16(Xwfexecleat),libc_Xw16fexecleat);
EXPORT(__SYMw32(Xwfexecleat),libc_Xw32fexecleat);
EXPORT(__SYMw16(Xwfexeclpat),libc_Xw16fexeclpat);
EXPORT(__SYMw32(Xwfexeclpat),libc_Xw32fexeclpat);
EXPORT(__SYMw16(Xwfexeclpeat),libc_Xw16fexeclpeat);
EXPORT(__SYMw32(Xwfexeclpeat),libc_Xw32fexeclpeat);
EXPORT(__SYMw16(Xwspawnl),libc_Xw16spawnl);
EXPORT(__SYMw32(Xwspawnl),libc_Xw32spawnl);
EXPORT(__SYMw16(Xwspawnle),libc_Xw16spawnle);
EXPORT(__SYMw32(Xwspawnle),libc_Xw32spawnle);
EXPORT(__SYMw16(Xwspawnlp),libc_Xw16spawnlp);
EXPORT(__SYMw32(Xwspawnlp),libc_Xw32spawnlp);
EXPORT(__SYMw16(Xwspawnlpe),libc_Xw16spawnlpe);
EXPORT(__SYMw32(Xwspawnlpe),libc_Xw32spawnlpe);
EXPORT(__SYMw16(Xwfspawnl),libc_Xw16fspawnl);
EXPORT(__SYMw32(Xwfspawnl),libc_Xw32fspawnl);
EXPORT(__SYMw16(Xwfspawnle),libc_Xw16fspawnle);
EXPORT(__SYMw32(Xwfspawnle),libc_Xw32fspawnle);
EXPORT(__SYMw16(Xwfspawnlat),libc_Xw16fspawnlat);
EXPORT(__SYMw32(Xwfspawnlat),libc_Xw32fspawnlat);
EXPORT(__SYMw16(Xwfspawnleat),libc_Xw16fspawnleat);
EXPORT(__SYMw32(Xwfspawnleat),libc_Xw32fspawnleat);
EXPORT(__SYMw16(Xwfspawnlpat),libc_Xw16fspawnlpat);
EXPORT(__SYMw32(Xwfspawnlpat),libc_Xw32fspawnlpat);
EXPORT(__SYMw16(Xwfspawnlpeat),libc_Xw16fspawnlpeat);
EXPORT(__SYMw32(Xwfspawnlpeat),libc_Xw32fspawnlpeat);
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16execl(char16_t const *path, char16_t const *args, ...) { CAPTURE_ARGV(); libc_Xw16execve(path,(char16_t **)argv,libc_Xget_w16environ()); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32execl(char32_t const *path, char32_t const *args, ...) { CAPTURE_ARGV(); libc_Xw32execve(path,(char32_t **)argv,libc_Xget_w32environ()); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16execle(char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xw16execve(path,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32execle(char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xw32execve(path,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16execlp(char16_t const *file, char16_t const *args, ...) { CAPTURE_ARGV(); libc_Xw16execvpe(file,(char16_t **)argv,libc_Xget_w16environ()); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32execlp(char32_t const *file, char32_t const *args, ...) { CAPTURE_ARGV(); libc_Xw32execvpe(file,(char32_t **)argv,libc_Xget_w32environ()); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16execlpe(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xw16execvpe(file,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32execlpe(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xw32execvpe(file,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16fexecl(int exec_fd, char16_t const *args, ...) { CAPTURE_ARGV(); libc_Xw16fexecve(exec_fd,(char16_t **)argv,libc_Xget_w16environ()); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32fexecl(int exec_fd, char32_t const *args, ...) { CAPTURE_ARGV(); libc_Xw32fexecve(exec_fd,(char32_t **)argv,libc_Xget_w32environ()); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16fexecle(int exec_fd, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xw16fexecve(exec_fd,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32fexecle(int exec_fd, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); libc_Xw32fexecve(exec_fd,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16fexeclat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); libc_Xw16fexecveat(dfd,path,(char16_t **)argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32fexeclat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); libc_Xw32fexecveat(dfd,path,(char32_t **)argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16fexecleat(fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); libc_Xw16fexecveat(dfd,path,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32fexecleat(fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); libc_Xw32fexecveat(dfd,path,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16fexeclpat(char16_t const *file, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); libc_Xw16fexecvpeat(file,(char16_t **)argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32fexeclpat(char32_t const *file, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); libc_Xw32fexecvpeat(file,(char32_t **)argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw16fexeclpeat(char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); libc_Xw16fexecvpeat(file,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void ATTR_CDECL libc_Xw32fexeclpeat(char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); libc_Xw32fexecvpeat(file,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16spawnl(int mode, char16_t const *path, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_Xw16spawnve(mode,path,(char16_t **)argv,libc_Xget_w16environ()); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32spawnl(int mode, char32_t const *path, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_Xw32spawnve(mode,path,(char32_t **)argv,libc_Xget_w32environ()); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16spawnle(int mode, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xw16spawnve(mode,path,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32spawnle(int mode, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xw32spawnve(mode,path,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16spawnlp(int mode, char16_t const *file, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_Xw16spawnvpe(mode,file,(char16_t **)argv,libc_Xget_w16environ()); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32spawnlp(int mode, char32_t const *file, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_Xw32spawnvpe(mode,file,(char32_t **)argv,libc_Xget_w32environ()); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16spawnlpe(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xw16spawnvpe(mode,file,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32spawnlpe(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xw32spawnvpe(mode,file,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16fspawnl(int mode, int exec_fd, char16_t const *args, ...) { CAPTURE_ARGV(); return libc_Xw16fspawnve(mode,exec_fd,(char16_t **)argv,libc_Xget_w16environ()); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32fspawnl(int mode, int exec_fd, char32_t const *args, ...) { CAPTURE_ARGV(); return libc_Xw32fspawnve(mode,exec_fd,(char32_t **)argv,libc_Xget_w32environ()); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16fspawnle(int mode, int exec_fd, char16_t const *args, ... /*, char16_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xw16fspawnve(mode,exec_fd,(char16_t **)argv,(char16_t **)envp); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32fspawnle(int mode, int exec_fd, char32_t const *args, ... /*, char32_t *const envp[] */) { CAPTURE_ARGV_ENVP(); return libc_Xw32fspawnve(mode,exec_fd,(char32_t **)argv,(char32_t **)envp); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16fspawnlat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_Xw16fspawnveat(mode,dfd,path,(char16_t **)argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32fspawnlat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_Xw32fspawnveat(mode,dfd,path,(char32_t **)argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16fspawnleat(int mode, fd_t dfd, char16_t const *path, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_Xw16fspawnveat(mode,dfd,path,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32fspawnleat(int mode, fd_t dfd, char32_t const *path, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_Xw32fspawnveat(mode,dfd,path,(char32_t **)argv,(char32_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16fspawnlpat(int mode, char16_t const *file, char16_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_Xw16fspawnvpeat(mode,file,(char16_t **)argv,libc_Xget_w16environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32fspawnlpat(int mode, char32_t const *file, char32_t const *args, ... /*, int flags*/) { CAPTURE_ARGV_FLAGS(); return libc_Xw32fspawnvpeat(mode,file,(char32_t **)argv,libc_Xget_w32environ(),flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw16fspawnlpeat(int mode, char16_t const *file, char16_t const *args, ... /*, char16_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_Xw16fspawnvpeat(mode,file,(char16_t **)argv,(char16_t **)envp,flags); }
CRT_WIDECHAR_EXCEPT pid_t ATTR_CDECL libc_Xw32fspawnlpeat(int mode, char32_t const *file, char32_t const *args, ... /*, char32_t *const envp[], int flags*/) { CAPTURE_ARGV_ENVP_FLAGS(); return libc_Xw32fspawnvpeat(mode,file,(char32_t **)argv,(char32_t **)envp,flags); }


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
EXPORT_STRONG(__read,libc_read);
EXPORT_STRONG(__write,libc_write);
EXPORT_STRONG(__pread64,libc_pread64);
EXPORT_STRONG(__pwrite64,libc_pwrite64);
EXPORT_STRONG(__sysconf,libc_sysconf);
EXPORT_STRONG(__select,libc_select);
EXPORT_STRONG(__poll,libc_poll);
EXPORT_STRONG(__pipe,libc_pipe);
EXPORT_STRONG(__open,libc_open);
EXPORT_STRONG(__lseek,libc_lseek);
EXPORT_STRONG(__libc_pread,libc_pread);
EXPORT_STRONG(__libc_pwrite,libc_pwrite);
EXPORT_STRONG(__getpagesize,libc_getpagesize);
EXPORT_STRONG(__fcntl,libc_fcntl);
EXPORT_STRONG(__dup2,libc_dup2);
EXPORT_STRONG(__close,libc_close);


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
