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
#ifndef GUARD_APPS_WMS_WINDOW_C
#define GUARD_APPS_WMS_WINDOW_C 1
#define _KOS_SOURCE 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/align.h>
#include <hybrid/list/list.h>
#include <kos/types.h>
#include <wm/api.h>
#include <malloc.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <syscall.h>
#include <sys/mman.h>

#include "window.h"
#include "display.h"
#include "render.h"

DECL_BEGIN

PRIVATE void WMCALL Window_UpdateDisplayArea(Window *__restrict self);

/* Claim visibility within `window_rect' from other windows
 * with a lower Z-order, as well as the background layer.
 * This function is called to acquire visibility from other
 * windows and the background when a window is moved. */
PRIVATE void WMCALL
Window_ClaimVisibility(Window *__restrict self,
                       struct rect window_rect,
                       bool render_immediatly);

/* Remove everything that was visible within `window_rect'
 * and let that area be inherited by other windows with a
 * lower Z-order.
 * This function is called to release visibility to other
 * windows and the background when a window is moved. */
PRIVATE void WMCALL
Window_RemoveAndInheritVisibility(Window *__restrict self,
                               struct rect window_rect);


/* Copy window memory from `window_rect' directly onto the screen. */
PRIVATE void WMCALL Window_RenderRect(Window *__restrict self, struct rect window_rect);
INTDEF void WMCALL Display_RenderBackground(Display *__restrict self, struct rect display_rect);

INTDEF unsigned int WMCALL
Window_GetPixel(Window *__restrict self,
                unsigned int x, unsigned int y) {
 unsigned int result;
 byte_t *ptr;
 assert(x < self->w_sizex);
 assert(y < self->w_sizey);
 ptr  = self->w_screen;
 ptr += y * self->w_stride; /* XXX: Use shifts here? */
 x   *= self->w_display->d_bpp;
 ptr += x/8;
 switch (self->w_display->d_bpp) {
 case 32:
  result = *(u32 *)ptr;
  break;
 case 16:
  result = *(u16 *)ptr;
  break;
 case 8:
  result = *(u8 *)ptr;
  break;
 case 4:
  x %= 8;
  assert(x == 0 || x == 4);
  result = *(u8 *)ptr >> x;
  result &= 0xf;
  break;
 case 2:
  x %= 8;
  assert(x == 0 || x == 2 || x == 4 || x == 6);
  result = *(u8 *)ptr >> x;
  result &= 0x3;
  break;
 case 1:
  result = *(u8 *)ptr >> (x % 8);
  result &= 0x1;
  break;
 default: assert(0);
 }
 return result;
}
INTERN void WMCALL
Window_PutPixel(Window *__restrict self,
                unsigned int x, unsigned int y,
                unsigned int pixel) {
 byte_t *ptr;
 assert(x < self->w_sizex);
 assert(y < self->w_sizey);
 ptr  = self->w_screen;
 ptr += y * self->w_stride; /* XXX: Use shifts here? */
 x   *= self->w_display->d_bpp;
 ptr += x/8;
 switch (self->w_display->d_bpp) {
 case 32:
  *(u32 *)ptr = (u32)pixel;
  break;
 case 16:
  *(u16 *)ptr = (u16)pixel;
  break;
 case 8:
  *(u8 *)ptr = (u8)pixel;
  break;
 case 4:
  x %= 8;
  assert(x == 0 || x == 4);
  *(u8 *)ptr &= ~(0xf << x);
  *(u8 *)ptr |= (pixel & 0xf) << x;
  break;
 case 2:
  x %= 8;
  assert(x == 0 || x == 2 || x == 4 || x == 6);
  *(u8 *)ptr &= ~(0x3 << x);
  *(u8 *)ptr |= (pixel & 0x3) << x;
  break;
 case 1:
  *(u8 *)ptr &= ~(0x1 << x);
  *(u8 *)ptr |= (pixel & 0x1) << x;
  break;
 default: assert(0);
 }
}


PRIVATE void WMCALL
Window_UpdateDisplayArea(Window *__restrict self) {
 struct rect area;
 Display *disp = self->w_display;
 area.r_xmin = self->w_posx <= 0 ? 0 : self->w_posx;
 area.r_ymin = self->w_posy <= 0 ? 0 : self->w_posy;
 area.r_xsiz = self->w_sizex;
 area.r_ysiz = self->w_sizey;
 /* Clamp negative window coords. */
 if (self->w_posx < 0) area.r_xsiz = area.r_xsiz > (unsigned int)-self->w_posx ? area.r_xsiz - -self->w_posx : 0;
 if (self->w_posy < 0) area.r_ysiz = area.r_ysiz > (unsigned int)-self->w_posy ? area.r_ysiz - -self->w_posy : 0;
 /* Clamp positive window coords. */
 /**/ if (area.r_xmin >= disp->d_sizex) area.r_xsiz = 0;
 else if (area.r_xmin+area.r_xsiz > disp->d_sizex) {
  area.r_xsiz = disp->d_sizex-area.r_xmin;
 }
 /**/ if (area.r_ymin >= disp->d_sizey) area.r_ysiz = 0;
 else if (area.r_ymin+area.r_ysiz > disp->d_sizey) {
  area.r_ysiz = disp->d_sizey-area.r_ymin;
 }
 self->w_disparea = area;
}


/* Claim visibility within `display_rect' from other windows
 * with a lower Z-order, as well as the background layer.
 * This function is called to acquire visibility from other
 * windows and the background when a window is moved. */
PRIVATE void WMCALL
Window_ClaimVisibility(Window *__restrict self,
                       struct rect display_rect,
                       bool render_immediatly) {
 Window *iter;
 Display *d = self->w_display;
 struct rect r;
 assert(display_rect.r_xsiz && display_rect.r_ysiz);
 /* Steal from windows with a lower Z-order. */
 for (iter  = self->w_zlink.le_next;
      iter != NULL;
      iter  = iter->w_zlink.le_next) {
again_window_visi:
  RECTS_FOREACH(r,iter->w_visi) {
   struct rect common;
   r.r_xmin += iter->w_posx;
   r.r_ymin += iter->w_posy;
   common = rect_intersect(r,display_rect);
   r.r_xmin -= iter->w_posx;
   r.r_ymin -= iter->w_posy;
   if (!common.r_xsiz || !common.r_ysiz) continue;
   /* Steal this visibility rectangle. */
   common.r_xmin -= iter->w_posx;
   common.r_ymin -= iter->w_posy;
   rects_remove(&iter->w_visi,common);
   common.r_xmin += iter->w_posx;
   common.r_ymin += iter->w_posy;
   common.r_xmin -= self->w_posx;
   common.r_ymin -= self->w_posy;
   /* Add the visible portion to our own
    * window and render the affected area. */
   rects_insert(&self->w_visi,common);
   if (render_immediatly)
       Window_RenderRect(self,common);
   goto again_window_visi;
  }
 }
again_background_visi:
 /* Steal from the background layer. */
 RECTS_FOREACH(r,d->d_backvisi) {
  struct rect common_rect;
  common_rect = rect_intersect(r,display_rect);
  if (!common_rect.r_xsiz || !common_rect.r_ysiz) continue;
  /* Steal this visibility rectangle. */
  rects_remove(&d->d_backvisi,common_rect);
  common_rect.r_xmin -= self->w_posx;
  common_rect.r_ymin -= self->w_posy;
  /* Add the visible portion to our own
   * window and render the affected area. */
  rects_insert(&self->w_visi,common_rect);
  if (render_immediatly)
      Window_RenderRect(self,common_rect);
  goto again_background_visi;
 }
}


/* Remove everything that was visible within `window_rect'
 * and let that area be inherited by other windows with a
 * lower Z-order.
 * This function is called to release visibility to other
 * windows and the background when a window is moved. */
PRIVATE void WMCALL
Window_RemoveAndInheritVisibility(Window *__restrict self,
                                  struct rect window_rect) {
 struct rect r;
 struct rects parts;
 Display *d = self->w_display;
 assert(window_rect.r_xsiz && window_rect.r_ysiz);
 parts.r_strips = NULL;
 /* Extract the intersection of all visible rectangles and `window_rect' */
 RECTS_FOREACH(r,self->w_visi) {
  struct rect common;
  common = rect_intersect(r,window_rect);
  common.r_xmin += self->w_posx;
  common.r_ymin += self->w_posy;
  rects_insert(&parts,common);
 }
 /* Remove everything that wasn't inherited by child windows. */
 rects_remove(&self->w_visi,window_rect);
 /* Now let lower-order windows, or the background layer
  * inherit everything that got removed from the visibility. */
again_parts:
 RECTS_FOREACH(r,parts) {
  Window *iter;
  for (iter  = self->w_zlink.le_next;
       iter != NULL;
       iter  = iter->w_zlink.le_next) {
   struct rect common;
   common = rect_intersect(r,iter->w_disparea);
   if (!common.r_xsiz || !common.r_ysiz)
        continue;
   /* Inherit this part. */
   common.r_xmin -= iter->w_posx;
   common.r_ymin -= iter->w_posy;
   rects_insert(&iter->w_visi,common);
   /* Render the inherited part immediately. */
   Window_RenderRect(iter,common);

   /* Remove the common part from the set of rects that must be inherited. */
   common.r_xmin += iter->w_posx;
   common.r_ymin += iter->w_posy;
   rects_remove(&parts,common);
   goto again_parts;
  }
 }
 /* Anything still left must be inherited by the background layer. */
 RECTS_FOREACH(r,parts) {
  rects_insert(&d->d_backvisi,r);
  Display_RenderBackground(d,r);
 }
 rects_fini(&parts);
}




/* Construct a new window. */
INTERN Window *WMCALL
Window_CreateUnlocked(Display *__restrict disp,
                      int posx, int posy,
                      unsigned int sizex,
                      unsigned int sizey,
                      u16 state, fd_t client_fd) {
 Window *result;
 result = (Window *)Xmalloc(sizeof(Window));
 result->w_posx    = posx;
 result->w_posy    = posy;
 result->w_state   = state;
 result->w_sizey   = sizey;
 result->w_sizex   = sizex;
 result->w_stride  = CEIL_ALIGN(CEILDIV(sizex*disp->d_bpp,8),16);
 result->w_display = disp;
 result->w_visi.r_strips = NULL;
 result->w_clientfd = client_fd;
 result->w_screenfd = Xsyscall(SYS_xvm_region_create,0x100000,O_CLOEXEC);
 result->w_screen = (byte_t *)Xmmap(NULL,sizex*result->w_stride,
                                    PROT_READ|PROT_WRITE|PROT_SHARED,
                                    MAP_SHARED|MAP_FILE,result->w_screenfd,0);
 Window_UpdateDisplayArea(result);
 /* Clamp the visible portion of the window to the display size. */
 if (posx < 0) sizex = sizex > (unsigned int)-posx ? sizex - -posx : 0,posx = 0;
 if (posy < 0) sizey = sizey > (unsigned int)-posy ? sizey - -posy : 0,posy = 0;
 /**/ if ((unsigned int)posx > disp->d_sizex) sizex = 0;
 else if ((unsigned int)posx+sizex > disp->d_sizex) sizex = disp->d_sizex-(unsigned int)posx;
 /**/ if ((unsigned int)posy > disp->d_sizey) sizey = 0;
 else if ((unsigned int)posy+sizey > disp->d_sizey) sizey = disp->d_sizey-(unsigned int)posy;
 /* Insert the new window in front of all others. */
 LIST_INSERT(disp->d_zorder,result,w_zlink);
 /* Construct the initial visibility setup of the window. */
 if (sizex && sizey && !(state & WM_WINDOW_STATE_FHIDDEN)) {
  /* Add the window to the visible part of the display render list. */
  LIST_INSERT(disp->d_windows,result,w_vlink);
  Window_ClaimVisibility(result,result->w_disparea,false);
 }
 return result;
}



/* Copy window memory from `window_rect' directly onto the screen. */
PRIVATE void WMCALL
Window_RenderRect(Window *__restrict self,
                  struct rect window_rect) {
 Display *d = self->w_display;
 Copy_Rect(d->d_screen,
           window_rect.r_xmin+self->w_posx,
           window_rect.r_ymin+self->w_posy,
           d->d_stride,
           self->w_screen,
           window_rect.r_xmin,
           window_rect.r_ymin,
           self->w_stride,
           window_rect.r_xsiz,
           window_rect.r_ysiz,
           d->d_bpp);
}

INTERN void WMCALL
Window_DrawUnlocked(Window *__restrict self) {
 struct rect visi;
 RECTS_FOREACH(visi,self->w_visi) {
  Window_RenderRect(self,visi);
 }
}
INTERN void WMCALL
Window_DrawRectsUnlocked(Window *__restrict self,
                         size_t count,
                         struct rect *__restrict vec) {
 struct rect visi;
 RECTS_FOREACH(visi,self->w_visi) {
  size_t i;
  for (i = 0; i < count; ++i) {
   struct rect common = rect_intersect(visi,vec[i]);
   if (common.r_xsiz && common.r_ysiz)
       Window_RenderRect(self,common);
  }
 }
}
INTERN void WMCALL
Window_DrawRectUnlocked(Window *__restrict self, struct rect r) {
 struct rect visi;
 RECTS_FOREACH(visi,self->w_visi) {
  struct rect common = rect_intersect(visi,r);
  if (common.r_xsiz && common.r_ysiz)
      Window_RenderRect(self,common);
 }
}



/* Destroy the given window. */
INTERN void WMCALL
Window_DestroyUnlocked(Window *__restrict self) {
 /* Hide the window prior to its destruction to ensure
  * that it doesn't leave behind any artifacts. */
 Window_HideUnlocked(self);
 /* TODO */
 munmap(self->w_screen,
        self->w_sizex*self->w_stride);
 close(self->w_screenfd);
 free(self);
}



/* Move around the given window. */
INTERN void WMCALL
Window_MoveUnlocked(Window *__restrict self,
                    int new_posx, int new_posy) {
 int old_posx = self->w_posx;
 int old_posy = self->w_posy;
 int rel_movx = new_posx - old_posx;
 int rel_movy = new_posy - old_posy;
 struct rect r,old_display_area;
 struct rects reclaim_area;
 if (!rel_movx && !rel_movy) return;
 old_display_area = self->w_disparea;
 if (self->w_state & WM_WINDOW_STATE_FHIDDEN) {
  self->w_posx = new_posx;
  self->w_posy = new_posy;
  return;
 }
 
 /* Adjust the set of visible window
  * rects to apply to the new location. */
 if (rel_movx != 0) {
  self->w_posx = new_posx;
  Window_UpdateDisplayArea(self);
  rects_move(&self->w_visi,-rel_movx,0);
  if (rel_movx < 0) {
   r.r_xmin = self->w_sizex - -rel_movx;
   r.r_xsiz = -rel_movx;
  } else {
   r.r_xmin = 0;
   r.r_xsiz = rel_movx;
  }
  r.r_ymin = 0;
  r.r_ysiz = self->w_sizey;
  /* Release visibility following a horizontal move */
  Window_RemoveAndInheritVisibility(self,r);
 }
 if (rel_movy != 0) {
  self->w_posy = new_posy;
  Window_UpdateDisplayArea(self);
  rects_move(&self->w_visi,0,-rel_movy);
  if (rel_movy < 0) {
   r.r_ymin = self->w_sizey - -rel_movy;
   r.r_ysiz = -rel_movy;
  } else {
   r.r_ymin = 0;
   r.r_ysiz = rel_movy;
  }
  r.r_xmin = 0;
  r.r_xsiz = self->w_sizex;
  /* Release visibility following a vertical move */
  Window_RemoveAndInheritVisibility(self,r);
 }
 /* Claim visibility for everything apart of the new
  * display area, that isn't part of the old display
  * area. */
 reclaim_area.r_strips = NULL;
 rects_insert(&reclaim_area,self->w_disparea);
 rects_remove(&reclaim_area,old_display_area);
 RECTS_FOREACH(r,reclaim_area) {
  Window_ClaimVisibility(self,r,true);
 }
 rects_fini(&reclaim_area);
}


/* Set active window flags. */
INTERN void WMCALL
Window_HideUnlocked(Window *__restrict self) {
 Window *lower;
 Display *d;
 struct rect r;
 if (self->w_state & WM_WINDOW_STATE_FHIDDEN) return;
 LIST_REMOVE(self,w_vlink);
 d = self->w_display;
 /* Adjust the windows visibility set to become display-relative. */
 rects_move(&self->w_visi,-self->w_posx,-self->w_posy);
 /* Let windows with a lower Z order inherit visibility. */
 for (lower  = self->w_zlink.le_next;
      lower != NULL;
      lower  = lower->w_zlink.le_next) {
restart_foreach:
  RECTS_FOREACH(r,self->w_visi) {
   struct rect common_area;
   common_area = rect_intersect(r,lower->w_disparea);
   if (!common_area.r_xsiz || !common_area.r_ysiz)
        continue;
   /* Any single rect can only be inherited once. */
   rects_remove(&self->w_visi,common_area);
   /* Move visible parts of this window to windows with a lower z-order. */
   common_area.r_xmin -= lower->w_posx;
   common_area.r_ymin -= lower->w_posy;
   rects_insert(&lower->w_visi,common_area);
   /* Copy window data of the underlying window onto the display. */
   Window_RenderRect(lower,common_area);
   goto restart_foreach;
  }
 }
 /* Anything that's still left in the set of remaining rects
  * cannot be inherited by another window, meaning it needs to
  * be inherited by the background plane. */
 RECTS_FOREACH(r,self->w_visi) {
  rects_insert(&d->d_backvisi,r);
  /* Render the background for the affected area. */
  Display_RenderBackground(d,r);
 }
 /* Free what's left of our rects-copy buffer. */
 rects_fini(&self->w_visi);
 self->w_visi.r_strips = NULL;
 /* Finally, set the hidden flag. */
 self->w_state |= WM_WINDOW_STATE_FHIDDEN;
}

INTERN void WMCALL
Window_ShowUnlocked(Window *__restrict self) {
 struct rect r;
 if (!(self->w_state & WM_WINDOW_STATE_FHIDDEN)) return;
 r.r_xmin = 0;
 r.r_ymin = 0;
 r.r_xsiz = self->w_sizex;
 r.r_ysiz = self->w_sizey;
 Window_ClaimVisibility(self,r,true);
 LIST_INSERT(self->w_display->d_windows,self,w_vlink);
 self->w_state &= ~WM_WINDOW_STATE_FHIDDEN;
}




DECL_END

#endif /* !GUARD_APPS_WMS_WINDOW_C */
