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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_VM_H
#define GUARD_KERNEL_INCLUDE_KERNEL_VM_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/list/list.h>
#include <hybrid/list/atree.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/sync/atomic-rwptr.h>
#include <kernel/paging.h>
#include <sched/task.h>
#include <sched/mutex.h>
#include <sched/rwlock.h>
#include <sched/signal.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <sys-generic/mman.h>
#include <kernel/memory.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/vm.h>
#else
#error "Unsupported Architecture"
#endif

DECL_BEGIN

#ifdef __CC__
struct vm;
struct vm_region;
struct vm_region_part;
struct vm_node;
struct inode;

typedef u16       vm_prot_t;   /* Virtual memory protection (Set of `PROT_*' from <sys/mman.h>). */
typedef uintptr_t vm_raddr_t;  /* VM Region page index (Relative to the start of the region). */
typedef intptr_t  vm_sraddr_t; /* Signed VM Region page index. */
#endif /* __CC__ */


#ifdef __CC__
struct futex {
    atomic_rwptr_t    f_next;   /* [TYPE(struct futex)][sort(ASCENDING(f_addr))]
                                 *  Pointer to the next futex at a greater address. */
    atomic_rwptr_t   *f_pself;  /* [TYPE(struct futex)][0..1][lock(f_next.fp_lock)]
                                 *  Self-pointer (or NULL for unlinked futex objects;
                                 *                aka. when the region was deleted). */
    ATOMIC_DATA ref_t f_refcnt; /* Reference counter for this futex. */
    vm_raddr_t        f_addr;   /* In-region address of this futex. */
    struct sig        f_sig;    /* Signal used to implement scheduling with this futex. */
};

/* Destroy a previously allocated futex. */
FUNDEF void KCALL futex_destroy(struct futex *__restrict self);

/* Increment/decrement the reference counter of the given futex `x' */
#define futex_incref(x)  ATOMIC_FETCHINC((x)->f_refcnt)
#define futex_decref(x) (ATOMIC_DECFETCH((x)->f_refcnt) || (futex_destroy(x),0))

#endif


#ifdef __CC__
struct vm_phys_scatter {
    pageptr_t                            ps_addr;   /* [owned] Scatter base address. */
    size_t                               ps_size;   /* Number of continuous, owned pages. */
};
struct vm_phys {
    size_t                               py_num_scatter; /* Number of physical memory scatter entries. */
    struct vm_phys_scatter               py_iscatter[1]; /* Inline-list of scattered, physical memory. */
};

struct vm_swap { int placeholder; };
#endif /* __CC__ */

/* VM Part state. */
#define VM_PART_MISSING  0x00 /* Memory hasn't been allocated/initialized (NOTE: Mandatory for guard regions). */
#define VM_PART_INCORE   0x01 /* Memory is loaded into the core (NOTE: Mandatory for physical regions). */
#define VM_PART_INSWAP   0x02 /* Memory has been off-loaded into swap. */
#define VM_PART_UNKNOWN  0x03 /* The memory state isn't managed by the vm, or not relevant (usually used alongside `VM_REGION_RESERVED').
                               * WARNING: This state may be overwritten when a region type other than `VM_REGION_RESERVED' is used! */

/* VM Part flags. */
#define VM_PART_FNORMAL    0x00 /* [const] Normal part flags. */
#define VM_PART_FKEEP      0x01 /* [const] Do not free memory/swap tickets for this part until the associated region is destroyed. */
#define VM_PART_FWEAKREF   0x02 /* [const] Never free the data pointed to by this part, even when the associated region is destroyed. */
#define VM_PART_FNOSWAP    0x04 /* [const] Don't swap out this VM part. */
#define VM_PART_FCHANGED   0x80 /* [atomic] The part has changed (Only set when `VM_REGION_FMONITOR' is, too). */
#ifndef NDEBUG
#define VM_PART_FSPLITTING 0x40 /* [atomic] The part is currently being split.
                                 *  Used to detect logic errors that could cause inconsistencies
                                 *  when vm parts cause a recursion while being split themself. */
#endif


#ifdef __CC__
struct vm_part {
    /* Virtual memory region part:
     *   - Always belongs to a single region
     *   - Describes the current, global state of a memory
     *     mapping (even when shared between processes) */
    ref_t                                vp_refcnt; /* [lock(:vr_lock)]
                                                     * Amount of VM-nodes that map this part or the region.
                                                     * NOTE: This field may be ZERO(0), in which case this part isn't
                                                     *       mapped by anyone. Though this does not mean that it has no
                                                     *       memory or stick associated, but simply that any associated data
                                                     *       may be freed at any point in time (Unless `VM_PART_FKEEP' is set).
                                                     *       aka.: `mt_state == VM_PART_MISSING'. */
#ifdef __INTELLISENSE__
    struct { struct vm_part *le_next,**le_pself; }
                                         vp_chain;  /* [lock(:vr_lock)] Chain of region parts. */
#else
    LIST_NODE(struct vm_part)            vp_chain;  /* [lock(:vr_lock)] Chain of region parts. */
#endif
    vm_raddr_t                           vp_start;  /* [lock(:vr_lock)] Starting address of the part. */
    u8                                   vp_state;  /* [lock(:vr_lock)] Part state (Set of `VM_PART_*') */
    u8                                   vp_flags;  /* Part flags (Set of `VM_PART_F*') */
    ATOMIC_DATA s16                      vp_locked; /* [lock(:vr_lock)] Recursion counter for locking/unlocking memory for in-core usage (mlock() / munlock()).
                                                     * NOTE: Locking is enabled when this field is `> 0', otherwise it is disabled.
                                                     * NOTE: This field is ignored and considered equal to `1' for `VM_REGION_PHYSICAL' typed regions.
                                                     * NOTE: This field being non-zero does _NOT_ require mt_state to be `VM_PART_INCORE'!
                                                     *       It merely means that once loaded, the part will stay in memory until it is unlocked at a later time. */
    union PACKED {
        struct vm_phys                   vp_phys;   /* [valid_if(tp_state == VM_PART_INCORE)][lock(:vr_lock)] Physical memory data. */
        struct vm_swap                   vp_swap;   /* [valid_if(tp_state == VM_PART_INSWAP)][lock(:vr_lock)] Swap memory data. */
    };
};

/* Fault the given path `self' by allocating `num_pages' of physical memory for it.
 * The given part is assumed to have a size of `sizeof(struct vm_part)', or in other
 * words: be able to sustain a single `struct vm_phys_scatter' entry.
 * If faulting requires more physical memory segments than that, then a new
 * `vm_part' is allocated, then holding the required number of scatter tabs.
 * In any case, the faulted part is returned.
 * @assume(self->vp_state == VM_PART_MISSING);
 * @assume(POST(return->vp_state == VM_PART_INCORE));
 * @assume(POST(return == self || return != self));
 */
FUNDEF ATTR_RETNONNULL struct vm_part *KCALL
vm_part_fault(struct vm_part *__restrict self, size_t num_pages);

#endif /* __CC__ */



#ifndef CONFIG_NO_VIO
#ifdef __CC__
struct vio_ops {
    /* [0..1] Called during destruction of the accompanying region. */
    void (KCALL *v_fini)(void *closure);
    /* Arithmetic ops are invoked as follows:
     *    REGULAR: READ() + WRITE()
     *    ATOMIC:  READ() + ATOMIC_CMPXCH()
     * @param: addr: The offset into the region where the VIO access should happen.
     * @throw: E_NOT_IMPLEMENTED: Re-throw this error as an `E_SEGFAULT' (see below)
     * NOTE: Every memory access is guarantied to be aligned
     *       by 1/2/4/8 bytes (for b/w/l/q respectively)
     * NOTE: Missing operators are substituted using others whenever possible.
     *       When neither reading nor writing is possible (due to lack of these
     *       operators), an `E_NOT_IMPLEMENTED' error is thrown internally that
     *       is then interpreted as forwarding the original memory access as
     *       an `E_SEGFAULT' error.
     *       In other words, the implementor of these operators should be aware
     *       that throwing `E_NOT_IMPLEMENTED' from one of these callbacks will
     *       have that error be transformed into an `E_SEGFAULT' referring to
     *       the original instruction.
     * HINT: For full support, at least one read, write and cmpxchg operator
     *       should be implemented.
     * NOTE: `cmpxchg' must always return the old value, and only write
     *       `new_value' if that old value was equal to `old_value'.
     *       In other words, the caller assumes that the operation
     *       successfully exchanged the stored value when `old_value'
     *       is returned. */
    u8   (KCALL *v_readb)(void *closure, uintptr_t addr);
    u16  (KCALL *v_readw)(void *closure, uintptr_t addr);
    u32  (KCALL *v_readl)(void *closure, uintptr_t addr);
#if __SIZEOF_POINTER__ > 4
    u64  (KCALL *v_readq)(void *closure, uintptr_t addr);
#endif
    void (KCALL *v_writeb)(void *closure, uintptr_t addr, u8 value);
    void (KCALL *v_writew)(void *closure, uintptr_t addr, u16 value);
    void (KCALL *v_writel)(void *closure, uintptr_t addr, u32 value);
#if __SIZEOF_POINTER__ > 4
    void (KCALL *v_writeq)(void *closure, uintptr_t addr, u64 value);
#endif
    u8   (KCALL *v_atomic_cmpxchgb)(void *closure, uintptr_t addr, u8 old_value, u8 new_value);
    u16  (KCALL *v_atomic_cmpxchgw)(void *closure, uintptr_t addr, u16 old_value, u16 new_value);
    u32  (KCALL *v_atomic_cmpxchgl)(void *closure, uintptr_t addr, u32 old_value, u32 new_value);
#if __SIZEOF_POINTER__ > 4
    u64  (KCALL *v_atomic_cmpxchgq)(void *closure, uintptr_t addr, u64 old_value, u64 new_value);
#endif
};
#endif /* __CC__ */
#endif /* !CONFIG_NO_VIO */

#ifdef __CC__
/* Region control callback.
 * @param: self:    The region to apply the control command to.
 * @param: command: One of `REGION_CTL_F*'
 * @param: address: Region-relative address (single-byte resolution)
 *                  what is at that address depends `command'
 * @param: arg:     Argument specific to `command'
 * @return: 0:      Command failed, or isn't implemented.
 * @return: *:      Meaning depends on `command' */
typedef ssize_t (KCALL *pregionctl)(struct vm_region *__restrict self,
                                    unsigned int command,
                                    uintptr_t address, void *arg);
#define REGION_CTL_FFIND_FDE    0x0001 /* [TYPE(struct fde_info *arg)]
                                        *  Find FDE unwind information for `address' and store then in `arg'
                                        *  This region control operator is required to allow for unwinding of
                                        *  stack frames through the X86 USHARE SYSENTER segment.
                                        *  @return: 0:    No FDE found.
                                        *  @return: != 0: Managed to find an FDE. */
#define REGION_CTL_FADDR2LINE   0x0002 /* [TYPE(struct dl_addr2line *arg)]
                                        *  Find addr2line information for `address' and store then in `arg'
                                        *  NOTE: The `d_begin' and `d_end' fields should be filled as pointers
                                        *        relative to the start of the region.
                                        *  @return: 0:    No debug information found.
                                        *  @return: != 0: Managed to find debug information. */
#endif


/* VM Region type. */
#define VM_REGION_MEM                    0x0000     /* Default region type used for mapping memory (NOTE: _MUST_ be ZERO(0)). */
#define VM_REGION_PHYSICAL               0x0001     /* A special kind of memory region that maps an explicit physical memory region.
                                                     * NOTE: This kind of region cannot be locked, as it is always loaded, with
                                                     *       the addition that all parts are always locked + `VM_PART_STATE_INCORE',
                                                     *       and associated memory scatter chains are not freed.
                                                     * NOTE: This is also the mapping used to describe the kernel core.
                                                     * >> This kind of region is meant for safely mapping device memory in userspace. */
#define VM_REGION_RESERVED               0x0002     /* Used internally: A reserved memory region that may or may not contain data, yet may not be un-mapped,
                                                     * or re-mapped in the associated page directory (it may still be deleted/moved in the vm).
                                                     * Internally, this region type is used to reserve memory for virtual address space used
                                                     * for page directory self-mappings. */
#ifndef CONFIG_NO_VIO
#define VM_REGION_VIO                    0x0003     /* The region describes a virtually controlled memory descriptor.
                                                     * A VIO memory region may specify callbacks that are invoked
                                                     * when memory of the page is accessed through various means.
                                                     * This is done by handling E_SEGFAULT and interpreting the bytecode
                                                     * at its return address to see what kind of memory access is being
                                                     * performed, then forwarding the resulting access to one of the
                                                     * operator callbacks defined in the `vio_ops' structure.
                                                     * XXX: This is a pretty awesome idea, but kind of weird.
                                                     *      I could see some pretty funny uses for this, like:
                                                     * >> #define SYSTEM_TIME_IN_SECONDS (*(volatile u32 *)0xc500a004)
                                                     * >> void sleep(unsigned int num_seconds) {
                                                     * >>     printf("Being sleeping at %u\n",SYSTEM_TIME_IN_SECONDS);
                                                     * >>     SYSTEM_TIME_IN_SECONDS += num_seconds; // Handles set operations as wait-until
                                                     * >>     printf("End sleeping at %u\n",SYSTEM_TIME_IN_SECONDS);
                                                     * >> }
                                                     * It would be doable (although the assembly decoder might be pretty
                                                     * complicated), but I have to wonder: what would be the point.
                                                     * -> It's a cool idea in theory, but without an awesome (and practical)
                                                     *    usage case where this could be something really sweet, it probably
                                                     *    won't get implemented...
                                                     * LATER:
                                                     *    Ok, so I started implementing this because I realized that this
                                                     *    could be used to get an infinite number of MEMORY BREAK POINTS!
                                                     *    Think about it: while x86 has hardware support for setting memory
                                                     *    breakpoints, not only does QEMU not support them, but no major
                                                     *    operating system really makes use of them.
                                                     *    Though when you think about it: it's actually a pretty useful thing
                                                     *    when you're trying to track down who's corrupting your data...
                                                     *    Sooner or later I'll add an API for setting memory break points
                                                     *    using this utility. However in addition, crazy stuff like shown
                                                     *    in the example above are actually possible now! */
#endif /* !CONFIG_NO_VIO */
/*      VM_REGION_...                    0x0004      */
/*      VM_REGION_...                    0x000d      */
#define VM_REGION_LOGUARD                0x000e     /* A guard region that will be replaced with a `VM_REGION_MEM'
                                                     * copy containing the same memory mappings, before re-mapping itself
                                                     * `vr_size' below its previous position, so long as that address doesn't
                                                     * underflow, isn't already in use and sufficient funds remain available.
                                                     * NOTE: If the region is set-up to use `VM_REGION_INIT_FFILE' initialization,
                                                     *       the `f_start' will be decremented by `vr_size' in the new mapping.
                                                     *       In the event that decrementing `f_start' underflows, the region's
                                                     *       type is changed to `VM_REGION_INIT_FBYTE'.
                                                     * NOTE: `VM_REGION_INIT_FUSER' are updated similarly, using `u_delta' */
#define VM_REGION_HIGUARD                0x000f     /* Same as `VM_REGION_LOGUARD', but remap `vr_size' above and increment `f_start' (or `u_delta'). */
#define VM_REGION_ISGUARD(x)          (((x)&0xe) == 0xe)

/* VM Region initialization mode. */
#define VM_REGION_INIT_FNORMAL           0x0000     /* Do not perform any special initialization. - Just map physical memory as it appears. */
#define VM_REGION_INIT_FFILLER           0x0001     /* Use `memsetl(...,s_filler)' for initialization. */
#define VM_REGION_INIT_FRANDOM           0x0002     /* Use pseudo-random data to initialize memory. */
#define VM_REGION_INIT_FFILE             0x0003     /* Read/write file mapping.
                                                     * NOTE: When this type of region is swapped, the kernel
                                                     *       is allowed to write changes to disk and load them
                                                     *       at a later time, rather than write them to the swap
                                                     *       partition.
                                                     * NOTE: In order for write-back of file mappings to function
                                                     *       properly, the `VM_REGION_FMONITOR' flag must be set. */
#define VM_REGION_INIT_FFILE_RO          0x0004     /* Read-only file mapping (writing are kept in memory, but not mirrored to disk). */
#define VM_REGION_INIT_FUSER             0x0005     /* Custom, user-defined initialization/save callbacks. */
#ifndef CONFIG_NO_VIO
#define VM_REGION_INIT_FVIO              0x0fff     /* Mandatory initialization type for `VM_REGION_VIO' regions. */
#endif /* !CONFIG_NO_VIO */

/* VM Region flags. */
#define VM_REGION_FNORMAL                0x0000     /* [const] Normal region flags. */
#define VM_REGION_FMONITOR               0x0001     /* [const] Monitor attempts to write to data in the region and
                                                     *         set the `VM_PART_FCHANGED' flag of changed parts. */
#define VM_REGION_FCANTSHARE             0x0800     /* [const] `PROT_SHARED' cannot be used to prevent copy-on-write. */
#define VM_REGION_FIMMUTABLE             0x1000     /* [const] The region cannot be unmapped (Only set for kernel core regions). */
#define VM_REGION_FDONTMERGE             0x2000     /* [const] Never merge this region with neighboring regions.
                                                     *  NOTE:  This flag is mandatory for VIO regions. */
#define VM_REGION_FLEAKINGPARTS          0x4000     /* [lock(WRITE_ONCE)] The region was allocated using the core-base allocator. */
#define VM_REGION_FCOREREGION            0x8000     /* [const] The region was allocated using the core-base allocator. */
#define VM_REGION_FCOPYMASK              0x00ff     /* Mask of flags inherited by a copied region after COW. */

/* Special VM region funds value. */
#define VM_REGION_FUNDS_INFINITE         0xffff     /* The guard can reproduce itself an infinite number of times. */


/* VM Region user-initialization commands. */
#define VM_REGION_USERCOMMAND_LOAD       0 /* Fill in `buf...+=bufsize' with custom data. */
#define VM_REGION_USERCOMMAND_SAVE       1 /* Only called when `VM_REGION_FMONITOR' is set: save changed data. */
#define VM_REGION_USERCOMMAND_INCREF     2 /* Increment the reference counter of `closure' and re-return `closure',
                                            * or create a copy of `closure' and return a pointer to it. */
#define VM_REGION_USERCOMMAND_DECREF     3 /* Decrement the reference counter of `closure' or free its associated ata. */


#ifdef __CC__
struct vm_region {
    /* Virtual memory region:
     *   - Sharable between different VMs
     *   - Tracked globally to implement swap capabilities.
     *   - COW and LOA functionality
     *   - Memory-mapped file functionality
     */
    ATOMIC_DATA ref_t                    vr_refcnt; /* Region reference counter. */
    mutex_t                              vr_lock;   /* Lock for accessing this region. */
    u16                                  vr_type;   /* [const] Region type (One of `VM_REGION_*') */
    u16                                  vr_init;   /* [const] Region initialization (One of `VM_REGION_INIT_F*') */
    u16                                  vr_flags;  /* Set of `VM_REGION_F*' */
    u16                                  vr_funds;  /* [valid_if(VM_REGION_ISGUARD(vr_type))] Region guard funds. */
    size_t                               vr_size;   /* [const] Size of this region (in pages) */
    union PACKED {
#ifndef CONFIG_NO_VIO
        struct {
            struct vio_ops              *v_ops;     /* [1..1][const] VIO operators. */
            void                        *v_closure; /* [?..?][const] Closure argument passed to VIO callbacks. */
        }                                s_vio;     /* [VM_REGION_INIT_FVIO] VIO function callbacks. */
#endif
        u32                              s_filler;  /* [VM_REGION_INIT_FFILLER][const] Pattern that should be repeated in initialized memory. */
        struct PACKED {
            REF struct inode            *f_node;    /* [1..1][const] The referenced INode. */
            /* NOTE: File mappings are initialized as follows:
             *                 
             *  FILE: abcdefghijklmnopqrtuvwxyzABCDEFGHIJKLMNOPQRTUVWXYZ...
             *        |         |              |
             *        0   f_start              f_start+f_size
             *                  |              |
             *                  +-+            +-+
             *                    |              |
             *  MEM:  XXXXXXXXXXXXklmnopqrtuvwxyzXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
             *        |           |              |                                   |
             *        0     f_begin              f_begin+f_size               vr_size*PAGESIZE
             * X: Initialized using `f_filler' */
            uintptr_t                    f_begin;   /* [<= :vr_size*PAGESIZE][const] Byte-pffset into the region, where data from
                                                     *  the file starts (memory before then is initialized using `mri_byte'). */
            pos_t                        f_start;   /* [const] Offset within `f_node' to the start of initializer data. */
            size_t                       f_size;    /* [<= :vr_size*PAGESIZE][const] Max amount of bytes to read/write to/from `f_node'. */
            u32                          f_filler;  /* [const] Filler byte for initializing bytes outside the file-mapped range. */
        }                                s_file;    /* [VM_REGION_INIT_FFILE, VM_REGION_INIT_FFILE_RO] File initialization. */
        struct PACKED {
            /* [1..1][const] Region memory controller callback.
             * @param: command: The command that should be implemented (One of `VM_REGION_USERCOMMAND_*')
             *                  Unknown commands should be ignored silently without inspection
             *                  any argument other than `closure' and `command' (obviously).
             * @param: offset:  The offset into the region of the affected memory.
             * @param: buf:     A pointer to the virtual mapping location.
             *                  For the duration of the call, this memory is lock in-core.
             * @param: bufsize: The size of the buffer (in bytes)
             * @return: * :     Dependent on `command' (if not specified, simply ignored) */
            void                *(KCALL *u_func)(void *closure, unsigned int command, vm_sraddr_t offset,
                                                 VIRT void *buf, PAGE_ALIGNED size_t bufsize);
            void                        *u_closure; /* [const] Argument passed to `u_alloc' */
            vm_sraddr_t                  u_delta;   /* [const] Delta added to `offset' before `u_func' is invoked.
                                                     *  This is used to implement guard-pages with user-initializers. */
        }                                s_user;    /* [VM_REGION_INIT_FUSER] User-defined region initializer. */
    }                                    vr_setup;  /* Setup information. */
    LIST_HEAD(struct vm_part)            vr_parts;  /* [1..1][lock(vr_lock)] Chain (ordered by address) of region parts. */
    struct vm_part                       vr_part0;  /* [lock(vr_lock)] A statically allocated initial part.
                                                     *  NOTE: This part is always allocated to support a single scatter entry! */
    atomic_rwptr_t                       vr_futex;  /* [TYPE(struct futex)] Known futex objects within this region. */
    pregionctl                           vr_ctl;    /* [0..1][const] Optional region control callback. */
};

#define VM_REGION_FOREACH_PART(part,self) \
    LIST_FOREACH(part,(self)->vr_parts,vp_chain)
#define __VM_REGION_INIT_PARTS_UNKNOWN(self) \
    &(self).vr_part0, { 1, { NULL, &(self).vr_parts }, 0, VM_PART_UNKNOWN, VM_PART_FNORMAL, 0 }

#ifndef CONFIG_NO_VIO
#define VM_REGION_INIT_VIO(self,num_pages,ops,closure) \
   { 1, MUTEX_INIT, VM_REGION_VIO, VM_REGION_INIT_FVIO, \
     VM_REGION_FDONTMERGE, 0, num_pages, \
   { { ops, closure } }, __VM_REGION_INIT_PARTS_UNKNOWN(self) }
#endif


#endif /* __CC__ */



#ifdef __CC__
/* Allocate a new, default-initialized VM-region descriptor.
 * The reference counter is set to ONE(1).
 * The returned region is not being tracked.
 * The region is initialized as anonymous,
 * uninitialized, swappable and non-prefaulted memory.
 * @throw E_BADALLOC: Failed to allocate sufficient memory. */
FUNDEF ATTR_RETNONNULL ATTR_MALLOC
REF struct vm_region *KCALL vm_region_alloc(size_t num_pages);

/* Allocate a new memory region describing a file mapping.
 * @param: num_pages:        The number of pages.
 * @param: enable_writeback: Should modifications made to the data be reflected
 *                           within the file (true), or should they become lost
 *                           once the region is deleted (false)
 * @param: node:             The node that should be mapped.
 * @param: begin:            Offset into the region (in bytes) where file data should start
 *                          (Memory before then is initialized using `memsetl(filler)')
 * @param: start:            Offset to the first byte in `node' that should appear at `return + begin'.
 * @param: size:             The max number of bytes to map from `node', starting at `start' / `begin'
 *                           Memory thereafter is initialized using `filler'
 *                           NOTE: If this range extends past the end of the region, the resulting
 *                                 effect is the same as if it only extended up to its end.
 * @return: * :              The newly allocated region.
 * @throw E_BADALLOC:        Failed to allocate sufficient memory. */
FUNDEF ATTR_RETNONNULL ATTR_MALLOC
REF struct vm_region *KCALL
vm_region_alloc_file(size_t num_pages,
                     bool enable_writeback,
                     struct inode *__restrict node,
                     uintptr_t begin,
                     pos_t start,
                     size_t size,
                     u32 filler);


#ifndef CONFIG_NO_VIO
/* Construct a new VM VIO region using the given arguments. */
FUNDEF ATTR_RETNONNULL ATTR_MALLOC REF struct vm_region *
KCALL vm_region_alloc_vio(size_t num_pages,
                          struct vio_ops *__restrict ops,
                          void *closure);
#endif

/* Destroy a previously allocated vm-region. */
FUNDEF void KCALL vm_region_destroy(struct vm_region *__restrict self);

/* Increment/decrement the reference counter of the given VM `x' */
#define vm_region_incref(x)  ATOMIC_FETCHINC((x)->vr_refcnt)
#define vm_region_decref(x) (ATOMIC_DECFETCH((x)->vr_refcnt) || (vm_region_destroy(x),0))

/* Increment/decrement the usage-counters within a given address range.
 * @throw: E_BADALLOC: [vm_region_incref_range] Failed to increment generate a required split.
 * NOTE: Technically, `vm_region_decref_range()' could also fail due to lack of memory.
 *       However, it wouldn't be a good idea to have the caller deal with such an error,
 *       considering that this function is likely called during some cleanup operation,
 *       where cleanup operations should not fail.
 *       Instead, the `VM_REGION_FLEAKINGPARTS' flag of the region is set, meaning
 *       that assumptions about part reference counters (`vp_refcnt') being ZERO must
 *       no longer be made.
 *       You can still assert them being non-ZERO, or being greater than <X> without
 *       checking the `VM_REGION_FLEAKINGPARTS' flag, though. */
FUNDEF void KCALL
vm_region_incref_range(struct vm_region *__restrict self,
                       vm_raddr_t first_page, size_t num_pages);
FUNDEF void KCALL
vm_region_decref_range(struct vm_region *__restrict self,
                       vm_raddr_t first_page, size_t num_pages);

/* Increment/decrement core-lock counters within the given region page range.
 * @throw: E_OVERFLOW: The lock counter of one of the affected parts would have overflown. */
FUNDEF void KCALL
vm_region_inclck_range(struct vm_region *__restrict self,
                       vm_raddr_t first_page, size_t num_pages);
FUNDEF void KCALL
vm_region_declck_range(struct vm_region *__restrict self,
                       vm_raddr_t first_page, size_t num_pages);

/* Prefault the given region.
 * WARNING: This function will not initialize the allocated physical memory.
 *          The caller is responsible to perform initialization once the
 *          region has been mapped somewhere. */
FUNDEF void KCALL vm_region_prefault(struct vm_region *__restrict self);

#endif /* __CC__ */





#ifdef __CC__
/* VM mapping notification callback.
 * @param: command:   The command that should be implemented (One of `VM_NOTIFY_*')
 *                    Unknown commands should be ignored silently without inspection
 *                    any argument other than `closure' and `command' (obviously),
 *                    before returning `NULL'.
 * @param: addr:      The virtual page number describing the starting address of the affected node.
 * @param: num_pages: The number of pages, starting at `addr'.
 * @param: arg:       Command-specific, extended data.
 * @return: * :       Dependent on `command' (if not specified, simply ignored)
 * HINT: The general-purpose implementation re-returns `closure' for all
 *       known commands, and returns `NULL' for any unknown `command'. */
typedef void *(KCALL *vm_notify_t)(void *closure, unsigned int command,
                                   vm_vpage_t addr, size_t num_pages,
                                   void *arg);
#endif /* __CC__ */
#define VM_NOTIFY_INCREF     0x0000 /* Notification: incref()
                                     * Increment the reference counter of `closure' and re-return `closure',
                                     * or create a copy of `closure' and return a pointer to it.
                                     * NOTE: `addr' and `num_pages' point to the mapping location of the associated node in `THIS_VM'. */
#define VM_NOTIFY_DECREF     0x0001 /* [NOEXCEPT] Notification: decref()
                                     * NOTE: This callback's VM context is undefined, and so are the `addr' and `num_pages' arguments. */
#define VM_NOTIFY_UNMAP      0x0002 /* [NOEXCEPT] Notification: unmap() (NOTE: Not called from `vm_destroy()', meaning this one shouldn't be used for finalization)
                                     * NOTE: Called in the context of the VM within which memory got unmapped. */
#define VM_NOTIFY_EXTRACT    0x0003 /* Notification: Extract the mapping from its current location.
                                     * NOTE: Always followed by one of `VM_NOTIFY_RESTORE', `VM_NOTIFY_RESTORE_VM' or `VM_NOTIFY_DELETE'
                                     * NOTE: Called in the context of the VM from which the node should be extracted.
                                     * NOTE: `addr' and `num_pages' point to the mapping location of the associated node in `THIS_VM'. */
#define VM_NOTIFY_RESTORE    0x0004 /* Notification: Restore the mapping (potentially at a different address than where it was extracted from)
                                     * NOTE: Called in the context of the VM from the node got extracted from.
                                     * NOTE: `addr' and `num_pages' point to the mapping location of the associated node in `THIS_VM'. */
#define VM_NOTIFY_RESTORE_VM 0x0005 /* Notification: Same as `VM_NOTIFY_RESTORE', but used if the new VM differs from the old one (NOTE: Called in the context of the new VM)
                                     * NOTE: Called in the context of the VM in which the node is being restored.
                                     * NOTE: `addr' and `num_pages' point to the mapping location of the associated node in `THIS_VM'. */
#define VM_NOTIFY_DELETE     0x0006 /* [NOEXCEPT] Notification: Delete the mapping after it has been extracted.
                                     * NOTE: This callback's VM context is undefined, and so are the `addr' and `num_pages' arguments. */
#define VM_NOTIFY_CLONE      0x0007 /* Called during `vm_clone()'
                                     * @param: arg:                     [TYPE(struct vm *)] The new VM.
                                     * @return: * :                      The value to-be used as tag/closure in the new VM.
                                     * @return: VM_NOTIFY_CLONE_FINCREF: Invoke `VM_NOTIFY_INCREF' and use its return value as new tag/closure.
                                     * @return: VM_NOTIFY_CLONE_FLOOSE:  Get rid of the node in the new VM (s.a.: `PROT_LOOSE').
                                     * NOTE: Called in the context of the old VM.
                                     * NOTE: `addr' and `num_pages' point to the mapping location of the associated node in `THIS_VM'. */
#define VM_NOTIFY_CLONE_FINCREF  ((void *)0)
#define VM_NOTIFY_CLONE_FLOOSE   ((void *)-1)
#define VM_NOTIFY_SPLIT      0x0008 /* Notification: The mapping is about to be split into 2 parts:
                                     * @param: arg: [TYPE(size_t)] The amount of pages used by the high part.
                                     *               The node previously occupied `addr...+=num_pages+(size_t)arg'.
                                     *               Now, it is being split into 2 parts: `addr...+=num_pages'
                                     *               and `addr+num_pages...+=(size_t)arg'.
                                     * @return: * :  The new closure argument to-be used for the high part.
                                     * @return: VM_NOTIFY_SPLIT_FINCREF:  Invoke `VM_NOTIFY_INCREF' and use its return value for the high part.
                                     * @return: VM_NOTIFY_SPLIT_FPREVENT: Don't allow the split. - Memory operations only partially overlapping the node will affect it in its entirety. */
#define VM_NOTIFY_SPLIT_FINCREF  ((void *)0)
#define VM_NOTIFY_SPLIT_FPREVENT ((void *)-1)



/* VM Node flags. */
#define VM_NODE_FNORMAL                   0x0000      /* Normal node flags. */
#define VM_NODE_FIMMUTABLE                0x4000      /* [const] The node cannot be unmapped. */
#define VM_NODE_FCORENODE                 0x8000      /* [const] The node was allocated using the core-base allocator. */

#ifdef __CC__
struct vm_node {
    ATREE_NODE(struct vm_node,vm_vpage_t) vn_node;    /* [lock(:vm_lock)] Address tree node. */
    LIST_NODE(struct vm_node)             vn_byaddr;  /* [lock(:vm_lock)][->vn_node.a_vmin > vn_node.a_vmax]
                                                       *  Address-ordered list of existing per-VM node branches. */
    vm_raddr_t                            vn_start;   /* Offset into `n_region' of where the mapping starts. */
    REF struct vm_region                 *vn_region;  /* [1..1][lock(:vm_lock)] Node region.
                                                       * NOTE: In addition to holding a reference to the region,
                                                       *       this pointer also holds a reference to every `vm_part'
                                                       *       that is contained within `vn_start...+=VM_NODE_SIZE(self)' */
    vm_notify_t                           vn_notify;  /* [0..1][lock(:vm_lock)] An optional callback that
                                                       *  is invoked under varying circumstances. */
    void                                 *vn_closure; /* [?..?][valid_if(vn_notify)] Closure argument passed to `n_notify'. */
    vm_prot_t                             vn_prot;    /* Node protection. */
    u16                                   vn_flag;    /* Node flags (Set of `VM_NODE_F*'). */

};

FUNDEF void KCALL vm_node_free(struct vm_node *__restrict self);

#define VM_NODE_MINADDR(self)      VM_PAGE2ADDR(VM_NODE_MIN(self))
#define VM_NODE_MAXADDR(self)     (VM_PAGE2ADDR(VM_NODE_MAX(self)+1)-1)

#define VM_NODE_MIN(self)          ((self)->vn_node.a_vmin)
#define VM_NODE_MAX(self)          ((self)->vn_node.a_vmax)
#define VM_NODE_BEGIN(self)        ((self)->vn_node.a_vmin)
#define VM_NODE_END(self)          ((self)->vn_node.a_vmax+1)
#define VM_NODE_SIZE(self)        (((self)->vn_node.a_vmax-(self)->vn_node.a_vmin)+1)
#define VM_NODE_HASNEXT(self,vm)   ((self)->vn_byaddr.le_next != NULL)
#define VM_NODE_HASPREV(self,vm)   ((self)->vn_byaddr.le_pself != &(vm)->vm_byaddr)
#define VM_NODE_NEXT(self)         ((self)->vn_byaddr.le_next)
#define VM_NODE_PREV(self)           __COMPILER_CONTAINER_OF((self)->vn_byaddr.le_pself,struct vm_node,vn_byaddr.le_next)
#define VM_NODE_RADDR(self,vptr)   ((self)->mb_start+((uintptr_t)(vptr)-(self)->vn_node.a_vmin))

struct vm_corepair {
    VIRT struct vm_node   *cp_node;   /* [1..1][owned] The core-node pointer. */
    VIRT struct vm_region *cp_region; /* [1..1][owned] The core-region pointer. */
};

/* Allocate a vm_region / vm_node pair for the purposes of
 * mapping a virtual memory segment within the kernel core.
 * This function is used to implement core-base allocation
 * required for even getting started and solving the
 * dependency loop caused by:
 *    
 *    kmalloc() --> vm_map() -----\
 *      ^   ^           |         |
 *      |   |           v         v
 *      |   \------ vm_region  vm_node
 *      |               ^         |  ^
 *      \---------------|---------+  |
 *                      |            |
 *                      \------------+----\
 *                                        |
 * However, this loop is solved by `vm_corealloc()'
 * NOTE: The slot pair returned by this function is ZERO-initialized,
 *       with the exception that the `VM_NODE_FCORENODE' and 
 *      `VM_REGION_FCOREREGION' flags have been set in both components.
 * NOTE: This function must be used to allocate memory regions/nodes
 *       for the core-locked version of `kmalloc()'.
 * @throw: E_BADALLOC: The current core-base no longer contained both an
 *                     unused node and an unused region, but attempting
 *                     to allocate+map another corebase page failed.
 */
FUNDEF struct vm_corepair KCALL vm_corealloc(void);
#endif /* __CC__ */



/* Propagate alignment requirements from the page directory. */
#define VM_ALIGN             PAGEDIR_ALIGN
#define VM_OFFSETOF_PAGEDIR  0
#define VM_OFFSETOF_PHYSDIR  PAGEDIR_SIZE

#undef CONFIG_VM_USE_RWLOCK
//#define CONFIG_VM_USE_RWLOCK  1

#ifdef __CC__
struct vm {
    /* The basic Virtual Memory controller structure. */
#ifndef __VM_INTERNAL_EXCLUDE_PAGEDIR
    pagedir_t                            vm_pagedir;  /* [lock(vm_lock)] The page directory associated with the VM. */
#endif
    vm_phys_t                            vm_physdir;  /* [1..1][const] The physical address of the page directory. */
    ATOMIC_DATA ref_t                    vm_refcnt;   /* Reference counter. */
#ifdef CONFIG_VM_USE_RWLOCK
    rwlock_t                             vm_lock;     /* Lock for accessing this VM. */
#else
    mutex_t                              vm_lock;     /* Lock for accessing this VM. */
#endif
    ATREE_HEAD(struct vm_node)           vm_map;      /* [lock(vm_lock)][0..1] Memory mappings (excluding kernel mappings). */
    LIST_HEAD(struct vm_node)            vm_byaddr;   /* [lock(vm_lock)][0..1] Address-ordered list of nodes. */
    LIST_HEAD(struct task)               vm_tasks;    /* [lock(vm_tasklock)][0..1] Chain of tasks using this VM. */
    atomic_rwlock_t                      vm_tasklock; /* Lock for accessing the `vm_tasks' chain of tasks. */
    size_t                               vm_size;     /* [const] Allocated size of this VM (as returned by `heap_alloc_untraced()') */
    /* PER-VM variables go here. */
};

#ifdef CONFIG_VM_USE_RWLOCK
#define vm_tryacquire(x)      rwlock_trywrite(&(x)->vm_lock)
#define vm_acquire(x)         rwlock_write(&(x)->vm_lock)
#define vm_release(x)         rwlock_endwrite(&(x)->vm_lock)
#define vm_holding(x)         rwlock_writing(&(x)->vm_lock)
#define vm_tryacquire_read(x) rwlock_tryread(&(x)->vm_lock)
#define vm_acquire_read(x)    rwlock_read(&(x)->vm_lock)
#define vm_release_read(x)    rwlock_endread(&(x)->vm_lock)
#define vm_holding_read(x)    rwlock_reading(&(x)->vm_lock)
#else
#define vm_tryacquire(x)      mutex_try(&(x)->vm_lock)
#define vm_acquire(x)         mutex_get(&(x)->vm_lock)
#define vm_release(x)         mutex_put(&(x)->vm_lock)
#define vm_holding(x)         mutex_holding(&(x)->vm_lock)
#define vm_tryacquire_read(x) mutex_try(&(x)->vm_lock)
#define vm_acquire_read(x)    mutex_get(&(x)->vm_lock)
#define vm_release_read(x)   (mutex_put(&(x)->vm_lock),0)
#define vm_holding_read(x)    mutex_holding(&(x)->vm_lock)
#endif


#define VM_FOREACH_NODE(node,self) \
    LIST_FOREACH(node,(self)->vm_byaddr,vn_byaddr)

/* The kernel's own virtual memory.
 * NOTE: This is the only VM that is allowed to map memory within
 *       the kernel-share segment (on x86, that is: above X86_KERNEL_BASE_PAGE) */
DATDEF struct vm vm_kernel;

/* Helper macros for accessing per-VM variables. */
#define THIS_VM           PERTASK_GET(this_task.t_vm)
#define PERVM(x)        (*(__typeof__(&(x)))((uintptr_t)THIS_VM+(uintptr_t)&(x)))
#define FORVM(self,x)   (*(__typeof__(&(x)))((uintptr_t)(self)+(uintptr_t)&(x)))

/* Allocate a new VM descriptor.
 * PERMV variables will have been initialized from the template,
 * and all other variables are default-initialized as unbound.
 * The reference counter is set to ONE(1)
 * @throw E_BADALLOC: Failed to allocate sufficient memory. */
FUNDEF ATTR_RETNONNULL ATTR_MALLOC REF struct vm *KCALL vm_alloc(void);

/* Create a copy of the calling thread's VM, remapping memory in
 * both the old and new VM for copy-on-write, while sharing all
 * other mappings for reading.
 * Note however that mappings created with `PROT_LOOSE' will now
 * appear in the new (returned) VM.
 * This function is called to implement `CLONE_VM' for `clone()' and `unshare()' */
FUNDEF ATTR_RETNONNULL ATTR_MALLOC REF struct vm *KCALL vm_clone(void);

/* Destroy a previously allocated vm. */
FUNDEF void KCALL vm_destroy(struct vm *__restrict self);

/* Increment/decrement the reference counter of the given VM `x' */
#define vm_incref(x)  ATOMIC_FETCHINC((x)->vm_refcnt)
#define vm_decref(x) (ATOMIC_DECFETCH((x)->vm_refcnt) || (vm_destroy(x),0))

/* Map the memory region into the current VM.
 * NOTE: If the `page_index' refers to a kernel-space, the mapping
 *       will be placed in `vm_kernel', rather than within the
 *       current VM.
 * NOTE: Any previously existing mapping at the given address range
 *       will be overwritten.
 * The caller must invoke `vm_sync()' after changes have been made.
 * @param: prot: Set of `PROT_*' from <sys/mman.h>
 * @throw: E_BADALLOC: Not enough available memory to allocate control structures.
 * @throw: * :         Failed to map the region for some reason. */
FUNDEF void KCALL vm_mapat(vm_vpage_t page_index, size_t num_pages,
                           vm_raddr_t region_start, struct vm_region *__restrict region,
                           vm_prot_t prot, vm_notify_t notify, void *closure);

/* Change the protection within a given address range.
 * The caller must invoke `vm_sync()' after changes have been made.
 * @param: mask:       Set of `PROT_*' from <sys/mman.h> that should be kept.
 * @param: flag:       Set of `PROT_*' from <sys/mman.h> that should be added.
 * @throw: E_BADALLOC: Not enough available memory to allocate control structures.
 * @return: * :        The actual number of affected pages. (Not all must have necessarily changed, though) */
FUNDEF size_t KCALL vm_protect(vm_vpage_t page_index, size_t num_pages,
                               vm_prot_t mask, vm_prot_t flag,
                               unsigned int mode, void *tag);
#define VM_PROTECT_NORMAL  0x0000 /* Change protection on all mappings. */
#define VM_PROTECT_TAG     0x0001 /* Only change protection on nodes who's `vn_closure' argument matches the given `tag' */

/* Delete all memory mappings with the given address range.
 * The caller must invoke `vm_sync()' after changes have been made.
 * @param: mode:       Unmapping mode (one of `VM_UNMAP_*')
 * @throw: E_BADALLOC: [!VM_UNMAP_NOEXCEPT] Not enough available memory to allocate control structures.
 * @return: * :        The actual number of unmapped pages. */
FUNDEF size_t KCALL vm_unmap(vm_vpage_t page_index, size_t num_pages,
                             unsigned int mode, void *tag);
#endif /* __CC__ */
#define VM_UNMAP_NORMAL    0x0000 /* Unmap all mappings. */
#define VM_UNMAP_TAG       0x0001 /* Only unmap nodes who's `vn_closure' argument matches the given `tag' */
#define VM_UNMAP_NOEXCEPT  0x0002 /* Ignore E_BADALLOC errors and simply log them. */
#define VM_UNMAP_IMMUTABLE 0x0004 /* Also unmap immutable nodes (Must only be set internally when the node will be re-mapped elsewhere). */
#define VM_UNMAP_SYNC      0x8000 /* Automatically sync unmapped address ranges. */


#ifdef __CC__
struct vm_maps {
    LIST_HEAD(struct vm_node) v_maps; /* [0..1] Ordered list of nodes.
                                       * NOTE: These nodes have been unlinked from any address tree,
                                       *       and their `a_vmin' is offset from the original `page_index'
                                       *       passed to `vm_extract'.
                                       *       They are chained together with the `vn_byaddr' chain. */
};

/* Delete VM mappings after they have been extracted. */
FUNDEF void KCALL
vm_maps_delete(struct vm_maps self);

/* Change protection characteristics of the given maps. */
FUNDEF void KCALL
vm_maps_chprot(struct vm_maps self,
               vm_prot_t prot_mask,
               vm_prot_t prot_flag);

/* Extract all nodes from the given address range and return them.
 * This function is similar to `vm_unmap()' in that it unmaps memory,
 * however rather than deleting extracted mappings, it instead returns
 * them as a structure that can be used to `vm_restore()' them elsewhere.
 * @param: page_index: The starting page index.
 * @param: num_pages:  The number of consecutive page index locations to search, starting at `page_index'
 * @param: mode:       Set of `VM_EXTRACT_*'
 * @param: tag:        Tag used to select which mappings to extract. */
FUNDEF struct vm_maps KCALL vm_extract(VIRT vm_vpage_t page_index, size_t num_pages,
                                       unsigned int mode, void *tag);

/* Restore previously extracted VM mappings, now offset from `page_index'.
 * WARNING: After a successful call to this function, you mustn't call
 *         `vm_maps_delete()', as this function will inherit the saved
 *          nodes.
 * NOTE: Existing mappings within the associated address range are deleted.
 * @param: mode: One of `VM_RESTORE_*'
 * @return: * :  The actual number of restored pages. */
FUNDEF size_t KCALL vm_restore(VIRT vm_vpage_t page_index,
                               struct vm_maps maps, unsigned int mode);
#endif /* __CC__ */

#define VM_EXTRACT_NORMAL 0x0000 /* Extract any mapping. */
#define VM_EXTRACT_TAG    0x0001 /* Only extract nodes who's `vn_closure' argument matches the given `tag' */
#define VM_EXTRACT_FULL   0x0002 /* Fail (return empty maps) if the sum of all nodes that would have been returned wouldn't equal `num_pages' */

#define VM_RESTORE_NORMAL VM_NOTIFY_RESTORE    /* Restore mappings within the same VM. */
#define VM_RESTORE_NEWVM  VM_NOTIFY_RESTORE_VM /* Restore mappings in a different VM. */



#ifdef __CC__
/* Remap all memory mappings with the given address range to another.
 * The caller must invoke `vm_sync()' after changes have been made.
 * Existing mappings at the new address will be overwritten.
 * The caller must ensure that the address ranges are within the same
 * visibility block (aka. the remap doesn't transfer ranges to/from the
 * kernel-share area, then back to user-space).
 * If the address range is part of the kernel-share segment, the remap()
 * is performed within `vm_kernel', rather than the current VM.
 * @param: prot_mask:  A mask and-ed with existing protection properties.
 * @param: prot_flag:  A mask or-ed with existing protection properties.
 * @param: mode:       Unmapping mode (one of `VM_UNMAP_*')
 * @throw: E_BADALLOC: Not enough available memory to allocate control structures.
 * @return: * :        The actual number of remapped pages. */
FUNDEF size_t KCALL vm_remap(vm_vpage_t old_page_index, size_t num_pages,
                             vm_vpage_t new_page_index, vm_prot_t prot_mask,
                             vm_prot_t prot_flag, unsigned int mode, void *tag);
#endif /* __CC__ */
#define VM_REMAP_NORMAL   0x0000 /* Remap any mapping. */
#define VM_REMAP_TAG      0x0001 /* Only remap nodes who's `vn_closure' argument matches the given `tag' */
#define VM_REMAP_FULL     0x0002 /* Fail (return 0) if `return' wouldn't be equal to `num_pages' */


#ifdef __CC__
/* Extend a VM node by `num_additional_pages' additional pages.
 * "Extend" may be the wrong term here, as in actuality, a new node
 * is created, either referring to the same region, or to a region
 * that is logically the same, which is then inserted relative to
 * the existing node.
 * NOTE: When a new region is created, file mappings will appear to be continuous,
 *       guard regions in the same direction are triggered, guard regions in the
 *       opposite direction have their lower memory allocated, physical memory
 *       mappings will have their first/last part extended, and user-defined
 *       regions will use properly adjusted delta values to mirror increased
 *       offsets from the original region address.
 * NOTE: No new region is created when the node found near `threshold' didn't
 *       map the entirety of the associated node, but rather only mapped a portion,
 *       in which case the new node will map the remaining memory.
 *       However, may not be done if `num_additional_pages' is greater than
 *       the unmapped portion, and a new region may be created regardless.
 * NOTE: This function does not delete existing mappings if there are any.
 * @param: threshold:             The threshold, bordering against which a node is extended.
 * @param: num_additional_pages:  The size of the extension (in bytes)
 * @param: prot_mask:             A mask and-ed to the protection bits of the extension.
 * @param: prot_flag:             A mask or-ed to the protection bits of the extension.
 * @param: mode:                  One of `VM_EXTEND_*'
 * @return: 0 :                   No node matching `mode's use of `threshold' was found.
 * @return: 0 :                   The memory range that the extension would have used is already in use.
 * @return: num_additional_pages: Successfully created the extension. */
FUNDEF size_t KCALL vm_extend(vm_vpage_t threshold, size_t num_additional_pages,
                              vm_prot_t prot_mask, vm_prot_t prot_flag,
                              unsigned int mode);
#endif /* __CC__ */
#define VM_EXTEND_ABOVE   0x0000 /* Extend a node ending at `threshold-1' upwards by `num_additional_pages'. */
#define VM_EXTEND_BELOW   0x0001 /* Extend a node starting at `threshold' downwards by `num_additional_pages'. */


#ifdef __CC__
/* Ensure that all memory within the given address range resides in-core.
 * To prevent memory from be re-swapped immediately, the caller should be
 * holding at least a read-lock on the effective VM.
 * @throw: E_BADALLOC: Insufficient virtual memory to (re-)allocate control structures.
 * @throw: E_BADALLOC: Insufficient physical memory to load everything into the core.
 * @return: * :        The actual number of physical pages that were allocated. */
FUNDEF size_t FCALL vm_unswap(vm_vpage_t page_index, size_t num_pages);

/* Synchronize changes to the current VM within the given address range.
 * In SMP, this function will automatically communicate changes to other
 * CPUs making use of the same VM, doing so asynchronously.
 * If the given address range is located within the kernel
 * share-segment, or if `vm_syncall()' was called, a
 * did-change RPC is broadcast to all other CPUs. */
FUNDEF void FCALL vm_sync(vm_vpage_t page_index, size_t num_pages);
FUNDEF void FCALL vm_syncone(vm_vpage_t page_index);
FUNDEF void FCALL vm_syncall(void);

/* Determine a suitable, free memory location for `num_pages'
 * aligned by a multiple of `min_alignment_in_pages' pages.
 * Additionally, try to maintain a gap of `min_gap_size' to
 * an other, existing mapping.
 * WARNING: The caller must be holding a lock to the effective VM (either vm_kernel, or THIS_VM).
 * NOTE: If `hint > X86_KERNEL_BASE_PAGE || (hint == X86_KERNEL_BASE_PAGE && (mode&VM_GETFREE_FABOVE))',
 *       search for a suitable, free memory location within the kernel page directory,
 *       though only within the kernel-share segment.
 *       The same goes the other way around, in which case only user-specific
 *       addresses are searched.
 *       However first searching through the kernel-share segment and failing to
 *       find a free spot, if the current VM is `vm_kernel', retry by searching
 *       its entirety (If `VM_GETFREE_FSTRICT' allows this).
 * @param: mode:                   Set of `VM_GETFREE_F*'
 * @param: hint:                   A hint used as base when searching for free memory ranges.
 * @param: min_alignment_in_pages: The minimum alignment required from the returned pointer (or `1')
 * @throw: E_BADALLOC(ERROR_BADALLOC_VIRTMEMORY): No more virtual memory available. */
FUNDEF vm_vpage_t KCALL
vm_getfree(vm_vpage_t hint, size_t num_pages,
           size_t min_alignment_in_pages,
           size_t min_gap_size, unsigned int mode);
#endif /* __CC__ */
#define VM_GETFREE_FABOVE      0x0000 /* Search for viable, free mappings that are `>= hint' */
#define VM_GETFREE_FBELOW      0x0001 /* Search for viable, free mappings that are `<= hint-num_pages'  */
#define VM_GETFREE_FSTRICT     0x0002 /* After a search for free space failed to yielding a viable memory
                                       * location in accordance to `VM_GETFREE_FABOVE|VM_GETFREE_FBELOW',
                                       * do _NOT_ re-attempt the search while looking to the other direction. */
#define VM_GETFREE_FFORCEGAP   0x0004 /* When no memory could be found (following `VM_GETFREE_FSTRICT' behavior),
                                       * do not re-attempt the search after dismissing a non-zero `min_gap_size'. */
#define VM_GETFREE_FNOSMARTGAP 0x0008 /* Enforce the gap for all memory mappings.
                                       * When not set, only require a gap before `VM_REGION_LOGUARD'
                                       * and after `VM_REGION_HIGUARD' mappings. */

#ifdef __CC__
/* Return the node at a given page, or NULL if none is mapped there.
 * WARNING: The caller must be holding a lock to the effective VM (either vm_kernel, or THIS_VM). */
FUNDEF struct vm_node *KCALL vm_getnode(vm_vpage_t page);
/* Return any node within the given address range, or NULL if none is mapped within.
 * WARNING: The caller must be holding a lock to the effective VM (either vm_kernel, or THIS_VM).
 * WARNING: The caller must also ensure that `min_page' and
 *         `max_page' both refer to the same effective VM. */
FUNDEF struct vm_node *KCALL vm_getanynode(vm_vpage_t min_page, vm_vpage_t max_page);

/* Load memory into the core / mark code as changed / acquire write permissions.
 * This is the main function that is invoked during a pagefault, or when `vm_read()'
 * or `vm_write()' is used to read/write memory from another Virtual Memory space.
 * @return: true:  Something was loaded, or something has changed.
 * @return: false: Nothing changed. (Re-check permissions and if they don't match, throw a SEGFAULT)
 * NOTE: If the given address range overlaps between user-space
 *       and kernel-space, only user-space will be loaded in-core. */
FUNDEF bool KCALL vm_loadcore(vm_vpage_t page, size_t num_pages,
                              unsigned int mode);
#endif /* __CC__ */
#define VM_LOADCORE_READ    0x00 /* Allocate missing pages. */
#define VM_LOADCORE_WRITE   0x02 /* Copy pages to acquire write-access after COW. */
#define VM_LOADCORE_USER    0x04 /* Only handle branches not marked as `PROT_NOUSER'. */
#define VM_LOADCORE_NOGUARD 0x08 /* Don't allocate guard pages when touching them. */
#define VM_LOADCORE_NOALOA  0x40 /* Don't lazily ALlocate On Access. */
#define VM_LOADCORE_LOCKED  0x80 /* Only load core memory that is also locked. */


#ifdef __CC__
/* Lock/Unlock memory mappings within the given address range.
 * NOTE: locking/unlocking works recursively in both ways:
 *       locking memory twice required unlocking it twice.
 *       unlocking memory first requires you to lock it 2
 *       times before mlock()-semantics actually being to take place.
 * WARNING: Make sure to lock swap/storage-device drivers
 *          in memory, as they might get called when a
 *          pagefault occurs, meaning they must already
 *          be loaded at that point.
 * NOTE: The caller must ensure that the address range
 *       only maps to a single effective page directory.
 * @param: mode: A set of `VM_LOCK_*'
 * @return: * :       The actual amount of pages locked/unlocked.
 * @throw E_BADALLOC: Not enough available memory. */
FUNDEF size_t KCALL vm_lock(vm_vpage_t page, size_t num_pages, unsigned int mode);
#endif /* __CC__ */
#define VM_LOCK_UNLOCK  0x00                /* Unlock previously locked memory. */
#define VM_LOCK_LOCK    0x01                /* When set, lock memory instead of unlocking it. */
#define VM_LOCK_INCORE  VM_LOADCORE_LOCKED  /* Make sure that all locked pages are also present in-core. */
#define VM_LOCK_WRITE   VM_LOADCORE_WRITE   /* [for:VM_LOCK_INCORE] Copy pages to acquire write-access after COW. */
#define VM_LOCK_USER    VM_LOADCORE_USER    /* [for:VM_LOCK_INCORE] Only handle branches not marked as `PROT_NOUSER'. */
#define VM_LOCK_NOGUARD VM_LOADCORE_NOGUARD /* [for:VM_LOCK_INCORE] Don't allocate guard pages when touching them. */


#ifdef __CC__
/* A helper function combining `vm_getfree()' with `vm_mapat()'
 * @param: hint:                   Same as `hint' of `vm_getfree()'
 * @param: num_pages:              The total number of pages that should be passed from `region'
 * @param: min_alignment_in_pages: The minimum alignment required from the returned pointer (or `1')
 * @param: min_gap_size:           Same as `min_gap_size' of `vm_getfree()'
 * @param: getfree_mode:           Same as `mode' of `vm_getfree()'
 * @param: region_start:           The starting address within `region', where mapping begins.
 * @param: region:                 The region that should be mapped.
 * @param: prot:                   The protection attributes that should be used.
 * @param: notify:                 An optional memory notification callback.
 * @param: closure:                The closure argument passed to `notify'
 * @throw: E_BADALLOC:             No virtual address matching the given requirements exists or is available.
 * @throw: E_BADALLOC:             Not enough available memory to allocate control structures.
 * @return: * :                    The virtual base address of the mapping (calculated using `vm_getfree()') */
FUNDEF VIRT void *KCALL vm_map(vm_vpage_t hint, size_t num_pages,
                               size_t min_alignment_in_pages,
                               size_t min_gap_size, unsigned int getfree_mode,
                               vm_raddr_t region_start, 
                               struct vm_region *__restrict region, vm_prot_t prot,
                               vm_notify_t notify, void *closure);

/* Read/write data to/from another VM (or the current; it doesn't matter)
 * @throw E_SEGFAULT: The given buffer is faulty, or memory at `other_addr'
 *                    has not been mapped, or has been mapped without the
 *                    permissions required for the operation.
 * NOTE: This function may or may not share the permission characteristics
 *       of `pagedir_map()', in that certain protection flags might imply
 *       others. */
FUNDEF void KCALL vm_read(struct vm *__restrict other_vm, vm_virt_t other_addr,
                          USER CHECKED void *buffer, size_t bufsize);
FUNDEF void KCALL vm_write(struct vm *__restrict other_vm, vm_virt_t other_addr,
                           USER CHECKED void const *buffer, size_t bufsize);

/* Return a pointer to the futex at the given address.
 * If not futex has been allocated at that address, allocate a new one.
 * @throw: E_SEGFAULT: The given address is part of the kernel-share segment.
 * @throw: E_SEGFAULT: The given address isn't mapped to any vm_region.
 * @throw: E_SEGFAULT: The vm_region at `addr' is typed as `VM_REGION_PHYSICAL' or `VM_REGION_RESERVED'.
 * @throw: E_BADALLOC: Failed to allocate a new futex where there was none before. */
FUNDEF ATTR_RETNONNULL REF struct futex *KCALL vm_futex(VIRT void *addr);
/* Similar to `vm_futex()', but never throw `E_SEGFAULT', but simply return NULL instead.
 * Additionally, don't allocate missing futex objects (meaning that
 * `E_BADALLOC' isn't thrown either), but return `NULL' for that case, too. */
FUNDEF REF struct futex *KCALL vm_getfutex(VIRT void *addr);

/* Clear the cache of pre-allocated futex objects,
 * returning the number of objects found in cache.
 * Since futex objects only exist as long as their reference counter is
 * non-ZERO, heavy user-space use of them leads to numerous allocations
 * and deallocations of futex objects. For that reason, a cache of free
 * futex objects exists that is consulted before using `kmalloc()' to
 * allocate more. */
FUNDEF size_t KCALL vm_futex_clearcache(void);

/* Unmap everything from user-space.
 * This function is called as part of the `exec()' system call and
 * must not be called from a thread using `&vm_kernel' for `THIS_VM'. */
FUNDEF void KCALL vm_unmap_userspace(void);

/* Ensure that memory in the given address range is writable to
 * user-space, if the host doesn't support copy-on-write semantics
 * in kernel-space (X86 without the CR0_WP bit).
 * This function has no effect when called by a `TASK_FKERNELJOB' thread,
 * and will guaranty that the marked memory will remain writable until
 * the end of the current system call (or rather, until the next jump
 * back to user-space)
 * Additionally, it is automatically called by `validate_writable()' and
 * `validate_writable_opt()', but must be manually called in places where
 * COW-enabled user-memory must be modified when already being validated
 * implicitly (such as in linkers)
 * NOTE: If the host supports copy-on-write in kernel-space,
 *       this function is a no-op.
 * If the given pointer isn't valid, this function will either do nothing,
 * or throw an E_SEGFAULT error (it's always does nothing when it's a no-op
 * all-together, but if it actually _does_ need to do something, it's undefined) */
#ifdef __INTELLISENSE__
FUNDEF void KCALL vm_cow(void *base, size_t num_bytes);
/* Helper functions for loading 1/2/4/8 bytes quickly. */
FUNDEF void KCALL vm_cowb(void *base);
FUNDEF void KCALL vm_coww(void *base);
FUNDEF void KCALL vm_cowl(void *base);
FUNDEF void KCALL vm_cowq(void *base);
FUNDEF void KCALL vm_cow_ptr(void *base);
#elif !defined(CONFIG_VM_NO_HOSTCOW)
#define vm_cow(base,num_bytes) (void)0
#define vm_cowb(base)          (void)0
#define vm_coww(base)          (void)0
#define vm_cowl(base)          (void)0
#define vm_cowq(base)          (void)0
#define vm_cow_ptr(base)       (void)0
#else
FUNDEF void KCALL __os_vm_cow(void *base, size_t num_bytes) ASMNAME("vm_cow");
FUNDEF void KCALL __os_vm_cowb(void *base) ASMNAME("vm_cowb");
FUNDEF void KCALL __os_vm_coww(void *base) ASMNAME("vm_coww");
FUNDEF void KCALL __os_vm_cowl(void *base) ASMNAME("vm_cowl");
FUNDEF void KCALL __os_vm_cowq(void *base) ASMNAME("vm_cowq");
FORCELOCAL void KCALL vm_cow(void *base, size_t num_bytes) {
 __os_vm_cow(base,num_bytes);
 COMPILER_WRITE_BARRIER();
}
FORCELOCAL void KCALL vm_cowb(void *base) {
 __os_vm_cowb(base);
 COMPILER_WRITE_BARRIER();
}
FORCELOCAL void KCALL vm_coww(void *base) {
 __os_vm_coww(base);
 COMPILER_WRITE_BARRIER();
}
FORCELOCAL void KCALL vm_cowl(void *base) {
 __os_vm_cowl(base);
 COMPILER_WRITE_BARRIER();
}
FORCELOCAL void KCALL vm_cowq(void *base) {
 __os_vm_cowq(base);
 COMPILER_WRITE_BARRIER();
}

#if __SIZEOF_POINTER__ == 4
#define vm_cow_ptr(base)  vm_cowl(base)
#elif __SIZEOF_POINTER__ == 8
#define vm_cow_ptr(base)  vm_cowq(base)
#elif __SIZEOF_POINTER__ == 2
#define vm_cow_ptr(base)  vm_coww(base)
#elif __SIZEOF_POINTER__ == 1
#define vm_cow_ptr(base)  vm_cowb(base)
#else
#define vm_cow_ptr(base)  vm_cow((base),__SIZEOF_POINTER__)
#endif
#endif


#ifndef CONFIG_NO_VIO
/* Create/Remove VM memory breakpoints.
 * These breakpoints are implemented using VIO,
 * meaning they're actually an unlimited resource.
 * Note however that memory access to pages containing memory breakpoints
 * is considerably slower than access to pages containing none. - Use sparingly!
 * @param: base:      The base address on which to start breaking.
 * @param: num_bytes: The amount of bytes that should be covered by the breakpoint.
 * @param: mode:      Either `VM_BREAKPOINT_REMOVE', or a set of `VM_BREAKPOINT_F*'
 * TODO: This function hasn't been implemented, yet. */
FUNDEF void KCALL vm_breakpoint(VIRT void *base, size_t num_bytes, unsigned int mode);
#define VM_BREAKPOINT_REMOVE 0x0000 /* Remove a breakpoint. */
#define VM_BREAKPOINT_FREAD  0x0001 /* Break on read. */
#define VM_BREAKPOINT_FWRITE 0x0002 /* Break on write. */

#endif /* !CONFIG_NO_VIO */


/* Extended VM functions */
FUNDEF void KCALL vm_insert_and_activate_node(struct vm *__restrict effective_vm,
                                              struct vm_node *__restrict node);
FUNDEF ATTR_NOTHROW void KCALL vm_insert_node(struct vm *__restrict effective_vm,
                                              struct vm_node *__restrict node);
FUNDEF void KCALL vm_map_node(struct vm_node *__restrict node);

/* Pop all nodes within the given page-range and
 * return a chain iterable through the `vn_byaddr'.
 * However, nodes are not unmapped during this process! */
FUNDEF struct vm_node *KCALL
vm_pop_nodes(struct vm *__restrict effective_vm,
             vm_vpage_t page_min, vm_vpage_t page_max,
             unsigned int mode, void *tag);

/* Split/merge virtual memory nodes at a given page index. */
FUNDEF void KCALL vm_split_before(struct vm *__restrict effective_vm, vm_vpage_t page_address);
FUNDEF ATTR_NOTHROW void KCALL vm_merge_before(struct vm *__restrict effective_vm, vm_vpage_t page_address);

#endif /* __CC__ */


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_VM_H */
