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
#ifndef GUARD_KERNEL_SRC_SCHED_RWLOCK_H
#define GUARD_KERNEL_SRC_SCHED_RWLOCK_H 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/sections.h>
#include <kernel/heap.h>

DECL_BEGIN


/* A dummy R/W lock pointer. */
#define READLOCK_DUMMYLOCK  ((struct rwlock *)-1)

struct read_lock {
    uintptr_t      rl_recursion; /* [valid_if(rl_rwlock != NULL && rl_rwlock != READLOCK_DUMMYLOCK)]
                                  *  Amount of recursive read-locks held. */
    struct rwlock *rl_rwlock;    /* [valid_if(!= NULL && != READLOCK_DUMMYLOCK)] Associated R/W-lock. */
};
#define CONFIG_TASK_STATIC_READLOCKS 4


#define RWLOCK_HASH(x) ((uintptr_t)(x) / HEAP_ALIGNMENT)
struct read_locks {
    struct read_lock  rls_sbuf[CONFIG_TASK_STATIC_READLOCKS];
    size_t            rls_use; /* Amount of read-locks in use. */
    size_t            rls_cnt; /* Amount of non-NULL R/W locks. */
    size_t            rls_msk; /* Allocated hash-mask of the read-hash-vector. */
    struct read_lock *rls_vec; /* [1..rls_msk+1][owned_if(!= rls_sbuf)]
                                * Hash-vector of thread-locally held read-locks.
                                * As hash-index, use `RWLOCK_HASH()' */
};

INTDEF ATTR_PERTASK struct read_locks my_readlocks;

/* Find an existing read-lock descriptor for `lock', or return NULL. */
INTDEF ATTR_NOTHROW struct read_lock *KCALL
rwlock_find_readlock(struct rwlock *__restrict lock);

/* Return a read-lock descriptor for `lock', or allocate a new one. */
INTDEF ATTR_RETNONNULL struct read_lock *KCALL
rwlock_get_readlock(struct rwlock *__restrict lock);

/* Return a read-lock descriptor for `lock', or allocate a new one. */
INTDEF struct read_lock *KCALL
rwlock_get_readlock_nothrow(struct rwlock *__restrict lock);

/* Delete the given rlock. */
INTDEF void KCALL rwlock_delete_readlock(struct read_lock *__restrict rlock);

DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_RWLOCK_H */
