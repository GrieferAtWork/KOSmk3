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
#ifndef GUARD_LIBS_LIBC_LIBC_H
#define GUARD_LIBS_LIBC_LIBC_H 1

#include "../../hybrid/hybrid.h"
#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <hybrid/timespec.h>
#include <hybrid/timeval.h>
#include <bits/sigset.h>
#include <bits/siginfo.h>
#include <bits/mbstate.h>
#include <bits/sigcontext.h>
#include <bits/waitflags.h>
#include <bits/stat.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <bits/fcntl-linux.h>
#include <bits/sched.h>
#include <bits/sigaction.h>
#include <bits/signum.h>
#include <time.h>
#include <utime.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <stdint.h>

#ifndef __KERNEL__
DECL_BEGIN

#define LIBC_ENUMERATE_MALLOC_EXT(x) x
#define LIBC_ENUMERATE_MALLOC_FUNCTIONS(callback) \
    callback(Xmalloc) \
    callback(Xcalloc) \
    callback(Xmemalign) \
    callback(Xmemalign_offset) \
    callback(Xmemcalign) \
    callback(Xmemcalign_offset) \
    callback(Xvalloc) \
    callback(Xpvalloc) \
    callback(Xrealloc) \
    callback(Xrecalloc) \
    callback(Xrealign) \
    callback(Xrealign_offset) \
    callback(Xrecalign) \
    callback(Xrecalign_offset) \
    callback(Xrealloc_in_place) \
    callback(Xrecalloc_in_place) \
    callback(Xmemdup) \
    callback(Xmemcdup) \
    callback(Xstrdup) \
    callback(Xstrndup) \
    LIBC_ENUMERATE_MALLOC_EXT(callback(Xw16dup)) \
    LIBC_ENUMERATE_MALLOC_EXT(callback(Xw32dup)) \
    LIBC_ENUMERATE_MALLOC_EXT(callback(Xw16ndup)) \
    LIBC_ENUMERATE_MALLOC_EXT(callback(Xw32ndup)) \
    callback(malloc) \
    callback(calloc) \
    callback(memalign) \
    callback(memalign_offset) \
    callback(memcalign) \
    callback(memcalign_offset) \
    callback(valloc) \
    callback(pvalloc) \
    callback(malloc_usable_size) \
    callback(realloc) \
    callback(recalloc) \
    callback(realign) \
    callback(realign_offset) \
    callback(recalign) \
    callback(recalign_offset) \
    callback(realloc_in_place) \
    callback(recalloc_in_place) \
    callback(free) \
    callback(posix_memalign) \
    callback(mallopt) \
    callback(malloc_trim) \
    callback(memdup) \
    callback(memcdup) \
    callback(strdup) \
    callback(strndup) \
    LIBC_ENUMERATE_MALLOC_EXT(callback(w16dup)) \
    LIBC_ENUMERATE_MALLOC_EXT(callback(w32dup)) \
    LIBC_ENUMERATE_MALLOC_EXT(callback(w16ndup)) \
    LIBC_ENUMERATE_MALLOC_EXT(callback(w32ndup)) \
    callback(mallinfo) \
    callback(malloc_stats) \
/**/


#define CRT_COLD             INTERN ATTR_SECTION(".text.crt.cold")        /* Rarely used CRT functions (usually for very specific purposes). */
#define CRT_RARE             INTERN ATTR_SECTION(".text.crt.rare")        /* Rarely used CRT functions (usually for dealing with errors). */
#define CRT_RARE_BSS         INTERN ATTR_SECTION(".bss.crt.rare")         /* Rarely used .bss. */
#define CRT_RARE_DATA        INTERN ATTR_SECTION(".data.crt.rare")        /* Rarely used .data. */
#define CRT_RARE_RODATA      INTERN ATTR_SECTION(".rodata.crt.rare")      /* Rarely used .rodata. */
#define CRT_DRARE            INTERN ATTR_SECTION(".data.crt.rare")        /* Rarely used CRT functions (usually for dealing with errors). */
#define CRT_EXCEPT           INTERN ATTR_SECTION(".text.crt.except")      /* Exception-related functions. */
#define CRT_EXCEPT_RARE      INTERN ATTR_SECTION(".text.crt.except.rare") /* Exception-related functions. */
#define CRT_TIMER            INTERN ATTR_SECTION(".text.crt.timer")       /* Timer (periodic timer)-related functions. */
#define CRT_ITIMER           INTERN ATTR_SECTION(".text.crt.itimer")      /* Itimer (periodic timer)-related functions. */
#define CRT_CLOCK            INTERN ATTR_SECTION(".text.crt.clock")       /* Clock (profiling)-related functions. */
#define CRT_TIME             INTERN ATTR_SECTION(".text.crt.time")        /* Time-related functions */
#define CRT_STDIO            INTERN ATTR_SECTION(".text.crt.stdio")       /* stdio (FILE) related core functions */
#define CRT_STDIO_API        INTERN ATTR_SECTION(".text.crt.stdio_api")   /* stdio (FILE) related API functions */
#define CRT_STDIO_XAPI       INTERN ATTR_SECTION(".text.crt.except.stdio_api") /* stdio (FILE) related wide-character API functions (with exception support) */
#define CRT_W16STDIO_API     INTERN ATTR_SECTION(".text.crt.dos.widechar.stdio")/* stdio (FILE) related wide-character API functions */
#define CRT_W16STDIO_XAPI    INTERN ATTR_SECTION(".text.crt.except.widechar16.stdio") /* stdio (FILE) related wide-character API functions (with exception support) */
#define CRT_W32STDIO_API     INTERN ATTR_SECTION(".text.crt.widechar32.stdio") /* stdio (FILE) related wide-character API functions */
#define CRT_W32STDIO_XAPI    INTERN ATTR_SECTION(".text.crt.except.widechar32.stdio") /* stdio (FILE) related wide-character API functions (with exception support) */
#define CRT_TTY              INTERN ATTR_SECTION(".text.crt.tty")         /* TTY-related functions */
#define CRT_TTY_EXCEPT       INTERN ATTR_SECTION(".text.crt.except.tty")  /* TTY-related functions */
#define CRT_KOS              INTERN ATTR_SECTION(".text.crt.kos")         /* KOS-specific functions not found anywhere else. */
#define CRT_KOS_DL           INTERN ATTR_SECTION(".text.crt.kos.dl")      /* KOS dynamic linking related functions. */
#define CRT_KOS_DP           INTERN ATTR_SECTION(".text.crt.kos.dp")      /* Deprecated KOS functionality. */
#define CRT_KOS_DP_DATA      INTERN ATTR_SECTION(".data.crt.kos.dp")      /* Deprecated KOS functionality. */
#define CRT_KOS_DP_BSS       INTERN ATTR_SECTION(".bss.crt.kos.dp")       /* Deprecated KOS functionality. */
#define CRT_GLC              INTERN ATTR_SECTION(".text.crt.glc")         /* GLibC-specific functions (ones that are implemented again for better KOS-integration, but still provided for compatibility; e.g. `stat()'). */
#define CRT_NET              INTERN ATTR_SECTION(".text.crt.net")         /* Networking-related functions. */
#define CRT_DOS              INTERN ATTR_SECTION(".text.crt.dos")         /* DOS-specific functions. */
#define CRT_DOS_RARE         INTERN ATTR_SECTION(".text.crt.dos.rare")    /* Rare, DOS-specific text. */
#define CRT_DOS_DATA         INTERN ATTR_SECTION(".data.crt.dos")         /* DOS-specific data. */
#define CRT_DOS_RODATA       INTERN ATTR_SECTION(".rodata.crt.dos")       /* DOS-specific rodata. */
#define CRT_DOS_BSS          INTERN ATTR_SECTION(".bss.crt.dos")          /* DOS-specific bss. */
#define CRT_DOS_EXT          INTERN ATTR_SECTION(".text.crt.dos.ext")     /* DOS-specific functions not found in msvcrt.dll */
#define CRT_DOS_NATIVE       INTERN ATTR_SECTION(".text.crt.dos.native")  /* DOS-specific functions that use dedicated system calls. */
#define CRT_WIDECHAR         INTERN ATTR_SECTION(".text.crt.widechar")
#define CRT_WIDECHAR_EXCEPT  INTERN ATTR_SECTION(".text.crt.except.widechar")
#define CRT_WIDECHAR_RODATA  INTERN ATTR_SECTION(".rodata.crt.widechar")


#ifdef __i386__
#define CONFIG_LIBC_HAVE_ARCH_ENTRY 1 /* Arch defines `__entry1' */
#define CONFIG_LIBC_HAVE_ARCH_EXIT  1 /* Arch defines `exit' */
#endif

#ifdef __CC__
#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */
typedef int derrno_t; /* Dos errno value. */
#ifndef __std_FILE_defined
#define __std_FILE_defined 1
__NAMESPACE_STD_BEGIN
typedef __FILE FILE;
__NAMESPACE_STD_END
#endif /* !__std_FILE_defined */
#ifndef __FILE_defined
#define __FILE_defined 1
__NAMESPACE_STD_USING(FILE)
#endif /* !__FILE_defined */


#ifndef __fsmask_defined
#define __fsmask_defined 1
struct fsmask {
    __UINT32_TYPE__ fm_mask; /* Filesystem mode mask. (Set of `AT_*') */
    __UINT32_TYPE__ fm_mode; /* Filesystem mode. (Set of `AT_*') */
};
#endif /* !__fsmask_defined */

/* Helper macros for defining exception wrappers */
#define DEFINE_EXCEPT_WRAPPER(Treturn,error_return,name,params,Xname,args) \
INTERN Treturn (LIBCCALL name) params { \
    Treturn COMPILER_IGNORE_UNINITIALIZED(result); \
    LIBC_TRY { \
        result = Xname args; \
    } LIBC_EXCEPT(libc_except_errno()) { \
        result = error_return; \
    } \
    return result; \
}
#define DEFINE_NULL_WRAPPER(Treturn,name,params,Xname,args) DEFINE_EXCEPT_WRAPPER(Treturn,NULL,name,params,Xname,args)
#define DEFINE_M1_WRAPPER(Treturn,name,params,Xname,args)   DEFINE_EXCEPT_WRAPPER(Treturn,-1,name,params,Xname,args)

INTDEF char const libc_empty_string[];
INTDEF char16_t const libc_empty_string16[] ASMNAME("libc_empty_string");
INTDEF char32_t const libc_empty_string32[] ASMNAME("libc_empty_string");

INTDEF ATTR_NORETURN void LIBCCALL
libc_throw_buffer_too_small(size_t reqsize, size_t bufsize);

#endif /* __CC__ */

DECL_END
#endif /* !__KERNEL__ */

#endif /* !GUARD_LIBS_LIBC_LIBC_H */
