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
#include <hybrid/host.h>
#include <hybrid/limits.h>
#include <asm/cfi.h>
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
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movl   16(%esp), %esi /* src */
	movl   20(%esp), %ecx /* num_bytes */
	movl   %edi, %eax
	movl   %ecx, %edx
	shrl   $2, %ecx
	rep    movsl
	jnc    1f /* if (!(num_bytes & 2)) goto 1f; */
	movsw
1:	testl  $1, %edx
	jz     1f /* if (!(num_bytes & 1)) goto 1f; */
	movsb
1:	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_memcpy)
DEFINE_PUBLIC_ALIAS(memcpy,libc_memcpy)

INTERN_ENTRY(libc_memcpyw)
	ENTER_CLD
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movl   16(%esp), %esi /* src */
	movl   20(%esp), %ecx /* num_words */
	movl   %edi, %eax
	shrl   $1, %ecx
	rep    movsl
	jnc    1f /* if (!(num_words & 1)) goto 1f; */
	movsw
1:	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_memcpyw)
DEFINE_PUBLIC_ALIAS(memcpyw,libc_memcpyw)

INTERN_ENTRY(libc_memcpyl)
	ENTER_CLD
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movl   16(%esp), %esi /* src */
	movl   20(%esp), %ecx /* num_dwords */
	movl   %edi, %eax
	rep    movsl
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_memcpyl)
DEFINE_PUBLIC_ALIAS(memcpyl,libc_memcpyl)

#ifndef CONFIG_NO_64BIT_STRING
INTERN_ENTRY(libc_memcpyq)
	ENTER_CLD
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movl   16(%esp), %esi /* src */
	movl   20(%esp), %ecx /* num_dwords */
	shll   $1, %ecx
	movl   %edi, %eax
	rep    movsl
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_memcpyq)
DEFINE_PUBLIC_ALIAS(memcpyq,libc_memcpyq)
#endif /* !CONFIG_NO_64BIT_STRING */
.cfi_endproc


.cfi_startproc
INTERN_ENTRY(libc_mempcpy)
	ENTER_CLD
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movl   16(%esp), %esi /* src */
	movl   20(%esp), %ecx /* num_bytes */
	movl   %ecx, %edx
	shrl   $2, %ecx
	rep    movsl
	jnc    1f /* if (!(num_bytes & 2)) goto 1f; */
	movsw
1:	testl  $1, %edx
	jz     1f /* if (!(num_bytes & 1)) goto 1f; */
	movsb
1:	movl   %edi, %eax
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_mempcpy)
DEFINE_PUBLIC_ALIAS(mempcpy,libc_mempcpy)

INTERN_ENTRY(libc_mempcpyw)
	ENTER_CLD
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movl   16(%esp), %esi /* src */
	movl   20(%esp), %ecx /* num_words */
	shrl   $1, %ecx
	rep    movsl
	jnc    1f /* if (!(num_words & 1)) goto 1f; */
	movsw
1:	movl   %edi, %eax
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_mempcpyw)
DEFINE_PUBLIC_ALIAS(mempcpyw,libc_mempcpyw)

INTERN_ENTRY(libc_mempcpyl)
	ENTER_CLD
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movl   16(%esp), %esi /* src */
	movl   20(%esp), %ecx /* num_dwords */
	rep    movsl
	movl   %edi, %eax
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_mempcpyl)
DEFINE_PUBLIC_ALIAS(mempcpyl,libc_mempcpyl)

#ifndef CONFIG_NO_64BIT_STRING
INTERN_ENTRY(libc_mempcpyq)
	ENTER_CLD
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movl   16(%esp), %esi /* src */
	movl   20(%esp), %ecx /* num_dwords */
	shll   $1, %ecx
	rep    movsl
	movl   %edi, %eax
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_mempcpyq)
DEFINE_PUBLIC_ALIAS(mempcpyq,libc_mempcpyq)
#endif /* !CONFIG_NO_64BIT_STRING */
.cfi_endproc

.cfi_startproc
INTERN_ENTRY(libc_memset)
	ENTER_CLD
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movzbl 16(%esp), %eax /* byte */
	movb   %al, %ah
	movw   %ax, %cx
	shll   $16, %eax
	movw   %cx, %ax
	movl   20(%esp), %ecx /* num_bytes */
	pushl_cfi %edi
	movl   %ecx, %edx
	shrl   $2, %ecx
	rep    stosl
	jnc    1f /* if (!(num_bytes & 2)) goto 1f; */
	stosw
1:	testl  $1, %edx
	jz     1f /* if (!(num_bytes & 1)) goto 1f; */
	stosb
1:	popl_cfi %eax
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_memset)
DEFINE_PUBLIC_ALIAS(memset,libc_memset)

INTERN_ENTRY(libc_memsetw)
	ENTER_CLD
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movzwl 16(%esp), %eax /* word */
	movw   %ax, %cx
	shll   $16, %eax
	movw   %cx, %ax
	movl   20(%esp), %ecx /* num_words */
	movl   %edi, %edx
	shrl   $1, %ecx
	rep    stosl
	jnc    1f /* if (!(num_words & 1)) goto 1f; */
	stosw
1:	movl   %edx, %eax
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_memsetw)
DEFINE_PUBLIC_ALIAS(memsetw,libc_memsetw)

INTERN_ENTRY(libc_memsetl)
	ENTER_CLD
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movl   16(%esp), %eax /* dword */
	movl   20(%esp), %ecx /* num_dwords */
	movl   %edi, %edx
	rep    stosl
	movl   %edx, %eax
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_memsetl)
DEFINE_PUBLIC_ALIAS(memsetl,libc_memsetl)
.cfi_endproc

.cfi_startproc
INTERN_ENTRY(libc_mempset)
	ENTER_CLD
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movzbl 16(%esp), %eax /* byte */
	movb   %al, %ah
	movw   %ax, %cx
	shll   $16, %eax
	movw   %cx, %ax
	movl   20(%esp), %ecx /* num_bytes */
	movl   %ecx, %edx
	shrl   $2, %ecx
	rep    stosl
	jnc    1f /* if (!(num_bytes & 2)) goto 1f; */
	stosw
1:	testl  $1, %edx
	jz     1f /* if (!(num_bytes & 1)) goto 1f; */
	stosb
1:	movl   %edi, %eax
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_mempset)
DEFINE_PUBLIC_ALIAS(mempset,libc_mempset)

INTERN_ENTRY(libc_mempsetw)
	ENTER_CLD
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movzwl 16(%esp), %eax /* word */
	movw   %ax, %cx
	shll   $16, %eax
	movw   %cx, %ax
	movl   20(%esp), %ecx /* num_words */
	shrl   $1, %ecx
	rep    stosl
	jnc    1f /* if (!(num_words & 1)) goto 1f; */
	stosw
1:	movl   %edi, %eax
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_mempsetw)
DEFINE_PUBLIC_ALIAS(mempsetw,libc_mempsetw)

INTERN_ENTRY(libc_mempsetl)
	ENTER_CLD
	pushl_cfi_r %edi
	pushl_cfi_r %esi
	movl   12(%esp), %edi /* dst */
	movl   16(%esp), %eax /* dword */
	movl   20(%esp), %ecx /* num_dwords */
	rep    stosl
	movl   %edi, %eax
	popl_cfi_r  %esi
	popl_cfi_r  %edi
	RET(12)
SYMEND(libc_mempsetl)
DEFINE_PUBLIC_ALIAS(mempsetl,libc_mempsetl)
.cfi_endproc




.section .text.crt.string
.cfi_startproc
INTERN_ENTRY(libc_strend)
	ENTER_CLD
	pushl_cfi_r %edi
	movl   8(%esp), %edi
	movl   $-1, %ecx
	xorl   %eax, %eax
	repne; scasb
	leal   -1(%edi), %eax
	popl_cfi_r %edi
	RET(4)
SYMEND(libc_strend)
DEFINE_PUBLIC_ALIAS(strend,libc_strend)

INTERN_ENTRY(libc_strlen)
	ENTER_CLD
	pushl_cfi_r %edi
	movl   8(%esp), %edi
	movl   $-1, %ecx
	xorl   %eax, %eax
	repne; scasb
	subl   %ecx, %eax /* result = 0 - ECX; */
	subl   $2, %eax   /* result = (0 - ECX) - 2; */
	popl_cfi_r %edi
	RET(4)
SYMEND(libc_strlen)
DEFINE_PUBLIC_ALIAS(strlen,libc_strlen)

INTERN_ENTRY(libc_strnlen)
	ENTER_CLD
	xorl   %eax, %eax
	movl   8(%esp), %edx
	testl  %edx, %edx
	jz     2f
	pushl_cfi_r %edi
	movl   8(%esp), %edi
	movl   %edx, %ecx
	repne; scasb
	jne    1f
	incl   %ecx
1:	subl   %ecx, %edx
	movl   %edx, %eax /* result = max_chars - ECX; */
	popl_cfi_r %edi
2:	RET(8)
SYMEND(libc_strnlen)
DEFINE_PUBLIC_ALIAS(strnlen,libc_strnlen)

INTERN_ENTRY(libc_strnend)
	ENTER_CLD
	pushl_cfi_r %edi
	xorl   %eax, %eax
	movl   8(%esp), %edi
	movl   12(%esp), %ecx
	testl  %ecx, %ecx
	jz     1f
	repne; scasb
	jne    1f
	decl   %edi
1:	movl   %edi, %eax
	popl_cfi_r %edi
	RET(8)
SYMEND(libc_strnend)
DEFINE_PUBLIC_ALIAS(strnend,libc_strnend)




#ifndef CONFIG_LIBC_LIMITED_API
INTERN_ENTRY(libc_w16end)
	ENTER_CLD
	pushl_cfi_r %edi
	movl   8(%esp), %edi
	movl   $-1, %ecx
	xorl   %eax, %eax
	repne; scasw
	leal   -2(%edi), %eax
	popl_cfi_r %edi
	RET(4)
SYMEND(libc_w16end)
DEFINE_PUBLIC_ALIAS(__SYMw16(wscend),libc_w16end)

INTERN_ENTRY(libc_w16len)
	ENTER_CLD
	pushl_cfi_r %edi
	movl   8(%esp), %edi
	movl   $-1, %ecx
	xorl   %eax, %eax
	repne; scasw
	subl   %ecx, %eax /* result = 0 - ECX; */
	subl   $2, %eax   /* result = (0 - ECX) - 2; */
	popl_cfi_r %edi
	RET(4)
SYMEND(libc_w16len)
DEFINE_PUBLIC_ALIAS(__SYMw16(wcslen),libc_w16len)

INTERN_ENTRY(libc_w16nlen)
	ENTER_CLD
	xorl   %eax, %eax
	movl   8(%esp), %edx
	testl  %edx, %edx
	jz     2f
	pushl_cfi_r %edi
	movl   8(%esp), %edi
	movl   %edx, %ecx
	repne; scasw
	jne    1f
	incl   %ecx
1:	subl   %ecx, %edx
	movl   %edx, %eax /* result = max_chars - ECX; */
	popl_cfi_r %edi
2:	RET(8)
SYMEND(libc_w16nlen)
DEFINE_PUBLIC_ALIAS(__SYMw16(wcsnlen),libc_w16nlen)

INTERN_ENTRY(libc_w16nend)
	ENTER_CLD
	pushl_cfi_r %edi
	xorl   %eax, %eax
	movl   8(%esp), %edi
	movl   12(%esp), %ecx
	testl  %ecx, %ecx
	jz     1f
	repne; scasw
	jne    1f
	subl   $2, %edi
1:	movl   %edi, %eax
	popl_cfi_r %edi
	RET(8)
SYMEND(libc_w16nend)
DEFINE_PUBLIC_ALIAS(__SYMw16(wcsnend),libc_w16nend)

INTERN_ENTRY(libc_w32end)
	ENTER_CLD
	pushl_cfi_r %edi
	movl   8(%esp), %edi
	movl   $-1, %ecx
	xorl   %eax, %eax
	repne; scasl
	leal   -4(%edi), %eax
	popl_cfi_r %edi
	RET(4)
SYMEND(libc_w32end)
DEFINE_PUBLIC_ALIAS(__SYMw32(wscend),libc_w32end)

INTERN_ENTRY(libc_w32len)
	ENTER_CLD
	pushl_cfi_r %edi
	movl   8(%esp), %edi
	movl   $-1, %ecx
	xorl   %eax, %eax
	repne; scasl
	subl   %ecx, %eax /* result = 0 - ECX; */
	subl   $2, %eax   /* result = (0 - ECX) - 2; */
	popl_cfi_r %edi
	RET(4)
SYMEND(libc_w32len)
DEFINE_PUBLIC_ALIAS(__SYMw32(wcslen),libc_w32len)

INTERN_ENTRY(libc_w32nlen)
	ENTER_CLD
	xorl   %eax, %eax
	movl   8(%esp), %edx
	testl  %edx, %edx
	jz     2f
	pushl_cfi_r %edi
	movl   8(%esp), %edi
	movl   %edx, %ecx
	repne; scasl
	jne    1f
	incl   %ecx
1:	subl   %ecx, %edx
	movl   %edx, %eax /* result = max_chars - ECX; */
	popl_cfi_r %edi
2:	RET(8)
SYMEND(libc_w32nlen)
DEFINE_PUBLIC_ALIAS(__SYMw32(wcsnlen),libc_w32nlen)

INTERN_ENTRY(libc_w32nend)
	ENTER_CLD
	pushl_cfi_r %edi
	xorl   %eax, %eax
	movl   8(%esp), %edi
	movl   12(%esp), %ecx
	testl  %ecx, %ecx
	jz     1f
	repne; scasl
	jne    1f
	subl   $4, %edi
1:	movl   %edi, %eax
	popl_cfi_r %edi
	RET(8)
SYMEND(libc_w32nend)
DEFINE_PUBLIC_ALIAS(__SYMw32(wcsnend),libc_w32nend)
#endif /* !CONFIG_LIBC_LIMITED_API */

.cfi_endproc

#endif


#if 1

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








































