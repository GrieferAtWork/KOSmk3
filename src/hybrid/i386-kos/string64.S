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
#if 1
#include <hybrid/compiler.h>
#include <hybrid/asm.h>
#include <hybrid/host.h>
#include <hybrid/limits.h>
#include <asm/universal.h>
#include "../hybrid.h"

#ifndef CONFIG_SMALL_MEMCPY_THRESHOLD
#define CONFIG_SMALL_MEMCPY_THRESHOLD  CACHELINE
#endif

#ifdef __KERNEL__
#define RET(x) ret $ x
#else
#define RET(x) ret
#endif

#if 1
.section .text.crt.string

#ifdef __KERNEL__
#define ENTER_CLD  cld
#else
#define ENTER_CLD  /* nothing */
#endif

.cfi_startproc
INTERN_ENTRY(libc_memcpy)
	ENTER_CLD
	movq   %rdx, %rcx /* count = num_bytes */
	movq   %rdi, %rax /* result = dst */
	shrl   $3, %ecx
	rep    movsq
	jnc    1f /* if (!(num_bytes & 4)) goto 1f; */
	movsl
1:	testq  $2, %rdx
	jz     1f /* if (!(num_bytes & 2)) goto 1f; */
	movsw
1:	testq  $1, %rdx
	jz     1f /* if (!(num_bytes & 1)) goto 1f; */
	movsb
1:	ret
SYMEND(libc_memcpy)
DEFINE_PUBLIC_ALIAS(memcpy,libc_memcpy)

INTERN_ENTRY(libc_memcpyw)
	ENTER_CLD
	movq   %rdx, %rcx /* count = num_words */
	movq   %rdi, %rax /* result = dst */
	shrl   $2, %ecx
	rep    movsq
	jnc    1f /* if (!(num_words & 2)) goto 1f; */
	movsl
1:	testq  $1, %rdx
	jz     1f /* if (!(num_words & 1)) goto 1f; */
	movsw
1:	ret
SYMEND(libc_memcpyw)
DEFINE_PUBLIC_ALIAS(memcpyw,libc_memcpyw)

INTERN_ENTRY(libc_memcpyl)
	ENTER_CLD
	movq   %rdx, %rcx /* count = num_dwords */
	movq   %rdi, %rax /* result = dst */
	shrl   $1, %ecx
	rep    movsq
	jnc    1f /* if (!(num_dwords & 1)) goto 1f; */
	movsl
1:	ret
SYMEND(libc_memcpyl)
DEFINE_PUBLIC_ALIAS(memcpyl,libc_memcpyl)

#ifndef CONFIG_NO_64BIT_STRING
INTERN_ENTRY(libc_memcpyq)
	ENTER_CLD
	movq   %rdx, %rcx /* count = num_dwords */
	movq   %rdi, %rax /* result = dst */
	rep    movsq
	ret
SYMEND(libc_memcpyq)
DEFINE_PUBLIC_ALIAS(memcpyq,libc_memcpyq)
#endif /* !CONFIG_NO_64BIT_STRING */
.cfi_endproc


/* TODO: mempcpy() */
/* TODO: memset() */
/* TODO: mempset() */
/* TODO: strend() */
/* TODO: strlen() */
/* TODO: strnend() */
/* TODO: strnlen() */
/* TODO: w16end() */
/* TODO: w16len() */
/* TODO: w16nend() */
/* TODO: w16nlen() */
/* TODO: w32end() */
/* TODO: w32len() */
/* TODO: w32nend() */
/* TODO: w32nlen() */
/* TODO: memchr() */
/* TODO: memxchr() */
/* TODO: memend() */
/* TODO: memxend() */
/* TODO: memlen() */
/* TODO: memxlen() */
/* TODO: memrchr() */
/* TODO: memrxchr() */
/* TODO: memrend() */
/* TODO: memrxend() */
/* TODO: memrlen() */
/* TODO: memrxlen() */
/* TODO: rawmemchr() */
/* TODO: rawmemxchr() */
/* TODO: rawmemrchr() */
/* TODO: rawmemrxchr() */
/* TODO: rawmemlen() */
/* TODO: rawmemxlen() */
/* TODO: rawmemrlen() */
/* TODO: rawmemrxlen() */

#endif








































#endif
