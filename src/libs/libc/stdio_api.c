/* Copyright (ch) 2018 Griefer@Work                                            *
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
#ifndef GUARD_LIBS_LIBC_STDIO_API_C
#define GUARD_LIBS_LIBC_STDIO_API_C 1

#include "libc.h"
#include "stdio.h"
#include "errno.h"
#include "unistd.h"
#include "malloc.h"
#include "exit.h"
#include "exec.h"
#include "format.h"
#include "tty.h"
#include "system.h"
#include "sched.h"
#include "rtl.h"
#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <bits/io-file.h>
#include <hybrid/align.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/minmax.h>
#include <hybrid/list/list.h>
#include <kos/sched/mutex.h>
#include <unistd.h>

#ifdef CONFIG_LIBC_USES_NEW_STDIO
DECL_BEGIN


#ifndef __INTELLISENSE__
PRIVATE struct iofile_data exdata_std_files[3] = {
    [0 ... 2] = {
        .__fb_zero  = 0,
        .__fb_refcnt = 1,
        .__fb_lock  = MUTEX_INIT,
        .__fb_ops   = { NULL, NULL, NULL, NULL },
        .__fb_arg   = NULL,
        .__fb_chng  = NULL,
        .__fb_chsz  = 0,
        .__fb_ttych = { NULL, NULL },
        .__fb_files = { NULL, NULL },
        .__fb_fblk  = 0,
        .__fb_fpos  = 0
    }
};

#if 0
#define STD_INPUT_FLAGS   (FILE_BUFFER_FISATTY|FILE_BUFFER_FREADONLY|FILE_BUFFER_FSTATIC)
#define STD_OUTPUT_FLAGS  (FILE_BUFFER_FLNBUF|FILE_BUFFER_FISATTY|FILE_BUFFER_FSTATIC)
#else
#define STD_INPUT_FLAGS   (FILE_BUFFER_FREADONLY|FILE_BUFFER_FSTATIC)
#define STD_OUTPUT_FLAGS  (FILE_BUFFER_FLNIFTTY|FILE_BUFFER_FSTATIC)
#endif

INTERN FILE libc_std_files[3] = {
    [0] = {
        .if_ptr    = NULL,
        .if_cnt    = 0,
        .if_base   = NULL,
        .if_flag   = STD_INPUT_FLAGS,
        .if_fd     = STDIN_FILENO,
        .if_bufsiz = 0,
        .if_exdata = &exdata_std_files[0]
    },
    [1] = {
        .if_ptr    = NULL,
        .if_cnt    = 0,
        .if_base   = NULL,
        .if_flag   = STD_OUTPUT_FLAGS,
        .if_fd     = STDOUT_FILENO,
        .if_bufsiz = 0,
        .if_exdata = &exdata_std_files[1]
    },
    [2] = {
        .if_ptr    = NULL,
        .if_cnt    = 0,
        .if_base   = NULL,
        .if_flag   = STD_OUTPUT_FLAGS,
        .if_fd     = STDERR_FILENO,
        .if_bufsiz = 0,
        .if_exdata = &exdata_std_files[2]
    }
};
PUBLIC FILE *stdin  = libc_std_files+0;
PUBLIC FILE *stdout = libc_std_files+1;
PUBLIC FILE *stderr = libc_std_files+2;
#endif



/* File operators. */
EXPORT(fread_unlocked,libc_fread_unlocked);
EXPORT(__DSYM(_fread_nolock),libc_fread_unlocked);
CRT_STDIO_API size_t LIBCCALL
libc_fread_unlocked(void *__restrict buf, size_t size,
                    size_t n, FILE *__restrict self) {
 return FileBuffer_ReadUnlocked(self,buf,size*n) / size;
}
EXPORT(Xfread_unlocked,libc_Xfread_unlocked);
CRT_STDIO_XAPI size_t LIBCCALL
libc_Xfread_unlocked(void *__restrict buf, size_t size,
                     size_t n, FILE *__restrict self) {
 return FileBuffer_XReadUnlocked(self,buf,size*n) / size;
}

EXPORT(fwrite_unlocked,libc_fwrite_unlocked);
EXPORT(__DSYM(_fwrite_nolock),libc_fwrite_unlocked);
CRT_STDIO_API size_t LIBCCALL
libc_fwrite_unlocked(void const *__restrict buf, size_t size,
                     size_t n, FILE *__restrict self) {
 return FileBuffer_WriteUnlocked(self,buf,size*n) / size;
}

EXPORT(Xfwrite_unlocked,libc_fwrite_unlocked);
CRT_STDIO_XAPI size_t LIBCCALL
libc_Xfwrite_unlocked(void const *__restrict buf, size_t size,
                      size_t n, FILE *__restrict self) {
 return FileBuffer_XWriteUnlocked(self,buf,size*n) / size;
}


EXPORT(fread,libc_fread);
CRT_STDIO_API size_t LIBCCALL
libc_fread(void *__restrict buf, size_t size,
           size_t n, FILE *__restrict self) {
 return FileBuffer_ReadUnlocked(self,buf,size*n) / size;
}
EXPORT(Xfread,libc_Xfread);
CRT_STDIO_XAPI size_t LIBCCALL
libc_Xfread(void *__restrict buf, size_t size,
            size_t n, FILE *__restrict self) {
 return FileBuffer_XReadUnlocked(self,buf,size*n) / size;
}

EXPORT(fwrite,libc_fwrite);
CRT_STDIO_API size_t LIBCCALL
libc_fwrite(void const *__restrict buf, size_t size,
            size_t n, FILE *__restrict self) {
 return FileBuffer_WriteUnlocked(self,buf,size*n) / size;
}

EXPORT(Xfwrite,libc_fwrite);
CRT_STDIO_XAPI size_t LIBCCALL
libc_Xfwrite(void const *__restrict buf, size_t size,
             size_t n, FILE *__restrict self) {
 return FileBuffer_XWriteUnlocked(self,buf,size*n) / size;
}

EXPORT(file_printer,libc_file_printer);
CRT_STDIO_API ssize_t LIBCCALL
libc_file_printer(char const *__restrict data, size_t datalen, void *closure) {
 return FileBuffer_Write((FileBuffer *)closure,data,datalen);
}

EXPORT(Xfile_printer,libc_Xfile_printer);
CRT_STDIO_XAPI ssize_t LIBCCALL
libc_Xfile_printer(char const *__restrict data, size_t datalen, void *closure) {
 return (ssize_t)FileBuffer_XWrite((FileBuffer *)closure,data,datalen);
}

EXPORT(file_printer_unlocked,libc_file_printer_unlocked);
CRT_STDIO_API ssize_t LIBCCALL
libc_file_printer_unlocked(char const *__restrict data, size_t datalen, void *closure) {
 return FileBuffer_WriteUnlocked((FileBuffer *)closure,data,datalen);
}

EXPORT(Xfile_printer_unlocked,libc_Xfile_printer_unlocked);
CRT_STDIO_XAPI ssize_t LIBCCALL
libc_Xfile_printer_unlocked(char const *__restrict data, size_t datalen, void *closure) {
 return (ssize_t)FileBuffer_XWriteUnlocked((FileBuffer *)closure,data,datalen);
}


EXPORT(flockfile,libc_flockfile);
EXPORT(__DSYM(_lock_file),libc_flockfile); /* DOS Alias */
CRT_STDIO_API void LIBCCALL
libc_flockfile(FILE *__restrict self) {
 while (FileBuffer_Lock(self)) {
  if (libc_geterrno() == EINTR) continue;
  /* XXX: What should we do if this fails for a reason other than EINTR? */
 }
}

EXPORT(Xflockfile,libc_Xflockfile);
CRT_STDIO_XAPI void LIBCCALL
libc_Xflockfile(FILE *__restrict self) {
 FileBuffer_XLock(self);
}

EXPORT(Xftrylockfile,libc_ftrylockfile);
CRT_STDIO_API int LIBCCALL
libc_ftrylockfile(FILE *__restrict self) {
 return libc_mutex_try(&self->fb_lock) ? 0 : -1;
}

EXPORT(funlockfile,libc_funlockfile);
EXPORT(__DSYM(_unlock_file),libc_funlockfile); /* DOS Alias */
CRT_STDIO_API void LIBCCALL
libc_funlockfile(FILE *__restrict self) {
 FileBuffer_Unlock(self);
}


EXPORT(fgetpos,libc_fgetpos);
CRT_STDIO_API int LIBCCALL
libc_fgetpos(FILE *__restrict self, pos32_t *__restrict pos) {
 off64_t off = FileBuffer_Tell(self);
 if (off < 0) return -1;
 *pos = (pos32_t)(pos64_t)off;
 return 0;
}
EXPORT(Xfgetpos,libc_Xfgetpos);
CRT_STDIO_XAPI void LIBCCALL
libc_Xfgetpos(FILE *__restrict self, pos32_t *__restrict pos) {
 *pos = (pos32_t)FileBuffer_XTell(self);
}

EXPORT(fgetpos_unlocked,libc_fgetpos_unlocked);
CRT_STDIO_API int LIBCCALL
libc_fgetpos_unlocked(FILE *__restrict self, pos32_t *__restrict pos) {
 *pos = (pos32_t)FileBuffer_TellUnlocked(self);
 return 0; /* Never fails... */
}

EXPORT(Xfgetpos_unlocked,libc_Xfgetpos_unlocked);
#ifdef CONFIG_LIBCCALL_INTRETURN_IS_VOID
DEFINE_INTERN_ALIAS(libc_Xfgetpos_unlocked,libc_fgetpos_unlocked);
#else
CRT_STDIO_XAPI void LIBCCALL
libc_Xfgetpos_unlocked(FILE *__restrict self, pos32_t *__restrict pos) {
 *pos = (pos32_t)FileBuffer_TellUnlocked(self);
}
#endif

EXPORT(fgetpos64,libc_fgetpos64);
CRT_STDIO_API int LIBCCALL
libc_fgetpos64(FILE *__restrict self, pos64_t *__restrict pos) {
 off64_t off = FileBuffer_Tell(self);
 if (off < 0) return -1;
 *pos = (pos64_t)off;
 return 0;
}
EXPORT(Xfgetpos64,libc_Xfgetpos64);
CRT_STDIO_XAPI void LIBCCALL
libc_Xfgetpos64(FILE *__restrict self, pos64_t *__restrict pos) {
 *pos = FileBuffer_XTell(self);
}

EXPORT(fgetpos64_unlocked,libc_fgetpos64_unlocked);
CRT_STDIO_API int LIBCCALL
libc_fgetpos64_unlocked(FILE *__restrict self, pos64_t *__restrict pos) {
 *pos = FileBuffer_TellUnlocked(self);
 return 0; /* Never fails... */
}

EXPORT(Xfgetpos64_unlocked,libc_Xfgetpos64_unlocked);
#ifdef CONFIG_LIBCCALL_INTRETURN_IS_VOID
DEFINE_INTERN_ALIAS(libc_Xfgetpos64_unlocked,libc_fgetpos64_unlocked);
#else
CRT_STDIO_XAPI void LIBCCALL
libc_Xfgetpos64_unlocked(FILE *__restrict self, pos64_t *__restrict pos) {
 libc_fgetpos64_unlocked(self,pos);
}
#endif


EXPORT(fsetpos,libc_fsetpos);
CRT_STDIO_API int LIBCCALL
libc_fsetpos(FILE *__restrict self, pos32_t const *__restrict pos) {
 return FileBuffer_Seek(self,(off64_t)(pos64_t)*pos,SEEK_SET);
}
EXPORT(Xfsetpos,libc_Xfsetpos);
CRT_STDIO_XAPI void LIBCCALL
libc_Xfsetpos(FILE *__restrict self, pos32_t const *__restrict pos) {
 FileBuffer_XSeek(self,(off64_t)(pos64_t)*pos,SEEK_SET);
}

EXPORT(fsetpos_unlocked,libc_fsetpos_unlocked);
CRT_STDIO_API int LIBCCALL
libc_fsetpos_unlocked(FILE *__restrict self, pos32_t const *__restrict pos) {
 return FileBuffer_SeekUnlocked(self,(off64_t)(pos64_t)*pos,SEEK_SET);
}
EXPORT(Xfsetpos_unlocked,libc_Xfsetpos_unlocked);
CRT_STDIO_XAPI void LIBCCALL
libc_Xfsetpos_unlocked(FILE *__restrict self, pos32_t const *__restrict pos) {
 FileBuffer_XSeekUnlocked(self,(off64_t)(pos64_t)*pos,SEEK_SET);
}

EXPORT(fsetpos64,libc_fsetpos64);
CRT_STDIO_API int LIBCCALL
libc_fsetpos64(FILE *__restrict self, pos64_t const *__restrict pos) {
 return FileBuffer_Seek(self,(off64_t)*pos,SEEK_SET);
}
EXPORT(Xfsetpos64,libc_Xfsetpos64);
CRT_STDIO_XAPI void LIBCCALL
libc_Xfsetpos64(FILE *__restrict self, pos64_t const *__restrict pos) {
 FileBuffer_XSeek(self,(off64_t)*pos,SEEK_SET);
}

EXPORT(fsetpos64_unlocked,libc_fsetpos64_unlocked);
CRT_STDIO_API int LIBCCALL
libc_fsetpos64_unlocked(FILE *__restrict self, pos64_t const *__restrict pos) {
 return FileBuffer_SeekUnlocked(self,(off64_t)*pos,SEEK_SET);
}
EXPORT(Xfsetpos64_unlocked,libc_Xfsetpos64_unlocked);
CRT_STDIO_XAPI void LIBCCALL
libc_Xfsetpos64_unlocked(FILE *__restrict self, pos64_t const *__restrict pos) {
 FileBuffer_XSeekUnlocked(self,(off64_t)*pos,SEEK_SET);
}

CRT_STDIO_API oflag_t LIBCCALL
libc_parsemode(char const *__restrict mode) {
 oflag_t result;
 bool open_binary = false;
 if (*mode == 'r') {
  result = O_RDONLY;
 } else if (*mode == 'w') {
  result = O_WRONLY|O_TRUNC|O_CREAT;
 } else if (*mode == 'a') {
  result = O_WRONLY|O_APPEND|O_CREAT;
 } else goto err;
 if (*++mode == 'b') ++mode,open_binary = true;
 if (*mode == 'e') ++mode,result |= O_CLOEXEC;
 if (*mode == 'b' && !open_binary) ++mode,open_binary = true;
 if (*mode == '+') ++mode,result &= ~O_ACCMODE,result |= O_RDWR;
 if (*mode == 'b' && !open_binary) ++mode,open_binary = true;
 if (*mode == 'e' && !(result&O_CLOEXEC)) ++mode,result |= O_CLOEXEC;
 if (*mode == 'b' && !open_binary) ++mode,open_binary = true;
 if (*mode == 'x' && (result&(O_TRUNC|O_CREAT)) == (O_TRUNC|O_CREAT)) ++mode,result |= O_EXCL;
 if (*mode == 't' && !open_binary) ++mode; /* Accept a trailing `t', as suggested by STD-C */
 if (*mode) goto err;
 return result;
err:
 return (oflag_t)-1;
}


PRIVATE int LIBCCALL stub_close(void *UNUSED(cookie)) {
 return 0;
}


EXPORT(fopencookie,libc_fopencookie);
CRT_STDIO_API ATTR_MALLOC FILE *LIBCCALL
libc_fopencookie(void *__restrict magic_cookie,
                 char const *__restrict modes,
                 cookie_io_functions_t io_funcs) {
 FILE *result; oflag_t mode;
 if unlikely((mode = libc_parsemode(modes)) == (oflag_t)-1) { libc_seterrno(EINVAL); return NULL; }
 if unlikely((result = FileBuffer_Alloc()) == NULL) goto done;
 result->fb_arg  = magic_cookie;
 result->fb_ops  = io_funcs;
 result->fb_file = -1;
 /* Since we use the close operator to indicate a cookie file,
  * assign a stub function if the user didn't specify that one. */
 if (!result->fb_ops.cio_close)
      result->fb_ops.cio_close = &stub_close;
 if ((mode & O_ACCMODE) == O_WRONLY)
      result->fb_flag |= FILE_BUFFER_FREADONLY;
 FileBuffer_Register(result);
done:
 return result;
}
CRT_STDIO_XAPI ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL
libc_Xfopencookie_impl(void *__restrict magic_cookie,
                       char const *__restrict modes,
                       cookie_io_functions_t io_funcs) {
 FILE *result; oflag_t mode;
 if unlikely((mode = libc_parsemode(modes)) == (oflag_t)-1)
    libc_error_throw(E_INVALID_ARGUMENT);
 result = FileBuffer_XAlloc();
 result->fb_arg  = magic_cookie;
 result->fb_ops  = io_funcs;
 result->fb_file = -1;
 /* Since we use the close operator to indicate a cookie file,
  * assign a stub function if the user didn't specify that one. */
 if (!result->fb_ops.cio_close)
      result->fb_ops.cio_close = &stub_close;
 if ((mode & O_ACCMODE) == O_WRONLY)
      result->fb_flag |= FILE_BUFFER_FREADONLY;
 return result;
}


EXPORT(Xfopencookie,libc_Xfopencookie);
CRT_STDIO_XAPI ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL
libc_Xfopencookie(void *__restrict magic_cookie,
                  char const *__restrict modes,
                  cookie_io_functions_t io_funcs) {
 FILE *result = libc_Xfopencookie_impl(magic_cookie,modes,io_funcs);
 FileBuffer_Register(result);
 return result;
}

EXPORT(tmpfile,libc_tmpfile);
CRT_STDIO_API ATTR_MALLOC FILE *LIBCCALL libc_tmpfile(void) {
 LIBC_TRY {
  return libc_Xtmpfile();
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return NULL;
}

EXPORT(Xtmpfile,libc_Xtmpfile);
CRT_STDIO_XAPI ATTR_MALLOC
ATTR_RETNONNULL FILE *LIBCCALL libc_Xtmpfile(void) {
 FILE *COMPILER_IGNORE_UNINITIALIZED(result);
 char name[32];
again:
 libc_sprintf(name,"::/tmp/stdio_%x.tmp",libc_rand());
 LIBC_TRY {
  result = libc_Xfopen(name,"w+x");
 } LIBC_CATCH (E_FILESYSTEM_ERROR) {
  /* If the file already exists, generate a new random number and try once more. */
  if (error_info()->e_error.e_filesystem_error.fs_errcode == ERROR_FS_FILE_ALREADY_EXISTS) {
   error_handled();
   goto again;
  }
  /* Propagate all other errors. */
  error_rethrow();
 }
 return result;
}

EXPORT(tmpfile64,libc_tmpfile64);
EXPORT(Xtmpfile64,libc_Xtmpfile64);
DEFINE_INTERN_ALIAS(libc_tmpfile64,libc_tmpfile);
DEFINE_INTERN_ALIAS(libc_Xtmpfile64,libc_Xtmpfile);



EXPORT(__KSYM(fopenat),libc_fopenat);
CRT_STDIO_API ATTR_MALLOC FILE *LIBCCALL
libc_fopenat(fd_t dfd, char const *__restrict filename,
             char const *__restrict modes, atflag_t flags) {
 FILE *COMPILER_IGNORE_UNINITIALIZED(result);
 LIBC_TRY {
  result = libc_Xfopenat(dfd,filename,modes,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
  result = NULL;
 }
 return result;
}
EXPORT(Xfopenat,libc_Xfopenat);
CRT_STDIO_API ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL
libc_Xfopenat(fd_t dfd, char const *__restrict filename,
              char const *__restrict modes, atflag_t flags) {
 FILE *COMPILER_IGNORE_UNINITIALIZED(result);
 fd_t fd; oflag_t mode = libc_parsemode(modes);
 if ((mode == (oflag_t)-1) ||
     (flags & ~(AT_SYMLINK_NOFOLLOW|AT_DOSPATH|AT_EMPTY_PATH)))
      libc_error_throw(E_INVALID_ARGUMENT);
 if (flags & AT_SYMLINK_NOFOLLOW) mode |= O_NOFOLLOW;
 if (flags & AT_DOSPATH)          mode |= O_DOSPATH;
 if (flags & AT_EMPTY_PATH) {
  size_t len;
  while (libc_isspace(*filename)) ++filename;
  len = libc_strlen(filename);
  while (len && libc_isspace(filename[len-1])) --len;
  if (!len) filename = ".";
 }
 fd = libc_Xopenat(dfd,filename,mode,0644);
 LIBC_TRY {
  result = FileBuffer_XAlloc();
  result->fb_file = fd;
  if ((mode & O_ACCMODE) == O_WRONLY)
       result->fb_flag |= FILE_BUFFER_FREADONLY;
 } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  sys_close(fd);
  error_rethrow();
 }
 FileBuffer_Register(result);
 return result;
}

EXPORT(__KSYM(fopenat64),libc_fopenat64);
EXPORT(Xfopenat64,libc_Xfopenat64);
DEFINE_INTERN_ALIAS(libc_fopenat64,libc_fopenat);
DEFINE_INTERN_ALIAS(libc_Xfopenat64,libc_Xfopenat);

EXPORT(fdopen,libc_fdopen);
EXPORT(__DSYM(_fdopen),libc_fdopen);
CRT_STDIO_API ATTR_MALLOC FILE *LIBCCALL
libc_fdopen(fd_t fd, char const *__restrict modes) {
 FILE *result; oflag_t mode;
 if unlikely((mode = libc_parsemode(modes)) == (oflag_t)-1) { libc_seterrno(EINVAL); return NULL; }
 result = FileBuffer_Alloc();
 if unlikely(!result) goto done;
 result->fb_file = fd;
 if ((mode & O_ACCMODE) == O_WRONLY)
      result->fb_flag |= FILE_BUFFER_FREADONLY;
 FileBuffer_Register(result);
done:
 return result;
}

EXPORT(Xfdopen,libc_Xfdopen);
CRT_STDIO_API ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL
libc_Xfdopen(fd_t fd, char const *__restrict modes) {
 FILE *result; oflag_t mode;
 if unlikely((mode = libc_parsemode(modes)) == (oflag_t)-1)
    libc_error_throw(E_INVALID_ARGUMENT);
 result = FileBuffer_XAlloc();
 result->fb_file = fd;
 if ((mode & O_ACCMODE) == O_WRONLY)
      result->fb_flag |= FILE_BUFFER_FREADONLY;
 FileBuffer_Register(result);
 return result;
}


EXPORT(__DSYM(fopenat),libc_dos_fopenat);
CRT_DOS_EXT ATTR_MALLOC FILE *LIBCCALL
libc_dos_fopenat(fd_t dfd, char const *__restrict filename,
                 char const *__restrict modes, atflag_t flags) {
 return libc_fopenat(dfd,filename,modes,flags|AT_DOSPATH);
}
EXPORT(__DSYM(fopenat64),libc_dos_fopenat64);
DEFINE_INTERN_ALIAS(libc_dos_fopenat64,libc_dos_fopenat);



EXPORT(__KSYM(fopen),libc_fopen);
CRT_STDIO_API ATTR_MALLOC FILE *LIBCCALL
libc_fopen(char const *__restrict filename,
           char const *__restrict modes) {
 return libc_fopenat(AT_FDCWD,filename,modes,0);
}
EXPORT(__DSYM(fopen),libc_dos_fopen);
CRT_DOS ATTR_MALLOC FILE *LIBCCALL
libc_dos_fopen(char const *__restrict filename,
               char const *__restrict modes) {
 return libc_fopenat(AT_FDCWD,filename,modes,AT_DOSPATH);
}
EXPORT(Xfopen,libc_Xfopen);
CRT_STDIO_XAPI ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL
libc_Xfopen(char const *__restrict filename,
            char const *__restrict modes) {
 return libc_Xfopenat(AT_FDCWD,filename,modes,0);
}

EXPORT(__KSYM(fopen64),libc_fopen64);
EXPORT(__DSYM(fopen64),libc_dos_fopen64);
EXPORT(Xfopen64,libc_Xfopen64);
DEFINE_INTERN_ALIAS(libc_fopen64,libc_fopen);
DEFINE_INTERN_ALIAS(libc_dos_fopen64,libc_dos_fopen);
DEFINE_INTERN_ALIAS(libc_Xfopen64,libc_Xfopen);


EXPORT(fdreopen,libc_fdreopen);
CRT_STDIO_API FILE *LIBCCALL
libc_fdreopen(fd_t fd, char const *__restrict modes,
              FILE *__restrict self, int mode) {
 LIBC_TRY {
  return libc_Xfdreopen(fd,modes,self,mode);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return NULL;
}
EXPORT(Xfdreopen,libc_Xfdreopen);
CRT_STDIO_XAPI ATTR_RETNONNULL FILE *LIBCCALL
libc_Xfdreopen(fd_t fd, char const *__restrict modes,
               FILE *__restrict self, int mode) {
 LIBC_TRY {
  FileBuffer_XLock(self);
  LIBC_TRY {
   oflag_t new_mode;
   new_mode = libc_parsemode(modes);
   if unlikely(new_mode == (oflag_t)-1)
      libc_error_throw(E_INVALID_ARGUMENT);
   /* Flush the old descriptor one last time. */
   FileBuffer_XFlushUnlocked(self);
   /* Duplicate the new descriptor to override the old. */
   if (self->fb_flag & __IO_FILE_IONOFD) {
    if (!(mode&FDREOPEN_INHERIT))
        fd = libc_Xdup(fd);
    self->fb_file = fd;
   } else if (mode & FDREOPEN_INHERIT) {
    /* Close the old handle. */
    sys_close(self->fb_file); /* Ignore errors. */
    /* Inherit the new fd. */
    self->fb_file = fd;
   } else {
    libc_Xdup2(fd,self->fb_file);
   }
   /* Delete the nofd flag, because now we definitely have one. */
   self->fb_flag &= ~(__IO_FILE_IONOFD|FILE_BUFFER_FREADONLY);
   if ((mode & O_ACCMODE) == O_WRONLY)
        self->fb_flag |= FILE_BUFFER_FREADONLY;
  } LIBC_FINALLY {
   FileBuffer_Unlock(self);
  }
 } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  /* Close the new descriptor on error. */
  if (mode&FDREOPEN_CLOSE_ON_ERROR)
      sys_close(fd);
  error_rethrow();
 }
 return self;
}

EXPORT(__KSYM(freopenat),libc_freopenat);
CRT_STDIO_API FILE *LIBCCALL
libc_freopenat(fd_t dfd, char const *__restrict filename,
               char const *__restrict modes, atflag_t flags,
               FILE *__restrict self) {
 FILE *COMPILER_IGNORE_UNINITIALIZED(result);
 LIBC_TRY {
  result = libc_Xfreopenat(dfd,filename,modes,flags,self);
 } LIBC_EXCEPT(libc_except_errno()) {
  result = NULL;
 }
 return result;
}

EXPORT(Xfreopenat,libc_Xfreopenat);
CRT_STDIO_XAPI ATTR_RETNONNULL FILE *LIBCCALL
libc_Xfreopenat(fd_t dfd, char const *__restrict filename,
                char const *__restrict modes, atflag_t flags,
                FILE *__restrict self) {
 oflag_t mode = libc_parsemode(modes);
 if ((mode == (oflag_t)-1) ||
     (flags & ~(AT_SYMLINK_NOFOLLOW|AT_DOSPATH|AT_EMPTY_PATH)))
      error_throw(E_INVALID_ARGUMENT);
 if (flags & AT_SYMLINK_NOFOLLOW) mode |= O_NOFOLLOW;
 if (flags & AT_DOSPATH)          mode |= O_DOSPATH;
 if (flags & AT_EMPTY_PATH) {
  size_t len;
  while (libc_isspace(*filename)) ++filename;
  len = libc_strlen(filename);
  while (len && libc_isspace(filename[len-1])) --len;
  if (!len) filename = ".";
 }
 return libc_Xfdreopen(libc_Xopenat(dfd,
                                    filename,
                                    mode,
                                    0644),
                       modes,
                       self,
                       FDREOPEN_INHERIT|
                       FDREOPEN_CLOSE_ON_ERROR);
}

EXPORT(__KSYM(freopenat64),libc_freopenat64);
EXPORT(Xfreopenat64,libc_Xfreopenat64);
DEFINE_INTERN_ALIAS(libc_freopenat64,libc_freopenat);
DEFINE_INTERN_ALIAS(libc_Xfreopenat64,libc_Xfreopenat);

EXPORT(__DSYM(freopenat),libc_dos_freopenat);
CRT_DOS_EXT FILE *LIBCCALL
libc_dos_freopenat(fd_t dfd, char const *__restrict filename,
                   char const *__restrict modes, atflag_t flags,
                   FILE *__restrict self) {
 return libc_freopenat(dfd,filename,modes,flags|AT_DOSPATH,self);
}

EXPORT(__DSYM(freopenat64),libc_dos_freopenat64);
DEFINE_INTERN_ALIAS(libc_dos_freopenat64,libc_dos_freopenat);


EXPORT(__KSYM(freopen),libc_freopen);
CRT_STDIO_API FILE *LIBCCALL
libc_freopen(char const *__restrict filename,
             char const *__restrict modes,
             FILE *__restrict self) {
 return libc_freopenat(AT_FDCWD,filename,modes,0,self);
}
EXPORT(__DSYM(freopen),libc_freopen);
CRT_DOS FILE *LIBCCALL
libc_dos_freopen(char const *__restrict filename,
                 char const *__restrict modes,
                 FILE *__restrict self) {
 return libc_freopenat(AT_FDCWD,filename,modes,AT_DOSPATH,self);
}

EXPORT(Xfreopen,libc_Xfreopen);
CRT_STDIO_XAPI ATTR_RETNONNULL FILE *LIBCCALL
libc_Xfreopen(char const *__restrict filename,
              char const *__restrict modes,
              FILE *__restrict self) {
 return libc_Xfreopenat(AT_FDCWD,filename,modes,0,self);
}

EXPORT(__KSYM(freopen64),libc_freopen64);
EXPORT(__DSYM(freopen64),libc_dos_freopen64);
EXPORT(Xfreopen64,libc_Xfreopen64);
DEFINE_INTERN_ALIAS(libc_freopen64,libc_freopen);
DEFINE_INTERN_ALIAS(libc_dos_freopen64,libc_dos_freopen);
DEFINE_INTERN_ALIAS(libc_Xfreopen64,libc_Xfreopen);

struct memopen_file {
    byte_t *mf_base;
    byte_t *mf_ptr;
    byte_t *mf_end;
};

PRIVATE ssize_t LIBCCALL
memopen_read(struct memopen_file *__restrict self,
             char *buffer, size_t bufsize) {
 size_t maxread = self->mf_end-self->mf_ptr;
 if (maxread > bufsize)
     maxread = bufsize;
 libc_memcpy(buffer,self->mf_ptr,maxread);
 self->mf_ptr += maxread;
 return (ssize_t)maxread;
}
PRIVATE ssize_t LIBCCALL
memopen_write(struct memopen_file *__restrict self,
              char const *buffer, size_t bufsize) {
 size_t maxwrite = self->mf_end-self->mf_ptr;
 if (maxwrite > bufsize)
     maxwrite = bufsize;
 libc_memcpy(self->mf_ptr,buffer,maxwrite);
 self->mf_ptr += maxwrite;
 /* Terminate the buffer with a NUL character. */
 if (self->mf_ptr != self->mf_end)
     *self->mf_ptr = 0;
 return (ssize_t)maxwrite;
}
PRIVATE int LIBCCALL
memopen_seek(struct memopen_file *__restrict self,
             off64_t *offset, int whence) {
 off64_t new_pos; size_t size;
 size    = (size_t)(self->mf_end-self->mf_base);
 new_pos = (size_t)(self->mf_ptr-self->mf_base);
 switch (whence) {
 case SEEK_SET: new_pos  = *offset; break;
 case SEEK_CUR: new_pos += *offset; break;
 case SEEK_END: new_pos  = size-*offset; break;
 default: libc_error_throw(E_INVALID_ARGUMENT);
 }
 if unlikely(new_pos < 0)
    libc_error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_NEGATIVE_SEEK);
 if ((pos64_t)new_pos >= size)
     new_pos = (off64_t)(pos64_t)size;
 self->mf_ptr = self->mf_base+(size_t)new_pos;
 *offset = (off64_t)new_pos;
 return 0;
}
PRIVATE int LIBCCALL
memopen_close(struct memopen_file *__restrict self) {
 libc_free(self);
 return 0;
}


EXPORT(fmemopen,libc_fmemopen);
CRT_STDIO_API ATTR_MALLOC FILE *LIBCCALL
libc_fmemopen(void *buf, size_t len, char const *modes) {
 FILE *result; struct memopen_file *cookie;
 cookie_io_functions_t io_funcs;
 cookie = (struct memopen_file *)libc_malloc(sizeof(struct memopen_file));
 if unlikely(!cookie) return NULL;
 cookie->mf_base = (byte_t *)buf;
 cookie->mf_ptr  = (byte_t *)buf;
 cookie->mf_end  = (byte_t *)buf+len;
 io_funcs.cio_read  = (cookie_read_function_t *)&memopen_read;
 io_funcs.cio_write = (cookie_write_function_t *)&memopen_write;
 io_funcs.cio_seek  = (cookie_seek_function_t *)&memopen_seek;
 io_funcs.cio_close = (cookie_close_function_t *)&memopen_close;
 result = libc_fopencookie(cookie,modes,io_funcs);
 if unlikely(!result) { libc_free(cookie); goto done; }
 /* Special handling for append-mode. */
 if (libc_parsemode(modes) & O_APPEND)
     cookie->mf_ptr = (byte_t *)libc_strnend((char *)buf,len);
done:
 return result;
}

EXPORT(Xfmemopen,libc_Xfmemopen);
CRT_STDIO_XAPI ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL
libc_Xfmemopen(void *buf, size_t len, char const *modes) {
 FILE *COMPILER_IGNORE_UNINITIALIZED(result); struct memopen_file *cookie;
 cookie = (struct memopen_file *)libc_Xmalloc(sizeof(struct memopen_file));
 LIBC_TRY {
  cookie_io_functions_t io_funcs;
  cookie->mf_base = (byte_t *)buf;
  cookie->mf_ptr  = (byte_t *)buf;
  cookie->mf_end  = (byte_t *)buf+len;
  io_funcs.cio_read  = (cookie_read_function_t *)&memopen_read;
  io_funcs.cio_write = (cookie_write_function_t *)&memopen_write;
  io_funcs.cio_seek  = (cookie_seek_function_t *)&memopen_seek;
  io_funcs.cio_close = (cookie_close_function_t *)&memopen_close;
  result = libc_Xfopencookie(cookie,modes,io_funcs);
  /* Special handling for append-mode. */
  if (libc_parsemode(modes) & O_APPEND)
      cookie->mf_ptr = (byte_t *)libc_strnend((char *)buf,len);
 } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  libc_free(cookie);
  error_rethrow();
 }
 return result;
}



struct memstream_file {
    byte_t **mf_pbase; /* Pointer to the user-defined base field. */
    size_t  *mf_psize; /* Pointer to the user-defined size field. */
    byte_t  *mf_base;  /* [0..1][owned] Allocated base pointer. */
    byte_t  *mf_ptr;   /* [0..1] Current read/write pointer (May be located beyond `mf_end'; allocated lazily during writes). */
    byte_t  *mf_end;   /* [0..1] Allocated buffer end pointer. */
};

PRIVATE ssize_t LIBCCALL
memstream_read(struct memstream_file *__restrict self,
               char *buffer, size_t bufsize) {
 size_t maxread = self->mf_end-self->mf_ptr;
 if (maxread > bufsize)
     maxread = bufsize;
 libc_memcpy(buffer,self->mf_ptr,maxread);
 self->mf_ptr += maxread;
 return (ssize_t)maxread;
}
PRIVATE ssize_t LIBCCALL
memstream_write(struct memstream_file *__restrict self,
                char const *buffer, size_t bufsize) {
 size_t new_alloc,result = 0;
 byte_t *new_buffer;
 if likely(self->mf_ptr < self->mf_end) {
  result = self->mf_end-self->mf_ptr;
  if (result > bufsize)
      result = bufsize;
  libc_memcpy(self->mf_ptr,buffer,bufsize);
  self->mf_ptr += result;
  buffer       += result;
  bufsize      -= result;
 }
 if (!bufsize) goto done;
 /* Allocate more memory. */
 new_alloc  = (size_t)(self->mf_ptr-self->mf_base);
 /* XXX: Use the x86 `into' instruction? */
 if unlikely(__builtin_add_overflow(new_alloc,bufsize,&new_alloc))
    error_throw(E_OVERFLOW);
 new_buffer = (byte_t *)libc_Xrealloc(self->mf_base,
                                     (new_alloc+1)*sizeof(char));
 self->mf_ptr  = new_buffer+(self->mf_ptr-self->mf_base);
 self->mf_base = new_buffer;
 self->mf_end  = new_buffer+new_alloc;
 /* Copy data into the new portion of the buffer. */
 libc_memcpy(self->mf_ptr,buffer,bufsize);
 *self->mf_end = 0; /* NUL-termination. */
 result += bufsize;
 /* Update the user-given pointer locations with buffer parameters. */
 *self->mf_pbase = self->mf_base;
 *self->mf_psize = (size_t)(self->mf_end-self->mf_base);
done:
 return result;
}
PRIVATE int LIBCCALL
memstream_seek(struct memstream_file *__restrict self,
             off64_t *offset, int whence) {
 off64_t new_pos;
 new_pos = (size_t)(self->mf_ptr-self->mf_base);
 switch (whence) {
 case SEEK_SET: new_pos  = *offset; break;
 case SEEK_CUR: new_pos += *offset; break;
 case SEEK_END: new_pos  = (size_t)(self->mf_end-self->mf_base)-*offset; break;
 default: libc_error_throw(E_INVALID_ARGUMENT);
 }
 if unlikely(new_pos < 0)
    libc_error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_NEGATIVE_SEEK);
 self->mf_ptr = self->mf_base+(size_t)new_pos;
 *offset = (off64_t)new_pos;
 return 0;
}
PRIVATE int LIBCCALL
memstream_close(struct memstream_file *__restrict self) {
 libc_free(self);
 return 0;
}


EXPORT(open_memstream,libc_open_memstream);
CRT_STDIO_API ATTR_MALLOC FILE *LIBCCALL
libc_open_memstream(char **bufloc, size_t *sizeloc) {
 FILE *result; struct memstream_file *cookie;
 cookie_io_functions_t io_funcs;
 cookie = (struct memstream_file *)libc_malloc(sizeof(struct memstream_file));
 if unlikely(!cookie) return NULL;
 cookie->mf_pbase   = (byte_t **)bufloc;
 cookie->mf_psize   = sizeloc;
 cookie->mf_base    = NULL;
 cookie->mf_ptr     = NULL;
 cookie->mf_end     = NULL;
 io_funcs.cio_read  = (cookie_read_function_t *)&memstream_read;
 io_funcs.cio_write = (cookie_write_function_t *)&memstream_write;
 io_funcs.cio_seek  = (cookie_seek_function_t *)&memstream_seek;
 io_funcs.cio_close = (cookie_close_function_t *)&memstream_close;
 result             = libc_fopencookie(cookie,"r+",io_funcs);
 if unlikely(!result) libc_free(cookie);
 return result;
}

EXPORT(Xopen_memstream,libc_Xopen_memstream);
CRT_STDIO_XAPI ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL
libc_Xopen_memstream(char **bufloc, size_t *sizeloc) {
 FILE *COMPILER_IGNORE_UNINITIALIZED(result); struct memstream_file *cookie;
 cookie = (struct memstream_file *)libc_Xmalloc(sizeof(struct memstream_file));
 LIBC_TRY {
  cookie_io_functions_t io_funcs;
  cookie->mf_pbase   = (byte_t **)bufloc;
  cookie->mf_psize   = sizeloc;
  cookie->mf_base    = NULL;
  cookie->mf_ptr     = NULL;
  cookie->mf_end     = NULL;
  io_funcs.cio_read  = (cookie_read_function_t *)&memstream_read;
  io_funcs.cio_write = (cookie_write_function_t *)&memstream_write;
  io_funcs.cio_seek  = (cookie_seek_function_t *)&memstream_seek;
  io_funcs.cio_close = (cookie_close_function_t *)&memstream_close;
  result             = libc_Xfopencookie(cookie,"r+",io_funcs);
 } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  libc_free(cookie);
  error_rethrow();
 }
 return result;
}



struct process_file {
    fd_t     pf_file;  /* Calling process end of the communication pipe. */
    pid_t    pf_cpid;  /* Child process ID. */
};

PRIVATE ssize_t LIBCCALL
processstream_read(struct process_file *__restrict self,
                   char *buffer, size_t bufsize) {
 return libc_Xread(self->pf_file,buffer,bufsize);
}
PRIVATE ssize_t LIBCCALL
processstream_write(struct process_file *__restrict self,
                char const *buffer, size_t bufsize) {
 return libc_Xwrite(self->pf_file,buffer,bufsize);
}
PRIVATE int LIBCCALL
processstream_close(struct process_file *__restrict self) {
 int result;
 /* Wait for the child process to exit. */
 if (self->pf_cpid >= 0) {
  while (libc_waitpid(self->pf_cpid,&result,WEXITED) < 0) {
   if (libc_geterrno() != EINTR) {
    result = -1;
    break;
   }
  }
 }
 libc_free(self);
 return result;
}


EXPORT(__KSYM(popen),libc_popen);
CRT_STDIO_API ATTR_MALLOC FILE *LIBCCALL
libc_popen(char const *command, char const *modes) {
 LIBC_TRY {
  return libc_Xpopen(command,modes);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return NULL;
}

EXPORT(Xpopen,libc_Xpopen);
CRT_STDIO_XAPI ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL
libc_Xpopen(char const *command, char const *modes) {
 FILE *COMPILER_IGNORE_UNINITIALIZED(result); struct process_file *cookie;
 cookie = (struct process_file *)libc_Xmalloc(sizeof(struct process_file));
 LIBC_TRY {
  cookie_io_functions_t io_funcs;
  cookie->pf_file = -1;
  cookie->pf_cpid = -1;
  COMPILER_WRITE_BARRIER();
  io_funcs.cio_read  = (cookie_read_function_t *)&processstream_read;
  io_funcs.cio_write = (cookie_write_function_t *)&processstream_write;
  io_funcs.cio_close = (cookie_close_function_t *)&processstream_close;
  io_funcs.cio_seek  = NULL;
  result = libc_Xfopencookie_impl(cookie,modes,io_funcs);
  LIBC_TRY {
   /* Create the communications pipe. */
   fd_t pipedes[2]; oflag_t flags; pid_t cpid;
   flags = libc_parsemode(modes);
   /* Make sure that the caller didn't request both read & write
    * permissions (which is impossible and would require 2 buffers,
    * whereas FILEs only have one buffer) */
   if ((flags & O_ACCMODE) == O_RDWR)
        error_throw(E_INVALID_ARGUMENT);
   libc_Xpipe2(pipedes,flags);
   /* Pipes have been created. - Now to spawn the child process! */
   if ((cpid = libc_Xfork()) == 0) {
    /* Connect the pipes. */
    if ((flags & O_ACCMODE) == O_RDONLY) {
     /* Calling process wants to read. -> Connect writer end to stdout & stderr. */
     libc_dup2(pipedes[1],STDOUT_FILENO);
     libc_dup2(pipedes[1],STDERR_FILENO);
    } else {
     /* Calling process wants to write. -> Connect reader end to stdin. */
     libc_dup2(pipedes[0],STDIN_FILENO);
    }
    /* Execute the given command in a shell. */
    libc_execl(libc_path_bin_sh,libc_str_sh,libc_str_dashc,command,NULL);
    libc_execl(libc_path_bin_busybox,libc_str_sh,libc_str_dashc,command,NULL);
    _libc_exit(127);
   }
   /* Save the child PID. */
   cookie->pf_cpid = cpid;
   /* Save our end of the pipe. */
   if ((flags & O_ACCMODE) == O_RDONLY) {
    /* We want to read. */
    sys_close(pipedes[1]);
    cookie->pf_file = pipedes[0];
    result->fb_ops.cio_write = NULL;
   } else {
    /* We want to write. */
    sys_close(pipedes[0]);
    cookie->pf_file = pipedes[1];
    result->fb_ops.cio_read = NULL;
   }
  } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
   FileBuffer_Unregister(result);
   if (cookie->pf_file >= 0) sys_close(cookie->pf_file);
   FileBuffer_Free(result);
   error_rethrow();
  }
 } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  libc_free(cookie);
  error_rethrow();
 }
 FileBuffer_Register(result);
 return result;
}

EXPORT(__DSYM(_popen),libc_dos_popen);
DEFINE_INTERN_ALIAS(libc_dos_popen,libc_popen);

EXPORT(fclose,libc_fclose);
EXPORT(fclose_unlocked,libc_fclose); /* No unlocked version for this! */
EXPORT(__DSYM(_fclose_nolock),libc_fclose); /* No unlocked version for this! */
CRT_STDIO_API int LIBCCALL
libc_fclose(FILE *__restrict self) {
 return FileBuffer_Decref(self);
}

EXPORT(pclose,libc_pclose);
EXPORT(__DSYM(_pclose),libc_pclose);
CRT_STDIO_API int LIBCCALL
libc_pclose(FILE *__restrict self) {
 pid_t cpid; int result;
 if (ATOMIC_DECIFONE((self)->fb_refcnt))
     return FileBuffer_Destroy(self);
 /* Manually wait for the child process _now_. */
 cpid = ATOMIC_XCH(((struct process_file *)self->fb_arg)->pf_cpid,-1);
 while (libc_waitpid(cpid,&result,WEXITED) < 0) {
  if (libc_geterrno() != EINTR) { result = -1; break; }
 }
 /* Drop our remaining reference. */
 FileBuffer_Decref(self);
 return result;
}


#undef putc
#undef Xputc
#undef putc_unlocked
#undef Xputc_unlocked

EXPORT(putc,libc_fputc);
EXPORT(fputc,libc_fputc);
CRT_STDIO_API int LIBCCALL
libc_fputc(int c, FILE *__restrict self) {
 char ch = (char)c;
 return FileBuffer_Write(self,&ch,sizeof(ch)) ? 0 : EOF;
}

EXPORT(Xputc,libc_Xfputc);
EXPORT(Xfputc,libc_Xfputc);
CRT_STDIO_XAPI int LIBCCALL
libc_Xfputc(int c, FILE *__restrict self) {
 char ch = (char)c;
 return FileBuffer_XWrite(self,&ch,sizeof(ch)) ? 0 : EOF;
}

EXPORT(putc_unlocked,libc_fputc_unlocked);
EXPORT(fputc_unlocked,libc_fputc_unlocked);
CRT_STDIO_API int LIBCCALL
libc_fputc_unlocked(int c, FILE *__restrict self) {
 char ch = (char)c;
 return FileBuffer_WriteUnlocked(self,&ch,sizeof(ch)) ? 0 : EOF;
}

EXPORT(Xputc_unlocked,libc_Xfputc_unlocked);
EXPORT(Xfputc_unlocked,libc_Xfputc_unlocked);
CRT_STDIO_XAPI int LIBCCALL
libc_Xfputc_unlocked(int c, FILE *__restrict self) {
 char ch = (char)c;
 return FileBuffer_XWriteUnlocked(self,&ch,sizeof(ch)) ? 0 : EOF;
}





EXPORT(getw,libc_getw);
EXPORT(__DSYM(_getw),libc_getw);
CRT_STDIO_API int LIBCCALL
libc_getw(FILE *__restrict self) {
 char bytes[2],ch;
 ch = libc_fgetc(self);
 if unlikely(ch == EOF) return EOF;
 bytes[0] = (char)ch;
 ch = libc_fgetc(self);
 if unlikely(ch == EOF) return EOF;
 bytes[1] = (char)ch;
 return (int)*(u16 *)bytes;
}

EXPORT(Xgetw,libc_Xgetw);
CRT_STDIO_XAPI int LIBCCALL
libc_Xgetw(FILE *__restrict self) {
 char bytes[2],ch;
 ch = libc_Xfgetc(self);
 if unlikely(ch == EOF) return EOF;
 bytes[0] = (char)ch;
 ch = libc_Xfgetc(self);
 if unlikely(ch == EOF) return EOF;
 bytes[1] = (char)ch;
 return (int)*(u16 *)bytes;
}

EXPORT(putw,libc_putw);
EXPORT(__DSYM(_putw),libc_putw);
CRT_STDIO_API int LIBCCALL
libc_putw(int w, FILE *__restrict self) {
 u16 word = (u16)(s16)w;
 return FileBuffer_Write(self,&word,2) ? 0 : EOF;
}

EXPORT(Xputw,libc_Xputw);
CRT_STDIO_XAPI int LIBCCALL
libc_Xputw(int w, FILE *__restrict self) {
 u16 word = (u16)(s16)w;
 return FileBuffer_XWrite(self,&word,2) ? 0 : EOF;
}



EXPORT(getw_unlocked,libc_getw_unlocked);
CRT_STDIO_API int LIBCCALL
libc_getw_unlocked(FILE *__restrict self) {
 char bytes[2],ch;
 ch = libc_fgetc_unlocked(self);
 if unlikely(ch == EOF) return EOF;
 bytes[0] = (char)ch;
 ch = libc_fgetc_unlocked(self);
 if unlikely(ch == EOF) return EOF;
 bytes[1] = (char)ch;
 return (int)*(u16 *)bytes;
}

EXPORT(Xgetw_unlocked,libc_Xgetw_unlocked);
CRT_STDIO_XAPI int LIBCCALL
libc_Xgetw_unlocked(FILE *__restrict self) {
 char bytes[2],ch;
 ch = libc_Xfgetc_unlocked(self);
 if unlikely(ch == EOF) return EOF;
 bytes[0] = (char)ch;
 ch = libc_Xfgetc_unlocked(self);
 if unlikely(ch == EOF) return EOF;
 bytes[1] = (char)ch;
 return (int)*(u16 *)bytes;
}

EXPORT(putw_unlocked,libc_putw_unlocked);
CRT_STDIO_API int LIBCCALL
libc_putw_unlocked(int w, FILE *__restrict self) {
 u16 word = (u16)(s16)w;
 return FileBuffer_WriteUnlocked(self,&word,2) ? 0 : EOF;
}

EXPORT(Xputw_unlocked,libc_Xputw_unlocked);
CRT_STDIO_XAPI int LIBCCALL
libc_Xputw_unlocked(int w, FILE *__restrict self) {
 u16 word = (u16)(s16)w;
 return FileBuffer_XWriteUnlocked(self,&word,2) ? 0 : EOF;
}


EXPORT(fputs,libc_fputs);
CRT_STDIO_API ssize_t LIBCCALL
libc_fputs(char const *__restrict s, FILE *__restrict self) {
 size_t result = FileBuffer_Write(self,s,libc_strlen(s)*sizeof(char));
 return result ? (ssize_t)result : EOF;
}

EXPORT(Xfputs,libc_Xfputs);
CRT_STDIO_XAPI ssize_t LIBCCALL
libc_Xfputs(char const *__restrict s, FILE *__restrict self) {
 size_t result = FileBuffer_XWrite(self,s,libc_strlen(s)*sizeof(char));
 return result ? (ssize_t)result : EOF;
}

EXPORT(fputs_unlocked,libc_fputs_unlocked);
CRT_STDIO_API ssize_t LIBCCALL
libc_fputs_unlocked(char const *__restrict s, FILE *__restrict self) {
 size_t result = FileBuffer_WriteUnlocked(self,s,libc_strlen(s)*sizeof(char));
 return result ? (ssize_t)result : EOF;
}

EXPORT(Xfputs_unlocked,libc_Xfputs_unlocked);
CRT_STDIO_XAPI ssize_t LIBCCALL
libc_Xfputs_unlocked(char const *__restrict s, FILE *__restrict self) {
 size_t result = FileBuffer_XWriteUnlocked(self,s,libc_strlen(s)*sizeof(char));
 return result ? (ssize_t)result : EOF;
}

EXPORT(clearerr,libc_clearerr);
CRT_STDIO_API void LIBCCALL
libc_clearerr(FILE *__restrict self) {
 LIBC_TRY {
  self->fb_flag &= ~FILE_BUFFER_FERR;
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
 }
}

EXPORT(fflush,libc_fflush);
CRT_STDIO_API int LIBCCALL libc_fflush(FILE *self) {
 if (!self) { FileBuffer_FlushAllBuffers(); return 0; }
 return FileBuffer_Flush(self);
}

EXPORT(Xfflush,libc_Xfflush);
CRT_STDIO_XAPI void LIBCCALL libc_Xfflush(FILE *self) {
 if (!self) FileBuffer_FlushAllBuffers();
 else FileBuffer_XFlush(self);
}

EXPORT(fflush_unlocked,libc_fflush_unlocked);
EXPORT(__DSYM(_fflush_nolock),libc_fflush_unlocked);
CRT_STDIO_API int LIBCCALL libc_fflush_unlocked(FILE *self) {
 if (!self) { FileBuffer_FlushAllBuffersUnlocked(); return 0; }
 return FileBuffer_FlushUnlocked(self);
}

EXPORT(Xfflush_unlocked,libc_Xfflush_unlocked);
CRT_STDIO_XAPI void LIBCCALL libc_Xfflush_unlocked(FILE *self) {
 if (!self) FileBuffer_FlushAllBuffersUnlocked();
 else FileBuffer_XFlushUnlocked(self);
}

EXPORT(setbuf,libc_setbuf);
CRT_STDIO_API void LIBCCALL
libc_setbuf(FILE *__restrict self, char *__restrict buf) {
 libc_setbuffer(self,buf,BUFSIZ);
}

EXPORT(setbuffer,libc_setbuffer);
CRT_STDIO_API void LIBCCALL
libc_setbuffer(FILE *__restrict self,
               char *__restrict buf, size_t size) {
 libc_setvbuf(self,buf,
              buf ? _IOFBF : _IONBF,
              buf ? size : 0);
}

EXPORT(setlinebuf,libc_setlinebuf);
CRT_STDIO_API void LIBCCALL
libc_setlinebuf(FILE *__restrict self) {
 libc_setvbuf(self,NULL,_IOLBF,0);
}

EXPORT(ungetc,libc_ungetc);
CRT_STDIO_API int LIBCCALL
libc_ungetc(int c, FILE *__restrict self) {
 return FileBuffer_Ungetc(self,c);
}

EXPORT(Xungetc,libc_Xungetc);
CRT_STDIO_XAPI int LIBCCALL
libc_Xungetc(int c, FILE *__restrict self) {
 return FileBuffer_XUngetc(self,c);
}

EXPORT(ungetc_unlocked,libc_ungetc_unlocked);
EXPORT(__DSYM(_ungetc_nolock),libc_ungetc_unlocked);
CRT_STDIO_API int LIBCCALL
libc_ungetc_unlocked(int c, FILE *__restrict self) {
 return FileBuffer_UngetcUnlocked(self,c);
}

EXPORT(Xungetc_unlocked,libc_Xungetc_unlocked);
CRT_STDIO_XAPI int LIBCCALL
libc_Xungetc_unlocked(int c, FILE *__restrict self) {
 return FileBuffer_XUngetcUnlocked(self,c);
}

EXPORT(getdelim_unlocked,libc_getdelim_unlocked);
CRT_STDIO_API ssize_t LIBCCALL
libc_getdelim_unlocked(char **__restrict lineptr,
                       size_t *__restrict n, int delimiter,
                       FILE *__restrict self) {
 int ch; char *buffer;
 size_t bufsize,result = 0;
 buffer = *lineptr;
 bufsize = buffer ? *n : 0;
 for (;;) {
  if (result+1 >= bufsize) {
   /* Allocate more memory. */
   size_t new_bufsize = bufsize*2;
   if (new_bufsize <= result+1)
       new_bufsize = 16;
   assert(new_bufsize > result+1);
   buffer = (char *)libc_realloc(buffer,new_bufsize*sizeof(char));
   if unlikely(!buffer) return -1;
   *lineptr = buffer,*n = bufsize;
  }
  ch = libc_Xfgetc_unlocked(self);
  if (ch == EOF) break; /* EOF */
  buffer[result++] = (char)ch;
  if (ch == delimiter)
      break; /* Delimiter reached */
  /* Special case for line-delimiter. */
  if (delimiter == '\n' && ch == '\r') {
   /* Deal with '\r\n', as well as '\r' */
   ch = libc_fgetc_unlocked(self);
   if (ch != EOF && ch != '\n')
       libc_ungetc_unlocked(ch,self);
   /* Unify linefeeds (to use POSIX notation) */
   buffer[result-1] = '\n';
   break;
  }
 }
 /* NUL-Terminate the buffer. */
 buffer[result] = '\0';
 return result;
}

EXPORT(Xgetdelim_unlocked,libc_Xgetdelim_unlocked);
CRT_STDIO_XAPI size_t LIBCCALL
libc_Xgetdelim_unlocked(char **__restrict lineptr,
                        size_t *__restrict n, int delimiter,
                        FILE *__restrict self) {
 int ch; char *buffer;
 size_t bufsize,result = 0;
 buffer = *lineptr;
 bufsize = buffer ? *n : 0;
 for (;;) {
  if (result+1 >= bufsize) {
   /* Allocate more memory. */
   size_t new_bufsize = bufsize*2;
   if (new_bufsize <= result+1)
       new_bufsize = 16;
   assert(new_bufsize > result+1);
   buffer = (char *)libc_Xrealloc(buffer,new_bufsize*sizeof(char));
   *lineptr = buffer,*n = bufsize;
  }
  ch = libc_Xfgetc_unlocked(self);
  if (ch == EOF) break; /* EOF */
  buffer[result++] = (char)ch;
  if (ch == delimiter)
      break; /* Delimiter reached */
  /* Special case for line-delimiter. */
  if (delimiter == '\n' && ch == '\r') {
   /* Deal with '\r\n', as well as '\r' */
   ch = libc_Xfgetc_unlocked(self);
   if (ch != EOF && ch != '\n')
       libc_Xungetc_unlocked(ch,self);
   /* Unify linefeeds (to use POSIX notation) */
   buffer[result-1] = '\n';
   break;
  }
 }
 /* NUL-Terminate the buffer. */
 buffer[result] = '\0';
 return result;
}

EXPORT(getdelim,libc_getdelim);
CRT_STDIO_API ssize_t LIBCCALL
libc_getdelim(char **__restrict lineptr,
              size_t *__restrict n, int delimiter,
              FILE *__restrict self) {
 ssize_t result;
 while (FileBuffer_Lock(self))
    if (libc_geterrno() != EINTR)
        return -1;
 result = libc_getdelim_unlocked(lineptr,n,delimiter,self);
 FileBuffer_Unlock(self);
 return result;
}

EXPORT(Xgetdelim,libc_Xgetdelim);
CRT_STDIO_XAPI size_t LIBCCALL
libc_Xgetdelim(char **__restrict lineptr,
               size_t *__restrict n, int delimiter,
               FILE *__restrict self) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(self);
 LIBC_TRY {
  result = libc_Xgetdelim_unlocked(lineptr,n,delimiter,self);
 } LIBC_FINALLY {
  FileBuffer_Unlock(self);
 }
 return result;
}


EXPORT(getline_unlocked,libc_getline_unlocked);
CRT_STDIO_API ssize_t LIBCCALL
libc_getline_unlocked(char **__restrict lineptr,
                      size_t *__restrict n,
                      FILE *__restrict self) {
 return libc_getdelim_unlocked(lineptr,n,'\n',self);
}
EXPORT(Xgetline_unlocked,libc_Xgetline_unlocked);
CRT_STDIO_XAPI size_t LIBCCALL
libc_Xgetline_unlocked(char **__restrict lineptr,
                       size_t *__restrict n,
                       FILE *__restrict self) {
 return libc_Xgetdelim_unlocked(lineptr,n,'\n',self);
}

EXPORT(getline,libc_getline);
CRT_STDIO_API ssize_t LIBCCALL
libc_getline(char **__restrict lineptr,
             size_t *__restrict n,
             FILE *__restrict self) {
 return libc_getdelim(lineptr,n,'\n',self);
}
EXPORT(Xgetline,libc_Xgetline);
CRT_STDIO_XAPI size_t LIBCCALL
libc_Xgetline(char **__restrict lineptr,
              size_t *__restrict n,
              FILE *__restrict self) {
 return libc_Xgetdelim(lineptr,n,'\n',self);
}


EXPORT(feof,libc_feof);
EXPORT(feof_unlocked,libc_feof); /* ??? Why? (with `fseek()' is WEAK(ATOMIC) anyways) */
CRT_STDIO_API int LIBCCALL
libc_feof(FILE *__restrict self) {
 LIBC_TRY {
  return (self->fb_flag & FILE_BUFFER_FEOF) != 0;
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}

EXPORT(ferror,libc_ferror);
EXPORT(ferror_unlocked,libc_ferror); /* ??? Why? (with `clearerr()' is WEAK(ATOMIC) anyways) */
CRT_STDIO_API int LIBCCALL
libc_ferror(FILE *__restrict self) {
 LIBC_TRY {
  return (self->fb_flag & FILE_BUFFER_FERR) != 0;
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}

EXPORT(fileno,libc_fileno);
EXPORT(fileno_unlocked,libc_fileno); /* ??? Why? (Once the function returns, it's undefined if the handle is still valid in any case...) */
EXPORT(__DSYM(_fileno),libc_fileno);
CRT_STDIO_API fd_t LIBCCALL
libc_fileno(FILE *__restrict self) {
 fd_t COMPILER_IGNORE_UNINITIALIZED(result);
 LIBC_TRY {
  result = ATOMIC_READ(self->fb_file);
  if unlikely(result < 0)
     libc_seterrno(EBADF),
     result = -1;
 } LIBC_EXCEPT(libc_except_errno()) {
  result = -1;
 }
 return result;
}

EXPORT(rewind,libc_rewind);
CRT_STDIO_API void LIBCCALL
libc_rewind(FILE *__restrict self) {
 LIBC_TRY {
  FileBuffer_Seek(self,0,SEEK_SET);
 } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
 }
}

EXPORT(rewind_unlocked,libc_rewind_unlocked);
CRT_STDIO_API void LIBCCALL
libc_rewind_unlocked(FILE *__restrict self) {
 LIBC_TRY {
  FileBuffer_SeekUnlocked(self,0,SEEK_SET);
 } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
 }
}

EXPORT(fgets_unlocked,libc_fgets_unlocked);
CRT_STDIO_API char *LIBCCALL
libc_fgets_unlocked(char *__restrict s, size_t n,
                    FILE *__restrict self) {
 char *dst = s;
 /* Subtract one from the buffer to ensure
  * space for a trailing NUL-character. */
 if unlikely(!n--) return s;
 for (; n; --n) {
  int ch = libc_fgetc_unlocked(self);
  /* Stop on EOF */
  if (ch == EOF) {
   if (dst == s) return NULL; /* EOF */
   break;
  }
  *dst++ = ch;
  /* Stop on linefeed. */
  if (ch == '\n') break;
  if (ch == '\r') {
   ch = libc_fgetc_unlocked(self);
   if (ch != '\n' && ch != EOF)
       libc_ungetc_unlocked(ch,self);
   break;
  }
 }
 /* Terminate the given buffer. */
 *dst = 0;
 return s;
}

EXPORT(Xfgets_unlocked,libc_Xfgets_unlocked);
CRT_STDIO_XAPI char *LIBCCALL
libc_Xfgets_unlocked(char *__restrict s, size_t n,
                     FILE *__restrict self) {
 char *dst = s;
 /* Subtract one from the buffer to ensure
  * space for a trailing NUL-character. */
 if unlikely(!n--) return s;
 for (; n; --n) {
  int ch = libc_Xfgetc_unlocked(self);
  /* Stop on EOF */
  if (ch == EOF) {
   if (dst == s) return NULL; /* EOF */
   break;
  }
  *dst++ = ch;
  /* Stop on linefeed. */
  if (ch == '\n') break;
  if (ch == '\r') {
   ch = libc_Xfgetc_unlocked(self);
   if (ch != '\n' && ch != EOF)
       libc_Xungetc_unlocked(ch,self);
   break;
  }
 }
 /* Terminate the given buffer. */
 *dst = 0;
 return s;
}

EXPORT(fgets,libc_fgets);
CRT_STDIO_API char *LIBCCALL
libc_fgets(char *__restrict s, size_t n,
           FILE *__restrict self) {
 char *result;
 while (FileBuffer_Lock(self))
    if (libc_geterrno() != EINTR)
        return NULL;
 result = libc_fgets_unlocked(s,n,self);
 FileBuffer_Unlock(self);
 return result;
}

EXPORT(Xfgets,libc_Xfgets);
CRT_STDIO_XAPI char *LIBCCALL
libc_Xfgets(char *__restrict s, size_t n,
            FILE *__restrict self) {
 char *COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(self);
 LIBC_TRY {
  result = libc_Xfgets_unlocked(s,n,self);
 } LIBC_FINALLY {
  FileBuffer_Unlock(self);
 }
 return result;
}



#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
EXPORT(fgets,libc_fgets);
EXPORT(Xfgets,libc_Xfgets);
EXPORT(fgets_unlocked,libc_fgets_unlocked);
EXPORT(Xfgets_unlocked,libc_Xfgets_unlocked);
DEFINE_INTERN_ALIAS(libc_fgets_int,libc_fgets);
DEFINE_INTERN_ALIAS(libc_Xfgets_int,libc_Xfgets);
DEFINE_INTERN_ALIAS(libc_fgets_int_unlocked,libc_fgets_unlocked);
DEFINE_INTERN_ALIAS(libc_Xfgets_int_unlocked,libc_Xfgets_unlocked);
#else
EXPORT(fgets,libc_fgets_int);
EXPORT(Xfgets,libc_Xfgets_int);
EXPORT(fgets_unlocked,libc_fgets_int_unlocked);
EXPORT(Xfgets_unlocked,libc_Xfgets_int_unlocked);
EXPORT(fgets_sz,libc_fgets);
EXPORT(Xfgets_sz,libc_Xfgets);
EXPORT(fgets_sz_unlocked,libc_fgets_unlocked);
EXPORT(Xfgets_sz_unlocked,libc_Xfgets_unlocked);

CRT_STDIO_API char *LIBCCALL
libc_fgets_int(char *__restrict s,
               unsigned int n,
               FILE *__restrict self) {
 return libc_fgets(s,n,self);
}
CRT_STDIO_XAPI char *LIBCCALL
libc_Xfgets_int(char *__restrict s,
                unsigned int n,
                FILE *__restrict self) {
 return libc_Xfgets(s,n,self);
}
CRT_STDIO_API char *LIBCCALL
libc_fgets_int_unlocked(char *__restrict s,
                        unsigned int n,
                        FILE *__restrict self) {
 return libc_fgets_unlocked(s,n,self);
}
CRT_STDIO_XAPI char *LIBCCALL
libc_Xfgets_int_unlocked(char *__restrict s,
                         unsigned int n,
                         FILE *__restrict self) {
 return libc_Xfgets_unlocked(s,n,self);
}
#endif


/* File functions for operating on std streams. */
EXPORT(getchar,libc_getchar);
EXPORT(__DSYM(_fgetchar),libc_getchar);
CRT_STDIO_API int LIBCCALL libc_getchar(void) {
 return libc_fgetc(libc_stdin);
}
EXPORT(Xgetchar,libc_Xgetchar);
CRT_STDIO_XAPI int LIBCCALL libc_Xgetchar(void) {
 return libc_Xfgetc(libc_stdin);
}

EXPORT(getchar_unlocked,libc_getchar_unlocked);
CRT_STDIO_API int LIBCCALL libc_getchar_unlocked(void) {
 return libc_fgetc_unlocked(libc_stdin);
}
EXPORT(Xgetchar_unlocked,libc_Xgetchar_unlocked);
CRT_STDIO_XAPI int LIBCCALL libc_Xgetchar_unlocked(void) {
 return libc_Xfgetc_unlocked(libc_stdin);
}

EXPORT(putchar,libc_putchar);
EXPORT(__DSYM(_fputchar),libc_putchar);
CRT_STDIO_API int LIBCCALL libc_putchar(int c) {
 return libc_fputc(c,libc_stdout);
}
EXPORT(Xputchar,libc_Xputchar);
CRT_STDIO_XAPI int LIBCCALL libc_Xputchar(int c) {
 return libc_Xfputc(c,libc_stdout);
}

EXPORT(putchar_unlocked,libc_putchar_unlocked);
CRT_STDIO_API int LIBCCALL libc_putchar_unlocked(int c) {
 return libc_fputc_unlocked(c,libc_stdout);
}
EXPORT(Xputchar_unlocked,libc_Xputchar_unlocked);
CRT_STDIO_XAPI int LIBCCALL libc_Xputchar_unlocked(int c) {
 return libc_Xfputc_unlocked(c,libc_stdout);
}

EXPORT(gets,libc_gets);
CRT_STDIO_API char *LIBCCALL libc_gets(char *__restrict s) {
 /* WARNING: no buffer limit checks! */
 return libc_fgets(s,(size_t)-1,stdin);
}

EXPORT(puts,libc_puts);
CRT_STDIO_API ssize_t LIBCCALL
libc_puts(char const *__restrict s) {
 ssize_t result;
 while (FileBuffer_Lock(libc_stdout))
    if (libc_geterrno() != EINTR)
        return -1;
 result = libc_puts_unlocked(s);
 FileBuffer_Unlock(libc_stdout);
 return result;
}

EXPORT(Xputs,libc_Xputs);
CRT_STDIO_XAPI size_t LIBCCALL
libc_Xputs(char const *__restrict s) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(libc_stdout);
 LIBC_TRY {
  result = libc_Xputs_unlocked(s);
 } LIBC_FINALLY {
  FileBuffer_Unlock(libc_stdout);
 }
 return result;
}


EXPORT(puts_unlocked,libc_puts_unlocked);
CRT_STDIO_API ssize_t LIBCCALL
libc_puts_unlocked(char const *__restrict s) {
 ssize_t result;
 result = libc_fputs_unlocked(s,libc_stdout);
 if (result >= 0) {
  if unlikely(libc_fputc_unlocked('\n',libc_stdout) == EOF)
     result = EOF-1;
  ++result;
 }
 return result;
}

EXPORT(Xputs_unlocked,libc_Xputs_unlocked);
CRT_STDIO_XAPI size_t LIBCCALL
libc_Xputs_unlocked(char const *__restrict s) {
 size_t result;
 result = libc_Xfputs_unlocked(s,libc_stdout);
 if unlikely(libc_Xfputc_unlocked('\n',libc_stdout) == EOF)
    result = EOF-1;
 ++result;
 return result;
}


/* DOS file functions. */
EXPORT(__DSYM(_flushall),libc_fflushall);
CRT_DOS int LIBCCALL libc_fflushall(void) {
 FileBuffer_FlushAllBuffers();
 return 0;
}


EXPORT(__DSYM(_flsbuf),libc_fputc_unlocked);
EXPORT(__DSYM(_filbuf),libc_fgetc_unlocked);

//EXPORT(_fsopen,libc_dos_fsopen);
//EXPORT(_getmaxstdio,libc_getmaxstdio);
//EXPORT(_setmaxstdio,libc_setmaxstdio);
//EXPORT(_get_printf_count_output,libc_get_printf_count_output);
//EXPORT(_set_printf_count_output,libc_set_printf_count_output);
//EXPORT(_get_output_format,libc_get_output_format);
//EXPORT(_set_output_format,libc_set_output_format);
//EXPORT(_rmtmp,libc_rmtmp);
//EXPORT(__KSYM(fopen_s),libc_fopen_s);
//EXPORT(__DSYM(fopen_s),libc_dos_fopen_s);
//EXPORT(__KSYM(freopen_s),libc_freopen_s);
//EXPORT(__DSYM(freopen_s),libc_dos_freopen_s);
//EXPORT(__KSYM(tmpfile_s),libc_tmpfile_s);
//EXPORT(__DSYM(tmpfile_s),libc_dos_tmpfile_s);
//EXPORT(clearerr_s,libc_clearerr_s);
//EXPORT(gets_s,libc_gets_s);


/* DOS's way of accessing std files. */
CRT_DOS FILE *LIBCCALL libc_p_iob(void) { return libc_std_files; }
CRT_DOS FILE *LIBCCALL libc_acrt_iob_func(unsigned int id) { return &libc_std_files[id]; }
EXPORT(_iob,libc_std_files);
EXPORT(__p__iob,libc_p_iob);
EXPORT(__iob_func,libc_p_iob);
EXPORT(__acrt_iob_func,libc_acrt_iob_func);



DECL_END

#endif /* CONFIG_LIBC_USES_NEW_STDIO */

#endif /* !GUARD_LIBS_LIBC_STDIO_API_C */
