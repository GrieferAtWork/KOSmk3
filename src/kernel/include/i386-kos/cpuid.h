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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_CPUID_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_CPUID_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <kernel/sections.h>
#include <sched/task.h>
#include <asm/cpu-flags.h>

DECL_BEGIN

#define CPU_BASIC_FEATURE_FNONE    0x0000
#define CPU_BASIC_FEATURE_FCPUID   0x0001 /* The `cpuid' instruction is supported. */

/* Basic CPU feature flags (Set of `CPU_BASIC_FEATURE_F*') */
DATDEF ATTR_PERCPU u16 const cpu_basic_features;

struct cpu_cpuid {
    union PACKED {
        u32           ci_0a;           /* [const][== cpuid(0).EAX] */
        u32           ci_bleaf_max;    /* [const][== cpuid(0).EAX] Max supported basic cpuid leaf number. */
    };
    union PACKED {
        u32           ci_80000000a;    /* [const][== cpuid(0x80000000).EAX] */
        u32           ci_eleaf_max;    /* [const][== cpuid(0x80000000).EAX] Max supported basic cpuid leaf number */
    };
    union PACKED {
        u32           ci_1a;           /* [const][== cpuid(1).EAX] Processor info... */
        struct PACKED {
            unsigned int ci_version : 12;
        };
        struct PACKED {
            unsigned int ci_stepping : 4;
            unsigned int ci_model : 4;
            unsigned int ci_family : 4;
            unsigned int ci_processor_type : 4;
            unsigned int ci_extended_model : 4;
            unsigned int ci_extended_family : 8;
            unsigned int __ci_pad0 : 4;
        };
    };
    u32               ci_1b;           /* [const][== cpuid(1).EBX] Processor info... */
    u32               ci_1d;           /* [const][== cpuid(1).EDX] Set of `CPUID_1D_*' */
    u32               ci_1c;           /* [const][== cpuid(1).ECX] Set of `CPUID_1C_*' */
    u32               ci_7d;           /* [const][== cpuid(7,0).EDX] Set of `CPUID_7D_*' */
    u32               ci_7c;           /* [const][== cpuid(7,0).ECX] Set of `CPUID_7C_*' */
    u32               ci_7b;           /* [const][== cpuid(7,0).EBX] Set of `CPUID_7B_*' */
    u32               ci_80000001c;    /* [const][== cpuid(0x80000001).ECX] Set of `CPUID_80000001C_*' */
    u32               ci_80000001d;    /* [const][== cpuid(0x80000001).EDX] Set of `CPUID_80000001D_*' */
    union PACKED {
        struct PACKED {
            u32       ci_0b;           /* [const][== cpuid(0).EBX] */
            u32       ci_0d;           /* [const][== cpuid(0).EDX] */
            u32       ci_0c;           /* [const][== cpuid(0).ECX] */
        };
        char          ci_vendorid[12]; /* [const][== cpuid(0).EBX~EDX~ECX] Vendor ID */
    };
    char            __ci_zero1;        /* [const][== 0] */
    union PACKED {
        struct PACKED {
            u32       ci_80000002a;    /* [const][== cpuid(0x80000002).EAX] */
            u32       ci_80000002b;    /* [const][== cpuid(0x80000002).EBX] */
            u32       ci_80000002c;    /* [const][== cpuid(0x80000002).ECX] */
            u32       ci_80000002d;    /* [const][== cpuid(0x80000002).EDX] */
            u32       ci_80000003a;    /* [const][== cpuid(0x80000003).EAX] */
            u32       ci_80000003b;    /* [const][== cpuid(0x80000003).EBX] */
            u32       ci_80000003c;    /* [const][== cpuid(0x80000003).ECX] */
            u32       ci_80000003d;    /* [const][== cpuid(0x80000003).EDX] */
            u32       ci_80000004a;    /* [const][== cpuid(0x80000004).EAX] */
            u32       ci_80000004b;    /* [const][== cpuid(0x80000004).EBX] */
            u32       ci_80000004c;    /* [const][== cpuid(0x80000004).ECX] */
            u32       ci_80000004d;    /* [const][== cpuid(0x80000004).EDX] */
        };
        char          ci_brand[48];    /* [const][== cpuid(0x80000002...0x80000004).EAX~EBX~ECX~EDX] Brand string. */
    };
    char            __ci_zero2;        /* [const][== 0] */
};

/* [valid_if(cpu_basic_features & CPU_BASIC_FEATURE_FCPUID)]
 * NOTE: When `CPU_BASIC_FEATURE_FCPUID' isn't set, all members are ZERO-initialized.
 * Information gathered from `cpuid' */
DATDEF ATTR_PERCPU struct cpu_cpuid const cpu_id_features;
#define CPU_FEATURES  PERCPU(cpu_id_features)




DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_CPUID_H */
