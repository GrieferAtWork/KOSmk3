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
#ifndef GUARD_KERNEL_SRC_SCHED_ONEXIT_C
#define GUARD_KERNEL_SRC_SCHED_ONEXIT_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/align.h>
#include <kos/safecall.h>
#include <kos/types.h>
#include <sched/onexit.h>
#include <kernel/bind.h>
#include <hybrid/sync/atomic-rwptr.h>
#include <kernel/sections.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <sched/task.h>
#include <fs/driver.h>
#include <except.h>
#include <string.h>

DECL_BEGIN

struct onexit_entry {
    task_onexit_t      oe_func;  /* [0..1] Function executed (NOTE: NULL is used as sentinel). */
    void              *oe_arg;   /* [?..?] Argument passed to `oe_func' */
    REF struct driver *oe_owner; /* [1..1] The owner of this entry. */
};
struct thread_onexit {
    struct onexit_entry te_entries[1]; /* [1..kmalloc_usable_size(self)/sizeof(struct onexit_entry)]
                                        *  Inline vector of on-exit callbacks. */
};

#if 1 /* XXX: Only if `offsetof(struct thread_onexit,te_entries) == 0' */
#define ONEXIT_SIZE(x) (kmalloc_usable_size(x) / sizeof(struct onexit_entry))
#else
LOCAL size_t KCALL ONEXIT_SIZE(struct thread_onexit *x) {
 size_t result = kmalloc_usable_size(x);
 if (result) result -= offsetof(struct thread_onexit,te_entries);
 return result / sizeof(struct onexit_entry);
}
#endif

/* [TYPE(struct thread_onexit *)][0..1][lock(.)][owned] */
INTERN ATTR_PERTASK DEFINE_ATOMIC_RWPTR(_this_onexit,NULL);


DEFINE_PERTASK_UNBIND_DRIVER(unbind_on_exit);
PRIVATE ATTR_USED void KCALL
unbind_on_exit(struct task *__restrict thread,
               struct driver *__restrict d) {
 struct thread_onexit *onexit; size_t i,count;
 atomic_rwptr_t *ptr = &FORTASK(thread,_this_onexit);
 bool has_write_lock = false;
 task_onexit_t func; void *arg;
 if (!ATOMIC_RWPTR_GET(*ptr)) return;
 atomic_rwptr_read(ptr);
again:
 COMPILER_READ_BARRIER();
 onexit = (struct thread_onexit *)ATOMIC_RWPTR_GET(*ptr);
 count = ONEXIT_SIZE(onexit);
 for (i = 0; i < count; ++i) {
  if (onexit->te_entries[i].oe_owner != d) continue;
  if (!onexit->te_entries[i].oe_func) break; /* Sentinel */
  /* Found one! */
  if (!has_write_lock) {
   has_write_lock = true;
   if (!atomic_rwptr_upgrade(ptr))
        goto again;
  }
  /* Capture the function so we can invoke it. */
  func = onexit->te_entries[i].oe_func;
  arg  = onexit->te_entries[i].oe_arg;
  /* Move other entires. */
  memmove(&onexit->te_entries[i],
          &onexit->te_entries[i+1],
         ((count-1)-i)*sizeof(struct onexit_entry));
  memset(&onexit->te_entries[count-1],0,
          sizeof(struct onexit_entry));
  atomic_rwptr_endwrite(ptr);
  /* Invoke the callback under the pretense of a driver close. */
  SAFECALL_KCALL_VOID_3(*func,thread,ONEXIT_REASON_DRIVERCLOSE,arg);
  assert(d->d_app.a_refcnt >= 2);
  ATOMIC_FETCHDEC(d->d_app.a_refcnt); /* The reference previously stored in `oe_owner' */
  goto again;
 }
 if (has_write_lock)
      atomic_rwptr_endwrite(ptr);
 else atomic_rwptr_endread(ptr);
}


DEFINE_PERTASK_CACHE_CLEAR(free_unused_onexit);
PRIVATE ATTR_USED void KCALL
free_unused_onexit(struct task *__restrict thread) {
 struct thread_onexit *onexit; size_t i,count;
 atomic_rwptr_t *ptr = &FORTASK(thread,_this_onexit);
 /* Quick (weak) check: are there any callbacks? */
 if (!ATOMIC_RWPTR_GET(*ptr)) return;
 COMPILER_READ_BARRIER();
 atomic_rwptr_write(ptr);
 onexit = (struct thread_onexit *)ATOMIC_RWPTR_GET(*ptr);
 count = ONEXIT_SIZE(onexit);
 for (i = 0; i < count; ++i)
    if (!onexit->te_entries[i].oe_func) break;
 if (i < count) {
  /* Since this operation only ever reduces the size, it's NOTHROW.
   * (The GFP_NOMOVE isn't actually necessary here...) */
  krealloc(onexit,
           offsetof(struct thread_onexit,te_entries)+
           i*sizeof(struct onexit_entry),
           GFP_CALLOC|GFP_NOMOVE|GFP_NOTRIM);
 }
}


PRIVATE ATTR_USED void KCALL
do_exec_onexit(struct task *__restrict thread, unsigned int reason) {
 struct thread_onexit *onexit; size_t i,count;
 atomic_rwptr_t *ptr = &FORTASK(thread,_this_onexit);
 /* Quick (weak) check: are there any callbacks? */
 if (!ATOMIC_RWPTR_GET(*ptr)) return;
 COMPILER_READ_BARRIER();
 atomic_rwptr_write(ptr);
 onexit = (struct thread_onexit *)ATOMIC_RWPTR_GET(*ptr);
 ATOMIC_WRITE(ptr->ap_data,0); /* Unlock + clear. */
 /* Execute callbacks (NOTE: If due to a race condition `onexit'
  *                          became `NULL', `kmalloc_usable_size()'
  *                          simply returns ZERO(0) for `NULL',
  *                          and kfree() is a no-op) */
 count = ONEXIT_SIZE(onexit);
 for (i = 0; i < count; ++i) {
  if unlikely(!onexit->te_entries[i].oe_func) {
   assertf(!memxchr(&onexit->te_entries[i],0,
                   (count-i)*sizeof(struct onexit_entry)),
            "A NULL-entry is a the sentinel, and all following entries must be NULL");
   break; /* End of the callback list */
  }
  /* Execute the callback. */
  SAFECALL_KCALL_VOID_3(*onexit->te_entries[i].oe_func,
                         thread,
                         reason,
                         onexit->te_entries[i].oe_arg);
  driver_decref(onexit->te_entries[i].oe_owner);
 }
 kfree(onexit);
}

DEFINE_PERTASK_FINI(exec_onexit);
PRIVATE ATTR_USED void KCALL
exec_onexit(struct task *__restrict thread) {
 do_exec_onexit(thread,ONEXIT_REASON_DESTRUCTION);
}

DEFINE_ABS_CALLBACK(".rodata.callback.pertask.cleanup.onexit",cleanup_onexit);
PRIVATE ATTR_USED void KCALL cleanup_onexit(void) {
 do_exec_onexit(THIS_TASK,ONEXIT_REASON_TERMINATION);
}




/* Query a function to-be executed by `thread' during its cleanup phase.
 * NOTES:
 *   - If `thread' hasn't been started yet and construction is aborted
 *     using `task_failed()', the given `callback' will be executed in
 *     the context of the thread that called `task_failed' before said
 *     function returns.
 *     With that in mind, if this function returns `true', it is
 *     guarantied that `callback' will be executed eventually.
 *   - onexit() callbacks are executed in the same order they were added.
 *   - onexit() callbacks are executed early on during the task cleanup
 *     phase: After `TASK_STATE_FTERMINATING' was set, and after `task_serve()'
 *     was called to serve any remaining RPC functions, yet prior to
 *     any callbacks defined using `DEFINE_PERTASK_CLEANUP()'
 *   - This function can be called multiple times with the same arguments,
 *     to have the same callback be registered more than once.
 * @return: false:    `thread' has already terminated, or is currently in
 *                     the process to terminating.
 *                     To prevent a race condition of new onexit() jobs
 *                     be scheduled once the thread is ready, or has
 *                     started executing already existing jobs, we simply
 *                     disallow the addition of new jobs, just as is the
 *                     case with `task_queue_rpc()'.
 * @return: true:      Successfully queued the given `callback' for execution.
 * @throw: E_BADALLOC: Failed to allocate sufficient memory for the internal
 *                     on-exit control structures.
 * @throw: E_DRIVER_CLOSED: The calling driver has been closed. */
PUBLIC bool KCALL
__os_task_queue_onexit(struct task *__restrict thread,
                       task_onexit_t callback, void *arg,
                       struct driver *__restrict owner) {
 atomic_rwptr_t *ptr;
 struct thread_onexit *old_onexit;
 struct thread_onexit *onexit;
 size_t old_size,new_size,i;
 assert(callback != NULL);
 ptr = &FORTASK(thread,_this_onexit);
again:
 atomic_rwptr_write(ptr);
 /* Check if the thread has already exited. */
 if unlikely(TASK_ISTERMINATING(thread)) {
  atomic_rwptr_endwrite(ptr);
  return false;
 }
 old_onexit = (struct thread_onexit *)ATOMIC_RWPTR_GET(*ptr);
 old_size   = ONEXIT_SIZE(old_onexit);
 for (i = 0; i < old_size; ++i) {
  if (old_onexit->te_entries[i].oe_func) continue;
  if unlikely((owner->d_app.a_flags & APPLICATION_FCLOSING) ||
              !driver_tryincref(owner)) {
   atomic_rwptr_endwrite(ptr);
   error_throw(E_DRIVER_CLOSED);
  }
  /* Found an unused entry. (Use it!) */
  old_onexit->te_entries[i].oe_func  = callback;
  old_onexit->te_entries[i].oe_arg   = arg;
  old_onexit->te_entries[i].oe_owner = owner; /* Inherit reference. */
  atomic_rwptr_endwrite(ptr);
  return true;
 }
 atomic_rwptr_endwrite(ptr);
 /* Allocate a larger onexit vector. */
 new_size  = old_size;
 new_size += CEILDIV(HEAP_ALIGNMENT,sizeof(struct onexit_entry));
 onexit    = (struct thread_onexit *)kmemalign(ATOMIC_RWPTR_ALIGN,
                                               offsetof(struct thread_onexit,te_entries)+
                                               new_size*sizeof(struct onexit_entry),
                                               GFP_SHARED|GFP_CALLOC);
 /* Re-lock the vector. */
 atomic_rwptr_write(ptr);
 old_onexit = (struct thread_onexit *)ATOMIC_RWPTR_GET(*ptr);
 old_size   = ONEXIT_SIZE(old_onexit);
 if unlikely(new_size < old_size) {
  /* Our new vector is too small... (start over) */
  atomic_rwptr_endwrite(ptr);
  kfree(onexit);
  goto again;
 }
 /* Copy the vector over. */
 memcpy(onexit,old_onexit,
        offsetof(struct thread_onexit,te_entries)+
       (old_size*sizeof(struct onexit_entry)));
 /* Find the first free spot. */
 for (i = 0; i < new_size; ++i) {
  if (old_onexit->te_entries[i].oe_func) continue;
  if unlikely((owner->d_app.a_flags & APPLICATION_FCLOSING) ||
              !driver_tryincref(owner)) {
   atomic_rwptr_endwrite(ptr);
   kfree(onexit);
   error_throw(E_DRIVER_CLOSED);
  }
  /* Found an unused entry. (Use it!) */
  old_onexit->te_entries[i].oe_func  = callback;
  old_onexit->te_entries[i].oe_arg   = arg;
  old_onexit->te_entries[i].oe_owner = owner; /* Inherit reference. */
  atomic_rwptr_endwrite(ptr);
  break;
 }
 /* Save the new vector and unlock the pointer. */
 ATOMIC_WRITE(ptr->ap_data,(uintptr_t)onexit);
 /* Free the old vector. */
 kfree(old_onexit);
 return true;
}

/* Deque a previously queued onexit() callback.
 * NOTE: Unlike `task_queue_onexit()', this function can actually be called
 *       from another onexit() callback for the same thread, allowing that
 *       callback to prevent the execution of callbacks still left to-be
 *       executed. (onexit() callbacks are executed in the same order in
 *       which they were queued originally)
 * NOTE: If the given `callback-arg' pair been registered more than once,
 *       the most recent registration will be deleted.
 * @return: false: Either all instance of `callback(thread,arg)' have already
 *                 been executed, or not a single one was ever queued to begin
 *                 with.
 * @return: true:  Successfully dequeued the most recent onexit-entry matching
 *                 the given `callback' and `arg' parameters. */
PUBLIC ATTR_NOTHROW bool KCALL
__os_task_dequeue_onexit(struct task *__restrict thread,
                         task_onexit_t callback, void *arg,
                         struct driver *__restrict owner) {
 atomic_rwptr_t *ptr;
 size_t size,i;
 struct thread_onexit *onexit;
 ptr = &FORTASK(thread,_this_onexit);
 atomic_rwptr_write(ptr);
 onexit = (struct thread_onexit *)ATOMIC_RWPTR_GET(*ptr);
 i = size = ONEXIT_SIZE(onexit);
 /* Find the newest entry matching the given arguments. */
 while (i--) {
  if (onexit->te_entries[i].oe_func != callback) continue;
  if (onexit->te_entries[i].oe_arg != arg) continue;
  if (onexit->te_entries[i].oe_owner != owner) continue;
  /* Found it! (shift upcoming entries downwards) */
  memmove(&onexit->te_entries[i],
          &onexit->te_entries[i+1],
         ((size-1)-i)*sizeof(struct thread_onexit));
  /* Clear out the last entry that just became free. */
  memset(&onexit->te_entries[size-1],0,
          sizeof(struct thread_onexit));
  atomic_rwptr_endwrite(ptr);
  driver_decref(owner); /* The reference held by `oe_owner' */
  return true;
 }
 atomic_rwptr_endwrite(ptr);
 return false;
}

DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_ONEXIT_C */
