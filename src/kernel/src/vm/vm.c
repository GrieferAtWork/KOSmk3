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
#ifndef GUARD_KERNEL_SRC_VM_VM_C
#define GUARD_KERNEL_SRC_VM_VM_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/align.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/debug.h>
#include <kernel/heap.h>
#include <kernel/bind.h>
#include <kernel/vm.h>
#include <kernel/malloc.h>
#include <fs/node.h>
#include <fs/linker.h>
#include <sched/userstack.h>
#include <bits/sched.h>
#include <bits/poll.h>
#include <sys/stat.h>
#include <string.h>
#include <except.h>
#include <assert.h>
#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/ipi.h>
#endif


#include <kos/safecall.h>
#if 1
#define INVOKE_NOTIFY(vn_notify,vn_closure,command,begin,end,arg) \
       SAFECALL_KCALL_5(*vn_notify,vn_closure,command,begin,end,arg)
#define INVOKE_NOTIFY_V(vn_notify,vn_closure,command,begin,end,arg) \
       SAFECALL_KCALL_VOID_5(*vn_notify,vn_closure,command,begin,end,arg)
#else
#define INVOKE_NOTIFY(vn_notify,vn_closure,command,begin,end,arg) \
       (*vn_notify)(vn_closure,command,begin,end,arg)
#define INVOKE_NOTIFY_V(vn_notify,vn_closure,command,begin,end,arg) \
       (*vn_notify)(vn_closure,command,begin,end,arg)
#endif


#if PAGESIZE == 0x800000
#define VM_PAGE_UNUSED_BITS   23
#elif PAGESIZE == 0x400000
#define VM_PAGE_UNUSED_BITS   22
#elif PAGESIZE == 0x200000
#define VM_PAGE_UNUSED_BITS   21
#elif PAGESIZE == 0x100000
#define VM_PAGE_UNUSED_BITS   20
#elif PAGESIZE == 0x80000
#define VM_PAGE_UNUSED_BITS   19
#elif PAGESIZE == 0x40000
#define VM_PAGE_UNUSED_BITS   18
#elif PAGESIZE == 0x20000
#define VM_PAGE_UNUSED_BITS   17
#elif PAGESIZE == 0x10000
#define VM_PAGE_UNUSED_BITS   16
#elif PAGESIZE == 0x8000
#define VM_PAGE_UNUSED_BITS   15
#elif PAGESIZE == 0x4000
#define VM_PAGE_UNUSED_BITS   14
#elif PAGESIZE == 0x2000
#define VM_PAGE_UNUSED_BITS   13
#elif PAGESIZE == 0x1000
#define VM_PAGE_UNUSED_BITS   12
#elif PAGESIZE == 0x800
#define VM_PAGE_UNUSED_BITS   11
#elif PAGESIZE == 0x400
#define VM_PAGE_UNUSED_BITS   10
#elif PAGESIZE == 0x200
#define VM_PAGE_UNUSED_BITS    9
#elif PAGESIZE == 0x100
#define VM_PAGE_UNUSED_BITS    8
#elif PAGESIZE == 0x80
#define VM_PAGE_UNUSED_BITS    7
#elif PAGESIZE == 0x40
#define VM_PAGE_UNUSED_BITS    6
#elif PAGESIZE == 0x20
#define VM_PAGE_UNUSED_BITS    5
#elif PAGESIZE == 0x10
#define VM_PAGE_UNUSED_BITS    4
#elif PAGESIZE == 0x8
#define VM_PAGE_UNUSED_BITS    3
#elif PAGESIZE == 0x4
#define VM_PAGE_UNUSED_BITS    2
#elif PAGESIZE == 0x2
#define VM_PAGE_UNUSED_BITS    1
#else
#define VM_PAGE_UNUSED_BITS    0
#endif


/* Since KOSmk3 now uses page-based VM addressing, rather than byte-based,
 * that leads to the fact that our address tree doesn't make use of a
 * portion of the SEMI-address bits any more.
 * To be more precise: The most significant `VM_PAGE_UNUSED_BITS'
 *                     bits of any `vm_vpage_t' are always ZERO(0)
 * The address tree can still operate in this situation, however because
 * it operates by using the bits of an address (starting at the most
 * significant position) to generate a binary tree, the that would
 * also leave the first `VM_PAGE_UNUSED_BITS' branches to not actually
 * be branches at all, but always point to the left.
 * For that reason, we tell it to only start counting at level
 * `(sizeof(void *)*8) - VM_PAGE_UNUSED_BITS', skipping the first
 * couple of bits, once again resulting in what you would call a tree.
 * HINT: To disable this optimization, replace the following line with `#if 1' */
#if 0 /* Keep set at `0' to optimize the address tree search algorithm
       * to take advantage of fixed-value bits in `vm_vpage_t' */
#undef VM_PAGE_UNUSED_BITS
#define VM_PAGE_UNUSED_BITS 0
#endif


#if __SIZEOF_POINTER__ == 4
#define VM_PAGE_BITS   (32-VM_PAGE_UNUSED_BITS)
#elif __SIZEOF_POINTER__ == 8
#define VM_PAGE_BITS   (64-VM_PAGE_UNUSED_BITS)
#else
#define VM_PAGE_BITS   ((__SIZEOF_POINTER__*8)-VM_PAGE_UNUSED_BITS)
#endif

#if VM_PAGE_UNUSED_BITS == 0
#define VM_SEMI0         ATREE_SEMI0(VIRT vm_vpage_t)
#define VM_LEVEL0        ATREE_LEVEL0(VIRT vm_vpage_t)
#else
#define VM_SEMI0         ATREE_SEMI0_BITS(VIRT vm_vpage_t,VM_PAGE_BITS)
#define VM_LEVEL0        ATREE_LEVEL0_BITS(VIRT vm_vpage_t,VM_PAGE_BITS)
#endif


/* Define the ABI for the address tree used by vm. */
#define ATREE(x)                 vm_node_tree_##x
#define Tkey                     vm_vpage_t
#define T                        struct vm_node
#define path                     vn_node
#define ATREE_LOCAL_SEMI0(Tkey)  VM_SEMI0
#define ATREE_LOCAL_LEVEL0(Tkey) VM_LEVEL0
#include <hybrid/list/atree-abi.h>


DECL_BEGIN

PUBLIC ATTR_WEAK void KCALL
pagedir_mapone(VIRT vm_vpage_t virt_page,
               PHYS vm_ppage_t phys_page, u16 perm) {
 /* Implement the weak default version of mapone()
  * by forwarding to the regular map() */
 pagedir_map(virt_page,1,phys_page,perm);
}


struct node_info {
    VIRT vm_vpage_t ni_page_min;
    VIRT vm_vpage_t ni_page_max;
    struct vm_node *ni_min;     /* [0..1] Lowest branch. */
    struct vm_node *ni_max;     /* [0..1] Greatest branch. */
    vm_vpage_t      ni_min_min; /* == VM_NODE_MIN(ni_min). */
    vm_vpage_t      ni_max_max; /* == VM_NODE_MAX(ni_max). */
};

PRIVATE void KCALL
vm_findnodes(struct node_info *__restrict info,
             struct vm_node *__restrict node,
             ATREE_SEMI_T(VIRT vm_vpage_t) addr_semi,
             ATREE_LEVEL_T addr_level) {
again:
 if (info->ni_page_min <= node->vn_node.a_vmax &&
     info->ni_page_max >= node->vn_node.a_vmin) {
  /* Found a matching entry!
   * NOTE: Since the caller already split branches
   *       near borders, we are allowed to simply
   *       update this entire branch! */
  if (!info->ni_min || node->vn_node.a_vmin < info->ni_min_min) {
   info->ni_min     = node;
   info->ni_min_min = node->vn_node.a_vmin;
  }
  if (!info->ni_max || node->vn_node.a_vmax < info->ni_max_max) {
   info->ni_max     = node;
   info->ni_max_max = node->vn_node.a_vmax;
  }
 }
 {
  bool walk_min,walk_max;
  walk_min = info->ni_page_min <  addr_semi && node->vn_node.a_min;
  walk_max = info->ni_page_max >= addr_semi && node->vn_node.a_max;
  if (walk_min) {
   /* Recursively continue searching left. */
   if (walk_max) {
    vm_findnodes(info,node->vn_node.a_max,
                 ATREE_NEXTMAX(VIRT vm_vpage_t,addr_semi,addr_level),
                 ATREE_NEXTLEVEL(addr_level));
   }
   ATREE_WALKMIN(VIRT vm_vpage_t,addr_semi,addr_level);
   node = node->vn_node.a_min;
   goto again;
  } else if (walk_max) {
   /* Recursively continue searching right. */
   ATREE_WALKMAX(VIRT vm_vpage_t,addr_semi,addr_level);
   node = node->vn_node.a_max;
   goto again;
  }
 }
}


#define vm_findany(vm,page_min,page_max) \
 ((vm)->vm_map ? impl_vm_findany((vm)->vm_map,page_min,page_max,\
                                  VM_SEMI0,VM_LEVEL0) : \
                 NULL)

PRIVATE struct vm_node *KCALL
impl_vm_findany(struct vm_node *__restrict node,
                vm_vpage_t page_min, vm_vpage_t page_max,
                ATREE_SEMI_T(VIRT vm_vpage_t) page_semi,
                ATREE_LEVEL_T page_level) {
 struct vm_node *result;
again:
 assert(node != node->vn_node.a_min);
 assert(node != node->vn_node.a_max);
 if (page_min <= node->vn_node.a_vmax &&
     page_max >= node->vn_node.a_vmin)
     return node;
 {
  bool walk_min,walk_max;
  walk_min = page_min <  page_semi && node->vn_node.a_min;
  walk_max = page_max >= page_semi && node->vn_node.a_max;
  if (walk_min) {
   /* Recursively continue searching left. */
   if (walk_max) {
    result = impl_vm_findany(node->vn_node.a_max,page_min,page_max,
                             ATREE_NEXTMAX(VIRT vm_vpage_t,page_semi,page_level),
                             ATREE_NEXTLEVEL(page_level));
    if (result) return result;
   }
   ATREE_WALKMIN(VIRT vm_vpage_t,page_semi,page_level);
   node = node->vn_node.a_min;
   goto again;
  } else if (walk_max) {
   /* Recursively continue searching right. */
   ATREE_WALKMAX(VIRT vm_vpage_t,page_semi,page_level);
   node = node->vn_node.a_max;
   goto again;
  }
 }
 return NULL;
}

PRIVATE void KCALL
impl_vm_pop_nodes(struct vm_node **__restrict pnode,
                  struct vm_node **__restrict presult,
                  vm_vpage_t page_min, vm_vpage_t page_max,
                  ATREE_SEMI_T(VIRT vm_vpage_t) addr_semi,
                  ATREE_LEVEL_T addr_level,
                  unsigned int mode, void *tag) {
 struct vm_node *node;
again:
 node = *pnode;
 if (!node) return;
 if (page_min <= node->vn_node.a_vmax &&
     page_max >= node->vn_node.a_vmin) {
  /* Check the closure tag if requested, to. */
  if (mode&VM_UNMAP_TAG &&
      node->vn_closure != tag)
      goto next;
  assertf(mode&VM_UNMAP_IMMUTABLE ||
          !(node->vn_flag&VM_NODE_FIMMUTABLE),
          "Cannot unmap immutable node %p...%p within %p...%p",
          VM_NODE_MINADDR(node),VM_NODE_MAXADDR(node),
          VM_PAGE2ADDR(page_min),VM_PAGE2ADDR(page_max+1)-1);
  /* Found a matching entry! */
#if 0
  if (VM_NODE_MIN(node) >= X86_KERNEL_BASE_PAGE)
      debug_printf("POP_NODE %p at %p...%p\n",
                   node,
                   VM_NODE_MINADDR(node),
                   VM_NODE_MAXADDR(node));
#endif
  asserte(vm_node_tree_pop_at(pnode,addr_semi,addr_level) == node);
  LIST_REMOVE(node,vn_byaddr);
  /* Add the removed node to the caller's list. */
  LIST_INSERT(*presult,node,vn_byaddr);
  goto again;
 }
next:
 { bool walk_min,walk_max;
   walk_min = page_min <  addr_semi && node->vn_node.a_min;
   walk_max = page_max >= addr_semi && node->vn_node.a_max;
   if (walk_min) {
    /* Recursively continue searching left. */
    if (walk_max) {
     impl_vm_pop_nodes(&node->vn_node.a_max,presult,page_min,page_max,
                       ATREE_NEXTMAX(VIRT vm_vpage_t,addr_semi,addr_level),
                       ATREE_NEXTLEVEL(addr_level),mode,tag);
    }
    ATREE_WALKMIN(VIRT vm_vpage_t,addr_semi,addr_level);
    pnode = &node->vn_node.a_min;
    goto again;
   } else if (walk_max) {
    /* Recursively continue searching right. */
    ATREE_WALKMAX(VIRT vm_vpage_t,addr_semi,addr_level);
    pnode = &node->vn_node.a_max;
    goto again;
   }
 }
}

/* Pop all nodes within the given page-range and
 * return a chain iterable through the `vn_byaddr'.
 * However, nodes are not unmapped during this process! */
PUBLIC struct vm_node *KCALL
vm_pop_nodes(struct vm *__restrict effective_vm,
             vm_vpage_t page_min, vm_vpage_t page_max,
             unsigned int mode, void *tag) {
 struct vm_node *result = NULL;
 impl_vm_pop_nodes(&effective_vm->vm_map,
                   &result,
                   page_min,page_max,
                   VM_SEMI0,
                   VM_LEVEL0,
                   mode,tag);
 return result;
}




PRIVATE size_t KCALL
impl_vm_protect(struct vm_node *__restrict node,
                vm_vpage_t page_min, vm_vpage_t page_max,
                ATREE_SEMI_T(VIRT vm_vpage_t) addr_semi,
                ATREE_LEVEL_T addr_level,
                vm_prot_t mask, vm_prot_t flag,
                unsigned int mode, void *tag) {
 size_t result = 0;
again:
 if (page_min <= node->vn_node.a_vmax &&
     page_max >= node->vn_node.a_vmin &&
  (!(mode&VM_PROTECT_TAG) || node->vn_closure == tag)) {
  vm_prot_t new_prot;
  /* Calculate the new protection. */
  new_prot = (node->vn_prot & mask) | flag;
  if (new_prot != node->vn_prot) {
   /* If protection changed, set the new values and remap the node. */
   node->vn_prot = new_prot;
   vm_map_node(node);
  }
  result += VM_NODE_SIZE(node);
 }
 { bool walk_min,walk_max;
   walk_min = page_min <  addr_semi && node->vn_node.a_min;
   walk_max = page_max >= addr_semi && node->vn_node.a_max;
   if (walk_min) {
    /* Recursively continue searching left. */
    if (walk_max) {
     result += impl_vm_protect(node->vn_node.a_max,page_min,page_max,
                               ATREE_NEXTMAX(VIRT vm_vpage_t,addr_semi,addr_level),
                               ATREE_NEXTLEVEL(addr_level),mask,flag,mode,tag);
    }
    ATREE_WALKMIN(VIRT vm_vpage_t,addr_semi,addr_level);
    node = node->vn_node.a_min;
    goto again;
   } else if (walk_max) {
    /* Recursively continue searching right. */
    ATREE_WALKMAX(VIRT vm_vpage_t,addr_semi,addr_level);
    node = node->vn_node.a_max;
    goto again;
   }
 }
 return result;
}
PUBLIC size_t KCALL
vm_protect(vm_vpage_t page_index, size_t num_pages,
           vm_prot_t mask, vm_prot_t flag,
           unsigned int mode, void *tag) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct vm *EXCEPT_VAR effective_vm;
 if unlikely(!num_pages) return 0;
 assert(page_index+num_pages > page_index);
 assert(page_index+num_pages <= VM_VPAGE_MAX+1);
 effective_vm = page_index >= X86_KERNEL_BASE_PAGE ? &vm_kernel : THIS_VM;
 vm_acquire(effective_vm);
 TRY {
  if unlikely(!effective_vm->vm_map)
      result = 0;
  else {
   /* Split VM nodes surrounding the requested area. */
   vm_split_before(effective_vm,page_index);
   vm_split_before(effective_vm,page_index+num_pages);
   /* Change protection of all affected nodes. */
   result = impl_vm_protect(effective_vm->vm_map,
                            page_index,
                            page_index+num_pages-1,
                            VM_SEMI0,
                            VM_LEVEL0,
                            mask,
                            flag,
                            mode,
                            tag);
   /* Try to merge the affected area again. */
   vm_merge_before(effective_vm,page_index+num_pages);
   vm_merge_before(effective_vm,page_index);
  }
 } FINALLY {
  vm_release(effective_vm);
 }
 return result;
}



INTERN struct vm_node *KCALL this_vm_getnode(vm_vpage_t page) {
 return vm_node_tree_locate(THIS_VM->vm_map,page);
}
PUBLIC struct vm_node *KCALL vm_getnode(vm_vpage_t page) {
 struct vm *effective_vm;
 effective_vm = page >= X86_KERNEL_BASE_PAGE ? &vm_kernel : THIS_VM;
 assert(vm_holding(effective_vm) || !PREEMPTION_ENABLED());
 return vm_node_tree_locate(effective_vm->vm_map,page);
}
FUNDEF struct vm_node *KCALL
vm_getanynode(vm_vpage_t min_page, vm_vpage_t max_page) {
 struct vm *effective_vm;
 effective_vm = min_page >= X86_KERNEL_BASE_PAGE ? &vm_kernel : THIS_VM;
 assertf((min_page >= X86_KERNEL_BASE_PAGE ? &vm_kernel : THIS_VM) == effective_vm,
         "Effective VM of virtual address range %p...%p is ambiguous",
         VM_PAGE2ADDR(min_page),VM_PAGE2ADDR(max_page+1)-1);
 assert(vm_holding(effective_vm));
 return vm_findany(effective_vm,min_page,max_page);
}





PUBLIC ATTR_NOTHROW void KCALL
vm_insert_node(struct vm *__restrict effective_vm,
               struct vm_node *__restrict node) {
 struct vm_node **pinsert,*insert_before;
#ifdef CONFIG_VM_USE_RWLOCK
 assertf(vm_holding(effective_vm),
         "effective_vm->vm_lock.rw_state  = %p\n"
         "effective_vm->vm_lock.rw_xowner = %p\n",
         effective_vm->vm_lock.rw_state,
         effective_vm->vm_lock.rw_xowner);
#else
 assert(vm_holding(effective_vm));
#endif
 assert(node->vn_region->vr_parts->vp_chain.le_pself ==
       &node->vn_region->vr_parts);
 assertf(VM_NODE_MIN(node) <= VM_NODE_MAX(node),
         "Unordered node: MIN(%p) >= MAX(%p)",
         VM_NODE_MINADDR(node),VM_NODE_MAXADDR(node));
 assertf(VM_NODE_MAX(node) <= VM_VPAGE_MAX,
         "Mapping of node covering pages %p...%p is out-of-bounds",
         VM_NODE_MIN(node),VM_NODE_MAX(node));
 assertf(!vm_findany(effective_vm,VM_NODE_MIN(node),VM_NODE_MAX(node)),
         "Some part of the node's memory range %p...%p is already in use by %p...%p",
         VM_NODE_MINADDR(node),VM_NODE_MAXADDR(node),
         VM_NODE_MINADDR(vm_findany(effective_vm,VM_NODE_MIN(node),VM_NODE_MAX(node))),
         VM_NODE_MAXADDR(vm_findany(effective_vm,VM_NODE_MIN(node),VM_NODE_MAX(node))));
 assert(node->vn_start < node->vn_region->vr_size);
 assertf(node->vn_start+VM_NODE_SIZE(node) <= node->vn_region->vr_size,
         "node->vn_start                    = %p\n"
         "VM_NODE_SIZE(node)                = %p\n"
         "node->vn_start+VM_NODE_SIZE(node) = %p\n"
         "node->vn_region->vr_size          = %p\n",
         node->vn_start,VM_NODE_SIZE(node),
         node->vn_start+VM_NODE_SIZE(node),
         node->vn_region->vr_size);
 assert((effective_vm->vm_map != NULL) ==
        (effective_vm->vm_byaddr != NULL));
#if 0
 if (VM_NODE_MIN(node) >= X86_KERNEL_BASE_PAGE)
     debug_printf("INSERT_NODE %p at %p...%p\n",
                  node,
                  VM_NODE_MINADDR(node),
                  VM_NODE_MAXADDR(node));
#endif
 vm_node_tree_insert(&effective_vm->vm_map,node);
 /* Figure out where we need to insert the node. */
 pinsert = &effective_vm->vm_byaddr;
 while ((insert_before = *pinsert) != NULL &&
        (insert_before->vn_node.a_vmax < node->vn_node.a_vmin))
         pinsert = &insert_before->vn_byaddr.le_next;
 assertf(!insert_before ||
          node->vn_node.a_vmax < insert_before->vn_node.a_vmin,
          "insert_before = %p:%p...%p\n"
          "node          = %p:%p...%p\n"
          "node->vn_flag = %#.4I16x\n",
          insert_before,VM_NODE_MINADDR(insert_before),VM_NODE_MAXADDR(insert_before),
          node,VM_NODE_MINADDR(node),VM_NODE_MAXADDR(node),
          node->vn_flag);
 /* Insert the node before `insert' at `pinsert' */
 node->vn_byaddr.le_pself = pinsert;
 node->vn_byaddr.le_next  = insert_before;
 if (insert_before)
     insert_before->vn_byaddr.le_pself = &node->vn_byaddr.le_next;
 *pinsert = node;
}

PUBLIC void KCALL
vm_map_node(struct vm_node *__restrict node) {
 struct vm_region *EXCEPT_VAR region; u16 perm;
 vm_raddr_t region_begin,region_end;
 region = node->vn_region;
 region_begin = node->vn_start;
 region_end   = region_begin+VM_NODE_SIZE(node);
 assertf(region_begin < region_end,
         "region_begin    = %p\n"
         "region_end      = %p\n",
         region_begin,region_end);
 assertf(region_end  <= region->vr_size,
         "region_begin    = %p\n"
         "region_end      = %p\n"
         "region->mr_size = %p\n",
         region_begin,region_end,region->vr_size);
 /* Don't touch page-directory mappings of reserved regions. */
 if unlikely(region->vr_type == VM_REGION_RESERVED)
    return;
 /* Determine mapping permissions. */
 perm = PAGEDIR_MAP_FUSER;
#if PROT_EXEC == PAGEDIR_MAP_FEXEC && \
    PROT_READ == PAGEDIR_MAP_FREAD && \
    PROT_WRITE == PAGEDIR_MAP_FWRITE
 perm |= node->vn_prot & (PROT_EXEC|PROT_READ|PROT_WRITE);
#else
 if (node->vn_prot & PROT_EXEC)
     perm |= PAGEDIR_MAP_FEXEC;
 if (node->vn_prot & PROT_READ)
     perm |= PAGEDIR_MAP_FREAD;
 if (node->vn_prot & PROT_WRITE)
     perm |= PAGEDIR_MAP_FWRITE;
#endif
 if (node->vn_prot & PROT_NOUSER)
     perm &= ~PAGEDIR_MAP_FUSER;

 mutex_get(&region->vr_lock);
 TRY {
  struct vm_part *part;
  VM_REGION_FOREACH_PART(part,region) {
   vm_raddr_t part_begin,part_end;
   vm_raddr_t using_begin,using_end; u16 part_prot;
   vm_vpage_t part_vpage; size_t part_size;
   part_begin = part->vp_start;
   part_end   = part->vp_chain.le_next ? part->vp_chain.le_next->vp_start : region->vr_size;
   assertf(part_end > part_begin,
           "part_begin             = %Iu\n"
           "part_end               = %Iu\n"
           "part                   = %p\n"
           "part->vp_chain.le_next = %p\n"
           "region->vr_parts       = %p"
           ,
           part_begin,part_end,
           part,
           part->vp_chain.le_next,
           region->vr_parts);
   if (part_end <= region_begin) continue; /* Ignore parts below the mapped range. */
   if (part_begin >= region_end) break;    /* Stop when the part is above the mapped range. */
   /* NOTE: The part may not begin/end where the branch does,
    *       simple because while the branch does require
    *       its own unique part references, the parts may
    *       have been merged again later, when neighboring
    *       reference counters matched up. */
   using_begin = MAX(part_begin,region_begin)-part_begin;
   using_end   = MIN(part_end,region_end)-part_begin;
   part_vpage  = node->vn_node.a_vmin+(MAX(part_begin,region_begin)-region_begin);
   assertf(part->vp_refcnt != 0,
           "How can this be zero? - We're using it!");
   assertf(using_end > using_begin,
           "part_begin   = %Iu\n"
           "part_end     = %Iu\n"
           "region_begin = %Iu\n"
           "region_end   = %Iu\n"
           "using_begin  = %Iu\n"
           "using_end    = %Iu\n",
           part_begin,part_end,
           region_begin,region_end,
           using_begin,using_end);
   part_prot  = perm;
   part_size  = (size_t)(using_end-using_begin);
#ifndef CONFIG_NO_VIO
   /* For VIO to function properly, its memory must not be mapped physically. */
   if (region->vr_type == VM_REGION_VIO)
       goto unmap_part;
#endif
   if (part->vp_state == VM_PART_INCORE) {
    size_t i;
    /* Take away write access, so we get information when a write happens. */
    if ((region->vr_flags&VM_REGION_FMONITOR) &&
       !(part->vp_flags&VM_PART_FCHANGED))
         part_prot &= ~PAGEDIR_MAP_FWRITE;
    /* Take away write access to allow for COW semantics. */
    if ((!(node->vn_prot&PROT_SHARED) ||
          (region->vr_flags & VM_REGION_FCANTSHARE)) &&
           part->vp_refcnt > 1 &&
           region->vr_type != VM_REGION_PHYSICAL)
           part_prot &= ~PAGEDIR_MAP_FWRITE;
    for (i = 0;; ++i) {
     vm_ppage_t phys_begin; size_t phys_size;
     assertf(i < part->vp_phys.py_num_scatter,
             "part_begin                           = %p\n"
             "part_end                             = %p\n"
             "using_begin                          = %p\n"
             "part_size                            = %Iu\n"
             "part->vp_phys.py_num_scatter         = %Iu\n"
             "part->vp_phys.py_iscatter[0].ps_size = %Iu\n",
             part_begin,part_end,
             using_begin,part_size,
             part->vp_phys.py_num_scatter,
             part->vp_phys.py_iscatter[0].ps_size);
     phys_begin = part->vp_phys.py_iscatter[i].ps_addr;
     phys_size  = part->vp_phys.py_iscatter[i].ps_size;
     /* Skip unused leading scatter entires. */
     if (using_begin != 0) {
      if (using_begin >= phys_size) {
       using_begin -= phys_size;
       continue;
      }
      phys_begin += using_begin;
      phys_size  -= using_begin;
      using_begin = 0;
     }
     /* Truncate the mapped size when only part of the scatter entry is mapped. */
     if (phys_size > part_size)
         phys_size = part_size;
     /* Actually map the memory part. */
     pagedir_map(part_vpage,phys_size,phys_begin,part_prot);

     part_size  -= phys_size;
     if (!part_size) break; /* Stop when everything has been mapped. */
     part_vpage += phys_size;
    }
   } else {
#ifndef CONFIG_NO_VIO
unmap_part:
#endif
    /* Delete the mapping. - this one's using LOA semantics. */
    pagedir_map(part_vpage,part_size,0,PAGEDIR_MAP_FUNMAP);
   }
  }
 } FINALLY {
  mutex_put(&region->vr_lock);
 }
}

PUBLIC void KCALL
vm_insert_and_activate_node(struct vm *__restrict effective_vm,
                            struct vm_node *__restrict node) {
 assert(vm_holding(effective_vm));
 vm_map_node(node);
 vm_insert_node(effective_vm,node);
}

INTDEF void KCALL
vmapps_destroy(struct vmapps *__restrict self);


PUBLIC void KCALL vm_unmap_userspace(void) {
 struct vm *EXCEPT_VAR uservm = THIS_VM;
 struct vm_node *nodes,*next;
 assert(uservm != &vm_kernel);
 vm_acquire(uservm);
 TRY {
  /* Unmap the entirety of user-space. */
  pagedir_map(0,X86_KERNEL_BASE_PAGE,0,PAGEDIR_MAP_FUNMAP);

  /* Extract nodes. */
  nodes                = uservm->vm_byaddr;
  uservm->vm_map       = NULL;
  uservm->vm_byaddr    = NULL;
  /* Delete all VM Nodes that were extracted. */
  while (nodes) {
   next = nodes->vn_byaddr.le_next;
   /* Invoke the notification callback (if it exists) */
   if (nodes->vn_notify) {
    INVOKE_NOTIFY_V(nodes->vn_notify,
                    nodes->vn_closure,
                    VM_NOTIFY_UNMAP,
                    VM_NODE_BEGIN(nodes),
                    VM_NODE_SIZE(nodes),NULL);
    INVOKE_NOTIFY_V(nodes->vn_notify,
                    nodes->vn_closure,
                    VM_NOTIFY_DECREF,
                    0,
                    0,
                    NULL);
   }
   vm_region_decref_range(nodes->vn_region,
                          nodes->vn_start,
                          VM_NODE_SIZE(nodes));
   vm_region_decref(nodes->vn_region);
   vm_node_free(nodes);
   nodes = next;
  }

  /* No more VM apps. */
  {
   struct vmapps *apps;
   apps = FORVM(uservm,vm_apps);
   if (apps) {
    assert(apps->va_share != 0);
    if (apps->va_share > 1) {
     /* Apps are being shared with another VM. */
     if unlikely(ATOMIC_DECFETCH(apps->va_share) == 0)
        vmapps_destroy(apps);
     FORVM(uservm,vm_apps) = NULL;
    } else {
     /* We're the only vm using this set of apps.
      * With that in mind, we can simply clear the cache and re-use the same buffer:
      * -> optimization to prevent allocation of a new buffer when linking a new application. */
     atomic_rwlock_write(&apps->va_lock);
     while (apps->va_count) {
      WEAK REF struct application *app;
      app = apps->va_apps[--apps->va_count];
      atomic_rwlock_endwrite(&apps->va_lock);
      application_weak_decref(app);
      atomic_rwlock_write(&apps->va_lock);
     }
     atomic_rwlock_endwrite(&apps->va_lock);
    }
   }
  }
 } FINALLY {
  vm_release(uservm);
 }
}

PUBLIC size_t FCALL
vm_unswap(vm_vpage_t page_index, size_t num_pages) {
 size_t result = 0; struct vm *EXCEPT_VAR effective_vm;
 assert(page_index+num_pages >= page_index);
 assert(page_index+num_pages <= VM_VPAGE_MAX+1);
 if unlikely(!num_pages) goto done;
 effective_vm = page_index >= X86_KERNEL_BASE_PAGE ? &vm_kernel : THIS_VM;
 vm_acquire(effective_vm);
 TRY {
  /* TODO */

 } FINALLY {
  vm_release(effective_vm);
 }
done:
 return result;
}

#ifndef CONFIG_NO_SMP
PUBLIC void FCALL
vm_sync(vm_vpage_t page_index, size_t num_pages) {
 kernel_cpuset_t set = KERNEL_CPUSET_INIT;
 struct vm *myvm = THIS_VM;
 struct task *iter; cpuid_t id;
 atomic_rwlock_read(&myvm->vm_tasklock);
 /* Collect the IDs of all CPUs hosting threads using this VM.
  * NOTE: If this set changes after we read it, that's still
  *       OK because any new thread starting to be hosted by
  *       some CPU will necessarily have its page directory
  *       invalidated during the first context switch. */
 for (iter = myvm->vm_tasks;
      iter; iter = iter->t_vmtasks.le_next)
      kernel_cpuset_add(set,iter->t_cpu->cpu_id);
 atomic_rwlock_endread(&myvm->vm_tasklock);
 /* Send an IPI, requesting page invalidation to all CPUs in the set. */
 KERNEL_CPUSET_FOREACH(set,id) {
  struct x86_ipi ipi;
  ipi.ipi_type                 = X86_IPI_INVLPG;
  ipi.ipi_flag                 = X86_IPI_FNORMAL;
  ipi.ipi_invlpg.ivp_pagedir   = &myvm->vm_pagedir;
  ipi.ipi_invlpg.ivp_pageindex = page_index;
  ipi.ipi_invlpg.ivp_numpages  = num_pages;
  x86_ipi_send(cpu_vector[id],&ipi);
 }
}

PUBLIC void FCALL vm_syncone(vm_vpage_t page_index) {
 kernel_cpuset_t set = KERNEL_CPUSET_INIT;
 struct vm *myvm = THIS_VM;
 struct task *iter; cpuid_t id;
 atomic_rwlock_read(&myvm->vm_tasklock);
 /* Collect the IDs of all CPUs hosting threads using this VM.
  * NOTE: If this set changes after we read it, that's still
  *       OK because any new thread starting to be hosted by
  *       some CPU will necessarily have its page directory
  *       invalidated during the first context switch. */
 for (iter = myvm->vm_tasks;
      iter; iter = iter->t_vmtasks.le_next)
      kernel_cpuset_add(set,iter->t_cpu->cpu_id);
 atomic_rwlock_endread(&myvm->vm_tasklock);
 /* Send an IPI, requesting page invalidation to all CPUs in the set. */
 KERNEL_CPUSET_FOREACH(set,id) {
  struct x86_ipi ipi;
  ipi.ipi_type                 = X86_IPI_INVLPG_ONE;
  ipi.ipi_flag                 = X86_IPI_FNORMAL;
  ipi.ipi_invlpg.ivp_pagedir   = &myvm->vm_pagedir;
  ipi.ipi_invlpg.ivp_pageindex = page_index;
  x86_ipi_send(cpu_vector[id],&ipi);
 }
}

PUBLIC void FCALL vm_syncall(void) {
 struct x86_ipi ipi;
 ipi.ipi_type = X86_IPI_INVLPG_ALL;
 ipi.ipi_flag = X86_IPI_FNORMAL;
 x86_ipi_broadcast(&ipi,true);
}
#endif


#define IMPL_VM_GETFREE_FAILED (VM_VPAGE_MAX+1)
PRIVATE vm_vpage_t KCALL
impl_vm_getfree(struct vm *__restrict effective_vm,
                vm_vpage_t hint, size_t num_pages,
                size_t min_alignment_in_pages,
                size_t min_gap_size, bool below) {
 vm_vpage_t candidate;
 struct node_info info;
 /* Step #1: Find the branch (if any) for the given hint.
  *          >> If no such branch exists, we can simply
  *             return the hint for perfect behavior! */
 info.ni_page_min = hint;
 info.ni_page_max = hint+num_pages-1;
 if unlikely(info.ni_page_max < info.ni_page_min ||
             info.ni_page_max > VM_VPAGE_MAX) {
  /* Special case: Overflowing hint. */
  goto fail;
 }
 info.ni_min = NULL;
 info.ni_max = NULL;
 vm_findnodes(&info,effective_vm->vm_map,
              VM_SEMI0,
              VM_LEVEL0);
 assert((info.ni_min != NULL) ==
        (info.ni_max != NULL));
 /* Simple case: No branch exists. */
 if (!info.ni_min) {
  if (min_gap_size) {
   /* TODO: Check if the gap can be ensured. */
  }
  return hint;
 }
 assertf(info.ni_min_min <= (hint+num_pages-1),
         "info.ni_min_min  = %p\n"
         "info.ni_max_max  = %p\n"
         "hint             = %p\n"
         "hint+num_pages-1 = %p\n"
         "num_pages        = %p",
         info.ni_min_min,info.ni_max_max,
         hint,hint+num_pages-1,num_pages);
 assertf(info.ni_max_max >= hint,
         "info.ni_min_min  = %p\n"
         "info.ni_max_max  = %p\n"
         "hint             = %p\n"
         "hint+num_pages-1 = %p\n"
         "num_pages        = %p",
         info.ni_min_min,info.ni_max_max,
         hint,hint+num_pages-1,num_pages);
 /* Step #2: based on `mode&MMAN_FINDSPACE_BELOW', search from info.ni_min/info.ni_max
  *          upwards/downwards until a free memory range of sufficient
  *          size was found, using the ordered branch chain. */
 if (below) {
  struct vm_node *iter;
  iter = info.ni_min;
  for (;;) {
   vm_vpage_t prev_end = 0;
   bool has_prev;
   has_prev = VM_NODE_HASPREV(iter,effective_vm);
   if (has_prev) {
    struct vm_node *prev_branch = VM_NODE_PREV(iter);
    prev_end = VM_NODE_END(prev_branch);
    /* Enforce padding before the next branch. */
    if (prev_branch->vn_region->vr_type == VM_REGION_HIGUARD)
        prev_end += min_gap_size;
   }
   candidate = VM_NODE_BEGIN(iter);
   if (iter->vn_region->vr_type == VM_REGION_HIGUARD)
       candidate -= min_gap_size;
   if (candidate > VM_NODE_BEGIN(iter)) break; /* Stop on overflow. */
   if ((candidate -= num_pages) > VM_NODE_BEGIN(iter)) break; /* Stop on overflow. */
   candidate = FLOOR_ALIGN(candidate,min_alignment_in_pages);
   if (candidate >= prev_end
#ifdef CONFIG_LOW_KERNEL
       &&
      (effective_vm == &vm_kernel ||
       candidate >= KERNEL_END_PAGE)
#endif
       ) {
    assertf(!vm_getanynode(candidate,candidate+num_pages-1),
            "vm_getfree() is borken!\n"
            "Already in use: %p...%p\n"
            "NODE:           %p...%p\n"
            "prev_end      = %p\n"
            "iter          = %p...%p\n"
            "iter->prev    = %p...%p",
            VM_PAGE2ADDR(candidate),
            VM_PAGE2ADDR(candidate+num_pages)-1,
            VM_NODE_MINADDR(vm_getanynode(candidate,candidate+num_pages-1)),
            VM_NODE_MAXADDR(vm_getanynode(candidate,candidate+num_pages-1)),
            VM_PAGE2ADDR(prev_end),
            VM_NODE_MINADDR(iter),
            VM_NODE_MAXADDR(iter),
            VM_NODE_HASPREV(iter,effective_vm) ? VM_NODE_MINADDR(VM_NODE_PREV(iter)) : 0,
            VM_NODE_HASPREV(iter,effective_vm) ? VM_NODE_MAXADDR(VM_NODE_PREV(iter)) : 0);
    goto winner;
   }
   if (!has_prev) break;
   iter = VM_NODE_PREV(iter);
  }
 } else {
  struct vm_node *iter;
  iter = info.ni_max;
  for (;;) {
   vm_vpage_t next_begin = 0;
   bool has_next;
   has_next = VM_NODE_HASNEXT(iter,effective_vm);
   if (has_next) {
    struct vm_node *next_branch = VM_NODE_NEXT(iter);
    next_begin = VM_NODE_BEGIN(next_branch);
    /* Enforce padding before the next branch. */
    if (next_branch->vn_region->vr_type == VM_REGION_LOGUARD)
        next_begin -= min_gap_size;
   }
   candidate = VM_NODE_END(iter);
   if (iter->vn_region->vr_type == VM_REGION_HIGUARD)
       candidate += min_gap_size;
   candidate = CEIL_ALIGN(candidate,min_alignment_in_pages);
   next_begin -= candidate;
   if (num_pages <= next_begin
#ifdef CONFIG_HIGH_KERNEL
       &&
      (effective_vm == &vm_kernel ||
       candidate+num_pages <= X86_KERNEL_BASE_PAGE)
#endif
       ) {
    assertf(!vm_getanynode(candidate,candidate+num_pages-1),
            "vm_getfree() is borken!\n"
            "Already in use: %p...%p\n"
            "NODE:           %p...%p\n",
            VM_PAGE2ADDR(candidate),
            VM_PAGE2ADDR(candidate+num_pages)-1,
            VM_NODE_MINADDR(vm_getanynode(candidate,candidate+num_pages-1)),
            VM_NODE_MAXADDR(vm_getanynode(candidate,candidate+num_pages-1)));
    goto winner;
   }
   if (!has_next) break;
   iter = VM_NODE_NEXT(iter);
  }
 }
fail:
 return IMPL_VM_GETFREE_FAILED;
winner:
 /* Check for address space overflows. */
 if unlikely((candidate + num_pages) < candidate)
    goto fail;
 if unlikely((candidate + num_pages) > VM_VPAGE_MAX+1)
    goto fail;
 /* Make sure that the given address is suitable. */
 if (effective_vm == &vm_kernel) {
  if (candidate < X86_KERNEL_BASE_PAGE)
      goto fail;
 } else {
  if (candidate+num_pages >= X86_KERNEL_BASE_PAGE)
      goto fail;
 }
 assertf(!vm_getanynode(candidate,candidate+num_pages-1),
         "vm_getfree() is borken!\n"
         "Already in use: %p...%p\n"
         "NODE:           %p...%p\n",
         VM_PAGE2ADDR(candidate),
         VM_PAGE2ADDR(candidate+num_pages)-1,
         VM_NODE_MINADDR(vm_getanynode(candidate,candidate+num_pages-1)),
         VM_NODE_MAXADDR(vm_getanynode(candidate,candidate+num_pages-1)));
 return candidate;
}


PUBLIC vm_vpage_t KCALL
vm_getfree(vm_vpage_t hint, size_t num_pages,
           size_t min_alignment_in_pages,
           size_t min_gap_size, unsigned int mode) {
 vm_vpage_t result;
 struct exception_info *info;
 struct vm *effective_vm;
 effective_vm = &vm_kernel;
 /* When searching downwards `hint' specifies the max-end-address.
  * So to translate it into the first checked start-address,
  * subtract `num_pages' */
 if (mode & VM_GETFREE_FBELOW) {
  if unlikely(hint < num_pages) {
   if (!hint) {
    hint = (VM_VPAGE_MAX+1) - num_pages;
   } else {
    if (mode & VM_GETFREE_FSTRICT)
        goto no_such_page;
    hint  = 0;
    mode &= VM_GETFREE_FBELOW;
   }
  } else {
   hint -= num_pages;
  }
 }
 if (hint < X86_KERNEL_BASE_PAGE ||
    (hint == X86_KERNEL_BASE_PAGE && (mode&VM_GETFREE_FBELOW)))
     effective_vm = THIS_VM;
 assert(vm_holding(effective_vm));
 /* Check if the request is too large to ever map. */
 if unlikely(num_pages > VM_VPAGE_MAX+1)
    goto no_such_page;
 /* Check for special cases: Empty range/empty map. */
 if unlikely(!num_pages || !effective_vm->vm_map) {
  /* Check for an overflowing address range. */
  if ((hint + num_pages) < hint ||
      (hint + num_pages) > VM_VPAGE_MAX+1) {
   /* If we're not allowed to move the hint downwards, fail immediately. */
   if (mode & VM_GETFREE_FSTRICT &&
     !(mode & VM_GETFREE_FBELOW))
       goto no_such_page;
   /* Move the hint downwards, clamping the
    * address range to the end of the VM. */
   hint = (VM_VPAGE_MAX+1)-num_pages;
   if (effective_vm == &vm_kernel) {
    if (hint < X86_KERNEL_BASE_PAGE)
        goto no_such_page;
   } else {
    if (hint+num_pages >= X86_KERNEL_BASE_PAGE)
        goto no_such_page;
   }
  }
  result = hint;
  goto done;
 }
retry:
 result = impl_vm_getfree(effective_vm,hint,num_pages,
                          min_alignment_in_pages,min_gap_size,
                       !!(mode&VM_GETFREE_FBELOW));
 if (result != IMPL_VM_GETFREE_FAILED) goto done;
 if (!(mode&VM_GETFREE_FSTRICT)) {
  /* Try searching in the opposite direction. */
  result = impl_vm_getfree(effective_vm,hint,num_pages,
                           min_alignment_in_pages,min_gap_size,
                         !(mode&VM_GETFREE_FBELOW));
  if (result != IMPL_VM_GETFREE_FAILED) goto done;
 }
 if (!(mode&VM_GETFREE_FFORCEGAP) && min_gap_size != 0) {
  min_gap_size = 0;
  goto retry;
 }
no_such_page:
 /* Throw a bad-allocation error. */
 info                         = error_info();
 info->e_error.e_code                 = E_BADALLOC;
 info->e_error.e_flag                 = ERR_FRESUMABLE;
 info->e_error.e_badalloc.ba_resource = ERROR_BADALLOC_VIRTMEMORY;
 info->e_error.e_badalloc.ba_amount   = num_pages * PAGESIZE;
 error_throw_current();
 goto retry;
done:
 assertf(!vm_getanynode(result,result+num_pages-1),
         "vm_getfree() is borken!\n"
         "Already in use: %p...%p\n"
         "NODE:           %p...%p\n",
         VM_PAGE2ADDR(result),
         VM_PAGE2ADDR(result+num_pages)-1,
         VM_NODE_MINADDR(vm_getanynode(result,result+num_pages-1)),
         VM_NODE_MAXADDR(vm_getanynode(result,result+num_pages-1)));
 return result;
}


PUBLIC size_t KCALL
vm_unmap(vm_vpage_t page, size_t num_pages,
         unsigned int mode, void *tag) {
 unsigned int EXCEPT_VAR xmode = mode;
 size_t result = 0;
 struct vm *EXCEPT_VAR effective_vm;
 struct vm_node *EXCEPT_VAR nodes = NULL;
 vm_vpage_t unmap_min = (vm_vpage_t)-1;
 vm_vpage_t unmap_max = 0;
 if unlikely(!num_pages) goto done;
 assert(page+num_pages > page);
 assert(page+num_pages <= VM_VPAGE_MAX+1);
 effective_vm = page >= X86_KERNEL_BASE_PAGE ? &vm_kernel : THIS_VM;
 if (mode & VM_UNMAP_NOEXCEPT)
     task_nothrow_serve();
 vm_acquire(effective_vm);
 TRY {
  struct vm_node *iter;
  TRY {
   /* Split the address space where the unmap should take place. */
   vm_split_before(effective_vm,page);
   vm_split_before(effective_vm,page+num_pages);
  } CATCH (E_BADALLOC) {
   if (mode & VM_UNMAP_NOEXCEPT) {
    error_handled();
    goto done_unlock;
   }
   error_rethrow();
  }
  /* Pop all nodes in the affected range. */
  nodes = vm_pop_nodes(effective_vm,page,page+num_pages-1,mode,tag);
  if (nodes) unmap_min = VM_NODE_MIN(nodes);
  iter = nodes;
  TRY {
   /* Try to unmap the nodes that were removed. */
   for (; iter; iter = iter->vn_byaddr.le_next) {
    if (unmap_max < VM_NODE_MAX(iter))
        unmap_max = VM_NODE_MAX(iter);
    pagedir_map(VM_NODE_BEGIN(iter),
                VM_NODE_SIZE(iter),
                0,PAGEDIR_MAP_FUNMAP);
    /* Sum the total number of unmapped pages. */
    result += VM_NODE_SIZE(iter);
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   TRY {
    struct vm_node *temp = nodes;
    /* Try to re-map all nodes that were already unmapped. */
    for (; temp != iter; temp = temp->vn_byaddr.le_next)
        vm_map_node(temp);
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   }

   /* Restore all nodes that were popped. */
   while (nodes) {
    struct vm_node *next = nodes->vn_byaddr.le_next;
    vm_insert_node(effective_vm,nodes);
    nodes = next;
   }
   result = 0;
   if (mode & VM_UNMAP_NOEXCEPT)
       goto done_unlock;
   error_rethrow();
  }
done_unlock:;
 } FINALLY {
  /* If the sync flag is set, automatically sync
   * unmapped memory before unlocking the VM. */
  if (xmode & VM_UNMAP_SYNC)
      vm_sync(unmap_min,(unmap_max-unmap_min)+1);
  vm_release(effective_vm);
  /* Drop all nodes that were unmapped. */
  while (nodes) {
   struct vm_node *next = nodes->vn_byaddr.le_next;
   if (nodes->vn_notify) {
    INVOKE_NOTIFY_V(nodes->vn_notify,
                    nodes->vn_closure,
                    VM_NOTIFY_UNMAP,
                    VM_NODE_BEGIN(nodes),
                    VM_NODE_SIZE(nodes),
                    NULL);
    INVOKE_NOTIFY_V(nodes->vn_notify,
                    nodes->vn_closure,
                    VM_NOTIFY_DECREF,
                    0,
                    0,
                    NULL);
   }
   vm_region_decref_range(nodes->vn_region,
                          nodes->vn_start,
                          VM_NODE_SIZE(nodes));
   vm_region_decref(nodes->vn_region);
   vm_node_free(nodes);
   nodes = next;
  }
  if (xmode & VM_UNMAP_NOEXCEPT)
      task_nothrow_end();
 }
done:
 return result;
}

PUBLIC void KCALL
vm_maps_chprot(struct vm_maps self,
               vm_prot_t prot_mask,
               vm_prot_t prot_flag) {
 struct vm_node *iter;
 if (prot_mask == (vm_prot_t)~0 &&
     prot_flag == (vm_prot_t)0)
     return; /* No-op */
 iter = self.v_maps;
 for (; iter; iter = iter->vn_byaddr.le_next) {
  iter->vn_prot &= prot_mask;
  iter->vn_prot |= prot_flag;
 }
}


PUBLIC size_t KCALL
vm_remap(vm_vpage_t old_page_index, size_t num_pages,
         vm_vpage_t new_page_index, vm_prot_t prot_mask,
         vm_prot_t prot_flag, unsigned int mode, void *tag) {
 struct vm_maps EXCEPT_VAR maps;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 STATIC_ASSERT(VM_REMAP_TAG == VM_EXTRACT_TAG);
 STATIC_ASSERT(VM_REMAP_FULL == VM_EXTRACT_FULL);
 maps = vm_extract(old_page_index,num_pages,mode,tag);
 TRY {
  vm_maps_chprot(maps,prot_mask,prot_flag);
  result = vm_restore(new_page_index,maps,VM_RESTORE_NORMAL);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  vm_maps_delete(maps);
  error_rethrow();
 }
 return result;
}

/* Delete VM mappings after they have been extracted. */
PUBLIC void KCALL vm_maps_delete(struct vm_maps self) {
 struct vm_node *next,*iter;
 iter = self.v_maps;
 while (iter) {
  next = iter->vn_byaddr.le_next;
  if (iter->vn_notify) {
   INVOKE_NOTIFY_V(iter->vn_notify,
                   iter->vn_closure,
                   VM_NOTIFY_DELETE,
                   0,
                   0,
                   NULL);
   INVOKE_NOTIFY_V(iter->vn_notify,
                   iter->vn_closure,
                   VM_NOTIFY_DECREF,
                   0,
                   0,
                   NULL);
  }
  vm_region_decref_range(iter->vn_region,
                         iter->vn_start,
                         VM_NODE_SIZE(iter));
  vm_region_decref(iter->vn_region);
  vm_node_free(iter);
  iter = next;
 }
}

/* Extract all nodes from the given address range and return them.
 * This function is similar to `vm_unmap()' in that it unmaps memory,
 * however rather than deleting extracted mappings, it instead returns
 * them as a structure that can be used to `vm_restore()' them elsewhere.
 * @param: page_index: The starting page index.
 * @param: num_pages:  The number of consecutive page index locations to search, starting at `page_index'
 * @param: mode:       Set of `VM_EXTRACT_*'
 * @param: tag:        Tag used to select which mappings to extract. */
PUBLIC struct vm_maps KCALL
vm_extract(VIRT vm_vpage_t page_index, size_t num_pages,
           unsigned int mode, void *tag) {
 VIRT vm_vpage_t EXCEPT_VAR xpage_index = page_index;
 STATIC_ASSERT(VM_UNMAP_TAG == VM_EXTRACT_TAG);
 struct vm_maps EXCEPT_VAR result;
 struct vm *EXCEPT_VAR effective_vm;
 result.v_maps = NULL;
 if unlikely(!num_pages) goto done;
 assert(page_index+num_pages > page_index);
 assert(page_index+num_pages <= VM_VPAGE_MAX+1);
 effective_vm = page_index >= X86_KERNEL_BASE_PAGE ? &vm_kernel : THIS_VM;
 vm_acquire(effective_vm);
 TRY {
  struct vm_node *EXCEPT_VAR iter;
  /* Split the address space where the unmap should take place. */
  vm_split_before(effective_vm,page_index);
  vm_split_before(effective_vm,page_index+num_pages);
  /* Pop all nodes in the affected range. */
  result.v_maps = vm_pop_nodes(effective_vm,
                               page_index,
                               page_index+num_pages-1,
                               mode,
                               tag);
  if (mode & VM_EXTRACT_FULL) {
   /* Make sure that everything has been mapped. */
   iter = result.v_maps;
   if (iter->vn_node.a_vmin != page_index) {
incomplete_unmap:
    /* Restore all nodes that were popped. */
    while (result.v_maps) {
     struct vm_node *next;
     next = result.v_maps->vn_byaddr.le_next;
     vm_insert_node(effective_vm,result.v_maps);
     result.v_maps = next;
    }
    goto done_unlock;
   }
   for (;;) {
    struct vm_node *next;
    next = iter->vn_byaddr.le_next;
    if (!next) break;
    if (iter->vn_node.a_vmax !=
        next->vn_node.a_vmin-1)
        goto incomplete_unmap;
    iter = next;
   }
   if (iter->vn_node.a_vmax != page_index+num_pages-1)
       goto incomplete_unmap;
  }

  iter = result.v_maps;
  TRY {
   /* Try to unmap the nodes that were removed. */
   for (; iter; iter = iter->vn_byaddr.le_next) {
    if (iter->vn_notify) {
     INVOKE_NOTIFY_V(iter->vn_notify,
                     iter->vn_closure,
                     VM_NOTIFY_EXTRACT,
                     VM_NODE_BEGIN(iter),
                     VM_NODE_SIZE(iter),
                     NULL);
    }
    pagedir_map(VM_NODE_BEGIN(iter),
                VM_NODE_SIZE(iter),
                0,
                PAGEDIR_MAP_FUNMAP);
    /* Adjust the relative node position. */
    iter->vn_node.a_vmin -= page_index;
    iter->vn_node.a_vmax -= page_index;
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   TRY {
    struct vm_node *temp = result.v_maps;
    /* Try to re-map all nodes that were already unmapped. */
    for (; temp != iter; temp = temp->vn_byaddr.le_next) {
     /* Restore the relative node position. */
     temp->vn_node.a_vmin += xpage_index;
     temp->vn_node.a_vmax += xpage_index;
     if (temp->vn_notify) {
      INVOKE_NOTIFY_V(temp->vn_notify,
                      temp->vn_closure,
                      VM_NOTIFY_RESTORE,
                      VM_NODE_BEGIN(temp),
                      VM_NODE_SIZE(temp),
                      NULL);
     }
     vm_map_node(temp);
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   }

   /* Restore all nodes that were popped. */
   while (result.v_maps) {
    struct vm_node *next;
    next = result.v_maps->vn_byaddr.le_next;
    vm_insert_node(effective_vm,result.v_maps);
    result.v_maps = next;
   }
   error_rethrow();
  }
done_unlock:
  ;
 } FINALLY {
  vm_release(effective_vm);
 }
done:
 return result;
}


/* Restore previously extracted VM mappings, now offset from `page_index'.
 * WARNING: After a successful call to this function, you mustn't call
 *         `vm_maps_delete()', as this function will inherit the saved
 *          nodes.
 * NOTE: Existing mappings within the associated address range are deleted.
 * @param: mode: One of `VM_RESTORE_*'
 * @return: * :  The actual number of restored pages. */
FUNDEF size_t KCALL
vm_restore(VIRT vm_vpage_t page_index,
           struct vm_maps maps,
           unsigned int mode) {
 VIRT vm_vpage_t EXCEPT_VAR xpage_index = page_index;
 struct vm_maps EXCEPT_VAR xmaps = maps;
 struct vm *EXCEPT_VAR effective_vm;
 struct vm_node *EXCEPT_VAR iter;
 struct vm_node *next;
 size_t num_pages;
 assert(mode == VM_RESTORE_NORMAL ||
        mode == VM_RESTORE_NEWVM);
 iter = maps.v_maps;
 if unlikely(!iter)
    return 0; /* nothing to do here... */
 /* Adjust the relative position of all nodes to-be restored. */
 for (;;) {
  iter->vn_node.a_vmin += page_index;
  iter->vn_node.a_vmax += page_index;
  next = iter->vn_byaddr.le_next;
  if (!next) break;
  assert(next->vn_node.a_vmin >
        (iter->vn_node.a_vmax-page_index));
  iter = next;
 }
 /* The end address of the last node+1 is the
  * number of consecutive pages to restore. */
 num_pages  = iter->vn_node.a_vmax+1;
 num_pages -= page_index; /* Re-adjust, because we already added this before. */

 TRY {
  /* Determine the effective VM. */
  effective_vm = page_index >= X86_KERNEL_BASE_PAGE ? &vm_kernel : THIS_VM;
  vm_acquire(effective_vm);
  TRY {
   /* Unmap all old memory in the range we're about to restore. */
   vm_unmap(page_index,
            num_pages,
            VM_UNMAP_NORMAL,
            NULL);
   /* With us holding a lock, and the target range now
    * clear, we can proceed with the restore operation. */
   iter = maps.v_maps;
   TRY {
    while (iter) {
     /* Signal the restore. */
     if (iter->vn_notify) {
      INVOKE_NOTIFY_V(iter->vn_notify,
                      iter->vn_closure,
                      mode,
                      VM_NODE_BEGIN(iter),
                      VM_NODE_SIZE(iter),
                      NULL);
     }
     iter = iter->vn_byaddr.le_next;
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    next = xmaps.v_maps;
    while (next != iter) {
     if (next->vn_notify) {
      INVOKE_NOTIFY_V(next->vn_notify,
                      next->vn_closure,
                      VM_NOTIFY_EXTRACT,
                      VM_NODE_BEGIN(next),
                      VM_NODE_SIZE(next),
                      NULL);
     }
     next = next->vn_byaddr.le_next;
    }
    error_rethrow();
   }
   /* With the restore signaled, insert all the node. */
   iter = maps.v_maps;
   while (iter) {
    next = iter->vn_byaddr.le_next;
    vm_insert_node(effective_vm,iter);
    iter = next;
   }
   /* NOTE: Because a restore is a positive mapping, as well
    *       as the fact that we can't create page directory
    *       mappings ~right now~ (because that might fail
    *       and we need to stain noexcept from here on),
    *       we simply leave the modes as they are right now.
    *       We know that the address range has been unmapped,
    *       so we can just assume that pagefaults will lazily
    *       create mappings as data is accessed. */
  } FINALLY {
   vm_release(effective_vm);
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Restore the original node relations. */
  iter = xmaps.v_maps;
  for (; iter; iter = iter->vn_byaddr.le_next) {
   iter->vn_node.a_vmin -= xpage_index;
   iter->vn_node.a_vmax -= xpage_index;
  }
  error_rethrow();
 }
 return num_pages;
}




#define VM_LOADNODE_UNCHANGED 0 /* Nothing changed */
#define VM_LOADNODE_CHANGED   1 /* The node has been loaded */
#define VM_LOADNODE_REMAPPED  2 /* The node has been loaded, and the VM layout may have changed */


INTDEF int KCALL
impl_vm_node_loadcore(struct vm_node *__restrict node,
                      struct vm *__restrict effective_vm,
                      vm_raddr_t region_starting_page,
                      size_t num_region_pages,
                      unsigned int mode);


PRIVATE int KCALL
impl_vm_loadcore(struct vm_node *__restrict node,
                 struct vm *__restrict effective_vm,
                 VIRT vm_vpage_t page_min,
                 VIRT vm_vpage_t page_max,
                 ATREE_SEMI_T(VIRT vm_vpage_t) page_semi,
                 ATREE_LEVEL_T page_level, unsigned int mode) {
 int temp,result = VM_LOADNODE_UNCHANGED;
 bool walk_min,walk_max;
 struct vm_node *min_branch;
 struct vm_node *max_branch;
again:
 assert(page_min <= page_max);
 min_branch = node->vn_node.a_min;
 max_branch = node->vn_node.a_max;
 walk_min = page_min <  page_semi && min_branch != NULL;
 walk_max = page_max >= page_semi && max_branch != NULL;

 if (page_min <= node->vn_node.a_vmax &&
     page_max >= node->vn_node.a_vmin) {
  vm_raddr_t region_begin,region_end;
  size_t region_size;
  /* Found a matching entry!
   * >> Figure out how much of this must be loaded into the core. */
  assert(page_min >= node->vn_node.a_vmin);
  assert(page_max <= node->vn_node.a_vmax);
  region_begin = node->vn_start+(page_min-node->vn_node.a_vmin);
  region_end   = node->vn_start+((page_max+1)-node->vn_node.a_vmin);
  region_size  = region_end-region_begin;
  /* Load the affected portion of this mapping. */
  temp = impl_vm_node_loadcore(node,effective_vm,region_begin,
                               region_size,mode);
  /* This check is required when `impl_vm_node_loadcore' remaps branches
   * in a way that would otherwise make us skip a whole bunch of branches. */
  if (temp) {
   result = temp;
   /* If something got remapped, or if we're only
    * loading a single page, return to the caller. */
   if (temp == VM_LOADNODE_REMAPPED ||
       page_min == page_max)
       goto end;
  }
 }

 if (walk_min) {
  /* Recursively continue searching left. */
  if (walk_max) {
   temp = impl_vm_loadcore(max_branch,effective_vm,page_min,page_max,
                           ATREE_NEXTMAX(VIRT vm_vpage_t,page_semi,page_level),
                           ATREE_NEXTLEVEL(page_level),mode);
   /* Update the result value. */
   if (result < temp)
       result = temp;
   /* If something got re-mapped, or the only page got loaded, return to the caller. */
   if (temp == VM_LOADNODE_REMAPPED ||
      (temp == VM_LOADNODE_CHANGED && page_min == page_max))
       goto end;
  }
  ATREE_WALKMIN(VIRT vm_vpage_t,page_semi,page_level);
  node = min_branch;
  goto again;
 } else if (walk_max) {
  /* Recursively continue searching right. */
  ATREE_WALKMAX(VIRT vm_vpage_t,page_semi,page_level);
  node = max_branch;
  goto again;
 }
end:
 return result;
}

PUBLIC bool KCALL
vm_loadcore(vm_vpage_t page, size_t num_pages,
            unsigned int mode) {
 bool result = false; int temp;
 struct vm *EXCEPT_VAR effective_vm;
 if (page < X86_KERNEL_BASE_PAGE) {
  effective_vm = THIS_VM;
  if (page+num_pages < page ||
      page+num_pages > X86_KERNEL_BASE_PAGE)
      num_pages = X86_KERNEL_BASE_PAGE-page;
 } else {
  effective_vm = &vm_kernel;
 }
 /* Quick check: If nothing needed to be done... */
 if (!num_pages)
      return false;
 vm_acquire(effective_vm);
 TRY {
loadcore_again:
  if (!effective_vm->vm_map)
       goto done; /* Nothing to load... */
  temp = impl_vm_loadcore(effective_vm->vm_map,effective_vm,
                          page,
                          page+num_pages-1,
                          VM_SEMI0,
                          VM_LEVEL0,
                          mode);
  if (temp != VM_LOADNODE_UNCHANGED) {
   result = true;
   /* If something was re-mapped and more than a single page
    * is being loaded, that means that some pages the caller
    * requested us to load may still not have been loaded.
    * For that reason, jump back and continue loading more stuff.
    */
   if (temp == VM_LOADNODE_REMAPPED && num_pages > 1)
       goto loadcore_again;
  }
done:;
 } FINALLY {
  vm_release(effective_vm);
 }
 return result;
}


PRIVATE size_t KCALL
vm_lock_impl(struct vm_node *__restrict node,
             struct vm *__restrict effective_vm,
             VIRT vm_vpage_t page_min,
             VIRT vm_vpage_t page_max,
             ATREE_SEMI_T(VIRT vm_vpage_t) page_semi,
             ATREE_LEVEL_T page_level, unsigned int mode) {
 size_t result = 0;
 bool walk_min,walk_max;
 struct vm_node *min_branch;
 struct vm_node *max_branch;
again:
 assert(page_min <= page_max);
 min_branch = node->vn_node.a_min;
 max_branch = node->vn_node.a_max;
 walk_min = page_min <  page_semi && min_branch != NULL;
 walk_max = page_max >= page_semi && max_branch != NULL;

 if (page_min <= node->vn_node.a_vmax &&
     page_max >= node->vn_node.a_vmin) {
  vm_raddr_t region_begin,region_end;
  size_t region_size;
  struct vm_region *EXCEPT_VAR region;
  /* Found a matching entry!
   * >> Figure out how much of this must be loaded into the core. */
  assert(page_min >= node->vn_node.a_vmin);
  assert(page_max <= node->vn_node.a_vmax);
  region_begin = node->vn_start+(page_min-node->vn_node.a_vmin);
  region_end   = node->vn_start+((page_max+1)-node->vn_node.a_vmin);
  region_size  = region_end-region_begin;
  region       = node->vn_region;
  mutex_get(&region->vr_lock);
  TRY {
   /* Create/Delete locks for the region in question. */
   if (mode & VM_LOCK_LOCK)
        vm_region_inclck_range(region,region_begin,region_size);
   else vm_region_declck_range(region,region_begin,region_size);
  } FINALLY {
   mutex_put(&region->vr_lock);
  }
  result += region_size;
 }

 if (walk_min) {
  /* Recursively continue searching left. */
  if (walk_max) {
   result += vm_lock_impl(max_branch,effective_vm,page_min,page_max,
                          ATREE_NEXTMAX(VIRT vm_vpage_t,page_semi,page_level),
                          ATREE_NEXTLEVEL(page_level),mode);
  }
  ATREE_WALKMIN(VIRT vm_vpage_t,page_semi,page_level);
  node = min_branch;
  goto again;
 } else if (walk_max) {
  /* Recursively continue searching right. */
  ATREE_WALKMAX(VIRT vm_vpage_t,page_semi,page_level);
  node = max_branch;
  goto again;
 }
 return result;
}


PUBLIC size_t KCALL
vm_lock(vm_vpage_t page, size_t num_pages, unsigned int mode) {
 struct vm *EXCEPT_VAR effective_vm;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 if unlikely(!num_pages) return 0;
 assert(page+num_pages > page);
 assert(page+num_pages-1 <= VM_VPAGE_MAX);
 effective_vm = page >= X86_KERNEL_BASE_PAGE ? &vm_kernel : THIS_VM;
 assertf(effective_vm == ((page+num_pages-1) >= X86_KERNEL_BASE_PAGE ? &vm_kernel : THIS_VM),
         "Effective VM of virtual address range %p...%p is ambiguous",
         VM_PAGE2ADDR(page),VM_PAGE2ADDR(page+num_pages)-1);
 vm_acquire(effective_vm);
 TRY {
  /* Recursively scan branches within the given range. */
  result = vm_lock_impl(effective_vm->vm_map,effective_vm,
                        page,
                        page+num_pages-1,
                        VM_SEMI0,
                        VM_LEVEL0,
                        mode);
  /* If requested, immediately load the pages that are now locked. */
  if (result && (mode&VM_LOCK_INCORE))
      impl_vm_loadcore(effective_vm->vm_map,effective_vm,
                       page,
                       page+num_pages-1,
                       VM_SEMI0,
                       VM_LEVEL0,
                       mode);
 } FINALLY {
  vm_release(effective_vm);
 }
 return result;
}




PUBLIC void KCALL
vm_split_before(struct vm *__restrict effective_vm,
                vm_vpage_t page_address) {
 struct vm_node **pnode,*node;
 struct vm_node *EXCEPT_VAR new_node;
 vm_vpage_t split_semi; unsigned int split_level;
 vm_raddr_t split_offset;
 assert(vm_holding(effective_vm));
 /* Search for the node that we're supposed to split. */
 split_semi  = VM_SEMI0;
 split_level = VM_LEVEL0;
 pnode = vm_node_tree_plocate_at(&effective_vm->vm_map,page_address,
                                 &split_semi,&split_level);
 /* If there is none, there's nothing for us to split.
  * If the given `page_address' is at the start of the node,
  * the the node has essentially already been split. */
 if (!pnode || (node = *pnode,page_address == VM_NODE_BEGIN(node))) return;
 /* If the node has the IMMUTABLE flag set, then we're not allowed to split it. */
 if unlikely(node->vn_flag & VM_NODE_FIMMUTABLE) return;
 assert(page_address < VM_NODE_END(node));
 assert(page_address != 0);
 /* Construct the secondary node that will describe the upper half after the split. */
 new_node = (struct vm_node *)kmalloc(sizeof(struct vm_node),
                                      GFP_SHARED|GFP_LOCKED);
 if (effective_vm == &vm_kernel) {
  /* The malloc() just now may have changed the layout of the VM in kernel-space.
   * Therefor, we must reload the indirection node pointer. */
  split_semi = VM_SEMI0;
  split_level = VM_LEVEL0;
  pnode = vm_node_tree_plocate_at(&effective_vm->vm_map,page_address,
                                  &split_semi,&split_level);
  assert(pnode);
  assert(node == *pnode);
 }

 split_offset = page_address - VM_NODE_BEGIN(node);
 /* Fill in the new node. */
 new_node->vn_node.a_vmin = page_address;
 new_node->vn_node.a_vmax = VM_NODE_MAX(node);
 new_node->vn_start       = node->vn_start + split_offset;
 new_node->vn_region      = node->vn_region;
 new_node->vn_prot        = node->vn_prot;
 new_node->vn_flag        = VM_NODE_FNORMAL;
 new_node->vn_closure     = node->vn_closure;
 new_node->vn_notify      = node->vn_notify;
 if (new_node->vn_notify) {
  /* TODO: VM_NOTIFY_SPLIT */
  TRY {
   /* Invoke the incref() notification. */
   new_node->vn_closure = INVOKE_NOTIFY(new_node->vn_notify,
                                        new_node->vn_closure,
                                        VM_NOTIFY_INCREF,
                                        VM_NODE_BEGIN(new_node),
                                        VM_NODE_SIZE(new_node),
                                        NULL);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   kfree(new_node);
   error_rethrow();
  }
 }

 /* Pop the old node so we can modify it. */
#if 0
 if (VM_NODE_MIN(node) >= X86_KERNEL_BASE_PAGE)
     debug_printf("POP_NODE %p at %p...%p\n",
                  node,
                  VM_NODE_MINADDR(node),
                  VM_NODE_MAXADDR(node));
#endif
 vm_node_tree_pop_at(pnode,split_semi,split_level);
 LIST_REMOVE(node,vn_byaddr);

 /* Update the effect pointers of the old node. */
 node->vn_node.a_vmax = page_address-1;

 /* Incref the pointed-to region itself.
  * NOTE: However, region part references are simply
  *       inherited by the upper half of the (now split)
  *       node. */
 vm_region_incref(new_node->vn_region);

 /* Re-insert the old, and insert the new nodes.
  * There is no need for a remap() here, because the described data didn't change. */
 vm_insert_node(effective_vm,node);
 vm_insert_node(effective_vm,new_node);
}


INTDEF ATTR_RETNONNULL struct vm_part *KCALL
vm_region_split_before(struct vm_region *__restrict self,
                       vm_raddr_t part_address);



/* XXX: This might not fully work, yet (needs some debugging...) */
//#define CONFIG_NO_VM_MERGING

#undef CONFIG_LOG_VM_MERGING
#if !defined(NDEBUG) && 0
#define CONFIG_LOG_VM_MERGING 1
#endif

/* Allow merging of adjacent VM nodes that point to the same underlying region.
 * This configuration option has an extension `CONFIG_VM_MERGING_COMPLEX', which
 * when enabled also allows merging to different regions that are syntactically
 * equivalent. */
#undef CONFIG_VM_MERGING
#ifndef CONFIG_NO_VM_MERGING
#define CONFIG_VM_MERGING 1
#endif

/* Allow merging of complex VM regions and nodes, that is the merging
 * of VM nodes that point to different, but syntactically equivalent regions.
 * Not only is this core quite complex (hence the config option's name),
 * and neither can we just ~always~ merge regions if they would match
 * (doing so would introduce an infinite loop). */
#undef CONFIG_VM_MERGING_COMPLEX
#ifndef CONFIG_NO_VM_MERGING_COMPLEX
#define CONFIG_VM_MERGING_COMPLEX 1
#endif

#ifndef CONFIG_VM_MERGING
#undef CONFIG_VM_MERGING_COMPLEX
#endif

INTDEF ATTR_NOTHROW ATTR_RETNONNULL struct vm_part *KCALL
vm_region_mergenext(struct vm_region *__restrict region,
                    struct vm_part *__restrict part);

#ifdef CONFIG_VM_MERGING
PRIVATE bool vm_disable_merging = false;
DEFINE_DRIVER_BOOL(vm_disable_merging,"nomerge");
#endif


PUBLIC ATTR_NOTHROW void KCALL
vm_merge_before(struct vm *__restrict effective_vm,
                vm_vpage_t page_address) {
 struct vm *EXCEPT_VAR xeffective_vm = effective_vm;
#ifndef CONFIG_VM_MERGING
 (void)xeffective_vm;
 (void)effective_vm;
 (void)page_address;
#else /* !CONFIG_VM_MERGING */
 struct vm_node **pnext,*next,*self;
 vm_vpage_t next_addr_semi = VM_SEMI0;
 unsigned int next_addr_level = VM_LEVEL0;
#ifdef CONFIG_VM_MERGING_COMPLEX
 /* Per-vm indicator to prevent infinite recursion caused by
  * the additional allocations when merging complex VM regions. */
 PRIVATE ATTR_PERVM bool vm_is_merging = false;
#endif
 /* Check if VM merging has been disabled via the kernel commandline. */
 if (vm_disable_merging)
     return;

 assert(vm_holding(effective_vm));
 pnext = vm_node_tree_plocate_at(&effective_vm->vm_map,
                                  page_address,
                                 &next_addr_semi,
                                 &next_addr_level);
 if (!pnext)
     return; /* Nothing to merge... */
 next = *pnext;
 assert(next);
 if (VM_NODE_BEGIN(next) != page_address)
     return; /* No mapping boundary here. */
 if (next == effective_vm->vm_byaddr)
     return; /* No predecessor to merge with. */
 self = COMPILER_CONTAINER_OF(next->vn_byaddr.le_pself,
                              struct vm_node,vn_byaddr.le_next);
 if (VM_NODE_END(self) != page_address)
     return; /* Previous mapping doesn't end where the next begins. */

 /* Actually check if the 2 mappings can be merged. */
 if (self->vn_prot != next->vn_prot) return;
 if (self->vn_notify != next->vn_notify) return;
 if (self->vn_closure != next->vn_closure) return;

 if (self->vn_region == next->vn_region) {
  assertf(self->vn_region->vr_refcnt >= 2,
          "But we have 2 nodes mapping this region...");
  /* Same region, no problem! */
  if ((self->vn_start + VM_NODE_SIZE(self)) != next->vn_start)
       return; /* Region mappings aren't adjacent. */
  if (self->vn_region->vr_flags & VM_REGION_FDONTMERGE)
      return; /* Mappings of this region aren't supposed to be merged. */

#ifdef CONFIG_LOG_VM_MERGING
  debug_printf("[VM] Merge adjacent nodes at %p...%p and %p...%p\n",
               VM_NODE_MINADDR(self),VM_NODE_MAXADDR(self),
               VM_NODE_MINADDR(next),VM_NODE_MAXADDR(next));
#endif

  /* Remove `self' and `next', then re-insert `self'. */
  asserte(vm_node_tree_pop_at(pnext,next_addr_level,next_addr_level) == next);
  asserte(vm_node_tree_remove(&effective_vm->vm_map,page_address-1) == self);

  /* The mappings are perfectly aligned.
   * >> We can simply get rid of `next' and
   *    have `self' implicitly inherit its data. */
  ATOMIC_DECFETCH(next->vn_region->vr_refcnt);

  /* NOTE: Region part references are implicitly transferred because
   *       the effective mappings are perfectly adjacent. */

  /* Extend the effective range of the first node to encompass the second. */
  self->vn_node.a_vmax = next->vn_node.a_vmax;

 } else {
#ifndef CONFIG_VM_MERGING_COMPLEX
  return;
#else /* !CONFIG_VM_MERGING_COMPLEX */
  REF struct vm_region *EXCEPT_VAR new_region;
  struct vm_region *self_region;
  struct vm_region *next_region;
  size_t self_size,next_size;
  self_region = self->vn_region;
  next_region = next->vn_region;
  if ((self_region->vr_flags & VM_REGION_FDONTMERGE) ||
      (next_region->vr_flags & VM_REGION_FDONTMERGE))
       return; /* Mappings of these regions aren't supposed to be merged. */
#define VM_REGION_FCOMPAREMASK  (VM_REGION_FMONITOR)

  if ((self_region->vr_flags & VM_REGION_FCOMPAREMASK) !=
      (next_region->vr_flags & VM_REGION_FCOMPAREMASK))
       return; /* Incompatible region flags. */

  if (ATOMIC_READ(self_region->vr_futex.ap_data) != 0 ||
      ATOMIC_READ(next_region->vr_futex.ap_data) != 0)
      return; /* Cannot merge when there are active futex objects.
               * (might be possible in the future, but I'm really unsure
               *  if modifying `f_addr' would introduce some race condition) */
  if (self_region->vr_type != next_region->vr_type)
      return; /* Differently typed regions. */
  if (FORVM(effective_vm,vm_is_merging))
      return; /* The VM is already performing another merge-operation. */
  self_size = VM_NODE_SIZE(self);
  next_size = VM_NODE_SIZE(next);
  assert(self_size != 0);
  assert(next_size != 0);

  /* Enable merging mode int the effective VM. */
  FORVM(effective_vm,vm_is_merging) = true;

  TRY {

#ifdef CONFIG_LOG_VM_MERGING
   debug_printf("[VM] Comparing virtual regions %p,%p at %p...%p and %p...%p\n",
                self_region,next_region,
                VM_NODE_MINADDR(self),VM_NODE_MAXADDR(self),
                VM_NODE_MINADDR(next),VM_NODE_MAXADDR(next));
#endif
   switch (self_region->vr_type) {

   case VM_REGION_MEM:
    /* Can only merge dynamic memory regions that
     * aren't being shared with other processes. */
    if (self_region->vr_refcnt > 1) goto dont_merge;
    if (next_region->vr_refcnt > 1) goto dont_merge;
    if (self_region->vr_init != next_region->vr_init)
        goto dont_merge; /* The regions use differing types of initialization. */

    /* Check if initialization functions are compatible. */
    switch (self_region->vr_init) {

    case VM_REGION_INIT_FNORMAL:
     break;

    case VM_REGION_INIT_FFILLER:
     if (self_region->vr_setup.s_filler !=
         next_region->vr_setup.s_filler)
         goto dont_merge;
     break;

    case VM_REGION_INIT_FRANDOM:
     break;

    {
     bool self_is_empty;      /* All bytes mapped by `self' are filled with the filler. */
     bool next_is_empty;      /* All bytes mapped by `next' are filled with the filler. */
     bool self_is_full;       /* No byte mapped by `self' in `self_region' doesn't originate from the file */
     bool next_is_full;       /* No byte mapped by `next' in `next_region' doesn't originate from the file */
     bool self_padding_back;  /* `self' maps data near the end that is filled with the filler */
     bool next_padding_front; /* `next' maps data near the end that is filled with the filler */
    case VM_REGION_INIT_FFILE:
    case VM_REGION_INIT_FFILE_RO:
     /* Compare effective initialization of mapped region area. */
     self_is_empty = ((/* File mapping ends before the portion mapped by the node */
                       self_region->vr_setup.s_file.f_begin+
                       self_region->vr_setup.s_file.f_size) <=
                       self->vn_start * PAGESIZE) ||
                      (/* File mapping starts after the portion mapped by the node */
                       self_region->vr_setup.s_file.f_begin >=
                      (self->vn_start+self_size) * PAGESIZE);
     next_is_empty = ((/* File mapping ends before the portion mapped by the node */
                       next_region->vr_setup.s_file.f_begin+
                       next_region->vr_setup.s_file.f_size) <=
                       next->vn_start * PAGESIZE) ||
                      (/* File mapping starts after the portion mapped by the node */
                       next_region->vr_setup.s_file.f_begin >=
                      (next->vn_start+next_size) * PAGESIZE);
     self_padding_back = !(self_region->vr_setup.s_file.f_begin+
                           self_region->vr_setup.s_file.f_size <=
                          (self->vn_start+self_size) * PAGESIZE);
     next_padding_front = !(next_region->vr_setup.s_file.f_begin >=
                            next->vn_start * PAGESIZE);
     self_is_full  = (/* No padding exists at the front */
                      (self_region->vr_setup.s_file.f_begin >=
                       self->vn_start * PAGESIZE) &&
                      /* No padding exists at the back */
                      !self_padding_back);
     next_is_full  = (/* No padding exists at the front */
                      !next_padding_front &&
                      /* No padding exists at the back */
                      (next_region->vr_setup.s_file.f_begin+
                       next_region->vr_setup.s_file.f_size <=
                      (next->vn_start+next_size) * PAGESIZE));
     assert(!(self_is_full && self_is_empty));
     assert(!(next_is_full && next_is_empty));
     assert(self_is_full ? !self_padding_back : 1);
     assert(next_is_full ? !next_padding_front : 1);
     if (self_is_empty && next_is_empty) {
      /* Special case: The merged region isn't actually a file mapping, but only
       *               consists of filler data. -> Create the mapping as a FILLER,
       *               but check that both file mappings use the same filler. */
      if (self_region->vr_setup.s_file.f_filler !=
          next_region->vr_setup.s_file.f_filler)
          goto dont_merge;
      new_region                    = vm_region_alloc(self_size+next_size);
      new_region->vr_init           = VM_REGION_INIT_FFILLER;
      new_region->vr_flags          = self_region->vr_flags & VM_REGION_FCOMPAREMASK;
      new_region->vr_setup.s_filler = self_region->vr_setup.s_file.f_filler;
      goto merge_region_parts;
     }
     if (self_is_full && next_is_full) {
      /* Special case: Both regions contain no unmapped portions. */
      if (self_region->vr_setup.s_file.f_node !=
          next_region->vr_setup.s_file.f_node)
          goto dont_merge;
      if (self_region->vr_setup.s_file.f_start -
         (self_region->vr_setup.s_file.f_begin + self->vn_start * PAGESIZE) +
         (self_size * PAGESIZE) !=
         (next_region->vr_setup.s_file.f_start -
         (next_region->vr_setup.s_file.f_begin + next->vn_start * PAGESIZE)))
          goto dont_merge; /* Different file portions are being mapped. */
      new_region                           = vm_region_alloc(self_size+next_size);
      new_region->vr_init                  = self_region->vr_init;
      new_region->vr_flags                 = self_region->vr_flags & VM_REGION_FCOMPAREMASK;
      new_region->vr_setup.s_file.f_node   = self_region->vr_setup.s_file.f_node;
      new_region->vr_setup.s_file.f_begin  = 0;
      new_region->vr_setup.s_file.f_size   = self_size+next_size;
      new_region->vr_setup.s_file.f_start  = self_region->vr_setup.s_file.f_start;
      new_region->vr_setup.s_file.f_start += self_region->vr_setup.s_file.f_begin;
      new_region->vr_setup.s_file.f_filler = 0;
      inode_incref(new_region->vr_setup.s_file.f_node);
      goto merge_region_parts;
     }
     if (self_is_empty) {
      uintptr_t next_start_offset;
      /* Special case: Nothing is mapped from the left file. */
      if (!next_is_full &&
          (self_region->vr_setup.s_file.f_filler !=
           next_region->vr_setup.s_file.f_filler))
           goto dont_merge; /* Different fillers */
      new_region                           = vm_region_alloc(self_size+next_size);
      new_region->vr_init                  = self_region->vr_init;
      new_region->vr_flags                 = self_region->vr_flags & VM_REGION_FCOMPAREMASK;
      new_region->vr_setup.s_file.f_node   = next_region->vr_setup.s_file.f_node;
      new_region->vr_setup.s_file.f_begin  = next_region->vr_setup.s_file.f_begin;
      new_region->vr_setup.s_file.f_begin += self_size * PAGESIZE;
      new_region->vr_setup.s_file.f_size   = MIN(next_region->vr_setup.s_file.f_size,
                                               ((next->vn_start + next_size) * PAGESIZE)-
                                                 next_region->vr_setup.s_file.f_begin);
      new_region->vr_setup.s_file.f_start  = next_region->vr_setup.s_file.f_start;
      new_region->vr_setup.s_file.f_start += self_size * PAGESIZE;
      new_region->vr_setup.s_file.f_filler = self_region->vr_setup.s_file.f_filler;
      next_start_offset = next->vn_start * PAGESIZE;
      if (new_region->vr_setup.s_file.f_begin >= next_start_offset)
          new_region->vr_setup.s_file.f_begin -= next_start_offset;
      else {
       uintptr_t diff;
       /* Deal with address truncation. */
       diff = next_start_offset-new_region->vr_setup.s_file.f_begin;
       assertf(new_region->vr_setup.s_file.f_size > diff,
               "If this wasn't the case, then `next_is_empty' should have been true, "
               "and since this branch is dealing with `self_is_empty', and the branch "
               "for `self_is_empty' and `next_is_empty' has already been passed, the "
               "effective mapping of the next region can't be empty!");
       new_region->vr_setup.s_file.f_begin  = 0;
       new_region->vr_setup.s_file.f_start += diff;
       new_region->vr_setup.s_file.f_size  -= diff;
      }
      inode_incref(new_region->vr_setup.s_file.f_node);
      goto merge_region_parts;
     }
     if (next_is_empty) {
      uintptr_t self_start_offset;
      /* Special case: Nothing is mapped from the right file. */
      if (!self_is_full &&
          (self_region->vr_setup.s_file.f_filler !=
           next_region->vr_setup.s_file.f_filler))
           goto dont_merge; /* Different fillers */
      new_region                           = vm_region_alloc(self_size+next_size);
      new_region->vr_init                  = self_region->vr_init;
      new_region->vr_flags                 = self_region->vr_flags & VM_REGION_FCOMPAREMASK;
      new_region->vr_setup.s_file.f_node   = self_region->vr_setup.s_file.f_node;
      new_region->vr_setup.s_file.f_begin  = self_region->vr_setup.s_file.f_begin;
      new_region->vr_setup.s_file.f_size   = MIN(self_region->vr_setup.s_file.f_size,
                                               ((self->vn_start + self_size) * PAGESIZE)-
                                                 self_region->vr_setup.s_file.f_begin);
      new_region->vr_setup.s_file.f_start  = self_region->vr_setup.s_file.f_start;
      new_region->vr_setup.s_file.f_filler = next_region->vr_setup.s_file.f_filler;
      self_start_offset = self->vn_start * PAGESIZE;
      if (self_start_offset >= new_region->vr_setup.s_file.f_begin)
          new_region->vr_setup.s_file.f_begin -= self_start_offset;
      else {
       uintptr_t diff;
       /* Deal with address truncation. */
       diff = new_region->vr_setup.s_file.f_begin - self_start_offset;
       assertf(new_region->vr_setup.s_file.f_size > diff,
               "If this wasn't the case, then `self_is_empty' should "
               "have been true, a case that was already handled above!");
       new_region->vr_setup.s_file.f_size  -= diff;
       new_region->vr_setup.s_file.f_begin  = 0;
       new_region->vr_setup.s_file.f_start += diff;
      }
      inode_incref(new_region->vr_setup.s_file.f_node);
      goto merge_region_parts;
     }

     /* With the special cases of empty mappings out of the way,
      * if there still exists padding data between the 2 regions,
      * then we can't actually map any memory. */
     if (self_padding_back)
         goto dont_merge;
     if (next_padding_front)
         goto dont_merge;

     /* With neither nodes empty, do a general
      * check to see if both files map the same node. */
     if (self_region->vr_setup.s_file.f_node !=
         next_region->vr_setup.s_file.f_node)
         goto dont_merge; /* Different files */

     /* Now that we know that both nodes must map at least ~some~ file
      * data, check to ensure that they map file file offsets whose
      * relative file positions are equal to their relative memory
      * addresses. */
     if (self_region->vr_setup.s_file.f_start -
        (self_region->vr_setup.s_file.f_begin + self->vn_start * PAGESIZE) +
        (self_size * PAGESIZE) !=
        (next_region->vr_setup.s_file.f_start -
        (next_region->vr_setup.s_file.f_begin + next->vn_start * PAGESIZE)))
         goto dont_merge; /* Different file portions are being mapped. */
     new_region                           = vm_region_alloc(self_size+next_size);
     new_region->vr_init                  = self_region->vr_init;
     new_region->vr_flags                 = self_region->vr_flags & VM_REGION_FCOMPAREMASK;
     new_region->vr_setup.s_file.f_node   = self_region->vr_setup.s_file.f_node;
     new_region->vr_setup.s_file.f_start  = self_region->vr_setup.s_file.f_start;
     new_region->vr_setup.s_file.f_size   = MIN(self_region->vr_setup.s_file.f_size,
                                              ((self->vn_start + self_size) * PAGESIZE)-
                                                self_region->vr_setup.s_file.f_begin);
     new_region->vr_setup.s_file.f_size  += MIN(next_region->vr_setup.s_file.f_size,
                                              ((next->vn_start + next_size) * PAGESIZE)-
                                                next_region->vr_setup.s_file.f_begin);
     new_region->vr_setup.s_file.f_begin  = self_region->vr_setup.s_file.f_begin;
     new_region->vr_setup.s_file.f_filler = self_region->vr_setup.s_file.f_filler;
     goto merge_region_parts;
    }

    case VM_REGION_INIT_FUSER:
     /* Compare the effective user-callback, as well as the associated delta. */
     if (self_region->vr_setup.s_user.u_func !=
         next_region->vr_setup.s_user.u_func)
         goto dont_merge; /* Different user-functions. */
     if (self_region->vr_setup.s_user.u_closure !=
         next_region->vr_setup.s_user.u_closure)
         goto dont_merge; /* Different user-closures. */
     if ((self_region->vr_setup.s_user.u_delta - self->vn_start) + (vm_sraddr_t)self_size !=
         (next_region->vr_setup.s_user.u_delta - next->vn_start))
         goto dont_merge; /* Different delta offsets. */
     break;

    default:
     /* Unknown initialization function. */
     goto dont_merge;
    }

    /* Create the new, combined region. */
    new_region           = vm_region_alloc(self_size+next_size);
    new_region->vr_init  = self_region->vr_init;
    new_region->vr_flags = self_region->vr_flags & VM_REGION_FCOMPAREMASK;
    TRY {
     /* Copy region initializers. */
     switch (self_region->vr_init) {
     case VM_REGION_INIT_FNORMAL:
     case VM_REGION_INIT_FRANDOM:
      break;
     case VM_REGION_INIT_FFILLER:
      new_region->vr_setup.s_filler = self_region->vr_setup.s_filler;
      break;
     case VM_REGION_INIT_FUSER:
      new_region->vr_setup.s_user.u_delta = self_region->vr_setup.s_user.u_delta-self->vn_start;
      new_region->vr_setup.s_user.u_closure = (*self_region->vr_setup.s_user.u_func)(self_region->vr_setup.s_user.u_closure,
                                                                                     VM_REGION_USERCOMMAND_INCREF,
                                                                                     new_region->vr_setup.s_user.u_delta,
                                                                                     NULL,new_region->vr_size);
      COMPILER_WRITE_BARRIER();
      new_region->vr_setup.s_user.u_func = self_region->vr_setup.s_user.u_func;
      COMPILER_WRITE_BARRIER();
      break;
     default: __builtin_unreachable();
     }
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     vm_region_decref(new_region);
     error_rethrow();
    }
    /* Finally, merge the parts of both input regions into the merged region. */
    goto merge_region_parts;

   case VM_REGION_PHYSICAL:
   {
    struct vm_part *COMPILER_IGNORE_UNINITIALIZED(self_parts);
    struct vm_part *COMPILER_IGNORE_UNINITIALIZED(next_parts);
    struct vm_part *last_part,*iter;
    vm_raddr_t self_end,next_end;
    /* Can only merge physical memory regions that
     * aren't being shared with other processes. */
    if (self_region->vr_refcnt > 1) goto dont_merge;
    if (next_region->vr_refcnt > 1) goto dont_merge;
    /* physical memory mappings can easily be merged,
     * since we simply combine the scatter vectors
     * and associated parts. */
#ifdef CONFIG_LOG_VM_MERGING
    debug_printf("[VM] Comparing physical regions %p,%p at %p...%p and %p...%p\n",
                 self_region,next_region,
                 VM_NODE_MINADDR(self),VM_NODE_MAXADDR(self),
                 VM_NODE_MINADDR(next),VM_NODE_MAXADDR(next));
#endif
#if 0
    goto dont_merge;
#endif
    new_region = vm_region_alloc(self_size+next_size);
    new_region->vr_type  = VM_REGION_PHYSICAL;
    new_region->vr_flags = self_region->vr_flags & VM_REGION_FCOMPAREMASK;
merge_region_parts:
    TRY {
     /* Split parts of the two regions at the effective
      * locations at which we've going to merge them.
      * That way, we can inherit those parts by stealing and relocating them. */
     assert((self->vn_start + self_size) <= self_region->vr_size);
     assert((next->vn_start + next_size) <= next_region->vr_size);
     if ((self->vn_start + self_size) != self_region->vr_size)
          vm_region_split_before(self_region,self->vn_start + self_size);
     if ((next->vn_start + next_size) != next_region->vr_size)
          vm_region_split_before(next_region,next->vn_start);
     self_parts = vm_region_split_before(self_region,self->vn_start);
     next_parts = vm_region_split_before(next_region,next->vn_start);
     if (next_parts == &next_region->vr_part0) {
      struct vm_part *copy;
      /* We need the first part of the high region to be allocated
       * dynamically so we can re-use it as a link in the chain of
       * inherited parts. */
      copy = (struct vm_part *)kmalloc(sizeof(struct vm_part),
                                       GFP_SHARED|GFP_LOCKED);
      memcpy(copy,next_parts,sizeof(struct vm_part));
      COMPILER_WRITE_BARRIER();
      /* Relocate parts of this region. */
      copy->vp_chain.le_pself = &next_region->vr_parts;
      if (copy->vp_chain.le_next)
          copy->vp_chain.le_next->vp_chain.le_pself = &copy->vp_chain.le_next;
      next_parts = copy;
     }
     COMPILER_BARRIER();
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     vm_region_decref(new_region);
     error_rethrow();
    }

    /* Extract inherited parts from their chains (we've already created the
     * necessary splits above), but keep the regions in a consistent state. */
    assert(self_parts);
    assert(next_parts);
    assert(self_parts->vp_start == self->vn_start);
    self_end = self->vn_start + self_size;
    next_end = next->vn_start + next_size;
    for (last_part = self_parts;;
         last_part = last_part->vp_chain.le_next) {
     last_part->vp_start -= self->vn_start;
     /* Fix the part reference counter if the source region contained leaked parts. */
     assert(last_part->vp_refcnt == 1 ||
           (self_region->vr_flags&VM_REGION_FLEAKINGPARTS));
     last_part->vp_refcnt = 1;
     if (!last_part->vp_chain.le_next ||
          last_part->vp_chain.le_next->vp_start >= self_end)
          break;
    }
    assert(self_parts->vp_start == 0);
    /* `last_part' now points to the last part that we're inherited from `self'.
     * NOTE: In order to keep `self_region' consistent, we must adjust
     *       all parts following thereafter (that weren't inherited) to
     *       still form a consistent chain. */
    iter = last_part->vp_chain.le_next;

    /* Append to chain of high parts not inherited
     * to the chain of non-inherited low parts.
     * NOTE: `iter' is currently the first non-inherited high-part from `self' */
    *self_parts->vp_chain.le_pself = iter;
    if (iter) iter->vp_chain.le_pself = self_parts->vp_chain.le_pself;

    for (; iter; iter = iter->vp_chain.le_next)
        iter->vp_start -= self_size; /* Adjust by what we've stolen. */

    /* Find the last part that we're actually inheriting from the `next'. */
    for (iter = next_parts;;
         iter = iter->vp_chain.le_next) {
     /* Relocate part offsets to indicate
      * successors to the chain inherited from `self' */
     iter->vp_start -= next->vn_start;
     iter->vp_start += self_size;
     /* Fix the part reference counter if the source region contained leaked parts. */
     assert(iter->vp_refcnt == 1 ||
           (next_region->vr_flags&VM_REGION_FLEAKINGPARTS));
     iter->vp_refcnt = 1;
     if (!iter->vp_chain.le_next ||
          iter->vp_chain.le_next->vp_start >= next_end)
          break;
    }
    /* `iter' is now the last inherited part from `next_region' */

    /* Unlink the chain of inherited parts from `next_region'. */
    *next_parts->vp_chain.le_pself = iter->vp_chain.le_next;
    if (iter->vp_chain.le_next) {
     struct vm_part *dismissed_parts;
     dismissed_parts = iter->vp_chain.le_next;
     dismissed_parts->vp_chain.le_pself = next_parts->vp_chain.le_pself;
     /* Same as above: bring `next_region' into a consistent state. */
     for (; dismissed_parts; dismissed_parts = dismissed_parts->vp_chain.le_next)
         dismissed_parts->vp_start -= next_size; /* Adjust by what we've stolen. */
     /* Terminate the chain of inherited parts. */
     iter->vp_chain.le_next = NULL;
    }

    /* Link the last part inherited from `self' to the
     * first part that is inherited from `next'. */
    last_part->vp_chain.le_next   = next_parts;
    next_parts->vp_chain.le_pself = &last_part->vp_chain.le_next;
    assertf(last_part->vp_start < next_parts->vp_start,
            "last_part->vp_start  = %Iu\n"
            "next_parts->vp_start = %Iu\n"
            "next->vn_start       = %Iu\n"
            "self_size            = %Iu\n"
            ,
            last_part->vp_start,
            next_parts->vp_start,
            next->vn_start,self_size);

    /* Set the chain of parts in the new region. */
    self_parts->vp_chain.le_pself = &new_region->vr_parts;
    new_region->vr_parts          = self_parts;

    if (self_parts == &self_region->vr_part0) {
     /* Must relocate the PART0 part from `self_region' into `new_region' */
     memcpy(&new_region->vr_part0,&self_region->vr_part0,sizeof(struct vm_part));
     new_region->vr_part0.vp_chain.le_pself = &new_region->vr_parts;
     new_region->vr_parts = &new_region->vr_part0;
     if (new_region->vr_part0.vp_chain.le_next)
         new_region->vr_part0.vp_chain.le_next->vp_chain.le_pself = &new_region->vr_part0.vp_chain.le_next;
     if (last_part == self_parts)
         last_part = &new_region->vr_part0;
    }
    /* Try to merge the last part inherited from the first
     * region with the first part inherited from the second. */
    vm_region_mergenext(new_region,last_part);
    assert(new_region->vr_parts->vp_start == 0);
   } break;

   case VM_REGION_LOGUARD:
   case VM_REGION_HIGUARD:
    /* Merging guard mappings (especially when
     * they're adjacent) would break their purpose. */
    goto dont_merge;

   case VM_REGION_RESERVED:
    /* Reserved virtual address space
     * This type of mapping can always be merged as its purpose
     * is simply to mark some region of memory that cannot be used
     * for automatic memory mapping. */
    new_region = vm_region_alloc(self_size+next_size);
    new_region->vr_type            = VM_REGION_RESERVED;
    new_region->vr_part0.vp_refcnt = 1; /* The combined reference. */
    new_region->vr_part0.vp_state  = VM_PART_UNKNOWN;
    /* Drop references from old parts. */
    vm_region_decref_range(self_region,self->vn_start,self_size);
    vm_region_decref_range(next_region,next->vn_start,next_size);
    break;

   default: /* Unknown/future region type. */
dont_merge:
    /* Delete the is-merging flag. */
    assert(FORVM(effective_vm,vm_is_merging));
    FORVM(effective_vm,vm_is_merging) = false;
    return;
   }
  } FINALLY {
   /* Delete the is-merging flag. */
   assert(FORVM(xeffective_vm,vm_is_merging));
   FORVM(xeffective_vm,vm_is_merging) = false;
   if (FINALLY_WILL_RETHROW &&
       error_code() == E_BADALLOC)
       return; /* Ignore failure to allocate a combining region. */
  }
#ifdef CONFIG_LOG_VM_MERGING
  debug_printf("[VM] Merging regions %p,%p at %p...%p and %p...%p\n",
               self_region,next_region,
               VM_NODE_MINADDR(self),VM_NODE_MAXADDR(self),
               VM_NODE_MINADDR(next),VM_NODE_MAXADDR(next));
#endif

  /* Remove `self' and `next', then re-insert `self'. */
  asserte(vm_node_tree_pop_at(pnext,next_addr_semi,next_addr_level) == next);
  asserte(vm_node_tree_remove(&effective_vm->vm_map,page_address-1) == self);

  /* Drop reference from the region of `next' and `self' */
  vm_region_decref(self_region);
  vm_region_decref(next_region);
  /* NOTE: part references are implicitly inherited by the
   *       fact that we stole all of the affected parts. */


  /* Update the region pointers of `self' to use the new (merged) region. */
  self->vn_start  = 0;
  self->vn_region = new_region; /* Inherit reference. */

  /* Extend the effective range of the first node to encompass the second. */
  self->vn_node.a_vmax = next->vn_node.a_vmax;
#endif  /* CONFIG_VM_MERGING_COMPLEX */
 }

 /* Remove the next node from the by-address chain of VM nodes. */
 LIST_REMOVE(next,vn_byaddr);

 /* Re-insert `self' */
 vm_node_tree_insert(&effective_vm->vm_map,self);

 /* Free the old `next'-node (region references have
  * already been inherited, or have been discarded) */
 if (next->vn_notify) {
  INVOKE_NOTIFY_V(next->vn_notify,
                  next->vn_closure,
                  VM_NOTIFY_DECREF,
                  0,
                  0,
                  NULL);
 }
 vm_node_free(next);
#endif /* CONFIG_VM_MERGING */
}



typedef void (KCALL *vm_func_t)(struct vm *__restrict vm);
INTDEF vm_func_t pervm_init_start[];
INTDEF vm_func_t pervm_init_end[];
INTDEF vm_func_t pervm_fini_start[];
INTDEF vm_func_t pervm_fini_end[];
INTDEF vm_func_t pervm_clone_start[];
INTDEF vm_func_t pervm_clone_end[];


#define VM_HEAP  (GFP_SHARED|GFP_LOCKED)

/* Allocate a new VM descriptor.
 * PERMV variables will have been initialized from the template,
 * and all other variables are default-initialized as unbound.
 * The reference counter is set to ONE(1)
 * @throw E_BADALLOC: Failed to allocate sufficient memory. */
PUBLIC ATTR_RETNONNULL ATTR_MALLOC
REF struct vm *KCALL vm_alloc(void) {
 /* NOTE: We use raw heap allocation for construction new VMs,
  *       so we don't have to account of the quite expensive
  *       overhead caused by the huge alignment requirements
  *       of the page directory stored inline with the VM. */
 vm_func_t *iter;
 struct heapptr EXCEPT_VAR vm_ptr;
 REF struct vm *result;
 vm_ptr = heap_align(&kernel_heaps[VM_HEAP],VM_ALIGN,0,
                    (size_t)kernel_pervm_size + PAGEDIR_SIZE,
                     VM_HEAP|GFP_CALLOC);
 result = (REF struct vm *)vm_ptr.hp_ptr;
 /* Save the allocated size in the VM structure. */
 assert(vm_ptr.hp_siz >= (size_t)kernel_pervm_size + PAGEDIR_SIZE);
 TRY {
  /* Initialize some basic fields.
   * NOTE: The VM initialization template excludes the page directory itself! */
  memcpy((&result->vm_pagedir)+1,
         (void *)kernel_pervm_start,
         (size_t)kernel_pervm_size);
  result->vm_refcnt = 1;
  result->vm_size   = vm_ptr.hp_siz;
  /* Determine the physical address of the page directory. */
  result->vm_physdir = pagedir_translate((vm_virt_t)&result->vm_pagedir);
#ifdef CONFIG_VM_USE_RWLOCK
  rwlock_cinit(&result->vm_lock);
#else
  mutex_cinit(&result->vm_lock);
#endif
  atomic_rwlock_cinit(&result->vm_tasklock);
  /* Initialize the associated page directory. */
  pagedir_init(&result->vm_pagedir,result->vm_physdir);

  /* Execute initializers. */
  TRY {
   for (iter = pervm_init_start;
        iter < pervm_init_end; ++iter)
        SAFECALL_KCALL_VOID_1(**iter,result);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   iter = pervm_fini_end;
   while (iter-- != pervm_fini_start)
        SAFECALL_KCALL_VOID_1(**iter,result);
  }

 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  heap_free(&kernel_heaps[VM_HEAP],
             vm_ptr.hp_ptr,
             vm_ptr.hp_siz,VM_HEAP);
  error_rethrow();
 }
 return result;
}



DEFINE_PERTASK_STARTUP(task_vm_startup);
INTERN void KCALL task_vm_startup(u32 flags) {
 struct vm *EXCEPT_VAR my_vm;
 if (flags & CLONE_VM) return;
 /* Map all nodes that were copied from the old VM in the new (current) one.
  * NOTE: No need to sync as we're the only thread using them right now. */
 my_vm = THIS_VM;
 vm_acquire(my_vm);
 TRY {
  struct vm_node *iter;
  iter = my_vm->vm_byaddr;
  for (; iter; iter = iter->vn_byaddr.le_next)
      vm_map_node(iter);
 } FINALLY {
  vm_release(my_vm);
 }
}


PUBLIC REF struct vm *KCALL vm_clone(void) {
 REF struct vm *EXCEPT_VAR result = vm_alloc();
 struct vm *EXCEPT_VAR myvm = THIS_VM;
 vm_vpage_t sync_min = (vm_vpage_t)-1;
 vm_vpage_t sync_max = 0;
 TRY {
  vm_acquire(myvm);
  TRY {
   vm_acquire(result);
   TRY {
    struct vm_node *EXCEPT_VAR iter;
    struct vm_node *EXCEPT_VAR copy;
    iter = myvm->vm_byaddr;
    for (; iter; iter = iter->vn_byaddr.le_next) {
     void *new_closure;
     if (iter->vn_prot & PROT_LOOSE)
         continue; /* Loose this mapping during fork(). */
     new_closure = iter->vn_closure;
     if (iter->vn_notify) {
      new_closure = INVOKE_NOTIFY(iter->vn_notify,
                                  new_closure,
                                  VM_NOTIFY_CLONE,
                                  VM_NODE_BEGIN(iter),
                                  VM_NODE_SIZE(iter),
                                  result);
      if (new_closure == VM_NOTIFY_CLONE_FLOOSE)
          continue; /* Loose this mapping. */
      /* Use INCREF() to generate the new closure. */
      if (new_closure == VM_NOTIFY_CLONE_FINCREF) {
       new_closure = INVOKE_NOTIFY(iter->vn_notify,
                                   iter->vn_closure,
                                   VM_NOTIFY_INCREF,
                                   VM_NODE_BEGIN(iter),
                                   VM_NODE_SIZE(iter),
                                   NULL);
      }
     }
     TRY {
      /* Copy the VM node. */
      copy = (struct vm_node *)kmalloc(sizeof(struct vm_node),
                                       GFP_SHARED|GFP_LOCKED);
      memcpy(copy,iter,sizeof(struct vm_node));
      TRY {
       vm_region_incref_range(copy->vn_region,
                              copy->vn_start,
                              VM_NODE_SIZE(copy));
       copy->vn_closure = new_closure;
       vm_region_incref(copy->vn_region);
      } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
       kfree(copy);
       error_rethrow();
      }
      /* Add the duplicated node to the new VM. */
      vm_insert_node(result,copy);
     } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
      if (iter->vn_notify) {
       INVOKE_NOTIFY_V(iter->vn_notify,
                       new_closure,
                       VM_NOTIFY_DECREF,
                       0,
                       0,
                       NULL);
      }
      error_rethrow();
     }

     /* Remap the node in the calling VM if need be. */
     if ((iter->vn_prot & PROT_SHARED) &&
        !(iter->vn_region->vr_flags & VM_REGION_FCANTSHARE));
     else if (iter->vn_region->vr_type == VM_REGION_RESERVED);
     else {
      /* Keep track of the upper bounds of the region
       * that will have to be synced in the old VM. */
      if (sync_min > VM_NODE_MIN(iter))
          sync_min = VM_NODE_MIN(iter);
      if (sync_max < VM_NODE_MAX(iter))
          sync_max = VM_NODE_MAX(iter);
      vm_map_node(iter);
     }
    }
    {
     /* With all nodes duplicated, mirror this in some other
      * fields, such as the vm-global application vector. */
     vm_func_t *iter = pervm_clone_start;
     for (; iter < pervm_clone_end; ++iter)
         SAFECALL_KCALL_VOID_1(*iter,result);
    }
   } FINALLY {
    vm_release(result);
   }
   /* Synchronize the current VM for the address range that got remapped. */
   if (sync_max > sync_min)
       vm_sync(sync_min,(sync_max-sync_min)+1);
  } FINALLY {
   vm_release(myvm);
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  vm_destroy(result);
  error_rethrow();
 }
 return result;
}



PRIVATE void KCALL
vm_update_ustack(struct vm_node *__restrict node,
                 VIRT vm_vpage_t page_min,
                 VIRT vm_vpage_t page_max,
                 ATREE_SEMI_T(VIRT vm_vpage_t) page_semi,
                 ATREE_LEVEL_T page_level,
                 struct userstack *__restrict old_stack,
                 struct userstack *__restrict new_stack) {
 bool walk_min,walk_max;
again:
 assert(node != node->vn_node.a_min);
 assert(node != node->vn_node.a_max);
 if (page_min <= node->vn_node.a_vmax &&
     page_max >= node->vn_node.a_vmin &&
     node->vn_notify == &application_notify &&
     node->vn_closure == old_stack) {
  userstack_incref(new_stack);
  userstack_decref(old_stack);
  node->vn_closure = new_stack;
 }
 walk_min = page_min <  page_semi && node->vn_node.a_min;
 walk_max = page_max >= page_semi && node->vn_node.a_max;
 if (walk_min) {
  /* Recursively continue searching left. */
  if (walk_max) {
   vm_update_ustack(node->vn_node.a_max,page_min,page_max,
                    ATREE_NEXTMAX(VIRT vm_vpage_t,page_semi,page_level),
                    ATREE_NEXTLEVEL(page_level),old_stack,new_stack);
  }
  ATREE_WALKMIN(VIRT vm_vpage_t,page_semi,page_level);
  node = node->vn_node.a_min;
  goto again;
 } else if (walk_max) {
  /* Recursively continue searching right. */
  ATREE_WALKMAX(VIRT vm_vpage_t,page_semi,page_level);
  node = node->vn_node.a_max;
  goto again;
 }
}

INTDEF void *KCALL
userseg_notify(void *closure, unsigned int code,
               vm_vpage_t addr, size_t num_pages,
               void *arg);

PRIVATE void KCALL
vm_update_useg(struct vm_node *__restrict node,
               VIRT vm_vpage_t page_min,
               VIRT vm_vpage_t page_max,
               ATREE_SEMI_T(VIRT vm_vpage_t) page_semi,
               ATREE_LEVEL_T page_level,
               struct task *__restrict old_task,
               struct task *__restrict new_task) {
 bool walk_min,walk_max;
again:
 assert(node != node->vn_node.a_min);
 assert(node != node->vn_node.a_max);
 if (page_min <= node->vn_node.a_vmax &&
     page_max >= node->vn_node.a_vmin &&
     node->vn_notify == &userseg_notify &&
     node->vn_closure == old_task)
     node->vn_closure = new_task;
 walk_min = page_min <  page_semi && node->vn_node.a_min;
 walk_max = page_max >= page_semi && node->vn_node.a_max;
 if (walk_min) {
  /* Recursively continue searching left. */
  if (walk_max) {
   vm_update_useg(node->vn_node.a_max,page_min,page_max,
                  ATREE_NEXTMAX(VIRT vm_vpage_t,page_semi,page_level),
                  ATREE_NEXTLEVEL(page_level),old_task,new_task);
  }
  ATREE_WALKMIN(VIRT vm_vpage_t,page_semi,page_level);
  node = node->vn_node.a_min;
  goto again;
 } else if (walk_max) {
  /* Recursively continue searching right. */
  ATREE_WALKMAX(VIRT vm_vpage_t,page_semi,page_level);
  node = node->vn_node.a_max;
  goto again;
 }
}

PRIVATE void KCALL
vm_delete_useg(struct vm_node **__restrict pnode,
               VIRT vm_vpage_t page_min,
               VIRT vm_vpage_t page_max,
               ATREE_SEMI_T(VIRT vm_vpage_t) page_semi,
               ATREE_LEVEL_T page_level,
               struct task *__restrict old_task) {
 bool walk_min,walk_max;
 struct vm_node *node;
again:
 node = *pnode;
 if (!node) return;
 if (page_min <= node->vn_node.a_vmax &&
     page_max >= node->vn_node.a_vmin &&
     node->vn_notify == &userseg_notify &&
     node->vn_closure == old_task) {
  /* Check the closure tag if requested, to. */
  asserte(vm_node_tree_pop_at(pnode,page_semi,page_level) == node);
  LIST_REMOVE(node,vn_byaddr);
#if 0 /* These are no-ops. */
  userseg_notify(old_task,VM_NOTIFY_UNMAP,
                 VM_NODE_BEGIN(node),
                 VM_NODE_SIZE(node),
                 NULL);
  userseg_notify(old_task,
                 VM_NOTIFY_DECREF,
                 0,
                 0,
                 NULL);
#endif
  vm_region_decref_range(node->vn_region,
                         node->vn_start,
                         VM_NODE_SIZE(node));
  vm_region_decref(node->vn_region);
  vm_node_free(node);
  goto again;
 }
 walk_min = page_min <  page_semi && node->vn_node.a_min;
 walk_max = page_max >= page_semi && node->vn_node.a_max;
 if (walk_min) {
  /* Recursively continue searching left. */
  if (walk_max) {
   vm_delete_useg(&node->vn_node.a_max,page_min,page_max,
                  ATREE_NEXTMAX(VIRT vm_vpage_t,page_semi,page_level),
                  ATREE_NEXTLEVEL(page_level),old_task);
  }
  ATREE_WALKMIN(VIRT vm_vpage_t,page_semi,page_level);
  pnode = &node->vn_node.a_min;
  goto again;
 } else if (walk_max) {
  /* Recursively continue searching right. */
  ATREE_WALKMAX(VIRT vm_vpage_t,page_semi,page_level);
  pnode = &node->vn_node.a_max;
  goto again;
 }
}


/* Called explicitly during `clone()' */
INTERN void KCALL task_vm_clone(struct task *__restrict new_thread, u32 flags) {
 assert(!FORTASK(new_thread,_this_user_stack));
 if (flags & CLONE_VM) {
  new_thread->t_vm = THIS_VM;
  vm_incref(THIS_VM);
 } else {
  struct userstack *ustack;
  /* Must create a duplicate of the current VM. */
  new_thread->t_vm = vm_clone();
  /* Duplicate and update the associated thread's user-space stack descriptor. */
  if (new_thread->t_vm->vm_map != NULL) {
   if ((ustack = PERTASK_GET(_this_user_stack)) != NULL) {
    struct userstack *new_stack;
    new_stack = (struct userstack *)kmalloc(sizeof(struct userstack),GFP_SHARED);
    new_stack->us_refcnt  = 1;
    new_stack->us_pagemin = ustack->us_pagemin;
    new_stack->us_pageend = ustack->us_pageend;
    FORTASK(new_thread,_this_user_stack) = new_stack; /* Inherit reference. */
    /* Replace references to the stack in the new VM. */
    vm_update_ustack(new_thread->t_vm->vm_map,
                     new_stack->us_pagemin,
                     new_stack->us_pageend-1,
                     VM_SEMI0,
                     VM_LEVEL0,
                     ustack,
                     new_stack);
   }
   if (PERTASK_TESTF(this_task.t_flags,TASK_FOWNUSERSEG|TASK_FUSEREXCEPT)) {
    if (!(flags & CLONE_SETTLS)) {
     /* Inherit user-segment addresses. */
     new_thread->t_userseg = PERTASK_GET(this_task.t_userseg);
     new_thread->t_flags  |= PERTASK_GET(this_task.t_flags) & TASK_FUSEREXCEPT;
    }
    if (PERTASK_TESTF(this_task.t_flags,TASK_FOWNUSERSEG)) {
     if (flags & CLONE_SETTLS) {
      /* A new, explicit user-segment will be defined. - Unmap the copy of the old one. */
      vm_delete_useg(&new_thread->t_vm->vm_map,
                     VM_ADDR2PAGE((uintptr_t)PERTASK_GET(this_task.t_userseg)),
                     VM_ADDR2PAGE((uintptr_t)PERTASK_GET(this_task.t_userseg)+
                                   sizeof(struct user_task_segment)-1),
                     VM_SEMI0,
                     VM_LEVEL0,
                     THIS_TASK);
     } else {
      /* Replace references to the user-segment in the new VM. */
      vm_update_useg(new_thread->t_vm->vm_map,
                     VM_ADDR2PAGE((uintptr_t)new_thread->t_userseg),
                     VM_ADDR2PAGE((uintptr_t)new_thread->t_userseg+
                                   sizeof(struct user_task_segment)-1),
                     VM_SEMI0,
                     VM_LEVEL0,
                     THIS_TASK,
                     new_thread);
      new_thread->t_flags |= TASK_FOWNUSERSEG;
     }
    }
   }
  }
 }
}






/* Destroy a previously allocated vm. */
PUBLIC ATTR_NOTHROW void KCALL
vm_destroy(struct vm *__restrict self) {
 struct vm_node *next,*node;
 vm_func_t *iter;
 assert(!self->vm_tasks);

 /* Execute finalizers. */
 iter = pervm_fini_end;
 while (iter-- != pervm_fini_start)
     SAFECALL_KCALL_VOID_1(**iter,self);

 assertf(self->vm_size >= (size_t)(kernel_pervm_size + PAGEDIR_SIZE),
         "self                             = %p\n"
         "self->vm_size                    = %p\n"
         "kernel_pervm_size + PAGEDIR_SIZE = %p\n",
         self,self->vm_size,kernel_pervm_size + PAGEDIR_SIZE);
 assert(!self->vm_tasks);
 assert((self->vm_map != NULL) == (self->vm_byaddr != NULL));
 /* Delete all the remaining mappings. */
 node = self->vm_byaddr;
 while (node) {
  next = node->vn_byaddr.le_next;
  if (node->vn_notify) {
   INVOKE_NOTIFY_V(node->vn_notify,
                   node->vn_closure,
                   VM_NOTIFY_DECREF,
                   0,
                   0,
                   NULL);
  }
  vm_region_decref_range(node->vn_region,
                         node->vn_start,
                         VM_NODE_SIZE(node));
  vm_region_decref(node->vn_region);
  vm_node_free(node);
  node = next;
 }

 /* Finalize the associated page directory. */
 pagedir_fini(&self->vm_pagedir);
 heap_free(&kernel_heaps[VM_HEAP],
            self,
            self->vm_size,
            VM_HEAP);
}
#undef VM_HEAP


PUBLIC void KCALL
vm_mapat(vm_vpage_t page_index,
         size_t num_pages,
         vm_raddr_t region_start,
         struct vm_region *__restrict region,
         vm_prot_t prot,
         vm_notify_t notify,
         void *closure) {
 size_t EXCEPT_VAR xnum_pages = num_pages;
 vm_raddr_t EXCEPT_VAR xregion_start = region_start;
 struct vm_region *EXCEPT_VAR xregion = region;
 vm_notify_t EXCEPT_VAR xnotify = notify;
 struct vm_node *EXCEPT_VAR node;
 struct vm *EXCEPT_VAR effective_vm;
 if unlikely(!num_pages) return;
 effective_vm = page_index >= X86_KERNEL_BASE_PAGE ? &vm_kernel : THIS_VM;
 assert(page_index+num_pages > page_index);
 assert(page_index+num_pages <= VM_VPAGE_MAX+1);
 assert(region_start+num_pages >= region_start);
 assert(region_start+num_pages <= region->vr_size);
 node = (struct vm_node *)kmalloc(sizeof(struct vm_node),
                                  GFP_SHARED|GFP_LOCKED);
 node->vn_start       = region_start;
 node->vn_region      = region;
 node->vn_notify      = notify;
 node->vn_closure     = closure;
 node->vn_prot        = prot;
 node->vn_flag        = VM_NODE_FNORMAL;
 node->vn_node.a_vmin = page_index;
 node->vn_node.a_vmax = page_index + num_pages - 1;
 TRY {
  if (notify) {
   node->vn_closure = INVOKE_NOTIFY(notify,
                                    closure,
                                    VM_NOTIFY_INCREF,
                                    page_index,
                                    num_pages,
                                    NULL);
  }
  vm_region_incref(region);
  TRY {
   /* Incref the effective region range. */
   vm_region_incref_range(region,region_start,num_pages);
   TRY {
    vm_acquire(effective_vm);
    TRY {
     struct vm_node *EXCEPT_VAR nodes;
     vm_split_before(effective_vm,page_index);
     vm_split_before(effective_vm,page_index+num_pages);
     /* Pop all nodes from the affected range. */
     nodes = vm_pop_nodes(effective_vm,
                          node->vn_node.a_vmin,
                          node->vn_node.a_vmax,
                          VM_UNMAP_NORMAL,NULL);
     TRY {
      /* Insert the new node into the VM and activate (map) it. */
      vm_insert_and_activate_node(effective_vm,node);
      /* Try to merge the new mapping with adjacent memory nodes. */
      vm_merge_before(effective_vm,page_index);
      vm_merge_before(effective_vm,page_index+num_pages);
     } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
      /* Restore all nodes that were popped. */
      while (nodes) {
       struct vm_node *next = nodes->vn_byaddr.le_next;
       vm_insert_node(effective_vm,nodes);
       nodes = next;
      }
      error_rethrow();
     }
     /* Drop all nodes that were unmapped. */
     while (nodes) {
      struct vm_node *next = nodes->vn_byaddr.le_next;
      assert(nodes->vn_node.a_vmin >= node->vn_node.a_vmin);
      assert(nodes->vn_node.a_vmax <= node->vn_node.a_vmax);
      if (nodes->vn_notify) {
       INVOKE_NOTIFY_V(nodes->vn_notify,
                       nodes->vn_closure,
                       VM_NOTIFY_UNMAP,
                       VM_NODE_BEGIN(nodes),
                       VM_NODE_SIZE(nodes),
                       NULL);
       INVOKE_NOTIFY_V(nodes->vn_notify,
                       nodes->vn_closure,
                       VM_NOTIFY_DECREF,
                       0,
                       0,
                       NULL);
      }
      vm_region_decref_range(nodes->vn_region,
                             nodes->vn_start,
                             VM_NODE_SIZE(nodes));
      vm_region_decref(nodes->vn_region);
      vm_node_free(nodes);
      nodes = next;
     }
    } FINALLY {
     vm_release(effective_vm);
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    vm_region_decref_range(xregion,xregion_start,xnum_pages);
    error_rethrow();
   }
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   vm_region_decref(xregion);
   if (xnotify) {
    INVOKE_NOTIFY_V(xnotify,
                    node->vn_closure,
                    VM_NOTIFY_DECREF,
                    0,
                    0,
                    NULL);
   }
   error_rethrow();
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(node);
  error_rethrow();
 }
}


PUBLIC VIRT void *KCALL
vm_map(vm_vpage_t hint,
       size_t num_pages,
       size_t min_alignment_in_pages,
       size_t min_gap_size,
       unsigned int getfree_mode,
       vm_raddr_t region_start, 
       struct vm_region *__restrict region,
       vm_prot_t prot,
       vm_notify_t notify,
       void *closure) {
 size_t EXCEPT_VAR xnum_pages = num_pages;
 vm_raddr_t EXCEPT_VAR xregion_start = region_start;
 struct vm_region *EXCEPT_VAR xregion = region;
 vm_notify_t EXCEPT_VAR xnotify = notify;
 struct vm_node *EXCEPT_VAR node;
 vm_vpage_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct vm *EXCEPT_VAR effective_vm;
 effective_vm = &vm_kernel;
 if (hint < X86_KERNEL_BASE_PAGE ||
    (hint == X86_KERNEL_BASE_PAGE && (getfree_mode&VM_GETFREE_FBELOW)))
     effective_vm = THIS_VM;
 assert(num_pages != 0);
 assert(region_start+num_pages >= region_start);
 assert(region_start+num_pages <= region->vr_size);
 node = (struct vm_node *)kmalloc(sizeof(struct vm_node),
                                  GFP_SHARED|GFP_LOCKED);
 node->vn_start   = region_start;
 node->vn_region  = region;
 node->vn_notify  = notify;
 node->vn_closure = closure;
 node->vn_prot    = prot;
 node->vn_flag    = VM_NODE_FNORMAL;
 TRY {
  vm_acquire(effective_vm);
  TRY {
   /* Find free memory and map the page there. */
   result = vm_getfree(hint,num_pages,min_alignment_in_pages,
                       min_gap_size,getfree_mode);
   node->vn_node.a_vmin = result;
   node->vn_node.a_vmax = result + num_pages - 1;
   if (notify) {
    node->vn_closure = INVOKE_NOTIFY(notify,
                                     closure,
                                     VM_NOTIFY_INCREF,
                                     result,
                                     num_pages,
                                     NULL);
   }
   vm_region_incref(region);
   TRY {
    /* Incref the effective region range. */
    vm_region_incref_range(region,region_start,num_pages);
    TRY {
     /* Insert the new node into the VM and activate (map) it. */
     vm_insert_and_activate_node(effective_vm,node);

     /* Try to merge the new mapping with surrounding nodes. */
     vm_merge_before(effective_vm,VM_NODE_BEGIN(node));
     vm_merge_before(effective_vm,VM_NODE_END(node));
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     vm_region_decref_range(xregion,xregion_start,xnum_pages);
     error_rethrow();
    }
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    vm_region_decref(xregion);
    if (xnotify) {
     INVOKE_NOTIFY_V(xnotify,
                     node->vn_closure,
                     VM_NOTIFY_DECREF,
                     0,
                     0,
                     NULL);
    }
    error_rethrow();
   }
  } FINALLY {
   vm_release(effective_vm);
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(node);
  error_rethrow();
 }
 return (void *)VM_PAGE2ADDR(result);
}


PUBLIC size_t KCALL
vm_extend(vm_vpage_t threshold, size_t num_additional_pages,
          vm_prot_t prot_mask, vm_prot_t prot_flag,
          unsigned int mode) {
 size_t result = 0;
 struct vm *EXCEPT_VAR effective_vm;
 struct vm_node *extension_node;
 assert(mode == VM_EXTEND_ABOVE ||
        mode == VM_EXTEND_BELOW);
 /* Deal with the special case of no-additional-pages. */
 if unlikely(!num_additional_pages)
    return 0;
 effective_vm = threshold >= X86_KERNEL_BASE_PAGE ? &vm_kernel : THIS_VM;
 vm_acquire(effective_vm);
 TRY {
  if (mode == VM_EXTEND_ABOVE) {
   assert(threshold+num_additional_pages > threshold);
   assert(threshold+num_additional_pages <= VM_VPAGE_MAX+1);
   assert(threshold >= X86_KERNEL_BASE_PAGE ||
          threshold+num_additional_pages <= X86_KERNEL_BASE_PAGE);
   /* Lookup the node that should be extended. */
   extension_node = vm_getnode(threshold-1);
   if (!extension_node ||
        extension_node->vn_node.a_vmax != threshold-1)
        goto done;
   /* Make sure the extension area isn't being used. */
   if (vm_getanynode(threshold,threshold+num_additional_pages-1))
       goto done;
  } else {
   assert(threshold >= num_additional_pages);
   assert(threshold < X86_KERNEL_BASE_PAGE ||
          threshold-num_additional_pages >= X86_KERNEL_BASE_PAGE);
   /* Lookup the node that should be extended. */
   extension_node = vm_getnode(threshold);
   if (!extension_node ||
        extension_node->vn_node.a_vmin != threshold)
        goto done;
   /* Make sure the extension area isn't being used. */
   if (vm_getanynode(threshold-num_additional_pages,threshold-1))
       goto done;
  }

  (void)result;
  result = num_additional_pages;

  /* TODO */
  error_throw(E_NOT_IMPLEMENTED);


done:
  ;
 } FINALLY {
  vm_release(effective_vm);
 }
 return 0;
}



































/* VM / VM_REGION Handle operators. */
INTERN unsigned int KCALL
handle_vm_poll(struct vm *__restrict self, unsigned int mode) {
#ifdef CONFIG_VM_USE_RWLOCK
 return rwlock_poll(&self->vm_lock,mode);
#else
 return mutex_poll(&self->vm_lock) ? (mode & (POLLIN|POLLOUT)) : 0;
#endif
}
INTERN unsigned int KCALL
handle_vm_region_poll(struct vm_region *__restrict self, unsigned int mode) {
 return mutex_poll(&self->vr_lock) ? (mode & (POLLIN|POLLOUT)) : 0;
}
INTERN void KCALL
handle_vm_state(struct vm *__restrict self,
                USER CHECKED struct stat64 *result) {
 struct vm *EXCEPT_VAR xself = self;
 size_t total_pages = 0,num_nodes = 0;
 vm_acquire(self);
 TRY {
  struct vm_node *node;
  VM_FOREACH_NODE(node,self) {
   total_pages += VM_NODE_SIZE(node);
   ++num_nodes;
  }
 } FINALLY {
  vm_release(xself);
 }
 memset(result,0,sizeof(struct stat64));
 result->st_size    = (pos_t)total_pages * PAGESIZE; /* Total mapped size. */
 result->st_blocks  = num_nodes;                     /* Number of blocks (nodes). */
 result->st_blksize = PAGESIZE;                      /* Size of a single page. */
}
INTERN void KCALL
handle_vm_region_stat(struct vm_region *__restrict self,
                      USER CHECKED struct stat64 *result) {
 struct vm_region *EXCEPT_VAR xself = self;
 size_t num_parts = 0;
 mutex_get(&self->vr_lock);
 TRY {
  struct vm_part *part;
  /* Count the current number of region parts. */
  VM_REGION_FOREACH_PART(part,self) {
   ++num_parts;
  }
 } FINALLY {
  mutex_put(&xself->vr_lock);
 }
 memset(result,0,sizeof(struct stat64));
 result->st_size    = (pos_t)self->vr_size * PAGESIZE; /* Total number of region pages. */
 result->st_blocks  = num_parts;                       /* Number of blocks (nodes). */
 result->st_blksize = PAGESIZE;                        /* Size of a single page. */
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_VM_VM_C */
