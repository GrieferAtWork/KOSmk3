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
#ifndef GUARD_LIBS_LIBC_TTY_H
#define GUARD_LIBS_LIBC_TTY_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN

/* ===================================================================================== */
/*     TTY                                                                               */
/* ===================================================================================== */
typedef unsigned int speed_t;
struct termios;
struct winsize;
struct utmp;
INTDEF char *LIBCCALL libc_ttyname(fd_t fd);
INTDEF int LIBCCALL libc_ttyname_r(fd_t fd, char *buf, size_t buflen);
INTDEF pid_t LIBCCALL libc_tcgetpgrp(fd_t fd);
INTDEF int LIBCCALL libc_tcsetpgrp(fd_t fd, pid_t pgrp_id);
INTDEF char *LIBCCALL libc_getlogin(void);
INTDEF int LIBCCALL libc_isatty(fd_t fd);
INTDEF int LIBCCALL libc_ttyslot(void);
INTDEF int LIBCCALL libc_getlogin_r(char *name, size_t name_len);
INTDEF int LIBCCALL libc_setlogin(char const *name);
INTDEF int LIBCCALL libc_vhangup(void);
INTDEF char *LIBCCALL libc_getpass(char const *__restrict prompt);
INTDEF int LIBCCALL libc_openpty(int *amaster, int *aslave, char *name, struct termios const *termp, struct winsize const *winp);
INTDEF pid_t LIBCCALL libc_forkpty(int *amaster, char *name, struct termios const *termp, struct winsize const *winp);
INTDEF speed_t LIBCCALL libc_cfgetospeed(struct termios const *termios_p);
INTDEF speed_t LIBCCALL libc_cfgetispeed(struct termios const *termios_p);
INTDEF int LIBCCALL libc_cfsetospeed(struct termios *termios_p, speed_t speed);
INTDEF int LIBCCALL libc_cfsetispeed(struct termios *termios_p, speed_t speed);
INTDEF int LIBCCALL libc_cfsetspeed(struct termios *termios_p, speed_t speed);
INTDEF int LIBCCALL libc_tcgetattr(fd_t fd, struct termios *termios_p);
INTDEF int LIBCCALL libc_tcsetattr(fd_t fd, int optional_actions, struct termios const *termios_p);
INTDEF int LIBCCALL libc_tcsendbreak(fd_t fd, int duration);
INTDEF int LIBCCALL libc_tcdrain(fd_t fd);
INTDEF int LIBCCALL libc_tcflush(fd_t fd, int queue_selector);
INTDEF int LIBCCALL libc_tcflow(fd_t fd, int action);
INTDEF pid_t LIBCCALL libc_tcgetsid(fd_t fd);
INTDEF void LIBCCALL libc_cfmakeraw(struct termios *termios_p);
INTDEF int LIBCCALL libc_login_tty(fd_t fd);
INTDEF void LIBCCALL libc_login(struct utmp const *entry);
INTDEF int LIBCCALL libc_logout(char const *ut_line);
INTDEF void LIBCCALL libc_logwtmp(char const *ut_line, char const *ut_name, char const *ut_host);
INTDEF void LIBCCALL libc_updwtmp(char const *wtmp_file, struct utmp const *utmp);
INTDEF int LIBCCALL libc_utmpname(char const *file);
INTDEF struct utmp *LIBCCALL libc_getutent(void);
INTDEF void LIBCCALL libc_setutent(void);
INTDEF void LIBCCALL libc_endutent(void);
INTDEF struct utmp *LIBCCALL libc_getutid(struct utmp const *id);
INTDEF struct utmp *LIBCCALL libc_getutline(struct utmp const *line);
INTDEF struct utmp *LIBCCALL libc_pututline(struct utmp const *utmp_ptr);
INTDEF int LIBCCALL libc_getutent_r(struct utmp *buffer, struct utmp **result);
INTDEF int LIBCCALL libc_getutid_r(struct utmp const *id, struct utmp *buffer, struct utmp **result);
INTDEF int LIBCCALL libc_getutline_r(struct utmp const *line, struct utmp *buffer, struct utmp **result);
INTDEF pid_t LIBCCALL libc_Xtcgetpgrp(fd_t fd);
INTDEF void LIBCCALL libc_Xtcsetpgrp(fd_t fd, pid_t pgrp_id);
INTDEF char *LIBCCALL libc_Xgetlogin(void);
INTDEF int LIBCCALL libc_Xttyslot(void);
INTDEF void LIBCCALL libc_Xgetlogin_r(char *name, size_t name_len);
INTDEF void LIBCCALL libc_Xsetlogin(char const *name);
INTDEF void LIBCCALL libc_Xvhangup(void);
INTDEF char *LIBCCALL libc_Xgetpass(char const *__restrict prompt);
INTDEF void LIBCCALL libc_Xopenpty(int *amaster, int *aslave, char *name, struct termios const *termp, struct winsize const *winp);
INTDEF pid_t LIBCCALL libc_Xforkpty(int *amaster, char *name, struct termios const *termp, struct winsize const *winp);
INTDEF speed_t LIBCCALL libc_Xcfgetospeed(struct termios const *termios_p);
INTDEF speed_t LIBCCALL libc_Xcfgetispeed(struct termios const *termios_p);
INTDEF void LIBCCALL libc_Xcfsetospeed(struct termios *termios_p, speed_t speed);
INTDEF void LIBCCALL libc_Xcfsetispeed(struct termios *termios_p, speed_t speed);
INTDEF void LIBCCALL libc_Xcfsetspeed(struct termios *termios_p, speed_t speed);
INTDEF void LIBCCALL libc_Xtcgetattr(fd_t fd, struct termios *termios_p);
INTDEF void LIBCCALL libc_Xtcsetattr(fd_t fd, int optional_actions, struct termios const *termios_p);
INTDEF void LIBCCALL libc_Xtcsendbreak(fd_t fd, int duration);
INTDEF void LIBCCALL libc_Xtcdrain(fd_t fd);
INTDEF void LIBCCALL libc_Xtcflush(fd_t fd, int queue_selector);
INTDEF void LIBCCALL libc_Xtcflow(fd_t fd, int action);
INTDEF pid_t LIBCCALL libc_Xtcgetsid(fd_t fd);

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_TTY_H */
