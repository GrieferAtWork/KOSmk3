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
#ifndef GUARD_KERNEL_SRC_SCHED_TASK_WEAKREF_C
#define GUARD_KERNEL_SRC_SCHED_TASK_WEAKREF_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <kos/types.h>
#include <kernel/sections.h>
#include <kernel/malloc.h>
#include <sched/task.h>
#include <sched/taskref.h>
#include <except.h>

DECL_BEGIN

/* Destroy a previously allocated task_weakref. */
PUBLIC ATTR_NOTHROW void KCALL
task_weakref_destroy(struct task_weakref *__restrict self) {
 kfree(self);
}


/* [0..1][lock(WRITE_ONCE)] Per-task weak reference descriptor. */
PRIVATE ATTR_PERTASK REF struct task_weakref *my_weakref = NULL;

DEFINE_PERTASK_FINI(task_weakref_fini);
INTERN void KCALL task_weakref_fini(struct task *__restrict thread) {
 REF struct task_weakref *ref;
 ref = FORTASK(thread,my_weakref);
 if (ref) {
  /* Clear the weak reference pointer
   * and drop the reference itself. */
  atomic_rwlock_write(&ref->tw_lock);
  assert(ref->tw_task == thread);
  ref->tw_task = NULL;
  atomic_rwlock_endwrite(&ref->tw_lock);
  task_weakref_decref(ref);
 }
}

/* Lazily allocates and returns the weak reference counter for `self'.
 * @throw: E_BADALLOC: Failed to lazily allocate the descriptor. */
PUBLIC ATTR_RETNONNULL REF struct task_weakref *
KCALL task_getweakref(struct task *__restrict self) {
 REF struct task_weakref *result,*new_result;
 assertf(self->t_refcnt != 0,
         "Cannot get weak reference descriptor "
         "of task that is being destroyed");
 result = FORTASK(self,my_weakref);
 if (result) {
incref_result:
  task_weakref_incref(result);
 } else {
  /* Construct a new weakref descriptor. */
  result = (REF struct task_weakref *)kmalloc(sizeof(struct task_weakref),
                                              GFP_SHARED/*|GFP_LOCKED*/);
  atomic_rwlock_init(&result->tw_lock);
  result->tw_refcnt = 2; /* +1: return; +1: my_weakref */
  result->tw_task   = self;
  /* NOTE: Upon success, this inherits one reference to `result' */
  new_result = ATOMIC_CMPXCH_VAL(FORTASK(self,my_weakref),
                                 NULL,result);
  if unlikely(new_result) {
   /* Race condition: Another thread allocate
    * the descriptor while we did the same. */
   kfree(result);
   result = new_result;
   goto incref_result;
  }
 }
 return result;
}

/* Lock a weak task reference, attempting to construct a new reference to the pointed-to task.
 * If this is not possible because the task has been destroyed, NULL is returned. */
PUBLIC REF struct task *KCALL
task_weakref_lock(struct task_weakref *__restrict self) {
 REF struct task *result;
 atomic_rwlock_read(&self->tw_lock);
 result = self->tw_task;
 /* Try to acquire a reference to the task
  * if the counter isn't already ZERO(0). */
 if (result && !ATOMIC_INCIFNONZERO(result->t_refcnt))
     result = NULL;
 atomic_rwlock_endread(&self->tw_lock);
 return result;
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_TASK_WEAKREF_C */
