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
#ifndef GUARD_KERNEL_SRC_KERNEL_CACHE_C
#define GUARD_KERNEL_SRC_KERNEL_CACHE_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kernel/malloc.h>
#include <kernel/cache.h>
#include <kernel/debug.h>
#include <kernel/heap.h>
#include <kernel/bind.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <kos/safecall.h>
#include <assert.h>

DECL_BEGIN

PRIVATE struct sig cc_finished = SIG_INIT;          /* Signal broadcast when caches have been cleared. */
PRIVATE ATOMIC_DATA struct task *cc_current = NULL; /* The thread currently clearing caches. */
PRIVATE bool (KCALL *cc_test)(void *arg) = NULL;    /* A test callback to determine when to stop clearing caches. */
PRIVATE void *cc_arg = NULL;                        /* Argument passed to `cc_test' */
PRIVATE bool cc_done = false;                       /* Set to true the first time `cc_test' succeeds. */


/* Returns `false' if caches have been sufficient reduced in size.
 * Returns `true' otherwise. */
PUBLIC ATTR_NOTHROW bool KCALL kernel_cc_done(void) {
 assertf(ATOMIC_READ(cc_current) == THIS_TASK,
         "Caches are not currently being cleared by the calling thread");
 if (!cc_done) cc_done = (*cc_test)(cc_arg);
 return cc_done;
}


typedef void (KCALL *cache_clear_t)(void);
INTDEF cache_clear_t global_clear_caches_start[];
INTDEF cache_clear_t global_clear_caches_end[];

PRIVATE void KCALL truncate_heap_caches(void) {
 /* Figure out which heaps we can actually access. */
 gfp_t i,mask = __GFP_HEAPMASK;
 if (THIS_VM != &vm_kernel) mask &= ~GFP_KERNEL;
 for (i = 0; i < mask; ++i) {
  /* Truncate kernel heaps. */
  if (heap_truncate(&kernel_heaps[i],0) &&
      kernel_cc_done())
      break;
 }
}


/* Clear global caches until `(*test)(arg)' has returned `true'.
 * NOTES:
 *   - If caches are already being cleared by the calling thread, return `false' immediately.
 *   - If caches are already being cleared by the another thread, wait for
 *     it to finish and return `true' (`text' isn't executed in this case)
 *   - If all caches have been cleared (as best as KOS is able to), `false' is returned. */
PUBLIC ATTR_NOTHROW bool KCALL
kernel_cc_invoke(/*ATTR_NOTHROW*/bool (KCALL *test)(void *arg), void *arg) {
 struct task *caller = THIS_TASK,*holder;
 bool result; cache_clear_t *iter;
 assert(test != NULL);
 assert(!task_isconnected());
again:
 holder = ATOMIC_CMPXCH_VAL(cc_current,NULL,caller);
 if (holder == caller) return false; /* The caller is already clearing caches. */
 if (holder != NULL) {
  /* Another thread is clearing caches (connect to the finish signal) */
  task_connect(&cc_finished);
  if (ATOMIC_CMPXCH(cc_current,NULL,caller)) {
   /* Race condition... */
   task_disconnect();
   goto again;
  }
  /* Wait for cache clearing to be completed. */
  task_wait();
  goto again;
 }
 /* Our thread is now holding an exclusive lock on the cache clearing machinery. */
 cc_test = test;
 cc_arg  = arg;
 cc_done = (*test)(arg);

 if (!cc_done) {
  /* Invoke global cache clearing callbacks.
   * Those will then invoke per-task callbacks and go into driver bindings, etc. */
  for (iter = global_clear_caches_start;
       iter < global_clear_caches_end; ++iter) {
#if 1
   debug_printf("%[vinfo:%f(%l,%c) : %n : %p : CC_INVOKE\n]",*iter);
#endif
   SAFECALL_KCALL_VOID_0(**iter);
   if (kernel_cc_done())
       goto done;
  }
  /* Clear kernel heaps in the very end because the cache
   * clear machinery probably just freed a whole bunch of
   * memory. */
  truncate_heap_caches();
 }

done:
 result = kernel_cc_done();
 /* Done clearing caches. - Signal waiting threads. */
 COMPILER_BARRIER();
 ATOMIC_WRITE(cc_current,NULL);
 sig_broadcast(&cc_finished);
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_CACHE_C */
