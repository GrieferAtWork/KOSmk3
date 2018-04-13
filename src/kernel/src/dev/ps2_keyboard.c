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
#ifndef GUARD_KERNEL_SRC_DEV_PS2_KEYBOARD_C
#define GUARD_KERNEL_SRC_DEV_PS2_KEYBOARD_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kernel/debug.h>
#include <kernel/malloc.h>
#include <kos/kdev_t.h>
#include <dev/ps2.h>
#include <dev/ps2-program.h>
#include <dev/ps2_keyboard.h>
#include <kos/keyboard.h>
#include <dev/keyboard.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <except.h>

#ifdef CONFIG_HAVE_DEV_PS2

#include "ps2_keymaps.h"

DECL_BEGIN

#define STATE_INITIAL(scanset) ((scanset) << 6)
#define STATE1                      0x40
#define STATE1_E0                   0x41
#define STATE1_E0_2A                0x42
#define STATE1_E0_2A_E0             0x43
#define STATE1_E0_B7                0x44
#define STATE1_E0_B7_E0             0x45
#define STATE1_E1                   0x46
#define STATE1_E1_1D                0x47
#define STATE1_E1_1D_45             0x48
#define STATE1_E1_1D_45_E1          0x49
#define STATE1_E1_1D_45_E1_9D       0x4a

#define STATE2                      0x80
#define STATE2_F0                   0x81
#define STATE2_E0                   0x82
#define STATE2_E0_F0                0x83
#define STATE2_E0_12                0x84
#define STATE2_E0_12_E0             0x85
#define STATE2_E0_F0_7C             0x86
#define STATE2_E0_F0_7C_E0          0x87
#define STATE2_E0_F0_7C_E0_F0       0x88
#define STATE2_E1                   0x89
#define STATE2_E1_14                0x8a
#define STATE2_E1_14_77             0x8b
#define STATE2_E1_14_77_E1          0x8c
#define STATE2_E1_14_77_E1_F0       0x8d
#define STATE2_E1_14_77_E1_F0_14    0x8e
#define STATE2_E1_14_77_E1_F0_14_F0 0x8f

#define STATE3                      0xc0
#define STATE3_F0                   0xc1


typedef struct ps2_keyboard Keyboard;
struct ps2_keyboard {
     struct keyboard p_keyboard; /* The underlying keyboard. */
     u8              p_port;     /* The PS/2 port of the keyboard (One of `PS2_PORT*') */

     u8              p_state;    /* The current state machine operations state. */
};

PRIVATE ASYNCSAFE void KCALL
Keyboard_Putc(Keyboard *__restrict self, keyboard_key_t key) {
 debug_printf("KEY: %x\n",key);
 if ((key & ~(KEY_PRESSED|KEY_RELEASED)) == KEY_UNKNOWN) {
  debug_printf("[PS/2] Unknown PS/2 keyboard key\n");
  return;
 }
#if 1 /* For debugging... */
 if (key == KEYUP(KEY_F12)) {
  /* release F12 */
  asm("int3");
  debug_printf("DONE_BREAK\n");
 }
#endif
 /* Add the key to the keyboard's word buffer. */
 wordbuffer_putword(&self->p_keyboard.k_buf,
                     key);
}

PRIVATE ASYNCSAFE void KCALL
Keyboard_Interrupt(Keyboard *__restrict self,
                   byte_t ps2_byte) {
 keyboard_key_t key;
again:
 switch (self->p_state) {

  /* Scanset #1 */
 case STATE1:
  if (ps2_byte == 0xe0) { self->p_state = STATE1_E0; goto done; }
  if (ps2_byte == 0xe1) { self->p_state = STATE1_E1; goto done; }
  key = KEYMAP_GET_PS2_SCANSET1(ps2_byte);
  break;
 case STATE1_E0:
  if (ps2_byte == 0x2a) { self->p_state = STATE1_E0_2A; goto done; }
  if (ps2_byte == 0xb7) { self->p_state = STATE1_E0_B7; goto done; }
  key = KEYMAP_GET_PS2_SCANSET1_E0(ps2_byte);
  self->p_state = STATE1;
  break;
 case STATE1_E0_2A:
  if (ps2_byte == 0xe0) { self->p_state = STATE1_E0_2A_E0; goto done; }
  /* Unwind. */
  Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1_E0(0x2a));
  self->p_state = STATE1;
  goto again;
 case STATE1_E0_2A_E0:
  if (ps2_byte == 0x37) {
   self->p_state = STATE1;
   key = KEY_PRESSED|KEY_PRINTSCREEN;
  } else {
   /* Unwind. */
   Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1_E0(0x2a));
   self->p_state = STATE1_E0;
   goto again;
  }
  break;
 case STATE1_E0_B7:
  if (ps2_byte == 0xe0) { self->p_state = STATE1_E0_B7_E0; goto done; }
  /* Unwind. */
  Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1_E0(0xb7));
  self->p_state = STATE1;
  goto again;
 case STATE1_E0_B7_E0:
  if (ps2_byte == 0xaa) {
   self->p_state = STATE1;
   key = KEY_RELEASED|KEY_PRINTSCREEN;
  } else {
   /* Unwind. */
   Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1_E0(0xb7));
   self->p_state = STATE1_E0;
   goto again;
  }
  break;
 case STATE1_E1:
  if (ps2_byte == 0x1d) { self->p_state = STATE1_E1_1D; goto done; }
  Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1(0xe1));
  self->p_state = STATE1;
  goto again;
 case STATE1_E1_1D:
  if (ps2_byte == 0x45) { self->p_state = STATE1_E1_1D_45; goto done; }
  Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1(0xe1));
  Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1(0x1d));
  self->p_state = STATE1;
  goto again;
 case STATE1_E1_1D_45:
  if (ps2_byte == 0xe1) { self->p_state = STATE1_E1_1D_45_E1; goto done; }
  Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1(0xe1));
  Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1(0x1d));
  Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1(0x45));
  self->p_state = STATE1;
  goto again;
 case STATE1_E1_1D_45_E1:
  if (ps2_byte == 0x9d) { self->p_state = STATE1_E1_1D_45_E1_9D; goto done; }
  Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1(0xe1));
  Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1(0x1d));
  Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1(0x45));
  self->p_state = STATE1_E1;
  goto again;
 case STATE1_E1_1D_45_E1_9D:
  self->p_state = STATE1;
  if (ps2_byte == 0x9d) {
   Keyboard_Putc(self,KEY_PRESSED|KEY_PAUSE);
   key = KEY_RELEASED|KEY_PAUSE;
  } else {
   Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1(0xe1));
   Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1(0x1d));
   Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1(0x45));
   Keyboard_Putc(self,KEYMAP_GET_PS2_SCANSET1(0xe1));
   goto again;
  }
  break;


  /* Scanset #2 */
 case STATE2:
  if (ps2_byte == 0xf0) { self->p_state = STATE2_F0; goto done; }
  if (ps2_byte == 0xe0) { self->p_state = STATE2_E0; goto done; }
  if (ps2_byte == 0xe1) { self->p_state = STATE2_E1; goto done; }
  key = (keyboard_key_t)keymap_ps2_scanset_2[ps2_byte];
  break;
 case STATE2_F0:
  key = (keyboard_key_t)keymap_ps2_scanset_2[ps2_byte]|KEY_RELEASED;
  self->p_state = STATE2;
  break;
 case STATE2_E0:
  if (ps2_byte == 0xf0) { self->p_state = STATE2_E0_F0; goto done; }
  if (ps2_byte == 0x12) { self->p_state = STATE2_E0_12; goto done; }
  key = keymap_ps2_scanset_2_e0[ps2_byte];
  self->p_state = STATE2;
  break;
 case STATE2_E0_F0:
  if (ps2_byte == 0x7c) { self->p_state = STATE2_E0_F0_7C; goto done; }
  key = KEY_RELEASED|keymap_ps2_scanset_2_e0[ps2_byte];
  self->p_state = STATE2;
  break;
 case STATE2_E0_12:
  if (ps2_byte == 0xe0) { self->p_state = STATE2_E0_12_E0; goto done; }
  Keyboard_Putc(self,keymap_ps2_scanset_2_e0[0x12]);
  self->p_state = STATE2;
  goto again;
 case STATE2_E0_12_E0:
  self->p_state = STATE2;
  if (ps2_byte == 0x7c) {
   key = KEY_PRESSED|KEY_PRINTSCREEN;
  } else {
   Keyboard_Putc(self,keymap_ps2_scanset_2_e0[0x12]);
   goto again;
  }
  break;
 case STATE2_E0_F0_7C:
  if (ps2_byte == 0xe0) { self->p_state = STATE2_E0_F0_7C_E0; goto done; }
  Keyboard_Putc(self,keymap_ps2_scanset_2_e0[0x7c]|KEY_RELEASED);
  self->p_state = STATE2;
  goto again;
 case STATE2_E0_F0_7C_E0:
  if (ps2_byte == 0xf0) { self->p_state = STATE2_E0_F0_7C_E0_F0; goto done; }
  Keyboard_Putc(self,keymap_ps2_scanset_2_e0[0x7c]|KEY_RELEASED);
  self->p_state = STATE2_E0;
  goto again;
 case STATE2_E0_F0_7C_E0_F0:
  self->p_state = STATE2;
  if (ps2_byte == 0x12) {
   key = KEY_RELEASED|KEY_PRINTSCREEN;
  } else {
   Keyboard_Putc(self,keymap_ps2_scanset_2_e0[0x7c]|KEY_RELEASED);
   self->p_state = STATE2_E0_F0;
   goto again;
  }
  break;
 case STATE2_E1:
  if (ps2_byte == 0x14) { self->p_state = STATE2_E1_14; goto done; }
  Keyboard_Putc(self,keymap_ps2_scanset_2[0xe1]);
  self->p_state = STATE2;
  goto again;
 case STATE2_E1_14:
  if (ps2_byte == 0x77) { self->p_state = STATE2_E1_14_77; goto done; }
  Keyboard_Putc(self,keymap_ps2_scanset_2[0xe1]);
  Keyboard_Putc(self,keymap_ps2_scanset_2[0x14]);
  self->p_state = STATE2;
  goto again;
 case STATE2_E1_14_77:
  if (ps2_byte == 0xe1) { self->p_state = STATE2_E1_14_77_E1; goto done; }
  Keyboard_Putc(self,keymap_ps2_scanset_2[0xe1]);
  Keyboard_Putc(self,keymap_ps2_scanset_2[0x14]);
  Keyboard_Putc(self,keymap_ps2_scanset_2[0x77]);
  self->p_state = STATE2;
  goto again;
 case STATE2_E1_14_77_E1:
  if (ps2_byte == 0xf0) { self->p_state = STATE2_E1_14_77_E1_F0; goto done; }
  Keyboard_Putc(self,keymap_ps2_scanset_2[0xe1]);
  Keyboard_Putc(self,keymap_ps2_scanset_2[0x14]);
  Keyboard_Putc(self,keymap_ps2_scanset_2[0x77]);
  self->p_state = STATE2_E1;
  goto again;
 case STATE2_E1_14_77_E1_F0:
  if (ps2_byte == 0x14) { self->p_state = STATE2_E1_14_77_E1_F0_14; goto done; }
  Keyboard_Putc(self,keymap_ps2_scanset_2[0xe1]);
  Keyboard_Putc(self,keymap_ps2_scanset_2[0x14]);
  Keyboard_Putc(self,keymap_ps2_scanset_2[0x77]);
  Keyboard_Putc(self,keymap_ps2_scanset_2[0xe1]);
  self->p_state = STATE2_F0;
  goto again;
 case STATE2_E1_14_77_E1_F0_14:
  if (ps2_byte == 0xf0) { self->p_state = STATE2_E1_14_77_E1_F0_14_F0; goto done; }
  Keyboard_Putc(self,keymap_ps2_scanset_2[0xe1]);
  Keyboard_Putc(self,keymap_ps2_scanset_2[0x14]);
  Keyboard_Putc(self,keymap_ps2_scanset_2[0x77]);
  Keyboard_Putc(self,keymap_ps2_scanset_2[0xe1]);
  Keyboard_Putc(self,keymap_ps2_scanset_2[0x14]|KEY_RELEASED);
  self->p_state = STATE2;
  goto again;
 case STATE2_E1_14_77_E1_F0_14_F0:
  if (ps2_byte == 0x12) {
   Keyboard_Putc(self,KEY_PRESSED|KEY_PAUSE);
   key = KEY_RELEASED|KEY_PAUSE;
   self->p_state = STATE2;
  } else {
   Keyboard_Putc(self,keymap_ps2_scanset_2[0xe1]);
   Keyboard_Putc(self,keymap_ps2_scanset_2[0x14]);
   Keyboard_Putc(self,keymap_ps2_scanset_2[0x77]);
   Keyboard_Putc(self,keymap_ps2_scanset_2[0xe1]);
   Keyboard_Putc(self,keymap_ps2_scanset_2[0x14]|KEY_RELEASED);
   self->p_state = STATE2_F0;
   goto again;
  }
  break;

 case STATE3:
  if (ps2_byte == 0xf0) { self->p_state = STATE3_F0; goto done; }
  key = keymap_ps2_scanset_3[ps2_byte];
  break;
 case STATE3_F0:
  key = keymap_ps2_scanset_3[ps2_byte]|KEY_RELEASED;
  break;


 default:
  assertf(0,"Invalid keyboard state %x",self->p_state);
  break;
 }
 Keyboard_Putc(self,key);
done:
 ;
}


PRIVATE void KCALL
Keyboard_Fini(Keyboard *__restrict self) {
 /* Delete the interrupt callback. */
 ps2_delete_callback(self->p_port,
                    (ps2_callback_t)&Keyboard_Interrupt,
                     self);
}
PRIVATE void KCALL
Keyboard_EnableScanning(Keyboard *__restrict self) {
 if (!ps2_runprogram(self->p_port,
                     NULL,
                     NULL,
                     ps2_keyboard_enable_scanning))
      error_throw(E_IOERROR);
}
PRIVATE void KCALL
Keyboard_DisableScanning(Keyboard *__restrict self) {
 if (!ps2_runprogram(self->p_port,
                     NULL,
                     NULL,
                     ps2_keyboard_disable_scanning))
      error_throw(E_IOERROR);
}


PRIVATE void KCALL
Keyboard_SetMode(Keyboard *__restrict self, u16 mode) {
 /* TODO */
}
PRIVATE void KCALL
Keyboard_SetLed(Keyboard *__restrict self,
                keyboard_ledset_t leds) {
 u8 argv[1];
 argv[0] = (u8)leds;
 if (!ps2_runprogram(self->p_port,
                     argv,
                     NULL,
                     ps2_keyboard_setleds))
      error_throw(E_IOERROR);
}
PRIVATE void KCALL
Keyboard_SetDelay(Keyboard *__restrict self,
                  struct kbdelay delay) {
 /* TODO */
}


PRIVATE struct keyboard_ops KeyboardOps = {
    .ko_fini             = (void (KCALL *)(struct keyboard *__restrict))&Keyboard_Fini,
    .ko_enable_scanning  = (void (KCALL *)(struct keyboard *__restrict))&Keyboard_EnableScanning,
    .ko_disable_scanning = (void (KCALL *)(struct keyboard *__restrict))&Keyboard_DisableScanning,
    .ko_setmode          = (void (KCALL *)(struct keyboard *__restrict,u16))&Keyboard_SetMode,
    .ko_setled           = (void (KCALL *)(struct keyboard *__restrict,keyboard_ledset_t))&Keyboard_SetLed,
    .ko_setdelay         = (void (KCALL *)(struct keyboard *__restrict,struct kbdelay))&Keyboard_SetDelay
};

/* Preferred scan-set. */
#define PS2_PREFERRED_SCANSET 2
#define PS2_ORIGINAL_SCANSET  1

PRIVATE ATTR_FREETEXT
void KCALL ps2_create_keyboard(u8 port) {
 REF Keyboard *kbd; u8 scanset;
 /* Try to reset the keyboard. */
 if (!ps2_runprogram(port,NULL,NULL,ps2_keyboard_reset))
      return;
 ps2_runprogram(port,NULL,NULL,ps2_keyboard_setdefaults);
 ps2_runprogram(port,NULL,NULL,ps2_keyboard_disable_scanning);
 {
  u8 argv[] = { PS2_PREFERRED_SCANSET };
  scanset = PS2_PREFERRED_SCANSET;
  if (!ps2_runprogram(port,argv,NULL,ps2_keyboard_setscanset)) {
   /* Ask the device what it's scanset is. */
   if (ps2_runprogram(port,NULL,&scanset,ps2_keyboard_getscanset)) {
    if unlikely(scanset < 1 || scanset > 3) {
     /* Wtf? (Maybe this keyboard doesn't understand the command?)
      * Assume the original scanset and try to ask it to set that one again... */
     scanset = PS2_ORIGINAL_SCANSET;
#if PS2_PREFERRED_SCANSET != PS2_ORIGINAL_SCANSET
     ps2_runprogram(port,&scanset,NULL,ps2_keyboard_setscanset);
#endif
    }
   } else {
    /* Default: Assume the original scanset. */
    scanset = PS2_ORIGINAL_SCANSET;
   }
  }
 }
 /* Re-enable scanning. */
 ps2_runprogram(port,NULL,NULL,ps2_keyboard_enable_scanning);

 kbd = KEYBOARD_ALLOC(Keyboard,&KeyboardOps);
 TRY {
  /* Initialize the device number and name fields. */
  kbd->p_keyboard.k_dev.c_device.d_devno = MKDEV(MINOR(DV_PS2_KEYBOARD),
                                                 port);
  sprintf(kbd->p_keyboard.k_dev.c_device.d_namebuf,
          "ps2_kbd%c",'a'+port);
  /* Setup port and initial keyboard state. */
  kbd->p_port  = port;
  kbd->p_state = STATE_INITIAL(scanset);
  /* Hook the PS/2 interrupt callback for the keyboard. */
  ps2_install_callback(port,
                      (ps2_callback_t)&Keyboard_Interrupt,
                       kbd);
  /* Register the device. */
  asserte(register_device(&kbd->p_keyboard.k_dev.c_device));
 } FINALLY {
  keyboard_decref(&kbd->p_keyboard);
 }
}

PRIVATE void KCALL
ps2_detect_keyboard_threadmain(void *arg) {
 /* Use the ECHO command to detect keyboards. */
 if (ps2_runprogram(PS2_PORT1,NULL,NULL,ps2_keyboard_echo))
     ps2_create_keyboard(PS2_PORT1);
 if (ps2_runprogram(PS2_PORT2,NULL,NULL,ps2_keyboard_echo))
     ps2_create_keyboard(PS2_PORT2);
}


DEFINE_DRIVER_INIT(ps2_init_keyboard);
INTERN ATTR_FREETEXT void KCALL ps2_init_keyboard(void) {
 /* Spawn a worker thread for keyboard initialization to deal with the timeouts. */
 REF struct task *worker;
 worker = task_alloc();
 TRY {
  task_setup_kernel(worker,
                   &ps2_detect_keyboard_threadmain,
                    NULL);
  task_start(worker);
  /* TODO: Race condition:
   *   User-space must be able to wait for this thread
   *   to terminate, because otherwise it won't know
   *   when the keyboard is actually ready...
   * Solution:
   *   Keep track of all threads that need to complete
   *   for initializing devices and add a kernctl()
   *   command to wait for those threads to complete.
   *   /bin/init can then invoke that command to make
   *   sure that stuff like the keyboard have finished
   *   initializing. */
 } FINALLY {
  task_decref(worker);
 }
}






PS2_DEFINE_PUBLIC_PROGRAM(
 ps2_keyboard_getscanset,
 ps2_send   PS2_COMMAND_SCANSET; /* Scanset command */
 ps2_wait   PS2_ACK;             /* Wait for ACK */
 ps2_send   0;                   /* Read scanset */
 ps2_wait   PS2_ACK;             /* Wait for ACK */
 ps2_wait   %res;                /* Wait for, and save results */
 ps2_stop;                       /* done */
);

PS2_DEFINE_PUBLIC_PROGRAM(
 ps2_keyboard_setscanset,
 ps2_send   PS2_COMMAND_SCANSET; /* Scanset command */
 ps2_wait   PS2_ACK;             /* Wait for ACK */
 ps2_send   %arg;                /* Set the new scanset. */
 ps2_wait   PS2_ACK;             /* Wait for ACK */
 ps2_stop;                       /* done */
);

PS2_DEFINE_PUBLIC_PROGRAM(
 ps2_keyboard_setleds,
 ps2_send   PS2_COMMAND_SETLED; /* Set LEDs */
 ps2_wait   PS2_ACK;            /* Wait for ACK */
 ps2_send   %arg;               /* Set the new scanset. */
 ps2_wait   PS2_ACK;            /* Wait for ACK */
 ps2_stop;                      /* done */
);

PS2_DEFINE_PUBLIC_PROGRAM(
 ps2_keyboard_enable_scanning,
 ps2_send   PS2_COMMAND_ENABLE_SCANNING;
 ps2_wait   PS2_ACK;
 ps2_stop;
);

PS2_DEFINE_PUBLIC_PROGRAM(
 ps2_keyboard_disable_scanning,
 ps2_send   PS2_COMMAND_DISABLE_SCANNING;
 ps2_wait   PS2_ACK;
 ps2_stop;
);

PS2_DEFINE_PUBLIC_PROGRAM(
 ps2_keyboard_echo,
 ps2_send   PS2_COMMAND_ECHO;
 ps2_wait   0xee;
 ps2_stop;
);

PS2_DEFINE_PUBLIC_PROGRAM(
 ps2_keyboard_reset,
 ps2_send   PS2_COMMAND_RESET;
 ps2_wait   PS2_ACK;
 ps2_wait   0xaa;
 ps2_stop;
);

PS2_DEFINE_PUBLIC_PROGRAM(
 ps2_keyboard_setdefaults,
 ps2_send   PS2_COMMAND_SETDEFAULT;
 ps2_wait   PS2_ACK;
 ps2_stop;
);

DECL_END

#endif /* CONFIG_HAVE_DEV_PS2 */

#endif /* !GUARD_KERNEL_SRC_DEV_PS2_KEYBOARD_C */
