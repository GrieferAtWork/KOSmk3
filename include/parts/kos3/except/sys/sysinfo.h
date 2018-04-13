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
#ifndef _PARTS_KOS3_EXCEPT_SYS_SYSINFO_H
#define _PARTS_KOS3_EXCEPT_SYS_SYSINFO_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

struct sysinfo;

__LIBC void (__LIBCCALL Xsysinfo)(struct sysinfo *__info);
__LIBC __WUNUSED unsigned int (__LIBCCALL Xget_nprocs_conf)(void);
__LIBC __WUNUSED unsigned int (__LIBCCALL Xget_nprocs)(void);
__LIBC __WUNUSED __UINTPTR_TYPE__ (__LIBCCALL Xget_phys_pages)(void);
__LIBC __WUNUSED __UINTPTR_TYPE__ (__LIBCCALL Xget_avphys_pages)(void);

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_SYS_SYSINFO_H */
