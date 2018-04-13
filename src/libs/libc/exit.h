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
#ifndef GUARD_LIBS_LIBC_EXIT_H
#define GUARD_LIBS_LIBC_EXIT_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     EXIT                                                                              */
/* ===================================================================================== */
INTDEF ATTR_NORETURN void LIBCCALL libc_exit(int status);
INTDEF ATTR_NORETURN void LIBCCALL libc_quick_exit(int status);
INTDEF ATTR_NORETURN void LIBCCALL _libc_exit(int status);
INTDEF void LIBCCALL libc_runexit(int status);
INTDEF void LIBCCALL libc_runquickexit(int status);
INTDEF void LIBCCALL libc_internal_onexit(void);
INTDEF int LIBCCALL libc_atexit(void (*LIBCCALL func)(void));
INTDEF int LIBCCALL libc_at_quick_exit(void (*LIBCCALL func)(void));
INTDEF int LIBCCALL libc_on_exit(void (LIBCCALL *func)(int status, void *arg), void *arg);
INTDEF ATTR_NORETURN void LIBCCALL libc_abort(void);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_EXIT_H */
