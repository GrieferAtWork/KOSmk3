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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_IOPORT_H
#define GUARD_KERNEL_INCLUDE_KERNEL_IOPORT_H 1

#include <hybrid/compiler.h>
#include <dev/devconfig.h>

#ifdef CONFIG_HAVE_IOPORTS
#include <stdbool.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/ioport.h>
#else
#error "Unsupported Architecture"
#endif

DECL_BEGIN

#ifdef __CC__
/* Dynamically allocate an I/O address range of `num_ports' consecutive I/O ports.
 * All 1-bits of the returned port can be masked by `mask'.
 * @throw: E_BADALLOC: Insufficient available dynamic I/O ports.
 * @return: * : The first allocated I/O port. */
FUNDEF ioport_t KCALL io_alloc(ioport_t mask, ioport_t num_ports);

/* Try to allocate the given I/O address range.
 * NOTE: Can be used to allocate I/O memory that is normally reserved. */
FUNDEF bool KCALL io_alloc_at(ioport_t base, ioport_t num_ports);

/* Free a previously allocated I/O port range. */
FUNDEF void KCALL io_free(ioport_t base, ioport_t num_ports);
#endif /* __CC__ */

DECL_END
#endif /* CONFIG_HAVE_IOPORTS */

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_IOPORT_H */
