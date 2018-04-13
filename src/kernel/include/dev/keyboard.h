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
#ifndef GUARD_KERNEL_INCLUDE_DEV_KEYBOARD_H
#define GUARD_KERNEL_INCLUDE_DEV_KEYBOARD_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/keyboard_ioctl.h>
#include <hybrid/atomic.h>
#include <sched/mutex.h>
#include <fs/device.h>
#include <fs/wordbuffer.h>
#include <fs/iomode.h>

/* Keyboard Abstract Kernel API Interface. */

DECL_BEGIN

struct keyboard;

/* Allocate a new keymap.
 * NOTE: These functions allocate/free whole pages, meaning that the returned
 *       keyboard map is suitable for mapping in user-space as a USHARE segment.
 * NOTE: The data contained within the region is a `struct keyboard_keymap' */
FUNDEF ATTR_RETNONNULL REF struct vm_region *KCALL keymap_alloc(void);


struct keyboard_ops {
    /* Keyboard finalizer */
    void (KCALL *ko_fini)(struct keyboard *__restrict self);

    /* [1..1][locked(k_lock)] Enable/Disable scanning of keycodes. */
    void (KCALL *ko_enable_scanning)(struct keyboard *__restrict self);
    void (KCALL *ko_disable_scanning)(struct keyboard *__restrict self);

    /* [0..1][locked(k_lock)] Set the keyboard LED state.
     * @param: mode: One of `KEYBOARD_MODE_F*' */
    void (KCALL *ko_setmode)(struct keyboard *__restrict self, u16 mode);

    /* [0..1][locked(k_lock)] Set the keyboard LED state. */
    void (KCALL *ko_setled)(struct keyboard *__restrict self, keyboard_ledset_t leds);

    /* [0..1][locked(k_lock)] Set keyboard delay properties. */
    void (KCALL *ko_setdelay)(struct keyboard *__restrict self, struct kbdelay delay);
};


struct keyboard {
    struct character_device k_dev;   /* Underlying character device. */
    struct keyboard_ops    *k_ops;   /* [1..1][const] Keyboard operators. */
    REF struct vm_region   *k_map;   /* [1..1][const] Keyboard key -> character map. */
    struct wordbuffer       k_buf;   /* Keyboard buffer. */
    mutex_t                 k_lock;  /* Lock for keyboard properties. */
    struct kbdelay          k_delay; /* [lock(k_lock)] The current keyboard delay. */
    u16                     k_mode;  /* [lock(k_lock)] Keyboard state & mode (One of `KEYBOARD_MODE_F*' + set of `KEYBOARD_F*') */
    u16                   __k_pad;   /* ... */
    WEAK keyboard_ledset_t  k_leds;  /* The current state of keyboard LEDs. */
    WEAK keyboard_state_t   k_state; /* The current keyboard delay. */
};

/* Increment/decrement the reference counter of the given keyboard `x' */
#define keyboard_incref(x) character_device_incref(&(x)->k_dev)
#define keyboard_decref(x) character_device_decref(&(x)->k_dev)

/* Allocate and pre/null/zero-initialize a new keyboard device,
 * who's control structure has a size of `struct_size' bytes.
 * The caller must initialize the following members:
 *    - k_dev.c_device.d_namebuf      (Unless `d_name' is set directly)
 *    - k_dev.c_device.d_name         (Optionally; pre-initialized to `d_namebuf')
 *    - k_dev.c_device.d_devno        (See explanation below)
 * The following fields have been specially initialized:
 *    - k_ops       == ops  (argument)
 *    - k_dev.c_ops == ...  (internal keyboard character device operators)
 *    - k_mode      == KEYBOARD_FINITIAL
 * The following character device operators are implemented by every keyboard:
 *    - c_file.f_ioctl; For the following IOCTL commands:
 *       - KEYBOARD_ENABLE_SCANNING
 *       - KEYBOARD_DISABLE_SCANNING
 *       - KEYBOARD_GET_LEDS
 *       - KEYBOARD_SET_LEDS
 *       - KEYBOARD_GET_MODE
 *       - KEYBOARD_SET_MODE
 *       - KEYBOARD_GET_DELAY
 *       - KEYBOARD_SET_DELAY
 *    - f_stat; Fills in the following members:
 *       - st_size      (Available bytes for reading)
 *       - st_blksize   (Set to `MAX_INPUT')
 *       - st_blocks    (Set to `1')
 *    - f_read;  Read `keyboard_key_t' objects from the buffer.
 *    - f_write; Add `keyboard_key_t' objects to the buffer (fake key strokes)
 *      @throw: E_INVALID_ARGUMENT: Cannot write `KEY_UNKNOWN'
 * @throw: E_DRIVER_CLOSED: The given driver has been closed/is closing.
 * Following this, a device ID should be allocated for, or assigned
 * to the device, either by directly setting `return->d_device.d_devno',
 * or by calling `devno_alloc()', or `register_dynamic_device()' (see above.) */
FUNDEF ATTR_RETNONNULL REF struct keyboard *KCALL
__keyboard_alloc(size_t struct_size,
                 struct keyboard_ops *__restrict ops,
                 struct driver *__restrict caller)
                 ASMNAME("keyboard_alloc");
#define keyboard_alloc(struct_size,ops) \
      __keyboard_alloc(struct_size,ops,&this_driver)
#define KEYBOARD_ALLOC(T,ops) \
  ((T *)keyboard_alloc(sizeof(T),ops))



/* Helper macros for reading/writing key strokes */
#define keyboard_getkey(self)     wordbuffer_getword(&(self)->k_buf)
#define keyboard_trygetkey(self)  wordbuffer_trygetword(&(self)->k_buf)
#define keyboard_putkey(self,key) wordbuffer_putword(&(self)->k_buf,(key))

/* Enable/Disable scanning of keycodes. */
FUNDEF void KCALL keyboard_enable_scanning(struct keyboard *__restrict self, iomode_t flags);
FUNDEF void KCALL keyboard_disable_scanning(struct keyboard *__restrict self, iomode_t flags);

/* Set the mode of the given keyboard.
 * The new mode is set to `(old_mode & mask) | mode'
 * @param: mask: Mask of mode bits to keep.
 * @param: mode: One of `KEYBOARD_MODE_F*', or'd with any of `KEYBOARD_F*'
 * HINT: `keyboard_setmode(...,~0,KEYBOARD_FDISABLED)' is the same as calling `keyboard_disable_scanning()'
 * HINT: `keyboard_setmode(...,~KEYBOARD_FDISABLED,0)' is the same as calling `keyboard_enable_scanning()' */
FUNDEF void KCALL keyboard_setmode(struct keyboard *__restrict self,
                                   unsigned int mask, unsigned int mode,
                                   iomode_t flags);

/* Get (return) the current mode of the keyboard. */
LOCAL unsigned int KCALL keyboard_getmode(struct keyboard *__restrict self);

/* Set the state of keyboard LEDs.
 * The new led state is set to `(old_leds & mask) | leds' */
FUNDEF void KCALL keyboard_setleds(struct keyboard *__restrict self,
                                   keyboard_ledset_t mask,
                                   keyboard_ledset_t leds,
                                   iomode_t flags);
/* Get (return) the current keyboard LED state. */
LOCAL keyboard_ledset_t KCALL keyboard_getleds(struct keyboard *__restrict self);

/* Load the keyboard key map from the given blob.
 * @return: true:  Successfully loaded the keymap.
 * @return: false: The KMP data blob is corrupted. */
FUNDEF bool KCALL keyboard_loadmap(struct keyboard *__restrict self,
                                   USER CHECKED void *blob);


/* Set the hardware-supported repeat capability of the keyboard. */
FUNDEF void KCALL keyboard_setrepeat(struct keyboard *__restrict self,
                                     struct kbdelay mode, iomode_t flags);
/* Get (return) the hardware-supported repeat capability of the keyboard. */
LOCAL struct kbdelay KCALL keyboard_getrepeat(struct keyboard *__restrict self);




#ifndef __INTELLISENSE__
LOCAL unsigned int KCALL
keyboard_getmode(struct keyboard *__restrict self) {
 return ATOMIC_READ(self->k_mode);
}
LOCAL keyboard_ledset_t KCALL
keyboard_getleds(struct keyboard *__restrict self) {
 return ATOMIC_READ(self->k_leds);
}
LOCAL struct kbdelay KCALL
keyboard_getrepeat(struct keyboard *__restrict self) {
 struct kbdelay result;
 result.kd_state = ATOMIC_READ(self->k_delay.kd_state);
 return result;
}
#endif


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_DEV_KEYBOARD_H */
