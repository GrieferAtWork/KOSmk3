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
#ifndef GUARD_KERNEL_I386_KOS_LINKER_C
#define GUARD_KERNEL_I386_KOS_LINKER_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <kernel/user.h>
#include <fs/linker.h>
#include <unwind/eh_frame.h>
#include <unwind/linker.h>
#include <syscall.h>
#include <kos/thread.h>
#include <string.h>

#include "posix_signals.h"

DECL_BEGIN

PRIVATE void KCALL
push_function(module_callback_t func,
              void ***__restrict pesp) {
 /* pushl %eip */
 *--(*pesp) = (void *)func;
}


PUBLIC void KCALL
application_loaduserinit(struct application *__restrict app,
                         struct cpu_hostcontext_user *__restrict context) {
 void **psp = (void **)context->c_psp;
 validate_writable(psp-1,sizeof(*psp));
 *--psp = (void *)context->c_pip;     /* pushp %pip */
 application_enuminit(app,            /* pushp init... */
                     (module_enumerator_t)&push_function,
                      &psp);
 context->c_pip = (uintptr_t)*psp++;  /* popp  %pip */
 context->c_psp = (uintptr_t)psp;
}
PUBLIC void KCALL
application_loaduserfini(struct application *__restrict app,
                         struct cpu_hostcontext_user *__restrict context) {
 void **psp = (void **)context->c_psp;
 validate_writable(psp-1,sizeof(*psp));
 *--psp = (void *)context->c_pip;     /* pushp %pip */
 application_enumfini(app,            /* pushp fini... */
                     (module_enumerator_t)&push_function,
                      &psp);
 context->c_pip = (uintptr_t)*psp++;  /* popp  %pip */
 context->c_psp = (uintptr_t)psp;
}

DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_LINKER_C */
