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
#include <kos/context.h>

__SYSDECL_BEGIN

#ifdef __CC__
#ifndef __greg_t_defined
#define __greg_t_defined 1
typedef __REGISTER_TYPE__ greg_t;
#endif /* !__greg_t_defined */
#endif /* __CC__ */
#define __SIZEOF_GREG_T__  __SIZEOF_REGISTER__


#define __UCONTEXT_REGNO_EDI    0
#define __UCONTEXT_REGNO_ESI    1
#define __UCONTEXT_REGNO_EBP    2
#define __UCONTEXT_REGNO_ESP    3
#define __UCONTEXT_REGNO_EBX    4
#define __UCONTEXT_REGNO_EDX    5
#define __UCONTEXT_REGNO_ECX    6
#define __UCONTEXT_REGNO_EAX    7
#define __UCONTEXT_REGNO_GS     8
#define __UCONTEXT_REGNO_FS     9
#ifdef CONFIG_X86_FIXED_SEGMENTATION
#define __UCONTEXT_REGNO_EIP    10
#define __UCONTEXT_REGNO_EFL    11
#define NGREG                   12 /* Number of general registers. */
#else /* CONFIG_X86_FIXED_SEGMENTATION */
#define __UCONTEXT_REGNO_ES     10
#define __UCONTEXT_REGNO_DS     11
#define __UCONTEXT_REGNO_EIP    12
#define __UCONTEXT_REGNO_EFL    13
#define __UCONTEXT_REGNO_CS     14
#define __UCONTEXT_REGNO_SS     15
#define NGREG                   16 /* Number of general registers. */
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */

#ifdef __CC__
typedef greg_t gregset_t[NGREG]; /* Container for all general registers. */
#endif

#ifdef __USE_GNU
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
#define REG_UESP   __UCONTEXT_REGNO_ESP
#define REG_SS     18
#endif

#define __MCONTEXT_FNORMAL    0x0000 /* Normal context flags. */
#define __MCONTEXT_FHAVECR2   0x0001 /* The `m_cr2' field contains a valid value. */
#define __MCONTEXT_FHAVEFPU   0x1000 /* The `m_fpu' structure contains valid data and must be restored.
                                      * When not set: The FPU hasn't been initialized yet, and a context
                                      *               load will leave the FPU unmodified if it hasn't been
                                      *               initialized, or will default-initialize it if it
                                      *               had been initialized in the mean time. */



#ifdef __CC__
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("gregs")
#pragma push_macro("cr2")
#pragma push_macro("fpregs")
#pragma push_macro("significand")
#pragma push_macro("exponent")
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
#undef gregs
#undef cr2
#undef fpregs
#undef significand
#undef exponent
#undef cw
#undef sw
#undef tag
#undef ipoff
#undef cssel
#undef dataoff
#undef datasel
#undef _st
#undef status

struct _libc_fpreg {
    __UINT16_TYPE__    significand[4];
    __UINT16_TYPE__    exponent;
};

struct __ATTR_PACKED _libc_fpstate {
    __UINT16_TYPE__     cw;
    __UINT16_TYPE__     sw;
    __UINT8_TYPE__      tag;
    __UINT8_TYPE__    __pad0[3];
    __UINT32_TYPE__     ipoff;
    __UINT16_TYPE__     cssel;
    __UINT8_TYPE__    __pad1[2];
    __UINT32_TYPE__     dataoff;
    __UINT16_TYPE__     datasel;
    __UINT8_TYPE__    __pad2[2];
    __UINT32_TYPE__     status;
    __UINT8_TYPE__    __pad3[4];
    struct _libc_fpreg  _st[8];
};

typedef struct _libc_fpstate fpregset_t[1];

typedef struct __ATTR_PACKED mcontext {
    union __ATTR_PACKED {
        gregset_t              gregs;     /* GLibC API compatibility... */
        gregset_t              m_gregs;   /* General purpose register array. */
#ifdef __KERNEL__
        struct cpu_usercontext m_context; /* The user-space CPU context */
#else
        struct cpu_context     m_context; /* The user-space CPU context */
#endif
    };
    union __ATTR_PACKED {
        fpregset_t             fpregs;    /* GLibC API compatibility... */
        struct fpu_context     m_fpu;     /* FPU Context. */
    };
    __UINTPTR_TYPE__           m_flags;   /* Set of `__MCONTEXT_F*' */
    union __ATTR_PACKED {
        __ULONGPTR_TYPE__      cr2;       /* GLibC API compatibility... */
        __ULONGPTR_TYPE__      m_cr2;     /* The fault address of SIGSEGV. */
    };
} mcontext_t;

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
#pragma pop_macro("exponent")
#pragma pop_macro("significand")
#pragma pop_macro("fpregs")
#pragma pop_macro("cr2")
#pragma pop_macro("gregs")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#endif /* __CC__ */



#ifdef __CC__
/* Userlevel context. */
typedef struct __ATTR_PACKED ucontext {
    mcontext_t           uc_mcontext;
    __sigset_t           uc_sigmask;
    stack_t              uc_stack;
    struct ucontext     *uc_link;
} ucontext_t;
#endif /* __CC__ */

__SYSDECL_END

#endif /* !_PARTS_GLC_I386_UCONTEXT_H */
