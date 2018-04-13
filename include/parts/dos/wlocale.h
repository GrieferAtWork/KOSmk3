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
#ifndef _PARTS_DOS_WLOCALE_H
#define _PARTS_DOS_WLOCALE_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <xlocale.h>

__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef _WLOCALE_DEFINED
#define _WLOCALE_DEFINED 1
__LIBC __PORT_DOSONLY wchar_t *(__LIBCCALL _wsetlocale)(int __category, wchar_t const *__locale);
__LIBC __PORT_DOSONLY __locale_t (__LIBCCALL _wcreate_locale)(int __category, wchar_t const *__locale);
#endif /* !_WLOCALE_DEFINED */

__SYSDECL_END

#endif /* !_PARTS_DOS_WLOCALE_H */
