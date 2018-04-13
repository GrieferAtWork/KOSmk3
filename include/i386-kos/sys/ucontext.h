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
#ifndef _I386_KOS_SYS_UCONTEXT_H
#define _I386_KOS_SYS_UCONTEXT_H 1
#define _SYS_UCONTEXT_H 1

#include <__stdinc.h>
#include <features.h>

#if !defined(__CRT_GLC) && !defined(__CRT_CYG)
#error "<sys/ucontext.h> is not supported by the linked libc"
#endif /* !__CRT_GLC && !__CRT_CYG */

#ifdef __CRT_KOS
#if __KOS_VERSION__ >= 300
#include <parts/kos3/i386/ucontext.h>
#else
#include <parts/glc/i386/ucontext.h>
#endif
#elif defined(__CRT_CYG)
#include <parts/cyg/i386/ucontext.h>
#else
#include <parts/glc/i386/ucontext.h>
#endif

#endif /* !_I386_KOS_SYS_UCONTEXT_H */
