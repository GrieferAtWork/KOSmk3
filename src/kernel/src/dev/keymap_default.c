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
#ifndef GUARD_KERNEL_SRC_DEV_KEYMAP_DEFAULT_C
#define GUARD_KERNEL_SRC_DEV_KEYMAP_DEFAULT_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kos/types.h>
#include <kos/keymap.h>
#include <kos/keyboard.h>
#include <kos/keyboard_ioctl.h>
#include <dev/keymap.h>
#include <fs/driver.h>
#include <assert.h>

DECL_BEGIN

PUBLIC ATTR_COLDBSS struct keyboard_keymap default_keymap;

/* Since decoded keyboard maps are quite large, save the default
 * map as a compiled keyboard map blob that is loaded at runtime.
 * That way, we can put the default map in .bss, and the blob, as
 * well as its initializer in .free:
 * $ deemon -DFILE=\"..\src\keymaps\en_US.txt\" scripts\keymap_compiler.dee > disk/en_US.kmp
 * $ deemon -F src\kernel\src\dev\keymap_default.c
 */
PRIVATE ATTR_FREERODATA byte_t const default_keymap_blob[] = {
/*[[[deemon
#include <file>
#include <fs>
fs.chdir(fs.path.head(__FILE__));
local data = file.open("../../../../disk/en_US.kmp").read();
local count = 0;
print "    ",;
for (local ch: data) {
    print "0x%.2x," % ((unsigned char)ch.ord()),;
    if ((count++ % 16) == 15) print "\n    ",;
}
]]]*/
    0x4b,0x6d,0x70,0x35,0x00,0x0e,0x00,0x00,0x65,0x6e,0x5f,0x55,0x53,0x00,0x00,0x00,
    0xfb,0x1c,0x7f,0xc1,0x1d,0x7e,0x1c,0x60,0x40,0x41,0x1d,0x40,0x42,0x43,0x44,0x45,
    0x1d,0x5e,0x46,0x1d,0x26,0x47,0x1d,0x2a,0x48,0x1d,0x28,0x48,0x1c,0x30,0x02,0x1d,
    0x5f,0x1c,0x2d,0x4c,0x1d,0x2b,0x1d,0x7c,0x1c,0x5c,0x1c,0x08,0xc2,0x1c,0x09,0x30,
    0x36,0x24,0x31,0x33,0x38,0x34,0x28,0x2e,0x2f,0x1d,0x7b,0x1c,0x5b,0x1d,0x7d,0x1c,
    0x5d,0x1d,0x0c,0x1c,0x0a,0x04,0x61,0x20,0x32,0x23,0x25,0x26,0x27,0x29,0x2a,0x2b,
    0x4a,0x1d,0x3a,0x41,0x1c,0x27,0x04,0x82,0x39,0x37,0x22,0x35,0x21,0x2d,0x2c,0x5b,
    0x1c,0x2c,0x02,0x5d,0x1c,0x2e,0x02,0x5e,0x1c,0x2f,0x04,0xa3,0x1c,0x20,0x04,0xc1,
    0x1c,0x2f,0x1c,0x2a,0x1c,0x2d,0x46,0x47,0x48,0x1c,0x2b,0x43,0x44,0x45,0x40,0x41,
    0x42,0x1c,0x0a,0x1c,0x30,0x1c,0x2e,0x00,
//[[[end]]]
};

DEFINE_DRIVER_PREINIT(default_keymap_initialize);
PRIVATE ATTR_USED void KCALL default_keymap_initialize(void) {
 assertef(keymap_load_blob(&default_keymap,(void *)default_keymap_blob),
          "Default keyboard map is corrupt");
}

DECL_END

#endif /* !GUARD_KERNEL_SRC_DEV_KEYMAP_DEFAULT_C */
