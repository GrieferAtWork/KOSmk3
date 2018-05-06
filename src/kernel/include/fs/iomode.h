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
typedef u16 packet_iomode_t;
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
#define IO_NOIOVCHECK  0x8000 /* When passed to one of the `readv'-style functions, omit
                               * pointer validation of the pointers found in the iov vector.
                               * This flag should be passed when the IOV vector is actually
                               * managed by kernel-space, with contained pointers either already
                               * having been checked, or being intended to refer to kernel-space
                               * buffers, rather than ones located in user-space.
                               * NOTE: For obvious reasons, this flag should not be exposed
                               *       to user-space, so keep it out of the `IO_MASK' below. */

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


/* Packet-related I/O flags. */
#define PACKET_IO_FRDNORMAL    0x0000 /* [RECV] Normal flags for packet I/O reader functions. */
#define PACKET_IO_FRDALWAYS    0x0000 /* [RECV] Always read and unlink a full packet,
                                       *        and any data that cannot be read */
#define PACKET_IO_FRDOOB       0x0001 /* [RECV][== MSG_OOB][???]
                                       *        Process out-of-band data. */
#define PACKET_IO_FRDNEVER     0x0002 /* [RECV][== MSG_PEEK]
                                       *        Never discard a packet (only peek data)
                                       *        This flag supersedes `PACKET_IO_FRDIFFIT' */
#define PACKET_IO_FRDIFFIT     0x0004 /* [RECV] Only unlink a packet if it fully fits into the provided user-buffer. */
#define PACKET_IO_FRDTRUNC     0x0008 /* [RECV] For use with `PACKET_IO_FRDIFFIT':
                                       *        If the user-buffer is too small to fit the packet, truncate the packet
                                       *        and only read the user-buffer portion for which there was enough space.
                                       *        However, ancillary data ignores this flag and will not be truncated. */
/*      PACKET_IO_FRD...       0x0020  * Reserved for `MSG_TRUNC' */
/*      PACKET_IO_FRD...       0x0040  * Reserved for `MSG_DONTWAIT' */
#define PACKET_IO_FRDWAITALL   0x0100 /* [RECV][== MSG_WAITALL]
                                       *        Combine packets from the same source to fill the provided buffer
                                       *        to its entirety, and return only once everything has been read. */
#define PACKET_IO_FRDERRQUEUE  0x2000 /* [RECV][== MSG_ERRQUEUE]
                                       *        Fetch message from error queue. */


#define PACKET_IO_FWRNORMAL    0x0000 /* [SEND] Normal flags for packet I/O writer functions. */
#define PACKET_IO_FRDOOB       0x0001 /* [SEND][== MSG_OOB][???]
                                       *        Process out-of-band data. */
#define PACKET_IO_FWRDONTROUTE 0x0004 /* [SEND][== MSG_DONTROUTE][???]
                                       *        Don't use local routing. */
#define PACKET_IO_FWRSPLIT     0x0008 /* [SEND][== PACKET_IO_FRDTRUNC]
                                       *        For use by packet I/O writer functions: the
                                       *        data block can be split into multiple packets. */
/*      PACKET_IO_FWR...       0x0040  * Reserved for `MSG_DONTWAIT' */
#define PACKET_IO_FWREOR       0x0080 /* [SEND][== MSG_EOR][???]
                                       *        End of record. */
#define PACKET_IO_FWRCONFIRM   0x0800 /* [SEND][== MSG_CONFIRM][???]
                                       *        Confirm path validity. */
/*      PACKET_IO_FWR...       0x4000  * Reserved for `MSG_NOSIGNAL' */
#define PACKET_IO_FWRMORE      0x8000 /* [SEND][== MSG_MORE] Sender will send more. */



#ifdef __CC__
struct iovec;

/* Read `num_bytes' from `src + iov_offset' into `dst'
 * @param: mode: When `IO_NOIOVCHECK' is passed, don't validate
 *               pointers found inside entries of the `src' vector.
 *               In this case, `src' should actually be:
 *              `HOST struct iovec const *__restrict src'
 * @throw: E_SEGFAULT: A faulty user-space buffer was provided. */
FUNDEF void KCALL iov_read(USER CHECKED void *dst,
                           USER CHECKED struct iovec const *src,
                           size_t iov_offset, size_t num_bytes,
                           iomode_t mode);

/* Write `num_bytes' from `src' into `dst + iov_offset'
 * @param: mode: When `IO_NOIOVCHECK' is passed, don't validate
 *               pointers found inside entries of the `dst' vector.
 *               In this case, `dst' should actually be:
 *              `HOST struct iovec const *__restrict dst'
 * @throw: E_SEGFAULT: A faulty user-space buffer was provided. */
FUNDEF void KCALL iov_write(USER CHECKED struct iovec const *dst, size_t iov_offset,
                            USER CHECKED void const *src, size_t num_bytes,
                            iomode_t mode);

#endif



DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_FS_IOMODE_H */
