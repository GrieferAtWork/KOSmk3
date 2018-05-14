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
#ifndef GUARD_KERNEL_SRC_FS_DEVICE_STREAM_C
#define GUARD_KERNEL_SRC_FS_DEVICE_STREAM_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <fs/device.h>
#include <fs/path.h>
#include <kernel/malloc.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <except.h>
#include <sys/stat.h>

DECL_BEGIN


/* Handle operators. */
INTERN size_t KCALL
handle_device_stream_read(struct device_stream *__restrict self,
                          USER CHECKED void *buf, size_t bufsize,
                          iomode_t flags) {
 size_t result;
 pos_t old_pos,new_pos;
 do {
  old_pos = ATOMIC_READ(self->ds_offset);
  result  = device_pread(self->ds_device,buf,bufsize,old_pos,flags);
  new_pos = old_pos + result; /* Advance the read/write-pointer. */
 } while (!ATOMIC_CMPXCH(self->ds_offset,old_pos,new_pos));
 return result;
}
INTERN size_t KCALL
handle_device_stream_write(struct device_stream *__restrict self,
                           USER CHECKED void const *buf,
                           size_t bufsize, iomode_t flags) {
 size_t result;
 pos_t old_pos,new_pos;
 do {
  old_pos = ATOMIC_READ(self->ds_offset);
  result  = device_pwrite(self->ds_device,buf,bufsize,old_pos,flags);
  new_pos = old_pos + result; /* Advance the read/write-pointer. */
 } while (!ATOMIC_CMPXCH(self->ds_offset,old_pos,new_pos));
 return result;
}
INTERN unsigned int KCALL
handle_device_stream_poll(struct device_stream *__restrict self,
                          unsigned int mode) {
 return device_poll(self->ds_device,mode);
}
INTERN pos_t KCALL
handle_device_stream_seek(struct device_stream *__restrict self,
                          off_t offset, int whence) {
 pos_t old_pos,new_pos;
 do {
  old_pos = ATOMIC_READ(self->ds_offset);
  switch (whence) {

  case SEEK_SET:
   new_pos = (pos_t)offset;
   break;

  case SEEK_CUR:
   if unlikely(offset < 0 && (pos_t)-offset > old_pos)
      error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_NEGATIVE_SEEK);
   new_pos = old_pos + (pos_t)offset;
   break;

  {
   struct stat64 st;
  case SEEK_END:
   device_stat(self->ds_device,&st);
   if unlikely(offset < 0 && (pos_t)-offset > st.st_size64)
      error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_NEGATIVE_SEEK);
   new_pos = st.st_size64 + offset;
  } break;

  default:
   error_throw(E_INVALID_ARGUMENT);
  }
  /* Adjust the file pointer. */
 } while (!ATOMIC_CMPXCH(self->ds_offset,old_pos,new_pos));
 return new_pos;
}

INTERN size_t KCALL
handle_device_stream_pread(struct device_stream *__restrict self,
                           USER CHECKED void *buf, size_t bufsize,
                           pos_t offset, iomode_t flags) {
 return device_pread(self->ds_device,buf,bufsize,offset,flags);
}
INTERN size_t KCALL
handle_device_stream_pwrite(struct device_stream *__restrict self,
                            USER CHECKED void const *buf, size_t bufsize,
                            pos_t offset, iomode_t flags) {
 return device_pwrite(self->ds_device,buf,bufsize,offset,flags);
}
INTERN void KCALL
handle_device_stream_sync(struct device_stream *__restrict self,
                          bool UNUSED(only_data)) {
 device_sync(self->ds_device);
}
INTERN ssize_t KCALL
handle_device_stream_ioctl(struct device_stream *__restrict self, unsigned long cmd,
                           USER UNCHECKED void *arg, iomode_t flags) {
 return device_ioctl(self->ds_device,cmd,arg,flags);
}
INTERN void KCALL
handle_device_stream_stat(struct device_stream *__restrict self,
                          USER CHECKED struct stat64 *result) {
 device_stat(self->ds_device,result);
}

PUBLIC ATTR_NOTHROW void KCALL
device_stream_destroy(struct device_stream *__restrict self) {
 device_decref(self->ds_device);
 path_decref(self->ds_path);
 kfree(self);
}


PUBLIC REF struct handle KCALL
device_open_stream(struct device *__restrict self,
                   struct path *__restrict p,
                   oflag_t open_mode) {
 REF struct handle result;
 REF struct device_stream *stream;
 /* Construct a new device stream. */
 stream = (REF struct device_stream *)kmalloc(sizeof(struct device_stream),
                                              GFP_SHARED);
 stream->ds_refcnt = 1;
 stream->ds_device = self;
 stream->ds_path   = p;
 stream->ds_offset = 0;
 device_incref(self);
 path_incref(p);
 
 result.h_mode = HANDLE_MODE(HANDLE_TYPE_FDEVICE_STREAM,
                             IO_FROM_O(open_mode));
 result.h_object.o_device_stream = stream; /* Inherit reference. */
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_DEVICE_STREAM_C */
