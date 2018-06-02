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
#ifndef _KOS_I386_KOS_BITS_FUTEX_COMPAT_H
#define _KOS_I386_KOS_BITS_FUTEX_COMPAT_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include "compat.h"

#if defined(__EXPOSE_CPU_COMPAT) && defined(__CC__)
__SYSDECL_BEGIN

#ifdef __x86_64__
#define pollfutex32     pollfutex_compat
#define pollfutex64     pollfutex
#define pollpid32       pollpid_compat
#define pollpid64       pollpid
#define __os_pollinfo32 __os_pollinfo_compat
#define __os_pollinfo64 __os_pollinfo
#else
#define pollfutex32     pollfutex
#define pollfutex64     pollfutex_compat
#define pollpid32       pollpid
#define pollpid64       pollpid_compat
#define __os_pollinfo32 __os_pollinfo
#define __os_pollinfo64 __os_pollinfo_compat
#endif

struct pollfutex_compat {
    __X86_PTRCC(__UINT32_TYPE__)      pf_futex;
    __uint8_t                         pf_action;
    __uint8_t                         pf_status;
    __uint8_t                       __pf_pad[sizeof(__X86_PTRCC(void))-2];
    union __ATTR_PACKED {
        __X86_INTPTRCC                pf_val;
        __X86_INTPTRCC                pf_result;
    };
    union __ATTR_PACKED {
        __X86_INTPTRCC                pf_val2;
        __X86_PTRCC(__UINT32_TYPE__) *pf_uaddr2;
    };
    __X86_INTPTRCC                    pf_val3;
};

struct pollpid_compat {
    __pid_t                    pp_pid;
    __pid_t                    pp_result;
    __uint16_t                 pp_which;
    __uint16_t                 pp_options;
#if __SIZEOF_X86_INTPTRCC__ > 4
    __uint32_t               __pp_pad;
#endif
    __X86_PTRCC(struct __siginfo_compat_struct)
                               pp_info;
    __X86_PTRCC(struct rusage) pp_ru;
};

struct __os_pollinfo_compat {
    __X86_PTRCC(struct pollfd)           i_ufdvec;
    __X86_INTPTRCC                       i_ufdcnt;
    __X86_PTRCC(struct pollfutex_compat) i_ftxvec;
    __X86_INTPTRCC                       i_ftxcnt;
    __X86_PTRCC(struct pollpid_compat)   i_pidvec;
    __X86_INTPTRCC                       i_pidcnt;
};

__SYSDECL_END
#endif

#endif /* !_KOS_I386_KOS_BITS_FUTEX_COMPAT_H */
