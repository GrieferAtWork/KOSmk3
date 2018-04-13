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
#ifndef GUARD_LIBS_LIBC_GROUP_H
#define GUARD_LIBS_LIBC_GROUP_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN


/* ===================================================================================== */
/*     GROUP                                                                             */
/* ===================================================================================== */
struct passwd;
struct group;
INTDEF struct passwd *LIBCCALL libc_getpwuid(uid_t uid);
INTDEF struct passwd *LIBCCALL libc_getpwnam(char const *name);
INTDEF void LIBCCALL libc_setpwent(void);
INTDEF void LIBCCALL libc_endpwent(void);
INTDEF struct passwd *LIBCCALL libc_getpwent(void);
INTDEF struct passwd *LIBCCALL libc_fgetpwent(FILE *stream);
INTDEF int LIBCCALL libc_putpwent(struct passwd const *__restrict p, FILE *__restrict f);
INTDEF int LIBCCALL libc_getpwuid_r(uid_t uid, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result);
INTDEF int LIBCCALL libc_getpwnam_r(char const *__restrict name, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result);
INTDEF int LIBCCALL libc_getpwent_r(struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result);
INTDEF int LIBCCALL libc_fgetpwent_r(FILE *__restrict stream, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result);
INTDEF int LIBCCALL libc_getpw(uid_t uid, char *buffer);
INTDEF struct group *LIBCCALL libc_getgrgid(gid_t gid);
INTDEF struct group *LIBCCALL libc_getgrnam(char const *name);
INTDEF void LIBCCALL libc_setgrent(void);
INTDEF void LIBCCALL libc_endgrent(void);
INTDEF struct group *LIBCCALL libc_getgrent(void);
INTDEF int LIBCCALL libc_putgrent(struct group const *__restrict p, FILE *__restrict f);
INTDEF int LIBCCALL libc_getgrgid_r(gid_t gid, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result);
INTDEF int LIBCCALL libc_getgrnam_r(char const *__restrict name, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result);
INTDEF int LIBCCALL libc_getgrent_r(struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result);
INTDEF int LIBCCALL libc_fgetgrent_r(FILE *__restrict stream, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result);
INTDEF struct group *LIBCCALL libc_fgetgrent(FILE *stream);
INTDEF int LIBCCALL libc_setgroups(size_t n, const gid_t *groups);
INTDEF int LIBCCALL libc_getgrouplist(char const *user, gid_t group, gid_t *groups, int *ngroups);
INTDEF int LIBCCALL libc_initgroups(char const *user, gid_t group);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_GROUP_H */
