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
#include <syscall.h>
#include <kos/ushare.h>
#include <asm/universal.h>

#define MALLOC_DEFINE_INTERN_CALLBACK(x) \
	DEFINE_INTERN(libc_##x##_f); \
	DEFINE_INTERN(libc_##x##_d);

LIBC_ENUMERATE_MALLOC_FUNCTIONS(MALLOC_DEFINE_INTERN_CALLBACK)
#undef MALLOC_DEFINE_INTERN_CALLBACK


/* NOTE: .data, so that `libc_set_debug_malloc()' can override pointers. */
.section .data
.cfi_startproc
INTERN_OBJECT(libc_malloc_jmptab)
#define MALLOC_DEFINE_JMPTAB_CALLBACK(x) \
	INTERN_ENTRY(libc_##x); \
		jmp libc_##x##_f; \
	SYMEND(libc_##x);
LIBC_ENUMERATE_MALLOC_FUNCTIONS(MALLOC_DEFINE_JMPTAB_CALLBACK)
#undef MALLOC_DEFINE_JMPTAB_CALLBACK
SYMEND(libc_malloc_jmptab)
.cfi_endproc

INTERN_OBJECT(libc_malloc_jmptab_d)
	/* Pattern for the debug-malloc jump table.
	 * When `libc_set_debug_malloc()' is called, it
	 * will simply copy this text into the table above. */
#define MALLOC_DEFINE_DEBUG_JMPTAB_CALLBACK(x) \
	jmp (libc_malloc_jmptab_d - libc_malloc_jmptab) + libc_##x##_d;
LIBC_ENUMERATE_MALLOC_FUNCTIONS(MALLOC_DEFINE_DEBUG_JMPTAB_CALLBACK)
#undef MALLOC_DEFINE_DEBUG_JMPTAB_CALLBACK
SYMEND(libc_malloc_jmptab_d)



/* Export malloc functions. */
#undef LIBC_ENUMERATE_MALLOC_EXT
#define LIBC_ENUMERATE_MALLOC_EXT(x) /* nothing */
#define MALLOC_EXPORT_SYMBOLS_CALLBACK(x) EXPORT(x,libc_##x)
LIBC_ENUMERATE_MALLOC_FUNCTIONS(MALLOC_EXPORT_SYMBOLS_CALLBACK)
#undef MALLOC_EXPORT_SYMBOLS_CALLBACK
#undef LIBC_ENUMERATE_MALLOC_EXT
#define LIBC_ENUMERATE_MALLOC_EXT(x) x

/* Export wide-character duplication functions. */
EXPORT(__SYMw16(_wcsdup),libc_w16dup);
EXPORT(__SYMw32(wcsdup),libc_w32dup);
EXPORT(__SYMw16(wcsndup),libc_w16ndup);
EXPORT(__SYMw32(wcsndup),libc_w32ndup);
EXPORT(__SYMw16(Xwcsdup),libc_Xw16dup);
EXPORT(__SYMw32(Xwcsdup),libc_Xw32dup);
EXPORT(__SYMw16(Xwcsndup),libc_Xw16ndup);
EXPORT(__SYMw32(Xwcsndup),libc_Xw32ndup);

/* Aliases... */
EXPORT(cfree,libc_free)
EXPORT(aligned_alloc,libc_memalign)

/* GLibc Aliases... */
EXPORT_STRONG(__strdup,libc_strdup)
EXPORT_STRONG(__strndup,libc_strndup)
EXPORT_STRONG(__libc_calloc,libc_calloc)
EXPORT_STRONG(__libc_free,libc_free)
EXPORT_STRONG(__libc_malloc,libc_malloc)
EXPORT_STRONG(__libc_mallopt,libc_mallopt)
EXPORT_STRONG(__libc_memalign,libc_memalign)
EXPORT_STRONG(__libc_pvalloc,libc_pvalloc)
EXPORT_STRONG(__libc_valloc,libc_valloc)


/* DOS Aliases... */
EXPORT(__DSYM(_strdup),libc_strdup)
EXPORT(__DSYM(_aligned_free),libc_free)
EXPORT(__DSYM(_recalloc),libc_recalloc)
EXPORT(__DSYM(_malloc_dbg),libc_malloc)
EXPORT(__DSYM(_calloc_dbg),libc_calloc)
EXPORT(__DSYM(_realloc_dbg),libc_realloc)
EXPORT(__DSYM(_recalloc_dbg),libc_recalloc)
EXPORT(__DSYM(_expand),libc_realloc_in_place)
EXPORT(__DSYM(_expand_dbg),libc_realloc_in_place)
EXPORT(__DSYM(_free_dbg),libc_free)
EXPORT(__DSYM(_aligned_free_dbg),libc_free)
EXPORT(__DSYM(_msize_dbg),libc_malloc_usable_size)
EXPORT(__DSYM(_aligned_msize_dbg),libc_malloc_usable_size)
EXPORT(__DSYM(_strdup_dbg),libc_strdup)
EXPORT(__DSYM(_wcsdup_dbg),libc_w16dup)


/* Deprecated (now stub) debug variants of KOS Mk2 allocator functions. */
EXPORT(_malloc_d,libc_malloc)
EXPORT(_free_d,libc_free)
EXPORT(_calloc_d,libc_calloc)
EXPORT(_realloc_d,libc_realloc)
EXPORT(_realloc_in_place_d,libc_realloc_in_place)
EXPORT(_memalign_d,libc_memalign)
EXPORT(_valloc_d,libc_valloc)
EXPORT(_pvalloc_d,libc_pvalloc)
EXPORT(_posix_memalign_d,libc_posix_memalign)
EXPORT(_mallopt_d,libc_mallopt)
EXPORT(_malloc_trim_d,libc_malloc_trim)
EXPORT(_malloc_usable_size_d,libc_malloc_usable_size)
EXPORT(_memdup_d,libc_memdup)
EXPORT(_strdup_d,libc_strdup)
EXPORT(_strndup_d,libc_strndup)
EXPORT(_memcdup_d,libc_memcdup)
EXPORT(_aligned_alloc_d,libc_memalign)
EXPORT(_cfree_d,libc_free)

/* The deprecated (now stub) MALL API from KOS Mk2 */
.section .text
INTERN_ENTRY(libc_x86_return_zero)
	xorp   %pax, %pax
INTERN_ENTRY(libc_x86_return)
	ret
SYMEND(libc_x86_return)
SYMEND(libc_x86_return_zero)
INTERN_ENTRY(libc_x86_return_arg0)
#ifdef __x86_64__
	movq   %rdi, %rax
#else
	movl   4(%esp), %eax
#endif
	ret
SYMEND(libc_x86_return_arg0)

EXPORT(_mall_getattrib,libc_x86_return_zero)
EXPORT(_mall_traceback,libc_x86_return_zero)
EXPORT(_mall_enum,libc_x86_return_zero)
EXPORT(_mall_getattrib_d,libc_x86_return_zero)
EXPORT(_mall_traceback_d,libc_x86_return_zero)
EXPORT(_mall_enum_d,libc_x86_return_zero)
EXPORT(_mall_printleaks,libc_x86_return)
EXPORT(_mall_validate,libc_x86_return)
EXPORT(_mall_printleaks_d,libc_x86_return)
EXPORT(_mall_validate_d,libc_x86_return)
EXPORT(_mall_untrack,libc_x86_return_arg0)
EXPORT(_mall_nofree,libc_x86_return_arg0)
EXPORT(_mall_untrack_d,libc_x86_return_arg0)
EXPORT(_mall_nofree_d,libc_x86_return_arg0)


