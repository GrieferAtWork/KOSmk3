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
#ifndef GUARD_LIBS_LIBC_GROUP_C
#define GUARD_LIBS_LIBC_GROUP_C 1

#include "libc.h"
#include "group.h"
#include "errno.h"
#include "stdio/file.h"

#include <errno.h>
#include <pwd.h>
#include <grp.h>

DECL_BEGIN

INTERN struct passwd *LIBCCALL libc_getpwuid(uid_t uid) { libc_seterrno(ENOSYS); return NULL; }
INTERN struct passwd *LIBCCALL libc_getpwnam(char const *name) { libc_seterrno(ENOSYS); return NULL; }
INTERN void LIBCCALL libc_setpwent(void) { libc_seterrno(ENOSYS); }
INTERN void LIBCCALL libc_endpwent(void) { libc_seterrno(ENOSYS); }
INTERN struct passwd *LIBCCALL libc_getpwent(void) { libc_seterrno(ENOSYS); return NULL; }
INTERN struct passwd *LIBCCALL libc_fgetpwent(FILE *stream) { libc_seterrno(ENOSYS); return NULL; }
INTERN int LIBCCALL libc_putpwent(struct passwd const *__restrict p, FILE *__restrict f) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getpwuid_r(uid_t uid, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getpwnam_r(char const *__restrict name, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getpwent_r(struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_fgetpwent_r(FILE *__restrict stream, struct passwd *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct passwd **__restrict result) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getpw(uid_t uid, char *buffer) { libc_seterrno(ENOSYS); return -1; }
INTERN struct group *LIBCCALL libc_getgrgid(gid_t gid) { libc_seterrno(ENOSYS); return NULL; }
INTERN struct group *LIBCCALL libc_getgrnam(char const *name) { libc_seterrno(ENOSYS); return NULL; }
INTERN void LIBCCALL libc_setgrent(void) { libc_seterrno(ENOSYS); }
INTERN void LIBCCALL libc_endgrent(void) { libc_seterrno(ENOSYS); }
INTERN struct group *LIBCCALL libc_getgrent(void) { libc_seterrno(ENOSYS); return NULL; }
INTERN int LIBCCALL libc_putgrent(struct group const *__restrict p, FILE *__restrict f) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getgrgid_r(gid_t gid, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getgrnam_r(char const *__restrict name, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getgrent_r(struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_fgetgrent_r(FILE *__restrict stream, struct group *__restrict resultbuf, char *__restrict buffer, size_t buflen, struct group **__restrict result) { libc_seterrno(ENOSYS); return -1; }
INTERN struct group *LIBCCALL libc_fgetgrent(FILE *stream) { libc_seterrno(ENOSYS); return NULL; }
INTERN int LIBCCALL libc_setgroups(size_t n, const gid_t *groups) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getgrouplist(char const *user, gid_t group, gid_t *groups, int *ngroups) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_initgroups(char const *user, gid_t group) { libc_seterrno(ENOSYS); return -1; }


EXPORT(getpwuid,libc_getpwuid);
EXPORT(getpwnam,libc_getpwnam);
EXPORT(setpwent,libc_setpwent);
EXPORT(endpwent,libc_endpwent);
EXPORT(getpwent,libc_getpwent);
EXPORT(fgetpwent,libc_fgetpwent);
EXPORT(putpwent,libc_putpwent);
EXPORT(getpwuid_r,libc_getpwuid_r);
EXPORT(getpwnam_r,libc_getpwnam_r);
EXPORT(getpwent_r,libc_getpwent_r);
EXPORT(fgetpwent_r,libc_fgetpwent_r);
EXPORT(getpw,libc_getpw);
EXPORT(getgrgid,libc_getgrgid);
EXPORT(getgrnam,libc_getgrnam);
EXPORT(setgrent,libc_setgrent);
EXPORT(endgrent,libc_endgrent);
EXPORT(getgrent,libc_getgrent);
EXPORT(putgrent,libc_putgrent);
EXPORT(getgrgid_r,libc_getgrgid_r);
EXPORT(getgrnam_r,libc_getgrnam_r);
EXPORT(getgrent_r,libc_getgrent_r);
EXPORT(fgetgrent_r,libc_fgetgrent_r);
EXPORT(fgetgrent,libc_fgetgrent);
EXPORT(setgroups,libc_setgroups);
EXPORT(getgrouplist,libc_getgrouplist);
EXPORT(initgroups,libc_initgroups);


DECL_END

#endif /* !GUARD_LIBS_LIBC_GROUP_C */
