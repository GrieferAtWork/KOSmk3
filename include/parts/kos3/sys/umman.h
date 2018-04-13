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
#ifndef _PARTS_KOS3_SYS_UMMAN_H
#define _PARTS_KOS3_SYS_UMMAN_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

#ifndef __KERNEL__
#ifdef __CRT_KOS

__REDIRECT_EXCEPT_UFSW16(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,shm_w16open,(char16_t const *__name, int __oflag, __mode_t __mode),shm_wopen,(__name,__oflag,__mode))
__REDIRECT_EXCEPT_UFSW32(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,shm_w32open,(char32_t const *__name, int __oflag, __mode_t __mode),shm_wopen,(__name,__oflag,__mode))
__REDIRECT_EXCEPT_UFSW16_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,shm_w16unlink,(char16_t const *__name),shm_wunlink,(__name))
__REDIRECT_EXCEPT_UFSW32_XVOID(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,shm_w32unlink,(char32_t const *__name),shm_wunlink,(__name))
#ifdef __USE_EXCEPT
__REDIRECT_W16(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,Xshm_w16open,(char16_t const *__name, int __oflag, __mode_t __mode),Xshm_wopen,(__name,__oflag,__mode))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY,int,__LIBCCALL,Xshm_w32open,(char32_t const *__name, int __oflag, __mode_t __mode),Xshm_wopen,(__name,__oflag,__mode))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xshm_w16unlink,(char16_t const *__name),Xshm_wunlink,(__name))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,Xshm_w32unlink,(char32_t const *__name),Xshm_wunlink,(__name))
#endif /* __USE_EXCEPT */

#endif /* __CRT_KOS */
#endif /* !__KERNEL__ */

__SYSDECL_END


#endif /* !_PARTS_KOS3_SYS_UMMAN_H */
