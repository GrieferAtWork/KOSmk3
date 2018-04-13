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
#ifndef _PROCESS_H
#define _PROCESS_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>
#include <parts/redirect-exec.h>

/* DOS Header. */

__SYSDECL_BEGIN

/* `MODE' argument values for spawn() functions. */
#define P_WAIT          0
#define P_NOWAIT        1
#define P_OVERLAY       2
#define P_NOWAITO       3
#define P_DETACH        4

/* `ACTION' argument values for cwait() functions. */
#define WAIT_CHILD      0
#define WAIT_GRANDCHILD 1

__SYSDECL_END

#ifdef __USE_KOS
#ifndef _PARTS_KOS2_PROCESS_H
#include <parts/kos2/process.h>
#endif
#ifdef _WCHAR_H
#ifndef _PARTS_KOS2_WPROCESS_H
#include <parts/kos2/wprocess.h>
#endif
#endif
#endif /* __USE_KOS */

#ifdef __USE_KOS3
#ifndef _PARTS_KOS3_PROCESS_H
#include <parts/kos3/process.h>
#endif
#ifdef _WCHAR_H
#ifndef _PARTS_KOS3_WPROCESS_H
#include <parts/kos3/wprocess.h>
#endif
#endif
#endif /* __USE_KOS3 */

#ifdef __USE_DOS
#ifndef _PARTS_DOS_PROCESS_H
#include <parts/dos/process.h>
#endif
#endif /* __USE_DOS */

#endif /* !_PROCESS_H */
