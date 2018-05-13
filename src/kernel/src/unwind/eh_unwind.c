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
#ifndef GUARD_KERNEL_SRC_KERNEL_UNWIND_EH_UNWIND_C
#define GUARD_KERNEL_SRC_KERNEL_UNWIND_EH_UNWIND_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <unwind/eh_frame.h>
#include <kernel/user.h>
#include <kos/context.h>
#include <kernel/debug.h>
#include <sched/task.h>
#include <string.h>
#include <except.h>
#include <assert.h>

#if defined(__i386__) || defined(__x86_64__)
#include <asm/cpu-flags.h>
#include <i386-kos/unwind.h>
#include <i386-kos/gdt.h>
#include <i386-kos/vm86.h>
#else
#error "Unsupported architecture"
#endif

DECL_BEGIN

#ifndef UNWIND_TRANSFORM_REGISTER
#define UNWIND_TRANSFORM_REGISTER(x) x
#endif /* !UNWIND_TRANSFORM_REGISTER */




#if !defined(NDEBUG) && 1
#define FAIL(x)           (debug_printf("%s(%d) : FAIL\n",__FILE__,__LINE__),x)
#define FAILF(x,...)      (debug_printf("%s(%d) : FAIL\n",__FILE__,__LINE__),debug_printf(__VA_ARGS__),x)
#define GOTO_FAIL(x)      do{debug_printf("%s(%d) : FAIL\n",__FILE__,__LINE__); goto x;}__WHILE0
#define GOTO_FAILF(x,...) do{debug_printf("%s(%d) : FAIL : ",__FILE__,__LINE__),debug_printf(__VA_ARGS__); goto x;}__WHILE0
#else
#define FAIL(x)                x
#define FAILF(x,...)           x
#define GOTO_FAIL(x)      goto x
#define GOTO_FAILF(x,...) goto x
#endif

/* Implementation of dwarf stack unwinding (http://www.dwarfstd.org/doc/DWARF4.pdf) */


/* Call Frame Information (section 6.4.1) */
#define rule_undefined      0
#define rule_same_value     1
#define rule_offsetn        2
#define rule_val_offsetn    3
#define rule_register       4
#define rule_expression     5
#define rule_val_expression 6

/* Call Frame Information (section 7.23) */
#define DW_CFA_advance_loc          0x40
#define DW_CFA_offset               0x80
#define DW_CFA_restore              0xC0
#define DW_CFA_nop                  0x00
#define DW_CFA_set_loc              0x01
#define DW_CFA_advance_loc1         0x02
#define DW_CFA_advance_loc2         0x03
#define DW_CFA_advance_loc4         0x04
#define DW_CFA_offset_extended      0x05
#define DW_CFA_restore_extended     0x06
#define DW_CFA_undefined            0x07
#define DW_CFA_same_value           0x08
#define DW_CFA_register             0x09
#define DW_CFA_remember_state       0x0A
#define DW_CFA_restore_state        0x0B
#define DW_CFA_def_cfa              0x0C
#define DW_CFA_def_cfa_register     0x0D
#define DW_CFA_def_cfa_offset       0x0E
#define DW_CFA_def_cfa_expression   0x0F
#define DW_CFA_expression           0x10
#define DW_CFA_offset_extended_sf   0x11
#define DW_CFA_def_cfa_sf           0x12
#define DW_CFA_def_cfa_offset_sf    0x13
#define DW_CFA_val_offset           0x14
#define DW_CFA_val_offset_sf        0x15
#define DW_CFA_val_expression       0x16

/* DWARF Call Frame Instruction (CFI) Extensions (section 10.5.2) */
#define DW_CFA_GNU_args_size                0x2E
#define DW_CFA_GNU_negative_offset_extended 0x2F

INTDEF intptr_t  KCALL decode_sleb128(byte_t **__restrict ptext);
INTDEF uintptr_t KCALL decode_uleb128(byte_t **__restrict ptext);
INTDEF uintptr_t KCALL decode_pointer(byte_t **__restrict ptext, u8 encoding);


struct dw_register {
    union {
        eh_instr_t *r_expr;    /* [valid_if(r_rule == rule_expression ||
                                *           r_rule == rule_val_expression)][1..1] Expression.
                                *  NOTE: This pointer is directed at a `uleb128', describing
                                *        the amount of types that follow immediately after,
                                *        containing expression text. */
        intptr_t    r_value;   /* [valid_if(r_rule != rule_expression &&
                                *           r_rule != rule_val_expression)]
                                *  Register value. */
    };
    u16             r_rule;    /* Register rule (One of `rule_*') */
    u16             r_pad[(sizeof(void *)-2)/2];
};


struct cfi_cfa {
    /* Canonical Frame Address (CFA) */
#define CFA_REGISTER   0        /* Register location. (default) */
#define CFA_EXPRESSION 1        /* Expression. */
    u16             cc_type;    /* Address type (one of `CFA_*') */
    u16             cc_pad[(sizeof(void *)-2)/2]; /* ... */
    intptr_t        cc_offset;  /* [valid_if(cc_type == CFA_REGISTER)]
                                 *  Address offset (added to the value of `cc_regno') */
    union {
        u16         cc_regno;   /* [valid_if(cc_type == CFA_REGISTER)] Register number. */
        eh_instr_t *cc_expr;    /* [valid_if(cc_type == CFA_EXPRESSION)][1..1] Expression.
                                 *  NOTE: This pointer is directed at a `uleb128', describing
                                 *        the amount of types that follow immediately after,
                                 *        containing expression text. */
    };
};
struct cfi_row {
    struct dw_register  cr_regs[UNWIND_NUM_REGISTERS]; /* DWARF registers */
    uintptr_t           cr_argsz;                   /* Argument size */
    struct cfi_cfa      cr_cfa;                     /* Canonical Frame Address */
};


struct dw_state {
    struct fde_info    *s_fde;         /* [1..1] FDE information. */
    eh_instr_t         *s_text;        /* [1..1] Current EH instruction pointer. */
    eh_instr_t         *s_end;         /* [1..1] EH text end pointer. */
    uintptr_t           s_pc;          /* The current program counter, relative to `s_fde->fi_pcbegin'. */
    struct cfi_row      s_row;         /* The current row. */
    struct cfi_row      s_initial_row; /* The initial row. */
    struct cfi_row      s_rem[UNWIND_REMEMBER_STACK_SIZE]; /* Stack of remembered rows. */
    u16                 s_remsz;       /* [<= UNWIND_REMEMBER_STACK_SIZE] Amount of remembered rows. */
};

LOCAL uintptr_t
_impl_FIX_REGNO(uintptr_t regno,
                uintptr_t return_register) {
 return regno == return_register ? UNWIND_CONTEXT_RETURN_REGISTER :
        UNWIND_TRANSFORM_REGISTER(regno);
}
#define FIX_REGNO(regno) ((__typeof__(regno))_impl_FIX_REGNO(regno,(self)->s_fde->fi_retreg))


PRIVATE bool KCALL
dw_eval_expression(eh_instr_t *__restrict text, uintptr_t cfa,
                   struct cpu_context *__restrict context,
                   uintptr_t *__restrict presult,
                   unsigned int flags);
PRIVATE int KCALL
dw_parse_instruction(struct dw_state *__restrict self);


PRIVATE bool KCALL
dw_state_init(struct dw_state *__restrict self,
              struct fde_info *__restrict info) {
 memset(self,0,sizeof(struct dw_state));
 self->s_fde = info;
 if (info->fi_initsize) {
  /* Evaluate the initial text. */
  self->s_text = info->fi_inittext;
  self->s_end  = self->s_text+info->fi_initsize;
  /* Use a zero-initialized row as initial row. */
  memcpy(&self->s_initial_row,
         &self->s_row,sizeof(struct cfi_row));
  while (self->s_text < self->s_end &&
         self->s_pc == 0) {
   if unlikely(dw_parse_instruction(self) < 0)
      return false;
  }
  self->s_pc    = 0;
  self->s_remsz = 0;
 }
 /* Set the text pointers to run the eval-text. */
 self->s_text = info->fi_evaltext;
 self->s_end  = self->s_text+info->fi_evalsize;
 /* Save the initial row. */
 memcpy(&self->s_initial_row,
        &self->s_row,sizeof(struct cfi_row));
 return true;
}



/* @return: -1: Cannot parse opcode.
 * @return:  0: Opcode successfully parsed.
 * @return:  1: Opcode successfully parsed, which was a `*_loc'
 *              opcode (Meaning that `state->s_pc' changed). */
PRIVATE int KCALL
dw_parse_instruction(struct dw_state *__restrict self) {
 eh_instr_t *text = self->s_text;
 u8 opcode,operand;
 int result = 0;
 opcode  = *text & 0xc0;
 operand = *text & 0x3f;
 if (!opcode) opcode = operand; /* Extended opcode. */
 ++text; /* Consume the opcode. */
 switch (opcode) {
#if !defined(NDEBUG) && 0
#define DEBUGF(...) debug_printf(__VA_ARGS__)
#else
#define DEBUGF(...) (void)0
#endif
#if !defined(NDEBUG) && 0
#define TARGET(x) case x: debug_printf("%s(%d) : %p : %s(%u)\n",__FILE__,__LINE__,text-1,#x,operand);
#else
#define TARGET(x) case x:
#endif

 TARGET(DW_CFA_advance_loc)
  DEBUGF("DW_CFA_advance_loc(%Iu)\n",(uintptr_t)operand * self->s_fde->fi_codealign);
  self->s_pc += (uintptr_t)operand * self->s_fde->fi_codealign;
  result      = 1;
  break;

 {
  intptr_t value;
 TARGET(DW_CFA_offset)
  value = ((intptr_t)decode_uleb128(&text) *
            self->s_fde->fi_dataalign);
  operand = FIX_REGNO(operand);
  if unlikely(operand >= UNWIND_NUM_REGISTERS)
     GOTO_FAILF(err,"operand = %u\n",operand);
  DEBUGF("DW_CFA_offset(%I8u,%Id)\n",operand,value);
  //debug_printf("DW_CFA_offset(%u,%Id)\n",operand,value);
  self->s_row.cr_regs[operand].r_rule  = rule_offsetn;
  self->s_row.cr_regs[operand].r_value = value;
 } break;

 TARGET(DW_CFA_restore)
  operand = FIX_REGNO(operand);
  if unlikely(operand >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  DEBUGF("DW_CFA_restore(%u)\n",operand);
  /* Restore registers from the initial row. */
  memcpy(&self->s_row.cr_regs[operand],
         &self->s_initial_row.cr_regs[operand],
          sizeof(struct dw_register));
  break;

 TARGET(DW_CFA_set_loc)
  /* Decode the PC pointer according to FDE pointer encoding. */
  self->s_pc = decode_pointer(&text,self->s_fde->fi_encptr);
  DEBUGF("DW_CFA_set_loc(%Iu)\n",self->s_pc);
  result = 1;
  break;

 TARGET(DW_CFA_advance_loc1)
  DEBUGF("DW_CFA_advance_loc1(%Iu)\n",
        (uintptr_t)(*(u8 *)text * self->s_fde->fi_codealign));
  self->s_pc += (uintptr_t)*(u8 *)text * self->s_fde->fi_codealign;
  text += 1,result = 1;
  break;
 TARGET(DW_CFA_advance_loc2)
  DEBUGF("DW_CFA_advance_loc2(%Iu)\n",
        (uintptr_t)(*(u16 *)text * self->s_fde->fi_codealign));
  self->s_pc += (uintptr_t)*(u16 *)text * self->s_fde->fi_codealign;
  text += 2,result = 1;
  break;
 TARGET(DW_CFA_advance_loc4)
  DEBUGF("DW_CFA_advance_loc4(%Iu)\n",
        (uintptr_t)(*(u64 *)text * self->s_fde->fi_codealign));
  self->s_pc += (uintptr_t)*(u32 *)text * self->s_fde->fi_codealign;
  text += 4,result = 1;
  break;
 {
  uintptr_t reg; intptr_t value;
 TARGET(DW_CFA_offset_extended)
  reg = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  value = ((intptr_t)decode_uleb128(&text) *
            self->s_fde->fi_dataalign);
  DEBUGF("DW_CFA_offset_extended(%Iu,%Id)\n",reg,value);
  self->s_row.cr_regs[reg].r_rule  = rule_offsetn;
  self->s_row.cr_regs[reg].r_value = value;
 } break;

 {
  uintptr_t reg;
 TARGET(DW_CFA_restore_extended)
  reg = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  DEBUGF("DW_CFA_restore_extended(%Iu)\n",reg);
  /* Restore registers from the initial row. */
  memcpy(&self->s_row.cr_regs[reg],
         &self->s_initial_row.cr_regs[reg],
          sizeof(struct dw_register));
 } break;

 {
  uintptr_t reg;
 TARGET(DW_CFA_undefined)
  reg = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  DEBUGF("DW_CFA_undefined(%Iu)\n",reg);
  self->s_row.cr_regs[reg].r_rule = rule_undefined;
 } break;

 {
  uintptr_t reg;
 TARGET(DW_CFA_same_value)
  reg = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg >= UNWIND_NUM_REGISTERS)
     GOTO_FAILF(err,"reg = %Iu\n",reg);
  DEBUGF("DW_CFA_same_value(%Iu)\n",reg);
  self->s_row.cr_regs[reg].r_rule = rule_same_value;
 } break;

 {
  uintptr_t reg1,reg2;
 TARGET(DW_CFA_register)
  reg1 = FIX_REGNO(decode_uleb128(&text));
  reg2 = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg1 >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  if unlikely(reg2 >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  DEBUGF("DW_CFA_register(%Iu,%Iu)\n",reg1,reg2);
  self->s_row.cr_regs[reg1].r_rule  = rule_register;
  self->s_row.cr_regs[reg1].r_value = (intptr_t)reg2;
 } break;                   

 TARGET(DW_CFA_remember_state)
  assert(self->s_remsz <= UNWIND_REMEMBER_STACK_SIZE);
  if unlikely(self->s_remsz == UNWIND_REMEMBER_STACK_SIZE)
     GOTO_FAIL(err); /* Too many remember-states. */
  DEBUGF("DW_CFA_remember_state()\n");
  /* Save (remember) the current row. */
  memcpy(&self->s_rem[self->s_remsz],
         &self->s_row,sizeof(struct cfi_row));
  ++self->s_remsz;
  break;

 TARGET(DW_CFA_restore_state)
  assert(self->s_remsz <= UNWIND_REMEMBER_STACK_SIZE);
  if unlikely(self->s_remsz == 0)
     GOTO_FAIL(err); /* No more remember-states. */
  DEBUGF("DW_CFA_restore_state()\n");
  /* Restore the current row. */
  --self->s_remsz;
  memcpy(&self->s_row,
         &self->s_rem[self->s_remsz],
          sizeof(struct cfi_row));
  break;

 {
  uintptr_t reg;
 TARGET(DW_CFA_def_cfa)
  reg = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  self->s_row.cr_cfa.cc_type   = CFA_REGISTER;
  self->s_row.cr_cfa.cc_regno  = (u16)reg;
  self->s_row.cr_cfa.cc_offset = (intptr_t)decode_uleb128(&text);
  DEBUGF("DW_CFA_def_cfa(%Iu,%Id)\n",reg,self->s_row.cr_cfa.cc_offset);
 } break;

 {
  uintptr_t reg;
 TARGET(DW_CFA_def_cfa_sf)
  reg = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  self->s_row.cr_cfa.cc_type   = CFA_REGISTER;
  self->s_row.cr_cfa.cc_regno  = (u16)reg;
  self->s_row.cr_cfa.cc_offset = (decode_sleb128(&text) *
                                  self->s_fde->fi_dataalign);
  DEBUGF("DW_CFA_def_cfa_sf(%Iu,%Id)\n",reg,self->s_row.cr_cfa.cc_offset);
 } break;

 {
  uintptr_t reg;
 TARGET(DW_CFA_def_cfa_register)
  if unlikely(self->s_row.cr_cfa.cc_type != CFA_REGISTER)
     GOTO_FAIL(err); /* Only allowed when using a register. */
  reg = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  DEBUGF("DW_CFA_def_cfa_register(%Iu)\n",reg);
  self->s_row.cr_cfa.cc_regno = (u16)reg;
  /*self->s_row.cr_cfa.cc_offset = ...;*/ /* Keep the old offset */
 } break;

 TARGET(DW_CFA_def_cfa_offset)
  if unlikely(self->s_row.cr_cfa.cc_type != CFA_REGISTER)
     GOTO_FAIL(err); /* Only allowed when using a register. */
  self->s_row.cr_cfa.cc_offset = (intptr_t)decode_uleb128(&text);
  DEBUGF("DW_CFA_def_cfa_offset(%Iu)\n",self->s_row.cr_cfa.cc_offset);
  break;
 TARGET(DW_CFA_def_cfa_offset_sf)
  if unlikely(self->s_row.cr_cfa.cc_type != CFA_REGISTER)
     GOTO_FAIL(err); /* Only allowed when using a register. */
  self->s_row.cr_cfa.cc_offset = (decode_sleb128(&text) *
                                  self->s_fde->fi_dataalign);
  DEBUGF("DW_CFA_def_cfa_offset_sf(%Id)\n",self->s_row.cr_cfa.cc_offset);
  break;

 TARGET(DW_CFA_def_cfa_expression)
  self->s_row.cr_cfa.cc_type = CFA_EXPRESSION;
  self->s_row.cr_cfa.cc_expr = text;
  DEBUGF("DW_CFA_def_cfa_expression(...)\n");
skip_expression:
  text += decode_uleb128(&text); /* Skip the expression (for now) */
  if unlikely(text < self->s_text)
     GOTO_FAIL(err); /* Check for overflow. */
  break;
 
 {
  uintptr_t reg;
 TARGET(DW_CFA_expression)
  reg = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  DEBUGF("DW_CFA_expression(%Iu,...)\n",reg);
  self->s_row.cr_regs[reg].r_expr = text;
  self->s_row.cr_regs[reg].r_rule  = rule_expression;
  goto skip_expression;
 }

 {
  uintptr_t reg; intptr_t value;
 TARGET(DW_CFA_offset_extended_sf)
  reg = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  value = (decode_sleb128(&text) *
           self->s_fde->fi_dataalign);
  DEBUGF("DW_CFA_offset_extended_sf(%Iu,%Id)\n",reg,value);
  self->s_row.cr_regs[reg].r_rule  = rule_offsetn;
  self->s_row.cr_regs[reg].r_value = value;
 } break;

 {
  uintptr_t reg; intptr_t value;
 TARGET(DW_CFA_val_offset)
  reg = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  value = ((intptr_t)decode_uleb128(&text) *
            self->s_fde->fi_dataalign);
  DEBUGF("DW_CFA_val_offset(%Iu,%Id)\n",reg,value);
  self->s_row.cr_regs[reg].r_rule  = rule_val_offsetn;
  self->s_row.cr_regs[reg].r_value = value;
 } break;

 {
  uintptr_t reg; intptr_t value;
 TARGET(DW_CFA_val_offset_sf)
  reg = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  value = (decode_sleb128(&text) *
           self->s_fde->fi_dataalign);
  DEBUGF("DW_CFA_val_offset_sf(%Iu,%Id)\n",reg,value);
  self->s_row.cr_regs[reg].r_rule  = rule_val_offsetn;
  self->s_row.cr_regs[reg].r_value = value;
 } break;

 {
  uintptr_t reg;
 TARGET(DW_CFA_val_expression)
  reg = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  DEBUGF("DW_CFA_val_expression(%Iu,...)\n",reg);
  self->s_row.cr_regs[reg].r_rule  = rule_val_expression;
  self->s_row.cr_regs[reg].r_expr  = text;
  goto skip_expression;
 }

 TARGET(DW_CFA_GNU_args_size)
  self->s_row.cr_argsz = decode_uleb128(&text);
  DEBUGF("DW_CFA_GNU_args_size(%Iu)\n",self->s_row.cr_argsz);
  break;

 {
  uintptr_t reg; intptr_t value;
 TARGET(DW_CFA_GNU_negative_offset_extended)
  reg = FIX_REGNO(decode_uleb128(&text));
  if unlikely(reg >= UNWIND_NUM_REGISTERS)
     GOTO_FAIL(err);
  value = -((intptr_t)decode_uleb128(&text) *
             self->s_fde->fi_dataalign);
  DEBUGF("DW_CFA_GNU_negative_offset_extended(%Iu,%Id)\n",reg,value);
  self->s_row.cr_regs[reg].r_rule  = rule_offsetn;
  self->s_row.cr_regs[reg].r_value = value;
 } break;

 TARGET(DW_CFA_nop) /* noop. */
  break;
#undef TARGET
 default:
err:
  return -1; /* Unrecognized opcode. */
 }
 self->s_text = text;
 return result;
}

/* @return: -1: error.
 * @return:  0: OK, but didn't change.
 * @return:  1: OK, and did change. */
PRIVATE int KCALL
dw_update_register(struct dw_register *__restrict reg,
                   struct cpu_context *__restrict ctx,
                   unsigned int regno, uintptr_t cfa,
                   unsigned int flags) {
 UnwindRegister COMPILER_IGNORE_UNINITIALIZED(result);
 switch (reg->r_rule) {
 case rule_offsetn:
  result = cfa + reg->r_value;
  TRY {
   if (flags & EH_FRESTRICT_USERSPACE)
       validate_readable((UnwindRegister *)result,sizeof(UnwindRegister));
   result = *(UnwindRegister *)result;
  } CATCH_HANDLED (E_SEGFAULT) {
   goto err;
  }
  break;
 case rule_val_offsetn:
  result = cfa + reg->r_value;
  break;
 {
  uintptr_t exprval;
 case rule_expression:
  if unlikely(!dw_eval_expression(reg->r_expr,cfa,ctx,&exprval,flags))
     GOTO_FAIL(err);
  TRY {
   if (flags & EH_FRESTRICT_USERSPACE)
       validate_readable((UnwindRegister *)exprval,sizeof(UnwindRegister));
   result = *(UnwindRegister *)exprval;
  } CATCH_HANDLED (E_SEGFAULT) {
   goto err;
  }
 } break;
 {
  uintptr_t exprval;
 case rule_val_expression:
  if unlikely(!dw_eval_expression(reg->r_expr,cfa,ctx,&exprval,flags))
     GOTO_FAIL(err);
  result = exprval;
 } break;
 
 case rule_register:
  result = UNWIND_GET_REGISTER(ctx,(u16)(uintptr_t)reg->r_value);
  break;

 default:
 case rule_same_value:
#if 1
  return 0;
#else
  result = UNWIND_GET_REGISTER(ctx,regno);
#endif
  break;
 }
 if (result == UNWIND_GET_REGISTER(ctx,regno))
     return 0;
 UNWIND_SET_REGISTER(ctx,regno,result);
 return 1;
err:
 return -1;
}

PRIVATE bool KCALL
private_decode_cfa(struct cfi_row *__restrict row,
                   struct cpu_context *__restrict context,
                   uintptr_t *__restrict presult,
                   unsigned int flags) {
 uintptr_t result;
 if (row->cr_cfa.cc_type == CFA_EXPRESSION)
     return dw_eval_expression(row->cr_cfa.cc_expr,0,context,presult,flags);
 result = UNWIND_GET_REGISTER(context,row->cr_cfa.cc_regno);
 if unlikely(__builtin_add_overflow(result,row->cr_cfa.cc_offset,&result))
    GOTO_FAIL(err);
 *presult = result;
 return true;
err:
 return false;
}


#if defined(__x86_64__) || defined(__i386__)
INTDEF ATTR_PERTASK struct x86_irregs_user iret_saved;
INTDEF NOIRQ void ASMCALL x86_redirect_preemption(void);
#endif

PUBLIC bool KCALL
eh_return(struct fde_info *__restrict info,
          struct cpu_context *__restrict ctx,
          unsigned int flags) {
 struct dw_state state;
 bool changed = false;
 uintptr_t cfa; unsigned int i;
 uintptr_t ip = UNWIND_CONTEXT_IP(ctx) - info->fi_pcbegin;
 if unlikely(!dw_state_init(&state,info))
    return false;
 /* Evaluate the code to find the proper IP. */
 while (state.s_pc <= ip) {
  int error;
  if (state.s_text >= state.s_end)
      break;
  error = dw_parse_instruction(&state);
  if unlikely(error < 0)
     GOTO_FAIL(err);
 }
 /* Calculate the CFA value. */
 if unlikely(!private_decode_cfa(&state.s_row,ctx,&cfa,flags))
    GOTO_FAIL(err);

 /* Apply new register values. */
 for (i = 0; i < UNWIND_NUM_REGISTERS; ++i) {
  int error;
  error = dw_update_register(&state.s_row.cr_regs[i],ctx,i,cfa,flags);
  if unlikely(error < 0)
     goto err;
  if (error) changed = true;
 }
 /* Set the new SP value. */
 cfa += state.s_row.cr_argsz;
 if (UNWIND_CONTEXT_SP(ctx) != cfa)
     changed = true;
 /*debug_printf("aug_str = %q\n",info->fi_augstr);*/

 UNWIND_CONTEXT_SP(ctx) = cfa;

 if (info->fi_sigframe &&
   !(flags & (EH_FRESTRICT_USERSPACE|EH_FDONT_UNWIND_SIGFRAME))) {
  /* Also unwind the signal frame. */
#ifdef __x86_64__
#error TODO
#elif defined(__i386__)
  if (ctx->c_eip == (uintptr_t)&x86_redirect_preemption) {
   struct x86_irregs_user *saved = &PERTASK(iret_saved);
   ctx->c_iret.ir_eip    = saved->ir_eip;
   ctx->c_iret.ir_cs     = saved->ir_cs;
   ctx->c_iret.ir_eflags = saved->ir_eflags;
   ctx->c_esp            = saved->ir_useresp;
#ifdef CONFIG_VM86
  } else if (ctx->c_eflags & EFLAGS_VM) {
   struct x86_irregs_vm86 *iret;
   iret = (struct x86_irregs_vm86 *)((struct x86_irregs_host32 *)ctx->c_esp-1);
   ctx->c_iret.ir_cs = iret->ir_cs;
   ctx->c_esp        = iret->ir_esp;
   assertf(ctx->c_eip == iret->ir_eip,"Forgot to restore EIP at %p",info->fi_pcbegin);
   assertf(ctx->c_eflags == iret->ir_eflags,"Forgot to restore EFLAGS at %p",info->fi_pcbegin);
#ifndef CONFIG_NO_X86_SEGMENTATION
   ctx->c_segments.sg_gs = iret->ir_gs;
   ctx->c_segments.sg_fs = iret->ir_fs;
   ctx->c_segments.sg_es = iret->ir_es;
   ctx->c_segments.sg_ds = iret->ir_ds;
#endif /* !CONFIG_NO_X86_SEGMENTATION */
#endif /* CONFIG_VM86 */
  } else {
   struct x86_irregs_user32 *iret;
   iret = (struct x86_irregs_user32 *)((struct x86_irregs_host32 *)ctx->c_esp-1);
   assertf(ctx->c_eip == iret->ir_eip,"Forgot to restore EIP at %p",info->fi_pcbegin);
   assertf(ctx->c_eflags == iret->ir_eflags,"Forgot to restore EFLAGS at %p",info->fi_pcbegin);
   ctx->c_iret.ir_cs = iret->ir_cs;
   if (iret->ir_cs & 3) {
    /* Return to user-space. */
    ctx->c_esp = iret->ir_useresp;
    assertf(iret->ir_eflags & EFLAGS_IF,"Corrupt unwind IRET tail {%p,%p,%p,%p}",
            ctx->c_eip,ctx->c_esp,iret->ir_cs,iret->ir_eflags);
    assertf(iret->ir_cs == X86_USER_CS,"Corrupt unwind IRET tail {%p,%p,%p,%p}",
            ctx->c_eip,ctx->c_esp,iret->ir_cs,iret->ir_eflags);
   } else {
    assertf(iret->ir_cs == X86_KERNEL_CS,"Corrupt unwind IRET tail=%p {%p,%p,%p,%p}",
            iret,ctx->c_eip,ctx->c_esp,iret->ir_cs,iret->ir_eflags);
   }
  }
  changed = true;
#else
#error "Unsupported architecture"
#endif
 }

 return changed;
err:
 return false;
}

PUBLIC bool KCALL
eh_jmp(struct fde_info *__restrict info,
       struct cpu_context *__restrict ctx,
       uintptr_t abs_ip, unsigned int flags) {
#if 0
 UNWIND_CONTEXT_IP(ctx) = abs_ip;
 return true;
#else
 struct dw_state state;
 uintptr_t cfa; unsigned int i;
 struct cpu_context start_context;
 uintptr_t new_sp,ip = UNWIND_CONTEXT_IP(ctx) - info->fi_pcbegin;
 if unlikely(!dw_state_init(&state,info))
    return false;
 /* Evaluate the code to find the proper IP. */
 while (state.s_pc <= ip) {
  int error;
  if (state.s_text >= state.s_end)
      break;
  error = dw_parse_instruction(&state);
  if unlikely(error < 0)
     GOTO_FAIL(err);
 }
 if (state.s_row.cr_regs[UNWIND_FRAME_REGSITER].r_rule != rule_undefined) {
  /* This right here goes hand-in-hand with the `__EXCEPT_CLOBBER_REGS()'
   * macro, which ensures that CFI has defined a consistent state for
   * the frame-register.
   * If it has, then all we really need to do, is to set `abs_ip'.
   * Otherwise, we can only try our best to reverse-engineer what's
   * going on at the target address. */
  UNWIND_CONTEXT_IP(ctx) = abs_ip;
  return true;
 }

 /* Save the initial CPU context. */
 memcpy(&start_context,ctx,sizeof(struct cpu_context));

 /* With the correct location found, unwind to get the start of the function. */
 if (!eh_return(info,ctx,flags|EH_FDONT_UNWIND_SIGFRAME))
      goto err;

 /* Set the new instruction pointer. */
 UNWIND_CONTEXT_IP(ctx) = abs_ip;
 cfa = UNWIND_CONTEXT_SP(ctx);
 /* Now reverse-engineer register values. */
 if (state.s_row.cr_cfa.cc_type == CFA_EXPRESSION) {
#if 1
  /* TODO: Continue decreasing UNWIND_CONTEXT_SP(&start_context) by `sizeof(void *)' until
   *      `dw_eval_expression(&start_context)' returns a stack-base-pointer
   *       that is == UNWIND_CONTEXT_SP(ctx) (with `ctx' currently pointing to the base of the function) */
  GOTO_FAIL(err);
#endif
 } else
 {
  if unlikely(__builtin_sub_overflow(cfa,state.s_row.cr_cfa.cc_offset,&new_sp))
     GOTO_FAIL(err); /* Subtract the stack-offset */
 }
 if unlikely(__builtin_sub_overflow(new_sp,state.s_row.cr_argsz,&new_sp))
    GOTO_FAIL(err); /* Subtract argument memory */

 /* If `.cfi_def_cfa_register' was used, then we can't determine
  * the proper stack-offset from ESP at the start of the function.
  * However, we need to know this in order to jump here...
  * Assigning `new_sp' here will free all local variables,
  * while still keeping return address and argument memory.
  * For that reason, we can only use the CFA stack pointer
  * as a hint that can increase the stack-size even further.
  * However, doing this introduces a memory leak caused by
  * SP potentially being larger than it should be. */
 new_sp = UNWIND_CONTEXT_SP(&start_context);
#ifdef CONFIG_STACK_GROWS_UPWARDS
 if (UNWIND_CONTEXT_SP(ctx) < new_sp)
     UNWIND_CONTEXT_SP(ctx) = new_sp;
#else
 if (UNWIND_CONTEXT_SP(ctx) > new_sp)
     UNWIND_CONTEXT_SP(ctx) = new_sp;
#endif

 /* Do this part in reverse order. */
 i = UNWIND_NUM_REGISTERS;
 while (i--) {
  /* Reverse engineer all the other registers. */
  struct dw_register *reg = &state.s_row.cr_regs[i];
  switch (reg->r_rule) {
  case rule_offsetn:
  case rule_undefined:
   /* Continue using the start-context's value of this register. */
   if (i == UNWIND_CONTEXT_RETURN_REGISTER) break;
#if 0
   debug_printf("Reusing old value %p for reg %d (override %p)\n",
                UNWIND_GET_REGISTER(&start_context,i),i,
                UNWIND_GET_REGISTER(ctx,i));
#endif
   UNWIND_SET_REGISTER(ctx,i,UNWIND_GET_REGISTER(&start_context,i));
   break;
  case rule_val_offsetn:
   UNWIND_SET_REGISTER(ctx,i,cfa-reg->r_value);
   break;
  case rule_register:
   /* Reverse register rule. */
#if 0
   debug_printf("Setting old value %p for reg %d (override %p)\n",
                UNWIND_GET_REGISTER(ctx,i),i,
                UNWIND_GET_REGISTER(&start_context,i));
#endif
   UNWIND_SET_REGISTER(&start_context,(u16)(uintptr_t)reg->r_value,
                       UNWIND_GET_REGISTER(ctx,i));
   break;
#if 0
  case rule_expression:
   GOTO_FAILF(err,"TODO\n");
   break;
  case rule_val_expression:
   GOTO_FAILF(err,"TODO\n");
   break;
#endif
  default:
  case rule_same_value:
   break;
  }
 }
 return true;
err:
 return false;
#endif
}




#define DW_OP_addr                  0x03
#define DW_OP_deref                 0x06
#define DW_OP_const1u               0x08
#define DW_OP_const1s               0x09
#define DW_OP_const2u               0x0A
#define DW_OP_const2s               0x0B
#define DW_OP_const4u               0x0C
#define DW_OP_const4s               0x0D
#define DW_OP_const8u               0x0E
#define DW_OP_const8s               0x0F
#define DW_OP_constu                0x10
#define DW_OP_consts                0x11
#define DW_OP_dup                   0x12
#define DW_OP_drop                  0x13
#define DW_OP_over                  0x14
#define DW_OP_pick                  0x15
#define DW_OP_swap                  0x16
#define DW_OP_rot                   0x17
#define DW_OP_xderef                0x18
#define DW_OP_abs                   0x19
#define DW_OP_and                   0x1A
#define DW_OP_div                   0x1B
#define DW_OP_minus                 0x1C
#define DW_OP_mod                   0x1D
#define DW_OP_mul                   0x1E
#define DW_OP_neg                   0x1F
#define DW_OP_not                   0x20
#define DW_OP_or                    0x21
#define DW_OP_plus                  0x22
#define DW_OP_plus_uconst           0x23
#define DW_OP_shl                   0x24
#define DW_OP_shr                   0x25
#define DW_OP_shra                  0x26
#define DW_OP_xor                   0x27
#define DW_OP_skip                  0x2F
#define DW_OP_bra                   0x28
#define DW_OP_eq                    0x29
#define DW_OP_ge                    0x2A
#define DW_OP_gt                    0x2B
#define DW_OP_le                    0x2C
#define DW_OP_lt                    0x2D
#define DW_OP_ne                    0x2E
#define DW_OP_lit0                  0x30
#define DW_OP_lit1                  0x31
#define DW_OP_lit2                  0x32
#define DW_OP_lit3                  0x33
#define DW_OP_lit4                  0x34
#define DW_OP_lit5                  0x35
#define DW_OP_lit6                  0x36
#define DW_OP_lit7                  0x37
#define DW_OP_lit8                  0x38
#define DW_OP_lit9                  0x39
#define DW_OP_lit10                 0x3A
#define DW_OP_lit11                 0x3B
#define DW_OP_lit12                 0x3C
#define DW_OP_lit13                 0x3D
#define DW_OP_lit14                 0x3E
#define DW_OP_lit15                 0x3F
#define DW_OP_lit16                 0x40
#define DW_OP_lit17                 0x41
#define DW_OP_lit18                 0x42
#define DW_OP_lit19                 0x43
#define DW_OP_lit20                 0x44
#define DW_OP_lit21                 0x45
#define DW_OP_lit22                 0x46
#define DW_OP_lit23                 0x47
#define DW_OP_lit24                 0x48
#define DW_OP_lit25                 0x49
#define DW_OP_lit26                 0x4A
#define DW_OP_lit27                 0x4B
#define DW_OP_lit28                 0x4C
#define DW_OP_lit29                 0x4D
#define DW_OP_lit30                 0x4E
#define DW_OP_lit31                 0x4F
#define DW_OP_reg0                  0x50
#define DW_OP_reg1                  0x51
#define DW_OP_reg2                  0x52
#define DW_OP_reg3                  0x53
#define DW_OP_reg4                  0x54
#define DW_OP_reg5                  0x55
#define DW_OP_reg6                  0x56
#define DW_OP_reg7                  0x57
#define DW_OP_reg8                  0x58
#define DW_OP_reg9                  0x59
#define DW_OP_reg10                 0x5A
#define DW_OP_reg11                 0x5B
#define DW_OP_reg12                 0x5C
#define DW_OP_reg13                 0x5D
#define DW_OP_reg14                 0x5E
#define DW_OP_reg15                 0x5F
#define DW_OP_reg16                 0x60
#define DW_OP_reg17                 0x61
#define DW_OP_reg18                 0x62
#define DW_OP_reg19                 0x63
#define DW_OP_reg20                 0x64
#define DW_OP_reg21                 0x65
#define DW_OP_reg22                 0x66
#define DW_OP_reg23                 0x67
#define DW_OP_reg24                 0x68
#define DW_OP_reg25                 0x69
#define DW_OP_reg26                 0x6A
#define DW_OP_reg27                 0x6B
#define DW_OP_reg28                 0x6C
#define DW_OP_reg29                 0x6D
#define DW_OP_reg30                 0x6E
#define DW_OP_reg31                 0x6F
#define DW_OP_breg0                 0x70
#define DW_OP_breg1                 0x71
#define DW_OP_breg2                 0x72
#define DW_OP_breg3                 0x73
#define DW_OP_breg4                 0x74
#define DW_OP_breg5                 0x75
#define DW_OP_breg6                 0x76
#define DW_OP_breg7                 0x77
#define DW_OP_breg8                 0x78
#define DW_OP_breg9                 0x79
#define DW_OP_breg10                0x7A
#define DW_OP_breg11                0x7B
#define DW_OP_breg12                0x7C
#define DW_OP_breg13                0x7D
#define DW_OP_breg14                0x7E
#define DW_OP_breg15                0x7F
#define DW_OP_breg16                0x80
#define DW_OP_breg17                0x81
#define DW_OP_breg18                0x82
#define DW_OP_breg19                0x83
#define DW_OP_breg20                0x84
#define DW_OP_breg21                0x85
#define DW_OP_breg22                0x86
#define DW_OP_breg23                0x87
#define DW_OP_breg24                0x88
#define DW_OP_breg25                0x89
#define DW_OP_breg26                0x8A
#define DW_OP_breg27                0x8B
#define DW_OP_breg28                0x8C
#define DW_OP_breg29                0x8D
#define DW_OP_breg30                0x8E
#define DW_OP_breg31                0x8F
#define DW_OP_regx                  0x90
#define DW_OP_fbreg                 0x91
#define DW_OP_bregx                 0x92
#define DW_OP_piece                 0x93
#define DW_OP_deref_size            0x94
#define DW_OP_xderef_size           0x95
#define DW_OP_nop                   0x96
#define DW_OP_push_object_addres    0x97
#define DW_OP_call2                 0x98
#define DW_OP_call4                 0x99
#define DW_OP_call_ref              0x9A
#define DW_OP_form_tls_address      0x9B
#define DW_OP_call_frame_cfa        0x9C
#define DW_OP_bit_piece             0x9D
#define DW_OP_implicit_value        0x9E
#define DW_OP_stack_value           0x9F
#define DW_OP_lo_user               0xE0
#define DW_OP_hi_user               0xFF


#define EXPRESSION_STACK_SIZE 32

PRIVATE bool KCALL
dw_eval_expression(eh_instr_t *__restrict text, uintptr_t cfa,
                   struct cpu_context *__restrict context,
                   uintptr_t *__restrict presult, unsigned int flags) {
 unsigned int i = 0;
 uintptr_t stack[EXPRESSION_STACK_SIZE];
 stack[0] = cfa;
 eh_instr_t *end = text+decode_uleb128((byte_t **)&text);
 TRY {
  while (text <= end) {
   eh_instr_t opcode = *text++;

   if unlikely(i >= EXPRESSION_STACK_SIZE-1)
      GOTO_FAIL(err);
   switch (opcode) {
   case DW_OP_addr:
    stack[++i] = *(uintptr_t *)text;
    text += sizeof(uintptr_t);
    break;

   case DW_OP_deref:
    if (stack[i] == 0)
        GOTO_FAIL(err);
    stack[i] = *(uintptr_t *)stack[i];
    break;

   case DW_OP_const1u: stack[++i] = (uintptr_t)*(u8 *)text; text += 1; break;
   case DW_OP_const1s: stack[++i] = (uintptr_t)*(s8 *)text; text += 1; break;
   case DW_OP_const2u: stack[++i] = (uintptr_t)*(u16 *)text; text += 2; break;
   case DW_OP_const2s: stack[++i] = (uintptr_t)*(s16 *)text; text += 2; break;
   case DW_OP_const4u: stack[++i] = (uintptr_t)*(u32 *)text; text += 4; break;
   case DW_OP_const4s: stack[++i] = (uintptr_t)*(s32 *)text; text += 4; break;
   case DW_OP_const8u: stack[++i] = (uintptr_t)*(u32 *)text; text += 8; break; /* XXX: 64-bit? */
   case DW_OP_const8s: stack[++i] = (uintptr_t)*(u32 *)text; text += 8; break; /* XXX: 64-bit? */
   case DW_OP_constu:  stack[++i] = decode_uleb128((byte_t **)&text); break;
   case DW_OP_consts:  stack[++i] = (uintptr_t)decode_sleb128((byte_t **)&text); break;
   case DW_OP_dup:     stack[i+1] = stack[i]; ++i; break;
   case DW_OP_drop:    if unlikely(i == 0) GOTO_FAIL(err); --i; break;
   case DW_OP_over:    if unlikely(i == 0) GOTO_FAIL(err); stack[i+1] = stack[i-1]; ++i; break;
   {
    u8 index;
   case DW_OP_pick:
    index = *(u8 *)text;
    ++text;
    if (index > i) GOTO_FAIL(err);
    stack[i+1] = stack[i-index];
    ++i;
   } break;

   {
    uintptr_t temp;
   case DW_OP_swap:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp       = stack[i];
    stack[i]   = stack[i-1];
    stack[i-1] = temp;
   } break;

   {
    uintptr_t temp;
   case DW_OP_rot:
    if unlikely(i <= 1)
       GOTO_FAIL(err);
    temp = stack[i];
    stack[i] = stack[i-1];
    stack[i-1] = stack[i-2];
    stack[i-2] = temp;
   } break;

   {
    uintptr_t temp;
   case DW_OP_xderef:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp = stack[i--];
    if (flags & EH_FRESTRICT_USERSPACE)
        validate_readable((uintptr_t *)temp,sizeof(uintptr_t));
    stack[i] = *(uintptr_t *)temp;
   } break;
   case DW_OP_abs:
    if ((intptr_t)stack[i] < 0)
         stack[i] = (uintptr_t)(-(intptr_t)stack[i]);
    break;

   {
    uintptr_t temp;
   case DW_OP_and:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp      = stack[i--];
    stack[i] &= temp;
   } break;

   {
    uintptr_t temp;
   case DW_OP_minus:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp      = stack[i--];
    stack[i] -= temp;
   } break;

   {
    uintptr_t temp;
   case DW_OP_mul:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp = stack[i--];
    stack[i] *= temp;
   } break;

   case DW_OP_neg:
    stack[i] = (uintptr_t)(-(intptr_t)stack[i]);
    break;

   case DW_OP_not:
    stack[i] = ~stack[i];
    break;

   {
    uintptr_t temp;
   case DW_OP_or:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp      = stack[i--];
    stack[i] |= temp;
   } break;

   {
    uintptr_t temp;
   case DW_OP_plus:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp      = stack[i--];
    stack[i] += temp;
   } break;

   case DW_OP_plus_uconst:
    stack[i] += decode_uleb128((byte_t **)&text);
    break;

   {
    uintptr_t temp;
   case DW_OP_shl:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp     = stack[i--];
    stack[i] = stack[i] << temp;
   } break;

   {
    uintptr_t temp;
   case DW_OP_shr:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp     = stack[i--];
    stack[i] = stack[i] >> temp;
   } break;

   {
    uintptr_t temp1; intptr_t temp2;
   case DW_OP_shra:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp1 = stack[i--];
    temp2 = (intptr_t)stack[i];
    stack[i] = (uintptr_t)(temp2 >> temp1);
   } break;

   {
    uintptr_t temp;
   case DW_OP_xor:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp      = stack[i--];
    stack[i] ^= temp;
   } break;

   {
    s16 temp;
   case DW_OP_skip:
    temp = *(s16 *)text;
    text += 2;
    text += temp;
   } break;

   {
    s16 temp;
   case DW_OP_bra:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp = *(s16 *)text;
    text += 2;
    if (stack[i--] != 0)
        text += temp;
   } break;

   {
    uintptr_t temp;
   case DW_OP_eq:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp     = stack[i--];
    stack[i] = (stack[i] == temp) ? 1 : 0;
   } break;

   {
    uintptr_t temp;
   case DW_OP_ge:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp     = stack[i--];
    stack[i] = (stack[i] >= temp) ? 1 : 0;
   } break;

   {
    uintptr_t temp;
   case DW_OP_gt:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp     = stack[i--];
    stack[i] = (stack[i] > temp) ? 1 : 0;
   } break;

   {
    uintptr_t temp;
   case DW_OP_le:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp     = stack[i--];
    stack[i] = (stack[i] <= temp) ? 1 : 0;
   } break;

   {
    uintptr_t temp;
   case DW_OP_lt:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp     = stack[i--];
    stack[i] = (stack[i] < temp) ? 1 : 0;
   } break;

   {
    uintptr_t temp;
   case DW_OP_ne:
    if unlikely(i == 0)
       GOTO_FAIL(err);
    temp     = stack[i--];
    stack[i] = (stack[i] != temp) ? 1 : 0;
   } break;

   case DW_OP_lit0 ... DW_OP_lit31:
    stack[++i] = opcode - DW_OP_lit0;
    break;

#if UNWIND_NUM_REGISTERS >= 32
   case DW_OP_reg0  ... DW_OP_reg31:  stack[++i] = UNWIND_GET_REGISTER(context,opcode - DW_OP_reg0); break;
   case DW_OP_breg0 ... DW_OP_breg31: stack[++i] = UNWIND_GET_REGISTER(context,opcode - DW_OP_breg0)+decode_sleb128((byte_t **)&text); break;
#else
   case DW_OP_reg0  ... DW_OP_reg0+UNWIND_NUM_REGISTERS-1:  stack[++i] = UNWIND_GET_REGISTER(context,opcode - DW_OP_reg0); break;
   case DW_OP_breg0 ... DW_OP_breg0+UNWIND_NUM_REGISTERS-1: stack[++i] = UNWIND_GET_REGISTER(context,opcode - DW_OP_breg0)+decode_sleb128((byte_t **)&text); break;
#endif
   {
    u8 regno;
   case DW_OP_regx:
    regno = (u8)decode_uleb128((byte_t **)&text);
    if unlikely(regno >= UNWIND_NUM_REGISTERS)
       GOTO_FAIL(err);
    stack[++i] = UNWIND_GET_REGISTER(context,regno);
   } break;

   {
    u8 regno;
   case DW_OP_bregx:
    regno = (u8)decode_uleb128((byte_t **)&text);
    if unlikely(regno >= UNWIND_NUM_REGISTERS)
       GOTO_FAIL(err);
    stack[++i] = UNWIND_GET_REGISTER(context,regno);
    stack[i]  += decode_sleb128((byte_t **)&text);
   } break;

   case DW_OP_deref_size:
    if (flags & EH_FRESTRICT_USERSPACE)
        validate_readable((void *)stack[i],*text);
    switch (*text++) {
    case 1: stack[i] = (uintptr_t)*(u8 *)stack[i]; break;
    case 2: stack[i] = (uintptr_t)*(u16 *)stack[i]; break;
    case 4: stack[i] = (uintptr_t)*(u32 *)stack[i]; break;
    case 8: stack[i] = (uintptr_t)*(u64 *)stack[i]; break;
    default: GOTO_FAIL(err);
    }
    break;
   default: GOTO_FAIL(err);
   }
  }
 } CATCH_HANDLED (E_SEGFAULT) {
  return false;
 }
 *presult = stack[i];
 return true;
err:
 return false;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_UNWIND_EH_UNWIND_C */
