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
#ifndef _BITS_GENERIC_MMAN_H
#define _BITS_GENERIC_MMAN_H 1
#ifndef _BITS_MMAN_H
#define _BITS_MMAN_H 1
#endif

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#if (defined(__USE_KOS) || defined(__KERNEL__)) && defined(__CRT_KOS)
/* Highly versatile descriptor for mapping memory while providing a
 * large assortment of options to customize locating free memory,
 * as well as how memory should be initialized, and where it should be. */

#ifdef __CC__
struct mmap_virt {
    __intptr_t         mv_file;  /* [valid_if(!MAP_ANONYMOUS)] File descriptor of the file to map.
                                  *  WARNING: The kernel cannot guaranty that file mappings
                                  *           remain identical, especially when being modified
                                  *           after being mapped.
                                  *           In addition, mapping a file descriptor opened for
                                  *           writing using `PROT_WRITE' will allow you to modify
                                  *           mapped data to find the changes written back to
                                  *           disk at some point.
                                  *           The KOS kernel however does not make any attempts
                                  *           at synchronizing such actions and write-backs may
                                  *           be performed out of order, or become otherwise
                                  *           corrupted.
                                  *        >> Basically, the system can only work perfectly when
                                  *           everyone is just reading (as is the case most of the time),
                                  *           or when only 1 process is writing (at which point
                                  *           changes will appear on-disk at a random point in time).
                                  *  In my defense though, I chose to go this route because I've
                                  *  never encountered a good reason why write-back file mappings
                                  *  are something actually useful: read-only mappings make total
                                  *  sense, allowing for lazy loading of data from executables
                                  *  only when data is actually being accessed.
                                  *  But writing? - I guess it would be cool to create persistent
                                  *  application memory, such as a program that saves its configuration
                                  *  within its own binary. But other than that: It just seems outdated and lazy.
                                  *  And in addition, this change allowed me to remove the annoying restriction
                                  *  of (mv_off & (PAGESIZE-1) == 0) unix imposes on file mappings.
                                  *  And besides: As long as only once instance is running, write-mappings do fully work! */
    /* NOTE: File mappings are initialized as follows:
     *                 
     *  FILE: abcdefghijklmnopqrtuvwxyzABCDEFGHIJKLMNOPQRTUVWXYZ...
     *        |         |              |
     *        0    mv_off              mv_off+mv_len
     *                  |              |
     *                  +-+            +-+
     *                    |              |
     *  MEM:  XXXXXXXXXXXXklmnopqrtuvwxyzXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
     *        |           |              |                                   |
     *        0    mv_begin              mv_begin+mv_len              :mi_size
     * X: Initialized using 'mv_fill' */
    __uintptr_t        mv_begin; /* [valid_if(!MAP_ANONYMOUS)] The address within the region where the file mapping starts. */
    __pos64_t          mv_off;   /* [valid_if(!MAP_ANONYMOUS)] Offset into 'mv_file' where mapping starts. */
    __size_t           mv_len;   /* [valid_if(!MAP_ANONYMOUS)] Max amount of bytes to read from 'mv_file', starting at 'mv_off/mv_off64'.
                                  *  Clamped to `mi_size' when greater; when lower, difference is
                                  *  _always_ initialized with `mi_fill' (ignoring `MAP_UNINITIALIZED').
                                  *  >> Very useful when mapping program headers with a smaller memory-size than file-size. */
    __uintptr_t        mv_fill;  /* [valid_if(MAP_ANONYMOUS ? !MAP_UNINITIALIZED : mv_len < mi_size)]
                                  *  Filler double/quad-word for any undefined memory.
                                  *  On some platforms, a multi-byte pattern described by this may be
                                  *  used to initialize memory, but KOS only guaranties that the lowest byte
                                  * (that is `mi_fill & 0xff') will always be used, meaning that a consistent
                                  *  initialization of memory is only guarantied when `mi_fill == [0x0101010101010101|0x01010101]*(mi_fill & 0xff)'. */
    __size_t           mv_guard; /* [valid_if(MAP_GROWSDOWN||MAP_GROWSUP)] Size of the guard region in bytes.
                                  *  NOTE: When ZERO(0), both the `MAP_GROWSDOWN' and `MAP_GROWSUP' flags are ignored.
                                  *  NOTE: Clamped to `mi_size-PAGESIZE' when greater.
                                  *  HINT: mmap() has this field set to `PAGESIZE'.
                                  *  HINT: When a guard region that uses file initialization is copied,
                                  *        both 'mv_off' and 'mv_len' are updated accordingly! */
    __uint16_t         mv_funds; /* [valid_if(MAP_GROWSDOWN||MAP_GROWSUP)] Max amount of times the guard can replicate itself.
                                  *  HINT: mmap() has this field set to 'MMAP_VIRT_MAXFUNDS' (to emulate linux behavior)
                                  *  >> The total max amount of bytes that can be allocated by the guard is equal to 'mv_funds*mv_guard'. */
    /* XXX: Add some way of receiving signals from guards?
     *     (define a signal number to-be called when guard events occur) */
};
#endif /* __CC__ */
#define MMAP_VIRT_MAXFUNDS ((__uint16_t)-1)

#define XMAP_VIRTUAL    0x00000000 /* Allocate virtual memory, allowing for custom initializers or file mappings. */
#define XMAP_PHYSICAL   0x00000001 /* Allocate physical memory at a fixed hardware address.
                                    * NOTE: Using this flag requires the 'CAP_SYS_ADMIN' capability.
                                    * NOTE: When this mode is used, no memory pre-initialization can be performed!
                                    *       This is pure, low-level hardware access and only meant for ring#3
                                    *       drivers that need access to memory-mapped device buffers or I/O. */
#if __KOS_VERSION__ >= 300
#define XMAP_PHYSICAL64 0x00000002 /* Same as `XMAP_PHYSICAL', but use a 64-bit physical memory pointer. */
#define XMAP_USHARE     0x00000003 /* Map a pre-defined user-share data segment.
                                    * Which segment should be mapped is determined by `mu_name' */
#endif
#define XMAP_TYPE       0x0000000f /* Mask for known XMAP types. */
#define XMAP_FINDAUTO   0x00000000 /* [valid_if(!MAP_FIXED)] Find free memory the same way `mmap()' does:
                                    *  When `mi_addr' is NULL, find suitable memory within the thread's user heap/stack.
                                    *  Otherwise, do the same as if both 'XMAP_FINDBELOW' and 'XMAP_FINDABOVE' were set. */
#define XMAP_FINDBELOW  0x00000010 /* [valid_if(!MAP_FIXED)] Use the nearest unused address range above (considering `mi_addr...+=mi_size' first and 'CEIL_ALIGN(mi_addr+mi_size,mi_align)...+=mi_size' next). */
#define XMAP_FINDABOVE  0x00000020 /* [valid_if(!MAP_FIXED)] Use the nearest unused address range below (considering `mi_addr...+=mi_size' first and `FLOOR_ALIGN(mi_addr-mi_size,mi_align)...+=mi_size' next).
                                    *  NOTE: When both 'XMAP_FINDBELOW' and 'XMAP_FINDABOVE' are set, use whichever
                                    *        discovered range (if any) is located closer to `mi_addr...+=mi_size' */
#define XMAP_NOTRYNGAP  0x00000040 /* [valid_if(!MAP_FIXED)] When the first attempt to find free memory fails, and
                                    *                        `mi_gap' was non-zero, don't try again without any gap. */
#define XMAP_FORCEGAP   0x00000080 /* [valid_if(!MAP_FIXED)] Force a gap before & after all types of memory mappings.
                                    *  When not set, only try to keep a gap before `MAP_GROWSDOWN' and after `MAP_GROWSUP' */
#if __KOS_VERSION__ >= 300
#define XMAP_NOREMAP    0x00000100 /* [valid_if(MAP_FIXED)]
                                    * Don't re-map memory at a fixed address when something was already mapped there.
                                    * Instead, fail by returning `(void *)-1' without throwing an error. */
#endif

#define XUNMAP_ALL      0x00000000 /* Unmap all mappings, ignoring the given tag. */
#if __KOS_VERSION__ < 300
#define XUNMAP_TAG      0x00000010 /* Only unmap mappings matching the given tag. */
#else
#define XUNMAP_TAG      0x00000001 /* Only unmap mappings matching the given tag. */
#endif

/* mmap_info version control. */
#define MMAP_INFO_V1      0
#define MMAP_INFO_CURRENT MMAP_INFO_V1
#define mmap_info_v1      mmap_info

#ifndef __mmap_info_v1_defined
#define __mmap_info_v1_defined 1
#ifdef __CC__
struct mmap_phys {
#if __KOS_VERSION__ >= 300
    union {
#endif
        __PHYS void           *mp_addr;   /* [valid_if(XMAP_PHYSICAL)] Physical address at which memory should be allocated.
                                           * [REQUIRES(!MAP_FIXED || mp_addr & (PAGESIZE-1) == mi_addr & (PAGESIZE-1))]
                                           *  NOTE: When mapping memory at a non-fixed address, sub-page alignments are made to match. */
#if __KOS_VERSION__ >= 300
        __PHYS __UINT64_TYPE__ mp_addr64; /* [valid_if(XMAP_PHYSICAL64)] Same as `mp_addr', but 64-bit. */
    };
#endif
};
#if __KOS_VERSION__ >= 300
struct mmap_ushare {
    __UINTPTR_TYPE__       mu_name;  /* User-share segment ID (One of `USHARE_*' from <kos/ushare.h>) */
    __UINTPTR_TYPE__       mu_start; /* Offset into the named segment where mapping should start (Usually ZERO(0)). */
};
#endif
struct mmap_info_v1 {
    __UINTPTR_TYPE__       mi_prot;  /* Set of `PROT_*' */
    __UINTPTR_TYPE__       mi_flags; /* Set of `MAP_*' */
    __UINTPTR_TYPE__       mi_xflag; /* Set of 'XMAP_*' */
    __VIRT void           *mi_addr;  /* Base/hint address.
                                      * NOTE: When `MAP_FIXED' is given, this member must be page-aligned for virtual allocations.
                                      *       This differs somewhat when allocating physical memory, in which case it is required
                                      *       that the physical address `mi_phys.mp_addr' has the same sub-page alignment as `mi_addr'
                                      *    -> Otherwise no such restriction is necessary.
                                      * WARNING: When `MAP_FIXED' is given, `mi_addr+mi_size' must not overflow, or point into kernel-space! */
    size_t                 mi_size;  /* Size of the memory mapping (in bytes)
                                      * NOTE: This member is always ceil-aligned by PAGESIZE internally. */
    size_t                 mi_align; /* [valid_if(!MAP_FIXED)] Minimum alignment required when searching for free ranges.
                                      *  NOTE: This value must be a power-of-2 and is ignored when < PAGESIZE. */
    size_t                 mi_gap;   /* [valid_if(!MAP_FIXED)] When searching for suitable addresses and `mi_addr...+=mi_size'
                                      *  was already in use, any address range considered there-after must not be closer to another existing range than `mi_gap' bytes.
                                      *  HINT: This member is useful for discovering free memory while leaving a gap for guard mappings to expand into.
                                      *  HINT: mmap() has sets this argument to 16*PAGESIZE, not setting 'XMAP_NOTRYNGAP'. */
    void                  *mi_tag;   /* A virtual memory mapping tag that can later be re-used to identify this mapping. */
    union {
        struct mmap_virt   mi_virt;   /* [valid_if(mi_xflag&XMAP_TYPE == XMAP_VIRTUAL)] Virtual mapping data. */
        struct mmap_phys   mi_phys;   /* [valid_if(mi_xflag&XMAP_TYPE == XMAP_PHYSICAL ||
                                       *           mi_xflag&XMAP_TYPE == XMAP_PHYSICAL64)]
                                       * Physical mapping data. */
#if __KOS_VERSION__ >= 300
        struct mmap_ushare mi_ushare; /* [valid_if(mi_xflag&XMAP_TYPE == XMAP_USHARE)] User-share mapping data. */
#endif
    };
};
#endif /* __CC__ */
#endif /* !__mmap_info_v1_defined */

#if !defined(__KERNEL__) && defined(__CC__)
__REDIRECT_EXCEPT_(__LIBC,__PORT_KOSONLY_ALT(mmap),void *,__LIBCCALL,
                   xmmap,(struct mmap_info const *__data),xmmap1,(__data))

/* @param: flags: Set of `XUNMAP_*'; @return: * : Number of modified bytes */
__LIBC __PORT_KOSONLY_ALT(munmap) __ssize_t (__LIBCCALL xmunmap)(void *__addr, size_t __len, int __flags, void *__tag);

#if __KOS_VERSION__ >= 300
/* @param: flags: Set of `XUNMAP_*'; @return: * : Number of modified bytes */
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY_ALT(mprotect),__EXCEPT_SELECT(size_t,__ssize_t),__LIBCCALL,
                  xmprotect,(void *__addr, size_t __len, int __protmask, int __protflag, int __flags, void *__tag),(__addr,__len,__protmask,__protflag,__flags,__tag))
#endif
#endif /* !__KERNEL__ && !__CC__ */
#endif /* __USE_KOS && __CRT_KOS */

__SYSDECL_END

#endif /* !_BITS_GENERIC_MMAN_H */
