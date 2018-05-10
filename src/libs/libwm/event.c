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

#if 0
typedef struct pending_event {
    struct pending_event *pe_next;  /* [0..1][lock(:pending_lock)] The next pending event. */
    union wm_event        pe_event; /* [const] The event that is pending. */
} PendingEvent;

PRIVATE DEFINE_ATOMIC_RWLOCK(pending_lock);  /* Lock for synchronizing pending events. */
PRIVATE struct pending_event *pending_front; /* [0..1][lock(pending_lock)] Next pending event. */
PRIVATE struct pending_event *pending_back;  /* [0..1][lock(pending_lock)] Last pending event. */
#endif






INTERN void WMCALL
libwm_event_dealwith(union wm_event *__restrict evt) {
 struct wm_winevent_ops *event_ops;
 assert(evt->e_common.c_window);
 if (evt->e_type == WM_EVENT_WINDOW) {
  struct wm_window *win = evt->e_common.c_window;
  atomic_rwlock_write(&win->s_lock);
  atomic_rwlock_endwrite(&win->s_lock);
 }
 if ((event_ops = evt->e_common.c_window->w_events) != NULL) {
  void (WMCALL *func)(union wm_event const *__restrict info);
  /* Directly invoke event callbacks (meant for GUI applications...) */
  switch (evt->e_type) {
  case WM_EVENT_KEY:    func = (void(WMCALL *)(union wm_event const *__restrict))event_ops->wo_key; break;
  case WM_EVENT_MOUSE:  func = (void(WMCALL *)(union wm_event const *__restrict))event_ops->wo_mouse; break;
  case WM_EVENT_WINDOW: func = (void(WMCALL *)(union wm_event const *__restrict))event_ops->wo_window; break;
  default: func = event_ops->wo_event; break;
  }
  /* When not implement, fallback to using the default event handler. */
  if (!func) func = event_ops->wo_event;
  /* Invoke the event callback, if implemented. */
  if (func) { (*func)(evt); return; }
 }

 /* TODO: Queue the event for being received
  *       using `libwm_event_[try]wait[for]()' */

}

DEFINE_PUBLIC_ALIAS(wm_event_process,libwm_event_process);
INTERN void WMCALL libwm_event_process(void) {
 /* TODO */
}

DEFINE_PUBLIC_ALIAS(wm_event_trywait,libwm_event_trywait);
INTERN bool WMCALL
libwm_event_trywait(union wm_event *__restrict result) {
 /* TODO */
 return false;
}

DEFINE_PUBLIC_ALIAS(wm_event_waitfor,libwm_event_waitfor);
INTERN void WMCALL
libwm_event_waitfor(union wm_event *__restrict result) {
 /* TODO */
}


DECL_END

#endif /* !GUARD_LIBS_LIBWM_EVENT_C */
