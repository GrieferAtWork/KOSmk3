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
#include <hybrid/byteswap.h>
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


PRIVATE struct PACKED {
    WM_PALETTE_TYPE(2)
} libwm_palette_2 = {
    .p_refcnt = 0x3fffffff,
    .p_bpp    = 1,
    .p_count  = 2,
    .p_colors = {
        {0x00,0x00,0x00},
        {0xff,0xff,0xff}
    }
};

PRIVATE struct PACKED {
    WM_PALETTE_TYPE(4)
} libwm_palette_4 = {
    .p_refcnt = 0x3fffffff,
    .p_bpp    = 2,
    .p_count  = 4,
    .p_colors = {
        {0x00,0x00,0x00},
        {0x55,0x55,0x55},
        {0xaa,0xaa,0xaa},
        {0xff,0xff,0xff}
    }
};

PRIVATE struct PACKED {
    WM_PALETTE_TYPE(16)
} libwm_palette_16 = {
    .p_refcnt = 0x3fffffff,
    .p_bpp    = 4,
    .p_count  = 16,
    .p_colors = {
        {0x00,0x00,0x00},
        {0x11,0x11,0x11},
        {0x22,0x22,0x22},
        {0x33,0x33,0x33},
        {0x44,0x44,0x44},
        {0x55,0x55,0x55},
        {0x66,0x66,0x66},
        {0x77,0x77,0x77},
        {0x88,0x88,0x88},
        {0x99,0x99,0x99},
        {0xaa,0xaa,0xaa},
        {0xbb,0xbb,0xbb},
        {0xcc,0xcc,0xcc},
        {0xdd,0xdd,0xdd},
        {0xee,0xee,0xee},
        {0xff,0xff,0xff}
    }
};

FORCELOCAL struct wm_format *WMCALL
libwm_lazy_alloc_format_pal(struct wm_format **__restrict pformat,
                            struct wm_palette *__restrict pal) {
 struct wm_format *result,*new_result;
 result = ATOMIC_READ(*pformat);
 if (result) return result;
 result = libwm_format_create_pal(pal);
 new_result = ATOMIC_CMPXCH_VAL(*pformat,NULL,result);
 if unlikely(new_result) {
  libwm_format_decref(result);
  result = new_result;
 }
 return result;
}

FORCELOCAL struct wm_format *WMCALL
libwm_lazy_alloc_format(struct wm_format **__restrict pformat,
                        wm_pixel_t rmask, u16 rshft,
                        wm_pixel_t gmask, u16 gshft,
                        wm_pixel_t bmask, u16 bshft,
                        wm_pixel_t amask, u16 ashft,
                        unsigned int bpp) {
 struct wm_format *result,*new_result;
 result = ATOMIC_READ(*pformat);
 if (result) return result;
 result = libwm_format_create(rmask,rshft,gmask,gshft,
                              bmask,bshft,amask,ashft,
                              bpp);
 new_result = ATOMIC_CMPXCH_VAL(*pformat,NULL,result);
 if unlikely(new_result) {
  libwm_format_decref(result);
  result = new_result;
 }
 return result;
}


/* Needs to be initialized statically to ensure that
 * `WM_FONT_SYSTEM_DEFAULT' can be pre-allocated
 * statically as well. */
INTERN struct wm_format libwm_format_monochrome = {
    .f_refcnt  = 0x3fffffff,
    .f_pal     = (struct wm_palette *)&libwm_palette_2,
    .f_color   = {
        [WM_COLOR_BLACK]   = 0, /* Black    #000000 rgb(0,0,0) */
        [WM_COLOR_MAROON]  = 0, /* Maroon   #800000 rgb(128,0,0) */
        [WM_COLOR_GREEN]   = 0, /* Green    #008000 rgb(0,128,0) */
        [WM_COLOR_OLIVE]   = 1, /* Olive    #808000 rgb(128,128,0) */
        [WM_COLOR_NAVY]    = 0, /* Navy     #000080 rgb(0,0,128) */
        [WM_COLOR_PURPLE]  = 1, /* Purple   #800080 rgb(128,0,128) */
        [WM_COLOR_TEAL]    = 1, /* Teal     #008080 rgb(0,128,128) */
        [WM_COLOR_SILVER]  = 1, /* Silver   #c0c0c0 rgb(192,192,192) */
        [WM_COLOR_GREY]    = 1, /* Grey     #808080 rgb(128,128,128) */
        [WM_COLOR_RED]     = 0, /* Red      #ff0000 rgb(255,0,0) */
        [WM_COLOR_LIME]    = 0, /* Lime     #00ff00 rgb(0,255,0) */
        [WM_COLOR_YELLOW]  = 1, /* Yellow   #ffff00 rgb(255,255,0) */
        [WM_COLOR_BLUE]    = 0, /* Blue     #0000ff rgb(0,0,255) */
        [WM_COLOR_FUCHSIA] = 1, /* Fuchsia  #ff00ff rgb(255,0,255) */
        [WM_COLOR_AQUA]    = 1, /* Aqua     #00ffff rgb(0,255,255) */
        [WM_COLOR_WHITE]   = 1, /* White    #ffffff rgb(255,255,255) */
    },
    .f_bpp     = 1,
    .f_colorof = &pal_format_colorof,
    .f_pixelof = &pal_format_pixelof,
};


/* Lookup/create a standard pixel format, given its `name'
 * @return: * : A pointer to the pixel format in question.
 *              This format is lazily allocated and loaded upon first
 *              access by the calling window client application.
 *              During this initial load, the function may throw any
 *              of the following errors, however will not throw any
 *              errors after a successful first call when passing the
 *              same `name' once again.
 * @param: name: One of `WM_FORMAT_*'
 * @throw: E_INVALID_ARGUMENT: The given `name' isn't a valid system font name.
 * @throw: E_BADALLOC: [...] */
DEFINE_PUBLIC_ALIAS(wm_format_lookup,libwm_format_lookup);
INTERN ATTR_RETNONNULL struct wm_format *WMCALL
libwm_format_lookup(unsigned int name) {
 struct wm_format *result;
 switch (name) {

 case WM_FORMAT_BLACK_AND_WHITE:
  result = &libwm_format_monochrome;
  break;

 case WM_FORMAT_GRAYSCALE4: {
  PRIVATE struct wm_format *format = NULL;
  result = libwm_lazy_alloc_format_pal(&format,
                                      (struct wm_palette *)&libwm_palette_4);
 } break;

 case WM_FORMAT_GRAYSCALE16: {
  PRIVATE struct wm_format *format = NULL;
  result = libwm_lazy_alloc_format_pal(&format,
                                      (struct wm_palette *)&libwm_palette_16);
 } break;

 case WM_FORMAT_BGR24: {
  PRIVATE struct wm_format *format = NULL;
  result = libwm_lazy_alloc_format(&format,
#if BYTE_ORDER == LITTLE_ENDIAN_ORDER
                                    0xff0000,16,
                                    0x00ff00,8,
                                    0x0000ff,0,
#else
                                    0x0000ff,0,
                                    0x00ff00,8,
                                    0xff0000,16,
#endif
                                    0,0,24);
 } break;

 case WM_FORMAT_RGB24: {
  PRIVATE struct wm_format *format = NULL;
  result = libwm_lazy_alloc_format(&format,
#if BYTE_ORDER == LITTLE_ENDIAN_ORDER
                                    0x0000ff,0,
                                    0x00ff00,8,
                                    0xff0000,16,
#else
                                    0xff0000,16,
                                    0x00ff00,8,
                                    0x0000ff,0,
#endif
                                    0,0,24);
 } break;

#if BYTE_ORDER == LITTLE_ENDIAN_ORDER
#define DEFINE_FORMAT32(name,rmask,rshift,gmask,gshift,bmask,bshift,amask,ashift) \
 case name: { \
  PRIVATE struct wm_format *format = NULL; \
  result = libwm_lazy_alloc_format(&format, \
                                    BSWAP32_C(rmask),rmask ? (32-rshift) : 0, \
                                    BSWAP32_C(gmask),gmask ? (32-gshift) : 0, \
                                    BSWAP32_C(bmask),bmask ? (32-bshift) : 0, \
                                    BSWAP32_C(amask),amask ? (32-ashift) : 0, \
                                    32); \
 } break
#else
#define DEFINE_FORMAT32(name,rmask,rshift,gmask,gshift,bmask,bshift,amask,ashift) \
 case name: { \
  PRIVATE struct wm_format *format = NULL; \
  result = libwm_lazy_alloc_format(&format, \
                                    rmask,rshift, \
                                    gmask,gshift, \
                                    bmask,bshift, \
                                    amask,ashift, \
                                    32); \
 } break
#endif
 DEFINE_FORMAT32(WM_FORMAT_XBGR32,
                 0x000000ff,0,
                 0x0000ff00,8,
                 0x00ff0000,16,
                 0x00000000,0);
 DEFINE_FORMAT32(WM_FORMAT_BGRX32,
                 0x0000ff00,8,
                 0x00ff0000,16,
                 0xff000000,24,
                 0x00000000,0);
 DEFINE_FORMAT32(WM_FORMAT_ABGR32,
                 0x000000ff,0,
                 0x0000ff00,8,
                 0x00ff0000,16,
                 0xff000000,24);
 DEFINE_FORMAT32(WM_FORMAT_BGRA32,
                 0x0000ff00,8,
                 0x00ff0000,16,
                 0xff000000,24,
                 0x000000ff,0);
 DEFINE_FORMAT32(WM_FORMAT_XRGB32,
                 0x00ff0000,16,
                 0x0000ff00,8,
                 0x000000ff,0,
                 0x00000000,0);
 DEFINE_FORMAT32(WM_FORMAT_RGBX32,
                 0xff000000,24,
                 0x00ff0000,16,
                 0x0000ff00,8,
                 0x00000000,0);
 DEFINE_FORMAT32(WM_FORMAT_RGBA32,
                 0xff000000,24,
                 0x00ff0000,16,
                 0x0000ff00,8,
                 0x000000ff,0);
 DEFINE_FORMAT32(WM_FORMAT_ARGB32,
                 0x00ff0000,16,
                 0x0000ff00,8,
                 0x000000ff,0,
                 0xff000000,24);
#undef DEFINE_FORMAT32

 default:
  error_throw(E_INVALID_ARGUMENT);
 }
 return result;
}


INTERN struct PACKED {
    WM_PALETTE_TYPE(256)
} libwm_palette_256_ ASMNAME("libwm_palette_256") = {
    .p_refcnt = 0x3fffffff,
    .p_bpp    = 8,
    .p_count  = 256,
    .p_colors = {
        /* TODO: These are VGA colors, meaning they're masked by `0x3f'. - Fix that! */
        {0x00,0x00,0x00},{0x00,0x00,0x2a},{0x00,0x2a,0x00},{0x00,0x2a,0x2a},{0x2a,0x00,0x00},{0x2a,0x00,0x2a},{0x2a,0x15,0x00},{0x2a,0x2a,0x2a},
        {0x15,0x15,0x15},{0x15,0x15,0x3f},{0x15,0x3f,0x15},{0x15,0x3f,0x3f},{0x3f,0x15,0x15},{0x3f,0x15,0x3f},{0x3f,0x3f,0x15},{0x3f,0x3f,0x3f},
        {0x00,0x00,0x00},{0x05,0x05,0x05},{0x08,0x08,0x08},{0x0b,0x0b,0x0b},{0x0e,0x0e,0x0e},{0x11,0x11,0x11},{0x14,0x14,0x14},{0x18,0x18,0x18},
        {0x1c,0x1c,0x1c},{0x20,0x20,0x20},{0x24,0x24,0x24},{0x28,0x28,0x28},{0x2d,0x2d,0x2d},{0x32,0x32,0x32},{0x38,0x38,0x38},{0x3f,0x3f,0x3f},
        {0x00,0x00,0x3f},{0x10,0x00,0x3f},{0x1f,0x00,0x3f},{0x2f,0x00,0x3f},{0x3f,0x00,0x3f},{0x3f,0x00,0x2f},{0x3f,0x00,0x1f},{0x3f,0x00,0x10},
        {0x3f,0x00,0x00},{0x3f,0x10,0x00},{0x3f,0x1f,0x00},{0x3f,0x2f,0x00},{0x3f,0x3f,0x00},{0x2f,0x3f,0x00},{0x1f,0x3f,0x00},{0x10,0x3f,0x00},
        {0x00,0x3f,0x00},{0x00,0x3f,0x10},{0x00,0x3f,0x1f},{0x00,0x3f,0x2f},{0x00,0x3f,0x3f},{0x00,0x2f,0x3f},{0x00,0x1f,0x3f},{0x00,0x10,0x3f},
        {0x1f,0x1f,0x3f},{0x27,0x1f,0x3f},{0x2f,0x1f,0x3f},{0x37,0x1f,0x3f},{0x3f,0x1f,0x3f},{0x3f,0x1f,0x37},{0x3f,0x1f,0x2f},{0x3f,0x1f,0x27},
        {0x3f,0x1f,0x1f},{0x3f,0x27,0x1f},{0x3f,0x2f,0x1f},{0x3f,0x37,0x1f},{0x3f,0x3f,0x1f},{0x37,0x3f,0x1f},{0x2f,0x3f,0x1f},{0x27,0x3f,0x1f},
        {0x1f,0x3f,0x1f},{0x1f,0x3f,0x27},{0x1f,0x3f,0x2f},{0x1f,0x3f,0x37},{0x1f,0x3f,0x3f},{0x1f,0x37,0x3f},{0x1f,0x2f,0x3f},{0x1f,0x27,0x3f},
        {0x2d,0x2d,0x3f},{0x31,0x2d,0x3f},{0x36,0x2d,0x3f},{0x3a,0x2d,0x3f},{0x3f,0x2d,0x3f},{0x3f,0x2d,0x3a},{0x3f,0x2d,0x36},{0x3f,0x2d,0x31},
        {0x3f,0x2d,0x2d},{0x3f,0x31,0x2d},{0x3f,0x36,0x2d},{0x3f,0x3a,0x2d},{0x3f,0x3f,0x2d},{0x3a,0x3f,0x2d},{0x36,0x3f,0x2d},{0x31,0x3f,0x2d},
        {0x2d,0x3f,0x2d},{0x2d,0x3f,0x31},{0x2d,0x3f,0x36},{0x2d,0x3f,0x3a},{0x2d,0x3f,0x3f},{0x2d,0x3a,0x3f},{0x2d,0x36,0x3f},{0x2d,0x31,0x3f},
        {0x00,0x00,0x1c},{0x07,0x00,0x1c},{0x0e,0x00,0x1c},{0x15,0x00,0x1c},{0x1c,0x00,0x1c},{0x1c,0x00,0x15},{0x1c,0x00,0x0e},{0x1c,0x00,0x07},
        {0x1c,0x00,0x00},{0x1c,0x07,0x00},{0x1c,0x0e,0x00},{0x1c,0x15,0x00},{0x1c,0x1c,0x00},{0x15,0x1c,0x00},{0x0e,0x1c,0x00},{0x07,0x1c,0x00},
        {0x00,0x1c,0x00},{0x00,0x1c,0x07},{0x00,0x1c,0x0e},{0x00,0x1c,0x15},{0x00,0x1c,0x1c},{0x00,0x15,0x1c},{0x00,0x0e,0x1c},{0x00,0x07,0x1c},
        {0x0e,0x0e,0x1c},{0x11,0x0e,0x1c},{0x15,0x0e,0x1c},{0x18,0x0e,0x1c},{0x1c,0x0e,0x1c},{0x1c,0x0e,0x18},{0x1c,0x0e,0x15},{0x1c,0x0e,0x11},
        {0x1c,0x0e,0x0e},{0x1c,0x11,0x0e},{0x1c,0x15,0x0e},{0x1c,0x18,0x0e},{0x1c,0x1c,0x0e},{0x18,0x1c,0x0e},{0x15,0x1c,0x0e},{0x11,0x1c,0x0e},
        {0x0e,0x1c,0x0e},{0x0e,0x1c,0x11},{0x0e,0x1c,0x15},{0x0e,0x1c,0x18},{0x0e,0x1c,0x1c},{0x0e,0x18,0x1c},{0x0e,0x15,0x1c},{0x0e,0x11,0x1c},
        {0x14,0x14,0x1c},{0x16,0x14,0x1c},{0x18,0x14,0x1c},{0x1a,0x14,0x1c},{0x1c,0x14,0x1c},{0x1c,0x14,0x1a},{0x1c,0x14,0x18},{0x1c,0x14,0x16},
        {0x1c,0x14,0x14},{0x1c,0x16,0x14},{0x1c,0x18,0x14},{0x1c,0x1a,0x14},{0x1c,0x1c,0x14},{0x1a,0x1c,0x14},{0x18,0x1c,0x14},{0x16,0x1c,0x14},
        {0x14,0x1c,0x14},{0x14,0x1c,0x16},{0x14,0x1c,0x18},{0x14,0x1c,0x1a},{0x14,0x1c,0x1c},{0x14,0x1a,0x1c},{0x14,0x18,0x1c},{0x14,0x16,0x1c},
        {0x00,0x00,0x10},{0x04,0x00,0x10},{0x08,0x00,0x10},{0x0c,0x00,0x10},{0x10,0x00,0x10},{0x10,0x00,0x0c},{0x10,0x00,0x08},{0x10,0x00,0x04},
        {0x10,0x00,0x00},{0x10,0x04,0x00},{0x10,0x08,0x00},{0x10,0x0c,0x00},{0x10,0x10,0x00},{0x0c,0x10,0x00},{0x08,0x10,0x00},{0x04,0x10,0x00},
        {0x00,0x10,0x00},{0x00,0x10,0x04},{0x00,0x10,0x08},{0x00,0x10,0x0c},{0x00,0x10,0x10},{0x00,0x0c,0x10},{0x00,0x08,0x10},{0x00,0x04,0x10},
        {0x08,0x08,0x10},{0x0a,0x08,0x10},{0x0c,0x08,0x10},{0x0e,0x08,0x10},{0x10,0x08,0x10},{0x10,0x08,0x0e},{0x10,0x08,0x0c},{0x10,0x08,0x0a},
        {0x10,0x08,0x08},{0x10,0x0a,0x08},{0x10,0x0c,0x08},{0x10,0x0e,0x08},{0x10,0x10,0x08},{0x0e,0x10,0x08},{0x0c,0x10,0x08},{0x0a,0x10,0x08},
        {0x08,0x10,0x08},{0x08,0x10,0x0a},{0x08,0x10,0x0c},{0x08,0x10,0x0e},{0x08,0x10,0x10},{0x08,0x0e,0x10},{0x08,0x0c,0x10},{0x08,0x0a,0x10},
        {0x0b,0x0b,0x10},{0x0c,0x0b,0x10},{0x0d,0x0b,0x10},{0x0f,0x0b,0x10},{0x10,0x0b,0x10},{0x10,0x0b,0x0f},{0x10,0x0b,0x0d},{0x10,0x0b,0x0c},
        {0x10,0x0b,0x0b},{0x10,0x0c,0x0b},{0x10,0x0d,0x0b},{0x10,0x0f,0x0b},{0x10,0x10,0x0b},{0x0f,0x10,0x0b},{0x0d,0x10,0x0b},{0x0c,0x10,0x0b},
        {0x0b,0x10,0x0b},{0x0b,0x10,0x0c},{0x0b,0x10,0x0d},{0x0b,0x10,0x0f},{0x0b,0x10,0x10},{0x0b,0x0f,0x10},{0x0b,0x0d,0x10},{0x0b,0x0c,0x10},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00}
    }
};



#ifndef __INTELLISENSE__
#define BPP        1
#define EMPTY_SURFACE 1
#define FUNC(name) empty_surface_##name
#include "surface-ops.c.inl"

#define BPP        1
#define FUNC(name) surface1_##name
#include "surface-ops.c.inl"
#define BPP        2
#define FUNC(name) surface2_##name
#include "surface-ops.c.inl"
#define BPP        4
#define FUNC(name) surface4_##name
#include "surface-ops.c.inl"
#define BPP        8
#define FUNC(name) surface8_##name
#include "surface-ops.c.inl"
#define BPP        16
#define FUNC(name) surface16_##name
#include "surface-ops.c.inl"
#define BPP        24
#define FUNC(name) surface24_##name
#include "surface-ops.c.inl"
#define BPP        32
#define FUNC(name) surface32_##name
#include "surface-ops.c.inl"

#define FOR_SURFACE_VIEW 1
#define BPP        1
#define FUNC(name) surface_view1_##name
#include "surface-ops.c.inl"
#define BPP        2
#define FUNC(name) surface_view2_##name
#include "surface-ops.c.inl"
#define BPP        4
#define FUNC(name) surface_view4_##name
#include "surface-ops.c.inl"
#define BPP        8
#define FUNC(name) surface_view8_##name
#include "surface-ops.c.inl"
#define BPP        16
#define FUNC(name) surface_view16_##name
#include "surface-ops.c.inl"
#define BPP        24
#define FUNC(name) surface_view24_##name
#include "surface-ops.c.inl"
#define BPP        32
#define FUNC(name) surface_view32_##name
#include "surface-ops.c.inl"
#undef FOR_SURFACE_VIEW

INTERN ATTR_NOTHROW void WMCALL
libwm_setup_surface_ops(struct wm_surface *__restrict self) {
 struct wm_surface_ops const *ops;
 assert(self->s_format->f_bpp >= 1 &&
        self->s_format->f_bpp <= 32);
 assert((self->s_format->f_bpp & (self->s_format->f_bpp-1)) == 0 ||
         self->s_format->f_bpp == 24);
 switch (self->s_format->f_bpp) {
 case 1:  ops = &surface1_ops;  break;
 case 2:  ops = &surface2_ops;  break;
 case 4:  ops = &surface4_ops;  break;
 case 8:  ops = &surface8_ops;  break;
 case 16: ops = &surface16_ops; break;
 case 24: ops = &surface24_ops; break;
 case 32: ops = &surface32_ops; break;
 default: __builtin_unreachable();
 }
 self->s_ops = ops;
}

INTERN ATTR_NOTHROW void WMCALL
libwm_setup_surface_view_ops(struct wm_surface_view *__restrict self) {
 if (!self->s_sizex || !self->s_sizey) {
  /* Surface views are allowed to be empty. */
  self->s_ops = &empty_surface_ops;
 } else if (!self->s_offx && !self->s_offy) {
  /* No sub-pixel offset (meaning we can use the regular ops) */
  libwm_setup_surface_ops((struct wm_surface *)self);
 } else {
  struct wm_surface_ops const *ops;
  assert(self->s_format->f_bpp >= 1 && self->s_format->f_bpp < 8);
  assert((self->s_format->f_bpp & (self->s_format->f_bpp-1)) == 0 ||
          self->s_format->f_bpp == 24);
  self->s_flags |= WM_SURFACE_FISVIEW;
  switch (self->s_format->f_bpp) {
  case 1:  ops = &surface_view1_ops;  break;
  case 2:  ops = &surface_view2_ops;  break;
  case 4:  ops = &surface_view4_ops;  break;
  case 8:  ops = &surface_view8_ops;  break;
  case 16: ops = &surface_view16_ops; break;
  case 24: ops = &surface_view24_ops; break;
  case 32: ops = &surface_view32_ops; break;
  default: __builtin_unreachable();
  }
  self->s_ops = ops;
 }
}

#endif


DECL_END

#endif /* !GUARD_LIBS_LIBWM_SURFACE_C */
