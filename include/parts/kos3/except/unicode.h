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
#ifndef _PARTS_KOS3_EXCEPT_UNICODE_H
#define _PARTS_KOS3_EXCEPT_UNICODE_H 1

#include "__stdinc.h"
#include "parts/kos2/malldefs.h"
#ifndef _UNICODE_H
#include <unicode.h>
#endif

#if defined(__CC__) && !defined(__KERNEL__) && defined(__USE_EXCEPT)
__SYSDECL_BEGIN

/* Convert 'utf8chars' utf8-characters  to utf-32, storing up to `bufchars32' characters
 * (that is `bufchars32*4' bytes) in 'utf32' and returning the actual amount of stored
 * characters (including a terminating NUL-character that will automatically be
 * appended with the 'utf32'-buffer is of sufficient length to represent the
 * entirety of the provided 'utf8' buffer).
 * @param: mode: Set of `UNICODE_F_*' found below (Unicode mode flags)
 * @return: UNICODE_ERROR: [!UNICODE_F_NOFAIL] An encoding error occurred and
 *                         `state' and 'utf32' are left in an undefined state.
 *                          You may pass `UNICODE_F_NOFAIL' to instead emit a
 *                         `UNICODE_REPLACEMENT' character.
 * >> wchar_t buf[128]; size_t buflen;
 * >> mbstate_t state = MBSTATE_INIT;
 * >> char *text = "Encode this text in UTF-32";
 * >> buflen = uni_utf8to32(text,(size_t)-1,buf,sizeof(buf),&state,UNICODE_F_NORMAL);
 * NOTE: Other functions behave the same, but for different encodings. */
__LIBC size_t (__LIBCCALL Xuni_utf8to32)(char const *__restrict __utf8, size_t __utf8chars,
                                         char32_t *__restrict __utf32, size_t __bufchars32,
                                         mbstate_t *__restrict __state, __UINT32_TYPE__ __mode);
__LIBC size_t (__LIBCCALL Xuni_utf8to16)(char const *__restrict __utf8, size_t __utf8chars,
                                         char16_t *__restrict __utf16, size_t __bufchars16,
                                         mbstate_t *__restrict __state, __UINT32_TYPE__ __mode);
__LIBC size_t (__LIBCCALL Xuni_utf32to8)(char32_t const *__restrict __utf32, size_t __utf32chars,
                                         char *__restrict __utf8, size_t __bufchars8,
                                         mbstate_t *__restrict __state, __UINT32_TYPE__ __mode);
__LIBC size_t (__LIBCCALL Xuni_utf16to8)(char16_t const *__restrict __utf16, size_t __utf16chars,
                                         char *__restrict __utf8, size_t __bufchars8,
                                         mbstate_t *__restrict __state, __UINT32_TYPE__ __mode);

/* Helper functions that return a malloc'ed string, or NULL upon error. */
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED char16_t *(__LIBCCALL Xuni_utf8to16m)(char const *__restrict __utf8);
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED char32_t *(__LIBCCALL Xuni_utf8to32m)(char const *__restrict __utf8);
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED char *(__LIBCCALL Xuni_utf16to8m)(char16_t const *__restrict __utf16);
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED char *(__LIBCCALL Xuni_utf32to8m)(char32_t const *__restrict __utf32);
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED char16_t *(__LIBCCALL Xuni_utf8to16ms)(char const *__restrict __utf8, size_t __utf8chars);
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED char32_t *(__LIBCCALL Xuni_utf8to32ms)(char const *__restrict __utf8, size_t __utf8chars);
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED char *(__LIBCCALL Xuni_utf16to8ms)(char16_t const *__restrict __utf16, size_t __utf16chars);
__LIBC __ATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED char *(__LIBCCALL Xuni_utf32to8ms)(char32_t const *__restrict __utf32, size_t __utf32chars);

/* Return the length (that is amount of indexable characters) of a
 * given utf-8 string containing 'utf8chars' characters.
 * >> char   utf8[] = "This is a UTF-8 string";
 * >> size_t utf8chars = (sizeof(u16_string)/sizeof(char16_t))-1; // Amount of indexable character units.
 * >> size_t utf8len   = uni_utf16len(u16_string);
 * >> assert(utf8len <= utf16chars); */
#define Xuni_utf8len16(utf8,utf8chars) __XBLOCK({ mbstate_t state = MBSTATE_INIT; __XRETURN Xuni_utf8to16(utf8,utf8chars,NULL,0,&state); })
#define Xuni_utf8len32(utf8,utf8chars) __XBLOCK({ mbstate_t state = MBSTATE_INIT; __XRETURN Xuni_utf8to32(utf8,utf8chars,NULL,0,&state); })

__SYSDECL_END
#endif

#endif /* !_PARTS_KOS3_EXCEPT_UNICODE_H */
