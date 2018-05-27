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
#ifndef GUARD_KERNEL_INCLUDE_I386_KOS_SCHEDULER_H
#define GUARD_KERNEL_INCLUDE_I386_KOS_SCHEDULER_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/sections.h>
#include <hybrid/xch.h>
#include <asm/param.h>
#include <kos/i386-kos/asm/pf-syscall.h>
#include <i386-kos/interrupt.h>
#include <i386-kos/vm86.h>
#ifdef __x86_64__
#include <i386-kos/gdt.h>
#endif

DECL_BEGIN

#ifndef HZ
#ifdef CONFIG_HZ
#define HZ CONFIG_HZ
#else
#define HZ 20
#endif
#endif
#ifndef CONFIG_HZ
#define CONFIG_HZ HZ
#endif


#ifdef __CC__

/* The kernel-space equivalent of the `ts_x86sysbase'
 * field found in the user-space per-task data segment.
 * Used to implement `sigreturn' without the need of mapping additional
 * code into user-space, or hiding that code in some USHARE segment... */
DATDEF ATTR_PERTASK uintptr_t x86_sysbase;

/* Generate a new, random value for `x86_sysbase' */
#define X86_SYSBASE_RAND()  \
 (KERNEL_BASE+((rand() % (0x01000000-X86_ENCODE_PFSYSCALL_SIZE)) & ~(16-1)))


/* Get the effective user-space return location
 * where execution will continue after returning
 * back to user-space.
 * WARNING: Preemption must be disabled when calling
 *          these functions as the effective IRET tail
 *          may change when an RPC function redirects
 *          the interrupt return address to insert a
 *          call to `task_serve_before_user()'
 *          However, regardless of whether or not this
 *          happened, these functions return a pointer
 *          to the set of registers as they describe
 *          user-space.
 * WARNING: Kernel-worker threads must not call these
 *          functions (as they never return to user-space).
 * As far as synchronization goes: The returned memory segment is
 * private to the calling thread. However, another thread on the
 * same CPU may write to it and exchange it for another segment
 * that will then be returned by this function instead.
 * With that in mind, always call this function with interrupts
 * disabled, and keep them disabled while you write to that pointer.
 * If you do that, you can assume that the pointer might change, but
 * the data that it points to will only change if _you_ write to it. */
FUNDEF NOIRQ ATTR_RETNONNULL struct x86_irregs_user *KCALL x86_interrupt_getiret(void);


/* Arch-independent interrupt-return information. */
FORCELOCAL bool KCALL interrupt_returns_to_user(void);
FORCELOCAL USER uintptr_t KCALL interrupt_getip(void);
FORCELOCAL USER uintptr_t KCALL interrupt_getsp(void);
FORCELOCAL USER uintptr_t KCALL interrupt_setip(uintptr_t newip);
FORCELOCAL USER uintptr_t KCALL interrupt_setsp(uintptr_t newsp);
FORCELOCAL void KCALL interrupt_getipsp(uintptr_t *__restrict pip, uintptr_t *__restrict psp);
FORCELOCAL void KCALL interrupt_setipsp(uintptr_t newip, uintptr_t newsp);

#ifdef __x86_64__
/* Return true if the interrupted task was
 * running in compatibility (32-bit) mode. */
FORCELOCAL bool KCALL interrupt_iscompat(void) {
 bool result;
 struct x86_irregs_user *iret;
 pflag_t was = PREEMPTION_PUSHOFF();
 iret = x86_interrupt_getiret();
 result = iret->ir_cs == X86_SEG_USER_CS32;
 PREEMPTION_POP(was);
 return result;
}
#else
#define interrupt_iscompat() true
#endif


FORCELOCAL bool KCALL interrupt_returns_to_user(void) {
 bool result;
 struct x86_irregs_user *iret;
 pflag_t was = PREEMPTION_PUSHOFF();
 iret = x86_interrupt_getiret();
#ifdef CONFIG_VM86
 result = (iret->ir_cs & 3) || (iret->ir_eflags & 0x20000);
#else
 result = (iret->ir_cs & 3) != 0;
#endif
 PREEMPTION_POP(was);
 return result;
}
FORCELOCAL USER uintptr_t KCALL interrupt_getip(void) {
 USER uintptr_t result;
 pflag_t was = PREEMPTION_PUSHOFF();
#ifdef __x86_64__
 result = x86_interrupt_getiret()->ir_rip;
#else
 result = x86_interrupt_getiret()->ir_eip;
#endif
 PREEMPTION_POP(was);
 return result;
}
FORCELOCAL USER uintptr_t KCALL interrupt_getsp(void) {
 USER uintptr_t result;
 pflag_t was = PREEMPTION_PUSHOFF();
#ifdef __x86_64__
 result = x86_interrupt_getiret()->ir_rsp;
#else
 result = x86_interrupt_getiret()->ir_useresp;
#endif
 PREEMPTION_POP(was);
 return result;
}
FORCELOCAL USER uintptr_t KCALL interrupt_setip(uintptr_t newip) {
 USER uintptr_t result;
 pflag_t was = PREEMPTION_PUSHOFF();
#ifdef __x86_64__
 result = XCH(x86_interrupt_getiret()->ir_rip,newip);
#else
 result = XCH(x86_interrupt_getiret()->ir_eip,newip);
#endif
 PREEMPTION_POP(was);
 return result;
}
FORCELOCAL USER uintptr_t KCALL interrupt_setsp(uintptr_t newsp) {
 USER uintptr_t result;
 pflag_t was = PREEMPTION_PUSHOFF();
#ifdef __x86_64__
 result = XCH(x86_interrupt_getiret()->ir_rsp,newsp);
#else
 result = XCH(x86_interrupt_getiret()->ir_useresp,newsp);
#endif
 PREEMPTION_POP(was);
 return result;
}
FORCELOCAL void KCALL
interrupt_getipsp(uintptr_t *__restrict pip, uintptr_t *__restrict psp) {
 pflag_t was = PREEMPTION_PUSHOFF();
#ifdef __x86_64__
 *pip = x86_interrupt_getiret()->ir_rip;
 *psp = x86_interrupt_getiret()->ir_rsp;
#else
 *pip = x86_interrupt_getiret()->ir_eip;
 *psp = x86_interrupt_getiret()->ir_useresp;
#endif
 PREEMPTION_POP(was);
}
FORCELOCAL void KCALL
interrupt_setipsp(uintptr_t newip, uintptr_t newsp) {
 pflag_t was = PREEMPTION_PUSHOFF();
#ifdef __x86_64__
 x86_interrupt_getiret()->ir_rip = newip;
 x86_interrupt_getiret()->ir_rsp = newsp;
#else
 x86_interrupt_getiret()->ir_eip     = newip;
 x86_interrupt_getiret()->ir_useresp = newsp;
#endif
 PREEMPTION_POP(was);
}


#endif


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_I386_KOS_SCHEDULER_H */
