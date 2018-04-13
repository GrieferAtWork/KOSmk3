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
#include "scheduler.h"

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/asm.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/interrupt.h>
#include <kernel/paging.h>
#include <i386-kos/pic.h>
#include <i386-kos/gdt.h>
#include <i386-kos/tss.h>
#include <i386-kos/apic.h>
#include <i386-kos/interrupt.h>
#include <i386-kos/vm86.h>
#include <kernel/vm.h>
#include <sched/task.h>
#include <sched/stat.h>
#include <asm/cfi.h>
#include <asm/cpu-flags.h>


/* struct cpu { */
#define c_running    CPU_OFFSETOF_RUNNING  /* REF struct task *c_running; */
#define c_sleeping   CPU_OFFSETOF_SLEEPING /* REF struct task *c_sleeping; */
#ifndef CONFIG_NO_SMP
#define c_pending    CPU_OFFSETOF_PENDING  /* REF struct task *c_pending; */
#endif
/* }; */

/* struct task { */
#define t_segment     TASK_OFFSETOF_SEGMENT
#define t_refcnt      TASK_OFFSETOF_REFCNT
#define t_context     TASK_OFFSETOF_CONTEXT
#ifndef CONFIG_NO_SMP
#define t_cpu         TASK_OFFSETOF_CPU
#endif /* !CONFIG_NO_SMP */
#define t_vm_lock     TASK_OFFSETOF_VM_LOCK
#define t_vm          TASK_OFFSETOF_VM
#define t_vmtasks     TASK_OFFSETOF_VMTASKS
#define t_userseg     TASK_OFFSETOF_USERSEG
#define t_stackmin    TASK_OFFSETOF_STACKMIN
#define t_stackend    TASK_OFFSETOF_STACKEND
#define t_sched       TASK_OFFSETOF_SCHED
#define t_addr2limit  TASK_OFFSETOF_ADDR2LIMIT
#define t_timeout     TASK_OFFSETOF_TIMEOUT
#define t_flags       TASK_OFFSETOF_FLAGS
#define t_state       TASK_OFFSETOF_STATE
#define t_data        TASK_OFFSETOF_DATA
/* }; */

/* struct RING_NODE { */
#define re_prev       0
#define re_next       4
/* }; */

/* struct LIST_NODE { */
#define le_next       0
#define le_pself      4
/* }; */

/* struct taskstat { */
#define ts_hswitch    TASKSTATE_OFFSETOF_HSWITCH
#define ts_uswitch    TASKSTATE_OFFSETOF_USWITCH
#define ts_hyield     TASKSTATE_OFFSETOF_HYIELD
#define ts_uyield     TASKSTATE_OFFSETOF_UYIELD
#define ts_sleep      TASKSTATE_OFFSETOF_SLEEP
#define ts_xrpc       TASKSTATE_OFFSETOF_XRPC
#define ts_qrpc       TASKSTATE_OFFSETOF_QRPC
/* }; */





.section .bss.hot
PUBLIC_OBJECT(jiffies)
	.long 0
	.long 0
SYMEND(jiffies)


/* The PIT interrupt handler. */
.section .text
INTERN_ENTRY(X86_IRQ_PIT)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4

	/* Save segment registers. */
#ifdef CONFIG_X86_SEGMENTATION
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs
	pushl_cfi_r %gs
#endif /* CONFIG_X86_SEGMENTATION */

	/* Save general-purpose registers. */
	pushal_cfi_r

	call    x86_load_segments

	/* Acknowledge the interrupt. */
INTERN_ENTRY(x86_pic_acknowledge)
	/* movl x86_lapic_base_address, %eax */
	.byte   0xa1
	.long   x86_lapic_base_address

	/* movl $APIC_EOI_FSIGNAL, APIC_EOI(%eax) */
	.byte   0xc7
	.byte   0x80
	.long   APIC_EOI
	.long   APIC_EOI_FSIGNAL
SYMEND(x86_pic_acknowledge)

	/* Increment the jiffies counter.
	 * XXX: `jiffies' should become a PER-CPU variable! */
	addl    $1, jiffies
	adcl    $0, jiffies + 4

	testl   $3, X86_CONTEXT32_OFFSETOF_IRET+X86_IRREGS32_OFFSETOF_CS(%esp)
	jz      1f
	/* Use this chance to track user-space preemption statistics. */
	INCSTAT(ts_uswitch)
	/* Serve RPC functions if we were preempted while in user-space. */
	movl   %esp, %ecx /* `struct cpu_hostcontext_user *__restrict context' */
	movl    $(TASK_USERCTX_FWITHINUSERCODE), %edx
	call    task_serve_before_user
#ifdef CONFIG_NO_TASKSTAT
1:
#else
	jmp     2f
1:	INCSTAT(ts_hswitch)
2:
#endif

#ifdef CONFIG_NO_SMP
#define CPU      _boot_cpu
#else
	movl    %taskseg:t_cpu,  %ebx  /* Load the hosting CPU */
#define CPU      0(%ebx)
#endif
	movl    c_running + CPU, %esi  /* Load the old task. */
	movl    %esp, t_context(%esi)  /* Save the old CPU context. */

.check_sleeping:
	movl    c_sleeping + CPU, %eax /* Check for sleeping tasks. */
	testl   %eax, %eax
	jz      .switch_tasks
	movl    jiffies + 4, %ecx /* Check if the task is supposed to time out. */
	cmpl    %ecx, t_timeout + 4(%eax)
	ja      .switch_tasks /* if (t_timeout[1] > jiffies[1]) goto .switch_tasks; */
	jne     1f            /* if (t_timeout[1] != jiffies[1]) goto 1f; */
	movl    jiffies, %ecx /* Check if the task is supposed to time out. */
	cmpl    %ecx, t_timeout(%eax)
	ja      .switch_tasks /* if (t_timeout[0] > jiffies[0]) goto .switch_tasks; */
1:	andl    $(~TASK_STATE_FSLEEPING), t_state(%eax) /* Clear the sleeping flag. */
	orl     $(TASK_STATE_FTIMEDOUT),  t_state(%eax) /* Set the timed-out flag. */
	/* Remove the sleeper for the sleeping-task chain. */
	movl    t_sched + le_next(%eax), %ecx
	movl    %ecx, c_sleeping + CPU
	testl   %ecx, %ecx
	jz      1f
#ifdef CONFIG_NO_SMP
	movl    $(c_sleeping + CPU), t_sched + le_pself(%ecx)
#else
	leal    c_sleeping + CPU, %edx
	movl    %edx, t_sched + le_pself(%ecx)
#endif
1:	/* Add the sleeper to the running-task chain. */
	movl    t_sched + re_next(%esi), %ecx
	movl    %esi, t_sched + re_prev(%eax) /* wake->t_sched.re_prev = old_task; */
	movl    %ecx, t_sched + re_next(%eax) /* wake->t_sched.re_next = old_task->t_sched.re_next; */
	movl    %eax, t_sched + re_prev(%ecx) /* wake->t_sched.re_next->t_sched.re_prev = wake; */
	movl    %eax, t_sched + re_next(%esi) /* old_task->t_sched.re_next = wake; */
	jmp     .check_sleeping /* Check if more tasks should be woken. */

.switch_tasks:
	movl    t_sched + re_next(%esi), %edi /* Load the next task to switch to. */
	movl    %edi, c_running + CPU         /* Set the new task as current. */

	/* Load the VM context of the new task and switch CPU states. */
	movl    t_vm(%edi), %ecx
	cmpl    %ecx, t_vm(%esi)
	je      .load_task_context_in_edi
	movl    VM_OFFSETOF_PHYSDIR(%ecx), %ecx
	movl    %ecx, %cr3
	jmp     .load_task_context_in_edi
#undef CPU
	.cfi_endproc
SYMEND(X86_IRQ_PIT)




/* Safely check for pending tasks and timeouts
 * before idling until the next hardware interrupt.
 * If another task can be reawoken, wake it before returning.
 * NOTE: The caller must disable preemption beforehand. */
.section .text
INTERN_ENTRY(x86_cpu_idle)
	.cfi_startproc
#ifndef CONFIG_NO_SMP
	movl    %taskseg:t_cpu, %edx
	cmpl    $0, c_pending(%edx)
	je      1f
	/* Extract pending tasks from the chain. */
	xorl    %eax, %eax
	lock xchgl c_pending(%edx), %eax
	testl   %eax, %eax
	jz      1f          /* if (!c_pending) goto 1f; */
	pushl_cfi %eax      /* Load the new pending tasks. */
	call    x86_scheduler_loadpending_chain
	.cfi_adjust_cfa_offset -4
	/* Enable interrupts and return without halting. */
	sti
	ret
1:
#endif
	sti /* STI will enable interrupts after `hlt', meaning that there
	     * is no race-condition between this instruction and the next,
	     * that could potentially allow interrupts to be served without
	     * causing `hlt' to return. */
	hlt
	ret
	.cfi_endproc
SYMEND(x86_cpu_idle)



.section .text
PUBLIC_ENTRY(task_tryyield)
	.cfi_startproc
	pause
	pushfl_cfi_r
	testl   $(EFLAGS_IF), (%esp)
	jnz     1f
	popfl_cfi_r
	/* Return `false' if interrupts have been disabled. */
99:	xorl    %eax, %eax
	stc              /* Set the carry bit. */
	ret
	.cfi_adjust_cfa_offset  4
1:	INCSTAT(ts_hyield)
	popfl_cfi
	/* Switch to the next task. */
	cli
	movl    %taskseg:t_sched+re_next, %edx
	cmpl    %edx, %taskseg:t_segment+TASK_SEGMENT_OFFSETOF_SELF
	je      99b /* Same thread (Don't yield) */
#ifdef CONFIG_NO_SMP
	movl    %edx, c_running + _boot_cpu
#else
	movl    %taskseg:t_cpu, %eax
	movl    %edx, c_running(%eax)
#endif
	movl    $1, %eax /* Return TRUE to the caller (when the next quantum starts) */
	clc              /* Clear the carry bit. */
	jmp     x86_exchange_context
	.cfi_endproc
SYMEND(task_tryyield)

PUBLIC_ENTRY(task_yield)
	.cfi_startproc
	pushfl_cfi_r
	testl   $(EFLAGS_IF), (%esp)
	jnz     1f
	popfl_cfi_r
	/* Throw a would-block error when interrupts have been disabled. */
	movl    $(E_WOULDBLOCK), %ecx
	call    error_throw_resumable
	jmp     task_yield
	.cfi_adjust_cfa_offset  4
1:	popfl_cfi

	INCSTAT(ts_hyield)
	pause

	/* Switch to the next task. */
	cli
	movl    %taskseg:t_sched+re_next, %edx
#ifdef CONFIG_NO_SMP
	movl    %edx, c_running + _boot_cpu
#else
	movl    %taskseg:t_cpu, %eax
	movl    %edx, c_running(%eax)
#endif


	/* Apply the new context. */
INTERN_ENTRY(x86_exchange_context)
	/* Save the current CPU context. */
	popl    -12(%esp) /* EIP */
	.cfi_adjust_cfa_offset -4
	.cfi_offset %eip,      -12
	pushfl_cfi
	.cfi_offset %eflags,   -4
	orl     $(EFLAGS_IF), 0(%esp) /* Set the #IF flag when the task resumes. */
	pushl_cfi %cs
	subl    $4, %esp /* Skip EIP */
	.cfi_adjust_cfa_offset  4
	/* With that, the IRET is now complete. */

	/* Save segment registers. */
#ifdef CONFIG_X86_SEGMENTATION
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs
	pushl_cfi_r %gs
#endif /* CONFIG_X86_SEGMENTATION */

	/* Save general purpose registers. */
	pushal_cfi_r

	/* Fix the ESP value of the register state. */
	/* addl  $(X86_SEGMENTS_SIZE+X86_IRREGS_HOST32_SIZE), X86_GPREGS_OFFSETOF_ESP(%esp) */

	/* Save the generated CPU context in the old task. */
	movl    %esp, %taskseg:t_context

#ifdef CONFIG_NO_SMP
	movl    c_running + _boot_cpu, %edi /* Load the new task to switch to. */
#else
	movl    %taskseg:t_cpu,  %ebx /* Load the hosting CPU */
	movl    c_running(%ebx), %edi /* Load the new task to switch to. */
#endif

	/* Check if the new task uses a different VM, and switch to it if it does. */
	movl    t_vm(%edi), %ecx
	cmpl    %ecx, %taskseg:t_vm
	je      1f
	movl    VM_OFFSETOF_PHYSDIR(%ecx), %ecx
	movl    %ecx, %cr3
1:	/* We're now in the VM context of the new task. */

.load_task_context_in_edi:
	/*     EDI: target_task
	 * #ifndef CONFIG_NO_SMP
	 *     EBX: calling_cpu
	 * #endif */

#ifndef CONFIG_NO_FPU
	/* Disable the FPU */
	movl    %cr0, %eax
	orl     $(CR0_TS), %eax
	movl    %eax, %cr0
#endif

#ifdef CONFIG_HAVE_LDT
	/* TODO: Switch LDT. */
#endif

	/* Update the segmentation addresses of the new task. */
#ifdef CONFIG_NO_SMP
#define GDT(x)  x86_cpugdt + (x)*8
#else
#define GDT(x)  x86_cpugdt + (x)*8(%ebx)
#endif
	movl    %edi, %ecx
	movl    %edi, %edx
	shrl    $24,  %ecx
	andl    $0x00ffffff, %edx
	andl    $0xff000000, X86_SEGMENT_OFFSETOF_BASELO + GDT(X86_SEG_HOST_TLS) /* Clear out base_low */
	orl     %edx,        X86_SEGMENT_OFFSETOF_BASELO + GDT(X86_SEG_HOST_TLS) /* Set base_low */
	movb    %cl,         X86_SEGMENT_OFFSETOF_BASEHI + GDT(X86_SEG_HOST_TLS) /* Set base_hi */

	/* Load the user-space segment register. */
	movl    t_userseg(%edi), %ecx
	movl    %ecx, %edx
	shrl    $24,  %ecx
	andl    $0x00ffffff, %edx
	andl    $0xff000000, X86_SEGMENT_OFFSETOF_BASELO + GDT(X86_SEG_USER_TLS) /* Clear out base_low */
	orl     %edx,        X86_SEGMENT_OFFSETOF_BASELO + GDT(X86_SEG_USER_TLS) /* Set base_low */
	movb    %cl,         X86_SEGMENT_OFFSETOF_BASEHI + GDT(X86_SEG_USER_TLS) /* Set base_hi */
#undef GDT

#ifndef CONFIG_X86_SEGMENTATION
	/* Without segmentation to reload the FS and GS
	 * registers for us, we must reload them manually. */
	movw    $(X86_SEG_FS), %ax
	movw    %ax, %fs
	movw    $(X86_SEG_GS), %ax
	movw    %ax, %gs
#endif


	movl    t_stackend(%edi), %eax
#ifdef CONFIG_NO_SMP
	movl    %eax, X86_TSS_OFFSETOF_ESP0 + x86_cputss       /* Set ESP0 */
#else
	movl    %eax, X86_TSS_OFFSETOF_ESP0 + x86_cputss(%ebx) /* Set ESP0 */
#endif

	/* Load the new CPU context */
	movl    t_context(%edi), %esp

INTERN_ENTRY(x86_load_cpu_state)
	/* Load general purpose registers. */
	popal_cfi_r

	/* Load segment registers. */
#ifdef CONFIG_X86_SEGMENTATION
	popl_cfi_r %gs
	popl_cfi_r %fs
	popl_cfi_r %es
	popl_cfi_r %ds
#endif /* CONFIG_X86_SEGMENTATION */

	/* Do an IRET */
	iret
	.cfi_endproc
SYMEND(x86_load_cpu_state)
SYMEND(x86_exchange_context)
SYMEND(task_yield)

.section .text
INTERN_ENTRY(x86_load_context)
	/* struct task *ecx */
	.cfi_startproc simple
	/* Load the page directory of the new task. */
	movl    t_vm(%ecx), %eax
	movl    VM_OFFSETOF_PHYSDIR(%eax), %eax
	cmpl    %eax, %edx
	je      1f
	movl    %eax, %cr3
1:	movl    %ecx, %edi
#ifndef CONFIG_NO_SMP
	movl    t_cpu(%edi), %ebx /* Load our own CPU descriptor from the target thread. */
#endif
	jmp     .load_task_context_in_edi
	.cfi_endproc
SYMEND(x86_load_context)






.section .text
INTERN_ENTRY(x86_redirect_preemption)
	.cfi_startproc simple
	/* At this point, we're 8 bytes from the base of the kernel-stack
	 * with those 8 bytes (supposedly) containing the user-space ESP
	 * and SS.
	 * However, we will override those registers irregardless in case
	 * `x86_interrupt_getiret()' was used to modify them in their backup. */
#if 1
	.cfi_signal_frame
	.cfi_def_cfa %esp,     0 /* We must hide the 8 bytes from `cfi_signal_frame' */
#else
	.cfi_def_cfa %esp,     8
#endif
	subl   $12, %esp /* Make space for the missing part of the IRET tail. */
	.cfi_adjust_cfa_offset 12
/*	.cfi_offset %ss,     -1*4 */
	.cfi_offset %esp,    -2*4

	/* Save segment registers. */
#ifdef CONFIG_X86_SEGMENTATION
	pushl_cfi_r %ds
	pushl_cfi_r %es
	pushl_cfi_r %fs
	pushl_cfi_r %gs
#endif /* CONFIG_X86_SEGMENTATION */

	/* Save user-space registers. */
	pushal_cfi_r

#ifdef CONFIG_X86_SEGMENTATION
#define CFA_OFFSET   68 /* 4*8 + 4*4 + 12 + 8 */
#else
#define CFA_OFFSET   52 /* 4*8 + 12 + 8 */
#endif
#define IRET_OFFSET  (CFA_OFFSET-20) /* - 5*4 (sizeof(struct x86_irregs_user32)) */

#ifdef CONFIG_X86_SEGMENTATION
	/* Since user-space segments had already been
	 * restore before our redirection was invoked,
	 * we must load kernel segments once again.
	 * Note however that when segmentation is disabled,
	 * kernel-compatible segments will still be active,
	 * meaning that in that case we don't have to load
	 * them yet again. */
	call   x86_load_segments
#endif

	/* Now copy the saved IRET tail onto our stack.
	 * (The 12 bytes subtracted above + 8 bytes left dangling)
	 * NOTE: The lower 8 bytes likely haven't changed. However,
	 *       they are allowed to (and actually do for some
	 *       system calls), so we must still load them all! */
	movl   %fs:iret_saved + 0*4, %eax    /* EIP (in user-space) */
	movl   %eax, IRET_OFFSET + 0*4(%esp)
	.cfi_offset %eip,    -5*4
	movl   %fs:iret_saved + 1*4, %eax    /* CS (should be X86_USER_CS) */
	movl   %eax, IRET_OFFSET + 1*4(%esp)
/*	.cfi_offset %cs,     -4*4 */
	movl   %fs:iret_saved + 2*4, %eax    /* EFLAGS */
	movl   %eax, IRET_OFFSET + 2*4(%esp)
	.cfi_offset %eflags, -3*4
	movl   %fs:iret_saved + 3*4, %eax    /* USERESP */
	movl   %eax, IRET_OFFSET + 3*4(%esp)
	movl   %fs:iret_saved + 4*4, %eax    /* SS (should be X86_USER_DS) */
	movl   %eax, IRET_OFFSET + 4*4(%esp)


	/* The original (and possibly modified) user-space IRET tail
	 * has now been restored. As far as anyone is concered, at this
	 * point, it was never overwritten to begin with and we can
	 * safely re-enable preemption before moving on to serve RPC
	 * functions that were scheduled for execution.
	 * In other words, with the IRET EIP no longer pointing to
	 * `x86_redirect_preemption()', `x86_interrupt_getiret()' will
	 * once again return a pointer to the user-space IRET tail
	 * that is stored at the base of the kernel stack. */
	sti

	movl   %esp, %ecx /* `struct cpu_hostcontext_user *__restrict context' */
	movl   $(TASK_USERCTX_FAFTERSYSCALL), %edx
	call   task_serve_before_user

	/* Restore user-space registers. */
	popal_cfi_r

	/* Load segment registers. */
#ifdef CONFIG_X86_SEGMENTATION
	popl_cfi_r %gs
	popl_cfi_r %fs
	popl_cfi_r %es
	popl_cfi_r %ds
#endif /* CONFIG_X86_SEGMENTATION */

	/* Jump to user-space (likely; we may just end up
	 * back above at the entry of `x86_redirect_preemption()'
	 * if further interrupts were scheduled before returning
	 * to user-space) */
	iret
	.cfi_endproc
.x86_redirect_preemption_end = .
SYMEND(x86_redirect_preemption)
X86_DEFINE_INTERRUPT_GUARD(x86_redirect_preemption,.x86_redirect_preemption_end)












