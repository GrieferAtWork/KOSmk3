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
#ifndef GUARD_KERNEL_MODULES_VGA_VGA_H
#define GUARD_KERNEL_MODULES_VGA_VGA_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/types.h>
#include <fs/device.h>
#include <sys/io.h>

#if defined(__i386__) || defined(__x86_64__)
#include <kos/i386-kos/vga.h>
#else
#error "No VGA definitions for this platform"
#endif

DECL_BEGIN

/* macro for composing an 8-bit VGA register
 * index and value into a single 16-bit quantity */
#define VGA_OUT16VAL(v,r)    (((v) << 8)|(r))

/* decide whether we should enable the faster 16-bit VGA register writes */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define VGA_OUTW_WRITE 1
#endif


/* generic VGA port read/write */
__FORCELOCAL u8   __KCALL vga_io_r(u16 port) { return inb_p(port); }
__FORCELOCAL void __KCALL vga_io_w(u16 port, u8 val) { outb_p(port,val); }
__FORCELOCAL void __KCALL vga_io_w_fast(u16 port, u8 reg, u8 val) { outw(port,VGA_OUT16VAL(val,reg)); }
__FORCELOCAL u8   __KCALL vga_mm_r(__MMIO void *regbase, u16 port) { return readb((__MMIO byte_t *)regbase+port); }
__FORCELOCAL void __KCALL vga_mm_w(__MMIO void *regbase, u16 port, u8 val) { writeb((__MMIO byte_t *)regbase+port,val); }
__FORCELOCAL void __KCALL vga_mm_w_fast(__MMIO void *regbase, u16 port, u8 reg, u8 val) { writew((__MMIO byte_t *)regbase+port,VGA_OUT16VAL(val,reg)); }
__FORCELOCAL u8   __KCALL vga_r(__MMIO void *regbase, u16 port) { return regbase ? vga_mm_r(regbase,port) : vga_io_r(port); }
__FORCELOCAL void __KCALL vga_w(__MMIO void *regbase, u16 port, u8 val) { regbase ? vga_mm_w(regbase,port,val) : vga_io_w(port,val); }
__FORCELOCAL void __KCALL vga_w_fast(__MMIO void *regbase, u16 port, u8 reg, u8 val) { regbase ? vga_mm_w_fast(regbase,port,reg,val) : vga_io_w_fast(port,reg,val); }

#ifdef VGA_OUTW_WRITE
#if 1
#define __VGA_OUTW_SELECTOR(func,port_i,port_d,reg,val)           func##_fast(port_i,reg,val)
#define __VGA_OUTW_SELECTOR2(func,regbase,port_i,port_d,reg,val)  func##_fast(regbase,port_i,reg,val)
#else
#define __VGA_OUTW_SELECTOR(func,port_i,port_d,reg,val)          ((port_i+1 == port_d) ? func##_fast(port_i,reg,val)         : (func(port_i,reg),func(port_d,val)))
#define __VGA_OUTW_SELECTOR2(func,regbase,port_i,port_d,reg,val) ((port_i+1 == port_d) ? func##_fast(regbase,port_i,reg,val) : (func(regbase,port_i,reg),func(regbase,port_d,val)))
#endif
#else
#define __VGA_OUTW_SELECTOR(func,port_i,port_d,reg,val)          (func(port_i,reg),func(port_d,val))
#define __VGA_OUTW_SELECTOR2(func,regbase,port_i,port_d,reg,val) (func(regbase,port_i,reg),func(regbase,port_d,val))
#endif

/* VGA CRTC register read/write */
LOCAL u8   KCALL vga_rcrt(MMIO void *regbase, u8 reg) { vga_w(regbase,VGA_CRT_IC,reg); return vga_r(regbase,VGA_CRT_DC); }
LOCAL void KCALL vga_wcrt(MMIO void *regbase, u8 reg, u8 val) { __VGA_OUTW_SELECTOR2(vga_w,regbase,VGA_CRT_IC,VGA_CRT_DC,reg,val); }
LOCAL u8   KCALL vga_io_rcrt(u8 reg) { vga_io_w(VGA_CRT_IC,reg); return vga_io_r(VGA_CRT_DC); }
LOCAL void KCALL vga_io_wcrt(u8 reg, u8 val) { __VGA_OUTW_SELECTOR(vga_io_w,VGA_CRT_IC,VGA_CRT_DC,reg,val); }
LOCAL u8   KCALL vga_mm_rcrt(MMIO void *regbase, u8 reg) { vga_mm_w(regbase,VGA_CRT_IC,reg); return vga_mm_r(regbase,VGA_CRT_DC); }
LOCAL void KCALL vga_mm_wcrt(MMIO void *regbase, u8 reg, u8 val) { __VGA_OUTW_SELECTOR2(vga_mm_w,regbase,VGA_CRT_IC,VGA_CRT_DC,reg,val); }

/* VGA sequencer register read/write */
LOCAL u8   KCALL vga_rseq(MMIO void *regbase, u8 reg) { vga_w(regbase,VGA_SEQ_I,reg); return vga_r(regbase,VGA_SEQ_D); }
LOCAL void KCALL vga_wseq(MMIO void *regbase, u8 reg, u8 val) { __VGA_OUTW_SELECTOR2(vga_w,regbase,VGA_SEQ_I,VGA_SEQ_D,reg,val); }
LOCAL u8   KCALL vga_io_rseq(u8 reg) { vga_io_w(VGA_SEQ_I,reg); return vga_io_r(VGA_SEQ_D); }
LOCAL void KCALL vga_io_wseq(u8 reg, u8 val) { __VGA_OUTW_SELECTOR(vga_io_w,VGA_SEQ_I,VGA_SEQ_D,reg,val); }
LOCAL u8   KCALL vga_mm_rseq(MMIO void *regbase, u8 reg) { vga_mm_w(regbase,VGA_SEQ_I,reg); return vga_mm_r(regbase,VGA_SEQ_D); }
LOCAL void KCALL vga_mm_wseq(MMIO void *regbase, u8 reg, u8 val) { __VGA_OUTW_SELECTOR2(vga_mm_w,regbase,VGA_SEQ_I,VGA_SEQ_D,reg,val); }

/* VGA graphics controller register read/write */
LOCAL u8   KCALL vga_rgfx(MMIO void *regbase, u8 reg) { vga_w(regbase,VGA_GFX_I,reg); return vga_r(regbase,VGA_GFX_D); }
LOCAL void KCALL vga_wgfx(MMIO void *regbase, u8 reg, u8 val) { __VGA_OUTW_SELECTOR2(vga_w,regbase,VGA_GFX_I,VGA_GFX_D,reg,val); }
LOCAL u8   KCALL vga_io_rgfx(u8 reg) { vga_io_w(VGA_GFX_I,reg); return vga_io_r(VGA_GFX_D); }
LOCAL void KCALL vga_io_wgfx(u8 reg, u8 val) { __VGA_OUTW_SELECTOR(vga_io_w,VGA_GFX_I,VGA_GFX_D,reg,val); }
LOCAL u8   KCALL vga_mm_rgfx(MMIO void *regbase, u8 reg) { vga_mm_w(regbase,VGA_GFX_I,reg); return vga_mm_r(regbase,VGA_GFX_D); }
LOCAL void KCALL vga_mm_wgfx(MMIO void *regbase, u8 reg, u8 val) { __VGA_OUTW_SELECTOR2(vga_mm_w,regbase,VGA_GFX_I,VGA_GFX_D,reg,val); }

/* VGA attribute controller register read/write */
LOCAL u8   KCALL vga_rattr(MMIO void *regbase, u8 reg) { vga_w(regbase,VGA_ATT_IW,reg); return vga_r(regbase,VGA_ATT_R); }
LOCAL void KCALL vga_wattr(MMIO void *regbase, u8 reg, u8 val) { vga_w(regbase,VGA_ATT_IW,reg); vga_w(regbase,VGA_ATT_W,val); }
LOCAL u8   KCALL vga_io_rattr(u8 reg) { vga_io_w(VGA_ATT_IW,reg); return vga_io_r(VGA_ATT_R); }
LOCAL void KCALL vga_io_wattr(u8 reg, u8 val) { vga_io_w(VGA_ATT_IW,reg); vga_io_w(VGA_ATT_W,val); }
LOCAL u8   KCALL vga_mm_rattr(MMIO void *regbase, u8 reg) { vga_mm_w(regbase,VGA_ATT_IW,reg); return vga_mm_r(regbase,VGA_ATT_R); }
LOCAL void KCALL vga_mm_wattr(MMIO void *regbase, u8 reg, u8 val) { vga_mm_w(regbase,VGA_ATT_IW,reg); vga_mm_w(regbase,VGA_ATT_W,val); }





typedef struct {
    struct character_device v_dev;       /* The underlying VGA character device. */
    atomic_rwlock_t         v_lock;      /* Lock for accessing the VGA Hardware. */
    VIRT MMIO void         *v_mmio;      /* [0..1][const] Virtual MMIO memory address (or NULL if not supported). */
    REF struct vm_region   *v_vram;      /* [0..1][lock(WRITE_ONCE)] The VGA VRAM memory region. */
    PHYS vm_ppage_t         v_vram_addr; /* [const] VRAM base address. */
    size_t                  v_vram_size; /* [const] VRAM size (in bytes). */
    /* VGA color/mono registers (Determined by `vga_r(v_mmio,VGA_MIS_R)&VGA_MIS_COLOR') */
    u16                     v_crt_i;     /* [const] CRT Controller Index (Either `VGA_CRT_IC' or `VGA_CRT_IM') */
    u16                     v_crt_d;     /* [const] CRT Controller Data Register (Either `VGA_CRT_DC' or `VGA_CRT_DM') */
    u16                     v_is1_r;     /* [const] Input Status Register 1 (Either `VGA_IS1_RC' or `VGA_IS1_RM') */
#define VGA_STATE_FNORMAL    0x0000      /* Normal VGA state flags. */
#define VGA_STATE_FSCREENOFF 0x0001      /* The screen has been turned off. */
    u16                     v_state;     /* [lock(v_lock)] VGA state flags. */
    struct vga_font         v_bios_font; /* [const] The VGA BIOS Font. */
} VGA;

INTDEF struct vga_mode    const vga_biosmode; /* 80x25 color text mode (BIOS 3h). */
INTDEF struct vga_palette const vga_biospal;  /* Palette associated with `vga_biosmode' */
INTDEF struct vga_mode    const vga_mode_gfx320x200_256;
INTDEF struct vga_mode    const vga_mode_gfx640x480_16;
INTDEF struct vga_palette const vga_pal_gfx16;
INTDEF struct vga_palette const vga_pal_gfx256;

INTDEF void KCALL VGA_SetMode(VGA *__restrict self, struct vga_mode const *__restrict mode);
INTDEF void KCALL VGA_GetMode(VGA *__restrict self, struct vga_mode *__restrict mode);
INTDEF void KCALL VGA_SetFont(VGA *__restrict self, USER CHECKED struct vga_font const *__restrict font);
INTDEF void KCALL VGA_GetFont(VGA *__restrict self, USER CHECKED struct vga_font *__restrict font);
INTDEF void KCALL VGA_SetPalette(VGA *__restrict self, USER CHECKED struct vga_palette const *__restrict pal);
INTDEF void KCALL VGA_GetPalette(VGA *__restrict self, USER CHECKED struct vga_palette *__restrict pal);
INTDEF void KCALL VGA_ScreenOn(VGA *__restrict self);
INTDEF void KCALL VGA_ScreenOff(VGA *__restrict self);


DECL_END

#endif /* !GUARD_KERNEL_MODULES_VGA_VGA_H */
