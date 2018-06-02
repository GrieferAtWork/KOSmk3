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
#include <i386-kos/vm86.h>
#include <kos/addr2line.h>
#include <string.h>

DECL_BEGIN


INTDEF byte_t *x86_sysenter_ushare_base;


PRIVATE char const
sysenter_names[USHARE_X86_SYSCALL_SYSENTER_COUNT][10] = {
    "sysenter0",
    "sysenter1",
    "sysenter2",
    "sysenter3",
    "sysenter4",
    "sysenter5",
    "sysenter6",
};

PRIVATE ssize_t KCALL
sysenter_pregion_ctl(struct vm_region *__restrict UNUSED(self),
                     unsigned int command,
                     uintptr_t address, void *arg) {
 switch (command) {

 {
  bool result;
 case REGION_CTL_FFIND_FDE:
  /* Find FDE information for the effectively mapped text. */
  result = kernel_eh_findfde((uintptr_t)x86_sysenter_ushare_base+address,
                             (struct fde_info *)arg);
  if (result) {
   ((struct fde_info *)arg)->fi_pcbegin -= (uintptr_t)x86_sysenter_ushare_base;
   ((struct fde_info *)arg)->fi_pcend   -= (uintptr_t)x86_sysenter_ushare_base;
  }
  return result;
 } break;

 {
  struct dl_addr2line *result;
 case REGION_CTL_FADDR2LINE:
  result = (struct dl_addr2line *)arg;
  memset(result,0,sizeof(struct dl_addr2line));
  result->d_file = "$$(KERNEL)";
  if (address < (USHARE_X86_SYSCALL_SYSENTER_COUNT*
                 USHARE_X86_SYSCALL_SYSENTER_STRIDE)) {
   uintptr_t index;
   index = address / USHARE_X86_SYSCALL_SYSENTER_STRIDE;
   result->d_begin = (void *)(index * USHARE_X86_SYSCALL_SYSENTER_STRIDE);
   result->d_end   = (void *)((index+1) * USHARE_X86_SYSCALL_SYSENTER_STRIDE);
   result->d_name  = sysenter_names[index];
  } else {
   result->d_name  = "syscall";
   result->d_begin = (void *)USHARE_X86_SYSCALL_OFFSETOF_SYSCALL;
   result->d_end   = (void *)USHARE_X86_SYSCALL_FSIZE;
  }
  return true;
 } break;

 default: break;
 }
 return 0;
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


#ifdef __x86_64__
INTDEF byte_t x86_ushare_sysenter_compat_pageno[];
PRIVATE struct vm_region x86_syscall_region_compat = {
    .vr_refcnt = 1,
    .vr_lock   = MUTEX_INIT,
    .vr_type   = VM_REGION_MEM,
    .vr_flags  = VM_REGION_FCANTSHARE|VM_REGION_FDONTMERGE,
    .vr_size   = 1,
    .vr_parts  = &x86_syscall_region_compat.vr_part0,
    .vr_part0  = {
        .vp_refcnt = 1,
        .vp_chain  = { .le_pself = &x86_syscall_region_compat.vr_parts },
        .vp_state  = VM_PART_INCORE,
        .vp_flags  = VM_PART_FNOSWAP|VM_PART_FKEEP|VM_PART_FWEAKREF,
        .vp_phys = {
            .py_num_scatter = 1,
            .py_iscatter = {
                [0] = {
                    .ps_addr = (uintptr_t)x86_ushare_sysenter_compat_pageno,
                    .ps_size = 1
                }
            }
        }
    },
    .vr_ctl = &sysenter_pregion_ctl
};
#endif /* __x86_64__ */



/* Lookup a user-share segment, given its `name'.
 * @throw: E_INVALID_ARGUMENT: The given `name' does not refer to a known ushare segment. */
INTERN ATTR_RETNONNULL REF struct vm_region *
KCALL arch_ushare_lookup(u32 name) {
 switch (name) {


#ifdef __x86_64__
 {
  REF struct vm_region *result;
 case USHARE_X86_SYSCALL_FNAME:
  result = &x86_syscall_region;
  /* Load the compatibility syscall segment
   * if the calling program is 32-bit. */
  if (interrupt_iscompat())
      result = &x86_syscall_region_compat;
  vm_region_incref(result);
  return result;
 }

 case USHARE_X86_SYSCALL32_FNAME:
  vm_region_incref(&x86_syscall_region);
  return &x86_syscall_region;
 case USHARE_X86_SYSCALL64_FNAME:
  vm_region_incref(&x86_syscall_region_compat);
  return &x86_syscall_region_compat;
#else
 case USHARE_X86_SYSCALL_FNAME:
  vm_region_incref(&x86_syscall_region);
  return &x86_syscall_region;
#endif

#ifdef CONFIG_VM86
 case USHARE_X86_VM86BIOS_FNAME:
  vm_region_incref(&vm86_identity_1mb);
  return &vm86_identity_1mb;
#endif /* CONFIG_VM86 */

 default: break;
 }
 error_throw(E_INVALID_ARGUMENT);
}

DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_USHARE_C */
