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
#ifndef GUARD_KERNEL_INCLUDE_FS_WORDBUFFER_H
#define GUARD_KERNEL_INCLUDE_FS_WORDBUFFER_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/typecore.h>
#include <sched/async_signal.h>
#include <linux/limits.h>

/* Async-write-safe buffer (with a limited size & read-all operation)
 * -> Used as canon input buffer for keyboard I/O */

DECL_BEGIN

typedef u32 wordbuffer_word_t;
#define WORDBUFFER_EMPTY_WORD  0

#define WORDBUFFER_SIZE   (MAX_INPUT+1)

union wordbuffer_state {
    ATOMIC_DATA uintptr_t          bs_state; /* The current state of the key buffer. */
    struct {
        ATOMIC_DATA uintptr_half_t bs_wpos;  /* [< WORDBUFFER_SIZE] Index to the buffer location where the next character will be written. */
        ATOMIC_DATA uintptr_half_t bs_rcnt;  /* [< WORDBUFFER_SIZE] The number of characters left to-be read. */
    };
};

struct wordbuffer {
    union wordbuffer_state  wb_state;                   /* The current state of the key buffer. */
    struct async_sig        wb_avail;                   /* Signal sent when data becomes available after the buffer was empty. */
    wordbuffer_word_t       wb_buffer[WORDBUFFER_SIZE]; /* The actual buffer. */
};

#define WORDBUFFER_INIT     { { 0 }, ASYNC_SIG_INIT, { 0, } }
#define wordbuffer_cinit(x) (async_sig_cinit(&(x)->wb_avail))


/* Read one key stroke from the buffer and return it.
 * Block indefinitely if no keys are available.
 * NOTE: Reads return keys in order of being put (typed) */
FUNDEF wordbuffer_word_t KCALL wordbuffer_getword(struct wordbuffer *__restrict self);
/* Same as `wordbuffer_getword()', but return `WORDBUFFER_EMPTY_WORD' if no words are available. */
FUNDEF wordbuffer_word_t KCALL wordbuffer_trygetword(struct wordbuffer *__restrict self);

/* Read in a packet consisting of `num_words' words.
 * Packets can only be read one-at-a-time, and the caller is responsible
 * to ensure packet alignment by _ALWAYS_ reading the same amount of data
 * as is written by multiple successive calls to `wordbuffer_putword()',
 * or better yet, a call to `wordbuffer_putpacket()'
 * If that rule was broken, packet would become unaligned _PERMANENTLY_
 * NOTE: To prevent misalignments, make sure that `(WORDBUFFER_SIZE % num_words) == 0' */
FUNDEF void KCALL wordbuffer_getpacket(struct wordbuffer *__restrict self,
                                       wordbuffer_word_t *__restrict buf,
                                       size_t num_words);
FUNDEF bool KCALL wordbuffer_trygetpacket(struct wordbuffer *__restrict self,
                                          wordbuffer_word_t *__restrict buf,
                                          size_t num_words);

/* Add the given `key' stroke to the buffer and wake waiters.
 * NOTE: This function is async-safe.
 * NOTE: If the buffer was full, the least recently written key is overwritten. */
FUNDEF ASYNCSAFE void KCALL
wordbuffer_putword(struct wordbuffer *__restrict self,
                   wordbuffer_word_t key);

/* Same semantic behavior as calling `wordbuffer_putword()' `num_words'
 * number of times, passing words from `words' in successive order.
 * However, this function will only broadcast the data-available signal
 * once, after the entire packet has been written. */
FUNDEF ASYNCSAFE void KCALL
wordbuffer_putpacket(struct wordbuffer *__restrict self,
                     wordbuffer_word_t const *__restrict words,
                     size_t num_words);


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_WORDBUFFER_H */
