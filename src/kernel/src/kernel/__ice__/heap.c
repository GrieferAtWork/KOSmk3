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

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/minmax.h>
#include <hybrid/align.h>
#include <kernel/malloc.h>
#include <kernel/heap.h>
#include <except.h>
#include <string.h>
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

// #undef NDEBUG
// #include <assert.h>

DECL_BEGIN

#if defined(NDEBUG) || 0
#define heap_validate(heap)     (void)0
#define heap_validate_all()     (void)0
#define heap_novalidate_begin() (void)0
#define heap_novalidate_end()   (void)0
#else
#ifndef CONFIG_ATOMIC_HEAP
#define CONFIG_HAVE_HEAP_NOVALIDATE 1
/* Required to prevent a problems that could arise when #PF handlers
 * try to validate the heap while the thread is manipulating the heap
 * in a way that has its state undefined at the moment. */
PRIVATE ATTR_PERTASK unsigned int heap_novalidate = 0;
#define heap_novalidate_begin()  ++PERTASK(heap_novalidate)
#define heap_novalidate_end()    --PERTASK(heap_novalidate)
#endif /* !CONFIG_ATOMIC_HEAP */
#endif

#define E_ACQUIRE_VM_LOCK   0x4003 /* Acquire a lock on `vm_kernel', then try again. */


#if PAGESIZE*16 > 65536
#define DEFAULT_OVERALLOC  (PAGESIZE*16)
#define DEFAULT_FREETHRESH (PAGESIZE*16)
#else
#define DEFAULT_OVERALLOC  (PAGESIZE*32)
#define DEFAULT_FREETHRESH (PAGESIZE*32)
#endif

PUBLIC struct mheap kernel_heaps[__GFP_HEAPCOUNT] = {
    /* Define the controller structures for the builtin kernel heaps. */
    [GFP_SHARED] = {
        .mh_overalloc  = DEFAULT_OVERALLOC,
        .mh_freethresh = DEFAULT_FREETHRESH,
        .mh_hintpage   = VM_KERNEL_SHAREDHEAP_HINT,
        .mh_hintmode   = VM_KERNEL_SHAREDHEAP_MODE
    },
    [GFP_KERNEL] = {
        .mh_overalloc  = DEFAULT_OVERALLOC,
        .mh_freethresh = DEFAULT_FREETHRESH,
        .mh_hintpage   = VM_KERNEL_KERNELHEAP_HINT,
        .mh_hintmode   = VM_KERNEL_KERNELHEAP_MODE
    },
    [GFP_SHARED | GFP_LOCKED] = {
        .mh_overalloc  = DEFAULT_OVERALLOC / 4,
        .mh_freethresh = DEFAULT_FREETHRESH / 4,
        .mh_hintpage   = VM_KERNEL_SHAREDHEAP_LOCKED_HINT,
        .mh_hintmode   = VM_KERNEL_SHAREDHEAP_LOCKED_MODE
    },
    [GFP_KERNEL | GFP_LOCKED] = {
        .mh_overalloc  = DEFAULT_OVERALLOC / 4,
        .mh_freethresh = DEFAULT_FREETHRESH / 4,
        .mh_hintpage   = VM_KERNEL_KERNELHEAP_LOCKED_HINT,
        .mh_hintmode   = VM_KERNEL_KERNELHEAP_LOCKED_MODE
    },
};






/* HEAP Debug functions. */
#ifndef NDEBUG
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

PRIVATE byte_t *KCALL
find_modified_address(byte_t *start, size_t num_bytes, u32 dword) {
 while ((uintptr_t)start & 3) {
  if (!num_bytes) return NULL;
  if (*(u8 *)start != ((u8 *)&dword)[(uintptr_t)start & 3])
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
  if (*(u32 *)start == dword) {
   start     += 4;
   num_bytes -= 4;
   continue;
  }
  if (((u8 *)start)[0] != ((u8 *)&dword)[0]) return start + 0;
  if (((u8 *)start)[1] != ((u8 *)&dword)[1]) return start + 1;
  if (((u8 *)start)[2] != ((u8 *)&dword)[2]) return start + 2;
  return start + 3;
 }
 while (num_bytes) {
  if (*(u8 *)start != ((u8 *)&dword)[(uintptr_t)start & 3])
      return start;
  --num_bytes,++start;
 }
 return NULL;
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
#ifndef NDEBUG
 struct mheap *heap;
 if (THIS_VM == &vm_kernel) {
  /* Validate all heaps. */
  for (heap  = kernel_heaps;
       heap != COMPILER_ENDOF(kernel_heaps); ++heap)
       heap_validate(heap);
 } else {
  /* Only shared heaps are visible, so only validate those. */
  heap_validate(&kernel_heaps[GFP_SHARED]);
  heap_validate(&kernel_heaps[GFP_SHARED|GFP_LOCKED]);
 }
#endif
}

PUBLIC void (KCALL heap_validate)(struct mheap *__restrict heap) {
#ifndef NDEBUG
 unsigned int i;
#ifdef CONFIG_HAVE_HEAP_NOVALIDATE
 if (PERTASK(heap_novalidate))
     return;
#endif

#ifdef CONFIG_ATOMIC_HEAP
 if (!atomic_rwlock_tryread(&heap->mh_lock))
      return;
#else
 if (!mutex_try(&heap->mh_lock))
      return;
 task_nothrow_serve();
 TRY {
#endif
 for (i = 0; i < COMPILER_LENOF(heap->mh_size); ++i) {
  struct mfree **piter,*iter;
  piter = &heap->mh_size[i];
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
           iter,iter->mf_size,i,COMPILER_LENOF(heap->mh_size));
   assertf((uintptr_t)iter+iter->mf_size > (uintptr_t)iter,
           "\tPotential USE-AFTER-FREE of <%p...%p>\n"
           "Free node at %p is too large (%Iu bytes) (size bucket %Iu/%Iu)\n",
          &iter->mf_size,(uintptr_t)&iter->mf_size+sizeof(size_t)-1,
           iter,iter->mf_size,i,COMPILER_LENOF(heap->mh_size));
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
   faulting_address = find_modified_address(iter->mf_data,
                                           (iter->mf_size-SIZEOF_MFREE)/4,
                                            expected_data);
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
#ifdef CONFIG_ATOMIC_HEAP
 atomic_rwlock_endread(&heap->mh_lock);
#else
 } FINALLY {
  mutex_put(&heap->mh_lock);
 }
 task_nothrow_end();
#endif
#else
 (void)heap;
#endif
}

#ifndef NDEBUG
PRIVATE void KCALL
reset_heap_data(byte_t *ptr, u32 pattern, size_t num_bytes) {
 if (num_bytes < PAGESIZE)
     goto do_remainder;
 /* Only write pages that have been allocated. */
 if ((uintptr_t)ptr & (PAGESIZE-1)) {
  size_t inpage_free = PAGESIZE - ((uintptr_t)ptr & (PAGESIZE-1));
  if (inpage_free > num_bytes)
      inpage_free = num_bytes;
#if HEAP_ALIGNMENT >= 4
  memsetl(ptr,pattern,inpage_free/4);
#else
  mempatl(ptr,pattern,inpage_free);
#endif
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
#if HEAP_ALIGNMENT >= 4
  memsetl(ptr,pattern,num_bytes/4);
#else
  mempatl(ptr,pattern,num_bytes);
#endif
 }
}
#endif


#ifndef CONFIG_ATOMIC_HEAP
PRIVATE void KCALL must_acquire_vm_lock(void) {
 debug_printf("must_acquire_vm_lock()\n");
 error_throw(E_ACQUIRE_VM_LOCK);
}
#endif /* !CONFIG_ATOMIC_HEAP */


PRIVATE VIRT vm_vpage_t KCALL
core_page_alloc(struct mheap *__restrict self,
                size_t num_pages, gfp_t flags) {
 struct vm_corepair corepair;
 vm_vpage_t result;
 assert(num_pages != 0);
 /* Throw a would-block error if we're not allowed to map new memory. */
 if (flags&GFP_NOMAP)
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
  corepair.cp_region->vr_init = VM_REGION_INIT_FILLER;
 }
#ifndef NDEBUG
 else {
  corepair.cp_region->vr_init           = VM_REGION_INIT_FILLER;
  corepair.cp_region->vr_setup.s_filler = DEBUGHEAP_FRESH_MEMORY;
 }
#endif
 corepair.cp_region->vr_part0.vp_refcnt = 1;
 corepair.cp_region->vr_parts = &corepair.cp_region->vr_part0;
 corepair.cp_region->vr_part0.vp_chain.le_pself = &corepair.cp_region->vr_parts;
 if (flags&GFP_LOCKED) {
  /* Allocate memory immediately, as direct, physical memory. */
  TRY {
   corepair.cp_region->vr_type                                 = VM_REGION_PHYSICAL;
   corepair.cp_region->vr_part0.vp_state                       = VM_PART_INCORE;
   corepair.cp_region->vr_part0.vp_phys.py_num_scatter         = 1;
   corepair.cp_region->vr_part0.vp_phys.py_iscatter[0].ps_size = num_pages; /* TODO: Use scatter */
   corepair.cp_region->vr_part0.vp_phys.py_iscatter[0].ps_addr = page_malloc(num_pages,MZONE_ANY);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   vm_region_destroy(corepair.cp_region);
   vm_node_free(corepair.cp_node);
   error_rethrow();
  }
 }

 TRY {
  if (!mutex_try(&vm_kernel.vm_lock)) {
#ifdef CONFIG_ATOMIC_HEAP
   if (flags & GFP_ATOMIC)
       error_throw(E_WOULDBLOCK);
   mutex_get(&vm_kernel.vm_lock);
#else
   must_acquire_vm_lock();
#endif
  }
  TRY {
   vm_vpage_t pagehint;
   /* Find a suitable, free location, using heap-specific hints. */
   assert(!(flags&GFP_KERNEL) || THIS_VM == &vm_kernel);
   pagehint = self->mh_hintpage;
   result   = vm_getfree(pagehint,num_pages,1,0,self->mh_hintmode);
   /* Save the new hint. */
   ATOMIC_CMPXCH(self->mh_hintpage,pagehint,result);
   corepair.cp_node->vn_node.a_vmin = result;
   corepair.cp_node->vn_node.a_vmax = result+num_pages-1;
   /* Register the region. */
   vm_region_track(corepair.cp_node->vn_region);
   /* Finally, map the corepair */
   vm_insert_and_activate_node(&vm_kernel,corepair.cp_node);
#ifndef CONFIG_ATOMIC_HEAP
   /* Try to optimize the VM by merging the new mapping with adjacent nodes. */
   vm_merge_before(&vm_kernel,result);
   vm_merge_before(&vm_kernel,result+num_pages);
#endif
  } FINALLY {
   mutex_put(&vm_kernel.vm_lock);
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
#ifndef NDEBUG
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
 struct vm_corepair corepair;
 assert(num_pages != 0);
 /* Throw a would-block error if we're not allowed to map new memory. */
 if (flags&GFP_NOMAP)
     error_throw(E_WOULDBLOCK);
 if (!mutex_try(&vm_kernel.vm_lock)) {
#ifdef CONFIG_ATOMIC_HEAP
  if (flags & GFP_ATOMIC)
      error_throw(E_WOULDBLOCK);
  mutex_get(&vm_kernel.vm_lock);
#else
  must_acquire_vm_lock();
#endif
 }
 TRY {
  /* Check if the kernel VM already contains a mapping at the given address. */
  if (vm_getanynode(page_index,page_index+num_pages)) {
   mutex_put(&vm_kernel.vm_lock);
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
   if (flags&GFP_CALLOC) {
    corepair.cp_region->vr_init = VM_REGION_INIT_FILLER;
   }
#ifndef NDEBUG
   else {
    corepair.cp_region->vr_init           = VM_REGION_INIT_FILLER;
    corepair.cp_region->vr_setup.s_filler = DEBUGHEAP_FRESH_MEMORY;
   }
#endif
   corepair.cp_region->vr_part0.vp_refcnt = 1;
   corepair.cp_region->vr_parts = &corepair.cp_region->vr_part0;
   corepair.cp_region->vr_part0.vp_chain.le_pself = &corepair.cp_region->vr_parts;
   if (flags&GFP_LOCKED) {
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
   /* Finally, map the corepair */
   vm_insert_and_activate_node(&vm_kernel,corepair.cp_node);
  } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   vm_region_destroy(corepair.cp_region);
   vm_node_free(corepair.cp_node);
   error_rethrow();
  }

#ifndef CONFIG_ATOMIC_HEAP
  /* Try to optimize the VM by merging the new mapping with adjacent nodes. */
  vm_merge_before(&vm_kernel,page_index);
  vm_merge_before(&vm_kernel,page_index+num_pages);
#endif
 } FINALLY {
  mutex_put(&vm_kernel.vm_lock);
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
#ifndef NDEBUG
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
#ifdef CONFIG_ATOMIC_HEAP
 /* XXX: free() should be atomic, but this could block... */
 mutex_get(&vm_kernel.vm_lock);
#else
 (void)flags; /* XXX: Make use of `flags'? */
 if (!mutex_try(&vm_kernel.vm_lock))
      must_acquire_vm_lock();
#endif
 vm_unmap(page_index,num_pages,
          VM_UNMAP_NOEXCEPT,NULL);
 mutex_put(&vm_kernel.vm_lock);
}



INTERN ATTR_NORETURN void KCALL
heap_allocation_overflow(size_t n_bytes) {
 struct exception_info *info;
 /* Throw a bad-allocation error */
 info         = error_info();
 info->e_code = E_BADALLOC;
 info->e_flag = ERR_FAFTERFAULT;
 memset(info->__e_data,0,sizeof(info->__e_data));
 info->e_badalloc.ba_amount   = n_bytes;
 info->e_badalloc.ba_resource = RESOURCE_TYPE_VIRTMEMORY;
 error_throw_current();
 __builtin_unreachable();
}

#ifdef CONFIG_ATOMIC_HEAP


LOCAL ATTR_NOTHROW void KCALL
heap_insert_node_unlocked(struct mheap *__restrict self,
                          struct mfree *__restrict node) {
 struct mfree **pslot,*slot;
 size_t num_bytes = node->mf_size;
 /* Insert the node into the address and size trees. */
 mfree_tree_insert(&self->mh_addr,node);
 /* Figure out where the free-slot should go in the chain of free ranges. */
 pslot = &self->mh_size[HEAP_BUCKET_OF(num_bytes)];
 while ((slot = *pslot) != NULL &&
         MFREE_SIZE(slot) < num_bytes)
         pslot = &slot->mf_lsize.le_next;
 node->mf_lsize.le_pself = pslot;
 node->mf_lsize.le_next  = slot;
 if (slot) slot->mf_lsize.le_pself = &node->mf_lsize.le_next;
 *pslot = node;
}


PRIVATE ATTR_NOTHROW void KCALL
heap_free_raw(struct mheap *__restrict self,
              VIRT void *ptr, size_t num_bytes,
              gfp_t flags) {
 ATREE_SEMI_T(uintptr_t) addr_semi;
 ATREE_LEVEL_T addr_level;
 struct mfree **pslot,*slot,*new_slot;
 assertf(num_bytes >= HEAP_MINSIZE,"Invalid heap_free(): Too few bytes (%Iu < %Iu)",num_bytes,HEAP_MINSIZE);
 assertf(IS_ALIGNED((uintptr_t)ptr,HEAP_ALIGNMENT),"Invalid heap_free(): Unaligned base pointer %p",ptr);
 assertf(IS_ALIGNED(num_bytes,HEAP_ALIGNMENT),"Invalid heap_free(): Unaligned free size %Iu (%#Ix)",num_bytes,num_bytes);
 assertf(((uintptr_t)ptr + num_bytes) > (uintptr_t)ptr,"Address space overflow when freeing %p...%p",ptr,(uintptr_t)ptr+num_bytes-1);
again:
#if !defined(NDEBUG) && 0
 {
  byte_t *iter,*end;
  end = (iter = (byte_t *)ptr)+num_bytes;
  if (flags & GFP_CALLOC) {
   for (; iter != end; ++iter) {
    if (*iter == 0) continue;
    PREEMPTION_DISABLE();
    format_hexdump(&debug_printer,NULL,ptr,num_bytes,16,FORMAT_HEXDUMP_FLAG_ADDRESS);
    debug_printf("\n");
    assertf(0,"Bad byte at %p in %p...%p",iter,ptr,end-1);
   }
  } else {
   u32 pattern = DEBUGHEAP_NO_MANS_LAND;
   for (; iter != end; ++iter) {
    if (*iter == ((u8 *)&pattern)[(uintptr_t)iter & 3]) continue;
    PREEMPTION_DISABLE();
    format_hexdump(&debug_printer,NULL,ptr,num_bytes,16,FORMAT_HEXDUMP_FLAG_ADDRESS);
    debug_printf("\n");
    assertf(0,"Bad byte at %p in %p...%p",iter,ptr,end-1);
   }
  }
 }
#endif
 heap_validate_all();
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);

 atomic_rwlock_write(&self->mh_lock);
 /* Search for a node below, that could then be extended above. */
 pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)ptr-1,
                               &addr_semi,&addr_level);
 if (pslot) {
  slot = *pslot;
  asserte(mfree_tree_pop_at(pslot,addr_semi,addr_level) == slot);
  LIST_REMOVE(slot,mf_lsize);
  /* Extend this node above. */
#ifdef NDEBUG
  slot->mf_flags &= flags & GFP_CALLOC;
  slot->mf_size  += num_bytes;
#else
  if ((flags & GFP_CALLOC) && (slot->mf_flags & GFP_CALLOC)) {
   slot->mf_size += num_bytes;
  } else {
   atomic_rwlock_endwrite(&self->mh_lock);
   heap_validate_all();
   if (flags & GFP_CALLOC)
       mempatl(ptr,DEBUGHEAP_NO_MANS_LAND,num_bytes),
       flags &= ~GFP_CALLOC;
   heap_validate_all();
   if (slot->mf_flags & GFP_CALLOC)
       mempatl((void *)((uintptr_t)slot+SIZEOF_MFREE),
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
  pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)ptr+num_bytes,
                                &addr_semi,&addr_level);
  if unlikely(pslot) {
   struct mfree *high_slot;
   high_slot = *pslot;
   /* Include this high-slot in the union that will be freed. */
   asserte(mfree_tree_pop_at(pslot,addr_semi,addr_level) == high_slot);
   LIST_REMOVE(high_slot,mf_lsize);
#ifdef NDEBUG
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
    atomic_rwlock_endwrite(&self->mh_lock);
    heap_validate_all();
    if (slot->mf_flags & GFP_CALLOC)
        mempatl((void *)((uintptr_t)slot+SIZEOF_MFREE),
                 DEBUGHEAP_NO_MANS_LAND,slot->mf_size-SIZEOF_MFREE);
    heap_validate_all();
    if (high_slot->mf_flags & GFP_CALLOC)
        mempatl((void *)((uintptr_t)high_slot+SIZEOF_MFREE),
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
  mfree_set_checksum(slot);
  heap_insert_node_unlocked(self,slot);
  atomic_rwlock_endwrite(&self->mh_lock);
  heap_validate_all();
  return;
 }

 /* Search for a node above, that could then be extended below. */
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);
 pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)ptr+num_bytes,
                               &addr_semi,&addr_level);
 if (pslot) {
  struct mfree *new_slot;
  gfp_t slot_flags; size_t slot_size;
  slot = *pslot;
  asserte(mfree_tree_pop_at(pslot,addr_semi,addr_level) == slot);
  LIST_REMOVE(slot,mf_lsize);
  /* Extend this node below. */

#ifdef NDEBUG
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
   atomic_rwlock_endwrite(&self->mh_lock);
   if (flags & GFP_CALLOC)
       mempatl(ptr,DEBUGHEAP_NO_MANS_LAND,num_bytes);
   if (slot_flags & GFP_CALLOC)
        mempatl(slot,DEBUGHEAP_NO_MANS_LAND,slot_size);
   else mempatl(slot,DEBUGHEAP_NO_MANS_LAND,SIZEOF_MFREE);
   flags     &= ~GFP_CALLOC;
   num_bytes += slot_size;
   goto again;
  }
#endif


  /* Re-insert our new, bigger node. */
  mfree_set_checksum(new_slot);
  heap_insert_node_unlocked(self,new_slot);
  atomic_rwlock_endwrite(&self->mh_lock);
  heap_validate_all();
  return;
 }

 /* Create a new node. */
 new_slot = (struct mfree *)ptr;
 new_slot->mf_size  = num_bytes;
 new_slot->mf_flags = flags & MFREE_FMASK;
 mfree_set_checksum(new_slot);
 /* Insert the node into the address and size trees. */
 heap_insert_node_unlocked(self,new_slot);
 /* Done! */
 atomic_rwlock_endwrite(&self->mh_lock);
 heap_validate_all();
}

PUBLIC ATTR_NOTHROW void KCALL
heap_free(struct mheap *__restrict self,
          VIRT void *ptr, size_t num_bytes,
          gfp_t flags) {
 assertf(num_bytes >= HEAP_MINSIZE,"Invalid heap_free(): Too few bytes (%Iu < %Iu)",num_bytes,HEAP_MINSIZE);
 assertf(IS_ALIGNED((uintptr_t)ptr,HEAP_ALIGNMENT),"Invalid heap_free(): Unaligned base pointer %p",ptr);
 assertf(IS_ALIGNED(num_bytes,HEAP_ALIGNMENT),"Invalid heap_free(): Unaligned free size %Iu (%#Ix)",num_bytes,num_bytes);
 /* Reset debug information. */
#ifndef NDEBUG
 if (!(flags & GFP_CALLOC))
     reset_heap_data((byte_t *)ptr,DEBUGHEAP_NO_MANS_LAND,num_bytes);
#endif
 heap_free_raw(self,ptr,num_bytes,flags);
}




PUBLIC struct heapptr KCALL
heap_alloc(struct mheap *__restrict self,
           size_t num_bytes, gfp_t flags) {
 struct heapptr result; struct mfree **iter,**end;
 if unlikely(__builtin_add_overflow(num_bytes,HEAP_ALIGNMENT-1,&result.hp_siz))
    heap_allocation_overflow(num_bytes);
 result.hp_siz &= ~(HEAP_ALIGNMENT-1);
 if unlikely(result.hp_siz < HEAP_MINSIZE)
             result.hp_siz = HEAP_MINSIZE;
 iter = &self->mh_size[HEAP_BUCKET_OF(result.hp_siz)];
 end  =  COMPILER_ENDOF(self->mh_size);
 assertf(iter >= self->mh_size &&
         iter <  COMPILER_ENDOF(self->mh_size),
         "HEAP_BUCKET_OF(%Iu) = %Iu/%Iu",
         result.hp_siz,HEAP_BUCKET_OF(result.hp_siz),
         COMPILER_LENOF(self->mh_size));
 heap_validate_all();
 atomic_rwlock_write(&self->mh_lock);
 for (; iter != end; ++iter) {
  size_t unused_size;
  struct mfree *chain;
  gfp_t chain_flags;
  /* Search this bucket. */
  chain = *iter;
  while (chain && MFREE_SIZE(chain) < result.hp_siz)
         chain = chain->mf_lsize.le_next;
  if (!chain) continue;
  asserte(mfree_tree_remove(&self->mh_addr,MFREE_BEGIN(chain)) == chain);
  LIST_REMOVE(chain,mf_lsize);
  atomic_rwlock_endwrite(&self->mh_lock);
  heap_validate_all();
  /* We've got the memory! */
  result.hp_ptr = (void *)chain;
  chain_flags = chain->mf_flags;
  unused_size = MFREE_SIZE(chain)-result.hp_siz;
  if (unused_size < HEAP_MINSIZE) {
   /* Remainder is too small. - Allocate it as well. */
   result.hp_siz += unused_size;
  } else {
   void *unused_begin = (void *)((uintptr_t)chain+result.hp_siz);
   /* Free the unused portion. */
   assert(IS_ALIGNED((uintptr_t)unused_begin,HEAP_ALIGNMENT));
   assert(IS_ALIGNED(unused_size,HEAP_ALIGNMENT));
   assert(unused_size < MFREE_SIZE(chain));
   heap_free_raw(self,unused_begin,unused_size,
                (flags&~(GFP_CALLOC))|chain_flags);
  }
  /* Initialize the result memory. */
  if (flags&GFP_CALLOC) {
   if (chain_flags&MFREE_FZERO)
        memset(result.hp_ptr,0,SIZEOF_MFREE);
   else memset(result.hp_ptr,0,result.hp_siz);
  }
#ifndef NDEBUG
  else {
   reset_heap_data((byte_t *)result.hp_ptr,
                    DEBUGHEAP_FRESH_MEMORY,
                    result.hp_siz);
  }
#endif
  assert(IS_ALIGNED((uintptr_t)result.hp_ptr,HEAP_ALIGNMENT));
  assert(IS_ALIGNED((uintptr_t)result.hp_siz,HEAP_ALIGNMENT));
  return result;
 }
 atomic_rwlock_endwrite(&self->mh_lock);

 /* No pre-allocated memory found. -> Allocate new memory. */
 {
  vm_vpage_t pageaddr; size_t page_bytes,unused_size;
  if unlikely(__builtin_add_overflow(result.hp_siz,self->mh_overalloc,&page_bytes))
     goto allocate_without_overalloc;
  if unlikely(__builtin_add_overflow(result.hp_siz,PAGESIZE-1,&page_bytes))
     goto allocate_without_overalloc;
  page_bytes &= ~(PAGESIZE-1);
  TRY {
   pageaddr = core_page_alloc(self,page_bytes/PAGESIZE,flags);
  } CATCH(E_BADALLOC) {
allocate_without_overalloc:
   /* Try again without overallocation. */
   if unlikely(__builtin_add_overflow(result.hp_siz,PAGESIZE-1,&page_bytes))
      heap_allocation_overflow(result.hp_siz);
   page_bytes &= ~(PAGESIZE-1);
   pageaddr = core_page_alloc(self,page_bytes/PAGESIZE,flags);
  }
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
#ifndef NDEBUG
   if (!(flags&GFP_CALLOC))
         mempatl(unused_begin,DEBUGHEAP_NO_MANS_LAND,unused_size);
#endif
   /* Release the unused memory. */
   heap_free_raw(self,unused_begin,unused_size,flags);
  }
 }
 return result;
}

PRIVATE size_t KCALL
heap_alloc_at_partial(struct mheap *__restrict self,
                      VIRT void *__restrict ptr, gfp_t flags) {
 ATREE_SEMI_T(uintptr_t) addr_semi;
 ATREE_LEVEL_T addr_level; gfp_t slot_flags;
 size_t result; struct mfree **pslot,*slot;
 assert(IS_ALIGNED((uintptr_t)ptr,HEAP_ALIGNMENT));
again:
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);

 /* Check if the requested address is in cache. */
 atomic_rwlock_write(&self->mh_lock);
 pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)ptr,
                               &addr_semi,&addr_level);
 if (!pslot) {
  vm_vpage_t ptr_page;
  atomic_rwlock_endwrite(&self->mh_lock);
  /* Not in cache. Try to allocate associated core memory. */
  ptr_page = VM_ADDR2PAGE((uintptr_t)ptr);
  if (!core_page_allocat(ptr_page,1,flags))
       return 0;
  /* Release the page to the heap and allocate again. */
  heap_free_raw(self,(void *)VM_PAGE2ADDR(ptr_page),PAGESIZE,flags);
  goto again;
 }
 slot = *pslot;
 assert((uintptr_t)ptr >= MFREE_BEGIN(slot));
 if ((uintptr_t)ptr == MFREE_BEGIN(slot)) {
  /* Allocate this entire slot, then remove unused memory from the end. */
  asserte(mfree_tree_pop_at(pslot,addr_semi,addr_level) == slot);
  LIST_REMOVE(slot,mf_lsize);
  atomic_rwlock_endwrite(&self->mh_lock);
  heap_validate_all();
  result     = slot->mf_size;
  slot_flags = (flags & __GFP_HEAPMASK) | slot->mf_flags;
#ifdef NDEBUG
  if ((slot_flags & GFP_CALLOC) && (flags & GFP_CALLOC))
      memset(slot,0,SIZEOF_MFREE);
#else
  mempatl(slot,(flags & GFP_CALLOC) ? 0 : DEBUGHEAP_NO_MANS_LAND,SIZEOF_MFREE);
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
   atomic_rwlock_endwrite(&self->mh_lock);
   if (MFREE_BEGIN(slot) & (PAGESIZE-1))
       return 0; /* Not page-aligned. */
   slot_page = VM_PAGE2ADDR(MFREE_BEGIN(slot));
   if unlikely(slot_page == 0)
       return 0; /* Shouldn't happen: can't allocate previous page that doesn't exist. */
   if (!core_page_allocat(slot_page-1,1,flags & __GFP_HEAPMASK))
       return 0; /* Failed to allocate the associated core-page. */
   /* Free the page, so-as to try and merge it with the slot from before. */
   heap_free_raw(self,(void *)VM_PAGE2ADDR(slot_page-1),
                 PAGESIZE,flags & __GFP_HEAPMASK);
   goto again;
  }
  asserte(mfree_tree_pop_at(pslot,addr_semi,addr_level) == slot);
  LIST_REMOVE(slot,mf_lsize);
  atomic_rwlock_endwrite(&self->mh_lock);
  heap_validate_all();
  result      = slot->mf_size-free_offset;
  slot_flags  = (flags & __GFP_HEAPMASK) | slot->mf_flags;
#ifdef NDEBUG
  if (slot_flags & GFP_CALLOC)
      memset(slot,0,MIN(free_offset,SIZEOF_MFREE));
#else
  memset(slot,(slot_flags&GFP_CALLOC) ? 0 : DEBUGHEAP_NO_MANS_LAND,
         MIN(free_offset,SIZEOF_MFREE));
#endif
  /* Release unused memory below the requested address. */
  heap_free_raw(self,(void *)MFREE_BEGIN(slot),
                free_offset,slot_flags);
 }
 /* Initialize newly allocated memory according to what the caller wants. */
#ifndef NDEBUG
 if (!(flags & GFP_CALLOC))
       reset_heap_data((byte_t *)ptr,DEBUGHEAP_FRESH_MEMORY,result);
#endif
 if ((flags & GFP_CALLOC) && !(slot_flags & GFP_CALLOC))
      memset(ptr,0,result);
 return result;
}

PUBLIC size_t KCALL
heap_alloc_at(struct mheap *__restrict self,
              VIRT void *__restrict ptr,
              size_t num_bytes, gfp_t flags) {
 size_t unused_size,alloc_size,result = 0;
 if unlikely(__builtin_add_overflow(num_bytes,HEAP_ALIGNMENT-1,&alloc_size))
    heap_allocation_overflow(num_bytes);
 alloc_size &= ~(HEAP_ALIGNMENT-1);
 if unlikely(alloc_size < HEAP_MINSIZE)
             alloc_size = HEAP_MINSIZE;
 /* Allocate memory from the given range. */
 while (result < alloc_size) {
  size_t part;
  /* Allocate the new missing part. */
  TRY {
   part = heap_alloc_at_partial(self,
                               (void *)((uintptr_t)ptr + result),
                                flags);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   if (result)
       heap_free(self,ptr,result,flags);
   error_rethrow();
  }
  if unlikely(!part) {
   /* Failed to allocate the entirety of the requested range.
    * Free what was already allocated. */
   if (result)
       heap_free(self,ptr,result,flags);
   return 0;
  }
  result += part;
 }
 /* With everything now allocated, free what the caller didn't ask for. */
 assert(result >= alloc_size);
 unused_size = result - alloc_size;
 if (unused_size >= HEAP_MINSIZE) {
  heap_free(self,
           (void *)((uintptr_t)ptr + alloc_size),
            unused_size,flags);
  result = alloc_size;
 }
 return result;
}


PUBLIC struct heapptr KCALL
heap_align(struct mheap *__restrict self,
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
     return heap_alloc(self,num_bytes,flags);
 if unlikely(__builtin_add_overflow(num_bytes,HEAP_ALIGNMENT-1,&alloc_bytes))
    heap_allocation_overflow(num_bytes);
 alloc_bytes &= ~(HEAP_ALIGNMENT-1);
 if unlikely(alloc_bytes < HEAP_MINSIZE)
             alloc_bytes = HEAP_MINSIZE;
 /* Must overallocate by at least `HEAP_MINSIZE',
  * so we can _always_ free unused lower memory. */
 if unlikely(__builtin_add_overflow(alloc_bytes,min_alignment,&heap_alloc_bytes))
    heap_allocation_overflow(MAX(alloc_bytes,min_alignment));
 if unlikely(__builtin_add_overflow(heap_alloc_bytes,HEAP_MINSIZE,&heap_alloc_bytes))
    heap_allocation_overflow(heap_alloc_bytes);
 result_base = heap_alloc(self,heap_alloc_bytes,flags);
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
 heap_free(self,result_base.hp_ptr,nouse_size,flags);
 result_base.hp_siz -= nouse_size;

 /* Try to release upper memory. */
 assert(result_base.hp_siz >= alloc_bytes);
 nouse_size = result_base.hp_siz-alloc_bytes;
 if (nouse_size >= HEAP_MINSIZE) {
  heap_free(self,
           (void *)((uintptr_t)result.hp_ptr+alloc_bytes),
            nouse_size,flags);
  result_base.hp_siz -= nouse_size;
 }
 assert(result_base.hp_siz >= alloc_bytes);
 assert(IS_ALIGNED((uintptr_t)result.hp_ptr+offset,min_alignment));
 result.hp_siz = result_base.hp_siz;
 return result;
}


#else

PRIVATE ATTR_NOTHROW void KCALL
mheap_release_nomerge(struct mheap *__restrict self, void *p,
                      size_t n_bytes, gfp_t flags) {
 struct mfree **piter,*iter;
 CHECK_HOST_DATA(p,n_bytes);
 assert(IS_ALIGNED((uintptr_t)p,HEAP_ALIGNMENT));
 assert(IS_ALIGNED((uintptr_t)n_bytes,HEAP_ALIGNMENT));
 assert(n_bytes >= HEAP_MINSIZE);
#define NEW_SLOT  ((struct mfree *)p)
 NEW_SLOT->mf_flags = flags & MFREE_FZERO;
 NEW_SLOT->mf_size  = n_bytes;
 mfree_set_checksum(NEW_SLOT);
 mfree_tree_insert(&self->mh_addr,NEW_SLOT);
 /* Figure out where the free-slot should go in the chain of free ranges. */
 piter = &self->mh_size[HEAP_BUCKET_OF(n_bytes)];
 while ((iter = *piter) != NULL &&
         MFREE_SIZE(iter) < n_bytes)
         piter = &iter->mf_lsize.le_next;
 NEW_SLOT->mf_lsize.le_pself = piter;
 NEW_SLOT->mf_lsize.le_next  = iter;
 if (iter) iter->mf_lsize.le_pself = &NEW_SLOT->mf_lsize.le_next;
 *piter = NEW_SLOT;
#undef NEW_SLOT
}
PRIVATE ATTR_NOTHROW void KCALL
mheap_unmapfree(struct mheap *__restrict self,
                struct mfree **__restrict pslot,
                ATREE_SEMI_T(uintptr_t) addr_semi,
                ATREE_LEVEL_T addr_level, gfp_t flags) {
 struct mfree *slot;
 assert(mutex_holding(&self->mh_lock));
 /* If the NOMAP flag is set, don't map/unmap additional memory. */
 if (flags&GFP_NOMAP) return;

 CHECK_HOST_DOBJ(pslot);
 slot = *pslot;
 CHECK_HOST_DOBJ(slot);
 assert(IS_ALIGNED(MFREE_BEGIN(slot),HEAP_ALIGNMENT));
 assert(IS_ALIGNED(MFREE_SIZE(slot),HEAP_ALIGNMENT));
 if (MFREE_SIZE(slot) >= self->mh_freethresh) {
  /* Release full pages. */
  PAGE_ALIGNED void *page_begin = (PAGE_ALIGNED void *)CEIL_ALIGN((uintptr_t)MFREE_BEGIN(slot),PAGESIZE);
  PAGE_ALIGNED void *page_end   = (PAGE_ALIGNED void *)FLOOR_ALIGN((uintptr_t)MFREE_END(slot),PAGESIZE);
  assert(page_begin <= page_end);
  assert((uintptr_t)page_begin >= MFREE_BEGIN(slot));
  assert((uintptr_t)page_end   <= MFREE_END(slot));
  if (page_begin != page_end) {
   /* Unlink the free portion. */
   struct mfree *hi_slot;
   gfp_t page_flags;
   size_t lo_size = (uintptr_t)page_begin-MFREE_BEGIN(slot);
   size_t hi_size = MFREE_END(slot)-(uintptr_t)page_end;
   assertf(IS_ALIGNED(lo_size,HEAP_ALIGNMENT),"lo_size = %Iu\n",lo_size);
   assertf(IS_ALIGNED(hi_size,HEAP_ALIGNMENT),"hi_size = %Iu\n",hi_size);
   hi_slot = (struct mfree *)page_end;
   if (lo_size && lo_size < HEAP_MINSIZE) {
    lo_size += PAGESIZE;
    *(uintptr_t *)&page_begin += PAGESIZE;
    if (page_begin == page_end)
        return;
   }
   if (hi_size && hi_size < HEAP_MINSIZE) {
    hi_size += PAGESIZE;
    *(uintptr_t *)&page_end -= PAGESIZE;
    *(uintptr_t *)&hi_slot -= PAGESIZE;
    if (page_begin == page_end)
        return;
   }
   assert(page_begin <= page_end);
   page_flags = slot->mf_flags|(flags&~GFP_CALLOC);

   /* Remove the old slot from the heap. */
   LIST_REMOVE(slot,mf_lsize);
   mfree_tree_pop_at(pslot,addr_semi,addr_level);

   /* Clear the small amount of memory filled with the free-controller. */
   if (page_flags&GFP_CALLOC)
       memset(slot,0,SIZEOF_MFREE);

   /* Create the low & high parts. */
   if (lo_size) mheap_release_nomerge(self,slot,lo_size,page_flags);
   if (hi_size) mheap_release_nomerge(self,hi_slot,hi_size,page_flags);

   /* Release heap data back to the system. */
   heap_validate_all();
   core_page_free(VM_ADDR2PAGE((uintptr_t)page_begin),
                  CEILDIV((PAGE_ALIGNED uintptr_t)page_end-
                          (PAGE_ALIGNED uintptr_t)page_begin,
                           PAGESIZE),
                  page_flags);
   return;
  }
 }
}
PRIVATE ATTR_NOTHROW bool KCALL
mheap_release(struct mheap *__restrict self,
              VIRT void *p, size_t n_bytes, gfp_t flags) {
 struct mfree **pslot,*slot,*iter,*next;
 struct mfree *free_slot;
 ATREE_SEMI_T(uintptr_t) addr_semi;
 ATREE_LEVEL_T addr_level;
 CHECK_HOST_DATA(p,n_bytes);
 assert(mutex_holding(&self->mh_lock));
 assert(IS_ALIGNED((uintptr_t)p,HEAP_ALIGNMENT));
 assert(IS_ALIGNED(n_bytes,HEAP_ALIGNMENT));
 assert(!mfree_tree_locate(self->mh_addr,(uintptr_t)p));
 assert(!mfree_tree_locate(self->mh_addr,(uintptr_t)p+n_bytes-1));
 if unlikely(!n_bytes)
    return true;
 heap_validate_all();
 /* Check for extending a free range above. */
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);
 pslot      = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)p-1,
                                    &addr_semi,&addr_level);
 if (pslot != NULL) {
  /* Extend this slot above. */
  slot = *pslot;
  CHECK_HOST_DOBJ(slot);
  assert(*slot->mf_lsize.le_pself == slot);
  assert(MFREE_END(slot) == (uintptr_t)p);
  heap_validate_all();
  heap_novalidate_begin();
  asserte(mfree_tree_pop_at(pslot,addr_semi,addr_level) == slot);
#ifndef NDEBUG
  if (flags&GFP_CALLOC && !(slot->mf_flags&MFREE_FZERO))
      mempatl(p,DEBUGHEAP_NO_MANS_LAND,n_bytes);
  if (slot->mf_flags&MFREE_FZERO && !(flags&GFP_CALLOC))
      mempatl((byte_t *)slot+SIZEOF_MFREE,DEBUGHEAP_NO_MANS_LAND,
              MFREE_SIZE(slot)-SIZEOF_MFREE);
#endif
  if (!(slot->mf_flags&MFREE_FZERO)) flags &= ~(GFP_CALLOC);
  slot->mf_flags = flags & GFP_CALLOC;
  slot->mf_size += n_bytes;
  mfree_set_checksum(slot);
  assert(slot->mf_size != 0);
  addr_semi  = ATREE_SEMI0(uintptr_t);
  addr_level = ATREE_LEVEL0(uintptr_t);
  pslot      = mfree_tree_pinsert_at(&self->mh_addr,slot,&addr_semi,&addr_level);
  iter       = slot->mf_lsize.le_next;
  if (iter && MFREE_SIZE(iter) < MFREE_SIZE(slot)) {
   /* Fix the size position. */
   LIST_REMOVE(slot,mf_lsize);
   while ((next = iter->mf_lsize.le_next) != NULL &&
           MFREE_SIZE(next) < MFREE_SIZE(slot)) iter = next;
   /* Re-insert the slot. */
   LIST_INSERT_AFTER(iter,slot,mf_lsize);
  }
  heap_novalidate_end();
  heap_validate_all();
  mheap_unmapfree(self,pslot,addr_semi,addr_level,flags);
  heap_validate_all();
  return true;
 }
 /* Check for extending a free range below. */
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);
 pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)p+n_bytes,
                               &addr_semi,&addr_level);
 if (pslot) {
  /* Extend this slot below. */
  slot = *pslot;
  CHECK_HOST_DOBJ(slot);
  assert(MFREE_BEGIN(slot) == (uintptr_t)p+n_bytes);
  assertf(MFREE_SIZE(slot) >= HEAP_MINSIZE,
          "MFREE_SIZE(slot) = %Iu\n",MFREE_SIZE(slot));
  heap_novalidate_begin();
  mfree_tree_pop_at(pslot,addr_semi,addr_level);
  free_slot = (struct mfree *)p;
  memcpy(free_slot,slot,SIZEOF_MFREE);
#ifndef NDEBUG
  if (free_slot->mf_flags&MFREE_FZERO && !(flags&GFP_CALLOC))
      mempatl((byte_t *)slot+SIZEOF_MFREE,DEBUGHEAP_NO_MANS_LAND,
              MFREE_SIZE(slot)-SIZEOF_MFREE);
  if (flags&GFP_CALLOC && !(free_slot->mf_flags&MFREE_FZERO))
      mempatl((byte_t *)free_slot+SIZEOF_MFREE,DEBUGHEAP_NO_MANS_LAND,n_bytes);
#endif
  if (!(free_slot->mf_flags&MFREE_FZERO)) flags &= ~(GFP_CALLOC);
  free_slot->mf_flags = flags & GFP_CALLOC;
  free_slot->mf_size += n_bytes;
  mfree_set_checksum(free_slot);
  assert(slot->mf_size != 0);
  *free_slot->mf_lsize.le_pself = free_slot;
  if (free_slot->mf_lsize.le_next != NULL)
      free_slot->mf_lsize.le_next->mf_lsize.le_pself = &free_slot->mf_lsize.le_next;
  assert(IS_ALIGNED(MFREE_SIZE(free_slot),HEAP_ALIGNMENT));
  if (free_slot->mf_flags&MFREE_FZERO) {
   if (n_bytes < SIZEOF_MFREE)
        memset((void *)((uintptr_t)slot+(SIZEOF_MFREE-n_bytes)),0,n_bytes);
   else memset(slot,0,SIZEOF_MFREE);
  }
#ifndef NDEBUG
  else {
   if (n_bytes < SIZEOF_MFREE)
        mempatl((void *)((uintptr_t)slot+(SIZEOF_MFREE-n_bytes)),
                 DEBUGHEAP_NO_MANS_LAND,n_bytes);
   else {
    mempatl(slot,DEBUGHEAP_NO_MANS_LAND,SIZEOF_MFREE);
   }
  }
#endif
  assert(IS_ALIGNED(MFREE_SIZE(free_slot),HEAP_ALIGNMENT));
  addr_semi  = ATREE_SEMI0(uintptr_t);
  addr_level = ATREE_LEVEL0(uintptr_t);
  pslot      = mfree_tree_pinsert_at(&self->mh_addr,free_slot,&addr_semi,&addr_level);
  iter       = free_slot->mf_lsize.le_next;
  if (iter && MFREE_SIZE(iter) < MFREE_SIZE(free_slot)) {
   /* Fix the size position. */
   LIST_REMOVE(free_slot,mf_lsize);
   while ((next = iter->mf_lsize.le_next) != NULL &&
           MFREE_SIZE(next) < MFREE_SIZE(free_slot)) iter = next;
   /* Re-insert the slot. */
   LIST_INSERT_AFTER(iter,free_slot,mf_lsize);
  }
  heap_novalidate_end();
  mheap_unmapfree(self,pslot,addr_semi,
                  addr_level,flags);
  heap_validate_all();
  return true;
 }
 /* Make sure the heap part wouldn't shrink too small. */
 if (n_bytes < HEAP_MINSIZE)
     return false;
 heap_validate_all();
 mheap_release_nomerge(self,p,n_bytes,flags);
 heap_validate_all();
 return true;
}



PRIVATE ATTR_NOTHROW void KCALL
heap_exec_freelater(struct mheap *__restrict self) {
 struct mheap_freelater *chain,*next;
 /* Quick check: are there free-later entries? */
 if (!ATOMIC_READ(self->mh_freelater)) return;
 atomic_rwlock_write(&self->mh_freelock);
 COMPILER_BARRIER();
 chain = self->mh_freelater;
 self->mh_freelater = NULL;
 atomic_rwlock_endwrite(&self->mh_freelock);
 while (chain) {
  size_t size = chain->fl_size;
  gfp_t  flag = chain->fl_flag;
  next = chain->fl_next;
  if (flag & GFP_CALLOC)
   memset(chain,0,sizeof(struct mheap_freelater));
#ifndef NDEBUG
  else {
   mempatl(chain,DEBUGHEAP_NO_MANS_LAND,
           sizeof(struct mheap_freelater));
  }
#endif

#if 0
  debug_printf("heap_exec_freelater() -- heap_freelater(%p,%Iu,%x)\n",
               chain,size,flag);
#endif
  mheap_release(self,chain,size,flag);
  chain = next;
 }
}
PRIVATE ATTR_NOTHROW void KCALL
heap_freelater(struct mheap *__restrict self,
               VIRT void *ptr, size_t num_bytes,
               gfp_t flags) {
 struct mheap_freelater *entry;
#if 0
 debug_printf("heap_freelater(%p,%Iu,%x)\n",ptr,num_bytes,flags);
#endif
 entry          = (struct mheap_freelater *)ptr;
 entry->fl_size = num_bytes;
 entry->fl_flag = flags;
 atomic_rwlock_write(&self->mh_freelock);
 entry->fl_next     = self->mh_freelater;
 self->mh_freelater = entry;
 atomic_rwlock_endwrite(&self->mh_freelock);
}




PRIVATE struct heapptr KCALL
heap_doalloc(struct mheap *__restrict self,
             size_t num_bytes, gfp_t flags) {
 struct heapptr result; vm_vpage_t pageaddr;
 struct mfree **iter,**end,*chain;
 size_t alloc_bytes,page_bytes,alloc_pages;
 if unlikely(__builtin_add_overflow(num_bytes,HEAP_ALIGNMENT-1,&alloc_bytes))
    goto err_overflow;
 alloc_bytes &= ~(HEAP_ALIGNMENT-1);
 if unlikely(alloc_bytes < HEAP_MINSIZE)
             alloc_bytes = HEAP_MINSIZE;
 iter = &self->mh_size[HEAP_BUCKET_OF(alloc_bytes)];
 end  =  COMPILER_ENDOF(self->mh_size);
 assertf(iter >= self->mh_size &&
         iter <  COMPILER_ENDOF(self->mh_size),
         "HEAP_BUCKET_OF(%Iu) = %Iu/%Iu",
         alloc_bytes,HEAP_BUCKET_OF(alloc_bytes),
         COMPILER_LENOF(self->mh_size));
 if (flags&GFP_ATOMIC) {
  if (!mutex_try(&self->mh_lock))
      error_throw(E_WOULDBLOCK);
 } else {
  mutex_get(&self->mh_lock);
 }
 TRY {
  heap_exec_freelater(self);
  heap_validate_all();
  for (; iter != end; ++iter) {
   chain = *iter;
   while (chain && MFREE_SIZE(chain) < alloc_bytes)
          chain = chain->mf_lsize.le_next;
   if (chain) {
    size_t unused_size;
    u8 chain_attr = chain->mf_flags;
    result.hp_ptr = (void *)chain;
#ifndef NDEBUG
    {
     struct mfree *del_entry;
     del_entry = mfree_tree_remove(&self->mh_addr,MFREE_BEGIN(chain));
     assertf(del_entry == chain,
             "Invalid tracking for %p...%p (%p != %p)",
             MFREE_MIN(chain),MFREE_MAX(chain),
             MFREE_BEGIN(del_entry),MFREE_BEGIN(chain));
    }
#else
    mfree_tree_remove(&self->mh_addr,MFREE_BEGIN(chain));
#endif
    LIST_REMOVE(chain,mf_lsize);
    unused_size = MFREE_SIZE(chain)-alloc_bytes;
    if (unused_size < HEAP_MINSIZE) {
     /* Remainder is too small. - Allocate it as well. */
     alloc_bytes += unused_size;
    } else {
     void *unused_begin = (void *)((uintptr_t)chain+alloc_bytes);
     /* Make sure the the remainder is properly aligned. */
     assert(IS_ALIGNED((uintptr_t)unused_begin,HEAP_ALIGNMENT));
     assert(IS_ALIGNED(unused_size,HEAP_ALIGNMENT));
     assert(unused_size < MFREE_SIZE(chain));
     heap_validate_all();
     mheap_release_nomerge(self,unused_begin,unused_size,
                          (flags&~(GFP_CALLOC))|chain_attr);
     heap_validate_all();
    }
    /* Initialize the result memory. */
    if (flags&GFP_CALLOC) {
     if (chain_attr&MFREE_FZERO)
          memset(result.hp_ptr,0,MIN(SIZEOF_MFREE,alloc_bytes));
     else memset(result.hp_ptr,0,alloc_bytes);
    }
#ifndef NDEBUG
    else {
     reset_heap_data((byte_t *)result.hp_ptr,
                      DEBUGHEAP_FRESH_MEMORY,
                      alloc_bytes);
    }
#endif
    result.hp_siz = alloc_bytes;
    assert(IS_ALIGNED((uintptr_t)result.hp_ptr,HEAP_ALIGNMENT));
    goto done;
   }
  }
  heap_validate_all();
  /* Allocate whole pages. */
  if unlikely(__builtin_add_overflow(alloc_bytes,PAGESIZE-1,&page_bytes))
     goto err_overflow_unlock;
  page_bytes &= ~(PAGESIZE-1);
  /* Add the overallocation addend (if this overflows, try without;
   * although unless the overallocation is absurdly large, that'll
   * just fail, too). */
  if unlikely(__builtin_add_overflow(page_bytes,self->mh_overalloc,&page_bytes))
     goto allocate_without_overalloc;
  TRY {
   if unlikely(__builtin_add_overflow(page_bytes,PAGESIZE-1,&alloc_pages))
      goto err_overflow_unlock;
   pageaddr = core_page_alloc(self,alloc_pages/PAGESIZE,flags);
  } CATCH(E_BADALLOC) {
   if (page_bytes == CEIL_ALIGN(alloc_bytes,PAGESIZE))
       error_rethrow();
allocate_without_overalloc:
   /* Try again without overallocation. */
   page_bytes = CEIL_ALIGN(alloc_bytes,PAGESIZE);
   if unlikely(__builtin_add_overflow(page_bytes,PAGESIZE-1,&alloc_pages))
      goto err_overflow_unlock;
   pageaddr   = core_page_alloc(self,alloc_pages/PAGESIZE,flags);
  }
  result.hp_ptr = (VIRT void *)VM_PAGE2ADDR(pageaddr);
  if (page_bytes != alloc_bytes) {
   void *unused_begin; size_t unused_size;
   /* Release all unused memory. */
   unused_begin = (void *)((uintptr_t)result.hp_ptr+alloc_bytes);
   unused_size  = page_bytes-alloc_bytes;
#ifndef NDEBUG
   if (!(flags&GFP_CALLOC))
         mempatl(unused_begin,DEBUGHEAP_NO_MANS_LAND,unused_size);
#endif
   if (!mheap_release(self,unused_begin,unused_size,flags))
        alloc_bytes = page_bytes;
   heap_validate_all();
  }
  result.hp_siz = alloc_bytes;
done:;
 } FINALLY {
  heap_validate_all();
  mutex_put(&self->mh_lock);
 }
 return result;
err_overflow_unlock:
 heap_validate_all();
 mutex_put(&self->mh_lock);
err_overflow:
 heap_allocation_overflow(num_bytes);
}


PUBLIC struct heapptr KCALL
heap_alloc(struct mheap *__restrict self,
           size_t num_bytes, gfp_t flags) {
 struct heapptr result;
 bool has_vm_lock = false;
again:
 TRY {
  TRY {
   result = heap_doalloc(self,num_bytes,flags);
  } FINALLY {
   if (has_vm_lock)
       mutex_put(&vm_kernel.vm_lock);
  }
 } CATCH (E_ACQUIRE_VM_LOCK) {
  assert(!has_vm_lock);
  has_vm_lock = true;
  if (flags&GFP_ATOMIC) {
   if (!mutex_try(&vm_kernel.vm_lock))
       error_throw(E_WOULDBLOCK);
  } else {
   mutex_get(&vm_kernel.vm_lock);
  }
  goto again;
 }
 return result;
}

PRIVATE struct heapptr KCALL
heap_doalign(struct mheap *__restrict self,
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
     return heap_doalloc(self,num_bytes,flags);
 if unlikely(__builtin_add_overflow(num_bytes,HEAP_ALIGNMENT-1,&alloc_bytes))
    goto err_overflow;
 alloc_bytes &= ~(HEAP_ALIGNMENT-1);
 if unlikely(alloc_bytes < HEAP_MINSIZE)
             alloc_bytes = HEAP_MINSIZE;
 if (flags&GFP_ATOMIC) {
  if (!mutex_try(&self->mh_lock))
      error_throw(E_WOULDBLOCK);
 } else {
  mutex_get(&self->mh_lock);
 }
 /* Must overallocate by at least `HEAP_MINSIZE',
  * so we can _always_ free unused lower memory. */
 if unlikely(__builtin_add_overflow(alloc_bytes,min_alignment,&heap_alloc_bytes))
    goto err_overflow_unlock;
 if unlikely(__builtin_add_overflow(heap_alloc_bytes,HEAP_MINSIZE,&heap_alloc_bytes))
    goto err_overflow_unlock;
 TRY {
  heap_exec_freelater(self);
  result_base = heap_doalloc(self,heap_alloc_bytes,flags);
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
  asserte(mheap_release(self,result_base.hp_ptr,nouse_size,flags));
  result_base.hp_siz -= nouse_size;

  /* Try to release upper memory. */
  assert(result_base.hp_siz >= alloc_bytes);
  nouse_size = result_base.hp_siz-alloc_bytes;
  if (mheap_release(self,(void *)((uintptr_t)result.hp_ptr+alloc_bytes),nouse_size,flags))
      result_base.hp_siz -= nouse_size;
 } FINALLY {
  heap_validate_all();
  mutex_put(&self->mh_lock);
 }
 assert(result_base.hp_siz >= alloc_bytes);
 assert(IS_ALIGNED((uintptr_t)result.hp_ptr+offset,min_alignment));
 result.hp_siz = result_base.hp_siz;
 return result;
err_overflow_unlock:
 heap_validate_all();
 mutex_get(&self->mh_lock);
err_overflow:
 heap_allocation_overflow(num_bytes);
}

PUBLIC struct heapptr KCALL
heap_align(struct mheap *__restrict self,
           size_t min_alignment, ptrdiff_t offset,
           size_t num_bytes, gfp_t flags) {
 struct heapptr result;
 bool has_vm_lock = false;
again:
 TRY {
  TRY {
   result = heap_doalign(self,min_alignment,offset,num_bytes,flags);
  } FINALLY {
   if (has_vm_lock)
       mutex_put(&vm_kernel.vm_lock);
  }
 } CATCH (E_ACQUIRE_VM_LOCK) {
  assert(!has_vm_lock);
  has_vm_lock = true;
  if (flags&GFP_ATOMIC) {
   if (!mutex_try(&vm_kernel.vm_lock))
       error_throw(E_WOULDBLOCK);
  } else {
   mutex_get(&vm_kernel.vm_lock);
  }
  goto again;
 }
 return result;
}


PRIVATE size_t KCALL
mheap_acquire_at(struct mheap *__restrict self, VIRT void *ptr,
                 size_t num_bytes, gfp_t flags) {
 struct mfree **pslot,*slot;
 ATREE_SEMI_T(uintptr_t) addr_semi;
 ATREE_LEVEL_T addr_level;
 size_t result,alloc_bytes;
 size_t unused_before; gfp_t slot_flags;
 assert(mutex_holding(&self->mh_lock));
 assert(IS_ALIGNED((uintptr_t)ptr,HEAP_ALIGNMENT));
 /* Align the given `num_bytes' by the required heap-alignment.
  * Also: Check for overflows. */
 if unlikely(__builtin_add_overflow(num_bytes,HEAP_ALIGNMENT-1,&alloc_bytes))
    heap_allocation_overflow(num_bytes);
 alloc_bytes &= ~(HEAP_ALIGNMENT-1);
 if unlikely(!alloc_bytes) return 0;
 addr_semi  = ATREE_SEMI0(uintptr_t);
 addr_level = ATREE_LEVEL0(uintptr_t);
 /* Search for a free slot at the given address. */
 pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)ptr,
                               &addr_semi,&addr_level);
 if unlikely(!pslot) {
  /* Easy enough: the slot doesn't exist, so allocate memory here. */
  PAGE_ALIGNED void  *ptr_page = (void *)FLOOR_ALIGN((uintptr_t)ptr,PAGESIZE);
  PAGE_ALIGNED size_t ptr_size = CEIL_ALIGN(((uintptr_t)ptr-(uintptr_t)ptr_page)+alloc_bytes,PAGESIZE);
  assertf(IS_ALIGNED(ptr_size,HEAP_ALIGNMENT),"ptr_size = %Iu",ptr_size);
  if (!core_page_allocat(VM_ADDR2PAGE((uintptr_t)ptr_page),
                         ptr_size / PAGESIZE,flags))
       return 0;
  slot = (struct mfree *)ptr_page;
  assert(IS_ALIGNED(MFREE_BEGIN(slot),HEAP_ALIGNMENT));
  assert((uintptr_t)ptr >= (uintptr_t)slot);
  result = ptr_size-((uintptr_t)ptr-(uintptr_t)slot);
  assert(IS_ALIGNED(result,HEAP_ALIGNMENT));
  slot_flags = flags;
 } else {
  slot = *pslot;
  CHECK_HOST_DOBJ(slot);
  assert((uintptr_t)ptr >= MFREE_MIN(slot));
  assert((uintptr_t)ptr <= MFREE_MAX(slot));
  assert(MFREE_SIZE(slot) >= HEAP_MINSIZE);
  assert(MFREE_SIZE(slot) > ((uintptr_t)ptr-MFREE_BEGIN(slot)));
  assert(IS_ALIGNED(MFREE_BEGIN(slot),HEAP_ALIGNMENT));
  result = MFREE_SIZE(slot)-((uintptr_t)ptr-MFREE_BEGIN(slot));
  slot_flags = slot->mf_flags;
  assert(IS_ALIGNED(result,HEAP_ALIGNMENT));
  if (result < alloc_bytes) {
   void  *missing_addr;
   size_t missing_size;
   size_t missing_alloc;
   /* The slot is too small. - allocate more memory afterwards. */
   missing_addr = (void *)MFREE_END(slot);
   missing_size = alloc_bytes-result;
   assert(IS_ALIGNED((uintptr_t)missing_addr,HEAP_ALIGNMENT));
   assert(IS_ALIGNED(missing_size,HEAP_ALIGNMENT));
   missing_alloc = mheap_acquire_at(self,missing_addr,missing_size,flags);
   if (!missing_alloc) return 0;
   /* Find the slot again, now that the heap has changed. */
   addr_semi  = ATREE_SEMI0(uintptr_t);
   addr_level = ATREE_LEVEL0(uintptr_t);
   /* Search for a free slot at the given address. */
   pslot = mfree_tree_plocate_at(&self->mh_addr,(uintptr_t)ptr,
                                 &addr_semi,&addr_level);
   assertf(pslot != NULL,"But we know it must exist! (%p...%p)");
   assert(*pslot == slot);
   assert(slot_flags == (*pslot)->mf_flags);
   result += missing_size;
  }
  /* Remove the slot from the address-tree & size-chain. */
  mfree_tree_pop_at(pslot,addr_semi,addr_level);
  LIST_REMOVE(slot,mf_lsize);
  /* Clear out the slot header. */
  if (slot_flags&MFREE_FZERO)
       memset(slot,0,SIZEOF_MFREE);
#ifndef NDEBUG
  else mempatl(slot,DEBUGHEAP_NO_MANS_LAND,SIZEOF_MFREE);
#endif
  slot_flags = (flags&~GFP_CALLOC)|slot_flags;
 }
 assert((uintptr_t)ptr >= MFREE_BEGIN(slot));
 unused_before = (uintptr_t)ptr-MFREE_BEGIN(slot);
 assert(IS_ALIGNED(unused_before,HEAP_ALIGNMENT));
 if (unused_before) {
  /* Make sure that the sub-pre-range isn't too small. */
  if (unused_before < HEAP_MINSIZE) {
   void *slot_before_addr;
   size_t slot_before_size,slot_before_alloc;
   slot_before_size = HEAP_MINSIZE-unused_before;
   assert(IS_ALIGNED(slot_before_size,HEAP_ALIGNMENT));
   assert(slot_before_size != 0);
   slot_before_addr = (void *)((uintptr_t)slot-slot_before_size);
   slot_before_alloc = mheap_acquire_at(self,slot_before_addr,slot_before_size,
                                        slot_flags);
   if (!slot_before_alloc) {
    asserte(mheap_release(self,slot,result+
                        ((uintptr_t)ptr-MFREE_BEGIN(slot)),
                          slot_flags));
    return 0;
   }
   /* Got some memory before the slot! */
   assert(IS_ALIGNED(slot_before_alloc,HEAP_ALIGNMENT));
   assert(slot_before_alloc == slot_before_size);
   slot           = (struct mfree *)slot_before_addr;
   unused_before += slot_before_alloc;
  }
  /* Free the unused memory before the slot. */
  mheap_release_nomerge(self,slot,unused_before,slot_flags);
 }
 /* At this point we've allocated `slot_avail' bytes at `p'
  * >> Now we must simply try to free as much of the difference as possible. */
 assertf(IS_ALIGNED(result,HEAP_ALIGNMENT),"slot_avail = %Iu\n",result);
 assert(IS_ALIGNED(alloc_bytes,HEAP_ALIGNMENT));
 assert(result >= alloc_bytes);
 {
  size_t unused_size = result-alloc_bytes;
  /* Try to release high memory. */
  if (unused_size &&
      mheap_release(self,
                   (void *)((uintptr_t)ptr+alloc_bytes),
                    unused_size,slot_flags))
      result -= unused_size;
 }
 /* Do final initialization of memory. */
 if (flags&GFP_CALLOC) {
  if (!(slot_flags&GFP_CALLOC))
        memset(ptr,0,result);
 }
#ifndef NDEBUG
 else {
  reset_heap_data((byte_t *)ptr,DEBUGHEAP_FRESH_MEMORY,result);
 }
#endif
 return result;
}


#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

PRIVATE size_t KCALL
heap_doalloc_at(struct mheap *__restrict self,
                VIRT void *__restrict ptr,
                size_t num_bytes, gfp_t flags) {
 size_t result;
 if (flags&GFP_ATOMIC) {
  if (!mutex_try(&self->mh_lock))
      error_throw(E_WOULDBLOCK);
 } else {
  mutex_get(&self->mh_lock);
 }
 TRY {
  heap_exec_freelater(self);
  result = mheap_acquire_at(self,ptr,num_bytes,flags);
 } FINALLY {
  heap_validate_all();
  mutex_put(&self->mh_lock);
 }
 return result;
}

PUBLIC size_t KCALL
heap_alloc_at(struct mheap *__restrict self,
              VIRT void *__restrict ptr,
              size_t num_bytes, gfp_t flags) {
 size_t result;
 bool has_vm_lock = false;
again:
 TRY {
  TRY {
   result = heap_doalloc_at(self,ptr,num_bytes,flags);
  } FINALLY {
   if (has_vm_lock)
       mutex_put(&vm_kernel.vm_lock);
  }
 } CATCH (E_ACQUIRE_VM_LOCK) {
  assert(!has_vm_lock);
  has_vm_lock = true;
  if (flags&GFP_ATOMIC) {
   if (!mutex_try(&vm_kernel.vm_lock))
       error_throw(E_WOULDBLOCK);
  } else {
   mutex_get(&vm_kernel.vm_lock);
  }
  goto again;
 }
 return result;
}

PRIVATE void KCALL
heap_dofree(struct mheap *__restrict self,
            VIRT void *ptr, size_t num_bytes,
            gfp_t flags) {
 if (!mutex_try(&self->mh_lock)) {
  heap_freelater(self,ptr,num_bytes,flags);
  return;
 }
 TRY {
  heap_validate_all();
  heap_exec_freelater(self);
#ifdef NDEBUG
  mheap_release(self,ptr,num_bytes,flags);
#else
  {
   bool is_ok;
   is_ok = mheap_release(self,ptr,num_bytes,flags);
   assertf(is_ok,"Failed to free %p...%p",
           ptr,(uintptr_t)ptr+num_bytes-1);
   heap_validate_all();
  }
#endif
 } FINALLY {
  heap_validate_all();
  mutex_put(&self->mh_lock);
 }
}


PUBLIC ATTR_NOTHROW void KCALL
heap_free(struct mheap *__restrict self,
          VIRT void *ptr, size_t num_bytes,
          gfp_t flags) {
 bool has_vm_lock = false;
 assertf(num_bytes >= HEAP_MINSIZE,
         "Invalid heap_free(): Too few bytes (%Iu < %Iu)",
         num_bytes,HEAP_MINSIZE);
 assertf(IS_ALIGNED((uintptr_t)ptr,HEAP_ALIGNMENT),
         "Invalid heap_free(): Unaligned base pointer %p",
         ptr);
 assertf(IS_ALIGNED(num_bytes,HEAP_ALIGNMENT),
         "Invalid heap_free(): Unaligned free size %Iu (%#Ix)",
         num_bytes,num_bytes);
#ifndef NDEBUG
 if (!(flags & GFP_CALLOC))
     reset_heap_data((byte_t *)ptr,DEBUGHEAP_NO_MANS_LAND,num_bytes);
#endif
again:
 TRY {
  TRY {
   heap_dofree(self,ptr,num_bytes,flags);
  } FINALLY {
   if (has_vm_lock)
       mutex_put(&vm_kernel.vm_lock);
  }
 } CATCH (E_ACQUIRE_VM_LOCK) {
  assert(!has_vm_lock);
  if (!mutex_try(&vm_kernel.vm_lock)) {
   heap_freelater(self,ptr,num_bytes,flags);
   return;
  }
  has_vm_lock = true;
  goto again;
 }
}

#endif

DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_HEAP_C */
