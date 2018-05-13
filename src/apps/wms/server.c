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
#ifndef GUARD_APPS_WMS_SERVER_C
#define GUARD_APPS_WMS_SERVER_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1
#define _EXCEPT_SOURCE 1
#define __BUILDING_WMSERVER 1

#include <hybrid/compiler.h>
#include <hybrid/align.h>
#include <hybrid/list/list.h>
#include <kos/types.h>
#include <wm/api.h>
#include <sys/socket.h>
#include <malloc.h>
#include <assert.h>
#include <syslog.h>
#include <stdbool.h>
#include <sched.h>
#include <string.h>
#include <except.h>
#include <unistd.h>
#include <wm/server.h>

#include "window.h"
#include "display.h"
#include "render.h"
#include "server.h"
#include <kos/futex.h>
#include <kos/thread.h>

DECL_BEGIN


PRIVATE int LIBCCALL ClientMain(void *arg) {
 fd_t EXCEPT_VAR client_fd = (fd_t)(intptr_t)(uintptr_t)arg;
 WindowMap windowmap;
 WindowMap *EXCEPT_VAR pwindowmap = &windowmap;
 /* Make our thread be stand-along so us crashing
  * won't bring down the entire server. */
 __current()->ts_state |= THREAD_STATE_FALONE;
 TRY {
  WindowMap_Init(&windowmap);
  TRY {
   for (;;) {
    struct wms_request req;
    struct wms_response resp;
    if (Xrecv(client_fd,&req,sizeof(struct wms_request),MSG_WAITALL) !=
                             sizeof(struct wms_request))
        break; /* Disconnect */
    syslog(LOG_DEBUG,"[WMS] Receive command: %u\n",req.r_command);

    memset(&resp,0,sizeof(struct wms_response));
    resp.r_echo = req.r_echo;

    TRY {
     /* Process the client's command. */
     switch (req.r_command) {

     case WMS_COMMAND_TEST:
      /* Simple echo command to test the connection.
       * The echo portion is always copied, as it is also used as token. */
      break;

     case WMS_COMMAND_MKWIN: {
      Window *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(new_window);
      Display *EXCEPT_VAR disp = &default_display; /* XXX: option? */

      atomic_rwlock_write(&disp->d_lock);
      TRY {
       /* Generate a automatic window position. */
       if (req.r_mkwin.mw_xmin == WM_WINDOW_AUTOPOS &&
           req.r_mkwin.mw_ymin == WM_WINDOW_AUTOPOS)
           req.r_mkwin.mw_xmin = (disp->d_sizex - req.r_mkwin.mw_xsiz) / 2,
           req.r_mkwin.mw_ymin = (disp->d_sizey - req.r_mkwin.mw_ysiz) / 2;

       /* Construct a new window. */
       new_window = Window_CreateUnlocked(disp,
                                          req.r_mkwin.mw_xmin,
                                          req.r_mkwin.mw_ymin,
                                          req.r_mkwin.mw_xsiz,
                                          req.r_mkwin.mw_ysiz,
                                          req.r_mkwin.mw_state,
                                          client_fd);
       TRY {
        /* Assign the window an ID and register it in the window map. */
        new_window->w_id = WindowMap_MakeID(&windowmap);
        WindowMap_Insert(&windowmap,new_window);
       } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
        Window_DestroyUnlocked(new_window);
        error_rethrow();
       }
       /* Fill in response information */
       resp.r_mkwin.w_posx   = new_window->w_posx;
       resp.r_mkwin.w_posy   = new_window->w_posy;
       resp.r_mkwin.w_sizex  = new_window->w_sizex;
       resp.r_mkwin.w_sizey  = new_window->w_sizey;
       resp.r_mkwin.w_stride = new_window->w_stride;
       resp.r_mkwin.w_bpp    = disp->d_bpp;
       resp.r_mkwin.w_state  = new_window->w_state;
      } FINALLY {
       atomic_rwlock_endwrite(&disp->d_lock);
      }
      resp.r_answer     = WMS_RESPONSE_MKWIN_OK;
      resp.r_mkwin.w_id = new_window->w_id;

      {
       struct msghdr msg;
       struct iovec iov[1];
       union {
        struct cmsghdr hdr;
        byte_t buf[CMSG_SPACE(sizeof(fd_t))];
       } send_buffer;
       iov[0].iov_base    = &resp;
       iov[0].iov_len     = sizeof(struct wms_response);
       msg.msg_name       = NULL;
       msg.msg_namelen    = 0;
       msg.msg_iov        = iov;
       msg.msg_iovlen     = 1;
       msg.msg_control    = send_buffer.buf;
       msg.msg_controllen = sizeof(send_buffer);
       msg.msg_flags      = 0;

       /* Include the window's screen buffer in the
        * form of an mmap()-able file descriptor. */
       send_buffer.hdr.cmsg_len   = CMSG_SPACE(sizeof(fd_t));
       send_buffer.hdr.cmsg_level = SOL_SOCKET;
       send_buffer.hdr.cmsg_type  = SCM_RIGHTS;
       *(fd_t *)CMSG_DATA(&send_buffer.hdr) = new_window->w_screenfd;

       /* Send the response back. */
       if (Xsendmsg(client_fd,&msg,0) != sizeof(struct wms_response))
           goto disconnect; /* Disconnect */
      }

      /* Don't send an ACK after we've already send the window-created message. */
      req.r_flags |= WMS_COMMAND_FNOACK;
     } break;

     {
      Window *EXCEPT_VAR win;
     case WMS_COMMAND_RMWIN:
      win = WindowMap_Lookup(&windowmap,
                              req.r_rmwin.rw_winid);
      if (!win) {
       if (req.r_flags & WMS_COMMAND_FNOACK)
           break;
       error_throw(E_INVALID_ARGUMENT);
      }
      WindowMap_Remove(&windowmap,win);
      TRY {
       Display *EXCEPT_VAR d = win->w_display;
       atomic_rwlock_write(&d->d_lock);
       TRY {
        /* Try to destroy the window. */
        Window_DestroyUnlocked(win);
       } FINALLY {
        atomic_rwlock_endwrite(&d->d_lock);
       }
      } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
       /* If that failed, re-insert the window. */
       WindowMap_Insert(pwindowmap,win);
       error_rethrow();
      }
     } break;

     {
      Window *win;
      Display *EXCEPT_VAR d;
     case WMS_COMMAND_MVWIN:
      win = WindowMap_Lookup(&windowmap,
                              req.r_mvwin.mw_winid);
      if (!win) {
       if (req.r_flags & WMS_COMMAND_FNOACK)
           break;
       error_throw(E_INVALID_ARGUMENT);
      }
      d = win->w_display;
      atomic_rwlock_read(&d->d_lock);
      TRY {
       int old_x = win->w_posx;
       int old_y = win->w_posy;
       Window_MoveUnlocked(win,
                           req.r_mvwin.mw_newx,
                           req.r_mvwin.mw_newy);
       resp.r_answer                        = WMS_RESPONSE_EVENT;
       resp.r_event.e_window.w_event        = WM_WINDOWEVENT_MOVED;
       resp.r_event.e_window.w_winid        = win->w_id;
       resp.r_event.e_window.w_moved.m_oldx = old_x;
       resp.r_event.e_window.w_moved.m_oldy = old_y;
       resp.r_event.e_window.w_moved.m_newx = win->w_posx;
       resp.r_event.e_window.w_moved.m_newy = win->w_posy;
      } FINALLY {
       atomic_rwlock_endread(&d->d_lock);
      }
     } break;

     case WMS_COMMAND_RZWIN:
      /* TODO */
      error_throw(E_NOT_IMPLEMENTED);
      break;

     {
      Window *win;
      Display *EXCEPT_VAR d;
     case WMS_COMMAND_DRAWALL:
     case WMS_COMMAND_DRAWONE:
      win = WindowMap_Lookup(&windowmap,
                              req.r_drawall.dw_winid);
      if (!win) {
       if (req.r_flags & WMS_COMMAND_FNOACK)
           break;
       error_throw(E_INVALID_ARGUMENT);
      }
      d = win->w_display;
      atomic_rwlock_read(&d->d_lock);
      TRY {
       /* Try to draw the window. */
       if (req.r_command == WMS_COMMAND_DRAWALL)
        Window_DrawUnlocked(win);
       else {
        STATIC_ASSERT(offsetof(struct rect,r_xmin) ==
                     (offsetof(struct wms_request,r_drawone.dw_xmin)-
                      offsetof(struct wms_request,r_drawone.dw_xmin)));
        STATIC_ASSERT(offsetof(struct rect,r_ymin) ==
                     (offsetof(struct wms_request,r_drawone.dw_ymin)-
                      offsetof(struct wms_request,r_drawone.dw_xmin)));
        STATIC_ASSERT(offsetof(struct rect,r_xsiz) ==
                     (offsetof(struct wms_request,r_drawone.dw_xsiz)-
                      offsetof(struct wms_request,r_drawone.dw_xmin)));
        STATIC_ASSERT(offsetof(struct rect,r_ysiz) ==
                     (offsetof(struct wms_request,r_drawone.dw_ysiz)-
                      offsetof(struct wms_request,r_drawone.dw_xmin)));
        Window_DrawRectUnlocked(win,*(struct rect *)&req.r_drawone.dw_xmin);
       }
      } FINALLY {
       atomic_rwlock_endread(&d->d_lock);
      }
      if (req.r_flags & WMS_COMMAND_DRAW_FVSYNC) {
       /* TODO: wait for a VSYNC on `d' */
      }
     } break;

     {
      Window *win;
      Display *EXCEPT_VAR d;
     case WMS_COMMAND_CHWIN:
      win = WindowMap_Lookup(&windowmap,
                              req.r_chwin.cw_winid);
      if (!win) {
       if (req.r_flags & WMS_COMMAND_FNOACK)
           break;
       error_throw(E_INVALID_ARGUMENT);
      }
      d = win->w_display;
      if (req.r_flags & WMS_COMMAND_FNOACK)
          req.r_echo = 0; /* Only send a response if the window actually changed state,
                           * and if this is done, send it as a generic event, rather than
                           * as a targeted response. */
      atomic_rwlock_read(&d->d_lock);
      TRY {
       /* Try to change the window's state. */
       Window_ChangeStateUnlocked(win,
                                  req.r_chwin.cw_mask,
                                  req.r_chwin.cw_flag,
                                  req.r_echo);
      } FINALLY {
       atomic_rwlock_endread(&d->d_lock);
      }
      /* The ChangeState() will have already send an event, informing the client. */
      req.r_flags |= WMS_COMMAND_FNOACK;
     } break;

     {
      Window *win;
      Display *EXCEPT_VAR d;
     case WMS_COMMAND_TOFRONT:
      win = WindowMap_Lookup(&windowmap,
                              req.r_tofront.fw_winid);
      if (!win) {
       if (req.r_flags & WMS_COMMAND_FNOACK)
           break;
       error_throw(E_INVALID_ARGUMENT);
      }
      d = win->w_display;
      atomic_rwlock_read(&d->d_lock);
      TRY {
       if (Window_BringToFront(win))
           req;
      } FINALLY {
       atomic_rwlock_endread(&d->d_lock);
      }
     } break;

     default:
      /* Unknown command. */
      resp.r_answer = WMS_RESPONSE_BADCMD;
      resp.r_badcmd.b_command = req.r_command;
      break;
     }

    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     error_printf("Command failed\n");
     error_handled();
     /* Propagate exceptions back to the client. */
     memset(&resp,0,sizeof(struct wms_response));
     resp.r_echo             = req.r_echo;
     resp.r_answer           = WMS_RESPONSE_FAILED;
     resp.r_failed.f_command = req.r_command;
     memcpy(&resp.r_failed.f_except,
            &error_info()->e_error,
             sizeof(struct exception_data));
    }
   
    if (!(req.r_flags & WMS_COMMAND_FNOACK)) {
     /* Send the response back. */
     if (Xsend(client_fd,&resp,sizeof(struct wms_response),0) != sizeof(struct wms_response))
         break; /* Disconnect */
    }
   }
disconnect:
   ;
  } FINALLY {
   WindowMap_Fini(pwindowmap);
  }
 } FINALLY {
  close(client_fd);
 }
 syslog(LOG_DEBUG,"[WMS] Client gracefully disconnected\n");
 return 0;
}


INTERN void WMCALL AcceptConnection(fd_t client_fd) {
 /* Spawn a new thread for dealing with the client. */
 clone(&ClientMain,
        CLONE_CHILDSTACK_AUTO,
        CLONE_NEW_THREAD,
       (void *)(uintptr_t)(intptr_t)client_fd);
}


DECL_END

#endif /* !GUARD_APPS_WMS_SERVER_C */
