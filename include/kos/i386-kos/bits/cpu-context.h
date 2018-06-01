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
#ifndef _KOS_I386_KOS_BITS_CPU_CONTEXT_H
#define _KOS_I386_KOS_BITS_CPU_CONTEXT_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>
#include <hybrid/host.h>
#include <kos/i386-kos/bits/compat.h>

__SYSDECL_BEGIN


/* A new segmentation mode currently in the process of being implemented:
 *   - This mode brings 32-bit X86 a little bit closer to x86_64, in that
 *     only few user-space segment registers are not respected and reset
 *     whenever a system call or interrupt happens.
 *     The only exception to this are VM86 threads.
 *   - Userspace is still able to manually set the base address of the
 *     FS and GS segments, just like it also can in long mode. However,
 *     other segments cannot be used for this. */
#undef CONFIG_X86_FIXED_SEGMENTATION
#define CONFIG_X86_FIXED_SEGMENTATION 1
#undef CONFIG_NO_X86_SEGMENTATION


/*
 * Standard CPU context structures:
 *
 *   - `struct cpu_context'
 *      General-purpose context filled in by `cpu_getcontext',
 *      and used by system calls such as clone(), xunwind(), etc.
 *      WARNING: This structure differs between kernel-space and user-space.
 *
 * Kernel-only CPU context structures:
 *   - `struct cpu_usercontext'
 *      The user-space `struct cpu_context' structure.
 *
 *   - `struct cpu_schedcontext'
 *      The CPU context saved/reloaded by an unscheduled task.
 *
 *   - `struct cpu_hostcontext_user'
 *      A `struct cpu_context'-compatible structure referring to
 *      only the user-space-description portion of the context.
 *
 *   - `struct cpu_anycontext'
 *      Any CPU context, except for `struct cpu_schedcontext'
 *
 */


#ifndef __X86_CONTEXT_SYMBOL
#if !defined(__KERNEL__) && !defined(__EXPOSE_CPU_CONTEXT)
#define __X86_CONTEXT_SYMBOL_HIDDEN
#define __X86_CONTEXT_SYMBOL(x)    __##x
#else
#define __X86_CONTEXT_SYMBOL(x)    x
#endif
#endif /* !__X86_CONTEXT_SYMBOL */




#ifdef __EXPOSE_CPU_COMPAT
#define __X86_DEFINE_GP_REGISTER64_X86_64(prefix,name) \
 union __ATTR_PACKED { \
     __UINT8_TYPE__    __X86_CONTEXT_SYMBOL(prefix ## r ## name ## l); \
     __UINT16_TYPE__   __X86_CONTEXT_SYMBOL(prefix ## r ## name ## w); \
     __ULONG32_TYPE__  __X86_CONTEXT_SYMBOL(prefix ## r ## name ## d); \
     __ULONG64_TYPE__  __X86_CONTEXT_SYMBOL(prefix ## r ## name); \
     __ULONGPTR_TYPE__ __X86_CONTEXT_SYMBOL(prefix ## p ## name); \
 }

#define __X86_DEFINE_GP_REGISTER64_386(prefix,name) \
 union __ATTR_PACKED { \
     __UINT8_TYPE__    __X86_CONTEXT_SYMBOL(prefix ## name ## l); \
     __UINT16_TYPE__   __X86_CONTEXT_SYMBOL(prefix ## name); \
     __ULONG32_TYPE__  __X86_CONTEXT_SYMBOL(prefix ## e ## name); \
     __ULONG64_TYPE__  __X86_CONTEXT_SYMBOL(prefix ## r ## name); \
     __ULONGPTR_TYPE__ __X86_CONTEXT_SYMBOL(prefix ## p ## name); \
 }
#define __X86_DEFINE_GP_REGISTER64_8086(prefix,name) \
 union __ATTR_PACKED { \
     struct __ATTR_PACKED { \
         __UINT8_TYPE__  __X86_CONTEXT_SYMBOL(prefix ## name ## l); \
         __UINT8_TYPE__  __X86_CONTEXT_SYMBOL(prefix ## name ## h); \
     }; \
     __UINT16_TYPE__     __X86_CONTEXT_SYMBOL(prefix ## name ## x); \
     __ULONG32_TYPE__    __X86_CONTEXT_SYMBOL(prefix ## e ## name ## x); \
     __ULONG64_TYPE__    __X86_CONTEXT_SYMBOL(prefix ## r ## name ## x); \
     __ULONGPTR_TYPE__   __X86_CONTEXT_SYMBOL(prefix ## p ## name ## x); \
 }
#endif /* __EXPOSE_CPU_COMPAT */
#ifdef __x86_64__
#define __X86_DEFINE_GP_REGISTER32_386(prefix,name) \
 union __ATTR_PACKED { \
     __UINT16_TYPE__   __X86_CONTEXT_SYMBOL(prefix ## name); \
     __ULONG32_TYPE__  __X86_CONTEXT_SYMBOL(prefix ## e ## name); \
 }
#define __X86_DEFINE_GP_REGISTER32_8086(prefix,name) \
 union __ATTR_PACKED { \
     struct __ATTR_PACKED { \
         __UINT8_TYPE__  __X86_CONTEXT_SYMBOL(prefix ## name ## l); \
         __UINT8_TYPE__  __X86_CONTEXT_SYMBOL(prefix ## name ## h); \
     }; \
     __UINT16_TYPE__     __X86_CONTEXT_SYMBOL(prefix ## name ## x); \
     __ULONG32_TYPE__    __X86_CONTEXT_SYMBOL(prefix ## e ## name ## x); \
 }
#else
#define __X86_DEFINE_GP_REGISTER32_386(prefix,name) \
 union __ATTR_PACKED { \
     __UINT16_TYPE__   __X86_CONTEXT_SYMBOL(prefix ## name); \
     __ULONG32_TYPE__  __X86_CONTEXT_SYMBOL(prefix ## e ## name); \
     __ULONGPTR_TYPE__ __X86_CONTEXT_SYMBOL(prefix ## p ## name); \
 }
#define __X86_DEFINE_GP_REGISTER32_8086(prefix,name) \
 union __ATTR_PACKED { \
     struct __ATTR_PACKED { \
         __UINT8_TYPE__  __X86_CONTEXT_SYMBOL(prefix ## name ## l); \
         __UINT8_TYPE__  __X86_CONTEXT_SYMBOL(prefix ## name ## h); \
     }; \
     __UINT16_TYPE__     __X86_CONTEXT_SYMBOL(prefix ## name ## x); \
     __ULONG32_TYPE__    __X86_CONTEXT_SYMBOL(prefix ## e ## name ## x); \
     __ULONGPTR_TYPE__   __X86_CONTEXT_SYMBOL(prefix ## p ## name ## x); \
 }
#endif



#ifdef __x86_64__
#define x86_segments64       x86_segments
#define x86_gpregs64         x86_gpregs
#define X86_GPREGS_SIZE      X86_GPREGS64_SIZE
#define X86_SEGMENTS_SIZE    X86_SEGMENTS64_SIZE
#define x86_irregs_host      x86_irregs_host64
#define x86_irregs_user      x86_irregs_user64
#define X86_IRREGS_HOST_SIZE X86_IRREGS_HOST64_SIZE
#define X86_IRREGS_USER_SIZE X86_IRREGS_USER64_SIZE
#else
#define x86_segments32       x86_segments
#define x86_gpregs32         x86_gpregs
#define X86_GPREGS_SIZE      X86_GPREGS32_SIZE
#define X86_SEGMENTS_SIZE    X86_SEGMENTS32_SIZE
#define x86_irregs_host32    x86_irregs_host
#define x86_irregs_user32    x86_irregs_user
#define X86_IRREGS_HOST_SIZE X86_IRREGS_HOST32_SIZE
#define X86_IRREGS_USER_SIZE X86_IRREGS_USER32_SIZE
#endif


#ifdef __EXPOSE_CPU_COMPAT
#define X86_GPREGS64_OFFSETOF_R15  0
#define X86_GPREGS64_OFFSETOF_R14  8
#define X86_GPREGS64_OFFSETOF_R13  16
#define X86_GPREGS64_OFFSETOF_R12  24
#define X86_GPREGS64_OFFSETOF_R11  32
#define X86_GPREGS64_OFFSETOF_R10  40
#define X86_GPREGS64_OFFSETOF_R9   48
#define X86_GPREGS64_OFFSETOF_R8   56
#define X86_GPREGS64_OFFSETOF_RDI  64
#define X86_GPREGS64_OFFSETOF_RSI  72
#define X86_GPREGS64_OFFSETOF_RBP  80
#define X86_GPREGS64_OFFSETOF_RBX  88
#define X86_GPREGS64_OFFSETOF_RDX  96
#define X86_GPREGS64_OFFSETOF_RCX  104
#define X86_GPREGS64_OFFSETOF_RAX  112
#define X86_GPREGS64_SIZE          120
#endif /* __EXPOSE_CPU_COMPAT */

#define X86_GPREGS32_OFFSETOF_EDI  0
#define X86_GPREGS32_OFFSETOF_ESI  4
#define X86_GPREGS32_OFFSETOF_EBP  8
#define X86_GPREGS32_OFFSETOF_ESP  12
#define X86_GPREGS32_OFFSETOF_EBX  16
#define X86_GPREGS32_OFFSETOF_EDX  20
#define X86_GPREGS32_OFFSETOF_ECX  24
#define X86_GPREGS32_OFFSETOF_EAX  28
#define X86_GPREGS32_SIZE          32

#ifdef __CC__
#ifdef __EXPOSE_CPU_COMPAT
struct __ATTR_PACKED x86_gpregs64 {
    __X86_DEFINE_GP_REGISTER64_X86_64(gp_,15);   /* General purpose register #15 */
    __X86_DEFINE_GP_REGISTER64_X86_64(gp_,14);   /* General purpose register #14 */
    __X86_DEFINE_GP_REGISTER64_X86_64(gp_,13);   /* General purpose register #13 */
    __X86_DEFINE_GP_REGISTER64_X86_64(gp_,12);   /* General purpose register #12 */
    __X86_DEFINE_GP_REGISTER64_X86_64(gp_,11);   /* General purpose register #11 */
    __X86_DEFINE_GP_REGISTER64_X86_64(gp_,10);   /* General purpose register #10 */
    __X86_DEFINE_GP_REGISTER64_X86_64(gp_,9);    /* General purpose register #9 */
    __X86_DEFINE_GP_REGISTER64_X86_64(gp_,8);    /* General purpose register #8 */
    __X86_DEFINE_GP_REGISTER64_386(gp_,di);      /* Destination pointer */
    __X86_DEFINE_GP_REGISTER64_386(gp_,si);      /* Source pointer */
    __X86_DEFINE_GP_REGISTER64_386(gp_,bp);      /* Frame base pointer */
    __X86_DEFINE_GP_REGISTER64_8086(gp_,b);      /* Base register */
    __X86_DEFINE_GP_REGISTER64_8086(gp_,d);      /* Data register */
    __X86_DEFINE_GP_REGISTER64_8086(gp_,c);      /* Count register */
    __X86_DEFINE_GP_REGISTER64_8086(gp_,a);      /* Accumulator register */
};
#endif /* __EXPOSE_CPU_COMPAT */

struct __ATTR_PACKED x86_gpregs32 {
    __X86_DEFINE_GP_REGISTER32_386(gp_,di);      /* Destination pointer */
    __X86_DEFINE_GP_REGISTER32_386(gp_,si);      /* Source pointer */
    __X86_DEFINE_GP_REGISTER32_386(gp_,bp);      /* Frame base pointer */
    __X86_DEFINE_GP_REGISTER32_386(gp_,sp);      /* Stack pointer */
    __X86_DEFINE_GP_REGISTER32_8086(gp_,b);      /* Base register */
    __X86_DEFINE_GP_REGISTER32_8086(gp_,d);      /* Data register */
    __X86_DEFINE_GP_REGISTER32_8086(gp_,c);      /* Count register */
    __X86_DEFINE_GP_REGISTER32_8086(gp_,a);      /* Accumulator register */
};
#endif /* __CC__ */














#ifdef __EXPOSE_CPU_COMPAT
#define X86_SEGMENTS64_OFFSETOF_GSBASE  0
#define X86_SEGMENTS64_OFFSETOF_FSBASE  8
#define X86_SEGMENTS64_SIZE             16
#endif /* __EXPOSE_CPU_COMPAT */

#define X86_SEGMENTS32_OFFSETOF_GS      0
#define X86_SEGMENTS32_OFFSETOF_FS      4
#ifdef CONFIG_X86_FIXED_SEGMENTATION
#define X86_SEGMENTS32_SIZE             8
#else /* !CONFIG_X86_FIXED_SEGMENTATION */
#define X86_SEGMENTS32_OFFSETOF_ES      8
#define X86_SEGMENTS32_OFFSETOF_DS      12
#define X86_SEGMENTS32_SIZE             16
#endif /* CONFIG_X86_FIXED_SEGMENTATION */

#ifdef __CC__
#ifdef __EXPOSE_CPU_COMPAT
struct __ATTR_PACKED x86_segments64 {
    __ULONG64_TYPE__             __X86_CONTEXT_SYMBOL(sg_gsbase);
    __ULONG64_TYPE__             __X86_CONTEXT_SYMBOL(sg_fsbase);
};
#endif /* __EXPOSE_CPU_COMPAT */
struct __ATTR_PACKED x86_segments32 {
    __ULONG32_TYPE__             __X86_CONTEXT_SYMBOL(sg_gs); /* G segment register */
    __ULONG32_TYPE__             __X86_CONTEXT_SYMBOL(sg_fs); /* F segment register */
#ifndef CONFIG_X86_FIXED_SEGMENTATION
    __ULONG32_TYPE__             __X86_CONTEXT_SYMBOL(sg_es); /* E (source) segment register */
    __ULONG32_TYPE__             __X86_CONTEXT_SYMBOL(sg_ds); /* D (destination) segment register */
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
};
#endif /* __CC__ */



















#ifdef __EXPOSE_CPU_COMPAT
#define X86_IRREGS64_OFFSETOF_RIP      0
#define X86_IRREGS64_OFFSETOF_CS       8
#define X86_IRREGS64_OFFSETOF_RFLAGS  16
#define X86_IRREGS64_OFFSETOF_USERRSP 24
#define X86_IRREGS64_OFFSETOF_SS      32
#define X86_IRREGS64_SIZE             40
#define x86_irregs_user64             x86_irregs64
#define x86_irregs_host64             x86_irregs64
#define X86_IRREGS_USER64_SIZE        X86_IRREGS64_SIZE
#define X86_IRREGS_HOST64_SIZE        X86_IRREGS64_SIZE
#ifdef __CC__
struct __ATTR_PACKED x86_irregs64 {
    __X86_DEFINE_GP_REGISTER64_386(ir_,ip);       /* Instruction pointer */
    __ULONG64_TYPE__ __X86_CONTEXT_SYMBOL(ir_cs); /* Code segment */
    __X86_DEFINE_GP_REGISTER64_386(ir_,flags);    /* Flags register */
    __X86_DEFINE_GP_REGISTER64_386(ir_,sp);       /* Stack pointer */
    __ULONG64_TYPE__ __X86_CONTEXT_SYMBOL(ir_ss); /* Stack segment */
};
#endif /* __CC__ */
#endif /* __EXPOSE_CPU_COMPAT */

#define X86_IRREGS32_OFFSETOF_EIP      0
#define X86_IRREGS32_OFFSETOF_CS       4
#define X86_IRREGS32_OFFSETOF_EFLAGS   8
#define X86_IRREGS32_OFFSETOF_USERESP 12
#define X86_IRREGS32_OFFSETOF_SS      16
#define X86_IRREGS32_SIZEOF_HOST      12
#define X86_IRREGS32_SIZEOF_USER      20

#define X86_IRREGS_HOST32_OFFSETOF_EIP     0
#define X86_IRREGS_HOST32_OFFSETOF_CS      4
#define X86_IRREGS_HOST32_OFFSETOF_EFLAGS  8
#define X86_IRREGS_HOST32_SIZE            12
#ifdef __CC__
struct __ATTR_PACKED x86_irregs_host32 {
    __X86_DEFINE_GP_REGISTER32_386(ir_,ip);       /* Instruction pointer */
    __ULONG32_TYPE__ __X86_CONTEXT_SYMBOL(ir_cs); /* Code segment */
    __X86_DEFINE_GP_REGISTER32_386(ir_,flags);    /* Flags register */
};
#endif

#define X86_IRREGS_USER32_OFFSETOF_EIP      0
#define X86_IRREGS_USER32_OFFSETOF_CS       4
#define X86_IRREGS_USER32_OFFSETOF_EFLAGS   8
#define X86_IRREGS_USER32_OFFSETOF_USERESP 12
#define X86_IRREGS_USER32_OFFSETOF_SS      16
#define X86_IRREGS_USER32_SIZE             20
#ifdef __CC__
struct __ATTR_PACKED x86_irregs_user32 {
    union __ATTR_PACKED {
        struct x86_irregs_host32 __X86_CONTEXT_SYMBOL(ir_host);    /* Host iret tail */
        struct __ATTR_PACKED {
            __X86_DEFINE_GP_REGISTER32_386(ir_,ip);       /* Instruction pointer */
            __ULONG32_TYPE__ __X86_CONTEXT_SYMBOL(ir_cs); /* Code segment */
            __X86_DEFINE_GP_REGISTER32_386(ir_,flags);    /* Flags register */
        };
    };
    __X86_DEFINE_GP_REGISTER32_386(ir_user,sp);           /* Stack pointer */
    __ULONG32_TYPE__ __X86_CONTEXT_SYMBOL(ir_ss);         /* Stack segment */
};
#endif /* __CC__ */







#define __CPU_CONTEXT_SIZE X86_CONTEXT_SIZE
#if defined(__KERNEL__) || defined(__USE_KOS)
#define CPU_CONTEXT_SIZE __CPU_CONTEXT_SIZE
#endif


#ifdef __CC__
#define __cpu_context_defined 1
#define __x86_context_defined 1
#define x86_context        cpu_context
#ifdef __x86_64__
#define __cpu_anycontext_defined   1
#define __x86_anycontext_defined   1
#define __x86_context64_defined    1
#define __x86_anycontext64_defined 1
#define cpu_anycontext     cpu_context
#define x86_context64      cpu_context
#define x86_anycontext     cpu_context
#define x86_anycontext64   cpu_context
#ifdef __KERNEL__
#define __cpu_usercontext_defined    1
#define __x86_usercontext_defined    1
#define __x86_usercontext64_defined  1
#define x86_usercontext    cpu_usercontext
#define x86_usercontext64  cpu_usercontext
#define __x86_schedcontext_defined   1
#define __x86_schedcontext64_defined 1
#define __cpu_schedcontext_defined   1
#define x86_schedcontext   cpu_schedcontext
#define x86_schedcontext64 cpu_schedcontext
#endif
#else
#define __cpu_anycontext_defined   1
#define __x86_anycontext_defined   1
#define __x86_context32_defined    1
#define __x86_anycontext32_defined 1
#define x86_anycontext     cpu_anycontext
#define x86_anycontext32   cpu_anycontext
#define x86_context32      cpu_context
#ifdef __KERNEL__
#define __cpu_usercontext_defined   1
#define __x86_usercontext_defined   1
#define __x86_usercontext32_defined 1
#define x86_usercontext    cpu_usercontext
#define x86_usercontext32  cpu_usercontext
#define __x86_schedcontext_defined   1
#define __x86_schedcontext64_defined 1
#define __cpu_schedcontext_defined   1
#define cpu_schedcontext   cpu_anycontext
#define x86_schedcontext   cpu_anycontext
#define x86_schedcontext32 cpu_anycontext
#endif
#endif

#ifdef __KERNEL__
#define __x86_hostcontext_host32_defined 1
#define __x86_context32_defined          1
#define x86_hostcontext_host32  x86_context32
#ifdef __x86_64__
#define __cpu_hostcontext_user_defined   1
#define cpu_hostcontext_user    x86_context64
#else
#define __cpu_hostcontext_user_defined   1
#define __x86_hostcontext_user32_defined 1
#define x86_hostcontext_user32  cpu_hostcontext_user
#endif
#endif
#endif /* __CC__ */

#ifdef __x86_64__
#define X86_CONTEXT_OFFSETOF_GPREGS   X86_CONTEXT64_OFFSETOF_GPREGS
#define X86_CONTEXT_OFFSETOF_SEGMENTS X86_CONTEXT64_OFFSETOF_SEGMENTS
#define X86_CONTEXT_OFFSETOF_IRET     X86_CONTEXT64_OFFSETOF_IRET
#define X86_CONTEXT_SIZE              X86_CONTEXT64_SIZE
#else /* __x86_64__ */
#ifdef __KERNEL__
#define X86_CONTEXT_OFFSETOF_GPREGS   X86_HOSTCONTEXT_HOST32_OFFSETOF_GPREGS
#define X86_CONTEXT_OFFSETOF_ESP      X86_HOSTCONTEXT_HOST32_OFFSETOF_ESP
#ifndef CONFIG_NO_X86_SEGMENTATION
#define X86_CONTEXT_OFFSETOF_SEGMENTS X86_HOSTCONTEXT_HOST32_OFFSETOF_SEGMENTS
#endif /* !CONFIG_NO_X86_SEGMENTATION */
#define X86_CONTEXT_OFFSETOF_IRET     X86_HOSTCONTEXT_HOST32_OFFSETOF_IRET
#define X86_CONTEXT_OFFSETOF_EIP      X86_HOSTCONTEXT_HOST32_OFFSETOF_EIP
#define X86_CONTEXT_OFFSETOF_EFLAGS   X86_HOSTCONTEXT_HOST32_OFFSETOF_EFLAGS
#define X86_CONTEXT_SIZE              X86_HOSTCONTEXT_HOST32_SIZE
#else /* __KERNEL__ */
#define X86_CONTEXT_OFFSETOF_GPREGS   X86_CONTEXT32_OFFSETOF_GPREGS
#define X86_CONTEXT_OFFSETOF_ESP      X86_CONTEXT32_OFFSETOF_ESP
#define X86_CONTEXT_OFFSETOF_SEGMENTS X86_CONTEXT32_OFFSETOF_SEGMENTS
#define X86_CONTEXT_OFFSETOF_EIP      X86_CONTEXT32_OFFSETOF_EIP
#define X86_CONTEXT_OFFSETOF_EFLAGS   X86_CONTEXT32_OFFSETOF_EFLAGS
#define X86_CONTEXT_SIZE              X86_CONTEXT32_SIZE
#endif /* !__KERNEL__ */
#endif /* !__x86_64__ */


#ifdef __KERNEL__
#define X86_CONTEXT32_OFFSETOF_GPREGS   X86_HOSTCONTEXT_HOST32_OFFSETOF_GPREGS
#define X86_CONTEXT32_OFFSETOF_ESP      X86_HOSTCONTEXT_HOST32_OFFSETOF_ESP
#ifndef CONFIG_NO_X86_SEGMENTATION
#define X86_CONTEXT32_OFFSETOF_SEGMENTS X86_HOSTCONTEXT_HOST32_OFFSETOF_SEGMENTS
#endif /* !CONFIG_NO_X86_SEGMENTATION */
#define X86_CONTEXT32_OFFSETOF_IRET     X86_HOSTCONTEXT_HOST32_OFFSETOF_IRET
#define X86_CONTEXT32_OFFSETOF_EIP      X86_HOSTCONTEXT_HOST32_OFFSETOF_EIP
#define X86_CONTEXT32_OFFSETOF_EFLAGS   X86_HOSTCONTEXT_HOST32_OFFSETOF_EFLAGS
#define X86_CONTEXT32_SIZE              X86_HOSTCONTEXT_HOST32_SIZE
#endif

#ifdef __x86_64__
#define __CPU_CONTEXT_IP(x) ((x).c_rip)
#define __CPU_CONTEXT_SP(x) ((x).c_rsp)
#else
#define __CPU_CONTEXT_IP(x) ((x).c_eip)
#define __CPU_CONTEXT_SP(x) ((x).c_esp)
#endif

#ifndef __X86_CONTEXT_SYMBOL_HIDDEN
#define CPU_CONTEXT_IP(x) __CPU_CONTEXT_IP(x)
#define CPU_CONTEXT_SP(x) __CPU_CONTEXT_SP(x)
#endif


#ifdef __EXPOSE_CPU_COMPAT
#define X86_CONTEXT64_OFFSETOF_GPREGS    0
#ifdef __KERNEL__
#define X86_CONTEXT64_OFFSETOF_IRET          X86_GPREGS64_SIZE
#define X86_CONTEXT64_OFFSETOF_RIP           X86_GPREGS64_SIZE
#define X86_CONTEXT64_OFFSETOF_CS           (X86_GPREGS64_SIZE+8)
#define X86_CONTEXT64_OFFSETOF_RFLAGS       (X86_GPREGS64_SIZE+16)
#define X86_CONTEXT64_OFFSETOF_RSP          (X86_GPREGS64_SIZE+24)
#define X86_CONTEXT64_OFFSETOF_SS           (X86_GPREGS64_SIZE+32)
#define X86_CONTEXT64_SIZE                  (X86_GPREGS64_SIZE+X86_IRREGS64_SIZE)
#define X86_USERCONTEXT64_OFFSETOF_SEGMENTS  X86_GPREGS64_SIZE
#define X86_USERCONTEXT64_OFFSETOF_IRET     (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE)
#define X86_USERCONTEXT64_OFFSETOF_RIP      (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE)
#define X86_USERCONTEXT64_OFFSETOF_CS       (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE+8)
#define X86_USERCONTEXT64_OFFSETOF_RFLAGS   (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE+16)
#define X86_USERCONTEXT64_OFFSETOF_RSP      (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE+24)
#define X86_USERCONTEXT64_OFFSETOF_SS       (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE+32)
#define X86_USERCONTEXT64_SIZE              (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE+X86_IRREGS64_SIZE)
#else
#define X86_CONTEXT64_OFFSETOF_SEGMENTS  X86_GPREGS64_SIZE
#define X86_CONTEXT64_OFFSETOF_IRET     (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE)
#define X86_CONTEXT64_OFFSETOF_RIP      (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE)
#define X86_CONTEXT64_OFFSETOF_CS       (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE+8)
#define X86_CONTEXT64_OFFSETOF_RFLAGS   (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE+16)
#define X86_CONTEXT64_OFFSETOF_RSP      (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE+24)
#define X86_CONTEXT64_OFFSETOF_SS       (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE+32)
#define X86_CONTEXT64_SIZE              (X86_GPREGS64_SIZE+X86_SEGMENTS64_SIZE+X86_IRREGS64_SIZE)
#endif

#ifndef x86_context64
#define __x86_anycontext64_defined 1
#define x86_context64      x86_anycontext64
#endif
#define __x86_context64_defined 1
#ifdef __CC__
struct __ATTR_PACKED x86_context64 {
    /* CPU Context: host --> host  (As seen in kernel exception handling / during task peemption) */
    struct x86_gpregs64          __X86_CONTEXT_SYMBOL(c_gpregs);         /* General purpose registers */
#ifndef __KERNEL__
    struct x86_segments64        __X86_CONTEXT_SYMBOL(c_segments);       /* Segment registers */
#ifdef CONFIG_X86_FIXED_SEGMENTATION
    __X86_DEFINE_GP_REGISTER64_386(c_,flags);                            /* Flags register */
    __X86_DEFINE_GP_REGISTER64_386(c_,sp);                               /* Stack pointer */
    __X86_DEFINE_GP_REGISTER64_386(c_,ip);                               /* Instruction pointer */
#else /* CONFIG_X86_FIXED_SEGMENTATION */
    union __ATTR_PACKED {
        struct x86_irregs64      __X86_CONTEXT_SYMBOL(c_iret);           /* IRet registers */
        struct __ATTR_PACKED {
            __X86_DEFINE_GP_REGISTER64_386(c_,ip);                       /* Instruction pointer */
            __ULONG64_TYPE__     __X86_CONTEXT_SYMBOL(c_cs);             /* Code segment */
            __X86_DEFINE_GP_REGISTER64_386(c_,flags);                    /* Flags register */
            __X86_DEFINE_GP_REGISTER64_386(c_,sp);                       /* Stack pointer */
            __ULONG64_TYPE__     __X86_CONTEXT_SYMBOL(c_ss);             /* Stack segment */
        };
    };
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
#else /* !__KERNEL__ */
    union __ATTR_PACKED {
        struct x86_irregs64      __X86_CONTEXT_SYMBOL(c_iret);           /* IRet registers */
        struct __ATTR_PACKED {
            __X86_DEFINE_GP_REGISTER64_386(c_,ip);                       /* Instruction pointer */
            __ULONG64_TYPE__     __X86_CONTEXT_SYMBOL(c_cs);             /* Code segment */
            __X86_DEFINE_GP_REGISTER64_386(c_,flags);                    /* Flags register */
            __X86_DEFINE_GP_REGISTER64_386(c_,sp);                       /* Stack pointer */
            __ULONG64_TYPE__     __X86_CONTEXT_SYMBOL(c_ss);             /* Stack segment */
        };
    };
#endif /* __KERNEL__ */
};
#endif /* __CC__ */
#ifdef __KERNEL__
#define X86_SCHEDCONTEXT64_OFFSETOF_GPREGS     0
#define X86_SCHEDCONTEXT64_OFFSETOF_IRET       X86_GPREGS64_SIZE
#define X86_SCHEDCONTEXT64_OFFSETOF_SEGMENTS  (X86_GPREGS64_SIZE+X86_IRREGS64_SIZE)
#define X86_SCHEDCONTEXT64_SIZE               (X86_GPREGS64_SIZE+X86_IRREGS64_SIZE+X86_SEGMENTS64_SIZE)
#ifdef __CC__
struct __ATTR_PACKED x86_schedcontext64 {
    /* While not scheduled, a 64-bit context also
     * contains the user-space FS/GS_base registers.
     * However, in order to remain compatible with the
     * kernel-space `struct x86_context64', as well as
     * be able to be cast to that structure, the FS/GS
     * registers are stored after the IRET tail, contrary
     * to the layout seen in the `struct x86_usercontext64'. */
    struct x86_gpregs64          __X86_CONTEXT_SYMBOL(c_gpregs);         /* General purpose registers */
    union __ATTR_PACKED {
        struct x86_irregs64      __X86_CONTEXT_SYMBOL(c_iret);           /* IRet registers */
        struct __ATTR_PACKED {
            __X86_DEFINE_GP_REGISTER64_386(c_,ip);                       /* Instruction pointer */
            __ULONG64_TYPE__     __X86_CONTEXT_SYMBOL(c_cs);             /* Code segment */
            __X86_DEFINE_GP_REGISTER64_386(c_,flags);                    /* Flags register */
            __X86_DEFINE_GP_REGISTER64_386(c_,sp);                       /* Stack pointer */
            __ULONG64_TYPE__     __X86_CONTEXT_SYMBOL(c_ss);             /* Stack segment */
        };
    };
    struct x86_segments64        __X86_CONTEXT_SYMBOL(c_segments);       /* Segment registers */
};
struct __ATTR_PACKED x86_usercontext64 {
    struct x86_gpregs64          __X86_CONTEXT_SYMBOL(c_gpregs);         /* General purpose registers */
    struct x86_segments64        __X86_CONTEXT_SYMBOL(c_segments);       /* Segment registers */
#ifdef CONFIG_X86_FIXED_SEGMENTATION
    __X86_DEFINE_GP_REGISTER64_386(c_,flags);                            /* Flags register */
    __X86_DEFINE_GP_REGISTER64_386(c_,sp);                               /* Stack pointer */
    __X86_DEFINE_GP_REGISTER64_386(c_,ip);                               /* Instruction pointer */
#else /* CONFIG_X86_FIXED_SEGMENTATION */
    union __ATTR_PACKED {
        struct x86_irregs64      __X86_CONTEXT_SYMBOL(c_iret);           /* IRet registers */
        struct __ATTR_PACKED {
            __X86_DEFINE_GP_REGISTER64_386(c_,ip);                       /* Instruction pointer */
            __ULONG64_TYPE__     __X86_CONTEXT_SYMBOL(c_cs);             /* Code segment */
            __X86_DEFINE_GP_REGISTER64_386(c_,flags);                    /* Flags register */
            __X86_DEFINE_GP_REGISTER64_386(c_,sp);                       /* Stack pointer */
            __ULONG64_TYPE__     __X86_CONTEXT_SYMBOL(c_ss);             /* Stack segment */
        };
    };
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
};
#endif /* __CC__ */
#endif /* __KERNEL__ */
#endif /* __EXPOSE_CPU_COMPAT */

#ifdef __KERNEL__
#define X86_USERCONTEXT32_OFFSETOF_GPREGS    0
#define X86_USERCONTEXT32_OFFSETOF_ESP       X86_GPREGS32_OFFSETOF_ESP
#define X86_USERCONTEXT32_OFFSETOF_SEGMENTS  X86_GPREGS32_SIZE
#define X86_USERCONTEXT32_OFFSETOF_EIP      (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE)
#define X86_USERCONTEXT32_OFFSETOF_EFLAGS   (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+4)
#ifdef CONFIG_X86_FIXED_SEGMENTATION
#define X86_USERCONTEXT32_SIZE              (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+8)
#else
#define X86_USERCONTEXT32_OFFSETOF_CS       (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+8)
#define X86_USERCONTEXT32_OFFSETOF_SS       (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+12)
#define X86_USERCONTEXT32_SIZE              (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+16)
#endif
#else
#define X86_CONTEXT32_OFFSETOF_GPREGS        0
#define X86_CONTEXT32_OFFSETOF_ESP           X86_GPREGS32_OFFSETOF_ESP
#define X86_CONTEXT32_OFFSETOF_SEGMENTS      X86_GPREGS32_SIZE
#define X86_CONTEXT32_OFFSETOF_EIP          (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE)
#define X86_CONTEXT32_OFFSETOF_EFLAGS       (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+4)
#ifdef CONFIG_X86_FIXED_SEGMENTATION
#define X86_CONTEXT32_SIZE                  (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+8)
#else
#define X86_CONTEXT32_OFFSETOF_CS           (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+8)
#define X86_CONTEXT32_OFFSETOF_SS           (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+12)
#define X86_CONTEXT32_SIZE                  (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+16)
#endif
#endif

#ifdef __CC__
#ifdef __KERNEL__
#define __x86_usercontext32_defined 1
struct __ATTR_PACKED x86_usercontext32
#else
#define __x86_context32_defined 1
struct __ATTR_PACKED x86_context32
#endif
{   /* CPU Context: user --> user  (As seen in user exception handling) */
    union __ATTR_PACKED {
        struct x86_gpregs32      __X86_CONTEXT_SYMBOL(c_gpregs);         /* General purpose registers */
        struct __ATTR_PACKED {
             __ULONG32_TYPE__    __c_pad1[X86_GPREGS32_OFFSETOF_ESP/4];  /* ... */
             __X86_DEFINE_GP_REGISTER32_386(c_,sp);                      /* Stack pointer */
        };
    };
    struct x86_segments32        __X86_CONTEXT_SYMBOL(c_segments);       /* Segment registers
                                                                          * NOTE: Unless the kernel was built without `CONFIG_NO_X86_SEGMENTATION',
                                                                          *       these registers are ignored unless constructing a vm86 thread.
                                                                          * NOTE: Registers specified as ZERO(0) will default to their standard
                                                                          *       values described by the `X86_USER_DS' / `X86_USER_TLS' constants. */
    __X86_DEFINE_GP_REGISTER32_386(c_,ip);                               /* Instruction pointer */
    __X86_DEFINE_GP_REGISTER32_386(c_,flags);                            /* Flags register */
#ifndef CONFIG_X86_FIXED_SEGMENTATION
    __ULONG32_TYPE__             __X86_CONTEXT_SYMBOL(c_cs);             /* Code segment (When specified as ZERO(0), default to `X86_USER_CS') */
    __ULONG32_TYPE__             __X86_CONTEXT_SYMBOL(c_ss);             /* Stack segment (When specified as ZERO(0), default to `X86_USER_DS') */
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
};
#endif /* __CC__ */


#ifdef __KERNEL__
#define X86_HOSTCONTEXT_HOST32_OFFSETOF_GPREGS    0
#define X86_HOSTCONTEXT_HOST32_OFFSETOF_ESP       X86_GPREGS32_OFFSETOF_ESP
#ifndef CONFIG_NO_X86_SEGMENTATION
#define X86_HOSTCONTEXT_HOST32_OFFSETOF_SEGMENTS  X86_GPREGS32_SIZE
#define X86_HOSTCONTEXT_HOST32_OFFSETOF_IRET     (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE)
#define X86_HOSTCONTEXT_HOST32_OFFSETOF_EIP      (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE)
#define X86_HOSTCONTEXT_HOST32_OFFSETOF_EFLAGS   (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+8)
#define X86_HOSTCONTEXT_HOST32_SIZE              (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+X86_IRREGS_HOST32_SIZE)
#else /* !CONFIG_NO_X86_SEGMENTATION */
#define X86_HOSTCONTEXT_HOST32_OFFSETOF_IRET      X86_GPREGS32_SIZE
#define X86_HOSTCONTEXT_HOST32_OFFSETOF_EIP      (X86_GPREGS32_SIZE)
#define X86_HOSTCONTEXT_HOST32_OFFSETOF_EFLAGS   (X86_GPREGS32_SIZE+8)
#define X86_HOSTCONTEXT_HOST32_SIZE              (X86_GPREGS32_SIZE+X86_IRREGS_HOST32_SIZE)
#endif /* CONFIG_NO_X86_SEGMENTATION */
#ifdef __CC__
#define __x86_hostcontext_host32_defined 1
struct __ATTR_PACKED x86_hostcontext_host32 {
    /* CPU Context: host --> host  (As seen in kernel exception handling / during task peemption) */
    union __ATTR_PACKED {
        struct x86_gpregs32      __X86_CONTEXT_SYMBOL(c_gpregs);         /* General purpose registers */
        struct __ATTR_PACKED {
             __ULONG32_TYPE__    __c_pad1[X86_GPREGS32_OFFSETOF_ESP/4];  /* ... */
             __X86_DEFINE_GP_REGISTER32_386(c_,sp);                      /* Stack pointer */
        };
    };
#ifndef CONFIG_NO_X86_SEGMENTATION
    struct x86_segments32        __X86_CONTEXT_SYMBOL(c_segments);       /* Segment registers */
#endif /* !CONFIG_NO_X86_SEGMENTATION */
    union __ATTR_PACKED {
        struct x86_irregs_host32 __X86_CONTEXT_SYMBOL(c_iret);           /* IRet registers */
        struct __ATTR_PACKED {
             __X86_DEFINE_GP_REGISTER32_386(c_,ip);                      /* Instruction pointer */
             __ULONG32_TYPE__    __c_pad;                                /* ... */
             __X86_DEFINE_GP_REGISTER32_386(c_,flags);                   /* Flags register */
        };
    };
};
#endif /* __CC__ */

#define X86_HOSTCONTEXT_USER32_OFFSETOF_HOST       0
#define X86_HOSTCONTEXT_USER32_OFFSETOF_GPREGS     0
#ifndef CONFIG_NO_X86_SEGMENTATION
#define X86_HOSTCONTEXT_USER32_OFFSETOF_SEGMENTS   X86_GPREGS32_SIZE
#define X86_HOSTCONTEXT_USER32_OFFSETOF_IRET      (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE)
#define X86_HOSTCONTEXT_USER32_OFFSETOF_EIP       (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE)
#define X86_HOSTCONTEXT_USER32_OFFSETOF_EFLAGS    (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+8)
#define X86_HOSTCONTEXT_USER32_OFFSETOF_ESP       (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+8)
#define X86_HOSTCONTEXT_USER32_SIZE               (X86_GPREGS32_SIZE+X86_SEGMENTS32_SIZE+X86_IRREGS_USER32_SIZE)
#else /* !CONFIG_NO_X86_SEGMENTATION */
#define X86_HOSTCONTEXT_USER32_OFFSETOF_IRET       X86_GPREGS32_SIZE
#define X86_HOSTCONTEXT_USER32_OFFSETOF_EIP       (X86_GPREGS32_SIZE)
#define X86_HOSTCONTEXT_USER32_OFFSETOF_EFLAGS    (X86_GPREGS32_SIZE+8)
#define X86_HOSTCONTEXT_USER32_OFFSETOF_ESP       (X86_GPREGS32_SIZE+8)
#define X86_HOSTCONTEXT_USER32_SIZE               (X86_GPREGS32_SIZE+X86_IRREGS_USER32_SIZE)
#endif /* CONFIG_NO_X86_SEGMENTATION */
#ifdef __CC__
#define __x86_hostcontext_user32_defined 1
struct __ATTR_PACKED x86_hostcontext_user32 {
    /* CPU Context: host --> user  (As seen during task peemption) */
    struct x86_gpregs32              __X86_CONTEXT_SYMBOL(c_gpregs);   /* General purpose registers */
#ifndef CONFIG_NO_X86_SEGMENTATION
    struct x86_segments32            __X86_CONTEXT_SYMBOL(c_segments); /* Segment registers */
#endif /* !CONFIG_NO_X86_SEGMENTATION */
    union __ATTR_PACKED {
        struct x86_irregs_user32     __X86_CONTEXT_SYMBOL(c_iret);     /* IRet registers */
        struct __ATTR_PACKED {
             __X86_DEFINE_GP_REGISTER32_386(c_,ip);                    /* Instruction pointer */
             __ULONG32_TYPE__        __c_pad;                          /* ... */
             __X86_DEFINE_GP_REGISTER32_386(c_,flags);                 /* Flags register */
             __X86_DEFINE_GP_REGISTER32_386(c_,sp);                    /* Stack pointer */
        };
    };
};
#define __x86_anycontext32_defined 1
struct __ATTR_PACKED x86_anycontext32 {
    /* CPU Context: host --> host / user
     * This context is used to represent what is used during
     * preemption, or interrupt handling, as such cases must
     * be handled when originating both from kernel-space, or
     * for user-space.
     * Use the macros below to check what the context is
     * describing, as well as gain access to the correct ESP.
     */
    union __ATTR_PACKED {
        struct x86_hostcontext_host32        __X86_CONTEXT_SYMBOL(c_host);     /* 32-bit HOST cpu context. */
        struct x86_hostcontext_user32        __X86_CONTEXT_SYMBOL(c_user);     /* 32-bit USER cpu context. */
        struct __ATTR_PACKED {
            union __ATTR_PACKED {
                struct x86_gpregs32          __X86_CONTEXT_SYMBOL(c_gpregs);   /* General purpose registers */
                struct __ATTR_PACKED {
                     __ULONG32_TYPE__        __c_pad1[X86_GPREGS32_OFFSETOF_ESP/4]; /* ... */
                     __X86_DEFINE_GP_REGISTER32_386(c_host,sp);                /* [valid_if(X86_ANYCONTEXT_ISHOST(self))] Host stack pointer */
                };
            };
#ifndef CONFIG_NO_X86_SEGMENTATION
            struct x86_segments32            __X86_CONTEXT_SYMBOL(c_segments); /* Segment registers */
#endif /* !CONFIG_NO_X86_SEGMENTATION */
            union __ATTR_PACKED {
                struct x86_irregs_user32     __X86_CONTEXT_SYMBOL(c_iret);     /* IRet registers */
                struct __ATTR_PACKED {
                     __X86_DEFINE_GP_REGISTER32_386(c_,ip);                    /* Instruction pointer */
                     __ULONG32_TYPE__        __c_pad;                          /* ... */
                     __X86_DEFINE_GP_REGISTER32_386(c_,flags);                 /* Flags register */
                     __X86_DEFINE_GP_REGISTER32_386(c_user,sp);                /* [valid_if(X86_ANYCONTEXT_ISUSER(self))] User stack pointer */
                };
            };
        };
    };
};

#if !defined(__x86_64__) && !defined(CONFIG_NO_VM86)
#define X86_ANYCONTEXT_ISUSER(x)  (((x).c_iret.ir_cs&3) || ((x).c_iret.ir_eflags&0x20000))
#define X86_ANYCONTEXT_ISHOST(x)   (!X86_ANYCONTEXT_ISUSER(x))
#define X86_ANYCONTEXT32_ESP(x)   (*(X86_ANYCONTEXT_ISUSER(x) ? &(x).c_useresp : &(x).c_hostesp))
#else
#define X86_ANYCONTEXT_ISUSER(x)   ((x).c_iret.ir_cs&3)
#define X86_ANYCONTEXT_ISHOST(x) (!((x).c_iret.ir_cs&3))
#define X86_ANYCONTEXT32_ESP(x)   (*(X86_ANYCONTEXT_ISUSER(x) ? &(x).c_useresp : &(x).c_hostesp))
#endif
#ifdef __x86_64__
#define X86_ANYCONTEXT_USERSP(c)    ((c).c_rsp)
#define X86_ANYCONTEXT_HOSTSP(c)    ((c).c_rsp)
#else
#define X86_ANYCONTEXT_USERSP(c)    ((c).c_useresp)
#define X86_ANYCONTEXT_HOSTSP(c)    ((c).c_hostesp)
#endif

#endif /* __CC__ */

#ifdef __x86_64__
#define __CPU_USERCONTEXT_SIZE  X86_USERCONTEXT64_SIZE
#else
#define __CPU_USERCONTEXT_SIZE  X86_USERCONTEXT32_SIZE
#endif

#endif /* __KERNEL__ */

__SYSDECL_END

#endif /* !_KOS_I386_KOS_BITS_CPU_CONTEXT_H */
