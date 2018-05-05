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
#ifndef GUARD_LIBS_LIBWM_LIBWM_H
#define GUARD_LIBS_LIBWM_LIBWM_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <wm/api.h>
#include <wm/event.h>
#include <wm/server.h>
#include <wm/surface.h>
#include <wm/window.h>

DECL_BEGIN

/* api.h */
INTDEF void WMCALL libwm_init(void);
INTDEF ATTR_NOTHROW void WMCALL libwm_fini(void);

/* event.h */
INTDEF void WMCALL libwm_event_dealwith(union wm_event *__restrict evt);
INTDEF void WMCALL libwm_event_process(void);
INTDEF bool WMCALL libwm_event_trywait(union wm_event *__restrict result);
INTDEF void WMCALL libwm_event_waitfor(union wm_event *__restrict result);

/* server.h */
INTDEF void WMCALL libwms_handle(struct wms_response *__restrict resp);
INTDEF unsigned int WMCALL libwms_sendrequest(struct wms_request *__restrict req);
INTDEF void WMCALL libwms_recvresponse(unsigned int token, struct wms_response *__restrict resp);
/* Same as `libwms_recvresponse()', but also receive
 * a single file descriptor from ancillary data. */
INTDEF fd_t WMCALL libwms_recvresponse_fd(unsigned int token, struct wms_response *__restrict resp);

/* surface.h */
INTDEF struct wm_palette libwm_palette_256;
INTDEF ATTR_RETNONNULL REF struct wm_palette *WMCALL
libwm_palette_create(u16 bpp, u8 mask);
#define libwm_palette_incref(self) (void)ATOMIC_FETCHINC((self)->p_refcnt)
#define libwm_palette_decref(self) (void)(ATOMIC_DECFETCH((self)->p_refcnt) || (libwm_palette_destroy(self),0))
INTDEF void WMCALL libwm_palette_destroy(struct wm_palette *__restrict self);
INTDEF ATTR_RETNONNULL REF struct wm_format *WMCALL
libwm_format_create(wm_pixel_t rmask, u16 rshft,
                    wm_pixel_t gmask, u16 gshft,
                    wm_pixel_t bmask, u16 bshft,
                    wm_pixel_t amask, u16 ashft);
INTDEF ATTR_RETNONNULL REF struct wm_format *WMCALL
libwm_format_create_pal(struct wm_palette *__restrict pal);
#define libwm_format_incref(self) (void)ATOMIC_FETCHINC((self)->f_refcnt)
#define libwm_format_decref(self) (void)(ATOMIC_DECFETCH((self)->f_refcnt) || (libwm_format_destroy(self),0))
INTDEF void WMCALL libwm_format_destroy(struct wm_format *__restrict self);
INTDEF ATTR_RETNONNULL REF struct wm_surface *WMCALL
libwm_surface_create(struct wm_format *__restrict format,
                     unsigned int sizex, unsigned int sizey);
INTDEF void WMCALL libwm_surface_destroy(struct wm_surface *__restrict self);
#define libwm_surface_incref(self) (void)ATOMIC_FETCHINC((self)->s_refcnt)
#define libwm_surface_decref(self) (void)(ATOMIC_DECFETCH((self)->s_refcnt) || (libwm_surface_destroy(self),0))
INTDEF void WMCALL libwm_surface_resize(struct wm_surface *__restrict self, unsigned int new_sizx, unsigned int new_sizy);
INTDEF ATTR_RETNONNULL REF struct wm_surface *WMCALL
libwm_surface_convert(struct wm_surface *__restrict self,
                      struct wm_format *__restrict new_format);


/* Return a pointer for the best matching surface operators for `bpp' */
INTDEF ATTR_NOTHROW ATTR_RETNONNULL struct wm_surface_ops const *
WMCALL libwm_lookup_surface_ops(unsigned int bpp);

/* window.h */
INTDEF ATTR_RETNONNULL REF struct wm_window *WMCALL
libwm_window_create(int pos_x, int pos_y, unsigned int size_x, unsigned int size_y,
                    char const *title, u32 features, u16 state, u16 mode,
                    struct wm_winevent_ops *eventops, void *userdata);
INTDEF void WMCALL libwm_window_destroy(struct wm_window *__restrict self);
#define libwm_window_incref(self) (void)ATOMIC_FETCHINC((self)->w_surface.s_refcnt)
#define libwm_window_decref(self) (void)(ATOMIC_DECFETCH((self)->w_surface.s_refcnt) || (libwm_window_destroy(self),0))

INTDEF void WMCALL libwm_window_move(struct wm_window *__restrict self, int new_posx, int new_posy);
INTDEF void WMCALL libwm_window_resize(struct wm_window *__restrict self, unsigned int new_sizx, unsigned int new_sizy);
INTDEF void WMCALL libwm_window_settitle(struct wm_window *__restrict self, char const *__restrict new_title);
INTDEF void WMCALL libwm_window_draw(struct wm_window *__restrict self, unsigned int mode);
INTDEF void WMCALL libwm_window_draw_rect(struct wm_window *__restrict self, int xmin, int ymin, unsigned int xsiz, unsigned int ysiz, unsigned int mode);
INTDEF void WMCALL libwm_window_draw_rects(struct wm_window *__restrict self, size_t rectc, struct wm_rect const *__restrict rectv, unsigned int mode);
INTDEF bool WMCALL libwm_window_bring_to_front(struct wm_window *__restrict self);
INTDEF void WMCALL libwm_window_close(struct wm_window *__restrict self);
INTDEF u32 WMCALL libwm_window_chfeat(struct wm_window *__restrict self, u32 mask, u32 flag);
INTDEF u16 WMCALL libwm_window_chstat(struct wm_window *__restrict self, u16 mask, u16 flag);
INTDEF u16 WMCALL libwm_window_chmode(struct wm_window *__restrict self, u16 mask, u16 flag);
INTDEF ATTR_CONST wms_window_id_t WMCALL libwm_window_getid(struct wm_window *__restrict self);
INTDEF REF struct wm_window *WMCALL libwm_window_fromid(wms_window_id_t id);


/* The file descriptors used for communication with the server. */
INTDEF fd_t libwms_socket; /* client -> server */


DECL_END

#endif /* !GUARD_LIBS_LIBWM_LIBWM_H */
