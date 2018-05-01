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
#ifndef GUARD_KERNEL_SRC_VM_REGION_IO_C
#define GUARD_KERNEL_SRC_VM_REGION_IO_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kernel/vm.h>
#include <assert.h>
#include <string.h>
#include <except.h>

DECL_BEGIN

PUBLIC size_t KCALL
vm_region_read(struct vm_region *__restrict self,
               uintptr_t addr, USER CHECKED void *buf,
               size_t bufsize, iomode_t mode) {
 /* TODO */
 error_throw(E_NOT_IMPLEMENTED);
}

PUBLIC size_t KCALL
vm_region_write(struct vm_region *__restrict self,
                uintptr_t addr, USER CHECKED void *buf,
                size_t bufsize, iomode_t mode,
                bool allow_if_shared) {
 /* TODO */
 error_throw(E_NOT_IMPLEMENTED);
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_VM_REGION_IO_C */
