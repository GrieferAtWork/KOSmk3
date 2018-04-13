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
#ifndef GUARD_KERNEL_INCLUDE_SCHED_USERTHREAD_H
#define GUARD_KERNEL_INCLUDE_SCHED_USERTHREAD_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/sections.h>

DECL_BEGIN

/* Control functionality for user-space thread libraries. */

#ifdef __CC__

/* [0..1][lock(PRIVATE(THIS_TASK))]
 * The user-space address of the TID pointer of this thread.
 * Initially, this pointer is set according to the `ctid' argument
 * of the `clone()' system call. And in addition, it may be altered
 * at a later time using the `set_tid_address()' system call.
 * -> When the thread terminates and this pointer is non-NULL, the kernel will attempt to
 *    write ZERO(0) to this address (failure to do this is silently ignored), before broadcasting
 *    the signal of a futex at `vm_getfutex(THIS_TID_ADDRESS)' (should that futex exists)
 *    This functionality is used to implement user-space thread_join()-style functionality. */
DATDEF ATTR_PERTASK USER UNCHECKED VIRT pid_t *_this_tid_address;
#define THIS_TID_ADDRESS PERTASK(_this_tid_address)

#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_SCHED_USERTHREAD_H */
