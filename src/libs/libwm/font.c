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
#ifndef GUARD_LIBS_LIBWM_FONT_C
#define GUARD_LIBS_LIBWM_FONT_C 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <wm/api.h>
#include <wm/font.h>
#include <wm/surface.h>

#include "libwm.h"

DECL_BEGIN

DEFINE_PUBLIC_ALIAS(wm_font_draw,libwm_font_draw);
INTERN void WMCALL
libwm_font_draw(struct wm_font const *__restrict self,
                char const *__restrict text, size_t textlen,
                struct wm_surface *__restrict dst, int x, int y,
                struct wm_text_state *state, unsigned int flags) {


}


PRIVATE byte_t const default_font_data[(128 / 8)*64] = {
    /* TODO */
};


INTDEF struct wm_format libwm_format_monochrome;
INTDEF struct wm_surface_ops surface1_ops;
PRIVATE struct wm_font wm_default_font = {
    .s_refcnt   = 0x3fffffff,
    .s_ops      = &surface1_ops,
    .s_format   = &libwm_format_monochrome,
    .s_flags    = WM_SURFACE_FISFONT,
    .s_lock     = ATOMIC_RWLOCK_INIT,
    .s_sizex    = 128,
    .s_sizey    = 64,
    .s_stride   = 128 / 8,
    .s_buffer   = (byte_t *)default_font_data,
    .f_colorkey = 0,
    .f_lnsiz    = 9,
    .f_chpad    = 1,
    .f_default  = {
        .f_color   = 0,
        .f_options = WM_TEXT_OPTION_FNORMAL,
    },
    .f_ascii = {
        /* TODO */
    }
};


DEFINE_PUBLIC_ALIAS(wm_font_system,libwm_font_system);
INTERN ATTR_RETNONNULL struct wm_font const *
WMCALL libwm_font_system(unsigned int name) {
 struct wm_font const *result;
 switch (name) {

 case WM_FONT_SYSTEM_DEFAULT:
  result = &wm_default_font;
  break;

 default:
  error_throw(E_INVALID_ARGUMENT);
  break;
 }
 return result;
}



DECL_END

#endif /* !GUARD_LIBS_LIBWM_FONT_C */
