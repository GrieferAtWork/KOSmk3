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
#ifndef _KOS_I386_KOS_BITS_THREAD_COMPAT_H
#define _KOS_I386_KOS_BITS_THREAD_COMPAT_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <kos/i386-kos/bits/compat.h>
#include <kos/i386-kos/bits/except-compat.h>
#if defined(__KERNEL__) || \
    defined(__BUILDING_LIBC) || \
    defined(__BUILDING_LIBPTHREAD)
#include <kos/i386-kos/bits/tib-compat.h>
#endif

__SYSDECL_BEGIN

#ifdef __EXPOSE_CPU_COMPAT
#ifdef __CC__

#ifdef __x86_64__
#define __task_segment32_defined 1
#define __task_segment64_defined 1
#define   task_segment32 task_segment_compat
#define   task_segment64 task_segment
#ifdef __KERNEL__
#define __user_task_segment32_defined 1
#define __user_task_segment64_defined 1
#define   user_task_segment32 user_task_segment_compat
#define   user_task_segment64 user_task_segment
#endif
#else /* __x86_64__ */
#define __task_segment32_defined 1
#define __task_segment64_defined 1
#define   task_segment32 task_segment
#define   task_segment64 task_segment_compat
#ifdef __KERNEL__
#define __user_task_segment32_defined 1
#define __user_task_segment64_defined 1
#define   user_task_segment32 user_task_segment
#define   user_task_segment64 user_task_segment_compat
#endif
#endif /* !__x86_64__ */



#ifdef __KERNEL__
#define __task_segment_compat_defined 1
struct __ATTR_PACKED task_segment_compat {
     struct task_segment_compat  *ts_self;
     struct exception_info_compat ts_xcurrent;
};
#endif
struct process_env_compat;
#ifdef __KERNEL__
#define __user_task_segment_compat_defined 1
struct __ATTR_PACKED user_task_segment_compat
#else
#define __task_segment_compat_defined      1
struct __ATTR_PACKED task_segment_compat
#endif
{
#ifdef __KERNEL__
     struct user_task_segment_compat  *ts_self;
     struct user_exception_info_compat ts_xcurrent;
#else
     struct task_segment_compat       *ts_self;
     struct exception_info_compat      ts_xcurrent;
#endif
     __UINT16_TYPE__            ts_state;
     __UINT8_TYPE__             ts_type;
     __UINT8_TYPE__             ts_eformat;
     __UINT32_TYPE__            ts_errno;
     __UINT32_TYPE__            ts_dos_errno;
     __UINT32_TYPE__            ts_no_rpc;
     __pid_t                    ts_tid;
     __X86_PTRCC(struct process_environ)
                                ts_process;
     __X86_PTRCC(void(*)(void)) ts_ueh;
     __X86_PTRCC(void)          ts_ueh_sp;
     __X86_INTPTRCC             ts_x86sysbase;
     __X86_INTPTRCC           __ts_tls;
     __X86_INTPTRCC           __ts_locks;
     __X86_INTPTRCC           __ts_pthread;
#if defined(__KERNEL__) || \
    defined(__BUILDING_LIBC) || \
    defined(__BUILDING_LIBPTHREAD)
#ifndef CONFIG_NO_DOS_COMPAT
     struct nt_tib_compat       ts_tib;
#endif /* !CONFIG_NO_DOS_COMPAT */
#endif
};
#endif /* __CC__ */

#endif /* __EXPOSE_CPU_COMPAT */

__SYSDECL_END

#endif /* !_KOS_I386_KOS_BITS_THREAD_COMPAT_H */
