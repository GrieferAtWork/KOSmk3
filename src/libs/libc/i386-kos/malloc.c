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
#ifndef GUARD_LIBS_LIBC_I386_KOS_MALLOC_C
#define GUARD_LIBS_LIBC_I386_KOS_MALLOC_C 1
#define _GNU_SOURCE 1

#include "../libc.h"

DECL_BEGIN

typedef struct PACKED {
    u8   je_jmp;
    u32  je_reladdr;
} jmpent_t;

struct jmptab {
#define JMPTAB_CALLBACK(x) jmpent_t j_##x;
    LIBC_ENUMERATE_MALLOC_FUNCTIONS(JMPTAB_CALLBACK)
#undef JMPTAB_CALLBACK
};

INTDEF struct jmptab libc_malloc_jmptab;
INTDEF struct jmptab const libc_malloc_jmptab_d;

/* Must be called before any allocations are made in order
 * to redirect libc heap functions to their debug-counterparts. */
INTERN void LIBCCALL libc_set_debug_malloc(void) {
 libc_memcpy(&libc_malloc_jmptab,
             &libc_malloc_jmptab_d,
              sizeof(struct jmptab));
}


EXPORT(__set_debug_malloc,libc_set_debug_malloc);


DECL_END

#endif /* !GUARD_LIBS_LIBC_I386_KOS_MALLOC_C */
