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
#ifndef _KOS_I386_KOS_ASM_TLS_H
#define _KOS_I386_KOS_ASM_TLS_H 1

#include <__stdinc.h>
#include <hybrid/host.h>

__SYSDECL_BEGIN

#ifndef __ASM_TASK_SEGMENT
#ifdef __x86_64__
#    define __ASM_HOSTTASK_SEGMENT        gs
#    define __ASM_USERTASK_SEGMENT        fs
#    define __ASM_HOSTTASK_SEGMENT_COMPAT fs
#    define __ASM_USERTASK_SEGMENT_COMPAT gs
#else
#    define __ASM_HOSTTASK_SEGMENT        fs
#    define __ASM_USERTASK_SEGMENT        gs
#    define __ASM_HOSTTASK_SEGMENT_COMPAT gs
#    define __ASM_USERTASK_SEGMENT_COMPAT fs
#endif
#ifdef __KERNEL__
#ifdef __x86_64__
#    define __ASM_TASK_SEGMENT_ISGS 1
#    define __ASM_TASK_SEGMENT_COMPAT_ISFS 1
#else
#    define __ASM_TASK_SEGMENT_ISFS 1
#    define __ASM_TASK_SEGMENT_COMPAT_ISGS 1
#endif
#    define __ASM_TASK_SEGMENT        __ASM_HOSTTASK_SEGMENT
#    define __ASM_TASK_SEGMENT_COMPAT __ASM_HOSTTASK_SEGMENT_COMPAT
#else
#ifdef __x86_64__
#    define __ASM_TASK_SEGMENT_ISFS 1
#    define __ASM_TASK_SEGMENT_COMPAT_ISGS 1
#else
#    define __ASM_TASK_SEGMENT_ISGS 1
#    define __ASM_TASK_SEGMENT_COMPAT_ISFS 1
#endif
#    define __ASM_TASK_SEGMENT        __ASM_USERTASK_SEGMENT
#    define __ASM_TASK_SEGMENT_COMPAT __ASM_USERTASK_SEGMENT_COMPAT
#endif
#endif /* !__ASM_TASK_SEGMENT */

#ifndef CONFIG_NO_DOS_COMPAT
#ifdef __x86_64__
#    define __ASM_TIBTASK_SEGMENT             gs
#    define __ASM_TIBTASK_SEGMENT_COMPAT      fs
#    define __ASM_TIBTASK_SEGMENT_ISGS        1
#    define __ASM_TIBTASK_SEGMENT_COMPAT_ISFS 1
#else
#    define __ASM_TIBTASK_SEGMENT             fs
#    define __ASM_TIBTASK_SEGMENT_COMPAT      gs
#    define __ASM_TIBTASK_SEGMENT_ISFS        1
#    define __ASM_TIBTASK_SEGMENT_COMPAT_ISGS 1
#endif
#endif /* !CONFIG_NO_DOS_COMPAT */

#ifdef __ASSEMBLER__
#    define taskseg   __ASM_TASK_SEGMENT
#endif

__SYSDECL_END

#endif /* !_KOS_I386_KOS_ASM_TLS_H */
