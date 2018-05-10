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
#ifndef GUARD_APPS_WMS_MAIN_C
#define GUARD_APPS_WMS_MAIN_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <kos/kernctl.h>
#include <string.h>
#include <malloc.h>
#include <syslog.h>
#include <wm/api.h>
#include <wm/server.h>
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <kos/i386-kos/vga.h>

#include "bind.h"
#include "display.h"
#include "server.h"
#include "event.h"

DECL_BEGIN

/* Filesystem bindings. */
INTERN fd_t wms_keyboard; /* open(WMS_PATH_KEYBOARD); */
INTERN fd_t wms_mouse;    /* open(WMS_PATH_MOUSE); */
INTERN fd_t wms_display;  /* open(WMS_PATH_DISPLAY); */
INTERN fd_t wms_server;   /* socket(AF_UNIX); bind(WMS_PATH_SERVER); */

PRIVATE DEFINE_SOCKADDR_UN(server_addr,WMS_PATH_SERVER);

PRIVATE pid_t init_process;

PRIVATE void
sigchld_handler(int UNUSED(signo),
                siginfo_t *info,
                struct ucontext *UNUSED(ctx)) {
 syslog(LOG_DEBUG,"sigchld_handler(%u) (looking for %u)\n",
        info->si_pid,init_process);
 if (info->si_pid != init_process)
     return;
 /* Exit the current process using the exit code of the child. */
 error_throwf(E_EXIT_PROCESS,
              WEXITSTATUS(info->si_status)
            ? WEXITSTATUS(info->si_status) + 0x30
            : 0);
}



int main(int argc, char *argv[]) {
 /* Setup all the bindings. */
 wms_keyboard = Xopen(WMS_PATH_KEYBOARD,O_RDWR);
 wms_mouse    = Xopen(WMS_PATH_MOUSE,O_RDWR);
 wms_display  = Xopen(WMS_PATH_DISPLAY,O_RDWR);

 /* Switch to graphics mode (256 color). */
 Xioctl(wms_display,VGA_SETMODE_DEFAULT,VGA_DEFAULT_MODE_GFX320X200_256);
 Xioctl(wms_display,VGA_SETPAL_DEFAULT,VGA_DEFAULT_PAL_GFX256);

 TRY {

  default_display.d_sizex  = 320;
  default_display.d_sizey  = 200;
  default_display.d_stride = 320;
  default_display.d_bpp    = 8;
  /* Map VGA Display memory. */
  default_display.d_screen = (byte_t *)Xmmap(0,8192*4*4,PROT_READ|PROT_WRITE,
                                             MAP_PRIVATE,wms_display,0);
  default_display.d_backgrnd = (byte_t *)Xcalloc(default_display.d_sizey,
                                                 default_display.d_stride);
  {
   unsigned int x,y;
   for (y = 0; y < default_display.d_sizey; ++y) {
    for (x = 0; x < default_display.d_sizex; ++x) {
     default_display.d_backgrnd[x + y*default_display.d_stride] = (u8)(x+y);
    }
   }
  }

  default_display.d_backvisi.r_strips = RECT_STRIP_ALLOC(1);
  default_display.d_backvisi.r_strips[0].rs_blkc = 1;
  default_display.d_backvisi.r_strips[0].rs_chain.le_next = NULL;
  default_display.d_backvisi.r_strips[0].rs_xmin = 0;
  default_display.d_backvisi.r_strips[0].rs_xsiz = default_display.d_sizex;
  default_display.d_backvisi.r_strips[0].rs_blkv[0].rb_ymin = 0;
  default_display.d_backvisi.r_strips[0].rs_blkv[0].rb_ysiz = default_display.d_sizey;


  /* Redraw the display for the first time. */
  Display_Redraw(&default_display);

  wms_server = Xsocket(AF_UNIX,
                       SOCK_STREAM|SOCK_CLOEXEC|SOCK_CLOFORK,
                       PF_UNIX);
  Xbind(wms_server,(struct sockaddr *)&server_addr,sizeof(server_addr));
  Xlisten(wms_server,5);

  /* Create the keyboard and mouse relay threads. */
  Xclone(&KeyboardRelayThread,CLONE_CHILDSTACK_AUTO,CLONE_NEW_THREAD,NULL);
  Xclone(&MouseRelayThread,CLONE_CHILDSTACK_AUTO,CLONE_NEW_THREAD,NULL);

  {
   /* Setup a signal handler to throw an exception when the init */
   struct sigaction act;
   memset(&act,0,sizeof(act));
   act.sa_flags     = SA_RESTART|SA_SIGINFO;
   act.sa_sigaction = &sigchld_handler;
   Xsigaction(SIGCLD,&act,NULL);
  }

  if ((init_process = Xfork()) == 0)
       Xexecv(argv[1],argv+1);

  for (;;) {
   fd_t client;
   client = Xaccept4(wms_server,NULL,NULL,SOCK_CLOEXEC);
   syslog(LOG_DEBUG,"[WMS] New client connected using FD %d\n",client);

   /* Spawn a new thread for managing the client. */
   TRY {
    AcceptConnection(client);
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    close(client);
    error_rethrow();
   }
  }
 } FINALLY {
  /* Return to text mode. */
  Xioctl(wms_display,VGA_RESET,
         VGA_RESET_FMODE|
         VGA_RESET_FFONT|
         VGA_RESET_FPAL);
 }
 return 0;
}



DECL_END

#endif /* !GUARD_APPS_WMS_MAIN_C */
