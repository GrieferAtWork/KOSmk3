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
#ifndef GUARD_KERNEL_MODULES_PROCFS_STRINGPRINTER_H
#define GUARD_KERNEL_MODULES_PROCFS_STRINGPRINTER_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>

DECL_BEGIN

struct stringprinter {
     char   *sp_bufpos; /* [1..1][>= sp_buffer][<= sp_bufend] . */
     char   *sp_buffer; /* [1..1] Allocate buffer base pointer. */
     char   *sp_bufend; /* [1..1] Buffer end (Pointer to currently allocated `'\0'´-character). */
};

INTDEF void KCALL StringPrinter_Init(struct stringprinter *__restrict self, size_t hint);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *KCALL StringPrinter_Pack(struct stringprinter *__restrict self);
INTDEF void KCALL StringPrinter_Fini(struct stringprinter *__restrict self);
INTDEF ssize_t KCALL StringPrinter_Print(char const *__restrict data, size_t datalen, void *closure);

DECL_END

#endif /* !GUARD_KERNEL_MODULES_PROCFS_STRINGPRINTER_H */
