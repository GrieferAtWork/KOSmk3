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
#ifndef GUARD_KERNEL_INCLUDE_FS_IOMODE_H
#define GUARD_KERNEL_INCLUDE_FS_IOMODE_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>

/* General-purpose I/O mode flags. */

DECL_BEGIN

/* Generic I/O flags (compatible with O_* flags) */
#ifdef __CC__
typedef u16 iomode_t;
#endif
#define IO_ACCMODE     0x0003
#define IO_RDONLY      0x0000 /* Read-only access */
#define IO_WRONLY      0x0001 /* Write-only access */
#define IO_RDWR        0x0002 /* Read/write access */
#define IO_RDWR_ALT    0x0003 /* Read/write access */
#define IO_APPEND      0x0400 /* Append newly written data at the end */
#define IO_NONBLOCK    0x0800 /* Don't block in I/O */
#define IO_SYNC        0x1000 /* Ensure that all modified caches are flushed during write() */
#define IO_ASYNC       0x2000 /* Use asynchronous I/O and generate SIGIO upon completion. */
#define IO_DIRECT      0x4000 /* Bypass input/output buffers if possible. - Try to read/write data directly to/from provided buffers. */

/* Mask of flags recognized. */
#define IO_MASK         (IO_ACCMODE|IO_APPEND|IO_NONBLOCK|IO_SYNC|IO_ASYNC|IO_DIRECT)

/* Mask of flags modifiable by `F_SETFD' / `F_SETFL'
 * NOTE: This is also the mask of flags that may
 *       be passed via `readf' / `writef' and friends. */
#define IO_SETFL_MASK  ((IO_MASK & ~(IO_ACCMODE)))
#define IO_SETFD_MASK  (IO_HANDLE_FMASK)


/* NOTE: The following flags don't affect I/O, but are stored in the same field.
 *       Additionally, their `O_*' quivalent have different values. */
#define IO_HANDLE_FCLOEXEC 0x0004 /* Close during exec() */
#define IO_HANDLE_FCLOFORK 0x0008 /* Close during fork() */
#define IO_HANDLE_FMASK    0x000c /* Mask for handle flags (flags not inherited during `dup()'). */
#define IO_HANDLE_FFROM_O(x)  (__CCAST(iomode_t)(((x)&0x180000) >> 15))
#define IO_HANDLE_FTO_O(x)   ((__CCAST(oflag_t)(x)&0xc) << 15)
/* Similar to `IO_HANDLE_FFROM_O()' / `IO_HANDLE_FTO_O()', but used for converting `FD_*' flags. */
#define IO_HANDLE_FFROM_FD(x) (__CCAST(iomode_t)((x) << 2))
#define IO_HANDLE_FTO_FD(x)  ((__CCAST(oflag_t)(x) >> 2)


/* Convert I/O flags to/from `O_*' flags. */
#define IO_FROM_O(x)   (((x)&0xfff3)|IO_HANDLE_FFROM_O(x))
#define IO_TO_O(x)     (((x)&0xfff3)|IO_HANDLE_FTO_O(x))

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_IOMODE_H */
