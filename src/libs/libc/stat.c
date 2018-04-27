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
#ifndef GUARD_LIBS_LIBC_STAT_C
#define GUARD_LIBS_LIBC_STAT_C 1
#define _GNU_SOURCE 1
#define __EXPOSE_STAT_STRUCTURES 1

#include "libc.h"
#include "stat.h"
#include "system.h"
#include "errno.h"
#include "widechar.h"
#include <errno.h>
#include <fcntl.h>
#include <bits/stat.h>
#include <sys/stat.h>

DECL_BEGIN

INTERN int LIBCCALL libc_kfstat64(fd_t fd, struct __kos_stat64 *buf) { return FORWARD_SYSTEM_ERROR(sys_fstat64(fd,buf)); }
INTERN int LIBCCALL libc_kstat64(char const *file, struct __kos_stat64 *buf) { return libc_kfstatat64(AT_FDCWD,file,buf,0); }
INTERN int LIBCCALL libc_kfstatat64(fd_t dfd, char const *file, struct __kos_stat64 *buf, int flags) { return FORWARD_SYSTEM_ERROR(sys_fstatat64(dfd,file,buf,flags)); }
INTERN int LIBCCALL libc_klstat64(char const *file, struct __kos_stat64 *buf) { return libc_kfstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW); }
CRT_DOS_EXT int LIBCCALL libc_dos_kstat64(char const *file, struct __kos_stat64 *buf) { return libc_kfstatat64(AT_FDCWD,file,buf,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_kfstatat64(fd_t dfd, char const *file, struct __kos_stat64 *buf, int flags) { return libc_kfstatat64(dfd,file,buf,flags|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_klstat64(char const *file, struct __kos_stat64 *buf) { return libc_kfstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
DEFINE_INTERN_ALIAS(libc_kfstat,libc_kfstat64);
DEFINE_INTERN_ALIAS(libc_kstat,libc_kstat64);
DEFINE_INTERN_ALIAS(libc_dos_kstat,libc_dos_kstat64);
DEFINE_INTERN_ALIAS(libc_kfstatat,libc_kfstatat64);
DEFINE_INTERN_ALIAS(libc_dos_kfstatat,libc_dos_kfstatat64);
DEFINE_INTERN_ALIAS(libc_klstat,libc_klstat64);
DEFINE_INTERN_ALIAS(libc_dos_klstat,libc_dos_klstat64);

CRT_GLC void LIBCCALL
stat2glc(struct __kos_stat64 const *__restrict in,
         struct __glc_stat *__restrict out) {
 out->st_dev     = in->st_dev;
 out->st_ino32   = in->st_ino32;
 out->st_nlink   = in->st_nlink;
 out->st_mode    = in->st_mode;
 out->st_uid     = in->st_uid;
 out->st_gid     = in->st_gid;
 out->st_rdev    = in->st_rdev;
 out->st_size32  = in->st_size32;
 out->st_blksize = in->st_blksize;
 out->st_blocks  = in->st_blocks;
 out->st_atim    = in->st_atim32;
 out->st_mtim    = in->st_mtim32;
 out->st_ctim    = in->st_ctim32;
}
CRT_GLC void LIBCCALL
stat2glc64(struct __kos_stat64 const *__restrict in,
           struct __glc_stat64 *__restrict out) {
 out->st_dev     = in->st_dev;
 out->st_ino32   = in->st_ino32;
 out->st_ino64   = in->st_ino64;
 out->st_nlink   = in->st_nlink;
 out->st_mode    = in->st_mode;
 out->st_uid     = in->st_uid;
 out->st_gid     = in->st_gid;
 out->st_rdev    = in->st_rdev;
 out->st_size    = in->st_size64;
 out->st_blksize = in->st_blksize;
 out->st_blocks  = in->st_blocks;
 out->st_atim    = in->st_atim32;
 out->st_mtim    = in->st_mtim32;
 out->st_ctim    = in->st_ctim32;
}

CRT_GLC int LIBCCALL libc_fstat(fd_t fd, struct __glc_stat *buf) {
 struct __kos_stat64 temp; int result;
 result = libc_kfstat64(fd,&temp);
 if (!result) stat2glc(&temp,buf);
 return result;
}
CRT_GLC int LIBCCALL libc_fstat64(fd_t fd, struct __glc_stat64 *buf) {
 struct __kos_stat64 temp; int result;
 result = libc_kfstat64(fd,&temp);
 if (!result) stat2glc64(&temp,buf);
 return result;
}
CRT_GLC int LIBCCALL libc_fstatat(fd_t dfd, char const *file, struct __glc_stat *buf, int flags) {
 struct __kos_stat64 temp; int result;
 result = libc_kfstatat64(dfd,file,&temp,flags);
 if (!result) stat2glc(&temp,buf);
 return result;
}
CRT_GLC int LIBCCALL libc_fstatat64(fd_t dfd, char const *file, struct __glc_stat64 *buf, int flags) {
 struct __kos_stat64 temp; int result;
 result = libc_kfstatat64(dfd,file,&temp,flags);
 if (!result) stat2glc64(&temp,buf);
 return result;
}

CRT_GLC int LIBCCALL libc_stat(char const *file, struct __glc_stat *buf) { return libc_fstatat(AT_FDCWD,file,buf,0); }
CRT_GLC int LIBCCALL libc_stat64(char const *file, struct __glc_stat64 *buf) { return libc_fstatat64(AT_FDCWD,file,buf,0); }
CRT_GLC int LIBCCALL libc_lstat(char const *file, struct __glc_stat *buf) { return libc_fstatat(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW); }
CRT_GLC int LIBCCALL libc_lstat64(char const *file, struct __glc_stat64 *buf) { return libc_fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW); }
CRT_DOS_EXT int LIBCCALL libc_dos_stat(char const *file, struct __glc_stat *buf) { return libc_fstatat(AT_FDCWD,file,buf,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_stat64(char const *file, struct __glc_stat64 *buf) { return libc_fstatat64(AT_FDCWD,file,buf,AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_lstat(char const *file, struct __glc_stat *buf) { return libc_fstatat(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_lstat64(char const *file, struct __glc_stat64 *buf) { return libc_fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fstatat(fd_t dfd, char const *file, struct __glc_stat *buf, int flags) { return libc_fstatat(dfd,file,buf,flags|AT_DOSPATH); }
CRT_DOS_EXT int LIBCCALL libc_dos_fstatat64(fd_t dfd, char const *file, struct __glc_stat64 *buf, int flags) { return libc_fstatat64(dfd,file,buf,flags|AT_DOSPATH); }

#define VERCHK { if (ver != 0) { libc_seterrno(EINVAL); return -1; } }
CRT_GLC int LIBCCALL libc_version_fxstat(int ver, fd_t fd, struct __glc_stat *statbuf) { VERCHK return libc_fstat(fd,statbuf); }
CRT_GLC int LIBCCALL libc_version_fxstat64(int ver, fd_t fd, struct __glc_stat64 *statbuf) { VERCHK return libc_fstat64(fd,statbuf); }
CRT_GLC int LIBCCALL libc_version_xstat(int ver, char const *filename, struct __glc_stat *statbuf) { VERCHK return libc_stat(filename,statbuf); }
CRT_GLC int LIBCCALL libc_version_xstat64(int ver, char const *filename, struct __glc_stat64 *statbuf) { VERCHK return libc_stat64(filename,statbuf); }
CRT_GLC int LIBCCALL libc_version_lxstat(int ver, char const *filename, struct __glc_stat *statbuf) { VERCHK return libc_lstat(filename,statbuf); }
CRT_GLC int LIBCCALL libc_version_lxstat64(int ver, char const *filename, struct __glc_stat64 *statbuf) { VERCHK return libc_lstat64(filename,statbuf); }
CRT_GLC int LIBCCALL libc_version_fxstatat(int ver, fd_t fd, char const *filename, struct __glc_stat *statbuf, int flags) { VERCHK return libc_fstatat(fd,filename,statbuf,flags); }
CRT_GLC int LIBCCALL libc_version_fxstatat64(int ver, fd_t fd, char const *filename, struct __glc_stat64 *statbuf, int flags) { VERCHK return libc_fstatat64(fd,filename,statbuf,flags); }
CRT_DOS_EXT int LIBCCALL libc_dos_version_xstat(int ver, char const *filename, struct __glc_stat *statbuf) { VERCHK return libc_dos_stat(filename,statbuf); }
CRT_DOS_EXT int LIBCCALL libc_dos_version_xstat64(int ver, char const *filename, struct __glc_stat64 *statbuf) { VERCHK return libc_dos_stat64(filename,statbuf); }
CRT_DOS_EXT int LIBCCALL libc_dos_version_lxstat(int ver, char const *filename, struct __glc_stat *statbuf) { VERCHK return libc_dos_lstat(filename,statbuf); }
CRT_DOS_EXT int LIBCCALL libc_dos_version_lxstat64(int ver, char const *filename, struct __glc_stat64 *statbuf) { VERCHK return libc_dos_lstat64(filename,statbuf); }
CRT_DOS_EXT int LIBCCALL libc_dos_version_fxstatat(int ver, fd_t fd, char const *filename, struct __glc_stat *statbuf, int flags) { VERCHK return libc_dos_fstatat(fd,filename,statbuf,flags); }
CRT_DOS_EXT int LIBCCALL libc_dos_version_fxstatat64(int ver, fd_t fd, char const *filename, struct __glc_stat64 *statbuf, int flags) { VERCHK return libc_dos_fstatat64(fd,filename,statbuf,flags); }
#undef VERCHK

/* DOS-FS mode + DOS-binary-compatible stat functions.
 * NOTE: These are only used when user-apps are configures as `__PE__' + `__USE_DOS'.
 *       Otherwise, KOS's kernel stat buffer data layout is used to maximize performance.  */
CRT_DOS void LIBCCALL
stat2comdos(struct __kos_stat64 const *__restrict src,
            struct __dos_stat32 *__restrict buf) {
 buf->st_dev   = (__dos_dev_t)src->st_dev;
 buf->st_ino   = (__dos_ino_t)src->st_ino;
 buf->st_mode  = (__uint16_t)src->st_mode;
 buf->st_nlink = (__int16_t)src->st_nlink;
 buf->st_uid   = (__int16_t)src->st_uid;
 buf->st_gid   = (__int16_t)src->st_gid;
 buf->st_rdev  = (__dos_dev_t)src->st_rdev;
}
CRT_DOS void LIBCCALL
stat2dos32(struct __kos_stat64 const *__restrict src,
           struct __dos_stat32 *__restrict buf) {
 stat2comdos(src,(struct __dos_stat32 *)buf);
 buf->st_size  = src->st_size32;
 buf->st_atime = src->st_atime32;
 buf->st_mtime = src->st_mtime32;
 buf->st_ctime = src->st_ctime32;
}
CRT_DOS void LIBCCALL
stat2dos64(struct __kos_stat64 const *__restrict src,
           struct __dos_stat64 *__restrict buf) {
 stat2comdos(src,(struct __dos_stat32 *)buf);
 buf->st_size  = src->st_size64;
 buf->st_atime = src->st_atime64;
 buf->st_mtime = src->st_mtime64;
 buf->st_ctime = src->st_ctime64;
}
CRT_DOS void LIBCCALL
stat2dos32i64(struct __kos_stat64 const *__restrict src,
              struct __dos_stat32i64 *__restrict buf) {
 stat2comdos(src,(struct __dos_stat32 *)buf);
 buf->st_size  = src->st_size64;
 buf->st_atime = src->st_atime32;
 buf->st_mtime = src->st_mtime32;
 buf->st_ctime = src->st_ctime32;
}

CRT_DOS int LIBCCALL libd_fstat32(fd_t fd, struct __dos_stat32 *buf) { struct __kos_stat64 temp; int result = libc_kfstat64(fd,&temp); if (!result) stat2dos32(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_fstat32i64(fd_t fd, struct __dos_stat32i64 *buf) { struct __kos_stat64 temp; int result = libc_kfstat64(fd,&temp); if (!result) stat2dos32i64(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_fstat64(fd_t fd, struct __dos_stat64 *buf) { struct __kos_stat64 temp; int result = libc_kfstat64(fd,&temp); if (!result) stat2dos64(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_stat32(char const *file, struct __dos_stat32 *buf) { struct __kos_stat64 temp; int result = libc_kstat64(file,&temp); if (!result) stat2dos32(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_stat32i64(char const *file, struct __dos_stat32i64 *buf) { struct __kos_stat64 temp; int result = libc_kstat64(file,&temp); if (!result) stat2dos32i64(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_stat64(char const *file, struct __dos_stat64 *buf) { struct __kos_stat64 temp; int result = libc_kstat64(file,&temp); if (!result) stat2dos64(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_dos_stat32(char const *file, struct __dos_stat32 *buf) { struct __kos_stat64 temp; int result = libc_dos_kstat64(file,&temp); if (!result) stat2dos32(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_dos_stat32i64(char const *file, struct __dos_stat32i64 *buf) { struct __kos_stat64 temp; int result = libc_dos_kstat64(file,&temp); if (!result) stat2dos32i64(&temp,buf); return result; }
CRT_DOS int LIBCCALL libd_dos_stat64(char const *file, struct __dos_stat64 *buf) { struct __kos_stat64 temp; int result = libc_dos_kstat64(file,&temp); if (!result) stat2dos64(&temp,buf); return result; }


/* Export symbols (STAT). */
EXPORT(kfstat,                     libc_kfstat);
EXPORT(kfstat64,                   libc_kfstat64);
EXPORT(__KSYM(kstat),              libc_kstat);
EXPORT(__DSYM(kstat),              libc_dos_kstat);
EXPORT(__KSYM(kstat64),            libc_kstat64);
EXPORT(__DSYM(kstat64),            libc_dos_kstat64);
EXPORT(__KSYM(kfstatat),           libc_kfstatat);
EXPORT(__DSYM(kfstatat),           libc_dos_kfstatat);
EXPORT(__KSYM(kfstatat64),         libc_kfstatat64);
EXPORT(__DSYM(kfstatat64),         libc_dos_kfstatat64);
EXPORT(__KSYM(klstat),             libc_klstat);
EXPORT(__DSYM(klstat),             libc_dos_klstat);
EXPORT(__KSYM(klstat64),           libc_klstat64);
EXPORT(__DSYM(klstat64),           libc_dos_klstat64);

EXPORT(fstat,                      libc_fstat);
EXPORT(fstat64,                    libc_fstat64);
EXPORT(__KSYM(stat),               libc_stat);
EXPORT(__DSYM(stat),               libc_dos_stat);
EXPORT(__KSYM(stat64),             libc_stat64);
EXPORT(__DSYM(stat64),             libc_dos_stat64);
EXPORT(__KSYM(fstatat),            libc_fstatat);
EXPORT(__DSYM(fstatat),            libc_dos_fstatat);
EXPORT(__KSYM(fstatat64),          libc_fstatat64);
EXPORT(__DSYM(fstatat64),          libc_dos_fstatat64);
EXPORT(__KSYM(lstat),              libc_lstat);
EXPORT(__DSYM(lstat),              libc_dos_lstat);
EXPORT(__KSYM(lstat64),            libc_lstat64);
EXPORT(__DSYM(lstat64),            libc_dos_lstat64);

EXPORT(__fxstat,                   libc_version_fxstat);
EXPORT(__fxstat64,                 libc_version_fxstat64);
EXPORT(__KSYM(__xstat),            libc_version_xstat);
EXPORT(__DSYM(__xstat),            libc_dos_version_xstat);
EXPORT(__KSYM(__xstat64),          libc_version_xstat64);
EXPORT(__DSYM(__xstat64),          libc_dos_version_xstat64);
EXPORT(__KSYM(__lxstat),           libc_version_lxstat);
EXPORT(__DSYM(__lxstat),           libc_dos_version_lxstat);
EXPORT(__KSYM(__lxstat64),         libc_version_lxstat64);
EXPORT(__DSYM(__lxstat64),         libc_dos_version_lxstat64);
EXPORT(__KSYM(__fxstatat),         libc_version_fxstatat);
EXPORT(__DSYM(__fxstatat),         libc_dos_version_fxstatat);
EXPORT(__KSYM(__fxstatat64),       libc_version_fxstatat64);
EXPORT(__DSYM(__fxstatat64),       libc_dos_version_fxstatat64);


EXPORT(_fstat,                     libd_fstat32);
EXPORT(_fstat32,                   libd_fstat32);
EXPORT(_fstati64,                  libd_fstat32i64);
EXPORT(_fstat32i64,                libd_fstat32i64);
EXPORT(_fstat64,                   libd_fstat64);
EXPORT(_fstat64i32,                libd_fstat64);
EXPORT(__KSYM(_stat),              libd_stat32);
EXPORT(__DSYM(_stat),              libd_dos_stat32);
EXPORT(__KSYM(_stat32),            libd_stat32);
EXPORT(__DSYM(_stat32),            libd_dos_stat32);
EXPORT(__KSYM(_stati64),           libd_stat32i64);
EXPORT(__DSYM(_stati64),           libd_dos_stat32i64);
EXPORT(__KSYM(_stat32i64),         libd_stat32i64);
EXPORT(__DSYM(_stat32i64),         libd_dos_stat32i64);
EXPORT(__KSYM(_stat64),            libd_stat64);
EXPORT(__DSYM(_stat64),            libd_dos_stat64);
EXPORT(__KSYM(_stat64i32),         libd_stat64);
EXPORT(__DSYM(_stat64i32),         libd_dos_stat64);


EXPORT(Xkstat64,libc_Xkstat64);
CRT_EXCEPT void LIBCCALL
libc_Xkstat64(char const *file, struct __kos_stat64 *buf) {
 Xsys_fstatat64(AT_FDCWD,file,buf,0);
}
DEFINE_INTERN_ALIAS(libc_Xkstat,libc_Xkstat64);
EXPORT(Xkstat,libc_Xkstat);

EXPORT(Xklstat64,libc_Xklstat64);
CRT_EXCEPT void LIBCCALL
libc_Xklstat64(char const *file, struct __kos_stat64 *buf) {
 Xsys_fstatat64(AT_FDCWD,file,buf,AT_SYMLINK_NOFOLLOW);
}
DEFINE_INTERN_ALIAS(libc_Xklstat,libc_Xklstat64);
EXPORT(Xklstat,libc_Xklstat);


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
 char *EXCEPT_VAR str = libc_Xloadutf16(filebuf,file);
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
 char *EXCEPT_VAR str = libc_Xloadutf32(filebuf,file);
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


DECL_END

#endif /* !GUARD_LIBS_LIBC_STAT_C */
