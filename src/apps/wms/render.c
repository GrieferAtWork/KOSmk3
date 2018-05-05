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
#ifndef GUARD_APPS_WMS_RENDER_H
#define GUARD_APPS_WMS_RENDER_H 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <wm/api.h>
#include <assert.h>
#include <syslog.h>
#include <string.h>

#include "render.h"

DECL_BEGIN



#undef CONFIG_COPYRECT_DO_OUTLINE
#if 1
#define CONFIG_COPYRECT_DO_OUTLINE  4
#endif


INTERN void KCALL
Copy_Rect(byte_t *__restrict dst_buffer,
          unsigned int dst_x, unsigned int dst_y, unsigned int dst_stride,
          byte_t const *__restrict src_buffer,
          unsigned int src_x, unsigned int src_y, unsigned int src_stride,
          unsigned int size_x, unsigned int size_y,
          unsigned int bpp) {
 assert(size_x || size_y);
#if 1
 syslog(LOG_DEBUG,"copyrect(%u,%u*%u <-- %u,%u*%u  w=%u,h=%u)\n",
        dst_x,dst_y,dst_stride,src_x,src_y,src_stride,size_x,size_y);
#endif
 dst_buffer += dst_y*dst_stride;
 src_buffer += src_y*src_stride;
 if (bpp == 8) {
do_bytewise_copy:
  assert(dst_x+size_x <= dst_stride);
  assert(src_x+size_x <= src_stride);
  dst_buffer += dst_x;
  src_buffer += src_x;
  /* Copy line-wise */
#ifdef CONFIG_COPYRECT_DO_OUTLINE
  --size_y;
  memset(dst_buffer,CONFIG_COPYRECT_DO_OUTLINE,size_x);
  src_buffer += src_stride;
  dst_buffer += dst_stride;
#endif
  for (; size_y; --size_y) {
   memcpy(dst_buffer,
          src_buffer,
          size_x);
#ifdef CONFIG_COPYRECT_DO_OUTLINE
   dst_buffer[0]        = CONFIG_COPYRECT_DO_OUTLINE;
   dst_buffer[size_x-1] = CONFIG_COPYRECT_DO_OUTLINE;
#endif
   dst_buffer += dst_stride;
   src_buffer += src_stride;
  }
#ifdef CONFIG_COPYRECT_DO_OUTLINE
  memset(dst_buffer-dst_stride,
         CONFIG_COPYRECT_DO_OUTLINE,size_x);
#endif
 } else if (!(bpp & 7)) {
  dst_x  *= bpp/8;
  src_x  *= bpp/8;
  size_x *= bpp/8;
  goto do_bytewise_copy;
 } else {
  assertf(0,"TODO");
 }
}

DECL_END

#endif /* !GUARD_APPS_WMS_RENDER_H */
