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
#ifndef _WM_EVENT_H
#define _WM_EVENT_H 1

#include "api.h"
#include "surface.h"
#include <hybrid/compiler.h>
#include <features.h>
#include <stdbool.h>
#include <bits/types.h>
#include <kos/keyboard-ioctl.h>
#include <kos/mouse-ioctl.h>

__SYSDECL_BEGIN

struct wm_window;

typedef __UINTPTR_HALF_TYPE__ wm_event_type_t;
typedef __UINTPTR_HALF_TYPE__ wm_event_flag_t;

#define WM_EVENT_NONE    0x0000 /* No event (should never be triggered) */
#define WM_EVENT_KEY     0x0001 /* Keyboard event (button pressed / released) */
#define WM_EVENT_MOUSE   0x0002 /* Mouse event (motion or button press) */
#define WM_EVENT_WINDOW  0x0010 /* Window-related event */
#define WM_EVENT_FNORMAL 0x0000 /* Normal event flags. */

#if defined(__BUILDING_LIBWM) || defined(__BUILDING_WMSERVER)
/* For obvious reasons, events send through the communications pipe encode
 * windows using IDs, rather than using the actual window structures. */
#define __WM_EVENT_WINDOW_HEADER(prefix) \
    union PACKED { \
        REF struct wm_window *prefix##window;  /* [1..1] The window targeted by the event. */ \
        wms_window_id_t       prefix##winid;   /* Window ID. */ \
    };
#else
#define __WM_EVENT_WINDOW_HEADER(prefix) \
        REF struct wm_window *prefix##window;  /* [1..1] The window targeted by the event. */
#endif

struct PACKED wm_commonevent {
    /* Common event header. */
    wm_event_type_t             c_type;    /* The type of event (One of `WM_EVENT_*') */
    wm_event_flag_t             c_flag;    /* Event flags (Set of `WM_EVENT_F*') */
    __WM_EVENT_WINDOW_HEADER(c_)
};
struct PACKED wm_keyevent {
    wm_event_type_t             k_type;    /* == WM_EVENT_KEY */
    wm_event_flag_t             k_flag;    /* Event flags (Set of `WM_EVENT_F*') */
    __WM_EVENT_WINDOW_HEADER(k_)
    keyboard_key_t              k_key;     /* The key pressed / released. */
    keyboard_state_t            k_state;   /* The keyboard state. */
    struct keyboard_keymap_char k_text;    /* UTF-8 encoded text that should be written in text prompts as a response to this key event.
                                            * When no such response should be performed, this is an empty string (aka. all NUL characters) */
    char                        k_zero;    /* Always ZERO (so-as to allow `k_text' to be used as NUL-terminated, C-style string) */
};
struct PACKED wm_mouseevent {
    wm_event_type_t             m_type;    /* == WM_EVENT_MOUSE */
    wm_event_flag_t             m_flag;    /* Event flags (Set of `WM_EVENT_MOUSE_F*') */
#define WM_EVENT_MOUSE_FENTER   0x0001     /* This is the first event after the mouse was moved over this window. */
#define WM_EVENT_MOUSE_FLEAVE   0x0002     /* Trailing event send after the mouse left the window. (The `m_mouse(x|y)' fields are truncated to fit into the window) */
    __WM_EVENT_WINDOW_HEADER(m_)
    int                         m_mousex;  /* The window-relative X mouse coord (with motion already applied). */
    int                         m_mousey;  /* The window-relative Y mouse coord (with motion already applied). */
    int                         m_clrelx;  /* The display-clamped relative mouse movement in X (aka. the cursor movement) */
    int                         m_clrely;  /* The display-clamped relative mouse movement in Y (aka. the cursor movement)  */
    struct mouse_packet         m_packet;  /* The mouse packet containing all the data.
                                            * This packet must be parsed to determine click events. */
};

#define WM_WINDOWEVENT_MOVED        0x0001 /* The window was moved. */
#define WM_WINDOWEVENT_RESIZED      0x0002 /* The usable area of the window has changed (also
                                            * send when features like `WM_WINDOW_FEAT_FNOHEADER'
                                            * are toggled) */
#define WM_WINDOWEVENT_STATE_CHANGE 0x0003 /* The window's state (`w_state') has changed. */
#define WM_WINDOWEVENT_CLOSE        0x0004 /* The window's close button was pressed.
                                            * This event is sent after the window was hidden.
                                            * However the window-hide part is skipped when the
                                            * `WM_WINDOW_FEAT_FNOAUTOHIDE' feature flag is set.
                                            * NOTE: This event is also triggered when `ALT-F4'
                                            *       is pressed and `WM_WINDOW_FEAT_FNOALTF4'
                                            *       isn't set, or when `wm_window_close()' is
                                            *       called. */
struct PACKED wm_windowevent {
    wm_event_type_t             w_type;    /* == WM_EVENT_WINDOW */
    wm_event_flag_t             w_flag;    /* Event flags (Set of `WM_EVENT_F*') */
    __WM_EVENT_WINDOW_HEADER(w_)
    __UINTPTR_HALF_TYPE__       w_event;   /* The type of window event that happend (One of `WM_WINDOWEVENT_*') */
    __UINTPTR_HALF_TYPE__     __w_pad;     /* ... */
    union {
        struct {
            int                 m_oldx;    /* The old window X coord. */
            int                 m_oldy;    /* The old window Y coord. */
            int                 m_newx;    /* The new window X coord. */
            int                 m_newy;    /* The new window Y coord. */
        }                       w_moved;   /* WM_WINDOWEVENT_MOVED */
        struct {
            unsigned int        r_oldxsiz; /* The old window X size (in pixels). */
            unsigned int        r_oldysiz; /* The old window Y size (in pixels). */
            unsigned int        r_newxsiz; /* The new window X size (in pixels). */
            unsigned int        r_newysiz; /* The new window Y size (in pixels). */
#ifdef __BUILDING_LIBWM
            unsigned int        r_stride;  /* The new stride of the window's display buffer. */
            unsigned int        r_bpp;     /* New bits per pixel. */
#endif /* __BUILDING_LIBWM */
        }                       w_resized; /* WM_WINDOWEVENT_RESIZED */
        struct {
            __uint16_t          s_oldstat; /* The old window state (Set of `WM_WINDOW_STATE_F*'). */
            __uint16_t          s_newstat; /* The new window state (Set of `WM_WINDOW_STATE_F*'). */
            __uint16_t        __s_pad[2];  /* ... */
        }                       w_changed; /* WM_WINDOWEVENT_STATE_CHANGE */
    };                                     /* Window-event-specific data. */
};
#undef __WM_EVENT_WINDOW_HEADER

union PACKED wm_event {
    wm_event_type_t             e_type;    /* The type of event (One of `WM_EVENT_*') */
    struct wm_commonevent       e_common;  /* Common event fields. */
    struct wm_keyevent          e_key;     /* WM_EVENT_KEY */
    struct wm_mouseevent        e_mouse;   /* WM_EVENT_MOUSE */
    struct wm_windowevent       e_window;  /* WM_EVENT_WINDOW */
};


struct PACKED wm_winevent_ops {
    /* NOTE: All window event operators are [0..1] */
    /* An optional callback that is executed before a window is destroyed. */
    void (WMCALL *wo_fini)(struct wm_window *__restrict win);
    /* Fallback/default event handler triggered when the
     * associated, dedicated event event handler isn't
     * implemented. */
    void (WMCALL *wo_event)(union wm_event const *__restrict info);
    /* Dedicated immediate event handlers. */
    void (WMCALL *wo_key)(struct wm_keyevent const *__restrict info);
    void (WMCALL *wo_mouse)(struct wm_mouseevent const *__restrict info);
    void (WMCALL *wo_window)(struct wm_windowevent const *__restrict info);
};


/* Empty the user-space event queue and handle events stored the
 * the kernel-space WMS command pipe (s.a. `WMS_RESPONSE_EVENT')
 * If the kernel WMS command pipe is empty, wait until something
 * shows up before returning.
 * This function is intended to repeatedly be called
 * from the main loop of a GUI-based application:
 * >> #include <wm/event.h>
 * >> #include <wm/window.h>
 * >>
 * >> int main() {
 * >>     wm_init();
 * >>     
 * >>     struct wm_window *w;
 * >>     w = wm_window_create(...);
 * >>     wm_window_settitle(w,"My window");
 * >>     wm_window_show(w);
 * >>
 * >>     // Process events...
 * >>     for (;;) wm_event_process();
 * >> }
 * REMINDER: Events are attempted to be processed using
 *          `struct wm_winevent_ops *w_events' before
 *           they are added to the user-space event queue,
 *           meaning that the above code example would attempt
 *           to invoke callbacks registered for the window
 *           before saving them in the .
 */
WMAPI void WMCALL wm_event_process(void);

/* Empty the user-space event queue and handle events stored the
 * the kernel-space WMS command pipe (s.a. `WMS_RESPONSE_EVENT')
 * If either contains an event, store that event in `result' and
 * return `true', otherwise return `false' */
WMAPI bool WMCALL wm_event_trywait(union wm_event *__restrict result);

/* Same as `wm_event_trywait()', but rather than returning `false',
 * wait for events to appear on the kernel-space WMS command pipe. */
WMAPI void WMCALL wm_event_waitfor(union wm_event *__restrict result);


__SYSDECL_END

#endif /* !_WM_EVENT_H */
