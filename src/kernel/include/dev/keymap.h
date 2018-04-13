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
#ifndef GUARD_KERNEL_INCLUDE_DEV_KEYMAP_H
#define GUARD_KERNEL_INCLUDE_DEV_KEYMAP_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <stdbool.h>

DECL_BEGIN

struct keyboard_keymap;

/* Load a new keyboard map from `blob' and store it in `*result'.
 * @return: true:  Successfully read the keyboard map.
 * @return: false: The keyboard map is corrupt. (`*result' is undefined) */
FUNDEF bool KCALL
keymap_load_blob(struct keyboard_keymap *__restrict result,
                 USER CHECKED void *blob);

/* The default kernel keymap (layout for `en_US') */
DATDEF struct keyboard_keymap default_keymap;


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_DEV_KEYMAP_H */
