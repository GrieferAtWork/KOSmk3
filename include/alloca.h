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
#ifndef _ALLOCA_H
#define _ALLOCA_H 1

#include "__stdinc.h"
#include <hybrid/alloca.h>

#define alloca(s)  __ALLOCA((s))

#ifdef __USE_KXS
#define talloca(T,n) (T *)__ALLOCA((n)*sizeof(T))
#define oalloca(T)   (T *)__ALLOCA(sizeof(T))
#endif

#ifdef __USE_KOS
#if defined(_MALLOC_H) || defined(_STDLIB_H) || \
   (defined(__KERNEL__) && __KOS_VERSION__ >= 300 && \
    defined(GUARD_KERNEL_INCLUDE_KERNEL_MALLOC_H))
#include "parts/kos2/malloca.h"
#define malloca(s) __malloca(s)
#define calloca(s) __calloca(s)
#define freea(p)   __freea(p)
#ifndef __USE_KOS_DEPRECATED
#define amalloc(s) __malloca(s)
#define acalloc(s) __calloca(s)
#define afree(p)   __freea(p)
#endif /* !__USE_KOS_DEPRECATED */
#endif
#endif /* __USE_KOS */

#endif /* !_ALLOCA_H */
