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
#ifndef GUARD_APPS_WM_MAIN_C
#define GUARD_APPS_WM_MAIN_C 1
#define _KOS_SOURCE 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <wm/api.h>
#include <wm/window.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

DECL_BEGIN

int main(int argc, char *argv[]) {
 struct wm_window *win;
 struct wm_surface *sfc;
 wm_init();

 win = wm_window_create(WM_WINDOW_AUTOPOS,
                        WM_WINDOW_AUTOPOS,
                        200,
                        160,
                        "My Window",
                        WM_WINDOW_FEAT_FNORMAL,
                        WM_WINDOW_STATE_FNORMAL,
                        WM_WINDOW_MODE_FNORMAL,
                        NULL,
                        NULL);

 sfc = wm_window_surface(win);
 for (unsigned int i = 0; i < 1000; ++i) {
  wm_surface_setpixel(sfc,
                     (unsigned int)rand() % sfc->s_sizex,
                     (unsigned int)rand() % sfc->s_sizey,
                      rand());
 }
 wm_window_draw(win,WM_WINDOW_DRAW_FNORMAL);

 pause();
 wm_window_decref(win);
}

DECL_END

#endif /* !GUARD_APPS_WM_MAIN_C */
