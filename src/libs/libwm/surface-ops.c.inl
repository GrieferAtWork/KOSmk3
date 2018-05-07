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
#ifdef __INTELLISENSE__
#include <wm/surface.h>
#include <kos/types.h>
#include <string.h>
#define BPP        8
#define FUNC(name) surface8_##name
#endif

DECL_BEGIN

#ifdef FOR_SURFACE_VIEW
#define ADJUST_COORDS    \
   ((x) += ((struct wm_surface_view *)self)->s_offx, \
    (y) += ((struct wm_surface_view *)self)->s_offy)
#else
#define ADJUST_COORDS    \
   (void)0
#endif

PRIVATE wm_pixel_t WMCALL
FUNC(getpixel)(struct wm_surface const *__restrict self,
               int x, int y) {
#ifdef EMPTY_SURFACE
 return self->s_format->f_color[WM_COLOR_BLACK];
#else
 byte_t *ptr;
 ADJUST_COORDS;
 if ((unsigned int)x >= self->s_sizex ||
     (unsigned int)y >= self->s_sizey)
      return self->s_format->f_color[WM_COLOR_BLACK];
 ptr  = self->s_buffer;
 ptr += y * self->s_stride; /* XXX: Use shifts here if possible? */
#if BPP == 32
 ptr += x*4;
 return *(u32 *)ptr;
#elif BPP == 16
 ptr += x*2;
 return *(u16 *)ptr;
#elif BPP == 8
 ptr += x;
 return *(u8 *)ptr;
#elif BPP == 4
 x  <<= 2;
 ptr += x >> 3;
 x   &= 0x7;
 assert(x == 0 || x == 4);
 return (*(u8 *)ptr >> x) & 0xf;
#elif BPP == 2
 x  <<= 1;
 ptr += x >> 3;
 x   &= 0x7;
 assert(x == 0 || x == 2 || x == 4 || x == 6);
 return (*(u8 *)ptr >> x) & 0x3;
#else
 ptr += x >> 3;
 x   &= 0x7;
 return (*(u8 *)ptr >> x) & 0x1;
#endif
#endif
}

PRIVATE void WMCALL
FUNC(setpixel)(struct wm_surface *__restrict self,
               int x, int y, wm_pixel_t pixel) {
#ifndef EMPTY_SURFACE
 byte_t *ptr;
 ADJUST_COORDS;
 if ((unsigned int)x >= self->s_sizex ||
     (unsigned int)y >= self->s_sizey)
      return;
 ptr  = self->s_buffer;
 ptr += y * self->s_stride; /* XXX: Use shifts here if possible? */
#if BPP == 32
 ptr += x*4;
 *(u32 *)ptr = (u32)pixel;
#elif BPP == 16
 ptr += x*2;
 *(u16 *)ptr = (u16)pixel;
#elif BPP == 8
 ptr += x;
 *(u8 *)ptr = (u8)pixel;
#elif BPP == 4
 x  <<= 2;
 ptr += x >> 3;
 x   &= 0x7;
 assert(x == 0 || x == 4);
 *(u8 *)ptr &= ~(0xf << x);
 *(u8 *)ptr |= (pixel & 0xf) << x;
#elif BPP == 2
 x  <<= 1;
 ptr += x >> 3;
 x   &= 0x7;
 assert(x == 0 || x == 2 || x == 4 || x == 6);
 *(u8 *)ptr &= ~(0x3 << x);
 *(u8 *)ptr |= (pixel & 0x3) << x;
#else
 ptr += x >> 3;
 x   &= 0x7;
 *(u8 *)ptr &= ~(0x1 << x);
 *(u8 *)ptr |= (pixel & 0x1) << x;
#endif
#endif
}

PRIVATE void WMCALL
FUNC(hline)(struct wm_surface *__restrict self,
            int x, int y, unsigned int size_x,
            wm_pixel_t pixel) {
#ifndef EMPTY_SURFACE
 unsigned int i;
 /* TODO: Optimizations. */
 for (i = 0; i < size_x; ++i)
     FUNC(setpixel)(self,x+i,y,pixel);
#endif
}

PRIVATE void WMCALL
FUNC(vline)(struct wm_surface *__restrict self,
            int x, int y, unsigned int size_y,
            wm_pixel_t pixel) {
#ifndef EMPTY_SURFACE
 unsigned int i;
 /* TODO: Optimizations. */
 for (i = 0; i < size_y; ++i)
     FUNC(setpixel)(self,x,y+i,pixel);
#endif
}

PRIVATE void WMCALL
FUNC(rect)(struct wm_surface *__restrict self,
           int x, int y, unsigned int size_x,
           unsigned int size_y, wm_pixel_t pixel) {
#ifndef EMPTY_SURFACE
 if (!size_x || !size_y) return;
 /* TODO: Optimizations. */
 FUNC(hline)(self,x,y,size_x,pixel);
 FUNC(hline)(self,x,y+size_y,size_x,pixel);
 FUNC(vline)(self,x,y+1,size_y-2,pixel);
 FUNC(vline)(self,x+size_x,y+1,size_y-2,pixel);
#endif
}

PRIVATE void WMCALL
FUNC(fill)(struct wm_surface *__restrict self,
           int x, int y, unsigned int size_x,
           unsigned int size_y, wm_pixel_t pixel) {
#ifndef EMPTY_SURFACE
 unsigned int i;
 /* TODO: Optimizations. */
 for (i = 0; i < size_y; ++i)
     FUNC(hline)(self,x,y+i,size_x,pixel);
#endif
}

PRIVATE void WMCALL
FUNC(bblit)(struct wm_surface *__restrict self, int x, int y,
            struct wm_surface const *__restrict source,
            int source_x, int source_y,
            unsigned int size_x, unsigned int size_y) {
#ifndef EMPTY_SURFACE
 unsigned int i,j;
 ADJUST_COORDS;
 if (source->s_flags & WM_SURFACE_FISVIEW) {
  source_x += ((struct wm_surface_view *)source)->s_offx;
  source_y += ((struct wm_surface_view *)source)->s_offy;
 }
 /* TODO: Optimizations. */
 if (self->s_format == source->s_format) {
  for (j = 0; j < size_y; ++j) {
   for (i = 0; i < size_x; ++i) {
    FUNC(setpixel)(self,x+i,y+j,
                   wm_surface_getpixel(source,
                                       source_x+i,
                                       source_y+j));
   }
  }
 } else {
  struct wm_format *dfmt = self->s_format;
  struct wm_format *sfmt = source->s_format;
  for (j = 0; j < size_y; ++j) {
   for (i = 0; i < size_x; ++i) {
    struct wm_color color;
    color = wm_format_colorof(sfmt,
                              wm_surface_getpixel(source,
                                                  source_x+i,
                                                  source_y+j));
    FUNC(setpixel)(self,x+i,y+j,
                   wm_format_pixelof(dfmt,color));
   }
  }
 }
#endif
}

PRIVATE void WMCALL
FUNC(cblit)(struct wm_surface *__restrict self, int x, int y,
            struct wm_surface const *__restrict source,
            int source_x, int source_y,
            unsigned int size_x, unsigned int size_y,
            wm_pixel_t color_key) {
#ifndef EMPTY_SURFACE
 unsigned int i,j;
 if (source->s_flags & WM_SURFACE_FISVIEW) {
  source_x += ((struct wm_surface_view *)source)->s_offx;
  source_y += ((struct wm_surface_view *)source)->s_offy;
 }

 /* TODO: Optimizations. */
 if (self->s_format == source->s_format) {
  for (j = 0; j < size_y; ++j) {
   for (i = 0; i < size_x; ++i) {
    wm_pixel_t pixel;
    pixel = wm_surface_getpixel(source,
                                source_x+i,
                                source_y+j);
    if (pixel == color_key) continue;
    FUNC(setpixel)(self,x+i,y+j,pixel);
   }
  }
 } else {
  struct wm_format *dfmt = self->s_format;
  struct wm_format *sfmt = source->s_format;
  for (j = 0; j < size_y; ++j) {
   for (i = 0; i < size_x; ++i) {
    wm_pixel_t pixel;
    struct wm_color color;
    pixel = wm_surface_getpixel(source,
                                source_x+i,
                                source_y+j);
    if (pixel == color_key) continue;
    color = wm_format_colorof(sfmt,pixel);
    FUNC(setpixel)(self,x+i,y+j,
                   wm_format_pixelof(dfmt,color));
   }
  }
 }
#endif
}

PRIVATE struct wm_surface_ops FUNC(ops) = {
    .so_getpixel = &FUNC(getpixel),
    .so_setpixel = &FUNC(setpixel),
    .so_hline    = &FUNC(hline),
    .so_vline    = &FUNC(vline),
    .so_rect     = &FUNC(rect),
    .so_fill     = &FUNC(fill),
    .so_bblit    = &FUNC(bblit),
    .so_cblit    = &FUNC(cblit)
};


#undef ADJUST_COORDS

DECL_END

#undef EMPTY_SURFACE
#undef FUNC
#undef BPP
