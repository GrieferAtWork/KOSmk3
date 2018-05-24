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
#include <wm/font.h>
#include <wm/event.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <sys/select.h>

DECL_BEGIN

PRIVATE void move_window(wm_window_t *__restrict win, int xoff, int yoff) {
 unsigned int i;
 for (i = 0; i < 30; ++i) {
  struct timeval tmo;
  wm_window_move(win,
                 win->w_posx+xoff,
                 win->w_posy+yoff);
  tmo.tv_sec  = 0;
  tmo.tv_usec = 2500;
  select(0,NULL,NULL,NULL,&tmo);
 }
}


int main(int argc, char *argv[]) {
 wm_window_t *win;
 wm_init();
 unsigned int num;
 for (num = 0; num < 3; ++num) {

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
  syslog(LOG_DEBUG,"Created window %p,%p\n",win,win->s_ops);
  for (unsigned int i = 0; i < 1000; ++i) {
   wm_surface_fill(win,
                  (unsigned int)rand() % win->s_sizex,
                  (unsigned int)rand() % win->s_sizey,
                   8,
                   8,
                   win->s_format->f_color[rand() & 15]);
  }
  {
   char const *text;
   text = "This is my test string.\n"
          "And this is the second line!";
   wm_font_draw(wm_font_system(WM_FONT_SYSTEM_DEFAULT),
                text,
                strlen(text),
                win,
                0,
                0,
                NULL,
                WM_FONT_DRAW_FNORMAL);
  }
  wm_window_draw(win,WM_WINDOW_DRAW_FNORMAL);
 }

#if 1
 for (;;) {
  wm_event_t evt;
  wm_event_waitfor(&evt);
  if (evt.e_type == WM_EVENT_KEY &&
      KEY_ISDOWN(evt.e_key.k_key)) {
   switch (KEYCODE(evt.e_key.k_key)) {
   case KEY_J:
    move_window(evt.e_common.c_window,-1,0);
    break;
   case KEY_I:
    move_window(evt.e_common.c_window,0,-1);
    break;
   case KEY_K:
    move_window(evt.e_common.c_window,0,1);
    break;
   case KEY_L:
    move_window(evt.e_common.c_window,1,0);
    break;
   default: break;
   }
  }
  wm_event_fini(&evt);
 }
#else
 for (;;) wm_event_process();
#endif
 wm_window_decref(win);
}

DECL_END

#endif /* !GUARD_APPS_WM_MAIN_C */
