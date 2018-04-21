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
#ifndef _KOS_KERNCTL_H
#define _KOS_KERNCTL_H 1

#include <__stdinc.h>
#include <bits/types.h>

#ifndef __KERNEL__
#include <unistd.h>
#include <syscall.h>
#endif

__SYSDECL_BEGIN

/* Kernel control commands. (Use of any of these requires the `CAP_SYSADMIN' capability)
 * NOTE: Because of their rarity, there is no libc wrapper for this system call.
 *       However, for convenience you may use the macros below.
 *       How many (if any at all) and what arguments are required depends
 *       on the control command, which documents them at `(param)' in the
 *       associated comment (Empty parenthesis `()' means no arguments required) */


/* Kernel debug control commands. */
#define KERNEL_CONTROL_DBG_DUMP_LEAKS     0xdb000001 /* () Dump kernel memory leaks (No-op when built without `CONFIG_DEBUG_MALLOC').
                                                      * @return: * : The number of leaked (unreachable) data blocks. */
#define KERNEL_CONTROL_DBG_CHECK_PADDING  0xdb000002 /* () Validate kmalloc() data pointer header and tail blocks (No-op when built without `CONFIG_DEBUG_MALLOC'). */
#define KERNEL_CONTROL_DBG_HEAP_VALIDATE  0xdb000003 /* () Validate heaps for illegal use-after-free (No-op when built without `CONFIG_DEBUG_HEAP'). */

/* Turn system call tracing on / off */
#define KERNEL_CONTROL_TRACE_SYSCALLS_ON  0x88000001 /* () Turn system call tracing on */
#define KERNEL_CONTROL_TRACE_SYSCALLS_OFF 0x88000002 /* () Turn system call tracing off */

/* Module loading API */
#define KERNEL_CONTROL_INSMOD             0x66000001 /* (char const *path, char const *commandline) -- Load a driver module (NOTE: `commandline' may be `NULL') */
#define KERNEL_CONTROL_DELMOD             0x66000002 /* (char const *path) -- Unload a driver module */



#ifndef __KERNEL__
/* Trigger kernel control commands.
 * @capability(CAP_SYSADMIN)
 * @throw: E_INVALID_ARGUMENT: The given command (first argument) was not recognized.
 * @return: * :                Depending on command. */
#define kernctl(...) \
        syscall(SYS_xkernctl,__VA_ARGS__)
#ifdef __USE_EXCEPT
#define Xkernctl(...) \
        Xsyscall(SYS_xkernctl,__VA_ARGS__)
#endif /* __USE_EXCEPT */
#else

/* Kernel-side version of the user-space `kernctl' command */
__PUBDEF __syscall_slong_t __KCALL
kernel_control(__syscall_ulong_t __command, __syscall_ulong_t __arg0,
               __syscall_ulong_t __arg1, __syscall_ulong_t __arg2,
               __syscall_ulong_t __arg3, __syscall_ulong_t __arg4);

#endif


__SYSDECL_END

#endif /* !_KOS_KERNCTL_H */
