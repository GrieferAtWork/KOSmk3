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
#ifndef GUARD_KERNEL_SRC_FS_RINGBUFFER_C
#define GUARD_KERNEL_SRC_FS_RINGBUFFER_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/minmax.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <sched/signal.h>
#include <kernel/debug.h>
#include <kernel/heap.h>
#include <fs/ringbuffer.h>
#include <assert.h>
#include <string.h>
#include <except.h>
#include <bits/poll.h>

/* General-purpose RING-buffer. */

DECL_BEGIN

#define RING_ALLOC(num_bytes)     heap_alloc_untraced(&kernel_heaps[GFP_SHARED],num_bytes,GFP_SHARED)
#define RING_ALLAT(ptr,num_bytes) heap_allat_untraced(&kernel_heaps[GFP_SHARED],ptr,num_bytes,GFP_SHARED)
#define RING_FREE(ptr,num_bytes)  heap_free_untraced(&kernel_heaps[GFP_SHARED],ptr,num_bytes,GFP_SHARED)


/* Special pointer positions:
 *   - READ == WRITE                 --> FULL
 *   - READ == END && WRITE == BEGIN --> EMPTY
 * Illegal pointer positions:
 *   - WRITE == END                   (Must be fixed by setting WRITE to BEGIN)
 *   - READ  == END && WRITE != BEGIN (Must be fixed by setting READ to BEGIN)
 */


/* Finalize a given ring-buffer. */
PUBLIC ATTR_NOTHROW void KCALL
ringbuffer_fini(struct ringbuffer *__restrict self) {
 if (self->r_size)
     RING_FREE(self->r_base,self->r_size);
}
PUBLIC ATTR_NOTHROW bool KCALL
ringbuffer_close(struct ringbuffer *__restrict self) {
 size_t old_limit;
 /* Set the closed-flag. */
 old_limit = ATOMIC_FETCHOR(self->r_limt,RBUFFER_LIMT_FCLOSED);
 if (old_limit & RBUFFER_LIMT_FCLOSED) return false;
 /* Broadcast the state-change signal to wake everyone. */
 sig_broadcast(&self->r_stat);
 return true;
}


PUBLIC ATTR_NOTHROW WEAK size_t KCALL
ringbuffer_maxread(struct ringbuffer *__restrict self) {
 byte_t *rptr,*wptr; size_t result;
 atomic_rwlock_read(&self->r_lock);
 rptr = ATOMIC_READ(self->r_rptr);
 wptr = self->r_wptr;
 if (rptr < wptr)
  result = (size_t)(wptr-rptr);
 else {
  result = self->r_size - (size_t)(rptr-wptr);
 }
 atomic_rwlock_endread(&self->r_lock);
 return result;
}
PUBLIC ATTR_NOTHROW WEAK size_t KCALL
ringbuffer_maxwrite(struct ringbuffer *__restrict self) {
 byte_t *rptr,*wptr; size_t result;
 atomic_rwlock_read(&self->r_lock);
 rptr = ATOMIC_READ(self->r_rptr);
 wptr = self->r_wptr;
 if (wptr < rptr)
  result = (size_t)(rptr-wptr);
 else {
  result = self->r_size - (size_t)(wptr-rptr);
 }
 atomic_rwlock_endread(&self->r_lock);
 return result;
}



/* Try to un-read up to `num_bytes' bytes and return how much could actually be unread. */
PUBLIC ATTR_NOTHROW size_t KCALL
ringbuffer_unread(struct ringbuffer *__restrict self, size_t num_bytes) {
 size_t result = 0;
 uintptr_t boardcast_mask = 0;
 atomic_rwlock_read(&self->r_lock);
 if (self->r_wptr != self->r_base) {
  byte_t *old_pointer,*new_pointer;
  do {
   old_pointer = ATOMIC_READ(self->r_rptr);
   if (old_pointer  == self->r_base+self->r_size &&
       self->r_wptr == self->r_base)
       break; /* Buffer is empty (and we don't know where last-read data was stored). */
   if (__builtin_sub_overflow((uintptr_t)old_pointer,num_bytes,
                              (uintptr_t *)&new_pointer) ||
       new_pointer < self->r_base)
       new_pointer = self->r_base;
   result = (size_t)(old_pointer-new_pointer);
   assert(result <= num_bytes);
   if (new_pointer == self->r_base && result != num_bytes) {
    /* Wrap around... */
    byte_t *bufend = self->r_base+self->r_size;
    size_t  max_skip = num_bytes-result;
    if (__builtin_sub_overflow((uintptr_t)bufend,max_skip,
                               (uintptr_t *)&new_pointer) ||
        new_pointer < self->r_wptr)
        new_pointer = self->r_wptr;
    result += (size_t)(bufend-new_pointer);
   }
  } while (!ATOMIC_CMPXCH_WEAK(self->r_rptr,old_pointer,new_pointer));
  /* If the buffer was full before, broadcast that new data can now be read. */
  if (result) {
   if (old_pointer == self->r_base+self->r_size)
    boardcast_mask = (uintptr_t)-1;
   else if (new_pointer == self->r_wptr) {
    boardcast_mask = RBUF_STATE_CHANNEL_FULL;
   }
  }
 }
 atomic_rwlock_endread(&self->r_lock);
 if (boardcast_mask != 0) {
  if (boardcast_mask == (uintptr_t)-1)
       sig_broadcast(&self->r_stat);
  else sig_broadcast_channel(&self->r_stat,boardcast_mask);
 }

 return result;
}

/* Try to un-write up to `num_bytes' bytes and return how much could actually be unwritten. */
PUBLIC ATTR_NOTHROW size_t KCALL
ringbuffer_unwrite(struct ringbuffer *__restrict self, size_t num_bytes) {
 bool was_full,is_empty; size_t result; byte_t *new_pointer;
 atomic_rwlock_write(&self->r_lock);
 if (self->r_rptr <= self->r_wptr) {
  /* Only memory below the write-pointer has been written. */
  result = self->r_wptr-self->r_rptr;
  if (result > num_bytes)
      result = num_bytes;
  new_pointer = self->r_wptr-result;
 } else {
  if (__builtin_sub_overflow((uintptr_t)self->r_wptr,num_bytes,
                             (uintptr_t *)&new_pointer) ||
      new_pointer < self->r_base)
      new_pointer = self->r_base;
  result = new_pointer-self->r_base;
  assert(result <= num_bytes);
  if (new_pointer == self->r_base && result != num_bytes) {
   /* Wrap around and unwrite more data. */
   byte_t *bufend   = self->r_base+self->r_size;
   size_t  max_skip = num_bytes-result;
   if (__builtin_sub_overflow((uintptr_t)bufend,max_skip,
                              (uintptr_t *)&new_pointer) ||
       new_pointer < self->r_rptr)
       new_pointer = self->r_rptr;
   assert(new_pointer);
   result += (size_t)(bufend-new_pointer);
  }
 }
 was_full = self->r_rptr == self->r_wptr;
 is_empty = self->r_rptr == new_pointer;
 if (is_empty) {
  /* Special case: the buffer is now empty. */
  self->r_rptr = self->r_base+self->r_size;
  new_pointer  = self->r_base;
 }
 /* Set the new write-pointer position. */
 self->r_wptr = new_pointer;
 atomic_rwlock_endwrite(&self->r_lock);
 /* Wake the next writer if the buffer was full and we managed to remove some. */
 if (result) {
  if (was_full)
      sig_send_channel(&self->r_stat,RBUF_STATE_CHANNEL_NOTFULL,1);
  if (is_empty)
      sig_broadcast_channel(&self->r_stat,RBUF_STATE_CHANNEL_EMPTY);
 }
 return result;
}

/* Discard up to `num_bytes' bytes of previously written data as though it was read.
 * Return the actual amount of bytes discarded. */
PUBLIC ATTR_NOTHROW size_t KCALL
ringbuffer_discard(struct ringbuffer *__restrict self, size_t num_bytes) {
 byte_t *bufend,*old_pointer,*new_pointer;
 size_t result; bool was_full,is_empty;
 bool has_write_lock = false;
 atomic_rwlock_read(&self->r_lock);
again:
 bufend = self->r_base+self->r_size;
 do {
  old_pointer = ATOMIC_READ(self->r_rptr);
  new_pointer = old_pointer;
  result      = 0;
  was_full    = old_pointer == self->r_wptr;
  /* If the read-pointer is located at the end, but the buffer
   * isn't empty, wrap around to read data from low memory. */
  if (old_pointer == bufend &&
      self->r_wptr != self->r_base)
      new_pointer = self->r_base;
  if (new_pointer < bufend &&
      new_pointer >= self->r_wptr) {
   /* Read data before the buffer end. */
   result = MIN(bufend-new_pointer,num_bytes);
   new_pointer += result;
   if (new_pointer == bufend && self->r_wptr != self->r_base)
       new_pointer = self->r_base;
  }
  if (new_pointer < self->r_wptr) {
   size_t max_read;
   /* Read data below the write-pointer. */
   max_read = num_bytes-result;
   max_read = MIN(self->r_wptr-new_pointer,max_read);
   result      += max_read;
   new_pointer += max_read;
  }
  is_empty = new_pointer == self->r_wptr;
  if (is_empty) {
   /* Buffer became empty. */
   if (!has_write_lock) {
    has_write_lock = true;
    if (!atomic_rwlock_upgrade(&self->r_lock))
         goto again;
   }
   /* Change the buffer to an empty state. */
   new_pointer = self->r_base+self->r_size;
   if (!ATOMIC_CMPXCH(self->r_rptr,old_pointer,new_pointer))
        goto again;
   self->r_wptr = self->r_base;
   COMPILER_WRITE_BARRIER();
   break;
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->r_rptr,old_pointer,new_pointer));
 if (has_write_lock)
      atomic_rwlock_endwrite(&self->r_lock);
 else atomic_rwlock_endread(&self->r_lock);
 /* If the buffer was full, wake a writer after we read some data. */
 if (result) {
  if (was_full)
      sig_send_channel(&self->r_stat,RBUF_STATE_CHANNEL_NOTFULL,1);
  if (is_empty)
      sig_broadcast_channel(&self->r_stat,RBUF_STATE_CHANNEL_EMPTY);
 }
 return result;
}



/* Read data from the buffer and return the amount of bytes read.
 * `ringbuffer_read()' will attempt to read at least 1 byte of data, which
 * if there isn't one at the time the function is called, it will block
 * until something can be read, even when ZERO(0) is passed for `num_bytes'.
 * `ringbuffer_read_atomic()' does the opposite and returns
 * immediately without blocking, even if no data could be read.
 * @throw: * :          [ringbuffer_read] This error was thrown by an RPC function.
 * @throw: E_INTERRUPT: [ringbuffer_read] The calling thread was interrupted.
 * @throw: E_SEGFAULT:  A faulty buffer was given. */
PUBLIC size_t KCALL
ringbuffer_read_atomic(struct ringbuffer *__restrict EXCEPT_VAR self,
                       USER CHECKED void *buf, size_t num_bytes) {
 byte_t *bufend,*old_pointer,*new_pointer,*dst;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 bool COMPILER_IGNORE_UNINITIALIZED(was_full);
 bool COMPILER_IGNORE_UNINITIALIZED(is_empty);
 bool has_write_lock = false;
 atomic_rwlock_read(&self->r_lock);
 TRY { /* Required for potentially faulty user-buffer pointers. */
again:
  bufend = self->r_base+self->r_size;
  do {
   dst         = (byte_t *)buf;
   old_pointer = ATOMIC_READ(self->r_rptr);
   new_pointer = old_pointer;
   result      = 0;
   was_full    = old_pointer == self->r_wptr;
   /* If the read-pointer is located at the end, but the buffer
    * isn't empty, wrap around to read data from low memory. */
   if (old_pointer == bufend &&
       self->r_wptr != self->r_base)
       new_pointer = self->r_base;
   if (new_pointer < bufend &&
       new_pointer >= self->r_wptr) {
    /* Read data before the buffer end. */
    result = MIN(bufend-new_pointer,num_bytes);
    memcpy(dst,new_pointer,result);
    dst         += result;
    new_pointer += result;
    if (new_pointer == bufend && self->r_wptr != self->r_base)
        new_pointer = self->r_base;
   }
   if (new_pointer < self->r_wptr) {
    size_t max_read;
    /* Read data below the write-pointer. */
    assert((size_t)(dst-(byte_t *)buf) == result);
    assert(num_bytes >= result);
    max_read = num_bytes-result;
    max_read = MIN(self->r_wptr-new_pointer,max_read);
    memcpy(dst,new_pointer,max_read);
    dst         += result;
    result      += max_read;
    new_pointer += max_read;
   }
   is_empty = new_pointer == self->r_wptr;
   if (is_empty) {
    /* Buffer became empty. */
    if (!has_write_lock) {
     has_write_lock = true;
     if (!atomic_rwlock_upgrade(&self->r_lock))
          goto again;
    }
    /* Change the buffer to an empty state. */
    new_pointer = self->r_base+self->r_size;
    if (!ATOMIC_CMPXCH(self->r_rptr,old_pointer,new_pointer))
         goto again;
    COMPILER_WRITE_BARRIER();
    self->r_wptr = self->r_base;
    COMPILER_WRITE_BARRIER();
    break;
   }
  } while (!ATOMIC_CMPXCH(self->r_rptr,old_pointer,new_pointer));
  /* TODO: RBUFFER_FREE_THRESHOLD */
#if 0
  if (!result) {
   debug_printf("Ring buffer is empty\n");
   debug_printf("    self->r_rptr = %p\n",self->r_rptr);
   debug_printf("    self->r_wptr = %p\n",self->r_wptr);
   debug_printf("    self->r_base = %p (%p...%p)\n",
                self->r_base,self->r_base,self->r_base+self->r_size-1);
   debug_printf("    self->r_size = %p\n",self->r_size);
   debug_printf("    self->r_limt = %p\n",self->r_limt & RBUFFER_LIMT_FMASK);
  }
#endif
 } FINALLY {
  if (has_write_lock)
       atomic_rwlock_endwrite(&self->r_lock);
  else atomic_rwlock_endread(&self->r_lock);
 }
 /* If the buffer was full, wake a writer after we read some data. */
 if (result) {
  if (was_full)
      sig_send_channel(&self->r_stat,RBUF_STATE_CHANNEL_NOTFULL,1);
  if (is_empty)
      sig_broadcast_channel(&self->r_stat,RBUF_STATE_CHANNEL_EMPTY);
 }
 return result;
}

PUBLIC size_t KCALL
ringbuffer_read(struct ringbuffer *__restrict self,
                USER CHECKED void *buf, size_t num_bytes, iomode_t flags) {
 size_t result;
 uintptr_t EXCEPT_VAR old_mask;
again:
 result = ringbuffer_read_atomic(self,buf,num_bytes);
 if (likely(result || !num_bytes) ||
    (self->r_limt&RBUFFER_LIMT_FCLOSED) ||
    (flags & IO_NONBLOCK))
     goto done;
 /* Connect to the signal of the buffer to wait for data to become available. */
 old_mask = task_channelmask(0);
 TRY {
  task_connect(&self->r_stat);
  TRY {
   /* Try to read data again now that we're connected. */
   result = ringbuffer_read_atomic(self,buf,num_bytes);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   task_disconnect();
   error_rethrow();
  }
  COMPILER_READ_BARRIER();
  if (result ||
     (self->r_limt&RBUFFER_LIMT_FCLOSED)) {
   task_disconnect();
   goto done;
  }
  /* Wait for data to become available. */
  task_wait();
 } FINALLY {
  task_channelmask(old_mask);
 }
 /* Jump back and try to read that data. */
 goto again;
done:
 return result;
}

/* Write data to the buffer and return the amount of bytes written.
 * `ringbuffer_write()' will attempt to write at least 1 byte of data and will
 * block until space becomes available if the buffer was full when the
 * function was called.
 * `ringbuffer_write_atomic()' does the opposite and returns
 * immediately without blocking, even if no data could be written.
 * @throw: * :          [ringbuffer_write] This error was thrown by an RPC function.
 * @throw: E_INTERRUPT: [ringbuffer_write] The calling thread was interrupted.
 * @throw: E_SEGFAULT:  A faulty buffer was given. */
PUBLIC size_t KCALL
ringbuffer_write_atomic(struct ringbuffer *__restrict EXCEPT_VAR self,
                        USER CHECKED void const *buf, size_t num_bytes) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 bool COMPILER_IGNORE_UNINITIALIZED(was_empty);
 bool COMPILER_IGNORE_UNINITIALIZED(is_full);
 byte_t *bufend;
 if unlikely(!num_bytes ||
            (self->r_limt&RBUFFER_LIMT_FCLOSED))
    return 0;
again_lock:
 atomic_rwlock_write(&self->r_lock);
again:
 is_full = false;
 TRY {
  bufend = self->r_base+self->r_size;
  assert(self->r_wptr != bufend || !self->r_size);
  if (self->r_wptr < self->r_rptr) {
   /* Write to the remaining buffer memory before the read-pointer. */
   was_empty = (self->r_wptr == self->r_base &&
                self->r_rptr == bufend);
   result = MIN((size_t)(self->r_rptr-self->r_wptr),num_bytes);
   /* Write data to the buffer. */
   assertf(self->r_wptr != NULL,
           "self->r_rptr = %p\n"
           "self->r_base = %p\n",
           self->r_rptr,
           self->r_base);
   memcpy(self->r_wptr,buf,result);
   self->r_wptr += result;
   is_full = self->r_wptr == self->r_rptr;
   /* Deal with the special case of just having filled up the buffer completely. */
   if unlikely(self->r_wptr == bufend) {
    assert(self->r_rptr == bufend);
    assert(is_full);
    self->r_rptr = self->r_base;
    self->r_wptr = self->r_base;
   }
  } else if (self->r_wptr == self->r_rptr) {
   goto increase_buffer;
  } else {
   /* Write data before the end of the buffer. */
   was_empty = false;
   result    = MIN((size_t)(bufend-self->r_wptr),num_bytes);
   memcpy(self->r_wptr,buf,result);
   self->r_wptr += result;
   if (self->r_wptr == bufend) {
    self->r_wptr = self->r_base;
    assert(self->r_rptr >= self->r_base);
    if (self->r_rptr != self->r_base) {
     size_t max_write;
     /* Write additional data before the read-pointer. */
     max_write = (size_t)(self->r_rptr-self->r_wptr);
     if (max_write > num_bytes-result)
         max_write = num_bytes-result;
     /* Copy additional data. */
     memcpy(self->r_wptr,(void *)((uintptr_t)buf+result),max_write);
     self->r_wptr += max_write;
     result       += max_write;
     is_full = self->r_wptr == self->r_rptr;
     assert(self->r_wptr != bufend);
    }
   }
  }
 } FINALLY {
  atomic_rwlock_endwrite(&self->r_lock);
 }
 /* Wake readers now that the buffer is no longer empty. */
 if (result) {
  if (was_empty)
   sig_broadcast(&self->r_stat);
  else if (is_full) {
   sig_broadcast_channel(&self->r_stat,RBUF_STATE_CHANNEL_FULL);
  }
 }
 return result;
 {
  struct heapptr EXCEPT_VAR new_buffer;
  struct heapptr old_buffer;
  size_t old_size;
  size_t old_size_hi,old_size_lo;
increase_buffer:
  /* The buffer is full (allocate more memory). */
  if (self->r_limt & RBUFFER_LIMT_FSTATIC) {
   atomic_rwlock_endwrite(&self->r_lock);
   return 0;
  }
  old_buffer.hp_ptr = self->r_base;
  old_buffer.hp_siz = self->r_size;
  new_buffer.hp_siz = old_buffer.hp_siz*2;
  if (!new_buffer.hp_siz)
       new_buffer.hp_siz = RBUFFER_INIT_ALLOC;
  while (new_buffer.hp_siz < old_buffer.hp_siz+num_bytes)
       new_buffer.hp_siz *= 2;
  if (new_buffer.hp_siz > (self->r_limt & RBUFFER_LIMT_FMASK))
      new_buffer.hp_siz = (self->r_limt & RBUFFER_LIMT_FMASK);
  if (new_buffer.hp_siz <= old_buffer.hp_siz) {
   /* The buffer is full and has reached its limit. */
#if 0
   debug_printf("Ring buffer is full\n");
   debug_printf("    self->r_rptr = %p\n",self->r_rptr);
   debug_printf("    self->r_wptr = %p\n",self->r_wptr);
   debug_printf("    self->r_base = %p (%p...%p)\n",
                self->r_base,self->r_base,self->r_base+self->r_size-1);
   debug_printf("    self->r_size = %p\n",self->r_size);
   debug_printf("    self->r_limt = %p\n",self->r_limt & RBUFFER_LIMT_FMASK);
   debug_printf("%$[hex]\n",self->r_size,self->r_base);
#endif
   atomic_rwlock_endwrite(&self->r_lock);
   return 0;
  }
  atomic_rwlock_endwrite(&self->r_lock);
  if (old_buffer.hp_siz != 0 &&
      old_buffer.hp_ptr == self->r_rptr) {
   size_t additional_size;
   additional_size = RING_ALLAT((void *)((uintptr_t)old_buffer.hp_ptr+old_buffer.hp_siz),
                                                    new_buffer.hp_siz-old_buffer.hp_siz);
   if (additional_size) {
    /* Managed to extend the buffer above. */
    atomic_rwlock_write(&self->r_lock);
    if unlikely(self->r_base != old_buffer.hp_ptr ||
                self->r_size != old_buffer.hp_siz) {
     /* The buffer changed in the mean time. */
     atomic_rwlock_endwrite(&self->r_lock);
     RING_FREE((void *)((uintptr_t)old_buffer.hp_ptr+old_buffer.hp_siz),
                                   additional_size);
     goto again_lock;
    }
    /* Update buffer pointers and try again. */
    self->r_wptr  = self->r_base+self->r_size;
    self->r_size += additional_size;
    assert(self->r_rptr != self->r_wptr);
    goto again;
   }
  }
  /* Allocate a new buffer. */
  new_buffer = RING_ALLOC(new_buffer.hp_siz);
  atomic_rwlock_write(&self->r_lock);
  if unlikely(new_buffer.hp_siz <= self->r_size ||
              self->r_wptr != self->r_rptr) {
   atomic_rwlock_endwrite(&self->r_lock);
   RING_FREE(new_buffer.hp_ptr,new_buffer.hp_siz);
   goto again_lock;
  }
  /* Install the new buffer. */
  bufend = (self->r_base+self->r_size);
  /* Copy unread data into the new buffer. */
  old_size_hi = (size_t)(bufend-self->r_rptr);
  old_size_lo = (size_t)(self->r_wptr-self->r_base);
  old_size    = old_size_hi+old_size_lo;
  TRY {
   assert(old_size < new_buffer.hp_siz);
   memcpy(new_buffer.hp_ptr,self->r_rptr,old_size_hi);
   memcpy((byte_t *)new_buffer.hp_ptr+old_size_hi,
           self->r_base,old_size_lo);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   atomic_rwlock_endwrite(&self->r_lock);
   RING_FREE(new_buffer.hp_ptr,new_buffer.hp_siz);
   error_rethrow();
  }
  old_buffer.hp_ptr = self->r_base;
  old_buffer.hp_siz = self->r_size;
  self->r_base = (byte_t *)new_buffer.hp_ptr;
  self->r_size = new_buffer.hp_siz;
  self->r_rptr = (byte_t *)new_buffer.hp_ptr;
  self->r_wptr = (byte_t *)new_buffer.hp_ptr + old_size;
  if (!old_size) self->r_rptr += new_buffer.hp_siz;
  assert(self->r_rptr >= self->r_base);
  assert(self->r_rptr <= self->r_base+self->r_size);
  assert(self->r_wptr >= self->r_base);
  assert(self->r_wptr <  self->r_base+self->r_size);
  assert(self->r_rptr != self->r_wptr);
  if (old_buffer.hp_siz) {
   atomic_rwlock_endwrite(&self->r_lock);
   /* Free the old buffer. */
   RING_FREE(old_buffer.hp_ptr,old_buffer.hp_siz);
   goto again_lock;
  }
  /* Write to the memory brought fourth by the new buffer. */
  goto again;
 }
}

PUBLIC size_t KCALL
ringbuffer_write(struct ringbuffer *__restrict self,
                 USER CHECKED void const *buf,
                 size_t num_bytes, iomode_t flags) {
 size_t result;
 uintptr_t EXCEPT_VAR old_mask;
again:
 result = ringbuffer_write_atomic(self,buf,num_bytes);
 if (likely(result || !num_bytes) ||
    (self->r_limt&RBUFFER_LIMT_FCLOSED) ||
    (flags & IO_NONBLOCK))
     goto done;
 /* Connect to the signal of the buffer to wait for space to become available. */
 old_mask = task_channelmask(RBUF_STATE_CHANNEL_NOTFULL);
 TRY {
  task_connect(&self->r_stat);
  TRY {
   /* Try to write data again now that we're connected. */
   result = ringbuffer_write_atomic(self,buf,num_bytes);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   task_disconnect();
   error_rethrow();
  }
  COMPILER_READ_BARRIER();
  if (result ||
     (self->r_limt&RBUFFER_LIMT_FCLOSED)) {
   task_disconnect();
   goto done;
  }
  /* Wait for space to become available. */
  task_wait();
 } FINALLY {
  task_channelmask(old_mask);
 }
 /* Jump back and try to write more data. */
 goto again;
done:
 return result;
}

PUBLIC unsigned int KCALL
ringbuffer_poll(struct ringbuffer *__restrict self, unsigned int mode) {
 unsigned int result = 0;
 byte_t *rptr,*wptr,*base,*bufend;
 if (mode & POLLOUT) {
  /* Open the channel for non-full buffers
   * if we're polling for that condition. */
  task_openchannel(RBUF_STATE_CHANNEL_NOTFULL);
  COMPILER_BARRIER();
 }
 /* Connect to the state-changed signal.
  * NOTE: Use ghost connections to prevent the deadlock
  *       scenario described by `task_connect_ghost()' */
 task_connect_ghost(&self->r_stat);
 atomic_rwlock_read(&self->r_lock);
 rptr   = ATOMIC_READ(self->r_rptr);
 wptr   = self->r_wptr;
 base   = self->r_base;
 bufend = base + self->r_size;
 atomic_rwlock_endread(&self->r_lock);
 if (self->r_limt & RBUFFER_LIMT_FCLOSED)
     result |= POLLHUP; /* Buffer closed. -> hang-up */
 if (mode & POLLIN) {
  /* if (!IS_EMPTY) ... */
  if (rptr != bufend || wptr != base)
      result |= POLLIN;
 }
 if (mode & POLLOUT) {
  /* if (!IS_FULL) ... */
  if (rptr != wptr)
      result |= POLLOUT;
 }
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_RINGBUFFER_C */
