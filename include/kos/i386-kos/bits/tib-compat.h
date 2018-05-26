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
#ifndef _KOS_I386_KOS_BITS_TIB_COMPAT_H
#define _KOS_I386_KOS_BITS_TIB_COMPAT_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>
#include "compat.h"

__SYSDECL_BEGIN

#ifdef __CC__
#ifndef CONFIG_NO_DOS_COMPAT

#ifdef __x86_64__
#define _EXCEPTION_REGISTRATION64 _EXCEPTION_REGISTRATION
#define _EXCEPTION_REGISTRATION32 _EXCEPTION_REGISTRATION_COMPAT
#define __nt_tib32_defined 1
#define __nt_tib64_defined 1
#define   nt_tib32 nt_tib_compat
#define   nt_tib64 nt_tib
#else
#define _EXCEPTION_REGISTRATION64 _EXCEPTION_REGISTRATION_COMPAT
#define _EXCEPTION_REGISTRATION32 _EXCEPTION_REGISTRATION
#define __nt_tib32_defined 1
#define __nt_tib64_defined 1
#define   nt_tib32 nt_tib
#define   nt_tib64 nt_tib_compat
#endif


#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("nt_errno")
#endif
#undef nt_errno

struct _EXCEPTION_REGISTRATION_COMPAT;
/* NT-compatible Thread Information Block:
 *   https://en.wikipedia.org/wiki/Win32_Thread_Information_Block */
#define __nt_tib_compat_defined 1
struct __ATTR_PACKED nt_tib_compat {
    __X86_PTRCC(struct _EXCEPTION_REGISTRATION_COMPAT) nt_seh;
    __X86_PTRCC(void)               nt_stack_top;
    __X86_PTRCC(void)               nt_stack_min;
    __X86_PTRCC(void)               nt_subsystem;
    __X86_PTRCC(void)               nt_fiberdata;
    __X86_PTRCC(void)               nt_userdata;
    __X86_PTRCC(struct nt_tib_compat) nt_teb;
    __X86_PTRCC(void)               nt_environ;
    __UINT32_TYPE__                 nt_pid;
    __UINT32_TYPE__                 nt_tid;
    __X86_PTRCC(void)               nt_curr_rpc;
    __X86_PTRCC(__X86_PTRCC(void))  nt_tls;
    __X86_PTRCC(void)               nt_peb;
    __UINT32_TYPE__                 nt_errno;
    __UINT32_TYPE__                 nt_numcrit;
    __X86_PTRCC(void)               nt_csr;
    __X86_PTRCC(void)               nt_info;
    __BYTE_TYPE__                   nt_client[124];
    __UINT32_TYPE__                 nt_wow64_syscall;
    __UINT32_TYPE__                 nt_curr_locale;
    __UINT32_TYPE__                 nt_fpsoftstatus;
    __BYTE_TYPE__                   nt_osdata[216];
    __UINT32_TYPE__                 nt_except_code;
    __BYTE_TYPE__                   nt_actcxt_stack[12];
    __BYTE_TYPE__                   nt_spare[24];
    __BYTE_TYPE__                   nt_osdata2[40];
    struct __ATTR_PACKED {
        __BYTE_TYPE__               g_teb_batch[1248];
        __UINT32_TYPE__             g_region;
        __UINT32_TYPE__             g_pen;
        __UINT32_TYPE__             g_brush;
        __UINT32_TYPE__             g_real_pid;
        __UINT32_TYPE__             g_real_tid;
        __UINT32_TYPE__             g_process_handle;
        __UINT32_TYPE__             g_client_pid;
        __UINT32_TYPE__             g_client_tid;
        __UINT32_TYPE__             g_tls;
    }                               nt_gdi;
    __BYTE_TYPE__                   nt_userdata2[20];
    __BYTE_TYPE__                   nt_gl[1248];
    __UINT32_TYPE__                 nt_laststatus;
    __BYTE_TYPE__                   nt_unibuf[532];
    __X86_PTRCC(void)               nt_stackmem;
    __X86_PTRCC(void)               nt_tlsdata[64];
    __X86_PTRCC(void)               nt_tlslink_f;
    __X86_PTRCC(void)               nt_tlslink_b;
    __X86_PTRCC(void)               nt_vdm;
    __X86_PTRCC(void)               nt_rpc;
    __UINTPTR_TYPE__                nt_error_mode;

};


#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("nt_errno")
#endif

#endif /* !CONFIG_NO_DOS_COMPAT */
#endif

__SYSDECL_END

#endif /* !_KOS_I386_KOS_BITS_TIB_COMPAT_H */
