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
#ifndef GUARD_KERNEL_INCLUDE_FS_CANONBUFFER_H
#define GUARD_KERNEL_INCLUDE_FS_CANONBUFFER_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <linux/limits.h>

/* General-purpose CANON-buffer. */

DECL_BEGIN

struct canonbuffer {
    atomic_rwlock_t cb_lock;    /* Lock for the canon buffer. */
    size_t          cb_written; /* [lock(cb_lock)] Amount of bytes already written. */
    size_t          cb_bufsize; /* [lock(cb_lock)] Total buffer size. */
    byte_t         *cb_buffer;  /* [0..cb_bufsize][owned][lock(cb_lock)] The underlying canon buffer. */
    WEAK size_t     cb_bufmax;  /* The max size that the buffer may grow to (usually `MAX_CANON').
                                 * NOTE: Due to heap alignment and lazy allocations, `cb_bufsize' may
                                 *       be slightly larger than this. Also: Lowering this value does
                                 *       not require discarding already written data that would violate
                                 *       new rules. */
};

struct canon {
    byte_t *c_base;  /* [0..c_size][owned] Canon base address. */
    size_t  c_size;  /* Canon size. */
    size_t  c_alloc; /* Allocated canon size. */
};

#define CANONBUFFER_INIT       {ATOMIC_RWLOCK_INIT,0,0,NULL,MAX_CANON}
#define canonbuffer_init(self) \
   (atomic_rwlock_init(&(self)->cb_lock), \
   (self)->cb_written = (self)->cb_bufsize = 0, \
   (self)->cb_buffer = NULL,(self)->cb_bufmax = MAX_CANON)
#define canonbuffer_cinit(self) \
   (atomic_rwlock_cinit(&(self)->cb_lock), \
   (self)->cb_bufmax = MAX_CANON)


/* Finalize a given canon buffer. */
FUNDEF ATTR_NOTHROW void KCALL
canonbuffer_fini(struct canonbuffer *__restrict self);

/* Add a single character to the canon buffer.
 * @throw: E_BADALLOC: The available buffer has been filled and a new one couldn't be allocated.
 * @return: true:      The given `ch' was added to the buffer.
 * @return: false:     The buffer is full and the given `ch' wasn't added. */
FUNDEF bool KCALL canonbuffer_putc(struct canonbuffer *__restrict self, byte_t ch);
/* Take away the last character written and store it in `*ch'
 * @return: true:  `*ch' was filled with the last character written.
 * @return: false: The canon was empty. */
FUNDEF bool KCALL canonbuffer_unputc(struct canonbuffer *__restrict self, byte_t *__restrict ch);

/* Capture / Release the current buffer, while simultaniously
 * resetting the active buffer to being empty.
 * These functions are what differentiates canons from other buffers.
 * >> struct canon EXCEPT_VAR c;
 * >> c = canonbuffer_capture(self);
 * >> TRY {
 * >>     PROCESS_DATA(c.c_base,c.c_size);
 * >> } FINALLY {
 * >>     canonbuffer_release(self,c);
 * >> } */
FUNDEF ATTR_NOTHROW struct canon KCALL
canonbuffer_capture(struct canonbuffer *__restrict self);
FUNDEF ATTR_NOTHROW void KCALL
canonbuffer_release(struct canonbuffer *__restrict self, struct canon can);

/* Same as `canonbuffer_capture()', but return a copy of the current buffer. */
FUNDEF struct canon KCALL
canonbuffer_clone(struct canonbuffer *__restrict self);


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_CANONBUFFER_H */
