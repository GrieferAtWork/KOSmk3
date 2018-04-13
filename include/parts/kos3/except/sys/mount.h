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
#ifndef _PARTS_KOS3_EXCEPT_SYS_MOUNT_H
#define _PARTS_KOS3_EXCEPT_SYS_MOUNT_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

__LIBC __WARN_NODOSFS __PORT_KOSONLY
void (__LIBCCALL Xmount)(char const *__special_file, char const *__dir,
                         char const *__fstype, unsigned long int __rwflag,
                         void const *__data);

__LIBC __WARN_NODOSFS __PORT_KOSONLY
void (__LIBCCALL Xumount)(char const *__special_file);

__LIBC __WARN_NODOSFS __PORT_KOSONLY
void (__LIBCCALL Xumount2)(char const *__special_file, int __flags);

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SYS_MOUNT_H */
