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
#ifndef GUARD_LIBS_LIBC_TTY_C
#define GUARD_LIBS_LIBC_TTY_C 1

#include "libc.h"
#include "tty.h"
#include "errno.h"
#include "system.h"
#include "unistd.h"
#include "sched.h"
#include "exit.h"
#include "dirent.h"
#include "stat.h"
#include "rtl.h"

#include <bits/io-file.h>
#include <hybrid/list/list.h>
#include <kos/sched/mutex.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <termios.h>
#include <syslog.h>

DECL_BEGIN

EXPORT(openpty,libc_openpty);
CRT_TTY int LIBCCALL
libc_openpty(int *amaster, int *aslave, char *name,
             struct termios const *termp,
             struct winsize const *winp) {
#if __SIZEOF_INT__ > 4
#error FIXME
#endif
 s64 result = sys_xopenpty(name,termp,winp);
 if (E_ISERR(result)) { libc_seterrno(-(errno_t)result); return -1; }
 LIBC_TRY {
  *amaster = (int)result;
  *aslave  = (int)(result >> 32);
 } LIBC_EXCEPT(libc_except_errno()) {
  sys_close((int)result);
  sys_close((int)(result >> 32));
  return -1;
 }
 return 0;
}

EXPORT(forkpty,libc_forkpty);
CRT_TTY pid_t LIBCCALL
libc_forkpty(int *amaster, char *name,
             struct termios const *termp,
             struct winsize const *winp) {
 int COMPILER_IGNORE_UNINITIALIZED(master);
 int COMPILER_IGNORE_UNINITIALIZED(slave);
 pid_t pid;
 if (libc_openpty(&master,&slave,name,termp,winp) == -1)
     return -1;
 switch (pid = libc_fork()) {
 case -1:
  libc_close(master);
  libc_close(slave);
  return -1;
 case 0:
  /* Child process. */
  libc_close(master);
  if (libc_login_tty(slave))
      _libc_exit(1);
  return 0;
 default:
  /* Parent process.  */
  *amaster = master;
  libc_close(slave);
  return pid;
 }
}

EXPORT(ttyname,libc_ttyname);
PRIVATE char ttyname_buffer[32];
CRT_TTY char *LIBCCALL libc_ttyname(fd_t fd) {
 return libc_ttyname_r(fd,ttyname_buffer,sizeof(ttyname_buffer)) ? NULL : ttyname_buffer;
}

EXPORT(ttyname_r,libc_ttyname_r);
PRIVATE char const dev[] = "/dev";
CRT_TTY int LIBCCALL libc_ttyname_r(fd_t fd, char *buf, size_t buflen) {
 struct stat64 st; struct dirent *d; DIR *dirstream;
 int safe; dev_t rdev;
 if unlikely(buflen < (COMPILER_STRLEN(dev)+1)*sizeof(char)) { libc_seterrno(ERANGE); return ERANGE; }
 if unlikely(!libc_isatty(fd)) { libc_seterrno(ENOTTY); return ENOTTY; }
 if unlikely(libc_kfstat64(fd,&st) < 0) return libc_geterrno();
 if ((dirstream = libc_opendir(dev)) == NULL) return libc_geterrno();
 libc_memcpy(buf,dev,COMPILER_STRLEN(dev)*sizeof(char));
 buf[COMPILER_STRLEN(dev)] = '/';
 buflen -= (COMPILER_STRLEN(dev)+1)*sizeof(char);
 safe = libc_geterrno();
 rdev = st.st_rdev;
 while ((d = libc_readdir(dirstream)) != NULL) {
  if (d->d_ino64 == st.st_ino64 &&
      libc_strcmp(d->d_name,"stdin") != 0 &&
      libc_strcmp(d->d_name,"stdout") != 0 &&
      libc_strcmp(d->d_name,"stderr") != 0) {
   size_t needed = _D_EXACT_NAMLEN(d)+1;
   if (needed > buflen) {
    libc_closedir(dirstream);
    libc_seterrno(ERANGE);
    return ERANGE;
   }
   libc_memcpy(&buf[sizeof(dev)],d->d_name,(needed+1)*sizeof(char));
   if (libc_kstat64(buf,&st) == 0 &&
       S_ISCHR(st.st_mode) &&
       st.st_rdev == rdev) {
    /* Found it! */
    libc_closedir(dirstream);
    libc_seterrno(safe);
    return 0;
   }
  }
 }
 libc_closedir(dirstream);
 libc_seterrno(safe);
 return ENOTTY;
}

EXPORT(__KSYM(isatty),libc_isatty);
EXPORT(__DSYM(_isatty),libc_isatty);
CRT_TTY int LIBCCALL libc_isatty(fd_t fd) {
 struct termios term;
 return libc_tcgetattr(fd,&term) == 0;
}
EXPORT(tcgetpgrp,libc_tcgetpgrp);
CRT_TTY pid_t LIBCCALL libc_tcgetpgrp(fd_t fd) {
 pid_t pgrp;
 return libc_ioctl(fd,TIOCGPGRP,&pgrp) < 0 ? -1 : pgrp;
}

EXPORT(tcsetpgrp,libc_tcsetpgrp);
CRT_TTY int LIBCCALL
libc_tcsetpgrp(fd_t fd, pid_t pgrp_id) {
 return libc_ioctl(fd,TIOCSPGRP,&pgrp_id);
}

EXPORT(cfgetospeed,libc_cfgetospeed);
CRT_TTY speed_t LIBCCALL
libc_cfgetospeed(struct termios const *termios_p) {
 return termios_p->c_ospeed;
}

EXPORT(cfgetispeed,libc_cfgetispeed);
CRT_TTY speed_t LIBCCALL
libc_cfgetispeed(struct termios const *termios_p) {
 return termios_p->c_ispeed;
}

EXPORT(cfsetospeed,libc_cfsetospeed);
CRT_TTY int LIBCCALL
libc_cfsetospeed(struct termios *termios_p, speed_t speed) {
 termios_p->c_ospeed = speed;
 return 0;
}

EXPORT(cfsetispeed,libc_cfsetispeed);
CRT_TTY int LIBCCALL
libc_cfsetispeed(struct termios *termios_p, speed_t speed) {
 termios_p->c_ispeed = speed;
 return 0;
}

EXPORT(cfsetspeed,libc_cfsetspeed);
CRT_TTY int LIBCCALL
libc_cfsetspeed(struct termios *termios_p, speed_t speed) {
 termios_p->c_ospeed = termios_p->c_ispeed = speed;
 return 0;
}

EXPORT(tcgetattr,libc_tcgetattr);
CRT_TTY int LIBCCALL
libc_tcgetattr(fd_t fd, struct termios *termios_p) {
 return libc_ioctl(fd,TCGETS,termios_p);
}


#if TCSANOW == 0 && TCSADRAIN == 1 && TCSAFLUSH == 2 && \
    TCSETS+1 == TCSETSW && TCSETS+2 == TCSETSF
#define TSC_VALIDACTION(x) ((x) < 3)
#define TSC_ACTION(x)      (TCSETS+(x))
#else
#define TSC_VALIDACTION(x) ((x) < COMPILER_LENOF(tsc_actions))
#define TSC_ACTION(x)  tsc_actions[x]
PRIVATE int const tsc_actions[] = {
    [TCSANOW]   = TCSETS,
    [TCSADRAIN] = TCSETSW,
    [TCSAFLUSH] = TCSETSF,
};
#endif

EXPORT(tcsetattr,libc_tcsetattr);
CRT_TTY int LIBCCALL
libc_tcsetattr(fd_t fd, int optional_actions,
               struct termios const *termios_p) {
 if (!TSC_VALIDACTION((unsigned int)optional_actions)) {
  libc_seterrno(EINVAL);
  return -1;
 }
 return libc_ioctl(fd,TSC_ACTION(optional_actions),termios_p);
}

EXPORT(tcsendbreak,libc_tcsendbreak);
CRT_TTY int LIBCCALL
libc_tcsendbreak(fd_t fd, int duration) {
 if (duration <= 0) return libc_ioctl(fd,TCSBRK,0);
 return libc_ioctl(fd,TCSBRKP,(duration+99)/100);
}

EXPORT(tcdrain,libc_tcdrain);
CRT_TTY int LIBCCALL
libc_tcdrain(fd_t fd) {
 return libc_ioctl(fd,TCSBRK,1);
}

EXPORT(tcflush,libc_tcflush);
CRT_TTY int LIBCCALL
libc_tcflush(fd_t fd, int queue_selector) {
 return libc_ioctl(fd,TCFLSH,queue_selector);
}

EXPORT(tcflow,libc_tcflow);
CRT_TTY int LIBCCALL
libc_tcflow(fd_t fd, int action) {
 return libc_ioctl(fd,TCXONC,action);
}

EXPORT(tcgetsid,libc_tcgetsid);
CRT_TTY pid_t LIBCCALL libc_tcgetsid(fd_t fd) {
 pid_t sid;
 if (libc_ioctl(fd,TIOCGSID,&sid) < 0)
     return (pid_t)-1;
 return sid;
}

EXPORT(cfmakeraw,libc_cfmakeraw);
CRT_TTY void LIBCCALL
libc_cfmakeraw(struct termios *termios_p) {
 termios_p->c_iflag    &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
 termios_p->c_oflag    &= ~(OPOST);
 termios_p->c_lflag    &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
 termios_p->c_cflag    &= ~(CSIZE|PARENB);
 termios_p->c_cflag    |= CS8;
 termios_p->c_cc[VMIN]  = 1; /* Read returns when one byte was read. */
 termios_p->c_cc[VTIME] = 0;
}

EXPORT(getlogin,libc_getlogin);
CRT_TTY char *LIBCCALL libc_getlogin(void) {
 libc_seterrno(ENOSYS);
 return NULL;
}

EXPORT(ttyslot,libc_ttyslot);
CRT_TTY int LIBCCALL libc_ttyslot(void) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(getlogin_r,libc_getlogin_r);
CRT_TTY int LIBCCALL
libc_getlogin_r(char *name, size_t name_len) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(setlogin,libc_setlogin);
CRT_TTY int LIBCCALL
libc_setlogin(char const *name) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(vhangup,libc_vhangup);
CRT_TTY int LIBCCALL libc_vhangup(void) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(getpass,libc_getpass);
CRT_TTY char *LIBCCALL
libc_getpass(char const *__restrict prompt) {
 libc_seterrno(ENOSYS);
 return NULL;
}


EXPORT(login_tty,libc_login_tty);
CRT_TTY int LIBCCALL libc_login_tty(fd_t fd) {
 libc_setsid();
 if (libc_ioctl(fd,TIOCSCTTY,(char *)NULL) == -1)
     return -1;
 while (libc_dup2(fd,0) == -1 && libc_geterrno() == EBUSY);
 while (libc_dup2(fd,1) == -1 && libc_geterrno() == EBUSY);
 while (libc_dup2(fd,2) == -1 && libc_geterrno() == EBUSY);
 if (fd > 2) libc_close(fd);
 return 0;
}

EXPORT(login,libc_login);
CRT_TTY void LIBCCALL
libc_login(struct utmp const *entry) {
 libc_seterrno(ENOSYS);
}

EXPORT(logout,libc_logout);
CRT_TTY int LIBCCALL
libc_logout(char const *ut_line) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(logwtmp,libc_logwtmp);
CRT_TTY void LIBCCALL
libc_logwtmp(char const *ut_line,
             char const *ut_name,
             char const *ut_host) {
 libc_seterrno(ENOSYS);
}

EXPORT(updwtmp,libc_updwtmp);
CRT_TTY void LIBCCALL
libc_updwtmp(char const *wtmp_file,
             struct utmp const *utmp) {
 libc_seterrno(ENOSYS);
}

EXPORT(utmpname,libc_utmpname);
CRT_TTY int LIBCCALL
libc_utmpname(char const *file) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(getutent,libc_getutent);
CRT_TTY struct utmp *LIBCCALL libc_getutent(void) {
 libc_seterrno(ENOSYS);
 return NULL;
}

EXPORT(setutent,libc_setutent);
CRT_TTY void LIBCCALL libc_setutent(void) {
 libc_seterrno(ENOSYS);
}

EXPORT(endutent,libc_endutent);
CRT_TTY void LIBCCALL libc_endutent(void) {
 libc_seterrno(ENOSYS);
}

EXPORT(getutid,libc_getutid);
CRT_TTY struct utmp *LIBCCALL
libc_getutid(struct utmp const *id) {
 libc_seterrno(ENOSYS);
 return NULL;
}

EXPORT(getutline,libc_getutline);
CRT_TTY struct utmp *LIBCCALL
libc_getutline(struct utmp const *line) {
 libc_seterrno(ENOSYS);
 return NULL;
}

EXPORT(pututline,libc_pututline);
CRT_TTY struct utmp *LIBCCALL
libc_pututline(struct utmp const *utmp_ptr) {
 libc_seterrno(ENOSYS);
 return NULL;
}

EXPORT(getutent_r,libc_getutent_r);
CRT_TTY int LIBCCALL
libc_getutent_r(struct utmp *buffer,
                struct utmp **result) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(getutid_r,libc_getutid_r);
CRT_TTY int LIBCCALL
libc_getutid_r(struct utmp const *id,
               struct utmp *buffer,
               struct utmp **result) {
 libc_seterrno(ENOSYS);
 return -1;
}

EXPORT(getutline_r,libc_getutline_r);
CRT_TTY int LIBCCALL
libc_getutline_r(struct utmp const *line,
                 struct utmp *buffer,
                 struct utmp **result) {
 libc_seterrno(ENOSYS);
 return -1;
}




EXPORT(Xtcgetpgrp,libc_Xtcgetpgrp);
CRT_TTY_EXCEPT pid_t LIBCCALL libc_Xtcgetpgrp(fd_t fd) {
 pid_t pgrp;
 libc_Xioctl(fd,TIOCGPGRP,&pgrp);
 return pgrp;
}
EXPORT(Xtcsetpgrp,libc_Xtcsetpgrp);
CRT_TTY_EXCEPT void LIBCCALL
libc_Xtcsetpgrp(fd_t fd, pid_t pgrp_id) {
 libc_Xioctl(fd,TIOCSPGRP,&pgrp_id);
}
EXPORT(Xgetlogin,libc_Xgetlogin);
CRT_TTY_EXCEPT ATTR_RETNONNULL char *LIBCCALL libc_Xgetlogin(void) {
 error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(Xgetlogin_r,libc_Xgetlogin_r);
CRT_TTY_EXCEPT void LIBCCALL libc_Xgetlogin_r(char *name, size_t name_len) {
 error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(Xttyslot,libc_Xttyslot);
CRT_TTY_EXCEPT int LIBCCALL libc_Xttyslot(void) {
 error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(Xsetlogin,libc_Xsetlogin);
CRT_TTY_EXCEPT void LIBCCALL libc_Xsetlogin(char const *name) {
 error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(Xvhangup,libc_Xvhangup);
CRT_TTY_EXCEPT void LIBCCALL libc_Xvhangup(void) {
 error_throw(E_NOT_IMPLEMENTED);
}
EXPORT(Xgetpass,libc_Xgetpass);
CRT_TTY_EXCEPT char *LIBCCALL
libc_Xgetpass(char const *__restrict prompt) {
 error_throw(E_NOT_IMPLEMENTED);
}



EXPORT(Xopenpty,libc_Xopenpty);
CRT_TTY_EXCEPT void LIBCCALL
libc_Xopenpty(int *amaster, int *aslave, char *name,
              struct termios const *termp, struct winsize const *winp) {
 s64 result = Xsys_xopenpty(name,termp,winp);
 LIBC_TRY {
  *amaster = (int)result;
  *aslave  = (int)(result >> 32);
 } LIBC_EXCEPT(EXCEPT_EXECUTE_HANDLER) {
  sys_close((int)result);
  sys_close((int)(result >> 32));
  error_rethrow();
 }
}

EXPORT(Xforkpty,libc_Xforkpty);
CRT_TTY_EXCEPT pid_t LIBCCALL
libc_Xforkpty(int *amaster, char *name,
              struct termios const *termp,
              struct winsize const *winp) {
 int COMPILER_IGNORE_UNINITIALIZED(master);
 int COMPILER_IGNORE_UNINITIALIZED(slave);
 pid_t COMPILER_IGNORE_UNINITIALIZED(pid);
 libc_Xopenpty(&master,&slave,name,termp,winp);
 LIBC_TRY {
  if ((pid = libc_Xfork()) == 0) {
   /* Child process. */
   libc_close(master);
   if (libc_login_tty(slave))
       _libc_exit(1);
  }
  /* Parent process.  */
  *amaster = master;
  libc_close(slave);
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  sys_close(master);
  sys_close(slave);
  error_rethrow();
 }
 return pid;
}

DEFINE_INTERN_ALIAS(libc_Xcfgetospeed,libc_cfgetospeed);
DEFINE_INTERN_ALIAS(libc_Xcfgetispeed,libc_cfgetispeed);
DEFINE_INTERN_ALIAS(Xcfgetospeed,libc_Xcfgetospeed);
DEFINE_INTERN_ALIAS(Xcfgetispeed,libc_Xcfgetispeed);

EXPORT(Xcfsetospeed,libc_Xcfsetospeed);
EXPORT(Xcfsetispeed,libc_Xcfsetispeed);
EXPORT(Xcfsetspeed,libc_Xcfsetspeed);

#ifdef CONFIG_LIBCCALL_INTRETURN_IS_VOID
DEFINE_INTERN_ALIAS(libc_Xcfsetospeed,libc_cfsetospeed);
DEFINE_INTERN_ALIAS(libc_Xcfsetispeed,libc_cfsetispeed);
DEFINE_INTERN_ALIAS(libc_Xcfsetspeed,libc_cfsetspeed);
#else
CRT_TTY void LIBCCALL
libc_Xcfsetospeed(struct termios *termios_p, speed_t speed) {
 termios_p->c_ospeed = speed;
}
CRT_TTY void LIBCCALL
libc_Xcfsetispeed(struct termios *termios_p, speed_t speed) {
 termios_p->c_ispeed = speed;
}
CRT_TTY void LIBCCALL
libc_Xcfsetspeed(struct termios *termios_p, speed_t speed) {
 termios_p->c_ospeed = termios_p->c_ispeed = speed;
}
#endif

EXPORT(Xtcgetattr,libc_Xtcgetattr);
CRT_TTY_EXCEPT void LIBCCALL
libc_Xtcgetattr(fd_t fd, struct termios *termios_p) {
 libc_Xioctl(fd,TCGETS,termios_p);
}
EXPORT(Xtcsetattr,libc_Xtcsetattr);
CRT_TTY_EXCEPT void LIBCCALL
libc_Xtcsetattr(fd_t fd, int optional_actions,
                struct termios const *termios_p) {
 if (!TSC_VALIDACTION((unsigned int)optional_actions))
     libc_error_throw(E_INVALID_ARGUMENT);
 libc_Xioctl(fd,TSC_ACTION(optional_actions),termios_p);
}
EXPORT(Xtcsendbreak,libc_Xtcsendbreak);
CRT_TTY_EXCEPT void LIBCCALL
libc_Xtcsendbreak(fd_t fd, int duration) {
 if (duration <= 0) {
  libc_Xioctl(fd,TCSBRK,0);
 } else {
  libc_Xioctl(fd,TCSBRKP,(duration+99)/100);
 }
}
EXPORT(Xtcdrain,libc_Xtcdrain);
CRT_TTY_EXCEPT void LIBCCALL
libc_Xtcdrain(fd_t fd) {
 libc_Xioctl(fd,TCSBRK,1);
}
EXPORT(Xtcflush,libc_Xtcflush);
CRT_TTY_EXCEPT void LIBCCALL
libc_Xtcflush(fd_t fd, int queue_selector) {
 libc_Xioctl(fd,TCFLSH,queue_selector);
}
EXPORT(Xtcflow,libc_Xtcflow);
CRT_TTY_EXCEPT void LIBCCALL
libc_Xtcflow(fd_t fd, int action) {
 libc_Xioctl(fd,TCXONC,action);
}
EXPORT(Xtcgetsid,libc_Xtcgetsid);
CRT_TTY_EXCEPT pid_t LIBCCALL
libc_Xtcgetsid(fd_t fd) {
 pid_t sid;
 libc_Xioctl(fd,TIOCGSID,&sid);
 return sid;
}



DECL_END

#endif /* !GUARD_LIBS_LIBC_TTY_C */
