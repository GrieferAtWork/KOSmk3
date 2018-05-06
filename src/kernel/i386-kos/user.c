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

PUBLIC void KCALL
validate_readable(UNCHECKED USER void const *base, size_t num_bytes) {
 uintptr_t addr_end,limit = PERTASK_GET(this_task.t_addrlimit);
 if unlikely(__builtin_add_overflow((uintptr_t)base,num_bytes,&addr_end)) {
  if unlikely((uintptr_t)base > limit)
     error_throwf(E_SEGFAULT,0,(void *)base);
  error_throwf(E_SEGFAULT,0,(void *)limit);
 }
 if unlikely(addr_end > limit && num_bytes)
    error_throwf(E_SEGFAULT,0,(void *)addr_end);
}
PUBLIC void KCALL
validate_readablem(UNCHECKED USER void const *base, size_t num_items, size_t item_size_in_bytes) {
 uintptr_t addr_end,limit = PERTASK_GET(this_task.t_addrlimit);
 if unlikely(__builtin_mul_overflow(num_items,item_size_in_bytes,&num_items) ||
             __builtin_add_overflow((uintptr_t)base,num_items,&addr_end)) {
  if unlikely((uintptr_t)base > limit)
     error_throwf(E_SEGFAULT,0,(void *)base);
  error_throwf(E_SEGFAULT,0,(void *)limit);
 }
 if unlikely(addr_end > limit && num_items)
    error_throwf(E_SEGFAULT,0,(void *)addr_end);
}
PUBLIC void KCALL
validate_writable(UNCHECKED USER void *base, size_t num_bytes) {
 uintptr_t addr_end,limit = PERTASK_GET(this_task.t_addrlimit);
 if unlikely(__builtin_add_overflow((uintptr_t)base,num_bytes,&addr_end)) {
  if unlikely((uintptr_t)base > limit)
     error_throwf(E_SEGFAULT,X86_SEGFAULT_FWRITE,(void *)base);
  error_throwf(E_SEGFAULT,X86_SEGFAULT_FWRITE,(void *)limit);
 }
 if (num_bytes) {
  if unlikely(addr_end > limit)
     error_throwf(E_SEGFAULT,X86_SEGFAULT_FWRITE,(void *)addr_end);
  /* Ensure cow-writability on the address range. */
  vm_cow(base,num_bytes);
 }
}
PUBLIC void KCALL
validate_writablem(UNCHECKED USER void *base, size_t num_items, size_t item_size_in_bytes) {
 uintptr_t addr_end,limit = PERTASK_GET(this_task.t_addrlimit);
 if unlikely(__builtin_mul_overflow(num_items,item_size_in_bytes,&num_items) ||
             __builtin_add_overflow((uintptr_t)base,num_items,&addr_end)) {
  if unlikely((uintptr_t)base > limit)
     error_throwf(E_SEGFAULT,X86_SEGFAULT_FWRITE,(void *)base);
  error_throwf(E_SEGFAULT,X86_SEGFAULT_FWRITE,(void *)limit);
 }
 if (num_items) {
  if unlikely(addr_end > limit)
     error_throwf(E_SEGFAULT,X86_SEGFAULT_FWRITE,(void *)addr_end);
  /* Ensure cow-writability on the address range. */
  vm_cow(base,num_items);
 }
}
PUBLIC void KCALL
validate_executable(UNCHECKED USER void const *base) {
 if unlikely((uintptr_t)base >= PERTASK_GET(this_task.t_addrlimit))
    error_throwf(E_SEGFAULT,X86_SEGFAULT_FEXEC,(void *)base);
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
    error_throwf(E_SEGFAULT,0,(void *)base);
 return strlen(base);
}


/* Since KOS on x86 uses a high-kernel, `NULL'
 * is implicitly a valid user-space pointer. */
DEFINE_PUBLIC_ALIAS(validate_user,validate_readable);
DEFINE_PUBLIC_ALIAS(validate_readable_opt,validate_readable);
DEFINE_PUBLIC_ALIAS(validate_writable_opt,validate_writable);
DEFINE_PUBLIC_ALIAS(validate_readablem_opt,validate_readablem);
DEFINE_PUBLIC_ALIAS(validate_writablem_opt,validate_writablem);
DEFINE_PUBLIC_ALIAS(validate_executable_opt,validate_executable);


DECL_END

#endif /* !GUARD_KERNEL_I386_KOS_USER_C */
