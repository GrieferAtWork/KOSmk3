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
#ifndef GUARD_KERNEL_SRC_FS_PIPE_C
#define GUARD_KERNEL_SRC_FS_PIPE_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/timespec.h>
#include <kernel/sections.h>
#include <kernel/malloc.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <fs/pipe.h>
#include <fs/handle.h>
#include <fcntl.h>
#include <except.h>
#include <string.h>
#include <bits/poll.h>
#include <sys/stat.h>

DECL_BEGIN

PUBLIC ATTR_NOTHROW void KCALL
pipe_destroy(struct pipe *__restrict self) {
 ringbuffer_fini(&self->p_buffer);
 kfree(self);
}

PUBLIC ATTR_NOTHROW void KCALL
pipereader_destroy(struct pipereader *__restrict self) {
 /* Close the pipe buffer. */
 ringbuffer_close(&self->pr_pipe->p_buffer);
 pipe_decref(self->pr_pipe);
 kfree(self);
}
#ifdef CONFIG_PIPEWRITER_MATCHES_PIPEREADER
DEFINE_PUBLIC_ALIAS(pipewriter_destroy,pipereader_destroy);
#else
PUBLIC ATTR_NOTHROW void KCALL
pipewriter_destroy(struct pipewriter *__restrict self) {
 /* Close the pipe buffer. */
 ringbuffer_close(&self->pw_pipe->p_buffer);
 pipe_decref(self->pw_pipe);
 kfree(self);
}
#endif


PUBLIC ATTR_RETNONNULL REF struct pipe *
KCALL pipe_alloc(size_t max_size) {
 REF struct pipe *result;
 result = (REF struct pipe *)kmalloc(sizeof(struct pipe),
                                     GFP_SHARED);
 ringbuffer_init(&result->p_buffer,max_size);
 result->p_refcnt = 1;
 return result;
}

PUBLIC ATTR_RETNONNULL REF struct pipereader *KCALL
pipereader_alloc(struct pipe *__restrict p) {
 REF struct pipereader *result;
 result = (REF struct pipereader *)kmalloc(sizeof(struct pipereader),
                                           GFP_SHARED);
 result->pr_refcnt = 1;
 result->pr_pipe   = p;
 pipe_incref(p);
 return result;
}
#ifdef CONFIG_PIPEWRITER_MATCHES_PIPEREADER
DEFINE_PUBLIC_ALIAS(pipewriter_alloc,pipereader_alloc);
#else
PUBLIC ATTR_RETNONNULL REF struct pipewriter *KCALL
pipewriter_alloc(struct pipe *__restrict p) {
 REF struct pipewriter *result;
 result = (REF struct pipewriter *)kmalloc(sizeof(struct pipewriter),
                                           GFP_SHARED);
 result->pw_refcnt = 1;
 result->pw_pipe   = p;
 pipe_incref(p);
 return result;
}
#endif




/* PIPE Handle operators. */
INTERN size_t KCALL
handle_pipe_read(struct pipe *__restrict self,
                 USER CHECKED void *buf,
                 size_t bufsize, iomode_t flags) {
 return ringbuffer_read(&self->p_buffer,buf,bufsize,flags);
}
INTERN size_t KCALL
handle_pipe_write(struct pipe *__restrict self,
                  USER CHECKED void const *buf,
                  size_t bufsize, iomode_t flags) {
 return ringbuffer_write(&self->p_buffer,buf,bufsize,flags);
}
INTERN size_t KCALL
handle_pipereader_read(struct pipereader *__restrict self,
                       USER CHECKED void *buf,
                       size_t bufsize, iomode_t flags) {
 return ringbuffer_read(&self->pr_pipe->p_buffer,buf,bufsize,flags);
}
INTERN size_t KCALL
handle_pipewriter_write(struct pipewriter *__restrict self,
                        USER CHECKED void const *buf,
                        size_t bufsize, iomode_t flags) {
 return ringbuffer_write(&self->pw_pipe->p_buffer,buf,bufsize,flags);
}
INTERN pos_t KCALL
handle_pipereader_seek(struct pipereader *__restrict self,
                       off_t off, int whence) {
 if (whence != SEEK_CUR)
     error_throw(E_INVALID_ARGUMENT);
 /* EXTENSION: `seek(42,SEEK_CUR)' -> DISCARD(42) (returns the number of discarded bytes) */
 if (off >= 0)
     return ringbuffer_discard(&self->pr_pipe->p_buffer,(size_t)off);
 /* EXTENSION: `seek(-42,SEEK_CUR)' -> UNREAD(42) (returns the number of unread bytes) */
 return ringbuffer_unread(&self->pr_pipe->p_buffer,(size_t)-off);
}
INTERN pos_t KCALL
handle_pipewriter_seek(struct pipewriter *__restrict self,
                       off_t off, int whence) {
 if (whence != SEEK_CUR || off > 0)
     error_throw(E_INVALID_ARGUMENT);
 /* EXTENSION: `seek(-42,SEEK_CUR)' -> UNWRITE(42) (returns the number of unwritten bytes) */
 return ringbuffer_unwrite(&self->pw_pipe->p_buffer,(size_t)-off);
}
INTERN void KCALL
handle_pipe_stat(struct pipe *__restrict self,
                 USER CHECKED struct stat64 *result) {
 size_t written_size; byte_t *rptr;
 atomic_rwlock_read(&self->p_buffer.r_lock);
 rptr = ATOMIC_READ(self->p_buffer.r_rptr);
 if (self->p_buffer.r_wptr > rptr)
  written_size = self->p_buffer.r_wptr-rptr;
 else {
  written_size = self->p_buffer.r_size-(rptr-self->p_buffer.r_wptr);
 }
 atomic_rwlock_endread(&self->p_buffer.r_lock);
 COMPILER_BARRIER();
 memset(result,0,sizeof(struct stat64));
 result->st_size = written_size;
}
INTERN void KCALL
handle_pipereader_stat(struct pipereader *__restrict self,
                       USER CHECKED struct stat64 *result) {
 handle_pipe_stat(self->pr_pipe,result);
}
#ifdef CONFIG_PIPEWRITER_MATCHES_PIPEREADER
DEFINE_INTERN_ALIAS(handle_pipewriter_stat,handle_pipereader_stat);
#else
INTERN void KCALL
handle_pipewriter_stat(struct pipewriter *__restrict self,
                       USER CHECKED struct stat64 *result) {
 handle_pipe_stat(self->pw_pipe,result);
}
#endif
INTERN unsigned int KCALL
handle_pipe_poll(struct pipe *__restrict self,
                 unsigned int mode) {
 return ringbuffer_poll(&self->p_buffer,mode);
}
INTERN unsigned int KCALL
handle_pipereader_poll(struct pipereader *__restrict self,
                       unsigned int mode) {
 return ringbuffer_poll(&self->pr_pipe->p_buffer,mode & POLLIN);
}
INTERN unsigned int KCALL
handle_pipewriter_poll(struct pipewriter *__restrict self,
                       unsigned int mode) {
 return ringbuffer_poll(&self->pw_pipe->p_buffer,mode & POLLOUT);
}

union PACKED pipefd {
    u64     pf_64;
    struct PACKED {
        u32 pf_reader;
        u32 pf_writer;
    };
};

DEFINE_SYSCALL1_64(xpipe,oflag_t,flags) {
 union pipefd result; REF struct pipe *p;
 struct handle EXCEPT_VAR hreader;
 struct handle EXCEPT_VAR hwriter;
 /* Setup handles. */
 hreader.h_mode = HANDLE_MODE(HANDLE_TYPE_FPIPEREADER,IO_RDONLY);
 hwriter.h_mode = HANDLE_MODE(HANDLE_TYPE_FPIPEWRITER,IO_WRONLY);
 hreader.h_flag |= IO_HANDLE_FFROM_O(flags);
 hwriter.h_flag |= IO_HANDLE_FFROM_O(flags);
 /* Construct a new pipe. */
 p = pipe_alloc(CONFIG_PIPE_DEFAULT_LIMIT);
 TRY {
  hreader.h_object.o_pipereader = pipereader_alloc(p);
  TRY {
   hwriter.h_object.o_pipewriter = pipewriter_alloc(p);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   pipereader_decref(hreader.h_object.o_pipereader);
   error_rethrow();
  }
 } FINALLY {
  pipe_decref(p);
 }
 /* Register the new handles. */
 TRY {
  result.pf_reader = (u32)handle_put(hreader);
  TRY {
   result.pf_writer = (u32)handle_put(hwriter);
  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   handle_close(result.pf_reader);
   error_rethrow();
  }
 } FINALLY {
  pipewriter_decref(hwriter.h_object.o_pipewriter);
  pipereader_decref(hreader.h_object.o_pipereader);
 }
 /* Return the file descriptor pair. */
 return result.pf_64;
}


DEFINE_SYSCALL2(pipe2,USER UNCHECKED int *,pipefd,oflag_t,flags) {
 union pipefd EXCEPT_VAR p;
 validate_writable(pipefd,2*sizeof(int));
 p.pf_64 = SYSC_xpipe(flags);
 TRY {
  COMPILER_BARRIER();
  /* Copy the pipe handles to user-space. */
  pipefd[0] = p.pf_reader;
  pipefd[1] = p.pf_writer;
  COMPILER_WRITE_BARRIER();
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Deal with SEGFAULTs */
  handle_close(p.pf_reader);
  handle_close(p.pf_writer);
  error_rethrow();
 }
 return 0;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_PIPE_C */
