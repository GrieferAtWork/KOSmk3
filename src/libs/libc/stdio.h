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
#ifndef GUARD_LIBS_LIBC_STDIO_H
#define GUARD_LIBS_LIBC_STDIO_H 1

#include "libc.h"
#include "sync.h"
#include <bits/io-file.h>
#include <bits-generic/io-file.h>
#include <hybrid/atomic.h>
#include <hybrid/list/list.h>
#include <kos/sched/mutex.h>

#undef CONFIG_LIBC_USES_NEW_STDIO
#define CONFIG_LIBC_USES_NEW_STDIO 1

#ifndef CONFIG_LIBC_USES_NEW_STDIO
#include "stdio/file.h"
#else
DECL_BEGIN

#ifndef __std_FILE_defined
#define __std_FILE_defined 1
__NAMESPACE_STD_BEGIN
typedef __FILE FILE;
__NAMESPACE_STD_END
#endif /* !__std_FILE_defined */
#ifndef __FILE_defined
#define __FILE_defined 1
__NAMESPACE_STD_USING(FILE)
#endif /* !__FILE_defined */


typedef ssize_t LIBCCALL cookie_read_function_t(void *cookie, char *buffer, size_t bufsize);
typedef ssize_t LIBCCALL cookie_write_function_t(void *cookie, char const *buffer, size_t bufsize);
typedef int LIBCCALL cookie_seek_function_t(void *cookie, off64_t *offset, int whence);
typedef int LIBCCALL cookie_close_function_t(void *cookie);
typedef struct {
    cookie_read_function_t  *cio_read;  /* [0..1] */
    cookie_write_function_t *cio_write; /* [0..1] */
    cookie_seek_function_t  *cio_seek;  /* [0..1] */
    cookie_close_function_t *cio_close; /* [0..1] */
} cookie_io_functions_t;

#define FILE_BUFFER_FNORMAL     0x00000000          /* Normal buffer flags. */
#define FILE_BUFFER_FEOF        __IO_FILE_IOEOF     /* Set when the file pointed to by 'fb_file' has been exhausted. */
#define FILE_BUFFER_FERR        __IO_FILE_IOERR     /* Set when an I/O error occurred. */
#define FILE_BUFFER_FREADONLY   0x00010000          /* The buffer can only be used for reading. */
#define FILE_BUFFER_FNODYNSCALE 0x00020000          /* The buffer is allowed to dynamically change its buffer size. */
#define FILE_BUFFER_FLNBUF      __IO_FILE_IOLNBUF   /* The buffer is line-buffered, meaning that it will
                                                     * flush its data whenever a line-feed is printed.
                                                     * Additionally if the `FILE_BUFFER_FISATTY' flag is set,
                                                     * attempting to read from a line-buffered file will cause
                                                     * all other existing line-buffered files to be synchronized
                                                     * first. This is done to ensure that interactive files are
                                                     * always up-to-date before data is read from one of them. */
#define FILE_BUFFER_FSYNC       0x00080000          /* Also synchronize the underlying file after flushing the buffer. */
#define FILE_BUFFER_FUTF16LS    0x04000000          /* The buffer has a pending UTF-16 low surrogate that should be returned the next time `getwchar16()' and friends are called.
                                                     * Alternatively (when writing), the buffer has a pending UTF-16 high surrogate that will be completed by the next UTF-16 character written. */
#define FILE_BUFFER_FREADING    0x08000000          /* The buffer is currently being read into and must not be changed or resized. */
#define FILE_BUFFER_FNOTATTY    0x10000000          /* This buffer does not refer to a TTY device. */
#define FILE_BUFFER_FISATTY     0x20000000          /* This buffer refers to a TTY device. */
#define FILE_BUFFER_FSTATICBUF  __IO_FILE_IOUSERBUF /* Implies `FILE_BUFFER_FNODYNSCALE': When set, the
                                                     * buffer doesn't actually own its buffer and must not
                                                     * attempt to free() it during destruction.
                                                     * The `FILE_BUFFER_FNODYNSCALE' is implied to prevent the
                                                     * buffer from attempting to resize (realloc) itself dynamically. */
#define FILE_BUFFER_FSTATIC     0x40000000          /* The FILE structure and its extended data field are statically allocated. */
#define FILE_BUFFER_FLNIFTTY    __IO_FILE_IOLNIFTYY /* Automatically set/delete the `FILE_BUFFER_FLNBUF' and
                                                     * `FILE_BUFFER_FISATTY' flags, and add/remove the file from
                                                     * `fb_ttych' the next this it comes into question. To determine
                                                     * this, the pointed-to file is tested for being a TTY device
                                                     * using `isatty(fb_file)'.
                                                     * HINT: This flag is set for all newly created buffers by default. */


#ifdef __INTELLISENSE__
__NAMESPACE_STD_BEGIN
struct __IO_FILE
#else
typedef FILE               FileBuffer;
struct iofile_data
#endif
{
#ifdef __INTELLISENSE__
    uintptr_t              fb_zero;  /* Always ZERO(0). - Required for binary compatibility with DOS. */
    ATOMIC_DATA uintptr_t  fb_refcnt;/* Reference counter. */
    mutex_t                fb_lock;  /* Lock for this file. */
    cookie_io_functions_t  fb_ops;   /* [const] File operator callbacks. */
    void                  *fb_arg;   /* [?..?][const] File operator argument. */
    byte_t                *fb_chng;  /* [>= fb_base][+fb_chsz <= fb_base+fb_size]
                                      * [valid_if(fb_chsz != 0)][lock(fb_lock)]
                                      *  Pointer to the first character that was
                                      *  changed since the buffer had been loaded. */
    size_t                 fb_chsz;  /* [lock(fb_lock)] Amount of bytes that were changed. */
    LIST_NODE(FileBuffer)  fb_ttych; /* Chain of changed TTY file buffers (buffers that are used with an interactive file).
                                      * Any buffer that is connected to an interactive device is flushed before
                                      * data is read from any other interactive device.
                                      * This chain is weakly linked in that buffer objects remove themself
                                      * before destruction, also meaning that any buffer contained in this
                                      * chain may have a reference counter to ZERO(0). */
    LIST_NODE(FileBuffer)  fb_files; /* Chain of all known file buffers (flushed during atexit(), and `fflushall()' / `fflush(NULL)'). */
    pos64_t                fb_fblk;  /* The starting address of the data block currently stored in `fb_base'. */
    pos64_t                fb_fpos;  /* The current (assumed) position within `fb_file'. */
    u16                    fb_utf16ls; /* [lock(fb_lock)][valid_if(FILE_BUFFER_FUTF16LS)] The pending utf-16 low surrogate. */

    int                    fb_file;  /* [lock(fb_lock)] The file referenced by this buffer. */
    byte_t                *fb_ptr;   /* [>= fb_base][+fb_cnt <= fb_base+fb_size][lock(fb_lock)]
                                      *  Pointer to the next character to-be read/written.
                                      *  The absolute in-file position is then `fb_fblk+(fb_ptr-fb_base)' */
    size_t                 fb_cnt;   /* [lock(fb_lock)] The amount of unread, buffered bytes located at `fb_ptr'. */
    byte_t                *fb_base;  /* [0..fb_size][owned_if(!FILE_BUFFER_FSTATICBUF)][lock(fb_lock)] Allocated buffer.
                                      * NOTE: This pointer must not be modified when `FILE_BUFFER_FREADING' is set. */
    size_t                 fb_size;  /* [lock(fb_lock)] Total allocated / available buffer size.
                                      * NOTE: This pointer must not be modified when `FILE_BUFFER_FREADING' is set. */
    u32                    fb_flag;  /* [lock(fb_lock)] The current state of the buffer. */
#else
    uintptr_t            __fb_zero;  /* Always ZERO(0). - Required for binary compatibility with DOS. */
    uintptr_t            __fb_refcnt;/* Reference counter. */
    mutex_t              __fb_lock;  /* Lock for this file. */
    cookie_io_functions_t __fb_ops;  /* [const] File operator callbacks. */
    void                *__fb_arg;   /* [?..?][const] File operator argument. */
    byte_t              *__fb_chng;  /* [>= fb_base][+fb_chsz <= fb_base+fb_size]
                                      * [valid_if(fb_chsz != 0)][lock(fb_lock)]
                                      *  Pointer to the first character that was
                                      *  changed since the buffer had been loaded. */
    size_t               __fb_chsz;  /* [lock(fb_lock)] Amount of bytes that were changed. */
    LIST_NODE(FileBuffer) __fb_ttych;/* Chain of changed TTY file buffers (buffers that are used with an interactive file).
                                      *  Any buffer that is connected to an interactive device is flushed before
                                      *  data is read from any other interactive device.
                                      *  This chain is weakly linked in that buffer objects remove themself
                                      *  before destruction, also meaning that any buffer contained in this
                                      *  chain may have a reference counter to ZERO(0). */
    LIST_NODE(FileBuffer) __fb_files;/* Chain of all known file buffers (flushed during atexit(), and `fflushall()' / `fflush(NULL)'). */
    pos64_t              __fb_fblk;  /* The starting address of the data block currently stored in `fb_base'. */
    pos64_t              __fb_fpos;  /* The current (assumed) position within `fb_file'. */
    u16                  __fb_utf16ls; /* [lock(fb_lock)][valid_if(FILE_BUFFER_FUTF16LS)] The pending utf-16 low surrogate. */
#define fb_zero          if_exdata->__fb_zero
#define fb_refcnt        if_exdata->__fb_refcnt
#define fb_lock          if_exdata->__fb_lock
#define fb_ops           if_exdata->__fb_ops
#define fb_arg           if_exdata->__fb_arg
#define fb_chng          if_exdata->__fb_chng
#define fb_chsz          if_exdata->__fb_chsz
#define fb_ttych         if_exdata->__fb_ttych
#define fb_files         if_exdata->__fb_files
#define fb_fblk          if_exdata->__fb_fblk
#define fb_fpos          if_exdata->__fb_fpos
#define fb_utf16ls       if_exdata->__fb_utf16ls
#define fb_file          if_fd
#define fb_ptr           if_ptr
#define fb_cnt           if_cnt
#define fb_base          if_base
#define fb_size          if_bufsiz
#define fb_flag          if_flag
#endif
};

#ifdef __INTELLISENSE__
typedef struct __IO_FILE FileBuffer;
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(FileBuffer)
#endif


#define FileBuffer_Incref(self)  ATOMIC_FETCHINC((self)->fb_refcnt)
#define FileBuffer_Decref(self) (ATOMIC_DECFETCH((self)->fb_refcnt) ? 0 : FileBuffer_Destroy(self))

#define FILE_BUFSIZ_MAX                8192 /* The max size to which the buffer may grow. */
#define FILE_BUFSIZ_MIN                512  /* The default size when no dynamic buffer was allocated before. */
#define FILE_BUFSIZ_RELOCATE_THRESHOLD 2048 /* When >= this amount of bytes are unused in the buffer, shrink the buffer. */

#define FileBuffer_HasCookie(self) ((self)->fb_ops.cio_close)
#define FileBuffer_Lock(self)         libc_mutex_get_timed64(&(self)->fb_lock,NULL)
#define FileBuffer_XLock(self)  (void)libc_Xmutex_get_timed64(&(self)->fb_lock,NULL)
#define FileBuffer_Unlock(self)       libc_mutex_put(&(self)->fb_lock)

/* FileBuffer allocation / destruction. */
INTDEF int LIBCCALL FileBuffer_Destroy(FileBuffer *__restrict self);
INTDEF ATTR_MALLOC FileBuffer *LIBCCALL FileBuffer_Alloc(void);
INTDEF ATTR_MALLOC ATTR_RETNONNULL FileBuffer *LIBCCALL FileBuffer_XAlloc(void);
INTDEF void LIBCCALL FileBuffer_Free(FileBuffer *__restrict self);

/* FileBuffer I/O Helper functions. */
INTDEF void LIBCCALL FileBuffer_XCheckTTY(FileBuffer *__restrict self);
INTDEF void LIBCCALL FileBuffer_FlushTTYLineBuffers(FileBuffer *sender);
INTDEF void LIBCCALL FileBuffer_FlushAllBuffers(void);
INTDEF void LIBCCALL FileBuffer_FlushAllBuffersUnlocked(void);
INTDEF void LIBCCALL FileBuffer_AddChangedTTY(FileBuffer *__restrict self);
INTDEF void LIBCCALL FileBuffer_RemoveChangedTTY(FileBuffer *__restrict self);
INTDEF void LIBCCALL FileBuffer_Register(FileBuffer *__restrict self);
INTDEF void LIBCCALL FileBuffer_Unregister(FileBuffer *__restrict self);

/* FileBuffer I/O Core functions. */
INTDEF size_t LIBCCALL FileBuffer_ReadUnlocked(FileBuffer *__restrict self, void *__restrict buffer, size_t bufsize);
INTDEF size_t LIBCCALL FileBuffer_WriteUnlocked(FileBuffer *__restrict self, void const *__restrict buffer, size_t bufsize);
INTDEF size_t LIBCCALL FileBuffer_XReadUnlocked(FileBuffer *__restrict self, void *__restrict buffer, size_t bufsize);
INTDEF size_t LIBCCALL FileBuffer_XWriteUnlocked(FileBuffer *__restrict self, void const *__restrict buffer, size_t bufsize);
INTDEF int LIBCCALL FileBuffer_FlushUnlocked(FileBuffer *__restrict self);
INTDEF void LIBCCALL FileBuffer_XFlushUnlocked(FileBuffer *__restrict self);
INTDEF pos64_t LIBCCALL FileBuffer_TellUnlocked(FileBuffer *__restrict self);
INTDEF int LIBCCALL FileBuffer_SeekUnlocked(FileBuffer *__restrict self, off64_t off, int whence);
INTDEF void LIBCCALL FileBuffer_XSeekUnlocked(FileBuffer *__restrict self, off64_t off, int whence);
INTDEF int LIBCCALL FileBuffer_SetvbufUnlocked(FileBuffer *__restrict self, char *__restrict buffer, int modes, size_t n);
INTDEF void LIBCCALL FileBuffer_XSetvbufUnlocked(FileBuffer *__restrict self, char *__restrict buffer, int modes, size_t n);
INTDEF int LIBCCALL FileBuffer_GetcUnlocked(FileBuffer *__restrict self);
INTDEF int LIBCCALL FileBuffer_XGetcUnlocked(FileBuffer *__restrict self);
INTDEF int LIBCCALL FileBuffer_UngetcUnlocked(FileBuffer *__restrict self, int ch);
INTDEF int LIBCCALL FileBuffer_XUngetcUnlocked(FileBuffer *__restrict self, int ch);
INTDEF size_t LIBCCALL FileBuffer_Read(FileBuffer *__restrict self, void *__restrict buffer, size_t bufsize);
INTDEF size_t LIBCCALL FileBuffer_Write(FileBuffer *__restrict self, void const *__restrict buffer, size_t bufsize);
INTDEF size_t LIBCCALL FileBuffer_XRead(FileBuffer *__restrict self, void *__restrict buffer, size_t bufsize);
INTDEF size_t LIBCCALL FileBuffer_XWrite(FileBuffer *__restrict self, void const *__restrict buffer, size_t bufsize);
INTDEF int LIBCCALL FileBuffer_Flush(FileBuffer *__restrict self);
INTDEF void LIBCCALL FileBuffer_XFlush(FileBuffer *__restrict self);
INTDEF off64_t LIBCCALL FileBuffer_Tell(FileBuffer *__restrict self);
INTDEF pos64_t LIBCCALL FileBuffer_XTell(FileBuffer *__restrict self);
INTDEF int LIBCCALL FileBuffer_Seek(FileBuffer *__restrict self, off64_t off, int whence);
INTDEF void LIBCCALL FileBuffer_XSeek(FileBuffer *__restrict self, off64_t off, int whence);
INTDEF int LIBCCALL FileBuffer_Setvbuf(FileBuffer *__restrict self, char *__restrict buffer, int modes, size_t n);
INTDEF void LIBCCALL FileBuffer_XSetvbuf(FileBuffer *__restrict self, char *__restrict buffer, int modes, size_t n);
INTDEF int LIBCCALL FileBuffer_Getc(FileBuffer *__restrict self);
INTDEF int LIBCCALL FileBuffer_XGetc(FileBuffer *__restrict self);
INTDEF int LIBCCALL FileBuffer_Ungetc(FileBuffer *__restrict self, int ch);
INTDEF int LIBCCALL FileBuffer_XUngetc(FileBuffer *__restrict self, int ch);

/* Try to fill unused buffer memory with new data,
 * allocating a new buffer if none was available before. */
INTDEF int LIBCCALL FileBuffer_FillUnlocked(FileBuffer *__restrict self);
INTDEF int LIBCCALL FileBuffer_XFillUnlocked(FileBuffer *__restrict self);
INTDEF int LIBCCALL FileBuffer_Fill(FileBuffer *__restrict self);
INTDEF int LIBCCALL FileBuffer_XFill(FileBuffer *__restrict self);
INTDEF ATTR_NOTHROW void LIBCCALL FileBuffer_SetError(FileBuffer *__restrict self);
INTDEF size_t LIBCCALL FileBuffer_SystemRead(FileBuffer *__restrict self, void *buffer, size_t bufsize);
INTDEF size_t LIBCCALL FileBuffer_SystemWrite(FileBuffer *__restrict self, void const *buffer, size_t bufsize);
INTDEF pos64_t LIBCCALL FileBuffer_SystemSeek(FileBuffer *__restrict self, off64_t offset, int whence);
INTDEF int LIBCCALL FileBuffer_SystemClose(FileBuffer *__restrict self);



/* File operators. */
INTDEF size_t LIBCCALL libc_fread_unlocked(void *__restrict buf, size_t size, size_t n, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_Xfread_unlocked(void *__restrict buf, size_t size, size_t n, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_fwrite_unlocked(void const *__restrict buf, size_t size, size_t n, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_Xfwrite_unlocked(void const *__restrict buf, size_t size, size_t n, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_fread(void *__restrict buf, size_t size, size_t n, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_Xfread(void *__restrict buf, size_t size, size_t n, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_fwrite(void const *__restrict buf, size_t size, size_t n, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_Xfwrite(void const *__restrict buf, size_t size, size_t n, FILE *__restrict self);
INTDEF ssize_t LIBCCALL libc_file_printer(char const *__restrict data, size_t datalen, void *closure);
INTDEF ssize_t LIBCCALL libc_Xfile_printer(char const *__restrict data, size_t datalen, void *closure);
INTDEF void LIBCCALL libc_flockfile(FILE *__restrict self);
INTDEF void LIBCCALL libc_Xflockfile(FILE *__restrict self);
INTDEF int LIBCCALL libc_ftrylockfile(FILE *__restrict self);
INTDEF void LIBCCALL libc_funlockfile(FILE *__restrict self);
INTDEF int LIBCCALL libc_fseek(FILE *__restrict self, long int off, int whence);
INTDEF int LIBCCALL libc_fseek_unlocked(FILE *__restrict self, long int off, int whence);
INTDEF void LIBCCALL libc_Xfseek(FILE *__restrict self, long int off, int whence);
INTDEF void LIBCCALL libc_Xfseek_unlocked(FILE *__restrict self, long int off, int whence);
INTDEF int LIBCCALL libc_fseeko(FILE *__restrict self, off32_t off, int whence);
INTDEF int LIBCCALL libc_fseeko_unlocked(FILE *__restrict self, off32_t off, int whence);
INTDEF void LIBCCALL libc_Xfseeko(FILE *__restrict self, off32_t off, int whence);
INTDEF void LIBCCALL libc_Xfseeko_unlocked(FILE *__restrict self, off32_t off, int whence);
INTDEF int LIBCCALL libc_fseeko64(FILE *__restrict self, off64_t off, int whence);
INTDEF int LIBCCALL libc_fseeko64_unlocked(FILE *__restrict self, off64_t off, int whence);
INTDEF void LIBCCALL libc_Xfseeko64(FILE *__restrict self, off64_t off, int whence);
INTDEF void LIBCCALL libc_Xfseeko64_unlocked(FILE *__restrict self, off64_t off, int whence);
INTDEF long int LIBCCALL libc_ftell(FILE *__restrict self);
INTDEF long int LIBCCALL libc_ftell_unlocked(FILE *__restrict self);
INTDEF unsigned long int LIBCCALL libc_Xftell(FILE *__restrict self);
INTDEF unsigned long int LIBCCALL libc_Xftell_unlocked(FILE *__restrict self);
INTDEF off32_t LIBCCALL libc_ftello(FILE *__restrict self);
INTDEF off32_t LIBCCALL libc_ftello_unlocked(FILE *__restrict self);
INTDEF pos32_t LIBCCALL libc_Xftello(FILE *__restrict self);
INTDEF pos32_t LIBCCALL libc_Xftello_unlocked(FILE *__restrict self);
INTDEF off64_t LIBCCALL libc_ftello64(FILE *__restrict self);
INTDEF off64_t LIBCCALL libc_ftello64_unlocked(FILE *__restrict self);
INTDEF pos64_t LIBCCALL libc_Xftello64(FILE *__restrict self);
INTDEF pos64_t LIBCCALL libc_Xftello64_unlocked(FILE *__restrict self);
INTDEF int LIBCCALL libc_fgetpos(FILE *__restrict self, pos32_t *__restrict pos);
INTDEF int LIBCCALL libc_fgetpos_unlocked(FILE *__restrict self, pos32_t *__restrict pos);
INTDEF void LIBCCALL libc_Xfgetpos(FILE *__restrict self, pos32_t *__restrict pos);
INTDEF void LIBCCALL libc_Xfgetpos_unlocked(FILE *__restrict self, pos32_t *__restrict pos);
INTDEF int LIBCCALL libc_fsetpos(FILE *__restrict self, pos32_t const *__restrict pos);
INTDEF int LIBCCALL libc_fsetpos_unlocked(FILE *__restrict self, pos32_t const *__restrict pos);
INTDEF void LIBCCALL libc_Xfsetpos(FILE *__restrict self, pos32_t const *__restrict pos);
INTDEF void LIBCCALL libc_Xfsetpos_unlocked(FILE *__restrict self, pos32_t const *__restrict pos);
INTDEF int LIBCCALL libc_fgetpos64(FILE *__restrict self, pos64_t *__restrict pos);
INTDEF int LIBCCALL libc_fgetpos64_unlocked(FILE *__restrict self, pos64_t *__restrict pos);
INTDEF void LIBCCALL libc_Xfgetpos64(FILE *__restrict self, pos64_t *__restrict pos);
INTDEF void LIBCCALL libc_Xfgetpos64_unlocked(FILE *__restrict self, pos64_t *__restrict pos);
INTDEF int LIBCCALL libc_fsetpos64(FILE *__restrict self, pos64_t const *__restrict pos);
INTDEF int LIBCCALL libc_fsetpos64_unlocked(FILE *__restrict self, pos64_t const *__restrict pos);
INTDEF void LIBCCALL libc_Xfsetpos64(FILE *__restrict self, pos64_t const *__restrict pos);
INTDEF void LIBCCALL libc_Xfsetpos64_unlocked(FILE *__restrict self, pos64_t const *__restrict pos);

INTDEF oflag_t LIBCCALL libc_parsemode(char const *__restrict mode); /* @return: -1: Invalid mode (Does not set errno). */

INTDEF ATTR_MALLOC FILE *LIBCCALL libc_fopencookie(void *__restrict magic_cookie, char const *__restrict modes, cookie_io_functions_t io_funcs);
INTDEF ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL libc_Xfopencookie(void *__restrict magic_cookie, char const *__restrict modes, cookie_io_functions_t io_funcs);
INTDEF ATTR_MALLOC FILE *LIBCCALL libc_tmpfile(void);
INTDEF ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL libc_Xtmpfile(void);
INTDEF ATTR_MALLOC FILE *LIBCCALL libc_tmpfile64(void);
INTDEF ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL libc_Xtmpfile64(void);

INTDEF ATTR_MALLOC FILE *LIBCCALL libc_fopenat(fd_t dfd, char const *__restrict filename, char const *__restrict modes, atflag_t flags);
INTDEF ATTR_MALLOC FILE *LIBCCALL libc_dos_fopenat(fd_t dfd, char const *__restrict filename, char const *__restrict modes, atflag_t flags);
INTDEF ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL libc_Xfopenat(fd_t dfd, char const *__restrict filename, char const *__restrict modes, atflag_t flags);
INTDEF ATTR_MALLOC FILE *LIBCCALL libc_fopenat64(fd_t dfd, char const *__restrict filename, char const *__restrict modes, atflag_t flags);
INTDEF ATTR_MALLOC FILE *LIBCCALL libc_dos_fopenat64(fd_t dfd, char const *__restrict filename, char const *__restrict modes, atflag_t flags);
INTDEF ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL libc_Xfopenat64(fd_t dfd, char const *__restrict filename, char const *__restrict modes, atflag_t flags);
INTDEF ATTR_MALLOC FILE *LIBCCALL libc_fdopen(fd_t fd, char const *__restrict modes);
INTDEF ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL libc_Xfdopen(fd_t fd, char const *__restrict modes);
INTDEF ATTR_MALLOC FILE *LIBCCALL libc_fopen(char const *__restrict filename, char const *__restrict modes);
INTDEF ATTR_MALLOC FILE *LIBCCALL libc_dos_fopen(char const *__restrict filename, char const *__restrict modes);
INTDEF ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL libc_Xfopen(char const *__restrict filename, char const *__restrict modes);
INTDEF ATTR_MALLOC FILE *LIBCCALL libc_fopen64(char const *__restrict filename, char const *__restrict modes);
INTDEF ATTR_MALLOC FILE *LIBCCALL libc_dos_fopen64(char const *__restrict filename, char const *__restrict modes);
INTDEF ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL libc_Xfopen64(char const *__restrict filename, char const *__restrict modes);

INTDEF FILE *LIBCCALL libc_freopenat(fd_t dfd, char const *__restrict filename, char const *__restrict modes, atflag_t flags, FILE *__restrict self);
INTDEF FILE *LIBCCALL libc_dos_freopenat(fd_t dfd, char const *__restrict filename, char const *__restrict modes, atflag_t flags, FILE *__restrict self);
INTDEF ATTR_RETNONNULL FILE *LIBCCALL libc_Xfreopenat(fd_t dfd, char const *__restrict filename, char const *__restrict modes, atflag_t flags, FILE *__restrict self);
INTDEF FILE *LIBCCALL libc_freopenat64(fd_t dfd, char const *__restrict filename, char const *__restrict modes, atflag_t flags, FILE *__restrict self);
INTDEF FILE *LIBCCALL libc_dos_freopenat64(fd_t dfd, char const *__restrict filename, char const *__restrict modes, atflag_t flags, FILE *__restrict self);
INTDEF ATTR_RETNONNULL FILE *LIBCCALL libc_Xfreopenat64(fd_t dfd, char const *__restrict filename, char const *__restrict modes, atflag_t flags, FILE *__restrict self);

INTDEF FILE *LIBCCALL libc_fdreopen(fd_t fd, char const *__restrict modes, FILE *__restrict self, int mode);
INTDEF ATTR_RETNONNULL FILE *LIBCCALL libc_Xfdreopen(fd_t fd, char const *__restrict modes, FILE *__restrict self, int mode);
INTDEF FILE *LIBCCALL libc_freopen(char const *__restrict filename, char const *__restrict modes, FILE *__restrict self);
INTDEF FILE *LIBCCALL libc_dos_freopen(char const *__restrict filename, char const *__restrict modes, FILE *__restrict self);
INTDEF ATTR_RETNONNULL FILE *LIBCCALL libc_Xfreopen(char const *__restrict filename, char const *__restrict modes, FILE *__restrict self);
INTDEF FILE *LIBCCALL libc_freopen64(char const *__restrict filename, char const *__restrict modes, FILE *__restrict self);
INTDEF FILE *LIBCCALL libc_dos_freopen64(char const *__restrict filename, char const *__restrict modes, FILE *__restrict self);
INTDEF ATTR_RETNONNULL FILE *LIBCCALL libc_Xfreopen64(char const *__restrict filename, char const *__restrict modes, FILE *__restrict self);

INTDEF ATTR_MALLOC FILE *LIBCCALL libc_fmemopen(void *s, size_t len, char const *modes);
INTDEF ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL libc_Xfmemopen(void *s, size_t len, char const *modes);
INTDEF ATTR_MALLOC FILE *LIBCCALL libc_open_memstream(char **bufloc, size_t *sizeloc);
INTDEF ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL libc_Xopen_memstream(char **bufloc, size_t *sizeloc);
INTDEF ATTR_MALLOC FILE *LIBCCALL libc_popen(char const *command, char const *modes);
INTDEF ATTR_MALLOC ATTR_RETNONNULL FILE *LIBCCALL libc_Xpopen(char const *command, char const *modes);
INTDEF ATTR_MALLOC FILE *LIBCCALL libc_dos_popen(char const *command, char const *modes);

INTDEF int LIBCCALL libc_fclose(FILE *__restrict self);
INTDEF int LIBCCALL libc_pclose(FILE *__restrict self);
INTDEF int LIBCCALL libc_fcloseall(void);

INTDEF int LIBCCALL libc_fgetc(FILE *__restrict self);
INTDEF int LIBCCALL libc_Xfgetc(FILE *__restrict self);
INTDEF int LIBCCALL libc_fgetc_unlocked(FILE *__restrict self);
INTDEF int LIBCCALL libc_Xfgetc_unlocked(FILE *__restrict self);
INTDEF int LIBCCALL libc_fputc(int c, FILE *__restrict self);
INTDEF int LIBCCALL libc_Xfputc(int c, FILE *__restrict self);
INTDEF int LIBCCALL libc_fputc_unlocked(int c, FILE *__restrict self);
INTDEF int LIBCCALL libc_Xfputc_unlocked(int c, FILE *__restrict self);
INTDEF int LIBCCALL libc_getw(FILE *__restrict self);
INTDEF int LIBCCALL libc_Xgetw(FILE *__restrict self);
INTDEF int LIBCCALL libc_putw(int w, FILE *__restrict self);
INTDEF int LIBCCALL libc_Xputw(int w, FILE *__restrict self);
INTDEF int LIBCCALL libc_getw_unlocked(FILE *__restrict self);
INTDEF int LIBCCALL libc_Xgetw_unlocked(FILE *__restrict self);
INTDEF int LIBCCALL libc_putw_unlocked(int w, FILE *__restrict self);
INTDEF int LIBCCALL libc_Xputw_unlocked(int w, FILE *__restrict self);

INTDEF ssize_t LIBCCALL libc_fputs(char const *__restrict s, FILE *__restrict self);
INTDEF ssize_t LIBCCALL libc_Xfputs(char const *__restrict s, FILE *__restrict self);
INTDEF ssize_t LIBCCALL libc_fputs_unlocked(char const *__restrict s, FILE *__restrict self);
INTDEF ssize_t LIBCCALL libc_Xfputs_unlocked(char const *__restrict s, FILE *__restrict self);

INTDEF void LIBCCALL libc_clearerr(FILE *__restrict self);

INTDEF int LIBCCALL libc_fflush(FILE *self);
INTDEF int LIBCCALL libc_fflush_unlocked(FILE *self);
INTDEF void LIBCCALL libc_Xfflush(FILE *self);
INTDEF void LIBCCALL libc_Xfflush_unlocked(FILE *self);
INTDEF int LIBCCALL libc_fflushall(void);

INTDEF void LIBCCALL libc_setbuf(FILE *__restrict self, char *__restrict buf);
INTDEF void LIBCCALL libc_setbuffer(FILE *__restrict self, char *__restrict buf, size_t size);
INTDEF void LIBCCALL libc_setlinebuf(FILE *__restrict self);
INTDEF int LIBCCALL libc_setvbuf(FILE *__restrict self, char *__restrict buf, int modes, size_t n);
INTDEF void LIBCCALL libc_Xsetvbuf(FILE *__restrict self, char *__restrict buf, int modes, size_t n);
INTDEF int LIBCCALL libc_setvbuf_unlocked(FILE *__restrict self, char *__restrict buf, int modes, size_t n);
INTDEF void LIBCCALL libc_Xsetvbuf_unlocked(FILE *__restrict self, char *__restrict buf, int modes, size_t n);

INTDEF int LIBCCALL libc_ungetc(int c, FILE *__restrict self);
INTDEF int LIBCCALL libc_Xungetc(int c, FILE *__restrict self);
INTDEF int LIBCCALL libc_ungetc_unlocked(int c, FILE *__restrict self);
INTDEF int LIBCCALL libc_Xungetc_unlocked(int c, FILE *__restrict self);

INTDEF ssize_t LIBCCALL libc_getdelim(char **__restrict lineptr, size_t *__restrict n, int delimiter, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_Xgetdelim(char **__restrict lineptr, size_t *__restrict n, int delimiter, FILE *__restrict self);
INTDEF ssize_t LIBCCALL libc_getdelim_unlocked(char **__restrict lineptr, size_t *__restrict n, int delimiter, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_Xgetdelim_unlocked(char **__restrict lineptr, size_t *__restrict n, int delimiter, FILE *__restrict self);
INTDEF ssize_t LIBCCALL libc_getline(char **__restrict lineptr, size_t *__restrict n, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_Xgetline(char **__restrict lineptr, size_t *__restrict n, FILE *__restrict self);
INTDEF ssize_t LIBCCALL libc_getline_unlocked(char **__restrict lineptr, size_t *__restrict n, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_Xgetline_unlocked(char **__restrict lineptr, size_t *__restrict n, FILE *__restrict self);
INTDEF int LIBCCALL libc_feof(FILE *__restrict self);
INTDEF int LIBCCALL libc_ferror(FILE *__restrict self);
INTDEF fd_t LIBCCALL libc_fileno(FILE *__restrict self);
INTDEF void LIBCCALL libc_rewind(FILE *__restrict self);
INTDEF void LIBCCALL libc_rewind_unlocked(FILE *__restrict self);

INTDEF char *LIBCCALL libc_fgets(char *__restrict s, size_t n, FILE *__restrict self);
INTDEF char *LIBCCALL libc_Xfgets(char *__restrict s, size_t n, FILE *__restrict self);
INTDEF char *LIBCCALL libc_fgets_int(char *__restrict s, unsigned int n, FILE *__restrict self);
INTDEF char *LIBCCALL libc_Xfgets_int(char *__restrict s, unsigned int n, FILE *__restrict self);
INTDEF char *LIBCCALL libc_fgets_unlocked(char *__restrict s, size_t n, FILE *__restrict self);
INTDEF char *LIBCCALL libc_Xfgets_unlocked(char *__restrict s, size_t n, FILE *__restrict self);
INTDEF char *LIBCCALL libc_fgets_int_unlocked(char *__restrict s, unsigned int n, FILE *__restrict self);
INTDEF char *LIBCCALL libc_Xfgets_int_unlocked(char *__restrict s, unsigned int n, FILE *__restrict self);

/* File functions for operating on std streams. */
INTDEF int LIBCCALL libc_getchar(void);
INTDEF int LIBCCALL libc_Xgetchar(void);
INTDEF int LIBCCALL libc_putchar(int c);
INTDEF int LIBCCALL libc_Xputchar(int c);
INTDEF int LIBCCALL libc_getchar_unlocked(void);
INTDEF int LIBCCALL libc_Xgetchar_unlocked(void);
INTDEF int LIBCCALL libc_putchar_unlocked(int c);
INTDEF int LIBCCALL libc_Xputchar_unlocked(int c);
INTDEF char *LIBCCALL libc_gets(char *__restrict s);
INTDEF ssize_t LIBCCALL libc_puts(char const *__restrict s);
INTDEF size_t LIBCCALL libc_Xputs(char const *__restrict s);
INTDEF ssize_t LIBCCALL libc_puts_unlocked(char const *__restrict s);
INTDEF size_t LIBCCALL libc_Xputs_unlocked(char const *__restrict s);


#undef stdin
#undef stdout
#undef stderr
INTDEF FILE libc_std_files[3];
DATDEF FILE *stdin;
DATDEF FILE *stdout;
DATDEF FILE *stderr;

#if 1
#define libc_stdin   (libc_std_files+0)
#define libc_stdout  (libc_std_files+1)
#define libc_stderr  (libc_std_files+2)
#else
#define libc_stdin    stdin
#define libc_stdout   stdout
#define libc_stderr   stderr
#endif



/* Wide character STDIO support */
INTDEF wint_t LIBCCALL libc_getwchar16(void);
INTDEF wint_t LIBCCALL libc_getwchar32(void);
INTDEF wint_t LIBCCALL libc_Xgetwchar16(void);
INTDEF wint_t LIBCCALL libc_Xgetwchar32(void);
INTDEF wint_t LIBCCALL libc_getwchar16_unlocked(void);
INTDEF wint_t LIBCCALL libc_getwchar32_unlocked(void);
INTDEF wint_t LIBCCALL libc_Xgetwchar16_unlocked(void);
INTDEF wint_t LIBCCALL libc_Xgetwchar32_unlocked(void);
INTDEF wint_t LIBCCALL libc_fgetwc16(FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_fgetwc32(FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_Xfgetwc16(FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_Xfgetwc32(FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_fgetwc16_unlocked(FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_fgetwc32_unlocked(FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_Xfgetwc16_unlocked(FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_Xfgetwc32_unlocked(FILE *__restrict stream);

INTDEF wint_t LIBCCALL libc_fputwc16(char16_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_fputwc32(char32_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_Xfputwc16(char16_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_Xfputwc32(char32_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_fputwc16_unlocked(char16_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_fputwc32_unlocked(char32_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_Xfputwc16_unlocked(char16_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_Xfputwc32_unlocked(char32_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_putwchar16(char16_t wc);
INTDEF wint_t LIBCCALL libc_putwchar32(char32_t wc);
INTDEF wint_t LIBCCALL libc_Xputwchar16(char16_t wc);
INTDEF wint_t LIBCCALL libc_Xputwchar32(char32_t wc);
INTDEF wint_t LIBCCALL libc_putwchar16_unlocked(char16_t wc);
INTDEF wint_t LIBCCALL libc_putwchar32_unlocked(char32_t wc);
INTDEF wint_t LIBCCALL libc_Xputwchar16_unlocked(char16_t wc);
INTDEF wint_t LIBCCALL libc_Xputwchar32_unlocked(char32_t wc);

INTDEF wint_t LIBCCALL libc_ungetwc16(wint_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_ungetwc32(wint_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_ungetwc16_unlocked(wint_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_ungetwc32_unlocked(wint_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_Xungetwc16(wint_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_Xungetwc32(wint_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_Xungetwc16_unlocked(wint_t wc, FILE *__restrict stream);
INTDEF wint_t LIBCCALL libc_Xungetwc32_unlocked(wint_t wc, FILE *__restrict stream);

INTDEF char16_t *LIBCCALL libc_fgetws16(char16_t *__restrict ws, size_t n, FILE *__restrict stream);
INTDEF char32_t *LIBCCALL libc_fgetws32(char32_t *__restrict ws, size_t n, FILE *__restrict stream);
INTDEF char16_t *LIBCCALL libc_fgetws16_int(char16_t *__restrict ws, int n, FILE *__restrict stream);
INTDEF char32_t *LIBCCALL libc_fgetws32_int(char32_t *__restrict ws, int n, FILE *__restrict stream);
INTDEF char16_t *LIBCCALL libc_fgetws16_unlocked(char16_t *__restrict ws, size_t n, FILE *__restrict stream);
INTDEF char32_t *LIBCCALL libc_fgetws32_unlocked(char32_t *__restrict ws, size_t n, FILE *__restrict stream);
INTDEF char16_t *LIBCCALL libc_fgetws16_int_unlocked(char16_t *__restrict ws, int n, FILE *__restrict stream);
INTDEF char32_t *LIBCCALL libc_fgetws32_int_unlocked(char32_t *__restrict ws, int n, FILE *__restrict stream);
INTDEF ATTR_RETNONNULL char16_t *LIBCCALL libc_Xfgetws16(char16_t *__restrict ws, size_t n, FILE *__restrict stream);
INTDEF ATTR_RETNONNULL char32_t *LIBCCALL libc_Xfgetws32(char32_t *__restrict ws, size_t n, FILE *__restrict stream);
INTDEF ATTR_RETNONNULL char16_t *LIBCCALL libc_Xfgetws16_int(char16_t *__restrict ws, int n, FILE *__restrict stream);
INTDEF ATTR_RETNONNULL char32_t *LIBCCALL libc_Xfgetws32_int(char32_t *__restrict ws, int n, FILE *__restrict stream);
INTDEF ATTR_RETNONNULL char16_t *LIBCCALL libc_Xfgetws16_unlocked(char16_t *__restrict ws, size_t n, FILE *__restrict stream);
INTDEF ATTR_RETNONNULL char32_t *LIBCCALL libc_Xfgetws32_unlocked(char32_t *__restrict ws, size_t n, FILE *__restrict stream);
INTDEF ATTR_RETNONNULL char16_t *LIBCCALL libc_Xfgetws16_int_unlocked(char16_t *__restrict ws, int n, FILE *__restrict stream);
INTDEF ATTR_RETNONNULL char32_t *LIBCCALL libc_Xfgetws32_int_unlocked(char32_t *__restrict ws, int n, FILE *__restrict stream);

INTDEF ssize_t LIBCCALL libc_fputws16(char16_t const *__restrict ws, FILE *__restrict stream);
INTDEF ssize_t LIBCCALL libc_fputws32(char32_t const *__restrict ws, FILE *__restrict stream);
INTDEF ssize_t LIBCCALL libc_fputws16_unlocked(char16_t const *__restrict ws, FILE *__restrict stream);
INTDEF ssize_t LIBCCALL libc_fputws32_unlocked(char32_t const *__restrict ws, FILE *__restrict stream);
INTDEF size_t LIBCCALL libc_Xfputws16(char16_t const *__restrict ws, FILE *__restrict stream);
INTDEF size_t LIBCCALL libc_Xfputws32(char32_t const *__restrict ws, FILE *__restrict stream);
INTDEF size_t LIBCCALL libc_Xfputws16_unlocked(char16_t const *__restrict ws, FILE *__restrict stream);
INTDEF size_t LIBCCALL libc_Xfputws32_unlocked(char32_t const *__restrict ws, FILE *__restrict stream);

/* Misc. functions only here for DOS compatibility. */
INTDEF ssize_t LIBCCALL libc_putws16(char16_t const *__restrict ws);
INTDEF ssize_t LIBCCALL libc_putws32(char32_t const *__restrict ws);
INTDEF char16_t *LIBCCALL libc_getws16(char16_t *__restrict buf);
INTDEF char32_t *LIBCCALL libc_getws32(char32_t *__restrict buf);
INTDEF char16_t *LIBCCALL libc_getws16_s(char16_t *__restrict buf, size_t buflen);
INTDEF char32_t *LIBCCALL libc_getws32_s(char32_t *__restrict buf, size_t buflen);


INTDEF int LIBCCALL libc_fwide(FILE *__restrict fp, int mode);


DECL_END
#endif /* CONFIG_LIBC_USES_NEW_STDIO */

#endif /* !GUARD_LIBS_LIBC_STDIO_H */
