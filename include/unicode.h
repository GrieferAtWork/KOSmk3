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
#ifndef _UNICODE_H
#define _UNICODE_H 1

#include "__stdinc.h"
#include "parts/kos2/malldefs.h"
#include <errno.h>
#include <features.h>
#include <bits/mbstate.h>
#include <hybrid/string.h>
#include <hybrid/typecore.h>

#ifndef __CRT_KOS
#error "<unicode.h> is not supported by the linked libc"
#endif /* !__CRT_KOS */

/* KOS Extension header for universal Unicode conversion functions. */

__SYSDECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#ifndef __char16_t_defined
#define __char16_t_defined 1
typedef __CHAR16_TYPE__ char16_t;
typedef __CHAR32_TYPE__ char32_t;
#endif /* !__char16_t_defined */

#ifndef MBSTATE_INIT
#define MBSTATE_INIT     __MBSTATE_INIT
#endif /* !MBSTATE_INIT */

#ifndef mbstate_reset
#define mbstate_reset(p)  __hybrid_memset(p,0,sizeof(mbstate_t))
#endif

#define UNICODE_MB_MAX    6

#ifndef __KERNEL__
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
 * NOTE: Other functions behave the same, but for different encodings.
 * @return: * : [!UNICODE_F_UPDATESRC]                          The amount of required characters
 * @return: * : [UNICODE_F_UPDATESRC]                           The amount of converted characters
 * @return: * : [UNICODE_F_UPDATESRC][UNICODE_F_NOZEROTERM]     The amount of converted characters
 * @return: * : [UNICODE_F_UPDATESRC][!UNICODE_F_NOZEROTERM]    The amount of converted characters +0; the source string wasn't fully processed
 * @return: * : [UNICODE_F_UPDATESRC][!UNICODE_F_NOZEROTERM]    The amount of converted characters +1 (may equal `BUFCHARS+1'); the source string was fully processed
 * @return: * : [UNICODE_F_UPDATESRC][UNICODE_F_ALWAYSZEROTERM] The amount of converted characters +1 (may equal `BUFCHARS+1')
 * NOTE: It is recommended to also pass `UNICODE_F_NOZEROTERM'
 *       when using `UNICODE_F_UPDATESRC' in order to prevent
 *       confusion about who should write the trailing NUL.
 */
__REDIRECT_EXCEPT(__LIBC,,size_t,__LIBCCALL,uni_utf8to32,
                 (char const *__restrict __utf8, size_t __utf8chars,
                  char32_t *__restrict __utf32, size_t __bufchars32,
                  mbstate_t *__restrict __state, __UINT32_TYPE__ __mode),
                 (__utf8,__utf8chars,__utf32,__bufchars32,__state,__mode))
__REDIRECT_EXCEPT(__LIBC,,size_t,__LIBCCALL,uni_utf8to16,
                 (char const *__restrict __utf8, size_t __utf8chars,
                  char16_t *__restrict __utf16, size_t __bufchars16,
                  mbstate_t *__restrict __state, __UINT32_TYPE__ __mode),
                 (__utf8,__utf8chars,__utf16,__bufchars16,__state,__mode))
__REDIRECT_EXCEPT(__LIBC,,size_t,__LIBCCALL,uni_utf32to8,
                 (char32_t const *__restrict __utf32, size_t __utf32chars,
                  char *__restrict __utf8, size_t __bufchars8,
                  mbstate_t *__restrict __state, __UINT32_TYPE__ __mode),
                 (__utf32,__utf32chars,__utf8,__bufchars8,__state,__mode))
__REDIRECT_EXCEPT(__LIBC,,size_t,__LIBCCALL,uni_utf16to8,
                 (char16_t const *__restrict __utf16, size_t __utf16chars,
                  char *__restrict __utf8, size_t __bufchars8,
                  mbstate_t *__restrict __state, __UINT32_TYPE__ __mode),
                 (__utf16,__utf16chars,__utf8,__bufchars8,__state,__mode))
#define UNICODE_ERROR           ((size_t)-1)
#if __KOS_VERSION__ < 300
#define UNICODE_UTF16HALF       ((size_t)-3)
#endif
#define UNICODE_IS_STATEDEPENDENT 1 /* ??? (I think...) */
#define UNICODE_REPLACEMENT      '?'

/* Unicode mode flags. */
#define UNICODE_F_NORMAL         0x0000 /* Regular string conversion. */
#define UNICODE_F_NOZEROTERM     0x0001 /* Don't terminate with a NUL-character that is included in the return value. */
#define UNICODE_F_ALWAYSZEROTERM 0x0002 /* Always zero-terminate, even when an incomplete sequence is passed. */
#define UNICODE_F_STOPONNUL      0x0004 /* Stop if a NUL-character is encountered in the input string (`UNICODE_F_UPDATESRC' will update the input string to the NUL-character). */
#define UNICODE_F_DOSINGLE       0x0008 /* Stop after the first fully encoded character. */
#define UNICODE_F_UPDATESRC      0x0020 /* Treat input arguments (the first two) as pointers and update them before returning upon success.
                                         * NOTE: When set, the first two arguments are interpreted with one additional indirection.
                                         * Also: Stop if the target buffer becomes full, and return the number of written characters.
                                         * WARNING: When `UNICODE_F_NOZEROTERM' isn't set, the return value after the buffer having filled up is 1+ the target buffer size.
                                         * >> In essence, this flag enables use of conversion functions as packet-style streams, in that
                                         *    they will process as much data as possible (returning how much exactly), and will continue
                                         *    to process data when invoked again. */
#define UNICODE_F_SETERRNO       0x0040 /* Set `errno' to `EILSEQ' ('ERROR_NO_UNICODE_TRANSLATION')
                                         * if a malformed sequence causes `UNICODE_ERROR' to be returned.
                                         * NOTE: This flag is ignored when `UNICODE_F_NOFAIL' is passed.
                                         * NOTE: In exception-mode, rather than setting `EILSEQ', `E_UNICODE_ERROR' is thrown.
                                         *       When not set, exception-mode and regular mode are identical for the functions above. */
#define UNICODE_F_NOFAIL         0x0080 /* Never fail and emit `UNICODE_REPLACEMENT' for illegal characters. */
#if __KOS_VERSION__ >= 300
#define UNICODE_F_BUFERROR       0x0100 /* Throw an `E_BUFFER_TOO_SMALL' error, or set `errno = ERANGE' if the provided target
                                         * buffer is too small When not set, the return value is always the required buffer size.
                                         * NOTE: This flag may be ignored when `UNICODE_F_UPDATESRC' is set. */
#else
#define UNICODE_F_UTF16HALF      0x0010 /* For `uni_utf8to16': When the target buffer is too small, return `UNICODE_UTF16HALF' and use the shift state to return the second half next call. */
#endif
#ifdef __BUILDING_LIBC
#define __UNICODE_F_EXCEPT       0x8000 /* Operate in exception-mode. */
#endif


/* Helper functions that return a malloc'ed string, or NULL upon error. */
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED,char16_t *,__LIBCCALL,uni_utf8to16m,(char const *__restrict __utf8),(__utf8))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED,char32_t *,__LIBCCALL,uni_utf8to32m,(char const *__restrict __utf8),(__utf8))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED,char *,__LIBCCALL,uni_utf16to8m,(char16_t const *__restrict __utf16),(__utf16))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED,char *,__LIBCCALL,uni_utf32to8m,(char32_t const *__restrict __utf32),(__utf32))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED,char16_t *,__LIBCCALL,uni_utf8to16ms,(char const *__restrict __utf8, size_t __utf8chars),(__utf8,__utf8chars))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED,char32_t *,__LIBCCALL,uni_utf8to32ms,(char const *__restrict __utf8, size_t __utf8chars),(__utf8,__utf8chars))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED,char *,__LIBCCALL,uni_utf16to8ms,(char16_t const *__restrict __utf16, size_t __utf16chars),(__utf16,__utf16chars))
__REDIRECT_EXCEPT(__LIBC,__XATTR_RETNONNULL __ATTR_MALLOC __WUNUSED __MALL_DEFAULT_ALIGNED,char *,__LIBCCALL,uni_utf32to8ms,(char32_t const *__restrict __utf32, size_t __utf32chars),(__utf32,__utf32chars))

#define uni_api_utf8to16(dst,dstlen,src,srclen) \
 __XBLOCK({ mbstate_t __state = MBSTATE_INIT; \
            size_t __req = uni_utf8to16(src,srclen,dst,dstlen,&__state,0); \
            __XRETURN __unlikely(__req >= (srclen)) ? (__set_errno(__req == UNICODE_ERROR ? EINVAL : ERANGE),NULL) : (dst); })
#define uni_api_utf8to32(dst,dstlen,src,srclen) \
 __XBLOCK({ mbstate_t __state = MBSTATE_INIT; \
            size_t __req = uni_utf8to32(src,srclen,dst,dstlen,&__state,0); \
            __XRETURN __unlikely(__req >= (srclen)) ? (__set_errno(__req == UNICODE_ERROR ? EINVAL : ERANGE),NULL) : (dst); })

/* Return the length (that is amount of indexable characters) of a
 * given utf-8 string containing 'utf8chars' characters.
 * >> char   utf8[] = "This is a UTF-8 string";
 * >> size_t utf8chars = (sizeof(u16_string)/sizeof(char16_t))-1; // Amount of indexable character units.
 * >> size_t utf8len   = uni_utf16len(u16_string);
 * >> assert(utf8len <= utf16chars); */
#define uni_utf8len16(utf8,utf8chars) __XBLOCK({ mbstate_t state = MBSTATE_INIT; __XRETURN uni_utf8to16(utf8,utf8chars,NULL,0,&state); })
#define uni_utf8len32(utf8,utf8chars) __XBLOCK({ mbstate_t state = MBSTATE_INIT; __XRETURN uni_utf8to32(utf8,utf8chars,NULL,0,&state); })

#endif /* !__KERNEL__ */

__SYSDECL_END

#ifdef __USE_EXCEPT
#include "parts/kos3/except/unicode.h"
#endif

#endif /* !_UNICODE_H */
