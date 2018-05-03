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
#include <unistd.h>
#include <sys/mman.h>
#include <kos/types.h>
#include <kos/kernctl.h>
#include <kos/keyboard.h>
#include <kos/keyboard-ioctl.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

#include "window.h"

DECL_BEGIN

INTERN int fd_mouse;
INTERN int fd_keyboard;

INTERN struct display default_display = {
    .d_sizex  = 320,
    .d_sizey  = 200,
    .d_stride = 320,
    .d_bpp    = 8
};

PRIVATE void *KCALL map_vga(void) {
 struct mmap_info info;
 info.mi_prot         = PROT_READ|PROT_WRITE;
 info.mi_flags        = MAP_PRIVATE;
 info.mi_xflag        = XMAP_PHYSICAL;
 info.mi_addr         = NULL;
 info.mi_size         = 8192*4*4; /* 128K */
 info.mi_align        = PAGESIZE;
 info.mi_gap          = 0;
 info.mi_tag          = NULL;
 info.mi_phys.mp_addr = (void *)0xA0000;
 return Xxmmap(&info);
}



int main(int argc, char *argv[]) {
/* Temporary, hidden command to enable video mode
 * until I get around to implement a new VGA driver. */
 struct window *win1,*win2;
#define KERNEL_CONTROL_ENABLE_VGA 0x86000123
 Xkernctl(KERNEL_CONTROL_ENABLE_VGA);

 default_display.d_screen   = (byte_t *)map_vga();
 default_display.d_backgrnd = (byte_t *)Xmalloc(320*200*1);
 memset(default_display.d_backgrnd,1,320*200*1);
 default_display.d_backvisi.r_strips = RECT_STRIP_ALLOC(1);
 default_display.d_backvisi.r_strips->rs_chain.le_next   = NULL;
 default_display.d_backvisi.r_strips->rs_blkc            = 1;
 default_display.d_backvisi.r_strips->rs_xmin            = 0;
 default_display.d_backvisi.r_strips->rs_xsiz            = 320;
 default_display.d_backvisi.r_strips->rs_blkv[0].rb_ymin = 0;
 default_display.d_backvisi.r_strips->rs_blkv[0].rb_ysiz = 200;

 /* Open the default mouse and keyboard for reading. */
 fd_mouse    = Xopen("/dev/mouse",O_RDONLY);
 fd_keyboard = Xopen("/dev/keyboard",O_RDONLY);

 /* Create a new window. */
 win1 = window_create(&default_display,
                      10,default_display.d_sizex-20,
                      10,default_display.d_sizey-20,
                      default_display.d_bpp,
                      WINDOW_FNORMAL);
 win2 = window_create(&default_display,
                      63,default_display.d_sizex-20,
                      28,default_display.d_sizey-20,
                      default_display.d_bpp,
                      WINDOW_FNORMAL);
 display_redraw(&default_display);

 for (;;) {
#if 1
  struct rect r;
  r.r_xsiz = 1;
  r.r_ysiz = 1;

  r.r_xmin = rand() % win1->w_sizex;
  r.r_ymin = rand() % win1->w_sizey;
  window_putpixel(win1,r.r_xmin,r.r_ymin,rand());
  window_draw_rect(win1,r);

  r.r_xmin = rand() % win2->w_sizex;
  r.r_ymin = rand() % win2->w_sizey;
  window_putpixel(win2,r.r_xmin,r.r_ymin,rand());
  window_draw_rect(win2,r);
#endif

  //display_redraw(&default_display);
  keyboard_key_t buf;
  read(fd_keyboard,&buf,sizeof(keyboard_key_t));
  syslog(LOG_DEBUG,"KEY PRESSED\n");
 }
 for (;;) pause();
 return 0;
}

DECL_END

#endif /* !GUARD_APPS_WM_MAIN_C */
