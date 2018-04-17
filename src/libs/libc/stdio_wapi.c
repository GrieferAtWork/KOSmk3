/* Copyright (ch) 2018 Griefer@Work                                            *
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
#ifndef GUARD_LIBS_LIBC_STDIO_WAPI_C
#define GUARD_LIBS_LIBC_STDIO_WAPI_C 1

#include "libc.h"
#include "stdio.h"
#include "errno.h"
#include "unistd.h"
#include "malloc.h"
#include "exit.h"
#include "tty.h"
#include "system.h"
#include "sched.h"
#include "rtl.h"
#include "unicode.h"
#include <stdio.h>
#include <unicode.h>
#include <syslog.h>
#include <errno.h>
#include <bits/io-file.h>
#include <hybrid/align.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/minmax.h>
#include <hybrid/list/list.h>
#include <kos/sched/mutex.h>
#include <unistd.h>
#include <wctype.h>

#ifdef CONFIG_LIBC_USES_NEW_STDIO
DECL_BEGIN

/* Assert some expectations on the arithmetic
 * behavior of EOF and WEOF by code below. */
STATIC_ASSERT((int)EOF    == (int)WEOF);
STATIC_ASSERT((wint_t)EOF == (wint_t)WEOF);
STATIC_ASSERT(EOF         <= 0x7f);

EXPORT(__SYMw32(getwc_unlocked),libc_fgetwc32_unlocked);
EXPORT(__SYMw32(fgetwc_unlocked),libc_fgetwc32_unlocked);
CRT_W32STDIO_API wint_t LIBCCALL
libc_fgetwc32_unlocked(FILE *__restrict stream) {
 u32 ch32; unsigned int n;
 int ch = libc_fgetc_unlocked(stream);
 if (ch <= 0x7f) return (wint_t)ch;
 /* Decode a UTF-8 sequences. */
 n = utf8_sequence_len[(byte_t)ch];
 if unlikely(!n) return (wint_t)ch;
 ch32 = ch-utf8_offset[n];
 while (n--) {
  ch = libc_fgetc_unlocked(stream);
  if (ch == EOF) return WEOF;
  if ((ch & 0xc0) != 0x80) {
   libc_seterrno(EILSEQ);
   return WEOF;
  }
  ch32 <<= 6;
  ch32 |= (byte_t)ch & 0x3f;
 }
 return ch32;
}

EXPORT(__SYMw16(getwc_unlocked),libc_fgetwc16_unlocked);
EXPORT(__SYMw16(fgetwc_unlocked),libc_fgetwc16_unlocked);
EXPORT(__DSYM(_fgetwc_nolock),libc_fgetwc16_unlocked);
CRT_W16STDIO_API wint_t LIBCCALL
libc_fgetwc16_unlocked(FILE *__restrict stream) {
 wint_t w32;
 /* Check for a pending high surrogate. */
 if (stream->fb_flag & FILE_BUFFER_FUTF16LS) {
  stream->fb_flag &= ~FILE_BUFFER_FUTF16LS;
  return stream->fb_utf16ls;
 }
 w32 = libc_fgetwc32_unlocked(stream);
 /* Convert this utf-32 character to utf-16 */
 if (w32 == WEOF || w32 <= 0xffff) return WEOF;
 /* Must split into a high and a low surrogate. */
 w32 -= 0x10000;
 stream->fb_utf16ls = UNI_SURROGATE_LOW_BEGIN|(w32 & 0x3ff);
 stream->fb_flag |= FILE_BUFFER_FUTF16LS;
 return UNI_SURROGATE_HIGH_BEGIN|(w32 >> 10);
}

EXPORT(__SYMw32(Xgetwc_unlocked),libc_Xfgetwc32_unlocked);
EXPORT(__SYMw32(Xfgetwc_unlocked),libc_Xfgetwc32_unlocked);
CRT_W32STDIO_XAPI wint_t LIBCCALL
libc_Xfgetwc32_unlocked(FILE *__restrict stream) {
 u32 ch32; unsigned int n;
 int ch = libc_Xfgetc_unlocked(stream);
 if (ch <= 0x7f) return (wint_t)ch;
 /* Decode a UTF-8 sequences. */
 n = utf8_sequence_len[(byte_t)ch];
 if unlikely(!n) return (wint_t)ch;
 ch32 = ch-utf8_offset[n];
 while (n--) {
  ch = libc_Xfgetc_unlocked(stream);
  if (ch == EOF) return WEOF;
  if ((ch & 0xc0) != 0x80)
       libc_error_throw(E_UNICODE_ERROR);
  ch32 <<= 6;
  ch32 |= (byte_t)ch & 0x3f;
 }
 return ch32;
}

EXPORT(__SYMw16(Xgetwc_unlocked),libc_Xfgetwc16_unlocked);
EXPORT(__SYMw16(Xfgetwc_unlocked),libc_Xfgetwc16_unlocked);
CRT_W16STDIO_XAPI wint_t LIBCCALL
libc_Xfgetwc16_unlocked(FILE *__restrict stream) {
 wint_t w32;
 /* Check for a pending high surrogate. */
 if (stream->fb_flag & FILE_BUFFER_FUTF16LS) {
  stream->fb_flag &= ~FILE_BUFFER_FUTF16LS;
  return stream->fb_utf16ls;
 }
 w32 = libc_Xfgetwc32_unlocked(stream);
 /* Convert this utf-32 character to utf-16 */
 if (w32 == WEOF || w32 <= 0xffff) return WEOF;
 /* Must split into a high and a low surrogate. */
 w32 -= 0x10000;
 stream->fb_utf16ls = UNI_SURROGATE_LOW_BEGIN|(w32 & 0x3ff);
 stream->fb_flag |= FILE_BUFFER_FUTF16LS;
 return UNI_SURROGATE_HIGH_BEGIN|(w32 >> 10);
}

EXPORT(__SYMw16(getwc),libc_fgetwc16);
EXPORT(__SYMw16(fgetwc),libc_fgetwc16);
CRT_W16STDIO_API wint_t LIBCCALL
libc_fgetwc16(FILE *__restrict stream) {
 wint_t result;
 while (FileBuffer_Lock(stream))
    if (libc_geterrno() != EINTR)
        return WEOF;
 result = libc_fgetwc16_unlocked(stream);
 FileBuffer_Unlock(stream);
 return result;
}

EXPORT(__SYMw32(getwc),libc_fgetwc32);
EXPORT(__SYMw32(fgetwc),libc_fgetwc32);
CRT_W32STDIO_API wint_t LIBCCALL
libc_fgetwc32(FILE *__restrict stream) {
 wint_t result;
 while (FileBuffer_Lock(stream))
    if (libc_geterrno() != EINTR)
        return WEOF;
 result = libc_fgetwc32_unlocked(stream);
 FileBuffer_Unlock(stream);
 return result;
}

EXPORT(__SYMw16(Xgetwc),libc_Xfgetwc16);
EXPORT(__SYMw16(Xfgetwc),libc_Xfgetwc16);
CRT_W16STDIO_XAPI wint_t LIBCCALL
libc_Xfgetwc16(FILE *__restrict stream) {
 wint_t COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(stream);
 LIBC_TRY {
  result = libc_Xfgetwc16_unlocked(stream);
 } LIBC_FINALLY {
  FileBuffer_Unlock(stream);
 }
 return result;
}

EXPORT(__SYMw32(Xgetwc),libc_Xfgetwc32);
EXPORT(__SYMw32(Xfgetwc),libc_Xfgetwc32);
CRT_W32STDIO_XAPI wint_t LIBCCALL
libc_Xfgetwc32(FILE *__restrict stream) {
 wint_t COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(stream);
 LIBC_TRY {
  result = libc_Xfgetwc32_unlocked(stream);
 } LIBC_FINALLY {
  FileBuffer_Unlock(stream);
 }
 return result;
}

EXPORT(__SYMw16(getwchar),libc_getwchar16);
EXPORT(__DSYM(_fgetwchar),libc_getwchar16);
CRT_W16STDIO_API wint_t LIBCCALL libc_getwchar16(void) {
 return libc_fgetwc16(libc_stdin);
}
EXPORT(__SYMw16(getwchar_unlocked),libc_getwchar16_unlocked);
CRT_W16STDIO_API wint_t LIBCCALL libc_getwchar16_unlocked(void) {
 return libc_fgetwc16_unlocked(libc_stdin);
}

EXPORT(__SYMw32(getwchar),libc_getwchar32);
CRT_W32STDIO_API wint_t LIBCCALL libc_getwchar32(void) {
 return libc_fgetwc32(libc_stdin);
}
EXPORT(__SYMw32(getwchar_unlocked),libc_getwchar32_unlocked);
CRT_W32STDIO_API wint_t LIBCCALL libc_getwchar32_unlocked(void) {
 return libc_fgetwc32_unlocked(libc_stdin);
}

EXPORT(__SYMw16(Xgetwchar),libc_Xgetwchar16);
CRT_W16STDIO_XAPI wint_t LIBCCALL libc_Xgetwchar16(void) {
 return libc_Xfgetwc16(libc_stdin);
}
EXPORT(__SYMw16(Xgetwchar_unlocked),libc_Xgetwchar16_unlocked);
CRT_W16STDIO_XAPI wint_t LIBCCALL libc_Xgetwchar16_unlocked(void) {
 return libc_Xfgetwc16_unlocked(libc_stdin);
}

EXPORT(__SYMw32(Xgetwchar),libc_Xgetwchar32);
CRT_W32STDIO_XAPI wint_t LIBCCALL libc_Xgetwchar32(void) {
 return libc_Xfgetwc32(libc_stdin);
}
EXPORT(__SYMw32(Xgetwchar_unlocked),libc_Xgetwchar32_unlocked);
CRT_W32STDIO_XAPI wint_t LIBCCALL libc_Xgetwchar32_unlocked(void) {
 return libc_Xfgetwc32_unlocked(libc_stdin);
}





EXPORT(__SYMw32(putwc),libc_fputwc32);
EXPORT(__SYMw32(fputwc),libc_fputwc32);
CRT_W32STDIO_API wint_t LIBCCALL
libc_fputwc32(char32_t wc, FILE *__restrict stream) {
 char temp[4]; size_t size;
 if (wc < 0x80)
     return (wint_t)libc_fputc((int)(char)(unsigned char)wc,stream);
 if (wc < 0x800) {
  temp[0] = (u8)(0xc0|(wc >> 6));
  temp[1] = (u8)(0x80|(wc & 0x3f));
  size    = 2;
 } else if (wc < 0x10000) {
  temp[0] = (u8)(0xe0|(wc >> 12));
  temp[1] = (u8)(0x80|((wc >> 6) & 0x3f));
  temp[2] = (u8)(0x80|(wc & 0x3f));
  size    = 3;
 } else if likely(wc <= 0x10ffff) {
  temp[0] = (u8)(0xe0|(wc >> 18));
  temp[1] = (u8)(0x80|((wc >> 12) & 0x3f));
  temp[2] = (u8)(0x80|((wc >> 6) & 0x3f));
  temp[3] = (u8)(0x80|(wc & 0x3f));
  size    = 4;
 } else {
  libc_seterrno(EILSEQ);
  return WEOF;
 }
 if (FileBuffer_Write(stream,temp,size) < size)
     return WEOF;
 return (wint_t)wc;
}

EXPORT(__SYMw32(putwc_unlocked),libc_fputwc32_unlocked);
EXPORT(__SYMw32(fputwc_unlocked),libc_fputwc32_unlocked);
CRT_W32STDIO_API wint_t LIBCCALL
libc_fputwc32_unlocked(char32_t wc, FILE *__restrict stream) {
 char temp[4]; size_t size;
 if (wc < 0x80)
     return (wint_t)libc_fputc_unlocked((int)(char)(unsigned char)wc,stream);
 if (wc < 0x800) {
  temp[0] = (u8)(0xc0|(wc >> 6));
  temp[1] = (u8)(0x80|(wc & 0x3f));
  size    = 2;
 } else if (wc < 0x10000) {
  temp[0] = (u8)(0xe0|(wc >> 12));
  temp[1] = (u8)(0x80|((wc >> 6) & 0x3f));
  temp[2] = (u8)(0x80|(wc & 0x3f));
  size    = 3;
 } else if likely(wc <= 0x10ffff) {
  temp[0] = (u8)(0xe0|(wc >> 18));
  temp[1] = (u8)(0x80|((wc >> 12) & 0x3f));
  temp[2] = (u8)(0x80|((wc >> 6) & 0x3f));
  temp[3] = (u8)(0x80|(wc & 0x3f));
  size    = 4;
 } else {
  libc_seterrno(EILSEQ);
  return WEOF;
 }
 if (FileBuffer_WriteUnlocked(stream,temp,size) < size)
     return WEOF;
 return (wint_t)wc;
}

EXPORT(__SYMw32(Xputwc),libc_Xfputwc32);
EXPORT(__SYMw32(Xfputwc),libc_Xfputwc32);
CRT_W32STDIO_XAPI wint_t LIBCCALL
libc_Xfputwc32(char32_t wc, FILE *__restrict stream) {
 char temp[4]; size_t size;
 if (wc < 0x80)
     return (wint_t)libc_Xfputc((int)(char)(unsigned char)wc,stream);
 if (wc < 0x800) {
  temp[0] = (u8)(0xc0|(wc >> 6));
  temp[1] = (u8)(0x80|(wc & 0x3f));
  size    = 2;
 } else if (wc < 0x10000) {
  temp[0] = (u8)(0xe0|(wc >> 12));
  temp[1] = (u8)(0x80|((wc >> 6) & 0x3f));
  temp[2] = (u8)(0x80|(wc & 0x3f));
  size    = 3;
 } else if likely(wc <= 0x10ffff) {
  temp[0] = (u8)(0xe0|(wc >> 18));
  temp[1] = (u8)(0x80|((wc >> 12) & 0x3f));
  temp[2] = (u8)(0x80|((wc >> 6) & 0x3f));
  temp[3] = (u8)(0x80|(wc & 0x3f));
  size    = 4;
 } else {
  libc_error_throw(E_UNICODE_ERROR);
 }
 if (FileBuffer_XWrite(stream,temp,size) < size)
     return WEOF;
 return (wint_t)wc;
}

EXPORT(__SYMw32(Xputwc_unlocked),libc_Xfputwc32_unlocked);
EXPORT(__SYMw32(Xfputwc_unlocked),libc_Xfputwc32_unlocked);
CRT_W32STDIO_XAPI wint_t LIBCCALL
libc_Xfputwc32_unlocked(char32_t wc, FILE *__restrict stream) {
 char temp[4]; size_t size;
 if (wc < 0x80)
     return (wint_t)libc_Xfputc_unlocked((int)(char)(unsigned char)wc,stream);
 if (wc < 0x800) {
  temp[0] = (u8)(0xc0|(wc >> 6));
  temp[1] = (u8)(0x80|(wc & 0x3f));
  size    = 2;
 } else if (wc < 0x10000) {
  temp[0] = (u8)(0xe0|(wc >> 12));
  temp[1] = (u8)(0x80|((wc >> 6) & 0x3f));
  temp[2] = (u8)(0x80|(wc & 0x3f));
  size    = 3;
 } else if likely(wc <= 0x10ffff) {
  temp[0] = (u8)(0xe0|(wc >> 18));
  temp[1] = (u8)(0x80|((wc >> 12) & 0x3f));
  temp[2] = (u8)(0x80|((wc >> 6) & 0x3f));
  temp[3] = (u8)(0x80|(wc & 0x3f));
  size    = 4;
 } else {
  libc_error_throw(E_UNICODE_ERROR);
 }
 if (FileBuffer_XWriteUnlocked(stream,temp,size) < size)
     return WEOF;
 return (wint_t)wc;
}

EXPORT(__SYMw16(putwc_unlocked),libc_fputwc16_unlocked);
EXPORT(__SYMw16(fputwc_unlocked),libc_fputwc16_unlocked);
EXPORT(__DSYM(_fputwc_nolock),libc_fputwc16_unlocked);
CRT_W16STDIO_API wint_t LIBCCALL
libc_fputwc16_unlocked(char16_t wc, FILE *__restrict stream) {
 if (stream->fb_flag & FILE_BUFFER_FUTF16LS) {
  /* Complete a surrogate pair. */
  char32_t c32;
  if (wc < UNI_SURROGATE_HIGH_BEGIN ||
      wc > UNI_SURROGATE_HIGH_END) {
   libc_seterrno(EILSEQ);
   return WEOF;
  }
  stream->fb_flag &= ~FILE_BUFFER_FUTF16LS;
  c32   = stream->fb_utf16ls;
  c32  -= UNI_SURROGATE_LOW_BEGIN;
  c32  |= (wc - UNI_SURROGATE_HIGH_BEGIN) << 10;
  if (libc_Xfputwc32_unlocked(c32,stream) == WEOF)
      return WEOF;
  return wc;
 }
 if (wc >= UNI_SURROGATE_HIGH_BEGIN &&
     wc <= UNI_SURROGATE_HIGH_END) {
  /* The given character is a high surrogate. */
  stream->fb_utf16ls = wc;
  stream->fb_flag   |= FILE_BUFFER_FUTF16LS;
  return wc;
 }
 /* The given character maps to UTF-32 without modifications. */
 return libc_fputwc32_unlocked((char32_t)wc,stream);
}

EXPORT(__SYMw16(Xputwc_unlocked),libc_Xfputwc16_unlocked);
EXPORT(__SYMw16(Xfputwc_unlocked),libc_Xfputwc16_unlocked);
CRT_W16STDIO_XAPI wint_t LIBCCALL
libc_Xfputwc16_unlocked(char16_t wc, FILE *__restrict stream) {
 if (stream->fb_flag & FILE_BUFFER_FUTF16LS) {
  /* Complete a surrogate pair. */
  char32_t c32;
  if (wc < UNI_SURROGATE_HIGH_BEGIN ||
      wc > UNI_SURROGATE_HIGH_END)
      libc_error_throw(E_UNICODE_ERROR);
  stream->fb_flag &= ~FILE_BUFFER_FUTF16LS;
  c32   = stream->fb_utf16ls;
  c32  -= UNI_SURROGATE_LOW_BEGIN;
  c32  |= (wc - UNI_SURROGATE_HIGH_BEGIN) << 10;
  if (libc_Xfputwc32_unlocked(c32,stream) == WEOF)
      return WEOF;
  return wc;
 }
 if (wc >= UNI_SURROGATE_HIGH_BEGIN &&
     wc <= UNI_SURROGATE_HIGH_END) {
  /* The given character is a high surrogate. */
  stream->fb_utf16ls = wc;
  stream->fb_flag   |= FILE_BUFFER_FUTF16LS;
  return wc;
 }
 /* The given character maps to UTF-32 without modifications. */
 return libc_Xfputwc32_unlocked((char32_t)wc,stream);
}


EXPORT(__SYMw16(putwc),libc_fputwc16);
EXPORT(__SYMw16(fputwc),libc_fputwc16);
CRT_W16STDIO_API wint_t LIBCCALL
libc_fputwc16(char16_t wc, FILE *__restrict stream) {
 wint_t result;
 while (FileBuffer_Lock(stream))
    if (libc_geterrno() != EINTR)
        return WEOF;
 result = libc_fputwc16_unlocked(wc,stream);
 FileBuffer_Unlock(stream);
 return result;
}

EXPORT(__SYMw16(Xputwc),libc_Xfputwc16);
EXPORT(__SYMw16(Xfputwc),libc_Xfputwc16);
CRT_W16STDIO_XAPI wint_t LIBCCALL
libc_Xfputwc16(char16_t wc, FILE *__restrict stream) {
 wint_t COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(stream);
 LIBC_TRY {
  result = libc_Xfputwc16_unlocked(wc,stream);
 } LIBC_FINALLY {
  FileBuffer_Unlock(stream);
 }
 return result;
}


EXPORT(__SYMw32(putwchar),libc_putwchar32);
CRT_W32STDIO_API wint_t LIBCCALL libc_putwchar32(char32_t wc) {
 return libc_fputwc32(wc,libc_stdout);
}
EXPORT(__SYMw32(putwchar_unlocked),libc_putwchar32_unlocked);
CRT_W32STDIO_API wint_t LIBCCALL libc_putwchar32_unlocked(char32_t wc) {
 return libc_fputwc32_unlocked(wc,libc_stdout);
}

EXPORT(__SYMw32(Xputwchar),libc_Xputwchar32);
CRT_W32STDIO_XAPI wint_t LIBCCALL libc_Xputwchar32(char32_t wc) {
 return libc_Xfputwc32(wc,libc_stdout);
}
EXPORT(__SYMw32(Xputwchar_unlocked),libc_Xputwchar32_unlocked);
CRT_W32STDIO_XAPI wint_t LIBCCALL libc_Xputwchar32_unlocked(char32_t wc) {
 return libc_Xfputwc32_unlocked(wc,libc_stdout);
}


EXPORT(__SYMw16(putwchar),libc_putwchar16);
EXPORT(__DSYM(_fputwchar),libc_putwchar16);
CRT_W16STDIO_API wint_t LIBCCALL libc_putwchar16(char16_t wc) {
 return libc_fputwc16(wc,libc_stdout);
}
EXPORT(__SYMw16(putwchar_unlocked),libc_putwchar16_unlocked);
CRT_W16STDIO_API wint_t LIBCCALL libc_putwchar16_unlocked(char16_t wc) {
 return libc_fputwc16_unlocked(wc,libc_stdout);
}

EXPORT(__SYMw16(Xputwchar),libc_Xputwchar16);
CRT_W16STDIO_XAPI wint_t LIBCCALL libc_Xputwchar16(char16_t wc) {
 return libc_Xfputwc16(wc,libc_stdout);
}
EXPORT(__SYMw16(Xputwchar_unlocked),libc_Xputwchar16_unlocked);
CRT_W16STDIO_XAPI wint_t LIBCCALL libc_Xputwchar16_unlocked(char16_t wc) {
 return libc_Xfputwc16_unlocked(wc,libc_stdout);
}




EXPORT(__SYMw16(fgetws_unlocked),libc_fgetws16_unlocked);
CRT_W16STDIO_API char16_t *LIBCCALL
libc_fgetws16_unlocked(char16_t *__restrict s, size_t n,
                       FILE *__restrict self) {
 char16_t *dst = s;
 /* Subtract one from the buffer to ensure
  * space for a trailing NUL-char16_tacter. */
 if unlikely(!n--) return s;
 for (; n; --n) {
  wint_t ch = libc_fgetwc16_unlocked(self);
  /* Stop on EOF */
  if (ch == WEOF) {
   if (dst == s) return NULL; /* EOF */
   break;
  }
  *dst++ = (char16_t)ch;
  /* Stop on linefeed. */
  if (ch == '\n') break;
  if (ch == '\r') {
   int ch2 = libc_fgetc_unlocked(self);
   if (ch2 != '\n' && ch2 != EOF)
       libc_ungetc_unlocked(ch2,self);
   break;
  }
 }
 /* Terminate the given buffer. */
 *dst = 0;
 return s;
}

EXPORT(Xfgetws16_unlocked,libc_Xfgetws16_unlocked);
CRT_W16STDIO_XAPI char16_t *LIBCCALL
libc_Xfgetws16_unlocked(char16_t *__restrict s, size_t n,
                        FILE *__restrict self) {
 char16_t *dst = s;
 /* Subtract one from the buffer to ensure
  * space for a trailing NUL-char16_tacter. */
 if unlikely(!n--) return s;
 for (; n; --n) {
  wint_t ch = libc_Xfgetwc16_unlocked(self);
  /* Stop on EOF */
  if (ch == WEOF) {
   if (dst == s) return NULL; /* EOF */
   break;
  }
  *dst++ = (char16_t)ch;
  /* Stop on linefeed. */
  if (ch == '\n') break;
  if (ch == '\r') {
   int ch2 = libc_Xfgetc_unlocked(self);
   if (ch2 != '\n' && ch2 != EOF)
       libc_Xungetc_unlocked(ch2,self);
   break;
  }
 }
 /* Terminate the given buffer. */
 *dst = 0;
 return s;
}

EXPORT(__SYMw16(fgetws),libc_fgetws16);
CRT_W16STDIO_API char16_t *LIBCCALL
libc_fgetws16(char16_t *__restrict s, size_t n,
              FILE *__restrict self) {
 char16_t *result;
 while (FileBuffer_Lock(self))
    if (libc_geterrno() != EINTR)
        return NULL;
 result = libc_fgetws16_unlocked(s,n,self);
 FileBuffer_Unlock(self);
 return result;
}

EXPORT(__SYMw16(Xfgetws),libc_Xfgetws16);
CRT_W16STDIO_XAPI char16_t *LIBCCALL
libc_Xfgetws16(char16_t *__restrict s, size_t n,
               FILE *__restrict self) {
 char16_t *COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(self);
 LIBC_TRY {
  result = libc_Xfgetws16_unlocked(s,n,self);
 } LIBC_FINALLY {
  FileBuffer_Unlock(self);
 }
 return result;
}



EXPORT(__SYMw32(fgetws_unlocked),libc_fgetws32_unlocked);
CRT_W32STDIO_API char32_t *LIBCCALL
libc_fgetws32_unlocked(char32_t *__restrict s, size_t n,
                       FILE *__restrict self) {
 char32_t *dst = s;
 /* Subtract one from the buffer to ensure
  * space for a trailing NUL-char32_tacter. */
 if unlikely(!n--) return s;
 for (; n; --n) {
  wint_t ch = libc_fgetwc32_unlocked(self);
  /* Stop on EOF */
  if (ch == WEOF) {
   if (dst == s) return NULL; /* EOF */
   break;
  }
  *dst++ = (char32_t)ch;
  /* Stop on linefeed. */
  if (ch == '\n') break;
  if (ch == '\r') {
   int ch2 = libc_fgetc_unlocked(self);
   if (ch2 != '\n' && ch2 != EOF)
       libc_ungetc_unlocked(ch2,self);
   break;
  }
 }
 /* Terminate the given buffer. */
 *dst = 0;
 return s;
}

EXPORT(Xfgetws32_unlocked,libc_Xfgetws32_unlocked);
CRT_W32STDIO_XAPI char32_t *LIBCCALL
libc_Xfgetws32_unlocked(char32_t *__restrict s, size_t n,
                        FILE *__restrict self) {
 char32_t *dst = s;
 /* Subtract one from the buffer to ensure
  * space for a trailing NUL-char32_tacter. */
 if unlikely(!n--) return s;
 for (; n; --n) {
  wint_t ch = libc_Xfgetwc32_unlocked(self);
  /* Stop on EOF */
  if (ch == WEOF) {
   if (dst == s) return NULL; /* EOF */
   break;
  }
  *dst++ = (char32_t)ch;
  /* Stop on linefeed. */
  if (ch == '\n') break;
  if (ch == '\r') {
   int ch2 = libc_Xfgetc_unlocked(self);
   if (ch2 != '\n' && ch2 != EOF)
       libc_Xungetc_unlocked(ch2,self);
   break;
  }
 }
 /* Terminate the given buffer. */
 *dst = 0;
 return s;
}

EXPORT(__SYMw32(fgetws),libc_fgetws32);
CRT_W32STDIO_API char32_t *LIBCCALL
libc_fgetws32(char32_t *__restrict s, size_t n,
              FILE *__restrict self) {
 char32_t *result;
 while (FileBuffer_Lock(self))
    if (libc_geterrno() != EINTR)
        return NULL;
 result = libc_fgetws32_unlocked(s,n,self);
 FileBuffer_Unlock(self);
 return result;
}

EXPORT(__SYMw32(Xfgetws),libc_Xfgetws32);
CRT_W32STDIO_XAPI char32_t *LIBCCALL
libc_Xfgetws32(char32_t *__restrict s, size_t n,
               FILE *__restrict self) {
 char32_t *COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(self);
 LIBC_TRY {
  result = libc_Xfgetws32_unlocked(s,n,self);
 } LIBC_FINALLY {
  FileBuffer_Unlock(self);
 }
 return result;
}



#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
EXPORT(__SYMw16(fgetws),libc_fgetws16);
EXPORT(__SYMw32(fgetws),libc_fgetws32);
EXPORT(__SYMw16(Xfgetws),libc_Xfgetws16);
EXPORT(__SYMw32(Xfgetws),libc_Xfgetws32);
EXPORT(__SYMw16(fgetws_unlocked),libc_fgetws16_unlocked);
EXPORT(__SYMw32(fgetws_unlocked),libc_fgetws32_unlocked);
EXPORT(__SYMw16(Xfgetws_unlocked),libc_Xfgetws16_unlocked);
EXPORT(__SYMw32(Xfgetws_unlocked),libc_Xfgetws32_unlocked);
DEFINE_INTERN_ALIAS(libc_fgetws16_int,libc_fgetws16);
DEFINE_INTERN_ALIAS(libc_fgetws32_int,libc_fgetws32);
DEFINE_INTERN_ALIAS(libc_Xfgetws16_int,libc_Xfgetws16);
DEFINE_INTERN_ALIAS(libc_Xfgetws32_int,libc_Xfgetws32);
DEFINE_INTERN_ALIAS(libc_fgetws16_int_unlocked,libc_fgetws16_unlocked);
DEFINE_INTERN_ALIAS(libc_fgetws32_int_unlocked,libc_fgetws32_unlocked);
DEFINE_INTERN_ALIAS(libc_Xfgetws16_int_unlocked,libc_Xfgetws16_unlocked);
DEFINE_INTERN_ALIAS(libc_Xfgetws32_int_unlocked,libc_Xfgetws32_unlocked);
#else
EXPORT(__SYMw16(fgetws),libc_fgetws16_int);
EXPORT(__SYMw32(fgetws),libc_fgetws32_int);
EXPORT(__SYMw16(Xfgetws),libc_Xfgetws16_int);
EXPORT(__SYMw32(Xfgetws),libc_Xfgetws32_int);
EXPORT(__SYMw16(fgetws_unlocked),libc_fgetws16_int_unlocked);
EXPORT(__SYMw32(fgetws_unlocked),libc_fgetws32_int_unlocked);
EXPORT(__SYMw16(Xfgetws_unlocked),libc_Xfgetws16_int_unlocked);
EXPORT(__SYMw32(Xfgetws_unlocked),libc_Xfgetws32_int_unlocked);
EXPORT(__SYMw16(fgetws_sz),libc_fgetws16);
EXPORT(__SYMw32(fgetws_sz),libc_fgetws32);
EXPORT(__SYMw16(Xfgetws_sz),libc_Xfgetws16);
EXPORT(__SYMw32(Xfgetws_sz),libc_Xfgetws32);
EXPORT(__SYMw16(fgetws_sz_unlocked),libc_fgetws16_unlocked);
EXPORT(__SYMw32(fgetws_sz_unlocked),libc_fgetws32_unlocked);
EXPORT(__SYMw16(Xfgetws_sz_unlocked),libc_Xfgetws16_unlocked);
EXPORT(__SYMw32(Xfgetws_sz_unlocked),libc_Xfgetws32_unlocked);

CRT_W16STDIO_API char16_t *LIBCCALL
libc_fgetws16_int(char16_t *__restrict s,
                  unsigned int n,
                  FILE *__restrict self) {
 return libc_fgetws16(s,n,self);
}
CRT_W16STDIO_XAPI char16_t *LIBCCALL
libc_Xfgetws16_int(char16_t *__restrict s,
                   unsigned int n,
                   FILE *__restrict self) {
 return libc_Xfgetws16(s,n,self);
}
CRT_W16STDIO_API char16_t *LIBCCALL
libc_fgetws16_int_unlocked(char16_t *__restrict s,
                           unsigned int n,
                           FILE *__restrict self) {
 return libc_fgetws16_unlocked(s,n,self);
}
CRT_W16STDIO_XAPI char16_t *LIBCCALL
libc_Xfgetws16_int_unlocked(char16_t *__restrict s,
                            unsigned int n,
                            FILE *__restrict self) {
 return libc_Xfgetws16_unlocked(s,n,self);
}

CRT_W32STDIO_API char32_t *LIBCCALL
libc_fgetws32_int(char32_t *__restrict s,
                  unsigned int n,
                  FILE *__restrict self) {
 return libc_fgetws32(s,n,self);
}
CRT_W32STDIO_XAPI char32_t *LIBCCALL
libc_Xfgetws32_int(char32_t *__restrict s,
                   unsigned int n,
                   FILE *__restrict self) {
 return libc_Xfgetws32(s,n,self);
}
CRT_W32STDIO_API char32_t *LIBCCALL
libc_fgetws32_int_unlocked(char32_t *__restrict s,
                           unsigned int n,
                           FILE *__restrict self) {
 return libc_fgetws32_unlocked(s,n,self);
}
CRT_W32STDIO_XAPI char32_t *LIBCCALL
libc_Xfgetws32_int_unlocked(char32_t *__restrict s,
                            unsigned int n,
                            FILE *__restrict self) {
 return libc_Xfgetws32_unlocked(s,n,self);
}
#endif

EXPORT(__SYMw32(fputws_unlocked),libc_fputws32_unlocked);
CRT_W32STDIO_API ssize_t LIBCCALL
libc_fputws32_unlocked(char32_t const *__restrict ws,
                       FILE *__restrict stream) {
 size_t count,temp,result = 0;
 size_t ws_length = (size_t)-1;
 mbstate_t state = MBSTATE_INIT;
 while (*ws) {
  char buffer[256];
  count = libc_utf32to8((char32_t *)&ws,
                        (size_t)&ws_length,
                         buffer,sizeof(buffer),
                        &state,
                         UNICODE_F_NOZEROTERM|
                         UNICODE_F_STOPONNUL|
                         UNICODE_F_UPDATESRC|
                         UNICODE_F_SETERRNO);
  if unlikely(!count) break;
  if unlikely(count == UNICODE_ERROR) return EOF;
  temp = FileBuffer_WriteUnlocked(stream,buffer,count);
  if unlikely(!temp) return EOF;
 }
 return result;
}

EXPORT(__SYMw32(Xfputws_unlocked),libc_Xfputws32_unlocked);
CRT_W32STDIO_XAPI size_t LIBCCALL
libc_Xfputws32_unlocked(char32_t const *__restrict ws,
                        FILE *__restrict stream) {
 size_t count,temp,result = 0;
 size_t ws_length = (size_t)-1;
 mbstate_t state = MBSTATE_INIT;
 while (*ws) {
  char buffer[256];
  count = libc_Xutf32to8((char32_t *)&ws,
                         (size_t)&ws_length,
                          buffer,sizeof(buffer),
                         &state,
                          UNICODE_F_NOZEROTERM|
                          UNICODE_F_STOPONNUL|
                          UNICODE_F_UPDATESRC|
                          UNICODE_F_SETERRNO);
  if unlikely(!count) break;
  temp = FileBuffer_XWriteUnlocked(stream,buffer,count);
  if unlikely(!temp) return EOF;
 }
 return result;
}

EXPORT(__SYMw16(fputws_unlocked),libc_fputws16_unlocked);
CRT_W16STDIO_API ssize_t LIBCCALL
libc_fputws16_unlocked(char16_t const *__restrict ws,
                       FILE *__restrict stream) {
 size_t count,temp,result = 0;
 size_t ws_length = (size_t)-1;
 mbstate_t state = MBSTATE_INIT;
 /* Copy a dangling surrogate. */
 if (stream->fb_flag & FILE_BUFFER_FUTF16LS) {
  state.__state.__count = 0x80;
  state.__word16 = stream->fb_utf16ls;
  stream->fb_flag &= ~FILE_BUFFER_FUTF16LS;
 }
 while (*ws) {
  char buffer[256];
  count = libc_utf16to8((char16_t *)&ws,
                        (size_t)&ws_length,
                         buffer,sizeof(buffer),
                        &state,
                         UNICODE_F_NOZEROTERM|
                         UNICODE_F_STOPONNUL|
                         UNICODE_F_UPDATESRC|
                         UNICODE_F_SETERRNO);
  if unlikely(!count) break;
  if unlikely(count == UNICODE_ERROR) return EOF;
  temp = FileBuffer_WriteUnlocked(stream,buffer,count);
  if unlikely(!temp) { result = EOF; break; }
 }
 /* Copy a dangling surrogate. */
 if (state.__state.__count & 0x80) {
  stream->fb_utf16ls = state.__word16;
  stream->fb_flag |= ~FILE_BUFFER_FUTF16LS;
 }
 return result;
}

EXPORT(__SYMw16(Xfputws_unlocked),libc_Xfputws16_unlocked);
CRT_W16STDIO_XAPI size_t LIBCCALL
libc_Xfputws16_unlocked(char16_t const *__restrict ws,
                        FILE *__restrict stream) {
 size_t count,temp,result = 0;
 size_t ws_length = (size_t)-1;
 mbstate_t state = MBSTATE_INIT;
 if (stream->fb_flag & FILE_BUFFER_FUTF16LS) {
  state.__state.__count = 0x80;
  state.__word16 = stream->fb_utf16ls;
  stream->fb_flag &= ~FILE_BUFFER_FUTF16LS;
 }
 while (*ws) {
  char buffer[256];
  count = libc_Xutf16to8((char16_t *)&ws,
                         (size_t)&ws_length,
                          buffer,sizeof(buffer),
                         &state,
                          UNICODE_F_NOZEROTERM|
                          UNICODE_F_STOPONNUL|
                          UNICODE_F_UPDATESRC|
                          UNICODE_F_SETERRNO);
  if unlikely(!count) break;
  temp = FileBuffer_XWriteUnlocked(stream,buffer,count);
  if unlikely(!temp) return EOF;
 }
 return result;
}

EXPORT(__SYMw32(fputws),libc_fputws32);
CRT_W32STDIO_API ssize_t LIBCCALL
libc_fputws32(char32_t const *__restrict ws, FILE *__restrict stream) {
 ssize_t result;
 while (FileBuffer_Lock(stream))
    if (libc_geterrno() != EINTR)
        return -1;
 result = libc_fputws32_unlocked(ws,stream);
 FileBuffer_Unlock(stream);
 return result;
}

EXPORT(__SYMw32(Xfputws),libc_Xfputws32);
CRT_W32STDIO_XAPI size_t LIBCCALL
libc_Xfputws32(char32_t const *__restrict ws, FILE *__restrict stream) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(stream);
 LIBC_TRY {
  result = libc_Xfputws32_unlocked(ws,stream);
 } LIBC_FINALLY {
  FileBuffer_Unlock(stream);
 }
 return result;
}

EXPORT(__SYMw16(fputws),libc_fputws16);
CRT_W16STDIO_API ssize_t LIBCCALL
libc_fputws16(char16_t const *__restrict ws, FILE *__restrict stream) {
 ssize_t result;
 while (FileBuffer_Lock(stream))
    if (libc_geterrno() != EINTR)
        return -1;
 result = libc_fputws16_unlocked(ws,stream);
 FileBuffer_Unlock(stream);
 return result;
}

EXPORT(__SYMw16(Xfputws),libc_Xfputws16);
CRT_W16STDIO_XAPI size_t LIBCCALL
libc_Xfputws16(char16_t const *__restrict ws, FILE *__restrict stream) {
 size_t COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(stream);
 LIBC_TRY {
  result = libc_Xfputws16_unlocked(ws,stream);
 } LIBC_FINALLY {
  FileBuffer_Unlock(stream);
 }
 return result;
}

EXPORT(__SYMw32(ungetwc_unlocked),libc_ungetwc32_unlocked);
CRT_W32STDIO_API wint_t LIBCCALL
libc_ungetwc32_unlocked(wint_t wc, FILE *__restrict stream) {
 char temp[4]; size_t size;
 if (wc < 0x80)
     return (wint_t)libc_ungetc_unlocked((int)(char)(unsigned char)wc,stream);
 if (wc < 0x800) {
  temp[0] = (u8)(0xc0|(wc >> 6));
  temp[1] = (u8)(0x80|(wc & 0x3f));
  size    = 2;
 } else if (wc < 0x10000) {
  temp[0] = (u8)(0xe0|(wc >> 12));
  temp[1] = (u8)(0x80|((wc >> 6) & 0x3f));
  temp[2] = (u8)(0x80|(wc & 0x3f));
  size    = 3;
 } else if likely(wc <= 0x10ffff) {
  temp[0] = (u8)(0xe0|(wc >> 18));
  temp[1] = (u8)(0x80|((wc >> 12) & 0x3f));
  temp[2] = (u8)(0x80|((wc >> 6) & 0x3f));
  temp[3] = (u8)(0x80|(wc & 0x3f));
  size    = 4;
 } else {
  libc_seterrno(EILSEQ);
  return WEOF;
 }
 while (size--) {
  if (libc_ungetc_unlocked((int)temp[size],stream) == EOF)
      return WEOF;
 }
 return (wint_t)wc;
}

EXPORT(__SYMw32(ungetwc),libc_ungetwc32);
CRT_W32STDIO_API wint_t LIBCCALL
libc_ungetwc32(wint_t wc, FILE *__restrict stream) {
 wint_t result;
 while (FileBuffer_Lock(stream))
    if (libc_geterrno() != EINTR)
        return -1;
 result = libc_ungetwc32_unlocked(wc,stream);
 FileBuffer_Unlock(stream);
 return result;
}

EXPORT(__SYMw32(Xungetwc_unlocked),libc_Xungetwc32_unlocked);
CRT_W32STDIO_XAPI wint_t LIBCCALL
libc_Xungetwc32_unlocked(wint_t wc, FILE *__restrict stream) {
 char temp[4]; size_t size;
 if (wc < 0x80)
     return (wint_t)libc_Xungetc_unlocked((int)(char)(unsigned char)wc,stream);
 if (wc < 0x800) {
  temp[0] = (u8)(0xc0|(wc >> 6));
  temp[1] = (u8)(0x80|(wc & 0x3f));
  size    = 2;
 } else if (wc < 0x10000) {
  temp[0] = (u8)(0xe0|(wc >> 12));
  temp[1] = (u8)(0x80|((wc >> 6) & 0x3f));
  temp[2] = (u8)(0x80|(wc & 0x3f));
  size    = 3;
 } else if likely(wc <= 0x10ffff) {
  temp[0] = (u8)(0xe0|(wc >> 18));
  temp[1] = (u8)(0x80|((wc >> 12) & 0x3f));
  temp[2] = (u8)(0x80|((wc >> 6) & 0x3f));
  temp[3] = (u8)(0x80|(wc & 0x3f));
  size    = 4;
 } else {
  libc_error_throw(E_UNICODE_ERROR);
 }
 while (size--) {
  if (libc_Xungetc_unlocked((int)temp[size],stream) == EOF)
      return WEOF;
 }
 return (wint_t)wc;
}

EXPORT(__SYMw32(Xungetwc),libc_Xungetwc32);
CRT_W32STDIO_XAPI wint_t LIBCCALL
libc_Xungetwc32(wint_t wc, FILE *__restrict stream) {
 wint_t COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(stream);
 LIBC_TRY {
  result = libc_Xungetwc32_unlocked(wc,stream);
 } LIBC_FINALLY {
  FileBuffer_Unlock(stream);
 }
 return result;
}

EXPORT(__SYMw16(ungetwc_unlocked),libc_ungetwc16_unlocked);
EXPORT(__DSYM(_ungetwc_nolock),libc_ungetwc16_unlocked);
CRT_W16STDIO_API wint_t LIBCCALL
libc_ungetwc16_unlocked(wint_t wc, FILE *__restrict stream) {
 if (stream->fb_flag & FILE_BUFFER_FUTF16LS) {
  wint_t c32;
  /* Combine surrogates. */
  if (wc < UNI_SURROGATE_HIGH_BEGIN ||
      wc > UNI_SURROGATE_HIGH_END) {
   libc_seterrno(EILSEQ);
   return WEOF;
  }
  stream->fb_flag &= ~FILE_BUFFER_FUTF16LS;
  c32  = stream->fb_utf16ls;
  c32 -= UNI_SURROGATE_LOW_BEGIN;
  c32 |= (wc - UNI_SURROGATE_HIGH_BEGIN) << 10;
  if (libc_ungetwc32_unlocked(c32,stream) == WEOF)
      return WEOF;
  return wc;
 }
 /* Check for surrogate. */
 if (wc >= UNI_SURROGATE_LOW_BEGIN &&
     wc <= UNI_SURROGATE_LOW_END) {
  stream->fb_flag   |= FILE_BUFFER_FUTF16LS;
  stream->fb_utf16ls = wc;
  return wc;
 }
 /* Character directly maps to UTF-32 */
 return libc_ungetwc32_unlocked(wc,stream);
}

EXPORT(__SYMw16(ungetwc),libc_ungetwc16);
CRT_W16STDIO_API wint_t LIBCCALL
libc_ungetwc16(wint_t wc, FILE *__restrict stream) {
 wint_t result;
 while (FileBuffer_Lock(stream))
    if (libc_geterrno() != EINTR)
        return -1;
 result = libc_ungetwc16_unlocked(wc,stream);
 FileBuffer_Unlock(stream);
 return result;
}

EXPORT(__SYMw16(Xungetwc_unlocked),libc_Xungetwc16_unlocked);
CRT_W16STDIO_API wint_t LIBCCALL
libc_Xungetwc16_unlocked(wint_t wc, FILE *__restrict stream) {
 if (stream->fb_flag & FILE_BUFFER_FUTF16LS) {
  wint_t c32;
  /* Combine surrogates. */
  if (wc < UNI_SURROGATE_HIGH_BEGIN ||
      wc > UNI_SURROGATE_HIGH_END)
      libc_error_throw(E_UNICODE_ERROR);
  stream->fb_flag &= ~FILE_BUFFER_FUTF16LS;
  c32  = stream->fb_utf16ls;
  c32 -= UNI_SURROGATE_LOW_BEGIN;
  c32 |= (wc - UNI_SURROGATE_HIGH_BEGIN) << 10;
  if (libc_Xungetwc32_unlocked(c32,stream) == WEOF)
      return WEOF;
  return wc;
 }
 /* Check for surrogate. */
 if (wc >= UNI_SURROGATE_LOW_BEGIN &&
     wc <= UNI_SURROGATE_LOW_END) {
  stream->fb_flag   |= FILE_BUFFER_FUTF16LS;
  stream->fb_utf16ls = wc;
  return wc;
 }
 /* Character directly maps to UTF-32 */
 return libc_Xungetwc32_unlocked(wc,stream);
}

EXPORT(__SYMw16(Xungetwc),libc_Xungetwc16);
CRT_W16STDIO_XAPI wint_t LIBCCALL
libc_Xungetwc16(wint_t wc, FILE *__restrict stream) {
 wint_t COMPILER_IGNORE_UNINITIALIZED(result);
 FileBuffer_XLock(stream);
 LIBC_TRY {
  result = libc_Xungetwc16_unlocked(wc,stream);
 } LIBC_FINALLY {
  FileBuffer_Unlock(stream);
 }
 return result;
}

EXPORT(fwide,libc_fwide);
CRT_W32STDIO_API int LIBCCALL
libc_fwide(FILE *__restrict fp, int mode) {
 (void)fp;
 (void)mode;
 /* ??? */
 return 0;
}



EXPORT(__SYMw16(_getws),libc_getws16);
CRT_W16STDIO_API char16_t *LIBCCALL
libc_getws16(char16_t *__restrict buf) {
 return libc_fgetws16(buf,(size_t)-1,libc_stdin);
}

EXPORT(__SYMw32(getws),libc_getws32);
CRT_W32STDIO_API char32_t *LIBCCALL
libc_getws32(char32_t *__restrict buf) {
 return libc_fgetws32(buf,(size_t)-1,libc_stdin);
}

EXPORT(__SYMw16(_getws_s),libc_getws16_s);
CRT_W16STDIO_API char16_t *LIBCCALL
libc_getws16_s(char16_t *__restrict buf, size_t buflen) {
 return libc_fgetws16(buf,buflen,libc_stdin);
}

EXPORT(__SYMw32(getws_s),libc_getws32_s);
CRT_W32STDIO_API char32_t *LIBCCALL
libc_getws32_s(char32_t *__restrict buf, size_t buflen) {
 return libc_fgetws32(buf,buflen,libc_stdin);
}


/* Put wide string functions */
EXPORT(__SYMw16(_putws),libc_putws16);
INTDEF ssize_t LIBCCALL
libc_putws16(char16_t const *__restrict ws) {
 ssize_t result;
 while (FileBuffer_Lock(libc_stdout))
    if (libc_geterrno() != EINTR)
        return -1;
 result = libc_fputws16_unlocked(ws,libc_stdout);
 if (libc_fputwc16_unlocked('\n',libc_stdout) == WEOF)
     result = EOF-1;
 ++result;
 FileBuffer_Unlock(libc_stdout);
 return result;
}
EXPORT(__SYMw32(putws),libc_putws32);
INTDEF ssize_t LIBCCALL
libc_putws32(char32_t const *__restrict ws) {
 ssize_t result;
 while (FileBuffer_Lock(libc_stdout))
    if (libc_geterrno() != EINTR)
        return -1;
 result = libc_fputws32_unlocked(ws,libc_stdout);
 if (libc_fputwc32_unlocked('\n',libc_stdout) == WEOF)
     result = EOF-1;
 ++result;
 FileBuffer_Unlock(libc_stdout);
 return result;
}



DECL_END
#endif /* CONFIG_LIBC_USES_NEW_STDIO */

#endif /* !GUARD_LIBS_LIBC_STDIO_WAPI_C */
