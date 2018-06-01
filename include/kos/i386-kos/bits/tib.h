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
#ifndef _KOS_I386_KOS_BITS_TIB_H
#define _KOS_I386_KOS_BITS_TIB_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>
#include "compat.h"


__SYSDECL_BEGIN

#ifdef __CC__
#ifndef CONFIG_NO_DOS_COMPAT

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("nt_errno")
#endif
#undef nt_errno

struct _EXCEPTION_REGISTRATION;
/* NT-compatible Thread Information Block:
 *   https://en.wikipedia.org/wiki/Win32_Thread_Information_Block */
#define __nt_tib_defined 1
struct __ATTR_PACKED nt_tib {
    struct _EXCEPTION_REGISTRATION *nt_seh;              /* [0..1] Current Structured Exception Handling (SEH) frame */
    void                           *nt_stack_top;        /* [0..1] One past the greatest address associated with the stack. */
    void                           *nt_stack_min;        /* [0..1] Lowest address associated with the stack. */
    void                           *nt_subsystem;        /* [???] SubSystemTib */
    void                           *nt_fiberdata;        /* [???] Fiber data */
    void                           *nt_userdata;         /* [?..?] Arbitrary data slot */
    struct nt_tib                  *nt_teb;              /* [1..1] Linear address of TEB/TIB (self pointer) */
    void                           *nt_environ;          /* [???] Environment Pointer */
    __UINT32_TYPE__                 nt_pid;              /* Process ID */
    __UINT32_TYPE__                 nt_tid;              /* Thread ID (Mirror of `%taskreg:ts_tid') */
    void                           *nt_curr_rpc;         /* [???] Active RPC Handle */
    void                          **nt_tls;              /* [== nt_tlsdata] Linear address of the thread-local storage array */
    void                           *nt_peb;              /* [???] Process environment block (Currently not emulated; KOS uses `struct process_environ') */
    __UINT32_TYPE__                 nt_errno;            /* GetLastError()-style errno. */
    __UINT32_TYPE__                 nt_numcrit;          /* [???] Count of owned critical sections. */
    void                           *nt_csr;              /* [???] Address of CSR Client Thread */
    void                           *nt_info;             /* [???] Win32 Thread Information */
    __BYTE_TYPE__                   nt_client[124];      /* [???] Win32 client information */
    __UINT32_TYPE__                 nt_wow64_syscall;    /* [???] Contains a pointer to FastSysCall in Wow64 */
    __UINT32_TYPE__                 nt_curr_locale;      /* [???] Current Locale */
    __UINT32_TYPE__                 nt_fpsoftstatus;     /* [???] FP Software Status Register */
    __BYTE_TYPE__                   nt_osdata[216];      /* [???] Reserved for OS */
    __UINT32_TYPE__                 nt_except_code;      /* [???] Exception code */
    __BYTE_TYPE__                   nt_actcxt_stack[12]; /* [???] Activation context stack */
    __BYTE_TYPE__                   nt_spare[24];        /* [???] Spare bytes */
    __BYTE_TYPE__                   nt_osdata2[40];      /* [???] Reserved for OS */
    struct __ATTR_PACKED {
        __BYTE_TYPE__               g_teb_batch[1248];   /* [???] GDI TEB Batch */
        __UINT32_TYPE__             g_region;            /* [???] GDI Region */
        __UINT32_TYPE__             g_pen;               /* [???] GDI Pen */
        __UINT32_TYPE__             g_brush;             /* [???] GDI Brush */
        __UINT32_TYPE__             g_real_pid;          /* [???] Real Process ID */
        __UINT32_TYPE__             g_real_tid;          /* [???] Real Thread ID */
        __UINT32_TYPE__             g_process_handle;    /* [???] GDI cached process handle */
        __UINT32_TYPE__             g_client_pid;        /* [???] GDI client process ID */
        __UINT32_TYPE__             g_client_tid;        /* [???] GDI client thread ID */
        __UINT32_TYPE__             g_tls;               /* [???] GDI thread locale information */
    }                               nt_gdi;              /* GDI-related data. */
    __BYTE_TYPE__                   nt_userdata2[20];    /* [???] Reserved for user application */
    __BYTE_TYPE__                   nt_gl[1248];         /* [???] Reserved for GL */
    __UINT32_TYPE__                 nt_laststatus;       /* [???] Last Status Value */
    __BYTE_TYPE__                   nt_unibuf[532];      /* [???] Static UNICODE_STRING buffer */
    void                           *nt_stackmem;         /* [???] Address of memory allocated for stack */
    void                           *nt_tlsdata[64];      /* TLS slots, 4/8 bytes per slot, 64 slots */
    void                           *nt_tlslink_f;        /* [???] TLS links (LIST_ENTRY structure) */
    void                           *nt_tlslink_b;        /* [???] TLS links (LIST_ENTRY structure) */
    void                           *nt_vdm;              /* [???] VDM */
    void                           *nt_rpc;              /* [???] Reserved for RPC */
    __UINTPTR_TYPE__                nt_error_mode;       /* [???] Thread error mode */

};


#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("nt_errno")
#endif

#endif /* !CONFIG_NO_DOS_COMPAT */
#endif

__SYSDECL_END

#ifdef __EXPOSE_CPU_COMPAT
#include "tib-compat.h"
#endif

#endif /* !_KOS_I386_KOS_BITS_TIB_H */
