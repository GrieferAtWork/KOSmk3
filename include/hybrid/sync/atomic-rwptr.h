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
#ifndef __GUARD_HYBRID_SYNC_ATOMIC_RWPTR_H
#define __GUARD_HYBRID_SYNC_ATOMIC_RWPTR_H 1

#include <hybrid/compiler.h>
#include <assert.h>
#include <hybrid/atomic.h>
#include <hybrid/critical.h>
#include <hybrid/sched/yield.h>
#include <hybrid/byteorder.h>

DECL_BEGIN

/* A small R/W-lock combined with a 16-byte (ATOMIC_RWPTR_ALIGN) aligned user-pointer. */

#define ATOMIC_RWPTR_ALIGN        16 /* Keep the lower 4 bits unused. */
#define ATOMIC_RWPTR_ADDR_MASK (~(__UINTPTR_TYPE__)(ATOMIC_RWPTR_ALIGN-1))
#define ATOMIC_RWPTR_LOCK_MASK  ((__UINTPTR_TYPE__)(ATOMIC_RWPTR_ALIGN-1))
#define ATOMIC_RWPTR_SHIFT_RCNT   0
#define ATOMIC_RWPTR_SHIFT_WMODE  3
#define ATOMIC_RWPTR_MASK_RCNT    0x7 /* 0111. */
#define ATOMIC_RWPTR_MASK_WMODE   0x8 /* 1000. */

#ifdef __CC__
typedef struct __ATTR_PACKED atomic_rwptr {
    union __ATTR_PACKED {
        __UINTPTR_TYPE__          ap_data; /* Pointer data. */
        __UINTPTR_TYPE__          ap_lock; /* A very special R/W-lock that lives within the lower 4 bits. */
        void                     *ap_ptr;  /* [0..1][lock(ip_lock)] User-defined pointer.
                                            *  NOTE: Must be aligned by `ATOMIC_RWPTR_ALIGN'. */
#if __BYTE_ORDER == __LITTLE_ENDIAN
        __BYTE_TYPE__             ap_lbyt; /* Lock byte. */
#elif __BYTE_ORDER == __LITTLE_ENDIAN
       struct __ATTR_PACKED {
        __BYTE_TYPE__             ap_pad[sizeof(__UINTPTR_TYPE__)-1];
        __BYTE_TYPE__             ap_lbyt; /* Lock byte. */};
#else
#   error FIXME
#endif
    };
} atomic_rwptr_t;

#define ATOMIC_RWPTR_INIT(p)      {{(__UINTPTR_TYPE__)(p)}}
#define atomic_rwptr_cinit(self)    (void)assert((self)->ap_data == 0)
#define atomic_rwptr_init(self)     (void)((self)->ap_data = 0)
#define DEFINE_ATOMIC_RWPTR(name,p)  atomic_rwptr_t name = ATOMIC_RWPTR_INIT(p)

#define atomic_rwptr_reading(x)   (ATOMIC_READ((x)->ap_data)&(ATOMIC_RWPTR_MASK_WMODE|ATOMIC_RWPTR_MASK_RCNT))
#define atomic_rwptr_writing(x)   (ATOMIC_READ((x)->ap_data)&(ATOMIC_RWPTR_MASK_WMODE))

/* Get the value of the atomic pointer.
 * NOTE: The caller must hold a read-lock! */
#define ATOMIC_RWPTR_GET(self)    ((void *)((self).ap_data&ATOMIC_RWPTR_ADDR_MASK))


/* Acquire an exclusive read/write lock. */
__LOCAL __BOOL __LIBCCALL atomic_rwptr_tryread(atomic_rwptr_t *__restrict self);
__LOCAL __BOOL __LIBCCALL atomic_rwptr_trywrite(atomic_rwptr_t *__restrict self);
__LOCAL void __LIBCCALL atomic_rwptr_read(atomic_rwptr_t *__restrict self);
__LOCAL void __LIBCCALL atomic_rwptr_write(atomic_rwptr_t *__restrict self);

/* Try to upgrade a read-lock to a write-lock. Return `FALSE' upon failure. */
__LOCAL __BOOL __LIBCCALL atomic_rwptr_tryupgrade(atomic_rwptr_t *__restrict self);

/* NOTE: The lock is always upgraded, but when `FALSE' is returned, no lock
 *       may have been held temporarily, meaning that the caller should
 *       re-load local copies of affected resources. */
__LOCAL __BOOL __LIBCCALL atomic_rwptr_upgrade(atomic_rwptr_t *__restrict self);
__LOCAL void __LIBCCALL atomic_rwptr_downgrade(atomic_rwptr_t *__restrict self);

/* End reading/writing/either. */
__LOCAL void __LIBCCALL atomic_rwptr_endwrite(atomic_rwptr_t *__restrict self);
__LOCAL void __LIBCCALL atomic_rwptr_endread(atomic_rwptr_t *__restrict self);
__LOCAL void __LIBCCALL atomic_rwptr_end(atomic_rwptr_t *__restrict self);


__LOCAL __BOOL __LIBCCALL
atomic_rwptr_tryread(atomic_rwptr_t *__restrict self) {
 __BYTE_TYPE__ lold,lnew;
 assert(TASK_ISSAFE());
 do {
  lold = ATOMIC_READ(self->ap_lbyt);
  if ((lold&ATOMIC_RWPTR_MASK_WMODE) ||  /* Fail when the pointer is in write-mode. */
      (lold&ATOMIC_RWPTR_MASK_RCNT) == ATOMIC_RWPTR_MASK_RCNT) /* Fail it there are reader slots available. */
       return 0;
  lnew = lold+(1 << ATOMIC_RWPTR_SHIFT_RCNT);
 } while (!ATOMIC_CMPXCH_WEAK(self->ap_lbyt,lold,lnew));
 COMPILER_READ_BARRIER();
 return 1;
}
__LOCAL void __LIBCCALL
atomic_rwptr_read(atomic_rwptr_t *__restrict self) {
 while (!atomic_rwptr_tryread(self)) SCHED_YIELD();
 COMPILER_READ_BARRIER();
}
__LOCAL void __LIBCCALL
atomic_rwptr_endread(atomic_rwptr_t *__restrict self) {
#ifdef CONFIG_DEBUG
 __BYTE_TYPE__ lold;
 COMPILER_READ_BARRIER();
 assert(TASK_ISSAFE());
 do lold = ATOMIC_READ(self->ap_lbyt);
 while (!ATOMIC_CMPXCH_WEAK(self->ap_lbyt,lold,lold-(1 << ATOMIC_RWPTR_SHIFT_RCNT)));
 __assertf((lold&ATOMIC_RWPTR_MASK_RCNT) != 0,"No readers registers.");
#elif ATOMIC_RWPTR_SHIFT_RCNT
 COMPILER_READ_BARRIER();
 ATOMIC_FETCHSUB(self->ap_lbyt,1 << ATOMIC_RWPTR_SHIFT_RCNT);
#else
 COMPILER_READ_BARRIER();
 ATOMIC_FETCHDEC(self->ap_lbyt);
#endif
}


__LOCAL __BOOL __LIBCCALL
atomic_rwptr_trywrite(atomic_rwptr_t *__restrict self) {
 __BYTE_TYPE__ lold;
 assert(TASK_ISSAFE());
 do {
  lold = ATOMIC_READ(self->ap_lbyt);
  /* Wait while the lock already is in write-mode or has readers. */
  if ((lold&(ATOMIC_RWPTR_MASK_WMODE|ATOMIC_RWPTR_MASK_RCNT)) != 0) return 0;
 } while (!ATOMIC_CMPXCH(self->ap_lbyt,lold,lold|ATOMIC_RWPTR_MASK_WMODE));
 COMPILER_BARRIER();
 return 1;
}
__LOCAL void __LIBCCALL
atomic_rwptr_write(atomic_rwptr_t *__restrict self) {
 while (!atomic_rwptr_trywrite(self)) SCHED_YIELD();
 COMPILER_BARRIER();
}
__LOCAL void __LIBCCALL
atomic_rwptr_endwrite(atomic_rwptr_t *__restrict self) {
#ifdef CONFIG_DEBUG
 __BYTE_TYPE__ lold;
 COMPILER_BARRIER();
 assert(TASK_ISSAFE());
 do lold = ATOMIC_READ(self->ap_lbyt);
 while (!ATOMIC_CMPXCH(self->ap_lbyt,lold,lold&~(ATOMIC_RWPTR_MASK_WMODE)));
 __assertf((lold&ATOMIC_RWPTR_MASK_WMODE) != 0,"Not in write mode.");
#else
 COMPILER_BARRIER();
 assert(TASK_ISSAFE());
 ATOMIC_FETCHAND(self->ap_lbyt,~(ATOMIC_RWPTR_MASK_WMODE));
#endif
}
__LOCAL void __LIBCCALL
atomic_rwptr_end(atomic_rwptr_t *__restrict self) {
 __UINTPTR_TYPE__ temp,newval;
 COMPILER_BARRIER();
 assert(TASK_ISSAFE());
 do {
  temp = ATOMIC_READ(self->ap_data);
  if (temp&ATOMIC_RWPTR_MASK_WMODE) {
   assert(!(temp&ATOMIC_RWPTR_MASK_RCNT));
   newval = temp&~ATOMIC_RWPTR_MASK_WMODE;
  } else {
   __assertf((temp&ATOMIC_RWPTR_MASK_RCNT) != 0,
             "You're not holding any locks");
   newval = temp-1;
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->ap_data,temp,newval));
}


__LOCAL __BOOL __LIBCCALL
atomic_rwptr_tryupgrade(atomic_rwptr_t *__restrict self) {
 __BYTE_TYPE__ lold;
 assert(TASK_ISSAFE());
 do {
  lold = ATOMIC_LOAD(self->ap_lbyt);
  __assertf(((lold&ATOMIC_RWPTR_MASK_RCNT) >> ATOMIC_RWPTR_SHIFT_RCNT) >= 1,
            "You're not holding a read-lock");
  /* Wait while the lock already is in write-mode or has readers. */
  if ((lold&ATOMIC_RWPTR_MASK_RCNT) !=
      (1 << ATOMIC_RWPTR_SHIFT_RCNT)) return 0; /* There is more than one reader. */
 } while (!ATOMIC_CMPXCH(self->ap_lbyt,lold,
         (lold&~ATOMIC_RWPTR_MASK_RCNT)|ATOMIC_RWPTR_MASK_WMODE));
 COMPILER_WRITE_BARRIER();
 return 1; /* Successfully upgraded the read-lock. */
}

__LOCAL __BOOL __LIBCCALL
atomic_rwptr_upgrade(atomic_rwptr_t *__restrict self) {
 if (atomic_rwptr_tryupgrade(self)) return 1;
 atomic_rwptr_endread(self);
 atomic_rwptr_write(self);
 return 0;
}
__LOCAL void __LIBCCALL
atomic_rwptr_downgrade(atomic_rwptr_t *__restrict self) {
 __BYTE_TYPE__ lold;
 COMPILER_WRITE_BARRIER();
 assert(TASK_ISSAFE());
 do {
  lold = ATOMIC_READ(self->ap_lbyt);
  __assertf((lold&ATOMIC_RWPTR_LOCK_MASK) == ATOMIC_RWPTR_MASK_WMODE,
            "Lock not in write-mode (%x)",lold&ATOMIC_RWPTR_LOCK_MASK);
 } while (!ATOMIC_CMPXCH_WEAK(self->ap_lbyt,lold,
         (lold&~ATOMIC_RWPTR_MASK_WMODE)|1));
}

#endif /* __CC__ */

DECL_END

#endif /* !__GUARD_HYBRID_SYNC_ATOMIC_RWPTR_H */
