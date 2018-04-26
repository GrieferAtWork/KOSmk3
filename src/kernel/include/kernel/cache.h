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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_CACHE_H
#define GUARD_KERNEL_INCLUDE_KERNEL_CACHE_H 1

#include <hybrid/compiler.h>
#include <stdbool.h>

/* Kernel Cache Control */

DECL_BEGIN

#ifdef __CC__
/* Returns `false' if caches have been sufficient reduced in size.
 * Returns `true' otherwise. */
FUNDEF ATTR_NOTHROW bool KCALL kernel_cc_continue(void);

/* Clear global caches until `(*test)(arg)' has returned `true'.
 * If caches are already being cleared by the calling thread, return `false' immediately.
 * If all caches have been cleared (as best as KOS is able to), `false' is returned. */
FUNDEF ATTR_NOTHROW bool KCALL
kernel_cc_invoke(/*ATTR_NOTHROW*/bool (KCALL *test)(void *arg), void *arg);
#endif

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_CACHE_H */
