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
#ifndef GUARD_KERNEL_SRC_VM_COREBASE_C
#define GUARD_KERNEL_SRC_VM_COREBASE_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/align.h>
#include <kernel/vm.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <string.h>
#include <except.h>

DECL_BEGIN

struct vm_coreslot {
    struct vm_node      cs_node;
    struct vm_region    cs_region;
};

struct vm_corebase {
    /* The corebase system is the base allocator for
     * creating virtual memory mappings with the kernel. */
    ATOMIC_DATA size_t  cb_inuse; /* The number of nodes + regions using this corebase block.
                                   * (+2 for the reserved pair when this is the last
                                   *     block in the internal `corebase_head' chain)
                                   * This field acts as a sort-of reference counter for
                                   * used/valid core slots. */
    LIST_NODE(struct vm_corebase)
                        cb_chain; /* [lock(vm_kernel.vm_lock)] Chain of core-base segments. */
    /* core-slot node/region pairs available for custom mappings.
     * NOTE: If this corebase block is the latest, `cb_maps[0]'
     *       is reserved for the next corebase block.
     * When a core-base slot is deallocated, it will unset its
     * `VM_NODE_FCORENODE' / `VM_REGION_FCOREREGION' flag, allowing
     * us to detect which slot pairs are currently not in use. */
    struct vm_coreslot  cb_maps[(PAGESIZE-
                                (sizeof(size_t)))/
                                 sizeof(struct vm_coreslot)];
};



/* The initial corebase data block. */
INTERN ATTR_SECTION(".bss.corebase_initial") struct vm_corebase corebase_initial;

/* Make sure that there are at least 2 mapping pairs in each corebase. */
STATIC_ASSERT(COMPILER_LENOF(corebase_initial.cb_maps) >= 2);

PUBLIC struct vm_corepair KCALL vm_corealloc(void) {
 struct vm_corepair result;
 struct vm_corebase *base,*new_base;
 struct vm_coreslot *slot;
 vm_acquire(&vm_kernel);
 TRY {
  if (!corebase_initial.cb_inuse) {
   /* Set the initial core-base inuse counter. */
   corebase_initial.cb_inuse = 2;
  }
  assertf(corebase_initial.cb_inuse >= 2,
          "The reserved core-next pointers require "
          "at least 2 in-use references be given for "
          "the current core-base");
  /* Search for an unused core slot pair. */
  result.cp_node   = NULL;
  result.cp_region = NULL;
  base = &corebase_initial;
  for (;;) {
   assertf(IS_ALIGNED((uintptr_t)base,PAGEALIGN),
           "base = %p\n",base);
   slot = base->cb_maps;
   if (!base->cb_chain.le_next)
       ++slot; /* +1 to skip the initial, reserved slot. */
   for (; slot != COMPILER_ENDOF(base->cb_maps); ++slot) {
    if (!result.cp_node &&
        !(slot->cs_node.vn_flag&VM_NODE_FCORENODE)) {
     result.cp_node = &slot->cs_node;
     ++base->cb_inuse;
     if (result.cp_region) goto found_pair;
    }
    if (!result.cp_region &&
        !(slot->cs_region.vr_flags&VM_REGION_FCOREREGION)) {
     result.cp_region = &slot->cs_region;
     ++base->cb_inuse;
     if (result.cp_node) goto found_pair;
    }
   }
   if (!base->cb_chain.le_next) break;
   base = base->cb_chain.le_next;
  }

  /* Must allocate a new slot pair.
   * As region and node components, we can use `cb_maps[0]' of `base'
   * chain, which is the one that was reserved for this exact purpose. */
#define NODE   base->cb_maps[0].cs_node
#define REGION base->cb_maps[0].cs_region
  NODE.vn_start    = 0;
  NODE.vn_region   = &REGION;
  NODE.vn_notify   = NULL;
  NODE.vn_closure  = NULL;
  NODE.vn_prot     = PROT_READ|PROT_WRITE|PROT_NOUSER;
  NODE.vn_flag     = VM_NODE_FCORENODE;
  REGION.vr_refcnt = 1;
  mutex_init(&REGION.vr_lock);
  atomic_rwptr_init(&REGION.vr_futex);
  /* Initialize the region as physical to keep the memory locked in-core.
   * The control structures for `vm_kernel' must not be
   * subject to swap and must not cause pagefaults due to LOA.
   * If they did, we'd have a problem because then the data structures
   * that would be required to bring stuff back into the core, would
   * themself no longer be part of the core (meaning we'd have locked
   * ourselves in our own prison, then lost the key...)
   * But that's easily subverted by ensuring `VM_REGION_PHYSICAL' here! */
  REGION.vr_type = VM_REGION_PHYSICAL;
  /* No special initialization. */
  REGION.vr_init  = VM_REGION_INIT_FNORMAL;
  /* Important: Set the COREREGION flag */
  REGION.vr_flags = VM_REGION_FCOREREGION;
  REGION.vr_funds = 0;
  REGION.vr_size  = 1;

  /* Setup the initial part. */
  REGION.vr_parts                   = &REGION.vr_part0;
  REGION.vr_part0.vp_chain.le_pself = &REGION.vr_parts;
  REGION.vr_part0.vp_chain.le_next  = NULL;
  REGION.vr_part0.vp_refcnt         = 1; /* The reference held by the associated node. */
  REGION.vr_part0.vp_start          = 0;
  REGION.vr_part0.vp_state          = VM_PART_INCORE;
  REGION.vr_part0.vp_flags          = VM_PART_INCORE;
  REGION.vr_part0.vp_locked         = 0;

  /* Allocate another page to which this region will point.
   * NOTE: This `page_malloc()' is the reason why this function
   *       can throw an `E_BADALLOC' error, as well as the purpose
   *       of the TRY-FINALLY block currently surrounding us. */
  REGION.vr_part0.vp_phys.py_num_scatter         = 1;
  REGION.vr_part0.vp_phys.py_iscatter[0].ps_addr = page_malloc(1,MZONE_ANY);
  REGION.vr_part0.vp_phys.py_iscatter[0].ps_size = 1;
  COMPILER_WRITE_BARRIER();

  TRY {

   /* Figure out a suitable spot where to map the next corebase. */
   NODE.vn_node.a_vmin = vm_getfree(VM_COREBASE_HINT,1,1,0,VM_COREBASE_MODE);
   NODE.vn_node.a_vmax = NODE.vn_node.a_vmin;

   /* With the NODE and REGION now set up, insert them into the kernel
    * VM and active them (update their page-directory mappings). */
   vm_insert_and_activate_node(&vm_kernel,&NODE);

   /* Synchronize that the new node has now been mapped. */
   vm_sync(NODE.vn_node.a_vmin,1);

  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   /* Free the page from above upon error. */
   page_free(REGION.vr_part0.vp_phys.py_iscatter[0].ps_addr,1);
   /* Rethrow the error. */
   error_rethrow();
  }

  /* The new core-base has now been mapped! */
  new_base = (struct vm_corebase *)VM_PAGE2ADDR(NODE.vn_node.a_vmin);

  /* Link the new base to the end of the previous. */
  base->cb_chain.le_next      = new_base;
  new_base->cb_chain.le_pself = &base->cb_chain.le_next;
  new_base->cb_chain.le_next  = NULL;

  /* Initialize the new base. */
  new_base->cb_inuse = 2;
  for (slot  = new_base->cb_maps+1;
       slot != COMPILER_ENDOF(new_base->cb_maps); ++slot) {
   assert(slot < COMPILER_ENDOF(new_base->cb_maps));
   /* Clear the flags of all newly allocated nodes and regions,
    * thus marking them as available for other uses. */
   slot->cs_node.vn_flag    = 0;
   slot->cs_region.vr_flags = 0;
  }

  /* Use the second slot of the new base for the result. */
  if (!result.cp_node)
       result.cp_node = &new_base->cb_maps[1].cs_node,
       ++new_base->cb_inuse;
  if (!result.cp_region)
       result.cp_region = &new_base->cb_maps[1].cs_region,
       ++new_base->cb_inuse;
#undef REGION
#undef NODE
found_pair:
  /* ZERO-initialize the result slot. */
  memset(result.cp_node,0,sizeof(struct vm_node));
  memset(result.cp_region,0,sizeof(struct vm_region));
  /* Set the core-flags for both components. */
  result.cp_node->vn_flag    |= VM_NODE_FCORENODE;
  result.cp_region->vr_flags |= VM_REGION_FCOREREGION;
 } FINALLY {
  vm_release(&vm_kernel);
 }
 return result;
}


#define COREBASE_FROMPART(x) \
   ((struct vm_corebase *)FLOOR_ALIGN((uintptr_t)(x),PAGEALIGN))
PRIVATE void KCALL
corebase_decref(struct vm_corebase *__restrict self) {
 assert(IS_ALIGNED((uintptr_t)self,PAGEALIGN));
 assert(self->cb_inuse != 0);
 if (!ATOMIC_DECFETCH(self->cb_inuse) && self != &corebase_initial) {
  /* Delete this corebase. */
  LIST_REMOVE(self,cb_chain);
  /* Unmap this corebase, seeing as how it's no longer being used. */
  vm_unmap(VM_ADDR2PAGE((uintptr_t)self),1,
           VM_UNMAP_NOEXCEPT,NULL);
 }
}

INTERN void KCALL
vm_region_free(struct vm_region *__restrict self) {
 assertf(!(self->vr_flags & VM_REGION_FIMMUTABLE),
           "Immutable regions is being destroyed");
 if (self->vr_flags&VM_REGION_FCOREREGION) {
  /* Special handling for destroying a core regions. */
  task_nothrow_serve();
  vm_acquire(&vm_kernel);
  self->vr_flags &= ~(VM_REGION_FCOREREGION);
  corebase_decref(COREBASE_FROMPART(self));
  vm_release(&vm_kernel);
  task_nothrow_end();
 } else {
  kfree(self);
 }
}
PUBLIC void KCALL
vm_node_free(struct vm_node *__restrict self) {
 assertf(!(self->vn_flag & VM_NODE_FIMMUTABLE),
           "Immutable node is being destroyed");
 if (self->vn_flag&VM_NODE_FCORENODE) {
  /* Special handling for destroying a core nodes. */
  task_nothrow_serve();
  vm_acquire(&vm_kernel);
  self->vn_flag &= ~(VM_NODE_FCORENODE);
  corebase_decref(COREBASE_FROMPART(self));
  vm_release(&vm_kernel);
  task_nothrow_end();
 } else {
  kfree(self);
 }
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_VM_COREBASE_C */
