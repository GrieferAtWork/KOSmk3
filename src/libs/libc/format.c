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
#ifndef GUARD_LIBS_LIBC_FORMAT_C
#define GUARD_LIBS_LIBC_FORMAT_C 1
#define _UTF_SOURCE 1

#include "libc.h"
#include "format.h"

#include <hybrid/compiler.h>
#include <format-printer.h>
#include <uchar.h>
#include <assert.h>
#include <except.h>

#ifndef __INTELLISENSE__

/* Define additional format helpers (quote / hexdump). */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#define FORMAT_OPTION_LOCALE     1
#include "../../hybrid/format-hexdump-impl.c.inl" /* libc_format_w16hexdump_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR32
#define FORMAT_OPTION_LOCALE     1
#include "../../hybrid/format-hexdump-impl.c.inl" /* libc_format_w32hexdump_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#define FORMAT_OPTION_LOCALE     1
#include "../../hybrid/format-quote-impl.c.inl" /* libc_format_w16quote_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR32
#define FORMAT_OPTION_LOCALE     1
#include "../../hybrid/format-quote-impl.c.inl" /* libc_format_w32quote_l */


/* Define additional format printers. */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR
#define FORMAT_OPTION_LOCALE     1
#define FORMAT_OPTION_POSITIONAL 1
#include "../../hybrid/format-printer-impl.c.inl" /* libc_format_printf_p_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#define FORMAT_OPTION_LOCALE     1
#define FORMAT_OPTION_POSITIONAL 1
#include "../../hybrid/format-printer-impl.c.inl" /* libc_format_w16printf_p_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR32
#define FORMAT_OPTION_LOCALE     1
#define FORMAT_OPTION_POSITIONAL 1
#include "../../hybrid/format-printer-impl.c.inl" /* libc_format_w32printf_p_l */
//#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR /* Already defined by `hybrid/format-printer.c' */
//#define FORMAT_OPTION_LOCALE     1
//#include "../../hybrid/format-printer-impl.c.inl" /* libc_format_printf_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#define FORMAT_OPTION_LOCALE     1
#include "../../hybrid/format-printer-impl.c.inl" /* libc_format_w16printf_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR32
#define FORMAT_OPTION_LOCALE     1
#include "../../hybrid/format-printer-impl.c.inl" /* libc_format_w32printf_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR
#define FORMAT_OPTION_LOCALE     1
#define FORMAT_OPTION_POSITIONAL 1
#define FORMAT_OPTION_WCHAR16    1
#include "../../hybrid/format-printer-impl.c.inl" /* libd_format_printf_p_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#define FORMAT_OPTION_LOCALE     1
#define FORMAT_OPTION_POSITIONAL 1
#define FORMAT_OPTION_WCHAR16    1
#include "../../hybrid/format-printer-impl.c.inl" /* libd_format_w16printf_p_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR32
#define FORMAT_OPTION_LOCALE     1
#define FORMAT_OPTION_POSITIONAL 1
#define FORMAT_OPTION_WCHAR16    1
#include "../../hybrid/format-printer-impl.c.inl" /* libd_format_w32printf_p_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR
#define FORMAT_OPTION_LOCALE     1
#define FORMAT_OPTION_WCHAR16    1
#include "../../hybrid/format-printer-impl.c.inl" /* libd_format_printf_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#define FORMAT_OPTION_LOCALE     1
#define FORMAT_OPTION_WCHAR16    1
#include "../../hybrid/format-printer-impl.c.inl" /* libd_format_w16printf_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR32
#define FORMAT_OPTION_LOCALE     1
#define FORMAT_OPTION_WCHAR16    1
#include "../../hybrid/format-printer-impl.c.inl" /* libd_format_w32printf_l */

/* Define additional format scanners. */
//#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR /* Already defined by `hybrid/format-scanner.c' */
//#define FORMAT_OPTION_LOCALE     1
//#include "../../hybrid/format-scanner-impl.c.inl" /* libc_format_vscanf_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR
#define FORMAT_OPTION_LOCALE     1
#define FORMAT_OPTION_WCHAR16    1
#include "../../hybrid/format-scanner-impl.c.inl" /* libd_format_vscanf_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#define FORMAT_OPTION_LOCALE     1
#include "../../hybrid/format-scanner-impl.c.inl" /* libc_format_vw16scanf_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR16
#define FORMAT_OPTION_LOCALE     1
#define FORMAT_OPTION_WCHAR16    1
#include "../../hybrid/format-scanner-impl.c.inl" /* libd_format_vw16scanf_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR32
#define FORMAT_OPTION_LOCALE     1
#include "../../hybrid/format-scanner-impl.c.inl" /* libc_format_vw32scanf_l */
#define FORMAT_OPTION_CHARTYPE   CHARACTER_TYPE_CHAR32
#define FORMAT_OPTION_LOCALE     1
#define FORMAT_OPTION_WCHAR16    1
#include "../../hybrid/format-scanner-impl.c.inl" /* libd_format_vw32scanf_l */


#define CHARACTER_TYPE   CHARACTER_TYPE_CHAR
#include "format-impl.c.inl"
#define CHARACTER_TYPE   CHARACTER_TYPE_CHAR16
#include "format-impl.c.inl"
#define CHARACTER_TYPE   CHARACTER_TYPE_CHAR32
#include "format-impl.c.inl"


/* Define format wrapper functions */
#include "format-wrapper.c.inl"

#endif

DECL_BEGIN

/* Delete all those wrapper macros... */
#ifndef __OPTIMIZE_SIZE__
#undef libc_format_w16quote
#undef libc_format_w32quote
#undef libc_format_w16hexdump
#undef libc_format_w32hexdump
#undef libc_format_w16printf
#undef libd_format_w16printf
#undef libc_format_w32printf
#undef libd_format_w32printf
#undef libc_format_vw16printf
#undef libd_format_vw16printf
#undef libc_format_vw32printf
#undef libd_format_vw32printf
#undef libc_format_printf_p
#undef libd_format_printf_p
#undef libc_format_w16printf_p
#undef libd_format_w16printf_p
#undef libc_format_w32printf_p
#undef libd_format_w32printf_p
#undef libc_format_vprintf_p
#undef libd_format_vprintf_p
#undef libc_format_vw16printf_p
#undef libd_format_vw16printf_p
#undef libc_format_vw32printf_p
#undef libd_format_vw32printf_p
#undef libc_format_bprintf
#undef libd_format_bprintf
#undef libc_format_w16bprintf
#undef libd_format_w16bprintf
#undef libc_format_w32bprintf
#undef libd_format_w32bprintf
#undef libc_format_vbprintf
#undef libd_format_vbprintf
#undef libc_format_vw16bprintf
#undef libd_format_vw16bprintf
#undef libc_format_vw32bprintf
#undef libd_format_vw32bprintf
#undef libc_format_bprintf_p
#undef libd_format_bprintf_p
#undef libc_format_w16bprintf_p
#undef libd_format_w16bprintf_p
#undef libc_format_w32bprintf_p
#undef libd_format_w32bprintf_p
#undef libc_format_vbprintf_p
#undef libd_format_vbprintf_p
#undef libc_format_vw16bprintf_p
#undef libd_format_vw16bprintf_p
#undef libc_format_vw32bprintf_p
#undef libd_format_vw32bprintf_p
#undef libc_sprintf
#undef libd_sprintf
#undef libc_sw16printf
#undef libd_sw16printf
#undef libc_sw32printf
#undef libd_sw32printf
#undef libc_sprintf_p
#undef libd_sprintf_p
#undef libc_sw16printf_p
#undef libd_sw16printf_p
#undef libc_sw32printf_p
#undef libd_sw32printf_p
#undef libc_vsprintf
#undef libd_vsprintf
#undef libc_vsw16printf
#undef libd_vsw16printf
#undef libc_vsw32printf
#undef libd_vsw32printf
#undef libc_vsprintf_p
#undef libd_vsprintf_p
#undef libc_vsw16printf_p
#undef libd_vsw16printf_p
#undef libc_vsw32printf_p
#undef libd_vsw32printf_p
#undef libc_snprintf
#undef libd_snprintf
#undef libc_snw16printf
#undef libd_snw16printf
#undef libc_snw32printf
#undef libd_snw32printf
#undef libc_vsnprintf
#undef libd_vsnprintf
#undef libc_vsnw16printf
#undef libd_vsnw16printf
#undef libc_vsnw32printf
#undef libd_vsnw32printf
#undef libc_snprintf_p
#undef libd_snprintf_p
#undef libc_snw16printf_p
#undef libd_snw16printf_p
#undef libc_snw32printf_p
#undef libd_snw32printf_p
#undef libc_vsnprintf_p
#undef libd_vsnprintf_p
#undef libc_vsnw16printf_p
#undef libd_vsnw16printf_p
#undef libc_vsnw32printf_p
#undef libd_vsnw32printf_p
#undef libc_snprintf_c
#undef libd_snprintf_c
#undef libc_snw16printf_c
#undef libd_snw16printf_c
#undef libc_snw32printf_c
#undef libd_snw32printf_c
#undef libc_vsnprintf_c
#undef libd_vsnprintf_c
#undef libc_vsnw16printf_c
#undef libd_vsnw16printf_c
#undef libc_vsnw32printf_c
#undef libd_vsnw32printf_c
#undef libc_scprintf
#undef libd_scprintf
#undef libc_scw16printf
#undef libd_scw16printf
#undef libc_scw32printf
#undef libd_scw32printf
#undef libc_scprintf_p
#undef libd_scprintf_p
#undef libc_scw16printf_p
#undef libd_scw16printf_p
#undef libc_scw32printf_p
#undef libd_scw32printf_p
#undef libc_vscprintf
#undef libd_vscprintf
#undef libc_vscw16printf
#undef libd_vscw16printf
#undef libc_vscw32printf
#undef libd_vscw32printf
#undef libc_vscprintf_p
#undef libd_vscprintf_p
#undef libc_vscw16printf_p
#undef libd_vscw16printf_p
#undef libc_vscw32printf_p
#undef libd_vscw32printf_p
#undef libc_dprintf
#undef libd_dprintf
#undef libc_w16dprintf
#undef libd_w16dprintf
#undef libc_w32dprintf
#undef libd_w32dprintf
#undef libc_dprintf_p
#undef libd_dprintf_p
#undef libc_w16dprintf_p
#undef libd_w16dprintf_p
#undef libc_w32dprintf_p
#undef libd_w32dprintf_p
#undef libc_vdprintf
#undef libd_vdprintf
#undef libc_vw16dprintf
#undef libd_vw16dprintf
#undef libc_vw32dprintf
#undef libd_vw32dprintf
#undef libc_vdprintf_p
#undef libd_vdprintf_p
#undef libc_vw16dprintf_p
#undef libd_vw16dprintf_p
#undef libc_vw32dprintf_p
#undef libd_vw32dprintf_p
#undef libc_asprintf
#undef libd_asprintf
#undef libc_asprintf_p
#undef libd_asprintf_p
#undef libc_vasprintf
#undef libd_vasprintf
#undef libc_vasprintf_p
#undef libd_vasprintf_p
#undef libc_strdupf
#undef libd_strdupf
#undef libc_strdupf_p
#undef libd_strdupf_p
#undef libc_vstrdupf
#undef libd_vstrdupf
#undef libc_vstrdupf_p
#undef libd_vstrdupf_p
#undef libc_Xstrdupf
#undef libd_Xstrdupf
#undef libc_Xstrdupf_p
#undef libd_Xstrdupf_p
#undef libc_Xvstrdupf
#undef libd_Xvstrdupf
#undef libc_Xvstrdupf_p
#undef libd_Xvstrdupf_p
#undef libc_sscanf
#undef libd_sscanf
#undef libc_sw16scanf
#undef libd_sw16scanf
#undef libc_sw32scanf
#undef libd_sw32scanf
#undef libc_vsscanf
#undef libd_vsscanf
#undef libc_vsw16scanf
#undef libd_vsw16scanf
#undef libc_vsw32scanf
#undef libd_vsw32scanf
#undef libc_snscanf
#undef libd_snscanf
#undef libc_snw16scanf
#undef libd_snw16scanf
#undef libc_snw32scanf
#undef libd_snw32scanf
#undef libc_vsnscanf
#undef libd_vsnscanf
#undef libc_vsnw16scanf
#undef libd_vsnw16scanf
#undef libc_vsnw32scanf
#undef libd_vsnw32scanf
#undef libc_printf
#undef libd_printf
#undef libc_w16printf
#undef libd_w16printf
#undef libc_w32printf
#undef libd_w32printf
#undef libc_vprintf
#undef libd_vprintf
#undef libc_vw16printf
#undef libd_vw16printf
#undef libc_vw32printf
#undef libd_vw32printf
#undef libc_printf_p
#undef libd_printf_p
#undef libc_w16printf_p
#undef libd_w16printf_p
#undef libc_w32printf_p
#undef libd_w32printf_p
#undef libc_vprintf_p
#undef libd_vprintf_p
#undef libc_vw16printf_p
#undef libd_vw16printf_p
#undef libc_vw32printf_p
#undef libd_vw32printf_p
#undef libc_fprintf
#undef libd_fprintf
#undef libc_fw16printf
#undef libd_fw16printf
#undef libc_fw32printf
#undef libd_fw32printf
#undef libc_vfprintf
#undef libd_vfprintf
#undef libc_vfw16printf
#undef libd_vfw16printf
#undef libc_vfw32printf
#undef libd_vfw32printf
#undef libc_fprintf_p
#undef libd_fprintf_p
#undef libc_fw16printf_p
#undef libd_fw16printf_p
#undef libc_fw32printf_p
#undef libd_fw32printf_p
#undef libc_vfprintf_p
#undef libd_vfprintf_p
#undef libc_vfw16printf_p
#undef libd_vfw16printf_p
#undef libc_vfw32printf_p
#undef libd_vfw32printf_p
#undef libc_scanf
#undef libd_scanf
#undef libc_w16scanf
#undef libd_w16scanf
#undef libc_w32scanf
#undef libd_w32scanf
#undef libc_vscanf
#undef libd_vscanf
#undef libc_vw16scanf
#undef libd_vw16scanf
#undef libc_vw32scanf
#undef libd_vw32scanf
#undef libc_fscanf
#undef libd_fscanf
#undef libc_fw16scanf
#undef libd_fw16scanf
#undef libc_fw32scanf
#undef libd_fw32scanf
#undef libc_vfscanf
#undef libd_vfscanf
#undef libc_vfw16scanf
#undef libd_vfw16scanf
#undef libc_vfw32scanf
#undef libd_vfw32scanf
#endif



/* The tedious task of exporting all those format functions...
 * NOTE: Be careful to only export stuff defined in here,
 *       not stuff already defined by `hybrid/format_printer.c'
 *       and `hybrid/format_scanner.c'. */


EXPORT(format_strftime,libc_format_strftime);
EXPORT(format_strftime_l,libc_format_strftime_l);
EXPORT(__SYMw16(format_wcsftime),libc_format_w16ftime);
EXPORT(__SYMw32(format_wcsftime),libc_format_w32ftime);
EXPORT(strftime,libc_strftime);
EXPORT(__KSYM(strftime_l),libc_strftime_l);
EXPORT(__DSYM(_strftime_l),libc_strftime_l);
EXPORT(__SYMw16(wcsftime),libc_w16ftime);
EXPORT(__SYMw32(wcsftime),libc_w32ftime);
EXPORT(__SYMw16(_wcsftime_l),libc_w16ftime_l);
EXPORT(__SYMw32(wcsftime_l),libc_w32ftime_l);



EXPORT(stringprinter_init,libc_stringprinter_init);
EXPORT(stringprinter_pack,libc_stringprinter_pack);
EXPORT(stringprinter_fini,libc_stringprinter_fini);
EXPORT(stringprinter_print,libc_stringprinter_print);
EXPORT(__SYMw16(wstringprinter_init),libc_string16printer_init);
EXPORT(__SYMw16(wstringprinter_pack),libc_string16printer_pack);
EXPORT(__SYMw16(wstringprinter_fini),libc_string16printer_fini);
EXPORT(__SYMw16(wstringprinter_print),libc_string16printer_print);
EXPORT(__SYMw32(wstringprinter_init),libc_string32printer_init);
EXPORT(__SYMw32(wstringprinter_pack),libc_string32printer_pack);
EXPORT(__SYMw32(wstringprinter_fini),libc_string32printer_fini);
EXPORT(__SYMw32(wstringprinter_print),libc_string32printer_print);

EXPORT(Xstringprinter_init,libc_Xstringprinter_init);
EXPORT(Xstringprinter_print,libc_Xstringprinter_print);
EXPORT(__SYMw16(Xwstringprinter_init),libc_Xstring16printer_init);
EXPORT(__SYMw16(Xwstringprinter_print),libc_Xstring16printer_print);
EXPORT(__SYMw32(Xwstringprinter_init),libc_Xstring32printer_init);
EXPORT(__SYMw32(Xwstringprinter_print),libc_Xstring32printer_print);

/* String buffers. */
EXPORT(buffer_init,libc_buffer_init);
EXPORT(buffer_fini,libc_buffer_fini);
EXPORT(buffer_flush,libc_buffer_flush);
EXPORT(buffer_print,libc_buffer_print);
EXPORT(__SYMw16(wbuffer_init),libc_w16buffer_init);
EXPORT(__SYMw16(wbuffer_fini),libc_w16buffer_fini);
EXPORT(__SYMw16(wbuffer_flush),libc_w16buffer_flush);
EXPORT(__SYMw16(wbuffer_print),libc_w16buffer_print);
EXPORT(__SYMw32(wbuffer_init),libc_w32buffer_init);
EXPORT(__SYMw32(wbuffer_fini),libc_w32buffer_fini);
EXPORT(__SYMw32(wbuffer_flush),libc_w32buffer_flush);
EXPORT(__SYMw32(wbuffer_print),libc_w32buffer_print);

/* Conversion printers. */
EXPORT(__SYMw16(wprinter_fini),libc_w16printer_fini);
EXPORT(__SYMw32(wprinter_fini),libc_w32printer_fini);
EXPORT(__SYMw16(wprinter_init),libc_w16printer_init);
EXPORT(__SYMw32(wprinter_init),libc_w32printer_init);
EXPORT(__SYMw16(wprinter_print),libc_w16printer_print);
EXPORT(__SYMw32(wprinter_print),libc_w32printer_print);
EXPORT(__SYMw16(Xwprinter_print),libc_Xw16printer_print);
EXPORT(__SYMw32(Xwprinter_print),libc_Xw32printer_print);

/* Wide-character format helpers. */
EXPORT(__SYMw16(format_wquote),libc_format_w16quote);
EXPORT(__SYMw32(format_wquote),libc_format_w32quote);
EXPORT(__SYMw16(format_whexdump),libc_format_w16hexdump);
EXPORT(__SYMw32(format_whexdump),libc_format_w32hexdump);
EXPORT(__SYMw16(format_wquote_l),libc_format_w16quote_l);
EXPORT(__SYMw32(format_wquote_l),libc_format_w32quote_l);
EXPORT(__SYMw16(format_whexdump_l),libc_format_w16hexdump_l);
EXPORT(__SYMw32(format_whexdump_l),libc_format_w32hexdump_l);
EXPORT(__SYMw16(format_wrepeat),libc_format_w16repeat);
EXPORT(__SYMw32(format_wrepeat),libc_format_w32repeat);

/* Wide-character format scanners. */
EXPORT(__KSYMw16(format_wscanf),libc_format_w16scanf);
EXPORT(__DSYMw16(format_wscanf),libd_format_w16scanf);
EXPORT(__KSYMw32(format_wscanf),libc_format_w32scanf);
EXPORT(__DSYMw32(format_wscanf),libd_format_w32scanf);
EXPORT(__KSYMw16(format_vwscanf),libc_format_vw16scanf);
EXPORT(__DSYMw16(format_vwscanf),libd_format_vw16scanf);
EXPORT(__KSYMw32(format_vwscanf),libc_format_vw32scanf);
EXPORT(__DSYMw32(format_vwscanf),libd_format_vw32scanf);
EXPORT(__KSYMw16(format_wscanf_l),libc_format_w16scanf_l);
EXPORT(__DSYMw16(format_wscanf_l),libd_format_w16scanf_l);
EXPORT(__KSYMw32(format_wscanf_l),libc_format_w32scanf_l);
EXPORT(__DSYMw32(format_wscanf_l),libd_format_w32scanf_l);
EXPORT(__KSYMw16(format_vwscanf_l),libc_format_vw16scanf_l);
EXPORT(__DSYMw16(format_vwscanf_l),libd_format_vw16scanf_l);
EXPORT(__KSYMw32(format_vwscanf_l),libc_format_vw32scanf_l);
EXPORT(__DSYMw32(format_vwscanf_l),libd_format_vw32scanf_l);

/* Wide-character format printers. */
EXPORT(__KSYMw16(format_wprintf),libc_format_w16printf);
EXPORT(__DSYMw16(format_wprintf),libd_format_w16printf);
EXPORT(__KSYMw32(format_wprintf),libc_format_w32printf);
EXPORT(__DSYMw32(format_wprintf),libd_format_w32printf);
EXPORT(__KSYMw16(format_vwprintf),libc_format_vw16printf);
EXPORT(__DSYMw16(format_vwprintf),libd_format_vw16printf);
EXPORT(__KSYMw32(format_vwprintf),libc_format_vw32printf);
EXPORT(__DSYMw32(format_vwprintf),libd_format_vw32printf);
EXPORT(__KSYMw16(format_wprintf_l),libc_format_w16printf_l);
EXPORT(__DSYMw16(format_wprintf_l),libd_format_w16printf_l);
EXPORT(__KSYMw32(format_wprintf_l),libc_format_w32printf_l);
EXPORT(__DSYMw32(format_wprintf_l),libd_format_w32printf_l);
EXPORT(__KSYMw16(format_vwprintf_l),libc_format_vw16printf_l);
EXPORT(__DSYMw16(format_vwprintf_l),libd_format_vw16printf_l);
EXPORT(__KSYMw32(format_vwprintf_l),libc_format_vw32printf_l);
EXPORT(__DSYMw32(format_vwprintf_l),libd_format_vw32printf_l);

/* Format printers with support for positional arguments. */
EXPORT(__KSYM(format_printf_p),libc_format_printf_p);
EXPORT(__DSYM(format_printf_p),libd_format_printf_p);
EXPORT(__KSYMw16(format_wprintf_p),libc_format_w16printf_p);
EXPORT(__DSYMw16(format_wprintf_p),libd_format_w16printf_p);
EXPORT(__KSYMw32(format_wprintf_p),libc_format_w32printf_p);
EXPORT(__DSYMw32(format_wprintf_p),libd_format_w32printf_p);
EXPORT(__KSYM(format_vprintf_p),libc_format_vprintf_p);
EXPORT(__DSYM(format_vprintf_p),libd_format_vprintf_p);
EXPORT(__KSYMw16(format_vwprintf_p),libc_format_vw16printf_p);
EXPORT(__DSYMw16(format_vwprintf_p),libd_format_vw16printf_p);
EXPORT(__KSYMw32(format_vwprintf_p),libc_format_vw32printf_p);
EXPORT(__DSYMw32(format_vwprintf_p),libd_format_vw32printf_p);
EXPORT(__KSYM(format_printf_p_l),libc_format_printf_p_l);
EXPORT(__DSYM(format_printf_p_l),libd_format_printf_p_l);
EXPORT(__KSYMw16(format_wprintf_p_l),libc_format_w16printf_p_l);
EXPORT(__DSYMw16(format_wprintf_p_l),libd_format_w16printf_p_l);
EXPORT(__KSYMw32(format_wprintf_p_l),libc_format_w32printf_p_l);
EXPORT(__DSYMw32(format_wprintf_p_l),libd_format_w32printf_p_l);
EXPORT(__KSYM(format_vprintf_p_l),libc_format_vprintf_p_l);
EXPORT(__DSYM(format_vprintf_p_l),libd_format_vprintf_p_l);
EXPORT(__KSYMw16(format_vwprintf_p_l),libc_format_vw16printf_p_l);
EXPORT(__DSYMw16(format_vwprintf_p_l),libd_format_vw16printf_p_l);
EXPORT(__KSYMw32(format_vwprintf_p_l),libc_format_vw32printf_p_l);
EXPORT(__DSYMw32(format_vwprintf_p_l),libd_format_vw32printf_p_l);

/* Format printers with an automatic), intermediate buffer. */
EXPORT(__KSYM(format_bprintf),libc_format_bprintf);
EXPORT(__DSYM(format_bprintf),libd_format_bprintf);
EXPORT(__KSYMw16(format_wbprintf),libc_format_w16bprintf);
EXPORT(__DSYMw16(format_wbprintf),libd_format_w16bprintf);
EXPORT(__KSYMw32(format_wbprintf),libc_format_w32bprintf);
EXPORT(__DSYMw32(format_wbprintf),libd_format_w32bprintf);
EXPORT(__KSYM(format_vbprintf),libc_format_vbprintf);
EXPORT(__DSYM(format_vbprintf),libd_format_vbprintf);
EXPORT(__KSYMw16(format_vwbprintf),libc_format_vw16bprintf);
EXPORT(__DSYMw16(format_vwbprintf),libd_format_vw16bprintf);
EXPORT(__KSYMw32(format_vwbprintf),libc_format_vw32bprintf);
EXPORT(__DSYMw32(format_vwbprintf),libd_format_vw32bprintf);
EXPORT(__KSYM(format_bprintf_p),libc_format_bprintf_p);
EXPORT(__DSYM(format_bprintf_p),libd_format_bprintf_p);
EXPORT(__KSYMw16(format_wbprintf_p),libc_format_w16bprintf_p);
EXPORT(__DSYMw16(format_wbprintf_p),libd_format_w16bprintf_p);
EXPORT(__KSYMw32(format_wbprintf_p),libc_format_w32bprintf_p);
EXPORT(__DSYMw32(format_wbprintf_p),libd_format_w32bprintf_p);
EXPORT(__KSYM(format_vbprintf_p),libc_format_vbprintf_p);
EXPORT(__DSYM(format_vbprintf_p),libd_format_vbprintf_p);
EXPORT(__KSYMw16(format_vwbprintf_p),libc_format_vw16bprintf_p);
EXPORT(__DSYMw16(format_vwbprintf_p),libd_format_vw16bprintf_p);
EXPORT(__KSYMw32(format_vwbprintf_p),libc_format_vw32bprintf_p);
EXPORT(__DSYMw32(format_vwbprintf_p),libd_format_vw32bprintf_p);
EXPORT(__KSYM(format_bprintf_l),libc_format_bprintf_l);
EXPORT(__DSYM(format_bprintf_l),libd_format_bprintf_l);
EXPORT(__KSYMw16(format_wbprintf_l),libc_format_w16bprintf_l);
EXPORT(__DSYMw16(format_wbprintf_l),libd_format_w16bprintf_l);
EXPORT(__KSYMw32(format_wbprintf_l),libc_format_w32bprintf_l);
EXPORT(__DSYMw32(format_wbprintf_l),libd_format_w32bprintf_l);
EXPORT(__KSYM(format_vbprintf_l),libc_format_vbprintf_l);
EXPORT(__DSYM(format_vbprintf_l),libd_format_vbprintf_l);
EXPORT(__KSYMw16(format_vwbprintf_l),libc_format_vw16bprintf_l);
EXPORT(__DSYMw16(format_vwbprintf_l),libd_format_vw16bprintf_l);
EXPORT(__KSYMw32(format_vwbprintf_l),libc_format_vw32bprintf_l);
EXPORT(__DSYMw32(format_vwbprintf_l),libd_format_vw32bprintf_l);
EXPORT(__KSYM(format_bprintf_p_l),libc_format_bprintf_p_l);
EXPORT(__DSYM(format_bprintf_p_l),libd_format_bprintf_p_l);
EXPORT(__KSYMw16(format_wbprintf_p_l),libc_format_w16bprintf_p_l);
EXPORT(__DSYMw16(format_wbprintf_p_l),libd_format_w16bprintf_p_l);
EXPORT(__KSYMw32(format_wbprintf_p_l),libc_format_w32bprintf_p_l);
EXPORT(__DSYMw32(format_wbprintf_p_l),libd_format_w32bprintf_p_l);
EXPORT(__KSYM(format_vbprintf_p_l),libc_format_vbprintf_p_l);
EXPORT(__DSYM(format_vbprintf_p_l),libd_format_vbprintf_p_l);
EXPORT(__KSYMw16(format_vwbprintf_p_l),libc_format_vw16bprintf_p_l);
EXPORT(__DSYMw16(format_vwbprintf_p_l),libd_format_vw16bprintf_p_l);
EXPORT(__KSYMw32(format_vwbprintf_p_l),libc_format_vw32bprintf_p_l);
EXPORT(__DSYMw32(format_vwbprintf_p_l),libd_format_vw32bprintf_p_l);


/* sprintf() */
EXPORT(__KSYM(sprintf),libc_sprintf);
EXPORT(__DSYM(sprintf),libd_sprintf);

/* vsprintf() */
EXPORT(__KSYM(vsprintf),libc_vsprintf);
EXPORT(__DSYM(vsprintf),libd_vsprintf);

/* sprintf_l() */
EXPORT(__KSYM(sprintf_l),libc_sprintf_l); /* !GLC */
EXPORT(__DSYM(_sprintf_l),libd_sprintf_l);

/* vsprintf_l() */
EXPORT(__KSYM(vsprintf_l),libc_vsprintf_l); /* !GLC */
EXPORT(__DSYM(_vsprintf_l),libd_vsprintf_l);

/* sprintf_p() */
EXPORT(__KSYM(sprintf_p),libc_sprintf_p); /* !GLC */
EXPORT(__DSYM(_sprintf_p),libd_sprintf_p);

/* vsprintf_p() */
EXPORT(__KSYM(vsprintf_p),libc_vsprintf_p); /* !GLC */
EXPORT(__DSYM(_vsprintf_p),libd_vsprintf_p);

/* sprintf_p_l() */
EXPORT(__KSYM(sprintf_p_l),libc_sprintf_p_l); /* !GLC */
EXPORT(__DSYM(_sprintf_p_l),libd_sprintf_p_l);

/* vsprintf_p_l() */
EXPORT(__KSYM(vsprintf_p_l),libc_vsprintf_p_l); /* !GLC */
EXPORT(__DSYM(_vsprintf_p_l),libd_vsprintf_p_l);

/* _swprintf() */
EXPORT(__KSYMw16(_swprintf),libc_sw16printf); /* !GLC */
EXPORT(__KSYMw32(_swprintf),libc_sw32printf);
EXPORT(__DSYMw16(_swprintf),libd_sw16printf);
EXPORT(__DSYMw32(_swprintf),libd_sw32printf);

/* _vswprintf() */
EXPORT(__KSYMw16(_vswprintf),libc_vsw16printf); /* !GLC */
EXPORT(__KSYMw32(_vswprintf),libc_vsw32printf);
EXPORT(__DSYMw16(_vswprintf),libd_vsw16printf);
EXPORT(__DSYMw32(_vswprintf),libd_vsw32printf);

/* __swprintf_l() */
EXPORT(__KSYMw16(__swprintf_l),libc_sw16printf_l); /* !GLC */
EXPORT(__KSYMw32(__swprintf_l),libc_sw32printf_l);
EXPORT(__DSYMw16(__swprintf_l),libd_sw16printf_l);
EXPORT(__DSYMw32(__swprintf_l),libd_sw32printf_l);

/* _vswprintf_l() */
EXPORT(__KSYMw16(__vswprintf_l),libd_vsw16printf_l);
EXPORT(__KSYMw32(__vswprintf_l),libd_vsw32printf_l);
EXPORT(__DSYMw16(__vswprintf_l),libd_vsw16printf_l);
EXPORT(__DSYMw32(__vswprintf_l),libd_vsw32printf_l);

/* ??? */
//EXPORT(__KSYMw16(???),libc_sw16printf_p);
//EXPORT(__KSYMw32(???),libc_sw32printf_p);
//EXPORT(__DSYMw16(???),libd_sw16printf_p);
//EXPORT(__DSYMw32(???),libd_sw32printf_p);

/* ??? */
//EXPORT(__KSYMw16(???),libc_vsw16printf_p);
//EXPORT(__KSYMw32(???),libc_vsw32printf_p);
//EXPORT(__DSYMw16(???),libd_vsw16printf_p);
//EXPORT(__DSYMw32(???),libd_vsw32printf_p);

/* ??? */
//EXPORT(__KSYMw16(???),libc_sw16printf_p_l);
//EXPORT(__KSYMw32(???),libc_sw32printf_p_l);
//EXPORT(__DSYMw16(???),libd_sw16printf_p_l);
//EXPORT(__DSYMw32(???),libd_sw32printf_p_l);

/* ??? */
//EXPORT(__KSYMw16(???),libc_vsw16printf_p_l);
//EXPORT(__KSYMw32(???),libc_vsw32printf_p_l);
//EXPORT(__DSYMw16(???),libd_vsw16printf_p_l);
//EXPORT(__DSYMw32(???),libd_vsw32printf_p_l);

/* snprintf() */
EXPORT(__KSYM(snprintf),libc_snprintf);
EXPORT(__DSYM(_snprintf),libd_snprintf);

/* vsnprintf() */
EXPORT(__KSYM(vsnprintf),libc_vsnprintf);
EXPORT(__DSYM(_vsnprintf),libd_vsnprintf);
EXPORT(__KSYM(__vsnprintf),libc_vsnprintf); /* GLibC alias */

/* snprintf_l() */
EXPORT(__KSYM(snprintf_l),libc_snprintf_l); /* !GLC */
EXPORT(__DSYM(_snprintf_l),libd_snprintf_l);

/* vsnprintf_l() */
EXPORT(__KSYM(vsnprintf_l),libc_vsnprintf_l); /* !GLC */
EXPORT(__DSYM(_vsnprintf_l),libd_vsnprintf_l);

/* snprintf_p() */
EXPORT(__KSYM(snprintf_p),libc_snprintf_p); /* !GLC */
EXPORT(__DSYM(_snprintf_p),libd_snprintf_p); /* !DOS */

/* vsnprintf_p() */
EXPORT(__KSYM(vsnprintf_p),libc_vsnprintf_p); /* !GLC */
EXPORT(__DSYM(_vsnprintf_p),libd_vsnprintf_p); /* !DOS */

/* snprintf_p_l() */
EXPORT(__KSYM(snprintf_p_l),libc_snprintf_p_l); /* !GLC */
EXPORT(__DSYM(_snprintf_p_l),libd_snprintf_p_l); /* !DOS */

/* vsnprintf_p_l() */
EXPORT(__KSYM(vsnprintf_p_l),libc_vsnprintf_p_l); /* !GLC */
EXPORT(__DSYM(_vsnprintf_p_l),libd_vsnprintf_p_l); /* !DOS */

/* swprintf() */
EXPORT(__KSYMw16(swprintf),libc_snw16printf);
EXPORT(__KSYMw32(swprintf),libc_snw32printf);
EXPORT(__DSYMw16(_snwprintf),libd_snw16printf);
EXPORT(__DSYMw32(snwprintf),libd_snw32printf);

/* vswprintf() */
EXPORT(__KSYMw16(vswprintf),libc_vsnw16printf);
EXPORT(__KSYMw32(vswprintf),libc_vsnw32printf);
EXPORT(__DSYMw16(_vsnwprintf),libd_vsnw16printf);
EXPORT(__DSYMw32(vsnwprintf),libd_vsnw32printf);

/* swprintf_l() */
EXPORT(__KSYMw16(swprintf_l),libc_snw16printf_l); /* !GLC */
EXPORT(__KSYMw32(swprintf_l),libc_snw32printf_l);
EXPORT(__DSYMw16(_snwprintf_l),libd_snw16printf_l);
EXPORT(__DSYMw32(snwprintf_l),libd_snw32printf_l);

/* vswprintf_l() */
EXPORT(__KSYMw16(vswprintf_l),libc_vsnw16printf_l); /* !GLC */
EXPORT(__KSYMw32(vswprintf_l),libc_vsnw32printf_l);
EXPORT(__DSYMw16(_vsnwprintf_l),libd_vsnw16printf_l);
EXPORT(__DSYMw32(vsnwprintf_l),libd_vsnw32printf_l);
EXPORT(__DSYMw16(_vswprintf_l),libd_vsnw16printf_l);
EXPORT(__DSYMw32(vswprintf_l),libd_vsnw32printf_l);

/* swprintf_p() */
EXPORT(__KSYMw16(swprintf_p),libc_snw16printf_p); /* !GLC */
EXPORT(__KSYMw32(swprintf_p),libc_snw32printf_p);
EXPORT(__DSYMw16(_swprintf_p),libd_snw16printf_p);
EXPORT(__DSYMw32(swprintf_p),libd_snw32printf_p);

/* vswprintf_p() */
EXPORT(__KSYMw16(vswprintf_p),libc_vsnw16printf_p); /* !GLC */
EXPORT(__KSYMw32(vswprintf_p),libc_vsnw32printf_p);
EXPORT(__DSYMw16(_vswprintf_p),libd_vsnw16printf_p);
EXPORT(__DSYMw32(vswprintf_p),libd_vsnw32printf_p);

/* swprintf_p_l() */
EXPORT(__KSYMw16(swprintf_p_l),libc_snw16printf_p_l); /* !GLC */
EXPORT(__KSYMw32(swprintf_p_l),libc_snw32printf_p_l);
EXPORT(__DSYMw16(_swprintf_p_l),libd_snw16printf_p_l);
EXPORT(__DSYMw32(swprintf_p_l),libd_snw32printf_p_l);

/* vswprintf_p_l() */
EXPORT(__KSYMw16(vswprintf_p_l),libc_vsnw16printf_p_l); /* !GLC */
EXPORT(__KSYMw32(vswprintf_p_l),libc_vsnw32printf_p_l);
EXPORT(__DSYMw16(_vswprintf_p_l),libd_vsnw16printf_p_l);
EXPORT(__DSYMw32(vswprintf_p_l),libd_vsnw32printf_p_l);

/* dprintf() */
EXPORT(__KSYM(dprintf),libc_dprintf);
EXPORT(__DSYM(dprintf),libd_dprintf); /* !DOS */
EXPORT(__KSYMw16(wdprintf),libc_w16dprintf); /* !GLC */
EXPORT(__DSYMw16(wdprintf),libd_w16dprintf); /* !DOS */
EXPORT(__KSYMw32(wdprintf),libc_w32dprintf); /* !GLC */
EXPORT(__DSYMw32(wdprintf),libd_w32dprintf); /* !DOS */
EXPORT(__KSYM(dprintf_p),libc_dprintf_p); /* !GLC */
EXPORT(__DSYM(dprintf_p),libd_dprintf_p); /* !DOS */
EXPORT(__KSYMw16(wdprintf_p),libc_w16dprintf_p); /* !GLC */
EXPORT(__DSYMw16(wdprintf_p),libd_w16dprintf_p); /* !DOS */
EXPORT(__KSYMw32(wdprintf_p),libc_w32dprintf_p); /* !GLC */
EXPORT(__DSYMw32(wdprintf_p),libd_w32dprintf_p); /* !DOS */
EXPORT(__KSYM(vdprintf),libc_vdprintf);
EXPORT(__DSYM(vdprintf),libd_vdprintf); /* !DOS */
EXPORT(__KSYMw16(vwdprintf),libc_vw16dprintf); /* !GLC */
EXPORT(__DSYMw16(vwdprintf),libd_vw16dprintf); /* !DOS */
EXPORT(__KSYMw32(vwdprintf),libc_vw32dprintf); /* !GLC */
EXPORT(__DSYMw32(vwdprintf),libd_vw32dprintf); /* !DOS */
EXPORT(__KSYM(vdprintf_p),libc_vdprintf_p); /* !GLC */
EXPORT(__DSYM(vdprintf_p),libd_vdprintf_p); /* !DOS */
EXPORT(__KSYMw16(vwdprintf_p),libc_vw16dprintf_p); /* !GLC */
EXPORT(__DSYMw16(vwdprintf_p),libd_vw16dprintf_p); /* !DOS */
EXPORT(__KSYMw32(vwdprintf_p),libc_vw32dprintf_p); /* !GLC */
EXPORT(__DSYMw32(vwdprintf_p),libd_vw32dprintf_p); /* !DOS */
EXPORT(__KSYM(dprintf_l),libc_dprintf_l); /* !GLC */
EXPORT(__DSYM(dprintf_l),libd_dprintf_l); /* !DOS */
EXPORT(__KSYMw16(wdprintf_l),libc_w16dprintf_l); /* !GLC */
EXPORT(__DSYMw16(wdprintf_l),libd_w16dprintf_l); /* !DOS */
EXPORT(__KSYMw32(wdprintf_l),libc_w32dprintf_l); /* !GLC */
EXPORT(__DSYMw32(wdprintf_l),libd_w32dprintf_l); /* !DOS */
EXPORT(__KSYM(dprintf_p_l),libc_dprintf_p_l); /* !GLC */
EXPORT(__DSYM(dprintf_p_l),libd_dprintf_p_l); /* !DOS */
EXPORT(__KSYMw16(wdprintf_p_l),libc_w16dprintf_p_l); /* !GLC */
EXPORT(__DSYMw16(wdprintf_p_l),libd_w16dprintf_p_l); /* !DOS */
EXPORT(__KSYMw32(wdprintf_p_l),libc_w32dprintf_p_l); /* !GLC */
EXPORT(__DSYMw32(wdprintf_p_l),libd_w32dprintf_p_l); /* !DOS */
EXPORT(__KSYM(vdprintf_l),libc_vdprintf_l); /* !GLC */
EXPORT(__DSYM(vdprintf_l),libd_vdprintf_l); /* !DOS */
EXPORT(__KSYMw16(vwdprintf_l),libc_vw16dprintf_l); /* !GLC */
EXPORT(__DSYMw16(vwdprintf_l),libd_vw16dprintf_l); /* !DOS */
EXPORT(__KSYMw32(vwdprintf_l),libc_vw32dprintf_l); /* !GLC */
EXPORT(__DSYMw32(vwdprintf_l),libd_vw32dprintf_l); /* !DOS */
EXPORT(__KSYM(vdprintf_p_l),libc_vdprintf_p_l); /* !GLC */
EXPORT(__DSYM(vdprintf_p_l),libd_vdprintf_p_l); /* !DOS */
EXPORT(__KSYMw16(vwdprintf_p_l),libc_vw16dprintf_p_l); /* !GLC */
EXPORT(__DSYMw16(vwdprintf_p_l),libd_vw16dprintf_p_l); /* !DOS */
EXPORT(__KSYMw32(vwdprintf_p_l),libc_vw32dprintf_p_l); /* !GLC */
EXPORT(__DSYMw32(vwdprintf_p_l),libd_vw32dprintf_p_l); /* !DOS */

/* asprintf() */
EXPORT(__KSYM(asprintf),libc_asprintf);
EXPORT(__KSYM(__asprintf),libc_asprintf);
EXPORT(__DSYM(asprintf),libd_asprintf); /* !DOS */
EXPORT(__KSYM(asprintf_p),libc_asprintf_p); /* !GLC */
EXPORT(__DSYM(asprintf_p),libd_asprintf_p); /* !DOS */
EXPORT(__KSYM(vasprintf),libc_vasprintf);
EXPORT(__DSYM(vasprintf),libd_vasprintf); /* !DOS */
EXPORT(__KSYM(vasprintf_p),libc_vasprintf_p); /* !GLC */
EXPORT(__DSYM(vasprintf_p),libd_vasprintf_p); /* !DOS */
EXPORT(__KSYM(asprintf_l),libc_asprintf_l); /* !GLC */
EXPORT(__DSYM(asprintf_l),libd_asprintf_l); /* !DOS */
EXPORT(__KSYM(asprintf_p_l),libc_asprintf_p_l); /* !GLC */
EXPORT(__DSYM(asprintf_p_l),libd_asprintf_p_l); /* !DOS */
EXPORT(__KSYM(vasprintf_l),libc_vasprintf_l); /* !GLC */
EXPORT(__DSYM(vasprintf_l),libd_vasprintf_l); /* !DOS */
EXPORT(__KSYM(vasprintf_p_l),libc_vasprintf_p_l); /* !GLC */
EXPORT(__DSYM(vasprintf_p_l),libd_vasprintf_p_l); /* !DOS */

/* strdupf() */
EXPORT(__KSYM(strdupf),libc_strdupf); /* !GLC */
EXPORT(__DSYM(strdupf),libd_strdupf); /* !DOS */
EXPORT(__KSYM(strdupf_p),libc_strdupf_p); /* !GLC */
EXPORT(__DSYM(strdupf_p),libd_strdupf_p); /* !DOS */
EXPORT(__KSYM(vstrdupf),libc_vstrdupf); /* !GLC */
EXPORT(__DSYM(vstrdupf),libd_vstrdupf); /* !DOS */
EXPORT(__KSYM(vstrdupf_p),libc_vstrdupf_p); /* !GLC */
EXPORT(__DSYM(vstrdupf_p),libd_vstrdupf_p); /* !DOS */
EXPORT(__KSYM(strdupf_l),libc_strdupf_l); /* !GLC */
EXPORT(__DSYM(strdupf_l),libd_strdupf_l); /* !DOS */
EXPORT(__KSYM(strdupf_p_l),libc_strdupf_p_l); /* !GLC */
EXPORT(__DSYM(strdupf_p_l),libd_strdupf_p_l); /* !DOS */
EXPORT(__KSYM(vstrdupf_l),libc_vstrdupf_l); /* !GLC */
EXPORT(__DSYM(vstrdupf_l),libd_vstrdupf_l); /* !DOS */
EXPORT(__KSYM(vstrdupf_p_l),libc_vstrdupf_p_l); /* !GLC */
EXPORT(__DSYM(vstrdupf_p_l),libd_vstrdupf_p_l); /* !DOS */

/* Xstrdupf() */
EXPORT(__KSYM(Xstrdupf),libc_Xstrdupf); /* !GLC */
EXPORT(__DSYM(Xstrdupf),libd_Xstrdupf); /* !DOS */
EXPORT(__KSYM(Xstrdupf_p),libc_Xstrdupf_p); /* !GLC */
EXPORT(__DSYM(Xstrdupf_p),libd_Xstrdupf_p); /* !DOS */
EXPORT(__KSYM(Xvstrdupf),libc_Xvstrdupf); /* !GLC */
EXPORT(__DSYM(Xvstrdupf),libd_Xvstrdupf); /* !DOS */
EXPORT(__KSYM(Xvstrdupf_p),libc_Xvstrdupf_p); /* !GLC */
EXPORT(__DSYM(Xvstrdupf_p),libd_Xvstrdupf_p); /* !DOS */
EXPORT(__KSYM(Xstrdupf_l),libc_Xstrdupf_l); /* !GLC */
EXPORT(__DSYM(Xstrdupf_l),libd_Xstrdupf_l); /* !DOS */
EXPORT(__KSYM(Xstrdupf_p_l),libc_Xstrdupf_p_l); /* !GLC */
EXPORT(__DSYM(Xstrdupf_p_l),libd_Xstrdupf_p_l); /* !DOS */
EXPORT(__KSYM(Xvstrdupf_l),libc_Xvstrdupf_l); /* !GLC */
EXPORT(__DSYM(Xvstrdupf_l),libd_Xvstrdupf_l); /* !DOS */
EXPORT(__KSYM(Xvstrdupf_p_l),libc_Xvstrdupf_p_l); /* !GLC */
EXPORT(__DSYM(Xvstrdupf_p_l),libd_Xvstrdupf_p_l); /* !DOS */

/* sscanf() */
EXPORT(__KSYM(sscanf),libc_sscanf);
EXPORT(__DSYM(sscanf),libd_sscanf);

/* vsscanf() */
EXPORT(__KSYM(vsscanf),libc_vsscanf);
EXPORT(__KSYM(__vsscanf),libc_vsscanf);
EXPORT(__DSYM(vsscanf),libd_vsscanf);

/* sscanf_l() */
EXPORT(__KSYM(sscanf_l),libc_sscanf_l); /* !GLC */
EXPORT(__DSYM(_sscanf_l),libd_sscanf_l);

/* vsscanf_l() */
EXPORT(__KSYM(vsscanf_l),libc_sscanf_l); /* !GLC */
EXPORT(__DSYM(_vsscanf_l),libd_sscanf_l); /* !DOS */

/* swscanf() */
EXPORT(__KSYMw16(swscanf),libc_sw16scanf);
EXPORT(__KSYMw32(swscanf),libc_sw32scanf);
EXPORT(__DSYMw16(swscanf),libd_sw16scanf);
EXPORT(__DSYMw32(swscanf),libd_sw32scanf);

/* vswscanf() */
EXPORT(__KSYMw16(vswscanf),libc_vsw16scanf);
EXPORT(__KSYMw32(vswscanf),libc_vsw32scanf);
EXPORT(__DSYMw16(vswscanf),libd_vsw16scanf);
EXPORT(__DSYMw32(vswscanf),libd_vsw32scanf);

/* swscanf_l() */
EXPORT(__KSYMw16(swscanf_l),libc_sw16scanf); /* !GLC */
EXPORT(__KSYMw32(swscanf_l),libc_sw32scanf); /* !GLC */
EXPORT(__DSYMw16(_swscanf_l),libd_sw16scanf);
EXPORT(__DSYMw32(swscanf_l),libd_sw32scanf);

/* vswscanf_l() */
EXPORT(__KSYMw16(vswscanf_l),libc_vsw16scanf); /* !GLC */
EXPORT(__KSYMw32(vswscanf_l),libc_vsw32scanf); /* !GLC */
EXPORT(__DSYMw16(_vswscanf_l),libd_vsw16scanf); /* !DOS */
EXPORT(__DSYMw32(vswscanf_l),libd_vsw32scanf); /* !DOS */

/* snscanf() */
EXPORT(__KSYM(snscanf),libc_snscanf); /* !GLC */
EXPORT(__DSYM(_snscanf),libd_snscanf);

/* vsnscanf() */
EXPORT(__KSYM(vsnscanf),libc_vsnscanf); /* !GLC */
EXPORT(__DSYM(_vsnscanf),libd_vsnscanf); /* !DOS */

/* snscanf_l() */
EXPORT(__KSYM(snscanf_l),libc_snscanf_l); /* !GLC */
EXPORT(__DSYM(_snscanf_l),libd_snscanf_l);

/* vsnscanf_l() */
EXPORT(__KSYM(vsnscanf_l),libc_snscanf_l); /* !GLC */
EXPORT(__DSYM(_vsnscanf_l),libd_snscanf_l); /* !DOS */

/* snwscanf() */
EXPORT(__KSYMw16(snwscanf),libc_snw16scanf); /* !GLC */
EXPORT(__KSYMw32(snwscanf),libc_snw32scanf); /* !GLC */
EXPORT(__DSYMw16(_snwscanf),libd_snw16scanf);
EXPORT(__DSYMw32(snwscanf),libd_snw32scanf);

/* vsnwscanf() */
EXPORT(__KSYMw16(vsnwscanf),libc_vsnw16scanf); /* !GLC */
EXPORT(__KSYMw32(vsnwscanf),libc_vsnw32scanf); /* !GLC */
EXPORT(__DSYMw16(vsnwscanf),libd_vsnw16scanf); /* !DOS */
EXPORT(__DSYMw32(vsnwscanf),libd_vsnw32scanf); /* !DOS */

/* snwscanf_l() */
EXPORT(__KSYMw16(snwscanf_l),libc_snw16scanf); /* !GLC */
EXPORT(__KSYMw32(snwscanf_l),libc_snw32scanf); /* !GLC */
EXPORT(__DSYMw16(_snwscanf_l),libd_snw16scanf);
EXPORT(__DSYMw32(snwscanf_l),libd_snw32scanf);

/* vsnwscanf_l() */
EXPORT(__KSYMw16(vsnwscanf_l),libc_vsnw16scanf); /* !GLC */
EXPORT(__KSYMw32(vsnwscanf_l),libc_vsnw32scanf); /* !GLC */
EXPORT(__DSYMw16(vsnwscanf_l),libd_vsnw16scanf); /* !DOS */
EXPORT(__DSYMw32(vsnwscanf_l),libd_vsnw32scanf); /* !DOS */

/* printf() */
EXPORT(__KSYM(printf),libc_printf);
EXPORT(__DSYM(printf),libd_printf);

/* vprintf() */
EXPORT(__KSYM(vprintf),libc_vprintf);
EXPORT(__DSYM(vprintf),libd_vprintf);

/* printf_p() */
EXPORT(__KSYM(printf_p),libc_printf_p); /* !GLC */
EXPORT(__DSYM(_printf_p),libd_printf_p);

/* vprintf_p() */
EXPORT(__KSYM(vprintf_p),libc_vprintf_p); /* !GLC */
EXPORT(__DSYM(_vprintf_p),libd_vprintf_p);

/* printf_l() */
EXPORT(__KSYM(printf_l),libc_printf_l); /* !GLC */
EXPORT(__DSYM(_printf_l),libd_printf_l);

/* vprintf_l() */
EXPORT(__KSYM(vprintf_l),libc_vprintf_l); /* !GLC */
EXPORT(__DSYM(_vprintf_l),libd_vprintf_l);

/* printf_p_l() */
EXPORT(__KSYM(printf_p_l),libc_printf_p_l); /* !GLC */
EXPORT(__DSYM(_printf_p_l),libd_printf_p_l);

/* vprintf_p_l() */
EXPORT(__KSYM(vprintf_p_l),libc_vprintf_p_l); /* !GLC */
EXPORT(__DSYM(_vprintf_p_l),libd_vprintf_p_l);

/* wprintf() */
EXPORT(__KSYMw16(wprintf),libc_w16printf);
EXPORT(__KSYMw32(wprintf),libc_w32printf);
EXPORT(__DSYMw16(wprintf),libd_w16printf);
EXPORT(__DSYMw32(wprintf),libd_w32printf);

/* vwprintf() */
EXPORT(__KSYMw16(vwprintf),libc_vw16printf);
EXPORT(__KSYMw32(vwprintf),libc_vw32printf);
EXPORT(__DSYMw16(vwprintf),libd_vw16printf);
EXPORT(__DSYMw32(vwprintf),libd_vw32printf);

/* wprintf_p() */
EXPORT(__KSYMw16(wprintf_p),libc_w16printf_p); /* !GLC */
EXPORT(__KSYMw32(wprintf_p),libc_w32printf_p); /* !GLC */
EXPORT(__DSYMw16(_wprintf_p),libd_w16printf_p);
EXPORT(__DSYMw32(wprintf_p),libd_w32printf_p);

/* vwprintf_p() */
EXPORT(__KSYMw16(vwprintf_p),libc_vw16printf_p); /* !GLC */
EXPORT(__KSYMw32(vwprintf_p),libc_vw32printf_p); /* !GLC */
EXPORT(__DSYMw16(_vwprintf_p),libd_vw16printf_p);
EXPORT(__DSYMw32(vwprintf_p),libd_vw32printf_p);

/* wprintf_l() */
EXPORT(__KSYMw16(wprintf_l),libc_w16printf_l); /* !GLC */
EXPORT(__KSYMw32(wprintf_l),libc_w32printf_l); /* !GLC */
EXPORT(__DSYMw16(_wprintf_l),libd_w16printf_l);
EXPORT(__DSYMw32(wprintf_l),libd_w32printf_l);

/* vwprintf_l() */
EXPORT(__KSYMw16(vwprintf_l),libc_vw16printf_l); /* !GLC */
EXPORT(__KSYMw32(vwprintf_l),libc_vw32printf_l); /* !GLC */
EXPORT(__DSYMw16(_vwprintf_l),libd_vw16printf_l);
EXPORT(__DSYMw32(vwprintf_l),libd_vw32printf_l);

/* wprintf_p_l() */
EXPORT(__KSYMw16(wprintf_p_l),libc_w16printf_p_l); /* !GLC */
EXPORT(__KSYMw32(wprintf_p_l),libc_w32printf_p_l); /* !GLC */
EXPORT(__DSYMw16(_wprintf_p_l),libd_w16printf_p_l);
EXPORT(__DSYMw32(wprintf_p_l),libd_w32printf_p_l);

/* vwprintf_p_l() */
EXPORT(__KSYMw16(vwprintf_p_l),libc_vw16printf_p_l); /* !GLC */
EXPORT(__KSYMw32(vwprintf_p_l),libc_vw32printf_p_l); /* !GLC */
EXPORT(__DSYMw16(_vwprintf_p_l),libd_vw16printf_p_l);
EXPORT(__DSYMw32(vwprintf_p_l),libd_vw32printf_p_l);

/* fprintf() */
EXPORT(__KSYM(fprintf),libc_fprintf);
EXPORT(__DSYM(fprintf),libd_fprintf);

/* vfprintf() */
EXPORT(__KSYM(vfprintf),libc_vfprintf);
EXPORT(__DSYM(vfprintf),libd_vfprintf);

/* fprintf_p() */
EXPORT(__KSYM(fprintf_p),libc_fprintf_p); /* !GLC */
EXPORT(__DSYM(_fprintf_p),libd_fprintf_p);

/* vfprintf_p() */
EXPORT(__KSYM(vfprintf_p),libc_vfprintf_p); /* !GLC */
EXPORT(__DSYM(_vfprintf_p),libd_vfprintf_p);

/* fprintf_l() */
EXPORT(__KSYM(fprintf_l),libc_fprintf_l); /* !GLC */
EXPORT(__DSYM(_fprintf_l),libd_fprintf_l);

/* vfprintf_l() */
EXPORT(__KSYM(vfprintf_l),libc_vfprintf_l); /* !GLC */
EXPORT(__DSYM(_vfprintf_l),libd_vfprintf_l);

/* fprintf_p_l() */
EXPORT(__KSYM(fprintf_p_l),libc_fprintf_p_l); /* !GLC */
EXPORT(__DSYM(_fprintf_p_l),libd_fprintf_p_l);

/* vfprintf_p_l() */
EXPORT(__KSYM(vfprintf_p_l),libc_vfprintf_p_l); /* !GLC */
EXPORT(__DSYM(_vfprintf_p_l),libd_vfprintf_p_l);

/* fwprintf() */
EXPORT(__KSYMw16(fwprintf),libc_fw16printf);
EXPORT(__KSYMw32(fwprintf),libc_fw32printf);
EXPORT(__DSYMw16(fwprintf),libd_fw16printf);
EXPORT(__DSYMw32(fwprintf),libd_fw32printf);

/* vfwprintf() */
EXPORT(__KSYMw16(vfwprintf),libc_vfw16printf);
EXPORT(__KSYMw32(vfwprintf),libc_vfw32printf);
EXPORT(__DSYMw16(vfwprintf),libd_vfw16printf);
EXPORT(__DSYMw32(vfwprintf),libd_vfw32printf);

/* fwprintf_p() */
EXPORT(__KSYMw16(fwprintf_p),libc_fw16printf_p); /* !GLC */
EXPORT(__KSYMw32(fwprintf_p),libc_fw32printf_p); /* !GLC */
EXPORT(__DSYMw16(_fwprintf_p),libd_fw16printf_p);
EXPORT(__DSYMw32(fwprintf_p),libd_fw32printf_p);

/* vfwprintf_p() */
EXPORT(__KSYMw16(vfwprintf_p),libc_vfw16printf_p); /* !GLC */
EXPORT(__KSYMw32(vfwprintf_p),libc_vfw32printf_p); /* !GLC */
EXPORT(__DSYMw16(_vfwprintf_p),libd_vfw16printf_p);
EXPORT(__DSYMw32(vfwprintf_p),libd_vfw32printf_p);

/* fwprintf_l() */
EXPORT(__KSYMw16(fwprintf_l),libc_fw16printf_l); /* !GLC */
EXPORT(__KSYMw32(fwprintf_l),libc_fw32printf_l); /* !GLC */
EXPORT(__DSYMw16(_fwprintf_l),libd_fw16printf_l);
EXPORT(__DSYMw32(fwprintf_l),libd_fw32printf_l);

/* vfwprintf_l() */
EXPORT(__KSYMw16(vfwprintf_l),libc_vfw16printf_l); /* !GLC */
EXPORT(__KSYMw32(vfwprintf_l),libc_vfw32printf_l); /* !GLC */
EXPORT(__DSYMw16(_vfwprintf_l),libd_vfw16printf_l);
EXPORT(__DSYMw32(vfwprintf_l),libd_vfw32printf_l);

/* fwprintf_p_l() */
EXPORT(__KSYMw16(fwprintf_p_l),libc_fw16printf_p_l); /* !GLC */
EXPORT(__KSYMw32(fwprintf_p_l),libc_fw32printf_p_l); /* !GLC */
EXPORT(__DSYMw16(_fwprintf_p_l),libd_fw16printf_p_l);
EXPORT(__DSYMw32(fwprintf_p_l),libd_fw32printf_p_l);

/* vfwprintf_p_l() */
EXPORT(__KSYMw16(vfwprintf_p_l),libc_vfw16printf_p_l); /* !GLC */
EXPORT(__KSYMw32(vfwprintf_p_l),libc_vfw32printf_p_l); /* !GLC */
EXPORT(__DSYMw16(_vfwprintf_p_l),libd_vfw16printf_p_l);
EXPORT(__DSYMw32(vfwprintf_p_l),libd_vfw32printf_p_l);

/* scanf() */
EXPORT(__KSYM(scanf),libc_scanf);
EXPORT(__DSYM(scanf),libd_scanf);

/* vscanf() */
EXPORT(__KSYM(vscanf),libc_vscanf);
EXPORT(__DSYM(vscanf),libd_vscanf);

/* scanf_l() */
EXPORT(__KSYM(scanf_l),libc_scanf_l); /* !GLC */
EXPORT(__DSYM(_scanf_l),libd_scanf_l);

/* vscanf_l() */
EXPORT(__KSYM(vscanf_l),libc_vscanf_l); /* !GLC */
EXPORT(__DSYM(_vscanf_l),libd_vscanf_l); /* !DOS */

/* wscanf() */
EXPORT(__KSYMw16(wscanf),libc_w16scanf);
EXPORT(__KSYMw32(wscanf),libc_w32scanf);
EXPORT(__DSYMw16(wscanf),libd_w16scanf);
EXPORT(__DSYMw32(wscanf),libd_w32scanf);

/* vwscanf() */
EXPORT(__KSYMw16(vwscanf),libc_vw16scanf);
EXPORT(__KSYMw32(vwscanf),libc_vw32scanf);
EXPORT(__DSYMw16(vwscanf),libd_vw16scanf);
EXPORT(__DSYMw32(vwscanf),libd_vw32scanf);

/* wscanf_l() */
EXPORT(__KSYMw16(wscanf_l),libc_w16scanf_l); /* !GLC */
EXPORT(__KSYMw32(wscanf_l),libc_w32scanf_l); /* !GLC */
EXPORT(__DSYMw16(_wscanf_l),libd_w16scanf_l);
EXPORT(__DSYMw32(wscanf_l),libd_w32scanf_l);

/* vwscanf_l() */
EXPORT(__KSYMw16(vwscanf_l),libc_vw16scanf_l); /* !GLC */
EXPORT(__KSYMw32(vwscanf_l),libc_vw32scanf_l); /* !GLC */
EXPORT(__DSYMw16(_vwscanf_l),libd_vw16scanf_l); /* !DOS */
EXPORT(__DSYMw32(vwscanf_l),libd_vw32scanf_l); /* !DOS */


/* fscanf() */
EXPORT(__KSYM(fscanf),libc_fscanf);
EXPORT(__DSYM(fscanf),libd_fscanf);

/* vfscanf() */
EXPORT(__KSYM(vfscanf),libc_vfscanf);
EXPORT(__KSYM(__vfscanf),libc_vfscanf);
EXPORT(__DSYM(vfscanf),libd_vfscanf);

/* fscanf_l() */
EXPORT(__KSYM(fscanf_l),libc_fscanf_l); /* !GLC */
EXPORT(__DSYM(_fscanf_l),libd_fscanf_l);

/* vfscanf_l() */
EXPORT(__KSYM(vfscanf_l),libc_vfscanf_l); /* !GLC */
EXPORT(__DSYM(_vfscanf_l),libd_vfscanf_l); /* !DOS */

/* fwscanf() */
EXPORT(__KSYMw16(fwscanf),libc_fw16scanf);
EXPORT(__KSYMw32(fwscanf),libc_fw32scanf);
EXPORT(__DSYMw16(fwscanf),libd_fw16scanf);
EXPORT(__DSYMw32(fwscanf),libd_fw32scanf);

/* vfwscanf() */
EXPORT(__KSYMw16(vfwscanf),libc_vfw16scanf);
EXPORT(__KSYMw32(vfwscanf),libc_vfw32scanf);
EXPORT(__DSYMw16(vfwscanf),libd_vfw16scanf);
EXPORT(__DSYMw32(vfwscanf),libd_vfw32scanf);

/* fwscanf_l() */
EXPORT(__KSYMw16(fwscanf_l),libc_fw16scanf_l); /* !GLC */
EXPORT(__KSYMw32(fwscanf_l),libc_fw32scanf_l); /* !GLC */
EXPORT(__DSYMw16(_fwscanf_l),libd_fw16scanf_l);
EXPORT(__DSYMw32(fwscanf_l),libd_fw32scanf_l);

/* vfwscanf_l() */
EXPORT(__KSYMw16(vfwscanf_l),libc_vfw16scanf_l); /* !GLC */
EXPORT(__KSYMw32(vfwscanf_l),libc_vfw32scanf_l); /* !GLC */
EXPORT(__DSYMw16(_vfwscanf_l),libd_vfw16scanf_l); /* !DOS */
EXPORT(__DSYMw32(vfwscanf_l),libd_vfw32scanf_l); /* !DOS */



/* Weird functions required for DOS compatibility */
EXPORT(__KSYM(snprintf_c),libc_vsnprintf_c);
EXPORT(__DSYM(_snprintf_c),libd_vsnprintf_c);
EXPORT(__KSYM(snprintf_c_l),libc_snprintf_c_l);
EXPORT(__DSYM(_snprintf_c_l),libd_snprintf_c_l);
EXPORT(__KSYM(vsnprintf_c),libc_vsnprintf_c);
EXPORT(__DSYM(_vsnprintf_c),libd_vsnprintf_c);
EXPORT(__KSYM(vsnprintf_c_l),libc_vsnprintf_c_l);
EXPORT(__DSYM(_vsnprintf_c_l),libd_vsnprintf_c_l);
EXPORT(__KSYMw16(swprintf_c),libc_snw16printf_c);
EXPORT(__KSYMw32(swprintf_c),libc_snw32printf_c);
EXPORT(__DSYMw16(_swprintf_c),libd_snw16printf_c);
EXPORT(__DSYMw32(swprintf_c),libd_snw32printf_c);
EXPORT(__KSYMw16(swprintf_c_l),libc_snw16printf_c_l);
EXPORT(__KSYMw32(swprintf_c_l),libc_snw32printf_c_l);
EXPORT(__DSYMw16(_swprintf_c_l),libd_snw16printf_c_l);
EXPORT(__DSYMw32(swprintf_c_l),libd_snw32printf_c_l);
EXPORT(__KSYMw16(vswprintf_c),libc_vsnw16printf_c);
EXPORT(__KSYMw32(vswprintf_c),libc_vsnw32printf_c);
EXPORT(__DSYMw16(_vswprintf_c),libd_vsnw16printf_c);
EXPORT(__DSYMw32(vswprintf_c),libd_vsnw32printf_c);
EXPORT(__KSYMw16(vswprintf_c_l),libc_vsnw16printf_c_l);
EXPORT(__KSYMw32(vswprintf_c_l),libc_vsnw32printf_c_l);
EXPORT(__DSYMw16(_vswprintf_c_l),libd_vsnw16printf_c_l);
EXPORT(__DSYMw32(vswprintf_c_l),libd_vsnw32printf_c_l);


/* DOS's ~safe~ format aliases. */
EXPORT(__KSYM(fprintf_s_l),libc_fprintf_l);
EXPORT(__DSYM(_fprintf_s_l),libd_fprintf_l);
EXPORT(__KSYM(fscanf_s_l),libc_fscanf_l);
EXPORT(__DSYM(_fscanf_s_l),libd_fscanf_l);
EXPORT(__KSYM(vfscanf_s_l),libc_vfscanf_l);
EXPORT(__DSYM(_vfscanf_s_l),libd_vfscanf_l);
EXPORT(__KSYM(printf_s_l),libc_printf_l);
EXPORT(__DSYM(_printf_s_l),libd_printf_l);
EXPORT(__KSYM(scanf_s_l),libc_scanf_l);
EXPORT(__DSYM(_scanf_s_l),libd_scanf_l);
EXPORT(__KSYM(vscanf_s_l),libc_vscanf_l);
EXPORT(__DSYM(_vscanf_s_l),libd_vscanf_l);
EXPORT(__KSYM(snprintf_s),libc_snprintf);
EXPORT(__DSYM(_snprintf_s),libd_snprintf);
EXPORT(__KSYM(snprintf_s_l),libc_snprintf_l);
EXPORT(__DSYM(_snprintf_s_l),libd_snprintf_l);
EXPORT(__KSYM(sprintf_s_l),libc_sprintf_l);
EXPORT(__DSYM(_sprintf_s_l),libd_sprintf_l);
EXPORT(__KSYM(sscanf_s_l),libc_sscanf_l);
EXPORT(__DSYM(_sscanf_s_l),libd_sscanf_l);
EXPORT(__KSYM(vsscanf_s_l),libc_vsscanf_l);
EXPORT(__DSYM(_vsscanf_s_l),libd_vsscanf_l);
EXPORT(__KSYM(vfprintf_s_l),libc_vfprintf_l);
EXPORT(__DSYM(_vfprintf_s_l),libd_vfprintf_l);
EXPORT(__KSYM(vprintf_s_l),libc_vprintf_l);
EXPORT(__DSYM(_vprintf_s_l),libd_vprintf_l);
EXPORT(__KSYM(vsnprintf_s),libc_vsnprintf);
EXPORT(__DSYM(_vsnprintf_s),libd_vsnprintf);
EXPORT(__KSYM(vsnprintf_s_l),libc_vsnprintf_l);
EXPORT(__DSYM(_vsnprintf_s_l),libd_vsnprintf_l);
EXPORT(__KSYM(vsprintf_s_l),libc_vsprintf_l);
EXPORT(__DSYM(_vsprintf_s_l),libd_vsprintf_l);
EXPORT(__KSYM(snscanf_s),libc_snscanf);
EXPORT(__DSYM(_snscanf_s),libd_snscanf);
EXPORT(__KSYM(vsnscanf_s),libc_vsnscanf);
EXPORT(__DSYM(_vsnscanf_s),libd_vsnscanf);
EXPORT(__KSYM(snscanf_s_l),libc_snscanf_l);
EXPORT(__DSYM(_snscanf_s_l),libd_snscanf_l);
EXPORT(__KSYM(vsnscanf_s_l),libc_vsnscanf_l);
EXPORT(__DSYM(_vsnscanf_s_l),libd_vsnscanf_l);
EXPORT(__KSYM(fprintf_s),libc_fprintf);
EXPORT(__DSYM(fprintf_s),libd_fprintf);
EXPORT(__KSYM(fscanf_s),libc_fscanf);
EXPORT(__DSYM(fscanf_s),libd_fscanf);
EXPORT(__KSYM(printf_s),libc_printf);
EXPORT(__DSYM(printf_s),libd_printf);
EXPORT(__KSYM(scanf_s),libc_scanf);
EXPORT(__DSYM(scanf_s),libd_scanf);
EXPORT(__KSYM(sprintf_s),libc_sprintf);
EXPORT(__DSYM(sprintf_s),libd_sprintf);
EXPORT(__KSYM(sscanf_s),libc_sscanf);
EXPORT(__DSYM(sscanf_s),libd_sscanf);
EXPORT(__KSYM(vfprintf_s),libc_vfprintf);
EXPORT(__DSYM(vfprintf_s),libd_vfprintf);
EXPORT(__KSYM(vfscanf_s),libc_vfscanf);
EXPORT(__DSYM(vfscanf_s),libd_vfscanf);
EXPORT(__KSYM(vprintf_s),libc_vprintf);
EXPORT(__DSYM(vprintf_s),libd_vprintf);
EXPORT(__KSYM(vscanf_s),libc_vscanf);
EXPORT(__DSYM(vscanf_s),libd_vscanf);
EXPORT(__KSYM(vsprintf_s),libc_vsprintf);
EXPORT(__DSYM(vsprintf_s),libd_vsprintf);
EXPORT(__KSYM(vsscanf_s),libc_vsscanf);
EXPORT(__DSYM(vsscanf_s),libd_vsscanf);
EXPORT(__KSYM(snscanf_s),libc_snscanf);
EXPORT(__KSYMw16(fwprintf_s_l),libc_fw16printf_l);
EXPORT(__KSYMw32(fwprintf_s_l),libc_fw32printf_l);
EXPORT(__DSYMw16(_fwprintf_s_l),libd_fw16printf_l);
EXPORT(__DSYMw32(fwprintf_s_l),libd_fw32printf_l);
EXPORT(__KSYMw16(snwprintf_s),libc_snw16printf);
EXPORT(__KSYMw32(snwprintf_s),libc_snw32printf);
EXPORT(__DSYMw16(_snwprintf_s),libd_snw16printf);
EXPORT(__DSYMw32(snwprintf_s),libd_snw32printf);
EXPORT(__KSYMw16(snwprintf_s_l),libc_snw16printf_l);
EXPORT(__KSYMw32(snwprintf_s_l),libc_snw32printf_l);
EXPORT(__DSYMw16(_snwprintf_s_l),libd_snw16printf_l);
EXPORT(__DSYMw32(snwprintf_s_l),libd_snw32printf_l);
EXPORT(__KSYMw16(swprintf_s_l),libc_sw16printf_l);
EXPORT(__KSYMw32(swprintf_s_l),libc_sw32printf_l);
EXPORT(__DSYMw16(_swprintf_s_l),libd_sw16printf_l);
EXPORT(__DSYMw32(swprintf_s_l),libd_sw32printf_l);
EXPORT(__KSYMw16(swscanf_s_l),libc_sw16scanf_l);
EXPORT(__KSYMw32(swscanf_s_l),libc_sw32scanf_l);
EXPORT(__DSYMw16(_swscanf_s_l),libd_sw16scanf_l);
EXPORT(__DSYMw32(swscanf_s_l),libd_sw32scanf_l);
EXPORT(__KSYMw16(vswscanf_s_l),libc_vsw16scanf_l);
EXPORT(__KSYMw32(vswscanf_s_l),libc_vsw32scanf_l);
EXPORT(__DSYMw16(_vswscanf_s_l),libd_vsw16scanf_l);
EXPORT(__DSYMw32(vswscanf_s_l),libd_vsw32scanf_l);
EXPORT(__KSYMw16(vfwprintf_s_l),libc_vfw16printf_l);
EXPORT(__KSYMw32(vfwprintf_s_l),libc_vfw32printf_l);
EXPORT(__DSYMw16(_vfwprintf_s_l),libd_vfw16printf_l);
EXPORT(__DSYMw32(vfwprintf_s_l),libd_vfw32printf_l);
EXPORT(__KSYMw16(vsnwprintf_s),libc_vsnw16printf);
EXPORT(__KSYMw32(vsnwprintf_s),libc_vsnw32printf);
EXPORT(__DSYMw16(_vsnwprintf_s),libd_vsnw16printf);
EXPORT(__DSYMw32(vsnwprintf_s),libd_vsnw32printf);
EXPORT(__KSYMw16(vsnwprintf_s_l),libc_vsnw16printf_l);
EXPORT(__KSYMw32(vsnwprintf_s_l),libc_vsnw32printf_l);
EXPORT(__DSYMw16(_vsnwprintf_s_l),libd_vsnw16printf_l);
EXPORT(__DSYMw32(vsnwprintf_s_l),libd_vsnw32printf_l);
EXPORT(__KSYMw16(vswprintf_s_l),libc_vsw16printf_l);
EXPORT(__KSYMw32(vswprintf_s_l),libc_vsw32printf_l);
EXPORT(__DSYMw16(_vswprintf_s_l),libd_vsw16printf_l);
EXPORT(__DSYMw32(vswprintf_s_l),libd_vsw32printf_l);
EXPORT(__KSYMw16(vwprintf_s_l),libc_vw16printf_l);
EXPORT(__KSYMw32(vwprintf_s_l),libc_vw32printf_l);
EXPORT(__DSYMw16(_vwprintf_s_l),libd_vw16printf_l);
EXPORT(__DSYMw32(vwprintf_s_l),libd_vw32printf_l);
EXPORT(__KSYMw16(wprintf_s_l),libc_w16printf_l);
EXPORT(__KSYMw32(wprintf_s_l),libc_w32printf_l);
EXPORT(__DSYMw16(_wprintf_s_l),libd_w16printf_l);
EXPORT(__DSYMw32(wprintf_s_l),libd_w32printf_l);
EXPORT(__KSYMw16(wscanf_s_l),libc_w16scanf_l);
EXPORT(__KSYMw32(wscanf_s_l),libc_w32scanf_l);
EXPORT(__DSYMw16(_wscanf_s_l),libd_w16scanf_l);
EXPORT(__DSYMw32(wscanf_s_l),libd_w32scanf_l);
EXPORT(__KSYMw16(vwscanf_s_l),libc_vw16scanf_l);
EXPORT(__KSYMw32(vwscanf_s_l),libc_vw32scanf_l);
EXPORT(__DSYMw16(_vwscanf_s_l),libd_vw16scanf_l);
EXPORT(__DSYMw32(vwscanf_s_l),libd_vw32scanf_l);
EXPORT(__KSYMw16(fwscanf_s_l),libc_fw16scanf_l);
EXPORT(__KSYMw32(fwscanf_s_l),libc_fw32scanf_l);
EXPORT(__DSYMw16(_fwscanf_s_l),libd_fw16scanf_l);
EXPORT(__DSYMw32(fwscanf_s_l),libd_fw32scanf_l);
EXPORT(__KSYMw16(vfwscanf_s_l),libc_vfw16scanf_l);
EXPORT(__KSYMw32(vfwscanf_s_l),libc_vfw32scanf_l);
EXPORT(__DSYMw16(_vfwscanf_s_l),libd_vfw16scanf_l);
EXPORT(__DSYMw32(vfwscanf_s_l),libd_vfw32scanf_l);
EXPORT(__KSYMw16(snwscanf_s),libc_snw16scanf);
EXPORT(__KSYMw32(snwscanf_s),libc_snw32scanf);
EXPORT(__DSYMw16(_snwscanf_s),libd_snw16scanf);
EXPORT(__DSYMw32(snwscanf_s),libd_snw32scanf);
EXPORT(__KSYMw16(vsnwscanf_s),libc_vsnw16scanf);
EXPORT(__KSYMw32(vsnwscanf_s),libc_vsnw32scanf);
EXPORT(__DSYMw16(_vsnwscanf_s),libd_vsnw16scanf);
EXPORT(__DSYMw32(vsnwscanf_s),libd_vsnw32scanf);
EXPORT(__KSYMw16(snwscanf_s_l),libc_snw16scanf_l);
EXPORT(__KSYMw32(snwscanf_s_l),libc_snw32scanf_l);
EXPORT(__DSYMw16(_snwscanf_s_l),libd_snw16scanf_l);
EXPORT(__DSYMw32(snwscanf_s_l),libd_snw32scanf_l);
EXPORT(__KSYMw16(vsnwscanf_s_l),libc_vsnw16scanf_l);
EXPORT(__KSYMw32(vsnwscanf_s_l),libc_vsnw32scanf_l);
EXPORT(__DSYMw16(_vsnwscanf_s_l),libd_vsnw16scanf_l);
EXPORT(__DSYMw32(vsnwscanf_s_l),libd_vsnw32scanf_l);
EXPORT(__KSYMw16(fwprintf_s),libc_fw16printf);
EXPORT(__KSYMw32(fwprintf_s),libc_fw32printf);
EXPORT(__DSYMw16(fwprintf_s),libd_fw16printf);
EXPORT(__DSYMw32(fwprintf_s),libd_fw32printf);
EXPORT(__KSYMw16(swprintf_s),libc_sw16printf);
EXPORT(__KSYMw32(swprintf_s),libc_sw32printf);
EXPORT(__DSYMw16(swprintf_s),libd_sw16printf);
EXPORT(__DSYMw32(swprintf_s),libd_sw32printf);
EXPORT(__KSYMw16(swscanf_s),libc_sw16scanf);
EXPORT(__KSYMw32(swscanf_s),libc_sw32scanf);
EXPORT(__DSYMw16(swscanf_s),libd_sw16scanf);
EXPORT(__DSYMw32(swscanf_s),libd_sw32scanf);
EXPORT(__KSYMw16(vfwprintf_s),libc_vfw16printf);
EXPORT(__KSYMw32(vfwprintf_s),libc_vfw32printf);
EXPORT(__DSYMw16(vfwprintf_s),libd_vfw16printf);
EXPORT(__DSYMw32(vfwprintf_s),libd_vfw32printf);
EXPORT(__KSYMw16(vswprintf_s),libc_vsw16printf);
EXPORT(__KSYMw32(vswprintf_s),libc_vsw32printf);
EXPORT(__DSYMw16(vswprintf_s),libd_vsw16printf);
EXPORT(__DSYMw32(vswprintf_s),libd_vsw32printf);
EXPORT(__KSYMw16(vswscanf_s),libc_vsw16scanf);
EXPORT(__KSYMw32(vswscanf_s),libc_vsw32scanf);
EXPORT(__DSYMw16(vswscanf_s),libd_vsw16scanf);
EXPORT(__DSYMw32(vswscanf_s),libd_vsw32scanf);
EXPORT(__KSYMw16(vwprintf_s),libc_vw16printf);
EXPORT(__KSYMw32(vwprintf_s),libc_vw32printf);
EXPORT(__DSYMw16(vwprintf_s),libd_vw16printf);
EXPORT(__DSYMw32(vwprintf_s),libd_vw32printf);
EXPORT(__KSYMw16(vwscanf_s),libc_vw16scanf);
EXPORT(__KSYMw32(vwscanf_s),libc_vw32scanf);
EXPORT(__DSYMw16(vwscanf_s),libd_vw16scanf);
EXPORT(__DSYMw32(vwscanf_s),libd_vw32scanf);
EXPORT(__KSYMw16(wprintf_s),libc_w16printf);
EXPORT(__KSYMw32(wprintf_s),libc_w32printf);
EXPORT(__DSYMw16(wprintf_s),libd_w16printf);
EXPORT(__DSYMw32(wprintf_s),libd_w32printf);
EXPORT(__KSYMw16(wscanf_s),libc_w16scanf);
EXPORT(__KSYMw32(wscanf_s),libc_w32scanf);
EXPORT(__DSYMw16(wscanf_s),libd_w16scanf);
EXPORT(__DSYMw32(wscanf_s),libd_w32scanf);
EXPORT(__KSYMw16(fwscanf_s),libc_fw16scanf);
EXPORT(__KSYMw32(fwscanf_s),libc_fw32scanf);
EXPORT(__DSYMw16(fwscanf_s),libd_fw16scanf);
EXPORT(__DSYMw32(fwscanf_s),libd_fw32scanf);
EXPORT(__KSYMw16(vfwscanf_s),libc_vfw16scanf);
EXPORT(__KSYMw32(vfwscanf_s),libc_vfw32scanf);
EXPORT(__DSYMw16(vfwscanf_s),libd_vfw16scanf);
EXPORT(__DSYMw32(vfwscanf_s),libd_vfw32scanf);




/* DOS's count-format functions (simply return how much characters are used by the format string) */
EXPORT(__KSYM(scprintf),libc_scprintf);
EXPORT(__DSYM(_scprintf),libd_scprintf);
EXPORT(__KSYM(scprintf_l),libc_scprintf_l);
EXPORT(__DSYM(_scprintf_l),libd_scprintf_l);
EXPORT(__KSYM(scprintf_p),libc_scprintf_p);
EXPORT(__DSYM(_scprintf_p),libd_scprintf_p);
EXPORT(__KSYM(scprintf_p_l),libc_scprintf_p_l);
EXPORT(__DSYM(_scprintf_p_l),libd_scprintf_p_l);
EXPORT(__KSYM(vscprintf),libc_vscprintf);
EXPORT(__DSYM(_vscprintf),libd_vscprintf);
EXPORT(__KSYM(vscprintf_l),libc_vscprintf_l);
EXPORT(__DSYM(_vscprintf_l),libd_vscprintf_l);
EXPORT(__KSYM(vscprintf_p),libc_vscprintf_p);
EXPORT(__DSYM(_vscprintf_p),libd_vscprintf_p);
EXPORT(__KSYM(vscprintf_p_l),libc_vscprintf_p_l);
EXPORT(__DSYM(_vscprintf_p_l),libd_vscprintf_p_l);
EXPORT(__KSYMw16(scwprintf),libc_scw16printf);
EXPORT(__KSYMw32(scwprintf),libc_scw32printf);
EXPORT(__DSYMw16(_scwprintf),libd_scw16printf);
EXPORT(__DSYMw32(scwprintf),libd_scw32printf);
EXPORT(__KSYMw16(scwprintf_l),libc_scw16printf_l);
EXPORT(__KSYMw32(scwprintf_l),libc_scw32printf_l);
EXPORT(__DSYMw16(_scwprintf_l),libd_scw16printf_l);
EXPORT(__DSYMw32(scwprintf_l),libd_scw32printf_l);
EXPORT(__KSYMw16(scwprintf_p),libc_scw16printf_p);
EXPORT(__KSYMw32(scwprintf_p),libc_scw32printf_p);
EXPORT(__DSYMw16(_scwprintf_p),libd_scw16printf_p);
EXPORT(__DSYMw32(scwprintf_p),libd_scw32printf_p);
EXPORT(__KSYMw16(scwprintf_p_l),libc_scw16printf_p_l);
EXPORT(__KSYMw32(scwprintf_p_l),libc_scw32printf_p_l);
EXPORT(__DSYMw16(_scwprintf_p_l),libd_scw16printf_p_l);
EXPORT(__DSYMw32(scwprintf_p_l),libd_scw32printf_p_l);
EXPORT(__KSYMw16(vscwprintf),libc_vscw16printf);
EXPORT(__KSYMw32(vscwprintf),libc_vscw32printf);
EXPORT(__DSYMw16(_vscwprintf),libd_vscw16printf);
EXPORT(__DSYMw32(vscwprintf),libd_vscw32printf);
EXPORT(__KSYMw16(vscwprintf_l),libc_vscw16printf_l);
EXPORT(__KSYMw32(vscwprintf_l),libc_vscw32printf_l);
EXPORT(__DSYMw16(_vscwprintf_l),libd_vscw16printf_l);
EXPORT(__DSYMw32(vscwprintf_l),libd_vscw32printf_l);
EXPORT(__KSYMw16(vscwprintf_p),libc_vscw16printf_p);
EXPORT(__KSYMw32(vscwprintf_p),libc_vscw32printf_p);
EXPORT(__DSYMw16(_vscwprintf_p),libd_vscw16printf_p);
EXPORT(__DSYMw32(vscwprintf_p),libd_vscw32printf_p);
EXPORT(__KSYMw16(vscwprintf_p_l),libc_vscw16printf_p_l);
EXPORT(__KSYMw32(vscwprintf_p_l),libc_vscw32printf_p_l);
EXPORT(__DSYMw16(_vscwprintf_p_l),libd_vscw16printf_p_l);
EXPORT(__DSYMw32(vscwprintf_p_l),libd_vscw32printf_p_l);


/* GLC aliases */
EXPORT_STRONG(__strftime_l,libc_strftime_l);
EXPORT_STRONG(__wcsftime_l,libc_w32ftime_l);

/*
TODO: GLC aliases / extensions
        _IO_fprintf
        _IO_printf
        _IO_sprintf
        _IO_sscanf
        _IO_vfprintf
        _IO_vfscanf
        _IO_vsprintf

        __isoc99_fscanf
        __isoc99_fwscanf
        __isoc99_scanf
        __isoc99_sscanf
        __isoc99_swscanf
        __isoc99_vfscanf
        __isoc99_vfwscanf
        __isoc99_vscanf
        __isoc99_vsscanf
        __isoc99_vswscanf
        __isoc99_vwscanf
        __isoc99_wscanf

        __printf_fp

        __asprintf_chk
        __dprintf_chk
        __fprintf_chk
        __fwprintf_chk
        __printf_chk
        __snprintf_chk
        __sprintf_chk
        __swprintf_chk
        __vasprintf_chk
        __vdprintf_chk
        __vfprintf_chk
        __vfwprintf_chk
        __vprintf_chk
        __vsnprintf_chk
        __vsprintf_chk
        __vswprintf_chk
        __vwprintf_chk
        __wprintf_chk
*/




/* Deprecated (now stub) debug variants of KOS Mk2 allocator functions. */
INTERN char *ATTR_CDECL
libc_compat_mk2_strdupf_d(DEBUGINFO_UNUSED, char const *__restrict format, ...) {
 char *COMPILER_IGNORE_UNINITIALIZED(result);
 va_list __EXCEPTVAR_VALIST args;
 va_start(args,format);
 __TRY_VALIST {
  result = libc_vstrdupf(format,args);
 } __FINALLY_VALIST {
  va_end(args);
 }
 return result;
}
EXPORT(_strdupf_d,libc_compat_mk2_strdupf_d);
EXPORT(_vstrdupf_d,libc_vstrdupf);


DECL_END

#endif /* !GUARD_LIBS_LIBC_FORMAT_C */
