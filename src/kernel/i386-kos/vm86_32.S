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
#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/asm.h>
#include <asm/cfi.h>
#include <i386-kos/vm86.h>
#include <kos/thread.h>
#include <sched/task.h>

#ifdef CONFIG_VM86
.section .text
PUBLIC_ENTRY(x86_enter_vm86)
	cli /* Disable interrupts to prevent a race condition */
	orw    $(TASK_FVM86), %taskseg:TASK_OFFSETOF_FLAGS
	movl   %ecx,  %esp /* Load the VM86 context. */
	popal
#ifdef CONFIG_X86_SEGMENTATION
	addl   $(X86_SEGMENTS32_SIZE), %esp
#endif /* CONFIG_X86_SEGMENTATION */
	iret /* Load the VM86 IRET tail. */
SYMEND(x86_enter_vm86)


PUBLIC_ENTRY(x86_leave_vm86)
	cli /* Disable interrupts to prevent a race condition */
	andw   $(~TASK_FVM86), %taskseg:TASK_OFFSETOF_FLAGS
.global cpu_setcontext
	jmp    cpu_setcontext
SYMEND(x86_leave_vm86)

#endif /* CONFIG_VM86 */








