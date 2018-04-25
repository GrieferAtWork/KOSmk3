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
#ifndef GUARD_LIBS_LIBC_DL_H
#define GUARD_LIBS_LIBC_DL_H 1

#include "libc.h"
#include <kos/dl.h>
#include <kos/types.h>
#include <kos/addr2line.h>

#ifdef __CC__
DECL_BEGIN

INTDEF ssize_t LIBCCALL libc_xdlmodule_info(void *handle, int info_class, void *buf, size_t bufsize);
INTDEF void *LIBCCALL libc_xfdlopenat(fd_t dfd, char const *path, atflag_t at_flags, int open_flags, char const *runpath);
INTDEF void *LIBCCALL libc_dos_xfdlopenat(fd_t dfd, char const *path, atflag_t at_flags, int open_flags, char const *runpath);
INTDEF void *LIBCCALL libc_xdlopen(char const *filename, int open_flags);
INTDEF void *LIBCCALL libc_dos_xdlopen(char const *filename, int open_flags);
INTDEF void *LIBCCALL libc_xfdlopen(fd_t fd, int open_flags);
INTDEF void *LIBCCALL libc_xdlsym(void *handle, char const *symbol);
INTDEF int LIBCCALL libc_xdlclose(void *handle);

INTDEF size_t LIBCCALL libc_Xxdlmodule_info(void *handle, int info_class, void *buf, size_t bufsize);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xxfdlopenat(fd_t dfd, char const *path, atflag_t at_flags, int open_flags, char const *runpath);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xxdlopen(char const *filename, int open_flags);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xxfdlopen(fd_t fd, int open_flags);
INTDEF ATTR_RETNONNULL void *LIBCCALL libc_Xxdlsym(void *handle, char const *symbol);
INTDEF void LIBCCALL libc_Xxdlclose(void *handle);

INTDEF void *LIBCCALL libc_loaddll(char *file);
INTDEF void *LIBCCALL libc_dos_loaddll(char *file);
INTDEF int LIBCCALL libc_unloaddll(void *hnd);
INTDEF void *LIBCCALL libc_getdllprocaddr(void *hnd, char const *symname, intptr_t ord);


struct dl_addr2line;
struct cpu_context;
struct fpu_context;
INTDEF ssize_t LIBCCALL libc_xdladdr2line(void *abs_pc, struct dl_addr2line *__restrict buf, size_t bufsize);
INTDEF size_t LIBCCALL libc_Xxdladdr2line(void *abs_pc, struct dl_addr2line *__restrict buf, size_t bufsize);
INTDEF int LIBCCALL libc_xunwind(struct cpu_context *__restrict ccontext, struct fpu_context *fcontext, sigset_t *signal_set);
INTDEF bool LIBCCALL libc_Xxunwind(struct cpu_context *__restrict ccontext, struct fpu_context *fcontext, sigset_t *signal_set);
INTDEF char *LIBCCALL libc_xdlpath(void *handle, char *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_xdlpath2(void *handle, char *buf, size_t bufsize, unsigned int type);
INTDEF ATTR_RETNONNULL char *LIBCCALL libc_Xxdlpath(void *handle, char *buf, size_t bufsize, unsigned int type);
INTDEF size_t LIBCCALL libc_Xxdlpath2(void *handle, char *buf, size_t bufsize, unsigned int type);
INTDEF char *LIBCCALL libc_dos_xdlpath(void *handle, char *buf, size_t bufsize, unsigned int type);
INTDEF ssize_t LIBCCALL libc_dos_xdlpath2(void *handle, char *buf, size_t bufsize, unsigned int type);


DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_DL_H */
