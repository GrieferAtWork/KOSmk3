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
#ifndef GUARD_KERNEL_INCLUDE_FS_DEVICE_H
#define GUARD_KERNEL_INCLUDE_FS_DEVICE_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/list/list.h>
#include <hybrid/list/atree.h>
#include <sched/rwlock.h>
#include <fs/driver.h>
#include <fs/iomode.h>
#include <fs/handle.h>

DECL_BEGIN

#ifdef __CC__
struct device;
struct block_device;
struct stat64;
#endif /* __CC__ */


#define DEVICE_TYPE_FBLOCKDEV 0x0000 /* Block-device (`struct block_device'). */
#define DEVICE_TYPE_FCHARDEV  0x0001 /* Character-device (`struct character_device'). */
#define DEVICE_TYPE_COUNT     2

#define DEVICE_FNORMAL        0x0000 /* Normal device flags. */
#define DEVICE_FREADONLY      0x0001 /* [lock(weak,atomic)]
                                      * The device is operating in read-only mode.
                                      * Write operations are not forwarded to operators
                                      * and dismissed immediately by throwing an
                                      * `ERROR_FS_READONLY_FILESYSTEM' error. */
#define DEVICE_FDYNDEVICE     0x8000 /* [const] The device number of this device has been
                                      *         allocated dynamically, and must be freed
                                      *         when the device is destroyed.
                                      *  NOTE:  When the device is a master block-device, its
                                      *         ID, as well as the following `b_partmaxcnt'
                                      *         IDs are freed. */


#define DEVICE_MAXNAME        16 /* The max length of a statically allocated device name. */

#ifdef __CC__
struct driver;
struct device {
    ATOMIC_DATA ref_t          d_refcnt; /* Device reference counter. */
    LIST_NODE(struct device)   d_chain;  /* [lock(INTERNAL(device_maps[*].dm_lock))] Chain of devices with the same `d_devno' hash. */
    REF struct driver         *d_driver; /* [const][1..1] The driver implementing this device. */
    dev_t                      d_devno;  /* [const] Device number. */
    u16                        d_type;   /* [const] Device type (One of `DEVICE_TYPE_F*') */
    u16                        d_flags;  /* Device flags (Set of `DEVICE_F*') */
    char                      *d_name;   /* [1..1][const][owned_if(!= d_namebuf)]
                                          * Pointer to a C-string detailing the name of this device.
                                          * That string is the name under which the device appears within the `/dev' filesystem. */
    char                       d_namebuf[DEVICE_MAXNAME]; /* Static buffer for the device name. */
};
#define DEVICE_ISREGISTERED(x) ((x)->d_chain.le_pself != NULL)

/* Allocate and pre/null/zero-initialize a new device,
 * who's control structure has a size of `struct_size' bytes.
 * The caller must initialize the following members:
 *    - d_namebuf      (Unless `d_name' is set directly)
 *    - d_name         (Optionally; pre-initialized to `d_namebuf')
 *    - d_devno        (See explanation below)
 * Following this, a device ID should be allocated for, or assigned
 * to the device, either by directly setting `return->d_devno',
 * or by calling `devno_alloc()', or `register_dynamic_device()' (see above.)
 * @param: device_type:     The type of device (One of `DEVICE_TYPE_F*')
 * @throw: E_DRIVER_CLOSED: The given driver has been closed/is closing. */
FUNDEF ATTR_RETNONNULL REF struct device *
KCALL __device_alloc(u16 device_type, size_t struct_size,
                     struct driver *__restrict caller)
                     ASMNAME("device_alloc");
#define device_alloc(device_type,struct_size) \
      __device_alloc(device_type,struct_size,&this_driver)


/* Destroy a previously allocated device. */
FUNDEF ATTR_NOTHROW void KCALL device_destroy(struct device *__restrict self);

/* Increment/decrement the reference counter of the given device `x' */
#define device_incref(x)  ATOMIC_FETCHINC((x)->d_refcnt)
#define device_decref(x) (ATOMIC_DECFETCH((x)->d_refcnt) || (device_destroy(x),0))

/* Lookup the device associated with a given type and device-number.
 * @throw: E_NO_DEVICE: The named device doesn't exist. */
FUNDEF ATTR_RETNONNULL REF struct device *KCALL lookup_device(u16 type, dev_t devno);
/* Similar to `lookup_device', but if the device doesn't exist, return `NULL'.  */
FUNDEF ATTR_NOTHROW REF struct device *KCALL try_lookup_device(u16 type, dev_t devno);

#ifdef __INTELLISENSE__
FUNDEF ATTR_RETNONNULL REF struct block_device *KCALL lookup_block_device(dev_t devno);
FUNDEF ATTR_RETNONNULL REF struct character_device *KCALL lookup_character_device(dev_t devno);
FUNDEF ATTR_NOTHROW REF struct block_device *KCALL try_lookup_block_device(dev_t devno);
FUNDEF ATTR_NOTHROW REF struct character_device *KCALL try_lookup_character_device(dev_t devno);
#else
#define lookup_block_device(devno)         ((struct block_device *)lookup_device(DEVICE_TYPE_FBLOCKDEV,devno))
#define lookup_character_device(devno)     ((struct character_device *)lookup_device(DEVICE_TYPE_FCHARDEV,devno))
#define try_lookup_block_device(devno)     ((struct block_device *)try_lookup_device(DEVICE_TYPE_FBLOCKDEV,devno))
#define try_lookup_character_device(devno) ((struct character_device *)try_lookup_device(DEVICE_TYPE_FCHARDEV,devno))
#endif

/* Register / delete a given device.
 * @return: true:  Successfully registered the given.
 * @return: false: Another device had already been registered
 *                 under the associated device number.
 * NOTE: When deleting a block-device master drive, `unregister_device()'
 *       will also delete all associated partitions. */
FUNDEF bool KCALL register_device(struct device *__restrict dev);
FUNDEF bool KCALL register_device_nodevfs(struct device *__restrict dev);
FUNDEF void KCALL unregister_device(struct device *__restrict dev);
FUNDEF ATTR_NOTHROW void KCALL unregister_device_nodevfs(struct device *__restrict dev);

/* Add/remove the given device from `devfs'
 * NOTE: These functions are automatically called
 *       from `register_device()' and `unregister_device()' */
FUNDEF void KCALL device_add_to_devfs(struct device *__restrict dev);
FUNDEF void KCALL device_remove_from_devfs(struct device *__restrict dev);


/* Dynamically allocate a device ID of the given device and
 * register the device under that number. */
FUNDEF void KCALL register_dynamic_device(struct device *__restrict dev);
/* Same as `register_dynamic_device()', but uses `devno_alloc_mask()' to allocate the device number. */
FUNDEF void KCALL register_dynamic_device_mask(struct device *__restrict dev, dev_t start, minor_t mask);

/* The major starting number of all dynamically allocated device IDs. */
#define DEVNO_DYNAMIC_BEGIN_MAJOR   1024

/* Allocate/Free dynamic device IDs.
 * NOTE: The major numbers of all device IDs
 *       allocated are `>= DEVNO_DYNAMIC_BEGIN_MAJOR'.
 * @param: type:        The type of the device (One of `DEVICE_TYPE_F*')
 * @param: num_numbers: The number of consecutive device IDs to allocate/free.
 *                      This is usually used to reserve slots for block-device
 *                      partitions (s.a. `b_partmaxcnt').
 * @throw: E_BADALLOC.ERROR_BADALLOC_DEVICEID: Failed to allocate an ID for the given purpose. */
FUNDEF WUNUSED dev_t KCALL devno_alloc(u16 type, minor_t num_numbers);
FUNDEF ATTR_NOTHROW void KCALL devno_free(u16 type, dev_t start, minor_t num_numbers);
/* Try to allocate the specified device number. */
FUNDEF WUNUSED bool KCALL devno_allocat(u16 type, dev_t start, minor_t num_numbers);

/* Allocate the lowest available device number
 * that is matching `MKDEV(MAJOR(start),MINOR(start) + (XXX & mask))'
 * This function is used to allocate device number for dynamic devices, such as PTY terminals.
 * @assume((MINOR(start) & mask) == 0);
 * @throw: E_BADALLOC.ERROR_BADALLOC_DEVICEID: Failed to allocate an ID for the given purpose. */
FUNDEF WUNUSED dev_t KCALL devno_alloc_mask(u16 type, dev_t start, minor_t mask, minor_t num_numbers);


/* Generic device operators. */

/* Run an I/O-control command on the given device. */
FUNDEF ssize_t KCALL device_ioctl(struct device *__restrict self, unsigned long cmd, USER UNCHECKED void *arg, iomode_t flags);
/* @throw: E_NOT_IMPLEMENTED: Stat information cannot be gathered about the given device. */
FUNDEF void KCALL device_stat(struct device *__restrict self, USER CHECKED struct stat64 *result);
/* Synchronizes the page buffer of a block-device, or try to invoke the `c_sync'
 * operator of character devices, but ignore it if that operator is missing. */
FUNDEF void KCALL device_sync(struct device *__restrict self);
/* Tries to invoke the `c_open' operator of character
 * devices, or simply re-return a handle to the given device. */
FUNDEF REF struct handle KCALL device_open(struct device *__restrict self, struct path *__restrict p, oflag_t open_mode);
/* @throw: E_NOT_IMPLEMENTED: Current-position reading isn't supported (e.g.: block devices). */
FUNDEF size_t KCALL device_read(struct device *__restrict self, USER CHECKED void *buf, size_t bufsize, iomode_t flags);
/* @throw: E_NOT_IMPLEMENTED: Current-position writing isn't supported (e.g.: block devices). */
FUNDEF size_t KCALL device_write(struct device *__restrict self, USER CHECKED void const *buf, size_t bufsize, iomode_t flags);
/* @return: 0: No signals are available, or polling isn't supported. */
FUNDEF unsigned int KCALL device_poll(struct device *__restrict self, unsigned int mode);
/* @throw: E_NOT_IMPLEMENTED: Seeking is not supported. */
FUNDEF pos_t KCALL device_seek(struct device *__restrict self, off_t offset, int whence);
/* @throw: E_NOT_IMPLEMENTED: Absolute-position reading isn't supported. */
FUNDEF size_t KCALL device_pread(struct device *__restrict self, USER CHECKED void *buf, size_t bufsize, pos_t offset, iomode_t flags);
/* @throw: E_NOT_IMPLEMENTED: Absolute-position writing isn't supported. */
FUNDEF size_t KCALL device_pwrite(struct device *__restrict self, USER CHECKED void const *buf, size_t bufsize, pos_t offset, iomode_t flags);


/* Block use buffering:
 *   - Limit max number of block pages.
 *   - Track usage counter in each page individually.
 *   - When limit is reached:
 *      - Determine lowest & greatest usage counts
 *      - Take some factor between the 2 as average
 *      - Delete (after writing changes) all pages with a use counter <= that average
 *      - Subtract some factor of the average from all remaining pages
 *      - Rehash if the map was shrunk considerably. */
struct block_page {
    blkaddr_t                    bp_addr;   /* Address of this block-page (used as hash-vector index). */
#define BLOCK_PAGE_DUMMY  ((VIRT byte_t *)1)
#define BLOCK_PAGE_ISDUMMYORNULL(x) ((uintptr_t)(x) <= 1)
    VIRT byte_t                 *bp_data;   /* [0..1][owned(HEAP(GFP_SHARED|GFP_S,bp_size))] Block-page data.
                                             * NOTE: The page entry is unused when this pointer is NULL.
                                             * NOTE: Set to `BLOCK_PAGE_DUMMY' for dummy pages. */
    size_t                       bp_size;   /* [const] Allocated size of the block-page (from the heap-pointer) */
#define BLOCK_PAGE_FNORMAL       0x0000     /* Normal block-page flags. */
#define BLOCK_PAGE_FCHANGED      0x0001     /* The block-page has been modified. */
    u16                          bp_flags;  /* Block-page flags (Set of `BLOCK_PAGE_F*') */
#if __SIZEOF_POINTER__ == 8
#define BLOCK_PAGE_USAGE_MAX   ((u32)-1)    /* Max value for `bp_usage' */
    u16                        __bp_pad;    /* ... */
    u32                          bp_usage;  /* Page usage counter */
#else
#define BLOCK_PAGE_USAGE_MAX   ((u16)-1)    /* Max value for `bp_usage' */
    u16                          bp_usage;  /* Page usage counter */
#endif
};


/* The default MAX-MASK to which the block-pages buffer may grow.
 * This is the default max count of buffered pages per block-device -1 */
#ifndef CONFIG_BLOCK_PAGES_MAXMASK
#define CONFIG_BLOCK_PAGES_MAXMASK 0xff
#endif

struct block_pages {
    rwlock_t            ps_lock;      /* Lock for accessing the block-page buffer. */
    size_t              ps_mapu;      /* [lock(ps_lock)][<= ps_mapc] Amount of used pages. */
    size_t              ps_mapc;      /* [lock(ps_lock)][<= ps_mapm] Amount of non-NULL pages (meaning dummies are included) */
    size_t              ps_mapm;      /* [lock(ps_lock)] Current hash-mask of the page vector. */
    size_t              ps_mapm_max;  /* [lock(ps_lock)] The max value for `ps_mapm'. */
    struct block_page  *ps_mapv;      /* [lock(ps_lock)][0..ps_mapm+1][owned] Page buffer vector. */
};

struct block_device {
    struct device                       b_device;     /* Underlying device. */
    blksize_t                           b_blocksize;  /* [const][!0] Size of a single data block (in bytes) */
    blkcnt_t                            b_blockcount; /* [const][!0] Amount of blocks provided by this device (indexed 0..b_blockcount-1) */
    pos_t                               b_partstart;  /* [const] Partition start offset (ZERO(0) in the master device) */
    atomic_rwlock_t                     b_fslock;     /* Lock for `b_filesystem' */
    WEAK struct superblock             *b_filesystem; /* [0..1][NULL_IF(self == &null_device)][lock(b_fslock)]
                                                       * The filesystem that is mounted on this device. */
    REF struct block_device            *b_master;     /* [1..1][const][ref_if(!= self)]
                                                       * [->b_master == b_master]
                                                       * The master device of this partition. */
    union {
        struct {
            WEAK LIST_NODE(struct block_device)
                                        p_partitions; /* [0..1][lock(p_master->b_partlock)]
                                                       * Chain of partitions of this block-device. */
        }                               b_partition;  /* [b_master != self] partition data. */
        struct {                                      /* [b_master == self] */
            struct block_pages          b_pagebuf;    /* Block page buffer. */
            atomic_rwlock_t             b_partlock;   /* Lock for `b_partitions' */
            WEAK LIST_HEAD(struct block_device)
                                        b_partitions; /* [0..1][->b_device.d_flags&DEVICE_FPARTITION]
                                                       * [lock(b_partlock)] Chain of partitions of this block-device. */
            minor_t                     b_partmaxcnt; /* [const] The max number of partitions. */
            struct PACKED {
                /* [1..1][const]
                 *  Disk-level read-block operator (for synchronous reading)
                 * @assume((first_block + num_blocks) <= self->b_blockcount)
                 * @throw E_SEGFAULT: The given user-buffer is faulty. */
                void (KCALL *io_read)(struct block_device *__restrict self,
                                      CHECKED USER void *buf, size_t num_blocks,
                                      blkaddr_t first_block);
                /* [1..1][const]
                 *  Disk-level write-block operator (for synchronous writing)
                 * @assume((first_block + num_blocks) <= self->b_blockcount)
                 * @throw E_SEGFAULT: The given user-buffer is faulty. */
                void (KCALL *io_write)(struct block_device *__restrict self,
                                       CHECKED USER void const *buf, size_t num_blocks,
                                       blkaddr_t first_block);
                /* [0..1][const] Finalizer. - Called during `device_destroy' */
                /*ATTR_NOTHROW*/void (KCALL *io_fini)(struct block_device *__restrict self);

                /* [0..1][const] I/O control callback.
                 * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator.
                 *                            Should be thrown for unrecognized commands.
                 * This callback is encouraged to implement the following ioctls:
                 *   - BLKSECTGET: The max number of sectors that can be read/written using a single command.
                 */
                ssize_t (KCALL *io_ioctl)(struct block_device *__restrict self,
                                          unsigned long cmd, USER UNCHECKED void *arg,
                                          iomode_t flags);
            }                           b_io;         /* [const] Block device I/O operations. */
        };
    };
};

/* Allocate and pre/null/zero-initialize a new block device,
 * who's control structure has a size of `struct_size' bytes.
 * The block-device has been initialize as a drive master device,
 * and the caller must initialize the following members:
 *    - b_device.d_namebuf      (Unless `d_name' is set directly)
 *    - b_device.d_name         (Optionally; pre-initialized to `d_namebuf')
 *    - b_device.d_devno        (See explanation below)
 *    - b_io.io_read
 *    - b_io.io_write
 *    - b_io.io_fini            (Optionally; pre-initialized to NULL)
 *    - b_io.io_ioctl           (Optionally; pre-initialized to NULL)
 *    - b_blocksize
 *    - b_blockcount
 *    - b_partmaxcnt            (Optionally; pre-initialized to `0')
 * @throw: E_DRIVER_CLOSED: The given driver has been closed/is closing.
 * Following this, a device ID should be allocated for, or assigned
 * to the device, either by directly setting `return->d_device.d_devno',
 * or by calling `devno_alloc()', or `register_dynamic_device()' (see above.) */
FUNDEF ATTR_RETNONNULL REF struct block_device *
KCALL __block_device_alloc(size_t struct_size, minor_t part_maxcount,
                           struct driver *__restrict caller)
                           ASMNAME("block_device_alloc");
#define block_device_alloc(struct_size,part_maxcount) \
      __block_device_alloc(struct_size,part_maxcount,&this_driver)
#define BLOCK_DEVICE_ALLOC(T,part_maxcount) \
  ((T *)block_device_alloc(sizeof(T),part_maxcount))


/* Increment/decrement the reference counter of the given block_device `x' */
#define block_device_incref(x) device_incref(&(x)->b_device)
#define block_device_decref(x) device_decref(&(x)->b_device)

/* A special block-device that is capable of linking to multiple
 * superblocks, rather than just a single. In exchange, attempting
 * to read or write data to/from this device will always cause an
 * `E_NO_DATA' error to be thrown. */
DATDEF struct block_device null_device;


/* Read/Write data to/from the given block-device.
 * NOTE: These functions interface with the block-device's buffer in
 *       order to allow data to be read with single-byte precision,
 *       rather than always having to write data whole blocks at a time.
 * @throw: E_SEGFAULT: The given user-buffer is faulty.
 * @throw: E_NO_DATA:  The given block range is overflowing, or out-of-bounds.
 * @return: * :        The actual number of read/written bytes.
 *                     NOTE: Guarantied equal to `num_bytes' when `O_NONBLOCK' isn't passed. */
FUNDEF size_t KCALL
block_device_read(struct block_device *__restrict self,
                  CHECKED USER void *buf, size_t num_bytes,
                  pos_t device_position, iomode_t flags);
FUNDEF size_t KCALL
block_device_write(struct block_device *__restrict self,
                   CHECKED USER void const *buf, size_t num_bytes,
                   pos_t device_position, iomode_t flags);

/* Synchronize all unwritten data with the underlying disk. */
FUNDEF void KCALL
block_device_sync(struct block_device *__restrict self);


/* Construct a new sub-partition of the given block-device.
 * NOTE: When `self' is another partition itself, the new partition
 *       will still be constructed in the master device, however
 *       its position will still be located in a sub-section of
 *       the partition that it is derived from.
 * NOTE: When the given partition range is beyond the valid bounds of
 *       the partition, a warning is logged and the bounds are truncated.
 * @param: self:             The drive master / partition to sub-partition.
 * @param: partition_offset: The offset of the partition (in bytes).
 * @param: partition_size:   The size of the partition (in blocks).
 * @param: partition_number: The partition number of this device
 *                          (minor device ID offset from the drive master; < `b_partmaxcnt').
 * @return: * :              The new partition describing the given range.
 * @return: NULL:            The device `self' isn't registered, or has been deleted (`DEVICE_ISREGISTERED()' returns false)
 * @return: NULL:            The given `partition_number' is `>= b_partmaxcnt'
 * @return: NULL:            The effective device ID of the partition was already in use.
 * @return: NULL:            After truncation, the partition size (`partition_size') equates to ZERO(0). */
FUNDEF REF struct block_device *KCALL
block_device_partition(struct block_device *__restrict self,
                       pos_t partition_offset,
                       blkcnt_t partition_size,
                       minor_t partition_number);



/* Try to return a pointer to a superblock that is using
 * the given block-device as filesystem storage location.
 * Otherwise, return NULL. */
FUNDEF REF struct superblock *KCALL
block_device_getsuper(struct block_device *__restrict self);



struct character_device_ops {
    /* [0..1] Finalizer. - Called during `device_destroy' */
    /*ATTR_NOTHROW*/void (KCALL *c_fini)(struct character_device *__restrict self);

    /* [0..1] Open the character device as a regular file.
     *  NOTE: When this operator isn't implemented, the character device
     *        itself will be opened as a handle, meaning that handle operators
     *        will invoke the callbacks defined below. */
    REF struct handle (KCALL *c_open)(struct character_device *__restrict self,
                                      struct path *__restrict p, oflag_t open_mode);

    struct {
        /* [0..1] I/O control callback.
         * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator.
         *                            Should be thrown for unrecognized commands. */
        ssize_t (KCALL *f_ioctl)(struct character_device *__restrict self,
                                 unsigned long cmd, USER UNCHECKED void *arg,
                                 iomode_t flags);

        /* [0..1] Query status information.
         * NOTE: Before being invoked, the caller will have already
         *       initialized the `st_dev' and `st_rdev' fields, while
         *       all other fields will have been initialized to ZERO(0).
         * NOTE: The only place where this operator is invoked is if the
         *       user invoked `fstat()' on a file descriptor referring
         *       to a device.
         *       Invoking `stat()' on an INode referring to a character
         *       device will _NOT_ invoke this operator, but only present
         *       information available through the INode itself. */
        void (KCALL *f_stat)(struct character_device *__restrict self,
                             USER CHECKED struct stat64 *result);

        /* [0..1] Synchronize unwritten data.
         *  NOTE: When not implemented, ignore as a no-op. */
        void (KCALL *f_sync)(struct character_device *__restrict self);


        /* [0..1] Read data from the device.
         * @param: flags: Read mode flags (Usually used for `O_NONBLOCK')
         * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator. */
        size_t (KCALL *f_read)(struct character_device *__restrict self,
                               USER CHECKED void *buf, size_t bufsize,
                               iomode_t flags);

        /* [0..1] Write data to the device.
         * @param: flags: Write mode flags (Usually used for `O_NONBLOCK')
         * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator. */
        size_t (KCALL *f_write)(struct character_device *__restrict self,
                                USER CHECKED void const *buf, size_t bufsize,
                                iomode_t flags);

        /* [0..1] Connect to signals broadcast when `mode' (set of `POLLIN|POLLPRI|POLLOUT|POLLERR')
         * becomes true. Return the set of signals who's associated condition is currently met:
         * >> unsigned int result = 0;
         * >> FOREACH_POLL_MODE(x,mode) {
         * >>     task_connect(GET_SIGNAL_FOR(self,x));
         * >>     if (CONDITION_IS_MET(self,x))
         * >>         result |= x;
         * >> }
         * >> return result;
         * @return: 0: No signals are available (same as not implementing this operator) */
        unsigned int (KCALL *f_poll)(struct character_device *__restrict self,
                                     unsigned int mode);

        /* [0..1] Seek within the device.
         * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator.
         * @throw: E_FILESYSTEM_ERROR.ERROR_FS_NEGATIVE_SEEK: [...] */
        pos_t (KCALL *f_seek)(struct character_device *__restrict self,
                              off_t offset, int whence);

        /* [0..1] Read data from a specific offset within the device.
         * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator. */
        size_t (KCALL *f_pread)(struct character_device *__restrict self,
                                USER CHECKED void *buf, size_t bufsize,
                                pos_t offset, iomode_t flags);

        /* [0..1] Write data to a specific offset within the device.
         * @throw: E_NOT_IMPLEMENTED: Same as not implementing this operator. */
        size_t (KCALL *f_pwrite)(struct character_device *__restrict self,
                                 USER CHECKED void const *buf, size_t bufsize,
                                 pos_t offset, iomode_t flags);
        /* [0..1]
         * [locked(READ(self->i_lock))]
         * Return a memory region containing data detailing
         * the memory for the given `page_index'.
         * If `page_index' isn't located at the start of the region,
         * save the offset into the region where mappings should start
         * within `*pregion_start'
         * @param: page_index:    The offset from the start of `self' for which a region
         *                        should be returned, such that `page_index' is located within.
         * @param: pregion_start: Upon success, must be filled with the offset into `return'
         *                        where the given `page_index' is located at.
         *                        If the returned region starts where the node itself starts,
         *                        this pointer should be filled with `page_index'
         * @assume(POST(*pregion_start <= page_index));
         * @assume(POST(return->vr_size > *pregion_start)); */
        REF struct vm_region *(KCALL *f_mmap)(struct character_device *__restrict self,
                                              /*vm_raddr_t*/uintptr_t page_index,
                                              /*vm_raddr_t*/uintptr_t *__restrict pregion_start);
    }                  c_file; /* File operators. */
};

struct character_device {
    struct device                c_device; /* Underlying device. */
    struct character_device_ops *c_ops;    /* [1..1][const] Character device operators. */
};


/* Increment/decrement the reference counter of the given character_device `x' */
#define character_device_incref(x) device_incref(&(x)->c_device)
#define character_device_decref(x) device_decref(&(x)->c_device)


/* Allocate and pre/null/zero-initialize a new character device,
 * who's control structure has a size of `struct_size' bytes.
 * The caller must initialize the following members:
 *    - c_device.d_namebuf      (Unless `d_name' is set directly)
 *    - c_device.d_name         (Optionally; pre-initialized to `d_namebuf')
 *    - c_device.d_devno        (See explanation below)
 *    - c_ops
 * @throw: E_DRIVER_CLOSED: The given driver has been closed/is closing.
 * Following this, a device ID should be allocated for, or assigned
 * to the device, either by directly setting `return->d_device.d_devno',
 * or by calling `devno_alloc()', or `register_dynamic_device()' (see above.) */
FUNDEF ATTR_RETNONNULL REF struct character_device *
KCALL __character_device_alloc(size_t struct_size,
                               struct driver *__restrict caller)
                               ASMNAME("character_device_alloc");
#define character_device_alloc(struct_size) \
      __character_device_alloc(struct_size,&this_driver)
#define CHARACTER_DEVICE_ALLOC(T) \
  ((T *)character_device_alloc(sizeof(T)))





#endif /* __CC__ */




DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_DEVICE_H */
