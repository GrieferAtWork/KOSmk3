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
#ifndef GUARD_HYBRID_BOUND_C
#define GUARD_HYBRID_BOUND_C 1
#define _KOS_SOURCE 2

#include "hybrid.h"

#include <hybrid/compiler.h>
#include <hybrid/asm.h>
#include <kos/types.h>
#include <kos/bound.h>
#include <stdbool.h>
#include <except.h>


DECL_BEGIN

PRIVATE ATTR_RETNONNULL struct exception_info *
LIBCCALL prepare_bfail(void) {
 struct exception_info *info = libc_error_info();
 libc_memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 info->e_error.e_code = E_INDEX_ERROR;
 info->e_error.e_flag = ERR_FRESUMEFUNC;
 return info;
}

INTERN bool LIBCCALL
libc_bound_chk_failb(struct boundb const *__restrict bnd, u8 index) {
 struct exception_info *info = prepare_bfail();
 info->e_error.e_index_error.b_boundmin = bnd->b_min;
 info->e_error.e_index_error.b_boundmax = bnd->b_max;
 info->e_error.e_index_error.b_index    = index;
 return libc_error_throw_current();
}
INTERN bool LIBCCALL
libc_bound_chk_failw(struct boundw const *__restrict bnd, u16 index) {
 struct exception_info *info = prepare_bfail();
 info->e_error.e_index_error.b_boundmin = bnd->b_min;
 info->e_error.e_index_error.b_boundmax = bnd->b_max;
 info->e_error.e_index_error.b_index    = index;
 return libc_error_throw_current();
}
INTERN bool LIBCCALL
libc_bound_chk_faill(struct boundl const *__restrict bnd, u32 index) {
 struct exception_info *info = prepare_bfail();
 info->e_error.e_index_error.b_boundmin = bnd->b_min;
 info->e_error.e_index_error.b_boundmax = bnd->b_max;
 info->e_error.e_index_error.b_index    = index;
 return libc_error_throw_current();
}
INTERN bool LIBCCALL
libc_bound_chk_failq(struct boundq const *__restrict bnd, u64 index) {
 struct exception_info *info = prepare_bfail();
 info->e_error.e_index_error.b_boundmin = bnd->b_min;
 info->e_error.e_index_error.b_boundmax = bnd->b_max;
 info->e_error.e_index_error.b_index    = index;
 return libc_error_throw_current();
}

INTERN void LIBCCALL
libc_bound_chkb(struct boundb const *__restrict bnd, u8 index) {
 do if (index >= bnd->b_min && index <= bnd->b_max) return;
 while (libc_bound_chk_failb(bnd,index));
}
INTERN void LIBCCALL
libc_bound_chkq(struct boundq const *__restrict bnd, u64 index) {
 do if (index >= bnd->b_min && index <= bnd->b_max) return;
 while (libc_bound_chk_failq(bnd,index));
}

#if defined(__i386__) || defined(__x86_64__)
#ifndef __KERNEL__
INTERN void LIBCCALL
libc_bound_chkw(struct boundw const *__restrict bnd, u16 index) {
 __asm__("boundw %w1, %0"
         :
         : "m" (*bnd)
         , "q" (index));
}
INTERN void LIBCCALL
libc_bound_chkl(struct boundl const *__restrict bnd, u32 index) {
 __asm__("boundl %1, %0"
         :
         : "m" (*bnd)
         , "r" (index));
}
#endif
#else
INTERN void LIBCCALL
libc_bound_chkw(struct boundw const *__restrict bnd, u16 index) {
 do if (index >= bnd->b_min && index <= bnd->b_max) return;
 while (libc_bound_chk_failw(bnd,index));
}
INTERN void LIBCCALL
libc_bound_chkl(struct boundl const *__restrict bnd, u32 index) {
 do if (index >= bnd->b_min && index <= bnd->b_max) return;
 while (libc_bound_chk_faill(bnd,index));
}
#endif

#if !defined(__KERNEL__) || \
   (!defined(__i386__) && !defined(__x86_64__))
EXPORT(__bound_chkw,libc_bound_chkw);
EXPORT(__bound_chkl,libc_bound_chkl);
#endif
EXPORT(__bound_chkb,libc_bound_chkb);
EXPORT(__bound_chkq,libc_bound_chkq);
EXPORT(__bound_chk_failb,libc_bound_chk_failb);
EXPORT(__bound_chk_failw,libc_bound_chk_failw);
EXPORT(__bound_chk_faill,libc_bound_chk_faill);
EXPORT(__bound_chk_failq,libc_bound_chk_failq);

DECL_END

#endif /* !GUARD_HYBRID_BOUND_C */
