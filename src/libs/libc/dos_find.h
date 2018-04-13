/* Copyright (c) 2018 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following __restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_LIBS_LIBC_DOS_FIND_H
#define GUARD_LIBS_LIBC_DOS_FIND_H 1

#define _finddata32_t       finddata32_t
#define __finddata64_t      finddata64_t
#define _finddata32i64_t    finddata32i64_t
#define _finddata64i32_t    finddata64i32_t
#define _w16finddata32_t    w16finddata32_t
#define _w32finddata32_t    w32finddata32_t
#define _w16finddata64_t    w16finddata64_t
#define _w32finddata64_t    w32finddata64_t
#define _w16finddata32i64_t w16finddata32i64_t
#define _w32finddata32i64_t w32finddata32i64_t
#define _w16finddata64i32_t w16finddata64i32_t
#define _w32finddata64i32_t w32finddata64i32_t

#include "libc.h"
#include "dirent.h"
#include <io.h>
#include <parts/kos2/uio.h>

#ifdef __CC__
DECL_BEGIN

struct find {
    DIR   f_dir;     /* Underlying directory stream. */
    char *f_pattern; /* [1..1][const][owned] Pattern string. */
};

#define FIND_INVALID ((struct find *)(uintptr_t)-1)

INTDEF int LIBCCALL libd_findclose(struct find *findfd);
INTDEF struct find *LIBCCALL libd_findopen(char const *__restrict query, oflag_t oflags);
INTDEF struct find *LIBCCALL libd_w16findopen(char16_t const *__restrict query, oflag_t oflags);
INTDEF struct find *LIBCCALL libd_w32findopen(char32_t const *__restrict query, oflag_t oflags);
INTDEF struct dirent *LIBCCALL libd_findread(struct find *findfd, struct __kos_stat64 *__restrict st);

INTDEF struct find *LIBCCALL libd_findfirst32(char const *__restrict file, struct finddata32_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_findfirst64(char const *__restrict file, struct finddata64_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_findfirst32i64(char const *__restrict file, struct finddata32i64_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_findfirst64i32(char const *__restrict file, struct finddata64i32_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_dos_findfirst32(char const *__restrict file, struct finddata32_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_dos_findfirst64(char const *__restrict file, struct finddata64_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_dos_findfirst32i64(char const *__restrict file, struct finddata32i64_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_dos_findfirst64i32(char const *__restrict file, struct finddata64i32_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_w16findfirst32(char16_t const *__restrict file, struct w16finddata32_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_w32findfirst32(char32_t const *__restrict file, struct w32finddata32_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_w16findfirst64(char16_t const *__restrict file, struct w16finddata64_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_w32findfirst64(char32_t const *__restrict file, struct w32finddata64_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_w16findfirst32i64(char16_t const *__restrict file, struct w16finddata32i64_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_w32findfirst32i64(char32_t const *__restrict file, struct w32finddata32i64_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_w16findfirst64i32(char16_t const *__restrict file, struct w16finddata64i32_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_w32findfirst64i32(char32_t const *__restrict file, struct w32finddata64i32_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_dos_w16findfirst32(char16_t const *__restrict file, struct w16finddata32_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_dos_w32findfirst32(char32_t const *__restrict file, struct w32finddata32_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_dos_w16findfirst64(char16_t const *__restrict file, struct w16finddata64_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_dos_w32findfirst64(char32_t const *__restrict file, struct w32finddata64_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_dos_w16findfirst32i64(char16_t const *__restrict file, struct w16finddata32i64_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_dos_w32findfirst32i64(char32_t const *__restrict file, struct w32finddata32i64_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_dos_w16findfirst64i32(char16_t const *__restrict file, struct w16finddata64i32_t *__restrict finddata);
INTDEF struct find *LIBCCALL libd_dos_w32findfirst64i32(char32_t const *__restrict file, struct w32finddata64i32_t *__restrict finddata);

INTDEF int LIBCCALL libd_findnext32(struct find *findfd, struct finddata32_t *__restrict finddata);
INTDEF int LIBCCALL libd_findnext64(struct find *findfd, struct finddata64_t *__restrict finddata);
INTDEF int LIBCCALL libd_findnext32i64(struct find *findfd, struct finddata32i64_t *__restrict finddata);
INTDEF int LIBCCALL libd_findnext64i32(struct find *findfd, struct finddata64i32_t *__restrict finddata);
INTDEF int LIBCCALL libd_w16findnext32(struct find *findfd, struct w16finddata32_t *__restrict finddata);
INTDEF int LIBCCALL libd_w32findnext32(struct find *findfd, struct w32finddata32_t *__restrict finddata);
INTDEF int LIBCCALL libd_w16findnext64(struct find *findfd, struct w16finddata64_t *__restrict finddata);
INTDEF int LIBCCALL libd_w32findnext64(struct find *findfd, struct w32finddata64_t *__restrict finddata);
INTDEF int LIBCCALL libd_w16findnext32i64(struct find *findfd, struct w16finddata32i64_t *__restrict finddata);
INTDEF int LIBCCALL libd_w32findnext32i64(struct find *findfd, struct w32finddata32i64_t *__restrict finddata);
INTDEF int LIBCCALL libd_w16findnext64i32(struct find *findfd, struct w16finddata64i32_t *__restrict finddata);
INTDEF int LIBCCALL libd_w32findnext64i32(struct find *findfd, struct w32finddata64i32_t *__restrict finddata);


DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_DOS_FIND_H */
