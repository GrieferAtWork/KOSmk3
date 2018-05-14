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
#ifndef GUARD_KERNEL_SRC_FS_HANDLE_WEAK_C
#define GUARD_KERNEL_SRC_FS_HANDLE_WEAK_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/sections.h>
#include <kernel/vm.h>
#include <sched/task.h>
#include <fs/pipe.h>
#include <fs/handle.h>
#include <fs/device.h>
#include <except.h>

DECL_BEGIN

INTERN ATTR_NOTHROW void KCALL handle_none_incref(void *__restrict UNUSED(ptr)) { }
INTERN ATTR_NOTHROW void KCALL handle_none_decref(void *__restrict UNUSED(ptr)) { }

INTDEF size_t KCALL illegal_write(void *__restrict ptr, USER CHECKED void const *buf, size_t bufsize, iomode_t flags);
DEFINE_INTERN_ALIAS(illegal_write,illegal_read);
INTDEF size_t KCALL illegal_pwrite(void *__restrict ptr, USER CHECKED void const *buf, size_t bufsize, pos_t pos, iomode_t flags);
DEFINE_INTERN_ALIAS(illegal_pwrite,illegal_pread);

INTERN size_t KCALL
illegal_read(void *__restrict UNUSED(ptr),
             USER CHECKED void *UNUSED(buf),
             size_t UNUSED(bufsize),
             iomode_t UNUSED(flags)) {
 error_throw(E_NOT_IMPLEMENTED);
}
INTERN pos_t KCALL
illegal_seek(void *__restrict UNUSED(ptr),
          off_t UNUSED(off),
          int UNUSED(whence)) {
 error_throw(E_NOT_IMPLEMENTED);
}
INTERN size_t KCALL
illegal_readdir(void *__restrict UNUSED(ptr),
             USER CHECKED struct dirent *UNUSED(buf),
             size_t UNUSED(bufsize),
             int UNUSED(mode), iomode_t UNUSED(flags)) {
 error_throw(E_NOT_IMPLEMENTED);
}
INTERN size_t KCALL
illegal_pread(void *__restrict UNUSED(ptr),
           USER CHECKED void *UNUSED(buf),
           size_t UNUSED(bufsize),
           pos_t UNUSED(pos), iomode_t UNUSED(flags)) {
 error_throw(E_NOT_IMPLEMENTED);
}
INTERN void KCALL
illegal_truncate(void *__restrict UNUSED(ptr),
              pos_t UNUSED(new_smaller_size)) {
 error_throw(E_NOT_IMPLEMENTED);
}
INTERN void KCALL
illegal_allocate(void *__restrict UNUSED(ptr),
                 int UNUSED(mode),
                 pos_t UNUSED(start),
                 pos_t UNUSED(length),
                 iomode_t UNUSED(flags)) {
 error_throw(E_INVALID_ARGUMENT);
}
INTERN ssize_t KCALL
illegal_ioctl(void *__restrict UNUSED(ptr),
              unsigned long UNUSED(cmd),
              USER UNCHECKED void *UNUSED(arg),
              iomode_t UNUSED(flags)) {
 error_throw(E_NOT_IMPLEMENTED);
}
INTERN void KCALL
illegal_sync(void *__restrict UNUSED(ptr),
             bool UNUSED(only_data)) {
 error_throw(E_NOT_IMPLEMENTED);
}
INTERN void KCALL
illegal_stat(void *__restrict UNUSED(ptr),
             USER CHECKED struct stat64 *UNUSED(result)) {
 error_throw(E_NOT_IMPLEMENTED);
}
INTERN unsigned int KCALL
illegal_poll(void *__restrict UNUSED(ptr),
             unsigned int UNUSED(mode)) {
 return 0;
}



#define FOREACH_HANDLE_TYPE(macro) \
    macro(none,HANDLE_TYPE_FNONE) \
    macro(device,HANDLE_TYPE_FDEVICE) \
    macro(inode,HANDLE_TYPE_FINODE) \
    macro(file,HANDLE_TYPE_FFILE) \
    macro(superblock,HANDLE_TYPE_FSUPERBLOCK) \
    macro(directory_entry,HANDLE_TYPE_FDIRECTORY_ENTRY) \
    macro(path,HANDLE_TYPE_FPATH) \
    macro(fs,HANDLE_TYPE_FFS) \
    macro(module,HANDLE_TYPE_FMODULE) \
    macro(application,HANDLE_TYPE_FAPPLICATION) \
    macro(thread,HANDLE_TYPE_FTHREAD) \
    macro(vm,HANDLE_TYPE_FVM) \
    macro(vm_region,HANDLE_TYPE_FVM_REGION) \
    macro(pipe,HANDLE_TYPE_FPIPE) \
    macro(pipereader,HANDLE_TYPE_FPIPEREADER) \
    macro(pipewriter,HANDLE_TYPE_FPIPEWRITER) \
    macro(socket,HANDLE_TYPE_FSOCKET) \
    macro(futex,HANDLE_TYPE_FFUTEX) \
    macro(futex_handle,HANDLE_TYPE_FFUTEX_HANDLE) \
    macro(device_stream,HANDLE_TYPE_FDEVICE_STREAM) \
/**/

#define DEFINE_WEAK_OPS(name,id) \
    INTDEF              void         KCALL handle_##name##_incref(void *__restrict ptr); \
    INTDEF ATTR_NOTHROW void         KCALL handle_##name##_decref(void *__restrict ptr); \
    INTDEF              size_t       KCALL handle_##name##_read(void *__restrict ptr, USER CHECKED void *buf, size_t bufsize, iomode_t flags); \
    INTDEF              size_t       KCALL handle_##name##_write(void *__restrict ptr, USER CHECKED void const *buf, size_t bufsize, iomode_t flags); \
    INTDEF              pos_t        KCALL handle_##name##_seek(void *__restrict ptr, off_t off, int whence); \
    INTDEF              size_t       KCALL handle_##name##_readdir(void *__restrict ptr, USER CHECKED struct dirent *buf, size_t bufsize, int mode, iomode_t flags); \
    INTDEF              size_t       KCALL handle_##name##_pread(void *__restrict ptr, USER CHECKED void *buf, size_t bufsize, pos_t pos, iomode_t flags); \
    INTDEF              size_t       KCALL handle_##name##_pwrite(void *__restrict ptr, USER CHECKED void const *buf, size_t bufsize, pos_t pos, iomode_t flags); \
    INTDEF              void         KCALL handle_##name##_truncate(void *__restrict ptr, pos_t new_smaller_size); \
    INTDEF              void         KCALL handle_##name##_allocate(void *__restrict ptr, int mode, pos_t start, pos_t length, iomode_t flags); \
    INTDEF              ssize_t      KCALL handle_##name##_ioctl(void *__restrict ptr, unsigned long cmd, USER UNCHECKED void *arg, iomode_t flags); \
    INTDEF              void         KCALL handle_##name##_sync(void *__restrict ptr, bool only_data); \
    INTDEF              void         KCALL handle_##name##_stat(void *__restrict ptr, USER CHECKED struct stat64 *result); \
    INTDEF              unsigned int KCALL handle_##name##_poll(void *__restrict ptr, unsigned int mode); \
/**/

#define DEFINE_INTERN_WEAKALIAS(new,old) \
    __asm__(".weak " PP_PRIVATE_STR(new) "\n" \
            ".global " PP_PRIVATE_STR(new) "\n" \
            ".hidden " PP_PRIVATE_STR(new) "\n" \
            ".set " PP_PRIVATE_STR(new) "," PP_PRIVATE_STR(old) "\n")

#define REDIRECT_WEAK_OPS(name,id) \
    DEFINE_INTERN_WEAKALIAS(handle_##name##_read,illegal_read); \
    DEFINE_INTERN_WEAKALIAS(handle_##name##_write,illegal_write); \
    DEFINE_INTERN_WEAKALIAS(handle_##name##_seek,illegal_seek); \
    DEFINE_INTERN_WEAKALIAS(handle_##name##_readdir,illegal_readdir); \
    DEFINE_INTERN_WEAKALIAS(handle_##name##_pread,illegal_pread); \
    DEFINE_INTERN_WEAKALIAS(handle_##name##_pwrite,illegal_pwrite); \
    DEFINE_INTERN_WEAKALIAS(handle_##name##_truncate,illegal_truncate); \
    DEFINE_INTERN_WEAKALIAS(handle_##name##_allocate,illegal_allocate); \
    DEFINE_INTERN_WEAKALIAS(handle_##name##_ioctl,illegal_ioctl); \
    DEFINE_INTERN_WEAKALIAS(handle_##name##_sync,illegal_sync); \
    DEFINE_INTERN_WEAKALIAS(handle_##name##_stat,illegal_stat); \
    DEFINE_INTERN_WEAKALIAS(handle_##name##_poll,illegal_poll); \
/**/


FOREACH_HANDLE_TYPE(DEFINE_WEAK_OPS)   /* Define function prototypes */
FOREACH_HANDLE_TYPE(REDIRECT_WEAK_OPS) /* Weakly redirect functions to illegal version.
                                        * Any operator that isn't implemented will point to a
                                        * function that simply throws an E_NOT_IMPLEMENTED error.
                                        * NOTE: incref() / decref() operators are not redirected. */


#define ENUM_INCREF(name,id)   [id] = &handle_##name##_incref,
#define ENUM_DECREF(name,id)   [id] = &handle_##name##_decref,
#define ENUM_READ(name,id)     [id] = &handle_##name##_read,
#define ENUM_WRITE(name,id)    [id] = &handle_##name##_write,
#define ENUM_SEEK(name,id)     [id] = &handle_##name##_seek,
#define ENUM_READDIR(name,id)  [id] = &handle_##name##_readdir,
#define ENUM_PREAD(name,id)    [id] = &handle_##name##_pread,
#define ENUM_PWRITE(name,id)   [id] = &handle_##name##_pwrite,
#define ENUM_TRUNCATE(name,id) [id] = &handle_##name##_truncate,
#define ENUM_ALLOCATE(name,id) [id] = &handle_##name##_allocate,
#define ENUM_IOCTL(name,id)    [id] = &handle_##name##_ioctl,
#define ENUM_SYNC(name,id)     [id] = &handle_##name##_sync,
#define ENUM_STAT(name,id)     [id] = &handle_##name##_stat,
#define ENUM_POLL(name,id)     [id] = &handle_##name##_poll,
                                        
PUBLIC struct handle_types_struct const handle_types = {
    .ht_incref   = { FOREACH_HANDLE_TYPE(ENUM_INCREF) },
    .ht_decref   = { FOREACH_HANDLE_TYPE(ENUM_DECREF) },
    .ht_read     = { FOREACH_HANDLE_TYPE(ENUM_READ) },
    .ht_write    = { FOREACH_HANDLE_TYPE(ENUM_WRITE) },
    .ht_seek     = { FOREACH_HANDLE_TYPE(ENUM_SEEK) },
    .ht_readdir  = { FOREACH_HANDLE_TYPE(ENUM_READDIR) },
    .ht_pread    = { FOREACH_HANDLE_TYPE(ENUM_PREAD) },
    .ht_pwrite   = { FOREACH_HANDLE_TYPE(ENUM_PWRITE) },
    .ht_truncate = { FOREACH_HANDLE_TYPE(ENUM_TRUNCATE) },
    .ht_allocate = { FOREACH_HANDLE_TYPE(ENUM_ALLOCATE) },
    .ht_ioctl    = { FOREACH_HANDLE_TYPE(ENUM_IOCTL) },
    .ht_sync     = { FOREACH_HANDLE_TYPE(ENUM_SYNC) },
    .ht_stat     = { FOREACH_HANDLE_TYPE(ENUM_STAT) },
    .ht_poll     = { FOREACH_HANDLE_TYPE(ENUM_POLL) },
};




DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_HANDLE_WEAK_C */
