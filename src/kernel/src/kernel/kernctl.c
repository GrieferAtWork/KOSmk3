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
#ifndef GUARD_KERNEL_SRC_KERNEL_KERNCTL_C
#define GUARD_KERNEL_SRC_KERNEL_KERNCTL_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <kernel/syscall.h>
#include <kernel/heap.h>
#include <kernel/malloc.h>
#include <kos/kernctl.h>
#include <except.h>

DECL_BEGIN

PUBLIC syscall_slong_t KCALL
kernel_control(syscall_ulong_t command, syscall_ulong_t arg0,
               syscall_ulong_t arg1, syscall_ulong_t arg2,
               syscall_ulong_t arg3, syscall_ulong_t arg4) {
 syscall_slong_t result = 0;
 switch (command) {

  /* Debug control commands. */
 case KERNEL_CONTROL_DBG_DUMP_LEAKS:
  result = mall_dump_leaks(__GFP_HEAPMASK);
  break;

 case KERNEL_CONTROL_DBG_CHECK_PADDING:
  mall_validate_padding(__GFP_HEAPMASK);
  break;

 case KERNEL_CONTROL_DBG_HEAP_VALIDATE:
  heap_validate_all();
  break;

 case KERNEL_CONTROL_TRACE_SYSCALLS_ON:
  enable_syscall_tracing();
  break;

 case KERNEL_CONTROL_TRACE_SYSCALLS_OFF:
  disable_syscall_tracing();
  break;

 default:
  error_throw(E_INVALID_ARGUMENT);
  break;
 }
 return result;
}



DEFINE_SYSCALL6(xkernctl,
                syscall_ulong_t,command,syscall_ulong_t,arg0,
                syscall_ulong_t,arg1,syscall_ulong_t,arg2,
                syscall_ulong_t,arg3,syscall_ulong_t,arg4) {
 /* TODO: Check `CAP_SYS_ADMIN' */
 return kernel_control(command,arg0,arg1,arg2,arg3,arg4);
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_KERNCTL_C */
