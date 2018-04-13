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
#ifndef GUARD_KERNEL_SRC_FS_WORDBUFFER_C
#define GUARD_KERNEL_SRC_FS_WORDBUFFER_C 1

#include <hybrid/compiler.h>
#include <fs/wordbuffer.h>

/* Async-write-safe buffer (with a limited size & read-all operation)
 * -> Used as canon input buffer for keyboard I/O */

DECL_BEGIN

/* Read one key stroke from the buffer and return it.
 * Block indefinitely if no keys are available. */
PUBLIC wordbuffer_word_t KCALL
wordbuffer_getword(struct wordbuffer *__restrict self) {
 wordbuffer_word_t result;
 /* Try to read in a key. */
 while ((result = wordbuffer_trygetword(self)) == WORDBUFFER_EMPTY_WORD) {
  struct async_task_connection con;
  /* Connect to the buffer-available signal. */
  task_connect_async(&con,&self->wb_avail);
  /* Check for available strokes again. */
  result = wordbuffer_trygetword(self);
  if (result != WORDBUFFER_EMPTY_WORD) {
   task_disconnect_async();
   break;
  }
  /* Wait for strokes to become ready. */
  task_wait_async();
 }
 return result;
}

/* Same as `wordbuffer_getword()', but return `WORDBUFFER_EMPTY_WORD' if no strokes are available. */
PUBLIC wordbuffer_word_t KCALL
wordbuffer_trygetword(struct wordbuffer *__restrict self) {
 union wordbuffer_state state,new_state;
 wordbuffer_word_t result;
 do {
  uintptr_t readpos;
  state.bs_state = ATOMIC_READ(self->wb_state.bs_state);
  if (!state.bs_rcnt) return WORDBUFFER_EMPTY_WORD; /* Buffer is empty. */
  /* Determine the position of the next character to-be read. */
  readpos = (state.bs_wpos-state.bs_rcnt) % MAX_INPUT;
  /* Read the character at the current read-position */
  result = self->wb_buffer[readpos];
  new_state.bs_state = state.bs_state;
  new_state.bs_rcnt -= 1;
  COMPILER_READ_BARRIER();
 } while (!ATOMIC_CMPXCH_WEAK(self->wb_state.bs_state,
                              state.bs_state,
                              new_state.bs_state));
 return result;
}

/* Add the given `key' stroke to the buffer and wake waiters.
 * NOTE: This function is async-safe. */
PUBLIC ASYNCSAFE void KCALL
wordbuffer_putword(struct wordbuffer *__restrict self, wordbuffer_word_t key) {
 union wordbuffer_state state,new_state;
 do {
  state.bs_state = ATOMIC_READ(self->wb_state.bs_state);
  /* Calculate the new keyboard state */
  new_state.bs_state = state.bs_state;
  if (new_state.bs_rcnt < MAX_INPUT)
      new_state.bs_rcnt += 1;
  new_state.bs_wpos += 1;
  new_state.bs_wpos %= MAX_INPUT;
  /* Write the new key. */
  self->wb_buffer[state.bs_wpos] = key;
 } while (!ATOMIC_CMPXCH_WEAK(self->wb_state.bs_state,
                              state.bs_state,
                              new_state.bs_state));
 /* If the buffer was empty before, signal that data is now available. */
 if (state.bs_rcnt == 0)
     async_sig_broadcast(&self->wb_avail);
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_WORDBUFFER_C */
