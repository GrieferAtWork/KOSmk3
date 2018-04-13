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
#ifndef GUARD_LIBS_LIBC_MOUNT_H
#define GUARD_LIBS_LIBC_MOUNT_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     MOUNT                                                                             */
/* ===================================================================================== */
struct mntent;
INTDEF FILE *LIBCCALL libc_setmntent(char const *file, char const *mode);
INTDEF struct mntent *LIBCCALL libc_getmntent(FILE *__restrict stream);
INTDEF int LIBCCALL libc_endmntent(FILE *stream);
INTDEF struct mntent *LIBCCALL libc_getmntent_r(FILE *__restrict stream, struct mntent *__restrict result, char *__restrict buffer, int bufsize);
INTDEF int LIBCCALL libc_addmntent(FILE *__restrict stream, struct mntent const *__restrict mnt);
INTDEF char *LIBCCALL libc_hasmntopt(struct mntent const *mnt, char const *opt);
INTDEF int LIBCCALL libc_mount(char const *special_file, char const *dir, char const *fstype, unsigned long int rwflag, void const *data);
INTDEF int LIBCCALL libc_umount(char const *special_file);
INTDEF int LIBCCALL libc_umount2(char const *special_file, int flags);
INTDEF void LIBCCALL libc_Xmount(char const *special_file, char const *dir, char const *fstype, unsigned long int rwflag, void const *data);
INTDEF void LIBCCALL libc_Xumount(char const *special_file);
INTDEF void LIBCCALL libc_Xumount2(char const *special_file, int flags);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_MOUNT_H */
