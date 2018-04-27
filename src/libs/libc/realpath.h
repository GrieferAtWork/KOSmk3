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
#ifndef GUARD_LIBS_LIBC_REALPATH_H
#define GUARD_LIBS_LIBC_REALPATH_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     REALPATH                                                                          */
/* ===================================================================================== */
INTDEF char *LIBCCALL libc_getcwd(char *buf, size_t size);
INTDEF char *LIBCCALL libc_dos_getcwd(char *buf, size_t size);
INTDEF char *LIBCCALL libc_getwd(char *buf);
INTDEF char *LIBCCALL libc_dos_getwd(char *buf);
INTDEF char *LIBCCALL libc_get_current_dir_name(void);
INTDEF char *LIBCCALL libc_dos_get_current_dir_name(void);
INTDEF char *LIBCCALL libc_xfdname(fd_t fd, int type, char *buf, size_t bufsize);
INTDEF char *LIBCCALL libc_dos_xfdname(fd_t fd, int type, char *buf, size_t bufsize);
INTDEF char *LIBCCALL libc_xfrealpath(fd_t fd, char *buf, size_t bufsize, unsigned int type);
INTDEF char *LIBCCALL libc_dos_xfrealpath(fd_t fd, char *buf, size_t bufsize, unsigned int type);
INTDEF char *LIBCCALL libc_xrealpath(char const *path, char *buf, size_t bufsize, unsigned int type);
INTDEF char *LIBCCALL libc_dos_xrealpath(char const *path, char *buf, size_t bufsize, unsigned int type);
INTDEF char *LIBCCALL libc_xfrealpathat(fd_t dfd, char const *path, int flags, char *buf, size_t bufsize, unsigned int type);
INTDEF char *LIBCCALL libc_dos_xfrealpathat(fd_t dfd, char const *path, int flags, char *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_xfdname2(fd_t fd, int type, char *buf, size_t bufsize);
INTDEF ssize_t LIBCCALL libc_dos_xfdname2(fd_t fd, int type, char *buf, size_t bufsize);
INTDEF ssize_t LIBCCALL libc_xfrealpath2(fd_t fd, char *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_dos_xfrealpath2(fd_t fd, char *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_xrealpath2(char const *path, char *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_dos_xrealpath2(char const *path, char *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_xfrealpathat2(fd_t dfd, char const *path, int flags, char *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_dos_xfrealpathat2(fd_t dfd, char const *path, int flags, char *buf, size_t bufsize, unsigned int type);
INTDEF char *LIBCCALL libc_realpath(char const *name, char *resolved);
INTDEF char *LIBCCALL libc_dos_realpath(char const *name, char *resolved);
INTDEF char *LIBCCALL libc_frealpath(fd_t fd, char *resolved, size_t bufsize);
INTDEF char *LIBCCALL libc_dos_frealpath(fd_t fd, char *resolved, size_t bufsize);
INTDEF ATTR_MALLOC char *LIBCCALL libc_canonicalize_file_name(char const *name);
INTDEF ATTR_MALLOC char *LIBCCALL libc_dos_canonicalize_file_name(char const *name);
INTDEF ATTR_RETNONNULL char *LIBCCALL libc_Xgetcwd(char *buf, size_t size);
INTDEF ATTR_RETNONNULL char *LIBCCALL libc_Xget_current_dir_name(void);
INTDEF ATTR_RETNONNULL char *LIBCCALL libc_Xrealpath(char const *name, char *resolved);
INTDEF ATTR_RETNONNULL char *LIBCCALL libc_Xfrealpath(fd_t fd, char *resolved, size_t bufsize);
INTDEF ATTR_MALLOC ATTR_RETNONNULL char *LIBCCALL libc_Xcanonicalize_file_name(char const *name);
INTDEF ATTR_RETNONNULL char *LIBCCALL libc_Xxfrealpath(fd_t fd, char *buf, size_t bufsize, unsigned int type);
INTDEF ATTR_RETNONNULL char *LIBCCALL libc_Xxrealpath(char const *path, char *buf, size_t bufsize, unsigned int type);
INTDEF ATTR_RETNONNULL char *LIBCCALL libc_Xxfrealpathat(fd_t dfd, char const *path, int flags, char *buf, size_t bufsize, unsigned int type);
INTDEF size_t LIBCCALL libc_Xxfrealpath2(fd_t fd, char *buf, size_t bufsize, unsigned int type);
INTDEF size_t LIBCCALL libc_Xxrealpath2(char const *path, char *buf, size_t bufsize, unsigned int type);
INTDEF size_t LIBCCALL libc_Xxfrealpathat2(fd_t dfd, char const *path, int flags, char *buf, size_t bufsize, unsigned int type);

INTDEF char16_t *LIBCCALL libc_xw16frealpath(fd_t fd, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF char32_t *LIBCCALL libc_xw32frealpath(fd_t fd, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF char16_t *LIBCCALL libc_xw16realpath(char16_t const *path, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF char32_t *LIBCCALL libc_xw32realpath(char32_t const *path, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF char16_t *LIBCCALL libc_xw16frealpathat(fd_t dfd, char16_t const *path, int flags, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF char32_t *LIBCCALL libc_xw32frealpathat(fd_t dfd, char32_t const *path, int flags, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF char16_t *LIBCCALL libc_dos_xw16realpath(char16_t const *path, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF char32_t *LIBCCALL libc_dos_xw32realpath(char32_t const *path, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF char16_t *LIBCCALL libc_dos_xw16frealpathat(fd_t dfd, char16_t const *path, int flags, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF char32_t *LIBCCALL libc_dos_xw32frealpathat(fd_t dfd, char32_t const *path, int flags, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_xw16frealpath2(fd_t fd, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_xw32frealpath2(fd_t fd, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_xw16realpath2(char16_t const *path, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_xw32realpath2(char32_t const *path, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_xw16frealpathat2(fd_t dfd, char16_t const *path, int flags, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_xw32frealpathat2(fd_t dfd, char32_t const *path, int flags, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_dos_xw16realpath2(char16_t const *path, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_dos_xw32realpath2(char32_t const *path, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_dos_xw16frealpathat2(fd_t dfd, char16_t const *path, int flags, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_dos_xw32frealpathat2(fd_t dfd, char32_t const *path, int flags, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF ATTR_RETNONNULL char16_t *LIBCCALL libc_Xxw16frealpath(fd_t fd, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF ATTR_RETNONNULL char32_t *LIBCCALL libc_Xxw32frealpath(fd_t fd, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF ATTR_RETNONNULL char16_t *LIBCCALL libc_Xxw16realpath(char16_t const *path, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF ATTR_RETNONNULL char32_t *LIBCCALL libc_Xxw32realpath(char32_t const *path, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF ATTR_RETNONNULL char16_t *LIBCCALL libc_Xxw16frealpathat(fd_t dfd, char16_t const *path, int flags, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF ATTR_RETNONNULL char32_t *LIBCCALL libc_Xxw32frealpathat(fd_t dfd, char32_t const *path, int flags, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF size_t LIBCCALL libc_Xxw16frealpath2(fd_t fd, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF size_t LIBCCALL libc_Xxw32frealpath2(fd_t fd, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF size_t LIBCCALL libc_Xxw16realpath2(char16_t const *path, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF size_t LIBCCALL libc_Xxw32realpath2(char32_t const *path, char32_t *buf, size_t bufsize, unsigned int type);
INTDEF size_t LIBCCALL libc_Xxw16frealpathat2(fd_t dfd, char16_t const *path, int flags, char16_t *buf, size_t bufsize, unsigned int type);
INTDEF size_t LIBCCALL libc_Xxw32frealpathat2(fd_t dfd, char32_t const *path, int flags, char32_t *buf, size_t bufsize, unsigned int type);

INTDEF char16_t *LIBCCALL libc_w16realpath(char16_t const *name, char16_t *resolved);
INTDEF char32_t *LIBCCALL libc_w32realpath(char32_t const *name, char32_t *resolved);
INTDEF char16_t *LIBCCALL libc_dos_w16realpath(char16_t const *name, char16_t *resolved);
INTDEF char32_t *LIBCCALL libc_dos_w32realpath(char32_t const *name, char32_t *resolved);
INTDEF char16_t *LIBCCALL libc_w16frealpath(fd_t fd, char16_t *resolved, size_t bufsize);
INTDEF char32_t *LIBCCALL libc_w32frealpath(fd_t fd, char32_t *resolved, size_t bufsize);
INTDEF char16_t *LIBCCALL libc_dos_w16frealpath(fd_t fd, char16_t *resolved, size_t bufsize);
INTDEF char32_t *LIBCCALL libc_dos_w32frealpath(fd_t fd, char32_t *resolved, size_t bufsize);
INTDEF ATTR_MALLOC char16_t *LIBCCALL libc_w16canonicalize_file_name(char16_t const *name);
INTDEF ATTR_MALLOC char32_t *LIBCCALL libc_w32canonicalize_file_name(char32_t const *name);
INTDEF ATTR_MALLOC char16_t *LIBCCALL libc_dos_w16canonicalize_file_name(char16_t const *name);
INTDEF ATTR_MALLOC char32_t *LIBCCALL libc_dos_w32canonicalize_file_name(char32_t const *name);
INTDEF ATTR_RETNONNULL char16_t *LIBCCALL libc_Xw16realpath(char16_t const *name, char16_t *resolved);
INTDEF ATTR_RETNONNULL char32_t *LIBCCALL libc_Xw32realpath(char32_t const *name, char32_t *resolved);
INTDEF ATTR_RETNONNULL char16_t *LIBCCALL libc_Xw16frealpath(fd_t fd, char16_t *resolved, size_t bufsize);
INTDEF ATTR_RETNONNULL char32_t *LIBCCALL libc_Xw32frealpath(fd_t fd, char32_t *resolved, size_t bufsize);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char16_t *LIBCCALL libc_Xw16canonicalize_file_name(char16_t const *name);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char32_t *LIBCCALL libc_Xw32canonicalize_file_name(char32_t const *name);

INTDEF char *LIBCCALL libd_getdcwd(int drive, char *buf, size_t size);
INTDEF char *LIBCCALL libd_dos_getdcwd(int drive, char *buf, size_t size);
INTDEF int LIBCCALL libd_chdrive(int drive);
INTDEF int LIBCCALL libd_getdrive(void);
INTDEF u32 LIBCCALL libd_getdrives(void);
INTDEF ATTR_RETNONNULL char *LIBCCALL libd_Xgetdcwd(int drive, char *buf, size_t size);
INTDEF void LIBCCALL libd_Xchdrive(int drive);
INTDEF int LIBCCALL libd_Xgetdrive(void);


DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_REALPATH_H */
