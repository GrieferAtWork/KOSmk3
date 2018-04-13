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
#ifndef GUARD_KERNEL_INCLUDE_FS_PIPE_H
#define GUARD_KERNEL_INCLUDE_FS_PIPE_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <sched/signal.h>
#include <fs/ringbuffer.h>

DECL_BEGIN

#ifndef CONFIG_PIPE_DEFAULT_LIMIT
#define CONFIG_PIPE_DEFAULT_LIMIT  4096
#endif

struct pipe {
    ATOMIC_DATA ref_t p_refcnt; /* Pipe reference counter. */
    struct ringbuffer p_buffer; /* Ring buffer for pipe data. */
};

/* Increment/decrement the reference counter of the given pipe `x' */
#define pipe_incref(x)  ATOMIC_FETCHINC((x)->p_refcnt)
#define pipe_decref(x) (ATOMIC_DECFETCH((x)->p_refcnt) || (pipe_destroy(x),0))

/* Destroy a previously allocated pipe. */
FUNDEF ATTR_NOTHROW void KCALL pipe_destroy(struct pipe *__restrict self);

/* Allocate and return a new pipe. */
FUNDEF ATTR_RETNONNULL REF struct pipe *KCALL pipe_alloc(size_t max_size);





struct pipereader {
    ATOMIC_DATA ref_t pr_refcnt; /* Pipe reader reference counter. */
    REF struct pipe  *pr_pipe;   /* [1..1][const] The associated regular pipe / reader-end. */
};

/* Increment/decrement the reference counter of the given pipereader `x' */
#define pipereader_incref(x)  ATOMIC_FETCHINC((x)->pr_refcnt)
#define pipereader_decref(x) (ATOMIC_DECFETCH((x)->pr_refcnt) || (pipereader_destroy(x),0))

/* Destroy a previously allocated pipereader. */
FUNDEF ATTR_NOTHROW void KCALL
pipereader_destroy(struct pipereader *__restrict self);

/* Allocate and return a new reader for the given pipe. */
FUNDEF ATTR_RETNONNULL REF struct pipereader *KCALL
pipereader_alloc(struct pipe *__restrict p);





#undef CONFIG_PIPEWRITER_MATCHES_PIPEREADER
#define CONFIG_PIPEWRITER_MATCHES_PIPEREADER 1
struct pipewriter {
    ATOMIC_DATA ref_t pw_refcnt; /* Pipe reader reference counter. */
    REF struct pipe  *pw_pipe;   /* [1..1][const] The associated regular pipe / writer-end. */
};

/* Increment/decrement the reference counter of the given pipewriter `x' */
#define pipewriter_incref(x)  ATOMIC_FETCHINC((x)->pw_refcnt)
#define pipewriter_decref(x) (ATOMIC_DECFETCH((x)->pw_refcnt) || (pipewriter_destroy(x),0))

/* Destroy a previously allocated pipewriter. */
FUNDEF ATTR_NOTHROW void KCALL
pipewriter_destroy(struct pipewriter *__restrict self);

/* Allocate and return a new reader for the given pipe. */
FUNDEF ATTR_RETNONNULL REF struct pipewriter *KCALL
pipewriter_alloc(struct pipe *__restrict p);



DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_PIPE_H */
