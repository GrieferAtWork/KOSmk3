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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_APIC_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_APIC_H 1

#include <hybrid/compiler.h>
#include <hybrid/host.h>
#include <kernel/sections.h>
#include <i386-kos/paging.h>
#include <sys/io.h>

DECL_BEGIN

#define APIC_ID                                 0x0020     /* LAPIC ID of the calling CPU. */
#    define APIC_ID_FMASK                       0xff000000 /* Mask of the actual ID. */
#    define APIC_ID_FSHIFT                      24         /* Shift of the actual ID. */

#define APIC_EOI                                0x00b0     /* EndOfInterrupt */
#    define APIC_EOI_FSIGNAL                    0x00000000 /* Written to `APIC_EOI' to signal END-OF-INTERRUPT */

#define APIC_SPURIOUS                           0x00f0     /* Spurious interrupt vector & apic-enabled. */
#    define APIC_SPURIOUS_FVECTOR               0x000000ff /* Mask of the spurious interrupt vector. */
#    define APIC_SPURIOUS_FENABLED              0x00000100 /* When set, the APIC is enabled. */

#define APIC_LINT0                              0x0350     /* Local interrupt vector #0 */
#define APIC_LINT1                              0x0360     /* Local interrupt vector #1 */
#    define APIC_LINT_FVECTOR                   0x000000ff /* Mask of the interrupt vector number. */
#    define APIC_LINT_FDELIVERY                 0x00000700 /* Mask for the delivery type used. */
#        define APIC_LINT_DELIVERY_FNORMAL      0x00000000 /* Normal delivery. */
/*       define APIC_LINT_DELIVERY_F            0x00000000 ... 0x00000300  */
#        define APIC_LINT_DELIVERY_FNMI         0x00000400 /* Normal delivery. */
/*       define APIC_LINT_DELIVERY_F            0x00000500 ... 0x00000600  */
#        define APIC_LINT_DELIVERY_FEXTERNAL    0x00000700 /* External interrupt (???) */
#    define APIC_LINT_FPENDING                  0x00001000 /* The interrupt is pending delivery. */
#    define APIC_LINT_FPOLARITY                 0x00002000 /* The polarity bit. */
#        define APIC_LINT_POLARITY_FHIGH        0x00000000 /* High polarity. */
#        define APIC_LINT_POLARITY_FLOW         0x00002000 /* Low polarity. */
#    define APIC_LINT_FLEVELACK                 0x00004000 /* Level-triggered interrupt has been acknowledged. */
#    define APIC_LINT_FTRIGGER                  0x00008000 /* The triggering mode. */
#        define APIC_LINT_TRIGGER_EDGE          0x00000000 /* Edge-triggered. */
#        define APIC_LINT_TRIGGER_LEVEL         0x00008000 /* Level-triggered. */
#    define APIC_LINT_FDISABLED                 0x00010000 /* The interrupt is disabled. */

#define APIC_TIMER                              0x0320     /* Interrupt vector fired by the LAPIC timer. */
#    define APIC_TIMER_FVECTOR                  0x000000ff /* Mask for the interrupt vector number fired by the timer. */
#    define APIC_TIMER_FPENDING                 0x00001000 /* The timer interrupt is pending delivery. */
#    define APIC_TIMER_FDISABLED                0x00010000 /* The timer interrupt is disabled. */
#    define APIC_TIMER_FMODE                    0x00020000 /* Mask for the timer operations mode. */
#        define APIC_TIMER_MODE_FONESHOT        0x00000000 /* The timer will only fire once. */
#        define APIC_TIMER_MODE_FPERIODIC       0x00020000 /* The timer will fire periodically. */
#    define APIC_TIMER_FSOURCE                  0x000c0000 /* Timer source. */
#        define APIC_TIMER_SOURCE_FCLKIN        0x00000000 /* Use CLKIN as input. */
#        define APIC_TIMER_SOURCE_FTMBASE       0x00040000 /* Use TMBASE as input. */
#        define APIC_TIMER_SOURCE_FDIV          0x00080000 /* Use the output of the divider as input. */

#define APIC_ICR0                               0x0300     /* Common options to send to `APIC_ICR1' */
#    define APIC_ICR0_FVECTOR                   0x000000ff /* Interrupt that should be triggered. */
#    define APIC_ICR0_FPAGENO                   0x000000ff /* Page number to execute (for `APIC_ICR0_TYPE_FSIPI'). */
#    define APIC_ICR0_FTYPE                     0x00000700 /* The type of interrupt to trigger. */
#        define APIC_ICR0_TYPE_FNORMAL          0x00000000 /* Trigger a normal interrupt. */
#        define APIC_ICR0_TYPE_FLOWPRIO         0x00000100 /* Trigger a low-priority interrupt. */
#        define APIC_ICR0_TYPE_FSMI             0x00000200 /* Trigger a system-management interrupt. */
#        define APIC_ICR0_TYPE_FNMI             0x00000400 /* Trigger a non-maskable interrupt. */
#        define APIC_ICR0_TYPE_FINIT            0x00000500 /* INIT or INIT-level de-assert. */
#        define APIC_ICR0_TYPE_FSIPI            0x00000600 /* Startup Inter-Processor Interrupt. */
#    define APIC_ICR0_FDEST                     0x00000800 /* The kind of destination to which to send the interrupt. */
#        define APIC_ICR0_DEST_PHYSICAL         0x00000000 /* Physical destination. */
#        define APIC_ICR0_DEST_LOGICAL          0x00000800 /* Logical destination. */
#    define APIC_ICR0_FPENDING                  0x00001000 /* The interrupt is pending delivery. */
#    define APIC_ICR0_FASSERT                   0x00004000 /* Assert the interrupt (set for all, except for INIT-de-assert) */
#    define APIC_ICR0_FLEVELTRIG                0x00008000 /* ??? Set for INIT-de-assert */
#    define APIC_ICR0_FTARGET                   0x000c0000 /* Mask for the target of the interrupt. */
#        define APIC_ICR0_TARGET_FICR1          0x00000000 /* Send to the CPU id stored in `APIC_ICR1'. */
#        define APIC_ICR0_TARGET_FSELF          0x00040000 /* Send to the calling CPU. */
#        define APIC_ICR0_TARGET_FALL           0x00080000 /* Send to all CPUs. */
#        define APIC_ICR0_TARGET_FOTHERS        0x000c0000 /* Send to all but the calling CPU. */

#define APIC_ICR1                               0x0310     /* ID of the target APIC. */
#    define APIC_ICR1_FDEST                     0xff000000 /* Mask of the target APIC ID. */
#    define APIC_ICR1_SDEST                     24         /* Shift of the target APIC ID. */
#    define APIC_ICR1_GTDEST(x)               ((x) >> APIC_ICR1_SDEST)
#    define APIC_ICR1_MKDEST(lapic_id)        ((lapic_id) << APIC_ICR1_SDEST)

#define APIC_TIMER_DIVIDE                       0x03e0     /* Timer divide register. */
#    define APIC_TIMER_DIVIDE_FMASK             0x0000000b /* Mask for the divide mode. */
#    define APIC_TIMER_DIVIDE_F1                0x0000000b /* Divide by 1 */
#    define APIC_TIMER_DIVIDE_F2                0x00000000 /* Divide by 2 */
#    define APIC_TIMER_DIVIDE_F4                0x00000001 /* Divide by 4 */
#    define APIC_TIMER_DIVIDE_F8                0x00000002 /* Divide by 8 */
#    define APIC_TIMER_DIVIDE_F16               0x00000003 /* Divide by 16 */
#    define APIC_TIMER_DIVIDE_F32               0x00000008 /* Divide by 32 */
#    define APIC_TIMER_DIVIDE_F64               0x00000009 /* Divide by 64 */
#    define APIC_TIMER_DIVIDE_F128              0x0000000a /* Divide by 128 */

#define APIC_TIMER_INITIAL                      0x0380     /* Initial timer value. */

#define APIC_TIMER_CURRENT                      0x0390     /* Current timer value. */

#define APIC_SIZE                               0x1500



#if 1
#else

 /* Slightly modified; from linux: "/arch/x86/include/asm/apicdef.h" */
/*
 * Constants for various Intel APICs. (local APIC, IOAPIC, etc.)
 *
 * Alan Cox <Alan.Cox@linux.org>, 1995.
 * Ingo Molnar <mingo@redhat.com>, 1999, 2000
 */

#define IO_APIC_DEFAULT_PHYS_BASE     0xfec00000
#define APIC_DEFAULT_PHYS_BASE        0xfee00000

/*
 * This is the IO-APIC register space as specified
 * by Intel docs:
 */
#define IO_APIC_SLOT_SIZE             1024

#define APIC_ID                       0x20

#define APIC_LVR                      0x30
#define     APIC_LVR_MASK             0xFF00FF
#define     APIC_LVR_DIRECTED_EOI    (1 << 24)
#define     GET_APIC_VERSION(x)     ((x) & 0xFFu)
#define     GET_APIC_MAXLVT(x)     (((x) >> 16) & 0xFFu)
#if __SIZEOF_POINTER__ == 4
#  define APIC_INTEGRATED(x)        ((x) & 0xF0u)
#else
#  define APIC_INTEGRATED(x)         (1)
#endif
#define     APIC_XAPIC(x)           ((x) >= 0x14)
#define     APIC_EXT_SPACE(x)       ((x) & 0x80000000)
#define APIC_TASKPRI                  0x80
#define     APIC_TPRI_MASK            0xFFu
#define APIC_ARBPRI                   0x90
#define     APIC_ARBPRI_MASK          0xFFu
#define APIC_PROCPRI                  0xA0
#define APIC_EOI                      0xB0
#define     APIC_EOI_ACK              0x0 /* Docs say 0 for future compat. */
#define APIC_RRR                      0xC0
#define APIC_LDR                      0xD0
#define     APIC_LDR_MASK            (0xFFu << 24)
#define     GET_APIC_LOGICAL_ID(x) (((x) >> 24) & 0xFFu)
#define     SET_APIC_LOGICAL_ID(x) (((x) << 24))
#define     APIC_ALL_CPUS             0xFFu
#define APIC_DFR                      0xE0
#define     APIC_DFR_CLUSTER          0x0FFFFFFFul
#define     APIC_DFR_FLAT             0xFFFFFFFFul
#define APIC_SPIV                     0xF0
#define     APIC_SPIV_DIRECTED_EOI   (1 << 12)
#define     APIC_SPIV_FOCUS_DISABLED (1 << 9)
#define     APIC_SPIV_APIC_ENABLED   (1 << 8)
#define APIC_ISR                      0x100
#define APIC_ISR_NR                   0x8 /* Number of 32 bit ISR registers. */
#define APIC_TMR                      0x180
#define APIC_IRR                      0x200
#define APIC_ESR                      0x280
#define     APIC_ESR_SEND_CS          0x00001
#define     APIC_ESR_RECV_CS          0x00002
#define     APIC_ESR_SEND_ACC         0x00004
#define     APIC_ESR_RECV_ACC         0x00008
#define     APIC_ESR_SENDILL          0x00020
#define     APIC_ESR_RECVILL          0x00040
#define     APIC_ESR_ILLREGA          0x00080
#define APIC_LVTCMCI                  0x2f0
#define APIC_ICR                      0x300
#define     APIC_DEST_SELF            0x40000
#define     APIC_DEST_ALLINC          0x80000
#define     APIC_DEST_ALLBUT          0xC0000
#define     APIC_ICR_RR_MASK          0x30000
#define     APIC_ICR_RR_INVALID       0x00000
#define     APIC_ICR_RR_INPROG        0x10000
#define     APIC_ICR_RR_VALID         0x20000
#define     APIC_INT_LEVELTRIG        0x08000
#define     APIC_INT_ASSERT           0x04000
#define     APIC_ICR_BUSY             0x01000 /* You never write this. */
#define     APIC_DEST_LOGICAL         0x00800
#define     APIC_DEST_PHYSICAL        0x00000
#define     APIC_DM_FIXED             0x00000
#define     APIC_DM_FIXED_MASK        0x00700
#define     APIC_DM_LOWEST            0x00100
#define     APIC_DM_SMI               0x00200
#define     APIC_DM_REMRD             0x00300
#define     APIC_DM_NMI               0x00400
#define     APIC_DM_INIT              0x00500
#define     APIC_DM_STARTUP           0x00600
#define     APIC_DM_EXTINT            0x00700
#define     APIC_VECTOR_MASK          0x000FF
#define APIC_ICR2                     0x310
#define     GET_APIC_DEST_FIELD(x) (((x) >> 24) & 0xFF)
#define     SET_APIC_DEST_FIELD(x)  ((x) << 24)
#define APIC_LVTT                     0x320
#define APIC_LVTTHMR                  0x330
#define APIC_LVTPC                    0x340
#define APIC_LVT0                     0x350
#define     APIC_LVT_TIMER_BASE_MASK (0x3 << 18)
#define     GET_APIC_TIMER_BASE(x) (((x) >> 18) & 0x3)
#define     SET_APIC_TIMER_BASE(x) (((x) << 18))
#define         APIC_TIMER_BASE_CLKIN 0x0
#define         APIC_TIMER_BASE_TMBASE 0x1
#define         APIC_TIMER_BASE_DIV   0x2
#define     APIC_LVT_TIMER_ONESHOT   (0 << 17)
#define     APIC_LVT_TIMER_PERIODIC  (1 << 17)
#define     APIC_LVT_TIMER_TSCDEADLINE (2 << 17)
#define     APIC_LVT_MASKED          (1 << 16)
#define     APIC_LVT_LEVEL_TRIGGER   (1 << 15)
#define     APIC_LVT_REMOTE_IRR      (1 << 14)
#define     APIC_INPUT_POLARITY      (1 << 13)
#define     APIC_SEND_PENDING        (1 << 12)
#define     APIC_MODE_MASK            0x700
#define     GET_APIC_DELIVERY_MODE(x) (((x) >> 8) & 0x7)
#define     SET_APIC_DELIVERY_MODE(x,y) (((x) & ~0x700) | ((y) << 8))
#define         APIC_MODE_FIXED       0x0
#define         APIC_MODE_NMI         0x4
#define         APIC_MODE_EXTINT      0x7
#define APIC_LVT1                     0x360
#define APIC_LVTERR                   0x370
#define APIC_TMICT                    0x380
#define APIC_TMCCT                    0x390
#define APIC_TDCR                     0x3E0
#define     APIC_TDR_DIV_TMBASE      (1 << 2)
#define     APIC_TDR_DIV_1            0xB
#define     APIC_TDR_DIV_2            0x0
#define     APIC_TDR_DIV_4            0x1
#define     APIC_TDR_DIV_8            0x2
#define     APIC_TDR_DIV_16           0x3
#define     APIC_TDR_DIV_32           0x8
#define     APIC_TDR_DIV_64           0x9
#define     APIC_TDR_DIV_128          0xA
#define APIC_SELF_IPI                 0x3F0
#define APIC_EFEAT                    0x400
#define APIC_ECTRL                    0x410
#define APIC_EILVTn(n)               (0x500 + 0x10 * n)
#define     APIC_EILVT_NR_AMD_K8      1 /* # of extended interrupts */
#define     APIC_EILVT_NR_AMD_10H     4
#define     APIC_EILVT_NR_MAX         APIC_EILVT_NR_AMD_10H
#define     APIC_EILVT_LVTOFF(x)   (((x) >> 4) & 0xF)
#define     APIC_EILVT_MSG_FIX        0x0
#define     APIC_EILVT_MSG_SMI        0x2
#define     APIC_EILVT_MSG_NMI        0x4
#define     APIC_EILVT_MSG_EXT        0x7
#define     APIC_EILVT_MASKED        (1 << 16)
#define APIC_SIZE                    APIC_EILVTn(256)
#endif
/* end... */




#ifdef __CC__
/* [const] Physical/Virtual base addresses of the local (per-cpu) APIC.
 *  NOTE: APIC stands for Advanced Programmable Interrupt Controller. */
DATDEF PHYS u32              const x86_lapic_base_address_phys; /* [valid_if(x86_lapic_base_address != NULL)] */
DATDEF VIRT volatile byte_t *const x86_lapic_base_address;      /* [const] Virtual base address of the LAPIC. */
#define X86_HAVE_LAPIC            (x86_lapic_base_address != NULL)
#define lapic_read(offset)         readl(x86_lapic_base_address+(offset))
#define lapic_write(offset,value)  writel(x86_lapic_base_address+(offset),value)

/* The current LAPIC timer frequency.
 * This frequency is measured in the number of ticks happening in the
 * LAPIC timer every second, assuming that `APIC_TIMER_DIVIDE_F16' is used.
 * NOTE: This number is adjusted periodically in order to keep jiffi timing
 *       as fair and consistent as possible. */
DATDEF ATTR_PERCPU volatile u32 x86_lapic_timer_freq;

/* [valid_if(X86_HAVE_LAPIC)] The LAPIC ID of the controller associated with the CPU. */
DATDEF ATTR_PERCPU u8 const x86_lapic_id;


#endif /* __CC__ */


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_APIC_H */
