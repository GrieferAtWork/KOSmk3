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
#include <hybrid/string.h>
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


#ifdef __CC__

/* Work around a problem (bug?) related to CSE (CommonSubexpressionElimination), that
 * will break expectations made about the validity of the return address present
 * when any of the no-return error_* functions is called.
 * For obvious reasons, that return address must be valid and actually point to
 * the location where the function was called from. However, thinking it's sooo
 * clever for doing it, GCC (WRONGLY!!!!) does the following:
 * >> volatile int x = 0;
 * >> volatile int y = 1;
 * >> TRY {
 * >>     if (x)
 * >>         error_throw(E_INVALID_ARGUMENT);
 * >> } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
 * >>     error_printf("How did we get here? x is zero...\n");
 * >>     error_handled();
 * >> }
 * >> if (y)
 * >>     error_throw(E_INVALID_ARGUMENT);
 * ASSEMBLY:
 * >>     ...
 * >> <TRY_BEGIN>
 * >>     movl   x(%ebp), %eax
 * >>     testl  %eax, %eax
 * >>     jz     1f
 * >> .Lthrow_invalid_argument:
 * >>     pushl  $E_INVALID_ARGUMENT
 * >>     call   error_throw
 * >> 1:
 * >>     jmp    .Lafter_try
 * >> <EXCEPT_BEGIN>
 * >>     ...
 * >> <TRY_END>
 * >> .Lafter_try:
 * >>     movl   y(%ebp), %eax
 * >>     testl  %eax, %eax
 * >>     jnz    .Lthrow_invalid_argument // <<--- This right here
 * >>     ret
 *
 * I understand where GCC comes from with this, noticing that
 * `error_throw()' is called twice with the same arguments,
 * however it (WRONGLY!) ignores that fact that the return
 * address of a function (which is even accessible using the
 * arch-independent builtin function `__builtin_return_address(0)')
 * is a valid side-effect of a call that should be considered
 * just as important as arguments passed to a function.
 *
 * Now I get that not every function marked as noreturn actually
 * needs to know about its return address the way that error_*
 * functions need to, but the problem also exists in c++, where
 * GCC _really_ can't optimize away the return address of a
 * function.
 * I haven't really looked into what g++ does to prevent this
 * optimization, but whatever it is, it should be possible to
 * enforce the same behavior in C using some kind of attribute
 * in the lines of `__attribute__((need_return))' or `__attribute__((no_cse))'.
 */
#if defined(__INTELLISENSE__) || defined(__cplusplus)
#define __EXCEPT_INVOKE_THROW(expr)          expr
#define __EXCEPT_INVOKE_THROW_T(T,expr)      expr
#define __EXCEPT_INVOKE_THROW_NORETURN(expr) expr
#define __EXCEPT_NORETURN                    __ATTR_NORETURN
#else
/* Work around: introduce non-optimizable side-effects
 *              to every call to an error_* function. */
#define __EXCEPT_INVOKE_THROW(expr) \
    __XBLOCK({ __asm__ __volatile__("#%="); expr; __asm__ __volatile__("#%="); })
#define __EXCEPT_INVOKE_THROW_T(T,expr) \
    __XBLOCK({ register T __it_temp; __asm__ __volatile__("#%="); __it_temp = expr; __asm__ __volatile__("#%="); __XRETURN __it_temp; })
#define __EXCEPT_INVOKE_THROW_NORETURN(expr) \
    __XBLOCK({ __asm__ __volatile__("#%="); expr; __asm__ __volatile__("#%="); __builtin_unreachable(); })
#define __EXCEPT_NORETURN                    /* nothing */
#endif

/* Helper functions for implementing exception handling. */


/* Throw an error `code' at the current code location.
 * Any additional exception information is ZERO-initialized.
 * @return: true:  The error-condition should be re-checked after `error_continue(true)'
 * @return: false: Ignore the error and continue normally after `error_continue(false)' */
__LIBC __BOOL (__FCALL error_throw_resumable)(except_t __code);
__LIBC __EXCEPT_NORETURN void (__FCALL error_throw)(except_t __code);
#ifndef error_throw_resumable
#define error_throw_resumable(code) __EXCEPT_INVOKE_THROW_T(__BOOL,(error_throw_resumable)(code))
#define error_throw(code)  __EXCEPT_INVOKE_THROW_NORETURN((error_throw)(code))
#endif

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
 *   (except_t code = E_NET_ERROR, unsigned int net_errcode);
 *   (except_t code = E_NO_DEVICE, unsigned int d_type, dev_t d_devno);
 *   (except_t code = E_UNHANDLED_INTERRUPT, unsigned int ui_intcode, uintptr_t ui_errcode);
 *   (except_t code = E_UNKNOWN_SYSTEMCALL, syscall_ulong_t us_sysno, [syscall_ulong_t args...]);
 *   (except_t code = E_EXIT_THREAD, int e_status);
 *   (except_t code = E_EXIT_PROCESS, int e_status);
 *   (except_t code = E_INVALID_SEGMENT, unsigned int is_register, unsigned int is_segment);
 * All other codes are thrown without additional arguments,
 * meaning the same way the functions above are thrown. */
__LIBC __BOOL (__ATTR_CDECL error_throw_resumablef)(except_t __code, ...);
__LIBC __EXCEPT_NORETURN void (__ATTR_CDECL error_throwf)(except_t __code, ...);
#ifndef error_throw_resumablef
#define error_throw_resumablef(...) __EXCEPT_INVOKE_THROW_T(__BOOL,(error_throw_resumablef)(__VA_ARGS__))
#define error_throwf(...)  __EXCEPT_INVOKE_THROW_NORETURN((error_throwf)(__VA_ARGS__))
#endif

/* Same as the functions above, but fill in exception data as
 * a direct copy of the passed `exception_data' structure. */
__LIBC __BOOL (__FCALL error_throw_resumable_ex)(except_t __code, struct exception_data const *__restrict __data);
__LIBC __EXCEPT_NORETURN void (__FCALL error_throw_ex)(except_t __code, struct exception_data const *__restrict __data);
#ifndef error_throw_resumable_ex
#define error_throw_resumable_ex(code,data) __EXCEPT_INVOKE_THROW_T(__BOOL,(error_throw_resumable_ex)(code,data))
#define error_throw_ex(code,data)  __EXCEPT_INVOKE_THROW_NORETURN((error_throw_ex)(code,data))
#endif

/* Throw the currently set exception exception information.
 * NOTE: When the `ERR_FRESUMABLE' flag isn't set, this function never returns.
 * Unlike `error_throw()', this function does not clear additional
 * exception information (as accessible through `error_info()'), meaning
 * that this function should be used when throwing errors with additional
 * information connected to them.
 * @return: true:  The error-condition should be re-checked after `error_continue(true)'
 * @return: false: Ignore the error and continue normally after `error_continue(false)' */
__LIBC __BOOL (__FCALL error_throw_current)(void);
#ifndef error_throw_current
#define error_throw_current() __EXCEPT_INVOKE_THROW_T(__BOOL,(error_throw_current)())
#endif

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
__LIBC __EXCEPT_NORETURN void (__FCALL error_rethrow)(void);
#ifndef error_rethrow
#define error_rethrow() __EXCEPT_INVOKE_THROW_NORETURN((error_rethrow)())
#endif

/* Continue execution
 * If the current exception has the `ERR_FRESUMABLE'
 * flag set, or the faulting instruction isn't
 * understood, an `E_NONCONTINUABLE' is thrown.
 * @param: retry: When non-zero, jump to the faulting instruction or have
 *               `error_throw_resumable()' / `error_throw_current()' return true.
 *                Otherwise, jump to the instruction thereafter or have
 *               `error_throw_resumable()' / `error_throw_current()' return false. */
__LIBC __EXCEPT_NORETURN void (__FCALL error_continue)(int __retry);
#ifndef error_continue
#define error_continue(retry) __EXCEPT_INVOKE_THROW_NORETURN((error_continue)(retry))
#endif

/* Process the exception disposition of an except-handler
 * This function does not return unless `mode > 0'.
 * When equal to `0', it will continue searching for more
 * exception handlers by jumping to run `error_rethrow()'.
 * Otherwise, it will invoke `error_continue(mode == EXCEPT_CONTINUE_RETRY)' */
__LIBC void (__FCALL error_except)(int __mode);
#ifndef error_except
#define error_except(mode) __EXCEPT_INVOKE_THROW((error_except)(mode))
#endif

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

/* Deallocate continue information if possible.
 * This function should be called prior to re-attempting an operation
 * in situations where an operation is to be re-attempted, should it
 * have failed before then.
 * WARNING: This function operates by moving the stack-pointer before
 *          returning, however will not do so if additional stack-memory
 *          has been allocated since execution returned to the location
 *          to the exception handler.
 * @return: * : The number of deallocated bytes of stack memory. */
__LIBC __SIZE_TYPE__ (__FCALL error_dealloc_continue)(void);

/* Same as `error_dealloc_continue()', but carrying a slightly
 * more obvious name that should easily imply its meaning.... */
__LIBC void (__FCALL error_handled)(void);

#ifndef __EXCEPT_INVOKE_DEALLOC_CONTINUE
#define __EXCEPT_INVOKE_DEALLOC_CONTINUE(x) x
#define __EXCEPT_INVOKE_HANDLED(x) x
#endif

#ifndef error_dealloc_continue
#define error_dealloc_continue() \
      __EXCEPT_INVOKE_DEALLOC_CONTINUE((error_dealloc_continue)())
#define error_handled() \
      __EXCEPT_INVOKE_HANDLED((error_handled)())
#endif



/* push/pop the current error information context.
 * Useful if you wish to preserve exception information across
 * a section of code that may cause, and then handle other exceptions:
 * >>    error_pushinfo();
 * >>again:
 * >>    TRY {
 * >>        some_interruptible_system_call();
 * >>    } CATCH (E_INTERRUPT) {
 * >>        goto again;
 * >>    }
 * >>    error_popinfo();
 * For more information, see `NOTES ON CLOBBERING EXCEPTIONS'
 */
#define error_pushinfo() \
do{ struct exception_info __push_info; \
    __hybrid_memcpy(&__push_info,error_info(),sizeof(struct exception_info))
#define error_popinfo() \
    __hybrid_memcpy(error_info(),&__push_info,sizeof(struct exception_info)); \
}__WHILE0


/*  === NOTES ON CLOBBERING EXCEPTIONS === 
 * 
 * - These notes only refer to ~current~ exception information,
 *   as addressible by the return value of `error_info()'.
 * - Generally speaking, thread-local exception information should
 *   be treated like a caller-safe/scratch register, that is a
 *   register which any function is allowed to modify without it
 *   also needing to restore its contents prior to returning.
 * - At first glance this might seem like a bad idea, but due to
 *   the fact that nobody is really perfect, any programmer using
 *   exceptions should realize that whenever they do something
 *   wrong, KOS might come around and throw an exception into
 *   their face.
 *   Considering that, it should become clear that making exception
 *   information callee-safe is a bad idea because in most situations
 *   an exception isn't even supposed to arise in the first place,
 *   which would also mean that having a library function such as
 *  `fwrite()' (which calls into `Xfwrite()' and translates exceptions
 *   thrown into `errno' values; s.a. `except_errno()'), would have
 *   to copy all data from `error_info()' _every_ _time_.
 *   That would be really slow.
 * - However, considering that some functions are likely to-be called
 *   from FINALLY-blocks, it also wouldn't make much sense to force
 *   users of FINALLY to have to preserve exception information every
 *   time their block is entered.
 *   And that is why so-called ~cleanup~ functions (as already defined
 *   by the documentation of `_EXCEPT_API' in `<features.h>') will not
 *   clobber active exception information.
 *   This includes functions such as `free()', `close()' or `unmap()',
 *   meaning that is will always be safe to call such functions from
 *   finally handlers.
 * - WARNING: Don't assume that a function marked as `ATTR_NOTHROW' will
 *            not clobber exceptions. In fact, it may just be implemented
 *            as a wrapper around another function to translate exceptions
 *            into errno values.
 *
 * Example:
 * >> TRY {
 * >>     for (;;) Xmalloc(0x87654321);
 * >> } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
 * >>     printf("Why isn't this working!?!?\n");    // WRONG (`printf()' might clobber exception information)
 * >>     error_printf(NULL);
 * >> }
 *
 * >> TRY {
 * >>     for (;;) Xmalloc(0x87654321);
 * >> } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
 * >>     error_pushinfo();                          // Must preserve exception information across a call to `printf()'
 * >>     printf("Why isn't this working!?!?\n");
 * >>     error_popinfo();
 * >>     error_printf(NULL);
 * >> }
 *
 *  === EXCEPTIONS AND POSIX SIGNAL HANDLERS === 
 *
 * - The kernel will push the old `error_info()' onto the stack prior
 *   to entering the signal handler, and will restore it before when
 *   it returns. This way, signal handlers can happily invoke library
 *   functions that may clobber error information without the need to
 *   manually preserve exception information, and without having to
 *   worry about what it is that the signal handler return location
 *   will see in case it is currently using signal handlers.
 * - Note however that posix signal handlers are allowed to throw
 *   exceptions, and those exception will in fact be propagated to
 *   the signal handler return site.
 *   If a signal handler is left by use of an exception, the previously
 *   push error information will in fact _not_ be restored.
 *
 */





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
#ifndef __INTELLISENSE__
#define except_errno()    (except_errno(),EXCEPT_EXECUTE_HANDLER)
#endif

/* Return the `errno' value of the currently active exception. */
__LIBC errno_t (__FCALL except_geterrno)(void);
#endif /* !__KERNEL__ */

/* The internal function used for unwinding the stack,
 * giving `context' describing where should start.
 * WARNING: `context' will be modified as the stack is unwound,
 *           and the initial instruction pointer must be directed
 *           at the start, or into the middle of the instruction which
 *           caused the exception. (Not at the instruction thereafter)
 * This function is invoked by `error_rethrow', using
 * a stack-allocated cpu-context of the caller. */
__LIBC __EXCEPT_NORETURN void (__FCALL __error_rethrow_at)(struct cpu_context *__restrict __context);
#ifndef __error_rethrow_at
#define __error_rethrow_at(context) __EXCEPT_INVOKE_THROW_NORETURN((__error_rethrow_at)(context))
#endif

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
#define __ERROR_CURRENT_HANDLED  error_handled
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
#define EXCEPT_HANDLED        else if
#define CATCH                 else if
#define CATCH_HANDLED         else if
#else
#define TRY                   __try
#define FINALLY               __finally
#define EXCEPT                __except
#define EXCEPT_HANDLED        __except
#define CATCH                 __except /* Same semantics, but different behavior. */
#define CATCH_HANDLED         __except /* Same semantics, but different behavior. */
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
#define EXCEPT_HANDLED       EXCEPT_HANDLED
#define CATCH_HANDLED        CATCH_HANDLED
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
 __EXCEPT_BARRIER(); \
 __asm__ __volatile__ goto("" : : : "memory" : __TRY_INDIRECTION_END); \
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
     ((mode) == EXCEPT_EXECUTE_HANDLER ? 0 : \
      (__ERROR_CURRENT_CONTINUE((mode) == EXCEPT_CONTINUE_RETRY),0)) : \
       __ERROR_CURRENT_EXCEPT(mode),0)); else __TRY_INDIRECTION_DECREMENT()
#define EXCEPT_HANDLED(mode) \
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
     ((mode) == EXCEPT_EXECUTE_HANDLER ? 0 : \
      (__ERROR_CURRENT_CONTINUE((mode) == EXCEPT_CONTINUE_RETRY),0)) : \
       __ERROR_CURRENT_EXCEPT(mode),__ERROR_CURRENT_HANDLED(),0)); \
   else __TRY_INDIRECTION_DECREMENT()
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
   if((__EXCEPT_CLOBBER_REGS(),__builtin_constant_p(error) ? 0 : \
      (__ERROR_CURRENT_EXCEPT(error_code() == error),0))); \
   else  __TRY_INDIRECTION_DECREMENT()
#define CATCH_HANDLED(error) \
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
   if((__EXCEPT_CLOBBER_REGS(),__builtin_constant_p(error) ? 0 : \
      (__ERROR_CURRENT_EXCEPT(error_code() == error), \
       __ERROR_CURRENT_HANDLED(),0))); \
   else __TRY_INDIRECTION_DECREMENT()
#else
#define __TRY_LABEL_LINE1 __PP_CAT2(__try_label_entry1_,__LINE__)
#define __TRY_LABEL_LINE2 __PP_CAT2(__try_label_entry2_,__LINE__)
#define __TRY_LABEL_LINE3 __PP_CAT2(__try_label_entry3_,__LINE__)
#define TRY \
 __IF1{ __label__ __try_label_begin; \
        __label__ __try_label_end; \
        __try_label_begin: \
        __asm__ __volatile__ goto("" : : : "memory" : __try_label_end); \
        __IF1
#define FINALLY_WILL_RETHROW  __rethrow

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
                            ((mode) == EXCEPT_EXECUTE_HANDLER ? 0 : \
                              __ERROR_CURRENT_CONTINUE((mode) == EXCEPT_CONTINUE_RETRY)) : \
                              __ERROR_CURRENT_EXCEPT(mode),0)); else 
#define EXCEPT_HANDLED(mode) \
        else{} \
        __EXCEPT_BARRIER(); \
        __try_label_end: \
        if (!__builtin_constant_p(mode) || (mode) != EXCEPT_CONTINUE_SEARCH) { \
            __DEFINE_EXCEPT_HANDLER(__try_label_begin, \
                                    __try_label_end, \
                                    __TRY_LABEL_LINE1) \
        } \
 } else __TRY_LABEL_LINE1:if((__EXCEPT_CLOBBER_REGS(),__builtin_constant_p(mode) ? \
                            ((mode) == EXCEPT_EXECUTE_HANDLER ? 0 : \
                              __ERROR_CURRENT_CONTINUE((mode) == EXCEPT_CONTINUE_RETRY)) : \
                              __ERROR_CURRENT_EXCEPT(mode),__ERROR_CURRENT_HANDLED(),0)); \
   else
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
 } else __TRY_LABEL_LINE1:if((__EXCEPT_CLOBBER_REGS(),__builtin_constant_p(error) ? 0 : \
                             (__ERROR_CURRENT_EXCEPT(error_code() == error),0)));else 
#define CATCH_HANDLED(error) \
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
 } else __TRY_LABEL_LINE1:if((__EXCEPT_CLOBBER_REGS(),__builtin_constant_p(error) ? (void)0 : \
                              __ERROR_CURRENT_EXCEPT(error_code() == error), \
                              __ERROR_CURRENT_HANDLED(),0));else 
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
 *       that are passed to functions as pointers.
 * WARNING: GCC bug?
 *     GCC seems to ignore `volatile' in function arguments and will load
 *     such arguments into registers and access them that way regardless.
 * ... This is _real_ baaaad. Especially considering that KOS relies on
 *     the fact that volatile variables are immediately saved (to memory)
 *     during every access, as well as remain allocated in a stack frame
 *     for as long as they remain visible.
 * I realize that this once again scrapes the definition of `volatile', and
 * having read the logs of more an one bug report filed for GCC that was
 * related to the interpretation of the C standard, or the request for a
 * change to some semantic meaning, where the change would allow for more
 * interesting and useful coding tricks, there'd probably be no point in
 * even mentioning this... :(
 * Because _NOT_ _A_ _SINGLE_ _TIME_ did one of those bugs get resolved,
 * and _EVERY_ _ONE_ _OF_ _THEM_ ends with:
 *   >> F$ck off. We own gcc, and if you don't like it: make your own compiler.
 * As a workaround however, it seems as though you can simply do this:
 * >> void foo(int x) {
 * >>     int EXCEPT_VAR except_x = x;
 * >>     printf("x = %d\n",x); // Can use the regular `x' here
 * >>     TRY {
 * >>         something_dangerous();
 * >>     } FINALLY {
 * >>         printf("x = %d\n",except_x); // Must use `except_x' here
 * >>     }
 * >> }
 * Instead of this:
 * >> void foo(int EXCEPT_VAR x) {
 * >>     printf("x = %d\n",x);
 * >>     TRY {
 * >>         something_dangerous();
 * >>     } FINALLY {
 * >>         // GCC ignores `volatile' and may generate code that puts
 * >>         // `x' into a register prior to the `TRY' above, meaning
 * >>         // that this could (definitly incorrectly) be compiled as:
 * >>         // >> pushl  %eax   // If `x' was loaded into EAX
 * >>         // >> pushl  $.Lx_eq_percent_d
 * >>         // >> call   printf
 * >>         // >> addl   $8, %esp
 * >>         printf("x = %d\n",x);
 * >>     }
 * >> }
 */
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


/* Turn off `__builtin_expect()' to prevent GCC from re-arranging code:
 * >> TRY {
 * >>     // Despite all efforts, while `__builtin_expect()' remains available
 * >>     // in code using exception handlers, GCC may decide to re-arrange
 * >>     // assembly in a way that will break exception handlers, as it would
 * >>     // otherwise be allowed to move the call to `allocate_data()' outside
 * >>     // of the guarded code section.
 * >>     // We are able to prohibit it from doing this without branch prediction
 * >>     // hints by passing `-fno-reorder-blocks' on the commandline, and for
 * >>     // the longest time I thought that's all that's actually required, but
 * >>     // after having no problems for the longest time, some exception handlers
 * >>     // suddenly stopped being invoked, at which point I noticed that GCC
 * >>     // was still re-arranging code that was manually tagged.
 * >>     // NOTE:
 * >>     //   - Tagging branches as likely, or unlikely is still a very
 * >>     //     good thing to do, if only to mean reading the code easier.
 * >>     //   - However once code starts using exception handlers, sadly
 * >>     //     we have to turn off branch hints.
 * >>     if unlikely(!data_is_allocated())
 * >>        data = allocate_data();
 * >> } CATCH (E_BADALLOC) {
 * >>     error_printf("Cannot allocate data\n");
 * >>     error_handled();
 * >> }
 */
#ifndef __NO_builtin_expect
#undef __builtin_expect
#define __builtin_expect(x,y) x
#endif


__SYSDECL_END

#endif /* !_EXCEPT_H */
