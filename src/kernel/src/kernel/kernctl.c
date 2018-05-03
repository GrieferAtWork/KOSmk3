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
#ifndef GUARD_KERNEL_SRC_KERNEL_KERNCTL_C
#define GUARD_KERNEL_SRC_KERNEL_KERNCTL_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <hybrid/host.h>
#include <kernel/syscall.h>
#include <kernel/heap.h>
#include <kernel/malloc.h>
#include <kernel/user.h>
#include <kernel/cache.h>
#include <fs/driver.h>
#include <fs/path.h>
#include <fs/node.h>
#include <kos/kernctl.h>
#include <except.h>

DECL_BEGIN

PRIVATE ATTR_NOTHROW bool KCALL test_fail(void *UNUSED(arg)) {
 return false;
}


#if 1
#include <sys/io.h>

/* VGA data register ports */
#define VGA_CRT_DC      0x3d5 /*< CRT Controller Data Register - color emulation. */
#define VGA_CRT_DM      0x3b5 /*< CRT Controller Data Register - mono emulation. */
#define VGA_ATT_R       0x3c1 /*< Attribute Controller Data Read Register. */
#define VGA_ATT_W       0x3c0 /*< Attribute Controller Data Write Register. */
#define VGA_GFX_D       0x3cf /*< Graphics Controller Data Register. */
#define VGA_SEQ_D       0x3c5 /*< Sequencer Data Register. */
#define VGA_MIS_R       0x3cc /*< Misc Output Read Register. */
#define VGA_MIS_W       0x3c2 /*< Misc Output Write Register. */
#   define VGA_MIS_RESERVED         0x10 /* Mask of reserved registers (Added by GrieferAtWork). */
#   define VGA_MIS_COLOR            0x01
#   define VGA_MIS_ENB_MEM_ACCESS   0x02
#   define VGA_MIS_DCLK_28322_720   0x04
#   define VGA_MIS_ENB_PLL_LOAD    (0x04|0x08)
#   define VGA_MIS_SEL_HIGH_PAGE    0x20
#define VGA_FTC_R       0x3ca /*< Feature Control Read Register. */
#define VGA_IS1_RC      0x3da /*< Input Status Register 1 - color emulation. */
#define VGA_IS1_RM      0x3ba /*< Input Status Register 1 - mono emulation. */
#define VGA_PEL_D       0x3c9 /*< PEL Data Register. */
#define VGA_PEL_MSK     0x3c6 /*< PEL mask register. */

/* EGA-specific registers */
#define EGA_GFX_E0      0x3cc /*< Graphics enable processor 0. */
#define EGA_GFX_E1      0x3ca /*< Graphics enable processor 1. */

/* VGA index register ports */
#define VGA_CRT_IC      0x3d4 /*< CRT Controller Index - color emulation. */
#define VGA_CRT_IM      0x3b4 /*< CRT Controller Index - mono emulation. */
#define VGA_ATT_IW      0x3c0 /*< Attribute Controller Index & Data Write Register. */
#define VGA_GFX_I       0x3ce /*< Graphics Controller Index. */
#define VGA_SEQ_I       0x3c4 /*< Sequencer Index. */
#define VGA_PEL_IW      0x3c8 /*< PEL Write Index. */
#define VGA_PEL_IR      0x3c7 /*< PEL Read Index. */

/* standard VGA indexes max counts */
#define VGA_CRT_C       0x19  /*< Number of CRT Controller Registers. */
#define VGA_ATT_C       0x15  /*< Number of Attribute Controller Registers. */
#define VGA_GFX_C       0x09  /*< Number of Graphics Controller Registers. */
#define VGA_SEQ_C       0x05  /*< Number of Sequencer Registers. */
#define VGA_MIS_C       0x01  /*< Number of Misc Output Register. */

/* VGA CRT controller register indices */
#define VGA_CRTC_H_TOTAL       0
#define VGA_CRTC_H_DISP        1
#define VGA_CRTC_H_BLANK_START 2
#define VGA_CRTC_H_BLANK_END   3
#   define VGA_CR3_MASK            0x1f /* Mask of bits used for hblank-end (Added by GrieferAtWork) */
#   define VGA_CR3_ALWAYS1         0x80 /* Always set this bit when writing this register (backward compatibility) */
#define VGA_CRTC_H_SYNC_START  4
#define VGA_CRTC_H_SYNC_END    5
#   define VGA_CR5_MASK            0x1f /* Mask of bits used for hsync-end (Added by GrieferAtWork) */
#   define VGA_CR5_H_BLANK_END_5   0x80 /* 5th bit for `VGA_CRTC_H_BLANK_END' (Added by GrieferAtWork) */
#define VGA_CRTC_V_TOTAL       6
#define VGA_CRTC_OVERFLOW      7
#   define VGA_CR7_V_TOTAL_8       0x01 /* 8th bit for `VGA_CRTC_V_TOTAL' (Added by GrieferAtWork) */
#   define VGA_CR7_V_DISP_END_8    0x02 /* 8th bit for `VGA_CRTC_V_DISP_END' (Added by GrieferAtWork) */
#   define VGA_CR7_V_SYNC_START_8  0x04 /* 8th bit for `VGA_CRTC_V_SYNC_START' (Added by GrieferAtWork) */
#   define VGA_CR7_V_BLANK_START_8 0x08 /* 8th bit for `VGA_CRTC_V_BLANK_START' (Added by GrieferAtWork) */
#   define VGA_CR7_V_LINECOMP_8    0x10 /* 8th bit for `VGA_CRTC_LINE_COMPARE' (Added by GrieferAtWork) */
#   define VGA_CR7_V_TOTAL_9       0x20 /* 9th bit for `VGA_CRTC_V_TOTAL' (Added by GrieferAtWork) */
#   define VGA_CR7_V_DISP_END_9    0x40 /* 9th bit for `VGA_CRTC_V_DISP_END' (Added by GrieferAtWork) */
#   define VGA_CR7_V_SYNC_START_9  0x80 /* 9th bit for `VGA_CRTC_V_SYNC_START' (Added by GrieferAtWork) */
#define VGA_CRTC_PRESET_ROW    8
#   define VGA_CR8_RESERVED        0x80 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_CRTC_MAX_SCAN      9
#   define VGA_CR9_MASK            0x1f /* Mask of bits used for max-scan (Added by GrieferAtWork) */
#   define VGA_CR9_V_BLANK_START_9 0x20 /* 9th bit for `VGA_CRTC_V_BLANK_START' (Added by GrieferAtWork) */
#   define VGA_CR9_V_LINECOMP_9    0x40 /* 9th bit for `VGA_CRTC_LINE_COMPARE' (Added by GrieferAtWork) */
#   define VGA_CR9_SCANDOUBLE      0x80 /* Better don't set... (Don't really understand what this done) (Added by GrieferAtWork) */
#define VGA_CRTC_CURSOR_START  0x0a
#   define VGA_CRTC_CURSOR_DISABLE 0x20 /* Disable the text-mode cursor (Added by GrieferAtWork) */
#define VGA_CRTC_CURSOR_END    0x0b
#define VGA_CRTC_START_HI      0x0c
#define VGA_CRTC_START_LO      0x0d
#define VGA_CRTC_CURSOR_HI     0x0e
#define VGA_CRTC_CURSOR_LO     0x0f
#define VGA_CRTC_V_SYNC_START  0x10
#define VGA_CRTC_V_SYNC_END    0x11
#   define VGA_CR11_RESERVED     0x30 /* Mask of reserved registers (Added by GrieferAtWork). */
#   define VGA_CR11_MASK         0x0f /* Mask of bits used for vsync-end (Added by GrieferAtWork) */
#   define VGA_CR11_LOCK_CR0_CR7 0x80 /* lock writes to CR0 - CR7. */
#define VGA_CRTC_V_DISP_END    0x12
#define VGA_CRTC_OFFSET        0x13
#define VGA_CRTC_UNDERLINE     0x14
#define VGA_CRTC_V_BLANK_START 0x15
#define VGA_CRTC_V_BLANK_END   0x16
#   define VGA_CR16_RESERVED     0x80 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_CRTC_MODE          0x17
#   define VGA_CR17_RESERVED     0x10 /* Mask of reserved registers (Added by GrieferAtWork). */
#   define VGA_CR17_H_V_SIGNALS_ENABLED 0x80
#define VGA_CRTC_LINE_COMPARE  0x18
#define VGA_CRTC_REGS          VGA_CRT_C

/* VGA attribute controller register indices */
#define VGA_ATC_PALETTE0       0x00
#define VGA_ATC_PALETTE1       0x01
#define VGA_ATC_PALETTE2       0x02
#define VGA_ATC_PALETTE3       0x03
#define VGA_ATC_PALETTE4       0x04
#define VGA_ATC_PALETTE5       0x05
#define VGA_ATC_PALETTE6       0x06
#define VGA_ATC_PALETTE7       0x07
#define VGA_ATC_PALETTE8       0x08
#define VGA_ATC_PALETTE9       0x09
#define VGA_ATC_PALETTEA       0x0a
#define VGA_ATC_PALETTEB       0x0b
#define VGA_ATC_PALETTEC       0x0c
#define VGA_ATC_PALETTED       0x0d
#define VGA_ATC_PALETTEE       0x0e
#define VGA_ATC_PALETTEF       0x0f
#define VGA_ATC_MODE           0x10
#   define VGA_AT10_RESERVED      0x10 /* Mask of reserved registers (Added by GrieferAtWork). */
#   define VGA_AT10_GRAPHICS      0x01 /* Enable graphics, rather than alphanumeric mode (Added by GrieferAtWork). */
#   define VGA_AT10_DUP9          0x04 /* Duplicate the 8`th text dot into the 9`th when `VGA_SR01_CHAR_CLK_8DOTS' isn't set, instead of filling it with background (Added by GrieferAtWork). */
#   define VGA_AT10_BLINK         0x08 /* Set to cause character attribute bit #7 to be used for blinking text;
                                        * NEVER SET THIS! YOU'LL CAUSE SEIZURES IN PEOPLE (Added by GrieferAtWork).
                                        * WARNING: After boot, a BIOS mode switch, or a video card reset, this flag
                                        *          is enabled and _MUST_ under all circumstances be disabled again! */
#   define VGA_AT10_8BITPAL       0x40 /* 8-bit palette index (Added by GrieferAtWork). */
#define VGA_ATC_OVERSCAN       0x11
#define VGA_ATC_PLANE_ENABLE   0x12
#   define VGA_AT12_MASK          0x0f /* Mask of planes (Added by GrieferAtWork). */
#   define VGA_AT12_RESERVED      0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_ATC_PEL            0x13
#   define VGA_AT13_MASK          0x0f /* Mask for pixel panning (Added by GrieferAtWork). */
#   define VGA_AT13_RESERVED      0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_ATC_COLOR_PAGE     0x14
#   define VGA_AT14_RESERVED      0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */

#define VGA_AR_ENABLE_DISPLAY  0x20

/* VGA sequencer register indices */
#define VGA_SEQ_RESET          0x00
#define VGA_SEQ_CLOCK_MODE     0x01
#   define VGA_SR01_CHAR_CLK_8DOTS 0x01 /* bit 0: character clocks 8 dots wide are generated */
#   define VGA_SR01_SCREEN_OFF     0x20 /* bit 5: Screen is off */
#   define VGA_SR01_RESERVED       0xc2 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_SEQ_PLANE_WRITE    0x02
#   define VGA_SR02_PLANE(i)     ((1) << i) /* Added by GrieferAtWork */
#   define VGA_SR02_ALL_PLANES     0x0f /* bits 3-0: enable access to all planes */
#   define VGA_SR02_RESERVED       0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_SEQ_CHARACTER_MAP  0x03
#   define VGA_SR03_RESERVED       0xc0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_SEQ_MEMORY_MODE    0x04
#   define VGA_SR04_EXT_MEM        0x02 /* bit 1: allows complete mem access to 256K */
#   define VGA_SR04_SEQ_MODE       0x04 /* bit 2: directs system to use a sequential addressing mode */
#   define VGA_SR04_CHN_4M         0x08 /* bit 3: selects modulo 4 addressing for CPU access to display memory */
#   define VGA_SR04_RESERVED       0xf1 /* Mask of reserved registers (Added by GrieferAtWork). */

/* VGA graphics controller register indices */
#define VGA_GFX_SR_VALUE        0x00
#   define VGA_GR00_RESERVED       0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_SR_ENABLE       0x01
#   define VGA_GR01_RESERVED       0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_COMPARE_VALUE   0x02
#   define VGA_GR02_RESERVED       0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_DATA_ROTATE     0x03
#   define VGA_GR03_RESERVED       0xe0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_PLANE_READ      0x04
#   define VGA_GR04_RESERVED       0xfc /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_MODE            0x05
#   define VGA_GR05_RESERVED       0x84 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_MISC            0x06
#   define VGA_GR06_RESERVED       0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#   define VGA_GR06_GRAPHICS_MODE  0x01
#define VGA_GFX_COMPARE_MASK    0x07
#   define VGA_GR07_RESERVED       0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_BIT_MASK        0x08

/* macro for composing an 8-bit VGA register
 * index and value into a single 16-bit quantity */
#define VGA_OUT16VAL(v,r)    (((v) << 8)|(r))

/* decide whether we should enable the faster 16-bit VGA register writes */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define VGA_OUTW_WRITE 1
#endif


/* generic VGA port read/write */
LOCAL u8   KCALL vga_io_r(u16 port) { return inb_p(port); }
LOCAL void KCALL vga_io_w(u16 port, u8 val) { outb_p(port,val); }
LOCAL void KCALL vga_io_w_fast(u16 port, u8 reg, u8 val) { outw(port,VGA_OUT16VAL(val,reg)); }
LOCAL u8   KCALL vga_mm_r(MMIO void *regbase, u16 port) { return readb((MMIO byte_t *)regbase+port); }
LOCAL void KCALL vga_mm_w(MMIO void *regbase, u16 port, u8 val) { writeb((MMIO byte_t *)regbase+port,val); }
LOCAL void KCALL vga_mm_w_fast(MMIO void *regbase, u16 port, u8 reg, u8 val) { writew((MMIO byte_t *)regbase+port,VGA_OUT16VAL(val,reg)); }
LOCAL u8   KCALL vga_r(MMIO void *regbase, u16 port) { return regbase ? vga_mm_r(regbase,port) : vga_io_r(port); }
LOCAL void KCALL vga_w(MMIO void *regbase, u16 port, u8 val) { regbase ? vga_mm_w(regbase,port,val) : vga_io_w(port,val); }
LOCAL void KCALL vga_w_fast(MMIO void *regbase, u16 port, u8 reg, u8 val) { regbase ? vga_mm_w_fast(regbase,port,reg,val) : vga_io_w_fast(port,reg,val); }

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



struct vga_pal { u8 vp_pal[256][3]; };
struct vga_mode {
 u8 vm_att_mode;          /*< VGA_ATC_MODE. */
 u8 vm_att_overscan;      /*< VGA_ATC_OVERSCAN. */
 u8 vm_att_plane_enable;  /*< VGA_ATC_PLANE_ENABLE. */
 u8 vm_att_pel;           /*< VGA_ATC_PEL. */
 u8 vm_att_color_page;    /*< VGA_ATC_COLOR_PAGE. */
 u8 vm_mis;               /*< VGA_MIS_R / VGA_MIS_W. */
 u8 vm_gfx_sr_value;      /*< VGA_GFX_SR_VALUE. */
 u8 vm_gfx_sr_enable;     /*< VGA_GFX_SR_ENABLE. */
 u8 vm_gfx_compare_value; /*< VGA_GFX_COMPARE_VALUE. */
 u8 vm_gfx_data_rotate;   /*< VGA_GFX_DATA_ROTATE. */
 u8 vm_gfx_mode;          /*< VGA_GFX_MODE. */
 u8 vm_gfx_misc;          /*< VGA_GFX_MISC. */
 u8 vm_gfx_compare_mask;  /*< VGA_GFX_COMPARE_MASK. */
 u8 vm_gfx_bit_mask;      /*< VGA_GFX_BIT_MASK. */
 u8 vm_crt_h_total;       /*< VGA_CRTC_H_TOTAL. */
 u8 vm_crt_h_disp;        /*< VGA_CRTC_H_DISP. */
 u8 vm_crt_h_blank_start; /*< VGA_CRTC_H_BLANK_START. */
 u8 vm_crt_h_blank_end;   /*< VGA_CRTC_H_BLANK_END. */
 u8 vm_crt_h_sync_start;  /*< VGA_CRTC_H_SYNC_START. */
 u8 vm_crt_h_sync_end;    /*< VGA_CRTC_H_SYNC_END. */
 u8 vm_crt_v_total;       /*< VGA_CRTC_V_TOTAL. */
 u8 vm_crt_overflow;      /*< VGA_CRTC_OVERFLOW. */
 u8 vm_crt_preset_row;    /*< VGA_CRTC_PRESET_ROW. */
 u8 vm_crt_max_scan;      /*< VGA_CRTC_MAX_SCAN. */
 u8 vm_crt_v_sync_start;  /*< VGA_CRTC_V_SYNC_START. */
 u8 vm_crt_v_sync_end;    /*< VGA_CRTC_V_SYNC_END. */
 u8 vm_crt_v_disp_end;    /*< VGA_CRTC_V_DISP_END. */
 u8 vm_crt_offset;        /*< VGA_CRTC_OFFSET. */
 u8 vm_crt_underline;     /*< VGA_CRTC_UNDERLINE. */
 u8 vm_crt_v_blank_start; /*< VGA_CRTC_V_BLANK_START. */
 u8 vm_crt_v_blank_end;   /*< VGA_CRTC_V_BLANK_END. */
 u8 vm_crt_mode;          /*< VGA_CRTC_MODE. */
 u8 vm_crt_line_compare;  /*< VGA_CRTC_LINE_COMPARE. */
 u8 vm_seq_clock_mode;    /*< VGA_SEQ_CLOCK_MODE. */
 u8 vm_seq_plane_write;   /*< VGA_SEQ_PLANE_WRITE. */
 u8 vm_seq_character_map; /*< VGA_SEQ_CHARACTER_MAP. */
 u8 vm_seq_memory_mode;   /*< VGA_SEQ_MEMORY_MODE. */
};
PRIVATE struct vga_mode const vm_gfx = {
    .vm_att_mode          = 0x41,
    .vm_att_overscan      = 0x00,
    .vm_att_plane_enable  = 0x0f,
    .vm_att_pel           = 0x00,
    .vm_att_color_page    = 0x00,
    .vm_mis               = 0x63, /* 0xe3 */
    .vm_gfx_sr_value      = 0x00,
    .vm_gfx_sr_enable     = 0x00,
    .vm_gfx_compare_value = 0x00,
    .vm_gfx_data_rotate   = 0x00,
    .vm_gfx_mode          = 0x40,
    .vm_gfx_misc          = 0x05, /* 0x01 */
    .vm_gfx_compare_mask  = 0x0f, /* 0x00 */
    .vm_gfx_bit_mask      = 0xff,
    .vm_crt_h_total       = 0x5f,
    .vm_crt_h_disp        = 0x4f,
    .vm_crt_h_blank_start = 0x50,
    .vm_crt_h_blank_end   = 0x82,
    .vm_crt_h_sync_start  = 0x54,
    .vm_crt_h_sync_end    = 0x80,
    .vm_crt_v_total       = 0xbf, /* 0x0d */
    .vm_crt_overflow      = 0x1f, /* 0x3e */
    .vm_crt_preset_row    = 0x00,
    .vm_crt_max_scan      = 0x41,
    .vm_crt_v_sync_start  = 0x9c, /* 0xea */
    .vm_crt_v_sync_end    = 0x8e, /* 0xac */
    .vm_crt_v_disp_end    = 0x8f, /* 0xdf */
    .vm_crt_offset        = 0x28,
    .vm_crt_underline     = 0x40, /* 0x00 */
    .vm_crt_v_blank_start = 0x96, /* 0xe7 */
    .vm_crt_v_blank_end   = 0xb9, /* 0x06 */
    .vm_crt_mode          = 0xa3, /* 0xe3 */
    .vm_crt_line_compare  = 0xff,
    .vm_seq_clock_mode    = 0x01,
    .vm_seq_plane_write   = VGA_SR02_ALL_PLANES,
    .vm_seq_character_map = 0x00,
    .vm_seq_memory_mode   = 0x0e, /* 0x06 */
};
PRIVATE struct vga_pal const vp_gfx_256 = {
    {   {0x00,0x00,0x00},{0x00,0x00,0x2a},{0x00,0x2a,0x00},{0x00,0x2a,0x2a},{0x2a,0x00,0x00},{0x2a,0x00,0x2a},{0x2a,0x15,0x00},{0x2a,0x2a,0x2a},
        {0x15,0x15,0x15},{0x15,0x15,0x3f},{0x15,0x3f,0x15},{0x15,0x3f,0x3f},{0x3f,0x15,0x15},{0x3f,0x15,0x3f},{0x3f,0x3f,0x15},{0x3f,0x3f,0x3f},
        {0x00,0x00,0x00},{0x05,0x05,0x05},{0x08,0x08,0x08},{0x0b,0x0b,0x0b},{0x0e,0x0e,0x0e},{0x11,0x11,0x11},{0x14,0x14,0x14},{0x18,0x18,0x18},
        {0x1c,0x1c,0x1c},{0x20,0x20,0x20},{0x24,0x24,0x24},{0x28,0x28,0x28},{0x2d,0x2d,0x2d},{0x32,0x32,0x32},{0x38,0x38,0x38},{0x3f,0x3f,0x3f},
        {0x00,0x00,0x3f},{0x10,0x00,0x3f},{0x1f,0x00,0x3f},{0x2f,0x00,0x3f},{0x3f,0x00,0x3f},{0x3f,0x00,0x2f},{0x3f,0x00,0x1f},{0x3f,0x00,0x10},
        {0x3f,0x00,0x00},{0x3f,0x10,0x00},{0x3f,0x1f,0x00},{0x3f,0x2f,0x00},{0x3f,0x3f,0x00},{0x2f,0x3f,0x00},{0x1f,0x3f,0x00},{0x10,0x3f,0x00},
        {0x00,0x3f,0x00},{0x00,0x3f,0x10},{0x00,0x3f,0x1f},{0x00,0x3f,0x2f},{0x00,0x3f,0x3f},{0x00,0x2f,0x3f},{0x00,0x1f,0x3f},{0x00,0x10,0x3f},
        {0x1f,0x1f,0x3f},{0x27,0x1f,0x3f},{0x2f,0x1f,0x3f},{0x37,0x1f,0x3f},{0x3f,0x1f,0x3f},{0x3f,0x1f,0x37},{0x3f,0x1f,0x2f},{0x3f,0x1f,0x27},
        {0x3f,0x1f,0x1f},{0x3f,0x27,0x1f},{0x3f,0x2f,0x1f},{0x3f,0x37,0x1f},{0x3f,0x3f,0x1f},{0x37,0x3f,0x1f},{0x2f,0x3f,0x1f},{0x27,0x3f,0x1f},
        {0x1f,0x3f,0x1f},{0x1f,0x3f,0x27},{0x1f,0x3f,0x2f},{0x1f,0x3f,0x37},{0x1f,0x3f,0x3f},{0x1f,0x37,0x3f},{0x1f,0x2f,0x3f},{0x1f,0x27,0x3f},
        {0x2d,0x2d,0x3f},{0x31,0x2d,0x3f},{0x36,0x2d,0x3f},{0x3a,0x2d,0x3f},{0x3f,0x2d,0x3f},{0x3f,0x2d,0x3a},{0x3f,0x2d,0x36},{0x3f,0x2d,0x31},
        {0x3f,0x2d,0x2d},{0x3f,0x31,0x2d},{0x3f,0x36,0x2d},{0x3f,0x3a,0x2d},{0x3f,0x3f,0x2d},{0x3a,0x3f,0x2d},{0x36,0x3f,0x2d},{0x31,0x3f,0x2d},
        {0x2d,0x3f,0x2d},{0x2d,0x3f,0x31},{0x2d,0x3f,0x36},{0x2d,0x3f,0x3a},{0x2d,0x3f,0x3f},{0x2d,0x3a,0x3f},{0x2d,0x36,0x3f},{0x2d,0x31,0x3f},
        {0x00,0x00,0x1c},{0x07,0x00,0x1c},{0x0e,0x00,0x1c},{0x15,0x00,0x1c},{0x1c,0x00,0x1c},{0x1c,0x00,0x15},{0x1c,0x00,0x0e},{0x1c,0x00,0x07},
        {0x1c,0x00,0x00},{0x1c,0x07,0x00},{0x1c,0x0e,0x00},{0x1c,0x15,0x00},{0x1c,0x1c,0x00},{0x15,0x1c,0x00},{0x0e,0x1c,0x00},{0x07,0x1c,0x00},
        {0x00,0x1c,0x00},{0x00,0x1c,0x07},{0x00,0x1c,0x0e},{0x00,0x1c,0x15},{0x00,0x1c,0x1c},{0x00,0x15,0x1c},{0x00,0x0e,0x1c},{0x00,0x07,0x1c},
        {0x0e,0x0e,0x1c},{0x11,0x0e,0x1c},{0x15,0x0e,0x1c},{0x18,0x0e,0x1c},{0x1c,0x0e,0x1c},{0x1c,0x0e,0x18},{0x1c,0x0e,0x15},{0x1c,0x0e,0x11},
        {0x1c,0x0e,0x0e},{0x1c,0x11,0x0e},{0x1c,0x15,0x0e},{0x1c,0x18,0x0e},{0x1c,0x1c,0x0e},{0x18,0x1c,0x0e},{0x15,0x1c,0x0e},{0x11,0x1c,0x0e},
        {0x0e,0x1c,0x0e},{0x0e,0x1c,0x11},{0x0e,0x1c,0x15},{0x0e,0x1c,0x18},{0x0e,0x1c,0x1c},{0x0e,0x18,0x1c},{0x0e,0x15,0x1c},{0x0e,0x11,0x1c},
        {0x14,0x14,0x1c},{0x16,0x14,0x1c},{0x18,0x14,0x1c},{0x1a,0x14,0x1c},{0x1c,0x14,0x1c},{0x1c,0x14,0x1a},{0x1c,0x14,0x18},{0x1c,0x14,0x16},
        {0x1c,0x14,0x14},{0x1c,0x16,0x14},{0x1c,0x18,0x14},{0x1c,0x1a,0x14},{0x1c,0x1c,0x14},{0x1a,0x1c,0x14},{0x18,0x1c,0x14},{0x16,0x1c,0x14},
        {0x14,0x1c,0x14},{0x14,0x1c,0x16},{0x14,0x1c,0x18},{0x14,0x1c,0x1a},{0x14,0x1c,0x1c},{0x14,0x1a,0x1c},{0x14,0x18,0x1c},{0x14,0x16,0x1c},
        {0x00,0x00,0x10},{0x04,0x00,0x10},{0x08,0x00,0x10},{0x0c,0x00,0x10},{0x10,0x00,0x10},{0x10,0x00,0x0c},{0x10,0x00,0x08},{0x10,0x00,0x04},
        {0x10,0x00,0x00},{0x10,0x04,0x00},{0x10,0x08,0x00},{0x10,0x0c,0x00},{0x10,0x10,0x00},{0x0c,0x10,0x00},{0x08,0x10,0x00},{0x04,0x10,0x00},
        {0x00,0x10,0x00},{0x00,0x10,0x04},{0x00,0x10,0x08},{0x00,0x10,0x0c},{0x00,0x10,0x10},{0x00,0x0c,0x10},{0x00,0x08,0x10},{0x00,0x04,0x10},
        {0x08,0x08,0x10},{0x0a,0x08,0x10},{0x0c,0x08,0x10},{0x0e,0x08,0x10},{0x10,0x08,0x10},{0x10,0x08,0x0e},{0x10,0x08,0x0c},{0x10,0x08,0x0a},
        {0x10,0x08,0x08},{0x10,0x0a,0x08},{0x10,0x0c,0x08},{0x10,0x0e,0x08},{0x10,0x10,0x08},{0x0e,0x10,0x08},{0x0c,0x10,0x08},{0x0a,0x10,0x08},
        {0x08,0x10,0x08},{0x08,0x10,0x0a},{0x08,0x10,0x0c},{0x08,0x10,0x0e},{0x08,0x10,0x10},{0x08,0x0e,0x10},{0x08,0x0c,0x10},{0x08,0x0a,0x10},
        {0x0b,0x0b,0x10},{0x0c,0x0b,0x10},{0x0d,0x0b,0x10},{0x0f,0x0b,0x10},{0x10,0x0b,0x10},{0x10,0x0b,0x0f},{0x10,0x0b,0x0d},{0x10,0x0b,0x0c},
        {0x10,0x0b,0x0b},{0x10,0x0c,0x0b},{0x10,0x0d,0x0b},{0x10,0x0f,0x0b},{0x10,0x10,0x0b},{0x0f,0x10,0x0b},{0x0d,0x10,0x0b},{0x0c,0x10,0x0b},
        {0x0b,0x10,0x0b},{0x0b,0x10,0x0c},{0x0b,0x10,0x0d},{0x0b,0x10,0x0f},{0x0b,0x10,0x10},{0x0b,0x0f,0x10},{0x0b,0x0d,0x10},{0x0b,0x0c,0x10},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
    }
};
PRIVATE void KCALL
vga_setmode(MMIO void *regbase,
            struct vga_mode const *__restrict mode) {
 /* Disable preemption to prevent interference. */
 pflag_t was = PREEMPTION_PUSHOFF();
 u8 temp,qr1 = vga_rseq(regbase,VGA_SEQ_CLOCK_MODE);

 /* Validate the given mode. */
 /* TODO: Stuff like this: assert(!(mode->vm_att_mode&0x10)); */

 /* Turn off the screen. */
 vga_wseq(regbase,VGA_SEQ_RESET,0x1);
 vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1|VGA_SR01_SCREEN_OFF);
 vga_wseq(regbase,VGA_SEQ_RESET,0x3);

 /* Enable graphics mode. */
 vga_r(regbase,VGA_IS1_RC),vga_w(regbase,VGA_ATT_W,0x00);
 vga_r(regbase,VGA_IS1_RC),temp = vga_rattr(regbase,VGA_ATC_MODE);
 vga_r(regbase,VGA_IS1_RC),vga_wattr(regbase,VGA_ATC_MODE,(temp&VGA_AT10_RESERVED)|mode->vm_att_mode);
 vga_r(regbase,VGA_IS1_RC),vga_wattr(regbase,VGA_ATC_OVERSCAN,mode->vm_att_overscan);
 vga_r(regbase,VGA_IS1_RC),temp = vga_rattr(regbase,VGA_ATC_PLANE_ENABLE);
 vga_r(regbase,VGA_IS1_RC),vga_wattr(regbase,VGA_ATC_PLANE_ENABLE,(temp&~VGA_AT12_MASK)|mode->vm_att_plane_enable);
 vga_r(regbase,VGA_IS1_RC),temp = vga_rattr(regbase,VGA_ATC_PEL);
 vga_r(regbase,VGA_IS1_RC),vga_wattr(regbase,VGA_ATC_PEL,(temp&VGA_AT13_RESERVED)|mode->vm_att_pel);
 vga_r(regbase,VGA_IS1_RC),temp = vga_rattr(regbase,VGA_ATC_COLOR_PAGE);
 vga_r(regbase,VGA_IS1_RC),vga_wattr(regbase,VGA_ATC_COLOR_PAGE,(temp&VGA_AT14_RESERVED)|mode->vm_att_color_page);
 vga_r(regbase,VGA_IS1_RC),vga_w(regbase,VGA_ATT_W,0x20);

 temp = vga_r(regbase,VGA_MIS_R);
 vga_w(regbase,VGA_MIS_W,(temp&VGA_MIS_RESERVED)|mode->vm_mis);

 temp = vga_rseq(regbase,VGA_SEQ_PLANE_WRITE);
 vga_wseq(regbase,VGA_SEQ_PLANE_WRITE,(temp&VGA_SR02_RESERVED)|mode->vm_seq_plane_write);
 temp = vga_rseq(regbase,VGA_SEQ_CHARACTER_MAP);
 vga_wseq(regbase,VGA_SEQ_CHARACTER_MAP,(temp&VGA_SR03_RESERVED)|mode->vm_seq_character_map);
 temp = vga_rseq(regbase,VGA_SEQ_MEMORY_MODE);
 vga_wseq(regbase,VGA_SEQ_MEMORY_MODE,(temp&VGA_SR04_RESERVED)|mode->vm_seq_memory_mode);

 temp = vga_rgfx(regbase,VGA_GFX_SR_VALUE),vga_wgfx(regbase,VGA_GFX_SR_VALUE,(temp&VGA_GR00_RESERVED)|mode->vm_gfx_sr_value);
 temp = vga_rgfx(regbase,VGA_GFX_SR_ENABLE),vga_wgfx(regbase,VGA_GFX_SR_ENABLE,(temp&VGA_GR01_RESERVED)|mode->vm_gfx_sr_enable);
 temp = vga_rgfx(regbase,VGA_GFX_COMPARE_VALUE),vga_wgfx(regbase,VGA_GFX_COMPARE_VALUE,(temp&VGA_GR02_RESERVED)|mode->vm_gfx_compare_value);
 temp = vga_rgfx(regbase,VGA_GFX_DATA_ROTATE),vga_wgfx(regbase,VGA_GFX_DATA_ROTATE,(temp&VGA_GR03_RESERVED)|mode->vm_gfx_data_rotate);
 temp = vga_rgfx(regbase,VGA_GFX_MODE),vga_wgfx(regbase,VGA_GFX_MODE,(temp&VGA_GR05_RESERVED)|mode->vm_gfx_mode);
 temp = vga_rgfx(regbase,VGA_GFX_MISC),vga_wgfx(regbase,VGA_GFX_MISC,(temp&VGA_GR06_RESERVED)|mode->vm_gfx_misc);
 temp = vga_rgfx(regbase,VGA_GFX_COMPARE_MASK),vga_wgfx(regbase,VGA_GFX_COMPARE_MASK,(temp&VGA_GR07_RESERVED)|mode->vm_gfx_compare_mask);
 vga_wgfx(regbase,VGA_GFX_BIT_MASK,mode->vm_gfx_bit_mask);

 /* Apply new graphics settings. */
 vga_wcrt(regbase,VGA_CRTC_H_TOTAL,mode->vm_crt_h_total);
 vga_wcrt(regbase,VGA_CRTC_H_DISP,mode->vm_crt_h_disp);
 vga_wcrt(regbase,VGA_CRTC_H_BLANK_START,mode->vm_crt_h_blank_start);
 vga_wcrt(regbase,VGA_CRTC_H_BLANK_END,mode->vm_crt_h_blank_end);
 vga_wcrt(regbase,VGA_CRTC_H_SYNC_START,mode->vm_crt_h_sync_start);
 vga_wcrt(regbase,VGA_CRTC_H_SYNC_END,mode->vm_crt_h_sync_end);
 vga_wcrt(regbase,VGA_CRTC_V_TOTAL,mode->vm_crt_v_total);
 vga_wcrt(regbase,VGA_CRTC_OVERFLOW,mode->vm_crt_overflow);
 temp = vga_rcrt(regbase,VGA_CRTC_PRESET_ROW);
 vga_wcrt(regbase,VGA_CRTC_PRESET_ROW,(temp&VGA_CR8_RESERVED)|mode->vm_crt_preset_row);
 vga_wcrt(regbase,VGA_CRTC_MAX_SCAN,mode->vm_crt_max_scan);
 vga_wcrt(regbase,VGA_CRTC_V_SYNC_START,mode->vm_crt_v_sync_start);
 temp = vga_rcrt(regbase,VGA_CRTC_V_SYNC_END);
 vga_wcrt(regbase,VGA_CRTC_V_SYNC_END,(temp&VGA_CR11_RESERVED)|mode->vm_crt_v_sync_end);
 vga_wcrt(regbase,VGA_CRTC_V_DISP_END,mode->vm_crt_v_disp_end);
 vga_wcrt(regbase,VGA_CRTC_OFFSET,mode->vm_crt_offset);
 vga_wcrt(regbase,VGA_CRTC_UNDERLINE,mode->vm_crt_underline);
 vga_wcrt(regbase,VGA_CRTC_V_BLANK_START,mode->vm_crt_v_blank_start);
 temp = vga_rcrt(regbase,VGA_CRTC_V_BLANK_END);
 vga_wcrt(regbase,VGA_CRTC_V_BLANK_END,(temp&VGA_CR16_RESERVED)|mode->vm_crt_v_blank_end);
 temp = vga_rcrt(regbase,VGA_CRTC_MODE);
 vga_wcrt(regbase,VGA_CRTC_MODE,(temp&VGA_CR17_RESERVED)|mode->vm_crt_mode);
 vga_wcrt(regbase,VGA_CRTC_LINE_COMPARE,mode->vm_crt_line_compare);

 /* Turn the screen back on. */
 vga_wseq(regbase,VGA_SEQ_RESET,0x1);
 vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,(qr1&VGA_SR01_RESERVED)|mode->vm_seq_clock_mode);
 vga_wseq(regbase,VGA_SEQ_RESET,0x3);
 PREEMPTION_POP(was);
}

#define VGA_CMAP_SIZE 768
#define DATA_PLANE(i) i
#define DATA_PLANE0   0
#define DATA_PLANE1   1
#define DATA_PLANE2   2
#define DATA_PLANE3   3
#define DATA_CMAP     4

PRIVATE void KCALL
vga_setpal(MMIO void *regbase, struct vga_pal const *__restrict pal) {
 unsigned int i;
 byte_t *buffer = (byte_t *)&pal->vp_pal[0][0];
 vga_w(regbase,VGA_PEL_MSK,0xff);
 vga_w(regbase,VGA_PEL_IW,0x00);
 for (i = 0; i < VGA_CMAP_SIZE; ++i)
      vga_w(regbase,VGA_PEL_D,buffer[i]);
}

#endif


PUBLIC syscall_slong_t KCALL
kernel_control(syscall_ulong_t command, syscall_ulong_t arg0,
               syscall_ulong_t arg1, syscall_ulong_t arg2,
               syscall_ulong_t arg3, syscall_ulong_t arg4) {
 syscall_slong_t result = 0;
 switch (command) {

  /* Debug control commands. */
 case KERNEL_CONTROL_DBG_DUMP_LEAKS:
  result = mall_dump_leaks(__GFP_HEAPMASK);
  break;

 case KERNEL_CONTROL_DBG_CHECK_PADDING:
  mall_validate_padding(__GFP_HEAPMASK);
  break;

 case KERNEL_CONTROL_DBG_HEAP_VALIDATE:
  heap_validate_all();
  break;

 case KERNEL_CONTROL_TRACE_SYSCALLS_ON:
  enable_syscall_tracing();
  break;

 case KERNEL_CONTROL_TRACE_SYSCALLS_OFF:
  disable_syscall_tracing();
  break;

 case KERNEL_CONTROL_CLEARCACHES:
  /* Clear all caches by passing a callback that always returns `false' */
  kernel_cc_invoke(&test_fail,NULL);
  break;

 case KERNEL_CONTROL_INSMOD:
 case KERNEL_CONTROL_DELMOD:
 {
  struct regular_node *node;
  struct path *modpath;
  REF struct module *EXCEPT_VAR COMPILER_IGNORE_UNINITIALIZED(mod);
  REF struct driver *EXCEPT_VAR driver;
  /* Use the first argument to lookup a filesystem path. */
  modpath = fs_path(NULL,(char *)arg0,user_strlen((char *)arg0),
                   (struct inode **)&node,FS_MODE_FNORMAL);
  TRY {
   if (!INODE_ISREG(&node->re_node))
        error_throw(E_NOT_EXECUTABLE);
   /* Open a module under that path. */
   mod = module_open(node,modpath);
  } FINALLY {
   path_decref(modpath);
   inode_decref((struct inode *)node);
  }
  TRY {
   if (command == KERNEL_CONTROL_INSMOD) {
    bool was_newly_loaded;
    size_t commandline_length;
    commandline_length = arg1 ? user_strlen((char *)arg1) : 0;
    /* Fail if the user-provided commandline is too long. */
    if unlikely(commandline_length >= 0x4000)
       error_throw(E_INVALID_ARGUMENT);
    /* Load a new driver. */
    driver = kernel_insmod(mod,&was_newly_loaded,
                          (char *)arg1,
                           commandline_length);
    driver_decref(driver);
    COMPILER_BARRIER();
    /* Fail if the driver had already been loaded. */
    if unlikely(!was_newly_loaded)
       error_throw(E_INVALID_ARGUMENT);
   } else {
    /* Lookup a driver instance of the given module. */
    driver = kernel_getmod(mod);
    if unlikely(!driver)
       error_throw(E_INVALID_ARGUMENT);
    TRY {
     /* Delete this driver. */
     kernel_delmod(driver,true);
    } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
     driver_decref(driver);
     error_rethrow();
    }
   }
  } FINALLY {
   module_decref(mod);
  }
 } break;

#if 1
 /* Temporary, hidden command to enable video mode
  * until I get around to implement a new VGA driver. */
#define KERNEL_CONTROL_ENABLE_VGA 0x86000123
 case KERNEL_CONTROL_ENABLE_VGA:
  vga_setmode(NULL,&vm_gfx);
  vga_setpal(NULL,&vp_gfx_256);
  break;
#endif


 default:
  error_throw(E_INVALID_ARGUMENT);
  break;
 }
 return result;
}



DEFINE_SYSCALL6(xkernctl,
                syscall_ulong_t,command,syscall_ulong_t,arg0,
                syscall_ulong_t,arg1,syscall_ulong_t,arg2,
                syscall_ulong_t,arg3,syscall_ulong_t,arg4) {
 /* TODO: Check `CAP_SYS_ADMIN' */
 return kernel_control(command,arg0,arg1,arg2,arg3,arg4);
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_KERNEL_KERNCTL_C */
