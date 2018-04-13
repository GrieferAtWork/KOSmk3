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
#ifndef GUARD_KERNEL_SRC_SCHED_UNSHARE_C
#define GUARD_KERNEL_SRC_SCHED_UNSHARE_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <sched/task.h>
#include <kernel/malloc.h>
#include <kernel/syscall.h>
#include <kernel/vm.h>
#include <bits/sched.h>
#include <fs/path.h>
#include <fs/handle.h>
#include <sched/posix_signals.h>
#include <except.h>

DECL_BEGIN


PUBLIC bool KCALL task_unshare_files(void) {
 REF struct handle_manager *new_man,*old_man;
 assert(THIS_HANDLE_MANAGER->hm_refcnt != 0);
 /* Check if the handler manager was already unshared. */
 if (THIS_HANDLE_MANAGER->hm_refcnt == 1)
     return false;
 new_man = handle_manager_clone();
 old_man = THIS_HANDLE_MANAGER;
 THIS_HANDLE_MANAGER = new_man;
 COMPILER_WRITE_BARRIER();
 handle_manager_decref(old_man);
 return true;
}

PUBLIC bool KCALL task_unshare_fs(void) {
 REF struct fs *new_fs,*old_fs;
 assert(THIS_FS->fs_refcnt != 0);
 /* Check if the handler manager was already unshared. */
 if (THIS_FS->fs_refcnt == 1)
     return false;
 new_fs = fs_clone();
 old_fs = THIS_FS;
 THIS_FS = new_fs;
 COMPILER_WRITE_BARRIER();
 fs_decref(old_fs);
 return true;
}


INTDEF void KCALL sighand_decref(struct sighand *__restrict self);
INTDEF void KCALL sighand_ptr_decref(struct sighand_ptr *__restrict self);

PUBLIC bool KCALL task_unshare_sighand(void) {
 REF struct sighand *hand;
 struct sighand_ptr *new_ptr;
 struct sighand_ptr *handptr = PERTASK(_this_sighand_ptr);
 if (!handptr) return false; /* No handlers are being used. */
 assert(handptr->sp_refcnt != 0);
 atomic_rwlock_read(&handptr->sp_lock);
 if (!handptr->sp_hand) {
  /* No handlers have been defined, but we might still
   * be sharing the hand-pointer with another thread. */
  if (handptr->sp_refcnt == 1) {
   atomic_rwlock_endread(&handptr->sp_lock);
   return false; /* Handlers are not being shared. */
  }
  atomic_rwlock_endread(&handptr->sp_lock);
  /* Release the hand pointer. */
  PERTASK(_this_sighand_ptr) = NULL;
  sighand_ptr_decref(handptr);
  return true;
 }
 /* There are actual handlers at use here. */
 hand = handptr->sp_hand;
 atomic_rwlock_write(&hand->sh_lock);
 atomic_rwlock_endread(&handptr->sp_lock);
 /* Increment the copy-on-write reference
  * counter of the SIGHAND descriptor. */
 ++hand->sh_share;
 atomic_rwlock_endwrite(&hand->sh_lock);
 TRY {
  /* Allocate a new sighand-share pointer for this thread. */
  new_ptr = (REF struct sighand_ptr *)kmalloc(sizeof(struct sighand_ptr),
                                              GFP_SHARED);
  new_ptr->sp_refcnt = 1;
  atomic_rwlock_init(&new_ptr->sp_lock);
  new_ptr->sp_hand = hand; /* Inherit reference. */
 } EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  /* Drop the shared reference we've created before. */
  sighand_decref(hand);
  error_rethrow();
 }
 PERTASK(_this_sighand_ptr) = new_ptr; /* Inherit reference. */
 sighand_ptr_decref(handptr);
 return true;
}

PUBLIC bool KCALL task_unshare_vm(void) {
 REF struct vm *new_vm;
 assert(THIS_VM->vm_refcnt != 0);
 if (THIS_VM->vm_refcnt == 1)
     return true;
 /* Create a clone of the current VM. */
 new_vm = vm_clone();
 TRY {
  /* Set the new VM as the one currently active. */
  task_setvm(new_vm);
 } FINALLY {
  /* Drop the reference returned by `vm_clone()' */
  vm_decref(new_vm);
 }
 return true;
}



PUBLIC size_t KCALL task_unshare(u32 flags) {
 size_t result = 0;
 if (flags & CLONE_FILES)
     result += task_unshare_files();
 if (flags & CLONE_FS)
     result += task_unshare_fs();
 if (flags & CLONE_SIGHAND)
     result += task_unshare_sighand();
 if (flags & CLONE_VM)
     result += task_unshare_vm();
 return result;
}



DEFINE_SYSCALL1(unshare,u32,flags) {
 /* Check if `flags' contains bits that we're not handling. */
 if (flags & ~(CLONE_FILES|CLONE_FS|CLONE_SIGHAND|CLONE_VM))
     error_throw(E_INVALID_ARGUMENT);
 /* Do the unshare. */
 task_unshare(flags);
 return 0;
}




DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_UNSHARE_C */
