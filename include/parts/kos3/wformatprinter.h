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
#ifndef _PARTS_KOS3_WFORMATPRINTER_H
#define _PARTS_KOS3_WFORMATPRINTER_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <parts/kos2/wformatprinter.h>
#include <xlocale.h>

#if __KOS_VERSION__ >= 300 && defined(__CRT_KOS) && !defined(__KERNEL__)

__SYSDECL_BEGIN

__LIBC __ssize_t (__LIBCCALL format_wrepeat)(pwformatprinter __printer, void *__closure,
                                             wchar_t __ch, __size_t __num_repetitions);

__NAMESPACE_STD_BEGIN
struct tm;
__NAMESPACE_STD_END

#if defined(__BUILDING_LIBC) || defined(__KERNEL__)
#define __DEFINE_STRINGPRINTER(T) \
     T   *sp_bufpos; /* [1..1][>= sp_buffer][<= sp_bufend] . */ \
     T   *sp_buffer; /* [1..1] Allocate buffer base pointer. */ \
     T   *sp_bufend; /* [1..1] Buffer end (Pointer to currently allocated `'\0'´-character). */
#else /* __BUILDING_LIBC || __KERNEL__ */
#define __DEFINE_STRINGPRINTER(T) \
     T *__sp_bufpos; /* [1..1][>= __sp_buffer][<= __sp_bufend] . */ \
     T *__sp_buffer; /* [1..1] Allocate buffer base pointer. */ \
     T *__sp_bufend; /* [1..1] Buffer end (Pointer to currently allocated `'\0'´-character). */
#endif /* !__BUILDING_LIBC && !__KERNEL__ */
#ifndef __wstringprinter_defined
#define __wstringprinter_defined 1
struct wstringprinter { __DEFINE_STRINGPRINTER(wchar_t) };
#endif /* !__wstringprinter_defined */
#undef __DEFINE_STRINGPRINTER


/* Helper functions for using any pformatprinter-style
 * function to print into a dynamically allocated string.
 * >> struct stringprinter printer; char *text;
 * >> if (stringprinter_init(&printer,0)) return handle_error();
 * >> if (format_printf(&stringprinter_print,&printer,"Hello %s","dynamic world")) {
 * >>   stringprinter_fini(&printer);
 * >>   return handle_error();
 * >> } else {
 * >>   text = stringprinter_pack(&printer,NULL);
 * >>   //stringprinter_fini(&printer); // No-op after pack has been called
 * >> }
 * >> ...
 * >> free(text);
 * @param: HINT: A hint as to how big the initial buffer should
 *               be allocated as (Pass ZERO if unknown).
 * @return:  0: Successfully printed to/initialized the given string printer.
 * @return: -1: Failed to initialize/print the given text (`errno' is set to ENOMEM) */
__REDIRECT_EXCEPT_XVOID(__LIBC,__PORT_KOSONLY __NONNULL((1)),int,__LIBCCALL,wstringprinter_init,(struct wstringprinter *__restrict __self, __size_t __hint),(__self,__hint))
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL wstringprinter_fini)(struct wstringprinter *__restrict __self);
__LIBC __PORT_KOSONLY __ATTR_RETNONNULL __NONNULL((1)) wchar_t *(__LIBCCALL wstringprinter_pack)(struct wstringprinter *__restrict __self, __size_t *__length);
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,wstringprinter_print,(wchar_t const *__restrict __data, __size_t __datalen, void *__closure),(__data,__datalen,__closure))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __NONNULL((1)) void (__LIBCCALL Xwstringprinter_init)(struct wstringprinter *__restrict __self, __size_t __hint);
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL Xwstringprinter_print)(wchar_t const *__restrict __data, __size_t __datalen, void *__closure);
#endif /* __USE_EXCEPT */

/* Wide-character format printers. */
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF(3,4) __NONNULL((1,3)) __ssize_t (__ATTR_CDECL format_wprintf)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF(3,0) __NONNULL((1,3,4)) __ssize_t (__LIBCCALL format_vwprintf)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF(3,5) __NONNULL((1,3)) __ssize_t (__ATTR_CDECL format_wprintf_l)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF(3,0) __NONNULL((1,3,5)) __ssize_t (__LIBCCALL format_vwprintf_l)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(3,4) __NONNULL((1,3)) __ssize_t (__ATTR_CDECL format_wprintf_p)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(3,0) __NONNULL((1,3,4)) __ssize_t (__LIBCCALL format_vwprintf_p)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(3,5) __NONNULL((1,3)) __ssize_t (__ATTR_CDECL format_wprintf_p_l)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(3,0) __NONNULL((1,3,5)) __ssize_t (__LIBCCALL format_vwprintf_p_l)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);

/* Wide-character format helpers. */
__LIBC __PORT_KOSONLY __NONNULL((1,3)) __ssize_t (__LIBCCALL format_wquote)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __text, __size_t __textlen, __UINT32_TYPE__ __flags);
__LIBC __PORT_KOSONLY __NONNULL((1,3)) __ssize_t (__LIBCCALL format_wquote_l)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __text, __size_t __textlen, __UINT32_TYPE__ __flags, __locale_t __locale);
__LIBC __PORT_KOSONLY __NONNULL((1,3)) __ssize_t (__LIBCCALL format_whexdump)(pwformatprinter __printer, void *__closure, void const *__restrict __data, __size_t __size, __size_t __linesize, __UINT32_TYPE__ __flags);
__LIBC __PORT_KOSONLY __NONNULL((1,3)) __ssize_t (__LIBCCALL format_whexdump_l)(pwformatprinter __printer, void *__closure, void const *__restrict __data, __size_t __size, __size_t __linesize, __UINT32_TYPE__ __flags, __locale_t __locale);

/* Wide-character time formatting. */
__LIBC __PORT_KOSONLY __ATTR_LIBC_WCSFTIME(3,4) __NONNULL((1,4)) __ssize_t (__LIBCCALL format_wcsftime)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, struct __NAMESPACE_STD_SYM tm const *__restrict __tm);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WCSFTIME(3,4) __NONNULL((1,4)) __ssize_t (__LIBCCALL format_wcsftime_l)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, struct __NAMESPACE_STD_SYM tm const *__restrict __tm, __locale_t __locale);

#ifndef __pwformatgetc_defined
#define __pwformatgetc_defined 1
/* NOTE: `pwformatgetc()' differs from `pformatgetc()', in
 *        that it must store the wide-character in `*PCH'
 * @return: >= 0: The character was stored in `*PCH'.
 * @return: -1:   The input stream has ended.
 * @return: < -1: An error occurred (Return the same value to the caller) */
typedef __ssize_t (__LIBCCALL *pwformatgetc)(wchar_t *__pch, void *__closure);
typedef __ssize_t (__LIBCCALL *pwformatungetc)(wchar_t __ch, void *__closure);
#endif /* !__pwformatgetc_defined */

/* Wide-character format scanners. */
__LIBC __PORT_KOSONLY __ATTR_LIBC_WSCANF(4,5) __NONNULL((1,2,4)) __ssize_t (__ATTR_CDECL format_wscanf)(pwformatgetc __pgetc, pwformatungetc __pungetc, void *__closure, wchar_t const *__restrict __format, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WSCANF(4,0) __NONNULL((1,2,4)) __ssize_t (__LIBCCALL format_vwscanf)(pwformatgetc __pgetc, pwformatungetc __pungetc, void *__closure, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WSCANF(4,6) __NONNULL((1,2,4)) __ssize_t (__ATTR_CDECL format_wscanf_l)(pwformatgetc __pgetc, pwformatungetc __pungetc, void *__closure, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WSCANF(4,0) __NONNULL((1,2,4)) __ssize_t (__LIBCCALL format_vwscanf_l)(pwformatgetc __pgetc, pwformatungetc __pungetc, void *__closure, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);




#ifndef __OMIT_BUFFER_DECLARATIONS
/* Buffered format printing.
 * >> Since format printing is used quite often thoughout the user-space and the kernel,
 *    many less-than optimized print generators are often chained together with fairly
 *    slow print receivers.
 *    To speed up performance by bunching together a whole lot of data, a buffer
 *    can be used to automatically collect data until it is flushed, or deleted:
 * HINT: If the buffer fails to allocate memory, it will try to flush itself and
 *       attempt to allocate memory again. If this still fails, print commands
 *       are passed through directly, meaning that the buffer is still going to
 *       generated the desired output, be it with less efficient throughput.
 *
 * >> struct buffer *log_buffer;
 * >> log_buffer = buffer_new(&syslog_printer,SYSLOG_PRINTER_CLOSURE(LOG_WARNING));
 * >> // `format_printf' is unbuffered, meaning that normally each component would call
 * >> // `syslog_printer()', resulting in a total to 7 calls: "a" "foo" ",b" "bar" ",c" "foobar" "\n"
 * >> // Using a buffer, this function returning.
 * >> format_printf(&buffer_print,log_buffer,"a%s,b%s,c%s\n","foo","bar","foobar");
 * 
 * WARNING: Buffers are themself not thread-safe. They are intended for local
 *          use, or require the caller to perform their own synchronization.
 */
#ifndef __wbuffer_defined
#define __wbuffer_defined 1
#if defined(__BUILDING_LIBC) || defined(__USE_KOS)
#define __DEFINE_BUFFER(Tprinter,T) \
    Tprinter          b_printer; /* [1..1] The underlying printer. */ \
    void             *b_closure; /* [?..?] The closure argument passed to `b_printer' */ \
    union{ \
       __uintptr_t  __b_align0;  /* ... */ \
       __ssize_t      b_state;   /* The current printer state (< 0: Last error code returned by `b_printer'; >= 0: Sum of `b_printer' callbacks). */ \
    }; \
    T                *b_buffer;  /* [0..1][owned] Base-pointer of the allocated buffer. */ \
    T                *b_bufpos;  /* [0..1][>= b_buffer && <= b_bufend] The current buffer position (Pointer to the buffer byte written to next). */ \
    T                *b_bufend;  /* [0..1] End of the allocated buffer (first byte no longer apart of the buffer). */ \
    void           *__padding;   /* ... (Forward-compatibility & align to `8*sizeof(void *)', which is quite the pretty number) */
#else
#define __DEFINE_BUFFER(Tprinter,T) \
    Tprinter          __b_printer; /* [1..1] The underlying printer. */ \
    void             *__b_closure; /* [?..?] The closure argument passed to `b_printer' */ \
    union{ \
       __uintptr_t    __b_align0;  /* ... */ \
       __ssize_t      __b_state;   /* The current printer state (< 0: Last error code returned by `b_printer'; >= 0: Sum of `b_printer' callbacks). */ \
    }; \
    T                *__b_buffer;  /* [0..1][owned] Base-pointer of the allocated buffer. */ \
    T                *__b_bufpos;  /* [0..1][>= b_buffer && <= b_bufend] The current buffer position (Pointer to the buffer byte written to next). */ \
    T                *__b_bufend;  /* [0..1] End of the allocated buffer (first byte no longer apart of the buffer). */ \
    void             *__padding;   /* ... (Forward-compatibility & align to `8*sizeof(void *)', which is quite the pretty number) */
#endif
struct wbuffer { __DEFINE_BUFFER(pwformatprinter,wchar_t) };
#undef __DEFINE_BUFFER
#endif /* !__wbuffer_defined */

#ifndef WBUFFER_INIT
#define WBUFFER_INIT(printer,closure)  {printer,closure,{0},__NULLPTR,__NULLPTR,__NULLPTR,__NULLPTR}
#endif

/* Wide-character version of string buffers. */
__LIBC __PORT_KOSONLY void (__LIBCCALL wbuffer_init)(struct wbuffer *__restrict self, pwformatprinter __printer, void *__closure);
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL wbuffer_fini)(struct wbuffer *__restrict __buf);
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL wbuffer_flush)(struct wbuffer *__restrict __buf);
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL wbuffer_print)(wchar_t const *__restrict __data, __size_t __datalen, void *__closure);

#endif /* !__OMIT_BUFFER_DECLARATIONS */


__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF(3,4) __NONNULL((1,3)) __ssize_t (__ATTR_CDECL format_wbprintf)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF(3,0) __NONNULL((1,3,4)) __ssize_t (__LIBCCALL format_vwbprintf)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF(3,5) __NONNULL((1,3)) __ssize_t (__ATTR_CDECL format_wbprintf_l)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF(3,0) __NONNULL((1,3,5)) __ssize_t (__LIBCCALL format_vwbprintf_l)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(3,4) __NONNULL((1,3)) __ssize_t (__ATTR_CDECL format_wbprintf_p)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(3,0) __NONNULL((1,3,4)) __ssize_t (__LIBCCALL format_vwbprintf_p)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, __builtin_va_list __args);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(3,5) __NONNULL((1,3)) __ssize_t (__ATTR_CDECL format_wbprintf_p_l)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, __locale_t __locale, ...);
__LIBC __PORT_KOSONLY __ATTR_LIBC_WPRINTF_P(3,0) __NONNULL((1,3,5)) __ssize_t (__LIBCCALL format_vwbprintf_p_l)(pwformatprinter __printer, void *__closure, wchar_t const *__restrict __format, __locale_t __locale, __builtin_va_list __args);

__SYSDECL_END

#endif /* __KOS_VERSION__ >= 300 && __CRT_KOS && !__KERNEL__ */

#endif /* !_PARTS_KOS3_WFORMATPRINTER_H */
