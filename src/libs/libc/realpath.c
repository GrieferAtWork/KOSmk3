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
#ifndef GUARD_LIBS_LIBC_REALPATH_C
#define GUARD_LIBS_LIBC_REALPATH_C 1
#define _GNU_SOURCE 1

#include "libc.h"
#include "realpath.h"
#include "unistd.h"
#include "errno.h"
#include "system.h"
#include "malloc.h"
#include "widechar.h"
#include "unicode.h"
#include <unicode.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

/* System call interface. */

DECL_BEGIN

INTERN char *LIBCCALL libc_getcwd(char *buf, size_t size) { return libc_xfrealpathat(AT_FDCWD,libc_empty_string,AT_EMPTY_PATH,buf,size,REALPATH_FPATH); }
CRT_DOS char *LIBCCALL libc_dos_getcwd(char *buf, size_t size) { return libc_xfrealpathat(AT_FDCWD,libc_empty_string,AT_EMPTY_PATH,buf,size,REALPATH_FPATH|REALPATH_FDOSPATH); }
INTERN char *LIBCCALL libc_getwd(char *buf) { return libc_getcwd(buf,(size_t)-1); }
CRT_DOS_EXT char *LIBCCALL libc_dos_getwd(char *buf) { return libc_dos_getcwd(buf,(size_t)-1); }
INTERN char *LIBCCALL libc_get_current_dir_name(void) { return libc_getcwd(NULL,0); }
CRT_DOS_EXT char *LIBCCALL libc_dos_get_current_dir_name(void) { return libc_dos_getcwd(NULL,0); }

CRT_DOS_EXT ssize_t LIBCCALL
libc_dos_xfrealpathat2(fd_t dfd, char const *path, int flags,
                       char *buf, size_t bufsize, unsigned int type) {
 return libc_xfrealpathat2(dfd,path,flags|AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH);
}
INTERN char *LIBCCALL
libc_xfrealpathat(fd_t fd, char const *path, int flags,
                  char *buf, size_t bufsize, unsigned int type) {
 ssize_t reqsize; bool is_libc_buffer = false;
 if (!buf && bufsize && (is_libc_buffer = true,
      buf = (char *)libc_malloc(bufsize*sizeof(char))) == NULL) return NULL;
#if 1
 else if (!buf && !bufsize) {
  char *result;
  /* Allocate a small, initial buffer */
  bufsize = (260+1);
  if ((buf = (char *)libc_malloc(bufsize*sizeof(char))) == NULL) return NULL;
  reqsize = libc_xfrealpathat2(fd,path,flags,buf,bufsize,type);
  if ((ssize_t)reqsize == -1) goto err_buffer;
  if ((size_t)reqsize >= bufsize) goto do_dynamic;
  /* Free unused memory. */
  result = (char *)libc_realloc(buf,((size_t)reqsize+1));
  return likely(result) ? result : buf;
 }
#endif
 reqsize = libc_xfrealpathat2(fd,path,flags,buf,bufsize,type);
 if (reqsize == -1) {
  if (is_libc_buffer)
      goto err_buffer;
  return NULL;
 }
 if ((size_t)reqsize >= bufsize) {
  if (!buf) {
   /* Allocate a new buffer dynamically. */
do_dynamic:
   do {
    char *new_buf;
    bufsize = (size_t)reqsize;
    new_buf = (char *)libc_realloc(buf,(bufsize+1)*sizeof(char));
    if unlikely(!new_buf) {err_buffer: libc_free(buf); return NULL; }
    buf = new_buf;
    reqsize = libc_xfrealpathat2(fd,path,flags,buf,bufsize+1,type);
    if unlikely(reqsize == -1) goto err_buffer;
   } while ((size_t)reqsize != bufsize);
   return buf;
  }
  libc_seterrno(ERANGE);
  return NULL;
 } else if (!reqsize && !buf) {
  /* Must allocate an empty buffer... */
  buf = (char *)libc_malloc(1*sizeof(char));
  if (buf) buf[0] = '\0';
 }
 return buf;
}
CRT_DOS_EXT char *LIBCCALL libc_dos_xfrealpathat(fd_t dfd, char const *path, int flags, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat(dfd,path,flags|AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH); }
INTERN char *LIBCCALL libc_xrealpath(char const *path, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat(AT_FDCWD,path,0,buf,bufsize,type); }
CRT_DOS_EXT char *LIBCCALL libc_dos_xrealpath(char const *path, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat(AT_FDCWD,path,AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH); }
INTERN char *LIBCCALL libc_xfrealpath(fd_t fd, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat(AT_FDCWD,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type); }
CRT_DOS_EXT char *LIBCCALL libc_dos_xfrealpath(fd_t fd, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat(AT_FDCWD,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type|REALPATH_FDOSPATH); }
INTERN ssize_t LIBCCALL libc_xrealpath2(char const *path, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat2(AT_FDCWD,path,0,buf,bufsize,type); }
CRT_DOS_EXT ssize_t LIBCCALL libc_dos_xrealpath2(char const *path, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat2(AT_FDCWD,path,AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH); }
INTERN ssize_t LIBCCALL libc_xfrealpath2(fd_t fd, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat2(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type); }
CRT_DOS_EXT ssize_t LIBCCALL libc_dos_xfrealpath2(fd_t fd, char *buf, size_t bufsize, unsigned int type) { return libc_xfrealpathat2(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type|REALPATH_FDOSPATH); }

INTERN char *LIBCCALL libc_realpath(char const *name, char *resolved) { return libc_xrealpath(name,resolved,resolved ? PATH_MAX : 0,REALPATH_FPATH); }
CRT_DOS_EXT char *LIBCCALL libc_dos_realpath(char const *name, char *resolved) { return libc_xrealpath(name,resolved,resolved ? PATH_MAX : 0,REALPATH_FPATH|REALPATH_FDOSPATH); }
INTERN char *LIBCCALL libc_frealpath(fd_t fd, char *resolved, size_t bufsize) { return libc_xfrealpath(fd,resolved,bufsize,REALPATH_FPATH); }
CRT_DOS_EXT char *LIBCCALL libc_dos_frealpath(fd_t fd, char *resolved, size_t bufsize) { return libc_xfrealpath(fd,resolved,bufsize,REALPATH_FPATH|REALPATH_FDOSPATH); }
INTERN char *LIBCCALL libc_canonicalize_file_name(char const *name) { return libc_realpath(name,NULL); }
CRT_DOS_EXT char *LIBCCALL libc_dos_canonicalize_file_name(char const *name) { return libc_dos_realpath(name,NULL); }

CRT_KOS_DP char *LIBCCALL libc_xfdname(fd_t fd, int type, char *buf, size_t bufsize) { return libc_xfrealpathat(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type); }
CRT_DOS_EXT char *LIBCCALL libc_dos_xfdname(fd_t fd, int type, char *buf, size_t bufsize) { return libc_xfrealpathat(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type|REALPATH_FDOSPATH); }
CRT_KOS_DP ssize_t LIBCCALL libc_xfdname2(fd_t fd, int type, char *buf, size_t bufsize) { return libc_xfrealpathat2(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type); }
CRT_DOS_EXT ssize_t LIBCCALL libc_dos_xfdname2(fd_t fd, int type, char *buf, size_t bufsize) { return libc_xfrealpathat2(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type|REALPATH_FDOSPATH); }


EXPORT(__KSYM(getcwd),             libc_getcwd);
EXPORT(__DSYM(_getcwd),            libc_dos_getcwd);
EXPORT(__KSYM(getwd),              libc_getwd);
EXPORT(__DSYM(getwd),              libc_dos_getwd);
EXPORT(__KSYM(get_current_dir_name),libc_get_current_dir_name);
EXPORT(__DSYM(get_current_dir_name),libc_dos_get_current_dir_name);
EXPORT(__KSYM(xfdname),            libc_xfdname);
EXPORT(__DSYM(xfdname),            libc_dos_xfdname);
EXPORT(__KSYM(xfrealpath),         libc_xfrealpath);
EXPORT(__DSYM(xfrealpath),         libc_dos_xfrealpath);
EXPORT(__KSYM(xrealpath),          libc_xrealpath);
EXPORT(__DSYM(xrealpath),          libc_dos_xrealpath);
EXPORT(__KSYM(xfrealpathat),       libc_xfrealpathat);
EXPORT(__DSYM(xfrealpathat),       libc_dos_xfrealpathat);
EXPORT(__KSYM(xfdname2),           libc_xfdname2);
EXPORT(__DSYM(xfdname2),           libc_dos_xfdname2);
EXPORT(__KSYM(xfrealpath2),        libc_xfrealpath2);
EXPORT(__DSYM(xfrealpath2),        libc_dos_xfrealpath2);
EXPORT(__KSYM(xrealpath2),         libc_xrealpath2);
EXPORT(__DSYM(xrealpath2),         libc_dos_xrealpath2);
EXPORT(__DSYM(xfrealpathat2),      libc_dos_xfrealpathat2);
EXPORT(__KSYM(realpath),           libc_realpath);
EXPORT(__DSYM(realpath),           libc_dos_realpath);
EXPORT(__KSYM(frealpath),          libc_frealpath);
EXPORT(__DSYM(frealpath),          libc_dos_frealpath);
EXPORT(__KSYM(canonicalize_file_name),libc_canonicalize_file_name);
EXPORT(__DSYM(canonicalize_file_name),libc_dos_canonicalize_file_name);






EXPORT(Xxfrealpath2,libc_Xxfrealpath2);
CRT_EXCEPT size_t LIBCCALL
libc_Xxfrealpath2(fd_t fd, char *buf, size_t bufsize, unsigned int type) {
 return libc_Xxfrealpathat2(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type);
}
EXPORT(Xxrealpath2,libc_Xxrealpath2);
CRT_EXCEPT size_t LIBCCALL
libc_Xxrealpath2(char const *path, char *buf, size_t bufsize, unsigned int type) {
 return libc_Xxfrealpathat2(AT_FDCWD,path,0,buf,bufsize,type);
}
EXPORT(Xxfrealpath,libc_Xxfrealpath);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libc_Xxfrealpath(fd_t fd, char *buf, size_t bufsize, unsigned int type) {
 return libc_Xxfrealpathat(fd,libc_empty_string,AT_EMPTY_PATH,buf,bufsize,type);
}
EXPORT(Xxrealpath,libc_Xxrealpath);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libc_Xxrealpath(char const *path, char *buf, size_t bufsize, unsigned int type) {
 return libc_Xxfrealpathat(AT_FDCWD,path,0,buf,bufsize,type);
}

CRT_EXCEPT ATTR_NORETURN void LIBCCALL
libc_throw_buffer_too_small(size_t reqsize, size_t bufsize) {
 struct exception_info *info = libc_error_info();
 libc_memset(info->e_error.e_pointers,0,
             sizeof(info->e_error.e_pointers));
 info->e_error.e_code = E_BUFFER_TOO_SMALL;
 info->e_error.e_flag = ERR_FNORMAL;
 info->e_error.e_buffer_too_small.bs_reqsize = reqsize;
 info->e_error.e_buffer_too_small.bs_bufsize = bufsize;
 libc_error_throw_current();
 __builtin_unreachable();
}

EXPORT(Xxfrealpathat,libc_Xxfrealpathat);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libc_Xxfrealpathat(fd_t fd, char const *path, int flags,
                   char *buf_, size_t bufsize, unsigned int type) {
 char *EXCEPT_VAR buf = buf_;
 size_t COMPILER_IGNORE_UNINITIALIZED(reqsize);
 bool EXCEPT_VAR is_libc_buffer = false;
 if (!buf && bufsize)
      is_libc_buffer = true,
      buf = (char *)libc_Xmalloc(bufsize*sizeof(char));
#if 1
 else if (!buf && !bufsize) {
  char *result;
  /* Allocate a small, initial buffer */
  bufsize = (260+1);
  buf = (char *)libc_Xmalloc(bufsize*sizeof(char));
  TRY {
   reqsize = libc_Xxfrealpathat2(fd,path,flags,buf,bufsize,type);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   libc_free(buf);
   error_rethrow();
  }
  if ((size_t)reqsize >= bufsize)
       goto do_dynamic;
  /* Free unused memory. */
  result = (char *)libc_realloc(buf,((size_t)reqsize+1));
  return likely(result) ? result : buf;
 }
#endif
 TRY {
  reqsize = libc_Xxfrealpathat2(fd,path,flags,buf,bufsize,type);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  if (is_libc_buffer)
      libc_free(buf);
  error_rethrow();
 }
 if (reqsize >= bufsize) {
  if (!buf) {
   /* Allocate a new buffer dynamically. */
do_dynamic:
   do {
    bufsize = (size_t)reqsize;
    TRY {
     buf = (char *)libc_Xrealloc(buf,(bufsize+1)*sizeof(char));
     reqsize = Xsys_xfrealpathat(fd,path,flags,buf,bufsize+1,type);
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     libc_free(buf);
     error_rethrow();
    }
   } while ((size_t)reqsize != bufsize);
   return buf;
  }
  libc_throw_buffer_too_small(reqsize,bufsize);
 } else if (!reqsize && !buf) {
  /* Must allocate an empty buffer... */
  buf = (char *)libc_Xmalloc(1*sizeof(char));
  buf[0] = '\0';
 }
 return buf;
}

EXPORT(Xrealpath,libc_Xrealpath);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libc_Xrealpath(char const *name, char *resolved) {
 return libc_Xxrealpath(name,resolved,resolved ? PATH_MAX : 0,REALPATH_FPATH);
}
EXPORT(Xfrealpath,libc_Xfrealpath);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libc_Xfrealpath(fd_t fd, char *resolved, size_t bufsize) {
 return libc_Xxfrealpath(fd,resolved,bufsize,REALPATH_FPATH);
}
EXPORT(Xcanonicalize_file_name,libc_Xcanonicalize_file_name);
CRT_EXCEPT ATTR_MALLOC ATTR_RETNONNULL char *LIBCCALL
libc_Xcanonicalize_file_name(char const *name) {
 return libc_Xrealpath(name,NULL);
}

EXPORT(Xgetcwd,libc_Xgetcwd);
CRT_EXCEPT char *LIBCCALL
libc_Xgetcwd(char *buf, size_t size) {
 return libc_Xxfrealpathat(AT_FDCWD,libc_empty_string,AT_EMPTY_PATH,buf,size,REALPATH_FPATH);
}
EXPORT(Xget_current_dir_name,libc_Xget_current_dir_name);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libc_Xget_current_dir_name(void) {
 return libc_Xgetcwd(NULL,0);
}

EXPORT(Xgetdcwd,libd_Xgetdcwd);
CRT_EXCEPT ATTR_RETNONNULL char *LIBCCALL
libd_Xgetdcwd(int drive, char *buf, size_t size) {
 return libc_Xxfrealpathat(AT_FDDRIVE_CWD('A'+drive),libc_empty_string,
                           AT_EMPTY_PATH,buf,size,REALPATH_FPATH);
}



EXPORT(__SYMw16(xwfrealpath),libc_xw16frealpath);
CRT_WIDECHAR char16_t *LIBCCALL
libc_xw16frealpath(fd_t fd, char16_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw16frealpathat(fd,libc_empty_string16,AT_EMPTY_PATH,buf,bufsize,type);
}
EXPORT(__SYMw32(xwfrealpath),libc_xw32frealpath);
CRT_WIDECHAR char32_t *LIBCCALL
libc_xw32frealpath(fd_t fd, char32_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw32frealpathat(fd,libc_empty_string32,AT_EMPTY_PATH,buf,bufsize,type);
}
EXPORT(__KSYMw16(xwrealpath),libc_xw16realpath);
CRT_WIDECHAR char16_t *LIBCCALL
libc_xw16realpath(char16_t const *path, char16_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw16frealpathat(AT_FDCWD,path,0,buf,bufsize,type);
}
EXPORT(__KSYMw32(xwrealpath),libc_xw32realpath);
CRT_WIDECHAR char32_t *LIBCCALL
libc_xw32realpath(char32_t const *path, char32_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw32frealpathat(AT_FDCWD,path,0,buf,bufsize,type);
}
EXPORT(__DSYMw16(xwrealpath),libc_dos_xw16realpath);
CRT_WIDECHAR char16_t *LIBCCALL
libc_dos_xw16realpath(char16_t const *path, char16_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw16frealpathat(AT_FDCWD,path,AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH);
}
EXPORT(__DSYMw32(xwrealpath),libc_dos_xw32realpath);
CRT_WIDECHAR char32_t *LIBCCALL
libc_dos_xw32realpath(char32_t const *path, char32_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw32frealpathat(AT_FDCWD,path,AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH);
}
EXPORT(__DSYMw16(xwfrealpathat),libc_dos_xw16frealpathat);
CRT_WIDECHAR char16_t *LIBCCALL
libc_dos_xw16frealpathat(fd_t dfd, char16_t const *path, int flags, char16_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw16frealpathat(dfd,path,flags|AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH);
}
EXPORT(__DSYMw32(xwfrealpathat),libc_dos_xw32frealpathat);
CRT_WIDECHAR char32_t *LIBCCALL
libc_dos_xw32frealpathat(fd_t dfd, char32_t const *path, int flags, char32_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw32frealpathat(dfd,path,flags|AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH);
}

EXPORT(__SYMw16(xwfrealpath2),libc_xw16frealpath2);
CRT_WIDECHAR ssize_t LIBCCALL
libc_xw16frealpath2(fd_t fd, char16_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw16frealpathat2(fd,libc_empty_string16,AT_EMPTY_PATH,buf,bufsize,type);
}
EXPORT(__SYMw32(xwfrealpath2),libc_xw32frealpath2);
CRT_WIDECHAR ssize_t LIBCCALL
libc_xw32frealpath2(fd_t fd, char32_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw32frealpathat2(fd,libc_empty_string32,AT_EMPTY_PATH,buf,bufsize,type);
}
EXPORT(__KSYMw16(xwrealpath2),libc_xw16realpath2);
CRT_WIDECHAR ssize_t LIBCCALL
libc_xw16realpath2(char16_t const *path, char16_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw16frealpathat2(AT_FDCWD,path,0,buf,bufsize,type);
}
EXPORT(__KSYMw32(xwrealpath2),libc_xw32realpath2);
CRT_WIDECHAR ssize_t LIBCCALL
libc_xw32realpath2(char32_t const *path, char32_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw32frealpathat2(AT_FDCWD,path,0,buf,bufsize,type);
}
EXPORT(__DSYMw16(xwrealpath2),libc_dos_xw16realpath2);
CRT_WIDECHAR ssize_t LIBCCALL
libc_dos_xw16realpath2(char16_t const *path, char16_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw16frealpathat2(AT_FDCWD,path,AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH);
}
EXPORT(__DSYMw32(xwrealpath2),libc_dos_xw32realpath2);
CRT_WIDECHAR ssize_t LIBCCALL
libc_dos_xw32realpath2(char32_t const *path, char32_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw32frealpathat2(AT_FDCWD,path,AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH);
}
EXPORT(__DSYMw16(xwfrealpathat2),libc_dos_xw16frealpathat2);
CRT_WIDECHAR ssize_t LIBCCALL
libc_dos_xw16frealpathat2(fd_t dfd, char16_t const *path, int flags, char16_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw16frealpathat2(dfd,path,flags|AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH);
}
EXPORT(__DSYMw32(xwfrealpathat2),libc_dos_xw32frealpathat2);
CRT_WIDECHAR ssize_t LIBCCALL
libc_dos_xw32frealpathat2(fd_t dfd, char32_t const *path, int flags, char32_t *buf, size_t bufsize, unsigned int type) {
 return libc_xw32frealpathat2(dfd,path,flags|AT_DOSPATH,buf,bufsize,type|REALPATH_FDOSPATH);
}


EXPORT(__SYMw16(Xxwfrealpath),libc_Xxw16frealpath);
CRT_WIDECHAR ATTR_RETNONNULL char16_t *LIBCCALL
libc_Xxw16frealpath(fd_t fd, char16_t *buf, size_t bufsize, unsigned int type) {
 return libc_Xxw16frealpathat(fd,libc_empty_string16,AT_EMPTY_PATH,buf,bufsize,type);
}
EXPORT(__SYMw32(Xxwfrealpath),libc_Xxw32frealpath);
CRT_WIDECHAR ATTR_RETNONNULL char32_t *LIBCCALL
libc_Xxw32frealpath(fd_t fd, char32_t *buf, size_t bufsize, unsigned int type) {
 return libc_Xxw32frealpathat(fd,libc_empty_string32,AT_EMPTY_PATH,buf,bufsize,type);
}
EXPORT(__SYMw16(Xxwrealpath),libc_Xxw16realpath);
CRT_WIDECHAR ATTR_RETNONNULL char16_t *LIBCCALL
libc_Xxw16realpath(char16_t const *path, char16_t *buf, size_t bufsize, unsigned int type) {
 return libc_Xxw16frealpathat(AT_FDCWD,path,0,buf,bufsize,type);
}
EXPORT(__SYMw32(Xxwrealpath),libc_Xxw32realpath);
CRT_WIDECHAR ATTR_RETNONNULL char32_t *LIBCCALL
libc_Xxw32realpath(char32_t const *path, char32_t *buf, size_t bufsize, unsigned int type) {
 return libc_Xxw32frealpathat(AT_FDCWD,path,0,buf,bufsize,type);
}
EXPORT(__SYMw16(Xxwfrealpath2),libc_Xxw16frealpath2);
CRT_WIDECHAR size_t LIBCCALL
libc_Xxw16frealpath2(fd_t fd, char16_t *buf, size_t bufsize, unsigned int type) {
 return libc_Xxw16frealpathat2(fd,libc_empty_string16,AT_EMPTY_PATH,buf,bufsize,type);
}
EXPORT(__SYMw32(Xxwfrealpath2),libc_Xxw32frealpath2);
CRT_WIDECHAR size_t LIBCCALL
libc_Xxw32frealpath2(fd_t fd, char32_t *buf, size_t bufsize, unsigned int type) {
 return libc_Xxw32frealpathat2(fd,libc_empty_string32,AT_EMPTY_PATH,buf,bufsize,type);
}
EXPORT(__SYMw16(Xxwrealpath2),libc_Xxw16realpath2);
CRT_WIDECHAR size_t LIBCCALL
libc_Xxw16realpath2(char16_t const *path, char16_t *buf, size_t bufsize, unsigned int type) {
 return libc_Xxw16frealpathat2(AT_FDCWD,path,0,buf,bufsize,type);
}
EXPORT(__SYMw32(Xxwrealpath2),libc_Xxw32realpath2);
CRT_WIDECHAR size_t LIBCCALL
libc_Xxw32realpath2(char32_t const *path, char32_t *buf, size_t bufsize, unsigned int type) {
 return libc_Xxw32frealpathat2(AT_FDCWD,path,0,buf,bufsize,type);
}

EXPORT(__KSYMw16(xwfrealpathat),libc_xw16frealpathat);
CRT_WIDECHAR char16_t *LIBCCALL
libc_xw16frealpathat(fd_t dfd, char16_t const *path, int flags,
                     char16_t *buf, size_t bufsize, unsigned int type) {
 LIBC_TRY {
  return libc_Xxw16frealpathat(dfd,path,flags,buf,bufsize,type);
 } LIBC_EXCEPT (libc_except_errno()) {
 }
 return NULL;
}

EXPORT(__KSYMw32(xwfrealpathat),libc_xw32frealpathat);
CRT_WIDECHAR char32_t *LIBCCALL
libc_xw32frealpathat(fd_t dfd, char32_t const *path, int flags,
                     char32_t *buf, size_t bufsize, unsigned int type) {
 LIBC_TRY {
  return libc_Xxw32frealpathat(dfd,path,flags,buf,bufsize,type);
 } LIBC_EXCEPT (libc_except_errno()) {
 }
 return NULL;
}

EXPORT(__KSYMw16(xwfrealpathat2),libc_xw16frealpathat2);
CRT_WIDECHAR ssize_t LIBCCALL
libc_xw16frealpathat2(fd_t dfd, char16_t const *path, int flags,
                      char16_t *buf, size_t bufsize, unsigned int type) {
 LIBC_TRY {
  return libc_Xxw16frealpathat2(dfd,path,flags,buf,bufsize,type);
 } LIBC_EXCEPT (libc_except_errno()) {
 }
 return -1;
}

EXPORT(__KSYMw32(xwfrealpathat2),libc_xw32frealpathat2);
CRT_WIDECHAR ssize_t LIBCCALL
libc_xw32frealpathat2(fd_t dfd, char32_t const *path, int flags,
                      char32_t *buf, size_t bufsize, unsigned int type) {
 LIBC_TRY {
  return libc_Xxw32frealpathat2(dfd,path,flags,buf,bufsize,type);
 } LIBC_EXCEPT (libc_except_errno()) {
 }
 return -1;
}


EXPORT(__SYMw16(Xxwfrealpathat),libc_Xxw16frealpathat);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL char16_t *LIBCCALL
libc_Xxw16frealpathat(fd_t dfd, char16_t const *path, int flags,
                      char16_t *buf_, size_t bufsize, unsigned int type) {
 char16_t *EXCEPT_VAR buf = buf_;
 size_t COMPILER_IGNORE_UNINITIALIZED(reqsize);
 bool EXCEPT_VAR is_libc_buffer = false;
 if (!buf && bufsize)
      is_libc_buffer = true,
      buf = (char16_t *)libc_Xmalloc(bufsize*sizeof(char16_t));
#if 1
 else if (!buf && !bufsize) {
  char16_t *result;
  /* Allocate a small, initial buffer */
  bufsize = (260+1);
  buf = (char16_t *)libc_Xmalloc(bufsize*sizeof(char16_t));
  TRY {
   reqsize = libc_Xxw16frealpathat2(dfd,path,flags,buf,bufsize,type);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   libc_free(buf);
   error_rethrow();
  }
  if ((size_t)reqsize >= bufsize)
       goto do_dynamic;
  /* Free unused memory. */
  result = (char16_t *)libc_realloc(buf,((size_t)reqsize+1));
  return likely(result) ? result : buf;
 }
#endif
 TRY {
  reqsize = libc_Xxw16frealpathat2(dfd,path,flags,buf,bufsize,type);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  if (is_libc_buffer)
      libc_free(buf);
  error_rethrow();
 }
 if (reqsize >= bufsize) {
  if (!buf) {
   /* Allocate a new buffer dynamically. */
do_dynamic:
   do {
    bufsize = (size_t)reqsize;
    TRY {
     buf = (char16_t *)libc_Xrealloc(buf,(bufsize+1)*sizeof(char16_t));
     reqsize = libc_Xxw16frealpathat2(dfd,path,flags,buf,bufsize+1,type);
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     libc_free(buf);
     error_rethrow();
    }
   } while ((size_t)reqsize != bufsize);
   return buf;
  }
  libc_throw_buffer_too_small(reqsize,bufsize);
 } else if (!reqsize && !buf) {
  /* Must allocate an empty buffer... */
  buf = (char16_t *)libc_Xmalloc(1*sizeof(char16_t));
  buf[0] = '\0';
 }
 return buf;
}
EXPORT(__SYMw32(Xxwfrealpathat),libc_Xxw32frealpathat);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL char32_t *LIBCCALL
libc_Xxw32frealpathat(fd_t dfd, char32_t const *path, int flags,
                      char32_t *buf_, size_t bufsize, unsigned int type) {
 char32_t *EXCEPT_VAR buf = buf_;
 size_t COMPILER_IGNORE_UNINITIALIZED(reqsize);
 bool EXCEPT_VAR is_libc_buffer = false;
 if (!buf && bufsize)
      is_libc_buffer = true,
      buf = (char32_t *)libc_Xmalloc(bufsize*sizeof(char32_t));
#if 1
 else if (!buf && !bufsize) {
  char32_t *result;
  /* Allocate a small, initial buffer */
  bufsize = (260+1);
  buf = (char32_t *)libc_Xmalloc(bufsize*sizeof(char32_t));
  TRY {
   reqsize = libc_Xxw32frealpathat2(dfd,path,flags,buf,bufsize,type);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   libc_free(buf);
   error_rethrow();
  }
  if ((size_t)reqsize >= bufsize)
       goto do_dynamic;
  /* Free unused memory. */
  result = (char32_t *)libc_realloc(buf,((size_t)reqsize+1));
  return likely(result) ? result : buf;
 }
#endif
 TRY {
  reqsize = libc_Xxw32frealpathat2(dfd,path,flags,buf,bufsize,type);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  if (is_libc_buffer)
      libc_free(buf);
  error_rethrow();
 }
 if (reqsize >= bufsize) {
  if (!buf) {
   /* Allocate a new buffer dynamically. */
do_dynamic:
   do {
    bufsize = (size_t)reqsize;
    TRY {
     buf = (char32_t *)libc_Xrealloc(buf,(bufsize+1)*sizeof(char32_t));
     reqsize = libc_Xxw32frealpathat2(dfd,path,flags,buf,bufsize+1,type);
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     libc_free(buf);
     error_rethrow();
    }
   } while ((size_t)reqsize != bufsize);
   return buf;
  }
  libc_throw_buffer_too_small(reqsize,bufsize);
 } else if (!reqsize && !buf) {
  /* Must allocate an empty buffer... */
  buf = (char32_t *)libc_Xmalloc(1*sizeof(char32_t));
  buf[0] = '\0';
 }
 return buf;
}

EXPORT(__SYMw16(Xxwfrealpathat2),libc_Xxw16frealpathat2);
CRT_WIDECHAR_EXCEPT size_t LIBCCALL
libc_Xxw16frealpathat2(fd_t dfd, char16_t const *path, int flags,
                       char16_t *buf, size_t bufsize, unsigned int type) {
 char tempbuf[UTF_STACK_BUFFER_SIZE];
 char pathbuf[UTF_STACK_BUFFER_SIZE];
 char *EXCEPT_VAR temp = NULL;
 char *EXCEPT_VAR path8;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 path8 = libc_Xloadutf16(pathbuf,path);
 LIBC_TRY {
  mbstate_t state = MBSTATE_INIT;
  LIBC_TRY {
   temp = libc_Xxfrealpathat(dfd,path8,flags,tempbuf,sizeof(tempbuf),type);
  } LIBC_CATCH_HANDLED (E_BUFFER_TOO_SMALL) {
   temp = libc_Xxfrealpathat(dfd,path8,flags,NULL,0,type);
  }
  result = libc_Xutf8to16(temp,(size_t)-1,buf,bufsize,&state,
                          UNICODE_F_STOPONNUL|
                          UNICODE_F_SETERRNO);
 } LIBC_FINALLY {
  if (temp != tempbuf)
      libc_free(temp);
  libc_freeutf(pathbuf,path8);
 }
 return result;
}

EXPORT(__SYMw32(Xxwfrealpathat2),libc_Xxw32frealpathat2);
CRT_WIDECHAR_EXCEPT size_t LIBCCALL
libc_Xxw32frealpathat2(fd_t dfd, char32_t const *path, int flags,
                       char32_t *buf, size_t bufsize, unsigned int type) {
 char tempbuf[UTF_STACK_BUFFER_SIZE];
 char pathbuf[UTF_STACK_BUFFER_SIZE];
 char *EXCEPT_VAR temp = NULL;
 char *EXCEPT_VAR path8;
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 path8 = libc_Xloadutf32(pathbuf,path);
 LIBC_TRY {
  mbstate_t state = MBSTATE_INIT;
  LIBC_TRY {
   temp = libc_Xxfrealpathat(dfd,path8,flags,tempbuf,sizeof(tempbuf),type);
  } LIBC_CATCH_HANDLED (E_BUFFER_TOO_SMALL) {
   temp = libc_Xxfrealpathat(dfd,path8,flags,NULL,0,type);
  }
  result = libc_Xutf8to32(temp,(size_t)-1,buf,bufsize,&state,
                          UNICODE_F_STOPONNUL|
                          UNICODE_F_SETERRNO);
 } LIBC_FINALLY {
  if (temp != tempbuf)
      libc_free(temp);
  libc_freeutf(pathbuf,path8);
 }
 return result;
}


EXPORT(__KSYMw16(wgetcwd),libc_w16getcwd);
CRT_WIDECHAR char16_t *LIBCCALL
libc_w16getcwd(char16_t *buf, size_t size) {
 return libc_xw16frealpathat(AT_FDCWD,libc_empty_string16,
                             AT_EMPTY_PATH,buf,size,REALPATH_FPATH);
}

EXPORT(__KSYMw32(wgetcwd),libc_w32getcwd);
CRT_WIDECHAR char32_t *LIBCCALL
libc_w32getcwd(char32_t *buf, size_t size) {
 return libc_xw32frealpathat(AT_FDCWD,libc_empty_string32,
                             AT_EMPTY_PATH,buf,size,REALPATH_FPATH);
}

EXPORT(__DSYMw16(_wgetcwd),libc_dos_w16getcwd);
CRT_WIDECHAR char16_t *LIBCCALL
libc_dos_w16getcwd(char16_t *buf, size_t size) {
 return libc_xw16frealpathat(AT_FDCWD,libc_empty_string16,
                             AT_EMPTY_PATH,buf,size,
                             REALPATH_FPATH|REALPATH_FDOSPATH);
}

EXPORT(__DSYMw32(wgetcwd),libc_dos_w32getcwd);
CRT_WIDECHAR char32_t *LIBCCALL
libc_dos_w32getcwd(char32_t *buf, size_t size) {
 return libc_dos_xw32frealpathat(AT_FDCWD,libc_empty_string32,
                                 AT_EMPTY_PATH,buf,size,
                                 REALPATH_FPATH|REALPATH_FDOSPATH);
}

EXPORT(__KSYMw16(wgetdcwd),libd_w16getdcwd);
CRT_WIDECHAR char16_t *LIBCCALL
libd_w16getdcwd(int drive, char16_t *buf, size_t size) {
 return libc_xw16frealpathat(AT_FDDRIVE_CWD('A'+drive),
                             libc_empty_string16,AT_EMPTY_PATH,
                             buf,size,REALPATH_FPATH);
}

EXPORT(__KSYMw32(wgetdcwd),libd_w32getdcwd);
CRT_WIDECHAR char32_t *LIBCCALL
libd_w32getdcwd(int drive, char32_t *buf, size_t size) {
 return libc_xw32frealpathat(AT_FDDRIVE_CWD('A'+drive),
                             libc_empty_string32,AT_EMPTY_PATH,
                             buf,size,REALPATH_FPATH);
}

EXPORT(__DSYMw16(_wgetdcwd),libd_dos_w16getdcwd);
CRT_WIDECHAR char16_t *LIBCCALL
libd_dos_w16getdcwd(int drive, char16_t *buf, size_t size) {
 return libc_xw16frealpathat(AT_FDDRIVE_CWD('A'+drive),
                             libc_empty_string16,AT_EMPTY_PATH,buf,size,
                             REALPATH_FPATH|REALPATH_FDOSPATH);
}

EXPORT(__DSYMw32(wgetdcwd),libd_dos_w32getdcwd);
CRT_WIDECHAR char32_t *LIBCCALL
libd_dos_w32getdcwd(int drive, char32_t *buf, size_t size) {
 return libc_xw32frealpathat(AT_FDDRIVE_CWD('A'+drive),
                             libc_empty_string32,AT_EMPTY_PATH,buf,size,
                             REALPATH_FPATH|REALPATH_FDOSPATH);
}

#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
EXPORT(__DSYMw16(_wgetcwd_dbg),libc_dos_w16getcwd);
EXPORT(__DSYMw16(_wgetdcwd_dbg),libd_dos_w16getdcwd);
#else
EXPORT(__DSYMw16(_wgetcwd_dbg),libc_dos_w16getcwd_dbg);
CRT_DOS char16_t *LIBCCALL
libc_dos_w16getcwd_dbg(char16_t *buf, size_t size,
                       int UNUSED(blocktype),
                       char const *UNUSED(filename),
                       int UNUSED(lno)) {
 return libc_dos_w16getcwd(buf,size);
}
EXPORT(__DSYMw16(_wgetdcwd_dbg),libc_dos_w16getdcwd_dbg);
CRT_DOS char16_t *LIBCCALL
libc_dos_w16getdcwd_dbg(int drive, char16_t *buf, size_t size,
                        int UNUSED(blocktype),
                        char const *UNUSED(filename),
                        int UNUSED(lno)) {
 return libd_dos_w16getdcwd(drive,buf,size);
}
#endif




EXPORT(__KSYMw16(wget_current_dir_name),libc_w16get_current_dir_name);
CRT_WIDECHAR ATTR_MALLOC char16_t *LIBCCALL
libc_w16get_current_dir_name(void) {
 return libc_w16getcwd(NULL,0);
}

EXPORT(__KSYMw32(wget_current_dir_name),libc_w32get_current_dir_name);
CRT_WIDECHAR ATTR_MALLOC char32_t *LIBCCALL
libc_w32get_current_dir_name(void) {
 return libc_w32getcwd(NULL,0);
}

EXPORT(__DSYMw16(wget_current_dir_name),libc_dos_w16get_current_dir_name);
CRT_WIDECHAR ATTR_MALLOC char16_t *LIBCCALL
libc_dos_w16get_current_dir_name(void) {
 return libc_dos_w16getcwd(NULL,0);
}

EXPORT(__DSYMw32(wget_current_dir_name),libc_dos_w32get_current_dir_name);
CRT_WIDECHAR ATTR_MALLOC char32_t *LIBCCALL
libc_dos_w32get_current_dir_name(void) {
 return libc_dos_w32getcwd(NULL,0);
}

EXPORT(__SYMw16(Xwgetcwd),libc_Xw16getcwd);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL char16_t *LIBCCALL
libc_Xw16getcwd(char16_t *buf, size_t size) {
 return libc_Xxw16frealpathat(AT_FDCWD,libc_empty_string16,
                              AT_EMPTY_PATH,buf,size,REALPATH_FPATH);
}
EXPORT(__SYMw32(Xwgetcwd),libc_Xw32getcwd);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL char32_t *LIBCCALL
libc_Xw32getcwd(char32_t *buf, size_t size) {
 return libc_Xxw32frealpathat(AT_FDCWD,libc_empty_string32,
                              AT_EMPTY_PATH,buf,size,REALPATH_FPATH);
}

EXPORT(__SYMw16(Xwgetdcwd),libc_Xw16getdcwd);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL char16_t *LIBCCALL
libc_Xw16getdcwd(int drive, char16_t *buf, size_t size) {
 return libc_Xxw16frealpathat(AT_FDDRIVE_CWD('A'+drive),
                              libc_empty_string16,AT_EMPTY_PATH,
                              buf,size,REALPATH_FPATH);
}
EXPORT(__SYMw32(Xwgetdcwd),libc_Xw32getdcwd);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL char32_t *LIBCCALL
libc_Xw32getdcwd(int drive, char32_t *buf, size_t size) {
 return libc_Xxw32frealpathat(AT_FDDRIVE_CWD('A'+drive),
                              libc_empty_string32,AT_EMPTY_PATH,
                              buf,size,REALPATH_FPATH);
}

EXPORT(__SYMw16(Xwget_current_dir_name),libc_Xw16get_current_dir_name);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL ATTR_MALLOC
char16_t *LIBCCALL libc_Xw16get_current_dir_name(void) {
 return libc_Xw16getcwd(NULL,0);
}

EXPORT(__SYMw32(Xwget_current_dir_name),libc_Xw32get_current_dir_name);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL ATTR_MALLOC
char32_t *LIBCCALL libc_Xw32get_current_dir_name(void) {
 return libc_Xw32getcwd(NULL,0);
}

EXPORT(__KSYMw16(wrealpath),libc_w16realpath);
CRT_WIDECHAR char16_t *LIBCCALL
libc_w16realpath(char16_t const *name, char16_t *resolved) {
 return libc_xw16realpath(name,resolved,resolved ? PATH_MAX : 0,REALPATH_FPATH);
}

EXPORT(__KSYMw32(wrealpath),libc_w16realpath);
CRT_WIDECHAR char32_t *LIBCCALL
libc_w32realpath(char32_t const *name, char32_t *resolved) {
 return libc_xw32realpath(name,resolved,resolved ? PATH_MAX : 0,REALPATH_FPATH);
}

EXPORT(__DSYMw16(wrealpath),libc_dos_w16realpath);
CRT_WIDECHAR char16_t *LIBCCALL
libc_dos_w16realpath(char16_t const *name, char16_t *resolved) {
 return libc_xw16frealpathat(AT_FDCWD,name,AT_DOSPATH,resolved,resolved ? PATH_MAX : 0,
                             REALPATH_FPATH|REALPATH_FDOSPATH);
}

EXPORT(__DSYMw32(wrealpath),libc_dos_w32realpath);
CRT_WIDECHAR char32_t *LIBCCALL
libc_dos_w32realpath(char32_t const *name, char32_t *resolved) {
 return libc_xw32frealpathat(AT_FDCWD,name,AT_DOSPATH,resolved,resolved ? PATH_MAX : 0,
                             REALPATH_FPATH|REALPATH_FDOSPATH);
}

EXPORT(__KSYMw16(wfrealpath),libc_w16frealpath);
CRT_WIDECHAR char16_t *LIBCCALL
libc_w16frealpath(fd_t fd, char16_t *resolved, size_t bufsize) {
 return libc_xw16frealpathat(fd,libc_empty_string16,AT_EMPTY_PATH,
                             resolved,resolved ? PATH_MAX : 0,
                             REALPATH_FPATH);
}
EXPORT(__KSYMw32(wfrealpath),libc_w32frealpath);
CRT_WIDECHAR char32_t *LIBCCALL
libc_w32frealpath(fd_t fd, char32_t *resolved, size_t bufsize) {
 return libc_xw32frealpathat(fd,libc_empty_string32,AT_EMPTY_PATH,
                             resolved,resolved ? PATH_MAX : 0,
                             REALPATH_FPATH);
}
EXPORT(__DSYMw16(wfrealpath),libc_dos_w16frealpath);
CRT_WIDECHAR char16_t *LIBCCALL
libc_dos_w16frealpath(fd_t fd, char16_t *resolved, size_t bufsize) {
 return libc_xw16frealpathat(fd,libc_empty_string16,AT_EMPTY_PATH,
                             resolved,resolved ? PATH_MAX : 0,
                             REALPATH_FPATH|REALPATH_FDOSPATH);
}
EXPORT(__DSYMw32(wfrealpath),libc_dos_w32frealpath);
CRT_WIDECHAR char32_t *LIBCCALL
libc_dos_w32frealpath(fd_t fd, char32_t *resolved, size_t bufsize) {
 return libc_xw32frealpathat(fd,libc_empty_string32,AT_EMPTY_PATH,
                             resolved,resolved ? PATH_MAX : 0,
                             REALPATH_FPATH|REALPATH_FDOSPATH);
}

EXPORT(__KSYMw16(wcanonicalize_file_name),libc_w16canonicalize_file_name);
CRT_WIDECHAR ATTR_MALLOC char16_t *LIBCCALL
libc_w16canonicalize_file_name(char16_t const *name) {
 return libc_w16realpath(name,NULL);
}
EXPORT(__KSYMw32(wcanonicalize_file_name),libc_w32canonicalize_file_name);
CRT_WIDECHAR ATTR_MALLOC char32_t *LIBCCALL
libc_w32canonicalize_file_name(char32_t const *name) {
 return libc_w32realpath(name,NULL);
}
EXPORT(__DSYMw16(wcanonicalize_file_name),libc_dos_w16canonicalize_file_name);
CRT_WIDECHAR ATTR_MALLOC char16_t *LIBCCALL
libc_dos_w16canonicalize_file_name(char16_t const *name) {
 return libc_dos_w16realpath(name,NULL);
}
EXPORT(__DSYMw32(wcanonicalize_file_name),libc_dos_w32canonicalize_file_name);
CRT_WIDECHAR ATTR_MALLOC char32_t *LIBCCALL
libc_dos_w32canonicalize_file_name(char32_t const *name) {
 return libc_dos_w32realpath(name,NULL);
}

EXPORT(__SYMw16(Xwrealpath),libc_Xw16realpath);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL char16_t *LIBCCALL
libc_Xw16realpath(char16_t const *name, char16_t *resolved) {
 return libc_Xxw16realpath(name,resolved,resolved ? PATH_MAX : 0,REALPATH_FPATH);
}

EXPORT(__SYMw32(Xwrealpath),libc_Xw32realpath);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL char32_t *LIBCCALL
libc_Xw32realpath(char32_t const *name, char32_t *resolved) {
 return libc_Xxw32realpath(name,resolved,resolved ? PATH_MAX : 0,REALPATH_FPATH);
}

EXPORT(__SYMw16(Xwfrealpath),libc_Xw16frealpath);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL char16_t *LIBCCALL
libc_Xw16frealpath(fd_t fd, char16_t *resolved, size_t bufsize) {
 return libc_Xxw16frealpathat(fd,libc_empty_string16,AT_EMPTY_PATH,
                              resolved,resolved ? PATH_MAX : 0,
                              REALPATH_FPATH);
}

EXPORT(__SYMw32(Xwfrealpath),libc_Xw32frealpath);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL char32_t *LIBCCALL
libc_Xw32frealpath(fd_t fd, char32_t *resolved, size_t bufsize) {
 return libc_Xxw32frealpathat(fd,libc_empty_string32,AT_EMPTY_PATH,
                              resolved,resolved ? PATH_MAX : 0,
                              REALPATH_FPATH);
}

EXPORT(__SYMw16(Xwcanonicalize_file_name),libc_Xw16canonicalize_file_name);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL ATTR_MALLOC char16_t *
LIBCCALL libc_Xw16canonicalize_file_name(char16_t const *name) {
 return libc_Xw16realpath(name,NULL);
}

EXPORT(__SYMw32(Xwcanonicalize_file_name),libc_Xw32canonicalize_file_name);
CRT_WIDECHAR_EXCEPT ATTR_RETNONNULL ATTR_MALLOC char32_t *
LIBCCALL libc_Xw32canonicalize_file_name(char32_t const *name) {
 return libc_Xw32realpath(name,NULL);
}

CRT_DOS_NATIVE char *LIBCCALL libd_getdcwd(int drive, char *buf, size_t size) { return libc_xfdname(AT_FDDRIVE_CWD('A'+drive),REALPATH_FPATH,buf,size); }
CRT_DOS char *LIBCCALL libd_dos_getdcwd(int drive, char *buf, size_t size) { return libc_xfdname(AT_FDDRIVE_CWD('A'+drive),REALPATH_FPATH|REALPATH_FDOSPATH,buf,size); }
CRT_DOS_NATIVE int LIBCCALL libd_chdrive(int drive) { char temp[3] = "?:"; temp[0] = 'A'+drive; return libc_dos_chdir(temp); }
CRT_DOS_NATIVE int LIBCCALL libd_getdrive(void) { char buf[1]; return libc_xfdname2(AT_FDCWD,REALPATH_FDRIVE,buf,1) < 0 ? -1 : (buf[0]-'A'); }

#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
EXPORT(__DSYMw16(_getcwd_dbg),libc_dos_getcwd);
EXPORT(__DSYMw16(_getdcwd_dbg),libd_dos_getdcwd);
#else
EXPORT(__DSYMw16(_getcwd_dbg),libc_dos_getcwd_dbg);
CRT_DOS char *LIBCCALL
libc_dos_getcwd_dbg(char *buf, size_t size,
                    int UNUSED(blocktype),
                    char const *UNUSED(filename),
                    int UNUSED(lno)) {
 return libc_dos_getcwd(buf,size);
}
EXPORT(__DSYMw16(_getdcwd_dbg),libd_dos_getdcwd_dbg);
CRT_DOS char *LIBCCALL
libd_dos_getdcwd_dbg(int drive, char *buf, size_t size,
                     int UNUSED(blocktype),
                     char const *UNUSED(filename),
                     int UNUSED(lno)) {
 return libc_dos_getdcwd(drive,buf,size);
}
#endif

EXPORT(__KSYM(chdrive),            libd_chdrive);
EXPORT(__DSYM(_chdrive),           libd_chdrive);
EXPORT(__KSYM(getdrive),           libd_getdrive);
EXPORT(__DSYM(_getdrive),          libd_getdrive);
EXPORT(__KSYM(getdcwd),            libd_getdcwd);
EXPORT(__DSYM(_getdcwd),           libd_dos_getdcwd);


EXPORT(Xchdrive,libd_Xchdrive);
CRT_EXCEPT void LIBCCALL libd_Xchdrive(int drive) {
 char temp[3] = "?:";
 temp[0] = 'A'+drive;
 libc_Xfchdirat(AT_FDCWD,temp,AT_DOSPATH);
}
EXPORT(Xgetdrive,libd_Xgetdrive);
CRT_EXCEPT int LIBCCALL libd_Xgetdrive(void) {
 char buf[1];
 if (!libc_Xxfrealpath2(AT_FDCWD,buf,1,REALPATH_FDRIVE))
      error_throw(E_NO_DATA); /* Shouldn't happen... */
 return buf[0]-'A';
}

DECL_END

#endif /* !GUARD_LIBS_LIBC_REALPATH_C */
