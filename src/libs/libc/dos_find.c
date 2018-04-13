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
#ifndef GUARD_LIBS_LIBC_DOS_FIND_C
#define GUARD_LIBS_LIBC_DOS_FIND_C 1

#include "dos_find.h"
#include "dirent.h"
#include "malloc.h"
#include "system.h"
#include "errno.h"
#include "widechar.h"
#include "ctype.h"
#include "unistd.h"
#include "unicode.h"
#include <unicode.h>
#include <errno.h>
#include <sys/stat.h>
#include <hybrid/minmax.h>

#ifdef __CC__
DECL_BEGIN

CRT_DOS struct find *LIBCCALL
libd_findopen(char const *__restrict query, oflag_t oflags) {
 PRIVATE char const dirpath_cwd[] = ".";
 struct find *result; fd_t fd;
 char const *pattern_start;
 char *dirpath; size_t dirlen;
 pattern_start = query;
 while (*pattern_start != '/' ||
        *pattern_start != '\\' ||
        libc_isspace(*pattern_start)) {
  if (!*pattern_start) {
   pattern_start = query;
   dirpath = (char *)dirpath_cwd;
   goto got_dirpath;
  }
  ++pattern_start;
 }
 dirlen  = (size_t)(pattern_start-query);
 dirpath = (char *)libc_memdup(query,dirlen);
 dirpath[dirlen-1] = '\0';
got_dirpath:
 result = (struct find *)libc_malloc(sizeof(struct find));
 if (!result) goto err;
 /* Duplicate the pattern string. */
 result->f_pattern = libc_strdup(pattern_start);
 if (!result->f_pattern) {
err_r:
  libc_free(result);
err:
  result = FIND_INVALID;
  goto done;
 }
 /* Not actually open the directory. */
 fd = libc_open(dirpath,oflags);
 if unlikely(fd < 0) {
  libc_free(result->f_pattern);
  goto err_r;
 }
 /* Initialize the directory stream of the find handle. */
 dirstream_init(&result->f_dir,fd);
done:
 if (dirpath != dirpath_cwd)
     libc_free(dirpath);
 return result;
}



CRT_DOS struct find *LIBCCALL
libd_w16findopen(char16_t const *__restrict query, oflag_t oflags) {
 char buf[UTF_STACK_BUFFER_SIZE]; struct find *result;
 char *str = libc_loadutf16(buf,query);
 if (!str) return FIND_INVALID;
 result = libd_findopen(str,oflags);
 libc_freeutf(buf,str);
 return result;
}
CRT_DOS struct find *LIBCCALL
libd_w32findopen(char32_t const *__restrict query, oflag_t oflags) {
 char buf[UTF_STACK_BUFFER_SIZE]; struct find *result;
 char *str = libc_loadutf32(buf,query);
 if (!str) return FIND_INVALID;
 result = libd_findopen(str,oflags);
 libc_freeutf(buf,str);
 return result;
}
CRT_DOS struct dirent *LIBCCALL
libd_findread(struct find *findfd, struct __kos_stat64 *__restrict st) {
 struct dirent *result;
 LIBC_TRY {
  libc_seterrno(ENOENT);
  while ((result = libc_readdir(&findfd->f_dir)) != NULL) {
   /* Check if the pattern matches. */
   if (libc_wildstrcasecmp(findfd->f_pattern,result->d_name) != 0)
       continue;
   if (libc_kfstatat64(findfd->f_dir.ds_fd,result->d_name,st,0))
       continue;
   return result;
  }
 } LIBC_EXCEPT(libc_except_errno()) {
  if (findfd == FIND_INVALID)
      libc_seterrno(EINVAL);
 }
 return NULL;
}


EXPORT(_findclose,libd_findclose);
CRT_DOS int LIBCCALL
libd_findclose(struct find *findfd) {
 if (findfd == FIND_INVALID) {
  libc_seterrno(EINVAL);
  return -1;
 }
 LIBC_TRY {
  libc_free(findfd->f_pattern);
  dirstream_fini(&findfd->f_dir);
  libc_free(findfd);
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return 0;
}

EXPORT(__KSYM(findfirst32),libd_findfirst32);
CRT_DOS struct find *LIBCCALL
libd_findfirst32(char const *__restrict file,
                 struct finddata32_t *__restrict finddata) {
 struct find *result;
 result = libd_findopen(file,O_RDONLY|O_DIRECTORY);
 if (libd_findnext32(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}
EXPORT(__KSYM(findfirst32i64),libd_findfirst64);
CRT_DOS struct find *LIBCCALL
libd_findfirst64(char const *__restrict file,
                 struct finddata64_t *__restrict finddata) {
 struct find *result;
 result = libd_findopen(file,O_RDONLY|O_DIRECTORY);
 if (libd_findnext64(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}
EXPORT(__KSYM(findfirst32i64),libd_findfirst32i64);
CRT_DOS struct find *LIBCCALL
libd_findfirst32i64(char const *__restrict file,
                    struct finddata32i64_t *__restrict finddata) {
 struct find *result;
 result = libd_findopen(file,O_RDONLY|O_DIRECTORY);
 if (libd_findnext32i64(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}

EXPORT(__DSYM(_findfirst32),libd_dos_findfirst32);
CRT_DOS struct find *LIBCCALL
libd_dos_findfirst32(char const *__restrict file,
                     struct finddata32_t *__restrict finddata) {
 struct find *result;
 result = libd_findopen(file,O_RDONLY|O_DIRECTORY|O_DOSPATH);
 if (libd_findnext32(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}
EXPORT(__DSYM(_findfirst64),libd_dos_findfirst64);
CRT_DOS struct find *LIBCCALL
libd_dos_findfirst64(char const *__restrict file,
                     struct finddata64_t *__restrict finddata) {
 struct find *result;
 result = libd_findopen(file,O_RDONLY|O_DIRECTORY|O_DOSPATH);
 if (libd_findnext64(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}
EXPORT(__DSYM(_findfirst32i64),libd_dos_findfirst32i64);
CRT_DOS struct find *LIBCCALL
libd_dos_findfirst32i64(char const *__restrict file,
                        struct finddata32i64_t *__restrict finddata) {
 struct find *result;
 result = libd_findopen(file,O_RDONLY|O_DIRECTORY|O_DOSPATH);
 if (libd_findnext32i64(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}


EXPORT(__KSYMw16(wfindfirst32),libd_w16findfirst32);
CRT_DOS struct find *LIBCCALL
libd_w16findfirst32(char16_t const *__restrict file,
                    struct w16finddata32_t *__restrict finddata) {
 struct find *result;
 result = libd_w16findopen(file,O_RDONLY|O_DIRECTORY);
 if (libd_w16findnext32(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}
EXPORT(__KSYMw16(wfindfirst64),libd_w16findfirst64);
CRT_DOS struct find *LIBCCALL
libd_w16findfirst64(char16_t const *__restrict file,
                    struct w16finddata64_t *__restrict finddata) {
 struct find *result;
 result = libd_w16findopen(file,O_RDONLY|O_DIRECTORY);
 if (libd_w16findnext64(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}
EXPORT(__KSYMw16(wfindfirst32i64),libd_w16findfirst32i64);
CRT_DOS struct find *LIBCCALL
libd_w16findfirst32i64(char16_t const *__restrict file,
                    struct w16finddata32i64_t *__restrict finddata) {
 struct find *result;
 result = libd_w16findopen(file,O_RDONLY|O_DIRECTORY);
 if (libd_w16findnext32i64(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}

EXPORT(__DSYMw16(_wfindfirst32),libd_dos_findfirst32);
CRT_DOS struct find *LIBCCALL
libd_dos_w16findfirst32(char16_t const *__restrict file,
                        struct w16finddata32_t *__restrict finddata) {
 struct find *result;
 result = libd_w16findopen(file,O_RDONLY|O_DIRECTORY|O_DOSPATH);
 if (libd_w16findnext32(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}
EXPORT(__DSYMw16(_wfindfirst64),libd_dos_findfirst64);
CRT_DOS struct find *LIBCCALL
libd_dos_w16findfirst64(char16_t const *__restrict file,
                        struct w16finddata64_t *__restrict finddata) {
 struct find *result;
 result = libd_w16findopen(file,O_RDONLY|O_DIRECTORY|O_DOSPATH);
 if (libd_w16findnext64(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}
EXPORT(__DSYMw16(_wfindfirst32i64),libd_dos_findfirst32i64);
CRT_DOS struct find *LIBCCALL
libd_dos_w16findfirst32i64(char16_t const *__restrict file,
                           struct w16finddata32i64_t *__restrict finddata) {
 struct find *result;
 result = libd_w16findopen(file,O_RDONLY|O_DIRECTORY|O_DOSPATH);
 if (libd_w16findnext32i64(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}

EXPORT(__KSYMw32(wfindfirst32),libd_w32findfirst32);
CRT_DOS struct find *LIBCCALL
libd_w32findfirst32(char32_t const *__restrict file,
                    struct w32finddata32_t *__restrict finddata) {
 struct find *result;
 result = libd_w32findopen(file,O_RDONLY|O_DIRECTORY);
 if (libd_w32findnext32(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}
EXPORT(__KSYMw32(wfindfirst32i64),libd_w32findfirst64);
CRT_DOS struct find *LIBCCALL
libd_w32findfirst64(char32_t const *__restrict file,
                    struct w32finddata64_t *__restrict finddata) {
 struct find *result;
 result = libd_w32findopen(file,O_RDONLY|O_DIRECTORY);
 if (libd_w32findnext64(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}
EXPORT(__KSYMw32(wfindfirst32i64),libd_w32findfirst32i64);
CRT_DOS struct find *LIBCCALL
libd_w32findfirst32i64(char32_t const *__restrict file,
                    struct w32finddata32i64_t *__restrict finddata) {
 struct find *result;
 result = libd_w32findopen(file,O_RDONLY|O_DIRECTORY);
 if (libd_w32findnext32i64(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}

EXPORT(__DSYMw32(_wfindfirst32),libd_dos_findfirst32);
CRT_DOS struct find *LIBCCALL
libd_dos_w32findfirst32(char32_t const *__restrict file,
                        struct w32finddata32_t *__restrict finddata) {
 struct find *result;
 result = libd_w32findopen(file,O_RDONLY|O_DIRECTORY|O_DOSPATH);
 if (libd_w32findnext32(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}
EXPORT(__DSYMw32(_wfindfirst64),libd_dos_findfirst64);
CRT_DOS struct find *LIBCCALL
libd_dos_w32findfirst64(char32_t const *__restrict file,
                        struct w32finddata64_t *__restrict finddata) {
 struct find *result;
 result = libd_w32findopen(file,O_RDONLY|O_DIRECTORY|O_DOSPATH);
 if (libd_w32findnext64(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}
EXPORT(__DSYMw32(_wfindfirst32i64),libd_dos_findfirst32i64);
CRT_DOS struct find *LIBCCALL
libd_dos_w32findfirst32i64(char32_t const *__restrict file,
                           struct w32finddata32i64_t *__restrict finddata) {
 struct find *result;
 result = libd_w32findopen(file,O_RDONLY|O_DIRECTORY|O_DOSPATH);
 if (libd_w32findnext32i64(result,finddata))
     libd_findclose(result),
     result = FIND_INVALID;
 return result;
}

CRT_DOS unsigned int LIBCCALL
libd_entattrib(struct dirent *__restrict ent,
               struct __kos_stat64 *__restrict st) {
 unsigned int result = _A_NORMAL;
 if (S_ISDIR(st->st_mode)) result |= _A_SUBDIR;
 if (!(st->st_mode & 0222)) result |= _A_RDONLY;
 if (ent->d_name[0] == '.') result |= _A_HIDDEN;
 return result;
}


EXPORT(_findnext32,libd_findnext32);
CRT_DOS int LIBCCALL
libd_findnext32(struct find *findfd, struct finddata32_t *__restrict finddata) {
 struct __kos_stat64 st;
 struct dirent *ent = libd_findread(findfd,&st);
 if (!ent) return -1;
 finddata->attrib      = libd_entattrib(ent,&st);
 finddata->time_create = st.st_ctim32.tv_sec;
 finddata->time_access = st.st_atim32.tv_sec;
 finddata->time_write  = st.st_mtim32.tv_sec;
 finddata->size        = st.st_size32;
 libc_memcpy(finddata->name,ent->d_name,
             MIN(sizeof(finddata->name),
                (ent->d_namlen+1)*sizeof(char)));
 COMPILER_ENDOF(finddata->name)[-1] = 0;
 return 0;
}
EXPORT(_findnext64,libd_findnext64);
CRT_DOS int LIBCCALL
libd_findnext64(struct find *findfd, struct finddata64_t *__restrict finddata) {
 struct __kos_stat64 st;
 struct dirent *ent = libd_findread(findfd,&st);
 if (!ent) return -1;
 finddata->attrib      = libd_entattrib(ent,&st);
 finddata->time_create = st.st_ctim64.tv_sec;
 finddata->time_access = st.st_atim64.tv_sec;
 finddata->time_write  = st.st_mtim64.tv_sec;
 finddata->size        = st.st_size64;
 libc_memcpy(finddata->name,ent->d_name,
             MIN(sizeof(finddata->name),
                (ent->d_namlen+1)*sizeof(char)));
 COMPILER_ENDOF(finddata->name)[-1] = 0;
 return 0;
}
EXPORT(_findnext32i64,libd_findnext32i64);
CRT_DOS int LIBCCALL
libd_findnext32i64(struct find *findfd, struct finddata32i64_t *__restrict finddata) {
 struct __kos_stat64 st;
 struct dirent *ent = libd_findread(findfd,&st);
 if (!ent) return -1;
 finddata->attrib      = libd_entattrib(ent,&st);
 finddata->time_create = st.st_ctim32.tv_sec;
 finddata->time_access = st.st_atim32.tv_sec;
 finddata->time_write  = st.st_mtim32.tv_sec;
 finddata->size        = st.st_size64;
 libc_memcpy(finddata->name,ent->d_name,
             MIN(sizeof(finddata->name),
                (ent->d_namlen+1)*sizeof(char)));
 COMPILER_ENDOF(finddata->name)[-1] = 0;
 return 0;
}
EXPORT(__SYMw16(_wfindnext32),libd_w16findnext32);
CRT_DOS int LIBCCALL
libd_w16findnext32(struct find *findfd, struct w16finddata32_t *__restrict finddata) {
 struct __kos_stat64 st;
 mbstate_t state = MBSTATE_INIT;
 struct dirent *ent = libd_findread(findfd,&st);
 if (!ent) return -1;
 finddata->attrib      = libd_entattrib(ent,&st);
 finddata->time_create = st.st_ctim32.tv_sec;
 finddata->time_access = st.st_atim32.tv_sec;
 finddata->time_write  = st.st_mtim32.tv_sec;
 finddata->size        = st.st_size32;
 if (libc_utf8to16(ent->d_name,ent->d_namlen,
                   finddata->name,COMPILER_LENOF(finddata->name),&state,
                   UNICODE_F_ALWAYSZEROTERM|UNICODE_F_SETERRNO) ==
                   UNICODE_ERROR)
     return -1;
 COMPILER_ENDOF(finddata->name)[-1] = 0;
 return 0;
}
EXPORT(__SYMw32(_wfindnext32),libd_w32findnext32);
CRT_DOS int LIBCCALL
libd_w32findnext32(struct find *findfd, struct w32finddata32_t *__restrict finddata) {
 struct __kos_stat64 st;
 mbstate_t state = MBSTATE_INIT;
 struct dirent *ent = libd_findread(findfd,&st);
 if (!ent) return -1;
 finddata->attrib      = libd_entattrib(ent,&st);
 finddata->time_create = st.st_ctim32.tv_sec;
 finddata->time_access = st.st_atim32.tv_sec;
 finddata->time_write  = st.st_mtim32.tv_sec;
 finddata->size        = st.st_size32;
 if (libc_utf8to32(ent->d_name,ent->d_namlen,
                   finddata->name,COMPILER_LENOF(finddata->name),&state,
                   UNICODE_F_ALWAYSZEROTERM|UNICODE_F_SETERRNO) ==
                   UNICODE_ERROR)
     return -1;
 COMPILER_ENDOF(finddata->name)[-1] = 0;
 return 0;
}
EXPORT(__SYMw16(_wfindnext64),libd_w16findnext64);
CRT_DOS int LIBCCALL
libd_w16findnext64(struct find *findfd, struct w16finddata64_t *__restrict finddata) {
 struct __kos_stat64 st;
 mbstate_t state = MBSTATE_INIT;
 struct dirent *ent = libd_findread(findfd,&st);
 if (!ent) return -1;
 finddata->attrib      = libd_entattrib(ent,&st);
 finddata->time_create = st.st_ctim64.tv_sec;
 finddata->time_access = st.st_atim64.tv_sec;
 finddata->time_write  = st.st_mtim64.tv_sec;
 finddata->size        = st.st_size64;
 if (libc_utf8to16(ent->d_name,ent->d_namlen,
                   finddata->name,COMPILER_LENOF(finddata->name),&state,
                   UNICODE_F_ALWAYSZEROTERM|UNICODE_F_SETERRNO) ==
                   UNICODE_ERROR)
     return -1;
 COMPILER_ENDOF(finddata->name)[-1] = 0;
 return 0;
}
EXPORT(__SYMw32(_wfindnext64),libd_w32findnext64);
CRT_DOS int LIBCCALL
libd_w32findnext64(struct find *findfd, struct w32finddata64_t *__restrict finddata) {
 struct __kos_stat64 st;
 mbstate_t state = MBSTATE_INIT;
 struct dirent *ent = libd_findread(findfd,&st);
 if (!ent) return -1;
 finddata->attrib      = libd_entattrib(ent,&st);
 finddata->time_create = st.st_ctim64.tv_sec;
 finddata->time_access = st.st_atim64.tv_sec;
 finddata->time_write  = st.st_mtim64.tv_sec;
 finddata->size        = st.st_size64;
 if (libc_utf8to32(ent->d_name,ent->d_namlen,
                   finddata->name,COMPILER_LENOF(finddata->name),&state,
                   UNICODE_F_ALWAYSZEROTERM|UNICODE_F_SETERRNO) ==
                   UNICODE_ERROR)
     return -1;
 COMPILER_ENDOF(finddata->name)[-1] = 0;
 return 0;
}
EXPORT(__SYMw16(_wfindnext32i64),libd_w16findnext32i64);
CRT_DOS int LIBCCALL
libd_w16findnext32i64(struct find *findfd, struct w16finddata32i64_t *__restrict finddata) {
 struct __kos_stat64 st;
 mbstate_t state = MBSTATE_INIT;
 struct dirent *ent = libd_findread(findfd,&st);
 if (!ent) return -1;
 finddata->attrib      = libd_entattrib(ent,&st);
 finddata->time_create = st.st_ctim32.tv_sec;
 finddata->time_access = st.st_atim32.tv_sec;
 finddata->time_write  = st.st_mtim32.tv_sec;
 finddata->size        = st.st_size64;
 if (libc_utf8to16(ent->d_name,ent->d_namlen,
                   finddata->name,COMPILER_LENOF(finddata->name),&state,
                   UNICODE_F_ALWAYSZEROTERM|UNICODE_F_SETERRNO) ==
                   UNICODE_ERROR)
     return -1;
 COMPILER_ENDOF(finddata->name)[-1] = 0;
 return 0;
}
EXPORT(__SYMw32(_wfindnext32i64),libd_w32findnext32i64);
CRT_DOS int LIBCCALL
libd_w32findnext32i64(struct find *findfd, struct w32finddata32i64_t *__restrict finddata) {
 struct __kos_stat64 st;
 mbstate_t state = MBSTATE_INIT;
 struct dirent *ent = libd_findread(findfd,&st);
 if (!ent) return -1;
 finddata->attrib      = libd_entattrib(ent,&st);
 finddata->time_create = st.st_ctim32.tv_sec;
 finddata->time_access = st.st_atim32.tv_sec;
 finddata->time_write  = st.st_mtim32.tv_sec;
 finddata->size        = st.st_size64;
 if (libc_utf8to32(ent->d_name,ent->d_namlen,
                   finddata->name,COMPILER_LENOF(finddata->name),&state,
                   UNICODE_F_ALWAYSZEROTERM|UNICODE_F_SETERRNO) ==
                   UNICODE_ERROR)
     return -1;
 COMPILER_ENDOF(finddata->name)[-1] = 0;
 return 0;
}


/* These are just aliases. */
DEFINE_INTERN_ALIAS(libd_findfirst64i32,libd_findfirst64);
DEFINE_INTERN_ALIAS(libd_dos_findfirst64i32,libd_dos_findfirst64);
DEFINE_INTERN_ALIAS(libd_w16findfirst64i32,libd_w16findfirst64);
DEFINE_INTERN_ALIAS(libd_w32findfirst64i32,libd_w32findfirst64);
DEFINE_INTERN_ALIAS(libd_dos_w16findfirst64i32,libd_dos_w16findfirst64);
DEFINE_INTERN_ALIAS(libd_dos_w32findfirst64i32,libd_dos_w32findfirst64);
DEFINE_INTERN_ALIAS(libd_findnext64i32,libd_findnext64);
DEFINE_INTERN_ALIAS(libd_w16findnext64i32,libd_w16findnext64);
DEFINE_INTERN_ALIAS(libd_w32findnext64i32,libd_w32findnext64);

EXPORT(__KSYM(findfirst64i32),libd_findfirst64i32);
EXPORT(__DSYM(_findfirst64i32),libd_dos_findfirst64i32);
EXPORT(__KSYMw16(wfindfirst64i32),libd_w16findfirst64i32);
EXPORT(__KSYMw32(wfindfirst64i32),libd_w32findfirst64i32);
EXPORT(__DSYMw16(_wfindfirst64i32),libd_dos_w16findfirst64i32);
EXPORT(__DSYMw32(_wfindfirst64i32),libd_dos_w32findfirst64i32);
EXPORT(_findnext64i32,libd_findnext64i32);
EXPORT(__SYMw16(_wfindnext64i32),libd_w16findnext64i32);
EXPORT(__SYMw32(_wfindnext64i32),libd_w32findnext64i32);


/* Old DOS names. */
#undef _findfirst
#undef _findnext
#undef _wfindfirst
#undef _wfindnext
EXPORT(__KSYM(findfirst),libd_findfirst32);
EXPORT(__DSYM(_findfirst),libd_dos_findfirst32);
EXPORT(__KSYMw16(wfindfirst),libd_w16findfirst32);
EXPORT(__KSYMw32(wfindfirst),libd_w32findfirst32);
EXPORT(__DSYMw16(_wfindfirst),libd_dos_w16findfirst32);
EXPORT(__DSYMw32(_wfindfirst),libd_dos_w32findfirst32);
EXPORT(_findnext,libd_findnext32);
EXPORT(__SYMw16(_wfindnext),libd_w16findnext32);
EXPORT(__SYMw32(_wfindnext),libd_w32findnext32);

#undef _findfirsti64
#undef _findnexti64
#undef _wfindfirsti64
#undef _wfindnexti64
EXPORT(__KSYM(findfirsti64),libd_findfirst32i64);
EXPORT(__DSYM(_findfirsti64),libd_dos_findfirst32i64);
EXPORT(__KSYMw16(wfindfirsti64),libd_w16findfirst32i64);
EXPORT(__KSYMw32(wfindfirsti64),libd_w32findfirst32i64);
EXPORT(__DSYMw16(_wfindfirsti64),libd_dos_w16findfirst32i64);
EXPORT(__DSYMw32(_wfindfirsti64),libd_dos_w32findfirst32i64);
EXPORT(_findnexti64,libd_findnext32i64);
EXPORT(__SYMw16(_wfindnexti64),libd_w16findnext32i64);
EXPORT(__SYMw32(_wfindnexti64),libd_w32findnext32i64);


DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_DOS_FIND_C */
