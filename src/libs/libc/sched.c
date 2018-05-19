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
#ifndef GUARD_LIBS_LIBC_SCHED_C
#define GUARD_LIBS_LIBC_SCHED_C 1
#define __EXPOSE_CPU_CONTEXT 1

#include "libc.h"
#include "sched.h"
#include "errno.h"
#include "system.h"
#include "unistd.h"
#include "exec.h"
#include "exit.h"
#include "malloc.h"

#include <errno.h>
#include <bits/dos-errno.h>
#include <kos/types.h>
#include <kos/context.h>

DECL_BEGIN

#if defined(__x86_64__)
#error "TODO"
#elif defined(__i386__)
INTERN ATTR_NORETURN void FCALL
libc_thread_entry(int (LIBCCALL *fn)(void *arg), void *arg);
PRIVATE void LIBCCALL
arch_setup_context(struct cpu_context *__restrict context,
                   int (LIBCCALL *fn)(void *arg),
                   void *child_stack, void *arg) {
 context->c_eip           = (uintptr_t)&libc_thread_entry;
 context->c_gpregs.gp_ecx = (uintptr_t)fn;
 context->c_gpregs.gp_edx = (uintptr_t)arg;
 context->c_gpregs.gp_esp = (uintptr_t)child_stack;
 /* NOTE: The kernel uses default values for segments that are set
  *       to ZERO, in other words meaning that we're already done! */
}
#else
#error "Unsupported architecture"
#endif

INTERN pid_t LIBCCALL
libc_clone(int (LIBCCALL *fn)(void *arg),
           void *child_stack, int flags,
           void *arg, ...) {
 struct cpu_context child_context;
 va_list args; pid_t *ptid,*ctid; void *tls;
 va_start(args,arg);
 ptid = (flags & CLONE_PARENT_SETTID) ? va_arg(args,pid_t *) : NULL;
 tls  = (flags & CLONE_SETTLS) ? va_arg(args,void *) : NULL;
 ctid = (flags & (CLONE_CHILD_SETTID|CLONE_CHILD_CLEARTID)) ? va_arg(args,pid_t *) : NULL;
 va_end(args);
 /* ZERO-initialize the child's CPU context. */
 libc_memset(&child_context,0,sizeof(struct cpu_context));
 /* Do arch-specific initialization of the child's CPU context. */
 arch_setup_context(&child_context,fn,child_stack,arg);
 /* Create the child process. */
 return Esys_clone(&child_context,flags,ptid,tls,ctid);
}


INTERN int LIBCCALL libc_nice(int inc) { libc_seterrno(ENOSYS); return -1; }
INTERN pid_t LIBCCALL libc_getpgrp(void) { return libc_getpgid(0); }
INTERN int LIBCCALL libc_setpgrp(void) { return libc_setpgid(0,0); }
INTERN pid_t LIBCCALL libc_wait(int *wstatus) { return libc_wait4(-1,wstatus,0,NULL); }
INTERN pid_t LIBCCALL libc_waitpid(pid_t pid, int *wstatus, int options) { return libc_wait4(pid,wstatus,options,NULL); }
INTERN int LIBCCALL libc_waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options) { return Esys_waitid(idtype,id,infop,options,NULL); }
INTERN pid_t LIBCCALL libc_wait3(int *wstatus, int options, struct rusage *usage) { return libc_wait4(-1,wstatus,options,usage); }
INTERN cpuid_t LIBCCALL libc_sched_getcpu(void) {
 unsigned int result;
 errno_t error = sys_getcpu(&result,NULL);
 if (E_ISERR(error)) {
  libc_seterrno(error);
  return (cpuid_t)-1;
 }
 return (cpuid_t)result;
}
INTERN int LIBCCALL libc_setns(fd_t fd, int nstype) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_sched_setparam(pid_t pid, struct sched_param const *param) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_sched_getparam(pid_t pid, struct sched_param *param) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_sched_setscheduler(pid_t pid, int policy, struct sched_param const *param) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_sched_getscheduler(pid_t pid) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_sched_get_priority_max(int algorithm) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_sched_get_priority_min(int algorithm) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_sched_setaffinity(pid_t pid, size_t cpusetsize, __cpu_set_t const *cpuset) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_sched_getaffinity(pid_t pid, size_t cpusetsize, __cpu_set_t *cpuset) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_sched_rr_get_interval(pid_t pid, struct timespec32 *t) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_sched_rr_get_interval64(pid_t pid, struct timespec64 *t) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getrlimit(int resource, struct rlimit *rlimits) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_setrlimit(int resource, struct rlimit const *rlimits) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getrusage(int who, struct rusage *usage) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getpriority(int which, id_t who) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_setpriority(int which, id_t who, int prio) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_getrlimit64(int resource, struct rlimit64 *rlimits) { libc_seterrno(ENOSYS); return -1; }
INTERN int LIBCCALL libc_setrlimit64(int resource, struct rlimit64 const *rlimits) { libc_seterrno(ENOSYS); return -1; }

INTERN char const libc_path_bin_sh[]      = "::/bin/sh";
INTERN char const libc_path_bin_busybox[] = "::/bin/busybox";
INTERN char const libc_str_sh[]           = "sh";
INTERN char const libc_str_dashc[]        = "-c";

DEFINE_INTERN_ALIAS(libc_dos_system,libc_system); /* XXX: Own version? */
INTERN int LIBCCALL libc_system(char const *command) {
 pid_t child,error; int status;
 if ((child = libc_fork()) < 0) return -1; /* XXX: Use vfork()? */
 if (child == 0) {
  libc_execl(libc_path_bin_sh,libc_str_sh,libc_str_dashc,command,NULL);
  libc_execl(libc_path_bin_busybox,libc_str_sh,libc_str_dashc,command,NULL);
  /* NOTE: system() must return ZERO(0) if no command processor is available. */
  _libc_exit(command ? 127 : 0);
 }
 for (;;) {
  error = libc_waitpid(child,&status,WEXITED);
  if (error == child) break;
  if (error >= 0) continue;
  if (libc_geterrno() != EINTR)
      return -1;
 }
 return status;
}


/* Export symbols. */
EXPORT(getpgrp,                    libc_getpgrp);
EXPORT(setpgrp,                    libc_setpgrp);
EXPORT(nice,                       libc_nice);
EXPORT(clone,                      libc_clone);
EXPORT(wait,                       libc_wait);
EXPORT(waitpid,                    libc_waitpid);
EXPORT(waitid,                     libc_waitid);
EXPORT(wait3,                      libc_wait3);
EXPORT(unshare,                    libc_unshare);
EXPORT(sched_getcpu,               libc_sched_getcpu);
EXPORT(setns,                      libc_setns);
EXPORT(sched_setparam,             libc_sched_setparam);
EXPORT(sched_getparam,             libc_sched_getparam);
EXPORT(sched_setscheduler,         libc_sched_setscheduler);
EXPORT(sched_getscheduler,         libc_sched_getscheduler);
EXPORT(sched_get_priority_max,     libc_sched_get_priority_max);
EXPORT(sched_get_priority_min,     libc_sched_get_priority_min);
EXPORT(sched_setaffinity,          libc_sched_setaffinity);
EXPORT(sched_getaffinity,          libc_sched_getaffinity);
EXPORT(sched_rr_get_interval,      libc_sched_rr_get_interval);
EXPORT(sched_rr_get_interval64,    libc_sched_rr_get_interval64);
EXPORT(getrlimit,                  libc_getrlimit);
EXPORT(setrlimit,                  libc_setrlimit);
EXPORT(getrusage,                  libc_getrusage);
EXPORT(getpriority,                libc_getpriority);
EXPORT(setpriority,                libc_setpriority);
EXPORT(getrlimit64,                libc_getrlimit64);
EXPORT(setrlimit64,                libc_setrlimit64);
EXPORT(__KSYM(system),             libc_system);
EXPORT(__DSYM(system),             libc_dos_system);
EXPORT("?Yield%Context%Concurrency%%SAXXZ",libc_sched_yield);
EXPORT("?_DoYield%?$_SpinWait%$00%details%Concurrency%%IEAAXXZ",libc_sched_yield);
EXPORT("?_DoYield%?$_SpinWait%$0A%%details%Concurrency%%IEAAXXZ",libc_sched_yield);
EXPORT("?_SpinYield%Context%Concurrency%%SAXXZ",libc_sched_yield);
EXPORT("?_Yield%_Context%details%Concurrency%%SAXXZ",libc_sched_yield);
EXPORT("?VirtualProcessorId%Context%Concurrency%%SAIXZ",libc_sched_getcpu);
EXPORT("?_Id%_CurrentScheduler%details%Concurrency%%SAIXZ",libc_sched_getcpu);














EXPORT(Xnice,libc_Xnice);
CRT_EXCEPT int LIBCCALL libc_Xnice(int inc) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsetpgrp,libc_Xsetpgrp);
CRT_EXCEPT void LIBCCALL libc_Xsetpgrp(void) {
 Xsys_setpgid(0,0);
}


EXPORT(Xwait,libc_Xwait);
INTERN pid_t LIBCCALL
libc_Xwait(int *wstatus) {
 return libc_Xwait4(-1,wstatus,0,NULL);
}

EXPORT(Xwaitpid,libc_Xwaitpid);
INTERN pid_t LIBCCALL
libc_Xwaitpid(pid_t pid, int *wstatus, int options) {
 return libc_Xwait4(pid,wstatus,options,NULL);
}

EXPORT(Xwait3,libc_Xwait3);
INTERN pid_t LIBCCALL
libc_Xwait3(int *wstatus, int options, struct rusage *usage) {
 return libc_Xwait4(-1,wstatus,options,usage);
}

EXPORT(Xwaitid,libc_Xwaitid);
INTERN int LIBCCALL
libc_Xwaitid(idtype_t idtype, id_t id, siginfo_t *infop, int options) {
 return Xsys_waitid(idtype,id,infop,options,NULL);
}

EXPORT(Xclone,libc_Xclone);
CRT_EXCEPT pid_t LIBCCALL
libc_Xclone(int (LIBCCALL *fn)(void *arg),
            void *child_stack, int flags,
            void *arg, ...) {
 struct cpu_context child_context;
 va_list args; pid_t *ptid,*ctid; void *tls;
 va_start(args,arg);
 ptid = (flags & CLONE_PARENT_SETTID) ? va_arg(args,pid_t *) : NULL;
 tls  = (flags & CLONE_SETTLS) ? va_arg(args,void *) : NULL;
 ctid = (flags & (CLONE_CHILD_SETTID|CLONE_CHILD_CLEARTID)) ? va_arg(args,pid_t *) : NULL;
 va_end(args);
 /* ZERO-initialize the child's CPU context. */
 libc_memset(&child_context,0,sizeof(struct cpu_context));
 /* Do arch-specific initialization of the child's CPU context. */
 arch_setup_context(&child_context,fn,child_stack,arg);
 /* Create the child process. */
 return Xsys_clone(&child_context,flags,ptid,tls,ctid);
}

EXPORT(Xsched_getcpu,libc_Xsched_getcpu);
CRT_EXCEPT cpuid_t LIBCCALL libc_Xsched_getcpu(void) {
 unsigned int result;
 Xsys_getcpu(&result,NULL);
 return (cpuid_t)result;
}

EXPORT(Xsetns,libc_Xsetns);
CRT_EXCEPT void LIBCCALL libc_Xsetns(fd_t fd, int nstype) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsched_setparam,libc_Xsched_setparam);
CRT_EXCEPT void LIBCCALL
libc_Xsched_setparam(pid_t pid, struct sched_param const *param) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsched_getparam,libc_Xsched_getparam);
CRT_EXCEPT void LIBCCALL
libc_Xsched_getparam(pid_t pid, struct sched_param *param) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsched_setscheduler,libc_Xsched_setscheduler);
CRT_EXCEPT void LIBCCALL
libc_Xsched_setscheduler(pid_t pid, int policy, struct sched_param const *param) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsched_getscheduler,libc_Xsched_getscheduler);
CRT_EXCEPT int LIBCCALL
libc_Xsched_getscheduler(pid_t pid) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsched_setaffinity,libc_Xsched_setaffinity);
CRT_EXCEPT void LIBCCALL
libc_Xsched_setaffinity(pid_t pid, size_t cpusetsize, __cpu_set_t const *cpuset) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsched_getaffinity,libc_Xsched_getaffinity);
CRT_EXCEPT void LIBCCALL
libc_Xsched_getaffinity(pid_t pid, size_t cpusetsize, __cpu_set_t *cpuset) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsched_rr_get_interval64,libc_Xsched_rr_get_interval64);
CRT_EXCEPT void LIBCCALL
libc_Xsched_rr_get_interval64(pid_t pid, struct timespec64 *t) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsched_rr_get_interval,libc_Xsched_rr_get_interval);
CRT_EXCEPT void LIBCCALL
libc_Xsched_rr_get_interval(pid_t pid, struct timespec32 *t) {
 struct timespec64 t64;
 libc_Xsched_rr_get_interval64(pid,&t64);
 t->tv_sec  = t64.tv_sec;
 t->tv_nsec = t64.tv_nsec;
}

EXPORT(Xgetrlimit,libc_Xgetrlimit);
CRT_EXCEPT void LIBCCALL
libc_Xgetrlimit(int resource, struct rlimit *rlimits) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsetrlimit,libc_Xsetrlimit);
CRT_EXCEPT void LIBCCALL
libc_Xsetrlimit(int resource, struct rlimit const *rlimits) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xgetrusage,libc_Xgetrusage);
CRT_EXCEPT void LIBCCALL
libc_Xgetrusage(int who, struct rusage *usage) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xgetpriority,libc_Xgetpriority);
CRT_EXCEPT int LIBCCALL
libc_Xgetpriority(int which, id_t who) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsetpriority,libc_Xsetpriority);
CRT_EXCEPT void LIBCCALL
libc_Xsetpriority(int which, id_t who, int prio) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xgetrlimit64,libc_Xgetrlimit64);
CRT_EXCEPT void LIBCCALL
libc_Xgetrlimit64(int resource, struct rlimit64 *rlimits) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsetrlimit64,libc_Xsetrlimit64);
CRT_EXCEPT void LIBCCALL
libc_Xsetrlimit64(int resource, struct rlimit64 const *rlimits) {
 libc_error_throw(E_NOT_IMPLEMENTED);
}

EXPORT(Xsystem,libc_Xsystem);
CRT_EXCEPT int LIBCCALL
libc_Xsystem(char const *command) {
 pid_t child,error; int status;
 if ((child = libc_Xfork()) == 0) {
  libc_execl(libc_path_bin_sh,libc_str_sh,libc_str_dashc,command,NULL);
  libc_execl(libc_path_bin_busybox,libc_str_sh,libc_str_dashc,command,NULL);
  /* NOTE: system() must return ZERO(0) if no command processor is available. */
  _libc_exit(command ? 127 : 0);
 }
 for (;;) {
  LIBC_TRY {
   error = libc_Xwaitpid(child,&status,WEXITED);
   if (error == child)
       return status;
  } LIBC_CATCH_HANDLED (E_INTERRUPT) {
  }
 }
}

/* GLibc aliases. */
EXPORT_STRONG(__wait,libc_wait);
EXPORT_STRONG(__waitpid,libc_waitpid);
EXPORT_STRONG(__vfork,libc_vfork);
EXPORT_STRONG(__libc_vfork,libc_vfork);
EXPORT_STRONG(__sched_yield,libc_sched_yield);
EXPORT_STRONG(__sched_get_priority_min,libc_sched_get_priority_min);
EXPORT_STRONG(__sched_get_priority_max,libc_sched_get_priority_max);
EXPORT_STRONG(__fork,libc_fork);
EXPORT_STRONG(__libc_fork,libc_fork);
EXPORT_STRONG(__libc_system,libc_system);
EXPORT_STRONG(__clone,libc_clone);



struct dos_thread_data {
 u32 (ATTR_STDCALL *entry)(void *arg);
 void              *arg;
};
CRT_DOS int dos_thread_entry(void *arg) {
 struct dos_thread_data data = *(struct dos_thread_data *)arg;
 libc_free(arg);
 return (int)(*data.entry)(data.arg);
}

CRT_DOS uintptr_t LIBCCALL
libc_beginthreadex(void *UNUSED(sec), u32 UNUSED(stacksz),
                   u32 (ATTR_STDCALL *entry)(void *arg),
                   void *arg, u32 UNUSED(flags), u32 *threadaddr) {
 struct dos_thread_data *data; uintptr_t result;
 data = (struct dos_thread_data *)libc_malloc(sizeof(struct dos_thread_data));
 if unlikely(!data) return (uintptr_t)-1;
 data->entry = entry,data->arg = arg;
 result = (uintptr_t)libc_clone(&dos_thread_entry,CLONE_CHILDSTACK_AUTO,
                                 CLONE_VM|CLONE_FS|CLONE_FILES|
                                 CLONE_SIGHAND|CLONE_THREAD,arg);
 if (result == (uintptr_t)-1) libc_free(data),result = 0;
 if (threadaddr) *threadaddr = (u32)result;
 return result;
}

struct simple_thread_data {
 void (LIBCCALL *entry)(void *arg);
 void           *arg;
};
EXPORT(_beginthread,libc_beginthread);
CRT_DOS u32 ATTR_STDCALL simple_thread_entry(void *arg) {
 struct simple_thread_data data = *(struct simple_thread_data *)arg;
 libc_free(arg);
 (*data.entry)(data.arg);
 return 0;
}
EXPORT(_beginthreadex,libc_beginthreadex);
CRT_DOS uintptr_t LIBCCALL
libc_beginthread(void (LIBCCALL *entry)(void *arg), u32 stacksz, void *arg) {
 struct simple_thread_data *data; uintptr_t result;
 data = (struct simple_thread_data *)libc_malloc(sizeof(struct simple_thread_data));
 if unlikely(!data) return (uintptr_t)-1;
 data->entry = entry,data->arg = arg;
 result = libc_beginthreadex(NULL,stacksz,&simple_thread_entry,data,0,NULL);
 if (!result) libc_free(data);
 return result;
}
EXPORT(_endthread,libc_endthread);
CRT_DOS void LIBCCALL libc_endthread(void) {
 libc_endthreadex(0);
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_SCHED_C */
