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
#ifndef GUARD_KERNEL_SRC_VM_LOADCORE_C
#define GUARD_KERNEL_SRC_VM_LOADCORE_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/align.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <kernel/debug.h>
#include <kernel/vm.h>
#include <kernel/malloc.h>
#include <kernel/heap.h>
#include <kernel/interrupt.h>
#include <fs/node.h>
#include <string.h>
#include <stdlib.h>
#include <except.h>
#include <assert.h>

DECL_BEGIN

#define VM_LOADNODE_UNCHANGED 0 /* Nothing changed */
#define VM_LOADNODE_CHANGED   1 /* The node has been loaded */
#define VM_LOADNODE_REMAPPED  2 /* The node has been loaded, and the VM layout may have changed */


INTDEF ATTR_RETNONNULL struct vm_part *KCALL
vm_part_splitafter(struct vm_part *__restrict part,
                   vm_raddr_t part_offset);
INTDEF ATTR_NOTHROW ATTR_RETNONNULL struct vm_part *KCALL
vm_region_mergenext(struct vm_region *__restrict region,
                    struct vm_part *__restrict part);

/* Allocate+initialize memory from the given region range. */
LOCAL bool KCALL
vm_region_load_core(struct vm_node *__restrict node,
                    struct vm_region *__restrict region,
                    vm_vpage_t region_base_page, /* Region start page index. */
                    vm_raddr_t starting_page,    /* Page positions in the region, where loading starts */
                    size_t num_pages,            /* Number of pages to load. */
                    unsigned int mode) {
 struct vm_part **ppart;
 struct vm_part *EXCEPT_VAR part;
 bool result = false;
 assert(starting_page+num_pages >  starting_page);
 assert(starting_page+num_pages <= region->vr_size);
 for (ppart = &region->vr_parts;
     (part  = *ppart) != NULL;
      ppart = &part->vp_chain.le_next) {
  u16 part_prot; vm_vpage_t EXCEPT_VAR part_page;
  vm_raddr_t part_end_page; size_t EXCEPT_VAR load_pages;
  assert(part->vp_chain.le_next != part);
  /* Skip parts with an unknown state, or that are already in-core. */
  if (part->vp_state == VM_PART_INCORE ||
      part->vp_state == VM_PART_UNKNOWN)
      continue;
  if (part->vp_state == VM_PART_MISSING &&
     (mode & VM_LOADCORE_NOALOA))
      continue; /* Load perform LOA. */
  /* Skip unlocked parts when we're not supposed to load them! */
  if ((mode&VM_LOADCORE_LOCKED) &&
       part->vp_locked <= 0)
       continue;
  assertf(region->vr_type != VM_REGION_PHYSICAL,
          "Physical regions must always be in-core");
  if (part->vp_start >= starting_page+num_pages) break; /* End of requested range. */
  part_end_page = part->vp_chain.le_next ? part->vp_chain.le_next->vp_start
                                         : region->vr_size;
  assertf(part_end_page > part->vp_start,
          "part_end_page          = %p\n"
          "part->vp_start         = %p\n"
          "part->vp_chain.le_next = %p\n"
          ,
          part_end_page,part->vp_start,
          part->vp_chain.le_next);
  if (part_end_page <= starting_page) continue; /* Part below requested range. */
  /* At least some portion of this part must be loaded in some way. */
  assert(starting_page >= part->vp_start);

#if 1
  /* Split the region parts to limit the load
   * operation to what is actually requested. */
#if 1
  if (starting_page != part->vp_start)
      part = vm_part_splitafter(part,starting_page-part->vp_start);
  assert(part_end_page == (part->vp_chain.le_next ? part->vp_chain.le_next->vp_start
                                                  : region->vr_size));
  assert(starting_page == part->vp_start);
#endif
#if 1
  if (starting_page+num_pages < part_end_page) {
   part_end_page = starting_page+num_pages;
   assert(part_end_page < (part->vp_chain.le_next ? part->vp_chain.le_next->vp_start
                                                  : region->vr_size));
   vm_part_splitafter(part,part_end_page-part->vp_start);
  }
#endif
#endif

  if (part->vp_state == VM_PART_INSWAP) {
   /* TODO: Load swap memory. */
   continue;
  }

  /* Allocate physical memory for the current part (`part')
   * XXX: Use scatter for this? */
  assert(part_end_page > part->vp_start);
  load_pages = part_end_page - part->vp_start;
  part->vp_phys.py_num_scatter = 1;
  part->vp_phys.py_iscatter[0].ps_size = load_pages;
  part->vp_phys.py_iscatter[0].ps_addr = page_malloc(load_pages,MZONE_ANY);
  /* With the part now allocated, mark it as in-core. */
  part->vp_state = VM_PART_INCORE;
  TRY {
   part_prot = PAGEDIR_MAP_FUSER;
#if PROT_EXEC == PAGEDIR_MAP_FEXEC && \
    PROT_READ == PAGEDIR_MAP_FREAD && \
    PROT_WRITE == PAGEDIR_MAP_FWRITE
   part_prot |= node->vn_prot & (PROT_EXEC|PROT_READ|PROT_WRITE);
#else
   if (node->vn_prot & PROT_EXEC)
       part_prot |= PAGEDIR_MAP_FEXEC;
   if (node->vn_prot & PROT_READ)
       part_prot |= PAGEDIR_MAP_FREAD;
   if (node->vn_prot & PROT_WRITE)
       part_prot |= PAGEDIR_MAP_FWRITE;
#endif
   if (node->vn_prot & PROT_NOUSER)
       part_prot &= ~PAGEDIR_MAP_FUSER;
   if (region->vr_flags&VM_REGION_FMONITOR) {
    /* Set the changed-flag if the access mode requires write-permissions. */
    if ((mode & VM_LOADCORE_WRITE) &&
        (part->vp_refcnt <= 1 || region->vr_type == VM_REGION_PHYSICAL))
        part->vp_flags |= VM_PART_FCHANGED;
    else {
     if (!(part->vp_flags&VM_PART_FCHANGED))
           part_prot &= ~PAGEDIR_MAP_FWRITE;
    }
   }
   /* Take away write access to allow for COW semantics. */
   if ((!(node->vn_prot&PROT_SHARED) ||
         (region->vr_flags & VM_REGION_FCANTSHARE)) &&
          part->vp_refcnt > 1 &&
          region->vr_type != VM_REGION_PHYSICAL)
          part_prot &= ~PAGEDIR_MAP_FWRITE;
   if (region->vr_init == VM_REGION_INIT_FNORMAL) {
    /* Special case: Without any custom initialization, we
     *               don't need to do 2-step data mapping. */
    part_page = region_base_page + part->vp_start;
    pagedir_map(part_page,load_pages,
                part->vp_phys.py_iscatter[0].ps_addr,
                part_prot);
   } else {
    VIRT byte_t *part_vaddr; size_t part_vsize;
    /* Map the part into virtual memory.
     * If the part is used for user-space, don't enable user-access,
     * as we haven't initialized the memory and can't have the user
     * gaining access to potentially security-crucial data. */
    part_page = region_base_page + part->vp_start;
    /* TODO: When the mapping is meant to appear in user-space,
     *       the we must use `task_temppage()' to temporarily
     *       map all pages in kernel-space.
     *       If we mapped them in user-space (as we currently do),
     *       then another user-space thread would be able to watch
     *       as we initialize data and even get a chance to read
     *       data before it has been initialized.
     *       However, when the mapping goes into kernel-space, we
     *       actually _have_ to map the page directly, as use of
     *       `task_temppage()' could otherwise cause a potential
     *       infinite loop when that function causes another #PF.
     * NOTE: The lack of `PAGEDIR_MAP_FUSER' is undone by the fact
     *       that lazy page directory bindings will consider it a
     *       race condition and set the flag upon first access...
     *      (Maybe somehow prevent it from doing that for the case
     *       of user-space failing to access a PROT_NOUSER page,
     *       even when the node doesn't have PROT_NOUSER set?) */
    pagedir_map(part_page,load_pages,
                part->vp_phys.py_iscatter[0].ps_addr,
                PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE);
    TRY {
     part_vaddr = (VIRT byte_t *)VM_PAGE2ADDR(part_page);
     part_vsize = load_pages * PAGESIZE;

     /* Initialize the page. */
     switch (region->vr_init) {

     case VM_REGION_INIT_FFILLER:
      memsetl(part_vaddr,region->vr_setup.s_filler,part_vsize / 4);
      break;

     {
      u32 *dst;
      size_t count;
     case VM_REGION_INIT_FRANDOM:
      /* Fill with pseudo-random data. */
      dst   = (u32 *)part_vaddr;
      count = part_vsize / 4;
      do *dst++ = rand();
      while (--count);
     } break;

     {
      uintptr_t part_start;
      size_t init_size;
     case VM_REGION_INIT_FFILE:
     case VM_REGION_INIT_FFILE_RO:
      part_start = part->vp_start * PAGESIZE;
      if (region->vr_setup.s_file.f_begin > part_start) {
       /* Initialize data before the file mapping. */
       init_size = region->vr_setup.s_file.f_begin-part_start;
       if (init_size > part_vsize)
           init_size = part_vsize;
       memset(part_vaddr,region->vr_setup.s_file.f_filler,init_size);
       part_vsize -= init_size;
       if (!part_vsize) break;
       part_vaddr += init_size;
       part_start += init_size;
      }
      init_size = (region->vr_setup.s_file.f_begin +
                   region->vr_setup.s_file.f_size);
      if (init_size > part_start) {
       pos_t file_pos;
       file_pos  = region->vr_setup.s_file.f_start;
       file_pos += part_start - region->vr_setup.s_file.f_begin; /* Add the offset into the load-region. */
       init_size -= part_start;
       if (init_size > part_vsize)
           init_size = part_vsize;
       /* Read file data */
       init_size = inode_kread(region->vr_setup.s_file.f_node,
                               part_vaddr,init_size,file_pos,
                               IO_RDONLY);
       assert(init_size <= part_vsize);
       part_vsize -= init_size;
       if (!part_vsize) break;
       part_vaddr += init_size;
      }
      /* Initialize remaining data past the file mapping. */
      memset(part_vaddr,region->vr_setup.s_file.f_filler,part_vsize);
     } break;

     case VM_REGION_INIT_FUSER:
      /* Invoke the custom user-callback. */
      (*region->vr_setup.s_user.u_func)(region->vr_setup.s_user.u_closure,
                                        VM_REGION_USERCOMMAND_LOAD,
                                        region->vr_setup.s_user.u_delta + part->vp_start,
                                        part_vaddr,part_vsize);
      break;

     default:
      break;
     }
     /* Now that it's been initialized, map the memory for real */
     if (part_prot != (PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE)) {
      pagedir_map(part_page,load_pages,
                  part->vp_phys.py_iscatter[0].ps_addr,
                  part_prot);
      pagedir_sync(part_page,load_pages);
     }
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     /* Make sure to unmap memory once again if something went wrong.
      * Otherwise, we might end up with dangling memory cluttering the
      * VM and breaking consistency. */
     pagedir_map(part_page,load_pages,0,
                 PAGEDIR_MAP_FUNMAP);
     pagedir_sync(part_page,load_pages);
     error_rethrow();
    }
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* Since something went wrong, deallocate the physical memory
    * that we previously allocated for the part, and mark the part
    * as missing. */
   size_t scatter = part->vp_phys.py_num_scatter;
   assert(part->vp_state == VM_PART_INCORE);
   /* Free scattered physical memory. */
   while (scatter--) {
    page_free(part->vp_phys.py_iscatter[scatter].ps_addr,
              part->vp_phys.py_iscatter[scatter].ps_size);
   }
   /* Mark the state as missing. */
   part->vp_state = VM_PART_MISSING;
   COMPILER_BARRIER();
   error_rethrow();
  }

  /* Try to re-merge the VM parts that we split before. */
  if (part != region->vr_parts) {
   struct vm_part *old_prev,*new_prev;
   old_prev = COMPILER_CONTAINER_OF(part->vp_chain.le_pself,
                                    struct vm_part,vp_chain.le_next);
   new_prev = vm_region_mergenext(region,old_prev);
   if (new_prev != old_prev) part = new_prev;
  }
  part = vm_region_mergenext(region,part);
  /* Indicate that we managed to load something */
  result = true;
 }
 return result;
}


LOCAL int KCALL
vm_region_copy_core(struct vm *__restrict effective_vm,
                    struct vm_node *__restrict node,
                    struct vm_region *__restrict region,
                    vm_vpage_t region_base_page, /* Region start page index. */
                    vm_raddr_t starting_page,    /* Page positions in the region, where loading starts */
                    size_t num_pages) {          /* Number of pages to load. */
 struct vm_part **ppart,*part;
 int result = VM_LOADNODE_UNCHANGED;
 assert(region->vr_refcnt != 0);
 assert(starting_page+num_pages >  starting_page);
 assert(starting_page+num_pages <= region->vr_size);
 if (!(region->vr_flags&VM_REGION_FMONITOR)) {
  /* If the region is physical, and not being monitored, then COW isn't be enabled. */
  if (region->vr_type == VM_REGION_PHYSICAL)
      return false;
  /* COW is not active for a shared mapping of an unmonitored region. */
  if ((node->vn_prot & PROT_SHARED) &&
     !(region->vr_flags & VM_REGION_FCANTSHARE))
      return false;
  /* If the region is only mapped in a single place,
   * no need to setup up reference counting either. */
  if (region->vr_refcnt == 1) {
   assert(region->vr_parts->vp_refcnt <= 1);
   return false;
  }
 }

 for (ppart = &region->vr_parts;
     (part  = *ppart) != NULL;
      ppart = &part->vp_chain.le_next) {
  u16 part_prot; vm_raddr_t part_end_page;
  size_t EXCEPT_VAR part_size;
  size_t chunk_start,i;
  assert(part->vp_chain.le_next != part);
  /* Skip parts that have not been loaded into the core. */
  if (part->vp_state != VM_PART_INCORE)
      continue;
  if (part->vp_start >= starting_page+num_pages) break; /* End of requested range. */
  part_end_page = part->vp_chain.le_next ? part->vp_chain.le_next->vp_start
                                         : region->vr_size;
  assert(part_end_page > part->vp_start);
  if (part_end_page <= starting_page) continue; /* Part below requested range. */
  /* At least some portion of this part must be loaded in some way. */
  assert(starting_page >= part->vp_start);
#if 0
  if (starting_page != part->vp_start) { /* XXX: Split the region part as `starting_page-part->vp_start' */ }
  if (starting_page+num_pages < part_end_page) { /* XXX: Split the region part? */ }
#endif


  /* Determine the new protection flags. */
  part_prot = PAGEDIR_MAP_FUSER;
#if PROT_EXEC == PAGEDIR_MAP_FEXEC && \
    PROT_READ == PAGEDIR_MAP_FREAD && \
    PROT_WRITE == PAGEDIR_MAP_FWRITE
  part_prot |= node->vn_prot & (PROT_EXEC|PROT_READ|PROT_WRITE);
#else
  if (node->vn_prot & PROT_EXEC)
      part_prot |= PAGEDIR_MAP_FEXEC;
  if (node->vn_prot & PROT_READ)
      part_prot |= PAGEDIR_MAP_FREAD;
  if (node->vn_prot & PROT_WRITE)
      part_prot |= PAGEDIR_MAP_FWRITE;
#endif
  if (node->vn_prot & PROT_NOUSER)
      part_prot &= ~PAGEDIR_MAP_FUSER;
  /* Handle setting the changed-flag in physical, or shared regions. */
  if ((region->vr_type == VM_REGION_PHYSICAL) ||
      ((node->vn_prot&PROT_SHARED) &&
      !(region->vr_flags & VM_REGION_FCANTSHARE))) {
   assert(region->vr_flags&VM_REGION_FMONITOR);
   if (!(part->vp_flags&VM_PART_FCHANGED)) {
    part->vp_flags |= VM_PART_FCHANGED;
    result = VM_LOADNODE_CHANGED;
    /* Re-map the part, now that the changed flag has been set. */
    goto part_remap;
   }
   continue;
  }
  /* No need for COW in parts not being shared. */
  if (part->vp_refcnt <= 1) {
   /* Must still set the changed-flag when the region is being monitored. */
   if (region->vr_flags&VM_REGION_FMONITOR &&
       !(part->vp_flags&VM_PART_FCHANGED)) {
    part->vp_flags |= VM_PART_FCHANGED;
    result = VM_LOADNODE_CHANGED;
    goto part_remap;
   }
   continue;
  }
#define COW_DESCRIPTOR_GFP  (GFP_SHARED|GFP_LOCKED)
  /* Duplicate this part into a new region. */
  part_end_page = part->vp_chain.le_next ? part->vp_chain.le_next->vp_start
                                         : region->vr_size;
  part_size = part_end_page - part->vp_start;
  {
   /* NOTE: Since virtual memory allocated within the kernel (using `GFP_SHARED|GFP_LOCKED')
    *       cannot be used for COW (because it is allowed using physical regions), we are
    *       actually allowed to use `kmalloc()' for this part (with `GFP_SHARED|GFP_LOCKED',
    *       as well as a few other flags). */
   vm_vpage_t EXCEPT_VAR temporary_mapping;
   vm_vpage_t COMPILER_IGNORE_UNINITIALIZED(copy_part_vpage);
   struct vm_region *EXCEPT_VAR new_region;
   new_region = (struct vm_region *)kmalloc(sizeof(struct vm_region),
                                            COW_DESCRIPTOR_GFP|GFP_CALLOC);
   new_region->vr_refcnt = 1;
   TRY {
    mutex_cinit(&new_region->vr_lock);
    atomic_rwptr_cinit(&new_region->vr_futex);
    copy_part_vpage = region_base_page+part->vp_start;
    new_region->vr_type                                 = VM_REGION_MEM;
    new_region->vr_size                                 = part_size;
    new_region->vr_flags                                = region->vr_flags & VM_REGION_FCOPYMASK;
    new_region->vr_parts                                = &new_region->vr_part0;
    new_region->vr_part0.vp_chain.le_pself              = &new_region->vr_parts;
    new_region->vr_part0.vp_phys.py_num_scatter         = 1;
    new_region->vr_part0.vp_refcnt                      = 1;
    new_region->vr_part0.vp_phys.py_iscatter[0].ps_addr = page_malloc(part_size,MZONE_ANY); /* XXX: Use scatter? */
    new_region->vr_part0.vp_phys.py_iscatter[0].ps_size = part_size;
    COMPILER_WRITE_BARRIER();
    new_region->vr_part0.vp_state                       = VM_PART_INCORE;

    /* Find a suitable location where we can temporarily map the region. */
    vm_acquire(&vm_kernel);
    TRY {
     temporary_mapping = vm_getfree(KERNEL_BASE_PAGE,part_size,1,0,VM_GETFREE_FABOVE);
     pagedir_map(temporary_mapping,part_size,
                 new_region->vr_part0.vp_phys.py_iscatter[0].ps_addr,
                 PAGEDIR_MAP_FREAD|PAGEDIR_MAP_FWRITE);
     TRY {
      /* If read-access has never been granted for
       * the node, temporarily map it as no-user. */
      if (!(node->vn_prot & PROT_READ)) {
       chunk_start = 0;
       for (i = 0; i < part->vp_phys.py_num_scatter; ++i) {
        pagedir_map(copy_part_vpage+chunk_start,
                    part->vp_phys.py_iscatter[i].ps_size,
                    part->vp_phys.py_iscatter[i].ps_addr,
                    PAGEDIR_MAP_FREAD);
        chunk_start += part->vp_phys.py_iscatter[i].ps_size;
       }
       /*pagedir_sync(copy_part_vpage,part_size);*/
      }
      /* Copy data from the old mapping. */
      memcpyl((void *)VM_PAGE2ADDR(temporary_mapping),
              (void *)VM_PAGE2ADDR(copy_part_vpage),
               part_size * PAGESIZE / 4);
     } FINALLY {
      /* Delete the temporary mapping. */
      pagedir_map(temporary_mapping,part_size,0,PAGEDIR_MAP_FUNMAP);
      /* Must flush the paging cache to ensure that the
       * cpu knows that the temporary page is gone again. */
      pagedir_sync(temporary_mapping,part_size);
     }
    } FINALLY {
     vm_release(&vm_kernel);
    }

    /* Split VM memory before and after the part that we're about to replace. */
    vm_split_before(effective_vm,copy_part_vpage);
    vm_split_before(effective_vm,copy_part_vpage+part_size);

    /* Map the copied part! */
    chunk_start = 0;
    for (i = 0; i < part->vp_phys.py_num_scatter; ++i) {
     pagedir_map(copy_part_vpage+chunk_start,
                 part->vp_phys.py_iscatter[i].ps_size,
                 part->vp_phys.py_iscatter[i].ps_addr,
                 PAGEDIR_MAP_FREAD);
     chunk_start += part->vp_phys.py_iscatter[i].ps_size;
    }

    /* Since different physical memory is now being mapped at
     * the location in question, we must ensure that everyone
     * gets the memo and uses the new TLB values.
     * NOTE: Since this is a global modification of an existing mapping,
     *      `pagedir_map()' isn't enough here, and we need a real `vm_sync()'
     *       so that other CPUs get notified as well. */
    vm_sync(copy_part_vpage,part_size);

    /* Get the node that we ant to replace.
     * Because of the 2 splits, the VM now looks like this:
     * [...][copy_part][...]
     * `copy_part' is contained within a single region, which
     * we're going to replace with our duplicated `new_region'. */
    node = vm_getnode(copy_part_vpage);
    assertf(node,"No existing node found at %p",
            VM_PAGE2ADDR(copy_part_vpage));
    assertf(node->vn_region == region,
            "The node should still be mapped to the old region");
    assertf(node->vn_start == (copy_part_vpage-region_base_page) &&
            VM_NODE_SIZE(node) == part_size,
            "The node isn't mapping the expected portion %p...%p, but instead %p...%p",
            VM_PAGE2ADDR(copy_part_vpage-region_base_page),
            VM_PAGE2ADDR((copy_part_vpage-region_base_page)+part_size)-1,
            VM_PAGE2ADDR(node->vn_start),
            VM_PAGE2ADDR(node->vn_start+VM_NODE_SIZE(node))-1);
    /* Now simply decrement the reference counter on the part.
     * NOTE: Because modifying this counter requires a lock to be held
     *       on the region itself, we can be sure that it hasn't changed
     *       since we checked it above! */
    assert(part->vp_refcnt > 1);
    --part->vp_refcnt;

    /* Update what the node is linking against. */
    node->vn_region = new_region; /* Inherit/Exchange reference. */

    /* The node is now mapping to the start of the new
     * region (which contains only the copied part) */
    node->vn_start  = 0;

    /* Drop a reference from the old region (Inherited from `node->vn_region'). */
    vm_region_decref(region);
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    /* decref() the region on error (will also delete the
     * pre-allocated physical memory, if that was already
     * allocated) */
    vm_region_decref(new_region);
    error_rethrow();
   }

   /* Map the copied node. */
   vm_map_node(node);

   /* Try to merge the duplicated region into surrounding mappings. */
   vm_merge_before(effective_vm,copy_part_vpage+part_size);
   vm_merge_before(effective_vm,copy_part_vpage);
  }
  /* Return immediately, indicating that we've remapped some memory. */
  return VM_LOADNODE_REMAPPED;
#undef COW_DESCRIPTOR_GFP
part_remap:
  chunk_start = 0;
  /* Remap the memory of this part using the new protection. */
  for (i = 0; i < part->vp_phys.py_num_scatter; ++i) {
   pagedir_map(region_base_page+part->vp_start+chunk_start,
               part->vp_phys.py_iscatter[i].ps_size,
               part->vp_phys.py_iscatter[i].ps_addr,part_prot);
   chunk_start += part->vp_phys.py_iscatter[i].ps_size;
  }
 }
 return result;
}


INTERN int KCALL
impl_vm_node_loadcore(struct vm_node *__restrict node,
                      struct vm *__restrict effective_vm,
                      vm_raddr_t region_starting_page,
                      size_t num_region_pages,
                      unsigned int mode) {
 struct vm_region *EXCEPT_VAR region;
 vm_vpage_t region_base;
 int result = VM_LOADNODE_UNCHANGED;
 /* Simple check: Ignore no-user branches in user-requests. */
 if ((mode&VM_LOADCORE_USER) &&
     (node->vn_prot&PROT_NOUSER))
      goto done;
 region = node->vn_region;
 
 /* Figure out the in-region core address. */
 assert(region_starting_page >= node->vn_start);
 assert(region_starting_page+num_region_pages <= node->vn_start+VM_NODE_SIZE(node));
 region_base = VM_NODE_BEGIN(node)-node->vn_start;
 assert(region_starting_page+num_region_pages > region_starting_page);
 assert(region_starting_page+num_region_pages <= region->vr_size);
#if 0
 debug_printf("impl_vm_node_loadcore: %p...%p in %p...%p\n"
              "region range:          %p...%p in %p...%p\n",
             ((VM_NODE_MIN(node)-node->vn_start)+region_starting_page)*PAGESIZE,
             ((VM_NODE_MIN(node)-node->vn_start)+region_starting_page+num_region_pages)*PAGESIZE-1,
               VM_NODE_MINADDR(node),
               VM_NODE_MAXADDR(node),
               region_starting_page*PAGESIZE,
              (region_starting_page+num_region_pages)*PAGESIZE-1,
               0,num_region_pages*PAGESIZE-1);
#endif
 vm_region_incref(region);
 mutex_get(&region->vr_lock);
 TRY {
  switch (region->vr_type) {

  case VM_REGION_LOGUARD:
  case VM_REGION_HIGUARD:
   if (mode&VM_LOADCORE_NOGUARD)
       goto done_unlock;
   /* TODO: Deal with guard pages */
   goto done_unlock;

#ifndef CONFIG_NO_VIO
  case VM_REGION_VIO:
   goto done_unlock; /* VIO regions operate differently. */
#endif

  default: break;
  }

  /* First off: make sure that the affected region is loaded. */
  if (vm_region_load_core(node,region,region_base,
                          region_starting_page,
                          num_region_pages,mode))
      result = VM_LOADNODE_CHANGED;

  if ((mode & VM_LOADCORE_WRITE) &&
      (node->vn_prot & PROT_WRITE)) {
   /* When requesting write-access, we must also implement copy-on-write,
    * meaning we must allocate new regions and parts for anything that is
    * being shared. */
   int temp;
   temp = vm_region_copy_core(effective_vm,
                              node,
                              region,
                              region_base,
                              region_starting_page,
                              num_region_pages);
   if (result < temp)
       result = temp;
  }
done_unlock:;
 } FINALLY {
  mutex_put(&region->vr_lock);
  vm_region_decref(region);
 }
done:
 return result;
}

DECL_END

#endif /* !GUARD_KERNEL_SRC_VM_LOADCORE_C */
