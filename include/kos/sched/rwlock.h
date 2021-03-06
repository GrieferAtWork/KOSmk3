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
#ifndef _KOS_SCHED_RWLOCK_H
#define _KOS_SCHED_RWLOCK_H 1

#include <__stdinc.h>
#ifdef __KERNEL__
#include <sched/rwlock.h>
#else /* __KERNEL__ */
#include <hybrid/typecore.h>
#include <hybrid/timespec.h>
#include <features.h>
#include <kos/futex.h>
#ifdef __BUILDING_LIBC
#include <hybrid/sync/atomic-rwlock.h>
#endif

__DECL_BEGIN

#ifdef __CC__
#ifdef __BUILDING_LIBC
#define __RWLOCK_SYMBOL(x)     x
#else
#define __RWLOCK_SYMBOL(x) __##x
#endif

typedef struct rwlock rwlock_t;

struct rwlock {
    /* User-space R/W-lock (only takes up 2 pointers!)
     * As far as functionality and semantics go, this R/W-lock behaves the same
     * as its kernel-space equivalent found in `/src/kernel/include/sched/rwlock.h'
     * That means that it uses exceptions to unwind readers, as well as offering a
     * way of manually killing off readers in order to aggressively acquire write locks.
     * All locks operate recursively, meaning that this R/W-lock implementation supports
     * read-after-write, and more importantly: write-after-read, as well as all the
     * usual functions which include upgrade(), downgrade() and try_(read|write|upgrade)().
     * Also, in order to make it easier to expose this structure to untrusted code, *_s
     * versions of context-sensitive functions are provided which will throw an exception
     * when it is detected that they have been called from an invalid context.
     * NOTE:
     *   In order to solve the write-after-read problem, you are _REQUIRED_ to make use
     *   of KOS's exception system. s.a. `rwlock_end()' / `rwlock_endread()'
     * As far as implementation goes, this R/W-lock makes heavy use of the quite
     * useful `FUTEX_WAIT_NMASK_BITSET' futex operation, which allows it to atomically
     * keep track of pending readers/writers (yes: releasing a lock will only invoke
     * a futex_wake() system call if it is know that readers/writers are waiting), as
     * well as easily mask lock state bits, as well as atomically set waiter-chain bits,
     * which then double as futex channel masks.
     * All-in-all, it is a _very_ efficient design that is probably just as efficient
     * as the kernel-space version. */
    union {
        __ATOMIC_DATA futex_t __RWLOCK_SYMBOL(rw_state); /* R/W-lock state & signal. */
        __UINTPTR_TYPE__      __RWLOCK_SYMBOL(rw_align); /* ... */
    };
    union {
#ifdef __BUILDING_LIBC
        struct readlock      *rw_locks;   /* [lock(RWLOCK_MODE_IRDLOCKBIT)]
                                           * Chain of different threads holding read-locks. */
        struct task_segment  *rw_owner;   /* The owner of the R/W-lock. */
#else
        void                 *__rw_locks; /* Internal field */
        void                 *__rw_owner; /* Internal field */
#endif
    };
};


/* Static R/W-lock initializer, and dynamic initializer macros. */
#define RWLOCK_INIT        { { 0 }, { __NULLPTR } }
#define rwlock_init(self)  (void)((self)->__RWLOCK_SYMBOL(rw_state) = 0, \
                                  (self)->__RWLOCK_SYMBOL(rw_locks) = __NULLPTR)
#define rwlock_cinit(self) (void)(assert((self)->__RWLOCK_SYMBOL(rw_state) == 0), \
                                  assert((self)->__RWLOCK_SYMBOL(rw_locks) == __NULLPTR))
#define DEFINE_RWLOCK(name) rwlock_t name = RWLOCK_INIT



__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_tryread,(rwlock_t *__restrict __self),rwlock_tryread,(__self))
__REDIRECT(__LIBC,__WUNUSED __ATTR_NOTHROW,__BOOL,__LIBCCALL,__os_rwlock_tryread_nothrow,(rwlock_t *__restrict __self),rwlock_tryread_nothrow,(__self))
__REDIRECT(__LIBC,__WUNUSED __ATTR_NOTHROW,__BOOL,__LIBCCALL,__os_rwlock_trywrite,(rwlock_t *__restrict __self),rwlock_trywrite,(__self))
__REDIRECT(__LIBC,__WUNUSED __ATTR_NOTHROW,__BOOL,__LIBCCALL,__os_rwlock_tryupgrade,(rwlock_t *__restrict __self),rwlock_tryupgrade,(__self))
__REDIRECT(__LIBC,__WUNUSED __ATTR_NOTHROW,__BOOL,__LIBCCALL,__os_rwlock_end,(rwlock_t *__restrict __self),rwlock_end,(__self))
__REDIRECT(__LIBC,__WUNUSED __ATTR_NOTHROW,__BOOL,__LIBCCALL,__os_rwlock_endread,(rwlock_t *__restrict __self),rwlock_endread,(__self))
__REDIRECT_VOID(__LIBC,__ATTR_NOTHROW,__LIBCCALL,__os_rwlock_endwrite,(rwlock_t *__restrict __self),rwlock_endwrite,(__self))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_rwlock_downgrade,(rwlock_t *__restrict __self),rwlock_downgrade,(__self))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_rwlock_read,(rwlock_t *__restrict __self),rwlock_read,(__self))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_rwlock_write,(rwlock_t *__restrict __self),rwlock_write,(__self))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_rwlock_upgrade,(rwlock_t *__restrict __self),rwlock_upgrade,(__self))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_rwlock_write_aggressive,(rwlock_t *__restrict __self),rwlock_write,(__self))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_rwlock_upgrade_aggressive,(rwlock_t *__restrict __self),rwlock_upgrade_aggressive,(__self))
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedread,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedread64,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedwrite,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedwrite64,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedupgrade,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedupgrade64,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedwrite_aggressive,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedwrite64_aggressive,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedupgrade_aggressive,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedupgrade64_aggressive,(__self,__abs_timeout))
#else /* __USE_TIME_BITS64 */
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedread,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedread,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedwrite,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedwrite,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedupgrade,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedupgrade,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedwrite_aggressive,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedwrite_aggressive,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedupgrade_aggressive,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedupgrade_aggressive,(__self,__abs_timeout))
#endif /* !__USE_TIME_BITS64 */
#ifdef __USE_TIME64
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedread64,(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout),rwlock_timedread64,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedwrite64,(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout),rwlock_timedwrite64,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedupgrade64,(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout),rwlock_timedupgrade64,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedwrite64_aggressive,(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout),rwlock_timedwrite64_aggressive,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedupgrade64_aggressive,(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout),rwlock_timedupgrade64_aggressive,(__self,__abs_timeout))
#endif /* __USE_TIME64 */


/* Check if the calling thread is holding a read/write lock for the given R/W-lock. */
__LIBC __WUNUSED __ATTR_NOTHROW __BOOL (__LIBCCALL rwlock_reading)(rwlock_t *__restrict __self);
__LIBC __WUNUSED __ATTR_NOTHROW __BOOL (__LIBCCALL rwlock_writing)(rwlock_t *__restrict __self);

/* Setup a pollfutex descriptor to wait for the given R/W-lock using `HOW'.
 * @param: HOW: A set of `POLLIN|POLLOUT' (from <bits/poll.h>)
 *              When `POLLIN' is passed, setup `DESCR' to wait for a read-lock
 *              When `POLLOUT' is passed, setup `DESCR' to wait for a write-lock
 *              When `POLLIN' and `POLLOUT' are passed, setup `DESCR' to wait for both
 * @return: * : Set of `mode & (POLLIN|POLLOUT)' describing the locks that are currently
 *              ready. To acquire such a lock, invoke `rwlock_try(read|write)' and re-poll
 *              if that invocation failed. */
__LIBC __ATTR_NOTHROW unsigned int
(__LIBCCALL rwlock_poll)(rwlock_t *__restrict __self,
                         struct pollfutex *__restrict __descr,
                         unsigned int __how);



/* Downgrade a write-lock held by the calling thread into a read-lock.
 * @throw: E_BADALLOC: Failed to allocate a read-lock descriptor. */
__FORCELOCAL void
(__LIBCCALL rwlock_downgrade)(rwlock_t *__restrict __self) {
 __COMPILER_WRITE_BARRIER();
 __os_rwlock_downgrade(__self);
}

/* Release a read- or write-lock held by the calling thread on `SELF'
 * It is assumed that this function is called from some kind of FINALLY-block,
 * in which case a return value of `true' is indicative of the calling thread
 * being required to re-attempt the acquisition of a read-lock, after their
 * thread was chosen to be unwound in order to allow another thread to acquire
 * a write lock, or upgrade a read-lock into a write-lock. (as is the basis
 * of how recursive write-locks are implemented by KOS)
 * >>     struct rwlock *EXCEPT_VAR my_lock = GET_LOCK();
 * >> again:
 * >>     rwlock_read(my_lock);
 * >>     TRY {
 * >>         ...
 * >>         if (need_write_lock)
 * >>             rwlock_upgrade(my_lock);
 * >>         ...
 * >>     } FINALLY {
 * >>         if (rwlock_end(my_lock))
 * >>             goto again;
 * >>     }
 * @return: true:  The lock was released (it was a read-lock by the way), but
 *                 the calling thread must loop back and re-acquire the read-lock
 *                 because another thread needed to get a write-lock, and this
 *                 thread's read-lock stood in the way of it getting it.
 *                 This value isn't returned if the calling thread is holding
 *                 write-locks to the given R/W-lock.
 * @return: false: Successfully released the lock. -> Carry on, sir. */
__FORCELOCAL __WUNUSED __ATTR_NOTHROW __BOOL
(__LIBCCALL rwlock_end)(rwlock_t *__restrict __self) {
 __COMPILER_BARRIER();
 return __os_rwlock_end(__self);
}

/* Release a read-lock held by the calling thread on `SELF'
 * It is assumed that this function is called from some kind of FINALLY-block,
 * in which case a return value of `true' is indicative of the calling thread
 * being required to re-attempt the acquisition of a read-lock, after their
 * thread was chosen to be unwound in order to allow another thread to acquire
 * a write lock, or upgrade a read-lock into a write-lock. (as is the basis
 * of how recursive write-locks are implemented by KOS)
 * >>     struct rwlock *EXCEPT_VAR my_lock = GET_LOCK();
 * >> again:
 * >>     rwlock_read(my_lock);
 * >>     TRY {
 * >>         ...
 * >>     } FINALLY {
 * >>         if (rwlock_endread(my_lock))
 * >>             goto again;
 * >>     }
 * @return: true:  The lock was released, but the calling thread must loop back and re-acquire
 *                 the read-lock because another thread needed to get a write-lock, and this
 *                 thread's read-lock stood in the way of it getting it.
 * @return: false: Successfully released the lock. -> Carry on, sir. */
__FORCELOCAL __WUNUSED __ATTR_NOTHROW __BOOL
(__LIBCCALL rwlock_endread)(rwlock_t *__restrict __self) {
 __COMPILER_READ_BARRIER();
 return __os_rwlock_endread(__self);
}

/* Release a write-lock held by the calling thread on `SELF' */
__FORCELOCAL __ATTR_NOTHROW void
(__LIBCCALL rwlock_endwrite)(rwlock_t *__restrict __self) {
 __COMPILER_BARRIER();
 __os_rwlock_endwrite(__self);
}

/* Try to acquire a read-lock.
 * @throw: E_BADALLOC: Failed to allocate a read-lock descriptor.
 * @return: true:  Successfully acquired a read-lock.
 * @return: false: Failed to acquire a read-lock (the R/W-lock is in write-mode). */
__FORCELOCAL __WUNUSED __BOOL
(__LIBCCALL rwlock_tryread)(rwlock_t *__restrict __self) {
 if (!__os_rwlock_tryread(__self)) return 0;
 __COMPILER_READ_BARRIER();
 return 1;
}

/* Same as `rwlock_tryread()', but return `false' on `E_BADALLOC'.
 * Also note that this function does _NOT_ clobber `error_info()', even
 * when `false' is returned after an exception was thrown internally.
 * Instead, exception information will be restored if that happened.
 * @errno(UNDEFINED): Failed to acquire a read-lock.
 * @errno(ENOMEM):   `rwlock_tryread()' would have thrown an `E_BADALLOC'. */
__FORCELOCAL __WUNUSED __ATTR_NOTHROW __BOOL
(__LIBCCALL rwlock_tryread_nothrow)(rwlock_t *__restrict __self) {
 if (!__os_rwlock_tryread_nothrow(__self)) return 0;
 __COMPILER_READ_BARRIER();
 return 1;
}

/* Try to acquire a write-lock.
 * @return: true:  Successfully acquired a write-lock.
 * @return: false: Failed to acquire a read-lock (There are readers, or another
 *                 thread is writing, or has switched the lock into upgrade-mode). */
__FORCELOCAL __WUNUSED __ATTR_NOTHROW __BOOL
(__LIBCCALL rwlock_trywrite)(rwlock_t *__restrict __self) {
 if (!__os_rwlock_trywrite(__self)) return 0;
 __COMPILER_BARRIER();
 return 1;
}

/* Try to upgrade a read-lock into a write-lock.
 * @return: true:  Successfully upgraded the read-lock into a write-lock.
 * @return: false: There are other threads also holding read-locks. */
__FORCELOCAL __WUNUSED __ATTR_NOTHROW __BOOL
(__LIBCCALL rwlock_tryupgrade)(rwlock_t *__restrict __self) {
 if (!__os_rwlock_tryupgrade(__self)) return 0;
 __COMPILER_WRITE_BARRIER();
 return 1;
}


/* Acquire a read-lock on `SELF' and block until doing so becomes possible.
 * @throw: * : As thrown by a synchronous RPC callback.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL void (__LIBCCALL rwlock_read)(rwlock_t *__restrict __self) {
 __os_rwlock_read(__self);
 __COMPILER_READ_BARRIER();
}

/* Acquire a write-lock on `SELF' and block until doing so becomes possible.
 * @throw: * : As thrown by a synchronous RPC callback.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL void (__LIBCCALL rwlock_write)(rwlock_t *__restrict __self) {
 __COMPILER_BARRIER();
 __os_rwlock_write(__self);
}

/* Upgrade a read-lock held by the calling thread on `SELF' into a write-lock.
 * @throw: * : As thrown by a synchronous RPC callback.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL void (__LIBCCALL rwlock_upgrade)(rwlock_t *__restrict __self) {
 __COMPILER_BARRIER();
 __os_rwlock_upgrade(__self);
}


/* Same as `rwlock_write()', but actively search for threads that are holding
 * read-locks on the given R/W-lock and schedule synchronous RPC callbacks
 * that will throw a targeted `__E_RETRY_RWLOCK' exception to get those threads
 * to release their read-locks and given the calling thread a chance to acquire
 * a write-lock.
 * NOTE: This function should be used sparingly, as excessive use might get
 *       quite expensive.
 *       However it should be noted that this is the only way to acquire
 *       a write-lock when there might be are threads holding read-locks,
 *       which are not intending to drop those locks, but instead are
 *       blocking when waiting on some other kind of lock.
 *       An example of where such functionality is used within the kernel,
 *       are sockets, where a call to shutdown() will aggressively acquire
 *       a write-lock to an internal R/W-lock used to synchronize socket
 *       access, thus interrupting blocking recv() operations that would
 *       normally block indefinitely, which would otherwise cause shutdown()
 *       to block indefinitely, too.
 * @throw: * : As thrown by a synchronous RPC callback.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL void (__LIBCCALL rwlock_write_aggressive)(rwlock_t *__restrict __self) {
 __COMPILER_BARRIER();
 __os_rwlock_write_aggressive(__self);
}

/* Upgrade a read-lock held by the calling thread on `SELF' into a write-lock.
 * For an explanation of what makes this function ~aggressive~, read the
 * documentation of `rwlock_write_aggressive()' above.
 * @throw: * : As thrown by a synchronous RPC callback. */
__FORCELOCAL void (__LIBCCALL rwlock_upgrade_aggressive)(rwlock_t *__restrict __self) {
 __COMPILER_BARRIER();
 __os_rwlock_upgrade_aggressive(__self);
}





/* Acquire a read-lock on `SELF' and block until doing so becomes possible.
 * @param: ABS_TIMEOUT: When `NULL', wait indefinitely, otherwise only wait until this time expires.
 * @return: true:            Successfully acquired a read-lock.
 * @return: false:           The given `ABS_TIMEOUT' has expired.
 * @throw: * :               As thrown by a synchronous RPC callback.
 * @throw: E_BADALLOC:       Failed to allocate a read-lock descriptor.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedread)(rwlock_t *__restrict __self, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_READ_BARRIER();
  __os_rwlock_read(__self);
 } else {
  if (!__os_rwlock_timedread(__self,__abs_timeout)) return 0;
  __COMPILER_READ_BARRIER();
 }
 return 1;
}

/* Acquire a write-lock on `SELF' and block until doing so becomes possible.
 * @param: ABS_TIMEOUT: When `NULL', wait indefinitely, otherwise only wait until this time expires.
 * @return: true:            Successfully acquired a read-lock.
 * @return: false:           The given `ABS_TIMEOUT' has expired.
 * @throw: * :               As thrown by a synchronous RPC callback.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedwrite)(rwlock_t *__restrict __self, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_BARRIER();
  __os_rwlock_write(__self);
 } else {
  if (!__os_rwlock_timedwrite(__self,__abs_timeout)) return 0;
  __COMPILER_BARRIER();
 }
 return 1;
}

/* Upgrade a read-lock held by the calling thread on `SELF' into a write-lock.
 * @param: ABS_TIMEOUT: When `NULL', wait indefinitely, otherwise only wait until this time expires.
 * @return: true:            Successfully acquired a read-lock.
 * @return: false:           The given `ABS_TIMEOUT' has expired.
 * @throw: * :               As thrown by a synchronous RPC callback.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedupgrade)(rwlock_t *__restrict __self, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_BARRIER();
  __os_rwlock_upgrade(__self);
 } else {
  if (!__os_rwlock_timedupgrade(__self,__abs_timeout)) return 0;
  __COMPILER_BARRIER();
 }
 return 1;
}


/* Same as `rwlock_write()', but actively search for threads that are holding
 * read-locks on the given R/W-lock and schedule synchronous RPC callbacks
 * that will throw a targeted `__E_RETRY_RWLOCK' exception to get those threads
 * to release their read-locks and given the calling thread a chance to acquire
 * a write-lock.
 * NOTE: This function should be used sparingly, as excessive use might get
 *       quite expensive.
 *       However it should be noted that this is the only way to acquire
 *       a write-lock when there might be are threads holding read-locks,
 *       which are not intending to drop those locks, but instead are
 *       blocking when waiting on some other kind of lock.
 *       An example of where such functionality is used within the kernel,
 *       are sockets, where a call to shutdown() will aggressively acquire
 *       a write-lock to an internal R/W-lock used to synchronize socket
 *       access, thus interrupting blocking recv() operations that would
 *       normally block indefinitely, which would otherwise cause shutdown()
 *       to block indefinitely, too.
 * @param: ABS_TIMEOUT: When `NULL', wait indefinitely, otherwise only wait until this time expires.
 * @return: true:            Successfully acquired a read-lock.
 * @return: false:           The given `ABS_TIMEOUT' has expired.
 * @throw: * :               As thrown by a synchronous RPC callback.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedwrite_aggressive)(rwlock_t *__restrict __self, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_BARRIER();
  __os_rwlock_write_aggressive(__self);
 } else {
  if (!__os_rwlock_timedwrite_aggressive(__self,__abs_timeout)) return 0;
  __COMPILER_BARRIER();
 }
 return 1;
}

/* Upgrade a read-lock held by the calling thread on `SELF' into a write-lock.
 * For an explanation of what makes this function ~aggressive~, read the
 * documentation of `rwlock_write_aggressive()' above.
 * @param: ABS_TIMEOUT: When `NULL', wait indefinitely, otherwise only wait until this time expires.
 * @return: true:            Successfully acquired a read-lock.
 * @return: false:           The given `ABS_TIMEOUT' has expired.
 * @throw: * :               As thrown by a synchronous RPC callback.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedupgrade_aggressive)(rwlock_t *__restrict __self, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_BARRIER();
  __os_rwlock_upgrade_aggressive(__self);
 } else {
  if (!__os_rwlock_timedupgrade_aggressive(__self,__abs_timeout)) return 0;
  __COMPILER_BARRIER();
 }
 return 1;
}



#ifdef __USE_TIME64
/* Acquire a read-lock on `SELF' and block until doing so becomes possible.
 * @param: ABS_TIMEOUT: When `NULL', wait indefinitely, otherwise only wait until this time expires.
 * @return: true:            Successfully acquired a read-lock.
 * @return: false:           The given `ABS_TIMEOUT' has expired.
 * @throw: * :               As thrown by a synchronous RPC callback.
 * @throw: E_BADALLOC:       Failed to allocate a read-lock descriptor.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedread64)(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_READ_BARRIER();
  __os_rwlock_read(__self);
 } else {
  if (!__os_rwlock_timedread64(__self,__abs_timeout)) return 0;
  __COMPILER_READ_BARRIER();
 }
 return 1;
}

/* Acquire a write-lock on `SELF' and block until doing so becomes possible.
 * @param: ABS_TIMEOUT: When `NULL', wait indefinitely, otherwise only wait until this time expires.
 * @return: true:            Successfully acquired a read-lock.
 * @return: false:           The given `ABS_TIMEOUT' has expired.
 * @throw: * :               As thrown by a synchronous RPC callback.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedwrite64)(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_BARRIER();
  __os_rwlock_write(__self);
 } else {
  if (!__os_rwlock_timedwrite64(__self,__abs_timeout)) return 0;
  __COMPILER_BARRIER();
 }
 return 1;
}

/* Upgrade a read-lock held by the calling thread on `SELF' into a write-lock.
 * @param: ABS_TIMEOUT: When `NULL', wait indefinitely, otherwise only wait until this time expires.
 * @return: true:            Successfully acquired a read-lock.
 * @return: false:           The given `ABS_TIMEOUT' has expired.
 * @throw: * :               As thrown by a synchronous RPC callback.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedupgrade64)(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_BARRIER();
  __os_rwlock_upgrade(__self);
 } else {
  if (!__os_rwlock_timedupgrade64(__self,__abs_timeout)) return 0;
  __COMPILER_BARRIER();
 }
 return 1;
}


/* Same as `rwlock_write()', but actively search for threads that are holding
 * read-locks on the given R/W-lock and schedule synchronous RPC callbacks
 * that will throw a targeted `__E_RETRY_RWLOCK' exception to get those threads
 * to release their read-locks and given the calling thread a chance to acquire
 * a write-lock.
 * NOTE: This function should be used sparingly, as excessive use might get
 *       quite expensive.
 *       However it should be noted that this is the only way to acquire
 *       a write-lock when there might be are threads holding read-locks,
 *       which are not intending to drop those locks, but instead are
 *       blocking when waiting on some other kind of lock.
 *       An example of where such functionality is used within the kernel,
 *       are sockets, where a call to shutdown() will aggressively acquire
 *       a write-lock to an internal R/W-lock used to synchronize socket
 *       access, thus interrupting blocking recv() operations that would
 *       normally block indefinitely, which would otherwise cause shutdown()
 *       to block indefinitely, too.
 * @param: ABS_TIMEOUT: When `NULL', wait indefinitely, otherwise only wait until this time expires.
 * @return: true:            Successfully acquired a read-lock.
 * @return: false:           The given `ABS_TIMEOUT' has expired.
 * @throw: * :               As thrown by a synchronous RPC callback.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedwrite64_aggressive)(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_BARRIER();
  __os_rwlock_write_aggressive(__self);
 } else {
  if (!__os_rwlock_timedwrite64_aggressive(__self,__abs_timeout)) return 0;
  __COMPILER_BARRIER();
 }
 return 1;
}

/* Upgrade a read-lock held by the calling thread on `SELF' into a write-lock.
 * For an explanation of what makes this function ~aggressive~, read the
 * documentation of `rwlock_write_aggressive()' above.
 * @param: ABS_TIMEOUT: When `NULL', wait indefinitely, otherwise only wait until this time expires.
 * @return: true:            Successfully acquired a read-lock.
 * @return: false:           The given `ABS_TIMEOUT' has expired.
 * @throw: * :               As thrown by a synchronous RPC callback.
 * @throw: __E_RETRY_RWLOCK: The calling thread was selected to loose their read-lock. */
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedupgrade64_aggressive)(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_BARRIER();
  __os_rwlock_upgrade_aggressive(__self);
 } else {
  if (!__os_rwlock_timedupgrade64_aggressive(__self,__abs_timeout)) return 0;
  __COMPILER_BARRIER();
 }
 return 1;
}
#endif /* __USE_TIME64 */


__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_tryupgrade_s,(rwlock_t *__restrict __self),rwlock_tryupgrade_s,(__self))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_end_s,(rwlock_t *__restrict __self),rwlock_end_s,(__self))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_endread_s,(rwlock_t *__restrict __self),rwlock_endread_s,(__self))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_rwlock_endwrite_s,(rwlock_t *__restrict __self),rwlock_endwrite_s,(__self))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_rwlock_downgrade_s,(rwlock_t *__restrict __self),rwlock_downgrade_s,(__self))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_rwlock_upgrade_s,(rwlock_t *__restrict __self),rwlock_upgrade_s,(__self))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_rwlock_upgrade_aggressive_s,(rwlock_t *__restrict __self),rwlock_upgrade_aggressive_s,(__self))
#ifdef __USE_TIME_BITS64
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedupgrade_s,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedupgrade64_s,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedupgrade_aggressive_s,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedupgrade64_aggressive_s,(__self,__abs_timeout))
#else /* __USE_TIME_BITS64 */
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedupgrade_s,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedupgrade_s,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedupgrade_aggressive_s,(rwlock_t *__restrict __self, struct timespec const *__abs_timeout),rwlock_timedupgrade_aggressive_s,(__self,__abs_timeout))
#endif /* !__USE_TIME_BITS64 */
#ifdef __USE_TIME64
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedupgrade64_s,(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout),rwlock_timedupgrade64_s,(__self,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_rwlock_timedupgrade64_aggressive_s,(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout),rwlock_timedupgrade64_aggressive_s,(__self,__abs_timeout))
#endif /* __USE_TIME64 */



/* Safe versions of the functions above which all throw an
 * `E_ILLEGAL_OPERATION' exception when invoked from an invalid context:
 *   - Attempting to `rwlock_downgrade_s()' without holding a write-lock
 *   - Attempting to `rwlock_end_s()' without holding any locks
 *   - Attempting to `rwlock_endread_s()' without holding a read-lock
 *   - Attempting to `rwlock_endwrite_s()' without holding a write-lock
 *   - Attempting to `rwlock_(try|timed)upgrade[64]_s()' without holding a read-lock
 * The non-safe versions will cause undefined behavior under these circumstances,
 * or will trigger assertion failures when libc was built in debug mode. */
/* Downgrade a write-lock held by the calling thread into a read-lock. */
__FORCELOCAL void (__LIBCCALL rwlock_downgrade_s)(rwlock_t *__restrict __self) { __COMPILER_WRITE_BARRIER(); __os_rwlock_downgrade_s(__self); }
__FORCELOCAL __WUNUSED __BOOL (__LIBCCALL rwlock_end_s)(rwlock_t *__restrict __self) { __COMPILER_BARRIER(); return __os_rwlock_end_s(__self); }
__FORCELOCAL __WUNUSED __BOOL (__LIBCCALL rwlock_endread_s)(rwlock_t *__restrict __self) { __COMPILER_READ_BARRIER(); return __os_rwlock_endread_s(__self); }
__FORCELOCAL void (__LIBCCALL rwlock_endwrite_s)(rwlock_t *__restrict __self) { __COMPILER_BARRIER(); __os_rwlock_endwrite_s(__self); }
__FORCELOCAL __WUNUSED __BOOL (__LIBCCALL rwlock_tryupgrade_s)(rwlock_t *__restrict __self) { if (!__os_rwlock_tryupgrade_s(__self)) return 0; __COMPILER_WRITE_BARRIER(); return 1; }
__FORCELOCAL void (__LIBCCALL rwlock_upgrade_s)(rwlock_t *__restrict __self) { __COMPILER_BARRIER(); __os_rwlock_upgrade_s(__self); }
__FORCELOCAL void (__LIBCCALL rwlock_upgrade_aggressive_s)(rwlock_t *__restrict __self) { __COMPILER_BARRIER(); __os_rwlock_upgrade_aggressive_s(__self); }
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedupgrade_s)(rwlock_t *__restrict __self, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_BARRIER();
  __os_rwlock_upgrade_s(__self);
 } else {
  if (!__os_rwlock_timedupgrade_s(__self,__abs_timeout)) return 0;
  __COMPILER_BARRIER();
 }
 return 1;
}
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedupgrade_aggressive_s)(rwlock_t *__restrict __self, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_BARRIER();
  __os_rwlock_upgrade_aggressive_s(__self);
 } else {
  if (!__os_rwlock_timedupgrade_aggressive_s(__self,__abs_timeout)) return 0;
  __COMPILER_BARRIER();
 }
 return 1;
}
#ifdef __USE_TIME64
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedupgrade64_s)(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_BARRIER();
  __os_rwlock_upgrade_s(__self);
 } else {
  if (!__os_rwlock_timedupgrade64_s(__self,__abs_timeout)) return 0;
  __COMPILER_BARRIER();
 }
 return 1;
}
__FORCELOCAL __BOOL (__LIBCCALL rwlock_timedupgrade64_aggressive_s)(rwlock_t *__restrict __self, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && !__abs_timeout) {
  __COMPILER_BARRIER();
  __os_rwlock_upgrade_aggressive_s(__self);
 } else {
  if (!__os_rwlock_timedupgrade64_aggressive_s(__self,__abs_timeout)) return 0;
  __COMPILER_BARRIER();
 }
 return 1;
}
#endif /* __USE_TIME64 */



/* Helper macros for safely acquiring / releasing a read-lock.
 * NOTE: Use of these requires inclusion of <except.h> */
#define RWLOCK_BEGIN_READ(self) \
do{ __label__ __rlock_retry; \
    rwlock_t *EXCEPT_VAR __rlock = (self); \
__rlock_retry: \
    rwlock_read(__rlock); \
    TRY
#define RWLOCK_END_READ \
    FINALLY { \
        if (rwlock_endread(__rlock)) \
            goto __rlock_retry; \
    } \
}__WHILE0


#endif /* __CC__ */

__DECL_END

#endif /* !__KERNEL__ */

#endif /* !_KOS_SCHED_RWLOCK_H */
