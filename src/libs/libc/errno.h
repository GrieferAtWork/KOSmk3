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
#ifndef GUARD_LIBS_LIBC_ERRNO_H
#define GUARD_LIBS_LIBC_ERRNO_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

#define KOS_STRUNCATE  (__EBASEMAX+1)

/* ===================================================================================== */
/*     ERRNO                                                                             */
/* ===================================================================================== */
INTDEF ATTR_RETNONNULL errno_t *LIBCCALL libc_errno(void);
INTDEF ATTR_RETNONNULL derrno_t *LIBCCALL libc_dos_errno(void);
INTDEF ATTR_RETNONNULL u32 *LIBCCALL libc_nt_errno(void);
INTDEF errno_t LIBCCALL libc_geterrno(void);
INTDEF derrno_t LIBCCALL libc_dos_geterrno(void);
INTDEF u32 LIBCCALL libc_nt_geterrno(void);
INTDEF void LIBCCALL libc_seterrno(errno_t value);
INTDEF void LIBCCALL libc_dos_seterrno(derrno_t value);
INTDEF void LIBCCALL libc_nt_seterrno(u32 value);
INTDEF derrno_t LIBCCALL libc_dos_geterrno2(derrno_t *value);
INTDEF derrno_t LIBCCALL libc_dos_seterrno2(derrno_t value);
INTDEF derrno_t LIBCCALL libc_nt_geterrno2(u32 *value);
INTDEF derrno_t LIBCCALL libc_nt_seterrno2(u32 value);
/* Convert between error encodings */
INTDEF errno_t LIBCCALL libc_errno_dos2kos(derrno_t value);
INTDEF errno_t LIBCCALL libc_errno_nt2kos(u32 value);
INTDEF derrno_t LIBCCALL libc_errno_nt2dos(u32 value);
INTDEF derrno_t LIBCCALL libc_errno_kos2dos(errno_t value);
INTDEF u32 LIBCCALL libc_errno_dos2nt(derrno_t value);
INTDEF u32 LIBCCALL libc_errno_kos2nt(errno_t value);
INTDEF char **LIBCCALL libc_sys_errlist(void);
INTDEF int *LIBCCALL libc_sys_nerr(void);
#define FORWARD_SYSTEM_ERROR(x) \
    XBLOCK({ __typeof__(x) _res = (x); \
             XRETURN (syscall_slong_t)_res < 0 ? \
                     (libc_seterrno((errno_t)-(syscall_slong_t)_res),(__typeof__(x))-1) : 0; })
#define FORWARD_SYSTEM_VALUE(x) \
    XBLOCK({ __typeof__(x) _res = (x); \
             XRETURN (syscall_slong_t)_res < 0 ? \
                     (libc_seterrno((errno_t)-(syscall_slong_t)_res),(__typeof__(x))-1) : \
                      _res; })
#define SET_SYSTEM_ERROR(x) \
    XBLOCK({ libc_seterrno((errno_t)-(syscall_slong_t)(x)); \
             XRETURN (__typeof__(x))-1; })

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_ERRNO_H */
