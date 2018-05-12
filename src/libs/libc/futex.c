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
#ifndef GUARD_LIBS_LIBC_FUTEX_C
#define GUARD_LIBS_LIBC_FUTEX_C 1
#define __EXPOSE_CPU_CONTEXT 1

#include "libc.h"
#include "futex.h"
#include "system.h"
#include <errno.h>
#include <linux/futex.h>
#include <kos/futex.h>

DECL_BEGIN

#define FUTEX_FOREACH_CASE_TIMEOUT \
    case FUTEX_WAIT: \
    case FUTEX_WAIT_GHOST: \
    case FUTEX_WAIT_MASK: \
    case FUTEX_WAIT_MASK_GHOST: \
    case FUTEX_WAIT_CMPXCH: \
    case FUTEX_WAIT_CMPXCH_GHOST: \
    case FUTEX_WAIT_CMPXCH2: \
    case FUTEX_WAIT_CMPXCH2_GHOST: \
    case FUTEX_WAIT_BITSET: \
    case FUTEX_WAIT_BITSET_GHOST: \
    case FUTEX_LOCK_PI: \
    case FUTEX_WAIT_REQUEUE_PI: \
/**/


EXPORT(futex,libc_futex);
INTERN syscall_slong_t LIBCCALL
libc_futex(u32 *uaddr, int futex_op, uintptr_t val,
           struct timespec32 const *timeout,
           u32 *uaddr2, uintptr_t val3) {
 struct timespec64 t64;
 switch (futex_op & FUTEX_CMD_MASK) {

 FUTEX_FOREACH_CASE_TIMEOUT
  break;

 default:
direct_64:
  return libc_futex64(uaddr,futex_op,val,
                     (struct timespec64 *)timeout,
                      uaddr2,val3);
 }
 if (!timeout) goto direct_64;
 LIBC_TRY {
  t64.tv_sec  = timeout->tv_sec;
  t64.tv_nsec = timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex64(uaddr,futex_op,val,&t64,uaddr2,val3);
}


EXPORT(Xfutex,libc_Xfutex);
CRT_EXCEPT syscall_slong_t LIBCCALL
libc_Xfutex(u32 *uaddr, int futex_op, uintptr_t val,
            struct timespec32 const *timeout,
            u32 *uaddr2, uintptr_t val3) {
 struct timespec64 t64;
 switch (futex_op & FUTEX_CMD_MASK) {

 FUTEX_FOREACH_CASE_TIMEOUT
  break;

 default:
direct_64:
  return libc_Xfutex64(uaddr,futex_op,val,
                      (struct timespec64 *)timeout,
                       uaddr2,val3);
 }
 if (!timeout) goto direct_64;
 t64.tv_sec  = timeout->tv_sec;
 t64.tv_nsec = timeout->tv_nsec;
 return libc_Xfutex64(uaddr,futex_op,val,&t64,uaddr2,val3);
}



EXPORT(futex_wait_inf,libc_futex_wait_inf);
INTERN int LIBCCALL
libc_futex_wait_inf(futex_t *uaddr, futex_t probe_value) {
 return libc_futex64(uaddr,FUTEX_WAIT,probe_value,NULL,NULL,0);
}

EXPORT(futex_wait_inf_mask,libc_futex_wait_inf_mask);
INTERN int LIBCCALL
libc_futex_wait_inf_mask(futex_t *uaddr, futex_t probe_mask,
                         futex_t probe_value, futex_t enable_bits) {
 return libc_futex64(uaddr,FUTEX_WAIT_MASK,probe_mask,NULL,
                    (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(futex_wait_inf_bitset,libc_futex_wait_inf_bitset);
INTERN int LIBCCALL
libc_futex_wait_inf_bitset(futex_t *uaddr, futex_t probe_value,
                           futex_channel_t channels) {
 return libc_futex64(uaddr,FUTEX_WAIT_BITSET,probe_value,NULL,NULL,channels);
}

EXPORT(futex_wait,libc_futex_wait);
INTERN int LIBCCALL
libc_futex_wait(futex_t *uaddr, futex_t probe_value,
                struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf(uaddr,probe_value);
 LIBC_TRY {
  t64.tv_sec  = abs_timeout->tv_sec;
  t64.tv_nsec = abs_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64(uaddr,probe_value,&t64);
}

EXPORT(futex_wait_mask,libc_futex_wait_mask);
INTERN int LIBCCALL
libc_futex_wait_mask(futex_t *uaddr, futex_t probe_mask,
                     futex_t probe_value, futex_t enable_bits,
                     struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_mask(uaddr,probe_mask,probe_value,enable_bits);
 LIBC_TRY {
  t64.tv_sec  = abs_timeout->tv_sec;
  t64.tv_nsec = abs_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64_mask(uaddr,probe_mask,probe_value,enable_bits,&t64);
}

EXPORT(futex_wait_bitset,libc_futex_wait_bitset);
INTERN int LIBCCALL
libc_futex_wait_bitset(futex_t *uaddr, futex_t probe_value,
                       futex_channel_t channels,
                       struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_bitset(uaddr,probe_value,channels);
 LIBC_TRY {
  t64.tv_sec  = abs_timeout->tv_sec;
  t64.tv_nsec = abs_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64_bitset(uaddr,probe_value,channels,&t64);
}

EXPORT(futex_wait64,libc_futex_wait64);
INTERN int LIBCCALL
libc_futex_wait64(futex_t *uaddr, futex_t probe_value,
                  struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_BITSET,probe_value,
                     abs_timeout,NULL,FUTEX_BITSET_MATCH_ANY);
}

EXPORT(futex_wait64_mask,libc_futex_wait64_mask);
INTERN int LIBCCALL
libc_futex_wait64_mask(futex_t *uaddr, futex_t probe_mask,
                       futex_t probe_value, futex_t enable_bits,
                       struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_MASK,probe_mask,abs_timeout,
                    (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(futex_wait64_bitset,libc_futex_wait64_bitset);
INTERN int LIBCCALL
libc_futex_wait64_bitset(futex_t *uaddr, futex_t probe_value,
                         futex_channel_t channels,
                         struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_BITSET,probe_value,
                     abs_timeout,NULL,channels);
}

EXPORT(futex_waitrel,libc_futex_waitrel);
INTERN int LIBCCALL
libc_futex_waitrel(futex_t *uaddr, futex_t probe_value,
                   struct timespec32 const *rel_timeout) {
 struct timespec64 t64;
 if (!rel_timeout)
      return libc_futex_wait_inf(uaddr,probe_value);
 LIBC_TRY {
  t64.tv_sec  = rel_timeout->tv_sec;
  t64.tv_nsec = rel_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64(uaddr,probe_value,&t64);
}

EXPORT(futex_waitrel64,libc_futex_waitrel64);
INTERN int LIBCCALL
libc_futex_waitrel64(futex_t *uaddr, futex_t probe_value,
                     struct timespec64 const *rel_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT,probe_value,rel_timeout,NULL,0);
}


EXPORT(futex_wait_inf_ghost,libc_futex_wait_inf_ghost);
INTERN int LIBCCALL
libc_futex_wait_inf_ghost(futex_t *uaddr, futex_t probe_value) {
 return libc_futex64(uaddr,FUTEX_WAIT_GHOST,probe_value,NULL,NULL,0);
}

EXPORT(futex_wait_inf_mask_ghost,libc_futex_wait_inf_mask_ghost);
INTERN int LIBCCALL
libc_futex_wait_inf_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                               futex_t probe_value, futex_t enable_bits) {
 return libc_futex64(uaddr,FUTEX_WAIT_MASK_GHOST,probe_mask,NULL,
                    (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(futex_wait_inf_bitset_ghost,libc_futex_wait_inf_bitset_ghost);
INTERN int LIBCCALL
libc_futex_wait_inf_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                                 futex_channel_t channels) {
 return libc_futex64(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,NULL,NULL,channels);
}

EXPORT(futex_wait_ghost,libc_futex_wait_ghost);
INTERN int LIBCCALL
libc_futex_wait_ghost(futex_t *uaddr, futex_t probe_value,
                      struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_ghost(uaddr,probe_value);
 LIBC_TRY {
  t64.tv_sec  = abs_timeout->tv_sec;
  t64.tv_nsec = abs_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64_ghost(uaddr,probe_value,&t64);
}

EXPORT(futex_wait_mask_ghost,libc_futex_wait_mask_ghost);
INTERN int LIBCCALL
libc_futex_wait_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                           futex_t probe_value, futex_t enable_bits,
                           struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_mask_ghost(uaddr,probe_mask,probe_value,enable_bits);
 LIBC_TRY {
  t64.tv_sec  = abs_timeout->tv_sec;
  t64.tv_nsec = abs_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64_mask_ghost(uaddr,probe_mask,probe_value,enable_bits,&t64);
}

EXPORT(futex_wait_bitset_ghost,libc_futex_wait_bitset_ghost);
INTERN int LIBCCALL
libc_futex_wait_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                             futex_channel_t channels,
                             struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_bitset_ghost(uaddr,probe_value,channels);
 LIBC_TRY {
  t64.tv_sec  = abs_timeout->tv_sec;
  t64.tv_nsec = abs_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64_bitset_ghost(uaddr,probe_value,channels,&t64);
}

EXPORT(futex_wait64_ghost,libc_futex_wait64_ghost);
INTERN int LIBCCALL
libc_futex_wait64_ghost(futex_t *uaddr, futex_t probe_value,
                        struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,
                     abs_timeout,NULL,FUTEX_BITSET_MATCH_ANY);
}

EXPORT(futex_wait64_mask_ghost,libc_futex_wait64_mask_ghost);
INTERN int LIBCCALL
libc_futex_wait64_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                             futex_t probe_value, futex_t enable_bits,
                             struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_MASK_GHOST,probe_mask,abs_timeout,
                    (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(futex_wait64_bitset_ghost,libc_futex_wait64_bitset_ghost);
INTERN int LIBCCALL
libc_futex_wait64_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                               futex_channel_t channels,
                               struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,
                     abs_timeout,NULL,channels);
}

EXPORT(futex_waitrel_ghost,libc_futex_waitrel_ghost);
INTERN int LIBCCALL
libc_futex_waitrel_ghost(futex_t *uaddr, futex_t probe_value,
                         struct timespec32 const *rel_timeout) {
 struct timespec64 t64;
 if (!rel_timeout)
      return libc_futex_wait_inf(uaddr,probe_value);
 LIBC_TRY {
  t64.tv_sec  = rel_timeout->tv_sec;
  t64.tv_nsec = rel_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64_ghost(uaddr,probe_value,&t64);
}

EXPORT(futex_waitrel64_ghost,libc_futex_waitrel64_ghost);
INTERN int LIBCCALL
libc_futex_waitrel64_ghost(futex_t *uaddr, futex_t probe_value,
                           struct timespec64 const *rel_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_GHOST,probe_value,rel_timeout,NULL,0);
}



EXPORT(futex_wake,libc_futex_wake);
INTERN ssize_t LIBCCALL
libc_futex_wake(futex_t *uaddr, size_t num_threads) {
 return (ssize_t)libc_futex64(uaddr,FUTEX_WAKE,(uintptr_t)num_threads,NULL,NULL,0);
}

EXPORT(futex_wake_bitset,libc_futex_wake_bitset);
INTERN ssize_t LIBCCALL
libc_futex_wake_bitset(futex_t *uaddr, size_t num_threads,
                       futex_channel_t channels) {
 return (ssize_t)libc_futex64(uaddr,FUTEX_WAKE_BITSET,(uintptr_t)num_threads,NULL,NULL,channels);
}

EXPORT(futex_lock_inf,libc_futex_lock_inf);
INTERN int LIBCCALL libc_futex_lock_inf(futex_t *uaddr) {
 return (int)libc_futex64(uaddr,FUTEX_LOCK_PI,0,NULL,NULL,0);
}

EXPORT(futex_lock,libc_futex_lock);
INTERN int LIBCCALL
libc_futex_lock(futex_t *uaddr, struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_lock_inf(uaddr);
 LIBC_TRY {
  t64.tv_sec  = abs_timeout->tv_sec;
  t64.tv_nsec = abs_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_lock64(uaddr,&t64);
}

EXPORT(futex_lock64,libc_futex_lock64);
INTERN int LIBCCALL
libc_futex_lock64(futex_t *uaddr, struct timespec64 const *abs_timeout) {
 return (int)libc_futex64(uaddr,FUTEX_LOCK_PI,0,abs_timeout,NULL,0);
}

EXPORT(futex_unlock,libc_futex_unlock);
INTERN ssize_t LIBCCALL libc_futex_unlock(futex_t *uaddr) {
 return (ssize_t)libc_futex64(uaddr,FUTEX_UNLOCK_PI,0,NULL,NULL,0);
}

EXPORT(futex_waitfd,libc_futex_waitfd);
INTERN fd_t LIBCCALL
libc_futex_waitfd(futex_t *uaddr, futex_t probe_value, oflag_t oflags) {
 return (fd_t)libc_futex64(uaddr,FUTEX_WAITFD,probe_value,
                          (struct timespec64 *)(uintptr_t)oflags,
                           NULL,0);
}

EXPORT(futex_waitfd_bitset,libc_futex_waitfd_bitset);
INTERN fd_t LIBCCALL
libc_futex_waitfd_bitset(futex_t *uaddr, futex_t probe_value,
                         oflag_t oflags, futex_channel_t channels) {
 return (fd_t)libc_futex64(uaddr,FUTEX_WAITFD,probe_value,
                          (struct timespec64 *)(uintptr_t)oflags,
                           NULL,channels);
}



EXPORT(Xfutex_wait_inf,libc_Xfutex_wait_inf);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf(futex_t *uaddr, futex_t probe_value) {
 libc_Xfutex64(uaddr,FUTEX_WAIT,probe_value,NULL,NULL,0);
}

EXPORT(Xfutex_wait_inf_mask,libc_Xfutex_wait_inf_mask);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf_mask(futex_t *uaddr, futex_t probe_mask,
                          futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex64(uaddr,FUTEX_WAIT_MASK,probe_mask,NULL,
              (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(Xfutex_wait_inf_bitset,libc_Xfutex_wait_inf_bitset);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf_bitset(futex_t *uaddr, futex_t probe_value,
                            futex_channel_t channels) {
 libc_Xfutex64(uaddr,FUTEX_WAIT_BITSET,probe_value,NULL,NULL,channels);
}

EXPORT(Xfutex_wait,libc_Xfutex_wait);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait(futex_t *uaddr, futex_t probe_value,
                 struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout) {
  libc_Xfutex_wait_inf(uaddr,probe_value);
  return true;
 }
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64(uaddr,probe_value,&t64);
}

EXPORT(Xfutex_wait_mask,libc_Xfutex_wait_mask);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait_mask(futex_t *uaddr, futex_t probe_mask,
                      futex_t probe_value, futex_t enable_bits,
                      struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout) {
  libc_Xfutex_wait_inf_mask(uaddr,probe_mask,probe_value,enable_bits);
  return true;
 }
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_mask(uaddr,probe_mask,probe_value,enable_bits,&t64);
}

EXPORT(Xfutex_wait_bitset,libc_Xfutex_wait_bitset);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait_bitset(futex_t *uaddr, futex_t probe_value,
                        futex_channel_t channels,
                        struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout) {
  libc_Xfutex_wait_inf_bitset(uaddr,probe_value,channels);
  return true;
 }
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_bitset(uaddr,probe_value,channels,&t64);
}

EXPORT(Xfutex_wait64,libc_Xfutex_wait64);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64(futex_t *uaddr, futex_t probe_value,
                   struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_BITSET,probe_value,
                      abs_timeout,NULL,FUTEX_BITSET_MATCH_ANY) != -ETIMEDOUT;
}

EXPORT(Xfutex_wait64_mask,libc_Xfutex_wait64_mask);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64_mask(futex_t *uaddr, futex_t probe_mask,
                        futex_t probe_value, futex_t enable_bits,
                        struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_MASK,probe_mask,abs_timeout,
                     (u32 *)(uintptr_t)probe_mask,enable_bits) != -ETIMEDOUT;
}

EXPORT(Xfutex_wait64_bitset,libc_Xfutex_wait64_bitset);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64_bitset(futex_t *uaddr, futex_t probe_value,
                          futex_channel_t channels,
                          struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_BITSET,probe_value,abs_timeout,NULL,channels) != -ETIMEDOUT;
}

EXPORT(Xfutex_waitrel,libc_Xfutex_waitrel);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_waitrel(futex_t *uaddr, futex_t probe_value,
                    struct timespec32 const *rel_timeout) {
 struct timespec64 t64;
 if (!rel_timeout) {
  libc_Xfutex_wait_inf(uaddr,probe_value);
  return true;
 }
 t64.tv_sec  = rel_timeout->tv_sec;
 t64.tv_nsec = rel_timeout->tv_nsec;
 return libc_Xfutex_waitrel64(uaddr,probe_value,&t64);
}

EXPORT(Xfutex_waitrel64,libc_Xfutex_waitrel64);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_waitrel64(futex_t *uaddr, futex_t probe_value,
                      struct timespec64 const *rel_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT,probe_value,rel_timeout,NULL,0) != -ETIMEDOUT;
}


EXPORT(Xfutex_wait_inf_ghost,libc_Xfutex_wait_inf_ghost);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf_ghost(futex_t *uaddr, futex_t probe_value) {
 libc_Xfutex64(uaddr,FUTEX_WAIT_GHOST,probe_value,NULL,NULL,0);
}

EXPORT(Xfutex_wait_inf_mask_ghost,libc_Xfutex_wait_inf_mask_ghost);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                                futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex64(uaddr,FUTEX_WAIT_MASK_GHOST,probe_mask,NULL,
              (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(Xfutex_wait_inf_bitset_ghost,libc_Xfutex_wait_inf_bitset_ghost);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                                  futex_channel_t channels) {
 libc_Xfutex64(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,NULL,NULL,channels);
}

EXPORT(Xfutex_wait_ghost,libc_Xfutex_wait_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait_ghost(futex_t *uaddr, futex_t probe_value,
                       struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout) {
  libc_Xfutex_wait_inf_ghost(uaddr,probe_value);
  return true;
 }
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_ghost(uaddr,probe_value,&t64);
}

EXPORT(Xfutex_wait_mask_ghost,libc_Xfutex_wait_mask_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                            futex_t probe_value, futex_t enable_bits,
                            struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout) {
  libc_Xfutex_wait_inf_mask_ghost(uaddr,probe_mask,probe_value,enable_bits);
  return true;
 }
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_mask_ghost(uaddr,probe_mask,probe_value,enable_bits,&t64);
}

EXPORT(Xfutex_wait_bitset_ghost,libc_Xfutex_wait_bitset_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                              futex_channel_t channels,
                              struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout) {
  libc_Xfutex_wait_inf_bitset_ghost(uaddr,probe_value,channels);
  return true;
 }
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_bitset_ghost(uaddr,probe_value,channels,&t64);
}

EXPORT(Xfutex_wait64_ghost,libc_Xfutex_wait64_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64_ghost(futex_t *uaddr, futex_t probe_value,
                         struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,
                      abs_timeout,NULL,FUTEX_BITSET_MATCH_ANY) != -ETIMEDOUT;
}

EXPORT(Xfutex_wait64_mask_ghost,libc_Xfutex_wait64_mask_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                              futex_t probe_value, futex_t enable_bits,
                              struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_MASK_GHOST,probe_mask,abs_timeout,
                     (u32 *)(uintptr_t)probe_value,enable_bits) != -ETIMEDOUT;
}

EXPORT(Xfutex_wait64_bitset_ghost,libc_Xfutex_wait64_bitset_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                                futex_channel_t channels,
                                struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,abs_timeout,NULL,channels) != -ETIMEDOUT;
}

EXPORT(Xfutex_waitrel_ghost,libc_Xfutex_waitrel_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_waitrel_ghost(futex_t *uaddr, futex_t probe_value,
                          struct timespec32 const *rel_timeout) {
 struct timespec64 t64;
 if (!rel_timeout) {
  libc_Xfutex_wait_inf_ghost(uaddr,probe_value);
  return true;
 }
 t64.tv_sec  = rel_timeout->tv_sec;
 t64.tv_nsec = rel_timeout->tv_nsec;
 return libc_Xfutex_waitrel64_ghost(uaddr,probe_value,&t64);
}

EXPORT(Xfutex_waitrel64_ghost,libc_Xfutex_waitrel64_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_waitrel64_ghost(futex_t *uaddr, futex_t probe_value,
                            struct timespec64 const *rel_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_GHOST,probe_value,rel_timeout,NULL,0) != -ETIMEDOUT;
}



EXPORT(Xfutex_wake,libc_Xfutex_wake);
CRT_EXCEPT size_t LIBCCALL
libc_Xfutex_wake(futex_t *uaddr, size_t num_threads) {
 return (size_t)libc_Xfutex64(uaddr,FUTEX_WAKE,num_threads,NULL,NULL,0);
}
EXPORT(Xfutex_wake_bitset,libc_Xfutex_wake_bitset);
CRT_EXCEPT size_t LIBCCALL
libc_Xfutex_wake_bitset(futex_t *uaddr, size_t num_threads, futex_channel_t channels) {
 return (size_t)libc_Xfutex64(uaddr,FUTEX_WAKE_BITSET,num_threads,NULL,NULL,channels);
}

EXPORT(Xfutex_lock_inf,libc_Xfutex_lock_inf);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_lock_inf(futex_t *uaddr) {
 libc_Xfutex64(uaddr,FUTEX_LOCK_PI,0,NULL,NULL,0);
}

EXPORT(Xfutex_lock,libc_Xfutex_lock);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_lock(futex_t *uaddr,
                 struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout) {
  libc_Xfutex_lock_inf(uaddr);
  return true;
 }
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_lock64(uaddr,&t64);
}

EXPORT(Xfutex_lock64,libc_Xfutex_lock64);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_lock64(futex_t *uaddr, struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_LOCK_PI,0,abs_timeout,NULL,0) != -ETIMEDOUT;
}

EXPORT(Xfutex_unlock,libc_Xfutex_unlock);
CRT_EXCEPT size_t LIBCCALL libc_Xfutex_unlock(futex_t *uaddr) {
 return (size_t)libc_Xfutex64(uaddr,FUTEX_UNLOCK_PI,0,NULL,NULL,0);
}

EXPORT(Xfutex_waitfd,libc_Xfutex_waitfd);
CRT_EXCEPT fd_t LIBCCALL
libc_Xfutex_waitfd(futex_t *uaddr, futex_t probe_value, oflag_t oflags) {
 return (fd_t)libc_Xfutex64(uaddr,FUTEX_WAITFD,probe_value,
                           (struct timespec64 *)(uintptr_t)oflags,
                            NULL,0);
}

EXPORT(Xfutex_waitfd_bitset,libc_Xfutex_waitfd_bitset);
CRT_EXCEPT fd_t LIBCCALL
libc_Xfutex_waitfd_bitset(futex_t *uaddr, futex_t probe_value,
                          oflag_t oflags, futex_channel_t channels) {
 return (fd_t)libc_Xfutex64(uaddr,FUTEX_WAITFD_BITSET,probe_value,
                           (struct timespec64 *)(uintptr_t)oflags,
                            NULL,channels);
}


EXPORT(xppoll,libc_xppoll);
INTERN ssize_t LIBCCALL
libc_xppoll(struct pollfd *ufds, size_t nfds,
            struct pollfutex *uftx, size_t nftx,
            struct timespec const *tsp, sigset_t *sig) {
 struct timespec64 t64;
 if (!tsp)
      return libc_xppoll64(ufds,nfds,uftx,nftx,NULL,sig);
 LIBC_TRY {
  t64.tv_sec  = tsp->tv_sec;
  t64.tv_nsec = tsp->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_xppoll64(ufds,nfds,uftx,nftx,&t64,sig);
}


EXPORT(xppoll64,libc_xppoll64);
INTERN ssize_t LIBCCALL
libc_xppoll64(struct pollfd *ufds, size_t nfds,
              struct pollfutex *uftx, size_t nftx,
              struct timespec64 const *tsp, sigset_t *sig) {
 struct { sigset_t const *p; size_t s; } sgm;
 sgm.p = sig;
 sgm.s = sizeof(sigset_t);
 return Esys_xppoll(ufds,nfds,uftx,nftx,tsp,&sgm);
}

EXPORT(Xxppoll,libc_Xxppoll);
CRT_EXCEPT size_t LIBCCALL
libc_Xxppoll(struct pollfd *ufds, size_t nfds,
             struct pollfutex *uftx, size_t nftx,
             struct timespec const *tsp, sigset_t *sig) {
 struct timespec64 t64;
 if (!tsp)
      return libc_Xxppoll64(ufds,nfds,uftx,nftx,NULL,sig);
 t64.tv_sec  = tsp->tv_sec;
 t64.tv_nsec = tsp->tv_nsec;
 return libc_Xxppoll64(ufds,nfds,uftx,nftx,&t64,sig);
}

EXPORT(Xxppoll64,libc_Xxppoll64);
CRT_EXCEPT size_t LIBCCALL
libc_Xxppoll64(struct pollfd *ufds, size_t nfds,
               struct pollfutex *uftx, size_t nftx,
               struct timespec64 const *tsp, sigset_t *sig) {
 struct { sigset_t const *p; size_t s; } sgm;
 sgm.p = sig;
 sgm.s = sizeof(sigset_t);
 return Xsys_xppoll(ufds,nfds,uftx,nftx,tsp,&sgm);
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_FUTEX_C */
