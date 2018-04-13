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
#ifndef GUARD_LIBS_LIBC_USHARE_H
#define GUARD_LIBS_LIBC_USHARE_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     USHARE                                                                            */
/* ===================================================================================== */
struct utsname;
INTDEF char const *LIBCCALL libc_strerror_s(errno_t errnum);
INTDEF char const *LIBCCALL libd_strerror_s(derrno_t errnum);
INTDEF char const *LIBCCALL libc_strerrorname_s(errno_t errnum);
INTDEF char const *LIBCCALL libd_strerrorname_s(derrno_t errnum);
INTDEF char const *LIBCCALL libc_strsignal_s(int signo);
INTDEF char const *LIBCCALL libd_strsignal_s(int signo);
INTDEF char const *LIBCCALL libc_strsignaltext_s(int signo);
INTDEF char const *LIBCCALL libd_strsignaltext_s(int signo);
INTDEF int LIBCCALL libc_xpg_strerror_r(errno_t errnum, char *buf, size_t buflen);
INTDEF int LIBCCALL libd_xpg_strerror_r(derrno_t errnum, char *buf, size_t buflen);
INTDEF char *LIBCCALL libc_strerror_r(errno_t errnum, char *buf, size_t buflen);
INTDEF char *LIBCCALL libd_strerror_r(derrno_t errnum, char *buf, size_t buflen);
INTDEF char *LIBCCALL libc_strerror(errno_t errnum);
INTDEF char *LIBCCALL libd_strerror(derrno_t errnum);
INTDEF char *LIBCCALL libc_strerror_l(errno_t errnum, locale_t locale);
INTDEF char *LIBCCALL libd_strerror_l(derrno_t errnum, locale_t locale);
INTDEF char *LIBCCALL libc_strsignal(int signo);
INTDEF char *LIBCCALL libd_strsignal(int signo);
INTDEF int LIBCCALL libc_uname(struct utsname *name);
INTDEF int LIBCCALL libc_gethostname(char *name, size_t buflen);
INTDEF int LIBCCALL libc_sethostname(char const *name, size_t len);
INTDEF int LIBCCALL libc_getdomainname(char *name, size_t buflen);
INTDEF int LIBCCALL libc_setdomainname(char const *name, size_t len);
INTDEF void LIBCCALL libc_Xuname(struct utsname *name);
INTDEF void LIBCCALL libc_Xgethostname(char *name, size_t buflen);
INTDEF void LIBCCALL libc_Xsethostname(char const *name, size_t len);
INTDEF void LIBCCALL libc_Xgetdomainname(char *name, size_t buflen);
INTDEF void LIBCCALL libc_Xsetdomainname(char const *name, size_t len);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_USHARE_H */
