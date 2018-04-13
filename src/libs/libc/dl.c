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
#ifndef GUARD_LIBS_LIBC_DL_C
#define GUARD_LIBS_LIBC_DL_C 1

#include "libc.h"
#include "system.h"
#include "errno.h"
#include "dl.h"
#include <kos/dl.h>
#include <errno.h>
#include <except.h>

DECL_BEGIN

EXPORT(xdlmodule_info,libc_xdlmodule_info);
CRT_KOS_DL ssize_t LIBCCALL
libc_xdlmodule_info(void *handle, int info_class,
                    void *buf, size_t bufsize) {
 ssize_t result;
 result = Xsys_xdlmodule_info(handle,info_class,buf,bufsize);
 if (E_ISOK(result)) return result;
 libc_seterrno(-result);
 return -1;
}

EXPORT(__KSYM(xfdlopenat),libc_xfdlopenat);
CRT_KOS_DL void *LIBCCALL
libc_xfdlopenat(fd_t dfd, char const *path, atflag_t at_flags,
                int open_flags, char const *runpath) {
 void *result;
 result = sys_xfdlopenat(dfd,path,at_flags,open_flags,runpath);
 if (E_ISOK(result)) return result;
 libc_seterrno(-E_GTERR(result));
 return NULL;
}

EXPORT(__DSYM(xfdlopenat),libc_dos_xfdlopenat);
CRT_KOS_DL void *LIBCCALL
libc_dos_xfdlopenat(fd_t dfd, char const *path, atflag_t at_flags,
                    int open_flags, char const *runpath) {
 return libc_xfdlopenat(dfd,path,at_flags|AT_DOSPATH,open_flags,runpath);
}

EXPORT(__KSYM(xdlopen),libc_xdlopen);
CRT_KOS_DL void *LIBCCALL
libc_xdlopen(char const *filename, int open_flags) {
 return libc_xfdlopenat(AT_FDCWD,filename,0,open_flags,NULL);
}

EXPORT(__DSYM(xdlopen),libc_dos_xdlopen);
CRT_KOS_DL void *LIBCCALL
libc_dos_xdlopen(char const *filename, int open_flags) {
 return libc_xfdlopenat(AT_FDCWD,filename,AT_DOSPATH,open_flags,NULL);
}

EXPORT(xfdlopen,libc_xfdlopen);
CRT_KOS_DL void *LIBCCALL
libc_xfdlopen(fd_t fd, int open_flags) {
 return libc_xfdlopenat(fd,libc_empty_string,AT_EMPTY_PATH,open_flags,NULL);
}

EXPORT(xdlsym,libc_xdlsym);
CRT_KOS_DL void *LIBCCALL
libc_xdlsym(void *handle, char const *symbol) {
 void *result;
 result = sys_xdlsym(handle,symbol);
 if (E_ISOK(result)) return result;
 libc_seterrno(-E_GTERR(result));
 return NULL;
}

EXPORT(xdlclose,libc_xdlclose);
CRT_KOS_DL int LIBCCALL
libc_xdlclose(void *handle) {
 return FORWARD_SYSTEM_ERROR(sys_xdlclose(handle));
}

EXPORT(Xxdlopen,libc_Xxdlopen);
CRT_KOS_DL ATTR_RETNONNULL void *LIBCCALL
libc_Xxdlopen(char const *filename, int open_flags) {
 return libc_Xxfdlopenat(AT_FDCWD,filename,0,open_flags,NULL);
}

EXPORT(Xxfdlopen,libc_Xxfdlopen);
CRT_KOS_DL ATTR_RETNONNULL void *LIBCCALL
libc_Xxfdlopen(fd_t fd, int open_flags) {
 return libc_Xxfdlopenat(fd,libc_empty_string,AT_EMPTY_PATH,open_flags,NULL);
}


EXPORT(__KSYM(_loaddll),libc_loaddll);
CRT_DOS void *LIBCCALL libc_loaddll(char *file) {
 return libc_xdlopen(file,DL_OPEN_FNORMAL);
}
EXPORT(__DSYM(_loaddll),libc_dos_loaddll);
CRT_DOS void *LIBCCALL libc_dos_loaddll(char *file) {
 return libc_dos_xdlopen(file,DL_OPEN_FNORMAL);
}

DEFINE_INTERN_ALIAS(libc_unloaddll,libc_xdlclose);
EXPORT(_unloaddll,libc_unloaddll);

#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_getdllprocaddr,libc_xdlsym);
#else
CRT_DOS void *LIBCCALL
libc_getdllprocaddr(void *hnd, char const *symname, intptr_t UNUSED(ord)) {
 return libc_xdlsym(hnd,symname);
}
#endif
EXPORT(_getdllprocaddr,libc_getdllprocaddr);


/* GLibc Aliases */
EXPORT_STRONG(__libc_dlsym,libc_xdlsym);
EXPORT_STRONG(__libc_dlclose,libc_xdlclose);


DECL_END

#endif /* !GUARD_LIBS_LIBC_DL_C */
