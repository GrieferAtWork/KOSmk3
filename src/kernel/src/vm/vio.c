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
#ifndef GUARD_KERNEL_SRC_VM_VIO_C
#define GUARD_KERNEL_SRC_VM_VIO_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kernel/vm.h>
#include <except.h>

#ifndef CONFIG_NO_VIO
DECL_BEGIN

#ifndef CONFIG_VM_ALLOW_UNALIGNED_VIO
/* TODO */
#else

PRIVATE ATTR_NORETURN void KCALL vio_not_implemented(void) {
 error_throw(E_NOT_IMPLEMENTED);
}

PRIVATE ATTR_NORETURN void KCALL vio_not_aligned(void) {
 error_throw(E_INVALID_ALIGNMENT);
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
        u16   w16_unaligned_40;
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
union PACKED word128 {
    u64 w64[2];
    u8  w8[16];
    u16 w16[8];
    u32 w32[4];
    struct PACKED {
        u8  __pad1;
        u32   w32_unaligned_8;
        u16   w16_unaligned_40;
        u16   w16_unaligned_56;
        u16   w16_unaligned_72;
        u16   w16_unaligned_88;
        u16   w16_unaligned_104;
    };
    struct PACKED {
        u16 __pad2;
        u32   w32_unaligned_16;
        u32   w32_unaligned_48;
        u32   w32_unaligned_80;
    };
    struct PACKED {
        u8  __pad3;
        u16   w16_unaligned_8;
        union PACKED {
        u16   w16_unaligned_24;
        u32   w32_unaligned_24;
        };
        u32   w32_unaligned_56;
        u32   w32_unaligned_88;
    };
    struct PACKED {
        u8  __pad4[5];
        u32   w32_unaligned_40;
        u32   w32_unaligned_72;
    };
    struct PACKED {
        u8  __pad5;
        u64   w64_unaligned_8;
    };
    struct PACKED {
        u8  __pad6[2];
        u64   w64_unaligned_16;
    };
    struct PACKED {
        u8  __pad7[3];
        u64   w64_unaligned_24;
    };
    struct PACKED {
        u8  __pad8[4];
        u64   w64_unaligned_32;
    };
    struct PACKED {
        u8  __pad9[5];
        u64   w64_unaligned_40;
    };
    struct PACKED {
        u8  __pad10[6];
        u64   w64_unaligned_48;
    };
    struct PACKED {
        u8  __pad11[7];
        u64   w64_unaligned_56;
    };
};


STATIC_ASSERT(sizeof(union word16) == 2);

STATIC_ASSERT(sizeof(union word32) == 4);
STATIC_ASSERT(offsetof(union word32,w16_unaligned_8)  == 1);

STATIC_ASSERT(sizeof(union word64) == 8);
STATIC_ASSERT(offsetof(union word64,w16_unaligned_8)  == 1);
STATIC_ASSERT(offsetof(union word64,w16_unaligned_24) == 3);
STATIC_ASSERT(offsetof(union word64,w16_unaligned_40) == 5);
STATIC_ASSERT(offsetof(union word64,w32_unaligned_8)  == 1);
STATIC_ASSERT(offsetof(union word64,w32_unaligned_16) == 2);
STATIC_ASSERT(offsetof(union word64,w32_unaligned_24) == 3);

STATIC_ASSERT(sizeof(union word128) == 16);
STATIC_ASSERT(offsetof(union word128,w16_unaligned_8)  == 1);
STATIC_ASSERT(offsetof(union word128,w16_unaligned_24) == 3);
STATIC_ASSERT(offsetof(union word128,w16_unaligned_40) == 5);
STATIC_ASSERT(offsetof(union word128,w32_unaligned_8)  == 1);
STATIC_ASSERT(offsetof(union word128,w32_unaligned_16) == 2);
STATIC_ASSERT(offsetof(union word128,w32_unaligned_24) == 3);
STATIC_ASSERT(offsetof(union word128,w32_unaligned_40) == 5);
STATIC_ASSERT(offsetof(union word128,w32_unaligned_48) == 6);
STATIC_ASSERT(offsetof(union word128,w32_unaligned_56) == 7);
STATIC_ASSERT(offsetof(union word128,w32_unaligned_72) == 9);
STATIC_ASSERT(offsetof(union word128,w32_unaligned_80) == 10);
STATIC_ASSERT(offsetof(union word128,w32_unaligned_88) == 11);
STATIC_ASSERT(offsetof(union word128,w64_unaligned_8)  == 1);
STATIC_ASSERT(offsetof(union word128,w64_unaligned_16) == 2);
STATIC_ASSERT(offsetof(union word128,w64_unaligned_24) == 3);
STATIC_ASSERT(offsetof(union word128,w64_unaligned_32) == 4);
STATIC_ASSERT(offsetof(union word128,w64_unaligned_40) == 5);
STATIC_ASSERT(offsetof(union word128,w64_unaligned_48) == 6);
STATIC_ASSERT(offsetof(union word128,w64_unaligned_56) == 7);


PUBLIC u8 KCALL
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
#if __SIZEOF_POINTER__ > 4
 if (ops->v_readq) {
  union word64 word;
  word.w64 = (*ops->v_readq)(closure,addr & ~7);
  return word.w8[addr & 7];
 }
#endif
 vio_not_implemented();
}
PUBLIC u16 KCALL
vio_readw(struct vio_ops *__restrict ops,
          void *closure, uintptr_t addr) {
 if (ops->v_readw && !(addr & 1))
     return (*ops->v_readw)(closure,addr);
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
 if (ops->v_readl) {
  union word64 word;
  word.w32[0] = (*ops->v_readl)(closure,addr & ~3);
  if (!(addr & 1)) return word.w16[(addr & 3) >> 1];
  switch (addr & 3) {
  case 3:
   word.w32[1] = (*ops->v_readl)(closure,addr & ~3);
   return word.w32_unaligned_24;
  case 1: return word.w32_unaligned_8;
  default: __builtin_unreachable();
  }
 }
#if __SIZEOF_POINTER__ >= 8
 if (ops->v_readq) {
  union word128 word;
  word.w64[0] = (*ops->v_readq)(closure,addr & ~7);
  if (!(addr & 1)) return word.w16[(addr & 7) >> 1];
  switch (addr & 7) {
  case 7:
   word.w64[1] = (*ops->v_readq)(closure,(addr & ~7)+8);
   return word.w16_unaligned_56;
  case 5: return word.w16_unaligned_40;
  case 3: return word.w16_unaligned_24;
  case 1: return word.w16_unaligned_8;
  default: __builtin_unreachable();
  }
 }
#endif
 vio_not_implemented();
}
PUBLIC u32 KCALL
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
#if __SIZEOF_POINTER__ >= 8
 if (ops->v_readq) {
  union word128 word;
  word.w64[0] = (*ops->v_readq)(closure,addr & ~7);
  switch (addr & 7) {
  case 0: return word.w32[0];
  case 1: return word.w32_unaligned_8;
  case 2: return word.w32_unaligned_16;
  case 3: return word.w32_unaligned_24;
  case 4: return word.w32[1];
  case 5:
   word.w64[1] = (*ops->v_readq)(closure,(addr & ~7)+8);
   return word.w32_unaligned_40;
  case 6:
   word.w64[1] = (*ops->v_readq)(closure,(addr & ~7)+8);
   return word.w32_unaligned_48;
  case 7:
   word.w64[1] = (*ops->v_readq)(closure,(addr & ~7)+8);
   return word.w32_unaligned_56;
  default: __builtin_unreachable();
  }
 }
#endif
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

#if __SIZEOF_POINTER__ >= 8
PUBLIC u64 KCALL
vio_readq(struct vio_ops *__restrict ops,
          void *closure, uintptr_t addr) {
 if (ops->v_readq) {
  union word128 word;
  if (!(addr & 7)) return (*ops->v_readq)(closure,addr);
  word.w64[0] = (*ops->v_readq)(closure,addr & ~7);
  word.w64[1] = (*ops->v_readq)(closure,(addr & ~7)+8);
  switch (addr & 7) {
  case 1: return word.w64_unaligned_8;
  case 2: return word.w64_unaligned_16;
  case 3: return word.w64_unaligned_24;
  case 4: return word.w64_unaligned_32;
  case 5: return word.w64_unaligned_40;
  case 6: return word.w64_unaligned_48;
  case 7: return word.w64_unaligned_56;
  default: __builtin_unreachable();
  }
 }
 if (ops->v_readl) {
  union word128 word;
  if (!(addr & 3)) {
   word.w32[0] = (*ops->v_readl)(closure,addr);
   word.w32[1] = (*ops->v_readl)(closure,addr+4);
   return word.w64[0];
  }
  word.w32[0] = (*ops->v_readl)(closure,addr & ~3);
  word.w32[1] = (*ops->v_readl)(closure,(addr & ~3)+4);
  word.w32[2] = (*ops->v_readl)(closure,(addr & ~3)+8);
  switch (addr&3) {
  case 1: return word.w64_unaligned_8;
  case 2: return word.w64_unaligned_16;
  case 3: return word.w64_unaligned_24;
  default: __builtin_unreachable();
  }
 }
 if (ops->v_readw) {
  if (!(addr & 1)) {
   union word64 word;
   word.w16[0] = (*ops->v_readw)(closure,addr);
   word.w16[1] = (*ops->v_readw)(closure,addr+2);
   word.w16[2] = (*ops->v_readw)(closure,addr+4);
   word.w16[3] = (*ops->v_readw)(closure,addr+8);
   return word.w64;
  } else {
   union word128 word;
   word.w16[0] = (*ops->v_readw)(closure,addr-1);
   word.w16[1] = (*ops->v_readw)(closure,addr+1);
   word.w16[2] = (*ops->v_readw)(closure,addr+3);
   word.w16[3] = (*ops->v_readw)(closure,addr+5);
   word.w16[4] = (*ops->v_readw)(closure,addr+7);
   return word.w64_unaligned_8;
  }
 }
 if (ops->v_readb) {
  union word64 word;
  word.w8[0] = (*ops->v_readb)(closure,addr);
  word.w8[1] = (*ops->v_readb)(closure,addr+1);
  word.w8[2] = (*ops->v_readb)(closure,addr+2);
  word.w8[3] = (*ops->v_readb)(closure,addr+3);
  word.w8[4] = (*ops->v_readb)(closure,addr+4);
  word.w8[5] = (*ops->v_readb)(closure,addr+5);
  word.w8[6] = (*ops->v_readb)(closure,addr+6);
  word.w8[7] = (*ops->v_readb)(closure,addr+7);
  return word.w64;
 }
 vio_not_implemented();
}
#endif

PUBLIC void KCALL
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
#if __SIZEOF_POINTER__ >= 8
  if (ops->v_readq) {
   union word64 word;
   word.w64 = (*ops->v_readq)(closure,addr & ~7);
   word.w8[addr & 7] = value;
   (*ops->v_writew)(closure,addr & ~1,word.w16[(addr & 6) >> 1]);
   return;
  }
#endif
 }
 if (ops->v_writel) {
  if (ops->v_readl) {
   union word32 word;
   word.w32 = (*ops->v_readl)(closure,addr & ~3);
   word.w8[addr & 3] = value;
   (*ops->v_writel)(closure,addr & ~3,word.w32);
   return;
  }
#if __SIZEOF_POINTER__ >= 8
  if (ops->v_readq) {
   union word64 word;
   word.w64 = (*ops->v_readq)(closure,addr & ~7);
   word.w8[addr & 7] = value;
   (*ops->v_writel)(closure,addr & ~3,word.w32[(addr & 4) >> 2]);
   return;
  }
#endif
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
#if __SIZEOF_POINTER__ >= 8
 if (ops->v_writeq) {
  if (ops->v_readq) {
   union word64 word;
   word.w64 = (*ops->v_readq)(closure,addr & ~7);
   word.w8[addr & 7] = value;
   (*ops->v_writeq)(closure,addr & ~7,word.w64);
   return;
  }
  if (ops->v_readl) {
   union word64 word;
   word.w32[0] = (*ops->v_readl)(closure,addr & ~7);
   word.w32[1] = (*ops->v_readl)(closure,(addr & ~7)+4);
   word.w8[addr & 7] = value;
   (*ops->v_writeq)(closure,addr & ~7,word.w64);
   return;
  }
  if (ops->v_readw) {
   union word64 word;
   word.w16[0] = (*ops->v_readw)(closure,addr & ~7);
   word.w16[1] = (*ops->v_readw)(closure,(addr & ~7)+2);
   word.w16[2] = (*ops->v_readw)(closure,(addr & ~7)+4);
   word.w16[3] = (*ops->v_readw)(closure,(addr & ~7)+6);
   word.w8[addr & 7] = value;
   (*ops->v_writeq)(closure,addr & ~7,word.w64);
   return;
  }
  if (ops->v_readb) {
   union word64 word;
   if ((addr & 7) != 0) word.w8[0] = (*ops->v_readb)(closure,addr & ~7);
   if ((addr & 7) != 1) word.w8[1] = (*ops->v_readb)(closure,(addr & ~7)+1);
   if ((addr & 7) != 2) word.w8[2] = (*ops->v_readb)(closure,(addr & ~7)+2);
   if ((addr & 7) != 3) word.w8[3] = (*ops->v_readb)(closure,(addr & ~7)+3);
   if ((addr & 7) != 4) word.w8[4] = (*ops->v_readb)(closure,(addr & ~7)+4);
   if ((addr & 7) != 5) word.w8[5] = (*ops->v_readb)(closure,(addr & ~7)+5);
   if ((addr & 7) != 6) word.w8[6] = (*ops->v_readb)(closure,(addr & ~7)+6);
   if ((addr & 7) != 7) word.w8[7] = (*ops->v_readb)(closure,(addr & ~7)+7);
   word.w8[addr & 7] = value;
   (*ops->v_writeq)(closure,addr & ~7,word.w64);
   return;
  }
 }
#endif
 vio_not_implemented();
}
PUBLIC void KCALL
vio_writew(struct vio_ops *__restrict ops,
           void *closure, uintptr_t addr, u16 value) {
 /* Unaligned write. */
#if __SIZEOF_POINTER__ >= 8
 switch (addr & 7)
#else
 switch (addr & 3)
#endif
 {

 case 0:
 case 2:
#if __SIZEOF_POINTER__ >= 8
 case 4:
 case 6:
#endif
  if (ops->v_writew) {
   (*ops->v_writew)(closure,addr,value);
   return;
  }
  if (ops->v_writel) {
   if (ops->v_readw) {
    union word32 word;
    if (addr & 2) {
     word.w16[0] = (*ops->v_readw)(closure,addr);
     word.w16[1] = value;
    } else {
     word.w16[0] = value;
     word.w16[1] = (*ops->v_readw)(closure,addr+2);
    }
    (*ops->v_writel)(closure,addr & ~3,word.w32);
    return;
   }
   if (ops->v_readl) {
    union word32 word;
    word.w32 = (*ops->v_readl)(closure,addr & ~3);
    word.w16[(addr & 2) >> 1] = value;
    (*ops->v_writel)(closure,addr & ~3,word.w32);
    return;
   }
#if __SIZEOF_POINTER__ >= 8
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr & ~7);
    word.w16[(addr & 6) >> 1] = value;
    if (addr & 4) {
     (*ops->v_writel)(closure,addr & ~3,word.w32[1]);
    } else {
     (*ops->v_writel)(closure,addr & ~7,word.w32[0]);
    }
    return;
   }
#endif
   if (ops->v_readb) {
    union word32 word;
    if (addr & 2) {
     word.w8[0]  = (*ops->v_readb)(closure,addr-2);
     word.w8[1]  = (*ops->v_readb)(closure,addr-1);
     word.w16[1] = value;
    } else {
     word.w16[0] = value;
     word.w8[2]  = (*ops->v_readb)(closure,addr+2);
     word.w8[3]  = (*ops->v_readb)(closure,addr+3);
    }
    (*ops->v_writel)(closure,addr & ~3,word.w32);
    return;
   }
  }
#if __SIZEOF_POINTER__ >= 8
  if (ops->v_writeq) {
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr & ~7);
    word.w16[(addr & 6) >> 1] = value;
    (*ops->v_writeq)(closure,addr & ~7,word.w64);
    return;
   }
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr & ~7);
    word.w32[1] = (*ops->v_readl)(closure,(addr & ~7)+4);
    word.w16[(addr & 6) >> 1] = value;
    (*ops->v_writeq)(closure,addr & ~7,word.w64);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    if ((addr & 6) != 0) word.w16[0] = (*ops->v_readl)(closure,addr & ~7);
    if ((addr & 6) != 2) word.w16[1] = (*ops->v_readl)(closure,(addr & ~7)+2);
    if ((addr & 6) != 4) word.w16[2] = (*ops->v_readl)(closure,(addr & ~7)+4);
    if ((addr & 6) != 6) word.w16[3] = (*ops->v_readl)(closure,(addr & ~7)+6);
    word.w16[(addr & 6) >> 1] = value;
    (*ops->v_writeq)(closure,addr & ~7,word.w64);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    if ((addr & 6) != 0)
         word.w8[0] = (*ops->v_readb)(closure,addr & ~7),
         word.w8[1] = (*ops->v_readb)(closure,(addr & ~7)+1);
    if ((addr & 6) != 2)
         word.w8[2] = (*ops->v_readb)(closure,(addr & ~7)+2),
         word.w8[3] = (*ops->v_readb)(closure,(addr & ~7)+3);
    if ((addr & 6) != 4)
         word.w8[4] = (*ops->v_readb)(closure,(addr & ~7)+4),
         word.w8[5] = (*ops->v_readb)(closure,(addr & ~7)+5);
    if ((addr & 6) != 6)
         word.w8[6] = (*ops->v_readb)(closure,(addr & ~7)+6),
         word.w8[7] = (*ops->v_readb)(closure,(addr & ~7)+7);
    word.w16[(addr & 6) >> 1] = value;
    (*ops->v_writeq)(closure,addr & ~7,word.w64);
    return;
   }
  }
#endif
  break;

#if __SIZEOF_POINTER__ >= 8
 case 7:
  /* MASK: 00000000000000FF FF00000000000000 */
  /* It's impossible to write this value as a single operation.
   * Therefor, the best course of action is to use the smallest
   * operand size that is implemented. */
  if (ops->v_writeb) goto do_write_as_bytes;
  if (ops->v_writew) {
   if (ops->v_readb) {
    union word32 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-1);
    word.w8[3] = (*ops->v_readb)(closure,addr+2);
    word.w16_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    return;
   }
   if (ops->v_readw) {
    union word32 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-1);
    word.w16[1] = (*ops->v_readw)(closure,addr+1);
    word.w16_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    return;
   }
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-3);
    word.w32[1] = (*ops->v_readl)(closure,addr+1);
    word.w16_unaligned_24 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[1]);
    (*ops->v_writew)(closure,addr+1,word.w16[2]);
    return;
   }
   if (ops->v_readq) {
    union word128 word;
    word.w64[0] = (*ops->v_readq)(closure,addr-7);
    word.w64[1] = (*ops->v_readq)(closure,addr+1);
    word.w16_unaligned_56 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[3]);
    (*ops->v_writew)(closure,addr+1,word.w16[4]);
    return;
   }
  }
  if (ops->v_writel) {
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-3);
    word.w32[1] = (*ops->v_readl)(closure,addr+1);
    word.w16_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
   if (ops->v_readq) {
    union word128 word;
    word.w64[0] = (*ops->v_readq)(closure,addr-7);
    word.w64[1] = (*ops->v_readq)(closure,addr+1);
    word.w16_unaligned_56 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[1]);
    (*ops->v_writel)(closure,addr+1,word.w32[2]);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-3);
    word.w16[1] = (*ops->v_readw)(closure,addr-1);
    word.w16[2] = (*ops->v_readw)(closure,addr+1);
    word.w16[3] = (*ops->v_readw)(closure,addr+3);
    word.w16_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-3);
    word.w8[1] = (*ops->v_readb)(closure,addr-2);
    word.w8[2] = (*ops->v_readb)(closure,addr-1);
    word.w8[5] = (*ops->v_readb)(closure,addr+2);
    word.w8[6] = (*ops->v_readb)(closure,addr+3);
    word.w8[7] = (*ops->v_readb)(closure,addr+4);
    word.w16_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
  }
  if (ops->v_writeq) {
   if (ops->v_readq) {
    union word128 word;
    word.w64[0] = (*ops->v_readq)(closure,addr-7);
    word.w64[1] = (*ops->v_readq)(closure,addr+1);
    word.w16_unaligned_56 = value;
    (*ops->v_writeq)(closure,addr-7,word.w64[0]);
    (*ops->v_writeq)(closure,addr+1,word.w64[1]);
    return;
   }
   if (ops->v_readl) {
    union word128 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-7);
    word.w32[1] = (*ops->v_readl)(closure,addr-3);
    word.w32[2] = (*ops->v_readl)(closure,addr+1);
    word.w32[3] = (*ops->v_readl)(closure,addr+5);
    word.w16_unaligned_56 = value;
    (*ops->v_writeq)(closure,addr-7,word.w64[0]);
    (*ops->v_writeq)(closure,addr+1,word.w64[1]);
    return;
   }
   if (ops->v_readw) {
    union word128 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-7);
    word.w16[1] = (*ops->v_readw)(closure,addr-5);
    word.w16[2] = (*ops->v_readw)(closure,addr-3);
    word.w16[3] = (*ops->v_readw)(closure,addr-1);
    word.w16[4] = (*ops->v_readw)(closure,addr+1);
    word.w16[5] = (*ops->v_readw)(closure,addr+3);
    word.w16[6] = (*ops->v_readw)(closure,addr+5);
    word.w16[7] = (*ops->v_readw)(closure,addr+7);
    word.w16_unaligned_56 = value;
    (*ops->v_writeq)(closure,addr-7,word.w64[0]);
    (*ops->v_writeq)(closure,addr+1,word.w64[1]);
    return;
   }
   if (ops->v_readb) {
    union word128 word;
    word.w8[0]  = (*ops->v_readb)(closure,addr-7);
    word.w8[1]  = (*ops->v_readb)(closure,addr-6);
    word.w8[2]  = (*ops->v_readb)(closure,addr-5);
    word.w8[3]  = (*ops->v_readb)(closure,addr-4);
    word.w8[4]  = (*ops->v_readb)(closure,addr-3);
    word.w8[5]  = (*ops->v_readb)(closure,addr-2);
    word.w8[6]  = (*ops->v_readb)(closure,addr-1);
    word.w8[9]  = (*ops->v_readb)(closure,addr+2);
    word.w8[10] = (*ops->v_readb)(closure,addr+3);
    word.w8[11] = (*ops->v_readb)(closure,addr+4);
    word.w8[12] = (*ops->v_readb)(closure,addr+5);
    word.w8[13] = (*ops->v_readb)(closure,addr+6);
    word.w8[14] = (*ops->v_readb)(closure,addr+7);
    word.w8[15] = (*ops->v_readb)(closure,addr+8);
    word.w16_unaligned_56 = value;
    (*ops->v_writeq)(closure,addr-7,word.w64[0]);
    (*ops->v_writeq)(closure,addr+1,word.w64[1]);
    return;
   }
  }
  goto not_implemented;

 case 5:
  /* MASK: 0000000000FFFF00 */
  if (ops->v_writel) {
   if (ops->v_readl) {
    union word32 word;
    word.w32 = (*ops->v_readl)(closure,addr-1);
    word.w16_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-1,word.w32);
    return;
   }
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readl)(closure,addr-5);
    word.w16_unaligned_40 = value;
    (*ops->v_writel)(closure,addr-1,word.w32[1]);
    return;
   }
  }
  if (ops->v_writeq && ops->v_readq) {
   union word64 word;
   word.w64 = (*ops->v_readq)(closure,addr-5);
   word.w16_unaligned_40 = value;
   (*ops->v_writeq)(closure,addr-5,word.w64);
   return;
  }
  if (ops->v_writel) {
   if (ops->v_readw) {
    union word32 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-1);
    word.w16[1] = (*ops->v_readw)(closure,addr+1);
    word.w16_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-1,word.w32);
    return;
   }
   if (ops->v_readb) {
    union word32 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-1);
    word.w8[3] = (*ops->v_readb)(closure,addr+2);
    word.w16_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-1,word.w32);
    return;
   }
  }
  if (ops->v_writeq) {
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-5);
    word.w32[1] = (*ops->v_readl)(closure,addr-1);
    word.w16_unaligned_40 = value;
    (*ops->v_writeq)(closure,addr-5,word.w64);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-5);
    word.w16[1] = (*ops->v_readw)(closure,addr-3);
    word.w16[2] = (*ops->v_readw)(closure,addr-1);
    word.w16[3] = (*ops->v_readw)(closure,addr+1);
    word.w16_unaligned_40 = value;
    (*ops->v_writeq)(closure,addr-5,word.w64);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    word.w8[0] = (*ops->v_readw)(closure,addr-5);
    word.w8[1] = (*ops->v_readw)(closure,addr-4);
    word.w8[2] = (*ops->v_readw)(closure,addr-3);
    word.w8[3] = (*ops->v_readw)(closure,addr-2);
    word.w8[4] = (*ops->v_readw)(closure,addr-1);
    word.w8[7] = (*ops->v_readw)(closure,addr+2);
    word.w16_unaligned_40 = value;
    (*ops->v_writeq)(closure,addr-5,word.w64);
    return;
   }
  }
  if (ops->v_writew) {
   if (ops->v_writeb) goto do_write_as_bytes;
   if (ops->v_readl) {
    union word32 word;
    word.w32 = (*ops->v_readl)(closure,addr-1);
    word.w16_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    return;
   }
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-5);
    word.w16_unaligned_40 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[2]);
    (*ops->v_writew)(closure,addr+1,word.w16[3]);
    return;
   }
   if (ops->v_readw) {
    union word32 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-1);
    word.w16[1] = (*ops->v_readw)(closure,addr+1);
    word.w16_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    return;
   }
   if (ops->v_readb) {
    union word32 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-1);
    word.w8[3] = (*ops->v_readb)(closure,addr+2);
    word.w16_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    return;
   }
  }
  break;

 case 3:
  /* MASK: 000000FFFF000000 */
  if (ops->v_writeq) {
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-3);
    word.w16_unaligned_24 = value;
    (*ops->v_writeq)(closure,addr-3,word.w64);
    return;
   }
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-3);
    word.w32[1] = (*ops->v_readl)(closure,addr+1);
    word.w16_unaligned_24 = value;
    (*ops->v_writeq)(closure,addr-3,word.w64);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    word.w16[0] = (*ops->v_readl)(closure,addr-3);
    word.w16[1] = (*ops->v_readl)(closure,addr-1);
    word.w16[2] = (*ops->v_readl)(closure,addr+1);
    word.w16[3] = (*ops->v_readl)(closure,addr+3);
    word.w16_unaligned_24 = value;
    (*ops->v_writeq)(closure,addr-3,word.w64);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    word.w8[0] = (*ops->v_readl)(closure,addr-3);
    word.w8[1] = (*ops->v_readl)(closure,addr-2);
    word.w8[2] = (*ops->v_readl)(closure,addr-1);
    word.w8[5] = (*ops->v_readl)(closure,addr+2);
    word.w8[6] = (*ops->v_readl)(closure,addr+3);
    word.w8[7] = (*ops->v_readl)(closure,addr+4);
    word.w16_unaligned_24 = value;
    (*ops->v_writeq)(closure,addr-3,word.w64);
    return;
   }
  }
  /* NOTE: Without writeq(), we have to write data in 2 blocks! */
  if (ops->v_writeb) goto do_write_as_bytes;
  if (ops->v_writew) {
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-3);
    word.w16_unaligned_24 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[1]);
    (*ops->v_writew)(closure,addr+1,word.w16[2]);
    return;
   }
   if (ops->v_readb) {
    union word32 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-1);
    word.w8[3] = (*ops->v_readb)(closure,addr+2);
    word.w16_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    return;
   }
   if (ops->v_readw) {
    union word32 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-1);
    word.w16[1] = (*ops->v_readw)(closure,addr+1);
    word.w16_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    return;
   }
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-3);
    word.w32[1] = (*ops->v_readl)(closure,addr+1);
    word.w16_unaligned_24 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[1]);
    (*ops->v_writew)(closure,addr+1,word.w16[2]);
    return;
   }
  }
  if (ops->v_writel) {
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-3);
    word.w16_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
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
    word.w16_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-3);
    word.w8[1] = (*ops->v_readb)(closure,addr-2);
    word.w8[2] = (*ops->v_readb)(closure,addr-1);
    word.w8[5] = (*ops->v_readb)(closure,addr+2);
    word.w8[6] = (*ops->v_readb)(closure,addr+3);
    word.w8[7] = (*ops->v_readb)(closure,addr+4);
    word.w16_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
  }
  goto not_implemented;

#else
 case 3:
  /* It's impossible to write this value as a single operation.
   * Therefor, the best course of action is to use the smallest
   * operand size that is implemented. */
  if (ops->v_writeb) goto do_write_as_bytes;
  /* MASK: 000000FF FF000000 */
  if (ops->v_writew) {
   if (ops->v_readb) {
    union word32 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-1);
    word.w8[3] = (*ops->v_readb)(closure,addr+2);
    word.w16_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    return;
   }
   if (ops->v_readw) {
    union word32 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-1);
    word.w16[1] = (*ops->v_readw)(closure,addr+1);
    word.w16_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    return;
   }
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-3);
    word.w32[1] = (*ops->v_readl)(closure,addr+1);
    word.w16_unaligned_40 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[1]);
    (*ops->v_writew)(closure,addr+1,word.w16[2]);
    return;
   }
  }
  if (ops->v_writel) {
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-3);
    word.w32[1] = (*ops->v_readl)(closure,addr+1);
    word.w16_unaligned_40 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
   }
   if (ops->v_readw) {
    union word64 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-3);
    word.w16[1] = (*ops->v_readw)(closure,addr-1);
    word.w16[2] = (*ops->v_readw)(closure,addr+1);
    word.w16[3] = (*ops->v_readw)(closure,addr+3);
    word.w16_unaligned_40 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
   }
   if (ops->v_readb) {
    union word64 word;
    word.w8[0] = (*ops->v_readw)(closure,addr-3);
    word.w8[1] = (*ops->v_readw)(closure,addr-2);
    word.w8[2] = (*ops->v_readw)(closure,addr-1);
    word.w8[5] = (*ops->v_readw)(closure,addr+2);
    word.w8[6] = (*ops->v_readw)(closure,addr+3);
    word.w8[7] = (*ops->v_readw)(closure,addr+4);
    word.w16_unaligned_40 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
   }
  }
  goto not_implemented;
#endif

 case 1:
  /* MASK: 00FFFF00 */
  /* MASK: 00FFFF0000000000 */
  if (ops->v_writel) {
   if (ops->v_readl) {
    union word32 word;
    word.w32 = (*ops->v_readl)(closure,addr-1);
    word.w16_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-1,word.w32);
    return;
   }
   if (ops->v_readb) {
    union word32 word;
    word.w8[0] = (*ops->v_readw)(closure,addr-1);
    word.w8[3] = (*ops->v_readw)(closure,addr+2);
    word.w16_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-1,word.w32);
    return;
   }
   if (ops->v_readw) {
    union word32 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-1);
    word.w16[1] = (*ops->v_readw)(closure,addr+1);
    word.w16_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-1,word.w32);
    return;
   }
  }
#if __SIZEOF_POINTER__ >= 8
  if (ops->v_writeq) {
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-1);
    word.w16_unaligned_8 = value;
    (*ops->v_writeq)(closure,addr-1,word.w64);
    return;
   }
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-1);
    word.w32[1] = (*ops->v_readl)(closure,addr+3);
    word.w16_unaligned_8 = value;
    (*ops->v_writeq)(closure,addr-1,word.w64);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-1);
    word.w16[1] = (*ops->v_readw)(closure,addr+1);
    word.w16[2] = (*ops->v_readw)(closure,addr+3);
    word.w16[3] = (*ops->v_readw)(closure,addr+5);
    word.w16_unaligned_8 = value;
    (*ops->v_writeq)(closure,addr-1,word.w64);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    word.w8[0] = (*ops->v_readw)(closure,addr-1);
    word.w8[3] = (*ops->v_readw)(closure,addr+2);
    word.w8[4] = (*ops->v_readw)(closure,addr+3);
    word.w8[5] = (*ops->v_readw)(closure,addr+4);
    word.w8[6] = (*ops->v_readw)(closure,addr+5);
    word.w8[7] = (*ops->v_readw)(closure,addr+6);
    word.w16_unaligned_8 = value;
    (*ops->v_writeq)(closure,addr-1,word.w64);
    return;
   }
  }
#endif
  /* Without quad or long, the write will have to be split in 2. */
  if (ops->v_writeb) goto do_write_as_bytes;
  if (ops->v_writew) {
   if (ops->v_readl) {
    union word32 word;
    word.w32 = (*ops->v_readl)(closure,addr-1);
    word.w16_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    return;
   }
#if __SIZEOF_POINTER__ >= 8
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-1);
    word.w16_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    return;
   }
#endif
   if (ops->v_readb) {
    union word32 word;
    word.w8[0] = (*ops->v_readw)(closure,addr-1);
    word.w8[3] = (*ops->v_readw)(closure,addr+2);
    word.w16_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    return;
   }
   if (ops->v_readw) {
    union word32 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-1);
    word.w16[1] = (*ops->v_readw)(closure,addr+1);
    word.w16_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    return;
   }
  }
  goto not_implemented;

 default: __builtin_unreachable();
 }

 if (ops->v_writeb) {
  union word16 word;
do_write_as_bytes:
  word.w16 = value;
  (*ops->v_writeb)(closure,addr,word.w8[0]);
  (*ops->v_writeb)(closure,addr+1,word.w8[1]);
  return;
 }
not_implemented:
 vio_not_implemented();
}






PUBLIC void KCALL
vio_writel(struct vio_ops *__restrict ops,
          void *closure, uintptr_t addr, u32 value) {
#if __SIZEOF_POINTER__ >= 8
 switch (addr & 7)
#else
 switch (addr & 3)
#endif
 {

 case 0:
#if __SIZEOF_POINTER__ >= 8
 case 4:
#endif
  if (ops->v_writel) {
   (*ops->v_writel)(closure,addr,value);
   return;
  }
#if __SIZEOF_POINTER__ >= 8
  if (ops->v_writeq) {
   /* MASK: FFFFFFFF00000000 */
   /* MASK: 00000000FFFFFFFF */
   if (ops->v_readl) {
    union word64 word;
    if (addr & 4) {
     word.w32[0] = (*ops->v_readl)(closure,addr-4);
     word.w32[1] = value;
    } else {
     word.w32[0] = value;
     word.w32[1] = (*ops->v_readl)(closure,addr+4);
    }
    (*ops->v_writeq)(closure,addr & ~7,word.w64);
    return;
   }
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr & ~7);
    word.w32[(addr & 4) >> 2] = value;
    (*ops->v_writeq)(closure,addr & ~7,word.w64);
    return;
   }
  }
#endif
  /* MASK: FFFFFFFF */
  if (ops->v_writew)
      goto write_as_2_words;
#if __SIZEOF_POINTER__ >= 8
  if (ops->v_writeq) {
   if (ops->v_readw) {
    union word64 word;
    if (addr & 4) {
     /* MASK: 00000000FFFFFFFF */
     word.w16[0] = (*ops->v_readw)(closure,addr-4);
     word.w16[1] = (*ops->v_readw)(closure,addr-2);
     word.w32[1] = value;
    } else {
     /* MASK: FFFFFFFF00000000 */
     word.w16[2] = (*ops->v_readw)(closure,addr+4);
     word.w16[3] = (*ops->v_readw)(closure,addr+6);
     word.w32[0] = value;
    }
    (*ops->v_writeq)(closure,addr & ~7,word.w64);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    if (addr & 4) {
     /* MASK: 00000000FFFFFFFF */
     word.w8[0] = (*ops->v_readb)(closure,addr-4);
     word.w8[1] = (*ops->v_readb)(closure,addr-3);
     word.w8[2] = (*ops->v_readb)(closure,addr-2);
     word.w8[3] = (*ops->v_readb)(closure,addr-1);
     word.w32[1] = value;
    } else {
     /* MASK: FFFFFFFF00000000 */
     word.w8[4] = (*ops->v_readb)(closure,addr+4);
     word.w8[5] = (*ops->v_readb)(closure,addr+5);
     word.w8[6] = (*ops->v_readb)(closure,addr+6);
     word.w8[7] = (*ops->v_readb)(closure,addr+7);
     word.w32[0] = value;
    }
    (*ops->v_writeq)(closure,addr & ~7,word.w64);
    return;
   }
  }
#endif
  break;

#if __SIZEOF_POINTER__ >= 8
 case 6:
  /* MASK: 000000000000FFFF FFFF000000000000 */
  if (ops->v_writew)
      goto write_as_2_words;
  if (ops->v_writel) {
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-2);
    word.w32[1] = (*ops->v_readl)(closure,addr+2);
    word.w32_unaligned_16 = value;
    (*ops->v_writel)(closure,addr-2,word.w32[0]);
    (*ops->v_writel)(closure,addr+2,word.w32[1]);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-2);
    word.w16[3] = (*ops->v_readw)(closure,addr+4);
    word.w32_unaligned_16 = value;
    (*ops->v_writel)(closure,addr-2,word.w32[0]);
    (*ops->v_writel)(closure,addr+2,word.w32[1]);
    return;
   }
   if (ops->v_readq) {
    union word128 word;
    word.w64[0] = (*ops->v_readq)(closure,addr-6);
    word.w64[1] = (*ops->v_readq)(closure,addr+4);
    word.w32_unaligned_80 = value;
    (*ops->v_writel)(closure,addr-2,word.w32[1]);
    (*ops->v_writel)(closure,addr+2,word.w32[2]);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-2);
    word.w8[1] = (*ops->v_readb)(closure,addr-1);
    word.w8[6] = (*ops->v_readb)(closure,addr+4);
    word.w8[7] = (*ops->v_readb)(closure,addr+5);
    word.w32_unaligned_16 = value;
    (*ops->v_writel)(closure,addr-2,word.w32[0]);
    (*ops->v_writel)(closure,addr+2,word.w32[1]);
    return;
   }
  }
  if (ops->v_writeq) {
   if (ops->v_readq) {
    union word128 word;
    word.w64[0] = (*ops->v_readq)(closure,addr-6);
    word.w64[1] = (*ops->v_readq)(closure,addr+2);
    word.w32_unaligned_80 = value;
    (*ops->v_writeq)(closure,addr-6,word.w64[0]);
    (*ops->v_writeq)(closure,addr+2,word.w64[1]);
    return;
   }
   if (ops->v_readl) {
    union word128 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-6);
    word.w32[1] = (*ops->v_readl)(closure,addr-2);
    word.w32[2] = (*ops->v_readl)(closure,addr+2);
    word.w32[3] = (*ops->v_readl)(closure,addr+6);
    word.w32_unaligned_80 = value;
    (*ops->v_writeq)(closure,addr-6,word.w64[0]);
    (*ops->v_writeq)(closure,addr+2,word.w64[1]);
    return;
   }
   if (ops->v_readw) {
    union word128 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-6);
    word.w16[1] = (*ops->v_readw)(closure,addr-4);
    word.w16[2] = (*ops->v_readw)(closure,addr-2);
    word.w16[5] = (*ops->v_readw)(closure,addr+4);
    word.w16[6] = (*ops->v_readw)(closure,addr+6);
    word.w16[7] = (*ops->v_readw)(closure,addr+8);
    word.w32_unaligned_80 = value;
    (*ops->v_writeq)(closure,addr-6,word.w64[0]);
    (*ops->v_writeq)(closure,addr+2,word.w64[1]);
    return;
   }
   if (ops->v_readb) {
    union word128 word;
    word.w8[0]  = (*ops->v_readw)(closure,addr-6);
    word.w8[1]  = (*ops->v_readw)(closure,addr-5);
    word.w8[2]  = (*ops->v_readw)(closure,addr-4);
    word.w8[3]  = (*ops->v_readw)(closure,addr-3);
    word.w8[4]  = (*ops->v_readw)(closure,addr-2);
    word.w8[5]  = (*ops->v_readw)(closure,addr-1);
    word.w8[10] = (*ops->v_readw)(closure,addr+4);
    word.w8[11] = (*ops->v_readw)(closure,addr+5);
    word.w8[12] = (*ops->v_readw)(closure,addr+6);
    word.w8[13] = (*ops->v_readw)(closure,addr+7);
    word.w8[14] = (*ops->v_readw)(closure,addr+8);
    word.w8[15] = (*ops->v_readw)(closure,addr+9);
    word.w32_unaligned_80 = value;
    (*ops->v_writeq)(closure,addr-6,word.w64[0]);
    (*ops->v_writeq)(closure,addr+2,word.w64[1]);
    return;
   }
  }
  break;
#endif

 case 2:
  /* MASK: 0000FFFFFFFF0000 */
#if __SIZEOF_POINTER__ >= 8
  if (ops->v_writeq) {
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-2);
    word.w32_unaligned_16 = value;
    (*ops->v_writeq)(closure,addr-2,word.w64);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-2);
    word.w16[3] = (*ops->v_readw)(closure,addr+4);
    word.w32_unaligned_16 = value;
    (*ops->v_writeq)(closure,addr-2,word.w64);
    return;
   }
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-2);
    word.w32[1] = (*ops->v_readl)(closure,addr+2);
    word.w32_unaligned_16 = value;
    (*ops->v_writeq)(closure,addr-2,word.w64);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-2);
    word.w8[1] = (*ops->v_readb)(closure,addr-1);
    word.w8[6] = (*ops->v_readb)(closure,addr+4);
    word.w8[7] = (*ops->v_readb)(closure,addr+5);
    word.w32_unaligned_16 = value;
    (*ops->v_writeq)(closure,addr-2,word.w64);
    return;
   }
  }
#endif
  /* MASK: FFFF FFFF */
  if (ops->v_writew) {
   union word32 word;
write_as_2_words:
   word.w32 = value;
   (*ops->v_writew)(closure,addr,word.w16[0]);
   (*ops->v_writew)(closure,addr+2,word.w16[1]);
   return;
  }
  if (ops->v_writel) {
   /* MASK: 0000FFFF FFFF0000 */
#if __SIZEOF_POINTER__ >= 8
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-2);
    word.w32_unaligned_16 = value;
    (*ops->v_writel)(closure,addr-2,word.w32[0]);
    (*ops->v_writel)(closure,addr+2,word.w32[1]);
    return;
   }
#endif
   if (ops->v_readw) {
    union word64 word;
    word.w16[0] = (*ops->v_readw)(closure,addr-2);
    word.w16[3] = (*ops->v_readw)(closure,addr+4);
    word.w32_unaligned_16 = value;
    (*ops->v_writel)(closure,addr-2,word.w32[0]);
    (*ops->v_writel)(closure,addr+2,word.w32[1]);
    return;
   }
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-2);
    word.w32[1] = (*ops->v_readl)(closure,addr+2);
    word.w32_unaligned_16 = value;
    (*ops->v_writel)(closure,addr-2,word.w32[0]);
    (*ops->v_writel)(closure,addr+2,word.w32[1]);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-2);
    word.w8[1] = (*ops->v_readb)(closure,addr-1);
    word.w8[6] = (*ops->v_readb)(closure,addr+4);
    word.w8[7] = (*ops->v_readb)(closure,addr+5);
    word.w32_unaligned_16 = value;
    (*ops->v_writel)(closure,addr-2,word.w32[0]);
    (*ops->v_writel)(closure,addr+2,word.w32[1]);
    return;
   }
  }
  break;


#if __SIZEOF_POINTER__ >= 8
 case 5:
  /* MASK: 0000000000FFFFFF FF00000000000000 */
  if (ops->v_writel) {
   if (ops->v_readl) {
    /* MASK: 00FFFFFF FF000000 */
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-1);
    word.w32[1] = (*ops->v_readl)(closure,addr+3);
    word.w32_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-1,word.w32[0]);
    (*ops->v_writel)(closure,addr+3,word.w32[1]);
    return;
   }
   if (ops->v_readq) {
    union word128 word;
    word.w64[0] = (*ops->v_readq)(closure,addr-5);
    word.w64[1] = (*ops->v_readq)(closure,addr+3);
    word.w32_unaligned_40 = value;
    (*ops->v_writel)(closure,addr-1,word.w32[1]);
    (*ops->v_writel)(closure,addr+3,word.w32[2]);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    if (ops->v_readb) {
     word.w8[0]  = (*ops->v_readb)(closure,addr-1);
     word.w8[5]  = (*ops->v_readb)(closure,addr+4);
    } else {
     word.w16[0] = (*ops->v_readw)(closure,addr-1);
     word.w16[2] = (*ops->v_readw)(closure,addr+3);
    }
    word.w16[3]  = (*ops->v_readw)(closure,addr+5);
    word.w32_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-1,word.w32[0]);
    (*ops->v_writel)(closure,addr+3,word.w32[1]);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-1);
    word.w8[5] = (*ops->v_readb)(closure,addr+4);
    word.w8[6] = (*ops->v_readb)(closure,addr+5);
    word.w8[7] = (*ops->v_readb)(closure,addr+6);
    word.w32_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-1,word.w32[0]);
    (*ops->v_writel)(closure,addr+3,word.w32[1]);
    return;
   }
  }
  /* MASK: 00FF FFFF FF00 */
  if (ops->v_writew) {
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
   if (ops->v_readq) {
    /* MASK: 0000000000FFFFFF FF00000000000000 */
    union word128 word;
    word.w64[0] = (*ops->v_readq)(closure,addr-5);
    word.w64[1] = (*ops->v_readq)(closure,addr+3);
    word.w32_unaligned_40 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[2]);
    (*ops->v_writew)(closure,addr+1,word.w16[3]);
    (*ops->v_writew)(closure,addr+3,word.w16[4]);
    return;
   }
  }
  /* MASK: 0000000000FFFFFF FF00000000000000 */
  if (ops->v_writeq) {
   if (ops->v_readl) {
    union word128 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-5);
    word.w32[1] = (*ops->v_readl)(closure,addr-1);
    word.w32[2] = (*ops->v_readl)(closure,addr+3);
    word.w32[3] = (*ops->v_readl)(closure,addr+7);
    word.w32_unaligned_8 = value;
    (*ops->v_writeq)(closure,addr-5,word.w64[0]);
    (*ops->v_writeq)(closure,addr+3,word.w64[1]);
    return;
   }
   if (ops->v_readq) {
    union word128 word;
    word.w64[0] = (*ops->v_readq)(closure,addr-5);
    word.w64[1] = (*ops->v_readq)(closure,addr+3);
    word.w32_unaligned_40 = value;
    (*ops->v_writeq)(closure,addr-5,word.w64[0]);
    (*ops->v_writeq)(closure,addr+3,word.w64[1]);
    return;
   }
   if (ops->v_readw) {
    union word128 word;
    /* MASK: 0000 0000 00FF FFFF FF00 0000 0000 0000 */
    if (ops->v_readb) {
     word.w8[4] = (*ops->v_readb)(closure,addr-1);
     word.w8[9] = (*ops->v_readb)(closure,addr+4);
    } else {
     word.w16[2] = (*ops->v_readw)(closure,addr-1);
     word.w16[4] = (*ops->v_readw)(closure,addr+3);
    }
    word.w16[0] = (*ops->v_readw)(closure,addr-5);
    word.w16[1] = (*ops->v_readw)(closure,addr-3);
    word.w16[5] = (*ops->v_readw)(closure,addr+5);
    word.w16[6] = (*ops->v_readw)(closure,addr+7);
    word.w16[7] = (*ops->v_readw)(closure,addr+9);
    word.w32_unaligned_40 = value;
    (*ops->v_writeq)(closure,addr-5,word.w64[0]);
    (*ops->v_writeq)(closure,addr+3,word.w64[1]);
    return;
   }
   if (ops->v_readb) {
    union word128 word;
    /* MASK: 00 00 00 00 00 FF FF FF FF 00 00 00 00 00 00 00 */
    word.w8[0]  = (*ops->v_readb)(closure,addr-5);
    word.w8[1]  = (*ops->v_readb)(closure,addr-4);
    word.w8[2]  = (*ops->v_readb)(closure,addr-3);
    word.w8[3]  = (*ops->v_readb)(closure,addr-2);
    word.w8[4]  = (*ops->v_readb)(closure,addr-1);
    word.w8[9]  = (*ops->v_readb)(closure,addr+4);
    word.w8[10] = (*ops->v_readb)(closure,addr+5);
    word.w8[11] = (*ops->v_readb)(closure,addr+6);
    word.w8[12] = (*ops->v_readb)(closure,addr+7);
    word.w8[13] = (*ops->v_readb)(closure,addr+8);
    word.w8[14] = (*ops->v_readb)(closure,addr+9);
    word.w8[15] = (*ops->v_readb)(closure,addr+10);
    word.w32_unaligned_40 = value;
    (*ops->v_writeq)(closure,addr-5,word.w64[0]);
    (*ops->v_writeq)(closure,addr+3,word.w64[1]);
    return;
   }
  }
  break;
#endif

 case 1:
#if __SIZEOF_POINTER__ >= 8
  /* MASK: 00FFFFFFFF000000 */
  if (ops->v_writeq) {
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-1);
    word.w32_unaligned_8 = value;
    (*ops->v_writeq)(closure,addr-1,word.w64);
    return;
   }
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-1);
    word.w32[1] = (*ops->v_readl)(closure,addr+3);
    word.w32_unaligned_8 = value;
    (*ops->v_writeq)(closure,addr-1,word.w64);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    if (ops->v_readb) {
     word.w8[0]  = (*ops->v_readb)(closure,addr-1);
     word.w8[5]  = (*ops->v_readb)(closure,addr+4);
    } else {
     word.w16[0] = (*ops->v_readw)(closure,addr-1);
     word.w16[2] = (*ops->v_readw)(closure,addr+3);
    }
    word.w16[3]  = (*ops->v_readw)(closure,addr+5);
    word.w32_unaligned_8 = value;
    (*ops->v_writeq)(closure,addr-1,word.w64);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-1);
    word.w8[5] = (*ops->v_readb)(closure,addr+4);
    word.w8[6] = (*ops->v_readb)(closure,addr+5);
    word.w8[7] = (*ops->v_readb)(closure,addr+6);
    word.w32_unaligned_8 = value;
    (*ops->v_writeq)(closure,addr-1,word.w64);
    return;
   }
  }
#endif
  /* MASK: 00FFFFFF FF000000 */
  if (ops->v_writel) {
#if __SIZEOF_POINTER__ >= 8
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-1);
    word.w32_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-1,word.w32[0]);
    (*ops->v_writel)(closure,addr+3,word.w32[1]);
    return;
   }
#endif
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-1);
    word.w32[1] = (*ops->v_readl)(closure,addr+3);
    word.w32_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-1,word.w32[0]);
    (*ops->v_writel)(closure,addr+3,word.w32[1]);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    if (ops->v_readb) {
     word.w8[0]  = (*ops->v_readb)(closure,addr-1);
     word.w8[5]  = (*ops->v_readb)(closure,addr+4);
    } else {
     word.w16[0] = (*ops->v_readw)(closure,addr-1);
     word.w16[2] = (*ops->v_readw)(closure,addr+3);
    }
    word.w16[3]  = (*ops->v_readw)(closure,addr+5);
    word.w32_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-1,word.w32[0]);
    (*ops->v_writel)(closure,addr+3,word.w32[1]);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    word.w8[0] = (*ops->v_readb)(closure,addr-1);
    word.w8[5] = (*ops->v_readb)(closure,addr+4);
    word.w8[6] = (*ops->v_readb)(closure,addr+5);
    word.w8[7] = (*ops->v_readb)(closure,addr+6);
    word.w32_unaligned_8 = value;
    (*ops->v_writel)(closure,addr-1,word.w32[0]);
    (*ops->v_writel)(closure,addr+3,word.w32[1]);
    return;
   }
  }
  /* MASK: 00FF FFFF FF00 */
  if (ops->v_writew) {
#if __SIZEOF_POINTER__ >= 8
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-1);
    word.w32_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    (*ops->v_writew)(closure,addr+3,word.w16[2]);
    return;
   }
#endif
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
  }
  break;

#if __SIZEOF_POINTER__ >= 8
 case 7:
  /* MASK: 00000000000000FF FFFFFF0000000000 */
  if (ops->v_writel) {
   if (ops->v_readl) {
    union word64 word;
    /* MASK: 000000FF FFFFFF00 */
    word.w32[0] = (*ops->v_readl)(closure,addr-3);
    word.w32[1] = (*ops->v_readl)(closure,addr+1);
    word.w32_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
   if (ops->v_readq) {
    union word128 word;
    /* MASK: 00000000 000000FF FFFFFF00 00000000 */
    word.w64[0] = (*ops->v_readq)(closure,addr-7);
    word.w64[1] = (*ops->v_readq)(closure,addr+5);
    word.w32_unaligned_56 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[1]);
    (*ops->v_writel)(closure,addr+1,word.w32[2]);
    (*ops->v_writel)(closure,addr+5,word.w32[3]);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    /* MASK: 0000 00FF FFFF FF00 */
    word.w16[0] = (*ops->v_readw)(closure,addr-3);
    if (ops->v_readb) {
     word.w8[2] = (*ops->v_readb)(closure,addr-1);
     word.w8[7] = (*ops->v_readb)(closure,addr+4);
    } else {
     word.w16[1] = (*ops->v_readw)(closure,addr-1);
     word.w16[3] = (*ops->v_readw)(closure,addr+3);
    }
    word.w32_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    /* MASK: 00 00 00 FF FF FF FF 00 */
    word.w8[0] = (*ops->v_readb)(closure,addr-3);
    word.w8[1] = (*ops->v_readb)(closure,addr-2);
    word.w8[2] = (*ops->v_readb)(closure,addr-1);
    word.w8[7] = (*ops->v_readb)(closure,addr+4);
    word.w32_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
  }
  if (ops->v_writeq) {
   /* MASK: 00000000000000FF FFFFFF0000000000 */
   if (ops->v_readq) {
    union word128 word;
    /* MASK: 00000000000000FF FFFFFF0000000000 */
    word.w64[0] = (*ops->v_readq)(closure,addr-7);
    word.w64[1] = (*ops->v_readq)(closure,addr+1);
    word.w32_unaligned_56 = value;
    (*ops->v_writeq)(closure,addr-7,word.w64[0]);
    (*ops->v_writeq)(closure,addr+1,word.w64[1]);
    return;
   }
   if (ops->v_readl) {
    union word128 word;
    /* MASK: 00000000 000000FF FFFFFF00 00000000 */
    word.w32[0] = (*ops->v_readl)(closure,addr-7);
    word.w32[1] = (*ops->v_readl)(closure,addr-3);
    word.w32[2] = (*ops->v_readl)(closure,addr+1);
    word.w32[3] = (*ops->v_readl)(closure,addr+5);
    word.w32_unaligned_56 = value;
    (*ops->v_writeq)(closure,addr-7,word.w64[0]);
    (*ops->v_writeq)(closure,addr+1,word.w64[1]);
    return;
   }
   if (ops->v_readw) {
    union word128 word;
    /* MASK: 0000 0000 0000 00FF FFFF FF00 0000 0000 */
    word.w16[0] = (*ops->v_readw)(closure,addr-7);
    word.w16[1] = (*ops->v_readw)(closure,addr-5);
    word.w16[2] = (*ops->v_readw)(closure,addr-3);
    word.w16[3] = (*ops->v_readw)(closure,addr-1);
    word.w16[5] = (*ops->v_readw)(closure,addr+3);
    word.w16[6] = (*ops->v_readw)(closure,addr+5);
    word.w16[7] = (*ops->v_readw)(closure,addr+7);
    word.w32_unaligned_56 = value;
    (*ops->v_writeq)(closure,addr-7,word.w64[0]);
    (*ops->v_writeq)(closure,addr+1,word.w64[1]);
    return;
   }
   if (ops->v_readb) {
    union word128 word;
    /* MASK: 00 00 00 00 00 00 00 FF FF FF FF 00 00 00 00 00 */
    word.w8[0]  = (*ops->v_readb)(closure,addr-7);
    word.w8[1]  = (*ops->v_readb)(closure,addr-6);
    word.w8[2]  = (*ops->v_readb)(closure,addr-5);
    word.w8[3]  = (*ops->v_readb)(closure,addr-4);
    word.w8[4]  = (*ops->v_readb)(closure,addr-3);
    word.w8[5]  = (*ops->v_readb)(closure,addr-2);
    word.w8[6]  = (*ops->v_readb)(closure,addr-1);
    word.w8[11] = (*ops->v_readb)(closure,addr+4);
    word.w8[12] = (*ops->v_readb)(closure,addr+5);
    word.w8[13] = (*ops->v_readb)(closure,addr+6);
    word.w8[14] = (*ops->v_readb)(closure,addr+7);
    word.w8[15] = (*ops->v_readb)(closure,addr+8);
    word.w32_unaligned_56 = value;
    (*ops->v_writeq)(closure,addr-7,word.w64[0]);
    (*ops->v_writeq)(closure,addr+1,word.w64[1]);
    return;
   }
  }
  if (ops->v_writew) {
   /* MASK: 0000 0000 0000 00FF FFFF FF00 0000 0000 */
   if (ops->v_readq) {
    union word128 word;
    /* MASK: 00000000000000FF FFFFFF0000000000 */
    word.w64[0] = (*ops->v_readq)(closure,addr-7);
    word.w64[1] = (*ops->v_readq)(closure,addr+1);
    word.w32_unaligned_56 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[3]);
    (*ops->v_writew)(closure,addr+1,word.w16[4]);
    (*ops->v_writew)(closure,addr+3,word.w16[5]);
    return;
   }
   if (ops->v_readl) {
    union word128 word;
    /* MASK: 00000000 000000FF FFFFFF00 00000000 */
    word.w32[1] = (*ops->v_readl)(closure,addr-3);
    word.w32[2] = (*ops->v_readl)(closure,addr+1);
    word.w32[3] = (*ops->v_readl)(closure,addr+5);
    word.w32_unaligned_56 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[3]);
    (*ops->v_writew)(closure,addr+1,word.w16[4]);
    (*ops->v_writew)(closure,addr+3,word.w16[5]);
    return;
   }
   if (ops->v_readw) {
    union word128 word;
    /* MASK: 0000 0000 0000 00FF FFFF FF00 0000 0000 */
    word.w16[3] = (*ops->v_readw)(closure,addr-1);
    word.w16[5] = (*ops->v_readw)(closure,addr+3);
    word.w32_unaligned_56 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[3]);
    (*ops->v_writew)(closure,addr+1,word.w16[4]);
    (*ops->v_writew)(closure,addr+3,word.w16[5]);
    return;
   }
   if (ops->v_readb) {
    union word128 word;
    /* MASK: 00 00 00 00 00 00 00 FF FF FF FF 00 00 00 00 00 */
    word.w8[6]  = (*ops->v_readb)(closure,addr-1);
    word.w8[11] = (*ops->v_readb)(closure,addr+4);
    word.w32_unaligned_56 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[3]);
    (*ops->v_writew)(closure,addr+1,word.w16[4]);
    (*ops->v_writew)(closure,addr+3,word.w16[5]);
    return;
   }
  }
  break;
#endif


 case 3:
#if __SIZEOF_POINTER__ >= 8
  if (ops->v_writeq) {
   if (ops->v_readq) {
    /* MASK: 000000FFFFFFFF00 */
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-3);
    word.w32_unaligned_24 = value;
    (*ops->v_writeq)(closure,addr-3,word.w64);
    return;
   }
   if (ops->v_readl) {
    /* MASK: 000000FF FFFFFF00 */
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-3);
    word.w32[1] = (*ops->v_readl)(closure,addr+1);
    word.w32_unaligned_24 = value;
    (*ops->v_writeq)(closure,addr-3,word.w64);
    return;
   }
   if (ops->v_readw) {
    /* MASK: 0000 00FF FFFF FF00 */
    union word64 word;
    word.w16[0] = (*ops->v_readl)(closure,addr-3);
    if (ops->v_readb) {
     word.w8[2] = (*ops->v_readl)(closure,addr-1);
     word.w8[7] = (*ops->v_readl)(closure,addr+4);
    } else {
     word.w16[1] = (*ops->v_readl)(closure,addr-1);
     word.w16[3] = (*ops->v_readl)(closure,addr+3);
    }
    word.w32_unaligned_24 = value;
    (*ops->v_writeq)(closure,addr-3,word.w64);
    return;
   }
   if (ops->v_readb) {
    /* MASK: 0000 00FF FFFF FF00 */
    union word64 word;
    word.w8[0] = (*ops->v_readl)(closure,addr-3);
    word.w8[1] = (*ops->v_readl)(closure,addr-2);
    word.w8[2] = (*ops->v_readl)(closure,addr-1);
    word.w8[7] = (*ops->v_readl)(closure,addr+4);
    word.w32_unaligned_24 = value;
    (*ops->v_writeq)(closure,addr-3,word.w64);
    return;
   }
  }
#endif
  /* MASK: 000000FF FFFFFF00 */
  if (ops->v_writel) {
#if __SIZEOF_POINTER__ >= 8
   if (ops->v_readq) {
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-3);
    word.w32_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
#endif
   if (ops->v_readl) {
    union word64 word;
    word.w32[0] = (*ops->v_readl)(closure,addr-3);
    if (ops->v_readb) {
     word.w8[7] = (*ops->v_readb)(closure,addr+4);
    } else {
     word.w32[1] = (*ops->v_readl)(closure,addr+1);
    }
    word.w32_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    /* MASK: 0000 00FF FFFF FF00 */
    word.w16[0] = (*ops->v_readw)(closure,addr-3);
    if (ops->v_readb) {
     word.w8[2] = (*ops->v_readb)(closure,addr-1);
     word.w8[7] = (*ops->v_readb)(closure,addr+4);
    } else {
     word.w16[1] = (*ops->v_readw)(closure,addr-1);
     word.w16[3] = (*ops->v_readw)(closure,addr+3);
    }
    word.w32_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
   if (ops->v_readb) {
    union word64 word;
    /* MASK: 00 00 00 FF FF FF FF 00 */
    word.w8[0] = (*ops->v_readb)(closure,addr-3);
    word.w8[1] = (*ops->v_readb)(closure,addr-2);
    word.w8[2] = (*ops->v_readb)(closure,addr-1);
    word.w8[7] = (*ops->v_readb)(closure,addr+4);
    word.w32_unaligned_24 = value;
    (*ops->v_writel)(closure,addr-3,word.w32[0]);
    (*ops->v_writel)(closure,addr+1,word.w32[1]);
    return;
   }
  }
  /* MASK: 00FF FFFF FF00 */
  if (ops->v_writew) {
#if __SIZEOF_POINTER__ >= 8
   if (ops->v_readq) {
    /* MASK: 0000 00FF FFFF FF00 */
    union word64 word;
    word.w64 = (*ops->v_readq)(closure,addr-3);
    word.w32_unaligned_24 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[1]);
    (*ops->v_writew)(closure,addr+1,word.w16[2]);
    (*ops->v_writew)(closure,addr+3,word.w16[3]);
    return;
   }
#endif
   if (ops->v_readb) {
    union word64 word;
    /* MASK: 00 FF FF FF FF 00 */
    word.w8[0] = (*ops->v_readb)(closure,addr-1);
    word.w8[5] = (*ops->v_readb)(closure,addr+4);
    word.w32_unaligned_8 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[0]);
    (*ops->v_writew)(closure,addr+1,word.w16[1]);
    (*ops->v_writew)(closure,addr+3,word.w16[2]);
    return;
   }
   if (ops->v_readw) {
    union word64 word;
    /* MASK: 0000 00FF FFFF FF00 */
    word.w16[1] = (*ops->v_readw)(closure,addr-1);
    word.w16[3] = (*ops->v_readw)(closure,addr+3);
    word.w32_unaligned_24 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[1]);
    (*ops->v_writew)(closure,addr+1,word.w16[2]);
    (*ops->v_writew)(closure,addr+3,word.w16[3]);
    return;
   }
   if (ops->v_readl) {
    union word64 word;
    /* MASK: 0000 00FF FFFF FF00 */
    word.w32[0] = (*ops->v_readl)(closure,addr-3);
    word.w32[1] = (*ops->v_readl)(closure,addr+1);
    word.w32_unaligned_24 = value;
    (*ops->v_writew)(closure,addr-1,word.w16[1]);
    (*ops->v_writew)(closure,addr+1,word.w16[2]);
    (*ops->v_writew)(closure,addr+3,word.w16[3]);
    return;
   }
  }
  break;

 default: __builtin_unreachable();
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

#if __SIZEOF_POINTER__ >= 8
PUBLIC void KCALL
vio_writeq(struct vio_ops *__restrict ops,
           void *closure, uintptr_t addr, u64 value) {
 switch (value & ~7) {

 case 0:
  if (ops->v_writeq) {
   (*ops->v_writeq)(closure,addr,value);
   return;
  }
  if (ops->v_writel) {
   union word64 word;
   word.w64 = value;
   (*ops->v_writel)(closure,addr,word.w32[0]);
   (*ops->v_writel)(closure,addr+4,word.w32[1]);
   return;
  }
  if (ops->v_writew) {
   union word64 word;
   word.w64 = value;
   (*ops->v_writew)(closure,addr,word.w16[0]);
   (*ops->v_writew)(closure,addr+2,word.w16[1]);
   (*ops->v_writew)(closure,addr+4,word.w16[2]);
   (*ops->v_writew)(closure,addr+6,word.w16[3]);
   return;
  }
  break;

 case 4:
  if (ops->v_writel) {
   union word64 word;
   word.w64 = value;
   (*ops->v_writel)(closure,addr,word.w32[0]);
   (*ops->v_writel)(closure,addr+4,word.w32[1]);
   return;
  }
  if (ops->v_writeq) {
   /* MASK: 00000000FFFFFFFF FFFFFFFF00000000 */
   /* TODO */
  }
  if (ops->v_writew) {
   union word64 word;
do_write_as_4_words:
   word.w64 = value;
   (*ops->v_writew)(closure,addr,word.w16[0]);
   (*ops->v_writew)(closure,addr+2,word.w16[1]);
   (*ops->v_writew)(closure,addr+4,word.w16[2]);
   (*ops->v_writew)(closure,addr+6,word.w16[3]);
   return;
  }
  break;

 case 2:
  /* MASK: 0000FFFFFFFFFFFF FFFF000000000000 */
  if (ops->v_writew) goto do_write_as_4_words;
  /* TODO */
  break;

 case 6:
  if (ops->v_writew) goto do_write_as_4_words;
  /* TODO */
  break;

 case 1:
  /* TODO */
  break;

 case 3:
  /* TODO */
  break;

 case 5:
  /* TODO */
  break;

 case 7:
  /* TODO */
  break;

 default: __builtin_unreachable();
 }
 if (ops->v_writeb) {
  union word64 word;
  word.w64 = value;
  (*ops->v_writeb)(closure,addr+0,word.w8[0]);
  (*ops->v_writeb)(closure,addr+1,word.w8[1]);
  (*ops->v_writeb)(closure,addr+2,word.w8[2]);
  (*ops->v_writeb)(closure,addr+3,word.w8[3]);
  (*ops->v_writeb)(closure,addr+4,word.w8[4]);
  (*ops->v_writeb)(closure,addr+5,word.w8[5]);
  (*ops->v_writeb)(closure,addr+6,word.w8[6]);
  (*ops->v_writeb)(closure,addr+7,word.w8[7]);
  return;
 }
 vio_not_implemented();
}
#endif



PUBLIC u8 KCALL
vio_atomic_cmpxchb(struct vio_ops *__restrict ops, void *closure,
                   uintptr_t addr, u8 oldval, u8 newval) {
 if (ops->v_atomic_cmpxchb)
     return (*ops->v_atomic_cmpxchb)(closure,addr,oldval,newval);
 if (ops->v_atomic_cmpxchw) {
  union word16 word,new_word,old_word;
  if (addr & 1) {
   word.w8[1] = oldval,new_word.w8[1] = newval;
   do {
    word.w8[0]     = vio_readb(ops,closure,addr-1);
    new_word.w8[0] = word.w8[0];
    old_word.w16 = (*ops->v_atomic_cmpxchw)(closure,addr-1,word.w16,new_word.w16);
   } while (old_word.w8[0] != word.w8[0]);
   return old_word.w8[1];
  } else {
   word.w8[0] = oldval,new_word.w8[0] = newval;
   do {
    word.w8[1]     = vio_readb(ops,closure,addr+1);
    new_word.w8[1] = word.w8[1];
    old_word.w16   = (*ops->v_atomic_cmpxchw)(closure,addr,word.w16,new_word.w16);
   } while (old_word.w8[1] != word.w8[1]);
   return old_word.w8[0];
  }
 }
 if (ops->v_atomic_cmpxchl) {
  union word32 word,new_word,old_word;
  unsigned int byte_addr = addr & 3;
  u32 byte_mask = ~((u32)0xff << (byte_addr*8));
  addr &= ~3;
  do {
   word.w32               = vio_readl(ops,closure,addr);
   new_word.w32           = word.w32;
   word.w8[byte_addr]     = oldval;
   new_word.w8[byte_addr] = newval;
   old_word.w32           = (*ops->v_atomic_cmpxchl)(closure,addr,word.w32,new_word.w32);
  } while ((old_word.w32&byte_mask) != (word.w32&byte_mask));
  return old_word.w8[byte_addr];
 }
#if __SIZEOF_POINTER__ >= 8
 if (ops->v_atomic_cmpxchq) {
  union word64 word,new_word,old_word;
  unsigned int byte_addr = addr & 7;
  u64 byte_mask = ~((u64)0xff << (byte_addr*8));
  addr &= ~7;
  do {
   word.w64               = vio_readq(ops,closure,addr);
   new_word.w64           = word.w64;
   word.w8[byte_addr]     = oldval;
   new_word.w8[byte_addr] = newval;
   old_word.w64           = (*ops->v_atomic_cmpxchq)(closure,addr,word.w64,new_word.w64);
  } while ((old_word.w64&byte_mask) != (word.w64&byte_mask));
  return old_word.w8[byte_addr];
 }
#endif
 vio_not_implemented();
}
PUBLIC u16 KCALL
vio_atomic_cmpxchw(struct vio_ops *__restrict ops, void *closure,
                   uintptr_t addr, u16 oldval, u16 newval) {
 if (ops->v_atomic_cmpxchw && !(addr & 1))
     return (*ops->v_atomic_cmpxchw)(closure,addr,oldval,newval);
#if __SIZEOF_POINTER__ >= 8
 if (ops->v_atomic_cmpxchl && (addr & 3) <= 2)
#else
 if ((addr & 3) <= 2)
#endif
 {
  union word32 word,new_word,old_word;
  u32 byte_mask;
#if __SIZEOF_POINTER__ < 8
  if (!ops->v_atomic_cmpxchl)
       vio_not_implemented();
#endif
  switch (addr & 3) {
  case 0: byte_mask = 0xffff0000; break;
  case 1: byte_mask = 0x00ffff00; break;
  case 2: byte_mask = 0x0000ffff; break;
  default: __builtin_unreachable();
  }
  do {
   word.w32 = vio_readl(ops,closure,addr & ~3);
   new_word.w32 = word.w32;
   switch (addr & 3) {
   case 0: word.w16[0] = oldval; new_word.w16[0] = newval; break;
   case 1: word.w16_unaligned_8 = oldval; new_word.w16_unaligned_8 = newval; break;
   case 2: word.w16[1] = oldval; new_word.w16[1] = newval; break;
   default: __builtin_unreachable();
   }
   old_word.w32 = (*ops->v_atomic_cmpxchl)(closure,addr,word.w32,new_word.w32);
  } while ((old_word.w32&byte_mask) != (word.w32&byte_mask));
  switch (addr & 3) {
  case 0: return word.w16[0];
  case 1: return word.w16_unaligned_8;
  case 2: return word.w16[1];
  default: __builtin_unreachable();
  }
 }
#if __SIZEOF_POINTER__ >= 8
 if ((addr & 7) <= 6) {
  if (!ops->v_atomic_cmpxchq)
       vio_not_implemented();
  union word64 word,new_word,old_word;
  u64 byte_mask;
  if (!ops->v_atomic_cmpxchq)
       vio_not_implemented();
  switch (addr & 7) {
  case 0: byte_mask = __UINT64_C(0xffff000000000000); break;
  case 1: byte_mask = __UINT64_C(0x00ffff0000000000); break;
  case 2: byte_mask = __UINT64_C(0x0000ffff00000000); break;
  case 3: byte_mask = __UINT64_C(0x000000ffff000000); break;
  case 4: byte_mask = __UINT64_C(0x00000000ffff0000); break;
  case 5: byte_mask = __UINT64_C(0x0000000000ffff00); break;
  case 6: byte_mask = __UINT64_C(0x000000000000ffff); break;
  default: __builtin_unreachable();
  }
  do {
   word.w64 = vio_readq(ops,closure,addr & ~7);
   new_word.w64 = word.w64;
   switch (addr & 7) {
   case 0: word.w16[0] = oldval; new_word.w32[0] = newval; break;
   case 1: word.w16_unaligned_8 = oldval; new_word.w32_unaligned_8 = newval; break;
   case 2: word.w16[1] = oldval; new_word.w32[1] = newval; break;
   case 3: word.w16_unaligned_24 = oldval; new_word.w32_unaligned_24 = newval; break;
   case 4: word.w16[2] = oldval; new_word.w32[2] = newval; break;
   case 5: word.w16_unaligned_40 = oldval; new_word.w16_unaligned_40 = newval; break;
   case 6: word.w16[3] = oldval; new_word.w32[3] = newval; break;
   default: __builtin_unreachable();
   }
   old_word.w64 = (*ops->v_atomic_cmpxchl)(closure,addr,word.w64,new_word.w64);
  } while ((old_word.w64 & byte_mask) != (word.w64 & byte_mask));
  switch (addr & 7) {
  case 0: return word.w16[0];
  case 1: return word.w16_unaligned_8;
  case 2: return word.w16[1];
  case 3: return word.w16_unaligned_24;
  case 4: return word.w16[2];
  case 5: return word.w16_unaligned_40;
  case 6: return word.w16[3];
  default: __builtin_unreachable();
  }
 }
#endif
 vio_not_aligned();
}

PUBLIC u32 KCALL
vio_atomic_cmpxchl(struct vio_ops *__restrict ops, void *closure,
                   uintptr_t addr, u32 oldval, u32 newval) {
#if __SIZEOF_POINTER__ >= 8
 if (ops->v_atomic_cmpxchl && !(addr & 3))
     return (*ops->v_atomic_cmpxchl)(closure,addr,oldval,newval);
 if ((addr & 7) <= 4) {
  union word64 word,new_word,old_word;
  u64 byte_mask;
  if (!ops->v_atomic_cmpxchq)
       vio_not_implemented();
  switch (addr & 7) {
  case 0: byte_mask = __UINT64_C(0xffffffff00000000); break;
  case 1: byte_mask = __UINT64_C(0x00ffffffff000000); break;
  case 2: byte_mask = __UINT64_C(0x0000ffffffff0000); break;
  case 3: byte_mask = __UINT64_C(0x000000ffffffff00); break;
  case 4: byte_mask = __UINT64_C(0x00000000ffffffff); break;
  default: __builtin_unreachable();
  }
  do {
   word.w64 = vio_readq(ops,closure,addr & ~7);
   new_word.w64 = word.w64;
   switch (addr & 7) {
   case 0: word.w32[0] = oldval; new_word.w32[0] = newval; break;
   case 1: word.w32_unaligned_8 = oldval; new_word.w32_unaligned_8 = newval; break;
   case 2: word.w32_unaligned_16 = oldval; new_word.w32_unaligned_16 = newval; break;
   case 3: word.w32_unaligned_24 = oldval; new_word.w32_unaligned_24 = newval; break;
   case 4: word.w32[1] = oldval; new_word.w32[1] = newval; break;
   default: __builtin_unreachable();
   }
   old_word.w64 = (*ops->v_atomic_cmpxchl)(closure,addr,word.w64,new_word.w64);
  } while ((old_word.w64 & byte_mask) != (word.w64 & byte_mask));
  switch (addr & 7) {
  case 0: return word.w32[0];
  case 1: return word.w32_unaligned_8;
  case 2: return word.w32_unaligned_16;
  case 3: return word.w32_unaligned_24;
  case 4: return word.w32[1];
  default: __builtin_unreachable();
  }
 }
#else
 if (!(addr & 3)) {
  if (!ops->v_atomic_cmpxchl)
       vio_not_implemented();
  return (*ops->v_atomic_cmpxchl)(closure,addr,oldval,newval);
 }
#endif
 vio_not_aligned();
}

#if __SIZEOF_POINTER__ >= 8
PUBLIC u64 KCALL
vio_atomic_cmpxchq(struct vio_ops *__restrict ops, void *closure,
                   uintptr_t addr, u64 oldval, u64 newval) {
 if (addr & 7) vio_not_aligned();
 if (!ops->v_atomic_cmpxchq) vio_not_implemented();
 return (*ops->v_atomic_cmpxchq)(closure,addr,oldval,newval);
}
#endif



#endif

DECL_END
#endif /* !CONFIG_NO_VIO */

#endif /* !GUARD_KERNEL_SRC_VM_VIO_C */
