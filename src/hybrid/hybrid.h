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
#ifndef GUARD_HYBRID_HYBRID_H
#define GUARD_HYBRID_HYBRID_H 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1
#define error_code()  libc_error_code()
#define error_info()  libc_error_info()

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <hybrid/debuginfo.h>
#include <xlocale.h>
#include <stdbool.h>
#include <stdarg.h>
#include <except.h>
#include <endian.h>

DECL_BEGIN

/* Symbol name generators for different symbol classes. */
#define __DSYMw16(x)  DOS$##x
#define __DSYMw32(x)  DOS$U##x
#define __KSYMw16(x)  u##x
#define __KSYMw32(x)  x
#define __SYMw16      __DSYMw16
#define __SYMw32      __KSYMw32
#define __DSYM        __DSYMw16
#define __KSYM        __KSYMw32

/* Optimization hint: `va_end()' is a no-op. */
#undef CONFIG_VA_END_IS_NOOP
#if defined(__x86_64__) || defined(__arm__) || defined(__i386__)
#define CONFIG_VA_END_IS_NOOP 1
#endif

/* Optimization hint: A LIBCCALL function can be invoked with any number
 *                    of arguments, where additional arguments will simply
 *                    be ignored.
 *                    This is usually the case when arguments
 *                    memory/registers must be allocated/freed
 *                    by the caller of a function. */
#undef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
#if defined(__x86_64__) || defined(__arm__)
#define CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP 1
#elif defined(__i386__) && !defined(__KERNEL__)
#define CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP 1
#endif

/* Optimization hint: LIBCCALL functions returning basic integral (non-floating)
 *                    values can be invoked as though they returned `void'. */
#undef CONFIG_LIBCCALL_INTRETURN_IS_VOID
#if defined(__x86_64__) || defined(__arm__) || defined(__i386__)
#define CONFIG_LIBCCALL_INTRETURN_IS_VOID 1
#endif

/* Optimization hint: A LIBCCALL function returning a 64-bit value
 *                    can be called as one returning a 32-bit value,
 *                    where the value then read is simply the 64-bit
 *                    value truncated to 32 bits. */
#undef LIBC_LIBCCALL_RET64_IS_RET32
#if defined(__x86_64__) || defined(__arm__) || defined(__i386__)
#define LIBC_LIBCCALL_RET64_IS_RET32 1
#endif

#if defined(__KERNEL__)
#define CONFIG_LIBC_LIMITED_API 1
#endif

#ifdef __KERNEL__
#define EXPORT(name,symbol)         DEFINE_PUBLIC_ALIAS(name,symbol)
#else
#define EXPORT(name,symbol)         DEFINE_PUBLIC_WEAK_ALIAS(name,symbol)
#endif
#define EXPORT_STRONG(name,symbol)  DEFINE_PUBLIC_ALIAS(name,symbol)

#ifdef __CC__
#if 1
#define LIBC_TRY                  TRY
#define LIBC_FINALLY              FINALLY
#define LIBC_FINALLY_WILL_RETHROW FINALLY_WILL_RETHROW
#define LIBC_EXCEPT               EXCEPT
#define LIBC_CATCH                CATCH
#else
#define LIBC_TRY                  /* nothing */
#define LIBC_FINALLY              /* nothing */
#define LIBC_FINALLY_WILL_RETHROW 0
#define LIBC_EXCEPT(x)            if(0)
#define LIBC_CATCH(x)             if(0)
#endif

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */


#ifdef CONFIG_VA_END_IS_NOOP
#define __TRY_VALIST     /* Nothing */
#define __FINALLY_VALIST /* Nothing */
#else
#define __TRY_VALIST     LIBC_TRY
#define __FINALLY_VALIST LIBC_FINALLY
#endif

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

/* Character type IDs used by template-generated code. */
#define CHARACTER_TYPE_CHAR   0
#define CHARACTER_TYPE_CHAR16 1
#define CHARACTER_TYPE_CHAR32 2


/* ===================================================================================== */
/*     CTYPE                                                                             */
/* ===================================================================================== */
#ifndef __wint_t_defined
#define __wint_t_defined 1
typedef __WINT_TYPE__ wint_t;
#endif /* !__wint_t_defined */
#ifndef __std_wctype_t_defined
#define __std_wctype_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __WCTYPE_TYPE__ wctype_t;
__NAMESPACE_STD_END
#endif /* !__std_wctype_t_defined */
#ifndef __wctype_t_defined
#define __wctype_t_defined 1
__NAMESPACE_STD_USING(wctype_t)
#endif /* !__wctype_t_defined */
#ifndef __wctrans_t_defined
#define __wctrans_t_defined 1
typedef s32 const *wctrans_t;
#endif /* !__wctrans_t_defined */
#if __BYTE_ORDER == __BIG_ENDIAN
#   define CTYPE_SLOT(x)    x
#else
#   define CTYPE_SLOT(x)   (u16)(x << 8|x >> 8)
#endif
#define CTYPE_UPPER  0x0001 /* UPPERCASE. */
#define CTYPE_LOWER  0x0002 /* lowercase. */
#define CTYPE_ALPHA  0x0004 /* Alphabetic. */
#define CTYPE_DIGIT  0x0008 /* Numeric. */
#define CTYPE_XDIGIT 0x0010 /* Hexadecimal numeric. */
#define CTYPE_SPACE  0x0020 /* Whitespace. */
#define CTYPE_PRINT  0x0040 /* Printing. */
#define CTYPE_GRAPH  0x0080 /* Graphical. */
#define CTYPE_BLANK  0x0100 /* Blank (usually SPC and TAB). */
#define CTYPE_CNTRL  0x0200 /* Control character. */
#define CTYPE_PUNCT  0x0400 /* Punctuation. */
#define CTYPE_ALNUM  0x0800 /* Alphanumeric. */
#define ASCII_ISCNTRL(ch)  ((ch) <= 0x1f || (ch) == 0x7f)
#define ASCII_ISBLANK(ch)  ((ch) == 0x09 || (ch) == 0x20)
#define ASCII_ISSPACE(ch)  (((ch) >= 0x09 && (ch) <= 0x0d) || (ch) == 0x20)
#define ASCII_ISUPPER(ch)  ((ch) >= 0x41 && (ch) <= 0x5a)
#define ASCII_ISLOWER(ch)  ((ch) >= 0x61 && (ch) <= 0x7a)
#define ASCII_ISALPHA(ch)  (ASCII_ISUPPER(ch) || ASCII_ISLOWER(ch))
#define ASCII_ISDIGIT(ch)  ((ch) >= 0x30 && (ch) <= 0x39)
#define ASCII_ISXDIGIT(ch) (ASCII_ISDIGIT(ch) || \
                           ((ch) >= 0x41 && (ch) <= 0x46) || \
                           ((ch) >= 0x61 && (ch) <= 0x66))
#define ASCII_ISALNUM(ch)  (ASCII_ISUPPER(ch) || ASCII_ISLOWER(ch) || ASCII_ISDIGIT(ch))
#define ASCII_ISPUNCT(ch)  (((ch) >= 0x21 && (ch) <= 0x2f) || \
                            ((ch) >= 0x3a && (ch) <= 0x40) || \
                            ((ch) >= 0x5b && (ch) <= 0x60) || \
                            ((ch) >= 0x7b && (ch) <= 0x7e))
#define ASCII_ISGRAPH(ch)  ((ch) >= 0x21 && (ch) <= 0x7e)
#define ASCII_ISPRINT(ch)  ((ch) >= 0x20 && (ch) <= 0x7e)
#define ASCII_TOLOWER(ch)  (ASCII_ISUPPER(ch) ? ((ch)+0x20) : (ch))
#define ASCII_TOUPPER(ch)  (ASCII_ISLOWER(ch) ? ((ch)-0x20) : (ch))

INTDEF u16 const libc_chattr[256];
INTDEF int (LIBCCALL libc_isalpha)(int ch);
INTDEF int (LIBCCALL libc_isupper)(int ch);
INTDEF int (LIBCCALL libc_islower)(int ch);
INTDEF int (LIBCCALL libc_isdigit)(int ch);
INTDEF int (LIBCCALL libc_isxdigit)(int ch);
INTDEF int (LIBCCALL libc_isspace)(int ch);
INTDEF int (LIBCCALL libc_ispunct)(int ch);
INTDEF int (LIBCCALL libc_isalnum)(int ch);
INTDEF int (LIBCCALL libc_isprint)(int ch);
INTDEF int (LIBCCALL libc_isgraph)(int ch);
INTDEF int (LIBCCALL libc_iscntrl)(int ch);
INTDEF int (LIBCCALL libc_isblank)(int ch);
INTDEF int (LIBCCALL libc_isascii)(int ch);
INTDEF int (LIBCCALL libc_toupper)(int ch);
INTDEF int (LIBCCALL libc_tolower)(int ch);
INTDEF int (LIBCCALL libc_isctype)(int ch, int mask);
#define libc_isctype(c,type) (libc_chattr[(__UINT8_TYPE__)(c)]&(__UINT16_TYPE__)(type))
#define libc_isalnum(c)  libc_isctype((c),CTYPE_SLOT(CTYPE_ALNUM))
#define libc_isalpha(c)  libc_isctype((c),CTYPE_SLOT(CTYPE_ALPHA))
#define libc_iscntrl(c)  libc_isctype((c),CTYPE_SLOT(CTYPE_CNTRL))
#define libc_isdigit(c)  libc_isctype((c),CTYPE_SLOT(CTYPE_DIGIT))
#define libc_islower(c)  libc_isctype((c),CTYPE_SLOT(CTYPE_LOWER))
#define libc_isgraph(c)  libc_isctype((c),CTYPE_SLOT(CTYPE_GRAPH))
#define libc_isprint(c)  libc_isctype((c),CTYPE_SLOT(CTYPE_PRINT))
#define libc_ispunct(c)  libc_isctype((c),CTYPE_SLOT(CTYPE_PUNCT))
#define libc_isspace(c)  libc_isctype((c),CTYPE_SLOT(CTYPE_SPACE))
#define libc_isupper(c)  libc_isctype((c),CTYPE_SLOT(CTYPE_UPPER))
#define libc_isxdigit(c) libc_isctype((c),CTYPE_SLOT(CTYPE_XDIGIT))
#define libc_isblank(c)  libc_isctype((c),CTYPE_SLOT(CTYPE_BLANK))
#define libc_isascii(c)  ((unsigned int)(c) <= 0x7f)

#ifndef CONFIG_LIBC_LIMITED_API
INTDEF int (LIBCCALL libc__toupper)(int ch);
INTDEF int (LIBCCALL libc__tolower)(int ch);
INTDEF int LIBCCALL libc_iscsym(int ch);
INTDEF int LIBCCALL libc_iscsymf(int ch);
INTDEF int LIBCCALL libc_toascii(int ch);
INTDEF int LIBCCALL libc_toascii_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isleadbyte(int wc);
INTDEF int LIBCCALL libc_dos_isctype(int ch, int mask);
INTDEF int LIBCCALL libc_isascii_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isalpha_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isupper_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_islower_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isdigit_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isxdigit_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isspace_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_ispunct_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isalnum_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isprint_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isgraph_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_iscntrl_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isblank_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_iscsym_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_iscsymf_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_isctype_l(int ch, int mask, locale_t locale);
INTDEF int LIBCCALL libc_isleadbyte_l(int wc, locale_t locale);
INTDEF int LIBCCALL libc_toupper_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_tolower_l(int ch, locale_t locale);
INTDEF int LIBCCALL libc_iswalnum(wint_t wc);
INTDEF int LIBCCALL libc_iswalpha(wint_t wc);
INTDEF int LIBCCALL libc_iswcntrl(wint_t wc);
INTDEF int LIBCCALL libc_iswdigit(wint_t wc);
INTDEF int LIBCCALL libc_iswgraph(wint_t wc);
INTDEF int LIBCCALL libc_iswlower(wint_t wc);
INTDEF int LIBCCALL libc_iswprint(wint_t wc);
INTDEF int LIBCCALL libc_iswpunct(wint_t wc);
INTDEF int LIBCCALL libc_iswspace(wint_t wc);
INTDEF int LIBCCALL libc_iswupper(wint_t wc);
INTDEF int LIBCCALL libc_iswxdigit(wint_t wc);
INTDEF int LIBCCALL libc_iswblank(wint_t wc);
INTDEF int LIBCCALL libc_iswascii(wint_t wc);
INTDEF int LIBCCALL libc_iswcsym(wint_t wc);
INTDEF int LIBCCALL libc_iswcsymf(wint_t wc);
INTDEF wint_t LIBCCALL libc_towlower(wint_t wc);
INTDEF wint_t LIBCCALL libc_towupper(wint_t wc);
INTDEF wctype_t LIBCCALL libc_wctype(char const *prop);
INTDEF int LIBCCALL libc_iswctype(wint_t wc, wctype_t desc);
INTDEF wctrans_t LIBCCALL libc_wctrans(char const *prop);
INTDEF wint_t LIBCCALL libc_towctrans(wint_t wc, wctrans_t desc);
INTDEF int LIBCCALL libc_iswalnum_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswalpha_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswcntrl_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswdigit_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswgraph_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswlower_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswprint_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswpunct_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswspace_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswupper_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswxdigit_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswblank_l(wint_t wc, locale_t locale);
INTDEF int LIBCCALL libc_iswcsym_l(wint_t ch, locale_t locale);
INTDEF int LIBCCALL libc_iswcsymf_l(wint_t ch, locale_t locale);
INTDEF wctype_t LIBCCALL libc_wctype_l(char const *prop, locale_t locale);
INTDEF int LIBCCALL libc_iswctype_l(wint_t wc, wctype_t desc, locale_t locale);
INTDEF wint_t LIBCCALL libc_towlower_l(wint_t wc, locale_t locale);
INTDEF wint_t LIBCCALL libc_towupper_l(wint_t wc, locale_t locale);
INTDEF wctrans_t LIBCCALL libc_wctrans_l(char const *prop, locale_t locale);
INTDEF wint_t LIBCCALL libc_towctrans_l(wint_t wc, wctrans_t desc, locale_t locale);
#if 0
/* DOS's multi-byte character trait functions. */
INTDEF int LIBCCALL libd_ismbbkalnum(__UINT32_TYPE__ ch);
INTDEF int LIBCCALL libd_ismbbkana(__UINT32_TYPE__ ch);
INTDEF int LIBCCALL libd_ismbbkpunct(__UINT32_TYPE__ ch);
INTDEF int LIBCCALL libd_ismbbkprint(__UINT32_TYPE__ ch);
INTDEF int LIBCCALL libd_ismbbalpha(__UINT32_TYPE__ ch);
INTDEF int LIBCCALL libd_ismbbpunct(__UINT32_TYPE__ ch);
INTDEF int LIBCCALL libd_ismbbblank(__UINT32_TYPE__ ch);
INTDEF int LIBCCALL libd_ismbbalnum(__UINT32_TYPE__ ch);
INTDEF int LIBCCALL libd_ismbbprint(__UINT32_TYPE__ ch);
INTDEF int LIBCCALL libd_ismbbgraph(__UINT32_TYPE__ ch);
INTDEF int LIBCCALL libd_ismbblead(__UINT32_TYPE__ ch);
INTDEF int LIBCCALL libd_ismbbtrail(__UINT32_TYPE__ ch);
INTDEF int LIBCCALL libd_ismbbkalnum_l(__UINT32_TYPE__ ch, locale_t locale);
INTDEF int LIBCCALL libd_ismbbkana_l(__UINT32_TYPE__ ch, locale_t locale);
INTDEF int LIBCCALL libd_ismbbkpunct_l(__UINT32_TYPE__ ch, locale_t locale);
INTDEF int LIBCCALL libd_ismbbkprint_l(__UINT32_TYPE__ ch, locale_t locale);
INTDEF int LIBCCALL libd_ismbbalpha_l(__UINT32_TYPE__ ch, locale_t locale);
INTDEF int LIBCCALL libd_ismbbpunct_l(__UINT32_TYPE__ ch, locale_t locale);
INTDEF int LIBCCALL libd_ismbbblank_l(__UINT32_TYPE__ ch, locale_t locale);
INTDEF int LIBCCALL libd_ismbbalnum_l(__UINT32_TYPE__ ch, locale_t locale);
INTDEF int LIBCCALL libd_ismbbprint_l(__UINT32_TYPE__ ch, locale_t locale);
INTDEF int LIBCCALL libd_ismbbgraph_l(__UINT32_TYPE__ ch, locale_t locale);
INTDEF int LIBCCALL libd_ismbblead_l(__UINT32_TYPE__ ch, locale_t locale);
INTDEF int LIBCCALL libd_ismbbtrail_l(__UINT32_TYPE__ ch, locale_t locale);
INTDEF int LIBCCALL libd_ismbslead(unsigned char const *str, unsigned char const *pos);
INTDEF int LIBCCALL libd_ismbstrail(unsigned char const *str, unsigned char const *pos);
INTDEF int LIBCCALL libd_ismbslead_l(unsigned char const *str, unsigned char const *pos, locale_t locale);
INTDEF int LIBCCALL libd_ismbstrail_l(unsigned char const *str, unsigned char const *pos, locale_t locale);
#endif
#endif /* !CONFIG_LIBC_LIMITED_API */






/* ===================================================================================== */
/*     FORMAT_PRINTER                                                                    */
/* ===================================================================================== */
#ifndef __pformatprinter_defined
#define __pformatprinter_defined 1
typedef ssize_t (LIBCCALL *pformatprinter)(char const *__restrict data, size_t datalen, void *closure);
typedef ssize_t (LIBCCALL *pformatgetc)(void *closure);
typedef ssize_t (LIBCCALL *pformatungetc)(int ch, void *closure);
#endif /* !__pformatprinter_defined */
INTDEF char const libc_decimals[2][17];
INTDEF ssize_t LIBCCALL libc_format_width(char const *__restrict data, size_t datalen, void *closure);
INTDEF ssize_t (LIBCCALL   libc_format_quote)(pformatprinter printer, void *closure, char const *__restrict text, size_t textlen, u32 flags);
INTDEF ssize_t (LIBCCALL   libc_format_hexdump)(pformatprinter printer, void *closure, void const *__restrict data, size_t size, size_t linesize, u32 flags);
INTDEF ssize_t (ATTR_CDECL libc_format_printf)(pformatprinter printer, void *closure, char const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_format_vprintf)(pformatprinter printer, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t  LIBCCALL   libc_format_repeat(pformatprinter printer, void *closure, char ch, size_t num_repetitions);
#ifndef CONFIG_LIBC_LIMITED_API
INTDEF ssize_t (ATTR_CDECL libd_format_printf)(pformatprinter printer, void *closure, char const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libd_format_vprintf)(pformatprinter printer, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t  LIBCCALL   libc_format_quote_l(pformatprinter printer, void *closure, char const *__restrict text, size_t textlen, u32 flags, locale_t locale);
INTDEF ssize_t  LIBCCALL   libc_format_hexdump_l(pformatprinter printer, void *closure, void const *__restrict data, size_t size, size_t linesize, u32 flags, locale_t locale);
INTDEF ssize_t  ATTR_CDECL libc_format_printf_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_printf_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_format_vprintf_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vprintf_l(pformatprinter printer, void *closure, char const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_format_quote(printer,closure,text,textlen,flags)         libc_format_quote_l(printer,closure,text,textlen,flags,NULL)
#define libc_format_hexdump(printer,closure,data,size,linesize,flags) libc_format_hexdump_l(printer,closure,data,size,linesize,flags,NULL)
#define libc_format_printf(printer,closure,format,...)                libc_format_printf_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libd_format_printf(printer,closure,format,...)                libd_format_printf_l(printer,closure,format,NULL,##__VA_ARGS__)
#define libc_format_vprintf(printer,closure,format,args)              libc_format_vprintf_l(printer,closure,format,NULL,args)
#define libd_format_vprintf(printer,closure,format,args)              libd_format_vprintf_l(printer,closure,format,NULL,args)
#endif /* !__OPTIMIZE_SIZE__ */
#endif







/* ===================================================================================== */
/*     FORMAT_SCANNER                                                                    */
/* ===================================================================================== */
INTDEF ssize_t (ATTR_CDECL libc_format_scanf)(pformatgetc scanner, pformatungetc returnch, void *closure, char const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libc_format_vscanf)(pformatgetc scanner, pformatungetc returnch, void *closure, char const *__restrict format, va_list args);
#ifndef CONFIG_LIBC_LIMITED_API
INTDEF ssize_t (ATTR_CDECL libd_format_scanf)(pformatgetc scanner, pformatungetc returnch, void *closure, char const *__restrict format, ...);
INTDEF ssize_t (LIBCCALL   libd_format_vscanf)(pformatgetc scanner, pformatungetc returnch, void *closure, char const *__restrict format, va_list args);
INTDEF ssize_t  ATTR_CDECL libc_format_scanf_l(pformatgetc scanner, pformatungetc returnch, void *closure, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  ATTR_CDECL libd_format_scanf_l(pformatgetc scanner, pformatungetc returnch, void *closure, char const *__restrict format, locale_t locale, ...);
INTDEF ssize_t  LIBCCALL   libc_format_vscanf_l(pformatgetc scanner, pformatungetc returnch, void *closure, char const *__restrict format, locale_t locale, va_list args);
INTDEF ssize_t  LIBCCALL   libd_format_vscanf_l(pformatgetc scanner, pformatungetc returnch, void *closure, char const *__restrict format, locale_t locale, va_list args);
#ifndef __OPTIMIZE_SIZE__
#define libc_format_scanf(scanner,returnch,closure,format,...)   libc_format_scanf_l(scanner,returnch,closure,format,NULL,##__VA_ARGS__)
#define libd_format_scanf(scanner,returnch,closure,format,...)   libd_format_scanf_l(scanner,returnch,closure,format,NULL,##__VA_ARGS__)
#endif /* !__OPTIMIZE_SIZE__ */
#define libc_format_vscanf(scanner,returnch,closure,format,args) libc_format_vscanf_l(scanner,returnch,closure,format,NULL,args)
#define libd_format_vscanf(scanner,returnch,closure,format,args) libd_format_vscanf_l(scanner,returnch,closure,format,NULL,args)
#endif







/* ===================================================================================== */
/*     MEMORY                                                                            */
/* ===================================================================================== */
#if !defined(GUARD_HYBRID_STRING_C) || defined(__INTELLISENSE__)
INTDEF void *LIBCCALL libc_memcpy(void *__restrict dst, void const *__restrict src, size_t num_bytes);
INTDEF void *LIBCCALL libc_memcpyw(void *__restrict dst, void const *__restrict src, size_t num_words);
INTDEF void *LIBCCALL libc_memcpyl(void *__restrict dst, void const *__restrict src, size_t num_dwords);
INTDEF void *LIBCCALL libc_memcpyq(void *__restrict dst, void const *__restrict src, size_t num_qwords);
INTDEF void *LIBCCALL libc_mempcpy(void *__restrict dst, void const *__restrict src, size_t num_bytes);
INTDEF void *LIBCCALL libc_mempcpyw(void *__restrict dst, void const *__restrict src, size_t num_words);
INTDEF void *LIBCCALL libc_mempcpyl(void *__restrict dst, void const *__restrict src, size_t num_dwords);
INTDEF void *LIBCCALL libc_mempcpyq(void *__restrict dst, void const *__restrict src, size_t num_qwords);
INTDEF void *LIBCCALL libc_memcpy_d(void *__restrict dst, void const *__restrict src, size_t num_bytes, DEBUGINFO);
INTDEF void *LIBCCALL libc_memcpyw_d(void *__restrict dst, void const *__restrict src, size_t num_words, DEBUGINFO);
INTDEF void *LIBCCALL libc_memcpyl_d(void *__restrict dst, void const *__restrict src, size_t num_dwords, DEBUGINFO);
INTDEF void *LIBCCALL libc_memcpyq_d(void *__restrict dst, void const *__restrict src, size_t num_qwords, DEBUGINFO);
INTDEF void *LIBCCALL libc_mempcpy_d(void *__restrict dst, void const *__restrict src, size_t num_bytes, DEBUGINFO);
INTDEF void *LIBCCALL libc_mempcpyw_d(void *__restrict dst, void const *__restrict src, size_t num_words, DEBUGINFO);
INTDEF void *LIBCCALL libc_mempcpyl_d(void *__restrict dst, void const *__restrict src, size_t num_dwords, DEBUGINFO);
INTDEF void *LIBCCALL libc_mempcpyq_d(void *__restrict dst, void const *__restrict src, size_t num_qwords, DEBUGINFO);
INTDEF void *LIBCCALL libc_memset(void *__restrict dst, int byte, size_t num_bytes);
INTDEF void *LIBCCALL libc_memsetw(void *__restrict dst, u16 word, size_t num_words);
INTDEF void *LIBCCALL libc_memsetl(void *__restrict dst, u32 dword, size_t num_dwords);
INTDEF void *LIBCCALL libc_memsetq(void *__restrict dst, u64 qword, size_t num_qwords);
INTDEF void *LIBCCALL libc_mempset(void *__restrict dst, int byte, size_t num_bytes);
INTDEF void *LIBCCALL libc_mempsetw(void *__restrict dst, u16 word, size_t num_words);
INTDEF void *LIBCCALL libc_mempsetl(void *__restrict dst, u32 dword, size_t num_dwords);
INTDEF void *LIBCCALL libc_mempsetq(void *__restrict dst, u64 qword, size_t num_qwords);
INTDEF void *LIBCCALL libc_memmove(void *dst, void const *src, size_t num_bytes);
INTDEF void *LIBCCALL libc_memmovew(void *dst, void const *src, size_t num_words);
INTDEF void *LIBCCALL libc_memmovel(void *dst, void const *src, size_t num_dwords);
INTDEF void *LIBCCALL libc_memmoveq(void *dst, void const *src, size_t num_qwords);
INTDEF void *LIBCCALL libc_mempmove(void *dst, void const *src, size_t num_bytes);
INTDEF void *LIBCCALL libc_mempmovew(void *dst, void const *src, size_t num_words);
INTDEF void *LIBCCALL libc_mempmovel(void *dst, void const *src, size_t num_dwords);
INTDEF void *LIBCCALL libc_mempmoveq(void *dst, void const *src, size_t num_qwords);
INTDEF int LIBCCALL libc_memcmp(void const *a, void const *b, size_t num_bytes);
INTDEF s16 LIBCCALL libc_memcmpw(void const *a, void const *b, size_t num_words);
INTDEF s32 LIBCCALL libc_memcmpl(void const *a, void const *b, size_t num_dwords);
INTDEF s64 LIBCCALL libc_memcmpq(void const *a, void const *b, size_t num_qwords);
INTDEF void *LIBCCALL libc_memchr(void const *__restrict buf, int byte, size_t num_bytes);
INTDEF u16 *LIBCCALL libc_memchrw(void const *__restrict buf, u16 byte, size_t num_words);
INTDEF u32 *LIBCCALL libc_memchrl(void const *__restrict buf, u32 byte, size_t num_dwords);
INTDEF u64 *LIBCCALL libc_memchrq(void const *__restrict buf, u64 byte, size_t num_qwords);
INTDEF void *LIBCCALL libc_memrchr(void const *__restrict buf, int byte, size_t num_bytes);
INTDEF u16 *LIBCCALL libc_memrchrw(void const *__restrict buf, u16 byte, size_t num_words);
INTDEF u32 *LIBCCALL libc_memrchrl(void const *__restrict buf, u32 byte, size_t num_dwords);
INTDEF u64 *LIBCCALL libc_memrchrq(void const *__restrict buf, u64 byte, size_t num_qwords);
INTDEF void *LIBCCALL libc_memend(void const *__restrict buf, int byte, size_t num_bytes);
INTDEF u16 *LIBCCALL libc_memendw(void const *__restrict buf, u16 byte, size_t num_words);
INTDEF u32 *LIBCCALL libc_memendl(void const *__restrict buf, u32 byte, size_t num_dwords);
INTDEF u64 *LIBCCALL libc_memendq(void const *__restrict buf, u64 byte, size_t num_qwords);
INTDEF void *LIBCCALL libc_memrend(void const *__restrict buf, int byte, size_t num_bytes);
INTDEF u16 *LIBCCALL libc_memrendw(void const *__restrict buf, u16 byte, size_t num_words);
INTDEF u32 *LIBCCALL libc_memrendl(void const *__restrict buf, u32 byte, size_t num_dwords);
INTDEF u64 *LIBCCALL libc_memrendq(void const *__restrict buf, u64 byte, size_t num_qwords);
INTDEF size_t LIBCCALL libc_memlen(void const *__restrict buf, int byte, size_t num_bytes);
INTDEF size_t LIBCCALL libc_memlenw(void const *__restrict buf, u16 byte, size_t num_words);
INTDEF size_t LIBCCALL libc_memlenl(void const *__restrict buf, u32 byte, size_t num_dwords);
INTDEF size_t LIBCCALL libc_memlenq(void const *__restrict buf, u64 byte, size_t num_qwords);
INTDEF size_t LIBCCALL libc_memrlen(void const *__restrict buf, int byte, size_t num_bytes);
INTDEF size_t LIBCCALL libc_memrlenw(void const *__restrict buf, u16 byte, size_t num_words);
INTDEF size_t LIBCCALL libc_memrlenl(void const *__restrict buf, u32 byte, size_t num_dwords);
INTDEF size_t LIBCCALL libc_memrlenq(void const *__restrict buf, u64 byte, size_t num_qwords);
INTDEF void *LIBCCALL libc_rawmemchr(void const *__restrict buf, int byte);
INTDEF u16 *LIBCCALL libc_rawmemchrw(void const *__restrict buf, u16 byte);
INTDEF u32 *LIBCCALL libc_rawmemchrl(void const *__restrict buf, u32 byte);
INTDEF u64 *LIBCCALL libc_rawmemchrq(void const *__restrict buf, u64 byte);
INTDEF size_t LIBCCALL libc_rawmemlen(void const *__restrict buf, int byte);
INTDEF size_t LIBCCALL libc_rawmemlenw(void const *__restrict buf, u16 byte);
INTDEF size_t LIBCCALL libc_rawmemlenl(void const *__restrict buf, u32 byte);
INTDEF size_t LIBCCALL libc_rawmemlenq(void const *__restrict buf, u64 byte);
INTDEF void *LIBCCALL libc_rawmemrchr(void const *__restrict buf, int byte);
INTDEF u16 *LIBCCALL libc_rawmemrchrw(void const *__restrict buf, u16 byte);
INTDEF u32 *LIBCCALL libc_rawmemrchrl(void const *__restrict buf, u32 byte);
INTDEF u64 *LIBCCALL libc_rawmemrchrq(void const *__restrict buf, u64 byte);
INTDEF size_t LIBCCALL libc_rawmemrlen(void const *__restrict buf, int byte);
INTDEF size_t LIBCCALL libc_rawmemrlenw(void const *__restrict buf, u16 byte);
INTDEF size_t LIBCCALL libc_rawmemrlenl(void const *__restrict buf, u32 byte);
INTDEF size_t LIBCCALL libc_rawmemrlenq(void const *__restrict buf, u64 byte);
INTDEF void *LIBCCALL libc_memxchr(void const *__restrict buf, int byte, size_t num_bytes);
INTDEF u16 *LIBCCALL libc_memxchrw(void const *__restrict buf, u16 byte, size_t num_words);
INTDEF u32 *LIBCCALL libc_memxchrl(void const *__restrict buf, u32 byte, size_t num_dwords);
INTDEF u64 *LIBCCALL libc_memxchrq(void const *__restrict buf, u64 byte, size_t num_qwords);
INTDEF void *LIBCCALL libc_memrxchr(void const *__restrict buf, int byte, size_t num_bytes);
INTDEF u16 *LIBCCALL libc_memrxchrw(void const *__restrict buf, u16 byte, size_t num_words);
INTDEF u32 *LIBCCALL libc_memrxchrl(void const *__restrict buf, u32 byte, size_t num_dwords);
INTDEF u64 *LIBCCALL libc_memrxchrq(void const *__restrict buf, u64 byte, size_t num_qwords);
INTDEF void *LIBCCALL libc_memxend(void const *__restrict buf, int byte, size_t num_bytes);
INTDEF u16 *LIBCCALL libc_memxendw(void const *__restrict buf, u16 byte, size_t num_words);
INTDEF u32 *LIBCCALL libc_memxendl(void const *__restrict buf, u32 byte, size_t num_dwords);
INTDEF u64 *LIBCCALL libc_memxendq(void const *__restrict buf, u64 byte, size_t num_qwords);
INTDEF void *LIBCCALL libc_memrxend(void const *__restrict buf, int byte, size_t num_bytes);
INTDEF u16 *LIBCCALL libc_memrxendw(void const *__restrict buf, u16 byte, size_t num_words);
INTDEF u32 *LIBCCALL libc_memrxendl(void const *__restrict buf, u32 byte, size_t num_dwords);
INTDEF u64 *LIBCCALL libc_memrxendq(void const *__restrict buf, u64 byte, size_t num_qwords);
INTDEF size_t LIBCCALL libc_memxlen(void const *__restrict buf, int byte, size_t num_bytes);
INTDEF size_t LIBCCALL libc_memxlenw(void const *__restrict buf, u16 byte, size_t num_words);
INTDEF size_t LIBCCALL libc_memxlenl(void const *__restrict buf, u32 byte, size_t num_dwords);
INTDEF size_t LIBCCALL libc_memxlenq(void const *__restrict buf, u64 byte, size_t num_qwords);
INTDEF size_t LIBCCALL libc_memrxlen(void const *__restrict buf, int byte, size_t num_bytes);
INTDEF size_t LIBCCALL libc_memrxlenw(void const *__restrict buf, u16 byte, size_t num_words);
INTDEF size_t LIBCCALL libc_memrxlenl(void const *__restrict buf, u32 byte, size_t num_dwords);
INTDEF size_t LIBCCALL libc_memrxlenq(void const *__restrict buf, u64 byte, size_t num_qwords);
INTDEF void *LIBCCALL libc_rawmemxchr(void const *__restrict buf, int byte);
INTDEF u16 *LIBCCALL libc_rawmemxchrw(void const *__restrict buf, u16 byte);
INTDEF u32 *LIBCCALL libc_rawmemxchrl(void const *__restrict buf, u32 byte);
INTDEF u64 *LIBCCALL libc_rawmemxchrq(void const *__restrict buf, u64 byte);
INTDEF size_t LIBCCALL libc_rawmemxlen(void const *__restrict buf, int byte);
INTDEF size_t LIBCCALL libc_rawmemxlenw(void const *__restrict buf, u16 byte);
INTDEF size_t LIBCCALL libc_rawmemxlenl(void const *__restrict buf, u32 byte);
INTDEF size_t LIBCCALL libc_rawmemxlenq(void const *__restrict buf, u64 byte);
INTDEF void *LIBCCALL libc_rawmemrxchr(void const *__restrict buf, int byte);
INTDEF u16 *LIBCCALL libc_rawmemrxchrw(void const *__restrict buf, u16 byte);
INTDEF u32 *LIBCCALL libc_rawmemrxchrl(void const *__restrict buf, u32 byte);
INTDEF u64 *LIBCCALL libc_rawmemrxchrq(void const *__restrict buf, u64 byte);
INTDEF size_t LIBCCALL libc_rawmemrxlen(void const *__restrict buf, int byte);
INTDEF size_t LIBCCALL libc_rawmemrxlenw(void const *__restrict buf, u16 byte);
INTDEF size_t LIBCCALL libc_rawmemrxlenl(void const *__restrict buf, u32 byte);
INTDEF size_t LIBCCALL libc_rawmemrxlenq(void const *__restrict buf, u64 byte);
INTDEF u16 *LIBCCALL libc_mempatw(void *__restrict dst, u16 pattern, size_t n_bytes);
INTDEF u32 *LIBCCALL libc_mempatl(void *__restrict dst, u32 pattern, size_t n_bytes);
INTDEF u64 *LIBCCALL libc_mempatq(void *__restrict dst, u64 pattern, size_t n_bytes);
INTDEF int LIBCCALL libc_memcasecmp(void const *a, void const *b, size_t num_bytes);
#ifndef CONFIG_LIBC_LIMITED_API
INTDEF void LIBCCALL libc_swab(void const *__restrict from, void *__restrict to, unsigned int n_bytes);
INTDEF int LIBCCALL libc_memcasecmp_l(void const *a, void const *b, size_t num_bytes, locale_t locale);
INTDEF void *LIBCCALL libc_memmem(void const *haystack, size_t haystacklen, void const *needle, size_t needlelen);
INTDEF void *LIBCCALL libc_memcasemem(void const *haystack, size_t haystacklen, void const *needle, size_t needlelen);
INTDEF void *LIBCCALL libc_memcasemem_l(void const *haystack, size_t haystacklen, void const *needle, size_t needlelen, locale_t locale);
INTDEF void LIBCCALL libc_bcopy(void const *src, void *dst, size_t n);
INTDEF void LIBCCALL libc_bzero(void *__restrict s, size_t n);
#endif /* !CONFIG_LIBC_LIMITED_API */
#endif /* !GUARD_HYBRID_STRING_C */






/* ===================================================================================== */
/*     STRING                                                                            */
/* ===================================================================================== */
#if !defined(GUARD_HYBRID_STRING_C) || defined(__INTELLISENSE__)
INTDEF char *LIBCCALL libc_strend(char const *__restrict string);
INTDEF size_t LIBCCALL libc_strlen(char const *__restrict string);
INTDEF char *LIBCCALL libc_strnend(char const *__restrict string, size_t maxlen);
INTDEF size_t LIBCCALL libc_strnlen(char const *__restrict string, size_t maxlen);
INTDEF char *LIBCCALL libc_strchr(char const *__restrict string, int ch);
INTDEF char *LIBCCALL libc_strchrnul(char const *__restrict string, int ch);
INTDEF char *LIBCCALL libc_strrchr(char const *__restrict string, int ch);
INTDEF char *LIBCCALL libc_strrchrnul(char const *__restrict string, int ch);
INTDEF int LIBCCALL libc_strcmp(char const *a, char const *b);
INTDEF int LIBCCALL libc_strncmp(char const *a, char const *b, size_t max_chars);
INTDEF int LIBCCALL libc_strcasecmp(char const *a, char const *b);
INTDEF int LIBCCALL libc_strncasecmp(char const *a, char const *b, size_t max_chars);
#ifndef CONFIG_LIBC_LIMITED_API
INTDEF void *LIBCCALL libc_memfrob(void *__restrict s, size_t n);
INTDEF char *LIBCCALL libc_dirname(char *__restrict path);
INTDEF char *LIBCCALL libc_basename(char const *__restrict path);
INTDEF char *LIBCCALL libc_xpg_basename(char *__restrict path);
INTERN void *LIBCCALL libc_memccpy(void *__restrict dst, void const *__restrict src, int c, size_t n);
INTDEF char16_t *LIBCCALL libc_w16end(char16_t const *__restrict string);
INTDEF char32_t *LIBCCALL libc_w32end(char32_t const *__restrict string);
INTDEF size_t LIBCCALL libc_w16len(char16_t const *__restrict string);
INTDEF size_t LIBCCALL libc_w32len(char32_t const *__restrict string);
INTDEF char16_t *LIBCCALL libc_w16nend(char16_t const *__restrict string, size_t maxlen);
INTDEF char32_t *LIBCCALL libc_w32nend(char32_t const *__restrict string, size_t maxlen);
INTDEF size_t LIBCCALL libc_w16nlen(char16_t const *__restrict string, size_t maxlen);
INTDEF size_t LIBCCALL libc_w32nlen(char32_t const *__restrict string, size_t maxlen);
INTDEF char16_t *LIBCCALL libc_w16chr(char16_t const *__restrict string, wint_t ch);
INTDEF char32_t *LIBCCALL libc_w32chr(char32_t const *__restrict string, wint_t ch);
INTDEF char16_t *LIBCCALL libc_w16chrnul(char16_t const *__restrict string, wint_t ch);
INTDEF char32_t *LIBCCALL libc_w32chrnul(char32_t const *__restrict string, wint_t ch);
INTDEF char16_t *LIBCCALL libc_w16rchr(char16_t const *__restrict string, wint_t ch);
INTDEF char32_t *LIBCCALL libc_w32rchr(char32_t const *__restrict string, wint_t ch);
INTDEF char16_t *LIBCCALL libc_w16rchrnul(char16_t const *__restrict string, wint_t ch);
INTDEF char32_t *LIBCCALL libc_w32rchrnul(char32_t const *__restrict string, wint_t ch);
INTDEF int LIBCCALL libc_w16cmp(char16_t const *a, char16_t const *b);
INTDEF int LIBCCALL libc_w32cmp(char32_t const *a, char32_t const *b);
INTDEF int LIBCCALL libc_w16ncmp(char16_t const *a, char16_t const *b, size_t max_chars);
INTDEF int LIBCCALL libc_w32ncmp(char32_t const *a, char32_t const *b, size_t max_chars);
INTDEF int LIBCCALL libc_w16casecmp(char16_t const *a, char16_t const *b);
INTDEF int LIBCCALL libc_w32casecmp(char32_t const *a, char32_t const *b);
INTDEF int LIBCCALL libc_w16ncasecmp(char16_t const *a, char16_t const *b, size_t max_chars);
INTDEF int LIBCCALL libc_w32ncasecmp(char32_t const *a, char32_t const *b, size_t max_chars);
INTDEF int LIBCCALL libc_strcasecmp_l(char const *a, char const *b, locale_t locale);
INTDEF int LIBCCALL libc_w16casecmp_l(char16_t const *a, char16_t const *b, locale_t locale);
INTDEF int LIBCCALL libc_w32casecmp_l(char32_t const *a, char32_t const *b, locale_t locale);
INTDEF int LIBCCALL libc_strncasecmp_l(char const *a, char const *b, size_t max_chars, locale_t locale);
INTDEF int LIBCCALL libc_w16ncasecmp_l(char16_t const *a, char16_t const *b, size_t max_chars, locale_t locale);
INTDEF int LIBCCALL libc_w32ncasecmp_l(char32_t const *a, char32_t const *b, size_t max_chars, locale_t locale);
INTDEF char *LIBCCALL libc_strnchr(char const *__restrict haystack, int needle, size_t maxlen);
INTDEF char16_t *LIBCCALL libc_w16nchr(char16_t const *__restrict haystack, wint_t needle, size_t maxlen);
INTDEF char32_t *LIBCCALL libc_w32nchr(char32_t const *__restrict haystack, wint_t needle, size_t maxlen);
INTDEF char *LIBCCALL libc_strnrchr(char const *__restrict haystack, int needle, size_t maxlen);
INTDEF char16_t *LIBCCALL libc_w16nrchr(char16_t const *__restrict haystack, wint_t needle, size_t maxlen);
INTDEF char32_t *LIBCCALL libc_w32nrchr(char32_t const *__restrict haystack, wint_t needle, size_t maxlen);
INTDEF char *LIBCCALL libc_strnchrnul(char const *__restrict haystack, int needle, size_t maxlen);
INTDEF char16_t *LIBCCALL libc_w16nchrnul(char16_t const *__restrict haystack, wint_t needle, size_t maxlen);
INTDEF char32_t *LIBCCALL libc_w32nchrnul(char32_t const *__restrict haystack, wint_t needle, size_t maxlen);
INTDEF char *LIBCCALL libc_strnrchrnul(char const *__restrict haystack, int needle, size_t maxlen);
INTDEF char16_t *LIBCCALL libc_w16nrchrnul(char16_t const *__restrict haystack, wint_t needle, size_t maxlen);
INTDEF char32_t *LIBCCALL libc_w32nrchrnul(char32_t const *__restrict haystack, wint_t needle, size_t maxlen);
INTDEF size_t LIBCCALL libc_stroff(char const *__restrict haystack, int needle);
INTDEF size_t LIBCCALL libc_w16off(char16_t const *__restrict haystack, wint_t needle);
INTDEF size_t LIBCCALL libc_w32off(char32_t const *__restrict haystack, wint_t needle);
INTDEF size_t LIBCCALL libc_strroff(char const *__restrict haystack, int needle);
INTDEF size_t LIBCCALL libc_w16roff(char16_t const *__restrict haystack, wint_t needle);
INTDEF size_t LIBCCALL libc_w32roff(char32_t const *__restrict haystack, wint_t needle);
INTDEF size_t LIBCCALL libc_strnoff(char const *__restrict haystack, int needle, size_t maxlen);
INTDEF size_t LIBCCALL libc_w16noff(char16_t const *__restrict haystack, wint_t needle, size_t maxlen);
INTDEF size_t LIBCCALL libc_w32noff(char32_t const *__restrict haystack, wint_t needle, size_t maxlen);
INTDEF size_t LIBCCALL libc_strnroff(char const *__restrict haystack, int needle, size_t maxlen);
INTDEF size_t LIBCCALL libc_w16nroff(char16_t const *__restrict haystack, wint_t needle, size_t maxlen);
INTDEF size_t LIBCCALL libc_w32nroff(char32_t const *__restrict haystack, wint_t needle, size_t maxlen);
INTDEF char *LIBCCALL libc_stpcpy(char *__restrict dst, char const *__restrict src);
INTDEF char16_t *LIBCCALL libc_w16pcpy(char16_t *__restrict dst, char16_t const *__restrict src);
INTDEF char32_t *LIBCCALL libc_w32pcpy(char32_t *__restrict dst, char32_t const *__restrict src);
INTDEF char *LIBCCALL libc_stpncpy(char *__restrict dst, char const *__restrict src, size_t n);
INTDEF char16_t *LIBCCALL libc_w16pncpy(char16_t *__restrict dst, char16_t const *__restrict src, size_t n);
INTDEF char32_t *LIBCCALL libc_w32pncpy(char32_t *__restrict dst, char32_t const *__restrict src, size_t n);
INTDEF char *LIBCCALL libc_strcpy(char *__restrict dst, char const *__restrict src);
INTDEF char16_t *LIBCCALL libc_w16cpy(char16_t *__restrict dst, char16_t const *__restrict src);
INTDEF char32_t *LIBCCALL libc_w32cpy(char32_t *__restrict dst, char32_t const *__restrict src);
INTDEF char *LIBCCALL libc_strncpy(char *__restrict dst, char const *__restrict src, size_t n);
INTDEF char16_t *LIBCCALL libc_w16ncpy(char16_t *__restrict dst, char16_t const *__restrict src, size_t n);
INTDEF char32_t *LIBCCALL libc_w32ncpy(char32_t *__restrict dst, char32_t const *__restrict src, size_t n);
INTDEF char *LIBCCALL libc_strcat(char *__restrict dst, char const *__restrict src);
INTDEF char16_t *LIBCCALL libc_w16cat(char16_t *__restrict dst, char16_t const *__restrict src);
INTDEF char32_t *LIBCCALL libc_w32cat(char32_t *__restrict dst, char32_t const *__restrict src);
INTDEF char *LIBCCALL libc_strncat(char *__restrict dst, char const *__restrict src, size_t n);
INTDEF char16_t *LIBCCALL libc_w16ncat(char16_t *__restrict dst, char16_t const *__restrict src, size_t n);
INTDEF char32_t *LIBCCALL libc_w32ncat(char32_t *__restrict dst, char32_t const *__restrict src, size_t n);
INTDEF char *LIBCCALL libc_strstr(char const *haystack, char const *needle);
INTDEF char16_t *LIBCCALL libc_w16str(char16_t const *haystack, char16_t const *needle);
INTDEF char32_t *LIBCCALL libc_w32str(char32_t const *haystack, char32_t const *needle);
INTDEF char *LIBCCALL libc_strcasestr(char const *haystack, char const *needle);
INTDEF char16_t *LIBCCALL libc_w16casestr(char16_t const *haystack, char16_t const *needle);
INTDEF char32_t *LIBCCALL libc_w32casestr(char32_t const *haystack, char32_t const *needle);
INTDEF char *LIBCCALL libc_strcasestr_l(char const *haystack, char const *needle, locale_t locale);
INTDEF char16_t *LIBCCALL libc_w16casestr_l(char16_t const *haystack, char16_t const *needle, locale_t locale);
INTDEF char32_t *LIBCCALL libc_w32casestr_l(char32_t const *haystack, char32_t const *needle, locale_t locale);
INTERN size_t LIBCCALL libc_fuzzy_memcmp(void const *a, size_t a_bytes, void const *b, size_t b_bytes);
INTERN size_t LIBCCALL libc_fuzzy_memcmpw(void const *a, size_t a_words, void const *b, size_t b_words);
INTERN size_t LIBCCALL libc_fuzzy_memcmpl(void const *a, size_t a_dwords, void const *b, size_t b_dwords);
INTERN size_t LIBCCALL libc_fuzzy_memcasecmp(void const *a, size_t a_bytes, void const *b, size_t b_bytes);
INTERN size_t LIBCCALL libc_fuzzy_memcasecmpw(void const *a, size_t a_words, void const *b, size_t b_words);
INTERN size_t LIBCCALL libc_fuzzy_memcasecmpl(void const *a, size_t a_dwords, void const *b, size_t b_dwords);
INTERN size_t LIBCCALL libc_fuzzy_memcasecmp_l(void const *a, size_t a_bytes, void const *b, size_t b_bytes, locale_t locale);
INTERN size_t LIBCCALL libc_fuzzy_memcasecmpw_l(void const *a, size_t a_words, void const *b, size_t b_words, locale_t locale);
INTERN size_t LIBCCALL libc_fuzzy_memcasecmpl_l(void const *a, size_t a_dwords, void const *b, size_t b_dwords, locale_t locale);
INTERN size_t LIBCCALL libc_fuzzy_strcmp(char const *a, char const *b);
INTERN size_t LIBCCALL libc_fuzzy_w16cmp(char16_t const *a, char16_t const *b);
INTERN size_t LIBCCALL libc_fuzzy_w32cmp(char32_t const *a, char32_t const *b);
INTERN size_t LIBCCALL libc_fuzzy_strcasecmp(char const *a, char const *b);
INTERN size_t LIBCCALL libc_fuzzy_w16casecmp(char16_t const *a, char16_t const *b);
INTERN size_t LIBCCALL libc_fuzzy_w32casecmp(char32_t const *a, char32_t const *b);
INTERN size_t LIBCCALL libc_fuzzy_strcasecmp_l(char const *a, char const *b, locale_t locale);
INTERN size_t LIBCCALL libc_fuzzy_w16casecmp_l(char16_t const *a, char16_t const *b, locale_t locale);
INTERN size_t LIBCCALL libc_fuzzy_w32casecmp_l(char32_t const *a, char32_t const *b, locale_t locale);
INTDEF int LIBCCALL libc_wildstrcmp(char const *pattern, char const *string);
INTDEF int LIBCCALL libc_wildw16cmp(char16_t const *pattern, char16_t const *string);
INTDEF int LIBCCALL libc_wildw32cmp(char32_t const *pattern, char32_t const *string);
INTDEF int LIBCCALL libc_wildstrcasecmp(char const *pattern, char const *string);
INTDEF int LIBCCALL libc_wildw16casecmp(char16_t const *pattern, char16_t const *string);
INTDEF int LIBCCALL libc_wildw32casecmp(char32_t const *pattern, char32_t const *string);
INTDEF int LIBCCALL libc_wildstrcasecmp_l(char const *pattern, char const *string, locale_t locale);
INTDEF int LIBCCALL libc_wildw16casecmp_l(char16_t const *pattern, char16_t const *string, locale_t locale);
INTDEF int LIBCCALL libc_wildw32casecmp_l(char32_t const *pattern, char32_t const *string, locale_t locale);
INTDEF int LIBCCALL libc_strverscmp(char const *s1, char const *s2);
INTDEF int LIBCCALL libc_w16verscmp(char16_t const *s1, char16_t const *s2);
INTDEF int LIBCCALL libc_w32verscmp(char32_t const *s1, char32_t const *s2);
INTDEF char *LIBCCALL libc_strsep(char **__restrict stringp, char const *__restrict delim);
INTDEF char16_t *LIBCCALL libc_w16sep(char16_t **__restrict stringp, char16_t const *__restrict delim);
INTDEF char32_t *LIBCCALL libc_w32sep(char32_t **__restrict stringp, char32_t const *__restrict delim);
INTDEF char *LIBCCALL libc_index(char const *__restrict haystack, int needle);
INTDEF char16_t *LIBCCALL libc_w16index(char16_t const *__restrict haystack, wint_t needle);
INTDEF char32_t *LIBCCALL libc_w32index(char32_t const *__restrict haystack, wint_t needle);
INTDEF char *LIBCCALL libc_rindex(char const *__restrict haystack, int needle);
INTDEF char16_t *LIBCCALL libc_w16rindex(char16_t const *__restrict haystack, wint_t needle);
INTDEF char32_t *LIBCCALL libc_w32rindex(char32_t const *__restrict haystack, wint_t needle);
INTDEF size_t LIBCCALL libc_strspn(char const *s, char const *reject);
INTDEF size_t LIBCCALL libc_w16spn(char16_t const *s, char16_t const *reject);
INTDEF size_t LIBCCALL libc_w32spn(char32_t const *s, char32_t const *reject);
INTDEF size_t LIBCCALL libc_strcspn(char const *s, char const *reject);
INTDEF size_t LIBCCALL libc_w16cspn(char16_t const *s, char16_t const *reject);
INTDEF size_t LIBCCALL libc_w32cspn(char32_t const *s, char32_t const *reject);
INTDEF char *LIBCCALL libc_strpbrk(char const *s, char const *accept);
INTDEF char16_t *LIBCCALL libc_w16pbrk(char16_t const *s, char16_t const *accept);
INTDEF char32_t *LIBCCALL libc_w32pbrk(char32_t const *s, char32_t const *accept);
INTDEF char *LIBCCALL libc_strtok_r(char *__restrict s, char const *__restrict delim, char **__restrict save_ptr);
INTDEF char16_t *LIBCCALL libc_w16tok_r(char16_t *__restrict s, char16_t const *__restrict delim, char16_t **__restrict save_ptr);
INTDEF char32_t *LIBCCALL libc_w32tok_r(char32_t *__restrict s, char32_t const *__restrict delim, char32_t **__restrict save_ptr);
INTDEF char *LIBCCALL libc_strtok(char *__restrict s, char const *__restrict delim);
INTDEF char16_t *LIBCCALL libc_w16tok(char16_t *__restrict s, char16_t const *__restrict delim);
INTDEF char32_t *LIBCCALL libc_w32tok(char32_t *__restrict s, char32_t const *__restrict delim);
INTDEF int LIBCCALL libc_strcoll(char const *s1, char const *s2);
INTDEF int LIBCCALL libc_w16coll(char16_t const *s1, char16_t const *s2);
INTDEF int LIBCCALL libc_w32coll(char32_t const *s1, char32_t const *s2);
INTDEF int LIBCCALL libc_strcoll_l(char const *s1, char const *s2, locale_t l);
INTDEF int LIBCCALL libc_w16coll_l(char16_t const *s1, char16_t const *s2, locale_t l);
INTDEF int LIBCCALL libc_w32coll_l(char32_t const *s1, char32_t const *s2, locale_t l);
INTDEF int LIBCCALL libc_strncoll(char const *s1, char const *s2, size_t max_chars);
INTDEF int LIBCCALL libc_w16ncoll(char16_t const *s1, char16_t const *s2, size_t max_chars);
INTDEF int LIBCCALL libc_w32ncoll(char32_t const *s1, char32_t const *s2, size_t max_chars);
INTDEF int LIBCCALL libc_strncoll_l(char const *s1, char const *s2, size_t max_chars, locale_t l);
INTDEF int LIBCCALL libc_w16ncoll_l(char16_t const *s1, char16_t const *s2, size_t max_chars, locale_t l);
INTDEF int LIBCCALL libc_w32ncoll_l(char32_t const *s1, char32_t const *s2, size_t max_chars, locale_t l);
INTDEF int LIBCCALL libc_strcasecoll(char const *s1, char const *s2);
INTDEF int LIBCCALL libc_w16casecoll(char16_t const *s1, char16_t const *s2);
INTDEF int LIBCCALL libc_w32casecoll(char32_t const *s1, char32_t const *s2);
INTDEF int LIBCCALL libc_strcasecoll_l(char const *s1, char const *s2, locale_t l);
INTDEF int LIBCCALL libc_w16casecoll_l(char16_t const *s1, char16_t const *s2, locale_t l);
INTDEF int LIBCCALL libc_w32casecoll_l(char32_t const *s1, char32_t const *s2, locale_t l);
INTDEF int LIBCCALL libc_strncasecoll(char const *s1, char const *s2, size_t max_chars);
INTDEF int LIBCCALL libc_w16ncasecoll(char16_t const *s1, char16_t const *s2, size_t max_chars);
INTDEF int LIBCCALL libc_w32ncasecoll(char32_t const *s1, char32_t const *s2, size_t max_chars);
INTDEF int LIBCCALL libc_strncasecoll_l(char const *s1, char const *s2, size_t max_chars, locale_t l);
INTDEF int LIBCCALL libc_w16ncasecoll_l(char16_t const *s1, char16_t const *s2, size_t max_chars, locale_t l);
INTDEF int LIBCCALL libc_w32ncasecoll_l(char32_t const *s1, char32_t const *s2, size_t max_chars, locale_t l);
INTDEF size_t LIBCCALL libc_strxfrm(char *__restrict dst, char const *__restrict src, size_t n);
INTDEF size_t LIBCCALL libc_w16xfrm(char16_t *__restrict dst, char16_t const *__restrict src, size_t n);
INTDEF size_t LIBCCALL libc_w32xfrm(char32_t *__restrict dst, char32_t const *__restrict src, size_t n);
INTDEF size_t LIBCCALL libc_strxfrm_l(char *__restrict dst, char const *__restrict src, size_t n, locale_t l);
INTDEF size_t LIBCCALL libc_w16xfrm_l(char16_t *__restrict dst, char16_t const *__restrict src, size_t n, locale_t l);
INTDEF size_t LIBCCALL libc_w32xfrm_l(char32_t *__restrict dst, char32_t const *__restrict src, size_t n, locale_t l);
INTDEF void *LIBCCALL libc_memrev(void *__restrict buf, size_t num_bytes);
INTDEF void *LIBCCALL libc_memrevw(void *__restrict buf, size_t num_words);
INTDEF void *LIBCCALL libc_memrevl(void *__restrict buf, size_t num_dwords);
INTDEF void *LIBCCALL libc_memrevq(void *__restrict buf, size_t num_qwords);
INTDEF char *LIBCCALL libc_strrev(char *__restrict str);
INTDEF char16_t *LIBCCALL libc_w16rev(char16_t *__restrict str);
INTDEF char32_t *LIBCCALL libc_w32rev(char32_t *__restrict str);
INTDEF char *LIBCCALL libc_strnrev(char *__restrict str, size_t max_chars);
INTDEF char16_t *LIBCCALL libc_w16nrev(char16_t *__restrict str, size_t max_chars);
INTDEF char32_t *LIBCCALL libc_w32nrev(char32_t *__restrict str, size_t max_chars);
INTDEF char *LIBCCALL libc_strlwr(char *__restrict str);
INTDEF char16_t *LIBCCALL libc_w16lwr(char16_t *__restrict str);
INTDEF char32_t *LIBCCALL libc_w32lwr(char32_t *__restrict str);
INTDEF char *LIBCCALL libc_strlwr_l(char *__restrict str, locale_t locale);
INTDEF char16_t *LIBCCALL libc_w16lwr_l(char16_t *__restrict str, locale_t locale);
INTDEF char32_t *LIBCCALL libc_w32lwr_l(char32_t *__restrict str, locale_t locale);
INTDEF char *LIBCCALL libc_strupr(char *__restrict str);
INTDEF char16_t *LIBCCALL libc_w16upr(char16_t *__restrict str);
INTDEF char32_t *LIBCCALL libc_w32upr(char32_t *__restrict str);
INTDEF char *LIBCCALL libc_strupr_l(char *__restrict str, locale_t locale);
INTDEF char16_t *LIBCCALL libc_w16upr_l(char16_t *__restrict str, locale_t locale);
INTDEF char32_t *LIBCCALL libc_w32upr_l(char32_t *__restrict str, locale_t locale);
INTDEF char *LIBCCALL libc_strset(char *__restrict str, int chr);
INTDEF char16_t *LIBCCALL libc_w16set(char16_t *__restrict str, wint_t chr);
INTDEF char32_t *LIBCCALL libc_w32set(char32_t *__restrict str, wint_t chr);
INTDEF char *LIBCCALL libc_strnset(char *__restrict str, int chr, size_t maxlen);
INTDEF char16_t *LIBCCALL libc_w16nset(char16_t *__restrict str, wint_t chr, size_t maxlen);
INTDEF char32_t *LIBCCALL libc_w32nset(char32_t *__restrict str, wint_t chr, size_t maxlen);
INTDEF char *LIBCCALL libc_strfry(char *__restrict string);
INTDEF char16_t *LIBCCALL libc_w16fry(char16_t *__restrict string);
INTDEF char32_t *LIBCCALL libc_w32fry(char32_t *__restrict string);
INTDEF s32 LIBCCALL libc_strto32(char const *__restrict nptr, char **endptr, int base);
INTDEF s32 LIBCCALL libc_w16to32(char16_t const *__restrict nptr, char16_t **endptr, int base);
INTDEF s32 LIBCCALL libc_w32to32(char32_t const *__restrict nptr, char32_t **endptr, int base);
INTDEF s32 LIBCCALL libc_strto32_l(char const *__restrict nptr, char **endptr, int base, locale_t locale);
INTDEF s32 LIBCCALL libc_w16to32_l(char16_t const *__restrict nptr, char16_t **endptr, int base, locale_t locale);
INTDEF s32 LIBCCALL libc_w32to32_l(char32_t const *__restrict nptr, char32_t **endptr, int base, locale_t locale);
INTDEF s64 LIBCCALL libc_strto64(char const *__restrict nptr, char **endptr, int base);
INTDEF s64 LIBCCALL libc_w16to64(char16_t const *__restrict nptr, char16_t **endptr, int base);
INTDEF s64 LIBCCALL libc_w32to64(char32_t const *__restrict nptr, char32_t **endptr, int base);
INTDEF s64 LIBCCALL libc_strto64_l(char const *__restrict nptr, char **endptr, int base, locale_t locale);
INTDEF s64 LIBCCALL libc_w16to64_l(char16_t const *__restrict nptr, char16_t **endptr, int base, locale_t locale);
INTDEF s64 LIBCCALL libc_w32to64_l(char32_t const *__restrict nptr, char32_t **endptr, int base, locale_t locale);
INTDEF u32 LIBCCALL libc_strtou32(char const *__restrict nptr, char **endptr, int base);
INTDEF u32 LIBCCALL libc_w16tou32(char16_t const *__restrict nptr, char16_t **endptr, int base);
INTDEF u32 LIBCCALL libc_w32tou32(char32_t const *__restrict nptr, char32_t **endptr, int base);
INTDEF u32 LIBCCALL libc_strtou32_l(char const *__restrict nptr, char **endptr, int base, locale_t locale);
INTDEF u32 LIBCCALL libc_w16tou32_l(char16_t const *__restrict nptr, char16_t **endptr, int base, locale_t locale);
INTDEF u32 LIBCCALL libc_w32tou32_l(char32_t const *__restrict nptr, char32_t **endptr, int base, locale_t locale);
INTDEF u64 LIBCCALL libc_strtou64(char const *__restrict nptr, char **endptr, int base);
INTDEF u64 LIBCCALL libc_w16tou64(char16_t const *__restrict nptr, char16_t **endptr, int base);
INTDEF u64 LIBCCALL libc_w32tou64(char32_t const *__restrict nptr, char32_t **endptr, int base);
INTDEF u64 LIBCCALL libc_strtou64_l(char const *__restrict nptr, char **endptr, int base, locale_t locale);
INTDEF u64 LIBCCALL libc_w16tou64_l(char16_t const *__restrict nptr, char16_t **endptr, int base, locale_t locale);
INTDEF u64 LIBCCALL libc_w32tou64_l(char32_t const *__restrict nptr, char32_t **endptr, int base, locale_t locale);
INTDEF float LIBCCALL libc_strtof(char const *__restrict nptr, char **endptr);
INTDEF float LIBCCALL libc_w16tof(char16_t const *__restrict nptr, char16_t **endptr);
INTDEF float LIBCCALL libc_w32tof(char32_t const *__restrict nptr, char32_t **endptr);
INTDEF float LIBCCALL libc_strtof_l(char const *__restrict nptr, char **endptr, locale_t locale);
INTDEF float LIBCCALL libc_w16tof_l(char16_t const *__restrict nptr, char16_t **endptr, locale_t locale);
INTDEF float LIBCCALL libc_w32tof_l(char32_t const *__restrict nptr, char32_t **endptr, locale_t locale);
INTDEF double LIBCCALL libc_strtod(char const *__restrict nptr, char **endptr);
INTDEF double LIBCCALL libc_w16tod(char16_t const *__restrict nptr, char16_t **endptr);
INTDEF double LIBCCALL libc_w32tod(char32_t const *__restrict nptr, char32_t **endptr);
INTDEF double LIBCCALL libc_strtod_l(char const *__restrict nptr, char **endptr, locale_t locale);
INTDEF double LIBCCALL libc_w16tod_l(char16_t const *__restrict nptr, char16_t **endptr, locale_t locale);
INTDEF double LIBCCALL libc_w32tod_l(char32_t const *__restrict nptr, char32_t **endptr, locale_t locale);
INTDEF long double LIBCCALL libc_strtold(char const *__restrict nptr, char **endptr);
INTDEF long double LIBCCALL libc_w16told(char16_t const *__restrict nptr, char16_t **endptr);
INTDEF long double LIBCCALL libc_w32told(char32_t const *__restrict nptr, char32_t **endptr);
INTDEF long double LIBCCALL libc_strtold_l(char const *__restrict nptr, char **endptr, locale_t locale);
INTDEF long double LIBCCALL libc_w16told_l(char16_t const *__restrict nptr, char16_t **endptr, locale_t locale);
INTDEF long double LIBCCALL libc_w32told_l(char32_t const *__restrict nptr, char32_t **endptr, locale_t locale);
INTDEF s32 LIBCCALL libc_ato32(char const *__restrict nptr);
INTDEF s32 LIBCCALL libc_aw16to32(char16_t const *__restrict nptr);
INTDEF s32 LIBCCALL libc_aw32to32(char32_t const *__restrict nptr);
INTDEF s32 LIBCCALL libc_ato32_l(char const *__restrict nptr, locale_t locale);
INTDEF s32 LIBCCALL libc_aw16to32_l(char16_t const *__restrict nptr, locale_t locale);
INTDEF s32 LIBCCALL libc_aw32to32_l(char32_t const *__restrict nptr, locale_t locale);
INTDEF s64 LIBCCALL libc_ato64(char const *__restrict nptr);
INTDEF s64 LIBCCALL libc_aw16to64(char16_t const *__restrict nptr);
INTDEF s64 LIBCCALL libc_aw32to64(char32_t const *__restrict nptr);
INTDEF s64 LIBCCALL libc_ato64_l(char const *__restrict nptr, locale_t locale);
INTDEF s64 LIBCCALL libc_aw16to64_l(char16_t const *__restrict nptr, locale_t locale);
INTDEF s64 LIBCCALL libc_aw32to64_l(char32_t const *__restrict nptr, locale_t locale);
INTDEF u32 LIBCCALL libc_atou32(char const *__restrict nptr);
INTDEF u32 LIBCCALL libc_aw16tou32(char16_t const *__restrict nptr);
INTDEF u32 LIBCCALL libc_aw32tou32(char32_t const *__restrict nptr);
INTDEF u32 LIBCCALL libc_atou32_l(char const *__restrict nptr, locale_t locale);
INTDEF u32 LIBCCALL libc_aw16tou32_l(char16_t const *__restrict nptr, locale_t locale);
INTDEF u32 LIBCCALL libc_aw32tou32_l(char32_t const *__restrict nptr, locale_t locale);
INTDEF u64 LIBCCALL libc_atou64(char const *__restrict nptr);
INTDEF u64 LIBCCALL libc_aw16tou64(char16_t const *__restrict nptr);
INTDEF u64 LIBCCALL libc_aw32tou64(char32_t const *__restrict nptr);
INTDEF u64 LIBCCALL libc_atou64_l(char const *__restrict nptr, locale_t locale);
INTDEF u64 LIBCCALL libc_aw16tou64_l(char16_t const *__restrict nptr, locale_t locale);
INTDEF u64 LIBCCALL libc_aw32tou64_l(char32_t const *__restrict nptr, locale_t locale);
INTDEF float LIBCCALL libc_atof(char const *__restrict nptr);
INTDEF float LIBCCALL libc_aw16tof(char16_t const *__restrict nptr);
INTDEF float LIBCCALL libc_aw32tof(char32_t const *__restrict nptr);
INTDEF float LIBCCALL libc_atof_l(char const *__restrict nptr, locale_t locale);
INTDEF float LIBCCALL libc_aw16tof_l(char16_t const *__restrict nptr, locale_t locale);
INTDEF float LIBCCALL libc_aw32tof_l(char32_t const *__restrict nptr, locale_t locale);
INTDEF double LIBCCALL libc_atod(char const *__restrict nptr);
INTDEF double LIBCCALL libc_aw16tod(char16_t const *__restrict nptr);
INTDEF double LIBCCALL libc_aw32tod(char32_t const *__restrict nptr);
INTDEF double LIBCCALL libc_atod_l(char const *__restrict nptr, locale_t locale);
INTDEF double LIBCCALL libc_aw16tod_l(char16_t const *__restrict nptr, locale_t locale);
INTDEF double LIBCCALL libc_aw32tod_l(char32_t const *__restrict nptr, locale_t locale);
INTDEF long double LIBCCALL libc_atold(char const *__restrict nptr);
INTDEF long double LIBCCALL libc_aw16told(char16_t const *__restrict nptr);
INTDEF long double LIBCCALL libc_aw32told(char32_t const *__restrict nptr);
INTDEF long double LIBCCALL libc_atold_l(char const *__restrict nptr, locale_t locale);
INTDEF long double LIBCCALL libc_aw16told_l(char16_t const *__restrict nptr, locale_t locale);
INTDEF long double LIBCCALL libc_aw32told_l(char32_t const *__restrict nptr, locale_t locale);
#endif /* !CONFIG_LIBC_LIMITED_API */
#endif /* !GUARD_HYBRID_STRING_C */






/* ===================================================================================== */
/*     RANDOM                                                                            */
/* ===================================================================================== */
#ifdef __KERNEL__
typedef __UINT32_TYPE__ rand_t;
typedef __UINT32_TYPE__ rand_seed_t;
typedef __UINT32_TYPE__ rand_seed_r_t;
#else
typedef int          rand_t;
typedef long         rand_seed_t;
typedef unsigned int rand_seed_r_t;
#endif
INTDEF void LIBCCALL libc_srand(rand_seed_t seed);
INTERN rand_t LIBCCALL libc_rand(void);
#ifndef CONFIG_LIBC_LIMITED_API
INTERN rand_t LIBCCALL libc_rand_r(rand_seed_r_t *__restrict pseed);
INTERN long LIBCCALL libc_random(void);
INTDEF void LIBCCALL libc_srandom(unsigned int seed);
#endif /* !CONFIG_LIBC_LIMITED_API */






/* ===================================================================================== */
/*     CONTEXT                                                                           */
/* ===================================================================================== */
struct cpu_context;
INTDEF int FCALL libc_cpu_getcontext(struct cpu_context *__restrict old_context);
INTDEF void FCALL libc_cpu_xchcontext(struct cpu_context const *__restrict new_context, struct cpu_context *__restrict old_context);
INTDEF ATTR_NORETURN void FCALL libc_cpu_setcontext(struct cpu_context const *__restrict new_context);






/* ===================================================================================== */
/*     EXCEPT                                                                            */
/* ===================================================================================== */
struct exception_info;
INTDEF ATTR_CONST ATTR_RETNONNULL struct exception_info *(FCALL libc_error_info)(void);
INTDEF bool FCALL libc_error_throw_resumable(u16 code);
INTDEF bool FCALL libc_error_throw_resumable_ex(u16 code, struct exception_data *__restrict data);
INTDEF bool ATTR_CDECL libc_error_throw_resumablef(u16 code, ...);
INTDEF ATTR_NORETURN void FCALL libc_error_throw(u16 code);
INTDEF ATTR_NORETURN void FCALL libc_error_throw_ex(u16 code, struct exception_data *__restrict data);
INTDEF ATTR_NORETURN void ATTR_CDECL libc_error_throwf(u16 code, ...);
INTDEF bool FCALL libc_error_throw_current(void);
INTDEF ATTR_NORETURN void FCALL libc_error_rethrow(void);
INTDEF ATTR_NORETURN void FCALL libc_error_continue(int retry);
INTDEF void FCALL libc_error_except(int mode);
INTDEF u16 (FCALL libc_error_code)(void);
INTDEF ATTR_NORETURN void FCALL libc_error_unhandled_exception(void);
INTDEF ATTR_NORETURN void FCALL libc_error_rethrow_at(struct cpu_context *__restrict context, int ip_is_after_faulting);
INTDEF void ATTR_CDECL libc_error_printf(char const *__restrict reason, ...);
INTDEF void LIBCCALL libc_error_vprintf(char const *__restrict reason, va_list args);
INTDEF errno_t FCALL libc_exception_errno(struct exception_info *__restrict info);
#ifndef CONFIG_LIBC_LIMITED_API
INTDEF int FCALL libc_except_errno(void);
INTDEF void ATTR_CDECL libc_error_fprintf(__FILE *__restrict fp, char const *__restrict reason, ...);
INTDEF void LIBCCALL libc_error_vfprintf(__FILE *__restrict fp, char const *__restrict reason, va_list args);
#endif /* !CONFIG_LIBC_LIMITED_API */

#if 0
#undef __ERROR_CURRENT_RETHROW
#undef __ERROR_CURRENT_CONTINUE
#undef __ERROR_CURRENT_EXCEPT
#ifndef __ERROR_CURRENT_RETHROW
#define __ERROR_CURRENT_RETHROW  libc_error_rethrow
#define __ERROR_CURRENT_CONTINUE libc_error_continue
#define __ERROR_CURRENT_EXCEPT   libc_error_except
#endif /* !__ERROR_CURRENT_RETHROW */
#endif






/* ===================================================================================== */
/*     INSTRUCTION                                                                       */
/* ===================================================================================== */
#ifndef GUARD_HYBRID_I386_KOS_INSTRUCTION_C
INTDEF void *FCALL libc_prev_instruction(void *__restrict ip);
INTDEF void *FCALL libc_next_instruction(void *__restrict ip);
#endif


#endif /* __CC__ */

DECL_END

#endif /* !GUARD_HYBRID_HYBRID_H */
