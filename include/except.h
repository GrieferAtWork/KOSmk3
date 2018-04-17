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
#ifndef _EXCEPT_H
#define _EXCEPT_H 1

#include "__stdinc.h"
#include <hybrid/typecore.h>
#include <kos/except.h>

#ifndef __CRT_KOS
#error "C exception handlers are specific to KOS"
#endif

__SYSDECL_BEGIN

/* Disposition values for the `mode' argument of `EXCEPT()' */
#define EXCEPT_CONTINUE_SEARCH      0  /* Continue unwinding frames. (Will _always_ be 0) */
#define EXCEPT_EXECUTE_HANDLER      1  /* Execute this handler. (Will _always_ be > 0) */
#define EXCEPT_CONTINUE_RETRY     (-1) /* Restore the CPU state at the time of the exception
                                        * happening and re-execute the faulting instruction. */
#define EXCEPT_CONTINUE_IGNORE    (-2) /* Restore the CPU state at the time of the exception
                                        * happening, then skip the faulting instruction.
                                        * If the faulting instruction cannot be understood,
                                        * an `E_NONCONTINUABLE' is thrown. */


/* Helper functions for implementing exception handling. */
#ifdef __CC__

/* Throw an error `code' at the current code location.
 * Any additional exception information is ZERO-initialized.
 * @return: true:  The error-condition should be re-checked after `error_continue(true)'
 * @return: false: Ignore the error and continue normally after `error_continue(false)' */
__LIBC __BOOL (__FCALL error_throw_resumable)(except_t __code);
__LIBC __ATTR_NORETURN void (__FCALL error_throw)(except_t __code);

/* Same as the functions above, but construct the error using format args.
 * Varargs are interpreted dependent on `CODE':
 *   (except_t code = E_BADALLOC, unsigned int ba_resource, size_t ba_amount);
 *   (except_t code = E_INVALID_HANDLE, int h_handle, int h_reason = ERROR_INVALID_HANDLE_FUNDEFINED, int h_illhnd);
 *   (except_t code = E_INVALID_HANDLE, int h_handle, int h_reason = ERROR_INVALID_HANDLE_FWRONGTYPE, int h_istype, int h_rqtype);
 *   (except_t code = E_INVALID_HANDLE, int h_handle, int h_reason = ERROR_INVALID_HANDLE_FWRONGKIND, int h_isrqtype, int h_rqkind);
 *   (except_t code = E_SEGFAULT, unsigned int sf_reason, void *sf_vaddr);
 *   (except_t code = E_STACK_OVERFLOW, unsigned int sf_reason, void *sf_vaddr);
 *   (except_t code = E_DIVIDE_BY_ZERO, unsigned int dz_type = ERROR_DIVIDE_BY_ZERO_INT, unsigned int dz_flag, int64_t da_int);
 *   (except_t code = E_DIVIDE_BY_ZERO, unsigned int dz_type = ERROR_DIVIDE_BY_ZERO_UINT, unsigned int dz_flag, uint64_t da_uint);
 * #ifndef __KERNEL__
 *   (except_t code = E_DIVIDE_BY_ZERO, unsigned int dz_type = ERROR_DIVIDE_BY_ZERO_FLT, unsigned int dz_flag, float da_flt);
 *   (except_t code = E_DIVIDE_BY_ZERO, unsigned int dz_type = ERROR_DIVIDE_BY_ZERO_DBL, unsigned int dz_flag, double da_dbl);
 *   (except_t code = E_DIVIDE_BY_ZERO, unsigned int dz_type = ERROR_DIVIDE_BY_ZERO_LDBL, unsigned int dz_flag, long double da_dbl);
 * #endif
 *   (except_t code = E_INDEX_ERROR, uint64_t b_index, uint64_t b_boundmin, uint64_t b_boundmax);
 *   (except_t code = E_BUFFER_TOO_SMALL, size_t bs_bufsize, size_t bs_reqsize);
 *   (except_t code = E_FILESYSTEM_ERROR, unsigned int fs_errcode);
 *   (except_t code = E_NO_DEVICE, unsigned int d_type, dev_t d_devno);
 *   (except_t code = E_UNHANDLED_INTERRUPT, unsigned int ui_intcode, uintptr_t ui_errcode);
 *   (except_t code = E_UNKNOWN_SYSTEMCALL, syscall_ulong_t us_sysno, [syscall_ulong_t args...]);
 *   (except_t code = E_EXIT_THREAD, int e_status);
 *   (except_t code = E_EXIT_PROCESS, int e_status);
 *   (except_t code = E_INVALID_SEGMENT, unsigned int is_register, unsigned int is_segment);
 * All other codes are thrown without additional arguments,
 * meaning the same way the functions above are thrown. */
__LIBC __BOOL (__ATTR_CDECL error_throw_resumablef)(except_t __code, ...);
__LIBC __ATTR_NORETURN void (__ATTR_CDECL error_throwf)(except_t __code, ...);

/* Same as the functions above, but fill in exception data as
 * a direct copy of the passed `exception_data' structure. */
__LIBC __BOOL (__FCALL error_throw_resumable_ex)(except_t __code, struct exception_data const *__restrict __data);
__LIBC __ATTR_NORETURN void (__FCALL error_throw_ex)(except_t __code, struct exception_data const *__restrict __data);

/* Throw the currently set exception exception information.
 * NOTE: When the `ERR_FRESUMABLE' flag isn't set, this function never returns.
 * Unlike `error_throw()', this function does not clear additional
 * exception information (as accessible through `error_info()'), meaning
 * that this function should be used when throwing errors with additional
 * information connected to them.
 * @return: true:  The error-condition should be re-checked after `error_continue(true)'
 * @return: false: Ignore the error and continue normally after `error_continue(false)' */
__LIBC __BOOL (__FCALL error_throw_current)(void);

/* Dump information about the current error to
 * STDERR, alongside a short printf()-style message
 * In the kernel, information is written using `debug_printer'.
 * NOTE: Passing `NULL' for `REASON' will print a standard message as reason instead.
 * NOTE: Passing `NULL' for `FP' is the same as passing `stderr'. */
__LIBC void (__ATTR_CDECL error_printf)(char const *__reason, ...);
__LIBC void (__LIBCCALL error_vprintf)(char const *__reason, __builtin_va_list __args);
#ifndef __KERNEL__
__LIBC void (__ATTR_CDECL error_fprintf)(__FILE *__fp, char const *__reason, ...);
__LIBC void (__LIBCCALL error_vfprintf)(__FILE *__fp, char const *__reason, __builtin_va_list __args);
#endif

/* Rethrow the current exception, but don't set a new error-context.
 * This function simply continues unwinding the
 * stack in search of exception handlers. */
__LIBC __ATTR_NORETURN void (__FCALL error_rethrow)(void);

/* Continue execution
 * If the current exception has the `ERR_FRESUMABLE'
 * flag set, or the faulting instruction isn't
 * understood, an `E_NONCONTINUABLE' is thrown.
 * @param: retry: When non-zero, jump to the faulting instruction or have
 *               `error_throw_resumable()' / `error_throw_current()' return true.
 *                Otherwise, jump to the instruction thereafter or have
 *               `error_throw_resumable()' / `error_throw_current()' return false. */
__LIBC __ATTR_NORETURN void (__FCALL error_continue)(int __retry);

/* Process the exception disposition of an except-handler
 * This function does not return unless `mode > 0'.
 * When equal to `0', it will continue searching for more
 * exception handlers by jumping to run `error_rethrow()'.
 * Otherwise, it will invoke `error_continue(mode == EXCEPT_CONTINUE_RETRY)' */
__LIBC void (__FCALL error_except)(int __mode);

/* Return the current exception code, or a
 * pointer to thread-local exception information. */
__LIBC except_t (__FCALL error_code)(void);
__LIBC __ATTR_CONST __ATTR_RETNONNULL struct exception_info *(__FCALL error_info)(void);

/* Function called when there are no exception handlers left to handle an error.
 * This function is not implemented by the arch-specific exception CRT.
 * NOTE: This function is also invoked when a thread is about to terminate
 *       due to an unhandled exception, thus allowing user-space to deal
 *       with unhandled exceptions. With that in mind, `libc' exports this
 *       symbols weakly, meaning if you want to do your own handling for
 *       unhandled exceptions, you must simply override this symbol. */
__LIBC __ATTR_NORETURN void (__FCALL error_unhandled_exception)(void);


#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */

/* Translate exception information into an ERRNO value. */
__LIBC errno_t (__FCALL exception_errno)(struct exception_info *__restrict __info);

#ifndef __KERNEL__
/* Translate the current exception into an ERRNO and save it as the `errno'
 * value of the calling thread, before returning `EXCEPT_EXECUTE_HANDLER'.
 * This function is intended to be used in EXCEPT() statements to translate
 * exceptions to posix-compatible errno values:
 * >> TRY {
 * >>     something_dangerous();
 * >> } EXCEPT (except_errno()) {
 * >>     // Handle exceptions as errno values.
 * >>     return -1;
 * >> }
 * >> return 0;
 * This functionality is used by libc for implementing wrappers
 * around system calls that require large setup / teardown. */
__LIBC int (__FCALL except_errno)(void);
#endif /* !__KERNEL__ */

/* The internal function used for unwinding the stack,
 * giving `context' describing where should start.
 * WARNING: `context' will be modified as the stack is unwound,
 *           and the initial instruction pointer must be directed
 *           at the start, or into the middle of the instruction which
 *           caused the exception. (Not at the instruction thereafter)
 * This function is invoked by `error_rethrow', using
 * a stack-allocated cpu-context of the caller. */
__LIBC __ATTR_NORETURN void
(__FCALL __error_rethrow_at)(struct cpu_context *__restrict __context,
                             int __ip_is_after_faulting);

/* Return a pointer to the instruction following `ip'
 * If the next instruction cannot be determined, `NULL' is returned.
 * These functions are used to implement `EXCEPT_CONTINUE_RETRY'
 * vs. `EXCEPT_CONTINUE_IGNORE'  */
__LIBC void const *(__FCALL __rtl_next_instruction)(void const *__restrict ip);
__LIBC void const *(__FCALL __rtl_prev_instruction)(void const *__restrict ip);

#endif /* __CC__ */



/* Rules about exception handling that you must conform to.
 *    - FINALLY-blocks are not executed when you jump/return
 *      from a TRY-block. - There is just no way to implement
 *      this without going into the compiler and adding some
 *      new extensions (that would be too complicated).
 *    - LEAVE can be used to jump to the finally-block
 *      or after the EXCEPT-handler.
 *    - Jumping out of a FINALLY-block is illegal and will cause
 *      the error to be discarded as if it was an EXCEPT(1) block.
 *      However, this behavior might change without notice.
 *    - EXCEPT expects an expression that evaluates to an integer.
 *      If that integer is lower than ZERO(0), execution will
 *      continue at the exception site.
 *      If the value equals ZERO(0), execution will continue
 *      unwinding the stack in search for additional exception
 *      handlers.
 *      Otherwise, execution will proceed by running the
 *      exception handler itself.
 *    - In all cases where FINALLY or EXCEPT are being used,
 *      the stack pointer will be the same as it was when the
 *      exception occurred, meaning that any structures allocated
 *      on the stack will continue to exist until the handling
 *      function returns, essentially freeing up all memory
 *      that was reserved for the exception handler.
 *    - When an exception handler is entered, all registers but SP
 *      will have been unwound and contain values that would be
 *      expected during normal code execution.
 *    - Using `alloca()' in FINALLY or EXCEPT blocks is illegal as
 *      they would allocate memory in the stack-frame of the function
 *      where the exception occurred.
 *      However, you are allowed to use `alloca()' in
 *      functions called from except/finally blocks!
 *    - KOS implements a so-called zero-effort exception system,
 *      in that all that's required for exception handling is a
 *      functioning way of unwinding the stack, and an exception
 *      descriptor defined using the `DEFINE_*_HANDLER' macros below.
 *      There is NO linked list of handlers that would otherwise
 *      need to be maintained. */



#ifdef __CC__
#ifndef __EXCEPT_CLOBBER_REGS
#define __EXCEPT_CLOBBER_REGS() (void)0
#endif
#ifndef __EXCEPT_BARRIER
#if 1
#define __EXCEPT_BARRIER() __asm__ __volatile__("" : : : "memory")
#else
#define __EXCEPT_BARRIER() __COMPILER_BARRIER()
#endif
#endif

#ifndef __ERROR_CURRENT_RETHROW
#define __ERROR_CURRENT_RETHROW  error_rethrow
#define __ERROR_CURRENT_CONTINUE error_continue
#define __ERROR_CURRENT_EXCEPT   error_except
#endif /* !__ERROR_CURRENT_RETHROW */

/* High-level try/finally/except macros.
 * WARNING: Use of these macros requires a compiler implementing the local-labels
 *          extension provided by GCC and its `__label__' keyword, or the use of
 *          TPP as a preprocessor in order to function properly. */
#ifdef __INTELLISENSE__
extern bool FINALLY_WILL_RETHROW;
#define FINALLY_WILL_RETHROW FINALLY_WILL_RETHROW
#if 1
#define TRY                   if(0)
#define FINALLY               else
#define EXCEPT                else if
#define CATCH                 else if
#else
#define TRY                   __try
#define FINALLY               __finally
#define EXCEPT                __except
#define CATCH                 __except /* Same semantics, but different behavior. */
#endif
#elif defined(__TPP_SECOND_PASS)
/* Special case: TPP is invoked during a second preprocessing pass.
 * Meant to be used kind-of like this:
 * >> cat foo.c | cpp -D__TPP_SECOND_PASS | tpp | gcc
 * In this configuration, platform/compiler defines don't have
 * to be configured for TPP prior to preprocessing, as input text
 * will have already been preprocessed for the most part. */
#define FINALLY_WILL_RETHROW FINALLY_WILL_RETHROW
#define TRY                  TRY
#define FINALLY              FINALLY
#define EXCEPT               EXCEPT
#define CATCH                CATCH
#elif defined(__TPP_VERSION__)
/* Use self-redefining macros to implement a
 * recursive way of independently defining labels.
 * KOSmk1 did something similar to this. */
#define __TRY_INDIRECTION             0
#define __TRY_INDIRECTION_INCREMENT() \
   __pragma(tpp_exec("#undef __TRY_INDIRECTION\n" \
                     "#define __TRY_INDIRECTION " __PP_STR(__TPP_EVAL(__TRY_INDIRECTION+1))))
#define __TRY_INDIRECTION_DECREMENT() \
   __pragma(tpp_exec("#undef __TRY_INDIRECTION\n" \
                     "#define __TRY_INDIRECTION " __PP_STR(__TPP_EVAL(__TRY_INDIRECTION-1))))
#define __TRY_INDIRECTION_NEXTUNIQUE() \
   __pragma(tpp_exec("#ifndef __TRY_INDIRECTION_U" __PP_STR(__TRY_INDIRECTION) "\n" \
                     "#define __TRY_INDIRECTION_U" __PP_STR(__TRY_INDIRECTION) " 0\n" \
                     "#else\n" \
                     "#undef __TRY_INDIRECTION_U" __PP_STR(__TRY_INDIRECTION) "\n" \
                     "#define __TRY_INDIRECTION_U" __PP_STR(__TRY_INDIRECTION) " " \
                              __PP_STR(__TPP_EVAL(__PP_CAT2(__TRY_INDIRECTION_U,__TRY_INDIRECTION)+1)) "\n" \
                     "#endif"))
#define __TRY_INDIRECTION_BEGIN   __PP_CAT4(__try_label_begin_,__TRY_INDIRECTION,__x,__PP_CAT2(__TRY_INDIRECTION_U,__TRY_INDIRECTION))
#define __TRY_INDIRECTION_END     __PP_CAT4(__try_label_end_,__TRY_INDIRECTION,__x,__PP_CAT2(__TRY_INDIRECTION_U,__TRY_INDIRECTION))
#define __TRY_INDIRECTION_ENTRY   __PP_CAT4(__try_label_entry_,__TRY_INDIRECTION,__x,__PP_CAT2(__TRY_INDIRECTION_U,__TRY_INDIRECTION))
#define __TRY_INDIRECTION_TEMP    __PP_CAT4(__try_label_temp_,__TRY_INDIRECTION,__x,__PP_CAT2(__TRY_INDIRECTION_U,__TRY_INDIRECTION))
#define TRY \
 __TRY_INDIRECTION_INCREMENT() \
 __TRY_INDIRECTION_NEXTUNIQUE() \
 __TRY_INDIRECTION_BEGIN: \
 __asm__ __volatile__ goto("" : : : : __TRY_INDIRECTION_END); \
 __EXCEPT_BARRIER(); \
 __IF1
#define FINALLY_WILL_RETHROW  __rethrow
#define FINALLY \
 else{} \
 __EXCEPT_BARRIER(); \
 __IF0 { \
 __TRY_INDIRECTION_END: \
     __DEFINE_FINALLY_HANDLER(__TRY_INDIRECTION_BEGIN, \
                              __TRY_INDIRECTION_END, \
                              __TRY_INDIRECTION_ENTRY) \
     __builtin_unreachable(); \
 } else for(register int __rethrow = 0; !__rethrow; \
                         __rethrow ? __ERROR_CURRENT_RETHROW() : (void)(__rethrow=1)) \
            __IF0{ __TRY_INDIRECTION_ENTRY: __EXCEPT_CLOBBER_REGS(); __rethrow = 1; goto __TRY_INDIRECTION_TEMP;} \
 else __TRY_INDIRECTION_TEMP: __TRY_INDIRECTION_DECREMENT()
#define EXCEPT(mode) \
 else{} \
 __EXCEPT_BARRIER(); \
 __IF0 { \
 __TRY_INDIRECTION_END: \
     if (!__builtin_constant_p(mode) || (mode) != EXCEPT_CONTINUE_SEARCH) { \
         __DEFINE_EXCEPT_HANDLER(__TRY_INDIRECTION_BEGIN, \
                                 __TRY_INDIRECTION_END, \
                                 __TRY_INDIRECTION_ENTRY) \
     } \
     __builtin_unreachable(); \
 } else __IF1; else __TRY_INDIRECTION_ENTRY: \
   if((__EXCEPT_CLOBBER_REGS(),__builtin_constant_p(mode) ? \
     ((mode) == EXCEPT_CONTINUE_SEARCH ? (__ERROR_CURRENT_RETHROW(),0) : \
      (mode) == EXCEPT_EXECUTE_HANDLER ? 0 : \
      (__ERROR_CURRENT_CONTINUE((mode) == EXCEPT_CONTINUE_RETRY),0)) : \
       __ERROR_CURRENT_EXCEPT(mode),0)); else __TRY_INDIRECTION_DECREMENT()
#define CATCH(error) \
 else{} \
 __EXCEPT_BARRIER(); \
 __IF0 { \
 __TRY_INDIRECTION_END: \
     if (__builtin_constant_p(error)) { \
         __DEFINE_CATCH_HANDLER(__TRY_INDIRECTION_BEGIN, \
                                __TRY_INDIRECTION_END, \
                                __TRY_INDIRECTION_ENTRY, \
                                error) \
     } else { \
         __DEFINE_EXCEPT_HANDLER(__TRY_INDIRECTION_BEGIN, \
                                 __TRY_INDIRECTION_END, \
                                 __TRY_INDIRECTION_ENTRY) \
     } \
     __builtin_unreachable(); \
 } else __IF1; else __TRY_INDIRECTION_ENTRY: \
   if((__EXCEPT_CLOBBER_REGS(), \
       __builtin_constant_p(error) ? 0 : \
      (__ERROR_CURRENT_EXCEPT(error_code() == error),0))); \
   else  __TRY_INDIRECTION_DECREMENT()
#else
#define __TRY_LABEL_LINE1 __PP_CAT2(__try_label_entry1_,__LINE__)
#define __TRY_LABEL_LINE2 __PP_CAT2(__try_label_entry2_,__LINE__)
#define __TRY_LABEL_LINE3 __PP_CAT2(__try_label_entry3_,__LINE__)

#define __FORCE_REACHABLE(label) \
   __asm__ __volatile__ goto("" : : : : label);
#define TRY \
 __IF1{ __label__ __try_label_begin; \
        __label__ __try_label_end; \
        __try_label_begin: \
        __EXCEPT_BARRIER(); \
        __FORCE_REACHABLE(__try_label_end) \
        __IF1
#define FINALLY_WILL_RETHROW  __rethrow

#if 1
#define FINALLY \
        else{} \
        __EXCEPT_BARRIER(); \
        __try_label_end: \
        __DEFINE_FINALLY_HANDLER(__try_label_begin, \
                                 __try_label_end, \
                                 __TRY_LABEL_LINE2) \
        goto __TRY_LABEL_LINE1; \
 } else __TRY_LABEL_LINE1: \
 for(register int __rethrow = 0; !__rethrow; \
     __rethrow ? __ERROR_CURRENT_RETHROW() : (void)(__rethrow=1)) \
 __IF0{ __TRY_LABEL_LINE2: __EXCEPT_CLOBBER_REGS(); __rethrow = 1; goto __TRY_LABEL_LINE3;} \
 else __TRY_LABEL_LINE3:
#define EXCEPT(mode) \
        else{} \
        __EXCEPT_BARRIER(); \
        __try_label_end: \
        if (!__builtin_constant_p(mode) || (mode) != EXCEPT_CONTINUE_SEARCH) { \
            __DEFINE_EXCEPT_HANDLER(__try_label_begin, \
                                    __try_label_end, \
                                    __TRY_LABEL_LINE1) \
        } \
 } else __TRY_LABEL_LINE1:if((__EXCEPT_CLOBBER_REGS(),__builtin_constant_p(mode) ? \
                            ((mode) == EXCEPT_CONTINUE_SEARCH ? (__ERROR_CURRENT_RETHROW(),0) : \
                             (mode) == EXCEPT_EXECUTE_HANDLER ? 0 : \
                             (__ERROR_CURRENT_CONTINUE((mode) == EXCEPT_CONTINUE_RETRY),0)) : \
                             (__ERROR_CURRENT_EXCEPT(mode),0))); else 
#define CATCH(error) \
        else{} \
        __EXCEPT_BARRIER(); \
        __try_label_end: \
        if (__builtin_constant_p(error)) { \
            __DEFINE_CATCH_HANDLER(__try_label_begin, \
                                   __try_label_end, \
                                   __TRY_LABEL_LINE1, \
                                   error) \
        } else { \
            __DEFINE_EXCEPT_HANDLER(__try_label_begin, \
                                    __try_label_end, \
                                    __TRY_LABEL_LINE1) \
        } \
 } else __TRY_LABEL_LINE1:if((__EXCEPT_CLOBBER_REGS(), \
                              __builtin_constant_p(error) ? 0 : \
                             (__ERROR_CURRENT_EXCEPT(error_code() == error),0)));else 
#else
#define FINALLY \
        else{} \
        __EXCEPT_BARRIER(); \
        __IF0 { \
        __try_label_end: \
            __DEFINE_FINALLY_HANDLER(__try_label_begin, \
                                     __try_label_end, \
                                     __TRY_LABEL_LINE2) \
            __builtin_unreachable(); \
        } \
        goto __TRY_LABEL_LINE1; \
 } else __TRY_LABEL_LINE1: \
 for(register int __rethrow = 0; !__rethrow; \
     __rethrow ? __ERROR_CURRENT_RETHROW() : (void)(__rethrow=1)) \
 __IF0{ __TRY_LABEL_LINE2: __EXCEPT_CLOBBER_REGS(); __rethrow = 1; goto __TRY_LABEL_LINE3;} \
 else __TRY_LABEL_LINE3:
#define EXCEPT(mode) \
        else{} \
        __EXCEPT_BARRIER(); \
        __IF0 { \
        __try_label_end: \
            if (!__builtin_constant_p(mode) || (mode) != EXCEPT_CONTINUE_SEARCH) { \
                __DEFINE_EXCEPT_HANDLER(__try_label_begin, \
                                        __try_label_end, \
                                        __TRY_LABEL_LINE1) \
            } \
            __builtin_unreachable(); \
        } \
 } else __TRY_LABEL_LINE1:if((__EXCEPT_CLOBBER_REGS(),__builtin_constant_p(mode) ? \
                            ((mode) == EXCEPT_CONTINUE_SEARCH ? (__ERROR_CURRENT_RETHROW(),0) : \
                             (mode) == EXCEPT_EXECUTE_HANDLER ? 0 : \
                             (__ERROR_CURRENT_CONTINUE((mode) == EXCEPT_CONTINUE_RETRY),0)) : \
                              __ERROR_CURRENT_EXCEPT(mode),0)); else 
#define CATCH(error) \
        else{} \
        __EXCEPT_BARRIER(); \
        __IF0 { \
        __try_label_end: \
            if (__builtin_constant_p(error)) { \
                __DEFINE_CATCH_HANDLER(__try_label_begin, \
                                       __try_label_end, \
                                       __TRY_LABEL_LINE1, \
                                       error) \
            } else { \
                __DEFINE_EXCEPT_HANDLER(__try_label_begin, \
                                        __try_label_end, \
                                        __TRY_LABEL_LINE1) \
            } \
            __builtin_unreachable(); \
        } \
 } else __TRY_LABEL_LINE1:if((__EXCEPT_CLOBBER_REGS(), \
                              __builtin_constant_p(error) ? 0 : \
                             (__ERROR_CURRENT_EXCEPT(error_code() == error),0)));else 
#endif
#endif
#endif /* __CC__ */

/* Declare a variable for use within an exception handler.
 * The volatile is required to ensure that the variable
 * is saved on the stack, and is in a consistent state.
 * >> After wasting half a day tracking down weirdly
 *    changing variables, I eventually figured that
 *    GCC was re-using the stack locations of variables
 *    that were seemingly not being used, prior to calling
 *    some function, or piece of code marked as ATTR_NORETURN.
 * USAGE:
 * >> int EXCEPT_VAR x = 42;
 * >> TRY {
 * >>     debug_printf("In try %d\n",x);
 * >>     x = 17;
 * >>     error_throw(E_INVALID_ARGUMENT);
 * >> } FINALLY {
 * >>     debug_printf("In finally %d\n",x);
 * >> }
 * NOTE: This modifier is not required for variables
 *       that are passed to functions as pointers. */
#ifdef __INTELLISENSE__
#define EXCEPT_VAR  /* nothing */
#else
#define EXCEPT_VAR  volatile
#endif


#if defined(__USE_KOS) || defined(__ASSEMBLER__)
/* Define public versions of the private define-handler macros. */
#define DEFINE_FINALLY_HANDLER(begin,end,entry)    __DEFINE_FINALLY_HANDLER(begin,end,entry)
#define DEFINE_EXCEPT_HANDLER(begin,end,entry)     __DEFINE_EXCEPT_HANDLER(begin,end,entry)
#define DEFINE_CATCH_HANDLER(begin,end,entry,mask) __DEFINE_CATCH_HANDLER(begin,end,entry,mask)
#endif


__SYSDECL_END

#endif /* !_EXCEPT_H */
