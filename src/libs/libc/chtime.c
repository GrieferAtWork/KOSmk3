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
#ifndef GUARD_LIBS_LIBC_CHTIME_C
#define GUARD_LIBS_LIBC_CHTIME_C 1
#define _GNU_SOURCE 1

#include "libc.h"
#include "chtime.h"
#include "system.h"
#include "errno.h"
#include "widechar.h"
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

DECL_BEGIN

/* FILESYSTEM TIME MODIFICATION. */
INTERN int LIBCCALL
libc_utimensat(fd_t dfd, char const *path, struct timespec32 const times[2], int flags) {
 struct timespec64 t64[3];
 if (!times) return libc_utimensat64(dfd,path,NULL,flags);
 t64[0].tv_sec  = times[0].tv_sec;
 t64[0].tv_nsec = times[0].tv_nsec;
 t64[1].tv_sec  = times[1].tv_sec;
 t64[1].tv_nsec = times[1].tv_nsec;
 if (flags & AT_CHANGE_CTIME) {
  t64[2].tv_sec  = times[2].tv_sec;
  t64[2].tv_nsec = times[2].tv_nsec;
 }
 return libc_utimensat64(dfd,path,t64,flags);
}
CRT_DOS_EXT int LIBCCALL
libc_dos_utimensat(fd_t dfd, char const *path,
                   struct timespec32 const times[2],
                   int flags) {
 return libc_utimensat(dfd,path,times,AT_DOSPATH|flags);
}
INTERN int LIBCCALL
libc_utimensat64(fd_t dfd, char const *path,
                 struct timespec64 const times[2], int flags) {
 return FORWARD_SYSTEM_ERROR(sys_utimensat(dfd,path,times,flags));
}
CRT_DOS_EXT int LIBCCALL
libc_dos_utimensat64(fd_t dfd, char const *path,
                     struct timespec64 const times[2], int flags) {
 return libc_utimensat64(dfd,path,times,AT_DOSPATH|flags);
}
INTERN int LIBCCALL
libc_futimens(fd_t fd, struct timespec32 const times[2]) {
 return libc_utimensat(fd,libc_empty_string,times,AT_EMPTY_PATH);
}
INTERN int LIBCCALL
libc_futimens64(fd_t fd, struct timespec64 const times[2]) {
 return libc_utimensat64(fd,libc_empty_string,times,AT_EMPTY_PATH);
}
INTERN int LIBCCALL
libc_futime(fd_t fd, struct utimbuf32 const *file_times) {
 return libc_futimeat(fd,libc_empty_string,file_times,0);
}
INTERN int LIBCCALL
libc_futime64(fd_t fd, struct utimbuf64 const *file_times) {
 return libc_futimeat64(fd,libc_empty_string,file_times,0);
}
INTERN int LIBCCALL
libc_futimes(fd_t fd, struct timeval32 const tvp[2]) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 return libc_futimens64(fd,times);
}
INTERN int LIBCCALL
libc_futimes64(fd_t fd, struct timeval64 const tvp[2]) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 return libc_futimens64(fd,times);
}
INTERN int LIBCCALL
libc_futimeat(fd_t dfd, char const *file,
              struct utimbuf32 const *file_times, int flags) {
 struct timespec64 times[2];
 if (!file_times) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  times[0].tv_sec  = file_times->actime;
  times[1].tv_sec  = file_times->modtime;
  times[0].tv_nsec = 0;
  times[1].tv_nsec = 0;
 }
 return libc_utimensat64(dfd,file,times,flags);
}
CRT_DOS_EXT int LIBCCALL
libc_dos_futimeat(fd_t dfd, char const *file,
                  struct utimbuf32 const *file_times, int flags) {
 return libc_futimeat(dfd,file,file_times,flags|AT_DOSPATH);
}
INTERN int LIBCCALL
libc_futimeat64(fd_t dfd, char const *file,
                struct utimbuf64 const *file_times, int flags) {
 struct timespec64 times[2];
 if (!file_times) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  times[0].tv_sec  = file_times->actime;
  times[1].tv_sec  = file_times->modtime;
  times[0].tv_nsec = 0;
  times[1].tv_nsec = 0;
 }
 return libc_utimensat64(dfd,file,times,flags);
}
CRT_DOS_EXT int LIBCCALL
libc_dos_futimeat64(fd_t dfd, char const *file, struct utimbuf64 const *file_times, int flags) {
 return libc_futimeat64(dfd,file,file_times,flags|AT_DOSPATH);
}
INTERN int LIBCCALL libc_utime(char const *file, struct utimbuf32 const *file_times) {
 return libc_futimeat(AT_FDCWD,file,file_times,0);
}
CRT_DOS_EXT int LIBCCALL
libc_dos_utime(char const *file, struct utimbuf32 const *file_times) {
 return libc_futimeat(AT_FDCWD,file,file_times,AT_DOSPATH);
}
INTERN int LIBCCALL libc_utime64(char const *file, struct utimbuf64 const *file_times) {
 return libc_futimeat64(AT_FDCWD,file,file_times,0);
}
CRT_DOS_EXT int LIBCCALL
libc_dos_utime64(char const *file, struct utimbuf64 const *file_times) {
 return libc_futimeat64(AT_FDCWD,file,file_times,AT_DOSPATH);
}
INTERN int LIBCCALL
libc_impl_futimesat64(fd_t dfd, char const *file,
                      struct timeval64 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 return libc_utimensat64(dfd,file,times,flags);
}
INTERN int LIBCCALL
libc_impl_futimesat(fd_t dfd, char const *file,
                    struct timeval32 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 return libc_utimensat64(dfd,file,times,flags);
}
INTERN int LIBCCALL libc_utimes(char const *file, struct timeval32 const tvp[2]) { return libc_impl_futimesat(AT_FDCWD,file,tvp,0);  }
CRT_DOS_EXT int LIBCCALL libc_dos_utimes(char const *file, struct timeval32 const tvp[2]) { return libc_impl_futimesat(AT_FDCWD,file,tvp,AT_DOSPATH);  }
INTERN int LIBCCALL libc_utimes64(char const *file, struct timeval64 const tvp[2]) { return libc_impl_futimesat64(AT_FDCWD,file,tvp,0);  }
CRT_DOS_EXT int LIBCCALL libc_dos_utimes64(char const *file, struct timeval64 const tvp[2]) { return libc_impl_futimesat64(AT_FDCWD,file,tvp,AT_DOSPATH);  }
INTERN int LIBCCALL libc_lutimes(char const *file, struct timeval32 const tvp[2]) { return libc_impl_futimesat(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW);  }
CRT_DOS_EXT int LIBCCALL libc_dos_lutimes(char const *file, struct timeval32 const tvp[2]) { return libc_impl_futimesat(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW|AT_DOSPATH);  }
INTERN int LIBCCALL libc_lutimes64(char const *file, struct timeval64 const tvp[2]) { return libc_impl_futimesat64(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW);  }
CRT_DOS_EXT int LIBCCALL libc_dos_lutimes64(char const *file, struct timeval64 const tvp[2]) { return libc_impl_futimesat64(AT_FDCWD,file,tvp,AT_SYMLINK_NOFOLLOW|AT_DOSPATH);  }
INTERN int LIBCCALL libc_futimesat(fd_t dfd, char const *file, struct timeval32 const tvp[2]) { return libc_impl_futimesat(dfd,file,tvp,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_futimesat(fd_t dfd, char const *file, struct timeval32 const tvp[2]) { return libc_impl_futimesat(dfd,file,tvp,AT_DOSPATH); }
INTERN int LIBCCALL libc_futimesat64(fd_t dfd, char const *file, struct timeval64 const tvp[2]) { return libc_impl_futimesat64(dfd,file,tvp,0); }
CRT_DOS_EXT int LIBCCALL libc_dos_futimesat64(fd_t dfd, char const *file, struct timeval64 const tvp[2]) { return libc_impl_futimesat64(dfd,file,tvp,AT_DOSPATH); }



/* FILESYSTEM TIME MODIFICATION. */
EXPORT(__KSYM(utimensat),          libc_utimensat);
EXPORT(__DSYM(utimensat),          libc_dos_utimensat);
EXPORT(__KSYM(utimensat64),        libc_utimensat64);
EXPORT(__DSYM(utimensat64),        libc_dos_utimensat64);
EXPORT(futimens,                   libc_futimens);
EXPORT(futimens64,                 libc_futimens64);
EXPORT(futime,                     libc_futime);
EXPORT(futime64,                   libc_futime64);
EXPORT(futimens,                   libc_futimens);
EXPORT(futimens64,                 libc_futimens64);
EXPORT(futimes,                    libc_futimes);
EXPORT(futimes64,                  libc_futimes64);
EXPORT(__KSYM(utimensat),          libc_utimensat);
EXPORT(__DSYM(utimensat),          libc_dos_utimensat);
EXPORT(__KSYM(utimensat64),        libc_utimensat64);
EXPORT(__DSYM(utimensat64),        libc_dos_utimensat64);
EXPORT(__KSYM(futimeat),           libc_futimeat);
EXPORT(__DSYM(futimeat),           libc_dos_futimeat);
EXPORT(__KSYM(futimeat64),         libc_futimeat64);
EXPORT(__DSYM(futimeat64),         libc_dos_futimeat64);
EXPORT(__KSYM(utime),              libc_utime);
EXPORT(__DSYM(utime),              libc_dos_utime);
EXPORT(__KSYM(utime64),            libc_utime64);
EXPORT(__DSYM(utime64),            libc_dos_utime64);
EXPORT(__KSYM(utimes),             libc_utimes);
EXPORT(__DSYM(utimes),             libc_dos_utimes);
EXPORT(__KSYM(utimes64),           libc_utimes64);
EXPORT(__DSYM(utimes64),           libc_dos_utimes64);
EXPORT(__KSYM(lutimes),            libc_lutimes);
EXPORT(__DSYM(lutimes),            libc_dos_lutimes);
EXPORT(__KSYM(lutimes64),          libc_lutimes64);
EXPORT(__DSYM(lutimes64),          libc_dos_lutimes64);
EXPORT(__KSYM(futimesat),          libc_futimesat);
EXPORT(__DSYM(futimesat),          libc_dos_futimesat);
EXPORT(__KSYM(futimesat64),        libc_futimesat64);
EXPORT(__DSYM(futimesat64),        libc_dos_futimesat64);

/* DOS aliases */
EXPORT(_futime,                    libc_futime); /* This is not an error. - DOS defines this name, too. */
EXPORT(_futime32,                  libc_futime);
EXPORT(_futime64,                  libc_futime64);
EXPORT(__KSYM(_utime),             libc_utime); /* This is not an error. - DOS defines this name, too. */
EXPORT(__DSYM(_utime),             libc_dos_utime); /* This is not an error. - DOS defines this name, too. */
EXPORT(__KSYM(_utime32),           libc_utime);
EXPORT(__DSYM(_utime32),           libc_dos_utime);
EXPORT(__KSYM(_utime64),           libc_utime64);
EXPORT(__DSYM(_utime64),           libc_dos_utime64);





EXPORT(Xfutimens,libc_Xfutimens);
CRT_EXCEPT void LIBCCALL
libc_Xfutimens(fd_t fd, struct timespec32 const times[2]) {
 libc_Xutimensat(fd,libc_empty_string,times,AT_EMPTY_PATH);
}
EXPORT(Xfutimens64,libc_Xfutimens64);
CRT_EXCEPT void LIBCCALL
libc_Xfutimens64(fd_t fd, struct timespec64 const times[2]) {
 Xsys_utimensat(fd,libc_empty_string,times,AT_EMPTY_PATH);
}
EXPORT(Xutimensat,libc_Xutimensat);
CRT_EXCEPT void LIBCCALL
libc_Xutimensat(fd_t dfd, char const *path, struct timespec32 const times[2], int flags) {
 if (!times) {
  Xsys_utimensat(dfd,path,NULL,flags);
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
  Xsys_utimensat(dfd,path,t64,flags);
 }
}
EXPORT(Xfutimeat,libc_Xfutimeat);
CRT_EXCEPT void LIBCCALL
libc_Xfutimeat(fd_t dfd, char const *file, struct utimbuf32 const *file_times, int flags) {
 struct timespec64 times[2];
 if (!file_times) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  times[0].tv_sec  = file_times->actime;
  times[1].tv_sec  = file_times->modtime;
  times[0].tv_nsec = 0;
  times[1].tv_nsec = 0;
 }
 Xsys_utimensat(dfd,file,times,flags);
}
EXPORT(Xfutimeat64,libc_Xfutimeat64);
CRT_EXCEPT void LIBCCALL
libc_Xfutimeat64(fd_t dfd, char const *file, struct utimbuf64 const *file_times, int flags) {
 struct timespec64 times[2];
 if (!file_times) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  times[0].tv_sec  = file_times->actime;
  times[1].tv_sec  = file_times->modtime;
  times[0].tv_nsec = 0;
  times[1].tv_nsec = 0;
 }
 Xsys_utimensat(dfd,file,times,flags);
}
EXPORT(Xfutime,libc_Xfutime);
CRT_EXCEPT void LIBCCALL
libc_Xfutime(fd_t fd, struct utimbuf32 const *file_times) {
 libc_Xfutimeat(fd,libc_empty_string,file_times,0); 
}
EXPORT(Xfutime64,libc_Xfutime64);
CRT_EXCEPT void LIBCCALL
libc_Xfutime64(fd_t fd, struct utimbuf64 const *file_times) {
 libc_Xfutimeat64(fd,libc_empty_string,file_times,0); 
}
EXPORT(Xutime,libc_Xutime);
CRT_EXCEPT void LIBCCALL
libc_Xutime(char const *file, struct utimbuf32 const *file_times) {
 libc_Xfutimeat(AT_FDCWD,file,file_times,0); 
}
EXPORT(Xutime64,libc_Xutime64);
CRT_EXCEPT void LIBCCALL
libc_Xutime64(char const *file, struct utimbuf64 const *file_times) {
 libc_Xfutimeat64(AT_FDCWD,file,file_times,0); 
}


CRT_EXCEPT void LIBCCALL
libc_impl_Xfutimesat64(fd_t fd, char const *file, struct timeval64 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 Xsys_utimensat(fd,file,times,flags);
}
CRT_EXCEPT void LIBCCALL
libc_impl_Xfutimesat(fd_t fd, char const *file, struct timeval32 const tvp[2], int flags) {
 struct timespec64 times[2];
 if (!tvp) {
  times[0].tv_nsec = UTIME_NOW;
  times[1].tv_nsec = UTIME_NOW;
 } else {
  TIMEVAL_TO_TIMESPEC(&tvp[0],&times[0]);
  TIMEVAL_TO_TIMESPEC(&tvp[1],&times[1]);
 }
 Xsys_utimensat(fd,file,times,flags);
}

EXPORT(Xfutimesat,libc_Xfutimesat);
CRT_EXCEPT void LIBCCALL
libc_Xfutimesat(fd_t fd, char const *file, struct timeval32 const tvp[2]) {
 libc_impl_Xfutimesat(fd,file,tvp,0);
}

EXPORT(Xfutimesat64,libc_Xfutimesat64);
CRT_EXCEPT void LIBCCALL
libc_Xfutimesat64(fd_t fd, char const *file, struct timeval64 const tvp[2]) {
 libc_impl_Xfutimesat64(fd,file,tvp,0);
}

EXPORT(Xfutimes,libc_Xfutimes);
CRT_EXCEPT void LIBCCALL libc_Xfutimes(fd_t fd, struct timeval32 const tvp[2]) {
 libc_impl_Xfutimesat(fd,libc_empty_string,tvp,0);
}

EXPORT(Xfutimes64,libc_Xfutimes64);
CRT_EXCEPT void LIBCCALL
libc_Xfutimes64(fd_t fd, struct timeval64 const tvp[2]) {
 libc_impl_Xfutimesat64(fd,libc_empty_string,tvp,0);
}

EXPORT(Xutimes,libc_Xutimes);
CRT_EXCEPT void LIBCCALL
libc_Xutimes(char const *file, struct timeval32 const tvp[2]) {
 libc_impl_Xfutimesat(AT_FDCWD,file,tvp,0);
}

EXPORT(Xutimes64,libc_Xutimes64);
CRT_EXCEPT void LIBCCALL
libc_Xutimes64(char const *file, struct timeval64 const tvp[2]) {
 libc_impl_Xfutimesat64(AT_FDCWD,file,tvp,0);
}

EXPORT(Xlutimes,libc_Xlutimes);
CRT_EXCEPT void LIBCCALL
libc_Xlutimes(char const *file, struct timeval32 const tvp[2]) {
 libc_impl_Xfutimesat(AT_FDCWD,file,tvp,0);
}

EXPORT(Xlutimes64,libc_Xlutimes64);
CRT_EXCEPT void LIBCCALL
libc_Xlutimes64(char const *file, struct timeval64 const tvp[2]) {
 libc_impl_Xfutimesat64(AT_FDCWD,file,tvp,0);
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
 char *EXCEPT_VAR str = libc_Xloadutf16(buf,path);
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
 char *EXCEPT_VAR str = libc_Xloadutf32(buf,path);
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
 char *EXCEPT_VAR str = libc_Xloadutf16(buf,file);
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
 char *EXCEPT_VAR str = libc_Xloadutf32(buf,file);
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

#endif /* !GUARD_LIBS_LIBC_CHTIME_C */
