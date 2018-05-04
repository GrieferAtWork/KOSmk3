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
#ifndef _KOS_HANDLE_H
#define _KOS_HANDLE_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>

__SYSDECL_BEGIN

/* File descriptor type IDs. */
#if __KOS_VERSION__ >= 300
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
#define HANDLE_TYPE_FSOCKET              0x0010 /* [NAME("socket")]              `struct socket' */
#define HANDLE_TYPE_FCOUNT               0x0011 /* Amount of handle types. */

/* Handle kinds (for use with `ERROR_INVALID_HANDLE_FWRONGKIND') */
#define HANDLE_KIND_FANY      0x0000 /* Any kind of handle was expected (set for reasons other than `ERROR_INVALID_HANDLE_FWRONGKIND') */
#define HANDLE_KIND_FTTY      0x0001 /* A TTY handle was expected. */
#define HANDLE_KIND_FBLOCK    0x0002 /* A block device handle was expected. */
#define HANDLE_KIND_FCHAR     0x0003 /* A character device handle was expected. */
#define HANDLE_KIND_FKEYBOARD 0x0004 /* A keyboard was expected. */
#define HANDLE_KIND_FMOUSE    0x0005 /* A mouse was expected. */
#define HANDLE_KIND_FFATFS    0x1001 /* A handle to a FAT filesystem component was expected. */

#endif

__SYSDECL_END

#endif /* !_KOS_HANDLE_H */
