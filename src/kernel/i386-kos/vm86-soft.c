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
#ifndef GUARD_KERNEL_I386_KOS_VM86_SOFT_C
#define GUARD_KERNEL_I386_KOS_VM86_SOFT_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/registers.h>
#include <kernel/debug.h>
#include <i386-kos/vm86-soft.h>
#include <asm/cpu-flags.h>
#include <sys/io.h>
#include <stdbool.h>
#include <stddef.h>
#include <except.h>
#include <string.h>

#ifndef CONFIG_NO_VM86_SOFT
DECL_BEGIN

typedef struct vm86_state State;
typedef struct vm86_modrm ModRm;


#define LEA(s,o)     ((uintptr_t)((s) * 16 + ((o) & 0xffff)))
#define MEMB(s,o)    (*(u8 *)LEA(s,o))
#define MEMW(s,o)    (*(u16 *)LEA(s,o))
#define MEML(s,o)    (*(u32 *)LEA(s,o))


PRIVATE bool
(KCALL Throw)(State const *__restrict st,
              struct exception_data const *__restrict exc) {
 /* TODO */
 return false;
}



PRIVATE ptrdiff_t const reg8_offsets[8] = {
     offsetof(struct vm86_regs,al),
     offsetof(struct vm86_regs,cl),
     offsetof(struct vm86_regs,dl),
     offsetof(struct vm86_regs,bl),
     offsetof(struct vm86_regs,ah),
     offsetof(struct vm86_regs,ch),
     offsetof(struct vm86_regs,dh),
     offsetof(struct vm86_regs,bh)
};
#define STATE_REG8(st,regno) \
   (*((u8 *)&(st)->vs_regs + reg8_offsets[regno]))
#define STATE_REG16(st,regno) \
   (*(u16 *)&((u32 *)&(st)->vs_regs)[regno])
#define STATE_REG32(st,regno) \
   (((u32 *)&(st)->vs_regs)[regno])


PRIVATE u16 KCALL
Modrm_MemAddr(ModRm const *__restrict rm,
              State const *__restrict st) {
 u16 result = rm->mr_offset;
 if (rm->mr_rm != 0xff)
     result += STATE_REG16(st,rm->mr_rm);
 if (rm->mr_index != 0xff)
     result += STATE_REG16(st,rm->mr_index);
 return result;
}


#define ModRm_Memb(rm,segment,st) (*_ModRm_Memb(rm,segment,st))
#define ModRm_Memw(rm,segment,st) (*_ModRm_Memw(rm,segment,st))
#define ModRm_Meml(rm,segment,st) (*(u32 *)_ModRm_Memw(rm,segment,st))
PRIVATE u8 *KCALL
_ModRm_Memb(ModRm const *__restrict rm,
            u16 segment, State const *__restrict st) {
 if (rm->mr_type == MODRM_REGISTER)
     return &STATE_REG8(st,rm->mr_rm);
 return &MEMB(segment,Modrm_MemAddr(rm,st));
}
PRIVATE u16 *KCALL
_ModRm_Memw(ModRm const *__restrict rm,
            u16 segment, State const *__restrict st) {
 if (rm->mr_type == MODRM_REGISTER)
     return &STATE_REG16(st,rm->mr_rm);
 return &MEMW(segment,Modrm_MemAddr(rm,st));
}



PRIVATE byte_t *KCALL
ModRm_Decode(byte_t *__restrict text, ModRm *__restrict result) {
 u8 rmbyte = *text++;
 result->mr_reg = X86_MODRM_GETREG(rmbyte);
 switch (X86_MODRM_GETMOD(rmbyte)) {

 case 0:
  result->mr_type = MODRM_MEMORY;
  if (X86_MODRM_GETRM(rmbyte) == X86_REG_ESI) {
   /* Special case */
   result->mr_rm     = 0xff;
   result->mr_index  = 0xff;
   result->mr_offset = *(u16 *)text;
   text += 2;
   break;
  }
  result->mr_offset = 0;
do_memory_operand:
  switch (X86_MODRM_GETRM(rmbyte)) {
  case X86_REG_EAX:
   result->mr_rm    = X86_REG_EBX;
   result->mr_index = X86_REG_ESI;
   break;
  case X86_REG_ECX:
   result->mr_rm    = X86_REG_EBX;
   result->mr_index = X86_REG_EDI;
   break;
  case X86_REG_EDX:
   result->mr_rm    = X86_REG_EBP;
   result->mr_index = X86_REG_ESI;
   break;
  case X86_REG_EBX:
   result->mr_rm    = X86_REG_EBP;
   result->mr_index = X86_REG_EDI;
   break;
  case X86_REG_ESP:
   result->mr_rm    = X86_REG_ESI;
   result->mr_index = 0xff;
   break;
  case X86_REG_EBP:
   result->mr_rm    = X86_REG_EDI;
   result->mr_index = 0xff;
   break;
  case X86_REG_ESI:
   result->mr_rm    = X86_REG_EBP;
   result->mr_index = 0xff;
   break;
  case X86_REG_EDI:
   result->mr_rm    = X86_REG_EBX;
   result->mr_index = 0xff;
   break;
  default: __builtin_unreachable();
  }
  break;

 case 1:
  result->mr_offset = *(u8 *)text;
  text += 1;
  goto do_memory_operand;

 case 2:
  result->mr_offset = *(u16 *)text;
  text += 2;
  goto do_memory_operand;


 case 3:
  /* Register operand. */
  result->mr_type = MODRM_REGISTER;
  result->mr_rm   = X86_MODRM_GETREG(rmbyte);
  break;

 default: __builtin_unreachable();
 }
 return text;
}






PUBLIC void KCALL
vm86_soft_exec(struct vm86_state *__restrict st) {
 struct exception_data exc;
#define F_OP32    0x0001 /* The 0x66 prefix is being used. */
#define F_AD32    0x0002 /* The 0x67 prefix is being used. */
#define F_LOCK    0x0004 /* The `lock' prefix is being used. */
#define F_REPNE   0x0010 /* The `repne' prefix is being used. */
#define F_REP     0x0020 /* The `rep' prefix is being used. */
 byte_t *eip; ModRm rm;
 u32 opcode; u16 flags,segment;
 struct vm86_state *EXCEPT_VAR xst = st;
again:
 flags = 0;
 segment = st->vs_regs.ds;
 eip = (byte_t *)LEA(st->vs_regs.cs,st->vs_regs.ip);
next_opcode:
 opcode  = 0;
extend_instruction:
 opcode |= *eip++;
 TRY {
  switch (opcode) {

  case 0x66: flags |= F_OP32; goto next_opcode;
  case 0x67: flags |= F_AD32; goto next_opcode;
  case 0xf0: flags |= F_LOCK; goto next_opcode;
  case 0xf2: flags |= F_REPNE; goto next_opcode;
  case 0xf3: flags |= F_REP; goto next_opcode;
  case 0x26: segment = st->vs_regs.es; goto next_opcode;
  case 0x2e: segment = st->vs_regs.cs; goto next_opcode;
  case 0x36: segment = st->vs_regs.ss; goto next_opcode;
  case 0x3e: segment = st->vs_regs.ds; goto next_opcode;
  case 0x64: segment = st->vs_regs.fs; goto next_opcode;
  case 0x65: segment = st->vs_regs.gs; goto next_opcode;
  case 0x0f: opcode <<= 8; goto extend_instruction;

#define RM_REG8  STATE_REG8(st,rm.mr_reg)
#define RM_REG16 STATE_REG16(st,rm.mr_reg)
#define RM_REG32 STATE_REG32(st,rm.mr_reg)
#define RM_MEM8  ModRm_Memb(&rm,segment,st)
#define RM_MEM16 ModRm_Memw(&rm,segment,st)
#define RM_MEM32 ModRm_Meml(&rm,segment,st)

  case 0x88:
   /* MOV r/m8,r8 */
   eip = ModRm_Decode(eip,&rm);
   RM_MEM8 = RM_REG8;
   break;
  case 0x89:
   /* MOV r/m16,r16 */
   /* MOV r/m32,r32 */
   eip = ModRm_Decode(eip,&rm);
   if (flags & F_OP32)
    RM_MEM32 = RM_REG32;
   else {
    RM_MEM16 = RM_REG16;
   }
   break;

  case 0x8a:
   /* MOV r8,r/m8 */
   eip = ModRm_Decode(eip,&rm);
   RM_REG8 = RM_MEM8;
   break;
  case 0x8b:
   /* MOV r16,r/m16 */
   /* MOV r32,r/m32 */
   eip = ModRm_Decode(eip,&rm);
   if (flags & F_OP32)
    RM_REG32 = RM_MEM32;
   else {
    RM_REG16 = RM_MEM16;
   }
   break;

  case 0x8c:
   /* MOV r/m16,Sreg** */
   eip = ModRm_Decode(eip,&rm);
   (&st->vs_regs.es)[rm.mr_reg] = RM_MEM16;
   break;
  case 0x8e:
   /* MOV Sreg**,r/m16 */
   eip = ModRm_Decode(eip,&rm);
   RM_MEM16 = (&st->vs_regs.es)[rm.mr_reg];
   break;

  case 0xa0:
   /* MOV AL,moffs8* */
   st->vs_regs.al = MEMB(segment,*(u8 *)eip);
   eip += 1;
   break;
  case 0xa1:
   /* MOV AX,moffs16* */
   /* MOV EAX,moffs32* */
   if (flags & F_AD32) {
    /* XXX: Did this one really exist in vm86? */
    if (flags & F_OP32) {
     st->vs_regs.eax = MEML(segment,*(u32 *)eip);
    } else {
     st->vs_regs.ax  = MEMW(segment,*(u32 *)eip);
    }
    eip += 4;
   } else {
    if (flags & F_OP32) {
     st->vs_regs.eax = MEML(segment,*(u16 *)eip);
    } else {
     st->vs_regs.ax  = MEMW(segment,*(u16 *)eip);
    }
    eip += 2;
   }
   break;
  case 0xa2:
   /* MOV moffs8*,AL */
   MEMB(segment,*(u8 *)eip) = st->vs_regs.al;
   eip += 1;
   break;
  case 0xa3:
   /* MOV AX,moffs16* */
   /* MOV EAX,moffs32* */
   if (flags & F_OP32) {
    MEML(segment,*(u32 *)eip) = st->vs_regs.eax;
    eip += 4; /* XXX: Did this one really exist in vm86? */
   } else {
    MEMW(segment,*(u16 *)eip) = st->vs_regs.ax;
    eip += 2;
   }
   break;

  case 0xb0: st->vs_regs.al = *(u8 *)eip; eip += 1; break;
  case 0xb1: st->vs_regs.cl = *(u8 *)eip; eip += 1; break;
  case 0xb2: st->vs_regs.dl = *(u8 *)eip; eip += 1; break;
  case 0xb3: st->vs_regs.bl = *(u8 *)eip; eip += 1; break;
  case 0xb4: st->vs_regs.ah = *(u8 *)eip; eip += 1; break;
  case 0xb5: st->vs_regs.ch = *(u8 *)eip; eip += 1; break;
  case 0xb6: st->vs_regs.dh = *(u8 *)eip; eip += 1; break;
  case 0xb7: st->vs_regs.bh = *(u8 *)eip; eip += 1; break;
  case 0xb8 ... 0xbf:
   if (flags & F_OP32) {
    STATE_REG32(st,opcode - 0xb8) = *(u32 *)eip;
    eip += 4;
   } else {
    STATE_REG16(st,opcode - 0xb8) = *(u16 *)eip;
    eip += 2;
   }
   break;

  case 0xc6:
   /* MOV r/m8,imm8 */
   eip = ModRm_Decode(eip,&rm);
   if (rm.mr_reg != 0)
       goto illegal_instruction;
   RM_MEM8 = *(u8 *)eip;
   eip += 1;
   break;
  case 0xc7:
   /* MOV r/m16,imm16 */
   /* MOV r/m32,imm32 */
   eip = ModRm_Decode(eip,&rm);
   if (rm.mr_reg != 0)
       goto illegal_instruction;
   if (flags & F_OP32) {
    RM_MEM32 = *(u32 *)eip;
    eip += 4;
   } else {
    RM_MEM16 = *(u16 *)eip;
    eip += 2;
   }
   break;

  case 0x0f20:
  case 0x0f22:
   /* MOV CRx,r32 */
   /* MOV r32,CRx */
   eip = ModRm_Decode(eip,&rm);
   /* Control registers are not implemented by vm86-soft. */
   memset(&exc,0,sizeof(exc));
   exc.e_code                           = E_ILLEGAL_INSTRUCTION;
   exc.e_flag                           = ERR_FNORMAL;
   exc.e_illegal_instruction.ii_errcode = 0;
   exc.e_illegal_instruction.ii_type    = (ERROR_ILLEGAL_INSTRUCTION_UNDEFINED|
                                           ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
   exc.e_illegal_instruction.ii_register_type = X86_REGISTER_CONTROL;
   exc.e_illegal_instruction.ii_register_number = rm.mr_reg;
   goto throw_exception;

  case 0x0f21:
  case 0x0f23:
   /* MOV DRx,r32 */
   /* MOV r32,DRx */
   eip = ModRm_Decode(eip,&rm);
   /* Debug registers are not implemented by vm86-soft. */
   memset(&exc,0,sizeof(exc));
   exc.e_code                           = E_ILLEGAL_INSTRUCTION;
   exc.e_flag                           = ERR_FNORMAL;
   exc.e_illegal_instruction.ii_errcode = 0;
   exc.e_illegal_instruction.ii_type    = (ERROR_ILLEGAL_INSTRUCTION_UNDEFINED|
                                           ERROR_ILLEGAL_INSTRUCTION_FREGISTER);
   exc.e_illegal_instruction.ii_register_type = X86_REGISTER_DEBUG;
   exc.e_illegal_instruction.ii_register_number = rm.mr_reg;
   goto throw_exception;


  default:
illegal_instruction:
   memset(&exc,0,sizeof(exc));
   exc.e_code                           = E_ILLEGAL_INSTRUCTION;
   exc.e_flag                           = ERR_FNORMAL;
   exc.e_illegal_instruction.ii_errcode = 0;
   exc.e_illegal_instruction.ii_type    = (ERROR_ILLEGAL_INSTRUCTION_UNDEFINED|
                                           ERROR_ILLEGAL_INSTRUCTION_FINSTRUCTION);
   goto throw_exception;
  }
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* TODO: Handle a couple of exceptions, such as #DE, #OF and #BR */
  (void)xst;
  error_rethrow();
 }
 st->vs_regs.ip = ((u16)(uintptr_t)eip) & 0xffff;
 return;
throw_exception:
 if (Throw(st,&exc))
     goto again;
}


DECL_END
#endif /* !CONFIG_NO_VM86_SOFT */

#endif /* !GUARD_KERNEL_I386_KOS_VM86_SOFT_C */
