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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_RWLOCK_H
#define GUARD_KERNEL_INCLUDE_SCHED_RWLOCK_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/atomic.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/sections.h>
#include <fs/iomode.h>
#include <sched/task.h>
#include <sched/signal.h>
#include <endian.h>
#ifndef __INTELLISENSE__
#include <except.h>
#endif

#ifdef __CC__
DECL_BEGIN

struct task;

/* NOTE: The read capabilities of this new R/W-lock primitive are
 *       fairly expensive due to the reason that all threads holding
 *       read-locks must be tracked in order to properly handle
 *       write-after-read recursion.
 *       For that reason, using a `mutex' (which would be equivalent
 *       to an rwlock that is only ever be used in write-lock), might
 *       be the better solution. */

/* Recursive, shared/mutually exclusive synchronization primitive
 * NOTE: This one must be capable of dealing with:
 * >> rwlock_read(x);
 * >> rwlock_read(x);  // Secondary read lock does not count to the total number of reading threads.
 * >> rwlock_write(x); // Upgrade lock by changing to a state that prevents new threads from being added,
 * >>                  // while also ensuring that the calling thread is the only one holding a lock.
 * >> rwlock_write(x);
 * >> rwlock_read(x);  // Once already holding a write-lock, this is the same as calling `rwlock_write()'
 * >> // .. Same as holding a single `rwlock_write(x)'
 * >> rwlock_endread(x);
 * >> rwlock_endwrite(x);
 * >> rwlock_endwrite(x);
 * >> rwlock_endread(x);
 * >> rwlock_endread(x);
 */


/* The `parallel-upgrade' problem:
 *   -- TODO
 */


#define RWLOCK_MODE_FREADING   0x00 /* MODE: ZERO, One or more threads are holding read locks. */
#define RWLOCK_MODE_FUPGRADING 0x01 /* MODE: A reader is being upgraded to a writer, or
                                     *        a writer is attempting to acquire a lock.
                                     *        New readers must wait until the rwlock
                                     *        has returned to `RWLOCK_MODE_FREADING'
                                     *        before acquiring a shared lock. */
#define RWLOCK_MODE_FWRITING   0x02 /* MODE: The rwlock is being owned
                                     *        exclusively by a single thread. */


#define RWLOCK_FNORMAL 0x00 /* Normal flags. */


typedef struct rwlock rwlock_t;

#define __RWLOCK_STATE(mode,ind) ((ind) << 16 | (mode))
#define __RWLOCK_MODE(state)     ((state) & 0xff)
#define __RWLOCK_IND(state)      ((state) >> 16)
#define __RWLOCK_INCIND(state)   ((state) + (1 << 16))
#define __RWLOCK_DECIND(state)   ((state) - (1 << 16))


struct rwlock {
    union PACKED {
        ATOMIC_DATA u32            rw_state;       /* R/W-lock state. */
#if __SIZEOF_POINTER__ > 4
        uintptr_t                  rw_pad;         /* Align by pointers. */
#endif
        struct PACKED {
#if __BYTE_ORDER == __LITTLE_ENDIAN
            ATOMIC_DATA u8         rw_mode;        /* R/W-lock mode (one of `RWLOCK_MODE_F*') */
            u8                     rw_flags;       /* R/W-lock flags (set of `RWLOCK_F*') */
            union PACKED {
                u16                rw_xind;        /* [valid_if(rw_mode == RWLOCK_MODE_FWRITING)]
                                                    * [lock(rw_xowner == THIS_TASK)]
                                                    *  Indirection/Recursion of the exclusive lock.
                                                    */
                ATOMIC_DATA u16    rw_scnt;        /* [valid_if(rw_mode == RWLOCK_MODE_FREADING)]
                                                    *  The total number of individual reader threads.
                                                    *  NOTE: Read-recursion is tracked internally
                                                    *        using a per-thread hash-vector of R/W-lock
                                                    *        structures to which that thread is currently
                                                    *        holding a READ/WRITE lock. */
            };
#elif __BYTE_ORDER == __BIG_ENDIAN
            union PACKED {
                u16                rw_xind;        /* [valid_if(rw_mode == RWLOCK_MODE_FWRITING)]
                                                    * [lock(rw_xowner == THIS_TASK)]
                                                    *  Indirection/Recursion of the exclusive lock.
                                                    */
                ATOMIC_DATA u16    rw_scnt;        /* [valid_if(rw_mode == RWLOCK_MODE_FREADING)]
                                                    *  The total number of individual reader threads.
                                                    *  NOTE: Read-recursion is tracked internally
                                                    *        using a per-thread hash-vector of R/W-lock
                                                    *        structures to which that thread is currently
                                                    *        holding a READ/WRITE lock. */
            };
            u8                     rw_flags;       /* R/W-lock flags (set of `RWLOCK_F*') */
            ATOMIC_DATA u8         rw_mode;        /* R/W-lock mode (one of `RWLOCK_MODE_F*') */
#endif
        };
    };
    struct sig                     rw_chmode;      /* Signal boardcast when `rw_mode' is downgraded (write -> read / write -> none).
                                                    * (Usually connected to threads calling `rwlock_read' or `rwlock_write') */
    struct sig                     rw_unshare;     /* Signal sent to a single thread when `rw_scnt' recursion drops to ZERO(0).
                                                    * (Usually connected to the thread that is calling `rwlock_upgrade()') */
    struct task                   *rw_xowner;      /* [valid_if(rw_mode == RWLOCK_MODE_FWRITING)]
                                                    * [lock(rw_xowner == THIS_TASK)]
                                                    *  Exclusive owner of this R/W lock. */
};

#define DEFINE_RWLOCK(name) rwlock_t name = RWLOCK_INIT
#define RWLOCK_INIT    {{0},SIG_INIT,SIG_INIT,NULL}
#define rwlock_init(x) \
   (void)((x)->rw_mode = RWLOCK_MODE_FREADING, \
          (x)->rw_flags = RWLOCK_FNORMAL, \
           sig_init(&(x)->rw_chmode), \
           sig_init(&(x)->rw_unshare), \
          (x)->rw_xind = 0)
#define rwlock_cinit(x) \
   (void)(assert((x)->rw_mode == RWLOCK_MODE_FREADING), \
          assert((x)->rw_flags == RWLOCK_FNORMAL), \
          sig_cinit(&(x)->rw_chmode), \
          sig_cinit(&(x)->rw_unshare), \
          assert((x)->rw_xind == 0))

FUNDEF ATTR_NOTHROW bool KCALL rwlock_reading(struct rwlock *__restrict self);
#ifdef __INTELLISENSE__
FUNDEF ATTR_NOTHROW bool KCALL rwlock_writing(struct rwlock *__restrict self);
#else
#define rwlock_writing(self) \
    ((self)->rw_mode == RWLOCK_MODE_FWRITING && \
     (self)->rw_xowner == THIS_TASK)
#endif


/* Try to acquire a shared/exclusive lock, or try to
 * atomically (non-locking) upgrade a shared to an exclusive lock.
 * @throw: E_BADALLOC: [rwlock_tryread] Failed to allocate a read-descriptor. */
FORCELOCAL bool KCALL rwlock_tryread(struct rwlock *__restrict self);
FORCELOCAL ATTR_NOTHROW bool KCALL rwlock_trywrite(struct rwlock *__restrict self);
FORCELOCAL ATTR_NOTHROW bool KCALL rwlock_tryupgrade(struct rwlock *__restrict self);

#ifndef __INTELLISENSE__
FUNDEF bool KCALL __os_rwlock_tryread(struct rwlock *__restrict self) ASMNAME("rwlock_tryread");
FUNDEF ATTR_NOTHROW bool KCALL __os_rwlock_trywrite(struct rwlock *__restrict self) ASMNAME("rwlock_trywrite");
FUNDEF bool KCALL __os_rwlock_tryupgrade(struct rwlock *__restrict self) ASMNAME("rwlock_tryupgrade");
FORCELOCAL bool KCALL
rwlock_tryread(struct rwlock *__restrict self) {
 if (!__os_rwlock_tryread(self))
      return false;
 COMPILER_READ_BARRIER();
 return true;
}
FORCELOCAL ATTR_NOTHROW bool KCALL
rwlock_trywrite(struct rwlock *__restrict self) {
 if (!__os_rwlock_trywrite(self))
      return false;
 COMPILER_BARRIER();
 return true;
}
FORCELOCAL ATTR_NOTHROW bool KCALL
rwlock_tryupgrade(struct rwlock *__restrict self) {
 if (!__os_rwlock_tryupgrade(self))
      return false;
 COMPILER_WRITE_BARRIER();
 return true;
}
#endif /* !__INTELLISENSE__ */


/* Blocking-acquire a shared/exclusive lock.
 * NOTE: These functions clobber task connections.
 * @throw: * :             [abs_timeout != JTIME_DONTWAIT] An exception was thrown by an RPC callback.
 * @throw: E_INTERRUPT:    [abs_timeout != JTIME_DONTWAIT] The calling thread was interrupted.
 * @throw: E_BADALLOC:     [rwlock_timedread] Failed to allocate a read-descriptor.
 * @throw: E_RETRY_RWLOCK: [rwlock_timedwrite | rwlock_write]
 *                          The parallel-upgrade deadlock was detected and the calling
 *                          thread was chosen to unwind their stack, calling
 *                         `rwlock_endread()' or `rwlock_end()' until one of the
 *                          two functions returns `true', in which case said function
 *                          should jump back to the code that was originally used
 *                          to acquire a read-lock further up the execution tree.
 *                          See the section on `parallel-upgrade' above.
 * @return: true:  The operation was successful.
 * @return: false: The given timeout has expired. */
FORCELOCAL bool KCALL rwlock_timedread(struct rwlock *__restrict self, jtime_t abs_timeout);
FORCELOCAL bool KCALL rwlock_timedwrite(struct rwlock *__restrict self, jtime_t abs_timeout);
#ifdef __INTELLISENSE__
void KCALL rwlock_read(struct rwlock *__restrict self);
void KCALL rwlock_write(struct rwlock *__restrict self);
void KCALL rwlock_readf(struct rwlock *__restrict self, iomode_t flags);
void KCALL rwlock_writef(struct rwlock *__restrict self, iomode_t flags);
#else
#define rwlock_read(self)       (void)rwlock_timedread(self,JTIME_INFINITE)
#define rwlock_write(self)      (void)rwlock_timedwrite(self,JTIME_INFINITE)
LOCAL void KCALL rwlock_readf(struct rwlock *__restrict self, iomode_t flags);
LOCAL void KCALL rwlock_writef(struct rwlock *__restrict self, iomode_t flags);

FUNDEF bool KCALL
__os_rwlock_timedread(struct rwlock *__restrict self,
                      jtime_t abs_timeout)
                      ASMNAME("rwlock_timedread");
FORCELOCAL bool KCALL
rwlock_timedread(struct rwlock *__restrict self,
                 jtime_t abs_timeout) {
 if (!__os_rwlock_timedread(self,abs_timeout))
      return false;
 COMPILER_READ_BARRIER();
 return true;
}
FUNDEF bool KCALL
__os_rwlock_timedwrite(struct rwlock *__restrict self,
                       jtime_t abs_timeout)
                       ASMNAME("rwlock_timedwrite");
FORCELOCAL bool KCALL
rwlock_timedwrite(struct rwlock *__restrict self,
                  jtime_t abs_timeout) {
 if (!__os_rwlock_timedwrite(self,abs_timeout))
      return false;
 COMPILER_BARRIER();
 return true;
}
#endif /* !__INTELLISENSE__ */

/* An advanced variant of `rwlock_write()' which makes use of
 * the RETRY_RWLOCK mechanism to quickly acquire a write lock.
 * The difference between this and the regular write, is that
 * this function will explicitly go and hunt down threads that
 * are holding read-locks on `self', scheduling RPC callbacks
 * in their context and throwing `E_RETRY_RWLOCK' with `self'
 * as associated r/w-lock.
 * This way, the calling thread is guarantied to be able to
 * eventually acquire a write-lock, even if there were other
 * threads holding read locks, which were also waiting for
 * some other kind of lock.
 * The `abs_timeout' only comes into effect when the r/w-lock
 * was already in write-mode, but the owner was some thread
 * other than the calling thread. In this case, a regular wait
 * will still be performed, and the function behaves just like
 * the regular `rwlock_timedwrite()'.
 * @throw: * :            [abs_timeout != JTIME_DONTWAIT] An exception was thrown by an RPC callback.
 * @throw: E_INTERRUPT:   [abs_timeout != JTIME_DONTWAIT] The calling thread was interrupted.
 * @throw: E_BADALLOC:     Failed to allocate an RPC callback for one of the reader threads.
 * @throw: E_RETRY_RWLOCK: The parallel-upgrade deadlock was detected and the calling
 *                         thread was chosen to unwind their stack, calling
 *                        `rwlock_endread()' or `rwlock_end()' until one of the
 *                         two functions returns `true', in which case said function
 *                         should jump back to the code that was originally used
 *                         to acquire a read-lock further up the execution tree.
 *                         See the section on `parallel-upgrade' above.
 * @return: true:  Successfully acquire a write lock.
 * @return: false: The given timeout has expired. */
FORCELOCAL bool KCALL rwlock_timedwrite_agressive(struct rwlock *__restrict self, jtime_t abs_timeout);
#ifdef __INTELLISENSE__
void KCALL rwlock_write_agressive(struct rwlock *__restrict self);
#else
#define rwlock_write_agressive(self) (void)rwlock_timedwrite_agressive(self,JTIME_INFINITE)

FUNDEF bool KCALL
__os_rwlock_timedwrite_agressive(struct rwlock *__restrict self,
                                 jtime_t abs_timeout)
                                 ASMNAME("rwlock_timedwrite_agressive");
FORCELOCAL bool KCALL
rwlock_timedwrite_agressive(struct rwlock *__restrict self,
                            jtime_t abs_timeout) {
 if (!__os_rwlock_timedwrite_agressive(self,abs_timeout))
      return false;
 COMPILER_BARRIER();
 return true;
}
#endif /* !__INTELLISENSE__ */


/* Update a read-lock to an exclusive write-lock.
 * @return: false:         The given timeout has expired. (a read-lock is still being held)
 * @return: true:          Successfully upgraded the lock. (you're now holding a write-lock)
 * @throw: E_RETRY_RWLOCK: The parallel-upgrade deadlock was detected and the calling
 *                         thread was chosen to unwind their stack, calling
 *                        `rwlock_endread()' or `rwlock_end()' until one of the
 *                         two functions returns `true', in which case said function
 *                         should jump back to the code that was originally used
 *                         to acquire a read-lock further up the execution tree.
 *                         See the section on `parallel-upgrade' above.
 *                        (a read-lock is still being held) */
FORCELOCAL bool KCALL rwlock_timedupgrade(struct rwlock *__restrict self, jtime_t abs_timeout);
#ifdef __INTELLISENSE__
FUNDEF bool KCALL rwlock_upgrade(struct rwlock *__restrict self);
#else
#define rwlock_upgrade(self)    (void)rwlock_timedupgrade(self,JTIME_INFINITE)
#endif

#ifndef __INTELLISENSE__
FUNDEF bool KCALL
__os_rwlock_timedupgrade(struct rwlock *__restrict self,
                         jtime_t abs_timeout) ASMNAME("rwlock_timedupgrade");
FORCELOCAL bool KCALL
rwlock_timedupgrade(struct rwlock *__restrict self,
                    jtime_t abs_timeout) {
 if (!__os_rwlock_timedupgrade(self,abs_timeout))
      return false;
 COMPILER_WRITE_BARRIER();
 return true;
}
#endif /* !__INTELLISENSE__ */

/* Downgrade a write-lock to a read-lock.
 * NOTE: When multiple write-locks are being held recursively,
 *       this function does nothing and previous, recursive
 *       write-locks will remain active and keep the caller
 *       the lock's exclusive write-owner.
 * @throw: E_BADALLOC: [rwlock_timedread] Failed to allocate a read-descriptor. */
FORCELOCAL void KCALL rwlock_downgrade(struct rwlock *__restrict self);

#ifndef __INTELLISENSE__
FUNDEF void KCALL
__os_rwlock_downgrade(struct rwlock *__restrict self) ASMNAME("rwlock_downgrade");
FORCELOCAL void KCALL
rwlock_downgrade(struct rwlock *__restrict self) {
 COMPILER_WRITE_BARRIER();
 __os_rwlock_downgrade(self);
}
#endif /* !__INTELLISENSE__ */

/* Release a shared/exclusive lock, or automatically the kind of lock last acquired.
 * NOTE: It is preferred that `rwlock_endwrite' or `rwlock_endread' are used
 *       to release locks. Not only because they are faster, but also because
 *       they allow for better detection of invalid uses, when it is known
 *       at runtime if the end of a read, or a write-lock is intended.
 * @return: false: The lock was released.
 * @return: true:  After another thread needed to upgrade `self', the calling
 *                 thread's own attempt to upgrade the same lock was halted
 *                 by throwing an `E_RETRY_RWLOCK' exception.
 *                 This is usually returned when called from a FINALLY-block.
 *           NOTE: See the section on `parallel-upgrade' on how to deal with this. */
FORCELOCAL ATTR_NOTHROW void KCALL rwlock_endwrite(struct rwlock *__restrict self);
FORCELOCAL ATTR_NOTHROW WUNUSED bool KCALL rwlock_endread(struct rwlock *__restrict self);
FORCELOCAL ATTR_NOTHROW WUNUSED bool KCALL rwlock_end(struct rwlock *__restrict self);

#ifndef __INTELLISENSE__
FUNDEF ATTR_NOTHROW void KCALL
__os_rwlock_endwrite(struct rwlock *__restrict self) ASMNAME("rwlock_endwrite");
FORCELOCAL ATTR_NOTHROW void KCALL
rwlock_endwrite(struct rwlock *__restrict self) {
 COMPILER_BARRIER();
 __os_rwlock_endwrite(self);
}

FUNDEF ATTR_NOTHROW WUNUSED bool KCALL
__os_rwlock_endread(struct rwlock *__restrict self) ASMNAME("rwlock_endread");
FORCELOCAL ATTR_NOTHROW WUNUSED bool KCALL
rwlock_endread(struct rwlock *__restrict self) {
 COMPILER_READ_BARRIER();
 return __os_rwlock_endread(self);
}

FUNDEF ATTR_NOTHROW WUNUSED bool KCALL
__os_rwlock_end(struct rwlock *__restrict self) ASMNAME("rwlock_end");
FORCELOCAL ATTR_NOTHROW WUNUSED bool KCALL
rwlock_end(struct rwlock *__restrict self) {
 COMPILER_BARRIER();
 return __os_rwlock_end(self);
}
#endif /* !__INTELLISENSE__ */

/* Asynchronously connect to the given R/W-lock for reading or writing:
 *   - Specify `POLLIN' to poll for a read-lock (`rwlock_tryread()')
 *   - Specify `POLLOUT' to poll for a write-lock (`rwlock_trywrite()')
 * @param: mode: Set of `POLLIN|POLLOUT' from <bits/poll.h>
 * @return: * :  Set of `POLLIN|POLLOUT'
 * @throw: E_WOULDBLOCK: `mode' contains neither `POLLIN', nor `POLLOUT' */
FUNDEF unsigned int KCALL rwlock_poll(struct rwlock *__restrict self, unsigned int mode);


#ifndef __INTELLISENSE__
LOCAL void KCALL
rwlock_readf(struct rwlock *__restrict self, iomode_t flags) {
 if (!rwlock_tryread(self)) {
  if (flags & IO_NONBLOCK)
      error_throw(E_WOULDBLOCK);
  rwlock_read(self);
 }
}
LOCAL void KCALL
rwlock_writef(struct rwlock *__restrict self, iomode_t flags) {
 if (!rwlock_trywrite(self)) {
  if (flags & IO_NONBLOCK)
      error_throw(E_WOULDBLOCK);
  rwlock_write(self);
 }
}

#endif

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_RWLOCK_H */
