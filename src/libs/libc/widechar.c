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
#ifndef GUARD_LIBS_LIBC_WIDECHAR_C
#define GUARD_LIBS_LIBC_WIDECHAR_C 1

#include "widechar.h"
#include "unicode.h"
#include "malloc.h"
#include "unistd.h"
#include "vm.h"
#include "errno.h"
#include "system.h"
#include "sched.h"

#include <bits/mbstate.h>
#include <bits/dos-errno.h>
#include <unicode.h>
#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

DECL_BEGIN

CRT_WIDECHAR char *LIBCCALL
libc_loadutf16(char buf[UTF_STACK_BUFFER_SIZE], char16_t const *string) {
 size_t buflen,reqlen,length = libc_w16len(string);
 mbstate_t state = MBSTATE_INIT; char *result;
 reqlen = libc_utf16to8(string,length,buf,
                        UTF_STACK_BUFFER_SIZE,
                       &state,UNICODE_F_SETERRNO);
 if unlikely(reqlen == UNICODE_ERROR) return NULL;
 if (reqlen <= UTF_STACK_BUFFER_SIZE) return buf;
 result = NULL,buflen = 0;
 for (;;) {
  char *new_result = (char *)libc_realloc(result,reqlen*sizeof(char));
  if unlikely(!new_result) {err: libc_free(result); return NULL; }
  result = new_result;
  if unlikely(reqlen <= buflen) break;
  buflen = reqlen;
  mbstate_reset(&state);
  reqlen = libc_utf16to8(string,length,result,buflen,
                        &state,UNICODE_F_SETERRNO);
  if unlikely(reqlen == UNICODE_ERROR) goto err;
 }
 return result;
}
CRT_WIDECHAR char *LIBCCALL
libc_loadutf32(char buf[UTF_STACK_BUFFER_SIZE], char32_t const *string) {
 size_t buflen,reqlen,length = libc_w32len(string);
 mbstate_t state = MBSTATE_INIT; char *result;
 reqlen = libc_utf32to8(string,length,buf,
                        UTF_STACK_BUFFER_SIZE,
                       &state,UNICODE_F_SETERRNO);
 if unlikely(reqlen == UNICODE_ERROR) return NULL;
 if (reqlen <= UTF_STACK_BUFFER_SIZE) return buf;
 result = NULL,buflen = 0;
 for (;;) {
  char *new_result = (char *)libc_realloc(result,reqlen*sizeof(char));
  if unlikely(!new_result) {err: libc_free(result); return NULL; }
  result = new_result;
  if unlikely(reqlen <= buflen) break;
  buflen = reqlen;
  mbstate_reset(&state);
  reqlen = libc_utf32to8(string,length,result,buflen,
                        &state,UNICODE_F_SETERRNO);
  if unlikely(reqlen == UNICODE_ERROR) goto err;
 }
 return result;
}

CRT_WIDECHAR_EXCEPT char *LIBCCALL
libc_Xloadutf16(char buf[UTF_STACK_BUFFER_SIZE], char16_t const *string) {
 size_t buflen,reqlen,length = libc_w16len(string);
 mbstate_t state = MBSTATE_INIT; char *result;
 reqlen = libc_Xutf16to8(string,length,buf,
                         UTF_STACK_BUFFER_SIZE,
                        &state,UNICODE_F_SETERRNO);
 if (reqlen <= UTF_STACK_BUFFER_SIZE) return buf;
 result = NULL,buflen = 0;
 LIBC_TRY {
  for (;;) {
   result = (char *)libc_Xrealloc(result,reqlen*sizeof(char));
   if unlikely(reqlen <= buflen) break;
   buflen = reqlen;
   mbstate_reset(&state);
   reqlen = libc_Xutf16to8(string,length,result,buflen,
                          &state,UNICODE_F_SETERRNO);
  }
 } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  libc_free(result);
  error_rethrow();
 }
 return result;
}
CRT_WIDECHAR_EXCEPT char *LIBCCALL
libc_Xloadutf32(char buf[UTF_STACK_BUFFER_SIZE], char32_t const *string) {
 size_t buflen,reqlen,length = libc_w32len(string);
 mbstate_t state = MBSTATE_INIT; char *result;
 reqlen = libc_Xutf32to8(string,length,buf,
                         UTF_STACK_BUFFER_SIZE,
                        &state,UNICODE_F_SETERRNO);
 if (reqlen <= UTF_STACK_BUFFER_SIZE) return buf;
 result = NULL,buflen = 0;
 LIBC_TRY {
  for (;;) {
   result = (char *)libc_Xrealloc(result,reqlen*sizeof(char));
   if unlikely(reqlen <= buflen) break;
   buflen = reqlen;
   mbstate_reset(&state);
   reqlen = libc_Xutf32to8(string,length,result,buflen,
                          &state,UNICODE_F_SETERRNO);
  }
 } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  libc_free(result);
  error_rethrow();
 }
 return result;
}

CRT_WIDECHAR void LIBCCALL
libc_freeutf(char buf[UTF_STACK_BUFFER_SIZE], char *str) {
 if (str != buf)
     libc_free(str);
}



EXPORT(__KSYMw16(shm_wopen),libc_shm_w16open);
CRT_WIDECHAR int LIBCCALL
libc_shm_w16open(char16_t const *name, oflag_t oflag, mode_t mode) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,name);
 if (!str) return -1;
 result = libc_shm_open(str,oflag,mode);
 libc_freeutf(buf,str);
 return result;
}
EXPORT(__KSYMw32(shm_wopen),libc_shm_w32open);
CRT_WIDECHAR int LIBCCALL
libc_shm_w32open(char32_t const *name, oflag_t oflag, mode_t mode) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,name);
 if (!str) return -1;
 result = libc_shm_open(str,oflag,mode);
 libc_freeutf(buf,str);
 return result;
}
EXPORT(__DSYMw16(shm_wopen),libc_dos_shm_w16open);
CRT_WIDECHAR int LIBCCALL
libc_dos_shm_w16open(char16_t const *name, oflag_t oflag, mode_t mode) {
 return libc_shm_w16open(name,oflag|O_DOSPATH,mode);
}
EXPORT(__DSYMw32(shm_wopen),libc_dos_shm_w32open);
CRT_WIDECHAR int LIBCCALL
libc_dos_shm_w32open(char32_t const *name, oflag_t oflag, mode_t mode) {
 return libc_shm_w32open(name,oflag|O_DOSPATH,mode);
}

EXPORT(__SYMw16(Xshm_wopen),libc_Xshm_w16open);
CRT_WIDECHAR_EXCEPT int LIBCCALL
libc_Xshm_w16open(char16_t const *name, oflag_t oflag, mode_t mode) {
 char buf[UTF_STACK_BUFFER_SIZE];
 int COMPILER_IGNORE_UNINITIALIZED(result);
 char *str = libc_Xloadutf16(buf,name);
 LIBC_TRY {
  result = libc_Xshm_open(str,oflag,mode);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
 return result;
}
EXPORT(__SYMw32(Xshm_wopen),libc_Xshm_w32open);
CRT_WIDECHAR_EXCEPT int LIBCCALL
libc_Xshm_w32open(char32_t const *name, oflag_t oflag, mode_t mode) {
 char buf[UTF_STACK_BUFFER_SIZE];
 int COMPILER_IGNORE_UNINITIALIZED(result);
 char *str = libc_Xloadutf32(buf,name);
 LIBC_TRY {
  result = libc_Xshm_open(str,oflag,mode);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
 return result;
}


EXPORT(__KSYMw16(shm_wunlink),libc_shm_w16unlink);
CRT_WIDECHAR int LIBCCALL
libc_shm_w16unlink(char16_t const *name) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,name);
 if (!str) return -1;
 result = libc_shm_unlink(str);
 libc_freeutf(buf,str);
 return result;
}
EXPORT(__KSYMw32(shm_wunlink),libc_shm_w32unlink);
CRT_WIDECHAR int LIBCCALL
libc_shm_w32unlink(char32_t const *name) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,name);
 if (!str) return -1;
 result = libc_shm_unlink(str);
 libc_freeutf(buf,str);
 return result;
}
EXPORT(__DSYMw16(shm_wunlink),libc_dos_shm_w16unlink);
CRT_WIDECHAR int LIBCCALL
libc_dos_shm_w16unlink(char16_t const *name) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,name);
 if (!str) return -1;
 result = libc_dos_shm_unlink(str);
 libc_freeutf(buf,str);
 return result;
}
EXPORT(__DSYMw32(shm_wunlink),libc_dos_shm_w32unlink);
CRT_WIDECHAR int LIBCCALL
libc_dos_shm_w32unlink(char32_t const *name) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,name);
 if (!str) return -1;
 result = libc_dos_shm_unlink(str);
 libc_freeutf(buf,str);
 return result;
}

EXPORT(__SYMw16(Xshm_wunlink),libc_Xshm_w16unlink);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xshm_w16unlink(char16_t const *name) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(buf,name);
 LIBC_TRY {
  libc_Xshm_unlink(str);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}
EXPORT(__SYMw32(Xshm_wunlink),libc_Xshm_w32unlink);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xshm_w32unlink(char32_t const *name) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(buf,name);
 LIBC_TRY {
  libc_Xshm_unlink(str);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}

EXPORT(__KSYMw16(wopenat),libc_w16openat);
EXPORT(__KSYMw16(wopenat64),libc_w16openat);
CRT_WIDECHAR int ATTR_CDECL
libc_w16openat(fd_t dfd, char16_t const *filename, oflag_t flags, ...) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result; va_list args;
 char *str = libc_loadutf16(buf,filename);
 if (!str) return -1;
 va_start(args,flags);
 result = libc_openat(dfd,str,flags,va_arg(args,mode_t));
 va_end(args);
 libc_freeutf(buf,str);
 return result;
}
EXPORT(__KSYMw32(wopenat),libc_w32openat);
EXPORT(__KSYMw32(wopenat64),libc_w32openat);
CRT_WIDECHAR int ATTR_CDECL
libc_w32openat(fd_t dfd, char32_t const *filename, oflag_t flags, ...) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result; va_list args;
 char *str = libc_loadutf32(buf,filename);
 if (!str) return -1;
 va_start(args,flags);
 result = libc_openat(dfd,str,flags,va_arg(args,mode_t));
 va_end(args);
 libc_freeutf(buf,str);
 return result;
}
EXPORT(__DSYMw16(wopenat),libc_dos_w16openat);
EXPORT(__DSYMw16(wopenat64),libc_dos_w16openat);
CRT_WIDECHAR int ATTR_CDECL
libc_dos_w16openat(fd_t dfd, char16_t const *filename, oflag_t flags, ...) {
 int result; va_list args;
 va_start(args,flags);
 result = libc_w16openat(dfd,filename,O_DOSPATH|flags,va_arg(args,mode_t));
 va_end(args);
 return result;
}
EXPORT(__DSYMw32(wopenat),libc_dos_w32openat);
EXPORT(__DSYMw32(wopenat64),libc_dos_w32openat);
CRT_WIDECHAR int ATTR_CDECL
libc_dos_w32openat(fd_t dfd, char32_t const *filename, oflag_t flags, ...) {
 int result; va_list args;
 va_start(args,flags);
 result = libc_w32openat(dfd,filename,O_DOSPATH|flags,va_arg(args,mode_t));
 va_end(args);
 return result;
}
EXPORT(__KSYMw16(wopen),libc_w16open);
EXPORT(__KSYMw16(wopen64),libc_w16open);
CRT_WIDECHAR int ATTR_CDECL
libc_w16open(char16_t const *filename, oflag_t flags, ...) {
 int result; va_list args;
 va_start(args,flags);
 result = libc_w16openat(AT_FDCWD,filename,flags,va_arg(args,mode_t));
 va_end(args);
 return result;
}
EXPORT(__KSYMw32(wopen),libc_w32open);
EXPORT(__KSYMw32(wopen64),libc_w32open);
CRT_WIDECHAR int ATTR_CDECL
libc_w32open(char32_t const *filename, oflag_t flags, ...) {
 int result; va_list args;
 va_start(args,flags);
 result = libc_w32openat(AT_FDCWD,filename,flags,va_arg(args,mode_t));
 va_end(args);
 return result;
}
EXPORT(__DSYMw16(_wopen),libc_dos_w16open);
EXPORT(__DSYMw16(wopen64),libc_dos_w16open);
EXPORT("DOS$?_wopen%%YAHPEB_WHH%Z",libc_dos_w16open);
CRT_WIDECHAR int ATTR_CDECL
libc_dos_w16open(char16_t const *filename, oflag_t flags, ...) {
 int result; va_list args;
 va_start(args,flags);
 result = libc_w16openat(AT_FDCWD,filename,O_DOSPATH|flags,va_arg(args,mode_t));
 va_end(args);
 return result;
}
EXPORT(__DSYMw32(wopen),libc_dos_w32open);
EXPORT(__DSYMw32(wopen64),libc_dos_w32open);
CRT_WIDECHAR int ATTR_CDECL
libc_dos_w32open(char32_t const *filename, oflag_t flags, ...) {
 int result; va_list args;
 va_start(args,flags);
 result = libc_w32openat(AT_FDCWD,filename,O_DOSPATH|flags,va_arg(args,mode_t));
 va_end(args);
 return result;
}
EXPORT(__KSYMw16(wcreat),libc_w16creat);
EXPORT(__KSYMw16(wcreat64),libc_w16creat);
CRT_WIDECHAR int LIBCCALL libc_w16creat(char16_t const *file, mode_t mode) {
 return libc_w16openat(AT_FDCWD,file,O_CREAT|O_WRONLY|O_TRUNC,mode);
}
EXPORT(__KSYMw32(wcreat),libc_w32creat);
EXPORT(__KSYMw32(wcreat64),libc_w32creat);
CRT_WIDECHAR int LIBCCALL libc_w32creat(char32_t const *file, mode_t mode) {
 return libc_w32openat(AT_FDCWD,file,O_CREAT|O_WRONLY|O_TRUNC,mode);
}
EXPORT(__DSYMw16(_wcreat),libc_dos_w16creat);
EXPORT(__DSYMw16(wcreat64),libc_dos_w16creat);
CRT_WIDECHAR int LIBCCALL libc_dos_w16creat(char16_t const *file, mode_t mode) {
 return libc_w16openat(AT_FDCWD,file,O_DOSPATH|O_CREAT|O_WRONLY|O_TRUNC,mode);
}
EXPORT(__DSYMw32(wcreat),libc_dos_w32creat);
EXPORT(__DSYMw32(wcreat64),libc_dos_w32creat);
CRT_WIDECHAR int LIBCCALL libc_dos_w32creat(char32_t const *file, mode_t mode) {
 return libc_w32openat(AT_FDCWD,file,O_DOSPATH|O_CREAT|O_WRONLY|O_TRUNC,mode);
}


EXPORT(__SYMw16(Xwopenat),libc_Xw16openat);
EXPORT(__SYMw16(Xwopenat64),libc_Xw16openat);
CRT_WIDECHAR_EXCEPT int ATTR_CDECL
libc_Xw16openat(fd_t dfd, char16_t const *filename, oflag_t flags, ...) {
 char buf[UTF_STACK_BUFFER_SIZE]; va_list args;
 int COMPILER_IGNORE_UNINITIALIZED(result);
 char *str = libc_Xloadutf16(buf,filename);
 va_start(args,flags);
 LIBC_TRY {
  result = libc_Xopenat(dfd,str,flags,va_arg(args,mode_t));
 } LIBC_FINALLY {
  va_end(args);
  libc_freeutf(buf,str);
 }
 return result;
}
EXPORT(__SYMw32(Xwopenat),libc_Xw32openat);
EXPORT(__SYMw32(Xwopenat64),libc_Xw32openat);
CRT_WIDECHAR_EXCEPT int ATTR_CDECL
libc_Xw32openat(fd_t dfd, char32_t const *filename, oflag_t flags, ...) {
 char buf[UTF_STACK_BUFFER_SIZE]; va_list args;
 int COMPILER_IGNORE_UNINITIALIZED(result);
 char *str = libc_Xloadutf32(buf,filename);
 va_start(args,flags);
 LIBC_TRY {
  result = libc_Xopenat(dfd,str,flags,va_arg(args,mode_t));
 } LIBC_FINALLY {
  va_end(args);
  libc_freeutf(buf,str);
 }
 return result;
}
EXPORT(__SYMw16(Xwopen),libc_Xw16open);
EXPORT(__SYMw16(Xwopen64),libc_Xw16open);
CRT_WIDECHAR_EXCEPT int ATTR_CDECL
libc_Xw16open(char16_t const *filename, oflag_t flags, ...) {
 int COMPILER_IGNORE_UNINITIALIZED(result);
 va_list __EXCEPTVAR_VALIST args;
 va_start(args,flags);
 __TRY_VALIST {
  result = libc_Xw16openat(AT_FDCWD,filename,flags,va_arg(args,mode_t));
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}
EXPORT(__SYMw32(Xwopen),libc_Xw32open);
EXPORT(__SYMw32(Xwopen64),libc_Xw32open);
CRT_WIDECHAR_EXCEPT int ATTR_CDECL
libc_Xw32open(char32_t const *filename, oflag_t flags, ...) {
 int COMPILER_IGNORE_UNINITIALIZED(result);
 va_list __EXCEPTVAR_VALIST args;
 va_start(args,flags);
 __TRY_VALIST {
  result = libc_Xw32openat(AT_FDCWD,filename,flags,va_arg(args,mode_t));
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}
EXPORT(__SYMw16(Xwcreat),libc_Xw16creat);
EXPORT(__SYMw16(Xwcreat64),libc_Xw16creat);
CRT_WIDECHAR_EXCEPT int LIBCCALL libc_Xw16creat(char16_t const *file, mode_t mode) {
 return libc_Xw16openat(AT_FDCWD,file,O_CREAT|O_WRONLY|O_TRUNC,mode);
}
EXPORT(__SYMw32(Xwcreat),libc_Xw32creat);
EXPORT(__SYMw32(Xwcreat64),libc_Xw32creat);
CRT_WIDECHAR int LIBCCALL libc_Xw32creat(char32_t const *file, mode_t mode) {
 return libc_Xw32openat(AT_FDCWD,file,O_CREAT|O_WRONLY|O_TRUNC,mode);
}



EXPORT(__KSYMw16(wfexecveat),libc_w16fexecveat);
CRT_WIDECHAR int LIBCCALL
libc_w16fexecveat(fd_t dfd,
                  char16_t const *path,
                  char16_t *const argv[],
                  char16_t *const envp[],
                  int flags) {
 LIBC_TRY {
  libc_Xw16fexecveat(dfd,path,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw16(wfexecvpeat),libc_w16fexecvpeat);
CRT_WIDECHAR int LIBCCALL
libc_w16fexecvpeat(char16_t const *file,
                   char16_t *const argv[],
                   char16_t *const envp[],
                   int flags) {
 LIBC_TRY {
  libc_Xw16fexecvpeat(file,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw16(wfspawnveat),libc_w16fspawnveat);
CRT_WIDECHAR pid_t LIBCCALL
libc_w16fspawnveat(int mode, fd_t dfd,
                   char16_t const *path,
                   char16_t *const argv[],
                   char16_t *const envp[],
                   int flags) {
 LIBC_TRY {
  return libc_Xw16fspawnveat(mode,dfd,path,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw16(wfspawnvpeat),libc_w16fspawnvpeat);
CRT_WIDECHAR pid_t LIBCCALL
libc_w16fspawnvpeat(int mode,
                    char16_t const *file,
                    char16_t *const argv[],
                    char16_t *const envp[],
                    int flags) {
 LIBC_TRY {
  return libc_Xw16fspawnvpeat(mode,file,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw32(wfexecveat),libc_w32fexecveat);
CRT_WIDECHAR int LIBCCALL
libc_w32fexecveat(fd_t dfd,
                  char32_t const *path,
                  char32_t *const argv[],
                  char32_t *const envp[],
                  int flags) {
 LIBC_TRY {
  libc_Xw32fexecveat(dfd,path,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw32(wfexecvpeat),libc_w32fexecvpeat);
CRT_WIDECHAR int LIBCCALL
libc_w32fexecvpeat(char32_t const *file,
                   char32_t *const argv[],
                   char32_t *const envp[],
                   int flags) {
 LIBC_TRY {
  libc_Xw32fexecvpeat(file,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw32(wfspawnveat),libc_w32fspawnveat);
CRT_WIDECHAR pid_t LIBCCALL
libc_w32fspawnveat(int mode, fd_t dfd,
                   char32_t const *path,
                   char32_t *const argv[],
                   char32_t *const envp[],
                   int flags) {
 LIBC_TRY {
  return libc_Xw32fspawnveat(mode,dfd,path,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}
EXPORT(__KSYMw32(wfspawnvpeat),libc_w32fspawnvpeat);
CRT_WIDECHAR pid_t LIBCCALL
libc_w32fspawnvpeat(int mode,
                    char32_t const *file,
                    char32_t *const argv[],
                    char32_t *const envp[],
                    int flags) {
 LIBC_TRY {
  return libc_Xw32fspawnvpeat(mode,file,argv,envp,flags);
 } LIBC_EXCEPT(libc_except_errno()) {
 }
 return -1;
}


EXPORT(__SYMw16(Xwfexecveat),libc_Xw16fexecveat);
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL
libc_Xw16fexecveat(fd_t dfd,
                   char16_t const *path,
                   char16_t *const argv[],
                   char16_t *const envp[],
                   int flags) {
 /* TODO */
 libc_error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(__SYMw16(Xwfexecvpeat),libc_Xw16fexecvpeat);
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL
libc_Xw16fexecvpeat(char16_t const *file,
                    char16_t *const argv[],
                    char16_t *const envp[],
                    int flags) {
 /* TODO */
 libc_error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(__SYMw16(Xwfspawnveat),libc_Xw16fspawnveat);
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL
libc_Xw16fspawnveat(int mode, fd_t dfd,
                    char16_t const *path,
                    char16_t *const argv[],
                    char16_t *const envp[],
                    int flags) {
 /* TODO */
 libc_error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(__SYMw16(Xwfspawnvpeat),libc_Xw16fspawnvpeat);
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL
libc_Xw16fspawnvpeat(int mode,
                     char16_t const *file,
                     char16_t *const argv[],
                     char16_t *const envp[],
                     int flags) {
 /* TODO: Search $PATH */
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(__SYMw32(Xwfexecveat),libc_Xw32fexecveat);
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL
libc_Xw32fexecveat(fd_t dfd,
                  char32_t const *path,
                  char32_t *const argv[],
                  char32_t *const envp[],
                  int flags) {
 /* TODO */
 libc_error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(__SYMw32(Xwfexecvpeat),libc_Xw32fexecvpeat);
CRT_WIDECHAR_EXCEPT ATTR_NORETURN void LIBCCALL
libc_Xw32fexecvpeat(char32_t const *file,
                    char32_t *const argv[],
                    char32_t *const envp[],
                    int flags) {
 /* TODO: Search $PATH */
 libc_error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(__SYMw32(Xwfspawnveat),libc_Xw32fspawnveat);
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL
libc_Xw32fspawnveat(int mode, fd_t dfd,
                    char32_t const *path,
                    char32_t *const argv[],
                    char32_t *const envp[],
                    int flags) {
 /* TODO */
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(__SYMw32(Xwfspawnvpeat),libc_Xw32fspawnvpeat);
CRT_WIDECHAR_EXCEPT pid_t LIBCCALL
libc_Xw32fspawnvpeat(int mode,
                     char32_t const *file,
                     char32_t *const argv[],
                     char32_t *const envp[],
                     int flags) {
 /* TODO: Search $PATH */
 libc_error_throw(E_NOT_IMPLEMENTED);
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
 char *temp = NULL,tempbuf[UTF_STACK_BUFFER_SIZE];
 char *path8,pathbuf[UTF_STACK_BUFFER_SIZE];
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
 char *temp = NULL,tempbuf[UTF_STACK_BUFFER_SIZE];
 char *path8,pathbuf[UTF_STACK_BUFFER_SIZE];
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



EXPORT(__KSYMw16(wchdir),libc_w16chdir);
CRT_WIDECHAR int LIBCCALL
libc_w16chdir(char16_t const *path) {
 return libc_w16fchdirat(AT_FDCWD,path,0);
}

EXPORT(__KSYMw32(wchdir),libc_w32chdir);
CRT_WIDECHAR int LIBCCALL
libc_w32chdir(char32_t const *path) {
 return libc_w32fchdirat(AT_FDCWD,path,0);
}

EXPORT(__DSYMw16(_wchdir),libc_dos_w16chdir);
CRT_WIDECHAR int LIBCCALL
libc_dos_w16chdir(char16_t const *path) {
 return libc_dos_w16fchdirat(AT_FDCWD,path,AT_DOSPATH);
}

EXPORT(__DSYMw32(wchdir),libc_dos_w32chdir);
CRT_WIDECHAR int LIBCCALL
libc_dos_w32chdir(char32_t const *path) {
 return libc_dos_w32fchdirat(AT_FDCWD,path,AT_DOSPATH);
}

EXPORT(__KSYMw16(wfchdirat),libc_w16fchdirat);
CRT_WIDECHAR int LIBCCALL
libc_w16fchdirat(fd_t dfd, char16_t const *path, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,path);
 if (!str) return -1;
 result = libc_fchdirat(dfd,str,flags);
 libc_freeutf(buf,str);
 return result;
}

EXPORT(__KSYMw32(wfchdirat),libc_w32fchdirat);
CRT_WIDECHAR int LIBCCALL
libc_w32fchdirat(fd_t dfd, char32_t const *path, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,path);
 if (!str) return -1;
 result = libc_fchdirat(dfd,str,flags);
 libc_freeutf(buf,str);
 return result;
}

EXPORT(__DSYMw16(wfchdirat),libc_dos_w16fchdirat);
CRT_WIDECHAR int LIBCCALL
libc_dos_w16fchdirat(fd_t dfd, char16_t const *path, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,path);
 if (!str) return -1;
 result = libc_fchdirat(dfd,str,flags|AT_DOSPATH);
 libc_freeutf(buf,str);
 return result;
}

EXPORT(__DSYMw32(wfchdirat),libc_dos_w32fchdirat);
CRT_WIDECHAR int LIBCCALL
libc_dos_w32fchdirat(fd_t dfd, char32_t const *path, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,path);
 if (!str) return -1;
 result = libc_fchdirat(dfd,str,flags|AT_DOSPATH);
 libc_freeutf(buf,str);
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

EXPORT(__SYMw16(Xwchdir),libc_Xw16chdir);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16chdir(char16_t const *path) {
 libc_Xw16fchdirat(AT_FDCWD,path,0);
}

EXPORT(__SYMw32(Xwchdir),libc_Xw32chdir);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32chdir(char32_t const *path) {
 libc_Xw32fchdirat(AT_FDCWD,path,0);
}

EXPORT(__SYMw16(Xwfchdirat),libc_Xw16fchdirat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16fchdirat(fd_t dfd, char16_t const *path, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(buf,path);
 LIBC_TRY {
  libc_Xfchdirat(dfd,str,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}

EXPORT(__SYMw32(Xwfchdirat),libc_Xw32fchdirat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32fchdirat(fd_t dfd, char32_t const *path, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(buf,path);
 LIBC_TRY {
  libc_Xfchdirat(dfd,str,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
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


EXPORT(__KSYMw16(wpathconf),libc_w16pathconf);
CRT_WIDECHAR long int LIBCCALL
libc_w16pathconf(char16_t const *path, int name) {
 return libc_w16fpathconfat(AT_FDCWD,path,name,0);
}
EXPORT(__KSYMw32(wpathconf),libc_w32pathconf);
CRT_WIDECHAR long int LIBCCALL
libc_w32pathconf(char32_t const *path, int name) {
 return libc_w32fpathconfat(AT_FDCWD,path,name,0);
}
EXPORT(__DSYMw16(wpathconf),libc_dos_w16pathconf);
CRT_WIDECHAR long int LIBCCALL
libc_dos_w16pathconf(char16_t const *path, int name) {
 return libc_w16fpathconfat(AT_FDCWD,path,name,AT_DOSPATH);
}
EXPORT(__DSYMw32(wpathconf),libc_dos_w32pathconf);
CRT_WIDECHAR long int LIBCCALL
libc_dos_w32pathconf(char32_t const *path, int name) {
 return libc_w32fpathconfat(AT_FDCWD,path,name,AT_DOSPATH);
}
EXPORT(__KSYMw16(wfpathconfat),libc_w16fpathconfat);
CRT_WIDECHAR long int LIBCCALL
libc_w16fpathconfat(fd_t dfd, char16_t const *file, int name, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; long int result;
 char *str = libc_loadutf16(buf,file);
 if (!str) return -1;
 result = libc_fpathconfat(dfd,str,name,flags);
 libc_freeutf(buf,str);
 return result;
}
EXPORT(__KSYMw32(wfpathconfat),libc_w32fpathconfat);
CRT_WIDECHAR long int LIBCCALL
libc_w32fpathconfat(fd_t dfd, char32_t const *file, int name, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; long int result;
 char *str = libc_loadutf32(buf,file);
 if (!str) return -1;
 result = libc_fpathconfat(dfd,str,name,flags);
 libc_freeutf(buf,str);
 return result;
}
EXPORT(__DSYMw16(wfpathconfat),libc_dos_w16fpathconfat);
CRT_WIDECHAR long int LIBCCALL
libc_dos_w16fpathconfat(fd_t dfd, char16_t const *file, int name, int flags) {
 return libc_w16fpathconfat(dfd,file,name,flags|AT_DOSPATH);
}
EXPORT(__DSYMw32(wfpathconfat),libc_dos_w32fpathconfat);
CRT_WIDECHAR long int LIBCCALL
libc_dos_w32fpathconfat(fd_t dfd, char32_t const *file, int name, int flags) {
 return libc_w32fpathconfat(dfd,file,name,flags|AT_DOSPATH);
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

EXPORT(__SYMw16(Xwpathconf),libc_Xw16pathconf);
CRT_WIDECHAR_EXCEPT long int LIBCCALL
libc_Xw16pathconf(char16_t const *path, int name) {
 return libc_Xw16fpathconfat(AT_FDCWD,path,name,0);
}
EXPORT(__SYMw32(Xwpathconf),libc_Xw32pathconf);
CRT_WIDECHAR_EXCEPT long int LIBCCALL
libc_Xw32pathconf(char32_t const *path, int name) {
 return libc_Xw32fpathconfat(AT_FDCWD,path,name,0);
}
EXPORT(__SYMw16(Xwfpathconfat),libc_Xw16fpathconfat);
CRT_WIDECHAR_EXCEPT long int LIBCCALL
libc_Xw16fpathconfat(fd_t dfd, char16_t const *file, int name, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 long int COMPILER_IGNORE_UNINITIALIZED(result);
 char *str = libc_Xloadutf16(buf,file);
 LIBC_TRY {
  result = libc_Xfpathconfat(dfd,str,name,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
 return result;
}
EXPORT(__SYMw32(Xwfpathconfat),libc_Xw32fpathconfat);
CRT_WIDECHAR_EXCEPT long int LIBCCALL
libc_Xw32fpathconfat(fd_t dfd, char32_t const *file, int name, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 long int COMPILER_IGNORE_UNINITIALIZED(result);
 char *str = libc_Xloadutf32(buf,file);
 LIBC_TRY {
  result = libc_Xfpathconfat(dfd,str,name,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
 return result;
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



EXPORT(__KSYMw16(wchown),libc_w16chown);
EXPORT(__KSYMw32(wchown),libc_w32chown);
EXPORT(__DSYMw16(_wchown),libc_dos_w16chown);
EXPORT(__DSYMw32(wchown),libc_dos_w32chown);
CRT_WIDECHAR int LIBCCALL libc_w16chown(char16_t const *file, uid_t owner, gid_t group) { return libc_w16fchownat(AT_FDCWD,file,owner,group,0); }
CRT_WIDECHAR int LIBCCALL libc_w32chown(char32_t const *file, uid_t owner, gid_t group) { return libc_w32fchownat(AT_FDCWD,file,owner,group,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16chown(char16_t const *file, uid_t owner, gid_t group) { return libc_w16fchownat(AT_FDCWD,file,owner,group,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32chown(char32_t const *file, uid_t owner, gid_t group) { return libc_w32fchownat(AT_FDCWD,file,owner,group,AT_DOSPATH); }

EXPORT(__KSYMw16(wlchown),libc_w16lchown);
EXPORT(__KSYMw32(wlchown),libc_w32lchown);
EXPORT(__DSYMw16(wlchown),libc_dos_w16lchown);
EXPORT(__DSYMw32(wlchown),libc_dos_w32lchown);
CRT_WIDECHAR int LIBCCALL libc_w16lchown(char16_t const *file, uid_t owner, gid_t group) { return libc_w16fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR int LIBCCALL libc_w32lchown(char32_t const *file, uid_t owner, gid_t group) { return libc_w32fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16lchown(char16_t const *file, uid_t owner, gid_t group) { return libc_w16fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32lchown(char32_t const *file, uid_t owner, gid_t group) { return libc_w32fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }

EXPORT(__KSYMw16(wfchownat),libc_w16fchownat);
EXPORT(__KSYMw32(wfchownat),libc_w32fchownat);
EXPORT(__DSYMw16(wfchownat),libc_dos_w16fchownat);
EXPORT(__DSYMw32(wfchownat),libc_dos_w32fchownat);
CRT_WIDECHAR int LIBCCALL
libc_w16fchownat(fd_t dfd, char16_t const *file, uid_t owner, gid_t group, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,file);
 if (!str) return -1;
 result = libc_fchownat(dfd,str,owner,group,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_w32fchownat(fd_t dfd, char32_t const *file, uid_t owner, gid_t group, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,file);
 if (!str) return -1;
 result = libc_fchownat(dfd,str,owner,group,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w16fchownat(fd_t dfd, char16_t const *file, uid_t owner, gid_t group, int flags) {
 return libc_w16fchownat(dfd,file,owner,group,flags|AT_DOSPATH);
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w32fchownat(fd_t dfd, char32_t const *file, uid_t owner, gid_t group, int flags) {
 return libc_w32fchownat(dfd,file,owner,group,flags|AT_DOSPATH);
}

EXPORT(__KSYMw16(wchmod),libc_w16chmod);
EXPORT(__KSYMw32(wchmod),libc_w32chmod);
EXPORT(__DSYMw16(_wchmod),libc_dos_w16chmod);
EXPORT(__DSYMw32(wchmod),libc_dos_w32chmod);
CRT_WIDECHAR int LIBCCALL libc_w16chmod(char16_t const *file, mode_t mode) { return libc_w16fchmodat(AT_FDCWD,file,mode,0); }
CRT_WIDECHAR int LIBCCALL libc_w32chmod(char32_t const *file, mode_t mode) { return libc_w32fchmodat(AT_FDCWD,file,mode,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16chmod(char16_t const *file, mode_t mode) { return libc_w16fchmodat(AT_FDCWD,file,mode,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32chmod(char32_t const *file, mode_t mode) { return libc_w32fchmodat(AT_FDCWD,file,mode,AT_DOSPATH); }

EXPORT(__KSYMw16(wlchmod),libc_w16lchmod);
EXPORT(__KSYMw32(wlchmod),libc_w32lchmod);
EXPORT(__DSYMw16(wlchmod),libc_dos_w16lchmod);
EXPORT(__DSYMw32(wlchmod),libc_dos_w32lchmod);
CRT_WIDECHAR int LIBCCALL libc_w16lchmod(char16_t const *file, mode_t mode) { return libc_w16fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR int LIBCCALL libc_w32lchmod(char32_t const *file, mode_t mode) { return libc_w32fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16lchmod(char16_t const *file, mode_t mode) { return libc_w16fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32lchmod(char32_t const *file, mode_t mode) { return libc_w32fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }

EXPORT(__KSYMw16(wfchmodat),libc_w16fchmodat);
EXPORT(__KSYMw32(wfchmodat),libc_w32fchmodat);
EXPORT(__DSYMw16(wfchmodat),libc_dos_w16fchmodat);
EXPORT(__DSYMw32(wfchmodat),libc_dos_w32fchmodat);
CRT_WIDECHAR int LIBCCALL
libc_w16fchmodat(fd_t dfd, char16_t const *file, mode_t mode, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,file);
 if (!str) return -1;
 result = libc_fchmodat(dfd,str,mode,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_w32fchmodat(fd_t dfd, char32_t const *file, mode_t mode, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,file);
 if (!str) return -1;
 result = libc_fchmodat(dfd,str,mode,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w16fchmodat(fd_t dfd, char16_t const *file, mode_t mode, int flags) {
 return libc_w16fchmodat(dfd,file,mode,flags|AT_DOSPATH);
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w32fchmodat(fd_t dfd, char32_t const *file, mode_t mode, int flags) {
 return libc_w32fchmodat(dfd,file,mode,flags|AT_DOSPATH);
}

EXPORT(__KSYMw16(wlink),libc_w16link);
EXPORT(__KSYMw32(wlink),libc_w32link);
EXPORT(__DSYMw16(wlink),libc_dos_w16link);
EXPORT(__DSYMw32(wlink),libc_dos_w32link);
CRT_WIDECHAR int LIBCCALL libc_w16link(char16_t const *from, char16_t const *to) { return libc_w16linkat(AT_FDCWD,from,AT_FDCWD,to,0); }
CRT_WIDECHAR int LIBCCALL libc_w32link(char32_t const *from, char32_t const *to) { return libc_w32linkat(AT_FDCWD,from,AT_FDCWD,to,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16link(char16_t const *from, char16_t const *to) { return libc_w16linkat(AT_FDCWD,from,AT_FDCWD,to,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32link(char32_t const *from, char32_t const *to) { return libc_w32linkat(AT_FDCWD,from,AT_FDCWD,to,AT_DOSPATH); }

EXPORT(__KSYMw16(wlinkat),libc_w16linkat);
EXPORT(__KSYMw32(wlinkat),libc_w32linkat);
EXPORT(__DSYMw16(wlinkat),libc_dos_w16linkat);
EXPORT(__DSYMw32(wlinkat),libc_dos_w32linkat);
CRT_WIDECHAR int LIBCCALL
libc_w16linkat(int fromfd, char16_t const *from,
               int tofd, char16_t const *to, int flags) {
 int result;
 char *to_str,to_buf[UTF_STACK_BUFFER_SIZE];
 char *from_str,from_buf[UTF_STACK_BUFFER_SIZE];
 if ((from_str = libc_loadutf16(from_buf,from)) == NULL) return -1;
 if ((to_str = libc_loadutf16(to_buf,to)) == NULL) { libc_freeutf(from_buf,from_str); return -1; }
 result = libc_linkat(fromfd,from_str,tofd,to_str,flags);
 libc_freeutf(to_buf,to_str);
 libc_freeutf(from_buf,from_str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_w32linkat(int fromfd, char32_t const *from,
               int tofd, char32_t const *to, int flags) {
 int result;
 char *to_str,to_buf[UTF_STACK_BUFFER_SIZE];
 char *from_str,from_buf[UTF_STACK_BUFFER_SIZE];
 if ((from_str = libc_loadutf32(from_buf,from)) == NULL) return -1;
 if ((to_str = libc_loadutf32(to_buf,to)) == NULL) { libc_freeutf(from_buf,from_str); return -1; }
 result = libc_linkat(fromfd,from_str,tofd,to_str,flags);
 libc_freeutf(to_buf,to_str);
 libc_freeutf(from_buf,from_str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w16linkat(int fromfd, char16_t const *from,
                   int tofd, char16_t const *to, int flags) {
 return libc_w16linkat(fromfd,from,tofd,to,flags|AT_DOSPATH);
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w32linkat(int fromfd, char32_t const *from,
                   int tofd, char32_t const *to, int flags) {
 return libc_w32linkat(fromfd,from,tofd,to,flags|AT_DOSPATH);
}

EXPORT(__KSYMw16(wunlink),libc_w16unlink);
EXPORT(__KSYMw32(wunlink),libc_w32unlink);
EXPORT(__DSYMw16(_wunlink),libc_dos_w16unlink);
EXPORT(__DSYMw32(wunlink),libc_dos_w32unlink);
CRT_WIDECHAR int LIBCCALL libc_w16unlink(char16_t const *name) { return libc_w16unlinkat(AT_FDCWD,name,0); }
CRT_WIDECHAR int LIBCCALL libc_w32unlink(char32_t const *name) { return libc_w32unlinkat(AT_FDCWD,name,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16unlink(char16_t const *name) { return libc_w16unlinkat(AT_FDCWD,name,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32unlink(char32_t const *name) { return libc_w32unlinkat(AT_FDCWD,name,AT_DOSPATH); }

EXPORT(__KSYMw16(wunlinkat),libc_w16unlinkat);
EXPORT(__KSYMw32(wunlinkat),libc_w32unlinkat);
EXPORT(__DSYMw16(wunlinkat),libc_dos_w16unlinkat);
EXPORT(__DSYMw32(wunlinkat),libc_dos_w32unlinkat);
CRT_WIDECHAR int LIBCCALL
libc_w16unlinkat(fd_t dfd, char16_t const *name, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,name);
 if (!str) return -1;
 result = libc_unlinkat(dfd,str,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_w32unlinkat(fd_t dfd, char32_t const *name, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,name);
 if (!str) return -1;
 result = libc_unlinkat(dfd,str,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w16unlinkat(fd_t dfd, char16_t const *name, int flags) {
 return libc_w16unlinkat(dfd,name,flags|AT_DOSPATH);
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w32unlinkat(fd_t dfd, char32_t const *name, int flags) {
 return libc_w32unlinkat(dfd,name,flags|AT_DOSPATH);
}

EXPORT(__KSYMw16(wrmdir),libc_w16rmdir);
EXPORT(__KSYMw32(wrmdir),libc_w32rmdir);
EXPORT(__DSYMw16(_wrmdir),libc_dos_w16rmdir);
EXPORT(__DSYMw32(wrmdir),libc_dos_w32rmdir);
CRT_WIDECHAR int LIBCCALL libc_w16rmdir(char16_t const *name) { return libc_w16unlinkat(AT_FDCWD,name,AT_REMOVEDIR); }
CRT_WIDECHAR int LIBCCALL libc_w32rmdir(char32_t const *name) { return libc_w32unlinkat(AT_FDCWD,name,AT_REMOVEDIR); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16rmdir(char16_t const *name) { return libc_w16unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32rmdir(char32_t const *name) { return libc_w32unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_DOSPATH); }

EXPORT(__KSYMw16(wremove),libc_w16remove);
EXPORT(__KSYMw32(wremove),libc_w32remove);
EXPORT(__DSYMw16(_wremove),libc_dos_w16remove);
EXPORT(__DSYMw32(wremove),libc_dos_w32remove);
CRT_WIDECHAR int LIBCCALL libc_w16remove(char16_t const *name) { return libc_w16unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_REMOVEREG); }
CRT_WIDECHAR int LIBCCALL libc_w32remove(char32_t const *name) { return libc_w32unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_REMOVEREG); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16remove(char16_t const *name) { return libc_w16unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_REMOVEREG|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32remove(char32_t const *name) { return libc_w32unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_REMOVEREG|AT_DOSPATH); }

EXPORT(__KSYMw16(wremoveat),libc_w16removeat);
EXPORT(__KSYMw32(wremoveat),libc_w32removeat);
EXPORT(__DSYMw16(wremoveat),libc_dos_w16removeat);
EXPORT(__DSYMw32(wremoveat),libc_dos_w32removeat);
CRT_WIDECHAR int LIBCCALL libc_w16removeat(fd_t dfd, char16_t const *name) { return libc_w16unlinkat(dfd,name,AT_REMOVEDIR|AT_REMOVEREG); }
CRT_WIDECHAR int LIBCCALL libc_w32removeat(fd_t dfd, char32_t const *name) { return libc_w32unlinkat(dfd,name,AT_REMOVEDIR|AT_REMOVEREG); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16removeat(fd_t dfd, char16_t const *name) { return libc_w16unlinkat(dfd,name,AT_REMOVEDIR|AT_REMOVEREG|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32removeat(fd_t dfd, char32_t const *name) { return libc_w32unlinkat(dfd,name,AT_REMOVEDIR|AT_REMOVEREG|AT_DOSPATH); }

EXPORT(__KSYMw16(wrename),libc_w16rename);
EXPORT(__KSYMw32(wrename),libc_w32rename);
EXPORT(__DSYMw16(_wrename),libc_dos_w16rename);
EXPORT(__DSYMw32(wrename),libc_dos_w32rename);
CRT_WIDECHAR int LIBCCALL libc_w16rename(char16_t const *oldname, char16_t const *newname) { return libc_w16frenameat(AT_FDCWD,oldname,AT_FDCWD,newname,0); }
CRT_WIDECHAR int LIBCCALL libc_w32rename(char32_t const *oldname, char32_t const *newname) { return libc_w32frenameat(AT_FDCWD,oldname,AT_FDCWD,newname,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16rename(char16_t const *oldname, char16_t const *newname) { return libc_w16frenameat(AT_FDCWD,oldname,AT_FDCWD,newname,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32rename(char32_t const *oldname, char32_t const *newname) { return libc_w32frenameat(AT_FDCWD,oldname,AT_FDCWD,newname,AT_DOSPATH); }

EXPORT(__KSYMw16(wrenameat),libc_w16renameat);
EXPORT(__KSYMw32(wrenameat),libc_w32renameat);
EXPORT(__DSYMw16(wrenameat),libc_dos_w16renameat);
EXPORT(__DSYMw32(wrenameat),libc_dos_w32renameat);
CRT_WIDECHAR int LIBCCALL libc_w16renameat(fd_t oldfd, char16_t const *oldname, fd_t newfd, char16_t const *newname) { return libc_w16frenameat(oldfd,oldname,newfd,newname,0); }
CRT_WIDECHAR int LIBCCALL libc_w32renameat(fd_t oldfd, char32_t const *oldname, fd_t newfd, char32_t const *newname) { return libc_w32frenameat(oldfd,oldname,newfd,newname,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16renameat(fd_t oldfd, char16_t const *oldname, fd_t newfd, char16_t const *newname) { return libc_w16frenameat(oldfd,oldname,newfd,newname,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32renameat(fd_t oldfd, char32_t const *oldname, fd_t newfd, char32_t const *newname) { return libc_w32frenameat(oldfd,oldname,newfd,newname,AT_DOSPATH); }

EXPORT(__KSYMw16(wfrenameat),libc_w16frenameat);
EXPORT(__KSYMw32(wfrenameat),libc_w32frenameat);
EXPORT(__DSYMw16(wfrenameat),libc_dos_w16frenameat);
EXPORT(__DSYMw32(wfrenameat),libc_dos_w32frenameat);
CRT_WIDECHAR int LIBCCALL
libc_w16frenameat(fd_t oldfd, char16_t const *oldname,
                  fd_t newfd, char16_t const *newname, int flags) {
 int result;
 char *old_str,old_buf[UTF_STACK_BUFFER_SIZE];
 char *new_str,new_buf[UTF_STACK_BUFFER_SIZE];
 if ((old_str = libc_loadutf16(old_buf,oldname)) == NULL) return -1;
 if ((new_str = libc_loadutf16(new_buf,newname)) == NULL) { libc_freeutf(new_buf,new_str); return -1; }
 result = libc_frenameat(oldfd,old_str,newfd,new_str,flags);
 libc_freeutf(new_buf,new_str);
 libc_freeutf(old_buf,old_str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_w32frenameat(fd_t oldfd, char32_t const *oldname,
                  fd_t newfd, char32_t const *newname, int flags) {
 int result;
 char *old_str,old_buf[UTF_STACK_BUFFER_SIZE];
 char *new_str,new_buf[UTF_STACK_BUFFER_SIZE];
 if ((old_str = libc_loadutf32(old_buf,oldname)) == NULL) return -1;
 if ((new_str = libc_loadutf32(new_buf,newname)) == NULL) { libc_freeutf(new_buf,new_str); return -1; }
 result = libc_frenameat(oldfd,old_str,newfd,new_str,flags);
 libc_freeutf(new_buf,new_str);
 libc_freeutf(old_buf,old_str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w16frenameat(fd_t oldfd, char16_t const *oldname,
                      fd_t newfd, char16_t const *newname, int flags) {
 return libc_w16frenameat(oldfd,oldname,newfd,newname,flags|AT_DOSPATH);
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w32frenameat(fd_t oldfd, char32_t const *oldname,
                      fd_t newfd, char32_t const *newname, int flags) {
 return libc_w32frenameat(oldfd,oldname,newfd,newname,flags|AT_DOSPATH);
}

EXPORT(__KSYMw16(_wmkdir),libc_w16mkdir1);
EXPORT(__KSYMw32(_wmkdir),libc_w32mkdir1);
EXPORT(__DSYMw16(_wmkdir),libc_dos_w16mkdir1);
EXPORT(__DSYMw32(_wmkdir),libc_dos_w32mkdir1);
CRT_WIDECHAR int LIBCCALL libc_w16mkdir1(char16_t const *name) { return libc_w16fmkdirat(AT_FDCWD,name,0755,0); }
CRT_WIDECHAR int LIBCCALL libc_w32mkdir1(char32_t const *name) { return libc_w32fmkdirat(AT_FDCWD,name,0755,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16mkdir1(char16_t const *name) { return libc_w16fmkdirat(AT_FDCWD,name,0755,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32mkdir1(char32_t const *name) { return libc_w32fmkdirat(AT_FDCWD,name,0755,AT_DOSPATH); }

EXPORT(__KSYMw16(wmkdir),libc_w16mkdir);
EXPORT(__KSYMw32(wmkdir),libc_w32mkdir);
EXPORT(__DSYMw16(wmkdir),libc_dos_w16mkdir);
EXPORT(__DSYMw32(wmkdir),libc_dos_w32mkdir);
CRT_WIDECHAR int LIBCCALL libc_w16mkdir(char16_t const *name, mode_t mode) { return libc_w16fmkdirat(AT_FDCWD,name,mode,0); }
CRT_WIDECHAR int LIBCCALL libc_w32mkdir(char32_t const *name, mode_t mode) { return libc_w32fmkdirat(AT_FDCWD,name,mode,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16mkdir(char16_t const *name, mode_t mode) { return libc_w16fmkdirat(AT_FDCWD,name,mode,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32mkdir(char32_t const *name, mode_t mode) { return libc_w32fmkdirat(AT_FDCWD,name,mode,AT_DOSPATH); }

EXPORT(__KSYMw16(wmkdirat),libc_w16mkdirat);
EXPORT(__KSYMw32(wmkdirat),libc_w32mkdirat);
EXPORT(__DSYMw16(wmkdirat),libc_dos_w16mkdirat);
EXPORT(__DSYMw32(wmkdirat),libc_dos_w32mkdirat);
CRT_WIDECHAR int LIBCCALL libc_w16mkdirat(fd_t dfd, char16_t const *name, mode_t mode) { return libc_w16fmkdirat(dfd,name,mode,0); }
CRT_WIDECHAR int LIBCCALL libc_w32mkdirat(fd_t dfd, char32_t const *name, mode_t mode) { return libc_w32fmkdirat(dfd,name,mode,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16mkdirat(fd_t dfd, char16_t const *name, mode_t mode) { return libc_w16fmkdirat(dfd,name,mode,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32mkdirat(fd_t dfd, char32_t const *name, mode_t mode) { return libc_w32fmkdirat(dfd,name,mode,AT_DOSPATH); }

EXPORT(__KSYMw16(wfmkdirat),libc_w16fmkdirat);
EXPORT(__KSYMw32(wfmkdirat),libc_w32fmkdirat);
EXPORT(__DSYMw16(wfmkdirat),libc_dos_w16fmkdirat);
EXPORT(__DSYMw32(wfmkdirat),libc_dos_w32fmkdirat);
CRT_WIDECHAR int LIBCCALL
libc_w16fmkdirat(fd_t dfd, char16_t const *name, mode_t mode, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,name);
 if (!str) return -1;
 result = libc_fmkdirat(dfd,str,mode,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_w32fmkdirat(fd_t dfd, char32_t const *name, mode_t mode, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,name);
 if (!str) return -1;
 result = libc_fmkdirat(dfd,str,mode,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w16fmkdirat(fd_t dfd, char16_t const *name, mode_t mode, int flags) {
 return libc_w16fmkdirat(dfd,name,mode,flags|AT_DOSPATH);
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w32fmkdirat(fd_t dfd, char32_t const *name, mode_t mode, int flags) {
 return libc_w32fmkdirat(dfd,name,mode,flags|AT_DOSPATH);
}

EXPORT(__KSYMw16(wsymlink),libc_w16symlink);
EXPORT(__KSYMw32(wsymlink),libc_w32symlink);
EXPORT(__DSYMw16(wsymlink),libc_dos_w16symlink);
EXPORT(__DSYMw32(wsymlink),libc_dos_w32symlink);
CRT_WIDECHAR int LIBCCALL libc_w16symlink(char16_t const *from, char16_t const *to) { return libc_w16fsymlinkat(from,AT_FDCWD,to,0); }
CRT_WIDECHAR int LIBCCALL libc_w32symlink(char32_t const *from, char32_t const *to) { return libc_w32fsymlinkat(from,AT_FDCWD,to,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16symlink(char16_t const *from, char16_t const *to) { return libc_w16fsymlinkat(from,AT_FDCWD,to,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32symlink(char32_t const *from, char32_t const *to) { return libc_w32fsymlinkat(from,AT_FDCWD,to,AT_DOSPATH); }

EXPORT(__KSYMw16(wsymlinkat),libc_w16symlinkat);
EXPORT(__KSYMw32(wsymlinkat),libc_w32symlinkat);
EXPORT(__DSYMw16(wsymlinkat),libc_dos_w16symlinkat);
EXPORT(__DSYMw32(wsymlinkat),libc_dos_w32symlinkat);
CRT_WIDECHAR int LIBCCALL libc_w16symlinkat(char16_t const *from, int tofd, char16_t const *to) { return libc_w16fsymlinkat(from,tofd,to,0); }
CRT_WIDECHAR int LIBCCALL libc_w32symlinkat(char32_t const *from, int tofd, char32_t const *to) { return libc_w32fsymlinkat(from,tofd,to,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16symlinkat(char16_t const *from, int tofd, char16_t const *to) { return libc_w16fsymlinkat(from,tofd,to,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32symlinkat(char32_t const *from, int tofd, char32_t const *to) { return libc_w32fsymlinkat(from,tofd,to,AT_DOSPATH); }

EXPORT(__KSYMw16(wfsymlinkat),libc_w16fsymlinkat);
EXPORT(__KSYMw32(wfsymlinkat),libc_w32fsymlinkat);
EXPORT(__DSYMw16(wfsymlinkat),libc_dos_w16fsymlinkat);
EXPORT(__DSYMw32(wfsymlinkat),libc_dos_w32fsymlinkat);
CRT_WIDECHAR int LIBCCALL
libc_w16fsymlinkat(char16_t const *from, int tofd, char16_t const *to, int flags) {
 int result;
 char *to_str,to_buf[UTF_STACK_BUFFER_SIZE];
 char *from_str,from_buf[UTF_STACK_BUFFER_SIZE];
 if ((from_str = libc_loadutf16(from_buf,from)) == NULL) return -1;
 if ((to_str = libc_loadutf16(to_buf,to)) == NULL) { libc_freeutf(from_buf,from_str); return -1; }
 result = libc_fsymlinkat(from_str,tofd,to_str,flags);
 libc_freeutf(to_buf,to_str);
 libc_freeutf(from_buf,from_str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_w32fsymlinkat(char32_t const *from, int tofd, char32_t const *to, int flags) {
 int result;
 char *to_str,to_buf[UTF_STACK_BUFFER_SIZE];
 char *from_str,from_buf[UTF_STACK_BUFFER_SIZE];
 if ((from_str = libc_loadutf32(from_buf,from)) == NULL) return -1;
 if ((to_str = libc_loadutf32(to_buf,to)) == NULL) { libc_freeutf(from_buf,from_str); return -1; }
 result = libc_fsymlinkat(from_str,tofd,to_str,flags);
 libc_freeutf(to_buf,to_str);
 libc_freeutf(from_buf,from_str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w16fsymlinkat(char16_t const *from, int tofd, char16_t const *to, int flags) {
 return libc_w16fsymlinkat(from,tofd,to,flags|AT_DOSPATH);
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w32fsymlinkat(char32_t const *from, int tofd, char32_t const *to, int flags) {
 return libc_w32fsymlinkat(from,tofd,to,flags|AT_DOSPATH);
}

EXPORT(__KSYMw16(wmknod),libc_w16mknod);
EXPORT(__KSYMw32(wmknod),libc_w32mknod);
EXPORT(__DSYMw16(wmknod),libc_dos_w16mknod);
EXPORT(__DSYMw32(wmknod),libc_dos_w32mknod);
CRT_WIDECHAR int LIBCCALL libc_w16mknod(char16_t const *path, mode_t mode, dev_t dev) { return libc_w16fmknodat(AT_FDCWD,path,mode,dev,0); }
CRT_WIDECHAR int LIBCCALL libc_w32mknod(char32_t const *path, mode_t mode, dev_t dev) { return libc_w32fmknodat(AT_FDCWD,path,mode,dev,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16mknod(char16_t const *path, mode_t mode, dev_t dev) { return libc_w16fmknodat(AT_FDCWD,path,mode,dev,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32mknod(char32_t const *path, mode_t mode, dev_t dev) { return libc_w32fmknodat(AT_FDCWD,path,mode,dev,AT_DOSPATH); }

EXPORT(__KSYMw16(wmknodat),libc_w16mknodat);
EXPORT(__KSYMw32(wmknodat),libc_w32mknodat);
EXPORT(__DSYMw16(wmknodat),libc_dos_w16mknodat);
EXPORT(__DSYMw32(wmknodat),libc_dos_w32mknodat);
CRT_WIDECHAR int LIBCCALL libc_w16mknodat(fd_t dfd, char16_t const *path, mode_t mode, dev_t dev) { return libc_w16fmknodat(dfd,path,mode,dev,0); }
CRT_WIDECHAR int LIBCCALL libc_w32mknodat(fd_t dfd, char32_t const *path, mode_t mode, dev_t dev) { return libc_w32fmknodat(dfd,path,mode,dev,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16mknodat(fd_t dfd, char16_t const *path, mode_t mode, dev_t dev) { return libc_w16fmknodat(dfd,path,mode,dev,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32mknodat(fd_t dfd, char32_t const *path, mode_t mode, dev_t dev) { return libc_w32fmknodat(dfd,path,mode,dev,AT_DOSPATH); }

EXPORT(__KSYMw16(wfmknodat),libc_w16fmknodat);
EXPORT(__KSYMw32(wfmknodat),libc_w32fmknodat);
EXPORT(__DSYMw16(wfmknodat),libc_dos_w16fmknodat);
EXPORT(__DSYMw32(wfmknodat),libc_dos_w32fmknodat);
CRT_WIDECHAR int LIBCCALL
libc_w16fmknodat(fd_t dfd, char16_t const *path, mode_t mode, dev_t dev, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,path);
 if (!str) return -1;
 result = libc_fmknodat(dfd,str,mode,dev,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_w32fmknodat(fd_t dfd, char32_t const *path, mode_t mode, dev_t dev, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,path);
 if (!str) return -1;
 result = libc_fmknodat(dfd,str,mode,dev,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w16fmknodat(fd_t dfd, char16_t const *path, mode_t mode, dev_t dev, int flags) {
 return libc_w16fmknodat(dfd,path,mode,dev,flags|AT_DOSPATH);
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w32fmknodat(fd_t dfd, char32_t const *path, mode_t mode, dev_t dev, int flags) {
 return libc_w32fmknodat(dfd,path,mode,dev,flags|AT_DOSPATH);
}

EXPORT(__KSYMw16(wmkfifo),libc_w16mkfifo);
EXPORT(__KSYMw32(wmkfifo),libc_w32mkfifo);
EXPORT(__DSYMw16(wmkfifo),libc_dos_w16mkfifo);
EXPORT(__DSYMw32(wmkfifo),libc_dos_w32mkfifo);
CRT_WIDECHAR int LIBCCALL libc_w16mkfifo(char16_t const *path, mode_t mode) { return libc_w16fmknodat(AT_FDCWD,path,S_IFIFO|mode,0,0); }
CRT_WIDECHAR int LIBCCALL libc_w32mkfifo(char32_t const *path, mode_t mode) { return libc_w32fmknodat(AT_FDCWD,path,S_IFIFO|mode,0,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16mkfifo(char16_t const *path, mode_t mode) { return libc_w16fmknodat(AT_FDCWD,path,S_IFIFO|mode,0,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32mkfifo(char32_t const *path, mode_t mode) { return libc_w32fmknodat(AT_FDCWD,path,S_IFIFO|mode,0,AT_DOSPATH); }

EXPORT(__KSYMw16(wmkfifoat),libc_w16mkfifoat);
EXPORT(__KSYMw32(wmkfifoat),libc_w32mkfifoat);
EXPORT(__DSYMw16(wmkfifoat),libc_dos_w16mkfifoat);
EXPORT(__DSYMw32(wmkfifoat),libc_dos_w32mkfifoat);
CRT_WIDECHAR int LIBCCALL libc_w16mkfifoat(fd_t dfd, char16_t const *path, mode_t mode) { return libc_w16fmknodat(dfd,path,S_IFIFO|mode,0,0); }
CRT_WIDECHAR int LIBCCALL libc_w32mkfifoat(fd_t dfd, char32_t const *path, mode_t mode) { return libc_w32fmknodat(dfd,path,S_IFIFO|mode,0,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16mkfifoat(fd_t dfd, char16_t const *path, mode_t mode) { return libc_w16fmknodat(dfd,path,S_IFIFO|mode,0,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32mkfifoat(fd_t dfd, char32_t const *path, mode_t mode) { return libc_w32fmknodat(dfd,path,S_IFIFO|mode,0,AT_DOSPATH); }

EXPORT(__KSYMw16(waccess_s),libd_w16access_s);
EXPORT(__KSYMw32(waccess_s),libd_w32access_s);
EXPORT(__DSYMw16(_waccess_s),libd_dos_w16access_s);
EXPORT(__DSYMw32(waccess_s),libd_dos_w32access_s);
CRT_WIDECHAR errno_t LIBCCALL libd_w16access_s(char16_t const *file, int type) { return libc_w16access(file,type) ? libc_geterrno() : 0; }
CRT_WIDECHAR errno_t LIBCCALL libd_w32access_s(char32_t const *file, int type) { return libc_w32access(file,type) ? libc_geterrno() : 0; }
CRT_WIDECHAR derrno_t LIBCCALL libd_dos_w16access_s(char16_t const *file, int type) { return libc_dos_w16access(file,type) ? libc_dos_geterrno() : 0; }
CRT_WIDECHAR derrno_t LIBCCALL libd_dos_w32access_s(char32_t const *file, int type) { return libc_dos_w32access(file,type) ? libc_dos_geterrno() : 0; }

EXPORT(__SYMw16(wfreadlink),libc_w16freadlink);
EXPORT(__SYMw32(wfreadlink),libc_w32freadlink);
CRT_WIDECHAR ssize_t LIBCCALL libc_w16freadlink(fd_t fd, char16_t *buf, size_t buflen) { return libc_w16freadlinkat(fd,libc_empty_string16,buf,buflen,AT_EMPTY_PATH); }
CRT_WIDECHAR ssize_t LIBCCALL libc_w32freadlink(fd_t fd, char32_t *buf, size_t buflen) { return libc_w32freadlinkat(fd,libc_empty_string32,buf,buflen,AT_EMPTY_PATH); }

EXPORT(__KSYMw16(wreadlink),libc_w16readlink);
EXPORT(__KSYMw32(wreadlink),libc_w32readlink);
EXPORT(__DSYMw16(wreadlink),libc_dos_w16readlink);
EXPORT(__DSYMw32(wreadlink),libc_dos_w32readlink);
CRT_WIDECHAR ssize_t LIBCCALL libc_w16readlink(char16_t const *path, char16_t *buf, size_t buflen) { return libc_w16freadlinkat(AT_FDCWD,path,buf,buflen,0); }
CRT_WIDECHAR ssize_t LIBCCALL libc_w32readlink(char32_t const *path, char32_t *buf, size_t buflen) { return libc_w32freadlinkat(AT_FDCWD,path,buf,buflen,0); }
CRT_WIDECHAR ssize_t LIBCCALL libc_dos_w16readlink(char16_t const *path, char16_t *buf, size_t buflen) { return libc_w16freadlinkat(AT_FDCWD,path,buf,buflen,AT_DOSPATH); }
CRT_WIDECHAR ssize_t LIBCCALL libc_dos_w32readlink(char32_t const *path, char32_t *buf, size_t buflen) { return libc_w32freadlinkat(AT_FDCWD,path,buf,buflen,AT_DOSPATH); }

EXPORT(__KSYMw16(wreadlinkat),libc_w16readlinkat);
EXPORT(__KSYMw32(wreadlinkat),libc_w32readlinkat);
EXPORT(__DSYMw16(wreadlinkat),libc_dos_w16readlinkat);
EXPORT(__DSYMw32(wreadlinkat),libc_dos_w32readlinkat);
CRT_WIDECHAR ssize_t LIBCCALL libc_w16readlinkat(fd_t dfd, char16_t const *path, char16_t *buf, size_t buflen) { return libc_w16freadlinkat(dfd,path,buf,buflen,0); }
CRT_WIDECHAR ssize_t LIBCCALL libc_w32readlinkat(fd_t dfd, char32_t const *path, char32_t *buf, size_t buflen) { return libc_w32freadlinkat(dfd,path,buf,buflen,0); }
CRT_WIDECHAR ssize_t LIBCCALL libc_dos_w16readlinkat(fd_t dfd, char16_t const *path, char16_t *buf, size_t buflen) { return libc_w16freadlinkat(dfd,path,buf,buflen,AT_DOSPATH); }
CRT_WIDECHAR ssize_t LIBCCALL libc_dos_w32readlinkat(fd_t dfd, char32_t const *path, char32_t *buf, size_t buflen) { return libc_w32freadlinkat(dfd,path,buf,buflen,AT_DOSPATH); }

EXPORT(__KSYMw16(wfreadlinkat),libc_w16freadlinkat);
EXPORT(__KSYMw32(wfreadlinkat),libc_w32freadlinkat);
EXPORT(__DSYMw16(wfreadlinkat),libc_dos_w16freadlinkat);
EXPORT(__DSYMw32(wfreadlinkat),libc_dos_w32freadlinkat);
CRT_WIDECHAR ssize_t LIBCCALL
libc_w16freadlinkat(fd_t dfd, char16_t const *path,
                    char16_t *buf, size_t buflen, int flags) {
 LIBC_TRY {
  return libc_Xw16freadlinkat(dfd,path,buf,buflen,flags);
 } LIBC_EXCEPT (libc_except_errno()) {
 }
 return -1;
}
CRT_WIDECHAR ssize_t LIBCCALL
libc_w32freadlinkat(fd_t dfd, char32_t const *path,
                    char32_t *buf, size_t buflen, int flags) {
 LIBC_TRY {
  return libc_Xw32freadlinkat(dfd,path,buf,buflen,flags);
 } LIBC_EXCEPT (libc_except_errno()) {
 }
 return -1;
}
CRT_WIDECHAR ssize_t LIBCCALL
libc_dos_w16freadlinkat(fd_t dfd, char16_t const *path, char16_t *buf, size_t buflen, int flags) {
 return libc_w16freadlinkat(dfd,path,buf,buflen,flags|AT_DOSPATH);
}
CRT_WIDECHAR ssize_t LIBCCALL
libc_dos_w32freadlinkat(fd_t dfd, char32_t const *path, char32_t *buf, size_t buflen, int flags) {
 return libc_w32freadlinkat(dfd,path,buf,buflen,flags|AT_DOSPATH);
}

EXPORT(__KSYMw16(waccess),libc_w16access);
EXPORT(__KSYMw32(waccess),libc_w32access);
EXPORT(__DSYMw16(_waccess),libc_dos_w16access);
EXPORT(__DSYMw32(waccess),libc_dos_w32access);
CRT_WIDECHAR int LIBCCALL libc_w16access(char16_t const *name, int type) { return libc_w16faccessat(AT_FDCWD,name,type,0); }
CRT_WIDECHAR int LIBCCALL libc_w32access(char32_t const *name, int type) { return libc_w32faccessat(AT_FDCWD,name,type,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16access(char16_t const *name, int type) { return libc_w16faccessat(AT_FDCWD,name,type,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32access(char32_t const *name, int type) { return libc_w32faccessat(AT_FDCWD,name,type,AT_DOSPATH); }

EXPORT(__KSYMw16(weaccess),libc_w16eaccess);
EXPORT(__KSYMw32(weaccess),libc_w32eaccess);
EXPORT(__DSYMw16(weaccess),libc_dos_w16eaccess);
EXPORT(__DSYMw32(weaccess),libc_dos_w32eaccess);
CRT_WIDECHAR int LIBCCALL libc_w16eaccess(char16_t const *name, int type) { return libc_w16faccessat(AT_FDCWD,name,type,AT_EACCESS); }
CRT_WIDECHAR int LIBCCALL libc_w32eaccess(char32_t const *name, int type) { return libc_w32faccessat(AT_FDCWD,name,type,AT_EACCESS); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16eaccess(char16_t const *name, int type) { return libc_w16faccessat(AT_FDCWD,name,type,AT_EACCESS|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32eaccess(char32_t const *name, int type) { return libc_w32faccessat(AT_FDCWD,name,type,AT_EACCESS|AT_DOSPATH); }

EXPORT(__KSYMw16(wfaccessat),libc_w16faccessat);
EXPORT(__KSYMw32(wfaccessat),libc_w32faccessat);
EXPORT(__DSYMw16(wfaccessat),libc_dos_w16faccessat);
EXPORT(__DSYMw32(wfaccessat),libc_dos_w32faccessat);
CRT_WIDECHAR int LIBCCALL
libc_w16faccessat(fd_t dfd, char16_t const *file, int type, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,file);
 if (!str) return -1;
 result = libc_faccessat(dfd,str,type,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_w32faccessat(fd_t dfd, char32_t const *file, int type, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,file);
 if (!str) return -1;
 result = libc_faccessat(dfd,str,type,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w16faccessat(fd_t dfd, char16_t const *file, int type, int flags) {
 return libc_w16faccessat(dfd,file,type,flags|AT_DOSPATH);
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w32faccessat(fd_t dfd, char32_t const *file, int type, int flags) {
 return libc_w32faccessat(dfd,file,type,flags|AT_DOSPATH);
}

EXPORT(__KSYMw16(wtruncate),libc_w16truncate);
EXPORT(__KSYMw32(wtruncate),libc_w32truncate);
EXPORT(__DSYMw16(wtruncate),libc_dos_w16truncate);
EXPORT(__DSYMw32(wtruncate),libc_dos_w32truncate);
CRT_WIDECHAR int LIBCCALL libc_w16truncate(char16_t const *file, pos32_t length) { return libc_w16ftruncateat64(AT_FDCWD,file,length,0); }
CRT_WIDECHAR int LIBCCALL libc_w32truncate(char32_t const *file, pos32_t length) { return libc_w32ftruncateat64(AT_FDCWD,file,length,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16truncate(char16_t const *file, pos32_t length) { return libc_w16ftruncateat64(AT_FDCWD,file,length,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32truncate(char32_t const *file, pos32_t length) { return libc_w32ftruncateat64(AT_FDCWD,file,length,AT_DOSPATH); }

EXPORT(__KSYMw16(wtruncate64),libc_w16truncate64);
EXPORT(__KSYMw32(wtruncate64),libc_w32truncate64);
EXPORT(__DSYMw16(wtruncate64),libc_dos_w16truncate64);
EXPORT(__DSYMw32(wtruncate64),libc_dos_w32truncate64);
CRT_WIDECHAR int LIBCCALL libc_w16truncate64(char16_t const *file, pos64_t length) { return libc_w16ftruncateat64(AT_FDCWD,file,length,0); }
CRT_WIDECHAR int LIBCCALL libc_w32truncate64(char32_t const *file, pos64_t length) { return libc_w32ftruncateat64(AT_FDCWD,file,length,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16truncate64(char16_t const *file, pos64_t length) { return libc_w16ftruncateat64(AT_FDCWD,file,length,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32truncate64(char32_t const *file, pos64_t length) { return libc_w32ftruncateat64(AT_FDCWD,file,length,AT_DOSPATH); }

EXPORT(__KSYMw16(wftruncateat),libc_w16ftruncateat);
EXPORT(__KSYMw32(wftruncateat),libc_w32ftruncateat);
EXPORT(__DSYMw16(wftruncateat),libc_dos_w16ftruncateat);
EXPORT(__DSYMw32(wftruncateat),libc_dos_w32ftruncateat);
CRT_WIDECHAR int LIBCCALL libc_w16ftruncateat(fd_t dfd, char16_t const *file, pos32_t length, int flags) { return libc_w16ftruncateat64(dfd,file,length,flags); }
CRT_WIDECHAR int LIBCCALL libc_w32ftruncateat(fd_t dfd, char32_t const *file, pos32_t length, int flags) { return libc_w32ftruncateat64(dfd,file,length,flags); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16ftruncateat(fd_t dfd, char16_t const *file, pos32_t length, int flags) { return libc_w16ftruncateat64(dfd,file,length,flags|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32ftruncateat(fd_t dfd, char32_t const *file, pos32_t length, int flags) { return libc_w32ftruncateat64(dfd,file,length,flags|AT_DOSPATH); }

EXPORT(__KSYMw16(wftruncateat64),libc_w16ftruncateat64);
EXPORT(__KSYMw32(wftruncateat64),libc_w32ftruncateat64);
EXPORT(__DSYMw16(wftruncateat64),libc_dos_w16ftruncateat64);
EXPORT(__DSYMw32(wftruncateat64),libc_dos_w32ftruncateat64);
CRT_WIDECHAR int LIBCCALL
libc_w16ftruncateat64(fd_t dfd, char16_t const *file, pos64_t length, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,file);
 if (!str) return -1;
 result = libc_ftruncateat64(dfd,str,length,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_w32ftruncateat64(fd_t dfd, char32_t const *file, pos64_t length, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,file);
 if (!str) return -1;
 result = libc_ftruncateat64(dfd,str,length,flags);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w16ftruncateat64(fd_t dfd, char16_t const *file, pos64_t length, int flags) {
 return libc_w16ftruncateat64(dfd,file,length,flags|AT_DOSPATH);
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w32ftruncateat64(fd_t dfd, char32_t const *file, pos64_t length, int flags) {
 return libc_w32ftruncateat64(dfd,file,length,flags|AT_DOSPATH);
}

EXPORT(__KSYMw16(wchroot),libc_w16chroot);
EXPORT(__KSYMw32(wchroot),libc_w32chroot);
EXPORT(__DSYMw16(wchroot),libc_dos_w16chroot);
EXPORT(__DSYMw32(wchroot),libc_dos_w32chroot);
CRT_WIDECHAR int LIBCCALL
libc_w16chroot(char16_t const *path) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,path);
 if (!str) return -1;
 result = libc_chroot(str);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_w32chroot(char32_t const *path) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,path);
 if (!str) return -1;
 result = libc_chroot(str);
 libc_freeutf(buf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w16chroot(char16_t const *path) {
 int result,new_root;
 new_root = libc_dos_w16open(path,O_RDONLY|O_DIRECTORY);
 if (new_root < 0) return -1;
 result = libc_dup2(new_root,AT_FDROOT);
 sys_close(new_root);
 return result < 0 ? result : 0;
}
CRT_WIDECHAR int LIBCCALL
libc_dos_w32chroot(char32_t const *path) {
 int result,new_root;
 new_root = libc_dos_w32open(path,O_RDONLY|O_DIRECTORY);
 if (new_root < 0) return -1;
 result = libc_dup2(new_root,AT_FDROOT);
 sys_close(new_root);
 return result < 0 ? result : 0;
}


EXPORT(__SYMw16(Xwchown),libc_Xw16chown);
EXPORT(__SYMw32(Xwchown),libc_Xw32chown);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16chown(char16_t const *file, uid_t owner, gid_t group) { libc_Xw16fchownat(AT_FDCWD,file,owner,group,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32chown(char32_t const *file, uid_t owner, gid_t group) { libc_Xw32fchownat(AT_FDCWD,file,owner,group,0); }

EXPORT(__SYMw16(Xwlchown),libc_Xw16lchown);
EXPORT(__SYMw32(Xwlchown),libc_Xw32lchown);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16lchown(char16_t const *file, uid_t owner, gid_t group) { libc_Xw16fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32lchown(char32_t const *file, uid_t owner, gid_t group) { libc_Xw32fchownat(AT_FDCWD,file,owner,group,AT_SYMLINK_NOFOLLOW); }

EXPORT(__SYMw16(Xwfchownat),libc_Xw16fchownat);
EXPORT(__SYMw32(Xwfchownat),libc_Xw32fchownat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16fchownat(fd_t dfd, char16_t const *file, uid_t owner, gid_t group, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(buf,file);
 LIBC_TRY {
  libc_Xfchownat(dfd,str,owner,group,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32fchownat(fd_t dfd, char32_t const *file, uid_t owner, gid_t group, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(buf,file);
 LIBC_TRY {
  libc_Xfchownat(dfd,str,owner,group,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}

EXPORT(__SYMw16(Xwchmod),libc_Xw16chmod);
EXPORT(__SYMw32(Xwchmod),libc_Xw32chmod);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16chmod(char16_t const *file, mode_t mode) { return libc_Xw16fchmodat(AT_FDCWD,file,mode,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32chmod(char32_t const *file, mode_t mode) { return libc_Xw32fchmodat(AT_FDCWD,file,mode,0); }

EXPORT(__SYMw16(Xwlchmod),libc_Xw16lchmod);
EXPORT(__SYMw32(Xwlchmod),libc_Xw32lchmod);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16lchmod(char16_t const *file, mode_t mode) { return libc_Xw16fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32lchmod(char32_t const *file, mode_t mode) { return libc_Xw32fchmodat(AT_FDCWD,file,mode,AT_SYMLINK_NOFOLLOW); }

EXPORT(__SYMw16(Xwfchmodat),libc_Xw16fchmodat);
EXPORT(__SYMw32(Xwfchmodat),libc_Xw32fchmodat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16fchmodat(fd_t dfd, char16_t const *file, mode_t mode, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(buf,file);
 LIBC_TRY {
  libc_Xfchmodat(dfd,str,mode,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32fchmodat(fd_t dfd, char32_t const *file, mode_t mode, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(buf,file);
 LIBC_TRY {
  libc_Xfchmodat(dfd,str,mode,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}

EXPORT(__SYMw16(Xwlink),libc_Xw16link);
EXPORT(__SYMw32(Xwlink),libc_Xw32link);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16link(char16_t const *from, char16_t const *to) { libc_Xw16linkat(AT_FDCWD,from,AT_FDCWD,to,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32link(char32_t const *from, char32_t const *to) { libc_Xw32linkat(AT_FDCWD,from,AT_FDCWD,to,0); }

EXPORT(__SYMw16(Xwlinkat),libc_Xw16linkat);
EXPORT(__SYMw32(Xwlinkat),libc_Xw32linkat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16linkat(int fromfd, char16_t const *from,
                int tofd, char16_t const *to, int flags) {
 char from_buf[UTF_STACK_BUFFER_SIZE],*from_str;
 char to_buf[UTF_STACK_BUFFER_SIZE],*to_str = to_buf;
 from_str = libc_Xloadutf16(from_buf,from);
 LIBC_TRY {
  to_str = libc_Xloadutf16(to_buf,to);
  libc_linkat(fromfd,from_str,tofd,to_str,flags);
 } LIBC_FINALLY {
  libc_freeutf(to_buf,to_str);
  libc_freeutf(from_buf,from_str);
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32linkat(int fromfd, char32_t const *from,
                int tofd, char32_t const *to, int flags) {
 char from_buf[UTF_STACK_BUFFER_SIZE],*from_str;
 char to_buf[UTF_STACK_BUFFER_SIZE],*to_str = to_buf;
 from_str = libc_Xloadutf32(from_buf,from);
 LIBC_TRY {
  to_str = libc_Xloadutf32(to_buf,to);
  libc_linkat(fromfd,from_str,tofd,to_str,flags);
 } LIBC_FINALLY {
  libc_freeutf(to_buf,to_str);
  libc_freeutf(from_buf,from_str);
 }
}

EXPORT(__SYMw16(Xwunlink),libc_Xw16unlink);
EXPORT(__SYMw32(Xwunlink),libc_Xw32unlink);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16unlink(char16_t const *name) { return libc_Xw16unlinkat(AT_FDCWD,name,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32unlink(char32_t const *name) { return libc_Xw32unlinkat(AT_FDCWD,name,0); }

EXPORT(__SYMw16(Xwunlinkat),libc_Xw16unlinkat);
EXPORT(__SYMw32(Xwunlinkat),libc_Xw32unlinkat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16unlinkat(fd_t dfd, char16_t const *name, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(buf,name);
 LIBC_TRY {
  libc_Xunlinkat(dfd,str,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32unlinkat(fd_t dfd, char32_t const *name, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(buf,name);
 LIBC_TRY {
  libc_Xunlinkat(dfd,str,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}

EXPORT(__SYMw16(Xwrmdir),libc_Xw16rmdir);
EXPORT(__SYMw32(Xwrmdir),libc_Xw32rmdir);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16rmdir(char16_t const *name) { libc_Xw16unlinkat(AT_FDCWD,name,AT_REMOVEDIR); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32rmdir(char32_t const *name) { libc_Xw32unlinkat(AT_FDCWD,name,AT_REMOVEDIR); }

EXPORT(__SYMw16(Xwremove),libc_Xw16remove);
EXPORT(__SYMw32(Xwremove),libc_Xw32remove);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16remove(char16_t const *name) { libc_Xw16unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_REMOVEREG); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32remove(char32_t const *name) { libc_Xw32unlinkat(AT_FDCWD,name,AT_REMOVEDIR|AT_REMOVEREG); }

EXPORT(__SYMw16(Xwremoveat),libc_Xw16removeat);
EXPORT(__SYMw32(Xwremoveat),libc_Xw32removeat);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16removeat(fd_t dfd, char16_t const *name) { libc_Xw16unlinkat(dfd,name,AT_REMOVEDIR|AT_REMOVEREG); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32removeat(fd_t dfd, char32_t const *name) { libc_Xw32unlinkat(dfd,name,AT_REMOVEDIR|AT_REMOVEREG); }

EXPORT(__SYMw16(Xwrename),libc_Xw16rename);
EXPORT(__SYMw32(Xwrename),libc_Xw32rename);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16rename(char16_t const *oldname, char16_t const *newname) { libc_Xw16frenameat(AT_FDCWD,oldname,AT_FDCWD,newname,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32rename(char32_t const *oldname, char32_t const *newname) { libc_Xw32frenameat(AT_FDCWD,oldname,AT_FDCWD,newname,0); }

EXPORT(__SYMw16(Xwrenameat),libc_Xw16renameat);
EXPORT(__SYMw32(Xwrenameat),libc_Xw32renameat);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16renameat(fd_t oldfd, char16_t const *oldname, fd_t newfd, char16_t const *newname) { libc_Xw16frenameat(oldfd,oldname,newfd,newname,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32renameat(fd_t oldfd, char32_t const *oldname, fd_t newfd, char32_t const *newname) { libc_Xw32frenameat(oldfd,oldname,newfd,newname,0); }

EXPORT(__SYMw16(Xwfrenameat),libc_Xw16frenameat);
EXPORT(__SYMw32(Xwfrenameat),libc_Xw32frenameat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16frenameat(fd_t oldfd, char16_t const *oldname,
                   fd_t newfd, char16_t const *newname,
                   int flags) {
 char old_buf[UTF_STACK_BUFFER_SIZE],*old_str;
 char new_buf[UTF_STACK_BUFFER_SIZE],*new_str = new_buf;
 old_str = libc_Xloadutf16(old_buf,oldname);
 LIBC_TRY {
  new_str = libc_Xloadutf16(new_buf,newname);
  libc_frenameat(oldfd,old_str,newfd,new_str,flags);
 } LIBC_FINALLY {
  libc_freeutf(new_buf,new_str);
  libc_freeutf(old_buf,old_str);
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32frenameat(fd_t oldfd, char32_t const *oldname,
                   fd_t newfd, char32_t const *newname,
                   int flags) {
 char old_buf[UTF_STACK_BUFFER_SIZE],*old_str;
 char new_buf[UTF_STACK_BUFFER_SIZE],*new_str = new_buf;
 old_str = libc_Xloadutf32(old_buf,oldname);
 LIBC_TRY {
  new_str = libc_Xloadutf32(new_buf,newname);
  libc_frenameat(oldfd,old_str,newfd,new_str,flags);
 } LIBC_FINALLY {
  libc_freeutf(new_buf,new_str);
  libc_freeutf(old_buf,old_str);
 }
}

EXPORT(__SYMw16(Xwmkdir),libc_Xw16mkdir);
EXPORT(__SYMw32(Xwmkdir),libc_Xw32mkdir);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16mkdir(char16_t const *name, mode_t mode) { libc_Xw16fmkdirat(AT_FDCWD,name,mode,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32mkdir(char32_t const *name, mode_t mode) { libc_Xw32fmkdirat(AT_FDCWD,name,mode,0); }

EXPORT(__SYMw16(Xwmkdirat),libc_Xw16mkdirat);
EXPORT(__SYMw32(Xwmkdirat),libc_Xw32mkdirat);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16mkdirat(fd_t dfd, char16_t const *name, mode_t mode) { libc_Xw16fmkdirat(dfd,name,mode,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32mkdirat(fd_t dfd, char32_t const *name, mode_t mode) { libc_Xw32fmkdirat(dfd,name,mode,0); }

EXPORT(__SYMw16(Xwfmkdirat),libc_Xw16fmkdirat);
EXPORT(__SYMw32(Xwfmkdirat),libc_Xw32fmkdirat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16fmkdirat(fd_t dfd, char16_t const *name, mode_t mode, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(buf,name);
 LIBC_TRY {
  libc_Xfmkdirat(dfd,str,mode,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32fmkdirat(fd_t dfd, char32_t const *name, mode_t mode, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(buf,name);
 LIBC_TRY {
  libc_Xfmkdirat(dfd,str,mode,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}

EXPORT(__SYMw16(Xwsymlink),libc_Xw16symlink);
EXPORT(__SYMw32(Xwsymlink),libc_Xw32symlink);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16symlink(char16_t const *from, char16_t const *to) { libc_Xw16fsymlinkat(from,AT_FDCWD,to,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32symlink(char32_t const *from, char32_t const *to) { libc_Xw32fsymlinkat(from,AT_FDCWD,to,0); }

EXPORT(__SYMw16(Xwsymlinkat),libc_Xw16symlinkat);
EXPORT(__SYMw32(Xwsymlinkat),libc_Xw32symlinkat);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16symlinkat(char16_t const *from, int tofd, char16_t const *to) { libc_Xw16fsymlinkat(from,tofd,to,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32symlinkat(char32_t const *from, int tofd, char32_t const *to) { libc_Xw32fsymlinkat(from,tofd,to,0); }

EXPORT(__SYMw16(Xwfsymlinkat),libc_Xw16fsymlinkat);
EXPORT(__SYMw32(Xwfsymlinkat),libc_Xw32fsymlinkat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16fsymlinkat(char16_t const *from, int tofd, char16_t const *to, int flags) {
 char from_buf[UTF_STACK_BUFFER_SIZE],*from_str;
 char to_buf[UTF_STACK_BUFFER_SIZE],*to_str = to_buf;
 from_str = libc_Xloadutf16(from_buf,from);
 LIBC_TRY {
  to_str = libc_Xloadutf16(to_buf,to);
  libc_fsymlinkat(from_str,tofd,to_str,flags);
 } LIBC_FINALLY {
  libc_freeutf(to_buf,to_str);
  libc_freeutf(from_buf,from_str);
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32fsymlinkat(char32_t const *from, int tofd, char32_t const *to, int flags) {
 char from_buf[UTF_STACK_BUFFER_SIZE],*from_str;
 char to_buf[UTF_STACK_BUFFER_SIZE],*to_str = to_buf;
 from_str = libc_Xloadutf32(from_buf,from);
 LIBC_TRY {
  to_str = libc_Xloadutf32(to_buf,to);
  libc_fsymlinkat(from_str,tofd,to_str,flags);
 } LIBC_FINALLY {
  libc_freeutf(to_buf,to_str);
  libc_freeutf(from_buf,from_str);
 }
}

EXPORT(__SYMw16(Xwmknod),libc_Xw16mknod);
EXPORT(__SYMw32(Xwmknod),libc_Xw32mknod);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16mknod(char16_t const *path, mode_t mode, dev_t dev) { libc_Xw16fmknodat(AT_FDCWD,path,mode,dev,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32mknod(char32_t const *path, mode_t mode, dev_t dev) { libc_Xw32fmknodat(AT_FDCWD,path,mode,dev,0); }

EXPORT(__SYMw16(Xwmknodat),libc_Xw16mknodat);
EXPORT(__SYMw32(Xwmknodat),libc_Xw32mknodat);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16mknodat(fd_t dfd, char16_t const *path, mode_t mode, dev_t dev) { libc_Xw16fmknodat(dfd,path,mode,dev,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32mknodat(fd_t dfd, char32_t const *path, mode_t mode, dev_t dev) { libc_Xw32fmknodat(dfd,path,mode,dev,0); }

EXPORT(__SYMw16(Xwfmknodat),libc_Xw16fmknodat);
EXPORT(__SYMw32(Xwfmknodat),libc_Xw32fmknodat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16fmknodat(fd_t dfd, char16_t const *path, mode_t mode, dev_t dev, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(buf,path);
 LIBC_TRY {
  libc_Xfmknodat(dfd,str,mode,dev,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32fmknodat(fd_t dfd, char32_t const *path, mode_t mode, dev_t dev, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(buf,path);
 LIBC_TRY {
  libc_Xfmknodat(dfd,str,mode,dev,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}

EXPORT(__SYMw16(Xwmkfifo),libc_Xw16mkfifo);
EXPORT(__SYMw32(Xwmkfifo),libc_Xw32mkfifo);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16mkfifo(char16_t const *path, mode_t mode) { libc_Xw16fmknodat(AT_FDCWD,path,S_IFIFO|mode,0,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32mkfifo(char32_t const *path, mode_t mode) { libc_Xw32fmknodat(AT_FDCWD,path,S_IFIFO|mode,0,0); }

EXPORT(__SYMw16(Xwmkfifoat),libc_Xw16mkfifoat);
EXPORT(__SYMw32(Xwmkfifoat),libc_Xw32mkfifoat);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16mkfifoat(fd_t dfd, char16_t const *path, mode_t mode) { libc_Xw16fmknodat(dfd,path,S_IFIFO|mode,0,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32mkfifoat(fd_t dfd, char32_t const *path, mode_t mode) { libc_Xw32fmknodat(dfd,path,S_IFIFO|mode,0,0); }

EXPORT(__SYMw16(Xwfreadlink),libc_Xw16freadlink);
EXPORT(__SYMw32(Xwfreadlink),libc_Xw32freadlink);
CRT_WIDECHAR_EXCEPT size_t LIBCCALL libc_Xw16freadlink(fd_t fd, char16_t *buf, size_t buflen) { return libc_Xw16freadlinkat(fd,libc_empty_string16,buf,buflen,AT_EMPTY_PATH); }
CRT_WIDECHAR_EXCEPT size_t LIBCCALL libc_Xw32freadlink(fd_t fd, char32_t *buf, size_t buflen) { return libc_Xw32freadlinkat(fd,libc_empty_string32,buf,buflen,AT_EMPTY_PATH); }

EXPORT(__SYMw16(Xwreadlink),libc_Xw16readlink);
EXPORT(__SYMw32(Xwreadlink),libc_Xw32readlink);
CRT_WIDECHAR_EXCEPT size_t LIBCCALL libc_Xw16readlink(char16_t const *path, char16_t *buf, size_t buflen) { return libc_Xw16freadlinkat(AT_FDCWD,path,buf,buflen,0); }
CRT_WIDECHAR_EXCEPT size_t LIBCCALL libc_Xw32readlink(char32_t const *path, char32_t *buf, size_t buflen) { return libc_Xw32freadlinkat(AT_FDCWD,path,buf,buflen,0); }

EXPORT(__SYMw16(Xwreadlinkat),libc_Xw16readlinkat);
EXPORT(__SYMw32(Xwreadlinkat),libc_Xw32readlinkat);
CRT_WIDECHAR_EXCEPT size_t LIBCCALL libc_Xw16readlinkat(fd_t dfd, char16_t const *path, char16_t *buf, size_t buflen) { return libc_Xw16freadlinkat(dfd,path,buf,buflen,0); }
CRT_WIDECHAR_EXCEPT size_t LIBCCALL libc_Xw32readlinkat(fd_t dfd, char32_t const *path, char32_t *buf, size_t buflen) { return libc_Xw32freadlinkat(dfd,path,buf,buflen,0); }


#define FULL_LINK_BUFSIZE  256
CRT_WIDECHAR_EXCEPT char *LIBCCALL
read_full_link(fd_t dfd, char const *path,
               char *buf, int flags) {
 size_t reqlen,new_reqlen; char *result;
 reqlen = libc_Xreadlinkat(dfd,path,buf,FULL_LINK_BUFSIZE);
 if (reqlen <= FULL_LINK_BUFSIZE)
     return buf;
 result = NULL;
 /* IF the static buffer doesn't suffice, allocate a dynamic one. */
 LIBC_TRY {
  for (;;) {
   result = (char *)libc_Xrealloc(result,reqlen*sizeof(char));
   new_reqlen = libc_Xreadlinkat(dfd,path,result,reqlen);
   if (new_reqlen <= reqlen)
       break;
   reqlen = new_reqlen;
  }
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  libc_free(result);
  error_rethrow();
 }
 return result;
}

EXPORT(__SYMw16(Xwfreadlinkat),libc_Xw16freadlinkat);
EXPORT(__SYMw32(Xwfreadlinkat),libc_Xw32freadlinkat);
CRT_WIDECHAR_EXCEPT size_t LIBCCALL
libc_Xw16freadlinkat(fd_t dfd, char16_t const *path,
                     char16_t *buf, size_t buflen, int flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 char linkbuf[FULL_LINK_BUFSIZE],*linkstr = linkbuf;
 char pathbuf[UTF_STACK_BUFFER_SIZE],*pathstr;
 pathstr = libc_Xloadutf16(pathbuf,path);
 LIBC_TRY {
  mbstate_t state = MBSTATE_INIT;
  linkstr = read_full_link(dfd,pathstr,linkbuf,flags);
  /* Translate the link string into unicode, using the given buffer.
   * NOTE: Follow buffering behavior defined by the `AT_READLINK_REQSIZE' flag. */
  result = libc_Xutf8to16(linkstr,(size_t)-1,buf,buflen,&state,
                         (flags&AT_READLINK_REQSIZE)
                       ? (UNICODE_F_STOPONNUL|UNICODE_F_SETERRNO)
                       : (UNICODE_F_STOPONNUL|UNICODE_F_SETERRNO|UNICODE_F_NOZEROTERM));
  if (result > buflen && !(flags&AT_READLINK_REQSIZE))
      result = buflen;
 } LIBC_FINALLY {
  if (linkstr != linkbuf)
      libc_free(linkstr);
  libc_freeutf(pathbuf,pathstr);
 }
 return result;
}
CRT_WIDECHAR_EXCEPT size_t LIBCCALL
libc_Xw32freadlinkat(fd_t dfd, char32_t const *path, char32_t *buf, size_t buflen, int flags) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 char linkbuf[FULL_LINK_BUFSIZE],*linkstr = linkbuf;
 char pathbuf[UTF_STACK_BUFFER_SIZE],*pathstr;
 pathstr = libc_Xloadutf32(pathbuf,path);
 LIBC_TRY {
  mbstate_t state = MBSTATE_INIT;
  linkstr = read_full_link(dfd,pathstr,linkbuf,flags);
  /* Translate the link string into unicode, using the given buffer.
   * NOTE: Follow buffering behavior defined by the `AT_READLINK_REQSIZE' flag. */
  result = libc_Xutf8to32(linkstr,(size_t)-1,buf,buflen,&state,
                         (flags&AT_READLINK_REQSIZE)
                       ? (UNICODE_F_STOPONNUL|UNICODE_F_SETERRNO)
                       : (UNICODE_F_STOPONNUL|UNICODE_F_SETERRNO|UNICODE_F_NOZEROTERM));
  if (result > buflen && !(flags&AT_READLINK_REQSIZE))
      result = buflen;
 } LIBC_FINALLY {
  if (linkstr != linkbuf)
      libc_free(linkstr);
  libc_freeutf(pathbuf,pathstr);
 }
 return result;
}

EXPORT(__SYMw16(Xwaccess),libc_Xw16access);
EXPORT(__SYMw32(Xwaccess),libc_Xw32access);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16access(char16_t const *name, int type) { libc_Xw16faccessat(AT_FDCWD,name,type,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32access(char32_t const *name, int type) { libc_Xw32faccessat(AT_FDCWD,name,type,0); }

EXPORT(__SYMw16(Xweaccess),libc_Xw16eaccess);
EXPORT(__SYMw32(Xweaccess),libc_Xw32eaccess);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16eaccess(char16_t const *name, int type) { libc_Xw16faccessat(AT_FDCWD,name,type,AT_EACCESS); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32eaccess(char32_t const *name, int type) { libc_Xw32faccessat(AT_FDCWD,name,type,AT_EACCESS); }

EXPORT(__SYMw16(Xwfaccessat),libc_Xw16faccessat);
EXPORT(__SYMw32(Xwfaccessat),libc_Xw32faccessat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16faccessat(fd_t dfd, char16_t const *file, int type, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(buf,file);
 LIBC_TRY {
  libc_Xfaccessat(dfd,str,type,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32faccessat(fd_t dfd, char32_t const *file, int type, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(buf,file);
 LIBC_TRY {
  libc_Xfaccessat(dfd,str,type,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}

EXPORT(__SYMw16(Xwtruncate),libc_Xw16truncate);
EXPORT(__SYMw32(Xwtruncate),libc_Xw32truncate);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16truncate(char16_t const *file, pos32_t length) { libc_Xw16ftruncateat64(AT_FDCWD,file,length,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32truncate(char32_t const *file, pos32_t length) { libc_Xw32ftruncateat64(AT_FDCWD,file,length,0); }

EXPORT(__SYMw16(Xwtruncate64),libc_Xw16truncate64);
EXPORT(__SYMw32(Xwtruncate64),libc_Xw32truncate64);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16truncate64(char16_t const *file, pos64_t length) { libc_Xw16ftruncateat64(AT_FDCWD,file,length,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32truncate64(char32_t const *file, pos64_t length) { libc_Xw32ftruncateat64(AT_FDCWD,file,length,0); }

EXPORT(__SYMw16(Xwftruncateat),libc_Xw16ftruncateat);
EXPORT(__SYMw32(Xwftruncateat),libc_Xw32ftruncateat);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16ftruncateat(fd_t dfd, char16_t const *file, pos32_t length, int flags) { libc_Xw16ftruncateat64(dfd,file,length,flags); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32ftruncateat(fd_t dfd, char32_t const *file, pos32_t length, int flags) { libc_Xw32ftruncateat64(dfd,file,length,flags); }

EXPORT(__SYMw16(Xwftruncateat64),libc_Xw16ftruncateat64);
EXPORT(__SYMw32(Xwftruncateat64),libc_Xw32ftruncateat64);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16ftruncateat64(fd_t dfd, char16_t const *file, pos64_t length, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(buf,file);
 LIBC_TRY {
  libc_Xftruncateat64(dfd,str,length,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32ftruncateat64(fd_t dfd, char32_t const *file, pos64_t length, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(buf,file);
 LIBC_TRY {
  libc_Xftruncateat64(dfd,str,length,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}

EXPORT(__SYMw16(Xwchroot),libc_Xw16chroot);
EXPORT(__SYMw32(Xwchroot),libc_Xw32chroot);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16chroot(char16_t const *path) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(buf,path);
 LIBC_TRY {
  libc_Xchroot(str);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32chroot(char32_t const *path) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(buf,path);
 LIBC_TRY {
  libc_Xchroot(str);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}


EXPORT(__KSYMw16(kwstat),libc_kw16stat);
EXPORT(__KSYMw32(kwstat),libc_kw32stat);
EXPORT(__DSYMw16(kwstat),libc_dos_kw16stat);
EXPORT(__DSYMw32(kwstat),libc_dos_kw32stat);
DEFINE_INTERN_ALIAS(libc_kw16stat,libc_kw16stat64);
DEFINE_INTERN_ALIAS(libc_kw32stat,libc_kw32stat64);
DEFINE_INTERN_ALIAS(libc_dos_kw16stat,libc_dos_kw16stat64);
DEFINE_INTERN_ALIAS(libc_dos_kw32stat,libc_dos_kw32stat64);

EXPORT(__KSYMw16(kwstat64),libc_kw16stat64);
EXPORT(__KSYMw32(kwstat64),libc_kw32stat64);
EXPORT(__DSYMw16(kwstat64),libc_dos_kw16stat64);
EXPORT(__DSYMw32(kwstat64),libc_dos_kw32stat64);
CRT_WIDECHAR int LIBCCALL libc_kw16stat64(char16_t const *file, struct __kos_stat64 *buf) { return libc_kw16fstatat64(AT_FDCWD,file,buf,0); }
CRT_WIDECHAR int LIBCCALL libc_kw32stat64(char32_t const *file, struct __kos_stat64 *buf) { return libc_kw32fstatat64(AT_FDCWD,file,buf,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_kw16stat64(char16_t const *file, struct __kos_stat64 *buf) { return libc_kw16fstatat64(AT_FDCWD,file,buf,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_kw32stat64(char32_t const *file, struct __kos_stat64 *buf) { return libc_kw32fstatat64(AT_FDCWD,file,buf,AT_DOSPATH); }

EXPORT(__KSYMw16(kwfstatat),libc_kw16fstatat);
EXPORT(__KSYMw32(kwfstatat),libc_kw32fstatat);
EXPORT(__DSYMw16(kwfstatat),libc_dos_kw16fstatat);
EXPORT(__DSYMw32(kwfstatat),libc_dos_kw32fstatat);
DEFINE_INTERN_ALIAS(libc_kw16fstatat,libc_kw16fstatat64);
DEFINE_INTERN_ALIAS(libc_kw32fstatat,libc_kw32fstatat64);
DEFINE_INTERN_ALIAS(libc_dos_kw16fstatat,libc_dos_kw16fstatat64);
DEFINE_INTERN_ALIAS(libc_dos_kw32fstatat,libc_dos_kw32fstatat64);

EXPORT(__KSYMw16(kwfstatat64),libc_kw16fstatat64);
EXPORT(__KSYMw32(kwfstatat64),libc_kw32fstatat64);
EXPORT(__DSYMw16(kwfstatat64),libc_dos_kw16fstatat64);
EXPORT(__DSYMw32(kwfstatat64),libc_dos_kw32fstatat64);
CRT_WIDECHAR int LIBCCALL
libc_kw16fstatat64(fd_t dfd, char16_t const *file,
                   struct __kos_stat64 *buf, int flags) {
 char filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(filebuf,file);
 if (!str) return -1;
 result = libc_kfstatat64(dfd,str,buf,flags);
 libc_freeutf(filebuf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_kw32fstatat64(fd_t dfd, char32_t const *file,
                   struct __kos_stat64 *buf, int flags) {
 char filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(filebuf,file);
 if (!str) return -1;
 result = libc_kfstatat64(dfd,str,buf,flags);
 libc_freeutf(filebuf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_dos_kw16fstatat64(fd_t dfd, char16_t const *file,
                       struct __kos_stat64 *buf, int flags) {
 return libc_kw16fstatat64(dfd,file,buf,flags|AT_DOSPATH);
}
CRT_WIDECHAR int LIBCCALL
libc_dos_kw32fstatat64(fd_t dfd, char32_t const *file,
                       struct __kos_stat64 *buf, int flags) {
 return libc_kw32fstatat64(dfd,file,buf,flags|AT_DOSPATH);
}

EXPORT(__KSYMw16(kwlstat),libc_kw16lstat);
EXPORT(__KSYMw32(kwlstat),libc_kw32lstat);
EXPORT(__DSYMw16(kwlstat),libc_dos_kw16lstat);
EXPORT(__DSYMw32(kwlstat),libc_dos_kw32lstat);
DEFINE_INTERN_ALIAS(libc_kw16lstat,libc_kw16lstat64);
DEFINE_INTERN_ALIAS(libc_kw32lstat,libc_kw32lstat64);
DEFINE_INTERN_ALIAS(libc_dos_kw16lstat,libc_dos_kw16lstat64);
DEFINE_INTERN_ALIAS(libc_dos_kw32lstat,libc_dos_kw32lstat64);

EXPORT(__KSYMw16(kwlstat64),libc_kw16lstat64);
EXPORT(__KSYMw32(kwlstat64),libc_kw32lstat64);
EXPORT(__DSYMw16(kwlstat64),libc_dos_kw16lstat64);
EXPORT(__DSYMw32(kwlstat64),libc_dos_kw32lstat64);
CRT_WIDECHAR int LIBCCALL libc_kw16lstat64(char16_t const *file, struct __kos_stat64 *buf) { return libc_kw16fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR int LIBCCALL libc_kw32lstat64(char32_t const *file, struct __kos_stat64 *buf) { return libc_kw32fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR int LIBCCALL libc_dos_kw16lstat64(char16_t const *file, struct __kos_stat64 *buf) { return libc_kw16fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_kw32lstat64(char32_t const *file, struct __kos_stat64 *buf) { return libc_kw32fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }

EXPORT(__SYMw16(Xkwstat),libc_Xkw16stat);
EXPORT(__SYMw32(Xkwstat),libc_Xkw32stat);
DEFINE_INTERN_ALIAS(libc_Xkw16stat,libc_Xkw16stat64);
DEFINE_INTERN_ALIAS(libc_Xkw32stat,libc_Xkw32stat64);

EXPORT(__SYMw16(Xkwstat64),libc_Xkw16stat64);
EXPORT(__SYMw32(Xkwstat64),libc_Xkw32stat64);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xkw16stat64(char16_t const *file, struct __kos_stat64 *buf) { libc_Xkw16fstatat64(AT_FDCWD,file,buf,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xkw32stat64(char32_t const *file, struct __kos_stat64 *buf) { libc_Xkw32fstatat64(AT_FDCWD,file,buf,0); }

EXPORT(__SYMw16(Xkwfstatat),libc_Xkw16fstatat);
EXPORT(__SYMw32(Xkwfstatat),libc_Xkw32fstatat);
DEFINE_INTERN_ALIAS(libc_Xkw16fstatat,libc_Xkw16fstatat64);
DEFINE_INTERN_ALIAS(libc_Xkw32fstatat,libc_Xkw32fstatat64);

EXPORT(__SYMw16(Xkwfstatat64),libc_Xkw16fstatat64);
EXPORT(__SYMw32(Xkwfstatat64),libc_Xkw32fstatat64);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xkw16fstatat64(fd_t dfd, char16_t const *file,
                    struct __kos_stat64 *buf, int flags) {
 char filebuf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(filebuf,file);
 LIBC_TRY {
  libc_Xkfstatat64(dfd,str,buf,flags);
 } LIBC_FINALLY {
  libc_freeutf(filebuf,str);
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xkw32fstatat64(fd_t dfd, char32_t const *file,
                    struct __kos_stat64 *buf, int flags) {
 char filebuf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(filebuf,file);
 LIBC_TRY {
  libc_Xkfstatat64(dfd,str,buf,flags);
 } LIBC_FINALLY {
  libc_freeutf(filebuf,str);
 }
}

EXPORT(__SYMw16(Xkwlstat),libc_Xkw16lstat);
EXPORT(__SYMw32(Xkwlstat),libc_Xkw32lstat);
DEFINE_INTERN_ALIAS(libc_Xkw16lstat,libc_Xkw16lstat64);
DEFINE_INTERN_ALIAS(libc_Xkw32lstat,libc_Xkw32lstat64);

EXPORT(__SYMw16(Xkwlstat64),libc_Xkw16lstat64);
EXPORT(__SYMw32(Xkwlstat64),libc_Xkw32lstat64);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xkw16lstat64(char16_t const *file, struct __kos_stat64 *buf) { libc_Xkw16fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xkw32lstat64(char32_t const *file, struct __kos_stat64 *buf) { libc_Xkw32fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW); }



CRT_WIDECHAR int LIBCCALL
libc_w16frevokeat(fd_t dfd, char16_t const *file, int flags) {
 char filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(filebuf,file);
 if (!str) return -1;
 result = libc_frevokeat(dfd,str,flags);
 libc_freeutf(filebuf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_w32frevokeat(fd_t dfd, char32_t const *file, int flags) {
 char filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(filebuf,file);
 if (!str) return -1;
 result = libc_frevokeat(dfd,str,flags);
 libc_freeutf(filebuf,str);
 return result;
}

CRT_WIDECHAR int LIBCCALL
libc_w16facctat(fd_t dfd, char16_t const *file, int flags) {
 char *str,filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 if (!file) return libc_facctat(dfd,NULL,flags);
 str = libc_loadutf16(filebuf,file);
 if (!str) return -1;
 result = libc_facctat(dfd,str,flags);
 libc_freeutf(filebuf,str);
 return result;
}
CRT_WIDECHAR int LIBCCALL
libc_w32facctat(fd_t dfd, char32_t const *file, int flags) {
 char *str,filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 if (!file) return libc_facctat(dfd,NULL,flags);
 str = libc_loadutf32(filebuf,file);
 if (!str) return -1;
 result = libc_facctat(dfd,str,flags);
 libc_freeutf(filebuf,str);
 return result;
}

EXPORT(__KSYMw16(wrevoke),libc_w16revoke);
EXPORT(__KSYMw32(wrevoke),libc_w32revoke);
EXPORT(__DSYMw16(wrevoke),libc_dos_w16revoke);
EXPORT(__DSYMw32(wrevoke),libc_dos_w32revoke);
CRT_WIDECHAR int LIBCCALL libc_w16revoke(char16_t const *file) { return libc_w16frevokeat(AT_FDCWD,file,0); }
CRT_WIDECHAR int LIBCCALL libc_w32revoke(char32_t const *file) { return libc_w32frevokeat(AT_FDCWD,file,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16revoke(char16_t const *file) { return libc_w16frevokeat(AT_FDCWD,file,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32revoke(char32_t const *file) { return libc_w32frevokeat(AT_FDCWD,file,AT_DOSPATH); }

EXPORT(__KSYMw16(wacct),libc_w16acct);
EXPORT(__KSYMw32(wacct),libc_w32acct);
EXPORT(__DSYMw16(wacct),libc_dos_w16acct);
EXPORT(__DSYMw32(wacct),libc_dos_w32acct);
CRT_WIDECHAR int LIBCCALL libc_w16acct(char16_t const *name) { return libc_w16facctat(AT_FDCWD,name,0); }
CRT_WIDECHAR int LIBCCALL libc_w32acct(char32_t const *name) { return libc_w32facctat(AT_FDCWD,name,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16acct(char16_t const *name) { return libc_w16facctat(AT_FDCWD,name,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32acct(char32_t const *name) { return libc_w32facctat(AT_FDCWD,name,AT_DOSPATH); }

EXPORT(__SYMw16(Xwrevoke),libc_Xw16revoke);
EXPORT(__SYMw32(Xwrevoke),libc_Xw32revoke);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16revoke(char16_t const *file) {
 char filebuf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(filebuf,file);
 LIBC_TRY {
  libc_Xrevoke(str);
 } LIBC_FINALLY {
  libc_freeutf(filebuf,str);
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32revoke(char32_t const *file) {
 char filebuf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(filebuf,file);
 LIBC_TRY {
  libc_Xrevoke(str);
 } LIBC_FINALLY {
  libc_freeutf(filebuf,str);
 }
}
EXPORT(__SYMw16(Xwacct),libc_Xw16acct);
EXPORT(__SYMw32(Xwacct),libc_Xw32acct);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16acct(char16_t const *file) {
 char *str,filebuf[UTF_STACK_BUFFER_SIZE];
 if (!file) {
  libc_Xacct(NULL);
 } else {
  str = libc_Xloadutf16(filebuf,file);
  LIBC_TRY {
   libc_Xacct(str);
  } LIBC_FINALLY {
   libc_freeutf(filebuf,str);
  }
 }
}
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32acct(char32_t const *file) {
 char *str,filebuf[UTF_STACK_BUFFER_SIZE];
 if (!file) {
  libc_Xacct(NULL);
 } else {
  str = libc_Xloadutf32(filebuf,file);
  LIBC_TRY {
   libc_Xacct(str);
  } LIBC_FINALLY {
   libc_freeutf(filebuf,str);
  }
 }
}


EXPORT(__KSYMw16(wsystem),libc_w16system);
INTERN int LIBCCALL
libc_w16system(char16_t const *command) {
 char *str,filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 if (!command) return libc_system(NULL);
 str = libc_loadutf16(filebuf,command);
 if (!str) return -1;
 result = libc_system(str);
 libc_freeutf(filebuf,str);
 return result;
}
EXPORT(__KSYMw32(wsystem),libc_w32system);
INTERN int LIBCCALL
libc_w32system(char32_t const *command) {
 char *str,filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 if (!command) return libc_system(NULL);
 str = libc_loadutf32(filebuf,command);
 if (!str) return -1;
 result = libc_system(str);
 libc_freeutf(filebuf,str);
 return result;
}

EXPORT(__DSYMw16(_wsystem),libc_dos_w16system);
INTERN int LIBCCALL
libc_dos_w16system(char16_t const *command) {
 char *str,filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 if (!command) return libc_system(NULL);
 str = libc_loadutf16(filebuf,command);
 if (!str) return -1;
 result = libc_dos_system(str);
 libc_freeutf(filebuf,str);
 return result;
}
EXPORT(__DSYMw32(wsystem),libc_dos_w32system);
INTERN int LIBCCALL
libc_dos_w32system(char32_t const *command) {
 char *str,filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 if (!command) return libc_system(NULL);
 str = libc_loadutf32(filebuf,command);
 if (!str) return -1;
 result = libc_dos_system(str);
 libc_freeutf(filebuf,str);
 return result;
}

EXPORT(__SYMw16(Xwsystem),libc_Xw16system);
INTERN int LIBCCALL
libc_Xw16system(char16_t const *command) {
 int COMPILER_IGNORE_UNINITIALIZED(result);
 char *str,filebuf[UTF_STACK_BUFFER_SIZE];
 if (!command) {
  result = libc_Xsystem(NULL);
 } else {
  str = libc_Xloadutf16(filebuf,command);
  LIBC_TRY {
   result = libc_Xsystem(str);
  } LIBC_FINALLY {
   libc_freeutf(filebuf,str);
  }
 }
 return result;
}

EXPORT(__SYMw32(Xwsystem),libc_Xw32system);
INTERN int LIBCCALL
libc_Xw32system(char32_t const *command) {
 int COMPILER_IGNORE_UNINITIALIZED(result);
 char *str,filebuf[UTF_STACK_BUFFER_SIZE];
 if (!command) {
  result = libc_Xsystem(NULL);
 } else {
  str = libc_Xloadutf32(filebuf,command);
  LIBC_TRY {
   result = libc_Xsystem(str);
  } LIBC_FINALLY {
   libc_freeutf(filebuf,str);
  }
 }
 return result;
}

EXPORT(__KSYMw16(wsopen),libc_w16sopen);
CRT_DOS WUNUSED int ATTR_CDECL
libc_w16sopen(char16_t const *file, int oflag, int sflag, ...) {
 va_list args; int result;
 va_start(args,sflag);
 result = libc_w16open(file,oflag,va_arg(args,mode_t));
 va_end(args);
 return result;
}
EXPORT(__KSYMw32(wsopen),libc_w32sopen);
CRT_DOS WUNUSED int ATTR_CDECL
libc_w32sopen(char32_t const *file, int oflag, int sflag, ...) {
 va_list args; int result;
 va_start(args,sflag);
 result = libc_w32open(file,oflag,va_arg(args,mode_t));
 va_end(args);
 return result;
}
EXPORT(__DSYMw16(_wsopen),libc_dos_w16sopen);
EXPORT("DOS$?_wsopen%%YAHPEB_WHHH%Z",libc_dos_w16sopen);
CRT_DOS WUNUSED int ATTR_CDECL
libc_dos_w16sopen(char16_t const *file, int oflag, int sflag, ...) {
 va_list args; int result;
 va_start(args,sflag);
 result = libc_dos_w16open(file,oflag,va_arg(args,mode_t));
 va_end(args);
 return result;
}
EXPORT(__DSYMw32(wsopen),libc_dos_w32sopen);
CRT_DOS WUNUSED int ATTR_CDECL
libc_dos_w32sopen(char32_t const *file, int oflag, int sflag, ...) {
 va_list args; int result;
 va_start(args,sflag);
 result = libc_dos_w32open(file,oflag,va_arg(args,mode_t));
 va_end(args);
 return result;
}
EXPORT(__KSYMw16(wsopen_s),libc_w16sopen_s);
CRT_DOS WUNUSED errno_t ATTR_CDECL
libc_w16sopen_s(int *fd, char16_t const *file, int oflag, int sflag, ...) {
 va_list args; int result;
 if (!fd) return EINVAL;
 va_start(args,sflag);
 result = libc_w16open(file,oflag,va_arg(args,mode_t));
 va_end(args);
 *fd = result;
 return result >= 0 ? 0 : libc_geterrno();
}
EXPORT(__KSYMw32(wsopen_s),libc_w32sopen_s);
CRT_DOS WUNUSED errno_t ATTR_CDECL
libc_w32sopen_s(int *fd, char32_t const *file, int oflag, int sflag, ...) {
 va_list args; int result;
 if (!fd) return EINVAL;
 va_start(args,sflag);
 result = libc_w32open(file,oflag,va_arg(args,mode_t));
 va_end(args);
 *fd = result;
 return result >= 0 ? 0 : libc_geterrno();
}
EXPORT(__DSYMw16(_wsopen_s),libc_dos_w16sopen_s);
CRT_DOS WUNUSED derrno_t ATTR_CDECL
libc_dos_w16sopen_s(int *fd, char16_t const *file, int oflag, int sflag, ...) {
 va_list args; int result;
 if (!fd) return __DOS_EINVAL;
 va_start(args,sflag);
 result = libc_dos_w16open(file,oflag,va_arg(args,mode_t));
 va_end(args);
 *fd = result;
 return result >= 0 ? 0 : libc_dos_geterrno();
}
EXPORT(__DSYMw32(wsopen_s),libc_dos_w32sopen_s);
CRT_DOS WUNUSED derrno_t ATTR_CDECL
libc_dos_w32sopen_s(int *fd, char32_t const *file, int oflag, int sflag, ...) {
 va_list args; int result;
 if (!fd) return __DOS_EINVAL;
 va_start(args,sflag);
 result = libc_dos_w32open(file,oflag,va_arg(args,mode_t));
 va_end(args);
 *fd = result;
 return result >= 0 ? 0 : libc_dos_geterrno();
}





EXPORT(__KSYMw16(wutimensat),libc_w16utimensat);
CRT_WIDECHAR int LIBCCALL
libc_w16utimensat(fd_t dfd, char16_t const *path,
                  struct timespec32 const times[2], int flags) {
 char filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(filebuf,path);
 if (!str) return -1;
 result = libc_utimensat(dfd,str,times,flags);
 libc_freeutf(filebuf,str);
 return result;
}
EXPORT(__KSYMw32(wutimensat),libc_w32utimensat);
CRT_WIDECHAR int LIBCCALL
libc_w32utimensat(fd_t dfd, char32_t const *path,
                  struct timespec32 const times[2], int flags) {
 char filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(filebuf,path);
 if (!str) return -1;
 result = libc_utimensat(dfd,str,times,flags);
 libc_freeutf(filebuf,str);
 return result;
}
EXPORT(__DSYMw16(wutimensat),libc_dos_w16utimensat);
CRT_WIDECHAR int LIBCCALL
libc_dos_w16utimensat(fd_t dfd, char16_t const *path,
                      struct timespec32 const times[2], int flags) {
 return libc_w16utimensat(dfd,path,times,flags|AT_DOSPATH);
}
EXPORT(__DSYMw32(wutimensat),libc_dos_w32utimensat);
CRT_WIDECHAR int LIBCCALL
libc_dos_w32utimensat(fd_t dfd, char32_t const *path,
                      struct timespec32 const times[2], int flags) {
 return libc_w32utimensat(dfd,path,times,flags|AT_DOSPATH);
}
EXPORT(__KSYMw16(wutimensat64),libc_w16utimensat64);
CRT_WIDECHAR int LIBCCALL
libc_w16utimensat64(fd_t dfd, char16_t const *path,
                    struct timespec64 const times[2], int flags) {
 char filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(filebuf,path);
 if (!str) return -1;
 result = libc_utimensat64(dfd,str,times,flags);
 libc_freeutf(filebuf,str);
 return result;
}
EXPORT(__KSYMw32(wutimensat64),libc_w32utimensat64);
CRT_WIDECHAR int LIBCCALL
libc_w32utimensat64(fd_t dfd, char32_t const *path,
                    struct timespec64 const times[2], int flags) {
 char filebuf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(filebuf,path);
 if (!str) return -1;
 result = libc_utimensat64(dfd,str,times,flags);
 libc_freeutf(filebuf,str);
 return result;
}
EXPORT(__DSYMw16(wutimensat64),libc_dos_w16utimensat64);
CRT_WIDECHAR int LIBCCALL
libc_dos_w16utimensat64(fd_t dfd, char16_t const *path,
                        struct timespec64 const times[2], int flags) {
 return libc_w16utimensat64(dfd,path,times,flags|AT_DOSPATH);
}
EXPORT(__DSYMw32(wutimensat64),libc_dos_w32utimensat64);
CRT_WIDECHAR int LIBCCALL
libc_dos_w32utimensat64(fd_t dfd, char32_t const *path,
                        struct timespec64 const times[2], int flags) {
 return libc_w32utimensat64(dfd,path,times,flags|AT_DOSPATH);
}

EXPORT(__KSYMw16(wfutimeat64),libc_w16futimeat64);
CRT_WIDECHAR int LIBCCALL
libc_w16futimeat64(fd_t dfd, char16_t const *file, struct utimbuf64 const *file_times, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf16(buf,file);
 if (!str) return -1;
 result = libc_futimeat64(dfd,str,file_times,flags);
 libc_freeutf(buf,str);
 return result;
}
EXPORT(__KSYMw32(wfutimeat64),libc_w32futimeat64);
CRT_WIDECHAR int LIBCCALL
libc_w32futimeat64(fd_t dfd, char32_t const *file, struct utimbuf64 const *file_times, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE]; int result;
 char *str = libc_loadutf32(buf,file);
 if (!str) return -1;
 result = libc_futimeat64(dfd,str,file_times,flags);
 libc_freeutf(buf,str);
 return result;
}
EXPORT(__KSYMw16(wfutimeat),libc_w16futimeat);
CRT_WIDECHAR int LIBCCALL
libc_w16futimeat(fd_t dfd, char16_t const *file, struct utimbuf32 const *file_times, int flags) {
 struct utimbuf64 buf64;
 if (!file_times)
      return libc_w16futimeat64(dfd,file,NULL,flags);
 buf64.actime  = file_times->actime;
 buf64.modtime = file_times->modtime;
 return libc_w16futimeat64(dfd,file,&buf64,flags);
}
EXPORT(__KSYMw32(wfutimeat),libc_w32futimeat);
CRT_WIDECHAR int LIBCCALL
libc_w32futimeat(fd_t dfd, char32_t const *file, struct utimbuf32 const *file_times, int flags) {
 struct utimbuf64 buf64;
 if (!file_times)
      return libc_w32futimeat64(dfd,file,NULL,flags);
 buf64.actime  = file_times->actime;
 buf64.modtime = file_times->modtime;
 return libc_w32futimeat64(dfd,file,&buf64,flags);
}

EXPORT(__DSYMw16(wfutimeat),libc_dos_w16futimeat);
CRT_WIDECHAR int LIBCCALL
libc_dos_w16futimeat(fd_t dfd, char16_t const *file,
                     struct utimbuf32 const *file_times, int flags) {
 return libc_w16futimeat(dfd,file,file_times,flags|AT_DOSPATH);
}
EXPORT(__DSYMw32(wfutimeat),libc_dos_w32futimeat);
CRT_WIDECHAR int LIBCCALL
libc_dos_w32futimeat(fd_t dfd, char32_t const *file,
                     struct utimbuf32 const *file_times, int flags) {
 return libc_w32futimeat(dfd,file,file_times,flags|AT_DOSPATH);
}
EXPORT(__DSYMw16(wfutimeat64),libc_dos_w16futimeat64);
CRT_WIDECHAR int LIBCCALL
libc_dos_w16futimeat64(fd_t dfd, char16_t const *file,
                       struct utimbuf64 const *file_times, int flags) {
 return libc_w16futimeat64(dfd,file,file_times,flags|AT_DOSPATH);
}
EXPORT(__DSYMw32(wfutimeat64),libc_dos_w32futimeat64);
CRT_WIDECHAR int LIBCCALL
libc_dos_w32futimeat64(fd_t dfd, char32_t const *file,
                       struct utimbuf64 const *file_times, int flags) {
 return libc_w32futimeat64(dfd,file,file_times,flags|AT_DOSPATH);
}
EXPORT(__KSYMw16(wutime),libc_w16utime);
CRT_WIDECHAR int LIBCCALL
libc_w16utime(char16_t const *file, struct utimbuf32 const *file_times) {
 return libc_w16futimeat(AT_FDCWD,file,file_times,0);
}
EXPORT(__KSYMw32(wutime),libc_w32utime);
CRT_WIDECHAR int LIBCCALL
libc_w32utime(char32_t const *file, struct utimbuf32 const *file_times) {
 return libc_w32futimeat(AT_FDCWD,file,file_times,0);
}
EXPORT(__DSYMw16(wutime),libc_dos_w16utime);
CRT_WIDECHAR int LIBCCALL
libc_dos_w16utime(char16_t const *file, struct utimbuf32 const *file_times) {
 return libc_w16futimeat(AT_FDCWD,file,file_times,AT_DOSPATH);
}
EXPORT(__DSYMw32(wutime),libc_dos_w32utime);
CRT_WIDECHAR int LIBCCALL
libc_dos_w32utime(char32_t const *file, struct utimbuf32 const *file_times) {
 return libc_w32futimeat(AT_FDCWD,file,file_times,AT_DOSPATH);
}
EXPORT(__KSYMw16(wutime64),libc_w16utime64);
CRT_WIDECHAR int LIBCCALL
libc_w16utime64(char16_t const *file, struct utimbuf64 const *file_times) {
 return libc_w16futimeat64(AT_FDCWD,file,file_times,0);
}
EXPORT(__KSYMw32(wutime64),libc_w32utime64);
CRT_WIDECHAR int LIBCCALL
libc_w32utime64(char32_t const *file, struct utimbuf64 const *file_times) {
 return libc_w32futimeat64(AT_FDCWD,file,file_times,0);
}
EXPORT(__DSYMw16(wutime64),libc_dos_w16utime64);
CRT_WIDECHAR int LIBCCALL
libc_dos_w16utime64(char16_t const *file, struct utimbuf64 const *file_times) {
 return libc_w16futimeat64(AT_FDCWD,file,file_times,AT_DOSPATH);
}
EXPORT(__DSYMw32(wutime64),libc_dos_w32utime64);
CRT_WIDECHAR int LIBCCALL
libc_dos_w32utime64(char32_t const *file, struct utimbuf64 const *file_times) {
 return libc_w32futimeat64(AT_FDCWD,file,file_times,AT_DOSPATH);
}


CRT_WIDECHAR int LIBCCALL
libc_impl_w16futimesat(fd_t dfd, char16_t const *file,
                       struct timeval32 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 return libc_w16utimensat64(dfd,file,times,flags);
}
CRT_WIDECHAR int LIBCCALL
libc_impl_w32futimesat(fd_t dfd, char32_t const *file,
                       struct timeval32 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 return libc_w32utimensat64(dfd,file,times,flags);
}
CRT_WIDECHAR int LIBCCALL
libc_impl_w16futimesat64(fd_t dfd, char16_t const *file,
                         struct timeval64 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 return libc_w16utimensat64(dfd,file,times,flags);
}
CRT_WIDECHAR int LIBCCALL
libc_impl_w32futimesat64(fd_t dfd, char32_t const *file,
                         struct timeval64 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 return libc_w32utimensat64(dfd,file,times,flags);
}

EXPORT(__KSYMw16(wutimes),libc_w16utimes);
EXPORT(__KSYMw32(wutimes),libc_w32utimes);
EXPORT(__DSYMw16(wutimes),libc_dos_w16utimes);
EXPORT(__DSYMw32(wutimes),libc_dos_w32utimes);
EXPORT(__KSYMw16(wutimes64),libc_w16utimes64);
EXPORT(__KSYMw32(wutimes64),libc_w32utimes64);
EXPORT(__DSYMw16(wutimes64),libc_dos_w16utimes64);
EXPORT(__DSYMw32(wutimes64),libc_dos_w32utimes64);
EXPORT(__KSYMw16(wlutimes),libc_w16lutimes);
EXPORT(__KSYMw32(wlutimes),libc_w32lutimes);
EXPORT(__DSYMw16(wlutimes),libc_dos_w16lutimes);
EXPORT(__DSYMw32(wlutimes),libc_dos_w32lutimes);
EXPORT(__KSYMw16(wlutimes64),libc_w16lutimes64);
EXPORT(__KSYMw32(wlutimes64),libc_w32lutimes64);
EXPORT(__DSYMw16(wlutimes64),libc_dos_w16lutimes64);
EXPORT(__DSYMw32(wlutimes64),libc_dos_w32lutimes64);
EXPORT(__KSYMw16(wfutimesat),libc_w16futimesat);
EXPORT(__KSYMw32(wfutimesat),libc_w32futimesat);
EXPORT(__DSYMw16(wfutimesat),libc_dos_w16futimesat);
EXPORT(__DSYMw32(wfutimesat),libc_dos_w32futimesat);
EXPORT(__KSYMw16(wfutimesat64),libc_w16futimesat64);
EXPORT(__KSYMw32(wfutimesat64),libc_w32futimesat64);
EXPORT(__DSYMw16(wfutimesat64),libc_dos_w16futimesat64);
EXPORT(__DSYMw32(wfutimesat64),libc_dos_w32futimesat64);
CRT_WIDECHAR int LIBCCALL libc_w16utimes(char16_t const *file, struct timeval32 const tvp[2]) { return libc_impl_w16futimesat(AT_FDCWD,file,tvp,0); }
CRT_WIDECHAR int LIBCCALL libc_w32utimes(char32_t const *file, struct timeval32 const tvp[2]) { return libc_impl_w32futimesat(AT_FDCWD,file,tvp,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16utimes(char16_t const *file, struct timeval32 const tvp[2]) { return libc_impl_w16futimesat(AT_FDCWD,file,tvp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32utimes(char32_t const *file, struct timeval32 const tvp[2]) { return libc_impl_w32futimesat(AT_FDCWD,file,tvp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_w16utimes64(char16_t const *file, struct timeval64 const tvp[2]) { return libc_impl_w16futimesat64(AT_FDCWD,file,tvp,0); }
CRT_WIDECHAR int LIBCCALL libc_w32utimes64(char32_t const *file, struct timeval64 const tvp[2]) { return libc_impl_w32futimesat64(AT_FDCWD,file,tvp,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16utimes64(char16_t const *file, struct timeval64 const tvp[2]) { return libc_impl_w16futimesat64(AT_FDCWD,file,tvp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32utimes64(char32_t const *file, struct timeval64 const tvp[2]) { return libc_impl_w32futimesat64(AT_FDCWD,file,tvp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_w16lutimes(char16_t const *file, struct timeval32 const tvp[2]) { return libc_impl_w16futimesat(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR int LIBCCALL libc_w32lutimes(char32_t const *file, struct timeval32 const tvp[2]) { return libc_impl_w32futimesat(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16lutimes(char16_t const *file, struct timeval32 const tvp[2]) { return libc_impl_w16futimesat(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32lutimes(char32_t const *file, struct timeval32 const tvp[2]) { return libc_impl_w32futimesat(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_w16lutimes64(char16_t const *file, struct timeval64 const tvp[2]) { return libc_impl_w16futimesat64(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR int LIBCCALL libc_w32lutimes64(char32_t const *file, struct timeval64 const tvp[2]) { return libc_impl_w32futimesat64(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16lutimes64(char16_t const *file, struct timeval64 const tvp[2]) { return libc_impl_w16futimesat64(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32lutimes64(char32_t const *file, struct timeval64 const tvp[2]) { return libc_impl_w32futimesat64(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_w16futimesat(fd_t dfd, char16_t const *file, struct timeval32 const tvp[2]) { return libc_impl_w16futimesat(dfd,file,tvp,0); }
CRT_WIDECHAR int LIBCCALL libc_w32futimesat(fd_t dfd, char32_t const *file, struct timeval32 const tvp[2]) { return libc_impl_w32futimesat(dfd,file,tvp,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16futimesat(fd_t dfd, char16_t const *file, struct timeval32 const tvp[2]) { return libc_impl_w16futimesat(dfd,file,tvp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32futimesat(fd_t dfd, char32_t const *file, struct timeval32 const tvp[2]) { return libc_impl_w32futimesat(dfd,file,tvp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_w16futimesat64(fd_t dfd, char16_t const *file, struct timeval64 const tvp[2]) { return libc_impl_w16futimesat64(dfd,file,tvp,0); }
CRT_WIDECHAR int LIBCCALL libc_w32futimesat64(fd_t dfd, char32_t const *file, struct timeval64 const tvp[2]) { return libc_impl_w32futimesat64(dfd,file,tvp,0); }
CRT_WIDECHAR int LIBCCALL libc_dos_w16futimesat64(fd_t dfd, char16_t const *file, struct timeval64 const tvp[2]) { return libc_impl_w16futimesat64(dfd,file,tvp,AT_DOSPATH); }
CRT_WIDECHAR int LIBCCALL libc_dos_w32futimesat64(fd_t dfd, char32_t const *file, struct timeval64 const tvp[2]) { return libc_impl_w32futimesat64(dfd,file,tvp,AT_DOSPATH); }

EXPORT(__KSYMw16(_wutime),libc_w16utime); /* Alias also defined by DOS. */
EXPORT(__KSYMw32(_wutime),libc_w32utime); /* Alias also defined by DOS. */
EXPORT(__DSYMw16(_wutime),libc_dos_w16utime); /* Alias also defined by DOS. */
EXPORT(__DSYMw32(_wutime),libc_dos_w32utime); /* Alias also defined by DOS. */

/* DOS-compatible wide character versions */
EXPORT(__KSYMw16(_wutime32),libc_w16utime);
EXPORT(__KSYMw32(_wutime32),libc_w32utime);
EXPORT(__DSYMw16(_wutime32),libc_dos_w16utime);
EXPORT(__DSYMw32(_wutime32),libc_dos_w32utime);
EXPORT(__KSYMw16(_wutime64),libc_w16utime64);
EXPORT(__KSYMw32(_wutime64),libc_w32utime64);
EXPORT(__DSYMw16(_wutime64),libc_dos_w16utime64);
EXPORT(__DSYMw32(_wutime64),libc_dos_w32utime64);

EXPORT(__SYMw16(Xwutimensat),libc_Xw16utimensat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16utimensat(fd_t dfd, char16_t const *path,
                   struct timespec32 const times[2], int flags) {
 if (!times) {
  libc_Xw16utimensat64(dfd,path,NULL,flags);
 } else {
  struct timespec64 t64[3];
  t64[0].tv_sec  = times[0].tv_sec;
  t64[0].tv_nsec = times[0].tv_nsec;
  t64[1].tv_sec  = times[1].tv_sec;
  t64[1].tv_nsec = times[1].tv_nsec;
  if (flags & AT_CHANGE_CTIME) {
   t64[2].tv_sec  = times[2].tv_sec;
   t64[2].tv_nsec = times[2].tv_nsec;
  }
  libc_Xw16utimensat64(dfd,path,t64,flags);
 }
}
EXPORT(__SYMw32(Xwutimensat),libc_Xw32utimensat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32utimensat(fd_t dfd, char32_t const *path,
                   struct timespec32 const times[2], int flags) {
 if (!times) {
  libc_Xw32utimensat64(dfd,path,NULL,flags);
 } else {
  struct timespec64 t64[3];
  t64[0].tv_sec  = times[0].tv_sec;
  t64[0].tv_nsec = times[0].tv_nsec;
  t64[1].tv_sec  = times[1].tv_sec;
  t64[1].tv_nsec = times[1].tv_nsec;
  if (flags & AT_CHANGE_CTIME) {
   t64[2].tv_sec  = times[2].tv_sec;
   t64[2].tv_nsec = times[2].tv_nsec;
  }
  libc_Xw32utimensat64(dfd,path,t64,flags);
 }
}
EXPORT(__SYMw16(Xwutimensat64),libc_Xw16utimensat64);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16utimensat64(fd_t dfd, char16_t const *path,
                     struct timespec64 const times[2], int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(buf,path);
 LIBC_TRY {
  libc_Xutimensat64(dfd,str,times,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}
EXPORT(__SYMw32(Xwutimensat64),libc_Xw32utimensat64);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32utimensat64(fd_t dfd, char32_t const *path,
                     struct timespec64 const times[2], int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(buf,path);
 LIBC_TRY {
  libc_Xutimensat64(dfd,str,times,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}

EXPORT(__SYMw16(Xwfutimeat64),libc_Xw16futimeat64);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16futimeat64(fd_t dfd, char16_t const *file, struct utimbuf64 const *file_times, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf16(buf,file);
 LIBC_TRY {
  libc_Xfutimeat64(dfd,str,file_times,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}
EXPORT(__SYMw32(Xwfutimeat64),libc_Xw32futimeat64);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32futimeat64(fd_t dfd, char32_t const *file, struct utimbuf64 const *file_times, int flags) {
 char buf[UTF_STACK_BUFFER_SIZE];
 char *str = libc_Xloadutf32(buf,file);
 LIBC_TRY {
  libc_Xfutimeat64(dfd,str,file_times,flags);
 } LIBC_FINALLY {
  libc_freeutf(buf,str);
 }
}
EXPORT(__SYMw16(wfutimeat),libc_Xw16futimeat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16futimeat(fd_t dfd, char16_t const *file, struct utimbuf32 const *file_times, int flags) {
 if (!file_times) {
  libc_Xw16futimeat64(dfd,file,NULL,flags);
 } else {
  struct utimbuf64 buf64;
  buf64.actime  = file_times->actime;
  buf64.modtime = file_times->modtime;
  libc_Xw16futimeat64(dfd,file,&buf64,flags);
 }
}
EXPORT(__SYMw32(Xwfutimeat),libc_Xw32futimeat);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32futimeat(fd_t dfd, char32_t const *file, struct utimbuf32 const *file_times, int flags) {
 if (!file_times) {
  libc_Xw32futimeat64(dfd,file,NULL,flags);
 } else {
  struct utimbuf64 buf64;
  buf64.actime  = file_times->actime;
  buf64.modtime = file_times->modtime;
  libc_Xw32futimeat64(dfd,file,&buf64,flags);
 }
}

EXPORT(__SYMw16(Xwutime),libc_Xw16utime);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16utime(char16_t const *file, struct utimbuf32 const *file_times) {
 libc_Xw16futimeat(AT_FDCWD,file,file_times,0);
}
EXPORT(__SYMw32(Xwutime),libc_Xw32utime);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32utime(char32_t const *file, struct utimbuf32 const *file_times) {
 libc_Xw32futimeat(AT_FDCWD,file,file_times,0);
}
EXPORT(__SYMw16(Xwutime64),libc_Xw16utime64);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw16utime64(char16_t const *file, struct utimbuf64 const *file_times) {
 libc_Xw16futimeat64(AT_FDCWD,file,file_times,0);
}
EXPORT(__SYMw32(Xwutime64),libc_Xw32utime64);
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_Xw32utime64(char32_t const *file, struct utimbuf64 const *file_times) {
 libc_Xw32futimeat64(AT_FDCWD,file,file_times,0);
}



CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_impl_Xw16futimesat(fd_t dfd, char16_t const *file,
                        struct timeval32 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 libc_Xw16utimensat64(dfd,file,times,flags);
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_impl_Xw32futimesat(fd_t dfd, char32_t const *file,
                        struct timeval32 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 libc_Xw32utimensat64(dfd,file,times,flags);
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_impl_Xw16futimesat64(fd_t dfd, char16_t const *file,
                          struct timeval64 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 libc_Xw16utimensat64(dfd,file,times,flags);
}
CRT_WIDECHAR_EXCEPT void LIBCCALL
libc_impl_Xw32futimesat64(fd_t dfd, char32_t const *file,
                          struct timeval64 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 libc_Xw32utimensat64(dfd,file,times,flags);
}



EXPORT(__SYMw16(Xwutimes),libc_Xw16utimes);
EXPORT(__SYMw32(Xwutimes),libc_Xw32utimes);
EXPORT(__SYMw16(Xwutimes64),libc_Xw16utimes64);
EXPORT(__SYMw32(Xwutimes64),libc_Xw32utimes64);
EXPORT(__SYMw16(Xwlutimes),libc_Xw16lutimes);
EXPORT(__SYMw32(Xwlutimes),libc_Xw32lutimes);
EXPORT(__SYMw16(Xwlutimes64),libc_Xw16lutimes64);
EXPORT(__SYMw32(Xwlutimes64),libc_Xw32lutimes64);
EXPORT(__SYMw16(Xwfutimesat),libc_Xw16futimesat);
EXPORT(__SYMw32(Xwfutimesat),libc_Xw32futimesat);
EXPORT(__SYMw16(Xwfutimesat64),libc_Xw16futimesat64);
EXPORT(__SYMw32(Xwfutimesat64),libc_Xw32futimesat64);
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16utimes(char16_t const *file, struct timeval32 const tvp[2]) { libc_impl_Xw16futimesat(AT_FDCWD,file,tvp,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32utimes(char32_t const *file, struct timeval32 const tvp[2]) { libc_impl_Xw32futimesat(AT_FDCWD,file,tvp,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16utimes64(char16_t const *file, struct timeval64 const tvp[2]) { libc_impl_Xw16futimesat64(AT_FDCWD,file,tvp,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32utimes64(char32_t const *file, struct timeval64 const tvp[2]) { libc_impl_Xw32futimesat64(AT_FDCWD,file,tvp,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16lutimes(char16_t const *file, struct timeval32 const tvp[2]) { libc_impl_Xw16futimesat(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32lutimes(char32_t const *file, struct timeval32 const tvp[2]) { libc_impl_Xw32futimesat(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16lutimes64(char16_t const *file, struct timeval64 const tvp[2]) { libc_impl_Xw16futimesat64(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32lutimes64(char32_t const *file, struct timeval64 const tvp[2]) { libc_impl_Xw32futimesat64(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16futimesat(fd_t dfd, char16_t const *file, struct timeval32 const tvp[2]) { libc_impl_Xw16futimesat(dfd,file,tvp,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32futimesat(fd_t dfd, char32_t const *file, struct timeval32 const tvp[2]) { libc_impl_Xw32futimesat(dfd,file,tvp,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw16futimesat64(fd_t dfd, char16_t const *file, struct timeval64 const tvp[2]) { libc_impl_Xw16futimesat64(dfd,file,tvp,0); }
CRT_WIDECHAR_EXCEPT void LIBCCALL libc_Xw32futimesat64(fd_t dfd, char32_t const *file, struct timeval64 const tvp[2]) { libc_impl_Xw32futimesat64(dfd,file,tvp,0); }



DECL_END

#endif /* !GUARD_LIBS_LIBC_WIDECHAR_C */
