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
#ifndef GUARD_LIBS_LIBC_POSIX_SIGNAL_H
#define GUARD_LIBS_LIBC_POSIX_SIGNAL_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     SIGNAL                                                                            */
/* ===================================================================================== */
#ifndef __sighandler_t_defined
#define __sighandler_t_defined 1
typedef __sighandler_t sighandler_t;
#endif /* !__sighandler_t_defined */
struct sigstack;
struct sigaltstack;
INTDEF int LIBCCALL libc_signo_dos2kos(int dos_signo);
INTDEF int LIBCCALL libc_sigms_dos2kos(int dos_sigms);
INTDEF int LIBCCALL libc_raise(int sig);
INTDEF int LIBCCALL libd_raise(int sig);
INTDEF sighandler_t LIBCCALL libc_sysv_signal(int sig, sighandler_t handler);
INTDEF sighandler_t LIBCCALL libd_sysv_signal(int sig, sighandler_t handler);
INTDEF sighandler_t LIBCCALL libc_bsd_signal(int sig, sighandler_t handler);
INTDEF sighandler_t LIBCCALL libd_bsd_signal(int sig, sighandler_t handler);
INTDEF sighandler_t LIBCCALL libc_signal(int sig, sighandler_t handler);
INTDEF sighandler_t LIBCCALL libd_signal(int sig, sighandler_t handler);
INTDEF sighandler_t LIBCCALL libc_ssignal(int sig, sighandler_t handler);
INTDEF sighandler_t LIBCCALL libd_ssignal(int sig, sighandler_t handler);
INTDEF sighandler_t LIBCCALL libc_sigset(int sig, sighandler_t disp);
INTDEF sighandler_t LIBCCALL libd_sigset(int sig, sighandler_t disp);
INTDEF int LIBCCALL libc_gsignal(int sig);
INTDEF int LIBCCALL libd_gsignal(int sig);
INTDEF int LIBCCALL libc_sigblock(int mask);
INTDEF int LIBCCALL libd_sigblock(int mask);
INTDEF int LIBCCALL libc_sigsetmask(int mask);
INTDEF int LIBCCALL libd_sigsetmask(int mask);
INTDEF int LIBCCALL libc_siggetmask(void);
DATDEF char const *const sys_siglist[_NSIG];
INTDEF ATTR_NORETURN void LIBCCALL libc_sigreturn(struct sigcontext const *scp);
INTDEF int LIBCCALL libc_sigpause(int sig_or_mask, int is_sig);
INTDEF int LIBCCALL libd_sigpause(int sig_or_mask, int is_sig);
INTDEF int LIBCCALL libc_xpg_sigpause(int sig);
INTDEF int LIBCCALL libd_xpg_sigpause(int sig);
INTDEF int LIBCCALL libc_kill(pid_t pid, int sig);
INTDEF int LIBCCALL libd_kill(pid_t pid, int sig);
INTDEF int LIBCCALL libc_killpg(pid_t pgrp, int sig);
INTDEF int LIBCCALL libd_killpg(pid_t pgrp, int sig);
INTDEF int LIBCCALL libc_sigemptyset(sigset_t *set);
INTDEF int LIBCCALL libc_sigfillset(sigset_t *set);
INTDEF int LIBCCALL libc_sigaddset(sigset_t *set, int signo);
INTDEF int LIBCCALL libc_sigdelset(sigset_t *set, int signo);
INTDEF int LIBCCALL libc_sigismember(sigset_t const *set, int signo);
INTDEF int LIBCCALL libc_sigprocmask(int how, sigset_t const *set, sigset_t *oset);
INTDEF int LIBCCALL libd_sigprocmask(int how, sigset_t const *set, sigset_t *oset);
INTDEF int LIBCCALL libc_sigsuspend(sigset_t const *set);
INTDEF int LIBCCALL libd_sigsuspend(sigset_t const *set);
INTDEF int LIBCCALL libc_sigaction(int sig, struct sigaction const *act, struct sigaction *oact);
INTDEF int LIBCCALL libd_sigaction(int sig, struct sigaction const *act, struct sigaction *oact);
INTDEF int LIBCCALL libc_sigpending(sigset_t *set);
INTDEF errno_t LIBCCALL libc_sigwait(sigset_t const *__restrict set, int *__restrict sig);
INTDEF derrno_t LIBCCALL libd_sigwait(sigset_t const *__restrict set, int *__restrict sig);
INTDEF int LIBCCALL libc_sigisemptyset(sigset_t const *set);
INTDEF int LIBCCALL libc_sigandset(sigset_t *set, sigset_t const *left, sigset_t const *right);
INTDEF int LIBCCALL libc_sigorset(sigset_t *set, sigset_t const *left, sigset_t const *right);
INTDEF int LIBCCALL libc_sigwaitinfo(sigset_t const *__restrict set, siginfo_t *__restrict info);
INTDEF int LIBCCALL libd_sigwaitinfo(sigset_t const *__restrict set, siginfo_t *__restrict info);
INTDEF int LIBCCALL libc_sigtimedwait(sigset_t const *__restrict set, siginfo_t *__restrict info, struct timespec32 const *timeout);
INTDEF int LIBCCALL libd_sigtimedwait(sigset_t const *__restrict set, siginfo_t *__restrict info, struct timespec32 const *timeout);
INTDEF int LIBCCALL libc_sigtimedwait64(sigset_t const *__restrict set, siginfo_t *__restrict info, struct timespec64 const *timeout);
INTDEF int LIBCCALL libd_sigtimedwait64(sigset_t const *__restrict set, siginfo_t *__restrict info, struct timespec64 const *timeout);
INTDEF int LIBCCALL libc_sigqueue(pid_t pid, int sig, union sigval const val);
INTDEF int LIBCCALL libd_sigqueue(pid_t pid, int sig, union sigval const val);
INTDEF int LIBCCALL libc_sigqueueinfo(pid_t pid, int sig, siginfo_t const *info);
INTDEF int LIBCCALL libd_sigqueueinfo(pid_t pid, int sig, siginfo_t const *info);
INTDEF int LIBCCALL libc_tgsigqueueinfo(pid_t tgid, pid_t tid, int sig, siginfo_t const *info);
INTDEF int LIBCCALL libd_tgsigqueueinfo(pid_t tgid, pid_t tid, int sig, siginfo_t const *info);
INTDEF void LIBCCALL libc_psignal(int sig, char const *s);
INTDEF void LIBCCALL libd_psignal(int sig, char const *s);
INTDEF void LIBCCALL libc_psiginfo(siginfo_t const *pinfo, char const *s);
INTDEF void LIBCCALL libd_psiginfo(siginfo_t const *pinfo, char const *s);
INTDEF int LIBCCALL libc_siginterrupt(int sig, int interrupt);
INTDEF int LIBCCALL libd_siginterrupt(int sig, int interrupt);
INTDEF int LIBCCALL libc_sigstack(struct sigstack *ss, struct sigstack *oss);
INTDEF int LIBCCALL libc_sigaltstack(struct sigaltstack const *ss, struct sigaltstack *oss);
INTDEF int LIBCCALL libc_sighold(int sig);
INTDEF int LIBCCALL libd_sighold(int sig);
INTDEF int LIBCCALL libc_sigrelse(int sig);
INTDEF int LIBCCALL libd_sigrelse(int sig);
INTDEF int LIBCCALL libc_sigignore(int sig);
INTDEF int LIBCCALL libd_sigignore(int sig);
INTDEF int LIBCCALL libc_current_sigrtmin(void);
INTDEF int LIBCCALL libc_current_sigrtmax(void);
INTDEF void LIBCCALL libc_Xraise(int sig);
INTDEF sighandler_t LIBCCALL libc_Xsysv_signal(int sig, sighandler_t handler);
INTDEF sighandler_t LIBCCALL libc_Xbsd_signal(int sig, sighandler_t handler);
INTDEF sighandler_t LIBCCALL libc_Xsignal(int sig, sighandler_t handler);
INTDEF sighandler_t LIBCCALL libc_Xssignal(int sig, sighandler_t handler);
INTDEF sighandler_t LIBCCALL libc_Xsigset(int sig, sighandler_t disp);
INTDEF void LIBCCALL libc_Xgsignal(int sig);
INTDEF void LIBCCALL libc_Xsigblock(unsigned int mask);
INTDEF void LIBCCALL libc_Xsigsetmask(unsigned int mask);
INTDEF unsigned int LIBCCALL libc_Xsiggetmask(void);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xsigpause(int sig_or_mask, int is_sig);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xxpg_sigpause(int sig);
INTDEF void LIBCCALL libc_Xkill(pid_t pid, int sig);
INTDEF void LIBCCALL libc_Xkillpg(pid_t pgrp, int sig);
INTDEF void LIBCCALL libc_Xsigprocmask(int how, sigset_t const *set, sigset_t *oset);
INTDEF ATTR_NORETURN void LIBCCALL libc_Xsigsuspend(sigset_t const *set);
INTDEF void LIBCCALL libc_Xsigaction(int sig, struct sigaction const *act, struct sigaction *oact);
INTDEF void LIBCCALL libc_Xsigpending(sigset_t *set);
INTDEF void LIBCCALL libc_Xsigwait(sigset_t const *__restrict set, int *__restrict sig);
INTDEF unsigned int LIBCCALL libc_Xsigwaitinfo(sigset_t const *__restrict set, siginfo_t *__restrict info);
INTDEF unsigned int LIBCCALL libc_Xsigtimedwait(sigset_t const *__restrict set, siginfo_t *__restrict info, struct timespec32 const *timeout);
INTDEF unsigned int LIBCCALL libc_Xsigtimedwait64(sigset_t const *__restrict set, siginfo_t *__restrict info, struct timespec64 const *timeout);
INTDEF void LIBCCALL libc_Xsigqueue(pid_t pid, int sig, union sigval const val);
INTDEF void LIBCCALL libc_Xsigqueueinfo(pid_t pid, int sig, siginfo_t const *info);
INTDEF void LIBCCALL libc_Xtgsigqueueinfo(pid_t tgid, pid_t tid, int sig, siginfo_t const *info);
INTDEF void LIBCCALL libc_Xsiginterrupt(int sig, int interrupt);
INTDEF void LIBCCALL libc_Xsigstack(struct sigstack *ss, struct sigstack *oss);
INTDEF void LIBCCALL libc_Xsigaltstack(struct sigaltstack const *ss, struct sigaltstack *oss);
INTDEF void LIBCCALL libc_Xsighold(int sig);
INTDEF void LIBCCALL libc_Xsigrelse(int sig);
INTDEF void LIBCCALL libc_Xsigignore(int sig);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_POSIX_SIGNAL_H */
