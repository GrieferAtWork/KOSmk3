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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_SIGNAL_H
#define GUARD_KERNEL_INCLUDE_SCHED_SIGNAL_H 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <kos/types.h>
#include <hybrid/sched/yield.h>
#include <assert.h>
#include <stdbool.h>

#ifdef __CC__
DECL_BEGIN

struct task_connection;


/* Since KOS is designed to only run on machines with a
 * pointer size of at least 32 bits, that means that
 * all instances of `struct task_connection' must be
 * aligned by at least 4 bytes.
 * With that in mind, we can use the bottom 2 bits for
 * our own purposes, in this case: As locking bits.
 * This way, we keep `struct sig' the size of a single
 * pointer, meaning it is one of the most light-weight
 * types there are (which is important, considering
 * that signals are the bottom-most structured used
 * to implement all other synchronization primitives) */
#define SIG_FLOCKBIT     0x1   /* Primary locking bit. */
#define SIG_FLOCKBIT2    0x2   /* Secondary locking bit (freely available to the user of the signal)
                                * This bit could be used to implement a quick, non-recursive mutex. */
#define SIG_FLOCKMASK    0x3
#define SIG_FADDRMASK  (~0x3)

struct sig {
    uintptr_t s_ptr; /* [TYPE(struct task_connection)]
                      *  Chain of connected tasks. */
};
#define SIG_INIT       { 0 }
#define SIG_GETCON(x) ((struct task_connection *)((x)->s_ptr & SIG_FADDRMASK))
#define sig_init(x)    (void)((x)->s_ptr = 0)
#define sig_cinit(x)   assert((x)->s_ptr == 0)
#define sig_holding(x) ((x)->s_ptr & SIG_FLOCKBIT)

/* Acquire / release a lock on the given signal. */
FORCELOCAL ATTR_NOTHROW bool KCALL sig_try(struct sig *__restrict self) {
 return !(ATOMIC_FETCHOR(self->s_ptr,SIG_FLOCKBIT) & SIG_FLOCKBIT);
}
FORCELOCAL ATTR_NOTHROW void KCALL sig_get(struct sig *__restrict self) {
 while (ATOMIC_FETCHOR(self->s_ptr,SIG_FLOCKBIT) & SIG_FLOCKBIT)
     SCHED_YIELD();
 COMPILER_BARRIER();
}
FORCELOCAL ATTR_NOTHROW void KCALL sig_put(struct sig *__restrict self) {
 COMPILER_BARRIER();
#ifdef NDEBUG
 ATOMIC_FETCHAND(self->s_ptr,~SIG_FLOCKBIT);
#else
 __assertef(ATOMIC_FETCHAND(self->s_ptr,~SIG_FLOCKBIT) & SIG_FLOCKBIT,
            "self = %p, self->s_ptr = %p",self,self->s_ptr);
#endif
}

/* Send signal `self' to up to `max_threads' connected threads.
 * @return: * : The actual number of threads notified. */
FUNDEF ATTR_NOTHROW size_t KCALL sig_send(struct sig *__restrict self, size_t max_threads);
FUNDEF ATTR_NOTHROW size_t KCALL sig_broadcast(struct sig *__restrict self);

/* Same as `sig_send()', but the caller is responsible for holding
 * a lock to `self', and that lock will not be let go of in-between
 * sending the signal to different threads. */
FUNDEF ATTR_NOTHROW size_t KCALL sig_send_locked(struct sig *__restrict self, size_t max_threads);
FUNDEF ATTR_NOTHROW size_t KCALL sig_broadcast_locked(struct sig *__restrict self);

/* Same as `sig_send()', but only wake thread connected to
 * any of the channels that are masked by `signal_mask'.
 * For more information, see the detailed documentation of
 * this functionality at the declaration of `task_channelmask()'.
 * @param: signal_mask: The mask of channels that should be signaled.
 * @return: * : The actual number of threads notified. */
FUNDEF ATTR_NOTHROW size_t KCALL sig_send_channel(struct sig *__restrict self, uintptr_t signal_mask, size_t max_threads);
FUNDEF ATTR_NOTHROW size_t KCALL sig_broadcast_channel(struct sig *__restrict self, uintptr_t signal_mask);
FUNDEF ATTR_NOTHROW size_t KCALL sig_send_channel_locked(struct sig *__restrict self, uintptr_t signal_mask, size_t max_threads);
FUNDEF ATTR_NOTHROW size_t KCALL sig_broadcast_channel_locked(struct sig *__restrict self, uintptr_t signal_mask);

/* As far as usage semantics go, these functions are 100% identical to
 * their counterparts above. However, unlike the functions above, and under 
 * the right conditions, these functions will do their best to have the
 * woken threads continue executing as soon as possible.
 * Use of these functions should be kept to a bare minimum, so-as not to
 * utterly confuse the scheduler. However when used correctly, they can
 * improve response time considerably, and should therefor be used to
 * signal events related to user-input.
 * NOTES:
 *   - If no threads are woken, these functions are truly identical to those above.
 *   - If more than one thread is woken, scheduling order to manipulated such that
 *     threads will execute in order of initially connecting to signals.
 *     If such a re-ordering operation isn't possible due to priority constraints,
 *     or due to threads operating on different cores (meaning that the calling thread's
 *     CPU can't interfere with the scheduling order of the woken thread's CPU),
 *     woken threads may not be re-ordered. */
FUNDEF ATTR_NOTHROW size_t KCALL sig_send_p(struct sig *__restrict self, size_t max_threads);
FUNDEF ATTR_NOTHROW size_t KCALL sig_broadcast_p(struct sig *__restrict self);
FUNDEF ATTR_NOTHROW size_t KCALL sig_send_locked_p(struct sig *__restrict self, size_t max_threads);
FUNDEF ATTR_NOTHROW size_t KCALL sig_broadcast_locked_p(struct sig *__restrict self);
FUNDEF ATTR_NOTHROW size_t KCALL sig_send_channel_p(struct sig *__restrict self, uintptr_t signal_mask, size_t max_threads);
FUNDEF ATTR_NOTHROW size_t KCALL sig_broadcast_channel_p(struct sig *__restrict self, uintptr_t signal_mask);
FUNDEF ATTR_NOTHROW size_t KCALL sig_send_channel_locked_p(struct sig *__restrict self, uintptr_t signal_mask, size_t max_threads);
FUNDEF ATTR_NOTHROW size_t KCALL sig_broadcast_channel_locked_p(struct sig *__restrict self, uintptr_t signal_mask);


DECL_END
#endif /* __CC__ */

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_SIGNAL_H */
