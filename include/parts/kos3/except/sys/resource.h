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
#ifndef _PARTS_KOS3_EXCEPT_SYS_RESOURCE_H
#define _PARTS_KOS3_EXCEPT_SYS_RESOURCE_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#ifndef _SYS_GENERIC_RESOURCE_H
#include <sys/resource.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

__REDIRECT_FS64(__LIBC,,void,__LIBCCALL,Xgetrlimit,(__rlimit_resource_t __resource, struct rlimit *__rlimits),(__resource,__rlimits))
__REDIRECT_FS64(__LIBC,,void,__LIBCCALL,Xsetrlimit,(__rlimit_resource_t __resource, struct rlimit const *__rlimits),(__resource,__rlimits))
__LIBC void (__LIBCCALL Xgetrusage)(__rusage_who_t __who, struct rusage *__usage);
__LIBC int (__LIBCCALL Xgetpriority)(__priority_which_t __which, id_t __who);
__LIBC void (__LIBCCALL Xsetpriority)(__priority_which_t __which, id_t __who, int __prio);
#ifdef __USE_LARGEFILE64
__LIBC void (__LIBCCALL Xgetrlimit64)(__rlimit_resource_t __resource, struct rlimit64 *__rlimits);
__LIBC void (__LIBCCALL Xsetrlimit64)(__rlimit_resource_t __resource, struct rlimit64 const *__rlimits);
#endif /* __USE_LARGEFILE64 */


__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SYS_RESOURCE_H */
