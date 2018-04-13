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
#ifndef _UCHAR_H
#define _UCHAR_H 1

#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/mbstate.h>
#ifdef __USE_UTF
#include <xlocale.h>
#include "parts/kos2/malldefs.h"
#endif /* __USE_UTF */

__SYSDECL_BEGIN

/* Define `size_t' */
#ifndef __std_size_t_defined
#define __std_size_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __SIZE_TYPE__ size_t;
__NAMESPACE_STD_END
#endif /* !__std_size_t_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __size_t_defined
#define __size_t_defined 1
__NAMESPACE_STD_USING(size_t)
#endif /* !__size_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

/* Libc uses utf16/utf32 to encode/decode char16_t and char32_t */
#define __STD_UTF_16__ 1
#define __STD_UTF_32__ 1

#ifndef __KERNEL__
#ifdef __CRT_GLC
__LIBC __PORT_NODOS size_t __NOTHROW((__LIBCCALL mbrtoc16)(char16_t *__restrict __pc16, char const *__restrict __s, size_t __n, __mbstate_t *__restrict __p));
__LIBC __PORT_NODOS size_t __NOTHROW((__LIBCCALL mbrtoc32)(char32_t *__restrict __pc32, char const *__restrict __s, size_t __n, __mbstate_t *__restrict __p));
__LIBC __PORT_NODOS size_t __NOTHROW((__LIBCCALL c16rtomb)(char *__restrict __s, char16_t __c16, __mbstate_t *__restrict __ps));
__LIBC __PORT_NODOS size_t __NOTHROW((__LIBCCALL c32rtomb)(char *__restrict __s, char32_t __c32, __mbstate_t *__restrict __ps));
#endif /* __CRT_GLC */

#endif /* !__KERNEL__ */

__SYSDECL_END

#ifdef __USE_UTF

#ifdef _STRING_H
#ifndef _PARTS_KOS2_USTRING_H
#include "parts/kos2/ustring.h"
#endif
#endif
#ifdef _PROCESS_H
#ifndef _PARTS_KOS2_UPROCESS_H
#include "parts/kos2/uprocess.h"
#endif
#endif
#ifdef _STDLIB_H
#ifndef _PARTS_KOS2_USTDLIB_H
#include "parts/kos2/ustdlib.h"
#endif
#endif
#ifdef _FCNTL_H
#ifndef _PARTS_KOS2_UFCNTL_H
#include "parts/kos2/ufcntl.h"
#endif
#endif
#ifdef _UNISTD_H
#ifndef _PARTS_KOS2_UUNISTD_H
#include "parts/kos2/uunistd.h"
#endif
#endif
#ifdef _FORMAT_PRINTER_H
#ifndef _PARTS_KOS2_UFORMATPRINTER_H
#include "parts/kos2/uformatprinter.h"
#endif
#endif
#ifdef _IO_H
#ifndef _PARTS_KOS2_UIO_H
#include "parts/kos2/uio.h"
#endif
#endif

#ifdef __USE_KOS3
#ifdef _STRING_H
#ifndef _PARTS_KOS3_USTRING_H
#include "parts/kos3/ustring.h"
#endif
#endif
#ifdef _PROCESS_H
#ifndef _PARTS_KOS3_UPROCESS_H
#include "parts/kos3/uprocess.h"
#endif
#endif
#ifdef _STDLIB_H
#ifndef _PARTS_KOS3_USTDLIB_H
#include "parts/kos3/ustdlib.h"
#endif
#endif
#ifdef _STDIO_H
#ifndef _PARTS_KOS3_USTDIO_H
#include "parts/kos3/ustdio.h"
#endif
#endif
#ifdef _UNISTD_H
#ifndef _PARTS_KOS3_UUNISTD_H
#include "parts/kos3/uunistd.h"
#endif
#endif
#ifdef _FORMAT_PRINTER_H
#ifndef _PARTS_KOS3_UFORMATPRINTER_H
#include "parts/kos3/uformatprinter.h"
#endif
#endif
#ifdef _SYS_STAT_H
#ifndef _PARTS_KOS3_SYS_USTAT_H
#include "parts/kos3/sys/ustat.h"
#endif
#endif
#ifdef _SYS_MMAN_H
#ifndef _PARTS_KOS3_SYS_UMMAN_H
#include "parts/kos3/sys/umman.h"
#endif
#endif
#endif /* __USE_KOS3 */

#ifdef __DOCGEN__
#include "parts/kos2/uprocess.h"
#include "parts/kos2/ustdlib.h"
#include "parts/kos2/ustring.h"
#include "parts/kos2/uformatprinter.h"
#include "parts/kos2/ufcntl.h"
#include "parts/kos2/uunistd.h"
#include "parts/kos2/uio.h"
#include "parts/kos3/uprocess.h"
#include "parts/kos3/ustdlib.h"
#include "parts/kos3/ustdio.h"
#include "parts/kos3/ustring.h"
#include "parts/kos3/uunistd.h"
#include "parts/kos3/uformatprinter.h"
#include "parts/kos3/sys/ustat.h"
#include "parts/kos3/sys/umman.h"
#endif /* __DOCGEN__ */

#endif /* __USE_UTF */


#endif /* !_UCHAR_H */
