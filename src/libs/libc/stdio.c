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
#ifndef GUARD_LIBS_LIBC_STDIO_C
#define GUARD_LIBS_LIBC_STDIO_C 1

#include "libc.h"
#include <bits/io-file.h>
#include <hybrid/list/list.h>
#include <kos/sched/mutex.h>

DECL_BEGIN

#if 0
typedef ssize_t cookie_read_function_t(void *cookie, char *buf, size_t size);
typedef ssize_t cookie_write_function_t(void *cookie, const char *buf, size_t size);
typedef int cookie_seek_function_t(void *cookie, off64_t *offset, int whence);
typedef int cookie_close_function_t(void *cookie);
typedef struct {
    cookie_read_function_t  *read;
    cookie_write_function_t *write;
    cookie_seek_function_t  *seek;
    cookie_close_function_t *close;
} cookie_io_functions_t;

typedef struct iofile_data FileData;

struct iofile_data {
    uintptr_t             fb_zero;  /* Always ZERO(0). - Required for binary compatibility with DOS. */
    mutex_t               fb_lock;  /* Lock for synchronizing access to the buffer. */
    cookie_io_functions_t fb_func;  /* [const] File cookie functions. */
    void                 *fb_coki;  /* [const] File cookie closure argument. */
    byte_t               *fb_ptr;   /* [>= fb_base][+fb_cnt <= fb_base+fb_size][lock(fb_lock)]
                                     * Pointer to the next character to-be read/written.
                                     * The absolute in-file position is then `fb_fblk+(fb_ptr-fb_base)' */
    size_t                fb_cnt;   /* [lock(fb_lock)] The amount of unread, buffered bytes located at `fb_ptr'. */
    byte_t               *fb_chng;  /* [>= fb_base][+fb_chsz <= fb_base+fb_size]
                                     * [valid_if(fb_chsz != 0)][lock(fb_lock)]
                                     * Pointer to the first character that was
                                     * changed since the buffer had been loaded. */
    size_t                fb_chsz;  /* [lock(fb_lock)] Amount of bytes that were changed. */
    byte_t               *fb_base;  /* [0..fb_size][owned_if(!FILE_BUFFER_FSTATICBUF)][lock(fb_lock)] Allocated buffer.
                                     * NOTE: This pointer must not be modified when `FILE_BUFFER_FREADING' is set. */
    size_t                fb_size;  /* [lock(fb_lock)] Total allocated / available buffer size.
                                     * NOTE: This pointer must not be modified when `FILE_BUFFER_FREADING' is set. */
    LIST_NODE(FileData)   fb_ttych; /* Chain of changed TTY file buffers (buffers that are used with an interactive file).
                                     * Any buffer that is connected to an interactive device is flushed before
                                     * data is read from any other interactive device.
                                     * This chain is weakly linked in that buffer objects remove themself
                                     * before destruction, also meaning that any buffer contained in this
                                     * chain may have a reference counter to ZERO(0). */
    pos64_t               fb_fblk;  /* The starting address of the data block currently stored in `fb_base'. */
    pos64_t               fb_fpos;  /* The current (assumed) position within `fb_file'. */
#define FILE_BUFFER_FNORMAL     0x0000 /* Normal buffer flags. */
#define FILE_BUFFER_FREADONLY   0x0001 /* The buffer can not only be used for reading, but also for writing. */
#define FILE_BUFFER_FNODYNSCALE 0x0002 /* The buffer is allowed to dynamically change its buffer size. */
#define FILE_BUFFER_FLNBUF      0x0004 /* The buffer is line-buffered, meaning that it will
                                        * flush its data whenever a line-feed is printed.
                                        * Additionally if the `FILE_BUFFER_FISATTY' flag is set,
                                        * attempting to read from a line-buffered file will cause
                                        * all other existing line-buffered files to be synchronized
                                        * first. This is done to ensure that interactive files are
                                        * always up-to-date before data is read from one of them. */
#define FILE_BUFFER_FSYNC       0x0008 /* Also synchronize the underlying file after flushing the buffer. */
#define FILE_BUFFER_FCLOFILE    0x0010 /* When the buffer is closed through use of `operator close',
                                        * also invoke `operator close' the associated file.
                                        * However, when `close()' is never invoked on the buffer, its
                                        * destructor will _NOT_ invoke close on the underlying file. */
#define FILE_BUFFER_FREADING    0x0800 /* The buffer is currently being read into and must not be changed or resized. */
#define FILE_BUFFER_FNOTATTY    0x1000 /* This buffer does not refer to a TTY device. */
#define FILE_BUFFER_FISATTY     0x2000 /* This buffer refers to a TTY device. */
#define FILE_BUFFER_FSTATICBUF  0x4000 /* Must be used with `FILE_BUFFER_FNODYNSCALE': When set,
                                        * the buffer doesn't actually own its buffer and must not
                                        * attempt to free() it during destruction.
                                        * The `FILE_BUFFER_FNODYNSCALE' must be set to prevent the
                                        * buffer from attempting to resize (realloc) it dynamically. */
#define FILE_BUFFER_FLNIFTTY    0x8000 /* Automatically set/delete the `FILE_BUFFER_FLNBUF' and
                                        * `FILE_BUFFER_FISATTY' flags, and add/remove the file from
                                        * `fb_ttys' the next this it comes into question. To determine
                                        * this, the pointed-to file is tested for being a TTY device
                                        * using `DeeFile_IsAtty(fb_file)'.
                                        * HINT: This flag is set for all newly created buffers by default. */
    u16                   fb_flag;  /* [lock(fb_lock)] The current state of the buffer. */
};



INTDEF size_t LIBCCALL libc_ReadFromFile(void *__restrict buf, size_t size, FILE *__restrict self);
INTDEF size_t LIBCCALL libc_WriteToFile(void const *__restrict buf, size_t size, FILE *__restrict self);
INTDEF int LIBCCALL libc_FlushFile(FILE *__restrict self);
INTDEF pos64_t LIBCCALL libc_TellFile(FILE *__restrict self);
INTDEF int LIBCCALL libc_SeekFile(FILE *__restrict self, off64_t off, int whence);
INTDEF int LIBCCALL libc_SetFileBuffer(FILE *__restrict self, char *__restrict buf, int modes, size_t n);
INTDEF int LIBCCALL libc_UngetFileCharacter(int c, FILE *__restrict self);
INTDEF int LIBCCALL libc_FillFileBuffer(FILE *__restrict self);
#endif


DECL_END

#endif /* !GUARD_LIBS_LIBC_STDIO_C */
