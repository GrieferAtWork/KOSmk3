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
#ifndef _WM_FONT_H
#define _WM_FONT_H 1

#include "api.h"
#include "surface.h"
#include <features.h>
#include <bits/types.h>

__SYSDECL_BEGIN


typedef struct wm_text_state wm_text_state_t;
struct wm_text_state {
    wm_pixel_t            f_color;    /* Current text color. */
    wm_pixel_t            f_shadow;   /* Current shadow color. */
#define WM_TEXT_OPTION_FNORMAL 0x0000 /* Normal text flags. */
#define WM_TEXT_OPTION_FUNDERL 0x0001 /* add an UNDERLine to rendered text. */
#define WM_TEXT_OPTION_FCURSIV 0x0002 /* draw CURSIVe text */
#define WM_TEXT_OPTION_FDWBOLD 0x0004 /* DraW as BOLD */
    unsigned int          f_options;  /* Currently enabled text options. */
    signed char           f_shadow_x; /* Shadow offset in X */
    signed char           f_shadow_y; /* Shadow offset in Y
                                       * NOTE: When both this and `f_shadow_x' and ZERO(0), the shadow is disabled. */
};



typedef struct wm_font wm_font_t;
#ifdef __cplusplus
struct PACKED wm_font: wm_surface
#else
struct PACKED wm_font
#endif
{
    __WM_SURFACE_STRUCT_MEMBERS         /* Font sprite table.
                                         * The surface's size is one of:
                                         *  - 256  * 128  (16*8 16x16 characters)
                                         *  - 128  * 128  (16*8  8x16 characters)
                                         *  - 128  * 64   (16*8  8x8  characters)
                                         *  - 64   * 64   (16*8  4x8  characters)
                                         *  - 64   * 32   (16*8  4x4  characters)
                                         * Other surface sizes are not allowed. */
    unsigned short int    f_lnsiz;      /* [const] Height of a single line (in pixels). */
    unsigned short int    f_log2_chsizx;/* [const] log2(Max width of a character in pixels) */
    unsigned short int    f_log2_chsizy;/* [const] log2(Height of a character in pixels) */
    unsigned short int    f_chsizy;     /* [const] Height of a character in pixels */
    unsigned short int    f_tabsize;    /* [const] Min width of the \t character (in pixels). Also used as tab alignment:
                                         * new_x = start_x + (((x - start_x) + (f_tabsize - 1)) / f_tabsize) * f_tabsize; */
    struct wm_text_state  f_default;    /* [const] Default font rendering state when no override is given. */
    __uint8_t             f_ascii[128]; /* [const] Sprite sizes (in X) of the first 128 characters (ASCII). */
    /* TODO: Unicode support */
};


/* Draw text onto `dst', using `self' as font.
 * This function is able to interpret special characters such as '\r' or '\n'.
 * Since this function isn't able to stop drawing text once outside of whatever
 * region of a target surface you want text to appear on, you should probably
 * make use of `wm_surface_makeview()' to create a sub-view of your actual target
 * and pass it as `dst':
 * >> struct wm_surface_view my_view;
 * >> // Prevent text from being rendered outside our textbox.
 * >> wm_surface_makeview(&my_view,
 * >>                      my_surface,
 * >>                      textbox_x,
 * >>                      textbox_y,
 * >>                      textbox_size_x,
 * >>                      textbox_size_y);
 * >> wm_font_draw(my_font,text,strlen(text),&my_view,0,0);
 * Additionally, this function is able to interpret various ANSI-terminal escape
 * codes, allowing the user of this function to embed color overrides, as well
 * as special functions such as underlined, cursive, or bold text.
 * NOTE: This function will update `state' according to escape codes found in `text'
 * @param: state: When non-NULL, the font state used for rendering, including
 *                special options such as bold, or underline, as well as color.
 *                When NULL, use a copy of `self->f_default' instead.
 * @param: flags: Text render flags (Set of `WM_FONT_DRAW_F*')
 */
WMAPI void WMCALL
wm_font_draw(struct wm_font const *__restrict self,
             char const *__restrict text, __size_t textlen,
             __WM_SURFACE_POINTER_R dst, int x, int y,
             struct wm_text_state *state, unsigned int flags);
#define WM_FONT_DRAW_FNORMAL 0x0000 /* Normal text rendering flags. */
#define WM_FONT_DRAW_FSINGLE 0x0001 /* Draw text in a single line and ignore line-feed and carriage-return
                                     * characters, as well as ANSI TTY escape codes that adjust the vertical
                                     * text position. Basically, just draw a single line of regular, old text. */
#define WM_FONT_DRAW_FNOESCP 0x0002 /* Disable processing of escape codes, essentially preventing
                                     * the text being rendered from altering its colors. */


/* Return a standard system font indicated by `name'.
 * @param: name: One of `WM_FONT_SYSTEM_*'
 * @return: * : A pointer to the system default font in question.
 *              With the exception of `WM_FONT_SYSTEM_DEFAULT', this
 *              font is lazily allocated and loaded upon first access
 *              by the calling window client application.
 *              During this initial load, the function may throw any
 *              of the following errors, however will not throw any
 *              errors after a successful first call when passing the
 *              same `name' once again.
 * @throw: E_INVALID_ARGUMENT: The given `name' isn't a valid system font name.
 * @throw: E_BADALLOC:                                  [...]
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_FILE_NOT_FOUND:  [...] (When being read from a file)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_NOT_A_DIRECTORY: [...] (...)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_TOO_MANY_LINKS:  [...] (...)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_PATH_NOT_FOUND:  [...] (...)
 * @throw: E_FILESYSTEM_ERROR.ERROR_FS_ACCESS_ERROR:    [...] (...) */
WMAPI ATTR_RETNONNULL struct wm_font const *WMCALL wm_font_system(unsigned int name);
#define WM_FONT_SYSTEM_DEFAULT      0x0000 /* System default font. */



__SYSDECL_END

#endif /* !_WM_FONT_H */
