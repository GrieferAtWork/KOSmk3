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
#ifndef GUARD_KERNEL_SRC_KERNEL_BOOT_C
#define GUARD_KERNEL_SRC_KERNEL_BOOT_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/malloc.h>
#include <hybrid/section.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <sched/task.h>
#include <assert.h>
#include <string.h>

DECL_BEGIN


PRIVATE ATTR_FREEBSS DEFINE_ATOMIC_RWLOCK(bw_thread_lock);
PRIVATE ATTR_FREEBSS size_t            bw_thread_a = 0;
PRIVATE ATTR_FREEBSS size_t            bw_thread_c = 0;
PRIVATE ATTR_FREEBSS REF struct task **bw_thread_v = NULL;

/* Register a thread that should be join()-ed prior to .free being deleted.
 * This function is used by the PS/2 detection thread to ensure that connected
 * devices have been detected and initialized before setup code is deleted. */
INTERN ATTR_FREETEXT void KCALL
kernel_register_bootworker(struct task *__restrict thread) {
 struct task **old_vector;
again:
 old_vector = NULL;
 atomic_rwlock_write(&bw_thread_lock);
 assert(bw_thread_c <= bw_thread_a);
 if (bw_thread_c == bw_thread_a) {
  REF struct task **new_vector;
  size_t new_alloc = bw_thread_a*2;
  if unlikely(!new_alloc) {
#if HEAP_ALIGNMENT >= __SIZEOF_POINTER__
   new_alloc = HEAP_ALIGNMENT/__SIZEOF_POINTER__;
#else
   new_alloc = 1;
#endif
  }
  atomic_rwlock_endwrite(&bw_thread_lock);
  new_vector = (REF struct task **)kmalloc(new_alloc*
                                           sizeof(REF struct task *),
                                           GFP_SHARED);
  atomic_rwlock_write(&bw_thread_lock);
  if unlikely(new_alloc <= bw_thread_a) {
   atomic_rwlock_endwrite(&bw_thread_lock);
   kfree(new_vector);
   goto again;
  }
  old_vector = bw_thread_v;
  /* Copy old pointers into the new vector. */
  memcpy(new_vector,old_vector,
         bw_thread_c*sizeof(REF struct task *));
  bw_thread_v = new_vector;
  bw_thread_a = new_alloc;
 }
 bw_thread_v[bw_thread_c++] = thread;
 atomic_rwlock_endwrite(&bw_thread_lock);
 /* The reference stored in `bw_thread_v' */
 task_incref(thread);
 /* Free the old vector. */
 kfree(old_vector);
}

/* Join all boot worker threads and return once they've all terminated. */
INTERN ATTR_FREETEXT void KCALL
kernel_join_bootworkers(void) {
 size_t i = 0;
 /* Join all the threads. */
 atomic_rwlock_read(&bw_thread_lock);
 for (; i < bw_thread_c; ++i) {
  struct task *thread = bw_thread_v[i];
  if (TASK_ISTERMINATING(thread))
      continue;
  atomic_rwlock_endread(&bw_thread_lock);
  /* Join this thread. */
  task_connect_join(thread);
  task_wait();

  atomic_rwlock_read(&bw_thread_lock);
 }
 atomic_rwlock_endread(&bw_thread_lock);

 /* Since the bootworker API is part of .free, we can assume
  * that we're now the only thread left that is still allowed
  * to use anything located in .free (since we're the ones
  * responsible to actually delete .free later on)
  * Because of that, no further lock is required now! */
 while (bw_thread_c--)
     task_decref(bw_thread_v[bw_thread_c]);
 kfree(bw_thread_v);
}

DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_BOOT_C */
