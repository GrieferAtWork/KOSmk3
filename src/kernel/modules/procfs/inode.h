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
#ifndef GUARD_KERNEL_MODULES_PROCFS_INODE_H
#define GUARD_KERNEL_MODULES_PROCFS_INODE_H 1

#include <hybrid/compiler.h>
#include <fs/node.h>

DECL_BEGIN

/*
 *  PROCFS INode numbers:
 *  xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx 00000000 xxxxxxxx xxxxxxxx xxxxxxxx
 *  [PID..............................] [CLASS.] [PROCESS OBJECT..........]
 *
 *  PID=0 is used for system global file (e.g. `/proc/self' or `/proc/sys/...')
 *  NOTE: `PID' is relative to the PID namespace that was used to create the PROCFS
 *  
 *  CLASS is used to identify larger object ranges, such as file descriptors.
 *        CLASS=0 is used to generic objects. (It is one of `PROCFS_CLASS_F*')
 */
#define PROCFS_INODE_GTPID(ino)                __CCAST(pid_t)((ino) >> 32)
#define PROCFS_INODE_GTCLS(ino)   __CCAST(u8)((__CCAST(u32)(ino) >> 24) & 0xff)
#define PROCFS_INODE_GTOBJ(ino)                __CCAST(u32)((ino) & 0xffffff)
#define PROCFS_INODE_MKINO(pid,cls,obj)      ((__CCAST(ino_t)(pid) << 32) | (__CCAST(ino_t)(cls) << 24) | __CCAST(ino_t)(obj))

#define PROCFS_CLASS_FNORMAL       0x00   /* [*] Regular files (see object codes below) */
#define PROCFS_CLASS_FFD           0x01   /* [l] File descriptors (`obj' is a handle number; s.a. `/proc/[PID]/fd/*') */

#define PROCFS_INODE               0x0000 /* [d] /proc/ */
#define PROCFS_INODE_CMDLINE       0x0001 /* [-] /proc/cmdline */
#define PROCFS_INODE_SELF          0x0002 /* [l] /proc/self */
#define PROCFS_INODE_THREAD_SELF   0x0003 /* [l] /proc/thread-self */

#define PROCFS_INODE_P             0x0000 /* [d] /proc/[PID]/ */
#define PROCFS_INODE_P_CMDLINE     0x0001 /* [-] /proc/[PID]/cmdline */
#define PROCFS_INODE_P_EXE         0x0002 /* [l] /proc/[PID]/exe */
#define PROCFS_INODE_P_CWD         0x0003 /* [l] /proc/[PID]/cwd */
#define PROCFS_INODE_P_ROOT        0x0004 /* [l] /proc/[PID]/root */
#define PROCFS_INODE_P_ENVIRON     0x0005 /* [-] /proc/[PID]/environ */
#define PROCFS_INODE_P_FD          0x0006 /* [d] /proc/[PID]/fd/ */
#define PROCFS_INODE_P_TASK        0x0007 /* [d] /proc/[PID]/task/ */


struct pidns;
struct superblock_data {
    REF struct pidns  *pf_pidns; /* [1..1][const] The PID namespace within which `/proc' operates.
                                  *               While being mounted, this is set to the PID
                                  *               namespace of the calling thread. */
};


INTDEF struct superblock_type procfs_type;

struct task;
struct thread_pid;
struct vm;
struct application;
struct fs;

/* Lookup a task or some of its sub-components, given it's PID.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_NOT_FOUND:
 *         Translated from `E_PROCESS_EXITED' */
INTDEF ATTR_RETNONNULL REF struct task *KCALL ProcFS_GetTask(struct superblock *__restrict self, pid_t pid);
INTDEF ATTR_RETNONNULL REF struct thread_pid *KCALL ProcFS_GetTaskPid(struct superblock *__restrict self, pid_t pid);
INTDEF ATTR_RETNONNULL REF struct vm *KCALL ProcFS_GetTaskVm(struct superblock *__restrict self, pid_t pid);
INTDEF ATTR_RETNONNULL REF struct application *KCALL ProcFS_GetTaskApp(struct superblock *__restrict self, pid_t pid);
INTDEF ATTR_RETNONNULL REF struct fs *KCALL ProcFS_GetTaskFs(struct superblock *__restrict self, pid_t pid);
INTDEF ATTR_RETNONNULL REF struct handle_manager *KCALL ProcFS_GetTaskHandleManager(struct superblock *__restrict self, pid_t pid);

INTDEF ATTR_NORETURN void KCALL ProcFS_FileNotFound(void);


INTDEF struct inode_operations Iprocfs_text_ro;
INTDEF struct inode_operations Iprocfs_text_rw;
struct procfs_text_ro_data {
    /* `struct inode_data' for `Iprocfs_text_ro' */
    byte_t const *td_base;  /* [const][1..td_size] Base address of data that can be read. */
    size_t        td_size;  /* [const] Total amount of readable data (in bytes). */
};
struct procfs_text_rw_data {
    /* `struct inode_data' for `Iprocfs_text_ro' */
    byte_t       *td_base; /* [const][1..td_size][owned] Base address of read-write data. */
    size_t        td_size; /* [const] Fixed number of read-write bytes. */
};

INTDEF ATTR_RETNONNULL struct inode_data *KCALL
ProcFS_OpenRoText(void const *data, size_t num_bytes);
/* NOTE: `data' is only inherited on success. */
INTDEF ATTR_RETNONNULL struct inode_data *KCALL
ProcFS_OpenRwText(/*inherit(kfree())*/void *data, size_t num_bytes);


INTDEF struct inode_operations Iprocfs_path_link;        /* [l] ... (`node->i_fsdata' is a `REF struct path *'; this link expands to the string of that path) */
INTDEF struct inode_operations Iprocfs_root_dir;         /* /proc */
INTDEF struct inode_operations Iprocfs_self_link;        /* /proc/self */
INTDEF struct inode_operations Iprocfs_thread_self_link; /* /proc/thread-self */
INTDEF struct inode_operations Iprocfs_p_fd_dir;         /* /proc/[PID]/fd/ */
INTDEF struct inode_operations Iprocfs_p_fd_link;        /* /proc/[PID]/fd/xxx */
INTDEF struct inode_operations Iprocfs_p_task_dir;       /* /proc/[PID]/task/ */

DECL_END

#endif /* !GUARD_KERNEL_MODULES_PROCFS_INODE_H */
