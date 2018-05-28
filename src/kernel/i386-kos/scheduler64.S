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
#include <asm/universal.h>
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
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %eflags, -3*8
	.cfi_offset %rip,    -5*8
	irq_enter

	/* Save general-purpose registers. */
	pushq_cfi_r  %rax
	pushq_cfi_r  %rcx
	pushq_cfi_r  %rdx
	pushq_cfi_r  %rbx
	pushq_cfi_r  %rbp
	pushq_cfi_r  %rsi
	pushq_cfi_r  %rdi
	pushq_cfi_r  %r8
	pushq_cfi_r  %r9
	pushq_cfi_r  %r10
	pushq_cfi_r  %r11
	pushq_cfi_r  %r12
	pushq_cfi_r  %r13
	pushq_cfi_r  %r14
	pushq_cfi_r  %r15

	/* Acknowledge the interrupt. */
INTERN_ENTRY(x86_pic_acknowledge)
	movq x86_lapic_base_address, %rax
//	/* movq x86_lapic_base_address, %rax */
//	.byte   0xa1
//	.long   x86_lapic_base_address

	movl $APIC_EOI_FSIGNAL, APIC_EOI(%rax)
//	/* movl $APIC_EOI_FSIGNAL, APIC_EOI(%eax) */
//	.byte   0xc7
//	.byte   0x80
//	.long   APIC_EOI
//	.long   APIC_EOI_FSIGNAL
SYMEND(x86_pic_acknowledge)

	/* Increment the jiffies counter.
	 * XXX: `jiffies' should become a PER-CPU variable! */
	incq    jiffies

	testq   $3, X86_CONTEXT64_OFFSETOF_CS(%esp)
	jz      1f
	/* Use this chance to track user-space preemption statistics. */
3:	INCSTAT(ts_uswitch)
	/* Serve RPC functions if we were preempted while in user-space. */
	movq    %rsp, %rdi /* `struct cpu_hostcontext_user *__restrict context' */
	movq    $(TASK_USERCTX_FTIMER|TASK_USERCTX_TYPE_WITHINUSERCODE), %rsi
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
	movq    %taskseg:t_cpu,  %rbx  /* Load the hosting CPU */
#define CPU      0(%ebx)
#endif
	movq    c_running + CPU, %rsi  /* Load the old task. */
	movq    %rsp, t_context(%rsi)  /* Save the old CPU context. */

.check_sleeping:
	movq    c_sleeping + CPU, %rax /* Check for sleeping tasks. */
	testq   %rax, %rax
	jz      .switch_tasks
	movq    jiffies, %rcx /* Check if the task is supposed to time out. */
	cmpq    %rcx, t_timeout(%rax)
	ja      .switch_tasks /* if (t_timeout > jiffies) goto .switch_tasks; */
	andw    $(~TASK_STATE_FSLEEPING), t_state(%rax) /* Clear the sleeping flag. */
	orw     $(TASK_STATE_FTIMEDOUT),  t_state(%rax) /* Set the timed-out flag. */
	/* Remove the sleeper for the sleeping-task chain. */
	movq    t_sched + le_next(%rax), %rcx
	movq    %rcx, c_sleeping + CPU
	testq   %rcx, %rcx
	jz      1f
#ifdef CONFIG_NO_SMP
	movq    $(c_sleeping + CPU), t_sched + le_pself(%rcx)
#else
	leaq    c_sleeping + CPU, %rdx
	movq    %rdx, t_sched + le_pself(%rcx)
#endif
1:	/* Add the sleeper to the running-task chain. */
	movq    t_sched + re_next(%rsi), %rcx
	movq    %rsi, t_sched + re_prev(%rax) /* wake->t_sched.re_prev = old_task; */
	movq    %rcx, t_sched + re_next(%rax) /* wake->t_sched.re_next = old_task->t_sched.re_next; */
	movq    %rax, t_sched + re_prev(%rcx) /* wake->t_sched.re_next->t_sched.re_prev = wake; */
	movq    %rax, t_sched + re_next(%rsi) /* old_task->t_sched.re_next = wake; */
	jmp     .check_sleeping /* Check if more tasks should be woken. */

.switch_tasks:
	movq    t_sched + re_next(%rsi), %rdi /* Load the next task to switch to. */
	movq    %rdi, c_running + CPU         /* Set the new task as current. */

	/* Load the VM context of the new task and switch CPU states. */
	movq    t_vm(%rdi), %rcx
	cmpq    %rcx, t_vm(%rsi)
	je      .load_task_context_in_rdi
	movq    VM_OFFSETOF_PHYSDIR(%rcx), %rcx
	movq    %rcx, %cr3
	jmp     .load_task_context_in_rdi
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
	movq    %taskseg:t_cpu, %rdx
	cmpq    $0, c_pending(%rdx)
	je      1f
	/* Extract pending tasks from the chain. */
	xorq    %rdi, %rdi
	lock xchgq c_pending(%rdx), %rdi
	testq   %rdi, %rdi
	jz      1f          /* if (!c_pending) goto 1f; */
	/* Load the new pending tasks. */
	call    x86_scheduler_loadpending_chain
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
	pause            /* Always yield the processor. */
	pushfq_cfi_r
	testq   $(EFLAGS_IF), 0(%rsp)
	jnz     1f
	popfq_cfi_r
	/* Return `false' if interrupts have been disabled. */
99:	xorq    %rax, %rax
	stc              /* Set the carry bit. */
	ret
	.cfi_adjust_cfa_offset  8
	.cfi_rel_offset %rflags, 0
1:	INCSTAT(ts_hyield)
	popfq_cfi_r

	/* Switch to the next task. */
	cli
	movq    %taskseg:t_sched+re_next, %rdx
	cmpq    %rdx, %taskseg:t_segment+TASK_SEGMENT_OFFSETOF_SELF
	je      98f /* Same thread (Don't yield) */
#ifdef CONFIG_NO_SMP
	movq    %rdx, c_running + _boot_cpu
#else
	movq    %taskseg:t_cpu, %rax
	movq    %rdx, c_running(%rax)
#endif
	movq    $1, %rax /* Return TRUE to the caller (when the next quantum starts) */
	clc              /* Clear the carry bit. */
	jmp     x86_exchange_context
98:	sti
	jmp     99b
	.cfi_endproc
SYMEND(task_tryyield)

PUBLIC_ENTRY(task_yield)
	.cfi_startproc
	pause            /* Always yield the processor. */
	pushfq_cfi_r
	testq   $(EFLAGS_IF), (%rsp)
	jnz     1f
	popfq_cfi_r
	/* Throw a would-block error when interrupts have been disabled. */
	movq    $(E_WOULDBLOCK), %rdi
	call    error_throw_resumable
	jmp     task_yield
	.cfi_adjust_cfa_offset  8
	.cfi_rel_offset %rflags, 0
1:	INCSTAT(ts_hyield)
	popfq_cfi_r

	/* Switch to the next task. */
	cli
	movq    %taskseg:t_sched+re_next, %rdx
#if 1
	cmpq    %rdx, %taskseg:t_segment+TASK_SEGMENT_OFFSETOF_SELF
	jne     1f
	sti
	ret     /* Nothing to switch to... */
1:
#endif
#ifdef CONFIG_NO_SMP
	movq    %rdx, c_running + _boot_cpu
#else
	movq    %taskseg:t_cpu, %rax
	movq    %rdx, c_running(%rax)
#endif


	/* Apply the new context. */
INTERN_ENTRY(x86_exchange_context)
	/* Save the current CPU context. */
	subq    $16, %rsp /* for FS/GS base. */
	popq    -40(%rsp) /* RIP */
	.cfi_adjust_cfa_offset -8
	.cfi_offset %rip,      -40
	pushq_cfi   $X86_SEG_HOST_SS
	pushq_cfi   %rsp
	addq    $24, 0(%rsp) /* Return to before `c_segments' */
	pushfq_cfi_r
	orq     $(EFLAGS_IF), 0(%rsp) /* Set the #IF flag when the task resumes. */
	pushq_cfi $X86_SEG_HOST_CS
	subq    $8, %rsp /* Skip RIP */
	.cfi_adjust_cfa_offset  8
	/* With that, the IRET is now complete. */

	/* Save general purpose registers. */
	pushq_cfi_r  %rax
	pushq_cfi_r  %rcx
	pushq_cfi_r  %rdx
	pushq_cfi_r  %rbx
	pushq_cfi_r  %rbp
	pushq_cfi_r  %rsi
	pushq_cfi_r  %rdi
	pushq_cfi_r  %r8
	pushq_cfi_r  %r9
	pushq_cfi_r  %r10
	pushq_cfi_r  %r11
	pushq_cfi_r  %r12
	pushq_cfi_r  %r13
	pushq_cfi_r  %r14
	pushq_cfi_r  %r15

	/* Now we have to save the user-space FS/GS base values. */
	rdfsbaseq    %rax
	movq         %rax, X86_SCHEDCONTEXT64_OFFSETOF_SEGMENTS+X86_SEGMENTS64_OFFSETOF_FSBASE(%rsp)
	movq         $IA32_KERNEL_GS_BASE, %rcx
	rdmsr
	movl         %eax, X86_SCHEDCONTEXT64_OFFSETOF_SEGMENTS+X86_SEGMENTS64_OFFSETOF_GSBASE(%rsp)
	movl         %edx, X86_SCHEDCONTEXT64_OFFSETOF_SEGMENTS+X86_SEGMENTS64_OFFSETOF_GSBASE+4(%rsp)

	/* Save the generated CPU context in the old task. */
	movq    %rsp, %taskseg:t_context

#ifdef CONFIG_NO_SMP
	movq    c_running + _boot_cpu, %rdi /* Load the new task to switch to. */
#else
	movq    %taskseg:t_cpu,  %rbx /* Load the hosting CPU */
	movq    c_running(%rbx), %rdi /* Load the new task to switch to. */
#endif

	/* Check if the new task uses a different VM, and switch to it if it does. */
	movq    t_vm(%rdi), %rcx
	cmpq    %rcx, %taskseg:t_vm
	je      1f
	movq    VM_OFFSETOF_PHYSDIR(%rcx), %rcx
	movq    %rcx, %cr3
1:	/* We're now in the VM context of the new task. */

.load_task_context_in_rdi:
	/*     RDI: target_task
	 * #ifndef CONFIG_NO_SMP
	 *     RBX: calling_cpu
	 * #endif */

#ifndef CONFIG_NO_FPU
	/* Disable the FPU */
	movq    %cr0, %rax
	orq     $(CR0_TS), %rax
	movq    %rax, %cr0
#endif

	movq    t_stackend(%rdi), %rax
#ifdef CONFIG_NO_SMP
	movq    %rax, X86_TSS_OFFSETOF_RSP0 + x86_cputss       /* Set RSP0 */
#else
	movq    %rax, X86_TSS_OFFSETOF_RSP0 + x86_cputss(%rbx) /* Set RSP0 */
#endif

	/* Load the new CPU context */
	movq    t_context(%rdi), %rsp

	/* Update the segmentation addresses of the new task. */
	wrgsbaseq %rdi /* HOST_GS_BASE = THIS_TASK */
	movq    X86_SCHEDCONTEXT64_OFFSETOF_SEGMENTS + X86_SEGMENTS64_OFFSETOF_FSBASE(%rsp), %rax
	wrfsbaseq %rax /* USER_FS_BASE... */
	movq    $IA32_KERNEL_GS_BASE, %rcx
	movl    X86_SCHEDCONTEXT64_OFFSETOF_SEGMENTS + X86_SEGMENTS64_OFFSETOF_GSBASE(%rsp), %eax
	movl    X86_SCHEDCONTEXT64_OFFSETOF_SEGMENTS + X86_SEGMENTS64_OFFSETOF_GSBASE(%rsp), %edx
	wrmsr          /* USER_GS_BASE... */


INTERN_ENTRY(x86_load_cpu_state)
	/* Load general purpose registers. */
	popq_cfi_r %r15
	popq_cfi_r %r14
	popq_cfi_r %r13
	popq_cfi_r %r12
	popq_cfi_r %r11
	popq_cfi_r %r10
	popq_cfi_r %r9
	popq_cfi_r %r8
	popq_cfi_r %rdi
	popq_cfi_r %rsi
	popq_cfi_r %rbp
	popq_cfi_r %rbx
	popq_cfi_r %rdx
	popq_cfi_r %rcx
	popq_cfi_r %rax

	/* Return to where the task was interrupted from. */
	irq_leave
	.cfi_endproc
SYMEND(x86_load_cpu_state)
SYMEND(x86_exchange_context)
SYMEND(task_yield)

.section .text
INTERN_ENTRY(x86_load_context)
	/* struct task *rdi    = NEW_TASK */
	/* PHYS pagedir_t *rsi = OLD_PAGEDIR */
//	.cfi_startproc simple
	/* Load the page directory of the new task. */
	movq    t_vm(%rcx), %rax
	movq    VM_OFFSETOF_PHYSDIR(%rax), %rax
	cmpq    %rax, %rdx
	je      1f
	movq    %rax, %cr3
1:	movq    %rcx, %rdi
#ifndef CONFIG_NO_SMP
	movq    t_cpu(%rdi), %rbx /* Load our own CPU descriptor from the target thread. */
#endif
	jmp     .load_task_context_in_rdi
//	.cfi_endproc
SYMEND(x86_load_context)






.section .text
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %rsp,    5*8
	.cfi_offset %eflags, -3*8
	.cfi_offset %rip,    -5*8
	nop /* Keep this here for the sake of tracebacks. */
INTERN_ENTRY(x86_redirect_preemption)

	/* TODO */

.cfi_endproc
.x86_redirect_preemption_end = .
SYMEND(x86_redirect_preemption)
X86_DEFINE_INTERRUPT_GUARD(x86_redirect_preemption,.x86_redirect_preemption_end)












