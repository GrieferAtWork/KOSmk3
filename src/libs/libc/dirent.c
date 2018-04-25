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
#ifndef GUARD_LIBS_LIBC_DIRENT_C
#define GUARD_LIBS_LIBC_DIRENT_C 1

#include "libc.h"
#include "dirent.h"
#include "malloc.h"
#include "unistd.h"
#include "system.h"
#include "errno.h"
#include "rtl.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <syslog.h>

DECL_BEGIN

EXPORT(fdopendir,libc_fdopendir);
INTERN DIR *LIBCCALL libc_fdopendir(fd_t fd) {
 DIR *result = (DIR *)libc_malloc(sizeof(DIR));
 if unlikely(!result) goto done;
 dirstream_init(result,fd);
done:
 return result;
}

PRIVATE DIR *LIBCCALL
libc_fopendirat(fd_t dfd, char const *name, oflag_t flags) {
 DIR *result;
 fd_t fd = libc_openat(dfd,name,flags);
 if (fd < 0) return NULL;
 result = libc_fdopendir(fd);
 if (!result) sys_close(fd);
 return result;
}

EXPORT(__KSYM(opendir),libc_opendir);
INTERN DIR *LIBCCALL
libc_opendir(char const *name) {
 return libc_fopendirat(AT_FDCWD,name,O_RDONLY|O_DIRECTORY);
}

EXPORT(__KSYM(opendirat),libc_opendirat);
INTERN DIR *LIBCCALL
libc_opendirat(fd_t dfd, char const *name) {
 return libc_fopendirat(dfd,name,O_RDONLY|O_DIRECTORY);
}

EXPORT(__DSYM(opendir),libc_dos_opendir);
CRT_DOS_EXT DIR *LIBCCALL
libc_dos_opendir(char const *name) {
 return libc_fopendirat(AT_FDCWD,name,O_RDONLY|O_DIRECTORY|O_DOSPATH);
}

EXPORT(__DSYM(opendirat),libc_dos_opendirat);
CRT_DOS_EXT DIR *LIBCCALL
libc_dos_opendirat(fd_t dfd, char const *name) {
 return libc_fopendirat(dfd,name,O_RDONLY|O_DIRECTORY|O_DOSPATH);
}

EXPORT(__KSYM(fopendirat),libc_public_fopendirat);
DEFINE_NULL_WRAPPER(DIR *,libc_public_fopendirat,(fd_t dfd, char const *name, int flags),
                          libc_public_Xfopendirat,(dfd,name,flags))

EXPORT(__DSYM(fopendirat),libc_dos_public_fopendirat);
CRT_DOS_EXT DIR *LIBCCALL
libc_dos_public_fopendirat(fd_t dfd, char const *name, int flags) {
 return libc_public_fopendirat(dfd,name,flags|AT_DOSPATH);
}

EXPORT(closedir,libc_closedir);
INTERN int LIBCCALL libc_closedir(DIR *self) {
 if (!self) { libc_seterrno(EINVAL); return -1; }
 dirstream_fini(self);
 libc_free(self);
 return 0;
}

EXPORT(readdir,libc_readdir);
INTERN struct dirent *LIBCCALL
libc_readdir(DIR *__restrict self) {
 struct dirent *result; ssize_t load_size;
again:
 result = self->ds_next;
 /* Check if there the pre-loaded next-pointer is valid. */
 if (READDIR_MULTIPLE_ISVALID(result,self->ds_buf,self->ds_lodsize)) {
  if (READDIR_MULTIPLE_ISEOF(result))
      return NULL; /* End of directory */
  self->ds_next = READDIR_MULTIPLE_GETNEXT(result);
  return result;
 }
 /* Read more entires. */
 load_size = libc_xreaddir(self->ds_fd,
                           self->ds_buf,
                           self->ds_bufsize,
                           READDIR_MULTIPLE|
                           READDIR_WANTEOF);
 if unlikely(load_size <= 0)
    return NULL; /* Error, or end-of-directory. */
 if unlikely((size_t)load_size > self->ds_bufsize) {
  size_t new_bufsize;
  struct dirent *new_buffer;
  /* Buffer is too small. -> Must allocate one that is larger. */
  if (self->ds_buf == (struct dirent *)self->ds_sbuf)
      libc_realloc_in_place(self,offsetof(DIR,ds_sbuf)),
      self->ds_buf = NULL;
  new_bufsize  = load_size << 1;
  if unlikely(new_bufsize < (size_t)load_size)
              new_bufsize = (size_t)load_size;
  new_buffer = (struct dirent *)libc_realloc(self->ds_buf,new_bufsize);
  if (!new_buffer) {
   new_bufsize = (size_t)load_size;
   new_buffer = (struct dirent *)libc_realloc(self->ds_buf,new_bufsize);
   if (!new_buffer) return NULL;
  }
  /* Install the new buffer. */
  self->ds_buf     = new_buffer;
  self->ds_bufsize = new_bufsize;
 }
 self->ds_next    = self->ds_buf;
 self->ds_lodsize = (size_t)load_size;
 goto again;
}
EXPORT(readdir64,libc_readdir64);
DEFINE_INTERN_ALIAS(libc_readdir64,libc_readdir);

EXPORT(rewinddir,libc_rewinddir);
INTERN void LIBCCALL
libc_rewinddir(DIR *__restrict self) {
 sys_lseek(self->ds_fd,0,SEEK_SET);
}

EXPORT(xreaddir,libc_xreaddir);
INTERN ssize_t LIBCCALL
libc_xreaddir(fd_t fd, struct dirent *buf,
              size_t bufsize, int mode) {
 return FORWARD_SYSTEM_VALUE(sys_xreaddir(fd,buf,bufsize,mode));
}
EXPORT(xreaddir64,libc_xreaddir64);
DEFINE_INTERN_ALIAS(libc_xreaddir64,libc_xreaddir);

EXPORT(xreaddirf,libc_xreaddirf);
INTERN ssize_t LIBCCALL
libc_xreaddirf(fd_t fd, struct dirent *buf,
               size_t bufsize, int mode, oflag_t flags) {
 return FORWARD_SYSTEM_VALUE(sys_xreaddirf(fd,buf,bufsize,mode,flags));
}
EXPORT(xreaddirf64,libc_xreaddirf64);
DEFINE_INTERN_ALIAS(libc_xreaddirf64,libc_xreaddirf);


EXPORT(seekdir,libc_seekdir);
INTERN int LIBCCALL libc_seekdir(DIR *__restrict self, long int pos) {
#if __SIZEOF_LONG__ == 4
 return libc_lseek(self->ds_fd,(off32_t)pos,SEEK_SET) < 0 ? -1 : 0;
#else
 return libc_lseek64(self->ds_fd,(off64_t)pos,SEEK_SET) < 0 ? -1 : 0;
#endif
}

EXPORT(telldir,libc_telldir);
INTERN long int LIBCCALL libc_telldir(DIR *__restrict self) {
#if __SIZEOF_LONG__ == 4
 return (long int)libc_lseek(self->ds_fd,0,SEEK_CUR);
#else
 return (long int)libc_lseek64(self->ds_fd,0,SEEK_CUR);
#endif
}


EXPORT(dirfd,libc_dirfd);
INTERN int LIBCCALL
libc_dirfd(DIR *__restrict self) {
 return self->ds_fd;
}

EXPORT(readdir_r,libc_readdir_r);
INTERN int LIBCCALL
libc_readdir_r(DIR *__restrict self,
               struct dirent *__restrict entry,
               struct dirent **__restrict result) {
 libc_seterrno(ENOSYS);
 return -1;
}
EXPORT(readdir64_r,libc_readdir64_r);
DEFINE_INTERN_ALIAS(libc_readdir64_r,libc_readdir_r);

EXPORT(scandir,libc_scandir);
INTERN int LIBCCALL
libc_scandir(char const *__restrict dir,
             struct dirent ***__restrict namelist,
             int (*selector)(struct dirent const *),
             int (*cmp)(struct dirent const **, struct dirent const **)) {
 libc_seterrno(ENOSYS);
 return -1;
}
EXPORT(scandir64,libc_scandir64);
DEFINE_INTERN_ALIAS(libc_scandir64,libc_scandir);

EXPORT(scandirat,libc_scandirat);
INTERN int LIBCCALL
libc_scandirat(fd_t dfd,
               char const *__restrict dir,
               struct dirent ***__restrict namelist,
               int(*selector)(struct dirent const *),
               int(*cmp)(struct dirent const **,struct dirent const **)) {
 libc_seterrno(ENOSYS);
 return -1;
}
EXPORT(scandirat64,libc_scandirat64);
DEFINE_INTERN_ALIAS(libc_scandirat64,libc_scandirat);

EXPORT(getdirentries,libc_getdirentries);
INTERN ssize_t LIBCCALL
libc_getdirentries(fd_t fd,
                   char *__restrict buf,
                   size_t nbytes,
                   pos_t *__restrict basep) {
 libc_seterrno(ENOSYS);
 return -1;
}
EXPORT(getdirentries64,libc_getdirentries64);
DEFINE_INTERN_ALIAS(libc_getdirentries64,libc_getdirentries);

EXPORT(alphasort,libc_alphasort);
INTERN int LIBCCALL
libc_alphasort(struct dirent const **e1,
               struct dirent const **e2) {
 libc_seterrno(ENOSYS);
 return -1;
}
EXPORT(alphasort64,libc_alphasort64);
DEFINE_INTERN_ALIAS(libc_alphasort64,libc_alphasort);

EXPORT(versionsort,libc_versionsort);
INTERN int LIBCCALL
libc_versionsort(struct dirent const **e1,
                 struct dirent const **e2) {
 libc_seterrno(ENOSYS);
 return -1;
}
EXPORT(versionsort64,libc_versionsort64);
DEFINE_INTERN_ALIAS(libc_versionsort64,libc_versionsort);




EXPORT(Xfdopendir,libc_Xfdopendir);
CRT_EXCEPT DIR *LIBCCALL libc_Xfdopendir(fd_t fd) {
 DIR *result;
 result = (DIR *)libc_Xmalloc(sizeof(DIR));
 result->ds_fd      = fd;
 result->ds_lodsize = 0;
 result->ds_bufsize = sizeof(result->ds_sbuf);
 result->ds_next    = (struct dirent *)result->ds_sbuf;
 result->ds_buf     = (struct dirent *)result->ds_sbuf;
 return result;
}

CRT_EXCEPT ATTR_RETNONNULL DIR *LIBCCALL
libc_Xfopendirat(fd_t dfd, char const *name, oflag_t flags) {
 DIR *COMPILER_IGNORE_UNINITIALIZED(result);
 fd_t fd = libc_Xopenat(dfd,name,flags);
 LIBC_TRY {
  result = libc_Xfdopendir(fd);
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  sys_close(fd);
  error_rethrow();
 }
 return result;
}

EXPORT(Xfopendirat,libc_public_Xfopendirat);
CRT_EXCEPT ATTR_RETNONNULL DIR *LIBCCALL
libc_public_Xfopendirat(fd_t dfd, char const *name, int flags) {
 oflag_t oflags = O_RDONLY|O_DIRECTORY;
 /* Check flags to be valid (same as would be done by the kernel) */
 if (flags & ~(AT_DOSPATH|AT_SYMLINK_NOFOLLOW|AT_EMPTY_PATH)) {
  libc_seterrno(EINVAL);
  return NULL;
 }
 /* Convert AT_* flags to O_* flags. */
 if (flags & AT_DOSPATH)          oflags |= O_DOSPATH;
 if (flags & AT_SYMLINK_NOFOLLOW) oflags |= O_NOFOLLOW;
 if (flags & AT_EMPTY_PATH) {
  char *end;
  while (libc_isspace(*name)) ++name;
  end = libc_strend(name);
  while (end > name &&
        (libc_isspace(end[-1]) || end[-1] == '/' ||
        (end[-1] == '\\' &&
        (flags & AT_DOSPATH ? !LIBC_DOSMODE_DISABLED()
                            : LIBC_DOSMODE_ENABLED()))))
         --end;
  if (end == name) {
   DIR *COMPILER_IGNORE_UNINITIALIZED(result);
   dfd = libc_dup(dfd);
   LIBC_TRY {
    result = libc_Xfdopendir(dfd);
   } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
    sys_close(dfd);
    error_rethrow();
   }
   return result;
  }
 }
 return libc_Xfopendirat(dfd,name,oflags);
}

EXPORT(Xopendirat,libc_Xopendirat);
CRT_EXCEPT ATTR_RETNONNULL DIR *LIBCCALL
libc_Xopendirat(fd_t dfd, char const *name) {
 return libc_Xfopendirat(dfd,name,O_RDONLY|O_DIRECTORY);
}

EXPORT(Xopendir,libc_Xopendir);
CRT_EXCEPT ATTR_RETNONNULL DIR *LIBCCALL
libc_Xopendir(char const *name) {
 return libc_Xfopendirat(AT_FDCWD,name,O_RDONLY|O_DIRECTORY);
}

EXPORT(Xreaddir,libc_Xreaddir);
CRT_EXCEPT struct dirent *LIBCCALL
libc_Xreaddir(DIR *__restrict self) {
 struct dirent *result; size_t load_size;
again:
 result = self->ds_next;
 /* Check if there the pre-loaded next-pointer is valid. */
 if (READDIR_MULTIPLE_ISVALID(result,self->ds_buf,self->ds_lodsize)) {
  if (READDIR_MULTIPLE_ISEOF(result))
      return NULL; /* End of directory */
  self->ds_next = READDIR_MULTIPLE_GETNEXT(result);
  return result;
 }
 /* Read more entires. */
 load_size = libc_Xxreaddir(self->ds_fd,
                            self->ds_buf,
                            self->ds_bufsize,
                            READDIR_MULTIPLE|
                            READDIR_WANTEOF);
 if unlikely(!load_size)
    return NULL; /* end-of-directory. */
 if unlikely((size_t)load_size > self->ds_bufsize) {
  size_t new_bufsize;
  /* Buffer is too small. -> Must allocate one that is larger. */
  if (self->ds_buf == (struct dirent *)self->ds_sbuf)
      libc_realloc_in_place(self,offsetof(DIR,ds_sbuf)),
      self->ds_buf = NULL;
  new_bufsize  = load_size << 1;
  if unlikely(new_bufsize < (size_t)load_size)
              new_bufsize = (size_t)load_size;
  LIBC_TRY {
   self->ds_buf = (struct dirent *)libc_Xrealloc(self->ds_buf,new_bufsize);
  } LIBC_CATCH_HANDLED (E_BADALLOC) {
   new_bufsize  = (size_t)load_size;
   self->ds_buf = (struct dirent *)libc_Xrealloc(self->ds_buf,new_bufsize);
  }
  self->ds_bufsize = new_bufsize;
 }
 self->ds_next    = self->ds_buf;
 self->ds_lodsize = (size_t)load_size;
 goto again;
}
DEFINE_INTERN_ALIAS(libc_Xreaddir64,libc_Xreaddir);
EXPORT(Xreaddir64,libc_Xreaddir64);

EXPORT(Xseekdir,libc_Xseekdir);
CRT_EXCEPT void LIBCCALL libc_Xseekdir(DIR *__restrict self, long int pos) {
#if __SIZEOF_LONG__ == 4
 libc_Xlseek(self->ds_fd,(off32_t)pos,SEEK_SET);
#else
 libc_Xlseek64(self->ds_fd,(off64_t)pos,SEEK_SET);
#endif
}

EXPORT(Xtelldir,libc_Xtelldir);
CRT_EXCEPT unsigned long int LIBCCALL libc_Xtelldir(DIR *__restrict self) {
#if __SIZEOF_LONG__ == 4
 return (unsigned long int)libc_Xlseek(self->ds_fd,0,SEEK_CUR);
#else
 return (unsigned long int)libc_Xlseek64(self->ds_fd,0,SEEK_CUR);
#endif
}

EXPORT(Xreaddir64_r,libc_Xreaddir64_r);
CRT_EXCEPT void LIBCCALL
libc_Xreaddir64_r(DIR *__restrict dirp,
                  struct dirent64 *__restrict entry,
                  struct dirent64 **__restrict result) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}
DEFINE_INTERN_ALIAS(libc_Xreaddir_r,libc_Xreaddir64_r);
EXPORT(Xreaddir_r,libc_Xreaddir_r);

EXPORT(Xscandir64,libc_Xscandir64);
CRT_EXCEPT void LIBCCALL
libc_Xscandir64(char const *__restrict dir,
                struct dirent64 ***__restrict namelist,
                int (*selector)(struct dirent64 const *),
                int (*cmp)(struct dirent64 const **, struct dirent64 const **)) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}
DEFINE_INTERN_ALIAS(libc_Xscandir,libc_Xscandir64);
EXPORT(Xscandir,libc_Xscandir);

EXPORT(Xscandirat64,libc_Xscandirat64);
CRT_EXCEPT void LIBCCALL
libc_Xscandirat64(fd_t dfd, char const *__restrict dir,
                  struct dirent64 ***__restrict namelist,
                  int (*selector)(struct dirent64 const *),
                  int (*cmp)(struct dirent64 const **, struct dirent64 const **)) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}
DEFINE_INTERN_ALIAS(libc_Xscandirat,libc_Xscandirat64);
EXPORT(Xscandirat,libc_Xscandirat);

EXPORT(Xgetdirentries64,libc_Xgetdirentries64);
CRT_EXCEPT size_t LIBCCALL
libc_Xgetdirentries64(fd_t fd, char *__restrict buf,
                      size_t nbytes, pos_t *__restrict basep) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}
DEFINE_INTERN_ALIAS(libc_Xgetdirentries,libc_Xgetdirentries64);
EXPORT(Xgetdirentries,libc_Xgetdirentries);






DECL_END

#endif /* !GUARD_LIBS_LIBC_DIRENT_C */
