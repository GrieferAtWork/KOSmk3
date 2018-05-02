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
#ifndef _KOS_MOUSE_IOCTL_H
#define _KOS_MOUSE_IOCTL_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <asm/ioctl.h>

__SYSDECL_BEGIN


#define MOUSE_BUTTON_FLEFT   0x0001 /* Left mouse button is held down */
#define MOUSE_BUTTON_FRIGHT  0x0002 /* Right mouse button is held down */
#define MOUSE_BUTTON_FMIDDLE 0x0004 /* Middle mouse button is held down */
#define MOUSE_BUTTON_F4      0x0010 /* 4th mouse button is held down */
#define MOUSE_BUTTON_F5      0x0020 /* 5th mouse button is held down */

struct mouse_packet {
    __jtime64_t       mp_time; /* Timestamp of this packet (in jiffies) */
    __INT64_TYPE__    mp_relx; /* Relative X distance traveled. (may be merged between multiple packets) */
    __INT64_TYPE__    mp_rely; /* Relative Y distance traveled. (may be merged between multiple packets) */
    __INT64_TYPE__    mp_relz; /* Relative wheel motion. */
    __UINT16_TYPE__   mp_keys; /* Mouse buttons currently held down. (Set of `MOUSE_BUTTON_F*') */
    __UINT16_TYPE__   mp_chng; /* Bitset of changed mouse buttons when compared to the previous mouse packet. */
};


__SYSDECL_END

#endif /* !_KOS_MOUSE_IOCTL_H */
