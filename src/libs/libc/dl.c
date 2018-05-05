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
#include "malloc.h"
#include <kos/dl.h>
#include <errno.h>
#include <except.h>
#include <unistd.h>

DECL_BEGIN

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


/* Extended DL APIs */
EXPORT(xunwind,libc_xunwind);
CRT_KOS int LIBCCALL
libc_xunwind(struct cpu_context *__restrict ccontext,
             struct fpu_context *fcontext,
             sigset_t *signal_set) {
 return FORWARD_SYSTEM_ERROR(sys_xunwind(ccontext,fcontext,signal_set,sizeof(sigset_t)));
}

EXPORT(Xxunwind,libc_Xxunwind);
CRT_KOS bool LIBCCALL
libc_Xxunwind(struct cpu_context *__restrict ccontext,
              struct fpu_context *fcontext, sigset_t *signal_set) {
 return Xsys_xunwind(ccontext,fcontext,signal_set,sizeof(sigset_t)) == -EOK;
}


EXPORT(__KSYM(xdlpath),libc_xdlpath);
CRT_KOS char *LIBCCALL
libc_xdlpath(void *handle, char *buf,
             size_t bufsize, unsigned int type) {
 ssize_t reqsize; bool is_libc_buffer = false;
 if (!buf && bufsize && (is_libc_buffer = true,
      buf = (char *)libc_malloc(bufsize*sizeof(char))) == NULL) return NULL;
#if 1
 else if (!buf && !bufsize) {
  char *result;
  /* Allocate a small, initial buffer */
  bufsize = (260+1);
  if ((buf = (char *)libc_malloc(bufsize*sizeof(char))) == NULL) return NULL;
  reqsize = libc_xdlpath2(handle,buf,bufsize,type);
  if ((ssize_t)reqsize == -1) goto err_buffer;
  if ((size_t)reqsize >= bufsize) goto do_dynamic;
  /* Free unused memory. */
  result = (char *)libc_realloc(buf,((size_t)reqsize+1));
  return likely(result) ? result : buf;
 }
#endif
 reqsize = libc_xdlpath2(handle,buf,bufsize,type);
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
    reqsize = libc_xdlpath2(handle,buf,bufsize+1,type);
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

EXPORT(Xxdlpath,libc_Xxdlpath);
CRT_KOS ATTR_RETNONNULL char *LIBCCALL
libc_Xxdlpath(void *handle, char *buf_,
              size_t bufsize, unsigned int type) {
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
   reqsize = libc_Xxdlpath2(handle,buf,bufsize,type);
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
  reqsize = libc_Xxdlpath2(handle,buf,bufsize,type);
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
     reqsize = libc_Xxdlpath2(handle,buf,bufsize+1,type);
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

EXPORT(__KSYM(xdlpath2),libc_xdlpath2);
CRT_KOS ssize_t LIBCCALL
libc_xdlpath2(void *handle, char *buf,
              size_t bufsize, unsigned int type) {
 struct module_path_info query;
 ssize_t response;
 query.pi_format  = type;
 query.pi_path    = buf;
 query.pi_pathlen = bufsize;
 response = libc_xdlmodule_info(handle,
                                MODULE_INFO_CLASS_PATH,
                               &query,
                                sizeof(query));
 if (response < 0) return -1;
 if (response != sizeof(query)) {
  libc_seterrno(ENOSYS);
  return -1;
 }
 return query.pi_pathlen;
}

EXPORT(Xxdlpath2,libc_Xxdlpath2);
CRT_KOS size_t LIBCCALL
libc_Xxdlpath2(void *handle, char *buf,
               size_t bufsize, unsigned int type) {
 struct module_path_info query;
 query.pi_format  = type;
 query.pi_path    = buf;
 query.pi_pathlen = bufsize;
 if (libc_Xxdlmodule_info(handle,MODULE_INFO_CLASS_PATH,&query,sizeof(query)) != sizeof(query))
     libc_error_throw(E_NOT_IMPLEMENTED);
 return query.pi_pathlen;
}


EXPORT(__DSYM(xdlpath),libc_dos_xdlpath);
CRT_DOS_EXT char *LIBCCALL
libc_dos_xdlpath(void *handle, char *buf, size_t bufsize, unsigned int type) {
 return libc_xdlpath(handle,buf,bufsize,type|REALPATH_FDOSPATH);
}
EXPORT(__DSYM(xdlpath2),libc_dos_xdlpath2);
CRT_DOS_EXT ssize_t LIBCCALL
libc_dos_xdlpath2(void *handle, char *buf, size_t bufsize, unsigned int type) {
 return libc_xdlpath2(handle,buf,bufsize,type|REALPATH_FDOSPATH);
}




DECL_END

#endif /* !GUARD_LIBS_LIBC_DL_C */
