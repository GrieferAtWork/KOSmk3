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
#define __EXPOSE_CPU_CONTEXT 1

#include <hybrid/compiler.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <kos/kernctl.h>
#include <kos/heap.h>
#include <kos/bochs-vbe.h>
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
#include <kos/i386-kos/io/vga.h>
#include <kos/ushare.h>
#include <asm/cpu-flags.h>
#include <errno.h>

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


#ifndef CONFIG_X86_FIXED_SEGMENTATION
void vm86_map_identity(void) {
 struct mmap_info info;
 memset(&info,0,sizeof(info));
 info.mi_addr            = 0;
 info.mi_prot            = PROT_READ|PROT_WRITE|PROT_EXEC;
 info.mi_flags           = MAP_PRIVATE|MAP_FIXED;
 info.mi_xflag           = XMAP_USHARE;
 info.mi_size            = USHARE_X86_VM86BIOS_FSIZE;
 info.mi_align           = PAGESIZE;
 info.mi_gap             = 0;
 info.mi_tag             = NULL;
 info.mi_ushare.mu_name  = USHARE_X86_VM86BIOS_FNAME;
 info.mi_ushare.mu_start = 0;
 Xxmmap(&info);
}

void print_bios_vga_mode(int mode) {
 struct cpu_context context; pid_t child;
 byte_t *text;
 byte_t *sp;
 memset(&context,0,sizeof(context));

 /* Map the identity page for the X86 bios. */
 vm86_map_identity();

 text    = (byte_t *)0x10000;
 sp      = (byte_t *)0x20000-2;
 text[0] = 0xcd;
 text[1] = 0x10; /* int $0x10 */
 text[2] = 0xf4; /* hlt # Cause it to crash (ugly, but this way we can join it) */

 context.c_eflags        = EFLAGS_VM;
 context.c_cs            = ((uintptr_t)text & ~0xffff)/16;
 context.c_ss            = ((uintptr_t)sp & ~0xffff)/16;
 context.c_pip           = (uintptr_t)text & 0xffff;
 context.c_psp           = (uintptr_t)sp & 0xffff;
 context.c_gpregs.gp_eax = mode;
 child = Xxclone(&context,CLONE_NEW_THREAD & ~CLONE_THREAD,NULL,NULL,NULL);
 Xwaitpid(child,NULL,WEXITED);

 {
  struct vga_mode mode;
  /* Read out the VGA mode, as set by the BIOS */
  Xioctl(wms_display,VGA_GETMODE,&mode);
  syslog(LOG_DEBUG,".vm_att_mode          = 0x%.2I8x\n",mode.vm_att_mode         );
  syslog(LOG_DEBUG,".vm_att_overscan      = 0x%.2I8x\n",mode.vm_att_overscan     );
  syslog(LOG_DEBUG,".vm_att_plane_enable  = 0x%.2I8x\n",mode.vm_att_plane_enable );
  syslog(LOG_DEBUG,".vm_att_pel           = 0x%.2I8x\n",mode.vm_att_pel          );
  syslog(LOG_DEBUG,".vm_att_color_page    = 0x%.2I8x\n",mode.vm_att_color_page   );
  syslog(LOG_DEBUG,".vm_mis               = 0x%.2I8x\n",mode.vm_mis              );
  syslog(LOG_DEBUG,".vm_gfx_sr_value      = 0x%.2I8x\n",mode.vm_gfx_sr_value     );
  syslog(LOG_DEBUG,".vm_gfx_sr_enable     = 0x%.2I8x\n",mode.vm_gfx_sr_enable    );
  syslog(LOG_DEBUG,".vm_gfx_compare_value = 0x%.2I8x\n",mode.vm_gfx_compare_value);
  syslog(LOG_DEBUG,".vm_gfx_data_rotate   = 0x%.2I8x\n",mode.vm_gfx_data_rotate  );
  syslog(LOG_DEBUG,".vm_gfx_mode          = 0x%.2I8x\n",mode.vm_gfx_mode         );
  syslog(LOG_DEBUG,".vm_gfx_misc          = 0x%.2I8x\n",mode.vm_gfx_misc         );
  syslog(LOG_DEBUG,".vm_gfx_compare_mask  = 0x%.2I8x\n",mode.vm_gfx_compare_mask );
  syslog(LOG_DEBUG,".vm_gfx_bit_mask      = 0x%.2I8x\n",mode.vm_gfx_bit_mask     );
  syslog(LOG_DEBUG,".vm_crt_h_total       = 0x%.2I8x\n",mode.vm_crt_h_total      );
  syslog(LOG_DEBUG,".vm_crt_h_disp        = 0x%.2I8x\n",mode.vm_crt_h_disp       );
  syslog(LOG_DEBUG,".vm_crt_h_blank_start = 0x%.2I8x\n",mode.vm_crt_h_blank_start);
  syslog(LOG_DEBUG,".vm_crt_h_blank_end   = 0x%.2I8x\n",mode.vm_crt_h_blank_end  );
  syslog(LOG_DEBUG,".vm_crt_h_sync_start  = 0x%.2I8x\n",mode.vm_crt_h_sync_start );
  syslog(LOG_DEBUG,".vm_crt_h_sync_end    = 0x%.2I8x\n",mode.vm_crt_h_sync_end   );
  syslog(LOG_DEBUG,".vm_crt_v_total       = 0x%.2I8x\n",mode.vm_crt_v_total      );
  syslog(LOG_DEBUG,".vm_crt_overflow      = 0x%.2I8x\n",mode.vm_crt_overflow     );
  syslog(LOG_DEBUG,".vm_crt_preset_row    = 0x%.2I8x\n",mode.vm_crt_preset_row   );
  syslog(LOG_DEBUG,".vm_crt_max_scan      = 0x%.2I8x\n",mode.vm_crt_max_scan     );
  syslog(LOG_DEBUG,".vm_crt_v_sync_start  = 0x%.2I8x\n",mode.vm_crt_v_sync_start );
  syslog(LOG_DEBUG,".vm_crt_v_sync_end    = 0x%.2I8x\n",mode.vm_crt_v_sync_end   );
  syslog(LOG_DEBUG,".vm_crt_v_disp_end    = 0x%.2I8x\n",mode.vm_crt_v_disp_end   );
  syslog(LOG_DEBUG,".vm_crt_offset        = 0x%.2I8x\n",mode.vm_crt_offset       );
  syslog(LOG_DEBUG,".vm_crt_underline     = 0x%.2I8x\n",mode.vm_crt_underline    );
  syslog(LOG_DEBUG,".vm_crt_v_blank_start = 0x%.2I8x\n",mode.vm_crt_v_blank_start);
  syslog(LOG_DEBUG,".vm_crt_v_blank_end   = 0x%.2I8x\n",mode.vm_crt_v_blank_end  );
  syslog(LOG_DEBUG,".vm_crt_mode          = 0x%.2I8x\n",mode.vm_crt_mode         );
  syslog(LOG_DEBUG,".vm_crt_line_compare  = 0x%.2I8x\n",mode.vm_crt_line_compare );
  syslog(LOG_DEBUG,".vm_seq_clock_mode    = 0x%.2I8x\n",mode.vm_seq_clock_mode   );
  syslog(LOG_DEBUG,".vm_seq_plane_write   = 0x%.2I8x\n",mode.vm_seq_plane_write  );
  syslog(LOG_DEBUG,".vm_seq_character_map = 0x%.2I8x\n",mode.vm_seq_character_map);
  syslog(LOG_DEBUG,".vm_seq_memory_mode   = 0x%.2I8x\n",mode.vm_seq_memory_mode  );
 }
 {
  struct vga_palette pal;
  unsigned int i;
  Xioctl(wms_display,VGA_GETPAL,&pal);
  for (i = 0; i < 256; ++i) {
   syslog(LOG_DEBUG,"C(0x%.2I8x,0x%.2I8x,0x%.2I8x),",
          pal.vp_pal[i].c_red >> 2,
          pal.vp_pal[i].c_green >> 2,
          pal.vp_pal[i].c_blue >> 2);
   if ((i % 8) == 7) syslog(LOG_DEBUG,"\n");

  }
 }
}
#endif /* !CONFIG_X86_FIXED_SEGMENTATION */


int main(int argc, char *argv[]) {
 fd_t EXCEPT_VAR bochs_vbe;
 /* Setup all the bindings. */
 wms_keyboard = Xopen(WMS_PATH_KEYBOARD,O_RDWR);
 wms_mouse    = Xopen(WMS_PATH_MOUSE,O_RDWR);
 wms_display  = Xopen(WMS_PATH_DISPLAY,O_RDWR);

 bochs_vbe = open("/dev/bochs-vbe",O_RDWR);

 TRY {

  /* Switch to graphics mode. */
  if (bochs_vbe >= 0) {
   /* Make use of BOCHS VBE extensions. */
   struct bochs_vbe_format format;
   format.vf_resx = 1024;
   format.vf_resy = 768;
   /* XXX: Larger BPP values? */
   format.vf_bpp  = 8;
   Xioctl(bochs_vbe,BOCHS_VBE_SETFORMAT,&format);
   Xioctl(bochs_vbe,BOCHS_VBE_SETENABLE,BOCHS_VBE_FENABLED);
   Xioctl(bochs_vbe,BOCHS_VBE_GETFORMAT,&format);
   default_display.d_sizex  = format.vf_resx;
   default_display.d_sizey  = format.vf_resy;
   default_display.d_stride = format.vf_resx;
   default_display.d_bpp    = format.vf_bpp;
   default_display.d_screen = (byte_t *)Xmmap(0,8*1024*1024,PROT_READ|PROT_WRITE,
                                              MAP_PRIVATE,bochs_vbe,0);
   Xioctl(wms_display,VGA_SETPAL_DEFAULT,VGA_DEFAULT_PAL_GFX256);

  } else {
   Xioctl(wms_display,VGA_SETMODE_DEFAULT,VGA_DEFAULT_MODE_GFX320X200_256);
   Xioctl(wms_display,VGA_SETPAL_DEFAULT,VGA_DEFAULT_PAL_GFX256);
   default_display.d_sizex  = 320;
   default_display.d_sizey  = 200;
   default_display.d_stride = 320;
   default_display.d_bpp    = 8;

   /* Map VGA Display memory. */
   default_display.d_screen = (byte_t *)Xmmap(0,8192*4*4,PROT_READ|PROT_WRITE,
                                              MAP_PRIVATE,wms_display,0);
  }
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
  default_display.d_backvisi.r_strips->rs_blkc            = 1;
  default_display.d_backvisi.r_strips->rs_chain.le_next   = NULL;
  default_display.d_backvisi.r_strips->rs_xmin            = 0;
  default_display.d_backvisi.r_strips->rs_xsiz            = default_display.d_sizex;
  default_display.d_backvisi.r_strips->rs_blkv[0].rb_ymin = 0;
  default_display.d_backvisi.r_strips->rs_blkv[0].rb_ysiz = default_display.d_sizey;


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
  if (bochs_vbe >= 0)
      ioctl(bochs_vbe,BOCHS_VBE_SETENABLE,BOCHS_VBE_FDISABLED);
  /* Return to text mode. */
  ioctl(wms_display,VGA_RESET,
        VGA_RESET_FMODE|
        VGA_RESET_FFONT|
        VGA_RESET_FPAL);
 }
 return 0;
}



DECL_END

#endif /* !GUARD_APPS_WMS_MAIN_C */
