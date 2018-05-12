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
#include <hybrid/limitcore.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <stdbool.h>
#include <features.h>
#include <bits/types.h>

__SYSDECL_BEGIN


/* Restrict valid surface sizes to 1...INT_MAX, thus
 * allowing code to assume that dividing, or modulating
 * using a non-view surface's sizeX or sizeY is valid,
 * as well as doing the following:
 * >> if ((unsigned int)x >= surface->s_sizex ||
 * >>     (unsigned int)y >= surface->s_sizey)
 * >>      return;
 * Which will check if a given coord is located within
 * the buffer of a given surface.
 * Since the unsigned variants of negative integers can
 * be found in the far off positive, assuming that the
 * surface's size will not exceed the maximum positive
 * value, we can always just check using unsigned compares,
 * allowing us to omit the checks for `x < 0 || ...' */
#define WM_MAX_SURFACE_SIZE  __INT_MAX__


typedef __uint32_t wm_pixel_t;
struct wm_surface;
struct wm_surface_ops;

#ifdef __cplusplus
#define __WM_SURFACE_POINTER          struct wm_surface *
#define __WM_SURFACE_POINTER_R        struct wm_surface *__restrict
#define __WM_SURFACE_CONST_POINTER    struct wm_surface const *
#define __WM_SURFACE_CONST_POINTER_R  struct wm_surface const *__restrict
#elif !defined(__NO_ATTR_TRANSPARENT_UNION)
#define __WM_FOREACH_SURFACE_CLASS(func) \
    func(wm_surface) \
    func(wm_surface_view) \
    func(wm_window) \
    func(wm_font) \
/**/


#define __WM_DECL_STRUCT(T) struct T;
__WM_FOREACH_SURFACE_CLASS(__WM_DECL_STRUCT)
#undef __WM_DECL_STRUCT

typedef union {
#define __WM_ENUM_STRUCT(T) struct T *__##T##__;
__WM_FOREACH_SURFACE_CLASS(__WM_ENUM_STRUCT)
#undef __WM_ENUM_STRUCT
} __wm_surface_pointer_t __ATTR_TRANSPARENT_UNION;
typedef union {
#define __WM_ENUM_STRUCT(T) struct T const *__##T##__;
__WM_FOREACH_SURFACE_CLASS(__WM_ENUM_STRUCT)
#undef __WM_ENUM_STRUCT
} __wm_surface_const_pointer_t __ATTR_TRANSPARENT_UNION;
typedef union {
#define __WM_ENUM_STRUCT(T) struct T *__restrict __##T##__;
__WM_FOREACH_SURFACE_CLASS(__WM_ENUM_STRUCT)
#undef __WM_ENUM_STRUCT
} __wm_surface_pointer_r_t __ATTR_TRANSPARENT_UNION;
typedef union {
#define __WM_ENUM_STRUCT(T) struct T const *__restrict __##T##__;
__WM_FOREACH_SURFACE_CLASS(__WM_ENUM_STRUCT)
#undef __WM_ENUM_STRUCT
} __wm_surface_const_pointer_r_t __ATTR_TRANSPARENT_UNION;
#undef __WM_FOREACH_SURFACE_CLASS

#define __WM_SURFACE_POINTER          __wm_surface_pointer_t
#define __WM_SURFACE_CONST_POINTER    __wm_surface_const_pointer_t
#define __WM_SURFACE_POINTER_R        __wm_surface_pointer_r_t
#define __WM_SURFACE_CONST_POINTER_R  __wm_surface_const_pointer_r_t
#else
#define __WM_SURFACE_POINTER          void *
#define __WM_SURFACE_CONST_POINTER    void const *
#define __WM_SURFACE_POINTER_R        void *__restrict
#define __WM_SURFACE_CONST_POINTER_R  void const *__restrict
#endif

struct PACKED wm_color_triple {
    __uint8_t   c_red;    /* Red color component. */
    __uint8_t   c_green;  /* Green color component. */
    __uint8_t   c_blue;   /* Blue color component. */
};

struct PACKED wm_color {
    union PACKED {
        struct PACKED {
            __uint8_t   c_red;    /* Red color component. */
            __uint8_t   c_green;  /* Green color component. */
            __uint8_t   c_blue;   /* Blue color component. */
            __uint8_t   c_alpha;  /* Transparent color component. */
        };
        __uint32_t      c_code;   /* The color code. */
        struct wm_color_triple
                        c_triple; /* RGB color triple */
    };
};

#define WM_COLOR_INIT(r,g,b,a) {{{ r, g, b, a }}}


/* Compare the 2 given colors for being similar, and return their
 * similarity score (lower values mean more similar; ZERO means identical)
 * The comparison tries to be as close to what the human eye would
 * consider similar colors, meaning that this does more than just
 * returning the sum of differences between color channels. */
WMAPI __uint32_t WMCALL wm_color_compare(struct wm_color_triple a,
                                         struct wm_color_triple b);


#define WM_PALETTE_TYPE(num_colors) \
    ATOMIC_DATA __ref_t          p_refcnt;             /* Palette reference counter. */ \
    __uint8_t                    p_bpp;                /* [>= 1 && <= 16] Bits per pixel (power-of-2). */ \
    __uint16_t                   p_count;              /* [2...2^16][== 1 << p_bpp] Number of colors in the palette. */ \
    struct wm_color_triple       p_colors[num_colors]; /* [p_count] Colors associated with individual palette indices. */ \
/**/

struct PACKED wm_palette {
    WM_PALETTE_TYPE(1)
};



/* Create a new palette.
 * @throw: E_BADALLOC: [...] */
WMAPI ATTR_RETNONNULL REF struct wm_palette *
WMCALL wm_palette_create(unsigned int bpp);

/* Increment/decrement the reference counter of a WM palette. */
#define wm_palette_incref(self) (void)ATOMIC_FETCHINC((self)->p_refcnt)
#define wm_palette_decref(self) (void)(ATOMIC_DECFETCH((self)->p_refcnt) || (wm_palette_destroy(self),0))
WMAPI ATTR_NOTHROW void WMCALL wm_palette_destroy(struct wm_palette *__restrict self);



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
    wm_pixel_t                   f_rmask;  /* [valid_if(!f_pal)][const] Mask for red color bits. */
    wm_pixel_t                   f_gmask;  /* [valid_if(!f_pal)][const] Mask for green color bits. */
    wm_pixel_t                   f_bmask;  /* [valid_if(!f_pal)][const] Mask for blue color bits. */
    wm_pixel_t                   f_amask;  /* [valid_if(!f_pal)][const] Mask for blue color bits. */
    __uint16_t                   f_rshft;  /* [valid_if(!f_pal)][const] Shift to reach the red color mask. */
    __uint16_t                   f_gshft;  /* [valid_if(!f_pal)][const] Shift to reach the green color mask. */
    __uint16_t                   f_bshft;  /* [valid_if(!f_pal)][const] Shift to reach the blue color mask. */
    __uint16_t                   f_ashft;  /* [valid_if(!f_pal)][const] Shift to reach the blue color mask. */
    wm_pixel_t                   f_color[16]; /* Pixel codes for some standard colors. */
    __uint16_t                   f_bpp;    /* [>= 1 && <= 32][const] Bits per pixel (power-of-2, or 24). */
    __uint16_t                 __f_pad[(sizeof(void *)-2)/2];
    /*ATTR_NOTHROW*/ struct wm_color (WMCALL *f_colorof)(struct wm_format const *__restrict self, wm_pixel_t pixel);
    /*ATTR_NOTHROW*/ wm_pixel_t (WMCALL *f_pixelof)(struct wm_format const *__restrict self, struct wm_color color);
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
WMAPI ATTR_RETNONNULL struct wm_format *WMCALL wm_format_lookup(unsigned int name);
#define WM_FORMAT_BLACK_AND_WHITE  0x0000 /* 1-bpp, palette(white=0, black=1) */
#define WM_FORMAT_GRAYSCALE4       0x0001 /* 2-bpp, palette(white=0, light-gray=1, dark-gray=1, black=3) */
#define WM_FORMAT_GRAYSCALE16      0x0002 /* 4-bpp, palette(white=0, ... black=15) */
#define WM_FORMAT_BGR24            0x0100 /* 24-bpp, [B, G, R] */
#define WM_FORMAT_RGB24            0x0101 /* 24-bpp, [R, G, B] */
#define WM_FORMAT_XBGR32           0x0200 /* 32-bpp, [-, B, G, R] */
#define WM_FORMAT_BGRX32           0x0201 /* 32-bpp, [B, G, R, -] */
#define WM_FORMAT_ABGR32           0x0202 /* 32-bpp, [A, B, G, R] */
#define WM_FORMAT_BGRA32           0x0203 /* 32-bpp, [B, G, R, A] */
#define WM_FORMAT_XRGB32           0x0204 /* 32-bpp, [-, R, G, B] */
#define WM_FORMAT_RGBX32           0x0205 /* 32-bpp, [R, G, B, -] */
#define WM_FORMAT_RGBA32           0x0206 /* 32-bpp, [R, G, B, A] */
#define WM_FORMAT_ARGB32           0x0207 /* 32-bpp, [A, R, G, B] */


/* Create a new mask-based pixel format.
 * @throw: E_BADALLOC: [...] */
WMAPI ATTR_RETNONNULL REF struct wm_format *WMCALL
wm_format_create(wm_pixel_t rmask, __uint16_t rshft,
                 wm_pixel_t gmask, __uint16_t gshft,
                 wm_pixel_t bmask, __uint16_t bshft,
                 wm_pixel_t amask, __uint16_t ashft,
                 unsigned int bpp);

/* Create a new palette-based pixel format.
 * @throw: E_BADALLOC: [...] */
WMAPI ATTR_RETNONNULL REF struct wm_format *WMCALL
wm_format_create_pal(struct wm_palette *__restrict pal);

/* Increment/decrement the reference counter of a WM format. */
#define wm_format_incref(self) (void)ATOMIC_FETCHINC((self)->f_refcnt)
#define wm_format_decref(self) (void)(ATOMIC_DECFETCH((self)->f_refcnt) || (wm_format_destroy(self),0))
WMAPI ATTR_NOTHROW void WMCALL wm_format_destroy(struct wm_format *__restrict self);

/* Return the color of a given pixel. */
FORCELOCAL ATTR_NOTHROW struct wm_color WMCALL
wm_format_colorof(struct wm_format const *__restrict self,
                  wm_pixel_t pixel) {
 return (*self->f_colorof)(self,pixel);
}

/* Return the (closest) pixel of a given color. */
FORCELOCAL ATTR_NOTHROW wm_pixel_t WMCALL
wm_format_pixelof(struct wm_format const *__restrict self,
                  struct wm_color color) {
 return (*self->f_pixelof)(self,color);
}


#define WM_SURFACE_FNORMAL       0x0000    /* Normal surface flags. */
#define WM_SURFACE_FWINDOW       0x0001    /* This surface is actually a `struct wm_window'. */
#define WM_SURFACE_FISFONT       0x0002    /* This surface is actually a `struct wm_font' */
#define WM_SURFACE_FISVIEW       0x8000    /* This surface is actually a `struct wm_surface_view'
                                            * with non-zero sub-pixel offsets. */

#define __WM_SURFACE_STRUCT_MEMBERS \
    ATOMIC_DATA __ref_t          s_refcnt; /* Surface reference counter. */ \
    struct wm_surface_ops const *s_ops;    /* [1..1][lock(s_lock)] Surface operators. */ \
    REF struct wm_format        *s_format; /* [1..1][lock(s_lock)] The surface format. */ \
    __UINTPTR_HALF_TYPE__        s_flags;  /* [const] Surface flags (Set of `WM_SURFACE_F*'). */ \
    __UINTPTR_HALF_TYPE__      __s_pad;    /* ... */ \
    atomic_rwlock_t              s_lock;   /* Lock that must be held (in read-mode only) \
                                            * when reading any of the surface's fields. \
                                            * The only place where this lock must be acquired \
                                            * for writing is when the surface should be resized. \
                                            * Also note that all the surface operators found below \
                                            * will automatically acquire this lock for themself. \
                                            * However since all of them (except for `wm_surface_resize()') \
                                            * will only acquire read-locks, it's actually OK if the caller \
                                            * acquires their own read-lock recursively. */ \
    unsigned int                 s_sizex;  /* [!0][lock(s_lock)] Surface width (in pixels). */ \
    unsigned int                 s_sizey;  /* [!0][lock(s_lock)] Surface height (in pixels). */ \
    unsigned int                 s_stride; /* [lock(s_lock)] Length of a surface buffer line (in bytes) */ \
    __byte_t                    *s_buffer; /* [1..1][owned][lock(s_lock)] Surface buffer. */ \
/**/

struct PACKED wm_surface {
    __WM_SURFACE_STRUCT_MEMBERS
};
#ifdef __cplusplus
#undef __WM_SURFACE_STRUCT_MEMBERS
#define __WM_SURFACE_STRUCT_MEMBERS /* nothing */
#endif


/* Lock/unlock the given WM surface.
 * Locking is only required to prevent the surface's size from changing.
 * This lock will not prevent other operations, even if the operation
 * for which this lock was acquired is a write operation.
 * Surface contents must be synchronized explicitly, or you could simply
 * try not to draw 2 things in the same position... */
#define wm_surface_trylock(self) atomic_rwlock_tryread(&(self)->s_lock)
#define wm_surface_lock(self)    atomic_rwlock_read(&(self)->s_lock)
#define wm_surface_unlock(self)  atomic_rwlock_endread(&(self)->s_lock)


/* Create a new surface with the given size and format.
 * @throw: E_INVALID_ARGUMENT: Either `sizex' or `sizey' is ZERO(0), or one
 *                             of them is larger than `WM_MAX_SURFACE_SIZE'
 * @throw: E_BADALLOC:         [...] */
WMAPI ATTR_RETNONNULL REF struct wm_surface *WMCALL
wm_surface_create(struct wm_format *__restrict format,
                  unsigned int sizex, unsigned int sizey);

/* Increment/decrement the reference counter of a WM surface. */
#define wm_surface_incref(self) (void)ATOMIC_FETCHINC((self)->s_refcnt)
#define wm_surface_decref(self) (void)(ATOMIC_DECFETCH((self)->s_refcnt) || (wm_surface_destroy(self),0))
WMAPI ATTR_NOTHROW void WMCALL wm_surface_destroy(__WM_SURFACE_POINTER_R self);



#ifdef __cplusplus
struct PACKED wm_surface_view: wm_surface
#else
struct PACKED wm_surface_view
#endif
{
    __WM_SURFACE_STRUCT_MEMBERS /* [OVERRIDE(.s_refcnt,[INVALID])]
                                 * [OVERRIDE(.s_format,[!REF])]
                                 * [OVERRIDE(.s_sizex,[![!0]][const])] // Is allowed to be ZERO, but is const
                                 * [OVERRIDE(.s_sizey,[![!0]][const])] // Is allowed to be ZERO, but is const
                                 * [OVERRIDE(.s_buffer,[![owned]])] */
    int s_offx; /* [const][<= 7] An offset that is added to every X coordinate internally. */
    int s_offy; /* [const][<= 7] An offset that is added to every Y coordinate internally. */
};


/* Create a sub-view of `surface' that is clamped to the
 * surface sub-area described by the specified coord rect.
 * The surface view is owned by the caller and must not be
 * incref() / decref()-ed to be destroyed, but will instead
 * weakly reference the data of the given `surface'.
 * The caller is responsible not to resize `surface' when
 * they still intent to keep on using any view created for
 * that surface:
 * >> struct wm_surface *my_surface = ...;
 * >> struct wm_surface_view my_view;
 * >> wm_surface_makeview(&my_view,my_surface,
 * >>                      10,20,7,8);
 * >> // Same as setting pixel `10,20' of `my_surface'
 * >> wm_surface_setpixel(&my_view,0,0,
 * >>                      my_view.s_format->f_color[WM_COLOR_BLACK]);
 * @return: * : [== view] Always re-return `view'
 */
WMAPI ATTR_NOTHROW ATTR_RETNONNULL struct wm_surface_view *WMCALL
wm_surface_makeview(struct wm_surface_view *__restrict view,
                    __WM_SURFACE_CONST_POINTER_R surface,
                    int posx, int posy, unsigned int sizex,
                    unsigned int sizey);


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
wm_surface_convert(__WM_SURFACE_POINTER_R self,
                   struct wm_format *__restrict new_format);


/* Resize a given WM surface or window. */
WMAPI void WMCALL
wm_surface_resize(__WM_SURFACE_POINTER_R self,
                  unsigned int new_sizx, unsigned int new_sizy);



struct wm_surface_ops {
    /* All fields in here are [1..1] */

    /* Get a single pixel. */
#define wm_surface_getpixel(self,x,y) \
     (*(self)->s_ops->so_getpixel)(self,x,y)
    wm_pixel_t (WMCALL *so_getpixel)(__WM_SURFACE_CONST_POINTER_R self, int x, int y);

    /* Set a single pixel.
     * @return: true:  The pixel was set.
     * @return: false: The given coords are out-of-bounds and the pixel wasn't set. */
#define wm_surface_setpixel(self,x,y,pixel) \
     (*(self)->s_ops->so_setpixel)(self,x,y,pixel)
    bool       (WMCALL *so_setpixel)(__WM_SURFACE_POINTER_R self, int x, int y, wm_pixel_t pixel);

    /* Draw a horizontal line. */
#define wm_surface_hline(self,x,y,size_x,pixel) \
     (*(self)->s_ops->so_hline)(self,x,y,size_x,pixel)
    void       (WMCALL *so_hline)(__WM_SURFACE_POINTER_R self, int x, int y, unsigned int size_x, wm_pixel_t pixel);

    /* Draw a vertical line. */
#define wm_surface_vline(self,x,y,size_y,pixel) \
     (*(self)->s_ops->so_vline)(self,x,y,size_y,pixel)
    void       (WMCALL *so_vline)(__WM_SURFACE_POINTER_R self, int x, int y, unsigned int size_y, wm_pixel_t pixel);

    /* Draw a rectangle on the inner borders of a given area. */
#define wm_surface_rect(self,x,y,size_x,size_y,pixel) \
     (*(self)->s_ops->so_rect)(self,x,y,size_x,size_y,pixel)
    void       (WMCALL *so_rect)(__WM_SURFACE_POINTER_R self, int x, int y, unsigned int size_x, unsigned int size_y, wm_pixel_t pixel);

    /* Fill (memset) a surface area with a solid color. */
#define wm_surface_fill(self,x,y,size_x,size_y,pixel) \
     (*(self)->s_ops->so_fill)(self,x,y,size_x,size_y,pixel)
    void       (WMCALL *so_fill)(__WM_SURFACE_POINTER_R self, int x, int y, unsigned int size_x, unsigned int size_y, wm_pixel_t pixel);

    /* Perform a basic blit operation that copies pixel from `source' and adjusts bits-per-pixel. */
#define wm_surface_bblit(self,x,y,source,source_x,source_y,size_x,size_y) \
     (*(self)->s_ops->so_bblit)(self,x,y,source,source_x,source_y,size_x,size_y)
    void       (WMCALL *so_bblit)(__WM_SURFACE_POINTER_R self, int x, int y,
                                  __WM_SURFACE_CONST_POINTER_R source,
                                  int source_x, int source_y,
                                  unsigned int size_x, unsigned int size_y);

    /* Same as `so_bblit', but leave pixels in `self' unchanged if their value
     * in `source' equals `color_key', essentially implementing a cheap and fast
     * way of 1-bit alpha blending. */
#define wm_surface_cblit(self,x,y,source,source_x,source_y,size_x,size_y,color_key) \
     (*(self)->s_ops->so_cblit)(self,x,y,source,source_x,source_y,size_x,size_y,color_key)
    void       (WMCALL *so_cblit)(__WM_SURFACE_POINTER_R self, int x, int y,
                                  __WM_SURFACE_CONST_POINTER_R source,
                                  int source_x, int source_y,
                                  unsigned int size_x, unsigned int size_y,
                                  wm_pixel_t color_key);

    /* Similar to `wm_surface_cblit()', but rather than copying pixels from
     * `source', load a source pixel and compare it to `color_key'.
     * If they are equal, don't blit anything for that pixel.
     * If they are not equal, fill the target pixel with `color'
     * This function is used by `wm_font_draw()' to render colored text. */
#define wm_surface_ccblit(self,x,y,source,source_x,source_y,size_x,size_y,color_key,color) \
     (*(self)->s_ops->so_ccblit)(self,x,y,source,source_x,source_y,size_x,size_y,color_key,color)
    void       (WMCALL *so_ccblit)(__WM_SURFACE_POINTER_R self, int x, int y,
                                   __WM_SURFACE_CONST_POINTER_R source,
                                   int source_x, int source_y,
                                   unsigned int size_x, unsigned int size_y,
                                   wm_pixel_t color_key, wm_pixel_t color);
};






__SYSDECL_END

#endif /* !_WM_SURFACE_H */
