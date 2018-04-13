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
#ifndef GUARD_KERNEL_SRC_DEV_PS2_KEYBOARD_H
#define GUARD_KERNEL_SRC_DEV_PS2_KEYBOARD_H 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <dev/ps2.h>
#include <kos/keyboard.h>
#include <dev/keyboard.h>

#ifdef CONFIG_HAVE_DEV_PS2

DECL_BEGIN

/* Get the current keyboard scanset.
 * IN:  [];         OUT: [scanset]
 * @param: scanset: ???? (this one's kind-of weird...) */
DATDEF u8 const ps2_keyboard_getscanset[];

/* Set the current keyboard scanset.
 * IN:  [scanset];  OUT: []
 * @param: scanset: One of `PS2_SCANSET_*' */
DATDEF u8 const ps2_keyboard_setscanset[];
#define PS2_SCANSET_1  0x01
#define PS2_SCANSET_2  0x02
#define PS2_SCANSET_3  0x03

/* Set the currently active keyboard LEDs.
 * IN:  [ledset];   OUT: []
 * @param: ledset: Set of `KEYBOARD_LED_F*' */
DATDEF u8 const ps2_keyboard_setleds[];


/* Start scanning.  IN:  [];   OUT: [] */
DATDEF u8 const ps2_keyboard_enable_scanning[];
/* Start scanning.  IN:  [];   OUT: [] */
DATDEF u8 const ps2_keyboard_disable_scanning[];
/* Check device presence.  IN:  [];   OUT: [] */
DATDEF u8 const ps2_keyboard_echo[];
/* Reset the keyboard.  IN:  [];   OUT: [] */
DATDEF u8 const ps2_keyboard_reset[];
/* Set default parameters.  IN:  [];   OUT: [] */
DATDEF u8 const ps2_keyboard_setdefaults[];


DECL_END

#endif /* CONFIG_HAVE_DEV_PS2 */

#endif /* !GUARD_KERNEL_SRC_DEV_PS2_KEYBOARD_H */
