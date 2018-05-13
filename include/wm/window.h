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
#ifndef _WM_WINDOW_H
#define _WM_WINDOW_H 1

#include "api.h"
#include "surface.h"
#include <features.h>
#include <stdbool.h>
#include <bits/types.h>
#include <hybrid/limitcore.h>
#ifdef __BUILDING_LIBWM
#include <hybrid/list/list.h>
#endif

__SYSDECL_BEGIN

/* Window feature flags. */
#define WM_WINDOW_FEAT_FNORMAL      0x0000 /* Normal window features. */
#define WM_WINDOW_FEAT_FNOBORDER    0x0001 /* Border-less window. */
#define WM_WINDOW_FEAT_FNOHEADER    0x0002 /* Header-less window. */
#define WM_WINDOW_FEAT_FNORESIZE    0x0004 /* Window cannot be resized by clicking & holding one of its borders. */
#define WM_WINDOW_FEAT_FDONTGRAB    0x0008 /* The window bar cannot be grabbed to move the window around. */
#define WM_WINDOW_FEAT_FNOAUTOHIDE  0x0010 /* Don't automatically hide the window when the close button is clicked. */
#define WM_WINDOW_FEAT_FEXITONCLOSE 0x0020 /* Throw an `E_EXIT_PROCESS' exception with status=0 when the window is closed. */
#define WM_WINDOW_FEAT_FNOTATASK    0x0040 /* Don't display the window in the task bar. */
#define WM_WINDOW_FEAT_FNOMIN       0x0100 /* The minimize-button doesn't exist. */
#define WM_WINDOW_FEAT_FNOMAX       0x0200 /* The maximize-button doesn't exist. */
#define WM_WINDOW_FEAT_FNOCLOSE     0x0400 /* The close-button doesn't exist. */
#define WM_WINDOW_FEAT_FNOALTF4     0x0800 /* The window cannot be closed using ALT-F4. */
#define WM_WINDOW_FEAT_FGRAYMIN     0x1000 /* The minimize-button cannot be clicked and is rendered differently. */
#define WM_WINDOW_FEAT_FGRAYMAX     0x2000 /* The maximize-button cannot be clicked and is rendered differently. */
#define WM_WINDOW_FEAT_FGRAYCLOSE   0x4000 /* The close-button cannot be clicked and is rendered differently. */

/* Window state flags. */
#define WM_WINDOW_STATE_FNORMAL     0x0000 /* Normal window state flags. */
#define WM_WINDOW_STATE_FHIDDEN     0x0001 /* The window has been hidden.
                                            * As far as the implementation is concerned, this flag
                                            * being set has the same effect as though the following
                                            * flags were set:
                                            *  - WM_WINDOW_FEAT_FNOTATASK
                                            *  - WM_WINDOW_MODE_FMINIMIZED
                                            * However, this flag operates independently and will
                                            * not modify other flags when set. */
#define WM_WINDOW_STATE_FFOCUSED    0x0002 /* [weak] The window is currently focused by the keyboard. */

/* Window mode flags. */
#define WM_WINDOW_MODE_FNORMAL      0x0000 /* Normal window mode. */
#define WM_WINDOW_MODE_FMINIMIZED   0x0001 /* The window has been minimized. */
#define WM_WINDOW_MODE_FMAXIMIZED   0x0002 /* The window has been maximized. */
#define WM_WINDOW_MODE_FFULLSCREEN  0x0004 /* The window takes up the whole screen. */

#ifdef __cplusplus
struct PACKED wm_window: wm_surface
#else
struct PACKED wm_window
#endif
{
    __WM_SURFACE_STRUCT_MEMBERS         /* The underlying window surface.
                                         * NOTE: This surface describes the exposed window
                                         *       data, as opposed to the real window data.
                                         *       In actuality, things like the close buttons,
                                         *       or the title bar are implement by the WM
                                         *       host application and are part of this surface.
                                         *       However, by adjusting the base pointer and width
                                         *       without modifying the stride, we can hide that
                                         *       detail. */
    int                     w_posx;     /* [lock(w_surface.s_lock)] Display-relative X-position of the window. */
    int                     w_posy;     /* [lock(w_surface.s_lock)] Display-relative Y-position of the window. */
    __uint32_t              w_features; /* [lock(w_surface.s_lock)] Window features (Set of `WM_WINDOW_FEAT_F*'). */
    __uint16_t              w_state;    /* [lock(w_surface.s_lock)] Window state (Set of `WM_WINDOW_STATE_F*'). */
    __uint16_t              w_mode;     /* [lock(w_surface.s_lock)] Window mode (Set of `WM_WINDOW_MODE_F*'). */
    void                   *w_userdata; /* [?..?] User-defined data field. */
    struct wm_winevent_ops *w_events;   /* [0..1][const] Window event callbacks for immediate processing.
                                         * When set to non-NULL, events generated for this window will
                                         * not be sent to the global (per-application) event pool, but
                                         * rather cause these callbacks to be executed immediately.
                                         * When defined, any operator not implemented inside is simply ignored. */
    char                   *w_title;    /* [0..1][lock(w_surface.s_lock)][owned] The title of the window. */
    void                   *w_pad[3];   /* ... */
#ifdef __BUILDING_LIBWM
    __byte_t               *w_buffer;   /* [1..1][lock(w_surface.s_lock)][owned]
                                         * The actual, underlying window buffer.
                                         * The surface buffer buffer only points back into this one. */
    unsigned int            w_sizex;    /* [lock(w_surface.s_lock)] The total window size in X (including borders). */
    unsigned int            w_sizey;    /* [lock(w_surface.s_lock)] The total window size in Y (including borders and the title bar). */
    unsigned int            w_oldsizex; /* [lock(w_surface.s_lock)] The old window size in X before `WM_WINDOW_MODE_FMAXIMIZED' or `WM_WINDOW_MODE_FFULLSCREEN' was set. */
    unsigned int            w_oldsizey; /* [lock(w_surface.s_lock)] The old window size in Y before `WM_WINDOW_MODE_FMAXIMIZED' or `WM_WINDOW_MODE_FFULLSCREEN' was set. */
    unsigned int            w_titlesz;  /* [lock(w_surface.s_lock)] The width of the border on the top (ymin). */
    unsigned int            w_bordersz; /* [lock(w_surface.s_lock)] The width of the border on all sides except for the top (ymin). */
    wms_window_id_t         w_winid;    /* [const][owned] Internal ID of the window. */
    __fd_t                  w_winfd;    /* [const][owned] A file descriptor referring to the mmap()able window display buffer. */
    LIST_NODE(struct wm_window) w_chain;/* [lock(INTERNAL(...))] Chain of windows with the same modulated hash. (for `wm_window_fromid()') */
#endif /* __BUILDING_LIBWM */
};

/* Increment/decrement the reference counter of a WM window.
 * NOTE: When the last reference of a window that hasn't been
 *       closed is dropped, the window will be set as hidden,
 *       but no `WM_WINDOWEVENT_STATE_CHANGE' event is sent
 *      (as that would create a new reference which is something
 *       we couldn't do in that situation, considering we just
 *       dropped the last one)
 *       If this isn't intended, you may consider to always
 *       explicitly hide the window first, or configure its
 *       features accordingly and call `wm_window_close()'. */
#define wm_window_incref(self)  wm_surface_incref(self)
#define wm_window_decref(self)  wm_surface_decref(self)

/* Special value which, when passing for `pos_x' _and_ `pos_y' in
 * a call to `wm_window_create()' will allow the implementation to
 * determine the location where to place the window automatically. */
#define WM_WINDOW_AUTOPOS    __INT_MAX__

/* Create a new window.
 * @param: pos_x:    The display-relative initial X position of the window.
 * @param: pos_y:    The display-relative initial Y position of the window.
 *          HINT:    You may pass `WM_WINDOW_AUTOPOS' for both `pos_x' and
 *                  `pos_y' to have the WM runtime automatically determine
 *                   a suitable location for the newly created window.
 * @param: size_x:   The initial size in X of the usable window surface area.
 * @param: size_y:   The initial size in Y of the usable window surface area.
 * @param: title:    The initial title of the window, or NULL if unnamed.
 * @param: features: The initial set of window features (Set of `WM_WINDOW_FEAT_F*')
 * @param: state:    The initial window state (Set of `WM_WINDOW_STATE_F*')
 * @param: mode:     The initial window mode (Set of `WM_WINDOW_MODE_F*')
 * @param: eventops: Callbacks for processing events directed at this window.
 *                   If you wish for events for this window to be added to
 *                   the user-space event queue (s.a. `wm_event_waitfor()'),
 *                   you should pass NULL for this argument.
 * @param: userdata: The initial value of the `w_userdata' field.
 * @return: * :      A reference to the newly created window. */
WMAPI ATTR_RETNONNULL REF struct wm_window *WMCALL
wm_window_create(int pos_x,
                 int pos_y,
                 unsigned int size_x,
                 unsigned int size_y,
#ifdef __cplusplus
                 char const *title                = __NULLPTR,
                 __uint32_t features              = WM_WINDOW_FEAT_FNORMAL,
                 __uint16_t state                 = WM_WINDOW_STATE_FNORMAL,
                 __uint16_t mode                  = WM_WINDOW_MODE_FNORMAL,
                 struct wm_winevent_ops *eventops = __NULLPTR,
                 void *userdata                   = __NULLPTR
#else
                 char const *title,
                 __uint32_t features,
                 __uint16_t state,
                 __uint16_t mode,
                 struct wm_winevent_ops *eventops,
                 void *userdata
#endif
                 );

/* Change the size of the usable area of a window.
 * If it wasn't already obvious by the fact that this is forwarded
 * to the surface-resize function, resizing a window will change the
 * effective surface size, that is the window size _exclusing_ its
 * border and title bar. */
#define wm_window_resize(self,new_sizx,new_sizy) \
        wm_surface_resize(self,new_sizx,new_sizy)


/* Move the given window to a new position. */
WMAPI void WMCALL wm_window_move(struct wm_window *__restrict self,
                                 int new_posx, int new_posy);

/* Set the new title of the given window. */
WMAPI void WMCALL
wm_window_settitle(struct wm_window *__restrict self,
                   char const *__restrict new_title);

struct wm_rect {
    int          r_xmin; /* Starting X coord. */
    int          r_ymin; /* Starting Y coord. */
    unsigned int r_xsiz; /* Number of pixels in X. */
    unsigned int r_ysiz; /* Number of pixels in Y. */
};

/* Redraw the window by copying the specified
 * portions of its surface onto the display.
 * The specified rectangles are relative to the window itself,
 * and any part of the window that is obstructed by another window
 * with a greater Z-order will obviously not be re-drawn.
 * @param: mode: Set of `WM_WINDOW_DRAW_F*' */
WMAPI void WMCALL wm_window_draw(struct wm_window *__restrict self, unsigned int mode);
WMAPI void WMCALL wm_window_draw_rect(struct wm_window *__restrict self,
                                      int xmin, int ymin,
                                      unsigned int xsiz, unsigned int ysiz,
                                      unsigned int mode);
WMAPI void WMCALL wm_window_draw_rects(struct wm_window *__restrict self, __size_t rectc,
                                       struct wm_rect const *__restrict rectv,
                                       unsigned int mode);
#define WM_WINDOW_DRAW_FNORMAL 0x0000 /* Normal window draw flags. */
#define WM_WINDOW_DRAW_FASYNC  0x0001 /* Don't wait for the window server to copy
                                       * surface data onto the display buffer.
                                       * Instead, the copy will be performed at some
                                       * random point into the future, meaning that
                                       * any further modifications made may or may
                                       * not appear before wm_window_draw is called again.
                                       * WARNING: It is unwise to set this flag for callbacks
                                       *          from some kind of game loop, however setting
                                       *          it after having redrawn GUI elements is a
                                       *          very good idea. */
#define WM_WINDOW_DRAW_FVSYNC  0x0002 /* After copying window data, wait for the next
                                       * display synchronization pulse to go through. */


/* Bring the window to the front of the Z-buffer.
 * @return: true:  The window was brought to the front.
 * @return: false: The window was already in the front.
 * WARNING: Because not only the user, but also all other applications
 *          that created windows are able to call this function in order
 *          to bring their window to the front of the Z-buffer, by the
 *          time this function returns, the window way no longer be in
 *          the front, irregardless of what was returned. */
WMAPI bool WMCALL wm_window_bring_to_front(struct wm_window *__restrict self);

/* Close the window:
 *  #1: When `WM_WINDOW_FEAT_FNOAUTOHIDE' isn't set, hide the window.
 *  #2: When `WM_WINDOW_FEAT_FEXITONCLOSE' is set, throw an E_EXIT_PROCESS exception
 *  #3: Trigger a `WM_WINDOWEVENT_CLOSE' window event.
 * WARNING: This function may be called more than once, in which case the
 *         `WM_WINDOWEVENT_CLOSE' may be triggered more than once, too.
 *          This is a race condition that cannot be inhibited due to
 *          the fact that the user might clock on close at the same time
 *          some internal piece of code calls this function. */
WMAPI void WMCALL wm_window_close(struct wm_window *__restrict self);

/* Change the window's features / state or mode.
 * @param: mask: Mask of bits to preserve.
 * @param: flag: Mask of bits to turn on.
 * @return: * :  The old window features / state or mode. */
WMAPI __uint32_t WMCALL wm_window_chfeat(struct wm_window *__restrict self, __uint32_t mask, __uint32_t flag);
WMAPI __uint16_t WMCALL wm_window_chstat(struct wm_window *__restrict self, __uint16_t mask, __uint16_t flag);
WMAPI __uint16_t WMCALL wm_window_chmode(struct wm_window *__restrict self, __uint16_t mask, __uint16_t flag);


/* Grab keyboard focus and have it deliver its signals to this window.
 * WARNING: Since keyboard focus can be controlled by the user, that
 *          also means that the actual focus can change at any moment
 *          Because of that, the return value of these functions cannot
 *          actually be trusted to indicate the current focus at the
 *          time of the caller checking interpreting their value.
 * @return: true:  The window has acquired the keyboard focus.
 * @return: false: The window had already acquired the keyboard focus. */
LOCAL bool WMCALL wm_window_focus(struct wm_window *__restrict self) { return !(wm_window_chstat(self,~0,WM_WINDOW_STATE_FFOCUSED) & WM_WINDOW_STATE_FFOCUSED); }
LOCAL bool WMCALL wm_window_unfocus(struct wm_window *__restrict self) { return !!(wm_window_chstat(self,~WM_WINDOW_STATE_FFOCUSED,0) & WM_WINDOW_STATE_FFOCUSED); }

/* Show/Hide the given window, setting/clearing the `WM_WINDOW_STATE_FHIDDEN' flag.
 * @return: true:  Successfully changed the window's state.
 * @return: false: The operation was a no-op because the window was already hidden/visible. */
LOCAL bool WMCALL wm_window_hide(struct wm_window *__restrict self) { return !(wm_window_chstat(self,~0,WM_WINDOW_STATE_FHIDDEN) & WM_WINDOW_STATE_FHIDDEN); }
LOCAL bool WMCALL wm_window_show(struct wm_window *__restrict self) { return !!(wm_window_chstat(self,~WM_WINDOW_STATE_FHIDDEN,0) & WM_WINDOW_STATE_FHIDDEN); }

/* Restore a normal window mode (non-maximized, non-minimized, non-fullscreen)
 * @return: true:  Successfully changed the window's mode.
 * @return: false: The operation was a no-op because the window already had the desired mode. */
LOCAL bool WMCALL wm_window_restore(struct wm_window *__restrict self) { return wm_window_chmode(self,WM_WINDOW_MODE_FNORMAL,WM_WINDOW_MODE_FNORMAL) != WM_WINDOW_MODE_FNORMAL; }
LOCAL bool WMCALL wm_window_minimize(struct wm_window *__restrict self) { return wm_window_chmode(self,WM_WINDOW_MODE_FNORMAL,WM_WINDOW_MODE_FMINIMIZED) != WM_WINDOW_MODE_FMINIMIZED; }
LOCAL bool WMCALL wm_window_maximize(struct wm_window *__restrict self) { return wm_window_chmode(self,WM_WINDOW_MODE_FNORMAL,WM_WINDOW_MODE_FMAXIMIZED) != WM_WINDOW_MODE_FMAXIMIZED; }
LOCAL bool WMCALL wm_window_fullscreen(struct wm_window *__restrict self) { return wm_window_chmode(self,WM_WINDOW_MODE_FNORMAL,WM_WINDOW_MODE_FFULLSCREEN) != WM_WINDOW_MODE_FFULLSCREEN; }


/* Translate a window to its id or do the reverse.
 * @return: NULL: [wm_window_fromid] No window is associated with the given ID. */
WMAPI ATTR_NOTHROW ATTR_CONST wms_window_id_t WMCALL wm_window_getid(struct wm_window *__restrict self);
WMAPI ATTR_NOTHROW REF struct wm_window *WMCALL wm_window_fromid(wms_window_id_t id);


/* Construct a view of a window in its entirety, including
 * its title bar, as well as any potential border.
 * When the window has all of the following flags
 * set, this view equals the window itself:
 *   - WM_WINDOW_FEAT_FNOBORDER
 *   - WM_WINDOW_FEAT_FNOHEADER
 * @return: * : [== view] Always re-return `view' */
WMAPI ATTR_NOTHROW ATTR_RETNONNULL struct wm_surface_view *WMCALL
wm_window_viewall(struct wm_surface_view *__restrict view,
                  struct wm_window const *__restrict self);

/* Very similar to `wm_window_viewall', but instead used to only view the title bar.
 * If there is no title bar, the returned view is empty */
WMAPI ATTR_NOTHROW ATTR_RETNONNULL struct wm_surface_view *WMCALL
wm_window_viewtitle(struct wm_surface_view *__restrict view,
                    struct wm_window const *__restrict self);


__SYSDECL_END

#endif /* !_WM_WINDOW_H */
