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
#ifndef GUARD_LIBS_LIBC_SYSTEM_H
#define GUARD_LIBS_LIBC_SYSTEM_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

struct stat64;
struct sysinfo;
struct sigaltstack;
struct timespec64;
struct sigcontext;
struct sigaction;
struct rusage;
struct pollfd;
struct timeval64;
struct timezone;
struct dirent;
struct termios;
struct winsize;
struct gc_specs;
struct gc_data;
struct dl_addr2line;
struct cpu_context;
struct fpu_context;
struct sockaddr;


/* ===================================================================================== */
/*     SYSTEM                                                                            */
/* ===================================================================================== */
INTDEF int LIBCCALL sys_openat(fd_t dfd, char const *filename, oflag_t flags, mode_t mode);
INTDEF errno_t LIBCCALL sys_close(fd_t fd);
INTDEF ATTR_NORETURN void LIBCCALL sys_exit(int exitcode);
INTDEF ATTR_NORETURN void LIBCCALL sys_exit_group(int exitcode);
INTDEF errno_t LIBCCALL sys_unshare(int flags);
INTDEF int LIBCCALL sys_munmap(void *addr, size_t len);
INTDEF void *LIBCCALL sys_mremap(void *addr, size_t old_len, size_t new_len, int flags, void *new_addr);
INTDEF void *LIBCCALL sys_mmap(void *addr, size_t len, int prot, int flags, fd_t fd, syscall_ulong_t off);
INTDEF ssize_t LIBCCALL sys_read(fd_t fd, void *buf, size_t bufsize);
INTDEF ssize_t LIBCCALL sys_write(fd_t fd, void const *buf, size_t bufsize);
INTDEF errno_t LIBCCALL sys_faccessat(fd_t dfd, char const *filename, int mode, atflag_t flags);
INTDEF errno_t LIBCCALL sys_chdir(char const *path);
INTDEF errno_t LIBCCALL sys_chroot(char const *path);
INTDEF errno_t LIBCCALL sys_fchdir(fd_t fd);
INTDEF errno_t LIBCCALL sys_fchmod(fd_t fd, mode_t mode);
INTDEF errno_t LIBCCALL sys_fchown(fd_t fd, uid_t user, gid_t group);
INTDEF errno_t LIBCCALL sys_fchmodat(fd_t dfd, char const *filename, mode_t mode, atflag_t flags);
INTDEF errno_t LIBCCALL sys_fchownat(fd_t dfd, char const *filename, uid_t user, gid_t group, atflag_t flags);
INTDEF fd_t LIBCCALL sys_dup(fd_t fd);
INTDEF fd_t LIBCCALL sys_dup2(fd_t oldfd, fd_t newfd);
INTDEF fd_t LIBCCALL sys_dup3(fd_t oldfd, fd_t newfd, oflag_t flags);
INTDEF syscall_slong_t LIBCCALL sys_fcntl(fd_t fd, unsigned int cmd, void *arg);
INTDEF syscall_slong_t LIBCCALL sys_ioctl(fd_t fd, unsigned long cmd, void *arg);
INTDEF errno_t LIBCCALL sys_execve(char const *filename, char *const *argv, char *const *envp);
INTDEF errno_t LIBCCALL sys_sched_yield(void);
INTDEF pid_t LIBCCALL sys_fork(void);
INTDEF int LIBCCALL sys_sync(void);
INTDEF errno_t LIBCCALL sys_fsync(fd_t fd);
INTDEF errno_t LIBCCALL sys_fdatasync(fd_t fd);
INTDEF errno_t LIBCCALL sys_syncfs(fd_t fs_fd);
INTDEF errno_t LIBCCALL sys_fstat64(fd_t fd, struct __kos_stat64 *statbuf);
INTDEF errno_t LIBCCALL sys_fstatat64(fd_t dfd, char const *filename, struct __kos_stat64 *statbuf, atflag_t flags);
INTDEF ssize_t LIBCCALL sys_readlinkat(fd_t fd, char const *path, char *buf, size_t len);
INTDEF errno_t LIBCCALL sys_mknodat(fd_t dfd, char const *filename, mode_t mode, dev_t dev);
INTDEF errno_t LIBCCALL sys_mkdirat(fd_t dfd, char const *pathname, mode_t mode);
INTDEF errno_t LIBCCALL sys_unlinkat(fd_t dfd, char const *pathname, atflag_t flags);
INTDEF errno_t LIBCCALL sys_symlinkat(char const *oldname, fd_t newdfd, char const *newname);
INTDEF errno_t LIBCCALL sys_linkat(fd_t olddfd, char const *oldname, fd_t newdfd, char const *newname, atflag_t flags);
INTDEF errno_t LIBCCALL sys_renameat(fd_t olddfd, char const *oldname, fd_t newdfd, char const *newname);
INTDEF errno_t LIBCCALL sys_utimensat(fd_t dfd, char const *filename, struct timespec64 const *utimes, atflag_t flags);
INTDEF pid_t LIBCCALL sys_getpid(void);
INTDEF pid_t LIBCCALL sys_getppid(void);
INTDEF pid_t LIBCCALL sys_gettid(void);
INTDEF pid_t LIBCCALL sys_getpgid(pid_t pid);
INTDEF int LIBCCALL sys_setpgid(pid_t pid, pid_t pgid);
INTDEF pid_t LIBCCALL sys_getsid(pid_t pid);
INTDEF pid_t LIBCCALL sys_setsid(void);
INTDEF errno_t LIBCCALL sys_sethostname(char const *name, size_t namelen);
INTDEF errno_t LIBCCALL sys_setdomainname(char const *name, size_t namelen);
INTDEF errno_t LIBCCALL sys_sigaction(int sig, struct sigaction const *act, struct sigaction *oact, size_t sigsetsize);
INTDEF errno_t LIBCCALL sys_sigprocmask(int how, __sigset_t const *set, __sigset_t *oldset, size_t sigsetsize);
INTDEF int LIBCCALL sys_sigtimedwait(__sigset_t const *uthese, siginfo_t *uinfo, struct timespec64 const *uts, size_t sigsetsize);
INTDEF errno_t LIBCCALL sys_sigpending(__sigset_t *uset, size_t sigsetsize);
INTDEF errno_t LIBCCALL sys_sigsuspend(__sigset_t const *unewset, size_t sigsetsize);
INTDEF errno_t LIBCCALL sys_kill(pid_t pid, int sig);
INTDEF errno_t LIBCCALL sys_tkill(pid_t pid, int sig);
INTDEF errno_t LIBCCALL sys_tgkill(pid_t tgid, pid_t pid, int sig);
INTDEF ssize_t LIBCCALL sys_getcwd(char *buf, size_t bufsize);
INTDEF errno_t LIBCCALL sys_waitid(int which, pid_t upid, siginfo_t *infop, int options, struct rusage *ru);
INTDEF pid_t LIBCCALL sys_wait4(pid_t upid, int *stat_addr, int options, struct rusage *ru);
INTDEF errno_t LIBCCALL sys_pipe(int pfd[2]);
INTDEF errno_t LIBCCALL sys_pipe2(int pfd[2], oflag_t flags);
INTDEF errno_t LIBCCALL sys_mount(char const *dev_name, char const *dir_name, char const *type, unsigned long flags, void const *data);
INTDEF errno_t LIBCCALL sys_umount2(char const *name, int flags);
INTDEF errno_t LIBCCALL sys_gettimeofday(struct timeval64 *tv, struct timezone *tz);
INTDEF errno_t LIBCCALL sys_settimeofday(struct timeval64 const *tv, struct timezone const *tz);
INTDEF errno_t LIBCCALL sys_nanosleep(struct timespec64 const *rqtp, struct timespec64 *rmtp);
INTDEF mode_t LIBCCALL sys_umask(mode_t mask);
INTDEF int LIBCCALL sys_mprotect(void *start, size_t len, int prot);
INTDEF errno_t LIBCCALL sys_swapon(char const *specialfile, int flags);
INTDEF errno_t LIBCCALL sys_swapoff(char const *specialfile);
INTDEF syscall_slong_t LIBCCALL sys_futex(u32 *uaddr, int op, u32 val, struct timespec64 const *utime, u32 *uaddr2, u32 val3);
INTDEF errno_t LIBCCALL sys_sysinfo(struct sysinfo *info);
INTDEF errno_t LIBCCALL sys_sigaltstack(struct sigaltstack const *new_stack, struct sigaltstack *old_stack);
INTDEF pid_t LIBCCALL sys_set_tid_address(pid_t *addr);
INTDEF s64 LIBCCALL sys_lseek(fd_t fd, s64 offset, int whence);
INTDEF ssize_t LIBCCALL sys_pread64(fd_t fd, void *buf, size_t bufsize, u64 pos);
INTDEF ssize_t LIBCCALL sys_pwrite64(fd_t fd, void const *buf, size_t bufsize, u64 pos);
INTDEF errno_t LIBCCALL sys_truncate(char const *path, u64 len);
INTDEF errno_t LIBCCALL sys_ftruncate(fd_t fd, u64 len);
INTDEF errno_t LIBCCALL sys_fallocate(fd_t fd, int mode, u64 off, u64 len);
INTDEF errno_t LIBCCALL sys_rt_sigqueueinfo(pid_t tgid, int sig, siginfo_t const *uinfo);
INTDEF errno_t LIBCCALL sys_rt_tgsigqueueinfo(pid_t tgid, pid_t tid, int sig, siginfo_t const *uinfo);
INTDEF ssize_t LIBCCALL sys_pselect6(size_t n, fd_set *inp, fd_set *outp, fd_set *exp, struct timespec64 const *tsp, void *sig);
INTDEF ssize_t LIBCCALL sys_ppoll(struct pollfd *ufds, size_t nfds, struct timespec64 const *tsp, __sigset_t const *sigmask, size_t sigsetsize);
INTDEF errno_t LIBCCALL sys_getcpu(unsigned int *pcpuid, unsigned int *pnodeid);
INTDEF errno_t LIBCCALL sys_execveat(fd_t dfd, char const *filename, char *const *argv, char *const *envp, atflag_t flags);
/* KOS system-call extensions. */
INTDEF ssize_t LIBCCALL sys_xfrealpathat(fd_t fd, char const *path, atflag_t flags, char *buf, size_t bufsize, int type);
INTDEF ssize_t LIBCCALL sys_xreaddir(fd_t fd, struct dirent *buf, size_t bufsize, int mode);
INTDEF s64 LIBCCALL sys_xopenpty(char *name, struct termios const *termp, struct winsize const *winp);
INTDEF s64 LIBCCALL sys_xpipe(int flags);
INTDEF ssize_t LIBCCALL sys_xsyslog(int type, char const *p, size_t len);
INTDEF void *LIBCCALL sys_xmmap(int version, struct mmap_info const *data);
INTDEF ssize_t LIBCCALL sys_xmunmap(void *addr, size_t len, int flags, void *tag);
INTDEF ssize_t LIBCCALL sys_xmprotect(void *start, size_t len, int protmask, int protflag, int flags, void *tag);
INTDEF errno_t LIBCCALL sys_xfchdirat(fd_t dfd, char const *path, atflag_t flags);
INTDEF errno_t LIBCCALL sys_xfrenameat(fd_t olddfd, char const *oldname, fd_t newdfd, char const *newname, atflag_t flags);
INTDEF void *LIBCCALL sys_xfdlopenat(fd_t dfd, char const *filename, atflag_t at_flags, int open_flags, char const *runpath);
INTDEF void *LIBCCALL sys_xdlsym(void *handle, char const *symbol);
INTDEF errno_t LIBCCALL sys_xdlfini(void);
INTDEF errno_t LIBCCALL sys_xdlclose(void *handle);
INTDEF ssize_t LIBCCALL sys_xdlmodule_info(void *handle, int info_class, void *buf, size_t bufsize);
INTDEF errno_t LIBCCALL sys_xfsymlinkat(char const *oldname, fd_t newdfd, char const *newname, atflag_t flags);
INTDEF ssize_t LIBCCALL sys_xfreadlinkat(fd_t dfd, char const *path, char *buf, size_t len, atflag_t flags);
INTDEF u64 LIBCCALL sys_xfsmask(u64 new_mask);
INTDEF errno_t LIBCCALL sys_xfmknodat(fd_t dfd, char const *filename, mode_t mode, dev_t dev, atflag_t flags);
INTDEF errno_t LIBCCALL sys_xfmkdirat(fd_t dfd, char const *filename, mode_t mode, atflag_t flags);
INTDEF errno_t LIBCCALL sys_xftruncateat(fd_t dfd, char const *path, u64 len, atflag_t flags);
INTDEF syscall_slong_t LIBCCALL sys_xfpathconfat(fd_t dfd, char const *path, int name, atflag_t flags);
INTDEF syscall_slong_t LIBCCALL sys_xsysconf(unsigned int name);
INTDEF u32 LIBCCALL sys_xgetdrives(void);
INTDEF pid_t LIBCCALL Esys_clone(struct cpu_context *context, syscall_ulong_t flags, pid_t *parent_tidptr, void *tls_val, pid_t *child_tidptr);
/* Dedicated system calls for hardware-accelerated string
 * operations designed for implementing fast debug heaps.
 * Making various assumptions about unallocated pages, or memory
 * that has been modified since the last call, large portions of
 * memory must rarely ever be checked for corruption, or data
 * having to be reset. */
INTDEF void *LIBCCALL sys_xreset_debug_data(void *ptr, u32 pattern, size_t num_bytes);
INTDEF void *LIBCCALL sys_xfind_modified_address(void *ptr, u32 pattern, size_t num_bytes);
INTDEF ssize_t LIBCCALL sys_xgc_search(struct gc_specs const *uspecs, unsigned int flags, struct gc_data *udata, syscall_ulong_t current_version);
INTDEF ssize_t LIBCCALL sys_xcapture_traceback(struct cpu_context *ctx, unsigned int num_skipframes, void **ptb, size_t bufsize);
INTDEF ssize_t LIBCCALL sys_xreadf(fd_t fd, void *buf, size_t bufsize, oflag_t flags);
INTDEF ssize_t LIBCCALL sys_xwritef(fd_t fd, void const *buf, size_t bufsize, oflag_t flags);
INTDEF ssize_t LIBCCALL sys_xpreadf64(fd_t fd, void *buf, size_t bufsize, u64 pos, oflag_t flags);
INTDEF ssize_t LIBCCALL sys_xpwritef64(fd_t fd, void const *buf, size_t bufsize, u64 pos, oflag_t flags);
INTDEF syscall_slong_t LIBCCALL sys_xioctlf(fd_t fd, unsigned long cmd, oflag_t flags, void *arg);
INTDEF ssize_t LIBCCALL sys_xreaddirf(fd_t fd, struct dirent *buf, size_t bufsize, int mode, oflag_t flags);
INTDEF errno_t LIBCCALL sys_xunwind_except(struct exception_info *except_info, struct cpu_context *dispatcher_ccontext, struct fpu_context *dispatcher_fcontext);
INTDEF errno_t LIBCCALL sys_xunwind(struct cpu_context *ccontext, struct fpu_context *fcontext, sigset_t *signal_set, size_t sigset_size);
INTDEF ssize_t LIBCCALL sys_xaddr2line(void *abs_pc, struct dl_addr2line *buf, size_t bufsize);



/* Exception-enabled versions of system calls. */
INTDEF unsigned int LIBCCALL Xsys_openat(fd_t dfd, char const *filename, oflag_t flags, mode_t mode);
INTDEF errno_t LIBCCALL Xsys_close(fd_t fd);
INTDEF void LIBCCALL Xsys_unshare(int flags);
INTDEF void LIBCCALL Xsys_munmap(void *addr, size_t len);
INTDEF void *LIBCCALL Xsys_mremap(void *addr, size_t old_len, size_t new_len, int flags, void *new_addr);
INTDEF void *LIBCCALL Xsys_mmap(void *addr, size_t len, int prot, int flags, fd_t fd, syscall_ulong_t off);
INTDEF size_t LIBCCALL Xsys_read(fd_t fd, void *buf, size_t bufsize);
INTDEF size_t LIBCCALL Xsys_write(fd_t fd, void const *buf, size_t bufsize);
INTDEF void LIBCCALL Xsys_faccessat(fd_t dfd, char const *filename, int mode, atflag_t flags);
INTDEF void LIBCCALL Xsys_chdir(char const *path);
INTDEF void LIBCCALL Xsys_chroot(char const *path);
INTDEF void LIBCCALL Xsys_fchdir(fd_t fd);
INTDEF void LIBCCALL Xsys_fchmod(fd_t fd, mode_t mode);
INTDEF void LIBCCALL Xsys_fchown(fd_t fd, uid_t user, gid_t group);
INTDEF void LIBCCALL Xsys_fchmodat(fd_t dfd, char const *filename, mode_t mode, atflag_t flags);
INTDEF void LIBCCALL Xsys_fchownat(fd_t dfd, char const *filename, uid_t user, gid_t group, atflag_t flags);
INTDEF fd_t LIBCCALL Xsys_dup(fd_t fd);
INTDEF fd_t LIBCCALL Xsys_dup2(fd_t oldfd, fd_t newfd);
INTDEF fd_t LIBCCALL Xsys_dup3(fd_t oldfd, fd_t newfd, oflag_t flags);
INTDEF syscall_slong_t LIBCCALL Xsys_fcntl(fd_t fd, unsigned int cmd, void *arg);
INTDEF syscall_slong_t LIBCCALL Xsys_ioctl(fd_t fd, unsigned long cmd, void *arg);
INTDEF ATTR_NORETURN void LIBCCALL Xsys_execve(char const *filename, char *const *argv, char *const *envp);
INTDEF pid_t LIBCCALL Xsys_fork(void);
INTDEF void LIBCCALL Xsys_sync(void);
INTDEF void LIBCCALL Xsys_fsync(fd_t fd);
INTDEF void LIBCCALL Xsys_fdatasync(fd_t fd);
INTDEF void LIBCCALL Xsys_syncfs(fd_t fs_fd);
INTDEF void LIBCCALL Xsys_fstat64(fd_t fd, struct __kos_stat64 *statbuf);
INTDEF void LIBCCALL Xsys_fstatat64(fd_t dfd, char const *filename, struct __kos_stat64 *statbuf, atflag_t flags);
INTDEF size_t LIBCCALL Xsys_readlinkat(fd_t fd, char const *path, char *buf, size_t len);
INTDEF void LIBCCALL Xsys_mknodat(fd_t dfd, char const *filename, mode_t mode, dev_t dev);
INTDEF void LIBCCALL Xsys_mkdirat(fd_t dfd, char const *pathname, mode_t mode);
INTDEF void LIBCCALL Xsys_unlinkat(fd_t dfd, char const *pathname, atflag_t flags);
INTDEF void LIBCCALL Xsys_symlinkat(char const *oldname, fd_t newdfd, char const *newname);
INTDEF void LIBCCALL Xsys_linkat(fd_t olddfd, char const *oldname, fd_t newdfd, char const *newname, atflag_t flags);
INTDEF void LIBCCALL Xsys_renameat(fd_t olddfd, char const *oldname, fd_t newdfd, char const *newname);
INTDEF void LIBCCALL Xsys_utimensat(fd_t dfd, char const *filename, struct timespec64 const *utimes, atflag_t flags);
INTDEF pid_t LIBCCALL Xsys_getpgid(pid_t pid);
INTDEF void LIBCCALL Xsys_setpgid(pid_t pid, pid_t pgid);
INTDEF pid_t LIBCCALL Xsys_setsid(void);
INTDEF pid_t LIBCCALL Xsys_getsid(pid_t pid);
INTDEF void LIBCCALL Xsys_sethostname(char const *name, size_t namelen);
INTDEF void LIBCCALL Xsys_setdomainname(char const *name, size_t namelen);
INTDEF void LIBCCALL Xsys_sigaction(int sig, struct sigaction const *act, struct sigaction *oact, size_t sigsetsize);
INTDEF void LIBCCALL Xsys_sigprocmask(int how, __sigset_t const *set, __sigset_t *oldset, size_t sigsetsize);
INTDEF unsigned int LIBCCALL Xsys_sigtimedwait(__sigset_t const *uthese, siginfo_t *uinfo, struct timespec64 const *uts, size_t sigsetsize);
INTDEF void LIBCCALL Xsys_sigpending(__sigset_t *uset, size_t sigsetsize);
INTDEF ATTR_NORETURN void LIBCCALL Xsys_sigsuspend(__sigset_t const *unewset, size_t sigsetsize);
INTDEF void LIBCCALL Xsys_kill(pid_t pid, int sig);
INTDEF void LIBCCALL Xsys_tkill(pid_t pid, int sig);
INTDEF void LIBCCALL Xsys_tgkill(pid_t tgid, pid_t pid, int sig);
INTDEF size_t LIBCCALL Xsys_getcwd(char *buf, size_t bufsize);
INTDEF errno_t LIBCCALL Xsys_waitid(int which, pid_t upid, siginfo_t *infop, int options, struct rusage *ru);
INTDEF pid_t LIBCCALL Xsys_wait4(pid_t upid, int *stat_addr, int options, struct rusage *ru);
INTDEF void LIBCCALL Xsys_pipe(int pfd[2]);
INTDEF void LIBCCALL Xsys_pipe2(int pfd[2], oflag_t flags);
INTDEF void LIBCCALL Xsys_mount(char const *dev_name, char const *dir_name, char const *type, unsigned long flags, void const *data);
INTDEF void LIBCCALL Xsys_umount2(char const *name, int flags);
INTDEF void LIBCCALL Xsys_gettimeofday(struct timeval64 *tv, struct timezone *tz);
INTDEF void LIBCCALL Xsys_settimeofday(struct timeval64 const *tv, struct timezone const *tz);
INTDEF void LIBCCALL Xsys_nanosleep(struct timespec64 const *rqtp, struct timespec64 *rmtp);
INTDEF void LIBCCALL Xsys_mprotect(void *start, size_t len, int prot);
INTDEF void LIBCCALL Xsys_swapon(char const *specialfile, int flags);
INTDEF void LIBCCALL Xsys_swapoff(char const *specialfile);
INTDEF syscall_slong_t LIBCCALL Xsys_futex(u32 *uaddr, int op, u32 val, struct timespec64 const *utime, u32 *uaddr2, u32 val3);
INTDEF void LIBCCALL Xsys_sysinfo(struct sysinfo *info);
INTDEF void LIBCCALL Xsys_sigaltstack(struct sigaltstack const *new_stack, struct sigaltstack *old_stack);
INTDEF u64 LIBCCALL Xsys_lseek(fd_t fd, s64 offset, int whence);
INTDEF size_t LIBCCALL Xsys_pread64(fd_t fd, void *buf, size_t bufsize, u64 pos);
INTDEF size_t LIBCCALL Xsys_pwrite64(fd_t fd, void const *buf, size_t bufsize, u64 pos);
INTDEF void LIBCCALL Xsys_truncate(char const *path, u64 len);
INTDEF void LIBCCALL Xsys_ftruncate(fd_t fd, u64 len);
INTDEF void LIBCCALL Xsys_fallocate(fd_t fd, int mode, u64 off, u64 len);
INTDEF errno_t LIBCCALL Xsys_rt_sigqueueinfo(pid_t tgid, int sig, siginfo_t const *uinfo);
INTDEF errno_t LIBCCALL Xsys_rt_tgsigqueueinfo(pid_t tgid, pid_t tid, int sig, siginfo_t const *uinfo);
INTDEF size_t LIBCCALL Xsys_pselect6(size_t n, fd_set *inp, fd_set *outp, fd_set *exp, struct timespec64 const *tsp, void *sig);
INTDEF size_t LIBCCALL Xsys_ppoll(struct pollfd *ufds, size_t nfds, struct timespec64 const *tsp, __sigset_t const *sigmask, size_t sigsetsize);
INTDEF void LIBCCALL Xsys_getcpu(unsigned int *pcpuid, unsigned int *pnodeid);
INTDEF ATTR_NORETURN void LIBCCALL Xsys_execveat(fd_t dfd, char const *filename, char *const *argv, char *const *envp, int flags);
INTDEF size_t LIBCCALL Xsys_xfrealpathat(fd_t fd, char const *path, atflag_t flags, char *buf, size_t bufsize, int type);
INTDEF size_t LIBCCALL Xsys_xreaddir(fd_t fd, struct dirent *buf, size_t bufsize, int mode);
INTDEF u64 LIBCCALL Xsys_xopenpty(char *name, struct termios const *termp, struct winsize const *winp);
INTDEF u64 LIBCCALL Xsys_xpipe(int flags);
INTDEF size_t LIBCCALL Xsys_xsyslog(int type, char const *p, size_t len);
INTDEF void *LIBCCALL Xsys_xmmap(int version, struct mmap_info const *data);
INTDEF size_t LIBCCALL Xsys_xmunmap(void *addr, size_t len, int flags, void *tag);
INTDEF size_t LIBCCALL Xsys_xmprotect(void *start, size_t len, int protmask, int protflag, int flags, void *tag);
INTDEF void LIBCCALL Xsys_xfchdirat(fd_t dfd, char const *path, atflag_t flags);
INTDEF void LIBCCALL Xsys_xfrenameat(fd_t olddfd, char const *oldname, fd_t newdfd, char const *newname, atflag_t flags);
INTDEF void *LIBCCALL Xsys_xfdlopenat(fd_t dfd, char const *filename, atflag_t at_flags, int open_flags, char const *runpath);
INTDEF void *LIBCCALL Xsys_xdlsym(void *handle, char const *symbol);
INTDEF void LIBCCALL Xsys_xdlfini(void);
INTDEF void LIBCCALL Xsys_xdlclose(void *handle);
INTDEF size_t LIBCCALL Xsys_xdlmodule_info(void *handle, int info_class, void *buf, size_t bufsize);
INTDEF void LIBCCALL Xsys_xfsymlinkat(char const *oldname, fd_t newdfd, char const *newname, atflag_t flags);
INTDEF size_t LIBCCALL Xsys_xfreadlinkat(fd_t dfd, char const *path, char *buf, size_t len, atflag_t flags);
INTDEF void LIBCCALL Xsys_xfmknodat(fd_t dfd, char const *filename, mode_t mode, dev_t dev, atflag_t flags);
INTDEF void LIBCCALL Xsys_xfmkdirat(fd_t dfd, char const *filename, mode_t mode, atflag_t flags);
INTDEF void LIBCCALL Xsys_xftruncateat(fd_t dfd, char const *path, u64 len, atflag_t flags);
INTDEF syscall_slong_t LIBCCALL Xsys_xfpathconfat(fd_t dfd, char const *path, int name, atflag_t flags, char const *runpath);
INTDEF syscall_slong_t LIBCCALL Xsys_xsysconf(unsigned int name);
INTDEF pid_t LIBCCALL Xsys_clone(struct cpu_context *context, syscall_ulong_t flags, pid_t *parent_tidptr, void *tls_val, pid_t *child_tidptr);
INTDEF void *LIBCCALL Xsys_xreset_debug_data(void *ptr, u32 pattern, size_t num_bytes);
INTDEF void *LIBCCALL Xsys_xfind_modified_address(void *ptr, u32 pattern, size_t num_bytes);
INTDEF size_t LIBCCALL Xsys_xgc_search(struct gc_specs const *uspecs, unsigned int flags, struct gc_data *udata, syscall_ulong_t current_version);
INTDEF size_t LIBCCALL Xsys_xcapture_traceback(struct cpu_context *ctx, unsigned int num_skipframes, void **ptb, size_t bufsize);
INTDEF size_t LIBCCALL Xsys_xreadf(fd_t fd, void *buf, size_t bufsize, oflag_t flags);
INTDEF size_t LIBCCALL Xsys_xwritef(fd_t fd, void const *buf, size_t bufsize, oflag_t flags);
INTDEF size_t LIBCCALL Xsys_xpreadf64(fd_t fd, void *buf, size_t bufsize, u64 pos, oflag_t flags);
INTDEF size_t LIBCCALL Xsys_xpwritef64(fd_t fd, void const *buf, size_t bufsize, u64 pos, oflag_t flags);
INTDEF syscall_slong_t LIBCCALL Xsys_xioctlf(fd_t fd, unsigned long cmd, oflag_t flags, void *arg);
INTDEF size_t LIBCCALL Xsys_xreaddirf(fd_t fd, struct dirent *buf, size_t bufsize, int mode, oflag_t flags);
INTDEF errno_t LIBCCALL Xsys_xunwind(struct cpu_context *ccontext, struct fpu_context *fcontext, sigset_t *signal_set, size_t sigset_size);
INTDEF ssize_t LIBCCALL Xsys_xaddr2line(void *abs_pc, struct dl_addr2line *buf, size_t bufsize);

/* Networking functions. */
INTDEF fd_t LIBCCALL sys_socket(int domain, int type, int protocol);
INTDEF errno_t LIBCCALL sys_bind(fd_t sockfd, struct sockaddr const *addr, socklen_t len);
INTDEF errno_t LIBCCALL sys_listen(fd_t sockfd, int max_backlog);
INTDEF fd_t LIBCCALL sys_accept4(fd_t sockfd, struct sockaddr *addr, socklen_t *len, int flags);
INTDEF fd_t LIBCCALL sys_accept(fd_t sockfd, struct sockaddr *addr, socklen_t *len);
INTDEF errno_t LIBCCALL sys_connect(fd_t sockfd, struct sockaddr const *addr, socklen_t len);
INTDEF socklen_t LIBCCALL sys_xgetsockname(fd_t sockfd, struct sockaddr *addr, socklen_t len);
INTDEF socklen_t LIBCCALL sys_xgetpeername(fd_t sockfd, struct sockaddr *addr, socklen_t len);
INTDEF errno_t LIBCCALL sys_getsockname(fd_t sockfd, struct sockaddr *addr, socklen_t *len);
INTDEF errno_t LIBCCALL sys_getpeername(fd_t sockfd, struct sockaddr *addr, socklen_t *len);
INTDEF ssize_t LIBCCALL sys_sendto(fd_t sockfd, void const *buf, size_t buflen, int flags, struct sockaddr const *addr, socklen_t len);
INTDEF ssize_t LIBCCALL sys_send(fd_t sockfd, void const *buf, size_t buflen, int flags);
INTDEF ssize_t LIBCCALL sys_recvfrom(fd_t sockfd, void *buf, size_t buflen, int flags, struct sockaddr *addr, socklen_t *len);
INTDEF ssize_t LIBCCALL sys_recv(fd_t sockfd, void *buf, size_t buflen, int flags);
INTDEF socklen_t LIBCCALL sys_xgetsockopt(fd_t sockfd, int level, int optname, void *optval, socklen_t optlen);
INTDEF errno_t LIBCCALL sys_getsockopt(fd_t sockfd, int level, int optname, void *optval, socklen_t *optlen);
INTDEF errno_t LIBCCALL sys_setsockopt(fd_t sockfd, int level, int optname, void const *optval, socklen_t optlen);
INTDEF errno_t LIBCCALL sys_shutdown(fd_t sockfd, int how);
INTDEF fd_t LIBCCALL Xsys_socket(int domain, int type, int protocol);
INTDEF void LIBCCALL Xsys_bind(fd_t sockfd, struct sockaddr const *addr, socklen_t len);
INTDEF void LIBCCALL Xsys_listen(fd_t sockfd, int max_backlog);
INTDEF fd_t LIBCCALL Xsys_accept4(fd_t sockfd, struct sockaddr *addr, socklen_t *len, int flags);
INTDEF fd_t LIBCCALL Xsys_accept(fd_t sockfd, struct sockaddr *addr, socklen_t *len);
INTDEF void LIBCCALL Xsys_connect(fd_t sockfd, struct sockaddr const *addr, socklen_t len);
INTDEF socklen_t LIBCCALL Xsys_xgetsockname(fd_t sockfd, struct sockaddr *addr, socklen_t len);
INTDEF socklen_t LIBCCALL Xsys_xgetpeername(fd_t sockfd, struct sockaddr *addr, socklen_t len);
INTDEF void LIBCCALL Xsys_getsockname(fd_t sockfd, struct sockaddr *addr, socklen_t *len);
INTDEF void LIBCCALL Xsys_getpeername(fd_t sockfd, struct sockaddr *addr, socklen_t *len);
INTDEF size_t LIBCCALL Xsys_sendto(fd_t sockfd, void const *buf, size_t buflen, int flags, struct sockaddr const *addr, socklen_t len);
INTDEF size_t LIBCCALL Xsys_send(fd_t sockfd, void const *buf, size_t buflen, int flags);
INTDEF size_t LIBCCALL Xsys_recvfrom(fd_t sockfd, void *buf, size_t buflen, int flags, struct sockaddr *addr, socklen_t *len);
INTDEF size_t LIBCCALL Xsys_recv(fd_t sockfd, void *buf, size_t buflen, int flags);
INTDEF socklen_t LIBCCALL Xsys_xgetsockopt(fd_t sockfd, int level, int optname, void *optval, socklen_t optlen);
INTDEF void LIBCCALL Xsys_getsockopt(fd_t sockfd, int level, int optname, void *optval, socklen_t *optlen);
INTDEF void LIBCCALL Xsys_setsockopt(fd_t sockfd, int level, int optname, void const *optval, socklen_t optlen);


DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_SYSTEM_H */
