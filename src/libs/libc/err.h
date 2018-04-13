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
#ifndef GUARD_LIBS_LIBC_ERR_H
#define GUARD_LIBS_LIBC_ERR_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

INTERN ATTR_NORETURN void LIBCCALL
libc_assert_perror_fail(int errnum, char const *file,
                        unsigned int line, char const *function);
INTERN void LIBCCALL libc_perror(char const *__restrict message);
INTERN void LIBCCALL libc_w16perror(char16_t const *__restrict message);
INTERN void LIBCCALL libc_w32perror(char32_t const *__restrict message);

INTDEF void LIBCCALL libc_fortify_fail(char const *__restrict message);
INTDEF char *LIBCCALL libc_program_invocation_name(void);
INTDEF char *LIBCCALL libc_program_invocation_short_name(void);
INTDEF void LIBCCALL libc_vwarn(char const *format, va_list args);
INTDEF void LIBCCALL libd_vwarn(char const *format, va_list args);
INTDEF void LIBCCALL libc_vwarnx(char const *format, va_list args);
INTDEF void LIBCCALL libd_vwarnx(char const *format, va_list args);
INTDEF ATTR_NORETURN void LIBCCALL libc_verr(int status, char const *format, va_list args);
INTDEF ATTR_NORETURN void LIBCCALL libd_verr(int status, char const *format, va_list args);
INTDEF ATTR_NORETURN void LIBCCALL libc_verrx(int status, char const *format, va_list args);
INTDEF ATTR_NORETURN void LIBCCALL libd_verrx(int status, char const *format, va_list args);
INTDEF void LIBCCALL libc_warn(char const *format, ...);
INTDEF void LIBCCALL libd_warn(char const *format, ...);
INTDEF void LIBCCALL libc_warnx(char const *format, ...);
INTDEF void LIBCCALL libd_warnx(char const *format, ...);
INTDEF ATTR_NORETURN void LIBCCALL libc_err(int status, char const *format, ...);
INTDEF ATTR_NORETURN void LIBCCALL libd_err(int status, char const *format, ...);
INTDEF ATTR_NORETURN void LIBCCALL libc_errx(int status, char const *format, ...);
INTDEF ATTR_NORETURN void LIBCCALL libd_errx(int status, char const *format, ...);
INTDEF void LIBCCALL libc_error(int status, errno_t errnum, char const *format, ...);
INTDEF void LIBCCALL libd_error(int status, errno_t errnum, char const *format, ...);
INTDEF void LIBCCALL libc_error_at_line(int status, errno_t errnum, char const *fname, unsigned int lineno, char const *format, ...);
INTDEF void LIBCCALL libd_error_at_line(int status, errno_t errnum, char const *fname, unsigned int lineno, char const *format, ...);


DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_ERR_H */
