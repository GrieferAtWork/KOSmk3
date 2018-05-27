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
#ifndef _KOS_I386_KOS_ASM_USHARE_H
#define _KOS_I386_KOS_ASM_USHARE_H 1

#include <__stdinc.h>
#include <hybrid/host.h>

__SYSDECL_BEGIN

/* A user-space-only address hint that is suggested
 * to-be used when mapping USHARE segments.
 * However, you're allowed to map them anywhere, though mapping
 * them at the standard address makes debugging easier, consider
 * that you'll be able to quickly realize what you're dealing
 * with by simply looking at an address. */
#ifndef USHARE_ADDRESS_HINT
#ifdef __x86_64__
#   define USHARE_ADDRESS_HINT  0xbeefc000 /* beefcake... (TODO) */
#elif defined(__i386__)
#   define USHARE_ADDRESS_HINT  0xbeefc000 /* beefcake... */
#else
#   error "Unsupported architecture"
#endif
#endif /* !USHARE_ADDRESS_HINT */


#if defined(__x86_64__) || defined(__i386__)
/* User-share segment: the x86 syscall entry page. */
#define USHARE_X86_SYSCALL_FNAME            USHARE_NAME(0x86,1)
#define USHARE_X86_SYSCALL_FSIZE            0x1000
#define USHARE_X86_SYSCALL_SYSENTER_STRIDE  0x100
#define USHARE_X86_SYSCALL_SYSENTER_COUNT   7
#define USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(num_args) \
       (USHARE_X86_SYSCALL_SYSENTER_STRIDE*(num_args))
/* Call this address to implement libc's `syscall()' function.
 * All the other `sysenter()' functions expect EAX/RAX to contain
 * the system call number, but this entry point takes that number
 * from the first argument, as defined by CDECL. */
#define USHARE_X86_SYSCALL_OFFSETOF_SYSCALL \
        USHARE_X86_SYSCALL_OFFSETOF_SYSENTER(7)

/* errno-enabled sysenter code.
 * These are identical to the version above, however when the system
 * call returns a negative value, they will negate that value again,
 * store it in the thread-local `errno' variable (located at
 * `%__ASM_USERTASK_SEGMENT:USER_TASK_SEGMENT_OFFSETOF_ERRNO'), before
 * setting the current errno format to `__X86_TASK_ERRNO_FKOS' (aka. writing
 * `__X86_TASK_ERRNO_FKOS' to `%__ASM_USERTASK_SEGMENT:USER_TASK_SEGMENT_OFFSETOF_EFORMAT')
 */
#define USHARE_X86_SYSCALL_OFFSETOF_SYSENTER_ERRNO(num_args) \
       (USHARE_X86_SYSCALL_SYSENTER_STRIDE*(8+(num_args)))
#define USHARE_X86_SYSCALL_OFFSETOF_SYSCALL_ERRNO \
        USHARE_X86_SYSCALL_OFFSETOF_SYSENTER_ERRNO(7)

/* User-share segment: The first 1Mb of physical memory.
 *                     Can be used to identity map memory needed for vm86 tasks.
 *               NOTE: Mapping this segment as writable will force-enable copy-on-write. */
#define USHARE_X86_VM86BIOS_FNAME           USHARE_NAME(0x86,2)
#define USHARE_X86_VM86BIOS_FSIZE           0x100000
#endif


  
__SYSDECL_END

#endif /* !_KOS_I386_KOS_ASM_USHARE_H */
