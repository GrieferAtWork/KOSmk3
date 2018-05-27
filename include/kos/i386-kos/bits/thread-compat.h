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
#endif /* __CC__ */




#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_SELF        0
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_XCURRENT    __SIZEOF_X86_INTPTRCC__
#ifdef __KERNEL__
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE      (__SIZEOF_X86_INTPTRCC__+__USER_EXCEPTION_INFO_COMPAT_SIZE)
#else
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE      (__SIZEOF_X86_INTPTRCC__+__EXCEPTION_INFO_COMPAT_SIZE)
#endif
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_TYPE       (__USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE+2)
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_EFORMAT    (__USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE+3)
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_ERRNO      (__USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE+4)
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_DOS_ERRNO  (__USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE+8)
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_NO_RPC     (__USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE+12)
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_TID        (__USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE+16)
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_PROCESS    (__USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE+20)
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_UEH        (__USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE+20+__SIZEOF_X86_INTPTRCC__)
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_UEH_SP     (__USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE+20+__SIZEOF_X86_INTPTRCC__*2)
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_X86SYSBASE (__USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE+20+__SIZEOF_X86_INTPTRCC__*3)
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_TLS        (__USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE+20+__SIZEOF_X86_INTPTRCC__*4)
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_LOCKS      (__USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE+20+__SIZEOF_X86_INTPTRCC__*5)
#define __USER_TASK_SEGMENT_COMPAT_OFFSETOF_PTHREAD    (__USER_TASK_SEGMENT_COMPAT_OFFSETOF_STATE+20+__SIZEOF_X86_INTPTRCC__*6)

#ifdef __KERNEL__
#define __TASK_SEGMENT_COMPAT_OFFSETOF_SELF          0
#define __TASK_SEGMENT_COMPAT_OFFSETOF_XCURRENT      __SIZEOF_X86_INTPTRCC__
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_SELF       __USER_TASK_SEGMENT_COMPAT_OFFSETOF_SELF
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_XCURRENT   __USER_TASK_SEGMENT_COMPAT_OFFSETOF_XCURRENT
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_TYPE       __USER_TASK_SEGMENT_COMPAT_OFFSETOF_TYPE
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_EFORMAT    __USER_TASK_SEGMENT_COMPAT_OFFSETOF_EFORMAT
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_ERRNO      __USER_TASK_SEGMENT_COMPAT_OFFSETOF_ERRNO
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_DOS_ERRNO  __USER_TASK_SEGMENT_COMPAT_OFFSETOF_DOS_ERRNO
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_NO_RPC     __USER_TASK_SEGMENT_COMPAT_OFFSETOF_NO_RPC
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_TID        __USER_TASK_SEGMENT_COMPAT_OFFSETOF_TID
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_PROCESS    __USER_TASK_SEGMENT_COMPAT_OFFSETOF_PROCESS
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_UEH        __USER_TASK_SEGMENT_COMPAT_OFFSETOF_UEH
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_UEH_SP     __USER_TASK_SEGMENT_COMPAT_OFFSETOF_UEH_SP
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_X86SYSBASE __USER_TASK_SEGMENT_COMPAT_OFFSETOF_X86SYSBASE
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_TLS        __USER_TASK_SEGMENT_COMPAT_OFFSETOF_TLS
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_LOCKS      __USER_TASK_SEGMENT_COMPAT_OFFSETOF_LOCKS
#define USER_TASK_SEGMENT_COMPAT_OFFSETOF_PTHREAD    __USER_TASK_SEGMENT_COMPAT_OFFSETOF_PTHREAD
#else
#define __TASK_SEGMENT_COMPAT_OFFSETOF_SELF          __USER_TASK_SEGMENT_COMPAT_OFFSETOF_SELF
#define __TASK_SEGMENT_COMPAT_OFFSETOF_XCURRENT      __USER_TASK_SEGMENT_COMPAT_OFFSETOF_XCURRENT
#define __TASK_SEGMENT_COMPAT_OFFSETOF_TYPE          __USER_TASK_SEGMENT_COMPAT_OFFSETOF_TYPE
#define __TASK_SEGMENT_COMPAT_OFFSETOF_EFORMAT       __USER_TASK_SEGMENT_COMPAT_OFFSETOF_EFORMAT
#define __TASK_SEGMENT_COMPAT_OFFSETOF_ERRNO         __USER_TASK_SEGMENT_COMPAT_OFFSETOF_ERRNO
#define __TASK_SEGMENT_COMPAT_OFFSETOF_DOS_ERRNO     __USER_TASK_SEGMENT_COMPAT_OFFSETOF_DOS_ERRNO
#define __TASK_SEGMENT_COMPAT_OFFSETOF_NO_RPC        __USER_TASK_SEGMENT_COMPAT_OFFSETOF_NO_RPC
#define __TASK_SEGMENT_COMPAT_OFFSETOF_TID           __USER_TASK_SEGMENT_COMPAT_OFFSETOF_TID
#define __TASK_SEGMENT_COMPAT_OFFSETOF_PROCESS       __USER_TASK_SEGMENT_COMPAT_OFFSETOF_PROCESS
#define __TASK_SEGMENT_COMPAT_OFFSETOF_UEH           __USER_TASK_SEGMENT_COMPAT_OFFSETOF_UEH
#define __TASK_SEGMENT_COMPAT_OFFSETOF_UEH_SP        __USER_TASK_SEGMENT_COMPAT_OFFSETOF_UEH_SP
#define __TASK_SEGMENT_COMPAT_OFFSETOF_X86SYSBASE    __USER_TASK_SEGMENT_COMPAT_OFFSETOF_X86SYSBASE
#define __TASK_SEGMENT_COMPAT_OFFSETOF_TLS           __USER_TASK_SEGMENT_COMPAT_OFFSETOF_TLS
#define __TASK_SEGMENT_COMPAT_OFFSETOF_LOCKS         __USER_TASK_SEGMENT_COMPAT_OFFSETOF_LOCKS
#define __TASK_SEGMENT_COMPAT_OFFSETOF_PTHREAD       __USER_TASK_SEGMENT_COMPAT_OFFSETOF_PTHREAD
#endif


#ifdef __CC__
#ifdef __KERNEL__
#define __task_segment_compat_defined 1
struct __ATTR_PACKED task_segment_compat {
     __X86_PTRCC(struct task_segment_compat) ts_self;
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
     __X86_PTRCC(struct user_task_segment_compat) ts_self;
     struct user_exception_info_compat ts_xcurrent;
#else
     __X86_PTRCC(struct task_segment_compat) ts_self;
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
