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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_INTERRUPT_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_INTERRUPT_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/sections.h>
#include <kos/i386-kos/except.h>

DECL_BEGIN

/* Amount of `struct idtentry' that make up a full IDT table. */
#define X86_IDT_TABLESIZE 256

#define X86_IDTENTRY_OFFSETOF_OFF1  0
#define X86_IDTENTRY_OFFSETOF_SEL   2
#define X86_IDTENTRY_OFFSETOF_ZERO  4
#define X86_IDTENTRY_OFFSETOF_FLAGS 5
#define X86_IDTENTRY_OFFSETOF_OFF2  6
#ifdef __x86_64__
#define X86_IDTENTRY_OFFSETOF_OFF3  8
#define X86_IDTENTRY_SIZE           16
#else
#define X86_IDTENTRY_SIZE           8
#endif

#ifdef __CC__
struct PACKED x86_idtentry {
    u16 ie_off1;  /* Lower 16 bits of an `irq_handler' pointer. */
    u16 ie_sel;   /* Kernel code segment (always `__X86_KERNEL_CS') */
#ifdef __x86_64__
    u8  ie_ist;   /* Bits 0..2 hold Interrupt Stack Table offset, rest of bits zero. */
#else /* __x86_64__ */
    u8  ie_zero;  /* Always ZERO(0). */
#endif /* !__x86_64__ */
    u8  ie_flags; /* Set of `X86_IDTFLAG_*|X86_IDTTYPE_*' */
    u16 ie_off2;  /* Upper 16 bits of an `irq_handler' pointer. */
#ifdef __x86_64__
    u32 ie_off3;  /* Bits 32..63 of the vector offset. */
    u32 ie_unused;/* Unused ata. */
#endif /* __x86_64__ */
};
#endif /* __CC__ */


#define X86_IDTFLAG_PRESENT                 0x80 /* Set to 0 for unused interrupts. */
/* Descriptor Privilege LevelGate call protection.
 * Specifies which privilege Level the calling Descriptor minimum should have.
 * So hardware and CPU interrupts can be protected from being called out of userspace. */
#define X86_IDTFLAG_DPL(n)          (((n)&3)<<5) /* Mask: 0x60 */
#define X86_IDTFLAG_DPL_MASK                0x60
#define X86_IDTFLAG_STORAGE_SEGMENT         0x10 /* Set to 0 for interrupt gates. */
#define X86_IDTTYPE_MASK                    0x0f
#define X86_IDTTYPE_80386_32_TASK_GATE      0x05
#define X86_IDTTYPE_80286_16_INTERRUPT_GATE 0x06
#define X86_IDTTYPE_80286_16_TRAP_GATE      0x07
#define X86_IDTTYPE_80386_32_INTERRUPT_GATE 0x0e
#ifndef __x86_64__
#define X86_IDTTYPE_80386_32_TRAP_GATE      0x0f
#endif

/* Different TASK vs. Interrupt gate:
 *   - Upon entry into an interrupt gate, #IF is disabled
 *   - Upon entry to a trap gate, #IF is left unchanged.
 * NOTE: The default X86-interrupt handling code used by
 *       KOS assumes that Interrupt gates are being used,
 *       as it modifies part of the start prior to allocating
 *       it, meaning that it assumes not to be interrupted
 *       and have some other piece of code tinker with
 *       unallocated stack memory. */


/* Return the IRQ numbers of hardware interrupt
 * lines wired either to the master, or slave PIC.
 * @param: i :  The line number (0..7)
 * @return: * : The IRQ number. */
#define X86_INTNO_PIC1(i) (X86_INTERRUPT_PIC1_BASE+(i))
#define X86_INTNO_PIC2(i) (X86_INTERRUPT_PIC2_BASE+(i))



#ifdef __CC__
/* A special exception descriptor that should be used to
 * implementing exception unwinding at interrupt entry point.
 * This handler will automatically propagate exceptions that originally
 * occurred while in kernel-space, to user-space, analyzing the
 * interrupt return tail that was used to enter the kernel.
 * If the error originated from kernel-space, continue
 * searching for exception handlers like usual.
 * Use the below macro `X86_DEFINE_INTERRUPT_GUARD()'
 * to define an interrupt exception guard. */
DATDEF struct except_desc const x86_interrupt_guard;
/* Very similar to `x86_interrupt_guard', but this
 * guard will set the `ERR_FSYSCALL' flag before
 * moving on to do the same as `x86_interrupt_guard' */
DATDEF struct except_desc const x86_syscall_guard;
/* Same as `x86_syscall_guard', but will also set the `ERR_FSYSCALL_EXC' flag. */
DATDEF struct except_desc const x86_syscall_except_guard;
#endif

/* Define an exception guard for a custom interrupt handler:
 * >> X86_DEFINE_INTERRUPT_GUARD(myint,myint_end);
 * >> myint:
 * >>     .cfi_startproc simple
 * >>     .cfi_signal_frame
 * >>     .cfi_def_cfa %esp,    3*4
 * >>     .cfi_offset %eflags, -1*4
 * >>     .cfi_offset %eip,    -3*4
 * >>     
 * >>     ... // Whatever your interrupt handler does goes here.
 * >>     
 * >>     iret
 * >>     .cfi_endproc
 * >> myint_end:
 */
#define X86_DEFINE_INTERRUPT_GUARD(begin,end) \
      __X86_DEFINE_EXCEPT_HANDLER(begin,end,x86_interrupt_guard, \
                                  EXCEPTION_HANDLER_FDESCRIPTOR,0)
#define X86_DEFINE_SYSCALL_GUARD(begin,end) \
      __X86_DEFINE_EXCEPT_HANDLER(begin,end,x86_syscall_guard, \
                                  EXCEPTION_HANDLER_FDESCRIPTOR,0)
#define X86_DEFINE_SYSCALL_EXCEPT_GUARD(begin,end) \
      __X86_DEFINE_EXCEPT_HANDLER(begin,end,x86_syscall_except_guard, \
                                  EXCEPTION_HANDLER_FDESCRIPTOR,0)


#ifdef __CC__
/* Preemption control. */
FORCELOCAL void x86_interrupt_enable(void) { __asm__("sti" : : : "memory"); }
FORCELOCAL void x86_interrupt_disable(void) { __asm__("cli" : : : "memory"); }
typedef register_t pflag_t;
#ifdef __x86_64__
FORCELOCAL pflag_t x86_interrupt_enabled(void) {
 pflag_t result;
 __asm__("pushfq\npopq %0\n" : "=g" (result) : : "memory");
 return result & 0x00000200; /* EFLAGS_IF */
}
FORCELOCAL pflag_t x86_interrupt_pushoff(void) {
 pflag_t result;
 __asm__("pushfq\npopq %0\ncli\n" : "=g" (result) : : "memory");
 return result & 0x00000200; /* EFLAGS_IF */
}
FORCELOCAL void x86_interrupt_pop(pflag_t flag) {
 __asm__("pushq %0\npopfq\n" : : "g" (flag) : "memory", "cc");
}
#else
FORCELOCAL pflag_t x86_interrupt_enabled(void) {
 pflag_t result;
 __asm__("pushfl\npopl %0\n" : "=g" (result) : : "memory");
 return result & 0x00000200; /* EFLAGS_IF */
}
FORCELOCAL pflag_t x86_interrupt_pushoff(void) {
 pflag_t result;
 __asm__("pushfl\npopl %0\ncli\n" : "=g" (result) : : "memory");
 return result & 0x00000200; /* EFLAGS_IF */
}
FORCELOCAL void x86_interrupt_pop(pflag_t flag) {
 __asm__("pushl %0\npopfl\n" : : "g" (flag) : "memory", "cc");
}
#endif

#define PREEMPTION_ENABLE()   x86_interrupt_enable()
#define PREEMPTION_DISABLE()  x86_interrupt_disable()
#define PREEMPTION_ENABLED()  x86_interrupt_enabled()
#define PREEMPTION_PUSHOFF()  x86_interrupt_pushoff()
#define PREEMPTION_POP(flag)  x86_interrupt_pop(flag)

#ifdef CONFIG_BUILDING_KERNEL_CORE
/* Load kernel segment registers.
 * Usually called at the start of an interrupt.
 * CLOBBER: %eax, %ecx
 * NOTE: This function is called early on during any kind
 *       of interrupt transition originating from user-space, or any
 *       transition period when `CONFIG_X86_LOADSEGMENTS_ONLY_USER'
 *       isn't defined. */
INTDEF void ASMCALL x86_load_segments(void);
#endif /* CONFIG_BUILDING_KERNEL_CORE */
#endif /* __CC__ */


#undef CONFIG_X86_LOADSEGMENTS_IFCHANGED
#undef CONFIG_X86_LOADSEGMENTS_IGNORE_USERTLS
#undef CONFIG_X86_LOADSEGMENTS_ONLY_USER
/* Additional configuration options for segmentation. */

/* Only load segment registers that differ from the
 * expectation when `x86_load_segments()' is called */
#ifndef CONFIG_NO_X86_LOADSEGMENTS_IFCHANGED
#define CONFIG_X86_LOADSEGMENTS_IFCHANGED 1
#endif

/* Don't restore the user-space TLS segment (%gs on i386)
 * when `x86_load_segments()' is called. */
#ifndef CONFIG_NO_X86_LOADSEGMENTS_IGNORE_USERTLS
#define CONFIG_X86_LOADSEGMENTS_IGNORE_USERTLS 1
#endif

/* Segment registers during interrupts originating from
 * within kernel space are to be trusted and do not
 * require `x86_load_segments()' to be called.
 * Note however that this isn't a rule. Any interrupt
 * is still free to disregard this option and call
 * `x86_load_segments()' regardless of where the
 * interrupt came from. */
#ifndef CONFIG_NO_X86_LOADSEGMENTS_ONLY_USER
#define CONFIG_X86_LOADSEGMENTS_ONLY_USER 1
#endif



/* Interrupt numbers. */
#define X86_INTERRUPT_SYSCALL       0x80 /* System call interrupt. */
#define X86_INTERRUPT_APIC_IPI      0xde
#define X86_INTERRUPT_APIC_SPURIOUS 0xdf
#define X86_INTERRUPT_PIC1_BASE     0xf0
#define X86_INTERRUPT_PIC2_BASE     0xf8

/* NOTE: These names must match the interrupt vectors above! */
#define X86_IRQ_APICIPI  irq_de
#define X86_IRQ_APICSPUR irq_df
#define X86_IRQ_PIT      irq_f0
#define X86_IRQ_ATA0     irq_fe
#define X86_IRQ_ATA1     irq_ff
#define X86_IRQ_KBD      irq_f1
#define X86_IRQ_PS2M     irq_fc


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_INTERRUPT_H */
