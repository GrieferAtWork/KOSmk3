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

#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <kos/types.h>
#include <wm/api.h>
#include <wm/server.h>

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


DECL_END

#endif /* !GUARD_APPS_WMS_DISPLAY_C */
