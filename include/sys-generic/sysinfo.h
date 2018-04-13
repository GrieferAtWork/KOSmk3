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
#ifndef _SYS_GENERIC_SYSINFO_H
#define _SYS_GENERIC_SYSINFO_H 1
#define _SYS_SYSINFO_H 1

#include <features.h>
#include <linux/kernel.h>

__SYSDECL_BEGIN

#ifndef __KERNEL__
__REDIRECT_EXCEPT_XVOID(__LIBC,,int,__LIBCCALL,sysinfo,(struct sysinfo *__info),(__info))
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(unsigned int,int),__LIBCCALL,get_nprocs_conf,(void),())
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(unsigned int,int),__LIBCCALL,get_nprocs,(void),())
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__UINTPTR_TYPE__,__INTPTR_TYPE__),__LIBCCALL,get_phys_pages,(void),())
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__EXCEPT_SELECT(__UINTPTR_TYPE__,__INTPTR_TYPE__),__LIBCCALL,get_avphys_pages,(void),())
#endif /* !__KERNEL__ */

__SYSDECL_END

#ifdef __USE_EXCEPT
#include <parts/kos3/except/sys/sysinfo.h>
#endif

#endif /* !_SYS_GENERIC_SYSINFO_H */
