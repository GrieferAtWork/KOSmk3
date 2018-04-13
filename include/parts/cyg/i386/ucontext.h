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
#ifndef _PARTS_CYG_I386_UCONTEXT_H
#define _PARTS_CYG_I386_UCONTEXT_H 1
#define _SYS_UCONTEXT_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/host.h>
#include <bits/sigset.h>
#include <bits/sigcontext.h>
#include <bits/sigstack.h>

__SYSDECL_BEGIN

#ifdef __CC__

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("significand")
#pragma push_macro("exponent")
#pragma push_macro("_fpstate")
#pragma push_macro("ctxflags")
#pragma push_macro("cs")
#pragma push_macro("ds")
#pragma push_macro("es")
#pragma push_macro("fs")
#pragma push_macro("gs")
#pragma push_macro("ss")
#pragma push_macro("eflags")
#pragma push_macro("dr0")
#pragma push_macro("dr1")
#pragma push_macro("dr2")
#pragma push_macro("dr3")
#pragma push_macro("dr6")
#pragma push_macro("dr7")
#pragma push_macro("oldmask")
#pragma push_macro("cr2")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

#undef significand
#undef exponent
#undef _fpstate
#undef ctxflags
#undef cs
#undef ds
#undef es
#undef fs
#undef gs
#undef ss
#undef eflags
#undef dr0
#undef dr1
#undef dr2
#undef dr3
#undef dr6
#undef dr7
#undef oldmask
#undef cr2

#ifdef __x86_64__
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("_uc_fpxreg")
#pragma push_macro("_uc_xmmreg")
#pragma push_macro("element")
#pragma push_macro("cwd")
#pragma push_macro("swd")
#pragma push_macro("ftw")
#pragma push_macro("fop")
#pragma push_macro("rdp")
#pragma push_macro("mxcr_mask")
#pragma push_macro("st")
#pragma push_macro("xmm")
#pragma push_macro("padding")
#pragma push_macro("p1home")
#pragma push_macro("p2home")
#pragma push_macro("p3home")
#pragma push_macro("p4home")
#pragma push_macro("p5home")
#pragma push_macro("p6home")
#pragma push_macro("mxcsr")
#pragma push_macro("rax")
#pragma push_macro("rcx")
#pragma push_macro("rdx")
#pragma push_macro("rbx")
#pragma push_macro("rsp")
#pragma push_macro("rbp")
#pragma push_macro("rsi")
#pragma push_macro("rdi")
#pragma push_macro("r8")
#pragma push_macro("r9")
#pragma push_macro("r10")
#pragma push_macro("r11")
#pragma push_macro("r12")
#pragma push_macro("r13")
#pragma push_macro("r14")
#pragma push_macro("r15")
#pragma push_macro("rip")
#pragma push_macro("fpregs")
#pragma push_macro("vregs")
#pragma push_macro("vcx")
#pragma push_macro("dbc")
#pragma push_macro("btr")
#pragma push_macro("bfr")
#pragma push_macro("etr")
#pragma push_macro("efr")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

#undef _uc_fpxreg
#undef _uc_xmmreg
#undef element
#undef cwd
#undef swd
#undef ftw
#undef fop
#undef rdp
#undef mxcr_mask
#undef st
#undef xmm
#undef padding
#undef p1home
#undef p2home
#undef p3home
#undef p4home
#undef p5home
#undef p6home
#undef mxcsr
#undef rax
#undef rcx
#undef rdx
#undef rbx
#undef rsp
#undef rbp
#undef rsi
#undef rdi
#undef r8
#undef r9
#undef r10
#undef r11
#undef r12
#undef r13
#undef r14
#undef r15
#undef rip
#undef fpregs
#undef vregs
#undef vcx
#undef dbc
#undef btr
#undef bfr
#undef etr
#undef efr

struct _uc_fpxreg {
    __uint16_t significand[4];
    __uint16_t exponent;
    __uint16_t padding[3];
};
struct _uc_xmmreg {
    __uint32_t element[4];
};
struct _fpstate {
    __uint16_t        cwd;
    __uint16_t        swd;
    __uint16_t        ftw;
    __uint16_t        fop;
    __uint64_t        rip;
    __uint64_t        rdp;
    __uint32_t        mxcsr;
    __uint32_t        mxcr_mask;
    struct _uc_fpxreg st[8];
    struct _uc_xmmreg xmm[16];
    __uint32_t        padding[24];
};

struct __ATTR_ALIGNED(16) __mcontext {
    __uint64_t      p1home;
    __uint64_t      p2home;
    __uint64_t      p3home;
    __uint64_t      p4home;
    __uint64_t      p5home;
    __uint64_t      p6home;
    __uint32_t      ctxflags;
    __uint32_t      mxcsr;
    __uint16_t      cs;
    __uint16_t      ds;
    __uint16_t      es;
    __uint16_t      fs;
    __uint16_t      gs;
    __uint16_t      ss;
    __uint32_t      eflags;
    __uint64_t      dr0;
    __uint64_t      dr1;
    __uint64_t      dr2;
    __uint64_t      dr3;
    __uint64_t      dr6;
    __uint64_t      dr7;
    __uint64_t      rax;
    __uint64_t      rcx;
    __uint64_t      rdx;
    __uint64_t      rbx;
    __uint64_t      rsp;
    __uint64_t      rbp;
    __uint64_t      rsi;
    __uint64_t      rdi;
    __uint64_t      r8;
    __uint64_t      r9;
    __uint64_t      r10;
    __uint64_t      r11;
    __uint64_t      r12;
    __uint64_t      r13;
    __uint64_t      r14;
    __uint64_t      r15;
    __uint64_t      rip;
    struct _fpstate fpregs;
    __uint64_t      vregs[52];
    __uint64_t      vcx;
    __uint64_t      dbc;
    __uint64_t      btr;
    __uint64_t      bfr;
    __uint64_t      etr;
    __uint64_t      efr;
    __uint64_t      oldmask;
    __uint64_t      cr2;
};

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("efr")
#pragma pop_macro("etr")
#pragma pop_macro("bfr")
#pragma pop_macro("btr")
#pragma pop_macro("dbc")
#pragma pop_macro("vcx")
#pragma pop_macro("vregs")
#pragma pop_macro("fpregs")
#pragma pop_macro("rip")
#pragma pop_macro("r15")
#pragma pop_macro("r14")
#pragma pop_macro("r13")
#pragma pop_macro("r12")
#pragma pop_macro("r11")
#pragma pop_macro("r10")
#pragma pop_macro("r9")
#pragma pop_macro("r8")
#pragma pop_macro("rdi")
#pragma pop_macro("rsi")
#pragma pop_macro("rbp")
#pragma pop_macro("rsp")
#pragma pop_macro("rbx")
#pragma pop_macro("rdx")
#pragma pop_macro("rcx")
#pragma pop_macro("rax")
#pragma pop_macro("mxcsr")
#pragma pop_macro("p6home")
#pragma pop_macro("p5home")
#pragma pop_macro("p4home")
#pragma pop_macro("p3home")
#pragma pop_macro("p2home")
#pragma pop_macro("p1home")
#pragma pop_macro("padding")
#pragma pop_macro("xmm")
#pragma pop_macro("st")
#pragma pop_macro("mxcr_mask")
#pragma pop_macro("rdp")
#pragma pop_macro("fop")
#pragma pop_macro("ftw")
#pragma pop_macro("swd")
#pragma pop_macro("cwd")
#pragma pop_macro("element")
#pragma pop_macro("_uc_xmmreg")
#pragma pop_macro("_uc_fpxreg")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

#else /* __x86_64__ */

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("_uc_fpreg")
#pragma push_macro("cw")
#pragma push_macro("sw")
#pragma push_macro("tag")
#pragma push_macro("ipoff")
#pragma push_macro("cssel")
#pragma push_macro("dataoff")
#pragma push_macro("datasel")
#pragma push_macro("_st")
#pragma push_macro("nxst")
#pragma push_macro("fpstate")
#pragma push_macro("edi")
#pragma push_macro("esi")
#pragma push_macro("ebx")
#pragma push_macro("edx")
#pragma push_macro("ecx")
#pragma push_macro("eax")
#pragma push_macro("ebp")
#pragma push_macro("eip")
#pragma push_macro("esp")
#pragma push_macro("reserved")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef _uc_fpreg
#undef cw
#undef sw
#undef tag
#undef ipoff
#undef cssel
#undef dataoff
#undef datasel
#undef _st
#undef nxst
#undef fpstate
#undef edi
#undef esi
#undef ebx
#undef edx
#undef ecx
#undef eax
#undef ebp
#undef eip
#undef esp
#undef reserved

struct _uc_fpreg {
    __uint16_t significand[4];
    __uint16_t exponent;
};
struct _fpstate {
    __uint32_t       cw;
    __uint32_t       sw;
    __uint32_t       tag;
    __uint32_t       ipoff;
    __uint32_t       cssel;
    __uint32_t       dataoff;
    __uint32_t       datasel;
    struct _uc_fpreg _st[8];
    __uint32_t       nxst;
};
struct __mcontext {
    __uint32_t      ctxflags;
    __uint32_t      dr0;
    __uint32_t      dr1;
    __uint32_t      dr2;
    __uint32_t      dr3;
    __uint32_t      dr6;
    __uint32_t      dr7;
    struct _fpstate fpstate;
    __uint32_t      gs;
    __uint32_t      fs;
    __uint32_t      es;
    __uint32_t      ds;
    __uint32_t      edi;
    __uint32_t      esi;
    __uint32_t      ebx;
    __uint32_t      edx;
    __uint32_t      ecx;
    __uint32_t      eax;
    __uint32_t      ebp;
    __uint32_t      eip;
    __uint32_t      cs;
    __uint32_t      eflags;
    __uint32_t      esp;
    __uint32_t      ss;
    __uint32_t      reserved[128];
    __uint32_t      oldmask;
    __uint32_t      cr2;
};

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("reserved")
#pragma pop_macro("esp")
#pragma pop_macro("eip")
#pragma pop_macro("ebp")
#pragma pop_macro("eax")
#pragma pop_macro("ecx")
#pragma pop_macro("edx")
#pragma pop_macro("ebx")
#pragma pop_macro("esi")
#pragma pop_macro("edi")
#pragma pop_macro("fpstate")
#pragma pop_macro("nxst")
#pragma pop_macro("_st")
#pragma pop_macro("datasel")
#pragma pop_macro("dataoff")
#pragma pop_macro("cssel")
#pragma pop_macro("ipoff")
#pragma pop_macro("tag")
#pragma pop_macro("sw")
#pragma pop_macro("cw")
#pragma pop_macro("_uc_fpreg")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

#endif /* !__x86_64__ */

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("cr2")
#pragma pop_macro("oldmask")
#pragma pop_macro("dr7")
#pragma pop_macro("dr6")
#pragma pop_macro("dr3")
#pragma pop_macro("dr2")
#pragma pop_macro("dr1")
#pragma pop_macro("dr0")
#pragma pop_macro("eflags")
#pragma pop_macro("ss")
#pragma pop_macro("gs")
#pragma pop_macro("fs")
#pragma pop_macro("es")
#pragma pop_macro("ds")
#pragma pop_macro("cs")
#pragma pop_macro("ctxflags")
#pragma pop_macro("_fpstate")
#pragma pop_macro("exponent")
#pragma pop_macro("significand")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

typedef struct __mcontext mcontext_t;

/* Userlevel context. */
typedef __ATTR_ALIGNED(16) struct __ucontext {
    mcontext_t         uc_mcontext;
    struct __ucontext *uc_link;
    __sigset_t         uc_sigmask;
    stack_t            uc_stack;
    unsigned long int  uc_flags;
} ucontext_t;
#endif /* __CC__ */

__SYSDECL_END

#endif /* !_PARTS_CYG_I386_UCONTEXT_H */
