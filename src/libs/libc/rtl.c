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
#ifndef GUARD_LIBS_LIBC_RTL_C
#define GUARD_LIBS_LIBC_RTL_C 1
#define __EXPOSE_CPU_CONTEXT 1

#include "libc.h"
#include "rtl.h"
#include "unistd.h"
#include "format.h"
#include "errno.h"
#include "sched.h"
#include "exit.h"
#include "system.h"
#include "stdio.h"

#include <hybrid/compiler.h>
#include <kos/context.h>
#include <format-printer.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <except.h>
#include <string.h>
#include <syslog.h>
#include <syscall.h>
#include <linux/unistd.h>
#include <kos/thread.h>

/* Basic RTL support. */

DECL_BEGIN

DEFINE_INTERN_ALIAS(__stack_chk_fail,__stack_chk_fail_local);
INTERN ATTR_NORETURN void LIBCCALL
libc_core_assertion_failure(char const *expr, DEBUGINFO,
                            char const *format, va_list args) {
 libc_syslog(LOG_EMERG,"%s(%d) : %s : Assertion failed : %q\n",
             __file,__line,__func,expr);
 if (format) libc_vsyslog(LOG_EMERG,format,args),
             libc_syslog(LOG_EMERG,"\n");
 asm("int3");
 sys_exit_group(1);
}

INTERN uintptr_t __stack_chk_guard = 0x1246Ab1f;
INTERN ATTR_NORETURN void __stack_chk_fail_local(void) {
 libc_syslog(LOG_EMERG,"__stack_chk_fail_local()\n");
 asm("int3");
 for (;;) {}
}


INTERN ATTR_NORETURN ATTR_COLD
void (libc_afailf)(char const *expr, DEBUGINFO, char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_core_assertion_failure(expr,DEBUGINFO_FWD,format,args);
}

INTERN ATTR_NORETURN ATTR_COLD
void (__LIBCCALL libc_afail)(char const *expr, DEBUGINFO) {
 va_list args;
 libc_memset(&args,0,sizeof(va_list));
 libc_core_assertion_failure(expr,DEBUGINFO_FWD,NULL,args);
}
INTERN ssize_t LIBCCALL
libc_syslog_printer(char const *__restrict data,
                    size_t datalen, void *closure) {
 if (!datalen) return 0;
 return sys_xsyslog((int)(uintptr_t)closure,data,datalen);
}
INTERN void LIBCCALL
libc_vsyslog(int level, char const *format, va_list args) {
 libc_format_vprintf(&libc_syslog_printer,SYSLOG_PRINTER_CLOSURE(level),
                     format,args);
}
INTERN void ATTR_CDECL
libc_syslog(int level, char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_vsyslog(level,format,args);
 va_end(args);
}

EXPORT(__afailf,libc_afailf);
EXPORT(__afail,libc_afail);
EXPORT(syslog_printer,libc_syslog_printer);
EXPORT(syslog,libc_syslog);
EXPORT(vsyslog,libc_vsyslog);




INTERN ATTR_NORETURN void FCALL
libc_error_rethrow_at(struct cpu_context *__restrict context) {
 assertf(context != &error_info()->e_context,
         "Remeber how this function is allowed to modify the context? "
         "Wouldn't make much sense if you passed the context that's supposed "
         "to represent what was going on when the exception was thrown...");
 /* Unwind the stack to the nearest handler.
  * If that fails, invoke the unhandled-exception handler. */
 /* TODO: FPU context */
 if (sys_xunwind_except(libc_error_info(),context,NULL) != -EOK)
     libc_error_unhandled_exception();
 /* Jump to the new exception context. */
 libc_cpu_setcontext(context);
}

INTERN void ATTR_CDECL
libc_error_fprintf(FILE *fp, char const *reason, ...) {
 va_list args;
 va_start(args,reason);
 libc_error_vfprintf(fp,reason,args);
 va_end(args);
}
INTERN void LIBCCALL
libc_error_vprintf(char const *reason, va_list args) {
 libc_error_vfprintf(NULL,reason,args);
}
INTERN void ATTR_CDECL
libc_error_printf(char const *reason, ...) {
 va_list args;
 va_start(args,reason);
 libc_error_vprintf(reason,args);
 va_end(args);
}


INTERN ATTR_NORETURN void FCALL
libc_error_unhandled_exception(void) {
 struct task_segment *thread = libc_current();
 /* Deal with special errors that should not be
  * caught by the unhandled exception handler. */
 if (thread->ts_xcurrent.e_error.e_code == E_EXIT_THREAD)
     sys_exit(thread->ts_xcurrent.e_error.e_exit.e_status); /* Exit the thread. */
 if (thread->ts_xcurrent.e_error.e_code == E_EXIT_PROCESS)
     libc_exit(thread->ts_xcurrent.e_error.e_exit.e_status); /* Exit the process. */
 if (thread->ts_ueh &&
   !(ATOMIC_FETCHOR(thread->ts_state,THREAD_STATE_FINUEH) & THREAD_STATE_FINUEH)) {
#if defined(__i386__) || defined(__x86_64__)
  __asm__ __volatile__("    cmp $0, %0\n"
                       "    je  1f\n"
                       "    mov %0, %%esp\n"
                       "1:  jmp *%1"
                       :
                       : "g" (thread->ts_ueh_sp)
                       , "g" (thread->ts_ueh)
                       : "memory");
  __builtin_unreachable();
#else
#error "Unsupported architecture"
#endif
 }
 
 /* Dump exception information */
 libc_error_printf(NULL);

 /* Flush stdio buffers so that everything written
  * by `libc_error_printf()' gets flushed. */
#ifdef CONFIG_LIBC_USES_NEW_STDIO
 FileBuffer_FlushAllBuffers();
#endif /* CONFIG_LIBC_USES_NEW_STDIO */

 if (thread->ts_state & THREAD_STATE_FALONE)
     sys_exit(1); /* Only exit the thread. */

 /* TODO: Create a coredump */
 _libc_exit(1);
}

INTERN ATTR_NORETURN void FCALL
libc_os_error_unhandled_exception(void) {
 /* NOTE: We don't call the `libc_error_unhandled_exception()' here,
  *       thus allowing our partnered application to simply override
  *       this function and have theirs be called instead. */
 error_unhandled_exception();
}

/* Secret entry point for when the kernel detects
 * an exception with no apparent handlers. */
DEFINE_PUBLIC_WEAK_ALIAS("__$$OS$error_unhandled_exception",
                         libc_os_error_unhandled_exception);


EXPORT(__error_rethrow_at,       libc_error_rethrow_at);
EXPORT(error_unhandled_exception,libc_error_unhandled_exception);
EXPORT(error_printf,             libc_error_printf);
EXPORT(error_vprintf,            libc_error_vprintf);
EXPORT(error_fprintf,            libc_error_fprintf);



EXPORT(except_errno,libc_except_errno);
INTERN int FCALL libc_except_errno(void) {
 libc_seterrno(libc_exception_errno(libc_error_info()));
 return EXCEPT_EXECUTE_HANDLER;
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_RTL_C */
