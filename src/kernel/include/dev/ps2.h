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
#ifndef GUARD_KERNEL_INCLUDE_DEV_PS2_H
#define GUARD_KERNEL_INCLUDE_DEV_PS2_H 1

#include <hybrid/compiler.h>
#include <dev/devconfig.h>

#ifdef CONFIG_HAVE_DEV_PS2
#include <hybrid/asm.h>
#include <kos/types.h>
#include <fs/device.h>

DECL_BEGIN

#define PS2_DATA   0x60
#define PS2_STATUS 0x64
#define PS2_CMD    0x64

/* PS/2 status port flags. */
#define PS2_STATUS_OUTFULL              0x01
#define PS2_STATUS_INFULL               0x02
#define PS2_STATUS_SYSTEM               0x04
#define PS2_STATUS_IN_IS_CONTROLLER_CMD 0x08
#define PS2_STATUS_KB_ENABLED           0x10
#define PS2_STATUS_OUTFULL2             0x20
#define PS2_STATUS_TIMEOUT_ERROR        0x40
#define PS2_STATUS_PARITY_ERROR         0x80

#define PS2_ACK    0xfa
#define PS2_RESEND 0xfe

/* PS/2 controller command codes. */
#define PS2_CONTROLLER_RRAM(n)           (0x20+(n))
#define PS2_CONTROLLER_WRAM(n)           (0x60+(n))
#define PS2_CONTROLLER_TEST_PORT1         0xaa
#define PS2_CONTROLLER_ENABLE_PORT1       0xae
#define PS2_CONTROLLER_DISABLE_PORT1      0xad
#define PS2_CONTROLLER_TEST_PORT2         0xa9
#define PS2_CONTROLLER_ENABLE_PORT2       0xa8
#define PS2_CONTROLLER_DISABLE_PORT2      0xa7
#define PS2_CONTROLLER_READ_OUTPUT        0xd0
#define PS2_CONTROLLER_WRITE_OUTPUT       0xd1
#define PS2_CONTROLLER_WRITE_PORT1_OUTPUT 0xd2
#define PS2_CONTROLLER_WRITE_PORT2_OUTPUT 0xd3
#define PS2_CONTROLLER_WRITE_PORT2_INPUT  0xd4

/* Layout of the `PS2_CONTROLLER_RRAM(0)' (Controller configuration) byte. */
#define PS2_CONTROLLER_CFG_PORT1_IRQ       0x01
#define PS2_CONTROLLER_CFG_PORT2_IRQ       0x02
#define PS2_CONTROLLER_CFG_SYSTEMFLAG      0x04
#define PS2_CONTROLLER_CFG_PORT1_CLOCK     0x10
#define PS2_CONTROLLER_CFG_PORT2_CLOCK     0x20
#define PS2_CONTROLLER_CFG_PORT1_TRANSLATE 0x40

#define PS2_PORT1      0x00 /* Send command to port #1 */
#define PS2_PORT2      0x01 /* Send command to port #2 */
#define PS2_PORTCOUNT  0x02 /* Number of PS/2 ports. */



#define PS2_PORT_DEVICE_FNOTHING  0x00 /* Nothing is connected. */
#define PS2_PORT_DEVICE_FUNKNOWN  0x00 /* An unknown device is connected. */
#define PS2_PORT_DEVICE_FKEYBOARD 0x01 /* A keyboard is connected. */
#define PS2_PORT_DEVICE_FMOUSE    0x02 /* A mouse is connected. */


#ifdef __CC__

/* [*] The type of device connected to the specified PS/2 port (One of `PS2_PORT_DEVICE_F*') */
DATDEF u8 ps2_port_device[PS2_PORTCOUNT];

typedef ASYNCSAFE void (KCALL *ps2_callback_t)(void *arg, byte_t byte);

/* Install/delete a PS/2 interrupt data callback.
 * WARNING: The callback is executed with preemption disabled, and must
 *          not re-enable preemption even for a single instruction!
 * @param: port:   The PS/2 port to connect/disconnect to/from (On of `PS2_PORT*')
 * @param: func:   The callback to install/delete for invocation during an interrupt.
 * @param: arg:    The argument passed to `func' during invocation.
 * @return: true:  The given `func' and `arg' was deleted and will no longer be invoked during an interrupt.
 * @return: false: The given `func' and `arg' wasn't installed.
 * @throw: E_BADALLOC:   [ps2_install_buffer] Failed to allocate sufficient memory.
 * @throw: E_WOULDBLOCK: Preemption has been disabled and the operation would have blocked. */
FUNDEF void KCALL ps2_install_callback(u8 port, ps2_callback_t func, void *arg);
FUNDEF bool KCALL ps2_delete_callback(u8 port, ps2_callback_t func, void *arg);

#endif

DECL_END

#endif /* CONFIG_HAVE_DEV_PS2 */

#endif /* !GUARD_KERNEL_INCLUDE_DEV_PS2_H */
