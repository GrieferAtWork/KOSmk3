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
#ifndef _PARTS_KOS3_SCHED_H
#define _PARTS_KOS3_SCHED_H 1

#include "__stdinc.h"
#include <features.h>
#include <bits/types.h>
#include <hybrid/typecore.h>

#if defined(__CC__) && !defined(__KERNEL__)
__SYSDECL_BEGIN

struct cpu_context;

/* Create a new thread using `CONTEXT' as initial CPU state when execution starts.
 * NOTE: You may set the stack-pointer value of `CONTEXT' to `CLONE_CHILDSTACK_AUTO'
 *       in order to have the kernel allocate an automatic stack for the new thread.
 * @param: FLAGS: Set of `CLONE_*' from <bits/sched.h>
 */
__REDIRECT_EXCEPT(__LIBC,,__pid_t,__LIBCCALL,xclone,
                 (struct cpu_context *__context, int __flags,
                  __pid_t *__parent_tidptr, void *__tls_val, __pid_t *__child_tidptr),
                 (__context,__flags,__parent_tidptr,__tls_val,__child_tidptr))
#ifdef __USE_EXCEPT
__LIBC __pid_t (__LIBCCALL Xxclone)(struct cpu_context *__context, int __flags,
                                    __pid_t *__parent_tidptr, void *__tls_val,
                                    __pid_t *__child_tidptr);
#endif /* __USE_EXCEPT */

__SYSDECL_END
#endif /* __CC__ && !__KERNEL__ */

#endif /* !_PARTS_KOS3_SCHED_H */
