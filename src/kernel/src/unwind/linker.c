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
#ifndef GUARD_KERNEL_SRC_UNWIND_LINKER_C
#define GUARD_KERNEL_SRC_UNWIND_LINKER_C 1
#define _NOSERVE_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <unwind/eh_frame.h>
#include <unwind/linker.h>
#include <stdbool.h>
#include <kernel/debug.h>
#include <kernel/vm.h>
#include <except.h>
#include <fs/driver.h>
#include <fs/linker.h>
#include <fs/path.h>
#include <fs/node.h>

DECL_BEGIN

PUBLIC bool KCALL
linker_findfde_consafe(uintptr_t ip, struct fde_info *__restrict result) {
 bool COMPILER_IGNORE_UNINITIALIZED(fde_ok);
 struct task_connections cons;
 task_push_connections(&cons);
 TRY {
  fde_ok = linker_findfde(ip,result);
 } FINALLY {
  task_pop_connections(&cons);
 }
 return fde_ok;
}

PUBLIC bool KCALL
linker_findexcept_consafe(uintptr_t ip, u16 exception_code,
                          struct exception_handler_info *__restrict result) {
 bool COMPILER_IGNORE_UNINITIALIZED(except_ok);
 struct task_connections cons;
 task_push_connections(&cons);
 TRY {
  except_ok = linker_findexcept(ip,exception_code,result);
 } FINALLY {
  task_pop_connections(&cons);
 }
 return except_ok;
}

INTDEF struct except_handler kernel_except_start[];
INTDEF struct except_handler kernel_except_end[];
INTDEF byte_t kernel_except_size[];
INTDEF byte_t kernel_ehframe_start[];
INTDEF byte_t kernel_ehframe_end[];
INTDEF byte_t kernel_ehframe_size[];


/* Lookup the application at `ip' and load the FDE entry associated with `ip'.
 * NOTE: The caller is responsible to ensure, or deal with problems
 *       caused by the associated application suddenly being unmapped.
 * @return: true:  Successfully located the FDE entry.
 * @return: false: Could not find an FDE entry for `ip' */
PUBLIC bool KCALL
linker_findfde(uintptr_t ip, struct fde_info *__restrict result) {
 REF struct application *EXCEPT_VAR app = NULL;
 bool COMPILER_IGNORE_UNINITIALIZED(fde_ok);
 REF struct vm_region *EXCEPT_VAR region; uintptr_t reloffset;
 struct vm *EXCEPT_VAR effective_vm; struct vm_node *node;
 effective_vm = ip >= KERNEL_BASE ? &vm_kernel : THIS_VM;
#ifdef NDEBUG
 if (!vm_tryacquire(effective_vm))
#endif
 {
  if (PREEMPTION_ENABLED()) {
   u16 EXCEPT_VAR old_state;
   old_state = ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FDONTSERVE);
   TRY {
    vm_acquire(effective_vm);
   } FINALLY {
    if (!(old_state & TASK_STATE_FDONTSERVE))
          ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FDONTSERVE);
   }
  } else {
   /* Can only search the kernel itself. */
   if (effective_vm != &vm_kernel)
       return false;
   return eh_findfde((byte_t *)kernel_ehframe_start,
                     (size_t)kernel_ehframe_size,
                      ip,result,NULL);
  }
 }
 TRY {
  node = vm_getnode(VM_ADDR2PAGE(ip));
  if (node) {
   if (node->vn_notify == &application_notify) {
    app = (REF struct application *)node->vn_closure;
    application_incref(app);
   } else if ((region = node->vn_region)->vr_ctl != NULL) {
    /* Make `ip' become relative to the region. */
    reloffset  = VM_NODE_MINADDR(node);
    reloffset += node->vn_start * PAGESIZE;
    vm_region_incref(region);
    vm_release(effective_vm);
    goto invoke_region; /* XXX: Skip over FINALLY is intentional */
   }
  }
 } FINALLY {
  vm_release(effective_vm);
 }
 if (!app) return false;
 TRY {
  if (!module_loadexcept(app) &&
      !app->a_module->m_sect.m_eh_frame.ds_size)
      fde_ok = false;
  else {
   struct module *mod = app->a_module;
   /* Lookup FDE information. */
   fde_ok = eh_findfde((byte_t *)
                      ((uintptr_t)mod->m_sect.m_eh_frame.ds_base + app->a_loadaddr),
                                  mod->m_sect.m_eh_frame.ds_size,
                        ip,result,NULL);
   /* XXX: `relinfo'? */
  }
 } FINALLY {
  application_decref(app);
 }
 return fde_ok;
invoke_region:
 TRY {
  /* Perform a region control command to find the FDE. */
  fde_ok = (*region->vr_ctl)(region,REGION_CTL_FFIND_FDE,
                             ip-reloffset,result) != 0;
  if (fde_ok)
      result->fi_pcbegin += reloffset,
      result->fi_pcend   += reloffset;
 } FINALLY {
  vm_region_decref(region);
 }
 return fde_ok;
}

PRIVATE bool KCALL
find_exception(struct except_handler *__restrict iter,
               struct except_handler *__restrict end,
               uintptr_t ip, u16 exception_code,
               struct application *__restrict app,
               struct exception_handler_info *__restrict result) {
 struct module *mod = app->a_module;
 /* Lookup exception information. */
 for (; iter < end; ++iter) {
  uintptr_t hand_begin,hand_end; u16 hand_flags;
  hand_flags = iter->eh_flag;
  COMPILER_READ_BARRIER();
  if (hand_flags & ~EXCEPTION_HANDLER_FMASK)
      break; /* Handler contains unknown flags (something's corrupt here). */
  if ((hand_flags & EXCEPTION_HANDLER_FHASMASK) &&
       iter->eh_mask != exception_code)
       continue; /* Non-matching mask. */
  hand_begin = (uintptr_t)iter->eh_begin;
  hand_end   = (uintptr_t)iter->eh_end;
  if (iter->eh_flag & EXCEPTION_HANDLER_FRELATIVE) {
   hand_begin += app->a_loadaddr;
   hand_end   += app->a_loadaddr;
  }
  /* Check handler IP ranges. */
  if (ip < hand_begin) continue;
  if (ip >= hand_end) continue;
  /* Got a matching handler! */
  if unlikely(hand_begin > hand_end)
     break; /* Corruption... */
  if unlikely(hand_begin < app->a_bounds.b_min)
     break; /* Invalid range start */
  if unlikely(hand_end > app->a_bounds.b_max+1)
     break; /* Invalid range end */
  /* All right! we found the handler we're looking for! */
  result->ehi_flag = hand_flags;
  result->ehi_mask = iter->eh_mask;
  if (hand_flags & EXCEPTION_HANDLER_FDESCRIPTOR) {
   uintptr_t entry;
   struct except_desc *descr; u16 descr_type,descr_flag;
   /* The handler uses an exception descriptor. */
   descr = (struct except_desc *)iter->eh_descr;
   if (hand_flags & EXCEPTION_HANDLER_FRELATIVE)
       descr += app->a_loadaddr;
   descr_type = descr->ed_type;
   descr_flag = descr->ed_flags;
   if unlikely(descr_type != EXCEPT_DESC_TYPE_BYPASS)
      break; /* Unknown descriptor type. */
   if unlikely(descr_flag & ~EXCEPT_DESC_FMASK)
      break; /* Unknown descriptor flags. */
   if unlikely((descr_flag & EXCEPT_DESC_FDISABLE_PREEMPTION) &&
               (app->a_type != APPLICATION_TYPE_FDRIVER))
      break; /* User-space isn't allowed to use this flag. */
   entry = (uintptr_t)descr->ed_handler;
   if (descr_flag & EXCEPT_DESC_FRELATIVE)
       entry += app->a_loadaddr;
   if unlikely(entry < (app->a_loadaddr + mod->m_imagemin) ||
               entry > (app->a_loadaddr + mod->m_imageend))
      break; /* Invalid handler entry point */
   /* Save additional information available through the descriptor. */
   result->ehi_entry         = (CHECKED void *)entry;
   result->ehi_desc.ed_type  = descr_type;
   result->ehi_desc.ed_flags = descr_flag;
   result->ehi_desc.ed_safe  = descr->ed_safe;
  } else {
   uintptr_t entry;
   entry = (uintptr_t)iter->eh_entry;
   if (hand_flags & EXCEPTION_HANDLER_FRELATIVE)
       entry += app->a_loadaddr;
   /* Validate the range of the entry point. */
   if unlikely(entry < (app->a_loadaddr + mod->m_imagemin) ||
               entry > (app->a_loadaddr + mod->m_imageend))
      break; /* Invalid handler entry point */
   result->ehi_entry = (UNCHECKED void *)entry;
  }
  return true;
 }
 return false;
}


/* Lookup the effective exception handler for the given `ip' and fill in `result'
 * NOTE: The caller is responsible to ensure, or deal with problems
 *       caused by the associated application suddenly being unmapped.
 * @return: true:  Successfully located an exception handler.
 * @return: false: Could not find an exception handler for `ip' */
FUNDEF bool KCALL
linker_findexcept(uintptr_t ip, u16 exception_code,
                  struct exception_handler_info *__restrict result) {
 REF struct application *EXCEPT_VAR app = NULL;
 bool COMPILER_IGNORE_UNINITIALIZED(except_ok);
 struct vm *EXCEPT_VAR effective_vm; struct vm_node *node;
 effective_vm = ip >= KERNEL_BASE ? &vm_kernel : THIS_VM;
#ifdef NDEBUG
 if (!vm_tryacquire(effective_vm))
#endif
 {
  if (PREEMPTION_ENABLED()) {
   u16 EXCEPT_VAR old_state;
   old_state = ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FDONTSERVE);
   TRY {
    vm_acquire(effective_vm);
   } FINALLY {
    if (!(old_state & TASK_STATE_FDONTSERVE))
          ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FDONTSERVE);
   }
  } else {
   /* Can only search the kernel itself. */
   if (effective_vm != &vm_kernel)
       return false;
   return find_exception(kernel_except_start,
                         kernel_except_end,
                         ip,
                         exception_code,
                        &kernel_driver.d_app,
                         result);
  }
 }
 TRY {
  node = vm_getnode(VM_ADDR2PAGE(ip));
  if (node && node->vn_notify == &application_notify) {
   app = (REF struct application *)node->vn_closure;
   application_incref(app);
  }
 } FINALLY {
  vm_release(effective_vm);
 }
 if (!app) return false;
 TRY {
  except_ok = false;
  if (module_loadexcept(app) ||
      app->a_module->m_sect.m_except.ds_size) {
   struct module *mod = app->a_module;
   struct except_handler *iter;
   iter = (struct except_handler *)((uintptr_t)mod->m_sect.m_except.ds_base+
                                               app->a_loadaddr);
   except_ok = find_exception(iter,
                             (struct except_handler *)((uintptr_t)iter+
                                                        mod->m_sect.m_except.ds_size),
                              ip,exception_code,app,result);
  }
 } FINALLY {
  application_decref(app);
 }
 return except_ok;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_UNWIND_LINKER_C */
