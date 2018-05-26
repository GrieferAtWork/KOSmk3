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
#ifndef _KOS_I386_KOS_BITS_COMPAT_H
#define _KOS_I386_KOS_BITS_COMPAT_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifndef __EXPOSE_CPU_COMPAT
#if (defined(__USE_KOS) && !defined(__KERNEL__)) || \
     defined(__x86_64__)
#define __EXPOSE_CPU_COMPAT 1
#endif
#endif /* !__EXPOSE_CPU_COMPAT */


#ifdef __EXPOSE_CPU_COMPAT
#ifdef __x86_64__
#define __X86_PTRCC(T)      __UINT32_TYPE__   /* Compatibility pointer. */
#define __X86_PTR32(T)      __UINT32_TYPE__   /* 32-bit pointer. */
#define __X86_PTR64(T)      T *               /* 64-bit pointer. */
#define __SIZEOF_X86_INTPTRCC__ 4
#define __X86_INTPTRCC      __UINT32_TYPE__   /* Compatibility Integer pointer. */
#define __X86_INTHALFPTRCC  __UINT16_TYPE__   /* Compatibility Integer half-pointer. */
#define __X86_INTPTR32      __UINT32_TYPE__   /* 32-bit Integer pointer. */
#define __X86_INTPTR64      __UINTPTR_TYPE__  /* 64-bit Integer pointer. */
#else
#define __X86_PTRCC(T)      __UINT64_TYPE__   /* Compatibility pointer. */
#define __X86_PTR32(T)      T *               /* 32-bit pointer. */
#define __X86_PTR64(T)      __UINT64_TYPE__   /* 64-bit pointer. */
#define __SIZEOF_X86_INTPTRCC__ 8
#define __X86_INTPTRCC      __UINT64_TYPE__   /* Compatibility Integer pointer. */
#define __X86_INTHALFPTRCC  __UINT32_TYPE__   /* Compatibility Integer half-pointer. */
#define __X86_INTPTR32      __UINTPTR_TYPE__  /* 32-bit Integer pointer. */
#define __X86_INTPTR64      __UINT64_TYPE__   /* 64-bit Integer pointer. */
#endif
#endif /* __EXPOSE_CPU_COMPAT */


__SYSDECL_END

#endif /* !_KOS_I386_KOS_BITS_COMPAT_H */
