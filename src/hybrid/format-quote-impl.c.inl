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
#ifdef __INTELLISENSE__
#include "format-printer.c"
#include "../libs/libc/libc.h"
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#define FORMAT_OPTION_LOCALE     1 /* Enable locale support. */
#endif

#include <format-printer.h>
#include <except.h>
#include <stdbool.h>

#ifndef FORMAT_OPTION_CHARTYPE
#error "Must #define FORMAT_OPTION_CHARTYPE"
#endif

#ifndef format_T_char
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#   define format_T_char       char
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define format_T_char       char16_t
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR32
#   define format_T_char       char32_t
#else
#   error "Invalid character type"
#endif
#endif /* !format_T_char */

#ifndef unsigned_format_T_char
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#   define unsigned_format_T_char       unsigned char
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define unsigned_format_T_char       u16
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR32
#   define unsigned_format_T_char       u32
#else
#   error "Invalid character type"
#endif
#endif /* !unsigned_format_T_char */

#ifndef LIBC_FORMAT_QUOTE
#ifdef FORMAT_OPTION_LOCALE
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_QUOTE   libc_format_quote_l
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_QUOTE   libc_format_w16quote_l
#   else
#      define LIBC_FORMAT_QUOTE   libc_format_w32quote_l
#   endif
#else /* FORMAT_OPTION_LOCALE */
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_QUOTE   libc_format_quote
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_QUOTE   libc_format_w16quote
#   else
#      define LIBC_FORMAT_QUOTE   libc_format_w32quote
#   endif
#endif /* !FORMAT_OPTION_LOCALE */
#endif /* !LIBC_FORMAT_QUOTE */

#ifndef LIBC_FORMAT_PRINTF
#ifdef FORMAT_OPTION_LOCALE
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_PRINTF   libc_format_printf_l
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_PRINTF   libc_format_w16printf_l
#   else
#      define LIBC_FORMAT_PRINTF   libc_format_w32printf_l
#   endif
#else /* FORMAT_OPTION_LOCALE */
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_PRINTF   libc_format_printf
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_PRINTF   libc_format_w16printf
#   else
#      define LIBC_FORMAT_PRINTF   libc_format_w32printf
#   endif
#endif /* !FORMAT_OPTION_LOCALE */
#endif /* !LIBC_FORMAT_PRINTF */

#ifndef PFORMATPRINTER
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#   define PFORMATPRINTER      pformatprinter
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define PFORMATPRINTER      pw16formatprinter
#else
#   define PFORMATPRINTER      pw32formatprinter
#endif
#endif /* !PFORMATPRINTER */


#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#   ifdef FORMAT_OPTION_LOCALE
#      define ISPRINT(x)  libc_isprint_l(x,locale)
#   else
#      define ISPRINT(x)  libc_isprint(x)
#   endif
#else
#   ifdef FORMAT_OPTION_LOCALE
#      define ISPRINT(x)  libc_iswprint_l(x,locale)
#   else
#      define ISPRINT(x)  libc_iswprint(x)
#   endif
#endif


DECL_BEGIN

#ifdef FORMAT_OPTION_LOCALE
#define PRINTF(format,...) \
do{ if ((temp = LIBC_FORMAT_PRINTF(printer,closure,format,locale,__VA_ARGS__)) < 0) goto err; \
    result += temp; \
}while(0)
#else
#define PRINTF(format,...) \
do{ if ((temp = LIBC_FORMAT_PRINTF(printer,closure,format,__VA_ARGS__)) < 0) goto err; \
    result += temp; \
}while(0)
#endif

#define PRINT(p,s) \
do{ if ((temp = (*printer)(p,s,closure)) < 0) goto err; \
    result += temp; \
}while(0)

#define TOOCT(c) ('0'+(c))


INTERN ssize_t
(LIBCCALL LIBC_FORMAT_QUOTE)(PFORMATPRINTER printer, void *closure,
                             format_T_char const *__restrict text,
                             size_t textlen, u32 flags
#ifdef FORMAT_OPTION_LOCALE
                             ,
                             locale_t locale
#endif
                             )
{
 PRIVATE format_T_char const quotation_mark[] = { '\"' };
 format_T_char encoded_text[4];
 size_t encoded_text_size;
 ssize_t result = 0,temp;
 unsigned_format_T_char ch; char const *c_hex;
 format_T_char const *iter,*end,*flush_start;
 end = (iter = flush_start = text)+textlen;
 c_hex = libc_decimals[!(flags&FORMAT_QUOTE_FLAG_UPPERHEX)];
 encoded_text[0] = '\\';
 if (!(flags&FORMAT_QUOTE_FLAG_PRINTRAW))
     PRINT(quotation_mark,1);
 while (iter != end) {
  ch = *(unsigned_format_T_char *)iter;
#if 0
  if (1)
#else
  if (ch < 32    || ch >= 127  || ch == '\'' ||
      ch == '\"' || ch == '\\' ||
     (flags&FORMAT_QUOTE_FLAG_NOASCII))
#endif
  {
   /* Character requires special encoding. */
#if 0
   if (!ch && !(flags&FORMAT_QUOTE_FLAG_QUOTEALL)) goto done;
#endif
   /* Flush unprinted text. */
   if (iter != flush_start)
       PRINT(flush_start,(size_t)(iter-flush_start));
   flush_start = iter+1;
   if (ch < 32) {
#if 0
    goto encode_hex;
#endif
    /* Control character. */
    if (flags&FORMAT_QUOTE_FLAG_NOCTRL) {
default_ctrl:
     if (flags&FORMAT_QUOTE_FLAG_FORCEHEX) goto encode_hex;
encode_oct:
     if (ch <= 0x07) {
      encoded_text[1] = TOOCT((ch & 0x07));
      encoded_text_size = 2;
     } else if (ch <= 0x38) {
      encoded_text[1] = TOOCT((ch & 0x38) >> 3);
      encoded_text[2] = TOOCT((ch & 0x07));
      encoded_text_size = 3;
     } else {
      encoded_text[1] = TOOCT((ch & 0xC0) >> 6);
      encoded_text[2] = TOOCT((ch & 0x38) >> 3);
      encoded_text[3] = TOOCT((ch & 0x07));
      encoded_text_size = 4;
     }
     goto print_encoded;
    }
special_control:
    switch (ch) {
    case '\a':   ch = 'a'; break;
    case '\b':   ch = 'b'; break;
    case '\f':   ch = 'f'; break;
    case '\n':   ch = 'n'; break;
    case '\r':   ch = 'r'; break;
    case '\t':   ch = 't'; break;
    case '\v':   ch = 'v'; break;
    case '\033': ch = 'e'; break;
    case '\\': case '\'': case '\"': break;
    default: goto default_ctrl;
    }
    encoded_text[1] = (format_T_char)ch;
    encoded_text_size = 2;
    goto print_encoded;
   } else if ((ch == '\\' || ch == '\'' || ch == '\"') &&
             !(flags&FORMAT_QUOTE_FLAG_NOCTRL)) {
    goto special_control;
   } else {
    /* Non-ascii character. */
/*default_nonascii:*/
    if (flags&FORMAT_QUOTE_FLAG_FORCEOCT) goto encode_oct;
encode_hex:
    encoded_text[1] = 'x';
    if (ch <= 0xf) {
     encoded_text[2] = c_hex[ch];
     encoded_text_size = 3;
    } else {
     encoded_text[2] = c_hex[(ch & 0xf0) >> 4];
     encoded_text[3] = c_hex[ch&0xf];
     encoded_text_size = 4;
    }
print_encoded:
    PRINT(encoded_text,encoded_text_size);
    goto next;
   }
  }
next:
  ++iter;
 }
/*done:*/
 if (iter != flush_start)
     PRINT(flush_start,(size_t)(iter-flush_start));
 if (!(flags&FORMAT_QUOTE_FLAG_PRINTRAW))
     PRINT(quotation_mark,1);
 return result;
err:
 return temp;
}

#undef FORMAT_OPTION_CHARTYPE
#undef FORMAT_OPTION_LOCALE

#undef TOOCT
#undef ISPRINT
#undef PRINT
#undef PRINTF
#undef format_T_char
#undef unsigned_format_T_char
#undef LIBC_FORMAT_QUOTE
#undef LIBC_FORMAT_PRINTF
#undef PFORMATPRINTER

DECL_END
