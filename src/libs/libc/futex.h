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
#ifndef GUARD_LIBS_LIBC_FUTEX_H
#define GUARD_LIBS_LIBC_FUTEX_H 1

#include "libc.h"
#include <kos/types.h>
#include <kos/futex.h>
#include <signal.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     FUTEX                                                                             */
/* ===================================================================================== */
INTDEF syscall_slong_t LIBCCALL libc_futex(u32 *uaddr, int futex_op, uintptr_t val, struct timespec32 const *timeout, u32 *uaddr2, uintptr_t val3);
INTDEF syscall_slong_t LIBCCALL libc_futex64(u32 *uaddr, int futex_op, uintptr_t val, struct timespec64 const *timeout, u32 *uaddr2, uintptr_t val3);
INTDEF syscall_slong_t LIBCCALL libc_Xfutex(u32 *uaddr, int futex_op, uintptr_t val, struct timespec32 const *timeout, u32 *uaddr2, uintptr_t val3);
INTDEF syscall_slong_t LIBCCALL libc_Xfutex64(u32 *uaddr, int futex_op, uintptr_t val, struct timespec64 const *timeout, u32 *uaddr2, uintptr_t val3);

INTDEF int LIBCCALL libc_futex_wait_inf(futex_t *uaddr, futex_t probe_value);
INTDEF int LIBCCALL libc_futex_wait_inf_mask(futex_t *uaddr, futex_t probe_mask, futex_t probe_value, futex_t enable_bits);
INTDEF int LIBCCALL libc_futex_wait_inf_bitset(futex_t *uaddr, futex_t probe_value, futex_channel_t channels);
INTDEF int LIBCCALL libc_futex_wait(futex_t *uaddr, futex_t probe_value, struct timespec32 const *abs_timeout);
INTDEF int LIBCCALL libc_futex_wait_mask(futex_t *uaddr, futex_t probe_mask, futex_t probe_value, futex_t enable_bits, struct timespec32 const *abs_timeout);
INTDEF int LIBCCALL libc_futex_wait_bitset(futex_t *uaddr, futex_t probe_value, futex_channel_t channels, struct timespec32 const *abs_timeout);
INTDEF int LIBCCALL libc_futex_wait64(futex_t *uaddr, futex_t probe_value, struct timespec64 const *abs_timeout);
INTDEF int LIBCCALL libc_futex_wait64_mask(futex_t *uaddr, futex_t probe_mask, futex_t probe_value, futex_t enable_bits, struct timespec64 const *abs_timeout);
INTDEF int LIBCCALL libc_futex_wait64_bitset(futex_t *uaddr, futex_t probe_value, futex_channel_t channels, struct timespec64 const *abs_timeout);
INTDEF int LIBCCALL libc_futex_waitrel(futex_t *uaddr, futex_t probe_value, struct timespec32 const *rel_timeout);
INTDEF int LIBCCALL libc_futex_waitrel64(futex_t *uaddr, futex_t probe_value, struct timespec64 const *rel_timeout);
INTDEF void LIBCCALL libc_Xfutex_wait_inf(futex_t *uaddr, futex_t probe_value);
INTDEF void LIBCCALL libc_Xfutex_wait_inf_mask(futex_t *uaddr, futex_t probe_mask, futex_t probe_value, futex_t enable_bits);
INTDEF void LIBCCALL libc_Xfutex_wait_inf_bitset(futex_t *uaddr, futex_t probe_value, futex_channel_t channels);
INTDEF bool LIBCCALL libc_Xfutex_wait(futex_t *uaddr, futex_t probe_value, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_Xfutex_wait_mask(futex_t *uaddr, futex_t probe_mask, futex_t probe_value, futex_t enable_bits, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_Xfutex_wait_bitset(futex_t *uaddr, futex_t probe_value, futex_channel_t channels, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_Xfutex_wait64(futex_t *uaddr, futex_t probe_value, struct timespec64 const *abs_timeout);
INTDEF bool LIBCCALL libc_Xfutex_wait64_mask(futex_t *uaddr, futex_t probe_mask, futex_t probe_value, futex_t enable_bits, struct timespec64 const *abs_timeout);
INTDEF bool LIBCCALL libc_Xfutex_wait64_bitset(futex_t *uaddr, futex_t probe_value, futex_channel_t channels, struct timespec64 const *abs_timeout);
INTDEF bool LIBCCALL libc_Xfutex_waitrel(futex_t *uaddr, futex_t probe_value, struct timespec32 const *rel_timeout);
INTDEF bool LIBCCALL libc_Xfutex_waitrel64(futex_t *uaddr, futex_t probe_value, struct timespec64 const *rel_timeout);

INTDEF int LIBCCALL libc_futex_wait_inf_ghost(futex_t *uaddr, futex_t probe_value);
INTDEF int LIBCCALL libc_futex_wait_inf_mask_ghost(futex_t *uaddr, futex_t probe_mask, futex_t probe_value, futex_t enable_bits);
INTDEF int LIBCCALL libc_futex_wait_inf_bitset_ghost(futex_t *uaddr, futex_t probe_value, futex_channel_t channels);
INTDEF int LIBCCALL libc_futex_wait_ghost(futex_t *uaddr, futex_t probe_value, struct timespec32 const *abs_timeout);
INTDEF int LIBCCALL libc_futex_wait_mask_ghost(futex_t *uaddr, futex_t probe_mask, futex_t probe_value, futex_t enable_bits, struct timespec32 const *abs_timeout);
INTDEF int LIBCCALL libc_futex_wait_bitset_ghost(futex_t *uaddr, futex_t probe_value, futex_channel_t channels, struct timespec32 const *abs_timeout);
INTDEF int LIBCCALL libc_futex_wait64_ghost(futex_t *uaddr, futex_t probe_value, struct timespec64 const *abs_timeout);
INTDEF int LIBCCALL libc_futex_wait64_mask_ghost(futex_t *uaddr, futex_t probe_mask, futex_t probe_value, futex_t enable_bits, struct timespec64 const *abs_timeout);
INTDEF int LIBCCALL libc_futex_wait64_bitset_ghost(futex_t *uaddr, futex_t probe_value, futex_channel_t channels, struct timespec64 const *abs_timeout);
INTDEF int LIBCCALL libc_futex_waitrel_ghost(futex_t *uaddr, futex_t probe_value, struct timespec32 const *rel_timeout);
INTDEF int LIBCCALL libc_futex_waitrel64_ghost(futex_t *uaddr, futex_t probe_value, struct timespec64 const *rel_timeout);
INTDEF void LIBCCALL libc_Xfutex_wait_inf_ghost(futex_t *uaddr, futex_t probe_value);
INTDEF void LIBCCALL libc_Xfutex_wait_inf_mask_ghost(futex_t *uaddr, futex_t probe_mask, futex_t probe_value, futex_t enable_bits);
INTDEF void LIBCCALL libc_Xfutex_wait_inf_bitset_ghost(futex_t *uaddr, futex_t probe_value, futex_channel_t channels);
INTDEF bool LIBCCALL libc_Xfutex_wait_ghost(futex_t *uaddr, futex_t probe_value, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_Xfutex_wait_mask_ghost(futex_t *uaddr, futex_t probe_mask, futex_t probe_value, futex_t enable_bits, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_Xfutex_wait_bitset_ghost(futex_t *uaddr, futex_t probe_value, futex_channel_t channels, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_Xfutex_wait64_ghost(futex_t *uaddr, futex_t probe_value, struct timespec64 const *abs_timeout);
INTDEF bool LIBCCALL libc_Xfutex_wait64_mask_ghost(futex_t *uaddr, futex_t probe_mask, futex_t probe_value, futex_t enable_bits, struct timespec64 const *abs_timeout);
INTDEF bool LIBCCALL libc_Xfutex_wait64_bitset_ghost(futex_t *uaddr, futex_t probe_value, futex_channel_t channels, struct timespec64 const *abs_timeout);
INTDEF bool LIBCCALL libc_Xfutex_waitrel_ghost(futex_t *uaddr, futex_t probe_value, struct timespec32 const *rel_timeout);
INTDEF bool LIBCCALL libc_Xfutex_waitrel64_ghost(futex_t *uaddr, futex_t probe_value, struct timespec64 const *rel_timeout);

INTDEF ssize_t LIBCCALL libc_futex_wake(futex_t *uaddr, size_t num_threads);
INTDEF ssize_t LIBCCALL libc_futex_wake_bitset(futex_t *uaddr, size_t num_threads, futex_channel_t channels);
INTDEF size_t LIBCCALL libc_Xfutex_wake(futex_t *uaddr, size_t num_threads);
INTDEF size_t LIBCCALL libc_Xfutex_wake_bitset(futex_t *uaddr, size_t num_threads, futex_channel_t channels);

INTDEF int LIBCCALL libc_futex_lock_inf(futex_t *uaddr);
INTDEF int LIBCCALL libc_futex_lock(futex_t *uaddr, struct timespec32 const *abs_timeout);
INTDEF int LIBCCALL libc_futex_lock64(futex_t *uaddr, struct timespec64 const *abs_timeout);
INTDEF ssize_t LIBCCALL libc_futex_unlock(futex_t *uaddr);
INTDEF void LIBCCALL libc_Xfutex_lock_inf(futex_t *uaddr);
INTDEF bool LIBCCALL libc_Xfutex_lock(futex_t *uaddr, struct timespec32 const *abs_timeout);
INTDEF bool LIBCCALL libc_Xfutex_lock64(futex_t *uaddr, struct timespec64 const *abs_timeout);
INTDEF size_t LIBCCALL libc_Xfutex_unlock(futex_t *uaddr);

INTDEF fd_t LIBCCALL libc_futex_waitfd(futex_t *uaddr, futex_t probe_value, oflag_t oflags);
INTDEF fd_t LIBCCALL libc_futex_waitfd_bitset(futex_t *uaddr, futex_t probe_value, oflag_t oflags, futex_channel_t channels);
INTDEF fd_t LIBCCALL libc_Xfutex_waitfd(futex_t *uaddr, futex_t probe_value, oflag_t oflags);
INTDEF fd_t LIBCCALL libc_Xfutex_waitfd_bitset(futex_t *uaddr, futex_t probe_value, oflag_t oflags, futex_channel_t channels);


/* Advanced ppoll() with integration for polling futex objects. */
struct pollfd;
struct pollfutex;
INTDEF ssize_t LIBCCALL libc_xppoll(struct pollfd *ufds, size_t nfds, struct pollfutex *uftx, size_t nftx, struct timespec const *tsp, sigset_t *sig);
INTDEF ssize_t LIBCCALL libc_xppoll64(struct pollfd *ufds, size_t nfds, struct pollfutex *uftx, size_t nftx, struct timespec64 const *tsp, sigset_t *sig);
INTDEF size_t LIBCCALL libc_Xxppoll(struct pollfd *ufds, size_t nfds, struct pollfutex *uftx, size_t nftx, struct timespec const *tsp, sigset_t *sig);
INTDEF size_t LIBCCALL libc_Xxppoll64(struct pollfd *ufds, size_t nfds, struct pollfutex *uftx, size_t nftx, struct timespec64 const *tsp, sigset_t *sig);



DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_FUTEX_H */
