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
#ifndef GUARD_KERNEL_SRC_KERNEL_HEAP_C
#define GUARD_KERNEL_SRC_KERNEL_HEAP_C 1
#define __OMIT_HEAP_CONSTANT_P_WRAPPERS 1
#define _KOS_SOURCE 1
#define _NOSERVE_SOURCE 1 /* Required for noexcept free() until we get an atomic munmap() */

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/minmax.h>
#include <hybrid/align.h>
#include <kernel/malloc.h>
#include <kernel/heap.h>
#include <fs/iomode.h>
#include <except.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <kernel/debug.h>

/* Define the ABI for the address tree used by heaps. */
#define ATREE(x)            mfree_tree_##x
#define ATREE_NODE_MIN      MFREE_MIN
#define ATREE_NODE_MAX      MFREE_MAX
#define Tkey                VIRT uintptr_t
#define T                   struct mfree
#define path                mf_laddr
#include <hybrid/list/atree-abi.h>

DECL_BEGIN

STATIC_ASSERT_MSG(GFP_ATOMIC == IO_NONBLOCK,
                  "Code is allowed to assume that these flags are identical");

#if defined(NDEBUG) || 1
#define heap_validate(heap)     (void)0
#define heap_validate_all()     (void)0
#define heap_novalidate_begin() (void)0
#define heap_novalidate_end()   (void)0
#endif

#if 0
#define DEFAULT_OVERALLOC  (PAGESIZE)
#define DEFAULT_FREETHRESH (PAGESIZE*2)
#elif PAGESIZE*16 > 65536
#define DEFAULT_OVERALLOC  (PAGESIZE*16)
#define DEFAULT_FREETHRESH (PAGESIZE*32)
#else
#define DEFAULT_OVERALLOC  (PAGESIZE*32)
#define DEFAULT_FREETHRESH (PAGESIZE*64)
#endif

PUBLIC struct heap kernel_heaps[__GFP_HEAPCOUNT] = {
    /* Define the controller structures for the builtin kernel heaps. */
    [GFP_SHARED] = {
        .h_lock       = ATOMIC_RWLOCK_INIT,
        .h_overalloc  = DEFAULT_OVERALLOC,
        .h_freethresh = DEFAULT_FREETHRESH,
        .h_hintpage   = VM_KERNEL_SHAREDHEAP_HINT,
        .h_hintmode   = VM_KERNEL_SHAREDHEAP_MODE
    },
    [GFP_KERNEL] = {
        .h_lock       = ATOMIC_RWLOCK_INIT,
        .h_overalloc  = DEFAULT_OVERALLOC,
        .h_freethresh = DEFAULT_FREETHRESH,
        .h_hintpage   = VM_KERNEL_KERNELHEAP_HINT,
        .h_hintmode   = VM_KERNEL_KERNELHEAP_MODE
    },
    [GFP_SHARED | GFP_LOCKED] = {
        .h_lock       = ATOMIC_RWLOCK_INIT,
        .h_overalloc  = DEFAULT_OVERALLOC / 4,
        .h_freethresh = DEFAULT_FREETHRESH / 4,
        .h_hintpage   = VM_KERNEL_SHAREDHEAP_LOCKED_HINT,
        .h_hintmode   = VM_KERNEL_SHAREDHEAP_LOCKED_MODE
    },
    [GFP_KERNEL | GFP_LOCKED] = {
        .h_lock       = ATOMIC_RWLOCK_INIT,
        .h_overalloc  = DEFAULT_OVERALLOC / 4,
        .h_freethresh = DEFAULT_FREETHRESH / 4,
        .h_hintpage   = VM_KERNEL_KERNELHEAP_LOCKED_HINT,
        .h_hintmode   = VM_KERNEL_KERNELHEAP_LOCKED_MODE
    },
};




INTERN byte_t *KCALL
find_modified_address(byte_t *start, u32 pattern, size_t num_bytes) {
 while ((uintptr_t)start & 3) {
  if (!num_bytes) return NULL;
  if (*(u8 *)start != ((u8 *)&pattern)[(uintptr_t)start & 3])
      return start;
  --num_bytes,++start;
 }
 while (num_bytes > 4) {
  if (!((uintptr_t)start & (PAGESIZE-1))) {
   for (;;) {
    vm_vpage_t page = VM_ADDR2PAGE((uintptr_t)start);
#if 0
#elif defined(CONFIG_HAVE_PAGEDIR_CHANGED)
    /* If supported by the host, speed this up using page directory dirty bits. */
    if (pagedir_haschanged(page)) {
     pagedir_unsetchanged(page);
     break;
    }
#else
    if (pagedir_ismapped(page))
        break;
#endif
    if (num_bytes <= PAGESIZE) return NULL;
    start     += PAGESIZE;
    num_bytes -= PAGESIZE;
   }
  }
  if (*(u32 *)start == pattern) {
   start     += 4;
   num_bytes -= 4;
   continue;
  }
  if (((u8 *)start)[0] != ((u8 *)&pattern)[0]) return start + 0;
  if (((u8 *)start)[1] != ((u8 *)&pattern)[1]) return start + 1;
  if (((u8 *)start)[2] != ((u8 *)&pattern)[2]) return start + 2;
  return start + 3;
 }
 while (num_bytes) {
  if (*(u8 *)start != ((u8 *)&pattern)[(uintptr_t)start & 3])
      return start;
  --num_bytes,++start;
 }
 return NULL;
}



/* HEAP Debug functions. */
#ifdef CONFIG_DEBUG_HEAP
PRIVATE u8 KCALL
mfree_get_checksum(struct mfree *__restrict self) {
 u8 sum = 0,*iter,*end;
 end = (iter = (u8 *)&self->mf_size)+sizeof(self->mf_size);
 for (; iter != end; ++iter) sum += *iter;
 return 0 - sum;
}
PRIVATE void KCALL
mfree_set_checksum(struct mfree *__restrict self) {
 self->mf_szchk = mfree_get_checksum(self);
}
PRIVATE bool KCALL
quick_verify_mfree(struct mfree *__restrict self) {
 TRY {
  if (!IS_ALIGNED((uintptr_t)self,HEAP_ALIGNMENT)) return false;
  if (!IS_ALIGNED(self->mf_size,HEAP_ALIGNMENT)) return false;
  if (*self->mf_lsize.le_pself != self) return false;
  return true;
 } CATCH (E_SEGFAULT) {
 }
 return false;
}
#else
#define mfree_set_checksum(self) (void)0
#endif

PUBLIC void (KCALL heap_validate_all)(void) {
#ifdef CONFIG_DEBUG_HEAP
 struct heap *h;
 if (THIS_VM == &vm_kernel) {
  /* Validate all heaps. */
  for (h  = kernel_heaps;
       h != COMPILER_ENDOF(kernel_heaps); ++h)
       (heap_validate)(h);
 } else {
  /* Only shared heaps are visible, so only validate those. */
  (heap_validate)(&kernel_heaps[GFP_SHARED]);
  (heap_validate)(&kernel_heaps[GFP_SHARED|GFP_LOCKED]);
 }
#endif
}

PUBLIC void (KCALL heap_validate)(struct heap *__restrict self) {
#ifdef CONFIG_DEBUG_HEAP
 unsigned int i;
#ifdef CONFIG_HAVE_HEAP_NOVALIDATE
 if (PERTASK_GET(heap_novalidate))
     return;
#endif
 if (!atomic_rwlock_tryread(&self->h_lock))
      return;
 for (i = 0; i < COMPILER_LENOF(self->h_size); ++i) {
  struct mfree **piter,*iter;
  piter = &self->h_size[i];
  for (; (iter = *piter) != NULL; piter = &iter->mf_lsize.le_next) {
   void *faulting_address; u32 expected_data;
   assertf(IS_ALIGNED((uintptr_t)iter,HEAP_ALIGNMENT),
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Free node at %p isn't aligned by `HEAP_ALIGNMENT' (pointed to by %p)",
           piter,(uintptr_t)piter+sizeof(void *)-1,iter,piter);
   assertf(iter->mf_size >= HEAP_MINSIZE,
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Free node at %p is too small (%Iu bytes) (size bucket %Iu/%Iu)\n",
          &iter->mf_size,(uintptr_t)&iter->mf_size+sizeof(size_t)-1,
           iter,iter->mf_size,i,COMPILER_LENOF(self->h_size));
   assertf((uintptr_t)iter+iter->mf_size > (uintptr_t)iter,
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Free node at %p is too large (%Iu bytes) (size bucket %Iu/%Iu)\n",
          &iter->mf_size,(uintptr_t)&iter->mf_size+sizeof(size_t)-1,
           iter,iter->mf_size,i,COMPILER_LENOF(self->h_size));
   assertf(IS_ALIGNED(iter->mf_size,HEAP_ALIGNMENT),
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Size of free node at %p...%p (%Iu;%#Ix bytes) isn't aligned by `HEAP_ALIGNMENT'",
          &iter->mf_size,(uintptr_t)&iter->mf_size+sizeof(size_t)-1,
           MFREE_MIN(iter),MFREE_MAX(iter),iter->mf_size,iter->mf_size);
   assertf(!(iter->mf_flags&~MFREE_FMASK),
           "\tPotential USE-AFTER-FREE of <%p>\n"
           "Free node at %p...%p contains invalid flags %x",
           &iter->mf_flags,MFREE_MIN(iter),
           MFREE_MAX(iter),iter->mf_flags);
   assertf(!iter->mf_laddr.a_min || quick_verify_mfree(iter->mf_laddr.a_min),
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Free node at %p...%p has broken min-pointer to invalid node at %p",
          (uintptr_t)&iter->mf_laddr.a_min,
          (uintptr_t)&iter->mf_laddr.a_min+sizeof(void *)-1,
           MFREE_MIN(iter),MFREE_MAX(iter),iter->mf_laddr.a_min);
   assertf(!iter->mf_laddr.a_max || quick_verify_mfree(iter->mf_laddr.a_max),
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Free node at %p...%p has broken min-pointer to invalid node at %p",
          (uintptr_t)&iter->mf_laddr.a_max,
          (uintptr_t)&iter->mf_laddr.a_max+sizeof(void *)-1,
           MFREE_MIN(iter),MFREE_MAX(iter),iter->mf_laddr.a_max);
   assertf(iter->mf_lsize.le_pself == piter,
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Expected self pointer of free node %p...%p at %p, but is actually at %p",
          (uintptr_t)&iter->mf_lsize.le_pself,
          (uintptr_t)&iter->mf_lsize.le_pself+sizeof(void *)-1,
           MFREE_MIN(iter),MFREE_MAX(iter),piter,iter->mf_lsize.le_pself);
   assertf(iter->mf_szchk == mfree_get_checksum(iter),
           "\tPotential USE-AFTER-FREE of <%p...%p> of %p\n"
           "Invalid checksum in free node %p...%p (expected %#.2I8x, but got %#.2I8x)",
          (uintptr_t)&iter->mf_size,(uintptr_t)(&iter->mf_size+1)-1,
          (uintptr_t)&iter->mf_szchk,
           MFREE_MIN(iter),MFREE_MAX(iter),
           mfree_get_checksum(iter),iter->mf_szchk);
   /* Verify memory of this node. */
   expected_data    = iter->mf_flags&MFREE_FZERO ? 0 : DEBUGHEAP_NO_MANS_LAND;
   faulting_address = find_modified_address(iter->mf_data,expected_data,
                                           (iter->mf_size-SIZEOF_MFREE)/4);
   if unlikely(faulting_address) {
    u8 *fault_start = (u8 *)faulting_address - 32;
    PREEMPTION_DISABLE();
    if (!pagedir_ismapped(VM_ADDR2PAGE((uintptr_t)fault_start)))
         fault_start = (u8 *)FLOOR_ALIGN((uintptr_t)faulting_address,PAGEALIGN);
    if (fault_start < (u8 *)iter->mf_data)
        fault_start = (u8 *)iter->mf_data;
    debug_printf("\n\n\n");
    format_hexdump(&debug_printer,NULL,
                   fault_start,16+2*((u8 *)faulting_address-fault_start),
                   16,FORMAT_HEXDUMP_FLAG_ADDRESS);
    debug_printf("\n");
    assertf(0,
            "\tIllegal USE-AFTER-FREE of <%p>\n"
            "Free node:     %p...%p\n"
            "Node offset:   %Iu (%#Ix)\n"
            "Expected byte: %.2I8x\n"
            "Found byte:    %.2I8x",
            faulting_address,
            MFREE_MIN(iter),MFREE_MAX(iter),
           (uintptr_t)faulting_address-MFREE_MIN(iter),
           (uintptr_t)faulting_address-MFREE_MIN(iter),
           ((u8 *)&expected_data)[(uintptr_t)faulting_address & 3],
           *(u8 *)faulting_address);
   }
  }
 }
 atomic_rwlock_endread(&self->h_lock);
#else
 (void)heap;
#endif
}

INTERN void KCALL
reset_heap_data(byte_t *ptr, u32 pattern, size_t num_bytes) {
 if (num_bytes < PAGESIZE)
     goto do_remainder;
 /* Only write pages that have been allocated. */
 if ((uintptr_t)ptr & (PAGESIZE-1)) {
  size_t inpage_free = PAGESIZE - ((uintptr_t)ptr & (PAGESIZE-1));
  if (inpage_free > num_bytes)
      inpage_free = num_bytes;
  mempatl(ptr,pattern,inpage_free);
  ptr       += inpage_free;
  num_bytes -= inpage_free;
 }
 while (num_bytes >= PAGESIZE) {
  /* Only reset pages that have been allocated.
   * This optimization goes hand-in-hand with `heap_validate_all()'
   * not checking pages that haven't been allocated. */
  if (pagedir_ismapped(VM_ADDR2PAGE((uintptr_t)ptr)))
      memsetl(ptr,pattern,PAGESIZE/4);
  num_bytes -= PAGESIZE;
  ptr       += PAGESIZE;
 }
 if (num_bytes) {
do_remainder:
  mempatl(ptr,pattern,num_bytes);
 }
}



/* Core HEAP Allocators. */


PRIVATE VIRT vm_vpage_t KCALL
core_page_alloc(struct heap *__restrict self,
                size_t num_pages, gfp_t flags) {
 struct vm_corepair EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(corepair);
 vm_vpage_t COMPILER_IGNORE_UNINITIALIZED(result);
 assert(num_pages != 0);
 /* Throw a would-block error if we're not allowed to map new memory. */
 if (flags & GFP_NOMAP)
     error_throw(E_WOULDBLOCK);
 heap_validate_all();
 /* Only allocate using corebase when `GFP_SHARED|GFP_LOCKED'
  * Otherwise, we can allocate the region and node using
  * that same set of flags in a call to `kmalloc()'. */
 if ((flags&__GFP_HEAPMASK) == (GFP_SHARED|GFP_LOCKED)) {
  /* Allocate a new corepair to describe this new mapping. */
  corepair = vm_corealloc();
 } else {
  corepair.cp_node = (VIRT struct vm_node *)kmalloc(sizeof(struct vm_node),
                                                    GFP_SHARED|GFP_LOCKED|GFP_CALLOC|
                                                   (flags & GFP_ATOMIC));
  TRY {
   corepair.cp_region = (VIRT struct vm_region *)kmalloc(sizeof(struct vm_region),
                                                         GFP_SHARED|GFP_LOCKED|GFP_CALLOC|
                                                        (flags & GFP_ATOMIC));
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   kfree(corepair.cp_node);
   error_rethrow();
  }
 }
 /* Setup the corepair. */
 corepair.cp_node->vn_region = corepair.cp_region;
 corepair.cp_node->vn_prot   = PROT_READ|PROT_WRITE|PROT_NOUSER;
 corepair.cp_region->vr_size = num_pages;
 corepair.cp_region->vr_refcnt = 1;
#if VM_REGION_MEM != 0
 corepair.cp_region->vr_type = VM_REGION_MEM;
#endif
 if (flags&GFP_CALLOC) {
  corepair.cp_region->vr_init = VM_REGION_INIT_FFILLER;
 }
#ifdef CONFIG_DEBUG_HEAP
 else {
  corepair.cp_region->vr_init           = VM_REGION_INIT_FFILLER;
  corepair.cp_region->vr_setup.s_filler = DEBUGHEAP_FRESH_MEMORY;
 }
#endif
 corepair.cp_region->vr_part0.vp_refcnt         = 1;
 corepair.cp_region->vr_parts                   = &corepair.cp_region->vr_part0;
 corepair.cp_region->vr_part0.vp_chain.le_pself = &corepair.cp_region->vr_parts;
 TRY {
  if (flags&GFP_LOCKED) {
   /* Allocate memory immediately, as direct, physical memory. */
   corepair.cp_region->vr_type                                 = VM_REGION_PHYSICAL;
   corepair.cp_region->vr_part0.vp_state                       = VM_PART_INCORE;
   corepair.cp_region->vr_part0.vp_phys.py_num_scatter         = 1;
   corepair.cp_region->vr_part0.vp_phys.py_iscatter[0].ps_size = num_pages; /* TODO: Use scatter */
   corepair.cp_region->vr_part0.vp_phys.py_iscatter[0].ps_addr = page_malloc(num_pages,MZONE_ANY);
  }
  if (!vm_tryacquire(&vm_kernel)) {
   if (flags & GFP_ATOMIC)
       error_throw(E_WOULDBLOCK);
   vm_acquire(&vm_kernel);
  }
  TRY {
   vm_vpage_t pagehint;
   /* Find a suitable, free location, using heap-specific hints. */
   assert(!(flags&GFP_KERNEL) || THIS_VM == &vm_kernel);
   pagehint = self->h_hintpage;
   result   = vm_getfree(pagehint,num_pages,1,0,self->h_hintmode);
   /* Save the new hint. */
   ATOMIC_CMPXCH(self->h_hintpage,pagehint,result);
   corepair.cp_node->vn_node.a_vmin = result;
   corepair.cp_node->vn_node.a_vmax = result+num_pages-1;
   /* Finally, map the corepair */
   vm_insert_and_activate_node(&vm_kernel,corepair.cp_node);
  } FINALLY {
   vm_release(&vm_kernel);
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  vm_region_decref(corepair.cp_region); /* Inherited by `corepair.cp_node' upon success. */
  vm_node_free(corepair.cp_node);       /* Inherited by `vm_insert_and_activate_node()' upon success. */
  error_rethrow();
 }
 heap_validate_all();
 if (flags&GFP_LOCKED) {
  /* Make sure that the newly mapped memory becomes visible _now_. */
  pagedir_sync(result,num_pages);
  /* Seeing as how we pre-allocated the region's memory
   * as physical, we must now initialize it ourselves! */
  if (flags&GFP_CALLOC) {
   memsetl((void *)VM_PAGE2ADDR(result),0,
            num_pages * PAGESIZE / 4);
  }
#ifdef CONFIG_DEBUG_HEAP
  else {
   memsetl((void *)VM_PAGE2ADDR(result),
            DEBUGHEAP_FRESH_MEMORY,
            num_pages * PAGESIZE / 4);
  }
#endif
 }
 heap_validate_all();
 return result;
}
PRIVATE bool KCALL
core_page_allocat(vm_vpage_t page_index,
                  size_t num_pages, gfp_t flags) {
 struct vm_corepair EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(corepair);
 assert(num_pages != 0);
 /* Throw a would-block error if we're not allowed to map new memory. */
 if (flags & GFP_NOMAP)
     error_throw(E_WOULDBLOCK);
 if (!vm_tryacquire(&vm_kernel)) {
  if (flags & GFP_ATOMIC)
      error_throw(E_WOULDBLOCK);
  vm_acquire(&vm_kernel);
 }
 TRY {
  /* Check if the kernel VM already contains a mapping at the given address. */
  if (vm_getanynode(page_index,page_index+num_pages)) {
   vm_release(&vm_kernel);
   return false; /* XXX: Skip finally is intended. */
  }
  /* Only allocate using corebase when `GFP_SHARED|GFP_LOCKED'
   * Otherwise, we can allocate the region and node using
   * that same set of flags in a call to `kmalloc()'. */
  if ((flags&__GFP_HEAPMASK) == (GFP_SHARED|GFP_LOCKED)) {
   /* Allocate a new corepair to describe this new mapping. */
   corepair = vm_corealloc();
  } else {
   corepair.cp_node = (VIRT struct vm_node *)kmalloc(sizeof(struct vm_node),
                                                     GFP_SHARED|GFP_LOCKED|GFP_CALLOC|
                                                    (flags & GFP_ATOMIC));
   TRY {
    corepair.cp_region = (VIRT struct vm_region *)kmalloc(sizeof(struct vm_region),
                                                          GFP_SHARED|GFP_LOCKED|GFP_CALLOC|
                                                         (flags & GFP_ATOMIC));
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    kfree(corepair.cp_node);
    error_rethrow();
   }
  }
  
  TRY {
   /* Setup the corepair. */
   corepair.cp_node->vn_region = corepair.cp_region;
   corepair.cp_region->vr_size = num_pages;
   corepair.cp_region->vr_refcnt = 1;
#if VM_REGION_MEM != 0
   corepair.cp_region->vr_type = VM_REGION_MEM;
#endif
   if (flags & GFP_CALLOC) {
    corepair.cp_region->vr_init = VM_REGION_INIT_FFILLER;
   }
#ifdef CONFIG_DEBUG_HEAP
   else {
    corepair.cp_region->vr_init           = VM_REGION_INIT_FFILLER;
    corepair.cp_region->vr_setup.s_filler = DEBUGHEAP_FRESH_MEMORY;
   }
#endif
   corepair.cp_region->vr_part0.vp_refcnt = 1;
   corepair.cp_region->vr_parts = &corepair.cp_region->vr_part0;
   corepair.cp_region->vr_part0.vp_chain.le_pself = &corepair.cp_region->vr_parts;
   if (flags & GFP_LOCKED) {
    /* Allocate memory immediately, as direct, physical memory. */
    corepair.cp_region->vr_type                                 = VM_REGION_PHYSICAL;
    corepair.cp_region->vr_part0.vp_state                       = VM_PART_INCORE;
    corepair.cp_region->vr_part0.vp_phys.py_num_scatter         = 1;
    corepair.cp_region->vr_part0.vp_phys.py_iscatter[0].ps_size = num_pages; /* TODO: Use scatter */
    corepair.cp_region->vr_part0.vp_phys.py_iscatter[0].ps_addr = page_malloc(num_pages,MZONE_ANY);
   }
   /* Find a suitable, free location. */
   corepair.cp_node->vn_node.a_vmin = page_index;
   corepair.cp_node->vn_node.a_vmax = page_index+num_pages-1;
   corepair.cp_node->vn_prot        = PROT_READ|PROT_WRITE|PROT_NOUSER;
   /* Finally, map the corepair */
   vm_insert_and_activate_node(&vm_kernel,corepair.cp_node);
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   vm_region_destroy(corepair.cp_region);
   vm_node_free(corepair.cp_node);
   error_rethrow();
  }
 } FINALLY {
  vm_release(&vm_kernel);
 }
 if (flags&GFP_LOCKED) {
  /* Make sure that the newly mapped memory becomes visible _now_. */
  pagedir_sync(page_index,num_pages);
  /* Seeing as how we pre-allocated the region's memory
   * as physical, we must now initialize it ourselves! */
  if (flags&GFP_CALLOC) {
   memsetl((void *)VM_PAGE2ADDR(page_index),0,
            num_pages * PAGESIZE / 4);
  }
#ifdef CONFIG_DEBUG_HEAP
  else {
   memsetl((void *)VM_PAGE2ADDR(page_index),
            DEBUGHEAP_FRESH_MEMORY,
            num_pages * PAGESIZE / 4);
  }
#endif
 }
 heap_validate_all();
 return true;
}


PRIVATE ATTR_NOTHROW void KCALL
core_page_free(vm_vpage_t page_index,
               size_t num_pages, gfp_t flags) {
#if 1
 /* free() should be atomic, but this could block...
  * As a hacky work-around, disable RPC serving for the during of the call. */
 u16 old_state;
 old_state = ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FDONTSERVE);
 assert(num_pages != 0);
 vm_unmap(page_index,num_pages,
          VM_UNMAP_NOEXCEPT|VM_UNMAP_SYNC,NULL);
 if (!(old_state & TASK_STATE_FDONTSERVE))
       ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FDONTSERVE);
#else
 assert(num_pages != 0);
 vm_unmap(page_index,num_pages,
          VM_UNMAP_NOEXCEPT|
          VM_UNMAP_ATOMIC|
          VM_UNMAP_SYNC,
          NULL);
#endif
}



INTERN ATTR_NORETURN void KCALL
heap_allocation_overflow(size_t n_bytes) {
 struct exception_info *info;
 /* Throw a bad-allocation error */
 info                 = error_info();
 info->e_error.e_code = E_BADALLOC;
 info->e_error.e_flag = ERR_FNORMAL;
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_badalloc.ba_amount   = n_bytes;
 info->e_error.e_badalloc.ba_resource = ERROR_BADALLOC_VIRTMEMORY;
 error_throw_current();
 __builtin_unreachable();
}

LOCAL ATTR_NOTHROW void KCALL
heap_insert_node_unlocked(struct heap *__restrict self,
                          struct mfree *__restrict node) {
 struct mfree **pslot,*slot;
 size_t num_bytes = node->mf_size;
 assertf(node->mf_size,"Empty node at %p",node);
 /* Insert the node into the address and size trees. */
 mfree_tree_insert(&self->h_addr,node);
 /* Figure out where the free-slot should go in the chain of free ranges. */
 pslot = &self->h_size[HEAP_BUCKET_OF(num_bytes)];
 while ((slot = *pslot) != NULL &&
         MFREE_SIZE(slot) < num_bytes)
         pslot = &slot->mf_lsize.le_next;
 node->mf_lsize.le_pself = pslot;
 node->mf_lsize.le_next  = slot;
 if (slot) slot->mf_lsize.le_pself = &node->mf_lsize.le_next;
 *pslot = node;
}


PRIVATE ATTR_NOTHROW void KCALL
heap_free_raw(struct heap *__restrict self,
              VIRT void *ptr, size_t num_bytes,
              gfp_t flags) {
 ATREE_SEMI_T(uintptr_t) addr_semi;
 ATREE_LEVEL_T addr_level;
 struct mfree **pslot,*slot,*new_slot;
#ifdef CONFIG_DEBUG_HEAP
#ifdef CONFIG_HEAP_TRACE_DANGLE
 size_t dandle_size = 0;
#endif
again:
#endif /* CONFIG_DEBUG_HEAP */
 assertf(num_bytes >= HEAP_MINSIZE,"Invalid heap_free(): Too few bytes (%Iu < %Iu)",num_bytes,HEAP_MINSIZE);
 assertf(IS_ALIGNED((uintptr_t)ptr,HEAP_ALIGNMENT),"Invalid heap_free(): Unaligned base pointer %p",ptr);
 assertf(IS_ALIGNED(num_bytes,HEAP_ALIGNMENT),"Invalid heap_free(): Unaligned free size %Iu (%#Ix)",num_bytes,num_bytes);
 assertf(((uintptr_t)ptr + num_bytes) > (uintptr_t)ptr,"Address space overflow when freeing %p...%p",ptr,(uintptr_t)ptr+num_bytes-1);
 heap_validate_all();
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);

 atomic_rwlock_write(&self->h_lock);
 /* Search for a node below, that could then be extended above. */
 pslot = mfree_tree_plocate_at(&self->h_addr,(uintptr_t)ptr-1,
                               &addr_semi,&addr_level);
 if (pslot) {
  slot = *pslot;
  asserte(mfree_tree_pop_at(pslot,addr_semi,addr_level) == slot);
  LIST_REMOVE(slot,mf_lsize);

  /* Extend this node above. */
#ifndef CONFIG_DEBUG_HEAP
  slot->mf_flags &= flags & GFP_CALLOC;
  slot->mf_size  += num_bytes;
#else
  if ((flags & GFP_CALLOC) && (slot->mf_flags & GFP_CALLOC)) {
   slot->mf_size += num_bytes;
  } else {
#ifdef CONFIG_HEAP_TRACE_DANGLE
   ATOMIC_FETCHADD(self->h_dangle,slot->mf_size);
   dandle_size += slot->mf_size;
#endif
   atomic_rwlock_endwrite(&self->h_lock);
   heap_validate_all();
   if (flags & GFP_CALLOC)
       reset_heap_data((byte_t *)ptr,DEBUGHEAP_NO_MANS_LAND,num_bytes),
       flags &= ~GFP_CALLOC;
   heap_validate_all();
   if (slot->mf_flags & GFP_CALLOC)
       reset_heap_data((byte_t *)((uintptr_t)slot+SIZEOF_MFREE),
                        DEBUGHEAP_NO_MANS_LAND,slot->mf_size-SIZEOF_MFREE);
   heap_validate_all();
   ptr        = (VIRT void *)slot;
   num_bytes += slot->mf_size;
   mempatl(ptr,DEBUGHEAP_NO_MANS_LAND,SIZEOF_MFREE);
   goto again;
  }
#endif

  /* Check if there is another node above that we must now merge with. */
  addr_semi  = ATREE_SEMI0(uintptr_t);
  addr_level = ATREE_LEVEL0(uintptr_t);
  pslot = mfree_tree_plocate_at(&self->h_addr,(uintptr_t)ptr+num_bytes,
                                &addr_semi,&addr_level);
  if unlikely(pslot) {
   struct mfree *high_slot;
   high_slot = *pslot;
   /* Include this high-slot in the union that will be freed. */
   asserte(mfree_tree_pop_at(pslot,addr_semi,addr_level) == high_slot);
   LIST_REMOVE(high_slot,mf_lsize);
#ifndef CONFIG_DEBUG_HEAP
   slot->mf_flags &= high_slot->mf_flags & GFP_CALLOC;
   slot->mf_size  += high_slot->mf_size;
   if (slot->mf_flags & GFP_CALLOC)
       memset(high_slot,0,SIZEOF_MFREE);
#else
   if ((slot->mf_flags & GFP_CALLOC) &&
       (high_slot->mf_flags & GFP_CALLOC)) {
    slot->mf_size += high_slot->mf_size;
    memset(high_slot,0,SIZEOF_MFREE);
   } else {
#ifdef CONFIG_HEAP_TRACE_DANGLE
    ATOMIC_FETCHADD(self->h_dangle,high_slot->mf_size);
    dandle_size += high_slot->mf_size;
#endif
    atomic_rwlock_endwrite(&self->h_lock);
    heap_validate_all();
    if (slot->mf_flags & GFP_CALLOC)
        reset_heap_data((byte_t *)((uintptr_t)slot+SIZEOF_MFREE),
                         DEBUGHEAP_NO_MANS_LAND,slot->mf_size-SIZEOF_MFREE);
    heap_validate_all();
    if (high_slot->mf_flags & GFP_CALLOC)
        reset_heap_data((byte_t *)((uintptr_t)high_slot+SIZEOF_MFREE),
                         DEBUGHEAP_NO_MANS_LAND,high_slot->mf_size-SIZEOF_MFREE);
    heap_validate_all();
    ptr       = (VIRT void *)slot;
    num_bytes = slot->mf_size + high_slot->mf_size;
    flags    &= ~GFP_CALLOC;
    mempatl(slot,DEBUGHEAP_NO_MANS_LAND,SIZEOF_MFREE);
    mempatl(high_slot,DEBUGHEAP_NO_MANS_LAND,SIZEOF_MFREE);
    heap_validate_all();
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
 pslot = mfree_tree_plocate_at(&self->h_addr,(uintptr_t)ptr+num_bytes,
                               &addr_semi,&addr_level);
 if (pslot) {
  gfp_t slot_flags; size_t slot_size;
  slot = *pslot;
  asserte(mfree_tree_pop_at(pslot,addr_semi,addr_level) == slot);
  LIST_REMOVE(slot,mf_lsize);
  /* Extend this node below. */

#ifndef CONFIG_DEBUG_HEAP
  slot_flags = slot->mf_flags & flags;
  slot_size  = slot->mf_size;
  if (slot_flags & GFP_CALLOC)
      memset(slot,0,SIZEOF_MFREE);
  new_slot           = (struct mfree *)ptr;
  new_slot->mf_size  = slot_size + num_bytes;
  new_slot->mf_flags = slot_flags;
#else
  slot_flags = slot->mf_flags;
  slot_size  = slot->mf_size;
  if ((slot_flags & GFP_CALLOC) && (flags & GFP_CALLOC)) {
   memset(slot,0,SIZEOF_MFREE);
   new_slot           = (struct mfree *)ptr;
   new_slot->mf_size  = slot_size + num_bytes;
   new_slot->mf_flags = slot_flags;
  } else {
#ifdef CONFIG_HEAP_TRACE_DANGLE
   ATOMIC_FETCHADD(self->h_dangle,slot_size);
   dandle_size += slot_size;
#endif
   atomic_rwlock_endwrite(&self->h_lock);
   if (flags & GFP_CALLOC)
       reset_heap_data((byte_t *)ptr,DEBUGHEAP_NO_MANS_LAND,num_bytes);
   if (slot_flags & GFP_CALLOC)
        reset_heap_data((byte_t *)slot,DEBUGHEAP_NO_MANS_LAND,slot_size);
   else mempatl(slot,DEBUGHEAP_NO_MANS_LAND,SIZEOF_MFREE);
   flags     &= ~GFP_CALLOC;
   num_bytes += slot_size;
   goto again;
  }
#endif
  /* Re-insert our new, bigger node. */
  goto load_new_slot;
 }

 /* Create a new node. */
 new_slot = (struct mfree *)ptr;
 new_slot->mf_size  = num_bytes;
 new_slot->mf_flags = flags & MFREE_FMASK;
load_new_slot:
#ifdef CONFIG_DEBUG_HEAP
#ifdef CONFIG_HEAP_TRACE_DANGLE
 /* Untrack danging heap data. */
 ATOMIC_FETCHSUB(self->h_dangle,dandle_size);
#endif
#endif
 if (!(flags & GFP_NOMAP) &&
       new_slot->mf_size >= self->h_freethresh) {
  /*  When a slot grows larger than `self->h_freethresh' and
   * `GFP_NOMAP' isn't set, free as much of it as possible by
   *  passing its memory to `core_page_free()' */
  vm_vpage_t free_minpage = CEILDIV(MFREE_MIN(new_slot),PAGESIZE);
  vm_vpage_t free_endpage = FLOORDIV(MFREE_END(new_slot),PAGESIZE);
  if (free_minpage < free_endpage) {
   uintptr_t hkeep,tkeep; vm_vpage_t old_hint;
   size_t hkeep_size,tkeep_size;
   /* Figure out the header and tail which we can't release. */
   hkeep      = MFREE_MIN(new_slot);
   tkeep      = (free_endpage*PAGESIZE);
   hkeep_size = (free_minpage*PAGESIZE)-(uintptr_t)hkeep;
   assertf(MFREE_END(new_slot) >= (uintptr_t)tkeep,
           "new_slot = %p...%p\n"
           "tkeep    = %p\n",
           MFREE_MIN(new_slot),
           MFREE_MAX(new_slot),tkeep);
   tkeep_size = MFREE_END(new_slot)-(uintptr_t)tkeep;
   flags     &= ~GFP_CALLOC;
   flags     |= new_slot->mf_flags;
   /* Ensure that the keep-segments are large enough. */
   if unlikely(hkeep_size && hkeep_size < HEAP_MINSIZE)
      hkeep_size += PAGESIZE,++free_minpage;
   if unlikely(tkeep_size && tkeep_size < HEAP_MINSIZE)
      tkeep_size += PAGESIZE,tkeep -= PAGESIZE,--free_endpage;
   if unlikely(free_minpage >= free_endpage)
      goto do_load_new_slot;
   atomic_rwlock_endwrite(&self->h_lock);

   /* Release this slot to the VM. */
   if (hkeep_size) {
    if (flags & GFP_CALLOC)
         memset((void *)hkeep,0,SIZEOF_MFREE);
#ifdef CONFIG_DEBUG_HEAP
    else mempatl((void *)hkeep,DEBUGHEAP_NO_MANS_LAND,SIZEOF_MFREE);
#endif
   }
   if (tkeep_size) {
    if (flags & GFP_CALLOC)
         memset((void *)tkeep,0,SIZEOF_MFREE);
#ifdef CONFIG_DEBUG_HEAP
    else mempatl((void *)tkeep,DEBUGHEAP_NO_MANS_LAND,SIZEOF_MFREE);
#endif
   }
#if 1
   debug_printf("[HEAP] Release to the system: [%p+%#Ix head] %p...%p [%p+%#Ix tail]\n",
                hkeep,hkeep_size,VM_PAGE2ADDR(free_minpage),VM_PAGE2ADDR(free_endpage)-1,
                tkeep,tkeep_size);
#endif

   /* Free unused header and tail data. */
   if (hkeep_size) heap_free_raw(self,(void *)hkeep,hkeep_size,flags);
   if (tkeep_size) heap_free_raw(self,(void *)tkeep,tkeep_size,flags);

   /* Release full pages back to the system. */
   core_page_free(free_minpage,
                  free_endpage-free_minpage,
                  flags);

   /* Reset the heap hint in a way that will keep our heap
    * around the same area of memory, preventing it from slowly
    * crawling across the entire address space. */
   if (self->h_hintmode & VM_GETFREE_FBELOW) {
    do if ((old_hint = ATOMIC_READ(self->h_hintpage)) >= free_endpage) break;
    while (!ATOMIC_CMPXCH_WEAK(self->h_hintpage,old_hint,free_endpage));
   } else {
    do if ((old_hint = ATOMIC_READ(self->h_hintpage)) <= free_minpage) break;
    while (!ATOMIC_CMPXCH_WEAK(self->h_hintpage,old_hint,free_minpage));
   }

   heap_validate_all();
   return;
  }
 }
do_load_new_slot:
 mfree_set_checksum(new_slot);
 /* Insert the node into the address and size trees. */
 heap_insert_node_unlocked(self,new_slot);
 /* Done! */
 atomic_rwlock_endwrite(&self->h_lock);
 heap_validate_all();
}

PUBLIC ATTR_NOTHROW void KCALL
heap_free_untraced(struct heap *__restrict self,
                   VIRT void *ptr, size_t num_bytes,
                   gfp_t flags) {
 assertf(num_bytes >= HEAP_MINSIZE,"Invalid heap_free(): Too few bytes (%Iu < %Iu)",num_bytes,HEAP_MINSIZE);
 assertf(IS_ALIGNED((uintptr_t)ptr,HEAP_ALIGNMENT),"Invalid heap_free(): Unaligned base pointer %p",ptr);
 assertf(IS_ALIGNED(num_bytes,HEAP_ALIGNMENT),"Invalid heap_free(): Unaligned free size %Iu (%#Ix)",num_bytes,num_bytes);
 /* Reset debug information. */
#ifdef CONFIG_DEBUG_HEAP
 if (!(flags & GFP_CALLOC))
     reset_heap_data((byte_t *)ptr,DEBUGHEAP_NO_MANS_LAND,num_bytes);
#endif
 heap_free_raw(self,ptr,num_bytes,flags);
}


/* Free a high-memory overallocation of `num_free_bytes'
 * at `overallocation_base', given the associated
 * base-pointer located at `base_pointer'
 * This function deals with debug-initialization in the
 * event that the overallocated base location hasn't been
 * allocated yet, in a way that prevent the associated
 * pages from being allocated during `heap_free_raw()' in
 * a way that would produce invalid (use-after-free) memory. */
#ifdef CONFIG_DEBUG_HEAP
PRIVATE void KCALL
heap_free_overallocation(struct heap *__restrict self,
                         void *base_pointer,
                         void *overallocation_base,
                         size_t num_free_bytes,
                         gfp_t flags) {
 assert(IS_ALIGNED((uintptr_t)overallocation_base,HEAP_ALIGNMENT));
 assert(IS_ALIGNED(num_free_bytes,HEAP_ALIGNMENT));
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
 debug_printf("Check no mans land %p, %p, %p\n",
             (uintptr_t)overallocation_base + (SIZEOF_MFREE-1),base_pointer,
             (uintptr_t)overallocation_base+num_free_bytes);
#endif
 /* NOTE: Must compare the end of the written-to portion of
  *       the unused data block with the resulting pointer.
  *       Since we know that `overallocation_base > base_pointer',
  *       we must check if the outer-most byte of the unused
  *       data block (which is written to during the free()
  *       operation) is is a different page. */
 if ((((uintptr_t)overallocation_base + (SIZEOF_MFREE-1)) & ~(PAGESIZE-1)) !=
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
  if (clear_size < SIZEOF_MFREE) clear_size += PAGESIZE;
  /* NOTE: When the clear size is larger than what is being un-used,
   *       then we must assume that some other node in the same page
   *       has already allocated that page. */
  if (clear_size <= num_free_bytes) {
   /* The the CALLOC flag is set, then the memory will initialize itself to ZERO */
   if (!(flags & GFP_CALLOC)) {
#if 0
    debug_printf("Reset no mans land for page %p...%p\n",
                 overallocation_base,(uintptr_t)overallocation_base+clear_size-1);
#endif
    mempatl(overallocation_base,DEBUGHEAP_NO_MANS_LAND,clear_size);
   }
  }
 }
 heap_free_raw(self,overallocation_base,
               num_free_bytes,flags);
}
#else
/* Without debug initialization, this isn't a problem! */
#define heap_free_overallocation(self,base_pointer,overallocation_base,num_free_bytes,flags) \
        heap_free_raw(self,overallocation_base,num_free_bytes,flags)
#endif


PUBLIC struct heapptr KCALL
heap_alloc_untraced(struct heap *__restrict self,
                    size_t num_bytes, gfp_t flags) {
 struct heapptr result; struct mfree **iter,**end;
 if unlikely(__builtin_add_overflow(num_bytes,HEAP_ALIGNMENT-1,&result.hp_siz))
    heap_allocation_overflow(num_bytes);
 result.hp_siz &= ~(HEAP_ALIGNMENT-1);
 if unlikely(result.hp_siz < HEAP_MINSIZE)
             result.hp_siz = HEAP_MINSIZE;
 iter = &self->h_size[HEAP_BUCKET_OF(result.hp_siz)];
 end  =  COMPILER_ENDOF(self->h_size);
 assertf(iter >= self->h_size &&
         iter <  COMPILER_ENDOF(self->h_size),
         "HEAP_BUCKET_OF(%Iu) = %Iu/%Iu",
         result.hp_siz,HEAP_BUCKET_OF(result.hp_siz),
         COMPILER_LENOF(self->h_size));
search_heap:
 heap_validate_all();
 atomic_rwlock_write(&self->h_lock);
 for (; iter != end; ++iter) {
#ifdef CONFIG_HEAP_TRACE_DANGLE
  size_t dangle_size;
#endif /* CONFIG_HEAP_TRACE_DANGLE */
  size_t unused_size;
  struct mfree *chain;
  gfp_t chain_flags;
  /* Search this bucket. */
  chain = *iter;
  while (chain && MFREE_SIZE(chain) < result.hp_siz)
         chain = chain->mf_lsize.le_next;
  if (!chain) continue;
  asserte(mfree_tree_remove(&self->h_addr,MFREE_BEGIN(chain)) == chain);
  LIST_REMOVE(chain,mf_lsize);
#ifdef CONFIG_HEAP_TRACE_DANGLE
  /* Track the potentially unused data size as dangling data. */
  dangle_size = MFREE_SIZE(chain)-result.hp_siz;
  ATOMIC_FETCHADD(self->h_dangle,dangle_size);
#endif /* CONFIG_HEAP_TRACE_DANGLE */
  atomic_rwlock_endwrite(&self->h_lock);
  heap_validate_all();
  /* We've got the memory! */
  result.hp_ptr = (void *)chain;
  chain_flags = chain->mf_flags;
#ifdef CONFIG_HEAP_TRACE_DANGLE
  unused_size = dangle_size;
#else
  unused_size = MFREE_SIZE(chain)-result.hp_siz;
#endif
  if (unused_size < HEAP_MINSIZE) {
   /* Remainder is too small. - Allocate it as well. */
   result.hp_siz += unused_size;
  } else {
   void *unused_begin = (void *)((uintptr_t)chain+result.hp_siz);
   /* Free the unused portion. */
#ifdef CONFIG_HEAP_RANDOMIZE_OFFSETS
   /* Randomize allocated memory by shifting the
    * resulting pointer somewhere up higher. */
   uintptr_t random_offset;
   random_offset  = rand() % unused_size;
   random_offset &= ~(HEAP_ALIGNMENT-1);
   if (random_offset >= HEAP_MINSIZE) {
    /* Rather than allocating `chain...+=num_bytes', instead
     * allocate `chain+random_offset...+=num_bytes' and free
     * `chain...+=random_offset' and
     * `chain+random_offset+num_bytes...+=unused_size-random_offset' */
    if (chain_flags&MFREE_FZERO)
         memset(chain,0,SIZEOF_MFREE);
#ifdef CONFIG_DEBUG_HEAP
    else mempatl(chain,DEBUGHEAP_NO_MANS_LAND,SIZEOF_MFREE);
#endif
    /* Free unused low memory. */
    heap_free_raw(self,chain,random_offset,
                 (flags&~(GFP_CALLOC))|chain_flags);
    /* Set the new resulting pointer. */
    result.hp_ptr = (void *)((uintptr_t)chain + random_offset);
    unused_size  -= random_offset;
    if (unused_size < HEAP_MINSIZE) {
     result.hp_siz += unused_size;
    } else {
     unused_begin = (void *)((uintptr_t)result.hp_ptr + result.hp_siz);
     heap_free_overallocation(self,
                              result.hp_ptr,
                              unused_begin,
                              unused_size,
                             (flags&~(GFP_CALLOC))|chain_flags);
    }
   } else
#endif /* CONFIG_HEAP_RANDOMIZE_OFFSETS */
   {
    assert(unused_size < MFREE_SIZE(chain));
    /* Free the unused overallocation. */
    heap_free_overallocation(self,
                             result.hp_ptr,
                             unused_begin,
                             unused_size,
                            (flags&~(GFP_CALLOC))|chain_flags);
   }
  }
#ifdef CONFIG_HEAP_TRACE_DANGLE
  /* Now that it's been returned, the data is no longer dangling. */
  ATOMIC_FETCHSUB(self->h_dangle,dangle_size);
#endif /* CONFIG_HEAP_TRACE_DANGLE */
  /* Initialize the result memory. */
  if (flags&GFP_CALLOC) {
   if (chain_flags&MFREE_FZERO)
        memset(result.hp_ptr,0,SIZEOF_MFREE);
   else memset(result.hp_ptr,0,result.hp_siz);
  }
#ifdef CONFIG_DEBUG_HEAP
  else {
   reset_heap_data((byte_t *)result.hp_ptr,
                    DEBUGHEAP_FRESH_MEMORY,
                    result.hp_siz);
  }
#endif
  assert(IS_ALIGNED((uintptr_t)result.hp_ptr,HEAP_ALIGNMENT));
  assert(IS_ALIGNED((uintptr_t)result.hp_siz,HEAP_ALIGNMENT));
  assert(result.hp_siz >= HEAP_MINSIZE);
  return result;
 }
#ifdef CONFIG_HEAP_TRACE_DANGLE
 /* Check for dangling data and don't allocate new memory if enough exists. */
 if (ATOMIC_READ(self->h_dangle) >= result.hp_siz) {
  atomic_rwlock_endwrite(&self->h_lock);
  /* Let some other thread about to release dangling
   * data do so, then search the heap again. */
  task_yield();
  goto search_heap;
 }
#endif /* CONFIG_HEAP_TRACE_DANGLE */
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
  vm_vpage_t EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(pageaddr);
  size_t EXCEPT_VAR page_bytes;
  size_t unused_size;
  if unlikely(__builtin_add_overflow(result.hp_siz,PAGESIZE-1,&page_bytes))
     heap_allocation_overflow(result.hp_siz);
  if (!(flags & GFP_NOOVER)) {
   /* Add overhead for overallocation. */
   if unlikely(__builtin_add_overflow(page_bytes,self->h_overalloc,&page_bytes))
      goto allocate_without_overalloc;
  }
  TRY {
   page_bytes &= ~(PAGESIZE-1);
   pageaddr    = core_page_alloc(self,page_bytes/PAGESIZE,flags);
  } CATCH(E_BADALLOC) {
allocate_without_overalloc:
   /* Try again without overallocation. */
   page_bytes  = result.hp_siz + (PAGESIZE-1);
   page_bytes &= ~(PAGESIZE-1);
   pageaddr    = core_page_alloc(self,page_bytes/PAGESIZE,flags);
  }
#if 0
  debug_printf("[HEAP] Acquire from the system: %p...%p\n",
               VM_PAGE2ADDR(pageaddr),VM_PAGE2ADDR(pageaddr)+page_bytes-1);
#endif
  /* Got it! */
  result.hp_ptr = (VIRT void *)VM_PAGE2ADDR(pageaddr);
  unused_size   = page_bytes - result.hp_siz;
  if unlikely(unused_size < HEAP_MINSIZE)
   result.hp_siz = page_bytes;
  else {
   void *unused_begin = (void *)((uintptr_t)result.hp_ptr + result.hp_siz);
   /* Free unused size. */
   assert(IS_ALIGNED(unused_size,HEAP_ALIGNMENT));
   assert(IS_ALIGNED((uintptr_t)unused_begin,HEAP_ALIGNMENT));
#ifdef CONFIG_DEBUG_HEAP
   if (!(flags&GFP_CALLOC))
         mempatl(unused_begin,DEBUGHEAP_NO_MANS_LAND,unused_size);
#endif
   /* Release the unused memory. */
   heap_free_raw(self,unused_begin,unused_size,flags);
  }
#if 1 /* ??? */
  if (vm_tryacquire(&vm_kernel)) {
   /* Try to optimize the VM by merging the new mapping with adjacent nodes. */
   vm_merge_before(&vm_kernel,pageaddr);
   vm_merge_before(&vm_kernel,pageaddr+page_bytes/PAGESIZE);
   vm_release(&vm_kernel);
  }
#endif
 }
 assert(IS_ALIGNED((uintptr_t)result.hp_ptr,HEAP_ALIGNMENT));
 assert(IS_ALIGNED((uintptr_t)result.hp_siz,HEAP_ALIGNMENT));
 assert(result.hp_siz >= HEAP_MINSIZE);
 return result;
}

PRIVATE size_t KCALL
heap_allat_partial(struct heap *__restrict self,
                   VIRT void *__restrict ptr, gfp_t flags) {
 ATREE_SEMI_T(uintptr_t) addr_semi;
 ATREE_LEVEL_T addr_level; gfp_t slot_flags;
 size_t result; struct mfree **pslot,*slot;
 assert(IS_ALIGNED((uintptr_t)ptr,HEAP_ALIGNMENT));
again:
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);

 /* Check if the requested address is in cache. */
 atomic_rwlock_write(&self->h_lock);
 pslot = mfree_tree_plocate_at(&self->h_addr,(uintptr_t)ptr,
                               &addr_semi,&addr_level);
 if (!pslot) {
  vm_vpage_t ptr_page;
  atomic_rwlock_endwrite(&self->h_lock);
  /* Not in cache. Try to allocate associated core memory. */
  ptr_page = VM_ADDR2PAGE((uintptr_t)ptr);
  if (!core_page_allocat(ptr_page,1,flags))
       return 0;
#ifdef CONFIG_DEBUG_HEAP
  memsetl((void *)VM_PAGE2ADDR(ptr_page),
           DEBUGHEAP_NO_MANS_LAND,PAGESIZE/4);
#endif
  /* Release the page to the heap and allocate again.
   * NOTE: Set the `GFP_NOMAP' to prevent the memory
   *       from be unmapped immediately. */
  heap_free_raw(self,(void *)VM_PAGE2ADDR(ptr_page),PAGESIZE,
                flags|GFP_NOMAP);
#if 1 /* ??? */
  if (vm_tryacquire(&vm_kernel)) {
   /* Try to optimize the VM by merging the new mapping with adjacent nodes. */
   vm_merge_before(&vm_kernel,ptr_page);
   vm_merge_before(&vm_kernel,ptr_page+1);
   vm_release(&vm_kernel);
  }
#endif
  goto again;
 }
 slot = *pslot;
 assert((uintptr_t)ptr >= MFREE_BEGIN(slot));
 if ((uintptr_t)ptr == MFREE_BEGIN(slot)) {
  /* Allocate this entire slot, then remove unused memory from the end. */
  asserte(mfree_tree_pop_at(pslot,addr_semi,addr_level) == slot);
  LIST_REMOVE(slot,mf_lsize);
  atomic_rwlock_endwrite(&self->h_lock);
  heap_validate_all();
  result     = slot->mf_size;
  slot_flags = (flags & __GFP_HEAPMASK) | slot->mf_flags;
#ifndef CONFIG_DEBUG_HEAP
  if ((slot_flags & GFP_CALLOC) && (flags & GFP_CALLOC))
      memset(slot,0,SIZEOF_MFREE);
#else
  if (flags & GFP_CALLOC)
       memset(slot,0,SIZEOF_MFREE);
  else mempatl(slot,DEBUGHEAP_NO_MANS_LAND,SIZEOF_MFREE);
#endif
 } else {
  size_t free_offset = (uintptr_t)ptr - MFREE_BEGIN(slot);
  assert(IS_ALIGNED(free_offset,HEAP_ALIGNMENT));
  if unlikely(free_offset < HEAP_MINSIZE) {
   /* The remaining part of the slot is too small.
    * Ask the core if it can allocate the the previous
    * page for us, so we can merge this slot with that
    * page to get a chance of leaving a part large enough
    * for us to return to the heap.
    * NOTE: If the slot doesn't start at a page boundary,
    *       we already know that the requested part has already
    *       been allocated (meaning this allocation is impossible) */
   vm_vpage_t slot_page;
   atomic_rwlock_endwrite(&self->h_lock);
   if (MFREE_BEGIN(slot) & (PAGESIZE-1))
       return 0; /* Not page-aligned. */
   slot_page = VM_PAGE2ADDR(MFREE_BEGIN(slot));
   if unlikely(slot_page == 0)
       return 0; /* Shouldn't happen: can't allocate previous page that doesn't exist. */
   if (!core_page_allocat(slot_page-1,1,flags & __GFP_HEAPMASK))
       return 0; /* Failed to allocate the associated core-page. */
   /* Free the page, so-as to try and merge it with the slot from before.
    * NOTE: Set the `GFP_NOMAP' to prevent the memory
    *       from be unmapped immediately. */
#ifdef CONFIG_DEBUG_HEAP
   memsetl((void *)VM_PAGE2ADDR(slot_page-1),
            DEBUGHEAP_NO_MANS_LAND,PAGESIZE/4);
#endif
   heap_free_raw(self,(void *)VM_PAGE2ADDR(slot_page-1),
                 PAGESIZE,(flags & __GFP_HEAPMASK)|GFP_NOMAP);
#if 1 /* ??? */
   if (vm_tryacquire(&vm_kernel)) {
    /* Try to optimize the VM by merging the new mapping with adjacent nodes. */
    vm_merge_before(&vm_kernel,slot_page-1);
    vm_merge_before(&vm_kernel,slot_page);
    vm_release(&vm_kernel);
   }
#endif
   goto again;
  }
  result = slot->mf_size-free_offset;
  if unlikely(result < HEAP_MINSIZE) {
   /* Too close to the back. - Try to allocate the next page. */
   uintptr_t slot_end = MFREE_END(slot);
   atomic_rwlock_endwrite(&self->h_lock);
   if (slot_end & (PAGESIZE-1))
       return 0; /* Not page-aligned. */
   if (!core_page_allocat(VM_ADDR2PAGE(slot_end),1,flags & __GFP_HEAPMASK))
       return 0; /* Failed to allocate the associated core-page. */
#ifdef CONFIG_DEBUG_HEAP
   memsetl((void *)slot_end,DEBUGHEAP_NO_MANS_LAND,PAGESIZE/4);
#endif
   heap_free_raw(self,(void *)slot_end,PAGESIZE,(flags & __GFP_HEAPMASK)|GFP_NOMAP);
   goto again;
  }
  asserte(mfree_tree_pop_at(pslot,addr_semi,addr_level) == slot);
  LIST_REMOVE(slot,mf_lsize);
#ifdef CONFIG_HEAP_TRACE_DANGLE
  /* Trace leading free data as dangling. */
  ATOMIC_FETCHADD(self->h_dangle,free_offset);
#endif /* CONFIG_HEAP_TRACE_DANGLE */
  atomic_rwlock_endwrite(&self->h_lock);
  heap_validate_all();
  slot_flags = (flags & __GFP_HEAPMASK) | slot->mf_flags;
  if (slot_flags & GFP_CALLOC)
      memset(slot,0,MIN(free_offset,SIZEOF_MFREE));
#ifdef CONFIG_DEBUG_HEAP
  else {
   mempatl(slot,DEBUGHEAP_NO_MANS_LAND,
           MIN(free_offset,SIZEOF_MFREE));
  }
#endif
  /* Release unused memory below the requested address. */
  heap_free_raw(self,(void *)MFREE_BEGIN(slot),
                free_offset,slot_flags);
#ifdef CONFIG_HEAP_TRACE_DANGLE
  ATOMIC_FETCHSUB(self->h_dangle,free_offset);
#endif /* CONFIG_HEAP_TRACE_DANGLE */
 }
 /* Initialize newly allocated memory according to what the caller wants. */
#ifdef CONFIG_DEBUG_HEAP
 if (!(flags & GFP_CALLOC))
       reset_heap_data((byte_t *)ptr,DEBUGHEAP_FRESH_MEMORY,result);
#endif
 if ((flags & GFP_CALLOC) && !(slot_flags & GFP_CALLOC))
      memset(ptr,0,result);
 assert(result >= HEAP_MINSIZE);
 return result;
}

PUBLIC size_t KCALL
heap_allat_untraced(struct heap *__restrict self,
                    VIRT void *__restrict ptr,
                    size_t num_bytes, gfp_t flags) {
 struct heap *EXCEPT_VAR xself = self;
 VIRT void *EXCEPT_VAR xptr = ptr;
 gfp_t EXCEPT_VAR xflags = flags;
 size_t unused_size,alloc_size;
 size_t EXCEPT_VAR result = 0;
 if unlikely(__builtin_add_overflow(num_bytes,HEAP_ALIGNMENT-1,&alloc_size))
    heap_allocation_overflow(num_bytes);
 alloc_size &= ~(HEAP_ALIGNMENT-1);
 if unlikely(alloc_size < HEAP_MINSIZE)
             alloc_size = HEAP_MINSIZE;
 /* Allocate memory from the given range. */
 while (result < alloc_size) {
  size_t COMPILER_IGNORE_UNINITIALIZED(part);
  /* Allocate the new missing part. */
  TRY {
   part = heap_allat_partial(self,
                            (void *)((uintptr_t)ptr + result),
                             flags);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   if (result)
       heap_free_untraced(xself,xptr,result,xflags);
   error_rethrow();
  }
  if unlikely(!part) {
   /* Failed to allocate the entirety of the requested range.
    * Free what was already allocated. */
   if (result)
       heap_free_untraced(self,ptr,result,flags);
   return 0;
  }
  result += part;
 }
 /* With everything now allocated, free what the caller didn't ask for. */
 assert(result >= alloc_size);
 unused_size = result - alloc_size;
 if (unused_size >= HEAP_MINSIZE) {
  heap_free_untraced(self,
                    (void *)((uintptr_t)ptr + alloc_size),
                     unused_size,flags);
  result = alloc_size;
 }
 return result;
}


PUBLIC struct heapptr KCALL
heap_align_untraced(struct heap *__restrict self,
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
 if (min_alignment <= HEAP_ALIGNMENT && !offset)
     return heap_alloc_untraced(self,num_bytes,flags);
 if unlikely(__builtin_add_overflow(num_bytes,HEAP_ALIGNMENT-1,&alloc_bytes))
    heap_allocation_overflow(num_bytes);
 alloc_bytes &= ~(HEAP_ALIGNMENT-1);
 if unlikely(alloc_bytes < HEAP_MINSIZE)
             alloc_bytes = HEAP_MINSIZE;
#if 1
 {
  struct heapptr result; struct mfree **iter,**end;
  iter = &self->h_size[HEAP_BUCKET_OF(alloc_bytes)];
  end  =  COMPILER_ENDOF(self->h_size);
  assertf(iter >= self->h_size &&
          iter <  COMPILER_ENDOF(self->h_size),
          "HEAP_BUCKET_OF(%Iu) = %Iu/%Iu",
          alloc_bytes,HEAP_BUCKET_OF(alloc_bytes),
          COMPILER_LENOF(self->h_size));
  heap_validate_all();
  atomic_rwlock_write(&self->h_lock);
  /* Search for existing free data that
   * fit the required alignment and offset. */
  for (; iter != end; ++iter) {
   struct mfree *chain; void *hkeep,*tkeep;
   gfp_t chain_flags; uintptr_t alignment_base;
   size_t hkeep_size,tkeep_size;
#ifdef CONFIG_HEAP_TRACE_DANGLE
   size_t dangle_size;
#endif /* CONFIG_HEAP_TRACE_DANGLE */
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
    while ((alignment_base-(uintptr_t)chain) < HEAP_MINSIZE)
        alignment_base += min_alignment;
   }
   /* Check if the node still contains enough memory for the requested allocation. */
   if ((alignment_base + alloc_bytes) > MFREE_END(chain))
       continue; /* The chain entry is too small once alignment was taken into consideration. */
   asserte(mfree_tree_remove(&self->h_addr,MFREE_BEGIN(chain)) == chain);
   LIST_REMOVE(chain,mf_lsize);
#ifdef CONFIG_HEAP_TRACE_DANGLE
   /* Trace potentially unused data as dangling. */
   dangle_size = chain->mf_size - alloc_bytes;
   ATOMIC_FETCHADD(self->h_dangle,dangle_size);
#endif /* CONFIG_HEAP_TRACE_DANGLE */
   atomic_rwlock_endwrite(&self->h_lock);
   /* All right! we can actually use this one! */
   result.hp_ptr = (VIRT void *)alignment_base;
   result.hp_siz = alloc_bytes;
   /* Figure out how much memory should be re-freed at the front and back. */
   chain_flags = chain->mf_flags;
   hkeep       = (void *)chain;
   hkeep_size  = alignment_base - (uintptr_t)chain;
   tkeep       = (void *)((uintptr_t)result.hp_ptr + alloc_bytes);
   tkeep_size  = MFREE_END(chain) - (uintptr_t)tkeep;
#ifdef CONFIG_HEAP_RANDOMIZE_OFFSETS
   if (tkeep_size) {
    /* Add a random offset to the resulting pointer. */
    uintptr_t random_offset;
    random_offset = rand() % tkeep_size;
    random_offset &= ~(min_alignment-1);
    /* Make sure to only add the offset if hkeep's size will be large enough! */
    if ((hkeep_size + random_offset) >= HEAP_MINSIZE) {
     hkeep_size                   += random_offset;
     tkeep_size                   -= random_offset;
     *(uintptr_t *)&tkeep         += random_offset;
     *(uintptr_t *)&result.hp_ptr += random_offset;
    }
   }
#endif

   if (hkeep_size) {
    assert(hkeep_size >= HEAP_MINSIZE);
    /* Reset data of the head if we're to re-free them. */
    if (chain_flags & GFP_CALLOC)
         memset(hkeep,0,SIZEOF_MFREE);
#ifdef CONFIG_DEBUG_HEAP
    else mempatl(hkeep,DEBUGHEAP_NO_MANS_LAND,SIZEOF_MFREE);
#endif
    /* Re-free the header pointer. */
    heap_free_raw(self,hkeep,hkeep_size,chain_flags);
   }
   if (tkeep_size < HEAP_MINSIZE) {
    /* The tail is too small (or non-existent).
     * -> We must allocate it, too. */
    result.hp_siz += tkeep_size;
   } else {
    /* Free the tail pointer. */
    heap_free_overallocation(self,
                             result.hp_ptr,
                             tkeep,
                             tkeep_size,
                            (flags&~(GFP_CALLOC))|chain_flags);
   }
#ifdef CONFIG_HEAP_TRACE_DANGLE
   /* Remove dangling data. */
   ATOMIC_FETCHSUB(self->h_dangle,dangle_size);
#endif /* CONFIG_HEAP_TRACE_DANGLE */
   /* Initialize the resulting memory. */
   if (flags&GFP_CALLOC) {
    if (chain_flags&MFREE_FZERO)
         memset(result.hp_ptr,0,SIZEOF_MFREE);
    else memset(result.hp_ptr,0,result.hp_siz);
   }
#ifdef CONFIG_DEBUG_HEAP
   else {
    reset_heap_data((byte_t *)result.hp_ptr,
                     DEBUGHEAP_FRESH_MEMORY,
                     result.hp_siz);
   }
#endif
   assert(IS_ALIGNED((uintptr_t)result.hp_ptr,HEAP_ALIGNMENT));
   assert(IS_ALIGNED((uintptr_t)result.hp_ptr+offset,min_alignment));
   assert(IS_ALIGNED((uintptr_t)result.hp_siz,HEAP_ALIGNMENT));
   assert(result.hp_siz >= HEAP_MINSIZE);
   return result;
  }
  atomic_rwlock_endwrite(&self->h_lock);
 }
#endif
 /* Fallback: Use overallocation to assert alignment. */

 /* Must overallocate by at least `HEAP_MINSIZE',
  * so we can _always_ free unused lower memory. */
 if unlikely(__builtin_add_overflow(alloc_bytes,min_alignment,&heap_alloc_bytes))
    heap_allocation_overflow(MAX(alloc_bytes,min_alignment));
 if unlikely(__builtin_add_overflow(heap_alloc_bytes,HEAP_MINSIZE,&heap_alloc_bytes))
    heap_allocation_overflow(heap_alloc_bytes);
 result_base = heap_alloc_untraced(self,heap_alloc_bytes,flags);
 assert(result_base.hp_siz >= heap_alloc_bytes);
 result.hp_ptr = (void *)(CEIL_ALIGN((uintptr_t)result_base.hp_ptr+
                                                HEAP_MINSIZE+offset,
                                      min_alignment)-offset);
 assert((uintptr_t)result.hp_ptr+alloc_bytes <=
        (uintptr_t)result_base.hp_ptr+result_base.hp_siz);
 nouse_size = (uintptr_t)result.hp_ptr-(uintptr_t)result_base.hp_ptr;
 assert(nouse_size+alloc_bytes <= result_base.hp_siz);
 assertf(nouse_size >= HEAP_MINSIZE,"nouse_size = %Iu",nouse_size);
 /* Release lower memory (This _MUST_ work because we've overallocated by `HEAP_MINSIZE'). */
 heap_free_untraced(self,result_base.hp_ptr,nouse_size,flags);
 result_base.hp_siz -= nouse_size;
 assert(result_base.hp_siz >= HEAP_MINSIZE);

 /* Try to release upper memory. */
 assert(result_base.hp_siz >= alloc_bytes);
 nouse_size = result_base.hp_siz-alloc_bytes;
 if (nouse_size >= HEAP_MINSIZE) {
  heap_free_untraced(self,
                    (void *)((uintptr_t)result.hp_ptr+alloc_bytes),
                     nouse_size,flags);
  result_base.hp_siz -= nouse_size;
 }
 assert(result_base.hp_siz >= alloc_bytes);
 assert(IS_ALIGNED((uintptr_t)result.hp_ptr+offset,min_alignment));
 result.hp_siz = result_base.hp_siz;
 assert(IS_ALIGNED((uintptr_t)result.hp_siz,HEAP_ALIGNMENT));
 assert(result.hp_siz >= HEAP_MINSIZE);
 return result;
}

PUBLIC struct heapptr KCALL
heap_realloc_untraced(struct heap *__restrict self,
                      VIRT void *old_ptr, size_t old_bytes,
                      size_t new_bytes, gfp_t alloc_flags,
                      gfp_t free_flags) {
 struct heap *EXCEPT_VAR xself = self;
 gfp_t EXCEPT_VAR xalloc_flags = alloc_flags;
 struct heapptr EXCEPT_VAR result; size_t missing_bytes;
 assert(IS_ALIGNED(old_bytes,HEAP_ALIGNMENT));
 assert(!old_bytes || IS_ALIGNED((uintptr_t)old_ptr,HEAP_ALIGNMENT));
 assert(!old_bytes || old_bytes >= HEAP_MINSIZE);
 if (old_bytes == 0) /* Special case: initial allocation */
     return heap_alloc_untraced(self,new_bytes,alloc_flags);
 new_bytes = CEIL_ALIGN(new_bytes,HEAP_ALIGNMENT);
 if unlikely(new_bytes < HEAP_MINSIZE)
             new_bytes = HEAP_MINSIZE;
 result.hp_ptr = old_ptr;
 result.hp_siz = old_bytes;
 if (new_bytes <= old_bytes) {
  size_t free_bytes;
  /* Free trailing memory. */
  free_bytes = old_bytes - new_bytes;
  if (free_bytes >= HEAP_MINSIZE) {
   heap_free_untraced(self,(void *)((uintptr_t)old_ptr+new_bytes),
                      free_bytes,free_flags);
   result.hp_siz = new_bytes;
  }
  return result;
 }
 missing_bytes = new_bytes - old_bytes;
 missing_bytes = heap_allat_untraced(self,(void *)((uintptr_t)old_ptr+old_bytes),
                                     missing_bytes,alloc_flags);
 if (missing_bytes) {
  /* Managed to extend the data block. */
  result.hp_siz += missing_bytes;
  return result;
 }
 /* Must allocate an entirely new data block and copy memory to it. */
 result = heap_alloc_untraced(self,new_bytes,alloc_flags);
 TRY {
  /* The try block is here because of the possibility of a LOA failure. */
  memcpy(result.hp_ptr,old_ptr,old_bytes);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  heap_free_untraced(xself,result.hp_ptr,result.hp_siz,
                     xalloc_flags & ~GFP_CALLOC);
  error_rethrow();
 }
 /* Free the old data block. */
 heap_free_untraced(self,old_ptr,old_bytes,
                    free_flags & ~GFP_CALLOC);
 return result;
}
PUBLIC struct heapptr KCALL
heap_realign_untraced(struct heap *__restrict self,
                      VIRT void *old_ptr, size_t old_bytes,
                      size_t min_alignment, ptrdiff_t offset,
                      size_t new_bytes, gfp_t alloc_flags,
                      gfp_t free_flags) {
 struct heap *EXCEPT_VAR xself = self;
 gfp_t EXCEPT_VAR xalloc_flags = alloc_flags;
 struct heapptr EXCEPT_VAR result; size_t missing_bytes;
 assert(IS_ALIGNED(old_bytes,HEAP_ALIGNMENT));
 assert(!old_bytes || IS_ALIGNED((uintptr_t)old_ptr,HEAP_ALIGNMENT));
 assert(!old_bytes || old_bytes >= HEAP_MINSIZE);
 if (old_bytes == 0) /* Special case: initial allocation */
     return heap_align_untraced(self,min_alignment,offset,new_bytes,alloc_flags);
 new_bytes = CEIL_ALIGN(new_bytes,HEAP_ALIGNMENT);
 result.hp_ptr = old_ptr;
 result.hp_siz = old_bytes;
 if (new_bytes <= old_bytes) {
  size_t free_bytes;
  if unlikely(new_bytes < HEAP_MINSIZE)
              new_bytes = HEAP_MINSIZE;
  /* Free trailing memory. */
  free_bytes = old_bytes - new_bytes;
  if (free_bytes >= HEAP_MINSIZE) {
   heap_free_untraced(self,(void *)((uintptr_t)old_ptr+new_bytes),
                      free_bytes,free_flags);
   result.hp_siz = new_bytes;
  }
  return result;
 }
 missing_bytes = new_bytes - old_bytes;
 missing_bytes = heap_allat_untraced(self,(void *)((uintptr_t)old_ptr+old_bytes),
                                     missing_bytes,alloc_flags);
 if (missing_bytes) {
  /* Managed to extend the data block. */
  result.hp_siz += missing_bytes;
  return result;
 }
 /* Must allocate an entirely new data block and copy memory to it. */
 result = heap_align_untraced(self,min_alignment,offset,new_bytes,alloc_flags);
 TRY {
  /* The try block is here because of the possibility of a LOA failure. */
  memcpy(result.hp_ptr,old_ptr,old_bytes);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  heap_free_untraced(xself,result.hp_ptr,result.hp_siz,
                     xalloc_flags & ~GFP_CALLOC);
  error_rethrow();
 }
 /* Free the old data block. */
 heap_free_untraced(self,old_ptr,old_bytes,
                    free_flags & ~GFP_CALLOC);
 return result;
}



/* Weakly alias the TRACED-versions of heap functions.
 * When DEBUG_MALLOC is enabled, `mall.c' will override these. */
DEFINE_PUBLIC_WEAK_ALIAS(heap_alloc,heap_alloc_untraced);
DEFINE_PUBLIC_WEAK_ALIAS(heap_align,heap_align_untraced);
DEFINE_PUBLIC_WEAK_ALIAS(heap_allat,heap_allat_untraced);
DEFINE_PUBLIC_WEAK_ALIAS(heap_free,heap_free_untraced);
DEFINE_PUBLIC_WEAK_ALIAS(heap_realloc,heap_realloc_untraced);
DEFINE_PUBLIC_WEAK_ALIAS(heap_realign,heap_realign_untraced);

DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_HEAP_C */
