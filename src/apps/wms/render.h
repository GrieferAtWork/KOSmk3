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

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <wm/api.h>

#include "rect.h"

DECL_BEGIN

INTDEF void KCALL
Copy_Rect(byte_t *__restrict dst_buffer,
          unsigned int dst_x, unsigned int dst_y, unsigned int dst_stride,
          byte_t const *__restrict src_buffer,
          unsigned int src_x, unsigned int src_y, unsigned int src_stride,
          unsigned int size_x, unsigned int size_y,
          unsigned int bpp);

DECL_END

#endif /* !GUARD_APPS_WMS_RENDER_H */
