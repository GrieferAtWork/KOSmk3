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
#ifndef _KOS_I386_KOS_TIB_H
#define _KOS_I386_KOS_TIB_H 1

#include <__stdinc.h>
#include <hybrid/host.h>

__SYSDECL_BEGIN

#ifdef __CC__
#ifndef CONFIG_NO_DOS_COMPAT
#ifdef CONFIG_NO_X86_SEGMENTATION
#error "CONFIGURATION ERROR: `CONFIG_NO_X86_SEGMENTATION' cannot be used without also enabling `CONFIG_NO_DOS_COMPAT'"
#endif

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("nt_errno")
#endif
#undef nt_errno

struct _EXCEPTION_REGISTRATION;
/* NT-compatible Thread Information Block:
 *   https://en.wikipedia.org/wiki/Win32_Thread_Information_Block */
struct nt_tib {
    struct _EXCEPTION_REGISTRATION *nt_seh;       /* [0..1] Current Structured Exception Handling (SEH) frame */
    void                           *nt_stack_top; /* [0..1] One past the greatest address associated with the stack. */
    void                           *nt_stack_min; /* [0..1] Lowest address associated with the stack. */
    void                           *nt_subsystem; /* [???] SubSystemTib */
    void                           *nt_fiberdata; /* [???] Fiber data */
    void                           *nt_userdata;  /* [?..?] Arbitrary data slot */
    struct nt_tib                  *nt_teb;       /* [1..1] Linear address of TEB/TIB (self pointer) */
    void                           *nt_environ;   /* [???] Environment Pointer */
    __UINTPTR_TYPE__                nt_pid;       /* Process ID */
    __UINTPTR_TYPE__                nt_tid;       /* Thread ID (Mirror of `%taskreg:ts_tid') */
    void                           *nt_curr_rpc;  /* [???] Active RPC Handle */
    void                           *nt_tls;       /* [???] Linear address of the thread-local storage array */
    void                           *nt_peb;       /* [???] Process environment block (Currently not emulated; KOS uses `struct process_environ') */
    __UINT32_TYPE__                 nt_errno;     /* GetLastError()-style errno. */
};


#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("nt_errno")
#endif

#endif /* !CONFIG_NO_DOS_COMPAT */
#endif


__SYSDECL_END

#endif /* !_KOS_I386_KOS_TIB_H */
