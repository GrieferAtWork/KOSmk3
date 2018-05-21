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
#ifndef GUARD_KERNEL_SRC_KERNEL_PANIC_C
#define GUARD_KERNEL_SRC_KERNEL_PANIC_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <fs/handle.h>
#include <fs/path.h>
#include <fs/node.h>
#include <fs/device.h>
#include <kos/handle.h>
#include <sched/group.h>
#include <hybrid/section.h>
#include <kernel/panic.h>
#include <kernel/vm.h>
#include <sched/task.h>
#include <sched/rwlock.h>
#include <sched/mutex.h>
#include <sched/posix_signals.h>
#include <sched/pid.h>
#include <except.h>

#include "../sched/rwlock.h"
#include "../sched/rpc.h"

DECL_BEGIN

#define LOCK_RWLOCK(x) \
 ((x).rw_state = __RWLOCK_STATE(RWLOCK_MODE_FWRITING,1), \
  sig_init(&(x).rw_chmode),sig_init(&(x).rw_unshare), \
  (x).rw_xowner = THIS_TASK)
#define LOCK_MUTEX(x) \
 (sig_init(&(x).m_signal),(x).m_owner = THIS_TASK,(x).m_ind = 1)


INTDEF ATTR_PERTASK struct sig task_join_signal;

PRIVATE ATTR_COLDTEXT ATTR_NOTHROW bool KCALL
lock_task(struct task *__restrict thread, uintptr_t mode) {
 TRY {
  atomic_rwlock_init(&thread->t_vm_lock);
  sig_init(&FORTASK(thread,task_join_signal));
  if (thread->t_vm) {
   atomic_rwlock_init(&thread->t_vm->vm_tasklock);
#ifdef CONFIG_VM_USE_RWLOCK
   LOCK_RWLOCK(thread->t_vm->vm_lock);
#else
   LOCK_MUTEX(thread->t_vm->vm_lock);
#endif
  }
  if (FORTASK(thread,_this_handle_manager))
      atomic_rwlock_init(&FORTASK(thread,_this_handle_manager)->hm_lock);
  if (FORTASK(thread,_this_fs)) {
   struct fs *f = FORTASK(thread,_this_fs);
   struct vfs *vf = f->fs_vfs;
   atomic_rwlock_init(&f->fs_lock);
   if (vf) {
    atomic_rwlock_init(&vf->v_recent.r_lock);
    atomic_rwlock_init(&vf->v_mount.m_lock);
    atomic_rwlock_init(&vf->v_drives.d_lock);
   }
  }
  if (thread == FORTASK(thread,_this_group).tg_leader) {
   atomic_rwlock_init(&FORTASK(thread,_this_group).tg_process.h_lock);
   sig_init(&FORTASK(thread,_this_group).tg_process.h_cldevent);
   atomic_rwlock_init(&FORTASK(thread,_this_group).tg_process.h_procgroup.pg_lock);
  }
  if (FORTASK(thread,_this_pid)) {
   struct pidns *ns = FORTASK(thread,_this_pid)->tp_ns;
   atomic_rwlock_init(&FORTASK(thread,_this_pid)->tp_task_lock);
   for (; ns; ns = ns->pn_parent)
       atomic_rwlock_init(&ns->pn_lock);
  }
  if (FORTASK(thread,_this_sighand_ptr)) {
   struct sighand *hand;
   atomic_rwlock_init(&FORTASK(thread,_this_sighand_ptr)->sp_lock);
   hand = FORTASK(thread,_this_sighand_ptr)->sp_hand;
   if (hand) {
    atomic_rwlock_init(&hand->sh_lock);
   }
  }
 } EXCEPT_HANDLED(EXCEPT_EXECUTE_HANDLER) {
 }
 return true;
}

PUBLIC ATTR_COLDTEXT ATTR_NOTHROW
void KCALL kernel_panic_lockall(unsigned int mode) {
 struct task *thread = THIS_TASK;
 error_pushinfo();

 /* Reset read-locks */
 FORTASK(thread,my_readlocks).rls_use = 0;
 FORTASK(thread,my_readlocks).rls_cnt = 0;
 FORTASK(thread,my_readlocks).rls_msk = CONFIG_TASK_STATIC_READLOCKS-1;
 FORTASK(thread,my_readlocks).rls_vec = FORTASK(thread,my_readlocks).rls_sbuf;
 
 /* Delete pending RPCs. */
 FORTASK(thread,my_rpc).ri_cnt = 0;
 ATOMIC_FETCHAND(thread->t_state,
               ~(TASK_STATE_FINTERRUPTED |
                 TASK_STATE_FINTERRUPTING | 
                 TASK_STATE_FHELPMETERM /* May as well make sure this flag isn't set here... */
                 ));

 /* Start by locking vm_kernel. */
#ifdef CONFIG_VM_USE_RWLOCK
 LOCK_RWLOCK(vm_kernel.vm_lock);
#else
 LOCK_MUTEX(vm_kernel.vm_lock);
#endif

 /* Lock some misc. kernel structures. */
 atomic_rwlock_init(&handle_manager_kernel.hm_lock);
 atomic_rwlock_init(&fs_kernel.fs_lock);
 atomic_rwlock_init(&vfs_kernel.v_recent.r_lock);
 atomic_rwlock_init(&vfs_kernel.v_mount.m_lock);
 atomic_rwlock_init(&vfs_kernel.v_drives.d_lock);

 /* Just in case the caller isn't tracked properly, lock them manually. */
 lock_task(thread,mode);

 /* Lock resources of all threads. */
 if (mode & PANIC_LOCK_PERTASK) {
  task_foreach_weak((bool(KCALL *)(struct task *__restrict,void *))&lock_task,
                    (void *)(uintptr_t)mode);
 }

 error_popinfo();
}

PUBLIC ATTR_NOTHROW void KCALL
kernel_panic_printtb(struct task *__restrict thread,
                     struct cpu_context *__restrict ctx,
                     unsigned int num_skip_frames) {
 /* TODO:
  *   #1: Attempt to unwind using CFI.
  *   #2: Attempt to unwind using EBP.
  *   #3: Attempt to unwind by dumping all text
  *       addresses in the active stack segment. */

}


DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_PANIC_C */
