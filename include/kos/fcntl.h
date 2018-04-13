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
#ifndef _KOS_FCNTL_H
#define _KOS_FCNTL_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>

__SYSDECL_BEGIN

/* File descriptor type IDs. */
#if __KOS_VERSION__ >= 300
/* NOTE: Keep this in sync with the macros from `/src/kernel/include/fs/handle.h' */
#define HANDLE_TYPE_FNONE                0x0000 /* [NAME("none")]                 No type (everything throws an `E_INVALID_HANDLE' error) */
#define HANDLE_TYPE_FDEVICE              0x0001 /* [NAME("device")]              `struct device' / `struct block_device' `struct character_device' */
#define HANDLE_TYPE_FINODE               0x0002 /* [NAME("inode")]               `struct inode' / `struct regular_node' / `struct directory_node' / `struct symlink_node' */
#define HANDLE_TYPE_FFILE                0x0003 /* [NAME("file")]                `struct file' */
#define HANDLE_TYPE_FSUPERBLOCK          0x0004 /* [NAME("superblock")]          `struct superblock' */
#define HANDLE_TYPE_FDIRECTORY_ENTRY     0x0005 /* [NAME("directory_entry")]     `struct directory_entry' */
#define HANDLE_TYPE_FPATH                0x0006 /* [NAME("path")]                `struct path' / `struct vfs' */
#define HANDLE_TYPE_FFS                  0x0007 /* [NAME("fs")]                  `struct fs' */
#define HANDLE_TYPE_FMODULE              0x0008 /* [NAME("module")]              `struct module' */
#define HANDLE_TYPE_FAPPLICATION         0x0009 /* [NAME("application")]         `struct application' / `struct driver' */
#define HANDLE_TYPE_FTHREAD              0x000a /* [NAME("thread")]              `struct task_weakref' (Don't keep a real reference to prevent the reference loop
                                                 *                                                     `task -> handle_manager -> HANDLE_TYPE_FTHREAD -> task') */
#define HANDLE_TYPE_FVM                  0x000b /* [NAME("vm")]                  `struct vm' */
#define HANDLE_TYPE_FVM_REGION           0x000c /* [NAME("vm_region")]           `struct vm_region' */
#define HANDLE_TYPE_FPIPE                0x000d /* [NAME("pipe")]                `struct pipe' */
#define HANDLE_TYPE_FPIPEREADER          0x000e /* [NAME("pipereader")]          `struct pipereader' */
#define HANDLE_TYPE_FPIPEWRITER          0x000f /* [NAME("pipewriter")]          `struct pipewriter' */
#define HANDLE_TYPE_FCOUNT               0x0010 /* Amount of handle types. */

/* Deprecated names for handle types. */
#define FD_TYPE_NULL     HANDLE_TYPE_FNONE   /* Invalid file descriptor object. (Never returned by `fcntl()') */
#define FD_TYPE_INODE    HANDLE_TYPE_FINODE  /* `inode', `superblock', `device', `blkdev', `chrdev', etc... */
#define FD_TYPE_FILE     HANDLE_TYPE_FFILE   /* `file' (or any sub-class). */
#define FD_TYPE_DENTRY   HANDLE_TYPE_FPATH   /* `dentry'. */
#define FD_TYPE_TASK     HANDLE_TYPE_FTHREAD /* `task'. */
#define FD_TYPE_MMAN     HANDLE_TYPE_FVM     /* `mman'. */
#if 0
#define FD_TYPE_STACK    ...                 /* `stack'. */
#endif
#define FD_TYPE_COUNT    HANDLE_TYPE_FCOUNT
#else
#define FD_TYPE_NULL     0x00 /* Invalid file descriptor object. (Never returned by `fcntl()') */
#define FD_TYPE_INODE    0x01 /* `inode', `superblock', `device', `blkdev', `chrdev', etc... */
#define FD_TYPE_FILE     0x02 /* `file' (or any sub-class). */
#define FD_TYPE_DENTRY   0x03 /* `dentry'. */
#define FD_TYPE_TASK     0x04 /* `task'. */
#define FD_TYPE_MMAN     0x05 /* `mman'. */
#define FD_TYPE_STACK    0x06 /* `stack'. */
/* Future descriptor types are added here. */
#define FD_TYPE_COUNT    0x07
#endif


#define FCNTL_KOS_BEGIN  0x35000
#define FCNTL_KOS(i)    (0x35000+i)
#define FCNTL_KOS_TYPE   FCNTL_KOS(1) /* Return the file-descriptor type (One of `FD_TYPE_*') */


#ifdef __USE_KOS_DEPRECATED
/* Arguments for the `type' argument of the KOS system calls `xfdname()' */
#define FDNAME_PATH      0x0000 /* "/etc/passwd" */
#define FDNAME_HEAD      0x0001 /* "/etc" */
#define FDNAME_TAIL      0x0002 /* "passwd" */

#ifndef __KERNEL__
#ifdef __CRT_KOS
/* NOTE: `xfdname()' follows `getcwd()' semantics by returning a newly
 *        allocated string when `NULL,0' is passed for `buf' and `bufsize',
 *        while `xfdname2()' follows the system call interface exactly
 *        by returning the amount of characters (include \0) in `buf',
 *        or -1 with `errno' set if something went wrong.
 * Long story short. - You'll probably just do this:
 * >> fd_t fd = open("/etc/passwd",O_RDONLY);
 * >> char *name = xfdname(fd,FDNAME_PATH,NULL,0);
 * >> printf("path = %q\n",name); // `path = "/etc/passwd"'
 * >> free(name);
 * >> close(fd);
 * NOTE: These functions have become deprecated in KOS Mk3.
 *       Consider using the more powerful `xfrealpathat()' functions below. */
__LIBC __PORT_KOSONLY __ATTR_DEPRECATED("Use `xfrealpath()' instead") char *(__LIBCCALL xfdname)(__fd_t __fd, unsigned int __type, char *__buf, __size_t __bufsize);
__LIBC __PORT_KOSONLY __ATTR_DEPRECATED("Use `xfrealpath2()' instead") __ssize_t (__LIBCCALL xfdname2)(__fd_t __fd, unsigned int __type, char *__buf, __size_t __bufsize);
#endif /* __CRT_KOS */
#endif /* !__KERNEL__ */
#endif /* __USE_KOS_DEPRECATED */

__SYSDECL_END

#endif /* !_KOS_FCNTL_H */
