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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_ENVIRON_H
#define GUARD_KERNEL_INCLUDE_KERNEL_ENVIRON_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/vm.h>
#include <kos/environ.h>

DECL_BEGIN

/* [?..?][lock(THIS_VM->vm_lock)] User-space address of the VM's environment block. */
DATDEF ATTR_PERVM USER UNCHECKED struct process_environ *vm_environ;


/* Construct a memory region describing application environment
 * data, matching the given `argv' and `envp' data vectors.
 * NOTE: This function is assuming that the calling thread is the
 *       only thread running in the associated VM (user-space portion)!
 * NOTE: The `struct process_environ' structure contained
 *       in the resulting region hasn't been relocated, yet.
 *       Once the caller has mapped it, they should invoke
 *      `environ_relocate()' in order to adjust region-relative
 *       pointers.
 * NOTE: Because of the fact that a node is required for a temporary
 *       mapping of the region, that node is returned, so the caller
 *       can re-use it. */
FUNDEF ATTR_RETNONNULL struct vm_node *KCALL
environ_alloc(USER UNCHECKED char *USER UNCHECKED *argv,
              USER UNCHECKED char *USER UNCHECKED *envp);

/* Relocate relative pointers in `self' to become absolute. */
FUNDEF void KCALL
environ_relocate(USER CHECKED struct process_environ *__restrict self);


/* Construct a default, minimal application environment suitable for /bin/init */
FUNDEF void KCALL environ_create(char const *__restrict init_name,
                                 size_t init_name_length);



DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_ENVIRON_H */
