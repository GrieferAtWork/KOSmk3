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
#ifndef GUARD_LIBS_LIBC_RTL_H
#define GUARD_LIBS_LIBC_RTL_H 1

#include "libc.h"
#include <kos/types.h>
#include <stdarg.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     RTL                                                                               */
/* ===================================================================================== */
INTDEF ATTR_NORETURN void __stack_chk_fail_local(void);
INTDEF ATTR_NORETURN void LIBCCALL libc_core_assertion_failure(char const *expr, DEBUGINFO, char const *format, va_list args);
INTDEF ATTR_NORETURN ATTR_COLD void ATTR_CDECL libc_afailf(char const *expr, DEBUGINFO, char const *format, ...);
INTDEF ATTR_NORETURN ATTR_COLD void LIBCCALL libc_afail(char const *expr, DEBUGINFO);
INTDEF ssize_t LIBCCALL libc_syslog_printer(char const *__restrict data, size_t datalen, void *closure);
INTDEF void LIBCCALL libc_vsyslog(int level, char const *format, va_list args);
INTDEF void ATTR_CDECL libc_syslog(int level, char const *format, ...);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_RTL_H */
