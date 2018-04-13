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
#ifndef GUARD_APPS_TERMINAL_VGA_MAIN_C
#define GUARD_APPS_TERMINAL_VGA_MAIN_C 1
#define _KOS_SOURCE 1

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <format-printer.h>
#include <hybrid/atomic.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kos/types.h>
#include <kos/keyboard.h>
#include <kos/keyboard_ioctl.h>
#include <sys/syslog.h>
#include <limits.h>
#include <pty.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <kos/ushare.h>
#include <unistd.h>

#include "libterm.h"

DECL_BEGIN

typedef u16 vtty_char_t;
#define VTTY_COLOR_BLACK          0
#define VTTY_COLOR_BLUE           1
#define VTTY_COLOR_GREEN          2
#define VTTY_COLOR_CYAN           3
#define VTTY_COLOR_RED            4
#define VTTY_COLOR_MAGENTA        5
#define VTTY_COLOR_BROWN          6
#define VTTY_COLOR_LIGHT_GREY     7
#define VTTY_COLOR_DARK_GREY      8
#define VTTY_COLOR_LIGHT_BLUE     9
#define VTTY_COLOR_LIGHT_GREEN    10
#define VTTY_COLOR_LIGHT_CYAN     11
#define VTTY_COLOR_LIGHT_RED      12
#define VTTY_COLOR_LIGHT_MAGENTA  13
#define VTTY_COLOR_LIGHT_BROWN    14
#define VTTY_COLOR_WHITE          15
#define VTTY_COLORS               16
#define vtty_entry_color(fg,bg) ((fg)|(bg) << 4)
#define vtty_entry(uc,color)    ((vtty_char_t)(uc)|(vtty_char_t)(color) << 8)

#define VTTY_DEFAULT_COLOR        vtty_entry_color(VTTY_COLOR_LIGHT_GREY,VTTY_COLOR_BLACK)
#define VTTY_ADDR               ((vtty_char_t *)phys_to_virt(0xB8000))
#define VTTY_WIDTH                80
#define VTTY_HEIGHT               25
#define VTTY_TABSIZE               4
#define VTTY_ENDADDR            (VTTY_ADDR+(VTTY_WIDTH*VTTY_HEIGHT))


PRIVATE struct term_rgba vga_colors[VTTY_COLORS] = {
#if 0 /* Perfect match for xterm 256 */
    [VTTY_COLOR_BLACK         ] = TERM_RGBA_INIT(0x00,0x00,0x00,0xff), // Black
    [VTTY_COLOR_BLUE          ] = TERM_RGBA_INIT(0x00,0x00,0x80,0xff), // Navy
    [VTTY_COLOR_GREEN         ] = TERM_RGBA_INIT(0x00,0x80,0x00,0xff), // Green
    [VTTY_COLOR_CYAN          ] = TERM_RGBA_INIT(0x00,0x80,0x80,0xff), // Teal
    [VTTY_COLOR_RED           ] = TERM_RGBA_INIT(0x80,0x00,0x00,0xff), // Maroon
    [VTTY_COLOR_MAGENTA       ] = TERM_RGBA_INIT(0x80,0x00,0x80,0xff), // Purple
    [VTTY_COLOR_BROWN         ] = TERM_RGBA_INIT(0x80,0x80,0x00,0xff), // Olive
    [VTTY_COLOR_LIGHT_GREY    ] = TERM_RGBA_INIT(0xc0,0xc0,0xc0,0xff), // Silver
    [VTTY_COLOR_DARK_GREY     ] = TERM_RGBA_INIT(0x80,0x80,0x80,0xff), // Grey
    [VTTY_COLOR_LIGHT_BLUE    ] = TERM_RGBA_INIT(0x00,0x00,0xff,0xff), // Blue
    [VTTY_COLOR_LIGHT_GREEN   ] = TERM_RGBA_INIT(0x00,0xff,0x00,0xff), // Lime
    [VTTY_COLOR_LIGHT_CYAN    ] = TERM_RGBA_INIT(0x00,0xff,0xff,0xff), // Aqua
    [VTTY_COLOR_LIGHT_RED     ] = TERM_RGBA_INIT(0xff,0x00,0x00,0xff), // Red
    [VTTY_COLOR_LIGHT_MAGENTA ] = TERM_RGBA_INIT(0xff,0x00,0xff,0xff), // Fuchsia
    [VTTY_COLOR_LIGHT_BROWN   ] = TERM_RGBA_INIT(0xff,0xff,0x00,0xff), // Yellow
    [VTTY_COLOR_WHITE         ] = TERM_RGBA_INIT(0xff,0xff,0xff,0xff), // White
#elif 0 /* Actual colors used by QEMU */
    [VTTY_COLOR_BLACK         ] = TERM_RGBA_INIT(0x00,0x00,0x00,0xff), // Black
    [VTTY_COLOR_BLUE          ] = TERM_RGBA_INIT(0x00,0x00,0xa8,0xff), // Navy
    [VTTY_COLOR_GREEN         ] = TERM_RGBA_INIT(0x00,0xa8,0x00,0xff), // Green
    [VTTY_COLOR_CYAN          ] = TERM_RGBA_INIT(0x00,0xa8,0xa8,0xff), // Teal
    [VTTY_COLOR_RED           ] = TERM_RGBA_INIT(0xa8,0x00,0x00,0xff), // Maroon
    [VTTY_COLOR_MAGENTA       ] = TERM_RGBA_INIT(0xa8,0x00,0xa8,0xff), // Purple
    [VTTY_COLOR_BROWN         ] = TERM_RGBA_INIT(0xa8,0x57,0x00,0xff), // Olive
    [VTTY_COLOR_LIGHT_GREY    ] = TERM_RGBA_INIT(0xa8,0xa8,0xa8,0xff), // Silver
    [VTTY_COLOR_DARK_GREY     ] = TERM_RGBA_INIT(0x57,0x57,0x57,0xff), // Grey
    [VTTY_COLOR_LIGHT_BLUE    ] = TERM_RGBA_INIT(0x57,0x57,0xff,0xff), // Blue
    [VTTY_COLOR_LIGHT_GREEN   ] = TERM_RGBA_INIT(0x57,0xff,0x57,0xff), // Lime
    [VTTY_COLOR_LIGHT_CYAN    ] = TERM_RGBA_INIT(0x57,0xff,0xff,0xff), // Aqua
    [VTTY_COLOR_LIGHT_RED     ] = TERM_RGBA_INIT(0xff,0x57,0x57,0xff), // Red
    [VTTY_COLOR_LIGHT_MAGENTA ] = TERM_RGBA_INIT(0xff,0x57,0xff,0xff), // Fuchsia
    [VTTY_COLOR_LIGHT_BROWN   ] = TERM_RGBA_INIT(0xff,0xff,0x57,0xff), // Yellow
    [VTTY_COLOR_WHITE         ] = TERM_RGBA_INIT(0xff,0xff,0xff,0xff), // White
#elif 1 /* Aproximation for best results (Use this!) */
    [VTTY_COLOR_BLACK         ] = TERM_RGBA_INIT(0x00,0x00,0x00,0xff), // Black
    [VTTY_COLOR_BLUE          ] = TERM_RGBA_INIT(0x00,0x00,0xAA,0xff), // Navy
    [VTTY_COLOR_GREEN         ] = TERM_RGBA_INIT(0x00,0xAA,0x00,0xff), // Green
    [VTTY_COLOR_CYAN          ] = TERM_RGBA_INIT(0x00,0xAA,0xAA,0xff), // Teal
    [VTTY_COLOR_RED           ] = TERM_RGBA_INIT(0xAA,0x00,0x00,0xff), // Maroon
    [VTTY_COLOR_MAGENTA       ] = TERM_RGBA_INIT(0xAA,0x00,0xAA,0xff), // Purple
    [VTTY_COLOR_BROWN         ] = TERM_RGBA_INIT(0xAA,0xAA,0x00,0xff), // Olive
    [VTTY_COLOR_LIGHT_GREY    ] = TERM_RGBA_INIT(0xAA,0xAA,0xAA,0xff), // Silver
    [VTTY_COLOR_DARK_GREY     ] = TERM_RGBA_INIT(0x80,0x80,0x80,0xff), // Grey
    [VTTY_COLOR_LIGHT_BLUE    ] = TERM_RGBA_INIT(0x00,0x00,0xff,0xff), // Blue
    [VTTY_COLOR_LIGHT_GREEN   ] = TERM_RGBA_INIT(0x00,0xff,0x00,0xff), // Lime
    [VTTY_COLOR_LIGHT_CYAN    ] = TERM_RGBA_INIT(0x00,0xff,0xff,0xff), // Aqua
    [VTTY_COLOR_LIGHT_RED     ] = TERM_RGBA_INIT(0xff,0x00,0x00,0xff), // Red
    [VTTY_COLOR_LIGHT_MAGENTA ] = TERM_RGBA_INIT(0xff,0x00,0xff,0xff), // Fuchsia
    [VTTY_COLOR_LIGHT_BROWN   ] = TERM_RGBA_INIT(0xff,0xff,0x00,0xff), // Yellow
    [VTTY_COLOR_WHITE         ] = TERM_RGBA_INIT(0xff,0xff,0xff,0xff), // White
#else /* Color codes according to VGA? (I think...) */
    [VTTY_COLOR_BLACK         ] = TERM_RGBA_INIT(0x00,0x00,0x00,0xff),
    [VTTY_COLOR_BLUE          ] = TERM_RGBA_INIT(0x00,0x00,0xaa,0xff),
    [VTTY_COLOR_GREEN         ] = TERM_RGBA_INIT(0x00,0xaa,0x00,0xff),
    [VTTY_COLOR_CYAN          ] = TERM_RGBA_INIT(0x00,0xaa,0xaa,0xff),
    [VTTY_COLOR_RED           ] = TERM_RGBA_INIT(0xaa,0x00,0x00,0xff),
    [VTTY_COLOR_MAGENTA       ] = TERM_RGBA_INIT(0xaa,0x00,0xaa,0xff),
    [VTTY_COLOR_BROWN         ] = TERM_RGBA_INIT(0xaa,0x55,0x00,0xff),
    [VTTY_COLOR_LIGHT_GREY    ] = TERM_RGBA_INIT(0xaa,0xaa,0xaa,0xff),
    [VTTY_COLOR_DARK_GREY     ] = TERM_RGBA_INIT(0x80,0x55,0x55,0xff),
    [VTTY_COLOR_LIGHT_BLUE    ] = TERM_RGBA_INIT(0x55,0x55,0xff,0xff),
    [VTTY_COLOR_LIGHT_GREEN   ] = TERM_RGBA_INIT(0x55,0xaa,0x55,0xff),
    [VTTY_COLOR_LIGHT_CYAN    ] = TERM_RGBA_INIT(0x55,0xff,0xff,0xff),
    [VTTY_COLOR_LIGHT_RED     ] = TERM_RGBA_INIT(0xff,0x55,0x55,0xff),
    [VTTY_COLOR_LIGHT_MAGENTA ] = TERM_RGBA_INIT(0xff,0x55,0xff,0xff),
    [VTTY_COLOR_LIGHT_BROWN   ] = TERM_RGBA_INIT(0xff,0xff,0x55,0xff),
    [VTTY_COLOR_WHITE         ] = TERM_RGBA_INIT(0xff,0xff,0xff,0xff),
#endif
};

/* UGH... Doing this correctly would have waaaay too huge of an overhead.
   But I have to admit that this approximation is pretty CRAP... */
#define sqr(x) ((x)*(x))
#if 0
#define rgba_distance(a,b) (unsigned int)\
 (sqr(((int)(b).sr-(int)(a).sr)*30)+\
  sqr(((int)(b).sg-(int)(a).sg)*59)+\
  sqr(((int)(b).sb-(int)(a).sb)*11))
#elif 1
#define rgba_distance(a,b) (unsigned int)\
 (sqr((a).sr-(b).sr)+\
  sqr((a).sg-(b).sg)+\
  sqr((a).sb-(b).sb))
#else
#define rgba_distance(a,b) (unsigned int)\
 (abs((a).sr-(b).sr)+\
  abs((a).sg-(b).sg)+\
  abs((a).sb-(b).sb))
#endif


LOCAL u8 vga_color(struct term_rgba const cl) {
 unsigned int d,winner = UINT_MAX;
 u8 i = 0,result = 0;
 for (; i < VTTY_COLORS; ++i) {
  d = rgba_distance(cl,vga_colors[i]);
  if (d < winner) result = i,winner = d;
 }
#if 0
 k_syslogf(KLOG_MSG,
           "COLOR: %u {%I8x,%I8x,%I8x} -> {%I8x,%I8x,%I8x}\n",
           winner,cl.r,cl.g,cl.b,
           vga_colors[result].r,
           vga_colors[result].g,
           vga_colors[result].b);
#endif
 return result;
}


typedef uint16_t cell_t;

/* user-level mappings of the VGA physical memory.
 * NOTE: This memory should be considered write-only. */
#if __KOS_VERSION__ < 300
PRIVATE int     vga_fd;
#endif
PRIVATE cell_t *vga_dev;
PRIVATE cell_t *vga_buf,*vga_bufpos,*vga_bufend;
PRIVATE cell_t *vga_bufsecondline,*vga_lastline;
#define vga_spaceline   vga_bufend
PRIVATE cell_t  vga_attrib;
#define VGA_SIZE   (VTTY_WIDTH*VTTY_HEIGHT)
#define CHR(x)    ((cell_t)(unsigned char)(x)|vga_attrib)
#define SPACE       CHR(' ')
#define INVCHR(x) ((cell_t)(unsigned char)(x)\
   | ((cell_t)vga_invert[(vga_attrib&0xf000) >> 12] << 12)\
   | ((cell_t)vga_invert[(vga_attrib&0x0f00) >> 8] << 8))


// Cell/Line access
#define DEV_CELL(x,y)  (vga_dev+(x)+(y)*VTTY_WIDTH)
#define BUF_CELL(x,y)  (vga_buf+(x)+(y)*VTTY_WIDTH)
#define DEV_LINE(y)    (vga_dev+(y)*VTTY_WIDTH)
#define BUF_LINE(y)    (vga_buf+(y)*VTTY_WIDTH)
#define DEV_CELL_CUR() (vga_dev+(vga_bufpos-vga_buf))
#define BUF_CELL_CUR() (vga_bufpos)

// Blit commands (swap buffers)
#define BLIT_CNT(start,count)  memcpy(vga_dev+(start),vga_buf+(start),(count)*sizeof(cell_t))
#define BLIT()                 BLIT_CNT(0,VGA_SIZE)
#define BLIT_LINE(y)           memcpy(DEV_LINE(y),BUF_LINE(y),VTTY_WIDTH*sizeof(cell_t))
#define BLIT_CELL(x,y)        (*DEV_CELL(x,y) = *BUF_CELL(x,y))
#define BLIT_CUR()            (*DEV_CELL_CUR() = *BUF_CELL_CUR())
#define BLIT_BUF(buf)         (vga_dev[(buf)-vga_buf] = *(buf))
#define BLIT_CUR_INV()        (*DEV_CELL_CUR() = INVERT(*BUF_CELL_CUR()))

// Get/Set/Move the cursor
#define GET_CUR_X()    (coord_t)((vga_bufpos-vga_buf)%VTTY_WIDTH)
#define GET_CUR_Y()    (coord_t)((vga_bufpos-vga_buf)/VTTY_WIDTH)
#define SET_CUR(x,y)   (vga_bufpos = vga_buf+((x)+(y)*VTTY_WIDTH))
#define SET_CUR_X(x)   (vga_bufpos -= (((vga_bufpos-vga_buf) % VTTY_WIDTH)-(x)))
#define SET_CUR_Y(y)    SET_CUR(GET_CUR_X(),y)
#define MOV_CUR(ox,oy) (vga_bufpos += ((ox)+(oy)*VTTY_WIDTH))
#define MOV_CUR_X(ox)  (vga_bufpos += (ox))
#define MOV_CUR_Y(oy)  (vga_bufpos += (ox)*VTTY_WIDTH)
#define CUR_LINE()     (vga_bufpos-((vga_bufpos-vga_buf) % VTTY_WIDTH))

#define CR()            SET_CUR_X(0)
#define LF()           (vga_bufpos += (VTTY_WIDTH-((vga_bufpos-vga_buf) % VTTY_WIDTH)))
#define BACK()         (vga_bufpos != vga_buf ? --vga_bufpos : vga_bufpos)

/* Terminal configuration */
#define HAVE_CURSOR_BLINK        1
#define HAVE_CURSOR_BLINK_THREAD 0
#define HAVE_CURSOR_LOCK         1
#define INVERT_CURSOR_AFTER_MOVE 1
#define TTY_DEVNAME              "/dev/vga"
#undef TERM_BELL


#if !HAVE_CURSOR_BLINK
#undef HAVE_CURSOR_BLINK_THREAD
#define HAVE_CURSOR_BLINK_THREAD 0
#undef INVERT_CURSOR_AFTER_MOVE
#define INVERT_CURSOR_AFTER_MOVE 0
#undef HAVE_CURSOR_LOCK
#define HAVE_CURSOR_LOCK 0
#endif

PRIVATE             pid_t relay_incoming_thread;
#if HAVE_CURSOR_BLINK_THREAD
PRIVATE             pid_t cursor_blink_thread;
#endif
#if HAVE_CURSOR_BLINK
PRIVATE ATOMIC_DATA int   cursor_blink_enabled = 1;
PRIVATE             int   cursor_inverted = 0;
#endif

#if HAVE_CURSOR_LOCK
PRIVATE DEFINE_ATOMIC_RWLOCK(cursor_lock);
#define CURSOR_LOCK   atomic_rwlock_write(&cursor_lock);
#define CURSOR_UNLOCK atomic_rwlock_endwrite(&cursor_lock);
#else
#define CURSOR_LOCK   /* Nothing */
#define CURSOR_UNLOCK /* Nothing */
#endif



#if defined(TERM_BELL) || HAVE_CURSOR_BLINK
PRIVATE u8 vga_invert[VTTY_COLORS] = {
    [VTTY_COLOR_BLACK]          = VTTY_COLOR_WHITE,
    [VTTY_COLOR_BLUE]           = VTTY_COLOR_LIGHT_BROWN,
    [VTTY_COLOR_GREEN]          = VTTY_COLOR_LIGHT_MAGENTA,
    [VTTY_COLOR_CYAN]           = VTTY_COLOR_LIGHT_RED,
    [VTTY_COLOR_RED]            = VTTY_COLOR_LIGHT_CYAN,
    [VTTY_COLOR_MAGENTA]        = VTTY_COLOR_LIGHT_GREEN,
    [VTTY_COLOR_BROWN]          = VTTY_COLOR_LIGHT_BLUE,
    [VTTY_COLOR_LIGHT_GREY]     = VTTY_COLOR_DARK_GREY,
    [VTTY_COLOR_DARK_GREY]      = VTTY_COLOR_LIGHT_GREY,
    [VTTY_COLOR_LIGHT_BLUE]     = VTTY_COLOR_BROWN,
    [VTTY_COLOR_LIGHT_GREEN]    = VTTY_COLOR_MAGENTA,
    [VTTY_COLOR_LIGHT_CYAN]     = VTTY_COLOR_RED,
    [VTTY_COLOR_LIGHT_RED]      = VTTY_COLOR_CYAN,
    [VTTY_COLOR_LIGHT_MAGENTA]  = VTTY_COLOR_GREEN,
    [VTTY_COLOR_LIGHT_BROWN]    = VTTY_COLOR_BLUE,
    [VTTY_COLOR_WHITE]          = VTTY_COLOR_BLACK,
};
#endif

#if HAVE_CURSOR_BLINK
PRIVATE void cursor_enable(void) {
 if (!ATOMIC_CMPXCH(cursor_blink_enabled,0,1)) return;
#if HAVE_CURSOR_BLINK_THREAD
 kill(cursor_blink_thread,SIGCONT);
#endif
}
PRIVATE void cursor_disable(void) {
 if (!ATOMIC_CMPXCH(cursor_blink_enabled,1,0)) return;
#if HAVE_CURSOR_BLINK_THREAD
 kill(cursor_blink_thread,SIGSTOP);
#endif
 if (cursor_inverted) {
  /* Fix inverted cursor */
  BLIT_CUR();
  cursor_inverted = 0;
 }
}
#define BEGIN_MOVE_CUR()      { CURSOR_LOCK if (cursor_inverted) BLIT_CUR(); }
#define END_MOVE_CUR()        { CURSOR_UNLOCK }
#else
#define cursor_enable()  (void)0
#define cursor_disable() (void)0
#define BEGIN_MOVE_CUR()      { CURSOR_LOCK }
#define END_MOVE_CUR()        { CURSOR_UNLOCK }
#endif

#define INVERT(cell) \
 (((cell)&0xff)\
   | ((cell_t)vga_invert[((cell)&0xf000) >> 12] << 12)\
   | ((cell_t)vga_invert[((cell)&0x0f00) >> 8] << 8))


#if HAVE_CURSOR_BLINK_THREAD
PRIVATE void blink_cur(void) {
 cell_t cur;
 CURSOR_LOCK
 assert(vga_bufpos <= vga_bufend);
 if (vga_bufpos != vga_bufend) {
  cur = *BUF_CELL_CUR();
  cursor_inverted ^= 1;
  if (cursor_inverted) cur = INVERT(cur);
  *DEV_CELL_CUR() = cur;
 }
 CURSOR_UNLOCK
}
PRIVATE ATTR_NORETURN void cursor_blink_threadmain(void) {
 for (;;) { usleep(300000); blink_cur(); }
}
#endif /* HAVE_CURSOR_BLINK_THREAD */

#ifdef TERM_BELL
PRIVATE void blit_inverted(void) {
 cell_t *iter,*end,*dst,cell;
 iter = vga_buf,end = vga_bufend;
 dst = vga_dev;
 for (; iter != end; ++iter,++dst) {
  cell = *iter;
  cell = INVERT(cell);
  *dst = cell;
 }
}
#endif


PRIVATE void term_doscroll(void) {
 memmove(vga_buf,vga_bufsecondline,
        (VGA_SIZE-VTTY_WIDTH)*sizeof(cell_t));
 memcpy(vga_lastline,vga_spaceline,VTTY_WIDTH*sizeof(cell_t));
 vga_bufpos = vga_lastline;
}
PRIVATE void term_doput(char ch) {
 if (vga_bufpos == vga_bufend) {
  /* Scroll at the end of the terminal */
  term_doscroll();
#if INVERT_CURSOR_AFTER_MOVE
  if (cursor_blink_enabled) {
   cursor_inverted = 1;
   *vga_bufpos++ = INVCHR(ch);
   BLIT();
   vga_bufpos[-1] = CHR(ch);
  } else
#endif
  {
   *vga_bufpos++ = CHR(ch);
   BLIT();
  }
 } else {
  *vga_bufpos = CHR(ch);
  BLIT_CUR();
#if INVERT_CURSOR_AFTER_MOVE
  if (cursor_blink_enabled) {
   cursor_inverted = 1;
   ++vga_bufpos;
   if (vga_bufpos != vga_bufend) {
    BLIT_CUR_INV();
   }
  } else
#endif
  {
   ++vga_bufpos;
   BLIT_CUR();
  }
 }
}

PRIVATE void TERM_CALL term_putc(struct term *UNUSED(t), char ch) {
 /* v This introducing lag is actually something that's currently intended. */
 //k_syslogf(KLOG_MSG,"%c",ch);
 BEGIN_MOVE_CUR()
 switch (ch) {
 case TERM_CR: CR(); break;
 case TERM_LF:
  //printf(":AFTER LS: %d\n",vga_bufpos == vga_bufend);
  if (vga_bufpos != vga_bufend) LF();
  else term_doscroll();
  break;
 case TERM_BACK:
  BACK();
#if INVERT_CURSOR_AFTER_MOVE
  if (cursor_blink_enabled) {
   cursor_inverted = 1;
   BLIT_CUR_INV();
  }
#endif
  break;
 {
  size_t chrs;
 case TERM_TAB:
  chrs = TERM_TABSIZE-(GET_CUR_X() % TERM_TABSIZE);
  while (chrs--) term_doput(' ');
 } break;
#ifdef TERM_BELL
 case TERM_BELL: // Bell
  /* Wait until a current retrace has ended. */
  while (inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE) task_yield();
  /* Draw the screen inverted. */
  blit_inverted();
  while (!(inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE)) task_yield();
  while (inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE) task_yield();
  usleep(1000);
  while (inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE) task_yield();
  while (!(inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE)) task_yield();
  /* Draw the screen normal again. */
  BLIT();
  /* Wait until the next retrace starts. */
  while (inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE) task_yield();
  while (!(inb(VGA_IS1_RC)&VGA_IS1_V_RETRACE)) task_yield();
  usleep(1000);
#else
 case '\a':
#endif
#if INVERT_CURSOR_AFTER_MOVE
  if (cursor_blink_enabled) {
   cursor_inverted = 1;
   BLIT_CUR_INV();
  }
#endif
  break;
 default: term_doput(ch); break;
 }
 END_MOVE_CUR()
}

PRIVATE uint8_t const vga_box_chars[] = {
 178,0,0,0,0,248,241,0,0,217,191,218,192,197,196,
 196,196,196,196,195,180,193,194,179,243,242,220
};

PRIVATE void TERM_CALL term_putb(struct term *UNUSED(t), char ch) {
 assert(ch >= 'a' && ch <= 'z');
 ch = vga_box_chars[ch-'a'];
 /*if (ch)*/ term_putc(NULL,ch);
}

PRIVATE void TERM_CALL
term_set_color(struct term *UNUSED(t),
               struct term_rgba fg,
               struct term_rgba bg) {
 vga_attrib = (vga_color(fg) | vga_color(bg) << 4) << 8;
}
PRIVATE void TERM_CALL term_set_cursor(struct term *UNUSED(t), coord_t x, coord_t y) {
 if (x >= VTTY_WIDTH)  x = *(__s32 *)&x < 0 ? 0 : VTTY_WIDTH-1;
 if (y >= VTTY_HEIGHT) y = *(__s32 *)&y < 0 ? 0 : VTTY_HEIGHT-1;
 BEGIN_MOVE_CUR()
 SET_CUR(x,y);
#if INVERT_CURSOR_AFTER_MOVE
 if (cursor_blink_enabled) {
  cursor_inverted = 1;
  BLIT_CUR_INV();
 }
#endif
 END_MOVE_CUR()
}
PRIVATE void TERM_CALL term_get_cursor(struct term *UNUSED(t), coord_t *x, coord_t *y) {
 *x = GET_CUR_X();
 *y = GET_CUR_Y();
}
PRIVATE void TERM_CALL term_show_cursor(struct term *UNUSED(t), int cmd) {
 if (cmd == TERM_SHOWCURSOR_YES) {
  cursor_enable();
 } else {
  cursor_disable();
 }
}
PRIVATE void TERM_CALL term_cls(struct term *UNUSED(t), int mode) {
 cell_t *begin,*end,*iter,filler = SPACE;
 switch (mode) {
 case TERM_CLS_BEFORE: begin = vga_buf; end = vga_bufpos; break;
 case TERM_CLS_AFTER : begin = vga_bufpos; end = vga_bufend; break;
 default             : begin = vga_buf; end = vga_bufend; break;
 }
 for (iter = begin; iter != end; ++iter) *iter = filler;
 BLIT_CNT(begin-vga_buf,end-begin);
#if INVERT_CURSOR_AFTER_MOVE
 if (cursor_blink_enabled) {
  cursor_inverted = 1;
  BLIT_CUR_INV();
 }
#endif
}
PRIVATE void TERM_CALL term_el(struct term *UNUSED(t), int mode) {
 cell_t *begin,*end,*iter,filler = SPACE;
 bool refresh_all = vga_bufpos == vga_bufend;
 if (refresh_all) term_doscroll();
 switch (mode) {
 case TERM_EL_BEFORE: begin = CUR_LINE(); end = vga_bufpos; break;
 case TERM_EL_AFTER : begin = vga_bufpos; end = CUR_LINE()+VTTY_WIDTH; break;
 default            : begin = CUR_LINE(); end = begin+VTTY_WIDTH; break;
 }
 for (iter = begin; iter != end; ++iter) *iter = filler;
 if (refresh_all) BLIT();
 else BLIT_CNT(begin-vga_buf,end-begin);
#if INVERT_CURSOR_AFTER_MOVE
 if (cursor_blink_enabled) {
  cursor_inverted = 1;
  BLIT_CUR_INV();
 }
#endif
}
PRIVATE void TERM_CALL term_scroll(struct term *UNUSED(t), offset_t offset) {
 cell_t filler = SPACE;
 cell_t *new_begin,*new_end;
 size_t copycells;
 /*if (!offset) return;*/
 if (offset >= VTTY_HEIGHT || offset <= -VTTY_HEIGHT) {
  term_cls(NULL,TERM_CLS_ALL);
  return;
 }
 if (offset > 0) {
  // Shift lines upwards
  copycells = (VTTY_HEIGHT-offset)*VTTY_WIDTH;
  memmove(vga_buf,vga_buf+offset*VTTY_WIDTH,copycells*sizeof(cell_t));
  new_begin = vga_buf+copycells;
  new_end = vga_bufend;
 } else {
  // Shift lines downwards
  copycells = (VTTY_HEIGHT+offset)*VTTY_WIDTH;
  memmove(vga_buf-offset*VTTY_WIDTH,vga_buf,copycells*sizeof(cell_t));
  new_end = (new_begin = vga_buf)+copycells;
 }
 for (; new_begin != new_end; ++new_begin)
  *new_begin = filler;
 BLIT();
}

PRIVATE struct term_operations const term_ops = {
    .to_putc          = &term_putc,
    .to_putb          = &term_putb,
    .to_set_color     = &term_set_color,
    .to_set_cursor    = &term_set_cursor,
    .to_get_cursor    = &term_get_cursor,
    .to_show_cursor   = &term_show_cursor,
    .to_cls           = &term_cls,
    .to_el            = &term_el,
    .to_scroll        = &term_scroll,
    .to_set_title     = NULL,
    .to_putimg        = NULL,
    .to_get_cell_size = NULL,
    .to_output        = NULL,
};

PRIVATE struct winsize const winsize = {
    .ws_row    = VTTY_HEIGHT,
    .ws_col    = VTTY_WIDTH,
    .ws_xpixel = 1,
    .ws_ypixel = 1,
};

PRIVATE ATTR_NORETURN void
usage(char const *name, int exitcode) {
 printf("Usage: %s EXEC-AFTER\n",name);
 exit(exitcode);
}

PRIVATE int keyboard_fd;
PRIVATE struct keyboard_keymap const *current_keymap;
PRIVATE struct term pty;
PRIVATE int amaster,aslave;
PRIVATE keyboard_state_t keystate;

/* Parse and relay keyboard-style inputs from the terminal driver's STDIN */
PRIVATE ATTR_NORETURN void relay_incoming_threadmain(void) {
 char text[32]; char *iter; keyboard_key_t key; ssize_t s;
 while ((s = read(keyboard_fd,&key,sizeof(keyboard_key_t))) ==
                          (ssize_t)sizeof(keyboard_key_t)) {
rescan:
  switch (key) {
#define MIRROR(key,state) \
  case KEYDOWN(key): KEYBOARD_STATE_FADD(keystate,state); break; \
  case KEYUP  (key): KEYBOARD_STATE_FDEL(keystate,state); break;
#define TOGGLE(key,state) \
  case KEYDOWN(key): KEYBOARD_STATE_FXOR(keystate,state); break;

   /* Update the keyboard state. */
  MIRROR(KEY_LSHIFT,KEYBOARD_STATE_FLSHIFT)
  MIRROR(KEY_RSHIFT,KEYBOARD_STATE_FRSHIFT)
  MIRROR(KEY_LCTRL,KEYBOARD_STATE_FLCTRL)
  MIRROR(KEY_RCTRL,KEYBOARD_STATE_FRCTRL)
  MIRROR(KEY_LGUI,KEYBOARD_STATE_FLGUI)
  MIRROR(KEY_RGUI,KEYBOARD_STATE_FRGUI)
  MIRROR(KEY_LALT,KEYBOARD_STATE_FLALT)
  MIRROR(KEY_RALT,KEYBOARD_STATE_FRALT)
  MIRROR(KEY_ESC,KEYBOARD_STATE_FESC)
  MIRROR(KEY_TAB,KEYBOARD_STATE_FTAB)
  MIRROR(KEY_SPACE,KEYBOARD_STATE_FSPACE)
  MIRROR(KEY_APPS,KEYBOARD_STATE_FAPPS)
  MIRROR(KEY_ENTER,KEYBOARD_STATE_FENTER)
  MIRROR(KEY_KP_ENTER,KEYBOARD_STATE_FKP_ENTER)
  MIRROR(KEY_INSERT,KEYBOARD_STATE_FOVERRIDE)

  TOGGLE(KEY_CAPSLOCK,KEYBOARD_STATE_FCAPSLOCK)
  TOGGLE(KEY_NUMLOCK,KEYBOARD_STATE_FNUMLOCK)
  TOGGLE(KEY_SCROLLLOCK,KEYBOARD_STATE_FSCROLLLOCK)

#undef TOGGLE
#undef MIRROR
  default: break;
  }

  if (!KEY_ISDOWN(key)) continue;
  iter = text;

  if (!KEYBOARD_STATE_FISNUMLOCK(keystate)) {
   /* Translate numpad keys to secondary actions unless numlock is on. */
   switch (key) {
   case KEY_KP_0:   key = KEY_INSERT; goto rescan;
   case KEY_KP_1:   key = KEY_END;    goto rescan;
   case KEY_KP_2:   key = KEY_DOWN;   goto rescan;
   case KEY_KP_3:   key = KEY_PGDOWN; goto rescan;
   case KEY_KP_4:   key = KEY_LEFT;   goto rescan;
   case KEY_KP_6:   key = KEY_RIGHT;  goto rescan;
   case KEY_KP_7:   key = KEY_HOME;   goto rescan;
   case KEY_KP_8:   key = KEY_UP;     goto rescan;
   case KEY_KP_9:   key = KEY_PGUP;   goto rescan;
   case KEY_KP_DOT: key = KEY_DELETE; goto rescan;
   default: break;
   }
  }

  if (KEYBOARD_STATE_FISALT(keystate) &&
     (!KEYBOARD_STATE_FISALTGR(keystate) ||
       key >= 256 ||
      !current_keymap->km_altgr[key].kmc_utf8[0]))
     *iter++ = TERM_ESCAPE;

  switch (key) {
  case KEY_F1    : iter = stpcpy(text,TERM_ESCAPE_S "[11~"); break;
  case KEY_F2    : iter = stpcpy(text,TERM_ESCAPE_S "[12~"); break;
  case KEY_F3    : iter = stpcpy(text,TERM_ESCAPE_S "[13~"); break;
  case KEY_F4    : iter = stpcpy(text,TERM_ESCAPE_S "[14~"); break;
  case KEY_F5    : iter = stpcpy(text,TERM_ESCAPE_S "[15~"); break;
  case KEY_F6    : iter = stpcpy(text,TERM_ESCAPE_S "[17~"); break;
  case KEY_F7    : iter = stpcpy(text,TERM_ESCAPE_S "[18~"); break;
  case KEY_F8    : iter = stpcpy(text,TERM_ESCAPE_S "[19~"); break;
  case KEY_F9    : iter = stpcpy(text,TERM_ESCAPE_S "[20~"); break;
  case KEY_F10   : iter = stpcpy(text,TERM_ESCAPE_S "[21~"); break;
  case KEY_F11   : iter = stpcpy(text,TERM_ESCAPE_S "[23~"); break;
  case KEY_F12   : iter = stpcpy(text,TERM_ESCAPE_S "[24~"); break;
  case KEY_UP    : iter = stpcpy(text,KEYBOARD_STATE_FISCTRL(keystate) ? TERM_ESCAPE_S "OA" : TERM_ESCAPE_S "[A"); break;
  case KEY_DOWN  : iter = stpcpy(text,KEYBOARD_STATE_FISCTRL(keystate) ? TERM_ESCAPE_S "OB" : TERM_ESCAPE_S "[B"); break;
  case KEY_RIGHT : iter = stpcpy(text,KEYBOARD_STATE_FISCTRL(keystate) ? TERM_ESCAPE_S "OC" : TERM_ESCAPE_S "[C"); break;
  case KEY_LEFT  : iter = stpcpy(text,KEYBOARD_STATE_FISCTRL(keystate) ? TERM_ESCAPE_S "OD" : TERM_ESCAPE_S "[D"); break;
  case KEY_PGUP  : iter = stpcpy(text,TERM_ESCAPE_S "[5~"); break;
  case KEY_PGDOWN: iter = stpcpy(text,TERM_ESCAPE_S "[6~"); break;
  case KEY_HOME  : iter = stpcpy(text,TERM_ESCAPE_S "OH"); break;
  case KEY_END   : iter = stpcpy(text,TERM_ESCAPE_S "OF"); break;
  case KEY_DELETE: iter = stpcpy(text,TERM_ESCAPE_S "[3~"); break;
  case KEY_ENTER:  iter = stpcpy(text,"\r"); break; /* Apparently this is a thing... */

  case KEY_LALT:
  case KEY_RALT:
   goto send_nothing;

  case KEY_TAB:
   if (KEYBOARD_STATE_FISCAPS(keystate)) {
    *iter++ = TERM_ESCAPE;
    *iter++ = '[';
    *iter++ = 'Z';
    break;
   }
  default:
   if (key < 256) {
    struct keyboard_keymap_char ch;
    if (KEYBOARD_STATE_FISALTGR(keystate)) {
     ch = current_keymap->km_altgr[key];
     if (!ch.kmc_utf8[0]) goto check_other;
    } else check_other: if (KEYBOARD_STATE_FISCAPS(keystate))
     ch = current_keymap->km_shift[key];
    else {
     ch = current_keymap->km_press[key];
    }
    if (ch.kmc_utf8[0]) {
     if (KEYBOARD_STATE_FISCTRL(keystate) &&
        (!KEYBOARD_STATE_FISALTGR(keystate) ||
         !current_keymap->km_altgr[key].kmc_utf8[0])) {
      /* Create control character (e.g.: CTRL+C --> '\03') */
      ch.kmc_utf8[0] = toupper(ch.kmc_utf8[0])-'@';
      ch.kmc_utf8[1] = 0;
      ch.kmc_utf8[2] = 0;
      ch.kmc_utf8[3] = 0;
     }
     *iter++ = ch.kmc_utf8[0];
     if (ch.kmc_utf8[1]) *iter++ = ch.kmc_utf8[1];
     if (ch.kmc_utf8[2]) *iter++ = ch.kmc_utf8[2];
     if (ch.kmc_utf8[3]) *iter++ = ch.kmc_utf8[3];
    }
   }
   break;
  }
  if (iter != text) {
#if 0
   syslog(LOG_DEBUG,
          "[TERMINAL-VGA] Relay: %Iu %$q\n",
          iter-text,(size_t)(iter-text),text);
#endif
   if (write(amaster,text,(size_t)(iter-text)) == -1) {
    perror("Relay failed");
    break;
   }
  }
send_nothing:;
 }
 syslog(LOG_DEBUG,"s = %Id\n",s);
 err(EXIT_FAILURE,"Unexpected input relay failure");
}

PRIVATE void *LIBCCALL map_phys(void *pptr, size_t size) {
 struct mmap_info info;
 /* Map the ushare segment into our address space. */
 info.mi_addr         = NULL;
 info.mi_prot         = PROT_READ|PROT_WRITE;
 info.mi_flags        = MAP_PRIVATE;
 info.mi_xflag        = XMAP_PHYSICAL;
 info.mi_size         = size;
 info.mi_align        = PAGESIZE;
 info.mi_gap          = 0;
 info.mi_tag          = NULL;
 info.mi_phys.mp_addr = pptr;
 return xmmap(&info);
}

int main(int argc, char *argv[]) {
 pid_t child_proc; int result;
 /* TODO: Re-write to support multiple pages allocated in a ring-buffer
  *       were appending a new line only requires shifting of some
  *       base-pointer, as well as duplication into VGA memory. */

 if (argc < 2) { usage(argv[0],EXIT_FAILURE); }

#if __KOS_VERSION__ >= 300
  keyboard_fd = open("/dev/keyboard",O_RDONLY);
 if (keyboard_fd < 0)
     err(EXIT_FAILURE,"Failed to open keyboard");
 current_keymap = (struct keyboard_keymap *)mmap(NULL,
                                                 sizeof(struct keyboard_keymap),
                                                 PROT_READ,
                                                 MAP_PRIVATE,
                                                 keyboard_fd,
                                                 KEYBOARD_KEYMAP_OFFSET);
 if (current_keymap == MAP_FAILED)
     err(EXIT_FAILURE,"Failed to load keyboard map");
 /* Map the x86 VGA terminal. */
 vga_dev = (cell_t *)map_phys((void *)0xB8000,PAGESIZE);
 if (vga_dev == (cell_t *)MAP_FAILED)
     err(EXIT_FAILURE,"Failed to mmap() VGA terminal");
#else
 vga_fd = open(TTY_DEVNAME,O_RDWR);
 if (vga_fd < 0) err(EXIT_FAILURE,"Failed to open %q",TTY_DEVNAME);
 fcntl(vga_fd,F_SETFL,FD_CLOEXEC|FD_CLOFORK);

 vga_dev = (cell_t *)mmap(0,PAGESIZE,PROT_READ|PROT_WRITE,MAP_SHARED,vga_fd,0);
 if (vga_dev == (cell_t *)MAP_FAILED)
     err(EXIT_FAILURE,"Failed to mmap() VGA terminal");
#endif

 /*printf("Mapped terminal driver to %p\n",vga_dev);*/
 vga_buf = (cell_t *)malloc((VGA_SIZE+VTTY_WIDTH)*sizeof(cell_t));
 if (!vga_buf) err(EXIT_FAILURE,"Failed to allocate VGA buffer");
 vga_bufend        = vga_buf+VGA_SIZE;
 vga_bufpos        = vga_buf;
 vga_bufsecondline = vga_buf+VTTY_WIDTH;
 vga_lastline      = vga_buf+(VGA_SIZE-VTTY_WIDTH);
 { cell_t *end = vga_bufend+VTTY_WIDTH;
   cell_t *iter = vga_buf,filler = SPACE;
   for (; iter != end; ++iter) *iter = filler;
 }
 if (!term_init(&pty,&term_ops,NULL,NULL))
      err(EXIT_FAILURE,"Failed to create terminal host");

 /* The terminal driver runtime is loaded and operational! */

 /* Create the PTY device. */
 if (openpty(&amaster,&aslave,NULL,NULL,&winsize) == -1)
     err(EXIT_FAILURE,"Failed to create PTY device");
 fcntl(amaster,F_SETFL,FD_CLOEXEC);

#if 1
 if ((child_proc = fork()) == 0) {
  /* === Child process */
  /* Redirect all standard files to the terminal */
  dup2(aslave,STDIN_FILENO);
  dup2(aslave,STDOUT_FILENO);
  dup2(aslave,STDERR_FILENO);
  /* Close all other descriptors */
  if (aslave >= 3) close(aslave);

  /* Set the child process as TTY foreground app. */
  tcsetpgrp(STDIN_FILENO,getpid());

  putenv("TERM=xterm");
  putenv("TERMINFO=/usr/share/terminfo");
  //putenv("NCURSES_TRACE=8191"); /* 0x1fff */

  /* Exec the given process */
  execv(argv[1],argv+1);
  perror("Failed to exec given process");
  exit(errno);
 } else if (child_proc == -1)
#endif
 {
  perror("Failed to fork child process");
  exit(EXIT_FAILURE);
 }

 /* Close the slave end of the terminal driver */
 close(aslave);

 /* Spawn helper threads */
#if HAVE_CURSOR_BLINK_THREAD
 if ((cursor_blink_thread = fork()) == 0)
      cursor_blink_threadmain();
#endif
 if ((relay_incoming_thread = fork()) == 0) {
  relay_incoming_threadmain();
 } else if (relay_incoming_thread < 0) {
  err(EXIT_FAILURE,"Failed to start relay-incoming process");
 }

 /*k_syslogf(KLOG_DEBUG,"Updating screen for the first time\n");*/
 BLIT();

 syslog(LOG_DEBUG,"Entering relay-display loop on %u\n",syscall(__NR_gettid));
 for (;;) {
  ssize_t s; char buf[128];
  /* Relay everything being printed to the terminal towards the screen. */
  while ((s = read(amaster,buf,sizeof(buf))) > 0) {
#if 0
   format_printf(&term_printer,&pty,"%$q\n",s,buf);
   syslog(LOG_DEBUG,"TERM: %$q\n",s,buf);
#else
   term_printer(buf,(size_t)s,&pty);
#endif
  }
  if (s < 0) {
   if (errno == EINTR) {
    /* Check if we were interrupted because the child died. */
    siginfo_t info;
    if (waitid(P_PID,child_proc,&info,WNOHANG|WEXITED) >= 0) {
     result = WEXITSTATUS(info.si_status);
     goto child_joined;
    }
    continue;
   }
   result = errno;
   error(0,result,"Unexpected slave output read failure");
  }
  break;
 }
 /* The pipe was broken. - Wait for the child to terminate. */
 syslog(LOG_DEBUG,"TERMINAL-VGA: Waiting for child process %d\n",child_proc);
 waitpid(child_proc,&result,WEXITED);
 result = WEXITSTATUS(result);
child_joined:

#if HAVE_CURSOR_BLINK_THREAD
 if (kill(cursor_blink_thread,SIGKILL))
     perror("Failed to kill <cursor_blink_thread>");
#endif
 if (kill(relay_incoming_thread,SIGKILL))
     perror("Failed to kill <relay_incoming_thread>");

#if 1
 term_fini(&pty);
 free(vga_buf);
 munmap(vga_dev,VGA_SIZE*sizeof(cell_t));
#if __KOS_VERSION__ < 300
 close(vga_fd);
#endif
#endif

 return result;
}

DECL_END

#endif /* !GUARD_APPS_TERMINAL_VGA_MAIN_C */
