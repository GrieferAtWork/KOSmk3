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
#ifndef GUARD_HYBRID_FORMAT_SCANNER_C
#define GUARD_HYBRID_FORMAT_SCANNER_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/minmax.h>
#include <hybrid/align.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>
#include <format-printer.h>

#include "hybrid.h"

#ifndef __INTELLISENSE__
/* libc_format_vscanf / libc_format_vscanf_l */
#define FORMAT_OPTION_CHARTYPE  CHARACTER_TYPE_CHAR
#ifndef CONFIG_LIBC_LIMITED_API
#define FORMAT_OPTION_LOCALE    1
#endif
#include "format-scanner-impl.c.inl"
#endif

DECL_BEGIN


INTERN ssize_t
(ATTR_CDECL libc_format_scanf)(pformatgetc scanner,
                               pformatungetc returnch,
                               void *closure,
                               char const *__restrict format, ...) {
 va_list args; ssize_t result;
 va_start(args,format);
 __TRY_VALIST {
  result = libc_format_vscanf(scanner,returnch,closure,format,args);
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}

#ifndef CONFIG_LIBC_LIMITED_API
INTERN ssize_t
(LIBCCALL libc_format_vscanf)(pformatgetc scanner,
                              pformatungetc returnch,
                              void *closure,
                              char const *__restrict format,
                              va_list args) {
 return libc_format_vscanf_l(scanner,returnch,closure,format,NULL,args);
}
INTERN ssize_t
ATTR_CDECL libc_format_scanf_l(pformatgetc scanner,
                               pformatungetc returnch,
                               void *closure,
                               char const *__restrict format,
                               locale_t locale, ...) {
 va_list args; ssize_t result;
 va_start(args,locale);
 __TRY_VALIST {
  result = libc_format_vscanf_l(scanner,returnch,closure,format,locale,args);
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}
#endif


#ifndef CONFIG_LIBC_LIMITED_API
INTERN ssize_t
(LIBCCALL libd_format_vscanf)(pformatgetc scanner, pformatungetc returnch,
                              void *closure, char const *__restrict format,
                              va_list args) {
 return libd_format_vscanf_l(scanner,returnch,closure,format,NULL,args);
}
INTERN ssize_t
(ATTR_CDECL libd_format_scanf)(pformatgetc scanner, pformatungetc returnch,
                               void *closure, char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 __TRY_VALIST {
  result = libd_format_vscanf(scanner,returnch,closure,format,args);
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}
INTERN ssize_t ATTR_CDECL
libd_format_scanf_l(pformatgetc scanner, pformatungetc returnch,
                    void *closure, char const *__restrict format,
                    locale_t locale, ...) {
 ssize_t result; va_list args;
 va_start(args,locale);
 __TRY_VALIST {
  result = libd_format_vscanf_l(scanner,returnch,closure,format,locale,args);
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}
#endif

#undef libc_format_vscanf
#undef libc_format_scanf
EXPORT(__KSYM(format_scanf),libc_format_scanf);
EXPORT(__KSYM(format_vscanf),libc_format_vscanf);

#ifndef CONFIG_LIBC_LIMITED_API
EXPORT(__KSYM(format_scanf_l),libc_format_scanf_l);
EXPORT(__KSYM(format_vscanf_l),libc_format_vscanf_l);
#undef libd_format_scanf
#undef libd_format_vscanf
EXPORT(__DSYM(format_scanf),libd_format_scanf);
EXPORT(__DSYM(format_vscanf),libd_format_vscanf);
EXPORT(__DSYM(format_scanf_l),libd_format_scanf_l);
EXPORT(__DSYM(format_vscanf_l),libd_format_vscanf_l);
#endif


DECL_END

#endif /* !GUARD_HYBRID_FORMAT_SCANNER_C */
