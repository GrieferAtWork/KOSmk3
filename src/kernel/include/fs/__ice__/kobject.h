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
#ifndef GUARD_KERNEL_INCLUDE_FS_KOBJECT_H
#define GUARD_KERNEL_INCLUDE_FS_KOBJECT_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/sections.h>
#include <sched/rwlock.h>

DECL_BEGIN

#ifdef __CC__
struct kobject;

struct kobject_ops {
    struct driver                *o_driver;    /* [1..1] The driver implementing this type of object. */
    LIST_NODE(struct kobject_ops) o_types;     /* [0..1][lock(INTERNAL(...))] Chain of known object types (used to allow for safe driver unbinding). */
    struct {
        atomic_rwlock_t           i_lock;      /* Lock for `i_list' */
        LIST_HEAD(struct kobject) i_list;      /* [0..1][lock(i_lock)] Chain of all instances of this object type. */
    }                             o_instances; /* List of object instances (killed when the driver is unloaded) */
    /* Object operators. (All of these are [0..1]) */

    /* [0..1][locked(WRITE(o_lock))] An optional object finalization callback. */
    void (KCALL *o_fini)(struct kobject *__restrict self);
};

FUNDEF void KCALL register_kobject_type(struct kobject_ops *__restrict ops);
#define DEFINE_KOBJECT_TYPE(ops)

struct kobject {
    ATOMIC_DATA ref_t         o_refcnt;  /* Reference counter. */
    rwlock_t                  o_lock;    /* Lock for this object. (used to lock driver access)
                                          * During invocation of operators, a read-lock is always held. */
    struct kobject_ops       *o_ops;     /* [1..1][lock(o_lock)] Object operators. */
    LIST_NODE(struct kobject) o_objects; /* [0..1][lock(o_ops->o_instances.i_lock)] Chain of all object instances. */
    /* ... Object-specific data goes here. */
};




#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_KOBJECT_H */
