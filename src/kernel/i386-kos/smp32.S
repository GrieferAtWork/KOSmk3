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
#include <i386-kos/ipi.h>
#include <kernel/vm.h>
#include <sched/task.h>
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


/* The APIC spurious interrupt handler. */
.section .text
INTERN_ENTRY(X86_IRQ_APICSPUR)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4
	pushl_cfi_r %eax
	pushl_cfi_r %ecx
	pushl_cfi_r %edx

#ifndef CONFIG_X86_FIXED_SEGMENTATION
	pushl_cfi_r %ds
	pushl_cfi_r %es
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
	pushl_cfi_r %fs
	pushl_cfi_r %gs

	call    x86_load_segments
	call    x86_apic_spur
	movl    x86_lapic_base_address, %eax
	movl    $(APIC_EOI_FSIGNAL), APIC_EOI(%eax)

	popl_cfi_r  %gs
	popl_cfi_r  %fs
#ifndef CONFIG_X86_FIXED_SEGMENTATION
	popl_cfi_r  %es
	popl_cfi_r  %ds
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */

	popl_cfi_r  %edx
	popl_cfi_r  %ecx
	popl_cfi_r  %eax
	iret
	.cfi_endproc
SYMEND(X86_IRQ_APICSPUR)



#ifndef CONFIG_NO_SMP
/* The IPI interrupt handler. */
.section .text
INTERN_ENTRY(X86_IRQ_APICIPI)
	.cfi_startproc simple
	.cfi_signal_frame
	.cfi_def_cfa %esp,    3*4
	.cfi_offset %eflags, -1*4
	.cfi_offset %eip,    -3*4
	/* Save callee-scratch registers. */
	pushl_cfi_r %eax
	pushl_cfi_r %ecx
	pushl_cfi_r %edx

#ifndef CONFIG_X86_FIXED_SEGMENTATION
	pushl_cfi_r %ds
	pushl_cfi_r %es
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */
	pushl_cfi_r %fs
	pushl_cfi_r %gs

	call    x86_load_segments

	/* Process IPIs. */
	call    x86_ipi_process

	/* If `x86_ipi_process' already acknowledged
	 * the IPI, then we mustn't do so again. */
	testl   %eax, %eax
	jnz     1f

	/* Acknowledge the IPI in our LAPIC and check for additional
	 * IPIs that may have arrived after `x86_ipi_process()'
	 * returned, but before we acknowledged the interrupt.
	 * If we didn't do this check, there would be a race
	 * condition where IPIs could get lost in the abyss,
	 * leaving the kernel in a soft-lock state. */
	movl    x86_lapic_base_address, %eax
	movl    $(APIC_EOI_FSIGNAL), APIC_EOI(%eax)
	movl    %taskseg:t_cpu, %eax
	cmpl    $0, ipi_valid(%eax)
	je      1f
	call    x86_ipi_process
1:

	popl_cfi_r  %gs
	popl_cfi_r  %fs
#ifndef CONFIG_X86_FIXED_SEGMENTATION
	popl_cfi_r  %es
	popl_cfi_r  %ds
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */

	popl_cfi_r  %edx
	popl_cfi_r  %ecx
	popl_cfi_r  %eax
	iret
	.cfi_endproc
SYMEND(X86_IRQ_APICIPI)
#endif







#ifndef CONFIG_NO_SMP
.section .text.free
INTERN_ENTRY(x86_smp_task0_entry)
	/* This is the task-level entry point for CPUs */
	call    x86_percpu_initialize
	jmp     x86_secondary_cpu_idle_loop
SYMEND(x86_smp_task0_entry)


.section .text
	/* This the loop executed by the IDLE-task that
	 * exists in all CPUs other than the boot CPU. */
INTERN_ENTRY(x86_secondary_cpu_idle_loop_error)
	/* Fix the expected register state and handle the exception. */
	movl    %taskseg:t_stackend, %esp
	call    x86_cpu_idle_exception
	movl    %taskseg:TASK_SEGMENT_OFFSETOF_SELF, %esi
	movl    t_cpu(%esi), %edx
INTERN_ENTRY(x86_secondary_cpu_idle_loop)
	/* EBX: THIS_CPU */
	/* ESI: THIS_TASK */
	/* Try to yield to other running tasks. */
	cli
1:	cmpl    t_sched+re_next(%esi), %esi
	jne     .switch_to_next
	/* Check for pending tasks. */
	cmpl    $0, c_pending(%ebx)
	je      .no_pending
	/* Extract pending tasks from the chain. */
	xorl    %edi, %edi
	lock xchgl c_pending(%ebx), %edi
	testl   %edi, %edi
	jz      .no_pending /* if (!c_pending) goto .no_pending; */
	pushl   %edi        /* Load the new pending tasks. */
	pushl   $1b /* Return address for `x86_scheduler_loadpending_chain()' */
	jmp     x86_scheduler_loadpending_chain
.no_pending:

	/* Serve RPC functions in the calling thread. */
	testw   $(TASK_STATE_FINTERRUPTED), t_state(%esi)
	jnz     .serve_rpc_functions

	/* Re-enable interrupts and wait for preemption.
	 * XXX: Disable PIT? */
	sti
	hlt

	jmp     x86_secondary_cpu_idle_loop
.switch_to_next:
	/* Switch to the next task. */
	movl    t_sched+re_next(%esi), %edi
	movl    %edi, c_running(%ebx)
	call    x86_exchange_context
	jmp     x86_secondary_cpu_idle_loop
.serve_rpc_functions:
	sti
	call    task_serve
	jmp     x86_secondary_cpu_idle_loop
SYMEND(x86_secondary_cpu_idle_loop)
.x86_secondary_cpu_idle_loop_end = .


DEFINE_EXCEPTION_HANDLER(
	x86_secondary_cpu_idle_loop,
	.x86_secondary_cpu_idle_loop_end,
	idle_loop_except_handler,
	EXCEPTION_HANDLER_FDESCRIPTOR,
	0
)

.section .rodata
DEFINE_EXCEPTION_DESCRIPTOR(
	idle_loop_except_handler,
	x86_secondary_cpu_idle_loop_error,
	EXCEPTION_DESCRIPTOR_TYPE_BYPASS,
	EXCEPTION_DESCRIPTOR_FDEALLOC_CONTINUE,
	0
)

#endif /* !CONFIG_NO_SMP */



#ifndef CONFIG_NO_SMP
.section .text.free
.code16
#define X86_SMP_ENTRY_IP   0x2000
#define S(x) (X86_SMP_ENTRY_IP + ((x) - x86_smp_entry))
INTERN_ENTRY(x86_smp_entry)
	/* NOTE: This code is actually executed at physical address `X86_SMP_ENTRY_IP' */
	cli
	lgdt    S(x86_smp_gdt)
	movl    %cr0, %eax
	orb     $(CR0_PE), %al
	movl    %eax, %cr0
	ljmpl   $8, $(x86_smp_entry32 - KERNEL_BASE)
.code32
INTERN_OBJECT(x86_smp_gdt)
	.word   3*8-1
	.long   1f - KERNEL_BASE
1:	.long   __X86_SEG_ENCODELO(0,0,0)
	.long   __X86_SEG_ENCODEHI(0,0,0)
	.long   __X86_SEG_ENCODELO(0,X86_SEG_LIMIT_MAX,X86_SEG_CODE_PL0)
	.long   __X86_SEG_ENCODEHI(0,X86_SEG_LIMIT_MAX,X86_SEG_CODE_PL0)
	.long   __X86_SEG_ENCODELO(0,X86_SEG_LIMIT_MAX,X86_SEG_DATA_PL0)
	.long   __X86_SEG_ENCODEHI(0,X86_SEG_LIMIT_MAX,X86_SEG_DATA_PL0)
SYMEND(x86_smp_gdt)
SYMEND(x86_smp_entry)
#undef S
x86_smp_entry_end = .
.hidden x86_smp_entry_end
.global x86_smp_entry_end

INTERN_ENTRY(x86_smp_entry32)
	movw    $16, %cx
	movw    %cx, %ds
	movw    %cx, %es
	movw    %cx, %ss
	/* Enable paging. */
	leal    pagedir_kernel_phys, %ecx
	movl    %ecx, %cr3
	orl     $(CR0_PG), %eax
	movl    %eax, %cr0
	movl    x86_lapic_base_address, %eax
	movl    APIC_ID(%eax), %eax
	shrl    $(APIC_ID_FSHIFT), %eax
	/* Find the CPU how's APIC_ID matches `%al' (that's us then) */
	movl    cpu_count, %ecx
	subl    $1, %ecx
	jz      .x86_smp_kill
1:	subl    $1, %ecx
	movl    cpu_vector + 4(,%ecx,4), %ebx
	movb    x86_lapic_id(%ebx), %dl
	cmpb    %dl, %al
	je      1f
	testl   %ecx, %ecx
	jnz     1b
	jmp     .x86_smp_kill
1:

	/* Tell the boot-cpu that we're now online. */
	incl    %ecx
	movl    %ecx, %eax
	andl    $7,   %ecx
	movl    $1,   %edx
	shrl    $3,   %eax /* EAX = CPU_ID / 8 */
	shll    %cl,  %edx /* EDX = 1 << (CPU_ID % 8) */
	not     %edx       /* EDX = ~(1 << (CPU_ID % 8)) */
	andb    %dl,  cpu_offline_mask(,%eax,1)

	/* Found our CPU (now stored in %EBX) */
	movl    c_running(%ebx), %edi /* TARGET task. */
	movl    t_context(%edi), %esp /* stack */

	/* Load the GDT of our new CPU */
	leal    x86_cpugdt(%ebx), %ecx
	pushl   %ecx
	pushw   $((X86_SEG_BUILTIN * 8) - 1)
	lgdt    (%esp)
	addl    $6, %esp

	/* Load the Task register. */
	movw    $(X86_SEG(X86_SEG_CPUTSS)), %ax
	ltr     %ax

	/* Load basic segments. */
	movw    $(X86_SEG_DS), %ax
	movw    %ax, %ds
	movw    %ax, %es
	movw    $(X86_KERNEL_DS), %ax
	movw    %ax, %ss
	/* NOTE: This also does the jump from physical into virtual memory! */
	ljmp    $(X86_KERNEL_CS), $1f
1:

	/* Load the interrupt descriptor table. */
	pushl   $_idt_start
	pushw   $((256 * X86_IDTENTRY_SIZE) - 1)
	lidt    (%esp)
	addl    $6, %esp

	/* Load the VM of the target task. */
	movl    t_vm(%edi), %eax
	cmpl    $(vm_kernel), %eax
	je      1f
	movl    VM_OFFSETOF_PHYSDIR(%eax), %eax
	movl    %eax, %cr3

1:	/* Finally, jump to load the CPU state pointed by to ESP.
	 *  -> That state should then lead us into
	 *     the `x86_secondary_cpu_idle_loop' loop */
	jmp     x86_load_cpu_state

.x86_smp_kill:
	/* Shouldn't really get here... */
	cli
	hlt
	jmp     .x86_smp_kill
SYMEND(x86_smp_entry32)

#endif /* !CONFIG_NO_SMP */
