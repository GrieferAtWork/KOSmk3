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
#ifndef GUARD_LIBS_LIBC_TLS_H
#define GUARD_LIBS_LIBC_TLS_H 1

#include "libc.h"
#include "sched.h"
#include <kos/thread.h>
#include <hybrid/compiler.h>
#include <hybrid/host.h>

#if defined(__i386__) || defined(__x86_64__)
#include <kos/intrin.h>
#include <stddef.h>
#ifdef __ASM_TASK_SEGMENT_ISGS
#define GET_DYNAMIC_TLS()  (struct dynamic_tls *)__readfsptr(offsetof(struct task_segment,ts_tls))
#define SET_DYNAMIC_TLS(v)  __writefsptr(offsetof(struct task_segment,ts_tls),v)
#else
#define GET_DYNAMIC_TLS()  (struct dynamic_tls *)__readgsptr(offsetof(struct task_segment,ts_tls))
#define SET_DYNAMIC_TLS(v)  __writegsptr(offsetof(struct task_segment,ts_tls),v)
#endif
#else
#define GET_DYNAMIC_TLS()  (libc_current()->ts_tls)
#define SET_DYNAMIC_TLS(v) (libc_current()->ts_tls = (v))
#endif


DECL_BEGIN


struct dynamic_tls {
    struct dynamic_tls *dt_next;    /* [0..1] Next dyn_tls segment. */
    uintptr_t           dt_module;  /* [const] Module handle to which this segment is bound. */
    byte_t              dt_data[1]; /* Segment TLS data. */
};




typedef struct {
    uintptr_t ti_moduleid;  /* [if(& 1, == MODULE_TLS_OFFSET << 1)]
                             * [if(!(& 1), == MODULE_HANDLE)] */
    uintptr_t ti_tlsoffset; /* Offset into the module's TLS segment. */
} TLS_index;

INTDEF void *FCALL libc_dynamic_tls_addr(TLS_index *__restrict index);
INTDEF void LIBCCALL libc_free_dynamic_tls(void);
INTDEF ATTR_NORETURN void LIBCCALL libc_exit_thread(int exit_code);


DECL_END

#endif /* !GUARD_LIBS_LIBC_TLS_H */
