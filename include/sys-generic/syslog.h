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
#ifndef _SYS_GENERIC_SYSLOG_H
#define _SYS_GENERIC_SYSLOG_H 1

#include <features.h>
#include <bits/syslog-path.h>
#ifdef __USE_KOS
#include <kos/types.h>
#endif /* __USE_KOS */

/*
 * Copyright (c) 1982, 1986, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)syslog.h	8.1 (Berkeley) 6/2/93
 */

#if !defined(__CRT_GLC) && !defined(__CRT_CYG)
#error "<sys/syslog.h> is not supported by the linked libc"
#endif /* !__CRT_GLC && !__CRT_CYG */

__SYSDECL_BEGIN

#define LOG_EMERG   0    /* system is unusable */
#define LOG_ALERT   1    /* action must be taken immediately */
#define LOG_CRIT    2    /* critical conditions */
#define LOG_ERR     3    /* error conditions */
#define LOG_WARNING 4    /* warning conditions */
#define LOG_NOTICE  5    /* normal but significant condition */
#define LOG_INFO    6    /* informational */
#define LOG_DEBUG   7    /* debug-level messages */
#define LOG_PRIMASK 0x07 /* mask to extract priority part (internal) */
/* extract priority */
#define LOG_PRI(p)           ((p)&LOG_PRIMASK)
#define LOG_MAKEPRI(fac,pri) ((fac)|(pri))

#if defined(__USE_KOS) || defined(__KERNEL__)
/* Aliases used throughout the kernel. */
#define LOG_ERROR   LOG_ERR
#define LOG_WARN    LOG_WARNING
#define LOG_CONFIRM LOG_NOTICE
#define LOG_MESSAGE LOG_INFO
#endif

#ifdef SYSLOG_NAMES
#define INTERNAL_NOPRI 0x10 /* the "no priority" priority */
#define INTERNAL_MARK  LOG_MAKEPRI(LOG_NFACILITIES << 3, 0) /* mark "facility" */
typedef struct _code {
    char *c_name;
    int   c_val;
} CODE;
CODE prioritynames[] = {
    { "alert", LOG_ALERT },
    { "crit", LOG_CRIT },
    { "debug", LOG_DEBUG },
    { "emerg", LOG_EMERG },
    { "err", LOG_ERR },
    { "error", LOG_ERR },        /* DEPRECATED */
    { "info", LOG_INFO },
    { "none", INTERNAL_NOPRI },        /* INTERNAL */
    { "notice", LOG_NOTICE },
    { "panic", LOG_EMERG },        /* DEPRECATED */
    { "warn", LOG_WARNING },        /* DEPRECATED */
    { "warning", LOG_WARNING },
    { NULL, -1 }
};
#endif /* SYSLOG_NAMES */

/* facility codes */
#define LOG_KERN     (0<<3)  /* kernel messages. */
#define LOG_USER     (1<<3)  /* random user-level messages. */
#define LOG_MAIL     (2<<3)  /* mail system. */
#define LOG_DAEMON   (3<<3)  /* system daemons. */
#define LOG_AUTH     (4<<3)  /* security/authorization messages. */
#define LOG_SYSLOG   (5<<3)  /* messages generated internally by syslogd. */
#define LOG_LPR      (6<<3)  /* line printer subsystem. */
#define LOG_NEWS     (7<<3)  /* network news subsystem. */
#define LOG_UUCP     (8<<3)  /* UUCP subsystem. */
#define LOG_CRON     (9<<3)  /* clock daemon. */
#define LOG_AUTHPRIV (10<<3) /* security/authorization messages (private). */
#define LOG_FTP      (11<<3) /* ftp daemon. */

/* other codes through 15 reserved for system use */
#define LOG_LOCAL0      (16<<3) /* reserved for local use */
#define LOG_LOCAL1      (17<<3) /* reserved for local use */
#define LOG_LOCAL2      (18<<3) /* reserved for local use */
#define LOG_LOCAL3      (19<<3) /* reserved for local use */
#define LOG_LOCAL4      (20<<3) /* reserved for local use */
#define LOG_LOCAL5      (21<<3) /* reserved for local use */
#define LOG_LOCAL6      (22<<3) /* reserved for local use */
#define LOG_LOCAL7      (23<<3) /* reserved for local use */
#ifdef __KERNEL__
/* More Log facilities only available within the kernel. */
#define LOG_FS          (24<<3)
#define LOG_MEM         (25<<3)
#define LOG_HW          (26<<3)
#define LOG_IRQ         (27<<3)
#define LOG_EXEC        (28<<3)
#define LOG_SCHED       (29<<3)
#define LOG_BOOT        (30<<3)
#define LOG_IO          (31<<3)
#define LOG_NFACILITIES  32     /* current number of facilities */
#else
#define LOG_NFACILITIES  24     /* current number of facilities */
#endif
#define LOG_FACMASK      0x03f8 /* mask to extract facility part */
#define LOG_FAC(p)   (((p) & LOG_FACMASK) >> 3) /* facility of pri */

#ifdef SYSLOG_NAMES
CODE facilitynames[] = {
    { "auth", LOG_AUTH },
    { "authpriv", LOG_AUTHPRIV },
    { "cron", LOG_CRON },
    { "daemon", LOG_DAEMON },
    { "ftp", LOG_FTP },
    { "kern", LOG_KERN },
    { "lpr", LOG_LPR },
    { "mail", LOG_MAIL },
    { "mark", INTERNAL_MARK },        /* INTERNAL */
    { "news", LOG_NEWS },
    { "security", LOG_AUTH },        /* DEPRECATED */
    { "syslog", LOG_SYSLOG },
    { "user", LOG_USER },
    { "uucp", LOG_UUCP },
    { "local0", LOG_LOCAL0 },
    { "local1", LOG_LOCAL1 },
    { "local2", LOG_LOCAL2 },
    { "local3", LOG_LOCAL3 },
    { "local4", LOG_LOCAL4 },
    { "local5", LOG_LOCAL5 },
    { "local6", LOG_LOCAL6 },
    { "local7", LOG_LOCAL7 },
#ifdef __KERNEL__
    { "fs", LOG_FS },
    { "mem", LOG_MEM },
    { "hw", LOG_HW },
    { "irq", LOG_IRQ },
    { "exec", LOG_EXEC },
    { "sched", LOG_SCHED },
    { "boot", LOG_BOOT },
    { "io", LOG_IO },
#endif /* __KERNEL__ */
    { NULL, -1 }
};
#endif /* SYSLOG_NAMES */

/* arguments to setlogmask. */
#define LOG_MASK(pri)  (1 <<  (pri)) /* mask for one priority. */
#define LOG_UPTO(pri) ((1 << ((pri)+1))-1) /* all priorities through pri. */

#define LOG_PID    0x01 /* log the pid with each message. */
#define LOG_CONS   0x02 /* log on the console if errors in sending. */
#define LOG_ODELAY 0x04 /* delay open until first syslog() (default). */
#define LOG_NDELAY 0x08 /* don't delay open. */
#define LOG_NOWAIT 0x10 /* don't wait for console forks: DEPRECATED. */
#define LOG_PERROR 0x20 /* log to stderr as well. */

#ifndef __KERNEL__
__LIBC void (__LIBCCALL closelog)(void);
__LIBC void (__LIBCCALL openlog)(char const *__ident, int __option, int __facility);
__LIBC int (__LIBCCALL setlogmask)(int __mask);
#endif /* !__KERNEL__ */

__LIBC __ATTR_LIBC_PRINTF(2,3) void (__ATTR_CDECL syslog)(int __level, char const *__format, ...);
#ifdef __USE_MISC
__LIBC __ATTR_LIBC_PRINTF(2,0) void (__LIBCCALL vsyslog)(int __level, char const *__format, __builtin_va_list __args);
#endif /* __USE_MISC */

#ifdef __USE_KOS
/* Helper functions for printing to the system log. */
__LIBC __ssize_t (__LIBCCALL syslog_printer)(char const *__restrict __data,
                                             __size_t __datalen, void *__closure);
#define SYSLOG_PRINTER_CLOSURE(level) ((void *)(uintptr_t)(int)(level))
#endif /* __USE_KOS */

__SYSDECL_END

#endif /* !_SYS_GENERIC_SYSLOG_H */
