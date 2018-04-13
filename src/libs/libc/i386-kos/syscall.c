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
#ifndef GUARD_LIBS_LIBC_I386_KOS_SYSCALL_C
#define GUARD_LIBS_LIBC_I386_KOS_SYSCALL_C 1

#include "../libc.h"
#include <hybrid/atomic.h>
#include <syscall.h>
#include <kos/ushare.h>
#include <syslog.h>

DECL_BEGIN


PRIVATE uintptr_t x86_syscall_segment = (uintptr_t)-1;
INTERN uintptr_t LIBCCALL x86_get_syscall_segment(void) {
 struct mmap_info info;
 uintptr_t old_address,new_address;
 /* Quick check: has the syscall segment already been loaded? */
 new_address = x86_syscall_segment;
 if (new_address != (uintptr_t)-1) {
  if (x86_syscall_segment == 0) __asm__("int3");
  return new_address;
 }

 /* Map the X86 syscall segment into our address space. */
 info.mi_addr            = (void *)USHARE_ADDRESS_HINT;
 info.mi_prot            = PROT_READ|PROT_EXEC;
 info.mi_flags           = MAP_PRIVATE;
 info.mi_xflag           = XMAP_USHARE;
 info.mi_size            = USHARE_X86_SYSCALL_FSIZE;
 info.mi_align           = PAGESIZE;
 info.mi_gap             = 0;
 info.mi_tag             = NULL;
 info.mi_ushare.mu_name  = USHARE_X86_SYSCALL_FNAME;
 info.mi_ushare.mu_start = 0;

 /* int 0x80 is always supported as a system call mechanism */
 __asm__ __volatile__("int $0x80"
                      : "=a" (new_address)
                      : "a" (SYS_xmmap)
                      , "b" (MMAP_INFO_CURRENT)
                      , "c" (&info)
                      : "memory", "edx");
#if 0 /* Shouldn't really happen to begin with... (If mmap() fails, we might as well crash spectacularly)
       * Also: If libc isn't able to initialize properly, we
       *       probably won't be able to tell the user at all...
       *      (whatever would have told them probably uses us, too) */
 if unlikely(new_address == (uintptr_t)-1)
    __asm__ __volatile__("int $0x80" : : "a" (SYS_exit_group), "b" (1));
#endif
  if (new_address == 0) __asm__("int3");
 old_address = ATOMIC_CMPXCH_VAL(x86_syscall_segment,(uintptr_t)-1,new_address);

 if (old_address == (uintptr_t)-1)
     return new_address;
 /* Deal with this race conditions...
  * NOTE: The only way this could happen is if the hosting application
  *       used a direct system call to spawn new threads _before_
  *       having those threads invoke some libc function that uses
  *       a system call.
  *       If user-applications only use libc for system calls, this
  *       shouldn't be able to ever happen. */
 __asm__ __volatile__("int $0x80"
                      :
                      : "a" (SYS_munmap)
                      , "b" (new_address)
                      , "c" (USHARE_X86_SYSCALL_FSIZE)
                      : "memory", "edx");
 return old_address;
}

DECL_END

#endif /* !GUARD_LIBS_LIBC_I386_KOS_SYSCALL_C */
