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
#ifndef GUARD_KERNEL_SRC_KERNEL_SYSTEM_C
#define GUARD_KERNEL_SRC_KERNEL_SYSTEM_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <hybrid/limits.h>
#include <hybrid/section.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <kos/context.h>
#include <kos/gc.h>
#include <linux/sysinfo.h>

DECL_BEGIN

DEFINE_SYSCALL1(sysinfo,USER UNCHECKED struct sysinfo *,info) {
 validate_writable(info,sizeof(struct sysinfo));
 /* TODO */
 info->uptime    = 42;
 info->loads[0]  = 42;
 info->loads[1]  = 42;
 info->loads[2]  = 42;
 info->totalram  = 42;
 info->freeram   = 42;
 info->sharedram = 42;
 info->bufferram = 42;
 info->totalswap = 42;
 info->freeswap  = 42;
 info->procs     = 42;
 info->totalhigh = 42;
 info->freehigh  = 42;
 info->mem_unit  = 42;
 return 0;
}



INTDEF void KCALL
reset_heap_data(byte_t *ptr, u32 pattern, size_t num_bytes);
INTDEF byte_t *KCALL
find_modified_address(byte_t *ptr, u32 pattern, size_t num_bytes);


/* System calls used to implement user-space debug heaps. */
DEFINE_SYSCALL3(xreset_debug_data,
                USER UNCHECKED void *,ptr,
                u32,pattern,size_t,num_bytes) {
 validate_writable(ptr,num_bytes);
 reset_heap_data((byte_t *)ptr,pattern,num_bytes);
 return (syscall_ulong_t)ptr;
}

DEFINE_SYSCALL3(xfind_modified_address,
                USER UNCHECKED void *,ptr,
                u32,pattern,size_t,num_bytes) {
 validate_readable(ptr,num_bytes);
 return (syscall_ulong_t)find_modified_address((byte_t *)ptr,pattern,num_bytes);
}




DEFINE_SYSCALL4(xgc_search,
                USER UNCHECKED struct gc_specs const *,uspecs,unsigned int,flags,
                USER UNCHECKED struct gc_data *,udata,gc_ver_t,current_version) {
 size_t result = 0;
 //struct gc_data data;
 validate_readable(uspecs,sizeof(struct gc_specs));
 validate_readable(udata,sizeof(struct gc_data));
 //data = *udata;
 COMPILER_READ_BARRIER();
 /* TODO */
 return result;
}

DEFINE_SYSCALL4(xcapture_traceback,
                USER UNCHECKED struct x86_usercontext *,ctx,unsigned int,num_skipframes,
                USER UNCHECKED void **,ptb,size_t,bufsize) {
 size_t result = 0;
 validate_readable(ctx,sizeof(struct x86_usercontext));
 validate_readable(ptb,bufsize);


 /* TODO */
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_SYSTEM_C */
