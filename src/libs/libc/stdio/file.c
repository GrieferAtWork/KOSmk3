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
#ifndef GUARD_LIBS_LIBC_STDIO_FILE_C
#define GUARD_LIBS_LIBC_STDIO_FILE_C 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpointer-sign"
#endif

#include "file.h"
#ifndef CONFIG_LIBC_USES_NEW_STDIO
#include "../libc.h"
#include "../unistd.h"
#include "../rtl.h"
#include "../tty.h"
#include "../malloc.h"
#include "../errno.h"
#include "../system.h"
#include "../unicode.h"

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <hybrid/compiler.h>
#include <kos/types.h>
#include <bits/fcntl-linux.h>
#include <hybrid/minmax.h>
#include <assert.h>
#include <syslog.h>
#include <hybrid/align.h>
#ifndef CONFIG_LIBC_NO_DOS_LIBC
#include <bits/dos-errno.h>
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_BEGIN

#undef stdin
#undef stdout
#undef stderr


PRIVATE struct iofile_data io_stdin  = { .io_lock = ATOMIC_OWNER_RWLOCK_INIT, };
PRIVATE struct iofile_data io_stdout = { .io_lock = ATOMIC_OWNER_RWLOCK_INIT, };
PRIVATE struct iofile_data io_stderr = { .io_lock = ATOMIC_OWNER_RWLOCK_INIT, };

INTERN FILE libc_std_files[3] = {
    [0] = { .if_flag = IO_LNBUF, .if_fd = STDIN_FILENO,  .if_exdata = &io_stdin,  },
#if 0
    [1] = { .if_flag = IO_USERBUF, .if_fd = STDOUT_FILENO, .if_exdata = &io_stdout, },
    [2] = { .if_flag = IO_USERBUF, .if_fd = STDERR_FILENO, .if_exdata = &io_stderr, },
#else
    [1] = { .if_flag = IO_LNIFTYY, .if_fd = STDOUT_FILENO, .if_exdata = &io_stdout, },
    [2] = { .if_flag = IO_LNIFTYY, .if_fd = STDERR_FILENO, .if_exdata = &io_stderr, },
#endif
};

PUBLIC FILE *stdin  = &libc_std_files[0];
PUBLIC FILE *stdout = &libc_std_files[1];
PUBLIC FILE *stderr = &libc_std_files[2];

INTERN DEFINE_ATOMIC_RWLOCK(libc_ffiles_lock);
INTERN DEFINE_ATOMIC_RWLOCK(libc_flnchg_lock);
INTERN LIST_HEAD(FILE) libc_ffiles = NULL;
INTERN LIST_HEAD(FILE) libc_flnchg = NULL;

INTERN void LIBCCALL
libc_flush_changed_lnbuf_files(FILE *__restrict sender) {
 while (ATOMIC_READ(libc_flnchg)) {
  FILE *flush_file;
  atomic_rwlock_read(&libc_flnchg_lock);
  COMPILER_READ_BARRIER();
  if (!libc_flnchg) { atomic_rwlock_endread(&libc_flnchg_lock); break; }
  if (!atomic_rwlock_upgrade(&libc_flnchg_lock) &&
      !ATOMIC_READ(libc_flnchg)) { atomic_rwlock_endwrite(&libc_flnchg_lock); break; }
  flush_file = libc_flnchg;
  if (flush_file != sender && !file_trywrite(flush_file)) {
   atomic_rwlock_endwrite(&libc_flnchg_lock);
   continue;
  }
  /* Unlink the file from the list of changed streams. */
  LIST_REMOVE(flush_file,if_exdata->io_lnch);
  LIST_MKUNBOUND(flush_file,if_exdata->io_lnch);
  atomic_rwlock_endwrite(&libc_flnchg_lock);
  /* Flush the file. */
  libc_fdoflush(flush_file);
  if (flush_file != sender)
      file_endwrite(flush_file);
 }
}

LOCAL void LIBCCALL
libc_fchecktty(FILE *__restrict self) {
 if (self->if_flag&IO_LNIFTYY) {
  self->if_flag &= ~IO_LNIFTYY;
  if (libc_isatty(self->if_fd)) {
   libc_syslog(LOG_DEBUG,"[LIBC] Stream handle %d is a tty\n",self->if_fd);
   self->if_flag |= IO_LNBUF;
  }
 }
}


INTERN size_t LIBCCALL
libc_fdoread(void *__restrict buf, size_t size, FILE *__restrict self) {
 size_t result,part,minsize;
 char *buffer; ssize_t temp;

 /* Read data from the loaded buffer. */
 result = MIN(self->if_cnt,size);
 libc_memcpy(buf,self->if_ptr,result);
 self->if_ptr += result;
 self->if_cnt -= result;
 size -= result;
 if (!size) goto end;
 *(uintptr_t *)&buf += result;
 libc_fchecktty(self);

 /* Read everything that is too large directly. */
#if !(IOBUF_MAX & (IOBUF_MAX-1))
 part = size & ~(IOBUF_MAX-1);
#else
 part = (size/IOBUF_MAX)*IOBUF_MAX;
#endif
 if (part) {
  if (self->if_flag&IO_LNBUF)
      libc_flush_changed_lnbuf_files(self);
  temp = libc_read(self->if_fd,buf,part);
  if (temp <= 0) goto err;
  self->if_exdata->io_pos += temp;
  result                  += temp;
  size                    -= temp;
  if (!size) goto end;
  *(uintptr_t *)&buf += temp;
 }
 assert(size);
 assert(!self->if_cnt);
 if (self->if_flag&IO_USERBUF) {
  /* Read all data that doesn't fit into the buffer directly. */
part_again:
  if (!self->if_bufsiz) part = size;
  else part = (size/self->if_bufsiz)*self->if_bufsiz;
  if (part) {
   if (self->if_flag&IO_LNBUF)
       libc_flush_changed_lnbuf_files(self);
   temp = libc_read(self->if_fd,buf,part);
   if (temp <= 0) goto err;
   *(uintptr_t *)&buf += temp;
   size -= temp;
   if ((size_t)temp != part)
       goto part_again;
  }
  if (!size) goto end;

  /* Fill the buffer. */
  assert(self->if_bufsiz);
  if (self->if_flag&IO_LNBUF)
      libc_flush_changed_lnbuf_files(self);
  temp = libc_read(self->if_fd,self->if_base,self->if_bufsiz);
  if (temp <= 0) goto err;
  self->if_ptr = self->if_base;
  self->if_cnt = (size_t)temp;
  self->if_exdata->io_read = (size_t)temp;
  self->if_flag |= IO_R;
  self->if_flag &= ~IO_W;
  goto load_buffer;
 }

 /* Allocate/Re-allocate a buffer of sufficient size. */
 minsize = CEIL_ALIGN(size,IOBUF_MIN);
 buffer  = self->if_base;
 if (minsize > self->if_bufsiz) {
  /* Must allocate more memory. */
  buffer = (char *)libc_realloc(buffer,minsize);
  if unlikely(!buffer) goto direct_io;
  self->if_base   = buffer;
  self->if_bufsiz = minsize;
  self->if_flag  |= IO_MALLBUF;
 } else if ((self->if_bufsiz-minsize) >=
             IOBUF_RELOCATE_THRESHOLD) {
  /* Try to free unused data. */
  assert(self->if_flag&IO_MALLBUF);
  buffer = (char *)libc_realloc(buffer,minsize);
  if unlikely(!buffer) { buffer = self->if_base; goto fill_buffer; }
  self->if_base   = buffer;
  self->if_bufsiz = minsize;
 }
fill_buffer:
 /* Read data into the buffer. */
 assert(minsize);
 if (self->if_flag&IO_LNBUF)
     libc_flush_changed_lnbuf_files(self);
 temp = libc_read(self->if_fd,buffer,minsize);
 if (temp <= 0) goto err;
 self->if_exdata->io_read = (size_t)temp;
 self->if_exdata->io_pos += temp;
 self->if_cnt = (size_t)temp;
 self->if_ptr = buffer;
 self->if_flag |= IO_R;
 self->if_flag &= ~IO_W;
load_buffer:
 part = MIN((size_t)temp,size);
 /* Copy data out of the buffer. */
 libc_memcpy(buf,self->if_ptr,part);
 self->if_ptr += part;
 self->if_cnt -= part;
 result       += part;
end:
 /* Update the EOF flag according to the result. */
 return result;
direct_io:
 /* Read the remainder using direct I/O. */
 if (self->if_flag&IO_LNBUF)
     libc_flush_changed_lnbuf_files(self);
 temp = libc_read(self->if_fd,buf,size);
 if (temp <= 0) goto err;
 result                  += temp;
 self->if_exdata->io_pos += temp;
 goto end;
err:
 if (temp == 0) { self->if_flag |= IO_EOF; return result; }
 self->if_flag |= IO_ERR;
 return 0;
}

INTERN int LIBCCALL
libc_fdoflush(FILE *__restrict self) {
 size_t flushsize; ssize_t temp;
 char *write_pointer; size_t write_size;
 /* Don't do anything if the buffer hasn't changed, or doesn't have a handle. */
 if (!(self->if_flag&IO_W) || (self->if_flag&IO_NOFD)) return 0;
 flushsize = (size_t)(self->if_ptr-self->if_base);
 assertf(flushsize <= self->if_bufsiz,"Invalid file layout (ptr: %p; buf: %p...%p)",
         self->if_ptr,self->if_base,self->if_base+self->if_bufsiz-1);
 /* If the input buffer was read before, we must seek
  * backwards to get back to where it was read from. */
 if (self->if_flag&IO_R && self->if_exdata->io_read) {
  __off_t pos = libc_lseek64(self->if_fd,-(ssize_t)self->if_exdata->io_read,SEEK_CUR);
  if (pos < 0) goto err;
  self->if_exdata->io_pos = (__pos_t)pos;
 }
 /* Write the entirety of the current buffer up until the current R/W position. */
 write_pointer = self->if_base;
 write_size    = (size_t)((uintptr_t)self->if_ptr-
                          (uintptr_t)write_pointer);
 while (write_size) {
  temp = libc_write(self->if_fd,write_pointer,write_size);
  if (temp < 0) goto err;
  if (!temp) {
   self->if_flag |= IO_EOF;
#ifdef CONFIG_FILE_DATASYNC_DURING_FLUSH
   if (libc_fdatasync(self->if_fd))
       goto err;
#endif
   return 0; /* XXX: Is this correct? */
  }
  self->if_exdata->io_pos += temp;
  write_pointer           += temp;
  write_size              -= temp;
 }
 if (self->if_flag&IO_LNBUF) {
  atomic_rwlock_write(&libc_flnchg_lock);
  if (!LIST_ISUNBOUND(self,if_exdata->io_lnch)) {
   LIST_REMOVE(self,if_exdata->io_lnch);
   LIST_MKUNBOUND(self,if_exdata->io_lnch);
  }
  atomic_rwlock_endwrite(&libc_flnchg_lock);
 }

 /* Delete the changed and EOF flags. */
 self->if_flag &= ~(IO_EOF|IO_W|IO_R);
 /* Mark the buffer as empty. */
 self->if_exdata->io_read = 0;
 self->if_ptr = self->if_base;
 self->if_cnt = 0;
#ifdef CONFIG_FILE_DATASYNC_DURING_FLUSH
 /* Do a disk sync. */
 if (libc_fdatasync(self->if_fd))
     goto err;
#endif
 return 0;
err:
 self->if_flag |= IO_ERR;
 return -1;
}

INTERN int LIBCCALL
libc_doffill(FILE *__restrict self) {
 size_t avail; ssize_t temp;
 avail = (self->if_base+self->if_bufsiz)-
         (self->if_ptr+self->if_cnt);
 if (!avail) {
  if (!self->if_bufsiz &&
      !(self->if_flag&IO_USERBUF)) {
   avail = IOBUF_MIN;
   /* Allocate an initial buffer. */
   do self->if_base = (char *)libc_malloc(avail);
   while (!self->if_base && (avail /= 2) != 0);
   if (!self->if_base) goto err;
   self->if_ptr    = self->if_base;
   self->if_bufsiz = avail;
  } else {
   /* Don't do anything if no data needs to be read. */
   return 0;
  }
 }
 assert(avail);
 libc_fchecktty(self);
 if (self->if_flag&IO_LNBUF)
     libc_flush_changed_lnbuf_files(self);
 /* Read more data. */
 temp = libc_read(self->if_fd,
                  self->if_ptr+self->if_cnt,
                  avail);
 if (temp <= 0) {
  if (temp) goto err;
  /* Handle EOF. (We don't signal it unless no more data can be read) */
  if (!self->if_cnt) {
   self->if_flag |= IO_EOF;
   return -1;
  }
 } else {
  /* Update the file to mirror newly available data. */
  self->if_flag &= ~(IO_EOF);
  self->if_cnt             += temp;
  self->if_exdata->io_read += temp;
  self->if_exdata->io_pos  += temp;
 }
 return 0;
err:
 self->if_flag |= IO_ERR;
 return -1;
}

LOCAL void LIBCCALL
libc_fmarkchanged(FILE *__restrict self) {
 if (self->if_flag&IO_W) return;
 self->if_flag |= IO_W;
 if (self->if_flag&IO_LNBUF) {
  atomic_rwlock_write(&libc_flnchg_lock);
  if (LIST_ISUNBOUND(self,if_exdata->io_lnch))
      LIST_INSERT(libc_flnchg,self,if_exdata->io_lnch);
  atomic_rwlock_endwrite(&libc_flnchg_lock);
 }
}

INTERN size_t LIBCCALL
libc_fdowrite(void const *__restrict buf, size_t size, FILE *__restrict self) {
 size_t result,part,minsize;
 char *buffer; ssize_t temp;
buffer_write_more:
 /* Write data to buffer (including to the overflow area). */
 result = MIN((size_t)((self->if_base+self->if_bufsiz)-self->if_ptr),size);
 libc_fchecktty(self);
 if (result) {
  libc_memcpy(self->if_ptr,buf,result);
  libc_fmarkchanged(self);
  self->if_ptr += result;
  if (result >= self->if_cnt)
      self->if_cnt = 0;
  else self->if_cnt -= result;
  size -= result;
  /* Flush the buffer if it is line-buffered. */
  if (self->if_flag&IO_LNBUF &&
      libc_memchr(buf,'\n',result)) {
   if (libc_fdoflush(self)) return 0;
   /* With the buffer now empty, we must write more data to it. */
   goto buffer_write_more;
  }
  if (!size) goto end;
  *(uintptr_t *)&buf += result;
 }
 if (!size) goto end;
 assert(!self->if_cnt);
 assert(self->if_ptr == self->if_base+self->if_bufsiz);

 /* Use direct I/O for anything that doesn't fit into the buffer. */
part_again:
 if (self->if_flag&IO_USERBUF) {
  if (!self->if_bufsiz) part = size;
  else part = (size/self->if_bufsiz)*self->if_bufsiz;
 } else {
  part = (size/IOBUF_MAX)*IOBUF_MAX;
 }
 /* Special case: if the last part contains line-feeds in a
  *               line-buffered file, use direct I/O for that part as well. */
 if (self->if_flag&IO_LNBUF &&
     libc_memchr((byte_t *)buf+part,'\n',size-part))
     part = size;
 if (part) {
  /* Flush the buffer before performing direct I/O to preserve write order. */
  if (libc_fdoflush(self)) return 0;
  temp = libc_write(self->if_fd,buf,part);
  if (temp < 0) goto err;
  self->if_exdata->io_pos += temp;
  result += temp;
  size   -= temp;
  if (!size) goto end;
  *(uintptr_t *)&buf += part;
  if ((size_t)temp != part) goto part_again;
 }
 /* Write the remainder to the buffer.
  * NOTE: we've already confirmed that it doesn't contain a line-feed. */
 assert(!(self->if_flag&IO_R));
 assert(!(self->if_flag&IO_LNBUF) || !libc_memchr(buf,'\n',size));
 buffer = self->if_base;
 if (!(self->if_flag&IO_USERBUF)) {
  /* Make sure the buffer is of sufficient size. */
  minsize = CEIL_ALIGN(size,IOBUF_MIN);
  if (minsize > self->if_bufsiz) {
   buffer = (char *)libc_realloc(buffer,minsize);
   if unlikely(!buffer) goto direct_io;
   self->if_base   = buffer;
   self->if_bufsiz = minsize;
   self->if_flag  |= IO_MALLBUF;
  } else if ((self->if_bufsiz-minsize) >=
              IOBUF_RELOCATE_THRESHOLD) {
   /* Try to free unused data. */
   assert(self->if_flag&IO_MALLBUF);
   buffer = (char *)libc_realloc(buffer,minsize);
   if unlikely(!buffer) { buffer = self->if_base; goto fill_buffer; }
   self->if_base   = buffer;
   self->if_bufsiz = minsize;
  }
 }
fill_buffer:
 assert(size);
 assert(size <= self->if_bufsiz);
 libc_memcpy(buffer,buf,size);
 self->if_ptr = buffer+size;
 assert(!self->if_cnt);
 result += size;
 libc_fmarkchanged(self);
end:
 return result;
direct_io:
 /* Read the remainder using direct I/O. */
 temp = libc_write(self->if_fd,buf,size);
 if (temp <= 0) goto err;
 result                  += temp;
 self->if_exdata->io_pos += temp;
 goto end;
err:
 self->if_flag |= IO_ERR;
 return 0;
}

INTERN __pos_t LIBCCALL
libc_fdotell(FILE *__restrict self) {
 __pos_t result = self->if_exdata->io_pos;
 if (self->if_flag&IO_R)
     result -= self->if_exdata->io_read;
 result += (size_t)(self->if_ptr-self->if_base);
 return result;
}

INTERN int LIBCCALL
libc_fdoseek(FILE *__restrict self, __off_t off, int whence) {
 __off_t new_pos;
 if ((whence == SEEK_SET || whence == SEEK_CUR) &&
      off <= (ssize_t)(((size_t)-1)/2)) {
  uintptr_t new_ptr;
  __off_t seek_offset = off;
  /* Special optimizations for seeking in-buffer. */
  if (whence == SEEK_SET)
      seek_offset = (__off_t)((__pos_t)off-libc_fdotell(self));
  if (!__builtin_add_overflow((uintptr_t)self->if_ptr,
                              (uintptr_t)seek_offset,
                              &new_ptr) &&
#if __SIZEOF_KERNEL_OFF_T__ > __SIZEOF_POINTER__
      seek_offset < (__off_t)(uintptr_t)-1 &&
#endif
      (uintptr_t)new_ptr >= (uintptr_t)self->if_base &&
      (uintptr_t)new_ptr <  (uintptr_t)self->if_ptr+self->if_cnt) {
   /* All right! - Successful seek within the currently loaded buffer. */
   self->if_ptr  = (char *)new_ptr;
   self->if_cnt += ((uintptr_t)self->if_ptr-
                    (uintptr_t)new_ptr);
   return 0;
  }
 }
 /* Flush the currently active buffer. */
 if (libc_fdoflush(self)) return -1;

 if (whence == SEEK_CUR) {
  /* Must adjust for the underlying descriptor position. */
  if (self->if_flag&IO_R)
      off -= self->if_exdata->io_read;
  off += (size_t)(self->if_ptr-self->if_base);
 }

 /* Mark the file buffer as empty. */
 self->if_exdata->io_read = 0;
 self->if_ptr   = self->if_base;
 self->if_cnt   = 0;
 self->if_flag &= ~(IO_R|IO_W);

 /* Invoke the underlying stream descriptor. */
#ifdef CONFIG_32BIT_FILESYSTEM
 new_pos = libc_lseek(self->if_fd,off,whence);
#else
 new_pos = libc_lseek64(self->if_fd,off,whence);
#endif

 /* Update the stored stream pointer. */
 if (new_pos < 0)
  self->if_flag |= IO_ERR;
 else {
  self->if_exdata->io_pos = (__pos_t)new_pos;
 }
 return 0;
}

INTERN int LIBCCALL
libc_dosetvbuf(FILE *__restrict self, char *__restrict buf, int modes, size_t n) {
 /* Start out by flushing everything. */
 if (libc_fdoflush(self)) return -1;

 /* Mark the file buffer as empty and delete special flags. */
 self->if_exdata->io_read = 0;
 self->if_ptr   = self->if_base;
 self->if_cnt   = 0;
 self->if_flag &= ~(IO_R|IO_W|IO_LNBUF|IO_LNIFTYY);

 if (modes == _IONBF || modes == __DOS_IONBF) {
  /* Don't use any buffer. */
  if (self->if_flag&IO_MALLBUF)
      libc_free(self->if_base);
  self->if_bufsiz = 0;
  self->if_ptr    = NULL;
  self->if_base   = NULL;
  return 0;
 }

 if (modes == _IOLBF || modes == __DOS_IOLBF) {
  self->if_flag |= IO_LNBUF;
  /* Passing ZERO(0) for 'n' here causes the previous buffer to be kept. */
  if (!n) return 0;
 } else if (modes != _IOFBF
#if __DOS_IOFBF != _IOFBF
         && modes != __DOS_IOFBF
#endif
            ) {
inval:
  libc_seterrno(EINVAL);
  return -1;
 }

 /* Allocate/use the given buffer. */
 if (n < 2) goto inval;
#if __SIZEOF_SIZE_T__ > 4
 if (n > (u32)-1 && n != (size_t)-1) goto inval;
#endif
 if (!buf) {
  /* Dynamically allocate a buffer. */
  if (self->if_flag&IO_MALLBUF) {
   /* (re-)allocate an existing buffer. */
   buf = self->if_base;
   /* Make sure the buffer's size has actually changed.
    * NOTE: As an extension, we accept `(size_t)-1' to keep the old buffer size. */
   if (n == (size_t)-1) n = (size_t)self->if_bufsiz;
   else if ((size_t)self->if_bufsiz != n) {
    buf = (char *)libc_realloc(buf,n);
    if unlikely(!buf) return -1;
   }
  } else {
   /* To go with the special behavior for (size_t)-1 above,
    * here that value indicates a max-length buffer as would be allocated regularly. */
   if (n == (size_t)-1)
       n = IOBUF_MAX;
   buf = (char *)libc_malloc(n);
   if unlikely(!buf) return -1;
   self->if_flag |= IO_MALLBUF;
  }
 } else {
  /* Mark the buffer as being fixed-length, thus preving it from being re-allocated. */
  self->if_flag |= IO_USERBUF;
 }

 /* Install the given buffer. */
 self->if_ptr    = buf;
 self->if_base   = buf;
 self->if_bufsiz = (u32)n;

 return 0;
}

INTERN int LIBCCALL libc_doungetc(int c, FILE *__restrict self) {
 pos_t buffer_start;
 if (self->if_ptr != self->if_base) {
  /* Simple case: we're not at the start of the buffer. */
  if (self->if_flag&IO_R &&
      self->if_ptr[-1] != (char)c)
      libc_fmarkchanged(self);
  *--self->if_ptr = (char)c;
  return c;
 }
 /* Make sure we're not going too far back. */
 buffer_start = self->if_exdata->io_pos;
 if (self->if_flag&IO_R)
     buffer_start -= self->if_exdata->io_read;
 if (!buffer_start) return EOF;

 /* This is where it gets complicated... */
 assert(self->if_ptr == self->if_base);
 assert(self->if_cnt <= self->if_bufsiz);
 if (self->if_cnt != self->if_bufsiz) {
insert_front:
  /* We can shift the entire buffer. */
  assert(self->if_exdata->io_read <= self->if_bufsiz);
  libc_memmove(self->if_base+1,self->if_base,
               self->if_exdata->io_read);
  /* Update the file to make it look like it was read
   * one byte before where it was really read at. */
  --self->if_exdata->io_pos;
  ++self->if_exdata->io_read;
  ++self->if_cnt;
 } else {
  char *new_buffer; size_t new_size;
  if (self->if_flag&IO_USERBUF) return -1;
  /* If the current buffer isn't user-given, we can simply allocate more. */
  new_size = CEIL_ALIGN(self->if_bufsiz+1,IOBUF_MIN);
#if __SIZEOF_SIZE_T__ > 4
  if unlikely(new_size > (size_t)(u32)-1)
     return -1;
#endif
realloc_again:
  new_buffer = (char *)libc_realloc(self->if_base,new_size);
  if (!new_buffer) {
   if (new_size != self->if_bufsiz+1) {
    new_size = self->if_bufsiz+1;
    goto realloc_again;
   }
   return -1;
  }
  /* Update buffer points. */
  self->if_ptr    = new_buffer;
  self->if_base   = new_buffer;
  self->if_bufsiz = new_size;
  assert(self->if_cnt < self->if_bufsiz);
  goto insert_front;
 }

 *self->if_base = (char)c;
 if (self->if_flag&IO_R)
     self->if_flag |= IO_R;
 return c;
}

INTERN void LIBCCALL libc_flushall_nostd(void) {
 FILE *iter;
 atomic_rwlock_read(&libc_ffiles_lock);
 LIST_FOREACH(iter,libc_ffiles,if_exdata->io_link) {
  file_write(iter);
  libc_fdoflush(iter);
  file_endwrite(iter);
 }
 atomic_rwlock_endread(&libc_ffiles_lock);
}

PRIVATE void LIBCCALL
libc_flushstdstream(FILE *self) {
 if (!self) return;
 file_write(self);
 libc_fdoflush(self);
 file_endwrite(self);
}

INTERN void LIBCCALL libc_flushall(void) {
 libc_flushstdstream(stdin);
 libc_flushstdstream(stdout);
 libc_flushstdstream(stderr);
 /* Finally, flush all non-standard streams. */
 libc_flushall_nostd();
}





INTERN ssize_t LIBCCALL
libc_file_printer(char const *__restrict data,
                  size_t datalen, void *closure) {
 ssize_t result;
 if unlikely(!closure) return 0;
 file_write((FILE *)closure);
 result = (ssize_t)libc_fdowrite(data,datalen*sizeof(char),(FILE *)closure);
 if unlikely(!result && FERROR((FILE *)closure)) result = -1;
 file_endwrite((FILE *)closure);
 return result;
}
INTERN size_t LIBCCALL libc_fread_unlocked(void *__restrict buf, size_t size, size_t n, FILE *__restrict self) { return libc_fdoread(buf,size*n,self)/size; }
INTERN size_t LIBCCALL libc_fwrite_unlocked(void const *__restrict buf, size_t size, size_t n, FILE *__restrict self) { return libc_fdowrite(buf,size*n,self)/size; }
INTERN size_t LIBCCALL libc_fread(void *__restrict buf, size_t size, size_t n, FILE *__restrict self) { size_t result; if (!self) return 0; file_write(self); result = libc_fdoread(buf,size*n,self)/size; file_endwrite(self); return result; }
INTERN size_t LIBCCALL libc_fwrite(void const *__restrict buf, size_t size, size_t n, FILE *__restrict self) { size_t result; if (!self) return 0; file_write(self); result = libc_fdowrite(buf,size*n,self)/size; file_endwrite(self); return result; }


INTERN off_t LIBCCALL libc_ftello(FILE *self) {
 off64_t result;
 if unlikely(!self) { libc_seterrno(EINVAL); return -1; }
 file_read(self);
 result = (off64_t)libc_fdotell(self);
 file_endread(self);
 return result;
}
INTERN off64_t LIBCCALL
libc_ftello64(FILE *self) {
 off64_t result;
 if unlikely(!self) { libc_seterrno(EINVAL); return -1; }
 file_read(self);
 result = (off64_t)libc_fdotell(self);
 file_endread(self);
 return result;
}
INTERN int LIBCCALL
libc_fseeko64(FILE *self, off64_t off, int whence) {
#if defined(__OPTIMIZE_SIZE__) && \
    defined(CONFIG_32BIT_FILESYSTEM)
 return libc_fseeko(self,(off_t)off,whence);
#else
 int result;
 if unlikely(!self) { libc_seterrno(EINVAL); return -1; }
 file_write(self);
 result = libc_fdoseek(self,(__off_t)off,whence);
 file_endwrite(self);
 return result;
#endif
}
INTERN int LIBCCALL libc_fseeko(FILE *self, off_t off, int whence) {
#if defined(__OPTIMIZE_SIZE__) && \
    !defined(CONFIG_32BIT_FILESYSTEM)
 return libc_fseeko64(self,(off64_t)off,whence);
#else
 int result;
 if unlikely(!self) { libc_seterrno(EINVAL); return -1; }
 file_write(self);
 result = libc_fdoseek(self,(__off_t)off,whence);
 file_endwrite(self);
 return result;
#endif
}

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN off_t LIBCCALL
libc_ftello_unlocked(FILE *self) {
 return self ? (off_t)libc_fdotell(self) : (libc_seterrno(EINVAL),-1);
}
INTERN off64_t LIBCCALL
libc_ftello64_unlocked(FILE *self) {
 return self ? (off64_t)libc_fdotell(self) : (libc_seterrno(EINVAL),-1);
}
INTERN int LIBCCALL
libc_fseeko64_unlocked(FILE *self, off64_t off, int whence) {
#if defined(__OPTIMIZE_SIZE__) && \
    defined(CONFIG_32BIT_FILESYSTEM)
 return libc_fseeko_unlocked(self,(off_t)off,whence);
#else
 return likely(self) ? libc_fdoseek(self,(__off_t)off,whence) : (libc_seterrno(EINVAL),-1);
#endif
}
INTERN int LIBCCALL
libc_fseeko_unlocked(FILE *self, off_t off, int whence) {
#if defined(__OPTIMIZE_SIZE__) && \
    !defined(CONFIG_32BIT_FILESYSTEM)
 return libc_fseeko64_unlocked(self,(off64_t)off,whence);
#else
 return likely(self) ? libc_fdoseek(self,(__off_t)off,whence) : (libc_seterrno(EINVAL),-1);
#endif
}
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */


/* Define the C standard seek/tell function pair. */
#if __SIZEOF_LONG__ == __FS_SIZEOF(OFF)
DEFINE_INTERN_ALIAS(libc_fseek,libc_fseeko);
DEFINE_INTERN_ALIAS(libc_ftell,libc_ftello);
#elif __SIZEOF_LONG__ == __SIZEOF_OFF64_T__
DEFINE_INTERN_ALIAS(libc_fseek,libc_fseeko64);
DEFINE_INTERN_ALIAS(libc_ftell,libc_ftello64);
#elif __SIZEOF_LONG__ > __SIZEOF_OFF64_T__
INTERN int LIBCCALL libc_fseek(FILE *self, long int off, int whence) { return libc_fseeko64(self,(off64_t)off,whence); }
INTERN long int LIBCCALL libc_ftell(FILE *self) { return (long int)libc_ftello64(self); }
#else
INTERN int LIBCCALL libc_fseek(FILE *self, long int off, int whence) { return libc_fseeko(self,(off_t)off,whence); }
INTERN long int LIBCCALL libc_ftell(FILE *self) { return (long int)libc_ftello(self); }
#endif

DEFINE_INTERN_ALIAS(libc_getc_unlocked,libc_fgetc_unlocked);
INTERN int LIBCCALL libc_fgetc_unlocked(FILE *self) {
 unsigned char result;
 return libc_fread_unlocked(&result,sizeof(result),1,self) ==
        sizeof(result) ? (int)result : EOF;
}
DEFINE_INTERN_ALIAS(libc_putc_unlocked,libc_fputc_unlocked);
INTERN int LIBCCALL libc_fputc_unlocked(int c, FILE *self) {
 unsigned char chr = (unsigned char)c;
 return libc_fwrite_unlocked(&chr,sizeof(chr),1,self) ? 0 : EOF;
}
INTERN int LIBCCALL libc_getw(FILE *self) {
 u16 result;
 return libc_fread(&result,sizeof(result),1,self) ? (int)result : EOF;
}
INTERN int LIBCCALL libc_putw(int w, FILE *self) {
 u16 c = (u16)w;
 return libc_fwrite(&c,sizeof(c),1,self) ? w : EOF;
}
DEFINE_INTERN_ALIAS(libc_getc,libc_fgetc);
INTERN int LIBCCALL libc_fgetc(FILE *self) {
 int result;
 file_write(self);
 result = libc_fgetc_unlocked(self);
 file_endwrite(self);
 return result;
}
DEFINE_INTERN_ALIAS(libc_putc,libc_fputc);
INTERN int LIBCCALL libc_fputc(int c, FILE *self) {
 int result;
 file_write(self);
 result = libc_fputc_unlocked(c,self);
 file_endwrite(self);
 return result;
}

INTERN ssize_t LIBCCALL libc_fputs(char const *__restrict s, FILE *__restrict self) {
 ssize_t result;
 file_write(self);
 result = libc_fputs_unlocked(s,self);
 file_endwrite(self);
 return result;
}
INTERN ssize_t LIBCCALL libc_fputs_unlocked(char const *__restrict s, FILE *__restrict self) {
 return libc_fwrite_unlocked(s,sizeof(char),libc_strlen(s),self);
}

INTERN int LIBCCALL libc_fflush(FILE *self) {
 int result;
 if (!self) {
#if 1
  /* STDC: 'If stream is a null pointer, all [...] streams are flushed.' */
  libc_flushall();
  return 0;
#else
  libc_seterrno(EINVAL);
  return -1;
#endif
 }
 file_write(self);
 result = libc_fdoflush(self);
 file_endwrite(self);
 return result;
}
INTERN int LIBCCALL
libc_setvbuf(FILE *__restrict self,
             char *__restrict buf,
             int modes, size_t n) {
 int result;
 if (!self) { libc_seterrno(EINVAL); return -1; }
 file_write(self);
 result = libc_dosetvbuf(self,buf,modes,n);
 file_endwrite(self);
 return result;
}
INTERN void LIBCCALL
libc_setbuf(FILE *__restrict self,
            char *__restrict buf) {
 libc_setbuffer(self,buf,BUFSIZ);
}
INTERN void LIBCCALL
libc_setbuffer(FILE *__restrict self,
               char *__restrict buf, size_t size) {
 libc_setvbuf(self,buf,
              buf ? _IOFBF : _IONBF,
              buf ? size : 0);
}
INTERN void LIBCCALL libc_setlinebuf(FILE *self) {
 libc_setvbuf(self,NULL,_IOLBF,0);
}
#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN int LIBCCALL
libc_ungetc_unlocked(int c, FILE *self) {
 return likely(self) ? libc_doungetc(c,self) : (libc_seterrno(EINVAL),-1);
}
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
INTERN int LIBCCALL libc_ungetc(int c, FILE *self) {
 int result;
 if unlikely(!self) { libc_seterrno(EINVAL); return -1; }
 file_write(self);
 result = libc_doungetc(c,self);
 file_endwrite(self);
 return result;
}

INTERN int LIBCCALL libc_fclose(FILE *self) {
 int result = 0;
 if (!self) { libc_seterrno(EINVAL); return -1; }
 libc_fdoflush(self);

 /* Remove the stream from the global list of known files. */
 atomic_rwlock_write(&libc_ffiles_lock);
 if (!LIST_ISUNBOUND(self,if_exdata->io_link))
      LIST_REMOVE(self,if_exdata->io_link);
 atomic_rwlock_endwrite(&libc_ffiles_lock);

 if (!(self->if_flag&IO_NOFD))
      libc_close(self->if_fd);

 /* Free a dynamically allocated buffer. */
 if (self->if_flag&IO_MALLBUF)
     libc_free(self->if_base);
 /* Make sure not to free one of the STD streams. */
 if (self < libc_std_files ||
     self >= COMPILER_ENDOF(libc_std_files)) {
  /* Free extended file data. */
  libc_free(self->if_exdata);
  /* Free the file buffer itself. */
  libc_free(self);
 } else {
  /* Mark STD streams as not having a handle. */
  self->if_flag |= IO_NOFD;
 }
 return result;
}

PRIVATE int LIBCCALL parse_open_modes(char const *__restrict modes) {
 int mode = O_RDONLY;
 if (modes) for (; *modes; ++modes) {
  if (*modes == 'r') mode = O_RDONLY;
  if (*modes == 'w') mode = O_WRONLY|O_CREAT|O_TRUNC;
  if (*modes == '+') mode &= ~(O_TRUNC|O_ACCMODE),mode |= O_RDWR;
 }
 return mode;
}


INTERN FILE *LIBCCALL
libc_fdopen(fd_t fd, char const *modes) {
 FILE *result = (FILE *)libc_calloc(1,sizeof(FILE));
 if (result) {
  result->if_exdata = (struct iofile_data *)libc_calloc(1,sizeof(struct iofile_data));
  if unlikely(!result->if_exdata) { libc_free(result); return NULL; }
  result->if_fd = fd;
  /* Check if the stream handle is a tty the first time it is read from. */
  result->if_flag = IO_LNIFTYY;
  /* Insert the new file stream into the global list of them. */
  atomic_rwlock_write(&libc_ffiles_lock);
  LIST_INSERT(libc_ffiles,result,if_exdata->io_link);
  atomic_rwlock_endwrite(&libc_ffiles_lock);
 }
 return result;
}
INTERN FILE *LIBCCALL
libc_fdreopen(fd_t fd, char const *__restrict modes,
              FILE *__restrict self, int mode) {
 if unlikely(!self) { libc_seterrno(EINVAL); goto err2; }
 file_write(self);
 /* Flush the old descriptor one last time. */
 if (libc_fdoflush(self)) goto err;
 /* Duplicate the new descriptor to override the old. */
 if (self->if_flag&IO_NOFD) {
  if (!(mode&FDREOPEN_INHERIT) &&
       (fd = libc_dup(fd)) < 0)
        goto err;
  self->if_fd = fd;
 } else if (mode&FDREOPEN_INHERIT) {
  /* Close the old handle. */
  libc_close(self->if_fd); /* Ignore errors. */
  /* Inherit the new fd. */
  self->if_fd = fd;
 } else {
  if (libc_dup2(fd,self->if_fd))
      goto err;
 }
 /* Delete the nofd flag, because now we definitely have one. */
 self->if_flag &= ~IO_NOFD;
 file_endwrite(self);
 return self;
err:
 file_endwrite(self);
err2:
 /* Close the new descriptor on error. */
 if (mode&FDREOPEN_CLOSE_ON_ERROR)
     sys_close(fd);
 return NULL;
}


INTERN FILE *LIBCCALL libc_fmemopen(void *s, size_t len, char const *modes) { libc_seterrno(ENOSYS); return NULL; }
INTERN FILE *LIBCCALL libc_open_memstream(char **bufloc, size_t *sizeloc) { libc_seterrno(ENOSYS); return NULL; }
INTERN FILE *LIBCCALL libc_popen(char const *command, char const *modes) { libc_seterrno(ENOSYS); return NULL; }
INTERN int LIBCCALL libc_pclose(FILE *self) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_fcloseall(void) { libc_seterrno(ENOSYS); return -1; }

DEFINE_INTERN_ALIAS(libc_tmpfile64,libc_tmpfile);
INTERN FILE *LIBCCALL libc_tmpfile(void) { libc_seterrno(ENOSYS); return NULL; }
DEFINE_INTERN_ALIAS(libc_fopen64,libc_fopen);
INTERN FILE *LIBCCALL libc_fopen(char const *__restrict filename, char const *__restrict modes) {
 fd_t fd; FILE *result;
 libc_syslog(LOG_DEBUG,"LIBC: fopen(%q,%q)\n",filename,modes);
#if 1
 /* Temporary hack to pipe curses trace logging into the system log. */
 if (!libc_strcmp(filename,"//trace")) {
  fd = libc_open("/dev/kmsg",O_WRONLY);
 } else
#endif
 {
  fd = libc_open(filename,parse_open_modes(modes),0644);
 }
 if (fd < 0) return NULL;
 result = libc_fdopen(fd,modes);
 if (!result) sys_close(fd);
 return result;
}

DEFINE_INTERN_ALIAS(libc_freopen64,libc_freopen);
INTERN FILE *LIBCCALL
libc_freopen(char const *__restrict filename,
             char const *__restrict modes,
             FILE *__restrict self) {
 fd_t fd = libc_open(filename,parse_open_modes(modes),0644);
 return libc_fdreopen(fd,modes,self,FDREOPEN_INHERIT|FDREOPEN_CLOSE_ON_ERROR);
}
INTERN int LIBCCALL libc_fflush_unlocked(FILE *self) {
 if (!self) { libc_seterrno(EINVAL); return -1; }
 return libc_fdoflush(self);
}
INTERN int LIBCCALL libc_feof_unlocked(FILE *self) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_ferror_unlocked(FILE *self) { libc_seterrno(ENOSYS); return 0; }
INTERN ssize_t LIBCCALL libc_getdelim(char **__restrict lineptr, size_t *__restrict n, int delimiter, FILE *__restrict self) { libc_seterrno(ENOSYS); return -1; }
INTERN ssize_t LIBCCALL libc_getline(char **__restrict lineptr, size_t *__restrict n, FILE *__restrict self) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_feof(FILE *self) { return self && FEOF(self); }
INTERN int LIBCCALL libc_ferror(FILE *self) { return self && FERROR(self); }
INTERN void LIBCCALL libc_clearerr(FILE *self) { if (self) ATOMIC_FETCHAND(self->if_flag,~IO_ERR); }
INTERN void LIBCCALL libc_clearerr_unlocked(FILE *self) { if (self) FCLEARERR(self); }
//INTERN FILE *LIBCCALL libc_fopencookie(void *__restrict magic_cookie, char const *__restrict modes, _IO_cookie_io_functions_t io_funcs);

INTERN char *LIBCCALL
libc_fgets_unlocked(char *__restrict s, size_t n,
                    FILE *__restrict self) {
 libc_seterrno(ENOSYS);
 return NULL;
}
INTERN char *LIBCCALL
libc_fgets(char *__restrict s, size_t n, FILE *__restrict self) {
 char *result;
 file_write(self);
 result = libc_fgets_unlocked(s,n,self);
 file_endwrite(self);
 return result;
}
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
DEFINE_INTERN_ALIAS(libc_fgets_int,libc_fgets);
DEFINE_INTERN_ALIAS(libc_fgets_unlocked_int,libc_fgets_unlocked);
#else
INTERN char *LIBCCALL libc_fgets_int(char *__restrict s, int n, FILE *__restrict self) { return libc_fgets(s,(size_t)n,self); }
INTERN char *LIBCCALL libc_fgets_unlocked_int(char *__restrict s, int n, FILE *__restrict self) { return libc_fgets_unlocked(s,(size_t)n,self); }
#endif


/* Doesn't really matter. - Use an atomic_read for both! */
DEFINE_INTERN_ALIAS(libc_fileno_unlocked,libc_fileno);
INTERN void LIBCCALL libc_flockfile(FILE *self) { file_write(self); }
INTERN int LIBCCALL libc_ftrylockfile(FILE *self) { return file_trywrite(self); }
INTERN void LIBCCALL libc_funlockfile(FILE *self) { file_endwrite(self); }
INTERN int LIBCCALL libc_fgetpos(FILE *__restrict self, fpos_t *__restrict pos) { return (int)(*pos = (fpos_t)libc_ftello(self)); }
INTERN int LIBCCALL libc_fsetpos(FILE *self, fpos_t const *pos) { return libc_fseeko(self,*pos,SEEK_SET); }
INTERN int LIBCCALL libc_fgetpos64(FILE *__restrict self, fpos64_t *__restrict pos) { return (int)(*pos = (fpos64_t)libc_ftello64(self)); }
INTERN int LIBCCALL libc_fsetpos64(FILE *self, fpos64_t const *pos) { return libc_fseeko64(self,(off64_t)*pos,SEEK_SET); }
INTERN int LIBCCALL libc_getchar(void) { return libc_fgetc(stdin); }
INTERN int LIBCCALL libc_putchar(int c) { return libc_fputc(c,stdout); }
INTERN int LIBCCALL libc_getchar_unlocked(void) { return libc_fgetc_unlocked(stdin); }
INTERN int LIBCCALL libc_putchar_unlocked(int c) { return libc_fputc_unlocked(c,stdout); }
INTERN void LIBCCALL libc_rewind(FILE *self) { libc_fseeko64(self,0,SEEK_SET); }
INTERN int LIBCCALL libc_fileno(FILE *self) { return ATOMIC_READ(self->if_fd); }
INTERN char *LIBCCALL libc_gets(char *s) { return libc_fgets(s,(size_t)-1,stdin); }
INTERN ssize_t LIBCCALL libc_puts(char const *s) {
 ssize_t result;
 file_write(stdout);
 result = libc_fputs_unlocked(s,stdout);
 if (result >= 0)
     result += libc_fwrite_unlocked("\n",sizeof(char),1,stdout);
 file_endwrite(stdout);
 return result;
}
INTERN int LIBCCALL libc_fwide(FILE *file, int mode) { libc_seterrno(ENOSYS); return -1; }


EXPORT(fread,libc_fread);
EXPORT(fwrite,libc_fwrite);
EXPORT(fread_unlocked,libc_fread_unlocked);
EXPORT(fwrite_unlocked,libc_fwrite_unlocked);
EXPORT(fseek,libc_fseek);
EXPORT(ftell,libc_ftell);
EXPORT(fseeko,libc_fseeko);
EXPORT(ftello,libc_ftello);
EXPORT(fseeko64,libc_fseeko64);
EXPORT(ftello64,libc_ftello64);
EXPORT(fgetpos,libc_fgetpos);
EXPORT(fsetpos,libc_fsetpos);
EXPORT(fgetpos64,libc_fgetpos64);
EXPORT(fsetpos64,libc_fsetpos64);
EXPORT(flockfile,libc_flockfile);
EXPORT(ftrylockfile,libc_ftrylockfile);
EXPORT(funlockfile,libc_funlockfile);
EXPORT(file_printer,libc_file_printer);
EXPORT(fgetc_unlocked,libc_fgetc_unlocked);
EXPORT(fputc_unlocked,libc_fputc_unlocked);
EXPORT(getw,libc_getw);
EXPORT(putw,libc_putw);
EXPORT(fgetc,libc_fgetc);
EXPORT(fputc,libc_fputc);
EXPORT(fputs,libc_fputs);
EXPORT(fputs_unlocked,libc_fputs_unlocked);
EXPORT(clearerr,libc_clearerr);
EXPORT(fclose,libc_fclose);
EXPORT(fflush,libc_fflush);
EXPORT(setbuf,libc_setbuf);
EXPORT(setvbuf,libc_setvbuf);
EXPORT(ungetc,libc_ungetc);
EXPORT(tmpfile64,libc_tmpfile64);
EXPORT(tmpfile,libc_tmpfile);
EXPORT(fopen,libc_fopen);
EXPORT(fopen64,libc_fopen64);
EXPORT(freopen,libc_freopen);
EXPORT(freopen64,libc_freopen64);
EXPORT(fflush_unlocked,libc_fflush_unlocked);
EXPORT(setbuffer,libc_setbuffer);
EXPORT(setlinebuf,libc_setlinebuf);
EXPORT(feof_unlocked,libc_feof_unlocked);
EXPORT(ferror_unlocked,libc_ferror_unlocked);
EXPORT(fdopen,libc_fdopen);
EXPORT(fdreopen,libc_fdreopen);
EXPORT(fmemopen,libc_fmemopen);
EXPORT(open_memstream,libc_open_memstream);
EXPORT(getdelim,libc_getdelim);
EXPORT(getline,libc_getline);
EXPORT(popen,libc_popen);
EXPORT(pclose,libc_pclose);
EXPORT(fcloseall,libc_fcloseall);
EXPORT(clearerr_unlocked,libc_clearerr_unlocked);
EXPORT(feof,libc_feof);
EXPORT(ferror,libc_ferror);
//EXPORT(fopencookie,libc_fopencookie);
#if __SIZEOF_SIZE_T__ == __SIZEOF_SIZE_T__
EXPORT(fgets,libc_fgets);
EXPORT(fgets_unlocked,libc_fgets_unlocked);
#else
EXPORT(fgets,libc_fgets_int);
EXPORT(fgets_sz,libc_fgets);
EXPORT(fgets_unlocked,libc_fgets_unlocked_int);
EXPORT(fgets_unlocked_sz,libc_fgets_unlocked);
#endif
EXPORT(getchar,libc_getchar);
EXPORT(putchar,libc_putchar);
EXPORT(getchar_unlocked,libc_getchar_unlocked);
EXPORT(putchar_unlocked,libc_putchar_unlocked);
EXPORT(rewind,libc_rewind);
EXPORT(fileno,libc_fileno);
EXPORT(gets,libc_gets);
EXPORT(puts,libc_puts);
EXPORT(fileno_unlocked,libc_fileno_unlocked);
EXPORT(getc,libc_getc);
EXPORT(putc,libc_putc);
EXPORT(getc_unlocked,libc_getc_unlocked);
EXPORT(putc_unlocked,libc_putc_unlocked);
EXPORT(__getdelim,libc_getdelim);
EXPORT(fwide,libc_fwide);


/* Wide-string API */
INTERN FILE *LIBCCALL libc_32open_wmemstream(char32_t **bufloc, size_t *sizeloc) { libc_seterrno(ENOSYS); return NULL; }
INTERN wint_t LIBCCALL libc_32getwchar(void) { return libc_32getwc(stdin); }
INTERN wint_t LIBCCALL libc_32putwchar(char32_t wc) { return libc_32putwc(wc,stdout); }
INTERN wint_t LIBCCALL libc_32getwchar_unlocked(void) { return libc_32getwc_unlocked(stdin); }
INTERN wint_t LIBCCALL libc_32putwchar_unlocked(char32_t wc) { return libc_32putwc_unlocked(wc,stdout); }
DEFINE_INTERN_ALIAS(libc_32getwc,libc_32fgetwc);
DEFINE_INTERN_ALIAS(libc_32putwc,libc_32fputwc);
DEFINE_INTERN_ALIAS(libc_32getwc_unlocked,libc_32fgetwc_unlocked);
DEFINE_INTERN_ALIAS(libc_32putwc_unlocked,libc_32fputwc_unlocked);
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
DEFINE_INTERN_ALIAS(libc_32fgetws_int,libc_32fgetws);
DEFINE_INTERN_ALIAS(libc_32fgetws_unlocked_int,libc_32fgetws_unlocked);
#else
INTERN char32_t *LIBCCALL libc_32fgetws_int(char32_t *__restrict buf, int n, FILE *__restrict self) { return libc_32fgetws(buf,(size_t)n,self); }
INTERN char32_t *LIBCCALL libc_32fgetws_unlocked_int(char32_t *__restrict buf, int n, FILE *__restrict self) { return libc_32fgetws_unlocked_int(buf,(size_t)n,self); }
#endif
INTERN wint_t LIBCCALL libc_32fgetwc(FILE *self) { wint_t result; file_write(self); result = libc_32fgetwc_unlocked(self); file_endwrite(self); return result; }
INTERN wint_t LIBCCALL libc_32fputwc(char32_t wc, FILE *self) { wint_t result; file_write(self); result = libc_32fputwc_unlocked(wc,self); file_endwrite(self); return result; }
INTERN char32_t *LIBCCALL libc_32fgetws(char32_t *__restrict buf, size_t n, FILE *__restrict self) { char32_t *result; file_write(self); result = libc_32fgetws_unlocked(buf,n,self); file_endwrite(self); return result; }
INTERN int LIBCCALL libc_32fputws(char32_t const *__restrict str, FILE *__restrict self) { int result; file_write(self); result = libc_32fputws_unlocked(str,self); file_endwrite(self); return result; }
#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN wint_t LIBCCALL libc_32ungetwc(wint_t wc, FILE *self) { wint_t result; file_write(self); result = libc_32ungetwc_unlocked(wc,self); file_endwrite(self); return result; }
INTERN wint_t LIBCCALL libc_32ungetwc_unlocked(wint_t wc, FILE *self) { /* TODO? */ return (wint_t)libc_ungetc_unlocked((int)wc,self); }
#else /* !CONFIG_LIBC_NO_DOS_LIBC */
INTERN wint_t LIBCCALL libc_32ungetwc(wint_t wc, FILE *self) { wint_t result; file_write(self); /* TODO? */ result = (wint_t)libc_doungetc((int)wc,self); file_endwrite(self); return result; }
#endif /* CONFIG_LIBC_NO_DOS_LIBC */
INTERN wint_t LIBCCALL libc_32fgetwc_unlocked(FILE *self) { /* TODO? */ return (wint_t)libc_fgetc_unlocked(self); }
INTERN wint_t LIBCCALL libc_32fputwc_unlocked(char32_t wc, FILE *self) { /* TODO? */ return (wint_t)libc_fputc_unlocked((int)wc,self); }
INTERN char32_t *LIBCCALL libc_32fgetws_unlocked(char32_t *__restrict buf, size_t n, FILE *__restrict self) { libc_seterrno(ENOSYS); return 0; }
INTERN int LIBCCALL libc_32fputws_unlocked(char32_t const *__restrict buf, FILE *__restrict self) {
 int result = -1; char *utf8_str = libc_utf32to8m(buf);
 if likely(utf8_str) result = libc_fputs_unlocked(utf8_str,self),libc_free(utf8_str);
 return result;
}



/* DOS Extensions. */
#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN FILE *LIBCCALL libc_p_iob(void) { return libc_std_files; }
INTERN FILE *LIBCCALL libc_acrt_iob_func(unsigned int id) { return &libc_std_files[id]; }
EXPORT(_iob,libc_std_files);
EXPORT(__p__iob,libc_p_iob);
EXPORT(__iob_func,libc_p_iob);
EXPORT(__acrt_iob_func,libc_acrt_iob_func);

DEFINE_INTERN_ALIAS(libc_dos_fopen64,libc_dos_fopen);
INTERN FILE *LIBCCALL
libc_dos_fopen(char const *__restrict filename,
               char const *__restrict modes) {
 fd_t fd; FILE *result;
 fd = libc_open(filename,O_DOSPATH|parse_open_modes(modes),0644);
 if (fd < 0) return NULL;
 result = libc_fdopen(fd,modes);
 if (!result) sys_close(fd);
 return result;
}

DEFINE_INTERN_ALIAS(libc_dos_freopen64,libc_dos_freopen);
INTERN FILE *LIBCCALL
libc_dos_freopen(char const *__restrict filename,
                 char const *__restrict modes,
                 FILE *__restrict self) {
 fd_t fd = libc_open(filename,O_DOSPATH|parse_open_modes(modes),0644);
 return libc_fdreopen(fd,modes,self,FDREOPEN_INHERIT|FDREOPEN_CLOSE_ON_ERROR);
}

/* DOS (16-bit) versions of wide-string functions. */
INTERN FILE *LIBCCALL libc_16open_wmemstream(char16_t **bufloc, size_t *sizeloc) { libc_seterrno(ENOSYS); return NULL; }
INTERN wint_t LIBCCALL libc_16getwchar(void) { return libc_16getwc(stdin); }
INTERN wint_t LIBCCALL libc_16putwchar(char16_t wc) { return libc_16putwc(wc,stdout); }
INTERN wint_t LIBCCALL libc_16getwchar_unlocked(void) { return libc_16getwc_unlocked(stdin); }
INTERN wint_t LIBCCALL libc_16putwchar_unlocked(char16_t wc) { return libc_16putwc_unlocked(wc,stdout); }
DEFINE_INTERN_ALIAS(libc_16getwc,libc_16fgetwc);
DEFINE_INTERN_ALIAS(libc_16putwc,libc_16fputwc);
DEFINE_INTERN_ALIAS(libc_16getwc_unlocked,libc_16fgetwc_unlocked);
DEFINE_INTERN_ALIAS(libc_16putwc_unlocked,libc_16fputwc_unlocked);
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
DEFINE_INTERN_ALIAS(libc_16fgetws_int,libc_16fgetws);
DEFINE_INTERN_ALIAS(libc_16fgetws_unlocked_int,libc_16fgetws_unlocked);
#else
INTERN char16_t *LIBCCALL libc_16fgetws_int(char16_t *__restrict buf, int n, FILE *__restrict self) { return libc_16fgetws(buf,(size_t)n,self); }
INTERN char16_t *LIBCCALL libc_16fgetws_unlocked_int(char16_t *__restrict buf, int n, FILE *__restrict self) { return libc_16fgetws_unlocked_int(buf,(size_t)n,self); }
#endif
INTERN wint_t LIBCCALL libc_16fgetwc(FILE *self) { wint_t result; file_write(self); result = libc_16fgetwc_unlocked(self); file_endwrite(self); return result; }
INTERN wint_t LIBCCALL libc_16fputwc(char16_t wc, FILE *self) { wint_t result; file_write(self); result = libc_16fputwc_unlocked(wc,self); file_endwrite(self); return result; }
INTERN char16_t *LIBCCALL libc_16fgetws(char16_t *__restrict buf, size_t n, FILE *__restrict self) { char16_t *result; file_write(self); result = libc_16fgetws_unlocked(buf,n,self); file_endwrite(self); return result; }
INTERN int LIBCCALL libc_16fputws(char16_t const *__restrict str, FILE *__restrict self) { int result; file_write(self); result = libc_16fputws_unlocked(str,self); file_endwrite(self); return result; }
INTERN wint_t LIBCCALL libc_16ungetwc(wint_t wc, FILE *self) { wint_t result; file_write(self); result = libc_16ungetwc_unlocked(wc,self); file_endwrite(self); return result; }
INTERN wint_t LIBCCALL libc_16ungetwc_unlocked(wint_t wc, FILE *self) { /* TODO? */ return (wint_t)libc_ungetc_unlocked((int)wc,self); }
INTERN wint_t LIBCCALL libc_16fgetwc_unlocked(FILE *self) { /* TODO? */ return (wint_t)libc_fgetc_unlocked(self); }
INTERN wint_t LIBCCALL libc_16fputwc_unlocked(char16_t wc, FILE *self) { /* TODO? */ return (wint_t)libc_fputc_unlocked((int)wc,self); }
INTERN char16_t *LIBCCALL libc_16fgetws_unlocked(char16_t *__restrict buf, size_t n, FILE *__restrict self) { libc_seterrno(ENOSYS); return 0; }
INTERN int LIBCCALL libc_16fputws_unlocked(char16_t const *__restrict buf, FILE *__restrict self) {
 int result = -1; char *utf8_str = libc_utf16to8m(buf);
 if likely(utf8_str) result = libc_fputs_unlocked(utf8_str,self),libc_free(utf8_str);
 return result;
}



EXPORT(__SYMw32(getwchar),libc_32getwchar);
EXPORT(__SYMw32(fgetwc),libc_32fgetwc);
EXPORT(__SYMw32(getwc),libc_32getwc);
EXPORT(__SYMw32(putwchar),libc_32putwchar);
EXPORT(__SYMw32(fputwc),libc_32fputwc);
EXPORT(__SYMw32(putwc),libc_32putwc);
#if __SIZEOF_SIZE_T__ == __SIZEOF_SIZE_T__
EXPORT(__SYMw32(fgetws),libc_32fgetws);
EXPORT(__SYMw32(fgetws_unlocked),libc_32fgetws_unlocked);
#else
EXPORT(__SYMw32(fgetws),libc_32fgetws_int);
EXPORT(__SYMw32(fgetws_sz),libc_32fgetws);
EXPORT(__SYMw32(fgetws_unlocked),libc_32fgetws_unlocked_int);
EXPORT(__SYMw32(fgetws_unlocked_sz),libc_32fgetws_unlocked);
#endif
EXPORT(__SYMw32(fputws),libc_32fputws);
EXPORT(__SYMw32(ungetwc),libc_32ungetwc);
EXPORT(__SYMw32(open_wmemstream),libc_32open_wmemstream);
EXPORT(__SYMw32(getwc_unlocked),libc_32getwc_unlocked);
EXPORT(__SYMw32(getwchar_unlocked),libc_32getwchar_unlocked);
EXPORT(__SYMw32(fgetwc_unlocked),libc_32fgetwc_unlocked);
EXPORT(__SYMw32(fputwc_unlocked),libc_32fputwc_unlocked);
EXPORT(__SYMw32(putwc_unlocked),libc_32putwc_unlocked);
EXPORT(__SYMw32(putwchar_unlocked),libc_32putwchar_unlocked);
EXPORT(__SYMw32(fputws_unlocked),libc_32fputws_unlocked);

EXPORT(__SYMw16(getwchar),libc_16getwchar);
EXPORT(__SYMw16(fgetwc),libc_16fgetwc);
EXPORT(__SYMw16(getwc),libc_16getwc);
EXPORT(__SYMw16(putwchar),libc_16putwchar);
EXPORT(__SYMw16(fputwc),libc_16fputwc);
EXPORT(__SYMw16(putwc),libc_16putwc);
#if __SIZEOF_SIZE_T__ == __SIZEOF_SIZE_T__
EXPORT(__SYMw16(fgetws),libc_16fgetws);
EXPORT(_fgetws_nolock,libc_16fgetws_unlocked);
#else
EXPORT(__SYMw16(fgetws),libc_16fgetws_int);
EXPORT(__SYMw16(fgetws_sz),libc_16fgetws);
EXPORT(_fgetws_nolock,libc_16fgetws_unlocked_int);
EXPORT(_fgetws_nolock_sz,libc_16fgetws_unlocked);
#endif
EXPORT(__SYMw16(fputws),libc_16fputws);
EXPORT(__SYMw16(ungetwc),libc_16ungetwc);
EXPORT(__SYMw16(open_wmemstream),libc_16open_wmemstream);
EXPORT(_getwchar_nolock,libc_16getwchar_unlocked);
EXPORT(_putwchar_nolock,libc_16putwchar_unlocked);
EXPORT(_fgetwc_nolock,libc_16fgetwc_unlocked);
EXPORT(_fputwc_nolock,libc_16fputwc_unlocked);
EXPORT(_fputws_nolock,libc_16fputws_unlocked);

/* DOS function aliases. */
EXPORT(_popen,libc_popen);
EXPORT(_pclose,libc_pclose);
EXPORT(_fcloseall,libc_fcloseall);
EXPORT(_fileno,libc_fileno);
EXPORT(_fgetchar,libc_getchar);
EXPORT(_fputchar,libc_putchar);
EXPORT(_getw,libc_getw);
EXPORT(_putw,libc_putw);
EXPORT(_fseeki64,libc_fseeko64);
EXPORT(_ftelli64,libc_ftello64);
EXPORT(_fdopen,libc_fdopen);
EXPORT(_lock_file,libc_flockfile);
EXPORT(_unlock_file,libc_funlockfile);
EXPORT(_fflush_nolock,libc_fflush_unlocked);
EXPORT(_fread_nolock,libc_fread_unlocked);
EXPORT(_fwrite_nolock,libc_fwrite_unlocked);
EXPORT(_fclose_nolock,libc_fclose); /* No unlocked version for this! */
EXPORT(fclose_unlocked,libc_fclose); /* No unlocked version for this! */

/* DOS file functions not defined by UNIX. */
EXPORT(_ungetc_nolock,libc_ungetc_unlocked);
EXPORT(_fseek_nolock,libc_fseeko_unlocked);
EXPORT(_ftell_nolock,libc_ftello_unlocked);
EXPORT(_fseeki64_nolock,libc_fseeko64_unlocked);
EXPORT(_ftelli64_nolock,libc_ftello64_unlocked);

/* DOS file functions. */
EXPORT(_flushall,libc_flushall);
EXPORT(_fsopen,libc_dos_fsopen);
EXPORT(_filbuf,libc_filbuf);
EXPORT(_flsbuf,libc_flsbuf);
EXPORT(_getmaxstdio,libc_getmaxstdio);
EXPORT(_setmaxstdio,libc_setmaxstdio);
EXPORT(_get_printf_count_output,libc_get_printf_count_output);
EXPORT(_set_printf_count_output,libc_set_printf_count_output);
EXPORT(_get_output_format,libc_get_output_format);
EXPORT(_set_output_format,libc_set_output_format);
EXPORT(_rmtmp,libc_rmtmp);
EXPORT(fread_s,libc_fread_s);
EXPORT(_fread_nolock_s,libc_fread_unlocked_s);

#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_dos_fsopen,libc_dos_fopen);
#else
INTERN FILE *LIBCCALL
libc_dos_fsopen(char const *filename, char const *mode, int UNUSED(shflag)) {
 return libc_dos_fopen(filename,modes);
}
#endif

INTERN int LIBCCALL libc_rmtmp(void) { libc_seterrno(ENOSYS); return -1; }
INTERN size_t LIBCCALL
libc_fread_s(void *__restrict buf, size_t bufsize,
             size_t elemsize, size_t elemcount,
             FILE *__restrict self) {
 if (bufsize < elemsize*elemcount) { libc_seterrno(ERANGE); return 0; }
 return libc_fread(buf,elemsize,elemcount,self);
}
INTERN size_t LIBCCALL
libc_fread_unlocked_s(void *__restrict buf, size_t bufsize,
                      size_t elemsize, size_t elemcount,
                      FILE *__restrict self) {
 if (bufsize < elemsize*elemcount) { libc_seterrno(ERANGE); return 0; }
 return libc_fread_unlocked(buf,elemsize,elemcount,self);
}

/* TODO: The following two, we should really implement soon (They're what's driving inline 'fgetc()' / 'fputc()') */
INTERN int LIBCCALL libc_filbuf(FILE *__restrict self) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_flsbuf(int ch, FILE *__restrict self) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getmaxstdio(void) { return INT_MAX; /* Unlimited */ }
INTERN int LIBCCALL libc_setmaxstdio(int UNUSED(val)) { return INT_MAX; /* ... */ }
INTERN int LIBCCALL libc_get_printf_count_output(void) { return 1; }
INTERN int LIBCCALL libc_set_printf_count_output(int UNUSED(val)) { return 1; }
INTERN u32 LIBCCALL libc_get_output_format(void) { return 0; /* ??? */ }
INTERN u32 LIBCCALL libc_set_output_format(u32 format) { return 0; /* ??? */ }


INTERN errno_t LIBCCALL
libc_fopen_s(FILE **pfile, char const *filename, char const *mode) {
 if unlikely(!pfile) return EINVAL;
 return (*pfile = libc_fopen(filename,mode)) != NULL ? EOK : libc_geterrno();
}
INTERN errno_t LIBCCALL
libc_dos_fopen_s(FILE **pfile, char const *filename, char const *mode) {
 if unlikely(!pfile) return __DOS_EINVAL;
 return (*pfile = libc_dos_fopen(filename,mode)) != NULL ? EOK : libc_dos_geterrno();
}
INTERN errno_t LIBCCALL
libc_freopen_s(FILE **pfile, char const *filename,
               char const *mode, FILE *oldfile) {
 if unlikely(!pfile) return EINVAL;
 return (*pfile = libc_freopen(filename,mode,oldfile)) != NULL ? EOK : libc_geterrno();
}
INTERN errno_t LIBCCALL
libc_dos_freopen_s(FILE **pfile, char const *filename,
                   char const *mode, FILE *oldfile) {
 if unlikely(!pfile) return __DOS_EINVAL;
 return (*pfile = libc_dos_freopen(filename,mode,oldfile)) != NULL ? EOK : libc_dos_geterrno();
}
INTERN errno_t LIBCCALL
libc_clearerr_s(FILE *__restrict self) {
 libc_clearerr(self);
 return EOK;
}
INTERN char *LIBCCALL
libc_gets_s(char *__restrict buf, size_t bufsize) {
 return libc_fgets(buf,bufsize,stdin);
}
INTERN errno_t LIBCCALL libc_tmpfile_s(FILE **pfile) {
 if (!pfile) return EINVAL;
 return (*pfile = libc_tmpfile()) != NULL ? EOK : libc_geterrno();
}
INTERN errno_t LIBCCALL libc_dos_tmpfile_s(FILE **pfile) {
 return libc_errno_kos2dos(libc_tmpfile_s(pfile));
}

EXPORT(__KSYM(fopen_s),libc_fopen_s);
EXPORT(__DSYM(fopen_s),libc_dos_fopen_s);
EXPORT(__KSYM(freopen_s),libc_freopen_s);
EXPORT(__DSYM(freopen_s),libc_dos_freopen_s);
EXPORT(__KSYM(tmpfile_s),libc_tmpfile_s);
EXPORT(__DSYM(tmpfile_s),libc_dos_tmpfile_s);
EXPORT(clearerr_s,libc_clearerr_s);
EXPORT(gets_s,libc_gets_s);

/* DOS Widechar extension API. */
#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_w16fsopen,libc_w16fopen);
DEFINE_INTERN_ALIAS(libc_w32fsopen,libc_w32fopen);
DEFINE_INTERN_ALIAS(libc_dos_w16fsopen,libc_dos_w16fopen);
DEFINE_INTERN_ALIAS(libc_dos_w32fsopen,libc_dos_w32fopen);
#else
INTERN FILE *LIBCCALL libc_w16fsopen(char16_t const *filename, char16_t const *mode, int UNUSED(shflag)) { return libc_w16fopen(filename,mode); }
INTERN FILE *LIBCCALL libc_w32fsopen(char32_t const *filename, char32_t const *mode, int UNUSED(shflag)) { return libc_w32fopen(filename,mode); }
INTERN FILE *LIBCCALL libc_dos_w16fsopen(char16_t const *filename, char16_t const *mode, int UNUSED(shflag)) { return libc_dos_w16fopen(filename,mode); }
INTERN FILE *LIBCCALL libc_dos_w32fsopen(char32_t const *filename, char32_t const *mode, int UNUSED(shflag)) { return libc_dos_w32fopen(filename,mode); }
#endif
PRIVATE FILE *LIBCCALL
libc_dofreopen(char const *__restrict filename,
               char const *__restrict mode,
               FILE *oldfile_or_null, int ext_oflags) {
 FILE *result; fd_t fd;
 fd = libc_open(filename,ext_oflags|parse_open_modes(mode),0644);
 if unlikely(fd < 0) return NULL;
 if (oldfile_or_null) {
  result = libc_fdreopen(fd,mode,oldfile_or_null,
                         FDREOPEN_INHERIT|
                         FDREOPEN_CLOSE_ON_ERROR);
 } else {
  result = libc_fdopen(fd,mode);
  if unlikely(!result) sys_close(fd);
 }
 return result;
}


INTERN FILE *LIBCCALL
libc_w16freopen_ex(char16_t const *filename, char16_t const *mode,
                   FILE *oldfile_or_null, int ext_oflags) {
 char *utf8_filename,*utf8_mode; FILE *result = NULL;
 utf8_filename = libc_utf16to8m(filename);
 if (!utf8_filename) goto end;
 utf8_mode = libc_utf16to8m(mode);
 if (!utf8_mode) goto end2;
 result = libc_dofreopen(utf8_filename,utf8_mode,
                         oldfile_or_null,ext_oflags);
 libc_free(utf8_mode);
end2: libc_free(utf8_filename);
end:  return result;
}
INTERN FILE *LIBCCALL
libc_w32freopen_ex(char32_t const *filename, char32_t const *mode,
                   FILE *oldfile_or_null, int ext_oflags) {
 char *utf8_filename,*utf8_mode; FILE *result = NULL;
 utf8_filename = libc_utf32to8m(filename);
 if (!utf8_filename) goto end;
 utf8_mode = libc_utf32to8m(mode);
 if (!utf8_mode) goto end2;
 result = libc_dofreopen(utf8_filename,utf8_mode,
                         oldfile_or_null,ext_oflags);
 libc_free(utf8_mode);
end2: libc_free(utf8_filename);
end:  return result;
}
INTERN FILE *LIBCCALL libc_w16fopen(char16_t const *filename, char16_t const *mode) { return libc_w16freopen_ex(filename,mode,NULL,0); }
INTERN FILE *LIBCCALL libc_w32fopen(char32_t const *filename, char32_t const *mode) { return libc_w32freopen_ex(filename,mode,NULL,0); }
INTERN FILE *LIBCCALL libc_w16freopen(char16_t const *filename, char16_t const *mode, FILE *oldfile) { return oldfile ? libc_w16freopen_ex(filename,mode,oldfile,0) : (libc_seterrno(EINVAL),NULL); }
INTERN FILE *LIBCCALL libc_w32freopen(char32_t const *filename, char32_t const *mode, FILE *oldfile) { return oldfile ? libc_w32freopen_ex(filename,mode,oldfile,0) : (libc_seterrno(EINVAL),NULL); }
INTERN FILE *LIBCCALL libc_dos_w16fopen(char16_t const *filename, char16_t const *mode) { return libc_w16freopen_ex(filename,mode,NULL,O_DOSPATH); }
INTERN FILE *LIBCCALL libc_dos_w32fopen(char32_t const *filename, char32_t const *mode) { return libc_w32freopen_ex(filename,mode,NULL,O_DOSPATH); }
INTERN FILE *LIBCCALL libc_dos_w16freopen(char16_t const *filename, char16_t const *mode, FILE *oldfile) { return oldfile ? libc_w16freopen_ex(filename,mode,oldfile,O_DOSPATH) : (libc_seterrno(EINVAL),NULL); }
INTERN FILE *LIBCCALL libc_dos_w32freopen(char32_t const *filename, char32_t const *mode, FILE *oldfile) { return oldfile ? libc_w32freopen_ex(filename,mode,oldfile,O_DOSPATH) : (libc_seterrno(EINVAL),NULL); }
INTERN errno_t LIBCCALL libc_w16fopen_s(FILE **pfile, char16_t const *filename, char16_t const *mode) { if unlikely(!pfile) return EINVAL; return (*pfile = libc_w16fopen(filename,mode)) != NULL ? EOK : libc_geterrno(); }
INTERN errno_t LIBCCALL libc_w32fopen_s(FILE **pfile, char32_t const *filename, char32_t const *mode) { if unlikely(!pfile) return EINVAL; return (*pfile = libc_w32fopen(filename,mode)) != NULL ? EOK : libc_geterrno(); }
INTERN errno_t LIBCCALL libc_w16freopen_s(FILE **pfile, char16_t const *filename, char16_t const *mode, FILE *oldfile) { if unlikely(!pfile) return EINVAL; return (*pfile = libc_w16freopen(filename,mode,oldfile)) != NULL ? EOK : libc_geterrno(); }
INTERN errno_t LIBCCALL libc_w32freopen_s(FILE **pfile, char32_t const *filename, char32_t const *mode, FILE *oldfile) { if unlikely(!pfile) return EINVAL; return (*pfile = libc_w32freopen(filename,mode,oldfile)) != NULL ? EOK : libc_geterrno(); }
INTERN errno_t LIBCCALL libc_dos_w16fopen_s(FILE **pfile, char16_t const *filename, char16_t const *mode) { if unlikely(!pfile) return EINVAL; return (*pfile = libc_dos_w16fopen(filename,mode)) != NULL ? EOK : libc_geterrno(); }
INTERN errno_t LIBCCALL libc_dos_w32fopen_s(FILE **pfile, char32_t const *filename, char32_t const *mode) { if unlikely(!pfile) return EINVAL; return (*pfile = libc_dos_w32fopen(filename,mode)) != NULL ? EOK : libc_geterrno(); }
INTERN errno_t LIBCCALL libc_dos_w16freopen_s(FILE **pfile, char16_t const *filename, char16_t const *mode, FILE *oldfile) { if unlikely(!pfile) return EINVAL; return (*pfile = libc_dos_w16freopen(filename,mode,oldfile)) != NULL ? EOK : libc_geterrno(); }
INTERN errno_t LIBCCALL libc_dos_w32freopen_s(FILE **pfile, char32_t const *filename, char32_t const *mode, FILE *oldfile) { if unlikely(!pfile) return EINVAL; return (*pfile = libc_dos_w32freopen(filename,mode,oldfile)) != NULL ? EOK : libc_geterrno(); }
INTERN FILE *LIBCCALL libc_w16fdopen(fd_t fd, char16_t const *mode) {
 char *utf8_mode; FILE *result;
 utf8_mode = libc_utf16to8m(mode);
 if (!utf8_mode) return NULL;
 result = libc_fdopen(fd,utf8_mode);
 libc_free(utf8_mode);
 return result;
}
INTERN FILE *LIBCCALL libc_w32fdopen(fd_t fd, char32_t const *mode) {
 char *utf8_mode; FILE *result;
 utf8_mode = libc_utf32to8m(mode);
 if (!utf8_mode) return NULL;
 result = libc_fdopen(fd,utf8_mode);
 libc_free(utf8_mode);
 return result;
}
INTERN FILE *LIBCCALL
libc_w16popen(char16_t const *command, char16_t const *mode) {
 char *utf8_command,*utf8_mode; FILE *result = NULL;
 utf8_command = libc_utf16to8m(command);
 if (!utf8_command) goto end;
 utf8_mode = libc_utf16to8m(mode);
 if (!utf8_mode) goto end2;
 result = libc_popen(utf8_command,utf8_mode);
 libc_free(utf8_mode);
end2: libc_free(utf8_command);
end:  return result;
}
INTERN FILE *LIBCCALL
libc_w32popen(char32_t const *command, char32_t const *mode) {
 char *utf8_command,*utf8_mode; FILE *result = NULL;
 utf8_command = libc_utf32to8m(command);
 if (!utf8_command) goto end;
 utf8_mode = libc_utf32to8m(mode);
 if (!utf8_mode) goto end2;
 result = libc_popen(utf8_command,utf8_mode);
 libc_free(utf8_mode);
end2: libc_free(utf8_command);
end:  return result;
}
EXPORT(__KSYMw16(_wfsopen),libc_w16fsopen);
EXPORT(__KSYMw32(_wfsopen),libc_w32fsopen);
EXPORT(__DSYMw16(_wfsopen),libc_dos_w16fsopen);
EXPORT(__DSYMw32(_wfsopen),libc_dos_w32fsopen);
EXPORT(__KSYMw16(_wfopen),libc_w16fopen);
EXPORT(__KSYMw32(_wfopen),libc_w32fopen);
EXPORT(__DSYMw16(_wfopen),libc_dos_w16fopen);
EXPORT(__DSYMw32(_wfopen),libc_dos_w32fopen);
EXPORT(__KSYMw16(_wfreopen),libc_w16freopen);
EXPORT(__KSYMw32(_wfreopen),libc_w32freopen);
EXPORT(__DSYMw16(_wfreopen),libc_dos_w16freopen);
EXPORT(__DSYMw32(_wfreopen),libc_dos_w32freopen);
EXPORT(__KSYMw16(_wfopen_s),libc_w16fopen_s);
EXPORT(__KSYMw32(_wfopen_s),libc_w32fopen_s);
EXPORT(__DSYMw16(_wfopen_s),libc_dos_w16fopen_s);
EXPORT(__DSYMw32(_wfopen_s),libc_dos_w32fopen_s);
EXPORT(__KSYMw16(_wfreopen_s),libc_w16freopen_s);
EXPORT(__KSYMw32(_wfreopen_s),libc_w32freopen_s);
EXPORT(__DSYMw16(_wfreopen_s),libc_dos_w16freopen_s);
EXPORT(__DSYMw32(_wfreopen_s),libc_dos_w32freopen_s);
EXPORT(wpopen,libc_w32popen);
EXPORT(_wpopen,libc_w16popen);
EXPORT(wfdopen,libc_w32fdopen);
EXPORT(_wfdopen,libc_w16fdopen);

/* Get wide character functions */
DEFINE_INTERN_ALIAS(libc_16fgetwchar,libc_16getwchar);
DEFINE_INTERN_ALIAS(libc_32fgetwchar,libc_32getwchar);
EXPORT(_fgetwchar,libc_16fgetwchar);
EXPORT(_fgetwc_nolock,libc_16fgetwc_unlocked);

/* Put wide character functions */
DEFINE_INTERN_ALIAS(libc_16fputwchar,libc_16putwchar);
DEFINE_INTERN_ALIAS(libc_32fputwchar,libc_32putwchar);
EXPORT(_fputwchar,libc_16fputwchar);
EXPORT(_fputwc_nolock,libc_16fputwc_unlocked);

/* Unget character functions */
EXPORT(ungetwc_unlocked,libc_32ungetwc_unlocked);
EXPORT(_ungetwc_nolock,libc_16ungetwc_unlocked);

/* Get wide string functions */
INTERN char16_t *LIBCCALL libc_16getws(char16_t *__restrict buf) { return libc_16fgetws(buf,(size_t)-1,stdin); }
INTERN char32_t *LIBCCALL libc_32getws(char32_t *__restrict buf) { return libc_32fgetws(buf,(size_t)-1,stdin); }
INTERN char16_t *LIBCCALL libc_16getws_s(char16_t *__restrict buf, size_t buflen) { return libc_16fgetws(buf,buflen,stdin); }
INTERN char32_t *LIBCCALL libc_32getws_s(char32_t *__restrict buf, size_t buflen) { return libc_32fgetws(buf,buflen,stdin); }
EXPORT(getws,libc_32getws);
EXPORT(_getws,libc_16getws);
EXPORT(getws_s,libc_32getws_s);
EXPORT(_getws_s,libc_16getws_s);

/* Put wide string functions */
INTERN int LIBCCALL libc_16putws(char16_t const *__restrict str) {
 int result = -1; char *utf8_str = libc_utf16to8m(str);
 if (utf8_str) result = libc_puts(utf8_str),libc_free(utf8_str);
 return result;
}
INTERN int LIBCCALL libc_32putws(char32_t const *__restrict str) {
 int result = -1; char *utf8_str = libc_utf32to8m(str);
 if (utf8_str) result = libc_puts(utf8_str),libc_free(utf8_str);
 return result;
}
EXPORT(putws,libc_32putws);
EXPORT(_putws,libc_16putws);

#endif /* CONFIG_LIBC_NO_DOS_LIBC */

DECL_END
#endif /* !CONFIG_LIBC_USES_NEW_STDIO */

#endif /* !GUARD_LIBS_LIBC_STDIO_FILE_C */
