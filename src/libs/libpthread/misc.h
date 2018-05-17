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
#ifndef GUARD_LIBS_LIBPTHREAD_MISC_H
#define GUARD_LIBS_LIBPTHREAD_MISC_H 1

#include "libpthread.h"
#include <kos/types.h>

DECL_BEGIN

typedef pthread_once_t thread_once_t;

INTDEF errno_t LIBPCALL thread_once(thread_once_t *__restrict once_control, void (*init_routine)(void));
INTDEF void LIBPCALL Xthread_once(thread_once_t *__restrict once_control, void (*init_routine)(void));

DECL_END

#endif /* !GUARD_LIBS_LIBPTHREAD_MISC_H */
