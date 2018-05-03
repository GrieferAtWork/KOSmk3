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
#ifndef GUARD_APPS_WM_WINDOW_C
#define GUARD_APPS_WM_WINDOW_C 1
#define _KOS_SOURCE 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <hybrid/align.h>
#include <hybrid/minmax.h>
#include <kos/types.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <syslog.h>

#include "window.h"

DECL_BEGIN

PRIVATE void KCALL window_update_display_area(struct window *__restrict self);

/* Claim visibility within `window_rect' from other windows
 * with a lower Z-order, as well as the background layer.
 * This function is called to acquire visibility from other
 * windows and the background when a window is moved. */
PRIVATE void KCALL
window_claim_visi(struct window *__restrict self,
                  struct rect window_rect,
                  bool render_immediatly);

/* Remove everything that was visible within `window_rect'
 * and let that area be inherited by other windows with a
 * lower Z-order.
 * This function is called to release visibility to other
 * windows and the background when a window is moved. */
PRIVATE void KCALL
window_remove_visi_and_inherit(struct window *__restrict self,
                               struct rect window_rect);


/* Copy window memory from `window_rect' directly onto the screen. */
PRIVATE void KCALL window_render_rect(struct window *__restrict self, struct rect window_rect);
PRIVATE void KCALL display_render_background(struct display *__restrict self, struct rect display_rect);

INTDEF unsigned int KCALL
window_getpixel(struct window *__restrict self,
                unsigned int x, unsigned int y) {
 unsigned int result;
 byte_t *ptr;
 assert(x < self->w_sizex);
 assert(y < self->w_sizey);
 ptr  = self->w_screen;
 ptr += y * self->w_stride; /* XXX: Use shifts here? */
 x   *= self->w_bpp;
 ptr += x/8;
 switch (self->w_bpp) {
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
INTERN void KCALL
window_putpixel(struct window *__restrict self,
                unsigned int x, unsigned int y,
                unsigned int pixel) {
 byte_t *ptr;
 assert(x < self->w_sizex);
 assert(y < self->w_sizey);
 ptr  = self->w_screen;
 ptr += y * self->w_stride; /* XXX: Use shifts here? */
 x   *= self->w_bpp;
 ptr += x/8;
 switch (self->w_bpp) {
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


PRIVATE void KCALL
window_update_display_area(struct window *__restrict self) {
 struct rect area;
 struct display *disp = self->w_display;
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
PRIVATE void KCALL
window_claim_visi(struct window *__restrict self,
                  struct rect display_rect,
                  bool render_immediatly) {
 struct window *iter;
 struct display *d = self->w_display;
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
       window_render_rect(self,common);
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
      window_render_rect(self,common_rect);
  goto again_background_visi;
 }
}


/* Remove everything that was visible within `window_rect'
 * and let that area be inherited by other windows with a
 * lower Z-order.
 * This function is called to release visibility to other
 * windows and the background when a window is moved. */
PRIVATE void KCALL
window_remove_visi_and_inherit(struct window *__restrict self,
                               struct rect window_rect) {
 struct rect r;
 struct rects parts;
 struct display *d = self->w_display;
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
  struct window *iter;
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
   window_render_rect(iter,common);

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
  display_render_background(d,r);
 }
 rects_fini(&parts);
}




/* Construct a new window. */
INTERN struct window *KCALL
window_create(struct display *__restrict disp,
              int posx, unsigned int sizex,
              int posy, unsigned int sizey,
              unsigned int bpp,
              unsigned int flags) {
 struct window *result;
 result = (struct window *)Xmalloc(sizeof(struct window));
 result->w_posx    = posx;
 result->w_posy    = posy;
 result->w_flags   = flags;
 result->w_sizey   = sizey;
 result->w_sizex   = sizex;
 result->w_stride  = CEIL_ALIGN(sizex,16);
 result->w_bpp     = bpp;
 result->w_display = disp;
 result->w_visi.r_strips = NULL;
 window_update_display_area(result);
 result->w_screen  = (byte_t *)Xmalloc(sizey*
                                       result->w_stride*
                                       CEILDIV(bpp,8));
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
 if (sizex && sizey && !(flags & WINDOW_FHIDDEN)) {
  /* Add the window to the visible part of the display render list. */
  LIST_INSERT(disp->d_windows,result,w_vlink);
  window_claim_visi(result,result->w_disparea,false);
 }
 return result;
}



/* Copy window memory from `window_rect' directly onto the screen. */
PRIVATE void KCALL
window_render_rect(struct window *__restrict self,
                   struct rect window_rect) {
 struct display *d = self->w_display;
 copy_rect(d->d_screen,
           window_rect.r_xmin+self->w_posx,
           window_rect.r_ymin+self->w_posy,
           d->d_bpp,
           d->d_stride,
           self->w_screen,
           window_rect.r_xmin,
           window_rect.r_ymin,
           self->w_bpp,
           self->w_stride,
           window_rect.r_xsiz,
           window_rect.r_ysiz);
}

PRIVATE void KCALL
display_render_background(struct display *__restrict self,
                          struct rect display_rect) {
 copy_rect(self->d_screen,
           display_rect.r_xmin,
           display_rect.r_ymin,
           self->d_bpp,
           self->d_stride,
           self->d_backgrnd,
           display_rect.r_xmin,
           display_rect.r_ymin,
           self->d_bpp,
           self->d_stride,
           display_rect.r_xsiz,
           display_rect.r_ysiz);
}


INTERN void KCALL
window_draw_rects(struct window *__restrict self,
                  size_t count, struct rect *__restrict vec) {
 struct rect visi;
 RECTS_FOREACH(visi,self->w_visi) {
  size_t i;
  for (i = 0; i < count; ++i) {
   struct rect common = rect_intersect(visi,vec[i]);
   if (common.r_xsiz && common.r_ysiz)
       window_render_rect(self,common);
  }
 }
}
INTERN void KCALL
window_draw_rect(struct window *__restrict self, struct rect r) {
 struct rect visi;
 RECTS_FOREACH(visi,self->w_visi) {
  struct rect common = rect_intersect(visi,r);
  if (common.r_xsiz && common.r_ysiz)
      window_render_rect(self,common);
 }
}



/* Destroy the given window. */
INTERN void KCALL
window_destroy(struct window *__restrict self) {
 /* Hide the window prior to its destruction to ensure
  * that it doesn't leave behind any artifacts. */
 window_hide(self);
 /* TODO */
 free(self);
}



/* Move around the given window. */
INTERN void KCALL
window_move(struct window *__restrict self,
            int new_posx, int new_posy) {
 int old_posx = self->w_posx;
 int old_posy = self->w_posy;
 int rel_movx = new_posx - old_posx;
 int rel_movy = new_posy - old_posy;
 struct rect r,old_display_area;
 struct rects reclaim_area;
 if (!rel_movx && !rel_movy) return;
 old_display_area = self->w_disparea;
 if (self->w_flags & WINDOW_FHIDDEN) {
  self->w_posx = new_posx;
  self->w_posy = new_posy;
  return;
 }
 
 /* Adjust the set of visible window
  * rects to apply to the new location. */
 if (rel_movx != 0) {
  self->w_posx = new_posx;
  window_update_display_area(self);
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
  window_remove_visi_and_inherit(self,r);
 }
 if (rel_movy != 0) {
  self->w_posy = new_posy;
  window_update_display_area(self);
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
  window_remove_visi_and_inherit(self,r);
 }
 /* Claim visibility for everything apart of the new
  * display area, that isn't part of the old display
  * area. */
 reclaim_area.r_strips = NULL;
 rects_insert(&reclaim_area,self->w_disparea);
 rects_remove(&reclaim_area,old_display_area);
 RECTS_FOREACH(r,reclaim_area) {
  window_claim_visi(self,r,true);
 }
 rects_fini(&reclaim_area);
}


/* Set active window flags. */
INTERN void KCALL
window_hide(struct window *__restrict self) {
 struct window *lower;
 struct display *d;
 struct rect r;
 if (self->w_flags & WINDOW_FHIDDEN) return;
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
   window_render_rect(lower,common_area);
   goto restart_foreach;
  }
 }
 /* Anything that's still left in the set of remaining rects
  * cannot be inherited by another window, meaning it needs to
  * be inherited by the background plane. */
 RECTS_FOREACH(r,self->w_visi) {
  rects_insert(&d->d_backvisi,r);
  /* Render the background for the affected area. */
  display_render_background(d,r);
 }
 /* Free what's left of our rects-copy buffer. */
 rects_fini(&self->w_visi);
 self->w_visi.r_strips = NULL;
 /* Finally, set the hidden flag. */
 self->w_flags |= WINDOW_FHIDDEN;
}
INTERN void KCALL
window_show(struct window *__restrict self) {
 struct rect r;
 if (!(self->w_flags & WINDOW_FHIDDEN)) return;
 r.r_xmin = 0;
 r.r_ymin = 0;
 r.r_xsiz = self->w_sizex;
 r.r_ysiz = self->w_sizey;
 window_claim_visi(self,r,true);
 LIST_INSERT(self->w_display->d_windows,self,w_vlink);
 self->w_flags &= ~WINDOW_FHIDDEN;
}

/* Copy visible window portions also covered by `vec' onto the display buffer. */
INTERN void KCALL
window_draw(struct window *__restrict self,
            size_t count, struct rect *__restrict vec) {
 /* TODO */
}





/* Do a full redraw of all visible windows. */
INTERN void KCALL
display_redraw(struct display *__restrict self) {
 struct rect r;
 struct window *win;
#if 1
 /* Copy the background first. */
 RECTS_FOREACH(r,self->d_backvisi) {
  copy_rect(self->d_screen,
            r.r_xmin,
            r.r_ymin,
            self->d_bpp,
            self->d_stride,
            self->d_backgrnd,
            r.r_xmin,
            r.r_ymin,
            self->d_bpp,
            self->d_stride,
            r.r_xsiz,
            r.r_ysiz);
 }
#endif
 /* Then copy all visible window portions. */
 for (win  = self->d_windows;
      win != NULL; win = win->w_vlink.le_next) {
  RECTS_FOREACH(r,win->w_visi) {
   copy_rect(self->d_screen,
             win->w_posx+r.r_xmin,
             win->w_posy+r.r_ymin,
             self->d_bpp,
             self->d_stride,
             win->w_screen,
             r.r_xmin,
             r.r_ymin,
             win->w_bpp,
             win->w_stride,
             r.r_xsiz,
             r.r_ysiz);
  }
 }
}



#undef CONFIG_COPYRECT_DO_OUTLINE
#define CONFIG_COPYRECT_DO_OUTLINE  4


INTERN void KCALL
copy_rect(byte_t *__restrict dst_buffer,
          unsigned int dst_x, unsigned int dst_y,
          unsigned int dst_bpp, unsigned int dst_stride,
          byte_t const *__restrict src_buffer,
          unsigned int src_x, unsigned int src_y,
          unsigned int src_bpp, unsigned int src_stride,
          unsigned int size_x, unsigned int size_y) {
 assert(size_x || size_y);
#if 1
 syslog(LOG_DEBUG,"copyrect(%u,%u*%u <-- %u,%u*%u  w=%u,h=%u)\n",
        dst_x,dst_y,dst_stride,src_x,src_y,src_stride,size_x,size_y);
#endif
 dst_buffer += dst_y*dst_stride;
 src_buffer += src_y*src_stride;
 if (dst_bpp == src_bpp) {
  if (dst_bpp == 8) {
do_bytewise_copy:
   assert(dst_x+size_x <= dst_stride);
   assert(src_x+size_x <= src_stride);
   dst_buffer += dst_x;
   src_buffer += src_x;
   /* Copy line-wise */
#ifdef CONFIG_COPYRECT_DO_OUTLINE
   --size_y;
   memset(dst_buffer,CONFIG_COPYRECT_DO_OUTLINE,size_x);
   src_buffer += src_stride;
   dst_buffer += dst_stride;
#endif
   for (; size_y; --size_y) {
    memcpy(dst_buffer,
           src_buffer,
           size_x);
#ifdef CONFIG_COPYRECT_DO_OUTLINE
    dst_buffer[0]        = CONFIG_COPYRECT_DO_OUTLINE;
    dst_buffer[size_x-1] = CONFIG_COPYRECT_DO_OUTLINE;
#endif
    dst_buffer += dst_stride;
    src_buffer += src_stride;
   }
#ifdef CONFIG_COPYRECT_DO_OUTLINE
   memset(dst_buffer-dst_stride,
          CONFIG_COPYRECT_DO_OUTLINE,size_x);
#endif
  } else if (!(dst_bpp & 7)) {
   dst_x  *= dst_bpp/8;
   src_x  *= dst_bpp/8;
   size_x *= dst_bpp/8;
   goto do_bytewise_copy;
  } else {
   assertf(0,"TODO");
  }
 } else {
  assertf(0,"TODO");
 }
}


DECL_END

#endif /* !GUARD_APPS_WM_WINDOW_C */
