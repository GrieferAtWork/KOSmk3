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
#ifndef _PARTS_KOS3_FORMATPRINTER_H
#define _PARTS_KOS3_FORMATPRINTER_H 1

#include "__stdinc.h"
#include <features.h>
#include <bits/types.h>
#include <hybrid/typecore.h>
#include <parts/kos2/pformatprinter.h>

#if __KOS_VERSION__ >= 300 && defined(__CRT_KOS)
__SYSDECL_BEGIN

/* Repeat `CH' a number of `NUM_REPETITIONS' times.
 * The usual format-printer rules apply, and this function
 * is allowed to call `PRINTER' as often as it chooses. */
__LIBC __ssize_t (__LIBCCALL format_repeat)(pformatprinter __printer, void *__closure,
                                            char __ch, __size_t __num_repetitions);

__SYSDECL_END
#endif /* __KOS_VERSION__ >= 300 && __CRT_KOS */

#endif /* !_PARTS_KOS3_FORMATPRINTER_H */
