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
#include <kos/rpc.h>
#include <kos/thread.h>
#include <errno.h>
#include <except.h>


.section .text.crt.tls
PUBLIC_ENTRY(___tls_get_addr)
	.cfi_startproc
	testl     $1, 0(%eax)
	jz        1f
	/* TLS memory has been allocated at a static offset! */
	movl      0(%eax), %edx
	sarl      $1, %edx
	movl      4(%eax),    %eax /* ti_tlsoffset */
	addl      %edx,       %eax /* MODULE_TLS_OFFSET */
	addl      %taskseg:0, %eax /* THREAD_SEGMENT_ADDRESS */
	ret
1:	movl      %eax, %ecx
DEFINE_INTERN(x86_dynamic_tls_addr)
	call      x86_dynamic_tls_addr
	ret
	.cfi_endproc
SYMEND(___tls_get_addr)





















