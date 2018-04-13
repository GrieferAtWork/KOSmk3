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
#ifndef _PARTS_KOS2_WFORMATPRINTER_H
#define _PARTS_KOS2_WFORMATPRINTER_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <bits/mbstate.h>
#include <parts/kos2/pformatprinter.h>


__SYSDECL_BEGIN

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef __KERNEL__
#ifdef __CRT_KOS

#ifndef __pwformatprinter_defined
#define __pwformatprinter_defined 1
typedef __ssize_t (__LIBCCALL *pwformatprinter)(wchar_t const *__restrict __data, __size_t __datalen, void *__closure);
#endif /* !__pwformatprinter_defined */

#ifndef __wprinter_defined
#define __wprinter_defined 1
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
struct wprinter   __DEFINE_PRINTER(wchar_t,pwformatprinter);
#undef __DEFINE_PRINTER
#endif /* !__wprinter_defined */

#ifndef WPRINTER_INIT
#define WPRINTER_INIT(printer,closure)   {printer,closure,NULL,0,__MBSTATE_INIT,NULL}
#endif


/* Generic unicode/wide-string to utf8 conversion, using a format-printer as target.
 * NOTE: The given `(C(16|32)|WCS)LEN' is the absolute amount of encoded characters,
 *       meaning that any NUL-characters before then are printed as well.
 *       To use strnlen-style semantics, use `format_*sntomb' instead.
 * NOTE: Upon encoding error, errno is set to `EILSEQ' and '-1' is returned.
 *       @EXCEPT: Throw an `TODO:E_* code for this' instead.
 * @param: MODE: Set of `UNICODE_F_*' from `<unicode.h>' (Only `UNICODE_F_NOFAIL' changes the behavior)
 * HINT: These functions are also used to implement `"%ls"'. */
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,format_wcsztomb,(pformatprinter __printer, void *__closure, wchar_t const *__restrict __wcs, __size_t __wcslen, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),(__printer,__closure,__wcs,__wcslen,__ps,__mode))
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,__EXCEPT_SELECT(__size_t,__ssize_t),__LIBCCALL,format_wcsntomb,(pformatprinter __printer, void *__closure, wchar_t const *__restrict __wcs, __size_t __wcsmax, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode),(__printer,__closure,__wcs,__wcsmax,__ps,__mode))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __size_t (__LIBCCALL Xformat_wcsztomb)(pformatprinter __printer, void *__closure, wchar_t const *__restrict __wcs, __size_t __wcslen, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode);
__LIBC __PORT_KOSONLY __size_t (__LIBCCALL Xformat_wcsntomb)(pformatprinter __printer, void *__closure, wchar_t const *__restrict __wcs, __size_t __wcsmax, mbstate_t *__restrict __ps, __UINT32_TYPE__ __mode);
#endif /* __USE_EXCEPT */



/* Convert UTF-8 to wide-characters using a streaming printer. */
__LIBC __PORT_KOSONLY void (__LIBCCALL wprinter_init)(struct wprinter *__restrict wp, pwformatprinter printer, void *__closure);
__LIBC __PORT_KOSONLY void (__LIBCCALL wprinter_fini)(struct wprinter *__restrict wp);
/* NOTE: Wide-character printers forward the return value of the underlying printer,
 *       or -1 if a format error occurred, alongside setting errno to EILSEQ. */
__REDIRECT_EXCEPT(__LIBC,__PORT_KOSONLY,__ssize_t,__LIBCCALL,wprinter_print,(char const *__restrict __data, __size_t __datalen, void *__closure),(__data,__datalen,__closure))
#ifdef __USE_EXCEPT
__LIBC __PORT_KOSONLY __ssize_t (__LIBCCALL Xwprinter_print)(char const *__restrict __data, __size_t __datalen, void *__closure);
#endif /* __USE_EXCEPT */

#endif /* __CRT_KOS */
#endif /* !__KERNEL__ */

__SYSDECL_END


#endif /* !_PARTS_KOS2_WFORMATPRINTER_H */
