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
#ifndef GUARD_KERNEL_SRC_DEV_KEYMAP_C
#define GUARD_KERNEL_SRC_DEV_KEYMAP_C 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/keymap.h>
#include <kos/keyboard_ioctl.h>
#include <kos/keyboard.h>
#include <kernel/malloc.h>
#include <dev/keymap.h>
#include <kernel/debug.h>
#include <kernel/user.h>
#include <except.h>
#include <string.h>
#include <endian.h>
#include <byteswap.h>
#include <fs/node.h>

DECL_BEGIN

#define KEY_GETROW(x) (((x) >> KEYBOARD_COL_BITS) & ((1 << KEYBOARD_ROW_BITS)-1))
#define KEY_GETCOL(x) ((x) & ((1 << KEYBOARD_COL_BITS)-1))

PRIVATE struct keyboard_keymap_char KCALL
decode_char(u8 **__restrict ppc, u8 enc) {
 struct keyboard_keymap_char result;
 result.kmc_utf8[1] = 0;
 result.kmc_utf8[2] = 0;
 result.kmc_utf8[3] = 0;
 switch (enc) {

 {
  unsigned int i;
 case KMP_ENCODING_UTF8:
  for (i = 0; i < 4; ++i) {
   u8 ch = *(*ppc)++;
   result.kmc_utf8[i] = ch;
   if (!(ch & 0x80)) break;
  }
 } break;

 {
  u16 chhi; u32 ch;
 case KMP_ENCODING_UTF16LE:
 case KMP_ENCODING_UTF16BE:
  chhi = *(*(u16 **)ppc)++;
#if BYTE_ORDER == LITTLE_ENDIAN
  if (enc == KMP_ENCODING_UTF16BE)
      chhi = bswap_16(chhi);
#else
  if (enc == KMP_ENCODING_UTF16LE)
      chhi = bswap_16(chhi);
#endif
  if (chhi >= 0xd800 && chhi <= 0xdbff) {
   /* High surrogate. */
   u16 chlo;
   chlo = *(*(u16 **)ppc)++;
#if BYTE_ORDER == LITTLE_ENDIAN
   if (enc == KMP_ENCODING_UTF16BE)
       chlo = bswap_16(chlo);
#else
   if (enc == KMP_ENCODING_UTF16LE)
       chlo = bswap_16(chlo);
#endif
   ch = (chhi << 16) | chlo;
  } else {
   ch = chhi;
  }
  /* Encode the unicode character as UTF-8 */
  if (ch < 0x80) {
   result.kmc_utf8[0] = (u8)ch;
  } else if (ch < 0x800) {
   result.kmc_utf8[0] = (u8)(0xc0|(ch >> 6));
   result.kmc_utf8[1] = (u8)(0x80|(ch & 0x3f));
  } else if (ch < 0x10000) {
   result.kmc_utf8[0] = (u8)(0xe0|(ch >> 12));
   result.kmc_utf8[1] = (u8)(0x80|((ch >> 6) & 0x3f));
   result.kmc_utf8[2] = (u8)(0x80|(ch & 0x3f));
  } else {
   result.kmc_utf8[0] = (u8)(0xe0|(ch >> 18));
   result.kmc_utf8[1] = (u8)(0x80|((ch >> 12) & 0x3f));
   result.kmc_utf8[2] = (u8)(0x80|((ch >> 6) & 0x3f));
   result.kmc_utf8[3] = (u8)(0x80|(ch & 0x3f));
  }
 } break;

 default:
  result.kmc_utf8[0] = *(*ppc)++;
  break;
 }
 return result;
}


#define FAIL(x)      do{debug_printf("%s(%d) : FAIL\n",__FILE__,__LINE__); goto x;}__WHILE0
#define FAILF(x,...) do{debug_printf("%s(%d) : FAIL : ",__FILE__,__LINE__); debug_printf(__VA_ARGS__); goto x;}__WHILE0

/* Load a new keyboard map from `blob' and store it in `*result'.
 * @return: true:  Successfully read the keyboard map.
 * @return: false: The keyboard map is corrupt. (`*result' is undefined) */
PUBLIC bool KCALL
keymap_load_blob(struct keyboard_keymap *__restrict result,
                 USER CHECKED void *blob) {
 Kmp_Header *header;
 Kmp_Data   *data;
 header = (Kmp_Header *)blob;
 if (header->h_magic[0] != KMP_MAG0) FAIL(fail);
 if (header->h_magic[1] != KMP_MAG1) FAIL(fail);
 if (header->h_magic[2] != KMP_MAG2) FAIL(fail);
 if (header->h_magic[3] != KMP_MAG3) FAIL(fail);
 if (header->h_version != KMP_VERSION_CURRENT) FAIL(fail);
 memset(result,0,sizeof(struct keyboard_keymap));
 memcpy(result->km_name,header->h_name,
        strnlen(header->h_name,
                COMPILER_LENOF(header->h_name))*
        sizeof(char));
 data = (Kmp_Data *)((uintptr_t)blob + header->h_hdrsize);
#if PAGESIZE <= 0xff
 validate_readable(data,1);
#endif

 {
  u8 op,*pc = data->d_code;
  u8 enc = data->d_encoding;
  u8 key = KEY(0,0);
check_enc:
  if (enc > KMP_ENCODING_MAX)
      FAILF(fail,"enc = %x\n",enc);
  for (;;) {
   op = *pc++;
#if 0
   debug_printf("%d,%d -- 0x%.2I8x\n",
                KEY_GETROW(key),
                KEY_GETCOL(key),
                op);
#endif
   switch (op) {
   case KMP_OP_STOP:
    goto done;
   case KMP_OP_INCCOL: key += KEY(0,1); break;
   case KMP_OP_DECCOL: key -= KEY(0,1); break;
   case KMP_OP_SETENC: enc = *pc++; goto check_enc;
   case KMP_OP_SETKEY: key = *pc++; break;
   case KMP_OP_PSETPRESS: result->km_press[(u8)(key-1)] = decode_char(&pc,enc); break;
   case KMP_OP_PSETSHIFT: result->km_shift[(u8)(key-1)] = decode_char(&pc,enc); break;
   case KMP_OP_PSETALTGR: result->km_altgr[(u8)(key-1)] = decode_char(&pc,enc); break;
   case KMP_OP_SETPRESS: result->km_press[key++] = decode_char(&pc,enc); break;
   case KMP_OP_SETSHIFT: result->km_shift[key] = decode_char(&pc,enc); break;
   case KMP_OP_SETALTGR: result->km_altgr[key] = decode_char(&pc,enc); break;
   case KMP_OP_NOP: break;
   default:
    switch (op & KMP_OP_MEXTENDED) {

    case KMP_OP_FDEFLATIN:
     *(u32 *)result->km_press[key].kmc_utf8 = 0;
     *(u32 *)result->km_shift[key].kmc_utf8 = 0;
     result->km_press[key].kmc_utf8[0] = 'a' + (op & 0x1f);
     result->km_shift[key].kmc_utf8[0] = 'A' + (op & 0x1f);
     ++key;
     break;

    case KMP_OP_FDEFDIGIT:
     *(u32 *)result->km_press[key].kmc_utf8 = 0;
     *(u32 *)result->km_shift[key].kmc_utf8 = 0;
     result->km_press[key].kmc_utf8[0] = '1' + (op & 0x1f);
     result->km_shift[key].kmc_utf8[0] = '!' + (op & 0x1f);
     ++key;
     break;

    case KMP_OP_FSETROW:
     key = KEY(op & 0x07,0);
     break;
    case KMP_OP_FSETCOL:
     key = KEY(KEY_GETROW(key),(op & 0x1f));
     break;

    default: FAIL(fail); /* Invalid opcode. */
    }
    break;
   }
  }
 }
done:
 return true;
fail:
 return false;
}

DECL_END

#endif /* !GUARD_KERNEL_SRC_DEV_KEYMAP_C */
