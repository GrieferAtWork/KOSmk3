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
#ifndef GUARD_APPS_WMS_DISPLAY_H
#define GUARD_APPS_WMS_DISPLAY_H 1

#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kos/types.h>
#include <wm/api.h>

#include "rect.h"
#include "window.h"

DECL_BEGIN

typedef struct display {
    fd_t              d_display;  /* [const] A file descriptor for the display. */
    atomic_rwlock_t   d_lock;     /* Lock for this display. */
    unsigned int      d_sizex;    /* [const] The display width in pixels. */
    unsigned int      d_sizey;    /* [const] The display height in pixels. */
    unsigned int      d_stride;   /* [const] Length of a screen buffer (`d_screen') line (in bytes) */
    unsigned int      d_bpp;      /* [const][!0] Bits per pixel in `d_screen'. */
    byte_t           *d_screen;   /* [1..1][const] Screen display buffer (this is device memory) */
    byte_t           *d_backgrnd; /* [1..1][const][owned] Display background (for parts where are not windows) */
    struct rects      d_backvisi; /* [lock(d_lock)] Parts of the display that should be rendered from `d_backgrnd' */
    LIST_HEAD(Window) d_zorder;   /* [lock(d_lock)] Chain of all windows present on this display.
                                   *                Ordered by their Z-order; front to back (w_zlink). */
    LIST_HEAD(Window) d_windows;  /* [lock(d_lock)] Chain of all visible windows (w_vlink). */
} Display;

/* The default display adapter. */
INTDEF Display default_display;

/* Do a full redraw of all visible windows. */
INTDEF void WMCALL Display_Redraw(Display *__restrict self);


DECL_END

#endif /* !GUARD_APPS_WMS_DISPLAY_H */
