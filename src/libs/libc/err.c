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
#ifndef GUARD_LIBS_LIBC_ERR_C
#define GUARD_LIBS_LIBC_ERR_C 1

#include "libc.h"
#include "err.h"
#include "rtl.h"
#include "errno.h"
#include "sched.h"
#include "format.h"
#include "ushare.h"
#include "exit.h"
#include "stdio/file.h"
#include "widechar.h"
#include <kos/environ.h>
#include <kos/types.h>
#include <stdarg.h>
#include <syslog.h>

DECL_BEGIN

#define ERROR_EXIT(code) _libc_exit(code)


#define warn_suffix_format error_suffix_format
CRT_RARE_RODATA char const warn_prefix_format[]  = "%s: ";
CRT_RARE_RODATA char const error_suffix_format[] = ": %[errno]\n";
CRT_RARE_RODATA char const error_prefix_format[] = "%s";
CRT_RARE_RODATA char const error_collon[]        = ": ";
CRT_RARE_RODATA char const error_format[]        = ":%s:%d: ";
CRT_RARE_RODATA char const perror_format[]       = "%s: %[errno]\n";


CRT_RARE ATTR_NORETURN void LIBCCALL
libc_assert_perror_fail(int errnum, char const *file,
                        unsigned int line, char const *function);
CRT_RARE void LIBCCALL
libc_perror(char const *__restrict message);


EXPORT(__libc_program_invocation_name,libc_program_invocation_name);
CRT_RARE char *LIBCCALL libc_program_invocation_name(void) {
 struct process_environ *proc;
 proc = libc_procenv();
 return proc && proc->pe_argv ? proc->pe_argv[0] : NULL;
}

EXPORT(__libc_program_invocation_short_name,libc_program_invocation_short_name);
CRT_RARE char *LIBCCALL libc_program_invocation_short_name(void) {
 return libc_basename(libc_program_invocation_name());
}

#if 1
#define PRINTF(format,...)   libc_syslog(LOG_ERROR,format,##__VA_ARGS__)
#define VPRINTF(format,args) libc_vsyslog(LOG_ERROR,format,args)
#else
#define PRINTF(format,...)   libc_fprintf(stderr,format,##__VA_ARGS__)
#define VPRINTF(format,args) libc_vfprintf(stderr,format,args)
#endif


EXPORT(__KSYM(vwarn),libc_vwarn);
CRT_RARE void LIBCCALL libc_vwarn(char const *format, va_list args) {
 PRINTF(warn_prefix_format,libc_program_invocation_short_name());
 VPRINTF(format,args);
 PRINTF(warn_suffix_format,libc_geterrno());
}
EXPORT(__KSYM(vwarnx),libc_vwarnx);
CRT_RARE void LIBCCALL libc_vwarnx(char const *format, va_list args) {
 PRINTF(warn_prefix_format,libc_program_invocation_short_name());
 VPRINTF(format,args);
}
EXPORT(__KSYM(verr),libc_verr);
CRT_RARE ATTR_NORETURN void LIBCCALL
libc_verr(int status, char const *format, va_list args) {
 libc_vwarn(format,args);
 ERROR_EXIT(status);
}
EXPORT(__KSYM(verrx),libc_verrx);
CRT_RARE ATTR_NORETURN void LIBCCALL
libc_verrx(int status, char const *format, va_list args) {
 libc_vwarnx(format,args);
 ERROR_EXIT(status);
}

EXPORT(__KSYM(warn),libc_warn);
CRT_RARE void LIBCCALL
libc_warn(char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_vwarn(format,args);
 va_end(args);
}
EXPORT(__KSYM(warnx),libc_warnx);
CRT_RARE void LIBCCALL
libc_warnx(char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_vwarnx(format,args);
 va_end(args);
}
EXPORT(__KSYM(err),libc_err);
CRT_RARE ATTR_NORETURN void LIBCCALL
libc_err(int status, char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_verr(status,format,args);
}
EXPORT(__KSYM(errx),libc_errx);
CRT_RARE ATTR_NORETURN void LIBCCALL
libc_errx(int status, char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_verrx(status,format,args);
}

EXPORT(__DSYM(vwarn),libd_vwarn);
CRT_RARE void LIBCCALL libd_vwarn(char const *format, va_list args) {
 PRINTF(warn_prefix_format,libc_program_invocation_short_name());
 libd_vfprintf(stderr,format,args);
 PRINTF(warn_suffix_format,libc_geterrno());
}
EXPORT(__DSYM(vwarnx),libd_vwarnx);
CRT_RARE void LIBCCALL libd_vwarnx(char const *format, va_list args) {
 PRINTF(warn_prefix_format,libc_program_invocation_short_name());
 libd_vfprintf(stderr,format,args);
}
EXPORT(__DSYM(verr),libd_verr);
CRT_RARE ATTR_NORETURN void LIBCCALL
libd_verr(int status, char const *format, va_list args) {
 libd_vwarn(format,args);
 ERROR_EXIT(status);
}
EXPORT(__DSYM(verrx),libd_verrx);
CRT_RARE ATTR_NORETURN void LIBCCALL
libd_verrx(int status, char const *format, va_list args) {
 libd_vwarnx(format,args);
 ERROR_EXIT(status);
}

EXPORT(__DSYM(warn),libd_warn);
CRT_RARE void LIBCCALL
libd_warn(char const *format, ...) {
 va_list args;
 va_start(args,format);
 libd_vwarn(format,args);
 va_end(args);
}
EXPORT(__DSYM(warnx),libd_warnx);
CRT_RARE void LIBCCALL
libd_warnx(char const *format, ...) {
 va_list args;
 va_start(args,format);
 libd_vwarnx(format,args);
 va_end(args);
}
EXPORT(__DSYM(err),libd_err);
CRT_RARE ATTR_NORETURN void LIBCCALL
libd_err(int status, char const *format, ...) {
 va_list args;
 va_start(args,format);
 libd_verr(status,format,args);
}
EXPORT(__DSYM(errx),libd_errx);
CRT_RARE ATTR_NORETURN void LIBCCALL
libd_errx(int status, char const *format, ...) {
 va_list args;
 va_start(args,format);
 libd_verrx(status,format,args);
}


PUBLIC ATTR_SECTION(".bss.crt.rare") unsigned int error_message_count = 0;
PUBLIC ATTR_SECTION(".bss.crt.rare") int error_one_per_line = 0;
PUBLIC ATTR_SECTION(".bss.crt.rare") void (*error_print_progname)(void) = NULL;

CRT_RARE void LIBCCALL error_prefix(void) {
 void (*print_name)(void) = error_print_progname;
 libc_fflush(stdout);
 if (print_name) {
  (*print_name)();
 } else {
  PRINTF(error_prefix_format,
               libc_program_invocation_short_name());
 }
}

CRT_RARE void LIBCCALL
error_suffix(int status, int errnum) {
 PRINTF(error_suffix_format,errnum);
 ++error_message_count;
 if (status)
     ERROR_EXIT(status);
}

EXPORT(__KSYM(error),libc_error);
CRT_RARE void LIBCCALL
libc_error(int status, errno_t errnum, char const *format, ...) {
 va_list args;
 error_prefix();
 libc_fwrite(error_collon,sizeof(char),COMPILER_LENOF(error_collon),stderr);
 va_start(args,format);
 VPRINTF(format,args);
 va_end(args);
 error_suffix(status,errnum);
}

EXPORT(__DSYM(error),libd_error);
CRT_RARE void LIBCCALL
libd_error(int status, errno_t errnum, char const *format, ...) {
 va_list args;
 error_prefix();
 libc_fwrite(error_collon,sizeof(char),COMPILER_LENOF(error_collon),stderr);
 va_start(args,format);
 libd_vfprintf(stderr,format,args);
 va_end(args);
 error_suffix(status,errnum);
}

EXPORT(__KSYM(error_at_line),libc_error_at_line);
CRT_RARE void LIBCCALL
libc_error_at_line(int status, errno_t errnum, char const *fname,
                   unsigned int lineno, char const *format, ...) {
 va_list args;
 error_prefix();
 PRINTF(error_format,fname,lineno);
 va_start(args,format);
 VPRINTF(format,args);
 va_end(args);
 error_suffix(status,errnum);
}
EXPORT(__DSYM(error_at_line),libd_error_at_line);
CRT_DOS_RARE void LIBCCALL
libd_error_at_line(int status, errno_t errnum, char const *fname,
                   unsigned int lineno, char const *format, ...) {
 va_list args;
 error_prefix();
 PRINTF(error_format,fname,lineno);
 va_start(args,format);
 libd_vfprintf(stderr,format,args);
 va_end(args);
 error_suffix(status,errnum);
}

EXPORT(perror,libc_perror);
CRT_RARE void LIBCCALL
libc_perror(char const *__restrict message) {
 PRINTF(perror_format,message,libc_geterrno());
}

EXPORT(__SYMw16(_wperror),libc_w16perror);
INTERN void LIBCCALL
libc_w16perror(char16_t const *__restrict message) {
 char buf[UTF_STACK_BUFFER_SIZE];
 int saved = libc_geterrno();
 char *str = libc_loadutf16(buf,message);
 if (!str) str = buf,buf[0] = 0;
 libc_seterrno(saved);
 libc_perror(str);
 libc_freeutf(buf,str);
}

EXPORT(__SYMw32(wperror),libc_w32perror);
INTERN void LIBCCALL
libc_w32perror(char32_t const *__restrict message) {
 char buf[UTF_STACK_BUFFER_SIZE];
 int saved = libc_geterrno();
 char *str = libc_loadutf32(buf,message);
 if (!str) str = buf,buf[0] = 0;
 libc_seterrno(saved);
 libc_perror(str);
 libc_freeutf(buf,str);
}


EXPORT_STRONG(__fortify_fail,libc_fortify_fail);
INTERN void LIBCCALL
libc_fortify_fail(char const *__restrict message) {
 /* XXX ??? */
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_ERR_C */
