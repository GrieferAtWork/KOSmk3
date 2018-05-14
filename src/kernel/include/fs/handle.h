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
#ifndef GUARD_KERNEL_INCLUDE_FS_HANDLE_H
#define GUARD_KERNEL_INCLUDE_FS_HANDLE_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/handle.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/sections.h>
#include <kos/safecall.h>
#include <sched/task.h>
#include <fs/iomode.h>
#include <endian.h>

DECL_BEGIN

/* Symbolic handle numbers. */
#define HANDLE_SYMBOLIC_CWD                    (-100) /* AT_FDCWD */
#define HANDLE_SYMBOLIC_ROOT                   (-101) /* AT_FDROOT */
#define HANDLE_SYMBOLIC_THIS_TASK              (-180) /* AT_THIS_TASK (Also serves as THIS_FS, THIS_MODULE, THIS_APPLICATION, THIS_VM) */
#define HANDLE_SYMBOLIC_DRIVE_CWD(drivechar)  ((-350)+((drivechar)-'A'))
#define HANDLE_SYMBOLIC_DRIVE_ROOT(drivechar) ((-300)+((drivechar)-'A'))

#ifdef __CC__
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define HANDLE_MODE(type,flags) ((type) | ((flags) << 16))
#else
#define HANDLE_MODE(type,flags) (((type) << 16) | (flags))
#endif

struct vm_region;
struct PACKED handle {
    union PACKED {
        struct PACKED {
            u16                             h_type;                /* [const] Handle type (One of `HANDLE_TYPE_F*') */
            iomode_t                        h_flag;                /* [lock(:hm_lock)] I/O mode flags (Set of `IO_*') */
        };
        uintptr_t                           h_mode;                /* Handle mode (constructed from `HANDLE_MODE'). */
        uintptr_t                         __h_pad;                 /* Padding. */
    };
    union PACKED {
        REF void                           *h_ptr;                 /* [1..1][const] Pointer to whatever object is described by this handle. */
        union PACKED {
            REF struct device              *o_device;              /* [1..1][const][HANDLE_TYPE_FDEVICE] */
            REF struct block_device        *o_block_device;        /* [1..1][const][HANDLE_TYPE_FDEVICE] */
            REF struct character_device    *o_character_device;    /* [1..1][const][HANDLE_TYPE_FDEVICE] */
            REF struct inode               *o_inode;               /* [1..1][const][HANDLE_TYPE_FINODE] */
            REF struct regular_node        *o_regular_node;        /* [1..1][const][HANDLE_TYPE_FINODE] */
            REF struct directory_node      *o_directory_node;      /* [1..1][const][HANDLE_TYPE_FINODE] */
            REF struct symlink_node        *o_symlink_node;        /* [1..1][const][HANDLE_TYPE_FINODE] */
            REF struct file                *o_file;                /* [1..1][const][HANDLE_TYPE_FFILE] */
            REF struct superblock          *o_superblock;          /* [1..1][const][HANDLE_TYPE_FSUPERBLOCK] */
            REF struct directory_entry     *o_directory_entry;     /* [1..1][const][HANDLE_TYPE_FDIRECTORY_ENTRY] */
            REF struct path                *o_path;                /* [1..1][const][HANDLE_TYPE_FPATH] */
            REF struct vfs                 *o_vfs;                 /* [1..1][const][HANDLE_TYPE_FPATH] */
            REF struct fs                  *o_fs;                  /* [1..1][const][HANDLE_TYPE_FFS] */
            REF struct module              *o_module;              /* [1..1][const][HANDLE_TYPE_FMODULE] */
            REF struct application         *o_application;         /* [1..1][const][HANDLE_TYPE_FAPPLICATION] */
            REF struct driver              *o_driver;              /* [1..1][const][HANDLE_TYPE_FAPPLICATION] */
            REF struct task_weakref        *o_thread;              /* [1..1][const][HANDLE_TYPE_FTHREAD] */
            REF struct vm                  *o_vm;                  /* [1..1][const][HANDLE_TYPE_FVM] */
            REF struct vm_region           *o_vm_region;           /* [1..1][const][HANDLE_TYPE_FVM_REGION] */
            REF struct pipe                *o_pipe;                /* [1..1][const][HANDLE_TYPE_FPIPE] */
            REF struct pipereader          *o_pipereader;          /* [1..1][const][HANDLE_TYPE_FPIPEREADER] */
            REF struct pipewriter          *o_pipewriter;          /* [1..1][const][HANDLE_TYPE_FPIPEWRITER] */
            REF struct socket              *o_socket;              /* [1..1][const][HANDLE_TYPE_FSOCKET] */
            REF struct futex               *o_futex;               /* [1..1][const][HANDLE_TYPE_FFUTEX] */
            REF struct futex_handle        *o_futex_handle;        /* [1..1][const][HANDLE_TYPE_FFUTEX_HANDLE] */
            REF struct device_stream       *o_device_stream;       /* [1..1][const][HANDLE_TYPE_FDEVICE_STREAM] */
        }                                   h_object;              /* [const] The object pointed to by this handle. */
    };
};

struct dirent;
struct stat64;

struct handle_types_struct {
    /* Handle operators (NOTE: All of these are [1..1][const])
     * Unsupported operators simply throw an `E_NOT_IMPLEMENTED' error. */
    /*ATTR_NOTHROW*/void (KCALL *ht_incref[HANDLE_TYPE_FCOUNT])(void *__restrict ptr);
    /*ATTR_NOTHROW*/void (KCALL *ht_decref[HANDLE_TYPE_FCOUNT])(void *__restrict ptr);
    /* NOTE: The caller must validate if read/write operations are even allowed (according to `IO_ACCMODE' of the handle's flags) */
    size_t               (KCALL *ht_read[HANDLE_TYPE_FCOUNT])(void *__restrict ptr, USER CHECKED void *buf, size_t bufsize, iomode_t flags);
    size_t               (KCALL *ht_write[HANDLE_TYPE_FCOUNT])(void *__restrict ptr, USER CHECKED void const *buf, size_t bufsize, iomode_t flags);
    pos_t                (KCALL *ht_seek[HANDLE_TYPE_FCOUNT])(void *__restrict ptr, off_t off, int whence);
    size_t               (KCALL *ht_readdir[HANDLE_TYPE_FCOUNT])(void *__restrict ptr, USER CHECKED struct dirent *buf, size_t bufsize, int mode, iomode_t flags);
    size_t               (KCALL *ht_pread[HANDLE_TYPE_FCOUNT])(void *__restrict ptr, USER CHECKED void *buf, size_t bufsize, pos_t pos, iomode_t flags);
    size_t               (KCALL *ht_pwrite[HANDLE_TYPE_FCOUNT])(void *__restrict ptr, USER CHECKED void const *buf, size_t bufsize, pos_t pos, iomode_t flags);
    void                 (KCALL *ht_truncate[HANDLE_TYPE_FCOUNT])(void *__restrict ptr, pos_t new_smaller_size);
    /* Allocate disk space.
     * @throw: E_INVALID_ARGUMENT: The given `mode' isn't supported (same as not implementing this operator) */
    void                 (KCALL *ht_allocate[HANDLE_TYPE_FCOUNT])(void *__restrict ptr, int mode, pos_t start, pos_t length, iomode_t flags);
    ssize_t              (KCALL *ht_ioctl[HANDLE_TYPE_FCOUNT])(void *__restrict ptr, unsigned long cmd, USER UNCHECKED void *arg, iomode_t flags);
    /* HINT: `ht_sync(...,false)' --> `fsync()'
     *       `ht_sync(...,true)'  --> `fdatasync()' */
    void                 (KCALL *ht_sync[HANDLE_TYPE_FCOUNT])(void *__restrict ptr, bool only_data);
    void                 (KCALL *ht_stat[HANDLE_TYPE_FCOUNT])(void *__restrict ptr, USER CHECKED struct stat64 *result);
    /* Connect to signals broadcast when `mode' (set of `POLLIN|POLLPRI|POLLOUT|POLLERR') becomes true.
     * Return the set of signals who's associated condition is currently met:
     * >> unsigned int result = 0;
     * >> FOREACH_POLL_MODE(x,mode) {
     * >>     task_connect(GET_SIGNAL_FOR(self,x));
     * >>     if (CONDITION_IS_MET(self,x))
     * >>         result |= x;
     * >> }
     * >> return result;
     * NOTE: Not implementing this operator is
     *       the same as always returning ZERO(0).
     */
    unsigned int         (KCALL *ht_poll[HANDLE_TYPE_FCOUNT])(void *__restrict ptr, unsigned int mode);
};

DATDEF struct handle_types_struct const handle_types;

/* Invoke generic operations on a given handle `x' */
#define handle_incref(x)                          SAFECALL_KCALL_VOID_1(handle_types.ht_incref[(x).h_type],(x).h_ptr)
#define handle_decref(x)                          SAFECALL_KCALL_VOID_1(handle_types.ht_decref[(x).h_type],(x).h_ptr)
#define handle_read(x,buf,bufsize)                SAFECALL_KCALL_4(handle_types.ht_read[(x).h_type],(x).h_ptr,buf,bufsize,(x).h_flag)
#define handle_readf(x,buf,bufsize,flags)         SAFECALL_KCALL_4(handle_types.ht_read[(x).h_type],(x).h_ptr,buf,bufsize,flags)
#define handle_write(x,buf,bufsize)               SAFECALL_KCALL_4(handle_types.ht_write[(x).h_type],(x).h_ptr,buf,bufsize,(x).h_flag)
#define handle_writef(x,buf,bufsize,flags)        SAFECALL_KCALL_4(handle_types.ht_write[(x).h_type],(x).h_ptr,buf,bufsize,flags)
#define handle_seek(x,off,whence)                 (*handle_types.ht_seek[(x).h_type])((x).h_ptr,off,whence)
#define handle_readdir(x,buf,bufsize,mode)        SAFECALL_KCALL_5(handle_types.ht_readdir[(x).h_type],(x).h_ptr,buf,bufsize,mode,(x).h_flag)
#define handle_readdirf(x,buf,bufsize,mode,flags) SAFECALL_KCALL_5(handle_types.ht_readdir[(x).h_type],(x).h_ptr,buf,bufsize,mode,flags)
#define handle_pread(x,buf,bufsize,pos)           (*handle_types.ht_pread[(x).h_type])((x).h_ptr,buf,bufsize,pos,(x).h_flag)
#define handle_preadf(x,buf,bufsize,pos,flags)    (*handle_types.ht_pread[(x).h_type])((x).h_ptr,buf,bufsize,pos,flags)
#define handle_pwrite(x,buf,bufsize,pos)          (*handle_types.ht_pwrite[(x).h_type])((x).h_ptr,buf,bufsize,pos,(x).h_flag)
#define handle_pwritef(x,buf,bufsize,pos,flags)   (*handle_types.ht_pwrite[(x).h_type])((x).h_ptr,buf,bufsize,pos,flags)
#define handle_truncate(x,new_smaller_size)       SAFECALL_KCALL_VOID_2(handle_types.ht_truncate[(x).h_type],(x).h_ptr,new_smaller_size)
#define handle_allocate(x,mode,start,length)      SAFECALL_KCALL_VOID_5(handle_types.ht_allocate[(x).h_type],(x).h_ptr,mode,start,length,(x).h_flag)
#define handle_ioctl(x,cmd,arg)                   SAFECALL_KCALL_4(handle_types.ht_ioctl[(x).h_type],(x).h_ptr,cmd,arg,(x).h_flag)
#define handle_ioctlf(x,cmd,arg,flags)            SAFECALL_KCALL_4(handle_types.ht_ioctl[(x).h_type],(x).h_ptr,cmd,arg,flags)
#define handle_sync(x,only_data)                  SAFECALL_KCALL_VOID_2(handle_types.ht_sync[(x).h_type],(x).h_ptr,only_data)
#define handle_stat(x,result)                     SAFECALL_KCALL_VOID_2(handle_types.ht_stat[(x).h_type],(x).h_ptr,result)
#define handle_poll(x,mode)                       SAFECALL_KCALL_2(handle_types.ht_poll[(x).h_type],(x).h_ptr,mode)



/* The default limit for the max number of handles
 * that are allowed in a single handle manager. */
#ifndef CONFIG_HANDLE_MANAGER_DEFAULT_LIMIT
#define CONFIG_HANDLE_MANAGER_DEFAULT_LIMIT  0x1000
#endif


struct handle_manager {
    /* Handle manager.
     * WARNING: The handle-manager system isn't PF-safe, meaning that
     *          code which may be called from page-fault handlers is
     *          not allowed to use it. */
    ATOMIC_DATA ref_t           hm_refcnt; /* Handle manager reference counter. */
    atomic_rwlock_t             hm_lock;   /* Lock for accessing this handle manager. */
    WEAK unsigned int           hm_limit;  /* Max number of handles that may be allocated. */
    unsigned int                hm_alloc;  /* [lock(hm_lock)] Allocated number of handles. */
    unsigned int                hm_count;  /* [lock(hm_lock)] Amount of handles currently in use. */
    struct handle              *hm_vector; /* [lock(hm_lock)][0..hm_alloc][owned] Vector of owned handles (unused handles have `HANDLE_TYPE_FNONE' set as type) */
#define HANDLE_MANAGER_FNORMAL  0x0000     /* Normal handle manager flags. */
    u16                         hm_flags;  /* [lock(hm_lock)] Set of `HANDLE_MANAGER_F*' */
};


/* Destroy a previously allocated handle_manager. */
FUNDEF void KCALL handle_manager_destroy(struct handle_manager *__restrict self);

/* Increment/decrement the reference counter of the given handle_manager `x' */
#define handle_manager_incref(x)  ATOMIC_FETCHINC((x)->hm_refcnt)
#define handle_manager_decref(x) (ATOMIC_DECFETCH((x)->hm_refcnt) || (handle_manager_destroy(x),0))

/* The handle manager of the kernel itself. */
DATDEF struct handle_manager handle_manager_kernel;

/* [1..1][lock(PRIVATE(THIS_TASK))] Handle manager of the calling thread. */
DATDEF ATTR_PERTASK struct handle_manager *_this_handle_manager;
#define THIS_HANDLE_MANAGER    PERTASK_GET(_this_handle_manager)


/* Allocate a new, empty handle manager. */
FUNDEF ATTR_RETNONNULL ATTR_MALLOC
REF struct handle_manager *KCALL handle_manager_alloc(void);

/* Clone the current handle manager, but omit all
 * handles with the `IO_HANDLE_FCLOFORK' flag set. */
FUNDEF ATTR_RETNONNULL ATTR_MALLOC
REF struct handle_manager *KCALL handle_manager_clone(void);

/* Close all handles with the `IO_HANDLE_FCLOEXEC'
 * flag set in the current handle manager. */
FUNDEF void KCALL handle_manager_close_exec(void);

/* Try to close the handle associated with `fd'.
 * @return: true:  The handle under `fd' was closed.
 * @return: false: No handle was associated with `fd'. */
FUNDEF bool KCALL handle_close(fd_t fd);

/* Close all file handles within the given range. */
FUNDEF unsigned int KCALL handle_closeall(unsigned int minfd, unsigned int maxfd);

/* Duplicate the handle `fd' and return a new descriptor for it.
 * @param: flags:                            Set of `IO_HANDLE_F*'
 * @throw: E_INVALID_HANDLE:                 The given `fd' is an invalid handle.
 * @throw: E_BADALLOC.ERROR_BADALLOC_HANDLE: Too many open handles. */
FUNDEF unsigned int KCALL handle_dup(fd_t fd, iomode_t flags);

/* Similar to `handle_dup()', but duplicate into a file
 * descriptor slot that is greater than, or equal to `hint'.
 * @param: flags:                            Set of `IO_HANDLE_F*'
 * @throw: E_INVALID_HANDLE:                 The given `fd' is an invalid handle.
 * @throw: E_BADALLOC.ERROR_BADALLOC_HANDLE: `hint' is great than the max allowed handle number.
 * @throw: E_BADALLOC.ERROR_BADALLOC_HANDLE: Too many open handles. */
FUNDEF unsigned int KCALL handle_dupat(fd_t fd, unsigned int hint, iomode_t flags);

/* Duplicate `fd' into `dfd'.
 * @param: flags:            Set of `IO_HANDLE_F*'
 * @throw: E_INVALID_HANDLE: The given `fd' is an invalid handle.
 * @throw: E_INVALID_HANDLE: The given `dfd' is too large. */
FUNDEF void KCALL handle_dupinto(fd_t fd, fd_t dfd, iomode_t flags);

/* Add the given handle to the handle manager and
 * return the handle number of where it was placed.
 * @throw: E_BADALLOC.ERROR_BADALLOC_HANDLE: Too many open handles. */
FUNDEF unsigned int KCALL handle_put(struct handle hnd);

/* Similar to `handle_put()', but place the handle in a
 * descriptor slot that is greater than, or equal to `hint'.
 * @throw: E_BADALLOC.ERROR_BADALLOC_HANDLE: `hint' is great than the max allowed handle number.
 * @throw: E_BADALLOC.ERROR_BADALLOC_HANDLE: Too many open handles. */
FUNDEF unsigned int KCALL handle_putat(struct handle hnd, unsigned int hint);

/* Place the given handle into the specified handler slot.
 * @throw: E_INVALID_HANDLE: The given `dfd' is too large. */
FUNDEF void KCALL handle_putinto(fd_t dfd, struct handle hnd);

/* The kernel-space equivalent of the user-space `fcntl()' function. */
FUNDEF syscall_slong_t KCALL
handle_fcntl(fd_t fd, unsigned int cmd, UNCHECKED USER void *arg);



/* Lookup handles, given their FileDescriptor number.
 * The following implicit conversions are performed:
 *    - HANDLE_TYPE_FFILE          --> HANDLE_TYPE_FINODE
 *    - HANDLE_TYPE_FFILE          --> HANDLE_TYPE_FPATH
 *    - HANDLE_TYPE_FPATH          --> HANDLE_TYPE_FINODE
 *    - HANDLE_TYPE_FSUPERBLOCK    --> HANDLE_TYPE_FDEVICE
 *    - HANDLE_TYPE_FDEVICE        --> HANDLE_TYPE_FSUPERBLOCK
 *     (Only for block-devices that have superblock
 *      associated; uses `block_device_getsuper()')
 *    - HANDLE_TYPE_FDEVICE_STREAM --> HANDLE_TYPE_FDEVICE
 *    - HANDLE_TYPE_FTHREAD        --> HANDLE_TYPE_FVM
 *    - HANDLE_TYPE_FVM            --> HANDLE_TYPE_FAPPLICATION (The root application)
 *    - HANDLE_TYPE_FAPPLICATION   --> HANDLE_TYPE_FMODULE
 * NOTE: Substitution is allowed to recurse, meaning that
 *      `HANDLE_TYPE_FTHREAD --> HANDLE_TYPE_FMODULE' can be done by
 *       going through `HANDLE_TYPE_FVM' and `HANDLE_TYPE_FAPPLICATION'
 * Additionally, the following substitutions are performed for `handle_get_superblock_relaxed':
 *    - HANDLE_TYPE_FINODE       --> HANDLE_TYPE_FSUPERBLOCK
 *    In other words meaning that `file', `path', `inode',
 *    `device' and `superblock' handles are accepted.
 * @throw: E_INVALID_HANDLE: The given file descriptor number does not match
 *                           the required type, or isn't associated with a handle. */
FUNDEF REF struct handle KCALL handle_get(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct device *KCALL handle_get_device(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct superblock *KCALL handle_get_superblock(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct superblock *KCALL handle_get_superblock_relaxed(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct inode *KCALL handle_get_inode(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct path *KCALL handle_get_path(fd_t fd);
/* @throw: E_PROCESS_EXITED: `fd' belongs to a task that is no longer allocated. */
FUNDEF ATTR_RETNONNULL REF struct task *KCALL handle_get_task(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct vm *KCALL handle_get_vm(fd_t fd);
/* @throw: E_NO_DATA: The VM associated with `fd' has no mapped applications. */
FUNDEF ATTR_RETNONNULL REF struct application *KCALL handle_get_application(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct module *KCALL handle_get_module(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct pipe *KCALL handle_get_pipe(fd_t fd);
#ifdef __INTELLISENSE__
FUNDEF ATTR_RETNONNULL REF struct file *KCALL handle_get_file(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct directory_entry *KCALL handle_get_directory_entry(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct fs *KCALL handle_get_fs(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct task_weakref *KCALL handle_get_thread(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct vm_region *KCALL handle_get_vm_region(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct pipewriter *KCALL handle_get_pipewriter(fd_t fd);
FUNDEF ATTR_RETNONNULL REF struct socket *KCALL handle_get_socket(fd_t fd);
#else
#define handle_get_file(fd)             ((REF struct file *)handle_get_typed(fd,HANDLE_TYPE_FFILE))
#define handle_get_directory_entry(fd)  ((REF struct directory_entry *)handle_get_typed(fd,HANDLE_TYPE_FDIRECTORY_ENTRY))
#define handle_get_fs(fd)               ((REF struct fs *)handle_get_typed(fd,HANDLE_TYPE_FFS))
#define handle_get_thread(fd)           ((REF struct task_weakref *)handle_get_typed(fd,HANDLE_TYPE_FTHREAD))
#define handle_get_vm_region(fd)        ((REF struct vm_region *)handle_get_typed(fd,HANDLE_TYPE_FVM_REGION))
#define handle_get_pipereader(fd)       ((REF struct pipereader *)handle_get_typed(fd,HANDLE_TYPE_FPIPEREADER))
#define handle_get_pipewriter(fd)       ((REF struct pipewriter *)handle_get_typed(fd,HANDLE_TYPE_FPIPEWRITER))
#define handle_get_socket(fd)           ((REF struct socket *)handle_get_typed(fd,HANDLE_TYPE_FSOCKET))
FUNDEF ATTR_RETNONNULL REF void *KCALL handle_get_typed(fd_t fd, u16 type);
#endif



#endif /* __CC__ */


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_HANDLE_H */
