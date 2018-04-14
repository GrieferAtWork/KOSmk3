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
#ifndef GUARD_LIBS_LIBC_UNICODE_C
#define GUARD_LIBS_LIBC_UNICODE_C 1

#include "libc.h"
#include "unicode.h"
#include "errno.h"
#include "malloc.h"

#include <unicode.h>
#include <errno.h>
#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/minmax.h>
#include <uchar.h>
#include <stdbool.h>
#include <assert.h>

DECL_BEGIN

#if 1

PRIVATE u8 const utf8_sequence_len[256] = {
    /* ASCII */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x00-0x0f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x10-0x1f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x20-0x2f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x30-0x3f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x40-0x4f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x50-0x5f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x60-0x6f */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x70-0x7f */
    /* Unicode follow-up word (`0b10??????'). */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x80-0x8f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x90-0x9f */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xa0-0xaf */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xb0-0xbf */
    /* `0b110?????' */
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* 0xc0-0xcf */
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* 0xd0-0xdf */
    /* `0b1110????' */
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, /* 0xe0-0xef */
    /* `0b11110???' */
    4,4,4,4,4,4,4,4,
    /* Unused (reserved for future) */
    0,0,0,0,0,0,0,0
};
PRIVATE u8 const utf8_offset[4] = {
    0x00,
    0xc0,
    0xe0,
    0xf0
};

#define UNI_SURROGATE_HIGH_BEGIN 0xd800
#define UNI_SURROGATE_HIGH_END   0xdbff
#define UNI_SURROGATE_LOW_BEGIN  0xdc00
#define UNI_SURROGATE_LOW_END    0xdfff


INTERN size_t LIBCCALL
libc_utf8to32(char const *__restrict utf8, size_t utf8len,
              char32_t *__restrict utf32, size_t buflen32,
              mbstate_t *__restrict state, u32 mode) {
 u8 *src,*src_end,temp; mbstate_t orig_state;
 size_t result = 0; u32 ch;
 if (mode & UNICODE_F_UPDATESRC) {
  src_end = (src = *(u8 **)utf8)+*(size_t *)utf8len;
 } else {
  src_end = (src = (u8 *)utf8)+utf8len;
 }
 orig_state = *state;
 if (orig_state.__state.__count & 0x7f) {
  unsigned int missing;
  ch = __MBSTATE_GETWORD24(&orig_state);
  missing = 4-orig_state.__state.__count;
  assert(missing >= 1 && missing <= 3);
  do {
   if (src == src_end ||
       /* Special case: fill the shift state as much as
        *               possible, but return ZERO(0). */
      (missing <= 1 && !buflen32 &&
      (mode&UNICODE_F_UPDATESRC))) {
    state->__word          = ch;
    state->__state.__count = 4-missing;
    if (mode & UNICODE_F_ALWAYSZEROTERM) {
     result = 1;
     if (buflen32)
         ((u32 *)utf32)[0] = 0;
    }
    goto done;
   }
   temp = *src;
   /* Validate secondary sequence character. */
   if ((temp & 0xc0) != 0x80) {
    state->__word = 0;
handle_error:
    if (mode & UNICODE_F_NOFAIL) { ch = UNICODE_REPLACEMENT; goto put_char; }
    if (mode & UNICODE_F_SETERRNO) {
     if (mode & __UNICODE_F_EXCEPT)
         libc_error_throw(E_UNICODE_ERROR);
     libc_seterrno(EILSEQ);
    }
    return UNICODE_ERROR;
   }
   ch <<= 6;
   ch  |= temp;
   ++src;
  } while (--missing);
  /* Clear the MB state. */
  state->__word = 0;
  /* Write the dangling character partially constructed from the MB state. */
  goto put_char;
 }
 while (src < src_end) {
  size_t missing,seqlen;
  ch = *src++;
  seqlen = utf8_sequence_len[ch];
  if unlikely(!seqlen) goto handle_error;
  if (!ch && (mode & UNICODE_F_STOPONNUL)) { --src; break; }
  missing = seqlen-1;
  ch -= utf8_offset[missing];

  /* Read in missing characters. */
  for (; missing; --missing) {
   if (src >= src_end ||
      (missing <= 1 && result >= buflen32 &&
      (mode&UNICODE_F_UPDATESRC))) {
    /* Need more source text. */
    state->__word          = ch;
    state->__state.__count = seqlen-missing;
    if (mode & UNICODE_F_ALWAYSZEROTERM) {
     if (result < buflen32)
         ((u32 *)utf32)[result] = 0;
     ++result;
    }
    goto done;
   }
   temp = *src++;
   if ((temp & 0xc0) != 0x80)
        goto handle_error;
   ch <<= 6;
   ch  |= temp;
  }
put_char:
  /* Add the new character to the result. */
  if (result < buflen32)
     ((u32 *)utf32)[result] = ch;
  ++result;
  if (mode & UNICODE_F_DOSINGLE)
      break;
 }
 if (!(mode & UNICODE_F_NOZEROTERM)) {
  if (result < buflen32)
     ((u32 *)utf32)[result] = 0;
  ++result;
 }
done:
 if (mode & UNICODE_F_UPDATESRC) {
  *(u8 **)utf8       = src;
  *(size_t *)utf8len = (size_t)(src_end-src);
 } else if (result > buflen32) {
  /* If the buffer wasn't large enough, restore the shift state. */
  *state = orig_state;
  if (mode & UNICODE_F_BUFERROR) {
   if (mode & __UNICODE_F_EXCEPT)
       libc_throw_buffer_too_small(result,buflen32);
   libc_seterrno(ERANGE);
   return UNICODE_ERROR;
  }
 }
 return result;
}
INTERN size_t LIBCCALL
libc_utf32to8(char32_t const *__restrict utf32, size_t utf32len,
              char *__restrict utf8, size_t buflen8,
              mbstate_t *__restrict state, u32 mode) {
 size_t result = 0; u32 *src,*src_end;
 mbstate_t orig_state = *state;
 if (orig_state.__state.__count & 0x7f) {
  /* Copy characters that weren't printed before. */
  libc_memcpy(utf8,orig_state.__state.__chars,
              MIN(orig_state.__state.__count,buflen8)*
              sizeof(char));
  result = orig_state.__state.__count;
  state->__word = 0;
 }
 if (mode & UNICODE_F_UPDATESRC) {
  src_end = (src = *(u32 **)utf32)+*(size_t *)utf32len;
 } else {
  src_end = (src = (u32 *)utf32)+utf32len;
 }
 while (src < src_end) {
  u32 ch = *src++; u8 i,dst_size,temp[4];
  if (!ch && (mode&UNICODE_F_STOPONNUL)) { --src; break; }
  if unlikely(ch >= UNI_SURROGATE_HIGH_BEGIN &&
              ch <= UNI_SURROGATE_LOW_END) {
handle_error:
   if (mode & UNICODE_F_NOFAIL) {
    ch = UNICODE_REPLACEMENT;
    dst_size = 0;
    goto put_char7;
   }
   if (mode & UNICODE_F_SETERRNO) {
    if (mode & __UNICODE_F_EXCEPT)
        libc_error_throw(E_UNICODE_ERROR);
    libc_seterrno(EILSEQ);
   }
   return UNICODE_ERROR;
  }
  if likely(ch < 0x80) {
put_char7:
   if (result < buflen8)
    ((u8 *)utf8)[result] = (u8)ch;
   else if (mode & UNICODE_F_UPDATESRC) {
    state->__word          = ch;
    state->__state.__count = 1;
    if (mode & UNICODE_F_ALWAYSZEROTERM)
        ++result;
    goto done_update_src;
   }
   ++result;
   goto done_ch;
  }
  if (ch < 0x800) {
   temp[0]  = (u8)(0xc0|(ch >> 6));
   temp[1]  = (u8)(0x80|(ch & 0x3f));
   dst_size = 2;
  } else if (ch < 0x10000) {
   temp[0]  = (u8)(0xe0|(ch >> 12));
   temp[1]  = (u8)(0x80|((ch >> 6) & 0x3f));
   temp[2]  = (u8)(0x80|(ch & 0x3f));
   dst_size = 3;
  } else if likely(ch <= 0x10ffff) {
   temp[0]  = (u8)(0xe0|(ch >> 18));
   temp[1]  = (u8)(0x80|((ch >> 12) & 0x3f));
   temp[2]  = (u8)(0x80|((ch >> 6) & 0x3f));
   temp[3]  = (u8)(0x80|(ch & 0x3f));
   dst_size = 4;
  } else {
   goto handle_error;
  }
  for (i = 0; i < dst_size; ++i) {
   if (result < buflen8)
    ((u8 *)utf8)[result] = temp[i];
   else if (mode & UNICODE_F_UPDATESRC) {
    /* Save unused characters in the shift state. */
    u8 missing = dst_size-i;
    if (mode & UNICODE_F_ALWAYSZEROTERM)
        ++result;
    if unlikely(missing == 4) { --src; goto done_update_src; }
    *(u32 *)state->__state.__chars = *(u32 *)temp;
    state->__state.__count = missing;
    goto done_update_src;
   }
   ++result;
  }
done_ch:
  if (mode&UNICODE_F_DOSINGLE) break;
 }
 if (!(mode & UNICODE_F_NOZEROTERM)) {
  if (result < buflen8)
     ((u8 *)utf8)[result] = 0;
  ++result;
 }
 if (mode & UNICODE_F_UPDATESRC) {
done_update_src:
  *(u32 **)utf32      = src;
  *(size_t *)utf32len = (size_t)(src_end-src);
 } else if (result > buflen8) {
  /* If the buffer wasn't large enough, restore the shift state. */
  *state = orig_state;
  if (mode & UNICODE_F_BUFERROR) {
   if (mode & __UNICODE_F_EXCEPT)
       libc_throw_buffer_too_small(result,buflen8);
   libc_seterrno(ERANGE);
   return UNICODE_ERROR;
  }
 }
 return result;
}


INTERN size_t LIBCCALL
libc_utf8to16(char const *__restrict utf8, size_t utf8len,
              char16_t *__restrict utf16, size_t buflen16,
              mbstate_t *__restrict state, u32 mode) {
 u8 *src,*src_end,temp; mbstate_t orig_state;
 size_t result = 0; u32 ch;
 if (mode & UNICODE_F_UPDATESRC) {
  src_end = (src = *(u8 **)utf8)+*(size_t *)utf8len;
 } else {
  src_end = (src = (u8 *)utf8)+utf8len;
 }
 orig_state = *state;
 if (orig_state.__state.__count != 0) {
  unsigned int missing;
  if (orig_state.__state.__count & 0x80) {
   /* Shift state contains an unwritten high surrogate. */
   if (buflen16)
    ((u16 *)utf16)[0] = orig_state.__word16;
   else if (mode&UNICODE_F_UPDATESRC)
    return 0; /* Buffer too small. */
   state->__word = 0;
   result        = 1;
  } else {
   ch = __MBSTATE_GETWORD24(&orig_state);
   missing = 4-orig_state.__state.__count;
   assert(missing >= 1 && missing <= 3);
   do {
    if (src == src_end ||
        /* Special case: fill the shift state as much as
         *               possible, but return ZERO(0). */
       (missing <= 1 && !buflen16 &&
       (mode&UNICODE_F_UPDATESRC))) {
     state->__word          = ch;
     state->__state.__count = 4-missing;
     if (mode & UNICODE_F_ALWAYSZEROTERM) {
      result = 1;
      if (buflen16)
          ((u16 *)utf16)[0] = 0;
     }
     goto done;
    }
    temp = *src;
    /* Validate secondary sequence character. */
    if ((temp & 0xc0) != 0x80) {
     state->__word = 0;
handle_error:
     if (mode & UNICODE_F_NOFAIL) { ch = UNICODE_REPLACEMENT; goto put_char; }
     if (mode & UNICODE_F_SETERRNO) {
      if (mode & __UNICODE_F_EXCEPT)
          libc_error_throw(E_UNICODE_ERROR);
      libc_seterrno(EILSEQ);
     }
     return UNICODE_ERROR;
    }
    ch <<= 6;
    ch  |= temp;
    ++src;
   } while (--missing);
   /* Clear the MB state. */
   state->__word = 0;
   /* Write the dangling character partially constructed from the MB state. */
   goto put_char;
  }
 }
 while (src < src_end) {
  size_t missing,seqlen;
  ch = *src++;
  seqlen = utf8_sequence_len[ch];
  if unlikely(!seqlen) goto handle_error;
  if (!ch && (mode & UNICODE_F_STOPONNUL)) { --src; break; }
  missing = seqlen-1;
  ch -= utf8_offset[missing];

  /* Read in missing characters. */
  for (; missing; --missing) {
   if (src >= src_end ||
      (missing <= 1 && result >= buflen16 &&
      (mode&UNICODE_F_UPDATESRC))) {
    /* Need more source text. */
    state->__word          = ch;
    state->__state.__count = seqlen-missing;
    if (mode & UNICODE_F_ALWAYSZEROTERM) {
     if (result < buflen16)
         ((u16 *)utf16)[result] = 0;
     ++result;
    }
    goto done;
   }
   temp = *src++;
   if ((temp & 0xc0) != 0x80)
        goto handle_error;
   ch <<= 6;
   ch  |= temp;
  }
put_char:
  if (ch <= 0xffff) {
   /* Add the new character to the result. */
   if (result < buflen16)
      ((u16 *)utf16)[result] = ch;
   ++result;
  } else {
   /* Character needs a low, and a high surrogate. */
   u16 low,high;
   ch  -= 0x10000;
   high = UNI_SURROGATE_HIGH_BEGIN|(ch >> 10);
   low  = UNI_SURROGATE_LOW_BEGIN|(ch & 0x3ff);
   if (result < buflen16)
      ((u16 *)utf16)[result] = high;
   ++result;
   if (result < buflen16)
      ((u16 *)utf16)[result] = low;
   else if (mode & UNICODE_F_UPDATESRC) {
    /* Return the low surrogate the next call. */
    state->__word16        = low;
    state->__state.__count = 0x80;
    goto done_update_src;
   }
   ++result;
  }
  if (mode & UNICODE_F_DOSINGLE)
      break;
 }
 if (!(mode & UNICODE_F_NOZEROTERM)) {
  if (result < buflen16)
     ((u16 *)utf16)[result] = 0;
  ++result;
 }
done:
 if (mode & UNICODE_F_UPDATESRC) {
done_update_src:
  *(u8 **)utf8       = src;
  *(size_t *)utf8len = (size_t)(src_end-src);
 } else if (result > buflen16) {
  /* If the buffer wasn't large enough, restore the shift state. */
  *state = orig_state;
  if (mode & UNICODE_F_BUFERROR) {
   if (mode & __UNICODE_F_EXCEPT)
       libc_throw_buffer_too_small(result,buflen16);
   libc_seterrno(ERANGE);
   return UNICODE_ERROR;
  }
 }
 return result;
}

INTERN size_t LIBCCALL
libc_utf16to8(char16_t const *__restrict utf16, size_t utf16len,
              char *__restrict utf8, size_t buflen8,
              mbstate_t *__restrict state, u32 mode) {
 size_t result = 0; u16 *src,*src_end;
 mbstate_t orig_state = *state; u32 ch;
 if (mode & UNICODE_F_UPDATESRC) {
  src_end = (src = *(u16 **)utf16)+*(size_t *)utf16len;
 } else {
  src_end = (src = (u16 *)utf16)+utf16len;
 }
 if (orig_state.__state.__count) {
  if (orig_state.__state.__count & 0x80) {
   /* Shift state contains a low surrogate. */
   ch = orig_state.__word16;
   state->__word = 0;
   goto do_high_surrogate;
  } else {
   /* Copy characters that weren't printed before. */
   libc_memcpy(utf8,orig_state.__state.__chars,
               MIN(orig_state.__state.__count,buflen8)*
               sizeof(char));
   result = orig_state.__state.__count;
   state->__word = 0;
  }
 }
 while (src < src_end) {
  u8 i,dst_size,temp[4];
  ch = *src++;
  if (!ch && (mode&UNICODE_F_STOPONNUL)) { --src; break; }
  if (ch >= UNI_SURROGATE_LOW_BEGIN &&
      ch <= UNI_SURROGATE_LOW_END) {
   u16 high;
   ch -= UNI_SURROGATE_LOW_BEGIN;
do_high_surrogate:
   if unlikely(src == src_end) {
    if (mode & UNICODE_F_UPDATESRC) {
     /* Parse the high surrogate the next time. */
     state->__word16        = (u16)ch;
     state->__state.__count = 0x80;
    }
    goto done;
   }
   high = *src++;
   if (high < UNI_SURROGATE_HIGH_BEGIN ||
       high > UNI_SURROGATE_LOW_END) {
handle_error:
    if (mode & UNICODE_F_NOFAIL) {
     ch = UNICODE_REPLACEMENT;
     dst_size = 0;
     goto put_char7;
    }
    if (mode & UNICODE_F_SETERRNO) {
     if (mode & __UNICODE_F_EXCEPT)
         libc_error_throw(E_UNICODE_ERROR);
     libc_seterrno(EILSEQ);
    }
    return UNICODE_ERROR;
   }
   /* Combine the low and high surrogates. */
   high -= UNI_SURROGATE_HIGH_BEGIN;
   ch  <<= 10;
   ch   |= high;
  }
  if likely(ch < 0x80) {
put_char7:
   if (result < buflen8)
    ((u8 *)utf8)[result] = (u8)ch;
   else if (mode & UNICODE_F_UPDATESRC) {
    state->__word          = ch;
    state->__state.__count = 1;
    if (mode & UNICODE_F_ALWAYSZEROTERM)
        ++result;
    goto done_update_src;
   }
   ++result;
   goto done_ch;
  }
  if (ch < 0x800) {
   temp[0]  = (u8)(0xc0|(ch >> 6));
   temp[1]  = (u8)(0x80|(ch & 0x3f));
   dst_size = 2;
  } else if (ch < 0x10000) {
   temp[0]  = (u8)(0xe0|(ch >> 12));
   temp[1]  = (u8)(0x80|((ch >> 6) & 0x3f));
   temp[2]  = (u8)(0x80|(ch & 0x3f));
   dst_size = 3;
  } else if likely(ch <= 0x10ffff) {
   temp[0]  = (u8)(0xe0|(ch >> 18));
   temp[1]  = (u8)(0x80|((ch >> 12) & 0x3f));
   temp[2]  = (u8)(0x80|((ch >> 6) & 0x3f));
   temp[3]  = (u8)(0x80|(ch & 0x3f));
   dst_size = 4;
  } else {
   goto handle_error;
  }
  for (i = 0; i < dst_size; ++i) {
   if (result < buflen8)
    ((u8 *)utf8)[result] = temp[i];
   else if (mode & UNICODE_F_UPDATESRC) {
    /* Save unused characters in the shift state. */
    u8 missing = dst_size-i;
    if (mode & UNICODE_F_ALWAYSZEROTERM)
        ++result;
    if unlikely(missing == 4) {
     if (ch > 0xffff) {
      /* Process the low surrogate again during the next call. */
      state->__word16        = (u16)(ch >> 10);
      state->__state.__count = 0x80;
     }
     --src;
     goto done_update_src;
    }
    *(u32 *)state->__state.__chars = *(u32 *)temp;
    state->__state.__count = missing;
    goto done_update_src;
   }
   ++result;
  }
done_ch:
  if (mode&UNICODE_F_DOSINGLE) break;
 }
 if (!(mode & UNICODE_F_NOZEROTERM)) {
  if (result < buflen8)
     ((u8 *)utf8)[result] = 0;
  ++result;
 }
done:
 if (mode & UNICODE_F_UPDATESRC) {
done_update_src:
  *(u16 **)utf16      = src;
  *(size_t *)utf16len = (size_t)(src_end-src);
 } else if (result > buflen8) {
  /* If the buffer wasn't large enough, restore the shift state. */
  *state = orig_state;
  if (mode & UNICODE_F_BUFERROR) {
   if (mode & __UNICODE_F_EXCEPT)
       libc_throw_buffer_too_small(result,buflen8);
   libc_seterrno(ERANGE);
   return UNICODE_ERROR;
  }
 }
 return result;
}



#define FORMAT_MB_BUFSIZE  256
INTERN ssize_t LIBCCALL
libc_format_w16sztomb(pformatprinter printer, void *closure,
                      char16_t const *__restrict c16, size_t c16len,
                      mbstate_t *__restrict ps, u32 mode) {
 char buf[FORMAT_MB_BUFSIZE];
 ssize_t temp,result = 0; size_t num_chars;
 while (c16len) {
  num_chars = libc_utf16to8((char16_t *)&c16,
                            (size_t)&c16len,
                             buf,COMPILER_LENOF(buf),ps,
                             UNICODE_F_NOZEROTERM|
                             UNICODE_F_UPDATESRC|mode);
  if unlikely(num_chars == UNICODE_ERROR)
     return -1;
  temp = (*printer)(buf,num_chars,closure);
  if unlikely(temp < 0) return temp;
  result += temp;
 }
 return result;
}
INTERN ssize_t LIBCCALL
libc_format_w32sztomb(pformatprinter printer, void *closure,
                      char32_t const *__restrict c32, size_t c32len,
                      mbstate_t *__restrict ps, u32 mode) {
 char buf[FORMAT_MB_BUFSIZE];
 ssize_t temp,result = 0; size_t num_chars;
 while (c32len) {
  num_chars = libc_utf32to8((char32_t *)&c32,
                            (size_t)&c32len,
                             buf,COMPILER_LENOF(buf),ps,
                             UNICODE_F_NOZEROTERM|
                             UNICODE_F_UPDATESRC|mode);
  if unlikely(num_chars == UNICODE_ERROR)
     return -1;
  temp = (*printer)(buf,num_chars,closure);
  if unlikely(temp < 0) return temp;
  result += temp;
 }
 return result;
}

#else

PRIVATE u8 const uni_bytemarks[7] = {0x00,0x00,0xC0,0xE0,0xF0,0xF8,0xFC};
PRIVATE u8 const utf8_trailing_bytes[256] = {
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5};
PRIVATE u32 const utf8_offsets[6] = {0x00000000,0x00003080,0x000E2080,
                                     0x03C82080,0xFA082080,0x82082080};
#define UNI_HALF_BASE            0x0010000
#define UNI_HALF_MASK            0x3FF
#define UNI_HALF_SHIFT           10
#define UNI_MAX_BMP              0x0000FFFF
#define UNI_MAX_LEGAL_UTF32      0x0010FFFF
#define UNI_MAX_UTF16            0x0010FFFF
#define UNI_MAX_UTF32            0x7FFFFFFF
#define UNI_SURROGATE_HIGH_END   0xDBFF
#define UNI_SURROGATE_HIGH_BEGIN 0xD800
#define UNI_SURROGATE_LOW_END    0xDFFF
#define UNI_SURROGATE_LOW_BEGIN  0xDC00

LOCAL bool LIBCCALL
libc_utf8_check(char const *__restrict utf8, size_t utf8chars) {
 u8 ch; char const *end = utf8+utf8chars;
 switch (utf8chars) {
 case 4: if ((ch = (u8)*--end) < 0x80 || ch > 0xBF) return false;
 case 3: if ((ch = (u8)*--end) < 0x80 || ch > 0xBF) return false;
 case 2: if ((ch = (u8)*--end) < 0x80 || ch > 0xBF) return false;
  switch ((u8)*utf8) {
  case 0xE0: if (ch < 0xA0) return false; break;
  case 0xED: if (ch > 0x9F) return false; break;
  case 0xF0: if (ch < 0x90) return false; break;
  case 0xF4: if (ch > 0x8F) return false; break;
  default:   if (ch < 0x80) return false; break;
  }
 case 1:
  if ((u8)*utf8 >= 0x80 && (u8)*utf8 < 0xC2)
      return false;
  break;
 default:
  return false;
 }
 if ((u8)*utf8 > 0xF4)
     return false;
 return true;
}


#define MBSTATE_ISEMPTY(x)    ((x)->__count == 0)
#define MBSTATE_GETMISSING(x) ((x)->__count & 0xff)
#define MBSTATE_GETSRCSIZE(x) (((x)->__count >> 16) & 0xff)
#define MBSTATE_SETCOUNT(x,missing,srcsize_minus_one) \
  ((x)->__count = (u32)(missing) | ((u32)(srcsize_minus_one) << 16))

#define GOTO_DONE() do{ if ((mode&(UNICODE_F_ALWAYSZEROTERM|UNICODE_F_NOZEROTERM)) == UNICODE_F_ALWAYSZEROTERM) goto done2; else goto done; }while(0)

INTERN size_t LIBCCALL
libc_utf8to32(char const *__restrict utf8, size_t utf8len,
              char32_t *__restrict utf32, size_t buflen32,
              mbstate_t *__restrict state, u32 mode) {
 char const *iter,*end; u8 src_size; u32 ch;
 u32 *dst = (u32 *)utf32,*dst_end = (u32 *)utf32+buflen32;
 if (mode&UNICODE_F_UPDATESRC) {
  iter = *(char const **)utf8;
  end = iter+(*(size_t *)utf8len);
 } else {
  end = (iter = utf8)+utf8len;
 }
 if (!MBSTATE_ISEMPTY(state)) {
  /* Non-empty input state. */
  if ((size_t)(end-iter) < MBSTATE_GETMISSING(state)) GOTO_DONE(); /* We need more! */
  src_size = MBSTATE_GETSRCSIZE(state);
  /* Read the remaining, missing characters. */
  ch = state->__value.__wch;
  switch (MBSTATE_GETMISSING(state)) {
  case 5: ch += (u8)*iter++; ch <<= 6;
  case 4: ch += (u8)*iter++; ch <<= 6;
  case 3: ch += (u8)*iter++; ch <<= 6;
  case 2: ch += (u8)*iter++; ch <<= 6;
  case 1: ch += (u8)*iter++;
  default: break;
  }
  state->__count = 0;
  goto got_char;
 }
 while (iter != end) {
  size_t avail = (size_t)(end-iter);
  if (mode&UNICODE_F_STOPONNUL && !*iter) break;
  src_size = utf8_trailing_bytes[(u8)*iter];
  ch = 0;
  if unlikely((size_t)src_size+1 > avail) {
   /* Not enough available input characters. - Use the mbstate. */
   /* Store the amount of missing bytes and total bytes. */
   MBSTATE_SETCOUNT(state,(src_size+1)-(u8)avail,src_size);
   assert(avail <= 5);
   assert(avail != 0);
   switch (avail) {
   case 5: ch += (u8)*iter++; ch <<= 6;
   case 4: ch += (u8)*iter++; ch <<= 6;
   case 3: ch += (u8)*iter++; ch <<= 6;
   case 2: ch += (u8)*iter++; ch <<= 6;
   case 1: ch += (u8)*iter++; break;
   }
   state->__value.__wch = ch;
   GOTO_DONE();
  }
  if unlikely(!libc_utf8_check(iter,src_size+1))
     goto err;
  switch (src_size) {
  case 5: ch += (u8)*iter++; ch <<= 6;
  case 4: ch += (u8)*iter++; ch <<= 6;
  case 3: ch += (u8)*iter++; ch <<= 6;
  case 2: ch += (u8)*iter++; ch <<= 6;
  case 1: ch += (u8)*iter++; ch <<= 6;
  case 0: ch += (u8)*iter++; break;
  }
got_char:
  ch -= utf8_offsets[src_size];
  if unlikely(ch > UNI_MAX_LEGAL_UTF32)
     goto err;
  if unlikely(ch >= UNI_SURROGATE_HIGH_BEGIN &&
              ch <= UNI_SURROGATE_LOW_END)
     goto err;
put_char:
  if (dst < dst_end) *dst = ch;
  else if (mode & UNICODE_F_UPDATESRC) goto done_loop_undo;
  ++dst;
  if (mode&UNICODE_F_DOSINGLE) break;
 }
done_loop:
 if (!(mode&UNICODE_F_NOZEROTERM)) {
done2:
  if (dst < dst_end) *dst = 0;
  ++dst;
 }
done:
 if ((mode&UNICODE_F_BUFERROR) && (dst > dst_end)) {
  if (mode & __UNICODE_F_EXCEPT)
      libc_throw_buffer_too_small((size_t)(dst-(u32 *)utf32),
                                   buflen32);
  libc_seterrno(ERANGE);
  return UNICODE_ERROR;
 }
 if (mode&UNICODE_F_UPDATESRC) {
  *(char const **)utf8 = iter;
  *(size_t *)utf8len = (size_t)(end-iter);
 }
 return (size_t)(dst-(u32 *)utf32);
done_loop_undo:
 /* XXX: This isn't correct if `MBSTATE_ISEMPTY(state)' was the case above. */
 iter -= src_size+1;
 goto done_loop;
err:
 if (mode&UNICODE_F_NOFAIL) {
  ch = UNICODE_REPLACEMENT;
  goto put_char;
 }
 if (mode&UNICODE_F_SETERRNO) {
  if (mode & __UNICODE_F_EXCEPT)
      libc_error_throw(E_UNICODE_ERROR);
  libc_seterrno(EILSEQ);
 }
 return UNICODE_ERROR;
}


INTERN size_t LIBCCALL
libc_utf8to16(char const *__restrict utf8, size_t utf8len,
              char16_t *__restrict utf16, size_t buflen16,
              mbstate_t *__restrict state, u32 mode) {
 char const *iter,*end; u8 src_size; u32 ch;
 u16 *dst = (u16 *)utf16,*dst_end = (u16 *)utf16+buflen16;
 if (mode&UNICODE_F_UPDATESRC) {
  iter = *(char const **)utf8;
  end  = iter+(*(size_t *)utf8len);
 } else {
  end = (iter = utf8)+utf8len;
 }
 if (!MBSTATE_ISEMPTY(state)) {
  /* Non-empty input state. */
  if ((end-iter) < MBSTATE_GETMISSING(state)) GOTO_DONE(); /* We need more! */
  src_size = MBSTATE_GETSRCSIZE(state);
  /* Read the remaining, missing characters. */
  ch = state->__value.__wch;
  switch (MBSTATE_GETMISSING(state)) {
  case 5: ch += (u8)*iter++; ch <<= 6;
  case 4: ch += (u8)*iter++; ch <<= 6;
  case 3: ch += (u8)*iter++; ch <<= 6;
  case 2: ch += (u8)*iter++; ch <<= 6;
  case 1: ch += (u8)*iter++;
  default: break;
  }
  state->__count = 0;
  goto got_char;
 }
 while (iter != end) {
  size_t avail = (size_t)(end-iter);
  if (mode&UNICODE_F_STOPONNUL && !*iter) break;
  src_size = utf8_trailing_bytes[(u8)*iter];
  ch = 0;
  if unlikely((size_t)src_size+1 > avail) {
   /* Not enough available input characters. - Use the mbstate. */
   /* Store the amount of missing bytes and total bytes. */
   MBSTATE_SETCOUNT(state,(src_size+1)-(u8)avail,src_size);
   assert(avail <= 5);
   assert(avail != 0);
   switch (avail) {
   case 5: ch += (u8)*iter++; ch <<= 6;
   case 4: ch += (u8)*iter++; ch <<= 6;
   case 3: ch += (u8)*iter++; ch <<= 6;
   case 2: ch += (u8)*iter++; ch <<= 6;
   case 1: ch += (u8)*iter++; break;
   }
   state->__value.__wch = ch;
   GOTO_DONE();
  }
  if unlikely(!libc_utf8_check(iter,src_size+1))
     goto err;
  switch (src_size) {
  case 5: ch += (u8)*iter++; ch <<= 6;
  case 4: ch += (u8)*iter++; ch <<= 6;
  case 3: ch += (u8)*iter++; ch <<= 6;
  case 2: ch += (u8)*iter++; ch <<= 6;
  case 1: ch += (u8)*iter++; ch <<= 6;
  case 0: ch += (u8)*iter++; break;
  }
got_char:
  ch -= utf8_offsets[src_size];
  if likely(ch <= UNI_MAX_BMP) {
   if unlikely(ch >= UNI_SURROGATE_HIGH_BEGIN &&
               ch <= UNI_SURROGATE_LOW_END)
      goto err;
put_char:
   if (dst < dst_end) *dst = (u16)ch;
   else if (mode & UNICODE_F_UPDATESRC) goto done_loop_undo;
   ++dst;
  } else if unlikely(ch > UNI_MAX_UTF16) {
   goto err;
  } else { /* Range: 0xFFFF - 0x10FFFF. */
   ch -= UNI_HALF_BASE;
   if (dst < dst_end) *dst = (char16_t)((u16)((ch >> UNI_HALF_SHIFT)+UNI_SURROGATE_HIGH_BEGIN));
   else if (mode & UNICODE_F_UPDATESRC) goto done_loop_undo;
   ++dst;
   if (dst < dst_end) *dst = (char16_t)((u16)((ch & UNI_HALF_MASK)+UNI_SURROGATE_LOW_BEGIN));
   else if (mode & UNICODE_F_UPDATESRC) { --dst; goto done_loop_undo; }
   else if (mode&UNICODE_F_UTF16HALF) {
    /* Use the shift state. */
    ch = ((ch & UNI_HALF_MASK)+UNI_HALF_BASE)+utf8_offsets[src_size];
    MBSTATE_SETCOUNT(state,0,src_size);
    state->__value.__wch = ch;
    if (!(mode&UNICODE_F_NOZEROTERM)) {
     /*if (dst < dst_end) *dst = 0;*/
     ++dst;
    }
    if (mode&UNICODE_F_UPDATESRC) {
     *(char const **)utf8 = iter;
     *(size_t *)utf8len = (size_t)(end-iter);
    }
    return UNICODE_UTF16HALF;
   }
   ++dst;
  }
  if (mode&UNICODE_F_DOSINGLE) break;
 }
done_loop:
 if (!(mode&UNICODE_F_NOZEROTERM)) {
done2:
  if (dst < dst_end) *dst = 0;
  ++dst;
 }
done:
 if ((mode&UNICODE_F_BUFERROR) && (dst > dst_end)) {
  if (mode & __UNICODE_F_EXCEPT)
      libc_throw_buffer_too_small((size_t)(dst-(u16 *)utf16),
                                   buflen16);
  libc_seterrno(ERANGE);
  return UNICODE_ERROR;
 }
 if (mode&UNICODE_F_UPDATESRC) {
  *(char const **)utf8 = iter;
  *(size_t *)utf8len = (size_t)(end-iter);
 }
 return (size_t)(dst-(u16 *)utf16);
done_loop_undo:
 iter -= src_size+1;
 goto done_loop;
err:
 if (mode&UNICODE_F_NOFAIL) {
  ch = UNICODE_REPLACEMENT;
  goto put_char;
 }
 if (mode&UNICODE_F_SETERRNO) {
  if (mode & __UNICODE_F_EXCEPT)
      libc_error_throw(E_UNICODE_ERROR);
  libc_seterrno(EILSEQ);
 }
 return UNICODE_ERROR;
}



INTERN size_t LIBCCALL
libc_utf32to8(char32_t const *__restrict utf32, size_t utf32len,
              char *__restrict utf8, size_t buflen8,
              mbstate_t *__restrict UNUSED(state), u32 mode) {
 char *dst = utf8,*dst_end = utf8+buflen8;
 u32 const *iter,*end;
 u32 ch; size_t dst_size;
 if (mode&UNICODE_F_UPDATESRC) {
  iter = *(u32 const **)utf32;
  end = iter+(*(size_t *)utf32len);
 } else {
  iter = (u32 const *)utf32;
  end = iter+utf32len;
 }
 while (iter != end) {
  ch = *iter++;
  if (!ch && mode&UNICODE_F_STOPONNUL) break;
  if unlikely(ch >= UNI_SURROGATE_HIGH_BEGIN &&
              ch <= UNI_SURROGATE_LOW_END)
     goto err;
  if likely(ch < (u32)0x80) dst_size = 1;
  else if (ch < (u32)0x800) dst_size = 2;
  else if (ch < (u32)0x10000) dst_size = 3;
  else if likely(ch <= UNI_MAX_LEGAL_UTF32) dst_size = 4;
  else goto err;
  if ((dst+dst_size) >= dst_end && (mode & UNICODE_F_UPDATESRC))
       goto done_loop_undo;
  switch (dst_size) {
  case 4: if (dst < dst_end) { *dst = (char)((ch|0x80)&0xBF); } ++dst; ch >>= 6;
  case 3: if (dst < dst_end) { *dst = (char)((ch|0x80)&0xBF); } ++dst; ch >>= 6;
  case 2: if (dst < dst_end) { *dst = (char)((ch|0x80)&0xBF); } ++dst; ch >>= 6;
  case 1: if (dst < dst_end) { *dst = (char) (ch|uni_bytemarks[dst_size]); } ++dst;
  }
done_ch:
  if (mode&UNICODE_F_DOSINGLE) break;
 }
done_loop:
 if (!(mode&UNICODE_F_NOZEROTERM)) {
/*done2:*/
  if (dst < dst_end) *dst = 0;
  ++dst;
 }
/*done:*/
 if ((mode&UNICODE_F_BUFERROR) && (dst > dst_end)) {
  if (mode & __UNICODE_F_EXCEPT)
      libc_throw_buffer_too_small((size_t)(dst-utf8),
                                   buflen8);
  libc_seterrno(ERANGE);
  return UNICODE_ERROR;
 }
 if (mode&UNICODE_F_UPDATESRC) {
  *(u32 const **)utf32 = iter;
  *(size_t *)utf32len = (size_t)(end-iter);
 }
 return (size_t)(dst-utf8);
done_loop_undo:
 --iter;
 goto done_loop;
err:
 if (mode&UNICODE_F_NOFAIL) {
  if (dst < dst_end) *dst = UNICODE_REPLACEMENT;
  else if (mode & UNICODE_F_UPDATESRC) goto done_loop;
  ++dst;
  goto done_ch;
 }
 if (mode&UNICODE_F_SETERRNO) {
  if (mode & __UNICODE_F_EXCEPT)
      libc_error_throw(E_UNICODE_ERROR);
  libc_seterrno(EILSEQ);
 }
 return UNICODE_ERROR;
}



INTERN size_t LIBCCALL
libc_utf16to8(char16_t const *__restrict utf16, size_t utf16len,
              char *__restrict utf8, size_t buflen8,
              mbstate_t *__restrict state, u32 mode) {
 char *temp,*dst = utf8,*dst_end = utf8+buflen8;
 u16 const *iter,*end,*old_iter;
 u32 ch; size_t dst_size;
 if (mode&UNICODE_F_UPDATESRC) {
  iter = *(u16 const **)utf16;
  end = iter+(*(size_t *)utf16len);
 } else {
  iter = (u16 const *)utf16;
  end  = iter+utf16len;
 }
 if (state->__count) {
  /* Load an old mb-state. */
  if unlikely(iter == end) GOTO_DONE();
  state->__count = 0;
  ch = (u32)state->__value.__wch;
  goto second_char;
 }
 while (iter != end) {
  old_iter = iter;
  ch = (u32)(u16)*iter++;
  if (!ch && mode&UNICODE_F_STOPONNUL) break;
  /* Convert surrogate pair to Utf32 */
  if unlikely(ch < UNI_SURROGATE_HIGH_BEGIN ||
              ch > UNI_SURROGATE_HIGH_END)
     goto err;
  if likely(iter < end) {
   u16 ch2;
second_char:
   ch2 = (u16)*iter;
   if unlikely(ch2 < UNI_SURROGATE_LOW_BEGIN ||
               ch2 > UNI_SURROGATE_LOW_END)
      goto err;
   ch = ((u32)(ch-UNI_SURROGATE_HIGH_BEGIN) << UNI_HALF_SHIFT)+
         (u32)(ch2-UNI_SURROGATE_LOW_BEGIN)+UNI_HALF_BASE;
   ++iter;
  } else {
   /* Partial input string (store last character in mb-state). */
   state->__count = 2;
   state->__value.__wch = ch;
   GOTO_DONE();
  }

  if likely(ch < (u32)0x80) dst_size = 1;
  else if (ch < (u32)0x800) dst_size = 2;
  else if (ch < (u32)0x10000) dst_size = 3;
  else if likely(ch < (u32)0x110000) dst_size = 4;
  else goto err;
  temp = (dst += dst_size);
  if (temp > dst_end && (mode & UNICODE_F_UPDATESRC)) goto done_undo_loop;
  switch (dst_size) {
  case 4: if (temp <= dst_end) { temp[-1] = (char)(u8)((ch|0x80)&0xBF); } --temp,ch >>= 6;
  case 3: if (temp <= dst_end) { temp[-1] = (char)(u8)((ch|0x80)&0xBF); } --temp,ch >>= 6;
  case 2: if (temp <= dst_end) { temp[-1] = (char)(u8)((ch|0x80)&0xBF); } --temp,ch >>= 6;
  case 1: if (temp <= dst_end) { temp[-1] = (char)(u8)(ch|uni_bytemarks[dst_size]); }
  }
done_ch:
  if (mode&UNICODE_F_DOSINGLE) break;
 }
done_loop:
 if (!(mode&UNICODE_F_NOZEROTERM)) {
done2:
  if (dst < dst_end) *dst = 0;
  ++dst;
 }
done:
 if ((mode&UNICODE_F_BUFERROR) && (dst > dst_end)) {
  if (mode & __UNICODE_F_EXCEPT)
      libc_throw_buffer_too_small((size_t)(dst-utf8),
                                   buflen8);
  libc_seterrno(ERANGE);
  return UNICODE_ERROR;
 }
 if (mode&UNICODE_F_UPDATESRC) {
  *(u16 const **)utf16 = iter;
  *(size_t *)utf16len = (size_t)(end-iter);
 }
 return (size_t)(dst-utf8);
done_undo_loop:
 iter = old_iter;
 goto done_loop;
err:
 if (mode&UNICODE_F_NOFAIL) {
  if (dst < dst_end) *dst = UNICODE_REPLACEMENT;
  ++dst;
  goto done_ch;
 }
 if (mode&UNICODE_F_SETERRNO) {
  if (mode & __UNICODE_F_EXCEPT)
      libc_error_throw(E_UNICODE_ERROR);
  libc_seterrno(EILSEQ);
 }
 return UNICODE_ERROR;
}



INTERN ssize_t LIBCCALL
libc_format_w16sztomb(pformatprinter printer, void *closure,
                      char16_t const *__restrict c16, size_t c16len,
                      mbstate_t *__restrict ps, u32 mode) {
 char *iter,buf[4];
 u16 const *end = (u16 const *)c16+c16len;
 u32 ch; size_t dst_size;
 ssize_t result = 0,temp;
 if (ps->__count) {
  /* Load an old mb-state. */
  if unlikely((u16 *)c16 == end) goto done;
  ps->__count = 0;
  ch = (u32)ps->__value.__wch;
  goto second_char;
 }
 while ((u16 *)c16 != end) {
  ch = (u32)(u16)*c16++;
  /* Convert surrogate pair to Utf32 */
  if unlikely(ch < UNI_SURROGATE_HIGH_BEGIN ||
              ch > UNI_SURROGATE_HIGH_END)
     goto err;
  if likely((u16 *)c16 < end) {
   u16 ch2;
second_char:
   ch2 = (u16)*c16;
   if unlikely(ch2 < UNI_SURROGATE_LOW_BEGIN ||
               ch2 > UNI_SURROGATE_LOW_END)
      goto err;
   ch = ((u32)(ch-UNI_SURROGATE_HIGH_BEGIN) << UNI_HALF_SHIFT)+
         (u32)(ch2-UNI_SURROGATE_LOW_BEGIN)+UNI_HALF_BASE;
   ++c16;
  } else {
   /* Partial input string (store last character in mb-state). */
   ps->__count = 2;
   ps->__value.__wch = ch;
   goto done;
  }
  if likely(ch < (u32)0x80) dst_size = 1;
  else if (ch < (u32)0x800) dst_size = 2;
  else if (ch < (u32)0x10000) dst_size = 3;
  else if likely(ch < (u32)0x110000) dst_size = 4;
  else goto err;
  iter = (buf+dst_size);
  switch (dst_size) {
   case 4: *--iter = (char)(u8)((ch|0x80)&0xBF); ch >>= 6;
   case 3: *--iter = (char)(u8)((ch|0x80)&0xBF); ch >>= 6;
   case 2: *--iter = (char)(u8)((ch|0x80)&0xBF); ch >>= 6;
   case 1: *--iter = (char)(u8)(ch|uni_bytemarks[dst_size]); break;
  }
  /* XXX: Maybe not print each character individually? */
print_buf:
  temp = (*printer)(buf,(size_t)(iter-buf),closure);
  if (temp < 0) return temp;
  result += temp;
 }
done:
 return result;
err:
 if (mode&UNICODE_F_NOFAIL) {
  iter = buf+1;
  buf[0] = UNICODE_REPLACEMENT;
  goto print_buf;
 }
 if (mode & __UNICODE_F_EXCEPT)
     libc_error_throw(E_UNICODE_ERROR);
 libc_seterrno(EILSEQ);
 return -1;
}



INTERN ssize_t LIBCCALL
libc_format_w32sztomb(pformatprinter printer, void *closure,
                      char32_t const *__restrict c32, size_t c32len,
                      mbstate_t *__restrict ps, u32 mode) {
 char *iter,buf[4];
 u32 const *end = (u32 *)c32+c32len;
 u32 ch; size_t dst_size;
 ssize_t result = 0,temp;
 while ((u32 *)c32 != end) {
  ch = (u32)*c32++;
  if unlikely(ch >= UNI_SURROGATE_HIGH_BEGIN &&
              ch <= UNI_SURROGATE_LOW_END)
     goto err;
  if likely(ch < (u32)0x80) dst_size = 1;
  else if (ch < (u32)0x800) dst_size = 2;
  else if (ch < (u32)0x10000) dst_size = 3;
  else if likely(ch <= UNI_MAX_LEGAL_UTF32) dst_size = 4;
  else goto err;
  iter = buf;
  switch (dst_size) {
  case 4: *iter++ = (char)((ch|0x80)&0xBF); ch >>= 6;
  case 3: *iter++ = (char)((ch|0x80)&0xBF); ch >>= 6;
  case 2: *iter++ = (char)((ch|0x80)&0xBF); ch >>= 6;
  case 1: *iter++ = (char)((ch|uni_bytemarks[dst_size])); break;
  }
  /* XXX: Maybe not print each character individually? */
print_buf:
  temp = (*printer)(buf,iter-buf,closure);
  if (temp < 0) return temp;
  result += temp;
 }
 return result;
err:
 if (mode&UNICODE_F_NOFAIL) {
  iter = buf+1;
  buf[0] = UNICODE_REPLACEMENT;
  goto print_buf;
 }
 if (mode & __UNICODE_F_EXCEPT)
     libc_error_throw(E_UNICODE_ERROR);
 libc_seterrno(EILSEQ);
 return -1;
}
#endif

INTERN ssize_t LIBCCALL
libc_format_w16sntomb(pformatprinter printer, void *closure,
                      char16_t const *__restrict c16, size_t c16max,
                      mbstate_t *__restrict ps, u32 mode) {
 return libc_format_w16sztomb(printer,closure,c16,
                              libc_w16nlen(c16,c16max),
                              ps,mode);
}
INTERN ssize_t LIBCCALL
libc_format_w32sntomb(pformatprinter printer, void *closure,
                      char32_t const *__restrict c32, size_t c32max,
                      mbstate_t *__restrict ps, u32 mode) {
 return libc_format_w32sztomb(printer,closure,c32,
                              libc_w32nlen(c32,c32max),
                              ps,mode);
}




INTERN char16_t *LIBCCALL libc_utf8to16ms(char const *__restrict utf8, size_t utf8len) {
 size_t reqlen,buflen = (utf8len+(utf8len/3)+1);
 char16_t *result = (char16_t *)libc_malloc(buflen*sizeof(char16_t));
 if unlikely(!result) return NULL;
again:
 { mbstate_t state = MBSTATE_INIT;
   reqlen = libc_utf8to16(utf8,utf8len,result,buflen,&state,0);
   if unlikely(reqlen == UNICODE_ERROR) goto err;
   if (reqlen != buflen) {
    char16_t *new_result = (char16_t *)libc_realloc(result,reqlen*sizeof(char16_t));
    if unlikely(!new_result) {err: libc_free(result); return NULL; }
    result = new_result;
    if unlikely(reqlen > buflen) { buflen = reqlen; goto again; }
   }
 }
 return result;
}
INTERN char32_t *LIBCCALL libc_utf8to32ms(char const *__restrict utf8, size_t utf8len) {
 size_t reqlen,buflen = (utf8len+(utf8len/3)+1);
 char32_t *result = (char32_t *)libc_malloc(buflen*sizeof(char32_t));
 if unlikely(!result) return NULL;
again:
 { mbstate_t state = MBSTATE_INIT;
   reqlen = libc_utf8to32(utf8,utf8len,result,buflen,&state,0);
   if unlikely(reqlen == UNICODE_ERROR) goto err;
   if (reqlen != buflen) {
    char32_t *new_result = (char32_t *)libc_realloc(result,reqlen*sizeof(char32_t));
    if unlikely(!new_result) {err: libc_free(result); return NULL; }
    result = new_result;
    if unlikely(reqlen > buflen) { buflen = reqlen; goto again; }
   }
 }
 return result;
}
INTERN char *LIBCCALL libc_utf16to8ms(char16_t const *__restrict utf16, size_t utf16len) {
 size_t reqlen,buflen = (utf16len+1);
 char *result = (char *)libc_malloc(buflen*sizeof(char));
 if unlikely(!result) return NULL;
again:
 { mbstate_t state = MBSTATE_INIT;
   reqlen = libc_utf16to8(utf16,utf16len,result,buflen,&state,0);
   if unlikely(reqlen == UNICODE_ERROR) goto err;
   if (reqlen != buflen) {
    char *new_result = (char *)libc_realloc(result,reqlen*sizeof(char));
    if unlikely(!new_result) {err: libc_free(result); return NULL; }
    result = new_result;
    if unlikely(reqlen > buflen) { buflen = reqlen; goto again; }
   }
 }
 return result;
}
INTERN char *LIBCCALL libc_utf32to8ms(char32_t const *__restrict utf32, size_t utf32len) {
 size_t reqlen,buflen = (utf32len+1);
 char *result = (char *)libc_malloc(buflen*sizeof(char));
 if unlikely(!result) return NULL;
again:
 { mbstate_t state = MBSTATE_INIT;
   reqlen = libc_utf32to8(utf32,utf32len,result,buflen,&state,0);
   if unlikely(reqlen == UNICODE_ERROR) goto err;
   if (reqlen != buflen) {
    char *new_result = (char *)libc_realloc(result,reqlen*sizeof(char));
    if unlikely(!new_result) {err: libc_free(result); return NULL; }
    result = new_result;
    if unlikely(reqlen > buflen) { buflen = reqlen; goto again; }
   }
 }
 return result;
}

INTERN char16_t *LIBCCALL libc_utf8to16m(char const *__restrict utf8) { return libc_utf8to16ms(utf8,libc_strlen(utf8)); }
INTERN char32_t *LIBCCALL libc_utf8to32m(char const *__restrict utf8) { return libc_utf8to32ms(utf8,libc_strlen(utf8)); }
INTERN char *LIBCCALL libc_utf32to8m(char32_t const *__restrict utf32) { return libc_utf32to8ms(utf32,libc_w32len(utf32)); }
INTERN char *LIBCCALL libc_utf16to8m(char16_t const *__restrict utf16) { return libc_utf16to8ms(utf16,libc_w16len(utf16)); }


/* Export the Unicode API (available through the <unicode.h> KOS extension header) */
EXPORT(uni_utf8to32,libc_utf8to32);
EXPORT(uni_utf8to16,libc_utf8to16);
EXPORT(uni_utf32to8,libc_utf32to8);
EXPORT(uni_utf16to8,libc_utf16to8);
EXPORT(uni_utf8to16m,libc_utf8to16m);
EXPORT(uni_utf8to32m,libc_utf8to32m);
EXPORT(uni_utf16to8m,libc_utf16to8m);
EXPORT(uni_utf32to8m,libc_utf32to8m);
EXPORT(uni_utf8to16ms,libc_utf8to16ms);
EXPORT(uni_utf8to32ms,libc_utf8to32ms);
EXPORT(uni_utf16to8ms,libc_utf16to8ms);
EXPORT(uni_utf32to8ms,libc_utf32to8ms);


/* Export format-printer conversion functions. */
EXPORT(__SYMw16(format_wcsztomb),libc_format_w16sztomb);
EXPORT(__SYMw32(format_wcsztomb),libc_format_w32sztomb);
EXPORT(__SYMw16(format_wcsntomb),libc_format_w16sntomb);
EXPORT(__SYMw32(format_wcsntomb),libc_format_w32sntomb);



EXPORT(Xuni_utf8to32,libc_Xutf8to32);
CRT_EXCEPT size_t LIBCCALL
libc_Xutf8to32(char const *__restrict utf8, size_t utf8chars,
               char32_t *__restrict utf32, size_t bufchars32,
               mbstate_t *__restrict state, u32 mode) {
 return libc_utf8to32(utf8,utf8chars,utf32,bufchars32,state,mode|__UNICODE_F_EXCEPT);
}

EXPORT(Xuni_utf8to16,libc_Xutf8to16);
CRT_EXCEPT size_t LIBCCALL
libc_Xutf8to16(char const *__restrict utf8, size_t utf8chars,
               char16_t *__restrict utf16, size_t bufchars16,
               mbstate_t *__restrict state, u32 mode) {
 return libc_utf8to16(utf8,utf8chars,utf16,bufchars16,state,mode|__UNICODE_F_EXCEPT);
}

EXPORT(Xuni_utf32to8,libc_Xutf32to8);
CRT_EXCEPT size_t LIBCCALL
libc_Xutf32to8(char32_t const *__restrict utf32, size_t utf32chars,
               char *__restrict utf8, size_t bufchars8,
               mbstate_t *__restrict state, u32 mode) {
 return libc_utf32to8(utf32,utf32chars,utf8,bufchars8,state,mode|__UNICODE_F_EXCEPT);
}

EXPORT(Xuni_utf16to8,libc_Xutf16to8);
CRT_EXCEPT size_t LIBCCALL
libc_Xutf16to8(char16_t const *__restrict utf16, size_t utf16chars,
               char *__restrict utf8, size_t bufchars8,
               mbstate_t *__restrict state, u32 mode) {
 return libc_utf16to8(utf16,utf16chars,utf8,bufchars8,state,mode|__UNICODE_F_EXCEPT);
}

EXPORT(Xuni_utf8to16ms,libc_Xutf8to16ms);
CRT_EXCEPT ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED
char16_t *LIBCCALL libc_Xutf8to16ms(char const *__restrict utf8, size_t utf8len) {
 size_t reqlen,buflen = (utf8len+(utf8len/3)+1);
 char16_t *result = (char16_t *)libc_Xmalloc(buflen*sizeof(char16_t));
again:
 LIBC_TRY {
  mbstate_t state = MBSTATE_INIT;
  reqlen = libc_utf8to16(utf8,utf8len,result,buflen,&state,
                         UNICODE_F_SETERRNO|__UNICODE_F_EXCEPT);
  if (reqlen != buflen) {
   result = (char16_t *)libc_Xrealloc(result,reqlen*sizeof(char16_t));
   if unlikely(reqlen > buflen) { buflen = reqlen; goto again; }
  }
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  libc_free(result);
  error_rethrow();
 }
 return result;
}

EXPORT(Xuni_utf8to32ms,libc_Xutf8to32ms);
CRT_EXCEPT ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED
char32_t *LIBCCALL libc_Xutf8to32ms(char const *__restrict utf8, size_t utf8len) {
 size_t reqlen,buflen = (utf8len+(utf8len/3)+1);
 char32_t *result = (char32_t *)libc_Xmalloc(buflen*sizeof(char32_t));
again:
 LIBC_TRY {
  mbstate_t state = MBSTATE_INIT;
  reqlen = libc_utf8to32(utf8,utf8len,result,buflen,&state,
                         UNICODE_F_SETERRNO|__UNICODE_F_EXCEPT);
  if (reqlen != buflen) {
   result = (char32_t *)libc_Xrealloc(result,reqlen*sizeof(char32_t));
   if unlikely(reqlen > buflen) { buflen = reqlen; goto again; }
  }
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  libc_free(result);
  error_rethrow();
 }
 return result;
}

EXPORT(Xuni_utf16to8ms,libc_Xutf16to8ms);
CRT_EXCEPT ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED
char *LIBCCALL libc_Xutf16to8ms(char16_t const *__restrict utf16, size_t utf16len) {
 size_t reqlen,buflen = (utf16len+1);
 char *result = (char *)libc_Xmalloc(buflen*sizeof(char));
 if unlikely(!result) return NULL;
again:
 TRY {
  mbstate_t state = MBSTATE_INIT;
  reqlen = libc_utf16to8(utf16,utf16len,result,buflen,&state,
                         UNICODE_F_SETERRNO|__UNICODE_F_EXCEPT);
  if (reqlen != buflen) {
   result = (char *)libc_Xrealloc(result,reqlen*sizeof(char));
   if unlikely(reqlen > buflen) { buflen = reqlen; goto again; }
  }
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  libc_free(result);
  error_rethrow();
 }
 return result;
}

EXPORT(Xuni_utf32to8ms,libc_Xutf32to8ms);
CRT_EXCEPT ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED
char *LIBCCALL libc_Xutf32to8ms(char32_t const *__restrict utf32, size_t utf32len) {
 size_t reqlen,buflen = (utf32len+1);
 char *result = (char *)libc_Xmalloc(buflen*sizeof(char));
 if unlikely(!result) return NULL;
again:
 TRY {
  mbstate_t state = MBSTATE_INIT;
  reqlen = libc_utf32to8(utf32,utf32len,result,buflen,&state,
                         UNICODE_F_SETERRNO|__UNICODE_F_EXCEPT);
  if (reqlen != buflen) {
   result = (char *)libc_Xrealloc(result,reqlen*sizeof(char));
   if unlikely(reqlen > buflen) { buflen = reqlen; goto again; }
  }
 } LIBC_EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  libc_free(result);
  error_rethrow();
 }
 return result;
}

EXPORT(Xuni_utf8to16m,libc_Xutf8to16m);
CRT_EXCEPT ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED
char16_t *LIBCCALL libc_Xutf8to16m(char const *__restrict utf8) {
 return libc_Xutf8to16ms(utf8,libc_strlen(utf8));
}

EXPORT(Xuni_utf8to32m,libc_Xutf8to32m);
CRT_EXCEPT ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED
char32_t *LIBCCALL libc_Xutf8to32m(char const *__restrict utf8) {
 return libc_Xutf8to32ms(utf8,libc_strlen(utf8));
}

EXPORT(Xuni_utf32to8m,libc_Xutf32to8m);
CRT_EXCEPT ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED
char *LIBCCALL libc_Xutf32to8m(char32_t const *__restrict utf32) {
 return libc_Xutf32to8ms(utf32,libc_w32len(utf32));
}

EXPORT(Xuni_utf16to8m,libc_Xutf16to8m);
CRT_EXCEPT ATTR_RETNONNULL ATTR_MALLOC __MALL_DEFAULT_ALIGNED
char *LIBCCALL libc_Xutf16to8m(char16_t const *__restrict utf16) {
 return libc_Xutf16to8ms(utf16,libc_w16len(utf16));
}

EXPORT(__SYMw16(Xformat_wcsztomb),libc_Xformat_w16sztomb);
CRT_EXCEPT ssize_t LIBCCALL
libc_Xformat_w16sztomb(pformatprinter printer, void *closure,
                       char16_t const *__restrict c16, size_t c16len,
                       mbstate_t *__restrict ps, u32 mode) {
 return libc_format_w16sztomb(printer,closure,c16,c16len,ps,
                              mode|__UNICODE_F_EXCEPT);
}

EXPORT(__SYMw32(Xformat_wcsztomb),libc_Xformat_w32sztomb);
CRT_EXCEPT ssize_t LIBCCALL
libc_Xformat_w32sztomb(pformatprinter printer, void *closure,
                       char32_t const *__restrict c32, size_t c32len,
                       mbstate_t *__restrict ps, u32 mode) {
 return libc_format_w32sztomb(printer,closure,c32,c32len,ps,
                              mode|__UNICODE_F_EXCEPT);
}

EXPORT(__SYMw16(Xformat_wcsntomb),libc_Xformat_w16sntomb);
CRT_EXCEPT ssize_t LIBCCALL
libc_Xformat_w16sntomb(pformatprinter printer, void *closure,
                       char16_t const *__restrict c16, size_t c16max,
                       mbstate_t *__restrict ps, u32 mode) {
 return libc_format_w16sntomb(printer,closure,c16,c16max,ps,
                              mode|__UNICODE_F_EXCEPT);
}

EXPORT(__SYMw32(Xformat_wcsntomb),libc_Xformat_w32sntomb);
CRT_EXCEPT ssize_t LIBCCALL
libc_Xformat_w32sntomb(pformatprinter printer, void *closure,
                       char32_t const *__restrict c32, size_t c32max,
                       mbstate_t *__restrict ps, u32 mode) {
 return libc_format_w32sntomb(printer,closure,c32,c32max,ps,
                              mode|__UNICODE_F_EXCEPT);
}



DECL_END

#endif /* !GUARD_LIBS_LIBC_UNICODE_C */
