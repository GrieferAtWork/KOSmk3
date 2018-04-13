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
#ifndef GUARD_KERNEL_SRC_KERNEL_SYSCONF_C
#define GUARD_KERNEL_SRC_KERNEL_SYSCONF_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <hybrid/limits.h>
#include <hybrid/section.h>
#include <linux/limits.h>
#include <kernel/syscall.h>
#include <kernel/sysconf.h>
#include <bits/confname.h>
#include <bits/time.h>
#include <bits/posix_opt.h>
#include <limits.h>
#include <except.h>
#include <errno.h>
#include <fs/handle.h>

DECL_BEGIN

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
PRIVATE ATTR_RARERODATA
long int const default_sysconf[_SC_COUNT] = {
    [0 ... _SC_COUNT-1] = -EINVAL,

    [_SC_ARG_MAX]                      = LONG_MAX, /* XXX: Add limit */
    [_SC_CHILD_MAX]                    = LONG_MAX, /* XXX: Add limit */
    [_SC_CLK_TCK]                      = 1000000,
    [_SC_NGROUPS_MAX]                  = LONG_MAX, /* XXX: Add limit */
    [_SC_TZNAME_MAX]                   = _POSIX_TZNAME_MAX,
    [_SC_JOB_CONTROL]                  = 1,        /* KOS implements process groups and associated terminal functions. */
    [_SC_SAVED_IDS]                    = 1,        /* TODO: setuid(), setgid(), seteuid(), setegid() */
    [_SC_REALTIME_SIGNALS]             = _POSIX_REALTIME_SIGNALS, /* ??? (I think? I've got everything linux's got...) */
    [_SC_PRIORITY_SCHEDULING]          = 0,        /* ??? */
    [_SC_TIMERS]                       = 0,        /* ??? */
    [_SC_ASYNCHRONOUS_IO]              = 0,        /* ??? */
    [_SC_PRIORITIZED_IO]               = 0,        /* ??? */
    [_SC_SYNCHRONIZED_IO]              = 0,        /* ??? */
    [_SC_FSYNC]                        = _POSIX_FSYNC,
    [_SC_MAPPED_FILES]                 = _POSIX_MAPPED_FILES,
    [_SC_MEMLOCK]                      = _POSIX_MEMLOCK,
    [_SC_MEMLOCK_RANGE]                = _POSIX_MEMLOCK_RANGE,
    [_SC_MEMORY_PROTECTION]            = _POSIX_MEMORY_PROTECTION,
    [_SC_MESSAGE_PASSING]              = 0,
    [_SC_SEMAPHORES]                   = 0,
    [_SC_SHARED_MEMORY_OBJECTS]        = 0,
    [_SC_AIO_LISTIO_MAX]               = 0,
    [_SC_AIO_MAX]                      = 0,
    [_SC_AIO_PRIO_DELTA_MAX]           = 0,
    [_SC_DELAYTIMER_MAX]               = 0,
    [_SC_MQ_OPEN_MAX]                  = 0,
    [_SC_MQ_PRIO_MAX]                  = 0,
    [_SC_PAGESIZE]                     = PAGESIZE,
    [_SC_RTSIG_MAX]                    = 0,
    [_SC_SEM_NSEMS_MAX]                = 0,
    [_SC_SEM_VALUE_MAX]                = 0,
    [_SC_SIGQUEUE_MAX]                 = 0,
    [_SC_TIMER_MAX]                    = 0,
    [_SC_BC_BASE_MAX]                  = 0,
    [_SC_BC_DIM_MAX]                   = 0,
    [_SC_BC_SCALE_MAX]                 = 0,
    [_SC_BC_STRING_MAX]                = 0,
    [_SC_COLL_WEIGHTS_MAX]             = 0,
    [_SC_EQUIV_CLASS_MAX]              = 0,
    [_SC_EXPR_NEST_MAX]                = 0,
    [_SC_LINE_MAX]                     = 0,
    [_SC_RE_DUP_MAX]                   = 0,
    [_SC_CHARCLASS_NAME_MAX]           = 0,
    [_SC_2_VERSION]                    = 0,
    [_SC_2_C_BIND]                     = 0,
    [_SC_2_C_DEV]                      = 0,
    [_SC_2_FORT_DEV]                   = 0,
    [_SC_2_FORT_RUN]                   = 0,
    [_SC_2_SW_DEV]                     = 0,
    [_SC_2_LOCALEDEF]                  = 0,
    [_SC_PII]                          = 0,
    [_SC_PII_XTI]                      = 0,
    [_SC_PII_SOCKET]                   = 0,
    [_SC_PII_INTERNET]                 = 0,
    [_SC_PII_OSI]                      = 0,
    [_SC_POLL]                         = 0,
    [_SC_SELECT]                       = 0,
    [_SC_UIO_MAXIOV]                   = 0,
    [_SC_PII_INTERNET_STREAM]          = 0,
    [_SC_PII_INTERNET_DGRAM]           = 0,
    [_SC_PII_OSI_COTS]                 = 0,
    [_SC_PII_OSI_CLTS]                 = 0,
    [_SC_PII_OSI_M]                    = 0,
    [_SC_T_IOV_MAX]                    = 0,
    [_SC_THREADS]                      = 0,
    [_SC_THREAD_SAFE_FUNCTIONS]        = 0,
    [_SC_GETGR_R_SIZE_MAX]             = 0,
    [_SC_GETPW_R_SIZE_MAX]             = 0,
    [_SC_LOGIN_NAME_MAX]               = 0,
    [_SC_TTY_NAME_MAX]                 = 0,
    [_SC_THREAD_DESTRUCTOR_ITERATIONS] = 0,
    [_SC_THREAD_KEYS_MAX]              = 0,
    [_SC_THREAD_STACK_MIN]             = 0,
    [_SC_THREAD_THREADS_MAX]           = 0,
    [_SC_THREAD_ATTR_STACKADDR]        = 0,
    [_SC_THREAD_ATTR_STACKSIZE]        = 0,
    [_SC_THREAD_PRIORITY_SCHEDULING]   = 0,
    [_SC_THREAD_PRIO_INHERIT]          = 0,
    [_SC_THREAD_PRIO_PROTECT]          = 0,
    [_SC_THREAD_PROCESS_SHARED]        = 0,
    [_SC_NPROCESSORS_CONF]             = 0,
    [_SC_NPROCESSORS_ONLN]             = 0,
    [_SC_PHYS_PAGES]                   = 0,
    [_SC_AVPHYS_PAGES]                 = 0,
    [_SC_ATEXIT_MAX]                   = 0,
    [_SC_PASS_MAX]                     = 0,
    [_SC_XOPEN_VERSION]                = 0,
    [_SC_XOPEN_XCU_VERSION]            = 0,
    [_SC_XOPEN_UNIX]                   = 0,
    [_SC_XOPEN_CRYPT]                  = 0,
    [_SC_XOPEN_ENH_I18N]               = 0,
    [_SC_XOPEN_SHM]                    = 0,
    [_SC_2_CHAR_TERM]                  = 0,
    [_SC_2_C_VERSION]                  = 0,
    [_SC_2_UPE]                        = 0,
    [_SC_XOPEN_XPG2]                   = 0,
    [_SC_XOPEN_XPG3]                   = 0,
    [_SC_XOPEN_XPG4]                   = 0,
    [_SC_NL_ARGMAX]                    = 0,
    [_SC_NL_LANGMAX]                   = 0,
    [_SC_NL_MSGMAX]                    = 0,
    [_SC_NL_NMAX]                      = 0,
    [_SC_NL_SETMAX]                    = 0,
    [_SC_NL_TEXTMAX]                   = 0,
    [_SC_XBS5_ILP32_OFF32]             = 0,
    [_SC_XBS5_ILP32_OFFBIG]            = 0,
    [_SC_XBS5_LP64_OFF64]              = 0,
    [_SC_XBS5_LPBIG_OFFBIG]            = 0,
    [_SC_XOPEN_LEGACY]                 = 0,
    [_SC_XOPEN_REALTIME]               = 0,
    [_SC_XOPEN_REALTIME_THREADS]       = 0,
    [_SC_ADVISORY_INFO]                = 0,
    [_SC_BARRIERS]                     = 0,
    [_SC_BASE]                         = 0,
    [_SC_C_LANG_SUPPORT]               = 0,
    [_SC_C_LANG_SUPPORT_R]             = 0,
    [_SC_CLOCK_SELECTION]              = 0,
    [_SC_CPUTIME]                      = 0,
    [_SC_THREAD_CPUTIME]               = 0,
    [_SC_DEVICE_IO]                    = 0,
    [_SC_DEVICE_SPECIFIC]              = 0,
    [_SC_DEVICE_SPECIFIC_R]            = 0,
    [_SC_FD_MGMT]                      = 0,
    [_SC_FIFO]                         = 0,
    [_SC_PIPE]                         = 0,
    [_SC_FILE_ATTRIBUTES]              = 0,
    [_SC_FILE_LOCKING]                 = 0,
    [_SC_FILE_SYSTEM]                  = 0,
    [_SC_MONOTONIC_CLOCK]              = 0,
    [_SC_MULTI_PROCESS]                = 0,
    [_SC_SINGLE_PROCESS]               = 0,
    [_SC_NETWORKING]                   = 0,
    [_SC_READER_WRITER_LOCKS]          = 0,
    [_SC_SPIN_LOCKS]                   = 0,
    [_SC_REGEXP]                       = 0,
    [_SC_REGEX_VERSION]                = 0,
    [_SC_SHELL]                        = 0,
    [_SC_SIGNALS]                      = 0,
    [_SC_SPAWN]                        = 0,
    [_SC_SPORADIC_SERVER]              = 0,
    [_SC_THREAD_SPORADIC_SERVER]       = 0,
    [_SC_SYSTEM_DATABASE]              = 0,
    [_SC_SYSTEM_DATABASE_R]            = 0,
    [_SC_TIMEOUTS]                     = 0,
    [_SC_TYPED_MEMORY_OBJECTS]         = 0,
    [_SC_USER_GROUPS]                  = 0,
    [_SC_USER_GROUPS_R]                = 0,
    [_SC_2_PBS]                        = 0,
    [_SC_2_PBS_ACCOUNTING]             = 0,
    [_SC_2_PBS_LOCATE]                 = 0,
    [_SC_2_PBS_MESSAGE]                = 0,
    [_SC_2_PBS_TRACK]                  = 0,
    [_SC_SYMLOOP_MAX]                  = 0,
    [_SC_STREAMS]                      = 0,
    [_SC_2_PBS_CHECKPOINT]             = 0,
    [_SC_V6_ILP32_OFF32]               = 0,
    [_SC_V6_ILP32_OFFBIG]              = 0,
    [_SC_V6_LP64_OFF64]                = 0,
    [_SC_V6_LPBIG_OFFBIG]              = 0,
    [_SC_HOST_NAME_MAX]                = 0,
    [_SC_TRACE]                        = 0,
    [_SC_TRACE_EVENT_FILTER]           = 0,
    [_SC_TRACE_INHERIT]                = 0,
    [_SC_TRACE_LOG]                    = 0,
    [_SC_LEVEL1_ICACHE_SIZE]           = 0,
    [_SC_LEVEL1_ICACHE_ASSOC]          = 0,
    [_SC_LEVEL1_ICACHE_LINESIZE]       = 0,
    [_SC_LEVEL1_DCACHE_SIZE]           = 0,
    [_SC_LEVEL1_DCACHE_ASSOC]          = 0,
    [_SC_LEVEL1_DCACHE_LINESIZE]       = 0,
    [_SC_LEVEL2_CACHE_SIZE]            = 0,
    [_SC_LEVEL2_CACHE_ASSOC]           = 0,
    [_SC_LEVEL2_CACHE_LINESIZE]        = 0,
    [_SC_LEVEL3_CACHE_SIZE]            = 0,
    [_SC_LEVEL3_CACHE_ASSOC]           = 0,
    [_SC_LEVEL3_CACHE_LINESIZE]        = 0,
    [_SC_LEVEL4_CACHE_SIZE]            = 0,
    [_SC_LEVEL4_CACHE_ASSOC]           = 0,
    [_SC_LEVEL4_CACHE_LINESIZE]        = 0,
    [_SC_IPV6]                         = 0,
    [_SC_RAW_SOCKETS]                  = 0,
    [_SC_V7_ILP32_OFF32]               = 0,
    [_SC_V7_ILP32_OFFBIG]              = 0,
    [_SC_V7_LP64_OFF64]                = 0,
    [_SC_V7_LPBIG_OFFBIG]              = 0,
    [_SC_SS_REPL_MAX]                  = 0,
    [_SC_TRACE_EVENT_NAME_MAX]         = 0,
    [_SC_TRACE_NAME_MAX]               = 0,
    [_SC_TRACE_SYS_MAX]                = 0,
    [_SC_TRACE_USER_EVENT_MAX]         = 0,
    [_SC_XOPEN_STREAMS]                = 0,
    [_SC_THREAD_ROBUST_PRIO_INHERIT]   = 0,
    [_SC_THREAD_ROBUST_PRIO_PROTECT]   = 0,
};
#pragma GCC diagnostic pop


/* Returns the user-space system configuration value for `name'.
 * For unknown names, `-EINVAL' is returned. */
PUBLIC ATTR_RARETEXT long int KCALL
kernel_sysconf(unsigned int name) {
 switch (name) {

 case _SC_OPEN_MAX:
 case _SC_STREAM_MAX:
  return THIS_HANDLE_MANAGER->hm_limit;

 default:
  break;
 }
 if (name < _SC_COUNT)
     return default_sysconf[name];
 return -EINVAL;
}

DEFINE_SYSCALL1(xsysconf,unsigned int,name) {
 return kernel_sysconf(name);
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_SYSCONF_C */
