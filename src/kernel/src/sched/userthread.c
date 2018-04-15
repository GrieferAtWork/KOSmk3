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
#ifndef GUARD_KERNEL_SRC_SCHED_USERTHREAD_C
#define GUARD_KERNEL_SRC_SCHED_USERTHREAD_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/atomic.h>
#include <kernel/vm.h>
#include <kernel/sections.h>
#include <kernel/user.h>
#include <kernel/syscall.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <sched/userthread.h>
#include <sched/pid.h>
#include <except.h>

DECL_BEGIN

/* [0..1][lock(PRIVATE(THIS_TASK))]
 * The user-space address of the TID pointer of this thread.
 * Initially, this pointer is set according to the `ctid' argument
 * of the `clone()' system call. And in addition, it may be altered
 * at a later time using the `set_tid_address()' system call.
 * When the thread terminates, the kernel will attempt to write ZERO(0) to this
 * address (failure to do this is silently ignored), before broadcasting the
 * signal of a futex at `vm_getfutex(THIS_TID_ADDRESS)' (should that futex exists)
 * This functionality is used to implement user-space thread_join() functionality. */
PUBLIC ATTR_PERTASK USER UNCHECKED VIRT pid_t *_this_tid_address = NULL;

DEFINE_PERTASK_CLEANUP(userthread_signal);
INTERN void KCALL userthread_signal(void) {
 USER VIRT pid_t *pid_addr;
 REF struct futex *futex_object;
 pid_addr = THIS_TID_ADDRESS;
 if (!pid_addr) return; /* Don't do anything when this is NULL. */
 TRY {
  validate_writable(pid_addr,sizeof(pid_t));
  ATOMIC_STORE(pid_addr,0);
  COMPILER_WRITE_BARRIER();
  /* Also try to signal a futex at the address. */
  futex_object = vm_getfutex(pid_addr);
  if (futex_object) {
   sig_broadcast(&futex_object->f_sig);
   futex_decref(futex_object);
  }
 } CATCH (E_SEGFAULT) {
  /* Ignore a possible segmentation fault
   * when writing to the TID address. */
 }
}



DEFINE_SYSCALL1(set_tid_address,
                USER UNCHECKED pid_t *,tid_pointer) {
 PERTASK_SET(_this_tid_address,tid_pointer);
 return posix_gettid();
}




DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_USERTHREAD_C */
