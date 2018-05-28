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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_GDT_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_GDT_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/sections.h>
#include <assert.h>
#include <kos/i386-kos/asm/tls.h>
#ifdef __x86_64__
#include <asm/cpu-flags.h>
#include <kos/intrin.h>
#endif

DECL_BEGIN

#define X86_SEGMENT_OFFSETOF_BASELO 2
#define X86_SEGMENT_OFFSETOF_BASEHI 7
#ifdef __x86_64__
#define X86_SEGMENT_OFFSETOF_BASEUP 8
#endif

#ifdef __CC__
/* Segment Descriptor / GDT (Global Descriptor Table) Entry
 * NOTE: Another valid name of this would be `gdtentry' or `ldtentry' */
struct PACKED x86_segment {
    union PACKED {
#ifdef __x86_64__
        struct PACKED {      u64 s_ul,s_uh; };
        struct PACKED {      s64 s_sl,s_sh; };
#else
        struct PACKED {      u32 s_ul,s_uh; };
        struct PACKED {      s32 s_sl,s_sh; };
        s64                      s_s;
        u64                      s_u;
#endif
        struct PACKED {      u32 s_ul32,s_uh32; };
        struct PACKED {      s32 s_sl32,s_sh32; };
        struct PACKED {
            u16                  s_sizelow;
            unsigned int         s_baselow:    24; /* Bits 0..23 of the base address. */
            union PACKED {
                u8               s_access;
                struct PACKED {
                    unsigned int s_type:     4;
                    unsigned int __unnamed2: 4;
                };
                struct PACKED {
                    /* Just set to 0. The CPU sets this to 1 when the x86_segment is accessed. */
                    unsigned int s_accessed:    1; /* Accessed bit. */
                    /* Readable bit for code selectors:
                     *   Whether read access for this x86_segment is allowed.
                     *   Write access is never allowed for code segments.
                     * Writable bit for data selectors:
                     *   Whether write access for this x86_segment is allowed.
                     *   Read access is always allowed for data segments. */
                    unsigned int s_rw:          1; /* Also the busy bit of tasks. */
                    /* Direction bit for data selectors:
                     *   Tells the direction. 0 the x86_segment grows up. 1 the x86_segment
                     *   grows down, ie. the offset has to be greater than the limit.
                     * Conforming bit for code selectors:
                     *     If 1: code in this x86_segment can be executed from an equal or lower
                     *           privilege level. For example, code in ring 3 can far-jump to
                     *           conforming code in a ring 2 x86_segment. The s_privl-bits represent
                     *           the highest privilege level that is allowed to execute the x86_segment.
                     *           For example, code in ring 0 cannot far-jump to a conforming code
                     *           x86_segment with s_privl==0x2, while code in ring 2 and 3 can. Note that
                     *           the privilege level remains the same, ie. a far-jump form ring
                     *           3 to a s_privl==2-x86_segment remains in ring 3 after the jump.
                     *     If 0: code in this x86_segment can only be executed from the ring set in s_privl. */
                    unsigned int s_dc:          1; /* Direction bit/Conforming bit. */
                    unsigned int s_execute:     1; /* Executable bit. If 1 code in this x86_segment can be executed, ie. a code selector. If 0 it is a data selector. */
                    unsigned int s_system:      1; /* 0: System descriptor; 1: Code/Data/Stack (always set to 1) */
                    unsigned int s_privl:       2; /* Privilege, 2 bits. Contains the ring level, 0 = highest (kernel), 3 = lowest (user applications). */
                    unsigned int s_present:     1; /* Present bit. This must be 1 for all valid selectors. */
                };
            };
            union PACKED {
                struct PACKED {
                    unsigned int __unnamed1:    4;
                    unsigned int s_flags:       4;
                };
                struct PACKED {
                    unsigned int s_sizehigh:    4;
                    unsigned int s_available:   1; /* Availability bit (Set to 1). */
                    unsigned int s_longmode:    1; /* Set to 0 (Used in x86_64). */
                    /* Indicates how the instructions(80386 and up) access register and memory data in protected mode.
                     * If 0: instructions are 16-bit instructions, with 16-bit offsets and 16-bit registers.
                     *       Stacks are assumed 16-bit wide and SP is used.
                     *       Must be set to 1 in x86_64 mode
                     * If 1: 32-bits are assumed.
                     * Allows 8086-80286 programs to run. */
                    unsigned int s_dbbit:       1;
                    /* If 0: segments can be 1 byte to 1MB in length.
                     * If 1: segments can be 4KB to 4GB in length. */
                    unsigned int s_granularity: 1;
                };
            };
            u8                   s_basehigh;  /* Bits 24..31 of the base address. */
#ifdef __x86_64__
            /* NOTE: Documentation on additions in 64-bit mode can be found here:
             * https://xem.github.io/minix86/manual/intel-x86-and-64-manual-vol3/o_fe12b1e2a880e0ce-245.html */
            u32                  s_baseupper; /* Bits 32..64 of the base address. */
            u32                  s_reserved;  /* ??? (Bit #8 Must be zero...) */
#endif /* __x86_64__ */
        };
    };
};
#endif /* __CC__ */


/* Access s_flags. */
#define X86_SEG_ACCESS_PRESENT     0x00008000 /* s_present. */
#define X86_SEG_ACCESS_PRIVL(n) (((n)&0x3) << 13) /* s_privl (mask: 0x00006000). */
#define X86_SEG_ACCESS_SYSTEM      0x00001000 /* s_system. */
#define X86_SEG_ACCESS_EXECUTE     0x00000800 /* s_execute. */
#define X86_SEG_ACCESS_DC          0x00000400 /* s_dc. */
#define X86_SEG_ACCESS_RW          0x00000200 /* s_rw. */
#define X86_SEG_ACCESS_ACCESSED    0x00000100 /* s_accessed. */

#define X86_SEG_ACCESS(ex,s_dc,s_rw,ac) \
 (((ex)*X86_SEG_ACCESS_EXECUTE)|((s_dc)*X86_SEG_ACCESS_DC)|\
  ((s_rw)*X86_SEG_ACCESS_RW)|((ac)*X86_SEG_ACCESS_ACCESSED))

#define X86_SEG_DATA_RD        X86_SEG_ACCESS(0,0,0,0) /* Read-Only. */
#define X86_SEG_DATA_RDA       X86_SEG_ACCESS(0,0,0,1) /* Read-Only, s_accessed. */
#define X86_SEG_DATA_RDWR      X86_SEG_ACCESS(0,0,1,0) /* Read/Write. */
#define X86_SEG_DATA_RDWRA     X86_SEG_ACCESS(0,0,1,1) /* Read/Write, s_accessed. */
#define X86_SEG_DATA_RDEXPD    X86_SEG_ACCESS(0,1,0,0) /* Read-Only, expand-down. */
#define X86_SEG_DATA_RDEXPDA   X86_SEG_ACCESS(0,1,0,1) /* Read-Only, expand-down, s_accessed. */
#define X86_SEG_DATA_RDWREXPD  X86_SEG_ACCESS(0,1,1,0) /* Read/Write, expand-down. */
#define X86_SEG_DATA_RDWREXPDA X86_SEG_ACCESS(0,1,1,1) /* Read/Write, expand-down, s_accessed. */
#define X86_SEG_CODE_EX        X86_SEG_ACCESS(1,0,0,0) /* Execute-Only. */
#define X86_SEG_CODE_EXA       X86_SEG_ACCESS(1,0,0,1) /* Execute-Only, s_accessed. */
#define X86_SEG_CODE_EXRD      X86_SEG_ACCESS(1,0,1,0) /* Execute/Read. */
#define X86_SEG_CODE_EXRDA     X86_SEG_ACCESS(1,0,1,1) /* Execute/Read, s_accessed. */
#define X86_SEG_CODE_EXC       X86_SEG_ACCESS(1,1,0,0) /* Execute-Only, conforming. */
#define X86_SEG_CODE_EXCA      X86_SEG_ACCESS(1,1,0,1) /* Execute-Only, conforming, s_accessed. */
#define X86_SEG_CODE_EXRDC     X86_SEG_ACCESS(1,1,1,0) /* Execute/Read, conforming. */
#define X86_SEG_CODE_EXRDCA    X86_SEG_ACCESS(1,1,1,1) /* Execute/Read, conforming, s_accessed. */

/* Flags */
#define X86_SEG_FLAG_GRAN      0x00800000 /* Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB). */
#define X86_SEG_FLAG_32BIT     0x00400000 /* s_dbbit = 1. */
#define X86_SEG_FLAG_LONGMODE  0x00200000 /* s_longmode = 1. */
#define X86_SEG_FLAG_AVAILABLE 0x00100000 /* s_available = 1. */

#ifdef __x86_64__
#define X86_SEG_CODE_PL0    (X86_SEG_FLAG_LONGMODE|X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(0)|X86_SEG_CODE_EXRD)
#define X86_SEG_DATA_PL0                          (X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(0)|X86_SEG_DATA_RDWR)
#define X86_SEG_CODE_PL3    (X86_SEG_FLAG_LONGMODE|X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(3)|X86_SEG_CODE_EXRD)
#define X86_SEG_DATA_PL3                          (X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(3)|X86_SEG_DATA_RDWR)
/* TODO: The following two have not been confirmed, yet (so they probably don't work...). */
#define X86_SEG_CODE_PL3_32                       (X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(3)|X86_SEG_CODE_EXRD)
#define X86_SEG_DATA_PL3_32                       (X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(3)|X86_SEG_DATA_RDWR)
#else
/* Useful predefined x86_segment configurations
 * NOTE: The following configs match what is described here: http://wiki.osdev.org/Getting_to_Ring_3
 *    >> THIS IS CONFIRMED WORKING! */
#define X86_SEG_CODE_PL0    (X86_SEG_FLAG_32BIT|X86_SEG_FLAG_AVAILABLE|X86_SEG_FLAG_GRAN|X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(0)|X86_SEG_CODE_EXRD)
#define X86_SEG_DATA_PL0    (X86_SEG_FLAG_32BIT|X86_SEG_FLAG_AVAILABLE|X86_SEG_FLAG_GRAN|X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(0)|X86_SEG_DATA_RDWR)
#define X86_SEG_CODE_PL3    (X86_SEG_FLAG_32BIT|X86_SEG_FLAG_AVAILABLE|X86_SEG_FLAG_GRAN|X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(3)|X86_SEG_CODE_EXRD)
#define X86_SEG_DATA_PL3    (X86_SEG_FLAG_32BIT|X86_SEG_FLAG_AVAILABLE|X86_SEG_FLAG_GRAN|X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(3)|X86_SEG_DATA_RDWR)
#endif
#define X86_SEG_CODE_PL0_16 (X86_SEG_FLAG_AVAILABLE|X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(0)|X86_SEG_CODE_EXRD)
#define X86_SEG_DATA_PL0_16 (X86_SEG_FLAG_AVAILABLE|X86_SEG_ACCESS_SYSTEM|X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(0)|X86_SEG_DATA_RDWR)
#define X86_SEG_TSS         (X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(0)|X86_SEG_CODE_EXA)
#define X86_SEG_LDT         (X86_SEG_ACCESS_PRESENT|X86_SEG_ACCESS_PRIVL(3)|X86_SEG_DATA_RDWR)

#define X86_SEG_LIMIT_MAX    0x000fffff


#define __X86_SEG_ENCODELO_32(base,size,config) \
 (__CCAST(u32)((size)&__UINT16_C(0xffff))|       /* 0x0000ffff */\
 (__CCAST(u32)((base)&__UINT16_C(0xffff)) << 16) /* 0xffff0000 */)
#define __X86_SEG_ENCODEHI_32(base,size,config) \
 ((__CCAST(u32)((base)&__UINT32_C(0x00ff0000)) >> 16)| /* 0x000000ff */\
   __CCAST(u32)((config)&__UINT32_C(0x00f0ff00))|      /* 0x00f0ff00 */\
   __CCAST(u32)((size)&__UINT32_C(0x000f0000))|        /* 0x000f0000 */\
   __CCAST(u32)((base)&__UINT32_C(0xff000000))         /* 0xff000000 */)
#define __ASM_X86_SEG_ENCODELO_32(base,size,config) \
 (((size)&0xffff)|       /* 0x0000ffff */\
 (((base)&0xffff) << 16) /* 0xffff0000 */)
#define __ASM_X86_SEG_ENCODEHI_32(base,size,config) \
 ((((base)&0x00ff0000) >> 16)| /* 0x000000ff */\
   ((config)&0x00f0ff00)|      /* 0x00f0ff00 */\
   ((size)&0x000f0000)|        /* 0x000f0000 */\
   ((base)&0xff000000)         /* 0xff000000 */)

#ifdef __x86_64__
#define __X86_SEG_ENCODELO(base,size,config) \
       (__CCAST(u64)(__X86_SEG_ENCODEHI_32(base,size,config)) << 32 | \
        __CCAST(u64)(__X86_SEG_ENCODELO_32(base,size,config)))
#define __X86_SEG_ENCODEHI(base,size,config) \
      ((__CCAST(u64)(base) >> 32) & __UINT64_C(0x00000000ffffffff))
#else
#define __X86_SEG_ENCODELO(base,size,config) __X86_SEG_ENCODELO_32(base,size,config)
#define __X86_SEG_ENCODEHI(base,size,config) __X86_SEG_ENCODEHI_32(base,size,config)
#endif


#ifdef __CC__
#define __X86_SEGMENT_GTBASE32(seg) \
      ((seg).s_ul32 >> 16 | ((seg).s_uh32&__UINT32_C(0xff000000)) | ((seg).s_uh32&__UINT32_C(0xff)) << 16)
#define __X86_SEGMENT_STBASE32(seg,addr) \
 (*(u32 *)(&(seg).s_sizelow+1) &=              __UINT32_C(0xff000000), \
  *(u32 *)(&(seg).s_sizelow+1) |=  (u32)(addr)&__UINT32_C(0x00ffffff), \
            (seg).s_basehigh    = ((u32)(addr)&__UINT32_C(0xff000000)) >> 24)

#ifdef __x86_64__
#define X86_SEGMENT_GTBASE(seg) \
   ((u64)__X86_SEGMENT_GTBASE32(seg) | ((u64)(seg).s_baseupper << 32))
#define X86_SEGMENT_STBASE(seg,addr) \
   (__X86_SEGMENT_STBASE32(seg,(u64)(addr)),(seg).s_baseupper = (u32)((u64)(addr) >> 32))
#else
#define X86_SEGMENT_GTBASE(seg)      __X86_SEGMENT_GTBASE32(seg)
#define X86_SEGMENT_STBASE(seg,addr) __X86_SEGMENT_STBASE32(seg,addr)
#endif
#define X86_SEGMENT_GTSIZE(seg)   (((seg).s_ul32 & __UINT32_C(0xffff)) | ((seg).s_uh32&__UINT32_C(0x000f0000)))

#define X86_SEGMENT_INIT(base,size,config) \
 {{{ __X86_SEG_ENCODELO(base,size,config), \
     __X86_SEG_ENCODEHI(base,size,config) }}}

LOCAL ATTR_CONST struct x86_segment KCALL
x86_makesegment(uintptr_t base, size_t size, u32 config) {
 struct x86_segment result;
 __assertf(size <= X86_SEG_LIMIT_MAX,"Size %#Ix is too large",size);
 result.s_ul = __X86_SEG_ENCODELO(base,size,config);
 result.s_uh = __X86_SEG_ENCODEHI(base,size,config);
 return result;
}

#ifndef __segid_t_defined
#define __segid_t_defined 1
typedef u16 segid_t; /* == Segment index*X86_SEG_INDEX_MULTIPLIER */
#endif /* !__segid_t_defined */
#endif /* __CC__ */

#ifdef __x86_64__
#define X86_SEGMENT_SIZE         16
#define X86_SEG_INDEX_MULTIPLIER 16
#else
#define X86_SEGMENT_SIZE         8
#define X86_SEG_INDEX_MULTIPLIER 8
#endif

#define X86_SEG(id)     ((id)*X86_SEG_INDEX_MULTIPLIER)
#define X86_SEG_ID(seg) ((seg)/X86_SEG_INDEX_MULTIPLIER)

/* When the third bit of a x86_segment index is set,
 * it's_s referencing the LDT instead of the GDT. */
#define X86_SEG_TOGDT(seg) ((seg)&~0x4)
#define X86_SEG_TOLDT(seg) ((seg)|0x4)
#define X86_SEG_ISLDT(seg) (((seg)&0x4)!=0)
#define X86_SEG_ISGDT(seg) (((seg)&0x4)==0)

/* Hard-coded, special segments ids. */
#define X86_SEG_NULL         0 /* [0x00] NULL Segment. */
#define X86_SEG_HOST_CODE    1 /* [0x08] Ring #0 code segment. */
#define X86_SEG_HOST_DATA    2 /* [0x10] Ring #0 data segment. */
#define X86_SEG_USER_CODE    3 /* [0x18] Ring #3 code segment. */         
#define X86_SEG_USER_DATA    4 /* [0x20] Ring #3 data segment. */
#ifdef __x86_64__
#define X86_SEG_USER_CODE32  5 /* [0x28] Ring #3 32-bit (compatibility mode) code segment. */
#define X86_SEG_USER_DATA32  6 /* [0x30] Ring #3 32-bit (compatibility mode) data segment. */
#else
#define X86_SEG_HOST_CODE16  5 /* [0x28] Ring #0 16-bit code segment. */
#define X86_SEG_HOST_DATA16  6 /* [0x30] Ring #0 16-bit data segment. */
#endif
#define X86_SEG_CPUTSS       7 /* [0x38] TSS segment of the current CPU. */
#ifndef __x86_64__
#define X86_SEG_CPUTSS_DF    8 /* [0x40] TSS segment of the current CPU (for #DF handling). */
#endif
#define X86_SEG_KERNEL_LDT   9 /* [0x48] Symbolic kernel LDT (Usually empty). */
#define X86_SEG_HOST_TLS    10 /* [0x50] Ring #0 thread-local block. */
#define X86_SEG_USER_TLS    11 /* [0x58] Ring #3 thread-local block. */
#ifndef CONFIG_NO_DOS_COMPAT
#define X86_SEG_USER_TIB    12 /* [0x60] Ring #3 thread-information block. (For DOS compatibility) */
#define X86_SEG_BUILTIN     13
#else
#define X86_SEG_BUILTIN     12
#endif

#define X86_SEG_MAX            0xffff
#define X86_SEG_ISBUILTIN(seg) ((seg) < X86_SEG(X86_SEG_BUILTIN))


/* Special segment values. */
#define X86_KERNEL_DS      X86_SEG(X86_SEG_HOST_DATA)
#define X86_KERNEL_CS      X86_SEG(X86_SEG_HOST_CODE)
#define X86_USER_DS       (X86_SEG(X86_SEG_USER_DATA)|3)
#define X86_USER_CS       (X86_SEG(X86_SEG_USER_CODE)|3)
#ifdef __x86_64__
#define X86_USER_DS32     (X86_SEG(X86_SEG_USER_DATA32)|3)
#define X86_USER_CS32     (X86_SEG(X86_SEG_USER_CODE32)|3)
#endif
#define X86_HOST_TLS       X86_SEG(X86_SEG_HOST_TLS)     /* Points the current HOST `struct task_segment' and `struct task' */
#define X86_USER_TLS      (X86_SEG(X86_SEG_USER_TLS)|3)  /* Points the current USER `struct task_segment' */
#ifndef CONFIG_NO_DOS_COMPAT
#define X86_USER_TIB      (X86_SEG(X86_SEG_USER_TIB)|3)  /* Points the current USER `struct nt_tib' */
#endif
#ifdef __x86_64__
#define X86_SEG_FS         X86_USER_TLS
#define X86_SEG_GS         X86_HOST_TLS
#define X86_SEG_USER_FS    X86_USER_TLS
#ifndef CONFIG_NO_DOS_COMPAT
#define X86_SEG_USER_GS    X86_USER_TIB
#else
#define X86_SEG_USER_GS    X86_HOST_TLS
#endif
#else
#define X86_SEG_FS         X86_HOST_TLS
#define X86_SEG_GS         X86_USER_TLS
#ifndef CONFIG_NO_DOS_COMPAT
#define X86_SEG_USER_FS    X86_USER_TIB
#else
#define X86_SEG_USER_FS    X86_HOST_TLS
#endif
#define X86_SEG_USER_GS    X86_USER_TLS
#endif

#define X86_SEG_USER_CS    X86_USER_CS
#define X86_SEG_USER_DS    X86_USER_DS
#define X86_SEG_USER_ES    X86_USER_DS
#define X86_SEG_USER_SS    X86_USER_DS
#ifdef __x86_64__
#define X86_SEG_USER_CS32  X86_USER_CS32
#define X86_SEG_USER_DS32  X86_USER_DS32
#define X86_SEG_USER_ES32  X86_USER_DS32
#define X86_SEG_USER_SS32  X86_USER_DS32
#endif


#define X86_SEG_HOST_CS    X86_KERNEL_CS
#define X86_SEG_HOST_DS    X86_KERNEL_DS
#define X86_SEG_HOST_ES    X86_KERNEL_DS
#define X86_SEG_HOST_SS    X86_KERNEL_DS
#ifdef __ASM_TASK_SEGMENT_ISGS
#define X86_SEG_HOST_FS    X86_KERNEL_DS
#define X86_SEG_HOST_GS    X86_HOST_TLS
#else
#define X86_SEG_HOST_FS    X86_HOST_TLS
#define X86_SEG_HOST_GS    X86_KERNEL_DS
#endif

#ifdef __CC__
/* The per-cpu GDT vector. */
DATDEF ATTR_PERCPU struct x86_segment x86_cpugdt[X86_SEG_BUILTIN];
#endif

#ifdef __x86_64__
/* TODO: Figure out how these should interact with
 *      `t_userseg' and the `CLONE_SETTLS' flag, as
 *       well as the `c_segments.sg_fsbase' field
 *       during a clone() system call. */
#define WR_USER_FSBASE(v) __wrfsbaseq(v)
#define WR_USER_GSBASE(v) __wrmsr(IA32_KERNEL_GS_BASE,v)

/* While in kernel-mode, the user-gs is saved in `IA32_KERNEL_GS_BASE' */
#define RD_USER_FSBASE()           __rdfsbaseq()
#define RD_USER_GSBASE()           __rdmsr(IA32_KERNEL_GS_BASE)
#define WR_USER_FSBASE_REGISTER(v) __wrfsbaseq(v)
#define WR_USER_GSBASE_REGISTER(v) __wrmsr(IA32_KERNEL_GS_BASE,v)
#endif


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_GDT_H */
