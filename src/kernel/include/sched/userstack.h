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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_USERSTACK_H
#define GUARD_KERNEL_INCLUDE_SCHED_USERSTACK_H 1

#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kos/types.h>
#include <hybrid/atomic.h>
#include <kernel/paging.h>
#include <kernel/sections.h>

DECL_BEGIN

#ifdef __CC__
/* Automatic, kernel-managed stack allocation for user-space. */
struct userstack {
    ATOMIC_DATA ref_t  us_refcnt;   /* Reference counter (used for stack notifications) */
    vm_vpage_t         us_pagemin;  /* [lock(:struct vm::vm_lock)] Lowest stack page. */
    vm_vpage_t         us_pageend;  /* [lock(:struct vm::vm_lock)] Stack end address. */
};

/* Destroy a previously allocated userstack. */
FUNDEF ATTR_NOTHROW void KCALL userstack_destroy(struct userstack *__restrict self);

/* Increment/decrement the reference counter of the given userstack `x' */
#define userstack_incref(x)  ATOMIC_FETCHINC((x)->us_refcnt)
#define userstack_decref(x) (ATOMIC_DECFETCH((x)->us_refcnt) || (userstack_destroy(x),0))

/* [0..1][owned][lock(PRIVATE(THIS_TASK,WRITE_ONCE))]
 * The user-space stack descriptor of the calling thread.
 * This pointer is lazily allocated by `task_alloc_userstack()'
 * Finally, this stack will be unmapped in `task_exit()'. */
DATDEF ATTR_PERTASK REF struct userstack *_this_user_stack;

/* Allocate an automatically-sized user-space stack for the calling thread.
 * This stack is lazily allocated and mapped, meaning that this function
 * will return the same object after being called a second time. */
FUNDEF ATTR_RETNONNULL ATTR_CONST struct userstack *KCALL task_alloc_userstack(void);

/* VM Notification callback to user-stacks. */
FUNDEF void *KCALL
userstack_notify(void *closure, unsigned int code,
                 vm_vpage_t addr, size_t num_pages,
                 void *arg);


#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_USERSTACK_H */
