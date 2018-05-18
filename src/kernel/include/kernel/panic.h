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
#ifndef GUARD_KERNEL_INCLUDE_KERNEL_PANIC_H
#define GUARD_KERNEL_INCLUDE_KERNEL_PANIC_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <stdarg.h>

DECL_BEGIN

#ifdef __CC__

/* Shoot down all secondary CPUs and unschedule all but the
 * calling thread from the set of actively scheduled threads.
 * Threads that were sleeping with a timeout will never wake up.
 * This function doesn't care about potentially creating memory
 * leaks from what it does. It only cares about killing everything
 * but the calling thread, and doing so with as little risk as possible.
 * NOTE: If multiple CPUs call this function at once, only
 *       the first will be able to shoot down the other.
 * @return: true:  Ok. - You're alone now!
 * @return: false: This is bad. - The calling thread caused a kernel panic recursion... */
FUNDEF ATTR_NOTHROW bool KCALL kernel_panic_shootdown(void);

/* Delete locks to all major synchronization locations, including
 * VM locks, as well as some other, ensuring that the calling thread
 * will be able to operate with most of the kernel without running
 * into any problems caused by other locks that may have been held
 * by other threads prior to calling `kernel_panic_shootdown()'. */
FUNDEF ATTR_NOTHROW void KCALL kernel_panic_lockall(unsigned int mode);
#define PANIC_LOCK_EVERYTHING   ((unsigned int)-1)
#define PANIC_LOCK_GENERIC        PANIC_LOCK_GLOBALS
#define PANIC_LOCK_GLOBALS        0x0000 /* (Always on) Lock global and the caller's locks. */
#define PANIC_LOCK_PERTASK        0x0001 /* Lock thread-local components of all threads. */


/* Tries its very best to print a traceback for the given context. */
FUNDEF ATTR_NOTHROW void KCALL
kernel_panic_printtb(struct task *__restrict thread,
                     struct cpu_context *__restrict ctx,
                     unsigned int num_skip_frames);

/* Try to switch back to text mode so that we can print to screen.
 * @param: colors: Color attributes (Set of
 *                 <TODO: arch-specific color flags; on X86 these are VGA character attributes>) */
FUNDEF ATTR_NOTHROW void KCALL kernel_panic_textmode(unsigned int colors);

/* Print kernel panic text.
 * Printed text is written to a serial debug connection (if existent),
 * as well as the display (when `kernel_panic_textmode()' was called before)
 * NOTE: The display print interpreter can only deal with
 *       the special characters `\n', `\r' and `\t'. */
FUNDEF ATTR_NOTHROW void ATTR_CDECL kernel_panic_printf(char const *__restrict format, ...);
FUNDEF ATTR_NOTHROW void KCALL kernel_panic_vprintf(char const *__restrict format, va_list args);

/* Switch the internal kernel panic sync() latch to prevent syncing.
 * This may automatically be done by `kernel_panic_lockall()' if it
 * is detected that some filesystem-data-related lock was held when
 * the system crashed.
 * NOTE: data-related lock here does not include read-locks, such
 *       as a kernel panic while rwlock_read() was called on some
 *       INode. - It only refers to locks indicative of data
 *       manipulations, meaning while locks, and write only! */
FUNDEF ATTR_NOTHROW void KCALL kernel_panic_dontsync(void);

/* Synchronize filesystems unless `kernel_panic_dontsync()' had been
 * called, in which case this function returns immediately. */
FUNDEF ATTR_NOTHROW void KCALL kernel_panic_sync(void);

/* Kernel panic reasons. */
#define KERNEL_PANIC_ASSERTION_FAILED       0x0000 /* Assertion failure. */
#define KERNEL_PANIC_UNHANDLED_EXCEPTION    0x0001 /* Unhandled exception handler (in a kernel thread). */
#define KERNEL_PANIC_DOUBLE_FAULT           0x0002 /* Double fault. */
#define KERNEL_PANIC_NON_MASKABLE_INTERRUPT 0x0003 /* Non-maskable interrupt. */
#define KERNEL_PANIC_UNHANDLED_INTERRUPT    0x0004 /* Unhandled interrupt. */



/* Initiate a quick kernel panic and freeze.
 * @param: reason:          One of `KERNEL_PANIC_*', optionally
 *                          or'd with a set of `KERNEL_PANIC_F*'
 * @param: num_skip_frames: The number of frames to skip in the dumped traceback. */
FUNDEF ATTR_NORETURN void ATTR_CDECL
kernel_panic(unsigned int reason,
             unsigned int num_skip_frames,
             char const *__restrict message, ...);
#define KERNEL_PANIC_FDUMPCLL               0x0000 /* FLAG: Dump a traceback using the caller's CPU context. */
#define KERNEL_PANIC_FDUMPEXC               0x8000 /* FLAG: Dump a traceback using the CPU context from error_info(). */
#define KERNEL_PANIC_FNOSYNC                0x4000 /* FLAG: Don't synchronize filesystems. */


#endif /* __CC__ */

DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_KERNEL_PANIC_H */
