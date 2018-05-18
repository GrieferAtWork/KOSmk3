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
#ifndef GUARD_HYBRID_I386_KOS_INSTRUCTION_C
#define GUARD_HYBRID_I386_KOS_INSTRUCTION_C 1
#define _KOS_SOURCE 2

#include "../hybrid.h"

#include <hybrid/compiler.h>
#include <hybrid/asm.h>
#include <kos/types.h>
#include <except.h>
#include <string.h>

#ifndef __KERNEL__
#include "../../libs/libc/rtl.h"
#include <syslog.h>
#endif

DECL_BEGIN

EXPORT(__rtl_next_instruction,libc_next_instruction);
EXPORT(__rtl_prev_instruction,libc_prev_instruction);


#ifndef SMALL_OPFLAG
#define SMALL_OPFLAG 1
#endif

typedef u8 opflag_t;
#define F_COM     0x0 /* Common opcode (no operand data). */
#define F_PFX16   0x1 /* Prefix opcode ('0x66'). */
#define F_PFX     0x2 /* Prefix opcode. */
#define F_1B      0x3 /* 1 byte immediate. */
#define F_24B     0x4 /* 2/4 byte immediate (based on F_PFX16 prefix). */
#define F_46B     0x5 /* 4/6 byte immediate (based on F_PFX16 prefix). */
#define F_RM_1B   0x6 /* modr/m operand + 1 byte immediate. */
#define F_RM_24B  0x7 /* modr/m operand + 2/4 byte immediate (based on F_PFX16 prefix). */
#define F_RM      0x8 /* modr/m operand. */
#define F_2B      0x9 /* 2 byte immediate. */
#define F_3B      0xa /* 3 byte immediate. */
#define F_SPEC1   0xb /* Special opcode #1 (Special arithmetic opcodes). */
#define F_SPEC2   0xc /* Special opcode #2 (Prefix byte: '0x0f'). */
/*      ...       0xd    ... */
/*      ...       0xe    ... */
#define F_NK(x)   0x0 /* Unknown opcode (Syntactically the same as 'F_COMMON'). */

#if SMALL_OPFLAG
#   define F_GET(group,op)  ((op)&1 ? (group[(op) >> 1] >> 4) : (group[(op) >> 1]&0xf))
#   define P(a,b)           (b<<4)|a
#else
#   define F_GET(group,op)   group[op]
#   define P(a,b)            a,b
#endif



#ifdef __DEEMON__
#define BEGIN_GROUP(name) \
{ local gname = #name; \
  local glist = [none]*256;
#define END_GROUP(name) \
  assert gname == #name;\
  print_group(gname,glist);\
}
#define BEGIN(kind) { local fkind = #kind;
#define END }
#define O(...) \
{ for (local x: [__VA_ARGS__]) {\
    if (glist[x] !is none) \
        print file.io.stderr: "Opcode %x was already defined" % x; \
    glist[x] = fkind;\
  }\
}
#endif

/*[[[deemon
#include <file>
function print_group(name,content) {
    print "PRIVATE opflag_t const "+name+"[256/(1+!!SMALL_OPFLAG)] = {";
    local i = 0,j = 0;
    local first = none;
    for (local x: content) {
        if (!i) print "    ",;
        if (x is none) x = "F_NK";
        if (x == "F_NK") x = "F_NK(%.2x)" % j;
        if (first is none) first = x;
        else {
            print "P("+(first+",").ljust(9)+x+"),"+(" "*(8-#x)),;
            first = none;
        }
        if (++i == 16) { print; i = 0; }
        ++j;
    }
    print "};";
}
#include "instruction-ops.c.inl"

]]]*/
PRIVATE opflag_t const x86_asm_common[256/(1+!!SMALL_OPFLAG)] = {
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_1B,    F_24B),   P(F_COM,   F_COM),   P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_1B,    F_24B),   P(F_COM,   F_SPEC2),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_1B,    F_24B),   P(F_COM,   F_COM),   P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_1B,    F_24B),   P(F_COM,   F_COM),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_1B,    F_24B),   P(F_PFX,   F_COM),   P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_1B,    F_24B),   P(F_PFX,   F_COM),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_1B,    F_24B),   P(F_PFX,   F_COM),   P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_1B,    F_24B),   P(F_PFX,   F_COM),
    P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),
    P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),
    P(F_COM,   F_COM),   P(F_RM,    F_RM),    P(F_PFX,   F_PFX),   P(F_PFX16, F_PFX),   P(F_24B,   F_RM_24B),P(F_1B,    F_RM_1B), P(F_COM,   F_COM),   P(F_COM,   F_COM),
    P(F_1B,    F_1B),    P(F_1B,    F_1B),    P(F_1B,    F_1B),    P(F_1B,    F_1B),    P(F_1B,    F_1B),    P(F_1B,    F_1B),    P(F_1B,    F_1B),    P(F_1B,    F_1B),
    P(F_RM_1B, F_RM_24B),P(F_RM_1B, F_RM_1B), P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),
    P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_46B,   F_PFX),   P(F_COM,   F_COM),   P(F_COM,   F_COM),
    P(F_1B,    F_24B),   P(F_1B,    F_24B),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_1B,    F_24B),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),
    P(F_1B,    F_1B),    P(F_1B,    F_1B),    P(F_1B,    F_1B),    P(F_1B,    F_1B),    P(F_24B,   F_24B),   P(F_24B,   F_24B),   P(F_24B,   F_24B),   P(F_24B,   F_24B),
    P(F_RM_1B, F_RM_1B), P(F_2B,    F_COM),   P(F_RM,    F_RM),    P(F_RM_1B, F_RM_24B),P(F_3B,    F_COM),   P(F_1B,    F_COM),   P(F_COM,   F_1B),    P(F_COM,   F_COM),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_1B,    F_1B),    P(F_COM,   F_COM),   P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),
    P(F_1B,    F_1B),    P(F_1B,    F_1B),    P(F_1B,    F_1B),    P(F_1B,    F_1B),    P(F_24B,   F_24B),   P(F_46B,   F_1B),    P(F_COM,   F_COM),   P(F_COM,   F_COM),
    P(F_PFX,   F_COM),   P(F_PFX,   F_PFX),   P(F_COM,   F_COM),   P(F_SPEC1, F_SPEC1), P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_RM,    F_RM),
};
PRIVATE opflag_t const x86_asm_0f[256/(1+!!SMALL_OPFLAG)] = {
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_NK(0a),F_COM),   P(F_NK(0c),F_NK(0d)),P(F_NK(0e),F_NK(0f)),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_NK(19)),P(F_NK(1a),F_NK(1b)),P(F_NK(1c),F_NK(1d)),P(F_NK(1e),F_NK(1f)),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_NK(25)),P(F_RM,    F_NK(27)),P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),
    P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_NK(36),F_NK(37)),P(F_NK(38),F_NK(39)),P(F_NK(3a),F_NK(3b)),P(F_NK(3c),F_NK(3d)),P(F_NK(3e),F_NK(3f)),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),
    P(F_RM_1B, F_RM_1B), P(F_RM_1B, F_RM_1B), P(F_RM,    F_RM),    P(F_RM,    F_COM),   P(F_NK(78),F_NK(79)),P(F_NK(7a),F_NK(7b)),P(F_RM,    F_RM),    P(F_RM,    F_RM),
    P(F_24B,   F_24B),   P(F_24B,   F_24B),   P(F_24B,   F_24B),   P(F_24B,   F_24B),   P(F_24B,   F_24B),   P(F_24B,   F_24B),   P(F_24B,   F_24B),   P(F_24B,   F_24B),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),
    P(F_COM,   F_COM),   P(F_COM,   F_RM),    P(F_RM_1B, F_RM),    P(F_NK(a6),F_NK(a7)),P(F_COM,   F_COM),   P(F_COM,   F_RM),    P(F_RM_1B, F_RM),    P(F_RM,    F_RM),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_NK(b8),F_COM),   P(F_RM_1B, F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),
    P(F_RM,    F_RM),    P(F_RM_1B, F_RM),    P(F_RM_1B, F_RM_1B), P(F_RM_1B, F_RM),    P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),   P(F_COM,   F_COM),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),
    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_RM),    P(F_RM,    F_NK(ff)),
};
//[[[end]]]
#undef P

#define X86_REG_EAX  0 /* Accumulator. */
#define X86_REG_ECX  1 /* Counter register. */
#define X86_REG_EDX  2 /* General purpose d-register. */
#define X86_REG_EBX  3 /* General purpose b-register. */
#define X86_REG_ESP  4 /* Stack pointer. */
#define X86_REG_EBP  5 /* Stack base pointer. */
#define X86_REG_ESI  6 /* Source pointer. */
#define X86_REG_EDI  7 /* Destination pointer. */

#define MODRM_MOD_MASK   0xc0 /* 0b11000000 */
#define MODRM_REG_MASK   0x38 /* 0b00111000 */
#define MODRM_RM_MASK    0x07 /* 0b00000111 */
#define MODRM_MOD_SHIFT  6
#define MODRM_REG_SHIFT  3
#define MODRM_RM_SHIFT   0

#define MODRM_MOD(x)          (((x) << MODRM_MOD_SHIFT)/*&MODRM_MOD_MASK*/)
#define MODRM_REG(x)          (((x) << MODRM_REG_SHIFT)/*&MODRM_REG_MASK*/)
#define MODRM_RM(x)           (((x) << MODRM_RM_SHIFT)/*&MODRM_RM_MASK*/)
#define MODRM_GETMOD(x)       (((x)&MODRM_MOD_MASK) >> MODRM_MOD_SHIFT)
#define MODRM_GETREG(x)       (((x)&MODRM_REG_MASK) >> MODRM_REG_SHIFT)
#define MODRM_GETRM(x)        (((x)&MODRM_RM_MASK) >> MODRM_RM_SHIFT)
#define MODRM_REGISTER(reg,rm) ((u8)(MODRM_MOD(B(11))|MODRM_REG(reg)|MODRM_RM(rm)))
#define MODRM_DISP16(reg)      ((u8)(MODRM_MOD(B(00))|MODRM_REG(reg)|MODRM_RM(B(110)))) /* Uses what would otherwise be displacement from BP. */
#define MODRM_DISP32(reg)      ((u8)(MODRM_MOD(B(00))|MODRM_REG(reg)|MODRM_RM(B(101))))
#define MODRM_SIBREGISTER       X86_REG_ESP /* When used with MOD 00, 01 or 10, an SIB byte follows. */
#define MODRM_RM_SIB            MODRM_RM(MODRM_SIBREGISTER) /* When used with MOD 00, 01 or 10, an SIB byte follows. */


#undef CONFIG_CODE16
//#define CONFIG_CODE16 1

INTERN u8 *FCALL
libc_next_instruction(u8 *__restrict ip) {
 u8 data;
#ifdef CONFIG_CODE16
 unsigned char has_16b = 2;
#else
 unsigned char has_16b = 4;
#endif
 size_t suffix = 0;
again:
 data = *ip++;
 data = F_GET(x86_asm_common,data);
main_switch:
 switch (data) {
#ifdef CONFIG_CODE16
 case F_PFX16 : if (has_16b != 2) break;
                has_16b = 4; /* fallthrough */
#else
 case F_PFX16 : if (has_16b != 4) break;
                has_16b = 2; /* fallthrough */
#endif
 case F_PFX   : goto again;
 case F_1B    : done_p1: ip += 1; break;
 case F_24B   : ip += has_16b; break;
 case F_46B   : ip += 2+has_16b; break;
 case F_RM_1B : suffix = 1; goto modrm;
 case F_RM_24B: suffix = has_16b; /* fallthrough */
 case F_RM    : goto modrm;
 case F_2B    : ip += 2; break;
 case F_3B    : ip += 3; break;
 case F_SPEC1 : data = *ip++;
                /* 'test' (with a r/m group of '0' ('MODRM_REG_MASK')
                 *         has 8/16/32 bits of immediate data) */
                if ((data&MODRM_REG_MASK) == 0) {
                 if (ip[-2] == 0xf6) suffix += 1;
                 else                suffix += has_16b;
                }
                goto modrm_fetched;
 case F_SPEC2 : data = *ip++;
                data = F_GET(x86_asm_0f,data);
                goto main_switch;
 case F_NK(*) : return NULL; /* Unknown opcode. */
 default      : break; /* Simple, one-byte opcode. */
 }
done:
 ip += suffix;
 return ip;
modrm:
 data = *ip++;
modrm_fetched:
 if (data >= 0xC0) goto done; /* Register operand. */
 /* memory access */
 if (MODRM_GETRM(data) == MODRM_SIBREGISTER) {
  /* instruction with SIB byte */
  u8 sib;
  sib = *ip++;
  if ((sib & 0x7) == 0x05) {
   if ((data & 0xC0) == 0x40) goto done_p1;
done_p4:
   ip += 4;
   goto done;
  }
 }
 switch (data & 0xC0) {
 case 0x0: /* 0/4 byte displacement */
  if ((data & 0x07) == 0x05) goto done_p4;
  goto done; /* 0-length offset */
 case 0x80: goto done_p4; /* 4 byte offset */
 default: goto done_p1;   /* one byte offset */
 }
}

INTERN u8 *FCALL
libc_prev_instruction(u8 *__restrict ip) {
 unsigned int count; u8 *result;
 /* `15' is the max instruction length on X86 */
#if 1
 for (count = 15; count != 0; --count)
#else
 for (count = 1; count <= 15; ++count)
#endif
 {
  /* Load the next instruction offset from here.
   * If it equals the given instruction pointer, we've
   * found the starting address of the previous instruction! */
  result = libc_next_instruction(ip - count);
  if (result == (void *)ip)
      return ip - count;
 }
#if !defined(__KERNEL__) && 0
 libc_syslog(LOG_DEBUG,"Unknown instruction:\n"
             "%$[hex]\n",15,ip - 15);
#endif
 return NULL;
}


DECL_END

#endif /* !GUARD_HYBRID_I386_KOS_INSTRUCTION_C */
