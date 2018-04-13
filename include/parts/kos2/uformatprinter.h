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
#ifndef _PARTS_KOS2_UFORMATPRINTER_H
#define _PARTS_KOS2_UFORMATPRINTER_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <bits/mbstate.h>
#include <parts/kos2/pformatprinter.h>

#ifdef __CRT_KOS

__SYSDECL_BEGIN

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

#ifndef __pw16formatprinter_defined
#define __pw16formatprinter_defined 1
typedef __ssize_t (__LIBCCALL *pw16formatprinter)(char16_t const *__restrict __data, __size_t __datalen, void *__closure);
typedef __ssize_t (__LIBCCALL *pw32formatprinter)(char32_t const *__restrict __data, __size_t __datalen, void *__closure);
#endif /* !__pw16formatprinter_defined */

#ifndef __w16printer_defined
#define __w16printer_defined 1
/* Printer-style multi-byte string to utf16/32 or wide-char conversion.
 * >> ssize_t LIBCCALL
 * >> my_wprinter(wchar_t const *__restrict data,
 * >>             size_t datalen, void *closure) {
 * >>     return printf("{WSTR:%$ls}",datalen,data);
 * >> }
 * >> 
 * >> void foo(void) {
 * >>     struct wprinter p;
 * >>     wprinter_init(&p,&my_wprinter,NULL);
 * >>     format_printf(&wprinter_print,&p,"This string %s\n",
 * >>                   "is converted to wide encoding");
 * >>     wprinter_fini(&p);
 * >> }
 */
#ifdef __BUILDING_LIBC
#define __DEFINE_PRINTER(T,Tpfp) { Tpfp p_printer; void *p_closure; T *p_buffer; __size_t p_buflen; mbstate_t p_mbstate; void *p_padding; }
#else
#define __DEFINE_PRINTER(T,Tpfp) { Tpfp __p_printer; void *__p_closure; T *__p_buffer; __size_t __p_buflen; mbstate_t __p_mbstate; void *__p_padding; }
#endif
struct w16printer __DEFINE_PRINTER(char16_t,pw16formatprinter);
struct w32printer __DEFINE_PRINTER(char32_t,pw32formatprinter);
#undef __DEFINE_PRINTER
#endif /* !__w16printer_defined */

#ifndef W16PRINTER_INIT
#define W16PRINTER_INIT(printer,closure)   {printer,closure,NULL,0,__MBSTATE_INIT,NULL}
#define W32PRINTER_INIT(printer,closure)   {printer,closure,NULL,0,__MBSTATE_INIT,NULL}
#endif

__REDIRECT_EXCEPT_W16(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,format_w16sztomb,(pformatprinter __printer, void *__closure, char16_t const *__restrict __w16, __size_t __w16len, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),format_wcsztomb,(__printer,__closure,__w16,__w16len,__ps,__mode))
__REDIRECT_EXCEPT_W32(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,format_w32sztomb,(pformatprinter __printer, void *__closure, char32_t const *__restrict __w32, __size_t __w32len, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),format_wcsztomb,(__printer,__closure,__w32,__w32len,__ps,__mode))
__REDIRECT_EXCEPT_W16(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,format_w16sntomb,(pformatprinter __printer, void *__closure, char16_t const *__restrict __w16, __size_t __w16max, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),format_wcsntomb,(__printer,__closure,__w16,__w16max,__ps,__mode))
__REDIRECT_EXCEPT_W32(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,format_w32sntomb,(pformatprinter __printer, void *__closure, char32_t const *__restrict __w32, __size_t __w32max, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),format_wcsntomb,(__printer,__closure,__w32,__w32max,__ps,__mode))
#ifdef __USE_EXCEPT
__REDIRECT_W16(__LIBC,__PORT_KOSONLY,__size_t,__LIBCCALL,Xformat_w16sztomb,(pformatprinter __printer, void *__closure, char16_t const *__restrict __w16, __size_t __w16len, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),Xformat_wcsztomb,(__printer,__closure,__w16,__w16len,__ps,__mode))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY,__size_t,__LIBCCALL,Xformat_w32sztomb,(pformatprinter __printer, void *__closure, char32_t const *__restrict __w32, __size_t __w32len, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),Xformat_wcsztomb,(__printer,__closure,__w32,__w32len,__ps,__mode))
__REDIRECT_W16(__LIBC,__PORT_KOSONLY,__size_t,__LIBCCALL,Xformat_w16sntomb,(pformatprinter __printer, void *__closure, char16_t const *__restrict __w16, __size_t __w16max, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),Xformat_wcsntomb,(__printer,__closure,__w16,__w16max,__ps,__mode))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY,__size_t,__LIBCCALL,Xformat_w32sntomb,(pformatprinter __printer, void *__closure, char32_t const *__restrict __w32, __size_t __w32max, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),Xformat_wcsntomb,(__printer,__closure,__w32,__w32max,__ps,__mode))
#endif /* __USE_EXCEPT */


__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,w16printer_init,(struct w16printer *__restrict wp, pw16formatprinter printer, void *__closure),wprinter_init,(wp,printer,__closure))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,w32printer_init,(struct w32printer *__restrict wp, pw32formatprinter printer, void *__closure),wprinter_init,(wp,printer,__closure))
__REDIRECT_W16_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,w16printer_fini,(struct w16printer *__restrict wp),wprinter_fini,(wp))
__REDIRECT_W32_VOID(__LIBC,__PORT_KOSONLY,__LIBCCALL,w32printer_fini,(struct w32printer *__restrict wp),wprinter_fini,(wp))
__REDIRECT_EXCEPT_W16(__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,w16printer_print,(char const *__restrict __data, __size_t __datalen, void *__closure),wprinter_print,(__data,__datalen,__closure))
__REDIRECT_EXCEPT_W32(__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,w32printer_print,(char const *__restrict __data, __size_t __datalen, void *__closure),wprinter_print,(__data,__datalen,__closure))
#ifdef __USE_EXCEPT
__REDIRECT_W16(__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,Xw16printer_print,(char const *__restrict __data, __size_t __datalen, void *__closure),Xwprinter_print,(__data,__datalen,__closure))
__REDIRECT_W32(__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,Xw32printer_print,(char const *__restrict __data, __size_t __datalen, void *__closure),Xwprinter_print,(__data,__datalen,__closure))
#endif /* __USE_EXCEPT */




/* Deprecated names from the Mk2 era. */
#ifdef __USE_KOS_DEPRECATED
#ifndef __pc16formatprinter_defined
#define __pc16formatprinter_defined 1
typedef pw16formatprinter pc16formatprinter;
typedef pw32formatprinter pc32formatprinter;
#endif /* !__pw16formatprinter_defined */
#ifndef C16PRINTER_INIT
#define C16PRINTER_INIT(printer,closure) W16PRINTER_INIT(printer,closure)
#define C32PRINTER_INIT(printer,closure) W32PRINTER_INIT(printer,closure)
#endif /* !C16PRINTER_INIT */
#endif /* __USE_KOS_DEPRECATED */


__SYSDECL_END

#endif /* __CRT_KOS */

#endif /* !_PARTS_KOS2_UFORMATPRINTER_H */
