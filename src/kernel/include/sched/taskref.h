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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_TASKREF_H
#define GUARD_KERNEL_INCLUDE_SCHED_TASKREF_H 1

#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kos/types.h>
#include <hybrid/atomic.h>

DECL_BEGIN

#ifdef __CC__
struct task;

/* Task weak referencing API. */
struct task_weakref {
    /* Task weak reference.
     * Every task has a private field implementing a [0..1][lock(WRITE_ONCE)]
     * pointer to a `struct task_weakref' structure that is allocated upon
     * first access through `task_getweakref()'.
     * When a task's reference counter hits ZERO, this structure will be
     * update to point to `NULL' for `tw_task'.
     * The reason why task weak referencing isn't implemented as a second
     * reference counter, but rather using this clutch is quite simple.
     * Tasks are _huge_ (even without posix-signals up to 1K, and with them around 4K)
     * However, an alloc-holding weak reference would keep the task from
     * being deallocated, even when its reference counter hits ZERO(0).
     * For that reason, you should instead keep a reference to this minimal
     * structure, which you can then use for attempting to acquire a reference
     * on the task itself.
     * Basically, this is a secondary reference counter that won't prevent
     * the underlying task from being destroyed, but rather gets informed
     * when that happens.
     * WARNING: Even when `tw_task' is non-NULL, the reference counter
     *          of the associated task may already be ZERO(0) in case
     *          the task is currently trying to acquire a lock to `tw_lock',
     *          or is otherwise in the process of cleanup up prior to
     *          overwriting the weak reference pointer. */
    ATOMIC_DATA ref_t            tw_refcnt;   /* Reference counter for this weak reference. */
    atomic_rwlock_t              tw_lock;     /* Lock for accessing `tw_task' */
    WEAK struct task            *tw_task;     /* [0..1][lock(tw_lock)] A weak pointer to a task. */
};

/* Destroy a previously allocated task_weakref. */
FUNDEF ATTR_NOTHROW void KCALL task_weakref_destroy(struct task_weakref *__restrict self);

/* Increment/decrement the reference counter of the given task_weakref `x' */
#define task_weakref_incref(x)  ATOMIC_FETCHINC((x)->tw_refcnt)
#define task_weakref_decref(x) (ATOMIC_DECFETCH((x)->tw_refcnt) || (task_weakref_destroy(x),0))


/* Lazily allocates and returns the weak reference counter for `self'.
 * WARNING: The weak reference descriptor is allocated without `GFP_LOCKED',
 *          meaning that it must not be used by code that may be called
 *          from a pagefault handler.
 * @throw: E_BADALLOC: Failed to lazily allocate the descriptor. */
FUNDEF ATTR_RETNONNULL REF struct task_weakref *
KCALL task_getweakref(struct task *__restrict self);

/* Lock a weak task reference, attempting to construct a new reference to the pointed-to task.
 * If this is not possible because the task has been destroyed, NULL is returned. */
FUNDEF REF struct task *KCALL
task_weakref_lock(struct task_weakref *__restrict self);

/* Weakly read the task pointer of the given task weakref.
 * The returned pointer may only be used for comparison, but not be dereferenced.
 * Additionally, no guaranty can be made that the task pointed to by the returned
 * value still exists by the time this function (macro) returns. */
#define task_weakref_weak(self) \
    ATOMIC_READ((self)->tw_task)

#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_TASKREF_H */
