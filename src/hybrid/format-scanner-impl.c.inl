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
#include "format-scanner.c"
#include "../libs/libc/libc.h"
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#define FORMAT_OPTION_LOCALE     1 /* Enable locale support. */
#define FORMAT_OPTION_WCHAR16    1 /* Wide characters are 16-bit. */
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

#ifndef LIBC_FORMAT_VSCANF
#ifdef FORMAT_OPTION_LOCALE
#   ifdef FORMAT_OPTION_WCHAR16
#      if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#         define LIBC_FORMAT_SCANF    libd_format_scanf_l
#         define LIBC_FORMAT_VSCANF   libd_format_vscanf_l
#      elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#         define LIBC_FORMAT_SCANF    libd_format_w16scanf_l
#         define LIBC_FORMAT_VSCANF   libd_format_vw16scanf_l
#      else
#         define LIBC_FORMAT_SCANF    libd_format_w32scanf_l
#         define LIBC_FORMAT_VSCANF   libd_format_vw32scanf_l
#      endif
#   else /* FORMAT_OPTION_WCHAR16 */
#      if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#         define LIBC_FORMAT_SCANF    libc_format_scanf_l
#         define LIBC_FORMAT_VSCANF   libc_format_vscanf_l
#      elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#         define LIBC_FORMAT_SCANF    libc_format_w16scanf_l
#         define LIBC_FORMAT_VSCANF   libc_format_vw16scanf_l
#      else
#         define LIBC_FORMAT_SCANF    libc_format_w32scanf_l
#         define LIBC_FORMAT_VSCANF   libc_format_vw32scanf_l
#      endif
#   endif /* !FORMAT_OPTION_WCHAR16 */
#else /* FORMAT_OPTION_LOCALE */
#   ifdef FORMAT_OPTION_WCHAR16
#      if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#         define LIBC_FORMAT_SCANF    libd_format_scanf
#         define LIBC_FORMAT_VSCANF   libd_format_vscanf
#      elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#         define LIBC_FORMAT_SCANF    libd_format_w16scanf
#         define LIBC_FORMAT_VSCANF   libd_format_vw16scanf
#      else
#         define LIBC_FORMAT_SCANF    libd_format_w32scanf
#         define LIBC_FORMAT_VSCANF   libd_format_vw32scanf
#      endif
#   else /* FORMAT_OPTION_WCHAR16 */
#      if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#         define LIBC_FORMAT_SCANF    libc_format_scanf
#         define LIBC_FORMAT_VSCANF   libc_format_vscanf
#      elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#         define LIBC_FORMAT_SCANF    libc_format_w16scanf
#         define LIBC_FORMAT_VSCANF   libc_format_vw16scanf
#      else
#         define LIBC_FORMAT_SCANF    libc_format_w32scanf
#         define LIBC_FORMAT_VSCANF   libc_format_vw32scanf
#      endif
#   endif /* !FORMAT_OPTION_WCHAR16 */
#endif /* !FORMAT_OPTION_LOCALE */
#endif /* !LIBC_FORMAT_VSCANF */

#ifndef PFORMATGETC
#if FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR
#   define PFORMATGETC    pformatgetc
#   define PFORMATUNGETC  pformatungetc
#elif FORMAT_OPTION_CHARTYPE == CHARACTER_TYPE_CHAR16
#   define PFORMATGETC    pw16formatgetc
#   define PFORMATUNGETC  pw16formatungetc
#else
#   define PFORMATGETC    pw32formatgetc
#   define PFORMATUNGETC  pw32formatungetc
#endif
#endif


DECL_BEGIN

INTERN ssize_t
(LIBCCALL LIBC_FORMAT_VSCANF)(PFORMATGETC scanner,
                              PFORMATUNGETC returnch, void *closure,
                              format_T_char const *__restrict format,
#ifdef FORMAT_OPTION_LOCALE
                              locale_t locale,
#endif
                              va_list args)
{
 (void)scanner;
 (void)returnch;
 (void)closure;
 (void)format;
#ifdef FORMAT_OPTION_LOCALE
 (void)locale;
#endif
 (void)args;

 /* TODO */

 return 0;
}

#undef FORMAT_OPTION_CHARTYPE
#undef FORMAT_OPTION_LOCALE
#undef FORMAT_OPTION_WCHAR16

#undef format_T_char
#undef LIBC_FORMAT_SCANF
#undef LIBC_FORMAT_VSCANF
#undef PFORMATGETC
#undef PFORMATUNGETC

DECL_END
