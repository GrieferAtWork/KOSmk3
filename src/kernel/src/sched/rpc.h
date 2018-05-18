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
#ifndef GUARD_KERNEL_SRC_SCHED_RPC_H
#define GUARD_KERNEL_SRC_SCHED_RPC_H 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <sched/task.h>
#include <sched/signal.h>

DECL_BEGIN

/* Amount of static RPC slots before dynamic ones must be allocated. */
#define CONFIG_STATIC_RPC_SLOTS  2

struct rpc_slot {
    task_rpc_t       rs_fun;  /* [1..1] the function that should be called. */
    void            *rs_arg;  /* [?..?] Argument passed to `ars_fun' */
#define RPC_SLOT_FNORMAL 0x0000
#define RPC_SLOT_FUASYNC TASK_RPC_UASYNC
#define RPC_SLOT_FUSYNC  TASK_RPC_USYNC
#define RPC_SLOT_FUSER   TASK_RPC_USER
    uintptr_t        rs_flag; /* Set of `RPC_SLOT_F*' */
    struct sig      *rs_done; /* [0..1] A signal that will be broadcast when the RPC has finished, or NULL.
                               *  HINT: This signal is used for synchronous execution of RPC commands. */
};

struct rpc_info {
    size_t           ri_cnt;  /* Amount of RPC buffers in use. */
    size_t           ri_siz;  /* Allocated size of `ri_vec' vector. */
    struct rpc_slot *ri_vec;  /* [0..ri_cnt|alloc(ri_siz)][owned_if(!= ri_sbuf)]
                               * Vector of RPC callbacks. */
#if CONFIG_STATIC_RPC_SLOTS != 0
    struct rpc_slot  ri_sbuf[CONFIG_STATIC_RPC_SLOTS]; /* Static (aka. preallocated) RPC buffer. */
#endif
};

INTDEF ATTR_PERTASK struct rpc_info my_rpc;

/* Allocate a new RPC slot and set the `TASK_STATE_FINTERRUPTING' bit.
 * Returns NULL and clear the `TASK_STATE_FINTERRUPTING'
 * bit if the `TASK_STATE_FTERMINATING' bit has been set. */
INTDEF struct rpc_slot *KCALL rpc_alloc(struct task *__restrict thread);

DECL_END

#endif /* !GUARD_KERNEL_SRC_SCHED_RPC_H */
