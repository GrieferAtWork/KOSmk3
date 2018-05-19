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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_TLS_H
#define GUARD_KERNEL_INCLUDE_KERNEL_TLS_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/vm.h>

DECL_BEGIN

#ifdef __CC__

typedef VIRT ptrdiff_t tls_addr_t;

FUNDEF uintptr_t KCALL
__tls_alloc_impl(USER CHECKED void *template_base,
                 size_t template_size, size_t num_bytes,
                 size_t tls_alignment) ASMNAME("tls_alloc_impl");
FUNDEF ATTR_NOTHROW void KCALL __tls_free_impl(uintptr_t base) ASMNAME("tls_free_impl");
FUNDEF void KCALL __tls_init_impl(uintptr_t base, USER CHECKED void *template_base, size_t template_size, size_t num_bytes) ASMNAME("tls_init_impl");



/* Allocate / Free static TLS memory in the current VM.
 * When initializing for a new thread, the first MIN(template_size,num_bytes) are
 * copied from the provided template, while the remainder is ZERO-initialized.
 * @assume(vm_holding(THIS_VM));
 * @assume(num_bytes != 0);
 * @throw: E_BADALLOC: Failed to allocate static TLS memory in all threads. */
FORCELOCAL tls_addr_t KCALL
tls_alloc(USER CHECKED void *template_base,
          size_t template_size, size_t num_bytes,
          size_t tls_alignment) {
 return -(tls_addr_t)__tls_alloc_impl(template_base,template_size,
                                      num_bytes,tls_alignment);
}

/* Free a previously allocated TLS segment, given its end address. */
FORCELOCAL ATTR_NOTHROW void KCALL tls_free(tls_addr_t addr) {
 __tls_free_impl((uintptr_t)-addr);
}

/* Initialize the per-thread TLS segment associated with `addr',
 * using its template as origin of data, in all threads running
 * within the current VM.
 * @assume(vm_holding(THIS_VM)); */
FORCELOCAL void KCALL
tls_init(tls_addr_t addr, USER CHECKED void *template_base,
         size_t template_size, size_t num_bytes) {
 __tls_init_impl((uintptr_t)-addr,template_base,template_size,num_bytes);
}


#endif /* __CC__ */


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_TLS_H */
