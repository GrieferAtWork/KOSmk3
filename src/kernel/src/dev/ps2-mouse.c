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
#ifndef GUARD_KERNEL_SRC_DEV_PS2_MOUSE_C
#define GUARD_KERNEL_SRC_DEV_PS2_MOUSE_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <dev/ps2.h>
#include <dev/ps2-mouse.h>
#include <dev/ps2-program.h>
#include <kos/mouse-ioctl.h>
#include <hybrid/section.h>
#include <except.h>
#include <kos/kdev_t.h>
#include <stdio.h>

#ifdef CONFIG_HAVE_DEV_PS2
DECL_BEGIN

typedef struct ps2_mouse Mouse;

/* The max size of a PS/2 generated mouse packet, when encoded using mouse actions:
 *    +5    -- for up to 5 `MOUSE_ACTION_FDOWN' or `MOUSE_ACTION_FUP'
 *    +6    -- for up to 3 MOUSE_ACTION_FMOVY
 *    +6    -- for up to 3 MOUSE_ACTION_FMOVXY
 *    +9    -- for MOUSE_ACTION_FPCKT64
 */
#define PS2_MOUSE_PACKET_MAXSIZE  32 /* 26 */

struct ps2_mouse_packet {
#define MOUSE_PACKET_FYO 0x80 /* Y-Axis Overflow */
#define MOUSE_PACKET_FXO 0x40 /* X-Axis Overflow */
#define MOUSE_PACKET_FYS 0x20 /* Y-Axis Sign Bit (9-Bit Y-Axis Relative Offset) */
#define MOUSE_PACKET_FXS 0x10 /* X-Axis Sign Bit (9-Bit X-Axis Relative Offset) */
#define MOUSE_PACKET_FAO 0x08 /* Always One */
#define MOUSE_PACKET_FBM 0x04 /* Button Middle (Normally Off = 0) */
#define MOUSE_PACKET_FBR 0x02 /* Button Right (Normally Off = 0) */
#define MOUSE_PACKET_FBL 0x01 /* Button Left (Normally Off = 0) */
    u8 flags;                 /* Set of `MOUSE_PACKET_F*' */
    u8 xm;                    /* X-Axis Movement Value  */
    u8 ym;                    /* Y-Axis Movement Value  */
};

STATIC_ASSERT(MOUSE_BUTTON_FLEFT   == MOUSE_PACKET_FBL);
STATIC_ASSERT(MOUSE_BUTTON_FRIGHT  == MOUSE_PACKET_FBR);
STATIC_ASSERT(MOUSE_BUTTON_FMIDDLE == MOUSE_PACKET_FBM);

PRIVATE ASYNCSAFE void KCALL
Mouse_Interrupt(Mouse *__restrict self,
                struct ps2_mouse_packet *__restrict pck) {
 byte_t buf[PS2_MOUSE_PACKET_MAXSIZE];
 byte_t *ptr = buf; s16 relx,rely;

 /* Check for button changes. */
 if ((self->pm_old_buttons & MOUSE_BUTTON_FLEFT) != (pck->flags & MOUSE_PACKET_FBL))
      ptr = mouse_encode_updown(ptr,0,!!(pck->flags & MOUSE_PACKET_FBL));
 if ((self->pm_old_buttons & MOUSE_BUTTON_FRIGHT) != (pck->flags & MOUSE_PACKET_FBR))
      ptr = mouse_encode_updown(ptr,1,!!(pck->flags & MOUSE_PACKET_FBR));
 if ((self->pm_old_buttons & MOUSE_BUTTON_FMIDDLE) != (pck->flags & MOUSE_PACKET_FBM))
      ptr = mouse_encode_updown(ptr,2,!!(pck->flags & MOUSE_PACKET_FBM));
 /* Save the new button state. */
 self->pm_old_buttons = pck->flags & (MOUSE_PACKET_FBL|MOUSE_PACKET_FBR|MOUSE_PACKET_FBM);

 /* Extract and encode relative mouse motion. */
 relx = pck->xm;
 rely = pck->ym;
 if (pck->flags & MOUSE_PACKET_FXS) relx = -relx;
 if (pck->flags & MOUSE_PACKET_FYS) rely = -rely;
 ptr = mouse_encode_movxy(ptr,relx,rely);

 /* Encode the current system time. */
 ptr = mouse_encode_pack(ptr,&self->pm_mouse);

 /* Transmit mouse action data. */
 mouse_action(&self->pm_mouse,
               buf,
              (u16)(ptr-buf));
}


PRIVATE void KCALL
Mouse_Fini(Mouse *__restrict self) {
 /* Delete the interrupt callback. */
 ps2_delete_callback(self->pm_port,
                    (ps2_callback_t)&Mouse_Interrupt,
                     self);
}

PRIVATE void KCALL
Mouse_EnableReports(Mouse *__restrict self) {
 if (!ps2_runprogram(self->pm_port,NULL,NULL,
      PS2_PROGRAM(
      ps2_send  PS2_MOUSE_FENABLE_REPORTING;
      ps2_wait  PS2_ACK;
      ps2_stop;
      )))
      error_throw(E_IOERROR);
}

PRIVATE void KCALL
Mouse_DisableReports(Mouse *__restrict self) {
 if (!ps2_runprogram(self->pm_port,NULL,NULL,
      PS2_PROGRAM(
      ps2_send  PS2_MOUSE_FDISABLE_REPORTING;
      ps2_wait  PS2_ACK;
      ps2_stop;
      )))
      error_throw(E_IOERROR);
}

PRIVATE struct mouse_ops MouseOps = {
    .mo_fini            = (void(KCALL *)(struct mouse *__restrict))&Mouse_Fini,
    .mo_enable_reports  = (void(KCALL *)(struct mouse *__restrict self))&Mouse_EnableReports,
    .mo_disable_reports = (void(KCALL *)(struct mouse *__restrict self))&Mouse_DisableReports,
};



INTERN ATTR_FREETEXT void KCALL
ps2_register_mouse(u8 port, u8 type) {
 Mouse *m;
 if (!ps2_runprogram(port,NULL,NULL,
      PS2_PROGRAM(
      ps2_send  PS2_MOUSE_FRESET;
      ps2_wait  PS2_ACK;
      ps2_wait  0xaa;
      ps2_wait  0x00; /* ??? (What the hell is this?) (QEMU send this, but I don't
                       *      know what it means. - The keyboard didn't have this...)
                       * OK. I found the relevant linux source:
                       *   /drivers/input/mouse/psmouse-base.c:psmouse_reset
                       * In there, it does practically the same as I do here, naming this
                       * 0x00 as `PSMOUSE_RET_ID', a macro that is (you guessed it: 0x00)
                       * While that doesn't really answer the question above, it does seem
                       * to suggest that the reset command always returns { 0xfe, 0xaa, 0x00 }
                       * Though I do wonder what linux's sources are for that name... (hmm... `RET_ID')
                       */
      ps2_send  PS2_MOUSE_FSETDEFAULT;
      ps2_wait  PS2_ACK;
      ps2_send  PS2_MOUSE_FENABLE_REPORTING;
      ps2_wait  PS2_ACK;
      ps2_stop;
      )))
      return;
 m = MOUSE_ALLOC(Mouse,&MouseOps);
 TRY {
  /* Initialize the device number and name fields. */
  m->pm_mouse.m_dev.c_device.d_devno = MKDEV(MAJOR(DV_PS2_KEYBOARD),
                                             MINOR(DV_PS2_MOUSE)+port);
  sprintf(m->pm_mouse.m_dev.c_device.d_namebuf,"ps2_mouse%c",'a'+port);
  /* Setup port and initial keyboard state. */
  m->pm_port = port;
  m->pm_type = type;
  /* The code about configured the mouse to already enable reporting. */
  m->pm_mouse.m_flags |= MOUSE_FREPORTING;
  /* Hook the PS/2 interrupt callback for the keyboard. */
  ps2_install_callback(port,
                      (ps2_callback_t)&Mouse_Interrupt,
                       m);
  /* Register the device. */
  asserte(register_device(&m->pm_mouse.m_dev.c_device));
 } FINALLY {
  mouse_decref(&m->pm_mouse);
 }
 /* It's a mouse! (remember that) */
 ps2_port_device[port] = PS2_PORT_DEVICE_FMOUSE;
 ps2_packet_size[port] = 3;
}



DECL_END
#endif /* CONFIG_HAVE_DEV_PS2 */

#endif /* !GUARD_KERNEL_SRC_DEV_PS2_MOUSE_C */
