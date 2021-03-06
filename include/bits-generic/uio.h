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
#ifndef _BITS_GENERIC_UIO_H
#define _BITS_GENERIC_UIO_H 1
#define _BITS_UIO_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif

#ifndef UIO_MAXIOV
#define UIO_MAXIOV  1024
#endif /* !UIO_MAXIOV */

#ifndef __iovec_defined
#define __iovec_defined 1
struct iovec {
    void  *iov_base; /* Pointer to data. */
    size_t iov_len;  /* Length of data. */
};
#endif /* !__iovec_defined */

__SYSDECL_END

#endif /* !_BITS_GENERIC_UIO_H */
