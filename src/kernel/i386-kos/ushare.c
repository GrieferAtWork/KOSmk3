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
#ifndef GUARD_KERNEL_I386_KOS_USHARE_C
#define GUARD_KERNEL_I386_KOS_USHARE_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/ushare.h>
#include <kernel/debug.h>
#include <kernel/ushare.h>
#include <kernel/vm.h>
#include <except.h>
#include <unwind/eh_frame.h>

DECL_BEGIN


INTDEF byte_t *x86_sysenter_ushare_base;
INTDEF byte_t kernel_ehframe_start[];
INTDEF byte_t kernel_ehframe_end[];
INTDEF byte_t kernel_ehframe_size[];


PRIVATE ssize_t KCALL
sysenter_pregion_ctl(struct vm_region *__restrict UNUSED(self),
                     unsigned int command,
                     uintptr_t address, void *arg) {
 bool result;
 if (command != REGION_CTL_FFIND_FDE)
     return 0;
 /* Find FDE information for the effectively mapped text. */
 result = eh_findfde(kernel_ehframe_start,
                    (size_t)kernel_ehframe_size,
                    (uintptr_t)x86_sysenter_ushare_base+address,
                    (struct fde_info *)arg,NULL);
 if (result) {
  ((struct fde_info *)arg)->fi_pcbegin -= (uintptr_t)x86_sysenter_ushare_base;
  ((struct fde_info *)arg)->fi_pcend   -= (uintptr_t)x86_sysenter_ushare_base;
 }
 return result;
}

INTDEF byte_t x86_ushare_sysenter_pageno[];
PRIVATE struct vm_region x86_syscall_region = {
    .vr_refcnt = 1,
    .vr_lock   = MUTEX_INIT,
    .vr_type   = VM_REGION_MEM,
    .vr_flags  = VM_REGION_FCANTSHARE|VM_REGION_FDONTMERGE,
    .vr_size   = 1,
    .vr_parts  = &x86_syscall_region.vr_part0,
    .vr_part0  = {
        .vp_refcnt = 1,
        .vp_chain  = { .le_pself = &x86_syscall_region.vr_parts },
        .vp_state  = VM_PART_INCORE,
        .vp_flags  = VM_PART_FNOSWAP|VM_PART_FKEEP|VM_PART_FWEAKREF,
        .vp_phys = {
            .py_num_scatter = 1,
            .py_iscatter = {
                [0] = {
                    .ps_addr = (uintptr_t)x86_ushare_sysenter_pageno,
                    .ps_size = 1
                }
            }
        }
    },
    .vr_ctl = &sysenter_pregion_ctl
};



/* Lookup a user-share segment, given its `name'.
 * @throw: E_INVALID_ARGUMENT: The given `name' does not refer to a known ushare segment. */
INTERN ATTR_RETNONNULL REF struct vm_region *
KCALL arch_ushare_lookup(u32 name) {
 switch (name) {

 case USHARE_X86_SYSCALL_FNAME:
  vm_region_incref(&x86_syscall_region);
  return &x86_syscall_region;

 default: break;
 }
 error_throw(E_INVALID_ARGUMENT);
}

DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_USHARE_C */
