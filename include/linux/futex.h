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
#ifndef _LINUX_FUTEX_H
#define _LINUX_FUTEX_H 1

#include <__stdinc.h>
#include <features.h>

/* DISCLAIMER: Mostly taken from /usr/include/linux/futex.h */

__SYSDECL_BEGIN

/* Second argument to futex syscall */
#define FUTEX_WAIT            0x00 /* >> while (*uaddr == val)
                                    * >>      WAIT(uaddr, mask = FUTEX_BITSET_MATCH_ANY, utime); */
#define FUTEX_WAKE            0x01 /* >> WAKE_MAX(uaddr, mask = FUTEX_BITSET_MATCH_ANY, max_threads = val); */
#define FUTEX_FD              0x02 /* Operates similarly to `FUTEX_WAIT_FD', and was originally implemented by
                                    * linux but later removed due to the fact that this one's _COMPLETELY_ _USELESS_
                                    * >> There is no interlocked check done when polling this one, meaning
                                    *    that waiting for a signal here is the definition of raciness. */
#define FUTEX_REQUEUE         0x03
#define FUTEX_CMP_REQUEUE     0x04
#define FUTEX_WAKE_OP         0x05
#define FUTEX_LOCK_PI         0x06 /* >> if ((*uaddr & FUTEX_TID_MASK) == 0) {
                                    * >>     *uaddr = (*uaddr & ~FUTEX_TID_MASK) | gettid();
                                    * >> } else {
                                    * >>     *uaddr |= FUTEX_WAITERS;
                                    * >>     WAIT(uaddr, mask = FUTEX_BITSET_MATCH_ANY, utime);
                                    * >> }
                                    */
#define FUTEX_UNLOCK_PI       0x07
#define FUTEX_TRYLOCK_PI      0x08
#define FUTEX_WAIT_BITSET     0x09 /* >> while (*uaddr == val)
                                    * >>      WAIT(uaddr, mask = val3, utime); */
#define FUTEX_WAKE_BITSET     0x0a /* >> WAKE_MAX(uaddr, mask = val3, max_threads = val); */
#define FUTEX_WAIT_REQUEUE_PI 0x0b
#define FUTEX_CMP_REQUEUE_PI  0x0c


#ifdef __KOS__
/* Robust waiter functions that ensure that all bits from
 * `val3' are set while atomically entering a wait-state.
 * These are useful for implementing a single-futex semaphore object
 * which can make use of some flag in order to determine if the futex
 * is currently available.
 * WARNING: If bits set by `val3' are included in the `val' mask,
 *          the thread _might_ (but might not necessarily) still begin
 *          sleeping immediately, despite the fact that its wait
 *          condition has already been fulfilled.
 *          In this case, the futex() system call may return for any
 *          number of reasons, and at any undefined point in time
 *          besides the reason of the futex getting triggered, which
 *          still guaranties to wake the thread if it was sleeping.
 *          With that in mind, the point in time when such a thread
 *          will be woken is any undefined time between it starting
 *          to sleep on the futex, and it getting triggered by another
 *          thread. */
#define FUTEX_WAIT_MASK               0x10 /* >> while ((*uaddr & val) == (u32)uaddr2) {
                                            * >>      *uaddr |= val3;
                                            * >>      WAIT(uaddr, mask = FUTEX_BITSET_MATCH_ANY, utime);
                                            * >> } */
#define FUTEX_WAIT_NMASK              0x11 /* >> while ((*uaddr & val) != (u32)uaddr2) {
                                            * >>      *uaddr |= val3;
                                            * >>      WAIT(uaddr, mask = FUTEX_BITSET_MATCH_ANY, utime);
                                            * >> } */
#define FUTEX_WAIT_CMPXCH             0x14 /* A highly advanced version of a futex_wait operation that atomically
                                            * does all of the following (read, compare, write, wake and wait):
                                            * >> if (uaddr2) {
                                            * >>     u32 old_value;
                                            * >>     old_value = ATOMIC_CMPXCH_VAL(*uaddr,val,val3);
                                            * >>     *uaddr2 = old_value; // This write happens once the thread has already started waiting.
                                            * >>     result = WAKE(uaddr2,ALL);
                                            * >>     if (old_value == val) {
                                            * >>         // Wait if the old value 
                                            * >>         if (!WAIT(uaddr, mask = FUTEX_BITSET_MATCH_ANY, utime))
                                            * >>              if (!result) result = -ETIMEDOUT;
                                            * >>     }
                                            * >> } else {
                                            * >>     if (ATOMIC_CMPXCH(*uaddr,val,val3)) {
                                            * >>         if (!WAIT(uaddr, mask = FUTEX_BITSET_MATCH_ANY, utime))
                                            * >>              result = -ETIMEDOUT;
                                            * >>     }
                                            * >> }
                                            * While not perfectly optimized for any situation, due to the fact
                                            * that it implements a compare-exchange operation, there shouldn't
                                            * be any arithmetic operation that couldn't be expressed using this.
                                            * Also: There should be some interesting things doable because of
                                            *       the fact that this one also allows waking another futex
                                            *       when already waiting for another one.
                                            * NOTE: When `uaddr2' matches `uaddr', the function will return
                                            *       immediately, as you're just going to wake up yourself
                                            *       before actually being un-scheduled. */
#define FUTEX_WAIT_CMPXCH2            0x15 /* Very similar to `FUTEX_WAIT_CMPXCH', but only trigger `uaddr' if the exchange failed:
                                            * >> u32 old_value;
                                            * >> old_value = ATOMIC_CMPXCH_VAL(*uaddr,val,val3);
                                            * >> if (old_value == val) {
                                            * >>     *uaddr2 = old_value; // This write happens once the thread has already started waiting.
                                            * >>     result = WAKE(uaddr2,ALL);
                                            * >>     // Wait if the old value 
                                            * >>     if (!WAIT(uaddr, mask = FUTEX_BITSET_MATCH_ANY, utime))
                                            * >>          if (!result) result = -ETIMEDOUT;
                                            * >> }
                                            * While not perfectly optimized for any situation, due to the fact
                                            * that it implements a compare-exchange operation, there shouldn't
                                            * be any arithmetic operation that couldn't be expressed using this.
                                            * Also: There should be some interesting things doable because of
                                            *       the fact that this one also allows waking another futex
                                            *       when already waiting for another one.
                                            * NOTE: When `uaddr2' matches `uaddr', the function will return
                                            *       immediately, as you're just going to wake up yourself
                                            *       before actually being un-scheduled. */
#define FUTEX_WAIT_MASK_BITSET        0x19 /* >> while ((*uaddr & val) == (u32)uaddr2) {
                                            * >>      *uaddr |= val3;
                                            * >>      WAIT(uaddr, mask = val3, utime);
                                            * >> } */
#define FUTEX_WAIT_NMASK_BITSET       0x1a /* >> while ((*uaddr & val) != (u32)uaddr2) {
                                            * >>      *uaddr |= val3;
                                            * >>      WAIT(uaddr, mask = val3, utime);
                                            * >> } */
#define FUTEX_NOP                     0x1f /* Does nothing. (futex() returns ZERO(0))
                                            * Mainly intended for `xppoll()' when a condition is already available. */
#define FUTEX_WAIT_GHOST              0x20 /* >> while (*uaddr == val)
                                            * >>      WAIT_GHOST(uaddr, mask = FUTEX_BITSET_MATCH_ANY, utime);
                                            * Same as `FUTEX_WAIT', but when being woken using a val3 that
                                            * isn't equal to ZERO(0), the calling thread will not count towards
                                            * the total number of woken threads.
                                            * This is highly useful to implement user-space poll(), or waitfor()
                                            * functions that wait for something like a mutex to get unlocked,
                                            * without actually preventing another thread that is waiting for the
                                            * mutex because it wants to acquire it from being woken because the
                                            * mutex_put() function uses val3=1 to only wake a single waiter.
                                            * In such a context, `FUTEX_WAIT_GHOST' can be used to wait for the
                                            * mutex to get unlocked without interfering with the regular process
                                            * of locking/unlocking the mutex, that could otherwise cause a soft-lock
                                            * when no thread is intending to acquire a mutex that has been unlocked,
                                            * with some threads still waiting to be woken by the associated futex. */
#define FUTEX_WAIT_BITSET_GHOST       0x29 /* >> if (*uaddr == val)
                                            * >>      WAIT_GHOST(uaddr, mask = val3, utime);
                                            * A bitset-enabled variant of `FUTEX_WAIT_GHOST' */
#define FUTEX_WAIT_MASK_GHOST         0x30 /* The ghost variant of `FUTEX_WAIT_MASK' */
#define FUTEX_WAIT_MASK_BITSET_GHOST  0x39 /* The ghost variant of `FUTEX_WAIT_MASK_BITSET' */
#define FUTEX_WAIT_NMASK_GHOST        0x31 /* The ghost variant of `FUTEX_WAIT_NMASK' */
#define FUTEX_WAIT_NMASK_BITSET_GHOST 0x3a /* The ghost variant of `FUTEX_WAIT_NMASK_BITSET' */
#define FUTEX_WAITFD                  0x40 /* Same as `FUTEX_WAIT_FD_BITSET' with `val3 = FUTEX_BITSET_MATCH_ANY' */
#define FUTEX_WAIT_CMPXCH_GHOST       0x44 /* The ghost variant of `FUTEX_WAIT_CMPXCH' */
#define FUTEX_WAIT_CMPXCH2_GHOST      0x35 /* The ghost variant of `FUTEX_WAIT_CMPXCH2' */
#define FUTEX_WAITFD_BITSET           0x49 /* @param: uaddr: The address of the futex to wait for, as well as the address to which to bind the file handle.
                                            * @param: val:   The value to probe `uaddr' for, during a poll() or read() operation
                                            * @param: val2:  A set of `O_ACCMODE' and `O_CLOEXEC|O_CLOFORK|O_NONBLOCK' applied to the new file descriptor.
                                            *         NOTE: `O_ACCMODE' specifies if `read()' and `write()' can be used on the handle.
                                            *                It however does not affect `poll()', which is always allowed.
                                            * @param: val3:  The futex channel mask to which to connect during poll() or read()
                                            *          NOTE: During a poll(), the channel mask might interact with other
                                            *                file descriptors being polled, potentially causing sporadic
                                            *                wake-ups when the states of other, unrelated file descriptors
                                            *                change.
                                            *                Because of this, you should assume that including a futex
                                            *                handle in a poll() system call may lead to sporadic wake ups.
                                            *               (Although due to the nature of poll(), the possibility of this
                                            *                happening shouldn't require any changes to existing code)
                                            * >> HANDLE(.futex = FUTEX_AT(.uaddr),
                                            * >>     poll = {
                                            * >>         if (!(mode & POLLIN))
                                            * >>               return 0;
                                            * >>         // NOTE: uaddr is evaluated in the VM of the polling() thread
                                            * >>         //       Passing this type of futex between processed is not
                                            * >>         //       recommended, as the address may be mapped differently.
                                            * >>         // NOTE: Polling here is always implemented as a ghost connection,
                                            * >>         //       quite simply because the only reason why one would want
                                            * >>         //       to use futex handles over the futex() system call itself,
                                            * >>         //       is to wait for more than a single condition, meaning that
                                            * >>         //       the reason why ghost connections exist always applies.
                                            * >>         //       In other words: using poll() must mean that the caller
                                            * >>         //       isn't sure if they actually want to acquire some lock, or
                                            * >>         //       if they'd rather prefer to acquire some other, unrelated
                                            * >>         //       lock.
                                            * >>         task_connect_ghost(.futex,.val3);
                                            * >>         if (*.uaddr != .val)
                                            * >>             return POLLIN;
                                            * >>         return 0;
                                            * >>     },
                                            * >>     read = {
                                            * >>         // read() acts like poll(), but atomically saves the value read
                                            * >>         // from the bound user-space address within the provided buffer,
                                            * >>         // thus allowing for atomic read+wait on a futex that may be
                                            * >>         // changing its value rather quickly.
                                            * >>         if (bufsize < 4)
                                            * >>             error_throwf(E_BUFFER_TOO_SMALL,...);
                                            * >>     again:
                                            * >>         task_connect(.futex,.val3);
                                            * >>         u32 value = *.uaddr;
                                            * >>         if (value != .val) {
                                            * >>             *(u32 *)buffer = value;
                                            * >>             task_disconnect();
                                            * >>             return 4;
                                            * >>         }
                                            * >>         if (O_NONBLOCK) {
                                            * >>             task_disconnect();
                                            * >>             return 0;
                                            * >>         }
                                            * >>         task_wait();
                                            * >>         goto again;
                                            * >>     },
                                            * >>     write = {
                                            * >>         // write() acts like `FUTEX_WAKE'
                                            * >>         if (bufsize < 4)
                                            * >>             error_throwf(E_BUFFER_TOO_SMALL,...);
                                            * >>         WAKE_MAX(.futex, mask = FUTEX_BITSET_MATCH_ANY, max_threads = *(u32 *)buffer)
                                            * >>         return 4;
                                            * >>     },
                                            * >>     pwrite = {
                                            * >>         // pwrite() acts like `FUTEX_WAKE_BITSET'
                                            * >>         if (pos > UINT32_MAX)
                                            * >>             error_throw(E_INVALID_ARGUMENT);
                                            * >>         if (bufsize < 4)
                                            * >>             error_throwf(E_BUFFER_TOO_SMALL,...);
                                            * >>         WAKE_MAX(.futex, mask = (u32)pos, max_threads = *(u32 *)buffer);
                                            * >>         return 4;
                                            * >>     });
                                            */
#endif /* KOS Extensions... */

#define FUTEX_PRIVATE_FLAG       0x80
#define FUTEX_CLOCK_REALTIME     0x100
#define FUTEX_CMD_MASK        (~(FUTEX_PRIVATE_FLAG|FUTEX_CLOCK_REALTIME))

#define FUTEX_WAIT_PRIVATE              (FUTEX_WAIT|FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_PRIVATE              (FUTEX_WAKE|FUTEX_PRIVATE_FLAG)
#define FUTEX_REQUEUE_PRIVATE           (FUTEX_REQUEUE|FUTEX_PRIVATE_FLAG)
#define FUTEX_CMP_REQUEUE_PRIVATE       (FUTEX_CMP_REQUEUE|FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_OP_PRIVATE           (FUTEX_WAKE_OP|FUTEX_PRIVATE_FLAG)
#define FUTEX_LOCK_PI_PRIVATE           (FUTEX_LOCK_PI|FUTEX_PRIVATE_FLAG)
#define FUTEX_UNLOCK_PI_PRIVATE         (FUTEX_UNLOCK_PI|FUTEX_PRIVATE_FLAG)
#define FUTEX_TRYLOCK_PI_PRIVATE        (FUTEX_TRYLOCK_PI|FUTEX_PRIVATE_FLAG)
#define FUTEX_WAIT_BITSET_PRIVATE       (FUTEX_WAIT_BITSET|FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_BITSET_PRIVATE       (FUTEX_WAKE_BITSET|FUTEX_PRIVATE_FLAG)
#define FUTEX_WAIT_REQUEUE_PI_PRIVATE   (FUTEX_WAIT_REQUEUE_PI|FUTEX_PRIVATE_FLAG)
#define FUTEX_CMP_REQUEUE_PI_PRIVATE    (FUTEX_CMP_REQUEUE_PI|FUTEX_PRIVATE_FLAG)

#ifdef __CC__
/*
 * Per-lock list entry - embedded in user-space locks, somewhere close
 * to the futex field. (Note: user-space uses a double-linked list to
 * achieve O(1) list add and remove, but the kernel only needs to know
 * about the forward link)
 *
 * NOTE: this structure is part of the syscall ABI, and must not be
 * changed.
 */
struct robust_list {
    struct robust_list *next;
};

/*
 * Per-thread list head:
 *
 * NOTE: this structure is part of the syscall ABI, and must only be
 * changed if the change is first communicated with the glibc folks.
 * (When an incompatible change is done, we'll increase the structure
 *  size, which glibc will detect)
 */
struct robust_list_head {
    /*
     * The head of the list. Points back to itself if empty:
     */
    struct robust_list list;

    /*
     * This relative offset is set by user-space, it gives the kernel
     * the relative position of the futex field to examine. This way
     * we keep userspace flexible, to freely shape its data-structure,
     * without hardcoding any particular offset into the kernel:
     */
    long futex_offset;

    /*
     * The death of the thread may race with userspace setting
     * up a lock's links. So to handle this race, userspace first
     * sets this field to the address of the to-be-taken lock,
     * then does the lock acquire, and then adds itself to the
     * list, and then clears this field. Hence the kernel will
     * always have full knowledge of all locks that the thread
     * _might_ have taken. We check the owner TID in any case,
     * so only truly owned locks will be handled.
     */
    struct robust_list *list_op_pending;
};
#endif /* __CC__ */

/*
 * Are there any waiters for this robust futex:
 */
#define FUTEX_WAITERS       0x80000000

/*
 * The kernel signals via this bit that a thread holding a futex
 * has exited without unlocking the futex. The kernel also does
 * a FUTEX_WAKE on such futexes, after setting the bit, to wake
 * up any possible waiters:
 */
#define FUTEX_OWNER_DIED    0x40000000

/*
 * The rest of the robust-futex field is for the TID:
 */
#define FUTEX_TID_MASK      0x3fffffff

/*
 * This limit protects against a deliberately circular list.
 * (Not worth introducing an rlimit for it)
 */
#define ROBUST_LIST_LIMIT   2048

/*
 * bitset with all bits set for the FUTEX_xxx_BITSET OPs to request a
 * match of any bit.
 */
#define FUTEX_BITSET_MATCH_ANY 0xffffffff


#define FUTEX_OP_SET           0    /* *(int *)UADDR2 = OPARG; */
#define FUTEX_OP_ADD           1    /* *(int *)UADDR2 += OPARG; */
#define FUTEX_OP_OR            2    /* *(int *)UADDR2 |= OPARG; */
#define FUTEX_OP_ANDN          3    /* *(int *)UADDR2 &= ~OPARG; */
#define FUTEX_OP_XOR           4    /* *(int *)UADDR2 ^= OPARG; */

#define FUTEX_OP_OPARG_SHIFT   8    /* Use (1 << OPARG) instead of OPARG.  */

#define FUTEX_OP_CMP_EQ        0    /* if (oldval == CMPARG) wake */
#define FUTEX_OP_CMP_NE        1    /* if (oldval != CMPARG) wake */
#define FUTEX_OP_CMP_LT        2    /* if (oldval < CMPARG) wake */
#define FUTEX_OP_CMP_LE        3    /* if (oldval <= CMPARG) wake */
#define FUTEX_OP_CMP_GT        4    /* if (oldval > CMPARG) wake */
#define FUTEX_OP_CMP_GE        5    /* if (oldval >= CMPARG) wake */

/* FUTEX_WAKE_OP will perform atomically
 * >> int oldval = *(int *)UADDR2;
 * >> *(int *)UADDR2 = oldval OP OPARG;
 * >> if (oldval CMP CMPARG)
 * >>     wake UADDR2; 
 */

#define FUTEX_OP(op,oparg,cmp,cmparg) \
  ((((op) & 0xf) << 28) | (((cmp) & 0xf) << 24) | \
   (((oparg) & 0xfff) << 12) | ((cmparg) & 0xfff))


__SYSDECL_END

#endif /* !_LINUX_FUTEX_H */
