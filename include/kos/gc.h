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
#ifndef _KOS_GC_H
#define _KOS_GC_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <asm/types.h>

__SYSDECL_BEGIN

#if __SIZEOF_POINTER__ == 4
typedef __uint16_t gc_ver_t;
#elif __SIZEOF_POINTER__ == 8
typedef __uint32_t gc_ver_t;
#else
#error FIXME
#endif


struct gc_specs {
    /* Data descriptor detailing the layout of a GC data block node.
     * An example GC node and GC root node pair would look like this:
     * >> struct gc_node {
     * >>     ATREE_NODE(struct gc_node,uintptr_t) n_node;
     * >>     gc_ver_t                             n_reach;
     * >>     gc_ver_t                             n_visit;
     * >> };
     * >> struct gc_root {
     * >>     uintptr_t       r_min;
     * >>     uintptr_t       r_max;
     * >>     struct gc_root *r_next;
     * >> };
     * >> PRIVATE struct gc_specs const specs = {
     * >>     .gs_node = {
     * >>         .n_offsetof_atree = offsetof(struct gc_node,n_node),
     * >>         .n_offsetof_reach = offsetof(struct gc_node,n_reach),
     * >>         .n_offsetof_visit = offsetof(struct gc_node,n_visit),
     * >>         .n_next_NULL      = NULL,
     * >>     },
     * >>     .gs_root = {
     * >>         .r_offsetof_addrmin = offsetof(struct gc_root,r_min),
     * >>         .r_offsetof_addrmax = offsetof(struct gc_root,r_max),
     * >>         .r_offsetof_next    = offsetof(struct gc_root,r_next),
     * >>         .r_next_NULL        = NULL,
     * >>     }
     * >> };
     */
    struct {
        __ptrdiff_t   n_offsetof_atree;     /* [TYPE(uintptr_t)] Offset from a GC node to an `ATREE_NODE()'. */
        __ptrdiff_t   n_offsetof_reach;     /* [TYPE(gc_ver_t)] Offset of the version of the last leak-check iteration when this node was reached. */
        __ptrdiff_t   n_offsetof_visit;     /* [TYPE(gc_ver_t)] Offset of the version of the last leak-check iteration when this node was visited. */
        void         *n_next_NULL;          /*  NULL-pointer value that may be found in `gs_offsetof_next1' or
                                             * `gs_offsetof_next2' in order to indicate the lack of a follow-up pointer.
                                             * Usually set to `NULL'. */
    }                 gs_node;              /* Description on the data layout of a GC node. */
    struct {
        __ptrdiff_t   r_offsetof_addrmin;   /* [TYPE(uintptr_t)] Offset from a GC node to the address-min field. */
        __ptrdiff_t   r_offsetof_addrmax;   /* [TYPE(uintptr_t)] Offset from a GC node to the address-max field. */
        __uintptr_t   r_offsetof_next;      /* [TYPE(uintptr_t)] Offset to a pointer to another GC root node that should be searched. */
        void         *r_next_NULL;          /* NULL-pointer value that may be found in `r_offsetof_next'
                                             * in order to indicate the lack of a follow-up pointer.
                                             * Usually set to `NULL'. */
    }                 gs_root;              /* Description on the data layout of a GC root node. */
};

#define GC_SEARCH_FROOTONLY   0x0000 /* Only search the given chain of root pointers. */
#define GC_SEARCH_FDATABSS    0x0001 /* Search the `.data' and `.bss' sections (writable sections) of all loaded modules. */
#define GC_SEARCH_FTHREADREGS 0x0002 /* Search general-purpose registers of threads running in the same VM. */
#define GC_SEARCH_FTHREADSTCK 0x0004 /* Search the active portions of the stacks of threads running in the same VM. */

struct gc_data {
    void *gd_tree; /* GC node tree. */
    void *gd_root; /* Chain of GC root nodes. */
};

/* The max number of times that the `r_offsetof_next' chain is
 * walked before it is determined that it is in fact an infinite
 * loop (causing `E_INVALID_ARGUMENT' to be thrown).
 * NOTE: This is a hard kernel limit! */
#define GC_ROOT_CHAIN_MAX  8192


/* High-level, thread-safe GC search implementation that is meant to be used
 * as a way of detecting memory leaks to data blocks that are not reachable
 * by any conventional means (holding some kind of pointer to them, or a
 * pointer to another data which is then holding a pointer to them).
 * In order to be thread-safe, kernel-help is required which does the following:
 *   - Prevent modifications to mapped memory by other
 *     threads for the duration of the system call.
 *   - Disable lazy allocation of memory upon first access in the
 *     calling thread only for the duration of the system call.
 *     -> Memory that was never used mustn't be searched, and must
 *        not be containing any of the GC descriptor structures.
 *   - Using information from `specs' specs, `gc_root_chain',
 *     as well as `flags', check all memory regions described
 *     for valid pointers into GC nodes reachable from `gc_node_root'.
 *     All nodes found during this pass have their `n_offsetof_reach'
 *     field set to `current_version'
 *     The total number of nodes reached during this pass is added to
 *     the result.
 *   - To prevent the unnecessary overhead, as well as invalid detection
 *     of memory blocks reachable from the GC node tree itself, memory
 *     within which `data' resides is not searched if that page is part
 *     of the `.data' or `.bss' section when `GC_SEARCH_FDATABSS' is given.
 *   - Following this, recursively search the data blocks of all nodes
 *     marked as `n_offsetof_reach = current_version' during the first
 *     pass, as well as more nodes marked as such during this pass.
 *     Prior to being searched, the `n_offsetof_visit' field of the
 *     node will be set to `current_version'.
 *     Any further pointers found are once again used to lookup a GC
 *     node in `gc_node_root' and upon finding such a node, it is
 *     queued for additional searches, so-long as its `n_offsetof_visit'
 *     doesn't already match `current_version'.
 *   - If faulty pointers are detected in any part of the user-given
 *     pointers, or pointers reachable from there, an `E_SEGFAULT'
 *     error is thrown.
 *   - If `gc_node_root' is determined to be an invalid address tree
 *    (such as if you were to try and create an infinite loop, you
 *     evil person), `E_INVALID_ARGUMENT' is thrown.
 * @return: * : The number of reached nodes. */
__LIBC __size_t (__LIBCCALL gc_search)(struct gc_specs const *__restrict __specs, unsigned int __flags,
                                       struct gc_data *__restrict __data, gc_ver_t __current_version);


__SYSDECL_END

#endif /* !_KOS_GC_H */
