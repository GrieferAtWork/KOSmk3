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
#ifndef GUARD_LIBS_LIBC_ENVIRON_H
#define GUARD_LIBS_LIBC_ENVIRON_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

#define ENVIRON_USEW16 0x01 /* Keep `w16environ' up to date */
#define ENVIRON_USEW32 0x02 /* Keep `w32environ' up to date */

/* ===================================================================================== */
/*     ENVIRON                                                                           */
/* ===================================================================================== */
DATDEF char **environ;
INTDEF char16_t **w16environ;
INTDEF char16_t **w32environ;
INTDEF char ***LIBCCALL libc_p_environ(void);
INTDEF char *LIBCCALL libc_getenv(char const *name);
INTDEF char16_t **LIBCCALL libc_get_w16environ(void);
INTDEF char32_t **LIBCCALL libc_get_w32environ(void);
INTDEF ATTR_RETNONNULL char16_t **LIBCCALL libc_Xget_w16environ(void);
INTDEF ATTR_RETNONNULL char32_t **LIBCCALL libc_Xget_w32environ(void);
INTDEF int LIBCCALL libc_clearenv(void);
INTDEF int LIBCCALL libc_setenv(char const *name, char const *value, int replace);
INTDEF int LIBCCALL libc_unsetenv(char const *name);
INTDEF int LIBCCALL libc_putenv(char *string);
INTDEF char *LIBCCALL libc_dos_getenv(char const *name);
INTDEF int LIBCCALL libc_dos_setenv(char const *name, char const *value, int replace);
INTDEF int LIBCCALL libc_dos_putenv(char *string);
INTDEF derrno_t LIBCCALL libd_putenv_s(char const *name, char const *value);
INTDEF derrno_t LIBCCALL libd_dos_putenv_s(char const *name, char const *value);


INTDEF int LIBCCALL libc_env_acquire(void);
INTDEF void LIBCCALL libc_env_release(void);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_ENVIRON_H */
