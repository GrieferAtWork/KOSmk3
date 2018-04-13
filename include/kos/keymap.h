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
#ifndef _KOS_KEYMAP_H
#define _KOS_KEYMAP_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#define KMP_MAG0             'K'
#define KMP_MAG1             'm'
#define KMP_MAG2             'p'
#define KMP_MAG3              0x35
#define KMP_VERSION_CURRENT   0
#ifdef __CC__
typedef struct PACKED {
    __UINT8_TYPE__   h_magic[4]; /* Header magic. */
    __UINT8_TYPE__   h_version;  /* File version. */
    __UINT8_TYPE__   h_hdrsize;  /* Total size of the header. (Absolute file-offset to a `Kmp_Data' structure) */
    __UINT8_TYPE__ __h_pad[2];   /* ... */
    char             h_name[64]; /* Name of the keyboard map (NUL-, or h_hdrsize-terminated). */
} Kmp_Header;
#endif /* __CC__ */


#define KMP_ENCODING_ASCII   0x00 /* All characters are encoded as 7-bit ascii characters with the 8th bit ZERO */
#define KMP_ENCODING_8BIT    0x00 /* All characters are encoded as 8-bit integers (single-byte UTF-8 characters) */
#define KMP_ENCODING_UTF8    0x01 /* All characters are encoded as UTF-8 sequences with a length of up to 4 characters each.
                                   * Sequences are terminated either when 4 characters are read, or when the most significant bit of
                                   * a character isn't set. */
#define KMP_ENCODING_UTF16LE 0x02 /* All characters are encoded as UTF-16 sequences (Little endian) with a length
                                   * of a single character each (max. 2 UTF-16 characters; 1 if it isn't a surrogate) */
#define KMP_ENCODING_UTF16BE 0x03 /* Same as `KMP_ENCODING_UTF16LE', but big endian. */
#define KMP_ENCODING_MAX     0x03 /* Max recognized encoding. */
#ifdef __CC__
typedef struct PACKED {
    __UINT8_TYPE__     d_encoding; /* KMP default encoding (One of `KMP_ENCODING_*'). */
    __UINT8_TYPE__   __d_pad[1];   /* ... */
    __UINT8_TYPE__     d_code[1];  /* keyboard mapping pseudo-code (see below) */
} Kmp_Data;
#endif /* __CC__ */


/* KMP registers:
 *   - %pc         The current program counter.
 *   - %key        Points to the next key to-be written. Pre-initialized to `KEY(0,0)'
 *   - %enc        The current encoding for large characters (Pre-initialized to `d_encoding')
 *   - km_press    Vector of characters, pre-initialized to all ZEROes.
 *   - km_shift    Vector of characters, pre-initialized to all ZEROes.
 *   - km_altgr    Vector of characters, pre-initialized to all ZEROes.
 */
#define KMP_OP_STOP      0x00 /* OPCODE: Stop execution */
#define KMP_OP_INCCOL    0x01 /* OPCODE: %key += KEY(0,1); */
#define KMP_OP_DECCOL    0x02 /* OPCODE: %key -= KEY(0,1); */
#define KMP_OP_SETENC    0x03 /* OPCODE: %enc  = *%pc; %pc += 1; */
#define KMP_OP_SETKEY    0x04 /* OPCODE: %key  = *%pc; %pc += 1; */
/*      KMP_OP_          0x05  * ... */
/*      KMP_OP_          ...   * ... */
/*      KMP_OP_          0x18  * ... */
#define KMP_OP_PSETPRESS 0x19 /* OPCODE: km_press[%key - KEY(0,1)] = READ_CHARACTER(%pc,%enc); */
#define KMP_OP_PSETSHIFT 0x1a /* OPCODE: km_shift[%key - KEY(0,1)] = READ_CHARACTER(%pc,%enc); */
#define KMP_OP_PSETALTGR 0x1b /* OPCODE: km_altgr[%key - KEY(0,1)] = READ_CHARACTER(%pc,%enc); */
#define KMP_OP_SETPRESS  0x1c /* OPCODE: km_press[%key] = READ_CHARACTER(%pc,%enc); %key += KEY(0,1); */
#define KMP_OP_SETSHIFT  0x1d /* OPCODE: km_shift[%key] = READ_CHARACTER(%pc,%enc); */
#define KMP_OP_SETALTGR  0x1e /* OPCODE: km_altgr[%key] = READ_CHARACTER(%pc,%enc); */
#define KMP_OP_NOP       0x1f /* OPCODE: // No-op */

/* Special opcodes that encode their operand as part of the instruction. */
#define KMP_OP_MEXTENDED 0xe0 /* Mask for merged operand instructions.
                               * if ((op & KMP_OP_MEXTENDED) == KMP_OP_FDEFLATIN) ... */
#define KMP_OP_FDEFLATIN 0x20 /* >> km_press[%key] = 'a' + (op & 0x1f);
                               * >> km_shift[%key] = 'A' + (op & 0x1f);
                               * >> %key += KEY(0,1); */
#define KMP_OP_FDEFDIGIT 0x40 /* >> km_press[%key] = '1' + (op & 0x1f);
                               * >> km_shift[%key] = '!' + (op & 0x1f);
                               * >> %key += KEY(0,1); */
/*      KMP_OP_F         0x60  * ... */
/*      KMP_OP_F         0x80  * ... */
/*      KMP_OP_F         0xa0  * ... */
#define KMP_OP_FSETROW   0xc0 /* `%key = KEY(op & 0x07,0);' */
#define KMP_OP_FSETCOL   0xe0 /* `%key = KEY(GETROW(%key),op & 0x1f);' */

__SYSDECL_END

#endif /* !_KOS_KEYMAP_H */
