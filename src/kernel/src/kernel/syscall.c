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
#ifndef GUARD_KERNEL_SRC_KERNEL_SYSCALL_C
#define GUARD_KERNEL_SRC_KERNEL_SYSCALL_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <hybrid/section.h>
#include <kernel/debug.h>
#include <kernel/syscall.h>
#include <syscall.h>
#include <except.h>
#include <bits/fcntl-linux.h>
#include <kernel/paging.h>
#include <sched/pid.h>

DECL_BEGIN

PUBLIC ATTR_NOTHROW void KCALL
syscall_trace(struct syscall_trace_regs *__restrict regs) {
 TRY {
  uintptr_t sysno;
  sysno = regs->str_args.a_sysno & ~0x80000000;
  switch (sysno) {
  case SYS_xsyslog:
   return; /* Not this one... */
  default: break;
  }
  debug_printf("[%u]trace.%s",
               posix_gettid(),
               regs->str_args.a_sysno & 0x80000000 ? "X" : "");
  switch (sysno) {

  case SYS_ioctl:
   debug_printf("sys_ioctl(%d,%#lx,%p)\n",
               (int)regs->str_args.a_arg0,
               (unsigned long)regs->str_args.a_arg1,
               (void *)regs->str_args.a_arg2);
   break;

  case SYS_dup:
   debug_printf("sys_dup(%d)\n",(int)regs->str_args.a_arg0);
   break;

  case SYS_dup3:
   debug_printf("sys_dup3(%d,%d,%x)\n",
               (int)regs->str_args.a_arg0,
               (int)regs->str_args.a_arg1,
               (unsigned int)regs->str_args.a_arg2);
   break;

  case SYS_mknodat:
   debug_printf("sys_mknodat(%d,%q,%o,%[dev])\n",
               (int)regs->str_args.a_arg0,
               (char *)regs->str_args.a_arg1,
               (unsigned int)regs->str_args.a_arg2,
               (dev_t)regs->str_args.a_arg3);
   break;

  case SYS_mkdirat:
   debug_printf("sys_mkdirat(%d,%q,%o)\n",
               (int)regs->str_args.a_arg0,
               (char *)regs->str_args.a_arg1,
               (unsigned int)regs->str_args.a_arg2);
   break;

  case SYS_unlinkat:
   debug_printf("sys_unlinkat(%d,%q,%x)\n",
               (int)regs->str_args.a_arg0,
               (char *)regs->str_args.a_arg1,
               (unsigned int)regs->str_args.a_arg2);
   break;

#define SYS_symlinkat __NR_symlinkat
#define SYS_linkat __NR_linkat
#define SYS_renameat __NR_renameat
#define SYS_umount2 __NR_umount2
  case SYS_mount:
   debug_printf("sys_mount(%q,%q,%q,%x,%p)\n",
               (char *)regs->str_args.a_arg0,
               (char *)regs->str_args.a_arg1,
               (char *)regs->str_args.a_arg2,
               (unsigned int)regs->str_args.a_arg3,
               (void *)regs->str_args.a_arg4);
   break;

#define SYS_truncate __NR_truncate
#define SYS_ftruncate __NR_ftruncate
#define SYS_fallocate __NR_fallocate
#define SYS_faccessat __NR_faccessat
  case SYS_chdir:
   debug_printf("sys_chdir(%q)\n",
               (char *)regs->str_args.a_arg0);
   break;
  case SYS_fchdir:
   debug_printf("sys_fchdir(%d)\n",
               (int)regs->str_args.a_arg0);
   break;
  case SYS_chroot:
   debug_printf("sys_chroot(%q)\n",
               (char *)regs->str_args.a_arg0);
   break;
  case SYS_fchmod:
   debug_printf("sys_fchmod(%d,%o)\n",
               (int)regs->str_args.a_arg0,
               (unsigned int)regs->str_args.a_arg1);
   break;
  case SYS_fchmodat:
   debug_printf("sys_fchmodat(%d,%q,%o,%x)\n",
               (int)regs->str_args.a_arg0,
               (char *)regs->str_args.a_arg1,
               (unsigned int)regs->str_args.a_arg2,
               (unsigned int)regs->str_args.a_arg3);
   break;
  case SYS_fchownat:
   debug_printf("sys_fchownat(%d,%q,%u,%u,%x)\n",
               (int)regs->str_args.a_arg0,
               (char *)regs->str_args.a_arg1,
               (unsigned int)regs->str_args.a_arg2,
               (unsigned int)regs->str_args.a_arg3,
               (unsigned int)regs->str_args.a_arg4);
   break;
  case SYS_fchown:
   debug_printf("sys_fchown(%d,%u,%u)\n",
               (int)regs->str_args.a_arg0,
               (unsigned int)regs->str_args.a_arg1,
               (unsigned int)regs->str_args.a_arg2);
   break;
  case SYS_openat:
   if (regs->str_args.a_arg2 & O_CREAT) {
    debug_printf("sys_openat(%d,%q,%#x,%#o)\n",
                (int)regs->str_args.a_arg0,
                (void *)regs->str_args.a_arg1,
                (unsigned int)regs->str_args.a_arg2,
                (unsigned int)regs->str_args.a_arg3);
   } else {
    debug_printf("sys_openat(%d,%q,%#x)\n",
                (int)regs->str_args.a_arg0,
                (void *)regs->str_args.a_arg1,
                (unsigned int)regs->str_args.a_arg2);
   }
   break;
  case SYS_close:
   debug_printf("sys_close(%d)\n",
               (int)regs->str_args.a_arg0);
   break;
  case SYS_pipe2:
   debug_printf("sys_pipe2(%p,%x)\n",
               (int *)regs->str_args.a_arg0,
               (unsigned int)regs->str_args.a_arg1);
   break;
  case SYS_lseek:
   debug_printf("sys_lseek(%d,%I64d,%d)\n",
               (int *)regs->str_args.a_arg0,
#ifdef CONFIG_WIDE_64BIT_SYSCALL
               *(u64 *)&regs->str_args.a_arg1,
               (int)regs->str_args.a_arg3
#else
               (s64)regs->str_args.a_arg1,
               (int)regs->str_args.a_arg2
#endif
               );
   break;
  case SYS_read:
   debug_printf("sys_read(%d,%p,%Iu)\n",
               (int)regs->str_args.a_arg0,
               (void *)regs->str_args.a_arg1,
               (size_t)regs->str_args.a_arg2);
   break;
  case SYS_write:
   if (regs->str_args.a_arg2 > 128 ||
       regs->str_args.a_arg1 >= KERNEL_BASE) {
    debug_printf("sys_write(%d,%p,%Iu)\n",
                (int)regs->str_args.a_arg0,
                (void *)regs->str_args.a_arg1,
                (size_t)regs->str_args.a_arg2);
   } else {
    debug_printf("sys_write(%d,%$q,%Iu)\n",
                (int)regs->str_args.a_arg0,
                (size_t)regs->str_args.a_arg2,
                (void *)regs->str_args.a_arg1,
                (size_t)regs->str_args.a_arg2);
   }
   break;
#define SYS_pread64 __NR_pread64
#define SYS_pwrite64 __NR_pwrite64
#define SYS_pselect6 __NR_pselect6
#define SYS_ppoll __NR_ppoll
  case SYS_readlinkat:
   debug_printf("sys_readlinkat(%d,%q,%p,%Iu)\n",
               (int)regs->str_args.a_arg0,
               (char *)regs->str_args.a_arg1,
               (void *)regs->str_args.a_arg2,
               (size_t)regs->str_args.a_arg3);
   break;
  case SYS_fstatat64:
   debug_printf("sys_fstatat64(%d,%q,%p,%x)\n",
               (int)regs->str_args.a_arg0,
               (char *)regs->str_args.a_arg1,
               (void *)regs->str_args.a_arg2,
               (unsigned int)regs->str_args.a_arg3);
   break;
  case SYS_fstat64:
   debug_printf("sys_fstat64(%d,%p)\n",
               (int)regs->str_args.a_arg0,
               (void *)regs->str_args.a_arg1);
   break;
  case SYS_fsync:
   debug_printf("sys_fsync(%d)\n",
               (int)regs->str_args.a_arg0);
   break;
  case SYS_fdatasync:
   debug_printf("sys_fdatasync(%d)\n",
               (int)regs->str_args.a_arg0);
   break;
  case SYS_utimensat:
   debug_printf("sys_utimensat(%d,%q,%p,%x)\n",
               (int)regs->str_args.a_arg0,
               (char *)regs->str_args.a_arg1,
               (void *)regs->str_args.a_arg2,
               (unsigned int)regs->str_args.a_arg3);
   break;
  case SYS_exit:
   debug_printf("sys_exit(%d)\n",
               (int)regs->str_args.a_arg0);
   break;
  case SYS_exit_group:
   debug_printf("sys_exit_group(%d)\n",
               (int)regs->str_args.a_arg0);
   break;
  case SYS_waitid:
   debug_printf("sys_waitid(%u,%u,%p,%x)\n",
               (unsigned int)regs->str_args.a_arg0,
               (unsigned int)regs->str_args.a_arg1,
               (void *)regs->str_args.a_arg2,
               (unsigned int)regs->str_args.a_arg3);
   break;
  case SYS_set_tid_address:
   debug_printf("sys_set_tid_address(%p)\n",
               (void *)regs->str_args.a_arg0);
   break;
  case SYS_unshare:
   debug_printf("sys_unshare(%x)\n",
               (unsigned int)regs->str_args.a_arg0);
   break;
#define SYS_futex __NR_futex
#define SYS_nanosleep __NR_nanosleep
  case SYS_kill:
   debug_printf("sys_kill(%u,%d)\n",
               (unsigned int)regs->str_args.a_arg0,
               (int)regs->str_args.a_arg1);
   break;
  case SYS_tkill:
   debug_printf("sys_tkill(%u,%d)\n",
               (unsigned int)regs->str_args.a_arg0,
               (int)regs->str_args.a_arg1);
   break;
  case SYS_tgkill:
   debug_printf("sys_tgkill(%u,%u,%d)\n",
               (unsigned int)regs->str_args.a_arg0,
               (unsigned int)regs->str_args.a_arg1,
               (int)regs->str_args.a_arg2);
   break;
#define SYS_sigaltstack __NR_sigaltstack
#define SYS_sigsuspend __NR_sigsuspend
  case SYS_sigaction:
   debug_printf("sys_sigaction(%u,%p,%p,%Iu)\n",
               (unsigned int)regs->str_args.a_arg0,
               (void *)regs->str_args.a_arg1,
               (void *)regs->str_args.a_arg2,
               (size_t)regs->str_args.a_arg3); 
   break;
  case SYS_sigprocmask:
   debug_printf("sys_sigprocmask(%u,%p,%p,%Iu)\n",
               (unsigned int)regs->str_args.a_arg0,
               (void *)regs->str_args.a_arg1,
               (void *)regs->str_args.a_arg2,
               (size_t)regs->str_args.a_arg3); 
   break;
  case SYS_sigpending:
   debug_printf("sys_sigpending(%p,%Iu)\n",
               (void *)regs->str_args.a_arg0,
               (size_t)regs->str_args.a_arg1); 
   break;
  case SYS_sigtimedwait:
   debug_printf("sys_sigtimedwait(%p,%p,%p,%Iu)\n",
               (void *)regs->str_args.a_arg0,
               (void *)regs->str_args.a_arg1,
               (void *)regs->str_args.a_arg2,
               (size_t)regs->str_args.a_arg3); 
   break;
#define SYS_rt_sigqueueinfo __NR_rt_sigqueueinfo
#define SYS_sigreturn __NR_sigreturn
#define SYS_rt_sigreturn __NR_rt_sigreturn
#define SYS_setpgid __NR_setpgid
#define SYS_getpgid __NR_getpgid
#define SYS_getsid __NR_getsid
#define SYS_setsid __NR_setsid
#define SYS_sethostname __NR_sethostname
#define SYS_setdomainname __NR_setdomainname
#define SYS_umask __NR_umask
#define SYS_getcpu __NR_getcpu
#define SYS_gettimeofday __NR_gettimeofday
#define SYS_settimeofday __NR_settimeofday
#define SYS_getpid __NR_getpid
#define SYS_getppid __NR_getppid
#define SYS_gettid __NR_gettid
#define SYS_sysinfo __NR_sysinfo
#define SYS_munmap __NR_munmap
#define SYS_mremap __NR_mremap
#define SYS_clone __NR_clone
  {
   char **iter;
  case SYS_execve:
   iter = (char **)regs->str_args.a_arg1;
   debug_printf("sys_execve(%q,%p:{",
               (char *)regs->str_args.a_arg0,iter);
   for (; *iter; ++iter) debug_printf("%s%q",iter == (char **)regs->str_args.a_arg1 ? "" : ",",*iter);
   iter = (char **)regs->str_args.a_arg2;
   debug_printf("},%p:{",iter);
   if (iter) for (; *iter; ++iter) {
    debug_printf("%s%q",iter == (char **)regs->str_args.a_arg2 ? "" : ",",*iter);
   }
   debug_printf("})\n");
  } break;
#define SYS_mmap __NR_mmap
#define SYS_swapon __NR_swapon
#define SYS_swapoff __NR_swapoff
#define SYS_mprotect __NR_mprotect
#define SYS_rt_tgsigqueueinfo __NR_rt_tgsigqueueinfo
#define SYS_wait4 __NR_wait4
#define SYS_syncfs __NR_syncfs
#define SYS_execveat __NR_execveat
#define SYS_fork __NR_fork
#define SYS_xsyslog __NR_xsyslog
#define SYS_xmmap __NR_xmmap
#define SYS_xmunmap __NR_xmunmap
#define SYS_xmprotect __NR_xmprotect
#define SYS_xreaddir __NR_xreaddir
#define SYS_xopenpty __NR_xopenpty
#define SYS_xfrealpathat __NR_xfrealpathat
#define SYS_xpipe __NR_xpipe
#define SYS_xfchdirat __NR_xfchdirat
#define SYS_xfrenameat __NR_xfrenameat
#define SYS_xfsymlinkat __NR_xfsymlinkat
#define SYS_xfreadlinkat __NR_xfreadlinkat
#define SYS_xfsmask __NR_xfsmask
#define SYS_xfmknodat __NR_xfmknodat
#define SYS_xfmkdirat __NR_xfmkdirat
#define SYS_xgetdrives __NR_xgetdrives
#define SYS_xftruncateat __NR_xftruncateat
#define SYS_xfpathconfat __NR_xfpathconfat
#define SYS_xsysconf __NR_xsysconf
#define SYS_xreset_debug_data __NR_xreset_debug_data
#define SYS_xfind_modified_address __NR_xfind_modified_address
#define SYS_xgc_search __NR_xgc_search
#define SYS_xcapture_traceback __NR_xcapture_traceback
#define SYS_xreadf __NR_xreadf
#define SYS_xwritef __NR_xwritef
#define SYS_xpreadf64 __NR_xpreadf64
#define SYS_xpwritef64 __NR_xpwritef64
#define SYS_xioctlf __NR_xioctlf
#define SYS_xreaddirf __NR_xreaddirf
#define SYS_xkernctl __NR_xkernctl
#define SYS_xunwind __NR_xunwind
#define SYS_xfdlopenat __NR_xfdlopenat
#define SYS_xdlclose __NR_xdlclose
#define SYS_xdlsym __NR_xdlsym
#define SYS_xdlfini __NR_xdlfini
#define SYS_xdlmodule_info __NR_xdlmodule_info

  case SYS_xfreadlinkat:
   debug_printf("sys_xfreadlinkat(%d,%q,%p,%Iu,%x)\n",
               (int)regs->str_args.a_arg0,
               (char *)regs->str_args.a_arg1,
               (char *)regs->str_args.a_arg2,
               (size_t)regs->str_args.a_arg3,
               (unsigned int)regs->str_args.a_arg4);
   break;

  case SYS_xfrealpathat:
   debug_printf("sys_xfrealpathat(%d,%q,%x,%p,%Iu,%x)\n",
               (int)regs->str_args.a_arg0,
               (char *)regs->str_args.a_arg1,
               (unsigned int)regs->str_args.a_arg2,
               (char *)regs->str_args.a_arg3,
               (size_t)regs->str_args.a_arg4,
               (unsigned int)regs->str_args.a_arg5);
   break;

  {
   char const *name;
  default:
   switch (sysno) {
#define __SYSCALL_ASM      __SYSCALL
#define __XSYSCALL         __SYSCALL
#define __XSYSCALL_ASM     __SYSCALL
#define __SYSCALL(id,sym) \
   case id: name = #sym; goto print_name;
#include <asm/syscallno.ci>
print_name:
    debug_printf("%s",name);
    break;
   default:
    debug_printf("sys_[%p]",regs->str_args.a_sysno);
    break;
   }
   {
    u8 argc = 6;
    if (sysno >= __NR_syscall_min &&
        sysno <= __NR_syscall_max) {
     argc = x86_syscall_argc[sysno-__NR_syscall_min];
    } else if (sysno >= __NR_xsyscall_min &&
               sysno <= __NR_xsyscall_max) {
     argc = x86_xsyscall_argc[sysno-__NR_xsyscall_min];
    }
    switch (argc) {
    case 0: debug_printf("()\n"); break;
    case 1: debug_printf("(%p)\n",(void *)regs->str_args.a_arg0); break;
    case 2:
     debug_printf("(%p,%p)\n",
                 (void *)regs->str_args.a_arg0,
                 (void *)regs->str_args.a_arg1);
     break;
    case 3:
     debug_printf("(%p,%p,%p)\n",
                 (void *)regs->str_args.a_arg0,
                 (void *)regs->str_args.a_arg1,
                 (void *)regs->str_args.a_arg2);
     break;
    case 4:
     debug_printf("(%p,%p,%p,%p)\n",
                 (void *)regs->str_args.a_arg0,
                 (void *)regs->str_args.a_arg1,
                 (void *)regs->str_args.a_arg2,
                 (void *)regs->str_args.a_arg3);
     break;
    case 5:
     debug_printf("(%p,%p,%p,%p,%p)\n",
                 (void *)regs->str_args.a_arg0,
                 (void *)regs->str_args.a_arg1,
                 (void *)regs->str_args.a_arg2,
                 (void *)regs->str_args.a_arg3,
                 (void *)regs->str_args.a_arg4);
     break;
    default:
     debug_printf("(%p,%p,%p,%p,%p,%p)\n",
                 (void *)regs->str_args.a_arg0,
                 (void *)regs->str_args.a_arg1,
                 (void *)regs->str_args.a_arg2,
                 (void *)regs->str_args.a_arg3,
                 (void *)regs->str_args.a_arg4,
                 (void *)regs->str_args.a_arg5);
     break;
    }
   }
  } break;
  }

 } CATCH_HANDLED (E_SEGFAULT) {
 }
}

DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_SYSCALL_C */
