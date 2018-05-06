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
#ifndef GUARD_KERNEL_SRC_FS_IOV_C
#define GUARD_KERNEL_SRC_FS_IOV_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <kos/types.h>
#include <kernel/user.h>
#include <fs/iomode.h>
#include <string.h>
#include <sys/uio.h>

DECL_BEGIN

/* Read `num_bytes' from `src + iov_offset' into `dst' */
PUBLIC void KCALL
iov_read(USER CHECKED void *dst,
         USER CHECKED struct iovec const *src,
         size_t iov_offset, size_t num_bytes,
         iomode_t mode) {
 size_t iov_index = 0;
 /* Skip leading IOV entries. */
 while (iov_offset) {
  size_t length;
  length = src[iov_index].iov_len;
  COMPILER_READ_BARRIER();
  if (length >= iov_offset) {
   void *base;
   base = src[iov_index].iov_base;
   COMPILER_READ_BARRIER();
   length -= iov_offset;
   length  = MIN(length,num_bytes);
   if (!(mode & IO_NOIOVCHECK))
         validate_readable(base,length);
   memcpy(dst,base,length);
   num_bytes -= length;
   *(uintptr_t *)&dst += length;
   ++iov_index;
   break;
  }
  iov_offset -= length;
 }
 /* Copy the actual IOV data. */
 while (num_bytes) {
  void *base;
  size_t length;
  length = src[iov_index].iov_len;
  base   = src[iov_index].iov_base;
  COMPILER_READ_BARRIER();
  length = MIN(length,num_bytes);
  if (!(mode & IO_NOIOVCHECK))
        validate_readable(base,length);
  memcpy(dst,base,length);
  num_bytes -= length;
  *(uintptr_t *)&dst += length;
  ++iov_index;
 }
}

/* Write `num_bytes' from `src' into `dst + iov_offset' */
PUBLIC void KCALL
iov_write(USER CHECKED struct iovec const *dst, size_t iov_offset,
          USER CHECKED void const *src, size_t num_bytes,
          iomode_t mode) {
 size_t iov_index = 0;
 /* Skip leading IOV entries. */
 while (iov_offset) {
  size_t length;
  length = dst[iov_index].iov_len;
  COMPILER_READ_BARRIER();
  if (length >= iov_offset) {
   void *base;
   base = dst[iov_index].iov_base;
   COMPILER_READ_BARRIER();
   length -= iov_offset;
   length  = MIN(length,num_bytes);
   if (!(mode & IO_NOIOVCHECK))
         validate_readable(base,length);
   memcpy(base,src,length);
   num_bytes -= length;
   *(uintptr_t *)&src += length;
   ++iov_index;
   break;
  }
  iov_offset -= length;
 }
 /* Copy the actual IOV data. */
 while (num_bytes) {
  void *base;
  size_t length;
  length = dst[iov_index].iov_len;
  base   = dst[iov_index].iov_base;
  COMPILER_READ_BARRIER();
  length = MIN(length,num_bytes);
  if (!(mode & IO_NOIOVCHECK))
        validate_readable(base,length);
  memcpy(base,src,length);
  num_bytes -= length;
  *(uintptr_t *)&src += length;
  ++iov_index;
 }
}



DECL_END

#endif /* !GUARD_KERNEL_SRC_FS_IOV_C */
