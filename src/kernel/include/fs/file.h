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
#ifndef GUARD_KERNEL_INCLUDE_FS_FILE_H
#define GUARD_KERNEL_INCLUDE_FS_FILE_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/atomic.h>
#include <fs/handle.h>
#include <fs/iomode.h>
#include <dirent.h>

DECL_BEGIN

struct inode;
struct path;
struct dirent;

#ifndef READDIR_DEFAULT
#define READDIR_DEFAULT  0 /* Yield to next entry when `buf' was of sufficient size. */
#define READDIR_CONTINUE 1 /* Always yield to next entry. */
#define READDIR_PEEK     2 /* Never yield to next entry. */
#endif /* !READDIR_DEFAULT */

struct file_operations {
    /* [0..1] Called during `file_destroy' */
    /*ATTR_NOTHROW*/void (KCALL *f_fini)(struct file *__restrict self);
    
    /* [1..1] Read data from a file stream.
     * [locked(READ(self->f_node->i_lock))]
     * NOTE: The caller will have already performed permissions checks.
     * @return: * : The number of read bytes.
     * @throw: E_SEGFAULT:                                       The given user-buffer was faulty.
     * @throw: E_FILESYSTEM_ERROR.ERROR_FS_UNSUPPORTED_FUNCTION: Cannot read from this file. (Same as not defining) */
    size_t (KCALL *f_read)(struct file *__restrict self, USER CHECKED void *buf, size_t bufsize, iomode_t flags);

    /* [1..1] Write data to a file stream.
     * [locked(WRITE(self->f_node->i_lock))]
     * NOTE: The caller will have already performed permissions checks.
     * NOTE: The caller will have set `self->f_pos' to `self->f_node->i_attr.a_size'
     *       when the `O_APPEND' flag has been set.
     * @return: * : The number of written bytes.
     * @throw: E_SEGFAULT:                                       The given user-buffer was faulty.
     * @throw: E_FILESYSTEM_ERROR.ERROR_FS_UNSUPPORTED_FUNCTION: Cannot write to this file. (Same as not defining) */
    size_t (KCALL *f_write)(struct file *__restrict self, USER CHECKED void const *buf, size_t bufsize, iomode_t flags);

    /* [1..1] Move the file pointer within a file stream.
     * @return: * : The new, absolute position within the file.
     * @throw: E_INVALID_ARGUMENT:                               The given `whence' is not supported.
     * @throw: E_FILESYSTEM_ERROR.ERROR_FS_NEGATIVE_SEEK:        The resulting file position would be negative or would overflow.
     * @throw: E_FILESYSTEM_ERROR.ERROR_FS_UNSUPPORTED_FUNCTION: Seeking is not allowed in this stream. (Same as not defining) */
    pos_t (KCALL *f_seek)(struct file *__restrict self, off_t offset, int whence);

    /* [1..1]
     * [locked(WRITE(self->f_node->i_lock))]
     * Perform an IO-control operation described by `cmd'.
     * In standard files, this operator simply forwards the command to the associated INode.
     * @return: * : Command-dependent return value (propagated to user-space)
     * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator.
     *                            Should be thrown for unrecognized commands. */
    ssize_t (KCALL *f_ioctl)(struct file *__restrict self, unsigned long cmd,
                             USER UNCHECKED void *arg, iomode_t flags);

    /* [1..1] Connect to signals broadcast when `mode' (set of `POLLIN|POLLPRI|POLLOUT|POLLERR')
     * becomes true. Return the set of signals who's associated condition is currently met:
     * >> unsigned int result = 0;
     * >> FOREACH_POLL_MODE(x,mode) {
     * >>     task_connect(GET_SIGNAL_FOR(self,x));
     * >>     if (CONDITION_IS_MET(self,x))
     * >>         result |= x;
     * >> }
     * >> return result; */
    unsigned int (KCALL *f_poll)(struct file *__restrict self, unsigned int mode);

    /* [1..1] Read a directory entry from the file stream.
     * [locked(READ(self->f_node->i_lock))]
     * NOTE: The caller will have already performed permissions checks.
     * NOTE: This function must also implement the special
     *       directory entries known as `.' and `..'.
     *       The default file operations (`default_file_operations')
     *       do this by adjusting `f_pos' by +2 to free up the first 2 file
     *       positions which are used to yield symbolic directory entries
     *       based on the inodes of `f_node' and `f_path->p_parent->p_node'.
     *       However, only `.' is yielded if `f_path' is the root directory,
     *       as set by `chroot()', or the true filesystem root of the VFS.
     * @param: mode: One of `READDIR_*' (See above)
     * @return: 0 : The end of the directory has been reached.
     * @return: * : The required buffer size. (what was actually written is `MIN(return,bufsize)')
     * @throw: E_SEGFAULT:                                  The given user-buffer was faulty.
     * @throw: E_FILESYSTEM_ERROR.ERROR_FS_NOT_A_DIRECTORY: `self' isn't a directory. (Same as not defining) */
    size_t (KCALL *f_readdir)(struct file *__restrict self,
                              USER CHECKED struct dirent *buf,
                              size_t bufsize, int mode, iomode_t flags);
};

struct file {
    ATOMIC_DATA ref_t               f_refcnt;    /* Reference counter. */
    REF struct inode               *f_node;      /* [1..1][const] The open INode stream. */
    REF struct path                *f_path;      /* [1..1][const] The path that was opened. */
    struct file_operations         *f_ops;       /* [1..1][const] File operations. */
    ATOMIC_DATA pos_t               f_pos;       /* [lock(f_node->i_lock)] Absolute in-file position. */
    struct {
        atomic_rwlock_t             d_curlck;    /* Lock for `d_curent' */
        ATOMIC_DATA pos_t           d_curidx;    /* [lock(f_node->i_lock && d_curlck)] The presumed directory stream index of `d_curent'. */
        REF struct directory_entry *d_curent;    /* [lock(f_node->i_lock && d_curlck)][0..1] The f_pos-2'th directory of `f_node', or NULL. */
    }                               f_directory; /* Optional data that is not required to appear in custom file streams. */
};

/* Destroy a previously allocated File. */
FUNDEF ATTR_NOTHROW void KCALL file_destroy(struct file *__restrict self);

/* Increment/decrement the reference counter of the given file `x' */
#define file_incref(x)  ATOMIC_FETCHINC((x)->f_refcnt)
#define file_decref(x) (ATOMIC_DECFETCH((x)->f_refcnt) || (file_destroy(x),0))


/* Default file operations implementing read/write operations using `f_pread' and `f_pwrite' */
DATDEF struct file_operations default_file_operations;

/* Allocate a new, NULL-initialized file with one reference. */
FUNDEF ATTR_RETNONNULL ATTR_MALLOC REF struct file *KCALL file_alloc(void);

/* High-level open a given INode and path pair as a file stream.
 * NOTE: If `node' doesn't implement `io_file.f_open' and refers to
 *       a device, that device will be opened using `device_open()'.
 * @throw: E_NO_DEVICE: `node' is a device node (s.a. `mknod()'),
 *                       but the referenced device is missing.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR: [...] */
FUNDEF REF struct handle KCALL file_open(struct inode *__restrict node,
                                         struct path *__restrict p,
                                         oflag_t flags);

/* HINT : This function calls `directory_creatfile()', using `flags' for `open_mode'.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_PATH_NOT_FOUND:      [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:        [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:        [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ILLEGAL_PATH:        `namelen' was ZERO(0)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_ALREADY_EXISTS: [...] (Only when `O_EXCL' is set)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_DISK_FULL:           [...] */
FUNDEF REF struct handle KCALL
file_creat(struct directory_node *__restrict path_node,
           struct path *__restrict path_desc,
           CHECKED USER char const *__restrict name,
           u16 namelen, oflag_t flags,
           uid_t owner, gid_t group, mode_t mode);

/* Perform stream operations on the given file (see operators above)
 * These are the high-level wrappers that are exposed through system calls. */
FUNDEF pos_t KCALL file_seek(struct file *__restrict self, off_t off, int whence);
FUNDEF size_t KCALL file_read(struct file *__restrict self, USER CHECKED void *buf, size_t bufsize, iomode_t flags);
FUNDEF size_t KCALL file_write(struct file *__restrict self, USER CHECKED void const *buf, size_t bufsize, iomode_t flags);
FUNDEF size_t KCALL file_readdir(struct file *__restrict self, USER CHECKED struct dirent *buf, size_t bufsize, int mode, iomode_t flags);
FUNDEF ssize_t KCALL file_ioctl(struct file *__restrict self, unsigned long cmd, USER UNCHECKED void *arg, iomode_t flags);
FUNDEF unsigned int KCALL file_poll(struct file *__restrict self, unsigned int mode);


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_FILE_H */
