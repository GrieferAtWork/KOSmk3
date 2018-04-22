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
#ifndef GUARD_KERNEL_INCLUDE_FS_NODE_H
#define GUARD_KERNEL_INCLUDE_FS_NODE_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/timespec.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <sched/rwlock.h>
#include <kernel/sections.h>
#include <fs/iomode.h>
#include <fs/handle.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>

DECL_BEGIN


/* HINT: In filesystems that do not support INodes or hardlinks, you can simply
 *       use the on-disk location of the file's directory entry, or some other
 *       kind of descriptor as INode number. (as is done by the FAT driver)
 * HINT: The KOS filesystem driver will never modify `i_nlink' itself.
 *       Modifying this field is the responsibility of the filesystem driver.
 *       Additional requirements for modifications to `i_nlink' are documented
 *       in individual operator hooks.
 */

struct block_device;
struct device;
struct directory_entry;
struct directory_node;
struct driver;
struct file;
struct inode;
struct regular_node;
struct superblock;
struct symlink_node;
struct wall;
struct path;
struct stat64;

/* Opaque, filesystem-specific INode / superblock data. */
struct inode_data;
struct superblock_data;


#define INODE_FNORMAL           0x0000     /* Normal INode flags. */
#define INODE_FCHANGED          0x0001     /* [lock(i_lock)] The node has changed and is part of the
                                            *                per-superblock chain of changed nodes. */
#define INODE_FATTRLOADED       0x0002     /* [lock(WRITE_ONCE,i_lock)] INode attributes have been read from disk. */
#define INODE_FDONTCACHE        0x1000     /* [const] Don't cache the node and keep it, and other caches referring
                                            *         to it alive only as long as it is being used. */
#define INODE_FPERSISTENT       0x2000     /* [const] This INode is persistent and must not be removed from caches.
                                            *         This flag is used by RAMFS filesystems that store persistent
                                            *         data in INodes that must therefor be kept in RAM (or swap...)
                                            * This flag prevents `superblock_clearcache()' from deleting this INode
                                            * so long as `INODE_FCLOSED' hasn't been set. */
#define INODE_FCLOSED           0x4000     /* [lock(WRITE_ONCE)] The superblock associated with the INode has been closed.
                                            *                    When this flag has been set, any further attempts to interact
                                            *                    with the file will fail the same way they would when `i_nlink'
                                            *                    was ZERO(0). */
#define INODE_FDIRLOADED        0x8000     /* [lock(i_lock)][valid_if(S_ISDIR(a_mode))]
                                            *  The directory map of the INode has been fully loaded. */

#define INODE_ISCLOSED(x) (!(x)->i_nlink || ((x)->i_flags&INODE_FCLOSED))


/* The callback invoked by directory enumerators. */
typedef void (KCALL *directory_enum_callback_t)(char const *__restrict name,
                                                u16 namelen, unsigned char type,
                                                ino_t ino, void *arg);

struct vm_region;
struct inode_operations {
    /* [0..1] Called during `inode_destroy'. */
    /*ATTR_NOTHROW*/ void (KCALL *io_fini)(struct inode *__restrict self);

    /* [1..1][locked(READ(self->i_lock))] Load INode attributes.
     * Upon success, the caller will set `INODE_FATTRLOADED'.
     * NOTE: Nodes that are only ever constructed with the `INODE_FATTRLOADED'
     *       flag already set (often such nodes are filesystem root nodes),
     *       do not need to implement this operator. */
    void (KCALL *io_loadattr)(struct inode *__restrict self);

    /* [0..1][locked(WRITE(self->i_lock))] Save INode attributes.
     * Upon success, the caller will clear `INODE_FATTRLOADED'. */
    void (KCALL *io_saveattr)(struct inode *__restrict self);

    /* [0..1][locked(WRITE(self->i_lock))]
     * Mask INode attributes not supported by the filesystem
     * by restoring their default default as would have been
     * initialized by `io_loadattr()'.
     * When this operator isn't implemented, all attributes
     * are assumed supported by the filesystem.
     * Otherwise, this operator is invoked for `chmod()' and
     * `chown()' in order to check if changes to attributes
     * would become undone once the node is saved.
     * If changes would become undone, this function will do
     * that before they could become visible in user-space,
     * as well as allow the caller to detect those changes and
     * throw an `E_FILESYSTEM_ERROR.ERROR_FS_UNSUPPORTED_FUNCTION'.
     * However, if chmod() or chown() is used to keep previous
     * values (or as with the case of `chmod()', filesystem types
     * like FAT still implement a way of setting a read-only attribute),
     * this function deciphers such uses when the underlying superblock
     * supports attributes only partially. */
    void (KCALL *io_maskattr)(struct inode *__restrict self);

    /* [0..1]
     * Query information about hard limits on various filesystem objects.
     * @param: name: One of `_PC_*' from `<bits/confname.h>'
     * @return: -EINVAL: The given `name' is not implemented by the underlying filesystem.
     * For more details on how this operator is invoked, see the documentation of `inode_pathconf()' */
    long int (KCALL *io_pathconf)(struct inode *__restrict self, int name);

    /* File-related operations. */
    struct PACKED {

        /* [0..1]
         * [locked(READ(self->i_lock))]
         * Read data from the INode and return the actual amount read. */
        size_t (KCALL *f_pread)(struct inode *__restrict self,
                                CHECKED USER void *buf, size_t bufsize,
                                pos_t pos, iomode_t flags);

        /* [0..1]
         * [locked(WRITE(self->i_lock))]
         * Write data to the INode and return the actual amount written. */
        size_t (KCALL *f_pwrite)(struct inode *__restrict self,
                                 CHECKED USER void const *buf, size_t bufsize,
                                 pos_t pos, iomode_t flags);

        /* [0..1]
         * [locked(READ(self->i_lock))]
         * Return a memory region containing data detailing
         * the memory for the given `page_index'.
         * If `page_index' isn't located at the start of the region,
         * save the offset into the region where mappings should start
         * within `*pregion_start'
         * Implementation of this operator is optional, but when implemented,
         * the `xmmap()' is forced to use it, and not create generic file->memory
         * mappings that would otherwise be used.
         * @param: page_index:    The offset from the start of `self' for which a region
         *                        should be returned, such that `page_index' is located within.
         * @param: pregion_start: Upon success, must be filled with the offset into `return'
         *                        where the given `page_index' is located at.
         *                        If the returned region starts where the node itself starts,
         *                        this pointer should be filled with `page_index'
         * @assume(POST(*pregion_start <= page_index));
         * @assume(POST(return->vr_size > *pregion_start)); */
        REF struct vm_region *(KCALL *f_mmap)(struct inode *__restrict self,
                                              /*vm_raddr_t*/uintptr_t page_index,
                                              /*vm_raddr_t*/uintptr_t *__restrict pregion_start);

        /* [0..1]
         * [locked(WRITE(self->i_lock))]
         * Truncate the length of the given INode to a maximum of `new_smaller_size' bytes.
         * The caller is responsible to ensure that `new_smaller_size' is actually smaller
         * than the node's old size (that size being `new_smaller_size' bytes)
         * @assume(new_smaller_size < self->i_attr.a_size);
         * Upon success, the caller will update `self->i_attr.a_size'
         * to be equal to `new_smaller_size', as well as set the `INODE_FCHANGED'
         * flag and add the node to its superblock's changed-list. However it is
         * the responsibility of this function to update `i_attr.a_blocks'.
         * NOTE: When ZERO(0) is passed for `new_smaller_size', this function
         *       must deallocate any blocks allocated for the file and set
         *      `i_attr.a_blocks' to ZERO(0). (These semantics are required
         *       during `unlink()' before removing the last reference to a
         *       regular file; however, empty directories are removed through
         *       use of the `d_rmdir()' operator when applied to the hosting
         *       directory) */
        void (KCALL *f_truncate)(struct inode *__restrict self,
                                 pos_t new_smaller_size);

        /* [0..1]
         * [locked(WRITE(self->i_lock))]
         * Perform an IO-control operation described by `cmd'.
         * @return: * :  Command-dependent return value (propagated to user-space)
         * @param: mode: Permissions and block-behavior that should
         *               be applied to limit allowed functions.
         * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator.
         *                            Should be thrown for unrecognized commands. */
        ssize_t (KCALL *f_ioctl)(struct inode *__restrict self, unsigned long cmd,
                                 USER UNCHECKED void *arg, iomode_t flags);

        /* [0..1] Connect to signals broadcast when `mode' (set of `POLLIN|POLLPRI|POLLOUT|POLLERR')
         * becomes true. Return the set of signals who's associated condition is currently met:
         * >> unsigned int result = 0;
         * >> FOREACH_POLL_MODE(x,mode) {
         * >>     task_connect(GET_SIGNAL_FOR(self,x));
         * >>     if (CONDITION_IS_MET(self,x))
         * >>         result |= x;
         * >> }
         * >> return result;
         * NOTE: When not implemented, use the INode's `i_lock' for read/write,
         *       polling meaning that `POLLIN' and `POLLOUT' are implicitly
         *       supported if this operator is missing. */
        unsigned int (KCALL *f_poll)(struct inode *__restrict self, unsigned int mode);

        /* [0..1] Open a file-stream to the given INode and path.
         * NOTE: When this operator isn't defined, file-io is emulated
         *       using the f_pread / f_pwrite operators above.
         * HINT: To open a file stream, use `file_open*()' or `file_creat()' from <fs/file.h> */
        REF struct handle (KCALL *f_open)(struct inode *__restrict self,
                                          struct path *__restrict p,
                                          oflag_t open_mode);

    } io_file;

    union PACKED {
        /* Directory-related operations. */
        struct PACKED {

            /* [0..1][locked(READ(self->i_lock))]
             * NOTE: This function should not yield entries for `.' and `..', as
             *       those will have automatically been created by the caller.
             * Read a new directory entry from the data stream of `self', starting at
             * data offset `*pentry_pos'. Upon success, return a reference to the newly
             * allocated directory entry and update `*pentry_pos' to point to the next
             * directory entry (which can then be re-used in further calls to this function).
             * When the directory has ended, return `NULL' instead to indicate EOF.
             * @throw: E_WOULDBLOCK: `IO_NONBLOCK' was specified and the operation would have blocked.
             * @throw: E_NO_DATA:     Same as returning `NULL' (accepted to automatically
             *                        handle filesystem corruptions leading to partition/file
             *                        overflows; s.a. `block_device_read()' throwing this error) */
            REF struct directory_entry *(KCALL *d_readdir)(struct directory_node *__restrict self,
                                                           pos_t *__restrict pentry_pos,
                                                           iomode_t flags);

            /* One-shot directory enumeration operators.
             * These can be provided as an alternative when `d_readdir'
             * isn't implemented, and are best suited for dynamic
             * directories such as `/proc'. */
            struct PACKED {
                /* [0..1][locked(READ(self->i_lock))]
                 * Optional operator that can be implemented to
                 * accommodate dynamically allocated directory entries.
                 * When implemented, this operator is used during
                 * path traversion instead of using `d_readdir' for that.
                 * @return: NULL: No file with the given name exists.
                 * @param: mode: Set of `FS_MODE_F*' (Most notably `FS_MODE_FDOSPATH')
                 * @throw: E_SEGFAULT: Failed to access the given `name'.
                 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:   [...]
                 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_NOT_FOUND: [...] (Same as returning NULL) */
                REF struct directory_entry *(KCALL *o_lookup)(struct directory_node *__restrict self,
                                                              CHECKED USER char const *__restrict name,
                                                              u16 namelen, uintptr_t hash, unsigned int mode);
                /* [0..1][locked(READ(self->i_lock))]
                 * Enumerate all entries of a dynamic directory `node':
                 * >> (*callback)("foo",DT_REG,42,arg);
                 * >> (*callback)("bar",DT_REG,43,arg);
                 * >> (*callback)("baz",DT_REG,44,arg); */
                void (KCALL *o_enum)(struct directory_node *__restrict node,
                                     directory_enum_callback_t callback,
                                     void *arg);
            }                      d_oneshot;

            /* [0..1]
             * [locked(WRITE(target_directory->i_lock))]
             * Allocate a new regular (`S_IFREG') INode referred to by a single link
             * that is created within `target_directory', using `target_dirent'
             * as INode directory entry.
             * This function must initialize the following members of `target_dirent':
             *     - de_fsdata (optionally, depending on filesystem implementation)
             *     - de_pos
             *     - de_ino
             * This function must initialize the following members of `new_node':
             *     - i_attr.a_ino (Same as `target_dirent->de_ino')
             *     - i_ops
             *     - i_nlink (To `>= 1')
             *     - i_fsdata (Optionally; pre-initialized to `NULL')
             * During this operation, the new directory entry should be
             * constructed on-disk, or scheduled for such an allocation.
             * NOTE: This function may also choose to set the `INODE_FCHANGED' flag
             *       of `new_node', in which case the caller will add the node to
             *       the set of changed nodes of the accompanying superblock, meaning
             *       that filesystem implementations may choose to lazily set save
             *       attributes of newly constructed nodes to disk once the disk
             *       is supposed to be synchronized.
             * @assume(new_node->re_node.i_refcnt == 1); (Exclusive user)
             * @assume(new_node->re_node.i_flags & INODE_FATTRLOADED);
             * @assume(new_node->re_node.i_super  == target_directory->d_node.i_super);
             * @assume(new_node->re_node.is_attr.a_size == 0);
             * @assume(new_node->re_node.is_attr.a_blocks == 0);
             * @assume(S_ISREG(new_node->i_attr.a_mode));
             * @assume(!directory_getentry(target_directory,target_dirent...));
             * @assume(target_dirent->de_namelen != 0);
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...] (Same as not implementing)
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         [...]
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:            [...]
             * @throw: E_NO_DATA: Translated to `E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL' */
            void (KCALL *d_creat)(struct directory_node *__restrict target_directory,
                                  struct directory_entry *__restrict target_dirent,
                                  struct regular_node *__restrict new_node);

            /* [0..1]
             * [locked(WRITE(target_directory->i_lock))]
             * Allocate a new directory INode referred to by a single link
             * that is created within `target_directory', using `target_dirent'
             * as INode directory entry.
             * This function must initialize the following members of `target_dirent':
             *     - de_fsdata (optionally, depending on filesystem implementation)
             *     - de_pos
             *     - de_ino
             * This function must initialize the following members of `new_directory':
             *     - d_node.i_attr.a_ino (Same as `target_dirent->de_ino')
             *     - d_node.i_ops
             *     - d_node.i_nlink (To `>= 1')
             *     - d_node.i_fsdata (Optionally; pre-initialized to `NULL')
             * During this operation, the new directory entry should be
             * constructed on-disk, or scheduled for such an allocation.
             * NOTE: This function may also choose to set the `INODE_FCHANGED' flag
             *       of `new_directory', in which case the caller will add the node to
             *       the set of changed nodes of the accompanying superblock, meaning
             *       that filesystem implementations may choose to lazily set save
             *       attributes of newly constructed nodes to disk once the disk
             *       is supposed to be synchronized.
             * @assume(new_directory->d_node.i_refcnt == 1); (Exclusive user)
             * @assume(new_directory->d_node.i_super == target_directory->d_node.i_super);
             * @assume(new_directory->d_node.i_flags & INODE_FATTRLOADED);
             * @assume(S_ISDIR(new_directory->d_node.i_attr.a_mode));
             * @assume(!directory_getentry(target_directory,target_dirent...));
             * @assume(target_dirent->de_namelen != 0);
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...] (Same as not implementing)
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         [...]
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:            [...]
             * @throw: E_NO_DATA: Translated to `E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL'
             */
            void (KCALL *d_mkdir)(struct directory_node *__restrict target_directory,
                                  struct directory_entry *__restrict target_dirent,
                                  struct directory_node *__restrict new_directory);

            /* [0..1]
             * [locked(WRITE(target_directory->i_lock))]
             *  Insert the given symbolic-link node `link_node' into the given
             *  target directory `target_directory' under `target_dirent', while
             *  initializing the following members of `link_node':
             *     - sl_node.i_attr.a_ino
             *     - sl_node.i_ops
             *     - sl_node.i_nlink (To `>= 1')
             *     - sl_node.i_fsdata (Optionally; pre-initialized to `NULL')
             *     - sl_node.i_attr.a_blocks (`a_size' has already been set; set this to the number of blocks used by the link text)
             *  This function must initialize the following members of `target_dirent':
             *     - de_fsdata (optionally, depending on filesystem implementation)
             *     - de_pos
             *     - de_ino (Same as `link_node->sl_node.i_attr.a_ino')
             * Upon success, this operator must fill in `target_dirent->de_pos'
             * @assume(link_node->sl_node.i_refcnt == 1); (Exclusive user)
             * @assume(link_node->sl_node.i_flags & INODE_FATTRLOADED);
             * @assume(link_node->sl_text != NULL);
             * @assume(!directory_getentry(target_directory,target_dirent...));
             * @assume(target_dirent->de_namelen != 0);
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_UNSUPPORTED_FUNCTION: [...] (Same as not implementing)
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...]
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         [...]
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:            [...]
             * @throw: E_NO_DATA: Translated to `E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL' */
            void (KCALL *d_symlink)(struct directory_node *__restrict target_directory,
                                    struct directory_entry *__restrict target_dirent,
                                    struct symlink_node *__restrict link_node);

            /* [0..1]
             * [locked(WRITE(target_directory->i_lock))]
             *  Allocate a new device node referred to by a single link that
             *  is created within `target_directory', using `target_dirent'
             *  as INode directory entry.
             *  This function must initialize the following members of `target_dirent':
             *     - de_fsdata (optionally, depending on filesystem implementation)
             *     - de_pos
             *     - de_ino
             *  This function must initialize the following members of `device_node':
             *     - i_attr.a_ino (Same as `target_dirent->de_ino')
             *     - i_ops
             *     - i_nlink (To `>= 1')
             *     - i_fsdata (Optionally; pre-initialized to `NULL')
             *  During this operation, the new directory entry should be
             *  constructed on-disk, or scheduled for such an allocation.
             *  NOTE: This function may also choose to set the `INODE_FCHANGED' flag
             *        of `device_node', in which case the caller will add the node to
             *        the set of changed nodes of the accompanying superblock, meaning
             *        that filesystem implementations may choose to lazily set save
             *        attributes of newly constructed nodes to disk once the disk
             *        is supposed to be synchronized.
             * @assume(device_node->i_refcnt == 1); (Exclusive user)
             * @assume(device_node->i_flags & INODE_FATTRLOADED);
             * @assume(device_node->i_super  == target_directory->d_node.i_super);
             * @assume(S_ISBLK(device_node->i_attr.a_mode) || S_ISCHR(device_node->i_attr.a_mode));
             * @assume(!directory_getentry(target_directory,target_dirent...));
             * @assume(target_dirent->de_namelen != 0);
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_UNSUPPORTED_FUNCTION: [...] (Same as not implementing)
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...]
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         [...]
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:            [...]
             * @throw: E_NO_DATA: Translated to `E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL' */
            void (KCALL *d_mknod)(struct directory_node *__restrict target_directory,
                                  struct directory_entry *__restrict target_dirent,
                                  struct inode *__restrict device_node);

            /* [0..1]
             * [locked(WRITE(target_directory->i_lock))]
             * [locked(WRITE(link_target->i_lock))]
             *  Construct a new hardlink to `link_target' as a new directory
             *  entry `target_dirent' within `target_directory'.
             *  This function must initialize the following members of `target_dirent':
             *     - de_fsdata (optionally, depending on filesystem implementation)
             *     - de_pos
             * @assume(target_directory->d_node.i_super == link_target->i_super);
             * @assume(!directory_getentry(target_directory,target_dirent...));
             * @assume(target_dirent->de_namelen != 0);
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_UNSUPPORTED_FUNCTION: [...] (Same as not implementing)
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...]
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         [...]
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:            [...]
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_TOO_MANY_HARD_LINKS:  [...]
             * @throw: E_NO_DATA: Translated to `E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL' */
            void (KCALL *d_link)(struct directory_node *__restrict target_directory,
                                 struct directory_entry *__restrict target_dirent,
                                 struct inode *__restrict link_target);

            /* [0..1]
             * [locked(WRITE(target_directory->i_lock))]
             * [locked(WRITE(source_directory->i_lock))]
             * [locked(WRITE(source_node->i_lock))]
             *  Move the given `source_node' from one directory into another.
             *  This operation may construct a new INode for the target directory
             *  and have it inherit all data from the old node before marking that
             *  node as deleted and removing its directory entry `source_dirent'
             *  from `source_directory'.
             *  This function must initialize the following members of `target_dirent':
             *     - de_fsdata (optionally, depending on filesystem implementation)
             *     - de_pos
             *     - de_ino (or `de_virtual' and set `de_type' is `DT_WHT')
             *  However if it chooses to do this, the operator itself is responsible
             *  for adding the new INode to the associated superblock!
             *  If the filesystem supports hardlinks, this operator can be left
             *  undefined, as it must then be possible to emulate it through use
             *  of `d_link(target_directory,target_dirent,source_node)', followed
             *  by `io_unlink(source_directory,source_dirent,source_node)'.
             *  NOTE: Only `d_rename' operator of `source_directory' is invoked for this operation.
             * @assume(INODE_ISDIR(source_directory));
             * @assume(INODE_ISDIR(target_directory));
             * @assume(source_directory->i_super == target_directory->i_super);
             * @assume(!directory_getentry(target_directory,target_dirent...));
             * @assume(source_dirent->de_namelen != 0);
             * @assume(target_dirent->de_namelen != 0);
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...]
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         [...] (in `target_dirent')
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:            [...]
             * @throw: E_NO_DATA: Translated to `E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL' */
            void (KCALL *d_rename)(struct directory_node *__restrict source_directory,
                                   struct directory_entry *__restrict source_dirent,
                                   struct directory_node *__restrict target_directory,
                                   struct directory_entry *__restrict target_dirent,
                                   struct inode *__restrict source_node);

            /* [0..1]
             * [locked(WRITE(containing_directory->i_lock))]
             * [locked(WRITE(node_to_unlink->i_lock))]
             * Unlink a given `node_to_unlink' from `containing_entry' in `containing_directory'.
             * NOTE: The given `containing_entry' may be another directory,
             *       in which case the caller is responsible to ensure that
             *      `i_nlink' is greater than ZERO(0).
             * NOTE: This function is not responsible to deallocate file data
             *       allocated for the INode. If the INode has a non-ZERO(0)
             *       `i_attr.a_size' or `i_attr.a_blocks' value prior to the
             *       caller choosing to invoke this operator, the `f_truncate'
             *       operator will be invoked first, making it its responsibility
             *       to deallocate any file data associated with the INode before
             *       it goes away.
             * NOTE: Upon success, this function must decrement `i_nlink'.
             * @assume(containing_entry->i_attr.a_size == 0);
             * @assume(INODE_ISDIR(containing_directory));
             * @assume(containing_entry->de_namelen != 0);
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...] (Same as not implementing) */
            void (KCALL *d_unlink)(struct directory_node *__restrict containing_directory,
                                   struct directory_entry *__restrict containing_entry,
                                   struct inode *__restrict node_to_unlink);

            /* [0..1]
             * [locked(WRITE(containing_directory->i_lock))]
             * [locked(WRITE(node_to_unlink->i_lock))]
             * Same as `io_unlink', but called when `node_to_unlink' is a directory.
             * @assume(IS_EMPTY_DIRECTORY_EXCEPT_FOR_PARENT_AND_SELF_LINKS(containing_entry))
             * @assume(node_to_unlink->d_node.i_nlink == 1); // Last link (Must be decremented upon success)
             * @assume(node_to_unlink->d_size         == 2); // Only special directory entries are remaining.
             * @assume(containing_entry->de_namelen != 0);
             * @assume(INODE_ISDIR(containing_directory));
             * @assume(INODE_ISDIR(node_to_unlink));
             * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...] (Same as not implementing) */
            void (KCALL *d_rmdir)(struct directory_node *__restrict containing_directory,
                                  struct directory_entry *__restrict containing_entry,
                                  struct directory_node *__restrict node_to_unlink);
        }              io_directory;
        struct PACKED {
            /* [1..1]
             * [locked(WRITE(self))]
             *  Load the link text of the given symlink node.
             *  This operator must fill in the following members of `self':
             *    - sl_text
             *    - sl_node.i_attr.a_size
             *    - sl_node.i_attr.a_blocks (Optionally)
             */
            void (KCALL *sl_readlink)(struct symlink_node *__restrict self);
        }              io_symlink;
    };
};

#define INODE_ISREG(x) S_ISREG((x)->i_attr.a_mode) /* Check if `x' is a `struct regular_node' */
#define INODE_ISDIR(x) S_ISDIR((x)->i_attr.a_mode) /* Check if `x' is a `struct directory_node' */
#define INODE_ISLNK(x) S_ISLNK((x)->i_attr.a_mode) /* Check if `x' is a `struct symlink_node' */
struct inode {
    ATOMIC_DATA ref_t           i_refcnt;  /* INode reference counter. */
    struct inode_operations const *i_ops;  /* [1..1][const] INode operations. */
    REF struct superblock      *i_super;   /* [1..1][ref_if(self != i_super->s_root)][const] The associated superblock. */
    rwlock_t                    i_lock;    /* [ORDER(AFTER(i_super->s_changed_lock))]
                                            * [ORDER(BEFORE(CONTAINING_DIRECTORY_INODE->i_lock))]
                                            * Lock for accessing this INode. */
    nlink_t                     i_nlink;   /* [lock(i_lock)][valid_if(INODE_FATTRLOADED)]
                                            * Link counter of this INode.
                                            * When this counter reaches ZERO(0), data of the node is
                                            * deleted and the node can no longer be operated upon. */
    u16                         i_flags;   /* INode flags (Set of `INODE_F*') */
    u16                         i_pad[(sizeof(void *)-2)/2]; /* ... */
    struct inode_data          *i_fsdata;  /* [0..?][lock(i_lock)] Pointer to filesystem-specific INode data. */
    LIST_NODE(struct inode)     i_changed; /* [valid_if(INODE_FCHANGED)][lock(i_super->s_changed_lock)]
                                            *  Chain of per-superblock changed nodes. */
    LIST_NODE(struct inode)     i_nodes;   /* [const][lock(i_super->s_nodes_lock)]
                                            *  Chain of nodes associated with `i_super'. */
    struct {
        ino_t                   a_ino;     /* [const] INode number. */
        ATOMIC_DATA mode_t      a_mode;    /* [lock(i_lock)][valid_if(INODE_FATTRLOADED)]
                                            *  Unix file access mode of this INode.
                                            *  NOTE: Bits masked by `S_IFMT' are constant.
                                            *  NOTE: A weak view of this field can be read atomically. */
        uid_t                   a_uid;     /* [lock(i_lock)] Owner user id. */
        gid_t                   a_gid;     /* [lock(i_lock)] Owner group id. */
        pos_t                   a_size;    /* [lock(i_lock)] File size / symlink length in bytes. */
        blkcnt_t                a_blocks;  /* [lock(i_lock)][valid_if(!(i_super->s_flags&SUPERBLOCK_FNOBLOCKS))] Block count. */
        struct timespec         a_atime;   /* [lock(i_lock)] Last accessed time. */
        struct timespec         a_mtime;   /* [lock(i_lock)] Last modified time. */
        struct timespec         a_ctime;   /* [lock(i_lock)] File creation time. */
        dev_t                   a_rdev;    /* [const] Referenced device number.
                                            *  NOTE: Only used when `S_IFCHR(a_mode)' or `S_IFBLK(a_mode)'
                                            *        Otherwise, this field is set to ZERO(0). */
    }                           i_attr;    /* INode attributes. */
};

struct regular_node {
    struct inode                re_node;   /* Underlying node. */
    struct PACKED {
        atomic_rwlock_t         m_lock;    /* Lock for `m_module' */
        WEAK struct module     *m_module;  /* [lock(m_lock)][0..1] A weak pointer to an application module addressible through this INode. */
    }                           re_module; /* Module cache information. */
};


#ifdef __INTELLISENSE__
#define __DEFINE_DIRECTORY_ENTRY_EX(symbol_name,name,...) \
struct { \
    ref_t de_refcnt; \
    struct directory_entry *de_next; \
    LIST_NODE(struct directory_entry) de_bypos; \
    struct PACKED { \
        pos_t de_start; \
        unsigned char de_data[16]; \
    } de_fsdata; \
    pos_t de_pos; \
    union PACKED { \
        REF struct inode *de_virtual; \
        ino_t de_ino; \
    }; \
    uintptr_t de_hash; \
    u16 de_namelen; \
    unsigned char de_type; \
    char de_name[COMPILER_LENOF(name)]; \
} symbol_name = { 0 }
#else
#define __DEFINE_DIRECTORY_ENTRY_EX(symbol_name,name,...) \
struct { \
    ref_t de_refcnt; \
    struct directory_entry *de_next; \
    LIST_NODE(struct directory_entry) de_bypos; \
    struct PACKED { \
        pos_t de_start; \
        unsigned char de_data[16]; \
    } de_fsdata; \
    pos_t de_pos; \
    union PACKED { \
        REF struct inode *de_virtual; \
        ino_t de_ino; \
    }; \
    uintptr_t de_hash; \
    u16 de_namelen; \
    unsigned char de_type; \
    char de_name[COMPILER_LENOF(name)]; \
} symbol_name = { \
    .de_refcnt = 0x3fffffff, \
     __VA_ARGS__ \
    .de_namelen = COMPILER_STRLEN(name), \
    .de_name = name \
}
#endif

#define DEFINE_DIRECTORY_ENTRY(symbol_name,name,hash,type,ino) \
      __DEFINE_DIRECTORY_ENTRY_EX(symbol_name,name,.de_hash = hash,.de_type = type,.de_ino = ino,)
#define DEFINE_DIRECTORY_ENTRY_V(symbol_name,name,hash,type,virtual) \
      __DEFINE_DIRECTORY_ENTRY_EX(symbol_name,name,.de_hash = hash,.de_type = type,.de_virtual = virtual,)


struct directory_entry {
    /* XXX: This structure must be kept in sync with `/src/kernel/src/fs/driver.c' */
    ATOMIC_DATA ref_t                 de_refcnt;  /* Reference counter for this data structure. */
    struct directory_entry           *de_next;    /* [0..1][lock(:i_lock)] Next directory entry with the same hash. */
    LIST_NODE(struct directory_entry) de_bypos;   /* [lock(:i_lock)] Chain of directory entires, sorted by their in-directory position.
                                                   *  NOTE: When a directory entry is removed, this link is set to NULL,
                                                   *        meaning that enumerating a directory is as simple as taking
                                                   *       `:d_bypos' of the directory node while holding `:d_node.i_lock'
                                                   *        and acquire a reference.
                                                   *        Then to continue enumeration, simply re-acquire the `:d_node.i_lock'
                                                   *        lock and traverse the `de_bypos' chain, stopping when NULL is encountered.
                                                   *        When `NULL' is encountered, you may check if your current node
                                                   *        is `d_bypos_end'. If it isn't, enumeration was halted because your
                                                   *        current node was deleted. It it was, make use of a secondary entry
                                                   *        index you've been keeping track of to skip to the next entry.
                                                   *  NOTE: If you reach EOF normally, check the `INODE_FDIRLOADED' flag to see
                                                   *        if additional directory entries exist that haven't been loaded yet. */
    struct PACKED { /* Optional, filesystem-specific data. */
        pos_t                         de_start;   /* [const][valid_if(?)]
                                                   *  Filesystem-specific starting position of data for this directory entry.
                                                   *  Filesystem where the actual name of the file is located before the its
                                                   *  data block (such as FAT's long file names), use this to point to the
                                                   *  start of such data.
                                                   *  NOTE: The filesystem is allowed to use this location for whatever it chooses. */
        unsigned char                 de_data[16];/*  Arbitrary, filesystem-specific data. (in FAT, this is the 8.3 version of the filename) */
    }                                 de_fsdata;
    pos_t                             de_pos;     /* [const] Filesystem-specific position with the INode's data
                                                   *         block where data for this directory entry can be found. */
    union PACKED {
        REF struct inode             *de_virtual; /* [valid_if(de_type == DT_WHT)][const][1..1] Virtual node binding.
                                                   *  Entries of this type should be enumerated as `IFTODT(de_virtual->i_attr.a_mode)',
                                                   *  must not be removed when the directory cache is cleared, and must not be saved
                                                   *  to disk under any circumstances.
                                                   *  > This mechanism is used to implement mounting points.
                                                   *  NOTE: The caller is responsible to ensure that creation of
                                                   *        virtual node mappings will not cause reference loops. */
        ino_t                         de_ino;     /* [valid_if(de_type != DT_WHT)][const] INode number of this directory entry. */
    };
    uintptr_t                         de_hash;    /* [const] Hash of this directory entry. */
    u16                               de_namelen; /* [const][!0] Length of the directory entry name (in characters). */
    unsigned char                     de_type;    /* [const] Directory entry type (one of `DT_*' from <dirent.h>) */
    char                              de_name[1]; /* [const][de_namelen] Directory entry name. (NUL-terminated) */
};

#define DIRECTORY_ENTRY_EMPTY_HASH 0 /* == directory_entry_hash("",0) */

/* Destroy a previously allocated directory_entry. */
FUNDEF ATTR_NOTHROW void KCALL directory_entry_destroy(struct directory_entry *__restrict self);

/* Increment/decrement the reference counter of the given directory_entry `x' */
#define directory_entry_incref(x)  ATOMIC_FETCHINC((x)->de_refcnt)
#define directory_entry_decref(x) (ATOMIC_DECFETCH((x)->de_refcnt) || (directory_entry_destroy(x),0))

/* Return the hash of a given directory entry name.
 * @throw: E_SEGFAULT: Failed to access the given `name'. */
FUNDEF WUNUSED uintptr_t KCALL
directory_entry_hash(CHECKED USER char const *__restrict name,
                     u16 namelen);



#define DIRECTORY_DEFAULT_MASK  7
struct directory_node {
    /* Directory INode (When `S_ISDIR(a_mode)' is true) */
    struct inode                      d_node;     /* [OVERRIDE(i_attr.a_rdev,[== 0])]
                                                   * [OVERRIDE(i_attr.a_mode,[S_ISDIR(this)])]
                                                   * The underlying node. */
    pos_t                             d_dirend;   /* [lock(d_node.i_lock)] Starting address of the next directory entry which hasn't been read yet. */
    size_t                            d_size;     /* [lock(d_node.i_lock)] Amount of directory entries. */
    size_t                            d_mask;     /* [!0][lock(d_node.i_lock)] Allocated directory entry hash-map mask. */
    REF struct directory_entry      **d_map;      /* [0..1][lock(d_node.i_lock)][1..d_mask+1][owned] Hash-map of directory entries. */
    struct directory_entry           *d_bypos;    /* [lock(d_node.i_lock)][0..1] Chain of all known directory entries, ordered by address. */
    struct directory_entry           *d_bypos_end;/* [lock(d_node.i_lock)][0..1] Last directory entry that marks the end of the directory. */
};

struct symlink_node {
    /* Symbolic link INode (When `S_ISLNK(a_mode)' is true) */
    struct inode                      sl_node;    /* [OVERRIDE(i_attr.a_size,[const])]
                                                   * [OVERRIDE(i_attr.a_blocks,[const])]
                                                   * [OVERRIDE(i_attr.a_rdev,[== 0])]
                                                   * [OVERRIDE(i_attr.a_mode,[S_ISLNK(this)])]
                                                   * The underlying node. */
    HOST char                        *sl_text;    /* [0..i_attr.a_size][valid_if(INODE_FLNKLOADED)]
                                                   * Symbolic link text. */
};


/* Destroy a previously allocated INode. */
FUNDEF ATTR_NOTHROW void KCALL inode_destroy(struct inode *__restrict self);

/* Increment/decrement the reference counter of the given inode `x' */
#define inode_tryincref(x)  ATOMIC_INCIFNONZERO((x)->i_refcnt)
#define inode_incref(x)     ATOMIC_FETCHINC((x)->i_refcnt)
#define inode_decref(x)    (ATOMIC_DECFETCH((x)->i_refcnt) || (inode_destroy(x),0))



/* Allocate and pre-initialize a new directory INode.
 * The caller must still initialize:
 *   - d_node.i_attr.a_mode
 *   - d_node.i_attr.a_ino
 *   - d_node.i_attr.a_uid
 *   - d_node.i_attr.a_gid
 *   - d_node.i_attr.a_atime
 *   - d_node.i_attr.a_mtime
 *   - d_node.i_attr.a_ctime
 *   - d_node.i_super */
FUNDEF WUNUSED ATTR_RETNONNULL
REF struct directory_node *KCALL directory_node_alloc(void);


/* Read data to/from a given INode.
 * NOTES:
 *   - If the INode was closed (`INODE_ISCLOSED(self)' is true),
 *     read/write operators will return ZERO(0) and `read/write-all'
 *     will throw `E_NO_DATA' / `ERROR_FS_READONLY_FILESYSTEM' errors.
 *   - If the INode doesn't implement the read-operator, the following
 *     actions are performed:
 *       - If the INode refers to a character device (`S_ISCHR(i_attr.a_mode)'),
 *         a query will be made for that device in order to attempt to invoke
 *         its `c_file.f_pread' / `c_file.f_pwrite' operator (if that operator
 *         has been implemented).
 *         Note however that no attempt is made to open() that character device,
 *         meaning that `inode -> character_device -> open -> pread' will not
 *         be resolved, however `inode -> character_device -> pread' is.
 *       - Otherwise, ZERO(0) is returned and `read/write-all' will
 *         throw `E_NO_DATA' / `ERROR_FS_READONLY_FILESYSTEM' errors. */
FUNDEF size_t KCALL inode_read(struct inode *__restrict self, CHECKED USER void *buf, size_t bufsize, pos_t pos, iomode_t flags);
FUNDEF size_t KCALL inode_write(struct inode *__restrict self, CHECKED USER void const *buf, size_t bufsize, pos_t pos, iomode_t flags);
FUNDEF size_t KCALL inode_kread(struct inode *__restrict self, void *__restrict buf, size_t bufsize, pos_t pos, iomode_t flags) ASMNAME("inode_read");
FUNDEF size_t KCALL inode_kwrite(struct inode *__restrict self, void const *__restrict buf, size_t bufsize, pos_t pos, iomode_t flags) ASMNAME("inode_write");
FUNDEF void KCALL inode_kreadall(struct inode *__restrict self, void *__restrict buf, size_t bufsize, pos_t pos, iomode_t flags);
FUNDEF void KCALL inode_kwriteall(struct inode *__restrict self, void const *__restrict buf, size_t bufsize, pos_t pos, iomode_t flags);

/* Validate access to the given INode for the current user.
 * @param: how: Set of `X_OK|W_OK|R_OK' from <unistd.h>
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR: [...] */
FUNDEF void KCALL inode_access(struct inode *__restrict self, int how);

/* Returns a set `X_OK|W_OK|R_OK' from <unistd.h> describing what
 * operations may be performed on the given inode by the calling thread. */
FUNDEF WUNUSED int KCALL inode_accessok(struct inode *__restrict self);

/* Update the INode's `i_attr.a_mtime' field, set the `INODE_FCHANGED'
 * flag, and add the INode to its superblock's chain of changed nodes.
 * NOTE: The caller must be holding a write-lock on `self->i_lock' */
FUNDEF void KCALL inode_changed(struct inode *__restrict self);

/* Ensure that attributes have been loaded for the given INode. */
FUNDEF void KCALL inode_loadattr(struct inode *__restrict self);


/* Truncate the length of the given INode.
 * @param: mode: Set of `DIRECTORY_REMOVE_F*'
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:          [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_NOT_FOUND:        [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:   [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_TRUNCATE_GREATER_SIZE: [...] */
FUNDEF void KCALL
inode_truncate(struct inode *__restrict self,
               pos_t new_smaller_size);

/* Return information on hard limits for the given INode.
 * @param: name: One of `_PC_*' from `<bits/confname.h>'
 * @return: -EINVAL: The given `name' is not implemented by the underlying filesystem.
 * NOTE: This function invokes the `io_pathconf' operator of given INode `self'.
 *       If that operator isn't implemented or returns `-EINVAL',
 *       the same operator is re-invoked on `self->i_super->s_root'.
 *       If that operator isn't implemented, or returns `-EINVAL' too,
 *       the following values are returned for specific `name's,
 *       or `-EINVAL' for any other name:
 *         - _PC_LINK_MAX:           LINK_MAX     (From <linux/limits.h>)
 *         - _PC_MAX_CANON:          MAX_CANON    (From <linux/limits.h>)
 *         - _PC_MAX_INPUT:          MAX_INPUT    (From <linux/limits.h>)
 *         - _PC_NAME_MAX:           65535
 *         - _PC_PATH_MAX:           LONG_MAX
 *         - _PC_PIPE_BUF:           PIPE_BUF     (From <linux/limits.h>)
 *         - _PC_CHOWN_RESTRICTED:   0
 *         - _PC_NO_TRUNC:           1
 *         - _PC_VDISABLE:           '\0'
 *         - _PC_FILESIZEBITS:       64
 *         - _PC_REC_INCR_XFER_SIZE: self->i_super->s_device->b_blocksize
 *         - _PC_REC_MAX_XFER_SIZE:  self->i_super->s_device->b_blocksize*8
 *         - _PC_REC_MIN_XFER_SIZE:  self->i_super->s_device->b_blocksize
 *         - _PC_REC_XFER_ALIGN:     self->i_super->s_device->b_blocksize
 *         - _PC_ALLOC_SIZE_MIN:     self->i_super->s_device->b_blocksize
 *         - _PC_SYMLINK_MAX:        self->i_ops->io_directory.d_symlink != NULL ? LONG_MAX : 0
 *         - _PC_2_SYMLINKS:         self->i_ops->io_directory.d_symlink != NULL ? 1 : 0 */
FUNDEF WUNUSED long int KCALL
inode_pathconf(struct inode *__restrict self, int name);


/* Collect stat()-information in the given INode and fill in `result'.
 * NOTE: This function only returns information available through
 *       the INode itself. Information available on a character or
 *       block device that might be reachable through `a_rdev' is
 *       not dereferenced. */
FUNDEF void KCALL
inode_stat(struct inode *__restrict self,
           USER CHECKED struct stat64 *result);


/* Connect to signals broadcast when `mode' (set of `POLLIN|POLLPRI|POLLOUT|POLLERR')
 * becomes true. Return the set of signals who's associated condition is currently met:
 * >> unsigned int result = 0;
 * >> FOREACH_POLL_MODE(x,mode) {
 * >>     task_connect(GET_SIGNAL_FOR(self,x));
 * >>     if (CONDITION_IS_MET(self,x))
 * >>         result |= x;
 * >> }
 * >> return result; */
FUNDEF unsigned int KCALL
inode_poll(struct inode *__restrict self,
           unsigned int mode);


/* Change all non-NULL the timestamp that are given.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...] */
FUNDEF void KCALL
inode_chtime(struct inode *__restrict self,
             struct timespec *new_atime,
             struct timespec *new_mtime,
             struct timespec *new_ctime);

/* Change permissions, SUID/SGID and the sticky
 * bit of the given INode (flags mask: 07777)
 * The new file mode is calculated as `(old_mode & perm_mask) | perm_flag',
 * before being masked by what the underlying filesystem is capable of
 * representing.
 * @return: * : The new file mode (as masked by `io_maskattr()')
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...] */
FUNDEF mode_t KCALL
inode_chmod(struct inode *__restrict self,
            mode_t perm_mask, mode_t perm_flag);

/* Change the owner and group of the given file.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...] */
FUNDEF void KCALL
inode_chown(struct inode *__restrict self,
            uid_t owner, gid_t group);


/* Perform an IO-control operation described by `cmd'.
 * NOTE: The caller must be holding a write-lock on `self'
 * @return: * :  Command-dependent return value (propagated to user-space)
 * @param: mode: Permissions and block-behavior that should
 *               be applied to limit allowed functions.
 * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator.
 *                            Should be thrown for unrecognized commands. */
FUNDEF ssize_t KCALL
inode_ioctl(struct inode *__restrict self, unsigned long cmd,
            USER UNCHECKED void *arg, iomode_t flags);

/* Synchronize data of the given INode and attributes (when `data_only' is false). */
FUNDEF void KCALL inode_sync(struct inode *__restrict self, bool data_only);

/* Synchronize attributes of the given INode.
 * @return: true:  Attributes have been written to disk, or some intermediate buffer.
 * @return: false: Attributes of the node haven't changed. */
FUNDEF bool KCALL inode_syncatttr(struct inode *__restrict self);

/* Ensure that the given symbolic-link INode has been loaded.
 * This function is a no-op when `self->sl_text != NULL' upon entry,
 * and guaranties that `self->sl_text' will be non-NULL upon success.
 * @throw: E_BADALLOC:                                       [...]
 * @throw: E_IO_ERROR:                                       [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_NOT_FOUND:       [...] */
FUNDEF void KCALL
symlink_node_load(struct symlink_node *__restrict self);


/* Read the next directory entry from
 * `self' that hasn't been loaded, yet.
 * Return NULL and set the `INODE_FDIRLOADED' flag
 * once the entirety of the directory has been loaded.
 * NOTE: The caller must be holding a read-lock on the directory INode. */
FUNDEF WUNUSED struct directory_entry *KCALL
directory_readnext(struct directory_node *__restrict self,
                   iomode_t flags);

/* Lookup a directory item, given its name.
 * NOTE: The caller must be holding a read-lock on `self->d_node.i_lock'
 * @return: * :   A reference to the associated INode.
 * @return: NULL: No INode with the given `name' exists.
 *              (`ERROR_FS_FILE_NOT_FOUND' / `ERROR_FS_PATH_NOT_FOUND'-style)
 * @throw: E_SEGFAULT: Failed to access the given `name'.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_NOT_FOUND:       [...] */
FUNDEF WUNUSED struct directory_entry *KCALL
directory_getentry(struct directory_node *__restrict self,
                   CHECKED USER char const *__restrict name,
                   u16 namelen, uintptr_t hash);
FUNDEF WUNUSED struct directory_entry *KCALL
directory_getcaseentry(struct directory_node *__restrict self,
                       CHECKED USER char const *__restrict name,
                       u16 namelen, uintptr_t hash);
/* Same as `directory_getentry()', but automatically dereference
 * the directory entry to retrieve the associated INode. */
FUNDEF WUNUSED REF struct inode *KCALL
directory_getnode(struct directory_node *__restrict self,
                  CHECKED USER char const *__restrict name,
                  u16 namelen, uintptr_t hash);
FUNDEF REF struct inode *KCALL
directory_getcasenode(struct directory_node *__restrict self,
                      CHECKED USER char const *__restrict name,
                      u16 namelen, uintptr_t hash);

/* Create a new file within the given directory.
 * NOTE: The caller must be holding a read-lock on `self->d_node.i_lock'
 * NOTE: When `open_mode & O_EXCL' is set, only `struct regular_node' are ever returned.
 * NOTE: When `open_mode & O_DOSPATH' is set, ignore casing when checking for existing files.
 * @param: pentry: When non-NULL, store a reference to the resulting node's directory entry here.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_PATH_NOT_FOUND:       [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         `namelen' was ZERO(0)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_ALREADY_EXISTS:  [...] (Only when `open_mode & O_EXCL' is set)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:            [...] */
FUNDEF ATTR_RETNONNULL REF struct inode *KCALL
directory_creatfile(struct directory_node *__restrict self,
                    CHECKED USER char const *__restrict name,
                    u16 namelen, oflag_t open_mode,
                    uid_t owner, gid_t group, mode_t mode,
                    REF struct directory_entry **pentry);


/* Remove an entry from this directory.
 * @param: self_path: When non-NULL, also try to remove the directory
 *                    entry from this path (if it exists therein, too)
 *              NOTE: This argument is required to keep the caller from
 *                    having to re-evaluate the given `path' after success,
 *                    as some malicious user could then change the path
 *                    in the mean time and get us to remove a virtual
 *                    mounting point, or something along those lines.
 * @param: mode: Set of `DIRECTORY_REMOVE_F*'
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_PATH_NOT_FOUND:       [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_NOT_FOUND:       [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DIRECTORY_NOT_EMPTY:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_RMDIR_REGULAR:        [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_UNLINK_DIRECTORY:     [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_REMOVE_MOUNTPOINT:    [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_UNMOUNT_NOTAMOUNT:    [...] */
FUNDEF void KCALL
directory_remove(struct directory_node *__restrict self,
                 CHECKED USER char const *__restrict name,
                 u16 namelen, uintptr_t hash, unsigned int mode,
                 struct path *self_path);
#define DIRECTORY_REMOVE_FREGULAR   0x0001 /* Remove regular files (`unlink()') */
#define DIRECTORY_REMOVE_FDIRECTORY 0x0002 /* Remove directories (`rmdir()') */
#define DIRECTORY_REMOVE_FVIRTUAL   0x0004 /* Remove virtual nodes (`umount()') */
#define DIRECTORY_REMOVE_FNOCASE    0x8000 /* Ignore casing. */


/* Rename/move an entry from one directory to another.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_PATH_NOT_FOUND:       [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_NOT_FOUND:       [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_CROSSDEVICE_LINK:     [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         `target_namelen' was ZERO(0)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_ALREADY_EXISTS:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:            [...] */
FUNDEF void KCALL
directory_rename(struct directory_node *__restrict source_directory,
                 struct directory_entry *__restrict source_dirent,
                 struct directory_node *__restrict target_directory,
                 CHECKED USER char const *__restrict target_name,
                 u16 target_namelen);

/* Create a hardlink.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_NOT_FOUND:       [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_UNSUPPORTED_FUNCTION: [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_CROSSDEVICE_LINK:     [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         `target_namelen' was ZERO(0)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_ALREADY_EXISTS:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:            [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_TOO_MANY_HARD_LINKS:  [...] */
FUNDEF void KCALL
directory_link(struct directory_node *__restrict target_directory,
               CHECKED USER char const *__restrict target_name,
               u16 target_namelen, struct inode *__restrict link_target,
               bool ignore_casing);

/* Create a symbolic link and return its INode.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_UNSUPPORTED_FUNCTION: [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         `target_namelen' was ZERO(0)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_ALREADY_EXISTS:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:            [...] */
FUNDEF WUNUSED ATTR_RETNONNULL REF struct symlink_node *KCALL
directory_symlink(struct directory_node *__restrict target_directory,
                  CHECKED USER char const *__restrict target_name, u16 target_namelen,
                  CHECKED USER char const *__restrict link_text, size_t link_text_size,
                  uid_t owner, gid_t group, mode_t mode, bool ignore_casing);

/* Create a file / device node.
 * @assume(S_ISREG(mode) || S_ISBLK(mode) || S_ISCHR(mode));
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_UNSUPPORTED_FUNCTION: [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         `target_namelen' was ZERO(0)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_ALREADY_EXISTS:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:            [...] */
FUNDEF WUNUSED ATTR_RETNONNULL REF struct inode *KCALL
directory_mknod(struct directory_node *__restrict target_directory,
                CHECKED USER char const *__restrict target_name,
                u16 target_namelen, mode_t mode, uid_t owner,
                gid_t group, dev_t referenced_device, bool ignore_casing);


/* Create a new directory node.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_READONLY_FILESYSTEM:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:         `target_namelen' was ZERO(0)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_ALREADY_EXISTS:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:            [...] */
FUNDEF WUNUSED ATTR_RETNONNULL REF struct directory_node *KCALL
directory_mkdir(struct directory_node *__restrict target_directory,
                CHECKED USER char const *__restrict target_name,
                u16 target_namelen, mode_t mode, uid_t owner,
                gid_t group, bool ignore_casing);



#define SUPERBLOCK_TYPE_NAME_MAX  15
struct superblock_type {
    LIST_NODE(struct superblock_type) 
         st_chain;    /* [lock(fs_filesystem_types.ft_typelock)] Chain of filesystem types. */
    char st_name[SUPERBLOCK_TYPE_NAME_MAX];
                      /* [const] Name of the superblock type (As passed to `mount' using
                       *         the `-t' option, as well as appearing in `/dev/filesystems') */
#define SUPERBLOCK_TYPE_FNORMAL 0x00 /* Normal superblock type flags. */
#define SUPERBLOCK_TYPE_FNODEV  0x01 /* The superblock should be constructed without a device.
                                      * When this flag is set, `null_device' should be passed
                                      * to as `dev' argument to `s_open'. */
#define SUPERBLOCK_TYPE_FSINGLE 0x80 /* This type of superblock is a singleton (e.g. the `devfs' superblock) */
    u8   st_flags; /* [const] Superblock type flags (Set of `SUPERBLOCK_TYPE_F*') */
    REF struct driver *st_driver; /* [1..1][const] The implementing driver application. */

    union PACKED {
        /* [1..1][const] Open a new superblock for `dev'.
         *  NOTE: To prevent race conditions, or hardware damage, this
         *        function should not attempt to modify on-disk data,
         *        as it may be called, only to have the return value
         *        discarded when multiple tasks attempt to open a
         *        given device as a superblock at the same time.
         *  This function must initialize the following members of `self':
         *    - s_root->d_node.i_attr.a_ino
         *    - s_root->d_node.i_ops
         *    - s_root->d_node.i_nlink (To `>= 1')
         *    - s_root->d_node.i_fsdata (Optionally; pre-initialized to `NULL')
         *    - s_fsdata (Optionally; pre-initialized to `NULL')
         *  Upon success, the caller will add `self->s_root' to the nodes
         *  cache `self->s_nodesv', as well as to the `s_changed' list if
         *  this function sets the `INODE_FCHANGED' flag of the root node.
         *  After this, the caller will add the superblock to the global chain
         *  of existing superblocks through use of the `s_filesystems' field.
         * @param: args: Type-specific user-space data passed to the constructor.
         *               When non-NULL, most filesystem types expect this to point
         *               to a comma-separated string of ~flags~ describing special
         *               mounting behavior, as pass to the `mount' command using
         *               the `-o' option.
         * @throw: E_FILESYSTEM_ERROR.ERROR_FS_CORRUPTED_FILESYSTEM: [...]
         * @throw: E_NO_DATA:     Same as `E_FILESYSTEM_ERROR.ERROR_FS_CORRUPTED_FILESYSTEM'
         * @throw: E_INDEX_ERROR: Same as `E_FILESYSTEM_ERROR.ERROR_FS_CORRUPTED_FILESYSTEM'
         */
        void (KCALL *st_open)(struct superblock *__restrict self,
                              UNCHECKED USER char *args);
        /* [1..1][SUPERBLOCK_TYPE_FSINGLE]
         * The superblock singleton. */
        struct superblock *st_singleton;
    };
    
    struct PACKED {

        /* [0..1][const] Called during `superblock_destroy'. */
        void (KCALL *f_fini)(struct superblock *__restrict self);

        /* [1..1][const]
         * Perform additional initialization for loading a given `node'.
         * The caller has already initialize the following members of `node':
         *    - i_attr.a_ino
         *    - i_attr.a_mode & I_FMT
         *    - ... (Generic inode attributes except for `i_ops')
         * This function must then initialize the following members of `node':
         *    - i_ops
         *    - i_nlink (To something `>= 1')
         *    - i_fsdata (Optionally; pre-initialized to `NULL')
         * Optionally, INode attributes may be initialized, as would also be
         * done with a call to `io_loadattr()'. If this is done, this operator
         * should set the `INODE_FATTRLOADED' flag before returning. */
        void (KCALL *f_opennode)(struct inode *__restrict node,
                                 struct directory_node *__restrict parent_directory,
                                 struct directory_entry *__restrict parent_directory_entry);

        /* [0..1][const]
         *  Synchronize all unwritten data of this superblock.
         *  NOTE: This function is called when `superblock_sync()'
         *        finishes flushing the data of all changed INodes. */
        void (KCALL *f_sync)(struct superblock *__restrict self);

    } st_functions;
};


#define SUPERBLOCK_FNORMAL   0x0000 /* Normal superblock flags. */
#define SUPERBLOCK_FDOATIME  0x0001 /* [lock(atomic,weak)] Keep track of last-accessed timestamps (when opening streams to files). */
#define SUPERBLOCK_FNOMTIME  0x0002 /* [lock(atomic,weak)] Do not update last-modified timestamps. */
#define SUPERBLOCK_FCLOSED   0x4000 /* [lock(WRITE_ONCE)] The superblock is being closed.
                                     * When this flag is set, all of the superblock's cached INodes
                                     * are deleted and `superblock_opennode()' or any other function
                                     * that may be used for constructing new INodes can no longer be used
                                     * and will instead fail by throwing an `ERROR_FS_READONLY_FILESYSTEM'
                                     * error.
                                     * This flag is set by `superblock_shutdown()' and intended to
                                     * be called when a superblock is supposed to be unmounted, as
                                     * it prevents new files from being opened. */
#define SUPERBLOCK_FNOBLOCKS 0x8000 /* The filesystem allocates blocks in a special manner that cannot be,
                                     * or only could with great expense be represented as `i_attr.a_blocks'.
                                     * Instead, `stat()' system calls should ignore the `a_blocks' attribute
                                     * and return the result of `i_attr.a_size / s_device->b_blocksize'. */

#define SUPERBLOCK_DEFAULT_NODES_MASK 63
struct superblock {
    ATOMIC_DATA ref_t            s_refcnt;       /* Superblock reference counter. */
    struct superblock_type const*s_type;         /* [1..1][const] Superblock type & operations. */
    REF struct directory_node   *s_root;         /* [1..1][const] The root directory INode of this superblock. */
    REF struct block_device     *s_device;       /* [1..1][const] The device supposedly carrying the data of this superblock. */
    REF struct driver           *s_driver;       /* [1..1][const] The driver implementing this superblock. */
    REF struct wall             *s_wall;         /* [1..1][lock(s_lock)] WALL-clock used to generate filesystem timestamps. */
    atomic_rwlock_t              s_lock;         /* Lock for misc. superblock data. */
    u16                          s_flags;        /* Superblock flags (Set of `SUPERBLOCK_F*') */
    u16                          s_pad[(sizeof(void *)-2)/2]; /* ... */
    size_t                       s_nodesc;       /* [lock(s_nodes_lock)] Amount of hashed INodes. */
    size_t                       s_nodesm;       /* [lock(s_nodes_lock)] Hash-mask for the INodes mash-map. */
    REF LIST_HEAD(struct inode) *s_nodesv;       /* [0..1][ref_if(!INODE_FDONTCACHE)][CHAIN(->i_nodes)]
                                                  * [1..s_nodesm+1][owned][lock(s_nodes_lock)]
                                                  * Hash-map of cached INodes (use INode numbers as hash-index,
                                                  * and mask bits using `s_nodesm').
                                                  * NOTE: This cache is automatically cleared for loaded
                                                  *       superblocks when memory becomes sparse.
                                                  *       However until then, this hash-map can be used
                                                  *       to quickly load INodes, simply given their ID. */
    atomic_rwlock_t              s_nodes_lock;   /* Lock for the chain of changed INodes. */
    LIST_HEAD(struct inode)      s_changed;      /* [0..1][lock(s_changed_lock)] Chain of changed INodes. */
    atomic_rwlock_t              s_changed_lock; /* [ORDER(AFTER(s_nodes_lock))] Lock for the chain of changed INodes. */
    REF LIST_HEAD(struct path)   s_mount;        /* [0..1][lock(s_mount_lock)] Chain of mounting points. */
    atomic_rwlock_t              s_mount_lock;   /* Lock for paths that are mounting this superblock. */
    LIST_NODE(struct superblock) s_filesystems;  /* [lock(:f_superlock)] Chain of known filesystems. */
    struct superblock_data      *s_fsdata;       /* [0..?] File-system specific superblock data. */
};

/* Returns non-ZERO if the given superblock has been loaded.
 * This macro is called when a superblock is bound to a block-device. */
#define SUPERBLOCK_LOADED(x) ((x)->s_nodesc != 0)

/* Allocate a new, default-initialized superblock structure. */
FUNDEF ATTR_RETNONNULL REF struct superblock *KCALL superblock_alloc(void);

/* Load the given superblock, using its saved type.
 * The caller must first initialize the following members, assuming
 * that the superblock was allocated using `superblock_alloc()':
 *   - s_type
 *   - s_device
 * After successfully calling `self->s_type->st_open', this function
 * will perform the additional setup for registering the superblock,
 * as described alongside that operator. */
FUNDEF void KCALL
superblock_load(struct superblock *__restrict self,
                UNCHECKED USER char *args);

/* High-level, open the given device as a superblock.
 * @param: type: When NULL, automatically attempt to open the device using
 *               all known filesystem types (from `fs_filesystem_types').
 *               Otherwise, only try to open the device using that type and
 *               throw an `E_FILESYSTEM_ERROR.ERROR_FS_CORRUPTED_FILESYSTEM'
 *               if that type cannot be used, or if `device' already has
 *               another superblock associated, which is of a different type.
 * NOTE: If the given `device' has already been opened, return the existing
 *       filesystem or throw an `E_FILESYSTEM_ERROR.ERROR_FS_CORRUPTED_FILESYSTEM'
 *       error if an explicit type is given, which doesn't match the existing
 *       association. */
FUNDEF REF struct superblock *KCALL
superblock_open(struct block_device *__restrict device,
                struct superblock_type *type,
                UNCHECKED USER char *args);


/* Increment/decrement the reference counter of the given superblock `x' */
#define superblock_incref(x)  ATOMIC_FETCHINC((x)->s_refcnt)
#define superblock_decref(x) (ATOMIC_DECFETCH((x)->s_refcnt) || (superblock_destroy(x),0))

/* Destroy a previously allocated superblock. */
FUNDEF ATTR_NOTHROW void KCALL
superblock_destroy(struct superblock *__restrict self);

/* Clear the cache of the given superblock by deleting up to `max_nodes' from it.
 * @return: * : The actual number of deleted nodes.
 * @throw: E_WOULDBLOCK: Preemption has been disabled. */
FUNDEF size_t KCALL
superblock_clearcache(struct superblock *__restrict self,
                      size_t max_nodes);

/* Synchronize this superblock.
 * @throw: * : Failed to synchronize data for some reason.
 * @throw: E_WOULDBLOCK: Preemption has been disabled. */
FUNDEF void KCALL
superblock_sync(struct superblock *__restrict self);

/* Synchronize all superblocks (implementing the `sync()' system call). */
FUNDEF void KCALL superblock_syncall(void);

/* Close the given superblock and all associated nodes.
 * HINT: Following this, you should call `superblock_sync()',
 *       then followed by `superblock_clearcache()' in order
 *       to allow the superblock to safely be unmounted.
 * @return: true:  The superblock has been closed.
 * @return: false: The superblock had already been closed.
 * @throw: E_WOULDBLOCK: Preemption has been disabled. */
FUNDEF bool KCALL
superblock_close(struct superblock *__restrict self);

/* Helper functions to do the following:
 * >> result = superblock_close(self);
 * >> superblock_umount_all(self);
 * >> superblock_sync(self);
 * >> superblock_clearcache(self,(size_t)-1);
 * >> return result;
 * @throw: E_WOULDBLOCK: Preemption has been disabled. */
FUNDEF bool KCALL
superblock_shutdown(struct superblock *__restrict self);

/* Shutdown (call `superblock_shutdown()') on all known superblocks.
 * @return: * : The number of `superblock_shutdown()' calls that returned `true'.
 * @throw: * : Failed to synchronize data for some reason.
 * @throw: E_WOULDBLOCK: Preemption has been disabled. */
FUNDEF size_t KCALL superblock_shutdownall(void);



/* Open an INode of type `type', given a directory entry referencing it. */
FUNDEF ATTR_RETNONNULL REF struct inode *KCALL
superblock_opennode(struct superblock *__restrict self,
                    struct directory_node *__restrict parent_directory,
                    struct directory_entry *__restrict parent_directory_entry);

struct filesystem_types {
    atomic_rwlock_t                   ft_typelock; /* Lock for the chain of known filesystem types. */
    LIST_HEAD(struct superblock_type) ft_types;    /* [lock(ft_typelock)][CHAIN(->st_chain)] Chain of known filesystem types. */
};
struct filesystems {
    atomic_rwlock_t              f_superlock;      /* Lock for the chain of known superblocks. */
    LIST_HEAD(struct superblock) f_superblocks;    /* [lock(f_superlock)][CHAIN(->s_filesystems)] Chain of known filesystems. */
};


DATDEF struct filesystem_types fs_filesystem_types; /* Global tracking of known filesystem types. */
DATDEF struct filesystems      fs_filesystems;      /* Global tracking of existing filesystems (superblocks). */


/* Returns a reference to (the driver of the) filesystem type, given its `name'.
 * @return: NULL: No filesystem type matching the given `name' was found. */
FUNDEF REF struct superblock_type *KCALL
lookup_filesystem_type(USER CHECKED char const *name);

/* Register a filesystem type. */
FUNDEF void KCALL register_filesystem_type(struct superblock_type *__restrict type);

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Internally called during driver cleanup. */
INTDEF void KCALL delete_filesystem_type(struct superblock_type *__restrict type);
#endif


#define DEFINE_FILESYSTEM_TYPE(x) \
   DEFINE_DRIVER_INIT(_fs_##x##_init); \
   INTERN ATTR_FREETEXT void KCALL _fs_##x##_init(void) \
   { \
     register_filesystem_type(&x); \
   }


/* Add the given INode to the given superblock's INode cache.
 * NOTE: This is an internal function and care should be taken when it is actually used.
 *       This function may find use by singleton superblock initializers. */
FUNDEF void KCALL
superblock_addnode(struct superblock *__restrict self,
                   struct inode *__restrict node);

/* Register the given superblock as global.
 * NOTE: This function is automatically called by `superblock_load()' */
LOCAL void KCALL superblock_register(struct superblock *__restrict self) {
 assert(self->s_filesystems.le_pself == NULL);
 assert(self->s_filesystems.le_next == NULL);
 atomic_rwlock_write(&fs_filesystems.f_superlock);
 LIST_INSERT(fs_filesystems.f_superblocks,self,s_filesystems);
 atomic_rwlock_endwrite(&fs_filesystems.f_superlock);
}


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_NODE_H */
