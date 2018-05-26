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
#ifndef _KOS___EXCEPTION_DATA_H
#define _KOS___EXCEPTION_DATA_H 1

#include <__stdinc.h>
#include <hybrid/host.h>

#if defined(__i386__) || defined(__x86_64__)
#include "i386-kos/bits/__exception_data.h"
#else
#error "Unsupported arch"
#endif

__DECL_BEGIN

#ifndef __ARCH_SYSCALL_MAX_ARGC
#define __ARCH_SYSCALL_MAX_ARGC 6
#endif

/* Default exception data containers. */
#ifndef __PRIVATE_DEFAULT_FOREACH_EXCEPTION_DATA
#define __PRIVATE_DEFAULT_FOREACH_EXCEPTION_DATA(FUNC) \
     FUNC(noncontinuable) \
     FUNC(badalloc) \
     FUNC(invalid_handle) \
     FUNC(segfault) \
     FUNC(divide_by_zero) \
     FUNC(index_error) \
     FUNC(buffer_too_small) \
     FUNC(filesystem_error) \
     FUNC(net_error) \
     FUNC(no_device) \
     FUNC(unhandled_interrupt) \
     FUNC(unknown_systemcall) \
     FUNC(exit) \
     FUNC(system) \
     FUNC(illegal_instruction) \
     FUNC(invalid_segment) \
     FUNC(retry_rwlock) \
/**/
#endif /* !__PRIVATE_DEFAULT_FOREACH_EXCEPTION_DATA */



/* Enumerate all exception data containers. */
#ifndef __PRIVATE_FOREACH_EXCEPTION_DATA
#ifdef __PRIVATE_ARCH_FOREACH_EXCEPTION_DATA
#define __PRIVATE_FOREACH_EXCEPTION_DATA(FUNC) \
        __PRIVATE_DEFAULT_FOREACH_EXCEPTION_DATA(FUNC) \
        __PRIVATE_ARCH_FOREACH_EXCEPTION_DATA(FUNC)
#else
#define __PRIVATE_FOREACH_EXCEPTION_DATA(FUNC) \
        __PRIVATE_DEFAULT_FOREACH_EXCEPTION_DATA(FUNC)
#endif
#endif /* !__PRIVATE_FOREACH_EXCEPTION_DATA */

/* The number of extended exception information data pointers. */
#ifndef __EXCEPTION_INFO_NUM_DATA_POINTERS
#define __EXCEPTION_INFO_NUM_DATA_POINTERS  15
#endif
#define __EXCEPTION_INFO_SIZEOF_DATA       \
       (__EXCEPTION_INFO_NUM_DATA_POINTERS*__SIZEOF_POINTER__)


__DECL_END


#endif /* !_KOS___EXCEPTION_DATA_H */
