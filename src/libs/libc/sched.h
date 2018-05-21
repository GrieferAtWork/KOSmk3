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
#ifndef GUARD_LIBS_LIBC_SCHED_H
#define GUARD_LIBS_LIBC_SCHED_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     SCHED                                                                             */
/* ===================================================================================== */
struct rusage;
struct task_segment;
struct rlimit;
struct rlimit64;
struct cpu_context;
INTDEF int LIBCCALL libc_nice(int inc);
INTDEF pid_t LIBCCALL libc_getppid(void);
INTDEF pid_t LIBCCALL libc_getpgrp(void);
INTDEF pid_t LIBCCALL libc_getpgid(pid_t pid);
INTDEF int LIBCCALL libc_setpgrp(void);
INTDEF int LIBCCALL libc_setpgid(pid_t pid, pid_t pgid);
INTDEF pid_t LIBCCALL libc_setsid(void);
INTDEF pid_t LIBCCALL libc_getsid(pid_t pid);
INTDEF pid_t LIBCCALL libc_fork(void);
INTDEF __ATTR_RETURNS_TWICE pid_t LIBCCALL libc_vfork(void);
INTDEF pid_t LIBCCALL libc_wait(int *wstatus);
INTDEF pid_t LIBCCALL libc_waitpid(pid_t pid, int *wstatus, int options);
INTDEF int LIBCCALL libc_waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
INTDEF pid_t LIBCCALL libc_wait3(int *wstatus, int options, struct rusage *usage);
INTDEF pid_t LIBCCALL libc_wait4(pid_t pid, int *wstatus, int options, struct rusage *usage);
INTDEF pid_t LIBCCALL libc_clone(int (LIBCCALL *fn)(void *arg), void *child_stack, int flags, void *arg, ...);
INTDEF pid_t LIBCCALL libc_xclone(struct cpu_context *context, syscall_ulong_t flags, pid_t *parent_tidptr, void *tls_val, pid_t *child_tidptr);
INTDEF int LIBCCALL libc_unshare(int flags);
INTDEF cpuid_t LIBCCALL libc_sched_getcpu(void);
INTDEF int LIBCCALL libc_setns(fd_t fd, int nstype);
INTDEF int LIBCCALL libc_sched_setparam(pid_t pid, struct sched_param const *param);
INTDEF int LIBCCALL libc_sched_getparam(pid_t pid, struct sched_param *param);
INTDEF int LIBCCALL libc_sched_setscheduler(pid_t pid, int policy, struct sched_param const *param);
INTDEF int LIBCCALL libc_sched_getscheduler(pid_t pid);
INTDEF int LIBCCALL libc_sched_yield(void);
INTDEF int LIBCCALL libc_sched_get_priority_max(int algorithm);
INTDEF int LIBCCALL libc_sched_get_priority_min(int algorithm);
INTDEF int LIBCCALL libc_sched_setaffinity(pid_t pid, size_t cpusetsize, __cpu_set_t const *cpuset);
INTDEF int LIBCCALL libc_sched_getaffinity(pid_t pid, size_t cpusetsize, __cpu_set_t *cpuset);
INTDEF int LIBCCALL libc_sched_rr_get_interval(pid_t pid, struct timespec32 *t);
INTDEF int LIBCCALL libc_sched_rr_get_interval64(pid_t pid, struct timespec64 *t);
INTDEF ATTR_RETNONNULL ATTR_CONST struct task_segment *LIBCCALL libc_current(void);
INTDEF ATTR_CONST pid_t LIBCCALL libc_gettid(void);
INTDEF ATTR_CONST struct process_environ *LIBCCALL libc_procenv(void);
INTDEF int LIBCCALL libc_getrlimit(int resource, struct rlimit *rlimits);
INTDEF int LIBCCALL libc_setrlimit(int resource, struct rlimit const *rlimits);
INTDEF int LIBCCALL libc_getrusage(int who, struct rusage *usage);
INTDEF int LIBCCALL libc_getpriority(int which, id_t who);
INTDEF int LIBCCALL libc_setpriority(int which, id_t who, int prio);
INTDEF int LIBCCALL libc_getrlimit64(int resource, struct rlimit64 *rlimits);
INTDEF int LIBCCALL libc_setrlimit64(int resource, struct rlimit64 const *rlimits);
INTDEF int LIBCCALL libc_system(char const *command);
INTDEF int LIBCCALL libc_dos_system(char const *command);
INTDEF int LIBCCALL libc_Xnice(int inc);
INTDEF pid_t LIBCCALL libc_Xgetpgid(pid_t pid);
INTDEF void LIBCCALL libc_Xsetpgrp(void);
INTDEF void LIBCCALL libc_Xsetpgid(pid_t pid, pid_t pgid);
INTDEF pid_t LIBCCALL libc_Xsetsid(void);
INTDEF pid_t LIBCCALL libc_Xgetsid(pid_t pid);
INTDEF pid_t LIBCCALL libc_Xfork(void);
INTDEF __ATTR_RETURNS_TWICE pid_t LIBCCALL libc_Xvfork(void);
INTDEF pid_t LIBCCALL libc_Xwait(int *wstatus);
INTDEF pid_t LIBCCALL libc_Xwaitpid(pid_t pid, int *wstatus, int options);
INTDEF int LIBCCALL libc_Xwaitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
INTDEF pid_t LIBCCALL libc_Xwait3(int *wstatus, int options, struct rusage *usage);
INTDEF pid_t LIBCCALL libc_Xwait4(pid_t pid, int *wstatus, int options, struct rusage *usage);
INTDEF pid_t LIBCCALL libc_Xclone(int (LIBCCALL *fn)(void *arg), void *child_stack, int flags, void *arg, ...);
INTDEF pid_t LIBCCALL libc_Xxclone(struct cpu_context *context, syscall_ulong_t flags, pid_t *parent_tidptr, void *tls_val, pid_t *child_tidptr);
INTDEF void LIBCCALL libc_Xunshare(int flags);
INTDEF cpuid_t LIBCCALL libc_Xsched_getcpu(void);
INTDEF void LIBCCALL libc_Xsetns(fd_t fd, int nstype);
INTDEF void LIBCCALL libc_Xsched_setparam(pid_t pid, struct sched_param const *param);
INTDEF void LIBCCALL libc_Xsched_getparam(pid_t pid, struct sched_param *param);
INTDEF void LIBCCALL libc_Xsched_setscheduler(pid_t pid, int policy, struct sched_param const *param);
INTDEF int LIBCCALL libc_Xsched_getscheduler(pid_t pid);
INTDEF void LIBCCALL libc_Xsched_setaffinity(pid_t pid, size_t cpusetsize, __cpu_set_t const *cpuset);
INTDEF void LIBCCALL libc_Xsched_getaffinity(pid_t pid, size_t cpusetsize, __cpu_set_t *cpuset);
INTDEF void LIBCCALL libc_Xsched_rr_get_interval(pid_t pid, struct timespec32 *t);
INTDEF void LIBCCALL libc_Xsched_rr_get_interval64(pid_t pid, struct timespec64 *t);
INTDEF void LIBCCALL libc_Xgetrlimit(int resource, struct rlimit *rlimits);
INTDEF void LIBCCALL libc_Xsetrlimit(int resource, struct rlimit const *rlimits);
INTDEF void LIBCCALL libc_Xgetrusage(int who, struct rusage *usage);
INTDEF int LIBCCALL libc_Xgetpriority(int which, id_t who);
INTDEF void LIBCCALL libc_Xsetpriority(int which, id_t who, int prio);
INTDEF void LIBCCALL libc_Xgetrlimit64(int resource, struct rlimit64 *rlimits);
INTDEF void LIBCCALL libc_Xsetrlimit64(int resource, struct rlimit64 const *rlimits);
INTDEF int LIBCCALL libc_Xsystem(char const *command);

INTDEF char const libc_path_bin_sh[];
INTDEF char const libc_path_bin_busybox[];
INTDEF char const libc_str_sh[];
INTDEF char const libc_str_dashc[];


INTDEF uintptr_t LIBCCALL libc_beginthreadex(void *UNUSED(sec), u32 UNUSED(stacksz), u32 (ATTR_STDCALL *entry)(void *arg), void *arg, u32 UNUSED(flags), u32 *threadaddr);
INTDEF uintptr_t LIBCCALL libc_beginthread(void (LIBCCALL *entry)(void *arg), u32 stacksz, void *arg);
INTDEF void LIBCCALL libc_endthread(void);
INTDEF void LIBCCALL libc_endthreadex(u32 exitcode);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_SCHED_H */
