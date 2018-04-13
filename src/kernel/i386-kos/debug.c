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
#ifndef GUARD_KERNEL_I386_KOS_DEBUG_C
#define GUARD_KERNEL_I386_KOS_DEBUG_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <format-printer.h>
#include <sys/io.h>

DECL_BEGIN

INTERN u16 debug_ioport = 0x3f8;

/* Print strings to an externally connected debug device. */
PUBLIC void KCALL
debug_print(char const *__restrict data, size_t datalen) {
 outsb(debug_ioport,data,datalen);
}

PUBLIC ssize_t KCALL
debug_printer(char const *__restrict data,
              size_t datalen, void *closure) {
 debug_print(data,datalen);
 return (ssize_t)datalen;
}


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_DEBUG_C */
