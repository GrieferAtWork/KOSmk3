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
#ifndef GUARD_KERNEL_I386_KOS_VM86_C
#define GUARD_KERNEL_I386_KOS_VM86_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/vm.h>
#include <kernel/debug.h>
#include <i386-kos/vm86.h>
#include <asm/cpu-flags.h>
#include <sys/io.h>
#include <stdbool.h>

#include "emulator.h"

#ifndef CONFIG_NO_VM86
DECL_BEGIN

PUBLIC struct vm_region vm86_identity_1mb = {
    .vr_refcnt = 0x3fffffff,
    .vr_lock   = MUTEX_INIT,
    .vr_type   = VM_REGION_MEM,
    /* Make sure to set the CANTSHARE flag to prevent user-space from
     * modifying the original by re-mapping as `PROT_SHARED' to disable
     * copy-on-write (Which is force-enabled when this flag is set). */
    .vr_flags  = VM_REGION_FDONTMERGE|VM_REGION_FCANTSHARE,
    .vr_size   = VM86_IDENTITY_1MB_SIZE,
    .vr_parts  = &vm86_identity_1mb.vr_part0,
    .vr_part0  = {
        .vp_refcnt = 0x3fffffff,
        .vp_chain  = { NULL, &vm86_identity_1mb.vr_parts },
        .vp_start  = 0,
        .vp_state  = VM_PART_INCORE,
        .vp_flags  = (VM_PART_FKEEP|VM_PART_FWEAKREF|VM_PART_FNOSWAP),
        .vp_phys   = {
            .py_num_scatter = 1,
            .py_iscatter    = {
                [0] = {
                    .ps_addr = 0, /* The _first_ 1 Mb */
                    .ps_size = VM86_IDENTITY_1MB_SIZE
                }
            }
        }
    }
};

PRIVATE u8 FCALL vm86_inb(u16 port) {
 u8 result = inb(port);
 debug_printf("[VM86] EMULATE: <inb $0x%.4I16x # -> 0x%.2I8x>\n",port,result);
 return result;
}
PRIVATE u16 FCALL vm86_inw(u16 port) {
 u16 result = inw(port);
 debug_printf("[VM86] EMULATE: <inw $0x%.4I16x # -> 0x%.4I16x>\n",port,result);
 return result;
}
PRIVATE u32 FCALL vm86_inl(u16 port) {
 u32 result = inl(port);
 debug_printf("[VM86] EMULATE: <inl $0x%.4I16x # -> 0x%.8I32x>\n",port,result);
 return result;
}

PRIVATE void FCALL vm86_outb(u16 port, u8 value) {
 debug_printf("[VM86] EMULATE: <outb $0x%.4I16x, $0x%.2I8x>\n",port,value);
 outb(port,value);
}
PRIVATE void FCALL vm86_outw(u16 port, u16 value) {
 debug_printf("[VM86] EMULATE: <outw $0x%.4I16x, $0x%.4I16x>\n",port,value);
 outw(port,value);
}
PRIVATE void FCALL vm86_outl(u16 port, u32 value) {
 debug_printf("[VM86] EMULATE: <outl $0x%.4I16x, $0x%.8I32x>\n",port,value);
 outl(port,value);
}


INTERN bool FCALL
vm86_gpf(struct cpu_context_vm86 *__restrict context,
         register_t error_code) {
#define F_OP32    F_OP16 /* The 0x66 prefix is being used. */
#define F_AD32    F_AD16 /* The 0x67 prefix is being used. */
 u16 flags = 0; byte_t *text; u32 opcode;
 text = (byte_t *)VM86_SEGMENT_ADDRESS(context->c_iret.ir_cs,
                                       context->c_eip);
next_byte:
 opcode = 0;
extend_instruction:
 opcode |= *text++;
 switch (opcode) {

  /* Prefix bytes */
 case 0x66: flags |= F_OP32; goto next_byte;
 case 0x67: flags |= F_AD32; goto next_byte;
 case 0xf0: flags |= F_LOCK; goto next_byte;
 case 0xf2: flags |= F_REPNE; goto next_byte;
 case 0xf3: flags |= F_REP; goto next_byte;
 case 0x26: flags = (flags & ~F_SEGMASK) | F_SEGES; goto next_byte;
 case 0x2e: flags = (flags & ~F_SEGMASK) | F_SEGCS; goto next_byte;
 case 0x36: flags = (flags & ~F_SEGMASK) | F_SEGSS; goto next_byte;
 case 0x3e: flags = (flags & ~F_SEGMASK) | F_SEGDS; goto next_byte;
 case 0x64: flags = (flags & ~F_SEGMASK) | F_SEGFS; goto next_byte;
 case 0x65: flags = (flags & ~F_SEGMASK) | F_SEGGS; goto next_byte;
 case 0x0f: opcode <<= 8; goto extend_instruction;

 {
  u8 intno;
  u32 segoff;
 case 0xcd:
  intno = *text++;
  debug_printf("[VM86] EMULATE: <int $0x%.2I8x>\n",intno);
  COMPILER_BARRIER();
  /* Read the interrupt target from the BIOS interrupt vector at 0x0000:0x0000 */
  segoff = *(u32 USER CHECKED *)(intno*4);
  COMPILER_BARRIER();
  debug_printf("[VM86] jump to %.4X:%.4X\n",segoff>>16,segoff&0xffff);
  /* Push values. */
  vm86_pushw(context,(u16)context->c_eflags);     /* flags */
  vm86_pushw(context,(u16)context->c_iret.ir_cs); /* cs */
  vm86_pushw(context,(u16)context->c_eip);        /* ip */
  context->c_iret.ir_cs      = (u16)(segoff >> 16);
  context->c_iret.ir_eip     = (u16)(segoff & 0xffff);
  context->c_iret.ir_eflags &= ~(EFLAGS_TF|EFLAGS_IF|EFLAGS_AC);
 } break;

 {
  u32 new_ip,new_cs,new_flags;
 case 0xcf:
  debug_printf("[VM86] EMULATE: <iret%s>\n",flags & F_OP32 ? "l" : "");
  if (flags & F_OP32) {
   new_ip    = vm86_popl(context);
   new_cs    = vm86_popl(context);
   new_flags = vm86_popl(context);
  } else {
   new_ip    = vm86_popw(context);
   new_cs    = vm86_popw(context);
   new_flags = vm86_popw(context);
  }
  context->c_eip             = new_ip;
  context->c_iret.ir_cs      = new_cs;
  context->c_iret.ir_eflags &= (EFLAGS_VM|EFLAGS_VIF|EFLAGS_VIP|EFLAGS_IOPL(3));
  context->c_iret.ir_eflags |= new_flags & (EFLAGS_CF|
                                            EFLAGS_PF|
                                            EFLAGS_AF|
                                            EFLAGS_ZF|
                                            EFLAGS_SF|
                                            EFLAGS_TF| /* XXX: System flag? */
                                            EFLAGS_IF| /* XXX: System flag? */
                                            EFLAGS_DF|
                                            EFLAGS_OF|
                                            EFLAGS_NT| /* XXX: System flag? */
                                            EFLAGS_RF| /* XXX: System flag? */
                                            EFLAGS_AC| /* XXX: System flag? */
                                            EFLAGS_ID  /* XXX: System flag? */
                                            );
 } break;

 case 0xfa:
  debug_printf("[VM86] EMULATE: <cli>\n");
  /* TODO */
  break;

 case 0xfb:
  debug_printf("[VM86] EMULATE: <sti>\n");
  /* TODO */
  break;

 case 0xe4:
  *(u8 *)&context->c_gpregs.gp_eax = vm86_inb(*text++);
  break;

 case 0xe5:
  if (flags & F_OP32) {
   context->c_gpregs.gp_eax = vm86_inl(*text++);
  } else {
   *(u16 *)&context->c_gpregs.gp_eax = vm86_inw(*text++);
  }
  break;

 case 0xe6:
  vm86_outb(*text++,(u8)context->c_gpregs.gp_eax);
  break;

 case 0xe7:
  if (flags & F_OP32) {
   vm86_outl(*text++,context->c_gpregs.gp_eax);
  } else {
   vm86_outw(*text++,(u16)context->c_gpregs.gp_eax);
  }
  break;

 case 0xec:
  *(u8 *)&context->c_gpregs.gp_eax = vm86_inb((u16)context->c_gpregs.gp_edx);
  break;

 case 0xed:
  if (flags & F_OP32) {
   context->c_gpregs.gp_eax = vm86_inl((u16)context->c_gpregs.gp_edx);
  } else {
   *(u16 *)&context->c_gpregs.gp_eax = vm86_inw((u16)context->c_gpregs.gp_edx);
  }
  break;

 case 0xee:
  vm86_outb((u16)context->c_gpregs.gp_edx,
            (u8)context->c_gpregs.gp_eax);
  break;

 case 0xef:
  if (flags & F_OP32) {
   vm86_outl((u16)context->c_gpregs.gp_edx,
                  context->c_gpregs.gp_eax);
  } else {
   vm86_outw((u16)context->c_gpregs.gp_edx,
             (u16)context->c_gpregs.gp_eax);
  }
  break;

 {
  u32 current_flags;
 case 0x9c:
  debug_printf("[VM86] EMULATE: <pushf%s>\n",flags & F_OP32 ? "l" : "");
  current_flags = context->c_iret.ir_eflags;
  if (flags & F_OP32) {
   vm86_pushl(context,current_flags);
  } else {
   vm86_pushw(context,(u16)current_flags);
  }
 } break;

 {
  u32 new_flags;
 case 0x9d:
  debug_printf("[VM86] EMULATE: <popf%s>\n",flags & F_OP32 ? "l" : "");
  if (flags & F_OP32) {
   new_flags = vm86_popl(context);
  } else {
   new_flags = vm86_popw(context);
  }
  context->c_iret.ir_eflags &= (EFLAGS_VM|EFLAGS_RF|EFLAGS_IOPL(3)|EFLAGS_VIP|EFLAGS_VIF);
  context->c_iret.ir_eflags |= new_flags & ~(EFLAGS_VM|EFLAGS_RF|EFLAGS_IOPL(3)|EFLAGS_VIP|EFLAGS_VIF);
  /* TODO: EFLAGS_IF */
 } break;


 default:
  debug_printf("[VM86] EMULATE: <0x%X> (UNSUPPORTED)\n",opcode);
  return false;
 }
 context->c_eip = (uintptr_t)text & 0xffff;
 return true;
}



DECL_END
#endif /* !CONFIG_NO_VM86 */

#endif /* !GUARD_KERNEL_I386_KOS_VM86_C */
