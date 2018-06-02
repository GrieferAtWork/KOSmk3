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
#include "errno.h"
#include "sched.h"
#include <errno.h>
#include <hybrid/atomic.h>
#include <linux/futex.h>
#include <kos/futex.h>

DECL_BEGIN

#define FUTEX_FOREACH_CASE_TIMEOUT \
    case FUTEX_WAIT: \
    case FUTEX_WAIT_GHOST: \
    case FUTEX_WAIT_MASK: \
    case FUTEX_WAIT_MASK_GHOST: \
    case FUTEX_WAIT_NMASK: \
    case FUTEX_WAIT_NMASK_GHOST: \
    case FUTEX_WAIT_CMPXCH: \
    case FUTEX_WAIT_CMPXCH_GHOST: \
    case FUTEX_WAIT_CMPXCH2: \
    case FUTEX_WAIT_CMPXCH2_GHOST: \
    case FUTEX_WAIT_BITSET: \
    case FUTEX_WAIT_BITSET_GHOST: \
    case FUTEX_LOCK_PI: \
    case FUTEX_WAIT_REQUEUE_PI: \
/**/

INTERN unsigned int libc_futex_spin = 4;
INTERN ATTR_NOTHROW unsigned int LIBCCALL
libc_futex_getspin(void) {
 return ATOMIC_READ(libc_futex_spin);
}
INTERN ATTR_NOTHROW unsigned int LIBCCALL
libc_futex_setspin(unsigned int new_spin) {
 return ATOMIC_XCH(libc_futex_spin,new_spin);
}

#undef CONFIG_FUTEX_SPINNING
#define CONFIG_FUTEX_SPINNING 1

#ifdef CONFIG_FUTEX_SPINNING
#define SPIN_WHILE(cond) \
do{ \
 LIBC_TRY { \
  unsigned int spin_counter; \
  if (!(cond)) \
      return 0; \
  spin_counter = ATOMIC_READ(libc_futex_spin); \
  while (spin_counter--) { \
   if (sys_sched_yield()) \
       break; \
   if (!(cond)) \
       return 0; \
  } \
 } LIBC_EXCEPT (libc_except_errno()) { \
  return -1; \
 } \
}__WHILE0
#define XSPIN_WHILE(cond,...) \
do{ \
 unsigned int spin_counter; \
 if (!(cond)) \
     return __VA_ARGS__; \
 spin_counter = ATOMIC_READ(libc_futex_spin); \
 while (spin_counter--) { \
  if (sys_sched_yield()) \
      break; \
  if (!(cond)) \
      return __VA_ARGS__; \
 } \
}__WHILE0
#else
#define SPIN_WHILE(cond)      (void)0
#define XSPIN_WHILE(cond,...) (void)0
#endif




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


EXPORT(futex64,libc_futex64);
INTERN syscall_slong_t LIBCCALL
libc_futex64(u32 *uaddr, int futex_op, uintptr_t val,
             struct timespec64 const *timeout,
             u32 *uaddr2, uintptr_t val3) {
#ifdef CONFIG_FUTEX_SPINNING
 unsigned int spin_counter;
 LIBC_TRY {
  switch (futex_op) {
   /* Do futex spinning. */

  case FUTEX_WAIT:
  case FUTEX_WAIT_BITSET:
  case FUTEX_WAIT_GHOST:
  case FUTEX_WAIT_BITSET_GHOST:
   if (ATOMIC_READ(*uaddr) != val)
       return 0;
   spin_counter = ATOMIC_READ(libc_futex_spin);
   while (spin_counter--) {
    if (sys_sched_yield())
        break;
    if (ATOMIC_READ(*uaddr) != val)
        return 0;
   }
   break;

  case FUTEX_WAIT_MASK:
  case FUTEX_WAIT_MASK_BITSET:
  case FUTEX_WAIT_MASK_GHOST:
  case FUTEX_WAIT_MASK_BITSET_GHOST:
   if ((ATOMIC_READ(*uaddr) & val) == (u32)(uintptr_t)uaddr2)
       return 0;
   spin_counter = ATOMIC_READ(libc_futex_spin);
   while (spin_counter--) {
    if (sys_sched_yield())
        break;
    if ((ATOMIC_READ(*uaddr) & val) == (u32)(uintptr_t)uaddr2)
        return 0;
   }
   break;

  case FUTEX_WAIT_NMASK:
  case FUTEX_WAIT_NMASK_BITSET:
  case FUTEX_WAIT_NMASK_GHOST:
  case FUTEX_WAIT_NMASK_BITSET_GHOST:
   if ((ATOMIC_READ(*uaddr) & val) != (u32)(uintptr_t)uaddr2)
       return 0;
   spin_counter = ATOMIC_READ(libc_futex_spin);
   while (spin_counter--) {
    if (sys_sched_yield())
        break;
    if ((ATOMIC_READ(*uaddr) & val) != (u32)(uintptr_t)uaddr2)
        return 0;
   }
   break;

  default: break;
  }
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
#endif /* CONFIG_FUTEX_SPINNING */
 return Esys_futex(uaddr,futex_op,val,timeout,uaddr2,val3);
}


EXPORT(Xfutex64,libc_Xfutex64);
CRT_EXCEPT syscall_slong_t LIBCCALL
libc_Xfutex64(u32 *uaddr, int futex_op, uintptr_t val,
              struct timespec64 const *timeout,
              u32 *uaddr2, uintptr_t val3) {
#ifdef CONFIG_FUTEX_SPINNING
 unsigned int spin_counter;
 switch (futex_op) {
  /* Do futex spinning. */

 case FUTEX_WAIT:
 case FUTEX_WAIT_BITSET:
 case FUTEX_WAIT_GHOST:
 case FUTEX_WAIT_BITSET_GHOST:
  if (ATOMIC_READ(*uaddr) != val)
      return 0;
  spin_counter = ATOMIC_READ(libc_futex_spin);
  while (spin_counter--) {
   if (sys_sched_yield())
       break;
   if (ATOMIC_READ(*uaddr) != val)
       return 0;
  }
  break;

 case FUTEX_WAIT_MASK:
 case FUTEX_WAIT_MASK_BITSET:
 case FUTEX_WAIT_MASK_GHOST:
 case FUTEX_WAIT_MASK_BITSET_GHOST:
  if ((ATOMIC_READ(*uaddr) & val) == (u32)(uintptr_t)uaddr2)
      return 0;
  spin_counter = ATOMIC_READ(libc_futex_spin);
  while (spin_counter--) {
   if (sys_sched_yield())
       break;
   if ((ATOMIC_READ(*uaddr) & val) == (u32)(uintptr_t)uaddr2)
       return 0;
  }
  break;

 case FUTEX_WAIT_NMASK:
 case FUTEX_WAIT_NMASK_BITSET:
 case FUTEX_WAIT_NMASK_GHOST:
 case FUTEX_WAIT_NMASK_BITSET_GHOST:
  if ((ATOMIC_READ(*uaddr) & val) != (u32)(uintptr_t)uaddr2)
      return 0;
  spin_counter = ATOMIC_READ(libc_futex_spin);
  while (spin_counter--) {
   if (sys_sched_yield())
       break;
   if ((ATOMIC_READ(*uaddr) & val) != (u32)(uintptr_t)uaddr2)
       return 0;
  }
  break;

 default: break;
 }
#endif /* CONFIG_FUTEX_SPINNING */
 return Xsys_futex(uaddr,futex_op,val,timeout,uaddr2,val3);
}


#if 1

CRT_KOS int LIBCCALL
libc_loadtimespec64(struct timespec64 *__restrict result,
                    struct timespec32 const *tsp32) {
 LIBC_TRY {
  result->tv_sec  = tsp32->tv_sec;
  result->tv_nsec = tsp32->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return 0;
}




/* ============================================================================== */
/*   FUTEX_WAIT                                                                   */
/* ============================================================================== */
EXPORT(futex_wait_inf,libc_futex_wait_inf);
EXPORT(Xfutex_wait_inf,libc_Xfutex_wait_inf);
EXPORT(futex_wait_inf_ghost,libc_futex_wait_inf_ghost);
EXPORT(Xfutex_wait_inf_ghost,libc_Xfutex_wait_inf_ghost);
#ifdef __OPTIMIZE_SIZE__
CRT_KOS int LIBCCALL
libc_futex_wait_inf(futex_t *uaddr, futex_t probe_value) {
 return libc_futex_wait64(uaddr,probe_value,NULL);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf(futex_t *uaddr, futex_t probe_value) {
 libc_Xfutex_wait64(uaddr,probe_value,NULL);
}
CRT_KOS int LIBCCALL
libc_futex_wait_inf_ghost(futex_t *uaddr, futex_t probe_value) {
 return libc_futex_wait64_ghost(uaddr,probe_value,NULL);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_ghost(futex_t *uaddr, futex_t probe_value) {
 libc_Xfutex_wait64_ghost(uaddr,probe_value,NULL);
}
#else
CRT_KOS int LIBCCALL
libc_futex_wait_inf(futex_t *uaddr, futex_t probe_value) {
 SPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT,probe_value,NULL,NULL,0);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf(futex_t *uaddr, futex_t probe_value) {
 XSPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 Xsys_futex(uaddr,FUTEX_WAIT,probe_value,NULL,NULL,0);
}
CRT_KOS int LIBCCALL
libc_futex_wait_inf_ghost(futex_t *uaddr, futex_t probe_value) {
 SPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_GHOST,probe_value,NULL,NULL,0);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_ghost(futex_t *uaddr, futex_t probe_value) {
 XSPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 Xsys_futex(uaddr,FUTEX_WAIT_GHOST,probe_value,NULL,NULL,0);
}
#endif

EXPORT(futex_wait64,libc_futex_wait64);
CRT_KOS int LIBCCALL
libc_futex_wait64(futex_t *uaddr, futex_t probe_value,
                  struct timespec64 const *abs_timeout) {
 SPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_BITSET,probe_value,
                        abs_timeout,NULL,FUTEX_BITSET_MATCH_ANY);
}
EXPORT(Xfutex_wait64,libc_Xfutex_wait64);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64(futex_t *uaddr, futex_t probe_value,
                   struct timespec64 const *abs_timeout) {
 XSPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value,true);
 return Xsys_futex(uaddr,FUTEX_WAIT_BITSET,probe_value,
                   abs_timeout,NULL,FUTEX_BITSET_MATCH_ANY) != -ETIMEDOUT;
}
EXPORT(futex_waitrel64,libc_futex_waitrel64);
CRT_KOS int LIBCCALL
libc_futex_waitrel64(futex_t *uaddr, futex_t probe_value,
                     struct timespec64 const *rel_timeout) {
 SPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT,probe_value,
                        rel_timeout,NULL,0);
}
EXPORT(Xfutex_waitrel64,libc_Xfutex_waitrel64);
CRT_KOS bool LIBCCALL
libc_Xfutex_waitrel64(futex_t *uaddr, futex_t probe_value,
                      struct timespec64 const *rel_timeout) {
 XSPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value,true);
 return Xsys_futex(uaddr,FUTEX_WAIT,probe_value,
                   rel_timeout,NULL,0) != -ETIMEDOUT;
}


EXPORT(futex_wait,libc_futex_wait);
CRT_KOS int LIBCCALL
libc_futex_wait(futex_t *uaddr, futex_t probe_value,
                struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf(uaddr,probe_value);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64(uaddr,probe_value,&t64);
}
EXPORT(Xfutex_wait,libc_Xfutex_wait);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait(futex_t *uaddr, futex_t probe_value,
                 struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf(uaddr,probe_value),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64(uaddr,probe_value,&t64);
}

EXPORT(futex_waitrel,libc_futex_waitrel);
CRT_KOS int LIBCCALL
libc_futex_waitrel(futex_t *uaddr, futex_t probe_value,
                   struct timespec32 const *rel_timeout) {
 struct timespec64 t64;
 if (!rel_timeout)
      return libc_futex_wait_inf(uaddr,probe_value);
 if (libc_loadtimespec64(&t64,rel_timeout)) return -1;
 return libc_futex_waitrel64(uaddr,probe_value,&t64);
}
EXPORT(Xfutex_waitrel,libc_Xfutex_waitrel);
CRT_KOS bool LIBCCALL
libc_Xfutex_waitrel(futex_t *uaddr, futex_t probe_value,
                    struct timespec32 const *rel_timeout) {
 struct timespec64 t64;
 if (!rel_timeout)
      return (libc_Xfutex_wait_inf(uaddr,probe_value),true);
 t64.tv_sec  = rel_timeout->tv_sec;
 t64.tv_nsec = rel_timeout->tv_nsec;
 return libc_Xfutex_waitrel64(uaddr,probe_value,&t64);
}

EXPORT(futex_wait64_ghost,libc_futex_wait64_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait64_ghost(futex_t *uaddr, futex_t probe_value,
                        struct timespec64 const *abs_timeout) {
 SPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,
                        abs_timeout,NULL,FUTEX_BITSET_MATCH_ANY);
}
EXPORT(Xfutex_wait64_ghost,libc_Xfutex_wait64_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_ghost(futex_t *uaddr, futex_t probe_value,
                         struct timespec64 const *abs_timeout) {
 XSPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value,true);
 return Xsys_futex(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,
                   abs_timeout,NULL,FUTEX_BITSET_MATCH_ANY) != -ETIMEDOUT;
}
EXPORT(futex_waitrel64_ghost,libc_futex_waitrel64_ghost);
CRT_KOS int LIBCCALL
libc_futex_waitrel64_ghost(futex_t *uaddr, futex_t probe_value,
                           struct timespec64 const *rel_timeout) {
 SPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_GHOST,probe_value,
                        rel_timeout,NULL,0);
}
EXPORT(Xfutex_waitrel64_ghost,libc_Xfutex_waitrel64_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_waitrel64_ghost(futex_t *uaddr, futex_t probe_value,
                            struct timespec64 const *rel_timeout) {
 XSPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value,true);
 return Xsys_futex(uaddr,FUTEX_WAIT_GHOST,probe_value,
                   rel_timeout,NULL,0) != -ETIMEDOUT;
}


EXPORT(futex_wait_ghost,libc_futex_wait_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_ghost(futex_t *uaddr, futex_t probe_value,
                      struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_ghost(uaddr,probe_value);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_ghost(uaddr,probe_value,&t64);
}
EXPORT(Xfutex_wait_ghost,libc_Xfutex_wait_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_ghost(futex_t *uaddr, futex_t probe_value,
                       struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_ghost(uaddr,probe_value),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_ghost(uaddr,probe_value,&t64);
}

EXPORT(futex_waitrel_ghost,libc_futex_waitrel_ghost);
CRT_KOS int LIBCCALL
libc_futex_waitrel_ghost(futex_t *uaddr, futex_t probe_value,
                         struct timespec32 const *rel_timeout) {
 struct timespec64 t64;
 if (!rel_timeout)
      return libc_futex_wait_inf_ghost(uaddr,probe_value);
 if (libc_loadtimespec64(&t64,rel_timeout)) return -1;
 return libc_futex_waitrel64_ghost(uaddr,probe_value,&t64);
}
EXPORT(Xfutex_waitrel_ghost,libc_Xfutex_waitrel_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_waitrel_ghost(futex_t *uaddr, futex_t probe_value,
                          struct timespec32 const *rel_timeout) {
 struct timespec64 t64;
 if (!rel_timeout)
      return (libc_Xfutex_wait_inf_ghost(uaddr,probe_value),true);
 t64.tv_sec  = rel_timeout->tv_sec;
 t64.tv_nsec = rel_timeout->tv_nsec;
 return libc_Xfutex_waitrel64_ghost(uaddr,probe_value,&t64);
}



/* ============================================================================== */
/*   FUTEX_WAIT_BITSET                                                            */
/* ============================================================================== */
EXPORT(futex_wait_inf_bitset,libc_futex_wait_inf_bitset);
EXPORT(Xfutex_wait_inf_bitset,libc_Xfutex_wait_inf_bitset);
EXPORT(futex_wait_inf_bitset_ghost,libc_futex_wait_inf_bitset_ghost);
EXPORT(Xfutex_wait_inf_bitset_ghost,libc_Xfutex_wait_inf_bitset_ghost);
#ifdef __OPTIMIZE_SIZE__
CRT_KOS int LIBCCALL
libc_futex_wait_inf_bitset(futex_t *uaddr, futex_t probe_value,
                           futex_channel_t channels) {
 return libc_futex_wait64_bitset(uaddr,probe_value,channels,NULL);
}

CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_bitset(futex_t *uaddr, futex_t probe_value,
                            futex_channel_t channels) {
 libc_Xfutex_wait64_bitset(uaddr,probe_value,channels,NULL);
}
CRT_KOS int LIBCCALL
libc_futex_wait_inf_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                                 futex_channel_t channels) {
 return libc_futex_wait64_bitset_ghost(uaddr,probe_value,channels,NULL);
}

CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                                  futex_channel_t channels) {
 libc_Xfutex_wait64_bitset_ghost(uaddr,probe_value,channels,NULL);
}
#else
CRT_KOS int LIBCCALL
libc_futex_wait_inf_bitset(futex_t *uaddr, futex_t probe_value,
                           futex_channel_t channels) {
 SPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_BITSET,probe_value,NULL,NULL,channels);
}

CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_bitset(futex_t *uaddr, futex_t probe_value,
                            futex_channel_t channels) {
 XSPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 Xsys_futex(uaddr,FUTEX_WAIT_BITSET,probe_value,NULL,NULL,channels);
}
CRT_KOS int LIBCCALL
libc_futex_wait_inf_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                                 futex_channel_t channels) {
 SPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,NULL,NULL,channels);
}

CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                                  futex_channel_t channels) {
 XSPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 Xsys_futex(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,NULL,NULL,channels);
}
#endif

EXPORT(futex_wait64_bitset,libc_futex_wait64_bitset);
CRT_KOS int LIBCCALL
libc_futex_wait64_bitset(futex_t *uaddr, futex_t probe_value,
                         futex_channel_t channels,
                         struct timespec64 const *abs_timeout) {
 SPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_BITSET,probe_value,abs_timeout,NULL,channels);
}

EXPORT(Xfutex_wait64_bitset,libc_Xfutex_wait64_bitset);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_bitset(futex_t *uaddr, futex_t probe_value,
                          futex_channel_t channels,
                          struct timespec64 const *abs_timeout) {
 XSPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value,true);
 return Esys_futex(uaddr,FUTEX_WAIT_BITSET,probe_value,
                   abs_timeout,NULL,channels) != -ETIMEDOUT;
}

EXPORT(futex_wait_bitset,libc_futex_wait_bitset);
CRT_KOS int LIBCCALL
libc_futex_wait_bitset(futex_t *uaddr, futex_t probe_value,
                       futex_channel_t channels,
                       struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_bitset(uaddr,probe_value,channels);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_bitset(uaddr,probe_value,channels,&t64);
}

EXPORT(Xfutex_wait_bitset,libc_Xfutex_wait_bitset);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_bitset(futex_t *uaddr, futex_t probe_value,
                        futex_channel_t channels,
                        struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_bitset(uaddr,probe_value,channels),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_bitset(uaddr,probe_value,channels,&t64);
}

EXPORT(futex_wait64_bitset_ghost,libc_futex_wait64_bitset_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait64_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                               futex_channel_t channels,
                               struct timespec64 const *abs_timeout) {
 SPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,abs_timeout,NULL,channels);
}

EXPORT(Xfutex_wait64_bitset_ghost,libc_Xfutex_wait64_bitset_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                                futex_channel_t channels,
                                struct timespec64 const *abs_timeout) {
 XSPIN_WHILE(ATOMIC_READ(*uaddr) == probe_value,true);
 return Esys_futex(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,
                   abs_timeout,NULL,channels) != -ETIMEDOUT;
}

EXPORT(futex_wait_bitset_ghost,libc_futex_wait_bitset_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                             futex_channel_t channels,
                             struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_bitset_ghost(uaddr,probe_value,channels);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_bitset_ghost(uaddr,probe_value,channels,&t64);
}

EXPORT(Xfutex_wait_bitset_ghost,libc_Xfutex_wait_bitset_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                              futex_channel_t channels,
                              struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_bitset_ghost(uaddr,probe_value,channels),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_bitset_ghost(uaddr,probe_value,channels,&t64);
}






/* ============================================================================== */
/*   FUTEX_WAIT_CMPXCH                                                            */
/* ============================================================================== */
EXPORT(futex_wait_inf_cmpxch,libc_futex_wait_inf_cmpxch);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_cmpxch(futex_t *uaddr, futex_t old_value,
                           futex_t new_value, futex_t *update_target) {
 return (int)Esys_futex(uaddr,FUTEX_WAIT_CMPXCH,old_value,
                        NULL,update_target,new_value);
}

EXPORT(Xfutex_wait_inf_cmpxch,libc_Xfutex_wait_inf_cmpxch);
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_cmpxch(futex_t *uaddr, futex_t old_value,
                            futex_t new_value, futex_t *update_target) {
 Xsys_futex(uaddr,FUTEX_WAIT_CMPXCH,old_value,NULL,update_target,new_value);
}

EXPORT(futex_wait_inf_cmpxch_ghost,libc_futex_wait_inf_cmpxch_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_cmpxch_ghost(futex_t *uaddr, futex_t old_value,
                                 futex_t new_value, futex_t *update_target) {
 return (int)Esys_futex(uaddr,FUTEX_WAIT_CMPXCH_GHOST,old_value,
                        NULL,update_target,new_value);
}

EXPORT(Xfutex_wait_inf_cmpxch_ghost,libc_Xfutex_wait_inf_cmpxch_ghost);
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_cmpxch_ghost(futex_t *uaddr, futex_t old_value,
                                  futex_t new_value, futex_t *update_target) {
 Xsys_futex(uaddr,FUTEX_WAIT_CMPXCH_GHOST,old_value,NULL,update_target,new_value);
}

EXPORT(futex_wait64_cmpxch,libc_futex_wait64_cmpxch);
CRT_KOS int LIBCCALL
libc_futex_wait64_cmpxch(futex_t *uaddr, futex_t old_value,
                         futex_t new_value, futex_t *update_target,
                         struct timespec64 const *abs_timeout) {
 return (int)Esys_futex(uaddr,FUTEX_WAIT_CMPXCH,old_value,
                        abs_timeout,update_target,new_value);
}

EXPORT(Xfutex_wait64_cmpxch,libc_Xfutex_wait64_cmpxch);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_cmpxch(futex_t *uaddr, futex_t old_value,
                          futex_t new_value, futex_t *update_target,
                          struct timespec64 const *abs_timeout) {
 return Xsys_futex(uaddr,FUTEX_WAIT_CMPXCH,old_value,
                   abs_timeout,update_target,new_value) != -ETIMEDOUT;
}

EXPORT(futex_wait_cmpxch,libc_futex_wait_cmpxch);
CRT_KOS int LIBCCALL
libc_futex_wait_cmpxch(futex_t *uaddr, futex_t old_value,
                       futex_t new_value, futex_t *update_target,
                       struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_cmpxch(uaddr,old_value,new_value,update_target);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_cmpxch(uaddr,old_value,new_value,update_target,&t64);
}

EXPORT(Xfutex_wait_cmpxch,libc_Xfutex_wait_cmpxch);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_cmpxch(futex_t *uaddr, futex_t old_value,
                        futex_t new_value, futex_t *update_target,
                        struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_cmpxch(uaddr,old_value,new_value,update_target),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_cmpxch(uaddr,old_value,new_value,update_target,&t64);
}

EXPORT(futex_wait64_cmpxch_ghost,libc_futex_wait64_cmpxch_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait64_cmpxch_ghost(futex_t *uaddr, futex_t old_value,
                               futex_t new_value, futex_t *update_target,
                               struct timespec64 const *abs_timeout) {
 return (int)Esys_futex(uaddr,FUTEX_WAIT_CMPXCH_GHOST,old_value,
                        abs_timeout,update_target,new_value);
}

EXPORT(Xfutex_wait64_cmpxch_ghost,libc_Xfutex_wait64_cmpxch_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_cmpxch_ghost(futex_t *uaddr, futex_t old_value,
                                futex_t new_value, futex_t *update_target,
                                struct timespec64 const *abs_timeout) {
 return Xsys_futex(uaddr,FUTEX_WAIT_CMPXCH_GHOST,old_value,
                   abs_timeout,update_target,new_value) != -ETIMEDOUT;
}

EXPORT(futex_wait_cmpxch_ghost,libc_futex_wait_cmpxch_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_cmpxch_ghost(futex_t *uaddr, futex_t old_value,
                             futex_t new_value, futex_t *update_target,
                             struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_cmpxch_ghost(uaddr,old_value,new_value,update_target);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_cmpxch_ghost(uaddr,old_value,new_value,update_target,&t64);
}

EXPORT(Xfutex_wait_cmpxch_ghost,libc_Xfutex_wait_cmpxch_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_cmpxch_ghost(futex_t *uaddr, futex_t old_value,
                              futex_t new_value, futex_t *update_target,
                              struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_cmpxch_ghost(uaddr,old_value,new_value,update_target),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_cmpxch_ghost(uaddr,old_value,new_value,update_target,&t64);
}






/* ============================================================================== */
/*   FUTEX_WAIT_CMPXCH2                                                           */
/* ============================================================================== */
EXPORT(futex_wait_inf_cmpxch2,libc_futex_wait_inf_cmpxch2);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_cmpxch2(futex_t *uaddr, futex_t old_value,
                            futex_t new_value, futex_t *update_target) {
 return (int)Esys_futex(uaddr,FUTEX_WAIT_CMPXCH,old_value,
                        NULL,update_target,new_value);
}

EXPORT(Xfutex_wait_inf_cmpxch2,libc_Xfutex_wait_inf_cmpxch2);
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_cmpxch2(futex_t *uaddr, futex_t old_value,
                             futex_t new_value, futex_t *update_target) {
 Xsys_futex(uaddr,FUTEX_WAIT_CMPXCH,old_value,NULL,update_target,new_value);
}

EXPORT(futex_wait_inf_cmpxch2_ghost,libc_futex_wait_inf_cmpxch2_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_cmpxch2_ghost(futex_t *uaddr, futex_t old_value,
                                  futex_t new_value, futex_t *update_target) {
 return (int)Esys_futex(uaddr,FUTEX_WAIT_CMPXCH_GHOST,old_value,
                        NULL,update_target,new_value);
}

EXPORT(Xfutex_wait_inf_cmpxch2_ghost,libc_Xfutex_wait_inf_cmpxch2_ghost);
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_cmpxch2_ghost(futex_t *uaddr, futex_t old_value,
                                   futex_t new_value, futex_t *update_target) {
 Xsys_futex(uaddr,FUTEX_WAIT_CMPXCH_GHOST,old_value,NULL,update_target,new_value);
}

EXPORT(futex_wait64_cmpxch2,libc_futex_wait64_cmpxch2);
CRT_KOS int LIBCCALL
libc_futex_wait64_cmpxch2(futex_t *uaddr, futex_t old_value,
                          futex_t new_value, futex_t *update_target,
                          struct timespec64 const *abs_timeout) {
 return (int)Esys_futex(uaddr,FUTEX_WAIT_CMPXCH,old_value,
                        abs_timeout,update_target,new_value);
}

EXPORT(Xfutex_wait64_cmpxch2,libc_Xfutex_wait64_cmpxch2);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_cmpxch2(futex_t *uaddr, futex_t old_value,
                           futex_t new_value, futex_t *update_target,
                           struct timespec64 const *abs_timeout) {
 return Xsys_futex(uaddr,FUTEX_WAIT_CMPXCH,old_value,
                   abs_timeout,update_target,new_value) != -ETIMEDOUT;
}

EXPORT(futex_wait_cmpxch2,libc_futex_wait_cmpxch2);
CRT_KOS int LIBCCALL
libc_futex_wait_cmpxch2(futex_t *uaddr, futex_t old_value,
                        futex_t new_value, futex_t *update_target,
                        struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_cmpxch2(uaddr,old_value,new_value,update_target);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_cmpxch2(uaddr,old_value,new_value,update_target,&t64);
}

EXPORT(Xfutex_wait_cmpxch2,libc_Xfutex_wait_cmpxch2);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_cmpxch2(futex_t *uaddr, futex_t old_value,
                         futex_t new_value, futex_t *update_target,
                         struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_cmpxch2(uaddr,old_value,new_value,update_target),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_cmpxch2(uaddr,old_value,new_value,update_target,&t64);
}

EXPORT(futex_wait64_cmpxch2_ghost,libc_futex_wait64_cmpxch2_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait64_cmpxch2_ghost(futex_t *uaddr, futex_t old_value,
                                futex_t new_value, futex_t *update_target,
                                struct timespec64 const *abs_timeout) {
 return (int)Esys_futex(uaddr,FUTEX_WAIT_CMPXCH_GHOST,old_value,
                        abs_timeout,update_target,new_value);
}

EXPORT(Xfutex_wait64_cmpxch2_ghost,libc_Xfutex_wait64_cmpxch2_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_cmpxch2_ghost(futex_t *uaddr, futex_t old_value,
                                 futex_t new_value, futex_t *update_target,
                                 struct timespec64 const *abs_timeout) {
 return Xsys_futex(uaddr,FUTEX_WAIT_CMPXCH_GHOST,old_value,
                   abs_timeout,update_target,new_value) != -ETIMEDOUT;
}

EXPORT(futex_wait_cmpxch2_ghost,libc_futex_wait_cmpxch2_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_cmpxch2_ghost(futex_t *uaddr, futex_t old_value,
                              futex_t new_value, futex_t *update_target,
                              struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_cmpxch2_ghost(uaddr,old_value,new_value,update_target);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_cmpxch2_ghost(uaddr,old_value,new_value,update_target,&t64);
}

EXPORT(Xfutex_wait_cmpxch2_ghost,libc_Xfutex_wait_cmpxch2_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_cmpxch2_ghost(futex_t *uaddr, futex_t old_value,
                               futex_t new_value, futex_t *update_target,
                               struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_cmpxch2_ghost(uaddr,old_value,new_value,update_target),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_cmpxch2_ghost(uaddr,old_value,new_value,update_target,&t64);
}





/* ============================================================================== */
/*   FUTEX_WAIT_MASK                                                              */
/* ============================================================================== */
EXPORT(futex_wait_inf_mask,libc_futex_wait_inf_mask);
EXPORT(Xfutex_wait_inf_mask,libc_Xfutex_wait_inf_mask);
EXPORT(futex_wait_inf_mask_ghost,libc_futex_wait_inf_mask_ghost);
EXPORT(Xfutex_wait_inf_mask_ghost,libc_Xfutex_wait_inf_mask_ghost);
#ifdef __OPTIMIZE_SIZE__
CRT_KOS int LIBCCALL
libc_futex_wait_inf_mask(futex_t *uaddr, futex_t probe_mask,
                         futex_t probe_value, futex_t enable_bits) {
 return libc_futex_wait64_mask(uaddr,probe_mask,probe_value,enable_bits,NULL);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_mask(futex_t *uaddr, futex_t probe_mask,
                          futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex_wait64_mask(uaddr,probe_mask,probe_value,enable_bits,NULL);
}
CRT_KOS int LIBCCALL
libc_futex_wait_inf_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                               futex_t probe_value, futex_t enable_bits) {
 return libc_futex_wait64_mask_ghost(uaddr,probe_mask,probe_value,enable_bits,NULL);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                                futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex_wait64_mask_ghost(uaddr,probe_mask,probe_value,enable_bits,NULL);
}
#else
CRT_KOS int LIBCCALL
libc_futex_wait_inf_mask(futex_t *uaddr, futex_t probe_mask,
                         futex_t probe_value, futex_t enable_bits) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_MASK,probe_mask,NULL,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_mask(futex_t *uaddr, futex_t probe_mask,
                          futex_t probe_value, futex_t enable_bits) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask) == probe_value);
 Xsys_futex(uaddr,FUTEX_WAIT_MASK,probe_mask,NULL,
           (u32 *)(uintptr_t)probe_value,enable_bits);
}
CRT_KOS int LIBCCALL
libc_futex_wait_inf_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                               futex_t probe_value, futex_t enable_bits) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_MASK_GHOST,probe_mask,NULL,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                                futex_t probe_value, futex_t enable_bits) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask) == probe_value);
 Xsys_futex(uaddr,FUTEX_WAIT_MASK_GHOST,probe_mask,NULL,
           (u32 *)(uintptr_t)probe_value,enable_bits);
}
#endif
EXPORT(futex_wait64_mask,libc_futex_wait64_mask);
CRT_KOS int LIBCCALL
libc_futex_wait64_mask(futex_t *uaddr, futex_t probe_mask,
                       futex_t probe_value, futex_t enable_bits,
                       struct timespec64 const *abs_timeout) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_MASK,probe_mask,abs_timeout,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
EXPORT(Xfutex_wait64_mask,libc_Xfutex_wait64_mask);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_mask(futex_t *uaddr, futex_t probe_mask,
                        futex_t probe_value, futex_t enable_bits,
                        struct timespec64 const *abs_timeout) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask) == probe_value,true);
 return Xsys_futex(uaddr,FUTEX_WAIT_MASK,probe_mask,abs_timeout,
                  (u32 *)(uintptr_t)probe_value,enable_bits) != -ETIMEDOUT;
}
EXPORT(futex_wait_mask,libc_futex_wait_mask);
CRT_KOS int LIBCCALL
libc_futex_wait_mask(futex_t *uaddr, futex_t probe_mask,
                     futex_t probe_value, futex_t enable_bits,
                     struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_mask(uaddr,probe_mask,probe_value,enable_bits);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_mask(uaddr,probe_mask,probe_value,enable_bits,&t64);
}
EXPORT(Xfutex_wait_mask,libc_Xfutex_wait_mask);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_mask(futex_t *uaddr, futex_t probe_mask,
                      futex_t probe_value, futex_t enable_bits,
                      struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_mask(uaddr,probe_mask,probe_value,enable_bits),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_mask(uaddr,probe_mask,probe_value,enable_bits,&t64) != -ETIMEDOUT;
}
EXPORT(futex_wait64_mask_ghost,libc_futex_wait64_mask_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait64_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                             futex_t probe_value, futex_t enable_bits,
                             struct timespec64 const *abs_timeout) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_MASK_GHOST,probe_mask,abs_timeout,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
EXPORT(Xfutex_wait64_mask_ghost,libc_Xfutex_wait64_mask_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                              futex_t probe_value, futex_t enable_bits,
                              struct timespec64 const *abs_timeout) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask) == probe_value,true);
 return Xsys_futex(uaddr,FUTEX_WAIT_MASK_GHOST,probe_mask,abs_timeout,
                  (u32 *)(uintptr_t)probe_value,enable_bits) != -ETIMEDOUT;
}
EXPORT(futex_wait_mask_ghost,libc_futex_wait_mask_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                           futex_t probe_value, futex_t enable_bits,
                           struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_mask_ghost(uaddr,probe_mask,probe_value,enable_bits);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_mask_ghost(uaddr,probe_mask,probe_value,enable_bits,&t64);
}
EXPORT(Xfutex_wait_mask_ghost,libc_Xfutex_wait_mask_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                            futex_t probe_value, futex_t enable_bits,
                            struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_mask_ghost(uaddr,probe_mask,probe_value,enable_bits),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_mask_ghost(uaddr,probe_mask,probe_value,enable_bits,&t64) != -ETIMEDOUT;
}





/* ============================================================================== */
/*   FUTEX_WAIT_NMASK                                                             */
/* ============================================================================== */
EXPORT(futex_wait_inf_nmask,libc_futex_wait_inf_nmask);
EXPORT(Xfutex_wait_inf_nmask,libc_Xfutex_wait_inf_nmask);
EXPORT(futex_wait_inf_nmask_ghost,libc_futex_wait_inf_nmask_ghost);
EXPORT(Xfutex_wait_inf_nmask_ghost,libc_Xfutex_wait_inf_nmask_ghost);
#ifdef __OPTIMIZE_SIZE__
CRT_KOS int LIBCCALL
libc_futex_wait_inf_nmask(futex_t *uaddr, futex_t probe_nmask,
                          futex_t probe_value, futex_t enable_bits) {
 return libc_futex_wait64_nmask(uaddr,probe_nmask,probe_value,enable_bits,NULL);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_nmask(futex_t *uaddr, futex_t probe_nmask,
                           futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex_wait64_nmask(uaddr,probe_nmask,probe_value,enable_bits,NULL);
}
CRT_KOS int LIBCCALL
libc_futex_wait_inf_nmask_ghost(futex_t *uaddr, futex_t probe_nmask,
                                futex_t probe_value, futex_t enable_bits) {
 return libc_futex_wait64_nmask_ghost(uaddr,probe_nmask,probe_value,enable_bits,NULL);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_nmask_ghost(futex_t *uaddr, futex_t probe_nmask,
                                 futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex_wait64_nmask_ghost(uaddr,probe_nmask,probe_value,enable_bits,NULL);
}
#else
CRT_KOS int LIBCCALL
libc_futex_wait_inf_nmask(futex_t *uaddr, futex_t probe_nmask,
                          futex_t probe_value, futex_t enable_bits) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask) != probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_NMASK,probe_nmask,NULL,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_nmask(futex_t *uaddr, futex_t probe_nmask,
                           futex_t probe_value, futex_t enable_bits) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask) != probe_value);
 Xsys_futex(uaddr,FUTEX_WAIT_NMASK,probe_nmask,NULL,
           (u32 *)(uintptr_t)probe_value,enable_bits);
}
CRT_KOS int LIBCCALL
libc_futex_wait_inf_nmask_ghost(futex_t *uaddr, futex_t probe_nmask,
                                futex_t probe_value, futex_t enable_bits) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask) != probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_NMASK_GHOST,probe_nmask,NULL,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_nmask_ghost(futex_t *uaddr, futex_t probe_nmask,
                                 futex_t probe_value, futex_t enable_bits) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask) != probe_value);
 Xsys_futex(uaddr,FUTEX_WAIT_NMASK_GHOST,probe_nmask,NULL,
           (u32 *)(uintptr_t)probe_value,enable_bits);
}
#endif
EXPORT(futex_wait64_nmask,libc_futex_wait64_nmask);
CRT_KOS int LIBCCALL
libc_futex_wait64_nmask(futex_t *uaddr, futex_t probe_nmask,
                        futex_t probe_value, futex_t enable_bits,
                        struct timespec64 const *abs_timeout) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask) != probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_NMASK,probe_nmask,abs_timeout,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
EXPORT(Xfutex_wait64_nmask,libc_Xfutex_wait64_nmask);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_nmask(futex_t *uaddr, futex_t probe_nmask,
                         futex_t probe_value, futex_t enable_bits,
                         struct timespec64 const *abs_timeout) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask) != probe_value,true);
 return Xsys_futex(uaddr,FUTEX_WAIT_NMASK,probe_nmask,abs_timeout,
                  (u32 *)(uintptr_t)probe_value,enable_bits) != -ETIMEDOUT;
}
EXPORT(futex_wait_nmask,libc_futex_wait_nmask);
CRT_KOS int LIBCCALL
libc_futex_wait_nmask(futex_t *uaddr, futex_t probe_nmask,
                      futex_t probe_value, futex_t enable_bits,
                      struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_nmask(uaddr,probe_nmask,probe_value,enable_bits);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_nmask(uaddr,probe_nmask,probe_value,enable_bits,&t64);
}
EXPORT(Xfutex_wait_nmask,libc_Xfutex_wait_nmask);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_nmask(futex_t *uaddr, futex_t probe_nmask,
                       futex_t probe_value, futex_t enable_bits,
                       struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_nmask(uaddr,probe_nmask,probe_value,enable_bits),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_nmask(uaddr,probe_nmask,probe_value,enable_bits,&t64) != -ETIMEDOUT;
}
EXPORT(futex_wait64_nmask_ghost,libc_futex_wait64_nmask_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait64_nmask_ghost(futex_t *uaddr, futex_t probe_nmask,
                              futex_t probe_value, futex_t enable_bits,
                              struct timespec64 const *abs_timeout) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask) != probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_NMASK_GHOST,probe_nmask,abs_timeout,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
EXPORT(Xfutex_wait64_nmask_ghost,libc_Xfutex_wait64_nmask_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_nmask_ghost(futex_t *uaddr, futex_t probe_nmask,
                               futex_t probe_value, futex_t enable_bits,
                               struct timespec64 const *abs_timeout) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask) != probe_value,true);
 return Xsys_futex(uaddr,FUTEX_WAIT_NMASK_GHOST,probe_nmask,abs_timeout,
                  (u32 *)(uintptr_t)probe_value,enable_bits) != -ETIMEDOUT;
}
EXPORT(futex_wait_nmask_ghost,libc_futex_wait_nmask_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_nmask_ghost(futex_t *uaddr, futex_t probe_nmask,
                            futex_t probe_value, futex_t enable_bits,
                            struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_nmask_ghost(uaddr,probe_nmask,probe_value,enable_bits);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_nmask_ghost(uaddr,probe_nmask,probe_value,enable_bits,&t64);
}
EXPORT(Xfutex_wait_nmask_ghost,libc_Xfutex_wait_nmask_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_nmask_ghost(futex_t *uaddr, futex_t probe_nmask,
                             futex_t probe_value, futex_t enable_bits,
                             struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_nmask_ghost(uaddr,probe_nmask,probe_value,enable_bits),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_nmask_ghost(uaddr,probe_nmask,probe_value,enable_bits,&t64) != -ETIMEDOUT;
}









/* ============================================================================== */
/*   FUTEX_WAIT_MASK_BITSET                                                       */
/* ============================================================================== */
EXPORT(futex_wait_inf_mask_bitset,libc_futex_wait_inf_mask_bitset);
EXPORT(Xfutex_wait_inf_mask_bitset,libc_Xfutex_wait_inf_mask_bitset);
EXPORT(futex_wait_inf_mask_bitset_ghost,libc_futex_wait_inf_mask_bitset_ghost);
EXPORT(Xfutex_wait_inf_mask_bitset_ghost,libc_Xfutex_wait_inf_mask_bitset_ghost);
#ifdef __OPTIMIZE_SIZE__
CRT_KOS int LIBCCALL
libc_futex_wait_inf_mask_bitset(futex_t *uaddr, futex_t probe_mask_bitset,
                                futex_t probe_value, futex_t enable_bits) {
 return libc_futex_wait64_mask_bitset(uaddr,probe_mask_bitset,probe_value,enable_bits,NULL);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_mask_bitset(futex_t *uaddr, futex_t probe_mask_bitset,
                                 futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex_wait64_mask_bitset(uaddr,probe_mask_bitset,probe_value,enable_bits,NULL);
}
CRT_KOS int LIBCCALL
libc_futex_wait_inf_mask_bitset_ghost(futex_t *uaddr, futex_t probe_mask_bitset,
                                      futex_t probe_value, futex_t enable_bits) {
 return libc_futex_wait64_mask_bitset_ghost(uaddr,probe_mask_bitset,probe_value,enable_bits,NULL);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_mask_bitset_ghost(futex_t *uaddr, futex_t probe_mask_bitset,
                                       futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex_wait64_mask_bitset_ghost(uaddr,probe_mask_bitset,probe_value,enable_bits,NULL);
}
#else
CRT_KOS int LIBCCALL
libc_futex_wait_inf_mask_bitset(futex_t *uaddr, futex_t probe_mask_bitset,
                                futex_t probe_value, futex_t enable_bits) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask_bitset) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_MASK_BITSET,probe_mask_bitset,NULL,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_mask_bitset(futex_t *uaddr, futex_t probe_mask_bitset,
                                 futex_t probe_value, futex_t enable_bits) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask_bitset) == probe_value);
 Xsys_futex(uaddr,FUTEX_WAIT_MASK_BITSET,probe_mask_bitset,NULL,
           (u32 *)(uintptr_t)probe_value,enable_bits);
}
CRT_KOS int LIBCCALL
libc_futex_wait_inf_mask_bitset_ghost(futex_t *uaddr, futex_t probe_mask_bitset,
                                      futex_t probe_value, futex_t enable_bits) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask_bitset) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_MASK_BITSET_GHOST,probe_mask_bitset,NULL,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_mask_bitset_ghost(futex_t *uaddr, futex_t probe_mask_bitset,
                                       futex_t probe_value, futex_t enable_bits) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask_bitset) == probe_value);
 Xsys_futex(uaddr,FUTEX_WAIT_MASK_BITSET_GHOST,probe_mask_bitset,NULL,
           (u32 *)(uintptr_t)probe_value,enable_bits);
}
#endif
EXPORT(futex_wait64_mask_bitset,libc_futex_wait64_mask_bitset);
CRT_KOS int LIBCCALL
libc_futex_wait64_mask_bitset(futex_t *uaddr, futex_t probe_mask_bitset,
                              futex_t probe_value, futex_t enable_bits,
                              struct timespec64 const *abs_timeout) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask_bitset) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_MASK_BITSET,probe_mask_bitset,abs_timeout,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
EXPORT(Xfutex_wait64_mask_bitset,libc_Xfutex_wait64_mask_bitset);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_mask_bitset(futex_t *uaddr, futex_t probe_mask_bitset,
                               futex_t probe_value, futex_t enable_bits,
                               struct timespec64 const *abs_timeout) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask_bitset) == probe_value,true);
 return Xsys_futex(uaddr,FUTEX_WAIT_MASK_BITSET,probe_mask_bitset,abs_timeout,
                  (u32 *)(uintptr_t)probe_value,enable_bits) != -ETIMEDOUT;
}
EXPORT(futex_wait_mask_bitset,libc_futex_wait_mask_bitset);
CRT_KOS int LIBCCALL
libc_futex_wait_mask_bitset(futex_t *uaddr, futex_t probe_mask_bitset,
                            futex_t probe_value, futex_t enable_bits,
                            struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_mask_bitset(uaddr,probe_mask_bitset,probe_value,enable_bits);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_mask_bitset(uaddr,probe_mask_bitset,probe_value,enable_bits,&t64);
}
EXPORT(Xfutex_wait_mask_bitset,libc_Xfutex_wait_mask_bitset);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_mask_bitset(futex_t *uaddr, futex_t probe_mask_bitset,
                             futex_t probe_value, futex_t enable_bits,
                             struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_mask_bitset(uaddr,probe_mask_bitset,probe_value,enable_bits),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_mask_bitset(uaddr,probe_mask_bitset,probe_value,enable_bits,&t64) != -ETIMEDOUT;
}
EXPORT(futex_wait64_mask_bitset_ghost,libc_futex_wait64_mask_bitset_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait64_mask_bitset_ghost(futex_t *uaddr, futex_t probe_mask_bitset,
                                    futex_t probe_value, futex_t enable_bits,
                                    struct timespec64 const *abs_timeout) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask_bitset) == probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_MASK_BITSET_GHOST,probe_mask_bitset,abs_timeout,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
EXPORT(Xfutex_wait64_mask_bitset_ghost,libc_Xfutex_wait64_mask_bitset_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_mask_bitset_ghost(futex_t *uaddr, futex_t probe_mask_bitset,
                                     futex_t probe_value, futex_t enable_bits,
                                     struct timespec64 const *abs_timeout) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_mask_bitset) == probe_value,true);
 return Xsys_futex(uaddr,FUTEX_WAIT_MASK_BITSET_GHOST,probe_mask_bitset,abs_timeout,
                  (u32 *)(uintptr_t)probe_value,enable_bits) != -ETIMEDOUT;
}
EXPORT(futex_wait_mask_bitset_ghost,libc_futex_wait_mask_bitset_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_mask_bitset_ghost(futex_t *uaddr, futex_t probe_mask_bitset,
                                  futex_t probe_value, futex_t enable_bits,
                                  struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_mask_bitset_ghost(uaddr,probe_mask_bitset,probe_value,enable_bits);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_mask_bitset_ghost(uaddr,probe_mask_bitset,probe_value,enable_bits,&t64);
}
EXPORT(Xfutex_wait_mask_bitset_ghost,libc_Xfutex_wait_mask_bitset_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_mask_bitset_ghost(futex_t *uaddr, futex_t probe_mask_bitset,
                                   futex_t probe_value, futex_t enable_bits,
                                   struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_mask_bitset_ghost(uaddr,probe_mask_bitset,probe_value,enable_bits),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_mask_bitset_ghost(uaddr,probe_mask_bitset,probe_value,enable_bits,&t64) != -ETIMEDOUT;
}





/* ============================================================================== */
/*   FUTEX_WAIT_NMASK_BITSET                                                             */
/* ============================================================================== */
EXPORT(futex_wait_inf_nmask_bitset,libc_futex_wait_inf_nmask_bitset);
EXPORT(Xfutex_wait_inf_nmask_bitset,libc_Xfutex_wait_inf_nmask_bitset);
EXPORT(futex_wait_inf_nmask_bitset_ghost,libc_futex_wait_inf_nmask_bitset_ghost);
EXPORT(Xfutex_wait_inf_nmask_bitset_ghost,libc_Xfutex_wait_inf_nmask_bitset_ghost);
#ifdef __OPTIMIZE_SIZE__
CRT_KOS int LIBCCALL
libc_futex_wait_inf_nmask_bitset(futex_t *uaddr, futex_t probe_nmask_bitset,
                                 futex_t probe_value, futex_t enable_bits) {
 return libc_futex_wait64_nmask_bitset(uaddr,probe_nmask_bitset,probe_value,enable_bits,NULL);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_nmask_bitset(futex_t *uaddr, futex_t probe_nmask_bitset,
                                  futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex_wait64_nmask_bitset(uaddr,probe_nmask_bitset,probe_value,enable_bits,NULL);
}
CRT_KOS int LIBCCALL
libc_futex_wait_inf_nmask_bitset_ghost(futex_t *uaddr, futex_t probe_nmask_bitset,
                                       futex_t probe_value, futex_t enable_bits) {
 return libc_futex_wait64_nmask_bitset_ghost(uaddr,probe_nmask_bitset,probe_value,enable_bits,NULL);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_nmask_bitset_ghost(futex_t *uaddr, futex_t probe_nmask_bitset,
                                        futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex_wait64_nmask_bitset_ghost(uaddr,probe_nmask_bitset,probe_value,enable_bits,NULL);
}
#else
CRT_KOS int LIBCCALL
libc_futex_wait_inf_nmask_bitset(futex_t *uaddr, futex_t probe_nmask_bitset,
                                 futex_t probe_value, futex_t enable_bits) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask_bitset) != probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_NMASK_BITSET,probe_nmask_bitset,NULL,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_nmask_bitset(futex_t *uaddr, futex_t probe_nmask_bitset,
                                  futex_t probe_value, futex_t enable_bits) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask_bitset) != probe_value);
 Xsys_futex(uaddr,FUTEX_WAIT_NMASK_BITSET,probe_nmask_bitset,NULL,
           (u32 *)(uintptr_t)probe_value,enable_bits);
}
CRT_KOS int LIBCCALL
libc_futex_wait_inf_nmask_bitset_ghost(futex_t *uaddr, futex_t probe_nmask_bitset,
                                       futex_t probe_value, futex_t enable_bits) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask_bitset) != probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_NMASK_BITSET_GHOST,probe_nmask_bitset,NULL,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
CRT_KOS void LIBCCALL
libc_Xfutex_wait_inf_nmask_bitset_ghost(futex_t *uaddr, futex_t probe_nmask_bitset,
                                        futex_t probe_value, futex_t enable_bits) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask_bitset) != probe_value);
 Xsys_futex(uaddr,FUTEX_WAIT_NMASK_BITSET_GHOST,probe_nmask_bitset,NULL,
           (u32 *)(uintptr_t)probe_value,enable_bits);
}
#endif
EXPORT(futex_wait64_nmask_bitset,libc_futex_wait64_nmask_bitset);
CRT_KOS int LIBCCALL
libc_futex_wait64_nmask_bitset(futex_t *uaddr, futex_t probe_nmask_bitset,
                               futex_t probe_value, futex_t enable_bits,
                               struct timespec64 const *abs_timeout) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask_bitset) != probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_NMASK_BITSET,probe_nmask_bitset,abs_timeout,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
EXPORT(Xfutex_wait64_nmask_bitset,libc_Xfutex_wait64_nmask_bitset);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_nmask_bitset(futex_t *uaddr, futex_t probe_nmask_bitset,
                                futex_t probe_value, futex_t enable_bits,
                                struct timespec64 const *abs_timeout) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask_bitset) != probe_value,true);
 return Xsys_futex(uaddr,FUTEX_WAIT_NMASK_BITSET,probe_nmask_bitset,abs_timeout,
                  (u32 *)(uintptr_t)probe_value,enable_bits) != -ETIMEDOUT;
}
EXPORT(futex_wait_nmask_bitset,libc_futex_wait_nmask_bitset);
CRT_KOS int LIBCCALL
libc_futex_wait_nmask_bitset(futex_t *uaddr, futex_t probe_nmask_bitset,
                             futex_t probe_value, futex_t enable_bits,
                             struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_nmask_bitset(uaddr,probe_nmask_bitset,probe_value,enable_bits);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_nmask_bitset(uaddr,probe_nmask_bitset,probe_value,enable_bits,&t64);
}
EXPORT(Xfutex_wait_nmask_bitset,libc_Xfutex_wait_nmask_bitset);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_nmask_bitset(futex_t *uaddr, futex_t probe_nmask_bitset,
                              futex_t probe_value, futex_t enable_bits,
                              struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_nmask_bitset(uaddr,probe_nmask_bitset,probe_value,enable_bits),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_nmask_bitset(uaddr,probe_nmask_bitset,probe_value,enable_bits,&t64) != -ETIMEDOUT;
}
EXPORT(futex_wait64_nmask_bitset_ghost,libc_futex_wait64_nmask_bitset_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait64_nmask_bitset_ghost(futex_t *uaddr, futex_t probe_nmask_bitset,
                                     futex_t probe_value, futex_t enable_bits,
                                     struct timespec64 const *abs_timeout) {
 SPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask_bitset) != probe_value);
 return (int)Esys_futex(uaddr,FUTEX_WAIT_NMASK_BITSET_GHOST,probe_nmask_bitset,abs_timeout,
                       (u32 *)(uintptr_t)probe_value,enable_bits);
}
EXPORT(Xfutex_wait64_nmask_bitset_ghost,libc_Xfutex_wait64_nmask_bitset_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait64_nmask_bitset_ghost(futex_t *uaddr, futex_t probe_nmask_bitset,
                                      futex_t probe_value, futex_t enable_bits,
                                      struct timespec64 const *abs_timeout) {
 XSPIN_WHILE((ATOMIC_READ(*uaddr) & probe_nmask_bitset) != probe_value,true);
 return Xsys_futex(uaddr,FUTEX_WAIT_NMASK_BITSET_GHOST,probe_nmask_bitset,abs_timeout,
                  (u32 *)(uintptr_t)probe_value,enable_bits) != -ETIMEDOUT;
}
EXPORT(futex_wait_nmask_bitset_ghost,libc_futex_wait_nmask_bitset_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_nmask_bitset_ghost(futex_t *uaddr, futex_t probe_nmask_bitset,
                                   futex_t probe_value, futex_t enable_bits,
                                   struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_nmask_bitset_ghost(uaddr,probe_nmask_bitset,probe_value,enable_bits);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_wait64_nmask_bitset_ghost(uaddr,probe_nmask_bitset,probe_value,enable_bits,&t64);
}
EXPORT(Xfutex_wait_nmask_bitset_ghost,libc_Xfutex_wait_nmask_bitset_ghost);
CRT_KOS bool LIBCCALL
libc_Xfutex_wait_nmask_bitset_ghost(futex_t *uaddr, futex_t probe_nmask_bitset,
                                    futex_t probe_value, futex_t enable_bits,
                                    struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_wait_inf_nmask_bitset_ghost(uaddr,probe_nmask_bitset,probe_value,enable_bits),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_nmask_bitset_ghost(uaddr,probe_nmask_bitset,probe_value,enable_bits,&t64) != -ETIMEDOUT;
}

#ifdef CONFIG_FUTEX_SPINNING
CRT_KOS bool LIBCCALL libc_futex_trylock(futex_t *uaddr) {
 for (;;) {
  futex_t value = ATOMIC_READ(*uaddr);
  if (value & FUTEX_TID_MASK) return false;
  if (ATOMIC_CMPXCH(*uaddr,value,
                   (value & ~FUTEX_TID_MASK) |
                   (futex_t)libc_gettid()))
      break;
 }
 return true;
}
#endif /* CONFIG_FUTEX_SPINNING */

EXPORT(futex_lock_inf,libc_futex_lock_inf);
EXPORT(Xfutex_lock_inf,libc_Xfutex_lock_inf);
#ifdef __OPTIMIZE_SIZE__
CRT_KOS int LIBCCALL libc_futex_lock_inf(futex_t *uaddr) {
 return libc_futex_lock64(uaddr,NULL);
}
CRT_EXCEPT void LIBCCALL
libc_Xfutex_lock_inf(futex_t *uaddr) {
 libc_Xfutex_lock64(uaddr,NULL);
}
#else
CRT_KOS int LIBCCALL libc_futex_lock_inf(futex_t *uaddr) {
 SPIN_WHILE(!libc_futex_trylock(uaddr));
 return (int)Esys_futex(uaddr,FUTEX_LOCK_PI,0,NULL,NULL,0);
}
CRT_EXCEPT void LIBCCALL
libc_Xfutex_lock_inf(futex_t *uaddr) {
 XSPIN_WHILE(!libc_futex_trylock(uaddr));
 Xsys_futex(uaddr,FUTEX_LOCK_PI,0,NULL,NULL,0);
}
#endif

EXPORT(futex_lock64,libc_futex_lock64);
CRT_KOS int LIBCCALL
libc_futex_lock64(futex_t *uaddr, struct timespec64 const *abs_timeout) {
 SPIN_WHILE(!libc_futex_trylock(uaddr));
 return (int)Esys_futex(uaddr,FUTEX_LOCK_PI,0,abs_timeout,NULL,0);
}
EXPORT(Xfutex_lock64,libc_Xfutex_lock64);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_lock64(futex_t *uaddr, struct timespec64 const *abs_timeout) {
 XSPIN_WHILE(!libc_futex_trylock(uaddr),true);
 return Xsys_futex(uaddr,FUTEX_LOCK_PI,0,abs_timeout,NULL,0) != -ETIMEDOUT;
}

EXPORT(futex_lock,libc_futex_lock);
CRT_KOS int LIBCCALL
libc_futex_lock(futex_t *uaddr, struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_lock_inf(uaddr);
 if (libc_loadtimespec64(&t64,abs_timeout)) return -1;
 return libc_futex_lock64(uaddr,&t64);
}

EXPORT(Xfutex_lock,libc_Xfutex_lock);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_lock(futex_t *uaddr,
                 struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return (libc_Xfutex_lock_inf(uaddr),true);
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_lock64(uaddr,&t64);
}



#else


EXPORT(futex_wait_inf,libc_futex_wait_inf);
CRT_KOS int LIBCCALL
libc_futex_wait_inf(futex_t *uaddr, futex_t probe_value) {
 return libc_futex64(uaddr,FUTEX_WAIT,probe_value,NULL,NULL,0);
}
EXPORT(futex_wait_inf_cmpxch,libc_futex_wait_inf_cmpxch);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_cmpxch(futex_t *uaddr, futex_t old_value,
                           futex_t new_value, futex_t *update_target) {
 return libc_futex64(uaddr,FUTEX_WAIT_CMPXCH,old_value,
                     NULL,update_target,new_value);
}

EXPORT(futex_wait_inf_cmpxch2,libc_futex_wait_inf_cmpxch2);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_cmpxch2(futex_t *uaddr, futex_t old_value,
                            futex_t new_value, futex_t *update_target) {
 THREAD_POLL_BEFORE_CONNECT({
  if (ATOMIC_CMPXCH(*uaddr,old_value,new_value)) {
   libc_seterrno(EAGAIN);
   return -1;
  }
 });
 return libc_futex64(uaddr,FUTEX_WAIT_CMPXCH2,old_value,
                     NULL,update_target,new_value);
}

EXPORT(futex_wait_inf_mask,libc_futex_wait_inf_mask);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_mask(futex_t *uaddr, futex_t probe_mask,
                         futex_t probe_value, futex_t enable_bits) {
 return libc_futex64(uaddr,FUTEX_WAIT_MASK,probe_mask,NULL,
                    (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(futex_wait_inf_nmask,libc_futex_wait_inf_nmask);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_nmask(futex_t *uaddr, futex_t probe_mask,
                          futex_t probe_value, futex_t enable_bits) {
 return libc_futex64(uaddr,FUTEX_WAIT_NMASK,probe_mask,NULL,
                    (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(futex_wait_inf_bitset,libc_futex_wait_inf_bitset);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_bitset(futex_t *uaddr, futex_t probe_value,
                           futex_channel_t channels) {
 return libc_futex64(uaddr,FUTEX_WAIT_BITSET,probe_value,NULL,NULL,channels);
}

EXPORT(futex_wait,libc_futex_wait);
CRT_KOS int LIBCCALL
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

EXPORT(futex_wait_cmpxch,libc_futex_wait_cmpxch);
CRT_KOS int LIBCCALL
libc_futex_wait_cmpxch(futex_t *uaddr, futex_t old_value,
                       futex_t new_value, futex_t *update_target,
                       struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_cmpxch(uaddr,old_value,new_value,update_target);
 LIBC_TRY {
  t64.tv_sec  = abs_timeout->tv_sec;
  t64.tv_nsec = abs_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64_cmpxch(uaddr,old_value,new_value,update_target,&t64);
}

EXPORT(futex_wait_cmpxch2,libc_futex_wait_cmpxch2);
CRT_KOS int LIBCCALL
libc_futex_wait_cmpxch2(futex_t *uaddr, futex_t old_value,
                        futex_t new_value, futex_t *update_target,
                        struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_cmpxch2(uaddr,old_value,new_value,update_target);
 LIBC_TRY {
  t64.tv_sec  = abs_timeout->tv_sec;
  t64.tv_nsec = abs_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64_cmpxch2(uaddr,old_value,new_value,update_target,&t64);
}

EXPORT(futex_wait_mask,libc_futex_wait_mask);
CRT_KOS int LIBCCALL
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

EXPORT(futex_wait_nmask,libc_futex_wait_nmask);
CRT_KOS int LIBCCALL
libc_futex_wait_nmask(futex_t *uaddr, futex_t probe_mask,
                      futex_t probe_value, futex_t enable_bits,
                      struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_nmask(uaddr,probe_mask,probe_value,enable_bits);
 LIBC_TRY {
  t64.tv_sec  = abs_timeout->tv_sec;
  t64.tv_nsec = abs_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64_nmask(uaddr,probe_mask,probe_value,enable_bits,&t64);
}

EXPORT(futex_wait_bitset,libc_futex_wait_bitset);
CRT_KOS int LIBCCALL
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
CRT_KOS int LIBCCALL
libc_futex_wait64(futex_t *uaddr, futex_t probe_value,
                  struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_BITSET,probe_value,
                     abs_timeout,NULL,FUTEX_BITSET_MATCH_ANY);
}

EXPORT(futex_wait64_cmpxch,libc_futex_wait64_cmpxch);
CRT_KOS int LIBCCALL
libc_futex_wait64_cmpxch(futex_t *uaddr, futex_t old_value,
                         futex_t new_value, futex_t *update_target,
                         struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_CMPXCH,old_value,
                     abs_timeout,update_target,new_value);
}

EXPORT(futex_wait64_cmpxch2,libc_futex_wait64_cmpxch2);
CRT_KOS int LIBCCALL
libc_futex_wait64_cmpxch2(futex_t *uaddr, futex_t old_value,
                          futex_t new_value, futex_t *update_target,
                          struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_CMPXCH,old_value,
                     abs_timeout,update_target,new_value);
}

EXPORT(futex_wait64_mask,libc_futex_wait64_mask);
CRT_KOS int LIBCCALL
libc_futex_wait64_mask(futex_t *uaddr, futex_t probe_mask,
                       futex_t probe_value, futex_t enable_bits,
                       struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_MASK,probe_mask,abs_timeout,
                    (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(futex_wait64_nmask,libc_futex_wait64_nmask);
CRT_KOS int LIBCCALL
libc_futex_wait64_nmask(futex_t *uaddr, futex_t probe_mask,
                        futex_t probe_value, futex_t enable_bits,
                        struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_NMASK,probe_mask,abs_timeout,
                    (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(futex_wait64_bitset,libc_futex_wait64_bitset);
CRT_KOS int LIBCCALL
libc_futex_wait64_bitset(futex_t *uaddr, futex_t probe_value,
                         futex_channel_t channels,
                         struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_BITSET,probe_value,
                     abs_timeout,NULL,channels);
}

EXPORT(futex_waitrel,libc_futex_waitrel);
CRT_KOS int LIBCCALL
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
CRT_KOS int LIBCCALL
libc_futex_waitrel64(futex_t *uaddr, futex_t probe_value,
                     struct timespec64 const *rel_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT,probe_value,rel_timeout,NULL,0);
}


EXPORT(futex_wait_inf_ghost,libc_futex_wait_inf_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_ghost(futex_t *uaddr, futex_t probe_value) {
 return libc_futex64(uaddr,FUTEX_WAIT_GHOST,probe_value,NULL,NULL,0);
}

EXPORT(futex_wait_inf_cmpxch_ghost,libc_futex_wait_inf_cmpxch_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_cmpxch_ghost(futex_t *uaddr, futex_t old_value,
                                 futex_t new_value, futex_t *update_target) {
 return libc_futex64(uaddr,FUTEX_WAIT_CMPXCH_GHOST,old_value,
                     NULL,update_target,new_value);
}

EXPORT(futex_wait_inf_cmpxch2_ghost,libc_futex_wait_inf_cmpxch2_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_cmpxch2_ghost(futex_t *uaddr, futex_t old_value,
                                  futex_t new_value, futex_t *update_target) {
 return libc_futex64(uaddr,FUTEX_WAIT_CMPXCH2_GHOST,old_value,
                     NULL,update_target,new_value);
}

EXPORT(futex_wait_inf_mask_ghost,libc_futex_wait_inf_mask_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                               futex_t probe_value, futex_t enable_bits) {
 return libc_futex64(uaddr,FUTEX_WAIT_MASK_GHOST,probe_mask,NULL,
                    (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(futex_wait_inf_nmask_ghost,libc_futex_wait_inf_nmask_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_nmask_ghost(futex_t *uaddr, futex_t probe_mask,
                                futex_t probe_value, futex_t enable_bits) {
 return libc_futex64(uaddr,FUTEX_WAIT_NMASK_GHOST,probe_mask,NULL,
                    (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(futex_wait_inf_bitset_ghost,libc_futex_wait_inf_bitset_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_inf_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                                 futex_channel_t channels) {
 return libc_futex64(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,NULL,NULL,channels);
}

EXPORT(futex_wait_ghost,libc_futex_wait_ghost);
CRT_KOS int LIBCCALL
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

EXPORT(futex_wait_cmpxch_ghost,libc_futex_wait_cmpxch_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_cmpxch_ghost(futex_t *uaddr, futex_t old_value,
                             futex_t new_value, futex_t *update_target,
                             struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_cmpxch_ghost(uaddr,old_value,new_value,update_target);
 LIBC_TRY {
  t64.tv_sec  = abs_timeout->tv_sec;
  t64.tv_nsec = abs_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64_cmpxch_ghost(uaddr,old_value,new_value,update_target,&t64);
}

EXPORT(futex_wait_cmpxch2_ghost,libc_futex_wait_cmpxch2_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_cmpxch2_ghost(futex_t *uaddr, futex_t old_value,
                              futex_t new_value, futex_t *update_target,
                              struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_cmpxch2_ghost(uaddr,old_value,new_value,update_target);
 LIBC_TRY {
  t64.tv_sec  = abs_timeout->tv_sec;
  t64.tv_nsec = abs_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64_cmpxch2_ghost(uaddr,old_value,new_value,update_target,&t64);
}

EXPORT(futex_wait_mask_ghost,libc_futex_wait_mask_ghost);
CRT_KOS int LIBCCALL
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

EXPORT(futex_wait_nmask_ghost,libc_futex_wait_nmask_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait_nmask_ghost(futex_t *uaddr, futex_t probe_mask,
                            futex_t probe_value, futex_t enable_bits,
                            struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout)
      return libc_futex_wait_inf_nmask_ghost(uaddr,probe_mask,probe_value,enable_bits);
 LIBC_TRY {
  t64.tv_sec  = abs_timeout->tv_sec;
  t64.tv_nsec = abs_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_futex_wait64_nmask_ghost(uaddr,probe_mask,probe_value,enable_bits,&t64);
}

EXPORT(futex_wait_bitset_ghost,libc_futex_wait_bitset_ghost);
CRT_KOS int LIBCCALL
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
CRT_KOS int LIBCCALL
libc_futex_wait64_ghost(futex_t *uaddr, futex_t probe_value,
                        struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,
                     abs_timeout,NULL,FUTEX_BITSET_MATCH_ANY);
}

EXPORT(futex_wait64_cmpxch_ghost,libc_futex_wait64_cmpxch_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait64_cmpxch_ghost(futex_t *uaddr, futex_t old_value,
                               futex_t new_value, futex_t *update_target,
                               struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_CMPXCH_GHOST,old_value,
                     abs_timeout,update_target,new_value);
}

EXPORT(futex_wait64_cmpxch2_ghost,libc_futex_wait64_cmpxch2_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait64_cmpxch2_ghost(futex_t *uaddr, futex_t old_value,
                                futex_t new_value, futex_t *update_target,
                                struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_CMPXCH2_GHOST,old_value,
                     abs_timeout,update_target,new_value);
}

EXPORT(futex_wait64_mask_ghost,libc_futex_wait64_mask_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait64_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                             futex_t probe_value, futex_t enable_bits,
                             struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_MASK_GHOST,probe_mask,abs_timeout,
                    (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(futex_wait64_nmask_ghost,libc_futex_wait64_nmask_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait64_nmask_ghost(futex_t *uaddr, futex_t probe_mask,
                              futex_t probe_value, futex_t enable_bits,
                              struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_NMASK_GHOST,probe_mask,abs_timeout,
                    (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(futex_wait64_bitset_ghost,libc_futex_wait64_bitset_ghost);
CRT_KOS int LIBCCALL
libc_futex_wait64_bitset_ghost(futex_t *uaddr, futex_t probe_value,
                               futex_channel_t channels,
                               struct timespec64 const *abs_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_BITSET_GHOST,probe_value,
                     abs_timeout,NULL,channels);
}

EXPORT(futex_waitrel_ghost,libc_futex_waitrel_ghost);
CRT_KOS int LIBCCALL
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
CRT_KOS int LIBCCALL
libc_futex_waitrel64_ghost(futex_t *uaddr, futex_t probe_value,
                           struct timespec64 const *rel_timeout) {
 return libc_futex64(uaddr,FUTEX_WAIT_GHOST,probe_value,rel_timeout,NULL,0);
}



EXPORT(Xfutex_wait_inf,libc_Xfutex_wait_inf);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf(futex_t *uaddr, futex_t probe_value) {
 libc_Xfutex64(uaddr,FUTEX_WAIT,probe_value,NULL,NULL,0);
}

EXPORT(Xfutex_wait_inf_cmpxch,libc_Xfutex_wait_inf_cmpxch);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf_cmpxch(futex_t *uaddr, futex_t old_value,
                            futex_t new_value, futex_t *update_target) {
 libc_Xfutex64(uaddr,FUTEX_WAIT_CMPXCH,old_value,
               NULL,update_target,new_value);
}

EXPORT(Xfutex_wait_inf_cmpxch2,libc_Xfutex_wait_inf_cmpxch2);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf_cmpxch2(futex_t *uaddr, futex_t old_value,
                             futex_t new_value, futex_t *update_target) {
 libc_Xfutex64(uaddr,FUTEX_WAIT_CMPXCH2,old_value,
               NULL,update_target,new_value);
}

EXPORT(Xfutex_wait_inf_mask,libc_Xfutex_wait_inf_mask);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf_mask(futex_t *uaddr, futex_t probe_mask,
                          futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex64(uaddr,FUTEX_WAIT_MASK,probe_mask,NULL,
              (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(Xfutex_wait_inf_nmask,libc_Xfutex_wait_inf_nmask);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf_nmask(futex_t *uaddr, futex_t probe_mask,
                           futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex64(uaddr,FUTEX_WAIT_NMASK,probe_mask,NULL,
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

EXPORT(Xfutex_wait_cmpxch,libc_Xfutex_wait_cmpxch);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait_cmpxch(futex_t *uaddr, futex_t old_value,
                        futex_t new_value, futex_t *update_target,
                        struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout) {
  libc_Xfutex_wait_inf_cmpxch(uaddr,old_value,new_value,update_target);
  return true;
 }
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_cmpxch(uaddr,old_value,new_value,update_target,&t64);
}

EXPORT(Xfutex_wait_cmpxch2,libc_Xfutex_wait_cmpxch2);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait_cmpxch2(futex_t *uaddr, futex_t old_value,
                         futex_t new_value, futex_t *update_target,
                         struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout) {
  libc_Xfutex_wait_inf_cmpxch2(uaddr,old_value,new_value,update_target);
  return true;
 }
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_cmpxch2(uaddr,old_value,new_value,update_target,&t64);
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

EXPORT(Xfutex_wait_nmask,libc_Xfutex_wait_nmask);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait_nmask(futex_t *uaddr, futex_t probe_mask,
                       futex_t probe_value, futex_t enable_bits,
                       struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout) {
  libc_Xfutex_wait_inf_nmask(uaddr,probe_mask,probe_value,enable_bits);
  return true;
 }
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_nmask(uaddr,probe_mask,probe_value,enable_bits,&t64);
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

EXPORT(Xfutex_wait64_cmpxch,libc_Xfutex_wait64_cmpxch);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64_cmpxch(futex_t *uaddr, futex_t old_value,
                          futex_t new_value, futex_t *update_target,
                          struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_CMPXCH,old_value,
                      abs_timeout,update_target,new_value) != -ETIMEDOUT;
}

EXPORT(Xfutex_wait64_cmpxch2,libc_Xfutex_wait64_cmpxch2);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64_cmpxch2(futex_t *uaddr, futex_t old_value,
                           futex_t new_value, futex_t *update_target,
                           struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_CMPXCH2,old_value,
                      abs_timeout,update_target,new_value) != -ETIMEDOUT;
}

EXPORT(Xfutex_wait64_mask,libc_Xfutex_wait64_mask);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64_mask(futex_t *uaddr, futex_t probe_mask,
                        futex_t probe_value, futex_t enable_bits,
                        struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_MASK,probe_mask,abs_timeout,
                     (u32 *)(uintptr_t)probe_value,enable_bits) != -ETIMEDOUT;
}

EXPORT(Xfutex_wait64_nmask,libc_Xfutex_wait64_nmask);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64_nmask(futex_t *uaddr, futex_t probe_mask,
                         futex_t probe_value, futex_t enable_bits,
                         struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_NMASK,probe_mask,abs_timeout,
                     (u32 *)(uintptr_t)probe_value,enable_bits) != -ETIMEDOUT;
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

EXPORT(Xfutex_wait_inf_cmpxch_ghost,libc_Xfutex_wait_inf_cmpxch_ghost);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf_cmpxch_ghost(futex_t *uaddr, futex_t old_value,
                                  futex_t new_value, futex_t *update_target) {
 libc_Xfutex64(uaddr,FUTEX_WAIT_CMPXCH_GHOST,old_value,
               NULL,update_target,new_value);
}

EXPORT(Xfutex_wait_inf_cmpxch2_ghost,libc_Xfutex_wait_inf_cmpxch2_ghost);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf_cmpxch2_ghost(futex_t *uaddr, futex_t old_value,
                                   futex_t new_value, futex_t *update_target) {
 libc_Xfutex64(uaddr,FUTEX_WAIT_CMPXCH2_GHOST,old_value,
               NULL,update_target,new_value);
}

EXPORT(Xfutex_wait_inf_mask_ghost,libc_Xfutex_wait_inf_mask_ghost);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                                futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex64(uaddr,FUTEX_WAIT_MASK_GHOST,probe_mask,NULL,
              (u32 *)(uintptr_t)probe_value,enable_bits);
}

EXPORT(Xfutex_wait_inf_nmask_ghost,libc_Xfutex_wait_inf_nmask_ghost);
CRT_EXCEPT void LIBCCALL
libc_Xfutex_wait_inf_nmask_ghost(futex_t *uaddr, futex_t probe_mask,
                                 futex_t probe_value, futex_t enable_bits) {
 libc_Xfutex64(uaddr,FUTEX_WAIT_NMASK_GHOST,probe_mask,NULL,
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

EXPORT(Xfutex_wait_cmpxch_ghost,libc_Xfutex_wait_cmpxch_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait_cmpxch_ghost(futex_t *uaddr, futex_t old_value,
                              futex_t new_value, futex_t *update_target,
                              struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout) {
  libc_Xfutex_wait_inf_cmpxch_ghost(uaddr,old_value,new_value,update_target);
  return true;
 }
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_cmpxch_ghost(uaddr,old_value,new_value,update_target,&t64);
}

EXPORT(Xfutex_wait_cmpxch2_ghost,libc_Xfutex_wait_cmpxch2_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait_cmpxch2_ghost(futex_t *uaddr, futex_t old_value,
                               futex_t new_value, futex_t *update_target,
                               struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout) {
  libc_Xfutex_wait_inf_cmpxch2_ghost(uaddr,old_value,new_value,update_target);
  return true;
 }
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_cmpxch2_ghost(uaddr,old_value,new_value,update_target,&t64);
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

EXPORT(Xfutex_wait_nmask_ghost,libc_Xfutex_wait_nmask_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait_nmask_ghost(futex_t *uaddr, futex_t probe_mask,
                             futex_t probe_value, futex_t enable_bits,
                             struct timespec32 const *abs_timeout) {
 struct timespec64 t64;
 if (!abs_timeout) {
  libc_Xfutex_wait_inf_nmask_ghost(uaddr,probe_mask,probe_value,enable_bits);
  return true;
 }
 t64.tv_sec  = abs_timeout->tv_sec;
 t64.tv_nsec = abs_timeout->tv_nsec;
 return libc_Xfutex_wait64_nmask_ghost(uaddr,probe_mask,probe_value,enable_bits,&t64);
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

EXPORT(Xfutex_wait64_cmpxch_ghost,libc_Xfutex_wait64_cmpxch_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64_cmpxch_ghost(futex_t *uaddr, futex_t old_value,
                                futex_t new_value, futex_t *update_target,
                                struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_CMPXCH_GHOST,old_value,
                      abs_timeout,update_target,new_value) != -ETIMEDOUT;
}

EXPORT(Xfutex_wait64_cmpxch2_ghost,libc_Xfutex_wait64_cmpxch2_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64_cmpxch2_ghost(futex_t *uaddr, futex_t old_value,
                                 futex_t new_value, futex_t *update_target,
                                 struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_CMPXCH2_GHOST,old_value,
                      abs_timeout,update_target,new_value) != -ETIMEDOUT;
}

EXPORT(Xfutex_wait64_mask_ghost,libc_Xfutex_wait64_mask_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64_mask_ghost(futex_t *uaddr, futex_t probe_mask,
                              futex_t probe_value, futex_t enable_bits,
                              struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_MASK_GHOST,probe_mask,abs_timeout,
                     (u32 *)(uintptr_t)probe_value,enable_bits) != -ETIMEDOUT;
}

EXPORT(Xfutex_wait64_nmask_ghost,libc_Xfutex_wait64_nmask_ghost);
CRT_EXCEPT bool LIBCCALL
libc_Xfutex_wait64_nmask_ghost(futex_t *uaddr, futex_t probe_mask,
                               futex_t probe_value, futex_t enable_bits,
                               struct timespec64 const *abs_timeout) {
 return libc_Xfutex64(uaddr,FUTEX_WAIT_NMASK_GHOST,probe_mask,abs_timeout,
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

EXPORT(futex_lock_inf,libc_futex_lock_inf);
CRT_KOS int LIBCCALL libc_futex_lock_inf(futex_t *uaddr) {
 return (int)libc_futex64(uaddr,FUTEX_LOCK_PI,0,NULL,NULL,0);
}

EXPORT(futex_lock,libc_futex_lock);
CRT_KOS int LIBCCALL
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
CRT_KOS int LIBCCALL
libc_futex_lock64(futex_t *uaddr, struct timespec64 const *abs_timeout) {
 return (int)libc_futex64(uaddr,FUTEX_LOCK_PI,0,abs_timeout,NULL,0);
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


#endif



EXPORT(futex_wake,libc_futex_wake);
CRT_KOS ssize_t LIBCCALL
libc_futex_wake(futex_t *uaddr, size_t num_threads) {
 return (ssize_t)Esys_futex(uaddr,FUTEX_WAKE,(uintptr_t)num_threads,NULL,NULL,0);
}

EXPORT(futex_wake_bitset,libc_futex_wake_bitset);
CRT_KOS ssize_t LIBCCALL
libc_futex_wake_bitset(futex_t *uaddr, size_t num_threads,
                       futex_channel_t channels) {
 return (ssize_t)Esys_futex(uaddr,FUTEX_WAKE_BITSET,(uintptr_t)num_threads,NULL,NULL,channels);
}

EXPORT(futex_unlock,libc_futex_unlock);
CRT_KOS ssize_t LIBCCALL libc_futex_unlock(futex_t *uaddr) {
 return (ssize_t)Esys_futex(uaddr,FUTEX_UNLOCK_PI,0,NULL,NULL,0);
}

EXPORT(futex_waitfd,libc_futex_waitfd);
CRT_KOS fd_t LIBCCALL
libc_futex_waitfd(futex_t *uaddr, futex_t probe_value, oflag_t oflags) {
 return (fd_t)Esys_futex(uaddr,FUTEX_WAITFD,probe_value,
                        (struct timespec64 *)(uintptr_t)oflags,
                         NULL,0);
}

EXPORT(futex_waitfd_bitset,libc_futex_waitfd_bitset);
CRT_KOS fd_t LIBCCALL
libc_futex_waitfd_bitset(futex_t *uaddr, futex_t probe_value,
                         oflag_t oflags, futex_channel_t channels) {
 return (fd_t)Esys_futex(uaddr,FUTEX_WAITFD,probe_value,
                        (struct timespec64 *)(uintptr_t)oflags,
                         NULL,channels);
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
CRT_KOS ssize_t LIBCCALL
libc_xppoll(struct pollfd *ufds, size_t nfds,
            struct pollfutex *uftx, size_t nftx,
            struct pollpid *upid, size_t npid,
            struct timespec const *rel_timeout, sigset_t *sig) {
 struct timespec64 t64;
 if (!rel_timeout)
      return libc_xppoll64(ufds,nfds,uftx,nftx,upid,npid,NULL,sig);
 LIBC_TRY {
  t64.tv_sec  = rel_timeout->tv_sec;
  t64.tv_nsec = rel_timeout->tv_nsec;
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 return libc_xppoll64(ufds,nfds,uftx,nftx,upid,npid,&t64,sig);
}


EXPORT(xppoll64,libc_xppoll64);
CRT_KOS ssize_t LIBCCALL
libc_xppoll64(struct pollfd *ufds, size_t nfds,
              struct pollfutex *uftx, size_t nftx,
              struct pollpid *upid, size_t npid,
              struct timespec64 const *rel_timeout, sigset_t *sig) {
 struct __os_pollinfo info;
 info.i_ufdvec = ufds;
 info.i_ufdcnt = nfds;
 info.i_ftxvec = uftx;
 info.i_ftxcnt = nftx;
 info.i_pidvec = upid;
 info.i_pidcnt = npid;
 return Esys_xppoll(&info,rel_timeout,sig,sizeof(sigset_t));
}

EXPORT(Xxppoll,libc_Xxppoll);
CRT_EXCEPT size_t LIBCCALL
libc_Xxppoll(struct pollfd *ufds, size_t nfds,
             struct pollfutex *uftx, size_t nftx,
             struct pollpid *upid, size_t npid,
             struct timespec const *rel_timeout, sigset_t *sig) {
 struct timespec64 t64;
 if (!rel_timeout)
      return libc_Xxppoll64(ufds,nfds,uftx,nftx,upid,npid,NULL,sig);
 t64.tv_sec  = rel_timeout->tv_sec;
 t64.tv_nsec = rel_timeout->tv_nsec;
 return libc_Xxppoll64(ufds,nfds,uftx,nftx,upid,npid,&t64,sig);
}

EXPORT(Xxppoll64,libc_Xxppoll64);
CRT_EXCEPT size_t LIBCCALL
libc_Xxppoll64(struct pollfd *ufds, size_t nfds,
               struct pollfutex *uftx, size_t nftx,
               struct pollpid *upid, size_t npid,
               struct timespec64 const *rel_timeout, sigset_t *sig) {
 struct __os_pollinfo info;
 info.i_ufdvec = ufds;
 info.i_ufdcnt = nfds;
 info.i_ftxvec = uftx;
 info.i_ftxcnt = nftx;
 info.i_pidvec = upid;
 info.i_pidcnt = npid;
 return Xsys_xppoll(&info,rel_timeout,sig,sizeof(sigset_t));
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_FUTEX_C */
