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
#ifndef _PARTS_KOS3_EXCEPT_STDLIB_H
#define _PARTS_KOS3_EXCEPT_STDLIB_H 1

#include "__stdinc.h"
#include <features.h>
#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN


#if defined(__USE_MISC) || \
   (defined(__USE_XOPEN_EXTENDED) && !defined(__USE_XOPEN2K))
#ifndef __Xvalloc_defined
#define __Xvalloc_defined 1
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED __MALL_ATTR_PAGEALIGNED __ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL Xvalloc)(size_t __n_bytes);
#endif /* !__Xvalloc_defined */
#endif

#ifdef __USE_ISOC11
__REDIRECT(__LIBC,__ATTR_RETNONNULL __PORT_KOSONLY __WUNUSED
           __ATTR_ALLOC_ALIGN(1) __ATTR_ALLOC_SIZE((2))
           __ATTR_MALLOC,void *,__LIBCCALL,Xaligned_alloc,
          (size_t __alignment, size_t __n_bytes),Xmemalign,(__alignment,__n_bytes))
#endif /* __USE_ISOC11 */

#ifndef __Xsystem_defined
#define __Xsystem_defined 1
__LIBC int (__LIBCCALL Xsystem)(char const *__command);
#endif /* !__Xsystem_defined */

__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED char *(__LIBCCALL Xrealpath)(char const *__restrict __name, char *__resolved);
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
/* NOTE: I didn't come up with this function (https://docs.oracle.com/cd/E36784_01/html/E36874/frealpath-3c.html),
 *       but it seems to be something that GLibC isn't implementing for some reason...
 *       Because of that I didn't really know where to put this, so I put it in the
 *       same _SOURCE-block as its `realpath()' companion. */
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __WUNUSED char *(__LIBCCALL Xfrealpath)(__fd_t __fd, char *__resolved, size_t __bufsize);
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED  */

#ifdef __USE_GNU
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __NONNULL((1)) char *(__LIBCCALL Xcanonicalize_file_name)(char const *__name);
#endif /* __USE_GNU */

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_STDLIB_H */
