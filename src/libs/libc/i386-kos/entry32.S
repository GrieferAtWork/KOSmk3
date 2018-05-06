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
#include "../libc.h"

#include <hybrid/compiler.h>
#include <hybrid/asm.h>
#include <asm/cfi.h>
#include <syscall.h>
#include <kos/ushare.h>
#include <kos/thread.h>
#include <kos/environ.h>


DEFINE_INTERN(libc_runexit)
DEFINE_INTERN(libc_runquickexit)
DEFINE_INTERN(libc_internal_onexit)
DEFINE_INTERN(_libc_exit)
DEFINE_INTERN(libc_entry_noproc)
DEFINE_INTERN(__x86.get_pc_thunk.bx)

.section .text.hot
/* NOTE: No CFI information, because trying to unwind this one'd be pointless... */
INTERN_ENTRY(libc_entry)
	/* ECX: `pmain main' */
	call     __x86.get_pc_thunk.bx
	addl     $_GLOBAL_OFFSET_TABLE_, %ebx
	movl     %taskseg:TASK_SEGMENT_OFFSETOF_PROCESS, %eax
	testl    %eax, %eax
	jz       libc_entry_noproc /* Shouldn't happen */
	/* Load arguments for main() (with the least possible stack-usage upon entry) */
	movl     PROCESS_ENVIRON_OFFSETOF_ENVP(%eax), %edx
	movl     %edx, environ@GOT(%ebx)
	pushl    %edx
	pushl    PROCESS_ENVIRON_OFFSETOF_ARGV(%eax)
	pushl    PROCESS_ENVIRON_OFFSETOF_ARGC(%eax)
	/* Invoke main() */
INTERN_ENTRY(libc_invoke_main)
	call    *%ecx
	addl     $12, %esp
	pushl    %eax /* The exit argument for all the calls below. */
1:	/* Invoke exit callbacks. */
	call     libc_runexit
	call     libc_runquickexit
	call     libc_internal_onexit
	/* Terminate the calling application. */
	call     _libc_exit
INTERN_ENTRY(libc_exit)
	addl     $4, %esp /* Return address. */
	jmp      1b
SYMEND(libc_exit)
SYMEND(libc_invoke_main)
SYMEND(libc_entry)

.section .bss.cold
PRIVATE_ENTRY(empty_vector)
	.long   0
SYMEND(empty_vector)

.section .text.cold
INTERN_ENTRY(libc_entry_noproc)
	leal     empty_vector@GOTOFF(%ebx), %edx
	movl     %edx, environ@GOT(%ebx)
	pushl    %edx
	pushl    %edx
	pushl    $0
	jmp      libc_invoke_main
SYMEND(libc_entry_noproc)

EXPORT(__entry1,libc_entry)
EXPORT(exit,libc_exit)




/* Thread entry point. */
.section .text
INTERN_ENTRY(libc_thread_entry)
	pushl    %edx             /* arg */
	call    *%ecx             /* (*fn)(arg) */
	movl     %eax, (%esp)     /* result = (*fn)(arg); */
DEFINE_INTERN(sys_exit)
	call     sys_exit         /* exit(result); // NOTE: Only exits the thread; not the process. */
SYMEND(libc_thread_entry)
















