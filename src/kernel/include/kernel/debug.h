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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_DEBUG_H
#define GUARD_KERNEL_INCLUDE_KERNEL_DEBUG_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <stdarg.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386-kos/debug.h>
#else
#error "Unsupported architecture"
#endif

DECL_BEGIN

FUNDEF ASYNCSAFE void ATTR_CDECL debug_printf(char const *__restrict format, ...);
FUNDEF ASYNCSAFE void KCALL debug_vprintf(char const *__restrict format, va_list args);

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_DEBUG_H */
