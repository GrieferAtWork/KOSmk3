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
#ifndef GUARD_APPS_WMS_BIND_H
#define GUARD_APPS_WMS_BIND_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <wm/api.h>

DECL_BEGIN

#define WMS_PATH_KEYBOARD "/dev/keyboard"
#define WMS_PATH_MOUSE    "/dev/mouse"
#define WMS_PATH_SERVER   "/dev/wms"
#define WMS_PATH_DISPLAY  "/dev/vga" /* TODO: Must re-implement the VGA driver first! */

/* Filesystem bindings. */
INTDEF fd_t wms_keyboard; /* open(WMS_PATH_KEYBOARD); */
INTDEF fd_t wms_mouse;    /* open(WMS_PATH_MOUSE); */
INTDEF fd_t wms_server;   /* socket(AF_UNIX); bind(WMS_PATH_SERVER); */

/* WMS threads. */
INTDEF fd_t wms_keyboard; /* open(WMS_PATH_KEYBOARD); */


DECL_END

#endif /* !GUARD_APPS_WMS_BIND_H */
