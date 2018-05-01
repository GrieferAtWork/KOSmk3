/* Copyright (c) 2018 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following __restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#define __EXPOSE_HEAP_INTERNALS 1
#define __OMIT_HEAP_CONSTANT_P_WRAPPERS 1
#include "libc.h"

#define OPTION_DEBUG_HEAP 1
//#define OPTION_CUSTOM_ALIGNMENT 1
#endif
#include "system.h"
#include "vm.h"
#include "heap.h"
#include "rtl.h"

#include <hybrid/section.h>
#include <hybrid/minmax.h>
#include <hybrid/align.h>
#include <kos/heap.h>
#include <stdbool.h>
#include <syslog.h>

#ifndef LOCAL_HEAP_TYPE_FCURRENT
#ifdef OPTION_DEBUG_HEAP
#ifdef OPTION_CUSTOM_ALIGNMENT
#   define LOCAL_HEAP_TYPE_FCURRENT  (HEAP_TYPE_FDEBUG|HEAP_TYPE_FALIGN)
#else
#   define LOCAL_HEAP_TYPE_FCURRENT   HEAP_TYPE_FDEBUG
#endif
#else /* OPTION_DEBUG_HEAP */
#ifdef OPTION_CUSTOM_ALIGNMENT
#   define LOCAL_HEAP_TYPE_FCURRENT   HEAP_TYPE_FALIGN
#else
#   define LOCAL_HEAP_TYPE_FCURRENT   HEAP_TYPE_FNORMAL
#endif
#endif /* !OPTION_DEBUG_HEAP */
#endif /* !LOCAL_HEAP_TYPE_FCURRENT */

#if LOCAL_HEAP_TYPE_FCURRENT == (HEAP_TYPE_FDEBUG|HEAP_TYPE_FALIGN)
#   define HSYM(x)  x##_da
#elif LOCAL_HEAP_TYPE_FCURRENT == HEAP_TYPE_FDEBUG
#   define HSYM(x)  x##_d
#elif LOCAL_HEAP_TYPE_FCURRENT == HEAP_TYPE_FALIGN
#   define HSYM(x)  x##_a
#elif LOCAL_HEAP_TYPE_FCURRENT == HEAP_TYPE_FNORMAL
#   define HSYM(x)  x
#else
#   error "Invalid current heap type"
#endif

#define LOCAL_HSYM(x) HSYM(local_##x)

#if !defined(NDEBUG) && 0
#define LIBC_HEAP_VALIDATE_ALL()  libc_heap_validate_all()
#else
#define LIBC_HEAP_VALIDATE_ALL() (void)0
#endif


/* Define the ABI for the address tree used by heaps. */
#ifndef LIBC_MFREE_TREE_ABI_DEFINED
#define LIBC_MFREE_TREE_ABI_DEFINED 1
#define ATREE(x)            mfree_tree_##x
#define ATREE_NODE_MIN      MFREE_MIN
#define ATREE_NODE_MAX      MFREE_MAX
#define Tkey                uintptr_t
#define T                   struct mfree
#define path                mf_laddr
#include <hybrid/list/atree-abi.h>
#endif /* !LIBC_MFREE_TREE_ABI_DEFINED */


#ifdef OPTION_CUSTOM_ALIGNMENT
#   define LOCAL_HEAP_ALIGNMENT     (self->h_almask+1)
#   define LOCAL_HEAP_BUCKET_OFFSET  1
#else
#   define LOCAL_HEAP_ALIGNMENT      __HEAP_GET_DEFAULT_ALIGNMENT(LOCAL_HEAP_TYPE_FCURRENT)
#if LOCAL_HEAP_ALIGNMENT == 4
#   define LOCAL_HEAP_BUCKET_OFFSET  3 /* FFS(LOCAL_HEAP_ALIGNMENT) */
#elif LOCAL_HEAP_ALIGNMENT == 8
#   define LOCAL_HEAP_BUCKET_OFFSET  4 /* FFS(LOCAL_HEAP_ALIGNMENT) */
#elif LOCAL_HEAP_ALIGNMENT == 16
#   define LOCAL_HEAP_BUCKET_OFFSET  5 /* FFS(LOCAL_HEAP_ALIGNMENT) */
#elif LOCAL_HEAP_ALIGNMENT == 32
#   define LOCAL_HEAP_BUCKET_OFFSET  6 /* FFS(LOCAL_HEAP_ALIGNMENT) */
#else
#error "Invalid default alignment"
#endif
#endif

#define LOCAL_HEAP_BUCKET_OF(size)   (((__SIZEOF_SIZE_T__*8)-__HEAP_CLZ(size))-LOCAL_HEAP_BUCKET_OFFSET)
#define LOCAL_HEAP_BUCKET_COUNT       ((__SIZEOF_SIZE_T__*8)-(LOCAL_HEAP_BUCKET_OFFSET-1))


#ifdef OPTION_CUSTOM_ALIGNMENT
#define LOCAL_HEAP_ALIGNMENT_MASK     (self->h_almask)
#define LOCAL_HEAP_ALIGNMENT_IMASK    (self->h_ialmask)
#else
#define LOCAL_HEAP_ALIGNMENT_MASK     (LOCAL_HEAP_ALIGNMENT-1)
#define LOCAL_HEAP_ALIGNMENT_IMASK  (~(LOCAL_HEAP_ALIGNMENT-1))
#endif

#define IS_HEAP_ALIGNED(x) (!((x) & LOCAL_HEAP_ALIGNMENT_MASK))


DECL_BEGIN
#define STRUCT_HEAP   struct LOCAL_HSYM(heap)
#define STRUCT_MFREE  struct LOCAL_HSYM(mfree)

#define LOCAL_SIZEOF_MFREE    offsetof(STRUCT_MFREE,mf_data)
#ifdef OPTION_CUSTOM_ALIGNMENT
#define LOCAL_HEAP_MINSIZE   (self->h_minsize)
#else
#define LOCAL_HEAP_MINSIZE  ((LOCAL_SIZEOF_MFREE + LOCAL_HEAP_ALIGNMENT_MASK) & LOCAL_HEAP_ALIGNMENT_IMASK)
#endif


struct PACKED LOCAL_HSYM(mfree) {
    LIST_NODE(STRUCT_MFREE)   mf_lsize;   /* [lock(:h_lock)][sort(ASCENDING(mf_size))] List of free entries ordered by size. */
    ATREE_XNODE(STRUCT_MFREE) mf_laddr;   /* [lock(:h_lock)][sort(ASCENDING(self))] List of free entries ordered by address. */
    __size_t                  mf_size;    /* Size of this block (in bytes; aligned by `LOCAL_HEAP_ALIGNMENT'; including this header) */
#define MFREE_FUNDEFINED      0x00        /* Memory initialization is undefined.
                                           * In debug mode, this means that memory is
                                           * initialized using `DEBUGHEAP_NO_MANS_LAND',
                                           * with portions that haven't been allocated yet
                                           * pending initialization for either `DEBUGHEAP_FRESH_MEMORY'
                                           * or ZERO(0), depending on how they were originally allocated. */
#define MFREE_FZERO           GFP_CALLOC  /* Memory is ZERO-initialized. */
#define MFREE_FMASK           MFREE_FZERO /* Mask of known flags. */
    __UINT8_TYPE__            mf_flags;   /* Set of `MFREE_F*' */
#ifdef OPTION_DEBUG_HEAP
    __UINT8_TYPE__            mf_szchk;   /* Checksum for `mf_size' */
#endif
    __BYTE_TYPE__             mf_data[1]; /* Block data. */
};


struct PACKED LOCAL_HSYM(heap) {
    __UINTPTR_HALF_TYPE__     h_type;       /* [== HEAP_TYPE_FCURRENT][const] The type of heap. */
    __UINTPTR_HALF_TYPE__     h_flags;      /* Heap flags (Set of `HEAP_F*'). */
    atomic_rwlock_t           h_lock;       /* Lock for this heap. */
    ATREE_HEAD(STRUCT_MFREE)  h_addr;       /* [lock(h_lock)][0..1] Heap sorted by address. */
    LIST_HEAD(STRUCT_MFREE)   h_size[LOCAL_HEAP_BUCKET_COUNT];
                                            /* [lock(h_lock)][0..1][*] Heap sorted by free range size. */
    WEAK __size_t             h_overalloc;  /* Amount (in bytes) by which to over-allocate memory in heaps.
                                             * NOTE: Set to ZERO(0) to disable overallocation. */
    WEAK __size_t             h_freethresh; /* Threshold that must be reached before any continuous block of free
                                             * data is unmapped from the kernel VM. (Should always be `>= PAGESIZE') */
    WEAK void                *h_corehint;   /* [valid_if(HEAP_FUSEHINTS)] Hint for where to allocate new memory. */
    ATOMIC_DATA __size_t      h_dangle;     /* [lock(INCREMENT(h_lock),DECREMENT(atomic),READ(atomic))]
                                             * Amount of dangling bytes of memory (memory that was allocated, but may be
                                             * release again shortly) When new memory would have to be requested from the
                                             * core, this field is checked to see if it is likely that some large block
                                             * of memory will be released soon, preventing a race condition that would
                                             * unnecessarily allocate more memory when `heap_free()' is merging a data
                                             * block with another, larger data block, for which it must temporarily
                                             * allocate that larger data block. Another thread allocating memory at the
                                             * same time may then think that the cache has grown too small for the allocation
                                             * and unnecessarily request more memory from the core. */
    struct heapstat           h_stat;       /* [lock(h_lock)] Heap statistics. */
#ifdef OPTION_DEBUG_HEAP
    LIST_NODE(STRUCT_HEAP)    h_chain;      /* [lock(INTERNAL(...))] Chain of all known debug heaps (for `LIBC_HEAP_VALIDATE_ALL()') */
#endif /* OPTION_DEBUG_HEAP */
#ifdef OPTION_CUSTOM_ALIGNMENT
    __size_t                  h_almask;     /* [const] Heap alignment mask (== HEAP_ALIGNMENT-1) */
    __size_t                  h_ialmask;    /* [const] Inverse heap alignment mask (== ~(HEAP_ALIGNMENT-1)) */
    __size_t                  h_minsize;    /* [const] Heap min size (== (sizeof(struct mfree) + HEAP_ALIGNMENT-1) & ~(HEAP_ALIGNMENT-1)) */
#endif
};                            


PRIVATE void *KCALL
HSYM(core_page_alloc)(STRUCT_HEAP *__restrict self,
                      size_t num_bytes, gfp_t flags) {
 void *result;
 struct mmap_info_v1 vmreq;
 if (flags&GFP_NOMAP)
     libc_error_throw(E_WOULDBLOCK);
 vmreq.mi_prot          = PROT_READ|PROT_WRITE;
 vmreq.mi_flags         = MAP_PRIVATE|MAP_ANONYMOUS;
 vmreq.mi_xflag         = XMAP_FINDAUTO;
 vmreq.mi_addr          = NULL;
 if (self->h_flags & HEAP_FUSEHINTS) {
  vmreq.mi_addr = self->h_corehint;
  if (self->h_flags & HEAP_FDOWNHINT)
       vmreq.mi_flags |= MAP_GROWSDOWN;
  else vmreq.mi_flags |= MAP_GROWSUP;
 }
 vmreq.mi_size          = num_bytes;
 vmreq.mi_align         = PAGESIZE;
 vmreq.mi_gap           = 0;
 if (flags & GFP_CALLOC) {
  vmreq.mi_virt.mv_fill = 0;
 } else {
#ifdef OPTION_DEBUG_HEAP
  vmreq.mi_virt.mv_fill = DEBUGHEAP_FRESH_MEMORY;
#else
  vmreq.mi_flags |= MAP_UNINITIALIZED;
#endif
 }
 vmreq.mi_virt.mv_guard = 0;
#if 1
 result = libc_Xxmmap1(&vmreq);
#else
 result = libc_xmmap1(&vmreq);
 if (result == MAP_FAILED)
     libc_heap_allocation_failed(num_bytes);
#endif
 if (self->h_flags & HEAP_FUSEHINTS)
     ATOMIC_CMPXCH(self->h_corehint,vmreq.mi_addr,result);
 ATOMIC_FETCHADD(self->h_stat.hs_mmap,num_bytes);
 return result;
}
PRIVATE bool KCALL
HSYM(core_page_allocat)(STRUCT_HEAP *__restrict self, void *address,
                        size_t num_bytes, gfp_t flags) {
 void *result;
 struct mmap_info_v1 vmreq;
 if (flags&GFP_NOMAP)
     libc_error_throw(E_WOULDBLOCK);
 vmreq.mi_prot  = PROT_READ|PROT_WRITE;
 vmreq.mi_flags = MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED;
 vmreq.mi_xflag = XMAP_FINDAUTO|XMAP_NOREMAP;
 vmreq.mi_addr  = address;
 vmreq.mi_size  = num_bytes;
 vmreq.mi_align = PAGESIZE;
 vmreq.mi_gap   = 0;
 if (flags & GFP_CALLOC) {
  vmreq.mi_virt.mv_fill = 0;
 } else {
#ifdef OPTION_DEBUG_HEAP
  vmreq.mi_virt.mv_fill = DEBUGHEAP_FRESH_MEMORY;
#else
  vmreq.mi_flags |= MAP_UNINITIALIZED;
#endif
 }
 vmreq.mi_virt.mv_guard = 0;
 result = libc_xmmap1(&vmreq);
 if (result == MAP_FAILED)
     return false;
 {
  size_t new_total;
  new_total = ATOMIC_ADDFETCH(self->h_stat.hs_mmap,num_bytes);
  if (self->h_stat.hs_mmap_peak < new_total)
      self->h_stat.hs_mmap_peak = new_total;
 }
 return true;
}

PRIVATE ATTR_NOTHROW void KCALL
HSYM(core_page_free)(STRUCT_HEAP *__restrict self, void *address_index,
                     size_t num_bytes, gfp_t UNUSED(flags)) {
 /* Use `sys_munmap()' to not clobber `errno' if something goes wrong. */
 ATOMIC_FETCHSUB(self->h_stat.hs_mmap,num_bytes);
 sys_munmap(address_index,num_bytes);
}







INTERN void LIBCCALL
HSYM(libc_heap_init)(STRUCT_HEAP *__restrict self,
                     size_t overalloc, size_t free_threshold,
                     u16 flags
#ifdef OPTION_CUSTOM_ALIGNMENT
                     , size_t heap_alignment
#endif
                     ) {
 libc_memset(self,0,sizeof(STRUCT_HEAP));
 self->h_type    = LOCAL_HEAP_TYPE_FCURRENT;
 self->h_flags   = flags;
#ifdef OPTION_CUSTOM_ALIGNMENT
 /* Initialize custom heap alignment fields. */
 self->h_almask  = heap_alignment-1;
 self->h_ialmask = ~(heap_alignment-1);
 self->h_minsize = (LOCAL_SIZEOF_MFREE + self->h_almask) & self->h_ialmask;
#endif
 atomic_rwlock_cinit(&self->h_lock);
 self->h_overalloc  = overalloc;
 self->h_freethresh = free_threshold;
 libc_heap_register_d((struct heap *)self);
}
INTERN void LIBCCALL
HSYM(libc_heap_fini)(STRUCT_HEAP *__restrict self) {
 libc_heap_unregister_d((struct heap *)self);
 /* TODO: release all remaining data back to the core. */
}



LOCAL ATTR_NOTHROW void KCALL
HSYM(heap_insert_node_unlocked)(STRUCT_HEAP *__restrict self,
                                STRUCT_MFREE *__restrict node) {
 STRUCT_MFREE **pslot,*slot;
 size_t num_bytes = node->mf_size;
 assertf(node->mf_size,"Empty node at %p",node);
 /* Insert the node into the address and size trees. */
 mfree_tree_insert((struct mfree **)&self->h_addr,
                   (struct mfree *)node);
 /* Figure out where the free-slot should go in the chain of free ranges. */
 pslot = &self->h_size[LOCAL_HEAP_BUCKET_OF(num_bytes)];
 while ((slot = *pslot) != NULL &&
         MFREE_SIZE(slot) < num_bytes)
         pslot = &slot->mf_lsize.le_next;
 node->mf_lsize.le_pself = pslot;
 node->mf_lsize.le_next  = slot;
 if (slot) slot->mf_lsize.le_pself = &node->mf_lsize.le_next;
 *pslot = node;
}

#ifdef OPTION_DEBUG_HEAP
PRIVATE void KCALL
HSYM(mfree_set_checksum)(STRUCT_MFREE *__restrict self) {
 self->mf_szchk = libc_mfree_get_checksum((struct mfree *)self);
}
#else
FORCELOCAL void KCALL
HSYM(mfree_set_checksum)(STRUCT_MFREE *__restrict UNUSED(self)) {
}
#endif


PRIVATE ATTR_NOTHROW void KCALL
HSYM(heap_free_raw)(STRUCT_HEAP *__restrict self,
                    void *ptr, size_t num_bytes,
                    gfp_t flags) {
 ATREE_SEMI_T(uintptr_t) addr_semi;
 ATREE_LEVEL_T addr_level;
 STRUCT_MFREE **pslot,*slot,*new_slot;
#ifdef OPTION_DEBUG_HEAP
 size_t dandle_size = 0;
again:
#endif /* OPTION_DEBUG_HEAP */
 assertf(num_bytes >= LOCAL_HEAP_MINSIZE,"Invalid heap_free(): Too few bytes (%Iu < %Iu)",num_bytes,LOCAL_HEAP_MINSIZE);
 assertf(IS_HEAP_ALIGNED((uintptr_t)ptr),"Invalid heap_free(): Unaligned base pointer %p",ptr);
 assertf(IS_HEAP_ALIGNED(num_bytes),"Invalid heap_free(): Unaligned free size %Iu (%#Ix)",num_bytes,num_bytes);
 assertf(((uintptr_t)ptr + num_bytes) > (uintptr_t)ptr,"Address space overflow when freeing %p...%p",ptr,(uintptr_t)ptr+num_bytes-1);
 LIBC_HEAP_VALIDATE_ALL();
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);

 atomic_rwlock_write(&self->h_lock);
 /* Search for a node below, that could then be extended above. */
 pslot = (STRUCT_MFREE **)mfree_tree_plocate_at((struct mfree **)&self->h_addr,
                                                (uintptr_t)ptr-1,
                                                &addr_semi,&addr_level);
 if (pslot) {
  slot = *pslot;
  asserte(mfree_tree_pop_at((struct mfree **)pslot,addr_semi,addr_level) == (struct mfree *)slot);
  LIST_REMOVE(slot,mf_lsize);

  /* Extend this node above. */
#ifndef OPTION_DEBUG_HEAP
  slot->mf_flags &= flags & GFP_CALLOC;
  slot->mf_size  += num_bytes;
#else
  if ((flags & GFP_CALLOC) && (slot->mf_flags & GFP_CALLOC)) {
   slot->mf_size += num_bytes;
  } else {
   ATOMIC_FETCHADD(self->h_dangle,slot->mf_size);
   dandle_size += slot->mf_size;
   atomic_rwlock_endwrite(&self->h_lock);
   LIBC_HEAP_VALIDATE_ALL();
   if (flags & GFP_CALLOC)
       sys_xreset_debug_data(ptr,DEBUGHEAP_NO_MANS_LAND,num_bytes),
       flags &= ~GFP_CALLOC;
   LIBC_HEAP_VALIDATE_ALL();
   if (slot->mf_flags & GFP_CALLOC)
       sys_xreset_debug_data((void *)((uintptr_t)slot+LOCAL_SIZEOF_MFREE),
                              DEBUGHEAP_NO_MANS_LAND,slot->mf_size-LOCAL_SIZEOF_MFREE);
   LIBC_HEAP_VALIDATE_ALL();
   ptr        = (void *)slot;
   num_bytes += slot->mf_size;
   libc_mempatl(ptr,DEBUGHEAP_NO_MANS_LAND,LOCAL_SIZEOF_MFREE);
   goto again;
  }
#endif

  /* Check if there is another node above that we must now merge with. */
  addr_semi  = ATREE_SEMI0(uintptr_t);
  addr_level = ATREE_LEVEL0(uintptr_t);
  pslot = (STRUCT_MFREE **)mfree_tree_plocate_at((struct mfree **)&self->h_addr,
                                                 (uintptr_t)ptr+num_bytes,
                                                 &addr_semi,&addr_level);
  if unlikely(pslot) {
   STRUCT_MFREE *high_slot;
   high_slot = *pslot;
   /* Include this high-slot in the union that will be freed. */
   asserte(mfree_tree_pop_at((struct mfree **)pslot,addr_semi,addr_level) == (struct mfree *)high_slot);
   LIST_REMOVE(high_slot,mf_lsize);
#ifndef OPTION_DEBUG_HEAP
   slot->mf_flags &= high_slot->mf_flags & GFP_CALLOC;
   slot->mf_size  += high_slot->mf_size;
   if (slot->mf_flags & GFP_CALLOC)
       libc_memset(high_slot,0,LOCAL_SIZEOF_MFREE);
#else
   if ((slot->mf_flags & GFP_CALLOC) &&
       (high_slot->mf_flags & GFP_CALLOC)) {
    slot->mf_size += high_slot->mf_size;
    libc_memset(high_slot,0,LOCAL_SIZEOF_MFREE);
   } else {
    ATOMIC_FETCHADD(self->h_dangle,high_slot->mf_size);
    dandle_size += high_slot->mf_size;
    atomic_rwlock_endwrite(&self->h_lock);
    LIBC_HEAP_VALIDATE_ALL();
    if (slot->mf_flags & GFP_CALLOC)
        sys_xreset_debug_data((void *)((uintptr_t)slot+LOCAL_SIZEOF_MFREE),
                               DEBUGHEAP_NO_MANS_LAND,slot->mf_size-LOCAL_SIZEOF_MFREE);
    LIBC_HEAP_VALIDATE_ALL();
    if (high_slot->mf_flags & GFP_CALLOC)
        sys_xreset_debug_data((void *)((uintptr_t)high_slot+LOCAL_SIZEOF_MFREE),
                               DEBUGHEAP_NO_MANS_LAND,high_slot->mf_size-LOCAL_SIZEOF_MFREE);
    LIBC_HEAP_VALIDATE_ALL();
    ptr       = (void *)slot;
    num_bytes = slot->mf_size + high_slot->mf_size;
    flags    &= ~GFP_CALLOC;
    libc_mempatl(slot,DEBUGHEAP_NO_MANS_LAND,LOCAL_SIZEOF_MFREE);
    libc_mempatl(high_slot,DEBUGHEAP_NO_MANS_LAND,LOCAL_SIZEOF_MFREE);
    LIBC_HEAP_VALIDATE_ALL();
    goto again;
   }
#endif
  }
  /* No high-slot. Just re-insert our new, bigger node. */
  new_slot = slot;
  goto load_new_slot;
 }

 /* Search for a node above, that could then be extended below. */
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);
 pslot = (STRUCT_MFREE **)mfree_tree_plocate_at((struct mfree **)&self->h_addr,
                                                (uintptr_t)ptr+num_bytes,
                                                &addr_semi,&addr_level);
 if (pslot) {
  gfp_t slot_flags; size_t slot_size;
  slot = *pslot;
  asserte(mfree_tree_pop_at((struct mfree **)pslot,addr_semi,addr_level) == (struct mfree *)slot);
  LIST_REMOVE(slot,mf_lsize);
  /* Extend this node below. */

#ifndef OPTION_DEBUG_HEAP
  slot_flags = slot->mf_flags & flags;
  slot_size  = slot->mf_size;
  if (slot_flags & GFP_CALLOC)
      libc_memset(slot,0,LOCAL_SIZEOF_MFREE);
  new_slot           = (STRUCT_MFREE *)ptr;
  new_slot->mf_size  = slot_size + num_bytes;
  new_slot->mf_flags = slot_flags;
#else
  slot_flags = slot->mf_flags;
  slot_size  = slot->mf_size;
  if ((slot_flags & GFP_CALLOC) && (flags & GFP_CALLOC)) {
   libc_memset(slot,0,LOCAL_SIZEOF_MFREE);
   new_slot           = (STRUCT_MFREE *)ptr;
   new_slot->mf_size  = slot_size + num_bytes;
   new_slot->mf_flags = slot_flags;
  } else {
   ATOMIC_FETCHADD(self->h_dangle,slot_size);
   dandle_size += slot_size;
   atomic_rwlock_endwrite(&self->h_lock);
   if (flags & GFP_CALLOC)
       sys_xreset_debug_data(ptr,DEBUGHEAP_NO_MANS_LAND,num_bytes);
   if (slot_flags & GFP_CALLOC)
        sys_xreset_debug_data(slot,DEBUGHEAP_NO_MANS_LAND,slot_size);
   else libc_mempatl(slot,DEBUGHEAP_NO_MANS_LAND,LOCAL_SIZEOF_MFREE);
   flags     &= ~GFP_CALLOC;
   num_bytes += slot_size;
   goto again;
  }
#endif
  /* Re-insert our new, bigger node. */
  goto load_new_slot;
 }

 /* Create a new node. */
 new_slot = (STRUCT_MFREE *)ptr;
 new_slot->mf_size  = num_bytes;
 new_slot->mf_flags = flags & MFREE_FMASK;
load_new_slot:
#ifdef OPTION_DEBUG_HEAP
 /* Untrack danging heap data. */
 ATOMIC_FETCHSUB(self->h_dangle,dandle_size);
#endif
 if (!(flags & GFP_NOMAP) &&
       new_slot->mf_size >= self->h_freethresh) {
  /*  When a slot grows larger than `self->h_freethresh' and
   * `GFP_NOMAP' isn't set, free as much of it as possible by
   *  passing its memory to `core_page_free()' */
  uintptr_t free_minaddr = CEIL_ALIGN(MFREE_MIN(new_slot),PAGESIZE);
  uintptr_t free_endaddr = FLOOR_ALIGN(MFREE_END(new_slot),PAGESIZE);
  if (free_minaddr < free_endaddr) {
   uintptr_t hkeep,tkeep;
   size_t hkeep_size,tkeep_size;
   /* Figure out the header and tail which we can't release. */
   hkeep      = MFREE_MIN(new_slot);
   tkeep      = free_endaddr;
   hkeep_size = free_minaddr-(uintptr_t)hkeep;
   assertf(MFREE_END(new_slot) >= (uintptr_t)tkeep,
           "new_slot = %p...%p\n"
           "tkeep    = %p\n",
           MFREE_MIN(new_slot),
           MFREE_MAX(new_slot),tkeep);
   tkeep_size = MFREE_END(new_slot)-(uintptr_t)tkeep;
   flags     &= ~GFP_CALLOC;
   flags     |= new_slot->mf_flags;
   /* Ensure that the keep-segments are large enough. */
   if unlikely(hkeep_size && hkeep_size < LOCAL_HEAP_MINSIZE)
      hkeep_size += PAGESIZE,free_minaddr += PAGESIZE;
   if unlikely(tkeep_size && tkeep_size < LOCAL_HEAP_MINSIZE)
      tkeep_size += PAGESIZE,tkeep -= PAGESIZE,free_endaddr -= PAGESIZE;
   if unlikely(free_minaddr >= free_endaddr)
      goto do_load_new_slot;
   atomic_rwlock_endwrite(&self->h_lock);
   /* Release this slot to the VM. */
   if (hkeep_size) {
    if (flags & GFP_CALLOC)
         libc_memset((void *)hkeep,0,LOCAL_SIZEOF_MFREE);
#ifdef OPTION_DEBUG_HEAP
    else libc_mempatl((void *)hkeep,DEBUGHEAP_NO_MANS_LAND,LOCAL_SIZEOF_MFREE);
#endif
   }
   if (tkeep_size) {
    if (flags & GFP_CALLOC)
         libc_memset((void *)tkeep,0,LOCAL_SIZEOF_MFREE);
#ifdef OPTION_DEBUG_HEAP
    else libc_mempatl((void *)tkeep,DEBUGHEAP_NO_MANS_LAND,LOCAL_SIZEOF_MFREE);
#endif
   }
   libc_syslog(LOG_DEBUG,
               "[HEAP] Release to the system: [%p+%#Ix head] %p...%p [%p+%#Ix tail]\n",
               hkeep,hkeep_size,free_minaddr,free_endaddr-1,
               tkeep,tkeep_size);

   /* Free unused header and tail data. */
   if (hkeep_size) HSYM(heap_free_raw)(self,(void *)hkeep,hkeep_size,flags);
   if (tkeep_size) HSYM(heap_free_raw)(self,(void *)tkeep,tkeep_size,flags);

   /* Release full pages back to the system. */
   HSYM(core_page_free)(self,
                       (void *)free_minaddr,
                        free_endaddr-free_minaddr,
                        flags);

   /* Reset the heap hint in a way that will keep our heap
    * around the same area of memory, preventing it from slowly
    * crawling across the entire address space. */
   if (self->h_flags & HEAP_FUSEHINTS) {
    void *old_hint;
    if (self->h_flags & HEAP_FDOWNHINT) {
     do if ((old_hint = ATOMIC_READ(self->h_corehint)) >= (void *)free_endaddr) break;
     while (!ATOMIC_CMPXCH_WEAK(self->h_corehint,old_hint,(void *)free_endaddr));
    } else {
     do if ((old_hint = ATOMIC_READ(self->h_corehint)) <= (void *)free_minaddr) break;
     while (!ATOMIC_CMPXCH_WEAK(self->h_corehint,old_hint,(void *)free_minaddr));
    }
   }
   LIBC_HEAP_VALIDATE_ALL();
   return;
  }
 }
do_load_new_slot:
 HSYM(mfree_set_checksum)(new_slot);
 /* Insert the node into the address and size trees. */
 HSYM(heap_insert_node_unlocked)(self,new_slot);
 /* Done! */
 atomic_rwlock_endwrite(&self->h_lock);
 LIBC_HEAP_VALIDATE_ALL();
}

INTERN void LIBCCALL
HSYM(libc_heap_free_untraced)(STRUCT_HEAP *__restrict self,
                              void *ptr, size_t num_bytes,
                              gfp_t flags) {
 assertf(num_bytes >= LOCAL_HEAP_MINSIZE,
         "Invalid heap_free(): Too few bytes (%Iu < %Iu) (ptr = %p)",
         num_bytes,LOCAL_HEAP_MINSIZE,ptr);
 assertf(IS_HEAP_ALIGNED((uintptr_t)ptr),
         "Invalid heap_free(): Unaligned base pointer %p",ptr);
 assertf(IS_HEAP_ALIGNED(num_bytes),
         "Invalid heap_free(): Unaligned free size %Iu (%#Ix)",num_bytes,num_bytes);
 /* Reset debug information. */
#ifdef OPTION_DEBUG_HEAP
 if (!(flags & GFP_CALLOC))
     sys_xreset_debug_data(ptr,DEBUGHEAP_NO_MANS_LAND,num_bytes);
#endif
 ATOMIC_FETCHSUB(self->h_stat.hs_alloc,num_bytes);
 HSYM(heap_free_raw)(self,ptr,num_bytes,flags);
}

INTERN size_t LIBCCALL
HSYM(libc_heap_trim)(STRUCT_HEAP *__restrict self,
                     size_t threshold) {
 size_t result = 0; STRUCT_MFREE **iter,**end;
 threshold = CEIL_ALIGN(threshold,PAGESIZE);
 if (!threshold) threshold = PAGESIZE;
again:
 /* Search all buckets for free data blocks of at least `threshold' bytes. */
 iter = &self->h_size[LOCAL_HEAP_BUCKET_OF(threshold)];
 end  =  COMPILER_ENDOF(self->h_size);
 atomic_rwlock_write(&self->h_lock);
 for (; iter != end; ++iter) {
  STRUCT_MFREE *chain;
  uintptr_t free_min;
  uintptr_t free_end;
  uintptr_t head_keep;
  uintptr_t tail_keep;
  void *tail_pointer;
  u8 free_flags;
  /* Search this bucket. */
  chain = *iter;
  while (chain &&
        (assertf(IS_ALIGNED(MFREE_SIZE(chain),HEAP_ALIGNMENT),
                           "MFREE_SIZE(chain) = 0x%Ix",
                            MFREE_SIZE(chain)),
         MFREE_SIZE(chain) < threshold))
         chain = chain->mf_lsize.le_next;
  if (!chain) continue;
  /* Figure out how much we can actually return to the core. */
  free_min = MFREE_BEGIN(chain);
  free_end = MFREE_END(chain);
  free_min = CEIL_ALIGN(free_min,PAGESIZE);
  free_end = FLOOR_ALIGN(free_end,PAGESIZE);
  if unlikely(free_min+threshold > free_end)
     continue; /* Even though the range is large enough, due to alignment it doesn't span a whole page */
  /* Figure out how much memory must be kept in the
   * head and tail portions of the free data block. */
  head_keep = free_min-MFREE_BEGIN(chain);
  tail_keep = MFREE_END(chain)-free_end;
  /* Make sure that data blocks that cannot be freed are still
   * large enough to remain representable as their own blocks. */
  if (head_keep && head_keep < HEAP_MINSIZE) continue;
  if (tail_keep && tail_keep < HEAP_MINSIZE) continue;
  /* Remove this chain entry. */
  asserte(mfree_tree_remove((struct mfree **)&self->h_addr,MFREE_BEGIN(chain)) == (struct mfree *)chain);
  LIST_REMOVE(chain,mf_lsize);
  atomic_rwlock_endwrite(&self->h_lock);

  tail_pointer = (void *)((uintptr_t)MFREE_END(chain)-tail_keep);
  free_flags = chain->mf_flags;

  /* Reset memory contained within the header of the data block we just allocated. */
  if (free_flags & GFP_CALLOC)
       libc_memset(chain,0,LOCAL_SIZEOF_MFREE);
#ifdef OPTION_DEBUG_HEAP
  else libc_mempatl(chain,DEBUGHEAP_NO_MANS_LAND,LOCAL_SIZEOF_MFREE);
#endif

  /* Re-release the unused portions of the head and tail data blocks. */
  if (head_keep) HSYM(heap_free_raw)(self,chain,head_keep,free_flags);
  if (tail_keep) HSYM(heap_free_raw)(self,tail_pointer,tail_keep,free_flags);

  /* Release full pages in-between back to the core. */
  HSYM(core_page_free)(self,
                      (void *)free_min,
                       free_end-free_min,
                       free_flags);

  /* Keep track of how much has already been released to the core. */
  result += free_end-free_min;
  goto again;
 }
 atomic_rwlock_endwrite(&self->h_lock);
 return result;
}


/* Free a high-memory overallocation of `num_free_bytes'
 * at `overallocation_base', given the associated
 * base-pointer located at `base_pointer'
 * This function deals with debug-initialization in the
 * event that the overallocated base location hasn't been
 * allocated yet, in a way that prevent the associated
 * pages from being allocated during `heap_free_raw()' in
 * a way that would produce invalid (use-after-free) memory. */
#ifdef OPTION_DEBUG_HEAP
#define HEAP_FREE_OVERALLOCATION  HSYM(heap_free_overallocation)
PRIVATE void KCALL
HSYM(heap_free_overallocation)(STRUCT_HEAP *__restrict self,
                               void *base_pointer,
                               void *overallocation_base,
                               size_t num_free_bytes,
                               gfp_t flags) {
 assert(IS_HEAP_ALIGNED((uintptr_t)overallocation_base));
 assert(IS_HEAP_ALIGNED(num_free_bytes));
 /* Work around lazy initialization of new memory:
  *    Since memory is default-initialized as `DEBUGHEAP_FRESH_MEMORY',
  *    if the unused portion is located in a different page than the
  *    resulting data block, as well as ends at a page boundary (if it
  *    doesn't then we know there must be another free-node within the
  *    same page, because of which the page must have been allocated
  *    in the past), the we must manually fault that page and initialize
  *    it as no-mans-land.
  *    If we didn't do this, then slot initialization in `heap_free_raw'
  *    would cause the page to fault, leaving it initialized as fresh
  *    memory, rather than no-mans-land, which would cause heap validation
  *    to fail, suggesting that memory was used after it was freed. */
#if 0
 libc_syslog(LOG_DEBUG,
             "Check no mans land %p, %p, %p\n",
            (uintptr_t)overallocation_base + (LOCAL_SIZEOF_MFREE-1),base_pointer,
            (uintptr_t)overallocation_base+num_free_bytes);
#endif
 /* NOTE: Must compare the end of the written-to portion of
  *       the unused data block with the resulting pointer.
  *       Since we know that `overallocation_base > base_pointer',
  *       we must check if the outer-most byte of the unused
  *       data block (which is written to during the free()
  *       operation) is is a different page. */
 if ((((uintptr_t)overallocation_base + (LOCAL_SIZEOF_MFREE-1)) & ~(PAGESIZE-1)) !=
      ((uintptr_t)base_pointer & ~(PAGESIZE-1)) &&
    ((((uintptr_t)overallocation_base+num_free_bytes) & (PAGESIZE-1)) == 0 ||
        /* If the unused part ends in another page, then we must
         * reset debug data as well because we don't know if it
         * was ever allocated to begin with. */
     (((uintptr_t)overallocation_base+num_free_bytes) & (PAGESIZE-1)) !=
      ((uintptr_t)overallocation_base & ~(PAGESIZE-1)))
  /*&& !pagedir_ismapped(VM_ADDR2PAGE((uintptr_t)overallocation_base))*/) {
  size_t clear_size;
  clear_size = PAGESIZE-((uintptr_t)(overallocation_base) & (PAGESIZE-1));
  /* NOTE: If the free operation would go one page further,
   *       then we must clear the next page as well. */
  if (clear_size < LOCAL_SIZEOF_MFREE) clear_size += PAGESIZE;
  /* NOTE: When the clear size is larger than what is being un-used,
   *       then we must assume that some other node in the same page
   *       has already allocated that page. */
  if (clear_size <= num_free_bytes) {
   /* The the CALLOC flag is set, then the memory will initialize itself to ZERO */
   if (!(flags & GFP_CALLOC)) {
#if 0
    libc_syslog(LOG_DEBUG,
                "Reset no mans land for page %p...%p\n",
                overallocation_base,(uintptr_t)overallocation_base+clear_size-1);
#endif
    libc_mempatl(overallocation_base,DEBUGHEAP_NO_MANS_LAND,clear_size);
   }
  }
 }
 HSYM(heap_free_raw)(self,overallocation_base,
                     num_free_bytes,flags);
}
#else
/* Without debug initialization, this isn't a problem! */
#define HEAP_FREE_OVERALLOCATION(self,base_pointer,overallocation_base,num_free_bytes,flags) \
        HSYM(heap_free_raw)(self,overallocation_base,num_free_bytes,flags)
#endif


INTERN struct heapptr LIBCCALL
HSYM(libc_heap_alloc_untraced)(STRUCT_HEAP *__restrict self,
                               size_t num_bytes, gfp_t flags) {
 STRUCT_HEAP *EXCEPT_VAR xself = self;
 gfp_t EXCEPT_VAR xflags = flags;
 struct heapptr EXCEPT_VAR result; STRUCT_MFREE **iter,**end;
 if unlikely(__builtin_add_overflow(num_bytes,LOCAL_HEAP_ALIGNMENT_MASK,&result.hp_siz))
    libc_heap_allocation_failed(num_bytes);
 result.hp_siz &= LOCAL_HEAP_ALIGNMENT_IMASK;
 if unlikely(result.hp_siz < LOCAL_HEAP_MINSIZE)
             result.hp_siz = LOCAL_HEAP_MINSIZE;
 iter = &self->h_size[LOCAL_HEAP_BUCKET_OF(result.hp_siz)];
 end  =  COMPILER_ENDOF(self->h_size);
 assertf(iter >= self->h_size &&
         iter <  COMPILER_ENDOF(self->h_size),
         "LOCAL_HEAP_BUCKET_OF(%Iu) = %Iu/%Iu",
         result.hp_siz,LOCAL_HEAP_BUCKET_OF(result.hp_siz),
         COMPILER_LENOF(self->h_size));
search_heap:
 LIBC_HEAP_VALIDATE_ALL();
 atomic_rwlock_write(&self->h_lock);
 for (; iter != end; ++iter) {
  size_t dangle_size;
  size_t unused_size;
  STRUCT_MFREE *chain;
  gfp_t chain_flags;
  /* Search this bucket. */
  chain = *iter;
  while (chain && MFREE_SIZE(chain) < result.hp_siz)
         chain = chain->mf_lsize.le_next;
  if (!chain) continue;
  asserte(mfree_tree_remove((struct mfree **)&self->h_addr,MFREE_BEGIN(chain)) == (struct mfree *)chain);
  LIST_REMOVE(chain,mf_lsize);
  /* Track the potentially unused data size as dangling data. */
  dangle_size = MFREE_SIZE(chain)-result.hp_siz;
  ATOMIC_FETCHADD(self->h_dangle,dangle_size);
  atomic_rwlock_endwrite(&self->h_lock);
  LIBC_HEAP_VALIDATE_ALL();
  /* We've got the memory! */
  result.hp_ptr = (void *)chain;
  chain_flags = chain->mf_flags;
  unused_size = dangle_size;
  if (unused_size < LOCAL_HEAP_MINSIZE) {
   /* Remainder is too small. - Allocate it as well. */
   result.hp_siz += unused_size;
  } else {
   void *unused_begin = (void *)((uintptr_t)chain+result.hp_siz);
   /* Free the unused portion. */
   uintptr_t random_offset;
   if (!(self->h_flags & HEAP_FRANDOMIZE))
         goto without_random;
   /* Randomize allocated memory by shifting the
    * resulting pointer somewhere up higher. */
   random_offset  = libc_rand() % unused_size;
   random_offset &= LOCAL_HEAP_ALIGNMENT_IMASK;
   if (random_offset >= LOCAL_HEAP_MINSIZE) {
    /* Rather than allocating `chain...+=num_bytes', instead
     * allocate `chain+random_offset...+=num_bytes' and free
     * `chain...+=random_offset' and
     * `chain+random_offset+num_bytes...+=unused_size-random_offset' */
    if (chain_flags&MFREE_FZERO)
         libc_memset(chain,0,LOCAL_SIZEOF_MFREE);
#ifdef OPTION_DEBUG_HEAP
    else libc_mempatl(chain,DEBUGHEAP_NO_MANS_LAND,LOCAL_SIZEOF_MFREE);
#endif
    /* Free unused low memory. */
    HSYM(heap_free_raw)(self,chain,random_offset,
                       (flags&~(GFP_CALLOC))|chain_flags);
    /* Set the new resulting pointer. */
    result.hp_ptr = (void *)((uintptr_t)chain + random_offset);
    unused_size  -= random_offset;
    if (unused_size < LOCAL_HEAP_MINSIZE) {
     result.hp_siz += unused_size;
    } else {
     unused_begin = (void *)((uintptr_t)result.hp_ptr + result.hp_siz);
     HEAP_FREE_OVERALLOCATION(self,
                              result.hp_ptr,
                              unused_begin,
                              unused_size,
                             (flags&~(GFP_CALLOC))|chain_flags);
    }
   } else {
without_random:
    assert(unused_size < MFREE_SIZE(chain));
    /* Free the unused overallocation. */
    HEAP_FREE_OVERALLOCATION(self,
                             result.hp_ptr,
                             unused_begin,
                             unused_size,
                            (flags&~(GFP_CALLOC))|chain_flags);
   }
  }
  /* Now that it's been returned, the data is no longer dangling. */
  ATOMIC_FETCHSUB(self->h_dangle,dangle_size);
  /* Initialize the result memory. */
  if (flags&GFP_CALLOC) {
   if (chain_flags&MFREE_FZERO)
        libc_memset(result.hp_ptr,0,LOCAL_SIZEOF_MFREE);
   else libc_memset(result.hp_ptr,0,result.hp_siz);
  }
#ifdef OPTION_DEBUG_HEAP
  else {
   sys_xreset_debug_data(result.hp_ptr,
                         DEBUGHEAP_FRESH_MEMORY,
                         result.hp_siz);
  }
#endif
  assert(IS_HEAP_ALIGNED((uintptr_t)result.hp_ptr));
  assert(IS_HEAP_ALIGNED((uintptr_t)result.hp_siz));
  assert(result.hp_siz >= LOCAL_HEAP_MINSIZE);
  assert(result.hp_siz >= num_bytes);
  ATOMIC_FETCHADD(self->h_stat.hs_alloc,result.hp_siz);
  return result;
 }
 /* Check for dangling data and don't allocate new memory if enough exists. */
 if (ATOMIC_READ(self->h_dangle) >= result.hp_siz) {
  atomic_rwlock_endwrite(&self->h_lock);
  /* Let some other thread about to release dangling
   * data do so, then search the heap again. */
  libc_sched_yield();
  goto search_heap;
 }
 /* NOTE: Don't track page overflow from below as dangling data
  *       here, so-as not to confuse allocators that are holding
  *       a lock to `vm_kernel.vm_lock'.
  *       Otherwise, we must end up with a soft-lock:
  *        THREAD #1: (holding lock to `vm_kernel.vm_lock')
  *                   kmalloc(1234);
  *                   -> Sees dangling data from new allocation
  *                      currently being made by THREAD #2
  *                   -> Doesn't allocate new pages, but tries
  *                      to yield to THREAD #2 and jumps back
  *        THREAD #2: In `core_page_alloc()'; tracking dangling
  *                   data that THREAD #1 is waiting for.
  *                  `core_page_alloc()' doesn't return because
  *                   THREAD #1 is locking `vm_kernel'
  *                   THREAD #1 can't release that lock because
  *                   it is waiting for THREAD #2.
  *        -> Soft-lock!
  */
 atomic_rwlock_endwrite(&self->h_lock);

 /* No pre-allocated memory found. -> Allocate new memory. */
 {
  size_t EXCEPT_VAR page_bytes;
  size_t unused_size;
  if unlikely(__builtin_add_overflow(result.hp_siz,PAGESIZE-1,&page_bytes))
     libc_heap_allocation_failed(result.hp_siz);
#ifdef OPTION_DEBUG_HEAP
  /* Make sure that the heap has been registered. */
  if (!self->h_chain.le_pself)
       libc_heap_register_d((struct heap *)self);
#endif
  if (!(flags & GFP_NOOVER)) {
   /* Add overhead for overallocation. */
   if unlikely(__builtin_add_overflow(page_bytes,self->h_overalloc,&page_bytes))
      goto allocate_without_overalloc;
  }
  TRY {
   page_bytes   &= ~(PAGESIZE-1);
   result.hp_ptr = HSYM(core_page_alloc)(self,page_bytes,flags);
  } CATCH_HANDLED (E_BADALLOC) {
allocate_without_overalloc:
   /* Try again without overallocation. */
   page_bytes    = result.hp_siz + (PAGESIZE-1);
   page_bytes   &= ~(PAGESIZE-1);
   result.hp_ptr = HSYM(core_page_alloc)(xself,page_bytes,xflags);
  }
  /* Got it! */
  unused_size = page_bytes - result.hp_siz;
  if unlikely(unused_size < LOCAL_HEAP_MINSIZE)
   result.hp_siz = page_bytes;
  else {
   void *unused_begin = (void *)((uintptr_t)result.hp_ptr + result.hp_siz);
   /* Free unused size. */
   assert(IS_HEAP_ALIGNED(unused_size));
   assert(IS_HEAP_ALIGNED((uintptr_t)unused_begin));
#ifdef OPTION_DEBUG_HEAP
   if (!(flags&GFP_CALLOC))
         libc_mempatl(unused_begin,DEBUGHEAP_NO_MANS_LAND,unused_size);
#endif
   /* Release the unused memory. */
   HSYM(heap_free_raw)(xself,unused_begin,unused_size,xflags);
  }
 }
 assert(IS_HEAP_ALIGNED((uintptr_t)result.hp_ptr));
 assert(IS_HEAP_ALIGNED((uintptr_t)result.hp_siz));
 assert(result.hp_siz >= LOCAL_HEAP_MINSIZE);
 assert(result.hp_siz >= num_bytes);
 ATOMIC_FETCHADD(self->h_stat.hs_alloc,result.hp_siz);
 return result;
}


PRIVATE size_t KCALL
HSYM(heap_allat_partial)(STRUCT_HEAP *__restrict self,
                         void *__restrict ptr, gfp_t flags) {
 ATREE_SEMI_T(uintptr_t) addr_semi;
 ATREE_LEVEL_T addr_level; gfp_t slot_flags;
 size_t result; STRUCT_MFREE **pslot,*slot;
 assert(IS_HEAP_ALIGNED((uintptr_t)ptr));
again:
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);

 /* Check if the requested address is in cache. */
 atomic_rwlock_write(&self->h_lock);
 pslot = (STRUCT_MFREE **)mfree_tree_plocate_at((struct mfree **)&self->h_addr,
                                                (uintptr_t)ptr,
                                                &addr_semi,&addr_level);
 if (!pslot) {
  void *ptr_page;
  atomic_rwlock_endwrite(&self->h_lock);
#ifdef OPTION_DEBUG_HEAP
  /* Make sure that the heap has been registered. */
  if (!self->h_chain.le_pself)
       libc_heap_register_d((struct heap *)self);
#endif
  /* Not in cache. Try to allocate associated core memory. */
  ptr_page = (void *)FLOOR_ALIGN((uintptr_t)ptr,PAGESIZE);
  if (!HSYM(core_page_allocat)(self,ptr_page,1,flags))
       return 0;
#ifdef OPTION_DEBUG_HEAP
  libc_memsetl(ptr_page,
               DEBUGHEAP_NO_MANS_LAND,PAGESIZE/4);
#endif
  /* Release the page to the heap and allocate again.
   * NOTE: Set the `GFP_NOMAP' to prevent the memory
   *       from be unmapped immediately. */
  HSYM(heap_free_raw)(self,ptr_page,PAGESIZE,flags|GFP_NOMAP);
  goto again;
 }
 slot = *pslot;
 assert((uintptr_t)ptr >= MFREE_BEGIN(slot));
 if ((uintptr_t)ptr == MFREE_BEGIN(slot)) {
  /* Allocate this entire slot, then remove unused memory from the end. */
  asserte(mfree_tree_pop_at((struct mfree **)pslot,addr_semi,addr_level) == (struct mfree *)slot);
  LIST_REMOVE(slot,mf_lsize);
  atomic_rwlock_endwrite(&self->h_lock);
  LIBC_HEAP_VALIDATE_ALL();
  result     = slot->mf_size;
  slot_flags = /*(flags & __GFP_HEAPMASK) | */slot->mf_flags;
#ifndef OPTION_DEBUG_HEAP
  if ((slot_flags & GFP_CALLOC) && (flags & GFP_CALLOC))
      libc_memset(slot,0,LOCAL_SIZEOF_MFREE);
#else
  if (flags & GFP_CALLOC)
       libc_memset(slot,0,LOCAL_SIZEOF_MFREE);
  else libc_mempatl(slot,DEBUGHEAP_NO_MANS_LAND,LOCAL_SIZEOF_MFREE);
#endif
 } else {
  size_t free_offset = (uintptr_t)ptr - MFREE_BEGIN(slot);
  assert(IS_HEAP_ALIGNED(free_offset));
  if unlikely(free_offset < LOCAL_HEAP_MINSIZE) {
   /* The remaining part of the slot is too small.
    * Ask the core if it can allocate the the previous
    * page for us, so we can merge this slot with that
    * page to get a chance of leaving a part large enough
    * for us to return to the heap.
    * NOTE: If the slot doesn't start at a page boundary,
    *       we already know that the requested part has already
    *       been allocated (meaning this allocation is impossible) */
   void *slot_page;
   atomic_rwlock_endwrite(&self->h_lock);
   if (MFREE_BEGIN(slot) & (PAGESIZE-1))
       return 0; /* Not page-aligned. */
   slot_page = (void *)MFREE_BEGIN(slot);
   if unlikely(slot_page == 0)
       return 0; /* Shouldn't happen: can't allocate previous page that doesn't exist. */
#ifdef OPTION_DEBUG_HEAP
   /* Make sure that the heap has been registered. */
   if (!self->h_chain.le_pself)
        libc_heap_register_d((struct heap *)self);
#endif
   *(uintptr_t *)&slot_page -= PAGESIZE;
   if (!HSYM(core_page_allocat)(self,slot_page,PAGESIZE,0 /*flags & __GFP_HEAPMASK*/))
       return 0; /* Failed to allocate the associated core-page. */
#ifdef OPTION_DEBUG_HEAP
   libc_memsetl(slot_page,
                DEBUGHEAP_NO_MANS_LAND,PAGESIZE/4);
#endif
   /* Free the page, so-as to try and merge it with the slot from before.
    * NOTE: Set the `GFP_NOMAP' to prevent the memory
    *       from be unmapped immediately. */
   HSYM(heap_free_raw)(self,slot_page,PAGESIZE,/*(flags & __GFP_HEAPMASK)|*/GFP_NOMAP);
   goto again;
  }
  result = slot->mf_size-free_offset;
  if unlikely(result < LOCAL_HEAP_MINSIZE) {
   /* Too close to the back. - Try to allocate the next page. */
   uintptr_t slot_end = MFREE_END(slot);
   atomic_rwlock_endwrite(&self->h_lock);
   if (slot_end & (PAGESIZE-1))
       return 0; /* Not page-aligned. */
   if (!HSYM(core_page_allocat)(self,(void *)slot_end,PAGESIZE,0 /*flags & __GFP_HEAPMASK*/))
       return 0; /* Failed to allocate the associated core-page. */
#ifdef OPTION_DEBUG_HEAP
   libc_memsetl((void *)slot_end,DEBUGHEAP_NO_MANS_LAND,PAGESIZE/4);
#endif
   HSYM(heap_free_raw)(self,(void *)slot_end,PAGESIZE,/*(flags & __GFP_HEAPMASK)|*/GFP_NOMAP);
   goto again;
  }
  asserte(mfree_tree_pop_at((struct mfree **)pslot,addr_semi,addr_level) == (struct mfree *)slot);
  LIST_REMOVE(slot,mf_lsize);
  /* Trace leading free data as dangling. */
  ATOMIC_FETCHADD(self->h_dangle,free_offset);
  atomic_rwlock_endwrite(&self->h_lock);
  LIBC_HEAP_VALIDATE_ALL();
  slot_flags = /*(flags & __GFP_HEAPMASK) | */slot->mf_flags;
  if (slot_flags & GFP_CALLOC)
      libc_memset(slot,0,MIN(free_offset,LOCAL_SIZEOF_MFREE));
#ifdef OPTION_DEBUG_HEAP
  else {
   libc_mempatl(slot,DEBUGHEAP_NO_MANS_LAND,
                MIN(free_offset,LOCAL_SIZEOF_MFREE));
  }
#endif
  /* Release unused memory below the requested address. */
  HSYM(heap_free_raw)(self,(void *)MFREE_BEGIN(slot),
                      free_offset,slot_flags);
  ATOMIC_FETCHSUB(self->h_dangle,free_offset);
 }
 /* Initialize newly allocated memory according to what the caller wants. */
#ifdef OPTION_DEBUG_HEAP
 if (!(flags & GFP_CALLOC))
       sys_xreset_debug_data(ptr,DEBUGHEAP_FRESH_MEMORY,result);
#endif
 if ((flags & GFP_CALLOC) && !(slot_flags & GFP_CALLOC))
      libc_memset(ptr,0,result);
 assert(result >= LOCAL_HEAP_MINSIZE);
 ATOMIC_FETCHADD(self->h_stat.hs_alloc,result);
 return result;
}

INTERN size_t LIBCCALL
HSYM(libc_heap_allat_untraced)(STRUCT_HEAP *__restrict self,
                               void *__restrict ptr,
                               size_t num_bytes, gfp_t flags) {
 STRUCT_HEAP *EXCEPT_VAR xself = self;
 void *EXCEPT_VAR xptr = ptr;
 gfp_t EXCEPT_VAR xflags = flags;
 size_t unused_size,alloc_size;
 size_t EXCEPT_VAR result = 0;
 if unlikely(__builtin_add_overflow(num_bytes,LOCAL_HEAP_ALIGNMENT_MASK,&alloc_size))
    libc_heap_allocation_failed(num_bytes);
 alloc_size &= LOCAL_HEAP_ALIGNMENT_IMASK;
 if unlikely(alloc_size < LOCAL_HEAP_MINSIZE)
             alloc_size = LOCAL_HEAP_MINSIZE;
 /* Allocate memory from the given range. */
 while (result < alloc_size) {
  size_t COMPILER_IGNORE_UNINITIALIZED(part);
  /* Allocate the new missing part. */
  TRY {
   part = HSYM(heap_allat_partial)(self,
                                  (void *)((uintptr_t)ptr + result),
                                   flags);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   if (result)
       HSYM(libc_heap_free_untraced)(xself,xptr,result,xflags);
   error_rethrow();
  }
  if unlikely(!part) {
   /* Failed to allocate the entirety of the requested range.
    * Free what was already allocated. */
   if (result)
       HSYM(libc_heap_free_untraced)(self,ptr,result,flags);
   return 0;
  }
  result += part;
 }
 /* With everything now allocated, free what the caller didn't ask for. */
 assert(result >= alloc_size);
 unused_size = result - alloc_size;
 if (unused_size >= LOCAL_HEAP_MINSIZE) {
  HSYM(libc_heap_free_untraced)(self,
                               (void *)((uintptr_t)ptr + alloc_size),
                                unused_size,flags);
  result = alloc_size;
 }
 return result;
}

INTERN struct heapptr LIBCCALL
HSYM(libc_heap_align_untraced)(STRUCT_HEAP *__restrict self,
                               size_t min_alignment, ptrdiff_t offset,
                               size_t num_bytes, gfp_t flags) {
 struct heapptr result_base,result;
 size_t nouse_size,alloc_bytes;
 size_t heap_alloc_bytes;
 assert(min_alignment != 0);
 assertf(!(min_alignment & (min_alignment-1)),
         "Invalid min_alignment: %IX",min_alignment);
 /* Truncate the offset, if it was a multiple of `min_alignment'
  * HINT: This also ensures that `offset' is positive. */
 offset &= (min_alignment-1);
 /* Forward to the regular allocator when the constraints allow it. */
 if (min_alignment <= LOCAL_HEAP_ALIGNMENT && !offset)
     return HSYM(libc_heap_alloc_untraced)(self,num_bytes,flags);
 if unlikely(__builtin_add_overflow(num_bytes,LOCAL_HEAP_ALIGNMENT_MASK,&alloc_bytes))
    libc_heap_allocation_failed(num_bytes);
 alloc_bytes &= LOCAL_HEAP_ALIGNMENT_IMASK;
 if unlikely(alloc_bytes < LOCAL_HEAP_MINSIZE)
             alloc_bytes = LOCAL_HEAP_MINSIZE;
#if 1
 {
  struct heapptr result; STRUCT_MFREE **iter,**end;
  iter = &self->h_size[LOCAL_HEAP_BUCKET_OF(alloc_bytes)];
  end  =  COMPILER_ENDOF(self->h_size);
  assertf(iter >= self->h_size &&
          iter <  COMPILER_ENDOF(self->h_size),
          "LOCAL_HEAP_BUCKET_OF(%Iu) = %Iu/%Iu",
          alloc_bytes,LOCAL_HEAP_BUCKET_OF(alloc_bytes),
          COMPILER_LENOF(self->h_size));
  LIBC_HEAP_VALIDATE_ALL();
  atomic_rwlock_write(&self->h_lock);
  /* Search for existing free data that
   * fit the required alignment and offset. */
  for (; iter != end; ++iter) {
   STRUCT_MFREE *chain; void *hkeep,*tkeep;
   gfp_t chain_flags; uintptr_t alignment_base;
   size_t hkeep_size,tkeep_size;
   size_t dangle_size;
   /* Search this bucket. */
   chain = *iter;
   while (chain && MFREE_SIZE(chain) < alloc_bytes)
          chain = chain->mf_lsize.le_next;
   if (!chain) continue;
   /* Check if this chain entry can sustain our required alignment. */
   alignment_base  =  (uintptr_t)chain;
   alignment_base -=   offset;
   alignment_base +=  (min_alignment-1);
   alignment_base &= ~(min_alignment-1);
   alignment_base +=   offset;
   /* `alignment_base' is now the effective base address which we want to use.
    * However, in case it doesn't match the free-node, we must advance it to
    * the nearest correctly aligned address (by adding `min_alignment').
    * This is required to ensure that the unused portion at `chain' continues
    * to be large enough to be re-freed (should we choose to use this node) */
   if (alignment_base != (uintptr_t)chain) {
    while ((alignment_base-(uintptr_t)chain) < LOCAL_HEAP_MINSIZE)
        alignment_base += min_alignment;
   }
   /* Check if the node still contains enough memory for the requested allocation. */
   if ((alignment_base + alloc_bytes) > MFREE_END(chain))
       continue; /* The chain entry is too small once alignment was taken into consideration. */
   asserte(mfree_tree_remove((struct mfree **)&self->h_addr,MFREE_BEGIN(chain)) == (struct mfree *)chain);
   LIST_REMOVE(chain,mf_lsize);
   /* Trace potentially unused data as dangling. */
   dangle_size = chain->mf_size - alloc_bytes;
   ATOMIC_FETCHADD(self->h_dangle,dangle_size);
   atomic_rwlock_endwrite(&self->h_lock);
   /* All right! we can actually use this one! */
   result.hp_ptr = (void *)alignment_base;
   result.hp_siz = alloc_bytes;
   /* Figure out how much memory should be re-freed at the front and back. */
   chain_flags = chain->mf_flags;
   hkeep       = (void *)chain;
   hkeep_size  = alignment_base - (uintptr_t)chain;
   tkeep       = (void *)((uintptr_t)result.hp_ptr + alloc_bytes);
   tkeep_size  = MFREE_END(chain) - (uintptr_t)tkeep;
   if (tkeep_size && (self->h_flags & HEAP_FRANDOMIZE)) {
    /* Add a random offset to the resulting pointer. */
    uintptr_t random_offset;
    random_offset = libc_rand() % tkeep_size;
    random_offset &= ~(min_alignment-1);
    /* Make sure to only add the offset if hkeep's size will be large enough! */
    if ((hkeep_size + random_offset) >= LOCAL_HEAP_MINSIZE) {
     hkeep_size                   += random_offset;
     tkeep_size                   -= random_offset;
     *(uintptr_t *)&tkeep         += random_offset;
     *(uintptr_t *)&result.hp_ptr += random_offset;
    }
   }

   if (hkeep_size) {
    assert(hkeep_size >= LOCAL_HEAP_MINSIZE);
    /* Reset data of the head if we're to re-free them. */
    if (chain_flags & GFP_CALLOC)
         libc_memset(hkeep,0,LOCAL_SIZEOF_MFREE);
#ifdef OPTION_DEBUG_HEAP
    else libc_mempatl(hkeep,DEBUGHEAP_NO_MANS_LAND,LOCAL_SIZEOF_MFREE);
#endif
    /* Re-free the header pointer. */
    HSYM(heap_free_raw)(self,hkeep,hkeep_size,chain_flags);
   }
   if (tkeep_size < LOCAL_HEAP_MINSIZE) {
    /* The tail is too small (or non-existent).
     * -> We must allocate it, too. */
    result.hp_siz += tkeep_size;
   } else {
    /* Free the tail pointer. */
    HEAP_FREE_OVERALLOCATION(self,
                             result.hp_ptr,
                             tkeep,
                             tkeep_size,
                            (flags&~(GFP_CALLOC))|chain_flags);
   }
   /* Remove dangling data. */
   ATOMIC_FETCHSUB(self->h_dangle,dangle_size);
   /* Initialize the resulting memory. */
   if (flags&GFP_CALLOC) {
    if (chain_flags&MFREE_FZERO)
         libc_memset(result.hp_ptr,0,LOCAL_SIZEOF_MFREE);
    else libc_memset(result.hp_ptr,0,result.hp_siz);
   }
#ifdef OPTION_DEBUG_HEAP
   else {
    sys_xreset_debug_data(result.hp_ptr,
                          DEBUGHEAP_FRESH_MEMORY,
                          result.hp_siz);
   }
#endif
   assert(IS_HEAP_ALIGNED((uintptr_t)result.hp_ptr));
   assert(IS_ALIGNED((uintptr_t)result.hp_ptr+offset,min_alignment));
   assert(IS_HEAP_ALIGNED((uintptr_t)result.hp_siz));
   assert(result.hp_siz >= LOCAL_HEAP_MINSIZE);
   assert(result.hp_siz >= num_bytes);
   ATOMIC_FETCHADD(self->h_stat.hs_alloc,result.hp_siz);
   return result;
  }
  atomic_rwlock_endwrite(&self->h_lock);
 }
#endif
 /* Fallback: Use overallocation to assert alignment. */

 /* Must overallocate by at least `LOCAL_HEAP_MINSIZE',
  * so we can _always_ free unused lower memory. */
 if unlikely(__builtin_add_overflow(alloc_bytes,min_alignment,&heap_alloc_bytes))
    libc_heap_allocation_failed(MAX(alloc_bytes,min_alignment));
 if unlikely(__builtin_add_overflow(heap_alloc_bytes,LOCAL_HEAP_MINSIZE,&heap_alloc_bytes))
    libc_heap_allocation_failed(heap_alloc_bytes);
 result_base = HSYM(libc_heap_alloc_untraced)(self,heap_alloc_bytes,flags);
 assert(result_base.hp_siz >= heap_alloc_bytes);
 result.hp_ptr = (void *)(CEIL_ALIGN((uintptr_t)result_base.hp_ptr+
                                                LOCAL_HEAP_MINSIZE+offset,
                                      min_alignment)-offset);
 assert((uintptr_t)result.hp_ptr+alloc_bytes <=
        (uintptr_t)result_base.hp_ptr+result_base.hp_siz);
 nouse_size = (uintptr_t)result.hp_ptr-(uintptr_t)result_base.hp_ptr;
 assert(nouse_size+alloc_bytes <= result_base.hp_siz);
 assertf(nouse_size >= LOCAL_HEAP_MINSIZE,"nouse_size = %Iu",nouse_size);
 /* Release lower memory (This _MUST_ work because we've overallocated by `LOCAL_HEAP_MINSIZE'). */
 HSYM(libc_heap_free_untraced)(self,result_base.hp_ptr,nouse_size,flags);
 result_base.hp_siz -= nouse_size;
 assert(result_base.hp_siz >= LOCAL_HEAP_MINSIZE);

 /* Try to release upper memory. */
 assert(result_base.hp_siz >= alloc_bytes);
 nouse_size = result_base.hp_siz-alloc_bytes;
 if (nouse_size >= LOCAL_HEAP_MINSIZE) {
  HSYM(libc_heap_free_untraced)(self,
                               (void *)((uintptr_t)result.hp_ptr+alloc_bytes),
                                nouse_size,flags);
  result_base.hp_siz -= nouse_size;
 }
 assert(result_base.hp_siz >= alloc_bytes);
 assert(IS_ALIGNED((uintptr_t)result.hp_ptr+offset,min_alignment));
 result.hp_siz = result_base.hp_siz;
 assert(IS_HEAP_ALIGNED((uintptr_t)result.hp_siz));
 assert(result.hp_siz >= LOCAL_HEAP_MINSIZE);
 assert(result.hp_siz >= num_bytes);
 return result;
}

#undef HEAP_FREE_OVERALLOCATION



INTERN struct heapptr LIBCCALL
HSYM(libc_heap_realloc_untraced)(STRUCT_HEAP *__restrict self,
                                 void *old_ptr,
                                 size_t old_bytes,
                                 size_t new_bytes,
                                 gfp_t alloc_flags,
                                 gfp_t free_flags) {
 STRUCT_HEAP *EXCEPT_VAR xself = self;
 gfp_t EXCEPT_VAR xalloc_flags = alloc_flags;
 struct heapptr EXCEPT_VAR result;
 size_t missing_bytes;
 assert(IS_HEAP_ALIGNED((uintptr_t)old_ptr));
 assert(IS_HEAP_ALIGNED(old_bytes));
 if (old_bytes == 0) /* Special case: initial allocation */
     return HSYM(libc_heap_alloc_untraced)(self,new_bytes,alloc_flags);
 if (__builtin_add_overflow(new_bytes,LOCAL_HEAP_ALIGNMENT_MASK,&new_bytes))
     libc_heap_allocation_failed(new_bytes-LOCAL_HEAP_ALIGNMENT_MASK);
 new_bytes &= LOCAL_HEAP_ALIGNMENT_IMASK;
 result.hp_ptr = old_ptr;
 result.hp_siz = old_bytes;
 if (new_bytes <= old_bytes) {
  size_t free_bytes;
  if unlikely(new_bytes < LOCAL_HEAP_MINSIZE)
              new_bytes = LOCAL_HEAP_MINSIZE;
  /* Free trailing memory. */
  free_bytes = old_bytes - new_bytes;
  if (free_bytes >= LOCAL_HEAP_MINSIZE) {
   HSYM(libc_heap_free_untraced)(self,(void *)((uintptr_t)old_ptr+new_bytes),
                                 free_bytes,free_flags);
   result.hp_siz = new_bytes;
  }
  return result;
 }
 missing_bytes = new_bytes - old_bytes;
 missing_bytes = HSYM(libc_heap_allat_untraced)(self,
                                               (void *)((uintptr_t)old_ptr+old_bytes),
                                                missing_bytes,
                                                alloc_flags);
 if (missing_bytes) {
  /* Managed to extend the data block. */
  result.hp_siz += missing_bytes;
  return result;
 }
 /* Must allocate an entirely new data block and copy memory to it. */
 result = HSYM(libc_heap_alloc_untraced)(self,new_bytes,alloc_flags);
 TRY {
  /* The try block is here because of the possibility of a LOA failure. */
  libc_memcpy(result.hp_ptr,old_ptr,old_bytes);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  HSYM(libc_heap_free_untraced)(xself,result.hp_ptr,result.hp_siz,
                                xalloc_flags & ~GFP_CALLOC);
  error_rethrow();
 }
 /* Free the old data block. */
 HSYM(libc_heap_free_untraced)(self,old_ptr,old_bytes,
                               free_flags & ~GFP_CALLOC);
 return result;
}

INTERN struct heapptr LIBCCALL
HSYM(libc_heap_realign_untraced)(STRUCT_HEAP *__restrict self,
                                 void *old_ptr, size_t old_bytes,
                                 size_t min_alignment, ptrdiff_t offset,
                                 size_t new_bytes, gfp_t alloc_flags,
                                 gfp_t free_flags) {
 struct heapptr result; size_t missing_bytes;
 assert(IS_HEAP_ALIGNED((uintptr_t)old_ptr));
 assert(IS_HEAP_ALIGNED(old_bytes));
 assert(!old_bytes || old_bytes >= LOCAL_HEAP_MINSIZE);
 if (old_bytes == 0) /* Special case: initial allocation */
     return HSYM(libc_heap_align_untraced)(self,min_alignment,offset,new_bytes,alloc_flags);
 if (__builtin_add_overflow(new_bytes,LOCAL_HEAP_ALIGNMENT_MASK,&new_bytes))
     libc_heap_allocation_failed(new_bytes-LOCAL_HEAP_ALIGNMENT_MASK);
 new_bytes &= LOCAL_HEAP_ALIGNMENT_IMASK;
 result.hp_ptr = old_ptr;
 result.hp_siz = old_bytes;
 if (new_bytes <= old_bytes) {
  size_t free_bytes;
  if unlikely(new_bytes < LOCAL_HEAP_MINSIZE)
              new_bytes = LOCAL_HEAP_MINSIZE;
  /* Free trailing memory. */
  free_bytes = old_bytes - new_bytes;
  if (free_bytes >= LOCAL_HEAP_MINSIZE) {
   HSYM(libc_heap_free_untraced)(self,
                                (void *)((uintptr_t)old_ptr+new_bytes),
                                 free_bytes,
                                 free_flags);
   result.hp_siz = new_bytes;
  }
  return result;
 }
 missing_bytes = new_bytes - old_bytes;
 missing_bytes = HSYM(libc_heap_allat_untraced)(self,
                                               (void *)((uintptr_t)old_ptr+old_bytes),
                                                missing_bytes,
                                                alloc_flags);
 if (missing_bytes) {
  /* Managed to extend the data block. */
  result.hp_siz += missing_bytes;
  return result;
 }
 /* Must allocate an entirely new data block and copy memory to it. */
 result = HSYM(libc_heap_align_untraced)(self,
                                         min_alignment,
                                         offset,
                                         new_bytes,
                                         alloc_flags);
 LIBC_TRY {
  /* The try block is here because of the possibility of a LOA failure. */
  libc_memcpy(result.hp_ptr,old_ptr,old_bytes);
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  HSYM(libc_heap_free_untraced)(self,result.hp_ptr,result.hp_siz,
                                alloc_flags & ~GFP_CALLOC);
  error_rethrow();
 }
 /* Free the old data block. */
 HSYM(libc_heap_free_untraced)(self,old_ptr,old_bytes,
                               free_flags & ~GFP_CALLOC);
 return result;
}










/* Traced memory allocators. */
#ifdef OPTION_DEBUG_HEAP
INTERN struct heapptr LIBCCALL
HSYM(libc_heap_alloc)(STRUCT_HEAP *__restrict self,
                      size_t num_bytes, gfp_t flags) {
 struct heapptr result;
 result = HSYM(libc_heap_alloc_untraced)(self,num_bytes,flags);
 LIBC_TRY {
  libc_heap_trace(result.hp_ptr,result.hp_siz,1);
 } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  HSYM(libc_heap_free_untraced)(self,result.hp_ptr,result.hp_siz,flags);
  error_rethrow();
 }
 return result;
}
INTERN struct heapptr LIBCCALL
HSYM(libc_heap_align)(STRUCT_HEAP *__restrict self,
                      size_t min_alignment, ptrdiff_t offset,
                      size_t num_bytes, gfp_t flags) {
 struct heapptr result;
 result = HSYM(libc_heap_align_untraced)(self,min_alignment,offset,num_bytes,flags);
 LIBC_TRY {
  libc_heap_trace(result.hp_ptr,result.hp_siz,1);
 } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  HSYM(libc_heap_free_untraced)(self,result.hp_ptr,result.hp_siz,flags);
  error_rethrow();
 }
 return result;
}
INTERN size_t LIBCCALL
HSYM(libc_heap_allat)(STRUCT_HEAP *__restrict self,
                      void *__restrict ptr,
                      size_t num_bytes, gfp_t flags) {
 size_t result;
 result = HSYM(libc_heap_allat_untraced)(self,ptr,num_bytes,flags);
 LIBC_TRY {
  libc_heap_trace(ptr,result,1);
 } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  HSYM(libc_heap_free_untraced)(self,ptr,result,flags);
  error_rethrow();
 }
 return result;
}
INTERN void LIBCCALL
HSYM(libc_heap_free)(STRUCT_HEAP *__restrict self,
                     void *ptr, size_t num_bytes,
                     gfp_t flags) {
 libc_heap_untrace(ptr);
 HSYM(libc_heap_free_untraced)(self,ptr,num_bytes,flags);
}
INTERN struct heapptr LIBCCALL
HSYM(libc_heap_realloc)(STRUCT_HEAP *__restrict self,
                        void *old_ptr, size_t old_bytes,
                        size_t new_bytes, gfp_t alloc_flags,
                        gfp_t free_flags) {
 struct heapptr result; size_t missing_bytes;
 assert(IS_HEAP_ALIGNED(old_bytes));
 assert(!old_bytes || IS_HEAP_ALIGNED((uintptr_t)old_ptr));
 assert(!old_bytes || old_bytes >= LOCAL_HEAP_ALIGNMENT);
 if (old_bytes == 0) /* Special case: initial allocation */
     return HSYM(libc_heap_alloc)(self,new_bytes,alloc_flags);
 if (__builtin_add_overflow(new_bytes,LOCAL_HEAP_ALIGNMENT_MASK,&new_bytes))
     libc_heap_allocation_failed(new_bytes-LOCAL_HEAP_ALIGNMENT_MASK);
 new_bytes &= LOCAL_HEAP_ALIGNMENT_IMASK;
 result.hp_ptr = old_ptr;
 result.hp_siz = old_bytes;
 if (new_bytes <= old_bytes) {
  size_t free_bytes;
  if unlikely(new_bytes < LOCAL_HEAP_MINSIZE)
              new_bytes = LOCAL_HEAP_MINSIZE;
  /* Free trailing memory. */
  free_bytes = old_bytes - new_bytes;
  if (free_bytes >= LOCAL_HEAP_MINSIZE) {
   HSYM(libc_heap_free)(self,(void *)((uintptr_t)old_ptr+new_bytes),
                        free_bytes,free_flags);
   result.hp_siz = new_bytes;
  }
  return result;
 }
 missing_bytes = new_bytes - old_bytes;
 missing_bytes = HSYM(libc_heap_allat)(self,(void *)((uintptr_t)old_ptr+old_bytes),
                                       missing_bytes,alloc_flags);
 if (missing_bytes) {
  /* Managed to extend the data block. */
  result.hp_siz += missing_bytes;
  return result;
 }
 /* Must allocate an entirely new data block and copy memory to it. */
 result = HSYM(libc_heap_alloc)(self,new_bytes,alloc_flags);
 LIBC_TRY {
  /* The try block is here because of the possibility of a LOA failure. */
  libc_memcpy(result.hp_ptr,old_ptr,old_bytes);
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  HSYM(libc_heap_free)(self,result.hp_ptr,result.hp_siz,
                       alloc_flags & ~GFP_CALLOC);
  error_rethrow();
 }
 /* Free the old data block. */
 HSYM(libc_heap_free)(self,old_ptr,old_bytes,
                      free_flags & ~GFP_CALLOC);
 return result;
}
INTERN struct heapptr LIBCCALL
HSYM(libc_heap_realign)(STRUCT_HEAP *__restrict self,
                        void *old_ptr, size_t old_bytes,
                        size_t min_alignment, ptrdiff_t offset,
                        size_t new_bytes, gfp_t alloc_flags,
                        gfp_t free_flags) {
 struct heapptr result; size_t missing_bytes;
 assert(IS_HEAP_ALIGNED(old_bytes));
 assert(!old_bytes || IS_HEAP_ALIGNED((uintptr_t)old_ptr));
 assert(!old_bytes || old_bytes >= LOCAL_HEAP_ALIGNMENT);
 if (old_bytes == 0) /* Special case: initial allocation */
     return HSYM(libc_heap_align)(self,min_alignment,offset,new_bytes,alloc_flags);
 if (__builtin_add_overflow(new_bytes,LOCAL_HEAP_ALIGNMENT_MASK,&new_bytes))
     libc_heap_allocation_failed(new_bytes-LOCAL_HEAP_ALIGNMENT_MASK);
 new_bytes &= LOCAL_HEAP_ALIGNMENT_IMASK;
 result.hp_ptr = old_ptr;
 result.hp_siz = old_bytes;
 if (new_bytes <= old_bytes) {
  size_t free_bytes;
  if unlikely(new_bytes < LOCAL_HEAP_MINSIZE)
              new_bytes = LOCAL_HEAP_MINSIZE;
  /* Free trailing memory. */
  free_bytes = old_bytes - new_bytes;
  if (free_bytes >= LOCAL_HEAP_MINSIZE) {
   HSYM(libc_heap_free)(self,(void *)((uintptr_t)old_ptr+new_bytes),
                        free_bytes,free_flags);
   result.hp_siz = new_bytes;
  }
  return result;
 }
 missing_bytes = new_bytes - old_bytes;
 missing_bytes = HSYM(libc_heap_allat)(self,(void *)((uintptr_t)old_ptr+old_bytes),
                                       missing_bytes,alloc_flags);
 if (missing_bytes) {
  /* Managed to extend the data block. */
  result.hp_siz += missing_bytes;
  return result;
 }
 /* Must allocate an entirely new data block and copy memory to it. */
 result = HSYM(libc_heap_align)(self,min_alignment,offset,
                                new_bytes,alloc_flags);
 LIBC_TRY {
  /* The try block is here because of the possibility of a LOA failure. */
  libc_memcpy(result.hp_ptr,old_ptr,old_bytes);
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  HSYM(libc_heap_free)(self,result.hp_ptr,result.hp_siz,
                       alloc_flags & ~GFP_CALLOC);
  error_rethrow();
 }
 /* Free the old data block. */
 HSYM(libc_heap_free)(self,old_ptr,old_bytes,
                             free_flags & ~GFP_CALLOC);
 return result;
}
#endif


INTERN ATTR_RARETEXT struct heapinfo LIBCCALL
HSYM(libc_heap_info)(STRUCT_HEAP *__restrict self) {
 struct heapinfo result;
 unsigned int bucket;
 STRUCT_MFREE *iter;
 result.hi_trimable  = 0;
 result.hi_free      = 0;
 result.hi_free_z    = 0;
 result.hi_free_min  = (__size_t)-1;
 result.hi_free_max  = 0;
 result.hi_free_cnt  = 0;
 result.hi_alloc     = self->h_stat.hs_alloc;
 result.hi_mmap      = self->h_stat.hs_mmap;
 result.hi_mmap_peak = self->h_stat.hs_mmap_peak;
 COMPILER_READ_BARRIER();
 /* Due to race conditions, other code may not have updated the
  * the peak field yet. Do it in our own stat copy manually. */
 if unlikely(result.hi_mmap_peak < result.hi_mmap)
             result.hi_mmap_peak = result.hi_mmap;
 atomic_rwlock_read(&self->h_lock);
 for (bucket = 0;
      bucket < COMPILER_LENOF(self->h_size); ++bucket) {
  iter = self->h_size[bucket];
  for (; iter; iter = iter->mf_lsize.le_next) {
   uintptr_t free_min,free_end;
   uintptr_t head_keep,tail_keep;
   result.hi_free += iter->mf_size;
   if (iter->mf_flags & MFREE_FZERO)
       result.hi_free_z += iter->mf_size;
   if (result.hi_free_min > iter->mf_size)
       result.hi_free_min = iter->mf_size;
   if (result.hi_free_max < iter->mf_size)
       result.hi_free_max = iter->mf_size;
   ++result.hi_free_cnt;
   /* Figure out how much we can actually return to the core. */
   free_min = MFREE_BEGIN(iter);
   free_end = MFREE_END(iter);
   free_min = CEIL_ALIGN(free_min,PAGESIZE);
   free_end = FLOOR_ALIGN(free_end,PAGESIZE);
   if unlikely(free_min >= free_end)
      continue; /* Even though the range is large enough, due to alignment it doesn't span a whole page */
   /* Figure out how much memory must be kept in the
    * head and tail portions of the free data block. */
   head_keep = free_min-MFREE_BEGIN(iter);
   tail_keep = MFREE_END(iter)-free_end;
   /* Make sure that data blocks that cannot be freed are still
    * large enough to remain representable as their own blocks. */
   if (head_keep && head_keep < HEAP_MINSIZE) continue;
   if (tail_keep && tail_keep < HEAP_MINSIZE) continue;
   /* This block can be trimmed. */
   result.hi_trimable += free_end-free_min;
  }
 }
 atomic_rwlock_endread(&self->h_lock);
 if (result.hi_free_min > result.hi_free_max)
     result.hi_free_min = result.hi_free_max = 0;
 return result;
}



EXPORT(HSYM(heap_info),HSYM(libc_heap_info));
#ifdef OPTION_DEBUG_HEAP
EXPORT(HSYM(heap_alloc),HSYM(libc_heap_alloc));
EXPORT(HSYM(heap_align),HSYM(libc_heap_align));
EXPORT(HSYM(heap_allat),HSYM(libc_heap_allat));
EXPORT(HSYM(heap_free),HSYM(libc_heap_free));
EXPORT(HSYM(heap_realloc),HSYM(libc_heap_realloc));
EXPORT(HSYM(heap_realign),HSYM(libc_heap_realign));
EXPORT(HSYM(heap_trim),HSYM(libc_heap_trim));
EXPORT(HSYM(heap_alloc_untraced),HSYM(libc_heap_alloc_untraced));
EXPORT(HSYM(heap_align_untraced),HSYM(libc_heap_align_untraced));
EXPORT(HSYM(heap_allat_untraced),HSYM(libc_heap_allat_untraced));
EXPORT(HSYM(heap_free_untraced),HSYM(libc_heap_free_untraced));
EXPORT(HSYM(heap_realloc_untraced),HSYM(libc_heap_realloc_untraced));
EXPORT(HSYM(heap_realign_untraced),HSYM(libc_heap_realign_untraced));
#else
EXPORT(HSYM(heap_alloc),HSYM(libc_heap_alloc_untraced));
EXPORT(HSYM(heap_align),HSYM(libc_heap_align_untraced));
EXPORT(HSYM(heap_allat),HSYM(libc_heap_allat_untraced));
EXPORT(HSYM(heap_free),HSYM(libc_heap_free_untraced));
EXPORT(HSYM(heap_realloc),HSYM(libc_heap_realloc_untraced));
EXPORT(HSYM(heap_realign),HSYM(libc_heap_realign_untraced));
EXPORT(HSYM(heap_trim),HSYM(libc_heap_trim));
#endif

DECL_END

#undef LOCAL_SIZEOF_MFREE
#undef LOCAL_HEAP_MINSIZE
#undef LOCAL_HEAP_ALIGNMENT
#undef LOCAL_HEAP_ALIGNMENT_MASK
#undef LOCAL_HEAP_ALIGNMENT_IMASK
#undef LOCAL_HEAP_BUCKET_OFFSET
#undef LOCAL_HEAP_BUCKET_OF
#undef LOCAL_HEAP_BUCKET_COUNT

#undef LIBC_HEAP_VALIDATE_ALL
#undef STRUCT_HEAP
#undef LOCAL_HSYM
#undef HSYM
#undef LOCAL_HEAP_TYPE_FCURRENT
#undef OPTION_DEBUG_HEAP
#undef OPTION_CUSTOM_ALIGNMENT
