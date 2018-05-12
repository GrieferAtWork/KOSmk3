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
#ifndef GUARD_APPS_WMS_DISPLAY_C
#define GUARD_APPS_WMS_DISPLAY_C 1
#define __BUILDING_WMSERVER 1

#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <kos/types.h>
#include <wm/api.h>
#include <wm/server.h>
#include <string.h>

#include "display.h"
#include "render.h"
#include "window.h"

DECL_BEGIN

/* The default display adapter. */
INTERN Display default_display = { 0, };


INTERN void WMCALL
Display_RenderBackground(Display *__restrict self,
                         struct rect display_rect) {
 Copy_Rect(self->d_screen,
           display_rect.r_xmin,
           display_rect.r_ymin,
           self->d_stride,
           self->d_backgrnd,
           display_rect.r_xmin,
           display_rect.r_ymin,
           self->d_stride,
           display_rect.r_xsiz,
           display_rect.r_ysiz,
           self->d_bpp);
}


INTERN void WMCALL
Display_Redraw(Display *__restrict self) {
 struct rect r;
 Window *win;
#if 1
 /* Copy the background first. */
 RECTS_FOREACH(r,self->d_backvisi) {
  Copy_Rect(self->d_screen,
            r.r_xmin,
            r.r_ymin,
            self->d_stride,
            self->d_backgrnd,
            r.r_xmin,
            r.r_ymin,
            self->d_stride,
            r.r_xsiz,
            r.r_ysiz,
            self->d_bpp);
 }
#endif
 /* Then copy all visible window portions. */
 for (win  = self->d_windows;
      win != NULL; win = win->w_vlink.le_next) {
  RECTS_FOREACH(r,win->w_visi) {
   Copy_Rect(self->d_screen,
             win->w_posx+r.r_xmin,
             win->w_posy+r.r_ymin,
             self->d_stride,
             win->w_screen,
             r.r_xmin,
             r.r_ymin,
             win->w_stride,
             r.r_xsiz,
             r.r_ysiz,
             self->d_bpp);
  }
 }
}

INTERN void WMCALL
Display_RedrawRect(Display *__restrict self, struct rect draw_rect) {
 struct rect r;
 Window *win;
#if 1
 /* Copy the background first. */
 RECTS_FOREACH(r,self->d_backvisi) {
  struct rect common;
  common = rect_intersect(r,draw_rect);
  if (!common.r_xsiz || !common.r_ysiz)
       continue;
  Copy_Rect(self->d_screen,
            common.r_xmin,
            common.r_ymin,
            self->d_stride,
            self->d_backgrnd,
            common.r_xmin,
            common.r_ymin,
            self->d_stride,
            common.r_xsiz,
            common.r_ysiz,
            self->d_bpp);
 }
#endif
 /* Then copy all visible window portions. */
 for (win  = self->d_windows;
      win != NULL; win = win->w_vlink.le_next) {
  RECTS_FOREACH(r,win->w_visi) {
   struct rect common;
   r.r_xmin += win->w_posx;
   r.r_ymin += win->w_posy;
   common = rect_intersect(r,draw_rect);
   r.r_xmin -= win->w_posx;
   r.r_ymin -= win->w_posy;
   if (!common.r_xsiz || !common.r_ysiz)
        continue;
   Copy_Rect(self->d_screen,
             common.r_xmin,
             common.r_ymin,
             self->d_stride,
             win->w_screen,
             common.r_xmin - win->w_posx,
             common.r_ymin - win->w_posy,
             win->w_stride,
             common.r_xsiz,
             common.r_ysiz,
             self->d_bpp);
  }
 }
}

/* Return the window located at the given coords, or NULL if no window is there. */
INTERN Window *WMCALL
Display_WindowAt(Display *__restrict self,
                 unsigned int x, unsigned int y) {
 Window *result = self->d_zorder;
 for (; result; result = result->w_zlink.le_next) {
  if (x >= result->w_disparea.r_xmin &&
      x <  result->w_disparea.r_xmin+result->w_disparea.r_xsiz &&
      y >= result->w_disparea.r_ymin &&
      y <  result->w_disparea.r_ymin+result->w_disparea.r_ysiz)
      break;
 }
 return result;
}

/* Set keyboard focus to the given window. */
INTERN void WMCALL
Display_FocusWindow(Display *__restrict self,
                    Window *__restrict win) {
 if (self->d_focus == win)
     return;
 if (self->d_focus) {
  struct wms_response msg;
  memset(&msg,0,sizeof(struct wms_response));
  msg.r_answer                             = WMS_RESPONSE_EVENT;
  msg.r_event.e_window.w_type              = WM_EVENT_WINDOW;
  msg.r_event.e_window.w_event             = WM_WINDOWEVENT_STATE_CHANGE;
  msg.r_event.e_window.w_winid             = self->d_focus->w_id;
  msg.r_event.e_window.w_changed.s_oldstat = self->d_focus->w_state|WM_WINDOW_STATE_FFOCUSED;
  msg.r_event.e_window.w_changed.s_oldstat = self->d_focus->w_state;
  Window_SendMessage(self->d_focus,&msg);
 }
 self->d_focus = win;
 if (win) {
  struct wms_response msg;
  memset(&msg,0,sizeof(struct wms_response));
  msg.r_answer                             = WMS_RESPONSE_EVENT;
  msg.r_event.e_window.w_type              = WM_EVENT_WINDOW;
  msg.r_event.e_window.w_event             = WM_WINDOWEVENT_STATE_CHANGE;
  msg.r_event.e_window.w_winid             = win->w_id;
  msg.r_event.e_window.w_changed.s_oldstat = win->w_state;
  msg.r_event.e_window.w_changed.s_oldstat = win->w_state|WM_WINDOW_STATE_FFOCUSED;
  Window_SendMessage(win,&msg);
 }
}


DECL_END

#endif /* !GUARD_APPS_WMS_DISPLAY_C */
