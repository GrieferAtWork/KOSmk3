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
#ifndef GUARD_LIBS_LIBC_FORMAT_H
#define GUARD_LIBS_LIBC_FORMAT_H 1

#include "libc.h"

#ifdef __CC__
DECL_BEGIN


/* ===================================================================================== */
/*     FORMAT                                                                            */
/* ===================================================================================== */
#ifndef __pw16formatprinter_defined
#define __pw16formatprinter_defined 1
typedef __ssize_t (__LIBCCALL *pw16formatprinter)(char16_t const *__restrict data, size_t datalen, void *closure);
typedef __ssize_t (__LIBCCALL *pw32formatprinter)(char32_t const *__restrict data, size_t datalen, void *closure);
#endif /* !__pw16formatprinter_defined */
#ifndef __pw16formatgetc_defined
#define __pw16formatgetc_defined 1
typedef __ssize_t (__LIBCCALL *pw16formatgetc)(char16_t *__pch, void *__closure);
typedef __ssize_t (__LIBCCALL *pw16formatungetc)(char16_t __ch, void *__closure);
typedef __ssize_t (__LIBCCALL *pw32formatgetc)(char32_t *__pch, void *__closure);
typedef __ssize_t (__LIBCCALL *pw32formatungetc)(char32_t __ch, void *__closure);
#endif /* !__pw16formatgetc_defined */
struct w16printer;
struct w32printer;
struct stringprinter;
struct string16printer;
struct string32printer;
struct buffer;
struct w16buffer;
struct w32buffer;

/* String printer functions for construction of C-style NUL-terminated strings. */
INTDEF int       LIBCCALL libc_stringprinter_init(struct stringprinter *__restrict self, size_t hint);
INTDEF char     *LIBCCALL libc_stringprinter_pack(struct stringprinter *__restrict self, size_t *length);
INTDEF void      LIBCCALL libc_stringprinter_fini(struct stringprinter *__restrict self);
INTDEF ssize_t   LIBCCALL libc_stringprinter_print(char const *__restrict data, size_t datalen, void *closure);
INTDEF int       LIBCCALL libc_string16printer_init(struct string16printer *__restrict self, size_t hint);
INTDEF char16_t *LIBCCALL libc_string16printer_pack(struct string16printer *__restrict self, size_t *length);
INTDEF void      LIBCCALL libc_string16printer_fini(struct string16printer *__restrict self);
INTDEF ssize_t   LIBCCALL libc_string16printer_print(char16_t const *__restrict data, size_t datalen, void *closure);
INTDEF int       LIBCCALL libc_string32printer_init(struct string32printer *__restrict self, size_t hint);
INTDEF char32_t *LIBCCALL libc_string32printer_pack(struct string32printer *__restrict self, size_t *length);
INTDEF void      LIBCCALL libc_string32printer_fini(struct string32printer *__restrict self);
INTDEF ssize_t   LIBCCALL libc_string32printer_print(char32_t const *__restrict data, size_t datalen, void *closure);
INTDEF void      LIBCCALL libc_Xstringprinter_init(struct stringprinter *__restrict self, size_t hint);
INTDEF ssize_t   LIBCCALL libc_Xstringprinter_print(char const *__restrict data, size_t datalen, void *closure);
INTDEF void      LIBCCALL libc_Xstring16printer_init(struct string16printer *__restrict self, size_t hint);
INTDEF ssize_t   LIBCCALL libc_Xstring16printer_print(char16_t const *__restrict data, size_t datalen, void *closure);
INTDEF void      LIBCCALL libc_Xstring32printer_init(struct string32printer *__restrict self, size_t hint);
INTDEF ssize_t   LIBCCALL libc_Xstring32printer_print(char32_t const *__restrict data, size_t datalen, void *closure);

/* String buffers. */
INTDEF void    LIBCCALL libc_buffer_init(struct buffer *__restrict self, pformatprinter printer, void *closure);
INTDEF ssize_t LIBCCALL libc_buffer_fini(struct buffer *__restrict buf);
INTDEF ssize_t LIBCCALL libc_buffer_flush(struct buffer *__restrict buf);
INTDEF ssize_t LIBCCALL libc_buffer_print(char const *__restrict data, size_t datalen, void *closure);
INTDEF void    LIBCCALL libc_w16buffer_init(struct w16buffer *__restrict self, pw16formatprinter printer, void *closure);
INTDEF ssize_t LIBCCALL libc_w16buffer_fini(struct w16buffer *__restrict buf);
INTDEF ssize_t LIBCCALL libc_w16buffer_flush(struct w16buffer *__restrict buf);
INTDEF ssize_t LIBCCALL libc_w16buffer_print(char16_t const *__restrict data, size_t datalen, void *closure);
INTDEF void    LIBCCALL libc_w32buffer_init(struct w32buffer *__restrict self, pw32formatprinter printer, void *closure);
INTDEF ssize_t LIBCCALL libc_w32buffer_fini(struct w32buffer *__restrict buf);
INTDEF ssize_t LIBCCALL libc_w32buffer_flush(struct w32buffer *__restrict buf);
INTDEF ssize_t LIBCCALL libc_w32buffer_print(char32_t const *__restrict data, size_t datalen, void *closure);

/* Conversion printers. */
INTDEF void LIBCCALL libc_w16printer_fini(struct w16printer *__restrict wp);
INTDEF void LIBCCALL libc_w32printer_fini(struct w32printer *__restrict wp);
INTDEF void LIBCCALL libc_w16printer_init(struct w16printer *__restrict wp, pw16formatprinter printer, void *closure);
INTDEF void LIBCCALL libc_w32printer_init(struct w32printer *__restrict wp, pw32formatprinter printer, void *closure);
INTDEF ssize_t LIBCCALL libc_w16printer_print(char const *__restrict data, size_t datalen, void *closure);
INTDEF ssize_t LIBCCALL libc_w32printer_print(char const *__restrict data, size_t datalen, void *closure);
INTDEF ssize_t LIBCCALL libc_Xw16printer_print(char const *__restrict data, size_t datalen, void *closure);
INTDEF ssize_t LIBCCALL libc_Xw32printer_print(char const *__restrict data, size_t datalen, void *closure);

/* Wide-character format helpers. */
INTDEF ssize_t (LIBCCALL libc_format_w16quote)(pw16formatprinter printer, void *closure, char16_t const *__restrict text, size_t textlen, u32 flags);
INTDEF ssize_t (LIBCCALL libc_format_w32quote)(pw32formatprinter printer, void *closure, char32_t const *__restrict text, size_t textlen, u32 flags);
INTDEF ssize_t (LIBCCALL libc_format_w16hexdump)(pw16formatprinter printer, void *closure, void const *__restrict data, size_t size, size_t linesize, u32 flags);
INTDEF ssize_t (LIBCCALL libc_format_w32hexdump)(pw32formatprinter printer, void *closure, void const *__restrict data, size_t size, size_t linesize, u32 flags);
INTDEF ssize_t  LIBCCALL libc_format_w16quote_l(pw16formatprinter printer, void *closure, char16_t const *__restrict text, size_t textlen, u32 flags, locale_t locale);
INTDEF ssize_t  LIBCCALL libc_format_w32quote_l(pw32formatprinter printer, void *closure, char32_t const *__restrict text, size_t textlen, u32 flags, locale_t locale);
INTDEF ssize_t  LIBCCALL libc_format_w16hexdump_l(pw16formatprinter printer, void *closure, void const *__restrict data, size_t size, size_t linesize, u32 flags, locale_t locale);
INTDEF ssize_t  LIBCCALL libc_format_w32hexdump_l(pw32formatprinter printer, void *closure, void const *__restrict data, size_t size, size_t linesize, u32 flags, locale_t locale);
#ifndef __OPTIMIZE_SIZE__
#define libc_format_w16quote(printer,closure,text,textlen,flags)         libc_format_w16quote_l(printer,closure,text,textlen,flags,NULL)
#define libc_format_w32quote(printer,closure,text,textlen,flags)         libc_format_w32quote_l(printer,closure,text,textlen,flags,NULL)
#define libc_format_w16hexdump(printer,closure,data,size,linesize,flags) libc_format_w16hexdump_l(printer,closure,data,size,linesize,flags,NULL)
#define libc_format_w32hexdump(printer,closure,data,size,linesize,flags) libc_format_w32hexdump_l(printer,closure,data,size,linesize,flags,NULL)
#endif

/* Wide-character format scanners. */
INTDEF ssize_t (ATTR_CDECL libc_format_w16scanf)(pw16formatgetc scanner, pw16formatungetc returnch, void *closure, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_format_w16scanf)(pw16formatgetc scanner, pw16formatungetc returnch, void *closure, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_format_w32scanf)(pw32formatgetc scanner, pw32formatungetc returnch, void *closure, char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_format_w32scanf)(pw32formatgetc scanner, pw32formatungetc returnch, void *closure, char32_t const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_format_vw16scanf)(pw16formatgetc scanner, pw16formatungetc returnch, void *closure, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_format_vw16scanf)(pw16formatgetc scanner, pw16formatungetc returnch, void *closure, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_format_vw32scanf)(pw32formatgetc scanner, pw32formatungetc returnch, void *closure, char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_format_vw32scanf)(pw32formatgetc scanner, pw32formatungetc returnch, void *closure, char32_t const *__restrict format, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_format_w16scanf_l(pw16formatgetc scanner, pw16formatungetc returnch, void *closure, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_w16scanf_l(pw16formatgetc scanner, pw16formatungetc returnch, void *closure, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_format_w32scanf_l(pw32formatgetc scanner, pw32formatungetc returnch, void *closure, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_w32scanf_l(pw32formatgetc scanner, pw32formatungetc returnch, void *closure, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_format_vw16scanf_l(pw16formatgetc scanner, pw16formatungetc returnch, void *closure, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vw16scanf_l(pw16formatgetc scanner, pw16formatungetc returnch, void *closure, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_format_vw32scanf_l(pw32formatgetc scanner, pw32formatungetc returnch, void *closure, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vw32scanf_l(pw32formatgetc scanner, pw32formatungetc returnch, void *closure, char32_t const *__restrict format, locale_t locale, va_list args);

/* Wide-character format printers. */
INTDEF ssize_t (ATTR_CDECL libc_format_w16printf)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_format_w16printf)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_format_w32printf)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_format_w32printf)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_format_vw16printf)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_format_vw16printf)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_format_vw32printf)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_format_vw32printf)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_format_w16printf_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_w16printf_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_format_w32printf_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_w32printf_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_format_vw16printf_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vw16printf_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_format_vw32printf_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vw32printf_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_format_w16printf(printer,closure,format,...)   libc_format_w16printf_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libd_format_w16printf(printer,closure,format,...)   libd_format_w16printf_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libc_format_w32printf(printer,closure,format,...)   libc_format_w32printf_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libd_format_w32printf(printer,closure,format,...)   libd_format_w32printf_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libc_format_vw16printf(printer,closure,format,args) libc_format_vw16printf_l(printer,closure,format,NULL,args)
#define libd_format_vw16printf(printer,closure,format,args) libd_format_vw16printf_l(printer,closure,format,NULL,args)
#define libc_format_vw32printf(printer,closure,format,args) libc_format_vw32printf_l(printer,closure,format,NULL,args)
#define libd_format_vw32printf(printer,closure,format,args) libd_format_vw32printf_l(printer,closure,format,NULL,args)
#endif /* !__OPTIMIZE_SIZE__ */

/* Format printers with support for positional arguments. */
INTDEF ssize_t (ATTR_CDECL libc_format_printf_p)(pformatprinter printer, void *closure, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_format_printf_p)(pformatprinter printer, void *closure, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_format_w16printf_p)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_format_w16printf_p)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_format_w32printf_p)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_format_w32printf_p)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_format_vprintf_p)(pformatprinter printer, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_format_vprintf_p)(pformatprinter printer, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_format_vw16printf_p)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_format_vw16printf_p)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_format_vw32printf_p)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_format_vw32printf_p)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_format_printf_p_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_printf_p_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_format_w16printf_p_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_w16printf_p_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_format_w32printf_p_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_w32printf_p_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_format_vprintf_p_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vprintf_p_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_format_vw16printf_p_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vw16printf_p_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_format_vw32printf_p_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vw32printf_p_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_format_printf_p(printer,closure,format,...)      libc_format_printf_p_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libd_format_printf_p(printer,closure,format,...)      libd_format_printf_p_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libc_format_w16printf_p(printer,closure,format,...)   libc_format_w16printf_p_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libd_format_w16printf_p(printer,closure,format,...)   libd_format_w16printf_p_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libc_format_w32printf_p(printer,closure,format,...)   libc_format_w32printf_p_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libd_format_w32printf_p(printer,closure,format,...)   libd_format_w32printf_p_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libc_format_vprintf_p(printer,closure,format,args)    libc_format_vprintf_p_l(printer,closure,format,NULL,args)
#define libd_format_vprintf_p(printer,closure,format,args)    libd_format_vprintf_p_l(printer,closure,format,NULL,args)
#define libc_format_vw16printf_p(printer,closure,format,args) libc_format_vw16printf_p_l(printer,closure,format,NULL,args)
#define libd_format_vw16printf_p(printer,closure,format,args) libd_format_vw16printf_p_l(printer,closure,format,NULL,args)
#define libc_format_vw32printf_p(printer,closure,format,args) libc_format_vw32printf_p_l(printer,closure,format,NULL,args)
#define libd_format_vw32printf_p(printer,closure,format,args) libd_format_vw32printf_p_l(printer,closure,format,NULL,args)
#endif /* !__OPTIMIZE_SIZE__ */

/* Format printers with an automatic, intermediate buffer. */
INTDEF ssize_t (ATTR_CDECL libc_format_bprintf)(pformatprinter printer, void *closure, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_format_bprintf)(pformatprinter printer, void *closure, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_format_w16bprintf)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_format_w16bprintf)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_format_w32bprintf)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_format_w32bprintf)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_format_vbprintf)(pformatprinter printer, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_format_vbprintf)(pformatprinter printer, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_format_vw16bprintf)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_format_vw16bprintf)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_format_vw32bprintf)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_format_vw32bprintf)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, va_list args);
INTDEF ssize_t (ATTR_CDECL libc_format_bprintf_p)(pformatprinter printer, void *closure, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_format_bprintf_p)(pformatprinter printer, void *closure, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_format_w16bprintf_p)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_format_w16bprintf_p)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_format_w32bprintf_p)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_format_w32bprintf_p)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_format_vbprintf_p)(pformatprinter printer, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_format_vbprintf_p)(pformatprinter printer, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_format_vw16bprintf_p)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_format_vw16bprintf_p)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_format_vw32bprintf_p)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_format_vw32bprintf_p)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_format_bprintf_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_bprintf_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_format_w16bprintf_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_w16bprintf_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_format_w32bprintf_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_w32bprintf_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_format_vbprintf_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vbprintf_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_format_vw16bprintf_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vw16bprintf_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_format_vw32bprintf_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vw32bprintf_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_format_bprintf_p_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_bprintf_p_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_format_w16bprintf_p_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_w16bprintf_p_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_format_w32bprintf_p_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_w32bprintf_p_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_format_vbprintf_p_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vbprintf_p_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_format_vw16bprintf_p_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vw16bprintf_p_l(pw16formatprinter printer, void *closure, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_format_vw32bprintf_p_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vw32bprintf_p_l(pw32formatprinter printer, void *closure, char32_t const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_format_bprintf(printer,closure,format,...)        libc_format_bprintf_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libd_format_bprintf(printer,closure,format,...)        libd_format_bprintf_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libc_format_w16bprintf(printer,closure,format,...)     libc_format_w16bprintf_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libd_format_w16bprintf(printer,closure,format,...)     libd_format_w16bprintf_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libc_format_w32bprintf(printer,closure,format,...)     libc_format_w32bprintf_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libd_format_w32bprintf(printer,closure,format,...)     libd_format_w32bprintf_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libc_format_vbprintf(printer,closure,format,args)      libc_format_vbprintf_l(printer,closure,format,NULL,args)
#define libd_format_vbprintf(printer,closure,format,args)      libd_format_vbprintf_l(printer,closure,format,NULL,args)
#define libc_format_vw16bprintf(printer,closure,format,args)   libc_format_vw16bprintf_l(printer,closure,format,NULL,args)
#define libd_format_vw16bprintf(printer,closure,format,args)   libd_format_vw16bprintf_l(printer,closure,format,NULL,args)
#define libc_format_vw32bprintf(printer,closure,format,args)   libc_format_vw32bprintf_l(printer,closure,format,NULL,args)
#define libd_format_vw32bprintf(printer,closure,format,args)   libd_format_vw32bprintf_l(printer,closure,format,NULL,args)
#define libc_format_bprintf_p(printer,closure,format,...)      libc_format_bprintf_p_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libd_format_bprintf_p(printer,closure,format,...)      libd_format_bprintf_p_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libc_format_w16bprintf_p(printer,closure,format,...)   libc_format_w16bprintf_p_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libd_format_w16bprintf_p(printer,closure,format,...)   libd_format_w16bprintf_p_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libc_format_w32bprintf_p(printer,closure,format,...)   libc_format_w32bprintf_p_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libd_format_w32bprintf_p(printer,closure,format,...)   libd_format_w32bprintf_p_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libc_format_vbprintf_p(printer,closure,format,args)    libc_format_vbprintf_p_l(printer,closure,format,NULL,args)
#define libd_format_vbprintf_p(printer,closure,format,args)    libd_format_vbprintf_p_l(printer,closure,format,NULL,args)
#define libc_format_vw16bprintf_p(printer,closure,format,args) libc_format_vw16bprintf_p_l(printer,closure,format,NULL,args)
#define libd_format_vw16bprintf_p(printer,closure,format,args) libd_format_vw16bprintf_p_l(printer,closure,format,NULL,args)
#define libc_format_vw32bprintf_p(printer,closure,format,args) libc_format_vw32bprintf_p_l(printer,closure,format,NULL,args)
#define libd_format_vw32bprintf_p(printer,closure,format,args) libd_format_vw32bprintf_p_l(printer,closure,format,NULL,args)
#endif /* !__OPTIMIZE_SIZE__ */


INTDEF size_t (ATTR_CDECL libc_sprintf)(char *__restrict buf, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_sprintf)(char *__restrict buf, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_sw16printf)(char16_t *__restrict buf, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_sw16printf)(char16_t *__restrict buf, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_sw32printf)(char32_t *__restrict buf, char32_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_sw32printf)(char32_t *__restrict buf, char32_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_sprintf_p)(char *__restrict buf, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_sprintf_p)(char *__restrict buf, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_sw16printf_p)(char16_t *__restrict buf, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_sw16printf_p)(char16_t *__restrict buf, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_sw32printf_p)(char32_t *__restrict buf, char32_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_sw32printf_p)(char32_t *__restrict buf, char32_t const *__restrict format, ...);
INTDEF size_t (LIBCCALL   libc_vsprintf)(char *__restrict buf, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsprintf)(char *__restrict buf, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libc_vsw16printf)(char16_t *__restrict buf, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsw16printf)(char16_t *__restrict buf, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libc_vsw32printf)(char32_t *__restrict buf, char32_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsw32printf)(char32_t *__restrict buf, char32_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libc_vsprintf_p)(char *__restrict buf, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsprintf_p)(char *__restrict buf, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libc_vsw16printf_p)(char16_t *__restrict buf, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsw16printf_p)(char16_t *__restrict buf, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libc_vsw32printf_p)(char32_t *__restrict buf, char32_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsw32printf_p)(char32_t *__restrict buf, char32_t const *__restrict format, va_list args);
INTDEF size_t  ATTR_CDECL libc_sprintf_l(char *__restrict buf, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_sprintf_l(char *__restrict buf, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_sw16printf_l(char16_t *__restrict buf, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_sw16printf_l(char16_t *__restrict buf, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_sw32printf_l(char32_t *__restrict buf, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_sw32printf_l(char32_t *__restrict buf, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_sprintf_p_l(char *__restrict buf, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_sprintf_p_l(char *__restrict buf, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_sw16printf_p_l(char16_t *__restrict buf, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_sw16printf_p_l(char16_t *__restrict buf, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_sw32printf_p_l(char32_t *__restrict buf, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_sw32printf_p_l(char32_t *__restrict buf, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  LIBCCALL   libc_vsprintf_l(char *__restrict buf, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsprintf_l(char *__restrict buf, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libc_vsw16printf_l(char16_t *__restrict buf, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsw16printf_l(char16_t *__restrict buf, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libc_vsw32printf_l(char32_t *__restrict buf, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsw32printf_l(char32_t *__restrict buf, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libc_vsprintf_p_l(char *__restrict buf, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsprintf_p_l(char *__restrict buf, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libc_vsw16printf_p_l(char16_t *__restrict buf, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsw16printf_p_l(char16_t *__restrict buf, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libc_vsw32printf_p_l(char32_t *__restrict buf, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsw32printf_p_l(char32_t *__restrict buf, char32_t const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_sprintf(buf,format,...)        libc_sprintf_l(buf,format,NULL,##__VA_ARGS__)
#define libd_sprintf(buf,format,...)        libd_sprintf_l(buf,format,NULL,##__VA_ARGS__)
#define libc_sw16printf(buf,format,...)     libc_sw16printf_l(buf,format,NULL,##__VA_ARGS__)
#define libd_sw16printf(buf,format,...)     libd_sw16printf_l(buf,format,NULL,##__VA_ARGS__)
#define libc_sw32printf(buf,format,...)     libc_sw32printf_l(buf,format,NULL,##__VA_ARGS__)
#define libd_sw32printf(buf,format,...)     libd_sw32printf_l(buf,format,NULL,##__VA_ARGS__)
#define libc_sprintf_p(buf,format,...)      libc_sprintf_p_l(buf,format,NULL,##__VA_ARGS__)
#define libd_sprintf_p(buf,format,...)      libd_sprintf_p_l(buf,format,NULL,##__VA_ARGS__)
#define libc_sw16printf_p(buf,format,...)   libc_sw16printf_p_l(buf,format,NULL,##__VA_ARGS__)
#define libd_sw16printf_p(buf,format,...)   libd_sw16printf_p_l(buf,format,NULL,##__VA_ARGS__)
#define libc_sw32printf_p(buf,format,...)   libc_sw32printf_p_l(buf,format,NULL,##__VA_ARGS__)
#define libd_sw32printf_p(buf,format,...)   libd_sw32printf_p_l(buf,format,NULL,##__VA_ARGS__)
#define libc_vsprintf(buf,format,args)      libc_vsprintf_l(buf,format,NULL,args)
#define libd_vsprintf(buf,format,args)      libd_vsprintf_l(buf,format,NULL,args)
#define libc_vsw16printf(buf,format,args)   libc_vsw16printf_l(buf,format,NULL,args)
#define libd_vsw16printf(buf,format,args)   libd_vsw16printf_l(buf,format,NULL,args)
#define libc_vsw32printf(buf,format,args)   libc_vsw32printf_l(buf,format,NULL,args)
#define libd_vsw32printf(buf,format,args)   libd_vsw32printf_l(buf,format,NULL,args)
#define libc_vsprintf_p(buf,format,args)    libc_vsprintf_p_l(buf,format,NULL,args)
#define libd_vsprintf_p(buf,format,args)    libd_vsprintf_p_l(buf,format,NULL,args)
#define libc_vsw16printf_p(buf,format,args) libc_vsw16printf_p_l(buf,format,NULL,args)
#define libd_vsw16printf_p(buf,format,args) libd_vsw16printf_p_l(buf,format,NULL,args)
#define libc_vsw32printf_p(buf,format,args) libc_vsw32printf_p_l(buf,format,NULL,args)
#define libd_vsw32printf_p(buf,format,args) libd_vsw32printf_p_l(buf,format,NULL,args)
#endif /* !__OPTIMIZE_SIZE__ */

INTDEF size_t (ATTR_CDECL libc_snprintf)(char *__restrict buf, size_t buflen, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_snprintf)(char *__restrict buf, size_t buflen, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_snw16printf)(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_snw16printf)(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_snw32printf)(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_snw32printf)(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, ...);
INTDEF size_t (LIBCCALL   libc_vsnprintf)(char *__restrict buf, size_t buflen, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsnprintf)(char *__restrict buf, size_t buflen, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libc_vsnw16printf)(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsnw16printf)(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libc_vsnw32printf)(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsnw32printf)(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, va_list args);
INTDEF size_t (ATTR_CDECL libc_snprintf_p)(char *__restrict buf, size_t buflen, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_snprintf_p)(char *__restrict buf, size_t buflen, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_snw16printf_p)(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_snw16printf_p)(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_snw32printf_p)(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_snw32printf_p)(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, ...);
INTDEF size_t (LIBCCALL   libc_vsnprintf_p)(char *__restrict buf, size_t buflen, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsnprintf_p)(char *__restrict buf, size_t buflen, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libc_vsnw16printf_p)(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsnw16printf_p)(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libc_vsnw32printf_p)(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsnw32printf_p)(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, va_list args);
INTDEF size_t  ATTR_CDECL libc_snprintf_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_snprintf_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_snw16printf_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_snw16printf_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_snw32printf_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_snw32printf_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  LIBCCALL   libc_vsnprintf_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsnprintf_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libc_vsnw16printf_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsnw16printf_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libc_vsnw32printf_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsnw32printf_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  ATTR_CDECL libc_snprintf_p_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_snprintf_p_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_snw16printf_p_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_snw16printf_p_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_snw32printf_p_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_snw32printf_p_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  LIBCCALL   libc_vsnprintf_p_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsnprintf_p_l(char *__restrict buf, size_t buflen, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libc_vsnw16printf_p_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsnw16printf_p_l(char16_t *__restrict buf, size_t buflen, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libc_vsnw32printf_p_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsnw32printf_p_l(char32_t *__restrict buf, size_t buflen, char32_t const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_snprintf(buf,buflen,format,...)        libc_snprintf_l(buf,buflen,format,NULL,##__VA_ARGS__)
#define libd_snprintf(buf,buflen,format,...)        libd_snprintf_l(buf,buflen,format,NULL,##__VA_ARGS__)
#define libc_snw16printf(buf,buflen,format,...)     libc_snw16printf_l(buf,buflen,format,NULL,##__VA_ARGS__)
#define libd_snw16printf(buf,buflen,format,...)     libd_snw16printf_l(buf,buflen,format,NULL,##__VA_ARGS__)
#define libc_snw32printf(buf,buflen,format,...)     libc_snw32printf_l(buf,buflen,format,NULL,##__VA_ARGS__)
#define libd_snw32printf(buf,buflen,format,...)     libd_snw32printf_l(buf,buflen,format,NULL,##__VA_ARGS__)
#define libc_vsnprintf(buf,buflen,format,args)      libc_vsnprintf_l(buf,buflen,format,NULL,args)
#define libd_vsnprintf(buf,buflen,format,args)      libd_vsnprintf_l(buf,buflen,format,NULL,args)
#define libc_vsnw16printf(buf,buflen,format,args)   libc_vsnw16printf_l(buf,buflen,format,NULL,args)
#define libd_vsnw16printf(buf,buflen,format,args)   libd_vsnw16printf_l(buf,buflen,format,NULL,args)
#define libc_vsnw32printf(buf,buflen,format,args)   libc_vsnw32printf_l(buf,buflen,format,NULL,args)
#define libd_vsnw32printf(buf,buflen,format,args)   libd_vsnw32printf_l(buf,buflen,format,NULL,args)
#define libc_snprintf_p(buf,buflen,format,...)      libc_snprintf_p_l(buf,buflen,format,NULL,##__VA_ARGS__)
#define libd_snprintf_p(buf,buflen,format,...)      libd_snprintf_p_l(buf,buflen,format,NULL,##__VA_ARGS__)
#define libc_snw16printf_p(buf,buflen,format,...)   libc_snw16printf_p_l(buf,buflen,format,NULL,##__VA_ARGS__)
#define libd_snw16printf_p(buf,buflen,format,...)   libd_snw16printf_p_l(buf,buflen,format,NULL,##__VA_ARGS__)
#define libc_snw32printf_p(buf,buflen,format,...)   libc_snw32printf_p_l(buf,buflen,format,NULL,##__VA_ARGS__)
#define libd_snw32printf_p(buf,buflen,format,...)   libd_snw32printf_p_l(buf,buflen,format,NULL,##__VA_ARGS__)
#define libc_vsnprintf_p(buf,buflen,format,args)    libc_vsnprintf_p_l(buf,buflen,format,NULL,args)
#define libd_vsnprintf_p(buf,buflen,format,args)    libd_vsnprintf_p_l(buf,buflen,format,NULL,args)
#define libc_vsnw16printf_p(buf,buflen,format,args) libc_vsnw16printf_p_l(buf,buflen,format,NULL,args)
#define libd_vsnw16printf_p(buf,buflen,format,args) libd_vsnw16printf_p_l(buf,buflen,format,NULL,args)
#define libc_vsnw32printf_p(buf,buflen,format,args) libc_vsnw32printf_p_l(buf,buflen,format,NULL,args)
#define libd_vsnw32printf_p(buf,buflen,format,args) libd_vsnw32printf_p_l(buf,buflen,format,NULL,args)
#endif /* !__OPTIMIZE_SIZE__ */
/* Weird functions required for DOS compatibility */
INTDEF size_t (ATTR_CDECL libc_snprintf_c)(char *buf, size_t buflen, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_snprintf_c)(char *buf, size_t buflen, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_snw16printf_c)(char16_t *buf, size_t buflen, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_snw16printf_c)(char16_t *buf, size_t buflen, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_snw32printf_c)(char32_t *buf, size_t buflen, char32_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_snw32printf_c)(char32_t *buf, size_t buflen, char32_t const *__restrict format, ...);
INTDEF size_t (LIBCCALL libc_vsnprintf_c)(char *buf, size_t buflen, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libd_vsnprintf_c)(char *buf, size_t buflen, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libc_vsnw16printf_c)(char16_t *buf, size_t buflen, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libd_vsnw16printf_c)(char16_t *buf, size_t buflen, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libc_vsnw32printf_c)(char32_t *buf, size_t buflen, char32_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libd_vsnw32printf_c)(char32_t *buf, size_t buflen, char32_t const *__restrict format, va_list args);
INTDEF size_t  ATTR_CDECL libc_snprintf_c_l(char *buf, size_t buflen, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_snprintf_c_l(char *buf, size_t buflen, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_snw16printf_c_l(char16_t *buf, size_t buflen, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_snw16printf_c_l(char16_t *buf, size_t buflen, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_snw32printf_c_l(char32_t *buf, size_t buflen, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_snw32printf_c_l(char32_t *buf, size_t buflen, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  LIBCCALL libc_vsnprintf_c_l(char *buf, size_t buflen, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL libd_vsnprintf_c_l(char *buf, size_t buflen, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL libc_vsnw16printf_c_l(char16_t *buf, size_t buflen, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL libd_vsnw16printf_c_l(char16_t *buf, size_t buflen, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL libc_vsnw32printf_c_l(char32_t *buf, size_t buflen, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL libd_vsnw32printf_c_l(char32_t *buf, size_t buflen, char32_t const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_snprintf_c(buf,buflen,format,...)      libc_snprintf_c_l(buf,buflen,format,NULL,__VA_ARGS__)
#define libd_snprintf_c(buf,buflen,format,...)      libd_snprintf_c_l(buf,buflen,format,NULL,__VA_ARGS__)
#define libc_snw16printf_c(buf,buflen,format,...)   libc_snw16printf_c_l(buf,buflen,format,NULL,__VA_ARGS__)
#define libd_snw16printf_c(buf,buflen,format,...)   libd_snw16printf_c_l(buf,buflen,format,NULL,__VA_ARGS__)
#define libc_snw32printf_c(buf,buflen,format,...)   libc_snw32printf_c_l(buf,buflen,format,NULL,__VA_ARGS__)
#define libd_snw32printf_c(buf,buflen,format,...)   libd_snw32printf_c_l(buf,buflen,format,NULL,__VA_ARGS__)
#define libc_vsnprintf_c(buf,buflen,format,args)    libc_vsnprintf_c_l(buf,buflen,format,NULL,args)
#define libd_vsnprintf_c(buf,buflen,format,args)    libd_vsnprintf_c_l(buf,buflen,format,NULL,args)
#define libc_vsnw16printf_c(buf,buflen,format,args) libc_vsnw16printf_c_l(buf,buflen,format,NULL,args)
#define libd_vsnw16printf_c(buf,buflen,format,args) libd_vsnw16printf_c_l(buf,buflen,format,NULL,args)
#define libc_vsnw32printf_c(buf,buflen,format,args) libc_vsnw32printf_c_l(buf,buflen,format,NULL,args)
#define libd_vsnw32printf_c(buf,buflen,format,args) libd_vsnw32printf_c_l(buf,buflen,format,NULL,args)
#endif

/* DOS's count-format functions (simply return how much characters are used by the format string) */
INTDEF size_t (ATTR_CDECL libc_scprintf)(char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_scprintf)(char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_scw16printf)(char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_scw16printf)(char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_scw32printf)(char32_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_scw32printf)(char32_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_scprintf_p)(char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_scprintf_p)(char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_scw16printf_p)(char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_scw16printf_p)(char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_scw32printf_p)(char32_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_scw32printf_p)(char32_t const *__restrict format, ...);
INTDEF size_t (LIBCCALL libc_vscprintf)(char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libd_vscprintf)(char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libc_vscw16printf)(char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libd_vscw16printf)(char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libc_vscw32printf)(char32_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libd_vscw32printf)(char32_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libc_vscprintf_p)(char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libd_vscprintf_p)(char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libc_vscw16printf_p)(char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libd_vscw16printf_p)(char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libc_vscw32printf_p)(char32_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL libd_vscw32printf_p)(char32_t const *__restrict format, va_list args);
INTDEF size_t ATTR_CDECL libc_scprintf_l(char const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libd_scprintf_l(char const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libc_scw16printf_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libd_scw16printf_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libc_scw32printf_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libd_scw32printf_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libc_scprintf_p_l(char const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libd_scprintf_p_l(char const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libc_scw16printf_p_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libd_scw16printf_p_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libc_scw32printf_p_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t ATTR_CDECL libd_scw32printf_p_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t LIBCCALL libc_vscprintf_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t LIBCCALL libd_vscprintf_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t LIBCCALL libc_vscw16printf_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t LIBCCALL libd_vscw16printf_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t LIBCCALL libc_vscw32printf_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t LIBCCALL libd_vscw32printf_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t LIBCCALL libc_vscprintf_p_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t LIBCCALL libd_vscprintf_p_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t LIBCCALL libc_vscw16printf_p_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t LIBCCALL libd_vscw16printf_p_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t LIBCCALL libc_vscw32printf_p_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t LIBCCALL libd_vscw32printf_p_l(char32_t const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_scprintf(format,...)        libc_scprintf_l(format,NULL,__VA_ARGS__)
#define libd_scprintf(format,...)        libd_scprintf_l(format,NULL,__VA_ARGS__)
#define libc_scw16printf(format,...)     libc_scw16printf_l(format,NULL,__VA_ARGS__)
#define libd_scw16printf(format,...)     libd_scw16printf_l(format,NULL,__VA_ARGS__)
#define libc_scw32printf(format,...)     libc_scw32printf_l(format,NULL,__VA_ARGS__)
#define libd_scw32printf(format,...)     libd_scw32printf_l(format,NULL,__VA_ARGS__)
#define libc_scprintf_p(format,...)      libc_scprintf_p_l(format,NULL,__VA_ARGS__)
#define libd_scprintf_p(format,...)      libd_scprintf_p_l(format,NULL,__VA_ARGS__)
#define libc_scw16printf_p(format,...)   libc_scw16printf_p_l(format,NULL,__VA_ARGS__)
#define libd_scw16printf_p(format,...)   libd_scw16printf_p_l(format,NULL,__VA_ARGS__)
#define libc_scw32printf_p(format,...)   libc_scw32printf_p_l(format,NULL,__VA_ARGS__)
#define libd_scw32printf_p(format,...)   libd_scw32printf_p_l(format,NULL,__VA_ARGS__)
#define libc_vscprintf(format,args)      libc_vscprintf_l(format,NULL,args)
#define libd_vscprintf(format,args)      libd_vscprintf_l(format,NULL,args)
#define libc_vscw16printf(format,args)   libc_vscw16printf_l(format,NULL,args)
#define libd_vscw16printf(format,args)   libd_vscw16printf_l(format,NULL,args)
#define libc_vscw32printf(format,args)   libc_vscw32printf_l(format,NULL,args)
#define libd_vscw32printf(format,args)   libd_vscw32printf_l(format,NULL,args)
#define libc_vscprintf_p(format,args)    libc_vscprintf_p_l(format,NULL,args)
#define libd_vscprintf_p(format,args)    libd_vscprintf_p_l(format,NULL,args)
#define libc_vscw16printf_p(format,args) libc_vscw16printf_p_l(format,NULL,args)
#define libd_vscw16printf_p(format,args) libd_vscw16printf_p_l(format,NULL,args)
#define libc_vscw32printf_p(format,args) libc_vscw32printf_p_l(format,NULL,args)
#define libd_vscw32printf_p(format,args) libd_vscw32printf_p_l(format,NULL,args)
#endif
#ifndef LIBC_DPRINTF_IS_BUFFERED
#define LIBC_DPRINTF_IS_BUFFERED 1 /* Define to an expression that is checked at runtime
                                    * to see if `libc_dprintf()' should be buffered. */
#endif
INTDEF ssize_t (ATTR_CDECL libc_dprintf)(fd_t fd, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_dprintf)(fd_t fd, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_w16dprintf)(fd_t fd, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_w16dprintf)(fd_t fd, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_w32dprintf)(fd_t fd, char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_w32dprintf)(fd_t fd, char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_dprintf_p)(fd_t fd, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_dprintf_p)(fd_t fd, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_w16dprintf_p)(fd_t fd, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_w16dprintf_p)(fd_t fd, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_w32dprintf_p)(fd_t fd, char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_w32dprintf_p)(fd_t fd, char32_t const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_vdprintf)(fd_t fd, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vdprintf)(fd_t fd, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vw16dprintf)(fd_t fd, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vw16dprintf)(fd_t fd, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vw32dprintf)(fd_t fd, char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vw32dprintf)(fd_t fd, char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vdprintf_p)(fd_t fd, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vdprintf_p)(fd_t fd, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vw16dprintf_p)(fd_t fd, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vw16dprintf_p)(fd_t fd, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vw32dprintf_p)(fd_t fd, char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vw32dprintf_p)(fd_t fd, char32_t const *__restrict format, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_dprintf_l(fd_t fd, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_dprintf_l(fd_t fd, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_w16dprintf_l(fd_t fd, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_w16dprintf_l(fd_t fd, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_w32dprintf_l(fd_t fd, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_w32dprintf_l(fd_t fd, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_dprintf_p_l(fd_t fd, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_dprintf_p_l(fd_t fd, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_w16dprintf_p_l(fd_t fd, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_w16dprintf_p_l(fd_t fd, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_w32dprintf_p_l(fd_t fd, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_w32dprintf_p_l(fd_t fd, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_vdprintf_l(fd_t fd, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vdprintf_l(fd_t fd, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vw16dprintf_l(fd_t fd, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vw16dprintf_l(fd_t fd, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vw32dprintf_l(fd_t fd, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vw32dprintf_l(fd_t fd, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vdprintf_p_l(fd_t fd, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vdprintf_p_l(fd_t fd, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vw16dprintf_p_l(fd_t fd, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vw16dprintf_p_l(fd_t fd, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vw32dprintf_p_l(fd_t fd, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vw32dprintf_p_l(fd_t fd, char32_t const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_dprintf(fd,format,...)        libc_dprintf_l(fd,format,NULL,##__VA_ARGS__)
#define libd_dprintf(fd,format,...)        libd_dprintf_l(fd,format,NULL,##__VA_ARGS__)
#define libc_w16dprintf(fd,format,...)     libc_w16dprintf_l(fd,format,NULL,##__VA_ARGS__)
#define libd_w16dprintf(fd,format,...)     libd_w16dprintf_l(fd,format,NULL,##__VA_ARGS__)
#define libc_w32dprintf(fd,format,...)     libc_w32dprintf_l(fd,format,NULL,##__VA_ARGS__)
#define libd_w32dprintf(fd,format,...)     libd_w32dprintf_l(fd,format,NULL,##__VA_ARGS__)
#define libc_dprintf_p(fd,format,...)      libc_dprintf_p_l(fd,format,NULL,##__VA_ARGS__)
#define libd_dprintf_p(fd,format,...)      libd_dprintf_p_l(fd,format,NULL,##__VA_ARGS__)
#define libc_w16dprintf_p(fd,format,...)   libc_w16dprintf_p_l(fd,format,NULL,##__VA_ARGS__)
#define libd_w16dprintf_p(fd,format,...)   libd_w16dprintf_p_l(fd,format,NULL,##__VA_ARGS__)
#define libc_w32dprintf_p(fd,format,...)   libc_w32dprintf_p_l(fd,format,NULL,##__VA_ARGS__)
#define libd_w32dprintf_p(fd,format,...)   libd_w32dprintf_p_l(fd,format,NULL,##__VA_ARGS__)
#define libc_vdprintf(fd,format,args)      libc_vdprintf_l(fd,format,NULL,args)
#define libd_vdprintf(fd,format,args)      libd_vdprintf_l(fd,format,NULL,args)
#define libc_vw16dprintf(fd,format,args)   libc_vw16dprintf_l(fd,format,NULL,args)
#define libd_vw16dprintf(fd,format,args)   libd_vw16dprintf_l(fd,format,NULL,args)
#define libc_vw32dprintf(fd,format,args)   libc_vw32dprintf_l(fd,format,NULL,args)
#define libd_vw32dprintf(fd,format,args)   libd_vw32dprintf_l(fd,format,NULL,args)
#define libc_vdprintf_p(fd,format,args)    libc_vdprintf_p_l(fd,format,NULL,args)
#define libd_vdprintf_p(fd,format,args)    libd_vdprintf_p_l(fd,format,NULL,args)
#define libc_vw16dprintf_p(fd,format,args) libc_vw16dprintf_p_l(fd,format,NULL,args)
#define libd_vw16dprintf_p(fd,format,args) libd_vw16dprintf_p_l(fd,format,NULL,args)
#define libc_vw32dprintf_p(fd,format,args) libc_vw32dprintf_p_l(fd,format,NULL,args)
#define libd_vw32dprintf_p(fd,format,args) libd_vw32dprintf_p_l(fd,format,NULL,args)
#endif /* !__OPTIMIZE_SIZE__ */

INTDEF ssize_t (ATTR_CDECL libc_asprintf)(char **__restrict ptr, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_asprintf)(char **__restrict ptr, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_asprintf_p)(char **__restrict ptr, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_asprintf_p)(char **__restrict ptr, char const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_vasprintf)(char **__restrict ptr, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vasprintf)(char **__restrict ptr, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vasprintf_p)(char **__restrict ptr, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vasprintf_p)(char **__restrict ptr, char const *__restrict format, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_asprintf_l(char **__restrict ptr, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_asprintf_l(char **__restrict ptr, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_asprintf_p_l(char **__restrict ptr, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_asprintf_p_l(char **__restrict ptr, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_vasprintf_l(char **__restrict ptr, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vasprintf_l(char **__restrict ptr, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vasprintf_p_l(char **__restrict ptr, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vasprintf_p_l(char **__restrict ptr, char const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_asprintf(ptr,format,...)     libc_asprintf_l(ptr,format,NULL,##__VA_ARGS__)
#define libd_asprintf(ptr,format,...)     libd_asprintf_l(ptr,format,NULL,##__VA_ARGS__)
#define libc_asprintf_p(ptr,format,...)   libc_asprintf_p_l(ptr,format,NULL,##__VA_ARGS__)
#define libd_asprintf_p(ptr,format,...)   libd_asprintf_p_l(ptr,format,NULL,##__VA_ARGS__)
#define libc_vasprintf(ptr,format,args)   libc_vasprintf_l(ptr,format,NULL,args)
#define libd_vasprintf(ptr,format,args)   libd_vasprintf_l(ptr,format,NULL,args)
#define libc_vasprintf_p(ptr,format,args) libc_vasprintf_p_l(ptr,format,NULL,args)
#define libd_vasprintf_p(ptr,format,args) libd_vasprintf_p_l(ptr,format,NULL,args)
#endif /* !__OPTIMIZE_SIZE__ */

INTDEF ATTR_MALLOC char *(ATTR_CDECL libc_strdupf)(char const *__restrict format, ...);
INTDEF ATTR_MALLOC char *(ATTR_CDECL libd_strdupf)(char const *__restrict format, ...);
INTDEF ATTR_MALLOC char *(ATTR_CDECL libc_strdupf_p)(char const *__restrict format, ...);
INTDEF ATTR_MALLOC char *(ATTR_CDECL libd_strdupf_p)(char const *__restrict format, ...);
INTDEF ATTR_MALLOC char *(LIBCCALL   libc_vstrdupf)(char const *__restrict format, va_list args);
INTDEF ATTR_MALLOC char *(LIBCCALL   libd_vstrdupf)(char const *__restrict format, va_list args);
INTDEF ATTR_MALLOC char *(LIBCCALL   libc_vstrdupf_p)(char const *__restrict format, va_list args);
INTDEF ATTR_MALLOC char *(LIBCCALL   libd_vstrdupf_p)(char const *__restrict format, va_list args);
INTDEF ATTR_MALLOC char  *ATTR_CDECL libc_strdupf_l(char const *__restrict format, locale_t locale, ...);
INTDEF ATTR_MALLOC char  *ATTR_CDECL libd_strdupf_l(char const *__restrict format, locale_t locale, ...);
INTDEF ATTR_MALLOC char  *ATTR_CDECL libc_strdupf_p_l(char const *__restrict format, locale_t locale, ...);
INTDEF ATTR_MALLOC char  *ATTR_CDECL libd_strdupf_p_l(char const *__restrict format, locale_t locale, ...);
INTDEF ATTR_MALLOC char  *LIBCCALL   libc_vstrdupf_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF ATTR_MALLOC char  *LIBCCALL   libd_vstrdupf_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF ATTR_MALLOC char  *LIBCCALL   libc_vstrdupf_p_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF ATTR_MALLOC char  *LIBCCALL   libd_vstrdupf_p_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *(ATTR_CDECL libc_Xstrdupf)(char const *__restrict format, ...);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *(ATTR_CDECL libd_Xstrdupf)(char const *__restrict format, ...);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *(ATTR_CDECL libc_Xstrdupf_p)(char const *__restrict format, ...);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *(ATTR_CDECL libd_Xstrdupf_p)(char const *__restrict format, ...);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *(LIBCCALL   libc_Xvstrdupf)(char const *__restrict format, va_list args);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *(LIBCCALL   libd_Xvstrdupf)(char const *__restrict format, va_list args);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *(LIBCCALL   libc_Xvstrdupf_p)(char const *__restrict format, va_list args);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char *(LIBCCALL   libd_Xvstrdupf_p)(char const *__restrict format, va_list args);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char  *ATTR_CDECL libc_Xstrdupf_l(char const *__restrict format, locale_t locale, ...);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char  *ATTR_CDECL libd_Xstrdupf_l(char const *__restrict format, locale_t locale, ...);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char  *ATTR_CDECL libc_Xstrdupf_p_l(char const *__restrict format, locale_t locale, ...);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char  *ATTR_CDECL libd_Xstrdupf_p_l(char const *__restrict format, locale_t locale, ...);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char  *LIBCCALL   libc_Xvstrdupf_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char  *LIBCCALL   libd_Xvstrdupf_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char  *LIBCCALL   libc_Xvstrdupf_p_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF ATTR_RETNONNULL ATTR_MALLOC char  *LIBCCALL   libd_Xvstrdupf_p_l(char const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_strdupf(format,...)      libc_strdupf_l(format,NULL,##__VA_ARGS__)
#define libd_strdupf(format,...)      libd_strdupf_l(format,NULL,##__VA_ARGS__)
#define libc_strdupf_p(format,...)    libc_strdupf_p_l(format,NULL,##__VA_ARGS__)
#define libd_strdupf_p(format,...)    libd_strdupf_p_l(format,NULL,##__VA_ARGS__)
#define libc_vstrdupf(format,args)    libc_vstrdupf_l(format,NULL,args)
#define libd_vstrdupf(format,args)    libd_vstrdupf_l(format,NULL,args)
#define libc_vstrdupf_p(format,args)  libc_vstrdupf_p_l(format,NULL,args)
#define libd_vstrdupf_p(format,args)  libd_vstrdupf_p_l(format,NULL,args)
#define libc_Xstrdupf(format,...)     libc_Xstrdupf_l(format,NULL,##__VA_ARGS__)
#define libd_Xstrdupf(format,...)     libd_Xstrdupf_l(format,NULL,##__VA_ARGS__)
#define libc_Xstrdupf_p(format,...)   libc_Xstrdupf_p_l(format,NULL,##__VA_ARGS__)
#define libd_Xstrdupf_p(format,...)   libd_Xstrdupf_p_l(format,NULL,##__VA_ARGS__)
#define libc_Xvstrdupf(format,args)   libc_Xvstrdupf_l(format,NULL,args)
#define libd_Xvstrdupf(format,args)   libd_Xvstrdupf_l(format,NULL,args)
#define libc_Xvstrdupf_p(format,args) libc_Xvstrdupf_p_l(format,NULL,args)
#define libd_Xvstrdupf_p(format,args) libd_Xvstrdupf_p_l(format,NULL,args)
#endif /* !__OPTIMIZE_SIZE__ */

INTDEF size_t (ATTR_CDECL libc_sscanf)(char const *__restrict src, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_sscanf)(char const *__restrict src, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_sw16scanf)(char16_t const *__restrict src, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_sw16scanf)(char16_t const *__restrict src, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_sw32scanf)(char32_t const *__restrict src, char32_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_sw32scanf)(char32_t const *__restrict src, char32_t const *__restrict format, ...);
INTDEF size_t (LIBCCALL   libc_vsscanf)(char const *__restrict src, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsscanf)(char const *__restrict src, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libc_vsw16scanf)(char16_t const *__restrict src, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsw16scanf)(char16_t const *__restrict src, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libc_vsw32scanf)(char32_t const *__restrict src, char32_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsw32scanf)(char32_t const *__restrict src, char32_t const *__restrict format, va_list args);
INTDEF size_t  ATTR_CDECL libc_sscanf_l(char const *__restrict src, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_sscanf_l(char const *__restrict src, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_sw16scanf_l(char16_t const *__restrict src, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_sw16scanf_l(char16_t const *__restrict src, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_sw32scanf_l(char32_t const *__restrict src, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_sw32scanf_l(char32_t const *__restrict src, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  LIBCCALL   libc_vsscanf_l(char const *__restrict src, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsscanf_l(char const *__restrict src, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libc_vsw16scanf_l(char16_t const *__restrict src, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsw16scanf_l(char16_t const *__restrict src, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libc_vsw32scanf_l(char32_t const *__restrict src, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsw32scanf_l(char32_t const *__restrict src, char32_t const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_sscanf(src,format,...)      libc_sscanf_l(src,format,NULL,##__VA_ARGS__)
#define libd_sscanf(src,format,...)      libd_sscanf_l(src,format,NULL,##__VA_ARGS__)
#define libc_sw16scanf(src,format,...)   libc_sw16scanf_l(src,format,NULL,##__VA_ARGS__)
#define libd_sw16scanf(src,format,...)   libd_sw16scanf_l(src,format,NULL,##__VA_ARGS__)
#define libc_sw32scanf(src,format,...)   libc_sw32scanf_l(src,format,NULL,##__VA_ARGS__)
#define libd_sw32scanf(src,format,...)   libd_sw32scanf_l(src,format,NULL,##__VA_ARGS__)
#define libc_vsscanf(src,format,args)    libc_vsscanf_l(src,format,NULL,args)
#define libd_vsscanf(src,format,args)    libd_vsscanf_l(src,format,NULL,args)
#define libc_vsw16scanf(src,format,args) libc_vsw16scanf_l(src,format,NULL,args)
#define libd_vsw16scanf(src,format,args) libd_vsw16scanf_l(src,format,NULL,args)
#define libc_vsw32scanf(src,format,args) libc_vsw32scanf_l(src,format,NULL,args)
#define libd_vsw32scanf(src,format,args) libd_vsw32scanf_l(src,format,NULL,args)
#endif /* !__OPTIMIZE_SIZE__ */
/* String scanners with explicit input length attribute. */
INTDEF size_t (ATTR_CDECL libc_snscanf)(char const *__restrict src, size_t srclen, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_snscanf)(char const *__restrict src, size_t srclen, char const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_snw16scanf)(char16_t const *__restrict src, size_t srclen, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_snw16scanf)(char16_t const *__restrict src, size_t srclen, char16_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libc_snw32scanf)(char32_t const *__restrict src, size_t srclen, char32_t const *__restrict format, ...);
INTDEF size_t (ATTR_CDECL libd_snw32scanf)(char32_t const *__restrict src, size_t srclen, char32_t const *__restrict format, ...);
INTDEF size_t (LIBCCALL   libc_vsnscanf)(char const *__restrict src, size_t srclen, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsnscanf)(char const *__restrict src, size_t srclen, char const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libc_vsnw16scanf)(char16_t const *__restrict src, size_t srclen, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsnw16scanf)(char16_t const *__restrict src, size_t srclen, char16_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libc_vsnw32scanf)(char32_t const *__restrict src, size_t srclen, char32_t const *__restrict format, va_list args);
INTDEF size_t (LIBCCALL   libd_vsnw32scanf)(char32_t const *__restrict src, size_t srclen, char32_t const *__restrict format, va_list args);
INTDEF size_t  ATTR_CDECL libc_snscanf_l(char const *__restrict src, size_t srclen, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_snscanf_l(char const *__restrict src, size_t srclen, char const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_snw16scanf_l(char16_t const *__restrict src, size_t srclen, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_snw16scanf_l(char16_t const *__restrict src, size_t srclen, char16_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libc_snw32scanf_l(char32_t const *__restrict src, size_t srclen, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  ATTR_CDECL libd_snw32scanf_l(char32_t const *__restrict src, size_t srclen, char32_t const *__restrict format, locale_t locale, ...);
INTDEF size_t  LIBCCALL   libc_vsnscanf_l(char const *__restrict src, size_t srclen, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsnscanf_l(char const *__restrict src, size_t srclen, char const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libc_vsnw16scanf_l(char16_t const *__restrict src, size_t srclen, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsnw16scanf_l(char16_t const *__restrict src, size_t srclen, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libc_vsnw32scanf_l(char32_t const *__restrict src, size_t srclen, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF size_t  LIBCCALL   libd_vsnw32scanf_l(char32_t const *__restrict src, size_t srclen, char32_t const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_snscanf(src,srclen,format,...)      libc_snscanf_l(src,srclen,format,NULL,##__VA_ARGS__)
#define libd_snscanf(src,srclen,format,...)      libd_snscanf_l(src,srclen,format,NULL,##__VA_ARGS__)
#define libc_snw16scanf(src,srclen,format,...)   libc_snw16scanf_l(src,srclen,format,NULL,##__VA_ARGS__)
#define libd_snw16scanf(src,srclen,format,...)   libd_snw16scanf_l(src,srclen,format,NULL,##__VA_ARGS__)
#define libc_snw32scanf(src,srclen,format,...)   libc_snw32scanf_l(src,srclen,format,NULL,##__VA_ARGS__)
#define libd_snw32scanf(src,srclen,format,...)   libd_snw32scanf_l(src,srclen,format,NULL,##__VA_ARGS__)
#define libc_vsnscanf(src,srclen,format,args)    libc_vsnscanf_l(src,srclen,format,NULL,args)
#define libd_vsnscanf(src,srclen,format,args)    libd_vsnscanf_l(src,srclen,format,NULL,args)
#define libc_vsnw16scanf(src,srclen,format,args) libc_vsnw16scanf_l(src,srclen,format,NULL,args)
#define libd_vsnw16scanf(src,srclen,format,args) libd_vsnw16scanf_l(src,srclen,format,NULL,args)
#define libc_vsnw32scanf(src,srclen,format,args) libc_vsnw32scanf_l(src,srclen,format,NULL,args)
#define libd_vsnw32scanf(src,srclen,format,args) libd_vsnw32scanf_l(src,srclen,format,NULL,args)
#endif /* !__OPTIMIZE_SIZE__ */

/* STDIO FILE Printers. */
INTDEF ssize_t (ATTR_CDECL libc_printf)(char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_printf)(char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_w16printf)(char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_w16printf)(char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_w32printf)(char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_w32printf)(char32_t const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_vprintf)(char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vprintf)(char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vw16printf)(char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vw16printf)(char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vw32printf)(char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vw32printf)(char32_t const *__restrict format, va_list args);
INTDEF ssize_t (ATTR_CDECL libc_printf_p)(char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_printf_p)(char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_w16printf_p)(char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_w16printf_p)(char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_w32printf_p)(char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_w32printf_p)(char32_t const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_vprintf_p)(char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vprintf_p)(char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vw16printf_p)(char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vw16printf_p)(char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vw32printf_p)(char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vw32printf_p)(char32_t const *__restrict format, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_printf_l(char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_printf_l(char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_w16printf_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_w16printf_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_w32printf_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_w32printf_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_vprintf_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vprintf_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vw16printf_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vw16printf_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vw32printf_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vw32printf_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_printf_p_l(char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_printf_p_l(char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_w16printf_p_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_w16printf_p_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_w32printf_p_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_w32printf_p_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_vprintf_p_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vprintf_p_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vw16printf_p_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vw16printf_p_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vw32printf_p_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vw32printf_p_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t (ATTR_CDECL libc_fprintf)(FILE *__restrict self, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_fprintf)(FILE *__restrict self, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_fw16printf)(FILE *__restrict self, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_fw16printf)(FILE *__restrict self, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_fw32printf)(FILE *__restrict self, char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_fw32printf)(FILE *__restrict self, char32_t const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_vfprintf)(FILE *__restrict self, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vfprintf)(FILE *__restrict self, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vfw16printf)(FILE *__restrict self, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vfw16printf)(FILE *__restrict self, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vfw32printf)(FILE *__restrict self, char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vfw32printf)(FILE *__restrict self, char32_t const *__restrict format, va_list args);
INTDEF ssize_t (ATTR_CDECL libc_fprintf_p)(FILE *__restrict self, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_fprintf_p)(FILE *__restrict self, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_fw16printf_p)(FILE *__restrict self, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_fw16printf_p)(FILE *__restrict self, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_fw32printf_p)(FILE *__restrict self, char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_fw32printf_p)(FILE *__restrict self, char32_t const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_vfprintf_p)(FILE *__restrict self, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vfprintf_p)(FILE *__restrict self, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vfw16printf_p)(FILE *__restrict self, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vfw16printf_p)(FILE *__restrict self, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vfw32printf_p)(FILE *__restrict self, char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vfw32printf_p)(FILE *__restrict self, char32_t const *__restrict format, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_fprintf_l(FILE *__restrict self, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_fprintf_l(FILE *__restrict self, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_fw16printf_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_fw16printf_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_fw32printf_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_fw32printf_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_vfprintf_l(FILE *__restrict self, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vfprintf_l(FILE *__restrict self, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vfw16printf_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vfw16printf_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vfw32printf_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vfw32printf_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_fprintf_p_l(FILE *__restrict self, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_fprintf_p_l(FILE *__restrict self, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_fw16printf_p_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_fw16printf_p_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_fw32printf_p_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_fw32printf_p_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_vfprintf_p_l(FILE *__restrict self, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vfprintf_p_l(FILE *__restrict self, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vfw16printf_p_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vfw16printf_p_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vfw32printf_p_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vfw32printf_p_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_printf(format,...)              libc_printf_l(format,NULL,##__VA_ARGS__)
#define libd_printf(format,...)              libd_printf_l(format,NULL,##__VA_ARGS__)
#define libc_w16printf(format,...)           libc_w16printf_l(format,NULL,##__VA_ARGS__)
#define libd_w16printf(format,...)           libd_w16printf_l(format,NULL,##__VA_ARGS__)
#define libc_w32printf(format,...)           libc_w32printf_l(format,NULL,##__VA_ARGS__)
#define libd_w32printf(format,...)           libd_w32printf_l(format,NULL,##__VA_ARGS__)
#define libc_vprintf(format,args)            libc_vprintf_l(format,NULL,args)
#define libd_vprintf(format,args)            libd_vprintf_l(format,NULL,args)
#define libc_vw16printf(format,args)         libc_vw16printf_l(format,NULL,args)
#define libd_vw16printf(format,args)         libd_vw16printf_l(format,NULL,args)
#define libc_vw32printf(format,args)         libc_vw32printf_l(format,NULL,args)
#define libd_vw32printf(format,args)         libd_vw32printf_l(format,NULL,args)
#define libc_printf_p(format,...)            libc_printf_p_l(format,NULL,##__VA_ARGS__)
#define libd_printf_p(format,...)            libd_printf_p_l(format,NULL,##__VA_ARGS__)
#define libc_w16printf_p(format,...)         libc_w16printf_p_l(format,NULL,##__VA_ARGS__)
#define libd_w16printf_p(format,...)         libd_w16printf_p_l(format,NULL,##__VA_ARGS__)
#define libc_w32printf_p(format,...)         libc_w32printf_p_l(format,NULL,##__VA_ARGS__)
#define libd_w32printf_p(format,...)         libd_w32printf_p_l(format,NULL,##__VA_ARGS__)
#define libc_vprintf_p(format,args)          libc_vprintf_p_l(format,NULL,args)
#define libd_vprintf_p(format,args)          libd_vprintf_p_l(format,NULL,args)
#define libc_vw16printf_p(format,args)       libc_vw16printf_p_l(format,NULL,args)
#define libd_vw16printf_p(format,args)       libd_vw16printf_p_l(format,NULL,args)
#define libc_vw32printf_p(format,args)       libc_vw32printf_p_l(format,NULL,args)
#define libd_vw32printf_p(format,args)       libd_vw32printf_p_l(format,NULL,args)
#define libc_fprintf(self,format,...)        libc_fprintf_l(self,format,NULL,##__VA_ARGS__)
#define libd_fprintf(self,format,...)        libd_fprintf_l(self,format,NULL,##__VA_ARGS__)
#define libc_fw16printf(self,format,...)     libc_fw16printf_l(self,format,NULL,##__VA_ARGS__)
#define libd_fw16printf(self,format,...)     libd_fw16printf_l(self,format,NULL,##__VA_ARGS__)
#define libc_fw32printf(self,format,...)     libc_fw32printf_l(self,format,NULL,##__VA_ARGS__)
#define libd_fw32printf(self,format,...)     libd_fw32printf_l(self,format,NULL,##__VA_ARGS__)
#define libc_vfprintf(self,format,args)      libc_vfprintf_l(self,format,NULL,args)
#define libd_vfprintf(self,format,args)      libd_vfprintf_l(self,format,NULL,args)
#define libc_vfw16printf(self,format,args)   libc_vfw16printf_l(self,format,NULL,args)
#define libd_vfw16printf(self,format,args)   libd_vfw16printf_l(self,format,NULL,args)
#define libc_vfw32printf(self,format,args)   libc_vfw32printf_l(self,format,NULL,args)
#define libd_vfw32printf(self,format,args)   libd_vfw32printf_l(self,format,NULL,args)
#define libc_fprintf_p(self,format,...)      libc_fprintf_p_l(self,format,NULL,##__VA_ARGS__)
#define libd_fprintf_p(self,format,...)      libd_fprintf_p_l(self,format,NULL,##__VA_ARGS__)
#define libc_fw16printf_p(self,format,...)   libc_fw16printf_p_l(self,format,NULL,##__VA_ARGS__)
#define libd_fw16printf_p(self,format,...)   libd_fw16printf_p_l(self,format,NULL,##__VA_ARGS__)
#define libc_fw32printf_p(self,format,...)   libc_fw32printf_p_l(self,format,NULL,##__VA_ARGS__)
#define libd_fw32printf_p(self,format,...)   libd_fw32printf_p_l(self,format,NULL,##__VA_ARGS__)
#define libc_vfprintf_p(self,format,args)    libc_vfprintf_p_l(self,format,NULL,args)
#define libd_vfprintf_p(self,format,args)    libd_vfprintf_p_l(self,format,NULL,args)
#define libc_vfw16printf_p(self,format,args) libc_vfw16printf_p_l(self,format,NULL,args)
#define libd_vfw16printf_p(self,format,args) libd_vfw16printf_p_l(self,format,NULL,args)
#define libc_vfw32printf_p(self,format,args) libc_vfw32printf_p_l(self,format,NULL,args)
#define libd_vfw32printf_p(self,format,args) libd_vfw32printf_p_l(self,format,NULL,args)
#endif /* !__OPTIMIZE_SIZE__ */

INTDEF ssize_t (ATTR_CDECL libc_scanf)(char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_scanf)(char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_w16scanf)(char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_w16scanf)(char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_w32scanf)(char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_w32scanf)(char32_t const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_vscanf)(char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vscanf)(char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vw16scanf)(char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vw16scanf)(char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vw32scanf)(char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vw32scanf)(char32_t const *__restrict format, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_scanf_l(char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_scanf_l(char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_w16scanf_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_w16scanf_l(char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_w32scanf_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_w32scanf_l(char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_vscanf_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vscanf_l(char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vw16scanf_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vw16scanf_l(char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vw32scanf_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vw32scanf_l(char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t (ATTR_CDECL libc_fscanf)(FILE *__restrict self, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_fscanf)(FILE *__restrict self, char const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_fw16scanf)(FILE *__restrict self, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_fw16scanf)(FILE *__restrict self, char16_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libc_fw32scanf)(FILE *__restrict self, char32_t const *__restrict format, ...);
INTDEF ssize_t (ATTR_CDECL libd_fw32scanf)(FILE *__restrict self, char32_t const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_vfscanf)(FILE *__restrict self, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vfscanf)(FILE *__restrict self, char const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vfw16scanf)(FILE *__restrict self, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vfw16scanf)(FILE *__restrict self, char16_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libc_vfw32scanf)(FILE *__restrict self, char32_t const *__restrict format, va_list args);
INTDEF ssize_t (LIBCCALL   libd_vfw32scanf)(FILE *__restrict self, char32_t const *__restrict format, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_fscanf_l(FILE *__restrict self, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_fscanf_l(FILE *__restrict self, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_fw16scanf_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_fw16scanf_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libc_fw32scanf_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_fw32scanf_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_vfscanf_l(FILE *__restrict self, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vfscanf_l(FILE *__restrict self, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vfw16scanf_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vfw16scanf_l(FILE *__restrict self, char16_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libc_vfw32scanf_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_vfw32scanf_l(FILE *__restrict self, char32_t const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_scanf(format,...)            libc_scanf_l(format,NULL,##__VA_ARGS__)
#define libd_scanf(format,...)            libd_scanf_l(format,NULL,##__VA_ARGS__)
#define libc_w16scanf(format,...)         libc_w16scanf_l(format,NULL,##__VA_ARGS__)
#define libd_w16scanf(format,...)         libd_w16scanf_l(format,NULL,##__VA_ARGS__)
#define libc_w32scanf(format,...)         libc_w32scanf_l(format,NULL,##__VA_ARGS__)
#define libd_w32scanf(format,...)         libd_w32scanf_l(format,NULL,##__VA_ARGS__)
#define libc_vscanf(format,args)          libc_vscanf_l(format,NULL,args)
#define libd_vscanf(format,args)          libd_vscanf_l(format,NULL,args)
#define libc_vw16scanf(format,args)       libc_vw16scanf_l(format,NULL,args)
#define libd_vw16scanf(format,args)       libd_vw16scanf_l(format,NULL,args)
#define libc_vw32scanf(format,args)       libc_vw32scanf_l(format,NULL,args)
#define libd_vw32scanf(format,args)       libd_vw32scanf_l(format,NULL,args)
#define libc_fscanf(self,format,...)      libc_fscanf_l(self,format,NULL,##__VA_ARGS__)
#define libd_fscanf(self,format,...)      libd_fscanf_l(self,format,NULL,##__VA_ARGS__)
#define libc_fw16scanf(self,format,...)   libc_fw16scanf_l(self,format,NULL,##__VA_ARGS__)
#define libd_fw16scanf(self,format,...)   libd_fw16scanf_l(self,format,NULL,##__VA_ARGS__)
#define libc_fw32scanf(self,format,...)   libc_fw32scanf_l(self,format,NULL,##__VA_ARGS__)
#define libd_fw32scanf(self,format,...)   libd_fw32scanf_l(self,format,NULL,##__VA_ARGS__)
#define libc_vfscanf(self,format,args)    libc_vfscanf_l(self,format,NULL,args)
#define libd_vfscanf(self,format,args)    libd_vfscanf_l(self,format,NULL,args)
#define libc_vfw16scanf(self,format,args) libc_vfw16scanf_l(self,format,NULL,args)
#define libd_vfw16scanf(self,format,args) libd_vfw16scanf_l(self,format,NULL,args)
#define libc_vfw32scanf(self,format,args) libc_vfw32scanf_l(self,format,NULL,args)
#define libd_vfw32scanf(self,format,args) libd_vfw32scanf_l(self,format,NULL,args)
#endif /* !__OPTIMIZE_SIZE__ */
INTDEF ssize_t (LIBCCALL libc_format_strftime_l)(pformatprinter printer, void *closure, char const *__restrict format, struct tm const *__restrict tm, locale_t locale);
INTDEF ssize_t (LIBCCALL libc_format_w16ftime_l)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, struct tm const *__restrict tm, locale_t locale);
INTDEF ssize_t (LIBCCALL libc_format_w32ftime_l)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, struct tm const *__restrict tm, locale_t locale);
INTDEF ssize_t (LIBCCALL libc_format_strftime)(pformatprinter printer, void *closure, char const *__restrict format, struct tm const *__restrict tm);
INTDEF ssize_t (LIBCCALL libc_format_w16ftime)(pw16formatprinter printer, void *closure, char16_t const *__restrict format, struct tm const *__restrict tm);
INTDEF ssize_t (LIBCCALL libc_format_w32ftime)(pw32formatprinter printer, void *closure, char32_t const *__restrict format, struct tm const *__restrict tm);
INTDEF size_t LIBCCALL libc_strftime(char *__restrict s, size_t maxsize, char const *__restrict format, struct tm const *__restrict tp);
INTDEF size_t LIBCCALL libc_strftime_l(char *__restrict s, size_t maxsize, char const *__restrict format, struct tm const *__restrict tp, locale_t loc);
INTDEF size_t LIBCCALL libc_w16ftime(char16_t *__restrict s, size_t maxsize, char16_t const *__restrict format, struct tm const *__restrict tp);
INTDEF size_t LIBCCALL libc_w16ftime_l(char16_t *__restrict s, size_t maxsize, char16_t const *__restrict format, struct tm const *__restrict tp, locale_t loc);
INTDEF size_t LIBCCALL libc_w32ftime(char32_t *__restrict s, size_t maxsize, char32_t const *__restrict format, struct tm const *__restrict tp);
INTDEF size_t LIBCCALL libc_w32ftime_l(char32_t *__restrict s, size_t maxsize, char32_t const *__restrict format, struct tm const *__restrict tp, locale_t loc);

DECL_END
#endif /* __CC__ */


#endif /* !GUARD_LIBS_LIBC_FORMAT_H */
