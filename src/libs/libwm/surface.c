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
#ifndef GUARD_LIBS_LIBWM_SURFACE_C
#define GUARD_LIBS_LIBWM_SURFACE_C 1
#define _EXCEPT_SOURCE 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <hybrid/align.h>
#include <wm/api.h>
#include <wm/server.h>
#include <wm/window.h>
#include <assert.h>
#include <unistd.h>
#include <except.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <malloc.h>

#include "libwm.h"

DECL_BEGIN

PRIVATE struct wm_color_triple const default_color_codes[WM_COLOR_COUNT] = {
    [WM_COLOR_BLACK  ] = { 0, 0, 0 },
    [WM_COLOR_MAROON ] = { 128, 0, 0 },
    [WM_COLOR_GREEN  ] = { 0, 128, 0 },
    [WM_COLOR_OLIVE  ] = { 128, 128, 0 },
    [WM_COLOR_NAVY   ] = { 0, 0, 128 },
    [WM_COLOR_PURPLE ] = { 128, 0, 128 },
    [WM_COLOR_TEAL   ] = { 0, 128, 128 },
    [WM_COLOR_SILVER ] = { 192, 192, 192 },
    [WM_COLOR_GREY   ] = { 128, 128, 128 },
    [WM_COLOR_RED    ] = { 255, 0, 0 },
    [WM_COLOR_LIME   ] = { 0, 255, 0 },
    [WM_COLOR_YELLOW ] = { 255, 255, 0 },
    [WM_COLOR_BLUE   ] = { 0, 0, 255 },
    [WM_COLOR_FUCHSIA] = { 255, 0, 255 },
    [WM_COLOR_AQUA   ] = { 0, 255, 255 },
    [WM_COLOR_WHITE  ] = { 255, 255, 255 } 
};


DEFINE_PUBLIC_ALIAS(wm_palette_create,libwm_palette_create);
INTERN ATTR_RETNONNULL REF struct wm_palette *WMCALL
libwm_palette_create(u16 bpp, u8 mask) {
 REF struct wm_palette *result; u16 count;
 assertf((bpp & (bpp-1)) == 0,"Not a power-of-2: %u",(unsigned)bpp);
 assertf(bpp >= 1 && bpp <= 16,"BPP is too large, or too small: %u",(unsigned)bpp);
 assertf((mask & (mask+1)) == 0,"Mask must be a power-of-2 minus 1: 0x%x",(unsigned)mask);
 count  = 1 << bpp;
 /* Allocate the palette */
 result = (REF struct wm_palette *)Xmalloc(offsetof(struct wm_palette,p_colors)+
                                           count*sizeof(struct wm_color_triple));
 result->p_refcnt = 1;
 result->p_bpp    = bpp;
 result->p_mask   = mask;
 result->p_count  = count;
 return result;
}

DEFINE_PUBLIC_ALIAS(wm_palette_destroy,libwm_palette_destroy);
INTERN void WMCALL
libwm_palette_destroy(struct wm_palette *__restrict self) {
 free(self);
}


DEFINE_PUBLIC_ALIAS(wm_format_create,libwm_format_create);
INTERN ATTR_RETNONNULL REF struct wm_format *WMCALL
libwm_format_create(wm_pixel_t rmask, u16 rshft,
                    wm_pixel_t gmask, u16 gshft,
                    wm_pixel_t bmask, u16 bshft,
                    wm_pixel_t amask, u16 ashft) {
 REF struct wm_format *result;
 wm_pixel_t max_mask; unsigned int i;
 result = (REF struct wm_format *)Xmalloc(sizeof(struct wm_format));
 result->f_refcnt = 1;
 result->f_pal    = NULL;
 result->f_rmask  = rmask;
 result->f_gmask  = gmask;
 result->f_bmask  = bmask;
 result->f_amask  = amask;
 result->f_rshft  = rshft;
 result->f_gshft  = gshft;
 result->f_bshft  = bshft;
 result->f_ashft  = ashft;
 /* Determine the mask BPP. */
 max_mask = rmask|gmask|bmask|amask;
 for (i = 0; i < 32 && (max_mask > (unsigned int)(1 << i)); ++i);
 result->f_bpp = i;
 /* Calculate standard colors. */
 for (i = 0; i < WM_COLOR_COUNT; ++i) {
  result->f_color[i] = ((((wm_pixel_t)default_color_codes[i].ct_red   << rshft) & rmask) |
                        (((wm_pixel_t)default_color_codes[i].ct_green << gshft) & gmask) |
                        (((wm_pixel_t)default_color_codes[i].ct_blue  << bshft) & bmask));
 }
 return result;
}

DEFINE_PUBLIC_ALIAS(wm_format_create_pal,libwm_format_create_pal);
INTERN ATTR_RETNONNULL REF struct wm_format *WMCALL
libwm_format_create_pal(struct wm_palette *__restrict pal) {
 REF struct wm_format *result;
 result = (REF struct wm_format *)Xmalloc(sizeof(struct wm_format));
 result->f_refcnt = 1;
 result->f_pal    = pal;
 result->f_bpp    = pal->p_bpp;
 result->f_ops    = libwm_lookup_surface_ops(pal->p_bpp);
 /* TODO: Lookup standard color codes. */
 return result;
}

DEFINE_PUBLIC_ALIAS(wm_format_destroy,libwm_format_destroy);
INTERN void WMCALL
libwm_format_destroy(struct wm_format *__restrict self) {
 if (self->f_pal)
     libwm_palette_decref(self->f_pal);
 free(self);
}




DEFINE_PUBLIC_ALIAS(wm_surface_create,libwm_surface_create);
INTERN ATTR_RETNONNULL REF struct wm_surface *WMCALL
libwm_surface_create(struct wm_format *__restrict format,
                     unsigned int sizex, unsigned int sizey) {
 REF struct wm_surface *result;
 result = (REF struct wm_surface *)Xmalloc(sizeof(struct wm_surface));
 result->s_refcnt = 1;
 result->s_ops    = format->f_ops;
 result->s_flags  = WM_SURFACE_FNORMAL;
 result->s_bpp    = format->f_bpp;
 atomic_rwlock_init(&result->s_lock);
 result->s_sizex  = sizex;
 result->s_sizey  = sizey;
 result->s_stride = CEIL_ALIGN(sizex,16);
 TRY {
  /* Allocate the surface buffer. */
  result->s_buffer = (byte_t *)Xmalloc(CEILDIV(result->s_stride*
                                               sizey*
                                               format->f_bpp,
                                               8));
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  free(result);
  error_rethrow();
 }
 libwm_format_incref(format);
 return result;
}


DEFINE_PUBLIC_ALIAS(wm_surface_resize,libwm_surface_resize);
INTERN void WMCALL
libwm_surface_resize(struct wm_surface *__restrict self,
                     unsigned int new_sizx,
                     unsigned int new_sizy) {
 if (self->s_flags & WM_SURFACE_FWINDOW)
     libwm_window_resize((struct wm_window *)self,new_sizx,new_sizy);
 else {
  /* TODO */
 }
}

DEFINE_PUBLIC_ALIAS(wm_surface_convert,libwm_surface_convert);
INTERN ATTR_RETNONNULL REF struct wm_surface *WMCALL
libwm_surface_convert(struct wm_surface *__restrict self,
                      struct wm_format *__restrict new_format) {
 REF struct wm_surface *COMPILER_IGNORE_UNINITIALIZED(result);
 if (self->s_format == new_format) {
  result = self;
  libwm_surface_incref(result);
 } else {
  wm_surface_lock(self);
  TRY {
   /* Create a new surface */
   result = libwm_surface_create(new_format,
                                 self->s_sizex,
                                 self->s_sizey);
   /* Blit the old surface onto the new one. */
   wm_surface_bblit(result,
                    0,
                    0,
                    self,
                    0,
                    0,
                    self->s_sizex,
                    self->s_sizey);
  } FINALLY {
   wm_surface_unlock(self);
  }
 }
 return result;
}



DEFINE_PUBLIC_ALIAS(wm_surface_destroy,libwm_surface_destroy);
INTERN void WMCALL
libwm_surface_destroy(struct wm_surface *__restrict self) {
 if (self->s_flags & WM_SURFACE_FWINDOW)
     libwm_window_destroy((struct wm_window *)self);
 else {
  libwm_format_decref(self->s_format);
  free(self->s_buffer);
  free(self);
 }
}


DECL_END

#endif /* !GUARD_LIBS_LIBWM_SURFACE_C */
