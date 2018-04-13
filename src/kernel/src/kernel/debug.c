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
#ifndef GUARD_KERNEL_SRC_KERNEL_DEBUG_C
#define GUARD_KERNEL_SRC_KERNEL_DEBUG_C 1

#include <hybrid/compiler.h>
#include <kernel/debug.h>
#include <stdarg.h>
#include <format-printer.h>

DECL_BEGIN

PUBLIC void KCALL
debug_vprintf(char const *__restrict format, va_list args) {
 format_vprintf(&debug_printer,NULL,format,args);
}
PUBLIC void KCALL
debug_printf(char const *__restrict format, ...) {
 va_list args;
 va_start(args,format);
 debug_vprintf(format,args);
 va_end(args);
}

DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_DEBUG_C */
