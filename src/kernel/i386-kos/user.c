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
#ifndef GUARD_KERNEL_I386_KOS_USER_C
#define GUARD_KERNEL_I386_KOS_USER_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <kernel/user.h>
#include <kernel/paging.h>
#include <kernel/vm.h>
#include <sched/task.h>
#include <string.h>
#include <except.h>

DECL_BEGIN

INTERN ATTR_NORETURN void KCALL
throw_segfault(VIRT void *addr, uintptr_t reason) {
 struct exception_info *info;
 info = error_info();
 memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
 /* Construct an segmentation fault error. */
 info->e_error.e_segfault.sf_vaddr  = addr;
 info->e_error.e_segfault.sf_reason = reason;
 info->e_error.e_code               = E_SEGFAULT;
 info->e_error.e_flag               = ERR_FNORMAL;
 error_throw_current();
 __builtin_unreachable();
}

PUBLIC void KCALL
validate_readable(UNCHECKED USER void const *base, size_t num_bytes) {
 uintptr_t addr_end,limit = PERTASK_GET(this_task.t_addrlimit);
 if unlikely(__builtin_add_overflow((uintptr_t)base,num_bytes,&addr_end)) {
  if unlikely((uintptr_t)base > limit)
     throw_segfault((void *)base,0);
  throw_segfault((void *)limit,0);
 }
 if unlikely(addr_end > limit && num_bytes)
    throw_segfault((void *)addr_end,0);
}
PUBLIC void KCALL
validate_writable(UNCHECKED USER void *base, size_t num_bytes) {
 uintptr_t addr_end,limit = PERTASK_GET(this_task.t_addrlimit);
 if unlikely(__builtin_add_overflow((uintptr_t)base,num_bytes,&addr_end)) {
  if unlikely((uintptr_t)base > limit)
     throw_segfault((void *)base,X86_SEGFAULT_FWRITE);
  throw_segfault((void *)limit,X86_SEGFAULT_FWRITE);
 }
 if unlikely(addr_end > limit && num_bytes)
    throw_segfault((void *)addr_end,X86_SEGFAULT_FWRITE);
 /* Ensure cow-writability on the address range. */
 vm_cow(base,num_bytes);
}
PUBLIC void KCALL
validate_executable(UNCHECKED USER void const *base) {
 if unlikely((uintptr_t)base >= PERTASK_GET(this_task.t_addrlimit))
    throw_segfault((void *)base,X86_SEGFAULT_FEXEC);
}

/* Search for the end of the given user-space string.
 * Search stops when an unmapped page is encountered, or if
 * the string extends into memory that the user does not have
 * access permissions for.
 * Upon such an error, an `E_SEGFAULT' is thrown. Otherwise,
 * the length of the string (in bytes) is returned. */
PUBLIC size_t KCALL
user_strlen(UNCHECKED USER char const *base) {
 if unlikely((uintptr_t)base >= PERTASK_GET(this_task.t_addrlimit))
    throw_segfault((void *)base,0);
 return strlen(base);
}


/* Since KOS on x86 uses a high-kernel, `NULL'
 * is implicitly a valid user-space pointer. */
DEFINE_PUBLIC_ALIAS(validate_user,validate_readable);
DEFINE_PUBLIC_ALIAS(validate_readable_opt,validate_readable);
DEFINE_PUBLIC_ALIAS(validate_writable_opt,validate_writable);
DEFINE_PUBLIC_ALIAS(validate_executable_opt,validate_executable);


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_USER_C */
