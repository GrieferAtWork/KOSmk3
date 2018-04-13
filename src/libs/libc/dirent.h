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
#ifndef GUARD_LIBS_LIBC_DIRENT_H
#define GUARD_LIBS_LIBC_DIRENT_H 1

#include "libc.h"
#include <kos/types.h>
#include <dirent.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     DIRENT                                                                            */
/* ===================================================================================== */
struct __dirstream {
    int            ds_fd;      /* [const][owned] The handle for the underlying file stream object. */
    size_t         ds_lodsize; /* Amount of bytes returned by the `xreaddir()' system call during its last invocation. */
    size_t         ds_bufsize; /* Size of the directory stream buffer (`ds_buf') in bytes. */
    struct dirent *ds_next;    /* [1..1][in(ds_buf)] Pointer to the next directory yet to-be read. */
    struct dirent *ds_buf;     /* [1..ds_bufsize][owned_if(!= ds_sbuf)] Directory entry buffer. */
    byte_t         ds_sbuf[offsetof(struct dirent,d_name) + 512]; /* Pre-allocated static dirent buffer.
                                                                   * NOTE: When a dynamic buffer has to be used,
                                                                   *      `realloc_in_place()' is used to attempt
                                                                   *       to free this buffer. */
};
#define dirstream_init(self,fd) \
 ((self)->ds_fd      = fd, \
  (self)->ds_lodsize = 0, \
  (self)->ds_bufsize = sizeof((self)->ds_sbuf), \
  (self)->ds_next    = (struct dirent *)(self)->ds_sbuf, \
  (self)->ds_buf     = (struct dirent *)(self)->ds_sbuf)
#define dirstream_fini(self) \
  (((self)->ds_buf != (struct dirent *)(self)->ds_sbuf) ? libc_free((self)->ds_buf) : (void)0, \
     sys_close((self)->ds_fd))

#ifndef __DIR_defined
#define __DIR_defined 1
typedef struct __dirstream DIR;
#endif /* !__DIR_defined */
struct dirent;
struct dirent64;
INTDEF DIR *LIBCCALL libc_fdopendir(fd_t fd);
INTDEF DIR *LIBCCALL libc_opendir(char const *name);
INTDEF DIR *LIBCCALL libc_dos_opendir(char const *name);
INTDEF DIR *LIBCCALL libc_opendirat(fd_t dfd, char const *name);
INTDEF DIR *LIBCCALL libc_dos_opendirat(fd_t dfd, char const *name);
INTDEF DIR *LIBCCALL libc_public_fopendirat(fd_t dfd, char const *name, int flags);
INTDEF DIR *LIBCCALL libc_dos_public_fopendirat(fd_t dfd, char const *name, int flags);
INTDEF int LIBCCALL libc_closedir(DIR *dirp);
INTDEF struct dirent *LIBCCALL libc_readdir(DIR *__restrict dirp);
INTDEF struct dirent64 *LIBCCALL libc_readdir64(DIR *__restrict dirp);
INTDEF void LIBCCALL libc_rewinddir(DIR *__restrict dirp);
INTDEF int LIBCCALL libc_readdir_r(DIR *__restrict dirp, struct dirent *__restrict entry, struct dirent **__restrict result);
INTDEF int LIBCCALL libc_readdir64_r(DIR *__restrict dirp, struct dirent64 *__restrict entry, struct dirent64 **__restrict result);
INTDEF int LIBCCALL libc_seekdir(DIR *__restrict dirp, long int pos);
INTDEF long int LIBCCALL libc_telldir(DIR *__restrict dirp);
INTDEF int LIBCCALL libc_dirfd(DIR *__restrict dirp);
INTDEF int LIBCCALL libc_scandir(char const *__restrict dir, struct dirent ***__restrict namelist, int (*selector)(struct dirent const *), int (*cmp)(struct dirent const **, struct dirent const **));
INTDEF int LIBCCALL libc_scandir64(char const *__restrict dir, struct dirent64 ***__restrict namelist, int (*selector)(struct dirent64 const *), int (*cmp)(struct dirent64 const **, struct dirent64 const **));
INTDEF int LIBCCALL libc_scandirat(fd_t dfd, char const *__restrict dir, struct dirent ***__restrict namelist, int (*selector)(struct dirent const *), int (*cmp)(struct dirent const **, struct dirent const **));
INTDEF int LIBCCALL libc_scandirat64(fd_t dfd, char const *__restrict dir, struct dirent64 ***__restrict namelist, int (*selector)(struct dirent64 const *), int (*cmp)(struct dirent64 const **, struct dirent64 const **));
INTDEF ssize_t LIBCCALL libc_getdirentries(fd_t fd, char *__restrict buf, size_t nbytes, pos_t *__restrict basep);
INTDEF ssize_t LIBCCALL libc_getdirentries64(fd_t fd, char *__restrict buf, size_t nbytes, pos_t *__restrict basep);
INTDEF int LIBCCALL libc_alphasort(struct dirent const **e1, struct dirent const **e2);
INTDEF int LIBCCALL libc_alphasort64(struct dirent64 const **e1, struct dirent64 const **e2);
INTDEF int LIBCCALL libc_versionsort(struct dirent const **e1, struct dirent const **e2);
INTDEF int LIBCCALL libc_versionsort64(struct dirent64 const **e1, struct dirent64 const **e2);
INTDEF ssize_t LIBCCALL libc_xreaddir(fd_t fd, struct dirent *buf, size_t bufsize, int mode);
INTDEF ssize_t LIBCCALL libc_xreaddirf(fd_t fd, struct dirent *buf, size_t bufsize, int mode, oflag_t flags);
INTDEF ssize_t LIBCCALL libc_xreaddir64(fd_t fd, struct dirent64 *buf, size_t bufsize, int mode);
INTDEF ssize_t LIBCCALL libc_xreaddirf64(fd_t fd, struct dirent64 *buf, size_t bufsize, int mode, oflag_t flags);
INTDEF ATTR_RETNONNULL DIR *LIBCCALL libc_Xfdopendir(fd_t fd);
INTDEF ATTR_RETNONNULL DIR *LIBCCALL libc_Xopendir(char const *name);
INTDEF ATTR_RETNONNULL DIR *LIBCCALL libc_Xopendirat(fd_t dfd, char const *name);
INTDEF ATTR_RETNONNULL DIR *LIBCCALL libc_public_Xfopendirat(fd_t dfd, char const *name, int flags);
INTDEF struct dirent *LIBCCALL libc_Xreaddir(DIR *__restrict dirp);
INTDEF struct dirent64 *LIBCCALL libc_Xreaddir64(DIR *__restrict dirp);
INTDEF size_t LIBCCALL libc_Xxreaddir(fd_t fd, struct dirent *buf, size_t bufsize, int mode);
INTDEF size_t LIBCCALL libc_Xxreaddirf(fd_t fd, struct dirent *buf, size_t bufsize, int mode, oflag_t flags);
INTDEF size_t LIBCCALL libc_Xxreaddir64(fd_t fd, struct dirent64 *buf, size_t bufsize, int mode);
INTDEF size_t LIBCCALL libc_Xxreaddirf64(fd_t fd, struct dirent64 *buf, size_t bufsize, int mode, oflag_t flags);
INTDEF void LIBCCALL libc_Xreaddir_r(DIR *__restrict dirp, struct dirent *__restrict entry, struct dirent **__restrict result);
INTDEF void LIBCCALL libc_Xreaddir64_r(DIR *__restrict dirp, struct dirent64 *__restrict entry, struct dirent64 **__restrict result);
INTDEF void LIBCCALL libc_Xseekdir(DIR *__restrict dirp, long int pos);
INTDEF unsigned long int LIBCCALL libc_Xtelldir(DIR *__restrict dirp);
INTDEF void LIBCCALL libc_Xscandir(char const *__restrict dir, struct dirent ***__restrict namelist, int (*selector)(struct dirent const *), int (*cmp)(struct dirent const **, struct dirent const **));
INTDEF void LIBCCALL libc_Xscandir64(char const *__restrict dir, struct dirent64 ***__restrict namelist, int (*selector)(struct dirent64 const *), int (*cmp)(struct dirent64 const **, struct dirent64 const **));
INTDEF void LIBCCALL libc_Xscandirat(fd_t dfd, char const *__restrict dir, struct dirent ***__restrict namelist, int (*selector)(struct dirent const *), int (*cmp)(struct dirent const **, struct dirent const **));
INTDEF void LIBCCALL libc_Xscandirat64(fd_t dfd, char const *__restrict dir, struct dirent64 ***__restrict namelist, int (*selector)(struct dirent64 const *), int (*cmp)(struct dirent64 const **, struct dirent64 const **));
INTDEF size_t LIBCCALL libc_Xgetdirentries(fd_t fd, char *__restrict buf, size_t nbytes, pos_t *__restrict basep);
INTDEF size_t LIBCCALL libc_Xgetdirentries64(fd_t fd, char *__restrict buf, size_t nbytes, pos_t *__restrict basep);
/* DOS's find functions. */

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_DIRENT_H */
