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
#ifndef _I386_KOS_BITS_SYSCALL_H
#define _I386_KOS_BITS_SYSCALL_H 1
#define _BITS_SYSCALL_H 1

/* Alias all system call numbers using the `SYS_*' notation in favor of `__NR_*' */

/*[[[deemon
#include <file>
#include <fs>
fs::chdir(fs::path::head(__FILE__));
local fp = file.open("../asm/syscallno.ci");
for (local line: fp) {
    local name;
    try name = line.scanf(" # define __NR_%s ")...;
    catch (...) continue;
    if (name.endswith("syscall_min") ||
        name.endswith("syscall_max"))
        continue;
    print "#define SYS_"+name,"__NR_"+name;
}
]]]*/
#define SYS_getcwd __NR_getcwd
#define SYS_dup __NR_dup
#define SYS_dup3 __NR_dup3
#define SYS_fcntl __NR_fcntl
#define SYS_ioctl __NR_ioctl
#define SYS_mknodat __NR_mknodat
#define SYS_mkdirat __NR_mkdirat
#define SYS_unlinkat __NR_unlinkat
#define SYS_symlinkat __NR_symlinkat
#define SYS_linkat __NR_linkat
#define SYS_renameat __NR_renameat
#define SYS_umount2 __NR_umount2
#define SYS_mount __NR_mount
#define SYS_truncate __NR_truncate
#define SYS_ftruncate __NR_ftruncate
#define SYS_fallocate __NR_fallocate
#define SYS_faccessat __NR_faccessat
#define SYS_chdir __NR_chdir
#define SYS_fchdir __NR_fchdir
#define SYS_chroot __NR_chroot
#define SYS_fchmod __NR_fchmod
#define SYS_fchmodat __NR_fchmodat
#define SYS_fchownat __NR_fchownat
#define SYS_fchown __NR_fchown
#define SYS_openat __NR_openat
#define SYS_close __NR_close
#define SYS_pipe2 __NR_pipe2
#define SYS_lseek __NR_lseek
#define SYS_read __NR_read
#define SYS_write __NR_write
#define SYS_pread64 __NR_pread64
#define SYS_pwrite64 __NR_pwrite64
#define SYS_pselect6 __NR_pselect6
#define SYS_ppoll __NR_ppoll
#define SYS_readlinkat __NR_readlinkat
#define SYS_fstatat64 __NR_fstatat64
#define SYS_fstat64 __NR_fstat64
#define SYS_sync __NR_sync
#define SYS_fsync __NR_fsync
#define SYS_fdatasync __NR_fdatasync
#define SYS_utimensat __NR_utimensat
#define SYS_exit __NR_exit
#define SYS_exit_group __NR_exit_group
#define SYS_waitid __NR_waitid
#define SYS_set_tid_address __NR_set_tid_address
#define SYS_unshare __NR_unshare
#define SYS_futex __NR_futex
#define SYS_nanosleep __NR_nanosleep
#define SYS_sched_yield __NR_sched_yield
#define SYS_kill __NR_kill
#define SYS_tkill __NR_tkill
#define SYS_tgkill __NR_tgkill
#define SYS_sigaltstack __NR_sigaltstack
#define SYS_sigsuspend __NR_sigsuspend
#define SYS_sigaction __NR_sigaction
#define SYS_sigprocmask __NR_sigprocmask
#define SYS_sigpending __NR_sigpending
#define SYS_sigtimedwait __NR_sigtimedwait
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
#define SYS_socket __NR_socket
#define SYS_bind __NR_bind
#define SYS_listen __NR_listen
#define SYS_accept __NR_accept
#define SYS_connect __NR_connect
#define SYS_getsockname __NR_getsockname
#define SYS_getpeername __NR_getpeername
#define SYS_sendto __NR_sendto
#define SYS_recvfrom __NR_recvfrom
#define SYS_setsockopt __NR_setsockopt
#define SYS_getsockopt __NR_getsockopt
#define SYS_shutdown __NR_shutdown
#define SYS_munmap __NR_munmap
#define SYS_mremap __NR_mremap
#define SYS_clone __NR_clone
#define SYS_execve __NR_execve
#define SYS_mmap __NR_mmap
#define SYS_swapon __NR_swapon
#define SYS_swapoff __NR_swapoff
#define SYS_mprotect __NR_mprotect
#define SYS_rt_tgsigqueueinfo __NR_rt_tgsigqueueinfo
#define SYS_accept4 __NR_accept4
#define SYS_wait4 __NR_wait4
#define SYS_syncfs __NR_syncfs
#define SYS_execveat __NR_execveat
#define SYS_recv __NR_recv
#define SYS_send __NR_send
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
#define SYS_xunwind_except __NR_xunwind_except
#define SYS_xaddr2line __NR_xaddr2line
#define SYS_xgetsockname __NR_xgetsockname
#define SYS_xgetpeername __NR_xgetpeername
#define SYS_xgetsockopt __NR_xgetsockopt
#define SYS_xfdlopenat __NR_xfdlopenat
#define SYS_xdlclose __NR_xdlclose
#define SYS_xdlsym __NR_xdlsym
#define SYS_xdlfini __NR_xdlfini
#define SYS_xdlmodule_info __NR_xdlmodule_info
//[[[end]]]

#endif /* !_I386_KOS_BITS_SYSCALL_H */
