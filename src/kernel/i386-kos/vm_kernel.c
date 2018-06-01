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
#ifndef GUARD_KERNEL_I386_KOS_VM_KERNEL_C
#define GUARD_KERNEL_I386_KOS_VM_KERNEL_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/align.h>
#include <hybrid/section.h>
#include <kernel/sections.h>
#include <kernel/paging.h>
#include <kernel/debug.h>
#include <kernel/vm.h>
#include <fs/linker.h>
#include <fs/driver.h>
#include <string.h>
#include <except.h>
#include <assert.h>

DECL_BEGIN

#ifndef KERNEL_CORE_BASE
#define KERNEL_CORE_BASE KERNEL_BASE
#endif

#ifdef __INTELLISENSE__
#define DEFINE_VM_REGION_PHYS(self,num_pages,physical_starting_page) { }
#define DEFINE_VM_REGION_RESV(self,num_pages)                        { }
#define DEFINE_VM_NODE(min_page,max_page,prot,region)                { }
#else
#define __DEFINE_VM_REGION(self,num_pages,type,physical_starting_page) \
    { \
        .vr_refcnt = 1, \
        .vr_lock   = MUTEX_INIT, \
        .vr_futex  = ATOMIC_RWPTR_INIT(NULL), \
        .vr_type   = type, \
        .vr_init   = VM_REGION_INIT_FNORMAL, \
        .vr_flags  = VM_REGION_FIMMUTABLE|VM_REGION_FDONTMERGE|VM_REGION_FLEAKINGPARTS, \
        .vr_funds  = 0, \
        .vr_size   = num_pages, \
        .vr_parts  = &self.vr_part0, \
        .vr_part0  = { \
            .vp_refcnt = 2, \
            .vp_chain = { \
                .le_next  = NULL, \
                .le_pself = &self.vr_parts, \
            }, \
            .vp_start  = 0, \
            .vp_state  = VM_PART_INCORE, \
            .vp_flags  = VM_PART_FKEEP|VM_PART_FWEAKREF|VM_PART_FNOSWAP, \
            .vp_locked = 0x3fff, \
            .vp_phys   = { \
                .py_num_scatter = 1, \
                .py_iscatter = { \
                    [0] = { \
                        .ps_addr = physical_starting_page, \
                        .ps_size = num_pages \
                    } \
                } \
            } \
        } \
    }
#define DEFINE_VM_REGION_PHYS(self,num_pages,physical_starting_page) \
    __DEFINE_VM_REGION(self,num_pages,VM_REGION_PHYSICAL,physical_starting_page)
#define DEFINE_VM_REGION_RESV(self,num_pages) \
    __DEFINE_VM_REGION(self,num_pages,VM_REGION_RESERVED,VM_PPAGE_MAX+1)
#define DEFINE_VM_NODE(min_page,max_page,prot,region) \
    { \
        .vn_node = { \
            .a_vmin = min_page, \
            .a_vmax = max_page \
        }, \
        .vn_start   = 0, \
        .vn_region  = region, \
        .vn_notify  = &application_notify, \
        .vn_closure = &kernel_driver, \
        .vn_prot    = PROT_SHARED|(prot), \
        .vn_flag    = VM_NODE_FIMMUTABLE \
    }
#endif

#undef CONFIG_X86_RESERVE_DEBUG_PAGES
#ifndef NDEBUG
#define CONFIG_X86_RESERVE_DEBUG_PAGES 1
#endif
#ifdef __x86_64__
/* On x86_64, all debug pages lie in non-canonical addresses, meaning that
 * we don't have to bother reserving them, as they simply _cannot_ exist!
 * Also note that the debug pages on x86_64 would be:
 *   - 0xcccccccccccc0000
 *   - 0xdeadbeefdead0000
 */
#undef CONFIG_X86_RESERVE_DEBUG_PAGES
#endif


PRIVATE struct vm_region kernel_vm_regions[] = {
    /* The first page of kernel-space is reserved and must not be mapped.
     * This way, we can skip some bound checks when we know that a pointer
     * originating from user-space cannot extend into arbitrary kernel memory.
     * A good example is instruction emulation where we might need to
     * look at the bytes following the instruction.
     * If the user maps their code using these instruction into the
     * last page of user-space at 0xbffff000, instruction emulation
     * might otherwise end up depending on kernel-memory if it then
     * crosses the gap to kernel-space.
     * However, if we ensure that the first page of kernel-space is
     * never actually used, we can always assume that any unchecked
     * memory access with a base pointer originating from user-space
     * can be done immediately, so long as the offset is small than
     * a single page. */
    DEFINE_VM_REGION_RESV(kernel_vm_regions[0],1),
    /* The kernel core segments are tracked as a physical regions. */
    DEFINE_VM_REGION_PHYS(kernel_vm_regions[1],(size_t)kernel_rx_num_pages,(uintptr_t)kernel_rx_minpage - KERNEL_BASE_PAGE),
    DEFINE_VM_REGION_PHYS(kernel_vm_regions[2],(size_t)kernel_ronx_num_pages,(uintptr_t)kernel_ronx_minpage - KERNEL_BASE_PAGE),
    DEFINE_VM_REGION_PHYS(kernel_vm_regions[3],(size_t)kernel_rwnf_num_pages,(uintptr_t)kernel_rwnf_minpage - KERNEL_BASE_PAGE),
    /* The page directory identity mapping is tracked as a reserved memory region. */
    DEFINE_VM_REGION_RESV(kernel_vm_regions[4],X86_PDIR_E1_IDENTITY_SIZE / PAGESIZE),
#ifdef CONFIG_X86_RESERVE_DEBUG_PAGES
    DEFINE_VM_REGION_RESV(kernel_vm_regions[5],16), /* 64K of reserved memory. */
#endif
};

PRIVATE struct vm_node kernel_vm_nodes[] = {
    /* Define VM Nodes for the first page of kernel-space being reserved. */
    DEFINE_VM_NODE((vm_vpage_t)KERNEL_BASE_PAGE,
                   (vm_vpage_t)KERNEL_BASE_PAGE,
                    PROT_NOUSER,&kernel_vm_regions[0]),
    /* Define VM Nodes for kernel regions. */
    DEFINE_VM_NODE((vm_vpage_t)kernel_rx_minpage,
                   (vm_vpage_t)kernel_rx_maxpage,
                    PROT_READ|PROT_EXEC|PROT_NOUSER,&kernel_vm_regions[1]),
    DEFINE_VM_NODE((vm_vpage_t)kernel_ronx_minpage,
                   (vm_vpage_t)kernel_ronx_maxpage,
                    PROT_READ|PROT_NOUSER,&kernel_vm_regions[2]),
    DEFINE_VM_NODE((vm_vpage_t)kernel_rwnf_minpage,
                   (vm_vpage_t)kernel_rwnf_maxpage,
                    PROT_READ|PROT_WRITE|PROT_NOUSER,&kernel_vm_regions[3]),
    DEFINE_VM_NODE(VM_ADDR2PAGE(X86_VM_KERNEL_PDIR_RESERVED_BASE),
                  (VM_ADDR2PAGE(X86_VM_KERNEL_PDIR_RESERVED_BASE)+
                               (X86_VM_KERNEL_PDIR_RESERVED_SIZE/PAGESIZE))-1,
                   PROT_READ|PROT_WRITE|PROT_NOUSER,&kernel_vm_regions[4]),
#ifdef CONFIG_X86_RESERVE_DEBUG_PAGES
#define VM_NODES_RESERVED_UNMAP_BEGIN  5
#define DEFINE_DEBUG_RESERVE_64K(baseaddr) \
    DEFINE_VM_NODE((vm_vpage_t)VM_ADDR2PAGE(baseaddr), \
                   (vm_vpage_t)VM_ADDR2PAGE(baseaddr)+15, \
                    PROT_NOUSER,&kernel_vm_regions[5])
    /* Reserve 64K (the lower 16 bits of an address) for
     * special kernel-space pointers that we want to be able
     * to use for debugging (to indicate uninitialized memory).
     * By reserving these ranges here, debug code can safely
     * assume that any pointer pointing inside of of these ranges
     * _must_ be invalid (and not just: is probably invalid). */
    DEFINE_DEBUG_RESERVE_64K(0xcccc0000), /* 0xcccccccc */
    DEFINE_DEBUG_RESERVE_64K(0xdead0000), /* 0xdeadbeef, 0xdeadc0de, 0xdeadda7a, 0xdead[whatever...] */
#undef DEFINE_DEBUG_RESERVE_64K
#endif
};

INTERN ATTR_FREEDATA struct vm_region kernel_free_region = 
    DEFINE_VM_REGION_PHYS(kernel_free_region,
                         (size_t)kernel_free_num_pages,
                         (uintptr_t)kernel_free_minpage - KERNEL_BASE_PAGE);
INTERN ATTR_FREEDATA struct vm_node kernel_free_node = 
    DEFINE_VM_NODE((vm_vpage_t)kernel_free_minpage,
                   (vm_vpage_t)kernel_free_maxpage,
                    PROT_EXEC|PROT_READ|PROT_WRITE|PROT_NOUSER,
                    &kernel_free_region);

/* The unaligned, physical end of the kernel after boot.
 * During early boot, we use memory immediately past the
 * kernel for tracking information about the host's
 * memory layout. */
INTDEF PHYS byte_t *mzone_heap_end;

INTERN ATTR_FREETEXT void KCALL
x86_initialize_kernel_vm_mappings(void) {
 unsigned int i; size_t kfree_size;
 kfree_size  = (size_t)(((uintptr_t)mzone_heap_end + KERNEL_CORE_BASE) - (uintptr_t)kernel_free_start);
 kfree_size += (PAGESIZE-1);
 kfree_size /= PAGESIZE;
 assert(kfree_size >= (uintptr_t)kernel_free_num_pages);
 if unlikely(kfree_size > (uintptr_t)kernel_free_num_pages) {
  /* Update the end pointers of the kernel core's Read/Write segment. */
  kernel_free_region.vr_size                                 = kfree_size;
  kernel_free_region.vr_part0.vp_phys.py_iscatter[0].ps_size = kfree_size;
  kernel_free_node.vn_node.a_vmax = (vm_vpage_t)kernel_free_minpage + kfree_size-1;
 }

 /* Insert all the nodes into the kernel VM. */
 asserte(vm_tryacquire(&vm_kernel));
 for (i = 0; i < COMPILER_LENOF(kernel_vm_nodes); ++i) {
  assertf(VM_NODE_SIZE(&kernel_vm_nodes[i]) != 0,
          "Kernel region #%u (at %p) is empty",
          i,VM_NODE_MINADDR(&kernel_vm_nodes[i]));
#if 0
  debug_printf("SEGMENT #%u at %p...%p\n",i,
               VM_NODE_MINADDR(&kernel_vm_nodes[i]),
               VM_NODE_MAXADDR(&kernel_vm_nodes[i]));
#endif
  vm_insert_node(&vm_kernel,&kernel_vm_nodes[i]);
 }
 /* Insert the special node that describes the .free region. */
 vm_insert_node(&vm_kernel,&kernel_free_node);
 vm_release(&vm_kernel);
}

INTDEF byte_t x86_boot_stack_guard_page[];

/* On x86_64, we don't delete the virtual
 * identity mapping of the first 2 Gib at -2Gb! */
INTERN ATTR_FREETEXT void KCALL
x86_delete_virtual_memory_identity(void) {
 /* Unmap all virtual memory that isn't mapped by a VM node. */
 struct vm_node *node;
 vm_vpage_t last_end;
#ifdef __x86_64__
 /* On x86_64, assembly already got rid of any secondary identity mappings. */
#else
 /* Unmap the physical identity mapping of the first 1Gb
  * NOTE: This must be like this, so that the physical pages
  *       used to describe this memory aren't freed by
  *       pagedir_map(), as early boot mirrored them above 3Gb.
  *       If we didn't do this, physical memory from
  *       ~0016xxxx to ~0026xxxx would be made available
  *       to the physical memory allocator.
  *       However if we did that, then all kernel page
  *       directory tables would be considered ~free~ and
  *       be used as general purpose RAM. */
 memsetl(pagedir_kernel.p_e2,0,256);
#endif
 last_end = 0;
 node = vm_kernel.vm_byaddr;
 for (;; node = node->vn_byaddr.le_next) {
  vm_vpage_t node_begin;
  /* Skip the reserved kernel header node to include
   * it while unmapping surrounding nodes.
   * >> We want the first page of kernel-space
   *    to always point into the void! */
  if (node == &kernel_vm_nodes[0]) continue;
#ifdef VM_NODES_RESERVED_UNMAP_BEGIN
  /* Just as with the reserved kernel-base page, also unmap
   * reservations made to keep debug address ranges unused. */
  if (node >= &kernel_vm_nodes[VM_NODES_RESERVED_UNMAP_BEGIN] &&
      node <  COMPILER_ENDOF(kernel_vm_nodes)) continue;
#endif
  node_begin = node ? VM_NODE_BEGIN(node) : VM_VPAGE_MAX+1;
  /* Delete mappings between this node and the previous one. */
  COMPILER_BARRIER();
  pagedir_map(last_end,(size_t)(node_begin-last_end),0,
              PAGEDIR_MAP_FUNMAP);
  COMPILER_BARRIER();
  if (!node) break;
  last_end = VM_NODE_END(node);
 }
 pagedir_map((vm_vpage_t)LOAD_FAR_POINTER(x86_boot_stack_guard_page),
              1,0,PAGEDIR_MAP_FUNMAP);
}

DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_VM_KERNEL_C */
