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
#ifndef GUARD_KERNEL_SRC_VM_FUTEX_C
#define GUARD_KERNEL_SRC_VM_FUTEX_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <fs/handle.h>
#include <kernel/vm.h>
#include <kernel/malloc.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sched/pid.h>
#include <string.h>
#include <stdint.h>
#include <except.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <sys/poll.h>

DECL_BEGIN

#define CONFIG_FUTEX_CACHE_MAXSIZE  1024
union free_futex {
    union free_futex *ff_next;  /* [0..1] Next free futex. */
    struct futex      ff_futex; /* The futex that is free. */
};

PRIVATE DEFINE_ATOMIC_RWLOCK(futex_cache_lock);
PRIVATE union free_futex *futex_cache = NULL;
PRIVATE size_t futex_cache_size = 0;

LOCAL ATTR_MALLOC ATTR_RETNONNULL
struct futex *KCALL futex_alloc(void) {
 union free_futex *result;
 atomic_rwlock_write(&futex_cache_lock);
 /* Consult the cache before allocating a new futex. */
 result = futex_cache;
 if likely(result) {
  futex_cache = result->ff_next;
  assert(futex_cache_size);
  --futex_cache_size;
  atomic_rwlock_endwrite(&futex_cache_lock);
  return &result->ff_futex;
 }
 atomic_rwlock_endwrite(&futex_cache_lock);
 return (struct futex *)kmalloc(sizeof(struct futex),GFP_SHARED);
}
LOCAL void KCALL
futex_free(struct futex *__restrict self) {
 atomic_rwlock_write(&futex_cache_lock);
 if (futex_cache_size < CONFIG_FUTEX_CACHE_MAXSIZE) {
  /* Add the free futex to the cache. */
  ((union free_futex *)self)->ff_next = futex_cache;
  futex_cache = ((union free_futex *)self);
  ++futex_cache_size;
  atomic_rwlock_endwrite(&futex_cache_lock);
  return;
 }
 atomic_rwlock_endwrite(&futex_cache_lock);
 /* Free the futex as any regular heap pointer. */
 kfree(self);
}

PUBLIC size_t KCALL vm_futex_clearcache(void) {
 union free_futex *iter,*next; size_t result;
 atomic_rwlock_write(&futex_cache_lock);
 result           = futex_cache_size;
 iter             = futex_cache;
 futex_cache_size = 0;
 futex_cache      = NULL;
 atomic_rwlock_endwrite(&futex_cache_lock);
 while (iter) {
  next = iter->ff_next;
  kfree(iter);
  iter = next;
 }
 return result;
}


PUBLIC void KCALL
futex_destroy(struct futex *__restrict self) {
 struct futex *next_futex;
 assert(!self->f_refcnt);
again:
 atomic_rwptr_write(&self->f_next); /* lock `f_pself' from being modified. */
 if (self->f_pself) {
  /* Lock the self-pointer to we can update it to point to our successor. */
  if (!atomic_rwptr_trywrite(self->f_pself)) {
   /* This may fail due to race conditions. */
   atomic_rwptr_endwrite(&self->f_next);
   SCHED_YIELD();
   goto again;
  }
 }
 next_futex = (struct futex *)ATOMIC_RWPTR_GET(self->f_next);
 if (next_futex) {
  /* Must update the next->pself pointer. */
  if (!atomic_rwptr_trywrite(&next_futex->f_next)) {
   if (self->f_pself) atomic_rwptr_endwrite(self->f_pself);
   atomic_rwptr_endwrite(&self->f_next);
   goto again;
  }
  next_futex->f_pself = self->f_pself;
  /* Update the pself-pointer to point to the next futex, and unlock the self pointer. */
  if (self->f_pself) ATOMIC_STORE(self->f_pself->ap_data,(uintptr_t)next_futex);
  atomic_rwptr_endwrite(&next_futex->f_next);
 } else {
  /* Update the pself-pointer to point to the next futex, and unlock the self pointer. */
  if (self->f_pself) ATOMIC_STORE(self->f_pself->ap_data,(uintptr_t)next_futex);
 }
 /* `self' is now fully unlinked. */
 assert(!SIG_GETCON(&self->f_sig));
 futex_free(self);
}



INTDEF ATTR_NOTHROW struct vm_node *KCALL this_vm_getnode(vm_vpage_t page);
PUBLIC ATTR_RETNONNULL REF
struct futex *KCALL vm_futex(VIRT void *addr) {
 struct vm *EXCEPT_VAR my_vm = THIS_VM;
 struct vm_node *node;
 struct futex *new_futex;
 struct futex *COMPILER_IGNORE_UNINITIALIZED(result);
 struct futex *next_futex;
 atomic_rwptr_t *iter;
again:
//  assert((uintptr_t)addr);
 if ((uintptr_t)addr >= KERNEL_BASE) goto bad_addr;
 vm_acquire_read(my_vm);
 node = this_vm_getnode(VM_ADDR2PAGE((uintptr_t)addr));
 if unlikely(!node) {
  struct exception_info *info;
bad_addr_unlock:
  vm_release_read(my_vm);
bad_addr:
  info                               = error_info();
  info->e_error.e_code               = E_SEGFAULT;
  info->e_error.e_flag               = ERR_FRESUMABLE|ERR_FRESUMEFUNC;
  info->e_error.e_segfault.sf_reason = SEGFAULT_BADUSERWRITE;
  info->e_error.e_segfault.sf_vaddr  = addr;
  error_throw_current();
  goto again;
 }
 /* Got the node. */
 TRY {
  vm_raddr_t raddr;
  /* Convert the given address to become region-relative. */
  raddr = (node->vn_start * PAGESIZE) + ((uintptr_t)addr - VM_NODE_MINADDR(node));
  new_futex = NULL;
search_again:
  iter = &node->vn_region->vr_futex;
  atomic_rwptr_read(iter);
  while ((result = (struct futex *)ATOMIC_RWPTR_GET(*iter)) != NULL) {
   if (result->f_addr >= raddr) { /* Found it, or doesn't exist. */
    if (result->f_addr != raddr) {
insert_futex_into_iter:
     /* Futex objects aren't allowed in these types of regions. */
     if unlikely(node->vn_region->vr_type == VM_REGION_PHYSICAL)
        goto bad_addr_unlock;
     if unlikely(node->vn_region->vr_type == VM_REGION_RESERVED)
        goto bad_addr_unlock;

     /* Must insert a new futex here. */
     if (!new_futex) {
      atomic_rwptr_endread(iter);
      new_futex = futex_alloc();
      goto search_again;
     }
     /* Upgrade the read-lock into a write-lock. */
     if (!atomic_rwptr_upgrade(iter)) {
      atomic_rwptr_endwrite(iter);
      goto search_again;
     }
     new_futex->f_refcnt = 1;
     new_futex->f_addr   = raddr;
     sig_init(&new_futex->f_sig);
     /* Link in the new futex. */
     new_futex->f_pself = iter; /* Prepare the self-pointer for setting iter  */
     new_futex->f_next.ap_data = iter->ap_data&ATOMIC_RWPTR_ADDR_MASK;
     if ((next_futex = (struct futex *)new_futex->f_next.ap_ptr) != NULL) {
      /* Try to lock the successor (If this fails, we must unlock everything and start over!) */
      if unlikely(!atomic_rwptr_trywrite(&next_futex->f_next)) {
       atomic_rwptr_endwrite(iter);
       task_yield();
       goto search_again;
      }
      next_futex->f_pself = &new_futex->f_next;
      atomic_rwptr_endwrite(&next_futex->f_next);
     }
     ATOMIC_STORE(iter->ap_data,(uintptr_t)new_futex); /* NOTE: This like also unlocks `iter'. */
     result = new_futex;
     goto done;
    }
    if unlikely(!ATOMIC_INCIFNONZERO(result->f_refcnt)) {
     /* This can happen due to a race-condition where an
      * existing futex at the same address is currently being
      * deallocated, and was currently trying to lock its
      * self-pointer when we were faster. */
     goto insert_futex_into_iter;
    }
    atomic_rwptr_endread(iter);
    if unlikely(new_futex)
       futex_free(new_futex);
    goto done;
   }
   atomic_rwptr_read(&result->f_next);
   atomic_rwptr_endread(iter);
   iter = &result->f_next;
  }
  goto insert_futex_into_iter;
done:;
 } FINALLY {
  vm_release_read(my_vm);
 }
 return result;
}

PUBLIC REF struct futex *KCALL vm_getfutex(VIRT void *addr) {
 struct vm *EXCEPT_VAR my_vm = THIS_VM;
 struct vm_node *node;
 struct futex *new_futex;
 struct futex *COMPILER_IGNORE_UNINITIALIZED(result);
 atomic_rwptr_t *iter;
 assert((uintptr_t)addr);
 if ((uintptr_t)addr >= KERNEL_BASE) goto fail;
 vm_acquire_read(my_vm);
 node = this_vm_getnode(VM_ADDR2PAGE((uintptr_t)addr));
 if unlikely(!node) {
fail_unlock:
  vm_release_read(my_vm);
fail:
  return NULL;
 }
 /* Got the node. */
 TRY {
  vm_raddr_t raddr;
  /* Convert the given address to become region-relative. */
  raddr = (node->vn_start * PAGESIZE) + ((uintptr_t)addr - VM_NODE_MINADDR(node));
  new_futex = NULL;
  iter = &node->vn_region->vr_futex;
  atomic_rwptr_read(iter);
  while ((result = (struct futex *)ATOMIC_RWPTR_GET(*iter)) != NULL) {
   if (result->f_addr >= raddr) { /* Found it, or doesn't exist. */
    if (result->f_addr != raddr)
        goto fail_unlock;
    if unlikely(!ATOMIC_INCIFNONZERO(result->f_refcnt))
       goto fail_unlock;
    atomic_rwptr_endread(iter);
    if unlikely(new_futex)
       futex_free(new_futex);
    goto done;
   }
   atomic_rwptr_read(&result->f_next);
   atomic_rwptr_endread(iter);
   iter = &result->f_next;
  }
  atomic_rwptr_endread(iter);
  goto fail_unlock;
done:;
 } FINALLY {
  vm_release_read(my_vm);
 }
 return result;
}


DEFINE_SYSCALL6(futex,
                USER UNCHECKED u32 *,uaddr,int,op,uintptr_t,val,
                USER UNCHECKED struct timespec64 *,utime,
                USER UNCHECKED u32 *,uaddr2,
                uintptr_t,val3) {
 syscall_slong_t result = 0;
 REF struct futex *EXCEPT_VAR ftx;
#ifdef __INTELLISENSE__
 u32 val2;
#else
#define val2  ((u32)(uintptr_t)utime)
#endif
 validate_readable(uaddr,sizeof(*uaddr));

 switch (op & FUTEX_CMD_MASK) {

 case FUTEX_WAIT_BITSET:
  task_channelmask((uintptr_t)val3);
 case FUTEX_WAIT:
  if (ATOMIC_READ(*uaddr) != (u32)val)
      return -EAGAIN;
  ftx = vm_futex(uaddr);
  /* Connect to the futex. */
  task_connect(&ftx->f_sig);
  TRY {
   /* Check the address again. */
   if (ATOMIC_READ(*uaddr) != (u32)val)
       result = -EAGAIN;
   else if (utime) {
    /* Must do the wait. */
    validate_readable(utime,sizeof(struct timespec64));
    if ((op & FUTEX_CMD_MASK) == FUTEX_WAIT ? !task_waitfor_tmrel(utime)
                                            : !task_waitfor_tmabs(utime))
         result = -ETIMEDOUT;
   } else {
    task_wait();
   }
  } FINALLY {
   task_disconnect(); /* In case the second access to `*uaddr' faults. */
   futex_decref(ftx);
  }
  break;

 case FUTEX_WAIT_BITSET_GHOST:
  task_channelmask((uintptr_t)val3);
 case FUTEX_WAIT_GHOST:
  if (ATOMIC_READ(*uaddr) != (u32)val)
      return -EAGAIN;
  ftx = vm_futex(uaddr);
  /* Connect to the futex (as a ghost). */
  task_connect_ghost(&ftx->f_sig);
  TRY {
   /* Check the address again. */
   if (ATOMIC_READ(*uaddr) != (u32)val)
       result = -EAGAIN;
   else if (utime) {
    /* Must do the wait. */
    validate_readable(utime,sizeof(struct timespec64));
    if ((op & FUTEX_CMD_MASK) == FUTEX_WAIT ? !task_waitfor_tmrel(utime)
                                            : !task_waitfor_tmabs(utime))
         result = -ETIMEDOUT;
   } else {
    task_wait();
   }
  } FINALLY {
   task_disconnect(); /* In case the second access to `*uaddr' faults. */
   futex_decref(ftx);
  }
  break;

 case FUTEX_WAIT_MASK:
  if ((ATOMIC_READ(*uaddr) & (u32)val) != (u32)(uintptr_t)uaddr2)
       return -EAGAIN;
  /* Connect to the futex. */
  ftx = vm_futex(uaddr);
  task_connect(&ftx->f_sig);
do_wait_mask:
  TRY {
   u32 old_value;
   /* Atomically set bits from val3, while also ensuring that the mask still applies. */
   do if (((old_value = ATOMIC_READ(*uaddr)) & (u32)val) != (u32)(uintptr_t)uaddr2)
      { result = -EAGAIN; goto done_mask; }
   while (!ATOMIC_CMPXCH(*uaddr,old_value,old_value|(u32)val3));
   /* Check the address again. */
   if ((old_value & (u32)val) == (u32)(uintptr_t)uaddr2)
       result = -EAGAIN;
   else {
    if (utime) {
     /* Must do the wait. */
     validate_readable(utime,sizeof(struct timespec64));
     if (!task_waitfor_tmabs(utime))
          result = -ETIMEDOUT;
    } else {
     task_wait();
    }
   }
done_mask:
   ;
  } FINALLY {
   task_disconnect(); /* In case the second access to `*uaddr' faults. */
   futex_decref(ftx);
  }
  break;

 case FUTEX_WAIT_MASK_GHOST:
  if ((ATOMIC_READ(*uaddr) & (u32)val) != (u32)(uintptr_t)uaddr2)
       return -EAGAIN;
  /* Connect to the futex. */
  ftx = vm_futex(uaddr);
  task_connect_ghost(&ftx->f_sig);
  goto do_wait_mask;

 case FUTEX_WAIT_CMPXCH:
  if (ATOMIC_CMPXCH(*uaddr,val,val3)) {
   /* Must still trigger the second futex, if it was defined. */
   if (!uaddr2)
        return -EAGAIN;
   /* Save the old futex value in the provided uaddr2 */
   ATOMIC_WRITE(*uaddr2,val);
   /* Broadcast a futex located at the given address. */
   ftx = vm_getfutex(uaddr2);
   if (ftx) {
    result = sig_broadcast(&ftx->f_sig);
    futex_decref(ftx);
   }
   return result;
  }
  /* Connect to the futex. */
  ftx = vm_futex(uaddr);
  task_connect(&ftx->f_sig);
do_wait_cmpxch:
  TRY {
   u32 real_old_value;
   /* Now that we're connected, try the CMPXCH again, this
    * time storing the old value in uaddr2 (if provided). */
   real_old_value = ATOMIC_CMPXCH_VAL(*uaddr,val,val3);
   if (uaddr2) {
    REF struct futex *ftx2;
    /* Save the old futex value in the provided uaddr2 */
    ATOMIC_WRITE(*uaddr2,real_old_value);
    /* Broadcast a futex located at the given address. */
    ftx2 = vm_getfutex(uaddr2);
    if (ftx2) {
     result = sig_broadcast(&ftx2->f_sig);
     futex_decref(ftx2);
    }
   }
   if (real_old_value == val) {
    if (!uaddr2)
         result = -EAGAIN; /* Don't wait. - The CMPXCH succeeded. */
   } else if (utime) {
    /* Must do the wait. */
    validate_readable(utime,sizeof(struct timespec64));
    if (!task_waitfor_tmabs(utime) && result == 0)
         result = -ETIMEDOUT;
   } else {
    task_wait();
   }
  } FINALLY {
   task_disconnect(); /* In case the second access to `*uaddr' faults. */
   futex_decref(ftx);
  }
  break;

 case FUTEX_WAIT_CMPXCH_GHOST:
  if (ATOMIC_CMPXCH(*uaddr,val,val3)) {
   /* Must still trigger the second futex, if it was defined. */
   if (!uaddr2)
        return -EAGAIN;
   /* Save the old futex value in the provided uaddr2 */
   ATOMIC_WRITE(*uaddr2,val);
   /* Broadcast a futex located at the given address. */
   ftx = vm_getfutex(uaddr2);
   if (ftx) {
    result = sig_broadcast(&ftx->f_sig);
    futex_decref(ftx);
   }
   return result;
  }
  /* Connect to the futex. */
  ftx = vm_futex(uaddr);
  task_connect_ghost(&ftx->f_sig);
  goto do_wait_cmpxch;

 case FUTEX_WAIT_CMPXCH2:
  if (ATOMIC_CMPXCH(*uaddr,val,val3))
      return -EAGAIN;
  /* Connect to the futex. */
  ftx = vm_futex(uaddr);
  task_connect(&ftx->f_sig);
do_wait_cmpxch2:
  TRY {
   u32 real_old_value;
   /* Now that we're connected, try the CMPXCH again, this
    * time storing the old value in uaddr2 (if provided). */
   real_old_value = ATOMIC_CMPXCH_VAL(*uaddr,val,val3);
   if (real_old_value == val)
    result = -EAGAIN;
   else if (utime) {
    REF struct futex *ftx2;
    /* Save the old futex value in the provided uaddr2 */
    ATOMIC_WRITE(*uaddr2,real_old_value);
    /* Broadcast a futex located at the given address. */
    ftx2 = vm_getfutex(uaddr2);
    if (ftx2) {
     result = sig_broadcast(&ftx2->f_sig);
     futex_decref(ftx2);
    }
    /* Must do the wait. */
    validate_readable(utime,sizeof(struct timespec64));
    if (!task_waitfor_tmabs(utime) && result == 0)
         result = -ETIMEDOUT;
   } else {
    task_wait();
   }
  } FINALLY {
   task_disconnect(); /* In case the second access to `*uaddr' faults. */
   futex_decref(ftx);
  }
  break;

 case FUTEX_WAIT_CMPXCH2_GHOST:
  if (ATOMIC_CMPXCH(*uaddr,val,val3))
      return -EAGAIN;
  /* Connect to the futex. */
  ftx = vm_futex(uaddr);
  task_connect_ghost(&ftx->f_sig);
  goto do_wait_cmpxch2;

 case FUTEX_WAKE_BITSET:
  if (val3 != FUTEX_BITSET_MATCH_ANY) {
   ftx = vm_getfutex(uaddr);
   if (ftx) {
    result = sig_send_channel(&ftx->f_sig,
                             (uintptr_t)val3,
                             (size_t)val);
    futex_decref(ftx);
   }
   break;
  }
  ATTR_FALLTHROUGH
 case FUTEX_WAKE:
  ftx = vm_getfutex(uaddr);
  if (ftx) {
   result = sig_send(&ftx->f_sig,(size_t)val);
   futex_decref(ftx);
  }
  break;

 {
  u32 caller_tid;
 case FUTEX_LOCK_PI:
  caller_tid = posix_gettid();
  if (ATOMIC_CMPXCH(*uaddr,0,caller_tid))
      return 0;
  ftx = vm_futex(uaddr);
  TRY {
   u32 temp;
lock_pi_connect:
   task_connect(&ftx->f_sig);
   do {
    for (;;) {
     temp = ATOMIC_READ(*uaddr);
     if (temp) break;
     if (ATOMIC_CMPXCH(*uaddr,0,caller_tid)) { task_disconnect(); goto done_lock_pi; }
     continue;
    }
   } while (!ATOMIC_CMPXCH(*uaddr,temp,temp|FUTEX_WAITERS));
   /* Wait for the futex to be signaled. */
   if (task_waitfor_tmabs(utime)) {
    /* Try to acquire the lock now. */
    if (ATOMIC_CMPXCH(*uaddr,0,caller_tid))
        goto done_lock_pi;
    goto lock_pi_connect;
   }
   result = -ETIMEDOUT;
done_lock_pi:;
  } FINALLY {
   futex_decref(ftx);
  }
 } break;

 {
  u32 caller_tid,value;
 case FUTEX_UNLOCK_PI:
  caller_tid = posix_gettid();
  for (;;) {
   value = ATOMIC_CMPXCH_VAL(*uaddr,caller_tid,0);
   if unlikely(value == caller_tid)
        return 0; /* Unlock */
   if ((value & ~FUTEX_WAITERS) != caller_tid)
        error_throw(E_ILLEGAL_OPERATION); /* You're not the owner. */
   if (ATOMIC_CMPXCH(*uaddr,value,0))
        break;
  }
  /* All right. Now wake one reader. */
  ftx = vm_getfutex(uaddr);
  if (ftx) {
   result = sig_send(&ftx->f_sig,1);
   futex_decref(ftx);
  }
 } break;

 {
  struct handle EXCEPT_VAR hresult;
 case FUTEX_FD:
  if (val != 0) {
   /* In linux, a val!=0 used to create a handle that would cause a
    * signal with that same number to be generated asynchronously
    * when the futex got signaled.
    *  -> KOS doesn't implement this.
    */
   error_throw(E_NOT_IMPLEMENTED);
  }
  /* Lookup the futex at the given address. */
  hresult.h_mode = HANDLE_MODE(HANDLE_TYPE_FFUTEX,0);
  hresult.h_object.o_futex = vm_futex(uaddr);
  TRY {
   /* Save the new futex handle in the handle manager. */
   result = handle_put(hresult);
  } FINALLY {
   futex_decref(hresult.h_object.o_futex);
  }
 } break;

 {
  struct handle EXCEPT_VAR hresult;
 case FUTEX_WAITFD:
  val3 = FUTEX_BITSET_MATCH_ANY;
 case FUTEX_WAITFD_BITSET:
  /* Validate the futex handle mode argument. */
  if (val2 & ~(O_ACCMODE|O_CLOEXEC|O_CLOFORK|O_NONBLOCK))
      error_throw(E_INVALID_ARGUMENT);
  /* Lookup the futex at the given address. */
  ftx = vm_futex(uaddr);
  hresult.h_mode = HANDLE_MODE(HANDLE_TYPE_FFUTEX_HANDLE,
                               IO_FROM_O(val2));
  TRY {
   /* Construct a new futex handle wrapper. */
   hresult.h_object.o_futex_handle = futex_handle_alloc(ftx,uaddr,(u32)val,val3);
  } FINALLY {
   futex_decref(ftx);
  }
  TRY {
   /* Save the new futex handle in the handle manager. */
   result = handle_put(hresult);
  } FINALLY {
   futex_handle_decref(hresult.h_object.o_futex_handle);
  }
 } break;

#if 0
 case FUTEX_REQUEUE:
  return futex_requeue(uaddr,flags,uaddr2,val,val2,NULL,0);

 case FUTEX_CMP_REQUEUE:
  return futex_requeue(uaddr,flags,uaddr2,val,val2,&val3,0);

 case FUTEX_TRYLOCK_PI:
  return futex_lock_pi(uaddr,flags,0,timeout,1);

 case FUTEX_WAIT_REQUEUE_PI:
  val3 = FUTEX_BITSET_MATCH_ANY;
  return futex_wait_requeue_pi(uaddr,flags,val,timeout,val3,uaddr2);

 case FUTEX_CMP_REQUEUE_PI:
  return futex_requeue(uaddr,flags,uaddr2,val,val2,&val3,1);
#endif

 default:
  error_throw(E_INVALID_ARGUMENT);
  break;
 }

#undef val2
 return result;
}



/* Construct a new futex handle. */
PUBLIC ATTR_MALLOC ATTR_RETNONNULL REF struct futex_handle *KCALL
futex_handle_alloc(struct futex *__restrict ftx,
                   USER CHECKED u32 *uaddr, u32 value, uintptr_t mask) {
 REF struct futex_handle *result;
 /* Construct a new futex handle. */
 result = (REF struct futex_handle *)kmalloc(sizeof(REF struct futex_handle),
                                             GFP_SHARED);
 /* Initialize the futex handle. */
 result->fh_refcnt = 1;
 futex_incref(ftx);
 result->fh_futex  = ftx;
 result->fh_uaddr  = uaddr;
 result->fh_value  = value;
 result->fh_mask   = mask;
 return result;
}

/* Destroy a previously allocated futex handle. */
PUBLIC void KCALL
futex_handle_destroy(struct futex_handle *__restrict self) {
 futex_decref(self->fh_futex);
 kfree(self);
}


/* futex_handle handle operators. */
INTERN unsigned int KCALL
handle_futex_handle_poll(struct futex_handle *__restrict self,
                         unsigned int mode) {
 u32 value;
 if (!(mode & POLLIN))
       return 0;
 /* Do an initial read on the futex word. */
 value = ATOMIC_READ(*self->fh_uaddr);
 if (value != self->fh_value)
     return POLLIN;
 /* Open all channels masked by the futex. */
 task_openchannel(self->fh_mask);
 /* Connect to the signal as a ghost. */
 task_connect_ghost(&self->fh_futex->f_sig);
 COMPILER_READ_BARRIER();

 /* With the connection established, repeat read of the
  * futex word as an operation interlocked with the signal. */
 value = ATOMIC_READ(*self->fh_uaddr);
 if (value != self->fh_value)
     return POLLIN;

 return 0;
}
INTERN size_t KCALL
handle_futex_handle_read(struct futex_handle *__restrict self,
                         USER CHECKED void *buf, size_t bufsize,
                         iomode_t flags) {
 u32 value;
 if (bufsize < 4)
     error_throwf(E_BUFFER_TOO_SMALL,bufsize,(size_t)4);
 /* Do an initial, non-interlocked check
  * and deal with the IO_NONBLOCK flag. */
 value = ATOMIC_READ(*self->fh_uaddr);
 if (value != self->fh_value) {
  *(USER CHECKED u32 *)buf = value;
  return 4;
 }
 if (flags & IO_NONBLOCK)
     return 0;
again:
 task_channelmask(self->fh_mask);
 task_connect(&self->fh_futex->f_sig);
 /* Repeat the check while in interlocked mode */
 value = ATOMIC_READ(*self->fh_uaddr);
 if (value != self->fh_value) {
  *(USER CHECKED u32 *)buf = value;
  task_disconnect();
  return 4;
 }
 task_wait();
 goto again;
}
INTERN size_t KCALL
handle_futex_handle_write(struct futex_handle *__restrict self,
                          USER CHECKED void const *buf, size_t bufsize,
                          iomode_t flags) {
 if (bufsize < 4)
     error_throwf(E_BUFFER_TOO_SMALL,bufsize,(size_t)4);
 sig_send(&self->fh_futex->f_sig,*(u32 *)buf);
 return 4;
}
INTERN size_t KCALL
handle_futex_handle_pwrite(struct futex_handle *__restrict self,
                           USER CHECKED void const *buf, size_t bufsize,
                           pos_t pos, iomode_t flags) {
 if (pos > UINT32_MAX)
     error_throw(E_INVALID_ARGUMENT);
 if (bufsize < 4)
     error_throwf(E_BUFFER_TOO_SMALL,bufsize,(size_t)4);
 /* Send out the signal. */
 if (pos == FUTEX_BITSET_MATCH_ANY) {
  sig_send(&self->fh_futex->f_sig,*(u32 *)buf);
 } else {
  sig_send_channel(&self->fh_futex->f_sig,(u32)pos,*(u32 *)buf);
 }
 return 4;
}




/* futex handle operators. */
INTERN unsigned int KCALL
handle_futex_poll(struct futex *__restrict self,
                  unsigned int mode) {
 /* It's small enough to implement, even if linux no longer does... */
 if (mode != 0)
     task_connect_ghost(&self->f_sig);
 return mode;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_VM_FUTEX_C */
