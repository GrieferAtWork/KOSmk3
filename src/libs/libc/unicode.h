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
#ifndef GUARD_LIBS_LIBC_UNICODE_H
#define GUARD_LIBS_LIBC_UNICODE_H 1

#include "libc.h"
#include <kos/types.h>

#ifdef __CC__
DECL_BEGIN


/* ===================================================================================== */
/*     UNICODE                                                                           */
/* ===================================================================================== */
#ifndef __std_mbstate_t_defined
#define __std_mbstate_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __mbstate_t mbstate_t;
__NAMESPACE_STD_END
#endif /* !__std_mbstate_t_defined */
#ifndef __mbstate_t_defined
#define __mbstate_t_defined 1
__NAMESPACE_STD_USING(mbstate_t)
#endif /* !__mbstate_t_defined */
INTDEF size_t LIBCCALL libc_utf8to32(char const *__restrict utf8, size_t utf8chars, char32_t *__restrict utf32, size_t bufchars32, mbstate_t *__restrict state, u32 mode);
INTDEF size_t LIBCCALL libc_utf8to16(char const *__restrict utf8, size_t utf8chars, char16_t *__restrict utf16, size_t bufchars16, mbstate_t *__restrict state, u32 mode);
INTDEF size_t LIBCCALL libc_utf32to8(char32_t const *__restrict utf32, size_t utf32chars, char *__restrict utf8, size_t bufchars8, mbstate_t *__restrict state, u32 mode);
INTDEF size_t LIBCCALL libc_utf16to8(char16_t const *__restrict utf16, size_t utf16chars, char *__restrict utf8, size_t bufchars8, mbstate_t *__restrict state, u32 mode);
INTDEF ATTR_MALLOC __MALL_DEFAULT_ALIGNED char16_t *LIBCCALL libc_utf8to16m(char const *__restrict utf8);
INTDEF ATTR_MALLOC __MALL_DEFAULT_ALIGNED char32_t *LIBCCALL libc_utf8to32m(char const *__restrict utf8);
INTDEF ATTR_MALLOC __MALL_DEFAULT_ALIGNED char *LIBCCALL libc_utf16to8m(char16_t const *__restrict utf16);
INTDEF ATTR_MALLOC __MALL_DEFAULT_ALIGNED char *LIBCCALL libc_utf32to8m(char32_t const *__restrict utf32);
INTDEF ATTR_MALLOC __MALL_DEFAULT_ALIGNED char16_t *LIBCCALL libc_utf8to16ms(char const *__restrict utf8, size_t utf8chars);
INTDEF ATTR_MALLOC __MALL_DEFAULT_ALIGNED char32_t *LIBCCALL libc_utf8to32ms(char const *__restrict utf8, size_t utf8chars);
INTDEF ATTR_MALLOC __MALL_DEFAULT_ALIGNED char *LIBCCALL libc_utf16to8ms(char16_t const *__restrict utf16, size_t utf16chars);
INTDEF ATTR_MALLOC __MALL_DEFAULT_ALIGNED char *LIBCCALL libc_utf32to8ms(char32_t const *__restrict utf32, size_t utf32chars);
INTDEF ssize_t LIBCCALL libc_format_w16sztomb(pformatprinter printer, void *closure, char16_t const *__restrict c16, size_t c16len, mbstate_t *__restrict ps, u32 mode);
INTDEF ssize_t LIBCCALL libc_format_w32sztomb(pformatprinter printer, void *closure, char32_t const *__restrict c32, size_t c32len, mbstate_t *__restrict ps, u32 mode);
INTDEF ssize_t LIBCCALL libc_format_w16sntomb(pformatprinter printer, void *closure, char16_t const *__restrict c16, size_t c16max, mbstate_t *__restrict ps, u32 mode);
INTDEF ssize_t LIBCCALL libc_format_w32sntomb(pformatprinter printer, void *closure, char32_t const *__restrict c32, size_t c32max, mbstate_t *__restrict ps, u32 mode);
INTDEF size_t LIBCCALL libc_Xutf8to32(char const *__restrict utf8, size_t utf8chars, char32_t *__restrict utf32, size_t bufchars32, mbstate_t *__restrict state, u32 mode);
INTDEF size_t LIBCCALL libc_Xutf8to16(char const *__restrict utf8, size_t utf8chars, char16_t *__restrict utf16, size_t bufchars16, mbstate_t *__restrict state, u32 mode);
INTDEF size_t LIBCCALL libc_Xutf32to8(char32_t const *__restrict utf32, size_t utf32chars, char *__restrict utf8, size_t bufchars8, mbstate_t *__restrict state, u32 mode);
INTDEF size_t LIBCCALL libc_Xutf16to8(char16_t const *__restrict utf16, size_t utf16chars, char *__restrict utf8, size_t bufchars8, mbstate_t *__restrict state, u32 mode);
INTDEF ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED char16_t *LIBCCALL libc_Xutf8to16m(char const *__restrict utf8);
INTDEF ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED char32_t *LIBCCALL libc_Xutf8to32m(char const *__restrict utf8);
INTDEF ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED char *LIBCCALL libc_Xutf16to8m(char16_t const *__restrict utf16);
INTDEF ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED char *LIBCCALL libc_Xutf32to8m(char32_t const *__restrict utf32);
INTDEF ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED char16_t *LIBCCALL libc_Xutf8to16ms(char const *__restrict utf8, size_t utf8chars);
INTDEF ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED char32_t *LIBCCALL libc_Xutf8to32ms(char const *__restrict utf8, size_t utf8chars);
INTDEF ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED char *LIBCCALL libc_Xutf16to8ms(char16_t const *__restrict utf16, size_t utf16chars);
INTDEF ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED char *LIBCCALL libc_Xutf32to8ms(char32_t const *__restrict utf32, size_t utf32chars);
INTDEF ssize_t LIBCCALL libc_Xformat_w16sztomb(pformatprinter printer, void *closure, char16_t const *__restrict c16, size_t c16len, mbstate_t *__restrict ps, u32 mode);
INTDEF ssize_t LIBCCALL libc_Xformat_w32sztomb(pformatprinter printer, void *closure, char32_t const *__restrict c32, size_t c32len, mbstate_t *__restrict ps, u32 mode);
INTDEF ssize_t LIBCCALL libc_Xformat_w16sntomb(pformatprinter printer, void *closure, char16_t const *__restrict c16, size_t c16max, mbstate_t *__restrict ps, u32 mode);
INTDEF ssize_t LIBCCALL libc_Xformat_w32sntomb(pformatprinter printer, void *closure, char32_t const *__restrict c32, size_t c32max, mbstate_t *__restrict ps, u32 mode);
#define libc_api_utf8to16(dst,dstlen,src,srclen) \
 XBLOCK({ mbstate_t state = MBSTATE_INIT; \
          size_t req = libc_utf8to16(src,srclen,dst,dstlen,&state,0); \
          XRETURN unlikely(req >= srclen) ? (libc_seterrno(req == UNICODE_ERROR ? EINVAL : ERANGE),NULL) : dst; })
#define libc_api_utf8to32(dst,dstlen,src,srclen) \
 XBLOCK({ mbstate_t state = MBSTATE_INIT; \
          size_t req = libc_utf8to32(src,srclen,dst,dstlen,&state,0); \
          XRETURN unlikely(req >= srclen) ? (libc_seterrno(req == UNICODE_ERROR ? EINVAL : ERANGE),NULL) : dst; })
/* Return the length (that is amount of indexable characters) of a
 * given utf-8 string containing 'utf8chars' characters.
 * >> char   utf8[] = "This is a UTF-8 string";
 * >> size_t utf8chars = (sizeof(u16_string)/sizeof(char16_t))-1; // Amount of indexable character units.
 * >> size_t utf8len   = libc_utf16len(u16_string);
 * >> assert(utf8len <= utf16chars); */
#define libc_utf8len16(utf8,utf8chars) XBLOCK({ mbstate_t state = MBSTATE_INIT; XRETURN libc_utf8to16(utf8,utf8chars,NULL,0,&state); })
#define libc_utf8len32(utf8,utf8chars) XBLOCK({ mbstate_t state = MBSTATE_INIT; XRETURN libc_utf8to32(utf8,utf8chars,NULL,0,&state); })

INTDEF u8 const utf8_sequence_len[256];
INTDEF u8 const utf8_offset[4];

#define UNI_SURROGATE_HIGH_BEGIN 0xd800
#define UNI_SURROGATE_HIGH_END   0xdbff
#define UNI_SURROGATE_LOW_BEGIN  0xdc00
#define UNI_SURROGATE_LOW_END    0xdfff


DECL_END
#endif /* __CC__ */

#endif /* !GUARD_LIBS_LIBC_UNICODE_H */
