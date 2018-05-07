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
#include <hybrid/asm.h>
#include <asm/cfi.h>
#include <kos/safecall.h>



#ifndef CONFIG_NO_SAFECALL
.section .text.cold
PUBLIC_ENTRY(__SAFECALL_MISSMATCH)
	.cfi_startproc
	popl_cfi %edx
	.cfi_register %eip, %edx
	movl %esp, %ecx
	movl %edx, -4(%__SAFECALL_SAFED_ESP)
	leal -4(%__SAFECALL_SAFED_ESP), %esp
	.cfi_adjust_cfa_offset 4
	.cfi_restore %eip
	/* EDX == SAFECALL_ADDRESS */
	/* EDI == TARGET_ADDRESS */
	/* ECX == ESP_INVALID */
	/* ESI == ESP_EXPECTED */
	pushl_cfi %__SAFECALL_SAFED_ESP
	pushl_cfi %ecx
	pushl_cfi %__SAFECALL_TARGET_EIP
	pushl_cfi %edx
	.cfi_rel_offset %eip, 0
DEFINE_INTERN(x86_safecall_panic)
	call x86_safecall_panic
	/* x86_safecall_panic() doesn't return... */
#if 0
	.cfi_adjust_cfa_offset -16
	ret
#endif
	.cfi_endproc
SYMEND(__SAFECALL_MISSMATCH)
#endif /* !CONFIG_NO_SAFECALL */

























