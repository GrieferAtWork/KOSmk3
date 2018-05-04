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
#ifndef GUARD_LIBS_LIBWM_EVENT_C
#define GUARD_LIBS_LIBWM_EVENT_C 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <wm/api.h>
#include <wm/server.h>
#include <wm/window.h>
#include <unistd.h>
#include <except.h>
#include <sys/poll.h>

#include "libwm.h"

DECL_BEGIN


INTERN void WMCALL
libwm_event_dealwith(union wm_event *__restrict evt) {
 /* TODO */
}

INTERN void WMCALL
libwm_event_process(void) {
 /* TODO */
}

INTERN bool WMCALL
libwm_event_trywait(union wm_event *__restrict result) {
 /* TODO */
 return false;
}

INTERN void WMCALL
libwm_event_waitfor(union wm_event *__restrict result) {
 /* TODO */
}


DECL_END

#endif /* !GUARD_LIBS_LIBWM_EVENT_C */
