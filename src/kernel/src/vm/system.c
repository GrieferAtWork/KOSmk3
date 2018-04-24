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
#ifndef GUARD_KERNEL_SRC_VM_SYSTEM_C
#define GUARD_KERNEL_SRC_VM_SYSTEM_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/align.h>
#include <kernel/vm.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <kernel/syscall.h>
#include <kernel/ushare.h>
#include <kernel/user.h>
#include <fs/path.h>
#include <fs/device.h>
#include <fs/node.h>
#include <fs/file.h>
#include <fs/handle.h>
#include <sys/mman.h>
#include <string.h>
#include <except.h>
#include <assert.h>

DECL_BEGIN


/* Optimized function that is allowed to assume that no
 * mapping already exists at the target address `page_index'. */
PRIVATE void KCALL
vm_user_mapnewat_fast(struct vm *__restrict myvm, vm_vpage_t page_index,
                      struct vm_region *__restrict region,
                      vm_prot_t prot, void *closure) {
 struct vm_region *EXCEPT_VAR xregion = region;
 struct vm_node *EXCEPT_VAR node;
 node = (struct vm_node *)kmalloc(sizeof(struct vm_node),
                                  GFP_SHARED|GFP_LOCKED);
 assert(region->vr_parts == &region->vr_part0);
 assert(region->vr_part0.vp_chain.le_next == NULL);
 assert(region->vr_part0.vp_refcnt == 0);
 region->vr_part0.vp_refcnt = 1; /* The reference held by the node. */
 vm_region_incref(region);
 node->vn_node.a_vmin = page_index;
 node->vn_node.a_vmax = page_index + region->vr_size - 1;
 node->vn_start       = 0;
 node->vn_region      = region; /* Inherit reference. */
 node->vn_notify      = NULL;
 node->vn_closure     = closure;
 node->vn_prot        = prot;
 node->vn_flag        = VM_NODE_FNORMAL;
 /* Insert and activate the new node.
  * NOTE: Since this is an activating map() operation, we
  *       can let other CPUs figure this one out lazily. */
 TRY {
  vm_insert_and_activate_node(myvm,node);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  assert(xregion->vr_part0.vp_refcnt == 1); /* The reference held by the node. */
  xregion->vr_part0.vp_refcnt = 0;
  vm_region_decref(xregion);
  kfree(node);
  error_rethrow();
 }
}

PRIVATE void KCALL
vm_user_mapnewat(struct vm *__restrict myvm, vm_vpage_t page_index,
                 vm_raddr_t EXCEPT_VAR region_start, size_t num_pages,
                 struct vm_region *__restrict region,
                 vm_prot_t prot, void *closure) {
 size_t EXCEPT_VAR xnum_pages = num_pages;
 struct vm_region *EXCEPT_VAR xregion = region;
 struct vm_node *EXCEPT_VAR node;
 assert(num_pages != 0);
 assert(region_start+num_pages >= region_start);
 assert(region_start+num_pages <= region->vr_size);
 node = (struct vm_node *)kmalloc(sizeof(struct vm_node),
                                  GFP_SHARED|GFP_LOCKED);
 node->vn_start   = region_start;
 node->vn_region  = region;
 node->vn_notify  = NULL;
 node->vn_closure = closure;
 node->vn_prot    = prot;
 node->vn_flag    = VM_NODE_FNORMAL;
 TRY {
  node->vn_node.a_vmin = page_index;
  node->vn_node.a_vmax = page_index + num_pages - 1;
  /* Incref the effective region range. */
  vm_region_incref_range(region,region_start,num_pages);
  vm_region_incref(region);
  TRY {
   /* Insert the new node into the VM and activate (map) it. */
   vm_insert_and_activate_node(myvm,node);

   /* Try to merge the new mapping with surrounding nodes. */
   vm_merge_before(myvm,VM_NODE_BEGIN(node));
   vm_merge_before(myvm,VM_NODE_END(node));
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   vm_region_decref(xregion);
   vm_region_decref_range(xregion,region_start,xnum_pages);
   error_rethrow();
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  kfree(node);
  error_rethrow();
 }
}

INTDEF ATTR_NORETURN void KCALL
throw_invalid_handle(fd_t fd, u16 reason, u16 istype, u16 rqtype, u16 rqkind);

PRIVATE VIRT void *KCALL
do_xmmap(struct mmap_info *__restrict info) {
 REF struct vm_region *EXCEPT_VAR region;
 REF struct vm_region *EXCEPT_VAR guard_region;
 bool is_extenal_region = false;
 struct vm *EXCEPT_VAR user_vm = THIS_VM;
 vm_vpage_t EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(result);
 vm_vpage_t EXCEPT_VAR hint;
 uintptr_t result_offset = 0;
 size_t EXCEPT_VAR num_pages;

 /* Validate known flag bits. */
 if (info->mi_xflag & ~(XMAP_TYPE|XMAP_FINDBELOW|XMAP_FINDABOVE|
                        XMAP_NOTRYNGAP|XMAP_FORCEGAP|XMAP_NOREMAP))
     error_throw(E_INVALID_ARGUMENT);
 if (info->mi_prot & ~(PROT_EXEC|PROT_WRITE|PROT_READ|PROT_SEM|
                       PROT_LOOSE|PROT_SHARED|PROT_NOUSER))
     error_throw(E_INVALID_ARGUMENT);
 if (info->mi_flags & ~(MAP_TYPE|MAP_FIXED|MAP_ANONYMOUS|
                        MAP_32BIT|MAP_GROWSDOWN|MAP_GROWSUP|MAP_DENYWRITE|
                        MAP_EXECUTABLE|MAP_LOCKED|MAP_NORESERVE|MAP_POPULATE|
                        MAP_NONBLOCK|MAP_STACK|MAP_HUGETLB|MAP_UNINITIALIZED|
                       (MAP_HUGE_MASK << MAP_HUGE_SHIFT)))
     error_throw(E_INVALID_ARGUMENT);

 switch (info->mi_flags & MAP_TYPE) {
 case MAP_AUTOMATIC: break;
 case MAP_SHARED:
  info->mi_prot |= PROT_SHARED;
  break;
 case MAP_PRIVATE:
  if (info->mi_prot & PROT_SHARED)
      error_throw(E_INVALID_ARGUMENT); /* Can't have both... */
  break;
 default: error_throw(E_INVALID_ARGUMENT);
 }
 switch (info->mi_xflag & XMAP_TYPE) {
 case XMAP_VIRTUAL: break;
 case XMAP_PHYSICAL:
  /* Extend the physical address to 64 bits. */
  info->mi_phys.mp_addr64 = (__UINT64_TYPE__)(uintptr_t)info->mi_phys.mp_addr;
 case XMAP_PHYSICAL64:
  /* TODO: Check permissions. */
  result_offset = (uintptr_t)info->mi_addr & (PAGESIZE-1);
  if (result_offset) {
   *(uintptr_t *)&info->mi_phys.mp_addr -= result_offset;
   *(uintptr_t *)&info->mi_size         += result_offset;
   *(uintptr_t *)&info->mi_addr         &= ~(PAGESIZE-1);
  }
  if ((uintptr_t)info->mi_phys.mp_addr & (PAGESIZE-1))
       error_throw(E_INVALID_ARGUMENT);
  break;
 case XMAP_USHARE:
  result_offset = (uintptr_t)info->mi_addr & (PAGESIZE-1);
  if (result_offset) {
   *(uintptr_t *)&info->mi_ushare.mu_start -= result_offset;
   *(uintptr_t *)&info->mi_size            += result_offset;
   *(uintptr_t *)&info->mi_addr            &= ~(PAGESIZE-1);
  }
  if ((uintptr_t)info->mi_ushare.mu_start & (PAGESIZE-1))
       error_throw(E_INVALID_ARGUMENT);
  break;
 default: error_throw(E_INVALID_ARGUMENT);
 }
 if (((uintptr_t)info->mi_addr & (PAGESIZE-1)) &&
      (info->mi_flags & MAP_FIXED))
       error_throw(E_INVALID_ARGUMENT); /* Improperly aligned fixed-memory pointer. */
 hint           = (uintptr_t)info->mi_addr / PAGESIZE;
 num_pages      = CEILDIV(info->mi_size,PAGESIZE);
 if unlikely(!num_pages)
    error_throw(E_INVALID_ARGUMENT); /* Empty mapping. */

 /* Construct a new VM region. */
 region = guard_region = NULL;
 TRY {
  vm_raddr_t region_start = 0;
  if ((info->mi_xflag & XMAP_TYPE) == XMAP_PHYSICAL) {
   region = vm_region_alloc(num_pages);
   region->vr_type                                 = VM_REGION_PHYSICAL;
   region->vr_part0.vp_state                       = VM_PART_INCORE;
   region->vr_part0.vp_flags                       = VM_PART_FKEEP|VM_PART_FWEAKREF;
   region->vr_part0.vp_phys.py_num_scatter         = 1;
   region->vr_part0.vp_phys.py_iscatter[0].ps_addr = (uintptr_t)info->mi_phys.mp_addr / PAGESIZE;
   region->vr_part0.vp_phys.py_iscatter[0].ps_size = info->mi_size;
  } else if ((info->mi_xflag & XMAP_TYPE) == XMAP_USHARE) {
   vm_raddr_t region_end;
   is_extenal_region = true;
   region = ushare_lookup(info->mi_ushare.mu_name);
   region_start = info->mi_ushare.mu_start / PAGESIZE;
   if unlikely(__builtin_add_overflow(region_start,num_pages,&region_end) ||
               region_end > region->vr_size)
      error_throw(E_INVALID_ARGUMENT); /* Cannot map out-of-bounds section of ushare region. */
  } else {
   size_t guard_pages;
   guard_pages = CEILDIV(info->mi_virt.mv_guard,PAGESIZE);
   if (!(info->mi_virt.mv_funds) ||
       !(info->mi_flags&(MAP_GROWSDOWN|MAP_GROWSUP)))
         guard_pages = 0;
   /* Setup memory initialization. */
   if (info->mi_flags & MAP_UNINITIALIZED) {
    /* XXX: Only for restricted processes. Trusted processes
     *      can leave this set to `VM_REGION_INIT_FNORMAL'. */
    region = vm_region_alloc(num_pages);
    region->vr_init = VM_REGION_INIT_FRANDOM;
   } else if (info->mi_flags & MAP_ANONYMOUS) {
    region = vm_region_alloc(num_pages);
    region->vr_init           = VM_REGION_INIT_FFILLER;
    region->vr_setup.s_filler = info->mi_virt.mv_fill;
   } else {
    REF struct handle EXCEPT_VAR hnd;
    REF struct inode *EXCEPT_VAR node;
    hnd = handle_get(info->mi_virt.mv_file);
    TRY {
     switch (hnd.h_type) {

     case HANDLE_TYPE_FPATH:
      atomic_rwlock_read(&hnd.h_object.o_path->p_lock);
      node = hnd.h_object.o_path->p_node;
      inode_incref(node);
      atomic_rwlock_endread(&hnd.h_object.o_path->p_lock);
      goto try_invoke_inode_mmap;

     case HANDLE_TYPE_FFILE:
      node = hnd.h_object.o_file->f_node;
      goto try_invoke_inode_mmap_incref;

     case HANDLE_TYPE_FINODE:
      node = hnd.h_object.o_inode;
try_invoke_inode_mmap_incref:
      inode_incref(node);
try_invoke_inode_mmap:
      if (node->i_ops->io_file.f_mmap) {
       /* Must use the `f_mmap()' operator to create the mapped region. */
       TRY {
        pos_t mmap_start;
        vm_raddr_t region_end;
        mmap_start  = info->mi_virt.mv_off64;
        if (info->mi_virt.mv_begin > mmap_start)
            error_throw(E_INVALID_ARGUMENT);
        mmap_start -= info->mi_virt.mv_begin;
        if (mmap_start & (PAGESIZE-1))
            error_throw(E_INVALID_ARGUMENT);
        region = (*node->i_ops->io_file.f_mmap)(node,
                                               (vm_raddr_t)VM_ADDR2PAGE(mmap_start),
                                               &region_start);
        /* Check if we're able to map the entirety of the request. */
        if (__builtin_add_overflow(region_start,num_pages,&region_end) ||
            region_end > region->vr_size) {
         vm_region_decref(region);
         error_throw(E_INVALID_ARGUMENT);
        }
       } FINALLY {
        inode_decref(node);
       }
       is_extenal_region = true;
       goto got_regions;
      }
      break;

     case HANDLE_TYPE_FDEVICE:
      /* Special case for device memory mappings. */
      if (hnd.h_object.o_device->d_type != DEVICE_TYPE_FCHARDEV)
          goto cannot_mmap_handle;
      if (!hnd.h_object.o_character_device->c_ops->c_file.f_mmap)
          goto cannot_mmap_handle;
      TRY {
       pos_t mmap_start;
       vm_raddr_t region_end;
       mmap_start  = info->mi_virt.mv_off64;
       if (info->mi_virt.mv_begin > mmap_start)
           error_throw(E_INVALID_ARGUMENT);
       mmap_start -= info->mi_virt.mv_begin;
       if (mmap_start & (PAGESIZE-1))
           error_throw(E_INVALID_ARGUMENT);
       region = (*hnd.h_object.o_character_device->c_ops->c_file.f_mmap)(hnd.h_object.o_character_device,
                                                                        (vm_raddr_t)VM_ADDR2PAGE(mmap_start),
                                                                        &region_start);
       /* Check if we're able to map the entirety of the request. */
       if (__builtin_add_overflow(region_start,num_pages,&region_end) ||
           region_end > region->vr_size) {
        vm_region_decref(region);
        error_throw(E_INVALID_ARGUMENT);
       }
      } FINALLY {
       device_decref(hnd.h_object.o_device);
      }
      is_extenal_region = true;
      goto got_regions;
      
     default:
cannot_mmap_handle:
      throw_invalid_handle(info->mi_virt.mv_file,
                           ERROR_INVALID_HANDLE_FWRONGTYPE,
                           hnd.h_type,
                           HANDLE_TYPE_FINODE,
                           HANDLE_KIND_FANY);
     }
     /* Map the INode as a regular file -> region memory mapping. */
     TRY {
      region = vm_region_alloc(num_pages);
     } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
      inode_decref(node);
      error_rethrow();
     }
     region->vr_init                  = VM_REGION_INIT_FFILE_RO;
     region->vr_setup.s_file.f_node   = node; /* Inherit reference. */
     region->vr_setup.s_file.f_start  = info->mi_virt.mv_off64;
     region->vr_setup.s_file.f_begin  = info->mi_virt.mv_begin;
     region->vr_setup.s_file.f_size   = info->mi_virt.mv_len;
     region->vr_setup.s_file.f_filler = info->mi_virt.mv_fill;
    } FINALLY {
     handle_decref(hnd);
    }
#if 0
    node = handle_get_inode(info->mi_virt.mv_file);
    if (node->i_ops->io_file.f_mmap) {
     /* Must use the `f_mmap()' operator to create the mapped region. */
     TRY {
      pos_t mmap_start;
      vm_raddr_t region_end;
      mmap_start  = info->mi_virt.mv_off64;
      if (info->mi_virt.mv_begin > mmap_start)
          error_throw(E_INVALID_ARGUMENT);
      mmap_start -= info->mi_virt.mv_begin;
      if (mmap_start & (PAGESIZE-1))
          error_throw(E_INVALID_ARGUMENT);
      region = (*node->i_ops->io_file.f_mmap)(node,
                                             (vm_raddr_t)VM_ADDR2PAGE(mmap_start),
                                             &region_start);
      /* Check if we're able to map the entirety of the request. */
      if (region &&
          __builtin_add_overflow(region_start,num_pages,&region_end) ||
          region_end > region->vr_size) {
       vm_region_decref(region);
       error_throw(E_INVALID_ARGUMENT);
      }
     } FINALLY {
      inode_decref(node);
     }
     is_extenal_region = true;
     goto got_regions;
    }
    region = vm_region_alloc(num_pages);
    region->vr_init                  = VM_REGION_INIT_FFILE_RO;
    region->vr_setup.s_file.f_node   = node;
    region->vr_setup.s_file.f_start  = info->mi_virt.mv_off64;
    region->vr_setup.s_file.f_begin  = info->mi_virt.mv_begin;
    region->vr_setup.s_file.f_size   = info->mi_virt.mv_len;
    region->vr_setup.s_file.f_filler = info->mi_virt.mv_fill;
#endif
   }
   if (info->mi_flags & MAP_LOCKED)
       region->vr_part0.vp_locked = 1;
   if (!guard_pages) {
    /* Don't create a guard. */
   } else {
    if unlikely(guard_pages > num_pages)
       error_throw(E_INVALID_ARGUMENT);
    if (num_pages == guard_pages) {
     assert(guard_pages);
     /* Only create the guard portion of the requested mapping. */
     guard_region          = region;
     region                = NULL;
     guard_region->vr_size = guard_pages;
    } else {
     /* Create both a guard, and a regular region. */
     region->vr_size        = num_pages - guard_pages;
     guard_region           = vm_region_alloc(guard_pages);
     guard_region->vr_funds = info->mi_virt.mv_funds;
     guard_region->vr_type  = VM_REGION_LOGUARD;
     if (info->mi_flags & MAP_GROWSUP)
         guard_region->vr_type = VM_REGION_HIGUARD;
     /* Copy initialization information from the main region. */
     guard_region->vr_init = region->vr_init;
     switch (guard_region->vr_init) {
     case VM_REGION_INIT_FFILLER:
      guard_region->vr_setup.s_filler = region->vr_setup.s_filler;
      break;
     case VM_REGION_INIT_FFILE:
     case VM_REGION_INIT_FFILE_RO:
      guard_region->vr_setup.s_file.f_node   = region->vr_setup.s_file.f_node;
      inode_incref(guard_region->vr_setup.s_file.f_node);
      guard_region->vr_setup.s_file.f_begin  = region->vr_setup.s_file.f_begin;
      guard_region->vr_setup.s_file.f_start  = region->vr_setup.s_file.f_start;
      guard_region->vr_setup.s_file.f_size   = region->vr_setup.s_file.f_size;
      guard_region->vr_setup.s_file.f_filler = region->vr_setup.s_file.f_filler;
      break;
     default: break;
     }
    }
   }
  }
got_regions:
  /* Regions have been crated. Now to actually map them. */
  vm_acquire(user_vm);
  TRY {
   if (info->mi_flags & MAP_FIXED) {
    vm_vpage_t page_end;
    if (__builtin_add_overflow(hint,num_pages,&page_end) ||
        page_end > X86_KERNEL_BASE_PAGE)
        error_throw(E_INVALID_ARGUMENT); /* Bad address range. */
    result = hint;
    if (info->mi_xflag & XMAP_NOREMAP &&
        vm_getanynode(result,page_end-1) != NULL) {
     /* Check if something's already there. */
     if (vm_getanynode(result,page_end-1)) {
      result        = VM_ADDR2PAGE((uintptr_t)-1);
      result_offset = ((uintptr_t)-1 & (PAGESIZE-1));
      goto dont_map;
     }
    }
    /* Map the regular and the guard region at the given address.
     * NOTE: We use `vm_mapat()' when `MAP_FIXED' was set, so we
     *       can safely deal with mappings already existing. */
    if (info->mi_flags & MAP_GROWSUP) {
     if (region) {
      vm_mapat(hint,region->vr_size,region_start,
               region,info->mi_prot,NULL,info->mi_tag);
      hint += region->vr_size;
     }
     if (guard_region)
         vm_mapat(hint,guard_region->vr_size,0,
                  guard_region,info->mi_prot,
                  NULL,info->mi_tag);
    } else {
     if (guard_region) {
      vm_mapat(hint,guard_region->vr_size,0,guard_region,
               info->mi_prot,NULL,info->mi_tag);
      hint += guard_region->vr_size;
     }
     if (region)
         vm_mapat(hint,region->vr_size,region_start,
                  region,info->mi_prot,NULL,info->mi_tag);
    }
   } else {
    unsigned int EXCEPT_VAR mode;
    vm_vpage_t EXCEPT_VAR gap_pages;
    size_t EXCEPT_VAR page_alignment;
    if (info->mi_align & (info->mi_align-1))
        error_throw(E_INVALID_ARGUMENT);
    if (info->mi_align < PAGEALIGN)
        info->mi_align = PAGEALIGN;
    page_alignment = CEILDIV(info->mi_align,PAGESIZE);

    /* Automatically assign  */
    if (hint != 0 || 
        info->mi_xflag&(XMAP_FINDBELOW|XMAP_FINDABOVE)) {
     if (!(info->mi_xflag&(XMAP_FINDBELOW|XMAP_FINDABOVE)))
           info->mi_xflag |= (XMAP_FINDBELOW|XMAP_FINDABOVE);
     /* Use the user-given hint and mode. */
     mode      = VM_GETFREE_FABOVE;
     gap_pages = CEILDIV(info->mi_gap,PAGESIZE);
     if (info->mi_xflag & XMAP_NOTRYNGAP)
         mode |= VM_GETFREE_FFORCEGAP;
     if (info->mi_xflag & XMAP_FORCEGAP)
         mode |= VM_GETFREE_FNOSMARTGAP;
     if (info->mi_xflag & XMAP_FINDABOVE) {
      if (info->mi_xflag & XMAP_FINDBELOW) {
       vm_vpage_t result_low;
       vm_vpage_t EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(result_high);
       TRY {
        result_high = vm_getfree(hint,num_pages,page_alignment,
                                 gap_pages,mode);
       } CATCH (E_BADALLOC) {
        /* If there is no high-location, still check for one in lower memory. */
        result = vm_getfree(hint,num_pages,page_alignment,
                            gap_pages,mode|VM_GETFREE_FBELOW);
        goto do_map_at_result;
       }
       if (result_high == hint ||
           result_high == hint+1)
        /* If we already got the perfect match (or near perfect),
         * no need to search for a better one. */
        result = result_high;
       else {
        TRY {
         size_t low_diff,high_diff;
         result_low = vm_getfree(hint,num_pages,page_alignment,
                                 gap_pages,mode|VM_GETFREE_FBELOW);
         /* Both a high and a low-memory location was found.
          * Check which one is closer to the given hint. */
#define DIFF_TO(x) (((x)+num_pages <= (hint)) ? ((hint)-((x)+num_pages)) : ((x)-(hint)))
         low_diff  = DIFF_TO(result_low);
         high_diff = DIFF_TO(result_high);
#undef DIFF_TO
         /* Choose the mapping with the smaller distance. */
         result = low_diff <= high_diff ? result_low : result_high;
        } CATCH (E_BADALLOC) {
         result = result_high; /* Use the high-memory location. */
        }
       }
      } else {
       result = vm_getfree(hint,num_pages,page_alignment,
                           gap_pages,mode);
      }
     } else {
      result = vm_getfree(hint,num_pages,page_alignment,
                          gap_pages,mode|VM_GETFREE_FBELOW);
     }
    } else {
     /* Automatically search for a free stack/heap location. */
     if (info->mi_flags & MAP_STACK) {
      result = vm_getfree(VM_USERSTACK_HINT,num_pages,page_alignment,
                          VM_USERSTACK_GAP,VM_USERSTACK_MODE);
     } else {
      result = vm_getfree(VM_USERHEAP_HINT,num_pages,page_alignment,
                          0,VM_USERHEAP_MODE);
     }
    }

do_map_at_result:
    hint = result;
    /* Map the regular and the guard region at their respective addresses. */
    if (info->mi_flags & MAP_GROWSUP) {
     if (region) {
      if (is_extenal_region) {
       vm_user_mapnewat(user_vm,hint,region_start,num_pages,
                        region,info->mi_prot,info->mi_tag);
      } else {
       vm_user_mapnewat_fast(user_vm,hint,region,
                             info->mi_prot,info->mi_tag);
      }
      hint += region->vr_size;
     }
     if (guard_region)
         vm_user_mapnewat_fast(user_vm,hint,guard_region,
                               info->mi_prot,info->mi_tag);
    } else {
     if (guard_region) {
      vm_user_mapnewat_fast(user_vm,hint,guard_region,
                            info->mi_prot,info->mi_tag);
      hint += guard_region->vr_size;
     }
     if (region) {
      if (is_extenal_region) {
       vm_user_mapnewat(user_vm,hint,region_start,num_pages,
                        region,info->mi_prot,info->mi_tag);
      } else {
       vm_user_mapnewat_fast(user_vm,hint,region,
                             info->mi_prot,info->mi_tag);
      }
     }
    }
   }
dont_map:;
  } FINALLY {
   vm_release(user_vm);
  }
 } FINALLY {
  if (region)
      vm_region_decref(region);
  if (guard_region)
      vm_region_decref(guard_region);
 }
 return (VIRT void *)(VM_PAGE2ADDR(result) + result_offset);
}


INTDEF ATTR_NORETURN void KCALL
throw_segfault(VIRT void *addr, uintptr_t reason);

DEFINE_SYSCALL5(mremap,
                VIRT void *,addr,size_t,old_len,size_t,new_len,
                int,flags,VIRT void *,new_addr) {
 uintptr_t offset = 0;
 vm_vpage_t EXCEPT_VAR old_page;
 vm_vpage_t EXCEPT_VAR result;
 vm_vpage_t new_page;
 size_t EXCEPT_VAR new_size;
 size_t old_size;
 /* Check for known flags. */
 if (flags & ~(MREMAP_MAYMOVE|MREMAP_FIXED))
     error_throw(E_INVALID_ARGUMENT);
 /* Copy alignment from the new address. */
 if (flags&MREMAP_FIXED) {
  offset = (uintptr_t)new_addr & (PAGESIZE-1);
  *(uintptr_t *)&new_addr -= offset;
  *(uintptr_t *)&addr     -= offset;
  old_len += offset;
  new_len += offset;
  /* Validate access to the new address range. */
  validate_user(new_addr,new_len);
 }
 /* Align the old and new length by pages. */
 old_len = CEIL_ALIGN(old_len,PAGESIZE);
 new_len = CEIL_ALIGN(new_len,PAGESIZE);
 if unlikely(!old_len || !new_len)
    error_throw(E_INVALID_ARGUMENT);
 if unlikely(!IS_ALIGNED((uintptr_t)addr,PAGESIZE))
    error_throw(E_INVALID_ARGUMENT);
 /* Validate access to the old address range. */
 validate_user(addr,old_len);
 /* At this point, all arguments are properly aligned. */
 old_page = VM_ADDR2PAGE((uintptr_t)addr);
 new_page = VM_ADDR2PAGE((uintptr_t)new_addr);
 old_size = VM_SIZE2PAGES(old_len);
 new_size = VM_SIZE2PAGES(new_len);

 if (new_size <= old_size) {
  /* Unmap trailing pages. */
  if (old_size != new_size) {
   vm_unmap(old_page+new_size,
            old_size-new_size,
            VM_UNMAP_NORMAL|VM_UNMAP_SYNC,
            NULL);
  }
  result = old_page;
  if ((flags & MREMAP_FIXED) && (old_page != new_page)) {
   /* Move the mapping from `old_page...+=new_size' to `new_page...+=new_size' */
   if (!vm_remap(old_page,new_size,new_page,(vm_prot_t)~0,(vm_prot_t)0,VM_REMAP_NORMAL|VM_REMAP_FULL,NULL))
        throw_segfault((void *)VM_PAGE2ADDR(old_page),SEGFAULT_BADREAD);
   result = new_page;
   vm_sync(old_page,old_size);
   vm_sync(new_page,new_size);
  } else if (old_size != new_size) {
   vm_sync(old_page+new_size,old_size-new_size);
  }
 } else if (flags & MREMAP_FIXED) {
  result = new_page;
  vm_acquire(THIS_VM);
  TRY {
   /* Move the old address range into the new one. */
   if (!vm_remap(old_page,old_size,new_page,(vm_prot_t)~0,(vm_prot_t)0,VM_REMAP_NORMAL|VM_REMAP_FULL,NULL))
        throw_segfault((void *)VM_PAGE2ADDR(old_page),SEGFAULT_BADREAD);
   vm_sync(old_page,old_size);
   vm_sync(new_page,old_size);
   /* Unmap memory in the new address range. */
   vm_unmap(new_page+old_page,new_size-old_size,
            VM_UNMAP_NORMAL|VM_UNMAP_SYNC,NULL);
   /* Extend the last page of the old range to fill the additional memory. */
   vm_extend(new_page+old_size,new_size-old_size,(vm_prot_t)~0,(vm_prot_t)0,VM_EXTEND_ABOVE);
  } FINALLY {
   vm_release(THIS_VM);
  }
 } else {
  result = old_page;
  vm_acquire(THIS_VM);
  TRY {
   if (!vm_extend(old_page+old_size,new_size-old_size,(vm_prot_t)~0,(vm_prot_t)0,VM_EXTEND_ABOVE)) {
    /* Move the range to a new location. */
    TRY {
     result = vm_getfree(old_page,new_size,1,0,VM_GETFREE_FABOVE);
    } CATCH (E_BADALLOC) {
     result = vm_getfree(old_page,new_size,1,0,VM_GETFREE_FBELOW);
    }
    /* Got a new location! */
    if (!vm_remap(old_page,old_size,result,
                 (vm_prot_t)~0,(vm_prot_t)0,
                  VM_REMAP_NORMAL|VM_REMAP_FULL,
                  NULL))
         throw_segfault((void *)VM_PAGE2ADDR(old_page),SEGFAULT_BADREAD);
    /* Finally, extend memory at the new location. */
    vm_extend(result+old_size,
              new_size-old_size,
             (vm_prot_t)~0,
             (vm_prot_t)0,
              VM_EXTEND_ABOVE);
   }
  } FINALLY {
   vm_release(THIS_VM);
  }
 }
 /* Return the address of the new mapping. */
 return (uintptr_t)(VM_PAGE2ADDR(result)+offset);
}


DEFINE_SYSCALL2(xmmap,int,version,USER struct mmap_info const *,data){
 /* KOS eXtended system call. */
 struct mmap_info info;
 void *result;
 if (version != MMAP_INFO_CURRENT)
     error_throw(E_NOT_IMPLEMENTED);
 memcpy(&info,data,sizeof(struct mmap_info));
 COMPILER_READ_BARRIER();
 result = do_xmmap(&info);
 return (syscall_ulong_t)result;
}

DEFINE_SYSCALL6(mmap,VIRT void *,addr,size_t,len,int,prot,
                int,flags,fd_t,fd,syscall_ulong_t,off) {
 /* linux-compatible system call. */
 struct mmap_info info;
 info.mi_prot          = (u32)prot;
 info.mi_flags         = (u32)flags;
 info.mi_xflag         = XMAP_FINDAUTO;
 info.mi_addr          = addr;
 info.mi_size          = len;
 info.mi_align         = PAGESIZE;
 info.mi_gap           = PAGESIZE*16;
 info.mi_virt.mv_file  = fd;
 info.mi_virt.mv_begin = 0;
 info.mi_virt.mv_off   = (off_t)off;
 info.mi_virt.mv_len   = CEIL_ALIGN(len,PAGESIZE);
 info.mi_virt.mv_fill  = 0;
 info.mi_virt.mv_guard = PAGESIZE;
 info.mi_virt.mv_funds = MMAP_VIRT_MAXFUNDS;
 return (syscall_ulong_t)do_xmmap(&info);
}

DEFINE_SYSCALL_MUSTRESTART(xmunmap);
DEFINE_SYSCALL4(xmunmap,VIRT void *,addr,size_t,len,int,flags,void *,tag) {
 size_t result,num_pages; vm_vpage_t starting_page;
 /* Validate known flag bits. */
 if (flags & ~(XUNMAP_ALL|XUNMAP_TAG))
     error_throw(E_INVALID_ARGUMENT);
 /* Align to full pages. */
 len += (uintptr_t)addr & (PAGESIZE-1);
 *(uintptr_t *)&addr &= (PAGESIZE-1);
 if unlikely(!len) return 0;
 /* Do the unmap. */
 starting_page = VM_ADDR2PAGE((uintptr_t)addr);
 num_pages     = len/PAGESIZE;
 result = vm_unmap(starting_page,num_pages,
#if XUNMAP_TAG == VM_UNMAP_TAG
                  (flags&XUNMAP_TAG)|VM_UNMAP_SYNC,
#else
                   flags&XUNMAP_TAG
                ? (VM_UNMAP_TAG|VM_UNMAP_SYNC)
                : (VM_UNMAP_NORMAL|VM_UNMAP_SYNC),
#endif
                   tag);
 return result;
}

DEFINE_SYSCALL_MUSTRESTART(munmap);
DEFINE_SYSCALL2(munmap,VIRT void *,addr,size_t,len) {
 size_t num_pages; vm_vpage_t starting_page;
 /* Align to full pages. */
 len += (uintptr_t)addr & (PAGESIZE-1);
 *(uintptr_t *)&addr &= (PAGESIZE-1);
 if unlikely(!len) return 0;
 /* Do the unmap. */
 starting_page = VM_ADDR2PAGE((uintptr_t)addr);
 num_pages     = len/PAGESIZE;
 vm_unmap(starting_page,num_pages,VM_UNMAP_NORMAL|VM_UNMAP_SYNC,NULL);
 return 0;
}

DEFINE_SYSCALL3(mprotect,USER void *,start,size_t,len,int,prot) {
 size_t num_pages; vm_vpage_t starting_page;
 if unlikely(!IS_ALIGNED((uintptr_t)start,PAGESIZE))
    error_throw(E_INVALID_ARGUMENT);
 if (prot & ~(PROT_EXEC|PROT_WRITE|PROT_READ|
              PROT_SEM|PROT_LOOSE|PROT_SHARED|PROT_NOUSER))
     error_throw(E_INVALID_ARGUMENT);
 len = CEIL_ALIGN(len,PAGESIZE);
 if unlikely((uintptr_t)start+len < (uintptr_t)start)
    return -ENOMEM; /* Linux does this too... */
 starting_page = VM_ADDR2PAGE((uintptr_t)start);
 num_pages     = len/PAGESIZE;
 if (vm_protect(starting_page,
                num_pages,
                0,
               (vm_prot_t)prot,
                VM_PROTECT_NORMAL,
                NULL))
     vm_sync(starting_page,num_pages);
 return 0;
}

DEFINE_SYSCALL6(xmprotect,
                USER void *,start,size_t,len,
                int,protmask,int,protflag,
                int,flags,void *,tag) {
 size_t result,num_pages; vm_vpage_t starting_page;
 if unlikely(!IS_ALIGNED((uintptr_t)start,PAGESIZE))
    error_throw(E_INVALID_ARGUMENT);
 if (protflag & ~(PROT_EXEC|PROT_WRITE|PROT_READ|
                  PROT_SEM|PROT_LOOSE|PROT_SHARED|
                  PROT_NOUSER))
     error_throw(E_INVALID_ARGUMENT);
 if (flags & ~(XUNMAP_ALL|XUNMAP_TAG))
     error_throw(E_INVALID_ARGUMENT);
 len = CEIL_ALIGN(len,PAGESIZE);
 if unlikely((uintptr_t)start+len < (uintptr_t)start)
    return -ENOMEM; /* Linux does this too... */
 starting_page = VM_ADDR2PAGE((uintptr_t)start);
 num_pages     = len/PAGESIZE;
 result = vm_protect(starting_page,
                     num_pages,
                     protmask,
                     protflag,
#if XUNMAP_TAG == VM_PROTECT_TAG
                     flags&XUNMAP_TAG,
#else
                     flags&XUNMAP_TAG ? VM_PROTECT_TAG : VM_PROTECT_NORMAL,
#endif
                     tag);
 if (result) vm_sync(starting_page,num_pages);
 return result;
}


#define __NR_mremap       216
#define __NR_swapon       224
#define __NR_swapoff      225


DECL_END

#endif /* !GUARD_KERNEL_SRC_VM_SYSTEM_C */
