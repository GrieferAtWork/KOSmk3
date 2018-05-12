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

PRIVATE struct wm_color const default_color_codes[WM_COLOR_COUNT] = {
    [WM_COLOR_BLACK  ] = WM_COLOR_INIT(0,   0,   0,   255),
    [WM_COLOR_MAROON ] = WM_COLOR_INIT(128, 0,   0,   255),
    [WM_COLOR_GREEN  ] = WM_COLOR_INIT(0,   128, 0,   255),
    [WM_COLOR_OLIVE  ] = WM_COLOR_INIT(128, 128, 0,   255),
    [WM_COLOR_NAVY   ] = WM_COLOR_INIT(0,   0,   128, 255),
    [WM_COLOR_PURPLE ] = WM_COLOR_INIT(128, 0,   128, 255),
    [WM_COLOR_TEAL   ] = WM_COLOR_INIT(0,   128, 128, 255),
    [WM_COLOR_SILVER ] = WM_COLOR_INIT(192, 192, 192, 255),
    [WM_COLOR_GREY   ] = WM_COLOR_INIT(128, 128, 128, 255),
    [WM_COLOR_RED    ] = WM_COLOR_INIT(255, 0,   0,   255),
    [WM_COLOR_LIME   ] = WM_COLOR_INIT(0,   255, 0,   255),
    [WM_COLOR_YELLOW ] = WM_COLOR_INIT(255, 255, 0,   255),
    [WM_COLOR_BLUE   ] = WM_COLOR_INIT(0,   0,   255, 255),
    [WM_COLOR_FUCHSIA] = WM_COLOR_INIT(255, 0,   255, 255),
    [WM_COLOR_AQUA   ] = WM_COLOR_INIT(0,   255, 255, 255),
    [WM_COLOR_WHITE  ] = WM_COLOR_INIT(255, 255, 255, 255)
};


DEFINE_PUBLIC_ALIAS(wm_palette_create,libwm_palette_create);
INTERN ATTR_RETNONNULL REF struct wm_palette *WMCALL
libwm_palette_create(unsigned int bpp) {
 REF struct wm_palette *result; u16 count;
 assertf((bpp & (bpp-1)) == 0,"Not a power-of-2: %u",(unsigned)bpp);
 assertf(bpp >= 1 && bpp <= 16,"BPP is too large, or too small: %u",(unsigned)bpp);
 count  = 1 << bpp;
 /* Allocate the palette */
 result = (REF struct wm_palette *)Xmalloc(offsetof(struct wm_palette,p_colors)+
                                           count*sizeof(struct wm_color_triple));
 result->p_refcnt = 1;
 result->p_bpp    = bpp;
 result->p_count  = count;
 return result;
}

DEFINE_PUBLIC_ALIAS(wm_palette_destroy,libwm_palette_destroy);
INTERN void WMCALL
libwm_palette_destroy(struct wm_palette *__restrict self) {
 free(self);
}

INTERN ATTR_NOTHROW struct wm_color WMCALL
rgba_format_colorof(struct wm_format const *__restrict self, wm_pixel_t pixel) {
 struct wm_color result;
 result.c_red   = (pixel & self->f_rmask) >> self->f_rshft;
 result.c_green = (pixel & self->f_gmask) >> self->f_gshft;
 result.c_blue  = (pixel & self->f_bmask) >> self->f_bshft;
 result.c_alpha = (pixel & self->f_amask) >> self->f_ashft;
 return result;
}
INTERN ATTR_NOTHROW wm_pixel_t WMCALL
rgba_format_pixelof(struct wm_format const *__restrict self,
                    struct wm_color color) {
 wm_pixel_t result;
 result  = (color.c_red   << self->f_rshft) & self->f_rmask;
 result |= (color.c_green << self->f_gshft) & self->f_gmask;
 result |= (color.c_blue  << self->f_bshft) & self->f_bmask;
 result |= (color.c_alpha << self->f_ashft) & self->f_amask;
 return result;
}


DEFINE_PUBLIC_ALIAS(wm_format_create,libwm_format_create);
INTERN ATTR_RETNONNULL REF struct wm_format *WMCALL
libwm_format_create(wm_pixel_t rmask, u16 rshft,
                    wm_pixel_t gmask, u16 gshft,
                    wm_pixel_t bmask, u16 bshft,
                    wm_pixel_t amask, u16 ashft,
                    unsigned int bpp) {
 REF struct wm_format *result;
 unsigned int i;
 assertf((bpp & (bpp-1)) == 0 || bpp == 24,"Not a power-of-2: %u",(unsigned)bpp);
 assertf(bpp >= 1 && bpp <= 32,"BPP is too large, or too small: %u",(unsigned)bpp);
 result = (REF struct wm_format *)Xmalloc(sizeof(struct wm_format));
 result->f_refcnt  = 1;
 result->f_pal     = NULL;
 result->f_rmask   = rmask;
 result->f_gmask   = gmask;
 result->f_bmask   = bmask;
 result->f_amask   = amask;
 result->f_rshft   = rshft;
 result->f_gshft   = gshft;
 result->f_bshft   = bshft;
 result->f_ashft   = ashft;
 result->f_colorof = &rgba_format_colorof;
 result->f_pixelof = &rgba_format_pixelof;
 result->f_bpp     = bpp;
 /* Calculate standard colors. */
 for (i = 0; i < WM_COLOR_COUNT; ++i) {
  result->f_color[i] = ((((wm_pixel_t)default_color_codes[i].c_red   << rshft) & rmask) |
                        (((wm_pixel_t)default_color_codes[i].c_green << gshft) & gmask) |
                        (((wm_pixel_t)default_color_codes[i].c_blue  << bshft) & bmask) |
                           amask); /* Max alpha, thus disabling transparency */
 }
 return result;
}


#if 0 /* XXX: This function might be useful to the window server... */
LOCAL u8 WMCALL scale_mask(u8 color, u8 mask) {
 assert((color & ~mask) == 0);
#if 0
 return (u8)((double)(color/mask)*256);
#else
 switch (mask) {
/*[[[deemon
#include <util>
for (local x: [0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f]) {
    print " case 0x%.2I8x: {" % x;
    print "  PRIVATE u8 const tab["+(x+1)+"] = {",;
    local digits = [none] * (x+1);
    for (local i: util.range(x+1)) {
        if (i == x) {
            digits[i] = 0xff;
        } else {
            digits[i] = ((uint8_t)(((float)i / x)*256));
        }
    }
    for (local i,v: util.enumerate(digits)) {
        if ((i % 16) == 0) {
            if (x >= 15) {
                print "\n   ",;
            } else {
                print " ",;
            }
        }
        print "0x%.2I8x," % v,;
    }
    print " };";
    print "  return tab[color];";
    print " }";
}
]]]*/
 case 0x01: {
  PRIVATE u8 const tab[2] = { 0x00,0xff, };
  return tab[color];
 }
 case 0x03: {
  PRIVATE u8 const tab[4] = { 0x00,0x55,0xaa,0xff, };
  return tab[color];
 }
 case 0x07: {
  PRIVATE u8 const tab[8] = { 0x00,0x24,0x49,0x6d,0x92,0xb6,0xdb,0xff, };
  return tab[color];
 }
 case 0x0f: {
  PRIVATE u8 const tab[16] = {
   0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff, };
  return tab[color];
 }
 case 0x1f: {
  PRIVATE u8 const tab[32] = {
   0x00,0x08,0x10,0x18,0x21,0x29,0x31,0x39,0x42,0x4a,0x52,0x5a,0x63,0x6b,0x73,0x7b,
   0x84,0x8c,0x94,0x9c,0xa5,0xad,0xb5,0xbd,0xc6,0xce,0xd6,0xde,0xe7,0xef,0xf7,0xff, };
  return tab[color];
 }
 case 0x3f: {
  PRIVATE u8 const tab[64] = {
   0x00,0x04,0x08,0x0c,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2c,0x30,0x34,0x38,0x3c,
   0x41,0x45,0x49,0x4d,0x51,0x55,0x59,0x5d,0x61,0x65,0x69,0x6d,0x71,0x75,0x79,0x7d,
   0x82,0x86,0x8a,0x8e,0x92,0x96,0x9a,0x9e,0xa2,0xa6,0xaa,0xae,0xb2,0xb6,0xba,0xbe,
   0xc3,0xc7,0xcb,0xcf,0xd3,0xd7,0xdb,0xdf,0xe3,0xe7,0xeb,0xef,0xf3,0xf7,0xfb,0xff, };
  return tab[color];
 }
 case 0x7f: {
  PRIVATE u8 const tab[128] = {
   0x00,0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,0x10,0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,
   0x20,0x22,0x24,0x26,0x28,0x2a,0x2c,0x2e,0x30,0x32,0x34,0x36,0x38,0x3a,0x3c,0x3e,
   0x40,0x42,0x44,0x46,0x48,0x4a,0x4c,0x4e,0x50,0x52,0x54,0x56,0x58,0x5a,0x5c,0x5e,
   0x60,0x62,0x64,0x66,0x68,0x6a,0x6c,0x6e,0x70,0x72,0x74,0x76,0x78,0x7a,0x7c,0x7e,
   0x81,0x83,0x85,0x87,0x89,0x8b,0x8d,0x8f,0x91,0x93,0x95,0x97,0x99,0x9b,0x9d,0x9f,
   0xa1,0xa3,0xa5,0xa7,0xa9,0xab,0xad,0xaf,0xb1,0xb3,0xb5,0xb7,0xb9,0xbb,0xbd,0xbf,
   0xc1,0xc3,0xc5,0xc7,0xc9,0xcb,0xcd,0xcf,0xd1,0xd3,0xd5,0xd7,0xd9,0xdb,0xdd,0xdf,
   0xe1,0xe3,0xe5,0xe7,0xe9,0xeb,0xed,0xef,0xf1,0xf3,0xf5,0xf7,0xf9,0xfb,0xfd,0xff, };
  return tab[color];
 }
//[[[end]]]
 case 0xff: return color;
 default: __builtin_unreachable();
 }
#endif
}
#endif

INTERN ATTR_NOTHROW struct wm_color WMCALL
pal_format_colorof(struct wm_format const *__restrict self, wm_pixel_t pixel) {
 struct wm_color result;
 if (pixel >= self->f_pal->p_count) {
  result.c_red   = 0;
  result.c_green = 0;
  result.c_blue  = 0;
  result.c_alpha = 0;
 } else {
  result.c_triple = self->f_pal->p_colors[pixel];
  result.c_alpha  = 0xff; /* Not supported by the palette model */
 }
 return result;
}

INTERN u32 WMCALL
libwm_color_compare(struct wm_color_triple a,
                    struct wm_color_triple b) {
#if 0
 /* TODO: Compare based on what the human eye sees. */
#else
#define SWAP(a,b) { u8 temp = a; a = b; b = temp; }
 if (a.c_red   < b.c_red)   SWAP(a.c_red,b.c_red);
 if (a.c_green < b.c_green) SWAP(a.c_green,b.c_green);
 if (a.c_blue  < b.c_blue ) SWAP(a.c_blue,b.c_blue);
#undef SWAP
 return ((a.c_red - b.c_red) +
         (a.c_green - b.c_green) +
         (a.c_blue - b.c_blue));
#endif
}


INTERN ATTR_NOTHROW wm_pixel_t WMCALL
pal_format_pixelof(struct wm_format const *__restrict self,
                   struct wm_color color) {
 wm_pixel_t COMPILER_IGNORE_UNINITIALIZED(result);
 struct wm_palette *pal = self->f_pal; wm_pixel_t i;
 u32 highscore = (u32)-1;
 for (i = 0; i < pal->p_count; ++i) {
  u32 score;
  score = libwm_color_compare(pal->p_colors[i],
                              color.c_triple);
  if (highscore > score)
      highscore = score,
      result = i;
 }
 return result;
}


DEFINE_PUBLIC_ALIAS(wm_format_create_pal,libwm_format_create_pal);
INTERN ATTR_RETNONNULL REF struct wm_format *WMCALL
libwm_format_create_pal(struct wm_palette *__restrict pal, unsigned int bpp) {
 REF struct wm_format *result;
 unsigned int i;
 assertf((bpp & (bpp-1)) == 0,"Not a power-of-2: %u",(unsigned)bpp);
 assertf(bpp >= 1 && bpp <= 16,"BPP is too large, or too small: %u",(unsigned)bpp);
 assertf(bpp <= pal->p_bpp,"BPP is larger than supported by the palette");
 result = (REF struct wm_format *)Xmalloc(sizeof(struct wm_format));
 result->f_refcnt  = 1;
 result->f_pal     = pal;
 result->f_bpp     = bpp;
 result->f_colorof = &pal_format_colorof;
 result->f_pixelof = &pal_format_pixelof;
 /* Calculate standard colors. */
 for (i = 0; i < WM_COLOR_COUNT; ++i) {
  result->f_color[i] = pal_format_pixelof(result,
                                          default_color_codes[i]);
 }
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
 if unlikely(!sizex || !sizey)
    error_throw(E_INVALID_ARGUMENT);
 if unlikely(sizex > WM_MAX_SURFACE_SIZE ||
             sizey > WM_MAX_SURFACE_SIZE)
    error_throw(E_INVALID_ARGUMENT);
 result = (REF struct wm_surface *)Xmalloc(sizeof(struct wm_surface));
 result->s_refcnt = 1;
 result->s_flags  = WM_SURFACE_FNORMAL;
 atomic_rwlock_init(&result->s_lock);
 result->s_sizex  = sizex;
 result->s_sizey  = sizey;
 result->s_stride = CEIL_ALIGN(CEILDIV(sizex * format->f_bpp,8),16);
 libwm_setup_surface_ops(result);
 TRY {
  /* Allocate the surface buffer. */
  result->s_buffer = (byte_t *)Xmalloc(result->s_stride * sizey);
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
 if unlikely(!new_sizx || !new_sizy)
    error_throw(E_INVALID_ARGUMENT);
 if unlikely(new_sizx > WM_MAX_SURFACE_SIZE ||
             new_sizy > WM_MAX_SURFACE_SIZE)
    error_throw(E_INVALID_ARGUMENT);
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
INTERN ATTR_NOTHROW void WMCALL
libwm_surface_destroy(struct wm_surface *__restrict self) {
 if (self->s_flags & WM_SURFACE_FWINDOW)
     libwm_window_destroy((struct wm_window *)self);
 else {
  libwm_format_decref(self->s_format);
  free(self->s_buffer);
  free(self);
 }
}


DEFINE_PUBLIC_ALIAS(wm_surface_makeview,libwm_surface_makeview);
INTERN ATTR_NOTHROW ATTR_RETNONNULL struct wm_surface_view *WMCALL
libwm_surface_makeview(struct wm_surface_view *__restrict view,
                       struct wm_surface const *__restrict surface,
                       int posx, int posy, unsigned int sizex,
                       unsigned int sizey) {
 if (surface->s_flags & WM_SURFACE_FISVIEW) {
  /* Deal with recursive sub-views. */
  posx += ((struct wm_surface_view *)surface)->s_offx;
  posy += ((struct wm_surface_view *)surface)->s_offy;
 }

 view->s_format  = surface->s_format;
 view->s_flags   = WM_SURFACE_FNORMAL;
 atomic_rwlock_init(&view->s_lock);
 view->s_sizex   = sizex;
 view->s_sizey   = sizey;
 view->s_stride  = surface->s_stride;
 view->s_buffer  = surface->s_buffer;

 /* Truncate the view on the X and Y axis. */
 /**/ if ((posx + sizex) <= 0) sizex = 0;
 else if ((posx + sizex) > surface->s_sizex) {
  sizex = surface->s_sizex - posx;
 }
 
 /**/ if ((posy + sizey) <= 0) sizey = 0;
 else if ((posy + sizey) > surface->s_sizey) {
  sizey = surface->s_sizey - posy;
 }

 /* Setup pixel offsets and truncate the buffer view. */
 if (posy < 0) {
  view->s_offy = posy;
 } else {
  view->s_offy    = 0;
  view->s_buffer += posy * surface->s_stride;
 }
 if (posx < 0) {
  view->s_offx = posx;
 } else {
  posx *= surface->s_format->f_bpp;
  view->s_buffer += posx / 8;
  view->s_offx    = posx % 8;
 }
 /* Setup view operators. */
 libwm_setup_surface_view_ops(view);

 return view;
}



DECL_END

#endif /* !GUARD_LIBS_LIBWM_SURFACE_C */
