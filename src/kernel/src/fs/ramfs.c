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
#ifndef GUARD_KERNEL_SRC_FS_RAMFS_C
#define GUARD_KERNEL_SRC_FS_RAMFS_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/align.h>
#include <hybrid/section.h>
#include <kos/types.h>
#include <kernel/malloc.h>
#include <kernel/debug.h>
#include <fs/device.h>
#include <fs/node.h>
#include <fs/ramfs.h>
#include <fs/driver.h>
#include <dev/wall.h>
#include <except.h>
#include <assert.h>

DECL_BEGIN

INTDEF struct inode_operations RamFs_DeviceOperations;
INTDEF struct inode_operations RamFs_SymlinkOperations;
INTDEF struct inode_operations RamFs_RegularOperations;
INTDEF struct inode_operations RamFs_DirectoryOperations;

struct superblock_data {
    ino_t sd_next_ino; /* Next INode number to allocate. */
};

PRIVATE ino_t KCALL
RamFs_AllocIno(struct superblock *__restrict self) {
 ino_t result = self->s_fsdata->sd_next_ino;
 /* TODO: Need a real INode number allocator. */
 self->s_fsdata->sd_next_ino = result+1;
 return result;
}
PRIVATE void KCALL
RamFs_FreeIno(struct superblock *__restrict self, ino_t ino) {
 /* TODO: Need a real INode number allocator. */
}




INTERN struct inode_operations RamFs_DeviceOperations = {
};
INTERN struct inode_operations RamFs_SymlinkOperations = {
};
INTERN struct inode_operations RamFs_RegularOperations = {
};



PRIVATE void KCALL
RamFs_Creat(struct directory_node *__restrict target_directory,
            struct directory_entry *__restrict target_dirent,
            struct regular_node *__restrict new_node) {
 target_dirent->de_pos          = 0;
 target_dirent->de_ino          = RamFs_AllocIno(target_directory->d_node.i_super);
 new_node->re_node.i_flags     |= INODE_FPERSISTENT;
 new_node->re_node.i_attr.a_ino = target_dirent->de_ino;
 new_node->re_node.i_nlink      = 1;
 new_node->re_node.i_ops        = &RamFs_RegularOperations;
}

PRIVATE void KCALL
RamFs_MkDir(struct directory_node *__restrict target_directory,
            struct directory_entry *__restrict target_dirent,
            struct directory_node *__restrict new_directory) {
 target_dirent->de_pos              = 0;
 target_dirent->de_ino              = RamFs_AllocIno(target_directory->d_node.i_super);
 new_directory->d_node.i_flags     |= INODE_FPERSISTENT|INODE_FDIRLOADED;
 new_directory->d_node.i_attr.a_ino = target_dirent->de_ino;
 new_directory->d_node.i_nlink      = 1;
 new_directory->d_node.i_ops        = &RamFs_DirectoryOperations;
}

PRIVATE void KCALL
RamFs_Symlink(struct directory_node *__restrict target_directory,
              struct directory_entry *__restrict target_dirent,
              struct symlink_node *__restrict link_node) {
 target_dirent->de_pos              = 0;
 target_dirent->de_ino              = RamFs_AllocIno(target_directory->d_node.i_super);
 link_node->sl_node.i_flags        |= INODE_FPERSISTENT;
 link_node->sl_node.i_attr.a_ino    = target_dirent->de_ino;
 link_node->sl_node.i_attr.a_blocks = CEILDIV(link_node->sl_node.i_attr.a_size,512);
 link_node->sl_node.i_nlink         = 1;
 link_node->sl_node.i_ops           = &RamFs_SymlinkOperations;
}

PRIVATE void KCALL
RamFs_MkNod(struct directory_node *__restrict target_directory,
            struct directory_entry *__restrict target_dirent,
            struct inode *__restrict device_node) {
 target_dirent->de_pos     = 0;
 target_dirent->de_ino     = RamFs_AllocIno(target_directory->d_node.i_super);
 device_node->i_flags     |= INODE_FPERSISTENT;
 device_node->i_attr.a_ino = target_dirent->de_ino;
 device_node->i_nlink      = 1;
 device_node->i_ops        = &RamFs_DeviceOperations;
}
PRIVATE void KCALL
RamFs_Link(struct directory_node *__restrict target_directory,
           struct directory_entry *__restrict target_dirent,
           struct inode *__restrict link_target) {
 target_dirent->de_pos = 0;
}
PRIVATE void KCALL
RamFs_Unlink(struct directory_node *__restrict containing_directory,
             struct directory_entry *__restrict containing_entry,
             struct inode *__restrict node_to_unlink) {
 if (ATOMIC_DECFETCH(node_to_unlink->i_nlink) == 0)
     RamFs_FreeIno(containing_directory->d_node.i_super,
                   node_to_unlink->i_attr.a_ino);
}
PRIVATE void KCALL
RamFs_RmDir(struct directory_node *__restrict containing_directory,
            struct directory_entry *__restrict containing_entry,
            struct directory_node *__restrict node_to_unlink) {
 asserte(ATOMIC_DECFETCH(node_to_unlink->d_node.i_nlink) == 0);
 RamFs_FreeIno(containing_directory->d_node.i_super,
               node_to_unlink->d_node.i_attr.a_ino);
}

INTERN struct inode_operations RamFs_DirectoryOperations = {
    .io_directory = {
        .d_creat   = &RamFs_Creat,
        .d_mkdir   = &RamFs_MkDir,
        .d_symlink = &RamFs_Symlink,
        .d_mknod   = &RamFs_MkNod,
        .d_link    = &RamFs_Link,
        .d_unlink  = &RamFs_Unlink,
        .d_rmdir   = &RamFs_RmDir,
    }
};



PRIVATE void KCALL
RamFs_Open(struct superblock *__restrict self,
           UNCHECKED USER char *args) {
 /* Setup the root INode as a directory node. */
 self->s_root->d_node.i_attr.a_ino = 0;
 self->s_root->d_node.i_ops        = &RamFs_DirectoryOperations;
 self->s_root->d_node.i_nlink      = 1;
 self->s_root->d_node.i_flags     |= INODE_FATTRLOADED|INODE_FPERSISTENT|INODE_FDIRLOADED;
 self->s_fsdata = (struct superblock_data *)kmalloc(sizeof(struct superblock_data),
                                                    GFP_SHARED);
 self->s_fsdata->sd_next_ino = 1;
 self->s_flags |= SUPERBLOCK_FNOBLOCKS;
}


PRIVATE void KCALL
RamFs_Fini(struct superblock *__restrict self) {
 kfree(self->s_fsdata);
}

PRIVATE void KCALL
RamFs_OpenNode(struct inode *__restrict UNUSED(node),
               struct directory_node *__restrict UNUSED(parent_directory),
               struct directory_entry *__restrict UNUSED(parent_directory_entry)) {
 error_throw(E_NO_SUCH_OBJECT);
}


PUBLIC struct superblock_type ramfs_type = {
    .st_name   = "ramfs",
    .st_flags  = SUPERBLOCK_TYPE_FNODEV,
    .st_driver = &this_driver,
    .st_open   = &RamFs_Open,
    .st_functions = {
        .f_fini     = &RamFs_Fini,
        .f_opennode = &RamFs_OpenNode,
    },
};
DEFINE_FILESYSTEM_TYPE(ramfs_type);

/* Allocate a new, fully initialized RAM filesystem superblock. */
PUBLIC REF struct superblock *KCALL ramfs_alloc(void) {
 return superblock_open(&null_device,&ramfs_type,NULL);
}


PRIVATE struct directory_node devfs_root = {
    .d_node = {
        .i_refcnt = 1,
        .i_ops    = &RamFs_DirectoryOperations,
        .i_super  = &devfs,
        .i_lock   = RWLOCK_INIT,
        .i_nlink  = 1,
        .i_flags  = INODE_FATTRLOADED|INODE_FDIRLOADED|INODE_FPERSISTENT,
        .i_fsdata = NULL,
        .i_attr   = {
            .a_ino    = 0,
            .a_mode   = S_IFDIR|0755,
            .a_uid    = 0,
            .a_gid    = 0,
            .a_size   = 0,
            .a_blocks = 0,
            .a_atime  = {0,0},
            .a_mtime  = {0,0},
            .a_ctime  = {0,0}
        }
    },
    .d_dirend    = 0,
    .d_size      = 0,
    .d_mask      = DIRECTORY_DEFAULT_MASK,
    .d_map       = NULL, /* Allocated by the driver initializer below. */
    .d_bypos     = NULL,
    .d_bypos_end = NULL
};
PRIVATE struct superblock_data devfs_data = {
    .sd_next_ino = 1
};

PUBLIC struct superblock devfs = {
    .s_refcnt       = 1,
    .s_type         = &devfs_type,
    .s_root         = &devfs_root,
    .s_device       = &null_device,
    .s_driver       = &this_driver,
    .s_wall         = &wall_kernel,
    .s_lock         = ATOMIC_RWLOCK_INIT,
    .s_flags        = SUPERBLOCK_FNOBLOCKS,
    .s_nodesc       = 0,
    .s_nodesm       = SUPERBLOCK_DEFAULT_NODES_MASK,
    .s_nodesv       = NULL, /* Allocated by the driver initializer below. */
    .s_nodes_lock   = ATOMIC_RWLOCK_INIT,
    .s_changed      = NULL,
    .s_changed_lock = ATOMIC_RWLOCK_INIT,
    .s_mount        = NULL,
    .s_mount_lock   = ATOMIC_RWLOCK_INIT,
    .s_fsdata       = &devfs_data
};


PUBLIC struct superblock_type devfs_type = {
    .st_name      = "devfs",
    .st_flags     = (SUPERBLOCK_TYPE_FNODEV|SUPERBLOCK_TYPE_FSINGLE),
    .st_driver    = &this_driver,
    .st_singleton = &devfs,
    .st_functions = {
        .f_fini     = &RamFs_Fini,
        .f_opennode = &RamFs_OpenNode,
    },
};


DEFINE_DRIVER_PREINIT(devfs_initialize);
PRIVATE ATTR_USED ATTR_FREETEXT void KCALL devfs_initialize(void) {
 /* Allocate dynamically allocated caches. */
 devfs_root.d_map = (REF struct directory_entry **)kmalloc((DIRECTORY_DEFAULT_MASK+1)*
                                                            sizeof(REF struct directory_entry *),
                                                            GFP_SHARED|GFP_CALLOC);
 devfs.s_nodesv = (REF LIST_HEAD(struct inode) *)kmalloc((SUPERBLOCK_DEFAULT_NODES_MASK+1)*
                                                          sizeof(LIST_HEAD(struct inode)),
                                                          GFP_SHARED|GFP_CALLOC);
 /* Add the root INode to the INode cache of `devfs' */
 superblock_addnode(&devfs,&devfs_root.d_node);
 register_filesystem_type(&devfs_type);
 superblock_register(&devfs);
}




DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_RAMFS_C */
