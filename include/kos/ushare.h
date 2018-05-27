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
#ifndef _KOS_USHARE_H
#define _KOS_USHARE_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/limits.h>

#if defined(__i386__) || defined(__x86_64__)
#include "i386-kos/asm/ushare.h"
#else
#error "Unsupported arch"
#endif

__SYSDECL_BEGIN

/* A user-space-only address hint that is suggested
 * to-be used when mapping USHARE segments.
 * However, you're allowed to map them anywhere, though mapping
 * them at the standard address makes debugging easier, consider
 * that you'll be able to quickly realize what you're dealing
 * with by simply looking at an address. */
#ifndef USHARE_ADDRESS_HINT
#error "Platform forgot to `#define USHARE_ADDRESS_HINT ...'"
#endif /* !USHARE_ADDRESS_HINT */


/* Invalid ushare-share segment. */
#define USHARE_INVALID           0x00000000
#define USHARE_NAME(group,id)  ((group) << 24 | (id))



 
/* A ushare segment containing the names of `errno' codes. */
#define USHARE_STRERROR_FNAME          USHARE_NAME('K','e')
#define USHARE_STRERROR_FSIZE        ((8192+(__PAGESIZE-1)) & ~(__PAGESIZE-1))
#define USHARE_STRERROR_VER_KOSMK2 0 /* The original (KOS Mk2) strerror format. */
#define USHARE_STRERROR_VER_KOSMK3 1 /* The new (KOS Mk3) strerror format, also including signal names. */
#ifndef USHARE_STRERROR_VERSION
#define USHARE_STRERROR_VERSION    \
        USHARE_STRERROR_VER_KOSMK3 /* The current strerror() version number. */
#endif
#define USHARE_STRERROR_SIGINDEX(signo) ((signo)-1)
#ifdef __CC__
struct ushare_strerror_entry {
    __UINT16_TYPE__  se_name;    /* Offset into 'se_strtab', to the name of the error. */
    __UINT16_TYPE__  se_text;    /* Offset into 'se_strtab', to a string describing the error. */
};
struct ushare_strerror_signl {
    __UINT16_TYPE__  se_name;    /* Offset into 'se_strtab', to the name of the signal. */
    __UINT16_TYPE__  se_text;    /* Offset into 'se_strtab', to a string describing the signal. */
};
struct ushare_strerror {
    /* This structure is mapped by the `USHARE_STRERROR_FNAME' segment.
     * NOTE: If the system locale changes, it will be updated immediately
     *       unless copy-on-write was used by a process to gain a private,
     *       writable copy of strerror data.
     * HINT: In KOS Mk2, data found in the STRERROR USHARE segment used to be
     *       addressable using the `xsharesym' system call with `"errnotext"'
     *       However, that system call no longer exists in KOS Mk3 due to its
     *       security problems and complication caused by allowing user-space
     *       to access a small portion of kernel-space, as well as having known
     *       data mapped at a fixed address. */
    __UINT32_TYPE__  se_version; /* [== USHARE_STRERROR_VER_KOSMK3]
                                  * (Strerror-data version; will always be backwards-compatible). */
    __INT32_TYPE__   se_strtab;  /* Offset from `USHARE_STRERROR' to a string table. */
    __INT32_TYPE__   se_enotab;  /* Offset from `USHARE_STRERROR' to a vector of `struct ushare_strerror_entry'. */
    __UINT32_TYPE__  se_enocnt;  /* Amount of entires in `se_enotab'. */
    __UINT32_TYPE__  se_enoent;  /* Size of a single entry within `se_enotab'. */
#if USHARE_STRERROR_VERSION >= USHARE_STRERROR_VER_KOSMK3
    __INT32_TYPE__   se_sigtab;  /* Offset from `USHARE_STRERROR' to a vector of `struct ushare_strerror_signl'.
                                  * NOTE: The index in this vector is returned by `USHARE_STRERROR_SIGINDEX()'. */
    __UINT32_TYPE__  se_sigcnt;  /* Amount of entires in `se_sigtab'. */
    __UINT32_TYPE__  se_sigent;  /* Size of a single entry within `se_sigtab'. */
#endif
};
#endif /* __CC__ */


/* A ushare segment containing kernel version information.
 * The information contained is formatted as a `struct utsname'
 * structure that is padded with ZERO-bytes to the next page-boundry.
 * HINT: `struct utsname' is defined in `<sys/utsname.h>'
 * HINT: This ushare segment is also used to implement `gethostname()' */
#define USHARE_UTSNAME_FNAME          USHARE_NAME('K','n')
#define USHARE_UTSNAME_FSIZE        ((sizeof(struct utsname)+(__PAGESIZE-1)) & ~(__PAGESIZE-1))




/* A VIO-based ushare segment containing process information.
 * When mapped, memory contents can be read as the following structure.
 * NOTE: This is less a useful memory mapping and more a demonstration
 *       of the VIO functionality of the KOS kernel, as this memory
 *       mapping doesn't actually have any real physical memory backing
 *       its contents, but rather uses virtual I/O mappings (emulated
 *       memory access) in order to implement what appears to be regular
 *       memory access.
 *       However, on the kernel-side, there's a function
 *       >> u32 procinfo_getl(uintptr_t address);
 *       that is invoked whenever user-space assembly accesses any part
 *       of the procinfo memory mappings (sorry to ruin the magic...) */
#define USHARE_PROCINFO_FNAME         USHARE_NAME('K','p')
#define USHARE_PROCINFO_FSIZE \
      ((sizeof(struct ushare_procinfo)+(__PAGESIZE-1)) & ~(__PAGESIZE-1))
#ifdef __CC__
struct ushare_procinfo {
    volatile __UINT32_TYPE__      pi_tid;        /* The calling thread's TID. */
    volatile __UINT32_TYPE__      pi_pid;        /* The calling process's PID. */
    volatile __UINT32_TYPE__      pi_ppid;       /* The calling process parent PID. */
    volatile __UINT32_TYPE__      pi_gpid;       /* The calling process group's GPID. */
    volatile __UINT32_TYPE__      pi_sid;        /* The calling process group's session id. */
    volatile __UINT32_TYPE__      pi_hz;         /* The number of jiffies passing every second. */
    volatile __UINT64_TYPE__      pi_time;       /* The current system time in jiffies. */
    struct {
        volatile __UINT64_TYPE__  t_start;       /* The time (in jiffies) when the current thread was started. */
        volatile __UINT32_TYPE__  t_hswitch;     /* Amount of times the thread was preempted while in kernel-space. */
        volatile __UINT32_TYPE__  t_uswitch;     /* Amount of times the thread was preempted while in user-space. */
        volatile __UINT32_TYPE__  t_hyield;      /* Amount of times the thread called `task_yield()' (aka. from kernel-space) */
        volatile __UINT32_TYPE__  t_uyield;      /* Amount of times the thread called `sched_yield()' (aka. from user-space) */
        volatile __UINT32_TYPE__  t_sleep;       /* Amount of times the thread entered a sleeping-state. */
        volatile __UINT32_TYPE__  t_xrpc;        /* Amount of RPC functions served by this thread (including those send by the thread itself). */
        volatile __UINT32_TYPE__  t_qrpc;        /* Amount of RPC functions queued (sent) by this thread (for execution by other threads, or the thread itself). */
    }                             pi_thread;     /* Statistical information about the calling thread. */
    volatile __UINT32_TYPE__      pi_exit;       /* Writing to this field will cause the calling thread to exit with the value written as status. */
    volatile __UINT32_TYPE__      pi_exit_group; /* Writing to this field will cause the calling process to exit with the value written as status. */
    volatile __UINT32_TYPE__      pi_rand;       /* Evaluates to a 32-bit pseudo-random integer every time it is read from (using the kernel's `rand()' function) */
    volatile __UINT32_TYPE__      pi_counter;    /* Returns how often this field has already been read from.
                                                  * NOTE: The counter for this is system-wide and incremented atomically,
                                                  *       meaning you could use this to generate system-wide unique IDs.
                                                  *       However, there is no way of safely detecting when it overflows... */
};
#endif

__SYSDECL_END

#endif /* !_KOS_USHARE_H */
