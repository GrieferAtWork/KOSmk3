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
#ifndef GUARD_LIBS_LIBWM_WINDOW_C
#define GUARD_LIBS_LIBWM_WINDOW_C 1
#define _EXCEPT_SOURCE 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <hybrid/align.h>
#include <wm/api.h>
#include <wm/server.h>
#include <wm/window.h>
#include <unistd.h>
#include <except.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <malloc.h>

#include "libwm.h"

DECL_BEGIN

DEFINE_PUBLIC_ALIAS(wm_window_move,libwm_window_move);
INTERN void WMCALL
libwm_window_move(struct wm_window *__restrict self,
                  int new_posx, int new_posy) {
 struct wms_request req; unsigned int token;
 struct wms_response resp;
 wm_window_lock(self);
 if (self->w_posx == new_posx &&
     self->w_posy == new_posy) {
  wm_window_unlock(self);
  /* Nothing to do here! */
  return;
 }
 wm_window_unlock(self);
 req.r_command        = WMS_COMMAND_MVWIN;
 req.r_flags          = WMS_COMMAND_FNORMAL;
 req.r_mvwin.mw_winid = self->w_winid;
 req.r_mvwin.mw_xmin  = new_posx;
 req.r_mvwin.mw_ymin  = new_posy;
 token = libwms_sendrequest(&req);
 /* Wait for the ACK.
  * NOTE: Once the window was actually moved, the server
  *       will send a `WM_WINDOWEVENT_MOVED' event.
  *       It is this event which will then update
  *       our window structure's position elements. */
 libwms_recvresponse(token,&resp);
}


DEFINE_PUBLIC_ALIAS(wm_window_bring_to_front,libwm_window_bring_to_front);
INTERN bool WMCALL
libwm_window_bring_to_front(struct wm_window *__restrict self) {
 struct wms_request req; unsigned int token;
 struct wms_response resp;
 req.r_command = WMS_COMMAND_TOFRONT;
 req.r_flags   = WMS_COMMAND_FNORMAL;
 req.r_tofront.fw_winid = self->w_winid;
 token = libwms_sendrequest(&req);
 libwms_recvresponse(token,&resp);
 return resp.r_answer == WMS_RESPONSE_TOFRONT_OK;
}

DEFINE_PUBLIC_ALIAS(wm_window_chstat,libwm_window_chstat);
INTERN u16 WMCALL
libwm_window_chstat(struct wm_window *__restrict self, u16 mask, u16 flag) {
 struct wms_request req; unsigned int token;
 struct wms_response resp; u16 result;
 if (flag & ~(WM_WINDOW_STATE_FHIDDEN|WM_WINDOW_STATE_FFOCUSED))
     error_throw(E_INVALID_ARGUMENT);
 wm_window_lock(self);
 result = self->w_state;
 if (((result & mask) | flag) == result) {
  /* Nothing changes */
  wm_window_unlock(self);
  return result;
 }
 wm_window_unlock(self);
 req.r_command = WMS_COMMAND_CHWIN;
 req.r_flags   = WMS_COMMAND_FNORMAL;
 req.r_chwin.cw_winid = self->w_winid;
 req.r_chwin.cw_mask  = mask;
 req.r_chwin.cw_flag  = flag;
 /* Window fields are actually updated by `WM_WINDOWEVENT_STATE_CHANGE' events. */
 token = libwms_sendrequest(&req);
 libwms_recvresponse(token,&resp);
 if (resp.r_answer != WMS_RESPONSE_CHWIN_OK)
     error_throw(E_NOT_IMPLEMENTED);
 return resp.r_chwin.cw_oldst;
}



STATIC_ASSERT(WM_WINDOW_DRAW_FASYNC == WMS_COMMAND_FNOACK);
STATIC_ASSERT(WM_WINDOW_DRAW_FVSYNC == WMS_COMMAND_DRAW_FVSYNC);
DEFINE_PUBLIC_ALIAS(wm_window_draw,libwm_window_draw);
INTERN void WMCALL
libwm_window_draw(struct wm_window *__restrict self,
                  unsigned int mode) {
 struct wms_request req; unsigned int token;
 struct wms_response resp;
 if (mode & ~(WM_WINDOW_DRAW_FASYNC|WM_WINDOW_DRAW_FVSYNC))
     error_throw(E_INVALID_ARGUMENT);
 req.r_command          = WMS_COMMAND_DRAWALL;
 req.r_flags            = mode;
 req.r_drawall.dw_winid = self->w_winid;
 token = libwms_sendrequest(&req);
 if (!(mode & WM_WINDOW_DRAW_FASYNC)) {
  /* Wait for the ACK. */
  libwms_recvresponse(token,&resp);
 }
}


DEFINE_PUBLIC_ALIAS(wm_window_draw_rect,libwm_window_draw_rect);
INTERN void WMCALL
libwm_window_draw_rect(struct wm_window *__restrict self,
                       int xmin, int ymin,
                       unsigned int xsiz, unsigned int ysiz,
                       unsigned int mode) {
 struct wms_request req; unsigned int token;
 struct wms_response resp;
 if (mode & ~(WM_WINDOW_DRAW_FASYNC|WM_WINDOW_DRAW_FVSYNC))
     error_throw(E_INVALID_ARGUMENT);
 if (xmin < 0) {
  if (xsiz <= (unsigned int)-xmin) goto empty;
  xsiz -= -xmin;
  xmin = 0;
 }
 if (ymin < 0) {
  if (ysiz <= (unsigned int)-ymin) goto empty;
  ysiz -= -ymin;
  ymin = 0;
 }
 wm_window_lock(self);
 TRY {
  if ((unsigned int)(xmin+xsiz) > self->w_surface.s_sizex) {
   if ((unsigned int)xmin >= self->w_surface.s_sizex) goto empty_unlock;
   xsiz = self->w_surface.s_sizex-xmin;
  }
  if ((unsigned int)(ymin+ysiz) > self->w_surface.s_sizey) {
   if ((unsigned int)ymin >= self->w_surface.s_sizey) goto empty_unlock;
   ysiz = self->w_surface.s_sizey-ymin;
  }
  /* Fill in the render request. */
  req.r_command          = WMS_COMMAND_DRAWONE;
  req.r_flags            = mode;
  req.r_drawone.dw_winid = self->w_winid;
  req.r_drawone.dw_xmin  = self->w_bordersz+(unsigned int)xmin;
  req.r_drawone.dw_ymin  = self->w_titlesz+(unsigned int)ymin;
  req.r_drawone.dw_xsiz  = xsiz;
  req.r_drawone.dw_ysiz  = ysiz;
 } FINALLY {
  wm_window_unlock(self);
 }
 /* Send the request. */
 token = libwms_sendrequest(&req);
 if (!(mode & WM_WINDOW_DRAW_FASYNC)) {
  /* Wait for the ACK. */
  libwms_recvresponse(token,&resp);
 }
 return;
empty_unlock:
 wm_window_unlock(self);
empty:
 if (mode & WM_WINDOW_DRAW_FASYNC) return;
 if (!(mode & WM_WINDOW_DRAW_FVSYNC)) return;
 /* Wait for a display sync. */
 req.r_command          = WMS_COMMAND_DRAWONE;
 req.r_flags            = WMS_COMMAND_DRAW_FVSYNC;
 req.r_drawone.dw_winid = self->w_winid;
 req.r_drawone.dw_xsiz  = 0;
 req.r_drawone.dw_ysiz  = 0;
 token = libwms_sendrequest(&req);
 libwms_recvresponse(token,&resp);
}

DEFINE_PUBLIC_ALIAS(wm_window_draw_rects,libwm_window_draw_rects);
INTERN void WMCALL
libwm_window_draw_rects(struct wm_window *__restrict self,
                        size_t rectc, struct wm_rect const *__restrict rectv,
                        unsigned int mode) {
 size_t i;
 if (!rectc) return;
 for (i = 0; i < rectc-1; ++i) {
  /* Draw all rects leading up to the last in async mode. */
  libwm_window_draw_rect(self,
                         rectv[i].r_xmin,
                         rectv[i].r_ymin,
                         rectv[i].r_xsiz,
                         rectv[i].r_ysiz,
                         WM_WINDOW_DRAW_FASYNC);
 }
 /* Draw the last rectangle in the specified mode. */
 libwm_window_draw_rect(self,
                        rectv[i].r_xmin,
                        rectv[i].r_ymin,
                        rectv[i].r_xsiz,
                        rectv[i].r_ysiz,
                        mode);
}


DEFINE_PUBLIC_ALIAS(wm_window_close,libwm_window_close);
INTERN void WMCALL
libwm_window_close(struct wm_window *__restrict self) {
 union wm_event evt;
 u32 feat = ATOMIC_READ(self->w_features);
 if (!(feat & WM_WINDOW_FEAT_FNOAUTOHIDE))
       libwm_window_chstat(self,~0,WM_WINDOW_STATE_FHIDDEN);
 if (feat & WM_WINDOW_FEAT_FEXITONCLOSE)
     error_throw(E_EXIT_PROCESS);
 /* Trigger a window-close event. */
 evt.e_window.w_type   = WM_EVENT_WINDOW;
 evt.e_window.w_flag   = WM_EVENT_FNORMAL;
 evt.e_window.w_window = self;
 evt.e_window.w_event  = WM_WINDOWEVENT_CLOSE;
 libwm_window_incref(self);
 TRY {
  libwm_event_dealwith(&evt);
 } FINALLY {
  libwm_window_decref(self);
 }
}

DEFINE_PUBLIC_ALIAS(wm_window_getid,libwm_window_getid);
INTERN ATTR_CONST wms_window_id_t WMCALL
libwm_window_getid(struct wm_window *__restrict self) {
 return self->w_winid;
}


PRIVATE void WMCALL
libwm_window_register(struct wm_window *__restrict win) {
 /* TODO: Insert into global window_id -> window hash table. */
}
PRIVATE void WMCALL
libwm_window_delete(struct wm_window *__restrict win) {
 /* TODO: Remove from global window_id -> window hash table. */
}


DEFINE_PUBLIC_ALIAS(wm_window_create,libwm_window_create);
INTERN ATTR_RETNONNULL REF struct wm_window *WMCALL
libwm_window_create(int pos_x, int pos_y, unsigned int size_x, unsigned int size_y,
                    char const *title, u32 features, u16 state, u16 mode,
                    struct wm_winevent_ops *eventops, void *userdata) {
 REF struct wm_window *EXCEPT_VAR result;
 result = (REF struct wm_window *)Xmalloc(sizeof(struct wm_window));
 TRY {
  struct wms_request req;
  struct wms_response resp;
  unsigned int token;
  result->w_winfd    = -1;
  result->w_bordersz = 3;
  result->w_titlesz  = 18;
  result->w_features = features;
  result->w_mode     = mode;
  if (features & WM_WINDOW_FEAT_FNOBORDER)
      result->w_titlesz -= result->w_bordersz,
      result->w_bordersz = 0;
  if (features & WM_WINDOW_FEAT_FNOHEADER)
      result->w_titlesz = result->w_bordersz;

  COMPILER_WRITE_BARRIER();
  req.r_command        = WMS_COMMAND_MKWIN;
  req.r_flags          = WMS_COMMAND_FNORMAL;
  req.r_mkwin.mw_xmin  = pos_x;
  req.r_mkwin.mw_ymin  = pos_y;
  req.r_mkwin.mw_xsiz  = size_x + (2 * result->w_bordersz);
  req.r_mkwin.mw_ysiz  = size_y + (result->w_bordersz +
                                   result->w_titlesz);
  req.r_mkwin.mw_state = state;

  /* Send the request. */
  token = libwms_sendrequest(&req);

  /* Receive the response. */
  result->w_winfd = libwms_recvresponse_fd(token,&resp);

  if (resp.r_answer != WMS_RESPONSE_MKWIN_OK)
      error_throw(E_NOT_IMPLEMENTED);

  TRY {
   /* Map the display buffer into memory. */
   result->w_buffer = (byte_t *)mmap(NULL,
                                     resp.r_mkwin.w_sizey*resp.r_mkwin.w_stride,
                                     PROT_READ|PROT_WRITE|PROT_SHARED,
                                     MAP_SHARED|MAP_FILE,
                                     result->w_winfd,
                                     0);
   TRY {

    /* Extract data from the response. */
    result->w_winid             = resp.r_mkwin.w_id;
    result->w_state             = resp.r_mkwin.w_state;
    result->w_posx              = resp.r_mkwin.w_posx;
    result->w_posy              = resp.r_mkwin.w_posy;
    result->w_sizex             = resp.r_mkwin.w_sizex;
    result->w_sizey             = resp.r_mkwin.w_sizex;
    result->w_oldsizex          = resp.r_mkwin.w_sizex;
    result->w_oldsizey          = resp.r_mkwin.w_sizey;

    /* Adjust so the surface describes the window contents without the border. */
    result->w_surface.s_flags   = WM_SURFACE_FWINDOW;
    result->w_surface.s_sizex   = resp.r_mkwin.w_sizex - (2 * result->w_bordersz);
    result->w_surface.s_sizey   = resp.r_mkwin.w_sizey - (result->w_bordersz + result->w_titlesz);
    result->w_surface.s_stride  = resp.r_mkwin.w_stride;
    result->w_surface.s_bpp     = resp.r_mkwin.w_bpp;
    result->w_surface.s_buffer  = result->w_buffer;
    result->w_surface.s_buffer += result->w_titlesz * resp.r_mkwin.w_stride;
    result->w_surface.s_buffer += result->w_bordersz;

    /* TODO: This currently assumes the format that is implemented by the server. */
    result->w_surface.s_format = libwm_format_create_pal(&libwm_palette_256);

    TRY {
     /* Map the window  */
     libwm_window_register(result);
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     libwm_format_decref(result->w_surface.s_format);
     error_rethrow();
    }

   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    munmap(result->w_buffer,
           resp.r_mkwin.w_sizey*
           resp.r_mkwin.w_stride);
    error_rethrow();
   }

  } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
   /* Destroy the window again if something went wrong. */
   error_pushinfo();
   req.r_command        = WMS_COMMAND_RMWIN;
   req.r_flags          = WMS_COMMAND_FNOACK;
   req.r_rmwin.rw_winid = resp.r_mkwin.w_id;
   libwms_sendrequest(&req);
   error_popinfo();
   error_rethrow();
  }

 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  if (result->w_winfd >= 0)
      close(result->w_winfd);
  free(result);
  error_rethrow();
 }
 return result;
}

INTERN void WMCALL
libwm_window_resize(struct wm_window *__restrict self,
                    unsigned int new_sizx,
                    unsigned int new_sizy) {
 /* TODO */
}



DEFINE_PUBLIC_ALIAS(wm_window_fromid,libwm_window_fromid);
INTERN REF struct wm_window *WMCALL
libwm_window_fromid(wms_window_id_t id) {
 /* TODO: Search global window_id -> window hash table.
  * NOTE: Windows with a reference counter of ZERO(0) must be returned! */
 return NULL;
}


INTERN void WMCALL
libwm_window_destroy(struct wm_window *__restrict self) {
 struct wms_request req; unsigned int token;
 struct wms_response resp;
 /* First off: Remove the window from the global id->window hash table. */
 libwm_window_delete(self);
 if (!(self->w_state & WM_WINDOW_STATE_FHIDDEN)) {
  /* Hide the window. */
  req.r_command = WMS_COMMAND_CHWIN;
  req.r_flags   = WMS_COMMAND_FNORMAL;
  req.r_chwin.cw_winid = self->w_winid;
  req.r_chwin.cw_mask  = ~0;
  req.r_chwin.cw_flag  = WM_WINDOW_STATE_FHIDDEN;
  /* XXX: What about exceptions here? */
  token = libwms_sendrequest(&req);
  libwms_recvresponse(token,&resp);
 }
 /* Invoke a user-defined window-fini callback. */
 if (self->w_events &&
     self->w_events->wo_fini)
   (*self->w_events->wo_fini)(self);
 /* Unmap the window display buffer. */
 munmap(self->w_buffer,
        self->w_surface.s_stride * self->w_sizey);
 /* Remove the window. */
 req.r_command        = WMS_COMMAND_RMWIN;
 req.r_flags          = WMS_COMMAND_FNOACK;
 req.r_rmwin.rw_winid = self->w_winid;
 /* XXX: What about exceptions here? */
 libwms_sendrequest(&req);

 /* Close the window screen buffer file. */
 close(self->w_winfd);

 /* Free all the remaining buffers and pointers. */
 libwm_format_decref(self->w_surface.s_format);
 free(self->w_title);
 free(self);
}




DEFINE_PUBLIC_ALIAS(wm_window_chfeat,libwm_window_chfeat);
INTERN u32 WMCALL
libwm_window_chfeat(struct wm_window *__restrict self, u32 mask, u32 flag) {
 /* TODO */
 return 0;
}

DEFINE_PUBLIC_ALIAS(wm_window_chmode,libwm_window_chmode);
INTERN u16 WMCALL
libwm_window_chmode(struct wm_window *__restrict self, u16 mask, u16 flag) {
 /* TODO */
 return 0;
}

DEFINE_PUBLIC_ALIAS(wm_window_settitle,libwm_window_settitle);
INTERN void WMCALL
libwm_window_settitle(struct wm_window *__restrict self,
                      char const *__restrict new_title) {
 /* TODO */
}


DECL_END

#endif /* !GUARD_LIBS_LIBWM_WINDOW_C */
