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
#ifndef GUARD_KERNEL_SRC_FS_CANONBUFFER_C
#define GUARD_KERNEL_SRC_FS_CANONBUFFER_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/heap.h>
#include <fs/canonbuffer.h>
#include <linux/limits.h>
#include <except.h>
#include <string.h>
#include <assert.h>

/* General-purpose CANON-buffer. */

DECL_BEGIN

#define CANON_GFP     GFP_SHARED
#define CANON_HEAP  (&kernel_heaps[CANON_GFP & __GFP_HEAPMASK])

/* Finalize a given canon buffer. */
PUBLIC ATTR_NOTHROW void KCALL
canonbuffer_fini(struct canonbuffer *__restrict self) {
 assertf(IS_ALIGNED(self->cb_bufsize,HEAP_ALIGNMENT),
         "%p",self->cb_bufsize);
 if (self->cb_bufsize)
     heap_free_untraced(CANON_HEAP,self->cb_buffer,self->cb_bufsize,CANON_GFP);
}

/* Add a single character to the canon buffer.
 * @throw: E_BADALLOC: The available buffer has been filled and a new one couldn't be allocated.
 * @return: true:  The given `ch' was added to the buffer.
 * @return: false: The buffer is full and the given `ch' wasn't added. */
PUBLIC bool KCALL
canonbuffer_putc(struct canonbuffer *__restrict self, byte_t ch) {
 size_t EXCEPT_VAR written_size;
again:
 atomic_rwlock_read(&self->cb_lock);
 do {
  written_size = ATOMIC_READ(self->cb_written);
  assert(IS_ALIGNED(self->cb_bufsize,HEAP_ALIGNMENT));
  assert(written_size <= self->cb_bufsize);
  if unlikely(written_size == self->cb_bufsize) {
   size_t new_size,max_size;
   struct heapptr EXCEPT_VAR new_buffer;
   struct heapptr old_buffer;
   /* Must increase the available buffer size. */
   atomic_rwlock_endread(&self->cb_lock);
   new_size = written_size * 2;
   if (!new_size) new_size = 32;
   max_size = ATOMIC_READ(self->cb_bufmax);
   if (new_size >= max_size)
       new_size = max_size;
   if unlikely(new_size <= written_size)
      return false; /* Buffer limit has been reached. */
   /* Allocate the new buffer. */
   TRY {
    new_buffer = heap_alloc_untraced(CANON_HEAP,new_size,CANON_GFP);
   } CATCH_HANDLED (E_BADALLOC) {
    new_buffer = heap_alloc_untraced(CANON_HEAP,written_size+1,CANON_GFP);
   }
   atomic_rwlock_write(&self->cb_lock);
   if unlikely(new_buffer.hp_siz <= self->cb_bufsize) {
    /* Buffer grew in the mean time. */
    atomic_rwlock_endwrite(&self->cb_lock);
    heap_free_untraced(CANON_HEAP,new_buffer.hp_ptr,new_buffer.hp_siz,CANON_GFP);
    goto again;
   }
   /* Install the new buffer. */
   old_buffer.hp_ptr = self->cb_buffer;
   old_buffer.hp_siz = self->cb_bufsize;
   /* Copy old buffer data. */
   memcpy(new_buffer.hp_ptr,
          old_buffer.hp_ptr,
          self->cb_written);
   self->cb_bufsize = new_buffer.hp_siz;
   assert(IS_ALIGNED(self->cb_bufsize,HEAP_ALIGNMENT));
   self->cb_buffer  = (byte_t *)new_buffer.hp_ptr;
   atomic_rwlock_endwrite(&self->cb_lock);
   /* Free the old buffer. */
   if (old_buffer.hp_siz)
       heap_free_untraced(CANON_HEAP,old_buffer.hp_ptr,old_buffer.hp_siz,CANON_GFP);
   /* Start over. */
   goto again;
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->cb_written,written_size,written_size+1));
 /* Actually write the new character. */
 self->cb_buffer[written_size] = ch;
 atomic_rwlock_endread(&self->cb_lock);
 return true;
}

/* Take away the last character written and store it in `*ch'
 * @return: true:  `*ch' was filled with the last character written.
 * @return: false: The canon was empty. */
PUBLIC bool KCALL
canonbuffer_unputc(struct canonbuffer *__restrict self, byte_t *__restrict ch) {
 size_t old_size; byte_t result_byte;
 atomic_rwlock_read(&self->cb_lock);
 do {
  old_size = ATOMIC_READ(self->cb_written);
  if (!old_size) {
   atomic_rwlock_endread(&self->cb_lock);
   return false;
  }
  result_byte = self->cb_buffer[old_size-1];
 } while (!ATOMIC_CMPXCH_WEAK(self->cb_written,old_size,old_size-1));
 atomic_rwlock_endread(&self->cb_lock);
 COMPILER_WRITE_BARRIER();
 *ch = result_byte;
 return true;
}


/* Capture / Release the current buffer, while simultaniously
 * resetting the active buffer to being empty.
 * These functions are what differentiates canons from other buffers.
 * >> struct canon EXCEPT_VAR c;
 * >> c = canonbuffer_capture(self);
 * >> TRY {
 * >>     PROCESS_DATA(c.c_base,c.c_size);
 * >> } FINALLY {
 * >>     canonbuffer_release(self,c);
 * >> }
 */
PUBLIC ATTR_NOTHROW struct canon KCALL
canonbuffer_capture(struct canonbuffer *__restrict self) {
 struct canon result;
 atomic_rwlock_write(&self->cb_lock);
 /* Steal the active buffer. */
 result.c_base     = self->cb_buffer;
 result.c_size     = self->cb_written;
 result.c_alloc    = self->cb_bufsize;
 self->cb_bufsize  = self->cb_written = 0;
 /*self->cb_buffer = NULL;*/ /* Not required... */
 atomic_rwlock_endwrite(&self->cb_lock);
 return result;
}

PUBLIC ATTR_NOTHROW void KCALL
canonbuffer_release(struct canonbuffer *__restrict self,
                    struct canon can) {
 /* Try to restore the given canon as active buffer.
  * If the given `can' is empty, or a new buffer has
  * already been allocated in the mean time, free the
  * given `can' and return normally. */
 if (!can.c_alloc) return; /* No canon provided. */
 atomic_rwlock_write(&self->cb_lock);
 if (!self->cb_written &&
      self->cb_bufsize < can.c_alloc) {
  /* Install the given `can' */
  struct heapptr old_canon;
  old_canon.hp_ptr = self->cb_buffer;
  old_canon.hp_siz = self->cb_bufsize;
  self->cb_buffer  = can.c_base;
  self->cb_bufsize = can.c_alloc;
  assert(IS_ALIGNED(self->cb_bufsize,HEAP_ALIGNMENT));
  atomic_rwlock_endwrite(&self->cb_lock);
  /* Free the old canon buffer. */
  if unlikely(old_canon.hp_siz)
     heap_free_untraced(CANON_HEAP,old_canon.hp_ptr,old_canon.hp_siz,CANON_GFP);
 } else {
  /* Data was written in the mean time, or the given canon is smaller than the one saved.
   * Either way, free the given canon and keep the one already installed. */
  atomic_rwlock_endwrite(&self->cb_lock);
  heap_free_untraced(CANON_HEAP,can.c_base,can.c_alloc,CANON_GFP);
 }
}

PUBLIC struct canon KCALL
canonbuffer_clone(struct canonbuffer *__restrict self) {
 struct canon result;
 struct heapptr EXCEPT_VAR buffer;
 buffer.hp_siz = 0;
 TRY {
  for (;;) {
   /* Allocate a data block of sufficient size. */
   buffer = heap_realloc_untraced(CANON_HEAP,buffer.hp_ptr,buffer.hp_siz,
                                  ATOMIC_READ(self->cb_written),
                                  CANON_GFP,CANON_GFP);
   atomic_rwlock_write(&self->cb_lock);
   result.c_size = self->cb_written;
   if unlikely(buffer.hp_siz < result.c_size) {
    atomic_rwlock_endwrite(&self->cb_lock);
    continue;
   }
   /* Copy canon data. */
   memcpy(buffer.hp_ptr,self->cb_buffer,result.c_size);
   atomic_rwlock_endwrite(&self->cb_lock);
   break;
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  if (buffer.hp_siz)
      heap_free_untraced(CANON_HEAP,buffer.hp_ptr,buffer.hp_siz,CANON_GFP);
  error_rethrow();
 }
 result.c_base  = (byte_t *)buffer.hp_ptr;
 result.c_alloc = buffer.hp_siz;
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_CANONBUFFER_C */
