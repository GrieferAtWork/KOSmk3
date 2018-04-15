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
#ifndef GUARD_KERNEL_INCLUDE_DEV_MOUSE_H
#define GUARD_KERNEL_INCLUDE_DEV_MOUSE_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/mouse-ioctl.h>
#include <hybrid/atomic.h>
#include <sched/mutex.h>
#include <fs/device.h>
#include <fs/wordbuffer.h>
#include <fs/iomode.h>

/* Mouse Abstract Kernel API Interface. */

DECL_BEGIN

struct mouse_ops {
    /* Mouse finalizer */
    void (KCALL *mo_fini)(struct mouse *__restrict self);

    /* [1..1][locked(k_lock)] Enable/Disable scanning of keycodes. */
    void (KCALL *mo_enable_reports)(struct mouse *__restrict self);
    void (KCALL *mo_disable_reports)(struct mouse *__restrict self);
};

struct device_mouse_packet {
    /* PS/2-compatible set of mouse bytes. */
    u8    dmp_bytes[4];
    /* XXX: Timestamp? */
};


struct mouse {
    struct character_device    m_dev;  /* Underlying character device. */
    struct mouse_ops          *m_ops;  /* [1..1][const] Mouse operators. */
    struct wordbuffer          m_buf;  /* Word buffer of `struct device_mouse_packet' structures. */
    mutex_t                    m_lock; /* Lock for mouse properties. */
    struct device_mouse_packet m_pend; /* [lock(m_lock)][valid_if(.dmp_bytes[0] & 0x08)]
                                        * Pending, read, but unprocessed mouse packet. */
};

/* Read a data packet from the given mouse and store its contents in
 * the user-space compatible format in the given `packet' buffer.
 * @return: true:  The given `packet' has been filled with data.
 * @return: false: `IO_NONBLOCK' Was specified and no data was immediately available. */
FUNDEF bool KCALL
mouse_readpacket(struct mouse *__restrict self,
                 USER CHECKED struct mouse_packet *__restrict packet, iomode_t flags);



DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_DEV_MOUSE_H */
