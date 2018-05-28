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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_SYSCALL_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_SYSCALL_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <bits/types.h>
#include <kos/context.h>
#ifdef __INTELLISENSE__
#include <asm/syscallno.ci>
#endif

DECL_BEGIN

#ifdef __x86_64__
/* Instruct the kernel to implement compatibility system calls. */
#define CONFIG_SYSCALL_COMPAT 1
//#define CONFIG_NO_X86_SYSCALL 1
//#define CONFIG_NO_X86_SYSENTER 1
#else
#define CONFIG_NO_X86_SYSCALL 1
//#define CONFIG_NO_X86_SYSENTER 1
#endif



/* Arch-specific flags to go alongside `TASK_USERCTX_F*'. */
#define X86_SYSCALL_TYPE_FINT80    0x0000 /* Restart a system call using a register state used by `int $0x80' */
#ifndef CONFIG_NO_X86_SYSENTER
#define X86_SYSCALL_TYPE_FSYSENTER 0x0100 /* Restart a system call using a register state used by `sysenter' */
#endif /* !CONFIG_NO_X86_SYSENTER */
#define X86_SYSCALL_TYPE_FPF       0x0200 /* Restart a system call using a register state used by #PF-based system calls. */
#ifndef CONFIG_NO_X86_SYSCALL
#define X86_SYSCALL_TYPE_FSYSCALL  0x0300 /* Restart a system call using a register state used by `syscall' */
#endif /* !CONFIG_NO_X86_SYSCALL */


#ifndef CONFIG_NO_X86_SYSENTER
/* KOS's sysenter ABI (for i386+):
 *   
 * CLOBBER:
 *   - %ecx
 *   - %edx (Only for 32-bit return values)
 * ARGUMENTS:
 *   - SYSCALLNO:      %eax
 *   - ARG0:           %ebx
 *   - ARG1:           %ecx
 *   - ARG2:           %edx
 *   - ARG3:           %esi
 *   - ARG4:         0(%ebp)
 *   - ARG5:         4(%ebp)
 * RETURN:
 *   - RET1:  %eax
 *   - RET2:  %edx    (When set, don't use sysexit for return)
 * CLEANUP:
 *   - return.%eip =   %edi
 *   - return.%esp =   %ebp
 * INVOCATION:
 *   >> syscall:
 *   >>     pushl   %ebp
 *   >>     pushl   %edi
 *   >>     pushl   %esi
 *   >>     pushl   %ebx
 *   >>     movl    20(%esp), %eax
 *   >>     movl    24(%esp), %ebx
 *   >>     movl    28(%esp), %ecx
 *   >>     movl    32(%esp), %edx
 *   >>     movl    36(%esp), %esi
 *   >>     pushl   44(%esp)   // ARG5 (not required if the system call doesn't take this argument)
 *   >>     pushl   40(%esp)   // ARG4 (not required if the system call doesn't take this argument)
 *   >>     movl    $1f,  %edi // Return address
 *   >>     movl    %esp, %ebp // Return SP + argument block
 *   >>     sysenter
 *   >>1:   addl    $8,   %esp
 *   >>     popl    %ebx
 *   >>     popl    %esi
 *   >>     popl    %edi
 *   >>     popl    %ebp
 *   >>     ret
 */
#endif /* !CONFIG_NO_X86_SYSENTER */


#ifndef CONFIG_NO_X86_SYSCALL
/* KOS's sysenter ABI (for x86_64):
 *   
 * CLOBBER:
 *   - %rcx
 *   - %r11
 *   - %rdx (Only for 32-bit return values)
 * ARGUMENTS:
 *   - SYSCALLNO:      %rax
 *   - ARG0:           %rdi
 *   - ARG1:           %rsi
 *   - ARG2:           %rdx
 *   - ARG3:           %r10
 *   - ARG4:           %r8
 *   - ARG5:           %r9
 * RETURN:
 *   - RET1:           %rax
 *   - RET2:           %rdx    (When set, don't use sysret for return)
 * INVOCATION:
 *   >> syscall:
 *   >>     movq       %rdi,   %rax // SYSCALLNO
 *   >>     movq       %rsi,   %rdi // ARG0
 *   >>     movq       %rdx,   %rsi // ARG1
 *   >>     movq       %rcx,   %rdx // ARG2
 *   >>     movq       %r8,    %r10 // ARG3
 *   >>     movq       %r9,    %r8  // ARG4
 *   >>     movq      8(%rsp), %r9  // ARG5
 *   >>     syscall
 *   >>     ret
 */
#endif /* !CONFIG_NO_X86_SYSENTER */




/* Syscall registers. (NOTE: Differs slightly from sysenter ABI; see above) */
#ifdef __x86_64__
#define X86_SYSCALL_REGNO  %rax /* System call vector. */
#define X86_SYSCALL_REG0   %rdi /* Arg #0 */
#define X86_SYSCALL_REG1   %rsi /* Arg #1 */
#define X86_SYSCALL_REG2   %rdx /* Arg #2 */
#define X86_SYSCALL_REG3   %r10 /* Arg #3 */
#define X86_SYSCALL_REG4   %r8  /* Arg #4 */
#define X86_SYSCALL_REG5   %r9  /* Arg #5 */
#define X86_SYSCALL_RET0   %rax /* Return register #0 */
#define X86_SYSCALL_RET1   %rdx /* Return register #1 */
#else
#define X86_SYSCALL_REGNO  %eax /* System call vector. */
#define X86_SYSCALL_REG0   %ebx /* Arg #0 */
#define X86_SYSCALL_REG1   %ecx /* Arg #1 */
#define X86_SYSCALL_REG2   %edx /* Arg #2 */
#define X86_SYSCALL_REG3   %esi /* Arg #3 */
#define X86_SYSCALL_REG4   %edi /* Arg #4 */
#define X86_SYSCALL_REG5   %ebp /* Arg #5 */
#define X86_SYSCALL_RET0   %eax /* Return register #0 */
#define X86_SYSCALL_RET1   %edx /* Return register #1 */
#endif

#define __PRIVATE_SYSCALL_LONG_0(...)                                 void
#define __PRIVATE_SYSCALL_LONG_1(T0,N0)                               syscall_ulong_t N0
#define __PRIVATE_SYSCALL_LONG_2(T0,N0,T1,N1)                         __PRIVATE_SYSCALL_LONG_1(T0,N0), syscall_ulong_t N1
#define __PRIVATE_SYSCALL_LONG_3(T0,N0,T1,N1,T2,N2)                   __PRIVATE_SYSCALL_LONG_2(T0,N0,T1,N1), syscall_ulong_t N2
#define __PRIVATE_SYSCALL_LONG_4(T0,N0,T1,N1,T2,N2,T3,N3)             __PRIVATE_SYSCALL_LONG_3(T0,N0,T1,N1,T2,N2), syscall_ulong_t N3
#define __PRIVATE_SYSCALL_LONG_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4)       __PRIVATE_SYSCALL_LONG_4(T0,N0,T1,N1,T2,N2,T3,N3), syscall_ulong_t N4
#define __PRIVATE_SYSCALL_LONG_6(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4,T5,N5) __PRIVATE_SYSCALL_LONG_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4), syscall_ulong_t N5
#define __SYSCALL_LONG(argc,argv)  __PRIVATE_SYSCALL_LONG_##argc argv

#define __PRIVATE_SYSCALL_DECL_0(...)                                 void
#define __PRIVATE_SYSCALL_DECL_1(T0,N0)                               T0 N0
#define __PRIVATE_SYSCALL_DECL_2(T0,N0,T1,N1)                         __PRIVATE_SYSCALL_DECL_1(T0,N0), T1 N1
#define __PRIVATE_SYSCALL_DECL_3(T0,N0,T1,N1,T2,N2)                   __PRIVATE_SYSCALL_DECL_2(T0,N0,T1,N1), T2 N2
#define __PRIVATE_SYSCALL_DECL_4(T0,N0,T1,N1,T2,N2,T3,N3)             __PRIVATE_SYSCALL_DECL_3(T0,N0,T1,N1,T2,N2), T3 N3
#define __PRIVATE_SYSCALL_DECL_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4)       __PRIVATE_SYSCALL_DECL_4(T0,N0,T1,N1,T2,N2,T3,N3), T4 N4
#define __PRIVATE_SYSCALL_DECL_6(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4,T5,N5) __PRIVATE_SYSCALL_DECL_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4), T5 N5
#define __SYSCALL_DECL(argc,argv)  __PRIVATE_SYSCALL_DECL_##argc argv

#define __PRIVATE_SYSCALL_CAST_0(...)                                 /* Nothing */
#define __PRIVATE_SYSCALL_CAST_1(T0,N0)                               (T0)N0
#define __PRIVATE_SYSCALL_CAST_2(T0,N0,T1,N1)                         __PRIVATE_SYSCALL_CAST_1(T0,N0),(T1)N1
#define __PRIVATE_SYSCALL_CAST_3(T0,N0,T1,N1,T2,N2)                   __PRIVATE_SYSCALL_CAST_2(T0,N0,T1,N1),(T2)N2
#define __PRIVATE_SYSCALL_CAST_4(T0,N0,T1,N1,T2,N2,T3,N3)             __PRIVATE_SYSCALL_CAST_3(T0,N0,T1,N1,T2,N2),(T3)N3
#define __PRIVATE_SYSCALL_CAST_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4)       __PRIVATE_SYSCALL_CAST_4(T0,N0,T1,N1,T2,N2,T3,N3),(T4)N4
#define __PRIVATE_SYSCALL_CAST_6(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4,T5,N5) __PRIVATE_SYSCALL_CAST_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4),(T5)N5
#define __SYSCALL_CAST(argc,argv)  __PRIVATE_SYSCALL_CAST_##argc argv

#ifdef __x86_64__
#define __PRIVATE_SYSCALL_ASSERT(T,N) STATIC_ASSERT_MSG(sizeof(T) <= 8,"Argument type " #T " of " #N " is too large");
#else
#define __PRIVATE_SYSCALL_ASSERT(T,N) STATIC_ASSERT_MSG(sizeof(T) <= 4,"Argument type " #T " of " #N " is too large");
#endif
#define __PRIVATE_SYSCALL_ASSERT_0(...)                                 /* Nothing */
#define __PRIVATE_SYSCALL_ASSERT_1(T0,N0)                               __PRIVATE_SYSCALL_ASSERT(T0,N0)
#define __PRIVATE_SYSCALL_ASSERT_2(T0,N0,T1,N1)                         __PRIVATE_SYSCALL_ASSERT_1(T0,N0) __PRIVATE_SYSCALL_ASSERT(T1,N1)
#define __PRIVATE_SYSCALL_ASSERT_3(T0,N0,T1,N1,T2,N2)                   __PRIVATE_SYSCALL_ASSERT_2(T0,N0,T1,N1) __PRIVATE_SYSCALL_ASSERT(T2,N2)
#define __PRIVATE_SYSCALL_ASSERT_4(T0,N0,T1,N1,T2,N2,T3,N3)             __PRIVATE_SYSCALL_ASSERT_3(T0,N0,T1,N1,T2,N2) __PRIVATE_SYSCALL_ASSERT(T3,N3)
#define __PRIVATE_SYSCALL_ASSERT_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4)       __PRIVATE_SYSCALL_ASSERT_4(T0,N0,T1,N1,T2,N2,T3,N3) __PRIVATE_SYSCALL_ASSERT(T4,N4)
#define __PRIVATE_SYSCALL_ASSERT_6(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4,T5,N5) __PRIVATE_SYSCALL_ASSERT_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4) __PRIVATE_SYSCALL_ASSERT(T5,N5)
#define __SYSCALL_ASSERT(argc,argv)  __PRIVATE_SYSCALL_ASSERT_##argc argv
#ifdef __INTELLISENSE__
#define __SYSCALL_ASSERT_NAME(name) (void)(__NR_##name);
#else
#define __SYSCALL_ASSERT_NAME(name) /* Nothing */
#endif

#define __X86_DEFINE_SYSCALLn(name,argc,argv) \
__asm__(".hidden argc_sys_" #name "\n" \
        ".global argc_sys_" #name "\n" \
        ".set argc_sys_" #name ", " #argc "\n"); \
LOCAL syscall_ulong_t ATTR_CDECL SYSC_##name(__SYSCALL_DECL(argc,argv)); \
INTERN syscall_ulong_t ATTR_CDECL sys_##name(__SYSCALL_LONG(argc,argv)) \
{ \
    __SYSCALL_ASSERT(argc,argv) \
    __SYSCALL_ASSERT_NAME(name) \
    return SYSC_##name(__SYSCALL_CAST(argc,argv)); \
} \
LOCAL syscall_ulong_t ATTR_CDECL SYSC_##name(__SYSCALL_DECL(argc,argv))



#ifdef __x86_64__
#define __X86_DEFINE_SYSCALLn64(name,argc,argv) \
        __X86_DEFINE_SYSCALLn(name,argc,argv)
#else
#define __X86_DEFINE_SYSCALLn64(name,argc,argv) \
__asm__(".hidden argc_sys_" #name "\n" \
        ".global argc_sys_" #name "\n" \
        ".set argc_sys_" #name ", " #argc "\n" \
        ".hidden sys_" #name "\n" \
        ".global sys_" #name "\n" \
        ".section .text\n" \
        "sys_" #name ":\n" \
        "    addl $x86_syscall64_adjustment, (%esp)\n" \
        "    jmp  sys64_" #name "\n" \
        ".size sys_" #name ", . - sys_" #name "\n"); \
LOCAL u64 ATTR_CDECL SYSC_##name(__SYSCALL_DECL(argc,argv)); \
INTERN u64 ATTR_CDECL sys64_##name(__SYSCALL_LONG(argc,argv)) \
{ \
    __SYSCALL_ASSERT(argc,argv) \
    __SYSCALL_ASSERT_NAME(name) \
    return SYSC_##name(__SYSCALL_CAST(argc,argv)); \
} \
LOCAL u64 ATTR_CDECL SYSC_##name(__SYSCALL_DECL(argc,argv))
#endif

#undef CONFIG_WIDE_64BIT_SYSCALL
#ifdef __ARCH_WIDE_64BIT_SYSCALL
#define CONFIG_WIDE_64BIT_SYSCALL 1
#define CONFIG_SYSCALL_ARG64_LOFIRST 1
#undef CONFIG_SYSCALL_ARG64_HIFIRST
#endif


#define X86_SYSCALL_RESTART_FAUTO  0x00
#define X86_SYSCALL_RESTART_FDONT  0x01
#define X86_SYSCALL_RESTART_FMUST  0x02
#define __X86_DEFINE_SYSCALL_RESTART(name,mode) \
__asm__(".hidden restart_sys_" #name "\n\t" \
        ".global restart_sys_" #name "\n\t" \
        ".set restart_sys_" #name ", " PP_PRIVATE_STR(mode))


#define DEFINE_SYSCALL0(name)        __X86_DEFINE_SYSCALLn(name,0,())
#define DEFINE_SYSCALL1(name,...)    __X86_DEFINE_SYSCALLn(name,1,(__VA_ARGS__))
#define DEFINE_SYSCALL2(name,...)    __X86_DEFINE_SYSCALLn(name,2,(__VA_ARGS__))
#define DEFINE_SYSCALL3(name,...)    __X86_DEFINE_SYSCALLn(name,3,(__VA_ARGS__))
#define DEFINE_SYSCALL4(name,...)    __X86_DEFINE_SYSCALLn(name,4,(__VA_ARGS__))
#define DEFINE_SYSCALL5(name,...)    __X86_DEFINE_SYSCALLn(name,5,(__VA_ARGS__))
#define DEFINE_SYSCALL6(name,...)    __X86_DEFINE_SYSCALLn(name,6,(__VA_ARGS__))
#define DEFINE_SYSCALL0_64(name)     __X86_DEFINE_SYSCALLn64(name,0,())
#define DEFINE_SYSCALL1_64(name,...) __X86_DEFINE_SYSCALLn64(name,1,(__VA_ARGS__))
#define DEFINE_SYSCALL2_64(name,...) __X86_DEFINE_SYSCALLn64(name,2,(__VA_ARGS__))
#define DEFINE_SYSCALL3_64(name,...) __X86_DEFINE_SYSCALLn64(name,3,(__VA_ARGS__))
#define DEFINE_SYSCALL4_64(name,...) __X86_DEFINE_SYSCALLn64(name,4,(__VA_ARGS__))
#define DEFINE_SYSCALL5_64(name,...) __X86_DEFINE_SYSCALLn64(name,5,(__VA_ARGS__))
#define DEFINE_SYSCALL6_64(name,...) __X86_DEFINE_SYSCALLn64(name,6,(__VA_ARGS__))

/* Define the restart-after-interrupt behavior for a system call `name':
 * DEFINE_SYSCALL_AUTORESTART:
 *    - Always restart after an `E_INTERRUPT'
 *    - Restart from sigreturn() if the signal handler had the `SA_RESTART' flag set
 * DEFINE_SYSCALL_DONTRESTART:
 *    - Never restart. - Always propagate `E_INTERRUPT'
 * DEFINE_SYSCALL_MUSTRESTART:
 *    - Always restart, even from `sigreturn()' when the
 *      handler didn't have the `SA_RESTART' flag set
 */
#define DEFINE_SYSCALL_AUTORESTART(name) __X86_DEFINE_SYSCALL_RESTART(name,X86_SYSCALL_RESTART_FAUTO)
#define DEFINE_SYSCALL_DONTRESTART(name) __X86_DEFINE_SYSCALL_RESTART(name,X86_SYSCALL_RESTART_FDONT)
#define DEFINE_SYSCALL_MUSTRESTART(name) __X86_DEFINE_SYSCALL_RESTART(name,X86_SYSCALL_RESTART_FMUST)







/* Compatibility system calls. */
#ifdef CONFIG_SYSCALL_COMPAT
#define X86_SYSCALL_COMPAT_REGNO  %eax /* System call vector. */
#define X86_SYSCALL_COMPAT_REG0   %ebx /* Arg #0 */
#define X86_SYSCALL_COMPAT_REG1   %ecx /* Arg #1 */
#define X86_SYSCALL_COMPAT_REG2   %edx /* Arg #2 */
#define X86_SYSCALL_COMPAT_REG3   %esi /* Arg #3 */
#define X86_SYSCALL_COMPAT_REG4   %edi /* Arg #4 */
#define X86_SYSCALL_COMPAT_REG5   %ebp /* Arg #5 */
#define X86_SYSCALL_COMPAT_RET0   %eax /* Return register #0 */
#define X86_SYSCALL_COMPAT_RET1   %edx /* Return register #1 */

#define __PRIVATE_SYSCALL_COMPAT_LONG_0(...)                                 void
#define __PRIVATE_SYSCALL_COMPAT_LONG_1(T0,N0)                               u32 N0
#define __PRIVATE_SYSCALL_COMPAT_LONG_2(T0,N0,T1,N1)                         __PRIVATE_SYSCALL_COMPAT_LONG_1(T0,N0), u32 N1
#define __PRIVATE_SYSCALL_COMPAT_LONG_3(T0,N0,T1,N1,T2,N2)                   __PRIVATE_SYSCALL_COMPAT_LONG_2(T0,N0,T1,N1), u32 N2
#define __PRIVATE_SYSCALL_COMPAT_LONG_4(T0,N0,T1,N1,T2,N2,T3,N3)             __PRIVATE_SYSCALL_COMPAT_LONG_3(T0,N0,T1,N1,T2,N2), u32 N3
#define __PRIVATE_SYSCALL_COMPAT_LONG_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4)       __PRIVATE_SYSCALL_COMPAT_LONG_4(T0,N0,T1,N1,T2,N2,T3,N3), u32 N4
#define __PRIVATE_SYSCALL_COMPAT_LONG_6(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4,T5,N5) __PRIVATE_SYSCALL_COMPAT_LONG_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4), u32 N5
#define __SYSCALL_COMPAT_LONG(argc,argv)  __PRIVATE_SYSCALL_COMPAT_LONG_##argc argv

#define __PRIVATE_SYSCALL_COMPAT_DECL_0(...)                                 void
#define __PRIVATE_SYSCALL_COMPAT_DECL_1(T0,N0)                               T0 N0
#define __PRIVATE_SYSCALL_COMPAT_DECL_2(T0,N0,T1,N1)                         __PRIVATE_SYSCALL_COMPAT_DECL_1(T0,N0), T1 N1
#define __PRIVATE_SYSCALL_COMPAT_DECL_3(T0,N0,T1,N1,T2,N2)                   __PRIVATE_SYSCALL_COMPAT_DECL_2(T0,N0,T1,N1), T2 N2
#define __PRIVATE_SYSCALL_COMPAT_DECL_4(T0,N0,T1,N1,T2,N2,T3,N3)             __PRIVATE_SYSCALL_COMPAT_DECL_3(T0,N0,T1,N1,T2,N2), T3 N3
#define __PRIVATE_SYSCALL_COMPAT_DECL_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4)       __PRIVATE_SYSCALL_COMPAT_DECL_4(T0,N0,T1,N1,T2,N2,T3,N3), T4 N4
#define __PRIVATE_SYSCALL_COMPAT_DECL_6(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4,T5,N5) __PRIVATE_SYSCALL_COMPAT_DECL_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4), T5 N5
#define __SYSCALL_COMPAT_DECL(argc,argv)  __PRIVATE_SYSCALL_COMPAT_DECL_##argc argv

#define __PRIVATE_SYSCALL_COMPAT_CAST_0(...)                                 /* Nothing */
#define __PRIVATE_SYSCALL_COMPAT_CAST_1(T0,N0)                               (T0)N0
#define __PRIVATE_SYSCALL_COMPAT_CAST_2(T0,N0,T1,N1)                         __PRIVATE_SYSCALL_COMPAT_CAST_1(T0,N0),(T1)N1
#define __PRIVATE_SYSCALL_COMPAT_CAST_3(T0,N0,T1,N1,T2,N2)                   __PRIVATE_SYSCALL_COMPAT_CAST_2(T0,N0,T1,N1),(T2)N2
#define __PRIVATE_SYSCALL_COMPAT_CAST_4(T0,N0,T1,N1,T2,N2,T3,N3)             __PRIVATE_SYSCALL_COMPAT_CAST_3(T0,N0,T1,N1,T2,N2),(T3)N3
#define __PRIVATE_SYSCALL_COMPAT_CAST_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4)       __PRIVATE_SYSCALL_COMPAT_CAST_4(T0,N0,T1,N1,T2,N2,T3,N3),(T4)N4
#define __PRIVATE_SYSCALL_COMPAT_CAST_6(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4,T5,N5) __PRIVATE_SYSCALL_COMPAT_CAST_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4),(T5)N5
#define __SYSCALL_COMPAT_CAST(argc,argv)  __PRIVATE_SYSCALL_COMPAT_CAST_##argc argv

#define __PRIVATE_SYSCALL_COMPAT_ASSERT(T,N) STATIC_ASSERT_MSG(sizeof(T) <= 4,"Argument type " #T " of " #N " is too large");
#define __PRIVATE_SYSCALL_COMPAT_ASSERT_0(...)                                 /* Nothing */
#define __PRIVATE_SYSCALL_COMPAT_ASSERT_1(T0,N0)                               __PRIVATE_SYSCALL_COMPAT_ASSERT(T0,N0)
#define __PRIVATE_SYSCALL_COMPAT_ASSERT_2(T0,N0,T1,N1)                         __PRIVATE_SYSCALL_COMPAT_ASSERT_1(T0,N0) __PRIVATE_SYSCALL_COMPAT_ASSERT(T1,N1)
#define __PRIVATE_SYSCALL_COMPAT_ASSERT_3(T0,N0,T1,N1,T2,N2)                   __PRIVATE_SYSCALL_COMPAT_ASSERT_2(T0,N0,T1,N1) __PRIVATE_SYSCALL_COMPAT_ASSERT(T2,N2)
#define __PRIVATE_SYSCALL_COMPAT_ASSERT_4(T0,N0,T1,N1,T2,N2,T3,N3)             __PRIVATE_SYSCALL_COMPAT_ASSERT_3(T0,N0,T1,N1,T2,N2) __PRIVATE_SYSCALL_COMPAT_ASSERT(T3,N3)
#define __PRIVATE_SYSCALL_COMPAT_ASSERT_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4)       __PRIVATE_SYSCALL_COMPAT_ASSERT_4(T0,N0,T1,N1,T2,N2,T3,N3) __PRIVATE_SYSCALL_COMPAT_ASSERT(T4,N4)
#define __PRIVATE_SYSCALL_COMPAT_ASSERT_6(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4,T5,N5) __PRIVATE_SYSCALL_COMPAT_ASSERT_5(T0,N0,T1,N1,T2,N2,T3,N3,T4,N4) __PRIVATE_SYSCALL_COMPAT_ASSERT(T5,N5)
#define __SYSCALL_COMPAT_ASSERT(argc,argv)  __PRIVATE_SYSCALL_COMPAT_ASSERT_##argc argv
#ifdef __INTELLISENSE__
#define __SYSCALL_COMPAT_ASSERT_NAME(name) (void)(__NR_##name);
#else
#define __SYSCALL_COMPAT_ASSERT_NAME(name) /* Nothing */
#endif

#define __X86_DEFINE_SYSCALL_COMPATn(name,argc,argv) \
__asm__(".hidden argc_sys_" #name "_compat\n" \
        ".global argc_sys_" #name "_compat\n" \
        ".set argc_sys_" #name "_compat, " #argc "\n"); \
LOCAL u32 ATTR_CDECL SYSC_##name##_compat(__SYSCALL_COMPAT_DECL(argc,argv)); \
INTERN u32 ATTR_CDECL sys_##name##_compat(__SYSCALL_COMPAT_LONG(argc,argv)) \
{ \
    __SYSCALL_COMPAT_ASSERT(argc,argv) \
    __SYSCALL_COMPAT_ASSERT_NAME(name) \
    return SYSC_##name##_compat(__SYSCALL_COMPAT_CAST(argc,argv)); \
} \
LOCAL u32 ATTR_CDECL SYSC_##name##_compat(__SYSCALL_COMPAT_DECL(argc,argv))

#define __X86_DEFINE_SYSCALL_COMPATn64(name,argc,argv) \
__asm__(".hidden argc_sys_" #name "_compat\n" \
        ".global argc_sys_" #name "_compat\n" \
        ".set argc_sys_" #name "_compat, " #argc "\n" \
        ".hidden sys_" #name "_compat\n" \
        ".global sys_" #name "_compat\n" \
        ".section .text\n" \
        "sys_" #name "_compat:\n" \
        "    addl $x86_syscall64_adjustment_compat, (%esp)\n" \
        "    jmp  sys64_" #name "_compat\n" \
        ".size sys_" #name "_compat, . - sys_" #name "_compat\n"); \
LOCAL u64 ATTR_CDECL SYSC_##name##_compat(__SYSCALL_COMPAT_DECL(argc,argv)); \
INTERN u64 ATTR_CDECL sys64_##name##_compat(__SYSCALL_COMPAT_LONG(argc,argv)) \
{ \
    __SYSCALL_COMPAT_ASSERT(argc,argv) \
    __SYSCALL_COMPAT_ASSERT_NAME(name) \
    return SYSC_##name##_compat(__SYSCALL_COMPAT_CAST(argc,argv)); \
} \
LOCAL u64 ATTR_CDECL SYSC_##name##_compat(__SYSCALL_COMPAT_DECL(argc,argv))

#define DEFINE_SYSCALL_COMPAT0(name)        __X86_DEFINE_SYSCALL_COMPATn(name,0,())
#define DEFINE_SYSCALL_COMPAT1(name,...)    __X86_DEFINE_SYSCALL_COMPATn(name,1,(__VA_ARGS__))
#define DEFINE_SYSCALL_COMPAT2(name,...)    __X86_DEFINE_SYSCALL_COMPATn(name,2,(__VA_ARGS__))
#define DEFINE_SYSCALL_COMPAT3(name,...)    __X86_DEFINE_SYSCALL_COMPATn(name,3,(__VA_ARGS__))
#define DEFINE_SYSCALL_COMPAT4(name,...)    __X86_DEFINE_SYSCALL_COMPATn(name,4,(__VA_ARGS__))
#define DEFINE_SYSCALL_COMPAT5(name,...)    __X86_DEFINE_SYSCALL_COMPATn(name,5,(__VA_ARGS__))
#define DEFINE_SYSCALL_COMPAT6(name,...)    __X86_DEFINE_SYSCALL_COMPATn(name,6,(__VA_ARGS__))
#define DEFINE_SYSCALL_COMPAT0_64(name)     __X86_DEFINE_SYSCALL_COMPATn64(name,0,())
#define DEFINE_SYSCALL_COMPAT1_64(name,...) __X86_DEFINE_SYSCALL_COMPATn64(name,1,(__VA_ARGS__))
#define DEFINE_SYSCALL_COMPAT2_64(name,...) __X86_DEFINE_SYSCALL_COMPATn64(name,2,(__VA_ARGS__))
#define DEFINE_SYSCALL_COMPAT3_64(name,...) __X86_DEFINE_SYSCALL_COMPATn64(name,3,(__VA_ARGS__))
#define DEFINE_SYSCALL_COMPAT4_64(name,...) __X86_DEFINE_SYSCALL_COMPATn64(name,4,(__VA_ARGS__))
#define DEFINE_SYSCALL_COMPAT5_64(name,...) __X86_DEFINE_SYSCALL_COMPATn64(name,5,(__VA_ARGS__))
#define DEFINE_SYSCALL_COMPAT6_64(name,...) __X86_DEFINE_SYSCALL_COMPATn64(name,6,(__VA_ARGS__))
#endif /* CONFIG_SYSCALL_COMPAT */





#ifdef __CC__
DATDEF void *const x86_syscall_router[];
DATDEF void *const x86_xsyscall_router[];
DATDEF u8 const x86_syscall_restart[];
DATDEF u8 const x86_xsyscall_restart[];
#ifndef CONFIG_NO_X86_SYSENTER
DATDEF u8 const x86_syscall_argc[];
DATDEF u8 const x86_xsyscall_argc[];
#endif /* !CONFIG_NO_X86_SYSENTER */
#ifdef CONFIG_SYSCALL_COMPAT
DATDEF void *const x86_syscall_compat_router[];
DATDEF void *const x86_xsyscall_compat_router[];
DATDEF u8 const x86_syscall_compat_restart[];
DATDEF u8 const x86_xsyscall_compat_restart[];
DATDEF u8 const x86_syscall_compat_argc[];
DATDEF u8 const x86_xsyscall_compat_argc[];
#endif /* !CONFIG_SYSCALL_COMPAT */
#endif /* __CC__ */


#ifdef __CC__
struct PACKED syscall_trace_regs {
    struct PACKED {
        syscall_ulong_t      a_arg0;       /* Arg #0 */
        syscall_ulong_t      a_arg1;       /* Arg #1 */
        syscall_ulong_t      a_arg2;       /* Arg #2 */
        syscall_ulong_t      a_arg3;       /* Arg #3 */
        syscall_ulong_t      a_arg4;       /* Arg #4 */
        syscall_ulong_t      a_arg5;       /* Arg #5 */
        syscall_ulong_t      a_sysno;      /* System call number (including special flags). */
    }                        str_args;     /* System call arguments. */
#if !defined(CONFIG_NO_X86_SEGMENTATION) && !defined(__x86_64__)
    struct x86_segments32    str_segments; /* User-space segment registers. */
#endif
#ifdef __x86_64__
    struct x86_irregs64      str_iret;     /* User-space IRET Tail. */
#else
    struct x86_irregs_user32 str_iret;     /* User-space IRET Tail. */
#endif
};


/* Execute a system call using register values found
 * within a `struct cpu_hostcontext_user' structure
 * that is located at the base of the calling thread's
 * kernel stack (iow.: Where the correct IRET tail is stored)
 * This function is used to implement the syscall-through-#PF
 * mechanism, as well as system-call-restarts within `sigreturn'.
 * Upon success, the same register state is updated to contain
 * the new register values, however note that some system calls
 * will throw an `E_INTERRUPT' exception to return to user-space
 * after scheduling an RPC function to-be executed prior to returning.
 * With that in mind, do assume that this function might not return
 * normally. */
FUNDEF void KCALL x86_syscall_exec80(void);

#endif


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_SYSCALL_H */
