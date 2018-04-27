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
#ifndef GUARD_LIBS_LIBC_STAT_H
#define GUARD_LIBS_LIBC_STAT_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     STAT                                                                              */
/* ===================================================================================== */
/* STAT (KOS) */
struct __kos_stat;
struct __kos_stat64;
INTDEF int LIBCCALL libc_kfstat(fd_t fd, struct __kos_stat *buf);
INTDEF int LIBCCALL libc_kfstat64(fd_t fd, struct __kos_stat64 *buf);
INTDEF int LIBCCALL libc_kstat(char const *file, struct __kos_stat *buf);
INTDEF int LIBCCALL libc_kstat64(char const *file, struct __kos_stat64 *buf);
INTDEF int LIBCCALL libc_kfstatat(fd_t dfd, char const *file, struct __kos_stat *buf, int flags);
INTDEF int LIBCCALL libc_kfstatat64(fd_t dfd, char const *file, struct __kos_stat64 *buf, int flags);
INTDEF int LIBCCALL libc_klstat(char const *file, struct __kos_stat *buf);
INTDEF int LIBCCALL libc_klstat64(char const *file, struct __kos_stat64 *buf);
INTDEF int LIBCCALL libc_dos_kstat(char const *file, struct __kos_stat *buf);
INTDEF int LIBCCALL libc_dos_kstat64(char const *file, struct __kos_stat64 *buf);
INTDEF int LIBCCALL libc_dos_kfstatat(fd_t dfd, char const *file, struct __kos_stat *buf, int flags);
INTDEF int LIBCCALL libc_dos_kfstatat64(fd_t dfd, char const *file, struct __kos_stat64 *buf, int flags);
INTDEF int LIBCCALL libc_dos_klstat(char const *file, struct __kos_stat *buf);
INTDEF int LIBCCALL libc_dos_klstat64(char const *file, struct __kos_stat64 *buf);
/* STAT (GLC) */
struct __glc_stat;
struct __glc_stat64;
INTDEF int LIBCCALL libc_fstat(fd_t fd, struct __glc_stat *buf);
INTDEF int LIBCCALL libc_fstat64(fd_t fd, struct __glc_stat64 *buf);
INTDEF int LIBCCALL libc_stat(char const *file, struct __glc_stat *buf);
INTDEF int LIBCCALL libc_stat64(char const *file, struct __glc_stat64 *buf);
INTDEF int LIBCCALL libc_fstatat(fd_t dfd, char const *file, struct __glc_stat *buf, int flags);
INTDEF int LIBCCALL libc_fstatat64(fd_t dfd, char const *file, struct __glc_stat64 *buf, int flags);
INTDEF int LIBCCALL libc_lstat(char const *file, struct __glc_stat *buf);
INTDEF int LIBCCALL libc_lstat64(char const *file, struct __glc_stat64 *buf);
INTDEF int LIBCCALL libc_dos_stat(char const *file, struct __glc_stat *buf);
INTDEF int LIBCCALL libc_dos_stat64(char const *file, struct __glc_stat64 *buf);
INTDEF int LIBCCALL libc_dos_fstatat(fd_t dfd, char const *file, struct __glc_stat *buf, int flags);
INTDEF int LIBCCALL libc_dos_fstatat64(fd_t dfd, char const *file, struct __glc_stat64 *buf, int flags);
INTDEF int LIBCCALL libc_dos_lstat(char const *file, struct __glc_stat *buf);
INTDEF int LIBCCALL libc_dos_lstat64(char const *file, struct __glc_stat64 *buf);
INTDEF int LIBCCALL libc_version_fxstat(int ver, fd_t fd, struct __glc_stat *statbuf);
INTDEF int LIBCCALL libc_version_fxstat64(int ver, fd_t fd, struct __glc_stat64 *statbuf);
INTDEF int LIBCCALL libc_version_xstat(int ver, char const *filename, struct __glc_stat *statbuf);
INTDEF int LIBCCALL libc_version_xstat64(int ver, char const *filename, struct __glc_stat64 *statbuf);
INTDEF int LIBCCALL libc_version_lxstat(int ver, char const *filename, struct __glc_stat *statbuf);
INTDEF int LIBCCALL libc_version_lxstat64(int ver, char const *filename, struct __glc_stat64 *statbuf);
INTDEF int LIBCCALL libc_version_fxstatat(int ver, fd_t fd, char const *filename, struct __glc_stat *statbuf, int flags);
INTDEF int LIBCCALL libc_version_fxstatat64(int ver, fd_t fd, char const *filename, struct __glc_stat64 *statbuf, int flags);
INTDEF int LIBCCALL libc_dos_version_xstat(int ver, char const *filename, struct __glc_stat *statbuf);
INTDEF int LIBCCALL libc_dos_version_xstat64(int ver, char const *filename, struct __glc_stat64 *statbuf);
INTDEF int LIBCCALL libc_dos_version_lxstat(int ver, char const *filename, struct __glc_stat *statbuf);
INTDEF int LIBCCALL libc_dos_version_lxstat64(int ver, char const *filename, struct __glc_stat64 *statbuf);
INTDEF int LIBCCALL libc_dos_version_fxstatat(int ver, fd_t fd, char const *filename, struct __glc_stat *statbuf, int flags);
INTDEF int LIBCCALL libc_dos_version_fxstatat64(int ver, fd_t fd, char const *filename, struct __glc_stat64 *statbuf, int flags);
/* STAT (DOS) */
struct __dos_stat32;
struct __dos_stat32i64;
struct __dos_stat64;
struct utimbuf32;
struct utimbuf64;
INTDEF int LIBCCALL libd_fstat32(fd_t fd, struct __dos_stat32 *buf);
INTDEF int LIBCCALL libd_fstat32i64(fd_t fd, struct __dos_stat32i64 *buf);
INTDEF int LIBCCALL libd_fstat64(fd_t fd, struct __dos_stat64 *buf);
INTDEF int LIBCCALL libd_stat32(char const *file, struct __dos_stat32 *buf);
INTDEF int LIBCCALL libd_stat32i64(char const *file, struct __dos_stat32i64 *buf);
INTDEF int LIBCCALL libd_stat64(char const *file, struct __dos_stat64 *buf);
INTDEF int LIBCCALL libd_dos_stat32(char const *file, struct __dos_stat32 *buf);
INTDEF int LIBCCALL libd_dos_stat32i64(char const *file, struct __dos_stat32i64 *buf);
INTDEF int LIBCCALL libd_dos_stat64(char const *file, struct __dos_stat64 *buf);

INTDEF void LIBCCALL libc_Xkfstat(fd_t fd, struct __kos_stat *buf);
INTDEF void LIBCCALL libc_Xkfstat64(fd_t fd, struct __kos_stat64 *buf);
INTDEF void LIBCCALL libc_Xkstat(char const *file, struct __kos_stat *buf);
INTDEF void LIBCCALL libc_Xkstat64(char const *file, struct __kos_stat64 *buf);
INTDEF void LIBCCALL libc_Xkfstatat(fd_t dfd, char const *file, struct __kos_stat *buf, int flags);
INTDEF void LIBCCALL libc_Xkfstatat64(fd_t dfd, char const *file, struct __kos_stat64 *buf, int flags);
INTDEF void LIBCCALL libc_Xklstat(char const *file, struct __kos_stat *buf);
INTDEF void LIBCCALL libc_Xklstat64(char const *file, struct __kos_stat64 *buf);

/* STAT (KOS) */
struct __kos_stat;
struct __kos_stat64;
INTDEF int LIBCCALL libc_kw16stat(char16_t const *file, struct __kos_stat *buf);
INTDEF int LIBCCALL libc_kw32stat(char32_t const *file, struct __kos_stat *buf);
INTDEF int LIBCCALL libc_kw16stat64(char16_t const *file, struct __kos_stat64 *buf);
INTDEF int LIBCCALL libc_kw32stat64(char32_t const *file, struct __kos_stat64 *buf);
INTDEF int LIBCCALL libc_kw16fstatat(fd_t dfd, char16_t const *file, struct __kos_stat *buf, int flags);
INTDEF int LIBCCALL libc_kw32fstatat(fd_t dfd, char32_t const *file, struct __kos_stat *buf, int flags);
INTDEF int LIBCCALL libc_kw16fstatat64(fd_t dfd, char16_t const *file, struct __kos_stat64 *buf, int flags);
INTDEF int LIBCCALL libc_kw32fstatat64(fd_t dfd, char32_t const *file, struct __kos_stat64 *buf, int flags);
INTDEF int LIBCCALL libc_kw16lstat(char16_t const *file, struct __kos_stat *buf);
INTDEF int LIBCCALL libc_kw32lstat(char32_t const *file, struct __kos_stat *buf);
INTDEF int LIBCCALL libc_kw16lstat64(char16_t const *file, struct __kos_stat64 *buf);
INTDEF int LIBCCALL libc_kw32lstat64(char32_t const *file, struct __kos_stat64 *buf);
INTDEF int LIBCCALL libc_dos_kw16stat(char16_t const *file, struct __kos_stat *buf);
INTDEF int LIBCCALL libc_dos_kw32stat(char32_t const *file, struct __kos_stat *buf);
INTDEF int LIBCCALL libc_dos_kw16stat64(char16_t const *file, struct __kos_stat64 *buf);
INTDEF int LIBCCALL libc_dos_kw32stat64(char32_t const *file, struct __kos_stat64 *buf);
INTDEF int LIBCCALL libc_dos_kw16fstatat(fd_t dfd, char16_t const *file, struct __kos_stat *buf, int flags);
INTDEF int LIBCCALL libc_dos_kw32fstatat(fd_t dfd, char32_t const *file, struct __kos_stat *buf, int flags);
INTDEF int LIBCCALL libc_dos_kw16fstatat64(fd_t dfd, char16_t const *file, struct __kos_stat64 *buf, int flags);
INTDEF int LIBCCALL libc_dos_kw32fstatat64(fd_t dfd, char32_t const *file, struct __kos_stat64 *buf, int flags);
INTDEF int LIBCCALL libc_dos_kw16lstat(char16_t const *file, struct __kos_stat *buf);
INTDEF int LIBCCALL libc_dos_kw32lstat(char32_t const *file, struct __kos_stat *buf);
INTDEF int LIBCCALL libc_dos_kw16lstat64(char16_t const *file, struct __kos_stat64 *buf);
INTDEF int LIBCCALL libc_dos_kw32lstat64(char32_t const *file, struct __kos_stat64 *buf);
INTDEF void LIBCCALL libc_Xkw16stat(char16_t const *file, struct __kos_stat *buf);
INTDEF void LIBCCALL libc_Xkw32stat(char32_t const *file, struct __kos_stat *buf);
INTDEF void LIBCCALL libc_Xkw16stat64(char16_t const *file, struct __kos_stat64 *buf);
INTDEF void LIBCCALL libc_Xkw32stat64(char32_t const *file, struct __kos_stat64 *buf);
INTDEF void LIBCCALL libc_Xkw16fstatat(fd_t dfd, char16_t const *file, struct __kos_stat *buf, int flags);
INTDEF void LIBCCALL libc_Xkw32fstatat(fd_t dfd, char32_t const *file, struct __kos_stat *buf, int flags);
INTDEF void LIBCCALL libc_Xkw16fstatat64(fd_t dfd, char16_t const *file, struct __kos_stat64 *buf, int flags);
INTDEF void LIBCCALL libc_Xkw32fstatat64(fd_t dfd, char32_t const *file, struct __kos_stat64 *buf, int flags);
INTDEF void LIBCCALL libc_Xkw16lstat(char16_t const *file, struct __kos_stat *buf);
INTDEF void LIBCCALL libc_Xkw32lstat(char32_t const *file, struct __kos_stat *buf);
INTDEF void LIBCCALL libc_Xkw16lstat64(char16_t const *file, struct __kos_stat64 *buf);
INTDEF void LIBCCALL libc_Xkw32lstat64(char32_t const *file, struct __kos_stat64 *buf);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_STAT_H */
