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
#ifndef GUARD_LIBS_LIBC_MOUNT_C
#define GUARD_LIBS_LIBC_MOUNT_C 1

#include "libc.h"
#include "mount.h"
#include "errno.h"
#include "system.h"
#include "stdio/file.h"

#include <sys/mount.h>
#include <mntent.h>
#include <hybrid/section.h>
#include <errno.h>

DECL_BEGIN

INTERN FILE *LIBCCALL
libc_setmntent(char const *file, char const *mode) {
 return libc_fopen(file,mode);
}
INTERN int LIBCCALL
libc_endmntent(FILE *stream) {
 libc_fclose(stream);
 return 1;
}
INTERN struct mntent *LIBCCALL
libc_getmntent_r(FILE *__restrict stream,
                 struct mntent *__restrict result,
                 char *__restrict buffer, int bufsize) {
 libc_seterrno(ENOSYS); /* TODO */
 return NULL;
}

PRIVATE ATTR_COLDBSS struct mntent static_mntent;
PRIVATE ATTR_COLDBSS char static_mntbuf[2048];
INTERN struct mntent *LIBCCALL
libc_getmntent(FILE *__restrict stream) {
 return libc_getmntent_r(stream,&static_mntent,static_mntbuf,sizeof(static_mntbuf));
}
INTERN int LIBCCALL
libc_addmntent(FILE *__restrict stream,
               struct mntent const *__restrict mnt) {
 libc_seterrno(ENOSYS); /* TODO */
 return -1;
}

INTERN char *LIBCCALL
libc_hasmntopt(struct mntent const *mnt,
               char const *opt) {
 char *next_opt;
 next_opt = mnt->mnt_opts;
 if (next_opt) {
  size_t optlen = libc_strlen(opt);
  for (;;) {
   if (libc_memcmp(next_opt,opt,optlen*sizeof(char)) == 0)
       return next_opt;
   next_opt = libc_strchr(next_opt,',');
   if (!next_opt) break;
   ++next_opt;
  }
 }
 return NULL;
}

INTERN int LIBCCALL
libc_mount(char const *special_file,
           char const *dir, char const *fstype,
           unsigned long int rwflag,
           void const *data) {
 return FORWARD_SYSTEM_ERROR(sys_mount(special_file,dir,fstype,rwflag,data));
}
INTERN int LIBCCALL
libc_umount(char const *special_file) {
 return libc_umount2(special_file,0);
}
INTERN void LIBCCALL
libc_Xumount(char const *special_file) {
 libc_Xumount2(special_file,0);
}
INTERN int LIBCCALL
libc_umount2(char const *special_file,
             int flags) {
 return FORWARD_SYSTEM_ERROR(sys_umount2(special_file,flags));
}


EXPORT(setmntent,libc_setmntent);
EXPORT(getmntent,libc_getmntent);
EXPORT(endmntent,libc_endmntent);
EXPORT(getmntent_r,libc_getmntent_r);
EXPORT(addmntent,libc_addmntent);
EXPORT(hasmntopt,libc_hasmntopt);
EXPORT(mount,libc_mount);
EXPORT(umount,libc_umount);
EXPORT(umount2,libc_umount2);
EXPORT(Xumount,libc_Xumount);


/* GLibc Aliases */
EXPORT_STRONG(__getmntent_r,libc_getmntent_r);

DECL_END

#endif /* !GUARD_LIBS_LIBC_MOUNT_C */
