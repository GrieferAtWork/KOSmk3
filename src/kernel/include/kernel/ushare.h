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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_USHARE_H
#define GUARD_KERNEL_INCLUDE_KERNEL_USHARE_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/ushare.h>

DECL_BEGIN

/* USHARE:
 *    - A special set of memory regions that user application can
 *      explicitly request the kernel to map into their address space.
 *    - The contents of the regions are pre-defined by the kernel itself,
 *      and usually either contain special runtime information, function
 *      assembly that may prove useful to user-space, or (most importantly)
 *      the preferred mechanism for performing a system call.
 * Using USHARE memory segments, `libc' implements its system call mechanism as follows:
 * >> .section .text
 * >>     ...
 * >> x86_syscall_load_3:
 * >>     pushl  %eax            // Save the system call number
 * >>     call   x86_get_syscall_segment
 * >>     addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(3)), %eax
 * >>     jmp    .x86_syscall_load_common
 * >> x86_syscall_load_4:
 * >>     pushl  %eax            // Save the system call number
 * >>     call   x86_get_syscall_segment
 * >>     addl   $(USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(4)), %eax
 * >>     jmp    .x86_syscall_load_common
 * >>     ...
 * >> .x86_syscall_load_common:
 * >>     movl   4(%esp),  %ecx
 * >>     subl   %ecx,     %eax  // Make the syscall entry address relative
 * >>     movl   %eax,  -4(%ecx) // Override the call instruction's target
 * >>     movb   $0xe9, -4(%ecx) // Turn the `call' instruction into a `jmp'
 * >>                            // NOTE: Because of the `ret', it's OK if another thread
 * >>                            //       executed the `call' before we did this.
 * >>     popl   %eax            // Restore the system call number
 * >>     subl   $5, %ecx        // Load the start of the (now) `jmp' instruction
 * >>     addl   $4, %esp        // Pop the return address
 * >>     jmp    *%ecx           // Re-execute the (now) `jmp' instruction
 * >> 
 * >> .section .data // .data because we re-write this code later
 * >> sys_openat:
 * >>     movl   $__NR_openat, %eax
 * >>     call   x86_syscall_load_4
 * >>     ret
 * C:
 * >> uintptr_t x86_syscall_segment = (uintptr_t)-1;
 * >> uintptr_t x86_get_syscall_segment(void) {
 * >>     struct mmap_info info;
 * >>     uintptr_t new_address,old_address;
 * >>     if (x86_syscall_segment != (uintptr_t)-1)
 * >>         return x86_syscall_segment;
 * >>     
 * >>     ...
 * >>     info.mi_xflag           = XMAP_USHARE;
 * >>     info.mi_size            = USHARE_X86_SYSCALL_FSIZE;
 * >>     info.mi_ushare.mu_name  = USHARE_X86_SYSCALL_FNAME;
 * >>     info.mi_ushare.mu_start = 0;
 * >>     __asm__ __volatile__("int $0x80"
 * >>                          : "=a" (new_address)
 * >>                          : "a" (__NR_xmmap)
 * >>                          , "b" (&info)
 * >>                          : "memory");
 * >>     old_address = ATOMIC_CMPXCH_VAL(x86_syscall_segment,(uintptr_t)-1,new_address);
 * >>     if (old_address == (uintptr_t)-1)
 * >>         return new_address;
 * >>     // Deal with race conditions...
 * >>     __asm__ __volatile__("int $0x80"
 * >>                          :
 * >>                          : "a" (__NR_munmap)
 * >>                          , "b" (new_address)
 * >>                          , "c" (USHARE_X86_SYSCALL_FSIZE)
 * >>                          : "memory");
 * >>     return old_address;
 * >> }
 * >> uintptr_t x86_get_syscall_entry4(void) {
 * >>     uintptr_t result = x86_get_syscall_segment();
 * >>     result += USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(4);
 * >>     return result;
 * >> }
 */

struct vm_region;

/* Lookup a user-share segment, given its `name'.
 * @throw: E_INVALID_ARGUMENT: The given `name' does not refer to a known ushare segment. */
FUNDEF ATTR_RETNONNULL REF struct vm_region *KCALL ushare_lookup(u32 name);

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_USHARE_H */
