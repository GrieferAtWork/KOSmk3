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
#ifndef GUARD_KERNEL_SRC_FS_SYSTEM_C
#define GUARD_KERNEL_SRC_FS_SYSTEM_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/futex.h>
#include <hybrid/minmax.h>
#include <kernel/sections.h>
#include <kernel/syscall.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <kernel/user.h>
#include <fs/node.h>
#include <fs/linker.h>
#include <dev/wall.h>
#include <fs/handle.h>
#include <fs/path.h>
#include <fs/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <except.h>
#include <string.h>
#include <sched/group.h>
#include <sched/async_signal.h>
#include <sched/userstack.h>
#include <sched/posix_signals.h>
#include <bits/signum.h>
#include <bits/waitstatus.h>
#include <kernel/environ.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <hybrid/align.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <alloca.h>
#include <termios.h>
#include <linux/fs.h>
#include <kos/keymap.h>
#include <kos/keyboard-ioctl.h>
#include <linux/msdos_fs.h>
#include <sched/pertask-arith.h>

/* FS System calls. */

DECL_BEGIN

#define throw_fs_error(fs_error_code) \
        __EXCEPT_INVOKE_THROW_NORETURN(throw_fs_error(fs_error_code))
PRIVATE __EXCEPT_NORETURN void
(KCALL throw_fs_error)(u16 fs_error_code) {
 struct exception_info *info;
 info = error_info();
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_code                        = E_FILESYSTEM_ERROR;
 info->e_error.e_flag                        = ERR_FNORMAL;
 info->e_error.e_filesystem_error.fs_errcode = fs_error_code;
 error_throw_current();
 __builtin_unreachable();
}


DEFINE_SYSCALL0(sync) {
 superblock_syncall();
 return 0;
}

DEFINE_SYSCALL1(syncfs,fd_t,fd) {
 REF struct superblock *EXCEPT_VAR block;
 block = handle_get_superblock_relaxed(fd);
 TRY {
  superblock_sync(block);
 } FINALLY {
  superblock_decref(block);
 }
 return 0;
}

#ifdef CONFIG_WIDE_64BIT_SYSCALL
#ifdef CONFIG_SYSCALL_ARG64_LOFIRST
DEFINE_SYSCALL5(xftruncateat,
                fd_t,dfd,USER UNCHECKED char const *,path,
                syscall_ulong_t,len_lo,
                syscall_ulong_t,len_hi,
                int,flags)
#else
DEFINE_SYSCALL5(xftruncateat,
                fd_t,dfd,USER UNCHECKED char const *,path,
                syscall_ulong_t,len_hi,
                syscall_ulong_t,len_lo,
                int,flags)
#endif
#else
DEFINE_SYSCALL4(xftruncateat,
                fd_t,dfd,USER UNCHECKED char const *,path,
                u64,length,int,flags)
#endif
{
 REF struct inode *EXCEPT_VAR node;
 REF struct path *p;
 if (flags & ~(FS_MODE_FKNOWNBITS))
     error_throw(E_INVALID_ARGUMENT);
 /* Lookup the user-path. */
 p = fs_pathat(dfd,path,user_strlen(path),
              (struct inode **)&node,FS_ATMODE(flags));
 path_decref(p);
 TRY {
  /* Truncate the INode. */
#ifdef CONFIG_WIDE_64BIT_SYSCALL
  inode_truncate(node,(pos_t)len_hi << 32 | (pos_t)len_lo);
#else
  inode_truncate(node,length);
#endif
 } FINALLY {
  inode_decref(node);
 }
 return 0;
}

DEFINE_SYSCALL4(xfpathconfat,
                fd_t,dfd,USER UNCHECKED char const *,path,
                int,name,int,flags) {
 long int COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct inode *EXCEPT_VAR node;
 REF struct path *p;
 if (flags & ~(FS_MODE_FKNOWNBITS))
     error_throw(E_INVALID_ARGUMENT);
 /* Lookup the user-path. */
 p = fs_pathat(dfd,path,user_strlen(path),
              (struct inode **)&node,FS_ATMODE(flags));
 path_decref(p);
 TRY {
  /* Query information in the INode. */
  result = inode_pathconf(node,name);
 } FINALLY {
  inode_decref(node);
 }
 return result;
}

DEFINE_SYSCALL6(xfrealpathat,fd_t,dfd,
                USER UNCHECKED char const *,path,int,flags,
                USER UNCHECKED char *,buf,size_t,bufsize,
                unsigned int,type) {
 ssize_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct path *EXCEPT_VAR p;
 if (flags & ~(FS_MODE_FKNOWNBITS))
     error_throw(E_INVALID_ARGUMENT);
 
 validate_writable(buf,bufsize);
 /* Apply the filesystem's DOS-mode to the type mask. */
 if (!(THIS_FS->fs_atmask & FS_MODE_FDOSPATH))
  type &= ~(REALPATH_FDOSPATH);
 else if (THIS_FS->fs_atflag & FS_MODE_FDOSPATH)
  type |= (REALPATH_FDOSPATH);
 /* Lookup the user-path. */
 p = fs_pathat(dfd,path,user_strlen(path),
               NULL,FS_ATMODE(flags));
 TRY {
  /* Read the filename of the path. */
  result = path_getname(p,buf,bufsize,type);
 } FINALLY {
  path_decref(p);
 }
 return result;
}

DEFINE_SYSCALL2(getcwd,USER UNCHECKED char *,buf,size_t,bufsize) {
 ssize_t COMPILER_IGNORE_UNINITIALIZED(result);
 REF struct path *EXCEPT_VAR p;
 validate_writable(buf,bufsize);
 p = fs_getcwd();
 TRY {
  /* Read the filename of the path associated with the CWD. */
  result = path_getname(p,buf,bufsize,REALPATH_FPATH);
 } FINALLY {
  path_decref(p);
 }
 return result;
}

DEFINE_SYSCALL2(fstat64,fd_t,fd,
                USER UNCHECKED struct stat64 *,statbuf) {
 REF struct handle EXCEPT_VAR hnd;
 hnd = handle_get(fd);
 TRY {
  /* Query information in the handle. */
  handle_stat(hnd,statbuf);
 } FINALLY {
  handle_decref(hnd);
 }
 return 0;
}

DEFINE_SYSCALL4(fstatat64,fd_t,dfd,USER UNCHECKED char const *,path,
                USER UNCHECKED struct stat64 *,statbuf,int,flags) {
 REF struct inode *EXCEPT_VAR node;
 REF struct path *p;
 if (flags & ~(FS_MODE_FKNOWNBITS))
     error_throw(E_INVALID_ARGUMENT);
 /* Lookup the user-path. */
 flags = FS_ATMODE(flags);
 p = fs_pathat(dfd,path,user_strlen(path),
              (struct inode **)&node,
               flags);
 path_decref(p);
 TRY {
  /* Query information in the INode. */
  inode_stat(node,statbuf);
 } FINALLY {
  inode_decref(node);
 }
 return 0;
}

DEFINE_SYSCALL4(utimensat,fd_t,dfd,USER UNCHECKED char const *,path,
                USER UNCHECKED struct timespec64 *,utimes,int,flags) {
 REF struct inode *EXCEPT_VAR node;
 REF struct path *p;
 if (flags & ~(FS_MODE_FKNOWNBITS|AT_CHANGE_CTIME))
     error_throw(E_INVALID_ARGUMENT);
 /* Lookup the user-path. */
 p = fs_pathat(dfd,path,user_strlen(path),
              (struct inode **)&node,FS_ATMODE(flags));
 path_decref(p);
 TRY {
  struct timespec new_times[3]; /* 0: A; 1: M ; 2: C */
  inode_access(node,W_OK);
  /* Set the new time times. */
  if (utimes) {
   if (flags & AT_CHANGE_CTIME) {
    memcpy(new_times,utimes,3*sizeof(struct timespec));
   } else {
    memcpy(new_times,utimes,2*sizeof(struct timespec));
    new_times[2].tv_nsec = UTIME_OMIT;
   }
   if (new_times[0].tv_nsec == UTIME_NOW ||
       new_times[1].tv_nsec == UTIME_NOW ||
       new_times[2].tv_nsec == UTIME_NOW) {
    struct timespec now = wall_gettime(node->i_super->s_wall);
    if (new_times[0].tv_nsec == UTIME_NOW) new_times[0] = now;
    if (new_times[1].tv_nsec == UTIME_NOW) new_times[1] = now;
    if (new_times[2].tv_nsec == UTIME_NOW) new_times[2] = now;
   }
  } else {
   /* Use NOW for both access & modification. */
   new_times[0] = wall_gettime(node->i_super->s_wall);
   new_times[1] = new_times[0];
  }
  /* Set new file times. */
  inode_chtime(node,
               new_times[0].tv_nsec != UTIME_OMIT ? &new_times[0] : NULL,
               new_times[1].tv_nsec != UTIME_OMIT ? &new_times[1] : NULL,
               new_times[2].tv_nsec != UTIME_OMIT ? &new_times[2] : NULL);
 } FINALLY {
  inode_decref(node);
 }
 return 0;
}

#ifdef CONFIG_WIDE_64BIT_SYSCALL
#ifdef CONFIG_SYSCALL_ARG64_LOFIRST
DEFINE_SYSCALL3(ftruncate,
                fd_t,fd,
                syscall_ulong_t,len_lo,
                syscall_ulong_t,len_hi)
#else
DEFINE_SYSCALL3(ftruncate,
                fd_t,fd,
                syscall_ulong_t,len_hi,
                syscall_ulong_t,len_lo)
#endif
#else
DEFINE_SYSCALL2(ftruncate,fd_t,fd,u64,length)
#endif
{
 REF struct inode *EXCEPT_VAR node;
 /* Lookup the user-path. */
 node = handle_get_inode(fd);
 TRY {
  /* Truncate the INode. */
#ifdef CONFIG_WIDE_64BIT_SYSCALL
  inode_truncate(node,(pos_t)len_hi << 32 | (pos_t)len_lo);
#else
  inode_truncate(node,length);
#endif
 } FINALLY {
  inode_decref(node);
 }
 return 0;
}

DEFINE_SYSCALL2(fchmod,fd_t,fd,mode_t,mode) {
 REF struct inode *EXCEPT_VAR node;
 if (mode & ~07777)
     error_throw(E_INVALID_ARGUMENT);
 node = handle_get_inode(fd);
 TRY {
  /* Change the file mode. */
  inode_chmod(node,0,mode);
 } FINALLY {
  inode_decref(node);
 }
 return 0;
}
DEFINE_SYSCALL4(fchmodat,
                fd_t,dfd,char const *,path,
                mode_t,mode,int,flags) {
 REF struct inode *EXCEPT_VAR node;
 REF struct path *p;
 if (mode & ~07777)
     error_throw(E_INVALID_ARGUMENT);
 if (flags & ~(FS_MODE_FKNOWNBITS))
     error_throw(E_INVALID_ARGUMENT);
 /* Lookup the user-path. */
 p = fs_pathat(dfd,path,user_strlen(path),
              (struct inode **)&node,FS_ATMODE(flags));
 path_decref(p);
 TRY {
  /* Change the file mode. */
  inode_chmod(node,0,mode);
 } FINALLY {
  inode_decref(node);
 }
 return 0;
}

DEFINE_SYSCALL3(fchown,fd_t,fd,uid_t,owner,gid_t,group) {
 REF struct inode *EXCEPT_VAR node;
 node = handle_get_inode(fd);
 TRY {
  /* Change the file owner and group. */
  inode_chown(node,owner,group);
 } FINALLY {
  inode_decref(node);
 }
 return 0;
}
DEFINE_SYSCALL5(fchownat,
                fd_t,dfd,char const *,path,
                uid_t,owner,gid_t,group,int,flags) {
 REF struct inode *EXCEPT_VAR node;
 REF struct path *p;
 if (flags & ~(FS_MODE_FKNOWNBITS))
     error_throw(E_INVALID_ARGUMENT);
 /* Lookup the user-path. */
 p = fs_pathat(dfd,path,user_strlen(path),
              (struct inode **)&node,FS_ATMODE(flags));
 path_decref(p);
 TRY {
  /* Change the file owner and group. */
  inode_chown(node,owner,group);
 } FINALLY {
  inode_decref(node);
 }
 return 0;
}

PRIVATE void KCALL
translate_ioctl_error(int fd, unsigned long cmd, struct handle hnd) {
 struct exception_info *info;
 u16 expected_kind;
 if (error_code() != E_NOT_IMPLEMENTED)
     return;
#define IOCTL_TYPEOF(x) (((x) >> _IOC_TYPESHIFT) & _IOC_TYPEMASK)
 switch (IOCTL_TYPEOF(cmd)) {
 case IOCTL_TYPEOF(TCGETS):
  expected_kind = HANDLE_KIND_FTTY;
  break;
 case IOCTL_TYPEOF(BLKROSET):
  expected_kind = HANDLE_KIND_FBLOCK;
  break;
#undef IOCTL_TYPEOF
 default:
#define IOCTL_MASKOF(x) ((x) & ((_IOC_NRMASK << _IOC_NRSHIFT)|(_IOC_TYPEMASK << _IOC_TYPESHIFT)))
  switch (IOCTL_MASKOF(cmd)) {
  case IOCTL_MASKOF(KEYBOARD_ENABLE_SCANNING):
  case IOCTL_MASKOF(KEYBOARD_DISABLE_SCANNING):
  case IOCTL_MASKOF(KEYBOARD_GET_LEDS):
  case IOCTL_MASKOF(KEYBOARD_SET_LEDS):
  case IOCTL_MASKOF(KEYBOARD_GET_MODE):
  case IOCTL_MASKOF(KEYBOARD_SET_MODE):
  case IOCTL_MASKOF(KEYBOARD_GET_DELAY):
  case IOCTL_MASKOF(KEYBOARD_SET_DELAY):
  case IOCTL_MASKOF(KEYBOARD_LOAD_KEYMAP):
   expected_kind = HANDLE_KIND_FKEYBOARD;
   break;
   /* TODO: Mouse ioctls */
  case IOCTL_MASKOF(VFAT_IOCTL_READDIR_BOTH):
  case IOCTL_MASKOF(VFAT_IOCTL_READDIR_SHORT):
  case IOCTL_MASKOF(FAT_IOCTL_GET_ATTRIBUTES):
  case IOCTL_MASKOF(FAT_IOCTL_SET_ATTRIBUTES):
  case IOCTL_MASKOF(FAT_IOCTL_GET_VOLUME_ID):
   expected_kind = HANDLE_KIND_FFATFS;
   break;
  default: return;
  }
#undef IOCTL_MASKOF
  break;
 }
 info = error_info();
 info->e_error.e_code                    = E_INVALID_HANDLE;
 info->e_error.e_invalid_handle.h_handle = fd;
 info->e_error.e_invalid_handle.h_reason = ERROR_INVALID_HANDLE_FWRONGKIND;
 info->e_error.e_invalid_handle.h_istype = hnd.h_type;
 info->e_error.e_invalid_handle.h_rqtype = hnd.h_type;
 info->e_error.e_invalid_handle.h_rqkind = expected_kind;
}

DEFINE_SYSCALL3(ioctl,fd_t,fd,
                unsigned long,cmd,void *,arg) {
 fd_t EXCEPT_VAR xfd = fd;
 unsigned long EXCEPT_VAR xcmd = cmd;
 struct handle EXCEPT_VAR hnd;
 ssize_t COMPILER_IGNORE_UNINITIALIZED(result);
 hnd = handle_get(fd);
 TRY {
  result = handle_ioctl(hnd,cmd,arg);
 } FINALLY {
  handle_decref(hnd);
  if (FINALLY_WILL_RETHROW)
      translate_ioctl_error(xfd,xcmd,hnd);
 }
 return result;
}
DEFINE_SYSCALL4(xioctlf,fd_t,fd,unsigned long,cmd,oflag_t,flags,void *,arg) {
 struct handle EXCEPT_VAR hnd;
 ssize_t COMPILER_IGNORE_UNINITIALIZED(result);
 hnd = handle_get(fd);
 TRY {
  result = handle_ioctlf(hnd,cmd,arg,flags);
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}
DEFINE_SYSCALL3(fcntl,fd_t,fd,unsigned int,cmd,void *,arg) {
 return handle_fcntl(fd,cmd,arg);
}

DEFINE_SYSCALL5(xfreadlinkat,fd_t,dfd,
                USER UNCHECKED char const *,path,
                USER UNCHECKED char *,buf,
                size_t,bufsize,int,flags) {
 REF struct path *p;
 REF struct symlink_node *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(node);
 size_t filename_length = user_strlen(path);
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 if (flags & ~FS_MODE_FKNOWNBITS)
     error_throw(E_INVALID_ARGUMENT);
 flags = FS_ATMODE(flags);
 p = fs_pathat(dfd,path,filename_length,
              (REF struct inode **)&node,flags|
               FS_MODE_FSYMLINK_NOFOLLOW);
 path_decref(p);
 TRY {
  if (!INODE_ISLNK(&node->sl_node))
       error_throw(E_INVALID_ARGUMENT); /* XXX: Dedicated FS-error? */
  if (symlink_node_load(node)) {
   if (flags & AT_READLINK_REQSIZE) {
    /* Return the required buffer size (including a terminating NUL-character). */
    result = (size_t)node->sl_node.i_attr.a_size+1;
    COMPILER_BARRIER();
    /* Copy the link's text to user-space. */
    memcpy(buf,node->sl_text,MIN(result,bufsize)*sizeof(char));
   } else {
    /* Why couldn't this system call just return the _required_ buffer size?
     * Then it wouldn't be a guessing game over in user-space... */
    result = (size_t)node->sl_node.i_attr.a_size;
    if (result > bufsize)
        result = bufsize;
    COMPILER_BARRIER();
    /* Copy the link's text to user-space. */
    memcpy(buf,node->sl_text,result*sizeof(char));
   }
  } else {
read_dynamic_again:
   rwlock_read(&node->sl_node.i_lock);
   TRY {
    /* Read the text of a dynamic symlink. */
    result = (*node->sl_node.i_ops->io_symlink.sl_readlink_dynamic)(node,
                                                                    buf,
                                                                    bufsize);
   } FINALLY {
    if (rwlock_endread(&node->sl_node.i_lock))
        goto read_dynamic_again;
   }
   if (flags & AT_READLINK_REQSIZE) {
    /* Append a trailing NUL-character. */
    if (result < bufsize)
        buf[result] = '\0';
    /* Include the trailing NUL-characer */
    ++result;
   } else {
    /* Don't include a trailing NUL-character, and return
     * the number of written bytes, rather than what a
     * successive call would require (Ugh...) */
    if (result > bufsize)
        result = bufsize;
   }
  }
 } FINALLY {
  if (FINALLY_WILL_RETHROW)
      error_printf("READLINK FAILED\n");
  inode_decref(&node->sl_node);
 }
 debug_printf("READLINK -> %Id\n",result);
 return result;
}

DEFINE_SYSCALL4(readlinkat,fd_t,dfd,
                USER UNCHECKED char const *,path,
                USER UNCHECKED char *,buf,
                size_t,len) {
 return SYSC_xfreadlinkat(dfd,path,buf,len,0);
}

DEFINE_SYSCALL5(xfmknodat,fd_t,dfd,
                USER UNCHECKED char const *,filename,
                mode_t,mode,dev_t,dev,int,flags) {
 REF struct directory_node *EXCEPT_VAR dir;
 REF struct path *p;
 REF struct inode *node;
 size_t filename_length = user_strlen(filename);
 /* Validate the mode argument. */
 if ((mode & ~(S_IFMT|0777)) ||
    ((mode & S_IFMT) != S_IFREG &&
     (mode & S_IFMT) != S_IFBLK &&
     (mode & S_IFMT) != S_IFCHR))
      error_throw(E_INVALID_ARGUMENT);
 if (flags & ~FS_MODE_FKNOWNBITS)
     error_throw(E_INVALID_ARGUMENT);
 flags = FS_ATMODE(flags);
 p = fs_lastpathat(dfd,&filename,&filename_length,
                  (REF struct inode **)&dir,
                   flags|FS_MODE_FDIRECTORY);
 path_decref(p);
 TRY {
  /* Create a new file in `dir' */
  node = directory_mknod(dir,filename,(u16)filename_length,
                         mode & ~THIS_FS->fs_umask,
                         fs_getuid(),fs_getgid(),dev,
                      !!(flags & FS_MODE_FDOSPATH));
  inode_decref(node);
 } FINALLY {
  inode_decref(&dir->d_node);
 }
 return 0;
}
DEFINE_SYSCALL4(xfmkdirat,fd_t,dfd,USER UNCHECKED char const *,filename,mode_t,mode,int,flags) {
 REF struct directory_node *EXCEPT_VAR dir;
 REF struct directory_node *node;
 REF struct path *p;
 size_t filename_length = user_strlen(filename);
 /* Validate the mode argument. */
 if (mode & ~(0777))
     error_throw(E_INVALID_ARGUMENT);
 if (flags & ~FS_MODE_FKNOWNBITS)
     error_throw(E_INVALID_ARGUMENT);
 flags = FS_ATMODE(flags);
 p = fs_lastpathat(dfd,&filename,&filename_length,
                  (REF struct inode **)&dir,
                   flags|FS_MODE_FDIRECTORY|
                   FS_MODE_FIGNORE_TRAILING_SLASHES);
 path_decref(p);
 TRY {
  /* Create a new sub-directory in `dir' */
  node = directory_mkdir(dir,filename,(u16)filename_length,
                        (mode & ~THIS_FS->fs_umask) & 0777,
                         fs_getuid(),fs_getgid(),
                      !!(flags & FS_MODE_FDOSPATH));
  inode_decref(&node->d_node);
 } FINALLY {
  inode_decref(&dir->d_node);
 }
 return 0;
}
DEFINE_SYSCALL3(unlinkat,fd_t,dfd,USER UNCHECKED char const *,filename,int,flags) {
 REF struct directory_node *EXCEPT_VAR dir;
 REF struct path *EXCEPT_VAR p; unsigned int mode;
 size_t filename_length = user_strlen(filename);
 mode = DIRECTORY_REMOVE_FREGULAR;
 /* Validate the flags argument. */
 if (flags & ~(FS_MODE_FKNOWNBITS|AT_REMOVEDIR|AT_REMOVEREG))
     error_throw(E_INVALID_ARGUMENT);
 flags = FS_ATMODE(flags);
 if (flags & AT_REMOVEDIR)
     mode  = DIRECTORY_REMOVE_FDIRECTORY,
     flags |= FS_MODE_FIGNORE_TRAILING_SLASHES;
 if (flags & AT_REMOVEREG)
     mode |= DIRECTORY_REMOVE_FREGULAR;
 if (flags & FS_MODE_FDOSPATH)
     mode |= DIRECTORY_REMOVE_FNOCASE;
 p = fs_lastpathat(dfd,&filename,&filename_length,
                  (REF struct inode **)&dir,
                   flags|FS_MODE_FDIRECTORY);
 TRY {
  /* Remove the specified filesystem object. */
  directory_remove(dir,filename,(u16)filename_length,
                   directory_entry_hash(filename,(u16)filename_length),
                   mode,p);
 } FINALLY {
  inode_decref(&dir->d_node);
  path_decref(p);
 }
 return 0;
}

DEFINE_SYSCALL4(xfsymlinkat,
                USER UNCHECKED char const *,link_text,
                fd_t,dfd,USER UNCHECKED char const *,filename,int,flags) {
 REF struct directory_node *EXCEPT_VAR dir;
 REF struct symlink_node *node; REF struct path *p;
 size_t link_text_size = user_strlen(link_text);
 size_t filename_length = user_strlen(filename);
 if (flags & ~FS_MODE_FKNOWNBITS)
     error_throw(E_INVALID_ARGUMENT);
 flags = FS_ATMODE(flags);
 p = fs_lastpathat(dfd,&filename,&filename_length,
                  (REF struct inode **)&dir,
                   flags|FS_MODE_FDIRECTORY);
 path_decref(p);
 TRY {
  /* Create the new symbolic link. */
  node = directory_symlink(dir,filename,(u16)filename_length,
                           link_text,link_text_size,
                           fs_getuid(),fs_getgid(),
                          ~THIS_FS->fs_umask,
                        !!(flags & FS_MODE_FDOSPATH));
  inode_decref(&node->sl_node);
 } FINALLY {
  inode_decref(&dir->d_node);
 }
 return 0;
}

DEFINE_SYSCALL5(linkat,
                int,olddfd,USER UNCHECKED char const *,oldname,
                int,newdfd,USER UNCHECKED char const *,newname,
                int,flags) {
 REF struct directory_node *EXCEPT_VAR target_dir;
 REF struct inode *EXCEPT_VAR link_target;
 REF struct path *p;
 size_t newname_length = user_strlen(newname);
 if (flags & ~((FS_MODE_FKNOWNBITS & ~FS_MODE_FSYMLINK_NOFOLLOW) | AT_SYMLINK_FOLLOW))
     error_throw(E_INVALID_ARGUMENT);
 /* Convert linkat()'s special `AT_SYMLINK_FOLLOW' to something we better understand. */
 if (!(flags & AT_SYMLINK_FOLLOW))
       flags |= FS_MODE_FSYMLINK_NOFOLLOW;
 flags = FS_ATMODE(flags);
 p = fs_lastpathat(newdfd,&newname,&newname_length,
                  (REF struct inode **)&target_dir,
                   flags|FS_MODE_FDIRECTORY);
 path_decref(p);
 TRY {
  p = fs_pathat(olddfd,oldname,user_strlen(oldname),(struct inode **)&link_target,
                flags|FS_MODE_FIGNORE_TRAILING_SLASHES);
  path_decref(p);
  TRY {
   /* Create the new hard link. */
   directory_link(target_dir,newname,
                 (u16)newname_length,
                  link_target,
               !!(flags & FS_MODE_FDOSPATH));
  } FINALLY {
   inode_decref(link_target);
  }
 } FINALLY {
  inode_decref(&target_dir->d_node);
 }
 return 0;
}

DEFINE_SYSCALL5(xfrenameat,
                int,olddfd,USER UNCHECKED char const *,oldname,
                int,newdfd,USER UNCHECKED char const *,newname,
                int,flags) {
 REF struct directory_node *EXCEPT_VAR target_dir;
 REF struct directory_node *EXCEPT_VAR source_dir;
 REF struct directory_entry *EXCEPT_VAR source_entry;
 REF struct path *EXCEPT_VAR p;
 size_t oldname_length = user_strlen(oldname);
 size_t newname_length = user_strlen(newname);
 if (flags & ~FS_MODE_FKNOWNBITS)
     error_throw(E_INVALID_ARGUMENT);
 flags = FS_ATMODE(flags);
 p = fs_lastpathat(newdfd,&newname,&newname_length,
                  (REF struct inode **)&target_dir,
                   flags|FS_MODE_FDIRECTORY|
                   FS_MODE_FIGNORE_TRAILING_SLASHES);
 path_decref(p);
 TRY {
  p = fs_lastpathat(olddfd,&oldname,&oldname_length,
                   (struct inode **)&source_dir,
                    flags|FS_MODE_FDIRECTORY|
                    FS_MODE_FIGNORE_TRAILING_SLASHES);
  TRY {
   if (!newname_length) {
    /* Use the filename of the original file if the target filename ends with a slash:
     * >> rename("/bar/baz.txt","/foo/");
     * Same as:
     * >> rename("/bar/baz.txt","/foo/baz.txt");
     */
    newname        = oldname;
    newname_length = oldname_length;
   }
   /* Lookup the directory entry for the source file. */
   source_entry = directory_getentry(source_dir,oldname,oldname_length,
                                     directory_entry_hash(oldname,oldname_length));
   TRY {
    /* Perform the actual rename operation. */
    directory_rename(source_dir,source_entry,
                     target_dir,newname,newname_length);
    /* Try to delete the directory for the (now removed) old filename. */
    path_delchild(p,source_dir,source_entry);
   } FINALLY {
    directory_entry_decref(source_entry);
   }
  } FINALLY {
   inode_decref(&source_dir->d_node);
   path_decref(p);
  }
 } FINALLY {
  inode_decref(&target_dir->d_node);
 }
 return 0;
}



DEFINE_SYSCALL1(dup,fd_t,fd) {
 return handle_dup(fd,0);
}
DEFINE_SYSCALL2(dup2,fd_t,oldfd,fd_t,newfd) {
 handle_dupinto(oldfd,newfd,0);
 return newfd;
}
DEFINE_SYSCALL3(dup3,fd_t,oldfd,fd_t,newfd,oflag_t,flags) {
 flags = IO_HANDLE_FFROM_O(flags);
 if (flags & ~IO_HANDLE_FMASK)
     error_throw(E_INVALID_ARGUMENT);
 handle_dupinto(oldfd,newfd,(iomode_t)flags);
 return newfd;
}

DEFINE_SYSCALL_MUSTRESTART(close);
DEFINE_SYSCALL1(close,fd_t,fd) {
 return handle_close(fd) ? 0 : -EBADF;
}

DEFINE_SYSCALL4(openat,
                fd_t,dfd,char const *,filename,
                oflag_t,flags,mode_t,mode) {
 REF struct inode *EXCEPT_VAR target_node;
 int COMPILER_IGNORE_UNINITIALIZED(result_fd);
 REF struct path *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(path);
 struct handle EXCEPT_VAR result;
 atflag_t at_flags;
 size_t newname_length = user_strlen(filename);
 at_flags = FS_MODE_FNORMAL;
 if (flags & (O_SYMLINK|O_NOFOLLOW)) {
  /* You can't combine both flags! */
  if ((flags & (O_SYMLINK|O_NOFOLLOW)) == (O_SYMLINK|O_NOFOLLOW))
       error_throw(E_INVALID_ARGUMENT);
  at_flags |= FS_MODE_FSYMLINK_NOFOLLOW;
 }
 if (flags & O_DIRECTORY) at_flags |= (FS_MODE_FDIRECTORY|FS_MODE_FIGNORE_TRAILING_SLASHES);
 if (flags & O_DOSPATH)   at_flags |= FS_MODE_FDOSPATH;
 at_flags = FS_ATMODE(at_flags);
 if (flags & O_CREAT) {
  if (mode & ~0777)
      error_throw(E_INVALID_ARGUMENT);
  /* Lookup the within which to create/open a file. */
  path = fs_lastpathat(dfd,
                      &filename,
                      &newname_length,
                      (struct inode **)&target_node,
                       at_flags);
  if (!(flags & (O_SYMLINK|O_EXCL))) {
   /* TODO: If the target file is a symbolic link, traverse that link! */
  }
  TRY {
   /* Create a new file (NOTE: `O_EXCL' is handled by `file_creat()') */
   result = file_creat((struct directory_node *)target_node,path,filename,
                       (u16)newname_length,flags,fs_getuid(),fs_getgid(),
                        mode & ~THIS_FS->fs_umask);
  } FINALLY {
   inode_decref(target_node);
   path_decref(path);
  }
 } else {
  path = fs_pathat(dfd,filename,newname_length,
                  (REF struct inode **)&target_node,
                   at_flags);
  if ((flags & O_NOFOLLOW) &&
       INODE_ISLNK(target_node)) {
   inode_decref(target_node);
   path_decref(path);
   throw_fs_error(ERROR_FS_IS_A_SYMLINK);
  }

  if (flags & O_PATH) {
   /* Open the path as a handle, not the INode itself. */
   result.h_mode = HANDLE_MODE(HANDLE_TYPE_FPATH,IO_FROM_O(flags));
   result.h_object.o_path = path;
   inode_decref(target_node);
  } else {
   TRY {
    /* Open a file stream. */
    result = file_open(target_node,path,flags);
   } FINALLY {
    inode_decref(target_node);
    path_decref(path);
   }
  }
 }
 TRY {
  /* Truncate the generated handle if the caller requested us doing so. */
  if (flags & O_TRUNC)
      handle_truncate(result,0);
  /* With a handle now opened, turn it into a descriptor. */
  result_fd = handle_put(result);
 } FINALLY {
  handle_decref(result);
 }
 return result_fd;
}

DEFINE_SYSCALL4(xreaddir,
                fd_t,fd,USER UNCHECKED struct dirent *,buf,
                size_t,bufsize,int,mode) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct handle EXCEPT_VAR hnd;
 hnd = handle_get(fd);
 TRY {
  unsigned int mode_id;
  /* Check for read permissions. */
  if ((hnd.h_flag & IO_ACCMODE) == IO_WRONLY)
       throw_fs_error(ERROR_FS_ACCESS_ERROR);
  /* Check for known flag bits. */
  if (mode & (~READDIR_MODEMASK & ~(READDIR_FLAGMASK)))
      error_throw(E_INVALID_ARGUMENT);
  mode_id = mode & READDIR_MODEMASK;
  if (mode_id == READDIR_MULTIPLE) {
   size_t partial,alignoff;
   result = 0;
   mode &= ~(READDIR_MODEMASK);
#if READDIR_DEFAULT != 0
   mode |= READDIR_DEFAULT;
#endif
   for (;;) {
    partial = handle_readdir(hnd,buf,bufsize,mode);
    if (!partial) {
     /* Append an EOF directory entry. */
     if ((mode & READDIR_WANTEOF) && result != 0 &&
         (bufsize >= COMPILER_OFFSETOF(struct dirent,d_name)+1)) {
      buf->d_namlen  = 0;
      buf->d_name[0] = '\0';
      result += COMPILER_OFFSETOF(struct dirent,d_name)+1;
     }
     break; /* End of directory. */
    }
    if (partial > bufsize) {
     /* User-space buffer has been used up.
      * If this is the first entry that was read, return its required size. */
     if (!result) result = partial;
     break;
    }
    /* Move the buffer past this entry. */
    *(uintptr_t *)&buf += partial;
    bufsize            -= partial;
    result             += partial;
    /* Align the buffer by INodes (8 bytes). */
    alignoff = (uintptr_t)buf & (sizeof(ino64_t)-1);
    if (alignoff) {
     alignoff = sizeof(ino64_t)-alignoff;
     if (bufsize < alignoff) break;
     *(uintptr_t *)&buf += alignoff;
     bufsize            -= alignoff;
     result             += alignoff;
    }
   }
  } else {
   if (mode_id > READDIR_MODEMAX)
       error_throw(E_INVALID_ARGUMENT);
   result = handle_readdir(hnd,buf,bufsize,mode);
  }
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}

DEFINE_SYSCALL5(xreaddirf,
                fd_t,fd,USER UNCHECKED struct dirent *,buf,
                size_t,bufsize,int,mode,oflag_t,flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct handle EXCEPT_VAR hnd;
 hnd = handle_get(fd);
 TRY {
  /* Check for read permissions. */
  if ((hnd.h_flag & IO_ACCMODE) == IO_WRONLY ||
      (flags & ~IO_SETFL_MASK))
       throw_fs_error(ERROR_FS_ACCESS_ERROR);
  if (mode == READDIR_MULTIPLE) {
   size_t partial,alignoff;
   result = 0;
   for (;;) {
    partial = handle_readdirf(hnd,buf,bufsize,READDIR_DEFAULT,
                            ((hnd.h_flag & ~IO_SETFL_MASK)| flags));
    if (!partial) break; /* End of directory. */
    if (partial > bufsize) {
     /* User-space buffer has been used up.
      * If this is the first entry that was read, return its required size. */
     if (!result) result = partial;
     break;
    }
    /* Move the buffer past this entry. */
    *(uintptr_t *)&buf += partial;
    bufsize            -= partial;
    result             += partial;
    /* Align the buffer by INodes (8 bytes). */
    alignoff = (uintptr_t)buf & (sizeof(ino64_t)-1);
    if (alignoff) {
     alignoff = sizeof(ino64_t)-alignoff;
     if (bufsize < alignoff) break;
     *(uintptr_t *)&buf += alignoff;
     bufsize            -= alignoff;
    }
   }
  } else {
   result = handle_readdirf(hnd,buf,bufsize,mode,
                          ((hnd.h_flag & ~IO_SETFL_MASK)| flags));
  }
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}

#ifdef CONFIG_WIDE_64BIT_SYSCALL
#ifdef CONFIG_SYSCALL_ARG64_LOFIRST
DEFINE_SYSCALL2_64(xfsmask,u32,mask_lo,u32,mask_hi)
#else
DEFINE_SYSCALL2_64(xfsmask,u32,mask_hi,u32,mask_lo)
#endif
#else
DEFINE_SYSCALL1_64(xfsmask,u64,mask)
#endif
{
 union fs_mask mask;
 union fs_mask result;
#ifdef CONFIG_WIDE_64BIT_SYSCALL
 mask.fs_lo = mask_lo;
 mask.fs_hi = mask_hi;
#else
 mask.fs_mode = mask;
#endif
 mask.fs_atmask &= ~FS_MODE_FALWAYS0MASK;
 mask.fs_atmask |=  FS_MODE_FALWAYS1MASK;
 mask.fs_atflag &= ~FS_MODE_FALWAYS0FLAG;
 mask.fs_atflag |=  FS_MODE_FALWAYS1FLAG;
 result.fs_mode = ATOMIC_XCH(THIS_FS->fs_mode,mask.fs_mode);
 return result.fs_mode;
}

DEFINE_SYSCALL3(xfchdirat,fd_t,dfd,USER UNCHECKED char const *,reldir,int,flags) {
 REF struct path *new_path,*old_path;
 struct fs *my_fs = THIS_FS;
 if (flags & ~FS_MODE_FKNOWNBITS)
     error_throw(E_INVALID_ARGUMENT);
 new_path = fs_pathat(dfd,reldir,user_strlen(reldir),NULL,
                      FS_ATMODE(flags)|FS_MODE_FDIRECTORY|
                      FS_MODE_FIGNORE_TRAILING_SLASHES);
 atomic_rwlock_write(&my_fs->fs_lock);
 old_path = my_fs->fs_cwd;
 my_fs->fs_cwd = new_path;
 atomic_rwlock_endwrite(&my_fs->fs_lock);
 path_decref(old_path);
 return 0;
}
DEFINE_SYSCALL3(read,fd_t,fd,USER UNCHECKED void *,buf,size_t,bufsize) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct handle EXCEPT_VAR hnd = handle_get(fd);
 TRY {
  /* Check for read permissions. */
  if ((hnd.h_flag & IO_ACCMODE) == IO_WRONLY)
       throw_fs_error(ERROR_FS_ACCESS_ERROR);
  result = handle_read(hnd,buf,bufsize);
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}
DEFINE_SYSCALL3(write,fd_t,fd,USER UNCHECKED void const *,buf,size_t,bufsize) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct handle EXCEPT_VAR hnd = handle_get(fd);
 TRY {
  /* Check for write permissions. */
  if ((hnd.h_flag & IO_ACCMODE) == IO_RDONLY)
       throw_fs_error(ERROR_FS_ACCESS_ERROR);
  result = handle_write(hnd,buf,bufsize);
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}
DEFINE_SYSCALL4(xreadf,
                fd_t,fd,USER UNCHECKED void *,buf,
                size_t,bufsize,oflag_t,flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct handle EXCEPT_VAR hnd = handle_get(fd);
 TRY {
  /* Check for read permissions. */
  if ((hnd.h_flag & IO_ACCMODE) == IO_WRONLY ||
      (flags & ~IO_SETFL_MASK))
       throw_fs_error(ERROR_FS_ACCESS_ERROR);
  result = handle_readf(hnd,buf,bufsize,
                       (hnd.h_flag & ~IO_SETFL_MASK)|
                        flags);
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}
DEFINE_SYSCALL4(xwritef,
                fd_t,fd,USER UNCHECKED void const *,buf,
                size_t,bufsize,oflag_t,flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct handle EXCEPT_VAR hnd = handle_get(fd);
 TRY {
  /* Check for read permissions. */
  if ((hnd.h_flag & IO_ACCMODE) == IO_WRONLY ||
      (flags & ~IO_SETFL_MASK))
       throw_fs_error(ERROR_FS_ACCESS_ERROR);
  result = handle_writef(hnd,buf,bufsize,
                        (hnd.h_flag & ~IO_SETFL_MASK)|
                         flags);
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}

#ifdef CONFIG_WIDE_64BIT_SYSCALL
#ifdef CONFIG_SYSCALL_ARG64_LOFIRST
DEFINE_SYSCALL4_64(lseek,fd_t,fd,
                   syscall_slong_t,off_lo,
                   syscall_slong_t,off_hi,
                   int,whence)
#else
DEFINE_SYSCALL4_64(lseek,fd_t,fd,
                   syscall_slong_t,off_hi,
                   syscall_slong_t,off_lo,
                   int,whence)
#endif
#else
DEFINE_SYSCALL3_64(lseek,fd_t,fd,s64,off,int,whence)
#endif
{
 pos_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct handle EXCEPT_VAR hnd = handle_get(fd);
 TRY {
#ifdef CONFIG_WIDE_64BIT_SYSCALL
  result = handle_seek(hnd,
                      (off_t)(((u64)off_hi << 32)|
                               (u64)off_lo),
                       whence);
#else
  result = handle_seek(hnd,off,whence);
#endif
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}

#ifdef CONFIG_WIDE_64BIT_SYSCALL
#ifdef CONFIG_SYSCALL_ARG64_LOFIRST
DEFINE_SYSCALL5(pread64,fd_t,fd,
                USER UNCHECKED void *,buf,size_t,bufsize,
                syscall_ulong_t,pos_lo,
                syscall_ulong_t,pos_hi)
#else
DEFINE_SYSCALL5(pread64,fd_t,fd,
                USER UNCHECKED void *,buf,size_t,bufsize,
                syscall_ulong_t,pos_hi,
                syscall_ulong_t,pos_lo)
#endif
#else
DEFINE_SYSCALL4(pread64,fd_t,fd,
                USER UNCHECKED void *,buf,size_t,bufsize,
                pos_t,pos)
#endif
{
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct handle EXCEPT_VAR hnd = handle_get(fd);
 TRY {
  /* Check for read permissions. */
  if ((hnd.h_flag & IO_ACCMODE) == IO_WRONLY)
       throw_fs_error(ERROR_FS_ACCESS_ERROR);
#ifdef CONFIG_WIDE_64BIT_SYSCALL
  result = handle_pread(hnd,buf,bufsize,
                      ((pos_t)pos_hi << 32)|
                       (pos_t)pos_lo);
#else
  result = handle_pread(hnd,buf,bufsize,pos);
#endif
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}

#ifdef CONFIG_WIDE_64BIT_SYSCALL
#ifdef CONFIG_SYSCALL_ARG64_LOFIRST
DEFINE_SYSCALL5(pwrite64,fd_t,fd,
                USER UNCHECKED void const *,buf,size_t,bufsize,
                syscall_ulong_t,pos_lo,
                syscall_ulong_t,pos_hi)
#else
DEFINE_SYSCALL5(pwrite64,fd_t,fd,
                USER UNCHECKED void const *,buf,size_t,bufsize,
                syscall_ulong_t,pos_hi,
                syscall_ulong_t,pos_lo)
#endif
#else
DEFINE_SYSCALL4(pwrite64,fd_t,fd,
                USER UNCHECKED void const *,buf,size_t,bufsize,
                pos_t,pos)
#endif
{
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct handle EXCEPT_VAR hnd = handle_get(fd);
 TRY {
  /* Check for write permissions. */
  if ((hnd.h_flag & IO_ACCMODE) == IO_RDONLY)
       throw_fs_error(ERROR_FS_ACCESS_ERROR);
#ifdef CONFIG_WIDE_64BIT_SYSCALL
  result = handle_pwrite(hnd,buf,bufsize,
                       ((pos_t)pos_hi << 32)|
                        (pos_t)pos_lo);
#else
  result = handle_pwrite(hnd,buf,bufsize,pos);
#endif
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}


#ifdef CONFIG_WIDE_64BIT_SYSCALL
#ifdef CONFIG_SYSCALL_ARG64_LOFIRST
DEFINE_SYSCALL6(xpreadf64,fd_t,fd,
                USER UNCHECKED void *,buf,size_t,bufsize,
                syscall_ulong_t,pos_lo,
                syscall_ulong_t,pos_hi,
                oflag_t,flags)
#else
DEFINE_SYSCALL6(xpreadf64,fd_t,fd,
                USER UNCHECKED void *,buf,size_t,bufsize,
                syscall_ulong_t,pos_hi,
                syscall_ulong_t,pos_lo,
                oflag_t,flags)
#endif
#else
DEFINE_SYSCALL5(xpreadf64,fd_t,fd,
                USER UNCHECKED void *,buf,size_t,bufsize,
                pos_t,pos,oflag_t,flags)
#endif
{
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct handle EXCEPT_VAR hnd = handle_get(fd);
 TRY {
  /* Check for read permissions. */
  if ((hnd.h_flag & IO_ACCMODE) == IO_WRONLY ||
      (flags & ~IO_SETFL_MASK))
       throw_fs_error(ERROR_FS_ACCESS_ERROR);
#ifdef CONFIG_WIDE_64BIT_SYSCALL
  result = handle_preadf(hnd,buf,bufsize,
                       ((pos_t)pos_hi << 32)|
                        (pos_t)pos_lo,
                        (hnd.h_flag & ~IO_SETFL_MASK)|
                         flags);
#else
  result = handle_preadf(hnd,buf,bufsize,pos,
                        (hnd.h_flag & ~IO_SETFL_MASK)|
                         flags);
#endif
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}

#ifdef CONFIG_WIDE_64BIT_SYSCALL
#ifdef CONFIG_SYSCALL_ARG64_LOFIRST
DEFINE_SYSCALL6(xpwritef64,fd_t,fd,
                USER UNCHECKED void const *,buf,size_t,bufsize,
                syscall_ulong_t,pos_lo,
                syscall_ulong_t,pos_hi,
                oflag_t,flags)
#else
DEFINE_SYSCALL6(xpwritef64,fd_t,fd,
                USER UNCHECKED void const *,buf,size_t,bufsize,
                syscall_ulong_t,pos_hi,
                syscall_ulong_t,pos_lo,
                oflag_t,flags)
#endif
#else
DEFINE_SYSCALL5(xpwritef64,fd_t,fd,
                USER UNCHECKED void const *,buf,size_t,bufsize,
                pos_t,pos,oflag_t,flags)
#endif
{
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct handle EXCEPT_VAR hnd = handle_get(fd);
 TRY {
  /* Check for write permissions. */
  if ((hnd.h_flag & IO_ACCMODE) == IO_RDONLY ||
      (flags & ~IO_SETFL_MASK))
       throw_fs_error(ERROR_FS_ACCESS_ERROR);
#ifdef CONFIG_WIDE_64BIT_SYSCALL
  result = handle_pwritef(hnd,buf,bufsize,
                        ((pos_t)pos_hi << 32)|
                         (pos_t)pos_lo,
                         (hnd.h_flag & ~IO_SETFL_MASK)|
                          flags);
#else
  result = handle_pwritef(hnd,buf,bufsize,pos,
                         (hnd.h_flag & ~IO_SETFL_MASK)|
                          flags);
#endif
 } FINALLY {
  handle_decref(hnd);
 }
 return result;
}


PRIVATE syscall_ulong_t KCALL
do_fsync(fd_t fd, bool data_only) {
 struct handle EXCEPT_VAR hnd = handle_get(fd);
 TRY {
  handle_sync(hnd,data_only);
 } FINALLY {
  handle_decref(hnd);
 }
 return 0;
}

DEFINE_SYSCALL1(fsync,fd_t,fd) {
 return do_fsync(fd,false);
}
DEFINE_SYSCALL1(fdatasync,fd_t,fd) {
 return do_fsync(fd,true);
}

DEFINE_SYSCALL_MUSTRESTART(xgetdrives);
DEFINE_SYSCALL0(xgetdrives) {
 /* Return a bit-set of all mounted DOS drives. */
 u32 result = 0; unsigned int i;
 struct vfs *v = THIS_VFS;
 atomic_rwlock_read(&v->v_drives.d_lock);
 for (i = 0; i < VFS_DRIVECOUNT; ++i) {
  if (v->v_drives.d_drives[i])
      result |= (u32)1 << i;
 }
 atomic_rwlock_endread(&v->v_drives.d_lock);
 return result;
}


PRIVATE void KCALL set_exit_reason(int exitcode) {
 struct exception_info *reason;
 reason = error_info();
 memset(reason->e_error.e_pointers,0,sizeof(reason->e_error.e_pointers));
 reason->e_error.e_flag          = ERR_FNORMAL;
 reason->e_error.e_exit.e_status = __W_EXITCODE(exitcode,0);
}

DEFINE_SYSCALL_MUSTRESTART(exit);
DEFINE_SYSCALL1(exit,int,exitcode) {
 set_exit_reason(exitcode);
 error_info()->e_error.e_code = E_EXIT_THREAD;
 error_throw_current();
 __builtin_unreachable();
}

DEFINE_SYSCALL_MUSTRESTART(exit_group);
DEFINE_SYSCALL1(exit_group,int,exitcode) {
 set_exit_reason(exitcode);
 error_info()->e_error.e_code = E_EXIT_PROCESS;
 error_throw_current();
 __builtin_unreachable();
}



/* Link compatibility system calls. */
DEFINE_SYSCALL4(mknodat,fd_t,dfd,USER UNCHECKED char const *,filename,mode_t,mode,dev_t,dev) {
 return SYSC_xfmknodat(dfd,filename,mode,dev,FS_MODE_FNORMAL);
}
DEFINE_SYSCALL3(mkdirat,fd_t,dfd,USER UNCHECKED char const *,filename,mode_t,mode) {
 return SYSC_xfmkdirat(dfd,filename,mode,FS_MODE_FNORMAL);
}
DEFINE_SYSCALL1(chdir,USER UNCHECKED char const *,reldir) {
 return SYSC_xfchdirat(HANDLE_SYMBOLIC_CWD,reldir,FS_MODE_FNORMAL);
}
DEFINE_SYSCALL1(chroot,USER UNCHECKED char const *,reldir) {
 /* In kos, implemented using open() + dup2() */
 REF struct path *new_path,*old_path;
 struct fs *my_fs = THIS_FS;
 new_path = fs_path(NULL,reldir,user_strlen(reldir),NULL,
                    FS_DEFAULT_ATMODE|FS_MODE_FDIRECTORY|
                    FS_MODE_FIGNORE_TRAILING_SLASHES);
 atomic_rwlock_write(&my_fs->fs_lock);
 old_path = my_fs->fs_root;
 my_fs->fs_root = new_path;
 atomic_rwlock_endwrite(&my_fs->fs_lock);
 path_decref(old_path);
 return 0;
}
DEFINE_SYSCALL1(fchdir,fd_t,fd) {
 /* In kos, implemented using `dup2()' */
 REF struct path *new_path,*old_path;
 struct fs *my_fs = THIS_FS;
 new_path = handle_get_path(fd);
 atomic_rwlock_write(&my_fs->fs_lock);
 old_path = my_fs->fs_cwd;
 my_fs->fs_cwd = new_path;
 atomic_rwlock_endwrite(&my_fs->fs_lock);
 path_decref(old_path);
 return 0;
}
DEFINE_SYSCALL4(renameat,
                int,olddfd,USER UNCHECKED char const *,oldname,
                int,newdfd,USER UNCHECKED char const *,newname) {
 return SYSC_xfrenameat(olddfd,oldname,newdfd,newname,FS_MODE_FNORMAL);
}
DEFINE_SYSCALL3(symlinkat,
                USER UNCHECKED char const *,link_text,
                fd_t,dfd,USER UNCHECKED char const *,filename) {
 return SYSC_xfsymlinkat(link_text,dfd,filename,FS_MODE_FNORMAL);
}
#ifdef CONFIG_WIDE_64BIT_SYSCALL
#ifdef CONFIG_SYSCALL_ARG64_LOFIRST
DEFINE_SYSCALL3(truncate,
                USER UNCHECKED char const *,path,
                syscall_ulong_t,len_lo,
                syscall_ulong_t,len_hi) {
 return SYSC_xftruncateat(AT_FDCWD,path,len_lo,len_hi,FS_MODE_FNORMAL);
}
#else
DEFINE_SYSCALL3(truncate,
                USER UNCHECKED char const *,path,
                syscall_ulong_t,len_hi,
                syscall_ulong_t,len_lo) {
 return SYSC_xftruncateat(AT_FDCWD,path,len_hi,len_lo,FS_MODE_FNORMAL);
}
#endif
#else
DEFINE_SYSCALL2(truncate,USER UNCHECKED char const *,path,u64,length) {
 return SYSC_xftruncateat(AT_FDCWD,path,length,FS_MODE_FNORMAL);
}
#endif


struct exec_args {
    REF struct module                   *ea_module;
    USER UNCHECKED char *USER UNCHECKED *ea_argv;
    USER UNCHECKED char *USER UNCHECKED *ea_envp;
};


PRIVATE void KCALL
exec_user(struct exec_args *__restrict args,
          struct cpu_hostcontext_user *__restrict context,
          unsigned int UNUSED(mode)) {
 struct cpu_hostcontext_user *EXCEPT_VAR xcontext = context;
 REF struct application *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(app);
 REF struct module *EXCEPT_VAR mod;
 USER UNCHECKED char *USER UNCHECKED *argv;
 USER UNCHECKED char *USER UNCHECKED *envp;
 /* Load arguments and free their buffer. */
 mod = args->ea_module;
 argv = args->ea_argv;
 envp = args->ea_envp;
 kfree(args);
 TRY {
  /* Construct a new application. */
  app = application_alloc(APPLICATION_TYPE_FUSERAPP);
  app->a_module = mod; /* Inherit reference. */
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  module_decref(mod);
  error_rethrow();
 }
 TRY {
  REF struct vm_node *EXCEPT_VAR environ_node;
  struct process_environ *COMPILER_IGNORE_UNINITIALIZED(environ_address);
  /* Terminate all other threads running in the current process.
   * NOTE: This function is executed in the context of the process leader. */
  task_exit_secondary_threads(__W_EXITCODE(0,0));

  /* Construct an application environment mapping. */
  environ_node = environ_alloc(argv,envp);
  TRY {
   /* Reset all signal actions of the calling thread to their default disposition. */
   signal_resetexec();
   /* With that out of the way, unmap _everything_ from user-space. */
   vm_unmap_userspace();
   pagedir_syncall();

   /* Update the thread configuration to indicate that stack and segments are gone. */
   PERTASK_AND(this_task.t_flags,~(TASK_FOWNUSERSEG));
   {
    REF struct userstack *stack;
    stack = PERTASK_XCH(_this_user_stack,NULL);
    /* XXX: Re-use the user-space stack object. */
    if (stack) userstack_decref(stack);
   }

   /* Setup the application. */
retry_loadroot:
   TRY {
    application_loadroot(app,
                         DL_OPEN_FGLOBAL,
                         "/lib:/usr/lib" /* XXX: This should be taken from environ. */
                         );
   } CATCH_HANDLED (E_INTERRUPT) {
    task_serve_before_user(xcontext,
                           TASK_USERCTX_TYPE_WITHINUSERCODE);
    goto retry_loadroot;
   }

   /* Fix the protection of the environment
    * node to allow for user-space access. */
   environ_node->vn_prot = PROT_READ|PROT_WRITE;

   /* Map the new environment region. */
retry_loadenv:
   TRY {
    vm_acquire(THIS_VM);
    TRY {
     vm_vpage_t environ_page;
     environ_page = vm_getfree(VM_USERENV_HINT,
                               VM_NODE_SIZE(environ_node),
                               1,
                               0,
                               VM_USERENV_MODE);
     /* Save the address of the new environment table. */
     environ_address = (struct process_environ *)VM_PAGE2ADDR(environ_page);
     PERVM(vm_environ) = environ_address;
     /* Update the ranges of the environment node. */
     environ_node->vn_node.a_vmax -= environ_node->vn_node.a_vmin;
     environ_node->vn_node.a_vmin  = environ_page;
     environ_node->vn_node.a_vmax += environ_page;
     /* Insert + map the new node in user-space. */
     vm_insert_and_activate_node(THIS_VM,environ_node);
    } FINALLY {
     vm_release(THIS_VM);
    }
   } CATCH_HANDLED (E_INTERRUPT) {
    task_serve_before_user(xcontext,
                           TASK_USERCTX_TYPE_WITHINUSERCODE);
    goto retry_loadenv;
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   vm_region_decref_range(environ_node->vn_region,
                          environ_node->vn_start,
                          VM_NODE_SIZE(environ_node));
   vm_region_decref(environ_node->vn_region);
   vm_node_free(environ_node);
   error_rethrow();
  }

retry_environ_relocate:
  TRY {
   /* Relocate the environment table to its proper address. */
   environ_relocate(environ_address);
  } CATCH_HANDLED (E_INTERRUPT) {
   task_serve_before_user(xcontext,
                          TASK_USERCTX_TYPE_WITHINUSERCODE);
   goto retry_environ_relocate;
  }

retry_final_setup:
  TRY {
   struct userstack *stack;
   struct user_task_segment *tls;
   /* The new application has now been loaded.
    * Allocate the user-space task segment and a new stack. */
   task_alloc_userseg();
   tls = PERTASK_GET(this_task.t_userseg);
   set_user_tls_register(tls);
#ifndef CONFIG_NO_DOS_COMPAT
   set_user_tib_register(&tls->ts_tib);
#endif /* !CONFIG_NO_DOS_COMPAT */
   stack = task_alloc_userstack();

   /* Finally, update the user-space CPU context
    * to enter at the new application's entry point. */
   CPU_CONTEXT_IP(*xcontext) = app->a_loadaddr + app->a_module->m_entry;
#ifdef CONFIG_STACK_GROWS_UPWARDS
   CPU_CONTEXT_SP(*xcontext) = VM_PAGE2ADDR(stack->us_pagemin);
#else
   CPU_CONTEXT_SP(*xcontext) = VM_PAGE2ADDR(stack->us_pageend);
#endif
   /* Queue execution of library initializers
    * for all currently loaded modules. */
   vm_apps_initall(xcontext);
  } CATCH_HANDLED (E_INTERRUPT) {
   task_serve_before_user(xcontext,
                          TASK_USERCTX_TYPE_WITHINUSERCODE);
   goto retry_final_setup;
  }

 } FINALLY {
  application_decref(app);
 }
 error_info()->e_error.e_code = E_USER_RESUME;
}


/* Linker functions. */
DEFINE_SYSCALL5(execveat,fd_t,dfd,
                USER UNCHECKED char const *,filename,
                USER UNCHECKED char **,argv,
                USER UNCHECKED char **,envp,int,flags) {
 REF struct module *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(exec_module);
 REF struct path *EXCEPT_VAR exec_path;
 REF struct inode *EXCEPT_VAR exec_node;
 struct exec_args *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(args);
 if (flags & ~(FS_MODE_FKNOWNBITS))
     error_throw(E_INVALID_ARGUMENT);
 /* Load the file that should be executed. */
 exec_path = fs_pathat(dfd,filename,user_strlen(filename),
                      (struct inode **)&exec_node,FS_ATMODE(flags));
 TRY {
  if (!INODE_ISREG(exec_node))
       error_throw(E_NOT_EXECUTABLE);
  /* Check for execute and read permissions. */
  inode_access(exec_node,R_OK|X_OK);
  /* Open a new module under the given path. */
  exec_module = module_open((struct regular_node *)exec_node,exec_path);
 } FINALLY {
  inode_decref(exec_node);
  path_decref(exec_path);
 }
 /* XXX: Copy `argv' and `envp' into kernel-space,
  *      or somehow preserve their memory. */
 TRY {
  args = (struct exec_args *)kmalloc(sizeof(struct exec_args),GFP_SHARED);
  args->ea_module = exec_module;
  args->ea_argv   = argv;
  args->ea_envp   = envp;
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  module_decref(exec_module);
  error_rethrow();
 }
 TRY {
  /* Check if the module actually has an entry point. */
  if (!(exec_module->m_flags & MODULE_FENTRY))
        error_throw(E_NOT_EXECUTABLE);

  /* Send a request to the. */
  if (!task_queue_rpc_user(get_this_process(),(task_user_rpc_t)&exec_user,
                           args,TASK_RPC_SYNC|TASK_RPC_USER))
       error_throw(E_INTERRUPT); /* If the leader has been terminated, we'll be too sooner or later. */
  else {
   /* We shouldn't actually get here because the RPC should have terminated
    * all other threads upon success (which would have included us.) */
   task_serve();
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* If we got here because of the INTERRUPT-exception thrown when a thread
   * tries to send an RPC to itself, then the RPC will have still been scheduled
   * successfully, meaning we must not decref() the module passed to the RPC function. */
  if (error_code() == E_INTERRUPT &&
      get_this_process() == THIS_TASK)
      error_rethrow();
  debug_printf("EXEC FAILED\n");
  /* NOTE: Upon success, `task_queue_rpc_user()' and the `exec_user()' function
   *       call will have inherited a reference to the exec-module. */
  module_decref(exec_module);
  kfree(args);
  error_rethrow();
 }
 return 0;
}

DEFINE_SYSCALL3(execve,
                USER UNCHECKED char const *,filename,
                USER UNCHECKED char **,argv,
                USER UNCHECKED char **,envp) {
 return SYSC_execveat(AT_FDCWD,filename,argv,envp,0);
}

DEFINE_SYSCALL_MUSTRESTART(umask);
DEFINE_SYSCALL1(umask,mode_t,mask) {
 /* Simply exchange the UMASK of the calling thread. */
 return ATOMIC_XCH(THIS_FS->fs_umask,mask & S_IRWXUGO);
}

DEFINE_SYSCALL_DONTRESTART(ppoll);
DEFINE_SYSCALL5(ppoll,
                USER UNCHECKED struct pollfd *,ufds_,size_t,nfds_,
                USER UNCHECKED struct timespec const *,tsp,
                USER UNCHECKED sigset_t const *,sigmask,
                size_t,sigsetsize) {
 USER UNCHECKED struct pollfd *EXCEPT_VAR ufds = ufds_;
 USER UNCHECKED sigset_t const *EXCEPT_VAR xsigmask = sigmask;
 size_t EXCEPT_VAR xsigsetsize = sigsetsize;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 size_t EXCEPT_VAR i;
 size_t EXCEPT_VAR nfds = nfds_;
 sigset_t old_blocking;
 /* */if (!sigmask);
 else if (sigsetsize > sizeof(sigset_t)) {
  error_throw(E_INVALID_ARGUMENT);
 }
 validate_readablem(ufds,nfds,sizeof(*ufds));
 validate_readable_opt(tsp,sizeof(*tsp));
 if (sigmask)
     signal_chmask(sigmask,&old_blocking,sigsetsize,SIGNAL_CHMASK_FBLOCK);
 TRY {
scan_again:
  result = 0;
  /* Clear the channel mask. Individual channels
   * may be re-opened by poll-callbacks as needed. */
  task_channelmask(0);
  for (i = 0; i < nfds; ++i) {
   struct handle EXCEPT_VAR hnd;
   TRY {
    hnd = handle_get(ufds[i].fd);
    TRY {
     unsigned int mask;
     size_t num_connections = task_numconnected();
     mask = handle_poll(hnd,ufds[i].events);
     if (mask) {
      ++result;
      ufds[i].revents = (u16)mask;
     } else if (num_connections == task_numconnected()) {
      /* This handle didn't add any new connections,
       * and neither are any of its states signaled.
       * As a conclusion: this handle doesn't support poll() */
      ufds[i].revents = POLLNVAL;
     } else {

      ufds[i].revents = 0;
     }
    } FINALLY {
     handle_decref(hnd);
    }
   } CATCH_HANDLED (E_INVALID_HANDLE) {
    ufds[i].revents = POLLERR;
   }
  }
  if (result) {
   /* At least one of the handles has been signaled. */
   task_udisconnect();
  } else if (!tsp) {
   task_uwait();
   goto scan_again;
  } else if (task_isconnected()) {
   /* Wait for signals to arrive and scan again. */
   if (task_uwaitfor_tmabs(tsp))
       goto scan_again;
   /* NOTE: If the timeout expires, ZERO(0) is returned. */
  }
 } FINALLY {
  if (xsigmask)
      signal_chmask(&old_blocking,NULL,xsigsetsize,SIGNAL_CHMASK_FBLOCK);
 }
 return result;
}

struct pselect6_sig {
    USER UNCHECKED sigset_t *set;
    size_t                   setsz;
};

DEFINE_SYSCALL_DONTRESTART(pselect6);
DEFINE_SYSCALL6(pselect6,size_t,n,
                USER UNCHECKED fd_set *,inp,
                USER UNCHECKED fd_set *,outp,
                USER UNCHECKED fd_set *,exp,
                USER UNCHECKED struct timespec const *,tsp,
                USER UNCHECKED struct pselect6_sig *,sig) {
 USER UNCHECKED struct pselect6_sig *EXCEPT_VAR xsig = sig;
 unsigned int COMPILER_IGNORE_UNINITIALIZED(result);
 sigset_t old_blocking;
 validate_readable_opt(sig,sizeof(*sig));
 validate_readable_opt(inp,CEILDIV(n,8));
 validate_readable_opt(outp,CEILDIV(n,8));
 validate_readable_opt(exp,CEILDIV(n,8));
 if (sig) {
  if (!sig->set) sig = NULL;
  else {
   signal_chmask(&old_blocking,sig->set,
                  sig->setsz,SIGNAL_CHMASK_FBLOCK);
  }
 }
 TRY {
  unsigned int fd_base;
scan_again:
  result = 0;
  /* Clear the channel mask. Individual channels
   * may be re-opened by poll-callbacks as needed. */
  task_channelmask(0);
  for (fd_base = 0; fd_base < n; fd_base += 8) {
   u8 part_i,part_o,part_e,mask; size_t fd_no;
   size_t part_bits = MIN(n-fd_base,8);
   size_t old_result = result;
   /* Figure out which descriptors we're supposed to wait for. */
   part_i = part_o = part_e = 0;
   if (inp)  part_i = *(u8 *)((byte_t *)inp +(fd_base/8));
   if (outp) part_o = *(u8 *)((byte_t *)outp +(fd_base/8));
   if (exp)  part_e = *(u8 *)((byte_t *)exp +(fd_base/8));
   /* Quick check: If we're not supposed to wait for anything, skip ahead. */
   if (!part_i && !part_o && !part_e) continue;
   for (fd_no = fd_base,mask = 1; part_bits; mask <<= 1,++fd_no,--part_bits) {
    REF struct handle EXCEPT_VAR hnd;
    unsigned int mode = 0;
    if (part_i & mask) mode |= POLLIN|POLLPRI;
    if (part_o & mask) mode |= POLLOUT;
    if (part_e & mask) mode |= POLLERR;
    if (!mode) continue;
    hnd = handle_get(fd_no);
    TRY {
     mode = handle_poll(hnd,mode);
    } FINALLY {
     handle_decref(hnd);
    }
    part_i &= ~mask;
    part_o &= ~mask;
    part_e &= ~mask;
    if (mode & (POLLIN|POLLPRI)) part_i |= mask,++result;
    if (mode & (POLLOUT))        part_o |= mask,++result;
    if (mode & (POLLERR))        part_e |= mask,++result;
   }
   /* Write back changes. */
   if (result) {
    /* Clear out all files that were never signaled. */
    if (!old_result && fd_base != 0) {
     if (inp)  memset(inp,0,fd_base/8);
     if (outp) memset(outp,0,fd_base/8);
     if (exp)  memset(exp,0,fd_base/8);
    }
    /* Update what is now available. */
    if (inp)  *(u8 *)((byte_t *)inp +(fd_base/8))  = part_i;
    if (outp) *(u8 *)((byte_t *)outp +(fd_base/8)) = part_o;
    if (exp)  *(u8 *)((byte_t *)exp +(fd_base/8))  = part_e;
   }
  }
  if (result) {
   /* Disconnect from all connected signals. */
   task_udisconnect();
  } else if (!tsp) {
   task_uwait();
   goto scan_again;
  } else if (task_isconnected()) {
   /* Wait for files to become ready. */
   if (task_uwaitfor_tmabs(tsp))
       goto scan_again;
   /* NOTE: If the timeout expires, ZERO(0) is returned. */
  }
 } FINALLY {
  if (xsig)
      signal_chmask(&old_blocking,NULL,xsig->setsz,SIGNAL_CHMASK_FBLOCK);
 }
 return result;
}


DEFINE_SYSCALL_DONTRESTART(xppoll);
DEFINE_SYSCALL6(xppoll,
                USER UNCHECKED struct pollfd *,ufds_,size_t,nfds_,
                USER UNCHECKED struct pollfutex *,uftx_,size_t,nftx_,
                USER UNCHECKED struct timespec const *,tsp,
                USER UNCHECKED struct pselect6_sig *,sig) {
 USER UNCHECKED sigset_t const *EXCEPT_VAR xsigmask = NULL;
 size_t EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(xsigsetsize);
 USER UNCHECKED struct pollfd *EXCEPT_VAR ufds = ufds_;
 USER UNCHECKED struct pollfutex *EXCEPT_VAR uftx = uftx_;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 size_t EXCEPT_VAR nfds = nfds_;
 size_t EXCEPT_VAR nftx = nftx_;
 size_t EXCEPT_VAR i; sigset_t old_blocking;
 REF struct futex **EXCEPT_VAR futex_vec = NULL;
 size_t EXCEPT_VAR futex_cnt;
 if (sig) {
  validate_readable(sig,sizeof(*sig));
  xsigmask    = sig->set;
  xsigsetsize = sig->setsz;
 }
 /* Must restrict the number of futexes because of the malloca() below. */
 if unlikely(nftx > 0x1000)
    error_throw(E_INVALID_ARGUMENT);

 /* */if (!xsigmask);
 else if (xsigsetsize > sizeof(sigset_t)) {
  error_throw(E_INVALID_ARGUMENT);
 }
 validate_readablem(ufds,nfds,sizeof(*ufds));
 validate_readablem(uftx,nftx,sizeof(*uftx));
 validate_readable_opt(tsp,sizeof(*tsp));
 if (xsigmask)
     signal_chmask(xsigmask,&old_blocking,xsigsetsize,SIGNAL_CHMASK_FBLOCK);
 TRY {
  futex_vec = (REF struct futex **)calloca(nftx*sizeof(REF struct futex *));
  futex_cnt = 0;
scan_again:
  assert(futex_cnt == 0);
  result = 0;
  /* Clear the channel mask. Individual channels
   * may be re-opened by poll-callbacks as needed. */
  task_channelmask(0);
  for (i = 0; i < nfds; ++i) {
   struct handle EXCEPT_VAR hnd;
   TRY {
    hnd = handle_get(ufds[i].fd);
    TRY {
     unsigned int mask;
     size_t num_connections = task_numconnected();
     mask = handle_poll(hnd,ufds[i].events);
     if (mask) {
      ++result;
      ufds[i].revents = (u16)mask;
     } else if (num_connections == task_numconnected()) {
      /* This handle didn't add any new connections,
       * and neither are any of its states signaled.
       * As a conclusion: this handle doesn't support poll() */
      ufds[i].revents = POLLNVAL;
     } else {

      ufds[i].revents = 0;
     }
    } FINALLY {
     handle_decref(hnd);
    }
   } CATCH_HANDLED (E_INVALID_HANDLE) {
    ufds[i].revents = POLLERR;
   }
  }
  for (i = 0; i < nftx; ++i) {
   REF struct futex *EXCEPT_VAR ftx;
   USER UNCHECKED futex_t *uaddr = uftx[i].pf_futex;
   COMPILER_READ_BARRIER();
   TRY {
    validate_writable(uaddr,sizeof(futex_t));
    switch (uftx[i].pf_action) {

    {
     futex_t probe_value;
    case FUTEX_WAIT_BITSET:
    case FUTEX_WAIT_BITSET_GHOST:
     /* Open all the channels described by the futex poll operation. */
     task_openchannel(uftx[i].pf_val3);
    case FUTEX_WAIT:
    case FUTEX_WAIT_GHOST:
     probe_value = (futex_t)uftx[i].pf_val;
     COMPILER_READ_BARRIER();
     if (ATOMIC_READ(*uaddr) != probe_value) {
      uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
      ++result;
     } else {
      /* Lookup and connect to the futex in question. */
      ftx = vm_futex(uaddr);
      COMPILER_BARRIER();
      futex_vec[futex_cnt++] = ftx; /* Inherit reference. */
      COMPILER_BARRIER();
      task_connect_ghost(&ftx->f_sig);

      /* Now that we're connected (and therefor interlocked), repeat the check. */
      COMPILER_READ_BARRIER();
      if (ATOMIC_READ(*uaddr) == probe_value) {
       uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
       ++result;
      }
      COMPILER_BARRIER();
     }
     break;
    } break;

    case FUTEX_LOCK_PI:
     /* A futex lock not held by anyone is indicated by a value equal to ZERO(0) */
     if (ATOMIC_READ(*uaddr) == 0) {
      uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
      ++result;
     } else {
      /* Lookup and connect to the futex in question. */
      ftx = vm_futex(uaddr);
      COMPILER_BARRIER();
      futex_vec[futex_cnt++] = ftx; /* Inherit reference. */
      COMPILER_BARRIER();
      task_connect_ghost(&ftx->f_sig);
      COMPILER_READ_BARRIER();
      for (;;) {
       /* Set the WAITERS bit to ensure that user-space can see that
        * someone is waiting for the futex to become available.
        * If we didn't do this, a user-space mutex implementation might
        * neglect to wake us when the associated lock becomes ready. */
       futex_t word = ATOMIC_READ(*uaddr);
       if (word == 0) {
        uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
        ++result;
        break;
       }
       if (ATOMIC_CMPXCH_WEAK(*uaddr,word,word|FUTEX_WAITERS))
           break;
      }
      COMPILER_BARRIER();
     }
     break;

    {
     futex_t probe_mask;
     futex_t probe_value;
     futex_t enable_bits;
    case FUTEX_WAIT_MASK:
    case FUTEX_WAIT_MASK_GHOST:
     probe_mask  = (futex_t)(uintptr_t)uftx[i].pf_val;
     probe_value = (futex_t)(uintptr_t)uftx[i].pf_uaddr2;
     enable_bits = (futex_t)(uintptr_t)uftx[i].pf_val3;
     COMPILER_READ_BARRIER();
     if ((ATOMIC_READ(*uaddr) & probe_mask) != probe_value) {
      uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
      ++result;
     } else {
      u32 old_value;
      ftx = vm_futex(uaddr);
      COMPILER_BARRIER();
      futex_vec[futex_cnt++] = ftx; /* Inherit reference. */
      COMPILER_BARRIER();
      task_connect_ghost(&ftx->f_sig);
      /* Atomically set bits from val3, while also ensuring that the mask still applies. */
      do if (((old_value = ATOMIC_READ(*uaddr)) & probe_mask) != probe_value) {
       uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
       ++result;
       break;
      } while (!ATOMIC_CMPXCH(*uaddr,old_value,old_value|enable_bits));
     }
    } break;

    {
     futex_t old_value;
     futex_t new_value;
     futex_t *uaddr2;
    case FUTEX_WAIT_CMPXCH:
    case FUTEX_WAIT_CMPXCH_GHOST:
     old_value = (futex_t)uftx[i].pf_val;
     new_value = (futex_t)uftx[i].pf_val3;
     uaddr2    = uftx[i].pf_uaddr2;
     validate_writable_opt(uaddr2,sizeof(*uaddr2));
     COMPILER_READ_BARRIER();
     if (ATOMIC_CMPXCH(*uaddr,old_value,new_value)) {
      if (uaddr2) {
       /* Save the old futex value in the provided uaddr2 */
       ATOMIC_WRITE(*uaddr2,old_value);
       /* Broadcast a futex located at the given address. */
       ftx = vm_getfutex(uaddr2);
       if (ftx) {
        size_t thread_count;
        thread_count = sig_broadcast(&ftx->f_sig);
        futex_decref(ftx);
        COMPILER_BARRIER();
        uftx[i].pf_result = thread_count;
        COMPILER_WRITE_BARRIER();
       }
      }
      uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
      ++result;
     } else {
      u32 real_old_value;
      /* The initial CMPXCH failed. - Connect to the futex at that address. */
      ftx = vm_futex(uaddr);
      COMPILER_BARRIER();
      futex_vec[futex_cnt++] = ftx; /* Inherit reference. */
      COMPILER_BARRIER();
      task_connect_ghost(&ftx->f_sig);
      /* Now that we're connected, try the CMPXCH again, this
       * time storing the old value in uaddr2 (if provided). */
      real_old_value = ATOMIC_CMPXCH_VAL(*uaddr,old_value,new_value);
      if (uaddr2) {
       /* Save the old futex value in the provided uaddr2 */
       ATOMIC_WRITE(*uaddr2,real_old_value);
       /* Broadcast a futex located at the given address. */
       ftx = vm_getfutex(uaddr2);
       if (ftx) {
        size_t thread_count;
        thread_count = sig_broadcast(&ftx->f_sig);
        futex_decref(ftx);
        COMPILER_BARRIER();
        uftx[i].pf_result = thread_count;
        COMPILER_WRITE_BARRIER();
       }
      }
      if (real_old_value == old_value) {
       /* The futex entered the required state while we were connected to it. */
       uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
       ++result;
      }
     }
    } break;

    {
     futex_t old_value;
     futex_t new_value;
     futex_t *uaddr2;
    case FUTEX_WAIT_CMPXCH2:
    case FUTEX_WAIT_CMPXCH2_GHOST:
     old_value = (futex_t)uftx[i].pf_val;
     new_value = (futex_t)uftx[i].pf_val3;
     uaddr2    = uftx[i].pf_uaddr2;
     validate_writable(uaddr2,sizeof(*uaddr2));
     COMPILER_READ_BARRIER();
     if (ATOMIC_CMPXCH(*uaddr,old_value,new_value)) {
      /* Save the old futex value in the provided uaddr2 */
      ATOMIC_WRITE(*uaddr2,old_value);
      /* Broadcast a futex located at the given address. */
      ftx = vm_getfutex(uaddr2);
      if (ftx) {
       size_t thread_count;
       thread_count = sig_broadcast(&ftx->f_sig);
       futex_decref(ftx);
       COMPILER_BARRIER();
       uftx[i].pf_result = thread_count;
       COMPILER_WRITE_BARRIER();
      }
      uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
      ++result;
     } else {
      u32 real_old_value;
      /* The initial CMPXCH failed. - Connect to the futex at that address. */
      ftx = vm_futex(uaddr);
      COMPILER_BARRIER();
      futex_vec[futex_cnt++] = ftx; /* Inherit reference. */
      COMPILER_BARRIER();
      task_connect_ghost(&ftx->f_sig);
      /* Now that we're connected, try the CMPXCH again, this
       * time storing the old value in uaddr2 (if provided). */
      real_old_value = ATOMIC_CMPXCH_VAL(*uaddr,old_value,new_value);
      if (real_old_value == old_value) {
       /* Save the old futex value in the provided uaddr2 */
       ATOMIC_WRITE(*uaddr2,real_old_value);
       /* Broadcast a futex located at the given address. */
       ftx = vm_getfutex(uaddr2);
       if (ftx) {
        size_t thread_count;
        thread_count = sig_broadcast(&ftx->f_sig);
        futex_decref(ftx);
        COMPILER_BARRIER();
        uftx[i].pf_result = thread_count;
        COMPILER_WRITE_BARRIER();
       }
       /* The futex entered the required state while we were connected to it. */
       uftx[i].pf_status = POLLFUTEX_STATUS_AVAILABLE;
       ++result;
      }
     }
    } break;

    default:
     uftx[i].pf_status = POLLFUTEX_STATUS_BADACTION;
     break;
    }
   } CATCH_HANDLED (E_SEGFAULT) {
    uftx[i].pf_status = POLLFUTEX_STATUS_BADFUTEX;
   }
  }
  if (result) {
   /* At least one of the handles has been signaled. */
   task_udisconnect();
  } else if (!tsp) {
   task_uwait();
scan_again_drop_futex:
   /* Drop all saved futex references. */
   while (futex_cnt) {
    --futex_cnt;
    futex_decref(futex_vec[futex_cnt]);
   }
   goto scan_again;
  } else if (task_isconnected()) {
   /* Wait for signals to arrive and scan again. */
   if (task_uwaitfor_tmabs(tsp))
       goto scan_again_drop_futex;
   /* NOTE: If the timeout expires, ZERO(0) is returned. */
  }
 } FINALLY {
  if (futex_vec) {
   /* Cleanup saved futex references. */
   while (futex_cnt--)
      futex_decref(futex_vec[futex_cnt]);
   freea(futex_vec);
  }
  if (xsigmask)
      signal_chmask(&old_blocking,NULL,xsigsetsize,SIGNAL_CHMASK_FBLOCK);
 }
 return result;
}



#ifdef CONFIG_WIDE_64BIT_SYSCALL
#ifdef CONFIG_SYSCALL_ARG64_LOFIRST
DEFINE_SYSCALL6(fallocate,
                fd_t,fd,int,mode,
                syscall_ulong_t,off_lo,
                syscall_ulong_t,off_hi,
                syscall_ulong_t,len_lo,
                syscall_ulong_t,len_hi)
#else
DEFINE_SYSCALL6(fallocate,
                fd_t,fd,int,mode,
                syscall_ulong_t,off_hi,
                syscall_ulong_t,off_lo,
                syscall_ulong_t,len_hi,
                syscall_ulong_t,len_lo)
#endif
#else
DEFINE_SYSCALL4(fallocate,
                fd_t,fd,int,mode,
                syscall_ulong_t,off,
                syscall_ulong_t,len)
#endif
{
 struct handle EXCEPT_VAR hnd;
 hnd = handle_get(fd);
 TRY {
#ifdef CONFIG_WIDE_64BIT_SYSCALL
  handle_allocate(hnd,mode,
                ((u64)off_hi << 32 | (u64)off_lo),
                ((u64)len_hi << 32 | (u64)len_lo));
#else
  handle_allocate(hnd,mode,off,len);
#endif
 } FINALLY {
  handle_decref(hnd);
 }
 return 0;
}


PRIVATE void KCALL
dlinit_rpc(void *arg,
           struct cpu_hostcontext_user *__restrict context,
           unsigned int UNUSED(mode)) {
#if defined(__x86_64__)
 context->c_gpregs.gp_rax = (uintptr_t)arg;
#elif defined(__i386__)
 context->c_gpregs.gp_eax = (uintptr_t)arg;
#else
#error "Unsupported architecture"
#endif
 vm_apps_initall(context);
 error_info()->e_error.e_code = E_USER_RESUME;
}


DEFINE_SYSCALL5(xfdlopenat,
                fd_t,dfd,USER UNCHECKED char const *,filename,
                int,at_flags,int,open_flags,char const *,runpath) {
 void *result;
 REF struct path *EXCEPT_VAR module_path;
 REF struct inode *EXCEPT_VAR module_inode;
 REF struct module *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(mod);
 REF struct application *root_app;
 REF struct application *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(result_app);
 /* Validate arguments. */
 validate_readable_opt(runpath,1);
 if ((at_flags & ~FS_MODE_FKNOWNBITS) ||
     (open_flags & ~DL_OPEN_FMASK))
      error_throw(E_INVALID_ARGUMENT);
 if (!runpath)
      runpath = "/lib:/usr/lib"; /* XXX: Take from environ? */
 at_flags = FS_ATMODE(at_flags);

 /* XXX: Search the library path? */
 module_path = fs_pathat(dfd,filename,user_strlen(filename),
                        (struct inode **)&module_inode,at_flags);
 TRY {
  if (!INODE_ISREG(module_inode))
       error_throw(E_NOT_EXECUTABLE);
  /* Open a module under the given path. */
  mod = module_open((struct regular_node *)module_inode,module_path);
 } FINALLY {
  inode_decref(module_inode);
  path_decref(module_path);
 }
 TRY {
again:
  vm_acquire_read(THIS_VM);
  TRY {
   root_app = vm_apps_primary(THIS_VM);
   if unlikely(!root_app) {
    /* No root application? This seems fishy, but ok... */
    result_app = application_alloc(APPLICATION_TYPE_FUSERAPP);
    TRY {
     module_incref(mod);
     result_app->a_module = mod;
     result_app->a_flags |= APPLICATION_FDYNAMIC;
     application_loadroot(result_app,
                          DL_OPEN_FGLOBAL,
                          runpath);
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     application_decref(result_app);
     error_rethrow();
    }
   } else {
    struct module_patcher EXCEPT_VAR patcher;
    /* Now load the module in the current VM (as a dependency of the root application). */
    patcher.mp_root     = (struct module_patcher *)&patcher;
    patcher.mp_prev     = NULL;
    patcher.mp_app      = root_app;
    patcher.mp_requirec = 0;
    patcher.mp_requirea = 0;
    patcher.mp_requirev = NULL;
    patcher.mp_runpath  = runpath;
    patcher.mp_altpath  = NULL;
    patcher.mp_flags    = open_flags;
    patcher.mp_apptype  = APPLICATION_TYPE_FUSERAPP;
    patcher.mp_appflags = APPLICATION_FDYNAMIC;
    TRY {
     /* Open the module user the new patcher. */
     result_app = patcher_require((struct module_patcher *)&patcher,mod);
     application_incref(result_app);
    } FINALLY {
     patcher_fini((struct module_patcher *)&patcher);
    }
   }
  } FINALLY {
   if (vm_release_read(THIS_VM))
       goto again;
  }
 } FINALLY {
  module_decref(mod);
 }
 /* Got the resulting application! */
 ATOMIC_FETCHINC(result_app->a_loadcnt);
 result = (void *)APPLICATION_MAPBEGIN(result_app);
 application_decref(result_app);
 /* Queue library initializers (s.a. `vm_apps_initall()'). */
 task_queue_rpc_user(THIS_TASK,
                    &dlinit_rpc,
                     result,
                     TASK_RPC_USER|
                     TASK_RPC_SINGLE);
 /* XXX: Don't get here? */
 return (syscall_ulong_t)result;
}


DEFINE_SYSCALL_MUSTRESTART(xdlclose);
DEFINE_SYSCALL1(xdlclose,void *,handle) {
 struct application *EXCEPT_VAR app;
 app = vm_getapp(handle);
 TRY {
  if (ATOMIC_DECIFNOTZERO(app->a_mapcnt) &&
     (app->a_flags & APPLICATION_FDYNAMIC)) {
   /* TODO: recursively find all unreachable (not part of dependencies)
    *       applications and queue their finalizers in user-space.
    *       Once those are executed, return (using a #PF-syscall) to
    *       kernel space and unmap all of their memory.
    *       Then (finally) return back to the caller.
    */
  }
 } FINALLY {
  application_decref(app);
 }
 return 0;
}

DEFINE_SYSCALL2(xdlsym,void *,handle,
                USER UNCHECKED char const *,symbol) {
 struct dl_symbol sym;
 validate_readable(symbol,1);
 if (!handle) {
  sym = vm_apps_dlsym(symbol);
 } else {
  REF struct application *EXCEPT_VAR app;
  app = vm_getapp(handle);
  TRY {
   /* Lookup a symbol within the specified application. */
   sym = application_dlsym(app,symbol);
  } FINALLY {
   application_decref(app);
  }
 }
 /* Check if a symbol was found. */
 if (sym.ds_type == MODULE_SYMBOL_INVALID)
     error_throw(E_NO_SUCH_OBJECT);
 return (syscall_ulong_t)sym.ds_base;
}



PRIVATE void KCALL
dlfini_rpc(void *UNUSED(arg),
           struct cpu_hostcontext_user *__restrict context,
           unsigned int UNUSED(mode)) {
 vm_apps_finiall(context);
 error_info()->e_error.e_code = E_USER_RESUME;
}

DEFINE_SYSCALL0(xdlfini) {
 task_queue_rpc_user(THIS_TASK,
                    &dlfini_rpc,
                     NULL,
                     TASK_RPC_USER|
                     TASK_RPC_SINGLE);
 return 0; /* XXX: Don't get here? */
}


struct callback_enum_data {
    module_callback_t *buf; /* Address of the next buffer slot for a callback. */
    size_t             rem; /* Number of remaining buffer slots. */
    size_t             cnt; /* Total number of requested slots. */
};
PRIVATE void KCALL
callback_enum_func(module_callback_t func,
                   struct callback_enum_data *__restrict data) {
 ++data->cnt;
 if (data->rem) {
  --data->rem;
  *data->buf++ = func;
 }
}




DEFINE_SYSCALL4(xdlmodule_info,USER UNCHECKED void *,handle,
                unsigned int,info_class,
                USER UNCHECKED void *,buf,size_t,bufsize) {
 REF struct application *app;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 validate_writable(buf,bufsize);
 app = vm_getapp(handle);
 TRY {
  switch (info_class) {

  {
   struct module_basic_info *info;
  case MODULE_INFO_CLASS_BASIC:
   result = sizeof(struct module_basic_info);
   if (bufsize < sizeof(struct module_basic_info))
       break;
   info = (struct module_basic_info *)buf;
   info->mi_loadaddr = app->a_loadaddr;
   info->mi_segstart = APPLICATION_MAPBEGIN(app);
   info->mi_segend   = APPLICATION_MAPEND(app);
  } break;

  {
   struct module_state_info *info;
  case MODULE_INFO_CLASS_STATE:
   result = sizeof(struct module_state_info);
   if (bufsize < sizeof(struct module_state_info))
       break;
   info = (struct module_state_info *)buf;
   info->si_mapcnt   = app->a_mapcnt;
   info->si_loadcnt  = app->a_loadcnt;
   info->si_appflags = app->a_flags;
   info->si_modflags = app->a_module->m_flags;
   info->si_entry    = (void *)(app->a_loadaddr + app->a_module->m_entry);
  } break;

  {
   struct module_path_info *info;
  case MODULE_INFO_CLASS_PATH:
   result = sizeof(struct module_path_info);
   if (bufsize < sizeof(struct module_path_info))
       break;
   info = (struct module_path_info *)buf;
   info->pi_loadaddr = app->a_loadaddr;
   if unlikely(!app->a_module->m_path) {
    if (info->pi_pathlen >= 1)
        info->pi_path[0] = '\0';
    info->pi_pathlen = 1;
    break;
   }
   /* Lookup the path name of the module. */
   info->pi_pathlen = path_getname(app->a_module->m_path,info->pi_path,
                                   info->pi_pathlen,info->pi_format);
  } break;

  {
   struct module_symbol_info *info;
   USER UNCHECKED char const *name;
   struct dl_symbol sym;
  case MODULE_INFO_CLASS_SYMBOL:
   result = sizeof(struct module_symbol_info);
   if (bufsize < sizeof(struct module_symbol_info))
       break;
   info = (struct module_symbol_info *)buf;
   name = info->si_name;
   COMPILER_READ_BARRIER();
   validate_readable(name,1);
   /* Lookup a symbol matching the given name. */
   sym = application_dlsym(app,name);
   if (sym.ds_type == MODULE_SYMBOL_INVALID) {
    /* Don't leak kernel data into user-space.
     * (fields other than TYPE are normally undefined for INVALID symbols) */
    memset(&sym,0,sizeof(sym));
    sym.ds_type = MODULE_SYMBOL_INVALID;
   }
   info->si_symbol = sym;
  } break;

  {
   struct module_section_info *info;
   USER UNCHECKED char const *name;
   struct dl_section sect;
  case MODULE_INFO_CLASS_SECTION:
   result = sizeof(struct module_section_info);
   if (bufsize < sizeof(struct module_section_info))
       break;
   info = (struct module_section_info *)buf;
   name = info->si_name;
   COMPILER_READ_BARRIER();
   validate_readable(name,1);
   /* Lookup a symbol matching the given name. */
   sect = application_dlsect(app,name);
   if (sect.ds_size == MODULE_SYMBOL_INVALID) {
    /* Don't leak kernel data into user-space.
     * (fields other than SIZE are normally undefined for INVALID sections) */
    memset(&sect,0,sizeof(sect));
   }
   info->si_section = sect;
  } break;

  {
   struct callback_enum_data data;
   void (KCALL *enum_operator)(struct application *__restrict app,
                               module_enumerator_t func, void *arg);
  case MODULE_INFO_CLASS_INIT:
   enum_operator = app->a_module->m_type->m_enuminit;
   goto do_enum_operator;
  case MODULE_INFO_CLASS_FINI:
   enum_operator = app->a_module->m_type->m_enumfini;
do_enum_operator:
   data.buf = (module_callback_t *)buf;
   data.rem = bufsize / sizeof(module_callback_t);
   data.cnt = 0;
   if (enum_operator) {
    /* Enumerate callbacks. */
    SAFECALL_KCALL_VOID_3(enum_operator,
                          app,
                         (module_enumerator_t)&callback_enum_func,
                         &data);
   }
   result = data.cnt * sizeof(module_callback_t);
  } break;

  {
   size_t i,count;
  case MODULE_INFO_REQUIREMENTS:
   result = app->a_requirec * sizeof(void *);
   /* Figure out how many handles can fit into the the user-space buffer. */
   count  = bufsize / sizeof(void *);
   if (count > app->a_requirec)
       count = app->a_requirec;
   /* Copy handles for module dependencies to user-space. */
   for (i = 0; i < count; ++i) {
    void *module_handle;
    module_handle = (void *)APPLICATION_MAPBEGIN(app->a_requirev[i]);
    ((void **)buf)[i] = module_handle;
   }
  } break;

  default:
   error_throw(E_INVALID_ARGUMENT);
  }
 } FINALLY {
  application_decref(app);
 }
 return result;
}

DEFINE_SYSCALL5(mount,
                USER UNCHECKED char const *,dev_name,
                USER UNCHECKED char const *,dir_name,
                USER UNCHECKED char const *,type,unsigned int,flags,
                USER UNCHECKED void const *,data) {
 REF struct path *EXCEPT_VAR p;
 REF struct path *dev_path;
 REF struct inode *EXCEPT_VAR dev_node;
 if (flags & ~(MS_RDONLY|MS_NOSUID|MS_NODEV|MS_NOEXEC|
               MS_SYNCHRONOUS|MS_REMOUNT|MS_MANDLOCK|
               MS_DIRSYNC|MS_NOATIME|MS_NODIRATIME|
               MS_BIND|MS_MOVE|MS_REC|MS_SILENT|MS_POSIXACL|
               MS_UNBINDABLE|MS_PRIVATE|MS_SLAVE|MS_SHARED|
               MS_RELATIME|MS_I_VERSION|MS_STRICTATIME|
               MS_LAZYTIME|MS_ACTIVE|MS_NOUSER))
     error_throw(E_INVALID_ARGUMENT);

 p = fs_path(NULL,dir_name,user_strlen(dir_name),
             NULL,FS_DEFAULT_ATMODE);
 TRY {
  REF struct superblock *EXCEPT_VAR block = NULL;
  if (flags & MS_BIND) {
   /* Create a virtual, secondary binding of another, existing mounting point. */
   dev_path = fs_path(NULL,dev_name,user_strlen(dev_name),
                     (struct inode **)&dev_node,FS_DEFAULT_ATMODE);
   /* XXX: Isn't there a race condition if `dev_node' is the root node
    *      of some superblock, if that superblock is currently being
    *      destroyed? */
   path_decref(dev_path);
  } else {
   if (!type) {
    REF struct block_device *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(inode_device);
    /* Automatically determine how to mount some superblock. */
    dev_path = fs_path(NULL,dev_name,user_strlen(dev_name),
                       (struct inode **)&dev_node,FS_DEFAULT_ATMODE);
    path_decref(dev_path);
    TRY {
     if (!S_ISBLK(dev_node->i_attr.a_mode))
          error_throw(E_INVALID_ARGUMENT); /* TODO: Loopback devices. */
     inode_device = lookup_block_device(dev_node->i_attr.a_rdev);
    } FINALLY {
     inode_decref(dev_node);
    }
    TRY {
     block = superblock_open(inode_device,NULL,(char *)data);
    } FINALLY {
     block_device_decref(inode_device);
    }
   } else {
    REF struct superblock_type *EXCEPT_VAR fstype;
    fstype = lookup_filesystem_type(type);
    if (!fstype)
         error_throw(E_INVALID_ARGUMENT);
    TRY {
     if (fstype->st_flags & SUPERBLOCK_TYPE_FSINGLE) {
      block = fstype->st_singleton;
      superblock_incref(block);
     } else if (fstype->st_flags & SUPERBLOCK_TYPE_FNODEV) {
      block = superblock_open(&null_device,fstype,(char *)data);
     } else {
      REF struct block_device *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(inode_device);
      dev_path = fs_path(NULL,dev_name,user_strlen(dev_name),
                        (struct inode **)&dev_node,FS_DEFAULT_ATMODE);
      path_decref(dev_path);
      TRY {
       if (!S_ISBLK(dev_node->i_attr.a_mode))
            error_throw(E_INVALID_ARGUMENT); /* TODO: Loopback devices. */
       /* Lookup the pointed-to block device. */
       inode_device = lookup_block_device(dev_node->i_attr.a_rdev);
      } FINALLY {
       inode_decref(dev_node);
      }
      TRY {
       block = superblock_open(inode_device,fstype,(char *)data);
      } FINALLY {
       block_device_decref(inode_device);
      }
     }
    } FINALLY {
     driver_decref(fstype->st_driver);
    }
   }
   if (block->s_refcnt == 1) {
    if (flags & (MS_NOATIME|MS_NODIRATIME))
        block->s_flags &= ~SUPERBLOCK_FDOATIME;
   }
   /* Mount the root node of the newly created superblock. */
   dev_node = &block->s_root->d_node;
   inode_incref(dev_node);
  }
  TRY {
   /* Mount the device node. */
   path_mount(p,dev_node);
  } FINALLY {
   inode_decref(dev_node);
   /* decref() the superbock afterwards! */
   if (block)
       superblock_decref(block);
  }
 } FINALLY {
  path_decref(p);
 }
 return 0;
}

DEFINE_SYSCALL_MUSTRESTART(umount2);
DEFINE_SYSCALL2(umount2,
                USER UNCHECKED char const *,name,
                int,flags) {
 atflag_t atflags; REF struct path *p;
 if (flags & ~(MNT_FORCE|MNT_DETACH|MNT_EXPIRE|UMOUNT_NOFOLLOW))
     error_throw(E_INVALID_ARGUMENT);
 atflags = 0;
 if (flags & UMOUNT_NOFOLLOW) atflags |= AT_SYMLINK_NOFOLLOW;
 p = fs_path(NULL,name,user_strlen(name),NULL,FS_ATMODE(atflags));
 if (!path_umount(p))
      throw_fs_error(ERROR_FS_UNMOUNT_NOTAMOUNT);
 return 0;
}

/* TODO */
#define __NR_faccessat    48
#define __NR_nanosleep    101
#define __NR_gettimeofday 169
#define __NR_settimeofday 170


/* Extended system calls (added by KOS). */
/*
#define __NR_xvirtinfo    0x8000002a
*/


DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_SYSTEM_C */
