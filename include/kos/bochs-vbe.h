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
#ifndef _KOS_BOCHS_VBE_H
#define _KOS_BOCHS_VBE_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>
#include <bits/types.h>
#include <asm/ioctl.h>

__SYSDECL_BEGIN

/* NOTE: Video memory can be mmap()ed by passing a BOCHS-VBE
 *       device handle to the mmap() system call. */

#define BOCHS_VBE_GETENABLE     _IOR('K',140,unsigned int) /* Get the bochs-vbe-enable state (One of `BOCHS_VBE_F*') */
#define BOCHS_VBE_SETENABLE     _IOW('K',140,unsigned int) /* Set the bochs-vbe-enable state (One of `BOCHS_VBE_F*') */
#   define BOCHS_VBE_FDISABLED  0x0000  /* Bochs VBE is disabled (VGA controls the screen) */
#   define BOCHS_VBE_FENABLED   0x0001  /* Bochs VBE is enabled (VGA doesn't control the screen) */
#define BOCHS_VBE_BPPSUPP       _IOR('K',141,struct bochs_vbe_bppsupp) /* Get the set of supported BPP values. */
struct bochs_vbe_bppsupp {
    __uint16_t   *bs_supp;    /* [0..bs_bufsiz/2] User-buffer for available BPP modes. */
    __size_t      bs_bufsiz;  /* IN:  Available buffer size.
                               * OUT: Required buffer size. */
};

#define BOCHS_VBE_GETFORMAT     _IOR('K',142,struct bochs_vbe_format) /* Get the current display format. */
#define BOCHS_VBE_SETFORMAT     _IOW('K',142,struct bochs_vbe_format) /* Set the current display format. */
struct bochs_vbe_format {
    __uint16_t    vf_resx;    /* Resolution in X (1..1024) */
    __uint16_t    vf_resy;    /* Resolution in Y (1..768) */
    __uint16_t    vf_bpp;     /* Bits per pixel (Always supported: 8; optionally supported: 4,8,15,16,24,32)
                               * To query available depths, use the `BOCHS_VBE_BPPSUPP' ioctl() */
};



__SYSDECL_END

#endif /* !_KOS_BOCHS_VBE_H */
