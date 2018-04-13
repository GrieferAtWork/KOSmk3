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
#ifndef _PARTS_KOS2_PFORMATPRINTER_H
#define _PARTS_KOS2_PFORMATPRINTER_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>

__SYSDECL_BEGIN

#ifndef __pformatprinter_defined
#define __pformatprinter_defined 1
/* Callback functions prototypes provided to format functions.
 * NOTE: 'pformatprinter' usually returns the number of characters printed, but isn't required to.
 * @param: DATA:    The base address of a DATALEN bytes long character vector that should be printed.
 * @param: DATALEN: The amount of characters that should be printed, starting at `data'.
 *                  Note that this is an exact value, meaning that a NUL-character appearing
 *                  before then should not terminate printing prematurely, but be printed as well.
 * @param: CLOSURE: The user-defined closure parameter passed alongside this function pointer.
 * @return: < 0:    An error occurred and the calling function shall return with this same value.
 * @return: >= 0:   The print was successful.
 *                  Usually, the return value is added to a sum of values which is then
 *                  returned by the calling function upon success, also meaning that the
 *                  usual return value used to indicate success in 'DATALEN'. */
typedef __ssize_t (__LIBCCALL *pformatprinter)(char const *__restrict __data,
                                               __size_t __datalen, void *__closure);
/* Read and return one character.
 * @return: >= 0:  The character that was read.
 * @return: EOF:   The input stream has ended.
 * @return: < EOF: An error occurred (Return the same value to the caller) */
typedef __ssize_t (__LIBCCALL *pformatgetc)(void *__closure);
typedef __ssize_t (__LIBCCALL *pformatungetc)(int __ch, void *__closure);
#endif /* !__pformatprinter_defined */

__SYSDECL_END

#endif /* !_PARTS_KOS2_PFORMATPRINTER_H */
