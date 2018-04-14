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
#ifndef GUARD_LIBS_LIBC_POSIX_SIGNAL_C
#define GUARD_LIBS_LIBC_POSIX_SIGNAL_C 1

#include "libc.h"
#include "posix_signal.h"
#include "system.h"
#include "errno.h"
#include "unistd.h"

#include <bits/signum.h>
#include <errno.h>
#include <signal.h>

DECL_BEGIN

INTERN int LIBCCALL libc_signo_dos2kos(int dos_signo) {
 if (dos_signo == 22) return SIGABRT;
 return dos_signo;
}
INTERN int LIBCCALL
libc_sigms_dos2kos(int dos_sigms) {
 if ((unsigned int)dos_sigms & (1 << (22-1)))
      dos_sigms &= ~(1 << (22-1)),
      dos_sigms |= ~(1 << (SIGABRT-1));
 return dos_sigms;
}


PRIVATE sigset_t __sigintr;

INTERN int LIBCCALL libc_raise(int sig) { return libc_kill(libc_gettid(),sig); }
INTERN int LIBCCALL libd_raise(int sig) { return libc_raise(libc_signo_dos2kos(sig)); }
INTERN sighandler_t LIBCCALL
libc_sysv_signal(int sig, sighandler_t handler) {
 struct sigaction act,oact;
 if (handler == SIG_ERR) { libc_seterrno(EINVAL); return SIG_ERR; }
 act.sa_handler = handler;
 libc_sigemptyset(&act.sa_mask);
 act.sa_flags  = SA_ONESHOT|SA_NOMASK|SA_INTERRUPT;
 act.sa_flags &= ~SA_RESTART;
 if (libc_sigaction(sig,&act,&oact) < 0)
     return SIG_ERR;
 return oact.sa_handler;
}
INTERN sighandler_t LIBCCALL
libd_sysv_signal(int sig, sighandler_t handler) {
 return libc_sysv_signal(libc_signo_dos2kos(sig),handler);
}
INTERN sighandler_t LIBCCALL
libc_bsd_signal(int sig, sighandler_t handler) {
 struct sigaction act,oact;
 if (handler == SIG_ERR) { libc_seterrno(EINVAL); return SIG_ERR; }
 act.sa_handler = handler;
 libc_sigemptyset(&act.sa_mask);
 libc_sigaddset(&act.sa_mask,sig);
 act.sa_flags = libc_sigismember(&__sigintr,sig) ? 0 : SA_RESTART;
 if (libc_sigaction(sig,&act,&oact) < 0) return SIG_ERR;
 return oact.sa_handler;
}
INTERN sighandler_t LIBCCALL
libd_bsd_signal(int sig, sighandler_t handler) {
 return libc_bsd_signal(libc_signo_dos2kos(sig),handler);
}
INTERN sighandler_t LIBCCALL libc_sigset(int sig, sighandler_t disp) {
 struct sigaction act,oact; sigset_t set,oset;
 if (disp == SIG_ERR) { libc_seterrno(EINVAL); return SIG_ERR; }
 libc_sigemptyset(&set);
 libc_sigaddset(&set,sig);
 if (disp == SIG_HOLD) {
  if (libc_sigprocmask(SIG_BLOCK,&set,&oset) < 0) return SIG_ERR;
  if (libc_sigismember(&oset,sig)) return SIG_HOLD;
  if (libc_sigaction(sig,NULL,&oact) < 0) return SIG_ERR;
  return oact.sa_handler;
 }
 act.sa_handler = disp;
 libc_sigemptyset(&act.sa_mask);
 act.sa_flags = 0;
 if (libc_sigaction(sig,&act,&oact) < 0) return SIG_ERR;
 if (libc_sigprocmask(SIG_UNBLOCK,&set,&oset) < 0) return SIG_ERR;
 return libc_sigismember(&oset,sig) ? SIG_HOLD : oact.sa_handler;
}
INTERN sighandler_t LIBCCALL libd_sigset(int sig, sighandler_t disp) {
 return libc_sigset(libc_signo_dos2kos(sig),disp);
}
INTERN int LIBCCALL
libc_siginterrupt(int sig, int interrupt) {
 struct sigaction action;
 if (libc_sigaction(sig,(struct sigaction *) NULL,&action) < 0)
     return -1;
 if (interrupt) {
  libc_sigaddset(&__sigintr,sig);
  action.sa_flags &= ~SA_RESTART;
 } else {
  libc_sigdelset(&__sigintr,sig);
  action.sa_flags |= SA_RESTART;
 }
 if (libc_sigaction(sig,&action,(struct sigaction *)NULL) < 0)
     return -1;
 return 0;
}
INTERN int LIBCCALL
libd_siginterrupt(int sig, int interrupt) {
 return libc_siginterrupt(libc_signo_dos2kos(sig),interrupt);
}
DEFINE_INTERN_ALIAS(libd_signal,libd_sysv_signal);
DEFINE_INTERN_ALIAS(libc_signal,libc_bsd_signal);
DEFINE_INTERN_ALIAS(libc_ssignal,libc_bsd_signal);
DEFINE_INTERN_ALIAS(libd_ssignal,libd_bsd_signal);
DEFINE_INTERN_ALIAS(libc_gsignal,libc_raise);
DEFINE_INTERN_ALIAS(libd_gsignal,libd_raise);

INTERN int LIBCCALL libc_sigblock(int mask) {
 return FORWARD_SYSTEM_ERROR(sys_sigprocmask(SIG_BLOCK,(sigset_t *)&mask,NULL,sizeof(mask)));
}
INTERN int LIBCCALL libc_sigsetmask(int mask) {
 return FORWARD_SYSTEM_ERROR(sys_sigprocmask(SIG_SETMASK,(sigset_t *)&mask,NULL,sizeof(mask)));
}
INTERN int LIBCCALL libd_sigblock(int mask) {
 return libc_sigblock(libc_sigms_dos2kos(mask));
}
INTERN int LIBCCALL libd_sigsetmask(int mask) {
 return libc_sigsetmask(libc_sigms_dos2kos(mask));
}

INTERN int LIBCCALL libc_siggetmask(void) {
 int value,result;
 result = FORWARD_SYSTEM_ERROR(sys_sigprocmask(SIG_SETMASK,NULL,
                              (sigset_t *)&value,sizeof(value)));
 if (!result) result = value;
 return result;
}
PRIVATE int LIBCCALL set_single_signal_action(int sig, int how) {
 sigset_t set;
 libc_sigemptyset(&set);
 libc_sigaddset(&set,sig);
 return libc_sigprocmask(SIG_BLOCK,&set,NULL);
}
INTERN int LIBCCALL libc_sighold(int sig) { return set_single_signal_action(sig,SIG_BLOCK); }
INTERN int LIBCCALL libc_sigrelse(int sig) { return set_single_signal_action(sig,SIG_UNBLOCK); }
INTERN int LIBCCALL libc_sigignore(int sig) { return libc_bsd_signal(sig,SIG_IGN) == SIG_ERR ? -1 : 0; }
INTERN int LIBCCALL libd_sighold(int sig) { return libc_sighold(libc_signo_dos2kos(sig)); }
INTERN int LIBCCALL libd_sigrelse(int sig) { return libc_sigrelse(libc_signo_dos2kos(sig)); }
INTERN int LIBCCALL libd_sigignore(int sig) { return libc_sigignore(libc_signo_dos2kos(sig)); }
INTERN int LIBCCALL libc_current_sigrtmin(void) { return __SIGRTMIN; }
INTERN int LIBCCALL libc_current_sigrtmax(void) { return __SIGRTMAX; }
INTERN int LIBCCALL
libc_sigaltstack(struct sigaltstack const *ss,
                 struct sigaltstack *oss) {
 return FORWARD_SYSTEM_ERROR(sys_sigaltstack(ss,oss));
}
INTERN int LIBCCALL libc_sigsuspend(sigset_t const *set) {
 return FORWARD_SYSTEM_ERROR(sys_sigsuspend(set,sizeof(sigset_t)));
}
INTERN int LIBCCALL libd_sigsuspend(sigset_t const *set) {
 sigset_t fixed_set;
 LIBC_TRY {
  libc_memcpy(&fixed_set,set,sizeof(sigset_t));
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 *(int *)&fixed_set = libc_sigms_dos2kos(*(int *)&fixed_set);
 return libc_sigsuspend(&fixed_set);
}

INTERN int LIBCCALL libc_sigpause(int sig_or_mask, int is_sig) {
 if (is_sig) return libc_xpg_sigpause(sig_or_mask);
 return FORWARD_SYSTEM_ERROR(sys_sigsuspend((sigset_t *)&sig_or_mask,sizeof(sig_or_mask)));
}
INTERN int LIBCCALL libd_sigpause(int sig_or_mask, int is_sig) {
 return libc_sigpause(is_sig ? libc_signo_dos2kos(sig_or_mask)
                             : libc_sigms_dos2kos(sig_or_mask),
                      is_sig);
}
INTERN int LIBCCALL libc_xpg_sigpause(int sig) {
 sigset_t set;
 libc_sigemptyset(&set);
 libc_sigaddset(&set,sig);
 return libc_sigsuspend(&set);
}
INTERN int LIBCCALL
libd_xpg_sigpause(int sig) {
 return libc_xpg_sigpause(libc_signo_dos2kos(sig));
}
INTERN int LIBCCALL
libc_kill(pid_t pid, int sig) {
 return FORWARD_SYSTEM_ERROR(sys_kill(pid,sig));
}
INTERN int LIBCCALL
libd_kill(pid_t pid, int sig) {
 return libc_kill(pid,libc_signo_dos2kos(sig));
}
INTERN int LIBCCALL
libc_killpg(pid_t pgrp, int sig) {
 return libc_kill(-pgrp,sig);
}
INTERN int LIBCCALL
libd_killpg(pid_t pgrp, int sig) {
 return libd_kill(-pgrp,sig);
}
#define SIGSETFN(name,body,const) \
INTERN int LIBCCALL name(sigset_t const *set, int sig) { \
 unsigned long int mask = __sigmask(sig); \
 unsigned long int word = __sigword(sig); \
 return body; \
}
SIGSETFN(libc_sigismember,(set->__val[word] & mask) ? 1 : 0,const)
SIGSETFN(libc_sigaddset,((set->__val[word] |= mask), 0),)
SIGSETFN(libc_sigdelset,((set->__val[word] &= ~mask), 0),)
#undef SIGSETFN
INTERN int LIBCCALL libc_sigemptyset(sigset_t *set) { return __sigemptyset(set); }
INTERN int LIBCCALL libc_sigfillset(sigset_t *set) { return __sigfillset(set); }
INTERN int LIBCCALL libc_sigisemptyset(sigset_t const *set) { return __sigisemptyset(set); }
INTERN int LIBCCALL libc_sigandset(sigset_t *set, sigset_t const *left, sigset_t const *right) { return __sigandset(set,left,right); }
INTERN int LIBCCALL libc_sigorset(sigset_t *set, sigset_t const *left, sigset_t const *right) { return __sigorset(set,left,right); }

INTERN int LIBCCALL
libc_sigprocmask(int how,
                 sigset_t const *set,
                 sigset_t *oset) {
 return FORWARD_SYSTEM_ERROR(sys_sigprocmask(how,set,oset,sizeof(sigset_t)));
}
INTERN int LIBCCALL
libd_sigprocmask(int how,
                 sigset_t const *set,
                 sigset_t *oset) {
 int result;
 if (set) {
  sigset_t fixed_set;
  LIBC_TRY {
   libc_memcpy(&fixed_set,set,sizeof(sigset_t));
  } LIBC_EXCEPT (libc_except_errno()) {
   return -1;
  }
  *(int *)&fixed_set = libc_sigms_dos2kos(*(int *)&fixed_set);
  result = libc_sigprocmask(how,&fixed_set,oset);
 } else {
  result = libc_sigprocmask(how,NULL,oset);
 }
 return result;
}
INTERN int LIBCCALL
libc_sigaction(int sig,
               struct sigaction const *act,
               struct sigaction *oact) {
 return FORWARD_SYSTEM_ERROR(sys_sigaction(sig,act,oact,sizeof(sigset_t)));
}
INTERN int LIBCCALL
libd_sigaction(int sig,
               struct sigaction const *act,
               struct sigaction *oact) {
 return libc_sigaction(libc_signo_dos2kos(sig),act,oact);
}
INTERN int LIBCCALL
libc_sigpending(sigset_t *set) {
 return FORWARD_SYSTEM_ERROR(sys_sigpending(set,sizeof(sigset_t)));
}

INTERN int LIBCCALL
libc_sigtimedwait64(sigset_t const *__restrict set,
                    siginfo_t *__restrict info,
                    struct timespec64 const *timeout) {
 return FORWARD_SYSTEM_VALUE(sys_sigtimedwait(set,info,timeout,sizeof(sigset_t)));
}
INTERN int LIBCCALL
libd_sigtimedwait64(sigset_t const *__restrict set,
                    siginfo_t *__restrict info,
                    struct timespec64 const *timeout) {
 sigset_t fixed_set;
 LIBC_TRY {
  libc_memcpy(&fixed_set,set,sizeof(sigset_t));
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 *(int *)&fixed_set = libc_sigms_dos2kos(*(int *)&fixed_set);
 return libc_sigtimedwait64(&fixed_set,info,timeout);
}
INTERN int LIBCCALL
libc_sigtimedwait(sigset_t const *__restrict set,
                  siginfo_t *__restrict info,
                  struct timespec32 const *timeout) {
 struct timespec64 t64;
 if (!timeout) return libc_sigtimedwait64(set,info,NULL);
 LIBC_TRY {
  t64.tv_sec  = timeout->tv_sec;
  t64.tv_nsec = timeout->tv_nsec;
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return libc_sigtimedwait64(set,info,&t64);
}
INTERN int LIBCCALL
libd_sigtimedwait(sigset_t const *__restrict set,
                  siginfo_t *__restrict info,
                  struct timespec32 const *timeout) {
 struct timespec64 t64;
 if (!timeout) return libd_sigtimedwait64(set,info,NULL);
 LIBC_TRY {
  t64.tv_sec  = timeout->tv_sec;
  t64.tv_nsec = timeout->tv_nsec;
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
 return libd_sigtimedwait64(set,info,&t64);
}
INTERN int LIBCCALL
libc_sigwaitinfo(sigset_t const *__restrict set,
                 siginfo_t *__restrict info) {
 return libc_sigtimedwait64(set,info,NULL);
}
INTERN int LIBCCALL
libd_sigwaitinfo(sigset_t const *__restrict set,
                 siginfo_t *__restrict info) {
 return libd_sigtimedwait64(set,info,NULL);
}
INTERN errno_t LIBCCALL
libc_sigwait(sigset_t const *__restrict set,
             int *__restrict sig) {
 siginfo_t info; int error;
 error = libc_sigtimedwait64(set,&info,NULL);
 if (error >= 0) {
  LIBC_TRY {
   *sig = error;
  } LIBC_EXCEPT(libc_except_errno()) {
   return -1;
  }
  return 0;
 }
 return libc_geterrno();
}
INTERN derrno_t LIBCCALL
libd_sigwait(sigset_t const *__restrict set,
             int *__restrict sig) {
 int error; siginfo_t info;
 sigset_t fixed_set;
 LIBC_TRY {
  libc_memcpy(&fixed_set,set,sizeof(sigset_t));
  *(int *)&fixed_set = libc_sigms_dos2kos(*(int *)&fixed_set);
  error = libd_sigtimedwait64(&fixed_set,&info,NULL);
  if (error >= 0) { *sig = error; return 0; }
 } LIBC_EXCEPT (libc_except_errno()) {
 }
 return libc_dos_geterrno();
}
INTERN int LIBCCALL
libc_sigqueueinfo(pid_t pid, int sig, siginfo_t const *info) {
 return FORWARD_SYSTEM_ERROR(sys_rt_sigqueueinfo(pid,sig,info));
}
INTERN int LIBCCALL
libd_sigqueueinfo(pid_t pid, int sig, siginfo_t const *info) {
 siginfo_t realinfo;
 LIBC_TRY {
  libc_memcpy(&realinfo,info,sizeof(siginfo_t));
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 realinfo.si_signo = libc_signo_dos2kos(realinfo.si_signo);
 return libc_sigqueueinfo(pid,libc_signo_dos2kos(sig),&realinfo);
}
INTERN int LIBCCALL
libc_tgsigqueueinfo(pid_t tgid, pid_t tid,
                    int sig, siginfo_t const *info) {
 return FORWARD_SYSTEM_ERROR(sys_rt_tgsigqueueinfo(tgid,tid,sig,info));
}
INTERN int LIBCCALL
libd_tgsigqueueinfo(pid_t tgid, pid_t tid,
                    int sig, siginfo_t const *info) {
 siginfo_t realinfo;
 LIBC_TRY {
  libc_memcpy(&realinfo,info,sizeof(siginfo_t));
 } LIBC_EXCEPT (libc_except_errno()) {
  return -1;
 }
 realinfo.si_signo = libc_signo_dos2kos(realinfo.si_signo);
 return libc_tgsigqueueinfo(tgid,tid,libc_signo_dos2kos(sig),&realinfo);
}
INTERN int LIBCCALL
libc_sigqueue(pid_t pid, int sig, union sigval const val) {
 siginfo_t info;
 libc_memset(&info,0,sizeof(siginfo_t));
 info.si_value = val;
 info.si_code  = SI_QUEUE;
 return libc_sigqueueinfo(pid,sig,&info);
}
INTERN int LIBCCALL
libd_sigqueue(pid_t pid, int sig, union sigval const val) {
 return libc_sigqueue(pid,libc_signo_dos2kos(sig),val);
}
INTERN void LIBCCALL
libc_psignal(int sig, char const *s) {
 /* TODO */
}
INTERN void LIBCCALL
libc_psiginfo(siginfo_t const *pinfo, char const *s) {
 /* TODO */
}
INTERN void LIBCCALL
libd_psignal(int sig, char const *s) {
 libc_psignal(libc_signo_dos2kos(sig),s);
}
INTERN void LIBCCALL
libd_psiginfo(siginfo_t const *pinfo, char const *s) {
 siginfo_t info;
 libc_memcpy(&info,pinfo,sizeof(siginfo_t));
 info.si_signo = libc_signo_dos2kos(info.si_signo);
 libc_psiginfo(&info,s);
}
INTERN int LIBCCALL
libc_sigstack(struct sigstack *ss, struct sigstack *oss) {
 struct sigaltstack ass,aoss; int result;
 LIBC_TRY {
  if (ss) {
    ass.ss_flags = ss->ss_onstack ? SS_ONSTACK : SS_DISABLE;
    ass.ss_sp    = ss->ss_sp;
    ass.ss_size  = (size_t)-1;
  }
  result = libc_sigaltstack(ss ? &ass : NULL,oss ? &aoss : NULL);
  if (!result && oss) {
   oss->ss_onstack = !!(aoss.ss_flags&SS_ONSTACK);
   oss->ss_sp      = aoss.ss_sp;
  }
  return result;
 } LIBC_EXCEPT(libc_except_errno()) {
  return -1;
 }
}



EXPORT(__KSYM(raise),              libc_raise);
EXPORT(__DSYM(raise),              libd_raise);
EXPORT(__KSYM(sysv_signal),        libc_sysv_signal);
EXPORT(__DSYM(sysv_signal),        libd_sysv_signal);
EXPORT(__KSYM(bsd_signal),         libc_bsd_signal);
EXPORT(__DSYM(bsd_signal),         libd_bsd_signal);
EXPORT(__KSYM(signal),             libc_signal);
EXPORT(__DSYM(signal),             libd_signal);
EXPORT(__KSYM(ssignal),            libc_ssignal);
EXPORT(__DSYM(ssignal),            libd_ssignal);
EXPORT(__KSYM(gsignal),            libc_gsignal);
EXPORT(__DSYM(gsignal),            libd_gsignal);
EXPORT(__KSYM(sigblock),           libc_sigblock);
EXPORT(__DSYM(sigblock),           libd_sigblock);
EXPORT(__KSYM(sigsetmask),         libc_sigsetmask);
EXPORT(__DSYM(sigsetmask),         libd_sigsetmask);
EXPORT(siggetmask,                 libc_siggetmask);
EXPORT(__KSYM(sigpause),           libc_sigpause);
EXPORT(__DSYM(sigpause),           libd_sigpause);
EXPORT(__KSYM(__xpg_sigpause),     libc_xpg_sigpause);
EXPORT(__DSYM(__xpg_sigpause),     libd_xpg_sigpause);
EXPORT(__KSYM(kill),               libc_kill);
EXPORT(__DSYM(kill),               libd_kill);
EXPORT(__KSYM(killpg),             libc_killpg);
EXPORT(__DSYM(killpg),             libd_killpg);
EXPORT(sigemptyset,                libc_sigemptyset);
EXPORT(sigfillset,                 libc_sigfillset);
EXPORT(sigaddset,                  libc_sigaddset);
EXPORT(sigdelset,                  libc_sigdelset);
EXPORT(sigismember,                libc_sigismember);
EXPORT(__KSYM(sigprocmask),        libc_sigprocmask);
EXPORT(__DSYM(sigprocmask),        libd_sigprocmask);
EXPORT(__KSYM(sigsuspend),         libc_sigsuspend);
EXPORT(__DSYM(sigsuspend),         libd_sigsuspend);
EXPORT(__KSYM(sigaction),          libc_sigaction);
EXPORT(__DSYM(sigaction),          libd_sigaction);
EXPORT(sigpending,                 libc_sigpending);
EXPORT(__KSYM(sigwait),            libc_sigwait);
EXPORT(__DSYM(sigwait),            libd_sigwait);
EXPORT(sigisemptyset,              libc_sigisemptyset);
EXPORT(sigandset,                  libc_sigandset);
EXPORT(sigorset,                   libc_sigorset);
EXPORT(__KSYM(sigwaitinfo),        libc_sigwaitinfo);
EXPORT(__DSYM(sigwaitinfo),        libd_sigwaitinfo);
EXPORT(__KSYM(sigtimedwait),       libc_sigtimedwait);
EXPORT(__DSYM(sigtimedwait),       libd_sigtimedwait);
EXPORT(__KSYM(sigtimedwait64),     libc_sigtimedwait64);
EXPORT(__DSYM(sigtimedwait64),     libd_sigtimedwait64);
EXPORT(__KSYM(sigqueueinfo),       libc_sigqueueinfo);
EXPORT(__DSYM(sigqueueinfo),       libd_sigqueueinfo);
EXPORT(__KSYM(tgsigqueueinfo),     libc_tgsigqueueinfo);
EXPORT(__DSYM(tgsigqueueinfo),     libd_tgsigqueueinfo);
EXPORT(__KSYM(sigqueue),           libc_sigqueue);
EXPORT(__DSYM(sigqueue),           libd_sigqueue);
EXPORT(__KSYM(psignal),            libc_psignal);
EXPORT(__DSYM(psignal),            libd_psignal);
EXPORT(__KSYM(psiginfo),           libc_psiginfo);
EXPORT(__DSYM(psiginfo),           libd_psiginfo);
EXPORT(__KSYM(siginterrupt),       libc_siginterrupt);
EXPORT(__DSYM(siginterrupt),       libd_siginterrupt);
EXPORT(sigstack,                   libc_sigstack);
EXPORT(sigaltstack,                libc_sigaltstack);
EXPORT(__KSYM(sighold),            libc_sighold);
EXPORT(__DSYM(sighold),            libd_sighold);
EXPORT(__KSYM(sigrelse),           libc_sigrelse);
EXPORT(__DSYM(sigrelse),           libd_sigrelse);
EXPORT(__KSYM(sigignore),          libc_sigignore);
EXPORT(__DSYM(sigignore),          libd_sigignore);
EXPORT(__KSYM(sigset),             libc_sigset);
EXPORT(__DSYM(sigset),             libd_sigset);
EXPORT(__libc_current_sigrtmin,    libc_current_sigrtmin);
EXPORT(__libc_current_sigrtmax,    libc_current_sigrtmax);


PUBLIC char const *const sys_siglist[_NSIG] = {
};



EXPORT(Xraise,libc_Xraise);
CRT_EXCEPT void LIBCCALL libc_Xraise(int sig) {
 libc_Xkill(libc_gettid(),sig);
}

EXPORT(Xsysv_signal,libc_Xsysv_signal);
CRT_EXCEPT sighandler_t LIBCCALL
libc_Xsysv_signal(int sig, sighandler_t handler) {
 struct sigaction act,oact;
 if (handler == SIG_ERR)
     libc_error_throw(E_INVALID_ARGUMENT);
 act.sa_handler = handler;
 libc_sigemptyset(&act.sa_mask);
 act.sa_flags  = SA_ONESHOT|SA_NOMASK|SA_INTERRUPT;
 act.sa_flags &= ~SA_RESTART;
 libc_Xsigaction(sig,&act,&oact);
 return oact.sa_handler;
}

EXPORT(Xbsd_signal,libc_Xbsd_signal);
CRT_EXCEPT sighandler_t LIBCCALL
libc_Xbsd_signal(int sig, sighandler_t handler) {
 struct sigaction act,oact;
 if (handler == SIG_ERR)
     libc_error_throw(E_INVALID_ARGUMENT);
 act.sa_handler = handler;
 libc_sigemptyset(&act.sa_mask);
 libc_sigaddset(&act.sa_mask,sig);
 act.sa_flags = libc_sigismember(&__sigintr,sig) ? 0 : SA_RESTART;
 libc_Xsigaction(sig,&act,&oact);
 return oact.sa_handler;
}

EXPORT(Xsignal,libc_Xsignal);
DEFINE_INTERN_ALIAS(libc_Xsignal,libc_Xbsd_signal);

EXPORT(Xssignal,libc_Xssignal);
DEFINE_INTERN_ALIAS(libc_Xssignal,libc_Xbsd_signal);

EXPORT(Xsigset,libc_Xsigset);
CRT_EXCEPT sighandler_t LIBCCALL
libc_Xsigset(int sig, sighandler_t disp) {
 struct sigaction act,oact; sigset_t set,oset;
 if (disp == SIG_ERR)
     libc_error_throw(E_INVALID_ARGUMENT);
 libc_sigemptyset(&set);
 libc_sigaddset(&set,sig);
 if (disp == SIG_HOLD) {
  libc_Xsigprocmask(SIG_BLOCK,&set,&oset);
  if (libc_sigismember(&oset,sig))
      return SIG_HOLD;
  libc_Xsigaction(sig,NULL,&oact);
  return oact.sa_handler;
 }
 act.sa_handler = disp;
 libc_sigemptyset(&act.sa_mask);
 act.sa_flags = 0;
 libc_Xsigaction(sig,&act,&oact);
 libc_Xsigprocmask(SIG_UNBLOCK,&set,&oset);
 return libc_sigismember(&oset,sig) ? SIG_HOLD : oact.sa_handler;
}

EXPORT(Xgsignal,libc_Xgsignal);
DEFINE_INTERN_ALIAS(libc_Xgsignal,libc_Xraise);

EXPORT(Xsigblock,libc_Xsigblock);
CRT_EXCEPT void LIBCCALL libc_Xsigblock(unsigned int mask) {
 Xsys_sigprocmask(SIG_BLOCK,(sigset_t *)&mask,NULL,sizeof(mask));
}

EXPORT(Xsigsetmask,libc_Xsigsetmask);
CRT_EXCEPT void LIBCCALL libc_Xsigsetmask(unsigned int mask) {
 Xsys_sigprocmask(SIG_SETMASK,(sigset_t *)&mask,NULL,sizeof(mask));
}

EXPORT(Xsiggetmask,libc_Xsiggetmask);
CRT_EXCEPT unsigned int LIBCCALL libc_Xsiggetmask(void) {
 unsigned int result;
 Xsys_sigprocmask(SIG_SETMASK,NULL,(sigset_t *)&result,sizeof(result));
 return result;
}

EXPORT(Xsigpause,libc_Xsigpause);
CRT_EXCEPT ATTR_NORETURN void LIBCCALL
libc_Xsigpause(int sig_or_mask, int is_sig) {
 if (is_sig)
  libc_Xxpg_sigpause(sig_or_mask);
 else {
  Xsys_sigsuspend((sigset_t *)&sig_or_mask,sizeof(sig_or_mask));
 }
}

EXPORT(X__xpg_sigpause,libc_Xxpg_sigpause);
CRT_EXCEPT ATTR_NORETURN void LIBCCALL
libc_Xxpg_sigpause(int sig) {
 sigset_t set;
 libc_sigemptyset(&set);
 libc_sigaddset(&set,sig);
 libc_Xsigsuspend(&set);
}

EXPORT(Xkillpg,libc_Xkillpg);
CRT_EXCEPT void LIBCCALL
libc_Xkillpg(pid_t pgrp, int sig) {
 libc_Xkill(-pgrp,sig);
}

EXPORT(Xsigprocmask,libc_Xsigprocmask);
CRT_EXCEPT void LIBCCALL
libc_Xsigprocmask(int how,
                  sigset_t const *set,
                  sigset_t *oset) {
 Xsys_sigprocmask(how,set,oset,sizeof(sigset_t));
}

EXPORT(Xsigsuspend,libc_Xsigsuspend);
CRT_EXCEPT ATTR_NORETURN void LIBCCALL
libc_Xsigsuspend(sigset_t const *set) {
 Xsys_sigsuspend(set,sizeof(sigset_t));
}

EXPORT(Xsigaction,libc_Xsigaction);
CRT_EXCEPT void LIBCCALL
libc_Xsigaction(int sig, struct sigaction const *act, struct sigaction *oact) {
 Xsys_sigaction(sig,act,oact,sizeof(sigset_t));
}

EXPORT(Xsigpending,libc_Xsigpending);
CRT_EXCEPT void LIBCCALL
libc_Xsigpending(sigset_t *set) {
 Xsys_sigpending(set,sizeof(sigset_t));
}

EXPORT(Xsigwait,libc_Xsigwait);
CRT_EXCEPT void LIBCCALL
libc_Xsigwait(sigset_t const *__restrict set,
              int *__restrict sig) {
 siginfo_t info;
 *sig = (int)libc_Xsigtimedwait64(set,&info,NULL);
}

EXPORT(Xsigwaitinfo,libc_Xsigwaitinfo);
CRT_EXCEPT unsigned int LIBCCALL
libc_Xsigwaitinfo(sigset_t const *__restrict set,
                  siginfo_t *__restrict info) {
 return libc_Xsigtimedwait64(set,info,NULL);
}

EXPORT(Xsigtimedwait,libc_Xsigtimedwait);
CRT_EXCEPT unsigned int LIBCCALL
libc_Xsigtimedwait(sigset_t const *__restrict set,
                   siginfo_t *__restrict info,
                   struct timespec32 const *timeout) {
 struct timespec64 t64;
 if (!timeout)
      return libc_Xsigtimedwait64(set,info,NULL);
 t64.tv_sec  = timeout->tv_sec;
 t64.tv_nsec = timeout->tv_nsec;
 return libc_Xsigtimedwait64(set,info,&t64);
}

EXPORT(Xsigtimedwait64,libc_Xsigtimedwait64);
CRT_EXCEPT unsigned int LIBCCALL
libc_Xsigtimedwait64(sigset_t const *__restrict set,
                     siginfo_t *__restrict info,
                     struct timespec64 const *timeout) {
 return Xsys_sigtimedwait(set,info,timeout,sizeof(sigset_t));
}

EXPORT(Xsigqueue,libc_Xsigqueue);
CRT_EXCEPT void LIBCCALL
libc_Xsigqueue(pid_t pid, int sig, union sigval const val) {
 siginfo_t info;
 libc_memset(&info,0,sizeof(siginfo_t));
 info.si_value = val;
 info.si_code  = SI_QUEUE;
 libc_Xsigqueueinfo(pid,sig,&info);
}

EXPORT(Xsiginterrupt,libc_Xsiginterrupt);
CRT_EXCEPT void LIBCCALL
libc_Xsiginterrupt(int sig, int interrupt) {
 struct sigaction action;
 libc_Xsigaction(sig,(struct sigaction *) NULL,&action);
 if (interrupt) {
  libc_sigaddset(&__sigintr,sig);
  action.sa_flags &= ~SA_RESTART;
 } else {
  libc_sigdelset(&__sigintr,sig);
  action.sa_flags |= SA_RESTART;
 }
 libc_Xsigaction(sig,&action,(struct sigaction *) NULL);
}

CRT_EXCEPT void LIBCCALL
libc_Xsigstack(struct sigstack *ss,
               struct sigstack *oss) {
 struct sigaltstack ass,aoss;
 if (ss) {
   ass.ss_flags = ss->ss_onstack ? SS_ONSTACK : SS_DISABLE;
   ass.ss_sp    = ss->ss_sp;
   ass.ss_size  = (size_t)-1;
 }
 libc_Xsigaltstack(ss ? &ass : NULL,oss ? &aoss : NULL);
 if (oss) {
  oss->ss_onstack = !!(aoss.ss_flags&SS_ONSTACK);
  oss->ss_sp      = aoss.ss_sp;
 }
}


PRIVATE void LIBCCALL
Xset_single_signal_action(int sig, int how) {
 sigset_t set;
 libc_sigemptyset(&set);
 libc_sigaddset(&set,sig);
 libc_Xsigprocmask(SIG_BLOCK,&set,NULL);
}

EXPORT(Xsighold,libc_Xsighold);
CRT_EXCEPT void LIBCCALL libc_Xsighold(int sig) {
 Xset_single_signal_action(sig,SIG_BLOCK);
}

EXPORT(Xsigrelse,libc_Xsigrelse);
CRT_EXCEPT void LIBCCALL libc_Xsigrelse(int sig) {
 Xset_single_signal_action(sig,SIG_UNBLOCK);
}

EXPORT(Xsigignore,libc_Xsigignore);
CRT_EXCEPT void LIBCCALL libc_Xsigignore(int sig) {
 libc_Xbsd_signal(sig,SIG_IGN);
}



/* GLibc Aliases */
EXPORT_STRONG(__sigaction,libc_sigaction);
EXPORT_STRONG(__sigaddset,libc_sigaddset);
EXPORT_STRONG(__sigdelset,libc_sigdelset);
EXPORT_STRONG(__sigismember,libc_sigismember);
EXPORT_STRONG(__sigpause,libc_sigpause);
EXPORT_STRONG(__sigsuspend,libc_sigsuspend);


DECL_END

#endif /* !GUARD_LIBS_LIBC_POSIX_SIGNAL_C */
