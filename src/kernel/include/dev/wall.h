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
#ifndef GUARD_KERNEL_INCLUDE_DEV_WALL_H
#define GUARD_KERNEL_INCLUDE_DEV_WALL_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/timespec.h>
#include <fs/device.h>

/* WALL-clock Abstract Kernel API Interface. */

DECL_BEGIN

struct wall {
    struct character_device w_dev; /* Underlying character device. */
};

/* A fallback, builtin wall clock that uses jiffies to calculate the time. */
DATDEF struct wall wall_kernel;

/* Get/Set the time according to the given wall.
 * @throw: E_IOERROR: Failed to read or write the time. */
FUNDEF struct timespec KCALL wall_gettime(struct wall *__restrict self);
FUNDEF void KCALL wall_settime(struct wall *__restrict self, struct timespec value);

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_DEV_WALL_H */
