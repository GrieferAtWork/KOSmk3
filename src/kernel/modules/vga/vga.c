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
#ifndef GUARD_KERNEL_MODULES_VGA_VGA_C
#define GUARD_KERNEL_MODULES_VGA_VGA_C 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kos/types.h>
#include <kos/kdev_t.h>
#include <kernel/bind.h>
#include <kernel/user.h>
#include <string.h>
#include <stdio.h>
#include <except.h>

#include "vga.h"

DECL_BEGIN


INTERN struct vga_mode const vga_biosmode = {
    /* BAD! DON'T YOU DARE TO BLINK!
     * Like literally, dis shit is dangerous to look at
     * (Sorry, but even though I never actually had a seizure
     *  from stupid $h1t like this, every time I see this blinking,
     *  I can just feel that if I were to stare at it for too long,
     *  I'd either throw up, or literally just die)
     * Especially since I use the intensity attribute when the kernel
     * panics, meaning that while this is still enabled, it'll blink
     * when I was sitting here knowing that I had to look at it to
     * figure out what happened.
     * And I know I could have simply used regular colors at any point,
     * but before starting this module and reading up on VGA I didn't
     * see the connection and didn't understand why text was sometimes
     * blinking.
     * (And wiki.osdev's VGA TTY page neglects to mention the seizure-
     *  inducing blinkyness that happens on real hardware and emulators
     *  ~supporting~ the VgA sTaNdArT's GrEaT iDeA oF iNcLuDiNg ThIs FeAtUrE) */
    .vm_att_mode          = 0x0c & ~(VGA_AT10_BLINK),
    .vm_att_overscan      = 0x00,
    .vm_att_plane_enable  = 0x0f,
    .vm_att_pel           = 0x08,
    .vm_att_color_page    = 0x00,
    .vm_mis               = 0xe3,
    .vm_gfx_sr_value      = 0x00,
    .vm_gfx_sr_enable     = 0x00,
    .vm_gfx_compare_value = 0x00,
    .vm_gfx_data_rotate   = 0x00,
    .vm_gfx_mode          = 0x10,
    .vm_gfx_misc          = 0x0e,
    .vm_gfx_compare_mask  = 0x0f,
    .vm_gfx_bit_mask      = 0xff,
    .vm_crt_h_total       = 0x5f,
    .vm_crt_h_disp        = 0x4f,
    .vm_crt_h_blank_start = 0x50,
    .vm_crt_h_blank_end   = 0x82,
    .vm_crt_h_sync_start  = 0x55,
    .vm_crt_h_sync_end    = 0x81,
    .vm_crt_v_total       = 0xbf,
    .vm_crt_overflow      = 0x1f,
    .vm_crt_preset_row    = 0x00,
    .vm_crt_max_scan      = 0x4f,
    .vm_crt_v_sync_start  = 0x9c,
    .vm_crt_v_sync_end    = 0x8e,
    .vm_crt_v_disp_end    = 0x8f,
    .vm_crt_offset        = 0x28,
    .vm_crt_underline     = 0x1f,
    .vm_crt_v_blank_start = 0x96,
    .vm_crt_v_blank_end   = 0xb9,
    .vm_crt_mode          = 0xa3,
    .vm_crt_line_compare  = 0xff,
    .vm_seq_clock_mode    = 0x00,
    .vm_seq_plane_write   = 0x03,
    .vm_seq_character_map = 0x00,
    .vm_seq_memory_mode   = 0x02,
};

INTERN struct vga_mode const vga_mode_gfx320x200_256 = {
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


INTERN struct vga_palette const vga_biospal = {
    {   {0x00,0x00,0x00},{0x00,0x00,0x2a},{0x00,0x2a,0x00},{0x00,0x2a,0x2a},{0x2a,0x00,0x00},{0x2a,0x00,0x2a},{0x2a,0x2a,0x00},{0x2a,0x2a,0x2a},
        {0x00,0x00,0x15},{0x00,0x00,0x3f},{0x00,0x2a,0x15},{0x00,0x2a,0x3f},{0x2a,0x00,0x15},{0x2a,0x00,0x3f},{0x2a,0x2a,0x15},{0x2a,0x2a,0x3f},
        {0x00,0x15,0x00},{0x00,0x15,0x2a},{0x00,0x3f,0x00},{0x00,0x3f,0x2a},{0x2a,0x15,0x00},{0x2a,0x15,0x2a},{0x2a,0x3f,0x00},{0x2a,0x3f,0x2a},
        {0x00,0x15,0x15},{0x00,0x15,0x3f},{0x00,0x3f,0x15},{0x00,0x3f,0x3f},{0x2a,0x15,0x15},{0x2a,0x15,0x3f},{0x2a,0x3f,0x15},{0x2a,0x3f,0x3f},
        {0x15,0x00,0x00},{0x15,0x00,0x2a},{0x15,0x2a,0x00},{0x15,0x2a,0x2a},{0x3f,0x00,0x00},{0x3f,0x00,0x2a},{0x3f,0x2a,0x00},{0x3f,0x2a,0x2a},
        {0x15,0x00,0x15},{0x15,0x00,0x3f},{0x15,0x2a,0x15},{0x15,0x2a,0x3f},{0x3f,0x00,0x15},{0x3f,0x00,0x3f},{0x3f,0x2a,0x15},{0x3f,0x2a,0x3f},
        {0x15,0x15,0x00},{0x15,0x15,0x2a},{0x15,0x3f,0x00},{0x15,0x3f,0x2a},{0x3f,0x15,0x00},{0x3f,0x15,0x2a},{0x3f,0x3f,0x00},{0x3f,0x3f,0x2a},
        {0x15,0x15,0x15},{0x15,0x15,0x3f},{0x15,0x3f,0x15},{0x15,0x3f,0x3f},{0x3f,0x15,0x15},{0x3f,0x15,0x3f},{0x3f,0x3f,0x15},{0x3f,0x3f,0x3f},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},
    }
};

INTERN struct vga_palette const vga_pal_gfx256 = {
#define C(r,g,b) {r<<2,g<<2,b<<2}
    {   C(0x00,0x00,0x00),C(0x00,0x00,0x2a),C(0x00,0x2a,0x00),C(0x00,0x2a,0x2a),C(0x2a,0x00,0x00),C(0x2a,0x00,0x2a),C(0x2a,0x15,0x00),C(0x2a,0x2a,0x2a),
        C(0x15,0x15,0x15),C(0x15,0x15,0x3f),C(0x15,0x3f,0x15),C(0x15,0x3f,0x3f),C(0x3f,0x15,0x15),C(0x3f,0x15,0x3f),C(0x3f,0x3f,0x15),C(0x3f,0x3f,0x3f),
        C(0x00,0x00,0x00),C(0x05,0x05,0x05),C(0x08,0x08,0x08),C(0x0b,0x0b,0x0b),C(0x0e,0x0e,0x0e),C(0x11,0x11,0x11),C(0x14,0x14,0x14),C(0x18,0x18,0x18),
        C(0x1c,0x1c,0x1c),C(0x20,0x20,0x20),C(0x24,0x24,0x24),C(0x28,0x28,0x28),C(0x2d,0x2d,0x2d),C(0x32,0x32,0x32),C(0x38,0x38,0x38),C(0x3f,0x3f,0x3f),
        C(0x00,0x00,0x3f),C(0x10,0x00,0x3f),C(0x1f,0x00,0x3f),C(0x2f,0x00,0x3f),C(0x3f,0x00,0x3f),C(0x3f,0x00,0x2f),C(0x3f,0x00,0x1f),C(0x3f,0x00,0x10),
        C(0x3f,0x00,0x00),C(0x3f,0x10,0x00),C(0x3f,0x1f,0x00),C(0x3f,0x2f,0x00),C(0x3f,0x3f,0x00),C(0x2f,0x3f,0x00),C(0x1f,0x3f,0x00),C(0x10,0x3f,0x00),
        C(0x00,0x3f,0x00),C(0x00,0x3f,0x10),C(0x00,0x3f,0x1f),C(0x00,0x3f,0x2f),C(0x00,0x3f,0x3f),C(0x00,0x2f,0x3f),C(0x00,0x1f,0x3f),C(0x00,0x10,0x3f),
        C(0x1f,0x1f,0x3f),C(0x27,0x1f,0x3f),C(0x2f,0x1f,0x3f),C(0x37,0x1f,0x3f),C(0x3f,0x1f,0x3f),C(0x3f,0x1f,0x37),C(0x3f,0x1f,0x2f),C(0x3f,0x1f,0x27),
        C(0x3f,0x1f,0x1f),C(0x3f,0x27,0x1f),C(0x3f,0x2f,0x1f),C(0x3f,0x37,0x1f),C(0x3f,0x3f,0x1f),C(0x37,0x3f,0x1f),C(0x2f,0x3f,0x1f),C(0x27,0x3f,0x1f),
        C(0x1f,0x3f,0x1f),C(0x1f,0x3f,0x27),C(0x1f,0x3f,0x2f),C(0x1f,0x3f,0x37),C(0x1f,0x3f,0x3f),C(0x1f,0x37,0x3f),C(0x1f,0x2f,0x3f),C(0x1f,0x27,0x3f),
        C(0x2d,0x2d,0x3f),C(0x31,0x2d,0x3f),C(0x36,0x2d,0x3f),C(0x3a,0x2d,0x3f),C(0x3f,0x2d,0x3f),C(0x3f,0x2d,0x3a),C(0x3f,0x2d,0x36),C(0x3f,0x2d,0x31),
        C(0x3f,0x2d,0x2d),C(0x3f,0x31,0x2d),C(0x3f,0x36,0x2d),C(0x3f,0x3a,0x2d),C(0x3f,0x3f,0x2d),C(0x3a,0x3f,0x2d),C(0x36,0x3f,0x2d),C(0x31,0x3f,0x2d),
        C(0x2d,0x3f,0x2d),C(0x2d,0x3f,0x31),C(0x2d,0x3f,0x36),C(0x2d,0x3f,0x3a),C(0x2d,0x3f,0x3f),C(0x2d,0x3a,0x3f),C(0x2d,0x36,0x3f),C(0x2d,0x31,0x3f),
        C(0x00,0x00,0x1c),C(0x07,0x00,0x1c),C(0x0e,0x00,0x1c),C(0x15,0x00,0x1c),C(0x1c,0x00,0x1c),C(0x1c,0x00,0x15),C(0x1c,0x00,0x0e),C(0x1c,0x00,0x07),
        C(0x1c,0x00,0x00),C(0x1c,0x07,0x00),C(0x1c,0x0e,0x00),C(0x1c,0x15,0x00),C(0x1c,0x1c,0x00),C(0x15,0x1c,0x00),C(0x0e,0x1c,0x00),C(0x07,0x1c,0x00),
        C(0x00,0x1c,0x00),C(0x00,0x1c,0x07),C(0x00,0x1c,0x0e),C(0x00,0x1c,0x15),C(0x00,0x1c,0x1c),C(0x00,0x15,0x1c),C(0x00,0x0e,0x1c),C(0x00,0x07,0x1c),
        C(0x0e,0x0e,0x1c),C(0x11,0x0e,0x1c),C(0x15,0x0e,0x1c),C(0x18,0x0e,0x1c),C(0x1c,0x0e,0x1c),C(0x1c,0x0e,0x18),C(0x1c,0x0e,0x15),C(0x1c,0x0e,0x11),
        C(0x1c,0x0e,0x0e),C(0x1c,0x11,0x0e),C(0x1c,0x15,0x0e),C(0x1c,0x18,0x0e),C(0x1c,0x1c,0x0e),C(0x18,0x1c,0x0e),C(0x15,0x1c,0x0e),C(0x11,0x1c,0x0e),
        C(0x0e,0x1c,0x0e),C(0x0e,0x1c,0x11),C(0x0e,0x1c,0x15),C(0x0e,0x1c,0x18),C(0x0e,0x1c,0x1c),C(0x0e,0x18,0x1c),C(0x0e,0x15,0x1c),C(0x0e,0x11,0x1c),
        C(0x14,0x14,0x1c),C(0x16,0x14,0x1c),C(0x18,0x14,0x1c),C(0x1a,0x14,0x1c),C(0x1c,0x14,0x1c),C(0x1c,0x14,0x1a),C(0x1c,0x14,0x18),C(0x1c,0x14,0x16),
        C(0x1c,0x14,0x14),C(0x1c,0x16,0x14),C(0x1c,0x18,0x14),C(0x1c,0x1a,0x14),C(0x1c,0x1c,0x14),C(0x1a,0x1c,0x14),C(0x18,0x1c,0x14),C(0x16,0x1c,0x14),
        C(0x14,0x1c,0x14),C(0x14,0x1c,0x16),C(0x14,0x1c,0x18),C(0x14,0x1c,0x1a),C(0x14,0x1c,0x1c),C(0x14,0x1a,0x1c),C(0x14,0x18,0x1c),C(0x14,0x16,0x1c),
        C(0x00,0x00,0x10),C(0x04,0x00,0x10),C(0x08,0x00,0x10),C(0x0c,0x00,0x10),C(0x10,0x00,0x10),C(0x10,0x00,0x0c),C(0x10,0x00,0x08),C(0x10,0x00,0x04),
        C(0x10,0x00,0x00),C(0x10,0x04,0x00),C(0x10,0x08,0x00),C(0x10,0x0c,0x00),C(0x10,0x10,0x00),C(0x0c,0x10,0x00),C(0x08,0x10,0x00),C(0x04,0x10,0x00),
        C(0x00,0x10,0x00),C(0x00,0x10,0x04),C(0x00,0x10,0x08),C(0x00,0x10,0x0c),C(0x00,0x10,0x10),C(0x00,0x0c,0x10),C(0x00,0x08,0x10),C(0x00,0x04,0x10),
        C(0x08,0x08,0x10),C(0x0a,0x08,0x10),C(0x0c,0x08,0x10),C(0x0e,0x08,0x10),C(0x10,0x08,0x10),C(0x10,0x08,0x0e),C(0x10,0x08,0x0c),C(0x10,0x08,0x0a),
        C(0x10,0x08,0x08),C(0x10,0x0a,0x08),C(0x10,0x0c,0x08),C(0x10,0x0e,0x08),C(0x10,0x10,0x08),C(0x0e,0x10,0x08),C(0x0c,0x10,0x08),C(0x0a,0x10,0x08),
        C(0x08,0x10,0x08),C(0x08,0x10,0x0a),C(0x08,0x10,0x0c),C(0x08,0x10,0x0e),C(0x08,0x10,0x10),C(0x08,0x0e,0x10),C(0x08,0x0c,0x10),C(0x08,0x0a,0x10),
        C(0x0b,0x0b,0x10),C(0x0c,0x0b,0x10),C(0x0d,0x0b,0x10),C(0x0f,0x0b,0x10),C(0x10,0x0b,0x10),C(0x10,0x0b,0x0f),C(0x10,0x0b,0x0d),C(0x10,0x0b,0x0c),
        C(0x10,0x0b,0x0b),C(0x10,0x0c,0x0b),C(0x10,0x0d,0x0b),C(0x10,0x0f,0x0b),C(0x10,0x10,0x0b),C(0x0f,0x10,0x0b),C(0x0d,0x10,0x0b),C(0x0c,0x10,0x0b),
        C(0x0b,0x10,0x0b),C(0x0b,0x10,0x0c),C(0x0b,0x10,0x0d),C(0x0b,0x10,0x0f),C(0x0b,0x10,0x10),C(0x0b,0x0f,0x10),C(0x0b,0x0d,0x10),C(0x0b,0x0c,0x10),
        C(0x00,0x00,0x00),C(0x00,0x00,0x00),C(0x00,0x00,0x00),C(0x00,0x00,0x00),C(0x00,0x00,0x00),C(0x00,0x00,0x00),C(0x00,0x00,0x00),C(0x00,0x00,0x00),
#undef  C
    }
};


INTERN void KCALL
VGA_SetMode(VGA *__restrict self,
            struct vga_mode const *__restrict mode) {
 u8 temp,qr1;
 void *regbase = self->v_mmio;
 atomic_rwlock_write(&self->v_lock);
#if 1
 /* Disable preemption to prevent interference. */
 pflag_t was = PREEMPTION_PUSHOFF();
 qr1 = vga_rseq(regbase,VGA_SEQ_CLOCK_MODE);

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
#else
 qr1 = vga_rseq(regbase,VGA_SEQ_CLOCK_MODE);

 /* Validate the given mode. */
 /* TODO: Stuff like this: assert(!(mode->vm_att_mode&0x10)); */

 /* Turn off the screen. */
 if (!(self->v_state & VGA_STATE_FSCREENOFF)) {
  vga_wseq(regbase,VGA_SEQ_RESET,0x1);
  vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1|VGA_SR01_SCREEN_OFF);
  vga_wseq(regbase,VGA_SEQ_RESET,0x3);
 }

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
 if (!(self->v_state & VGA_STATE_FSCREENOFF)) {
  vga_wseq(regbase,VGA_SEQ_RESET,0x1);
  vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,
          (qr1 & VGA_SR01_RESERVED) |
          (mode->vm_seq_clock_mode & ~VGA_SR01_SCREEN_OFF));
  vga_wseq(regbase,VGA_SEQ_RESET,0x3);
 } else {
  u8 new_mode;
  new_mode  = mode->vm_seq_clock_mode | VGA_SR01_SCREEN_OFF;
  new_mode &= ~VGA_SR01_RESERVED;
  new_mode |= qr1 & VGA_SR01_RESERVED;
  if (new_mode != qr1) {
   vga_wseq(regbase,VGA_SEQ_RESET,0x1);
   vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,new_mode);
   vga_wseq(regbase,VGA_SEQ_RESET,0x3);
  }
 }
#endif
 atomic_rwlock_endwrite(&self->v_lock);
}

INTERN void KCALL
VGA_GetMode(VGA *__restrict self,
            struct vga_mode *__restrict mode) {
 void *regbase = self->v_mmio;
 atomic_rwlock_write(&self->v_lock);
 vga_r(regbase,VGA_IS1_RC),vga_w(regbase,VGA_ATT_W,0x00);
 vga_r(regbase,VGA_IS1_RC),mode->vm_att_mode         = vga_rattr(regbase,VGA_ATC_MODE) & ~VGA_AT10_RESERVED;
 vga_r(regbase,VGA_IS1_RC),mode->vm_att_overscan     = vga_rattr(regbase,VGA_ATC_OVERSCAN);
 vga_r(regbase,VGA_IS1_RC),mode->vm_att_plane_enable = vga_rattr(regbase,VGA_ATC_PLANE_ENABLE) & ~VGA_AT12_RESERVED;
 vga_r(regbase,VGA_IS1_RC),mode->vm_att_pel          = vga_rattr(regbase,VGA_ATC_PEL) & ~VGA_AT13_RESERVED;
 vga_r(regbase,VGA_IS1_RC),mode->vm_att_color_page   = vga_rattr(regbase,VGA_ATC_COLOR_PAGE) & ~VGA_AT14_RESERVED;
 vga_r(regbase,VGA_IS1_RC),vga_w(regbase,VGA_ATT_W,0x20);

 mode->vm_mis               = vga_r(regbase,VGA_MIS_R) & ~VGA_MIS_RESERVED;
 mode->vm_gfx_sr_value      = vga_rgfx(regbase,VGA_GFX_SR_VALUE) & ~VGA_GR00_RESERVED;
 mode->vm_gfx_sr_enable     = vga_rgfx(regbase,VGA_GFX_SR_ENABLE) & ~VGA_GR01_RESERVED;
 mode->vm_gfx_compare_value = vga_rgfx(regbase,VGA_GFX_COMPARE_VALUE) & ~VGA_GR02_RESERVED;
 mode->vm_gfx_data_rotate   = vga_rgfx(regbase,VGA_GFX_DATA_ROTATE) & ~VGA_GR03_RESERVED;
 mode->vm_gfx_mode          = vga_rgfx(regbase,VGA_GFX_MODE) & ~VGA_GR05_RESERVED;
 mode->vm_gfx_misc          = vga_rgfx(regbase,VGA_GFX_MISC) & ~VGA_GR06_RESERVED;
 mode->vm_gfx_compare_mask  = vga_rgfx(regbase,VGA_GFX_COMPARE_MASK) & ~VGA_GR07_RESERVED;
 mode->vm_gfx_bit_mask      = vga_rgfx(regbase,VGA_GFX_BIT_MASK);
 mode->vm_crt_h_total       = vga_rcrt(regbase,VGA_CRTC_H_TOTAL);
 mode->vm_crt_h_disp        = vga_rcrt(regbase,VGA_CRTC_H_DISP);
 mode->vm_crt_h_blank_start = vga_rcrt(regbase,VGA_CRTC_H_BLANK_START);
 mode->vm_crt_h_blank_end   = vga_rcrt(regbase,VGA_CRTC_H_BLANK_END);
 mode->vm_crt_h_sync_start  = vga_rcrt(regbase,VGA_CRTC_H_SYNC_START);
 mode->vm_crt_h_sync_end    = vga_rcrt(regbase,VGA_CRTC_H_SYNC_END);
 mode->vm_crt_v_total       = vga_rcrt(regbase,VGA_CRTC_V_TOTAL);
 mode->vm_crt_overflow      = vga_rcrt(regbase,VGA_CRTC_OVERFLOW);
 mode->vm_crt_preset_row    = vga_rcrt(regbase,VGA_CRTC_PRESET_ROW) & ~VGA_CR8_RESERVED;
 mode->vm_crt_max_scan      = vga_rcrt(regbase,VGA_CRTC_MAX_SCAN);
 mode->vm_crt_v_sync_start  = vga_rcrt(regbase,VGA_CRTC_V_SYNC_START);
 mode->vm_crt_v_sync_end    = vga_rcrt(regbase,VGA_CRTC_V_SYNC_END) & ~VGA_CR11_RESERVED;
 mode->vm_crt_v_disp_end    = vga_rcrt(regbase,VGA_CRTC_V_DISP_END);
 mode->vm_crt_offset        = vga_rcrt(regbase,VGA_CRTC_OFFSET);
 mode->vm_crt_underline     = vga_rcrt(regbase,VGA_CRTC_UNDERLINE);
 mode->vm_crt_v_blank_start = vga_rcrt(regbase,VGA_CRTC_V_BLANK_START);
 mode->vm_crt_v_blank_end   = vga_rcrt(regbase,VGA_CRTC_V_BLANK_END) & ~VGA_CR16_RESERVED;
 mode->vm_crt_mode          = vga_rcrt(regbase,VGA_CRTC_MODE) & ~VGA_CR17_RESERVED;
 mode->vm_crt_line_compare  = vga_rcrt(regbase,VGA_CRTC_LINE_COMPARE);
 mode->vm_seq_plane_write   = vga_rseq(regbase,VGA_SEQ_PLANE_WRITE) & ~VGA_SR02_RESERVED;
 mode->vm_seq_character_map = vga_rseq(regbase,VGA_SEQ_CHARACTER_MAP) & ~VGA_SR03_RESERVED;
 mode->vm_seq_memory_mode   = vga_rseq(regbase,VGA_SEQ_MEMORY_MODE) & ~VGA_SR04_RESERVED;
 mode->vm_seq_clock_mode    = vga_rseq(regbase,VGA_SEQ_CLOCK_MODE) & ~VGA_SR01_RESERVED;
 atomic_rwlock_endwrite(&self->v_lock);
}


INTERN void KCALL
VGA_SetFont(VGA *__restrict self,
            USER CHECKED struct vga_font const *__restrict font) {
 vm_copytophys(self->v_vram_addr,font,sizeof(struct vga_font));
}
INTERN void KCALL
VGA_GetFont(VGA *__restrict self,
            USER CHECKED struct vga_font *__restrict font) {
 vm_copyfromphys(font,self->v_vram_addr,sizeof(struct vga_font));
}


INTERN void KCALL
VGA_SetPalette(VGA *__restrict self,
               USER CHECKED struct vga_palette const *__restrict pal) {
 unsigned int EXCEPT_VAR i;
 void *regbase = self->v_mmio;
 atomic_rwlock_write(&self->v_lock);
 TRY {
  vga_w(regbase,VGA_PEL_MSK,0xff);
  vga_w(regbase,VGA_PEL_IW,0x00);
  for (i = 0; i < 768; ++i)
       vga_w(regbase,VGA_PEL_D,((u8 *)pal)[i] >> 2);
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Must complete the operation. - VGA wouldn't understand otherwise. */
  for (; i < 768; ++i)
       vga_w(regbase,VGA_PEL_D,0);
  atomic_rwlock_endwrite(&self->v_lock);
  error_rethrow();
 }
 atomic_rwlock_endwrite(&self->v_lock);
}
INTERN void KCALL
VGA_GetPalette(VGA *__restrict self,
               USER CHECKED struct vga_palette *__restrict pal) {
 VGA *EXCEPT_VAR xself = self;
 unsigned int EXCEPT_VAR i;
 void *regbase = self->v_mmio;
 atomic_rwlock_write(&self->v_lock);
 TRY {
  vga_w(regbase,VGA_PEL_MSK,0xff);
  vga_w(regbase,VGA_PEL_IR,0x00);
  for (i = 0; i < 768; ++i)
       ((u8 *)pal)[i] = vga_r(regbase,VGA_PEL_D) << 2;
 } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
  /* Must complete the operation. - VGA wouldn't understand otherwise. */
  for (; i < 768; ++i)
       vga_r(xself->v_mmio,VGA_PEL_D);
  atomic_rwlock_endwrite(&xself->v_lock);
  error_rethrow();
 }
 atomic_rwlock_endwrite(&self->v_lock);
}

INTERN void KCALL
VGA_ScreenOn(VGA *__restrict self) {
 u8 qr1;
 void *regbase = self->v_mmio;
 atomic_rwlock_write(&self->v_lock);
 qr1  = vga_rseq(regbase,VGA_SEQ_CLOCK_MODE);
 qr1 &= ~VGA_SR01_SCREEN_OFF;
 vga_wseq(regbase,VGA_SEQ_RESET,0x1);
 vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1);
 vga_wseq(regbase,VGA_SEQ_RESET,0x3);
 self->v_state &= ~VGA_STATE_FSCREENOFF;
 atomic_rwlock_endwrite(&self->v_lock);
}
INTERN void KCALL
VGA_ScreenOff(VGA *__restrict self) {
 u8 qr1;
 void *regbase = self->v_mmio;
 atomic_rwlock_write(&self->v_lock);
 qr1  = vga_rseq(regbase,VGA_SEQ_CLOCK_MODE);
 qr1 |= VGA_SR01_SCREEN_OFF;
 vga_wseq(regbase,VGA_SEQ_RESET,0x1);
 vga_wseq(regbase,VGA_SEQ_CLOCK_MODE,qr1);
 vga_wseq(regbase,VGA_SEQ_RESET,0x3);
 self->v_state |= VGA_STATE_FSCREENOFF;
 atomic_rwlock_endwrite(&self->v_lock);
}


PRIVATE REF struct vm_region *KCALL
VGA_MMap(VGA *__restrict self,
         vm_raddr_t page_index,
         vm_raddr_t *__restrict pregion_start) {
 REF struct vm_region *result,*new_result;
 result = self->v_vram;
 if (!result) {
  /* Construct a new physical memory region */
  result = vm_region_alloc(self->v_vram_size/PAGESIZE);
  result->vr_type           = VM_REGION_PHYSICAL;
  result->vr_part0.vp_state = VM_PART_INCORE;
  result->vr_part0.vp_flags = VM_PART_FWEAKREF|VM_PART_FNOSWAP;
  result->vr_part0.vp_phys.py_num_scatter = 1;
  result->vr_part0.vp_phys.py_iscatter[0].ps_addr = VM_ADDR2PAGE(self->v_vram_addr);
  result->vr_part0.vp_phys.py_iscatter[0].ps_size = self->v_vram_size;

  /* Save the constructed memory region. */
  new_result = ATOMIC_CMPXCH_VAL(self->v_vram,NULL,result);
  if unlikely(new_result) {
   vm_region_decref(result);
   result = new_result;
  }
 }
 /* Use the mmap-offset as region starting page. */
 *pregion_start = page_index;
 /* Return the new region. */
 vm_region_incref(result);
 return result;
}

PRIVATE ssize_t KCALL
VGA_Ioctl(VGA *__restrict self,
          unsigned long cmd, USER UNCHECKED void *arg,
          iomode_t flags) {
 ssize_t result = 0;
 switch (cmd) {

 case VGA_RESET:
  /* Reset various VGA components. */
  if ((unsigned int)(uintptr_t)arg & ~(VGA_RESET_FMODE|VGA_RESET_FFONT|VGA_RESET_FPAL|
                                       VGA_RESET_FON|VGA_RESET_FOFF))
       error_throw(E_INVALID_ARGUMENT);
  if ((unsigned int)(uintptr_t)arg & VGA_RESET_FOFF)
       VGA_ScreenOff(self);
  if ((unsigned int)(uintptr_t)arg & VGA_RESET_FMODE)
       VGA_SetMode(self,&vga_biosmode);
  if ((unsigned int)(uintptr_t)arg & VGA_RESET_FFONT)
       VGA_SetFont(self,&self->v_bios_font);
  if ((unsigned int)(uintptr_t)arg & VGA_RESET_FPAL)
       VGA_SetPalette(self,&vga_biospal);
  if ((unsigned int)(uintptr_t)arg & VGA_RESET_FON)
       VGA_ScreenOn(self);
  break;

 {
  struct vga_mode kernel_mode;
 case VGA_GETMODE:
  validate_writable(arg,sizeof(struct vga_mode));
  VGA_GetMode(self,&kernel_mode);
  COMPILER_BARRIER();
  memcpy(arg,&kernel_mode,sizeof(struct vga_mode));
 } break;

 case VGA_GETMODE_DEFAULT:
  validate_writable(arg,sizeof(struct vga_mode));
  memcpy(arg,&vga_biosmode,sizeof(struct vga_mode));
  break;

 {
  struct vga_mode kernel_mode;
 case VGA_SETMODE:
  validate_readable(arg,sizeof(struct vga_mode));
  memcpy(&kernel_mode,arg,sizeof(struct vga_mode));
  COMPILER_BARRIER();
  VGA_SetMode(self,&kernel_mode);
 } break;

 case VGA_SETMODE_DEFAULT:
  switch ((unsigned int)arg) {

  case VGA_DEFAULT_MODE_TXT80X25_16:
   VGA_SetMode(self,&vga_biosmode);
   break;

  case VGA_DEFAULT_MODE_GFX320X200_256:
   VGA_SetMode(self,&vga_mode_gfx320x200_256);
   break;

  default:
   error_throw(E_INVALID_ARGUMENT);
   break;
  }
  break;

 case VGA_GETPAL:
  validate_writable(arg,sizeof(struct vga_palette));
  VGA_GetPalette(self,(struct vga_palette *)arg);
  break;

 case VGA_GETPAL_DEFAULT:
  validate_writable(arg,sizeof(struct vga_palette));
  memcpy((struct vga_palette *)arg,&vga_biospal,sizeof(struct vga_palette));
  break;

 case VGA_SETPAL:
  validate_readable(arg,sizeof(struct vga_palette));
  VGA_SetPalette(self,(struct vga_palette *)arg);
  break;

 case VGA_SETPAL_DEFAULT:
  switch ((unsigned int)arg) {

  case VGA_DEFAULT_PAL_TXT16:
   VGA_SetPalette(self,&vga_biospal);
   break;

  case VGA_DEFAULT_PAL_GFX256:
   VGA_SetPalette(self,&vga_pal_gfx256);
   break;

  default:
   error_throw(E_INVALID_ARGUMENT);
   break;
  }
  break;

 case VGA_GETFONT:
  validate_writable(arg,sizeof(struct vga_font));
  VGA_GetFont(self,(struct vga_font *)arg);
  break;

 case VGA_GETFONT_DEFAULT:
  validate_writable(arg,sizeof(struct vga_font));
  memcpy((struct vga_font *)arg,&self->v_bios_font,sizeof(struct vga_font));
  break;

 case VGA_SETFONT:
  validate_readable(arg,sizeof(struct vga_font));
  VGA_SetFont(self,(struct vga_font *)arg);
  break;

 default:
  error_throw(E_NOT_IMPLEMENTED);
  break;
 }
 return result;
}



PRIVATE struct character_device_ops VGA_Ops = {
    .c_file = {
        .f_ioctl = (ssize_t(KCALL *)(struct character_device *__restrict,unsigned long,USER UNCHECKED void *,iomode_t))&VGA_Ioctl,
        .f_mmap  = (REF struct vm_region *(KCALL *)(struct character_device *__restrict,uintptr_t,uintptr_t *__restrict))&VGA_MMap,
    }
};


PRIVATE ATTR_FREETEXT void KCALL
vga_disable_annoying_blinking(void) {
 u8 qr1; /* QEMU don't emulate this annoyingness, so no need. */
 //if (boot_emulation == BOOT_EMULATION_QEMU) return;

 qr1 = vga_rseq(NULL,VGA_SEQ_CLOCK_MODE);
 vga_wseq(NULL,VGA_SEQ_RESET,0x1);
 vga_wseq(NULL,VGA_SEQ_CLOCK_MODE,qr1|VGA_SR01_SCREEN_OFF);
 vga_wseq(NULL,VGA_SEQ_RESET,0x3);

 vga_r(NULL,VGA_IS1_RC);
 vga_w(NULL,VGA_ATT_W,0x00);

 vga_r(NULL,VGA_IS1_RC);
 u8 temp = vga_rattr(NULL,VGA_ATC_MODE);
 vga_r(NULL,VGA_IS1_RC);
 vga_wattr(NULL,VGA_ATC_MODE,temp & ~(VGA_AT10_BLINK));

 vga_r(NULL,VGA_IS1_RC);
 vga_w(NULL,VGA_ATT_W,0x20);

 vga_wseq(NULL,VGA_SEQ_RESET,0x1);
 vga_wseq(NULL,VGA_SEQ_CLOCK_MODE,qr1 & ~VGA_SR01_SCREEN_OFF);
 vga_wseq(NULL,VGA_SEQ_RESET,0x3);
}

DEFINE_DRIVER_INIT(VGA_Init);
PRIVATE ATTR_USED ATTR_FREETEXT void KCALL VGA_Init(void) {
 VGA *EXCEPT_VAR dev;
 vga_disable_annoying_blinking();

 dev = CHARACTER_DEVICE_ALLOC(VGA);
 TRY {
  dev->v_dev.c_ops = &VGA_Ops;
  dev->v_crt_i     = VGA_CRT_IC;
  dev->v_crt_d     = VGA_CRT_DC;
  dev->v_is1_r     = VGA_IS1_RC;
  dev->v_vram_addr = 0xA0000;
  dev->v_vram_size = 8192*4*4; /* 128K */
  if unlikely(!(vga_r(dev->v_mmio,VGA_MIS_R) & VGA_MIS_COLOR)) {
   dev->v_crt_i = VGA_CRT_IM;
   dev->v_crt_d = VGA_CRT_DM;
   dev->v_is1_r = VGA_IS1_RM;
  }
  dev->v_dev.c_device.d_devno = DV_VGA;
  sprintf(dev->v_dev.c_device.d_namebuf,FREESTR("vga"));

  /* Load the original BIOS font. */
  VGA_GetFont(dev,&dev->v_bios_font);

  /* Register the VGA adapter device. */
  register_device(&dev->v_dev.c_device);
 } FINALLY {
  character_device_decref(&dev->v_dev);
 }
}



DECL_END

#endif /* !GUARD_KERNEL_MODULES_VGA_VGA_C */
