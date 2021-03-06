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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_IOPORT_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_IOPORT_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>

DECL_BEGIN

#define __SIZEOF_IOPORT_T__ 2
#ifdef __CC__
typedef u16 ioport_t;
#endif /* __CC__ */

#define IO_ALLOCBASE  0x1000 /* First I/O address that can be allocated dynamically. */
#define IO_MAXPORT    0xffff /* The greatest legal I/O port. */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_IOPORT_H */
