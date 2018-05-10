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
#ifndef GUARD_APPS_WMS_WINDOW_H
#define GUARD_APPS_WMS_WINDOW_H 1

#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <kos/types.h>
#include <wm/api.h>
#include <wm/window.h>
#include <stdbool.h>

#include "rect.h"

DECL_BEGIN

typedef struct window {
    ATOMIC_DATA ref_t        w_weakcnt;  /* Weak window reference counter. */
    LIST_NODE(struct window) w_zlink;    /* [lock(w_display->d_lock)]
                                          * All existing windows, ordered by Z-order:
                                          *   - le_next points to a window with a lower Z-order (behind this window)
                                          *   - le_pself points to a window with a greater Z-order (infront of this window) */
    LIST_NODE(struct window) w_vlink;    /* [lock(w_display->d_lock)] All visible windows (windows with content that aren't entirely covered by other windows) */
    int                      w_posx;     /* [lock(w_display->d_lock)] Window X position (in pixels; relative to the display). */
    int                      w_posy;     /* [lock(w_display->d_lock)] Window Y position (in pixels; relative to the display). */
#define WM_WINDOW_STATE_FDESTROYED 0x8000 /* [lock(w_display->d_lock)]
                                           * Internal window state indicative of a window that has been destroyed.
                                           * Until set, the window owner thread is holding a reference to `w_weakcnt' */
    u16                      w_state;    /* [lock(w_display->d_lock)] Set of `WM_WINDOW_STATE_F*'. */
    u16                    __w_pad;      /* ... */
    unsigned int             w_sizex;    /* [lock(w_display->d_lock)][!0] The window width in pixels. */
    unsigned int             w_sizey;    /* [lock(w_display->d_lock)][!0] The window height in pixels. */
    struct rect              w_disparea; /* [lock(w_display->d_lock)] Window rectangle actually drawn on-screen (coords are display-relative) */
    unsigned int             w_stride;   /* [lock(w_display->d_lock)] Length of a screen buffer (`w_screen') line (in bytes) */
    struct display          *w_display;  /* [1..1][const] The associated display. */
    byte_t                  *w_screen;   /* [1..1][owned][lock(w_display->d_lock)] The window screen buffer. (Allocated in shared memory) */
    struct rects             w_visi;     /* [lock(w_display->d_lock)]
                                          * Vector of window portions not obstructed by other windows.
                                          * Coords are relative to the window itself. */
    wms_window_id_t          w_id;       /* ID of this window within the associated process. */
    fd_t                     w_screenfd; /* [const] An anonymous memory region used to map `w_screen', and shared with the client. */
    fd_t                     w_clientfd; /* [const] The accept(2)-ed socket used to communicate with the client. */
    LIST_NODE(struct window) w_idchain;  /* [owned] Chain of windows with a similar ID hash. */
} Window;

#define Window_WeakIncref(self)  ATOMIC_FETCHINC((self)->w_weakcnt)
#define Window_WeakDecref(self) (ATOMIC_DECFETCH((self)->w_weakcnt) || (free(self),0))


/* Get/Set the pixel at a specific coord. */
INTDEF unsigned int WMCALL Window_GetPixel(struct window *__restrict self,
                                           unsigned int x, unsigned int y);
INTDEF void WMCALL Window_PutPixel(struct window *__restrict self,
                                   unsigned int x, unsigned int y,
                                   unsigned int pixel);


/* Construct a new window. */
INTDEF Window *WMCALL
Window_CreateUnlocked(struct display *__restrict disp,
                      int posx, int posy,
                      unsigned int sizex,
                      unsigned int sizey,
                      u16 state, fd_t client_fd);

/* Destroy the given window. */
INTDEF void WMCALL Window_DestroyUnlocked(Window *__restrict self);

/* Move around the given window. */
INTDEF void WMCALL Window_MoveUnlocked(Window *__restrict self,
                                       int new_posx, int new_posy);

/* Copy visible window portions also covered by `vec' onto the display buffer. */
INTDEF void WMCALL Window_DrawRectsUnlocked(Window *__restrict self,
                                           size_t count, struct rect *__restrict vec);
INTDEF void WMCALL Window_DrawRectUnlocked(Window *__restrict self, struct rect r);
INTDEF void WMCALL Window_DrawUnlocked(Window *__restrict self);

/* Hide / show the given window. */
INTDEF void WMCALL Window_HideUnlocked(Window *__restrict self);
INTDEF void WMCALL Window_ShowUnlocked(Window *__restrict self);
INTDEF bool WMCALL Window_BringToFront(Window *__restrict self);
INTDEF void WMCALL Window_ChangeStateUnlocked(Window *__restrict self, u16 mask, u16 flag, unsigned int token);

struct wms_response;
/* Set a message to the given window. */
INTDEF bool WMCALL
Window_SendMessage(Window *__restrict self,
                   struct wms_response const *__restrict msg);



typedef struct {
    wms_window_id_t    wm_size;     /* Number of windows in the window map. */
    wms_window_id_t    wm_mask;     /* Hash-mask. */
    wms_window_id_t    wm_nextid;   /* The next window ID that should be assigned. */
    LIST_HEAD(Window) *wm_map;      /* [0..1][owned][1..wm_mask+1][owned] Hash map. */
    bool               wm_overflow; /* Set to true once `wm_nextid' has overflown. */
} WindowMap;

INTDEF void WMCALL WindowMap_Init(WindowMap *__restrict self);
INTDEF ATTR_NOTHROW void WMCALL WindowMap_Fini(WindowMap *__restrict self);
INTDEF ATTR_NOTHROW wms_window_id_t WMCALL WindowMap_MakeID(WindowMap *__restrict self);
INTDEF void WMCALL WindowMap_Insert(WindowMap *__restrict self, Window *__restrict win);
INTDEF ATTR_NOTHROW void WMCALL WindowMap_Remove(WindowMap *__restrict self, Window *__restrict win);
/* @return: NULL: Invalid window ID. */
INTDEF ATTR_NOTHROW Window *WMCALL WindowMap_Lookup(WindowMap *__restrict self, wms_window_id_t id);



DECL_END

#endif /* !GUARD_APPS_WMS_WINDOW_H */
