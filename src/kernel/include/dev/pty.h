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
#ifndef GUARD_KERNEL_INCLUDE_DEV_PTY_H
#define GUARD_KERNEL_INCLUDE_DEV_PTY_H 1

#include <hybrid/compiler.h>
#include <dev/tty.h>
#include <fs/ringbuffer.h>
#include <kos/types.h>
#include <kos/kdev_t.h>

DECL_BEGIN

struct ptyslave {
    struct tty              ps_tty;     /* Underlying TTY device. */
    struct ringbuffer       ps_display; /* Display buffer (for Slave --> Master comunications) */
};
/* Increment/decrement the reference counter of the given ptyslave `x' */
#define ptyslave_incref(x) tty_incref(&(x)->ps_tty)
#define ptyslave_decref(x) tty_decref(&(x)->ps_tty)


struct ptymaster {
    struct character_device pm_dev;    /* Underlying character device. */
    REF struct ptyslave    *pm_slave;  /* [1..1] The associated slave. */
};
/* Increment/decrement the reference counter of the given ptymaster `x' */
#define ptymaster_incref(x) character_device_incref(&(x)->pm_dev)
#define ptymaster_decref(x) character_device_decref(&(x)->pm_dev)


/* Allocate and pre/null/zero-initialize a new PTY master device.
 * The caller must initialize the following members:
 *    - t_dev.c_device.d_namebuf      (Unless `d_name' is set directly)
 *    - t_dev.c_device.d_name         (Optionally; pre-initialized to `d_namebuf')
 *    - t_dev.c_device.d_devno        (See explanation below)
 * Following this, a device ID should be allocated for, or assigned
 * to the device, either by directly setting `return->ps_tty.t_dev.d_device.d_devno',
 * or by calling `devno_alloc()', or `register_dynamic_device()' (see above.) */
FUNDEF ATTR_RETNONNULL ATTR_MALLOC
REF struct ptyslave *KCALL pty_allocslave(void);
FUNDEF ATTR_RETNONNULL ATTR_MALLOC REF struct ptymaster *
KCALL pty_allocmaster(struct ptyslave *__restrict master);


/* Register a new PTY master/slave pair.
 * This function allocates a device number and initializes
 * the name of both devices before registering both.
 * @throw: E_BADALLOC.ERROR_BADALLOC_DEVICEID: Failed to allocate an unused PTY ID. */
FUNDEF void KCALL
pty_register(struct ptymaster *__restrict master,
             struct ptyslave *__restrict slave);



struct pty_devpair {
    dev_t dp_master; /* Master device number. */
    dev_t dp_slave;  /* Slave device number. */
};

/* Allocate a device number pair for a PTY terminal. */
FUNDEF struct pty_devpair KCALL devno_alloc_pty(void);



DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_DEV_PTY_H */
