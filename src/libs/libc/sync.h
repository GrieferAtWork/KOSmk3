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
#ifndef GUARD_LIBS_LIBC_SYNC_H
#define GUARD_LIBS_LIBC_SYNC_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     SYNCHRONIZATION PRIMITIVES                                                        */
/* ===================================================================================== */
struct mutex;
INTDEF int LIBCCALL libc_mutex_get_timed(struct mutex *__restrict self, struct timespec32 *abs_timeout);
INTDEF int LIBCCALL libc_mutex_get_timed64(struct mutex *__restrict self, struct timespec64 *abs_timeout);
INTDEF void LIBCCALL libc_mutex_put(struct mutex *__restrict self);
INTDEF int LIBCCALL libc_Xmutex_get_timed(struct mutex *__restrict self, struct timespec32 *abs_timeout);
INTDEF int LIBCCALL libc_Xmutex_get_timed64(struct mutex *__restrict self, struct timespec64 *abs_timeout);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_SYNC_H */
