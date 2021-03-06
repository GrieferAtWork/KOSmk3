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
#ifndef _PARTS_GLC_I386_UCONTEXT_H
#define _PARTS_GLC_I386_UCONTEXT_H 1
#define _SYS_UCONTEXT_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/host.h>
#include <bits/sigset.h>
#include <bits/sigcontext.h>
#include <bits/sigstack.h>

/* Copyright (C) 2001-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

__SYSDECL_BEGIN

#ifdef __CC__
#ifndef __greg_t_defined
#define __greg_t_defined 1
typedef __SREGISTER_TYPE__ greg_t;
#endif /* !__greg_t_defined */
#endif /* __CC__ */
#define __SIZEOF_GREG_T__  __SIZEOF_REGISTER__


#ifdef __x86_64__
#define NGREG  23 /* Number of general registers. */

#ifdef __CC__
typedef greg_t gregset_t[NGREG]; /* Container for all general registers. */
#endif /* __CC__ */

#ifdef __USE_GNU
/* Number of each register in the `gregset_t' array. */
#ifdef __CC__
enum {
    REG_R8 = 0,
    REG_R9,
    REG_R10,
    REG_R11,
    REG_R12,
    REG_R13,
    REG_R14,
    REG_R15,
    REG_RDI,
    REG_RSI,
    REG_RBP,
    REG_RBX,
    REG_RDX,
    REG_RAX,
    REG_RCX,
    REG_RSP,
    REG_RIP,
    REG_EFL,
    REG_CSGSFS, /* Actually short cs, gs, fs, __pad0. */
    REG_ERR,
    REG_TRAPNO,
    REG_OLDMASK,
    REG_CR2
};
#endif /* __CC__ */
#define REG_R8      0
#define REG_R9      1
#define REG_R10     2
#define REG_R11     3
#define REG_R12     4
#define REG_R13     5
#define REG_R14     6
#define REG_R15     7
#define REG_RDI     8
#define REG_RSI     9
#define REG_RBP     10
#define REG_RBX     11
#define REG_RDX     12
#define REG_RAX     13
#define REG_RCX     14
#define REG_RSP     15
#define REG_RIP     16
#define REG_EFL     17
#define REG_CSGSFS  18
#define REG_ERR     19
#define REG_TRAPNO  20
#define REG_OLDMASK 21
#define REG_CR2     22
#endif /* __USE_GNU */

#define __LIBC_FPXREG_OFFSETOF_SIGNIFICAND 0
#define __LIBC_FPXREG_OFFSETOF_EXPONENT    8
#define __LIBC_FPXREG_OFFSETOF_PADDING     10
#define __LIBC_FPXREG_SIZE                 16


#ifdef __CC__
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("significand")
#pragma push_macro("exponent")
#pragma push_macro("padding")
#endif
#undef significand
#undef exponent
#undef padding
struct _libc_fpxreg {
    __UINT16_TYPE__ significand[4];
    __UINT16_TYPE__ exponent;
    __UINT16_TYPE__ padding[3];
};
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("padding")
#pragma pop_macro("exponent")
#pragma pop_macro("significand")
#endif
#endif /* __CC__ */

#define __LIBC_XMMREG_OFFSETOF_ELEMENT     0
#define __LIBC_XMMREG_SIZE                 16
#ifdef __CC__
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("element")
#endif
#undef element
struct _libc_xmmreg {
    __uint32_t element[4];
};
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("element")
#endif
#endif /* __CC__ */

#define __LIBC_FPSTATE_OFFSETOF_CWD        0
#define __LIBC_FPSTATE_OFFSETOF_SWD        2
#define __LIBC_FPSTATE_OFFSETOF_FTW        4
#define __LIBC_FPSTATE_OFFSETOF_FOP        6
#define __LIBC_FPSTATE_OFFSETOF_RIP        8
#define __LIBC_FPSTATE_OFFSETOF_RDP        16
#define __LIBC_FPSTATE_OFFSETOF_MXCSR      24
#define __LIBC_FPSTATE_OFFSETOF_MXCR_MASK  28
#define __LIBC_FPSTATE_OFFSETOF_ST         32
#define __LIBC_FPSTATE_OFFSETOF_XMM        160
#define __LIBC_FPSTATE_OFFSETOF_PADDING    416
#define __LIBC_FPSTATE_SIZE                512

#ifdef __CC__
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("cwd")
#pragma push_macro("swd")
#pragma push_macro("ftw")
#pragma push_macro("fop")
#pragma push_macro("rip")
#pragma push_macro("rdp")
#pragma push_macro("mxcsr")
#pragma push_macro("mxcr_mask")
#pragma push_macro("_st")
#pragma push_macro("_xmm")
#pragma push_macro("padding")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef cwd
#undef swd
#undef ftw
#undef fop
#undef rip
#undef rdp
#undef mxcsr
#undef mxcr_mask
#undef _st
#undef _xmm
#undef padding
struct _libc_fpstate {
    /* 64-bit FXSAVE format. */
    __uint16_t          cwd;
    __uint16_t          swd;
    __uint16_t          ftw;
    __uint16_t          fop;
    __uint64_t          rip;
    __uint64_t          rdp;
    __uint32_t          mxcsr;
    __uint32_t          mxcr_mask;
    struct _libc_fpxreg _st[8];
    struct _libc_xmmreg _xmm[16];
    __uint32_t          padding[24];
};
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("padding")
#pragma pop_macro("_xmm")
#pragma pop_macro("_st")
#pragma pop_macro("mxcr_mask")
#pragma pop_macro("mxcsr")
#pragma pop_macro("rdp")
#pragma pop_macro("rip")
#pragma pop_macro("fop")
#pragma pop_macro("ftw")
#pragma pop_macro("swd")
#pragma pop_macro("cwd")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

/* Structure to describe FPU registers. */
typedef struct _libc_fpstate *fpregset_t;
#endif


#define __MCONTEXT_OFFSETOF_GREGS      0
#define __MCONTEXT_OFFSETOF_FPREGS    (__SIZEOF_GREG_T__*NGREG)
#if defined(__USE_KOS) || defined(__KERNEL__)
#define __MCONTEXT_OFFSETOF_FS_BASE   (__SIZEOF_GREG_T__*NGREG+__SIZEOF_POINTER__)
#define __MCONTEXT_OFFSETOF_GS_BASE   (__SIZEOF_GREG_T__*NGREG+2*__SIZEOF_POINTER__)
#endif
#define __MCONTEXT_SIZE               (__SIZEOF_GREG_T__*NGREG+9*__SIZEOF_POINTER__)
#ifdef __CC__
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("gregs")
#pragma push_macro("fpregs")
#if defined(__USE_KOS) || defined(__KERNEL__)
#pragma push_macro("fs_base")
#pragma push_macro("gs_base")
#endif
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef gregs
#undef fpregs
#if defined(__USE_KOS) || defined(__KERNEL__)
#undef fs_base
#undef gs_base
#endif
/* Context to describe whole processor state. */
typedef struct {
    gregset_t        gregs;
    fpregset_t       fpregs;  /* Note that fpregs is a pointer. */
#if defined(__USE_KOS) || defined(__KERNEL__)
    __UINTPTR_TYPE__ fs_base; /* User-space `fs_base' address. */
    __UINTPTR_TYPE__ gs_base; /* User-space `gs_base' address. */
    __UINTPTR_TYPE__ __reserved1[6];
#else
    __UINTPTR_TYPE__ __reserved1[8];
#endif
} mcontext_t;
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#if defined(__USE_KOS) || defined(__KERNEL__)
#pragma pop_macro("gs_base")
#pragma pop_macro("fs_base")
#endif
#pragma pop_macro("fpregs")
#pragma pop_macro("gregs")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#endif


#define __UCONTEXT_OFFSETOF_FLAGS        0
#define __UCONTEXT_OFFSETOF_LINK         __SIZEOF_POINTER__
#define __UCONTEXT_OFFSETOF_STACK     (2*__SIZEOF_POINTER__)
#define __UCONTEXT_OFFSETOF_MCONTEXT  (2*__SIZEOF_POINTER__+__STACK_SIZE)
#define __UCONTEXT_OFFSETOF_SIGMASK   (2*__SIZEOF_POINTER__+__STACK_SIZE+__MCONTEXT_SIZE)
#define __UCONTEXT_OFFSETOF_FPREGS    (2*__SIZEOF_POINTER__+__STACK_SIZE+__MCONTEXT_SIZE+__SIZEOF_SIGSET_T__)
#define __UCONTEXT_SIZE               (2*__SIZEOF_POINTER__+__STACK_SIZE+__MCONTEXT_SIZE+__SIZEOF_SIGSET_T__+__LIBC_FPSTATE_SIZE)

#ifdef __CC__
/* Userlevel context. */
typedef struct ucontext {
    __ULONGPTR_TYPE__    uc_flags;
    struct ucontext     *uc_link;
    stack_t              uc_stack;
    mcontext_t           uc_mcontext;
    __sigset_t           uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
} ucontext_t;
#endif /* __CC__ */

#else /* __x86_64__ */

#define NGREG    19 /* Number of general registers. */
#ifdef __CC__
typedef greg_t gregset_t[NGREG]; /* Container for all general registers. */
#endif

#ifdef __USE_GNU
#ifdef __COMPILER_PREFERR_ENUMS
/* Number of each register is the `gregset_t' array. */
enum {
    REG_GS     = 0,
    REG_FS     = 1,
    REG_ES     = 2,
    REG_DS     = 3,
    REG_EDI    = 4,
    REG_ESI    = 5,
    REG_EBP    = 6,
    REG_ESP    = 7,
    REG_EBX    = 8,
    REG_EDX    = 9,
    REG_ECX    = 10,
    REG_EAX    = 11,
    REG_TRAPNO = 12,
    REG_ERR    = 13,
    REG_EIP    = 14,
    REG_CS     = 15,
    REG_EFL    = 16,
    REG_UESP   = 17,
    REG_SS     = 18
};
#define REG_GS     REG_GS
#define REG_FS     REG_FS
#define REG_ES     REG_ES
#define REG_DS     REG_DS
#define REG_EDI    REG_EDI
#define REG_ESI    REG_ESI
#define REG_EBP    REG_EBP
#define REG_ESP    REG_ESP
#define REG_EBX    REG_EBX
#define REG_EDX    REG_EDX
#define REG_ECX    REG_ECX
#define REG_EAX    REG_EAX
#define REG_TRAPNO REG_TRAPNO
#define REG_ERR    REG_ERR
#define REG_EIP    REG_EIP
#define REG_CS     REG_CS
#define REG_EFL    REG_EFL
#define REG_UESP   REG_UESP
#define REG_SS     REG_SS
#else /* __COMPILER_PREFERR_ENUMS */
/* Number of each register is the `gregset_t' array. */
#define REG_GS     0
#define REG_FS     1
#define REG_ES     2
#define REG_DS     3
#define REG_EDI    4
#define REG_ESI    5
#define REG_EBP    6
#define REG_ESP    7
#define REG_EBX    8
#define REG_EDX    9
#define REG_ECX    10
#define REG_EAX    11
#define REG_TRAPNO 12
#define REG_ERR    13
#define REG_EIP    14
#define REG_CS     15
#define REG_EFL    16
#define REG_UESP   17
#define REG_SS     18
#endif /* !__COMPILER_PREFERR_ENUMS */
#endif

#define __LIBC_FPREG_OFFSETOF_SIGNIFICAND 0
#define __LIBC_FPREG_OFFSETOF_EXPONENT    8
#define __LIBC_FPREG_SIZE                 10

#ifdef __CC__
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("significand")
#pragma push_macro("exponent")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef significand
#undef exponent
struct _libc_fpreg {
    /* Definitions taken from the kernel headers. */
    unsigned short int significand[4];
    unsigned short int exponent;
};
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("exponent")
#pragma pop_macro("significand")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#endif /* __CC__ */

#define __LIBC_FPSTATE_OFFSETOF_CW       0
#define __LIBC_FPSTATE_OFFSETOF_SW       4
#define __LIBC_FPSTATE_OFFSETOF_TAG      8
#define __LIBC_FPSTATE_OFFSETOF_IPOFF    12
#define __LIBC_FPSTATE_OFFSETOF_CSSEL    16
#define __LIBC_FPSTATE_OFFSETOF_DATAOFF  20
#define __LIBC_FPSTATE_OFFSETOF_DATASEL  24
#define __LIBC_FPSTATE_OFFSETOF_ST       28
#define __LIBC_FPSTATE_OFFSETOF_STATUS   108
#define __LIBC_FPSTATE_SIZE              112

#ifdef __CC__
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("cw")
#pragma push_macro("sw")
#pragma push_macro("tag")
#pragma push_macro("ipoff")
#pragma push_macro("cssel")
#pragma push_macro("dataoff")
#pragma push_macro("datasel")
#pragma push_macro("_st")
#pragma push_macro("status")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef cw
#undef sw
#undef tag
#undef ipoff
#undef cssel
#undef dataoff
#undef datasel
#undef _st
#undef status
struct _libc_fpstate {
    __ULONGPTR_TYPE__  cw;
    __ULONGPTR_TYPE__  sw;
    __ULONGPTR_TYPE__  tag;
    __ULONGPTR_TYPE__  ipoff;
    __ULONGPTR_TYPE__  cssel;
    __ULONGPTR_TYPE__  dataoff;
    __ULONGPTR_TYPE__  datasel;
    struct _libc_fpreg _st[8];
    __ULONGPTR_TYPE__  status;
};
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("status")
#pragma pop_macro("_st")
#pragma pop_macro("datasel")
#pragma pop_macro("dataoff")
#pragma pop_macro("cssel")
#pragma pop_macro("ipoff")
#pragma pop_macro("tag")
#pragma pop_macro("sw")
#pragma pop_macro("cw")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

/* Structure to describe FPU registers. */
typedef struct _libc_fpstate *fpregset_t;
#endif /* __CC__ */


#define __MCONTEXT_OFFSETOF_GREGS      0
#define __MCONTEXT_OFFSETOF_FPREGS    (__SIZEOF_GREG_T__*NGREG)
#define __MCONTEXT_OFFSETOF_OLDMASK   (__SIZEOF_GREG_T__*NGREG+4)
#define __MCONTEXT_OFFSETOF_CR2       (__SIZEOF_GREG_T__*NGREG+8)
#define __MCONTEXT_SIZE               (__SIZEOF_GREG_T__*NGREG+12)
#ifdef __CC__
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("gregs")
#pragma push_macro("fpregs")
#pragma push_macro("oldmask")
#pragma push_macro("cr2")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef gregs
#undef fpregs
#undef oldmask
#undef cr2
typedef struct {
    /* Context to describe whole processor state. */
    gregset_t gregs;
    /* Due to Linux's history we have to use a pointer here.
     * The SysV/i386 ABI requires a struct with the values. */
    fpregset_t fpregs;
    __ULONGPTR_TYPE__ oldmask;
    __ULONGPTR_TYPE__ cr2;
} mcontext_t;
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("cr2")
#pragma pop_macro("oldmask")
#pragma pop_macro("fpregs")
#pragma pop_macro("gregs")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#endif /* __CC__ */

#define __UCONTEXT_OFFSETOF_FLAGS        0
#define __UCONTEXT_OFFSETOF_LINK         __SIZEOF_POINTER__
#define __UCONTEXT_OFFSETOF_STACK     (2*__SIZEOF_POINTER__)
#define __UCONTEXT_OFFSETOF_MCONTEXT  (2*__SIZEOF_POINTER__+__STACK_SIZE)
#define __UCONTEXT_OFFSETOF_SIGMASK   (2*__SIZEOF_POINTER__+__STACK_SIZE+__MCONTEXT_SIZE)
#define __UCONTEXT_OFFSETOF_FPREGS    (2*__SIZEOF_POINTER__+__STACK_SIZE+__MCONTEXT_SIZE+__SIZEOF_SIGSET_T__)
#define __UCONTEXT_SIZE               (2*__SIZEOF_POINTER__+__STACK_SIZE+__MCONTEXT_SIZE+__SIZEOF_SIGSET_T__+__LIBC_FPSTATE_SIZE)

#ifdef __CC__
/* Userlevel context. */
typedef struct ucontext {
    __ULONGPTR_TYPE__    uc_flags;
    struct ucontext     *uc_link;
    stack_t              uc_stack;
    mcontext_t           uc_mcontext;
    __sigset_t           uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
} ucontext_t;
#endif /* __CC__ */
#endif /* !__x86_64__ */

__SYSDECL_END

#endif /* !_PARTS_GLC_I386_UCONTEXT_H */
