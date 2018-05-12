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
#ifndef _KOS_I386_KOS_VGA_H
#define _KOS_I386_KOS_VGA_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>
#include <asm/ioctl.h>

__SYSDECL_BEGIN


/* Taken from linux, before changes were made. */

/*
 * linux/include/video/vga.h -- standard VGA chipset interaction
 *
 * Copyright 1999 Jeff Garzik <jgarzik@pobox.com>
 * 
 * Copyright history from vga16fb.c:
 *	Copyright 1999 Ben Pfaff and Petr Vandrovec
 *	Based on VGA info at http://www.osdever.net/FreeVGA/home.htm 
 *	Based on VESA framebuffer (c) 1998 Gerd Knorr
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.  
 *
 */

/* Some of the code below is taken from SVGAlib.  The original,
   unmodified copyright notice for that code is below. */
/* VGAlib version 1.2 - (c) 1993 Tommy Frandsen                    */
/*                                                                 */
/* This library is free software; you can redistribute it and/or   */
/* modify it without any restrictions. This library is distributed */
/* in the hope that it will be useful, but without any warranty.   */

/* Multi-chipset support Copyright 1993 Harm Hanemaayer */
/* partially copyrighted (C) 1993 by Hartmut Schirmer */

/* VGA data register ports */
#define VGA_CRT_DC      0x3d5 /* CRT Controller Data Register - color emulation. */
#define VGA_CRT_DM      0x3b5 /* CRT Controller Data Register - mono emulation. */
#define VGA_ATT_R       0x3c1 /* Attribute Controller Data Read Register. */
#define VGA_ATT_W       0x3c0 /* Attribute Controller Data Write Register. */
#define VGA_GFX_D       0x3cf /* Graphics Controller Data Register. */
#define VGA_SEQ_D       0x3c5 /* Sequencer Data Register. */
#define VGA_MIS_R       0x3cc /* Misc Output Read Register. */
#define VGA_MIS_W       0x3c2 /* Misc Output Write Register. */
#   define VGA_MIS_FRESERVED         0x10 /* Mask of reserved registers (Added by GrieferAtWork). */
#   define VGA_MIS_FCOLOR            0x01
#   define VGA_MIS_FENB_MEM_ACCESS   0x02 /* Enable system access to the display buffer. */
#   define VGA_MIS_FCLOCK_25175_640  0x00 /* 25 Mhz clock -- for 320/640 pixel wide modes */
#   define VGA_MIS_FCLOCK_28322_720  0x04 /* 28 Mhz clock -- for 360/720 pixel wide modes */
#   define VGA_MIS_FCLOCKMASK        0x0c /* Mask for the clock selection. */
#   define VGA_MIS_FSEL_HIGH_PAGE    0x20 /* Select high memory when configued for even/odd mode */
#   define VGA_MIS_FHSYNCPOL         0x40 /* HSYNC polarity (0 = positive horizontal retrace sync pulse) */
#   define VGA_MIS_FVSYNCPOL         0x80 /* VSYNC polarity (0 = positive vertical retrace sync pulse) */
#define VGA_FTC_R       0x3ca /* Feature Control Read Register. */
#define VGA_IS1_RC      0x3da /* Input Status Register 1 - color emulation. */
#define VGA_IS1_RM      0x3ba /* Input Status Register 1 - mono emulation. */
#define VGA_PEL_D       0x3c9 /* PEL Data Register. */
#define VGA_PEL_MSK     0x3c6 /* PEL mask register. */

/* EGA-specific registers */
#define EGA_GFX_E0      0x3cc /* Graphics enable processor 0. */
#define EGA_GFX_E1      0x3ca /* Graphics enable processor 1. */

/* VGA index register ports */
#define VGA_CRT_IC      0x3d4 /* CRT Controller Index - color emulation. */
#define VGA_CRT_IM      0x3b4 /* CRT Controller Index - mono emulation. */
#define VGA_ATT_IW      0x3c0 /* Attribute Controller Index & Data Write Register. */
#define VGA_GFX_I       0x3ce /* Graphics Controller Index. */
#define VGA_SEQ_I       0x3c4 /* Sequencer Index. */
#define VGA_PEL_IW      0x3c8 /* PEL Write Index. */
#define VGA_PEL_IR      0x3c7 /* PEL Read Index. */

/* standard VGA indexes max counts */
#define VGA_CRT_C       0x19  /* Number of CRT Controller Registers. */
#define VGA_ATT_C       0x15  /* Number of Attribute Controller Registers. */
#define VGA_GFX_C       0x09  /* Number of Graphics Controller Registers. */
#define VGA_SEQ_C       0x05  /* Number of Sequencer Registers. */
#define VGA_MIS_C       0x01  /* Number of Misc Output Register. */

/* VGA CRT controller register indices */
#define VGA_CRTC_H_TOTAL       0
#define VGA_CRTC_H_DISP        1
#define VGA_CRTC_H_BLANK_START 2
#define VGA_CRTC_H_BLANK_END   3
#   define VGA_CR3_FMASK            0x1f /* Mask of bits used for hblank-end (Added by GrieferAtWork) */
#   define VGA_CR3_FALWAYS1         0x80 /* Always set this bit when writing this register (backward compatibility) */
#define VGA_CRTC_H_SYNC_START  4
#define VGA_CRTC_H_SYNC_END    5
#   define VGA_CR5_FMASK            0x1f /* Mask of bits used for hsync-end (Added by GrieferAtWork) */
#   define VGA_CR5_FH_BLANK_END_5   0x80 /* 5th bit for `VGA_CRTC_H_BLANK_END' (Added by GrieferAtWork) */
#define VGA_CRTC_V_TOTAL       6
#define VGA_CRTC_OVERFLOW      7
#   define VGA_CR7_FV_TOTAL_8       0x01 /* 8th bit for `VGA_CRTC_V_TOTAL' (Added by GrieferAtWork) */
#   define VGA_CR7_FV_DISP_END_8    0x02 /* 8th bit for `VGA_CRTC_V_DISP_END' (Added by GrieferAtWork) */
#   define VGA_CR7_FV_SYNC_START_8  0x04 /* 8th bit for `VGA_CRTC_V_SYNC_START' (Added by GrieferAtWork) */
#   define VGA_CR7_FV_BLANK_START_8 0x08 /* 8th bit for `VGA_CRTC_V_BLANK_START' (Added by GrieferAtWork) */
#   define VGA_CR7_FV_LINECOMP_8    0x10 /* 8th bit for `VGA_CRTC_LINE_COMPARE' (Added by GrieferAtWork) */
#   define VGA_CR7_FV_TOTAL_9       0x20 /* 9th bit for `VGA_CRTC_V_TOTAL' (Added by GrieferAtWork) */
#   define VGA_CR7_FV_DISP_END_9    0x40 /* 9th bit for `VGA_CRTC_V_DISP_END' (Added by GrieferAtWork) */
#   define VGA_CR7_FV_SYNC_START_9  0x80 /* 9th bit for `VGA_CRTC_V_SYNC_START' (Added by GrieferAtWork) */
#define VGA_CRTC_PRESET_ROW    8
#   define VGA_CR8_FRESERVED        0x80 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_CRTC_MAX_SCAN      9
#   define VGA_CR9_FMASK            0x1f /* Mask of bits used for max-scan (Added by GrieferAtWork) */
#   define VGA_CR9_FV_BLANK_START_9 0x20 /* 9th bit for `VGA_CRTC_V_BLANK_START' (Added by GrieferAtWork) */
#   define VGA_CR9_FV_LINECOMP_9    0x40 /* 9th bit for `VGA_CRTC_LINE_COMPARE' (Added by GrieferAtWork) */
#   define VGA_CR9_FSCANDOUBLE      0x80 /* Better don't set... (Don't really understand what this done) (Added by GrieferAtWork) */
#define VGA_CRTC_CURSOR_START  0x0a
#   define VGA_CRTC_FCURSOR_DISABLE 0x20 /* Disable the text-mode cursor (Added by GrieferAtWork) */
#define VGA_CRTC_CURSOR_END    0x0b
#define VGA_CRTC_START_HI      0x0c
#define VGA_CRTC_START_LO      0x0d
#define VGA_CRTC_CURSOR_HI     0x0e
#define VGA_CRTC_CURSOR_LO     0x0f
#define VGA_CRTC_V_SYNC_START  0x10
#define VGA_CRTC_V_SYNC_END    0x11
#   define VGA_CR11_FRESERVED     0x30 /* Mask of reserved registers (Added by GrieferAtWork). */
#   define VGA_CR11_FMASK         0x0f /* Mask of bits used for vsync-end (Added by GrieferAtWork) */
#   define VGA_CR11_FLOCK_CR0_CR7 0x80 /* lock writes to CR0 - CR7. */
#define VGA_CRTC_V_DISP_END    0x12
#define VGA_CRTC_OFFSET        0x13
#define VGA_CRTC_UNDERLINE     0x14
#define VGA_CRTC_V_BLANK_START 0x15
#define VGA_CRTC_V_BLANK_END   0x16
#   define VGA_CR16_FRESERVED     0x80 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_CRTC_MODE          0x17
#   define VGA_CR17_FRESERVED     0x10 /* Mask of reserved registers (Added by GrieferAtWork). */
#   define VGA_CR17_FH_V_SIGNALS_ENABLED 0x80
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
#   define VGA_AT10_FRESERVED      0x10 /* Mask of reserved registers (Added by GrieferAtWork). */
#   define VGA_AT10_FGRAPHICS      0x01 /* Enable graphics, rather than alphanumeric mode (Added by GrieferAtWork). */
#   define VGA_AT10_FDUP9          0x04 /* Duplicate the 8`th text dot into the 9`th when `VGA_SR01_CHAR_CLK_8DOTS' isn't set, instead of filling it with background (Added by GrieferAtWork). */
#   define VGA_AT10_FBLINK         0x08 /* Set to cause character attribute bit #7 to be used for blinking text;
                                         * NEVER SET THIS! YOU'LL CAUSE SEIZURES IN PEOPLE (Added by GrieferAtWork).
                                         * WARNING: After boot, a BIOS mode switch, or a video card reset, this flag
                                         *          is enabled and _MUST_ under all circumstances be disabled again! */
#   define VGA_AT10_F8BITPAL       0x40 /* 8-bit palette index (Added by GrieferAtWork). */
#define VGA_ATC_OVERSCAN       0x11
#define VGA_ATC_PLANE_ENABLE   0x12
#   define VGA_AT12_FMASK          0x0f /* Mask of planes (Added by GrieferAtWork). */
#   define VGA_AT12_FRESERVED      0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_ATC_PEL            0x13
#   define VGA_AT13_FMASK          0x0f /* Mask for pixel panning (Added by GrieferAtWork). */
#   define VGA_AT13_FRESERVED      0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_ATC_COLOR_PAGE     0x14
#   define VGA_AT14_FRESERVED      0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */

#define VGA_AR_ENABLE_DISPLAY  0x20

/* VGA sequencer register indices */
#define VGA_SEQ_RESET          0x00
#define VGA_SEQ_CLOCK_MODE     0x01
#   define VGA_SR01_FCHAR_CLK_8DOTS 0x01 /* bit 0: character clocks 8 dots wide are generated */
#   define VGA_SR01_FSCREEN_OFF     0x20 /* bit 5: Screen is off
                                          * NOTE: The `VGA_SETMODE' ioctl ignores this bit! */
#   define VGA_SR01_FRESERVED       0xc2 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_SEQ_PLANE_WRITE    0x02
#   define VGA_SR02_FPLANE(i)     ((1) << i) /* Added by GrieferAtWork */
#   define VGA_SR02_FALL_PLANES     0x0f /* bits 3-0: enable access to all planes */
#   define VGA_SR02_FRESERVED       0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_SEQ_CHARACTER_MAP  0x03
#   define VGA_SR03_FRESERVED       0xc0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_SEQ_MEMORY_MODE    0x04
#   define VGA_SR04_FEXT_MEM        0x02 /* bit 1: allows complete mem access to 256K */
#   define VGA_SR04_FSEQ_MODE       0x04 /* bit 2: directs system to use a sequential addressing mode */
#   define VGA_SR04_FCHN_4M         0x08 /* bit 3: selects modulo 4 addressing for CPU access to display memory */
#   define VGA_SR04_FRESERVED       0xf1 /* Mask of reserved registers (Added by GrieferAtWork). */

/* VGA graphics controller register indices */
#define VGA_GFX_SR_VALUE        0x00
#   define VGA_GR00_FRESERVED       0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_SR_ENABLE       0x01
#   define VGA_GR01_FRESERVED       0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_COMPARE_VALUE   0x02
#   define VGA_GR02_FRESERVED       0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_DATA_ROTATE     0x03
#   define VGA_GR03_FRESERVED       0xe0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_PLANE_READ      0x04
#   define VGA_GR04_FRESERVED       0xfc /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_MODE            0x05
#   define VGA_GR05_FRESERVED       0x84 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_MISC            0x06
#   define VGA_GR06_FRESERVED       0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#   define VGA_GR06_FGRAPHICS_MODE  0x01
#define VGA_GFX_COMPARE_MASK    0x07
#   define VGA_GR07_FRESERVED       0xf0 /* Mask of reserved registers (Added by GrieferAtWork). */
#define VGA_GFX_BIT_MASK        0x08
/* === END OF LINUX-DERIVED CONTENT === */




struct __ATTR_PACKED vga_color {
    /* NOTE: When loading/saving the color palette, the kernel
     *       will automatically up/down-scale the color intensity
     *       in order to clamp it to VGA's limit of `0x3f'.
     *       With that in mind, to get `0x3f', you should pass `0xff'! */
    __UINT8_TYPE__   c_red;       /* Red color component. */
    __UINT8_TYPE__   c_green;     /* Green color component. */
    __UINT8_TYPE__   c_blue;      /* Blue color component. */
};

struct __ATTR_PACKED vga_palette {
    struct vga_color vp_pal[256]; /* VGA color palette. */
};


struct __ATTR_PACKED vga_font {
    __BYTE_TYPE__    vf_blob[32][256]; /* Font data blob. */
};

struct __ATTR_PACKED vga_mode {
    __UINT8_TYPE__ vm_att_mode;          /* VGA_ATC_MODE. */
    __UINT8_TYPE__ vm_att_overscan;      /* VGA_ATC_OVERSCAN. */
    __UINT8_TYPE__ vm_att_plane_enable;  /* VGA_ATC_PLANE_ENABLE. */
    __UINT8_TYPE__ vm_att_pel;           /* VGA_ATC_PEL. */
    __UINT8_TYPE__ vm_att_color_page;    /* VGA_ATC_COLOR_PAGE. */
    __UINT8_TYPE__ vm_mis;               /* VGA_MIS_R / VGA_MIS_W. */
    __UINT8_TYPE__ vm_gfx_sr_value;      /* VGA_GFX_SR_VALUE. */
    __UINT8_TYPE__ vm_gfx_sr_enable;     /* VGA_GFX_SR_ENABLE. */
    __UINT8_TYPE__ vm_gfx_compare_value; /* VGA_GFX_COMPARE_VALUE. */
    __UINT8_TYPE__ vm_gfx_data_rotate;   /* VGA_GFX_DATA_ROTATE. */
    __UINT8_TYPE__ vm_gfx_mode;          /* VGA_GFX_MODE. */
    __UINT8_TYPE__ vm_gfx_misc;          /* VGA_GFX_MISC. */
    __UINT8_TYPE__ vm_gfx_compare_mask;  /* VGA_GFX_COMPARE_MASK. */
    __UINT8_TYPE__ vm_gfx_bit_mask;      /* VGA_GFX_BIT_MASK. */
    __UINT8_TYPE__ vm_crt_h_total;       /* VGA_CRTC_H_TOTAL. */
    __UINT8_TYPE__ vm_crt_h_disp;        /* VGA_CRTC_H_DISP. */
    __UINT8_TYPE__ vm_crt_h_blank_start; /* VGA_CRTC_H_BLANK_START. */
    __UINT8_TYPE__ vm_crt_h_blank_end;   /* VGA_CRTC_H_BLANK_END. */
    __UINT8_TYPE__ vm_crt_h_sync_start;  /* VGA_CRTC_H_SYNC_START. */
    __UINT8_TYPE__ vm_crt_h_sync_end;    /* VGA_CRTC_H_SYNC_END. */
    __UINT8_TYPE__ vm_crt_v_total;       /* VGA_CRTC_V_TOTAL. */
    __UINT8_TYPE__ vm_crt_overflow;      /* VGA_CRTC_OVERFLOW. */
    __UINT8_TYPE__ vm_crt_preset_row;    /* VGA_CRTC_PRESET_ROW. */
    __UINT8_TYPE__ vm_crt_max_scan;      /* VGA_CRTC_MAX_SCAN. */
    __UINT8_TYPE__ vm_crt_v_sync_start;  /* VGA_CRTC_V_SYNC_START. */
    __UINT8_TYPE__ vm_crt_v_sync_end;    /* VGA_CRTC_V_SYNC_END. */
    __UINT8_TYPE__ vm_crt_v_disp_end;    /* VGA_CRTC_V_DISP_END. */
    __UINT8_TYPE__ vm_crt_offset;        /* VGA_CRTC_OFFSET. */
    __UINT8_TYPE__ vm_crt_underline;     /* VGA_CRTC_UNDERLINE. */
    __UINT8_TYPE__ vm_crt_v_blank_start; /* VGA_CRTC_V_BLANK_START. */
    __UINT8_TYPE__ vm_crt_v_blank_end;   /* VGA_CRTC_V_BLANK_END. */
    __UINT8_TYPE__ vm_crt_mode;          /* VGA_CRTC_MODE. */
    __UINT8_TYPE__ vm_crt_line_compare;  /* VGA_CRTC_LINE_COMPARE. */
    __UINT8_TYPE__ vm_seq_clock_mode;    /* VGA_SEQ_CLOCK_MODE. */
    __UINT8_TYPE__ vm_seq_plane_write;   /* VGA_SEQ_PLANE_WRITE. */
    __UINT8_TYPE__ vm_seq_character_map; /* VGA_SEQ_CHARACTER_MAP. */
    __UINT8_TYPE__ vm_seq_memory_mode;   /* VGA_SEQ_MEMORY_MODE. */
};




/* VGA ioctl() commands (for /dev/vga) */
#define VGA_RESET     _IOW('K',128,unsigned int) /* Reset the VGA adapter (arg: Set of `VGA_RESET_F*') */
#   define VGA_RESET_FOFF   0x0001 /* Turn the screen off */
#   define VGA_RESET_FMODE  0x0010 /* Reset the display mode to 80x25 color text mode (BIOS 3h) */
#   define VGA_RESET_FFONT  0x0020 /* Reset the display font to what the bios originally pre-set. */
#   define VGA_RESET_FPAL   0x0040 /* Reset the display palette to what the bios originally pre-set. */
#   define VGA_RESET_FON    0x8000 /* Turn the screen on */
#define VGA_GETMODE   _IOR('K',129,struct vga_mode)          /* Get the current display mode. */
#define VGA_GETMODE_DEFAULT _IOR('K',128,struct vga_mode)    /* Get the default (bios) mode. */
#define VGA_SETMODE   _IOW('K',129,struct vga_mode)          /* Set the current display mode. */
#define VGA_SETMODE_DEFAULT _IOW('K',129,unsigned int)       /* Set the current display mode to one of the default modes (arg: One of `VGA_DEFAULT_MODE_*'). */
#   define VGA_DEFAULT_MODE_TXT80X25_16    0x0000            /* Set 80x25 16-color text mode. */
#   define VGA_DEFAULT_MODE_GFX320X200_256 0x0001            /* Set 320x200 256-color graphics mode. */
#   define VGA_DEFAULT_MODE_GFX640X480_16  0x0002            /* Set 640x480 16-color graphics mode. */
#define VGA_GETPAL    _IOR('K',130,struct vga_palette)       /* Get the current palette. */
#define VGA_GETPAL_DEFAULT  _IOR('K',128,struct vga_palette) /* Get the default (bios) palette. */
#define VGA_SETPAL    _IOW('K',130,struct vga_palette)       /* Set the current palette. */
#define VGA_SETPAL_DEFAULT  _IOW('K',130,unsigned int)       /* Set the current palette to one of the default palettes (arg: One of `VGA_DEFAULT_PAL_*'). */
#   define VGA_DEFAULT_PAL_TXT16           0x0000            /* Set 16-color text mode palette. */
#   define VGA_DEFAULT_PAL_GFX16           0x0001            /* Set 16-color graphics mode palette. */
#   define VGA_DEFAULT_PAL_GFX256          0x0002            /* Set 256-color graphics mode palette.
                                                              * See the palette here: http://www.fountainware.com/EXPL/vga_color_palettes.htm */
#define VGA_GETFONT   _IOR('K',131,struct vga_font)          /* Get the current font.
                                                              * NOTE: If the given buffer is too small, the call still succeeds,
                                                              *       but `vf_height' is always updated to the required character
                                                              *       height, which can then be re-used to calculated the required
                                                              *       buffer size before repeating the call. */
#define VGA_GETFONT_DEFAULT _IOR('K',128,struct vga_font)    /* Get the default (bios) font. */
#define VGA_SETFONT   _IOW('K',131,struct vga_font)          /* Set the current font. */
#define VGA_SETFONT_DEFAULT _IOR('K',131,unsigned int)       /* Get the current font to one of the default fonts (arg: One of `VGA_DEFAULT_FONT_*'). */
#   define VGA_DEFAULT_FONT_BIOS           0x0000            /* Restore the bios-predefined default VGA font. */



__SYSDECL_END

#endif /* !_KOS_I386_KOS_VGA_H */
