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
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1
#define _EXCEPT_SOURCE 1
#define __BUILDING_LIBWM 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <hybrid/align.h>
#include <wm/api.h>
#include <wm/server.h>
#include <wm/window.h>
#include <malloc.h>
#include <unistd.h>
#include <except.h>
#include <string.h>
#include <syslog.h>
#include <sys/poll.h>
#include <kos/sched/semaphore.h>
#include <kos/futex.h>
#include <linux/futex.h>
#include <sys/mman.h>
#include <sys/socket.h>

#include "libwm.h"

DECL_BEGIN

#if 1
typedef struct pending_event {
    struct pending_event *pe_next;  /* [0..1][lock(:pending_lock)] The next pending event. */
    union wm_event        pe_event; /* [const] The event that is pending. */
} PendingEvent;

PRIVATE DEFINE_SEMAPHORE(pending_avail,0);   /* A semaphore that is fed a ticket for every . */
PRIVATE DEFINE_ATOMIC_RWLOCK(pending_lock);  /* Lock for synchronizing pending events. */
PRIVATE struct pending_event *pending_front; /* [0..1][lock(pending_lock)] Next pending event. */
PRIVATE struct pending_event *pending_back;  /* [0..1][lock(pending_lock)] Last pending event. */

#endif

/* Poll for events to become available. */
PRIVATE void WMCALL libwm_poll_events(void) {
 struct pollfd    pfds[1];
 struct pollfutex pftx[1];
 /* Poll the socket for read(), and the pending_avail semaphore at once.
  * KOS FTW!!! -- Linux doesn't let you poll a futex in this manner. */
 pfds[0].fd     = libwms_socket;
 pfds[0].events = POLLIN;
 semaphore_poll(&pending_avail,pftx);
 /* Wait for one of the events to become triggered. */
 Xxppoll(pfds,1,pftx,1,NULL,0,NULL,NULL);
}



/* Process the given event and return TRUE if the event should
 * _NOT_ be passed on to the caller of `libwm_event_waitfor()'
 * and friends, or appended to the pending_* list. */
PRIVATE bool WMCALL
process_event(union wm_event *__restrict evt) {
 struct wm_winevent_ops *event_ops;
 assert(evt->e_common.c_window);
 if (evt->e_type == WM_EVENT_WINDOW) {
  struct wm_window *EXCEPT_VAR win;
  win = evt->e_window.w_window;
  atomic_rwlock_write(&win->s_lock);
  TRY {
   /* Update window characteristics changed by outside events. */
   switch (evt->e_window.w_event) {
   case WM_WINDOWEVENT_MOVED:
    win->w_posx = evt->e_window.w_moved.m_newx;
    win->w_posy = evt->e_window.w_moved.m_newy;
    break;

   {
    size_t old_surface_pages;
    size_t new_surface_pages;
   case WM_WINDOWEVENT_RESIZED:
    old_surface_pages = (win->s_sizey * win->s_stride);
    new_surface_pages = (evt->e_window.w_resized.r_newysiz *
                         evt->e_window.w_resized.r_stride);
    old_surface_pages = CEIL_ALIGN(old_surface_pages,PAGESIZE);
    new_surface_pages = CEIL_ALIGN(new_surface_pages,PAGESIZE);
    /* Extend the length of the window surface buffer. */
    if (new_surface_pages > old_surface_pages) {
     win->w_buffer = (byte_t *)Xmremap(win->w_buffer,
                                       old_surface_pages,
                                       new_surface_pages,
                                       MREMAP_MAYMOVE);
    }
    win->w_sizex   = evt->e_window.w_resized.r_newxsiz;
    win->w_sizey   = evt->e_window.w_resized.r_newysiz;
    win->s_sizex   = win->w_sizex - (2 * win->w_bordersz);
    win->s_sizey   = win->w_sizey - (win->w_bordersz + win->w_titlesz);
    win->s_stride  = evt->e_window.w_resized.r_stride;
    /* Re-calculate the buffer start position. */
    win->s_buffer  = win->w_buffer;
    win->s_buffer += win->w_titlesz * win->s_stride;
    win->s_buffer += win->w_bordersz;
    if (evt->e_window.w_resized.r_bpp != win->s_format->f_bpp) {
     /* The window's surface format has changed. */
     if (win->s_format->f_refcnt == 1) {
      win->s_format->f_bpp = evt->e_window.w_resized.r_bpp;
     } else {
      REF struct wm_format *new_format;
      /* Create a new window format. */
      new_format = libwm_format_create_pal(&libwm_palette_256,
                                            evt->e_window.w_resized.r_bpp);
      libwm_format_decref(win->s_format);
      win->s_format = new_format;
     }
     /* Determine the new set of window operators. */
     libwm_setup_surface_ops((struct wm_surface *)win);
     /* XXX: Convert window surface data? */
    }
    /* XXX: Scale / re-align window contents? */

    /* Truncate the length of the window surface buffer. */
    if (new_surface_pages < old_surface_pages) {
     munmap(win->w_buffer+new_surface_pages,
            old_surface_pages-new_surface_pages);
    }
   } break;

   case WM_WINDOWEVENT_STATE_CHANGE:
    win->w_state = evt->e_window.w_changed.s_newstat;
    break;

   default: break;
   }
  } FINALLY {
   atomic_rwlock_endwrite(&win->s_lock);
  }
 }
 if ((event_ops = evt->e_common.c_window->w_events) != NULL) {
  bool (WMCALL *func)(union wm_event const *__restrict info);
  /* Directly invoke event callbacks (meant for GUI applications...) */
  switch (evt->e_type) {
  case WM_EVENT_KEY:    func = (bool(WMCALL *)(union wm_event const *__restrict))event_ops->wo_key; break;
  case WM_EVENT_MOUSE:  func = (bool(WMCALL *)(union wm_event const *__restrict))event_ops->wo_mouse; break;
  case WM_EVENT_WINDOW: func = (bool(WMCALL *)(union wm_event const *__restrict))event_ops->wo_window; break;
  default: func = event_ops->wo_event; break;
  }
  /* When not implement, fallback to using the default event handler. */
  if (!func) func = event_ops->wo_event;
  /* Invoke the event callback, if implemented. */
  if (func && (*func)(evt))
      return true;
 }
 return false;
}


INTERN void WMCALL
libwm_event_dealwith(union wm_event *__restrict evt) {
 struct pending_event *pending;
 /* Process window events and try to invoke asynchronous I/O hooks */
 if (process_event(evt))
     return; /* Don't enqueue events already processed as async I/O hooks. */
 /* Copy the event and enqueue it in the chain of pending events. */
 pending = (struct pending_event *)Xmalloc(sizeof(struct pending_event));
 memcpy(&pending->pe_event,evt,sizeof(union wm_event));
 libwm_window_incref(pending->pe_event.e_common.c_window);
 pending->pe_next = NULL;
 atomic_rwlock_write(&pending_lock);
 assert((pending_back != NULL) == (pending_front != NULL));
 if (!pending_back)
      pending_front = pending;
 else pending_back->pe_next = pending;
 pending_back = pending;
 atomic_rwlock_endwrite(&pending_lock);
 /* Signal the addition by generating a ticket. */
 semaphore_release(&pending_avail,1);
}


DEFINE_PUBLIC_ALIAS(wm_event_process,libwm_event_process);
INTERN void WMCALL libwm_event_process(void) {
 union wm_event evt;
 if (wm_event_trywait(&evt)) {
  /* Empty all event queues. */
  do libwm_window_decref(evt.e_common.c_window);
  while (libwm_event_trywait(&evt));
 } else {
  /* Wait for events until we can process at least one. */
  do
      libwm_poll_events();
  while (!libwm_event_trywait(&evt));
  libwm_window_decref(evt.e_common.c_window);
 }
}

DEFINE_PUBLIC_ALIAS(wm_event_trywait,libwm_event_trywait);
INTERN bool WMCALL
libwm_event_trywait(union wm_event *__restrict result_) {
 union wm_event *EXCEPT_VAR result = result_;
again:
 if (semaphore_tryacquire(&pending_avail)) {
  struct pending_event *evt;
  atomic_rwlock_write(&pending_lock);
  evt = pending_front;
  if likely(evt) {
   pending_front = evt->pe_next;
   assert((pending_back == evt) ==
          (pending_front == NULL));
   if (pending_back == evt)
       pending_back = NULL;
   atomic_rwlock_endwrite(&pending_lock);
   /* Copy the event into the provided buffer. */
   TRY {
    memcpy(result,&evt->pe_event,sizeof(union wm_event));
   } FINALLY {
    free(evt);
   }
#if 0
   syslog(LOG_DEBUG,"FROM QUEUE: %u -> %u\n",
          result->e_common.c_type,
          result->e_common.c_window->w_winid);
#endif
   return true;
  }
  atomic_rwlock_endwrite(&pending_lock);
 }
 /* Using `MSG_DONTWAIT', try to read an event packet from the server socket. */
 if (mutex_try(&libwms_lock)) {
  struct wms_response resp;
  bool EXCEPT_VAR event_ok = true;
  TRY {
   if (Xrecv(libwms_socket,&resp,sizeof(struct wms_response),MSG_DONTWAIT) !=
                                 sizeof(struct wms_response))
       event_ok = false;
  } FINALLY {
   if (FINALLY_WILL_RETHROW && error_code() == E_WOULDBLOCK) {
    event_ok = false;
    FINALLY_WILL_RETHROW = false;
   }
   mutex_put(&libwms_lock);
  }
  if (event_ok) {
   if (resp.r_answer != WMS_RESPONSE_EVENT)
       goto again;
   resp.r_event.e_common.c_window = libwm_window_fromid(resp.r_event.e_common.c_winid);
   if unlikely(!resp.r_event.e_common.c_window)
       goto again;
   TRY {
    /* Try to process generic events. */
    if (process_event(&resp.r_event)) {
     libwm_window_decref(resp.r_event.e_common.c_window);
     goto again;
    }
    /* Ensure proper event order by checking if there are new pending events. */
    if unlikely(ATOMIC_READ(pending_front) != NULL) {
     /* Appending the event as pending. */
     struct pending_event *pending;
     pending = (struct pending_event *)Xmalloc(sizeof(struct pending_event));
     memcpy(&pending->pe_event,&resp.r_event,sizeof(union wm_event)); /* Inherit reference to `c_window' */
     atomic_rwlock_write(&pending_lock);
     assert((pending_back != NULL) == (pending_front != NULL));
     if (!pending_back)
          pending_front = pending;
     else pending_back->pe_next = pending;
     pending_back = pending;
     atomic_rwlock_endwrite(&pending_lock);
     /* Signal the addition by generating a ticket. */
     semaphore_release(&pending_avail,1);
     goto again;
    }
    /* Return the event to the caller. */
    memcpy(result,&resp.r_event,sizeof(union wm_event));
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    libwm_window_decref(resp.r_event.e_common.c_window);
    error_rethrow();
   }
   /* Event was loaded directly from the socket. */
#if 0
   syslog(LOG_DEBUG,"FROM SOCKET: %u -> %u\n",
          result->e_common.c_type,
          result->e_common.c_window->w_winid);
#endif
   return true;
  }
 }
 return false;
}

DEFINE_PUBLIC_ALIAS(wm_event_waitfor,libwm_event_waitfor);
INTERN void WMCALL
libwm_event_waitfor(union wm_event *__restrict result) {
 /* Try to read an event, and if that fails, poll for events to become available. */
 while (!libwm_event_trywait(result))
      libwm_poll_events();
}


DECL_END

#endif /* !GUARD_LIBS_LIBWM_EVENT_C */
