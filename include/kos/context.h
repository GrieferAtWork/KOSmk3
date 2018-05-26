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
#ifndef _KOS_CONTEXT_H
#define _KOS_CONTEXT_H 1

#include <__stdinc.h>
#include <hybrid/host.h>

#if defined(__i386__) || defined(__x86_64__)
#include "i386-kos/bits/cpu-context.h"
#include "i386-kos/bits/fpu-context.h"
#else
#error "Unsupported arch"
#endif

#ifdef __CC__
__SYSDECL_BEGIN

/* Get/Set/exchange the current CPU context.
 * NOTE: `cpu_getcontext()' returns `1' after the initial call, and `0' after every other. */
__LIBC __ATTR_RETURNS_TWICE int (__FCALL cpu_getcontext)(struct cpu_context *__restrict __old_context);
__LIBC void (__FCALL cpu_xchcontext)(struct cpu_context const *__restrict __new_context, struct cpu_context *__restrict __old_context);
__LIBC __ATTR_NORETURN void (__FCALL cpu_setcontext)(struct cpu_context const *__restrict __new_context);

__SYSDECL_END
#endif

#endif /* !_KOS_CONTEXT_H */
