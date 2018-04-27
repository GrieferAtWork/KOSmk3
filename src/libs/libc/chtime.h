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
#ifndef GUARD_LIBS_LIBC_CHTIME_H
#define GUARD_LIBS_LIBC_CHTIME_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     FILESYSTEM TIME MODIFICATION                                                      */
/* ===================================================================================== */
INTDEF int LIBCCALL libc_futime(fd_t fd, struct utimbuf32 const *file_times);
INTDEF int LIBCCALL libc_futime64(fd_t fd, struct utimbuf64 const *file_times);
INTDEF int LIBCCALL libc_futimens(fd_t fd, struct timespec32 const times[2]);
INTDEF int LIBCCALL libc_futimens64(fd_t fd, struct timespec64 const times[2]);
INTDEF int LIBCCALL libc_futimes(fd_t fd, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_futimes64(fd_t fd, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_utimensat(fd_t dfd, char const *path, struct timespec32 const times[2], int flags);
INTDEF int LIBCCALL libc_dos_utimensat(fd_t dfd, char const *path, struct timespec32 const times[2], int flags);
INTDEF int LIBCCALL libc_utimensat64(fd_t dfd, char const *path, struct timespec64 const times[2], int flags);
INTDEF int LIBCCALL libc_dos_utimensat64(fd_t dfd, char const *path, struct timespec64 const times[2], int flags);
INTDEF int LIBCCALL libc_futimeat(fd_t dfd, char const *file, struct utimbuf32 const *file_times, int flags);
INTDEF int LIBCCALL libc_dos_futimeat(fd_t dfd, char const *file, struct utimbuf32 const *file_times, int flags);
INTDEF int LIBCCALL libc_futimeat64(fd_t dfd, char const *file, struct utimbuf64 const *file_times, int flags);
INTDEF int LIBCCALL libc_dos_futimeat64(fd_t dfd, char const *file, struct utimbuf64 const *file_times, int flags);
INTDEF int LIBCCALL libc_utime(char const *file, struct utimbuf32 const *file_times);
INTDEF int LIBCCALL libc_dos_utime(char const *file, struct utimbuf32 const *file_times);
INTDEF int LIBCCALL libc_utime64(char const *file, struct utimbuf64 const *file_times);
INTDEF int LIBCCALL libc_dos_utime64(char const *file, struct utimbuf64 const *file_times);
INTDEF int LIBCCALL libc_impl_futimesat(fd_t dfd, char const *file, struct timeval32 const tvp[2], int flags);
INTDEF int LIBCCALL libc_impl_futimesat64(fd_t dfd, char const *file, struct timeval64 const tvp[2], int flags);
INTDEF int LIBCCALL libc_utimes(char const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_utimes64(char const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_dos_utimes(char const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_dos_utimes64(char const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_lutimes(char const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_lutimes64(char const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_dos_lutimes(char const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_dos_lutimes64(char const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_futimesat(fd_t dfd, char const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_futimesat64(fd_t dfd, char const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_dos_futimesat(fd_t dfd, char const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_dos_futimesat64(fd_t dfd, char const *file, struct timeval64 const tvp[2]);
INTDEF void LIBCCALL libc_Xfutime(fd_t fd, struct utimbuf32 const *file_times);
INTDEF void LIBCCALL libc_Xfutime64(fd_t fd, struct utimbuf64 const *file_times);
INTDEF void LIBCCALL libc_Xfutimens(fd_t fd, struct timespec32 const times[2]);
INTDEF void LIBCCALL libc_Xfutimens64(fd_t fd, struct timespec64 const times[2]);
INTDEF void LIBCCALL libc_Xfutimes(fd_t fd, struct timeval32 const tvp[2]);
INTDEF void LIBCCALL libc_Xfutimes64(fd_t fd, struct timeval64 const tvp[2]);
INTDEF void LIBCCALL libc_Xutimensat(fd_t dfd, char const *path, struct timespec32 const times[2], int flags);
INTDEF void LIBCCALL libc_Xutimensat64(fd_t dfd, char const *path, struct timespec64 const times[2], int flags);
INTDEF void LIBCCALL libc_Xfutimeat(fd_t dfd, char const *file, struct utimbuf32 const *file_times, int flags);
INTDEF void LIBCCALL libc_Xfutimeat64(fd_t dfd, char const *file, struct utimbuf64 const *file_times, int flags);
INTDEF void LIBCCALL libc_Xutime(char const *file, struct utimbuf32 const *file_times);
INTDEF void LIBCCALL libc_Xutime64(char const *file, struct utimbuf64 const *file_times);
INTDEF void LIBCCALL libc_impl_Xfutimesat(fd_t fd, char const *file, struct timeval32 const tvp[2], int flags);
INTDEF void LIBCCALL libc_impl_Xfutimesat64(fd_t fd, char const *file, struct timeval64 const tvp[2], int flags);
INTDEF void LIBCCALL libc_Xutimes(char const *file, struct timeval32 const tvp[2]);
INTDEF void LIBCCALL libc_Xutimes64(char const *file, struct timeval64 const tvp[2]);
INTDEF void LIBCCALL libc_Xlutimes(char const *file, struct timeval32 const tvp[2]);
INTDEF void LIBCCALL libc_Xlutimes64(char const *file, struct timeval64 const tvp[2]);
INTDEF void LIBCCALL libc_Xfutimesat(fd_t fd, char const *file, struct timeval32 const tvp[2]);
INTDEF void LIBCCALL libc_Xfutimesat64(fd_t fd, char const *file, struct timeval64 const tvp[2]);

/* FILESYSTEM TIME MODIFICATION. */
INTDEF int LIBCCALL libc_w16utimensat(fd_t dfd, char16_t const *path, struct timespec32 const times[2], int flags);
INTDEF int LIBCCALL libc_w32utimensat(fd_t dfd, char32_t const *path, struct timespec32 const times[2], int flags);
INTDEF int LIBCCALL libc_dos_w16utimensat(fd_t dfd, char16_t const *path, struct timespec32 const times[2], int flags);
INTDEF int LIBCCALL libc_dos_w32utimensat(fd_t dfd, char32_t const *path, struct timespec32 const times[2], int flags);
INTDEF int LIBCCALL libc_w16utimensat64(fd_t dfd, char16_t const *path, struct timespec64 const times[2], int flags);
INTDEF int LIBCCALL libc_w32utimensat64(fd_t dfd, char32_t const *path, struct timespec64 const times[2], int flags);
INTDEF int LIBCCALL libc_dos_w16utimensat64(fd_t dfd, char16_t const *path, struct timespec64 const times[2], int flags);
INTDEF int LIBCCALL libc_dos_w32utimensat64(fd_t dfd, char32_t const *path, struct timespec64 const times[2], int flags);
INTDEF int LIBCCALL libc_w16futimeat(fd_t dfd, char16_t const *file, struct utimbuf32 const *file_times, int flags);
INTDEF int LIBCCALL libc_w32futimeat(fd_t dfd, char32_t const *file, struct utimbuf32 const *file_times, int flags);
INTDEF int LIBCCALL libc_dos_w16futimeat(fd_t dfd, char16_t const *file, struct utimbuf32 const *file_times, int flags);
INTDEF int LIBCCALL libc_dos_w32futimeat(fd_t dfd, char32_t const *file, struct utimbuf32 const *file_times, int flags);
INTDEF int LIBCCALL libc_w16futimeat64(fd_t dfd, char16_t const *file, struct utimbuf64 const *file_times, int flags);
INTDEF int LIBCCALL libc_w32futimeat64(fd_t dfd, char32_t const *file, struct utimbuf64 const *file_times, int flags);
INTDEF int LIBCCALL libc_dos_w16futimeat64(fd_t dfd, char16_t const *file, struct utimbuf64 const *file_times, int flags);
INTDEF int LIBCCALL libc_dos_w32futimeat64(fd_t dfd, char32_t const *file, struct utimbuf64 const *file_times, int flags);
INTDEF int LIBCCALL libc_w16utime(char16_t const *file, struct utimbuf32 const *file_times);
INTDEF int LIBCCALL libc_w32utime(char32_t const *file, struct utimbuf32 const *file_times);
INTDEF int LIBCCALL libc_dos_w16utime(char16_t const *file, struct utimbuf32 const *file_times);
INTDEF int LIBCCALL libc_dos_w32utime(char32_t const *file, struct utimbuf32 const *file_times);
INTDEF int LIBCCALL libc_w16utime64(char16_t const *file, struct utimbuf64 const *file_times);
INTDEF int LIBCCALL libc_w32utime64(char32_t const *file, struct utimbuf64 const *file_times);
INTDEF int LIBCCALL libc_dos_w16utime64(char16_t const *file, struct utimbuf64 const *file_times);
INTDEF int LIBCCALL libc_dos_w32utime64(char32_t const *file, struct utimbuf64 const *file_times);
INTDEF int LIBCCALL libc_impl_w16futimesat(fd_t dfd, char16_t const *file, struct timeval32 const tvp[2], int flags);
INTDEF int LIBCCALL libc_impl_w32futimesat(fd_t dfd, char32_t const *file, struct timeval32 const tvp[2], int flags);
INTDEF int LIBCCALL libc_impl_w16futimesat64(fd_t dfd, char16_t const *file, struct timeval64 const tvp[2], int flags);
INTDEF int LIBCCALL libc_impl_w32futimesat64(fd_t dfd, char32_t const *file, struct timeval64 const tvp[2], int flags);
INTDEF int LIBCCALL libc_w16utimes(char16_t const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_w32utimes(char32_t const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_dos_w16utimes(char16_t const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_dos_w32utimes(char32_t const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_w16utimes64(char16_t const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_w32utimes64(char32_t const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_dos_w16utimes64(char16_t const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_dos_w32utimes64(char32_t const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_w16lutimes(char16_t const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_w32lutimes(char32_t const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_dos_w16lutimes(char16_t const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_dos_w32lutimes(char32_t const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_w16lutimes64(char16_t const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_w32lutimes64(char32_t const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_dos_w16lutimes64(char16_t const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_dos_w32lutimes64(char32_t const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_w16futimesat(fd_t dfd, char16_t const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_w32futimesat(fd_t dfd, char32_t const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_dos_w16futimesat(fd_t dfd, char16_t const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_dos_w32futimesat(fd_t dfd, char32_t const *file, struct timeval32 const tvp[2]);
INTDEF int LIBCCALL libc_w16futimesat64(fd_t dfd, char16_t const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_w32futimesat64(fd_t dfd, char32_t const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_dos_w16futimesat64(fd_t dfd, char16_t const *file, struct timeval64 const tvp[2]);
INTDEF int LIBCCALL libc_dos_w32futimesat64(fd_t dfd, char32_t const *file, struct timeval64 const tvp[2]);

INTDEF void LIBCCALL libc_Xw16utimensat(fd_t dfd, char16_t const *path, struct timespec32 const times[2], int flags);
INTDEF void LIBCCALL libc_Xw32utimensat(fd_t dfd, char32_t const *path, struct timespec32 const times[2], int flags);
INTDEF void LIBCCALL libc_Xw16utimensat64(fd_t dfd, char16_t const *path, struct timespec64 const times[2], int flags);
INTDEF void LIBCCALL libc_Xw32utimensat64(fd_t dfd, char32_t const *path, struct timespec64 const times[2], int flags);
INTDEF void LIBCCALL libc_Xw16futimeat(fd_t dfd, char16_t const *file, struct utimbuf32 const *file_times, int flags);
INTDEF void LIBCCALL libc_Xw32futimeat(fd_t dfd, char32_t const *file, struct utimbuf32 const *file_times, int flags);
INTDEF void LIBCCALL libc_Xw16futimeat64(fd_t dfd, char16_t const *file, struct utimbuf64 const *file_times, int flags);
INTDEF void LIBCCALL libc_Xw32futimeat64(fd_t dfd, char32_t const *file, struct utimbuf64 const *file_times, int flags);
INTDEF void LIBCCALL libc_Xw16utime(char16_t const *file, struct utimbuf32 const *file_times);
INTDEF void LIBCCALL libc_Xw32utime(char32_t const *file, struct utimbuf32 const *file_times);
INTDEF void LIBCCALL libc_Xw16utime64(char16_t const *file, struct utimbuf64 const *file_times);
INTDEF void LIBCCALL libc_Xw32utime64(char32_t const *file, struct utimbuf64 const *file_times);
INTDEF void LIBCCALL libc_impl_Xw16futimesat(fd_t dfd, char16_t const *file, struct timeval32 const tvp[2], int flags);
INTDEF void LIBCCALL libc_impl_Xw32futimesat(fd_t dfd, char32_t const *file, struct timeval32 const tvp[2], int flags);
INTDEF void LIBCCALL libc_impl_Xw16futimesat64(fd_t dfd, char16_t const *file, struct timeval64 const tvp[2], int flags);
INTDEF void LIBCCALL libc_impl_Xw32futimesat64(fd_t dfd, char32_t const *file, struct timeval64 const tvp[2], int flags);
INTDEF void LIBCCALL libc_Xw16utimes(char16_t const *file, struct timeval32 const tvp[2]);
INTDEF void LIBCCALL libc_Xw32utimes(char32_t const *file, struct timeval32 const tvp[2]);
INTDEF void LIBCCALL libc_Xw16utimes64(char16_t const *file, struct timeval64 const tvp[2]);
INTDEF void LIBCCALL libc_Xw32utimes64(char32_t const *file, struct timeval64 const tvp[2]);
INTDEF void LIBCCALL libc_Xw16lutimes(char16_t const *file, struct timeval32 const tvp[2]);
INTDEF void LIBCCALL libc_Xw32lutimes(char32_t const *file, struct timeval32 const tvp[2]);
INTDEF void LIBCCALL libc_Xw16lutimes64(char16_t const *file, struct timeval64 const tvp[2]);
INTDEF void LIBCCALL libc_Xw32lutimes64(char32_t const *file, struct timeval64 const tvp[2]);
INTDEF void LIBCCALL libc_Xw16futimesat(fd_t dfd, char16_t const *file, struct timeval32 const tvp[2]);
INTDEF void LIBCCALL libc_Xw32futimesat(fd_t dfd, char32_t const *file, struct timeval32 const tvp[2]);
INTDEF void LIBCCALL libc_Xw16futimesat64(fd_t dfd, char16_t const *file, struct timeval64 const tvp[2]);
INTDEF void LIBCCALL libc_Xw32futimesat64(fd_t dfd, char32_t const *file, struct timeval64 const tvp[2]);


DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_CHTIME_H */
