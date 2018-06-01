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

INTERN_ENTRY(libc_mempcpy)
	ENTER_CLD
	movq   %rdx, %rcx /* count = num_bytes */
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
1:	movq   %rdi, %rax /* result = dst */
	ret
SYMEND(libc_mempcpy)
DEFINE_PUBLIC_ALIAS(mempcpy,libc_mempcpy)

INTERN_ENTRY(libc_mempcpyw)
	ENTER_CLD
	movq   %rdx, %rcx /* count = num_words */
	shrl   $2, %ecx
	rep    movsq
	jnc    1f /* if (!(num_words & 2)) goto 1f; */
	movsl
1:	testq  $1, %rdx
	jz     1f /* if (!(num_words & 1)) goto 1f; */
	movsw
1:	movq   %rdi, %rax /* result = dst */
	ret
SYMEND(libc_mempcpyw)
DEFINE_PUBLIC_ALIAS(mempcpyw,libc_mempcpyw)

INTERN_ENTRY(libc_mempcpyl)
	ENTER_CLD
	movq   %rdx, %rcx /* count = num_dwords */
	shrl   $1, %ecx
	rep    movsq
	jnc    1f /* if (!(num_dwords & 1)) goto 1f; */
	movsl
1:	movq   %rdi, %rax /* result = dst */
	ret
SYMEND(libc_mempcpyl)
DEFINE_PUBLIC_ALIAS(mempcpyl,libc_mempcpyl)

#ifndef CONFIG_NO_64BIT_STRING
INTERN_ENTRY(libc_mempcpyq)
	ENTER_CLD
	movq   %rdx, %rcx /* count = num_dwords */
	rep    movsq
	movq   %rdi, %rax /* result = dst */
	ret
SYMEND(libc_mempcpyq)
DEFINE_PUBLIC_ALIAS(mempcpyq,libc_mempcpyq)
#endif /* !CONFIG_NO_64BIT_STRING */

INTERN_ENTRY(libc_memset)
	ENTER_CLD
	movq   %rdx, %rcx /* num_bytes */
	movzbq %sil, %rax
	movb   %al,  %ah
	movw   %ax,  %si
	shll   $16,  %eax
	movw   %si,  %ax
	movzlq %eax, %rsi
	shlq   $32,  %rax
	orq    %rsi, %rax /* RAX = 0x0101010101010101 * BYTE; */
	shrq   $3, %rcx
	movq   %rdi, %rsi
	rep    stosq
	jnc    1f /* if (!(num_bytes & 4)) goto 1f; */
	stosl
1:	testq  $2, %rdx
	jz     1f /* if (!(num_bytes & 2)) goto 1f; */
	stosw
1:	testq  $1, %rdx
	jz     1f /* if (!(num_bytes & 1)) goto 1f; */
	stosb
1:	movq   %rsi, %rax
	ret
SYMEND(libc_memset)
DEFINE_PUBLIC_ALIAS(memset,libc_memset)

INTERN_ENTRY(libc_memsetw)
	ENTER_CLD
	movq   %rdx, %rcx /* num_words */
	movzwq %si,  %rax
	shll   $16,  %eax
	movw   %si,  %ax
	movzlq %eax, %rsi
	shlq   $32,  %rax
	orq    %rsi, %rax /* RAX = 0x0001000100010001 * WORD; */
	shrq   $2,   %rcx
	movq   %rdi, %rsi
	rep    stosq
	jnc    1f /* if (!(num_words & 2)) goto 1f; */
	stosl
1:	testq  $1, %rdx
	jz     1f /* if (!(num_words & 1)) goto 1f; */
	stosw
1:	movq   %rsi, %rax
	ret
SYMEND(libc_memsetw)
DEFINE_PUBLIC_ALIAS(memsetw,libc_memsetw)

INTERN_ENTRY(libc_memsetl)
	ENTER_CLD
	movq   %rdx, %rcx /* num_dwords */
	movzlq %esi, %rax
	shlq   $32,  %rsi
	orq    %rsi, %rax /* RAX = 0x0000000100000001 * DWORD; */
	shrq   $1,   %rcx
	movq   %rdi, %rsi
	rep    stosq
	jnc    1f /* if (!(num_dwords & 1)) goto 1f; */
	stosl
1:	movq   %rsi, %rax
	ret
SYMEND(libc_memsetl)
DEFINE_PUBLIC_ALIAS(memsetl,libc_memsetl)

#ifndef CONFIG_NO_64BIT_STRING
INTERN_ENTRY(libc_memsetq)
	ENTER_CLD
	movq   %rdx, %rcx /* num_qwords */
	movq   %rsi, %rax
	movq   %rdi, %rsi
	rep    stosq
	movq   %rsi, %rax
	ret
SYMEND(libc_memsetq)
DEFINE_PUBLIC_ALIAS(memsetq,libc_memsetq)
#endif


INTERN_ENTRY(libc_mempset)
	ENTER_CLD
	movq   %rdx, %rcx /* num_bytes */
	movzbq %sil, %rax
	movb   %al,  %ah
	movw   %ax,  %si
	shll   $16,  %eax
	movw   %si,  %ax
	movzlq %eax, %rsi
	shlq   $32,  %rax
	orq    %rsi, %rax /* RAX = 0x0101010101010101 * BYTE; */
	shrq   $3, %rcx
	rep    stosq
	jnc    1f /* if (!(num_bytes & 4)) goto 1f; */
	stosl
1:	testq  $2, %rdx
	jz     1f /* if (!(num_bytes & 2)) goto 1f; */
	stosw
1:	testq  $1, %rdx
	jz     1f /* if (!(num_bytes & 1)) goto 1f; */
	stosb
1:	movq   %rdi, %rax
	ret
SYMEND(libc_mempset)
DEFINE_PUBLIC_ALIAS(mempset,libc_mempset)

INTERN_ENTRY(libc_mempsetw)
	ENTER_CLD
	movq   %rdx, %rcx /* num_words */
	movzwq %si,  %rax
	shll   $16,  %eax
	movw   %si,  %ax
	movzlq %eax, %rsi
	shlq   $32,  %rax
	orq    %rsi, %rax /* RAX = 0x0001000100010001 * WORD; */
	shrq   $2,   %rcx
	rep    stosq
	jnc    1f /* if (!(num_words & 2)) goto 1f; */
	stosl
1:	testq  $1, %rdx
	jz     1f /* if (!(num_words & 1)) goto 1f; */
	stosw
1:	movq   %rdi, %rax
	ret
SYMEND(libc_mempsetw)
DEFINE_PUBLIC_ALIAS(mempsetw,libc_mempsetw)

INTERN_ENTRY(libc_mempsetl)
	ENTER_CLD
	movq   %rdx, %rcx /* num_dwords */
	movzlq %esi, %rax
	shlq   $32,  %rsi
	orq    %rsi, %rax /* RAX = 0x0000000100000001 * DWORD; */
	shrq   $1,   %rcx
	rep    stosq
	jnc    1f /* if (!(num_dwords & 1)) goto 1f; */
	stosl
1:	movq   %rdi, %rax
	ret
SYMEND(libc_mempsetl)
DEFINE_PUBLIC_ALIAS(mempsetl,libc_mempsetl)

#ifndef CONFIG_NO_64BIT_STRING
INTERN_ENTRY(libc_mempsetq)
	ENTER_CLD
	movq   %rdx, %rcx /* num_qwords */
	movq   %rsi, %rax
	rep    stosq
	movq   %rdi, %rax
	ret
SYMEND(libc_mempsetq)
DEFINE_PUBLIC_ALIAS(mempsetq,libc_mempsetq)
#endif

INTERN_ENTRY(libc_strend)
	ENTER_CLD
	movq   $-1, %rcx
	xorq   %rax, %rax
	repne; scasb
	leaq   -1(%rdi), %rax
	ret
SYMEND(libc_strend)
DEFINE_PUBLIC_ALIAS(strend,libc_strend)

INTERN_ENTRY(libc_strlen)
	ENTER_CLD
	movq   $-1, %rcx
	xorq   %rax, %rax
	repne; scasb
	subq   %rcx, %rax /* result = 0 - ECX; */
	subq   $2, %rax   /* result = (0 - ECX) - 2; */
	ret
SYMEND(libc_strlen)
DEFINE_PUBLIC_ALIAS(strlen,libc_strlen)

INTERN_ENTRY(libc_strnlen)
	ENTER_CLD
	xorq   %rax, %rax
	testq  %rsi, %rsi
	jz     2f
	movq   %rsi, %rcx
	repne; scasb
	jne    1f
	incq   %rcx
1:	subq   %rcx, %rsi
	movq   %rsi, %rax /* result = max_chars - ECX; */
2:	ret
SYMEND(libc_strnlen)
DEFINE_PUBLIC_ALIAS(strnlen,libc_strnlen)

INTERN_ENTRY(libc_strnend)
	ENTER_CLD
	xorq   %rax, %rax
	testq  %rdx, %rdx
	jz     1f
	movq   %rdx, %rcx
	repne; scasb
	jne    1f
	decq   %rdi
1:	movq   %rdi, %rax
	ret
SYMEND(libc_strnend)
DEFINE_PUBLIC_ALIAS(strnend,libc_strnend)


#ifndef CONFIG_LIBC_LIMITED_API
INTERN_ENTRY(libc_w16end)
	ENTER_CLD
	movq   $-1, %rcx
	xorq   %rax, %rax
	repne; scasw
	leaq   -2(%rdi), %rax
	ret
SYMEND(libc_w16end)
DEFINE_PUBLIC_ALIAS(w16end,libc_w16end)

INTERN_ENTRY(libc_w16len)
	ENTER_CLD
	movq   $-1, %rcx
	xorq   %rax, %rax
	repne; scasw
	subq   %rcx, %rax /* result = 0 - ECX; */
	subq   $2, %rax   /* result = (0 - ECX) - 2; */
	ret
SYMEND(libc_w16len)
DEFINE_PUBLIC_ALIAS(w16len,libc_w16len)

INTERN_ENTRY(libc_w16nlen)
	ENTER_CLD
	xorq   %rax, %rax
	testq  %rsi, %rsi
	jz     2f
	movq   %rsi, %rcx
	repne; scasw
	jne    1f
	incq   %rcx
1:	subq   %rcx, %rsi
	movq   %rsi, %rax /* result = max_chars - ECX; */
2:	ret
SYMEND(libc_w16nlen)
DEFINE_PUBLIC_ALIAS(w16nlen,libc_w16nlen)

INTERN_ENTRY(libc_w16nend)
	ENTER_CLD
	xorq   %rax, %rax
	testq  %rdx, %rdx
	jz     1f
	movq   %rdx, %rcx
	repne; scasb
	jne    1f
	subq   $2, %rdi
1:	movq   %rdi, %rax
	ret
SYMEND(libc_w16nend)
DEFINE_PUBLIC_ALIAS(w16nend,libc_w16nend)

INTERN_ENTRY(libc_w32end)
	ENTER_CLD
	movq   $-1, %rcx
	xorq   %rax, %rax
	repne; scasl
	leaq   -4(%rdi), %rax
	ret
SYMEND(libc_w32end)
DEFINE_PUBLIC_ALIAS(w32end,libc_w32end)

INTERN_ENTRY(libc_w32len)
	ENTER_CLD
	movq   $-1, %rcx
	xorq   %rax, %rax
	repne; scasl
	subq   %rcx, %rax /* result = 0 - ECX; */
	subq   $2, %rax   /* result = (0 - ECX) - 2; */
	ret
SYMEND(libc_w32len)
DEFINE_PUBLIC_ALIAS(w32len,libc_w32len)

INTERN_ENTRY(libc_w32nlen)
	ENTER_CLD
	xorq   %rax, %rax
	testq  %rsi, %rsi
	jz     2f
	movq   %rsi, %rcx
	repne; scasl
	jne    1f
	incq   %rcx
1:	subq   %rcx, %rsi
	movq   %rsi, %rax /* result = max_chars - ECX; */
2:	ret
SYMEND(libc_w32nlen)
DEFINE_PUBLIC_ALIAS(w32nlen,libc_w32nlen)

INTERN_ENTRY(libc_w32nend)
	ENTER_CLD
	xorq   %rax, %rax
	testq  %rdx, %rdx
	jz     1f
	movq   %rdx, %rcx
	repne; scasl
	jne    1f
	subq   $4, %rdi
1:	movq   %rdi, %rax
	ret
SYMEND(libc_w32nend)
DEFINE_PUBLIC_ALIAS(w32nend,libc_w32nend)
#endif


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

.cfi_endproc

#endif








































#endif
