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
#ifndef GUARD_LIBS_LIBC_RWLOCK_H
#define GUARD_LIBS_LIBC_RWLOCK_H 1

#include "libc.h"
#include "sched.h"
#include <kos/types.h>
#include <kos/sched/rwlock.h>
#include <kos/thread.h>
#include <hybrid/timespec.h>
#include <stdbool.h>

#if defined(__i386__) || defined(__x86_64__)
#include <kos/intrin.h>
#include <stddef.h>
#ifdef __ASM_TASK_SEGMENT_ISGS
#define GET_LOCKS()   (struct readlocks *)__readfsptr(offsetof(struct task_segment,ts_locks))
#define SET_LOCKS(v)   __writefsptr(offsetof(struct task_segment,ts_locks),v)
#define GET_CURRENT() (struct task_segment *)__readfsptr(offsetof(struct task_segment,ts_self))
#else
#define GET_LOCKS()  (struct readlocks *)__readgsptr(offsetof(struct task_segment,ts_locks))
#define SET_LOCKS(v)  __writegsptr(offsetof(struct task_segment,ts_locks),v)
#define GET_CURRENT() (struct task_segment *)__readgsptr(offsetof(struct task_segment,ts_self))
#endif
#else
#define GET_LOCKS()   (libc_current()->ts_locks)
#define SET_LOCKS(v)  (libc_current()->ts_locks = (v))
#define GET_CURRENT()  libc_current()
#endif


#ifdef __CC__
DECL_BEGIN

struct readlock {
    uintptr_t          rl_thread; /* [const][== gettid()] The PID of the thread holding this readlock. */
    size_t             rl_count;  /* [lock(PRIVATE(THIS_TASK))] Number of recursive read-locks held by this thread. */
    struct rwlock     *rl_lock;   /* [1..1][valid_if(rl_count != 0)] The R/W-lock associated with this read-lock. */
    struct readlock  **rl_pself;  /* [1..1][valid_if(rl_count != 0 && !rwlock_writing(rl_lock))]
                                   * [lock(rl_lock:RWLOCK_MODE_IRDLOCKBIT)]
                                   * Self-pointer in the chain of read-locks. */
    struct readlock   *rl_next;   /* [0..1][valid_if(rl_count != 0 && !rwlock_writing(rl_lock))]
                                   * [lock(rl_lock:RWLOCK_MODE_IRDLOCKBIT)]
                                   * Next readlock for the same R/W-lock. */
};
struct readlocks {
    size_t             rls_use;   /* Amount of read-locks in use. */
    size_t             rls_cnt;   /* Amount of non-NULL R/W locks. */
    size_t             rls_msk;   /* Allocated hash-mask of the read-hash-vector. */
    struct readlock    rls_vec[]; /* [1..rls_msk+1][owned_if(!= rls_sbuf)]
                                   * Hash-vector of thread-locally held read-locks.
                                   * As hash-index, use `RWLOCK_HASH()' */
};

#define RWLOCK_MODE_IRDLOCKBIT   0x08000000 /* indicates that `rw_locks' is locked. */
#define RWLOCK_MODE_INDMASK      0x07ffffff /* R/W-lock indirection mask. */
#define RWLOCK_MODE_FWAITERS     0x30000000 /* Mask indicative of lock waiters. */
#define RWLOCK_MODE_FRWAITERS    0x10000000 /* Bit indicating that there are threads waiting to acquire a read-lock. */
#define RWLOCK_MODE_FWWAITERS    0x20000000 /* Bit indicating that there is a thread waiting to acquire a write-lock. */
#define RWLOCK_MODE_FMASK        0xc0000000 /* Mode mask. */
#define RWLOCK_MODE_FREADING     0x00000000 /* MODE: ZERO, One or more threads are holding read locks. */
#define RWLOCK_MODE_FUPGRADING   0x40000000 /* MODE: A reader is being upgraded to a writer, or
                                             *       a writer is attempting to acquire a lock.
                                             *       New readers must wait until the rwlock
                                             *       has returned to `RWLOCK_MODE_FREADING'
                                             *       before acquiring a shared lock. */
#define RWLOCK_MODE_FWRITING     0x80000000 /* MODE: The rwlock is being owned exclusively by a single thread. */
#define RWLOCK_STATE(mode,ind)   ((mode) | (ind))
#define RWLOCK_MODE(state)       ((state) & RWLOCK_MODE_FMASK)
#define RWLOCK_IND(state)        ((state) & RWLOCK_MODE_INDMASK)
#define RWLOCK_IND_DIFF            1
#define RWLOCK_INCIND(state)     ((state) + RWLOCK_IND_DIFF)
#define RWLOCK_DECIND(state)     ((state) - RWLOCK_IND_DIFF)

#if RWLOCK_IND_DIFF == 1
#define RWLOCK_ATOMIC_INCIND(self) ATOMIC_FETCHINC((self)->rw_state)
#define RWLOCK_ATOMIC_DECIND(self) ATOMIC_FETCHDEC((self)->rw_state)
#else
#define RWLOCK_ATOMIC_INCIND(self) ATOMIC_FETCHADD((self)->rw_state,RWLOCK_IND_DIFF)
#define RWLOCK_ATOMIC_DECIND(self) ATOMIC_FETCHSUB((self)->rw_state,RWLOCK_IND_DIFF)
#endif


/* Try to lookup a read lock held by the calling thread on `self'
 * If the calling thread isn't holding a read-lock, return `NULL'
 * NOTE: The returned lock never has an `rl_count' value of ZERO(0). */
INTDEF ATTR_NOTHROW struct readlock *LIBCCALL libc_rwlock_getread(rwlock_t *__restrict self);
/* Lookup, or create a new read-lock on `self'.
 * If a new lock is created, the returned lock's `rl_count' field is ZERO(0). */
INTDEF ATTR_RETNONNULL struct readlock *LIBCCALL libc_rwlock_newread(rwlock_t *__restrict self);
/* Same as `libc_rwlock_newread()', but return NULL instead
 * of throwing E_BADALLOC if allocation of a read lock fails. */
INTDEF ATTR_NOTHROW struct readlock *LIBCCALL libc_rwlock_trynewread(rwlock_t *__restrict self);
/* Delete a read-lock that is no longer in use. */
INTDEF ATTR_NOTHROW void LIBCCALL libc_rwlock_delread(struct readlock *__restrict lock);

INTDEF WUNUSED ATTR_NOTHROW bool LIBCCALL libc_rwlock_reading(rwlock_t *__restrict self);
INTDEF WUNUSED ATTR_NOTHROW bool LIBCCALL libc_rwlock_writing(rwlock_t *__restrict self);
INTDEF ATTR_NOTHROW unsigned int LIBCCALL libc_rwlock_poll(rwlock_t *__restrict self, struct pollfutex *__restrict descr, unsigned int how);

INTDEF WUNUSED bool LIBCCALL libc_rwlock_tryread(rwlock_t *__restrict self);
INTDEF WUNUSED ATTR_NOTHROW bool LIBCCALL libc_rwlock_tryread_nothrow(rwlock_t *__restrict self);
INTDEF WUNUSED ATTR_NOTHROW bool LIBCCALL libc_rwlock_trywrite(rwlock_t *__restrict self);
INTDEF WUNUSED ATTR_NOTHROW bool LIBCCALL libc_rwlock_tryupgrade(rwlock_t *__restrict self);
INTDEF WUNUSED ATTR_NOTHROW bool LIBCCALL libc_rwlock_end(rwlock_t *__restrict self);
INTDEF WUNUSED ATTR_NOTHROW bool LIBCCALL libc_rwlock_endread(rwlock_t *__restrict self);
INTDEF ATTR_NOTHROW void LIBCCALL libc_rwlock_endwrite(rwlock_t *__restrict self);
INTDEF void LIBCCALL libc_rwlock_downgrade(rwlock_t *__restrict self);
INTDEF void LIBCCALL libc_rwlock_read(rwlock_t *__restrict self);
INTDEF void LIBCCALL libc_rwlock_write(rwlock_t *__restrict self);
INTDEF void LIBCCALL libc_rwlock_upgrade(rwlock_t *__restrict self);
INTDEF void LIBCCALL libc_rwlock_write_aggressive(rwlock_t *__restrict self);
INTDEF void LIBCCALL libc_rwlock_upgrade_aggressive(rwlock_t *__restrict self);
INTDEF bool LIBCCALL libc_rwlock_timedread(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_rwlock_timedwrite(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_rwlock_timedupgrade(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_rwlock_timedwrite_aggressive(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_rwlock_timedupgrade_aggressive(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_rwlock_timedread64(rwlock_t *__restrict self, struct timespec64 const *abs_timeout);
INTDEF bool LIBCCALL libc_rwlock_timedwrite64(rwlock_t *__restrict self, struct timespec64 const *abs_timeout);
INTDEF bool LIBCCALL libc_rwlock_timedupgrade64(rwlock_t *__restrict self, struct timespec64 const *abs_timeout);
INTDEF bool LIBCCALL libc_rwlock_timedwrite64_aggressive(rwlock_t *__restrict self, struct timespec64 const *abs_timeout);
INTDEF bool LIBCCALL libc_rwlock_timedupgrade64_aggressive(rwlock_t *__restrict self, struct timespec64 const *abs_timeout);

INTDEF WUNUSED bool LIBCCALL libc_rwlock_tryupgrade_s(rwlock_t *__restrict self);
INTDEF WUNUSED bool LIBCCALL libc_rwlock_end_s(rwlock_t *__restrict self);
INTDEF WUNUSED bool LIBCCALL libc_rwlock_endread_s(rwlock_t *__restrict self);
INTDEF void LIBCCALL libc_rwlock_endwrite_s(rwlock_t *__restrict self);
INTDEF void LIBCCALL libc_rwlock_downgrade_s(rwlock_t *__restrict self);
INTDEF void LIBCCALL libc_rwlock_upgrade_s(rwlock_t *__restrict self);
INTDEF void LIBCCALL libc_rwlock_upgrade_aggressive_s(rwlock_t *__restrict self);
INTDEF bool LIBCCALL libc_rwlock_timedupgrade_s(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_rwlock_timedupgrade_aggressive_s(rwlock_t *__restrict self, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_rwlock_timedupgrade64_s(rwlock_t *__restrict self, struct timespec64 const *abs_timeout);
INTDEF bool LIBCCALL libc_rwlock_timedupgrade64_aggressive_s(rwlock_t *__restrict self, struct timespec64 const *abs_timeout);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_RWLOCK_H */
