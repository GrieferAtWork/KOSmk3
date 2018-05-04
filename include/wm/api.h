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
#ifndef _WM_API_H
#define _WM_API_H 1

#include <__stdinc.h>
#include <hybrid/compiler.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN


/*
 * ==== GENERIC NOTES ON THE WM API ====
 *
 *   - To make use of WM, link with `-lwm'
 *
 *   - The WM (WindowManager) API makes use of KOS
 *     exceptions, rather than POSIX errno values.
 *     Additionally, there is no link option to emulate
 *     posix behavior by translating errors to errno values.
 *     If such a behavior is intended, the library user must
 *     implement it themself by making use of `except_errno()'
 *
 *   - The WM API uses a user-space display server application
 *     which is communicated with using a combination of pipes
 *     and shared memory.
 *
 *   - KOS's WM is designed to operate as a draw-on-demand
 *     window manager, with every window's surface doubling as
 *     a back-buffer from which data is only rendered when the
 *    `wm_window_draw*()' family of functions is invoked, or
 *     when another window that was located in front of another
 *     one that was previously obscured is moved out of the way.
 *     The WM server keeps track of which parts of the screen
 *     are drawn by which window and will copy only visible parts
 *     if those parts changed.
 *     With that in mind, data from the window's surface will only
 *     be copied when `wm_window_draw*()' is called, or in certain
 *     race conditions that are not really worth mentioning, related
 *     to the user moving around windows.
 *
 *   - The WM server is able to detect user-application termination
 *     when the WM command pipe opened by every hosted application
 *     is closed, at which point it will close all the windows opened
 *     by that application, and free all resources still associated
 *     with it.
 *     HINT: This pipe connection is what is established when
 *          `wm_init()' is called.
 *
 *   - The WM API is designed to provide a low-level graphics
 *     interface including only the bare minimum for what is
 *     required to create a window that can co-exist with any
 *     number of other windows, potentially created by other
 *     applications.
 *     Support for GUI elements like buttons is then built
 *     on top of it, rather than the other way around.
 *
 *   - You might find the WM API particularly easy to use
 *     if you've ever worked with SDL.
 *
 */

#ifdef __BUILDING_LIBWM
#define WMAPI     EXPDEF
#else
#define WMAPI     IMPDEF
#endif
#define WMCALL    ATTR_STDCALL

#ifdef __CC__

/* Initialize / finalize the WM-API. */
WMAPI void WMCALL wm_init(void);
WMAPI ATTR_NOTHROW void WMCALL wm_fini(void);


typedef __UINTPTR_TYPE__ wms_window_id_t;

#endif /* __CC__ */

__SYSDECL_END

#endif /* !_WM_API_H */
