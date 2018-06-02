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
#ifndef _KOS_I386_KOS_BITS_FUTEX_H
#define _KOS_I386_KOS_BITS_FUTEX_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>
#include "compat.h"

__SYSDECL_BEGIN

#if defined(__CC__) && \
   (defined(__EXPOSE_CPU_COMPAT) || defined(__KERNEL__) || \
    defined(__BUILDING_LIBC))
struct __os_pollinfo {
    struct pollfd    *i_ufdvec; /* [0..i_cnt] Vector of poll descriptors. */
    __SIZE_TYPE__     i_ufdcnt; /* Number of elements. */
    struct pollfutex *i_ftxvec; /* [0..i_cnt] Vector of poll descriptors. */
    __SIZE_TYPE__     i_ftxcnt; /* Number of elements. */
    struct pollpid   *i_pidvec; /* [0..i_cnt] Vector of poll descriptors. */
    __SIZE_TYPE__     i_pidcnt; /* Number of elements. */
};
#endif


__SYSDECL_END

#ifdef __EXPOSE_CPU_COMPAT
#include "futex-compat.h"
#endif

#endif /* !_KOS_I386_KOS_BITS_FUTEX_H */
