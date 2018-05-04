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
#ifndef _WM_SURFACE_H
#define _WM_SURFACE_H 1

#include "api.h"
#include <hybrid/atomic.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <features.h>
#include <bits/types.h>

__SYSDECL_BEGIN

typedef __uint32_t wm_pixel_t;
struct wm_surface;
struct wm_surface_ops;

struct PACKED wm_color_triple {
    __uint8_t   ct_red;   /* [<= :p_mask] Red color component. */
    __uint8_t   ct_green; /* [<= :p_mask] Green color component. */
    __uint8_t   ct_blue;  /* [<= :p_mask] Blue color component. */
};

struct PACKED wm_palette {
    ATOMIC_DATA __ref_t          p_refcnt;    /* Palette reference counter. */
    __uint8_t                    p_bpp;       /* [>= 1 && <= 16] Bits per pixel (power-of-2). */
    __uint8_t                    p_mask;      /* Per-color channel mask of effective color bits.
                                               * Bits not masked by this must be set to ZERO. */
    __uint16_t                   p_count;     /* [2...2^16][== 1 << p_bpp] Number of colors in the palette. */
    struct wm_color_triple       p_colors[1]; /* [p_count] Colors associated with individual palette indices. */
};

/* Create a new palette. */
WMAPI ATTR_RETNONNULL REF struct wm_palette *
WMCALL wm_palette_create(__uint16_t bpp, __uint8_t mask);

/* Increment/decrement the reference counter of a WM palette. */
#define wm_palette_incref(self) (void)ATOMIC_FETCHINC((self)->p_refcnt)
#define wm_palette_decref(self) (void)(ATOMIC_DECFETCH((self)->p_refcnt) || (wm_palette_destroy(self),0))
WMAPI void WMCALL wm_palette_destroy(struct wm_palette *__restrict self);



/* The closest representations which can be provided for the following colors.
 * Note that in environments with less colors (such as black+white, or gray-scale),
 * approximations are taken to try and get everything to look as close as possible. */
#define WM_COLOR_BLACK   0x0 /* Black    #000000 rgb(0,0,0) */
#define WM_COLOR_MAROON  0x1 /* Maroon   #800000 rgb(128,0,0) */
#define WM_COLOR_GREEN   0x2 /* Green    #008000 rgb(0,128,0) */
#define WM_COLOR_OLIVE   0x3 /* Olive    #808000 rgb(128,128,0) */
#define WM_COLOR_NAVY    0x4 /* Navy     #000080 rgb(0,0,128) */
#define WM_COLOR_PURPLE  0x5 /* Purple   #800080 rgb(128,0,128) */
#define WM_COLOR_TEAL    0x6 /* Teal     #008080 rgb(0,128,128) */
#define WM_COLOR_SILVER  0x7 /* Silver   #c0c0c0 rgb(192,192,192) */
#define WM_COLOR_GREY    0x8 /* Grey     #808080 rgb(128,128,128) */
#define WM_COLOR_RED     0x9 /* Red      #ff0000 rgb(255,0,0) */
#define WM_COLOR_LIME    0xa /* Lime     #00ff00 rgb(0,255,0) */
#define WM_COLOR_YELLOW  0xb /* Yellow   #ffff00 rgb(255,255,0) */
#define WM_COLOR_BLUE    0xc /* Blue     #0000ff rgb(0,0,255) */
#define WM_COLOR_FUCHSIA 0xd /* Fuchsia  #ff00ff rgb(255,0,255) */
#define WM_COLOR_AQUA    0xe /* Aqua     #00ffff rgb(0,255,255) */
#define WM_COLOR_WHITE   0xf /* White    #ffffff rgb(255,255,255) */
#define WM_COLOR_COUNT   16

struct wm_format {
    ATOMIC_DATA __ref_t          f_refcnt; /* Format reference counter. */
    REF struct wm_palette       *f_pal;    /* [0..1][const] Palette (if used) */
    struct wm_surface_ops const *f_ops;    /* [1..1][const] Surface operators. */
    wm_pixel_t                   f_rmask;  /* [valid_if(!f_pal)] Mask for red color bits. */
    wm_pixel_t                   f_gmask;  /* [valid_if(!f_pal)] Mask for green color bits. */
    wm_pixel_t                   f_bmask;  /* [valid_if(!f_pal)] Mask for blue color bits. */
    wm_pixel_t                   f_amask;  /* [valid_if(!f_pal)] Mask for blue color bits. */
    __uint16_t                   f_rshft;  /* [valid_if(!f_pal)] Shift to reach the red color mask. */
    __uint16_t                   f_gshft;  /* [valid_if(!f_pal)] Shift to reach the green color mask. */
    __uint16_t                   f_bshft;  /* [valid_if(!f_pal)] Shift to reach the blue color mask. */
    __uint16_t                   f_ashft;  /* [valid_if(!f_pal)] Shift to reach the blue color mask. */
    wm_pixel_t                   f_color[WM_COLOR_COUNT]; /* Pixel codes for some standard colors. */
    __uint16_t                   f_bpp;    /* [>= 1 && <= 32] Bits per pixel (power-of-2). */
};


/* Create a new mask-based pixel format. */
WMAPI ATTR_RETNONNULL REF struct wm_format *WMCALL
wm_format_create(wm_pixel_t rmask, __uint16_t rshft,
                 wm_pixel_t gmask, __uint16_t gshft,
                 wm_pixel_t bmask, __uint16_t bshft,
                 wm_pixel_t amask, __uint16_t ashft);

/* Create a new palette-based pixel format. */
WMAPI ATTR_RETNONNULL REF struct wm_format *WMCALL
wm_format_create_pal(struct wm_palette *__restrict pal);

/* Increment/decrement the reference counter of a WM format. */
#define wm_format_incref(self) (void)ATOMIC_FETCHINC((self)->f_refcnt)
#define wm_format_decref(self) (void)(ATOMIC_DECFETCH((self)->f_refcnt) || (wm_format_destroy(self),0))
WMAPI void WMCALL wm_format_destroy(struct wm_format *__restrict self);



struct wm_surface {
    ATOMIC_DATA __ref_t          s_refcnt; /* Surface reference counter. */
    struct wm_surface_ops const *s_ops;    /* [1..1][const] Surface operators. */
    REF struct wm_format        *s_format; /* [1..1][const] The surface format. */
#define WM_SURFACE_FNORMAL       0x0000    /* Normal surface flags. */
#define WM_SURFACE_FWINDOW       0x0001    /* This surface is actually a `struct wm_window'. */
    __UINTPTR_HALF_TYPE__        s_flags;  /* [const] Surface flags (Set of `WM_SURFACE_F*'). */
    __UINTPTR_HALF_TYPE__        s_bpp;    /* [const][== s_format->f_bpp] Bits per pixel */
    atomic_rwlock_t              s_lock;   /* Lock that must be held (in read-mode only)
                                            * when reading any of the surface's fields.
                                            * The only place where this lock must be acquired
                                            * for writing is when the surface should be resized.
                                            * Also note that all the surface operators found below
                                            * will automatically acquire this lock for themself.
                                            * However since all of them (except for `wm_surface_resize()')
                                            * will only acquire read-locks, it's actually OK if the caller
                                            * acquires their own read-lock recursively. */
    unsigned int                 s_sizex;  /* [!0][lock(s_lock)] Surface width (in pixels). */
    unsigned int                 s_sizey;  /* [!0][lock(s_lock)] Surface height (in pixels). */
    unsigned int                 s_stride; /* [lock(s_lock)] Length of a surface buffer line (in bytes) */
    __byte_t                    *s_buffer; /* [1..1][owned][lock(s_lock)] Surface buffer. */
};


/* Lock/unlock the given WM surface.
 * Locking is only required to prevent the surface's size from changing.
 * This lock will not prevent other operations, even if the operation
 * for which this lock was acquired is a write operation.
 * Surface contents must be synchronized explicitly, or you could simply
 * try not to draw 2 things in the same position... */
#define wm_surface_trylock(self) atomic_rwlock_tryread(&(self)->s_lock)
#define wm_surface_lock(self)    atomic_rwlock_read(&(self)->s_lock)
#define wm_surface_unlock(self)  atomic_rwlock_endread(&(self)->s_lock)


/* Create a new surface with the given size and format. */
WMAPI ATTR_RETNONNULL REF struct wm_surface *WMCALL
wm_surface_create(struct wm_format *__restrict format,
                  unsigned int sizex, unsigned int sizey);
WMAPI void WMCALL wm_surface_destroy(struct wm_surface *__restrict self);

/* Increment/decrement the reference counter of a WM surface. */
#define wm_surface_incref(self) (void)ATOMIC_FETCHINC((self)->s_refcnt)
#define wm_surface_decref(self) (void)(ATOMIC_DECFETCH((self)->s_refcnt) || (wm_surface_destroy(self),0))


/* Convert the given surface into a new surface of `new_format'.
 * If the format of `self' already matches `new_format', simply
 * re-return a new reference to `self'.
 * If the format of `self' doesn't match `new_format', this
 * function behaves equivalent to the following:
 * >> REF struct wm_surface *result;
 * >> wm_surface_lock(self);
 * >> result = wm_surface_create(new_format,
 * >>                            self->s_sizex,
 * >>                            self->s_sizey);
 * >> wm_surface_bblit(result,0,0,self,0,0,self->s_sizex,self->s_sizey);
 * >> wm_surface_unlock(self);
 * >> return result;
 */
WMAPI ATTR_RETNONNULL REF struct wm_surface *WMCALL
wm_surface_convert(struct wm_surface *__restrict self,
                   struct wm_format *__restrict new_format);


/* Resize a given WM surface or window. */
WMAPI void WMCALL
wm_surface_resize(struct wm_surface *__restrict self,
                  unsigned int new_sizx, unsigned int new_sizy);



struct wm_surface_ops {
    /* Get a single pixel. */
#define wm_surface_getpixel(self,x,y) \
     (*(self)->s_ops->so_getpixel)(self,x,y)
    wm_pixel_t (WMCALL *so_getpixel)(struct wm_surface const *__restrict self, int x, int y);

    /* Set a single pixel. */
#define wm_surface_setpixel(self,x,y,pixel) \
     (*(self)->s_ops->so_setpixel)(self,x,y,pixel)
    void       (WMCALL *so_setpixel)(struct wm_surface *__restrict self, int x, int y, wm_pixel_t pixel);

    /* Draw a horizontal line. */
#define wm_surface_hline(self,x,y,size_x,pixel) \
     (*(self)->s_ops->so_hline)(self,x,y,size_x,pixel)
    void       (WMCALL *so_hline)(struct wm_surface *__restrict self, int x, int y, unsigned int size_x, wm_pixel_t pixel);

    /* Draw a vertical line. */
#define wm_surface_vline(self,x,y,size_y,pixel) \
     (*(self)->s_ops->so_vline)(self,x,y,size_y,pixel)
    void       (WMCALL *so_vline)(struct wm_surface *__restrict self, int x, int y, unsigned int size_y, wm_pixel_t pixel);

    /* Draw a rectangle on the inner borders of a given area. */
#define wm_surface_rect(self,x,y,size_x,size_y,pixel) \
     (*(self)->s_ops->so_rect)(self,x,y,size_x,size_y,pixel)
    void       (WMCALL *so_rect)(struct wm_surface *__restrict self, int x, int y, unsigned int size_x, unsigned int size_y, wm_pixel_t pixel);

    /* Fill (memset) a surface area with a solid color. */
#define wm_surface_fill(self,x,y,size_x,size_y,pixel) \
     (*(self)->s_ops->so_fill)(self,x,y,size_x,size_y,pixel)
    void       (WMCALL *so_fill)(struct wm_surface *__restrict self, int x, int y, unsigned int size_x, unsigned int size_y, wm_pixel_t pixel);

    /* Perform a basic blit operation that copies pixel from `source' and adjusts bits-per-pixel. */
#define wm_surface_bblit(self,x,y,source,source_x,source_y,size_x,size_y) \
     (*(self)->s_ops->so_bblit)(self,x,y,source,source_x,source_y,size_x,size_y)
    void       (WMCALL *so_bblit)(struct wm_surface *__restrict self, int x, int y,
                                  struct wm_surface const *__restrict source,
                                  unsigned int source_x, unsigned int source_y,
                                  unsigned int size_x, unsigned int size_y);

    /* Same as `so_bblit', but leave pixels in `self' unchanged if their value
     * in `source' equals `color_key', essentially implementing a cheap and fast
     * way of 1-bit alpha blending. */
#define wm_surface_cblit(self,x,y,source,source_x,source_y,size_x,size_y,color_key) \
     (*(self)->s_ops->so_cblit)(self,x,y,source,source_x,source_y,size_x,size_y,color_key)
    void       (WMCALL *so_cblit)(struct wm_surface *__restrict self, int x, int y,
                                  struct wm_surface const *__restrict source,
                                  unsigned int source_x, unsigned int source_y,
                                  unsigned int size_x, unsigned int size_y,
                                  wm_pixel_t color_key);
};






__SYSDECL_END

#endif /* !_WM_SURFACE_H */
