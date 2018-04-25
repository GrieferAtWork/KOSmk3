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
#ifndef GUARD_KERNEL_SRC_VM_REGION_C
#define GUARD_KERNEL_SRC_VM_REGION_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/heap.h>
#include <kernel/vm.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <stdint.h>
#include <fs/node.h>
#include <string.h>
#include <except.h>
#include <assert.h>

DECL_BEGIN

INTDEF void KCALL vm_region_free(struct vm_region *__restrict self);

PUBLIC void KCALL
vm_region_destroy(struct vm_region *__restrict self) {
 struct vm_part *part,*next;
 /* Cleanup/free/untrack the region and its components.
  * NOTE: We can (and should) assert that all parts have a reference counter of ZERO(0)
  *       However, this can only be asserted when the `VM_REGION_FLEAKINGPARTS' flag
  *       hasn't been set (as is set when `vm_region_decref_range()' fails to split a part) */
 part = self->vr_parts;
 while (part) {
  size_t i;
#if 0 /* Partially allocated regions don't follow this rule... */
  assertf(part->vp_refcnt == 0 ||
         (self->vr_flags&VM_REGION_FLEAKINGPARTS),
          "Dangling region part wasn't properly decref()'d");
#endif

  next = part->vp_chain.le_next;
  switch (part->vp_state) {

  case VM_PART_INCORE:
   if (part->vp_flags & VM_PART_FCHANGED) {
    /* XXX: Save modified file data / invoke `VM_REGION_USERCOMMAND_SAVE'? */
   }
   if (part->vp_flags & VM_PART_FWEAKREF)
       break;
   /* Free allocated physical memory. */
   for (i = 0; i < part->vp_phys.py_num_scatter; ++i)
        page_free(part->vp_phys.py_iscatter[i].ps_addr,
                  part->vp_phys.py_iscatter[i].ps_size);
   break;

  default: break;
  }
  if (part != &self->vr_part0)
      kfree(part);
  part = next;
 }
 /* Finalize setup information. */
 switch (self->vr_init) {

 case VM_REGION_INIT_FFILE:
 case VM_REGION_INIT_FFILE_RO:
  if (self->vr_setup.s_file.f_node)
      inode_decref(self->vr_setup.s_file.f_node);
  break;

 case VM_REGION_INIT_FUSER:
  /* Invoke the custom user-callback with a DECREF() command. */
  if (self->vr_setup.s_user.u_func)
     (*self->vr_setup.s_user.u_func)(self->vr_setup.s_user.u_closure,
                                     VM_REGION_USERCOMMAND_DECREF,
                                     self->vr_setup.s_user.u_delta,
                                     NULL,self->vr_size*PAGESIZE);
  break;

#ifndef CONFIG_NO_VIO
 case VM_REGION_INIT_FVIO:
  /* Invoke the special finalization VIO operator (if it was defined). */
  if (self->vr_setup.s_vio.v_ops->v_fini)
    (*self->vr_setup.s_vio.v_ops->v_fini)(self->vr_setup.s_vio.v_closure);
  break;
#endif

 default: break;
 }

 vm_region_free(self);
}

/* Allocate a new, default-initialized VM-region descriptor.
 * The reference counter is set to ONE(1)
 * The returned region is not being tracked.
 * @throw E_BADALLOC: Failed to allocate sufficient memory. */
PUBLIC ATTR_RETNONNULL ATTR_MALLOC
REF struct vm_region *KCALL vm_region_alloc(size_t num_pages) {
 REF struct vm_region *result;
 result = (struct vm_region *)kmalloc(sizeof(struct vm_region),
                                      GFP_SHARED|GFP_LOCKED|GFP_CALLOC);
 result->vr_refcnt = 1;
 mutex_cinit(&result->vr_lock);
 atomic_rwptr_cinit(&result->vr_futex);
 result->vr_size                    = num_pages;
 result->vr_parts                   = &result->vr_part0;
 result->vr_part0.vp_chain.le_pself = &result->vr_parts;
 return result;
}

PUBLIC ATTR_RETNONNULL ATTR_MALLOC
REF struct vm_region *KCALL
vm_region_alloc_file(size_t num_pages,
                     bool enable_writeback,
                     struct inode *__restrict node,
                     uintptr_t begin,
                     pos_t start,
                     size_t size,
                     u32 filler) {
 REF struct vm_region *result;
 result = vm_region_alloc(num_pages);
 result->vr_init = VM_REGION_INIT_FFILE_RO;
 if (enable_writeback)
     result->vr_init = VM_REGION_INIT_FFILE;
 /* Fill in setup data. */
 inode_incref(node);
 result->vr_setup.s_file.f_node   = node;
 result->vr_setup.s_file.f_begin  = begin;
 result->vr_setup.s_file.f_start  = start;
 result->vr_setup.s_file.f_size   = size;
 result->vr_setup.s_file.f_filler = filler;
 return result;
}


#ifndef CONFIG_NO_VIO
/* Construct a new VM VIO region using the given arguments. */
PUBLIC ATTR_RETNONNULL ATTR_MALLOC REF struct vm_region *
KCALL vm_region_alloc_vio(size_t num_pages,
                          struct vio_ops *__restrict ops,
                          void *closure) {
 REF struct vm_region *result;
 result = vm_region_alloc(num_pages);
 result->vr_type                  = VM_REGION_VIO;
 result->vr_init                  = VM_REGION_INIT_FVIO;
 result->vr_flags                |= VM_REGION_FDONTMERGE;
 result->vr_setup.s_vio.v_ops     = ops;
 result->vr_setup.s_vio.v_closure = closure;
 return result;
}
#endif



FUNDEF void KCALL
vm_region_prefault(struct vm_region *__restrict self) {
 self->vr_part0.vp_state                       = VM_PART_INCORE;
 self->vr_part0.vp_phys.py_num_scatter         = 1;
 self->vr_part0.vp_phys.py_iscatter[0].ps_addr = page_malloc(self->vr_size,MZONE_ANY);
 self->vr_part0.vp_phys.py_iscatter[0].ps_size = self->vr_size; /* TODO: Use scattered allocation. */
}


INTERN ATTR_RETNONNULL struct vm_part *KCALL
vm_part_splitafter(struct vm_part *__restrict
#ifndef NDEBUG
                   EXCEPT_VAR
#endif
                   part,
                   vm_raddr_t part_offset) {
 struct vm_part *COMPILER_IGNORE_UNINITIALIZED(new_part);
 assert(part_offset != 0);
 assertf(part->vp_start+part_offset > part->vp_start,
         "part->vp_start = %Iu\n"
         "part_offset    = %Iu\n",
         part->vp_start,part_offset);
 assert(!part->vp_chain.le_next ||
        (part->vp_start + part_offset) < part->vp_chain.le_next->vp_start);
#ifndef NDEBUG
 assertf(!(part->vp_flags & VM_PART_FSPLITTING),
         "Part %p is already being split (%x)",part,part->vp_flags);
 ATOMIC_FETCHOR(part->vp_flags,VM_PART_FSPLITTING);
 TRY {
#endif

 /* Split this part in two. */
 switch (part->vp_state) {

 {
  size_t scatter_total,scatter_split,i;
  struct vm_phys_scatter *split;
 case VM_PART_INCORE:
#if 1
  if (part->vp_phys.py_num_scatter == 1)
  /* Optimization: if there is only one entry to begin with, we
   *               always need one to represent the upper split! */
   scatter_split = 1,i = 0,scatter_total = 0;
  else
#endif
  {
   scatter_total = 0;
   for (i = 0;; ++i) {
    assertf(i < part->vp_phys.py_num_scatter,
            "part->vp_phys.py_num_scatter = %Iu\n"
            "scatter_total                = %Iu\n"
            "part_offset                  = %Iu\n",
            part->vp_phys.py_num_scatter,
            scatter_total,part_offset);
    scatter_total += part->vp_phys.py_iscatter[i].ps_size;
    if (scatter_total > part_offset) break;
   }
   /* The number of physical scatter entires required to represent the new part. */
   scatter_total -= part->vp_phys.py_iscatter[i].ps_size;
   scatter_split  = part->vp_phys.py_num_scatter-i;
  }
  /* Allocate the new part. */
  new_part = (struct vm_part *)kmalloc(offsetof(struct vm_part,vp_phys.py_iscatter)+
                                      (scatter_split*sizeof(struct vm_phys_scatter)),
                                       GFP_SHARED|GFP_LOCKED|GFP_NOOVER);
  new_part->vp_phys.py_num_scatter = scatter_split;
  /* Copy scatter information that always belongs to the high-part. */
  split = &part->vp_phys.py_iscatter[i];
  memcpy(&new_part->vp_phys.py_iscatter[1],split+1,
        (part->vp_phys.py_num_scatter-(i+1))*
         sizeof(struct vm_phys_scatter));
  /* Update the new split counter of this part. */
  part->vp_phys.py_num_scatter = i+1;

  /* Special handling to deal with the split'd part.
   * This one we must split at `part_offset - scatter_total' */
  scatter_total = part_offset - scatter_total;
  assert(split->ps_size > scatter_total);
  new_part->vp_phys.py_iscatter[0].ps_addr = split->ps_addr + scatter_total;
  new_part->vp_phys.py_iscatter[0].ps_size = split->ps_size - scatter_total;
  split->ps_size = scatter_total;

  /* Clone part information. */
  new_part->vp_refcnt = part->vp_refcnt;
  new_part->vp_state  = part->vp_state;
  new_part->vp_flags  = part->vp_flags;
  new_part->vp_locked = part->vp_locked;
#ifndef NDEBUG
  new_part->vp_flags &= ~VM_PART_FSPLITTING;
#endif
 } break;

 default:
  /* This part doesn't use any variable data. */
  new_part = (struct vm_part *)kmalloc(sizeof(struct vm_part),
                                       GFP_SHARED|GFP_LOCKED|GFP_NOOVER);
  memcpy(new_part,part,sizeof(struct vm_part));
#ifndef NDEBUG
  new_part->vp_flags &= ~VM_PART_FSPLITTING;
#endif
  break;
 }
#ifndef NDEBUG
 } FINALLY {
  ATOMIC_FETCHAND(part->vp_flags,~VM_PART_FSPLITTING);
 }
#endif

 /* Set the start offset of the new part. */
 new_part->vp_start = part->vp_start + part_offset;
 assertf(!part->vp_chain.le_next ||
          new_part->vp_start < part->vp_chain.le_next->vp_start,
          "new_part->vp_start               = %Iu\n"
          "part->vp_chain.le_next->vp_start = %Iu\n",
          new_part->vp_start,
          part->vp_chain.le_next->vp_start);

 /* Insert the new part after the old one. */
 new_part->vp_chain.le_next = part->vp_chain.le_next;
 if (new_part->vp_chain.le_next)
     new_part->vp_chain.le_next->vp_chain.le_pself = &new_part->vp_chain.le_next;
 part->vp_chain.le_next = new_part;
 new_part->vp_chain.le_pself = &part->vp_chain.le_next;
 return new_part;
}

/* Check if some part of `self' starts at the given `part_address' (`vn_start == part_address').
 * If one such part exists, return a pointer to it.
 * Otherwise, locate the part containing `part_address' and split it in 2,
 * the lower part starting where that part originally started, while the upper
 * part is setup to start at `part_address', before returning a pointer to
 * that upper part.
 * @assume(return->vn_start == part_address); */
INTERN ATTR_RETNONNULL struct vm_part *KCALL
vm_region_split_before(struct vm_region *__restrict self,
                       vm_raddr_t part_address) {
 struct vm_part *part;
 assert(part_address < self->vr_size);
 assert(self->vr_parts->vp_start == 0);
 part = self->vr_parts;
 /* Find the first part that contains `part_address' */
 while (part->vp_chain.le_next &&
        part->vp_chain.le_next->vp_start <= part_address)
        part = part->vp_chain.le_next;
 if (part->vp_start == part_address)
     return part; /* That part already exists. */
 assert(!part->vp_chain.le_next ||
         part->vp_chain.le_next->vp_start > part_address);
 assertf(part_address > part->vp_start,
         "self->vr_size  = %Iu\n"
         "part->vp_start = %Iu\n"
         "part_address   = %Iu\n",
         self->vr_size,
         part->vp_start,
         part_address);
 /* Split this part in two. */
 return vm_part_splitafter(part,part_address - part->vp_start);
}


/* Try to merge `part' with its successor, or doing so is at all possible. */
INTERN ATTR_NOTHROW ATTR_RETNONNULL struct vm_part *KCALL
vm_region_mergenext(struct vm_region *__restrict region,
                    struct vm_part *__restrict self) {
 struct vm_part *next;
 next = self->vp_chain.le_next;
 if (!next) goto nomerge;
 /* Match attributes. */
 if (next->vp_refcnt != self->vp_refcnt) goto nomerge;
 if (next->vp_locked != self->vp_locked) goto nomerge;
 if (next->vp_state  != self->vp_state) goto nomerge;
 /* The parts _can_ actually be merged! */
 switch (self->vp_state) {

 {
  size_t required_scatter;
  struct vm_part *old_next;
 case VM_PART_INCORE:
  assert(self->vp_phys.py_num_scatter != 0);
  assert(next->vp_phys.py_num_scatter != 0);
  if (self->vp_phys.py_num_scatter == 1) {
   /* When only one scatter exists in the first part, there is a good
    * chance that it can be grafted onto the front of the next part. */
   if (self->vp_phys.py_iscatter[0].ps_addr+
       self->vp_phys.py_iscatter[0].ps_size ==
       next->vp_phys.py_iscatter[0].ps_addr) {
    /* The physical memory can be merged directly. */
    assertf(self->vp_start+self->vp_phys.py_iscatter[0].ps_size == next->vp_start,
            "self->vp_start                                      = %Iu\n"
            "self->vp_phys.py_iscatter[0].ps_size                = %Iu\n"
            "self->vp_start+self->vp_phys.py_iscatter[0].ps_size = %Iu\n"
            "next->vp_start                                      = %Iu\n",
            self->vp_start,self->vp_phys.py_iscatter[0].ps_size,
            self->vp_start+self->vp_phys.py_iscatter[0].ps_size,
            next->vp_start);
    next->vp_phys.py_iscatter[0].ps_addr  = self->vp_phys.py_iscatter[0].ps_addr;
    next->vp_phys.py_iscatter[0].ps_size += self->vp_phys.py_iscatter[0].ps_size;
    goto data_updated;
   }
   required_scatter = next->vp_phys.py_num_scatter+1;
  } else {
   required_scatter = self->vp_phys.py_num_scatter;
   /* Check if the scatter vector ends could be merged. */
   if (self->vp_phys.py_iscatter[required_scatter-1].ps_addr+
       self->vp_phys.py_iscatter[required_scatter-1].ps_size ==
       next->vp_phys.py_iscatter[0].ps_addr)
       --required_scatter;
   required_scatter += next->vp_phys.py_num_scatter;
  }
  /* Re-allocate the next part to make space for new scatter entries. */
  old_next = next;
  TRY {
   assert(next != &region->vr_part0);
#if 1
   /* Try to extend the next pointer without moving it. */
   next = (struct vm_part *)krealloc(old_next,offsetof(struct vm_part,vp_phys.py_iscatter)+
                                    (required_scatter*sizeof(struct vm_phys_scatter)),
                                     GFP_SHARED|GFP_LOCKED|GFP_NOMOVE|GFP_NOOVER);
   assert(!next || next == old_next);
   if (!next)
#endif
   {
    /* Allocate a new next-pointer. */
    next = (struct vm_part *)kmalloc(offsetof(struct vm_part,vp_phys.py_iscatter)+
                                    (required_scatter*sizeof(struct vm_phys_scatter)),
                                     GFP_SHARED|GFP_LOCKED|GFP_NOOVER);
   }
  } CATCH_HANDLED (E_BADALLOC) {
   goto nomerge; /* Then we just don't merge this one... */
  }
  assert(self->vp_chain.le_next == old_next);
  if (old_next != next) {
   /* Copy data from the old next-part. */
   memcpy(next,old_next,
          offsetof(struct vm_part,vp_phys.py_iscatter)+
         (old_next->vp_phys.py_num_scatter*
          sizeof(struct vm_phys_scatter)));
   /* In case `next' got re-located, update its chain pointers. */
   self->vp_chain.le_next = next;
   assert(next->vp_chain.le_pself == &self->vp_chain.le_next);
   if (next->vp_chain.le_next)
       next->vp_chain.le_next->vp_chain.le_pself = &next->vp_chain.le_next;
   COMPILER_WRITE_BARRIER();
   kfree(old_next);
  }
  /* Shift the scatter vector of the part to make space for the new parts
   * that are going to be transferred from the previous part (`self'). */
  required_scatter -= next->vp_phys.py_num_scatter;
  assert(required_scatter != 0);
  memmove(&next->vp_phys.py_iscatter[required_scatter],
          &next->vp_phys.py_iscatter[0],
          next->vp_phys.py_num_scatter*
          sizeof(struct vm_phys_scatter));
  next->vp_phys.py_num_scatter += required_scatter;
  /* Transfer all full parts from the original set. */
  memcpy(&next->vp_phys.py_iscatter[0],
         &self->vp_phys.py_iscatter[0],
         (self->vp_phys.py_num_scatter-1)*
          sizeof(struct vm_phys_scatter));
  if (required_scatter == self->vp_phys.py_num_scatter) {
   /* Copy the last part normally. */
   next->vp_phys.py_iscatter[required_scatter-1] = self->vp_phys.py_iscatter[required_scatter-1];
  } else {
   /* The last part of `self' can be merged with the first (old) part of `next' */
   assert(required_scatter == self->vp_phys.py_num_scatter-1);
   assertf(self->vp_phys.py_iscatter[required_scatter].ps_addr+
           self->vp_phys.py_iscatter[required_scatter].ps_size ==
           next->vp_phys.py_iscatter[required_scatter].ps_addr,
           "required_scatter                                                                                        = %Iu\n"
           "next->vp_phys.py_num_scatter                                                                            = %Iu\n"
           "self->vp_phys.py_iscatter[required_scatter].ps_addr+self->vp_phys.py_iscatter[required_scatter].ps_size = %p\n"
           "next->vp_phys.py_iscatter[required_scatter].ps_addr                                                     = %p\n",
          (uintptr_t)required_scatter,next->vp_phys.py_num_scatter,
          (uintptr_t)(self->vp_phys.py_iscatter[required_scatter].ps_addr+self->vp_phys.py_iscatter[required_scatter].ps_size),
          (uintptr_t)(next->vp_phys.py_iscatter[required_scatter].ps_addr));
   next->vp_phys.py_iscatter[required_scatter].ps_addr  = self->vp_phys.py_iscatter[required_scatter].ps_addr;
   next->vp_phys.py_iscatter[required_scatter].ps_size += self->vp_phys.py_iscatter[required_scatter].ps_size;
  }
  /* All right! the physical memory scatter vectors have been merged! */
 } break;

 default:
  break;
 }
data_updated:
 /* Update the part start pointer of the next part (we'll be getting rid of `self') */
 next->vp_start = self->vp_start;

 /* Remove `self' from the chain of parts. */
 LIST_REMOVE(self,vp_chain);
 COMPILER_WRITE_BARRIER();

 assert(next->vp_chain.le_pself == &region->vr_parts ||
        COMPILER_CONTAINER_OF(next->vp_chain.le_pself,
                              struct vm_part,vp_chain.le_next)->vp_start <
        next->vp_start);

 /* Free the (now) deleted primary part. */
 if (self != &region->vr_part0)
     kfree(self);
 return next;
nomerge:
 return self;
}



PUBLIC void KCALL
vm_region_incref_range(struct vm_region *__restrict self,
                       vm_raddr_t first_page, size_t num_pages) {
 struct vm_region *EXCEPT_VAR xself = self;
 vm_raddr_t incref_end;
 incref_end = first_page+num_pages;
 assert(incref_end <= self->vr_size);
 mutex_get(&self->vr_lock);
 TRY {
  struct vm_part *EXCEPT_VAR first_part;
  struct vm_part *EXCEPT_VAR iter;
  /* Split at the lower bound. */
  first_part = vm_region_split_before(self,first_page);
  /* Enumerate all pages in the affected area. */
  for (iter = first_part;
       assert(iter),incref_end > iter->vp_start;
       iter = iter->vp_chain.le_next) {
   vm_raddr_t part_end;
   part_end = iter->vp_chain.le_next ? iter->vp_chain.le_next->vp_start
                                     : self->vr_size;
   if (incref_end < part_end) {
    /* Last part to incref must only be incref'd partially.
     * -> Must split the last part once again. */
    TRY {
     vm_part_splitafter(iter,incref_end-iter->vp_start);
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     /* Decrement the reference counters of all parts thus far (undoing what we already did). */
     struct vm_part *undo = first_part;
     for (; undo != iter; undo = undo->vp_chain.le_next) {
      assert(undo->vp_refcnt);
      --undo->vp_refcnt;
     }
     error_rethrow();
    }
    ++iter->vp_refcnt;
    break;
   }
   /* Incref this part. */
   ++iter->vp_refcnt;
   if (incref_end == part_end)
       break;
  }
  /* Try to merge the first part with its predecessor. */
  if (first_part != self->vr_parts) {
   struct vm_part *before_first,*new_first;
   before_first = COMPILER_CONTAINER_OF(first_part->vp_chain.le_pself,
                                        struct vm_part,vp_chain.le_next);
   new_first = vm_region_mergenext(self,before_first);
   if (iter == first_part && new_first != before_first) iter = new_first;
  }
  /* Try to merge the last part with its successor. */
  vm_region_mergenext(self,iter);
 } FINALLY {
  mutex_put(&xself->vr_lock);
 }
}
PUBLIC void KCALL
vm_region_decref_range(struct vm_region *__restrict self,
                       vm_raddr_t first_page, size_t num_pages) {
 struct vm_region *EXCEPT_VAR xself = self;
 vm_raddr_t decref_end;
 decref_end = first_page+num_pages;
 assert(decref_end <= self->vr_size);
 task_nothrow_serve();
 mutex_get(&self->vr_lock);
 TRY {
  struct vm_part *first_part,*iter;
  /* Split at the lower bound. */
  first_part = vm_region_split_before(self,first_page);
  /* Enumerate all pages in the affected area. */
  for (iter = first_part;
       assert(iter),decref_end > iter->vp_start;
       iter = iter->vp_chain.le_next) {
   vm_raddr_t part_end;
   assertf(iter->vp_refcnt != 0,"Reference counter is already ZERO(0)");
   part_end = iter->vp_chain.le_next ? iter->vp_chain.le_next->vp_start
                                     : self->vr_size;
   if (decref_end < part_end) {
    /* Last part to decref must only be incref'd partially.
     * -> Must split the last part once again. */
    vm_part_splitafter(iter,decref_end-iter->vp_start);
    --iter->vp_refcnt;
    break;
   }
   /* Decref this part. */
   --iter->vp_refcnt;
   if (decref_end == part_end)
       break;
  }
  /* Try to merge the first part with its predecessor. */
  if (first_part != self->vr_parts) {
   struct vm_part *before_first,*new_first;
   before_first = COMPILER_CONTAINER_OF(first_part->vp_chain.le_pself,
                                        struct vm_part,vp_chain.le_next);
   new_first = vm_region_mergenext(self,before_first);
   if (iter == first_part && new_first != before_first) iter = new_first;
  }
  /* Try to merge the last part with its successor. */
  vm_region_mergenext(self,iter);
 } FINALLY {
  if (FINALLY_WILL_RETHROW) {
   /* Indicate that parts of this region are being leaked. */
   xself->vr_flags |= VM_REGION_FLEAKINGPARTS;
   /* Don't propagate errors caused because of a bad allocation. */
   if (error_code() == E_BADALLOC)
       FINALLY_WILL_RETHROW = false;
  }
  mutex_put(&self->vr_lock);
  task_nothrow_end();
 }
}


PUBLIC void KCALL
vm_region_inclck_range(struct vm_region *__restrict self,
                       vm_raddr_t first_page, size_t num_pages) {
 struct vm_region *EXCEPT_VAR xself = self;
 vm_raddr_t inclck_end;
 inclck_end = first_page+num_pages;
 assert(inclck_end <= self->vr_size);
 mutex_get(&self->vr_lock);
 TRY {
  struct vm_part *EXCEPT_VAR first_part;
  struct vm_part *EXCEPT_VAR iter;
  /* Split at the lower bound. */
  first_part = vm_region_split_before(self,first_page);
  /* Enumerate all pages in the affected area. */
  for (iter = first_part;
       assert(iter),inclck_end > iter->vp_start;
       iter = iter->vp_chain.le_next) {
   vm_raddr_t part_end;
   part_end = iter->vp_chain.le_next ? iter->vp_chain.le_next->vp_start
                                     : self->vr_size;
   TRY {
    if (inclck_end < part_end) {
     /* Last part to lock must only be locked partially.
      * -> Must split the last part once again. */
     vm_part_splitafter(iter,inclck_end-iter->vp_start);
     if unlikely(iter->vp_locked == INT16_MAX)
        error_throw(E_OVERFLOW);
     ++iter->vp_locked;
     break;
    }
    /* Lock this part. */
    if unlikely(iter->vp_locked == INT16_MAX)
       error_throw(E_OVERFLOW);
    ++iter->vp_locked;
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    /* Decrement the lock counters of all parts thus far (undoing what we already did). */
    struct vm_part *undo = first_part;
    for (; undo != iter; undo = undo->vp_chain.le_next)
         --undo->vp_locked;
    error_rethrow();
   }
   if (inclck_end == part_end)
       break;
  }
  /* Try to merge the first part with its predecessor. */
  if (first_part != self->vr_parts) {
   struct vm_part *before_first,*new_first;
   before_first = COMPILER_CONTAINER_OF(first_part->vp_chain.le_pself,
                                        struct vm_part,vp_chain.le_next);
   new_first = vm_region_mergenext(self,before_first);
   if (iter == first_part && new_first != before_first) iter = new_first;
  }
  /* Try to merge the last part with its successor. */
  vm_region_mergenext(self,iter);
 } FINALLY {
  mutex_put(&xself->vr_lock);
 }
}
PUBLIC void KCALL
vm_region_declck_range(struct vm_region *__restrict self,
                       vm_raddr_t first_page, size_t num_pages) {
 struct vm_region *EXCEPT_VAR xself = self;
 vm_raddr_t inclck_end;
 inclck_end = first_page+num_pages;
 assert(inclck_end <= self->vr_size);
 mutex_get(&self->vr_lock);
 TRY {
  struct vm_part *EXCEPT_VAR first_part;
  struct vm_part *EXCEPT_VAR iter;
  /* Split at the lower bound. */
  first_part = vm_region_split_before(self,first_page);
  /* Enumerate all pages in the affected area. */
  for (iter = first_part;
       assert(iter),inclck_end > iter->vp_start;
       iter = iter->vp_chain.le_next) {
   vm_raddr_t part_end;
   part_end = iter->vp_chain.le_next ? iter->vp_chain.le_next->vp_start
                                     : self->vr_size;
   TRY {
    if (inclck_end < part_end) {
     /* Last part to unlock must only be unlocked partially.
      * -> Must split the last part once again. */
     vm_part_splitafter(iter,inclck_end-iter->vp_start);
     if unlikely(iter->vp_locked == INT16_MIN)
        error_throw(E_OVERFLOW);
     --iter->vp_locked;
     break;
    }
    /* Lock this part. */
    if unlikely(iter->vp_locked == INT16_MIN)
       error_throw(E_OVERFLOW);
    --iter->vp_locked;
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    /* Increment the lock counters of all parts thus far (undoing what we already did). */
    struct vm_part *undo = first_part;
    for (; undo != iter; undo = undo->vp_chain.le_next)
         ++undo->vp_locked;
    error_rethrow();
   }
   if (inclck_end == part_end)
       break;
  }
  /* Try to merge the first part with its predecessor. */
  if (first_part != self->vr_parts) {
   struct vm_part *before_first,*new_first;
   before_first = COMPILER_CONTAINER_OF(first_part->vp_chain.le_pself,
                                        struct vm_part,vp_chain.le_next);
   new_first = vm_region_mergenext(self,before_first);
   if (iter == first_part && new_first != before_first) iter = new_first;
  }
  /* Try to merge the last part with its successor. */
  vm_region_mergenext(self,iter);
 } FINALLY {
  mutex_put(&xself->vr_lock);
 }
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_VM_REGION_C */
