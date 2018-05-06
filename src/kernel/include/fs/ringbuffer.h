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
#ifndef GUARD_KERNEL_INCLUDE_FS_RINGBUFFER_H
#define GUARD_KERNEL_INCLUDE_FS_RINGBUFFER_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <sched/signal.h>
#include <fs/iomode.h>
#ifndef __INTELLISENSE__
#include <except.h>
#include <assert.h>
#include <sched/task.h>
#include <hybrid/atomic.h>
#endif

/* General-purpose RING-buffer. */

DECL_BEGIN

#define RBUFFER_INIT_ALLOC     512  /* The size of the initial data block. */
#define RBUFFER_FREE_THRESHOLD 1024 /* Free unused buffer memory when there is at lest this much */

struct iovec;
struct ringbuffer {
    atomic_rwlock_t     r_lock; /* Lock for this ring buffer.
                                 * WARNING: This lock may be held during pagefaults! */
    ATOMIC_DATA byte_t *r_rptr; /* [in(r_base)] Read pointer of the ring-buffer. */
    byte_t             *r_wptr; /* [lock(r_lock)][in(r_base)] Write pointer of the ring-buffer. */
    byte_t             *r_base; /* [lock(r_lock)][0..r_size][owned] Base-pointer to the buffer. */
    size_t              r_size; /* [lock(r_lock)] Allocated buffer size.
                                 *  NOTE: `r_base' is allosed using `heap_alloc_untraced()' and
                                 *        this is the argument returned by that function.
                                 *        In other words, this field is allowed to slightly
                                 *        outgrow `r_limt'. */
#define RBUFFER_LIMT_FMASK   (~3ul) /* Mask for the buffer limit. */
#define RBUFFER_LIMT_FSTATIC   0x1  /* [const] FLAG: Don't allocate a dynamic buffer. - `r_base' is statically allocated and fixed-length. */
#define RBUFFER_LIMT_FCLOSED   0x2  /* [lock(WEAK(WRITE_ONCE))] The buffer was closed. */
    size_t              r_limt; /* [lock(r_lock)] The max size that the buffer may grow to. */
    struct sig          r_stat; /* Signal broadcast / send to notify the following state changes:
                                 *   - The buffer was closed -> BROADCAST(*)
                                 *   - Data was written to an empty buffer -> BROADCAST(*)
                                 *   - Data was read from a full buffer -> SEND(RBUF_STATE_CHANNEL_NOTFULL,1)
                                 *   - The buffer is empty -> BROADCAST(RBUF_STATE_CHANNEL_EMPTY)
                                 *   - The buffer is full -> BROADCAST(RBUF_STATE_CHANNEL_FULL)
                                 */
};

#define RBUF_STATE_CHANNEL_NOTFULL 0x1 /* Channel mask send to a single thread via `r_stat'
                                        * after the ring buffer was full (no further data could
                                        * be written). */
#define RBUF_STATE_CHANNEL_EMPTY   0x2 /* Channel mask broadcast when the buffer becomes empty. */
#define RBUF_STATE_CHANNEL_FULL    0x4 /* Channel mask broadcast when the buffer becomes full. */

#define RINGBUFFER_INIT(limt) \
    { ATOMIC_RWLOCK_INIT, NULL, NULL, NULL, 0, \
     (limt) & RBUFFER_LIMT_FMASK, SIG_INIT }
#define ringbuffer_init(self,limt) \
    (atomic_rwlock_init(&(self)->r_lock), \
    (self)->r_rptr = (self)->r_wptr = (self)->r_base = NULL, \
    (self)->r_size = 0,(self)->r_limt = (limt) & RBUFFER_LIMT_FMASK, \
     sig_init(&(self)->r_stat))
#define ringbuffer_cinit(self,limt) \
    (atomic_rwlock_cinit(&(self)->r_lock), \
     assert((self)->r_rptr == NULL),assert((self)->r_wptr == NULL), \
     assert((self)->r_base == NULL),assert((self)->r_size == 0), \
    (self)->r_limt = (limt) & RBUFFER_LIMT_FMASK,sig_cinit(&(self)->r_stat))


/* Return the max number of characters that can be
 * read/written without blocking or allocating new memory.
 * WARNING: By the time this function returns, the
 *          returned value may no longer be up-to-date. */
FUNDEF ATTR_NOTHROW WEAK size_t KCALL ringbuffer_maxread(struct ringbuffer *__restrict self);
FUNDEF ATTR_NOTHROW WEAK size_t KCALL ringbuffer_maxwrite(struct ringbuffer *__restrict self);

/* Finalize a given ring-buffer. */
FUNDEF ATTR_NOTHROW void KCALL ringbuffer_fini(struct ringbuffer *__restrict self);

/* Close the buffer and wake all blocking readers/writers. */
FUNDEF ATTR_NOTHROW bool KCALL ringbuffer_close(struct ringbuffer *__restrict self);


/* Try to un-read up to `num_bytes' bytes and return how much could actually be unread. */
FUNDEF ATTR_NOTHROW size_t KCALL ringbuffer_unread(struct ringbuffer *__restrict self, size_t num_bytes);

/* Try to un-write up to `num_bytes' bytes and return how much could actually be unwritten. */
FUNDEF ATTR_NOTHROW size_t KCALL ringbuffer_unwrite(struct ringbuffer *__restrict self, size_t num_bytes);

/* Discard up to `num_bytes' bytes of previously written data as though it was read.
 * Return the actual amount of bytes discarded. */
FUNDEF ATTR_NOTHROW size_t KCALL ringbuffer_discard(struct ringbuffer *__restrict self, size_t num_bytes);

/* Read data from the buffer and return the amount of bytes read.
 * `ringbuffer_read()' will attempt to read at least 1 byte of data, which
 * if there isn't one at the time the function is called, it will block
 * until something can be read, even when ZERO(0) is passed for `num_bytes'.
 * `IO_NONBLOCK' / `ringbuffer_read_atomic()' do the opposite and
 *  return immediately without blocking, even if no data could be read.
 * NOTE: When the `RBUFFER_LIMT_FCLOSED' flag has been set, `ringbuffer_read()' will never block.
 * @throw: * :          [ringbuffer_read] This error was thrown by an RPC function.
 * @throw: E_INTERRUPT: [ringbuffer_read] The calling thread was interrupted.
 * @throw: E_SEGFAULT:  A faulty buffer was given. */
FUNDEF size_t KCALL ringbuffer_read(struct ringbuffer *__restrict self, USER CHECKED void *buf, size_t num_bytes, iomode_t mode);
FUNDEF size_t KCALL ringbuffer_read_atomic(struct ringbuffer *__restrict self, USER CHECKED void *buf, size_t num_bytes);
FUNDEF size_t KCALL ringbuffer_readv(struct ringbuffer *__restrict self, USER CHECKED struct iovec const *iov, size_t iov_offset, size_t num_bytes, iomode_t mode);
FUNDEF size_t KCALL ringbuffer_readv_atomic(struct ringbuffer *__restrict self, USER CHECKED struct iovec const *iov, size_t iov_offset, size_t num_bytes, iomode_t mode);

/* Read data and fill the given buffer entirely while keeping
 * on waiting for data that only arrives in chunks at a time.
 * If the ring buffer is closed while is is happened, only
 * return what has been read thus far.
 * WARNING: If the calling thread is interrupted, any data already read will be lost! */
FUNDEF size_t KCALL ringbuffer_readall(struct ringbuffer *__restrict self, USER CHECKED void *buf, size_t num_bytes, iomode_t mode);

/* Same as `ringbuffer_read()', but don't discard data as it is read. */
FUNDEF size_t KCALL ringbuffer_peek(struct ringbuffer *__restrict self, USER CHECKED void *buf, size_t num_bytes, iomode_t mode);
FUNDEF size_t KCALL ringbuffer_peek_atomic(struct ringbuffer *__restrict self, USER CHECKED void *buf, size_t num_bytes);
FUNDEF size_t KCALL ringbuffer_peekv(struct ringbuffer *__restrict self, USER CHECKED struct iovec const *iov, size_t iov_offset, size_t num_bytes, iomode_t mode);
FUNDEF size_t KCALL ringbuffer_peekv_atomic(struct ringbuffer *__restrict self, USER CHECKED struct iovec const *iov, size_t iov_offset, size_t num_bytes, iomode_t mode);
/* Similar to `ringbuffer_peek()', but peek on trying until the ensure user-buffer
 * has been read into, or until the amount read equals the max amount of data that
 * can be stored in the buffer at once, or until no data was peek, as indicative
 * of the IO_NONBLOCK flag, or the fact that the ring buffer was closed. */
FUNDEF size_t KCALL ringbuffer_peekall(struct ringbuffer *__restrict self, USER CHECKED void *buf, size_t num_bytes, iomode_t mode);

/* Write data to the buffer and return the amount of bytes written.
 * `ringbuffer_write()' will attempt to write at least 1 byte of data and will
 * block until space becomes available if the buffer was full when the
 * function was called.
 * `IO_NONBLOCK' / `ringbuffer_write_atomic()' do the opposite and
 *  return immediately without blocking, even if no data could be written.
 * NOTE: Once the `RBUFFER_LIMT_FCLOSED' flag has been set,
 *       these functions always return ZERO(0) immediately.
 * @throw: * :          [ringbuffer_write] This error was thrown by an RPC function.
 * @throw: E_INTERRUPT: [ringbuffer_write] The calling thread was interrupted.
 * @throw: E_SEGFAULT:  A faulty buffer was given. */
FUNDEF size_t KCALL ringbuffer_write(struct ringbuffer *__restrict self, USER CHECKED void const *buf, size_t num_bytes, iomode_t mode);
FUNDEF size_t KCALL ringbuffer_write_atomic(struct ringbuffer *__restrict self, USER CHECKED void const *buf, size_t num_bytes);
FUNDEF size_t KCALL ringbuffer_writev(struct ringbuffer *__restrict self, USER CHECKED struct iovec const *iov, size_t iov_offset, size_t num_bytes, iomode_t mode);
FUNDEF size_t KCALL ringbuffer_writev_atomic(struct ringbuffer *__restrict self, USER CHECKED struct iovec const *iov, size_t iov_offset, size_t num_bytes, iomode_t mode);

/* Write all data to the ring buffer and keep blocking
 * while a portion remains that hasn't been written.
 * If the buffer is closed, stop prematurely and return the amount that was written. */
FUNDEF size_t KCALL ringbuffer_writeall(struct ringbuffer *__restrict self, USER CHECKED void const *buf, size_t num_bytes, iomode_t mode);

/* Asynchronously wait for a read/write operation to become non-blocking:
 *   - Specify `POLLIN' to poll for `ringbuffer_read()'
 *   - Specify `POLLOUT' to poll for `ringbuffer_write()'
 * @param: mode: Set of `POLLIN|POLLOUT' from <bits/poll.h>
 * @return: * :  Set of `POLLIN|POLLOUT|POLLHUP'
 * NOTE: `POLLHUP' is returned when the buffer was been closed. */
FUNDEF unsigned int KCALL ringbuffer_poll(struct ringbuffer *__restrict self, unsigned int mode);

/* Poll for a specific buffer conditions.
 * @return: true:  The specified condition was met.
 * @return: false: The specified condition isn't met. */
LOCAL bool KCALL ringbuffer_poll_full(struct ringbuffer *__restrict self);
LOCAL bool KCALL ringbuffer_poll_nonfull(struct ringbuffer *__restrict self);
LOCAL bool KCALL ringbuffer_poll_empty(struct ringbuffer *__restrict self);
LOCAL bool KCALL ringbuffer_poll_nonempty(struct ringbuffer *__restrict self);
LOCAL bool KCALL ringbuffer_poll_close(struct ringbuffer *__restrict self);

/* Wait for a specific buffer conditions.
 * @return: true:  The specified condition was met.
 * @return: false: The given `abs_timeout' expired. */
LOCAL bool KCALL ringbuffer_wait_full(struct ringbuffer *__restrict self, jtime_t abs_timeout);
LOCAL bool KCALL ringbuffer_wait_nonfull(struct ringbuffer *__restrict self, jtime_t abs_timeout);
LOCAL bool KCALL ringbuffer_wait_empty(struct ringbuffer *__restrict self, jtime_t abs_timeout);
LOCAL bool KCALL ringbuffer_wait_nonempty(struct ringbuffer *__restrict self, jtime_t abs_timeout);
LOCAL bool KCALL ringbuffer_wait_close(struct ringbuffer *__restrict self, jtime_t abs_timeout);

/* Check if the given condition is currently met. */
LOCAL ATTR_NOTHROW WEAK bool KCALL ringbuffer_isfull(struct ringbuffer *__restrict self);
LOCAL ATTR_NOTHROW WEAK bool KCALL ringbuffer_isnonfull(struct ringbuffer *__restrict self);
LOCAL ATTR_NOTHROW WEAK bool KCALL ringbuffer_isempty(struct ringbuffer *__restrict self);
LOCAL ATTR_NOTHROW WEAK bool KCALL ringbuffer_isnonempty(struct ringbuffer *__restrict self);
LOCAL ATTR_NOTHROW WEAK bool KCALL ringbuffer_isclosed(struct ringbuffer *__restrict self);


#ifndef __INTELLISENSE__
LOCAL ATTR_NOTHROW WEAK bool KCALL
ringbuffer_isfull(struct ringbuffer *__restrict self) {
 bool result;
 atomic_rwlock_read(&self->r_lock);
 result = ATOMIC_READ(self->r_rptr) == self->r_wptr;
 atomic_rwlock_endread(&self->r_lock);
 if (!result) result = ringbuffer_isclosed(self);
 return result;
}
LOCAL ATTR_NOTHROW WEAK bool KCALL
ringbuffer_isnonfull(struct ringbuffer *__restrict self) {
 bool result;
 atomic_rwlock_read(&self->r_lock);
 result = ATOMIC_READ(self->r_rptr) != self->r_wptr;
 atomic_rwlock_endread(&self->r_lock);
 if (!result) result = ringbuffer_isclosed(self);
 return result;
}

LOCAL ATTR_NOTHROW WEAK bool KCALL
ringbuffer_isempty(struct ringbuffer *__restrict self) {
 bool result;
 atomic_rwlock_read(&self->r_lock);
 result = (self->r_wptr == self->r_base &&
           ATOMIC_READ(self->r_rptr) == self->r_base+self->r_size);
 atomic_rwlock_endread(&self->r_lock);
 if (!result) result = ringbuffer_isclosed(self);
 return result;
}

LOCAL ATTR_NOTHROW WEAK bool KCALL
ringbuffer_isnonempty(struct ringbuffer *__restrict self) {
 byte_t *rptr; bool result;
 atomic_rwlock_read(&self->r_lock);
 rptr = ATOMIC_READ(self->r_rptr);
 result = (self->r_wptr != self->r_base ||
           rptr != self->r_base+self->r_size);
 atomic_rwlock_endread(&self->r_lock);
 if (!result) result = ringbuffer_isclosed(self);
 return result;
}

LOCAL ATTR_NOTHROW WEAK bool KCALL
ringbuffer_isclosed(struct ringbuffer *__restrict self) {
 return ATOMIC_READ(self->r_limt) & RBUFFER_LIMT_FCLOSED;
}

LOCAL bool KCALL
ringbuffer_poll_full(struct ringbuffer *__restrict self) {
 task_openchannel(RBUF_STATE_CHANNEL_FULL);
 task_connect_ghost(&self->r_stat);
 return ringbuffer_isfull(self);
}
LOCAL bool KCALL
ringbuffer_poll_nonfull(struct ringbuffer *__restrict self) {
 task_openchannel(RBUF_STATE_CHANNEL_NOTFULL);
 task_connect_ghost(&self->r_stat);
 return ringbuffer_isnonfull(self);
}
LOCAL bool KCALL
ringbuffer_poll_empty(struct ringbuffer *__restrict self) {
 task_openchannel(RBUF_STATE_CHANNEL_EMPTY);
 task_connect_ghost(&self->r_stat);
 return ringbuffer_isempty(self);
}
LOCAL bool KCALL
ringbuffer_poll_nonempty(struct ringbuffer *__restrict self) {
 task_connect_ghost(&self->r_stat);
 return ringbuffer_isnonempty(self);
}
LOCAL bool KCALL
ringbuffer_poll_close(struct ringbuffer *__restrict self) {
 task_connect_ghost(&self->r_stat);
 return ringbuffer_isclosed(self);
}

LOCAL bool KCALL
ringbuffer_wait_full(struct ringbuffer *__restrict self, jtime_t abs_timeout) {
 bool result = true;
 uintptr_t EXCEPT_VAR old_mask;
 assert(!task_isconnected());
 old_mask = task_channelmask(RBUF_STATE_CHANNEL_FULL);
 TRY {
  while (!ringbuffer_isfull(self)) {
   task_connect_ghost(&self->r_stat);
   if (ringbuffer_isfull(self))
       break;
   if (!task_waitfor(abs_timeout)) {
    result = false;
    break;
   }
  }
 } FINALLY {
  task_channelmask(old_mask);
 }
 return result;
}
LOCAL bool KCALL
ringbuffer_wait_nonfull(struct ringbuffer *__restrict self, jtime_t abs_timeout) {
 bool result = true;
 uintptr_t EXCEPT_VAR old_mask;
 assert(!task_isconnected());
 old_mask = task_channelmask(RBUF_STATE_CHANNEL_NOTFULL);
 TRY {
  while (!ringbuffer_isnonfull(self)) {
   task_connect_ghost(&self->r_stat);
   if (ringbuffer_isnonfull(self))
       break;
   if (!task_waitfor(abs_timeout)) {
    result = false;
    break;
   }
  }
 } FINALLY {
  task_channelmask(old_mask);
 }
 return result;
}
LOCAL bool KCALL
ringbuffer_wait_empty(struct ringbuffer *__restrict self, jtime_t abs_timeout) {
 bool result = true;
 uintptr_t EXCEPT_VAR old_mask;
 assert(!task_isconnected());
 old_mask = task_channelmask(RBUF_STATE_CHANNEL_EMPTY);
 TRY {
  while (!ringbuffer_isempty(self)) {
   task_connect_ghost(&self->r_stat);
   if (ringbuffer_isempty(self))
       break;
   if (!task_waitfor(abs_timeout)) {
    result = false;
    break;
   }
  }
 } FINALLY {
  task_channelmask(old_mask);
 }
 return result;
}
LOCAL bool KCALL
ringbuffer_wait_nonempty(struct ringbuffer *__restrict self, jtime_t abs_timeout) {
 bool result = true;
 uintptr_t EXCEPT_VAR old_mask;
 assert(!task_isconnected());
 old_mask = task_channelmask(0);
 TRY {
  while (!ringbuffer_isnonempty(self)) {
   task_connect_ghost(&self->r_stat);
   if (ringbuffer_isnonempty(self))
       break;
   if (!task_waitfor(abs_timeout)) {
    result = false;
    break;
   }
  }
 } FINALLY {
  task_channelmask(old_mask);
 }
 return result;
}
LOCAL bool KCALL
ringbuffer_wait_close(struct ringbuffer *__restrict self, jtime_t abs_timeout) {
 bool result = true;
 uintptr_t EXCEPT_VAR old_mask;
 assert(!task_isconnected());
 old_mask = task_channelmask(0);
 TRY {
  while (!ringbuffer_isclosed(self)) {
   task_connect_ghost(&self->r_stat);
   if (ringbuffer_isclosed(self))
       break;
   if (!task_waitfor(abs_timeout)) {
    result = false;
    break;
   }
  }
 } FINALLY {
  task_channelmask(old_mask);
 }
 return result;
}

#endif /* __INTELLISENSE__ */


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_RINGBUFFER_H */
