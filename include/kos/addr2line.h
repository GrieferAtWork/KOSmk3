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
#ifndef _KOS_ADDR2LINE_H
#define _KOS_ADDR2LINE_H 1

#include <__stdinc.h>
#include <features.h>
#include <bits/types.h>
#include <bits/sigset.h>

__SYSDECL_BEGIN

#define MODULE_ADDR2LINE_FNORMAL 0x0000
#define MODULE_ADDR2LINE_FISSTMT 0x0001 /* The address is part of statement text, rather than that of an expression. */
#define MODULE_ADDR2LINE_FBBLOCK 0x0002 /* The address is the start of a basic_block. */
#define MODULE_ADDR2LINE_FINFUNC 0x0004 /* The address is part of a function, rather than data. */
#define MODULE_ADDR2LINE_FPROLOG 0x0010 /* The address is located within an auto-generated function prolog. */
#define MODULE_ADDR2LINE_FEPILOG 0x0020 /* The address is located within an auto-generated function epilog. */



struct dl_addr2line {
    /* Module address 2 line debug information. */
    void             *d_begin;   /* [1..1] Starting address of text describing debug information for the requested IP. */
    void             *d_end;     /* [1..1] End address of text describing debug information for the requested IP. */
    unsigned int      d_discr;   /* Source location discriminator. */
    unsigned int      d_srcno;   /* Compilation unit number (can be used to discriminate individual object files) */
    char const       *d_base;    /* [0..1] Source path base string (if non-NULL, should be prepended before `d_path') */
    char const       *d_path;    /* [0..1] The path of the associated source.
                                  *  NOTE: This string points into a private
                                  *        data block of `module_debug' data. */
    char const       *d_file;    /* [0..1] The filename of the associated source. */
    char const       *d_name;    /* [0..1] The name of the containing function (if known) */
    int               d_line;    /* Source line number (1-based; `0' if unknown) */
    int               d_column;  /* Source column number (1-based; `0' if unknown) */
    __uint16_t        d_flags;   /* Set of `MODULE_ADDR2LINE_F*' */
    __uint16_t      __d_pad;     /* ... */
    /* String data not mapped in user-space is written here. */
};

#ifndef __KERNEL__
struct cpu_context;
struct fpu_context;

/* Query debug informations on the given address.
 * @return: * : The required buffer size.
 * @return: 0 : No debug informations available for `ABS_PC'.
 * @return: -1: Failed to query debug informations (see errno / @throw below)
 * @throw: E_SEGFAULT:         The given `BUF ... += BUFSIZE' is faulty.
 * @throw: E_BUFFER_TOO_SMALL: [Xxdladdr2line] The given buffer is too small.
 */
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY __WUNUSED,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,xdladdr2line,
                 (void *__abs_pc, struct dl_addr2line *__restrict __buf, __size_t __bufsize),(__abs_pc,__buf,__bufsize))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __size_t (__LIBCCALL Xxdladdr2line)(void *__abs_pc, struct dl_addr2line *__restrict __buf, __size_t __bufsize);
#endif /* __USE_EXCEPT */

/* Unwind the given `CPU_CONTEXT' and `FPU_CONTEXT' to unwind a single stack-frame.
 * Additionally, if the stack-frame refers to a posix_signal-frame, update
 * `SIGNAL_SET' (when non-NULL) to refer to the new (updated) signal set.
 * @param: CPU_CONTEXT: [1..1] The CPU context to update.
 * @param: FPU_CONTEXT: [0..1] The FPU context to update (not updated when `NULL').
 * @param: SIGNAL_SET:  [0..1] The set of blocked signals (updated if a posix_signal-frame is unwound)
 * @return: 0 / true:                   Successfully unwound the stack-frame.
 * @return: -1 (errno = EPERM) / false: Failed to unwind the stack-frame
 *                                     (no unwind information present, or corrupted register state)
 * @throw: E_SEGFAULT: One of the given pointers is faulty. */
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY __WUNUSED,int,__LIBCCALL,xunwind,
                 (struct cpu_context *__restrict __cpu_context,
                  struct fpu_context *__fpu_context, __sigset_t *__signal_set),
                 (__cpu_context,__fpu_context,__signal_set))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __WUNUSED __BOOL (__LIBCCALL Xxunwind)(struct cpu_context *__restrict __cpu_context,
                                                             struct fpu_context *__fpu_context,
                                                             __sigset_t *__signal_set);
#endif /* __USE_EXCEPT */
#endif /* !__KERNEL__ */


__SYSDECL_END

#endif /* !_KOS_ADDR2LINE_H */
