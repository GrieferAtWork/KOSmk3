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
#ifndef GUARD_HYBRID_FORMAT_PRINTER_C
#define GUARD_HYBRID_FORMAT_PRINTER_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/minmax.h>
#include <hybrid/align.h>
#include <alloca.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>
#include <format-printer.h>

#include "hybrid.h"

DECL_BEGIN


INTERN char const libc_decimals[2][17] = {
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'x' },
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'X' },
};


#ifndef __INTELLISENSE__
/* libc_format_vprintf / libc_format_vprintf_l */
#define FORMAT_OPTION_CHARTYPE  CHARACTER_TYPE_CHAR
#ifndef CONFIG_LIBC_LIMITED_API
#define FORMAT_OPTION_LOCALE    1
#endif
#include "format-printer-impl.c.inl"

/* libc_format_hexdump / libc_format_hexdump_l */
#define FORMAT_OPTION_CHARTYPE  CHARACTER_TYPE_CHAR
#ifndef CONFIG_LIBC_LIMITED_API
#define FORMAT_OPTION_LOCALE    1
#endif
#include "format-hexdump-impl.c.inl"

/* libc_format_quote / libc_format_quote_l */
#define FORMAT_OPTION_CHARTYPE  CHARACTER_TYPE_CHAR
#ifndef CONFIG_LIBC_LIMITED_API
#define FORMAT_OPTION_LOCALE    1
#endif
#include "format-quote-impl.c.inl"

#endif



#define print(p,s) \
do{ if unlikely((temp = (*printer)(p,s,closure)) < 0) goto err; \
    result += temp; \
}__WHILE0

#ifndef CONFIG_LIBC_LIMITED_API
INTERN ssize_t ATTR_CDECL
libc_format_printf_l(pformatprinter printer, void *closure,
                     char const *__restrict format,
                     locale_t locale, ...) {
 ssize_t result; va_list args;
 va_start(args,locale);
 __TRY_VALIST {
  result = libc_format_vprintf_l(printer,closure,format,locale,args);
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}
INTERN ssize_t
(LIBCCALL libc_format_vprintf)(pformatprinter printer, void *closure,
                               char const *__restrict format,
                               va_list args) {
 return libc_format_vprintf_l(printer,closure,format,NULL,args);
}
#endif
INTERN ssize_t
(ATTR_CDECL libc_format_printf)(pformatprinter printer, void *closure,
                                char const *__restrict format, ...) {
 ssize_t result;
 va_list args;
 va_start(args,format);
 __TRY_VALIST {
  result = libc_format_vprintf(printer,closure,format,args);
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}


#ifndef CONFIG_LIBC_LIMITED_API
INTERN ssize_t ATTR_CDECL
libd_format_printf_l(pformatprinter printer, void *closure,
                     char const *__restrict format,
                     locale_t locale, ...) {
 ssize_t result; va_list args;
 va_start(args,locale);
 __TRY_VALIST {
  result = libd_format_vprintf_l(printer,closure,format,locale,args);
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}
INTERN ssize_t
(LIBCCALL libd_format_vprintf)(pformatprinter printer, void *closure,
                               char const *__restrict format,
                               va_list args) {
 return libd_format_vprintf_l(printer,closure,format,NULL,args);
}

INTERN ssize_t
(ATTR_CDECL libd_format_printf)(pformatprinter printer, void *closure,
                                char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 __TRY_VALIST {
  result = libd_format_vprintf(printer,closure,format,args);
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}

INTERN ssize_t
(LIBCCALL libc_format_quote)(pformatprinter printer, void *closure,
                             char const *__restrict text,
                             size_t textlen, u32 flags) {
 return libc_format_quote_l(printer,closure,text,textlen,flags,NULL);
}
INTERN ssize_t
(LIBCCALL libc_format_hexdump)(pformatprinter printer, void *closure,
                               void const *__restrict data, size_t size,
                               size_t linesize, u32 flags) {
 return libc_format_hexdump_l(printer,closure,data,size,linesize,flags,NULL);
}
#endif

#undef libc_format_vprintf
#undef libc_format_printf
#undef libc_format_quote
#undef libc_format_hexdump
EXPORT(__KSYM(format_printf),libc_format_printf);
EXPORT(__KSYM(format_vprintf),libc_format_vprintf);
EXPORT(format_quote,libc_format_quote);
EXPORT(format_hexdump,libc_format_hexdump);
EXPORT(format_repeat,libc_format_repeat);

#ifndef CONFIG_LIBC_LIMITED_API
EXPORT(__KSYM(format_printf_l),libc_format_printf_l);
EXPORT(__KSYM(format_vprintf_l),libc_format_vprintf_l);
EXPORT(format_quote_l,libc_format_quote_l);
EXPORT(format_hexdump_l,libc_format_hexdump_l);
#undef libd_format_printf
#undef libd_format_vprintf
EXPORT(__DSYM(format_printf),libd_format_printf);
EXPORT(__DSYM(format_vprintf),libd_format_vprintf);
EXPORT(__DSYM(format_printf_l),libd_format_printf_l);
EXPORT(__DSYM(format_vprintf_l),libd_format_vprintf_l);
#endif


EXPORT(format_width,libc_format_width);
INTERN ssize_t LIBCCALL
libc_format_width(char const *__restrict UNUSED(data),
                  size_t datalen, void *UNUSED(closure)) {
 /* Simply re-return `datalen', thus allowing this printer
  * to be used to determine the width of text. */
 return (ssize_t)datalen;
}



/* Unlimited string printers for kernel-space.
 * User-space defines these as part of libc (including a wide variety
 * of secondary version for positional argument and locale support),
 * but since we don't include libc in kernel-space, to prevent a lot
 * to hassle, we just define these functions for ourself. */
#ifdef CONFIG_LIBC_LIMITED_API

PRIVATE ssize_t LIBCCALL
printf_callback(char const *__restrict data, size_t datalen,
                char **pbuffer) {
 libc_memcpy(*pbuffer,data,datalen*sizeof(char));
 *pbuffer += datalen;
 return datalen;
}

EXPORT(vsprintf,libc_vsprintf);
INTERN size_t LIBCCALL
libc_vsprintf(char *__restrict buf,
              char const *__restrict format,
              va_list args) {
 size_t result;
 result = (size_t)libc_format_vprintf((pformatprinter)&printf_callback,
                                      (void *)&buf,format,args);
 /* Random fact: Forgetting to terminate the string here
  *              breaks tab-completion in busybox's ash... */
 *buf = '\0';
 return result;
}

EXPORT(sprintf,libc_sprintf);
INTERN size_t ATTR_CDECL
libc_sprintf(char *__restrict buf,
             char const *__restrict format, ...) {
 va_list args; size_t result;
 va_start(args,format);
 __TRY_VALIST {
  result = libc_vsprintf(buf,format,args);
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}

struct snprintf_data { char *bufpos,*bufend; };
PRIVATE ssize_t LIBCCALL
snprintf_callback(char const *__restrict data, size_t datalen,
                  struct snprintf_data *__restrict buf) {
 /* Don't exceed the buffer end */
 if (buf->bufpos < buf->bufend) {
  size_t maxwrite = (size_t)(buf->bufend-buf->bufpos);
  libc_memcpy(buf->bufpos,data,
              MIN(maxwrite,datalen)*
              sizeof(char));
 }
 /* Still seek past the end, as to
  * calculate the required buffersize. */
 buf->bufpos += datalen;
 return datalen;
}

EXPORT(vsnprintf,libc_vsnprintf);
INTERN size_t LIBCCALL
libc_vsnprintf(char *__restrict buf, size_t buflen,
               char const *__restrict format, va_list args) {
 struct snprintf_data data; data.bufpos = buf;
 if (__builtin_add_overflow((uintptr_t)buf,buflen,(uintptr_t *)&data.bufend))
     data.bufend = (char *)(uintptr_t)-1;
 libc_format_vprintf((pformatprinter)&snprintf_callback,
                     &data,format,args);
 if likely(data.bufpos < data.bufend) *data.bufpos = '\0';
 return (size_t)(data.bufpos-buf);
}

EXPORT(snprintf,libc_snprintf);
INTERN size_t ATTR_CDECL
libc_snprintf(char *__restrict buf, size_t buflen,
              char const *__restrict format, ...) {
 va_list args; size_t result;
 va_start(args,format);
 __TRY_VALIST {
  result = libc_vsnprintf(buf,buflen,format,args);
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}

#endif /* CONFIG_LIBC_LIMITED_API */

DECL_END

#endif /* !GUARD_HYBRID_FORMAT_PRINTER_C */
