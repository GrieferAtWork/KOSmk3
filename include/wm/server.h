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
#ifndef _WM_SERVER_H
#define _WM_SERVER_H 1

#include "api.h"
#include "surface.h"
#include "event.h"
#include "window.h"
#include <features.h>
#include <bits/types.h>
#include <except.h>

__SYSDECL_BEGIN


#define WMS_COMMAND_TEST    0x0000 /* Test / echo command. (does nothing, but the
                                    * `r_echo' field of the response can be checked
                                    * against the request's echo field to synchronize
                                    * the communications channel) */
#define WMS_COMMAND_MKWIN   0x0001 /* Create a new window. */
#define WMS_COMMAND_RMWIN   0x0002 /* Delete a window. */
#define WMS_COMMAND_MVWIN   0x0003 /* Move a window. */
#define WMS_COMMAND_RZWIN   0x0004 /* Resize a window. */
#define WMS_COMMAND_CHWIN   0x0005 /* Change the state of a window. */
#define WMS_COMMAND_DRAWALL 0x0006 /* Draw the entire window. */
#define WMS_COMMAND_DRAWONE 0x0007 /* Draw a single window rectangle. */
#define WMS_COMMAND_TOFRONT 0x0008 /* Move a window to the front of the Z order. */

#define WMS_COMMAND_FNORMAL 0x0000 /* Normal command flags. */
#define WMS_COMMAND_FNOACK  0x0001 /* Don't send a command acknowledge response packet.
                                    * This flag is used to implement asynchronous commands. */
#define WMS_COMMAND_DRAW_FVSYNC 0x0002 /* Wait for a display synchronization before acknowledging.
                                        * Ignored when `WMS_COMMAND_FNOACK' is specified, too. */
struct PACKED wms_request {
    __uint16_t                    r_command; /* The command that should be performed. */
    __uint16_t                    r_flags;   /* Command flags (Set of `WMS_COMMAND_F*'). */
    unsigned int                  r_echo;    /* A packet ID that is echoed by the response. */
    union PACKED {
        struct PACKED {
            int                   mw_xmin;   /* The display-relative initial X position of the window. */
            int                   mw_ymin;   /* The display-relative initial Y position of the window.
                                              * NOTE: passing `WM_WINDOW_AUTOPOS' will have the server
                                              *       automatically determine a suitable location for
                                              *       the window. */
            unsigned int          mw_xsiz;   /* The initial size in X of the usable window surface area. */
            unsigned int          mw_ysiz;   /* The initial size in Y of the usable window surface area. */
            __uint16_t            mw_state;  /* The initial state of the window. */
        }                         r_mkwin;   /* [WMS_COMMAND_MKWIN] Create a new window. */
        struct PACKED {
            wms_window_id_t       rw_winid;  /* The ID of the window in question. */
        }                         r_rmwin;   /* [WMS_COMMAND_RMWIN] Delete a window. */
        struct PACKED {
            wms_window_id_t       mw_winid;  /* The ID of the window in question. */
            int                   mw_xmin;   /* The display-relative new X position of the window. */
            int                   mw_ymin;   /* The display-relative new Y position of the window. */
        }                         r_mvwin;   /* [WMS_COMMAND_MVWIN] Move a window. */
        struct PACKED {
            wms_window_id_t       rw_winid;  /* The ID of the window in question. */
            unsigned int          rw_xsiz;   /* New window size in X. */
            unsigned int          rw_ysiz;   /* New window size in Y. */
        }                         r_rzwin;   /* [WMS_COMMAND_RZWIN] Resize a window. */
        struct PACKED {
            wms_window_id_t       cw_winid;  /* The ID of the window in question. */
            __uint16_t            cw_mask;   /* Window state mask. */
            __uint16_t            cw_flag;   /* Window state flags. */
        }                         r_chwin;   /* [WMS_COMMAND_CHWIN] Change window state. */
        struct PACKED {
            wms_window_id_t       dw_winid;  /* The ID of the window in question. */
        }                         r_drawall; /* [WMS_COMMAND_DRAWALL] Draw window contents. */
        struct PACKED {
            wms_window_id_t       dw_winid;  /* The ID of the window in question. */
            unsigned int          dw_xmin;   /* The window-relative X position that should be redrawn. */
            unsigned int          dw_ymin;   /* The window-relative Y position that should be redrawn. */
            unsigned int          dw_xsiz;   /* The number of pixels in X to redraw. */
            unsigned int          dw_ysiz;   /* The number of pixels in Y to redraw. */
        }                         r_drawone; /* [WMS_COMMAND_DRAWONE] Draw window contents. */
        struct PACKED {
            wms_window_id_t       fw_winid;  /* The ID of the window in question. */
        }                         r_tofront; /* [WMS_COMMAND_TOFRONT] Bring window to the front. */
    };
};




#define WMS_RESPONSE_ACK        0x0000 /* Generic ACK (sent when `WMS_COMMAND_FNOACK' isn't set,
                                        * and the command doesn't have a special response packet) */
#define WMS_RESPONSE_RESEND     0x0001 /* The command could not be completed. (Try again for a limited number of times) */
#define WMS_RESPONSE_FAILED     0x0002 /* The execution of the command failed. (see exception information) */
#define WMS_RESPONSE_BADCMD     0x0003 /* The specified command is unknown, or not supported. */
#define WMS_RESPONSE_EVENT      0x0008 /* An event occurred in a window managed by this application. */
#define WMS_RESPONSE_MKWIN_OK   0x0010 /* Newly created window. */
#define WMS_RESPONSE_RZWIN_OK   0x0011 /* Window successfully resized. */
#define WMS_RESPONSE_CHWIN_OK   0x0012 /* Window state successfully changed. */
#define WMS_RESPONSE_TOFRONT_OK 0x0013 /* If the window wasn't already in front, this is send. Otherwise, `WMS_RESPONSE_ACK' is send. */
#define WMS_RESPONSE_FNORMAL    0x0000 /* Normal response flags. */
struct PACKED wms_response {
    __uint16_t                    r_answer;  /* The type of answer (One of `WMS_RESPONSE_*') */
    __uint16_t                    r_flags;   /* Response flags (Set of `WMS_RESPONSE_F*') */
    unsigned int                  r_echo;    /* The original value of `p_echo'.
                                              * Used to re-align WMS packets and discard packets
                                              * from past commands that were performed asynchronously.
                                              * Event-related packets (`WMS_RESPONSE_EVENT')
                                              * will have this field set to ZERO(0). */
    union PACKED {
        struct PACKED {
            __uint16_t            f_command; /* The command that caused the exception. */
            __uint16_t          __f_pad[(sizeof(void *)-2)/2]; /* The command that caused the exception. */
            struct exception_data f_except;  /* Exception data describing the exception
                                              * that caused the command to fail.
                                              * The user-application should re-throw this exception,
                                              * as it was probably their fault that something went
                                              * wrong. */
        }                         r_failed;  /* [WMS_RESPONSE_FAILED] Window creation response. */
        struct PACKED {
            __uint16_t            b_command; /* The command that was attempted to be executed. */
        }                         r_badcmd;  /* [WMS_RESPONSE_BADCMD] Bad command. */
        union wm_event            r_event;   /* [WMS_RESPONSE_EVENT] Window creation response. */
        struct PACKED {
            wms_window_id_t       w_id;      /* The ID of the newly created window. */
            int                   w_posx;    /* The X position where the window was created. */
            int                   w_posy;    /* The Y position where the window was created. */
            unsigned int          w_sizex;   /* The X size of the created window. */
            unsigned int          w_sizey;   /* The Y size of the created window. */
            unsigned int          w_stride;  /* The stride of the window's display buffer. */
            unsigned int          w_bpp;     /* Bits per pixel. */
            __uint16_t            w_state;   /* The initial window state. */
            /* In its ancillary data, this response carries a file descriptor
             * which, when used to mmap()-ed at offset ZERO(0) a number of
             * `w_sizey * w_stride' bytes, then contains the screen buffer.
             * NOTE: memory must be mapped as `PROT_SHARED'! */
        }                         r_mkwin;   /* [WMS_RESPONSE_MKWIN_OK] Window creation response. */
        struct PACKED {
            wms_window_id_t     __w_pad;     /* ... */
            int                 __w_pad2;    /* ... */
            int                 __w_pad3;    /* ... */
            unsigned int          w_sizex;   /* The X size of the window. */
            unsigned int          w_sizey;   /* The Y size of the window. */
            unsigned int          w_stride;  /* The stride of the window's display buffer. */
            unsigned int          w_bpp;     /* Bits per pixel. */
        }                         r_rzwin;   /* [WMS_RESPONSE_RZWIN_OK] Window resize response. */
        struct PACKED {
            __uint16_t            cw_oldst;  /* Old window state. */
            __uint16_t            cw_newst;  /* New window state. */
        }                         r_chwin;   /* [WMS_RESPONSE_CHWIN_OK] Change window state. */
    };
};




/* Send a request to the WM server.
 * @param: req: The request to send to the server.
 *              WARNING: The function may (will) modify this buffer.
 * @return: * : A token (`r_echo' field) that can be used to receive a
 *              response from the server when the `WMS_COMMAND_FNOACK'
 *              flag wasn't set in the given `req'. */
WMAPI unsigned int WMCALL wms_sendrequest(struct wms_request *__restrict req);

/* Receive a response for `token' from the WMS server.
 * Event response packets are also handled immediately by this function.
 * Upon success, the server response is written to `resp'.
 * @throw: * :                 The server response was a `WMS_RESPONSE_FAILED'
 * @throw: E_NOT_IMPLEMENTED:  The server response was a `WMS_RESPONSE_BADCMD'
 * @throw: E_INVALID_ARGUMENT: The server didn't respond in time (by default: 2 seconds) */
WMAPI void WMCALL wms_recvresponse(unsigned int token, struct wms_response *__restrict resp);


__SYSDECL_END

#endif /* !_WM_SERVER_H */
