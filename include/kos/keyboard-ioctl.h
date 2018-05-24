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
#ifndef _KOS_KEYBOARD_IOCTL_H
#define _KOS_KEYBOARD_IOCTL_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>
#include <asm/ioctl.h>
#include <kos/keyboard.h>

__SYSDECL_BEGIN

#ifdef __CC__
typedef __UINT32_TYPE__ keyboard_key_t;    /* One of `KEY_*' */
typedef __UINT32_TYPE__ keyboard_state_t;  /* Set of `KEYBOARD_STATE_F*' */
typedef __UINT32_TYPE__ keyboard_ledset_t; /* Set of `KEYBOARD_LED_F*' */
#endif /* __CC__ */

#define KEYBOARD_STATE_FADD(x,y) ((x) |= (y))
#define KEYBOARD_STATE_FXOR(x,y) ((x) ^= (y))
#define KEYBOARD_STATE_FDEL(x,y) ((x) &= ~(y))
#define KEYBOARD_STATE_FHAS(x,y) ((x)&(y))

#define KEYBOARD_STATE_FLSHIFT     0x00000001 /* [MIRROR(KEY_LSHIFT)] */
#define KEYBOARD_STATE_FRSHIFT     0x00000002 /* [MIRROR(KEY_RSHIFT)] */
#define KEYBOARD_STATE_FLCTRL      0x00000004 /* [MIRROR(KEY_LCTRL)] */
#define KEYBOARD_STATE_FRCTRL      0x00000008 /* [MIRROR(KEY_RCTRL)] */
#define KEYBOARD_STATE_FLGUI       0x00000010 /* [MIRROR(KEY_LGUI)] */
#define KEYBOARD_STATE_FRGUI       0x00000020 /* [MIRROR(KEY_RGUI)] */
#define KEYBOARD_STATE_FLALT       0x00000040 /* [MIRROR(KEY_LALT)] */
#define KEYBOARD_STATE_FRALT       0x00000080 /* [MIRROR(KEY_RALT)] */
#define KEYBOARD_STATE_FESC        0x00000100 /* [MIRROR(KEY_ESC)] */
#define KEYBOARD_STATE_FTAB        0x00000200 /* [MIRROR(KEY_TAB)] */
#define KEYBOARD_STATE_FSPACE      0x00000400 /* [MIRROR(KEY_SPACE)] */
#define KEYBOARD_STATE_FAPPS       0x00000800 /* [MIRROR(KEY_APPS)] */
#define KEYBOARD_STATE_FENTER      0x00001000 /* [MIRROR(KEY_ENTER)] */
#define KEYBOARD_STATE_FKP_ENTER   0x00002000 /* [MIRROR(KEY_KP_ENTER)] */
#define KEYBOARD_STATE_FOVERRIDE   0x00004000 /* [TOGGLE(KEY_INSERT)] */
#define KEYBOARD_STATE_FCAPSLOCK   0x10000000 /* [TOGGLE(KEY_CAPSLOCK)] */
#define KEYBOARD_STATE_FNUMLOCK    0x20000000 /* [TOGGLE(KEY_NUMLOCK)] */
#define KEYBOARD_STATE_FSCROLLLOCK 0x40000000 /* [TOGGLE(KEY_SCROLLLOCK)] */

/* Helper macros for checking key states. */
#define KEYBOARD_STATE_FISCTRL(x)       ((x)&(KEYBOARD_STATE_FLCTRL|KEYBOARD_STATE_FRCTRL))
#define KEYBOARD_STATE_FISSHIFT(x)      ((x)&(KEYBOARD_STATE_FLSHIFT|KEYBOARD_STATE_FRSHIFT))
#define KEYBOARD_STATE_FISGUI(x)        ((x)&(KEYBOARD_STATE_FLGUI|KEYBOARD_STATE_FRGUI))
#define KEYBOARD_STATE_FISALT(x)        ((x)&(KEYBOARD_STATE_FLALT|KEYBOARD_STATE_FRALT))
#define KEYBOARD_STATE_FISALTGR(x)      ((x)&KEYBOARD_STATE_FRALT || ((x)&(KEYBOARD_STATE_FLALT) && KEYBOARD_STATE_FISCTRL(x)))
#define KEYBOARD_STATE_FISCAPS(x)    (!!((x)&KEYBOARD_STATE_FCAPSLOCK) ^ !!KEYBOARD_STATE_FISSHIFT(x))
#define KEYBOARD_STATE_FISNUMLOCK(x) (!!((x)&KEYBOARD_STATE_FNUMLOCK) ^ !!KEYBOARD_STATE_FISSHIFT(x)) /* Yes: Numlock can be inverted with shift ;) */

/* ioctl() commands. */
#define KEYBOARD_ENABLE_SCANNING   _IO('K',16)
#define KEYBOARD_DISABLE_SCANNING  _IO('K',17)
#define     KEYBOARD_LED_FSCROLLLOCK   0x0001
#define     KEYBOARD_LED_FNUMLOCK      0x0002
#define     KEYBOARD_LED_FCAPSLOCK     0x0004
#define KEYBOARD_GET_LEDS          _IOR('K',18,keyboard_ledset_t)
#define KEYBOARD_SET_LEDS          _IOW('K',19,keyboard_ledset_t[2]) /* mask, flags */
#define     KEYBOARD_MODE_FUPDOWN   0x0000 /* Keyboard mode: Send keycodes for up (release) and down (press) */
#define     KEYBOARD_MODE_FDOWNONLY 0x0001 /* Keyboard mode: Only send keycodes for down (press) */
#define     KEYBOARD_FENABLED       0x0000 /* Scanning has been enabled. */
#define     KEYBOARD_FMIRRORLEDS    0x4000 /* FLAG: Drivers should mirror `k_state' in `k_leds'. */
#define     KEYBOARD_FDISABLED      0x8000 /* FLAG: Scanning has been disabled. */
#define     KEYBOARD_FINITIAL      (KEYBOARD_MODE_FUPDOWN|KEYBOARD_FDISABLED|KEYBOARD_FMIRRORLEDS)
#define KEYBOARD_GET_MODE          _IOR('K',20,__UINT16_TYPE__)
#define KEYBOARD_SET_MODE          _IOW('K',21,__UINT16_TYPE__[2]) /* mask, flags */
#ifdef __CC__
struct kbdelay {
    union {
        struct {
            __UINT16_TYPE__   kd_repeat_delay; /* Delay before keystrokes are repeated (in milliseconds; 0 -> don't repeat) */
            __UINT16_TYPE__   kd_repeat_rate;  /* Keystroke repeat rate (in HZ; repeats/second) */
        };
        __UINT32_TYPE__       kd_state;        /* Keyboard repeat state control word. */
    };
};
#endif
#define KEYBOARD_GET_DELAY         _IOR('K',22,struct kbdelay)
#define KEYBOARD_SET_DELAY         _IOW('K',23,struct kbdelay)
#define KEYBOARD_LOAD_KEYMAP       _IOW('K',24,Kmp_Header) /* The keymap can be mmap()-ed into userspace by specifying
                                                            * the keyboard file descriptor and `KEYBOARD_KEYMAP_OFFSET'
                                                            * as file offset.
                                                            * The argument to this ioctl() is the pre-loaded data blob
                                                            * of a `.kmp' file (s.a. `<kos/keymap.h>')
                                                            * Unmodified mappings of the keymap are implicitly updated
                                                            * to refer to the new keymap upon success. */
#define KEYBOARD_KEYMAP_OFFSET     0x10000 /* Keyboard keymap mmap() file offset.
                                            * >> struct keyboard_keymap *map;
                                            * >> fd_t fd = open("/dev/keyboard",O_RDONLY);
                                            * >> map = (struct keyboard_keymap *)mmap(NULL,
                                            * >>                                      sizeof(struct keyboard_keymap),
                                            * >>                                      PROT_READ,
                                            * >>                                      MAP_PRIVATE,
                                            * >>                                      fd,
                                            * >>                                      KEYBOARD_KEYMAP_OFFSET);
                                            * >> printf("Active keymap: %.64q\n",map->km_name);
                                            * >> keyboard_key_t key;
                                            * >> read(fd,&key,sizeof(key));
                                            * >> key &= ~(KEY_FPRESSED|KEY_FRELEASED);
                                            * >> if (key >= 256) {
                                            * >>     printf("Special key %x pressed\n",key);
                                            * >> } else {
                                            * >>     printf("You pressed %x: %.4q, %.4q (shift), %.4q (ctrl+alt)\n"
                                            * >>            key,
                                            * >>            map->km_press[key],
                                            * >>            map->km_shift[key],
                                            * >>            map->km_altgr[key]);
                                            * >> } */

#ifdef __CC__
struct keyboard_keymap_char {
    char kmc_utf8[4]; /* NUL/strnlen-terminated UTF-8 character sequence
                       * encoding a UTF-32 character that should be printed
                       * when this key is pressed.
                       * Use functions from <unicode.h> to translate to a
                       * UTF-32 character, or simply print a number of
                       * `strnlen(kmc_utf8,4)' characters from this string.
                       * >> struct keyboard_keymap_char *ch = GETCH();
                       * >> write(STDOUT_FILENO,ch->kmc_utf8,
                       * >>       strnlen(ch->kmc_utf8,COMPILER_LENOF(ch->kmc_utf8))); */
};
struct keyboard_keymap {
    char                        km_name[64];   /* Keymap name (NUL-terminated) (e.g. `en_us'). */
    struct keyboard_keymap_char km_press[256]; /* Regular key map. */
    struct keyboard_keymap_char km_shift[256]; /* Key map while the shift-key is held. */
    struct keyboard_keymap_char km_altgr[256]; /* Key map when RALT or CTRL+ALT is held down. */
};
#endif

__SYSDECL_END

#endif /* !_KOS_KEYBOARD_IOCTL_H */
