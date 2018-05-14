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
#ifndef _KOS_EXCEPT_H
#define _KOS_EXCEPT_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <bits/types.h>

#if defined(__i386__) || defined(__x86_64__)
#include "i386-kos/except.h"
#else
#error "Unsupported arch"
#endif

__DECL_BEGIN

#define __SIZEOF_EXCEPT_T__ 2
#ifdef __CC__
typedef __UINT16_TYPE__ except_t;
#endif /* __CC__ */

/* Unhandled exception handling / exception handling in different situations:
 *    [[EXCEPTION_IN_SYSTEM_CALL]]
 *       >> if (ARE_EXCEPTIONS_ENABLED(system_call_number)) {
 *       >>     SET_EXCEPTION_FLAG(ERR_FSYSCALL_XPT);
 *       >> } else {
 *       >>     SET_EXCEPTION_FLAG(ERR_FSYSCALL);
 *       >> }
 *       >> PROPAGATE_EXCEPTION();
 *
 *    [[EXCEPTION_IN_INTERRUPT]]
 *       >> PROPAGATE_EXCEPTION();
 *
 *    [[EXCEPTION_IN_USERSPACE]]
 *       >> PROPAGATE_EXCEPTION();
 *
 *    [[EXCEPTION_IN_KERNELJOB]]
 *       >> UNHANDLED_EXCEPTION();
 *    
 *    UNHANDLED_EXCEPTION()
 *       >>    UNHANDLED_EXCEPTION_EX(true);
 *
 *    UNHANDLED_EXCEPTION_EX(is_thread_alone)
 *       >>    if (error_code() == E_INTERRUPT ||
 *       >>        error_code() == E_EXIT_THREAD ||
 *       >>        error_code() == E_EXIT_PROCESS)
 *       >>        EXIT_THREAD(error_info()->e_exit.e_status);
 *       >>    exit_code = GENERATE_EXITCODE(error_code());
 *       >>    
 *       >>    ... // - Dump information (`error_print()'-style)
 *       >>        // - Create a coredump
 *       >>        // - Break into a debugger
 *       >>        // - etc... (Exact behavior depends on circumstances)
 *       >>    
 *       >>    if (is_thread_alone)
 *       >>        EXIT_THREAD(exit_code);
 *       >>    else {
 *       >>        EXIT_PROCESS(exit_code);
 *       >>    }
 *
 *    PROPAGATE_EXCEPTION()
 *          """Propagate exception to user-space"""
 *       >>    is_thread_alone = false;
 *       >>    info = *error_info();
 *       >>serve_rpc:
 *       >>    TRY {
 *       >>        SERVE_RPC_BEFORE_USER();
 *       >>    } EXCEPT(1) {
 *       >>        if (PRIORITY(error_info()) > PRIORITY(info))
 *       >>            info = *error_info();
 *       >>        goto serve_rpc;
 *       >>    }
 *       >>
 *       >>    // Special exceptions
 *       >>    if (info.e_code == E_USER_RESUME)
 *       >>        RETURN_TO_USERSPACE();
 *       >>    if (info.e_code == E_EXIT_PROCESS)
 *       >>        EXIT_PROCESS(info.e_exit.e_status);
 *       >>    if (info.e_code == E_EXIT_THREAD)
 *       >>        EXIT_THREAD(info.e_exit.e_status);
 *       >>
 *       >>    // Exception in system call without exception support
 *       >>    if (info.e_flag & ERR_FSYSCALL)
 *       >>        RETURN_TO_USERSPACE(TRANSLATE_TO_ERRNO(info));
 *       >>
 *       >>    // High-level, KOS-specific exception handling
 *       >>    if (THREAD_HAS_USER_TASK_SEGMENT()) {
 *       >>        unwind = GET_USER_RETURN_CONTEXT();
 *       >>        if (UNWIND_STACK_AND_FIND_EXCEPTION_HANDLER_FOR(unwind,info)) {
 *       >>            COPY_EXCEPTION_INFO(info,GET_USER_TASK_SEGMENT());
 *       >>            COPY_CPU_CONTEXT(GET_USER_RETURN_CONTEXT(),GET_USER_TASK_SEGMENT());
 *       >>            RETURN_TO_USERSPACE();
 *       >>        }
 *       >>    }
 *       >>
 *       >>    // Posix-signal-based exception handling
 *       >>    if (EXCEPTION_HAS_SIGNAL(info.e_code)) {
 *       >>        siginfo_t sinfo = CREATE_EXCEPTION_SIGNAL(info);
 *       >>        if (IS_NONDEFAULT_SIGNALING_BEHAVIOR(GET_CURRENT_SIGHAND(),sinfo.si_signo)) {
 *       >>            REDIRECT_SIGNAL_HANDLER_WITH_EXCEPTION(sinfo,
 *       >>                                                   GET_ACTION(GET_CURRENT_SIGHAND(),sinfo.si_signo),
 *       >>                                                   GET_USER_RETURN_CONTEXT());
 *       >>            RETURN_TO_USERSPACE();
 *       >>        }
 *       >>    }
 *       >>
 *       >>    // Unhandled exception handling
 *       >>    if (THREAD_HAS_USER_TASK_SEGMENT()) {
 *       >>        struct user_task_segment *seg = GET_USER_TASK_SEGMENT();
 *       >>        if (seg->ts_state & THREAD_STATE_FALONE)
 *       >>            is_thread_alone = true;
 *       >>        if (!(seg->ts_state & THREAD_STATE_FINUEH)) {
 *       >>            void *ueh = seg->ts_ueh;
 *       >>            if (ueh == NULL) {
 *       >>                // Name not documented, but symbol is exported by
 *       >>                // `libc', which will forward the call to the user-
 *       >>                // space variant of `error_unhandled_exception()',
 *       >>                // meaning if you want your function to be called
 *       >>                // here, you must simply define a function:
 *       >>                // >> ATTR_NORETURN void error_unhandled_exception(void);
 *       >>                ueh = FIND_GLOBAL_SYMBOL("...");
 *       >>            }
 *       >>            if (ueh != NULL) {
 *       >>                SET_CONTEXT_IP(GET_USER_RETURN_CONTEXT(),ueh);
 *       >>                if (seg->ts_ueh_sp != NULL)
 *       >>                    SET_CONTEXT_SP(GET_USER_RETURN_CONTEXT(),seg->ts_ueh_sp);
 *       >>                // Always ensure that user-space will have no problems
 *       >>                // accessing error information using the TLS register.
 *       >>                SET_USER_TLS_REGISTER(GET_USER_RETURN_CONTEXT(),
 *       >>                                      GET_USER_TASK_SEGMENT());
 *       >>                RETURN_TO_USERSPACE();
 *       >>            }
 *       >>        }
 *       >>    }
 *       >>
 *       >>    // Fallback: Unhandled exception
 *       >>    // HINT: `is_thread_alone' may have been overwritten
 *       >>    //        by the user-space thread segment.
 *       >>    UNHANDLED_EXCEPTION_EX(is_thread_alone);
 */




/* Exception code allocations:
 *
 *   0000 ... 0000     Reserved exception code that can be used to indicate a no-error situation.
 *                     However, that DOESN'T mean you can do something like `if (error_code() == E_OK) ...'
 *   0001 ... 00ff     Standard, software-generated exception codes
 *   0100 ... 7fff     User-defined exception codes (not standartized; application specific)
 *                     NOTE: In kernel-space, codes `0800 ... 08ff' are reserved for kernel-specific uses.
 *   8000 ... efff     Reserved for future use.
 *   f000 ... f0ff     Standard, software-generated signal codes.
 *                     Signals have lower priority than exceptions in situations where more than one exception is encountered.
 *   f100 ... faff     User-defined signal codes (not standartized; application specific)
 *                     NOTE: In kernel-space, codes `f800 ... f8ff' are reserved for kernel-specific uses.
 *                     Signals are usually meant to travel short distances, acting as a faster substitute for
 *                     situations where `setjmp()' / `longjmp()' would be used in portable applications.
 *                     These exceptions usually aren't supposed to leave the source-file scope.
 *   fb00 ... fdff     Reserved for future use.
 *   fe00 ... fe7f     Exceptions that have special meaning (RTL exceptions), in that they are meant to
 *                     be handled by unhandled exception handlers. This includes the codes for exiting
 *                     the current thread or process. (NOTE: These codes have the highest priority)
 *   fe80 ... febf     Exceptions raised under special circumstances when an error state is detected
 *                     that the system does not know how to deal with, such as attempting to resume
 *                     a non-continuable exception, or when an undefined hardware interrupt occurs.
 *   fec0 ... feff     CPU-specific exceptions (such as `E_INVALID_SEGMENT' on i386).
 *   ff00 ... ffff     Hardware-generated exceptions, usually caused by interrupts
 *                     This includes, but is not limited to: SEGFAULT, DIVIDE_BY_ZERO, etc...
 */


/* Macros for addressing bounds of error code ranges documented above */
#define ERRORCODE_STDERROR_MIN       0x0001
#define ERRORCODE_STDERROR_MAX       0x00ff
#define ERRORCODE_USERERROR_MIN      0x0100
#define ERRORCODE_USERERROR_MAX      0x7fff
#define ERRORCODE_RESERVED1_MIN      0x8000
#define ERRORCODE_RESERVED1_MAX      0xefff
#define ERRORCODE_STDSIGNAL_MIN      0xf000
#define ERRORCODE_STDSIGNAL_MAX      0xf0ff
#define ERRORCODE_USERSIGNAL_MIN     0xf100
#define ERRORCODE_USERSIGNAL_MAX     0xfaff
#define ERRORCODE_RESERVED2_MIN      0xfb00
#define ERRORCODE_RESERVED2_MAX      0xfdff
#define ERRORCODE_RTL_MIN            0xfe00
#define ERRORCODE_RTL_MAX            0xfe7f
#define ERRORCODE_UNHANDLED_MIN      0xfe80
#define ERRORCODE_UNHANDLED_MAX      0xfebf
#define ERRORCODE_CPU_MIN            0xfec0
#define ERRORCODE_CPU_MAX            0xfeff
#define ERRORCODE_HARDWARE_MIN       0xff00
#define ERRORCODE_HARDWARE_MAX       0xffff

/* Check if the given error is low- (may get overwritten
 * by !LOW || HIGH), or high- (doesn't get overwritten) priority.
 * This mechanism is used for dealing with exceptions thrown by
 * RPC function callbacks executed prior to returning to user-space
 * when checking how an exception should be propagated to user-space. */
#define ERRORCODE_ISLOWPRIORITY(x)   ((x) >= ERRORCODE_STDSIGNAL_MIN && (x) <= ERRORCODE_USERSIGNAL_MAX)
#define ERRORCODE_ISHIGHPRIORITY(x)  ((x) >= ERRORCODE_RTL_MIN && (x) <= ERRORCODE_UNHANDLED_MAX)



/* Exception codes. */
#define E_OK                     0x0000          /* Never thrown; may be used to indicate error-less state */
#define E_BADALLOC               0x0001          /* [ERRNO(ENOMEM)] Failed to allocate sufficient resources. */
#define E_INVALID_ARGUMENT       0x0002          /* [ERRNO(EINVAL)] Invalid argument. */
#define E_NOT_IMPLEMENTED        0x0003          /* [ERRNO(ENOSYS)] Unimplemented function. */
#define E_INVALID_HANDLE         0x0004          /* [ERRNO(EBADF)] Bad file descriptor. */
#define E_WOULDBLOCK             0x0005          /* [ERRNO(EWOULDBLOCK)] A blocking call was attempted when non-blocking behavior was requested. */
#define E_NO_DATA                0x0006          /* [ERRNO(ENODATA)] No data is available. */
#define E_PROCESS_EXITED         0x0016          /* [ERRNO(ESRCH)] The task controller for the specified process has already been deallocated.
                                                  * This implies that the process has exited. However, it doesn't imply that
                                                  * the task controller is immediately deallocated when a process exists. */
#define E_NO_DEVICE              0x0026          /* [ERRNO(ENODEV)] The named device doesn't exist. */
#define E_UNKNOWN_SYSTEMCALL     0x0036          /* [ERRNO(ENOSYS)] Attempted to execute an unknown system call. */
#define E_NO_SUCH_OBJECT         E_NO_DATA       /* [ERRNO(ENODATA)] The named / indexed object could not be found. */
#define E_ILLEGAL_OPERATION      0x0007          /* [ERRNO(EPERM)] Illegal operation. */
#define E_FILESYSTEM_ERROR       0x0080          /* [ERRNO(...)] Filesystem error (see below) */
#define E_NET_ERROR              0x0081          /* [ERRNO(...)] Network error (see below) */
#define E_IOERROR                0x0082          /* [ERRNO(EIO)] Hardware error, or miss-behaving/miss-configured device. */
#define E_NOT_EXECUTABLE         0x0083          /* [ERRNO(ENOEXEC)] The named file was not recognized as a valid executable. */
#define E_BUFFER_TOO_SMALL       0x0084          /* [ERRNO(ERANGE)] The provided buffer is too small. */
#define E_UNICODE_ERROR          0x0085          /* [ERRNO(EILSEQ)] An illegal sequence was encountered in a unicode string. */

/* Signal codes. */
#define E_INTERRUPT              0xf000          /* [ERRNO(EINTR)]
                                                  * The thread has been interrupted by a PRC function,
                                                  * causing a premature return to user-space.
                                                  * NOTE: If the system communication facility that was used for the interrupt
                                                  *       supports restarting (e.g. loadcore() following a #PF on X86), or if the
                                                  *       interrupt happened during a restartable system call (e.g. `sys_close()'),
                                                  *       and either wasn't caused by a posix_signal, or that posix_signal had
                                                  *       the `SA_RESTART' flag set, this exception isn't propagated into user-space,
                                                  *       but rather causes the underlying interrupt to be restarted.
                                                  * KOS Implements 3 distinct system call interrupt-restart behaviors:
                                                  *  #1: The system call is always restarted (this behavior is also used when
                                                  *      some other type of interrupt handler is interrupted, such as loadcore()
                                                  *      during an ALOA operation, or when loading a file into memory)
                                                  *      This mainly includes system calls where interrupts are undesired,
                                                  *      or would otherwise be unexpected, most notably ~cleanup~ system calls,
                                                  *      as also mentioned by the documentation of `_EXCEPT_API' in `<features.h>'
                                                  *     (e.g. `close()' or `munmap()')
                                                  *  #2: The system call is never restarted (required for some system calls
                                                  *      that are meant to wait for events internally delivered by an interrupt
                                                  *      mechanism, such as `sigtimedwait()'; the behavior here mirrors what is
                                                  *      also done by linux, as described on this page:
                                                  *     `http://man7.org/linux/man-pages/man7/signal.7.html')
                                                  *  #3: Restart the system call if the interrupt wasn't caused by a posix_signal,
                                                  *      or if it was caused by a posix_signal with the `SA_RESTART' flag set. */

/* RTL / Special exception codes. */
#define E_EXIT_THREAD            0xfe00          /* The thread is supposed to terminate. */
#define E_EXIT_PROCESS           0xfe01          /* The entire process is supposed to terminate. */
#define E_NONCONTINUABLE         0xfefe          /* Attempted to resume a non-continuable exception. */
#define E_UNHANDLED_INTERRUPT    0xfeff          /* [ERRNO(EFAULT)] Unhandled system interrupt. */



#ifdef __KERNEL__
/* Kernel-specific exception codes. */
#define E_DRIVER_CLOSED          0x0800          /* Attempted to register a new global hook using a closed driver. */
#define E_USER_RESUME            0xf800          /* Resume execution in user-space (weakened form of `E_INTERRUPT') */
#define E_RETRY_RWLOCK           0xf810          /* The thread should re-attempt to acquire an R/W-lock. */
#endif


/* Error flags. */
#define ERR_FNORMAL      0x0000 /* Normal exception context flags. */
#define ERR_FRESUMABLE   0x0001 /* Execution can be continued after the error. */
#define ERR_FRESUMEFUNC  0x0002 /* Special handling must be done to continue from this exception.
                                 * This flag is set when an error is thrown using
                                 * `error_throw_current()', or `error_throw_resumable()'. */
#define ERR_FRESUMENEXT  0x0004 /* The saved instruction pointer is directed at the start of the faulting instruction. */
#define ERR_FUSERMASK    0xff00 /* Mask of user-defined error flags. */



#ifdef __CC__
struct __ATTR_PACKED exception_data_noncontinuable {
    except_t            nc_origcode;  /* The original code of the error that were thrown. (One of `E_*') */
    __UINT16_TYPE__     nc_origflag;  /* The original exception flags. (Set of `ERR_F*') */
#if __SIZEOF_POINTER__ > 4
    __UINT16_TYPE__   __nc_pad[(__SIZEOF_POINTER__-4)/2];
#endif
    __ULONGPTR_TYPE__   nc_origip;    /* The instruction pointer of the original exception. */
};
#endif


#define ERROR_BADALLOC_VIRTMEMORY 0x0000 /* Virtual memory resource. */
#define ERROR_BADALLOC_PHYSMEMORY 0x0001 /* Physical memory resource. */
#define ERROR_BADALLOC_DEVICEID   0x0002 /* Dynamically allocated device IDs (e.g. `pty' ids). */
#define ERROR_BADALLOC_IOPORT     0x0003 /* Dynamically allocated I/O port IDs. */
#define ERROR_BADALLOC_HANDLE     0x0004 /* Too many handles (`ba_amount' total number of handle IDs that would have had to be allocated; not the number of new handles!). */
#ifdef __CC__
struct __ATTR_PACKED exception_data_badalloc {
    __UINT16_TYPE__      ba_resource;  /* The type of that was attempted to be allocated (One of `ERROR_BADALLOC_*'). */
    __UINT16_TYPE__      ba_pad[(sizeof(void *)-2)/2]; /* ... */
    __SIZE_TYPE__        ba_amount;    /* The amount (e.g.: bytes for memory) that could not be allocated. */
};
#endif /* __CC__ */

#define ERROR_INVALID_HANDLE_FUNDEFINED     0x0000 /* No object has been assigned to the handle.
                                                    * In this case, `h_istype' and `h_rqtype' are set to `HANDLE_TYPE_FNONE',
                                                    * though may in rare cases be set to some something else (don't rely on this).
                                                    * The `h_illhnd' field is filled with additional information on why the handle is undefined. */
#define ERROR_INVALID_HANDLE_FWRONGTYPE     0x0001 /* The associated object's type cannot be used.
                                                    * In this case, `h_istype' and `h_rqtype' are filled
                                                    * with the actual, as well as the required type.
                                                    * Note however that some types can implicitly be cast to other types. */
#define ERROR_INVALID_HANDLE_FWRONGKIND     0x0002 /* The object may have the correct typing, but is not the correct type of object.
                                                    * Mainly used by ioctl() commands invoked on objects of an improper type.
                                                    * Note however that within the kernel, `E_NOT_IMPLEMENTED' is used to signify
                                                    * an ioctl() command not be supported. That error is then only translated to this
                                                    * before being propagated to user-space.
                                                    * In this case, `h_istype' and `h_rqtype' match each other, and `h_rqkind'
                                                    * is filled with the kind of handle that was expected. */
#define ERROR_INVALID_HANDLE_ILLHND_FUNSET  0x0000 /* The handle isn't assigned an object. */
#define ERROR_INVALID_HANDLE_ILLHND_FNOSYM  0x0001 /* The handle refers to a non-existent symbolic handle location. */
#define ERROR_INVALID_HANDLE_ILLHND_FRMSYM  0x0002 /* The handle refers to an unbound symbolic handle location (e.g. an unmounted DOS DRIVE). */
#define ERROR_INVALID_HANDLE_ILLHND_FROSYM  0x0003 /* The handle refers to a read-only symbolic handle location. (may be thrown by dup2() with a read-only symbolic DFD) */
#define ERROR_INVALID_HANDLE_ILLHND_FBOUND  0x0004 /* The handle ID exceeds the limit that is imposed by the kernel on the greatest allowed handle number (may be thrown by dup2()). */
#ifdef __CC__
struct __ATTR_PACKED exception_data_invalid_handle {
    int                  h_handle;     /* The file descriptor number that is invalid. */
    __UINT16_TYPE__      h_reason;     /* The reason for the error (one of `ERROR_INVALID_HANDLE_F*') */
    __UINT16_TYPE__      h_istype;     /* The type of handle that was found (One of `HANDLE_TYPE_F*') */
    __UINT16_TYPE__      h_rqtype;     /* The type of handle that was expected (One of `HANDLE_TYPE_F*') */
    union __ATTR_PACKED {
        __UINT16_TYPE__  h_illhnd;     /* Extended information on the reasoning behind `ERROR_INVALID_HANDLE_FUNDEFINED'. (One of `ERROR_INVALID_HANDLE_ILLHND_F*') */
        __UINT16_TYPE__  h_rqkind;     /* The kind of a handle that was expected (for `ERROR_INVALID_HANDLE_FWRONGKIND'; one of `HANDLE_KIND_F*') */
    };
};
#endif /* __CC__ */

#ifndef ERROR_DIVIDE_BY_ZERO_INT
#define ERROR_DIVIDE_BY_ZERO_INT     0x0000 /* Signed integer division by ZERO */
#define ERROR_DIVIDE_BY_ZERO_UINT    0x0001 /* Unsigned integer division by ZERO */
#define ERROR_DIVIDE_BY_ZERO_FFLT    0x1000 /* BIT: When set, floating point operand. */
#define ERROR_DIVIDE_BY_ZERO_FLT     0x1000 /* Floating point division by ZERO (`float') */
#define ERROR_DIVIDE_BY_ZERO_DBL     0x1001 /* Floating point division by ZERO (`double') */
#define ERROR_DIVIDE_BY_ZERO_LDBL    0x1002 /* Floating point division by ZERO (`long double') */
#define ERROR_DIVIDE_BY_ZERO_FNORMAL 0x0000 /* Normal flags. */
#endif /* !ERROR_DIVIDE_BY_ZERO_INT */

#ifdef __CC__
struct __ATTR_PACKED exception_data_divide_by_zero {
    __UINT16_TYPE__      dz_type;      /* The type of division (One of `ERROR_DIVIDE_BY_ZERO_*') */
    __UINT16_TYPE__      dz_flag;      /* Division flags (Set of `ERROR_DIVIDE_BY_ZERO_F*') */
#if __SIZEOF_POINTER__ > 4
     __UINT16_TYPE__   __dz_pad[(__SIZEOF_POINTER__-4)/2];
#endif
     union __ATTR_PACKED {
         __INT64_TYPE__  da_int;       /* [ERROR_DIVIDE_BY_ZERO_INT] Integer operand. */
         __UINT64_TYPE__ da_uint;      /* [ERROR_DIVIDE_BY_ZERO_UINT] Unsigned integer operand. */
         float           da_flt;       /* [ERROR_DIVIDE_BY_ZERO_FLT] Floating point operand. */
         double          da_dbl;       /* [ERROR_DIVIDE_BY_ZERO_DBL] Double precision floating point operand. */
         long double     da_ldbl;      /* [ERROR_DIVIDE_BY_ZERO_LDBL] Long-double precision floating point operand. */
     }                   dz_arg;       /* The operand that the division was attempted to divide. */
};
struct __ATTR_PACKED exception_data_index_error {
    __UINTPTR_TYPE__   __b_pad;         /* ... */
    __UINT64_TYPE__      b_index;       /* The index that was tested, or ZERO(0). */
    __UINT64_TYPE__      b_boundmin;    /* The lower bound, or ZERO(0). */
    __UINT64_TYPE__      b_boundmax;    /* The upper bound, or ZERO(0). */
};
struct __ATTR_PACKED exception_data_buffer_too_small {
    __SIZE_TYPE__        bs_bufsize;    /* Provided buffer size. */
    __SIZE_TYPE__        bs_reqsize;    /* Required buffer size. */
};
#endif /* __CC__ */


#define ERROR_FS_NOERROR               0x0000 /* No error. */
#define ERROR_FS_FILE_NOT_FOUND        0x0001 /* [ERRNO(ENOENT)]       File could not be found (when `foo' does, but `bar' doesn't exist in `/foo/bar').
                                               *                       Also thrown when attempting to operate on a file that has been deleted. */
#define ERROR_FS_PATH_NOT_FOUND        0x0002 /* [ERRNO(ENOENT)]       Directory could not be found (when `foo' doesn't exist in `/foo/bar'). */
#define ERROR_FS_NOT_A_DIRECTORY       0x0003 /* [ERRNO(ENOTDIR)]      Expected a directory when encountering a file while traversing a path. */
#define ERROR_FS_TOO_MANY_LINKS        0x0004 /* [ERRNO(ELOOP)]        Too many symbolic links encountered while evaluating a path. */
#define ERROR_FS_IS_NOT_FOUND(x)     ((x) <= ERROR_FS_TOO_MANY_LINKS) /* A file, directory, or path doesn't exists */
#define ERROR_FS_DIRECTORY_NOT_EMPTY   0x0005 /* [ERRNO(ENOTEMPTY)]    Cannot remove a directory that isn't empty. */
#define ERROR_FS_ILLEGAL_PATH          0x0006 /* [ERRNO(EINVAL)]       A portion of the path contains characters that are not supported by the hosting filesystem.
                                               *                      (Only thrown when creating files; during lookup `ERROR_FS_FILE_NOT_FOUND' is returned instead). */
#define ERROR_FS_CROSSDEVICE_LINK      0x0007 /* [ERRNO(EXDEV)]        The source and destination of a `rename()' or `link()' operation do not target the same device. */
#define ERROR_FS_FILENAME_TOO_LONG     0x0008 /* [ERRNO(ENAMETOOLONG)] A single path segment is longer than the max supported length (which is `65535' characters) */
#define ERROR_FS_FILE_ALREADY_EXISTS   0x0009 /* [ERRNO(EEXIST)]       The target of a `rename()' or `link()' operation already exists, or `O_EXCL' was passed to `open()', and the file already exists. */
#define ERROR_FS_UNSUPPORTED_FUNCTION  0x000a /* [ERRNO(EPERM)]        Some function required for the current operation is not supported by the filesystem (e.g.: hardlinks on FAT). */
#define ERROR_FS_READONLY_FILESYSTEM   0x000b /* [ERRNO(EROFS)]        The filesystem has been mounted as read-only, or associated block-device cannot be written to during some
                                               *                       operation that either requires (e.g. `mkdir()') or requests (e.g.: `open(...,O_WRONLY)') write permissions
                                               *                       In some cases, this error may also be thrown instead of `ERROR_FS_UNSUPPORTED_FUNCTION'. */
#define ERROR_FS_ACCESS_ERROR          0x000e /* [ERRNO(EACCES)]       The calling process does not have sufficient permissions for the operation. */
#define ERROR_FS_DISK_FULL             0x000f /* [ERRNO(ENOSPC)]       The associated disk is full or insufficient space is available to complete the operation. */
#define ERROR_FS_RMDIR_REGULAR         0x0010 /* [ERRNO(ENOTDIR)]     `rmdir()' cannot be used to remove a non-directory file. */
#define ERROR_FS_UNLINK_DIRECTORY      0x0011 /* [ERRNO(EISDIR)]       Cannot unlink() a directory. */
#define ERROR_FS_REMOVE_MOUNTPOINT     0x0012 /* [ERRNO(EBUSY)]       `rmdir()' or `unlink()' cannot be used to remove a mounting point (Virtual INode). */
#define ERROR_FS_UNMOUNT_NOTAMOUNT     0x0013 /* [ERRNO(EINVAL)]      `umount()' cannot be used to remove something that isn't a mounting point. */
#define ERROR_FS_RENAME_NOTAMOUNT      0x0014 /* [ERRNO(EBUSY)]       `rename()' cannot be used to perform a re-mount operation. */
#define ERROR_FS_NEGATIVE_SEEK         0x0015 /* [ERRNO(ESPIPE)]       Cannot seek() to a negative file position. */
#define ERROR_FS_FILE_TOO_LARGE        0x0016 /* [ERRNO(EFBIG)]        A file or device is too large to be representable in the associated operation. */
#define ERROR_FS_TOO_MANY_HARD_LINKS   0x0017 /* [ERRNO(EMLINK)]       Cannot create another new hardlink. The hard link counter has already reached
                                               *                       its limit (Highly unlikely, considering a common limit of at least 2^16). */
#define ERROR_FS_OBJECT_IS_BUSY        0x0018 /* [ERRNO(EBUSY)]        The named filesystem component cannot be operated upon before some sort of persistent lock is removed. */
#define ERROR_FS_NOT_A_SYMLINK         0x0019 /* [ERRNO(ENOENT)]      `O_SYMLINK' was used with `O_EXCL', but the named file isn't a symbolic link. */
#define ERROR_FS_IS_A_SYMLINK          0x001a /* [ERRNO(ELOOP)]       `O_NOFOLLOW' was used, but the named file is a symbolic link. */
#define ERROR_FS_CORRUPTED_FILESYSTEM  0xffff /* [ERRNO(EIO)]          Corrupted, miss-configured, or otherwise not compatible filesystem. */
#ifdef __CC__
struct __ATTR_PACKED exception_data_filesystem_error {
    __UINT16_TYPE__      fs_errcode;  /* Filesystem operation error code (One of `ERROR_FS_*').
                                       * NOTE: Never `ERROR_FS_NOERROR' */
};
#endif /* __CC__ */


#define ERROR_NET_NOERROR                0x0000 /* No error. */
#define ERROR_NET_UNSUPPORTED_DOMAIN     0x0001 /* [ERRNO(EAFNOSUPPORT)]    The specified socket domain (address family) is not supported (missing driver?) */
#define ERROR_NET_UNSUPPORTED_TYPE       0x0002 /* [ERRNO(EINVAL)]          The specified socket type is not supported by the socket domain. */
#define ERROR_NET_UNSUPPORTED_PROTOCOL   0x0003 /* [ERRNO(EPROTONOSUPPORT)] The specified socket protocol is not supported by the socket domain. */
#define ERROR_NET_SHUTDOWN               0x0004 /* [ERRNO(ESHUTDOWN)]       The socket has been `shutdown(2)' */
#define ERROR_NET_CANNOT_RECONNECT       0x0005 /* [ERRNO(EALREADY)]        A connection-oriented socket cannot be reconnected. */
#define ERROR_NET_CANNOT_REBIND          0x0006 /* [ERRNO(EALREADY)]        A connection-oriented socket cannot be rebound. */
#define ERROR_NET_NOT_BOUND              0x0007 /* [ERRNO(EOPNOTSUPP)]      Cannot `getsockname(2)', `listen(2)', `accept(2)', `sendto(2)', `read(2)', `recv(2)' or `recvfrom(2)' using an unbound socket. */
#define ERROR_NET_NOT_CONNECTED          0x0008 /* [ERRNO(ENOTCONN)]        Cannot `getpeername(2)', `write(2)' or `send(2)' using an unconnected socket. */
#define ERROR_NET_NOT_LISTENING          0x0009 /* [ERRNO(EINVAL)]          Cannot `accept(2)' using a socket that isn't listening. */
#define ERROR_NET_ALREADY_LISTENING      0x000a /* [ERRNO(EALREADY)]        The socket has already started listening. */
#define ERROR_NET_CANNOT_LISTEN          0x000b /* [ERRNO(EOPNOTSUPP)]      The socket doesn't implement the `listen(2)' function. */
#define ERROR_NET_CANNOT_ACCEPT          0x000c /* [ERRNO(EOPNOTSUPP)]      The socket doesn't implement the `accept(2)' function. */
#define ERROR_NET_INVALID_SOCKET_ADDRESS 0x000d /* [ERRNO(EINVAL)]          The socket address passed to `connect()', `bind()', or `sendto()' is malformed (usually thrown if it's too small). */
#define ERROR_NET_INVALID_ADDRESS_FAMILY 0x000e /* [ERRNO(EAFNOSUPPORT)]    The address family specified in a call to `connect()', `bind()', or `sendto()' isn't implemented by the socket. */
#define ERROR_NET_CONNECTION_REFUSED     0x000f /* [ERRNO(ECONNREFUSED)]    Failed to connect to an address with no one listen(2)-ing on the other end. */
#define ERROR_NET_ADDRESS_IN_USE         0x0010 /* [ERRNO(EADDRINUSE)]      Local address is already in use (mainly by `bind()'). */
#define ERROR_NET_PACKET_TOO_LARGE       0x0011 /* [ERRNO(EMSGSIZE)]        A packet is too large for the associated protocol. */
#ifdef __CC__
struct __ATTR_PACKED exception_data_net_error {
    __UINT16_TYPE__      n_errcode;   /* Network operation error code (One of `ERROR_NET_*').
                                       * NOTE: Never `ERROR_NET_NOERROR' */
};
#endif /* __CC__ */

#define ERROR_NO_DEVICE_FBLOCKDEV 0x0000 /* The missing device is a block-device. */
#define ERROR_NO_DEVICE_FCHARDEV  0x0001 /* The missing device is a character-device. */
#ifdef __CC__
struct __ATTR_PACKED exception_data_no_device {
    __UINT16_TYPE__      d_type;      /* The type of device that is missing (One of `ERROR_NO_DEVICE_F*'). */
    __UINT16_TYPE__    __d_pad[(sizeof(void *)-2)/2]; /* ... */
    __dev_t              d_devno;     /* The missing device number. */
};

struct __ATTR_PACKED exception_data_unhandled_interrupt {
    __UINTPTR_TYPE__     ui_errcode;   /* An exception code passed alongside the interrupt, or ZERO. */
    __UINT8_TYPE__       ui_intcode;   /* The interrupt vector that has not been handled. */
    __UINT8_TYPE__     __ui_pad[sizeof(void *)-1]; /* ... */
};
#ifndef __ARCH_SYSCALL_MAX_ARGC
#define __ARCH_SYSCALL_MAX_ARGC 6
#endif
struct __ATTR_PACKED exception_data_unknown_systemcall {
    __syscall_ulong_t    us_sysno;                         /* System call number. */
    __syscall_ulong_t    us_args[__ARCH_SYSCALL_MAX_ARGC]; /* System call arguments. */
};
struct __ATTR_PACKED exception_data_exit {
#ifdef __KERNEL__
    int                  e_status;     /* Wait status (s.a. `union wait' from `<bits/waitstatus.h>') */
#else
    int                  e_status;     /* Thread/process exit code (as passed to `pthread_exit()' / `exit()') */
#endif
};

#ifdef __KERNEL__
struct __ATTR_PACKED exception_data_retry_rwlock {
    void                *e_rwlock_ptr; /* Address of the R/W-lock in question. */
};
#endif

#endif /* __CC__ */


/* The number of extended exception information data pointers. */
#define __EXCEPTION_INFO_NUM_DATA_POINTERS  15
#define __EXCEPTION_INFO_SIZEOF_DATA       (__EXCEPTION_INFO_NUM_DATA_POINTERS*__SIZEOF_POINTER__)

#ifdef __CC__
struct exception_data {
    except_t                     e_code;       /* The code of the error that was thrown. (One of `E_*') */
    __UINT16_TYPE__              e_flag;       /* Exception flags. (Set of `ERR_F*') */
#if __SIZEOF_POINTER__ > 4
    /* Pad to align exception-specific data to a full pointer */
    __UINT16_TYPE__            __e_pad[(__SIZEOF_POINTER__-4)/2];
#endif
    union __ATTR_PACKED {
        /* Exception-specific data. */
        void            *e_pointers[__EXCEPTION_INFO_NUM_DATA_POINTERS]; /* Exception data as a bunch of untyped pointers. */
        __UINTPTR_TYPE__ e_words[__EXCEPTION_INFO_NUM_DATA_POINTERS];    /* Exception data as a bunch of untyped data words. */
        struct exception_data_noncontinuable      e_noncont;             /* E_NONCONTINUABLE */
        struct exception_data_badalloc            e_badalloc;            /* E_BADALLOC */
        struct exception_data_invalid_handle      e_invalid_handle;      /* E_INVALID_HANDLE */
        struct exception_data_segfault            e_segfault;            /* E_SEGFAULT, E_STACK_OVERFLOW */
        struct exception_data_divide_by_zero      e_divide_by_zero;      /* E_DIVIDE_BY_ZERO */
        struct exception_data_index_error         e_index_error;         /* E_INDEX_ERROR */
        struct exception_data_buffer_too_small    e_buffer_too_small;    /* E_BUFFER_TOO_SMALL */
        struct exception_data_filesystem_error    e_filesystem_error;    /* E_FILESYSTEM_ERROR */
        struct exception_data_net_error           e_net_error;           /* E_NET_ERROR */
        struct exception_data_no_device           e_no_device;           /* E_NO_DEVICE */
        struct exception_data_unhandled_interrupt e_unhandled_interrupt; /* E_UNHANDLED_INTERRUPT. */
        struct exception_data_unknown_systemcall  e_unknown_systemcall;  /* E_UNKNOWN_SYSTEMCALL. */
        struct exception_data_exit                e_exit;                /* E_EXIT_THREAD, E_EXIT_PROCESS */
#ifdef __exception_data_system_defined
        struct exception_data_system              e_system;              /* Common header for system exceptions. */
#endif
#ifdef E_ILLEGAL_INSTRUCTION
        struct exception_data_illegal_instruction e_illegal_instruction; /* E_ILLEGAL_INSTRUCTION. */
#endif
#ifdef E_INVALID_SEGMENT
        struct exception_data_invalid_segment     e_invalid_segment;     /* E_INVALID_SEGMENT. */
#endif
#ifdef __KERNEL__
        struct exception_data_retry_rwlock        e_retry_rwlock;        /* E_RETRY_RWLOCK */
#endif
    };
};
#endif /* __CC__ */


#define __EXCEPTION_INFO_OFFSETOF_CODE      0
#define __EXCEPTION_INFO_OFFSETOF_FLAG      2
#define __EXCEPTION_INFO_OFFSETOF_DATA      __SIZEOF_POINTER__
#ifdef __EXCEPTION_RT_DATA_SIZE
#define __EXCEPTION_INFO_OFFSETOF_RTDATA   (__SIZEOF_POINTER__+__EXCEPTION_INFO_SIZEOF_DATA)
#define __EXCEPTION_INFO_OFFSETOF_CONTEXT  (__SIZEOF_POINTER__+__EXCEPTION_INFO_SIZEOF_DATA+__EXCEPTION_RT_DATA_SIZE)
#else
#define __EXCEPTION_INFO_OFFSETOF_CONTEXT  (__SIZEOF_POINTER__+__EXCEPTION_INFO_SIZEOF_DATA)
#endif
#define __EXCEPTION_INFO_SIZE              (__EXCEPTION_INFO_OFFSETOF_CONTEXT+__CPU_CONTEXT_SIZE)
#ifdef __x86_64__
#define __USEREXCEPTION_INFO_SIZE          (__EXCEPTION_INFO_OFFSETOF_CONTEXT+__CPU_CONTEXT_SIZE)
#elif defined(__KERNEL__)
#define __USEREXCEPTION_INFO_SIZE          (__EXCEPTION_INFO_OFFSETOF_CONTEXT+X86_USERCONTEXT32_SIZE)
#else
#define __USEREXCEPTION_INFO_SIZE          (__EXCEPTION_INFO_OFFSETOF_CONTEXT+X86_CONTEXT32_SIZE)
#endif

#if defined(__KERNEL__) || defined(__USE_KOS)
#define EXCEPTION_INFO_NUM_DATA_POINTERS    __EXCEPTION_INFO_NUM_DATA_POINTERS
#define EXCEPTION_INFO_SIZEOF_DATA          __EXCEPTION_INFO_SIZEOF_DATA
#define EXCEPTION_INFO_OFFSETOF_CODE        __EXCEPTION_INFO_OFFSETOF_CODE
#define EXCEPTION_INFO_OFFSETOF_FLAG        __EXCEPTION_INFO_OFFSETOF_FLAG
#define EXCEPTION_INFO_OFFSETOF_DATA        __EXCEPTION_INFO_OFFSETOF_DATA
#ifdef __EXCEPTION_INFO_OFFSETOF_RTDATA
#define EXCEPTION_INFO_OFFSETOF_RTDATA      __EXCEPTION_INFO_OFFSETOF_RTDATA
#endif
#define EXCEPTION_INFO_OFFSETOF_CONTEXT     __EXCEPTION_INFO_OFFSETOF_CONTEXT
#define EXCEPTION_INFO_SIZE                 __EXCEPTION_INFO_SIZE
#endif

#ifdef __CC__
struct __ATTR_PACKED exception_info {
    /* The exception-information data structure. */
    struct exception_data         e_error;      /* Error information. */
#ifdef __EXCEPTION_RT_DATA_SIZE
    struct exception_rt_data      e_rtdata;     /* Exception runtime data. */
#endif
    struct cpu_context            e_context;    /* The CPU context at the time of the interrupt happening.
                                                 * The instruction pointer is either directed at the start of
                                                 * the faulting instruction, or at the following instruction,
                                                 * depending on the `ERR_FRESUMENEXT' flag. */
};
#ifdef __KERNEL__
struct __ATTR_PACKED user_exception_info {
    /* The exception-information data structure (for user-space). */
    struct exception_data         e_error;      /* Error information. */
#ifdef __USER_EXCEPTION_RT_DATA_SIZE
    struct user_exception_rt_data e_rtdata;     /* Exception runtime data. */
#elif defined(__EXCEPTION_RT_DATA_SIZE)
    struct exception_rt_data      e_rtdata;     /* Exception runtime data. */
#endif
    struct x86_usercontext        e_context;    /* The CPU context at the time of the interrupt happening.
                                                 * The instruction pointer is either directed at the start of
                                                 * the faulting instruction, or at the following instruction,
                                                 * depending on the `ERR_FRESUMENEXT' flag. */
};
#endif /* __KERNEL__ */
#endif /* __CC__ */


/* Exception descriptor types. */
#define EXCEPT_DESC_TYPE_BYPASS         0x0000 /* The handler bypasses regular exception handling
                                                * in that the guarded function will be fully unwound
                                                * before certain aspects are saved/restored according
                                                * to descriptor flags, followed by a jump to the handler. */

/* Exception descriptor flags. */
#define EXCEPT_DESC_FNORMAL             0x0000 /* Normal descriptor flags. */
#define EXCEPT_DESC_FDEALLOC_CONTINUE   0x0001 /* Deallocate continue-information and set the stack-pointer to the value that the
                                                * caller of the function in which the guarded area is located inside of is expecting.
                                                * When this flag is set, `error_continue()' can no longer be used safely, unless the
                                                * exception descriptor takes it upon itself to undo the updated stack-pointer.
                                                * If this type of handler returns normally, the faulting function returns with the
                                                * value that the bypass handler returns.
                                                * Also note that when this flag is set, rather than being appended at the end,
                                                * the descriptor handler function will override the faulting function. */
#define EXCEPT_DESC_FRELATIVE           0x4000 /* Descriptor pointers are relative to the associated application's load address */
#ifdef __KERNEL__
#define EXCEPT_DESC_FDISABLE_PREEMPTION 0x8000 /* Disable preemption before resuming execution. */
#define EXCEPT_DESC_FMASK               0xc001 /* Mask of known descriptor flags. */
#else
#define EXCEPT_DESC_FMASK               0x4001 /* Mask of known descriptor flags. */
#endif

#ifdef __CC__
struct except_desc {
    /* Extended exception descriptor. */
    void            *ed_handler; /* [1..1] Exception descriptor entry address. */
    __UINT16_TYPE__  ed_type;    /* The type of descriptor (One of `EXCEPT_DESC_TYPE_*'). */
    __UINT16_TYPE__  ed_flags;   /* Descriptor flags (Set of `EXCEPT_DESC_F*'). */
    __UINT16_TYPE__  ed_safe;    /* The amount of bytes of stack-memory that should be
                                  * reserved before the descriptor is invoked.
                                  * This usually equals the total size of arguments passed to the
                                  * function in which code is being protected (when it's STDCALL),
                                  * plus an additional 4/8 bytes for the return address.
                                  * Unless the `EXCEPT_DESC_FDEALLOC_CONTINUE' flag is set,
                                  * this is the amount of stack-memory that is copied before
                                  * jumping to `ed_handler', and when `EXCEPT_DESC_FDEALLOC_CONTINUE'
                                  * isn't set, this is the offset subtracted from SP to re-reserve
                                  * this amount of memory and prevent it from being clobbered.
                                  * In either case, ESP/RSP will point to `CFA(FUNCTION_OF(:eh_begin)) - ed_safe'
                                  * before more additional information is optionally pushed onto the stack,
                                  * based on flags defined above (XXX: No such flags exist, yet).
                                  * This value should never be smaller than 4/8 because it must
                                  * include the exception handler return address. */
    __UINT16_TYPE__ __ed_pad;    /* ... */
};
#endif /* __CC__ */

/* Exception handler flags. */
#define EXCEPTION_HANDLER_FNORMAL      0x0000  /* Normal handler flags. */
#define EXCEPTION_HANDLER_FHASMASK     0x0001  /* FLAG: `eh_mask' contains a valid value.
                                                *        When no other mask-related flags are set, `eh_mask' is
                                                *        the exception code exclusively caught by the handler.
                                                *        Otherwise, all exceptions can be caught. */
#define EXCEPTION_HANDLER_FUSERFLAGS   0x0002  /* FLAG:  Before jumping to the handler, do the following:
                                                * >> error_info()->e_error.e_flag |= HANDLER->eh_mask & ERR_FUSERMASK. */
#define EXCEPTION_HANDLER_FFINALLY     0x0004  /* FLAG:  This is a finally-handler. */
#define EXCEPTION_HANDLER_FRELATIVE    0x4000  /* Handler pointers are relative to the associated application's load address */
#define EXCEPTION_HANDLER_FDESCRIPTOR  0x8000  /* FLAG:  The handling of this exception is done using an exception descriptor. */
#define EXCEPTION_HANDLER_FMASK        0xc007  /* Mask of known flags. */

#ifdef __CC__
struct __ATTR_PACKED except_handler {
    /* The internal structure found in `.except'
     * These descriptors are iterated in reverse order, meaning that
     * an entry defined after another is considered after the first.
     * WARNING: Modifications to .except after an application has
     *          already been started can lead to undefined behavior.
     *          The kernel keeps an internal cache of exception handlers
     *          in order to speed up lookup time of exception handlers.
     *          However this also means that changes made to an exception
     *          handler after the application has already started might
     *          not become visible, even if they had never been used
     *          by the application.
     *          This is because the cache of known exception handlers
     *          kept by the kernel is shared between all instances of
     *          an application, and may even be kept in memory for a
     *          while when no instance of the application is actually
     *          running.
     *       -> So long story short: Don't bother writing self-modifying
     *          code that will change it's own set of exception handlers,
     *          because those changes will either not become visible, or
     *          they might become visible to other instances of your app
     *          that hadn't even modified their .except section, yet. */
    void                        *eh_begin;     /* [<= eh_end] Inclusive start address of the exception handling range */
    void                        *eh_end;       /* [>= eh_begin] Non-inclusive end address of the exception handling range */
    union __ATTR_PACKED {
        struct except_desc const*eh_descr;     /* [1..1][valid_if(EXCEPTION_HANDLER_FDESCRIPTOR)] Exception descriptor. */
        __UINTPTR_TYPE__         eh_entry;     /* Handler entry point address (Must be part of the same eh_frame as `eh_begin...eh_end') */
    };
    __UINTPTR_HALF_TYPE__        eh_flag;      /* Handler flags (Set of `EXCEPTION_HANDLER_F*') */
    __UINTPTR_HALF_TYPE__        eh_mask;      /* Handler mask (When `EXCEPTION_HANDLER_FHASMASK' is set) */
};
#endif /* __CC__ */

__DECL_END


#endif /* !_KOS_EXCEPT_H */
