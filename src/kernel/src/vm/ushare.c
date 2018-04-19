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
#ifndef GUARD_KERNEL_VM_USHARE_C
#define GUARD_KERNEL_VM_USHARE_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <bits/waitstatus.h>
#include <except.h>
#include <kernel/ushare.h>
#include <kernel/vm.h>
#include <kernel/version.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <kos/ushare.h>
#include <sched/pid.h>
#include <sched/stat.h>
#include <string.h>
#include <stdlib.h>
#include <sys/utsname.h>

DECL_BEGIN

#ifndef CONFIG_NO_VIO
PRIVATE u32 KCALL
procinfo_readl(void *UNUSED(closure), uintptr_t addr) {
 assert(PREEMPTION_ENABLED());

#define FIELD(x) case offsetof(struct ushare_procinfo,x):
 switch (addr) {

 FIELD(pi_tid)
  return posix_gettid();

 FIELD(pi_pid)
  return posix_getpid();

 FIELD(pi_ppid)
  return posix_getppid();

 FIELD(pi_gpid)
  return posix_getpgid();

 FIELD(pi_sid)
  return posix_getsid();

 FIELD(pi_hz)
  return HZ;

 FIELD(pi_time)
  return jiffies32[0];
 case offsetof(struct ushare_procinfo,pi_time) + 4:
  return jiffies32[1];

#ifndef CONFIG_NO_TASKSTAT
 FIELD(pi_thread.t_start)
  return THIS_STAT.ts_started32[0];
 case offsetof(struct ushare_procinfo,pi_thread.t_start) + 4:
  return THIS_STAT.ts_started32[1];
 FIELD(pi_thread.t_hswitch)
  return THIS_STAT.ts_hswitch;
 FIELD(pi_thread.t_uswitch)
  return THIS_STAT.ts_uswitch;
 FIELD(pi_thread.t_hyield)
  return THIS_STAT.ts_hyield;
 FIELD(pi_thread.t_uyield)
  return THIS_STAT.ts_uyield;
 FIELD(pi_thread.t_sleep)
  return THIS_STAT.ts_sleep;
 FIELD(pi_thread.t_xrpc)
  return THIS_STAT.ts_xrpc;
 FIELD(pi_thread.t_qrpc)
  return THIS_STAT.ts_qrpc;
#endif

 FIELD(pi_rand)
  return rand();

 {
  PRIVATE u32 procinfo_counter = 0;
 FIELD(pi_counter)
  return ATOMIC_FETCHINC(procinfo_counter);
 }

 default: break;
 }
#undef FIELD
 error_throw(E_NOT_IMPLEMENTED);
}

PRIVATE void KCALL
procinfo_writel(void *UNUSED(closure), uintptr_t addr, u32 value) {
#define FIELD(x) case offsetof(struct ushare_procinfo,x):
 switch (addr) {

 {
  struct exception_info *info;
 FIELD(pi_exit)
 FIELD(pi_exit_group)
  info = error_info();
  memset(info->e_error.e_pointers,0,sizeof(info->e_error.e_pointers));
  info->e_error.e_code = E_EXIT_THREAD;
  if (addr == offsetof(struct ushare_procinfo,pi_exit_group))
      info->e_error.e_code = E_EXIT_PROCESS;
  info->e_error.e_flag = ERR_FNORMAL;
  info->e_error.e_exit.e_status = __W_EXITCODE(value,0);
  error_throw_current();
  __builtin_unreachable();
 } break;

 default: break;
 }
#undef FIELD
 error_throw(E_NOT_IMPLEMENTED);
}
PRIVATE struct vio_ops ushare_procinfo_ops = {
    .v_readl  = &procinfo_readl,
    .v_writel = &procinfo_writel,
};
PRIVATE struct vm_region ushare_procinfo_region = 
    VM_REGION_INIT_VIO(ushare_procinfo_region,1,&ushare_procinfo_ops,NULL);
#endif /* !CONFIG_NO_VIO */


/* Define a region for the strerror() USHARE segment. */
INTDEF byte_t ushare_strerror_pageno[];
PRIVATE struct vm_region ushare_strerror_region = {
    .vr_refcnt = 1,
    .vr_lock   = MUTEX_INIT,
    .vr_type   = VM_REGION_MEM,
    .vr_init   = VM_REGION_INIT_FNORMAL,
    .vr_flags  = VM_REGION_FCANTSHARE|VM_REGION_FDONTMERGE,
    .vr_size   = USHARE_STRERROR_FSIZE/PAGESIZE,
    .vr_parts  = &ushare_strerror_region.vr_part0,
    .vr_part0  = {
        .vp_refcnt = 1,
        .vp_chain  = { NULL, &ushare_strerror_region.vr_parts },
        .vp_state  = VM_PART_INCORE,
        .vp_flags  = VM_PART_FKEEP|VM_PART_FWEAKREF|VM_PART_FNOSWAP,
        .vp_phys   = {
            .py_num_scatter = 1,
            .py_iscatter = {
                [0] = {
                    .ps_addr = (uintptr_t)ushare_strerror_pageno,
                    .ps_size = USHARE_STRERROR_FSIZE/PAGESIZE
                }
            }
        }
    }
};


/* Define a region for the utsname USHARE segment. */
struct utsname_pad {
    struct utsname n;
    byte_t pad[PAGESIZE-sizeof(struct utsname)];
};
INTERN ATTR_SECTION(".data.ushare.utsname")
struct utsname_pad _ushare_utsname ASMNAME("ushare_utsname") = {
    .n = {
        .sysname    = "KOS",
        .release    = PP_STR(KOS_VERSION_MAJOR),
        .version    = PP_STR(KOS_VERSION_MINOR),
        .nodename   = "insert-hostname-here",
        .domainname = "insert-domainname-here",
#ifdef __i386__
        .machine    = "i386",
#elif defined(__x86_64__)
        .machine    = "x86_64",
#elif defined(__arm__)
        .machine    = "arm",
#else
#warning "Unknown host architecture"
        .machine    = "UNKNOWN",
#endif
    }
};
INTDEF byte_t ushare_utsname_pageno[];
PRIVATE struct vm_region ushare_utsname_region = {
    .vr_refcnt = 1,
    .vr_lock   = MUTEX_INIT,
    .vr_type   = VM_REGION_MEM,
    .vr_init   = VM_REGION_INIT_FNORMAL,
    .vr_flags  = VM_REGION_FCANTSHARE|VM_REGION_FDONTMERGE,
    .vr_size   = USHARE_UTSNAME_FSIZE/PAGESIZE,
    .vr_parts  = &ushare_utsname_region.vr_part0,
    .vr_part0  = {
        .vp_refcnt = 1,
        .vp_chain  = { NULL, &ushare_utsname_region.vr_parts },
        .vp_state  = VM_PART_INCORE,
        .vp_flags  = VM_PART_FKEEP|VM_PART_FWEAKREF|VM_PART_FNOSWAP,
        .vp_phys   = {
            .py_num_scatter = 1,
            .py_iscatter = {
                [0] = {
                    .ps_addr = (uintptr_t)ushare_utsname_pageno,
                    .ps_size = USHARE_UTSNAME_FSIZE/PAGESIZE
                }
            }
        }
    }
};







INTDEF ATTR_RETNONNULL REF struct vm_region *KCALL arch_ushare_lookup(u32 name);

/* Lookup a user-share segment, given its `name'.
 * @throw: E_INVALID_ARGUMENT: The given `name' does not refer to a known ushare segment. */
PUBLIC ATTR_RETNONNULL REF struct vm_region *
KCALL ushare_lookup(u32 name) {
 /* arch-independent user-share segments... */
 switch (name) {

#ifndef CONFIG_NO_VIO
 case USHARE_PROCINFO_FNAME:
  vm_region_incref(&ushare_procinfo_region);
  return &ushare_procinfo_region;
#endif

 case USHARE_STRERROR_FNAME:
  vm_region_incref(&ushare_strerror_region);
  return &ushare_strerror_region;

 case USHARE_UTSNAME_FNAME:
  vm_region_incref(&ushare_utsname_region);
  return &ushare_utsname_region;

 default: break;
 }
 return arch_ushare_lookup(name);
}




INTERN ATTR_WEAK ATTR_RETNONNULL
REF struct vm_region *KCALL
arch_ushare_lookup(u32 name) {
 error_throw(E_INVALID_ARGUMENT);
}





DEFINE_SYSCALL2(sethostname,
                USER UNCHECKED char const *,name,
                size_t,namelen) {
 char temp[_UTSNAME_NODENAME_LENGTH];
 /* TODO: capable(CAP_SYS_ADMIN). */
 if unlikely(namelen > _UTSNAME_NODENAME_LENGTH)
    error_throw(E_INVALID_ARGUMENT);
 validate_readable(name,namelen);
 /* Copy the name into a temporary buffer.
  * -> If this memcpy() faults, we don't want to
  *    leave the hostname in an undefined state. */
 memcpy(temp,name,namelen*sizeof(char));
 memset(temp+namelen,0,
       (_UTSNAME_NODENAME_LENGTH -
        namelen)*sizeof(char));
 COMPILER_READ_BARRIER();
 /* Copy the new name into the UTSNAME USHARE segment. */
 memcpy(_ushare_utsname.n.nodename,temp,
        _UTSNAME_NODENAME_LENGTH*sizeof(char));
 /* And we're done! */
 return 0;
}

DEFINE_SYSCALL2(setdomainname,
                USER UNCHECKED char const *,name,
                size_t,namelen) {
 char temp[_UTSNAME_DOMAIN_LENGTH];
 /* TODO: capable(CAP_SYS_ADMIN). */
 if unlikely(namelen > _UTSNAME_DOMAIN_LENGTH)
    error_throw(E_INVALID_ARGUMENT);
 validate_readable(name,namelen);
 /* Copy the name into a temporary buffer.
  * -> If this memcpy() faults, we don't want to
  *    leave the hostname in an undefined state. */
 memcpy(temp,name,namelen*sizeof(char));
 memset(temp+namelen,0,
       (_UTSNAME_DOMAIN_LENGTH -
        namelen)*sizeof(char));
 COMPILER_READ_BARRIER();
 /* Copy the new name into the UTSNAME USHARE segment. */
 memcpy(_ushare_utsname.n.domainname,temp,
        _UTSNAME_DOMAIN_LENGTH*sizeof(char));
 /* And we're done! */
 return 0;
}



DECL_END

#endif /* !GUARD_KERNEL_VM_USHARE_C */
