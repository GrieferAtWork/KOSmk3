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
#ifndef GUARD_APPS_WMS_EVENT_C
#define GUARD_APPS_WMS_EVENT_C 1
#define _KOS_SOURCE 1
#define _EXCEPT_SOURCE 1
#define __BUILDING_WMSERVER 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <wm/api.h>
#include <wm/server.h>
#include <kos/keyboard-ioctl.h>
#include <kos/mouse-ioctl.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <malloc.h>

#include "render.h"
#include "bind.h"
#include "window.h"
#include "display.h"
#include "event.h"

DECL_BEGIN

#ifdef CONFIG_COPYRECT_DO_OUTLINE
PRIVATE bool redraw_full_on_move = false;
#endif

INTERN int KeyboardRelayThread(void *arg) {
 Display *EXCEPT_VAR d = &default_display; /* XXX: Multiple display? */
 struct keyboard_keymap *keymap;
 keyboard_state_t keystate = 0;
 /* Map the keyboard keymap into memory. */
 keymap = (struct keyboard_keymap *)Xmmap(NULL,sizeof(struct keyboard_keymap),
                                          PROT_READ,MAP_SHARED,wms_keyboard,
                                          KEYBOARD_KEYMAP_OFFSET);
 TRY {
  for (;;) {
   struct wms_response resp;
   memset(&resp,0,sizeof(resp));
   if (!Xread(wms_keyboard,&resp.r_event.e_key.k_key,sizeof(resp.r_event.e_key.k_key)))
        error_throw(E_INVALID_ARGUMENT); /* EOF? */

   switch (resp.r_event.e_key.k_key) {

#define MIRROR(key,state) \
   case KEYDOWN(key): KEYBOARD_STATE_FADD(keystate,state); break; \
   case KEYUP  (key): KEYBOARD_STATE_FDEL(keystate,state); break;
#define TOGGLE(key,state) \
   case KEYDOWN(key): KEYBOARD_STATE_FXOR(keystate,state); break;
    /* Update the keyboard state. */
   MIRROR(KEY_LSHIFT,KEYBOARD_STATE_FLSHIFT)
   MIRROR(KEY_RSHIFT,KEYBOARD_STATE_FRSHIFT)
   MIRROR(KEY_LCTRL,KEYBOARD_STATE_FLCTRL)
   MIRROR(KEY_RCTRL,KEYBOARD_STATE_FRCTRL)
   MIRROR(KEY_LGUI,KEYBOARD_STATE_FLGUI)
   MIRROR(KEY_RGUI,KEYBOARD_STATE_FRGUI)
   MIRROR(KEY_LALT,KEYBOARD_STATE_FLALT)
   MIRROR(KEY_RALT,KEYBOARD_STATE_FRALT)
   MIRROR(KEY_ESC,KEYBOARD_STATE_FESC)
   MIRROR(KEY_TAB,KEYBOARD_STATE_FTAB)
   MIRROR(KEY_SPACE,KEYBOARD_STATE_FSPACE)
   MIRROR(KEY_APPS,KEYBOARD_STATE_FAPPS)
   MIRROR(KEY_ENTER,KEYBOARD_STATE_FENTER)
   MIRROR(KEY_KP_ENTER,KEYBOARD_STATE_FKP_ENTER)
   MIRROR(KEY_INSERT,KEYBOARD_STATE_FOVERRIDE)

   TOGGLE(KEY_CAPSLOCK,KEYBOARD_STATE_FCAPSLOCK)
   TOGGLE(KEY_NUMLOCK,KEYBOARD_STATE_FNUMLOCK)
   TOGGLE(KEY_SCROLLLOCK,KEYBOARD_STATE_FSCROLLLOCK)
#undef TOGGLE
#undef MIRROR


#if 1 /* Debug functionality. */
   case KEY_F5:
    atomic_rwlock_read(&d->d_lock);
    TRY {
     Display_Redraw(d);
    } FINALLY {
     atomic_rwlock_endread(&d->d_lock);
    }
    break;

#ifdef CONFIG_COPYRECT_DO_OUTLINE
   case KEY_F6:
    copyrect_do_outline = !copyrect_do_outline;
    break;
   case KEY_F7:
    redraw_full_on_move = !redraw_full_on_move;
    break;
#endif
#endif

   default: break;
   }

   /* Setup a key event. */
   resp.r_answer              = WMS_RESPONSE_EVENT;
   resp.r_event.e_key.k_type  = WM_EVENT_KEY;
   resp.r_event.e_key.k_state = keystate;
   /* Translate the key into its character representation. */
   if (resp.r_event.e_key.k_key < 256) {
    if (KEYBOARD_STATE_FISALTGR(keystate)) {
     resp.r_event.e_key.k_text = keymap->km_altgr[resp.r_event.e_key.k_key];
     if (!resp.r_event.e_key.k_text.kmc_utf8[0]) goto check_other;
    } else check_other: if (KEYBOARD_STATE_FISCAPS(keystate))
     resp.r_event.e_key.k_text = keymap->km_shift[resp.r_event.e_key.k_key];
    else {
     resp.r_event.e_key.k_text = keymap->km_press[resp.r_event.e_key.k_key];
    }
   }

   /* Send the event to the focused window. */
   atomic_rwlock_read(&d->d_lock);
   TRY {
    if (d->d_focus)
        Window_SendMessage(d->d_focus,&resp);
   } FINALLY {
    atomic_rwlock_endread(&d->d_lock);
   }
  }
 } FINALLY {
  munmap(keymap,sizeof(struct keyboard_keymap));
 }
 return 0;
}

#define CURSOR_SIZE_X  8
#define CURSOR_SIZE_Y  8

#define B(x) \
 ((0x##x & 0x00000001) |\
  (0x##x & 0x00000010) >> 3 |\
  (0x##x & 0x00000100) >> 6 |\
  (0x##x & 0x00001000) >> 9 |\
  (0x##x & 0x00010000) >> 12 |\
  (0x##x & 0x00100000) >> 15 |\
  (0x##x & 0x01000000) >> 18 |\
  (0x##x & 0x10000000) >> 21)

PRIVATE u8 const cursor_xor[8] = {
    B(00000001),
    B(00000011),
    B(00000111),
    B(00001111),
    B(00011111),
    B(00111111),
    B(00001111),
    B(00001001)
};

INTERN void WMCALL
xor_display_pixel(Display *__restrict self,
                  unsigned int x,
                  unsigned int y) {
 byte_t *ptr;
 if (x >= self->d_sizex ||
     y >= self->d_sizey)
     return;
 ptr  = self->d_screen;
 ptr += y * self->d_stride; /* XXX: Use shifts here? */
 x   *= self->d_bpp;
 ptr += x/8;
 switch (self->d_bpp) {
 case 32:
  *(u32 *)ptr ^= 0xffffffff;
  break;
 case 16:
  *(u16 *)ptr ^= 0xffff;
  break;
 case 8:
  *(u8 *)ptr &= 0x3f;
  *(u8 *)ptr ^= 0x38;
  break;
 case 4:
  x %= 8;
  assert(x == 0 || x == 4);
  *(u8 *)ptr ^= (0xf << x);
  break;
 case 2:
  x %= 8;
  assert(x == 0 || x == 2 || x == 4 || x == 6);
  *(u8 *)ptr ^= (0x3 << x);
  break;
 case 1:
  x %= 8;
  *(u8 *)ptr ^= (0x1 << (7 - x));
  break;
 default: assert(0);
 }
}


PRIVATE void WMCALL
draw_cursor(Display *__restrict d,
            unsigned int x, unsigned int y) {
 unsigned int i,j;
 for (j = 0; j < 8; ++j) {
  u8 line = cursor_xor[j];
  for (i = 0; i < 8; ++i) {
   if (line & (1 << i))
       xor_display_pixel(d,x+i,y+j);
  }
 }
}

PRIVATE void WMCALL
undraw_cursor(Display *__restrict d, unsigned int x, unsigned int y) {
 struct rect r;
 r.r_xmin = x;
 r.r_ymin = y;
 r.r_xsiz = CURSOR_SIZE_X;
 r.r_ysiz = CURSOR_SIZE_Y;
#ifdef CONFIG_COPYRECT_DO_OUTLINE
 if (copyrect_do_outline) {
  copyrect_do_outline = false;
  Display_RedrawRect(d,r);
  copyrect_do_outline = true;
 } else
#endif
 {
  Display_RedrawRect(d,r);
 }
}


INTERN int MouseRelayThread(void *arg) {
 Display *EXCEPT_VAR d = &default_display; /* XXX: Multiple display? */
 WEAK REF Window *EXCEPT_VAR hover_window = NULL;
 Window *new_hover_window;
 unsigned int mouse_display_x = d->d_sizex / 2;
 unsigned int mouse_display_y = d->d_sizey / 2;
 unsigned int new_mouse_display_x;
 unsigned int new_mouse_display_y;
 struct wms_response resp;
 draw_cursor(d,mouse_display_x,mouse_display_y);
 TRY {
  for (;;) {
   memset(&resp,0,sizeof(resp));
   resp.r_answer               = WMS_RESPONSE_EVENT;
   resp.r_event.e_mouse.m_type = WM_EVENT_MOUSE;
   if (!Xread(wms_mouse,&resp.r_event.e_mouse.m_packet,
              sizeof(struct mouse_packet)))
       error_throw(E_INVALID_ARGUMENT); /* EOF? */
   /* Invert movement in Y to account for the fact that y=0 is at the top. */
   resp.r_event.e_mouse.m_packet.mp_rely = -resp.r_event.e_mouse.m_packet.mp_rely;
   new_mouse_display_x = mouse_display_x + (int)resp.r_event.e_mouse.m_packet.mp_relx;
   new_mouse_display_y = mouse_display_y + (int)resp.r_event.e_mouse.m_packet.mp_rely;
   if ((int)new_mouse_display_x < 0) new_mouse_display_x = 0;
   else if (new_mouse_display_x >= d->d_sizex)
       new_mouse_display_x = d->d_sizex-1;
   if ((int)new_mouse_display_y < 0) new_mouse_display_y = 0;
   else if (new_mouse_display_y >= d->d_sizey)
       new_mouse_display_y = d->d_sizey-1;
   resp.r_event.e_mouse.m_clrelx = new_mouse_display_x - mouse_display_x;
   resp.r_event.e_mouse.m_clrely = new_mouse_display_y - mouse_display_y;
   atomic_rwlock_read(&d->d_lock);
   TRY {
    new_hover_window = Display_WindowAt(d,
                                        new_mouse_display_x,
                                        new_mouse_display_y);
    if (new_hover_window != hover_window) {
     if (hover_window) {
      if (!(hover_window->w_state & WM_WINDOW_STATE_FDESTROYED)) {
       /* Mouse-over window changed (Send an event to the old window that explains this) */
       struct wms_response leave;
       memcpy(&leave,&resp,sizeof(resp));
       leave.r_event.e_mouse.m_flag |= WM_EVENT_MOUSE_FLEAVE; /* Leave the old window. */
       /* Hide mouse key state information from the old window. */
       leave.r_event.e_mouse.m_packet.mp_keys ^= leave.r_event.e_mouse.m_packet.mp_chng;
       leave.r_event.e_mouse.m_packet.mp_chng  = 0;
       leave.r_event.e_mouse.m_packet.mp_relz  = 0;
       leave.r_event.e_mouse.m_winid  = hover_window->w_id;
       leave.r_event.e_mouse.m_mousex = new_mouse_display_x - hover_window->w_posx;
       leave.r_event.e_mouse.m_mousey = new_mouse_display_y - hover_window->w_posy;
       Window_SendMessage(hover_window,&leave);
      }
      assertf(hover_window->w_weakcnt >= 2 ||
             (hover_window->w_weakcnt == 1 &&
             (hover_window->w_state & WM_WINDOW_STATE_FDESTROYED)),
              "hover_window->w_weakcnt = %u\n"
              "hover_window->w_state   = %x\n"
              ,hover_window->w_weakcnt
              ,hover_window->w_state);
      Window_WeakDecref(hover_window);
     }
     if (new_hover_window)
         Window_WeakIncref(new_hover_window);
     hover_window = new_hover_window;
     resp.r_event.e_mouse.m_flag |= WM_EVENT_MOUSE_FENTER; /* Enter the new window. */
    }
    if (hover_window) {
     /* Send the mouse event to the window being hovered. */
     resp.r_event.e_mouse.m_winid  = hover_window->w_id;
     resp.r_event.e_mouse.m_mousex = new_mouse_display_x - hover_window->w_posx;
     resp.r_event.e_mouse.m_mousey = new_mouse_display_y - hover_window->w_posy;
     if (resp.r_event.e_mouse.m_packet.mp_keys &
         resp.r_event.e_mouse.m_packet.mp_chng) {
      /* A mouse button was pressed.
       * -> Bring the window into the front and into focus. */
      Window_BringToFront(hover_window);
      /* Set the keyboard focus towards this window. */
      Window_ChangeStateUnlocked(hover_window,~0,WM_WINDOW_STATE_FFOCUSED,0);
     }
     /* Send the event to the window. */
     Window_SendMessage(hover_window,&resp);
    }
    if (mouse_display_x != new_mouse_display_x ||
        mouse_display_y != new_mouse_display_y) {
     /* Re-draw the cursor.
      * XXX: This is really badly designed when compared to the draw-on-demand system
      *      originally designed to prevent the need of having to draw over already
      *      rendered portions of the screen... */
     undraw_cursor(d,mouse_display_x,mouse_display_y);
     draw_cursor(d,new_mouse_display_x,new_mouse_display_y);
     mouse_display_x = new_mouse_display_x;
     mouse_display_y = new_mouse_display_y;
    }
   } FINALLY {
    atomic_rwlock_endread(&d->d_lock);
   }
#if 1
   if ((resp.r_event.e_mouse.m_packet.mp_keys & MOUSE_BUTTON_FLEFT) &&
       (hover_window != NULL)) {
    atomic_rwlock_write(&d->d_lock);
    TRY {
     if (!(hover_window->w_state & WM_WINDOW_STATE_FDESTROYED)) {
      Window_MoveUnlocked(hover_window,
                          hover_window->w_posx + (int)resp.r_event.e_mouse.m_clrelx,
                          hover_window->w_posy + (int)resp.r_event.e_mouse.m_clrely);
#ifdef CONFIG_COPYRECT_DO_OUTLINE
      /* Doing a full redraw here allow one to better see the redraw rects. */
      if (redraw_full_on_move)
          Display_Redraw(d);
#endif
     }
    } FINALLY {
     atomic_rwlock_endwrite(&d->d_lock);
    }
   }
#endif
  }
 } FINALLY {
  if (hover_window)
      Window_WeakDecref(hover_window);
 }
 return 0;
}


DECL_END

#endif /* !GUARD_APPS_WMS_EVENT_C */
