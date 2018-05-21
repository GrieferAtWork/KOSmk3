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
#ifndef _KOS_FUTEX_H
#define _KOS_FUTEX_H 1

#include <hybrid/compiler.h>
#include <hybrid/timespec.h>
#include <features.h>
#include <linux/futex.h>
#include <bits/types.h>
#include <bits/sigset.h>

DECL_BEGIN

#ifdef __CC__
#ifndef __futex_t_defined
#define __futex_t_defined 1
typedef __uint32_t  futex_t;
typedef __uintptr_t futex_channel_t;
#endif /* !__futex_t_defined */

#ifdef __CRT_KOS
#ifndef __pollfutex_defined
#define __pollfutex_defined 1
struct pollfutex {
    /* Descriptor of a futex when using the xppoll() system call.
     * NOTE: sizeof(struct pollfutex) == 4 * sizeof(void *)
     */
    USER UNCHECKED futex_t    *pf_futex;                 /* [1..1] The futex word that should be polled.
                                                          * When faulty, `pf_status' is set to `POLLFUTEX_STATUS_BADFUTEX' */
    __uint8_t                  pf_action;                /* The action that should be performed in order to poll the futex word.
                                                          * One of (optionally or'd with `FUTEX_PRIVATE_FLAG'):
                                                          *  - `FUTEX_WAIT', `FUTEX_WAIT_GHOST'               (`if (*pf_futex != pf_val) READY()')
                                                          *  - `FUTEX_WAIT_BITSET', `FUTEX_WAIT_BITSET_GHOST' (`if (*pf_futex != pf_val) READY()')
                                                          *  - `FUTEX_LOCK_PI'  (Will ensure that the `FUTEX_WAITERS' bit is set)
                                                          *  - `FUTEX_WAITMASK', `FUTEX_WAITMASK_GHOST'  (Will ensure that all bits from `val3' are set)
                                                          *  - `FUTEX_WAIT_CMPXCH', `FUTEX_WAIT_CMPXCH_GHOST'  (Will perform the CMPXCH, properly save the value, and also trigger UADDR2 after polling was started)
                                                          *  - `FUTEX_WAIT_CMPXCH2', `FUTEX_WAIT_CMPXCH2_GHOST'  (Will perform the CMPXCH, properly save the value, and also trigger UADDR2 after polling was started)
                                                          * When signaled, the same action will not block
                                                          * when executed using regular futex operations.
                                                          * Note however that another thread may have also been informed, meaning
                                                          * that the futex may already be locked again when xppoll() returns. */
#define POLLFUTEX_STATUS_NOEVENT   0x00                  /* No event has been triggered for this futex. (ALWAYS ZERO(0))
                                                          * NOTE: Never set or interpreted by the kernel. - Must be pre-set by
                                                          *       the user if knowledge of which futex was triggered is important. */
#define POLLFUTEX_STATUS_AVAILABLE 0x01                  /* The futex has become available. */
#define POLLFUTEX_STATUS_BADFUTEX  0xfe                  /* The specified `pf_futex' address is faulty. */
#define POLLFUTEX_STATUS_BADACTION 0xff                  /* The specified `pf_action' isn't known. */
    __uint8_t                  pf_status;                /* Poll status (One of `POLLFUTEX_STATUS_*') */
    __uint8_t                __pf_pad[sizeof(void *)-2]; /* ... */
    union PACKED {
        __uintptr_t            pf_val;                   /* The `val' argument in an equivalent futex() system call. */
        __uintptr_t            pf_result;                /* The storage location for the `result' of the following actions:
                                                          *   - FUTEX_WAIT_CMPXCH   (Only for the case of `result = WAKE(uaddr2,ALL)')
                                                          *   - FUTEX_WAIT_CMPXCH2  (Only for the case of `result = WAKE(uaddr2,ALL)')
                                                          */
    };
    union PACKED {
        __uintptr_t            pf_val2;                  /* The `val2' argument in an equivalent futex() system call. */
        USER UNCHECKED futex_t*pf_uaddr2;                /* The `uaddr2' argument in an equivalent futex() system call. */
    };
    __uintptr_t                pf_val3;                  /* The `val3' argument in an equivalent futex() system call. */
};
#endif /* !__pollfutex_defined */

/* Initialization helpers for pollfutex objects. */
__FORCELOCAL struct pollfutex *(__LIBCCALL pollfutex_init_nop)
(struct pollfutex *__restrict __self) {
 __self->pf_action = FUTEX_NOP;
 return __self;
}
__FORCELOCAL struct pollfutex *(__LIBCCALL pollfutex_init_wait)
(struct pollfutex *__restrict __self, futex_t *__uaddr, futex_t __proble_value) {
 __self->pf_futex  = __uaddr;
 __self->pf_action = FUTEX_WAIT;
 __self->pf_status = POLLFUTEX_STATUS_NOEVENT;
 __self->pf_val    = __proble_value;
 return __self;
}
__FORCELOCAL struct pollfutex *(__LIBCCALL pollfutex_init_wait_bitset)
(struct pollfutex *__restrict __self, futex_t *__uaddr,
 futex_t __proble_value, futex_channel_t __channels) {
 __self->pf_futex  = __uaddr;
 __self->pf_action = FUTEX_WAIT_BITSET;
 __self->pf_status = POLLFUTEX_STATUS_NOEVENT;
 __self->pf_val    = __proble_value;
 __self->pf_val3   = __channels;
 return __self;
}
__FORCELOCAL struct pollfutex *(__LIBCCALL pollfutex_init_wait_mask)
(struct pollfutex *__restrict __self, futex_t *__uaddr,
 futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits) {
 __self->pf_futex  = __uaddr;
 __self->pf_action = FUTEX_WAIT_MASK;
 __self->pf_status = POLLFUTEX_STATUS_NOEVENT;
 __self->pf_val    = __probe_mask;
 __self->pf_val2   = __probe_value;
 __self->pf_val3   = __enable_bits;
 return __self;
}
__FORCELOCAL struct pollfutex *(__LIBCCALL pollfutex_init_wait_nmask)
(struct pollfutex *__restrict __self, futex_t *__uaddr,
 futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits) {
 __self->pf_futex  = __uaddr;
 __self->pf_action = FUTEX_WAIT_NMASK;
 __self->pf_status = POLLFUTEX_STATUS_NOEVENT;
 __self->pf_val    = __probe_mask;
 __self->pf_val2   = __probe_value;
 __self->pf_val3   = __enable_bits;
 return __self;
}
__FORCELOCAL struct pollfutex *(__LIBCCALL pollfutex_init_wait_cmpxch)
(struct pollfutex *__restrict __self, futex_t *__uaddr,
 futex_t __old_value, futex_t __new_value, futex_t *__update_target) {
 __self->pf_futex  = __uaddr;
 __self->pf_action = FUTEX_WAIT_CMPXCH;
 __self->pf_status = POLLFUTEX_STATUS_NOEVENT;
 __self->pf_val    = __old_value;
 __self->pf_uaddr2 = __update_target;
 __self->pf_val3   = __new_value;
 return __self;
}
__FORCELOCAL struct pollfutex *(__LIBCCALL pollfutex_init_wait_cmpxch2)
(struct pollfutex *__restrict __self, futex_t *__uaddr,
 futex_t __old_value, futex_t __new_value, futex_t *__update_target) {
 __self->pf_futex  = __uaddr;
 __self->pf_action = FUTEX_WAIT_CMPXCH2;
 __self->pf_status = POLLFUTEX_STATUS_NOEVENT;
 __self->pf_val    = __old_value;
 __self->pf_uaddr2 = __update_target;
 __self->pf_val3   = __new_value;
 return __self;
}


#ifndef __pollpid_defined
#define __pollpid_defined 1
struct __siginfo_struct;
struct pollpid {
    /* Descriptor of a PID when using the xppoll() system call.
     * NOTE: Polling a PID for an event will not reset that event,
     *       like performing a call to wait(2) would, nor does it
     *       kill a PID's zombie, regardless of the `WNOHANG' flag.
     *       In other words, polling a PID is just that:
     *       wait for something to happen to the thread.
     * NOTE: Posix describes a special behavior for wait(2) when
     *       SIGCHLD is being ignored. This behavior does _NOT_
     *       come into action when polling a PID.
     * >> This functionality is required to safely implement the
     *    WAITFOR flag for user-space RPCs, due to the fact that
     *    the receiving thread may die after having acknowledged
     *    the RPC, but before having completed it.
     * HINT: This is the only way of waiting for a thread/process
     *       while also providing a timeout, without the use of
     *       futex exit signals. */
    __pid_t                    pp_pid;     /* PID of the process to poll() (Requires the same permissions as wait()) */
    __pid_t                    pp_result;  /* The PID that got triggered. */
    __uint16_t                 pp_which;   /* One of `P_*' (e.g. P_PGID) describing the kind of PID that `pp_pid' is.
                                            * When set of `P_ALL', `pp_pid' is ignored. */
    __uint16_t                 pp_options; /* Set of `W*' describing what to wait for. */
#if __SIZEOF_POINTER__ > 4
    __uint32_t               __pp_pad;     /* ... */
#endif
    struct __siginfo_struct   *pp_info;    /* [0..1] When non-NULL, filled with information on what happened. */
    struct rusage             *pp_ru;      /* [0..1] When non-NULL, filled with rusage information. */
};
#endif /* !__pollpid_defined */

/* Initialization helpers for pollpid objects. */
__FORCELOCAL struct pollpid *(__LIBCCALL pollpid_init)
(struct pollpid *__restrict __self, int __which, __pid_t __pid, int __options,
 struct __siginfo_struct *__info, struct rusage *__ru) {
 __self->pp_pid     = __pid;
 __self->pp_result  = 0;
 __self->pp_which   = __which;
 __self->pp_options = __options;
 __self->pp_info    = __info;
 __self->pp_ru      = __ru;
 return __self;
}

#if defined(__KERNEL__) || defined(__BUILDING_LIBC)
struct poll_info {
    USER UNCHECKED struct pollfd    *i_ufdvec; /* [0..i_cnt] Vector of poll descriptors. */
    size_t                           i_ufdcnt; /* Number of elements. */
    USER UNCHECKED struct pollfutex *i_ftxvec; /* [0..i_cnt] Vector of poll descriptors. */
    size_t                           i_ftxcnt; /* Number of elements. */
    USER UNCHECKED struct pollpid   *i_pidvec; /* [0..i_cnt] Vector of poll descriptors. */
    size_t                           i_pidcnt; /* Number of elements. */
};
#endif
#endif /* __CRT_KOS */

struct pollfd;

#if !defined(__KERNEL__) && defined(__CRT_KOS)
/* An extension to the poll() / select() family of system calls,
 * that allows the user to not only wait for file descriptors to
 * become ready, but also wait for an arbitrary number of futex
 * events to happen, while also implementing support for atomic
 * posix_signal masking.
 * For more information, see the definition of `struct pollfutex'
 * HINT: Even without this, one could create futex file descriptors
 *       using `futex_waitfd()', which could then be passed to a
 *       regular ppoll() system call, however this version offers
 *       more functionality, and uses less system resources, as
 *       well as not requiring additional file descriptor indices.
 * @return: 0 : The given timeout has expired.
 * @return: * : The sum of available futexes and handles combined. */
__REDIRECT_EXCEPT_TM64(__LIBC,,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xppoll,
                      (struct pollfd *__ufds, __size_t __nfds,
                       struct pollfutex *__uftx, __size_t __nftx,
                       struct pollpid *__upid, __size_t __npid,
                       struct timespec const *__abs_timeout, __sigset_t *sig),
                      (__ufds,__nfds,__uftx,__nftx,__upid,__npid,__abs_timeout,sig))
#ifdef __USE_TIME64
__REDIRECT_EXCEPT(__LIBC,,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xppoll64,
                 (struct pollfd *__ufds, __size_t __nfds,
                  struct pollfutex *__uftx, __size_t __nftx,
                  struct pollpid *__upid, __size_t __npid,
                  struct __timespec64 const *__abs_timeout, __sigset_t *sig),
                 (__ufds,__nfds,__uftx,__nftx,__upid,__npid,__abs_timeout,sig))
#endif /* __USE_TIME64 */
#ifdef __USE_EXCEPT
__REDIRECT_TM64(__LIBC,,__size_t,__LIBCCALL,Xxppoll,
               (struct pollfd *__ufds, __size_t __nfds,
                struct pollfutex *__uftx, __size_t __nftx,
                struct pollpid *__upid, __size_t __npid,
                struct timespec const *__abs_timeout, __sigset_t *sig),
               (__ufds,__nfds,__uftx,__nftx,__upid,__npid,__abs_timeout,sig))
#ifdef __USE_TIME64
__LIBC __size_t (__LIBCCALL Xxppoll64)(struct pollfd *__ufds, __size_t __nfds,
                                       struct pollfutex *__uftx, __size_t __nftx,
                                       struct pollpid *__upid, __size_t __npid,
                                       struct __timespec64 const *__abs_timeout, __sigset_t *sig);
#endif /* __USE_TIME64 */
#endif /* __USE_EXCEPT */

#if !defined(__NO_ATTR_FORCEINLINE) && !defined(__NO_builtin_constant_p)
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,__os_futex_wait_inf,(futex_t *__uaddr, futex_t __probe_value),futex_wait_inf,(__uaddr,__probe_value))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,__os_futex_wait_inf_cmpxch,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target),futex_wait_inf_cmpxch,(__uaddr,__old_value,__new_value,__update_target))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,__os_futex_wait_inf_cmpxch2,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target),futex_wait_inf_cmpxch2,(__uaddr,__old_value,__new_value,__update_target))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,__os_futex_wait_inf_mask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits),futex_wait_inf_mask,(__uaddr,__probe_mask,__probe_value,__enable_bits))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,__os_futex_wait_inf_nmask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits),futex_wait_inf_nmask,(__uaddr,__probe_mask,__probe_value,__enable_bits))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,__os_futex_wait_inf_bitset,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels),futex_wait_inf_bitset,(__uaddr,__probe_value,__channels))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,__os_futex_wait_inf_ghost,(futex_t *__uaddr, futex_t __probe_value),futex_wait_inf_ghost,(__uaddr,__probe_value))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,__os_futex_wait_inf_cmpxch_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target),futex_wait_inf_cmpxch_ghost,(__uaddr,__old_value,__new_value,__update_target))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,__os_futex_wait_inf_cmpxch2_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target),futex_wait_inf_cmpxch2_ghost,(__uaddr,__old_value,__new_value,__update_target))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,__os_futex_wait_inf_mask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits),futex_wait_inf_mask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,__os_futex_wait_inf_nmask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits),futex_wait_inf_nmask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,__os_futex_wait_inf_bitset_ghost,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels),futex_wait_inf_bitset_ghost,(__uaddr,__probe_value,__channels))
__REDIRECT_EXCEPT_XVOID_(__LIBC,,int,__LIBCCALL,__os_futex_lock_inf,(futex_t *__uaddr),futex_lock_inf,(__uaddr))
#ifdef __USE_EXCEPT
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_Xfutex_wait_inf,(futex_t *__uaddr, futex_t __probe_value),Xfutex_wait_inf,(__uaddr,__probe_value))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_Xfutex_wait_inf_cmpxch,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target),Xfutex_wait_inf_cmpxch,(__uaddr,__old_value,__new_value,__update_target))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_Xfutex_wait_inf_cmpxch2,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target),Xfutex_wait_inf_cmpxch2,(__uaddr,__old_value,__new_value,__update_target))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_Xfutex_wait_inf_mask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits),Xfutex_wait_inf_mask,(__uaddr,__probe_mask,__probe_value,__enable_bits))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_Xfutex_wait_inf_nmask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits),Xfutex_wait_inf_nmask,(__uaddr,__probe_mask,__probe_value,__enable_bits))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_Xfutex_wait_inf_bitset,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels),Xfutex_wait_inf_bitset,(__uaddr,__probe_value,__channels))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_Xfutex_wait_inf_ghost,(futex_t *__uaddr, futex_t __probe_value),Xfutex_wait_inf_ghost,(__uaddr,__probe_value))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_Xfutex_wait_inf_cmpxch_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target),Xfutex_wait_inf_cmpxch_ghost,(__uaddr,__old_value,__new_value,__update_target))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_Xfutex_wait_inf_cmpxch2_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target),Xfutex_wait_inf_cmpxch2_ghost,(__uaddr,__old_value,__new_value,__update_target))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_Xfutex_wait_inf_mask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits),Xfutex_wait_inf_mask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_Xfutex_wait_inf_nmask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits),Xfutex_wait_inf_nmask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_Xfutex_wait_inf_bitset_ghost,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels),Xfutex_wait_inf_bitset_ghost,(__uaddr,__probe_value,__channels))
__REDIRECT_VOID(__LIBC,,__LIBCCALL,__os_Xfutex_lock_inf,(futex_t *__uaddr),Xfutex_lock_inf,(__uaddr))
#endif /* __USE_EXCEPT */
#ifdef __USE_TIME_BITS64
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout),futex_wait64,(__uaddr,__probe_value,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_cmpxch,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),futex_wait64_cmpxch,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_cmpxch2,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),futex_wait64_cmpxch2,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_mask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),futex_wait64_mask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_nmask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),futex_wait64_nmask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_bitset,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),futex_wait64_bitset,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_ghost,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout),futex_wait64_ghost,(__uaddr,__probe_value,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_cmpxch_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),futex_wait64_cmpxch_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_cmpxch2_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),futex_wait64_cmpxch2_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_mask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),futex_wait64_mask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_nmask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),futex_wait64_nmask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_bitset_ghost,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),futex_wait64_bitset_ghost,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_waitrel,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout),futex_waitrel64,(__uaddr,__probe_value,__rel_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_waitrel_ghost,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout),futex_waitrel64_ghost,(__uaddr,__probe_value,__rel_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_lock,(futex_t *__uaddr, struct timespec const *__abs_timeout),futex_lock64,(__uaddr,__abs_timeout))
#ifdef __USE_EXCEPT
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout),Xfutex_wait64,(__uaddr,__probe_value,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_cmpxch,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),Xfutex_wait64_cmpxch,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_cmpxch2,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),Xfutex_wait64_cmpxch2,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_mask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),Xfutex_wait64_mask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_nmask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),Xfutex_wait64_nmask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_bitset,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),Xfutex_wait64_bitset,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_ghost,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout),Xfutex_wait64_ghost,(__uaddr,__probe_value,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_cmpxch_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),Xfutex_wait64_cmpxch_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_cmpxch2_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),Xfutex_wait64_cmpxch2_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_mask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),Xfutex_wait64_mask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_nmask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),Xfutex_wait64_nmask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_bitset_ghost,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),Xfutex_wait64_bitset_ghost,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_waitrel,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout),Xfutex_waitrel64,(__uaddr,__probe_value,__rel_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_waitrel_ghost,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout),Xfutex_waitrel64_ghost,(__uaddr,__probe_value,__rel_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_lock,(futex_t *__uaddr, struct timespec const *__abs_timeout),Xfutex_lock64,(__uaddr,__abs_timeout))
#endif /* __USE_EXCEPT */
#ifdef __USE_TIME64
#define __os_futex_wait64                __os_futex_wait
#define __os_futex_wait64_cmpxch         __os_futex_wait_cmpxch
#define __os_futex_wait64_cmpxch2        __os_futex_wait_cmpxch2
#define __os_futex_wait64_mask           __os_futex_wait_mask
#define __os_futex_wait64_nmask          __os_futex_wait_nmask
#define __os_futex_wait64_bitset         __os_futex_wait_bitset
#define __os_futex_wait64_ghost          __os_futex_wait_ghost
#define __os_futex_wait64_cmpxch_ghost   __os_futex_wait_cmpxch_ghost
#define __os_futex_wait64_cmpxch2_ghost  __os_futex_wait_cmpxch2_ghost
#define __os_futex_wait64_mask_ghost     __os_futex_wait_mask_ghost
#define __os_futex_wait64_nmask_ghost    __os_futex_wait_nmask_ghost
#define __os_futex_wait64_bitset_ghost   __os_futex_wait_bitset_ghost
#define __os_futex_waitrel64             __os_futex_waitrel
#define __os_futex_waitrel64_ghost       __os_futex_waitrel_ghost
#define __os_futex_lock64                __os_futex_lock
#ifdef __USE_EXCEPT
#define __os_Xfutex_wait64               __os_Xfutex_wait
#define __os_Xfutex_wait64_cmpxch        __os_Xfutex_wait_cmpxch
#define __os_Xfutex_wait64_cmpxch2       __os_Xfutex_wait_cmpxch2
#define __os_Xfutex_wait64_mask          __os_Xfutex_wait_mask
#define __os_Xfutex_wait64_nmask         __os_Xfutex_wait_nmask
#define __os_Xfutex_wait64_bitset        __os_Xfutex_wait_bitset
#define __os_Xfutex_wait64_ghost         __os_Xfutex_wait_ghost
#define __os_Xfutex_wait64_cmpxch_ghost  __os_Xfutex_wait_cmpxch_ghost
#define __os_Xfutex_wait64_cmpxch2_ghost __os_Xfutex_wait_cmpxch2_ghost
#define __os_Xfutex_wait64_mask_ghost    __os_Xfutex_wait_mask_ghost
#define __os_Xfutex_wait64_nmask_ghost   __os_Xfutex_wait_nmask_ghost
#define __os_Xfutex_wait64_bitset_ghost  __os_Xfutex_wait_bitset_ghost
#define __os_Xfutex_waitrel64            __os_Xfutex_waitrel
#define __os_Xfutex_waitrel64_ghost      __os_Xfutex_waitrel_ghost
#define __os_Xfutex_lock64               __os_Xfutex_lock
#endif
#endif /* __USE_TIME64 */
#else /* __USE_TIME_BITS64 */
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout),futex_wait,(__uaddr,__probe_value,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_cmpxch,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),futex_wait_cmpxch,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_cmpxch2,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),futex_wait_cmpxch2,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_mask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),futex_wait_mask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_nmask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),futex_wait_nmask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_bitset,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),futex_wait_bitset,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_ghost,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout),futex_wait_ghost,(__uaddr,__probe_value,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_cmpxch_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),futex_wait_cmpxch_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_cmpxch2_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),futex_wait_cmpxch2_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_mask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),futex_wait_mask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_nmask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),futex_wait_nmask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait_bitset_ghost,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),futex_wait_bitset_ghost,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_waitrel,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout),futex_waitrel,(__uaddr,__probe_value,__rel_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_waitrel_ghost,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout),futex_waitrel_ghost,(__uaddr,__probe_value,__rel_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_lock,(futex_t *__uaddr, struct timespec const *__abs_timeout),futex_lock,(__uaddr,__abs_timeout))
#ifdef __USE_EXCEPT
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout),Xfutex_wait,(__uaddr,__probe_value,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_cmpxch,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),Xfutex_wait_cmpxch,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_cmpxch2,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),Xfutex_wait_cmpxch2,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_mask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),Xfutex_wait_mask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_nmask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),Xfutex_wait_nmask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_bitset,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),Xfutex_wait_bitset,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_ghost,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout),Xfutex_wait_ghost,(__uaddr,__probe_value,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_cmpxch_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),Xfutex_wait_cmpxch_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_cmpxch2_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),Xfutex_wait_cmpxch2_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_mask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),Xfutex_wait_mask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_nmask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),Xfutex_wait_nmask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait_bitset_ghost,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),Xfutex_wait_bitset_ghost,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_waitrel,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout),Xfutex_waitrel,(__uaddr,__probe_value,__rel_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_waitrel_ghost,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout),Xfutex_waitrel_ghost,(__uaddr,__probe_value,__rel_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_lock,(futex_t *__uaddr, struct timespec const *__abs_timeout),Xfutex_lock,(__uaddr,__abs_timeout))
#endif /* __USE_EXCEPT */
#ifdef __USE_TIME64
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait64,(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__abs_timeout),futex_wait64,(__uaddr,__probe_value,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait64_cmpxch,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout),futex_wait64_cmpxch,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait64_cmpxch2,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout),futex_wait64_cmpxch2,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait64_mask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout),futex_wait64_mask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait64_nmask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout),futex_wait64_nmask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait64_bitset,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct __timespec64 const *__abs_timeout),futex_wait64_bitset,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait64_ghost,(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__abs_timeout),futex_wait64_ghost,(__uaddr,__probe_value,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait64_cmpxch_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout),futex_wait64_cmpxch_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait64_cmpxch2_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout),futex_wait64_cmpxch2_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait64_mask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout),futex_wait64_mask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait64_nmask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout),futex_wait64_nmask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_wait64_bitset_ghost,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct __timespec64 const *__abs_timeout),futex_wait64_bitset_ghost,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_waitrel64,(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__rel_timeout),futex_waitrel64,(__uaddr,__probe_value,__rel_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_waitrel64_ghost,(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__rel_timeout),futex_waitrel64_ghost,(__uaddr,__probe_value,__rel_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,__os_futex_lock64,(futex_t *__uaddr, struct __timespec64 const *__abs_timeout),futex_lock64,(__uaddr,__abs_timeout))
#ifdef __USE_EXCEPT
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait64,(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__abs_timeout),Xfutex_wait64,(__uaddr,__probe_value,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait64_cmpxch,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout),Xfutex_wait64_cmpxch,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait64_cmpxch2,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout),Xfutex_wait64_cmpxch2,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait64_mask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout),Xfutex_wait64_mask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait64_nmask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout),Xfutex_wait64_nmask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait64_bitset,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct __timespec64 const *__abs_timeout),Xfutex_wait64_bitset,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait64_ghost,(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__abs_timeout),Xfutex_wait64_ghost,(__uaddr,__probe_value,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait64_cmpxch_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout),Xfutex_wait64_cmpxch_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait64_cmpxch2_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout),Xfutex_wait64_cmpxch2_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait64_mask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout),Xfutex_wait64_mask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait64_nmask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout),Xfutex_wait64_nmask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_wait64_bitset_ghost,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct __timespec64 const *__abs_timeout),Xfutex_wait64_bitset_ghost,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_waitrel64,(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__rel_timeout),Xfutex_waitrel64,(__uaddr,__probe_value,__rel_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_waitrel64_ghost,(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__rel_timeout),Xfutex_waitrel64_ghost,(__uaddr,__probe_value,__rel_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,__os_Xfutex_lock64,(futex_t *__uaddr, struct __timespec64 const *__abs_timeout),Xfutex_lock64,(__uaddr,__abs_timeout))
#endif /* __USE_EXCEPT */
#endif /* __USE_TIME64 */
#endif /* !__USE_TIME_BITS64 */
#endif



/* FUTEX_WAIT / FUTEX_WAIT_BITSET / FUTEX_WAIT_GHOST / FUTEX_WAIT_BITSET_GHOST:
 * FUTEX_WAIT_MASK / FUTEX_WAIT_MASK_GHOST:
 * @return: __EXCEPT_SELECT(false,-1 && errno == ETIMEDOUT):
 *          Only when `TIMEOUT != NULL': The specified timeout has expired.
 * @return: __EXCEPT_SELECT(true,-1 && errno == EAGAIN):
 *          The wait condition failed during interlocked mode
 * @return: __EXCEPT_SELECT(true,0):
 *          The futex was triggered, and the caller should
 *          re-attempt acquiring the associated lock.
 * @throw:  E_SEGFAULT: The given `UADDR' or `TIMEOUT' are faulty.
 * @throw:  E_BADALLOC: Failed to allocate the futex controller in kernel-space. */
#if !defined(__NO_ATTR_FORCEINLINE) && !defined(__NO_builtin_constant_p)
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait)
(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf(__uaddr,__probe_value),1),
                             __os_futex_wait_inf(__uaddr,__probe_value));
 return __os_futex_wait(__uaddr,__probe_value,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_waitrel)
(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout) {
 if (__builtin_constant_p(__rel_timeout) && __rel_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf(__uaddr,__probe_value),1),
                             __os_futex_wait_inf(__uaddr,__probe_value));
 return __os_futex_waitrel(__uaddr,__probe_value,__rel_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait_cmpxch)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_cmpxch(__uaddr,__old_value,__new_value,__update_target),1),
                             __os_futex_wait_inf_cmpxch(__uaddr,__old_value,__new_value,__update_target));
 return __os_futex_wait_cmpxch(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait_cmpxch2)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_cmpxch2(__uaddr,__old_value,__new_value,__update_target),1),
                             __os_futex_wait_inf_cmpxch2(__uaddr,__old_value,__new_value,__update_target));
 return __os_futex_wait_cmpxch2(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait_mask)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_mask(__uaddr,__probe_mask,__probe_value,__enable_bits),1),
                             __os_futex_wait_inf_mask(__uaddr,__probe_mask,__probe_value,__enable_bits));
 return __os_futex_wait_mask(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait_nmask)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_nmask(__uaddr,__probe_mask,__probe_value,__enable_bits),1),
                             __os_futex_wait_inf_nmask(__uaddr,__probe_mask,__probe_value,__enable_bits));
 return __os_futex_wait_nmask(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait_bitset)
(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__channels) && __channels == FUTEX_BITSET_MATCH_ANY) {
  if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
      return __EXCEPT_SELECT((__os_futex_wait_inf(__uaddr,__probe_value),1),
                              __os_futex_wait_inf(__uaddr,__probe_value));
  return __os_futex_wait(__uaddr,__probe_value,__abs_timeout);
 }
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_bitset(__uaddr,__probe_value,__channels),1),
                             __os_futex_wait_inf_bitset(__uaddr,__probe_value,__channels));
 return __os_futex_wait_bitset(__uaddr,__probe_value,__channels,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait_ghost)
(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_ghost(__uaddr,__probe_value),1),
                             __os_futex_wait_inf_ghost(__uaddr,__probe_value));
 return __os_futex_wait_ghost(__uaddr,__probe_value,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_waitrel_ghost)
(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout) {
 if (__builtin_constant_p(__rel_timeout) && __rel_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_ghost(__uaddr,__probe_value),1),
                             __os_futex_wait_inf_ghost(__uaddr,__probe_value));
 return __os_futex_waitrel_ghost(__uaddr,__probe_value,__rel_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait_cmpxch_ghost)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_cmpxch_ghost(__uaddr,__old_value,__new_value,__update_target),1),
                             __os_futex_wait_inf_cmpxch_ghost(__uaddr,__old_value,__new_value,__update_target));
 return __os_futex_wait_cmpxch_ghost(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait_cmpxch2_ghost)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_cmpxch2_ghost(__uaddr,__old_value,__new_value,__update_target),1),
                             __os_futex_wait_inf_cmpxch2_ghost(__uaddr,__old_value,__new_value,__update_target));
 return __os_futex_wait_cmpxch2_ghost(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait_mask_ghost)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_mask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits),1),
                             __os_futex_wait_inf_mask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits));
 return __os_futex_wait_mask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait_nmask_ghost)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_nmask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits),1),
                             __os_futex_wait_inf_nmask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits));
 return __os_futex_wait_nmask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait_bitset_ghost)
(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__channels) && __channels == FUTEX_BITSET_MATCH_ANY) {
  if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
      return __EXCEPT_SELECT((__os_futex_wait_inf_ghost(__uaddr,__probe_value),1),
                              __os_futex_wait_inf_ghost(__uaddr,__probe_value));
  return __os_futex_wait_ghost(__uaddr,__probe_value,__abs_timeout);
 }
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_bitset_ghost(__uaddr,__probe_value,__channels),1),
                             __os_futex_wait_inf_bitset_ghost(__uaddr,__probe_value,__channels));
 return __os_futex_wait_bitset_ghost(__uaddr,__probe_value,__channels,__abs_timeout);
}
#ifdef __USE_EXCEPT
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait)
(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf(__uaddr,__probe_value),1);
 return __os_Xfutex_wait(__uaddr,__probe_value,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_waitrel)
(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout) {
 if (__builtin_constant_p(__rel_timeout) && __rel_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf(__uaddr,__probe_value),1);
 return __os_Xfutex_waitrel(__uaddr,__probe_value,__rel_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait_cmpxch)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_cmpxch(__uaddr,__old_value,__new_value,__update_target),1);
 return __os_Xfutex_wait_cmpxch(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait_cmpxch2)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_cmpxch2(__uaddr,__old_value,__new_value,__update_target),1);
 return __os_Xfutex_wait_cmpxch2(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait_mask)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_mask(__uaddr,__probe_mask,__probe_value,__enable_bits),1);
 return __os_Xfutex_wait_mask(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait_nmask)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_nmask(__uaddr,__probe_mask,__probe_value,__enable_bits),1);
 return __os_Xfutex_wait_nmask(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait_bitset)
(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__channels) && __channels == FUTEX_BITSET_MATCH_ANY) {
  if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
      return (__os_Xfutex_wait_inf(__uaddr,__probe_value),1);
  return __os_Xfutex_wait(__uaddr,__probe_value,__abs_timeout);
 }
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_bitset(__uaddr,__probe_value,__channels),1);
 return __os_Xfutex_wait_bitset(__uaddr,__probe_value,__channels,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait_ghost)
(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_ghost(__uaddr,__probe_value),1);
 return __os_Xfutex_wait_ghost(__uaddr,__probe_value,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_waitrel_ghost)
(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout) {
 if (__builtin_constant_p(__rel_timeout) && __rel_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_ghost(__uaddr,__probe_value),1);
 return __os_Xfutex_waitrel_ghost(__uaddr,__probe_value,__rel_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait_cmpxch_ghost)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_cmpxch_ghost(__uaddr,__old_value,__new_value,__update_target),1);
 return __os_Xfutex_wait_cmpxch_ghost(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait_cmpxch2_ghost)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_cmpxch2_ghost(__uaddr,__old_value,__new_value,__update_target),1);
 return __os_Xfutex_wait_cmpxch2_ghost(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait_mask_ghost)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_mask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits),1);
 return __os_Xfutex_wait_mask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait_nmask_ghost)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_nmask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits),1);
 return __os_Xfutex_wait_nmask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait_bitset_ghost)
(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__channels) && __channels == FUTEX_BITSET_MATCH_ANY) {
  if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
      return (__os_Xfutex_wait_inf_ghost(__uaddr,__probe_value),1);
  return __os_Xfutex_wait_ghost(__uaddr,__probe_value,__abs_timeout);
 }
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_bitset_ghost(__uaddr,__probe_value,__channels),1);
 return __os_Xfutex_wait_bitset_ghost(__uaddr,__probe_value,__channels,__abs_timeout);
}
#endif /* __USE_EXCEPT */
#ifdef __USE_TIME64
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait64)
(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf(__uaddr,__probe_value),1),
                             __os_futex_wait_inf(__uaddr,__probe_value));
 return __os_futex_wait64(__uaddr,__probe_value,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_waitrel64)
(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__rel_timeout) {
 if (__builtin_constant_p(__rel_timeout) && __rel_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf(__uaddr,__probe_value),1),
                             __os_futex_wait_inf(__uaddr,__probe_value));
 return __os_futex_waitrel64(__uaddr,__probe_value,__rel_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait64_cmpxch)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_cmpxch(__uaddr,__old_value,__new_value,__update_target),1),
                             __os_futex_wait_inf_cmpxch(__uaddr,__old_value,__new_value,__update_target));
 return __os_futex_wait64_cmpxch(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait64_cmpxch2)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_cmpxch2(__uaddr,__old_value,__new_value,__update_target),1),
                             __os_futex_wait_inf_cmpxch2(__uaddr,__old_value,__new_value,__update_target));
 return __os_futex_wait64_cmpxch2(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait64_mask)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_mask(__uaddr,__probe_mask,__probe_value,__enable_bits),1),
                             __os_futex_wait_inf_mask(__uaddr,__probe_mask,__probe_value,__enable_bits));
 return __os_futex_wait64_mask(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait64_nmask)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_nmask(__uaddr,__probe_mask,__probe_value,__enable_bits),1),
                             __os_futex_wait_inf_nmask(__uaddr,__probe_mask,__probe_value,__enable_bits));
 return __os_futex_wait64_nmask(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait64_bitset)
(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__channels) && __channels == FUTEX_BITSET_MATCH_ANY) {
  if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
      return __EXCEPT_SELECT((__os_futex_wait_inf(__uaddr,__probe_value),1),
                              __os_futex_wait_inf(__uaddr,__probe_value));
  return __os_futex_wait64(__uaddr,__probe_value,__abs_timeout);
 }
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_bitset(__uaddr,__probe_value,__channels),1),
                             __os_futex_wait_inf_bitset(__uaddr,__probe_value,__channels));
 return __os_futex_wait64_bitset(__uaddr,__probe_value,__channels,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait64_ghost)
(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_ghost(__uaddr,__probe_value),1),
                             __os_futex_wait_inf_ghost(__uaddr,__probe_value));
 return __os_futex_wait64_ghost(__uaddr,__probe_value,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_waitrel64_ghost)
(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__rel_timeout) {
 if (__builtin_constant_p(__rel_timeout) && __rel_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_ghost(__uaddr,__probe_value),1),
                             __os_futex_wait_inf_ghost(__uaddr,__probe_value));
 return __os_futex_waitrel64_ghost(__uaddr,__probe_value,__rel_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait64_cmpxch_ghost)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_cmpxch_ghost(__uaddr,__old_value,__new_value,__update_target),1),
                             __os_futex_wait_inf_cmpxch_ghost(__uaddr,__old_value,__new_value,__update_target));
 return __os_futex_wait64_cmpxch_ghost(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait64_cmpxch2_ghost)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_cmpxch2_ghost(__uaddr,__old_value,__new_value,__update_target),1),
                             __os_futex_wait_inf_cmpxch2_ghost(__uaddr,__old_value,__new_value,__update_target));
 return __os_futex_wait64_cmpxch2_ghost(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait64_mask_ghost)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_mask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits),1),
                             __os_futex_wait_inf_mask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits));
 return __os_futex_wait64_mask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait64_nmask_ghost)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_nmask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits),1),
                             __os_futex_wait_inf_nmask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits));
 return __os_futex_wait64_nmask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __EXCEPT_SELECT(__BOOL,int) (__LIBCCALL futex_wait64_bitset_ghost)
(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__channels) && __channels == FUTEX_BITSET_MATCH_ANY) {
  if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
      return __EXCEPT_SELECT((__os_futex_wait_inf_ghost(__uaddr,__probe_value),1),
                              __os_futex_wait_inf_ghost(__uaddr,__probe_value));
  return __os_futex_wait64_ghost(__uaddr,__probe_value,__abs_timeout);
 }
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_wait_inf_bitset_ghost(__uaddr,__probe_value,__channels),1),
                             __os_futex_wait_inf_bitset_ghost(__uaddr,__probe_value,__channels));
 return __os_futex_wait64_bitset_ghost(__uaddr,__probe_value,__channels,__abs_timeout);
}
#ifdef __USE_EXCEPT
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait64)
(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf(__uaddr,__probe_value),1);
 return __os_Xfutex_wait64(__uaddr,__probe_value,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_waitrel64)
(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__rel_timeout) {
 if (__builtin_constant_p(__rel_timeout) && __rel_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf(__uaddr,__probe_value),1);
 return __os_Xfutex_waitrel64(__uaddr,__probe_value,__rel_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait64_cmpxch)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_cmpxch(__uaddr,__old_value,__new_value,__update_target),1);
 return __os_Xfutex_wait64_cmpxch(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait64_cmpxch2)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_cmpxch2(__uaddr,__old_value,__new_value,__update_target),1);
 return __os_Xfutex_wait64_cmpxch2(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait64_mask)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_mask(__uaddr,__probe_mask,__probe_value,__enable_bits),1);
 return __os_Xfutex_wait64_mask(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait64_nmask)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_nmask(__uaddr,__probe_mask,__probe_value,__enable_bits),1);
 return __os_Xfutex_wait64_nmask(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait64_bitset)
(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__channels) && __channels == FUTEX_BITSET_MATCH_ANY) {
  if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
      return (__os_Xfutex_wait_inf(__uaddr,__probe_value),1);
  return __os_Xfutex_wait64(__uaddr,__probe_value,__abs_timeout);
 }
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_bitset(__uaddr,__probe_value,__channels),1);
 return __os_Xfutex_wait64_bitset(__uaddr,__probe_value,__channels,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait64_ghost)
(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_ghost(__uaddr,__probe_value),1);
 return __os_Xfutex_wait64_ghost(__uaddr,__probe_value,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_waitrel64_ghost)
(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__rel_timeout) {
 if (__builtin_constant_p(__rel_timeout) && __rel_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_ghost(__uaddr,__probe_value),1);
 return __os_Xfutex_waitrel64_ghost(__uaddr,__probe_value,__rel_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait64_cmpxch_ghost)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_cmpxch_ghost(__uaddr,__old_value,__new_value,__update_target),1);
 return __os_Xfutex_wait64_cmpxch_ghost(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait64_cmpxch2_ghost)
(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_cmpxch2_ghost(__uaddr,__old_value,__new_value,__update_target),1);
 return __os_Xfutex_wait64_cmpxch2_ghost(__uaddr,__old_value,__new_value,__update_target,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait64_mask_ghost)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_mask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits),1);
 return __os_Xfutex_wait64_mask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait64_nmask_ghost)
(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_nmask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits),1);
 return __os_Xfutex_wait64_nmask_ghost(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout);
}
__FORCELOCAL __BOOL (__LIBCCALL Xfutex_wait64_bitset_ghost)
(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__channels) && __channels == FUTEX_BITSET_MATCH_ANY) {
  if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
      return (__os_Xfutex_wait_inf_ghost(__uaddr,__probe_value),1);
  return __os_Xfutex_wait64_ghost(__uaddr,__probe_value,__abs_timeout);
 }
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_wait_inf_bitset_ghost(__uaddr,__probe_value,__channels),1);
 return __os_Xfutex_wait64_bitset_ghost(__uaddr,__probe_value,__channels,__abs_timeout);
}
#endif /* __USE_EXCEPT */
#endif /* __USE_TIME64 */
#else
__REDIRECT_EXCEPT_TM64(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout),(__uaddr,__probe_value,__abs_timeout))
__REDIRECT_EXCEPT_TM64(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_ghost,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout),(__uaddr,__probe_value,__abs_timeout))
__REDIRECT_EXCEPT_TM64(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_waitrel,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout),(__uaddr,__probe_value,__rel_timeout))
__REDIRECT_EXCEPT_TM64(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_waitrel_ghost,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout),(__uaddr,__probe_value,__rel_timeout))
#ifdef __USE_EXCEPT
__REDIRECT_TM64(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_wait,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout),(__uaddr,__probe_value,__abs_timeout))
__REDIRECT_TM64(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_wait_ghost,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__abs_timeout),(__uaddr,__probe_value,__abs_timeout))
__REDIRECT_TM64(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_waitrel,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout),(__uaddr,__probe_value,__rel_timeout))
__REDIRECT_TM64(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_waitrel_ghost,(futex_t *__uaddr, futex_t __probe_value, struct timespec const *__rel_timeout),(__uaddr,__probe_value,__rel_timeout))
#endif /* __USE_EXCEPT */
#ifdef __USE_TIME_BITS64
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_cmpxch,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),futex_wait64_cmpxch,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_cmpxch_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),futex_wait64_cmpxch_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_cmpxch2,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),futex_wait64_cmpxch2,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_cmpxch2_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),futex_wait64_cmpxch2_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_mask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),futex_wait64_mask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_mask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),futex_wait64_mask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_nmask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),futex_wait64_nmask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_nmask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),futex_wait64_nmask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_bitset,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),futex_wait64_bitset,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_bitset_ghost,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),futex_wait64_bitset_ghost,(__uaddr,__probe_value,__channels,__abs_timeout))
#ifdef __USE_EXCEPT
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_wait_cmpxch,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),Xfutex_wait64_cmpxch,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_wait_cmpxch_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),Xfutex_wait64_cmpxch_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_wait_cmpxch2,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),Xfutex_wait64_cmpxch2,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_wait_cmpxch2_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),Xfutex_wait64_cmpxch2_ghost,(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_wait_mask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),Xfutex_wait64_mask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_wait_mask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),Xfutex_wait64_mask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_wait_nmask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),Xfutex_wait64_nmask,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_wait_nmask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),Xfutex_wait64_nmask_ghost,(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_wait_bitset,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),Xfutex_wait64_bitset,(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_wait_bitset_ghost,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),Xfutex_wait64_bitset_ghost,(__uaddr,__probe_value,__channels,__abs_timeout))
#endif /* __USE_EXCEPT */
#else /* __USE_TIME_BITS64 */
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_cmpxch,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_cmpxch_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_cmpxch2,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_cmpxch2_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout),(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_mask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_mask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_nmask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_nmask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout),(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_bitset,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait_bitset_ghost,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout),(__uaddr,__probe_value,__channels,__abs_timeout))
#ifdef __USE_EXCEPT
__LIBC __BOOL (__LIBCCALL Xfutex_wait_cmpxch)(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait_cmpxch_ghost)(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait_cmpxch2)(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait_cmpxch2_ghost)(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct timespec const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait_mask)(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait_mask_ghost)(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait_nmask)(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait_nmask_ghost)(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct timespec const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait_bitset)(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait_bitset_ghost)(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct timespec const *__abs_timeout);
#endif /* __USE_EXCEPT */
#endif /* !__USE_TIME_BITS64 */
#ifdef __USE_TIME64
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait64,(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__abs_timeout),(__uaddr,__probe_value,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait64_cmpxch,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout),(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait64_cmpxch2,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout),(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait64_mask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout),(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait64_nmask,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout),(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait64_bitset,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct __timespec64 const *__abs_timeout),(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait64_ghost,(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__abs_timeout),(__uaddr,__probe_value,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait64_cmpxch_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout),(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait64_cmpxch2_ghost,(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout),(__uaddr,__old_value,__new_value,__update_target,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait64_mask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout),(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait64_nmask_ghost,(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout),(__uaddr,__probe_mask,__probe_value,__enable_bits,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_wait64_bitset_ghost,(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct __timespec64 const *__abs_timeout),(__uaddr,__probe_value,__channels,__abs_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_waitrel64,(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__rel_timeout),(__uaddr,__probe_value,__rel_timeout))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_waitrel64_ghost,(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__rel_timeout),(__uaddr,__probe_value,__rel_timeout))
#ifdef __USE_EXCEPT
__LIBC __BOOL (__LIBCCALL Xfutex_wait64)(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait64_cmpxch)(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait64_cmpxch2)(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait64_mask)(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait64_nmask)(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait64_bitset)(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct __timespec64 const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait64_ghost)(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait64_cmpxch_ghost)(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait64_cmpxch2_ghost)(futex_t *__uaddr, futex_t __old_value, futex_t __new_value, futex_t *__update_target, struct __timespec64 const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait64_mask_ghost)(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait64_nmask_ghost)(futex_t *__uaddr, futex_t __probe_mask, futex_t __probe_value, futex_t __enable_bits, struct __timespec64 const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_wait64_bitset_ghost)(futex_t *__uaddr, futex_t __probe_value, futex_channel_t __channels, struct __timespec64 const *__abs_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_waitrel64)(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__rel_timeout);
__LIBC __BOOL (__LIBCCALL Xfutex_waitrel64_ghost)(futex_t *__uaddr, futex_t __probe_value, struct __timespec64 const *__rel_timeout);
#endif /* __USE_EXCEPT */
#endif /* __USE_TIME64 */
#endif


/* FUTEX_WAKE / FUTEX_WAKE_BITSET:
 * @return: * :        The number of non-ghosting threads woken.
 * @throw: E_SEGFAULT: The given `UADDR' or `TIMEOUT' are faulty. */
#if 1 /* Because it usually appears in something like mutex_put(), which in itself
       * usually appears within a FINALLY-block, futex_wake() is considered a cleanup
       * function. */
__LIBC __CLEANUP __ssize_t (__LIBCCALL futex_wake)(futex_t *__uaddr, __size_t __num_threads);
__LIBC __CLEANUP __ssize_t (__LIBCCALL futex_wake_bitset)(futex_t *__uaddr, __size_t __num_threads, futex_channel_t __channels);
#else
__REDIRECT_EXCEPT(__LIBC,,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,futex_wake,(futex_t *__uaddr, __size_t __num_threads),(__uaddr,__num_threads))
__REDIRECT_EXCEPT(__LIBC,,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,futex_wake_bitset,(futex_t *__uaddr, __size_t __num_threads, futex_channel_t __channels),(__uaddr,__num_threads,__channels))
#endif
#ifdef __USE_EXCEPT
__LIBC __size_t (__LIBCCALL Xfutex_wake)(futex_t *__uaddr, __size_t __num_threads);
__LIBC __size_t (__LIBCCALL Xfutex_wake_bitset)(futex_t *__uaddr, __size_t __num_threads, futex_channel_t __channels);
#endif /* __USE_EXCEPT */




/* FUTEX_LOCK_PI:
 * @return: __EXCEPT_SELECT(false,-1 && errno == ETIMEDOUT):
 *          Only when `TIMEOUT != NULL': The specified timeout has expired.
 * @return: __EXCEPT_SELECT(true,0):
 *          The calling thread is now holding the lock.
 * @throw:  E_SEGFAULT: The given `UADDR' or `TIMEOUT' are faulty.
 * @throw:  E_BADALLOC: Failed to allocate the futex controller in kernel-space. */
#if !defined(__NO_ATTR_FORCEINLINE) && !defined(__NO_builtin_constant_p)
__FORCELOCAL __WUNUSED __EXCEPT_SELECT(__BOOL,int)
(__LIBCCALL futex_lock)(futex_t *__uaddr, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_lock_inf(__uaddr),1),
                             __os_futex_lock_inf(__uaddr));
 return __os_futex_lock(__uaddr,__abs_timeout);
}
#ifdef __USE_EXCEPT
__FORCELOCAL __WUNUSED __BOOL
(__LIBCCALL Xfutex_lock)(futex_t *__uaddr, struct timespec const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_lock_inf(__uaddr),1);
 return __os_Xfutex_lock(__uaddr,__abs_timeout);
}
#endif /* __USE_EXCEPT */
#ifdef __USE_TIME64
__FORCELOCAL __WUNUSED __EXCEPT_SELECT(__BOOL,int)
(__LIBCCALL futex_lock64)(futex_t *__uaddr, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return __EXCEPT_SELECT((__os_futex_lock_inf(__uaddr),1),
                             __os_futex_lock_inf(__uaddr));
 return __os_futex_lock64(__uaddr,__abs_timeout);
}
#ifdef __USE_EXCEPT
__FORCELOCAL __WUNUSED __BOOL
(__LIBCCALL Xfutex_lock64)(futex_t *__uaddr, struct __timespec64 const *__abs_timeout) {
 if (__builtin_constant_p(__abs_timeout) && __abs_timeout == __NULLPTR)
     return (__os_Xfutex_lock_inf(__uaddr),1);
 return __os_Xfutex_lock64(__uaddr,__abs_timeout);
}
#endif /* __USE_EXCEPT */
#endif /* __USE_TIME64 */
#else
__REDIRECT_EXCEPT_TM64(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_lock,(futex_t *__uaddr, struct timespec const *__abs_timeout),(__uaddr,__abs_timeout))
#ifdef __USE_EXCEPT
__REDIRECT_TM64(__LIBC,__WUNUSED,__BOOL,__LIBCCALL,Xfutex_lock,(futex_t *__uaddr, struct timespec const *__abs_timeout),(__uaddr,__abs_timeout))
#endif /* __USE_EXCEPT */
#ifdef __USE_TIME64
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__BOOL,int),__LIBCCALL,futex_lock64,(futex_t *__uaddr, struct __timespec64 const *__abs_timeout),(__uaddr,__abs_timeout))
#ifdef __USE_EXCEPT
__LIBC __WUNUSED __BOOL (__LIBCCALL Xfutex_lock64)(futex_t *__uaddr, struct __timespec64 const *__abs_timeout);
#endif /* __USE_EXCEPT */
#endif /* __USE_TIME64 */
#endif

/* FUTEX_UNLOCK_PI:
 * @return: 0 : Successfully unlocked the futex, but no waiter was signaled
 * @return: 1 : Successfully unlocked the futex, and a waiter was signaled
 * @throw: E_ILLEGAL_OPERATION: The calling thread wasn't holding a lock to this futex.
 * @throw: E_SEGFAULT:          The given `UADDR' is faulty. */
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,futex_unlock,(futex_t *__uaddr),(__uaddr))
#ifdef __USE_EXCEPT
__LIBC __WUNUSED __size_t (__LIBCCALL Xfutex_unlock)(futex_t *__uaddr);
#endif



/* FUTEX_WAITFD / FUTEX_WAITFD_BITSET:
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
 * @param: UADDR:       The address of the futex to wait for, as well as the address to which to bind the file handle.
 * @param: PROBE_VALUE: The value to probe `UADDR' for, during a poll() or read() operation
 * @param: OFLAGS:      A set of `O_ACCMODE' and `O_CLOEXEC|O_CLOFORK|O_NONBLOCK' applied to the new file descriptor.
 *               NOTE: `O_ACCMODE' specifies if `read()' and `write()' can be used on the handle.
 *                      It however does not affect `poll()', which is always allowed.
 * @param: CHANNELS:    The futex channel mask to which to connect during poll() or read()
 *                NOTE: During a poll(), the channel mask might interact with other
 *                      file descriptors being polled, potentially causing sporadic
 *                      wake-ups when the states of other, unrelated file descriptors
 *                      change.
 *                      Because of this, you should assume that including a futex
 *                      handle in a poll() system call may lead to sporadic wake ups.
 *                     (Although due to the nature of poll(), the possibility of this
 *                      happening shouldn't require any changes to existing code)
 * @throw: E_INVALID_ARGUMENT:                The given `OFLAGS' is invalid.
 * @throw: E_BADALLOC.ERROR_BADALLOC_*MEMORY: Failed to allocate the futex, or its handle descriptor.
 * @throw: E_BADALLOC.ERROR_BADALLOC_HANDLE:  Too many open handles. */
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__fd_t,__LIBCCALL,futex_waitfd,(futex_t *__uaddr, futex_t __probe_value, __oflag_t __oflags),(__uaddr,__probe_value,__oflags))
#ifdef __USE_EXCEPT
__LIBC __WUNUSED __fd_t (__LIBCCALL Xfutex_waitfd)(futex_t *__uaddr, futex_t __probe_value, __oflag_t __oflags);
#endif
#if !defined(__NO_ATTR_FORCEINLINE) && !defined(__NO_builtin_constant_p)
__REDIRECT_EXCEPT_(__LIBC,__WUNUSED,__fd_t,__LIBCCALL,__os_futex_waitfd_bitset,(futex_t *__uaddr, futex_t __probe_value, __oflag_t __oflags, futex_channel_t __channels),futex_waitfd_bitset,(__uaddr,__probe_value,__oflags,__channels))
__FORCELOCAL __WUNUSED __fd_t (__LIBCCALL futex_waitfd_bitset)(futex_t *__uaddr, futex_t __probe_value, __oflag_t __oflags, futex_channel_t __channels) {
 if (__builtin_constant_p(__channels) && __channels == FUTEX_BITSET_MATCH_ANY)
     return futex_waitfd(__uaddr,__probe_value,__oflags);
 return __os_futex_waitfd_bitset(__uaddr,__probe_value,__oflags,__channels);
}
#ifdef __USE_EXCEPT
__REDIRECT(__LIBC,__WUNUSED,__fd_t,__LIBCCALL,__os_Xfutex_waitfd_bitset,(futex_t *__uaddr, futex_t __probe_value, __oflag_t __oflags, futex_channel_t __channels),Xfutex_waitfd_bitset,(__uaddr,__probe_value,__oflags,__channels))
__FORCELOCAL __WUNUSED __fd_t (__LIBCCALL Xfutex_waitfd_bitset)(futex_t *__uaddr, futex_t __probe_value, __oflag_t __oflags, futex_channel_t __channels) {
 if (__builtin_constant_p(__channels) && __channels == FUTEX_BITSET_MATCH_ANY)
     return Xfutex_waitfd(__uaddr,__probe_value,__oflags);
 return __os_Xfutex_waitfd_bitset(__uaddr,__probe_value,__oflags,__channels);
}
#endif
#else
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__fd_t,__LIBCCALL,futex_waitfd_bitset,(futex_t *__uaddr, futex_t __probe_value, __oflag_t __oflags, futex_channel_t __channels),(__uaddr,__probe_value,__oflags,__channels))
#ifdef __USE_EXCEPT
__LIBC __WUNUSED __fd_t (__LIBCCALL Xfutex_waitfd_bitset)(futex_t *__uaddr, futex_t __probe_value, __oflag_t __oflags, futex_channel_t __channels);
#endif
#endif








/* Low-level futex() system call interface.
 * NOTE: For the purposes of type safety and binary image size, it
 *       is recommended to use the wrapper functions above, rather
 *       than these functions. */
__REDIRECT_EXCEPT_TM64(__LIBC,,__syscall_slong_t,__LIBCCALL,futex,
                      (futex_t *__uaddr, int __futex_op, __UINTPTR_TYPE__ __val,
                       struct timespec const *__timeout /* or: __UINTPTR_TYPE__ __val2 */,
                       futex_t *__uaddr2, __UINTPTR_TYPE__ __val3),
                      (__uaddr,__futex_op,__val,__timeout,__uaddr2,__val3))
#ifdef __USE_TIME64
__REDIRECT_EXCEPT(__LIBC,,__syscall_slong_t,__LIBCCALL,futex64,
                 (futex_t *__uaddr, int __futex_op, __UINTPTR_TYPE__ __val,
                  struct __timespec64 const *__timeout /* or: __UINTPTR_TYPE__ __val2 */,
                  futex_t *__uaddr2, __UINTPTR_TYPE__ __val3),
                 (__uaddr,__futex_op,__val,__timeout,__uaddr2,__val3))
#endif
__REDIRECT_EXCEPT_(__LIBC,,__syscall_slong_t,__LIBCCALL,futexv,
                  (futex_t *__uaddr, int __futex_op, __UINTPTR_TYPE__ __val,
                   __UINTPTR_TYPE__ __val2, futex_t *__uaddr2, __UINTPTR_TYPE__ __val3),
                   futex64,(__uaddr,__futex_op,__val,__timeout,__uaddr2,__val3));
#ifdef __USE_EXCEPT
__REDIRECT_TM64(__LIBC,,__syscall_slong_t,__LIBCCALL,Xfutex,
               (futex_t *__uaddr, int __futex_op, __UINTPTR_TYPE__ __val,
                struct timespec const *__timeout /* or: __UINTPTR_TYPE__ __val2 */,
                futex_t *__uaddr2, __UINTPTR_TYPE__ __val3),
               (__uaddr,__futex_op,__val,__timeout,__uaddr2,__val3))
#ifdef __USE_TIME64
__LIBC __syscall_slong_t (__LIBCCALL Xfutex64)(futex_t *__uaddr, int __futex_op, __UINTPTR_TYPE__ __val,
                                               struct __timespec64 const *__timeout /* or: __UINTPTR_TYPE__ __val2 */,
                                               futex_t *__uaddr2, __UINTPTR_TYPE__ __val3);
#endif
__REDIRECT(__LIBC,,__syscall_slong_t,__LIBCCALL,Xfutexv,
          (futex_t *__uaddr, int __futex_op, __UINTPTR_TYPE__ __val,
           __UINTPTR_TYPE__ __val2, futex_t *__uaddr2, __UINT32_TYPE__ __val3),
           Xfutex64,(__uaddr,__futex_op,__val,__timeout,__uaddr2,__val3));
#endif /* __USE_EXCEPT */


struct task_segment;
/* Basic thread-local context control functions. */
#ifdef __BUILDING_LIBC
__REDIRECT(__LIBC,__WUNUSED __ATTR_CONST __ATTR_NOTHROW,__pid_t,__LIBCCALL,__gettid,(void),libc_gettid,())
__REDIRECT(__LIBC,__WUNUSED __ATTR_CONST __ATTR_NOTHROW,__pid_t,__LIBCCALL,gettid,(void),libc_gettid,())
__REDIRECT(__LIBC,__WUNUSED __ATTR_RETNONNULL __ATTR_CONST __ATTR_NOTHROW,struct task_segment *,__LIBCCALL,__current,(void),libc_current,())
__REDIRECT(__LIBC,__ATTR_NOTHROW,int,__LIBCCALL,__sched_yield,(void),libc_sched_yield,())
#else
/* Return the TID of the calling thread, as read from `__current()->ts_tid' */
__REDIRECT(__LIBC,__WUNUSED __ATTR_CONST __ATTR_NOTHROW,__pid_t,__LIBCCALL,__gettid,(void),gettid,())
__LIBC __WUNUSED __ATTR_CONST __ATTR_NOTHROW __pid_t (__LIBCCALL gettid)(void);
/* Return a pointer to the calling thread's task segment, that is a block of memory
 * mapped by the kernel, often addressable by some means using an arch-specific TLS register.
 * On X86, the %fs / %gs segment registers are used for this. */
__LIBC __WUNUSED __ATTR_RETNONNULL __ATTR_CONST __ATTR_NOTHROW struct task_segment *(__LIBCCALL __current)(void);
/* Yield the remainder of the calling thread's current quantum to
 * allow some other thread on the same cpu to start working prematurely.
 * As far as semantics go, this function should be called in the same places
 * as the X86 `pause' instruction would usually be placed, that is wherever
 * some piece of code has entered an IDLE-loop in an attempt to acquire a
 * lock currently held by some other thread (s.a. `hybrid/sync/atomic-rwlock.h') */
__REDIRECT(__LIBC,__ATTR_NOTHROW,int,__LIBCCALL,__sched_yield,(void),sched_yield,())
#endif


/* Configuration option for standard synchronization primitives.
 * Before connecting to a signal, try to yield a couple of times
 * to try and get other threads to release some kind of lock,
 * as `sched_yield()' is a much faster operation than full scheduling.
 * Doing this may improve performance, especially on single-core machines. */
#ifndef CONFIG_YIELD_BEFORE_CONNECT
#ifndef CONFIG_NO_YIELD_BEFORE_CONNECT
#define CONFIG_YIELD_BEFORE_CONNECT  4
#endif
#elif defined(CONFIG_NO_YIELD_BEFORE_CONNECT)
#undef CONFIG_YIELD_BEFORE_CONNECT
#endif

#ifdef CONFIG_YIELD_BEFORE_CONNECT
#if (CONFIG_YIELD_BEFORE_CONNECT+0) == 0
#undef CONFIG_YIELD_BEFORE_CONNECT
#define THREAD_POLL_BEFORE_CONNECT(...) (void)0
#elif CONFIG_YIELD_BEFORE_CONNECT == 1
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ if (__sched_yield()) break; __VA_ARGS__; }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 2
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
   }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 3
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
   }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 4
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
   }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 5
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
   }__WHILE0
#elif CONFIG_YIELD_BEFORE_CONNECT == 6
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
       if (__sched_yield()) break; __VA_ARGS__; \
   }__WHILE0
#else
#define THREAD_POLL_BEFORE_CONNECT(...) \
   do{ unsigned int __poll_count = CONFIG_YIELD_BEFORE_CONNECT; \
       do { \
           if (__sched_yield()) break; \
           __VA_ARGS__; \
       } while (--__poll_count); \
   }__WHILE0
#endif
#else
#define THREAD_POLL_BEFORE_CONNECT(...) (void)0
#endif

#endif
#endif /* __CC__ */


DECL_END


#endif /* !_KOS_FUTEX_H */
