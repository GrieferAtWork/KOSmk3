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
#ifndef GUARD_KERNEL_MODULES_PROCFS_INODE_TYPES_C
#define GUARD_KERNEL_MODULES_PROCFS_INODE_TYPES_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <fs/node.h>
#include <fs/driver.h>
#include <fs/path.h>
#include <kernel/debug.h>
#include <except.h>
#include <sched/pid.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "inode.h"
#include "stringprinter.h"

DECL_BEGIN

INTERN ATTR_RETNONNULL struct inode_data *KCALL
ProcFS_OpenRoText(void const *data, size_t num_bytes) {
 struct procfs_text_ro_data *result;
 result = (struct procfs_text_ro_data *)kmalloc(sizeof(struct procfs_text_ro_data),
                                                GFP_SHARED);
 result->td_base = (byte_t *)data;
 result->td_size = num_bytes;
 return (struct inode_data *)result;
}

#if 1
DEFINE_INTERN_ALIAS(ProcFS_OpenRwText,ProcFS_OpenRoText);
#else
INTERN ATTR_RETNONNULL struct inode_data *KCALL
ProcFS_OpenRwText(/*inherit(kfree())*/void *data, size_t num_bytes) {
 struct procfs_text_rw_data *result;
 result = (struct procfs_text_rw_data *)kmalloc(sizeof(struct procfs_text_rw_data),
                                                GFP_SHARED);
 result->td_base = (byte_t *)data;
 result->td_size = num_bytes;
 return (struct inode_data *)result;
}
#endif

PRIVATE ATTR_NOTHROW void KCALL
RoTextFile_Fini(struct inode *__restrict self) {
 struct procfs_text_ro_data *data;
 data = (struct procfs_text_ro_data *)self->i_fsdata;
 kfree(data);
}

PRIVATE ATTR_NOTHROW void KCALL
RwTextFile_Fini(struct inode *__restrict self) {
 struct procfs_text_rw_data *data;
 data = (struct procfs_text_rw_data *)self->i_fsdata;
 if unlikely(!data) return;
 kfree(data->td_base);
 kfree(data);
}

PRIVATE size_t KCALL
TextFile_PRead(struct inode *__restrict self,
               CHECKED USER void *buf, size_t bufsize,
               pos_t pos, iomode_t UNUSED(flags)) {
 struct procfs_text_ro_data *data; size_t result;
 data = (struct procfs_text_ro_data *)self->i_fsdata;
 if unlikely(pos >= data->td_size)
    return 0;
 result = MIN(data->td_size-(size_t)pos,bufsize);
 memcpy(buf,data->td_base+(size_t)pos,result);
 return result;
}

PRIVATE size_t KCALL
TextFile_PWrite(struct inode *__restrict self,
                CHECKED USER void const *buf, size_t bufsize,
                pos_t pos, iomode_t UNUSED(flags)) {
 struct procfs_text_rw_data *data; size_t result;
 data = (struct procfs_text_rw_data *)self->i_fsdata;
 if unlikely(pos >= data->td_size)
    return 0;
 result = MIN(data->td_size-(size_t)pos,bufsize);
 memcpy(data->td_base+(size_t)pos,buf,result);
 return result;
}


INTERN struct inode_operations Iprocfs_text_ro = {
    .io_fini = &RoTextFile_Fini,
    .io_file = {
        .f_pread = &TextFile_PRead,
    }
};
INTERN struct inode_operations Iprocfs_text_rw = {
    .io_fini = &RwTextFile_Fini,
    .io_file = {
        .f_pread  = &TextFile_PRead,
        .f_pwrite = &TextFile_PWrite,
    }
};




PRIVATE ATTR_NOTHROW void KCALL
PathLink_Fini(struct inode *__restrict self) {
 REF struct path *p = (REF struct path *)self->i_fsdata;
 if (p) path_decref(p);
}

PRIVATE void KCALL
PathLink_Readlink(struct symlink_node *__restrict self) {
 struct stringprinter printer;
 StringPrinter_Init(&printer,64);
 TRY {
  path_print((struct path *)self->sl_node.i_fsdata,
              &StringPrinter_Print,&printer,
              REALPATH_FPATH);
  self->sl_text = StringPrinter_Pack(&printer);
 } FINALLY {
  StringPrinter_Fini(&printer);
 }
}


/* [l] ... (`node->i_fsdata' is a `REF struct path *';
 *           this link expands to the string of that path) */
INTERN struct inode_operations Iprocfs_path_link = {
    .io_fini = &PathLink_Fini,
    .io_symlink = {
        .sl_readlink = &PathLink_Readlink
    }
};



PRIVATE size_t KCALL
ProcSelf_Readlink(struct symlink_node *__restrict self,
                  USER CHECKED char *buf, size_t bufsize) {
 return snprintf(buf,bufsize,
                 "%u",
                 posix_getpid());
}

INTERN struct inode_operations Iprocfs_self_link = {
    /* /proc/self */
    .io_symlink = {
        .sl_readlink_dynamic = &ProcSelf_Readlink
    }
};


PRIVATE size_t KCALL
ProcThreadSelf_Readlink(struct symlink_node *__restrict self,
                        USER CHECKED char *buf, size_t bufsize) {
 return snprintf(buf,bufsize,
                 "%u/task/%u",
                 posix_getpid(),
                 posix_gettid());
}

INTERN struct inode_operations Iprocfs_thread_self_link = {
    /* /proc/thread-self */
    .io_symlink = {
        .sl_readlink_dynamic = &ProcThreadSelf_Readlink
    }
};


INTERN struct inode_operations Iprocfs_p_root_dir = {};       /* /proc/[PID]/ */
INTERN struct inode_operations Iprocfs_p_fd_dir = {};         /* /proc/[PID]/fd/ */
INTERN struct inode_operations Iprocfs_p_fd_link = {};        /* /proc/[PID]/fd/xxx */
INTERN struct inode_operations Iprocfs_p_task_dir = {};       /* /proc/[PID]/task/ */


DECL_END

#endif /* !GUARD_KERNEL_MODULES_PROCFS_INODE_TYPES_C */
