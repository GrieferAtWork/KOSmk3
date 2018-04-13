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

#include <hybrid/minmax.h>
#include <hybrid/align.h>
#include <format-printer.h>
#include <except.h>
#include <stdbool.h>
#include <alloca.h>

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

#ifndef LIBC_FORMAT_HEXDUMP
#ifdef FORMAT_OPTION_LOCALE
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_HEXDUMP   libc_format_hexdump_l
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_HEXDUMP   libc_format_w16hexdump_l
#   else
#      define LIBC_FORMAT_HEXDUMP   libc_format_w32hexdump_l
#   endif
#else /* FORMAT_OPTION_LOCALE */
#   if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#      define LIBC_FORMAT_HEXDUMP   libc_format_hexdump
#   elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#      define LIBC_FORMAT_HEXDUMP   libc_format_w16hexdump
#   else
#      define LIBC_FORMAT_HEXDUMP   libc_format_w32hexdump
#   endif
#endif /* !FORMAT_OPTION_LOCALE */
#endif /* !LIBC_FORMAT_HEXDUMP */

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

#define PRINT_SPACE   PP_CAT2(LIBC_FORMAT_HEXDUMP,_print_space)

#ifndef MAX_SPACE_SIZE
#define MAX_SPACE_SIZE  64
#endif
#ifndef MAX_ASCII_SIZE
#define MAX_ASCII_SIZE  64
#endif

PRIVATE ssize_t LIBCCALL
PRINT_SPACE(PFORMATPRINTER printer,
            void *closure, size_t count) {
 size_t used_size,bufsize;
 format_T_char *spacebuf; ssize_t result = 0,temp;
 bufsize = MIN(count,MAX_SPACE_SIZE);
 spacebuf = (format_T_char *)alloca(bufsize*sizeof(format_T_char));
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
 libc_memset(spacebuf,' ',bufsize);
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
 libc_memsetw(spacebuf,' ',bufsize);
#else
 libc_memsetl(spacebuf,' ',bufsize);
#endif
 for (;;) {
  used_size = MIN(count,bufsize);
  assert(spacebuf[0] == ' ');
  assert(spacebuf[used_size-1] == ' ');
  temp = (*printer)(spacebuf,used_size,closure);
  if unlikely(temp < 0) goto err;
  result += temp;
  if (used_size == count) break;
  count -= used_size;
 }
 return result;
err:
 return temp;
}

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

INTERN ssize_t
(LIBCCALL LIBC_FORMAT_HEXDUMP)(PFORMATPRINTER printer, void *closure,
                               void const *__restrict data, size_t size,
                               size_t linesize, u32 flags
#ifdef FORMAT_OPTION_LOCALE
                               ,
                               locale_t locale
#endif
                               )
{
 format_T_char hex_buf[3],*ascii_line;
 char const *hex_translate;
 byte_t const *line,*iter,*end; byte_t b;
 ssize_t result = 0,temp;
 size_t lineuse,overflow,ascii_size;
 unsigned int offset_size = 0;
 if unlikely(!size) return 0;
 if unlikely(!linesize) linesize = 16;
 if (!(flags&FORMAT_HEXDUMP_FLAG_NOASCII)) {
  /* Allocate a small buffer we can overwrite for ascii text. */
  ascii_size = MIN(MAX_ASCII_SIZE,linesize);
  ascii_line = (format_T_char *)alloca(ascii_size*sizeof(format_T_char));
 }
 if (flags&FORMAT_HEXDUMP_FLAG_OFFSETS) {
  /* Figure out how wide we should pad the address offset field. */
  size_t i = (size_t)1 << (offset_size = __SIZEOF_POINTER__*8-1);
  while (!(linesize&i)) --offset_size,i >>= 1;
  offset_size = CEILDIV(offset_size,4);
 }
 /* The last character of the hex buffer is always a space. */
 hex_buf[2] = ' ';
 /* Figure out the hex translation vector that should be used. */
 hex_translate = libc_decimals[!!(flags&FORMAT_HEXDUMP_FLAG_HEXLOWER)];
 for (line = (byte_t const *)data;;) {
  if (linesize <= size) {
   lineuse  = linesize;
   overflow = 0;
  } else {
   lineuse  = size;
   overflow = linesize-size;
  }
  if (flags&(FORMAT_HEXDUMP_FLAG_ADDRESS|FORMAT_HEXDUMP_FLAG_OFFSETS)) {
   PRIVATE format_T_char const address_format[] = { '%', 'p', ' ', 0 };
   PRIVATE format_T_char const offset_format[] = { '+', '%', '.', '*', 'I', 'x', ' ', 0 };
   /* Must print some sort of prefix. */
   if (flags&FORMAT_HEXDUMP_FLAG_ADDRESS)
       PRINTF(address_format,line);
   if (flags&FORMAT_HEXDUMP_FLAG_OFFSETS)
       PRINTF(offset_format,offset_size,(uintptr_t)line-(uintptr_t)data);
  }
  end = line+lineuse;
  if (!(flags&FORMAT_HEXDUMP_FLAG_NOHEX)) {
   for (iter = line; iter != end; ++iter) {
    b = *iter;
    hex_buf[0] = hex_translate[(b&0xf0) >> 4];
    hex_buf[1] = hex_translate[b&0xf];
    PRINT(hex_buf,COMPILER_LENOF(hex_buf));
   }
   if (overflow) {
    temp = PRINT_SPACE(printer,closure,overflow*
                       COMPILER_LENOF(hex_buf));
    if unlikely(temp < 0) goto err;
    result += temp;
   }
  }
  if (!(flags&FORMAT_HEXDUMP_FLAG_NOASCII)) {
   for (iter = line; iter != end;) {
    format_T_char *aiter,*aend;
    size_t textcount = MIN(ascii_size,(size_t)(end-iter));
    libc_memcpy(ascii_line,iter,textcount);
    /* Filter out non-printable characters, replacing them with '.' */
    aend = (aiter = ascii_line)+textcount;
    for (; aiter != aend; ++aiter) {
     if (!ISPRINT(*aiter))
         *aiter = '.';
    }
    /* Print our ascii text portion. */
    PRINT(ascii_line,textcount);
    iter += textcount;
   }
   if (overflow) {
    /* Fill any overflow with space. */
    temp = PRINT_SPACE(printer,closure,overflow);
    if unlikely(temp < 0) goto err;
    result += temp;
   }
  }
  if (size == lineuse) break;
  line  = end;
  size -= lineuse;
  {
   PRIVATE format_T_char const lf[] = { '\n' };
   PRINT(lf,1);
  }
 }
 return result;
err:
 return temp;
}

#undef FORMAT_OPTION_CHARTYPE
#undef FORMAT_OPTION_LOCALE

#undef ISPRINT
#undef PRINT_SPACE
#undef PRINT
#undef PRINTF
#undef format_T_char
#undef LIBC_FORMAT_HEXDUMP
#undef LIBC_FORMAT_PRINTF
#undef PFORMATPRINTER

DECL_END
