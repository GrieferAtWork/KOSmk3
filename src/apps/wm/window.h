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
#ifndef GUARD_APPS_WM_WINDOW_H
#define GUARD_APPS_WM_WINDOW_H 1

#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <kos/types.h>

#include "rect.h"

DECL_BEGIN

struct window;
struct display;

struct window {
    LIST_NODE(struct window) w_zlink;    /* All existing windows, ordered by Z-order:
                                          *   - le_next points to a window with a lower Z-order (behind this window)
                                          *   - le_pself points to a window with a greater Z-order (infront of this window) */
    LIST_NODE(struct window) w_vlink;    /* All visible windows (windows with content that aren't entirely covered by other windows) */
    int                      w_posx;     /* Window X position (in pixels; relative to the display). */
    int                      w_posy;     /* Window Y position (in pixels; relative to the display). */
#define WINDOW_FNORMAL       0x0000      /* Normal window flags. */
#define WINDOW_FHIDDEN       0x0001      /* The window is invisible (as though `w_visi' was empty). */
    unsigned int             w_flags;    /* Set of `WINDOW_F*'. */
    unsigned int             w_sizex;    /* [!0] The window width in pixels. */
    unsigned int             w_sizey;    /* [!0] The window height in pixels. */
    struct rect              w_disparea; /* Window rectangle actually drawn on-screen (coords are display-relative) */
    unsigned int             w_stride;   /* Length of a screen buffer (`w_screen') line (in bytes) */
    struct display          *w_display;  /* [1..1][const] The associated display. */
    byte_t                  *w_screen;   /* [1..1][owned] The window screen buffer. (Allocated in shared memory) */
    struct rects             w_visi;     /* Vector of window portions not obstructed by other windows.
                                          * Coords are relative to the window itself. */
    struct rects             w_grab;     /* Vector of rectangles where the window can be grabbed (i.e click + move using the mouse). */
    struct rects             w_capt;     /* Vector of rectangles for which mouse events are translated into window events.
                                          * This vector overlays over `w_visi', meaning that mouse events are handled before grab events. */
};


/* Get/Set the pixel at a specific coord. */
INTDEF unsigned int KCALL window_getpixel(struct window *__restrict self,
                                          unsigned int x, unsigned int y);
INTDEF void KCALL window_putpixel(struct window *__restrict self,
                                  unsigned int x, unsigned int y,
                                  unsigned int pixel);

/* Construct a new window. */
INTDEF struct window *KCALL
window_create(struct display *__restrict disp,
              int posx, unsigned int sizex,
              int posy, unsigned int sizey,
              unsigned int flags);

/* Destroy the given window. */
INTDEF void KCALL window_destroy(struct window *__restrict self);

/* Move around the given window. */
INTDEF void KCALL window_move(struct window *__restrict self,
                              int new_posx, int new_posy);

/* Copy visible window portions also covered by `vec' onto the display buffer. */
INTDEF void KCALL window_draw_rects(struct window *__restrict self,
                                    size_t count, struct rect *__restrict vec);
INTDEF void KCALL window_draw_rect(struct window *__restrict self, struct rect r);

/* Hide / show the given window. */
INTDEF void KCALL window_hide(struct window *__restrict self);
INTDEF void KCALL window_show(struct window *__restrict self);


struct display {
    unsigned int             d_sizex;    /* The display width in pixels. */
    unsigned int             d_sizey;    /* The display height in pixels. */
    unsigned int             d_stride;   /* Length of a screen buffer (`d_screen') line (in bytes) */
    unsigned int             d_bpp;      /* [!0] Bits per pixel in `d_screen'. */
    byte_t                  *d_screen;   /* [1..1] Screen display buffer (this is device memory) */
    byte_t                  *d_backgrnd; /* [1..1][owned] Display background (for parts where are not windows) */
    struct rects             d_backvisi; /* Parts of the display that should be rendered from `d_backgrnd' */
    LIST_HEAD(struct window) d_zorder;   /* Chain of all windows present on this display.
                                          * Ordered by their Z-order; front to back (w_zlink). */
    LIST_HEAD(struct window) d_windows;  /* Chain of all visible windows (w_vlink). */
};

INTDEF struct display default_display;
INTDEF int fd_mouse;
INTDEF int fd_keyboard;


/* Do a full redraw of all visible windows. */
INTDEF void KCALL display_redraw(struct display *__restrict self);



/* Copy a rectangle from one buffer to another. */
INTDEF void KCALL
copy_rect(byte_t *__restrict dst_buffer,
          unsigned int dst_x, unsigned int dst_y, unsigned int dst_stride,
          byte_t const *__restrict src_buffer,
          unsigned int src_x, unsigned int src_y, unsigned int src_stride,
          unsigned int size_x, unsigned int size_y,
          unsigned int bpp);


DECL_END

#endif /* !GUARD_APPS_WM_WINDOW_H */
