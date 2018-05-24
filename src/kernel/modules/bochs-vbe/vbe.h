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
#ifndef GUARD_KERNEL_MODULES_BOCHS_VBE_VBE_H
#define GUARD_KERNEL_MODULES_BOCHS_VBE_VBE_H 1

#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/paging.h>
#include <kos/types.h>
#include <fs/device.h>
#include <sys/io.h>

DECL_BEGIN

/* Based on this:
 *  >> http://cvs.savannah.nongnu.org/viewvc/*checkout*\
 *     /vgabios/vgabios/vbe_display_api.txt?revision=1.14
 */

#define VBE_DISPI_TOTAL_VIDEO_MEMORY_MB 8
#define VBE_DISPI_BANK_ADDRESS          0xa0000
#define VBE_DISPI_BANK_SIZE_KB          64

#define VBE_DISPI_MAX_XRES              1024
#define VBE_DISPI_MAX_YRES              768

#define VBE_DISPI_IOPORT_INDEX          0x01ce
#define VBE_DISPI_IOPORT_DATA           0x01cf

#define VBE_DISPI_INDEX_ID              0x0
#define VBE_DISPI_INDEX_XRES            0x1
#define VBE_DISPI_INDEX_YRES            0x2
#define VBE_DISPI_INDEX_BPP             0x3
#define VBE_DISPI_INDEX_ENABLE          0x4
#define VBE_DISPI_INDEX_BANK            0x5
#define VBE_DISPI_INDEX_VIRT_WIDTH      0x6
#define VBE_DISPI_INDEX_VIRT_HEIGHT     0x7
#define VBE_DISPI_INDEX_X_OFFSET        0x8
#define VBE_DISPI_INDEX_Y_OFFSET        0x9

#define VBE_DISPI_ID0                   0xb0c0
#define VBE_DISPI_ID1                   0xb0c1
#define VBE_DISPI_ID2                   0xb0c2
#define VBE_DISPI_ID3                   0xb0c3
#define VBE_DISPI_ID4                   0xb0c4
#define VBE_DISPI_ID_FUTURE_MAX         0xb0ff

#define VBE_DISPI_DISABLED              0x00
#define VBE_DISPI_ENABLED               0x01
#define VBE_DISPI_VBE_ENABLED           0x40
#define VBE_DISPI_NOCLEARMEM            0x80

#define VBE_DISPI_LFB_PHYSICAL_ADDRESS  0xe0000000

#define VBE_DISPI_BPP_4                 0x04
#define VBE_DISPI_BPP_8                 0x08
#define VBE_DISPI_BPP_15                0x0f
#define VBE_DISPI_BPP_16                0x10
#define VBE_DISPI_BPP_24                0x18
#define VBE_DISPI_BPP_32                0x20


/* VBE global variables. */
INTDEF vm_ppage_t      vbe_lfb;         /* [const] Physical page of the linear frame buffer. */
INTDEF atomic_rwlock_t vbe_lock;        /* VBE lock. */
INTDEF u16             vbe_id;          /* [const] The initial value of `VBE_DISPI_INDEX_ID' */
INTDEF u16             vbe_curr_xres;   /* [lock(vbe_lock)] The current value of `VBE_DISPI_INDEX_XRES' */
INTDEF u16             vbe_curr_yres;   /* [lock(vbe_lock)] The current value of `VBE_DISPI_INDEX_YRES' */
INTDEF u16             vbe_curr_bpp;    /* [lock(vbe_lock)] The current value of `VBE_DISPI_INDEX_BPP' */
INTDEF u16             vbe_curr_enable; /* [lock(vbe_lock)] The current value of `VBE_DISPI_INDEX_ENABLE' */


LOCAL u16 KCALL vbe_read(u16 index) {
 outw(VBE_DISPI_IOPORT_INDEX,index);
 return inw(VBE_DISPI_IOPORT_DATA);
}
LOCAL void KCALL vbe_write(u16 index, u16 value) {
 outw(VBE_DISPI_IOPORT_INDEX,index);
 outw(VBE_DISPI_IOPORT_DATA,value);
}


DECL_END

#endif /* !GUARD_KERNEL_MODULES_BOCHS_VBE_VBE_H */
