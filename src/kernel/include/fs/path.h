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
#ifndef GUARD_KERNEL_INCLUDE_FS_PATH_H
#define GUARD_KERNEL_INCLUDE_FS_PATH_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/sections.h>
#include <sched/task.h>
#include <endian.h>
#include <format-printer.h>

DECL_BEGIN

struct inode;
struct directory_entry;
struct directory_node;
struct superblock;
struct vfs;


#define CONFIG_PATH_CHILD_MAPSIZE   4
#define CONFIG_VFS_RECENT_CACHESIZE 32



struct path {
    /* Virtual file system location. */
    ATOMIC_DATA ref_t               p_refcnt;    /* Reference counter. */
    REF struct vfs                 *p_vfs;       /* [1..1][const][ref_if(->v_root != self)] Associated VFS. */
    REF struct path                *p_parent;    /* [0..1][const] Path of the parent directory (NULL for the VFS root directory itself).
                                                  *  NOTE: When NULL, this structure is actually a `struct vfs' object! */
    WEAK LIST_NODE(struct path)     p_siblings;  /* [lock(p_parent->p_directory.d_lock)] Chain of  */
    REF struct directory_entry     *p_dirent;    /* [0..1][const] Node directory entry (NULL for the VFS root directory itself).
                                                  *  This directory entry is part of the directory at `p_parent->p_node' */
    atomic_rwlock_t                 p_lock;      /* [order(BEFORE(p_node->i_super->s_mount_lock))]
                                                  * [order(BEFORE(p_vfs->v_mount.m_lock))]
                                                  *  Lock for accessing `p_node' */
    REF struct inode               *p_node;      /* [1..1][lock(p_lock)] INode pointed to by this path.
                                                  * NOTE: When `PATH_ISMOUNT(self)' is true, this pointer also
                                                  *       holds a reference to `p_node->i_super->s_refcnt'. */
    struct {
        REF struct inode           *m_rnode;     /* [0..1][ref_if(!= p_node)][const]
                                                  *  Real INode (the node pointed to by this
                                                  *  path before a filesystem was mounted).
                                                  *  NOTE: NULL for the VFS root path, or when
                                                  *        the mounting point is artificial. */
        REF LIST_NODE(struct path)  m_fsmount;   /* [valid_if(PATH_ISMOUNT)][CHAIN(p_node->i_super->s_mount)]
                                                  * Chain of mounting points for `p_node->i_super'. */
        LIST_NODE(struct path)      m_mount;     /* [valid_if(PATH_ISMOUNT)][CHAIN(p_vfs->v_mount)]
                                                  * Chain of mounting points within the assigned VFS. */
    }                               p_mount;     /* Mounting-related members. */
    struct {
        atomic_rwlock_t             d_lock;      /* [order(AFTER(:p_lock))]
                                                  * Lock for this directory. */
        WEAK struct path           *d_child[CONFIG_PATH_CHILD_MAPSIZE];
                                                 /* [0..1][*][lock(d_lock)]
                                                  * Hash-map of child paths (using `->p_dirent->de_hash' as hash) */
    }                               p_directory; /* Directory children. */
    u16                             p_recent;    /* [lock(:v_recent.r_lock)] Recently-used counter.
                                                  * NOTE: The recently-used system mirrors the
                                                  *       design of block-device page buffering. */
#define PATH_FNORMAL                0x0000       /* Normal path flags. */
#define PATH_FISDRIVE               0x0001       /* [lock(p_vfs->v_drives.d_lock)] This path is a DOS drive mounting point. */
#define PATH_FCLOSED                0x4000       /* [lock(WRITE_ONCE)]
                                                  * The path or VFS has been closed and can
                                                  * no longer be used as a mounting point. */
    u16                             p_flags;     /* Set of `PATH_F*' */
#if __SIZEOF_POINTER__ > 4
    u8                            __p_pad[sizeof(void *)-4];
#endif
};

#define PATH_ISVFS(x)   ((x)->p_parent == NULL)
#define PATH_ISMOUNT(x) ((x)->p_node != (x)->p_mount.m_rnode)

/* Increment/decrement the reference counter of the given path `x' */
#define path_incref(x)  ATOMIC_FETCHINC((x)->p_refcnt)
#define path_decref(x) (ATOMIC_DECFETCH((x)->p_refcnt) || (path_destroy(x),0))

/* Destroy a previously allocated path. */
FUNDEF ATTR_NOTHROW void KCALL
path_destroy(struct path *__restrict self);


/* Return a reference to the child of a given path.
 * NOTE: This function also caches the returned path
 *       as recently-used in the accompanying VFS.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_PATH_NOT_FOUND:  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_NOT_A_DIRECTORY: [...] */
FUNDEF ATTR_RETNONNULL REF struct path *KCALL
path_child(struct path *__restrict self,
           USER CHECKED char const *path, u16 pathlen);
/* Same as `path_child()', but ignore casing when matching filenames. */
FUNDEF ATTR_RETNONNULL REF struct path *KCALL
path_casechild(struct path *__restrict self,
               USER CHECKED char const *path, u16 pathlen);

/* Similar to `path_child()', but should be used to convert
 * newly constructed nodes and associated directory entries
 * into paths.
 * NOTE: Because of the rare race condition of `self' having
 *       been turned into a mounting point since the caller
 *       originally acquired the associated node, when `self_node'
 *       no longer matches the node pointed to by `self', the
 *       resulting path is created as a ghost path that is not
 *       registered as a child of `self' (meaning that `path_child()'
 *       will not return it), though it will still have its
 *       parent field set to `self', just as one would expect
 *       if the resulting path would have been created before
 *       `self' had been turned into a mounting point.
 * NOTE: The secondary race condition that could arise when
 *       another thread already called `path_child()' with
 *       the same name as `child_entry' after the node that
 *       the caller created for `child_node' had already been
 *       added to the filesystem, this function will still
 *       return the same path node, as was returned during
 *       the previous call to `path_child()' */
FUNDEF ATTR_RETNONNULL REF struct path *KCALL
path_newchild(struct path *__restrict self,
              struct directory_node *__restrict self_node,
              struct inode *__restrict child_node,
              struct directory_entry *__restrict child_entry);

/* Try to remove the given `child_entry' from the given path.
 * This function is called by `directory_remove()' when the
 * passed `self_path' was non-NULL. */
FUNDEF void KCALL
path_delchild(struct path *__restrict self,
              struct directory_node *__restrict self_node,
              struct directory_entry *__restrict child_entry);

/* Mount the given `node' under `self'.
 * NOTE: The given node is usually the root-directory node
 *       of a superblock. However, it can also be any other
 *       node, in which case that node will be mounted, while
 *       the path will still be registered in the superblock's
 *       mounting list, meaning that calling
 *       >> `superblock_umount_all(node->i_super)'
 *       will unmount this mounting point.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_ALREADY_EXISTS:
 *         The given path already is a mounting point for some filesystem.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_NOT_FOUND:
 *         The `INODE_FCLOSED' flag has been set for `node', or the
 *        `SUPERBLOCK_FCLOSED' flag has been set for `node->i_super', or
 *          the `PATH_FCLOSED' flag has been set for `self' or `self->p_vfs' */
FUNDEF void KCALL
path_mount(struct path *__restrict self,
           struct inode *__restrict node);

/* Unbound a filesystem previously mounted under `self'
 * @return: true:  Unmounted the superblock previously mounted under `self'.
 * @return: false: `self' wasn't a mounting point. */
FUNDEF bool KCALL path_umount(struct path *__restrict self);

/* Print the name of the given path, according to the specified `type'.
 * @param: type: One of `REALPATH_F*', optionally or'd with `REALPATH_FDOSPATH'
 * @return: < 0: The first negative value returned by `printer'
 * @return: * :  The sum of all calls to `printer'.
 * @throw: E_INVALID_ARGUMENT: The given `type' is invalid. */
FUNDEF ssize_t KCALL
path_print(struct path *__restrict self, pformatprinter printer,
           void *closure, unsigned int flags);

/* Using `path_print()', fill in the given user-space buffer before
 * appending a NUL-character and returning the number of required
 * characters (excluding the NUL-character).
 * @param: type: One of `REALPATH_F*', optionally or'd with `REALPATH_FDOSPATH' */
FUNDEF size_t KCALL
path_getname(struct path *__restrict self,
             USER CHECKED char *buf,
             size_t bufsize, unsigned int type);


/* Unmount all bindings for a superblock or vfs.
 * WARNING: Unless the `PATH_FCLOSED' flag for `vfs_umount_all()', or
 *          the `SUPERBLOCK_FCLOSED' flag for `superblock_umount_all()'
 *          have been set, new mounting mounts may have already appeared
 *          by the time these functions return.
 * @return: * : The number of mounting points that have been removed. */
FUNDEF size_t KCALL vfs_umount_all(struct vfs *__restrict self);
FUNDEF size_t KCALL superblock_umount_all(struct superblock *__restrict self);




#define VFS_DRIVECOUNT  (('Z'-'A')+1)
struct vfs_drives {
    atomic_rwlock_t             d_lock;    /* Lock of DOS drive mounting points. */
    REF struct path            *d_drives[VFS_DRIVECOUNT]; /* [0..1][lock(d_lock)][*] List of drives. */
};


struct vfs {
    /* Virtual file system controller (s.a. `CLONE_NEWNS'). */
    struct path                 v_root;    /* The root mounting point. */
    ATOMIC_DATA ref_t           v_fscount; /* The number of `struct fs' objects using this VFS.
                                            * When this counter is ZERO(0), the VFS's cache must
                                            * be cleared whenever its regular reference counter is
                                            * decremented.
                                            * Additionally, when this counter hits ZERO(0), all DOS
                                            * drives are unmounted, and new ones can only be mounted
                                            * when the counter is non-ZERO(0). */
    struct {
        atomic_rwlock_t         r_lock;    /* Lock for the recent-subsystem. */
        REF struct path        *r_recent[CONFIG_VFS_RECENT_CACHESIZE];
                                           /* [lock(r_lock)][1..1][r_size]
                                            * Vector of recently used paths.
                                            * NOTE: This cache mirrors the design of
                                            *       the page-buffer of block devices. */
        u16                     r_size;    /* [<= CONFIG_VFS_RECENT_CACHESIZE]
                                            * [lock(r_lock)] Amount of recently used paths. */
    }                           v_recent;  /* Recently used path cache. */
    struct {
        atomic_rwlock_t         m_lock;    /* [order(BEFORE(struct superblock::s_mount_lock))]
                                            * [order(AFTER(struct path::v_mount::m_lock))]
                                            * Lock for the chain of mounting points. */
        LIST_HEAD(struct path)  m_chain;   /* [0..1][lock(m_lock)] Chain of active mounting points. */
    }                           v_mount;   /* VFS mounting point information. */
    struct vfs_drives           v_drives;  /* DOS Drive mounting points. */
};



/* [1..1] Per-thread VFS mounting namespace. */
#define THIS_VFS   (THIS_FS->fs_vfs)

/* Increment/decrement the reference counter of the given vfs `x' */
#define vfs_incref(x)  path_incref(&(x)->v_root)
#define vfs_decref(x) ((ATOMIC_READ((x)->v_fscount) || (vfs_clear_recent(x),0)),path_decref(&(x)->v_root))

/* Increment/decrement the filesystem usage counter of the given vfs `x' */
#define vfs_incfscount(x)  ATOMIC_FETCHINC((x)->v_fscount)
#define vfs_decfscount(x) (ATOMIC_DECFETCH((x)->v_fscount) || (vfs_unbind_drives(x),0))

/* Allocate a new VFS object and pre-initialize it to unmounted. */
FUNDEF ATTR_MALLOC ATTR_RETNONNULL REF struct vfs *KCALL vfs_alloc(void);

/* Create a deep copy of the current VFS (used when `CLONE_NEWNS' is set) */
FUNDEF ATTR_MALLOC ATTR_RETNONNULL REF struct vfs *KCALL vfs_clone(void);

/* Clear the cache of recently used VFS paths. */
FUNDEF ATTR_NOTHROW void KCALL vfs_clear_recent(struct vfs *__restrict self);

/* Remove the given path `p' from the set of recently used path.
 * No-op if the given path `p' hasn't been used recently. */
FUNDEF ATTR_NOTHROW void KCALL vfs_remove_recent(struct path *__restrict p);



/* DOS Drive binding support. */

/* Bind DOS drives.
 * If another drive had already been bound to the
 * same `drive_number', the old path is overwritten.
 * The caller is required to ensure that `location->p_vfs->v_fscount' is non-ZERO.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR: The calling this isn't allowed to bind or unbind DOS drives.
 * @param: drive_number: The drive number (0..VFS_DRIVECOUNT-1)
 * @param: location:     The path that should be bound to a DOS drive. */
FUNDEF void KCALL vfs_bind_drive(u8 drive_number, struct path *__restrict location);
/* Unbound a DOS drive.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR: The calling this isn't allowed to bind or unbind DOS drives.
 * @return: true:  The drive has been unbound.
 * @return: false: The drive was never bound to begin with. */
FUNDEF bool KCALL vfs_unbind_drive(struct vfs *__restrict self, u8 drive_number);
/* Return a reference to the path bound to the DOS drive `drive_number' in the current VFS.
 * @param: drive_number:                               The drive number (0..VFS_DRIVECOUNT-1)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_PATH_NOT_FOUND: The drive indexed by `drive_number' wasn't bound. */
FUNDEF ATTR_RETNONNULL REF struct path *KCALL vfs_drive(u8 drive_number);

/* Unmount all DOS drives. */
FUNDEF ATTR_NOTHROW void KCALL vfs_unbind_drives(struct vfs *__restrict self);


#define CONFIG_FS_UMASK_DEFAULT   0 /* XXX: Is this correct? I've already forgotten how this works again... */
#define CONFIG_FS_LNKMAX_DEFAULT  32



/* Filesystem mode flags (NOTE: These match the `AT_*' flags from <bits/fcntl-linux.h>). */
#define FS_MODE_FNORMAL                   0x00000000 /* Operate normally. */
#define FS_MODE_FDIRECTORY                0x00000001 /* Open a directory. */
#define FS_MODE_FIGNORE_TRAILING_SLASHES  0x00000002 /* Ignore empty trailing path segments. */
#define FS_MODE_FSYMLINK_NOFOLLOW         0x00000100 /* Do not follow symbolic links. */
#define FS_MODE_FNO_AUTOMOUNT             0x00000800 /* Suppress terminal automount traversal. */
#define FS_MODE_FEMPTY_PATH               0x00001000 /* Allow empty relative pathname. */
#define FS_MODE_FDOSPATH                  0x00100000 /* Interpret '\\' as '/', and ignore casing during path resolution. */
#define FS_MODE_FALWAYS0MASK              0x000000ff /* Mask of bits always 0 in `fs_atmask' */
#define FS_MODE_FALWAYS1MASK              0xffefe100 /* Mask of bits always 1 in `fs_atmask' */
#define FS_MODE_FALWAYS0FLAG              0xffefe6ff /* Mask of bits always 0 in `fs_atflag' */
#define FS_MODE_FALWAYS1FLAG              0x00000000 /* Mask of bits always 1 in `fs_atflag' */
#define FS_MODE_FKNOWNBITS   (~FS_MODE_FALWAYS0FLAG) /* Mask of known, general-purpose user-mode path flags */

/* Use this macro to mask user-space filesystem mode flags. */
#define FS_ATMODE(x)      (((x) & THIS_FS->fs_atmask) | THIS_FS->fs_atflag)
#define FS_DEFAULT_ATMODE                              (THIS_FS->fs_atflag)


union fs_mask {
    ATOMIC_DATA u64     fs_mode;   /* File system operations mode. */
    struct PACKED {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        ATOMIC_DATA u32 fs_lo;
        ATOMIC_DATA u32 fs_hi;
#else
        ATOMIC_DATA u32 fs_hi;
        ATOMIC_DATA u32 fs_lo;
#endif
        ATOMIC_DATA u32 fs_atmask; /* File system operations mode mask (Set of negated `FS_MODE_F*'). */
        ATOMIC_DATA u32 fs_atflag; /* File system operations mode flags (Set of negated `FS_MODE_F*'). */
    };
};

struct fs {
    /* Filesystem information (s.a. `CLONE_FS'). */
    ATOMIC_DATA ref_t       fs_refcnt; /* Reference counter. */
#ifdef __INTELLISENSE__
        struct vfs         *fs_vfs;    /* [1..1][const] The virtual file system mounting namespace.
                                        *  NOTE: This pointer also holds a reference to the VFS's
                                        *        `v_fscount' reference counter. */
#else
    REF struct vfs         *fs_vfs;    /* [1..1][const] The virtual file system mounting namespace.
                                        *  NOTE: This pointer also holds a reference to the VFS's
                                        *        `v_fscount' reference counter. */
#endif
    atomic_rwlock_t         fs_lock;   /* Lock for access filesystem information. */
    REF struct path        *fs_root;   /* [1..1][lock(fs_lock)] Filesystem root directory. */
    REF struct path        *fs_cwd;    /* [1..1][lock(fs_lock)] Current working directory. */
    REF struct path        *fs_dcwd[VFS_DRIVECOUNT]; /* [0..1][lock(fs_lock)][*] List of per-drive current working directories. */
    ATOMIC_DATA mode_t      fs_umask;  /* The currently effective UMASK.
                                        * NOTE: All bits not masked by `0777' _MUST_ always be ZERO(0)! */
    WEAK u16                fs_lnkmax; /* The max number of symbolic links allowed during resolution of a path.
                                        * This field defaults to `CONFIG_FS_LNKMAX_DEFAULT'. */
    u16                   __fs_pad;    /* ... */
    union PACKED {
        ATOMIC_DATA u64     fs_mode;   /* File system operations mode. */
        struct PACKED {
#if __BYTE_ORDER == __LITTLE_ENDIAN
            ATOMIC_DATA u32 fs_atmask; /* File system operations mode mask (Set of negated `FS_MODE_F*'). */
            ATOMIC_DATA u32 fs_atflag; /* File system operations mode flags (Set of negated `FS_MODE_F*'). */
#else
            ATOMIC_DATA u32 fs_atmask; /* File system operations mode mask (Set of negated `FS_MODE_F*'). */
            ATOMIC_DATA u32 fs_atflag; /* File system operations mode flags (Set of negated `FS_MODE_F*'). */
#endif
        };
    };
};

/* Allocate a new FS object and pre-initialize it to NULL, except for the following:
 *    - fs_umask  = CONFIG_FS_UMASK_DEFAULT
 *    - fs_lnkmax = CONFIG_FS_LNKMAX_DEFAULT */
FUNDEF ATTR_MALLOC ATTR_RETNONNULL REF struct fs *KCALL fs_alloc(void);

/* Create a deep copy of the current VFS (used when `CLONE_FS' isn't set) */
FUNDEF ATTR_MALLOC ATTR_RETNONNULL REF struct fs *KCALL fs_clone(void);


/* Destroy a previously allocated FS. */
FUNDEF void KCALL fs_destroy(struct fs *__restrict self);

/* Increment/decrement the reference counter of the given FS `x' */
#define fs_incref(x)  ATOMIC_FETCHINC((x)->fs_refcnt)
#define fs_decref(x) (ATOMIC_DECFETCH((x)->fs_refcnt) || (fs_destroy(x),0))

/* The kernel's own filesystem / the filesystem used when running `/bin/init'. */
DATDEF struct fs  fs_kernel;
DATDEF struct vfs vfs_kernel;

/* [1..1] Per-thread filesystem information.
 * NOTE: Initialized to NULL. - Must be initialized before the task is started. */
DATDEF ATTR_PERTASK REF struct fs *_this_fs;
#define THIS_FS     PERTASK_GET(_this_fs)


/* Return a reference for a given path.
 * Traverse a given path, using the current CWD as starting point:
 * >> directory_traverse(my_directory,"foo /bar ///baz/../xy");
 * This function handles multiple consecutive slashes, as well as
 * whitespace surrounding them, as well as following symbolic links.
 * NOTE: This function also deals with root-directory
 *       referencing causing by leading slashes.
 * @param: cwd:  When NULL, replace with `fs_getcwd()'
 *               -> The directory used for resolving relative paths.
 * @param: flags: Set of `FS_MODE_F*' flags. The following flags affect behavior:
 *                - `FS_MODE_FDIRECTORY':
 *                                 Throw an `ERROR_FS_NOT_A_DIRECTORY' error
 *                                 if the final path isn't a directory, even when the
 *                                 given `path' does not end with any trailing slashes.
 *                - `FS_MODE_FEMPTY_PATH':
 *                                 If `path' is empty, re-return `cwd'.
 *                                 When `path' is empty and this flag isn't set,
 *                                 an `E_FILESYSTEM_ERROR.ERROR_FS_PATH_NOT_FOUND'
 *                                 error is thrown instead.
 *                - `FS_MODE_FSYMLINK_NOFOLLOW':
 *                                 Do not follow symbolic links, but throw an
 *                                `E_FILESYSTEM_ERROR.ERROR_FS_TOO_MANY_LINKS'
 *                                 error instead, when any are encountered.
 * @param: pnode: The final node of the returned path that was checked
 *                for being a directory when `O_DIRECTORY' was passed,
 *                or the given path ended with a trailing slash.
 *                Usually, this should be the same as `return->p_node',
 *                however due to race conditions, another thread may
 *                have created a mounting point in the mean time that
 *                would have overwritten the actual node checked.
 * @throw: E_SEGFAULT: The given `path' is faulty.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_PATH_NOT_FOUND:
 *             Some component of the given `path' doesn't exist.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_NOT_A_DIRECTORY:
 *             Some part of the given path isn't a directory, or
 *             the path ends with a trailing slash and the last
 *             component isn't a path, or `O_DIRECTORY' was given
 *             and the last component isn't a directory, regardless
 *             of whether or not the path ends with a trailing slash.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_TOO_MANY_LINKS:
 *             More than `mode&O_NOFOLLOW ? 0 : THIS_FS->fs_lnkmax'
 *             have been encountered while resolving the given path.
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILENAME_TOO_LONG:
 *             At least one segment of the given path is longer than 2^16-1 characters. */
FUNDEF ATTR_RETNONNULL REF struct path *KCALL
fs_path(struct path *cwd, USER CHECKED char const *path,
        size_t pathlen, REF struct inode **pnode, int flags);
/* Same as `fs_path()', but use the CWD associated with the handle `dfd' */
FUNDEF ATTR_RETNONNULL REF struct path *KCALL
fs_pathat(fd_t dfd, USER CHECKED char const *path,
          size_t pathlen, REF struct inode **pnode, int flags);



/* Similar to `fs_path()', but stop just before the last segment and
 * instead update `*ppath' and `**ppathlen' to point to said segment:
 * @param: flags: Set of `FS_MODE_F*' flags. The following flags affect behavior:
 *                - ... Same as `fs_path()'
 *                - `FS_MODE_FIGNORE_TRAILING_SLASHES':
 *                                 Ignore trailing slashes that result in
 *                                 empty path segments at the end of the path.
 *                                 This flag should be set when parsing the
 *                                 path of an `mkdir()' system call, thus allowing
 *                                 the use of `mkdir("/bin/")' as alias for `mkdir("/bin")'.
 * @throw: ERROR_FS_FILENAME_TOO_LONG: The last path segment is too long.
 * @assume(POST(*ppathlen <= 0xffff));
 * >> fs_lastpath("/foo/bar/foobar")
 * RETURN: fs_path("/foo/bar")
 * *ppath: "foobar" */
FUNDEF ATTR_RETNONNULL REF struct path *KCALL
fs_lastpath(struct path *cwd, USER CHECKED char const **__restrict ppath,
            size_t *__restrict ppathlen, REF struct inode **pnode, int flags);
/* Same as `fs_lastpath()', but use the CWD associated with the handle `dfd' */
FUNDEF ATTR_RETNONNULL REF struct path *KCALL
fs_lastpathat(fd_t dfd, USER CHECKED char const **__restrict ppath,
              size_t *__restrict ppathlen, REF struct inode **pnode, int flags);

/* Set the current working/root directory of the VFS of the calling thread.
 * NOTE: These functions invalidate certain caches that are
 *       automatically constructed in order to speed up `vfs_path()'. */
FUNDEF void KCALL fs_chdir(struct path *__restrict new_path);
FUNDEF void KCALL fs_chroot(struct path *__restrict new_path);

/* Return the current working / root directories. */
FUNDEF ATTR_RETNONNULL REF struct path *KCALL fs_getcwd(void);
FUNDEF ATTR_RETNONNULL REF struct path *KCALL fs_getroot(void);


/* TODO... */
#define fs_getuid() 0
#define fs_getgid() 0


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_PATH_H */
