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
#ifndef GUARD_KERNEL_I386_KOS_VIO_C
#define GUARD_KERNEL_I386_KOS_VIO_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/section.h>
#include <kernel/interrupt.h>
#include <kernel/malloc.h>
#include <kernel/vm.h>
#include <i386-kos/interrupt.h>
#include <kos/i386-kos/context.h>
#include <kos/i386-kos/except.h>
#include <kos/intrin.h>
#include <asm/cpu-flags.h>
#include <kernel/debug.h>
#include <unwind/eh_frame.h>
#include <kos/context.h>
#include <except.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>

#include "emulator.h"

#ifndef CONFIG_NO_VIO

DECL_BEGIN

#ifdef __x86_64__
#define CONTEXT_IP(x)    ((x).c_rip)
#define CONTEXT_FLAGS(x) ((x).c_rflags)
#define CONTEXT_AREG(x)  ((x).c_gpregs.gp_rax)
#define CONTEXT_CREG(x)  ((x).c_gpregs.gp_rcx)
#define CONTEXT_DREG(x)  ((x).c_gpregs.gp_rdx)
#define CONTEXT_BREG(x)  ((x).c_gpregs.gp_rbx)
#define CONTEXT_DIREG(x) ((x).c_gpregs.gp_rdi)
#define CONTEXT_SIREG(x) ((x).c_gpregs.gp_rsi)
#else
#define CONTEXT_IP(x)    ((x).c_eip)
#define CONTEXT_FLAGS(x) ((x).c_eflags)
#define CONTEXT_AREG(x)  ((x).c_gpregs.gp_eax)
#define CONTEXT_CREG(x)  ((x).c_gpregs.gp_ecx)
#define CONTEXT_DREG(x)  ((x).c_gpregs.gp_edx)
#define CONTEXT_BREG(x)  ((x).c_gpregs.gp_ebx)
#define CONTEXT_DIREG(x) ((x).c_gpregs.gp_edi)
#define CONTEXT_SIREG(x) ((x).c_gpregs.gp_esi)
#endif
#define X86_GPREG(context,no)       ((uintptr_t *)&(context).c_gpregs)[7-(no)]
#define X86_GPREG8(context,no)      ((u8 *)&(context).c_gpregs+32)[7-(no)]
#define MODRM_REG(context,modrm)     X86_GPREG(*(context),(modrm)->mi_reg)
#define MODRM_REG8(context,modrm)    X86_GPREG8(*(context),(modrm)->mi_reg)


PRIVATE ATTR_NORETURN void KCALL vio_not_implemented(void) {
 error_throw(E_NOT_IMPLEMENTED);
}



union PACKED word16 {
    u16 w16;
    u8  w8[2];
};
union PACKED word32 {
    u32 w32;
    u8  w8[4];
    u16 w16[2];
    struct PACKED {
        u8  __pad1;
        u16   w16_unaligned_8;
    };
};
union PACKED word64 {
    u64 w64;
    u8  w8[8];
    u16 w16[4];
    u32 w32[2];
    struct PACKED {
        u8  __pad1;
        u32   w32_unaligned_8;
    };
    struct PACKED {
        u16 __pad2;
        u32   w32_unaligned_16;
    };
    struct PACKED {
        u8  __pad3;
        u16   w16_unaligned_8;
        union PACKED {
        u16   w16_unaligned_24;
        u32   w32_unaligned_24;
        };
    };
};


STATIC_ASSERT(offsetof(union word32,w16_unaligned_8)  == 1);
STATIC_ASSERT(offsetof(union word64,w32_unaligned_8)  == 1);
STATIC_ASSERT(offsetof(union word64,w32_unaligned_16) == 2);
STATIC_ASSERT(offsetof(union word64,w32_unaligned_24) == 3);


LOCAL u8 KCALL
vio_readb(struct vio_ops *__restrict ops,
          void *closure, uintptr_t addr) {
 if (ops->v_readb)
     return (*ops->v_readb)(closure,addr);
 if (ops->v_readw) {
  union word16 word;
  word.w16 = (*ops->v_readw)(closure,addr & ~1);
  return word.w8[addr & 1];
 }
 if (ops->v_readl) {
  union word32 word;
  word.w32 = (*ops->v_readl)(closure,addr & ~3);
  return word.w8[addr & 3];
 }
 vio_not_implemented();
}
LOCAL u16 KCALL
vio_readw(struct vio_ops *__restrict ops,
          void *closure, uintptr_t addr) {
 if (ops->v_readw && !(addr & 1))
     return (*ops->v_readw)(closure,addr);
 if (ops->v_readl) {
  union word32 word;
  word.w32 = (*ops->v_readl)(closure,addr & ~3);
  return (addr & 1) ? word.w16_unaligned_8 : word.w16[0];
 }
 if (ops->v_readw) {
  union word32 word;
  word.w16[0] = (*ops->v_readw)(closure,addr-1);
  word.w16[1] = (*ops->v_readw)(closure,addr+1);
  return word.w16_unaligned_8;
 }
 if (ops->v_readb) {
  union word16 word;
  word.w8[0] = (*ops->v_readb)(closure,addr);
  word.w8[1] = (*ops->v_readb)(closure,addr+1);
  return word.w16;
 }
 vio_not_implemented();
}
LOCAL u32 KCALL
vio_readl(struct vio_ops *__restrict ops,
          void *closure, uintptr_t addr) {
 if (ops->v_readl) {
  union word64 word;
  if (!(addr & 3)) return (*ops->v_readl)(closure,addr);
  word.w32[0] = (*ops->v_readl)(closure,addr & ~3);
  word.w32[1] = (*ops->v_readl)(closure,(addr & ~3)+4);
  switch (addr&3) {
  case 1: return word.w32_unaligned_8;
  case 2: return word.w32_unaligned_16;
  case 3: return word.w32_unaligned_24;
  default: __builtin_unreachable();
  }
 }
 if (ops->v_readw) {
  if (!(addr & 1)) {
   union word32 word;
   word.w16[0] = (*ops->v_readw)(closure,addr);
   word.w16[1] = (*ops->v_readw)(closure,addr+2);
   return word.w32;
  } else {
   union word64 word;
   word.w16[0] = (*ops->v_readw)(closure,addr-1);
   word.w16[1] = (*ops->v_readw)(closure,addr+1);
   word.w16[2] = (*ops->v_readw)(closure,addr+3);
   return word.w32_unaligned_8;
  }
 }
 if (ops->v_readb) {
  union word32 word;
  word.w8[0] = (*ops->v_readb)(closure,addr);
  word.w8[1] = (*ops->v_readb)(closure,addr+1);
  word.w8[2] = (*ops->v_readb)(closure,addr+2);
  word.w8[3] = (*ops->v_readb)(closure,addr+3);
  return word.w32;
 }
 vio_not_implemented();
}

LOCAL void KCALL
vio_writeb(struct vio_ops *__restrict ops,
          void *closure, uintptr_t addr, u8 value) {
 if (ops->v_writeb) {
  (*ops->v_writeb)(closure,addr,value);
  return;
 }
 if (ops->v_writew) {
  if (ops->v_readw) {
   union word16 word;
   word.w16 = (*ops->v_readw)(closure,addr & ~1);
   word.w8[addr & 1] = value;
   (*ops->v_writew)(closure,addr & ~1,word.w16);
   return;
  }
  if (ops->v_readb) {
   union word16 word;
   if (addr & 1) {
    word.w8[0] = (*ops->v_readb)(closure,addr-1);
    word.w8[1] = value;
   } else {
    word.w8[0] = value;
    word.w8[1] = (*ops->v_readb)(closure,addr+1);
   }
   (*ops->v_writew)(closure,addr & ~1,word.w16);
   return;
  }
  if (ops->v_readl) {
   union word32 word;
   word.w32 = (*ops->v_readl)(closure,addr & ~3);
   word.w8[addr & 3] = value;
   (*ops->v_writew)(closure,addr & ~1,word.w16[(addr & 2) >> 1]);
   return;
  }
 }
 if (ops->v_writel) {
  if (ops->v_readl) {
   union word32 word;
   word.w32 = (*ops->v_readl)(closure,addr & ~3);
   word.w8[addr & 3] = value;
   (*ops->v_writel)(closure,addr & ~3,word.w32);
   return;
  }
  if (ops->v_readw) {
   union word32 word;
   word.w16[0] = (*ops->v_readw)(closure,addr & ~3);
   word.w16[1] = (*ops->v_readw)(closure,(addr & ~3)+2);
   word.w8[addr & 3] = value;
   (*ops->v_writel)(closure,addr & ~3,word.w32);
   return;
  }
  if (ops->v_readb) {
   union word32 word;
   if ((addr & 3) != 0) word.w8[0] = (*ops->v_readb)(closure,addr & ~3);
   if ((addr & 3) != 1) word.w8[1] = (*ops->v_readb)(closure,(addr & ~3)+1);
   if ((addr & 3) != 2) word.w8[2] = (*ops->v_readb)(closure,(addr & ~3)+2);
   if ((addr & 3) != 3) word.w8[3] = (*ops->v_readb)(closure,(addr & ~3)+3);
   word.w8[addr & 3] = value;
   (*ops->v_writel)(closure,addr & ~3,word.w32);
   return;
  }
 }
 vio_not_implemented();
}
LOCAL void KCALL
vio_writew(struct vio_ops *__restrict ops,
          void *closure, uintptr_t addr, u16 value) {
 if (ops->v_writew && !(addr & 1)) {
  (*ops->v_writew)(closure,addr,value);
  return;
 }
 if (ops->v_writel) {
  if ((addr & 3) == 3) {
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-3);
    word.w32[1] = (*ops->v_readl)(closure,addr+1);
    word.w16_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-3);
    word.w16[1] = (*ops->v_readw)(closure,addr-1);
    word.w16[2] = (*ops->v_readw)(closure,addr+1);
    word.w16[3] = (*ops->v_readw)(closure,addr+3);
    word.w16_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-3);
    word.w8[1] = (*ops->v_readb)(closure,addr-2);
    word.w8[2] = (*ops->v_readb)(closure,addr-1);
    word.w8[3] = (*ops->v_readb)(closure,addr);
    word.w8[3] = (*ops->v_readb)(closure,addr+1);
    word.w8[4] = (*ops->v_readb)(closure,addr+2);
    word.w8[5] = (*ops->v_readb)(closure,addr+3);
    word.w8[6] = (*ops->v_readb)(closure,addr+4);
    word.w8[7] = (*ops->v_readb)(closure,addr+5);
    word.w16_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
  } else {
   if (ops->v_readl) {
    union word32 word;
    word.w32 = (*ops->v_readl)(closure,addr & ~3);
    switch (addr & 3) {
    case 0: word.w16[0]        = value; break;
    case 1: word.w16_unaligned_8 = value; break;
    case 2: word.w16[1]        = value; break;
    default: __builtin_unreachable();
    }
    (*ops->v_writel)(closure,addr & ~3,word.w32);
    return;
   }
   if (ops->v_readw) {
    union word32 word;
    word.w16[0] = (*ops->v_readw)(closure,addr & ~3);
    word.w16[1] = (*ops->v_readw)(closure,(addr & ~3)+2);
    switch (addr & 3) {
    case 0: word.w16[0]          = value; break;
    case 1: word.w16_unaligned_8 = value; break;
    case 2: word.w16[1]          = value; break;
    default: __builtin_unreachable();
    }
    (*ops->v_writel)(closure,addr & ~3,word.w32);
    return;
   }
   if (ops->v_readb) {
    union word32 word;
    switch (addr & 3) {
    case 0:
     word.w16[0] = value;
     word.w8[2] = (*ops->v_readb)(closure,(addr & ~3)+2);
     word.w8[3] = (*ops->v_readb)(closure,(addr & ~3)+3);
     break;
    case 1:
     word.w8[0] = (*ops->v_readb)(closure,(addr & ~3)+0);
     word.w16_unaligned_8 = value;
     word.w8[3] = (*ops->v_readb)(closure,(addr & ~3)+3);
     break;
    case 2:
     word.w8[0] = (*ops->v_readb)(closure,(addr & ~3)+0);
     word.w8[1] = (*ops->v_readb)(closure,(addr & ~3)+1);
     word.w16[1] = value;
     break;
    default: __builtin_unreachable();
    }
    (*ops->v_writel)(closure,addr & ~3,word.w32);
    return;
   }
  }
 }
 if (ops->v_writeb) {
  union word16 word;
  word.w16 = value;
  (*ops->v_writeb)(closure,addr,word.w8[0]);
  (*ops->v_writeb)(closure,addr+1,word.w8[1]);
  return;
 }
 vio_not_implemented();
}
LOCAL void KCALL
vio_writel(struct vio_ops *__restrict ops,
          void *closure, uintptr_t addr, u32 value) {
 if (ops->v_writel) {
  if (!(addr & 3)) {
   (*ops->v_writel)(closure,addr,value);
   return;
  }
  if (ops->v_readl) {
   union word64 word;
   word.w32[0] = (*ops->v_readl)(closure,addr-3);
   word.w32[1] = (*ops->v_readl)(closure,addr+1);
   switch (addr & 3) {
   case 1: word.w32_unaligned_8  = value; break;
   case 2: word.w32_unaligned_16 = value; break;
   case 3: word.w32_unaligned_24 = value; break;
   default: __builtin_unreachable();
   }
   (*ops->v_writel)(closure,addr-3,word.w32[0]);
   (*ops->v_writel)(closure,addr+1,word.w32[1]);
   return;
  }
  if (ops->v_readw) {
   union word64 word;
   word.w16[0] = (*ops->v_readw)(closure,addr-3);
   word.w16[1] = (*ops->v_readw)(closure,addr-1);
   word.w16[2] = (*ops->v_readw)(closure,addr+1);
   word.w16[3] = (*ops->v_readw)(closure,addr+3);
   switch (addr & 3) {
   case 1: word.w32_unaligned_8  = value; break;
   case 2: word.w32_unaligned_16 = value; break;
   case 3: word.w32_unaligned_24 = value; break;
   default: __builtin_unreachable();
   }
   (*ops->v_writel)(closure,addr-3,word.w32[0]);
   (*ops->v_writel)(closure,addr+1,word.w32[1]);
   return;
  }
  if (ops->v_readb) {
   union word64 word;
   word.w8[0] = (*ops->v_readw)(closure,addr-3);
   word.w8[1] = (*ops->v_readw)(closure,addr-2);
   word.w8[2] = (*ops->v_readw)(closure,addr-1);
   word.w8[3] = (*ops->v_readw)(closure,addr);
   word.w8[3] = (*ops->v_readw)(closure,addr+1);
   word.w8[4] = (*ops->v_readw)(closure,addr+2);
   word.w8[5] = (*ops->v_readw)(closure,addr+3);
   word.w8[6] = (*ops->v_readw)(closure,addr+4);
   word.w8[7] = (*ops->v_readw)(closure,addr+5);
   switch (addr & 3) {
   case 1: word.w32_unaligned_8  = value; break;
   case 2: word.w32_unaligned_16 = value; break;
   case 3: word.w32_unaligned_24 = value; break;
   default: __builtin_unreachable();
   }
   (*ops->v_writel)(closure,addr-3,word.w32[0]);
   (*ops->v_writel)(closure,addr+1,word.w32[1]);
   return;
  }
 }
 if (ops->v_writew) {
  if (!(addr & 1)) {
   union word32 word;
   word.w32 = value;
   (*ops->v_writew)(closure,addr+0,word.w16[0]);
   (*ops->v_writew)(closure,addr+2,word.w16[1]);
   return;
  } else {
   if (ops->v_readw) {
    union word64 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-1);
    word.w16[2] = (*ops->v_readw)(closure,addr+3);
    word.w32_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    (*ops->v_writew)(closure,addr+3,word.w16[2]);
    return;
   }
   if ((addr & 3) == 1) {
    if (ops->v_readl) {
     union word64 word;
     word.w32[0] = (*ops->v_readl)(closure,addr-1);
     word.w32[1] = (*ops->v_readl)(closure,addr+3);
     word.w32_unaligned_8 = value;
     (*ops->v_writew)(closure,addr-1,word.w16[0]);
     (*ops->v_writew)(closure,addr+1,word.w16[1]);
     (*ops->v_writew)(closure,addr+3,word.w16[2]);
     return;
    }
    if (ops->v_readb) {
     union word64 word;
     word.w8[0] = (*ops->v_readb)(closure,addr-1);
     word.w8[5] = (*ops->v_readb)(closure,addr+4);
     word.w32_unaligned_8 = value;
     (*ops->v_writew)(closure,addr-1,word.w16[0]);
     (*ops->v_writew)(closure,addr+1,word.w16[1]);
     (*ops->v_writew)(closure,addr+3,word.w16[2]);
     return;
    }
   } else {
    assert((addr & 3) == 3);
    if (ops->v_readl) {
     union word64 word;
     word.w32[0] = (*ops->v_readl)(closure,addr-3);
     word.w32[1] = (*ops->v_readl)(closure,addr+1);
     word.w32_unaligned_24 = value;
     (*ops->v_writew)(closure,addr-1,word.w16[1]);
     (*ops->v_writew)(closure,addr+1,word.w16[2]);
     (*ops->v_writew)(closure,addr+3,word.w16[3]);
     return;
    }
    if (ops->v_readb) {
     union word64 word;
     word.w8[2] = (*ops->v_readb)(closure,addr-1);
     word.w8[7] = (*ops->v_readb)(closure,addr+4);
     word.w32_unaligned_24 = value;
     (*ops->v_writew)(closure,addr-1,word.w16[1]);
     (*ops->v_writew)(closure,addr+1,word.w16[2]);
     (*ops->v_writew)(closure,addr+3,word.w16[3]);
     return;
    }
   }
  }
 }
 if (ops->v_writeb) {
  union word32 word;
  word.w32 = value;
  (*ops->v_writeb)(closure,addr+0,word.w8[0]);
  (*ops->v_writeb)(closure,addr+1,word.w8[1]);
  (*ops->v_writeb)(closure,addr+2,word.w8[2]);
  (*ops->v_writeb)(closure,addr+3,word.w8[3]);
  return;
 }
 vio_not_implemented();
}


LOCAL u8 KCALL
vio_cmpxchb(struct vio_ops *__restrict ops, void *closure,
            uintptr_t addr, u8 oldval, u8 newval) {
 if (ops->v_atomic_cmpxchgb)
     return (*ops->v_atomic_cmpxchgb)(closure,addr,oldval,newval);
 if (ops->v_atomic_cmpxchgw) {
  union word16 word,new_word,old_word;
  if (addr & 1) {
   word.w8[1] = oldval,new_word.w8[1] = newval;
   do {
    word.w8[0]     = vio_readb(ops,closure,addr-1);
    new_word.w8[0] = word.w8[0];
    old_word.w16 = (*ops->v_atomic_cmpxchgw)(closure,addr-1,word.w16,new_word.w16);
   } while (old_word.w8[0] != word.w8[0]);
   return word.w8[1];
  } else {
   word.w8[0] = oldval,new_word.w8[0] = newval;
   do {
    word.w8[1]     = vio_readb(ops,closure,addr+1);
    new_word.w8[1] = word.w8[1];
    old_word.w16 = (*ops->v_atomic_cmpxchgw)(closure,addr,word.w16,new_word.w16);
   } while (old_word.w8[1] != word.w8[1]);
   return word.w8[0];
  }
 }
 if (ops->v_atomic_cmpxchgl) {
  union word32 word,new_word,old_word;
  unsigned int byte_addr = addr & 3;
  u32 byte_mask = ~((u32)0xff << (byte_addr*8));
  addr &= ~3;
  do {
   word.w32               = vio_readl(ops,closure,addr);
   new_word.w32           = word.w32;
   word.w8[byte_addr]     = oldval;
   new_word.w8[byte_addr] = newval;
   old_word.w32 = (*ops->v_atomic_cmpxchgl)(closure,addr,word.w32,new_word.w32);
  } while ((old_word.w32&byte_mask) != (word.w32&byte_mask));
  return word.w8[byte_addr];
 }
 vio_not_implemented();
}
LOCAL u16 KCALL
vio_cmpxchw(struct vio_ops *__restrict ops, void *closure,
            uintptr_t addr, u16 oldval, u16 newval) {
 if (ops->v_atomic_cmpxchgw && !(addr & 1))
     return (*ops->v_atomic_cmpxchgw)(closure,addr,oldval,newval);
 if (ops->v_atomic_cmpxchgl && (addr & 3) != 3) {
  union word32 word,new_word,old_word;
  u32 byte_mask;
  switch (addr&3) {
  case 0: byte_mask = 0xffff0000; break;
  case 1: byte_mask = 0x00ffff00; break;
  case 2: byte_mask = 0x0000ffff; break;
  default: __builtin_unreachable();
  }
  do {
   word.w32               = vio_readl(ops,closure,addr&~3);
   new_word.w32           = word.w32;
   switch (addr&3) {
   case 0: word.w16[0] = oldval; new_word.w16[0] = newval; break;
   case 1: word.w16_unaligned_8 = oldval; new_word.w16_unaligned_8 = newval; break;
   case 2: word.w16[1] = oldval; new_word.w16[1] = newval; break;
   default: __builtin_unreachable();
   }
   old_word.w32 = (*ops->v_atomic_cmpxchgl)(closure,addr,word.w32,new_word.w32);
  } while ((old_word.w32&byte_mask) != (word.w32&byte_mask));
  switch (addr&3) {
  case 0: return word.w16[0];
  case 1: return word.w16_unaligned_8;
  case 2: return word.w16[1];
  default: __builtin_unreachable();
  }
 }
 vio_not_implemented();
}
LOCAL u32 KCALL
vio_cmpxchl(struct vio_ops *__restrict ops, void *closure,
            uintptr_t addr, u32 oldval, u32 newval) {
 if (ops->v_atomic_cmpxchgl && !(addr & 3))
     return (*ops->v_atomic_cmpxchgl)(closure,addr,oldval,newval);
 vio_not_implemented();
}


#define GPARG_u8  "mq"
#define GPARG_u16 "g"
#define GPARG_u32 "g"
#define GPARG(x) GPARG_##x
#define GRARG_u8  "q"
#define GRARG_u16 "r"
#define GRARG_u32 "r"
#define GRARG(x) GRARG_##x


#define DEFINE_UNARY_PROXY_W(name,T,width) \
LOCAL register_t KCALL \
vio_##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr) \
{ \
 T value; register_t result; \
 value = vio_read##width(ops,closure,addr); \
 __asm__ __volatile__(#name #width " %0; pushfl\npopl %1" \
                      : "+" GPARG(T) (value) \
                      , "=g" (result) \
                      : : "cc"); \
 vio_write##width(ops,closure,addr,value); \
 return result; \
} \
LOCAL register_t KCALL \
vio_atomic_##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr) \
{ \
 T value,new_value; register_t result; \
 do { \
  value = vio_read##width(ops,closure,addr); \
  new_value = value; \
  __asm__ __volatile__(#name #width " %0; pushfl\npopl %1" \
                       : "+" GPARG(T) (new_value) \
                       , "=g" (result) \
                       : : "cc"); \
 } while (vio_cmpxch##width(ops,closure,addr,value,new_value) != value); \
 return result; \
}
#define DEFINE_VUNARY_PROXY_W(name,T,width) \
LOCAL void KCALL \
vio_##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr) \
{ \
 T value; \
 value = vio_read##width(ops,closure,addr); \
 __asm__ __volatile__(#name #width " %0" \
                      : "+" GPARG(T) (value) \
                      : \
                      : "cc"); \
 vio_write##width(ops,closure,addr,value); \
} \
LOCAL void KCALL \
vio_atomic_##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr) \
{ \
 T value,new_value; \
 do { \
  value = vio_read##width(ops,closure,addr); \
  new_value = value; \
  __asm__ __volatile__(#name #width " %0" \
                       : "+g" (new_value) \
                       : : "cc"); \
 } while (vio_cmpxch##width(ops,closure,addr,value,new_value) != value); \
}

#define DEFINE_LBINARY_PROXY_W(name,T,width) \
LOCAL register_t KCALL \
vio_l##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr, T *__restrict plhs) \
{ \
 T value; register_t result; \
 value = vio_read##width(ops,closure,addr); \
 __asm__ __volatile__(#name #width " %2, %0; pushfl\npopl %1" \
                      : "+" GPARG(T) (*plhs) \
                      , "=g" (result) \
                      : GRARG(T) (value) \
                      : "cc"); \
 return result; \
}
#define DEFINE_LBINARYRO_PROXY_W(name,T,width) \
LOCAL register_t KCALL \
vio_l##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr, T lhs) \
{ \
 T value; register_t result; \
 value = vio_read##width(ops,closure,addr); \
 __asm__ __volatile__(#name #width " %1, %2; pushfl\npopl %0" \
                      : "=g" (result) \
                      : GRARG(T) (value) \
                      , GPARG(T) (lhs) \
                      : "cc"); \
 return result; \
}
#define DEFINE_RBINARY_PROXY_W(name,T,width) \
LOCAL register_t KCALL \
vio_r##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr, T rhs) \
{ \
 T value; register_t result; \
 value = vio_read##width(ops,closure,addr); \
 __asm__ __volatile__(#name #width " %2, %0; pushfl\npopl %1" \
                      : "+" GPARG(T) (value) \
                      , "=g" (result) \
                      : GRARG(T) (rhs) \
                      : "cc"); \
 vio_write##width(ops,closure,addr,value); \
 return result; \
} \
LOCAL register_t KCALL \
vio_atomic_r##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr, T rhs) \
{ \
 T value,new_value; register_t result; \
 do { \
  value = new_value = vio_read##width(ops,closure,addr); \
  __asm__ __volatile__(#name #width " %2, %0; pushfl\npopl %1" \
                       : "+" GPARG(T) (new_value) \
                       , "=g" (result) \
                       : GRARG(T) (rhs) \
                       : "cc"); \
 } while (vio_cmpxch##width(ops,closure,addr,value,new_value) != value); \
 return result; \
}
#define DEFINE_LRBINARY_PROXY_W(name,T,width) \
LOCAL register_t KCALL \
vio_r##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr, T *__restrict prhs) \
{ \
 T value; register_t result; \
 value = vio_read##width(ops,closure,addr); \
 __asm__ __volatile__(#name #width " %2, %0; pushfl\npopl %1" \
                      : "+" GPARG(T) (value) \
                      , "=g" (result) \
                      , "+" GRARG(T) (*prhs) \
                      : \
                      : "cc"); \
 vio_write##width(ops,closure,addr,value); \
 return result; \
} \
LOCAL register_t KCALL \
vio_atomic_r##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr, T *__restrict prhs) \
{ \
 T value,new_value; register_t result; \
 do { \
  value = new_value = vio_read##width(ops,closure,addr); \
  __asm__ __volatile__(#name #width " %2, %0; pushfl\npopl %1" \
                       : "+" GPARG(T) (new_value) \
                       , "=g" (result) \
                       , "+" GRARG(T) (*prhs) \
                       : \
                       : "cc"); \
 } while (vio_cmpxch##width(ops,closure,addr,value,new_value) != value); \
 return result; \
}
#define DEFINE_VLRBINARY_PROXY_W(name,T,width) \
LOCAL void KCALL \
vio_r##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr, T *__restrict prhs) \
{ \
 T value; \
 value = vio_read##width(ops,closure,addr); \
 __asm__ __volatile__(#name #width " %1, %0" \
                      : "+" GPARG(T) (value) \
                      , "+" GRARG(T) (*prhs) \
                      : \
                      : "cc"); \
 vio_write##width(ops,closure,addr,value); \
} \
LOCAL void KCALL \
vio_atomic_r##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr, T *__restrict prhs) \
{ \
 T value,new_value; \
 do { \
  value = new_value = vio_read##width(ops,closure,addr); \
  __asm__ __volatile__(#name #width " %1, %0" \
                       : "+" GPARG(T) (new_value) \
                       , "+" GRARG(T) (*prhs) \
                       : \
                       : "cc"); \
 } while (vio_cmpxch##width(ops,closure,addr,value,new_value) != value); \
}
#define DEFINE_RBINARYRO_PROXY_W(name,T,width) \
LOCAL register_t KCALL \
vio_r##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr, T rhs) \
{ \
 T value; register_t result; \
 value = vio_read##width(ops,closure,addr); \
 __asm__ __volatile__(#name #width " %2, %1; pushfl\npopl %0" \
                      : "=g" (result) \
                      : GPARG(T) (value) \
                      , GRARG(T) (rhs) \
                      : "cc"); \
 return result; \
}

#define DEFINE_UNARY_PROXY(name)     DEFINE_UNARY_PROXY_W(name,u8,b) DEFINE_UNARY_PROXY_W(name,u16,w) DEFINE_UNARY_PROXY_W(name,u32,l)
#define DEFINE_VUNARY_PROXY(name)    DEFINE_VUNARY_PROXY_W(name,u8,b) DEFINE_VUNARY_PROXY_W(name,u16,w) DEFINE_VUNARY_PROXY_W(name,u32,l)
#define DEFINE_LBINARY_PROXY(name)   DEFINE_LBINARY_PROXY_W(name,u8,b) DEFINE_LBINARY_PROXY_W(name,u16,w) DEFINE_LBINARY_PROXY_W(name,u32,l)
#define DEFINE_LBINARYRO_PROXY(name) DEFINE_LBINARYRO_PROXY_W(name,u8,b) DEFINE_LBINARYRO_PROXY_W(name,u16,w) DEFINE_LBINARYRO_PROXY_W(name,u32,l)
#define DEFINE_RBINARY_PROXY(name)   DEFINE_RBINARY_PROXY_W(name,u8,b) DEFINE_RBINARY_PROXY_W(name,u16,w) DEFINE_RBINARY_PROXY_W(name,u32,l)
#define DEFINE_LRBINARY_PROXY(name)  DEFINE_LRBINARY_PROXY_W(name,u8,b) DEFINE_LRBINARY_PROXY_W(name,u16,w) DEFINE_LRBINARY_PROXY_W(name,u32,l)
#define DEFINE_VLRBINARY_PROXY(name) DEFINE_VLRBINARY_PROXY_W(name,u8,b) DEFINE_VLRBINARY_PROXY_W(name,u16,w) DEFINE_VLRBINARY_PROXY_W(name,u32,l)
#define DEFINE_RBINARYRO_PROXY(name) DEFINE_RBINARYRO_PROXY_W(name,u8,b) DEFINE_RBINARYRO_PROXY_W(name,u16,w) DEFINE_RBINARYRO_PROXY_W(name,u32,l)

DEFINE_UNARY_PROXY(inc)
DEFINE_UNARY_PROXY(dec)
DEFINE_VUNARY_PROXY(not)
DEFINE_UNARY_PROXY(neg)
DEFINE_RBINARY_PROXY(add) DEFINE_LBINARY_PROXY(add)
DEFINE_RBINARY_PROXY(sub) DEFINE_LBINARY_PROXY(sub)
DEFINE_RBINARY_PROXY(and) DEFINE_LBINARY_PROXY(and)
DEFINE_RBINARY_PROXY(or)  DEFINE_LBINARY_PROXY(or)
DEFINE_RBINARY_PROXY(xor) DEFINE_LBINARY_PROXY(xor)
DEFINE_VLRBINARY_PROXY(xchg)
DEFINE_LRBINARY_PROXY(xadd)
DEFINE_LBINARYRO_PROXY(cmp)
DEFINE_RBINARYRO_PROXY(cmp)
DEFINE_LBINARYRO_PROXY(test)
DEFINE_RBINARYRO_PROXY(test)
DEFINE_RBINARYRO_PROXY(bt)
DEFINE_RBINARY_PROXY(btc)
DEFINE_RBINARY_PROXY(btr)
DEFINE_RBINARY_PROXY(bts)

#define DEFINE_SHIFT_PROXY_W(name,T,width) \
LOCAL register_t KCALL \
vio_r##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr, T rhs) \
{ \
 T value; register_t result; \
 value = vio_read##width(ops,closure,addr); \
 __asm__ __volatile__(#name #width " %b2, %0; pushfl\npopl %1" \
                      : "+" GPARG(T) (value) \
                      , "=g" (result) \
                      : "ic" (rhs) \
                      : "cc"); \
 vio_write##width(ops,closure,addr,value); \
 return result; \
} \
LOCAL register_t KCALL \
vio_atomic_r##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr, T rhs) \
{ \
 T value,new_value; register_t result; \
 do { \
  value = new_value = vio_read##width(ops,closure,addr); \
  __asm__ __volatile__(#name #width " %b2, %0; pushfl\npopl %1" \
                       : "+" GPARG(T) (new_value) \
                       , "=g" (result) \
                       : "ic" (rhs) \
                       : "cc"); \
 } while (vio_cmpxch##width(ops,closure,addr,value,new_value) != value); \
 return result; \
}
#define DEFINE_SHIFT_PROXY(name) \
   DEFINE_SHIFT_PROXY_W(name,u8,b) \
   DEFINE_SHIFT_PROXY_W(name,u16,w) \
   DEFINE_SHIFT_PROXY_W(name,u32,l)

DEFINE_SHIFT_PROXY(shl)
DEFINE_SHIFT_PROXY(shr)
DEFINE_SHIFT_PROXY(sar)
DEFINE_SHIFT_PROXY(rol)
DEFINE_SHIFT_PROXY(ror)

#define DEFINE_RCX_PROXY_W(name,T,width) \
LOCAL register_t KCALL \
vio_r##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr, bool cf, T rhs) \
{ \
 T value; register_t result; \
 value = vio_read##width(ops,closure,addr); \
 __asm__ __volatile__("clc; cmp $0, %3; je 1f; stc; 1:\n" \
                      #name #width " %b2, %0; pushfl\npopl %1" \
                      : "+" GPARG(T) (value) \
                      , "=g" (result) \
                      : "ic" (rhs) \
                      , "mq" (cf) \
                      : "cc"); \
 vio_write##width(ops,closure,addr,value); \
 return result; \
} \
LOCAL register_t KCALL \
vio_atomic_r##name##width(struct vio_ops *__restrict ops, void *closure, uintptr_t addr, bool cf, T rhs) \
{ \
 T value,new_value; register_t result; \
 do { \
  value = new_value = vio_read##width(ops,closure,addr); \
  __asm__ __volatile__("clc; cmp $0, %3; je 1f; stc; 1:\n" \
                      #name #width " %b2, %0; pushfl\npopl %1" \
                       : "+" GPARG(T) (new_value) \
                       , "=" GPARG(T) (result) \
                       : "ic" (rhs) \
                       , "mq" (cf) \
                       : "cc"); \
 } while (vio_cmpxch##width(ops,closure,addr,value,new_value) != value); \
 return result; \
}
#define DEFINE_RCX_PROXY(name) \
   DEFINE_RCX_PROXY_W(name,u8,b) \
   DEFINE_RCX_PROXY_W(name,u16,w) \
   DEFINE_RCX_PROXY_W(name,u32,l)
DEFINE_RCX_PROXY(rcl)
DEFINE_RCX_PROXY(rcr)




/* Compare operands and return EFLAGS. */
LOCAL register_t KCALL x86_cmpb(u8 a, u8 b) {
 register_t result;
 __asm__("cmpb %b2, %b1\n"
         "pushfl\n"
         "popl %0\n"
         : "=g" (result)
         : "q" (a)
         , "q" (b)
         : "cc");
 return result;
}
LOCAL register_t KCALL x86_cmpw(u16 a, u16 b) {
 register_t result;
 __asm__("cmpw %w2, %w1\n"
         "pushfl\n"
         "popl %0\n"
         : "=g" (result)
         : "q" (a)
         , "q" (b)
         : "cc");
 return result;
}
LOCAL register_t KCALL x86_cmpl(u32 a, u32 b) {
 register_t result;
 __asm__("cmpl %2, %1\n"
         "pushfl\n"
         "popl %0\n"
         : "=g" (result)
         : "r" (a)
         , "r" (b)
         : "cc");
 return result;
}
LOCAL register_t KCALL x86_mulb(u8 x, u8 y, u16 *__restrict pres) {
 register_t result;
 __asm__("mulb %3; pushfl\npopl %1"
         : "=a" (*pres)
         , "=g" (result)
         : "0" (x)
         , "g" (y)
         : "cc");
 return result;
}
LOCAL register_t KCALL x86_mulw(u16 x, u16 y,
                                u16 *__restrict pres_a,
                                u16 *__restrict pres_b) {
 register_t result;
 __asm__("mulw %4; pushfl\npopl %2"
         : "=a" (*pres_a)
         , "=d" (*pres_b)
         , "=g" (result)
         : "0" (x)
         , "g" (y)
         : "cc");
 return result;
}
LOCAL register_t KCALL x86_mull(u32 x, u32 y,
                                u32 *__restrict pres_a,
                                u32 *__restrict pres_b) {
 register_t result;
 __asm__("mull %4; pushfl\npopl %2"
         : "=a" (*pres_a)
         , "=d" (*pres_b)
         , "=g" (result)
         : "0" (x)
         , "g" (y)
         : "cc");
 return result;
}
LOCAL register_t KCALL x86_imulb(u8 x, u8 y, u16 *__restrict pres) {
 register_t result;
 __asm__("imulb %3; pushfl\npopl %1"
         : "=a" (*pres)
         , "=g" (result)
         : "0" (x)
         , "g" (y)
         : "cc");
 return result;
}
LOCAL register_t KCALL x86_imulw(u16 x, u16 y,
                                 u16 *__restrict pres_a,
                                 u16 *__restrict pres_b) {
 register_t result;
 __asm__("imulw %4; pushfl\npopl %2"
         : "=a" (*pres_a)
         , "=d" (*pres_b)
         , "=g" (result)
         : "0" (x)
         , "g" (y)
         : "cc");
 return result;
}
LOCAL register_t KCALL x86_imull(u32 x, u32 y,
                                 u32 *__restrict pres_a,
                                 u32 *__restrict pres_b) {
 register_t result;
 __asm__("imull %4; pushfl\npopl %2"
         : "=a" (*pres_a)
         , "=d" (*pres_b)
         , "=g" (result)
         : "0" (x)
         , "g" (y)
         : "cc");
 return result;
}
LOCAL void KCALL x86_divb(u16 x, u8 y,
                          u8 *__restrict pquot_rem) {
 __asm__("divb %2"
         : "=a" (*pquot_rem)
         : "0" (x)
         , "g" (y)
         : "cc");
}
LOCAL void KCALL x86_divw(u16 x0, u16 x1, u16 y,
                          u16 *__restrict pquot,
                          u16 *__restrict prem) {
 __asm__("divw %2"
         : "=d" (*prem)
         , "=a" (*pquot)
         : "0" (x0)
         , "1" (x1)
         , "g" (y)
         : "cc");
}
LOCAL void KCALL x86_divl(u32 x0, u32 x1, u32 y,
                          u32 *__restrict pquot,
                          u32 *__restrict prem) {
 __asm__("divl %2"
         : "=d" (*prem)
         , "=a" (*pquot)
         : "0" (x0)
         , "1" (x1)
         , "g" (y)
         : "cc");
}
LOCAL void KCALL x86_idivb(u16 x, u8 y,
                           u8 *__restrict pquot_rem) {
 __asm__("idivb %2"
         : "=a" (*pquot_rem)
         : "0" (x)
         , "g" (y)
         : "cc");
}
LOCAL void KCALL x86_idivw(u16 x0, u16 x1, u16 y,
                           u16 *__restrict pquot,
                           u16 *__restrict prem) {
 __asm__("idivw %2"
         : "=d" (*prem)
         , "=a" (*pquot)
         : "0" (x0)
         , "1" (x1)
         , "g" (y)
         : "cc");
}
LOCAL void KCALL x86_idivl(u32 x0, u32 x1, u32 y,
                           u32 *__restrict pquot,
                           u32 *__restrict prem) {
 __asm__("idivl %2"
         : "=d" (*prem)
         , "=a" (*pquot)
         : "0" (x0)
         , "1" (x1)
         , "g" (y)
         : "cc");
}
LOCAL register_t KCALL x86_bsfw(u16 x, u16 *__restrict presult) {
 register_t result;
 __asm__("bsfw %2, %0; pushfl\npopl %1"
         : "=r" (*presult)
         , "=g" (result)
         : "g" (x)
         : "cc");
 return result;
}
LOCAL register_t KCALL x86_bsfl(u32 x, u32 *__restrict presult) {
 register_t result;
 __asm__("bsfl %2, %0; pushfl\npopl %1"
         : "=r" (*presult)
         , "=g" (result)
         : "g" (x)
         : "cc");
 return result;
}
LOCAL register_t KCALL x86_bsrw(u16 x, u16 *__restrict presult) {
 register_t result;
 __asm__("bsrw %2, %0; pushfl\npopl %1"
         : "=r" (*presult)
         , "=g" (result)
         : "g" (x)
         : "cc");
 return result;
}
LOCAL register_t KCALL x86_bsrl(u32 x, u32 *__restrict presult) {
 register_t result;
 __asm__("bsrl %2, %0; pushfl\npopl %1"
         : "=r" (*presult)
         , "=g" (result)
         : "g" (x)
         : "cc");
 return result;
}



#define CF EFLAGS_CF
#define PF EFLAGS_PF
#define AF EFLAGS_AF
#define ZF EFLAGS_ZF
#define SF EFLAGS_SF
#define TF EFLAGS_TF
#define IF EFLAGS_IF
#define DF EFLAGS_DF
#define OF EFLAGS_OF


INTERN bool KCALL
x86_handle_vio(struct cpu_anycontext *__restrict context,
               struct vm_region *__restrict region,
               uintptr_t addr, VIRT void *abs_addr) {
#define F_OP16  0x0001 /* The 0x66 prefix is being used. */
#define F_AD16  0x0002 /* The 0x67 prefix is being used. */
#define F_LOCK  0x0004 /* The `lock' prefix is being used. */
#define F_REPNE 0x0010 /* The `repne' prefix is being used. */
#define F_REP   0x0020 /* The `rep' prefix is being used. */
 u16 flags = 0; byte_t *text; u32 opcode;
 struct modrm_info modrm;
 struct vio_ops *ops = region->vr_setup.s_vio.v_ops;
 void *closure = region->vr_setup.s_vio.v_closure;
 text = (byte_t *)CPU_CONTEXT_IP(*context);
next_byte:
 opcode = 0;
extend_instruction:
 TRY opcode |= *text++;
 CATCH_HANDLED (E_SEGFAULT) {
  error_handled();
  goto fail;
 }
#if 0
 debug_printf("OPCODE = %I32X (%p)\n",opcode,text-1);
#endif

 /* Instruction emulator used to bypass regular memory operations
  * and forward everything to our own VIO operator callbacks.
  * NOTE: Opcode numbers and flag effects are taken from reference
  *       material found online, most notably the following page:
  *     - https://c9x.me/x86/
  */
 switch (opcode) {

 case 0x66: flags |= F_OP16; goto next_byte;
 case 0x67: flags |= F_AD16; goto next_byte;
 case 0xf0: flags |= F_LOCK; goto next_byte;
 case 0xf2: flags |= F_REPNE; goto next_byte;
 case 0xf3: flags |= F_REP; goto next_byte;
 case 0x26: case 0x2e: case 0x36:
 case 0x3e: case 0x64: case 0x65:
  /* Segment prefix... */
  goto next_byte;
 case 0x0f: opcode <<= 8; goto extend_instruction;

 {
  u8 oldval,value;
 case 0x0fb0:
  /* cmpxchg r/m8,r8 */
  text   = x86_decode_modrm(text,&modrm);
  oldval = (u8)CONTEXT_AREG(*context);
  value  = vio_cmpxchb(ops,closure,addr,oldval,
                       MODRM_REG8(context,&modrm));
  context->c_eflags &= ~(ZF|CF|PF|AF|SF|OF);
  context->c_eflags |= x86_cmpb(oldval,value);
  *(u8 *)&CONTEXT_AREG(*context) = value;
  goto ok;
 }

 case 0x0fb1:
  /* cmpxchg r/m16,r16 */
  /* cmpxchg r/m32,r32 */
  text = x86_decode_modrm(text,&modrm);
  if (flags & F_OP16) {
   u16 oldval,value;
   oldval = (u16)CONTEXT_AREG(*context);
   value  = vio_cmpxchw(ops,closure,addr,oldval,
                       (u16)MODRM_REG(context,&modrm));
   context->c_eflags &= ~(ZF|CF|PF|AF|SF|OF);
   context->c_eflags |= x86_cmpw(oldval,value);
   *(u16 *)&CONTEXT_AREG(*context) = value;
  } else {
   u32 oldval,value;
   oldval = (u32)CONTEXT_AREG(*context);
   value  = vio_cmpxchl(ops,closure,addr,oldval,
                       (u32)MODRM_REG(context,&modrm));
   context->c_eflags &= ~(ZF|CF|PF|AF|SF|OF);
   context->c_eflags |= x86_cmpl(oldval,value);
   *(u32 *)&CONTEXT_AREG(*context) = value;
  }
  goto ok;

 case 0x88:
  /* mov r/m8,r8 */
  text = x86_decode_modrm(text,&modrm);
  vio_writeb(ops,closure,addr,(u8)MODRM_REG(context,&modrm));
  goto ok;

 case 0x89:
  /* mov r/m16,r16 */
  /* mov r/m32,r32 */
  text = x86_decode_modrm(text,&modrm);
  if (flags & F_OP16) {
   vio_writew(ops,closure,addr,(u16)MODRM_REG(context,&modrm));
  } else {
   vio_writel(ops,closure,addr,(u32)MODRM_REG(context,&modrm));
  }
  goto ok;

 case 0x8a:
  /* mov r8,r/m8 */
  text = x86_decode_modrm(text,&modrm);
  MODRM_REG8(context,&modrm) = vio_readb(ops,closure,addr);
  goto ok;

 case 0x8b:
  /* mov r/m16,r16 */
  /* mov r/m32,r32 */
  text = x86_decode_modrm(text,&modrm);
  if (flags & F_OP16) {
   *(u16 *)&MODRM_REG(context,&modrm) = vio_readw(ops,closure,addr);
  } else {
   MODRM_REG(context,&modrm) = vio_readl(ops,closure,addr);
  }
  goto ok;

 case 0xc6:
  /* mov r/m8,imm8 */
  text = x86_decode_modrm(text,&modrm);
  vio_writeb(ops,closure,addr,*(u8 *)text);
  text += 1;
  goto ok;

 case 0xc7:
  /* mov r/m16,imm16 */
  /* mov r/m32,imm32 */
  text = x86_decode_modrm(text,&modrm);
  if (flags & F_OP16) {
   vio_writew(ops,closure,addr,*(u16 *)text);
   text += 2;
  } else {
   vio_writel(ops,closure,addr,*(u32 *)text);
   text += 4;
  }
  goto ok;

 case 0xa0:
  /* mov al,moffs8* */
  if (flags & F_AD16)
       text += 2;
  else text += 4;
  *(u8 *)&CONTEXT_AREG(*context) = vio_readb(ops,closure,addr);
  goto ok;
 case 0xa1:
  /* mov ax,moffs16* */
  /* mov eax,moffs32* */
  if (flags & F_AD16)
       text += 2;
  else text += 4;
  if (flags & F_OP16) {
   *(u16 *)&CONTEXT_AREG(*context) = vio_readw(ops,closure,addr);
  } else {
   CONTEXT_AREG(*context) = vio_readl(ops,closure,addr);
  }
  goto ok;

 case 0xa2:
  /* mov moffs8*,al */
  if (flags & F_AD16)
       text += 2;
  else text += 4;
  vio_writeb(ops,closure,addr,(u8)CONTEXT_AREG(*context));
  goto ok;

 case 0xa3:
  /* mov moffs16*,ax */
  /* mov moffs32*,eax */
  if (flags & F_AD16)
       text += 2;
  else text += 4;
  if (flags & F_OP16) {
   vio_writew(ops,closure,addr,(u16)CONTEXT_AREG(*context));
  } else {
   vio_writel(ops,closure,addr,(u32)CONTEXT_AREG(*context));
  }
  goto ok;

 case 0x86:
  /* xchg r/m8, r8 */
  text = x86_decode_modrm(text,&modrm);
  vio_rxchgb(ops,closure,addr,&MODRM_REG8(context,&modrm));
  goto ok;

 case 0x87:
  /* xchg r/m16, r16 */
  /* xchg r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  if (flags & F_OP16) {
   vio_rxchgw(ops,closure,addr,(u16 *)&MODRM_REG(context,&modrm));
  } else {
   vio_rxchgl(ops,closure,addr,(u32 *)&MODRM_REG(context,&modrm));
  }
  goto ok;

 case 0x0fc0:
  /* xadd r/m8, r8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(CF|PF|AF|SF|ZF|OF);
  CONTEXT_FLAGS(*context) |= vio_rxaddb(ops,closure,addr,&MODRM_REG8(context,&modrm)) & (CF|PF|AF|SF|ZF|OF);
  goto ok;

 case 0x0fc1:
  /* xadd r/m8, r8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(CF|PF|AF|SF|ZF|OF);
  if (flags & F_OP16) {
   CONTEXT_FLAGS(*context) |= vio_rxaddw(ops,closure,addr,(u16 *)&MODRM_REG(context,&modrm)) & (CF|PF|AF|SF|ZF|OF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_rxaddl(ops,closure,addr,(u32 *)&MODRM_REG(context,&modrm)) & (CF|PF|AF|SF|ZF|OF);
  }
  goto ok;

 case 0xf6:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  case 0:
   /* test r/m8,imm8 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
   CONTEXT_FLAGS(*context) |= vio_ltestb(ops,closure,addr,*(u8 *)text) & (OF|CF|SF|ZF|PF);
   text += 1;
   goto ok;

  case 2:
   /* not r/m8 */
   if (flags & F_LOCK) {
    vio_atomic_notb(ops,closure,addr);
   } else {
    vio_notb(ops,closure,addr);
   }
   goto ok;

  case 3:
   /* neg r/m8 */
   CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|AF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_negb(ops,closure,addr) & (CF|OF|SF|ZF|AF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_negb(ops,closure,addr) & (CF|OF|SF|ZF|AF|PF);
   }
   goto ok;

  case 4:
   /* mul r/m8 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF);
   CONTEXT_FLAGS(*context) |= x86_mulb((u8)CONTEXT_AREG(*context),
                                        vio_readb(ops,closure,addr),
                                       (u16 *)&CONTEXT_AREG(*context)) & (OF|CF);
   goto ok;

  case 5:
   /* imul r/m8 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF);
   CONTEXT_FLAGS(*context) |= x86_imulb((u8)CONTEXT_AREG(*context),
                                         vio_readb(ops,closure,addr),
                                        (u16 *)&CONTEXT_AREG(*context)) & (OF|CF);
   goto ok;

  case 6:
   /* div r/m8 */
   x86_divb((u16)CONTEXT_AREG(*context),
             vio_readb(ops,closure,addr),
            (u8 *)&CONTEXT_AREG(*context));
   goto ok;

  case 7:
   /* idiv r/m8 */
   x86_idivb((u16)CONTEXT_AREG(*context),
              vio_readb(ops,closure,addr),
             (u8 *)&CONTEXT_AREG(*context));
   goto ok;

  default: break;
  }
  break;

 case 0xf7:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  case 0:
   /* test r/m16,imm16 */
   /* test r/m32,imm32 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
   if (flags & F_OP16) {
    CONTEXT_FLAGS(*context) |= vio_ltestw(ops,closure,addr,*(u16 *)text) & (OF|CF|SF|ZF|PF);
    text += 2;
   } else {
    CONTEXT_FLAGS(*context) |= vio_ltestl(ops,closure,addr,*(u32 *)text) & (OF|CF|SF|ZF|PF);
    text += 4;
   }
   goto ok;

  case 2:
   /* not r/m16 */
   /* not r/m32 */
   switch (flags & (F_LOCK|F_OP16)) {
   case 0:             vio_notb(ops,closure,addr); break;
   case F_LOCK:        vio_atomic_notb(ops,closure,addr); break;
   case F_OP16:        vio_notw(ops,closure,addr); break;
   case F_OP16|F_LOCK: vio_atomic_notw(ops,closure,addr); break;
   default: __builtin_unreachable();
   }
   goto ok;

  case 3:
   /* neg r/m16 */
   /* neg r/m32 */
   CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|AF|PF);
   switch (flags & (F_LOCK|F_OP16)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_negb(ops,closure,addr) & (CF|OF|SF|ZF|AF|PF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_negb(ops,closure,addr) & (CF|OF|SF|ZF|AF|PF); break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_negw(ops,closure,addr) & (CF|OF|SF|ZF|AF|PF); break;
   case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_negw(ops,closure,addr) & (CF|OF|SF|ZF|AF|PF); break;
   default: __builtin_unreachable();
   }
   goto ok;

  case 4:
   /* mul r/m16 */
   /* mul r/m32 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF);
   if (flags & F_OP16) {
    CONTEXT_FLAGS(*context) |= x86_mulw((u16)CONTEXT_AREG(*context),
                                         vio_readw(ops,closure,addr),
                                        (u16 *)&CONTEXT_AREG(*context),
                                        (u16 *)&CONTEXT_DREG(*context)) & (OF|CF);
   } else {
    CONTEXT_FLAGS(*context) |= x86_mull((u32)CONTEXT_AREG(*context),
                                         vio_readl(ops,closure,addr),
                                        (u32 *)&CONTEXT_AREG(*context),
                                        (u32 *)&CONTEXT_DREG(*context)) & (OF|CF);
   }
   goto ok;

  case 5:
   /* imul r/m16 */
   /* imul r/m32 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF);
   if (flags & F_OP16) {
    CONTEXT_FLAGS(*context) |= x86_imulw((u16)CONTEXT_AREG(*context),
                                          vio_readw(ops,closure,addr),
                                         (u16 *)&CONTEXT_AREG(*context),
                                         (u16 *)&CONTEXT_DREG(*context)) & (OF|CF);
   } else {
    CONTEXT_FLAGS(*context) |= x86_imull((u32)CONTEXT_AREG(*context),
                                          vio_readl(ops,closure,addr),
                                         (u32 *)&CONTEXT_AREG(*context),
                                         (u32 *)&CONTEXT_DREG(*context)) & (OF|CF);
   }
   goto ok;

  case 6:
   /* div r/m16 */
   /* div r/m32 */
   if (flags & F_OP16) {
    x86_divw((u16)CONTEXT_DREG(*context),
             (u16)CONTEXT_AREG(*context),
              vio_readw(ops,closure,addr),
             (u16 *)&CONTEXT_AREG(*context),
             (u16 *)&CONTEXT_DREG(*context));
   } else {
    x86_divl((u32)CONTEXT_DREG(*context),
             (u32)CONTEXT_AREG(*context),
              vio_readl(ops,closure,addr),
             (u32 *)&CONTEXT_AREG(*context),
             (u32 *)&CONTEXT_DREG(*context));
   }
   goto ok;

  case 7:
   /* idiv r/m16 */
   /* idiv r/m32 */
   if (flags & F_OP16) {
    x86_idivw((u16)CONTEXT_DREG(*context),
              (u16)CONTEXT_AREG(*context),
               vio_readw(ops,closure,addr),
              (u16 *)&CONTEXT_AREG(*context),
              (u16 *)&CONTEXT_DREG(*context));
   } else {
    x86_idivl((u32)CONTEXT_DREG(*context),
              (u32)CONTEXT_AREG(*context),
               vio_readl(ops,closure,addr),
              (u32 *)&CONTEXT_AREG(*context),
              (u32 *)&CONTEXT_DREG(*context));
   }
   goto ok;

  default: break;
  }
  break;

 case 0x0faf:
  /* imul r16,r/m16 */
  /* imul r32,r/m32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF);
  if (flags & F_OP16) {
   if (__builtin_mul_overflow(*(s16 *)&MODRM_REG(context,&modrm),(s16)vio_readw(ops,closure,addr),
                               (s16 *)&MODRM_REG(context,&modrm)))
       CONTEXT_FLAGS(*context) |= (OF|CF);
  } else {
   if (__builtin_mul_overflow(*(s32 *)&MODRM_REG(context,&modrm),(s32)vio_readl(ops,closure,addr),
                               (s32 *)&MODRM_REG(context,&modrm)))
       CONTEXT_FLAGS(*context) |= (OF|CF);
  }
  goto ok;

 case 0x6b:
  /* imul r16,r/m16,imm8 */
  /* imul r32,r/m32,imm8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF);
  if (flags & F_OP16) {
   if (__builtin_mul_overflow((s16)*(s8 *)text,(s16)vio_readw(ops,closure,addr),
                              (s16 *)&MODRM_REG(context,&modrm)))
       CONTEXT_FLAGS(*context) |= (OF|CF);
  } else {
   if (__builtin_mul_overflow((s32)*(s8 *)text,(s32)vio_readl(ops,closure,addr),
                              (s32 *)&MODRM_REG(context,&modrm)))
       CONTEXT_FLAGS(*context) |= (OF|CF);
  }
  text += 1;
  goto ok;

 case 0x69:
  /* imul r16,r/m16,imm16 */
  /* imul r32,r/m32,imm32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF);
  if (flags & F_OP16) {
   if (__builtin_mul_overflow(*(s16 *)text,(s16)vio_readw(ops,closure,addr),
                              (s16 *)&MODRM_REG(context,&modrm)))
       CONTEXT_FLAGS(*context) |= (OF|CF);
   text += 2;
  } else {
   if (__builtin_mul_overflow(*(s32 *)text,(s32)vio_readl(ops,closure,addr),
                              (s32 *)&MODRM_REG(context,&modrm)))
       CONTEXT_FLAGS(*context) |= (OF|CF);
   text += 4;
  }
  goto ok;

 case 0x84:
  /* test r/m8,r8 */
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  CONTEXT_FLAGS(*context) |= vio_ltestb(ops,closure,addr,MODRM_REG8(context,&modrm)) & (OF|CF|SF|ZF|PF);
  goto ok;

 case 0x85:
  /* test r/m16,r16 */
  /* test r/m32,r32 */
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  if (flags & F_OP16) {
   CONTEXT_FLAGS(*context) |= vio_ltestw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_ltestl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF);
  }
  goto ok;

 case 0xfe:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  case 0:
   /* inc r/m8 */
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_incb(ops,closure,addr) & (OF|SF|ZF|AF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_incb(ops,closure,addr) & (OF|SF|ZF|AF|PF);
   }
   goto ok;

  case 1:
   /* dec r/m8 */
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_decb(ops,closure,addr) & (OF|SF|ZF|AF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_decb(ops,closure,addr) & (OF|SF|ZF|AF|PF);
   }
   goto ok;

  default: break;
  }
  break;

 case 0xff:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  case 0:
   /* inc r/m16 */
   /* inc r/m32 */
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|PF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_incl(ops,closure,addr) & (OF|SF|ZF|AF|PF);
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_incw(ops,closure,addr) & (OF|SF|ZF|AF|PF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_incl(ops,closure,addr) & (OF|SF|ZF|AF|PF);
   case F_LOCK|F_OP16: CONTEXT_FLAGS(*context) |= vio_atomic_incw(ops,closure,addr) & (OF|SF|ZF|AF|PF); break;
   default: __builtin_unreachable();
   }
   goto ok;

  case 1:
   /* dec r/m16 */
   /* dec r/m32 */
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|PF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_decl(ops,closure,addr) & (OF|SF|ZF|AF|PF);
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_decw(ops,closure,addr) & (OF|SF|ZF|AF|PF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_decl(ops,closure,addr) & (OF|SF|ZF|AF|PF);
   case F_LOCK|F_OP16: CONTEXT_FLAGS(*context) |= vio_atomic_decw(ops,closure,addr) & (OF|SF|ZF|AF|PF); break;
   default: __builtin_unreachable();
   }
   goto ok;

  case 2:
   /* call r/m16 */
   /* call r/m32 */
   if (flags & F_OP16) {
    TRY *--(*(u16 **)&X86_ANYCONTEXT32_ESP(*context)) = (u16)CONTEXT_IP(*context);
    CATCH_HANDLED (E_SEGFAULT) goto fail;
    CONTEXT_IP(*context) = (uintptr_t)vio_readw(ops,closure,addr);
   } else {
    TRY *--(*(uintptr_t **)&X86_ANYCONTEXT32_ESP(*context)) = CONTEXT_IP(*context);
    CATCH_HANDLED (E_SEGFAULT) goto fail;
#ifdef __x86_64__
    CONTEXT_IP(*context) = (uintptr_t)vio_readq(ops,closure,addr);
#else
    CONTEXT_IP(*context) = (uintptr_t)vio_readl(ops,closure,addr);
#endif
   }
   goto ok;

  case 4:
   /* jmp r/m16 */
   /* jmp r/m32 */
   if (flags & F_OP16) {
    CONTEXT_IP(*context) = (uintptr_t)vio_readw(ops,closure,addr);
   } else {
#ifdef __x86_64__
    CONTEXT_IP(*context) = (uintptr_t)vio_readq(ops,closure,addr);
#else
    CONTEXT_IP(*context) = (uintptr_t)vio_readl(ops,closure,addr);
#endif
   }
   goto ok;

  case 6:
   /* push r/m16 */
   /* push r/m32 */
   if (X86_ANYCONTEXT32_ESP(*context) == (uintptr_t)abs_addr)
       goto fail;
   if (flags & F_OP16) {
    u16 value = vio_readw(ops,closure,addr);
    TRY *--(*(u16 **)&X86_ANYCONTEXT32_ESP(*context)) = value;
    CATCH_HANDLED (E_SEGFAULT) goto fail;
   } else {       
    u32 value = vio_readw(ops,closure,addr);
    TRY *--(*(u32 **)&X86_ANYCONTEXT32_ESP(*context)) = value;
    CATCH_HANDLED (E_SEGFAULT) goto fail;
   }
   goto ok;

  default: break;
  }
  break;

 case 0x8f:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  case 0:
   /* pop r/m16 */
   /* pop r/m32 */
   if (X86_ANYCONTEXT32_ESP(*context) == (uintptr_t)abs_addr)
       goto fail;
   if (flags & F_OP16) {
    u16 COMPILER_IGNORE_UNINITIALIZED(value);
    TRY value = *(*(u16 **)&X86_ANYCONTEXT32_ESP(*context))++;
    CATCH_HANDLED (E_SEGFAULT) goto fail;
    vio_writew(ops,closure,addr,value);
   } else {
    u32 COMPILER_IGNORE_UNINITIALIZED(value);
    TRY value = *(*(u32 **)&X86_ANYCONTEXT32_ESP(*context))++;
    CATCH_HANDLED (E_SEGFAULT) goto fail;
    vio_writel(ops,closure,addr,value);
   }
   goto ok;

  default: break;
  }
  break;

 case 0x80:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  case 0:
   /* add r/m8, imm8 */
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_raddb(ops,closure,addr,*(u8 *)text) & (OF|SF|ZF|AF|CF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_raddb(ops,closure,addr,*(u8 *)text) & (OF|SF|ZF|AF|CF|PF);
   }
   text += 1;
   goto ok;

  case 1:
   /* or r/m8, imm8 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_rorb(ops,closure,addr,*(u8 *)text) & (OF|CF|SF|ZF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_rorb(ops,closure,addr,*(u8 *)text) & (OF|CF|SF|ZF|PF);
   }
   text += 1;
   goto ok;

  {
   u8 value;
  case 2:
   /* adc r/m8, imm8 */
   value = *(u8 *)text;
   if (CONTEXT_FLAGS(*context) & CF) ++value;
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_raddb(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_raddb(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
   }
   if (!value) CONTEXT_FLAGS(*context) |= CF;
   text += 1;
   goto ok;
  }

  {
   u8 value;
  case 3:
   /* sbb r/m8, imm8 */
   value = *(u8 *)text;
   if (CONTEXT_FLAGS(*context) & CF) ++value;
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_rsubb(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_rsubb(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
   }
   if (!value) CONTEXT_FLAGS(*context) |= CF;
   text += 1;
   goto ok;
  }

  case 4:
   /* and r/m8, imm8 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_randb(ops,closure,addr,*(u8 *)text) & (OF|CF|SF|ZF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_randb(ops,closure,addr,*(u8 *)text) & (OF|CF|SF|ZF|PF);
   }
   text += 1;
   goto ok;

  case 5:
   /* sub r/m8, imm8 */
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_rsubb(ops,closure,addr,*(u8 *)text) & (OF|SF|ZF|AF|CF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_rsubb(ops,closure,addr,*(u8 *)text) & (OF|SF|ZF|AF|CF|PF);
   }
   text += 1;
   goto ok;

  case 6:
   /* xor r/m8, imm8 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_rxorb(ops,closure,addr,*(u8 *)text) & (OF|CF|SF|ZF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_rxorb(ops,closure,addr,*(u8 *)text) & (OF|CF|SF|ZF|PF);
   }
   text += 1;
   goto ok;

  case 7:
   /* cmp r/m8, imm8 */
   CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|AF|PF);
   CONTEXT_FLAGS(*context) |= vio_rcmpb(ops,closure,addr,*(u8 *)text) & (CF|OF|SF|ZF|AF|PF);
   text += 1;
   goto ok;

  default: break;
  }
  break;

 case 0x81:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  case 0:
   /* add r/m16, imm16 */
   /* add r/m32, imm32 */
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_raddl(ops,closure,addr,*(u32 *)text) & (OF|SF|ZF|AF|CF|PF); text += 4; break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_raddw(ops,closure,addr,*(u16 *)text) & (OF|SF|ZF|AF|CF|PF); text += 2; break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_raddl(ops,closure,addr,*(u32 *)text) & (OF|SF|ZF|AF|CF|PF); text += 4; break;
   case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_raddw(ops,closure,addr,*(u16 *)text) & (OF|SF|ZF|AF|CF|PF); text += 2; break;
   default: __builtin_unreachable();
   }
   goto ok;

  case 1:
   /* or r/m16, imm16 */
   /* or r/m32, imm32 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_rorl(ops,closure,addr,*(u32 *)text) & (OF|CF|SF|ZF|PF); text += 4; break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rorw(ops,closure,addr,*(u16 *)text) & (OF|CF|SF|ZF|PF); text += 2; break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rorl(ops,closure,addr,*(u32 *)text) & (OF|CF|SF|ZF|PF); text += 4; break;
   case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_rorw(ops,closure,addr,*(u16 *)text) & (OF|CF|SF|ZF|PF); text += 2; break;
   default: __builtin_unreachable();
   }
   goto ok;

  case 2:
   /* adc r/m16, imm16 */
   /* adc r/m32, imm32 */
   if (flags & F_OP16) {
    u16 value = *(u16 *)text;
    if (CONTEXT_FLAGS(*context) & CF) ++value;
    CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
    if (flags & F_LOCK) {
     CONTEXT_FLAGS(*context) |= vio_atomic_raddw(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    } else {
     CONTEXT_FLAGS(*context) |= vio_raddw(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    }
    if (!value) CONTEXT_FLAGS(*context) |= CF;
    text += 2;
   } else {
    u32 value = *(u32 *)text;
    if (CONTEXT_FLAGS(*context) & CF) ++value;
    CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
    if (flags & F_LOCK) {
     CONTEXT_FLAGS(*context) |= vio_atomic_raddl(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    } else {
     CONTEXT_FLAGS(*context) |= vio_raddl(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    }
    if (!value) CONTEXT_FLAGS(*context) |= CF;
    text += 4;
   }
   goto ok;

  case 3:
   /* sbb r/m16, imm16 */
   /* sbb r/m32, imm32 */
   if (flags & F_OP16) {
    u16 value = *(u16 *)text;
    if (CONTEXT_FLAGS(*context) & CF) ++value;
    CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
    if (flags & F_LOCK) {
     CONTEXT_FLAGS(*context) |= vio_atomic_rsubw(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    } else {
     CONTEXT_FLAGS(*context) |= vio_rsubw(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    }
    if (!value) CONTEXT_FLAGS(*context) |= CF;
    text += 2;
   } else {
    u32 value = *(u32 *)text;
    if (CONTEXT_FLAGS(*context) & CF) ++value;
    CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
    if (flags & F_LOCK) {
     CONTEXT_FLAGS(*context) |= vio_atomic_rsubl(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    } else {
     CONTEXT_FLAGS(*context) |= vio_rsubl(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    }
    if (!value) CONTEXT_FLAGS(*context) |= CF;
    text += 4;
   }
   goto ok;

  case 4:
   /* and r/m16, imm16 */
   /* and r/m32, imm32 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_randl(ops,closure,addr,*(u32 *)text) & (OF|CF|SF|ZF|PF); text += 4; break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_randw(ops,closure,addr,*(u16 *)text) & (OF|CF|SF|ZF|PF); text += 2; break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_randl(ops,closure,addr,*(u32 *)text) & (OF|CF|SF|ZF|PF); text += 4; break;
   case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_randw(ops,closure,addr,*(u16 *)text) & (OF|CF|SF|ZF|PF); text += 2; break;
   default: __builtin_unreachable();
   }
   goto ok;

  case 5:
   /* sub r/m16, imm16 */
   /* sub r/m32, imm32 */
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_rsubl(ops,closure,addr,*(u32 *)text) & (OF|SF|ZF|AF|CF|PF); text += 4; break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rsubw(ops,closure,addr,*(u16 *)text) & (OF|SF|ZF|AF|CF|PF); text += 2; break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rsubl(ops,closure,addr,*(u32 *)text) & (OF|SF|ZF|AF|CF|PF); text += 4; break;
   case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_rsubw(ops,closure,addr,*(u16 *)text) & (OF|SF|ZF|AF|CF|PF); text += 2; break;
   default: __builtin_unreachable();
   }
   goto ok;

  case 6:
   /* xor r/m16, imm16 */
   /* xor r/m32, imm32 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_rxorl(ops,closure,addr,*(u32 *)text) & (OF|CF|SF|ZF|PF); text += 4; break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rxorw(ops,closure,addr,*(u16 *)text) & (OF|CF|SF|ZF|PF); text += 2; break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rxorl(ops,closure,addr,*(u32 *)text) & (OF|CF|SF|ZF|PF); text += 4; break;
   case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_rxorw(ops,closure,addr,*(u16 *)text) & (OF|CF|SF|ZF|PF); text += 2; break;
   default: __builtin_unreachable();
   }
   goto ok;

  case 7:
   /* cmp r/m16, imm16 */
   /* cmp r/m32, imm32 */
   CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|AF|PF);
   if (flags & F_OP16) {
    CONTEXT_FLAGS(*context) |= vio_rcmpw(ops,closure,addr,*(u16 *)text) & (CF|OF|SF|ZF|AF|PF);
    text += 2;
   } else {
    CONTEXT_FLAGS(*context) |= vio_rcmpl(ops,closure,addr,*(u32 *)text) & (CF|OF|SF|ZF|AF|PF);
    text += 4;
   }
   goto ok;

  default: break;
  }
  break;

 case 0x83:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  case 0:
   /* add r/m8, Simm8 */
   /* add r/m16, Simm8 */
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_raddl(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|SF|ZF|AF|CF|PF); break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_raddw(ops,closure,addr,(u32)(s32)*(u8 *)text) & (OF|SF|ZF|AF|CF|PF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_raddl(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|SF|ZF|AF|CF|PF); break;
   case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_raddw(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|SF|ZF|AF|CF|PF); break;
   default: __builtin_unreachable();
   }
   text += 1;
   goto ok;

  case 1:
   /* or r/m8, Simm8 */
   /* or r/m16, Simm8 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_rorl(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|CF|SF|ZF|PF); break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rorw(ops,closure,addr,(u32)(s32)*(u8 *)text) & (OF|CF|SF|ZF|PF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rorl(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|CF|SF|ZF|PF); break;
   case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_rorw(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|CF|SF|ZF|PF); break;
   default: __builtin_unreachable();
   }
   text += 1;
   goto ok;

  case 2:
   /* adc r/m16, Simm8 */
   /* adc r/m32, Simm8 */
   if (flags & F_OP16) {
    u16 value = (u16)(s16)*(s8 *)text;
    if (CONTEXT_FLAGS(*context) & CF) ++value;
    CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
    if (flags & F_LOCK) {
     CONTEXT_FLAGS(*context) |= vio_atomic_raddw(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    } else {
     CONTEXT_FLAGS(*context) |= vio_raddw(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    }
    if (!value) CONTEXT_FLAGS(*context) |= CF;
    text += 2;
   } else {
    u32 value = (u32)(s32)*(s8 *)text;
    if (CONTEXT_FLAGS(*context) & CF) ++value;
    CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
    if (flags & F_LOCK) {
     CONTEXT_FLAGS(*context) |= vio_atomic_raddl(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    } else {
     CONTEXT_FLAGS(*context) |= vio_raddl(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    }
    if (!value) CONTEXT_FLAGS(*context) |= CF;
    text += 4;
   }
   goto ok;

  case 3:
   /* sbb r/m16, Simm8 */
   /* sbb r/m32, Simm8 */
   if (flags & F_OP16) {
    u16 value = (u16)(s16)*(s8 *)text;
    if (CONTEXT_FLAGS(*context) & CF) ++value;
    CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
    if (flags & F_LOCK) {
     CONTEXT_FLAGS(*context) |= vio_atomic_rsubw(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    } else {
     CONTEXT_FLAGS(*context) |= vio_rsubw(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    }
    if (!value) CONTEXT_FLAGS(*context) |= CF;
    text += 2;
   } else {
    u32 value = (u32)(s32)*(s8 *)text;
    if (CONTEXT_FLAGS(*context) & CF) ++value;
    CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
    if (flags & F_LOCK) {
     CONTEXT_FLAGS(*context) |= vio_atomic_rsubl(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    } else {
     CONTEXT_FLAGS(*context) |= vio_rsubl(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
    }
    if (!value) CONTEXT_FLAGS(*context) |= CF;
    text += 4;
   }
   goto ok;

  case 4:
   /* and r/m8, Simm8 */
   /* and r/m16, Simm8 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_randl(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|CF|SF|ZF|PF); break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_randw(ops,closure,addr,(u32)(s32)*(u8 *)text) & (OF|CF|SF|ZF|PF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_randl(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|CF|SF|ZF|PF); break;
   case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_randw(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|CF|SF|ZF|PF); break;
   default: __builtin_unreachable();
   }
   text += 1;
   goto ok;

  case 5:
   /* sub r/m8, Simm8 */
   /* sub r/m16, Simm8 */
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_rsubl(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|SF|ZF|AF|CF|PF); break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rsubw(ops,closure,addr,(u32)(s32)*(u8 *)text) & (OF|SF|ZF|AF|CF|PF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rsubl(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|SF|ZF|AF|CF|PF); break;
   case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_rsubw(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|SF|ZF|AF|CF|PF); break;
   default: __builtin_unreachable();
   }
   text += 1;
   goto ok;

  case 6:
   /* xor r/m8, Simm8 */
   /* xor r/m16, Simm8 */
   CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_rxorl(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|CF|SF|ZF|PF); break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rxorw(ops,closure,addr,(u32)(s32)*(u8 *)text) & (OF|CF|SF|ZF|PF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rxorl(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|CF|SF|ZF|PF); break;
   case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_rxorw(ops,closure,addr,(u32)(s32)*(s8 *)text) & (OF|CF|SF|ZF|PF); break;
   default: __builtin_unreachable();
   }
   text += 1;
   goto ok;

  case 7:
   /* cmp r/m16, imm16 */
   /* cmp r/m32, imm32 */
   CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|AF|PF);
   if (flags & F_OP16) {
    CONTEXT_FLAGS(*context) |= vio_rsubw(ops,closure,addr,(u16)*(u8 *)text) & (CF|OF|SF|ZF|AF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_rsubl(ops,closure,addr,(u32)*(u8 *)text) & (CF|OF|SF|ZF|AF|PF);
   }
   text += 1;
   goto ok;

  default: break;
  }
  break;

 case 0x00:
  /* add r/m8, r8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
  if (flags & F_LOCK) {
   CONTEXT_FLAGS(*context) |= vio_atomic_raddb(ops,closure,addr,MODRM_REG8(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_raddb(ops,closure,addr,MODRM_REG8(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
  }
  goto ok;

 case 0x01:
  /* add r/m16, r16 */
  /* add r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
  switch (flags & (F_OP16|F_LOCK)) {
  case 0:             CONTEXT_FLAGS(*context) |= vio_raddl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF); break;
  case F_OP16:        CONTEXT_FLAGS(*context) |= vio_raddw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF); break;
  case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_raddl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF); break;
  case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_raddw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF); break;
  default: __builtin_unreachable();
  }
  goto ok;

 case 0x02:
  /* add r8, r/m8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
  CONTEXT_FLAGS(*context) |= vio_laddb(ops,closure,addr,&MODRM_REG8(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
  goto ok;

 case 0x03:
  /* add r/m16, r16 */
  /* add r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
  if (flags & F_OP16) {
   CONTEXT_FLAGS(*context) |= vio_laddw(ops,closure,addr,(u16 *)&MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_laddl(ops,closure,addr,(u32 *)&MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
  }
  goto ok;

 case 0x08:
  /* or r/m8, r8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  if (flags & F_LOCK) {
   CONTEXT_FLAGS(*context) |= vio_atomic_rorb(ops,closure,addr,MODRM_REG8(context,&modrm)) & (OF|CF|SF|ZF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_rorb(ops,closure,addr,MODRM_REG8(context,&modrm)) & (OF|CF|SF|ZF|PF);
  }
  goto ok;

 case 0x09:
  /* or r/m16, r16 */
  /* or r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  switch (flags & (F_OP16|F_LOCK)) {
  case 0:             CONTEXT_FLAGS(*context) |= vio_rorl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF); break;
  case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rorw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF); break;
  case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rorl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF); break;
  case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_rorw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF); break;
  default: __builtin_unreachable();
  }
  goto ok;

 case 0x0a:
  /* or r8, r/m8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  CONTEXT_FLAGS(*context) |= vio_lorb(ops,closure,addr,&MODRM_REG8(context,&modrm)) & (OF|CF|SF|ZF|PF);
  goto ok;

 case 0x0b:
  /* or r/m16, r16 */
  /* or r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  if (flags & F_OP16) {
   CONTEXT_FLAGS(*context) |= vio_lorw(ops,closure,addr,(u16 *)&MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_lorl(ops,closure,addr,(u32 *)&MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF);
  }
  goto ok;


 {
  u8 value;
 case 0x10:
  /* adc r/m8, r8 */
  text = x86_decode_modrm(text,&modrm);
  value = MODRM_REG8(context,&modrm);
  if (CONTEXT_FLAGS(*context) & CF) ++value;
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
  if (flags & F_LOCK) {
   CONTEXT_FLAGS(*context) |= vio_atomic_raddb(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_raddb(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
  }
  if (!value) CONTEXT_FLAGS(*context) |= CF;
  goto ok;
 }

 case 0x11:
  /* adc r/m16, r16 */
  /* adc r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  if (flags & F_OP16) {
   u16 value = (u16)MODRM_REG(context,&modrm);
   if (CONTEXT_FLAGS(*context) & CF) ++value;
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_raddw(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_raddw(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
   }
   if (!value) CONTEXT_FLAGS(*context) |= CF;
  } else {
   u32 value = (u32)MODRM_REG(context,&modrm);
   if (CONTEXT_FLAGS(*context) & CF) ++value;
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_raddl(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_raddl(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
   }
   if (!value) CONTEXT_FLAGS(*context) |= CF;
  }
  goto ok;

 case 0x12:
  /* adc r8, r/m8 */
  text = x86_decode_modrm(text,&modrm);
  if (CONTEXT_FLAGS(*context) & CF) ++MODRM_REG8(context,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
  CONTEXT_FLAGS(*context) |= vio_laddb(ops,closure,addr,&MODRM_REG8(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
  if (!MODRM_REG8(context,&modrm)) CONTEXT_FLAGS(*context) |= CF;
  goto ok;

 case 0x13:
  /* adc r/m16, r16 */
  /* adc r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  if (flags & F_OP16) {
   if (CONTEXT_FLAGS(*context) & CF) ++*(u16 *)&MODRM_REG(context,&modrm);
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   CONTEXT_FLAGS(*context) |= vio_laddw(ops,closure,addr,(u16 *)&MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
   if (!*(u16 *)&MODRM_REG(context,&modrm)) CONTEXT_FLAGS(*context) |= CF;
  } else {
   if (CONTEXT_FLAGS(*context) & CF) ++*(u32 *)&MODRM_REG(context,&modrm);
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   CONTEXT_FLAGS(*context) |= vio_laddl(ops,closure,addr,(u32 *)&MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
   if (!*(u32 *)&MODRM_REG(context,&modrm)) CONTEXT_FLAGS(*context) |= CF;
  }
  goto ok;

 {
  u8 value;
 case 0x18:
  /* sbb r/m8, r8 */
  text = x86_decode_modrm(text,&modrm);
  value = MODRM_REG8(context,&modrm);
  if (CONTEXT_FLAGS(*context) & CF) ++value;
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
  if (flags & F_LOCK) {
   CONTEXT_FLAGS(*context) |= vio_atomic_rsubb(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_rsubb(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
  }
  if (!value) CONTEXT_FLAGS(*context) |= CF;
  goto ok;
 }

 case 0x19:
  /* sbb r/m16, r16 */
  /* sbb r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  if (flags & F_OP16) {
   u16 value = (u16)MODRM_REG(context,&modrm);
   if (CONTEXT_FLAGS(*context) & CF) ++value;
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_rsubw(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_rsubw(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
   }
   if (!value) CONTEXT_FLAGS(*context) |= CF;
  } else {
   u32 value = (u32)MODRM_REG(context,&modrm);
   if (CONTEXT_FLAGS(*context) & CF) ++value;
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_rsubl(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_rsubl(ops,closure,addr,value) & (OF|SF|ZF|AF|CF|PF);
   }
   if (!value) CONTEXT_FLAGS(*context) |= CF;
  }
  goto ok;

 case 0x1a:
  /* sbb r8, r/m8 */
  text = x86_decode_modrm(text,&modrm);
  if (CONTEXT_FLAGS(*context) & CF) ++MODRM_REG8(context,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
  CONTEXT_FLAGS(*context) |= vio_lsubb(ops,closure,addr,&MODRM_REG8(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
  if (!MODRM_REG8(context,&modrm)) CONTEXT_FLAGS(*context) |= CF;
  goto ok;

 case 0x1b:
  /* sbb r/m16, r16 */
  /* sbb r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  if (flags & F_OP16) {
   if (CONTEXT_FLAGS(*context) & CF) ++*(u16 *)&MODRM_REG(context,&modrm);
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   CONTEXT_FLAGS(*context) |= vio_lsubw(ops,closure,addr,(u16 *)&MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
   if (!*(u16 *)&MODRM_REG(context,&modrm)) CONTEXT_FLAGS(*context) |= CF;
  } else {
   if (CONTEXT_FLAGS(*context) & CF) ++*(u32 *)&MODRM_REG(context,&modrm);
   CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
   CONTEXT_FLAGS(*context) |= vio_lsubl(ops,closure,addr,(u32 *)&MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
   if (!*(u32 *)&MODRM_REG(context,&modrm)) CONTEXT_FLAGS(*context) |= CF;
  }
  goto ok;

 case 0x20:
  /* and r/m8, r8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  if (flags & F_LOCK) {
   CONTEXT_FLAGS(*context) |= vio_atomic_randb(ops,closure,addr,MODRM_REG8(context,&modrm)) & (OF|CF|SF|ZF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_randb(ops,closure,addr,MODRM_REG8(context,&modrm)) & (OF|CF|SF|ZF|PF);
  }
  goto ok;

 case 0x21:
  /* and r/m16, r16 */
  /* and r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  switch (flags & (F_OP16|F_LOCK)) {
  case 0:             CONTEXT_FLAGS(*context) |= vio_randl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF); break;
  case F_OP16:        CONTEXT_FLAGS(*context) |= vio_randw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF); break;
  case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_randl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF); break;
  case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_randw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF); break;
  default: __builtin_unreachable();
  }
  goto ok;

 case 0x22:
  /* and r8, r/m8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  CONTEXT_FLAGS(*context) |= vio_landb(ops,closure,addr,&MODRM_REG8(context,&modrm)) & (OF|CF|SF|ZF|PF);
  goto ok;

 case 0x23:
  /* and r/m16, r16 */
  /* and r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  if (flags & F_OP16) {
   CONTEXT_FLAGS(*context) |= vio_landw(ops,closure,addr,(u16 *)&MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_landl(ops,closure,addr,(u32 *)&MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF);
  }
  goto ok;

 case 0x28:
  /* sub r/m8, r8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
  if (flags & F_LOCK) {
   CONTEXT_FLAGS(*context) |= vio_atomic_rsubb(ops,closure,addr,MODRM_REG8(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_rsubb(ops,closure,addr,MODRM_REG8(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
  }
  goto ok;

 case 0x29:
  /* sub r/m16, r16 */
  /* sub r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
  switch (flags & (F_OP16|F_LOCK)) {
  case 0:             CONTEXT_FLAGS(*context) |= vio_rsubl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF); break;
  case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rsubw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF); break;
  case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rsubl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF); break;
  case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_rsubw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF); break;
  default: __builtin_unreachable();
  }
  goto ok;

 case 0x2a:
  /* sub r8, r/m8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
  CONTEXT_FLAGS(*context) |= vio_lsubb(ops,closure,addr,&MODRM_REG8(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
  goto ok;

 case 0x2b:
  /* sub r/m16, r16 */
  /* sub r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|CF|PF);
  if (flags & F_OP16) {
   CONTEXT_FLAGS(*context) |= vio_lsubw(ops,closure,addr,(u16 *)&MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_lsubl(ops,closure,addr,(u32 *)&MODRM_REG(context,&modrm)) & (OF|SF|ZF|AF|CF|PF);
  }
  goto ok;

 case 0x30:
  /* xor r/m8, r8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  if (flags & F_LOCK) {
   CONTEXT_FLAGS(*context) |= vio_atomic_rxorb(ops,closure,addr,MODRM_REG8(context,&modrm)) & (OF|CF|SF|ZF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_rxorb(ops,closure,addr,MODRM_REG8(context,&modrm)) & (OF|CF|SF|ZF|PF);
  }
  goto ok;

 case 0x31:
  /* xor r/m16, r16 */
  /* xor r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  switch (flags & (F_OP16|F_LOCK)) {
  case 0:             CONTEXT_FLAGS(*context) |= vio_rxorl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF); break;
  case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rxorw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF); break;
  case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rxorl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF); break;
  case F_OP16|F_LOCK: CONTEXT_FLAGS(*context) |= vio_atomic_rxorw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF); break;
  default: __builtin_unreachable();
  }
  goto ok;

 case 0x32:
  /* xor r8, r/m8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  CONTEXT_FLAGS(*context) |= vio_lxorb(ops,closure,addr,&MODRM_REG8(context,&modrm)) & (OF|CF|SF|ZF|PF);
  goto ok;

 case 0x33:
  /* xor r/m16, r16 */
  /* xor r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(OF|CF|SF|ZF|PF);
  if (flags & F_OP16) {
   CONTEXT_FLAGS(*context) |= vio_lxorw(ops,closure,addr,(u16 *)&MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_lxorl(ops,closure,addr,(u32 *)&MODRM_REG(context,&modrm)) & (OF|CF|SF|ZF|PF);
  }
  goto ok;

 case 0x38:
  /* cmp r/m8, r8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|AF|PF);
  CONTEXT_FLAGS(*context) |= vio_rcmpb(ops,closure,addr,MODRM_REG8(context,&modrm)) & (CF|OF|SF|ZF|AF|PF);
  goto ok;

 case 0x39:
  /* cmp r/m16, r16 */
  /* cmp r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|AF|PF);
  if (flags & F_OP16) {
   CONTEXT_FLAGS(*context) |= vio_rcmpw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (CF|OF|SF|ZF|AF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_rcmpl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (CF|OF|SF|ZF|AF|PF);
  }
  goto ok;

 case 0x3a:
  /* cmp r8, r/m8 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|AF|PF);
  CONTEXT_FLAGS(*context) |= vio_lcmpb(ops,closure,addr,MODRM_REG8(context,&modrm)) & (CF|OF|SF|ZF|AF|PF);
  goto ok;

 case 0x3b:
  /* cmp r/m16, r16 */
  /* cmp r/m32, r32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|AF|PF);
  if (flags & F_OP16) {
   CONTEXT_FLAGS(*context) |= vio_lcmpw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (CF|OF|SF|ZF|AF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_lcmpl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (CF|OF|SF|ZF|AF|PF);
  }
  goto ok;

 case 0xc0:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  {
   register_t new_flags;
   u8 num_bits;
  case 0:
   /* rol r/m8,imm8 */
   num_bits = *(u8 *)text;
   text += 1;
   if (flags & F_LOCK) {
    new_flags = vio_atomic_rrolb(ops,closure,addr,num_bits);
   } else {
    new_flags = vio_rrolb(ops,closure,addr,num_bits);
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 1:
   /* ror r/m8,imm8 */
   num_bits = *(u8 *)text;
   text += 1;
   if (flags & F_LOCK) {
    new_flags = vio_atomic_rrorb(ops,closure,addr,num_bits);
   } else {
    new_flags = vio_rrorb(ops,closure,addr,num_bits);
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 2:
   /* rcl r/m8,imm8 */
   num_bits = *(u8 *)text;
   text += 1;
   if (flags & F_LOCK) {
    new_flags = vio_atomic_rrclb(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits);
   } else {
    new_flags = vio_rrclb(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits);
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 3:
   /* rcr r/m8,imm8 */
   num_bits = *(u8 *)text;
   text += 1;
   if (flags & F_LOCK) {
    new_flags = vio_atomic_rrcrb(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits);
   } else {
    new_flags = vio_rrcrb(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits);
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 4:
   /* sal r/m8,imm8 */
   /* shl r/m8,imm8 */
   num_bits = *(u8 *)text;
   text += 1;
   if (flags & F_LOCK) {
    new_flags = vio_atomic_rshlb(ops,closure,addr,num_bits);
   } else {
    new_flags = vio_rshlb(ops,closure,addr,num_bits);
   }
   if (num_bits) {
    CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
    CONTEXT_FLAGS(*context) |= new_flags & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 5:
   /* shr r/m8,imm8 */
   num_bits = *(u8 *)text;
   text += 1;
   if (flags & F_LOCK) {
    new_flags = vio_atomic_rshrb(ops,closure,addr,num_bits);
   } else {
    new_flags = vio_rshrb(ops,closure,addr,num_bits);
   }
   if (num_bits) {
    CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
    CONTEXT_FLAGS(*context) |= new_flags & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 7:
   /* sar r/m8,imm8 */
   num_bits = *(u8 *)text;
   text += 1;
   if (flags & F_LOCK) {
    new_flags = vio_atomic_rsarb(ops,closure,addr,num_bits);
   } else {
    new_flags = vio_rsarb(ops,closure,addr,num_bits);
   }
   if (num_bits) {
    CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
    CONTEXT_FLAGS(*context) |= new_flags & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;
  }

  default: break;
  }
  break;

 case 0xc1:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  {
   register_t new_flags;
   u8 num_bits;
  case 0:
   /* rol r/m16,imm8 */
   /* rol r/m32,imm8 */
   num_bits = *(u8 *)text;
   text += 1;
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rroll(ops,closure,addr,num_bits); break;
   case F_OP16:        new_flags = vio_rrolw(ops,closure,addr,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rroll(ops,closure,addr,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rrolw(ops,closure,addr,num_bits); break;
   default: __builtin_unreachable();
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 1:
   /* ror r/m16,imm8 */
   /* ror r/m32,imm8 */
   num_bits = *(u8 *)text;
   text += 1;
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rrorl(ops,closure,addr,num_bits); break;
   case F_OP16:        new_flags = vio_rrorw(ops,closure,addr,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rrorl(ops,closure,addr,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rrorw(ops,closure,addr,num_bits); break;
   default: __builtin_unreachable();
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 2:
   /* rcl r/m16,imm8 */
   /* rcl r/m32,imm8 */
   num_bits = *(u8 *)text;
   text += 1;
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rrcll(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   case F_OP16:        new_flags = vio_rrclw(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rrcll(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rrclw(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   default: __builtin_unreachable();
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 3:
   /* rcr r/m16,imm8 */
   /* rcr r/m32,imm8 */
   num_bits = *(u8 *)text;
   text += 1;
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rrcrl(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   case F_OP16:        new_flags = vio_rrcrw(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rrcrl(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rrcrw(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   default: __builtin_unreachable();
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 4:
   /* sal r/m16,imm8 */
   /* sal r/m32,imm8 */
   /* shl r/m16,imm8 */
   /* shl r/m32,imm8 */
   num_bits = *(u8 *)text;
   text += 1;
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rshll(ops,closure,addr,num_bits); break;
   case F_OP16:        new_flags = vio_rshlw(ops,closure,addr,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rshll(ops,closure,addr,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rshlw(ops,closure,addr,num_bits); break;
   default: __builtin_unreachable();
   }
   if (num_bits) {
    CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
    CONTEXT_FLAGS(*context) |= new_flags & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 5:
   /* shr r/m16,imm16 */
   /* shr r/m32,imm32 */
   num_bits = *(u8 *)text;
   text += 1;
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rshrl(ops,closure,addr,num_bits); break;
   case F_OP16:        new_flags = vio_rshrw(ops,closure,addr,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rshrl(ops,closure,addr,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rshrw(ops,closure,addr,num_bits); break;
   default: __builtin_unreachable();
   }
   if (num_bits) {
    CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
    CONTEXT_FLAGS(*context) |= new_flags & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 7:
   /* sar r/m16,imm16 */
   /* sar r/m32,imm32 */
   num_bits = *(u8 *)text;
   text += 1;
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rsarl(ops,closure,addr,num_bits); break;
   case F_OP16:        new_flags = vio_rsarw(ops,closure,addr,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rsarl(ops,closure,addr,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rsarw(ops,closure,addr,num_bits); break;
   default: __builtin_unreachable();
   }
   if (num_bits) {
    CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
    CONTEXT_FLAGS(*context) |= new_flags & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;
  }

  default: break;
  }
  break;

 case 0xd0:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  {
   register_t new_flags;
  case 0:
   /* rol r/m8, 1 */
   if (flags & F_LOCK) {
    new_flags = vio_atomic_rrolb(ops,closure,addr,1);
   } else {
    new_flags = vio_rrolb(ops,closure,addr,1);
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
  case 1:
   /* ror r/m8, 1 */
   if (flags & F_LOCK) {
    new_flags = vio_atomic_rrorb(ops,closure,addr,1);
   } else {
    new_flags = vio_rrorb(ops,closure,addr,1);
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
  case 2:
   /* rcl r/m8, 1 */
   if (flags & F_LOCK) {
    new_flags = vio_atomic_rrclb(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,1);
   } else {
    new_flags = vio_rrclb(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,1);
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
  case 3:
   /* rcr r/m8, 1 */
   if (flags & F_LOCK) {
    new_flags = vio_atomic_rrcrb(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,1);
   } else {
    new_flags = vio_rrcrb(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,1);
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  case 4:
   /* shl r/m8 */
   /* sal r/m8 */
   CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_rshlb(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_rshlb(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;

  case 5:
   /* shr r/m8 */
   CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_rshrb(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_rshrb(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;

  case 7:
   /* sar r/m8 */
   CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
   if (flags & F_LOCK) {
    CONTEXT_FLAGS(*context) |= vio_atomic_rsarb(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_rsarb(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;

  default: break;
  }
  break;

 case 0xd1:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  {
   register_t new_flags;
  case 0:
   /* rol r/m16, 1 */
   /* rol r/m32, 1 */
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rroll(ops,closure,addr,1); break;
   case F_OP16:        new_flags = vio_rrolw(ops,closure,addr,1); break;
   case F_LOCK:        new_flags = vio_atomic_rroll(ops,closure,addr,1); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rrolw(ops,closure,addr,1); break;
   default: __builtin_unreachable();
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
  case 1:
   /* ror r/m16, 1 */
   /* ror r/m32, 1 */
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rrorl(ops,closure,addr,1); break;
   case F_OP16:        new_flags = vio_rrorw(ops,closure,addr,1); break;
   case F_LOCK:        new_flags = vio_atomic_rrorl(ops,closure,addr,1); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rrorw(ops,closure,addr,1); break;
   default: __builtin_unreachable();
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
  case 2:
   /* rcl r/m16, 1 */
   /* rcl r/m32, 1 */
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rrcll(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,1); break;
   case F_OP16:        new_flags = vio_rrclw(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,1); break;
   case F_LOCK:        new_flags = vio_atomic_rrcll(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,1); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rrclw(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,1); break;
   default: __builtin_unreachable();
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
  case 3:
   /* rcr r/m16, 1 */
   /* rcr r/m32, 1 */
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rrcrl(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,1); break;
   case F_OP16:        new_flags = vio_rrcrw(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,1); break;
   case F_LOCK:        new_flags = vio_atomic_rrcrl(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,1); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rrcrw(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,1); break;
   default: __builtin_unreachable();
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  case 4:
   /* shl r/m16 */
   /* shl r/m32 */
   /* sal r/m16 */
   /* sal r/m32 */
   CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_rshll(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF); break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rshlw(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rshll(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF); break;
   case F_LOCK|F_OP16: CONTEXT_FLAGS(*context) |= vio_atomic_rshlw(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF); break;
   default: __builtin_unreachable();
   }
   goto ok;

  case 5:
   /* shr r/m16 */
   /* shr r/m32 */
   CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_rshrl(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF); break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rshrw(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rshrl(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF); break;
   case F_LOCK|F_OP16: CONTEXT_FLAGS(*context) |= vio_atomic_rshrw(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF); break;
   default: __builtin_unreachable();
   }
   goto ok;

  case 7:
   /* sar r/m16 */
   /* sar r/m32 */
   CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_rsarl(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF); break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rsarw(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rsarl(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF); break;
   case F_LOCK|F_OP16: CONTEXT_FLAGS(*context) |= vio_atomic_rsarw(ops,closure,addr,1) & (CF|OF|SF|ZF|PF|AF); break;
   default: __builtin_unreachable();
   }
   goto ok;

  default: break;
  }
  break;

 case 0xd2:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  {
   register_t new_flags;
  case 0:
   /* rol r/m8,cl */
   new_flags = vio_rrolb(ops,closure,addr,
                        (u8)CONTEXT_CREG(*context));
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
  case 1:
   /* ror r/m8,cl */
   new_flags = vio_rrorb(ops,closure,addr,
                        (u8)CONTEXT_CREG(*context));
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
  case 2:
   /* rcl r/m8,cl */
   new_flags = vio_rrclb(ops,closure,addr,
                         CONTEXT_FLAGS(*context)&CF,
                        (u8)CONTEXT_CREG(*context));
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
  case 3:
   /* rcr r/m8,cl */
   new_flags = vio_rrcrb(ops,closure,addr,
                         CONTEXT_FLAGS(*context)&CF,
                        (u8)CONTEXT_CREG(*context));
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 4:
   /* sal r/m8,cl */
   /* shl r/m8,cl */
   num_bits = (u8)CONTEXT_CREG(*context);
   new_flags = vio_rshlb(ops,closure,addr,num_bits);
   if (num_bits) {
    CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
    CONTEXT_FLAGS(*context) |= new_flags & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 5:
   /* shr r/m8,cl */
   num_bits = (u8)CONTEXT_CREG(*context);
   new_flags = vio_rshrb(ops,closure,addr,num_bits);
   if (num_bits) {
    CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
    CONTEXT_FLAGS(*context) |= new_flags & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 7:
   /* sar r/m8,cl */
   num_bits = (u8)CONTEXT_CREG(*context);
   new_flags = vio_rsarb(ops,closure,addr,num_bits);
   if (num_bits) {
    CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
    CONTEXT_FLAGS(*context) |= new_flags & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;
  }

  default: break;
  }
  break;

 case 0xd3:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  {
   register_t new_flags;
   u8 num_bits;
  case 0:
   /* rol r/m16,cl */
   /* rol r/m32,cl */
   num_bits = (u8)CONTEXT_CREG(*context);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rroll(ops,closure,addr,num_bits); break;
   case F_OP16:        new_flags = vio_rrolw(ops,closure,addr,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rroll(ops,closure,addr,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rrolw(ops,closure,addr,num_bits); break;
   default: __builtin_unreachable();
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 1:
   /* ror r/m16,cl */
   /* ror r/m32,cl */
   num_bits = (u8)CONTEXT_CREG(*context);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rrorl(ops,closure,addr,num_bits); break;
   case F_OP16:        new_flags = vio_rrorw(ops,closure,addr,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rrorl(ops,closure,addr,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rrorw(ops,closure,addr,num_bits); break;
   default: __builtin_unreachable();
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 2:
   /* rcl r/m16,cl */
   /* rcl r/m32,cl */
   num_bits = (u8)CONTEXT_CREG(*context);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rrcll(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   case F_OP16:        new_flags = vio_rrclw(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rrcll(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rrclw(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   default: __builtin_unreachable();
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 3:
   /* rcr r/m16,cl */
   /* rcr r/m32,cl */
   num_bits = (u8)CONTEXT_CREG(*context);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rrcrl(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   case F_OP16:        new_flags = vio_rrcrw(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rrcrl(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rrcrw(ops,closure,addr,CONTEXT_FLAGS(*context)&CF,num_bits); break;
   default: __builtin_unreachable();
   }
   CONTEXT_FLAGS(*context) &= ~(CF|OF);
   CONTEXT_FLAGS(*context) |= new_flags & (CF|OF);
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 4:
   /* sal r/m16,cl */
   /* sal r/m32,cl */
   /* shl r/m16,cl */
   /* shl r/m32,cl */
   num_bits = (u8)CONTEXT_CREG(*context);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rshll(ops,closure,addr,num_bits); break;
   case F_OP16:        new_flags = vio_rshlw(ops,closure,addr,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rshll(ops,closure,addr,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rshlw(ops,closure,addr,num_bits); break;
   default: __builtin_unreachable();
   }
   if (num_bits) {
    CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
    CONTEXT_FLAGS(*context) |= new_flags & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 5:
   /* shr r/m16,cl */
   /* shr r/m32,cl */
   num_bits = (u8)CONTEXT_CREG(*context);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rshrl(ops,closure,addr,num_bits); break;
   case F_OP16:        new_flags = vio_rshrw(ops,closure,addr,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rshrl(ops,closure,addr,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rshrw(ops,closure,addr,num_bits); break;
   default: __builtin_unreachable();
   }
   if (num_bits) {
    CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
    CONTEXT_FLAGS(*context) |= new_flags & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;
  }

  {
   register_t new_flags;
   u8 num_bits;
  case 7:
   /* sar r/m16,cl */
   /* sar r/m32,cl */
   num_bits = (u8)CONTEXT_CREG(*context);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             new_flags = vio_rsarl(ops,closure,addr,num_bits); break;
   case F_OP16:        new_flags = vio_rsarw(ops,closure,addr,num_bits); break;
   case F_LOCK:        new_flags = vio_atomic_rsarl(ops,closure,addr,num_bits); break;
   case F_LOCK|F_OP16: new_flags = vio_atomic_rsarw(ops,closure,addr,num_bits); break;
   default: __builtin_unreachable();
   }
   if (num_bits) {
    CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|PF|AF);
    CONTEXT_FLAGS(*context) |= new_flags & (CF|OF|SF|ZF|PF|AF);
   }
   goto ok;
  }

  default: break;
  }
  break;

 case 0xd7:
  /* xlatb */
  *(u8 *)&CONTEXT_AREG(*context) = vio_readb(ops,closure,addr);
  goto ok;

  /* SETcc r/m8 */
 case 0x0f90: vio_writeb(ops,closure,addr,!!(CONTEXT_FLAGS(*context) & OF)); goto skip_modrm;
 case 0x0f91: vio_writeb(ops,closure,addr,!(CONTEXT_FLAGS(*context) & OF)); goto skip_modrm;
 case 0x0f92: vio_writeb(ops,closure,addr,!!(CONTEXT_FLAGS(*context) & CF)); goto skip_modrm;
 case 0x0f93: vio_writeb(ops,closure,addr,!(CONTEXT_FLAGS(*context) & CF)); goto skip_modrm;
 case 0x0f94: vio_writeb(ops,closure,addr,!!(CONTEXT_FLAGS(*context) & ZF)); goto skip_modrm;
 case 0x0f95: vio_writeb(ops,closure,addr,!(CONTEXT_FLAGS(*context) & ZF)); goto skip_modrm;
 case 0x0f96: vio_writeb(ops,closure,addr,!!(CONTEXT_FLAGS(*context) & (CF|ZF))); goto skip_modrm;
 case 0x0f97: vio_writeb(ops,closure,addr,!(CONTEXT_FLAGS(*context) & (CF|ZF))); goto skip_modrm;
 case 0x0f98: vio_writeb(ops,closure,addr,!!(CONTEXT_FLAGS(*context) & SF)); goto skip_modrm;
 case 0x0f99: vio_writeb(ops,closure,addr,!(CONTEXT_FLAGS(*context) & SF)); goto skip_modrm;
 case 0x0f9a: vio_writeb(ops,closure,addr,!!(CONTEXT_FLAGS(*context) & PF)); goto skip_modrm;
 case 0x0f9b: vio_writeb(ops,closure,addr,!(CONTEXT_FLAGS(*context) & PF)); goto skip_modrm;
 case 0x0f9c: vio_writeb(ops,closure,addr,!!(CONTEXT_FLAGS(*context) & SF) !=
                                          !!(CONTEXT_FLAGS(*context) & OF)); goto skip_modrm;
 case 0x0f9d: vio_writeb(ops,closure,addr,!!(CONTEXT_FLAGS(*context) & SF) ==
                                          !!(CONTEXT_FLAGS(*context) & OF)); goto skip_modrm;
 case 0x0f9e: vio_writeb(ops,closure,addr,!!(CONTEXT_FLAGS(*context) & ZF) ||
                                         (!!(CONTEXT_FLAGS(*context) & SF) !=
                                          !!(CONTEXT_FLAGS(*context) & OF))); goto skip_modrm;
 case 0x0f9f: vio_writeb(ops,closure,addr,!(CONTEXT_FLAGS(*context) & ZF) ||
                                         (!!(CONTEXT_FLAGS(*context) & SF) ==
                                          !!(CONTEXT_FLAGS(*context) & OF)));
skip_modrm:
  text = x86_decode_modrm(text,&modrm);
  goto ok;

 case 0x0f40 ... 0x0f4f:
  /* MOVcc r16, r/m16 */
  /* MOVcc r32, r/m32 */
  text = x86_decode_modrm(text,&modrm);
  /* NOTE: Because we are here, we already know that the condition
   *       must be true, because otherwise there wouldn't have been
   *       an attempt to write data.  */
  if (flags & F_OP16) {
   vio_writew(ops,closure,addr,(u16)MODRM_REG(context,&modrm));
  } else {
   vio_writel(ops,closure,addr,(u32)MODRM_REG(context,&modrm));
  }
  goto ok;

 case 0x0fba:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  case 4:
   /* bt r/m16, imm8 */
   /* bt r/m32, imm8 */
   CONTEXT_FLAGS(*context) &= ~(CF);
   if (flags & F_OP16) {
    CONTEXT_FLAGS(*context) |= vio_rbtw(ops,closure,addr,*(u8 *)text) & (CF);
   } else {
    CONTEXT_FLAGS(*context) |= vio_rbtl(ops,closure,addr,*(u8 *)text) & (CF);
   }
   text += 1;
   goto ok;

  case 5:
   /* bts r/m16, imm8 */
   /* bts r/m32, imm8 */
   CONTEXT_FLAGS(*context) &= ~(CF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_rbtsl(ops,closure,addr,*(u8 *)text) & (CF); break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rbtsw(ops,closure,addr,*(u8 *)text) & (CF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rbtsl(ops,closure,addr,*(u8 *)text) & (CF); break;
   case F_LOCK|F_OP16: CONTEXT_FLAGS(*context) |= vio_atomic_rbtsw(ops,closure,addr,*(u8 *)text) & (CF); break;
   default: __builtin_unreachable();
   }
   text += 1;
   goto ok;

  case 6:
   /* btr r/m16, imm8 */
   /* btr r/m32, imm8 */
   CONTEXT_FLAGS(*context) &= ~(CF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_rbtrl(ops,closure,addr,*(u8 *)text) & (CF); break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rbtrw(ops,closure,addr,*(u8 *)text) & (CF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rbtrl(ops,closure,addr,*(u8 *)text) & (CF); break;
   case F_LOCK|F_OP16: CONTEXT_FLAGS(*context) |= vio_atomic_rbtrw(ops,closure,addr,*(u8 *)text) & (CF); break;
   default: __builtin_unreachable();
   }
   text += 1;
   goto ok;

  case 7:
   /* btc r/m16, imm8 */
   /* btc r/m32, imm8 */
   CONTEXT_FLAGS(*context) &= ~(CF);
   switch (flags & (F_OP16|F_LOCK)) {
   case 0:             CONTEXT_FLAGS(*context) |= vio_rbtcl(ops,closure,addr,*(u8 *)text) & (CF); break;
   case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rbtcw(ops,closure,addr,*(u8 *)text) & (CF); break;
   case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rbtcl(ops,closure,addr,*(u8 *)text) & (CF); break;
   case F_LOCK|F_OP16: CONTEXT_FLAGS(*context) |= vio_atomic_rbtcw(ops,closure,addr,*(u8 *)text) & (CF); break;
   default: __builtin_unreachable();
   }
   text += 1;
   goto ok;

  default: break;
  }
  break;

 case 0x0fb6:
  /* movzx r16,r/m8 */
  /* movzx r32,r/m8 */
  text = x86_decode_modrm(text,&modrm);
  if (flags & F_OP16) {
   *(u16 *)&MODRM_REG(context,&modrm) = (u16)vio_readb(ops,closure,addr);
  } else {
   *(u32 *)&MODRM_REG(context,&modrm) = (u32)vio_readb(ops,closure,addr);
  }
  goto ok;

 case 0x0fb7:
  /* movzx r32,r/m16 */
  text = x86_decode_modrm(text,&modrm);
  *(u32 *)&MODRM_REG(context,&modrm) = (u32)vio_readw(ops,closure,addr);
  goto ok;

 case 0x0fbe:
  /* movsx r16,r/m8 */
  /* movsx r32,r/m8 */
  text = x86_decode_modrm(text,&modrm);
  if (flags & F_OP16) {
   *(s16 *)&MODRM_REG(context,&modrm) = (s16)(s8)vio_readb(ops,closure,addr);
  } else {
   *(s32 *)&MODRM_REG(context,&modrm) = (s32)(s8)vio_readb(ops,closure,addr);
  }
  goto ok;

 case 0x0fbf:
  /* movsx r32,r/m16 */
  text = x86_decode_modrm(text,&modrm);
  *(s32 *)&MODRM_REG(context,&modrm) = (s32)(s16)vio_readw(ops,closure,addr);
  goto ok;

 case 0x0fa3:
  text = x86_decode_modrm(text,&modrm);
  /* bt r/m16, r16 */
  /* bt r/m32, r32 */
  CONTEXT_FLAGS(*context) &= ~(CF);
  if (flags & F_OP16) {
   CONTEXT_FLAGS(*context) |= vio_rbtw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (CF);
  } else {
   CONTEXT_FLAGS(*context) |= vio_rbtl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (CF);
  }
  goto ok;

 case 0x0fb3:
  text = x86_decode_modrm(text,&modrm);
  /* btr r/m16, r16 */
  /* btr r/m32, r32 */
  CONTEXT_FLAGS(*context) &= ~(CF);
  switch (flags & (F_OP16|F_LOCK)) {
  case 0:             CONTEXT_FLAGS(*context) |= vio_rbtrl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (CF); break;
  case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rbtrw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (CF); break;
  case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rbtrl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (CF); break;
  case F_LOCK|F_OP16: CONTEXT_FLAGS(*context) |= vio_atomic_rbtrw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (CF); break;
  default: __builtin_unreachable();
  }
  goto ok;

 case 0x0fbb:
  text = x86_decode_modrm(text,&modrm);
  /* btc r/m16, r16 */
  /* btc r/m32, r32 */
  CONTEXT_FLAGS(*context) &= ~(CF);
  switch (flags & (F_OP16|F_LOCK)) {
  case 0:             CONTEXT_FLAGS(*context) |= vio_rbtcl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (CF); break;
  case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rbtcw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (CF); break;
  case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rbtcl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (CF); break;
  case F_LOCK|F_OP16: CONTEXT_FLAGS(*context) |= vio_atomic_rbtcw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (CF); break;
  default: __builtin_unreachable();
  }
  goto ok;

 case 0x0fab:
  text = x86_decode_modrm(text,&modrm);
  /* bts r/m16, r16 */
  /* bts r/m32, r32 */
  CONTEXT_FLAGS(*context) &= ~(CF);
  switch (flags & (F_OP16|F_LOCK)) {
  case 0:             CONTEXT_FLAGS(*context) |= vio_rbtsl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (CF); break;
  case F_OP16:        CONTEXT_FLAGS(*context) |= vio_rbtsw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (CF); break;
  case F_LOCK:        CONTEXT_FLAGS(*context) |= vio_atomic_rbtsl(ops,closure,addr,(u32)MODRM_REG(context,&modrm)) & (CF); break;
  case F_LOCK|F_OP16: CONTEXT_FLAGS(*context) |= vio_atomic_rbtsw(ops,closure,addr,(u16)MODRM_REG(context,&modrm)) & (CF); break;
  default: __builtin_unreachable();
  }
  goto ok;

 case 0xaa:
  /* stosb */
  vio_writeb(ops,closure,addr,(u8)CONTEXT_AREG(*context));
  if (CONTEXT_FLAGS(*context) & DF) {
   ++CONTEXT_DIREG(*context);
   ++CONTEXT_SIREG(*context);
  } else {
   --CONTEXT_DIREG(*context);
   --CONTEXT_SIREG(*context);
  }
  goto ok_handle_rep;

 case 0xab:
  /* stosw */
  /* stosl */
  if (flags & F_OP16) {
   vio_writew(ops,closure,addr,(u16)CONTEXT_AREG(*context));
   if (CONTEXT_FLAGS(*context) & DF) {
    CONTEXT_DIREG(*context) -= 2;
    CONTEXT_SIREG(*context) -= 2;
   } else {
    CONTEXT_DIREG(*context) += 2;
    CONTEXT_SIREG(*context) += 2;
   }
  } else {
   vio_writel(ops,closure,addr,(u32)CONTEXT_AREG(*context));
   if (CONTEXT_FLAGS(*context) & DF) {
    CONTEXT_DIREG(*context) -= 4;
    CONTEXT_SIREG(*context) -= 4;
   } else {
    CONTEXT_DIREG(*context) += 4;
    CONTEXT_SIREG(*context) += 4;
   }
  }
  goto ok_handle_rep;

 case 0xac:
  /* lodsb */
  *(u8 *)&CONTEXT_AREG(*context) = vio_readb(ops,closure,addr);
  if (CONTEXT_FLAGS(*context) & DF) {
   --CONTEXT_SIREG(*context);
  } else {
   ++CONTEXT_SIREG(*context);
  }
  goto ok_handle_rep;

 case 0xad:
  /* lodsw */
  /* lodsl */
  if (flags & F_OP16) {
   *(u16 *)&CONTEXT_AREG(*context) = vio_readw(ops,closure,addr);
   if (CONTEXT_FLAGS(*context) & DF) {
    CONTEXT_DIREG(*context) -= 2;
   } else {
    CONTEXT_DIREG(*context) += 2;
   }
  } else {
   *(u32 *)&CONTEXT_AREG(*context) = vio_readl(ops,closure,addr);
   if (CONTEXT_FLAGS(*context) & DF) {
    CONTEXT_DIREG(*context) -= 4;
   } else {
    CONTEXT_DIREG(*context) += 4;
   }
  }
  goto ok_handle_rep;

 case 0xae:
  /* scasb */
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|PF|CF);
  CONTEXT_FLAGS(*context) |= x86_cmpb(*(u8 *)&CONTEXT_AREG(*context),vio_readb(ops,closure,addr));
  if (CONTEXT_FLAGS(*context) & DF) {
   --CONTEXT_DIREG(*context);
  } else {
   ++CONTEXT_DIREG(*context);
  }
  goto ok_handle_repe;

 case 0xaf:
  /* scasw */
  /* scasl */
  CONTEXT_FLAGS(*context) &= ~(OF|SF|ZF|AF|PF|CF);
  if (flags & F_OP16) {
   CONTEXT_FLAGS(*context) |= x86_cmpw(*(u16 *)&CONTEXT_AREG(*context),
                                       vio_readw(ops,closure,addr));
   if (CONTEXT_FLAGS(*context) & DF) {
    CONTEXT_DIREG(*context) -= 2;
   } else {
    CONTEXT_DIREG(*context) += 2;
   }
  } else {
   CONTEXT_FLAGS(*context) |= x86_cmpl(*(u32 *)&CONTEXT_AREG(*context),
                                       vio_readl(ops,closure,addr));
   if (CONTEXT_FLAGS(*context) & DF) {
    CONTEXT_DIREG(*context) -= 4;
   } else {
    CONTEXT_DIREG(*context) += 4;
   }
  }
  goto ok_handle_repe;

 case 0xa6:
  /* cmpsb */
  CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|AF|PF);
  if ((uintptr_t)abs_addr == CONTEXT_DIREG(*context)) {
   CONTEXT_FLAGS(*context) |= x86_cmpb(*(u8 *)CONTEXT_SIREG(*context),
                                       vio_readb(ops,closure,addr)) &
                                      (CF|OF|SF|ZF|AF|PF);
  } else {
   CONTEXT_FLAGS(*context) |= x86_cmpb(vio_readb(ops,closure,addr),
                                       *(u8 *)CONTEXT_DIREG(*context)) &
                                      (CF|OF|SF|ZF|AF|PF);
  }
  if (CONTEXT_FLAGS(*context) & DF) {
   CONTEXT_DIREG(*context) -= 1;
   CONTEXT_SIREG(*context) -= 1;
  } else {
   CONTEXT_DIREG(*context) += 1;
   CONTEXT_SIREG(*context) += 1;
  }
  goto ok_handle_repe;

 case 0xa7:
  /* cmpsw */
  /* cmpsl */
  CONTEXT_FLAGS(*context) &= ~(CF|OF|SF|ZF|AF|PF);
  if (flags & F_OP16) {
   if ((uintptr_t)abs_addr == CONTEXT_DIREG(*context)) {
    CONTEXT_FLAGS(*context) |= x86_cmpw(*(u16 *)CONTEXT_SIREG(*context),
                                        vio_readw(ops,closure,addr)) &
                                       (CF|OF|SF|ZF|AF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= x86_cmpw(vio_readw(ops,closure,addr),
                                        *(u16 *)CONTEXT_DIREG(*context)) &
                                       (CF|OF|SF|ZF|AF|PF);
   }
   CONTEXT_DIREG(*context) += 2;
   CONTEXT_SIREG(*context) += 2;
  } else {
   if ((uintptr_t)abs_addr == CONTEXT_DIREG(*context)) {
    CONTEXT_FLAGS(*context) |= x86_cmpl(*(u32 *)CONTEXT_SIREG(*context),
                                        vio_readl(ops,closure,addr)) &
                                       (CF|OF|SF|ZF|AF|PF);
   } else {
    CONTEXT_FLAGS(*context) |= x86_cmpl(vio_readl(ops,closure,addr),
                                        *(u32 *)CONTEXT_DIREG(*context)) &
                                       (CF|OF|SF|ZF|AF|PF);
   }
   CONTEXT_DIREG(*context) += 4;
   CONTEXT_SIREG(*context) += 4;
  }
  goto ok_handle_repe;

 case 0x0fbc:
  /* bsf r16, r/m16 */
  /* bsf r32, r/m32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(ZF);
  if (flags & F_OP16) {
   CONTEXT_FLAGS(*context) |= x86_bsfw(vio_readw(ops,closure,addr),
                                      (u16 *)&MODRM_REG(context,&modrm)) & ZF;
  } else {
   CONTEXT_FLAGS(*context) |= x86_bsfl(vio_readl(ops,closure,addr),
                                      (u32 *)&MODRM_REG(context,&modrm)) & ZF;
  }
  goto ok;

 case 0x0fbd:
  /* bsr r16, r/m16 */
  /* bsr r32, r/m32 */
  text = x86_decode_modrm(text,&modrm);
  CONTEXT_FLAGS(*context) &= ~(ZF);
  if (flags & F_OP16) {
   CONTEXT_FLAGS(*context) |= x86_bsrw(vio_readw(ops,closure,addr),
                                      (u16 *)&MODRM_REG(context,&modrm)) & ZF;
  } else {
   CONTEXT_FLAGS(*context) |= x86_bsrl(vio_readl(ops,closure,addr),
                                      (u32 *)&MODRM_REG(context,&modrm)) & ZF;
  }
  goto ok;

 case 0x0f00:
  text = x86_decode_modrm(text,&modrm);
  switch (modrm.mi_rm) {

  case 4:
   /* verr r/m16 */
   CONTEXT_FLAGS(*context) &= ~(ZF);
   if (__verr(vio_readw(ops,closure,addr)))
       CONTEXT_FLAGS(*context) |= ZF;
   break;

  case 5:
   /* verw r/m16 */
   CONTEXT_FLAGS(*context) &= ~(ZF);
   if (__verw(vio_readw(ops,closure,addr)))
       CONTEXT_FLAGS(*context) |= ZF;
   break;

  default: break;
  }
  break;

 default: break;
 }
fail:
 return false;
ok_handle_rep:
 if (flags & F_REP) {
  if (--CONTEXT_CREG(*context) != 0)
      goto ok_noip;
 }
 goto ok;
ok_handle_repe:
 if (flags & (F_REP|F_REPNE)) {
  if (--CONTEXT_CREG(*context) != 0) {
   if (flags & F_REPNE) {
    if (!(CONTEXT_FLAGS(*context) & ZF))
          goto ok_noip;
   } else {
    if (CONTEXT_FLAGS(*context) & ZF)
        goto ok_noip;
   }
  }
 }
ok:
 CPU_CONTEXT_IP(*context) = (uintptr_t)text;
ok_noip:
 return true;
}



DECL_END

#endif /* !CONFIG_NO_VIO */

#endif /* !GUARD_KERNEL_I386_KOS_VIO_C */
