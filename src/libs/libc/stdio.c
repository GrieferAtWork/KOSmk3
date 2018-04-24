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
#ifndef GUARD_LIBS_LIBC_STDIO_C
#define GUARD_LIBS_LIBC_STDIO_C 1

#include "libc.h"
#include "stdio.h"
#include "errno.h"
#include "unistd.h"
#include "malloc.h"
#include "tty.h"
#include "rtl.h"
#include <stdio.h>
#include <errno.h>
#include <syslog.h>
#include <bits/io-file.h>
#include <hybrid/align.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/minmax.h>
#include <hybrid/list/list.h>
#include <kos/sched/mutex.h>

#ifdef CONFIG_LIBC_USES_NEW_STDIO
DECL_BEGIN


CRT_STDIO ATTR_NORETURN void LIBCCALL
libc_throw_io_error(void) {
 libc_error_throw(E_IOERROR);
}

CRT_STDIO ATTR_NOTHROW void LIBCCALL
FileBuffer_SetError(FileBuffer *__restrict self) {
 LIBC_TRY {
  self->fb_flag |= __IO_FILE_IOERR;
  COMPILER_WRITE_BARRIER();
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
 }
}


CRT_STDIO size_t LIBCCALL
FileBuffer_SystemRead(FileBuffer *__restrict self,
                      void *buffer, size_t bufsize) {
 ssize_t result;
 if likely(!FileBuffer_HasCookie(self))
    return libc_Xread(self->fb_file,buffer,bufsize);
 if (!self->fb_ops.cio_read) return 0;
 result = (*self->fb_ops.cio_read)(self->fb_arg,
                                  (char *)buffer,
                                   bufsize);
 if unlikely(result < 0)
    libc_throw_io_error();
 return (size_t)result;
}
CRT_STDIO size_t LIBCCALL
FileBuffer_SystemWrite(FileBuffer *__restrict self,
                       void const *buffer, size_t bufsize) {
 ssize_t result;
 if likely(!FileBuffer_HasCookie(self))
    return libc_Xwrite(self->fb_file,buffer,bufsize);
 if (!self->fb_ops.cio_write) return bufsize;
 result = (*self->fb_ops.cio_write)(self->fb_arg,
                                   (char *)buffer,
                                    bufsize);
 if unlikely(result < 0)
    libc_throw_io_error();
 return (size_t)result;
}
CRT_STDIO pos64_t LIBCCALL
FileBuffer_SystemSeek(FileBuffer *__restrict self,
                      off64_t offset, int whence) {
 if likely(!self->fb_ops.cio_seek)
    return libc_Xlseek64(self->fb_file,offset,whence);
 if (!self->fb_ops.cio_seek)
    libc_error_throw(E_NOT_IMPLEMENTED);
 if unlikely((*self->fb_ops.cio_seek)(self->fb_arg,&offset,whence) < 0)
    libc_throw_io_error();
 return (pos64_t)offset;
}
CRT_STDIO int LIBCCALL
FileBuffer_SystemClose(FileBuffer *__restrict self) {
 if likely(!self->fb_ops.cio_close) {
  if (self->fb_file & __IO_FILE_IONOFD)
      return 0;
  return libc_close(self->fb_file);
 }
 return (*self->fb_ops.cio_close)(self->fb_arg);
}




PRIVATE DEFINE_ATOMIC_RWLOCK(buffer_ttys_lock);
PRIVATE WEAK FileBuffer *buffer_ttys = NULL;
#define buffer_ttys_lock_enter() atomic_rwlock_write(&buffer_ttys_lock)
#define buffer_ttys_lock_leave() atomic_rwlock_endwrite(&buffer_ttys_lock)

PRIVATE DEFINE_ATOMIC_RWLOCK(all_buffers_lock);
PRIVATE WEAK FileBuffer *all_buffers = NULL;
#define all_buffers_lock_enter() atomic_rwlock_write(&all_buffers_lock)
#define all_buffers_lock_leave() atomic_rwlock_endwrite(&all_buffers_lock)


CRT_STDIO void LIBCCALL
FileBuffer_AddChangedTTY(FileBuffer *__restrict self) {
 buffer_ttys_lock_enter();
 if (!self->fb_ttych.le_pself) {
  self->fb_ttych.le_pself = &buffer_ttys;
  if ((self->fb_ttych.le_next = buffer_ttys) != NULL)
       buffer_ttys->fb_ttych.le_pself = &self->fb_ttych.le_next;
  buffer_ttys = self;
 }
 buffer_ttys_lock_leave();
}
CRT_STDIO void LIBCCALL
FileBuffer_RemoveChangedTTY(FileBuffer *__restrict self) {
 buffer_ttys_lock_enter();
 if (self->fb_ttych.le_pself) {
  if ((*self->fb_ttych.le_pself = self->fb_ttych.le_next) != NULL)
        self->fb_ttych.le_next->fb_ttych.le_pself = self->fb_ttych.le_pself;
  self->fb_ttych.le_pself = NULL;
 }
 buffer_ttys_lock_leave();
}
CRT_STDIO void LIBCCALL
FileBuffer_Register(FileBuffer *__restrict self) {
 all_buffers_lock_enter();
 LIST_INSERT(all_buffers,self,fb_files);
 all_buffers_lock_leave();
}
CRT_STDIO void LIBCCALL
FileBuffer_Unregister(FileBuffer *__restrict self) {
 all_buffers_lock_enter();
 LIST_REMOVE(self,fb_files);
 all_buffers_lock_leave();
}


CRT_STDIO_API void LIBCCALL
FileBuffer_InitFile(FileBuffer *__restrict self) {
 self->fb_zero = 0;
 mutex_init(&self->fb_lock);
 self->fb_refcnt         = 1;
 self->fb_ops.cio_close  = NULL;
 self->fb_chng           = NULL;
 self->fb_chsz           = 0;
 self->fb_ttych.le_pself = NULL;
 self->fb_files.le_pself = NULL;
 self->fb_fblk           = 0;
 self->fb_fpos           = 0;
 self->fb_cnt            = 0;
 self->fb_base           = NULL;
 self->fb_size           = 0;
 self->fb_flag           = FILE_BUFFER_FNORMAL;
}

CRT_STDIO_API ATTR_MALLOC
FileBuffer *LIBCCALL FileBuffer_Alloc(void) {
 FileBuffer *result;
 result = (FileBuffer *)libc_malloc(sizeof(FileBuffer));
#ifndef __INTELLISENSE__
 if likely(result) {
  result->if_exdata = (struct iofile_data *)libc_malloc(sizeof(struct iofile_data));
  if unlikely(!result->if_exdata) {
   libc_free(result);
   result = NULL;
  }
 }
#endif
 FileBuffer_InitFile(result);
 return result;
}
CRT_STDIO_API ATTR_MALLOC ATTR_RETNONNULL
FileBuffer *LIBCCALL FileBuffer_XAlloc(void) {
 FileBuffer *result;
 result = (FileBuffer *)libc_Xmalloc(sizeof(FileBuffer));
#ifndef __INTELLISENSE__
 LIBC_TRY {
  result->if_exdata = (struct iofile_data *)libc_Xmalloc(sizeof(struct iofile_data));
  FileBuffer_InitFile(result);
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  libc_free(result);
  error_rethrow();
 }
#endif
 return result;
}
CRT_STDIO_API void LIBCCALL
FileBuffer_Free(FileBuffer *__restrict self) {
#ifndef __INTELLISENSE__
 libc_free(self->if_exdata);
#endif
 libc_free(self);
}


CRT_STDIO int LIBCCALL
FileBuffer_Destroy(FileBuffer *__restrict self) {
 int COMPILER_IGNORE_UNINITIALIZED(result);
 LIBC_TRY {
  /* Unregister the file. */
  FileBuffer_RemoveChangedTTY(self);
  FileBuffer_Unregister(self);
  /* Flush the buffer one last time. */
  FileBuffer_Flush(self);
  /* Close + free the file. */
  result = FileBuffer_SystemClose(self);
  if (!(self->fb_flag & FILE_BUFFER_FSTATIC))
        FileBuffer_Free(self);
 } LIBC_EXCEPT(libc_except_errno()) {
  result = -1;
 }
 return result;
}




CRT_STDIO void LIBCCALL
FileBuffer_FlushTTYLineBuffers(FileBuffer *sender) {
 FileBuffer *buffer;
 for (;;) {
  buffer_ttys_lock_enter();
  buffer = buffer_ttys;
  if (!buffer) {
   buffer_ttys_lock_leave();
   break;
  }
  FileBuffer_Incref(buffer);
  buffer_ttys_lock_leave();
  /* Synchronize this buffer. */
  if (buffer == sender) {
   FileBuffer_FlushUnlocked(buffer);
  } else {
   FileBuffer_Flush(buffer);
  }
  FileBuffer_Decref(buffer);
 }
}

CRT_STDIO void LIBCCALL
FileBuffer_FlushAllBuffers(void) {
 FileBuffer *buffer;
 for (;;) {
  all_buffers_lock_enter();
  buffer = all_buffers;
  while (buffer && !buffer->fb_chsz)
      buffer = buffer->fb_files.le_next;
  if (buffer) FileBuffer_Incref(buffer);
  all_buffers_lock_leave();
  if (!buffer) break;
  /* Synchronize this buffer. */
  FileBuffer_Flush(buffer);
  FileBuffer_Decref(buffer);
 }
 FileBuffer_Flush(libc_stdin);
 FileBuffer_Flush(libc_stdout);
 FileBuffer_Flush(libc_stderr);
}

CRT_STDIO void LIBCCALL
FileBuffer_FlushAllBuffersUnlocked(void) {
 FileBuffer *buffer;
 for (;;) {
  all_buffers_lock_enter();
  buffer = all_buffers;
  while (buffer && !buffer->fb_chsz)
      buffer = buffer->fb_files.le_next;
  if (buffer) FileBuffer_Incref(buffer);
  all_buffers_lock_leave();
  if (!buffer) break;
  /* Synchronize this buffer. */
  FileBuffer_FlushUnlocked(buffer);
  FileBuffer_Decref(buffer);
 }
}

EXPORT(__KSYM(fcloseall),libc_fcloseall);
EXPORT(__DSYM(_fcloseall),libc_fcloseall);
CRT_STDIO_API int LIBCCALL libc_fcloseall(void) {
 /* This function is the definition of unsafe!
  * Why anyone would want this, I don't know. So please: don't call it... */
 while (all_buffers)
     FileBuffer_Destroy(all_buffers);
 /* Close the STD streams. */
 FileBuffer_Destroy(libc_stdin);
 FileBuffer_Destroy(libc_stdout);
 FileBuffer_Destroy(libc_stderr);
 return 0;
}



PRIVATE ATTR_RETNONNULL uint8_t *LIBCCALL
buffer_realloc_nolock(FileBuffer *__restrict self, size_t new_size) {
 if (self->fb_flag&FILE_BUFFER_FSTATICBUF)
     return (uint8_t *)libc_Xmalloc(new_size);
 assert(!(self->fb_flag&FILE_BUFFER_FREADING));
 return (uint8_t *)libc_Xrealloc(self->fb_base,new_size);
}
PRIVATE uint8_t *LIBCCALL
buffer_tryrealloc_nolock(FileBuffer *__restrict self, size_t new_size) {
 if (self->fb_flag&FILE_BUFFER_FSTATICBUF)
     return (uint8_t *)libc_malloc(new_size);
 assert(!(self->fb_flag&FILE_BUFFER_FREADING));
 return (uint8_t *)libc_realloc(self->fb_base,new_size);
}


CRT_STDIO void LIBCCALL
FileBuffer_XCheckTTY(FileBuffer *__restrict self) {
 int is_a_tty;
 u32 flags = self->fb_flag;
 if (flags&(FILE_BUFFER_FISATTY|FILE_BUFFER_FNOTATTY)) {
set_lfflag:
  if (flags&FILE_BUFFER_FLNIFTTY) {
   self->fb_flag &= ~FILE_BUFFER_FLNIFTTY;
   /* Set the line-buffered flag if it is a TTY. */
   if (flags&FILE_BUFFER_FISATTY)
       self->fb_flag |= FILE_BUFFER_FLNBUF;
  }
  return;
 }
 COMPILER_BARRIER();
 if (FileBuffer_HasCookie(self)) {
  is_a_tty = self->fb_ops.cio_seek == NULL;
 } else {
  is_a_tty = libc_isatty(self->fb_file);
 }
 COMPILER_BARRIER();
 if (is_a_tty)
     libc_syslog(LOG_DEBUG,"[STDIO] Stream handle %d is a tty\n",self->fb_file);
 self->fb_flag |= is_a_tty ? FILE_BUFFER_FISATTY : FILE_BUFFER_FNOTATTY;
 flags          = self->fb_flag;
 goto set_lfflag;
}




CRT_STDIO size_t LIBCCALL
FileBuffer_XReadUnlocked(FileBuffer *__restrict self,
                         void *__restrict buffer,
                         size_t bufsize) {
 size_t read_size,result = 0; size_t bufavail;
 u32 old_flags; pos64_t next_data;
 uint8_t *new_buffer;
again:
 bufavail = self->fb_cnt;
 if likely(bufavail) {
read_from_buffer:
  if (bufavail > bufsize)
      bufavail = bufsize;
  libc_memcpy(buffer,self->fb_ptr,bufavail);
  /* Update buffer pointers. */
  self->fb_cnt -= bufavail;
  self->fb_ptr += bufavail;
  result       += bufavail;
  bufsize      -= bufavail;
  if (!bufsize) goto done;
  *(uintptr_t *)&buffer += bufavail;
 }
 /* The buffer is empty and must be re-filled. */
 /* First off: Flush any changes that had been made. */
 COMPILER_BARRIER();
 FileBuffer_XFlushUnlocked(self);
 FileBuffer_XCheckTTY(self);
 COMPILER_BARRIER();
 self->fb_chng = self->fb_base;
 self->fb_chsz = 0;
 /* Unlikely: But can happen due to recursive callbacks. */
 if unlikely(self->fb_cnt) goto read_from_buffer;
 /* If we're a TTY buffer, flush all other TTY buffers before reading. */
 if (self->fb_flag&FILE_BUFFER_FISATTY) {
  COMPILER_BARRIER();
  FileBuffer_FlushTTYLineBuffers(self);
  COMPILER_BARRIER();
  if unlikely(self->fb_cnt) goto read_from_buffer;
  self->fb_chng = self->fb_base;
  self->fb_chsz = 0;
 }

 /* Determine where the next block of data is. */
 next_data = self->fb_fblk+(self->fb_ptr-self->fb_base);

 if unlikely(self->fb_flag&(FILE_BUFFER_FNODYNSCALE|
                            FILE_BUFFER_FSTATICBUF)) {
  /* Dynamic scaling is disabled. Must forward the getc() to the underlying file. */
read_through:
  if (next_data != self->fb_fpos) {
   /* Seek in the underlying file to get where we need to go. */
   COMPILER_BARRIER();
   self->fb_fpos = FileBuffer_SystemSeek(self,(off64_t)next_data,SEEK_SET);
   COMPILER_BARRIER();
   goto again; /* Must start over because of recursive callbacks. */
  }
  COMPILER_BARRIER();
  read_size = FileBuffer_SystemRead(self,buffer,bufsize);
  COMPILER_BARRIER();
  if (!read_size)
   self->fb_flag |= FILE_BUFFER_FEOF;
  else {
   self->fb_flag &= ~FILE_BUFFER_FEOF;
  }
  self->fb_fpos = next_data+bufsize;
  result += read_size;
  goto done;
 }
 /* If no buffer had been allocated, allocate one now. */
 if unlikely(!self->fb_size) {
  /* Start out with the smallest size. */
  if (bufsize >= FILE_BUFSIZ_MAX) {
   new_buffer = buffer_realloc_nolock(self,FILE_BUFSIZ_MAX);
  } else if (bufsize <= FILE_BUFSIZ_MIN) {
   new_buffer = buffer_realloc_nolock(self,FILE_BUFSIZ_MIN);
  } else {
   /* Use a larger size based on `bufsize' */
   size_t new_alloc = 1;
   while (new_alloc < bufsize) new_alloc <<= 1;
   new_buffer = buffer_realloc_nolock(self,new_alloc);
  }
  self->fb_base = new_buffer;
  self->fb_size = FILE_BUFSIZ_MIN;
 } else if (bufsize >= self->fb_size) {
  /* The caller wants at least as much as this buffer could even handle.
   * Upscale the buffer, or use load data using read-through mode. */
  if (self->fb_flag&(FILE_BUFFER_FNODYNSCALE|
                     FILE_BUFFER_FSTATICBUF|
                     FILE_BUFFER_FREADING))
      goto read_through;
  if (bufsize >= FILE_BUFSIZ_MAX)
      goto read_through;
  /* Upscale the buffer. */
  new_buffer = buffer_tryrealloc_nolock(self,bufsize);
  /* If the allocation failed, also use read-through mode. */
  if unlikely(!new_buffer)
     goto read_through;
  self->fb_base = new_buffer;
  self->fb_size = bufsize;
 }

 self->fb_ptr  = self->fb_base;
 self->fb_chng = self->fb_base;
 self->fb_fblk = next_data;
 assert(self->fb_cnt  == 0);
 assert(self->fb_chsz == 0);
 assert(self->fb_size != 0);
 if (next_data != self->fb_fpos) {
  /* Seek in the underlying file to get where we need to go. */
  COMPILER_BARRIER();
  FileBuffer_SystemSeek(self,(off64_t)next_data,SEEK_SET);
  COMPILER_BARRIER();
  self->fb_fpos = next_data;
  /* Must start over because of recursive callbacks. */
  goto again;
 }

 /* Actually read the data. */
 new_buffer     = self->fb_base;
 old_flags      = self->fb_flag;
 self->fb_flag |= FILE_BUFFER_FREADING;
 COMPILER_BARRIER();
 read_size = FileBuffer_SystemRead(self,
                                   self->fb_base,
                                   self->fb_size);
 COMPILER_BARRIER();
 self->fb_flag &= ~FILE_BUFFER_FREADING;
 self->fb_flag |= old_flags&FILE_BUFFER_FREADING;
 if unlikely(read_size == 0) goto done_eof;
 self->fb_flag &= ~FILE_BUFFER_FEOF;
 self->fb_fpos = next_data+(size_t)read_size;
 self->fb_ptr  = self->fb_base;
 self->fb_cnt  = (size_t)read_size;
 goto again;
done_eof:
 self->fb_flag |= FILE_BUFFER_FEOF;
done:
 return result;
}

CRT_STDIO size_t LIBCCALL
FileBuffer_XWriteUnlocked(FileBuffer *__restrict self,
                          void const *__restrict buffer,
                          size_t bufsize) {
 size_t result = 0; size_t new_bufsize;
 size_t bufavail; uint8_t *new_buffer;
 FileBuffer_XCheckTTY(self);
 if (self->fb_flag & FILE_BUFFER_FREADONLY)
     libc_error_throwf(E_FILESYSTEM_ERROR,ERROR_FS_ACCESS_ERROR);
again:
 /* Fill available buffer. */
 bufavail = (self->fb_base+self->fb_size)-self->fb_ptr;
 if likely(bufavail) {
  if (bufavail > bufsize)
      bufavail = bufsize;
  if unlikely(!bufavail) goto done;
  libc_memcpy(self->fb_ptr,buffer,bufavail);
  result += bufavail;
  /* Update the changed file-area. */
  if (!self->fb_chsz) {
   self->fb_chng = self->fb_ptr;
   self->fb_chsz = bufavail;
  } else {
   if (self->fb_chng > self->fb_ptr) {
    self->fb_chsz += (size_t)(self->fb_chng-self->fb_ptr);
    self->fb_chng  = self->fb_ptr;
   }
   if (self->fb_chng+self->fb_chsz < self->fb_ptr+bufavail)
       self->fb_chsz = (size_t)((self->fb_ptr+bufavail)-self->fb_chng);
  }
  /* If this is a TTY device, add it to the chain of changed ones. */
  if (self->fb_flag & FILE_BUFFER_FISATTY)
      FileBuffer_AddChangedTTY(self);

  /* Update the file pointer. */
  self->fb_ptr += bufavail;
  if (self->fb_cnt >= bufavail)
      self->fb_cnt -= bufavail;
  else self->fb_cnt = 0;
  /* When operating in line-buffered mode, check
   * if there was a linefeed in what we just wrote. */
  if ((self->fb_flag & FILE_BUFFER_FLNBUF) &&
      (libc_memchr(buffer,'\n',bufsize) ||
       libc_memchr(buffer,'\r',bufsize))) {
   if (self->fb_flag & FILE_BUFFER_FISATTY)
       FileBuffer_FlushTTYLineBuffers(self);
   /* Flush this file. */
   COMPILER_BARRIER();
   FileBuffer_XFlushUnlocked(self);
   COMPILER_BARRIER();
   bufsize -= bufavail;
   if (!bufsize) goto done;
   *(uintptr_t *)&buffer += bufavail;
   goto again;
  }

  bufsize -= bufavail;
  if (!bufsize) goto done;
  *(uintptr_t *)&buffer += bufavail;
 }
 /* No more buffer available.
  * Either we must flush the buffer, or we must extend it. */
 if (self->fb_size >= FILE_BUFSIZ_MAX ||
    (self->fb_flag&(FILE_BUFFER_FNODYNSCALE|
                    FILE_BUFFER_FSTATICBUF|
                    FILE_BUFFER_FREADING))) {
  /* Buffer is too large or cannot be relocated.
   * >> Therefor, we must flush it, then try again. */
  if (self->fb_flag & FILE_BUFFER_FISATTY)
      FileBuffer_FlushTTYLineBuffers(self);
  COMPILER_BARRIER();
  FileBuffer_XFlushUnlocked(self);
  COMPILER_BARRIER();
  self->fb_chng = self->fb_base;
  self->fb_chsz = 0;
  /* Check for special case: If the buffer is fixed to zero-length,
   *                         we must act as a write-through buffer. */
  if (!self->fb_size) {
do_writethrough:
   COMPILER_BARRIER();
   result += FileBuffer_SystemWrite(self,buffer,bufsize);
   COMPILER_BARRIER();
   goto done;
  }
  /* If there is no more available buffer to-be read,
   * reset the file pointer and change to the next chunk. */
  if (!self->fb_cnt) {
   self->fb_fblk += (size_t)(self->fb_ptr-self->fb_base);
   self->fb_ptr   = self->fb_base;
  }
  goto again;
 }

 /* Extend the buffer */
 new_bufsize = self->fb_size*2;
 if (new_bufsize < FILE_BUFSIZ_MIN)
     new_bufsize = FILE_BUFSIZ_MIN;
 new_buffer = buffer_tryrealloc_nolock(self,new_bufsize);
 if unlikely(!new_buffer) {
  /* Buffer relocation failed. - sync() + operate in write-through mode as fallback. */
  if (self->fb_flag & FILE_BUFFER_FISATTY)
      FileBuffer_FlushTTYLineBuffers(self);
  COMPILER_BARRIER();
  FileBuffer_XFlushUnlocked(self);
  COMPILER_BARRIER();
  self->fb_chng = self->fb_base;
  self->fb_chsz = 0;
  goto do_writethrough;
 }
 /* Install the new buffer. */
 self->fb_ptr  = new_buffer+(self->fb_ptr-self->fb_base);
 self->fb_chng = new_buffer+(self->fb_chng-self->fb_base);
 self->fb_size = new_bufsize;
 self->fb_base = new_buffer;
 /* Go back and use the new buffer. */
 goto again;
done:
 return result;
}

CRT_STDIO void LIBCCALL
FileBuffer_XFlushUnlocked(FileBuffer *__restrict self) {
 pos64_t abs_chngpos;
 size_t changed_size;
again:
 changed_size = self->fb_chsz;
 if (!changed_size) goto done;
 abs_chngpos  = self->fb_fblk;
 abs_chngpos += (self->fb_chng-self->fb_base);
 if (abs_chngpos != self->fb_fpos) {
  /* Seek to the position where we need to change stuff. */
  COMPILER_BARRIER();
  self->fb_fpos = FileBuffer_SystemSeek(self,(off64_t)abs_chngpos,SEEK_SET);
  COMPILER_BARRIER();
  /* Since the buffer may have changed, we need to start over. */
  goto again;
 }
 /* Write all changed data. */
 COMPILER_BARRIER();
 changed_size = FileBuffer_SystemWrite(self,
                                       self->fb_chng,
                                       changed_size);
 COMPILER_BARRIER();
 if (changed_size == self->fb_chsz && self->fb_file &&
     abs_chngpos  == self->fb_fblk+(self->fb_chng-self->fb_base)) {
  /* Data was synchronized. */
  self->fb_chsz  = 0;
  self->fb_fpos += changed_size;
  self->fb_ptr   = self->fb_chng+changed_size;

  /* Remove the buffer from the chain of changed, line-buffered TTYs. */
  FileBuffer_RemoveChangedTTY(self);

  /* Also synchronize the underlying file. */
  if ((self->fb_flag & FILE_BUFFER_FSYNC) &&
      !FileBuffer_HasCookie(self))
       libc_Xfsync(self->fb_file);
 }
done:
 ;
}

CRT_STDIO pos64_t LIBCCALL
FileBuffer_TellUnlocked(FileBuffer *__restrict self) {
 pos64_t result;
 result  = self->fb_fblk;
 result += (self->fb_ptr - self->fb_base);
 return result;
}

CRT_STDIO void LIBCCALL
FileBuffer_XSeekUnlocked(FileBuffer *__restrict self,
                         off64_t off, int whence) {
 pos64_t new_abspos;
 if (whence == SEEK_SET || whence == SEEK_CUR) {
  pos64_t old_abspos;
  uint8_t *new_pos;
  /* For these modes, we can calculate the new position,
   * allowing for in-buffer seek, as well as delayed seek. */
  old_abspos  = self->fb_fblk;
  old_abspos += (self->fb_ptr-self->fb_base);
  if (whence == SEEK_SET)
   new_abspos = (pos64_t)off;
  else {
   new_abspos = old_abspos + off;
  }
  if (new_abspos < old_abspos)
      goto full_seek;
#if __SIZEOF_POINTER__ < 8
  if ((new_abspos-old_abspos) >= SIZE_MAX)
      goto full_seek;
#endif
  /* Seek-ahead-in-buffer. */
  new_pos = self->fb_base+(size_t)(new_abspos-self->fb_fblk);
#if __SIZEOF_POINTER__ < 8
  if (new_pos < self->fb_base) goto full_seek;
#endif
  /* Truncate the read buffer */
  if (new_pos < self->fb_ptr) {
   /* Seek below the current pointer (but we don't
    * remember how much was actually read there, so
    * we simply truncate the buffer fully) */
   self->fb_cnt = 0;
  } else {
   size_t skipsz = (size_t)(new_pos-self->fb_ptr);
   if (self->fb_cnt >= skipsz)
       self->fb_cnt -= skipsz;
   else self->fb_cnt = 0;
  }
  self->fb_ptr = new_pos;
  return;
 }
full_seek:
 FileBuffer_XCheckTTY(self);
 if (self->fb_flag & FILE_BUFFER_FISATTY)
     FileBuffer_FlushTTYLineBuffers(self);
 /* Synchronize the buffer. */
 COMPILER_BARRIER();
 FileBuffer_XFlushUnlocked(self);
 COMPILER_BARRIER();
 self->fb_chng = self->fb_base;
 self->fb_chsz = 0;
 COMPILER_BARRIER();
 /* Do a full seek using the underlying file. */
 new_abspos = FileBuffer_SystemSeek(self,off,whence);
 COMPILER_BARRIER();

 /* Clear the buffer and set the new file pointer. */
 self->fb_fblk = new_abspos;
 self->fb_fpos = new_abspos;
 self->fb_cnt  = 0;
 self->fb_ptr  = self->fb_base;
 self->fb_chng = self->fb_base;
 self->fb_chsz = 0;
}

CRT_STDIO int LIBCCALL
FileBuffer_XGetcUnlocked(FileBuffer *__restrict self) {
again:
 if (self->fb_cnt) {
  /* Simple case: we can read from the active buffer. */
  --self->fb_cnt;
  return (int)(char)*self->fb_ptr++;
 }
 /* The buffer is empty and must be re-filled. */
 LIBC_TRY {
  if (FileBuffer_XFillUnlocked(self) == EOF)
      return EOF;
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  pos64_t next_data; char result;
  if (libc_error_code() != E_BUFFER_TOO_SMALL ||
      libc_error_code() != E_BADALLOC)
      libc_error_rethrow();
  /* Use read-through if we couldn't read using a buffer. */
  next_data = self->fb_fblk+(self->fb_ptr-self->fb_base);
  /* Dynamic scaling is disabled. Must forward the getc() to the underlying file. */
  if (next_data != self->fb_fpos) {
   /* Seek in the underlying file to get where we need to go. */
   COMPILER_BARRIER();
   self->fb_fpos = FileBuffer_SystemSeek(self,
                                        (off64_t)next_data,
                                         SEEK_SET);
   COMPILER_BARRIER();
   /* Must start over because of recursive callbacks. */
   goto again;
  }
  COMPILER_BARRIER();
  FileBuffer_XCheckTTY(self);
  if (self->fb_flag & FILE_BUFFER_FISATTY) {
   COMPILER_BARRIER();
   FileBuffer_FlushTTYLineBuffers(self);
   COMPILER_BARRIER();
   if unlikely(self->fb_cnt)
      goto again;
  }
  if (!FileBuffer_SystemRead(self,&result,sizeof(char)))
       return EOF;
  /* Set the file and block address. */
  self->fb_fpos = next_data+1;
  self->fb_fblk = next_data+1;
  return (int)result;
 }
 goto again;
}

CRT_STDIO int LIBCCALL
FileBuffer_XUngetcUnlocked(FileBuffer *__restrict self, int ch) {
 uint8_t *new_buffer;
 size_t new_bufsize,inc_size;
 /* Simple case: unget() the character. */
 if (self->fb_ptr != self->fb_base)
     goto unget_in_buffer;
 /* The buffer is already full. - Try to resize it, then insert at the front. */
 if (self->fb_flag&FILE_BUFFER_FREADING)
     goto eof;
 if (self->fb_flag&(FILE_BUFFER_FNODYNSCALE|
                    FILE_BUFFER_FSTATICBUF)) {
  /* Check for special case: Even when dynscale is disabled,
   * still allow for an unget buffer of at least a single byte. */
  if (self->fb_size != 0) goto eof;
 }
 /* If the current block cannot be further extended, that's an EOF. */
 if (!self->fb_fblk) goto eof;
 inc_size = self->fb_size;
 /* Determine the minimum buffer size. */
 if unlikely(!inc_size)
    inc_size = (self->fb_flag&(FILE_BUFFER_FNODYNSCALE|FILE_BUFFER_FSTATICBUF)) ? 1 : FILE_BUFSIZ_MIN;
 if (inc_size > self->fb_fblk)
     inc_size = (size_t)self->fb_fblk;
 new_bufsize = self->fb_size+inc_size;
 new_buffer  = buffer_tryrealloc_nolock(self,new_bufsize);
 if unlikely(!new_buffer) {
  inc_size    = 1;
  new_bufsize = self->fb_size+1;
  new_buffer  = buffer_realloc_nolock(self,new_bufsize);
 }
 assert(new_bufsize > self->fb_size);
 /* Install the new buffer. */
 self->fb_ptr   = new_buffer+(self->fb_ptr-self->fb_base)+inc_size;
 if (self->fb_chsz) self->fb_chng = new_buffer+(self->fb_chng-self->fb_base)+inc_size;
 self->fb_fblk -= inc_size;
 self->fb_base  = new_buffer;
 self->fb_size  = new_bufsize;
 assert(self->fb_ptr != self->fb_base);
 /* Finally, insert the character into the buffer. */
unget_in_buffer:
 *--self->fb_ptr = (uint8_t)(char)ch;
 ++self->fb_cnt;
 return 0;
eof:
 return EOF;
}

CRT_STDIO int LIBCCALL
FileBuffer_XFillUnlocked(FileBuffer *__restrict self) {
 uint8_t *new_buffer; size_t read_size;
 u32 old_flags; pos64_t next_data;
again:
 /* First off: Flush any changes that had been made. */
 FileBuffer_XFlushUnlocked(self);
 FileBuffer_XCheckTTY(self);
 COMPILER_BARRIER();
 self->fb_chng = self->fb_base;
 self->fb_chsz = 0;
 /* Unlikely: But can happen due to recursive callbacks. */
 if unlikely(self->fb_cnt) return 0;

 /* If we're a TTY buffer, flush all other TTY buffers before reading. */
 if (self->fb_flag & FILE_BUFFER_FISATTY) {
  COMPILER_BARRIER();
  FileBuffer_FlushTTYLineBuffers(self);
  COMPILER_BARRIER();
  if unlikely(self->fb_cnt) return 0;
  self->fb_chng = self->fb_base;
  self->fb_chsz = 0;
 }

 /* Determine where the next block of data is. */
 next_data = self->fb_fblk+(self->fb_ptr-self->fb_base);

 /* If no buffer had been allocated, allocate one now. */
 if unlikely(!self->fb_size) {
  if unlikely(self->fb_flag&(FILE_BUFFER_FNODYNSCALE|FILE_BUFFER_FSTATICBUF))
     libc_error_throwf(E_BUFFER_TOO_SMALL,(size_t)0,(size_t)1);
  /* Start out with the smallest size. */
  new_buffer    = buffer_realloc_nolock(self,FILE_BUFSIZ_MIN);
  self->fb_base = new_buffer;
  self->fb_size = FILE_BUFSIZ_MIN;
 } else {
  if (self->fb_size < FILE_BUFSIZ_MIN &&
    !(self->fb_flag&(FILE_BUFFER_FNODYNSCALE|
                     FILE_BUFFER_FSTATICBUF|
                     FILE_BUFFER_FREADING))) {
   /* Upscale the buffer. */
   new_buffer    = buffer_realloc_nolock(self,FILE_BUFSIZ_MIN);
   self->fb_base = new_buffer;
   self->fb_size = FILE_BUFSIZ_MIN;
  }
 }

 self->fb_ptr  = self->fb_base;
 self->fb_chng = self->fb_base;
 self->fb_fblk = next_data;
 assert(self->fb_cnt  == 0);
 assert(self->fb_chsz == 0);
 assert(self->fb_size != 0);
 if (next_data != self->fb_fpos) {
  /* Seek in the underlying file to get where we need to go. */
  COMPILER_BARRIER();
  self->fb_fpos = FileBuffer_SystemSeek(self,
                                       (off64_t)next_data,
                                        SEEK_SET);
  COMPILER_BARRIER();
  if (self->fb_cnt) return 0;
  goto again; /* Must start over because of recursive callbacks. */
 }

 /* Actually read the data. */
 new_buffer = self->fb_base;
 old_flags  = self->fb_flag;
 self->fb_flag |= FILE_BUFFER_FREADING;
 COMPILER_BARRIER();
 read_size = FileBuffer_SystemRead(self,
                                   self->fb_base,
                                   self->fb_size);
 COMPILER_BARRIER();
 self->fb_flag &= ~FILE_BUFFER_FREADING;
 self->fb_flag |= old_flags&FILE_BUFFER_FREADING;
 if unlikely(self->fb_cnt) return 0;
 if unlikely(self->fb_chsz) goto again;
 self->fb_fpos = next_data+(size_t)read_size;
 /* Check for special case: EOF reached. */
 if (!read_size) {
  self->fb_flag |= FILE_BUFFER_FEOF;
  return EOF;
 }
 self->fb_cnt = read_size;
 self->fb_ptr = self->fb_base;
 return 0;
}

CRT_STDIO void LIBCCALL
FileBuffer_XSetvbufUnlocked(FileBuffer *__restrict self,
                            char *__restrict buffer,
                            int modes, size_t n) {
 FileBuffer_XFlushUnlocked(self);
 /* Mark the file buffer as empty and delete special flags. */
 self->fb_ptr   = self->fb_base;
 self->fb_cnt   = 0;
 self->fb_flag &= ~(FILE_BUFFER_FLNBUF|
                    FILE_BUFFER_FLNIFTTY);
 if (modes == _IONBF || modes == __DOS_IONBF) {
  /* Don't use any buffer. */
  if (!(self->fb_flag & FILE_BUFFER_FSTATICBUF))
        libc_free(self->fb_base);
  self->fb_base = NULL;
  self->fb_size = 0;
  self->fb_ptr  = NULL;
  self->fb_chng = NULL;
  self->fb_chsz = 0;
  return;
 }
 if (modes == _IOLBF || modes == __DOS_IOLBF) {
  self->fb_flag |= FILE_BUFFER_FLNBUF;
  /* Passing ZERO(0) for 'n' here causes the previous buffer to be kept. */
  if (!n) return;
 } else if (modes == _IOFBF
#if __DOS_IOFBF != _IOFBF
         && modes == __DOS_IOFBF
#endif
            ) {
 } else {
invalid_argument:
  libc_error_throw(E_INVALID_ARGUMENT);
 }
 /* Allocate/use the given buffer. */
 if (n < 2) goto invalid_argument;
 if (!buffer) {
  /* Dynamically allocate a buffer. */
  if (!(self->fb_flag & FILE_BUFFER_FSTATICBUF)) {
   /* (re-)allocate an existing buffer. */
   buffer = (char *)self->fb_base;
   /* Make sure the buffer's size has actually changed.
    * NOTE: As an extension, we accept `(size_t)-1' to keep the old buffer size. */
   if (n == (size_t)-1) n = (size_t)self->fb_size;
   else if ((size_t)self->fb_size != n) {
    buffer = (char *)libc_Xrealloc(buffer,n);
   }
  } else {
   /* Limit the max automatic buffer size to `FILE_BUFSIZ_MAX' */
   if (n > FILE_BUFSIZ_MAX)
       n = FILE_BUFSIZ_MAX;
   buffer = (char *)libc_Xmalloc(n);
  }
 } else {
  /* Mark the buffer as being fixed-length,
   * thus preventing it from being re-allocated. */
  self->fb_flag |= FILE_BUFFER_FSTATICBUF;
 }
 /* Install the given buffer. */
 self->fb_ptr  = (byte_t *)buffer;
 self->fb_base = (byte_t *)buffer;
 self->fb_size = n;
}





CRT_STDIO size_t LIBCCALL
FileBuffer_ReadUnlocked(FileBuffer *__restrict self,
                      void *__restrict buffer,
                      size_t bufsize) {
 LIBC_TRY {
  return FileBuffer_XReadUnlocked(self,buffer,bufsize);
 } LIBC_EXCEPT(libc_except_errno()) {
  FileBuffer_SetError(self);
 }
 return 0;
}

CRT_STDIO size_t LIBCCALL
FileBuffer_WriteUnlocked(FileBuffer *__restrict self,
                       void const *__restrict buffer,
                       size_t bufsize) {
 LIBC_TRY {
  return FileBuffer_XWriteUnlocked(self,buffer,bufsize);
 } LIBC_EXCEPT(libc_except_errno()) {
  FileBuffer_SetError(self);
 }
 return 0;
}

CRT_STDIO int LIBCCALL
FileBuffer_FlushUnlocked(FileBuffer *__restrict self) {
 LIBC_TRY {
  FileBuffer_XFlushUnlocked(self);
  return 0;
 } LIBC_EXCEPT(libc_except_errno()) {
  FileBuffer_SetError(self);
 }
 return -1;
}

CRT_STDIO int LIBCCALL
FileBuffer_SeekUnlocked(FileBuffer *__restrict self, off64_t off, int whence) {
 LIBC_TRY {
  FileBuffer_XSeekUnlocked(self,off,whence);
  return 0;
 } LIBC_EXCEPT(libc_except_errno()) {
  FileBuffer_SetError(self);
 }
 return -1;
}

CRT_STDIO int LIBCCALL
FileBuffer_SetvbufUnlocked(FileBuffer *__restrict self,
                         char *__restrict buffer,
                         int modes, size_t n) {
 LIBC_TRY {
  FileBuffer_XSetvbufUnlocked(self,buffer,modes,n);
  return 0;
 } LIBC_EXCEPT(libc_except_errno()) {
  FileBuffer_SetError(self);
 }
 return -1;
}

CRT_STDIO int LIBCCALL
FileBuffer_GetcUnlocked(FileBuffer *__restrict self) {
 LIBC_TRY {
  return FileBuffer_XGetcUnlocked(self);
 } LIBC_EXCEPT(libc_except_errno()) {
  FileBuffer_SetError(self);
 }
 return EOF;
}
CRT_STDIO int LIBCCALL
FileBuffer_UngetcUnlocked(FileBuffer *__restrict self, int ch) {
 LIBC_TRY {
  return FileBuffer_XUngetcUnlocked(self,ch);
 } LIBC_EXCEPT(libc_except_errno()) {
  FileBuffer_SetError(self);
 }
 return -1;
}

CRT_STDIO int LIBCCALL
FileBuffer_FillUnlocked(FileBuffer *__restrict self) {
 LIBC_TRY {
  FileBuffer_XFillUnlocked(self);
  return 0;
 } LIBC_EXCEPT(libc_except_errno()) {
  FileBuffer_SetError(self);
 }
 return -1;
}

CRT_STDIO size_t LIBCCALL
FileBuffer_XRead(FileBuffer *__restrict self,
              void *__restrict buffer,
              size_t bufsize) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(self);
 LIBC_TRY {
  result = FileBuffer_XReadUnlocked(self,buffer,bufsize);
 } LIBC_FINALLY {
  FileBuffer_Unlock(self);
 }
 return result;
}
CRT_STDIO size_t LIBCCALL
FileBuffer_XWrite(FileBuffer *__restrict self,
               void const *__restrict buffer,
               size_t bufsize) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(self);
 LIBC_TRY {
  result = FileBuffer_XWriteUnlocked(self,buffer,bufsize);
 } LIBC_FINALLY {
  FileBuffer_Unlock(self);
 }
 return result;
}
CRT_STDIO void LIBCCALL
FileBuffer_XFlush(FileBuffer *__restrict self) {
 FileBuffer_XLock(self);
 LIBC_TRY {
  FileBuffer_XFlushUnlocked(self);
 } LIBC_FINALLY {
  FileBuffer_Unlock(self);
 }
}
CRT_STDIO void LIBCCALL
FileBuffer_XSeek(FileBuffer *__restrict self,
              off64_t off, int whence) {
 FileBuffer_XLock(self);
 LIBC_TRY {
  FileBuffer_XSeekUnlocked(self,off,whence);
 } LIBC_FINALLY {
  FileBuffer_Unlock(self);
 }
}
CRT_STDIO void LIBCCALL
FileBuffer_XSetvbuf(FileBuffer *__restrict self,
                 char *__restrict buffer,
                 int modes, size_t n) {
 FileBuffer_XLock(self);
 LIBC_TRY {
  FileBuffer_XSetvbufUnlocked(self,buffer,modes,n);
 } LIBC_FINALLY {
  FileBuffer_Unlock(self);
 }
}

CRT_STDIO int LIBCCALL
FileBuffer_XGetc(FileBuffer *__restrict self) {
 int COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(self);
 LIBC_TRY {
  result = FileBuffer_XGetcUnlocked(self);
 } LIBC_FINALLY {
  FileBuffer_Unlock(self);
 }
 return result;
}

CRT_STDIO int LIBCCALL
FileBuffer_XUngetc(FileBuffer *__restrict self, int ch) {
 int COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(self);
 LIBC_TRY {
  result = FileBuffer_XUngetcUnlocked(self,ch);
 } LIBC_FINALLY {
  FileBuffer_Unlock(self);
 }
 return result;
}

CRT_STDIO int LIBCCALL
FileBuffer_XFill(FileBuffer *__restrict self) {
 int COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(self);
 LIBC_TRY {
  result = FileBuffer_XFillUnlocked(self);
 } LIBC_FINALLY {
  FileBuffer_Unlock(self);
 }
 return result;
}

CRT_STDIO pos64_t LIBCCALL
FileBuffer_XTell(FileBuffer *__restrict self) {
 pos64_t result;
 FileBuffer_XLock(self);
 result = FileBuffer_TellUnlocked(self);
 FileBuffer_Unlock(self);
 return result;
}


CRT_STDIO size_t LIBCCALL
FileBuffer_Read(FileBuffer *__restrict self,
             void *__restrict buffer,
             size_t bufsize) {
 size_t result;
 while (FileBuffer_Lock(self))
    if (libc_geterrno() != EINTR)
        return 0;
 result = FileBuffer_ReadUnlocked(self,buffer,bufsize);
 FileBuffer_Unlock(self);
 return result;
}

CRT_STDIO size_t LIBCCALL
FileBuffer_Write(FileBuffer *__restrict self,
              void const *__restrict buffer,
              size_t bufsize) {
 size_t result;
 while (FileBuffer_Lock(self))
    if (libc_geterrno() != EINTR)
        return 0;
 result = FileBuffer_WriteUnlocked(self,buffer,bufsize);
 FileBuffer_Unlock(self);
 return result;
}

CRT_STDIO int LIBCCALL
FileBuffer_Flush(FileBuffer *__restrict self) {
 int result;
 if ((result = FileBuffer_Lock(self)) == 0) {
  result = FileBuffer_FlushUnlocked(self);
  FileBuffer_Unlock(self);
 }
 return result;
}

CRT_STDIO off64_t LIBCCALL
FileBuffer_Tell(FileBuffer *__restrict self) {
 off64_t result;
 if (FileBuffer_Lock(self))
     return -1;
 result = (off64_t)FileBuffer_TellUnlocked(self);
 FileBuffer_Unlock(self);
 return result;
}

CRT_STDIO int LIBCCALL
FileBuffer_Seek(FileBuffer *__restrict self, off64_t off, int whence) {
 int result;
 if ((result = FileBuffer_Lock(self)) == 0) {
  result = FileBuffer_SeekUnlocked(self,off,whence);
  FileBuffer_Unlock(self);
 }
 return result;
}

CRT_STDIO int LIBCCALL
FileBuffer_Setvbuf(FileBuffer *__restrict self,
                char *__restrict buffer,
                int modes, size_t n) {
 int result;
 if ((result = FileBuffer_Lock(self)) == 0) {
  result = FileBuffer_SetvbufUnlocked(self,buffer,modes,n);
  FileBuffer_Unlock(self);
 }
 return result;
}

CRT_STDIO int LIBCCALL
FileBuffer_Getc(FileBuffer *__restrict self) {
 int result;
 STATIC_ASSERT(EOF == -1);
 if ((result = FileBuffer_Lock(self)) == 0) {
  result = FileBuffer_GetcUnlocked(self);
  FileBuffer_Unlock(self);
 }
 return result;
}
CRT_STDIO int LIBCCALL
FileBuffer_Ungetc(FileBuffer *__restrict self, int ch) {
 int result;
 if ((result = FileBuffer_Lock(self)) == 0) {
  result = FileBuffer_UngetcUnlocked(self,ch);
  FileBuffer_Unlock(self);
 }
 return result;
}

CRT_STDIO int LIBCCALL
FileBuffer_Fill(FileBuffer *__restrict self) {
 int result;
 if ((result = FileBuffer_Lock(self)) == 0) {
  result = FileBuffer_FillUnlocked(self);
  FileBuffer_Unlock(self);
 }
 return result;
}


/* Redirect API functions that map one-on-one */
DEFINE_INTERN_ALIAS(libc_fseeko64,FileBuffer_Seek);
DEFINE_INTERN_ALIAS(libc_fseeko64_unlocked,FileBuffer_SeekUnlocked);
DEFINE_INTERN_ALIAS(libc_Xfseeko64,FileBuffer_XSeek);
DEFINE_INTERN_ALIAS(libc_Xfseeko64_unlocked,FileBuffer_XSeekUnlocked);
EXPORT(fseeko64,libc_fseeko64);
EXPORT(fseeko64_unlocked,libc_fseeko64_unlocked);
EXPORT(Xfseeko64,libc_Xfseeko64);
EXPORT(Xfseeko64_unlocked,libc_Xfseeko64_unlocked);
EXPORT(__DSYM(_fseeki64),libc_fseeko64);
EXPORT(__DSYM(_fseeki64_nolock),libc_fseeko64_unlocked);

DEFINE_INTERN_ALIAS(libc_ftello64,FileBuffer_Tell);
DEFINE_INTERN_ALIAS(libc_ftello64_unlocked,FileBuffer_TellUnlocked);
DEFINE_INTERN_ALIAS(libc_Xftello64,FileBuffer_XTell);
DEFINE_INTERN_ALIAS(libc_Xftello64_unlocked,FileBuffer_TellUnlocked);
EXPORT(ftello64,libc_ftello64);
EXPORT(ftello64_unlocked,libc_ftello64_unlocked);
EXPORT(Xftello64,libc_Xftello64);
EXPORT(Xftello64_unlocked,libc_Xftello64_unlocked);
EXPORT(__DSYM(_ftelli64),libc_ftello64);
EXPORT(__DSYM(_ftelli64_nolock),libc_ftello64_unlocked);

EXPORT(fseeko,libc_fseeko);
CRT_STDIO_API int LIBCCALL
libc_fseeko(FILE *__restrict self, off32_t off, int whence) {
 return FileBuffer_Seek(self,off,whence);
}
EXPORT(Xfseeko,libc_Xfseeko);
CRT_STDIO_XAPI void LIBCCALL
libc_Xfseeko(FILE *__restrict self, off32_t off, int whence) {
 FileBuffer_XSeek(self,off,whence);
}
EXPORT(__DSYM(_fseek_nolock),libc_fseeko_unlocked);
EXPORT(fseeko_unlocked,libc_fseeko_unlocked);
CRT_STDIO_API int LIBCCALL
libc_fseeko_unlocked(FILE *__restrict self, off32_t off, int whence) {
 return FileBuffer_SeekUnlocked(self,off,whence);
}
EXPORT(Xfseeko_unlocked,libc_Xfseeko_unlocked);
CRT_STDIO_XAPI void LIBCCALL
libc_Xfseeko_unlocked(FILE *__restrict self, off32_t off, int whence) {
 FileBuffer_XSeekUnlocked(self,off,whence);
}


#ifdef LIBC_LIBCCALL_RET64_IS_RET32
DEFINE_INTERN_ALIAS(libc_ftello,FileBuffer_Tell);
DEFINE_INTERN_ALIAS(libc_Xftello,FileBuffer_XTell);
DEFINE_INTERN_ALIAS(libc_ftello_unlocked,FileBuffer_TellUnlocked);
#else
CRT_STDIO_API off32_t LIBCCALL
libc_ftello(FILE *__restrict self) {
 return (off32_t)FileBuffer_Tell(self);
}
CRT_STDIO_XAPI pos32_t LIBCCALL
libc_Xftello(FILE *__restrict self) {
 return (pos32_t)FileBuffer_XTell(self);
}
CRT_STDIO_API off32_t LIBCCALL
libc_ftello_unlocked(FILE *__restrict self) {
 return FileBuffer_TellUnlocked(self);
}
#endif
DEFINE_INTERN_ALIAS(libc_Xftello_unlocked,libc_ftello_unlocked);
EXPORT(ftello,libc_ftello);
EXPORT(Xftello,libc_Xftello);
EXPORT(__DSYM(_ftell_nolock),libc_ftello_unlocked);
EXPORT(ftello_unlocked,libc_ftello_unlocked);
EXPORT(Xftello_unlocked,libc_Xftello_unlocked);


#if __SIZEOF_LONG__ > 4
DEFINE_INTERN_ALIAS(libc_fseek,FileBuffer_Seek);
DEFINE_INTERN_ALIAS(libc_Xfseek,FileBuffer_XSeek);
DEFINE_INTERN_ALIAS(libc_fseek_unlocked,FileBuffer_SeekUnlocked);
DEFINE_INTERN_ALIAS(libc_Xfseek_unlocked,FileBuffer_XSeekUnlocked);
DEFINE_INTERN_ALIAS(libc_ftell,FileBuffer_Tell);
DEFINE_INTERN_ALIAS(libc_Xftell,FileBuffer_XTell);
DEFINE_INTERN_ALIAS(libc_ftell_unlocked,FileBuffer_TellUnlocked);
DEFINE_INTERN_ALIAS(libc_Xftell_unlocked,FileBuffer_TellUnlocked);
#else
DEFINE_INTERN_ALIAS(libc_fseek,libc_fseeko);
DEFINE_INTERN_ALIAS(libc_Xfseek,libc_Xfseeko);
DEFINE_INTERN_ALIAS(libc_fseek_unlocked,libc_fseeko_unlocked);
DEFINE_INTERN_ALIAS(libc_Xfseek_unlocked,libc_Xfseeko_unlocked);
DEFINE_INTERN_ALIAS(libc_ftell,libc_ftello);
DEFINE_INTERN_ALIAS(libc_Xftell,libc_Xftello);
DEFINE_INTERN_ALIAS(libc_ftell_unlocked,libc_ftello_unlocked);
DEFINE_INTERN_ALIAS(libc_Xftell_unlocked,libc_Xftello_unlocked);
#endif
EXPORT(fseek,libc_fseek);
EXPORT(Xfseek,libc_Xfseek);
EXPORT(fseek_unlocked,libc_fseek_unlocked);
EXPORT(Xfseek_unlocked,libc_Xfseek_unlocked);
EXPORT(ftell,libc_ftell);
EXPORT(Xftell,libc_Xftell);
EXPORT(ftell_unlocked,libc_ftell_unlocked);
EXPORT(Xftell_unlocked,libc_Xftell_unlocked);


#undef getc
#undef Xgetc
#undef getc_unlocked
#undef Xgetc_unlocked

EXPORT(getc,libc_fgetc);
EXPORT(fgetc,libc_fgetc);
EXPORT(Xgetc,libc_Xfgetc);
EXPORT(Xfgetc,libc_Xfgetc);
DEFINE_INTERN_ALIAS(libc_fgetc,FileBuffer_Getc);
DEFINE_INTERN_ALIAS(libc_Xfgetc,FileBuffer_XGetc);

EXPORT(getc_unlocked,libc_fgetc_unlocked);
EXPORT(fgetc_unlocked,libc_fgetc_unlocked);
EXPORT(Xgetc_unlocked,libc_Xfgetc_unlocked);
EXPORT(Xfgetc_unlocked,libc_Xfgetc_unlocked);
EXPORT(__DSYM(_getc_nolock),libc_fgetc_unlocked);
DEFINE_INTERN_ALIAS(libc_fgetc_unlocked,FileBuffer_GetcUnlocked);
DEFINE_INTERN_ALIAS(libc_Xfgetc_unlocked,FileBuffer_XGetcUnlocked);



EXPORT(setvbuf,libc_setvbuf);
EXPORT(Xsetvbuf,libc_Xsetvbuf);
EXPORT(setvbuf_unlocked,libc_setvbuf_unlocked);
EXPORT(Xsetvbuf_unlocked,libc_Xsetvbuf_unlocked);
DEFINE_INTERN_ALIAS(libc_setvbuf,FileBuffer_Setvbuf);
DEFINE_INTERN_ALIAS(libc_Xsetvbuf,FileBuffer_XSetvbuf);
DEFINE_INTERN_ALIAS(libc_setvbuf_unlocked,FileBuffer_SetvbufUnlocked);
DEFINE_INTERN_ALIAS(libc_Xsetvbuf_unlocked,FileBuffer_XSetvbufUnlocked);



DECL_END
#endif /* CONFIG_LIBC_USES_NEW_STDIO */

#endif /* !GUARD_LIBS_LIBC_STDIO_C */
