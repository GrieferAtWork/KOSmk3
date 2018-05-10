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
#ifndef GUARD_KERNEL_INCLUDE_DEV_MOUSE_H
#define GUARD_KERNEL_INCLUDE_DEV_MOUSE_H 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <kos/mouse-ioctl.h>
#include <hybrid/atomic.h>
#include <sched/mutex.h>
#include <fs/device.h>
#include <fs/wordbuffer.h>
#include <fs/iomode.h>
#include <stdint.h>
#include <stdbool.h>
#include <sched/task.h>

/* Mouse Abstract Kernel API Interface. */

DECL_BEGIN

struct mouse;

struct mouse_ops {
    /* Mouse finalizer */
    void (KCALL *mo_fini)(struct mouse *__restrict self);

    /* [1..1][locked(m_lock)] Enable/Disable scanning of keycodes. */
    void (KCALL *mo_enable_reports)(struct mouse *__restrict self);
    void (KCALL *mo_disable_reports)(struct mouse *__restrict self);
};


#define MOUSE_ACTION_FOVERFL 0x00 /* [1] 0,0,0,0,0,0,0,0  Read data from the overflow buffer. */
#define MOUSE_ACTION_FMOVY   0x01 /* [2] 0,0,0,0,0,0,0,1  [y,Y,Y,Y,Y,Y,Y,Y] */
#define MOUSE_ACTION_FPCKT32 0x0e /* [5] 0,0,0,0,1,1,1,0  [T,T,T,T,T,T,T,T]*4 (Written in big endian to ensure that the last byte isn't ZERO) */
#define MOUSE_ACTION_FPCKT64 0x0f /* [9] 0,0,0,0,1,1,1,1  [T,T,T,T,T,T,T,T]*8 (Written in big endian to ensure that the last byte isn't ZERO) */
#define MOUSE_ACTION_FDOWN   0x10 /* [1] 0,0,0,1,S,S,S,S  (S -- shift for button is (`1 << S' is one of `MOUSE_BUTTON_F*'))  */
#define MOUSE_ACTION_FUP     0x20 /* [1] 0,0,1,0,S,S,S,S  (S -- see `MOUSE_ACTION_FDOWN')  */
#define MOUSE_ACTION_FUP_GET(a)       ((a) & 0xf)
#define MOUSE_ACTION_FPCKT4  0x30 /* [1] 0,0,1,1,T,T,T,T  (T -- Time since previous packet in jiffies) */
#define MOUSE_ACTION_FPCKT4_GET(a)    ((a) & 0xf)
#define MOUSE_ACTION_FMOVZ   0x40 /* [1] 0,1,w,W,W,W,W,W  (W -- Wheel delta; w -- sign bit) */
#define MOUSE_ACTION_FMOVZ_GET(a)     ((s8)(((a) & 0x20) << 2) | (((a) & 0x20) << 1) | ((a) & 0x3f))
#define MOUSE_ACTION_FMOVXY  0x80 /* [2] 1,y,Y,Y,Y,Y,Y,Y   x,X,X,X,X,X,X,X */
#define MOUSE_ACTION_FMOVXY_GETX(a,b) ((int)(s8)(b))
#define MOUSE_ACTION_FMOVXY_GETY(a,b) (((s8)(((a) & 0x40) << 1)) | ((a) & 0x7f))

#define MOUSE_BUTTON_COUNT  5
struct mouse_overflow {
    u64 ms_time;                     /* Relative time since the previous packet (in jiffies). */
    s64 ms_relx;                     /* Relative mouse X motion since previous packet. */
    s64 ms_rely;                     /* Relative mouse Y motion since previous packet. */
    s64 ms_relz;                     /* Relative mouse Z motion since previous packet. */
    u16 ms_down[MOUSE_BUTTON_COUNT]; /* Number of times a button was pushed down. */
    u16 ms_up[MOUSE_BUTTON_COUNT];   /* Number of times a button was released. */
};


#define MOUSE_ACTION_BACKLOG_SIZE  2048

union PACKED mouse_action_state {
   ATOMIC_DATA u32        as_word; /* Action state control word. */
   struct PACKED {
       ATOMIC_DATA u16    as_rcnt; /* [< MOUSE_ACTION_BACKLOG_SIZE] Number of action bytes that remain to be read. */
       ATOMIC_DATA u16    as_wptr; /* [< MOUSE_ACTION_BACKLOG_SIZE] Index into `m_act' where the next action is written. */
   };
};

struct mouse {
    /* Mouse packet buffering is implemented to be as smart as possible,
     * while still ensuring that even in a scenario of absolutely horrendous
     * lag, no mouse button presses, or mouse motion will ever become lost.
     * How is this done? Well... I didn't say that the order in which the
     * user did those things was kept...
     * Essentially, this means that when the mouse action backlog has filled
     * up, we start updating the `m_overflow' field to contain the sum of all
     * motion data, and button press counts for mouse events which happened
     * after the buffer filled up. Once mouse data is eventually read again,
     * all backed up data from the full backlog will be read, following which
     * a packet containing all motion data from `m_overflow' is send, before
     * whatever number of packets it takes to describe all the mouse button
     * presses the user did is sent.
     * Note however, that this mechanism only takes effect when either no one
     * is reading from the mouse device, or when there is some extreme lag
     * going on. */
    struct character_device    m_dev;       /* Underlying character device. */
    struct mouse_ops          *m_ops;       /* [1..1][const] Mouse operators. */
    mutex_t                    m_lock;      /* Lock for mouse properties. */
    struct mouse_overflow      m_overflow;  /* Overflow mouse state. */
#define MOUSE_FREPORTING       0x0001       /* The mouse buffer was fill */
#define MOUSE_FMERGEONREAD     0x0002       /* [lock(m_readlock)]
                                             * Allow mouse packets to be merged if the only thing
                                             * that changed in the mean time is the cursor, or wheel
                                             * position, in which case their relative offsets are
                                             * simply added to each other. */
#define MOUSE_FHASLATEBUTTON   0x0004       /* [lock(m_readlock)] Late button operations from
                                             * `m_down_late' and `m_up_late' must be processed. */
    u16                        m_flags;     /* Mouse flags. */
    u16                        m_buttons;   /* Last read mouse button state. */
    union mouse_action_state   m_act_state; /* Mouse action state. */
    byte_t                     m_act[MOUSE_ACTION_BACKLOG_SIZE]; /* Mouse action data backlog. */
    struct async_sig           m_avail;     /* Signal broadcast when new data arrives. */
    ATOMIC_DATA jtime_t        m_lastwrite; /* [lock(CALLER_IS_PRODUCER)] Absolute time of the last written packet. */
    jtime_t                    m_lastread;  /* [lock(m_readlock)] Absolute time of the last read packet. */
    atomic_rwlock_t            m_readlock;  /* Lock used to ensure correct timings of produces mouse packets. */
    u16                        m_down_late[MOUSE_BUTTON_COUNT]; /* [lock(m_readlock)] Late button down operations. */
    u16                        m_up_late[MOUSE_BUTTON_COUNT];   /* [lock(m_readlock)] Late button up operations. */
};

/* Increment/decrement the reference counter of the given mouse `x' */
#define mouse_incref(x) character_device_incref(&(x)->m_dev)
#define mouse_decref(x) character_device_decref(&(x)->m_dev)

/* Allocate and pre/null/zero-initialize a new mouse device,
 * who's control structure has a size of `struct_size' bytes.
 * The caller must initialize the following members:
 *    - m_dev.c_device.d_namebuf      (Unless `d_name' is set directly)
 *    - m_dev.c_device.d_name         (Optionally; pre-initialized to `d_namebuf')
 *    - m_dev.c_device.d_devno        (See explanation below)
 * @throw: E_DRIVER_CLOSED: The given driver has been closed/is closing.
 * Following this, a device ID should be allocated for, or assigned
 * to the device, either by directly setting `return->d_device.d_devno',
 * or by calling `devno_alloc()', or `register_dynamic_device()' (see above.) */
FUNDEF ATTR_RETNONNULL REF struct mouse *
KCALL __mouse_alloc(size_t struct_size,
                    struct mouse_ops *__restrict ops,
                    struct driver *__restrict caller)
                    ASMNAME("mouse_alloc");
#define mouse_alloc(struct_size,ops) \
      __mouse_alloc(struct_size,ops,&this_driver)
#define MOUSE_ALLOC(T,ops) \
  ((T *)mouse_alloc(sizeof(T),ops))


/* Feed the mouse some action data.
 * `data' should be a tightly packed vector, as described by `MOUSE_ACTION_FPCKT32'. */
FUNDEF ATTR_NOTHROW ASYNCSAFE NOMP void KCALL
mouse_action(struct mouse *__restrict self,
             byte_t const *__restrict data,
             u16 datalen);

/* Read a single mouse packet from the given mouse.
 * @return: true:  Successfully read a mouse packet.
 * @return: false: `IO_NONBLOCK' was passed and no data was available immediately. */
FUNDEF bool KCALL
mouse_read(struct mouse *__restrict self,
           struct mouse_packet *__restrict packet,
           iomode_t mode);


LOCAL byte_t *KCALL
mouse_encode_movxy(byte_t *__restrict buf, s16 relx, s16 rely) {
 s8 x_part,y_part;
 while (relx || rely) {
  if (!relx) {
   /* Must encode the remainder using only y-movements. */
   /**/ if (rely < INT8_MIN) y_part = INT8_MIN;
   else if (rely > INT8_MAX) y_part = INT8_MAX;
   else /*                */ y_part = (s8)rely;
   *buf++ = (byte_t)MOUSE_ACTION_FMOVY;
   *buf++ = (byte_t)y_part;
  } else {
   /* Encode x + y movement. */
   /**/ if (relx < INT8_MIN) x_part = INT8_MIN;
   else if (relx > INT8_MAX) x_part = INT8_MAX;
   else /*                */ x_part = (s8)relx;
   /**/ if (rely < INT8_MIN/2) y_part = INT8_MIN/2;
   else if (rely > INT8_MAX/2) y_part = INT8_MAX/2;
   else /*                */ y_part = (s8)rely;
   *buf++ = MOUSE_ACTION_FMOVXY|(byte_t)y_part;
   *buf++ = (byte_t)x_part;
   relx -= x_part;
  }
  rely -= y_part;
 }
 return buf;
}

LOCAL byte_t *KCALL
mouse_encode_movz(byte_t *__restrict buf, s16 relz) {
 s8 z_part;
 while (relz) {
  /**/ if (relz < INT8_MIN/4) z_part = INT8_MIN/4;
  else if (relz > INT8_MAX/4) z_part = INT8_MAX/4;
  else /*                */ z_part = (s8)relz;
  *buf++ = MOUSE_ACTION_FMOVZ|(z_part & 0x3f);
  relz -= z_part;
 }
 return buf;
}

LOCAL byte_t *KCALL
mouse_encode_down(byte_t *__restrict buf, u8 button_index) {
 *buf++ = MOUSE_ACTION_FDOWN | button_index;
 return buf;
}

LOCAL byte_t *KCALL
mouse_encode_up(byte_t *__restrict buf, u8 button_index) {
 *buf++ = MOUSE_ACTION_FUP | button_index;
 return buf;
}

LOCAL byte_t *KCALL
mouse_encode_updown(byte_t *__restrict buf, u8 button_index, bool is_down) {
 *buf++ = (is_down ? MOUSE_ACTION_FDOWN : MOUSE_ACTION_FUP) | button_index;
 return buf;
}

LOCAL byte_t *KCALL
mouse_encode_pack(byte_t *__restrict buf, struct mouse *__restrict m) {
 jtime_t now = jiffies;
 jtime_t before = ATOMIC_XCH(m->m_lastwrite,now);
 now -= before;
 if (now > UINT32_MAX) {
  /* Encode as 64-bit time shift */
  *buf++ = MOUSE_ACTION_FPCKT64;
  *buf++ = (byte_t)(now >> 56);
  *buf++ = (byte_t)(now >> 48);
  *buf++ = (byte_t)(now >> 40);
  *buf++ = (byte_t)(now >> 32);
  *buf++ = (byte_t)(now >> 24);
  *buf++ = (byte_t)(now >> 16);
  *buf++ = (byte_t)(now >> 8);
  *buf++ = (byte_t)now;
 } else if (now > 0xf) {
  /* Encode as 32-bit time shift */
  *buf++ = MOUSE_ACTION_FPCKT32;
  *buf++ = (byte_t)(now >> 24);
  *buf++ = (byte_t)(now >> 16);
  *buf++ = (byte_t)(now >> 8);
  *buf++ = (byte_t)now;
 } else {
  /* Encode immediate operand time shift */
  *buf++ = MOUSE_ACTION_FPCKT4|(byte_t)now;
 }
 return buf;
}


DECL_END

#endif /* !GUARD_KERNEL_INCLUDE_DEV_MOUSE_H */
